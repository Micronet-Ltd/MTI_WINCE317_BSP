// btmodem_service.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include "MicUserSdk.h"

char* StatusMessages[] = 
{
	"Turning on the modem...",
	"Modem is turned on",
	"Waiting for the SIM registration...",
	"SIM registered",
	"Activating the MUX driver...",
	"MUX driver activated",
	"Starting the Bluetooth DUN profile service...",
	"Service started",
	"Stopping the Bluetooth DUN profile service...",
	"Service stopped",
	"Deactivating the MUX driver...",
	"MUX driver deactivated",
	"Turning off the modem...",
	"Modem is turned off" 
};

HANDLE ghOutMsgQueue;
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

	if (ghOutMsgQueue != INVALID_HANDLE_VALUE)
	{
		if (!WriteMsgQueue(ghOutMsgQueue, &msg, sizeof(BTMODEM_MESSAGE), 0, 0))
			return false;
	}

	return true;
}

DWORD WINAPI MsgTh(PVOID param)
{
	TCHAR str[256];

	MSGQUEUEOPTIONS msgOptions	= {0};
	msgOptions.dwSize		= sizeof(MSGQUEUEOPTIONS);
	msgOptions.dwFlags		= MSGQUEUE_ALLOW_BROKEN;
	msgOptions.cbMaxMessage = sizeof(BTMODEM_MESSAGE);
	msgOptions.bReadAccess	= 1;

	HANDLE hMsgQueue = CreateMsgQueue(BTMODEM_OUTQUEUE_NAME, &msgOptions);
	if (hMsgQueue == INVALID_HANDLE_VALUE)
		return 0;

	BTMODEM_MESSAGE lsdata;
	DWORD dwSize, dwFlags;
	
	HANDLE ev[2];
	ev[0] = param;
	ev[1] = hMsgQueue;

	while ((WAIT_OBJECT_0 + 1) == WaitForMultipleObjects(sizeof(ev)/sizeof(ev[0]), ev, 0, INFINITE))
	{
		if (ReadMsgQueue(hMsgQueue, &lsdata, sizeof(BTMODEM_MESSAGE), &dwSize, 0, &dwFlags)) 
		{
			switch (lsdata.Type)
			{
				case BTMODEM_SERVICE_REQUEST:
					{
						if (lsdata.MsgBody.ServiceRequest.Request)
						{
							wsprintf(str, L"Device (%04x%08x) is trying to connect. Do you accept this connection?", GET_NAP(lsdata.MsgBody.ServiceRequest.Addr), GET_SAP(lsdata.MsgBody.ServiceRequest.Addr));
							int res = MessageBox(NULL, str, L"Attempt to connect", MB_YESNO | MB_TOPMOST);
							if (res == 0)
								DebugBreak();

							lsdata.MsgBody.ServiceRequest.Request = (res == IDYES) ? 1 : 0;
							PutMessageToQueue(BTMODEM_SERVICE_REQUEST, &(lsdata.MsgBody));
						}
						else
						{
							// disconnected
							printf("Device (%04x%08x) is disconnected.\r\n", GET_NAP(lsdata.MsgBody.ServiceRequest.Addr), GET_SAP(lsdata.MsgBody.ServiceRequest.Addr));
						}
					}
					break;

				case BTMODEM_MODEM_STATUS:
					printf("Modem status: %d.\r\n", lsdata.MsgBody.ModemStatus.Status);
					break;

				case BTMODEM_SERVICE_STATUS:
					printf("Service status: %s\r\n", StatusMessages[lsdata.MsgBody.ServiceStatus.Status]);
					break;
			}
		}
		else
			break;
	}

	CloseMsgQueue(hMsgQueue);
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	UINT32 res;

	MSGQUEUEOPTIONS msgOptions	= {0};
	msgOptions.dwSize		= sizeof(MSGQUEUEOPTIONS);
	msgOptions.dwFlags		= 0;
	msgOptions.cbMaxMessage = sizeof(BTMODEM_MESSAGE);
	msgOptions.bReadAccess	= 0;

	ghOutMsgQueue = CreateMsgQueue(BTMODEM_INQUEUE_NAME, &msgOptions);
	if (ghOutMsgQueue == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, L"Can't create message queue.", L"BTModem test error", MB_OK);
		return 0;
	}

	HANDLE hExitEvent = CreateEvent(0, 0, 0, 0);
	HANDLE hThread = CreateThread(NULL, 0, MsgTh, (void *)hExitEvent, 0, NULL);
	Sleep(500);

	UINT32 power;
	if (MIC_GSMGetPowerStatus(&power) == 0)
	{
		if (!power)
			res = MIC_GSMPower(1);
	}

	HANDLE hService = CreateFile(BTMODEM_PREFIX, 0, 0, NULL, 0, 0, NULL);
	if (INVALID_HANDLE_VALUE == hService)
	{
		MessageBox(NULL, L"Can't open BTModem device.", L"BTModem test error", MB_OK);
		goto close_all;
	}

	if (MIC_BTModemNotifyRegister(hService) != 0)
	{
		CloseHandle(hService);
		MessageBox(NULL, L"Can't register BTModem notifications engine.", L"BTModem test error", MB_OK);
		goto close_all;
	}

	res = MIC_BTModemActivate(hService, L"Telit");
	if (res != 0)
	{
		CloseHandle(hService);
		printf("Activation error = %d.\r\n", res);
		MessageBox(NULL, L"Can't activate BTModem service.", L"BTModem test error", MB_OK);
		goto close_all;
	}

	MessageBox(NULL, L"Press OK to terminate...", L"BTModem test", MB_OK);

	res = MIC_BTModemDeactivate(hService);
	if (res != 0)
		MessageBox(NULL, L"Can't deactivate BTModem service.", L"BTModem test error", MB_OK);

close_all:;
	CloseHandle(hService);
	SetEvent(hExitEvent);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hExitEvent);
	CloseHandle(hThread);

	CloseMsgQueue(ghOutMsgQueue);
	return 0;
}

