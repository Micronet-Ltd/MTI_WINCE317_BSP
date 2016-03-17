//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft shared
// source or premium shared source license agreement under which you licensed
// this source code. If you did not accept the terms of the license agreement,
// you are not authorized to use this source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the SOURCE.RTF on your install media or the root of your tools installation.
// THE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//

#include "btagpriv.h"
#include "btagnetwork.h"


#ifdef DEBUG

DBGPARAM dpCurSettings = 
{
    TEXT("BTAGSVC"), 
 {
  TEXT("Output"),
  TEXT("Misc"),
  TEXT("Service"),
  TEXT("Parser"),
  TEXT("Handler"),  
  TEXT("Network"),
  TEXT("PhoneUI"),
  TEXT("Bond"),
  TEXT("Undefined"),
  TEXT("Undefined"),  
  TEXT("Undefined"),
  TEXT("Undefined"),
  TEXT("Undefined"),  
  TEXT("Undefined"),  
  TEXT("Warning"),
  TEXT("Error") 
 },
    0x0000C001
};

#endif


#define MAX_SDP_RECORD_SIZE 0x048


#define SDP_HS_RECORD_SIZE     0x0000003d
BYTE rgbSdpRecordHeadsetHS[] = {
        0x35, 0x3b, 0x09, 0x00, 0x01, 0x35, 0x06, 0x19,
        0x11, 0x12, 0x19, 0x12, 0x03, 0x09, 0x00, 0x04,
        0x35, 0x0c, 0x35, 0x03, 0x19, 0x01, 0x00, 0x35,
        0x05, 0x19, 0x00, 0x03, 0x08, 0x0a, 0x09, 0x00,
        0x09, 0x35, 0x08, 0x35, 0x06, 0x19, 0x11, 0x08,
        0x09, 0x01, 0x00, 0x09, 0x01, 0x00, 0x25, 0x0d,
        0x56, 0x6f, 0x69, 0x63, 0x65, 0x20, 0x47, 0x61,
        0x74, 0x65, 0x77, 0x61, 0x79
};

#define SDP_HS_CHANNEL_OFFSET  29

#define SDP_HF_RECORD_SIZE     0x00000048
BYTE rgbSdpRecordHeadsetHF[] = {
        0x35, 0x46, 0x09, 0x00, 0x01, 0x35, 0x06, 0x19,
        0x11, 0x1f, 0x19, 0x12, 0x03, 0x09, 0x00, 0x04,
        0x35, 0x0c, 0x35, 0x03, 0x19, 0x01, 0x00, 0x35,
        0x05, 0x19, 0x00, 0x03, 0x08, 0x0a, 0x09, 0x00,
        0x09, 0x35, 0x08, 0x35, 0x06, 0x19, 0x11, 0x1e,
        0x09, 0x01, 0x01, 0x09, 0x01, 0x00, 0x25, 0x0d,
        0x56, 0x6f, 0x69, 0x63, 0x65, 0x20, 0x47, 0x61,
        0x74, 0x65, 0x77, 0x61, 0x79, 0x09, 0x03, 0x01,
        0x08, 0x01, 0x09, 0x03, 0x11, 0x09, 0x00, 0x00
};

#define SDP_HF_CHANNEL_OFFSET       29
#define SDP_HF_CAPABILITY_OFFSET    71


CAGService::CAGService()
{
    m_hThread = NULL;
    m_sockServer[0] = INVALID_SOCKET;
    m_sockServer[1] = INVALID_SOCKET;
    m_fShutdown = FALSE;
    m_SDPRecordHS = 0;
    m_SDPRecordHF = 0;
}

// Test onlu
//volatile DWORD gAgTestVarCnt = 0;


// This method initializes the AG
DWORD CAGService::Init(void)
{
    DWORD dwRetVal = ERROR_SUCCESS;
    WSADATA wsd;

    DEBUGMSG(ZONE_OUTPUT, (L"BTAGSVC: Initializing Audio Gateway Service.\n"));
	RETAILMSG(1, (L"BTAGSVC:+%S\r\n",__FUNCTION__));

    g_pAGEngine = &m_AGEngine;

    svsutil_Initialize();
    
    dwRetVal = WSAStartup(MAKEWORD(1,0), &wsd);

	RETAILMSG(1, (L"BTAGSVC:-%S, ret = 0x%X\r\n",__FUNCTION__, dwRetVal));
    return dwRetVal;
}


// This method deinitializes the AG
void CAGService::Deinit(void)
{
    DEBUGMSG(ZONE_OUTPUT, (L"BTAGSVC: Deinitializing Audio Gateway Service.\n"));

    WSACleanup();
    svsutil_DeInitialize();
    
    g_pAGEngine = NULL;
}


