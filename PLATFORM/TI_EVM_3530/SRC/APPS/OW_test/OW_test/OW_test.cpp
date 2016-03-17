// OW_test.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "OW_test.h"
#include "ow_test.h"
#include <windows.h>
#include <commctrl.h>
#include <MicUserSdk.h>

#define MAX_LOADSTRING 100

typedef struct _oneWire_ROM_Id_WASCII_t
{
  TCHAR    wfamilyCode[3];
  TCHAR    wSerialNumber[13];
  TCHAR    wCRC[3];

} ONEWIRE_ROM_ID_WASCII_T;


// Global Variables:
HINSTANCE			g_hInst;			// current instance

DWORD WINAPI RunnerThread(LPVOID lpParam);

// Forward declarations of functions included in this code module:
//ATOM			    MyRegisterClass(HINSTANCE, LPTSTR);
//BOOL			    InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

HANDLE             g_hOWR = NULL;
OWR_INPUT_CONTEXT  g_iButtonInContext = {sizeof(OWR_INPUT_CONTEXT), sizeof(ONEWIRE_ROM_ID_T), 0, 0 };
ONEWIRE_ROM_ID_T   g_iButtonData;

typedef void (*VFPTR)(void);

BOOL  ret;
TCHAR myTest[32];
HWND  g_hDlg = NULL;
UINT32  g_DlgCnt = 0;
BOOL   g_outASCIItype = FALSE;

