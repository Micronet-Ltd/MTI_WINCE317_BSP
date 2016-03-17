//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
*/
//
//  This code implements boot-time activation of registered Bluetooth peers,
//  as well as authentication UI.
//
//  It is heavily dependent on registry key structure created by
//  public\common\oak\drivers\netui components (btmgmtui).
//
//  Please maintain synchronization between the two files.
//
//

#include <windows.h>
#include <service.h>
#include <stdio.h>
#include <devload.h>

#include <svsutil.hxx>

#include <winsock2.h>
#include <ws2bth.h>
#include <bt_sdp.h>
#include <bthapi.h>
#include <bt_api.h>

#include "btmodem_api.h"
#include "btmodem.h"
#include "gsm_api.h"
#include "gsm710_api.h"
#include <bsp.h>

//#define   DUMPCOMM            1

#define BTH_MODEM_TIMEOUT   15000
#define BTH_MODEM_MTU       256
#define BUF_MAX             1024
#define BTH_MODEM_REGBASE   L"software\\microsoft\\bluetooth\\modemgw"

#define BTH_MODEM_BAUD      115200

#define BTH_MODEM_HARDWARE  1
#define BTH_MODEM_SOFTWARE  2
#define BTH_MODEM_NONE      3

#define BTH_BLUETOOTH_READER L"Bluetooth Port Reader"
#define BTH_MODEM_READER     L"Modem Port Reader"

#define MAX_WRITE_FAILS     5
#define MAX_READ_FAILS      5

#define CRLF L"\r\n"

static DWORD            gfServiceState = SERVICE_STATE_OFF;
static HANDLE           ghServiceThread;
static HANDLE           ghServiceExitEvent;
static HANDLE           ghQueueExitEvent;
static HANDLE           ghServiceRequestEvent;
static BOOL				gfAcceptConnection =  TRUE;
static HANDLE           ghMsgQueueThread = NULL;
static HANDLE			ghInMsgQueue = INVALID_HANDLE_VALUE;
static HANDLE			ghOutMsgQueue = INVALID_HANDLE_VALUE;

#define SDP_RECORD_SIZE 0x0000004d
#define SDP_CHANNEL_OFFSET 26

static const BYTE rgbSdpRecordDUN[] = {
    0x35, 0x4b, 0x09, 0x00, 0x01, 0x35, 0x03, 0x19,
    0x11, 0x03, 0x09, 0x00, 0x04, 0x35, 0x0c, 0x35,
    0x03, 0x19, 0x01, 0x00, 0x35, 0x05, 0x19, 0x00,
    0x03, 0x08,
                0x01,                               // server channel goes here (+26)
                      0x09, 0x00, 0x06, 0x35, 0x09,
    0x09, 0x65, 0x6e, 0x09, 0x00, 0x6a, 0x09, 0x01,
    0x00, 0x09, 0x00, 0x09, 0x35, 0x08, 0x35, 0x06,
    0x19, 0x11, 0x03, 0x09, 0x01, 0x00, 0x09, 0x01,
    0x00, 0x25, 0x12, 'D',  'i',  'a',  'l',  '-',
    'u',  'p',  ' ',  'N',  'e',  't',  'w',  'o',
    'r',  'k',  'i',  'n',  'g'
};

