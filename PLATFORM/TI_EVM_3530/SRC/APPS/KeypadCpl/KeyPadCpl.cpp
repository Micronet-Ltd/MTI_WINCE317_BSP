// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
//

//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/**
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

**/


#include "KeyPadCpl.h"
#include <creg.hxx>
#include "Sliders.h"
#include <keypad_ls.h>

// Returns the number of characters in an expression.
#define LENGTHOF(exp) ((sizeof((exp)))/sizeof((*(exp))))
#define LIGHTSENSE_CONFIG		



HINSTANCE g_hInstance = NULL;

BOOL CALLBACK KeypadCplDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

//void	InitSlider(HWND hwndSlider, DWORD dwLevel);
//DWORD	GetSliderValue( HWND hwndSlider );
//UINT32	Volts2Luma(UINT32 luma);
//UINT32	Luma2Volts(UINT32 volts);
void	InitKpSettings(HWND hDlg, CTrackbarsArr& TrackArr);
void	UpdateKpSettings(HWND hDlg, CTrackbarsArr& TrackArr);

BOOL	GetHLFromReg( DWORD* pValH, DWORD* pValN, DWORD* pValL, DWORD* pValEnable, DWORD* pValSdEnable, DWORD* pValSuspEnable  );
BOOL	SetHLToReg	(  DWORD ValH, DWORD ValN, DWORD ValL, DWORD ValEnable, DWORD ValSdEnable, DWORD ValSuspEnable   );

extern "C" BOOL WINAPI DllEntry(HANDLE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			// Remember the instance
			g_hInstance = (HINSTANCE)hInstance;
			break;

		case DLL_PROCESS_DETACH:
			g_hInstance = NULL;
			break;
    }
    return TRUE;
}

void InitKpSettings(HWND hDlg, CTrackbarsArr& TrackArr )
{
	DWORD LevelLow	= 0; 
	DWORD LevelHigh = 0;
	DWORD LevelNorm	= 0; 
	DWORD fEnable	= 0;
	DWORD fSdEnable		= 1; 
	DWORD fSuspEnable	= 1;
	HWND	ctl;

	GetHLFromReg( &LevelHigh, &LevelNorm, &LevelLow, &fEnable, &fSdEnable, &fSuspEnable );

	RETAILMSG(0, (_T("InitKpSettings from reg: %d %d %d %d \r\n"), LevelHigh, LevelNorm, LevelLow, fEnable ));
	
	if( ctl = GetDlgItem(hDlg, IDC_CHB_LS) )
		SendMessage( ctl, BM_SETCHECK, (fEnable ? BST_CHECKED : BST_UNCHECKED), 0  );
		
	TrackArr.Init( hDlg, IDC_TRACKBAR_KEYPAD_HIGH,	LED_MAX_SETTING - LevelHigh );
	TrackArr.Init( hDlg, IDC_TRACKBAR_KEYPAD_NORM,	LED_MAX_SETTING - LevelNorm );
	TrackArr.Init( hDlg, IDC_TRACKBAR_KEYPAD_LOW,	LED_MAX_SETTING - LevelLow  );


	if( ctl = GetDlgItem(hDlg, IDC_SHDOWND_EN) )
		SendMessage( ctl, BM_SETCHECK, (fSdEnable ? BST_CHECKED : BST_UNCHECKED), 0  );
	if( ctl = GetDlgItem(hDlg, IDC_SUSP_EN) )
		SendMessage( ctl, BM_SETCHECK, (fSuspEnable ? BST_CHECKED : BST_UNCHECKED), 0  );

	RETAILMSG(0, (_T("InitKpSettings- \r\n")));
	
}
////////////////////////////////////////////////////////////////////////////////
//
//  void UpdateLsSettings(HWND hDlg) 
//
//  Updates LS settings to the registry from trackbar control & signal 
//  change event. 
//
//////////////////////////////////////////////////////////////////////////////// 
void UpdateKpSettings(HWND hDlg, CTrackbarsArr& TrackArr )
{
	if( hDlg )
	{
		DWORD LevelLow = 0, LevelNorm = 0, LevelHigh = 0;
		DWORD DependLsFlag = SendMessage( GetDlgItem(hDlg, IDC_CHB_LS), BM_GETCHECK, 0, 0  );
		
		LevelHigh = LED_MAX_SETTING - TrackArr.GetBlValue( IDC_TRACKBAR_KEYPAD_HIGH );
		LevelNorm = LED_MAX_SETTING - TrackArr.GetBlValue( IDC_TRACKBAR_KEYPAD_NORM );
		LevelLow =  LED_MAX_SETTING - TrackArr.GetBlValue( IDC_TRACKBAR_KEYPAD_LOW );
	 
		DWORD SdEnable		= SendMessage( GetDlgItem(hDlg, IDC_SHDOWND_EN), BM_GETCHECK, 0, 0  );
		DWORD SuspEnable	= SendMessage( GetDlgItem(hDlg, IDC_SUSP_EN), BM_GETCHECK, 0, 0  );

		SetHLToReg( LevelHigh, LevelNorm, LevelLow, DependLsFlag, SdEnable, SuspEnable );
	}
}
BOOL GetHLFromReg( DWORD* pValH, DWORD* pValN, DWORD* pValL, DWORD* pValEnable,
				  DWORD* pValShEnable, DWORD* pValSuspEnable)
{
	CReg reg;
	if( reg.Open( HKEY_LOCAL_MACHINE, KP_REG_KEY ) )
	{
		*pValH		= reg.ValueDW(KP_LIGHT_REG_LH,		0x62);
		*pValN		= reg.ValueDW(KP_LIGHT_REG_LN,		0x60);
		*pValL		= reg.ValueDW(KP_LIGHT_REG_LL,		0x32);
		*pValEnable = reg.ValueDW(KP_LIGHT_REG_DEPEND,	1	);

		*pValShEnable	= reg.ValueDW(KP_PW_REG_SD,		1);
		*pValSuspEnable = reg.ValueDW(KP_PW_REG_SUSP,	1);
		return 1;
	}
	RETAILMSG(1, (_T("Cannot open reg key %s\r\n"), KP_REG_KEY ));
	return 0;
}

