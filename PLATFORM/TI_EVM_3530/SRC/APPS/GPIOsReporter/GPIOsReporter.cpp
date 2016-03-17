// GPIOsReporter.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "GPIOsReporter.h"
#include <windowsx.h>
#include <commctrl.h>
#include "socket.h"
#include "Iphlpapi.h"
#include "Aio_api.h"
#include "winioctl.h"

#define MAX_LOADSTRING 100
#define OPEN_BUFFER_LENGTH            sizeof(AIO_NOTIFY_CONTEXT)     // in bytes

// Global Variables:
HINSTANCE			g_hInst;			// current instance
HWND				g_hWndCommandBar;	// command bar handle
HBRUSH g_hWhiteBrush = NULL;
HBRUSH g_hSeaBrush = NULL;
HANDLE g_hevSend[3] = {0};
HANDLE hMSGQ = NULL;
Socket _socket;
char  desktopIp[16] = {0};
char  Buffer[100];
DWORD inputBuffer[OPEN_BUFFER_LENGTH] = {0};
int   port = 50000;

enum MSG_QUEUE {_MSG_QUEUE, _MSG_QUEUE_Occured, _MSG_QUEUE_Exit};

// Forward declarations of functions included in this code module:
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI MSG_QUEUE_Thread(HANDLE);
BOOL GetDesktopIp(char *ipAddr, int len);
DWORD WINAPI SocketThread(HANDLE);
DWORD WINAPI AutomotiveThread(HANDLE);
DWORD WINAPI GPIO182_Thread(HANDLE);
HANDLE hMutex;


int SendNotification(char* buf, size_t bufSize)
{
	DWORD dwWaitResult;

	dwWaitResult = WaitForSingleObject (hMutex,   // Handle of mutex object
		5000L);   // Five-second time-out
	switch (dwWaitResult)
	{
	case WAIT_OBJECT_0:
		__try
		{
			if (_socket.IsConnected())
				_socket.Send(buf, bufSize);
		}
		__finally
		{
			if (! ReleaseMutex (hMutex))
			{
				// Your code to deal with the error goes here.
			}
		}
		break;

		// Cannot get mutex object ownership due to time-out
	case WAIT_TIMEOUT:
		return FALSE;

		// Got ownership of an abandoned mutex object
	case WAIT_ABANDONED:
		return FALSE;

	case WAIT_FAILED:
		int ret = GetLastError();
		ret = 0;
	}

}


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	// Set callback named "WndProc" for IDD_ABOUTBOX dialog box
	DialogBox(hInstance, (LPCTSTR)IDD_DLG, 0, (DLGPROC)WndProc);

	return 0;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hExitEvent;
	static HANDLE hSocketThread; 
	static HANDLE hMsgQueueThread;
	static HANDLE hAutomotive;
	static HANDLE hGPIO182;
	
	switch (message)
	{
		case WM_INITDIALOG:
			{
				if (!GetDesktopIp(desktopIp, sizeof(desktopIp)))
				{
					strcpy_s(desktopIp, sizeof("192.168.55.100\0"), "192.168.55.100\0");
				}

				hMutex = CreateMutex (
					NULL,                       // No security descriptor
					FALSE,                      // Mutex object not owned
					TEXT("TCPSender")); // Object name

				// Start messagequeue thread
				g_hevSend[_MSG_QUEUE] = CreateEvent(0, 0, 0, 0);
				g_hevSend[_MSG_QUEUE_Occured] = hMSGQ;
				g_hevSend[_MSG_QUEUE_Exit] = CreateEvent(0, 0, 0, 0);

				hExitEvent = CreateEvent(0, 1, 0, 0);
				hAutomotive = CreateThread(NULL, 0, AutomotiveThread, (void *)hExitEvent, 0, NULL);
				hSocketThread = CreateThread(NULL, 0, SocketThread, (void *)hExitEvent, 0, NULL);
				hMsgQueueThread = CreateThread(NULL, 0, MSG_QUEUE_Thread, (void *)hExitEvent, 0, NULL);

				hGPIO182 =  CreateThread(NULL, 0, GPIO182_Thread, (void *)hExitEvent, 0, NULL);

			}
			return TRUE;	

			case WM_CTLCOLORSTATIC:
			{
				HDC hDC = (HDC)wParam;
				SetTextColor(hDC, RGB(0, 64, 128));
				SetBkColor(hDC, RGB(255, 255, 255));
			}
			return (long)GetStockObject(WHITE_BRUSH);

		case WM_CTLCOLORBTN:
			{
				HDC hDC = (HDC)wParam;
				SetTextColor(hDC, RGB(255, 255, 255));
				SetBkColor(hDC, RGB(255, 255, 255));
			
				if(g_hSeaBrush == NULL)
					g_hSeaBrush = CreateSolidBrush(RGB(0, 64, 128));
			}
			return (long)g_hSeaBrush;


		case WM_CTLCOLORDLG:
			if(g_hWhiteBrush == NULL)
				g_hWhiteBrush = CreateSolidBrush(RGB(55, 55, 55));

			return (long)g_hWhiteBrush;

		case WM_COMMAND:
			{
				switch(GET_WM_COMMAND_ID(wParam, lParam))
				{
				case IDC_BUTTON_NOTIFICATION:
					strcpy_s (Buffer,100,"GPIO Reporter Test~\r\n\0");
					SendNotification(Buffer, 19);
					break;
				case IDC_EXIT:
				case IDCANCEL:
					SetEvent(hExitEvent);
					WaitForSingleObject(hMsgQueueThread, INFINITE);
					CloseHandle(hMsgQueueThread);
					CloseHandle(hAutomotive);
					CloseHandle(hSocketThread);
					CloseHandle(hGPIO182);
					CloseHandle(hExitEvent);
					CloseHandle(hMutex);
					EndDialog(hDlg, 0);
				break;
				}
			}
			break;
	}
    return FALSE; // switch (message)}

}