LRESULT CALLBACK Test(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  static int okCount=0;

	switch (Msg)
	{
		case WM_INITDIALOG:
		{
			/*
			RECT rt, rt1;
			int DlgWidth, DlgHeight; // dialog width and height in pixel units
			int NewPosX, NewPosY;
			// trying to center the dialog
			if (GetWindowRect(hDlg, &rt1))
			{
				GetClientRect(GetParent(hDlg), &rt);
				DlgWidth	= rt1.right  - rt1.left;
				DlgHeight	= rt1.bottom - rt1.top ;
				NewPosX		= (rt.right  - rt.left - DlgWidth )/2;
				NewPosY		= (rt.bottom - rt.top  - DlgHeight)/2;
				if (NewPosX < 0) NewPosX = 0;
				if (NewPosY < 0) NewPosY = 0;
			}
			*/
			
			// Test, read before Open! Pass
            //ret = MIC_OneWireRead(g_hOWR,&g_iButtonInContext, &g_iButtonData);

			g_hOWR =  MIC_OWROpen();

            g_hDlg = hDlg;
			g_DlgCnt++;

			// Test, read after Open, modify hanlde! Pass
			//g_hOWR = (HANDLE)((UINT32)g_hOWR +1);
			//ret = MIC_OneWireRead(g_hOWR,&g_iButtonInContext, &g_iButtonData);

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

			return TRUE;
		}

		case 0://WM_ERASEBKGND:
		{
			RECT rc;
			HBRUSH hBkBrush = CreateSolidBrush(GetSysColor(COLOR_BACKGROUND));
			GetWindowRect(hDlg, &rc);
			FillRect((HDC)wParam, &rc, hBkBrush);
			DeleteObject(hBkBrush);
			return TRUE;
		}

		case WM_ACTIVATE:
		{
		  if(LOWORD(wParam) == WA_ACTIVE)
		  {
           wsprintf( myTest, _T("Dialog%d [0x%08X]"), g_DlgCnt, hDlg );
		   SetWindowText(hDlg, myTest);
           //SetDlgItemText( hDlg, IDD_DIALOG1, myTest );  
		  }

            okCount++;
			return TRUE;
		} break;


#if 0
		case WM_PAINT:
		{
			RECT r;
			GetClientRect(hDlg, &r);
			break;
		}
#endif
		case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case VK_BACK:
				{
					EndDialog(hDlg, LOWORD(wParam));
					return FALSE;
				}
				case VK_F6:
				{
					RECT rc;
					HBRUSH hBkBrush = CreateSolidBrush(GetSysColor(COLOR_BACKGROUND));
					GetClientRect(hDlg, &rc);
					FillRect(GetDC(hDlg), &rc, hBkBrush);
					DeleteObject(hBkBrush);
					return FALSE;
				}
			}
			break;
		}
	    case WM_COMMAND:
		{
			switch( wParam )
			{
				case IDCANCEL:
					EndDialog(hDlg, LOWORD(wParam));
					g_DlgCnt--;
					MIC_OWRClose(g_hOWR);
					//Read after Closing, ret == 0, but error code some random number!
					//ret = MIC_OneWireRead(g_hOWR,&g_iButtonInContext, &g_iButtonData);
				return TRUE;

				case IDOK:
					//EndDialog(hDlg, LOWORD(wParam));
					okCount++;
					//MIC_OWROut(g_hOWR);
				return TRUE;

				case IDC_BUTTON1:
					okCount++;
					//MIC_OWRTest(g_hOWR, 1, NULL);

					//MIC_OWROut(g_hOWR);
				return TRUE;

				case IDC_BUTTON2:
					{
					 okCount++;
					 // Read
					 ret = MIC_OneWireRead(g_hOWR,&g_iButtonInContext, &g_iButtonData);
					 if( ret == TRUE )
					 {
					   wsprintf( myTest, _T("[%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X]"), 
					             g_iButtonData.familyCode, 
								 g_iButtonData.SerialNumber[0],
								 g_iButtonData.SerialNumber[1],
								 g_iButtonData.SerialNumber[2],
								 g_iButtonData.SerialNumber[3],
								 g_iButtonData.SerialNumber[4],
								 g_iButtonData.SerialNumber[5],
								 g_iButtonData.CRC
								 );
					 }
					 else
					 {
                       wsprintf( myTest, _T("read err = %d"), g_iButtonInContext.opErrorCode );
					 }

					 SetDlgItemText( hDlg, /*IDC_BUTTON6*/ IDC_STATUS1, myTest );
					}
				return TRUE;

				case IDC_BUTTON3:
					okCount++;
					{
					  ret =	MIC_OneWireEventRegistering(g_hOWR, &g_iButtonInContext, TRUE );
					  if( ( ret == TRUE )&& (g_iButtonInContext.opErrorCode == OWR_OK) )
					  {
                       wsprintf( myTest, _T("registered OK"));
					  }
					  else
					  {
                       wsprintf( myTest, _T("registered err = %d"), g_iButtonInContext.opErrorCode );                 
					  }

					  SetDlgItemText( hDlg, IDC_STATUS1, myTest );
					}
					
				return TRUE;

				case IDC_BUTTON4:
					// Unregister for notify
					  ret =	MIC_OneWireEventRegistering(g_hOWR, &g_iButtonInContext, FALSE );
					  if( ( ret == TRUE )&& (g_iButtonInContext.opErrorCode == OWR_OK) )
					  {
                       wsprintf( myTest, _T("unregistered OK"));
					  }
					  else
					  {
                       wsprintf( myTest, _T("unregistered err = %d"), g_iButtonInContext.opErrorCode );                 
					  }

					  SetDlgItemText( hDlg, IDC_STATUS1, myTest );

					okCount++;
					//MIC_OWROut(g_hOWR);
				return TRUE;

				case IDC_BUTTON6:
					// Set Private access
					  ret =	MIC_OneWirePrivateAccess(g_hOWR, &g_iButtonInContext, TRUE );
					  if( ( ret == TRUE )&& (g_iButtonInContext.opErrorCode == OWR_OK) )
					  {
                       wsprintf( myTest, _T("access is private OK"));
					  }
					  else
					  {
                       wsprintf( myTest, _T("access is private err = %d"), g_iButtonInContext.opErrorCode );
					  }

					  SetDlgItemText( hDlg, IDC_STATUS1, myTest );

					okCount++;
				return TRUE;

				case IDC_BUTTON5:
					// Set Private access
					  ret =	MIC_OneWirePrivateAccess(g_hOWR, &g_iButtonInContext, FALSE );
					  if( ( ret == TRUE )&& (g_iButtonInContext.opErrorCode == OWR_OK) )
					  {
                       wsprintf( myTest, _T("access is public OK"));
					  }
					  else
					  {
                       wsprintf( myTest, _T("access is public err = %d"), g_iButtonInContext.opErrorCode );
					  }

					  SetDlgItemText( hDlg, IDC_STATUS1, myTest );

					okCount++;
				return TRUE;

				case  IDC_CHECK1:

					   if(IsDlgButtonChecked(hDlg, IDC_CHECK1) == BST_CHECKED)
						{
							//CheckDlgButton(hDlg, IDC_CHECK1, BST_UNCHECKED);
							g_outASCIItype = TRUE;
						}
						else
						{
							//CheckDlgButton(hDlg, IDC_CHECK1, BST_CHECKED);
							g_outASCIItype = FALSE;
						}

                return TRUE;

			}
			break;
		}
	}
	return FALSE;
}



