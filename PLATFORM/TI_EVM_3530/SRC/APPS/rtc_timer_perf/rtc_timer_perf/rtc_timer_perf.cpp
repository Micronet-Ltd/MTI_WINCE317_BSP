// rtc_timer_perf.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "rtc_timer_perf.h"
//#include <windows.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE			g_hInst;			// current instance
#ifdef SHELL_AYGSHELL
HWND				g_hWndMenuBar;		// menu bar handle
#else // SHELL_AYGSHELL
HWND				g_hWndCommandBar;	// command bar handle
#endif // SHELL_AYGSHELL

// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
#ifndef WIN32_PLATFORM_WFSP
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
#endif // !WIN32_PLATFORM_WFSP

int g_Done = 0;
static HANDLE hAct = 0, hTC = 0, hn = 0;
unsigned __int64 nAlarm = 200000000;
UINT32 alarm_res = 10;
TCHAR notify[256] = {0};
TCHAR got_notify[256] = {0};
TCHAR set_notify[256] = {0};
TCHAR tc_notify[256] = {0};
HWND g_hWnd = 0;
int do_suspend = 0;

HANDLE InitTimeNotification(HANDLE hNotification)
{
	CE_NOTIFICATION_TRIGGER nt = {0};
	CE_USER_NOTIFICATION un = {0};

	nt.dwSize = sizeof(CE_NOTIFICATION_TRIGGER);
	nt.dwType = CNT_EVENT;
	nt.dwEvent = NOTIFICATION_EVENT_TIME_CHANGE;
	nt.lpszApplication = TEXT("\\\\.\\Notifications\\NamedEvents\\System_time_changed");
	nt.lpszArguments = 0;

	return CeSetUserNotificationEx(hNotification,&nt,0);
}

HANDLE InitPowerNotification(HANDLE hNotification)
{
	CE_NOTIFICATION_TRIGGER nt = {0};
	CE_USER_NOTIFICATION un = {0};
	SYSTEMTIME st = {0};
	FILETIME ft;


	GetLocalTime(&st);
	SystemTimeToFileTime(&st, &ft);
	unsigned __int64 t;
	memcpy(&t, &ft, sizeof(t));
	t += 6*nAlarm;
	t /= nAlarm;
	t *= nAlarm;

	memcpy(&ft, &t, sizeof(t));
	FileTimeToSystemTime(&ft, &st);
	nt.dwSize = sizeof(CE_NOTIFICATION_TRIGGER);
	nt.dwType = CNT_TIME;
	nt.lpszApplication = TEXT("\\\\.\\Notifications\\NamedEvents\\_____Update_Time");
	nt.lpszArguments = 0;
	memcpy(&nt.stStartTime, &st, sizeof(SYSTEMTIME));
	memcpy(&nt.stEndTime, &st, sizeof(SYSTEMTIME));
	wsprintf(notify, L"%d/%d/%d %d:%d:%d power on alarm set\r\n", nt.stStartTime.wYear, nt.stStartTime.wMonth, nt.stStartTime.wDay,
								nt.stStartTime.wHour, nt.stStartTime.wMinute, nt.stStartTime.wSecond);
	OutputDebugString(notify);

	return CeSetUserNotificationEx(hNotification,&nt,&un);
}