// This method starts the AG
DWORD CAGService::Start(void)
{
    DWORD dwRetVal = ERROR_SUCCESS;
    AG_PROPS    AGProps;

    DEBUGMSG(ZONE_OUTPUT, (L"BTAGSVC: Starting Audio Gateway Service.\n"));
	RETAILMSG(1, (L"BTAGSVC:+%S\r\n",__FUNCTION__));

    Lock();

    m_fShutdown = FALSE;

    (void) m_ATParser.Init();

    dwRetVal = m_AGEngine.Init(&m_ATParser);
    if (ERROR_SUCCESS != dwRetVal) {
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error initializing AG engine: %d.\n", dwRetVal));
		RETAILMSG(1, (L"BTAGSVC:%S, Error initializing AG engine: %d.\r\n",__FUNCTION__, dwRetVal));
        goto exit;
    }

    (void) LoadAGState();

    m_hCloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (! m_hCloseEvent) {
        dwRetVal = GetLastError();
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error creating close event: %d.\n", dwRetVal));
		RETAILMSG(1, (L"BTAGSVC:%S, Error creating close event: %d.\r\n",__FUNCTION__, dwRetVal));
        goto exit;
    }

    m_hThread = CreateThread(NULL, 0, ListenThread, this, 0, NULL);
    if (! m_hThread) {
        dwRetVal = GetLastError();
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error creating ListenThread: %d.\n", dwRetVal));
		RETAILMSG(1, (L"BTAGSVC:%S, Error creating ListenThread: %d.\r\n",__FUNCTION__, dwRetVal));
        goto exit;
    }

    m_AGEngine.GetAGProps(&AGProps);

    m_hScoThread = CreateThread(NULL, 0, SCOListenThread, this, 0, NULL);
    if (! m_hScoThread) {
        dwRetVal = GetLastError();
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error creating SCOListenThread: %d.\n", dwRetVal));
		RETAILMSG(1, (L"BTAGSVC:%S, Error creating SCOListenThread: %d.\r\n",__FUNCTION__, dwRetVal));
        goto exit;
    }

    DEBUGMSG(ZONE_OUTPUT, (L"BTAGSVC: Audio Gateway Service started successfully.\n"));
	RETAILMSG(1, (L"BTAGSVC:%S, Audio Gateway Service started successfully.\r\n",__FUNCTION__));

exit:
    Unlock();
    
    if (ERROR_SUCCESS != dwRetVal) {
        Stop();
    }
    
    return dwRetVal;
}


// This method stops the AG
void CAGService::Stop(void)
{
    DEBUGMSG(ZONE_OUTPUT, (L"BTAGSVC: Stopping Audio Gateway Service.\n"));
	RETAILMSG(1, (L"BTAGSVC:+%S, Stopping Audio Gateway Service.\r\n",__FUNCTION__));
    
    Lock();

    // TEst 
	//gAgTestVarCnt++;
	//RETAILMSG(1, (L"BTAGSVC:%S, gAgTestVarCnt (0x%X) = %d\r\n",__FUNCTION__, &gAgTestVarCnt, gAgTestVarCnt));

	RETAILMSG(1, (L"BTAGSVC:%S, SaveAGState:\r\n",__FUNCTION__));
    SaveAGState();
    
    HANDLE h1 = m_hThread;
    HANDLE h2 = m_hScoThread;
    
    m_hThread = NULL;
    m_hScoThread = NULL;
    
    m_fShutdown = TRUE;

	RETAILMSG(1, (L"BTAGSVC:%S, close sockets:\r\n",__FUNCTION__));

    if (INVALID_SOCKET != m_sockServer[0]) {
        closesocket(m_sockServer[0]);
        m_sockServer[0] = INVALID_SOCKET;
    }
    if (INVALID_SOCKET != m_sockServer[1]) {        
        closesocket(m_sockServer[1]);
        m_sockServer[1] = INVALID_SOCKET;
    }

    if (m_hCloseEvent) {
        SetEvent(m_hCloseEvent);
    }

	RETAILMSG(1, (L"BTAGSVC:%S, m_AGEngine.Deinit:\r\n",__FUNCTION__));
    m_AGEngine.Deinit();
	RETAILMSG(1, (L"BTAGSVC:%S, m_ATParser.Deinit:\r\n",__FUNCTION__));
    m_ATParser.Deinit();

    Unlock();

    RETAILMSG(1, (L"BTAGSVC:%S,Wait for closing thread 'm_hThread (0x%X)':\r\n",__FUNCTION__, m_hThread));
    if (h1) {
        if (h1 != (HANDLE) GetCurrentThreadId()) {
            WaitForSingleObject(h1, INFINITE);
        }

        CloseHandle(h1);
    }

	RETAILMSG(1, (L"BTAGSVC:%S,Wait for closing thread 'm_hScoThread (0x%X)':\r\n",__FUNCTION__, m_hScoThread));
    if (h2) {
        if (h2 != (HANDLE) GetCurrentThreadId()) {
            WaitForSingleObject(h2, INFINITE);
        }

        CloseHandle(h2);
    }

    DEBUGMSG(ZONE_OUTPUT, (L"BTAGSVC: Audio Gateway Service stopped.\n"));
	RETAILMSG(1, (L"BTAGSVC:-%S, Audio Gateway Service stopped.\r\n",__FUNCTION__));
}


