// CheckRAM.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "CheckRAM.h"


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE			g_hInst;			// current instance
HWND				g_hWndCommandBar;	// command bar handle

// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

#define MEM_SIZE (2*16777216) //16Mbyte
HWND g_wnd;
void CheckMemory()
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


	cMemory1 = Memory1;
	sMemory1 = (unsigned short *)Memory1;
	iMemory1 = (unsigned int *)Memory1;
	LMemory1 = (unsigned __int64 *)Memory1;
	cMemory2 = Memory2;
	sMemory2 = (unsigned short *)Memory2;
	iMemory2 = (unsigned int *)Memory2;
	LMemory2 = (unsigned __int64 *)Memory2;
    TickBegin = GetTickCount();
	do
	{
		*cMemory1++ = 0xA5;
		*cMemory2++ = 0x5A;
	}while(cMemory1 < cMemory1out);
	TickEnd = GetTickCount();
	Msec1Byte = TickEnd - TickBegin;
    TickBegin = GetTickCount();
	do
	{
		*sMemory1++ = 0xAA55;
		*sMemory2++ = 0x55AA;
	}while(sMemory1 < sMemory1out);
	TickEnd = GetTickCount();
	Msec2Byte = TickEnd - TickBegin;
    TickBegin = GetTickCount();
 	do
	{
		*iMemory1++ = 0xAAAA5555;
		*iMemory2++ = 0x5555AAAA;
	}while(iMemory1 < iMemory1out);
	TickEnd = GetTickCount();
	Msec4Byte = TickEnd - TickBegin;
    TickBegin = GetTickCount();
 	do
	{
		*LMemory1++ = 0xAAAAAAAA55555555;
		*LMemory2++ = 0x55555555AAAAAAAA;
	}while(LMemory1 < LMemory1out);
	TickEnd = GetTickCount();
	Msec8Byte = TickEnd - TickBegin;

	result = VirtualFree(cMemory1, MEM_SIZE, MEM_DECOMMIT);
	result = VirtualFree(cMemory2, MEM_SIZE, MEM_DECOMMIT);

	result = CeSetThreadPriority(GetCurrentThread(), InitialPriority);

	TCHAR Message[4096];

	_stprintf(Message, _T(" Filling by portion 1 byte - %dmsec\n\
						 Filling by portion 2 byte - %dmsec\n\
						 Filling by portion 4 byte - %dmsec\n\
						 Fillingn by portion 8 byte - %dmsec"),Msec1Byte1, Msec2Byte1, Msec4Byte1, Msec8Byte1); 
	MessageBox(g_wnd, Message, _T("Checking RAM 1 Buffer"),MB_OK); 
	_stprintf(Message, _T(" Filling by portion 1 byte - %dmsec\n\
						 Filling by portion 2 byte - %dmsec\n\
						 Filling by portion 4 byte - %dmsec\n\
						 Filling by portion 8 byte - %dmsec"),Msec1Byte, Msec2Byte, Msec4Byte, Msec8Byte); 
	MessageBox(g_wnd, Message, _T("Checking RAM 2 Buffers"),MB_OK); 
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

	CheckMemory();

	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CHECKRAM));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
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
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHECKRAM));
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


    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
    LoadString(hInstance, IDC_CHECKRAM, szWindowClass, MAX_LOADSTRING);


    if (!MyRegisterClass(hInstance, szWindowClass))
    {
    	return FALSE;
    }

    g_wnd = hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }


    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    if (g_hWndCommandBar)
    {
        CommandBar_Show(g_hWndCommandBar, TRUE);
    }

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

	
    switch (message) 
    {
        case WM_COMMAND:
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_HELP_ABOUT:
                    DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, About);
                    break;
                case IDM_FILE_EXIT:
                    DestroyWindow(hWnd);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_CREATE:
            g_hWndCommandBar = CommandBar_Create(g_hInst, hWnd, 1);
            CommandBar_InsertMenubar(g_hWndCommandBar, g_hInst, IDR_MENU, 0);
            CommandBar_AddAdornments(g_hWndCommandBar, 0, 0);
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            
            // TODO: Add any drawing code here...
            
            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:
            CommandBar_Destroy(g_hWndCommandBar);
            PostQuitMessage(0);
            break;


        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
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

        case WM_COMMAND:
            if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog(hDlg, message);
            return TRUE;

    }
    return (INT_PTR)FALSE;
}

