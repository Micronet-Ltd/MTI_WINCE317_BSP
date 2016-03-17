// get_notif.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "get_notif.h"
#include <windows.h>
#include <commctrl.h>
#include "\WINCE600\PUBLIC\COMMON\OAK\INC\pwinuser.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE			g_hInst;			// current instance
#ifdef SHELL_AYGSHELL
HWND				g_hWndMenuBar;		// menu bar handle
#else // SHELL_AYGSHELL
HWND				g_hWndCommandBar;	// command bar handle
#endif // SHELL_AYGSHELL

HWND g_hWnd = 0;
int g_Done = 0;

// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
#ifndef WIN32_PLATFORM_WFSP
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
#endif // !WIN32_PLATFORM_WFSP

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

	nt.dwSize = sizeof(CE_NOTIFICATION_TRIGGER);
	nt.dwType = CNT_EVENT;
	nt.dwEvent = NOTIFICATION_EVENT_WAKEUP;
	nt.lpszApplication = TEXT("\\\\.\\Notifications\\NamedEvents\\Platform_resumed");
	nt.lpszArguments = 0;

	int err = CeRunAppAtEvent(nt.lpszApplication, NOTIFICATION_EVENT_WAKEUP);
	if(!err)
		err = GetLastError();
	return (HANDLE)err;//CeSetUserNotificationEx(hNotification,&nt,&un);
}

DWORD WINAPI PSThread(PVOID pvParam)
{
    HANDLE hNotify = 0, hNotify2 = 0;
	CE_NOTIFICATION_TRIGGER nt = {0};
	CE_USER_NOTIFICATION un = {0};
	int err;

	HANDLE hAct = CreateEvent(0,0,0,TEXT("Platform_resumed"));
	if(!hAct)
		return 0;

//	hNotify2 = InitTimeNotification(hNotify2);
	hNotify = InitPowerNotification(0);
	if(!hNotify)
	{
		err = GetLastError();
	}

	while(!g_Done)
	{
		DWORD dwStatus = WaitForSingleObject(hAct, 1000);
		switch(dwStatus)
		{
			case WAIT_TIMEOUT:
			default:
			{
				break;
			}
			case (WAIT_OBJECT_0 + 0):
			{
				if(g_hWnd)
				{
					PostMessage(g_hWnd, WM_POWERBROADCAST, PBT_APMRESUMESUSPEND, 0);
				}
				break;
			}
		}
	}

//	if(hNotify)
//		CeClearUserNotification(hNotify);

	CeRunAppAtEvent(TEXT("\\\\.\\Notifications\\NamedEvents\\Platform_resumed"), NOTIFICATION_EVENT_NONE);
	CloseHandle(hAct);

	return 1;
}