extern "C" BOOL WINAPI DllMain( HANDLE hInstDll, DWORD fdwReason, LPVOID lpvReserved) {
    switch(fdwReason) {
        case DLL_PROCESS_ATTACH:
            break;

        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

static ULONG RegisterSDP (HANDLE hFile) {
    ULONG recordHandle = 0;

    DWORD port = 0;
    DWORD dwSizeOut = 0;

    if (! DeviceIoControl (hFile, IOCTL_BLUETOOTH_GET_RFCOMM_CHANNEL, NULL, 0, &port, sizeof(port), &dwSizeOut, NULL)) {
        RETAILMSG(1, (L"Bluetooth modem gateway: Failed to retrieve port server channel, error = %d" CRLF, GetLastError ()));

        return 0;
    }

    struct {
        BTHNS_SETBLOB   b;
        unsigned char   uca[SDP_RECORD_SIZE];
    } bigBlob;

    ULONG ulSdpVersion = BTH_SDP_VERSION;

    bigBlob.b.pRecordHandle   = &recordHandle;
    bigBlob.b.pSdpVersion     = &ulSdpVersion;
    bigBlob.b.fSecurity       = 0;
    bigBlob.b.fOptions        = 0;
    bigBlob.b.ulRecordLength  = SDP_RECORD_SIZE;

    memcpy (bigBlob.b.pRecord, rgbSdpRecordDUN, SDP_RECORD_SIZE);

    bigBlob.b.pRecord[SDP_CHANNEL_OFFSET] = (unsigned char)port;

    BLOB blob;
    blob.cbSize    = sizeof(BTHNS_SETBLOB) + SDP_RECORD_SIZE - 1;
    blob.pBlobData = (PBYTE) &bigBlob;

    WSAQUERYSET Service;
    memset (&Service, 0, sizeof(Service));
    Service.dwSize = sizeof(Service);
    Service.lpBlob = &blob;
    Service.dwNameSpace = NS_BTH;

    int iRet = BthNsSetService (&Service, RNRSERVICE_REGISTER,0);
    if (iRet != ERROR_SUCCESS) {
        RETAILMSG(1, (L"Bluetooth modem gateway: BthNsSetService fails with status %d" CRLF, iRet));
        return 0;
    }

    RETAILMSG(1, (L"Bluetooth modem gateway: Created SDP record 0x%08x, channel %d" CRLF, recordHandle, port));
    return recordHandle;
}

static void DeRegisterSDP (ULONG recordHandle) {
    ULONG ulSdpVersion = BTH_SDP_VERSION;

    BTHNS_SETBLOB delBlob;
    memset (&delBlob, 0, sizeof(delBlob));
    delBlob.pRecordHandle = &recordHandle;
    delBlob.pSdpVersion = &ulSdpVersion;

    BLOB blob;
    blob.cbSize    = sizeof(BTHNS_SETBLOB);
    blob.pBlobData = (PBYTE) &delBlob;

    WSAQUERYSET Service;

    memset (&Service, 0, sizeof(Service));
    Service.dwSize = sizeof(Service);
    Service.lpBlob = &blob;
    Service.dwNameSpace = NS_BTH;

    int iErr = BthNsSetService (&Service, RNRSERVICE_DELETE, 0);
    RETAILMSG (1, (L"Bluetooth modem gateway: removed SDP record 0x%08x (%d)" CRLF, recordHandle, iErr));
}

static UINT32 FindFreeBluetoothPort () 
{
	for (int i = 2 ; i < 10 ; ++i) {
		WCHAR szPortName[20];
		StringCchPrintf (STRING_AND_COUNTOF(szPortName), L"BSP%d:", i);

		HANDLE hFile = CreateFile (szPortName, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return i;
		CloseHandle (hFile);
	}

	return -1;
}

static HANDLE CreatePort (int port, int channel, int mtu, int fa, int fe) {
    PORTEMUPortParams pp;

    memset (&pp, 0, sizeof(pp));
    pp.flocal  = TRUE;
    pp.channel = channel;
    pp.imtu    = mtu;

    if (fa)
        pp.uiportflags |= RFCOMM_PORT_FLAGS_AUTHENTICATE;

    if (fe)
        pp.uiportflags |= RFCOMM_PORT_FLAGS_ENCRYPT;

    WCHAR szKeyName[_MAX_PATH];

    StringCchPrintf (STRING_AND_COUNTOF(szKeyName), BTH_MODEM_REGBASE L"\\BSP%d", port);

    HKEY hk;
    DWORD dwDisp = 0;

    if (ERROR_SUCCESS != RegCreateKeyEx (HKEY_LOCAL_MACHINE, szKeyName, 0, NULL, 0, KEY_WRITE, NULL, &hk, &dwDisp)) {
        RETAILMSG(1, (L"Bluetooth modem gateway: Failed to create registry key %s, error = %d" CRLF, szKeyName, GetLastError ()));
        return NULL;
    }

    RegSetValueEx (hk, L"dll", 0, REG_SZ, (BYTE *)L"btd.dll", sizeof(L"btd.dll"));
    RegSetValueEx (hk, L"prefix", 0, REG_SZ, (BYTE *)L"BSP", sizeof(L"BSP"));

    DWORD dw = port;
    RegSetValueEx (hk, L"index", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));

    dw = (DWORD) &pp;
    RegSetValueEx (hk, L"context", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));

    RegCloseKey (hk);

    HANDLE hDevice = ActivateDevice (szKeyName, 0);

    if (! hDevice) {
        RETAILMSG(1, (L"Bluetooth modem gateway: Failed to activate device @ %s (error = %d)" CRLF, szKeyName, GetLastError ()));
    }

    RegDeleteKey (HKEY_LOCAL_MACHINE, szKeyName);

    return hDevice;
}

#if defined (DUMPCOMM)
#define BPR     8

static void DumpBuff (WCHAR *szLineHeader, unsigned char *lpBuffer, unsigned int cBuffer) {
    WCHAR szLine[5 + 7 + 2 + 4 * BPR];

    for (int i = 0 ; i < (int)cBuffer ; i += BPR) {
        int bpr = cBuffer - i;
        if (bpr > BPR)
            bpr = BPR;

        wsprintf (szLine, L"%04x ", i);
        WCHAR *p = szLine + wcslen (szLine);

        for (int j = 0 ; j < bpr ; ++j) {
            WCHAR c = (lpBuffer[i + j] >> 4) & 0xf;
            if (c > 9) c += L'a' - 10; else c += L'0';
            *p++ = c;
            c = lpBuffer[i + j] & 0xf;
            if (c > 9) c += L'a' - 10; else c += L'0';
            *p++ = c;
            *p++ = L' ';
        }

        for ( ; j < BPR ; ++j) {
            *p++ = L' ';
            *p++ = L' ';
            *p++ = L' ';
        }

        *p++ = L' ';
        *p++ = L' ';
        *p++ = L' ';
        *p++ = L'|';
        *p++ = L' ';
        *p++ = L' ';
        *p++ = L' ';

        for (j = 0 ; j < bpr ; ++j) {
            WCHAR c = lpBuffer[i + j];
            if ((c < L' ') || (c >= 127))
                c = L'.';

            *p++ = c;
        }

        for ( ; j < BPR ; ++j) {
            *p++ = L' ';
        }

        *p++ = L'\0';

        RETAILMSG(1, (L"%s %s" CRLF, szLineHeader ? szLineHeader : L"", szLine));
    }
}
#endif

static void CleanPort(HANDLE hFile)
{
	char resp[16];
	DWORD dwRead = 1;
    PurgeComm (hFile, PURGE_RXCLEAR|PURGE_TXCLEAR);
	while (ReadFile (hFile, resp, sizeof(resp)-1, &dwRead, NULL) && dwRead)
	{}
}

static void SendCommand (HANDLE hFile, char *command, int sleeptime, bool doClearDtr = false) {
#if defined (DUMPCOMM)
    WCHAR szCommand[1024];
    if (MultiByteToWideChar (CP_ACP, 0, command, -1, szCommand, 1024))
        RETAILMSG (1, (L"Bluetooth modem gateway: writing %s" CRLF, szCommand));
#endif

    DWORD ch = strlen (command);
	char resp[100];
	DWORD dwRead = 0;
    DWORD dwWritFails = 0;
    DWORD dwWrit = 0;

	CleanPort(hFile);

    while (ch > 0 && (dwWritFails < MAX_WRITE_FAILS)) {
        int fRet = WriteFile (hFile, command, ch, &dwWrit, NULL);
        if (! fRet) {
#if defined (DUMPCOMM)
            RETAILMSG(1, (L"Bluetooth modem : WRITE FAIL ERROR %d" CRLF, GetLastError ()));
#endif
            return;
        }

        command += dwWrit;
        ch -= dwWrit;

        if (dwWrit == 0)
            dwWritFails++;
        else
            dwWritFails = 0;

        dwWrit = 0;
    }

	if (doClearDtr)
		EscapeCommFunction (hFile, CLRDTR);

	if (sleeptime)
		Sleep (sleeptime);

    DWORD dwReadFails = 0;
    do {
        int fRet = ReadFile (hFile, resp, sizeof(resp)-1, &dwRead, NULL);
        if (! fRet) {
#if defined (DUMPCOMM)
            RETAILMSG(1, (L"Bluetooth modem : READ FAIL ERROR %d" CRLF, GetLastError ()));
#endif
            break;
        }

        if (dwRead) {
#if defined (DUMPCOMM)
            DumpBuff (L"Bluetooth modem : Send Response >", (unsigned char *)resp, dwRead);
#endif
            dwReadFails = 0;
            resp[dwRead] = '\0';
            if (strchr (resp, '\n') || strchr (resp, '\r'))
                break;
        } else {
#if defined (DUMPCOMM)
            RETAILMSG(1, (L"Bluetooth modem : READ NOTHING" CRLF));
#endif
            ++dwReadFails;
			Sleep(300);
        }
    } while (dwReadFails < MAX_READ_FAILS);

	CleanPort(hFile);
}


static int HangUpModem (HANDLE hModem) {
#if defined (DUMPCOMM)
    RETAILMSG(1, (L"Bluetooth modem gateway: Hanging up" CRLF));
#endif
    DWORD dwModem2 = 0;
    GetCommModemStatus (hModem, &dwModem2);
    if (dwModem2 & MS_RLSD_ON) {    // We're finished with Bluetooth connection, but the other line is still up!
#if defined (DUMPCOMM)
        RETAILMSG(1, (L"Bluetooth modem gateway: RLSD ON on modem - terminating connection..." CRLF));
#endif
		DCB  commDCB;
		DWORD RtsControl, DtrControl, OutxCtsFlow;
		//
		// Turn off hardware flow control while sending commands.
		//
		SetCommMask (hModem, EV_ERR);

		GetCommState( hModem, &commDCB );
		RtsControl = commDCB.fRtsControl;
		DtrControl = commDCB.fDtrControl;
		OutxCtsFlow = commDCB.fOutxCtsFlow;
		commDCB.fRtsControl = RTS_CONTROL_ENABLE;
		commDCB.fDtrControl = DTR_CONTROL_ENABLE;
		commDCB.fOutxCtsFlow = 0;
		SetCommState( hModem, &commDCB );

        Sleep (1500);

		RETAILMSG(1, (L"Bluetooth modem gateway: +++ on modem - terminating connection..." CRLF));
        SendCommand (hModem, "+++", 2000, true);
		EscapeCommFunction (hModem, SETDTR);
        SendCommand (hModem, "ATH\r", 0);

		commDCB.fRtsControl = RtsControl; 
		commDCB.fDtrControl = DtrControl; 
		commDCB.fOutxCtsFlow = OutxCtsFlow;
		SetCommState( hModem, &commDCB );

		SetCommMask (hModem, EV_RXCHAR | EV_CTS | EV_DSR | EV_RLSD | EV_ERR | EV_RING | EV_EVENT1);
        return TRUE;
    }

    return FALSE;
}

bool PutMessageToQueue(BTMODEM_MESSAGE_TYPE type, BTMODEM_MESSAGE_BODY *msgBody)
{
	BTMODEM_MESSAGE msg;
	msg.Type = type;
	if (type == BTMODEM_SERVICE_REQUEST)
	{
		msg.MsgBody.ServiceRequest.Request = msgBody->ServiceRequest.Request;
		msg.MsgBody.ServiceRequest.Addr = msgBody->ServiceRequest.Addr;
	}
	else if (type == BTMODEM_MODEM_STATUS)
	{
		msg.MsgBody.ModemStatus.Status = msgBody->ModemStatus.Status;
	}
	else if (type == BTMODEM_SERVICE_STATUS)
	{
		msg.MsgBody.ServiceStatus.Status = msgBody->ServiceStatus.Status;
	}

	if (ghOutMsgQueue != INVALID_HANDLE_VALUE)
	{
		if (!WriteMsgQueue(ghOutMsgQueue, &msg, sizeof(BTMODEM_MESSAGE), 0, 0))
			return false;
	}

	return true;
}

bool PutMessageToQueue(UINT32 status)
{
	BTMODEM_MESSAGE_BODY msgBody;
	msgBody.ServiceStatus.Status = status;
	return PutMessageToQueue(BTMODEM_SERVICE_STATUS, &msgBody);
}

static DWORD WINAPI ComCopyThread (LPVOID lpVoid) {
    HANDLE hSource = NULL;
    HANDLE hDest   = NULL;

    WCHAR name[MAX_PATH];

    __try {
        void **pah = (HANDLE *)lpVoid;

        hSource = (HANDLE)pah[0];
        hDest   = (HANDLE)pah[1];

        if (wcslen ((WCHAR *)pah[2]) >= _MAX_PATH) {
            RETAILMSG (1, (L"Bluetooth modem gateway: invalid parameters in reader thread" CRLF));
            SetEvent (ghServiceExitEvent);
            return 0;
        }

        wcsncpy (name, (WCHAR *)pah[2], MAX_PATH);
        name[MAX_PATH-1]='\0';
        
    } __except (1) {
        RETAILMSG (1, (L"Bluetooth Modem Gateway: Exception in reader thread" CRLF));
        SetEvent (ghServiceExitEvent);
        return 0;
    }

    int fBluetoothThread = (wcscmp (name, BTH_BLUETOOTH_READER) == 0);
	int	fModemFlow = 1;
	DCB dcb;
	if (fBluetoothThread && GetCommState(hDest, &dcb))
		fModemFlow = dcb.fOutxCtsFlow;

    RETAILMSG (1, (L"Bluetooth Modem Gateway: %s thread up" CRLF, name));

    SetCommMask (hSource, EV_RXCHAR | EV_CTS | EV_DSR | EV_RLSD | EV_ERR | EV_RING | EV_EVENT1);

    for ( ; ; ) {
        DWORD ModemState = 0;

        if (! WaitCommEvent (hSource, &ModemState, NULL))
            break;

#if defined (DUMPCOMM)
        RETAILMSG(1, (L"Bluetooth modem gateway: %s thread woke up, Event flags: 0x%08x" CRLF, name, ModemState));
#endif
		// ghServiceExitEvent is terminate service event
		if (WaitForSingleObject(ghServiceExitEvent, 0) == WAIT_OBJECT_0)
			break;

#if defined (DUMPCOMM)
        if (ModemState & EV_ERR)
            RETAILMSG(1, (L"Bluetooth gateway: %s thread: EV_ERR" CRLF, name));

        if (ModemState & EV_CTS)
            RETAILMSG(1, (L"Bluetooth gateway: %s thread: EV_CTS" CRLF, name));

        if (ModemState & EV_DSR)
            RETAILMSG(1, (L"Bluetooth gateway: %s thread: EV_DSR" CRLF, name));

        if (ModemState & EV_RLSD)
            RETAILMSG(1, (L"Bluetooth gateway: %s thread: EV_RLSD" CRLF, name));

        if (ModemState & EV_RING)
            RETAILMSG(1, (L"Bluetooth gateway: %s thread: EV_RING" CRLF, name));
#endif

		BTMODEM_MESSAGE_BODY body = {0};
		if (ModemState & EV_RLSD)
		{
			if (fBluetoothThread)
			{
				gfAcceptConnection = FALSE;
				DWORD dwModem = 0;
				if (GetCommModemStatus(hSource, &dwModem))
				{
					body.ServiceRequest.Request = dwModem & MS_RLSD_ON;
					body.ServiceRequest.Addr = 0;
					if (ghOutMsgQueue != INVALID_HANDLE_VALUE)
					{
						DWORD dwSizeOut = 0;
						if (!DeviceIoControl(hSource, IOCTL_BLUETOOTH_GET_PEER_DEVICE, NULL, 0, &body.ServiceRequest.Addr, sizeof(BT_ADDR), &dwSizeOut, NULL))
							body.ServiceRequest.Addr = 0;

						if (PutMessageToQueue(BTMODEM_SERVICE_REQUEST, &body))
						{
							if (body.ServiceRequest.Request)
							{
								SetCommMask (hDest, EV_ERR);
								HANDLE ev[] = { ghServiceExitEvent, ghServiceRequestEvent };
								DWORD res = WaitForMultipleObjects(sizeof(ev)/sizeof(ev[0]), ev, 0, INFINITE);
								if (res == WAIT_OBJECT_0)
									break;

								CleanPort(hSource);
								CleanPort(hDest);
								SetCommMask (hDest, EV_RXCHAR | EV_CTS | EV_DSR | EV_RLSD | EV_ERR | EV_RING | EV_EVENT1);
							}
							else
								gfAcceptConnection = TRUE;
						}
					}

					if (!body.ServiceRequest.Request && !body.ServiceRequest.Addr)
						HangUpModem(hDest);
				}
			}
			else
			{
				// RLSD in modem port thread ???
				if (GetCommModemStatus(hSource, (DWORD *)&(body.ModemStatus.Status)))
					PutMessageToQueue(BTMODEM_MODEM_STATUS, &body);
			}
		}

		if (ModemState & (EV_CTS | EV_DSR)) {
            DWORD dwModem = 0;
            GetCommModemStatus (hSource, &dwModem);

#if defined (DUMPCOMM)
            RETAILMSG(1, (L"Bluetooth modem gateway: %s modem status: 0x%08x" CRLF, name, dwModem));
#endif

			if (fBluetoothThread & !fModemFlow)
			{
				if (ModemState & EV_CTS)
				{
					EscapeCommFunction (hDest, (dwModem & MS_CTS_ON) ? SETDTR : CLRDTR);
					continue;
				}

				if (ModemState & EV_DSR)
					EscapeCommFunction (hDest, (dwModem & MS_DSR_ON) ? SETRTS : CLRRTS);
			}
        }

        if (ModemState & EV_RXCHAR) {
            char buffer[BUF_MAX];
            DWORD dwRead = 0;
            if (! ReadFile (hSource, buffer, sizeof(buffer), &dwRead, NULL))
                break;

#if defined (DUMPCOMM)
            DumpBuff (name, (unsigned char *)buffer, dwRead);
#endif

            DWORD dwWrit = 0;
            char *pbuffer = buffer;
            DWORD dwWritFails = 0;
			HANDLE h = hDest;
			if (fBluetoothThread && !gfAcceptConnection)
			{
				pbuffer = "\r\nERROR\r\n";
				dwRead = strlen(pbuffer);
				h = hSource;
				RETAILMSG(1, (L"Bluetooth modem gateway: %s ERROR is sent back" CRLF, name));
			}

            while (dwRead && WriteFile (h, pbuffer, dwRead, &dwWrit, NULL) && (dwWritFails < MAX_WRITE_FAILS)) {
                pbuffer += dwWrit;
                dwRead -= dwWrit;
                if (dwWrit == 0)
                    dwWritFails++;
                else
                    dwWritFails = 0;

                dwWrit = 0;
            }

#if defined (DUMPCOMM)
            if (dwRead != 0)
                RETAILMSG(1, (L"Bluetooth modem gateway: %s Failed to write everything." CRLF, name));
#endif
        }

/*
        if (fBluetoothThread && (ModemState & EV_RLSD)) {
            DWORD dwModem = 0;
            GetCommModemStatus (hSource, &dwModem);
            if (! (dwModem & MS_RLSD_ON)) {
                if (HangUpModem (hDest))
                    continue;
            }
        }
*/
    }
    SetEvent (ghServiceExitEvent);
    RETAILMSG(1, (L"Bluetooth modem gateway: Exiting %s..." CRLF, name));

    return 0;
}

static HANDLE CreateQueue(LPCWSTR lpszName, BOOL readAccess)
{
	MSGQUEUEOPTIONS msgOptions	= {0};
	msgOptions.dwSize		= sizeof(MSGQUEUEOPTIONS);
	msgOptions.dwFlags		= MSGQUEUE_ALLOW_BROKEN;
	msgOptions.cbMaxMessage = sizeof(BTMODEM_MESSAGE);
	msgOptions.bReadAccess	= readAccess;

	return CreateMsgQueue(lpszName, &msgOptions);
}


static DWORD WINAPI MsgQueueThread(LPVOID lpNull)
{
	BTMODEM_MESSAGE msg;
	DWORD dwSize, dwFlags;
	
	ghInMsgQueue = CreateQueue(BTMODEM_INQUEUE_NAME, 1);
	if (ghInMsgQueue == INVALID_HANDLE_VALUE)
	{
		goto error;
	}

	ghOutMsgQueue = CreateQueue(BTMODEM_OUTQUEUE_NAME, 0);
	if (ghOutMsgQueue == INVALID_HANDLE_VALUE)
	{
		goto error;
	}

	HANDLE ev[] = { ghQueueExitEvent, ghInMsgQueue };

	while ((WAIT_OBJECT_0 + 1) == WaitForMultipleObjects(sizeof(ev)/sizeof(ev[0]), ev, 0, INFINITE))
	{
		if (ReadMsgQueue(ghInMsgQueue, &msg, sizeof(BTMODEM_MESSAGE), &dwSize, 0, &dwFlags)) 
		{
			switch (msg.Type)
			{
				case BTMODEM_SERVICE_REQUEST:
				{
					gfAcceptConnection = msg.MsgBody.ServiceRequest.Request;
					SetEvent(ghServiceRequestEvent);
				}
				break;
			}
		}
	}

error:;
	if (ghInMsgQueue != INVALID_HANDLE_VALUE)
	{
		CloseMsgQueue(ghInMsgQueue);
		ghInMsgQueue = INVALID_HANDLE_VALUE;
	}

	if (ghOutMsgQueue != INVALID_HANDLE_VALUE)
	{
		CloseMsgQueue(ghOutMsgQueue);
		ghOutMsgQueue = INVALID_HANDLE_VALUE;
	}

	return 0;
}

void SendModemOnToMux()
{
	MSGQUEUEOPTIONS msgOptions	= {0};
	msgOptions.dwSize		= sizeof(MSGQUEUEOPTIONS);
	msgOptions.dwFlags		= 0;
	msgOptions.cbMaxMessage = sizeof(GSM710MESSAGE);
	msgOptions.bReadAccess	= 0;

	HANDLE h = CreateMsgQueue(GSM710_INQUEUE_NAME, &msgOptions);
	if (h != INVALID_HANDLE_VALUE)
	{
		GSM710MESSAGE msg;
		msg.Type = GSM710MESSAGETYPE_MODEMSTATE;
		msg.Value = 0;
		
		WriteMsgQueue(h, &msg, sizeof(GSM710MESSAGE), 0, 0);
		CloseMsgQueue(h);
	}
}

DWORD WINAPI MuxMsgQueueTh(HANDLE hStartEvent)
{
	//RETAILMSG(1, (L"+MuxMsgQueueTh" CRLF));
	DWORD ret = 0;
	GSM710MESSAGE msg;
	DWORD dwSize, dwFlags;
	MSGQUEUEOPTIONS msgOptions	= {0};
	msgOptions.dwSize		= sizeof(MSGQUEUEOPTIONS);
	msgOptions.dwFlags		= MSGQUEUE_ALLOW_BROKEN;
	msgOptions.cbMaxMessage = sizeof(GSM710MESSAGE);
	msgOptions.bReadAccess	= 1;

	HANDLE hMuxQueue = CreateMsgQueue(GSM710_OUTQUEUE_NAME, &msgOptions);
	
	if (hStartEvent)
		SetEvent(hStartEvent);

	if (hMuxQueue != INVALID_HANDLE_VALUE)
	{
		while ((WAIT_OBJECT_0) == WaitForSingleObject(hMuxQueue, MUX_UP_MAX_TIMEOUT))
		{
			if (ReadMsgQueue(hMuxQueue, &msg, sizeof(GSM710MESSAGE), &dwSize, 0, &dwFlags)) 
			{
				if (msg.Type == GSM710MESSAGETYPE_CONNECTIONSTATE)
				{
					ret = msg.Value ? 1 : 2;
					break;
				}
				else if (msg.Type == GSM710MESSAGETYPE_MODEMSTATE)
					SendModemOnToMux();
				else if (msg.Type == GSM710MESSAGETYPE_CONNECTIONERROR)
					break;
			}
		}

		CloseMsgQueue(hMuxQueue);
	}

	//RETAILMSG(1, (L"-MuxMsgQueueTh" CRLF));
	return ret;
}

static UINT32 PrepareRegistry(BTMODEM_CONTEXT *btContext)
{
	HKEY hKey = NULL;
	INT32 ret = BTMODEM_ERROR_SUCCESS;
	TCHAR szString[MAX_PATH];

	wsprintf(szString, TEXT("%s\\%s"), REGKEY_EXTMODEMS, btContext->ModemName);
	LONG lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szString, 0, 0, &hKey) ;
    if (ERROR_SUCCESS != lStatus)
    {
		ret = BTMODEM_ERROR_INVALID_MODEM_NAME;
        goto error;
    }

    DWORD dwSize = sizeof(szString);
    DWORD dwValType = REG_SZ;
    lStatus = RegQueryValueEx(hKey, TEXT("Port"), NULL, &dwValType, (LPBYTE)&szString, &dwSize);
    if (ERROR_SUCCESS != lStatus)
    {
		ret = BTMODEM_ERROR_INVALID_REGISTRY_VALUE;
        goto error;
    }

	RegCloseKey(hKey);
	hKey = NULL;

	lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_MODEMGW, 0, 0, &hKey) ;
    if (ERROR_SUCCESS != lStatus)
    {
		ret = BTMODEM_ERROR_INVALID_REGISTRY_KEY;
        goto error;
    }

	lStatus = RegSetValueEx(hKey, TEXT("ModemPortName"), 0, dwValType, (LPBYTE)szString, dwSize); 
	if (ERROR_SUCCESS != lStatus)
    {
		ret = BTMODEM_ERROR_REGISTRY_UPDATING_ERROR;
        goto error;
    }

	RegFlushKey(hKey);
