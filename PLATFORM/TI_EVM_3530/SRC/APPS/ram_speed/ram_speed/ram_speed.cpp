// ram_speed.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ram_speed.h"
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
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RAM_SPEED));
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
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RAM_SPEED));
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
    LoadString(hInstance, IDC_RAM_SPEED, szWindowClass, MAX_LOADSTRING);

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

#define MEM_SIZE (2*16777216) //16Mbyte
HWND g_wnd;
void CheckMemory(HWND hwnd)
{
	int InitialPriority = CeGetThreadPriority(GetCurrentThread());
	BOOL result = CeSetThreadPriority(GetCurrentThread(), 101);

	unsigned char *Memory1;
	unsigned char *cMemory1;
	unsigned short *sMemory1;
	unsigned int *iMemory1;
	unsigned __int64 *LMemory1;
	unsigned char *Memory2;
	unsigned char *cMemory2;
	unsigned short *sMemory2;
	unsigned int *iMemory2;
	unsigned __int64 *LMemory2;

	unsigned int TickBegin;
	unsigned int TickEnd;
	unsigned int Msec1Byte1;
	unsigned int Msec2Byte1;
	unsigned int Msec4Byte1;
	unsigned int Msec8Byte1;
	unsigned int Msec1Byte;
	unsigned int Msec2Byte;
	unsigned int Msec4Byte;
	unsigned int Msec8Byte;

	Memory1 = (unsigned char *)VirtualAlloc( NULL, MEM_SIZE, MEM_COMMIT, PAGE_READWRITE );
	Memory2 = (unsigned char *)VirtualAlloc( NULL, MEM_SIZE, MEM_COMMIT, PAGE_READWRITE );
	
	cMemory1 = Memory1;
	sMemory1 = (unsigned short *)Memory1;
	iMemory1 = (unsigned int *)Memory1;
	LMemory1 = (unsigned __int64 *)Memory1;

	unsigned char *cMemory1out = cMemory1 + MEM_SIZE;
	unsigned short *sMemory1out = sMemory1 + MEM_SIZE / 2;
	unsigned int *iMemory1out = iMemory1 + MEM_SIZE / 4;
	unsigned __int64 *LMemory1out = LMemory1 + MEM_SIZE / 8;

	TickBegin = GetTickCount();
	do
	{
		*cMemory1++ = 0xA5;
	}while(cMemory1 < cMemory1out);
	TickEnd = GetTickCount();
	Msec1Byte1 = TickEnd - TickBegin;
    TickBegin = GetTickCount();
	do
	{
		*sMemory1++ = 0xAA55;
	}while(sMemory1 < sMemory1out);
	TickEnd = GetTickCount();
	Msec2Byte1 = TickEnd - TickBegin;
    TickBegin = GetTickCount();
 	do
	{
		*iMemory1++ = 0xAAAA5555;
	}while(iMemory1 < iMemory1out);
	TickEnd = GetTickCount();
	Msec4Byte1 = TickEnd - TickBegin;
    TickBegin = GetTickCount();
 	do
	{
		*LMemory1++ = 0xAAAAAAAA55555555;
	}while(LMemory1 < LMemory1out);
	TickEnd = GetTickCount();
	Msec8Byte1 = TickEnd - TickBegin;
	result = VirtualFree(Memory1, 0, MEM_RELEASE);
	result = VirtualFree(Memory2, 0, MEM_RELEASE);



	Memory1 = (unsigned char *)VirtualAlloc( NULL, MEM_SIZE, MEM_COMMIT, PAGE_READWRITE | PAGE_NOCACHE);
	Memory2 = (unsigned char *)VirtualAlloc( NULL, MEM_SIZE, MEM_COMMIT, PAGE_READWRITE | PAGE_NOCACHE);

	cMemory1 = Memory1;
	sMemory1 = (unsigned short *)Memory1;
	iMemory1 = (unsigned int *)Memory1;
	LMemory1 = (unsigned __int64 *)Memory1;
	cMemory2 = Memory2;
	sMemory2 = (unsigned short *)Memory2;
	iMemory2 = (unsigned int *)Memory2;
	LMemory2 = (unsigned __int64 *)Memory2;

	cMemory1out = cMemory1 + MEM_SIZE;
	sMemory1out = sMemory1 + MEM_SIZE / 2;
	iMemory1out = iMemory1 + MEM_SIZE / 4;
	LMemory1out = LMemory1 + MEM_SIZE / 8;


    TickBegin = GetTickCount();
	do
	{
		*cMemory1++ = 0xA5;
//		*cMemory2++ = 0x5A;
	}while(cMemory1 < cMemory1out);
	TickEnd = GetTickCount();
	Msec1Byte = TickEnd - TickBegin;
    TickBegin = GetTickCount();
	do
	{
		*sMemory1++ = 0xAA55;
//		*sMemory2++ = 0x55AA;
	}while(sMemory1 < sMemory1out);
	TickEnd = GetTickCount();
	Msec2Byte = TickEnd - TickBegin;
    TickBegin = GetTickCount();
 	do
	{
		*iMemory1++ = 0xAAAA5555;
//		*iMemory2++ = 0x5555AAAA;
	}while(iMemory1 < iMemory1out);
	TickEnd = GetTickCount();
	Msec4Byte = TickEnd - TickBegin;
    TickBegin = GetTickCount();
 	do
	{
		*LMemory1++ = 0xAAAAAAAA55555555;
//		*LMemory2++ = 0x55555555AAAAAAAA;
	}while(LMemory1 < LMemory1out);
	TickEnd = GetTickCount();
	Msec8Byte = TickEnd - TickBegin;

	result = VirtualFree(Memory1, 0, MEM_RELEASE);
	result = VirtualFree(Memory2, 0, MEM_RELEASE);

	result = CeSetThreadPriority(GetCurrentThread(), InitialPriority);

	TCHAR Message[4096];
	TCHAR Cap[4096];

	wsprintf(Cap, L"Cached write of %d MB buffer", MEM_SIZE);
	wsprintf(Message, _T(" 8-bit access write: %d msec \
						 16-bit access write: %d msec \
						 32-bit access write: %d msec \
						 64-bit access write: %d msec"),Msec1Byte1, Msec2Byte1, Msec4Byte1, Msec8Byte1); 
	MessageBox(hwnd, Message, Cap,MB_OK); 
	wsprintf(Message, _T(" 8-bit access write: %d msec \
						 16-bit access write: %d msec \
						 32-bit access write: %d msec \
						 64-bit access write: %d msec"),Msec1Byte, Msec2Byte, Msec4Byte, Msec8Byte); 
	wsprintf(Cap, L"Unached write of %d MB buffer", MEM_SIZE);
	MessageBox(hwnd, Message, Cap,MB_OK); 
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
				case ID_FILE_RAMSPEED:
				{
					CheckMemory(hWnd);
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
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            
            // TODO: Add any drawing code here...
            
            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:
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
