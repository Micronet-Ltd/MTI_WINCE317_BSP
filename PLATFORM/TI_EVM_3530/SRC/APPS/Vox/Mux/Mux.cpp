// wce_mux_act.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <pm.h>
#include "mux.h"
#include <winioctl.h>
#include "MicUserSdk.h"
#include "Mixer.h"

#define MAX_LOADSTRING 100

HINSTANCE			g_hInst;			// current instance
HWND				g_hWndCommandBar;	// command bar handle
HWND				g_hMainWnd;
BOOL				g_isPortOpenedByListener = false;
BOOL				g_isPort6Opened = false;
BOOL				g_isCom6Test = false;
BOOL				g_isRssiStarted = false;
HANDLE				g_hCom = INVALID_HANDLE_VALUE;
TCHAR				g_LastDialedNumber[100] = {0};


// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	EditWndProc(HWND, UINT, WPARAM, LPARAM);

HWND hListbox = NULL;
HANDLE hMuxQueue, hBreakEvent, hRssiExitEvent;

void AddToList(TCHAR *text)
{
	LRESULT count = SendMessage(hListbox, LB_GETCOUNT, 0, 0);
	if (count > 500)
	{
		SendMessage(hListbox, LB_RESETCONTENT, 0, 0);
		count = 0;
	}

	SendMessage(hListbox, LB_ADDSTRING, 0, (LPARAM)text);
	SendMessage(hListbox, LB_SETCURSEL, count, 0);
	UpdateWindow(hListbox);
}

void AddToList(char *text)
{
	TCHAR tbuf[100];
	int len = strlen(text);

	for (int i = 0; i < len; i++)
	{
		if (text[i] == 13 || text[i] == 10)
		{
			text[i] = ' ';
		}
	}

	mbstowcs(tbuf, text, len + 1);
	AddToList(tbuf);
}

DWORD WINAPI RssiTh(PVOID hEvent)
{
	DWORD wlen;

	while (WaitForSingleObject((HANDLE)hEvent, 1000) == WAIT_TIMEOUT)
	{
		WriteFile(g_hCom, "AT+CSQ\r", strlen("AT+CSQ\r"), &wlen, NULL);
	}

	AddToList(L"RSSI test stopped");
	g_isRssiStarted = false;
	return 0;
}

DWORD WINAPI PortListenerTh(PVOID param)
{
	TCHAR msg[100];
	char buf[100];
	DWORD len;

	g_isPort6Opened = _tcsstr((TCHAR *)param, L"6") != NULL;

	g_hCom = CreateFile((TCHAR *)param, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == g_hCom)
	{
		wsprintf(msg, L"Can't open port. Error %d.", GetLastError()); 
		AddToList(msg);
		return 0;
	}

	g_isPortOpenedByListener = true;
	wsprintf(msg, L"Start listening %s port...", param);
	AddToList(msg);
	DWORD dwBytesReceived;
	unsigned long Mask;
	SetCommMask(g_hCom, EV_RXCHAR);

	while (WaitCommEvent(g_hCom, &Mask, 0))
	{
		if (Mask == 0)
			break;
		else
		{
			if (!ReadFile(g_hCom, buf, 99, &dwBytesReceived, 0))
				break;
			else
			{
				buf[dwBytesReceived] = 0;
				AddToList(buf);
			}
		}
	}

	CloseHandle(g_hCom);
	g_isPortOpenedByListener = false;
	g_hCom = INVALID_HANDLE_VALUE;
	wsprintf(msg, L"Stop listening %s port", param);
	AddToList(msg);
	return 0;
}

BOOL AT(HANDLE h, char *command, char *response)
{
	char mem[255] = {0};
	char *rbuf;
	DWORD wlen, rlen;

	if (response != NULL)
		rbuf = response;
	else
		rbuf = mem;

	BOOL wf = WriteFile(h, command, strlen(command), &wlen, NULL);
	Sleep(500);
	BOOL rf = ReadFile(h, rbuf, 255, &rlen, NULL);
	return wf;
}