error:;
	if (hKey)
        RegCloseKey(hKey) ;

	return ret;
}

BOOL IsMuxModem(LPCWSTR modemName)
{
	HKEY hKey = NULL;
	TCHAR szString[MAX_PATH];

	wsprintf(szString, TEXT("%s\\%s\\MUX Init"), REGKEY_EXTMODEMS, modemName);
	LONG lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szString, 0, 0, &hKey) ;
    if (ERROR_SUCCESS != lStatus)
		return FALSE;

	RegCloseKey(hKey);
	return TRUE;
}

BOOL IsMuxActivated()
{
	HKEY hKey = NULL;
	LONG lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, DRIVER_MUX07_10_REG_KEY1, 0, 0, &hKey) ;
	if (ERROR_SUCCESS == lStatus)
	{	
		DWORD dwPortIndex;
		DWORD dwSize = sizeof(DWORD);
		DWORD dwValType = REG_DWORD;

		lStatus = RegQueryValueEx(hKey, TEXT("Index"), NULL, &dwValType, (LPBYTE)&dwPortIndex, &dwSize);
		RegCloseKey(hKey);
		if (ERROR_SUCCESS == lStatus)
		{			
			TCHAR szCtrlPort[16];
			wsprintf(szCtrlPort, TEXT("COM%d:"), dwPortIndex);
			HANDLE hPort = CreateFile(szCtrlPort, DEVACCESS_BUSNAMESPACE, 0, 0, 0, 0, 0);
			if (hPort != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hPort);
				return TRUE;
			}
		}
	}

	return FALSE;
}