DWORD WINAPI MSG_QUEUE_Thread(HANDLE hExitEvent)
{
	DWORD dwRes;
	char szLogLine[512] = {0};

	while (WaitForSingleObject(hExitEvent, 100) == WAIT_TIMEOUT)
	{	
		dwRes = WaitForMultipleObjects(2, g_hevSend, false, 1000);
		switch(dwRes)
		{
			// Request for exit signalled
			case WAIT_OBJECT_0:
				// Signal that exit is occured
				SetEvent(g_hevSend[_MSG_QUEUE_Exit]);
				return 0;
			
			// POWER STATE event occured
			case WAIT_OBJECT_0 + 1:

				break;
		}// switch
	}// while

	return 0;
}


DWORD WINAPI SocketThread(HANDLE hExitEvent)
{
	while (WaitForSingleObject(hExitEvent, 100) == WAIT_TIMEOUT)
	{
		if (_socket.IsConnected())
		{
			char *buffer = (char *)_socket.Receive(6);
			if (!buffer)
				continue;

			if (buffer[0] == '`')
			{
				/*
				switch (buffer[1])
				{
					case CommandGet:
						MIC_TemperatureSensorForceEnqueue(hTempSensor);
						break;

					case CommandExit:
						_socket.Disconnect();
						SetEvent(hExitEvent);
						return 0;

					case CommandRestart:
						_socket.Disconnect();
						SetSystemPowerState(NULL, POWER_STATE_RESET, POWER_FORCE);
				}
				*/
			}

			free(buffer);
		}
		else
		{
			_socket.Disconnect();
			_socket.Connect(desktopIp, port);
		}
	}

	return 0;
}

DWORD WINAPI AutomotiveThread(HANDLE hExitEvent)
{
	static HANDLE hIO;
	DWORD rc;
	AIO_INPUT_CONTEXT inputContext;
	inputBuffer[0] = OPEN_BUFFER_LENGTH;
	hIO = MIC_AIOOpen(inputBuffer);
	memset(&inputContext,0,sizeof(inputContext));

	inputContext.size = sizeof(AIO_INPUT_CONTEXT);
	inputContext.pinNo.AUT_IN_1 = 1;
	rc = MIC_AIOSetInputMode(hIO, &inputContext);
	rc = MIC_AIOSetSignalStabilityTime(hIO, &inputContext, 20);
	MIC_AIOGetInputInfo(hIO, &inputContext);

#define PIN_0 0

	rc = MIC_AIORegisterForStateChange(hIO, inputBuffer,(LPVOID)&inputContext,TRUE);
	rc = MIC_AIOStartStateChangeNotify(hIO, inputBuffer,(LPVOID)&inputContext);

	while (WaitForSingleObject(hExitEvent, 10) == WAIT_TIMEOUT)
	{
		rc = MIC_AIOWaitForStateChange(hIO, inputBuffer,(LPVOID)&inputContext);
		if (rc)
		{
			sprintf_s (Buffer,59,"Power Detection Interrupt Event - Previous %x Current %x~\0",inputContext.prevState ,inputContext.currentState);
			SendNotification(Buffer, 59);
			RETAILMSG(1, (L"inputContext.currentState state is 0x%x\r\n",inputContext.currentState));
		}
	}

	MIC_AIOClose(hIO, inputBuffer);

	return 0;
}

DWORD WINAPI GPIO182_Thread(HANDLE hExitEvent)
{

	HANDLE lhnd = OpenEvent(EVENT_ALL_ACCESS,FALSE,L"CDSEXTERNAL182");

	RETAILMSG(1, (L"GPIO182_Thread lhnd = %x\r\n",lhnd));

	while (1)
	{
		DWORD state = WaitForSingleObject(lhnd,INFINITE);

		sprintf_s (Buffer,100,"MDT Power Toggle~\0");
		SendNotification(Buffer, 18);
		RETAILMSG(1, (L"GPIO 182 state = %x\r\n",state));

		ResetEvent(lhnd);
	}
	
	return 0;
}



BOOL GetDesktopIp(char *ipAddr, int len)
{
	BOOL ret = FALSE;

	IP_ADAPTER_INFO *pAdapterInfo = (IP_ADAPTER_INFO *)new BYTE[sizeof(IP_ADAPTER_INFO)];
	ULONG OutBufLen = 0;
	DWORD res = GetAdaptersInfo(pAdapterInfo, &OutBufLen);
	if (res == ERROR_BUFFER_OVERFLOW)
	{
		delete (pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)new BYTE[OutBufLen];
		res = GetAdaptersInfo(pAdapterInfo, &OutBufLen);
	}

	if (res == NO_ERROR)
	{
		IP_ADAPTER_INFO *info = pAdapterInfo;
		do
		{
			//RETAILMSG(1, (L"AdapterName = %s\r\n", info->AdapterName));
			if (strstr(info->AdapterName, "USB CABLE"))
			{
				strncpy_s(ipAddr, len, info->GatewayList.IpAddress.String, len);
				ret = TRUE;
				break;
			}

			info = info->Next;
		}
		while (info);

	}

	delete (pAdapterInfo);
	return ret;
}
