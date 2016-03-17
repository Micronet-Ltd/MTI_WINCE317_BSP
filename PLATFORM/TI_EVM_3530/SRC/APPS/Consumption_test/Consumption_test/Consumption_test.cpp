// Consumption_test.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Consumption_test.h"
#include <windows.h>
#include <commctrl.h>

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

TCHAR notify[256] = L"Alarm isn't set";
TCHAR alarm[256] = {0};
TCHAR ltime[256] = {0};
SYSTEMTIME atime = {0};

HANDLE InitTimeBasedNotification(HANDLE hNotification, SYSTEMTIME *st)
{
	CE_NOTIFICATION_TRIGGER nt = {0};
	CE_USER_NOTIFICATION un = {0};

	nt.dwSize = sizeof(CE_NOTIFICATION_TRIGGER);
	nt.dwType = CNT_TIME;
	nt.lpszApplication = TEXT("\\\\.\\Notifications\\NamedEvents\\_____Update_Time");
	nt.lpszArguments = 0;
	memcpy(&nt.stStartTime, st, sizeof(SYSTEMTIME));
	memcpy(&nt.stEndTime, st, sizeof(SYSTEMTIME));
	wsprintf(notify, L"Alarm set to: %d/%d/%d %d:%d:%d", nt.stStartTime.wYear, nt.stStartTime.wMonth, nt.stStartTime.wDay,
								nt.stStartTime.wHour, nt.stStartTime.wMinute, nt.stStartTime.wSecond);
	//OutputDebugString(notify);

	return CeSetUserNotificationEx(hNotification,&nt,&un);
}

ULONG WINAPI safe_boot(void *arg)
{
	void *vp = OpenWatchDogTimer(L"Safe reboot", 0);

	WaitForSingleObject(vp, INFINITE);
	SetSystemPowerState(0, POWER_STATE_RESET, POWER_FORCE);

	CloseHandle(vp);
	return 0;
}

ULONG WINAPI busy_loop(void *arg)
{
	int quit = *(int *)arg;
	int i = 0;
	TCHAR buf[128];

	wsprintf(buf, L"busy_loop(%d)", quit);
	OutputDebugString(buf);

	do
	{
		quit = *(int *)arg;
		if(i == 16*1024*1024) //268435455)
		{
			Sleep(0);
			i = 0;
		}
		i++;
	}while(!quit);

	wsprintf(buf, L"busy_loop(%d)", quit);
	OutputDebugString(buf);

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
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CONSUMPTION_TEST));
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
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CONSUMPTION_TEST));
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
    LoadString(hInstance, IDC_CONSUMPTION_TEST, szWindowClass, MAX_LOADSTRING);

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

INT_PTR CALLBACK setalarm(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
		{
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
            return (INT_PTR)TRUE;
		}
        case WM_COMMAND:
            if((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
            {
				if(LOWORD(wParam) == IDOK)
				{
					SYSTEMTIME t;
					DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_DATE_ALARM),(LPARAM)&t);
					atime.wYear = t.wYear;
					atime.wMonth = t.wMonth;
					atime.wDay = t.wDay;
					DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_TIME_ALARM),(LPARAM)&t);
					atime.wHour = t.wHour;
					atime.wMinute = t.wMinute;
					atime.wSecond = 20*t.wSecond/20;
				}
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
	static void *vp = 0;
	static int quit = 0;

#if defined(SHELL_AYGSHELL) && !defined(WIN32_PLATFORM_WFSP)
    static SHACTIVATEINFO s_sai;