void kill_p(TCHAR *tc)
{
	UINT32 res;
	PROCESSENTRY32 pe = {0};
	void *ssh = CreateToolhelp32Snapshot(TH32CS_GETALLMODS | TH32CS_SNAPALL, GetCurrentProcessId());

	if((void *)-1 != ssh)
	{
		pe.dwSize = sizeof(PROCESSENTRY32);
		__try
		{
			res = Process32First(ssh, &pe);
		}
		__except(1)
		{
		}

		while(res)
		{
/*			if(0==_tcsncmp(pe.szExeFile, tc, _tcslen(tc)*sizeof(tc[0])))
			{
				void *hp = OpenProcess(0, 0, pe.th32ProcessID);
				if(hp)
				{
					DWORD err;
					WaitForSingleObject(hp, 1000);
					GetExitCodeProcess(hp, &err);
					if(err == STILL_ACTIVE)
					{
						TerminateProcess(hp, 0);
					}
					CloseHandle(hp);
					break;
				}
			}
*/
			__try
			{
				res = Process32Next(ssh, &pe);
			}
			__except(1)
			{
			}
		}

		CloseToolhelp32Snapshot(ssh);
	}
}
void enum_notifications(HWND list)
{
	HANDLE hn;
	DWORD dwHowMany;
	DWORD *buff = 0;
	TCHAR notify[512] = {0};

	if(list)
		SendMessage(list, LB_RESETCONTENT, 0, 0);
	CeGetUserNotificationHandles (0, 0, &dwHowMany);
	buff = (DWORD *)new char[dwHowMany*sizeof(HANDLE)];

	CeGetUserNotificationHandles ((HANDLE *)buff, dwHowMany, &dwHowMany);
	for(int i=0; i < dwHowMany; i++)
	{
		DWORD *buf = 0;
		DWORD need;
		TCHAR line[1024], *pl;
		hn = (HANDLE)buff[i];
		int nRes = CeGetUserNotification(hn, 0, &need, 0);
		if(1)
		{
			buf = (DWORD *)new char[need];
			if(buf)
			{
				nRes = CeGetUserNotification(hn, need, &need, (unsigned char *)buf);
				CE_NOTIFICATION_INFO_HEADER *head;
				head = (CE_NOTIFICATION_INFO_HEADER *)buf;
				RETAILMSG(1,(L"st %d ty %d ev %d %s\r\n%d/%d/%d  %d:%d:%d\r\n", 
					head->dwStatus, 
					head->pcent->dwType,
					head->pcent->dwEvent,
					(head->pcent->lpszApplication)?head->pcent->lpszApplication:L" ", 
					head->pcent->stStartTime.wDay,
					head->pcent->stStartTime.wMonth,
					head->pcent->stStartTime.wYear,
					head->pcent->stStartTime.wHour,
					head->pcent->stStartTime.wMinute,
					head->pcent->stStartTime.wSecond
				));
				pl = line;
				need = wsprintf(pl, L"%s(%d)",
									(head->pcent->dwType == CNT_EVENT)?L"Event":
									(head->pcent->dwType == CNT_TIME)?L"Time based":L"another",
									head->dwStatus);

				if(head->pcent->dwType == CNT_EVENT)
				{
					pl += need;
					need = wsprintf(pl, L" %d(%s)", head->pcent->dwEvent,
												   (head->pcent->lpszArguments)?head->pcent->lpszArguments:L" ");
				}
				if(head->pcent->dwType == CNT_TIME)
				{
					pl += need;
					need = wsprintf(pl, L" %d/%d/%d  %d:%d:%d",
										head->pcent->stStartTime.wDay,
										head->pcent->stStartTime.wMonth,
										head->pcent->stStartTime.wYear,
										head->pcent->stStartTime.wHour,
										head->pcent->stStartTime.wMinute,
										head->pcent->stStartTime.wSecond);
				}

				pl += need;
				need = wsprintf(pl, L"%s", (head->pcent->lpszApplication)?head->pcent->lpszApplication:L" ");
				if(list)
					SendMessage(list, LB_ADDSTRING, 0, (LPARAM)line);
				delete[] buf;
			}
		}
		//CeClearUserNotification(hn);
	}

	delete[] buff;
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
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GET_NOTIF));
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
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GET_NOTIF));
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
    LoadString(hInstance, IDC_GET_NOTIF, szWindowClass, MAX_LOADSTRING);

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
	static HWND hlist = 0;
	static HANDLE hTh = 0;

#if defined(SHELL_AYGSHELL) && !defined(WIN32_PLATFORM_WFSP)
    static SHACTIVATEINFO s_sai;
#endif // SHELL_AYGSHELL && !WIN32_PLATFORM_WFSP
	
    switch (message) 
    {
		case WM_POWERBROADCAST:
		{
			switch ((DWORD) wParam)
			{
				case PBT_APMSUSPEND:
				{
					OutputDebugString(_T("PBT_APMSUSPEND\r\n"));
				}
				break;				
				case PBT_APMRESUMECRITICAL:
				{
					OutputDebugString(_T("PBT_APMRESUMECRITICAL\r\n"));
				}
				break;				
				case PBT_APMRESUMESUSPEND:
				{
					OutputDebugString(_T("PBT_APMRESUMESUSPEND\r\n"));
					enum_notifications(hlist);
				}
				break;				
			}
		}
		break;
       case WM_COMMAND:
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            // Parse the menu selections:
            switch (wmId)
            {
				case ID_FILE_SHOWNOTIFICATIONS:
				{
					enum_notifications(hlist);
					break;
				}
				case ID_FILE_SUSPEND:
				{
					kill_p(L"repllog.exe");
					GwesPowerOffSystem();
					//SetSystemPowerState(0, POWER_STATE_SUSPEND, POWER_FORCE);
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
        case WM_CREATE:
		{
			g_hWnd = hWnd;
			g_Done = 0;
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
			RECT r;
			GetWindowRect(hWnd, &r);
			r.top +=  GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYBORDER); //GetSystemMetrics(SM_CYMENU) +
//			r.left += GetSystemMetrics(SM_CXBORDER);

			hlist = CreateWindow(L"Listbox", 0, LBS_STANDARD | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | WS_TABSTOP,
								 r.left, r.top, r.right - r.left/* - GetSystemMetrics(SM_CXBORDER)*/,
								 r.bottom - r.top/* - GetSystemMetrics(SM_CYBORDER)*/,
								 hWnd,(HMENU)0, g_hInst, 0);
			hTh = CreateThread(0, 0, PSThread, 0, 0, 0);

			enum_notifications(hlist);
            break;
		}
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            
            // TODO: Add any drawing code here...
            
            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:
			g_Done = 1;
			WaitForSingleObject(hTh, INFINITE);
			CloseHandle(hTh);
			if(hlist)
				DestroyWindow(hlist);
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