DWORD WINAPI GetRegTh(PVOID param)
{
	HANDLE port = ((void **)param)[0];
	char *command = ((char **)param)[1];
	DWORD ret = 0, i = REGISTRATION_MAX_TIMEOUT, wlen;
	while (WriteFile(port, command, strlen(command), &wlen, NULL) && i > 0)
	{
		Sleep(1000);
		i--;
	}

	SetCommMask(port, 0);
	return 0;
}

UINT32 WaitForModemRegistration(BOOL *isRegistered, char *cmd, char *response)
{
	*isRegistered = FALSE;
	HANDLE hPort = CreateFile(GSM_PORT_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hPort)
		return BTMODEM_ERROR_MODEM_PORT_ERROR;

	DCB dcb = {0};
	dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(hPort, &dcb))
    {
		CloseHandle(hPort);
		return BTMODEM_ERROR_MODEM_PORT_ERROR;
    }

    // Set com port
    dcb.fBinary = TRUE;
    dcb.fParity = 0;
    dcb.fOutxCtsFlow = FALSE; //TRUE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fTXContinueOnXoff = TRUE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    dcb.fErrorChar = FALSE;
    dcb.fNull = FALSE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE; // RTS_CONTROL_HANDSHAKE;
    dcb.fAbortOnError = FALSE;
    dcb.ByteSize	= 8;
    dcb.Parity		= NOPARITY;
    dcb.StopBits	= ONESTOPBIT;

    dcb.BaudRate = CBR_115200;

    if (!SetCommState(hPort, &dcb))
    {
		CloseHandle(hPort);
		return BTMODEM_ERROR_MODEM_PORT_ERROR;
    }

    COMMTIMEOUTS cto;
    memset(&cto, 0, sizeof(COMMTIMEOUTS));
    cto.ReadIntervalTimeout = MAXDWORD;
    cto.WriteTotalTimeoutConstant = 1000;
    if (!SetCommTimeouts(hPort, &cto))
	{
		CloseHandle(hPort);
		return BTMODEM_ERROR_MODEM_PORT_ERROR;
	}

	DWORD dwBytesReceived;
	unsigned long Mask;
	SetCommMask(hPort, EV_RXCHAR);

	void *args[2];

    args[0] = hPort;
    args[1] = cmd;

	HANDLE hThread = CreateThread(NULL, 0, GetRegTh, args, 0, NULL);

	while (WaitCommEvent(hPort, &Mask, 0))
	{
		if (Mask == 0)
			break;
		else
		{
			char buf[32];
			memset(buf, 0, sizeof(buf));
			if (!ReadFile(hPort, buf, sizeof(buf) - 1, &dwBytesReceived, 0))
				break;
			else
			{
				if (strstr(buf, response))
				{
					*isRegistered = TRUE;
					break;
				}
			}
		}
	}

	CloseHandle(hPort);
	CloseHandle(hThread);
	return 0;
}


