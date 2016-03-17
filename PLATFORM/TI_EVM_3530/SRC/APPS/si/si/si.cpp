// si.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "si.h"
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
/*
	static LPCWSTR g_oalIoCtlPlatformOEM  = L"Micronet CE-50X";
	unsigned short lcd = '4';
	unsigned short *p = (unsigned short *)g_oalIoCtlPlatformOEM;
	for(;*p;p++)
		;
	p--;
	*p = 0034;
*/
	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}

#ifndef WIN32_PLATFORM_WFSP
	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SI));
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
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SI));
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
    LoadString(hInstance, IDC_SI, szWindowClass, MAX_LOADSTRING);

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
/*
typedef struct _PROCESSOR_INFO {
    WORD    wVersion;
    WCHAR   szProcessCore[40];
    WORD    wCoreRevision;
    WCHAR   szProcessorName[40];
    WORD    wProcessorRevision;
    WCHAR   szCatalogNumber[100];
    WCHAR   szVendor[100];
    DWORD   dwInstructionSet;
    DWORD   dwClockSpeed;
} PROCESSOR_INFO;
#define IOCTL_PROCESSOR_INFORMATION             CTL_CODE(FILE_DEVICE_HAL, 25, METHOD_BUFFERED, FILE_ANY_ACCESS)
extern "C" BOOL KernelIoControl(DWORD dwIoControlCode, LPVOID lpInBuf, DWORD nInBufSize, LPVOID lpOutBuf, DWORD nOutBufSize, LPDWORD lpBytesReturned);
*/
void Getinfo(HWND hDlg)
{
	wchar_t oemInfo1[128];
	wchar_t oemInfo2[128];
	wchar_t oemInfo3[128];
	wchar_t oemInfo4[128];
	wchar_t oemInfo5[128];
	wchar_t oemInfo6[128];
	wchar_t oemInfo7[128];
	wchar_t oemInfo8[128];
	wchar_t oemInfo9[128];
	wchar_t oemInfo10[128];
	wchar_t oemInfo11[128];
	RECT r;
	DWORD bat_ito;
	GUID guid, guid2;
	int result1 = SystemParametersInfo(SPI_GETGUIDPATTERN, sizeof(guid2), &guid2, 0);
	int result2 = SystemParametersInfo(SPI_GETBOOTMENAME, sizeof(oemInfo2), oemInfo2, 0);
	int result3 = SystemParametersInfo(SPI_GETOEMINFO, sizeof(oemInfo3), oemInfo3, 0);
	int result4 = SystemParametersInfo(SPI_GETPLATFORMMANUFACTURER, sizeof(oemInfo4), oemInfo4, 0);
	int result5 = SystemParametersInfo(SPI_GETPLATFORMTYPE, sizeof(oemInfo5), oemInfo5, 0);
	int result6 = SystemParametersInfo(SPI_GETPLATFORMNAME, sizeof(oemInfo6), oemInfo6, 0);
	int result7 = SystemParametersInfo(SPI_GETUUID, sizeof(guid), &guid, 0);
	int result8 = SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
	int result9 = SystemParametersInfo(SPI_GETEXTERNALIDLETIMEOUT, 0, &bat_ito, 0);
	

	if(result1)
	{
		wsprintf(oemInfo1, L"GUID PATERN: %08x, %04x, %04x, {%x, %x, %x, %x, %x, %x, %x, %x}",
			guid2.Data1, guid2.Data2, guid2.Data3, guid2.Data4[0], guid2.Data4[1], guid2.Data4[2], guid2.Data4[3], guid2.Data4[4],
			guid2.Data4[5], guid2.Data4[6], guid2.Data4[7]);
		SetDlgItemText(hDlg, IDC_STATIC1, oemInfo1);
	}
	if(result2)
	{
		wsprintf(oemInfo9, L"BOOTME: %s", oemInfo2);
		SetDlgItemText(hDlg, IDC_STATIC2, oemInfo9);
	}
	if(result3)
	{
		wsprintf(oemInfo9, L"OEM: %s", oemInfo3);
		SetDlgItemText(hDlg, IDC_STATIC3, oemInfo9);
	}
	if(result4)
		wsprintf(oemInfo9, L"Platform manufacturer: %s", oemInfo4);
	else
		wsprintf(oemInfo9, L"Platform manufacturer: unknown (%d)", GetLastError());
	SetDlgItemText(hDlg, IDC_STATIC4, oemInfo9);
	if(result5)
	{
		wsprintf(oemInfo9, L"Platform type: %s", oemInfo5);
		SetDlgItemText(hDlg, IDC_STATIC5, oemInfo9);
	}
	if(result6)
	{
		wsprintf(oemInfo9, L"Platform name: %s", oemInfo6);
		SetDlgItemText(hDlg, IDC_STATIC6, oemInfo9);
	}
	if(result7)
	{
		wsprintf(oemInfo7, L"GUID: %08x, %04x, %04x, {%x, %x, %x, %x, %x, %x, %x, %x}",
			guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4],
			guid.Data4[5], guid.Data4[6], guid.Data4[7]);
		SetDlgItemText(hDlg, IDC_STATIC7, oemInfo7);
	}
	if(result8)
	{
		wsprintf(oemInfo8, L"WA: %d, %d, %d, %d", r.top, r.left, r.right, r.bottom);
		SetDlgItemText(hDlg, IDC_STATIC8, oemInfo8);
	}
	if(result9)
	{
		wsprintf(oemInfo9, L"Battery idle timeout: %d", bat_ito);
		SetDlgItemText(hDlg, IDC_STATIC9, oemInfo9);
	}

	SYSTEM_INFO si;
	TCHAR *arch[] = {L"Intel", L"MIPS", L"Alpha", L"PPC", L"SHX", L"ARM", L"IA64", L"Alpha64", L"Unknownn"};

	GetSystemInfo(&si);
	wsprintf(oemInfo10, L"Architecture: %s", arch[si.wProcessorArchitecture]);
	SetDlgItemText(hDlg, IDC_STATIC10, oemInfo10);
	//PROCESSOR_ARCHITECTURE_INTEL 
	//PROCESSOR_INTEL_386
	PROCESSOR_INFO pi;
	KernelIoControl(IOCTL_PROCESSOR_INFORMATION, 0, 0, &pi, sizeof(pi), (DWORD *)&result9);

	wsprintf(oemInfo10, L"Name: %s", pi.szProcessorName);
	SetDlgItemText(hDlg, IDC_STATIC11, oemInfo10);
	wsprintf(oemInfo10, L"Core: %s", pi.szProcessCore);
	SetDlgItemText(hDlg, IDC_STATIC12, oemInfo10);
	wsprintf(oemInfo10, L"Vendor: %s", pi.szVendor);
	SetDlgItemText(hDlg, IDC_STATIC13, oemInfo10);
	wsprintf(oemInfo10, L"MPU Clock: %d MHz", pi.dwClockSpeed);
	SetDlgItemText(hDlg, IDC_STATIC14, oemInfo10);

}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
			Getinfo(hDlg);
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