DWORD WINAPI PSThread(PVOID pvParam)
{
	int debug_to = 0;
    HANDLE hNotify = 0;
	SYSTEMTIME st = {0};
	FILETIME ft;
	CE_NOTIFICATION_TRIGGER nt = {0};
	CE_USER_NOTIFICATION un = {0};

	hAct = CreateEvent(0,0,0,TEXT("_____Update_Time"));
	if(!hAct)
		return 0;

	hTC	= CreateEvent(0, 0, 0,TEXT("System_time_changed"));
	if(!hTC)
		return 0;

	hNotify = InitTimeNotification(0);
//	debug_to = KernelIoControl(IOCTL_KLIB_GETALARMRESOLUTION, 0, 0, (DWORD *)&nAlarm, sizeof(nAlarm), 0);//(DWORD *)&debug_to);

//	debug_to = 0;

	GetLocalTime(&st);
	SystemTimeToFileTime(&st, &ft);
	unsigned __int64 t;
	memcpy(&t, &ft, sizeof(t));
	t += nAlarm;
	memcpy(&ft, &t, sizeof(t));
	FileTimeToSystemTime(&ft, &st);
	st.wSecond = 0;

	nt.dwSize = sizeof(CE_NOTIFICATION_TRIGGER);
	nt.dwType = CNT_TIME;
	nt.lpszApplication = TEXT("\\\\.\\Notifications\\NamedEvents\\_____Update_Time");
	nt.lpszArguments = 0;
	memcpy(&nt.stStartTime, &st, sizeof(SYSTEMTIME));
	memcpy(&nt.stEndTime, &st, sizeof(SYSTEMTIME));
	hn = CeSetUserNotificationEx(hn, &nt, &un);
	wsprintf(notify, L"%d/%d/%d %d:%d:%d alarm set\r\n", nt.stStartTime.wYear, nt.stStartTime.wMonth, nt.stStartTime.wDay,
					nt.stStartTime.wHour, nt.stStartTime.wMinute, nt.stStartTime.wSecond);
	wsprintf(set_notify, L"%d/%d/%d %d:%d:%d alarm set", nt.stStartTime.wYear, nt.stStartTime.wMonth, nt.stStartTime.wDay,
					nt.stStartTime.wHour, nt.stStartTime.wMinute, nt.stStartTime.wSecond);
	OutputDebugString(notify);


	while(!g_Done)
	{
		HANDLE obj[2] = {hAct, hTC};
		DWORD dwStatus = WaitForMultipleObjects(2, obj, 0, 15000);
		switch(dwStatus)
		{
			case WAIT_TIMEOUT:
			{
				if(debug_to)
				{
					if(hn)
					{
						DWORD *buf = 0;
						DWORD need;
						int nRes = CeGetUserNotification(hn, 0, &need, 0);
						if(1)
						{
							buf = (DWORD *)new char[need];
							if(buf)
							{
								nRes = CeGetUserNotification(hn, need, &need, (unsigned char *)buf);
								CE_NOTIFICATION_INFO_HEADER *head;
								head = (CE_NOTIFICATION_INFO_HEADER *)buf;
								
								memcpy(&head->pcent->stEndTime, &head->pcent->stStartTime, sizeof(SYSTEMTIME));

								hn = CeSetUserNotificationEx(hn, head->pcent, &un);
								if(!hn)
								{
									wsprintf(notify, L"%d fatal error\r\n", GetLastError());
									OutputDebugString(notify);
									g_Done = 1;
								}
								delete[] buf;
							}
						}
					}
//					DebugBreak();
//					debug_to = 0;
				}
				break;
			}
			case (WAIT_OBJECT_0 + 0):
			{
				///////////////////////////////////////
				// Attention !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				//
				// Here you should realize algoritm of recognizing from which notification the event has been got
				// It's mandatary to prevent GWES and OS memory leaks
				//
				//
				/////////////////////////////
				if(hn)
				{
				//	CeClearUserNotification(hn);
					CloseHandle(hn);
				}

				hn = 0;

				GetLocalTime(&nt.stStartTime);
				wsprintf(notify, L"%d/%d/%d %d:%d:%d alarm got\r\n", nt.stStartTime.wYear, nt.stStartTime.wMonth, nt.stStartTime.wDay,
								nt.stStartTime.wHour, nt.stStartTime.wMinute, nt.stStartTime.wSecond);
				wsprintf(got_notify, L"%d/%d/%d %d:%d:%d alarm got", nt.stStartTime.wYear, nt.stStartTime.wMonth, nt.stStartTime.wDay,
								nt.stStartTime.wHour, nt.stStartTime.wMinute, nt.stStartTime.wSecond);
				OutputDebugString(notify);
				GetLocalTime(&st);
				SystemTimeToFileTime(&st, &ft);
				unsigned __int64 t;
				memcpy(&t, &ft, sizeof(t));
				t += nAlarm;
				t += nAlarm/2;
				t /= nAlarm;
				t *= nAlarm;
//				if(t % nAlarm)
//				{
//					t += nAlarm;
//					t /= nAlarm;
//					t *= nAlarm;
//				}
				memcpy(&ft, &t, sizeof(t));
				FileTimeToSystemTime(&ft, &st);
/*
				if(st.wSecond % alarm_res)
				{
					st.wSecond += alarm_res/2;
					st.wSecond /= alarm_res;
					st.wSecond *= alarm_res;
				}
*/
				nt.dwSize = sizeof(CE_NOTIFICATION_TRIGGER);
				nt.dwType = CNT_TIME;
				nt.lpszApplication = TEXT("\\\\.\\Notifications\\NamedEvents\\_____Update_Time");
				nt.lpszArguments = 0;
				memcpy(&nt.stStartTime, &st, sizeof(SYSTEMTIME));
				memcpy(&nt.stEndTime, &st, sizeof(SYSTEMTIME));

				hn = CeSetUserNotificationEx(hn, &nt, &un);
				if(!hn)
				{
					wsprintf(notify, L"%d fatal error\r\n", GetLastError());
					OutputDebugString(notify);
					g_Done = 1;
					break;
				}

				wsprintf(notify, L"%d/%d/%d %d:%d:%d alarm set\r\n", nt.stStartTime.wYear, nt.stStartTime.wMonth, nt.stStartTime.wDay,
					nt.stStartTime.wHour, nt.stStartTime.wMinute, nt.stStartTime.wSecond);
				wsprintf(set_notify, L"%d/%d/%d %d:%d:%d alarm set", nt.stStartTime.wYear, nt.stStartTime.wMonth, nt.stStartTime.wDay,
					nt.stStartTime.wHour, nt.stStartTime.wMinute, nt.stStartTime.wSecond);
				OutputDebugString(notify);
//				debug_to = 1;
				if(g_hWnd)
				{
					RECT r;
					GetClientRect(g_hWnd, &r);
					InvalidateRect(g_hWnd, &r, 1);
				}
				if(do_suspend)
					SetTimer(g_hWnd, 200, 10000, 0);
//					SetSystemPowerState(0, POWER_STATE_SUSPEND, POWER_FORCE);

				break;
			}
			case (WAIT_OBJECT_0 + 1):
			{
				GetLocalTime(&st);
				wsprintf(tc_notify, L"%d/%d/%d %d:%d:%d time changed", st.wYear, st.wMonth, st.wDay,
								st.wHour, st.wMinute, st.wSecond);
				if(hn)
				{
					DWORD *buf = 0;
					DWORD need;
					int nRes = CeGetUserNotification(hn, 0, &need, 0);
					if(1)
					{
						buf = (DWORD *)new char[need];
						if(buf)
						{
							nRes = CeGetUserNotification(hn, need, &need, (unsigned char *)buf);
							CE_NOTIFICATION_INFO_HEADER *head;
							head = (CE_NOTIFICATION_INFO_HEADER *)buf;
							
							GetLocalTime(&st);
							SystemTimeToFileTime(&st, &ft);
							unsigned __int64 t;
							memcpy(&t, &ft, sizeof(t));
							t += nAlarm;
							t += nAlarm/2;
							t /= nAlarm;
							t *= nAlarm;

							memcpy(&ft, &t, sizeof(t));
							FileTimeToSystemTime(&ft, &st);
							memcpy(&head->pcent->stStartTime, &st, sizeof(SYSTEMTIME));
							memcpy(&head->pcent->stEndTime, &st, sizeof(SYSTEMTIME));
							wsprintf(set_notify, L"%d/%d/%d %d:%d:%d alarm set", head->pcent->stStartTime.wYear,
								head->pcent->stStartTime.wMonth, head->pcent->stStartTime.wDay,
								head->pcent->stStartTime.wHour, head->pcent->stStartTime.wMinute, head->pcent->stStartTime.wSecond);

							hn = CeSetUserNotificationEx(hn, head->pcent, &un);
							if(!hn)
							{
								wsprintf(notify, L"%d fatal error\r\n", GetLastError());
								OutputDebugString(notify);
								g_Done = 1;
							}
							delete[] buf;
						}
					}
				}
				if(g_hWnd)
				{
					RECT r;
					GetClientRect(g_hWnd, &r);
					InvalidateRect(g_hWnd, &r, 1);
				}
				break;
			}
		}
	}
	return 1;
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	MSG msg;


	HANDLE hTh = CreateThread(0, 0, PSThread, 0, 0, 0);
	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}