// This method opens an audio session on the headset
DWORD CAGService::OpenAudio(void)
{
    DWORD dwRetVal; 

    DEBUGMSG(ZONE_OUTPUT, (L"BTAGSVC: Opening Bluetooth audio connection.\n"));
	RETAILMSG(1, (L"BTAGSVC:+%S, Opening Bluetooth audio connection.\r\n",__FUNCTION__));

    Lock();

    dwRetVal = m_AGEngine.OpenAGConnection(TRUE, FALSE);
    if ((ERROR_SUCCESS != dwRetVal) && (ERROR_ALREADY_INITIALIZED != dwRetVal)) {
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error opening AG Connection: %d.\n", dwRetVal));
		RETAILMSG(1, (L"BTAGSVC:%S, Error opening AG Connection: %d.\r\n",__FUNCTION__, dwRetVal));
        goto exit;
    }

    dwRetVal = ERROR_SUCCESS;

exit:
    Unlock();
    return dwRetVal;
}


// This method closes an audio session on the headset
DWORD CAGService::CloseAudio(void)
{
    DEBUGMSG(ZONE_OUTPUT, (L"BTAGSVC: Closing Bluetooth audio connection.\n"));
	RETAILMSG(1, (L"BTAGSVC:+%S\r\n",__FUNCTION__));

    Lock();    
    m_AGEngine.CloseAGConnection(FALSE);
    Unlock();
    
	RETAILMSG(1, (L"BTAGSVC:-%S\r\n",__FUNCTION__));
    return ERROR_SUCCESS;
}


// This method opens the control connection
DWORD CAGService::OpenControlConnection(BOOL fFirstOnly)
{
    DWORD dwRetVal; 

    DEBUGMSG(ZONE_OUTPUT, (L"BTAGSVC: Opening Bluetooth control connection.\n"));
	RETAILMSG(1, (L"BTAGSVC:+%S, Opening Bluetooth control connection.\r\n",__FUNCTION__));

    Lock();

    dwRetVal = m_AGEngine.OpenAGConnection(FALSE, fFirstOnly);
    if ((ERROR_SUCCESS != dwRetVal) && (ERROR_ALREADY_INITIALIZED != dwRetVal)) {
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error opening AG Connection: %d.\n", dwRetVal));
		RETAILMSG(1, (L"BTAGSVC:+%S, Error opening AG Connection: %d.\r\n",__FUNCTION__, dwRetVal));
        goto exit;
    }

    dwRetVal = ERROR_SUCCESS;

exit:
    Unlock();
    return dwRetVal;
}


// This method closes the control connection
DWORD CAGService::CloseControlConnection(void)
{
    DEBUGMSG(ZONE_OUTPUT, (L"BTAGSVC: Closing Bluetooth control connection.\n"));
	RETAILMSG(1, (L"BTAGSVC:+%S\r\n",__FUNCTION__));

    Lock();    
    m_AGEngine.CloseAGConnection(TRUE);
    Unlock();

	RETAILMSG(1, (L"BTAGSVC:-%S\r\n",__FUNCTION__));

    return ERROR_SUCCESS;
}


// This method sets the speaker volume of the peer
DWORD CAGService::SetSpeakerVolume(unsigned short usVolume)
{
    DEBUGMSG(ZONE_OUTPUT, (L"BTAGSVC: Setting Bluetooth audio speaker volume.\n"));
	RETAILMSG(1, (L"BTAGSVC:+%S, Setting Bluetooth audio speaker volume.\r\n",__FUNCTION__));
    
    if (usVolume <= 15) {
        Lock();
        m_AGEngine.SetSpeakerVolume(usVolume);
        Unlock();
        return ERROR_SUCCESS;
    }    

    return ERROR_INVALID_PARAMETER;
}


// This method sets the mic volume of the peer
DWORD CAGService::SetMicVolume(unsigned short usVolume)
{
    DEBUGMSG(ZONE_OUTPUT, (L"BTAGSVC: Setting Bluetooth audio microphone volume.\n"));
	RETAILMSG(1, (L"BTAGSVC:+%S, Setting Bluetooth audio microphone volume.\r\n",__FUNCTION__));
    
    if (usVolume <= 15) {
        Lock();
        m_AGEngine.SetMicVolume(usVolume);
        Unlock();
        return ERROR_SUCCESS;
    }    

    return ERROR_INVALID_PARAMETER;
}


// This method gets the speaker volume of the peer
DWORD CAGService::GetSpeakerVolume(unsigned short* pusVolume)
{
    AG_PROPS AGProps;

    Lock();
    
    m_AGEngine.GetAGProps(&AGProps);
    *pusVolume = AGProps.usSpeakerVolume;

    Unlock();

	RETAILMSG(1, (L"BTAGSVC:+%S, Get Bluetooth audio speaker volume = %d.\r\n",__FUNCTION__, *pusVolume));

    return ERROR_SUCCESS;
}


// This method gets the mic volume of the peer
DWORD CAGService::GetMicVolume(unsigned short* pusVolume)
{
    AG_PROPS AGProps;

    Lock();
    
    m_AGEngine.GetAGProps(&AGProps);
    *pusVolume = AGProps.usMicVolume;

    Unlock();

	RETAILMSG(1, (L"BTAGSVC:+%S, Get Bluetooth audio mic volume = %d.\r\n",__FUNCTION__, *pusVolume));

    return ERROR_SUCCESS;
}