UINT32 PrepareModem(BTMODEM_CONTEXT *btContext, char *cmd, char *response)
{
	// check mux if it on
	btContext->IsMuxModem = IsMuxModem(btContext->ModemName);
	btContext->MuxWasActive = IsMuxActivated();

	if (btContext->MuxWasActive)
	{
		if (btContext->IsMuxModem)
			return BTMODEM_ERROR_SUCCESS;
		else
			return BTMODEM_ERROR_MODEM_PORT_ERROR;
	}

	if (MIC_GSMGetPowerStatus(&(btContext->ModemWasOn)) == GSM_OK)
	{
		if (!(btContext->ModemWasOn))
		{
			PutMessageToQueue(BTMODEM_TURNING_ON_MODEM);
			if (MIC_GSMPower(1) != GSM_OK)
				return BTMODEM_ERROR_MODEM_ON_FAIL;
		}
	}
	else 
		return BTMODEM_ERROR_GET_MODEM_POWER_FAIL;

	PutMessageToQueue(BTMODEM_MODEM_TURNED_ON);
	PutMessageToQueue(BTMODEM_WAITING_FOR_SIM_REGISTRATION);

	BOOL isRegistered;
	UINT32 ret = WaitForModemRegistration(&isRegistered, cmd, response);
	if (ret != BTMODEM_ERROR_SUCCESS)
		return ret;

	if (!isRegistered)
		return BTMODEM_ERROR_MODEM_REGISTRATION_ERROR;

	PutMessageToQueue(BTMODEM_SIM_REGISTERED);

	if (btContext->IsMuxModem)
	{
		PutMessageToQueue(BTMODEM_MUX_ACTIVATION);

		if (MIC_GSM710Activate(1, 0) != GSM710ERROR_SUCCESS)
			return BTMODEM_ERROR_MUX_ACTIVATION_FAIL;

		HANDLE hThread = CreateThread(NULL, 0, MuxMsgQueueTh, (void *)0, 0, NULL);
		WaitForSingleObject(hThread, INFINITE);
		DWORD ExitCode; 
		if (!GetExitCodeThread(hThread, &ExitCode))
		{
			CloseHandle(hThread);
			return BTMODEM_ERROR_UNKNOWN_ERROR;
		}

		CloseHandle(hThread);
		if (ExitCode != 1)
			return BTMODEM_ERROR_MUX_ACTIVATION_FAIL;

		PutMessageToQueue(BTMODEM_MUX_ACTIVATED);
	}

	return BTMODEM_ERROR_SUCCESS;
}