BOOL	SetHLToReg(	DWORD ValH, DWORD ValN, DWORD ValL, DWORD ValEnable,
					DWORD ValSdEnable, DWORD ValSuspEnable				)
{
	BOOL res = 0;
	CReg reg;
	if( reg.OpenOrCreateRegKey( HKEY_LOCAL_MACHINE, KP_REG_KEY ) )
	{
		if( res = reg.SetDW(KP_LIGHT_REG_LH, ValH) )
		{
			if(  res = reg.SetDW(KP_LIGHT_REG_LN, ValN) )
			{
				if( res = reg.SetDW(KP_LIGHT_REG_LL, ValL ) )
					res = reg.SetDW(KP_LIGHT_REG_DEPEND, ValEnable );
			}
		}
		reg.SetDW(KP_PW_REG_SD,		ValSdEnable);
		reg.SetDW(KP_PW_REG_SUSP,	ValSuspEnable);
	}
	return res;
}


BOOL	NotifyChange()
{
	HANDLE hEv;
	if( hEv = CreateEvent( 0, 0, 0, KP_LEDS_CONFIG ) )
	{
		SetEvent( hEv );
		CloseHandle( hEv );
		return 1;
	}
	return 0;
}

BOOL CALLBACK KeypadCplDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static CTrackbarsArr s_TrackArr;

	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_KEYPAD_ICON));
			if (hIcon)
				SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (WPARAM)hIcon);
			InitKpSettings( hwndDlg, s_TrackArr );
			return TRUE;
		}
 		case WM_HSCROLL:            // track bar message
            switch LOWORD(wParam)
            {
				case TB_ENDTRACK:
					s_TrackArr.PosChanged((HWND)lParam); //RecheckPos( hwndPage, (HWND)lParam );
				break;

				case TB_BOTTOM:
                case TB_THUMBPOSITION:
                case TB_LINEUP:
                case TB_LINEDOWN:
                case TB_PAGEUP:
                case TB_PAGEDOWN:
                case TB_TOP:
                    return TRUE;
            }

            break;
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDOK:
				{
					UpdateKpSettings(hwndDlg,s_TrackArr);
					RegFlushKey(HKEY_LOCAL_MACHINE);
					//RegFlushKey(HKEY_CURRENT_USER);

					NotifyChange();
					EndDialog(hwndDlg, IDOK);
				}
				break;

				case IDCANCEL:
				{
					EndDialog(hwndDlg, IDCANCEL);
				}
				break;
			}
		}
		break;
		default:
			break;
	}
	return FALSE;
}
// When main control panel is loaded, KeyPadCpl.cpl will be loaded, then
// CPL_INIT, CPL_GETCOUNT and CPL_NEWINQUIRE will be called, then
// KeyPadCpl.cpl will be unloaded. When the KeyPadCpl icon is double clicked,
// KeyPadCpl.cpl will be loaded, then CPL_INIT and CPL_DBLCLK will be called.