// Get the power-save mode
DWORD CAGService::GetPowerMode(BOOL* pfPowerSave)
{
    AG_PROPS AGProps;

    Lock();
    
    m_AGEngine.GetAGProps(&AGProps);
    *pfPowerSave = AGProps.fPowerSave;

    Unlock();

    return ERROR_SUCCESS;
}

// Get the power-save mode
DWORD CAGService::SetPowerMode(BOOL fPowerSave)
{
    AG_PROPS AGProps;

    Lock();

    m_AGEngine.GetAGProps(&AGProps);

    if (AGProps.fPowerSave != fPowerSave) {
        AGProps.fPowerSave = fPowerSave;
        m_AGEngine.SetAGProps(&AGProps);
        SaveAGState();
    }

    Unlock();

    return ERROR_SUCCESS;
}

// Set the use Handsfree audio mode
DWORD CAGService::SetUseHFAudio(BOOL fUseHFAudio)
{
	RETAILMSG(1, (L"BTAGSVC:+%S, Set the use Handsfree audio mode = %d.\r\n",__FUNCTION__, fUseHFAudio));

    Lock();

    m_AGEngine.SetUseHFAudio(fUseHFAudio);

    Unlock();

    return ERROR_SUCCESS;
}

// Controls whether inband ring tones are enabled
DWORD CAGService::SetUseInbandRing(BOOL fUseInbandRing)
{
	RETAILMSG(1, (L"BTAGSVC:+%S, inband ring tones = %d.\r\n",__FUNCTION__, fUseInbandRing));

    Lock();

    m_AGEngine.SetUseInbandRing(fUseInbandRing);

    Unlock();

    return ERROR_SUCCESS;
}


// AG server thread
DWORD WINAPI CAGService::ListenThread(LPVOID pv)
{
    CAGService* pInst = (CAGService*)pv;
    pInst->ListenThread_Int();

    return 0;
}


