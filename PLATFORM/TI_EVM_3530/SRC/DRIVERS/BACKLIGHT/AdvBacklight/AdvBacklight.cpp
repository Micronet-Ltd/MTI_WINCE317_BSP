//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
/**
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Purpose:    Customised "Advanced..." button under Display Properties\Backlight tab
**/
//------------------------------------------------------------------------------
//
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------


//#include "AdvBacklight.h"
#include <creg.hxx>
#include "Sliders.h"

HINSTANCE		g_hInst = NULL;
PROPSHEETPAGE*	rgPropPages;
PROPSHEETHEADER PropHdr;


static const TCHAR szRegKey[]           = BL_REG_KEY;
//
static const TCHAR szRegBatteryLevel[]  = BL_REG_BATT_LEVEL;
static const TCHAR szRegACLevel[]       = BL_REG_AC_LEVEL;

//by power d0 - d4

DWORD	BatteryLevels[STATES_QTY]	= { BACKLIGHT_HIGH, BACKLIGHT_LOW, BACKLIGHT_LOW, BACKLIGHT_LOW, BACKLIGHT_LOW };
DWORD	ACLevels[STATES_QTY]		= { BACKLIGHT_HIGH, BACKLIGHT_LOW, BACKLIGHT_LOW, BACKLIGHT_LOW, BACKLIGHT_LOW };

//HWND g_hDlg0 = 0;
HWND g_hDlg1 = 0;
//DWORD	UpdateDlgSetting( HWND hDlgFrom, HWND hDlg );
void		InitDlgSetting( HWND hwndPage );
DWORD		g_LastActive	= 0;
DWORD		g_First			= 1;

CTrackbarsArr TrackArr;

////////////////////////////////////////////////////////////////////////////////
//
//  BOOL BacklightAdvApplet(HWND hDlg)
//
//  Function name is based on the definition under cplmain\regcpl.h:
//  
//  This is the entry point we look for in the CPL pointed to by RV_ADVANCEDCPL
//
////////////////////////////////////////////////////////////////////////////////
BOOL BacklightAdvApplet(HWND hDlg)
{
    BOOL fRet = FALSE;
    int iData = 0;  //To track the OK|Cancel choice.

    // Allocate the page array (one page in this sample code).
    rgPropPages = (PROPSHEETPAGE*)LocalAlloc(LPTR, 2 * sizeof(PROPSHEETPAGE));

    if (NULL == rgPropPages) {
        return FALSE;
    }

    rgPropPages[0].dwSize           = sizeof(PROPSHEETPAGE);
    rgPropPages[0].dwFlags          = PSP_USETITLE;
    rgPropPages[0].hInstance        = g_hInst;
    rgPropPages[0].pszTemplate      = MAKEINTRESOURCE(IDD_DLG); 
    rgPropPages[0].pszIcon          = NULL;
    rgPropPages[0].pfnDlgProc       = AdvancedDlgProc;  //Pointer to the dialog box procedure for the page. 
                                                                                                    //The dlg proc must not call the Win32 EndDialog function. 
    rgPropPages[0].pszTitle         = MAKEINTRESOURCE(IDS_PRSHDIALOG);
    rgPropPages[0].pfnCallback      = NULL;
    rgPropPages[0].lParam           = (LPARAM)&iData;

    rgPropPages[1].dwSize           = sizeof(PROPSHEETPAGE);
    rgPropPages[1].dwFlags          = PSP_USETITLE;
    rgPropPages[1].hInstance        = g_hInst;
    rgPropPages[1].pszTemplate      = MAKEINTRESOURCE(IDD_DLG_LS); 
    rgPropPages[1].pszIcon          = NULL;
    rgPropPages[1].pfnDlgProc       = AdvancedDlgProcLs;  //Pointer to the dialog box procedure for the page. 
                                                                                                    //The dlg proc must not call the Win32 EndDialog function. 
    rgPropPages[1].pszTitle         = MAKEINTRESOURCE(IDS_PRSHDLG_LS);
    rgPropPages[1].pfnCallback      = NULL;
    rgPropPages[1].lParam           = 0;//(LPARAM)&iData;

	//Create a property sheet and add the page defined in the property sheet header structure. 
    PropHdr.dwSize     = sizeof(PROPSHEETHEADER);
    PropHdr.dwFlags    = PSH_PROPSHEETPAGE;                                             
    PropHdr.hwndParent = hDlg; 
    PropHdr.hInstance  = g_hInst;
    PropHdr.pszIcon    = NULL;                      
    PropHdr.pszCaption = MAKEINTRESOURCE(IDS_PRSHTITLE);    
    PropHdr.nPages     = 2; 
    PropHdr.nStartPage = 0;
    PropHdr.ppsp       = rgPropPages;
    
    //PropertySheet creates a modal dialog. Return -1 if it fails.
    fRet = PropertySheet(&PropHdr);     
    
    if (-1 != fRet && iData) 
	{
        fRet = TRUE;
    } 
	else 
	{
        fRet = FALSE;
    }
    
    if(rgPropPages) LocalFree(rgPropPages);

    return fRet;
}

