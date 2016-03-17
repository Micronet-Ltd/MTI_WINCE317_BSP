// key_map_test.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "key_map_test.h"
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
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KEY_MAP_TEST));
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
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KEY_MAP_TEST));
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
    LoadString(hInstance, IDC_KEY_MAP_TEST, szWindowClass, MAX_LOADSTRING);

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
	RECT r;
	static HMODULE lib;
	static pfnSwitchKeysMap pfn;
	static int clean = 0;
	static UINT32 down;
	static UINT32 up;
	static UINT32 ch;
	static TCHAR buff[128] = {0};

#if defined(SHELL_AYGSHELL) && !defined(WIN32_PLATFORM_WFSP)
    static SHACTIVATEINFO s_sai;
#endif // SHELL_AYGSHELL && !WIN32_PLATFORM_WFSP
	
    switch (message) 
    {
		case WM_KEYDOWN:
		{
			down = wParam;
			clean = 0;
			GetClientRect(hWnd, &r);
			InvalidateRect(hWnd, &r, 1);

			break;
		}
		case WM_CHAR:
		{
			ch = wParam;
			GetClientRect(hWnd, &r);
			InvalidateRect(hWnd, &r, 1);

			break;
		}
		case WM_KEYUP:
		{
			up = wParam;
			clean = 1;
			GetClientRect(hWnd, &r);
			InvalidateRect(hWnd, &r, 1);

			break;
		}
        case WM_COMMAND:
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            // Parse the menu selections:
            switch (wmId)
            {
				case IDM_SWITCH_KEY_MAP_DEFAULT:
				{
					int res;

					if(pfn)
					{
						res = pfn(KEYPAD_KEYS_MAP_DEFAULT);
						if(!res)
							res = GetLastError();
					}
					break;
				}
				case IDM_SWITCH_KEY_MAP_PEOPLENET:
				{
					int res;

					if(pfn)
					{
						res = pfn(KEYPAD_KEYS_MAP_EXT);
						if(!res)
							res = GetLastError();
					}
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
			lib = LoadLibrary(L"micusersdk.dll");
			if(lib)
				pfn = (pfnSwitchKeysMap)GetProcAddress(lib, L"MIC_SwitchKeysMap");
			int err;
			if(!pfn)
				err = GetLastError();
            break;
        case WM_PAINT:
		{
			TCHAR cdown[16] = {0};
			TCHAR cch[16] = {0};
			TCHAR cup[16] = {0};

			if((down >= 0x30 && down <= 0x39) || (down >= 0x40 && down <= 0x5A))
				cdown[0] = down;
			else if(down >= VK_SEMICOLON && down <= VK_BACKQUOTE)
				cdown[0] = ';' + down - VK_SEMICOLON;
			else if(down >= VK_LBRACKET && down <= VK_APOSTROPHE)
				cdown[0] = '[' + down - VK_LBRACKET;
			else if(down >= VK_NUMPAD0 && down <= VK_NUMPAD9)
				cdown[0] = 0x30 + down - VK_NUMPAD0;
			else if(down >= VK_MULTIPLY && down <= VK_DIVIDE)
				cdown[0] = '*' + down - VK_MULTIPLY;
			else if(down >= VK_F1 && down <= VK_F24)
				wsprintf(cdown, L"VK_F%d", down - VK_F1 + 1);
			else
			{
				switch(down)
				{
					case VK_BACK:
						wsprintf(cdown, L"VK_BACK");
						break;
					case VK_TAB:
						wsprintf(cdown, L"VK_TAB");
						break;
					case VK_CLEAR:
						wsprintf(cdown, L"VK_CLEAR");
						break;
					case VK_CONTROL:
						wsprintf(cdown, L"VK_CONTROL");
						break;
					case VK_SHIFT:
						wsprintf(cdown, L"VK_SHIFT");
						break;
					case VK_CAPITAL:
						wsprintf(cdown, L"VK_CAPITAL");
						break;
					case VK_SPACE:
						wsprintf(cdown, L"VK_SPACE");
						break;
					case VK_ESCAPE:
						wsprintf(cdown, L"VK_ESCAPE");
						break;
					case VK_RETURN:
						wsprintf(cdown, L"VK_RETURN");
						break;
					case VK_RIGHT:
						wsprintf(cdown, L"VK_RIGHT");
						break;
					case VK_LEFT:
						wsprintf(cdown, L"VK_LEFT");
						break;
					case VK_UP:
						wsprintf(cdown, L"VK_UP");
						break;
					case VK_DOWN:
						wsprintf(cdown, L"VK_DOWN");
						break;
					default:
						cdown[0] = 0;
				}
			}
			if(ch)
				cch[0] = ch;
			else
				cch[0] = 0;
/*
			if((ch >= 0x30 && ch <= 0x39) || (ch >= 0x40 && ch <= 0x5A))
				cch[0] = ch;
			else if(ch >= VK_SEMICOLON && ch <= VK_BACKQUOTE)
				cch[0] = ';' + ch - VK_SEMICOLON;
			else if(ch >= VK_LBRACKET && ch <= VK_APOSTROPHE)
				cch[0] = '[' + ch - VK_LBRACKET;
			else if(ch >= VK_NUMPAD0 && ch <= VK_NUMPAD9)
				cch[0] = 0x30 + ch - VK_NUMPAD0;
			else if(ch >= VK_MULTIPLY && ch <= VK_DIVIDE)
				cch[0] = '*' + ch - VK_MULTIPLY;
			else
			if(!ch)
			{
				ch = down;
				if(ch >= VK_F1 && ch <= VK_F24)
					wsprintf(cch, L"VK_F%d", ch - VK_F1 + 1);
				else
				{
					switch(ch)
					{
						case VK_BACK:
							wsprintf(cch, L"VK_BACK");
							break;
						case VK_TAB:
							wsprintf(cch, L"VK_TAB");
							break;
						case VK_CLEAR:
							wsprintf(cch, L"VK_CLEAR");
							break;
						case VK_CONTROL:
							wsprintf(cch, L"VK_CONTROL");
							break;
						case VK_SHIFT:
							wsprintf(cch, L"VK_SHIFT");
							break;
						case VK_CAPITAL:
							wsprintf(cch, L"VK_CAPITAL");
							break;
						case VK_SPACE:
							wsprintf(cch, L"VK_SPACE");
							break;
						case VK_ESCAPE:
							wsprintf(cch, L"VK_ESCAPE");
							break;
						case VK_RETURN:
							wsprintf(cch, L"VK_RETURN");
							break;
						case VK_RIGHT:
							wsprintf(cch, L"VK_RIGHT");
							break;
						case VK_LEFT:
							wsprintf(cch, L"VK_LEFT");
							break;
						case VK_UP:
							wsprintf(cch, L"VK_UP");
							break;
						case VK_DOWN:
							wsprintf(cch, L"VK_DOWN");
							break;
						default:
							//cch[0] = 0;
							cch[0] = ch;
					}
				}
			}
*/
			if((up >= 0x30 && up <= 0x39) || (up >= 0x40 && up <= 0x5A))
				cup[0] = up;
			else if(up >= VK_SEMICOLON && up <= VK_BACKQUOTE)
				cup[0] = ';' + up - VK_SEMICOLON;
			else if(up >= VK_LBRACKET && up <= VK_APOSTROPHE)
				cup[0] = '[' + up - VK_LBRACKET;
			else if(up >= VK_NUMPAD0 && up <= VK_NUMPAD9)
				cup[0] = 0x30 + up - VK_NUMPAD0;
			else if(up >= VK_MULTIPLY && up <= VK_DIVIDE)
				cup[0] = '*' + up - VK_MULTIPLY;
			else if(up >= VK_F1 && up <= VK_F24)
				wsprintf(cup, L"VK_F%d", up - VK_F1 + 1);
			else
			{
				switch(up)
				{
					case VK_BACK:
						wsprintf(cup, L"VK_BACK");
						break;
					case VK_TAB:
						wsprintf(cup, L"VK_TAB");
						break;
					case VK_CLEAR:
						wsprintf(cup, L"VK_CLEAR");
						break;
					case VK_CONTROL:
						wsprintf(cup, L"VK_CONTROL");
						break;
					case VK_SHIFT:
						wsprintf(cup, L"VK_SHIFT");
						break;
					case VK_CAPITAL:
						wsprintf(cup, L"VK_CAPITAL");
						break;
					case VK_SPACE:
						wsprintf(cup, L"VK_SPACE");
						break;
					case VK_ESCAPE:
						wsprintf(cup, L"VK_ESCAPE");
						break;
					case VK_RETURN:
						wsprintf(cup, L"VK_RETURN");
						break;
					case VK_RIGHT:
						wsprintf(cup, L"VK_RIGHT");
						break;
					case VK_LEFT:
						wsprintf(cup, L"VK_LEFT");
						break;
					case VK_UP:
						wsprintf(cup, L"VK_UP");
						break;
					case VK_DOWN:
						wsprintf(cup, L"VK_DOWN");
						break;
					default:
						cup[0] = 0;
				}
			}
			wsprintf(buff, L"WM_KEYDOWN:%s    WM_CHAR:%s    WM_KEYUP:%s", cdown, cch, cup);

			if(clean)
			{
				down = 0;
				ch = 0;
				up = 0;
			}
			GetClientRect(hWnd, &r);

			hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code here...
			DrawText(hdc, buff, _tcslen(buff), &r,  DT_CENTER | DT_VCENTER);

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
			FreeLibrary(lib);
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