int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	int ret;
#if 0
	MSG msg;


	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OW_TEST));

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
#endif

#if 1  // test

typedef struct  PACKET_1_tag
{
 UINT8 b1;
 UINT8 b2;
 UINT8 b3;

}SPACKET_1;

typedef struct  PACKET_2_tag
{
 UINT8 c1;
 UINT8 c2;
 UINT8 c3;
 UINT8 c4;
 UINT8 c5;
 UINT8 c6;
 UINT8 c7;

}SPACKET_2;

typedef union
{
 UINT8      cBuf[4];
 UINT32     wb;

}uPACKED_3;

SPACKET_1  s1;
SPACKET_2  s2;
uPACKED_3  s3;

s1.b1 = 0x1;
s1.b2 = 0x2;
s1.b3 = 0x3;

s2.c1 = 0x1;
s2.c2 = 0x2;
s2.c3 = 0x3;
s2.c4 = 0x4;
s2.c5 = 0x5;
s2.c6 = 0x6;
s2.c7 = 0x7;

s3.cBuf[0] = 0x1;
s3.cBuf[1] = 0x2;
s3.cBuf[2] = 0x3;
s3.cBuf[3] = 0x4;

#endif

	ret = DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), 0, (DLGPROC)Test);
	//if( ret == -1 )
	ret =	GetLastError();
	return 0;
}

#if 0
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
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OW_TEST));
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
    LoadString(hInstance, IDC_OW_TEST, szWindowClass, MAX_LOADSTRING);

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

    //hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
        //CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    hWnd = CreateWindow(szWindowClass, szTitle, /*WS_VISIBLE | WS_SIZEBOX */ WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX ,
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
#endif


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
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_CREATE:
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            
            // TODO: Add any drawing code here...
            
            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:
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
                return (INT_PTR)TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog(hDlg, message);
            return (INT_PTR)TRUE;

    }
    return (INT_PTR)FALSE;
}