////////////////////////////////////////////////////////////////////////////////
//
//  BOOL WINAPI DllEntry(HANDLE hInstance, DWORD fdwReason, LPVOID lpvReserved)
//
//  Win 32 Initialization DLL
//
////////////////////////////////////////////////////////////////////////////////
BOOL WINAPI DllEntry(HANDLE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // Remember the instance
            g_hInst = (HINSTANCE)hInstance;
            break;

        case DLL_PROCESS_DETACH:
            g_hInst = NULL;
            break;
    }
    return TRUE;
}
BOOL CALLBACK AdvancedDlgProcLs(HWND hwndPage, UINT message, WPARAM wParam, LPARAM lParam)
{       
	switch (message)     
    {
        case WM_INITDIALOG:
		{
			g_hDlg1 = hwndPage;
			g_First = 0;

			InitDlgSetting( hwndPage );

			return TRUE;
		}
 		case WM_HSCROLL:            // track bar message
		{
			switch LOWORD(wParam)
            {
				case TB_ENDTRACK:
					TrackArr.PosChanged((HWND)lParam); //RecheckPos( hwndPage, (HWND)lParam );
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

		}
		break;
        case WM_NOTIFY:
        {
            switch(((NMHDR*)lParam)->code)
            {
				case PSN_SETACTIVE:
				{
					g_LastActive = 1;
					TrackArr.UpdatePos();
					UpdateLables( hwndPage );
				}
				break;
			}
		}
	}
	return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
//
//  BOOL APIENTRY AdvancedDlgProc (HWND hwndPage, UINT message, WPARAM wParam, LPARAM lParam)
//
//  Settings tab dialog proc: DlgProc for the tab.
//
//////////////////////////////////////////////////////////////////////////////// 
BOOL CALLBACK AdvancedDlgProc(HWND hwndPage, UINT message, WPARAM wParam, LPARAM lParam)
{       
    PROPSHEETPAGE *psp;
    int *piData;
    
    switch (message)     
    {
        case WM_INITDIALOG:
		{
			// Get application-defined information (lParam field).
            psp = (PROPSHEETPAGE *)lParam;          
            if( psp ) 
			{
				RECT Rect1 = {0};
				RECT Rectd = {0};

				HWND hWndP;
				HWND hWndd = GetParent( hwndPage );//dlg window			

				if( hWndd )
				{
					hWndP = GetParent( hWndd );//parent of dlg
					if( hWndP )
					{
						if( GetWindowRect( hWndP, &Rect1 ) )	
							if( GetWindowRect( hWndd, &Rectd ) )
								MoveWindow( hWndd, Rect1.left, Rect1.top, Rectd.right - Rectd.left, Rectd.bottom - Rectd.top, 0 );
					}
				}
				g_First = 1;
				SetWindowLong(hwndPage, GWL_USERDATA, psp->lParam); 
				piData = (int *)psp->lParam;
	            // Sets new extra information that is private to the application
		        SetWindowLong(hwndPage, DWL_USER, (LONG)piData);
			}

            // get backlight driver info from registry & init controls
			BOOL fNotif = 0;
			InitBklSettings(hwndPage, &fNotif);
			if(fNotif)
				UpdateBklSettings(g_LastActive);

			
			return TRUE;
		}
 		case WM_HSCROLL:            // track bar message
		{
			switch LOWORD(wParam)
            {
				case TB_ENDTRACK:
					TrackArr.PosChanged((HWND)lParam);//RecheckPos( hwndPage, (HWND)lParam );
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

		}
		break;
        case WM_NOTIFY:
        {
            switch(((NMHDR*)lParam)->code)
            {
				case PSN_SETACTIVE:
				{
					g_LastActive = 0;
					TrackArr.UpdatePos0( g_hDlg1 ); 
				}
				break;
                case PSN_APPLY:     //Notifies a page that the OK button is chosen.                                 
				{
					if( piData = (int *)GetWindowLong(hwndPage, DWL_USER) )
					{
						*piData = 1;

						// update backlight levels
						UpdateBklSettings(g_LastActive);
						RegFlushKey(HKEY_LOCAL_MACHINE);
						RegFlushKey(HKEY_CURRENT_USER);
					}					
                    //Accept the changes and allow the property sheet to be destroyed. 
                    SetWindowLong(hwndPage, DWL_MSGRESULT, PSNRET_NOERROR);
					g_hDlg1 = 0;

					return TRUE; 
				}
                case PSN_RESET:     //Notifies a page that the Cancel button is chosen
                                    //and the property sheet is about to be destroyed.                  
                    if( piData = (int *)GetWindowLong (hwndPage, DWL_USER) )
						*piData = 0;    //Cancel is chosen.

                    SetWindowLong(hwndPage, DWL_MSGRESULT, FALSE);
					g_hDlg1 = 0;

					return TRUE;
            }
            break;
        }
    }
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
//
//  void GetMultiSzFromReg( const TCHAR * lpszRegKey, const TCHAR *pName, DWORD *pParam )    
//
//  Get values from the registry. Set values to 1 in case of query errors.
//
////////////////////////////////////////////////////////////////////////////////
LONG GetMultiSzFromReg( const TCHAR * lpszRegKey, const TCHAR *pName, DWORD *pParam )
{
    HKEY    hKey;

    DWORD status, size, type, qty;
    TCHAR pBuffer[MAX_REGSTR] = {0};
	TCHAR *pPos;

    if(ERROR_SUCCESS != ( status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszRegKey, 0, 0, &hKey)) )
    {
		return status;
	}

    // Get registry value type and size
    status = RegQueryValueEx(hKey, pName, NULL, &type, NULL, &size);
    
	if (status != ERROR_SUCCESS	||	type != REG_MULTI_SZ || 0 == size || MAX_REGSTR * sizeof(TCHAR) < size )
    {
		RegCloseKey(hKey);      
		return (( status != ERROR_SUCCESS ) ? status : -1 );
    }

    // Read registry value
    if( ERROR_SUCCESS == RegQueryValueEx( hKey, pName, NULL, NULL, (UCHAR*)pBuffer, &size ) )
	{
    
		pPos	= pBuffer;
		qty		= STATES_QTY;

		while( qty && *pPos != L'\0' )
		{
			while(*pPos == L' ') 
				pPos++;
            
			*pParam		= wcstoul(pPos, &pPos, 16);
			pParam++;
			qty--;

			if(*pPos == L'\0')
				pPos++;
		}

		RegCloseKey(hKey);      
	}
	
	return ERROR_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////
//
//  void SetMultiSzToReg((LPCTSTR lpszRegKey, LPCTSTR pName, DWORD* pValues) 
//
//  Set values to the regsitry. 
//
//////////////////////////////////////////////////////////////////////////////// 
void SetMultiSzToReg(LPCTSTR lpszRegKey, LPCTSTR pName, DWORD* pValues)    
{
    HKEY    hKey;
    DWORD	dwType;
	DWORD	dwTotalSize = 0, dwSubSize;

	TCHAR	RegStr[MAX_REGSTR]	= {0};
	TCHAR*	pStr				= RegStr;

	if( ERROR_SUCCESS == RegCreateKeyEx( HKEY_LOCAL_MACHINE, lpszRegKey, 0, NULL, 0, 0, NULL, &hKey, &dwType ) )
    {   
		
		for(DWORD i = 0; i < STATES_QTY; i++)
		{
			_ultow(pValues[i], pStr, 16);
			
			dwSubSize = wcslen(pStr) + 1;
			

			pStr		+= dwSubSize;
			
			dwTotalSize += dwSubSize;

			if( ( dwTotalSize + 1 ) > MAX_REGSTR * sizeof(TCHAR) )
			{
				//error NKMSG(_T("DWORD list too long!"));
				break;;
			}
		}

        RegSetValueEx(hKey, pName, 0, REG_MULTI_SZ, (PBYTE)RegStr, ( dwTotalSize + 1 ) * sizeof(TCHAR));
        RegCloseKey(hKey);
    }   
}
void InitDlgSetting(HWND hDlg )
{
	//get from reg
	DWORD DependLsFlag = 1;
	HWND ctl = 0;

	CReg reg;
	if( reg.Open( HKEY_CURRENT_USER, BACKLIGHT_REGKEY ) )
	{
		DependLsFlag = reg.ValueDW(BACKLIGHT_REG_LS, 1);
	}

	if( ctl = GetDlgItem(hDlg, IDC_CHB_LS) )
		SendMessage( ctl, BM_SETCHECK, (DependLsFlag ? 1 : 0 ), 0  );

	RETAILMSG(0,(L"Init AC: %d %d %d %d %d\r\n", ACLevels[0], ACLevels[1], ACLevels[2], ACLevels[5], ACLevels[6]));

	TrackArr.Init( hDlg, IDC_TRACKBAR_BATT_HIGH,	BKL_MAX_SETTING - BatteryLevels[0] );
	TrackArr.Init( hDlg, IDC_TRACKBAR_BATT_NORM,	BKL_MAX_SETTING - BatteryLevels[5] );
	TrackArr.Init( hDlg, IDC_TRACKBAR_BATT_LOW,		BKL_MAX_SETTING - BatteryLevels[6] );
	TrackArr.Init( hDlg, IDC_TRACKBAR_AC_HIGH,		BKL_MAX_SETTING - ACLevels[0] );
	TrackArr.Init( hDlg, IDC_TRACKBAR_AC_NORM,		BKL_MAX_SETTING - ACLevels[5] );
	TrackArr.Init( hDlg, IDC_TRACKBAR_AC_LOW,		BKL_MAX_SETTING - ACLevels[6] );
	
}
void InitBklSettings(HWND hDlg, BOOL* pNotif)
{
	DWORD def = 1, minDuty = 100;
	CReg reg;


	if(GetSystemMetrics(SM_CXSCREEN) == 800)
		def = 0;

	if( reg.Open( HKEY_LOCAL_MACHINE, szRegKey ) )
	{
		if(def)
			minDuty = reg.ValueDW(BL_REG_MIN_LEVEL_DEF, BKL_DEFAULT_DUTY_CYCLE);
		else
			minDuty = reg.ValueDW(BL_REG_MIN_LEVEL_7I, BKL_7INCH_DUTY_CYCLE);
	}
	else
		minDuty = def ? BKL_DEFAULT_DUTY_CYCLE : BKL_7INCH_DUTY_CYCLE;

//	TrackArr.SetMinPos(BKL_MAX_SETTING - minDuty);

	GetMultiSzFromReg( szRegKey, szRegBatteryLevel, BatteryLevels );
	RETAILMSG(0,(L"%d %d %d %d %d %d\r\n", BatteryLevels[0], BatteryLevels[1], BatteryLevels[2], BatteryLevels[5],BatteryLevels[6], minDuty));
	if( minDuty < BatteryLevels[0])
	{
		*pNotif = 1;
		BatteryLevels[0] = minDuty;
	}
	if(minDuty < BatteryLevels[1])
	{
		*pNotif = 1;
		BatteryLevels[1] = minDuty;
	}
	if(minDuty < BatteryLevels[2])
	{
		*pNotif = 1;
		BatteryLevels[2] = minDuty;
	}
	if(minDuty < BatteryLevels[5])
	{
		*pNotif = 1;
		BatteryLevels[5] = minDuty;
	}
	if(minDuty < BatteryLevels[6])
	{
		*pNotif = 1;
		BatteryLevels[6] = minDuty;
	}

	BatteryLevels[0] *= 100;
	BatteryLevels[0] /= minDuty; 
	BatteryLevels[1] *= 100;
	BatteryLevels[1] /= minDuty; 
	BatteryLevels[2] *= 100;
	BatteryLevels[2] /= minDuty; 
	BatteryLevels[5] *= 100;
	BatteryLevels[5] /= minDuty; 
	BatteryLevels[6] *= 100;
	BatteryLevels[6] /= minDuty; 

	RETAILMSG(0,(L"%d %d %d %d %d %d\r\n", BatteryLevels[0], BatteryLevels[1], BatteryLevels[2], BatteryLevels[5],BatteryLevels[6], minDuty));
	TrackArr.Init( hDlg, IDC_TRACKBAR_BT_ON,	BKL_MAX_SETTING - BatteryLevels[0] );
	TrackArr.Init( hDlg, IDC_TRACKBAR_BT_OFF,	BKL_MAX_SETTING - BatteryLevels[1] );
	
	GetMultiSzFromReg( szRegKey, szRegACLevel, ACLevels );
	RETAILMSG(0,(L"%d %d %d %d %d %d\r\n", ACLevels[0], ACLevels[1], ACLevels[2], ACLevels[5], ACLevels[6], minDuty));

	if( minDuty < ACLevels[0])
	{
		*pNotif = 1;
		ACLevels[0] = minDuty;
	}
	if(minDuty < ACLevels[1])
	{
		*pNotif = 1;
		ACLevels[1] = minDuty;
	}
	if(minDuty < ACLevels[2])
	{
		*pNotif = 1;
		ACLevels[2] = minDuty;
	}
	if(minDuty < ACLevels[5])
	{
		*pNotif = 1;
		ACLevels[5] = minDuty;
	}
	if(minDuty < ACLevels[6])
	{
		*pNotif = 1;
		ACLevels[6] = minDuty;
	}
	ACLevels[0] *= 100;
	ACLevels[0] /= minDuty; 
	ACLevels[1] *= 100;
	ACLevels[1] /= minDuty; 
	ACLevels[2] *= 100;
	ACLevels[2] /= minDuty; 
	ACLevels[5] *= 100;
	ACLevels[5] /= minDuty; 
	ACLevels[6] *= 100;
	ACLevels[6] /= minDuty; 

	RETAILMSG(0,(L"%d %d %d %d %d %d\r\n", ACLevels[0], ACLevels[1], ACLevels[2], ACLevels[5], ACLevels[6], minDuty));
	TrackArr.Init( hDlg, IDC_TRACKBAR_AC_ON,	BKL_MAX_SETTING - ACLevels[0] );
	TrackArr.Init( hDlg, IDC_TRACKBAR_AC_OFF,	BKL_MAX_SETTING - ACLevels[1] );

	RETAILMSG(0,(L"minDuty %d notify %d\r\n", minDuty, *pNotif));
}
////////////////////////////////////////////////////////////////////////////////
//
//  void UpdateBklSettings(HWND hDlg) 
//
//  Updates Backlight settings to the registry from trackbar control & signal 
//  change event. 
//
//////////////////////////////////////////////////////////////////////////////// 
void UpdateBklSettings( DWORD LastActive )
{
	//BKL_MAX_SETTING    =     BACKLIGHT_LOW;
	DWORD DependLsFlag = 1, minDuty = 100;
	CReg reg;
	BOOL	def = 1;
	DWORD	BatteryLevelsTmp[sizeof(BatteryLevels)];
	DWORD	ACLevelsTmp[sizeof(ACLevels)];

	if(g_First)//if first - don't update
	{
		if(reg.Open( HKEY_CURRENT_USER, BACKLIGHT_REGKEY ) )
			DependLsFlag = reg.ValueDW(BACKLIGHT_REG_LS, 1);
	}
	else
	{
		DependLsFlag = SendMessage( GetDlgItem(g_hDlg1, IDC_CHB_LS), BM_GETCHECK, 0, 0  );

		if( reg.OpenOrCreateRegKey( HKEY_CURRENT_USER, BACKLIGHT_REGKEY  ) )
		{
			reg.SetDW(BACKLIGHT_REG_LS, DependLsFlag);
		}
	}

	if(GetSystemMetrics(SM_CXSCREEN) == 800)
		def = 0;

	if( reg.Open( HKEY_LOCAL_MACHINE, szRegKey ) )
	{
		if(def)
			minDuty = reg.ValueDW(BL_REG_MIN_LEVEL_DEF, BKL_DEFAULT_DUTY_CYCLE);
		else
			minDuty = reg.ValueDW(BL_REG_MIN_LEVEL_7I, BKL_7INCH_DUTY_CYCLE);
	}
	else
		minDuty = def ? BKL_DEFAULT_DUTY_CYCLE : BKL_7INCH_DUTY_CYCLE;

	if( DependLsFlag && LastActive )//from LS dlg
 		BatteryLevels[0] = BKL_MAX_SETTING - TrackArr.GetBlValue( IDC_TRACKBAR_BATT_HIGH );
	else//from Setting dlg
 		BatteryLevels[0] = BKL_MAX_SETTING - TrackArr.GetBlValue( IDC_TRACKBAR_BT_ON );

	BatteryLevels[1] = BKL_MAX_SETTING - TrackArr.GetBlValue( IDC_TRACKBAR_BT_OFF );

 	if( BatteryLevels[1] < BatteryLevels[0] )
		BatteryLevels[1] = BatteryLevels[0];

	BatteryLevels[2] = BatteryLevels[1] + ( BKL_MAX_SETTING -  BatteryLevels[1] ) / 2;  

	BatteryLevels[3] = BKL_MAX_SETTING;
	BatteryLevels[4] = BKL_MAX_SETTING; 

	if( g_hDlg1 )// get the value if dlg was active only
	{
		BatteryLevels[5] = BKL_MAX_SETTING - TrackArr.GetBlValue( IDC_TRACKBAR_BATT_NORM );
	}

	if( BatteryLevels[5] < BatteryLevels[0] )
		BatteryLevels[5] = BatteryLevels[0];
	else if( BatteryLevels[5] > BatteryLevels[1] )
		BatteryLevels[5] = BatteryLevels[1];

	if( g_hDlg1 )// get the value if dlg was active only
	{
		BatteryLevels[6] = BKL_MAX_SETTING - TrackArr.GetBlValue( IDC_TRACKBAR_BATT_LOW );
	}

	if( BatteryLevels[6] < BatteryLevels[5] )
		BatteryLevels[6] = BatteryLevels[5];
	else if( BatteryLevels[6] > BatteryLevels[1] )
		BatteryLevels[6] = BatteryLevels[1];

	RETAILMSG(0,(L"Bt: %d %d %d %d %d %d\r\n", BatteryLevels[0], BatteryLevels[1], BatteryLevels[2], BatteryLevels[5], BatteryLevels[6], minDuty));
	memcpy(BatteryLevelsTmp, BatteryLevels, sizeof(BatteryLevelsTmp));
	BatteryLevelsTmp[0] = (BatteryLevels[0] * minDuty) / 100;
	BatteryLevelsTmp[1] = (BatteryLevels[1] * minDuty) / 100;
	BatteryLevelsTmp[2] = BatteryLevelsTmp[1] + ( BKL_MAX_SETTING -  BatteryLevelsTmp[1] ) / 2;  
	if(BatteryLevelsTmp[2] > minDuty)
		BatteryLevelsTmp[2] = minDuty;
	BatteryLevelsTmp[3] = BKL_MAX_SETTING;
	BatteryLevelsTmp[4] = BKL_MAX_SETTING;
	BatteryLevelsTmp[5] = (BatteryLevels[5] * minDuty) / 100;
	BatteryLevelsTmp[6] = (BatteryLevels[6] * minDuty) / 100;

	RETAILMSG(0,(L"Bt: %d %d %d %d %d %d\r\n", BatteryLevelsTmp[0], BatteryLevelsTmp[1], BatteryLevelsTmp[2], BatteryLevelsTmp[5], BatteryLevelsTmp[6], minDuty));

	SetMultiSzToReg( szRegKey, szRegBatteryLevel, BatteryLevelsTmp );

	if( DependLsFlag &&  LastActive )//from LS dlg
		ACLevels[0] = BKL_MAX_SETTING - TrackArr.GetBlValue( IDC_TRACKBAR_AC_HIGH );
	else//from Setting dlg
		ACLevels[0] = BKL_MAX_SETTING - TrackArr.GetBlValue( IDC_TRACKBAR_AC_ON );

	ACLevels[1] = BKL_MAX_SETTING - TrackArr.GetBlValue( IDC_TRACKBAR_AC_OFF );

	if( ACLevels[1] < ACLevels[0] )
		ACLevels[1] = ACLevels[0];

	ACLevels[2] = ACLevels[1] + ( BKL_MAX_SETTING -  ACLevels[1] ) / 2;  

	ACLevels[3] = BKL_MAX_SETTING;
	ACLevels[4] = BKL_MAX_SETTING; 

	if( g_hDlg1 )// get the value if dlg was active only
	{
		ACLevels[5] = BKL_MAX_SETTING - TrackArr.GetBlValue( IDC_TRACKBAR_AC_NORM );
	}

	if( ACLevels[5] < ACLevels[0] )
		ACLevels[5] = ACLevels[0];
	else if( ACLevels[5] > ACLevels[1] )
		ACLevels[5] = ACLevels[1];

	if( g_hDlg1 )// get the value if dlg was active only
	{
		ACLevels[6] = BKL_MAX_SETTING - TrackArr.GetBlValue( IDC_TRACKBAR_AC_LOW );
	}

	if( ACLevels[6] < ACLevels[5] )
		ACLevels[6] = ACLevels[5];
	else if( ACLevels[6] > ACLevels[1] )
		ACLevels[6] = ACLevels[1];

	RETAILMSG(0,(L"AC: %d %d %d %d %d %d\r\n", ACLevels[0], ACLevels[1], ACLevels[2], ACLevels[5], ACLevels[6], minDuty));

	ACLevelsTmp[0] = (ACLevels[0] * minDuty) / 100;
	ACLevelsTmp[1] = ACLevels[1] * minDuty / 100;
	ACLevelsTmp[2] = ACLevelsTmp[1] + ( BKL_MAX_SETTING -  ACLevelsTmp[1] ) / 2;	
	if(ACLevelsTmp[2] > minDuty)
		ACLevelsTmp[2] = minDuty;
	ACLevelsTmp[3] = BKL_MAX_SETTING;
	ACLevelsTmp[4] = BKL_MAX_SETTING;
	ACLevelsTmp[5] = (ACLevels[5] * minDuty) / 100;
	ACLevelsTmp[6] = (ACLevels[6] * minDuty) / 100;

	RETAILMSG(0,(L"AC: %d %d %d %d %d %d\r\n", ACLevelsTmp[0], ACLevelsTmp[1], ACLevelsTmp[2], ACLevelsTmp[5], ACLevelsTmp[6], ACLevelsTmp));
	SetMultiSzToReg( szRegKey, szRegACLevel, ACLevelsTmp);
}
void UpdateLables(HWND hDlg )
{
	static TCHAR str[32] = {0};
	DWORD minl, maxl;
	HWND hLabl;

	minl = TrackArr.GetBlValue( IDC_TRACKBAR_BT_OFF );
	maxl = TrackArr.GetBlValue( IDC_TRACKBAR_BT_ON );
	wsprintf( str, _T("%d-%d"), minl, maxl );
	if( hLabl = GetDlgItem( hDlg, IDC_BTT ) )
		SetWindowText( hLabl, str );
	
	minl = TrackArr.GetBlValue( IDC_TRACKBAR_AC_OFF );
	maxl = TrackArr.GetBlValue( IDC_TRACKBAR_AC_ON );
	wsprintf( str, _T("%d-%d"), minl, maxl );
	if( hLabl = GetDlgItem( hDlg, IDC_AC ) )
		SetWindowText( hLabl, str );
}
////////////////////////////////////////////////////////////////////////////////
//
//  void RestoreBklSettings(DWORD dwBattLevel, DWORD dwACLevel)
//
//  Updates Backlight settings to the registry & signal change event. 
//
//////////////////////////////////////////////////////////////////////////////// 
/*void RestoreBklSettings(DWORD dwACLevel)
{
    HANDLE hEvent;
    
    DEBUGMSG(1, (L"Restore: batt=%d AC=%d \r\n", dwBattLevel, dwACLevel));
    
    SetToRegistry(&dwBattLevel, &dwACLevel, szRegKey, szRegBatteryLevel, szRegACLevel);  

    // Signal backlight driver to update
    hEvent = CreateEvent(NULL, FALSE, FALSE, EVENTNAME_BACKLIGHTLEVELCHANGEEVENT);
    if(hEvent != NULL)
    {
        SetEvent(hEvent);
        CloseHandle(hEvent);
        DEBUGMSG(1, (L"Change event triggered\r\n"));
    }
}
*/