// This method listens for incoming connections to the AG
void CAGService::ListenThread_Int(void)
{
    SOCKADDR_BTH    saServer;
    int             iLen;
    int             on = TRUE;
    AG_PROPS        AGProps;
    BOOL            fStackUp = FALSE;
    BOOL            fHandsfree;

    DEBUGMSG(ZONE_SERVICE, (L"BTAGSVC: ++ListenThread_Int\n"));
	RETAILMSG(1, (L"BTAGSVC:+%S\r\n",__FUNCTION__));

    Lock();

    // Make sure BT stack is up
    for (int i = 0 ; i < 100 ; ++i) {
        HANDLE hBthStackInited = OpenEvent (EVENT_ALL_ACCESS, FALSE, BTH_NAMEDEVENT_STACK_INITED);

        if (hBthStackInited) {
            DWORD dwRes = WaitForSingleObject (hBthStackInited, INFINITE);
            CloseHandle (hBthStackInited);
            if (WAIT_OBJECT_0 == dwRes) {
                fStackUp = TRUE;
                break;
            }
        }

        DEBUGMSG(ZONE_WARN, (L"BTAGSVC: BT stack is not ready, waiting...\n"));
		RETAILMSG(1, (L"BTAGSVC:%S BT stack is not ready, waiting...\r\n",__FUNCTION__));

        Sleep (1000);
    }

    if (! fStackUp) {
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Bluetooth stack is not up - aborting ListenThread_Int: %d.\n", GetLastError()));
		RETAILMSG(1, (L"BTAGSVC:%S Bluetooth stack is not up - aborting ListenThread_Int: %d.\r\n",__FUNCTION__, GetLastError()));
        goto exit;
    }
    
    m_AGEngine.GetAGProps(&AGProps);
    fHandsfree = ! AGProps.fNoHandsfree;

    // Create headset and hands-free sockets
    
    m_sockServer[0] = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    if (INVALID_SOCKET == m_sockServer[0]) {
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error opening socket: %d.\n", GetLastError()));
		RETAILMSG(1, (L"BTAGSVC:%S Error opening socket: %d.\r\n",__FUNCTION__, GetLastError()));
        goto exit;
    }

    if (fHandsfree) {
        m_sockServer[1] = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
        if (INVALID_SOCKET == m_sockServer[1]) {
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error opening socket: %d.\n", GetLastError()));
			RETAILMSG(1, (L"BTAGSVC:%S Error opening socket: %d.\r\n",__FUNCTION__, GetLastError()));
            goto exit;
        }
    }

    // Bind headset socket and add SDP record

    memset(&saServer, 0, sizeof(saServer));
    saServer.addressFamily = AF_BTH;
   
    if (0 != bind(m_sockServer[0], (SOCKADDR*)&saServer, sizeof(saServer))) {
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error calling bind: %d.\n", GetLastError()));
		RETAILMSG(1, (L"BTAGSVC:%S Error calling bind: %d.\r\n",__FUNCTION__, GetLastError()));
        goto exit;
    }

    iLen = sizeof(saServer);
    if (SOCKET_ERROR == getsockname(m_sockServer[0], (SOCKADDR*)&saServer, &iLen)) {
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error calling getsockname: %d.\n", GetLastError()));
		RETAILMSG(1, (L"BTAGSVC:%S Error calling getsockname: %d.\r\n",__FUNCTION__, GetLastError()));
        goto exit;
    }    

    if (ERROR_SUCCESS != AddSDPRecord(rgbSdpRecordHeadsetHS, SDP_HS_RECORD_SIZE, SDP_HS_CHANNEL_OFFSET, saServer.port, &m_SDPRecordHS)) {
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error adding headset SDP record.\n"));
		RETAILMSG(1, (L"BTAGSVC:%S Error adding headset SDP record.\r\n",__FUNCTION__));
        goto exit;
    }

    // Bind hands-free socket and add SDP record

    if (fHandsfree) {
        memset(&saServer, 0, sizeof(saServer));
        saServer.addressFamily = AF_BTH;
        
        if (0 != bind(m_sockServer[1], (SOCKADDR*)&saServer, sizeof(saServer))) {
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error calling bind: %d.\n", GetLastError()));
			RETAILMSG(1, (L"BTAGSVC:%S Error calling bind: %d.\r\n",__FUNCTION__, GetLastError()));
            goto exit;
        }    

        iLen = sizeof(saServer);
        if (SOCKET_ERROR == getsockname(m_sockServer[1], (SOCKADDR*)&saServer, &iLen)) {
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error calling getsockname: %d.\n", GetLastError()));
			RETAILMSG(1, (L"BTAGSVC:%S Error calling getsockname: %d.\r\n",__FUNCTION__, GetLastError()));
            goto exit;
        }

        rgbSdpRecordHeadsetHF[SDP_HF_CAPABILITY_OFFSET] = (BYTE) AGProps.usHFCapability & 0x1f; // first 5 bits in SDP record

        if (ERROR_SUCCESS != AddSDPRecord(rgbSdpRecordHeadsetHF, SDP_HF_RECORD_SIZE, SDP_HF_CHANNEL_OFFSET, saServer.port, &m_SDPRecordHF)) {
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error adding hands-free SDP record.\n"));    
			RETAILMSG(1, (L"BTAGSVC:%S Error adding hands-free SDP record.\r\n",__FUNCTION__));
            goto exit;
        }
    }

    // Continue setting up sockets

    listen(m_sockServer[0], 2);
    if (fHandsfree) {
        listen(m_sockServer[1], 2);
    }

    m_AGEngine.GetAGProps(&AGProps);

    if (AGProps.fAuth) {
        DEBUGMSG(ZONE_SERVICE, (L"BTAGSVC: Setting SO_BTH_AUTHENTICATE on server socket.\n"));
		RETAILMSG(1, (L"BTAGSVC:%S Setting SO_BTH_AUTHENTICATE on server socket.\r\n",__FUNCTION__));
        setsockopt(m_sockServer[0], SOL_RFCOMM, SO_BTH_AUTHENTICATE, (char *)&on, sizeof(on));
        if (fHandsfree) {
            setsockopt(m_sockServer[1], SOL_RFCOMM, SO_BTH_AUTHENTICATE, (char *)&on, sizeof(on));
        }
    }
    if (AGProps.fEncrypt) {
        DEBUGMSG(ZONE_SERVICE, (L"BTAGSVC: Setting SO_BTH_ENCRYPT on server socket.\n"));
		RETAILMSG(1, (L"BTAGSVC:%S Setting SO_BTH_ENCRYPT on server socket.\r\n",__FUNCTION__));
        setsockopt(m_sockServer[0], SOL_RFCOMM, SO_BTH_ENCRYPT, (char *)&on, sizeof(on));
        if (fHandsfree) {
            setsockopt(m_sockServer[1], SOL_RFCOMM, SO_BTH_ENCRYPT, (char *)&on, sizeof(on));
        }
    }

    // Set AG MTU to be RFCOMM default
    DWORD dwMTU = AG_MTU;     
    if (SOCKET_ERROR == setsockopt(m_sockServer[0], SOL_RFCOMM, SO_BTH_SET_MTU, (char *)&dwMTU, sizeof(dwMTU)))
    {
        DEBUGMSG(ZONE_WARN, (L"BTAGSVC: Error setting MTU: %d\n", GetLastError()));
		RETAILMSG(1, (L"BTAGSVC:%S Error setting MTU: %d.\r\n",__FUNCTION__, GetLastError()));
    }
    if (fHandsfree) {
        if (SOCKET_ERROR == setsockopt(m_sockServer[1], SOL_RFCOMM, SO_BTH_SET_MTU, (char *)&dwMTU, sizeof(dwMTU)))
        {
            DEBUGMSG(ZONE_WARN, (L"BTAGSVC: Error setting MTU: %d\n", GetLastError()));
			RETAILMSG(1, (L"BTAGSVC:%S Error setting MTU: %d.\r\n",__FUNCTION__, GetLastError()));
        }
    }

    while (1) {
        SOCKADDR_BTH    saClient;
        SOCKET          sockClient;
        USHORT          usRole = 0;
        BOOL            fHFSupport = FALSE;

        fd_set sockSet;
        
        FD_ZERO(&sockSet);
        FD_SET(m_sockServer[0], &sockSet);
        if (fHandsfree) {
            FD_SET(m_sockServer[1], &sockSet);
        }

		RETAILMSG(1, (L"BTAGSVC:%S fHandsfree = %d.\r\n",__FUNCTION__, fHandsfree));

        Unlock();
        int iSockets = select(0,&sockSet,NULL,NULL,NULL);
        Lock();

        if (m_fShutdown || (0 == iSockets) || (SOCKET_ERROR == iSockets)) {
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error in call to select: %d\n", GetLastError()));
			RETAILMSG(1, (L"BTAGSVC:%S Error in call to select %d.\r\n",__FUNCTION__, GetLastError()));
            goto exit;
        }

        SOCKET s = sockSet.fd_array[iSockets - 1];
        if (fHandsfree && (s == m_sockServer[1])) {
            // Peer device supports HFP
            fHFSupport = TRUE;
        }

		RETAILMSG(1, (L"BTAGSVC:%S Peer device supports HFP = %d.\r\n",__FUNCTION__, fHFSupport));

        iLen = sizeof(saClient);
        sockClient = accept(s, (SOCKADDR*)&saClient, &iLen);
        if (INVALID_SOCKET == sockClient) {
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error in call to accept, breaking... - error:%d\n", GetLastError()));
			RETAILMSG(1, (L"BTAGSVC:%S Error in call to accept, breaking... - error: %d.\r\n",__FUNCTION__, GetLastError()));
            goto exit;
        }

        DEBUGMSG(ZONE_SERVICE, (L"BTAGSVC: A Bluetooth peer device has connected to the Audio Gateway.\n"));
		RETAILMSG(1, (L"BTAGSVC:%S A Bluetooth peer device has connected to the Audio Gateway.\r\n",__FUNCTION__));

        if (!m_AGEngine.FindBTAddrInList(saClient.btAddr)) {
            DEBUGMSG(ZONE_WARN, (L"BTAGSVC: The peer device was not accepted since the user has never confirmed it as a device to be used.\n"));
			RETAILMSG(1, (L"BTAGSVC:%S The peer device was not accepted since the user has never confirmed it as a device to be used.\r\n",__FUNCTION__));
            closesocket(sockClient);
            continue;            
        }

        m_AGEngine.GetAGProps(&AGProps);
        if (AGProps.btAddrClient && (AGProps.btAddrClient != saClient.btAddr)) {
            DEBUGMSG(ZONE_WARN, (L"BTAGSVC: The peer device was not accepted since we already have an active connection.\n"));
			RETAILMSG(1, (L"BTAGSVC:%S The peer device was not accepted since twe already have an active connection.\r\n",__FUNCTION__));
            closesocket(sockClient);
            continue;            
        }

        (void) m_AGEngine.SetBTAddrList(saClient.btAddr, fHFSupport); // ignore return value in this case

        // We would like to switch to be Master if possible.  If baseband does not support scatternet
        // it is better for the Phone to be master.  It also allows us to enter low power modes instead
        // of just suggesting them.
        if (!AGProps.fNoRoleSwitch && (ERROR_SUCCESS == BthGetRole(&saClient.btAddr, &usRole))) {
            if (usRole != 0) { // Check if we are not the master
                usRole = 0; // Become master
                DWORD dwErr = BthSwitchRole(&saClient.btAddr, usRole);
                if (ERROR_SUCCESS != dwErr) {
                    DEBUGMSG(ZONE_WARN, (L"BTAGSVC: Failed to switch role to master.\n"));
					RETAILMSG(1, (L"BTAGSVC:%S Failed to switch role to master.\r\n",__FUNCTION__));
                }
            }
        }

        if (ERROR_SUCCESS != m_AGEngine.NotifyConnect(sockClient, fHFSupport)) {
            DEBUGMSG(ZONE_WARN, (L"BTAGSVC: The AG Engine did not accept the connection to the device.\n"));
			RETAILMSG(1, (L"BTAGSVC:%S The AG Engine did not accept the connection to the device.\r\n",__FUNCTION__));
            closesocket(sockClient);
            continue;
        }
    }

exit:
    if (m_SDPRecordHF) {
        RemoveSDPRecord(&m_SDPRecordHF);
    }
    if (m_SDPRecordHS) {
        RemoveSDPRecord(&m_SDPRecordHS);
    }

    g_dwState = SERVICE_STATE_OFF;
   
    Unlock();

    if (! m_fShutdown) {
        Stop();
    } 

    DEBUGMSG(ZONE_SERVICE, (L"BTAGSVC: --ListenThread_Int\n"));
	RETAILMSG(1, (L"BTAGSVC:-%S\r\n",__FUNCTION__));
}