#ifndef WIN32_PLATFORM_WFSP
	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RTC_TIMER_PERF));
#endif // !WIN32_PLATFORM_WFSP

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

	WaitForSingleObject(hTh, INFINITE);
	CloseHandle(hTh);

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
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RTC_TIMER_PERF));
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
    LoadString(hInstance, IDC_RTC_TIMER_PERF, szWindowClass, MAX_LOADSTRING);

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

		g_hWnd = hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

#ifdef WIN32_PLATFORM_PSPC
    // When the main window is created using CW_USEDEFAULT the height of the menubar (if one
    // is created is not taken into account). So we resize the window after creating it
    // if a menubar is present
    if (g_hWndMenuBar)
    {
        RECT rc;
        RECT rcMenuBar;

        GetWindowRect(hWnd, &rc);
        GetWindowRect(g_hWndMenuBar, &rcMenuBar);
        rc.bottom -= (rcMenuBar.bottom - rcMenuBar.top);
		
        MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);
    }
#endif // WIN32_PLATFORM_PSPC

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

UINT32 NAND_Write(LPVOID lpv)
{
	HANDLE outf = CreateFile(L"\\Storage Card\\nk2.bin", GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if(outf == INVALID_HANDLE_VALUE)
		return 0;
	HANDLE inf = CreateFile(L"nk2.bin", GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	if(inf == INVALID_HANDLE_VALUE)
	{
		CloseHandle(outf);
		return 0;
	}

	int Ret;
	DWORD Read, Written;
	UINT8 buff[1024];
	TCHAR dbg[512];

	wsprintf(dbg, L"=================> Start Copy(NAND->SD)\r\n");
	OutputDebugString(dbg);
	do
	{
		Ret = ReadFile(inf, buff, sizeof(buff), &Read, 0);
		if(!Ret)
		{
			wsprintf(dbg, L"%d read error\r\n", GetLastError());
			OutputDebugString(dbg);
			break;
		}
		if(Read)
		{
			Ret = WriteFile(outf, buff, Read, &Written, 0);
			if(!Ret)
			{
				wsprintf(dbg, L"%d write error\r\n", GetLastError());
				OutputDebugString(dbg);
				break;
			}
		}
	}while(Ret && Read && Written);

	CloseHandle(inf);
	CloseHandle(outf);
	wsprintf(dbg, L"Copy Finished (NAND->SD) <=================\r\n");
	OutputDebugString(dbg);

	return 1;
}

UINT32 SD_Write(LPVOID lpv)
{
	HANDLE outf = CreateFile(L"nk.bin", GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if(outf == INVALID_HANDLE_VALUE)
		return 0;
	HANDLE inf = CreateFile(L"\\Storage Card\\nk.bin", GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	if(inf == INVALID_HANDLE_VALUE)
	{
		CloseHandle(outf);
		return 0;
	}

	int Ret;
	DWORD Read, Written;
	UINT8 buff[1024];
	TCHAR dbg[512];

	wsprintf(dbg, L"=================> Start Copy (SD->NAND)\r\n");
	OutputDebugString(dbg);
	do
	{
		Ret = ReadFile(inf, buff, sizeof(buff), &Read, 0);
		if(!Ret)
		{
			wsprintf(dbg, L"%d read error\r\n", GetLastError());
			OutputDebugString(dbg);
			break;
		}
		if(Read)
		{
			Ret = WriteFile(outf, buff, Read, &Written, 0);
			if(!Ret)
			{
				wsprintf(dbg, L"%d write error\r\n", GetLastError());
				OutputDebugString(dbg);
				break;
			}
		}
	}while(Ret && Read && Written);

	CloseHandle(inf);
	CloseHandle(outf);
	wsprintf(dbg, L"Copy Finished (SD->NAND) <=================\r\n");
	OutputDebugString(dbg);

	return 1;
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
	static HANDLE hRTC;
	static SYSTEMTIME lt, lt2;
	static unsigned __int64 ft, ft2;
	static TCHAR tch[512] = {0};
	static int timer = 2000;

#if defined(SHELL_AYGSHELL) && !defined(WIN32_PLATFORM_WFSP)
    static SHACTIVATEINFO s_sai;
#endif // SHELL_AYGSHELL && !WIN32_PLATFORM_WFSP
	
    switch (message) 
    {
		case WM_TIMER:
		{
			if(wParam == 200)
			{
				KillTimer(hWnd, 200);
				SetSystemPowerState(0, POWER_STATE_SUSPEND, POWER_FORCE);
				break;
			}
			if(hRTC != INVALID_HANDLE_VALUE)
			{
				DeviceIoControl(hRTC, IOCTL_RTC_GETTIME, 0, 0, &lt2, sizeof(lt2), 0, 0);

				SystemTimeToFileTime(&lt, (FILETIME *)&ft);
				SystemTimeToFileTime(&lt2, (FILETIME *)&ft2);

				ft2 -= ft;
				ft2 /= (10000 * 1000);

				wsprintf(tch, L"%d ticks for %d seconds", timer, ft2);
				memcpy(&lt, &lt2, sizeof(SYSTEMTIME));

				RECT r;
				GetClientRect(hWnd, &r);

				InvalidateRect(hWnd, &r, 1);
			}
			KillTimer(hWnd, 100);
			if(timer >= 24*60*60*1000)
				timer = 2000;
			else
				timer += 1000;
			SetTimer(hWnd, 100, timer, 0);
			break;
		}
        case WM_COMMAND:
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            // Parse the menu selections:
            switch (wmId)
            {
#ifndef WIN32_PLATFORM_WFSP
                case IDM_HELP_ABOUT:
                    DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, About);
                    break;
#endif // !WIN32_PLATFORM_WFSP
#ifndef SHELL_AYGSHELL
                case IDM_FILE_EXIT:
                    DestroyWindow(hWnd);
                    break;
				case IDM_FILE_SHD:
					g_Done = 1;
					InitPowerNotification(0);
					SetSystemPowerState(0, POWER_STATE_OFF, POWER_FORCE);
					break;
				case IDM_FILE_SB:
//					InitTimeNotification(0);
//					SetTimer(hWnd, 200, 10000, 0);
					if(do_suspend)
						do_suspend = 0;
					else
						do_suspend = 1;
					break;
				case ID_FILE_WATCHDOG:
				{
					void  *wd = CreateWatchDogTimer(0, 1000, 0, WDOG_RESET_DEVICE, 0, 0);
					StartWatchDogTimer(wd, 0);
					int err = GetLastError();
					break;
				}
				case ID_FILE_WRITEFILE:
				{
					static HANDLE hth = 0;

					if(hth)
					{
						if(WAIT_TIMEOUT == WaitForSingleObject(hth, 0))
							break;
						CloseHandle(hth);
						hth = 0;
					}
					hth = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SD_Write, 0, 0, 0);
					break;
				}
				case ID_FILE_COPYNAND:
				{
					static HANDLE hth = 0;

					if(hth)
					{
						if(WAIT_TIMEOUT == WaitForSingleObject(hth, 0))
							break;
						CloseHandle(hth);
						hth = 0;
					}
					hth = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)NAND_Write, 0, 0, 0);
					break;
				}
#endif // !SHELL_AYGSHELL
#ifdef WIN32_PLATFORM_WFSP
                case IDM_OK:
                    DestroyWindow(hWnd);
                    break;
#endif // WIN32_PLATFORM_WFSP
#ifdef WIN32_PLATFORM_PSPC
                case IDM_OK:
                    SendMessage (hWnd, WM_CLOSE, 0, 0);				
                    break;
#endif // WIN32_PLATFORM_PSPC
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_CREATE:
		{
			HFONT hFont = 0;
			LOGFONT logFont;

#ifndef SHELL_AYGSHELL
            g_hWndCommandBar = CommandBar_Create(g_hInst, hWnd, 1);
            CommandBar_InsertMenubar(g_hWndCommandBar, g_hInst, IDR_MENU, 0);
            CommandBar_AddAdornments(g_hWndCommandBar, 0, 0);
#endif // !SHELL_AYGSHELL
#ifdef SHELL_AYGSHELL
            SHMENUBARINFO mbi;

            memset(&mbi, 0, sizeof(SHMENUBARINFO));
            mbi.cbSize     = sizeof(SHMENUBARINFO);
            mbi.hwndParent = hWnd;
            mbi.nToolBarId = IDR_MENU;
            mbi.hInstRes   = g_hInst;

            if (!SHCreateMenuBar(&mbi)) 
            {
                g_hWndMenuBar = NULL;
            }
            else
            {
                g_hWndMenuBar = mbi.hwndMB;
            }

#ifndef WIN32_PLATFORM_WFSP
            // Initialize the shell activate info structure
            memset(&s_sai, 0, sizeof (s_sai));
            s_sai.cbSize = sizeof (s_sai);
#endif // !WIN32_PLATFORM_WFSP
#endif // SHELL_AYGSHELL

			memset(&logFont, 0, sizeof(logFont));

			_tcscpy (logFont.lfFaceName, L"Courier New");
			logFont.lfHeight = -20;

//			HDC hDC = GetDC(hWnd);
//			logFont.lfHeight = -(logFont.lfHeight * GetDeviceCaps(hDC, LOGPIXELSY) + 36) / 72;
//			ReleaseDC(hWnd, hDC);

			logFont.lfPitchAndFamily = FIXED_PITCH;
			logFont.lfCharSet = OEM_CHARSET;


			hFont = CreateFontIndirect(&logFont);
			SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

			hRTC = CreateFile(RTCDEV, 0, 0, 0, 0, 0, 0);
			if(hRTC != INVALID_HANDLE_VALUE)
			{
				DeviceIoControl(hRTC, IOCTL_RTC_GETTIME, 0, 0, &lt, sizeof(lt), 0, 0);
			}
			SetTimer(hWnd, 100, timer, 0);

			break;
		}
        case WM_PAINT:
		{
			RECT r;

            hdc = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &r);
            
            // TODO: Add any drawing code here...
			r.left	+= r.right/3;
			r.top	+= r.bottom/3;
			DrawText(hdc, tch, lstrlen(tch), &r,  DT_SINGLELINE | DT_TOP);
			r.top += 20;
			DrawText(hdc, set_notify, lstrlen(set_notify), &r,  DT_SINGLELINE | DT_TOP);
			r.top += 20;
			DrawText(hdc, got_notify, lstrlen(got_notify), &r,  DT_SINGLELINE | DT_TOP);
			r.top += 20;
			DrawText(hdc, tc_notify, lstrlen(tc_notify), &r,  DT_SINGLELINE | DT_TOP);
            EndPaint(hWnd, &ps);
            break;
		}
        case WM_DESTROY:
#ifndef SHELL_AYGSHELL
            CommandBar_Destroy(g_hWndCommandBar);
#endif // !SHELL_AYGSHELL
#ifdef SHELL_AYGSHELL
            CommandBar_Destroy(g_hWndMenuBar);
#endif // SHELL_AYGSHELL
			if(hRTC != INVALID_HANDLE_VALUE)
				CloseHandle(hRTC);
			g_Done = 1;
			KillTimer(hWnd, 100);
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

#ifndef WIN32_PLATFORM_WFSP
// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
#ifndef SHELL_AYGSHELL
            RECT rectChild, rectParent;
            int DlgWidth, DlgHeight;	// dialog width and height in pixel units
            int NewPosX, NewPosY;

            // trying to center the About dialog
            if (GetWindowRect(hDlg, &rectChild)) 
            {
                GetClientRect(GetParent(hDlg), &rectParent);
                DlgWidth	= rectChild.right - rectChild.left;
                DlgHeight	= rectChild.bottom - rectChild.top ;
                NewPosX		= (rectParent.right - rectParent.left - DlgWidth) / 2;
                NewPosY		= (rectParent.bottom - rectParent.top - DlgHeight) / 2;
				
                // if the About box is larger than the physical screen 
                if (NewPosX < 0) NewPosX = 0;
                if (NewPosY < 0) NewPosY = 0;
                SetWindowPos(hDlg, 0, NewPosX, NewPosY,
                    0, 0, SWP_NOZORDER | SWP_NOSIZE);
            }
#endif // !SHELL_AYGSHELL
#ifdef SHELL_AYGSHELL
            {
                // Create a Done button and size it.  
                SHINITDLGINFO shidi;
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
                shidi.hDlg = hDlg;
                SHInitDialog(&shidi);
            }
#endif // SHELL_AYGSHELL

            return (INT_PTR)TRUE;

        case WM_COMMAND:
#ifndef SHELL_AYGSHELL
            if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
#endif // !SHELL_AYGSHELL
#ifdef SHELL_AYGSHELL
            if (LOWORD(wParam) == IDOK)
#endif
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog(hDlg, message);
            return (INT_PTR)TRUE;

    }
    return (INT_PTR)FALSE;
}
#endif // !WIN32_PLATFORM_WFSP