DWORD WINAPI RunnerThread(LPVOID lpParam)
{ 
//  int            bReturn;
//  HDC            hdc;
  //HWND           hlDlg;
//  PAINTSTRUCT    ps;
//  RECT rect;
//  DWORD dwRetVal;
  BOOL               lret;
  OWR_INPUT_CONTEXT  iButtonInContext = {sizeof(OWR_INPUT_CONTEXT), sizeof(ONEWIRE_ROM_ID_T), 0, 0 };
  //ONEWIRE_ROM_ID_ASCII_T
  OWR_INPUT_CONTEXT  iButtonInContextString = {sizeof(OWR_INPUT_CONTEXT), sizeof(ONEWIRE_ROM_ID_ASCII_T), 0, 0 };
  TCHAR          lTestStr[64];
  TCHAR          lTestStrData[32];
  ONEWIRE_ROM_ID_T         iButtonData;
  ONEWIRE_ROM_ID_ASCII_T   iButtonDataString;
  ONEWIRE_ROM_ID_WASCII_T  iButtonDataUString;
  static DWORD   eventCnt = 0;
  size_t         len1, len2 /*, len*/;


    if( ( g_hOWR == NULL )||( g_hOWR == INVALID_HANDLE_VALUE ) )
	{
 	 if( g_hDlg )
	 {
      wsprintf( lTestStrData, _T("No access to iButton - exit") ); 
	  SetDlgItemText( g_hDlg, IDC_STATUS1, lTestStr );
	  return(1);
	 }

	}


  while(1)
  {
   //Sleep(10);

   //dwRetVal = SendMessage(g_hWnd, WM_COMMAND, 0x44556677, 0x13);

		lret =	MIC_OneWireWaitForEvent(g_hOWR, &iButtonInContext );
		//Dialog1 
		//hlDlg = FindWindow(NULL, L"Dialog1" );

		eventCnt++;

		if( lret == TRUE  )
		{
		 wsprintf( lTestStr, _T("event # %d is OK"), eventCnt);

		 switch(iButtonInContext.oneWireEventType)
		 {
		   case OWR_I_BUTTON_ATTACH:
			 {

			  if( g_outASCIItype == FALSE )
			  {
				 lret = MIC_OneWireRead(g_hOWR, &iButtonInContext, &iButtonData);
				 if( lret == TRUE )
				 {
					  wsprintf( lTestStrData, _T("  %02X %02X%02X%02X%02X%02X%02X %02X"), 
											 iButtonData.familyCode, 
											 iButtonData.SerialNumber[0],
											 iButtonData.SerialNumber[1],
											 iButtonData.SerialNumber[2],
											 iButtonData.SerialNumber[3],
											 iButtonData.SerialNumber[4],
											 iButtonData.SerialNumber[5],
											 iButtonData.CRC
											 );
				 }
				 else
				 {
				   wsprintf( lTestStrData, _T("read failure") );
				 }
			  }
			  else
			  {
               lret = MIC_OneWireReadASCII(g_hOWR, &iButtonInContextString, &iButtonDataString);
			   if( lret == TRUE )
			   {
                 mbstowcs(iButtonDataUString.wCRC, (const char *)iButtonDataString.CRC, sizeof(iButtonDataString.CRC) );
				 mbstowcs(iButtonDataUString.wfamilyCode, (const char *)iButtonDataString.familyCode, sizeof(iButtonDataString.familyCode) );
				 mbstowcs(iButtonDataUString.wSerialNumber, (const char *)iButtonDataString.SerialNumber, sizeof(iButtonDataString.SerialNumber) );

				 wsprintf( lTestStrData, _T("  %s  %s  %s") , iButtonDataUString.wCRC, iButtonDataUString.wSerialNumber, iButtonDataUString.wfamilyCode );
			   }
			   else
			   {
			    wsprintf( lTestStrData, _T("read failure") );
			   }

			  }

			 } break;

			 case OWR_I_BUTTON_DEATTACH:
			 {
			   // Deattach
				 wsprintf( lTestStrData, _T("  deattach!") );
			 } break;

			 case OWR_I_BUTTON_ERROR:
			 {
			   // Deattach
				 wsprintf( lTestStrData, _T(" error = %d event!"), iButtonInContext.opErrorCode );
			 } break;


		 } // switch

		   if( ( StringCchLength(lTestStr, 60, &len1) == S_OK ) &&
			   ( StringCchLength(lTestStrData, 30, &len2) == S_OK ) 
			 )
		   {
			if( len1 + len2 < 63 )
 			 StringCchCat(lTestStr, len1 + len2+1, lTestStrData );
		   }

		}
		else
		{
         if( iButtonInContext.opErrorCode == OWR_ERROR_CLOSED_HANDLE )
		 {
		  wsprintf( lTestStr, _T("Closing... event # %d err = %d"), eventCnt, iButtonInContext.opErrorCode );
		  if( g_hDlg )
		    SetDlgItemText( g_hDlg, IDC_STATUS2, lTestStr );

		  return 0;
		 }
		 else if( iButtonInContext.opErrorCode == OWR_ERROR_NOTIFICATION ) 
		 {
		  wsprintf( lTestStr, _T("Error notification, quit... event # %d err = %d"), eventCnt, iButtonInContext.opErrorCode );
		  if( g_hDlg )
		    SetDlgItemText( g_hDlg, IDC_STATUS2, lTestStr );

		  return 0;
		 }
		 else
		 {
		  wsprintf( lTestStr, _T("Unrecognized Error, quit... event # %d err = %d"), eventCnt, iButtonInContext.opErrorCode );
		  if( g_hDlg )
		    SetDlgItemText( g_hDlg, IDC_STATUS2, lTestStr );

		  return 0;
		 }
		}

		if( g_hDlg )
		 SetDlgItemText( g_hDlg, IDC_STATUS2, lTestStr );

  }

	return 0;
}