// AG SCO Server Thread
DWORD WINAPI CAGService::SCOListenThread(LPVOID pv)
{
    CAGService* pInst = (CAGService*)pv;
    pInst->SCOListenThread_Int();

    return 0;
}


// This method listens for incoming SCO connections to the AG
void CAGService::SCOListenThread_Int(void)
{
    DWORD dwErr = ERROR_SUCCESS;
    AG_PROPS AGProps;

    DEBUGMSG(ZONE_SERVICE, (L"BTAGSVC: ++SCOListenThread_Int\n"));

    Lock();

    m_AGEngine.GetAGProps(&AGProps);
    
    if (AGProps.fPCMMode) {
        dwErr = BthAcceptSCOConnections(TRUE);
    }
    
    if (ERROR_SUCCESS == dwErr) {
        HANDLE hBTConnect = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"system/events/bluetooth/ConnectionsChange");
        if (hBTConnect) {
            HANDLE h[2];            
            DWORD dwWait;

            h[0] = hBTConnect;
            h[1] = m_hCloseEvent;
            
            while (1) {            
                Unlock();                
                dwWait = WaitForMultipleObjects(2, h, FALSE, INFINITE);
                Lock();
                
                if (dwWait != WAIT_OBJECT_0) {
                    break;
                }

                DEBUGMSG(ZONE_SERVICE, (L"BTAGSVC: SCOListenThread_Int - Connection Event.\n"));

                m_AGEngine.ConnectionEvent();
            }
        }
        else {
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: SCOListenThread_Int: Error opening named event: %d\n", dwErr));
        }

        if (AGProps.fPCMMode) {
            BthAcceptSCOConnections(FALSE);
        }
    }
    else {
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: SCOListenThread_Int: Error accepting SCO connections: %d\n", dwErr));
    }

    Unlock();

    DEBUGMSG(ZONE_SERVICE, (L"BTAGSVC: --SCOListenThread_Int\n"));
}