void Mic_GSM710ModemOn(BOOL bNeedRegistration)
{
	TCHAR b[100] = {0};
	INT32 res = MIC_GSMPower(1);
	if (res != 0)
	{
		wsprintf(b, L"MIC_GSMPower(ON) error = %d", res);
		AddToList(b);
	}
	else
		AddToList(L"Modem is ON");

	if (!bNeedRegistration)
		return;

	HANDLE hCom = CreateFile(L"COM0:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hCom == INVALID_HANDLE_VALUE)
	{
		AddToList(L"Can't open COM0 port");
		return;
	}

	DCB dcb = {0};
	dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(hCom, &dcb))
    {
		wsprintf(b, L"GetCommState failed, error = %d\r\n", GetLastError());
		AddToList(b);
		CloseHandle(hCom);
		return;
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

    if (!SetCommState(hCom, &dcb))
    {
		wsprintf(b, L"SetCommState failed, error = %d\r\n", GetLastError());
		AddToList(b);
		CloseHandle(hCom);
		return;
    }

    COMMTIMEOUTS cto;
    memset(&cto, 0, sizeof(COMMTIMEOUTS));
    cto.ReadIntervalTimeout = MAXDWORD;
    cto.WriteTotalTimeoutConstant = 1000;
    if (!SetCommTimeouts(hCom, &cto))
	{
		wsprintf(b, L"SetCommTimeouts failed, error = %d\r\n", GetLastError());
		AddToList(b);
		CloseHandle(hCom);
		return;
	}

	AddToList(L"Waiting for modem registration...");
	char response[255] = {};
	DWORD ticks = GetTickCount() + 30000;
	BOOL bRegistered = FALSE;
	
	while (ticks > GetTickCount())
	{
		AT(hCom, "at+creg?\r", response);
		if (strstr(response, "+CREG: 0,1"))
		{
			bRegistered = TRUE;
			break;
		}
	}

	CloseHandle(hCom);
	if (!bRegistered)
	{
		AddToList(L"Registration timeout error");
		return;
	}

	AddToList(L"Modem successfully registered");
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
		
		if (!WriteMsgQueue(h, &msg, sizeof(GSM710MESSAGE), 0, 0))
		{
			DWORD error = GetLastError();
			if (ERROR_PIPE_NOT_CONNECTED == error)
				AddToList(L"Error sending message. MUX not active?");
			else
				AddToList(L"Error sending message");
		}
		else
			AddToList(L"Message sent");

		CloseMsgQueue(h);
	}
	else
		AddToList(L"Message queue error");
}

DWORD WINAPI MsgTh(PVOID param)
{
	GSM710MESSAGE lsdata;
	DWORD dwSize, dwFlags;
	TCHAR b[100];
	
	HANDLE ev[2];
	ev[0] = hBreakEvent;
	ev[1] = hMuxQueue;

	while ((WAIT_OBJECT_0 + 1) == WaitForMultipleObjects(sizeof(ev)/sizeof(ev[0]), ev, 0, INFINITE))
	{
		if (ReadMsgQueue(hMuxQueue, &lsdata, sizeof(GSM710MESSAGE), &dwSize, 0, &dwFlags)) 
		{
			if (lsdata.Type == GSM710MESSAGETYPE_CONNECTIONSTATE)
				AddToList(lsdata.Value == 1 ? L"GSM710MUX driver activated." : L"GSM710MUX driver deactivated.");
			else if (lsdata.Type == GSM710MESSAGETYPE_MODEMSTATE)
				AddToList(L"Please turn on the GPRS modem.");
			else if (lsdata.Type == GSM710MESSAGETYPE_POWERSTATE)
			{
				CEDEVICE_POWER_STATE power_state = (CEDEVICE_POWER_STATE)lsdata.Value;
				wsprintf(b, L"Power state was changed to %d", power_state);
				AddToList(b);
			}
			else if (lsdata.Type == GSM710MESSAGETYPE_CONNECTIONERROR)
			{
				wsprintf(b, L"Connection error = %d", lsdata.Value);
				AddToList(b);
			}

		}
	}

	return 0;
}


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	MSG msg;

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}