extern "C" LONG CALLBACK CPlApplet(HWND hwndCPL, UINT message, LPARAM lParam1, LPARAM lParam2)
{
    switch (message)
    {
        case CPL_INIT:
            // Perform global initializations, especially memory  allocations, here.
            // Return 1 for success or 0 for failure.
            // Control Panel does not load if failure is returned.
            RETAILMSG(0, (TEXT("KeyPadCpl: CPL_INIT\r\n")));
            return 1;

        case CPL_GETCOUNT:
            // The number of actions supported by this applet
            RETAILMSG(0, (TEXT("KeyPadCpl: CPL_GETCOUNT\r\n")));
            return 1;

        case CPL_NEWINQUIRE:
		{
            // This message is sent once for each dialog box, as
            // determined by the value returned from CPL_GETCOUNT.
            // lParam1 is the 0-based index of the dialog box.
            // lParam2 is a pointer to the NEWCPLINFO structure.
            RETAILMSG(0, (TEXT("KeyPadCpl: CPL_NEWINQUIRE\r\n")));

            // this applet supports only 1 action, lParam1 must be 0
            ASSERT(0 == lParam1);
            ASSERT(lParam2);

            NEWCPLINFO* lpNewCplInfo = (NEWCPLINFO *) lParam2;
 
			if (lpNewCplInfo)
            {
				_tcscpy(lpNewCplInfo->szName, _T(""));
				_tcscpy(lpNewCplInfo->szInfo, _T(""));

				lpNewCplInfo->dwSize = sizeof(NEWCPLINFO);
                lpNewCplInfo->dwFlags = 0;
                lpNewCplInfo->dwHelpContext = 0;
 				
                LoadString(g_hInstance, IDS_KEYPAD_TITLE, lpNewCplInfo->szName, LENGTHOF(lpNewCplInfo->szName));
                // szInfo displayed when icon is selected
                LoadString(g_hInstance, IDS_KEYPAD_DESCRIPTION, lpNewCplInfo->szInfo, LENGTHOF(lpNewCplInfo->szInfo));

                // The large icon for this application. Do not free this 
                // HICON; it is freed by the Control Panel infrastructure.
                lpNewCplInfo->lData = IDI_KEYPAD_ICON;
                lpNewCplInfo->hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_KEYPAD_ICON));

				RETAILMSG(0, (TEXT("KeyPadCpl: szInfo = \"%s\"\r\n"), lpNewCplInfo->szInfo));
                RETAILMSG(0, (TEXT("KeyPadCpl: szName = \"%s\"\r\n"), lpNewCplInfo->szName));

                // szHelpFile is unused
                _tcscpy(lpNewCplInfo->szHelpFile, _T(""));
                return 0;
            }
            DEBUGMSG(1, (TEXT("KeyPadCpl: CPL_NEWINQUIRE failed\r\n")));
            return 1;  // Nonzero value means CPlApplet failed.
        }
			
        case CPL_DBLCLK:
		{
			// The user has double-clicked the icon for the dialog box in lParam1 (zero-based).
			TCHAR pszTitle[MAX_PATH] = {0};

			LoadString(g_hInstance, IDS_KEYPAD_DESCRIPTION, pszTitle,MAX_PATH);
			ASSERT(pszTitle);
            
			RETAILMSG(0, (TEXT("KeyPadCpl: CPL_DBLCLK - %s\r\n"), pszTitle));
			HWND hWnd = FindWindow(L"Dialog", pszTitle);
			if (hWnd)
			{
				SetForegroundWindow(hWnd);
			}
			else
			{
				InitCommonControls();
				if( -1 == DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_DLG), hwndCPL, KeypadCplDlgProc) )
				{
		            RETAILMSG(1, (TEXT("KeyPadCpl: CPL_DBLCLK err %d\r\n"), GetLastError() ));

				}
			}
		}			
		return 0;
			
        case CPL_STOP:
            // Called once for each dialog box. Used for cleanup.
            RETAILMSG(0, (TEXT("KeyPadCpl: CPL_STOP\r\n")));
            return 0;

        case CPL_EXIT:
            // Called only once for the application. Used for cleanup.
            RETAILMSG(0, (TEXT("KeyPadCpl: CPL_EXIT\r\n")));
            return 0;

        default:
            return 0;
    }

    return 1;  // CPlApplet failed.
}
