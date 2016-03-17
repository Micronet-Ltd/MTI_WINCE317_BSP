// OW_test.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "CANt.h"
#include <windows.h>
#include <commctrl.h>
#include "winioctl.h"
#include <MicUserSdk.h>
#include "CanOpenLib_HW.h"
#include "can_api_ex.h"
#include "CANBDIOctl.h"

#define MAX_LOADSTRING 100



// Global Variables:
HINSTANCE			g_hInst;			// current instance

DWORD WINAPI RunnerThread(LPVOID lpParam);

// Forward declarations of functions included in this code module:
//ATOM			    MyRegisterClass(HINSTANCE, LPTSTR);
//BOOL			    InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

static CAN_OPMODE  CANGetOpMode(canPortHandle hDevice);
static BOOL CANSetOpMode(canPortHandle hDevice, CAN_OPMODE OpMode);

//HANDLE             g_hOWR = NULL;
canPortHandle      g_hCAN = 0;

typedef void (*VFPTR)(void);

BOOL  ret;
TCHAR myTest[32];
TCHAR myTest2[32];
HWND  g_hDlg = NULL;
UINT32  g_DlgCnt = 0;
HANDLE g_hThr = NULL;

GENCANCONFIG  gCANCNF;



LRESULT CALLBACK TestDi(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  static int okCount=0;
  canOpenStatus  canStatus;

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

			//g_hOWR =  MIC_OWROpen();
			canStatus = canPortOpen( 0, &g_hCAN );

#if 1
			canStatus = canPortGetConfig(g_hCAN, &gCANCNF);
 		    if( canStatus == CANOPEN_OK )
			{
             gCANCNF.CANfilter.Filter[0].bUpdate  = TRUE;
			 gCANCNF.CANfilter.Filter[0].bFISATO  = FALSE;
			 gCANCNF.CANfilter.Filter[0].SID = 0xF0;
			 gCANCNF.CANfilter.Filter[0].EID = 0xE0;

             gCANCNF.CANfilter.Filter[1].bUpdate  = TRUE;
			 gCANCNF.CANfilter.Filter[1].bFISATO  = TRUE;
			 gCANCNF.CANfilter.Filter[1].SID = 0xF1;
			 gCANCNF.CANfilter.Filter[1].EID = 0xE1;

             gCANCNF.CANfilter.Filter[2].bUpdate  = TRUE;
			 gCANCNF.CANfilter.Filter[2].bFISATO  = FALSE;
			 gCANCNF.CANfilter.Filter[2].SID = 0xF2;
			 gCANCNF.CANfilter.Filter[2].EID = 0xE2;

             gCANCNF.CANfilter.Filter[3].bUpdate  = TRUE;
			 gCANCNF.CANfilter.Filter[3].bFISATO  = FALSE;
			 gCANCNF.CANfilter.Filter[3].SID = 0xF3;
			 gCANCNF.CANfilter.Filter[3].EID = 0xE3;

             gCANCNF.CANfilter.Filter[4].bUpdate  = TRUE;
			 gCANCNF.CANfilter.Filter[4].bFISATO  = FALSE;
			 gCANCNF.CANfilter.Filter[4].SID = 0xF4;
			 gCANCNF.CANfilter.Filter[4].EID = 0xE4;

             gCANCNF.CANfilter.Filter[5].bUpdate  = TRUE;
			 gCANCNF.CANfilter.Filter[5].bFISATO  = FALSE;
			 gCANCNF.CANfilter.Filter[5].SID = 0xF5;
			 gCANCNF.CANfilter.Filter[5].EID = 0xE5;

			 gCANCNF.CANmask.Mask[0].bUpdate = TRUE; 
			 gCANCNF.CANmask.Mask[0].bFISATO = FALSE;
			 gCANCNF.CANmask.Mask[0].SID = 0x00;
			 gCANCNF.CANmask.Mask[0].EID = 0x00;

			 gCANCNF.CANmask.Mask[1].bUpdate = TRUE; 
			 gCANCNF.CANmask.Mask[1].bFISATO = FALSE;
			 gCANCNF.CANmask.Mask[1].SID = 0x00;
			 gCANCNF.CANmask.Mask[1].EID = 0x00;

			 canStatus = canPortGoBusOff(g_hCAN);
			 canStatus = canPortSetConfig(g_hCAN, &gCANCNF);
			}
#endif 

		    canStatus = canPortBitrateSet(g_hCAN, CANBR_250KBPS);
			canStatus = canPortGetConfig(g_hCAN, &gCANCNF);

            g_hDlg = hDlg;
			g_DlgCnt++;

			// Test, read after Open, modify hanlde! Pass
			//g_hOWR = (HANDLE)((UINT32)g_hOWR +1);
			//ret = MIC_OneWireRead(g_hOWR,&g_iButtonInContext, &g_iButtonData);

			HANDLE hThr = CreateThread(0, 0, RunnerThread, NULL, 0, 0 );
			
			if( !hThr )
			{
				//DebugPrint("Faild GetInitParam thread creation");
				g_hThr = hThr;
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
           //wsprintf( myTest, _T("Dialog%d [0x%08X]"), g_DlgCnt, hDlg );
		   //SetWindowText(hDlg, myTest);
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
					CloseHandle( g_hThr );
					//MIC_OWRClose(g_hOWR);
					canPortClose(g_hCAN);

					//Read after Closing, ret == 0, but error code some random number!
					//ret = MIC_OneWireRead(g_hOWR,&g_iButtonInContext, &g_iButtonData);
				return TRUE;

				case IDOK:
					//EndDialog(hDlg, LOWORD(wParam));
					okCount++;
					//MIC_OWROut(g_hOWR);
				return TRUE;

				case IDC_BUTTON1:
					{
                     //long id;
					 char  testMsg[8] = "CAN_001";
                     //unsigned int dlc;
                     //unsigned int flags;

					// Send message
                     canStatus = canPortWrite(g_hCAN, 0x555, testMsg, 8, 0 );

					 if( canStatus == CANOPEN_OK )
					  wsprintf( myTest, _T("Send Ok") );
					 else
                      wsprintf( myTest, _T("Send  fail, err = %d"), canStatus);

					 SetDlgItemText( hDlg, IDC_STATUS1, myTest );

					okCount++;
					}
				return TRUE;

				case IDC_BUTTON2:
					{
                     //long id;
					 char  testMsg[8] = "CAN_E01";
                     //unsigned int dlc;
                     //unsigned int flags;

					// Send message
                     canStatus = canPortWrite(g_hCAN, 0x05555, testMsg, 8, CAN_MSG_EXT );

					 if( canStatus == CANOPEN_OK )
					  wsprintf( myTest, _T("Send ext Ok") );
					 else
                      wsprintf( myTest, _T("Send ext fail, err = %d"), canStatus);

					 SetDlgItemText( hDlg, IDC_STATUS1, myTest );

					okCount++;
					}
				return TRUE;

				case IDC_BUTTON3:
					{
					 canStatus = canPortGetConfig(g_hCAN, &gCANCNF);

					 if( canStatus == CANOPEN_OK )
					  wsprintf( myTest, _T("get CNF Ok") );
					 else
                      wsprintf( myTest, _T("get CNF fail, err = %d"), canStatus);

					 SetDlgItemText( hDlg, IDC_STATUS1, myTest );

					 okCount++;
					}
				return TRUE;

				// Speed query from ECU Simulator
				case IDC_BUTTON4:
					{
                     UINT16 obd_id = 0x7DF; 
					 UINT8  numOfAddDatabytes = 2;
					 UINT8  Mode = 0x01;
					 UINT8  PIDcode = 0x0D;  // Speed
					 UINT8  testMsg[8];

					 testMsg[0] = numOfAddDatabytes;
					 testMsg[1] = Mode;
					 testMsg[2] = PIDcode;


					// Send message
                     canStatus = canPortWrite(g_hCAN, obd_id, testMsg, numOfAddDatabytes+1, 0 );

					 if( canStatus == CANOPEN_OK )
					  wsprintf( myTest, _T("Send speed query Ok") );
					 else
                      wsprintf( myTest, _T("Send speed query, err = %d"), canStatus);

					 SetDlgItemText( hDlg, IDC_STATUS1, myTest );

					okCount++;
					}
				return TRUE;

				case IDC_BUTTON5:
					{
                     // Loopback test
					 CAN_OPMODE curOpMode;
					 CAN_OPMODE OpMode;
					 BOOL       ret; 

                     // Save current mode
					 curOpMode = CANGetOpMode(g_hCAN);

					 // Set Loop back mode
					 ret = CANSetOpMode(g_hCAN, CAN_OPMODE_LOOPBACK);
					 if( ret == TRUE )
					 {
					   char  testMsg[8] = "CAN_LB1";

						// Send message
						 canStatus = canPortWrite(g_hCAN, 0x333, testMsg, 8, 0 );

						 if( canStatus == CANOPEN_OK )
						  wsprintf( myTest, _T("Send LB Ok") );
						 else
						  wsprintf( myTest, _T("Send LB fail, err = %d"), canStatus);

                       // Restore CAN operational mode
                       CANSetOpMode(g_hCAN, curOpMode);
					 }
					 else
					 {
                       wsprintf( myTest, _T("Cannot Enter LoopBack mode") );
					 }
                    
					 SetDlgItemText( hDlg, IDC_STATUS1, myTest );
					}
                return TRUE;
               

				case  IDC_CHECK1:

					   if(IsDlgButtonChecked(hDlg, IDC_CHECK1) == BST_CHECKED)
						{
							//CheckDlgButton(hDlg, IDC_CHECK1, BST_UNCHECKED);
							canStatus = canPortGoBusOn(g_hCAN);
							if( canStatus == CANOPEN_OK )
							  wsprintf( myTest, _T("Bus On") );
							else
                              wsprintf( myTest, _T("Bus On fail, err = %d"), canStatus);

							SetDlgItemText( hDlg, IDC_STATUS1, myTest );
						}
						else
						{
							//CheckDlgButton(hDlg, IDC_CHECK1, BST_CHECKED);
							canStatus = canPortGoBusOff(g_hCAN);
							if( canStatus == CANOPEN_OK )
							  wsprintf( myTest, _T("Bus Off") );
							else
                              wsprintf( myTest, _T("Bus Off fail, err = %d"), canStatus);

							SetDlgItemText( hDlg, IDC_STATUS1, myTest );
						}

                return TRUE;


				case  IDC_CHECK2:

					   if(IsDlgButtonChecked(hDlg, IDC_CHECK2) == BST_CHECKED)
						{
							//CheckDlgButton(hDlg, IDC_CHECK1, BST_UNCHECKED);
							canStatus = canPortEcho(g_hCAN, TRUE);
							if( canStatus == CANOPEN_OK )
							  wsprintf( myTest, _T("Echo On") );
							else
                              wsprintf( myTest, _T("Echo On fail, err = %d"), canStatus);

							SetDlgItemText( hDlg, IDC_STATUS1, myTest );
						}
						else
						{
							//CheckDlgButton(hDlg, IDC_CHECK1, BST_CHECKED);
							canStatus = canPortEcho(g_hCAN, FALSE);
							if( canStatus == CANOPEN_OK )
							  wsprintf( myTest, _T("Echo Off") );
							else
                              wsprintf( myTest, _T("Echo Off fail, err = %d"), canStatus);

							SetDlgItemText( hDlg, IDC_STATUS1, myTest );
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

#if 0 // test

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

	ret = DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), 0, (DLGPROC)TestDi);
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
//  BOOL               lret;
  TCHAR          lTestStr[64];
  TCHAR          lTestStrData[32];
  static DWORD   eventCnt = 0;
//  size_t         len1, len2 /*, len*/;
  canOpenStatus  canStatus;
  long           id;
  unsigned int            i, len;
  unsigned int   dlc, flags;
  unsigned char  testMsg[8];
  TCHAR          tflagFr[4];
  TCHAR          tflagRt[4];

    if( g_hCAN == 0 )
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
   //canStatus = canPortWrite(g_hCAN, 0x17, testMsg, 8, 0 );
   canStatus = canPortRead(g_hCAN, &id, testMsg, &dlc, &flags ); 
   eventCnt++;

   //  [%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X]
	 if( ( canStatus == CANOPEN_OK )&& (dlc <=8 ) )
	 {
	   if(	flags & CAN_MSG_EXT )
		StringCchCopy(tflagFr, sizeof(tflagFr), _T("EXT"));
	   else
        StringCchCopy(tflagFr, sizeof(tflagFr), _T("STD"));

	   if(	flags & CAN_MSG_RTR ) //tflagRt
		StringCchCopy(tflagRt, sizeof(tflagRt), _T("RTR"));
	   else
        StringCchCopy(tflagRt, sizeof(tflagRt), _T("   "));

	   wsprintf( lTestStr, _T("[%d]: (id = %X)(%s)(%s)  "), eventCnt, id, tflagFr, tflagRt );

       for( i = 0; i < dlc; i++ )
	   {
        StringCchLength(lTestStr, 64, &len );

  	    wsprintf( lTestStrData, _T("[%02X]"), testMsg[i] );
		StringCchCat(lTestStr, len + 5*sizeof(TCHAR) , lTestStrData );
	   }

	    StringCchLength(lTestStr, 64, &len );
  	    wsprintf( lTestStrData, _T("\r\n"), testMsg[i] );
		StringCchCat(lTestStr, len + 2*sizeof(TCHAR) , lTestStrData );

	   SetDlgItemText( g_hDlg, IDC_STATUS1, lTestStr );
	 }
	 else
	 {
       if( canStatus != CANOPEN_TIMEOUT ) 
	   {
	    wsprintf( lTestStr, _T("read # %d fail, err = %d"), eventCnt, canStatus);
		SetDlgItemText( g_hDlg, IDC_STATUS1, lTestStr );
	   }
	 }



   //dwRetVal = SendMessage(g_hWnd, WM_COMMAND, 0x44556677, 0x13);

   //		lret =	MIC_OneWireWaitForEvent(g_hOWR, &iButtonInContext );
		//Dialog1 
		//hlDlg = FindWindow(NULL, L"Dialog1" );


  }

	return 0;
}


static CAN_OPMODE  CANGetOpMode(canPortHandle hDevice)
{
  //return( (CAN_OPMODE)MCP2515GetModeSPI() );
 BOOL        ret;
 DWORD       dwBytesReturned;
 CAN_OPMODE  OpMode;

 ret = DeviceIoControl( (HANDLE)hDevice, IOCTL_CAN_GET_OPMODE, NULL, 0, &OpMode,  sizeof(CAN_OPMODE), &dwBytesReturned, NULL);

 return OpMode;

}

static BOOL CANSetOpMode(canPortHandle hDevice, CAN_OPMODE OpMode)
{
 BOOL    ret;
 DWORD   dwBytesReturned;

 ret = DeviceIoControl( (HANDLE)hDevice, IOCTL_CAN_SET_OPMODE, &OpMode, sizeof(CAN_OPMODE), 0, 0, &dwBytesReturned, NULL);

 return ret;
}