// This method loads AG settings
DWORD CAGService::LoadAGState(void)
{
    DWORD       dwRetVal    = ERROR_SUCCESS;
    DWORD       cdwBytes    = 0;
    DWORD       dwData      = 0;
    HKEY        hk          = NULL;
    AG_PROPS    AGProps;

    m_AGEngine.GetAGProps(&AGProps);
    
    dwRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RK_AUDIO_GATEWAY, 0, 0, &hk);
    if (dwRetVal != ERROR_SUCCESS) {
        goto exit;
    }

    cdwBytes = sizeof(dwData);
    if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("MapAudioToPcmMode"), 0, NULL, (PBYTE)&dwData, &cdwBytes)) {
        AGProps.fPCMMode = dwData;
    }

    cdwBytes = sizeof(dwData);
    if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("MicVolume"), 0, NULL, (PBYTE)&dwData, &cdwBytes)) {
        AGProps.usMicVolume = (USHORT) dwData;
    }

    cdwBytes = sizeof(dwData);
    if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("SpkVolume"), 0, NULL, (PBYTE)&dwData, &cdwBytes)) {
        AGProps.usSpeakerVolume = (USHORT) dwData;
    }

    cdwBytes = sizeof(dwData);
    if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("Authenticate"), 0, NULL, (PBYTE)&dwData, &cdwBytes)) {
        AGProps.fAuth = dwData;
    }

    cdwBytes = sizeof(dwData);
    if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("Encrypt"), 0, NULL, (PBYTE)&dwData, &cdwBytes)) {
        AGProps.fEncrypt = dwData;
    }

    cdwBytes = sizeof(dwData);
    if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("NoHandsfree"), 0, NULL, (PBYTE)&dwData, &cdwBytes)) {
        AGProps.fNoHandsfree = dwData;
    }

    cdwBytes = sizeof(dwData);
    if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("PowerSave"), 0, NULL, (PBYTE)&dwData, &cdwBytes)) {
        AGProps.fPowerSave = dwData;
    }

    cdwBytes = sizeof(dwData);
    if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("Capability"), 0, NULL, (PBYTE)&dwData, &cdwBytes)) {
        AGProps.usHFCapability = (USHORT) dwData;
    }

    cdwBytes = sizeof(dwData);
    if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("PageTimeout"), 0, NULL, (PBYTE)&dwData, &cdwBytes)) {
        AGProps.usPageTimeout = (USHORT) dwData;
    }

    cdwBytes = sizeof(dwData);
    if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("SniffDelay"), 0, NULL, (PBYTE)&dwData, &cdwBytes)) {
        AGProps.ulSniffDelay = (ULONG) dwData;
    }

    cdwBytes = sizeof(dwData);
    if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("SniffMax"), 0, NULL, (PBYTE)&dwData, &cdwBytes)) {
        AGProps.usSniffMax = (USHORT) dwData;
    }

    cdwBytes = sizeof(dwData);
    if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("SniffMin"), 0, NULL, (PBYTE)&dwData, &cdwBytes)) {
        AGProps.usSniffMin = (USHORT) dwData;
    }

    cdwBytes = sizeof(dwData);
    if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("SniffAttempt"), 0, NULL, (PBYTE)&dwData, &cdwBytes)) {
        AGProps.usSniffAttempt = (USHORT) dwData;
    }

    cdwBytes = sizeof(dwData);
    if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("SniffTimeout"), 0, NULL, (PBYTE)&dwData, &cdwBytes)) {
        AGProps.usSniffTimeout = (USHORT) dwData;
    }

    cdwBytes = sizeof(dwData);
    if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("NoRoleSwitch"), 0, NULL, (PBYTE)&dwData, &cdwBytes)) {
        AGProps.fNoRoleSwitch = dwData;
    }

    cdwBytes = sizeof(dwData);
    if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("UnattendedMode"), 0, NULL, (PBYTE)&dwData, &cdwBytes)) {
        AGProps.fUnattendedMode = dwData;
    } 

    cdwBytes = sizeof(dwData);
    if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("ConnectScoInAlways"), 0, NULL, (PBYTE)&dwData, &cdwBytes)) {
        AGProps.fUseHFAudioAlways = dwData;
    }

    RegCloseKey(hk);
    
    m_AGEngine.SetAGProps(&AGProps);
    