#ifndef WIN32_PLATFORM_WFSP
	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WCE_MUX_ACT));
#endif // !WIN32_PLATFORM_WFSP

	turnVoiceOnOff(0, FALSE);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
#ifndef WIN32_PLATFORM_WFSP
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
#endif // !WIN32_PLATFORM_WFSP
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WCE_MUX_ACT));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;
    TCHAR szTitle[MAX_LOADSTRING];		// title bar text
    TCHAR szWindowClass[MAX_LOADSTRING];	// main window class name

    g_hInst = hInstance; // Store instance handle in our global variable

#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
    // SHInitExtraControls should be called once during your application's initialization to initialize any
    // of the device specific controls such as CAPEDIT and SIPPREF.
    SHInitExtraControls();
#endif // WIN32_PLATFORM_PSPC || WIN32_PLATFORM_WFSP

    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
    LoadString(hInstance, IDC_WCE_MUX_ACT, szWindowClass, MAX_LOADSTRING);

#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
    //If it is already running, then focus on the window, and exit
    hWnd = FindWindow(szWindowClass, szTitle);	
    if (hWnd) 
    {
        // set focus to foremost child window
        // The "| 0x00000001" is used to bring any owned windows to the foreground and
        // activate them.
        SetForegroundWindow((HWND)((ULONG) hWnd | 0x00000001));
        return 0;
    } 
#endif // WIN32_PLATFORM_PSPC || WIN32_PLATFORM_WFSP

    if (!MyRegisterClass(hInstance, szWindowClass))
    {
    	return FALSE;
    }

    hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

	hListbox = CreateWindow(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOSEL, 10, 30, 460, 200, hWnd, NULL, hInstance, NULL); 

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

#ifndef SHELL_AYGSHELL
    if (g_hWndCommandBar)
    {
        CommandBar_Show(g_hWndCommandBar, TRUE);
    }