static DWORD WINAPI ComServiceThread (LPVOID param) {
	BTMODEM_CONTEXT *btContext = (BTMODEM_CONTEXT *)param;
	btContext->LastError = BTMODEM_ERROR_SUCCESS;

#if ! defined (SDK_BUILD)
    int fHaveStack = FALSE;
    for (int i = 0 ; i < 20 ; ++i) {
        HANDLE hBthApiEvent = OpenEvent (EVENT_ALL_ACCESS, FALSE, BTH_NAMEDEVENT_STACK_INITED);
        if (hBthApiEvent) {
            if (WAIT_OBJECT_0 == WaitForSingleObject (hBthApiEvent, BTH_MODEM_TIMEOUT)) {
                fHaveStack = TRUE;
                CloseHandle (hBthApiEvent);
                break;
            }
            CloseHandle (hBthApiEvent);
        }
        Sleep (1000);
    }

    if (! fHaveStack) {
        gfServiceState = SERVICE_STATE_OFF;
		btContext->LastError = BTMODEM_ERROR_NO_BLUETOOTH_STACK_DETECTED;
        RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: no bluetooth stack detected" CRLF));
        return 0;
    }
#endif

    WCHAR   szModemPortName[_MAX_PATH];
    WCHAR   szBthPortName[_MAX_PATH];
	WCHAR   szTemp[64];
	char	atCmd[64];
	char	atResponse[64];

    // Set up pre-configured stuff

    // Modem
    DWORD   dwModemBaud     = BTH_MODEM_BAUD;
    DWORD   dwByteSize      = 8;
    DWORD   dwParity        = NOPARITY;
    DWORD   dwStopBits      = ONESTOPBIT;
    DWORD   dwFlowControl   = BTH_MODEM_HARDWARE;
    int     fUseBaud        = FALSE;
    int     fUseByteSize    = FALSE;
    int     fUseParity      = FALSE;
    int     fUseStopBits    = FALSE;
    int     fUseFlow        = FALSE;

    DCB dcb;
    int fUseDCB = FALSE;

    // Bluetooth
    int fAuthenticate   = TRUE;
    int fEncrypt        = TRUE;
    int channel         = 0xfe;
    int mtu             = BTH_MODEM_MTU;

	PutMessageToQueue(BTMODEM_STARTING_SERVICE);

	btContext->LastError = PrepareRegistry(btContext);
	if (BTMODEM_ERROR_SUCCESS != btContext->LastError)
		return 0;

    // First, read the configuration parameters

    HKEY    hk;

    if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, BTH_MODEM_REGBASE, 0, KEY_READ, &hk)) {
        gfServiceState = SERVICE_STATE_OFF;
		btContext->LastError = BTMODEM_ERROR_INVALID_REGISTRY_KEY;
        RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: no configuration" CRLF));
        return 0;
    }

	// If an error occurs in the following steps, then it's a registry problem.
	btContext->LastError = BTMODEM_ERROR_INVALID_REGISTRY_VALUE;

    DWORD dwSize = sizeof(szModemPortName) - sizeof(WCHAR);
    DWORD dwType = 0;

    szModemPortName[sizeof(szModemPortName)/sizeof(szModemPortName[0])-1] = '\0';

    if ((ERROR_SUCCESS != RegQueryValueEx (hk, L"ModemPortName", NULL, &dwType, (LPBYTE)szModemPortName, &dwSize)) ||
        (dwType != REG_SZ) || (dwSize >= sizeof(szModemPortName))) {
        RegCloseKey (hk);

        RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: modem port name not specified or invalid" CRLF));

        gfServiceState = SERVICE_STATE_OFF;
        return 0;
    }

    dwSize = sizeof (dwModemBaud);
    if (ERROR_SUCCESS == RegQueryValueEx (hk, L"ModemBaud", NULL, &dwType, (LPBYTE)&dwModemBaud, &dwSize)) {
        if ((dwType != REG_DWORD) || (dwSize != sizeof(dwModemBaud))) {
            RegCloseKey (hk);

            RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: modem baud rate invalid" CRLF));

            gfServiceState = SERVICE_STATE_OFF;
            return 0;
        }

        fUseBaud = TRUE;
    }

    dwSize = sizeof (dwByteSize);
    if (ERROR_SUCCESS == RegQueryValueEx (hk, L"ModemByteSize", NULL, &dwType, (LPBYTE)&dwByteSize, &dwSize)) {
        if ((dwType != REG_DWORD) || (dwSize != sizeof(dwByteSize)) || (dwByteSize < 4) || (dwByteSize > 8)) {
            RegCloseKey (hk);

            RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: modem byte size invalid" CRLF));

            gfServiceState = SERVICE_STATE_OFF;
            return 0;
        }

        fUseByteSize = TRUE;
    }

    dwSize = sizeof (dwParity);
    if (ERROR_SUCCESS == RegQueryValueEx (hk, L"ModemParity", NULL, &dwType, (LPBYTE)&dwParity, &dwSize)) {
        if ((dwType != REG_DWORD) || (dwSize != sizeof(dwParity)) || (dwParity > SPACEPARITY)) {
            RegCloseKey (hk);

            RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: modem parity invalid" CRLF));

            gfServiceState = SERVICE_STATE_OFF;
            return 0;
        }

        fUseParity = TRUE;
    }

    dwSize = sizeof (dwStopBits);
    if (ERROR_SUCCESS == RegQueryValueEx (hk, L"ModemStopBits", NULL, &dwType, (LPBYTE)&dwStopBits, &dwSize)) {
        if ((dwType != REG_DWORD) || (dwSize != sizeof(dwStopBits)) || (dwStopBits > TWOSTOPBITS)) {
            RegCloseKey (hk);

            RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: modem stop bits invalid" CRLF));

            gfServiceState = SERVICE_STATE_OFF;
            return 0;
        }

        fUseStopBits = TRUE;
    }

    dwSize = sizeof (dwFlowControl);
    if (ERROR_SUCCESS == RegQueryValueEx (hk, L"ModemFlowControl", NULL, &dwType, (LPBYTE)&dwFlowControl, &dwSize)) {
        if ((dwType != REG_DWORD) || (dwSize != sizeof(dwFlowControl)) || (dwFlowControl > BTH_MODEM_NONE)) {
            RegCloseKey (hk);

            RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: modem flow control invalid" CRLF));

            gfServiceState = SERVICE_STATE_OFF;
            return 0;
        }

        fUseFlow = TRUE;
    }

    DCB dcb2;
    dwSize = sizeof (dcb2);
    if (ERROR_SUCCESS == RegQueryValueEx (hk, L"ModemDCB", NULL, &dwType, (LPBYTE)&dcb2, &dwSize)) {
        if ((dwType != REG_BINARY) || (dwSize != sizeof(dcb2)) || (dcb2.DCBlength != sizeof(dcb2))) {
            RegCloseKey (hk);

            gfServiceState = SERVICE_STATE_OFF;

            RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: modem DCB invalid" CRLF));
            return 0;
        }

        memcpy (&dcb, &dcb2, sizeof(dcb));
        fUseDCB = TRUE;
    }

    dwSize = sizeof(szBthPortName) - sizeof(WCHAR);
    dwType = 0;
    szBthPortName[sizeof(szBthPortName)/sizeof(szBthPortName[0])-1] = '\0';

    if ((ERROR_SUCCESS != RegQueryValueEx (hk, L"BthPortName", NULL, &dwType, (LPBYTE)szBthPortName, &dwSize)) ||
        (dwType != REG_SZ) || (dwSize >= sizeof(szBthPortName))) {
        RegCloseKey (hk);

        gfServiceState = SERVICE_STATE_OFF;

        RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: Bluetooth port name not specified or invalid" CRLF));

        return 0;
    }

    dwSize = sizeof (mtu);
    if (ERROR_SUCCESS == RegQueryValueEx (hk, L"MTU", NULL, &dwType, (LPBYTE)&mtu, &dwSize)) {
        if ((dwType != REG_DWORD) || (dwSize != sizeof(mtu))) {
            RegCloseKey (hk);

            RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: MTU invalid" CRLF));

            gfServiceState = SERVICE_STATE_OFF;
            return 0;
        }
    }

    dwSize = sizeof (channel);
    if (ERROR_SUCCESS == RegQueryValueEx (hk, L"channel", NULL, &dwType, (LPBYTE)&channel, &dwSize)) {
        if ((dwType != REG_DWORD) || (dwSize != sizeof(channel)) || (channel < 1) || ((channel > 31) && (channel != 0xfe))) {
            RegCloseKey (hk);

            RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: channel invalid" CRLF));

            gfServiceState = SERVICE_STATE_OFF;
            return 0;
        }
    }

    dwSize = sizeof (fAuthenticate);
    if (ERROR_SUCCESS == RegQueryValueEx (hk, L"Authenticate", NULL, &dwType, (LPBYTE)&fAuthenticate, &dwSize)) {
        if ((dwType != REG_DWORD) || (dwSize != sizeof(fAuthenticate))) {
            RegCloseKey (hk);

            RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: authentication flag invalid" CRLF));

            gfServiceState = SERVICE_STATE_OFF;
            return 0;
        }
    }

    dwSize = sizeof (fEncrypt);
    if (ERROR_SUCCESS == RegQueryValueEx (hk, L"Encrypt", NULL, &dwType, (LPBYTE)&fEncrypt, &dwSize)) {
        if ((dwType != REG_DWORD) || (dwSize != sizeof(fEncrypt))) {
            RegCloseKey (hk);

            RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: encryption flag invalid" CRLF));

            gfServiceState = SERVICE_STATE_OFF;
            return 0;
        }
    }

	dwSize = sizeof(szTemp) - sizeof(WCHAR);
    dwType = 0;
    szTemp[sizeof(szTemp)/sizeof(szTemp[0])-1] = '\0';

    if ((ERROR_SUCCESS != RegQueryValueEx (hk, L"ModemRegCmd", NULL, &dwType, (LPBYTE)szTemp, &dwSize)) ||
        (dwType != REG_SZ) || (dwSize >= sizeof(szTemp))) {
        RegCloseKey (hk);

        gfServiceState = SERVICE_STATE_OFF;

        RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: Modem registration AT command not specified or invalid" CRLF));

        return 0;
    }
	ExpandATMacro(szTemp, atCmd);

	dwSize = sizeof(szTemp) - sizeof(WCHAR);
    dwType = 0;
    szTemp[sizeof(szTemp)/sizeof(szTemp[0])-1] = '\0';

    if ((ERROR_SUCCESS != RegQueryValueEx (hk, L"ModemRegOkResp", NULL, &dwType, (LPBYTE)szTemp, &dwSize)) ||
        (dwType != REG_SZ) || (dwSize >= sizeof(szTemp))) {
        RegCloseKey (hk);

        gfServiceState = SERVICE_STATE_OFF;

        RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: Modem registration response not specified or invalid" CRLF));

        return 0;
    }
	ExpandATMacro(szTemp, atResponse);

    RegCloseKey (hk);

    int port = szBthPortName[3] - '0';

	btContext->LastError = PrepareModem(btContext, atCmd, atResponse);
	if (BTMODEM_ERROR_SUCCESS != btContext->LastError)
	{
		gfServiceState = SERVICE_STATE_OFF;
		return 0;
	}

    // Open and configure the modem port

    HANDLE  hModemComPort = CreateFile(szModemPortName,
                GENERIC_READ | GENERIC_WRITE,
                0,    // comm devices must be opened w/exclusive-access
                NULL, // no security attrs
                OPEN_EXISTING, // comm devices must use OPEN_EXISTING
                FILE_ATTRIBUTE_NORMAL,    // overlapped I/O 
                NULL  // hTemplate must be NULL for comm devices  
                );

    if (hModemComPort == INVALID_HANDLE_VALUE) {
        gfServiceState = SERVICE_STATE_OFF;

        RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: could not open modem port (error %d)" CRLF, GetLastError ()));
		btContext->LastError = BTMODEM_ERROR_MODEM_PORT_ERROR;
        return 0;
    }

	/*
	#define     IOCTL_SET_MODEM_POWER_STATE            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0300,METHOD_BUFFERED,FILE_ANY_ACCESS)	
	INT32 power = 1, error;
	BOOL ret = DeviceIoControl(hModemComPort, IOCTL_SET_MODEM_POWER_STATE, &power,
	        sizeof(power), &error, sizeof(DWORD), NULL, NULL);
	if (!ret)
		RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: could not turn modem on (IOCtrl error %d)" CRLF, GetLastError ()));
	else
		RETAILMSG(1, (L"Bluetooth modem gateway: Modem on result = %d)" CRLF, error));
	*/

    if (! fUseDCB) {
        if (! GetCommState (hModemComPort, &dcb)) {
            RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: could setup modem port (error %d)" CRLF, GetLastError ()));
			btContext->LastError = BTMODEM_ERROR_MODEM_PORT_ERROR;
            CloseHandle (hModemComPort);

            gfServiceState = SERVICE_STATE_OFF;
            return 0;
        }
    }

    if (fUseBaud)
        dcb.BaudRate = dwModemBaud;

    if (fUseByteSize)
        dcb.ByteSize = (unsigned char)dwByteSize;

    if (fUseStopBits)
        dcb.StopBits = (unsigned char)dwStopBits;

    if (fUseParity) {
        dcb.fParity = (dwParity != NOPARITY);
        dcb.Parity = (unsigned char)dwParity;
    }

    if (fUseFlow) {
        dcb.fDsrSensitivity = FALSE;
        dcb.fDtrControl     = DTR_CONTROL_ENABLE;

        if (dwFlowControl == BTH_MODEM_HARDWARE) {
            // Enable RTS/CTS Flow Control
            dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
            dcb.fOutxCtsFlow = 1;
            dcb.fOutX = 0;
            dcb.fInX = 0;
        } else if (dwFlowControl == BTH_MODEM_SOFTWARE) {
            // Enable XON/XOFF Flow Control
            dcb.fRtsControl = RTS_CONTROL_ENABLE;
            dcb.fOutxCtsFlow = 0;
            dcb.fOutX = 1;
            dcb.fInX  = 1;  
        } else {
            dcb.fRtsControl = RTS_CONTROL_ENABLE;
            dcb.fOutxCtsFlow = 0;
            dcb.fOutX = 0;
            dcb.fInX  = 0;
        }
    }

    if (! SetupComm (hModemComPort, 20000, 20000)) {
        // Ignore this failure
        RETAILMSG(1, (L"Bluetooth modem gateway: Could setup modem port.  Ignoring failure... (error %d)" CRLF, GetLastError ()));
    } 

    // purge any information in the buffer

    if ( ! PurgeComm (hModemComPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR )) {
        RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: could not purge modem port (error %d)" CRLF, GetLastError ()));
		btContext->LastError = BTMODEM_ERROR_MODEM_PORT_ERROR;
        CloseHandle (hModemComPort);

        gfServiceState = SERVICE_STATE_OFF;
        return 0;
    }

    COMMTIMEOUTS commTimeouts;
    commTimeouts.ReadTotalTimeoutMultiplier = 1;
    commTimeouts.ReadIntervalTimeout = 50;
    commTimeouts.ReadTotalTimeoutConstant = 50;
    commTimeouts.WriteTotalTimeoutMultiplier = 5;
    commTimeouts.WriteTotalTimeoutConstant = 500;

    if (! SetCommTimeouts (hModemComPort, &commTimeouts)) {
        RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: could not configure timeouts on modem port (error %d)" CRLF, GetLastError ()));
		btContext->LastError = BTMODEM_ERROR_MODEM_PORT_ERROR;
        CloseHandle (hModemComPort);

        gfServiceState = SERVICE_STATE_OFF;
        return 0;
    }

    if (! SetCommState(hModemComPort, &dcb)) {
        RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: could not configure state of modem port (error %d)" CRLF, GetLastError ()));
		btContext->LastError = BTMODEM_ERROR_MODEM_PORT_ERROR;
        CloseHandle (hModemComPort);

        gfServiceState = SERVICE_STATE_OFF;
        return 0;
    }

    // Create and open Bluetooth device
	port = FindFreeBluetoothPort();
	if (port < 0)
	{
		CloseHandle (hModemComPort);
		btContext->LastError = BTMODEM_ERROR_BLUETOOTH_PORT_NOT_FOUND;
        gfServiceState = SERVICE_STATE_OFF;
        return 0;
	}

    HANDLE hBthDevice = CreatePort (port, channel, mtu, fAuthenticate, fEncrypt);
    if (! hBthDevice) {
        CloseHandle (hModemComPort);
		btContext->LastError = BTMODEM_ERROR_BLUETOOTH_PORT_ERROR;
        gfServiceState = SERVICE_STATE_OFF;
        return 0;
    }

	StringCchPrintf (STRING_AND_COUNTOF(szBthPortName), L"BSP%d:", port);
    HANDLE hBthComPort = CreateFile (szBthPortName,
                GENERIC_READ | GENERIC_WRITE,
                0,    // comm devices must be opened w/exclusive-access
                NULL, // no security attrs
                OPEN_EXISTING, // comm devices must use OPEN_EXISTING
                FILE_ATTRIBUTE_NORMAL,    // overlapped I/O 
                NULL  // hTemplate must be NULL for comm devices  
                );

    if (hBthComPort == INVALID_HANDLE_VALUE) {
        RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: could not open Bluetooth device port %s (error %d)" CRLF, szBthPortName, GetLastError ()));
		btContext->LastError = BTMODEM_ERROR_BLUETOOTH_PORT_ERROR;
        DeactivateDevice (hBthDevice);
        CloseHandle (hModemComPort);

        gfServiceState = SERVICE_STATE_OFF;
        return 0;
    }

    commTimeouts.ReadTotalTimeoutMultiplier = 1;
    commTimeouts.ReadIntervalTimeout = 50;
    commTimeouts.ReadTotalTimeoutConstant = 50;
    commTimeouts.WriteTotalTimeoutMultiplier = 5;
    commTimeouts.WriteTotalTimeoutConstant = 500;

    if (! SetCommTimeouts (hBthComPort, &commTimeouts)) {
        RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: could not configure timeouts on bluetooth port (error %d)" CRLF, GetLastError ()));
		btContext->LastError = BTMODEM_ERROR_BLUETOOTH_PORT_ERROR;
        CloseHandle (hBthComPort);
        DeactivateDevice (hBthDevice);
        CloseHandle (hModemComPort);

        gfServiceState = SERVICE_STATE_OFF;
        return 0;
    }

    unsigned long ulSdpRecord = RegisterSDP (hBthComPort);

    if (! ulSdpRecord) {
        RETAILMSG(1, (L"Bluetooth modem gateway: Initialization failed: could not register SDP" CRLF));
		btContext->LastError = BTMODEM_ERROR_BLUETOOTH_PORT_ERROR;
        CloseHandle (hBthComPort);
        DeactivateDevice (hBthDevice);
        CloseHandle (hModemComPort);

        gfServiceState = SERVICE_STATE_OFF;
        return 0;
    }

    // Go on listening...

    void *ahA[3];
    void *ahB[3];

    ahA[0] = hBthComPort;
    ahA[1] = hModemComPort;
    ahA[2] = BTH_BLUETOOTH_READER;

    ahB[0] = hModemComPort;
    ahB[1] = hBthComPort;
    ahB[2] = BTH_MODEM_READER;

    HANDLE hThreadA = CreateThread (NULL, 0, ComCopyThread, ahA, 0, NULL);
    HANDLE hThreadB = CreateThread (NULL, 0, ComCopyThread, ahB, 0, NULL);

    if (hThreadA && hThreadB) {
        gfServiceState = SERVICE_STATE_ON;
		PutMessageToQueue(BTMODEM_SERVICE_STARTED);
        BT_ADDR bt;
        if (BthReadLocalAddr (&bt) == ERROR_SUCCESS)
            RETAILMSG(1, (L"Bluetooth modem gateway: Initialization complete. Local address %04x%08x" CRLF, GET_NAP(bt), GET_SAP(bt)));
        else
            RETAILMSG(1, (L"Bluetooth modem gateway: Initialization complete. No bluetooth adapter detected." CRLF));

        WaitForSingleObject (ghServiceExitEvent, INFINITE);
    } else
        RETAILMSG(1, (L"Bluetooth modem gateway: could not create worker threads." CRLF));

    gfServiceState = SERVICE_STATE_SHUTTING_DOWN;
	PutMessageToQueue(BTMODEM_STOPPING_SERVICE);

    RETAILMSG(1, (L"Bluetooth modem gateway: closing down." CRLF));
    SetEvent (ghServiceExitEvent);
    CloseHandle (hBthComPort);
	SetCommMask(hModemComPort, 0);
    HangUpModem (hModemComPort);
    CloseHandle (hModemComPort);

    DeRegisterSDP (ulSdpRecord);
    DeactivateDevice (hBthDevice);

    if (hThreadA) {
        WaitForSingleObject (hThreadA, INFINITE);        
        CloseHandle (hThreadA);
    }
    if (hThreadB) {
        WaitForSingleObject (hThreadB, INFINITE);        
        CloseHandle (hThreadB);
    }

	if (btContext->IsMuxModem && !btContext->MuxWasActive)
	{
		PutMessageToQueue(BTMODEM_MUX_DEACTIVATION);

		HANDLE hStartEvent = CreateEvent(NULL, 0, 0, NULL);
		if (hStartEvent)
		{
			HANDLE hThread = CreateThread(NULL, 0, MuxMsgQueueTh, (void *)hStartEvent, 0, NULL);
			if (hThread)
			{
				WaitForSingleObject(hStartEvent, INFINITE);
				if (MIC_GSM710Activate(0, 0) == GSM710ERROR_SUCCESS)
				{
					WaitForSingleObject(hThread, INFINITE);	
					DWORD ExitCode;
					GetExitCodeThread(hThread, &ExitCode);
					if (ExitCode != 2)
						btContext->LastError = BTMODEM_ERROR_MUX_DEACTIVATION_FAIL;
				}
				else
					btContext->LastError = BTMODEM_ERROR_MUX_DEACTIVATION_FAIL;

				CloseHandle(hThread);
			}

			CloseHandle(hStartEvent);
		}

		if (btContext->LastError == BTMODEM_ERROR_SUCCESS)
			PutMessageToQueue(BTMODEM_MUX_DEACTIVATED);
	}

	if (!btContext->ModemWasOn)
	{
		PutMessageToQueue(BTMODEM_TURNING_OFF_MODEM);

		if (MIC_GSMPower(0) == GSM_OK)
			PutMessageToQueue(BTMODEM_MODEM_TURNED_OFF);
		else
			btContext->LastError = BTMODEM_ERROR_MODEM_OFF_FAIL;
	}

    gfServiceState = SERVICE_STATE_OFF;
	PutMessageToQueue(BTMODEM_SERVICE_STOPPED);
    RETAILMSG(1, (L"Bluetooth modem gateway: down." CRLF));

    return 0;
}

