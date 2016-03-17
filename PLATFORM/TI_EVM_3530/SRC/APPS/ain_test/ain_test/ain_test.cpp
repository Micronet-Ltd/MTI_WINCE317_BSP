// ain_test.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ain_test.h"
#include <windows.h>
#include <commctrl.h>

#include <MicUserSdk.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE			g_hInst;			// current instance
HWND				g_hWndCommandBar;	// command bar handle

HWND                g_hWnd;
HANDLE              g_hAin;

// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

DWORD WINAPI RunnerThread(LPVOID lpParam);


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

	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_AIN_TEST));


	HANDLE hThr = CreateThread(0, 0, RunnerThread, NULL, 0, 0 );
	
	if( !hThr )
	{
		//DebugPrint("Faild GetInitParam thread creation");
	}
	else
	{
		CloseHandle( hThr );
		hThr = 0;
	}

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
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AIN_TEST));
	wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
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
    LoadString(hInstance, IDC_AIN_TEST, szWindowClass, MAX_LOADSTRING);


    if (!MyRegisterClass(hInstance, szWindowClass))
    {
    	return FALSE;
    }

    hWnd = CreateWindow(szWindowClass, szTitle, /*WS_VISIBLE | WS_SIZEBOX */ WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX ,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

	g_hWnd = hWnd;

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
	int bReturn;
	RECT rect;
    int idTimer = -1;
	static int count = 0;
	TCHAR  sBuff[128];
    // font values definition
    LOGFONT lf =  {0};
    static HFONT hf;
    static HANDLE  hAin;
    BOOL   ret;
	static UINT16 ainRawData[2];
	static UINT16 ainVoltage[2]; 
	char  *ptr;

	
    switch (message) 
    {
        case WM_COMMAND:

			if( wParam == 0x44556677 )
			{
                //wsprintf( sBuff, L"message from other thread" );
			    //DrawText (hdc, sBuff, -1, &rect, DT_LEFT | DT_SINGLELINE);
			}

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

            // Start the timer.  
            SetTimer(hWnd, idTimer = 1, 1000, NULL); 

			// Set bigger font size
			memset(&lf, 0, sizeof(LOGFONT));
            lf.lfHeight = 26;
            lf.lfWeight = FW_BOLD;
            _tcscpy(lf.lfFaceName, TEXT("Tahoma"));
            hf = CreateFontIndirect(&lf);

			ainRawData[0] = sizeof(ainRawData);
			ainVoltage[0] = sizeof(ainVoltage);
            hAin = MIC_AINOpen();
            g_hAin = hAin;

            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            // Get the size of the client rectangle
            GetClientRect (hWnd, &rect);

            SelectObject(hdc, hf);


			if( count == 0 )
			{
			 DrawText (hdc, TEXT ("Hello Windows CE!"), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			}
			else
			{
              //DrawText (hdc, TEXT ("Hello Windows CE!!!"), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
             // bReturn = DrawText (hdc, TEXT ("Periodic"), -1, &rect, DT_LEFT | DT_TOP | DT_SINGLELINE);

			// hAin = NULL;
             //(DWORD *)hAin = (DWORD *)hAin + 1; 
             ret = MIC_AINRawRead( hAin /*hAin*/, ainRawData); // ainVoltage
			 if( ret == FALSE )
			 {
              wsprintf( sBuff, L"MIC_AINRawRead Failure" );
			  DrawText (hdc, sBuff, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			 }
			 else
			 {
              //ptr =  (char *)hAin;
  			  //ptr +=4;
			  //hAin = (HANDLE)ptr;

              ret = MIC_AINmVRead( hAin, ainVoltage);
              if( ret == FALSE ) 
     		   {
                wsprintf( sBuff, L"MIC_AINmVRead Failure" );
			    DrawText (hdc, sBuff, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			   }
			  else
			  {
               wsprintf( sBuff, L"count = %d, A2D = %d, Voltage = %d", count, ainRawData[1], ainVoltage[1]); 
			   //wsprintf( sBuff, L"count = %d, A2D = %d", count, ainRawData); 
			   DrawText (hdc, sBuff, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			  }
			 }
			}

#if 0
            bReturn = ExtTextOut (hdc,
                        0,
                        0,
                        0,                  // No flags set
                        &rect,               // No clipping rectangle
                        L"Paint",           // String to display
                        lstrlen(L"Paint"), // Number of characters
                        NULL);              // Use default spacing.
#endif 
            // TODO: Add any drawing code here...
            
            EndPaint(hWnd, &ps);

            break;

        case WM_TIMER: 
             
            count++;  
            //UpdateWindow(hWnd);
			InvalidateRect(hWnd, NULL, TRUE);
			//RedrawWindow(hWnd, NULL, NULL, RDW_INTERNALPAINT );

        break;


        case WM_DESTROY:
			MIC_AINClose(hAin);
            CommandBar_Destroy(g_hWndCommandBar);
			KillTimer(hWnd, 1); 
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



DWORD WINAPI RunnerThread(LPVOID lpParam)
{ 
  int            bReturn;
  HDC            hdc;
  PAINTSTRUCT    ps;
  RECT rect;
  DWORD dwRetVal;
  BOOL           ret;
  UINT16         ainVoltage[2];

  ainVoltage[0] = sizeof(ainVoltage);

  while(1)
  {
   //Sleep(10);

   ret = MIC_AINmVRead( g_hAin, ainVoltage);
   dwRetVal = SendMessage(g_hWnd, WM_COMMAND, 0x44556677, 0x13);

#if 0
   hdc = BeginPaint(g_hWnd, &ps);
            
            // TODO: Add any drawing code here...
   GetClientRect (g_hWnd, &rect);

   bReturn = DrawText (hdc, TEXT ("Periodic"), -1, &rect, DT_LEFT | DT_TOP | DT_SINGLELINE);

            

   bReturn = ExtTextOut (hdc,
                        1,
                        1,
                        0,                  // No flags set
                        NULL,               // No clipping rectangle
                        TEXT("Periodic"),           // String to display
                        lstrlen(TEXT("Periodic")), // Number of characters
                        NULL);              // Use default spacing.


    EndPaint(g_hWnd, &ps);
#endif
	//SendMessage
  }
	return 0;
}