#endif // !SHELL_AYGSHELL

    return TRUE;
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;
	TCHAR buf[80];

    switch (message) 
    {
		case WM_AT_COMM_TEST:
			{
				HANDLE h = g_hCom;

				if (!g_isPortOpenedByListener || (g_isPort6Opened != g_isCom6Test))
				{
					h = CreateFile(g_isCom6Test ? L"COM6:" : L"COM8:", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
					if (INVALID_HANDLE_VALUE == h)
					{
						AddToList(L"Can't open port");
						return FALSE;
					}
				}
				
				DWORD wlen;
				if (!WriteFile(h, (char*)wParam, strlen((char*)wParam), &wlen, NULL))
					AddToList(L"Can't write to port");
	
				if (!g_isPortOpenedByListener || (g_isPort6Opened != g_isCom6Test))
					CloseHandle(h);
			}
			break;

		case WM_DIAL_NUMBER:
				{
					char g_buf[100];
					DWORD len;

					HANDLE g_hCom = CreateFile(L"COM6:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
					if (INVALID_HANDLE_VALUE == g_hCom)
					{
						AddToList(L"Can't open port");
						return FALSE;
					}

					wsprintf(buf, L"ATD%s;\r", g_LastDialedNumber);
					wcstombs(g_buf, buf, 99);
					WriteFile(g_hCom, g_buf, strlen(g_buf), &len, NULL);

					MessageBox(hWnd, L"Press OK to hang up", L"Voice call", 0);

					strcpy(g_buf, "ATH\r");
					WriteFile(g_hCom, g_buf, strlen(g_buf), &len, NULL);

					CloseHandle(g_hCom);
				}
				break;

        case WM_COMMAND:
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            // Parse the menu selections:
            switch (wmId)
            {
			    case ID_LISTEN_PORT_COM0: 
				case ID_LISTEN_PORT_COM6:
				case ID_LISTEN_PORT_COM8:
					{
                      TCHAR  portNum[8];
					  DWORD  dwPortNum = 0;

					  if(wmId == ID_LISTEN_PORT_COM6)
						  dwPortNum = 6;
					  else if(wmId == ID_LISTEN_PORT_COM8) 
                          dwPortNum = 8;


						if (!g_isPortOpenedByListener)
						{
							wsprintf( portNum, _T("COM%d:"), dwPortNum );
							CloseHandle(CreateThread(NULL, 0, PortListenerTh, (LPVOID)portNum, 0, NULL));
							//CloseHandle(CreateThread(NULL, 0, PortListenerTh, (LPVOID)(wmId == ID_LISTEN_PORT_COM6 ? L"COM6:" : L"COM8:"), 0, NULL));
						}
						else
							AddToList(L"Already listening");
					}
					break;

				case ID_VOICE_STOP_LISTENING:
					{
						if (g_isPortOpenedByListener)
							SetCommMask(g_hCom, 0);
					}
					break;

				case ID_VOICE_ACTIVATE:
					{
						HANDLE hCom = CreateFile(L"COM6:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
						if (INVALID_HANDLE_VALUE == hCom)
						{
							AddToList(L"Can't open port");
							return FALSE;
						}
						INT32 res = MIC_GSMVoice(hCom, 1, 3);
						CloseHandle(hCom);

						wsprintf(buf, L"MIC_GSMVoice result = %d", res);
						AddToList(buf);

						if (res == 0)
						{
							bool bMyRes = turnVoiceOnOff(1, FALSE);
							if (bMyRes)
							{
								bMyRes = putVoiceInGain(20, FALSE);
								if (bMyRes)
									bMyRes = putVoiceOutGain(12, FALSE);
							}
							if (!bMyRes)
							{
								AddToList(L"Mixer init failed");
							}
						}
					}
					break;

				case ID_MODEM_TURN_ON:
				case ID_MODEM_TURN_ON_REG:
					{
						Mic_GSM710ModemOn(wmId == ID_MODEM_TURN_ON_REG);
					}
					break;

				case ID_MODEM_TURN_OFF:
					{
						INT32 res = MIC_GSMPower(0);
						if (res != 0)
						{
							wsprintf(buf, L"MIC_GSMPower(OFF) error = %d", res);
							AddToList(buf);
						}
						else
							AddToList(L"Modem is OFF");
					}
					break;

				case ID_MUX_SEND_MODEM_ON_MESSAGE:
					{
						SendModemOnToMux();
					}
					break;

				case ID_MUX_ACTIVATE:
					{
						INT32 res = MIC_GSM710Activate(1, 0);
						wsprintf(buf, L"Activation result = %d", res);
						AddToList(buf);
					}
					break;

				case ID_MUX_DEACTIVATE:
					{
						INT32 res = MIC_GSM710Activate(0, 0);
						turnVoiceOnOff(0, FALSE);
						wsprintf(buf, L"Deactivation result = %d", res);
						AddToList(buf);
					}
					break;

				case ID_MUX_ACTIVATE_DEFAULT:
					{
						INT32 res = MIC_GSM710Activate(1, 1);
						wsprintf(buf, L"Activation (with default) result = %d", res);
						AddToList(buf);
					}
					break;

				case ID_MUX_DEACTIVATE_DEFAULT:
					{
						INT32 res = MIC_GSM710Activate(0, 1);
						turnVoiceOnOff(0, FALSE);
						wsprintf(buf, L"Deactivation (with default) result = %d", res);
						AddToList(buf);
					}
					break;

                case ID_SENTATCOMMANDTO_COM0:
				case ID_SENTATCOMMANDTO_COM6:
				case ID_SENTATCOMMANDTO_COM8:
					{
						g_isCom6Test = wmId == ID_SENTATCOMMANDTO_COM6;
	                    DialogBox(g_hInst, (LPCTSTR)IDD_AT_COMMAND, hWnd, EditWndProc);
					}
					break;

				case ID_TESTS_STARTRSSITEST:
					{
						if (g_isPortOpenedByListener)
						{
							if (!g_isRssiStarted)
							{
								g_isRssiStarted = true;
								CloseHandle(CreateThread(NULL, 0, RssiTh, (LPVOID)hRssiExitEvent, 0, NULL));
							}
						}
						else
							AddToList(L"Start listening first!");

					}
					break;

				case ID_TESTS_STOPRSSITEST:
					{
						if (g_isRssiStarted)
						{
							SetEvent(hRssiExitEvent);
						}
					}
					break;

				case ID_TEST_OPEN_PORTS:
					{
						TCHAR s[256];
						wcscpy(s, L"COM0: ");
						HANDLE hCom = CreateFile(L"COM0:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
						if (INVALID_HANDLE_VALUE == hCom)
							wcscat(s, L"Error");
						else
						{
							CloseHandle(hCom);
							wcscat(s, L"Ok");
						}

						wcscat(s, L"\r\nCOM6: ");
						hCom = CreateFile(L"COM6:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
						if (INVALID_HANDLE_VALUE == hCom)
							wcscat(s, L"Error");
						else
						{
							CloseHandle(hCom);
							wcscat(s, L"Ok");
						}

						wcscat(s, L"\r\nCOM8: ");
						hCom = CreateFile(L"COM8:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
						if (INVALID_HANDLE_VALUE == hCom)
							wcscat(s, L"Error");
						else
						{
							CloseHandle(hCom);
							wcscat(s, L"Ok");
						}

						MessageBox(hWnd, s, L"Open ports", 0);
					}
					break;

				case ID_VOICE_CALL:
					{
						DialogBox(g_hInst, (LPCTSTR)IDD_VOICE_CALL, hWnd, EditWndProc);
					}
					break;
				
				case ID_VOICE_AUTOANSWER:
					{
						char g_buf[64];
						DWORD len;

						HANDLE g_hCom = CreateFile(L"COM6:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
						if (INVALID_HANDLE_VALUE == g_hCom)
						{
							AddToList(L"Can't open port");
							return FALSE;
						}

						strcpy(g_buf, "ATS0=2\r\n");
						WriteFile(g_hCom, g_buf, strlen(g_buf), &len, NULL);

						CloseHandle(g_hCom);

					}
					break; 
                case ID_EXIT:
                    DestroyWindow(hWnd);
                    break;

                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_CREATE:
#ifndef SHELL_AYGSHELL
            g_hWndCommandBar = CommandBar_Create(g_hInst, hWnd, 1);
            CommandBar_InsertMenubar(g_hWndCommandBar, g_hInst, IDR_MENU, 0);
            CommandBar_AddAdornments(g_hWndCommandBar, 0, 0);
#endif // !SHELL_AYGSHELL

			{
				MSGQUEUEOPTIONS msgOptions	= {0};
				msgOptions.dwSize		= sizeof(MSGQUEUEOPTIONS);
				msgOptions.dwFlags		= MSGQUEUE_ALLOW_BROKEN;
				msgOptions.cbMaxMessage = sizeof(GSM710MESSAGE);
				msgOptions.bReadAccess	= 1;

				hMuxQueue = CreateMsgQueue(GSM710_OUTQUEUE_NAME, &msgOptions);
				if (hMuxQueue == INVALID_HANDLE_VALUE)
					DebugBreak();

				hBreakEvent = CreateEvent(0, 0, 0, 0);
				hRssiExitEvent = CreateEvent(0, 0, 0, 0);
				CloseHandle(CreateThread(NULL, 0, MsgTh, (LPVOID)hWnd, 0, NULL));
				g_hMainWnd = hWnd;

				MIXER_ERROR_CODES  mixErr = enumMixer();
			}
            break;

        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            
            // TODO: Add any drawing code here...
            
            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:

			deEnumMixer();

			if (g_isPortOpenedByListener)
				SetCommMask(g_hCom, 0);

			SetEvent(hBreakEvent);
			SetEvent(hRssiExitEvent);
			CloseHandle(hBreakEvent);
			CloseHandle(hRssiExitEvent);
			CloseMsgQueue(hMuxQueue);
#ifndef SHELL_AYGSHELL
            CommandBar_Destroy(g_hWndCommandBar);
#endif // !SHELL_AYGSHELL
#ifdef SHELL_AYGSHELL
            CommandBar_Destroy(g_hWndMenuBar);
#endif // SHELL_AYGSHELL
            PostQuitMessage(0);
            break;

#if defined(SHELL_AYGSHELL) && !defined(WIN32_PLATFORM_WFSP)
        case WM_ACTIVATE:
            // Notify shell of our activate message
            SHHandleWMActivate(hWnd, wParam, lParam, &s_sai, FALSE);
            break;
        case WM_SETTINGCHANGE:
            SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
            break;
#endif // SHELL_AYGSHELL && !WIN32_PLATFORM_WFSP

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK EditWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;
	TCHAR buf[100], buf1[100];
	char atcmd[100];

#if defined(SHELL_AYGSHELL) && !defined(WIN32_PLATFORM_WFSP)
    static SHACTIVATEINFO s_sai;
#endif // SHELL_AYGSHELL && !WIN32_PLATFORM_WFSP
	
    switch (message) 
    {
        case WM_COMMAND:
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            // Parse the menu selections:
			if (wmId >= ID0 && wmId <= ID9)
			{
				GetDlgItemText(hWnd, IDC_NUMBER, buf, 99);
				wsprintf(buf1, L"%c", wmId - ID0 + '0');
				wcscat(buf, buf1);
				SetDlgItemText(hWnd, IDC_NUMBER, buf);
				return 0;
			}
            switch (wmId)
            {
				case IDDIAL:
					{
						if (GetDlgItemText(hWnd, IDC_NUMBER, buf, 99) > 0)
						{
							GetDlgItemText(hWnd, IDC_NUMBER, g_LastDialedNumber, 99);
							PostMessage(g_hMainWnd, WM_DIAL_NUMBER, 0, 0);
							DestroyWindow(hWnd);
						}
					}
					break;

				case IDCLEAR:
                    SetDlgItemText(hWnd, IDC_NUMBER, L"");
                    break;

				case IDBACKSPACE:
                    GetDlgItemText(hWnd, IDC_NUMBER, buf, 99);
					buf[wcslen(buf) - 1] = 0;
					SetDlgItemText(hWnd, IDC_NUMBER, buf);
                    break;

                case IDCANCEL:
                    DestroyWindow(hWnd);
                    break;

                case IDOK:
					GetDlgItemText(hWnd, IDC_EDIT1, buf, 99);
					wcstombs(atcmd, buf, 99);
					if (!strstr(atcmd, "+++"))
						strcat(atcmd, "\r\n");
					SendMessage(g_hMainWnd, WM_AT_COMM_TEST, (WPARAM)atcmd, 0);
                    DestroyWindow(hWnd);
                    break;

                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;

        case WM_INITDIALOG:
			SetDlgItemText(hWnd, IDC_NUMBER, g_LastDialedNumber);
			// SetWindowText(hWnd, g_isCom6Test ? L"COM6 Test" : L"COM8 Test");
            break;

		case WM_DESTROY:
            // PostQuitMessage(0);
            break;


        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