extern "C" DWORD BTM_Init (DWORD dwData)
{
	BTMODEM_CONTEXT *btContext = new BTMODEM_CONTEXT;
	btContext->IsOpened = FALSE;
    return (DWORD)btContext;
}

extern "C" BOOL BTM_Deinit(DWORD dwData)
{
	if (dwData)
        delete (BTMODEM_CONTEXT *)dwData ;

	return TRUE;
}

extern "C" DWORD BTM_Open (DWORD dwData, DWORD dwAccess, DWORD dwShareMode)
{
	BTMODEM_CONTEXT *btContext = (BTMODEM_CONTEXT *)dwData;
	if (btContext->IsOpened)
		return 0;

    ghServiceExitEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
    if (! ghServiceExitEvent)
        return 0;

	ghServiceRequestEvent = CreateEvent (NULL, 0, 0, NULL);
	if (!ghServiceRequestEvent)
        return 0;

	ghQueueExitEvent = CreateEvent (NULL, 0, 0, NULL);
	if (!ghQueueExitEvent)
        return 0;

	btContext->IsOpened = TRUE;
    return dwData;
}

extern "C" BOOL BTM_Close (DWORD dwData) 
{	
	BTMODEM_CONTEXT *btContext = (BTMODEM_CONTEXT *)dwData;

	if (ghServiceExitEvent) {
        SetEvent (ghServiceExitEvent);
        CloseHandle (ghServiceExitEvent);
        ghServiceExitEvent = NULL;
    }

	if (ghServiceRequestEvent)
		CloseHandle(ghServiceRequestEvent);

    if (ghServiceThread) {
        WaitForSingleObject (ghServiceThread, INFINITE);
        CloseHandle (ghServiceThread);
        ghServiceThread = NULL;
    }

	if (ghQueueExitEvent)
	{
		SetEvent(ghQueueExitEvent);
		CloseHandle(ghQueueExitEvent);
		ghQueueExitEvent = NULL;
	}

	if (ghMsgQueueThread)
	{
		WaitForSingleObject(ghMsgQueueThread, INFINITE);        
        CloseHandle(ghMsgQueueThread);
		ghMsgQueueThread = NULL;
	}

	btContext->IsOpened = FALSE;
    return TRUE;
}