exit:    
    return dwRetVal;
}


// This method saves AG settings
DWORD CAGService::SaveAGState(void)
{
    DWORD       dwRetVal    = ERROR_SUCCESS;
    DWORD       dwDisp      = 0;
    DWORD       dwData      = 0;
    HKEY        hk          = NULL;
    AG_PROPS    AGProps;

    dwRetVal = RegCreateKeyEx(HKEY_LOCAL_MACHINE, RK_AUDIO_GATEWAY, 0, NULL, 0, NULL, NULL, &hk, &dwDisp);
    if (ERROR_SUCCESS != dwRetVal) {
        goto exit;
    }

    m_AGEngine.GetAGProps(&AGProps);

    dwData = AGProps.usMicVolume;
    dwRetVal = RegSetValueEx(hk, _T("MicVolume"), 0, REG_DWORD, (PBYTE)&dwData, sizeof(dwData));
    if (ERROR_SUCCESS != dwRetVal) {
        goto exit;
    }

    dwData = AGProps.usSpeakerVolume;
    dwRetVal = RegSetValueEx(hk, _T("SpkVolume"), 0, REG_DWORD, (PBYTE)&dwData, sizeof(dwData));
    if (ERROR_SUCCESS != dwRetVal) {
        goto exit;
    }

    dwData = AGProps.fPowerSave;
    dwRetVal = RegSetValueEx(hk, _T("PowerSave"), 0, REG_DWORD, (PBYTE)&dwData, sizeof(dwData));
    if (ERROR_SUCCESS != dwRetVal) {
        goto exit;
    }    

exit:
    if (hk) {
        RegCloseKey(hk);
    }
    
    return dwRetVal;
}


// This method adds an SDP record
DWORD CAGService::AddSDPRecord(PBYTE rbgSdpRecord, DWORD cbSdpRecord, DWORD dwChannelOffset, unsigned long ulPort, unsigned long* pSdpRecord)
{
    DWORD dwRetVal  = ERROR_SUCCESS;
    DWORD dwSizeOut = 0;

    struct {
        BTHNS_SETBLOB   b;
        unsigned char   uca[MAX_SDP_RECORD_SIZE];
    } bigBlob;

    ULONG ulSdpVersion = BTH_SDP_VERSION;
    *pSdpRecord = 0;

    bigBlob.b.pRecordHandle   = pSdpRecord;
    bigBlob.b.pSdpVersion     = &ulSdpVersion;
    bigBlob.b.fSecurity       = 0;
    bigBlob.b.fOptions        = 0;
    bigBlob.b.ulRecordLength  = cbSdpRecord;

    PREFAST_ASSERT(cbSdpRecord <= MAX_SDP_RECORD_SIZE);
    memcpy(bigBlob.b.pRecord, rbgSdpRecord, cbSdpRecord);

    bigBlob.b.pRecord[dwChannelOffset] = (unsigned char)ulPort;

    BLOB blob;
    blob.cbSize    = sizeof(BTHNS_SETBLOB) + cbSdpRecord - 1;
    blob.pBlobData = (PBYTE) &bigBlob;

    WSAQUERYSET Service;
    memset(&Service, 0, sizeof(Service));
    Service.dwSize = sizeof(Service);
    Service.lpBlob = &blob;
    Service.dwNameSpace = NS_BTH;

    dwRetVal = WSASetService(&Service, RNRSERVICE_REGISTER, 0);
    if (ERROR_SUCCESS != dwRetVal) {
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error adding SDP record.\n"));
        goto exit;
    }

    DEBUGMSG(ZONE_SERVICE, (L"BTAGSVC: Successfully added SDP record.\n"));
    
exit:
    return dwRetVal;
}


// This method removes an SDP record
DWORD CAGService::RemoveSDPRecord(unsigned long* pSdpRecord)
{
    DWORD dwRetVal      = ERROR_SUCCESS;
    ULONG ulSdpVersion  = BTH_SDP_VERSION;

    BTHNS_SETBLOB delBlob;
    memset(&delBlob, 0, sizeof(delBlob));
    delBlob.pRecordHandle   = pSdpRecord;
    delBlob.pSdpVersion     = &ulSdpVersion;

    BLOB blob;
    blob.cbSize    = sizeof(BTHNS_SETBLOB);
    blob.pBlobData = (PBYTE) &delBlob;

    WSAQUERYSET Service;
    memset(&Service, 0, sizeof(Service));
    Service.dwSize = sizeof(Service);
    Service.lpBlob = &blob;
    Service.dwNameSpace = NS_BTH;

    dwRetVal = WSASetService(&Service, RNRSERVICE_DELETE, 0);

    *pSdpRecord = 0;

#ifdef DEBUG
    if (ERROR_SUCCESS == dwRetVal) {
        DEBUGMSG(ZONE_SERVICE, (L"BTAGSVC: Successfully removed SDP record.\n"));
    }
#endif // DEBUG

    return dwRetVal;
}