#endif // SHELL_AYGSHELL && !WIN32_PLATFORM_WFSP
	
    switch (message) 
    {
        case WM_COMMAND:
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            // Parse the menu selections:
            switch (wmId)
            {
				case ID_FILE_ON_PS:
				{
					SetSystemPowerState(0, POWER_STATE_ON, POWER_FORCE);
					break;
				}
				case ID_FILE_IDLE_PS:
				{
					SetSystemPowerState(0, POWER_STATE_USERIDLE, POWER_FORCE);
					break;
				}
				case ID_FILE_SB_PS:
				{
					int ret;
					ret = SetSystemPowerState(0, POWER_STATE_USERIDLE, POWER_FORCE);
					Sleep(1000);
					ret = SetSystemPowerState(0, POWER_STATE_IDLE, POWER_FORCE);
//					ret = SetSystemPowerState(L"systemidle", POWER_STATE_IDLE, POWER_NAME | POWER_FORCE);
					ret = GetLastError();
					break;
				}
				case ID_FILE_SUSPEND_PS:
				{
					SetSystemPowerState(0, POWER_STATE_SUSPEND, POWER_FORCE);
					break;
				}
				case ID_FILE_REBOOT_PS:
				{
					SetSystemPowerState(0, POWER_STATE_RESET, POWER_FORCE);
					break;
				}
				case ID_FILE_OFF_PS:
				{
					SetSystemPowerState(0, POWER_STATE_OFF, POWER_FORCE);
					break;
				}
				case ID_WATCHDOG_SIMULATE:
				{
					CeSetThreadPriority(GetCurrentThread(), 50);
					while(1)
					{
						while(1);
					}
					break;
				}
				case ID_WATCHDOG_UR:
				{
					void *vp = CreateWatchDogTimer(L"Unsafe reboot", 4000, 0, WDOG_RESET_DEVICE, 0, 0);
					StartWatchDogTimer(vp, 0);
					break;
				}
				case ID_WATCHDOG_TSR:
				{
					void *vp = CreateWatchDogTimer(L"Safe reboot", 4000, 2000, WDOG_RESET_DEVICE, 0, 0);
					StartWatchDogTimer(vp, 0);
					CloseHandle(CreateThread(0, 0, safe_boot, 0, 0, 0));
					break;
				}
				case ID_WATCHDOG_SR:
				{
					void *vp = CreateWatchDogTimer(L"Safe reboot", 4000, 2000, WDOG_NO_DFLT_ACTION, 0, 0);
					StartWatchDogTimer(vp, 0);
					CloseHandle(CreateThread(0, 0, safe_boot, 0, 0, 0));
					break;
				}
				case ID_FILE_SETALARM:
				{
                    int ret = DialogBox(g_hInst, (LPCTSTR)IDD_DIALOG_TIME, hWnd, setalarm);
					if(IDOK == ret)
						ret = (int)InitTimeBasedNotification(0, &atime);
					break;
				}
#ifndef WIN32_PLATFORM_WFSP
                case IDM_HELP_ABOUT:
                    DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, About);
                    break;
#endif // !WIN32_PLATFORM_WFSP
#ifndef SHELL_AYGSHELL
                case IDM_FILE_EXIT:
                    DestroyWindow(hWnd);
                    break;
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
		case WM_TIMER:
		{
			SYSTEMTIME lt;

			GetLocalTime(&lt);

			wsprintf(ltime, L"Current time: %d/%d/%d  %d:%d:%d", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond);
			RECT r;
			GetClientRect(hWnd, &r);

			InvalidateRect(hWnd, &r, 1);

			break;
		}
        case WM_CREATE:
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
			vp = CreateThread(0, 0, busy_loop, &quit, 0, 0);

			SYSTEMTIME lt;
			INITCOMMONCONTROLSEX ccex;

			ccex.dwSize = sizeof(ccex);
			ccex.dwICC = ICC_DATE_CLASSES;

			InitCommonControlsEx(&ccex);
			GetLocalTime(&lt);

			wsprintf(ltime, L"Current time: %d/%d/%d  %d:%d:%d", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond);
			SetTimer(hWnd, 100, 1000, 0);
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            
            // TODO: Add any drawing code here...
			RECT r;

			GetClientRect(hWnd, &r);
            
            // TODO: Add any drawing code here...
			r.left	+= r.right/3;
			r.top	+= r.bottom/3;
			DrawText(hdc, ltime, lstrlen(ltime), &r,  DT_SINGLELINE | DT_TOP);
			r.top += 20;
			DrawText(hdc, notify, lstrlen(notify), &r,  DT_SINGLELINE | DT_TOP);
			r.top += 20;
			DrawText(hdc, alarm, lstrlen(alarm), &r,  DT_SINGLELINE | DT_TOP);
            
            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:
#ifndef SHELL_AYGSHELL
            CommandBar_Destroy(g_hWndCommandBar);
#endif // !SHELL_AYGSHELL
#ifdef SHELL_AYGSHELL
            CommandBar_Destroy(g_hWndMenuBar);
#endif // SHELL_AYGSHELL
			quit = 1;
			KillTimer(hWnd, 100);
			WaitForSingleObject(vp, INFINITE);
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