extern "C" DWORD BTM_Write (DWORD dwData, LPCVOID pInBuf, DWORD dwInLen)
{
    return -1;
}

extern "C" DWORD BTM_Read (DWORD dwData, LPVOID pBuf, DWORD dwLen)
{
    return -1;
}

extern "C" DWORD BTM_Seek (DWORD dwData, long pos, DWORD type)
{
    return (DWORD)-1;
}

extern "C" void BTM_PowerUp(void)
{
    return;
}

extern "C" void BTM_PowerDown(void)
{
    return;
}
 
extern "C" BOOL BTM_IOControl(DWORD dwData, DWORD dwCode, PBYTE pBufIn,
              DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
              PDWORD pdwActualOut)
{
	BTMODEM_CONTEXT *btContext = (BTMODEM_CONTEXT *)dwData;

    switch (dwCode) {
        // Control code sent to start a service (not the same as IOCTL_SERVICE_STARTED).
        case IOCTL_SERVICE_START: // start a service that is currently in the stopped stage.
			return TRUE;

                // stop a service and for refresh bring it up again.
                // REFRESH is required for more complicated servers
                // that only read in their registry configuration
                // during initilization.

        case IOCTL_SERVICE_REFRESH:
        case IOCTL_SERVICE_STOP:
			return TRUE;


        case IOCTL_SERVICE_STATUS:
            if (pBufOut && dwLenOut == sizeof(DWORD))  {
                __try {
                    *(DWORD *)pBufOut = gfServiceState;
                    if (pdwActualOut)
                        *pdwActualOut = sizeof(DWORD);
                } __except (1) {
                    SetLastError (ERROR_INVALID_USER_BUFFER);
                    return FALSE;
                }
                return TRUE;
            }
            break;

                // IOCTL_SERVICE_CONTROL contains service specific information passed to it by
                // a calling application.
        case IOCTL_SERVICE_CONTROL:
			if (dwLenIn == sizeof(DWORD) && pBufIn != NULL)
			{
				switch (*pBufIn)
				{
					case BTMCOMMAND_GET_EXTENDED_ERROR:
						if ((dwLenOut < sizeof(btContext->LastError)) || (NULL == pBufOut))
						{
							SetLastError (ERROR_INVALID_PARAMETER);
							return FALSE;
						}

						return CeSafeCopyMemory(pBufOut, &btContext->LastError, sizeof(btContext->LastError) );

					case BTMCOMMAND_ACTIVATE:
						{
							if (gfServiceState == SERVICE_STATE_OFF) 
							{
								if (ghServiceThread) 
								{
									if (WAIT_TIMEOUT == WaitForSingleObject (ghServiceThread, BTH_MODEM_TIMEOUT))
									{
										btContext->LastError = BTMODEM_ERROR_TIMEOUT_ERROR;
										return FALSE;
									}

									CloseHandle (ghServiceThread);
									ghServiceThread = NULL;
								}

								ResetEvent (ghServiceExitEvent);
								gfServiceState = SERVICE_STATE_STARTING_UP;
								ghServiceThread = CreateThread (NULL, 0, ComServiceThread, (void *)btContext, 0, NULL);
								if (!ghServiceThread)
								{
									btContext->LastError = BTMODEM_ERROR_UNKNOWN_ERROR;
									return FALSE;
								}

								return TRUE;
							}

							btContext->LastError = BTMODEM_ERROR_ALREADY_ACTIVATED;
							return FALSE;
						}
						break;

					case BTMCOMMAND_DEACTIVATE:
						{
							if (gfServiceState == SERVICE_STATE_ON) 
							{
								if (ghServiceThread) 
								{
									SetEvent (ghServiceExitEvent);
									if (WAIT_TIMEOUT == WaitForSingleObject (ghServiceThread, BTH_MODEM_TIMEOUT))
									{
										btContext->LastError = BTMODEM_ERROR_TIMEOUT_ERROR;
										return FALSE;
									}

									CloseHandle (ghServiceThread);
									ghServiceThread = NULL;
								}

								return TRUE;
							}

							btContext->LastError = BTMODEM_ERROR_NOT_ACTIVATED;
							return FALSE;
						}
						break;

					case BTMCOMMAND_CREATE_MSG_QUEUE:
						{
							if (ghMsgQueueThread == NULL)
							{
								ghMsgQueueThread = CreateThread(NULL, 0, MsgQueueThread, (void *)0, 0, NULL);
								if (ghMsgQueueThread == NULL)
									return FALSE;
							}
							else
							{
								SetLastError(ERROR_ALREADY_REGISTERED);
								return FALSE;
							}
						}
						break;

					case BTMCOMMAND_GET_CONTEXT:
						{
							if ((dwLenOut == sizeof(BTMODEM_CONTEXT)) && pBufOut)
								return CeSafeCopyMemory(pBufOut, btContext, dwLenOut);

							return FALSE;
						}

					case BTMCOMMAND_SET_CONTEXT:
						{
							if ((dwLenOut == sizeof(BTMODEM_CONTEXT)) && pBufOut)
								return CeSafeCopyMemory(btContext, pBufOut, dwLenOut);

							return FALSE;
						}

					default:
						return FALSE;
				}

				return TRUE;
			}
			else
				return FALSE;

                // Called once all initialization has completed and the session is ready
                // to start running.  By convention all services will (or at least should) be
                // passed their service handle (what RegisterService() returns).  This value
                // can be saved in case a service wishes to call DeregisterService on itself.
                //
                // This is not available in device.exe!
                //
        case IOCTL_SERVICE_STARTED:
            return TRUE;

        case IOCTL_SERVICE_REGISTER_SOCKADDR:
                // The super services thread or an application will call this IOCTL will
                // pBufIn = NULL as a means of querying whether or not the particular service
                // will support having a socket bound to it or not.  Service will return FALSE
                // if it was not designed to take IOCTL_SERVICE_CONNECTION commands, or
                // if it is not ready.
                //
                // This is not available in device.exe!
                //
            return FALSE;

        case IOCTL_SERVICE_DEREGISTER_SOCKADDR:
                // When our sockets are being disconnected this message is sent to us.
                //
                // This is not available in device.exe!
                //
            return TRUE;

        case IOCTL_SERVICE_CONNECTION:
                // This IOControl is called when a socket connection for a socked associated
                // with the service comes off the wire.  The socket accept() returns is the
                // value passed.  Note that it is the service's responsibilty to close
                // the socket, even if the service is not in the active state.
                //
                // This is not available in device.exe!
                //
            return FALSE;

                // This can be used to programmatically turn on/off debug zones.
        case IOCTL_SERVICE_DEBUG:
            return TRUE;

//      Unsupported SERVICES IOCTLs
//      case IOCTL_SERVICE_INSTALL:
//      case IOCTL_SERVICE_UNINSTALL:
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}
