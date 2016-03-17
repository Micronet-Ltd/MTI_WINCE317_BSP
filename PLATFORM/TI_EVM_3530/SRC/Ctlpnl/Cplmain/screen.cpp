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

Abstract: Code specific to the Screen CPL
**/

#include "cplpch.h"
#include <commdlg.h>
#include <pm.h>

#define INITGUID
#include <initguid.h>
#include <imaging.h>
#undef INITGUID

#define EXTENSION 5
#define FORMATDESC 32

typedef struct tagDECODERSINFO
{
	TCHAR szExtension[EXTENSION + 1];
	TCHAR szFormatDesc[FORMATDESC];

} DECODERSINFO, *LPDECODERSINFO;

BOOL BrowseForWallpaper(HWND hDlg, LPTSTR pszPath, int iLenPath);
void InitImaging();
void UnInitImaging();
BOOL IsDecoderAvailable(LPCTSTR pszPath);
HBITMAP LoadWallpaperBitmap(HWND hDlg, LPCTSTR pszPath, BOOL fTile);

static void PaintDesktopBitmap(HWND hBox, HDC hdc, HBITMAP hbm, BOOL fTile);
static void PaintWithImaging(HWND hBox, HDC hdc, IImage* pImage, BOOL fTile);

UINT APIENTRY OFNHookProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK BmpBoxWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define STR_SECONDS				_T(" seconds")

//in use only second member of COMBODATA
const COMBODATA rgCBOnBattery[] = {
	IDS_15SECS, 5,
	IDS_15SECS, 10,
	IDS_15SECS, 15,
	IDS_30SECS, 30,
	IDS_1MIN,	60,
	IDS_2MIN,	2*60,
	IDS_2MIN,	3*60,
	IDS_2MIN,	4*60,
	IDS_5MIN,	5*60,
	0, 0
};
const COMBODATA rgCBOnExternal[] = {
	IDS_30SECS, 30,
	IDS_1MIN,	60,
	IDS_2MIN,	2*60,
	IDS_5MIN,	5*60,
	IDS_10MIN,	10*60,
	IDS_15MIN,	15*60,
	IDS_30MIN,	30*60,
	0, 0
};

enum
{
	extbmp = 0,
	ext2bp,
	extgif,
	extjpg
};

static const TCHAR* s_szExt[] = {TEXT("BMP"), TEXT("2bp"), TEXT("GIF"), TEXT("JPG")};
static const TCHAR* s_szExtEx[] = {TEXT("*.BMP"), TEXT("*.2bp"), TEXT("*.GIF"), TEXT("*.JPG")};

//moved to cplreg.h
//#define DEF_BATTERYTIMEOUT 15
//#define DEF_EXTTIMEOUT      60

typedef BOOL (FAR WINAPI *ADVANCEDPROC)(HWND);

/*
BOOL NotifyPowerMgr();
BOOL SetPowerRegTimeouts(DWORD BattUserTimeout, DWORD AcUserTimeout);
BOOL GetPowerRegTimeouts(DWORD& BattUserTimeout, DWORD& AcUserTimeout);

BOOL GetPowerRegTimeouts(DWORD& BattUserTimeout, DWORD& AcUserTimeout)
{
	DWORD dwNoValue = 0xbeef;

	CReg reg(HKEY_LOCAL_MACHINE, FULL_TIMEOUTS_KEY_SZ);

	if (!reg.IsOK()) 
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Could not open power key %s\r\n"), FULL_TIMEOUTS_KEY_SZ));
		return FALSE;
	}
	BattUserTimeout = reg.ValueDW(PM_BATT_USER_IDLE_SZ, dwNoValue);
	if(BattUserTimeout == dwNoValue) 
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Unable to get value %s\r\n"), PM_BATT_USER_IDLE_SZ));
		return FALSE;
	}
	AcUserTimeout = reg.ValueDW(PM_AC_USER_IDLE_SZ, dwNoValue);
	if(AcUserTimeout == dwNoValue) 
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Unable to get value %s\r\n"), PM_BATT_USER_IDLE_SZ));
		return FALSE;
	}

	return TRUE;
}

BOOL SetPowerRegTimeouts(DWORD BattUserTimeout, DWORD AcUserTimeout)
{
	CReg reg(HKEY_LOCAL_MACHINE, FULL_TIMEOUTS_KEY_SZ);

	if (!reg.IsOK()) 
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Could not open power key %s\r\n"), FULL_TIMEOUTS_KEY_SZ));
		return FALSE;
	}

	if(!reg.SetDW(PM_BATT_USER_IDLE_SZ, BattUserTimeout)) 
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Unable to set value %s\r\n"), PM_BATT_USER_IDLE_SZ));
		return FALSE;
	}
	if (!reg.SetDW(PM_AC_USER_IDLE_SZ, AcUserTimeout))
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Unable to set value %s\r\n"), PM_AC_USER_IDLE_SZ));
		return FALSE;
	}
	
	return 1;
}

BOOL NotifyPowerMgr()
{
	// Tell power manager to reload the registry timeouts
	HANDLE hevReloadActivityTimeouts = OpenEvent(EVENT_ALL_ACCESS, FALSE, PM_EVENT_NAME);
	if(!hevReloadActivityTimeouts) 
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Error: Could not open PM reload event\r\n")));
	}
	SetEvent(hevReloadActivityTimeouts);
	CloseHandle(hevReloadActivityTimeouts);
	return TRUE;
}
*/
///////////////////////////////////////////////////////////////
//iii
void LoadCBStr(HWND hwndCB, const COMBODATA rgCBData[]) 
{
	const int iBufSize = 127;
	TCHAR szBuf[iBufSize + 1] = {0};

	DWORD dwTime;
	for(int i=0; rgCBData[i].idcString; i++) 
	{
		dwTime = rgCBData[i].dwData;

		if( 60 > dwTime )
		{
			const TCHAR*	ptr		= 0;
			const TCHAR*	szSec_s = CELOADSZ(IDS_15SECS);// string "15 seconds"
			if( szSec_s )
				ptr = _tcschr( szSec_s, _T('5') );
				
			if( ptr )
			{
				if( *(ptr + 1) != _T('1') )//if it's not previous char
					ptr++;
				else
					ptr--;
			}
			else
				ptr = STR_SECONDS;

			_sntprintf(szBuf, iBufSize, TEXT("%2u%s"), dwTime, ptr);
			}
			else 
			{ 
				dwTime /= 60;
				const TCHAR *szMinute_s = (dwTime == 1) ? CELOADSZ(IDS_POWER_MINUTE) : CELOADSZ(IDS_POWER_MINUTES);
				_sntprintf(szBuf, iBufSize, TEXT("%2u %s"), dwTime, szMinute_s);
			}		
		int iIndex = ComboBox_AddString(hwndCB, szBuf);
		if(iIndex!=CB_ERR) 
		{
			ComboBox_SetItemData(hwndCB, iIndex, rgCBData[i].dwData);
		}
	}
}

extern "C" BOOL APIENTRY BacklightDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	static HINSTANCE hAdvanced;
	static ADVANCEDPROC pfnAdvanced;

	switch (message)
	{
        case WM_INITDIALOG:
        {
			int iBatteryTime;
         int iExternalTime;
            
        	HWND hBatteryCB = GetDlgItem(hDlg, IDC_BBATTERY_TIME);
			HWND hExternalCB = GetDlgItem(hDlg, IDC_BEXTERNAL_TIME);

			// To add more strings, add to table(s) above. Note the external CB
			// includes all the strings in the Battery CB plus extras

////temp!!!!//////////////////////////////////////////
//			TCHAR	txt[64];
//			HWND ctl, hFirst = 0;
//			hFirst = GetDlgItem(hDlg, IDC_BBATTERY);
//			DWORD fRev = 0, style;
//
//			ctl = GetDlgItem(hDlg, IDC_BBATTERY_TIME);
//			ShowWindow(ctl, SW_HIDE);
//			ctl = GetDlgItem(hDlg, IDC_BEXTERNAL_TIME);
//			ShowWindow(ctl, SW_HIDE);
//
//
//			style = GetWindowLong(hFirst, GWL_STYLE);
//			SetWindowLong(hFirst, GWL_STYLE, WS_GROUP | style );
//			
//			ctl = GetNextDlgGroupItem(hDlg, 0, 0);
//			
//			while(ctl = GetNextDlgGroupItem(hDlg, ctl, fRev))
//			{
//				if( hFirst == ctl )
//				{
//					RETAILMSG(0,(_T("first %x \r\n"),hFirst));
//					if( 0 == fRev )
//					{
//						fRev = 1;
//						continue;
//					}
//					break;
//				}
//				GetClassName(ctl, txt, 63);
//				RETAILMSG(0,(_T("%x, class name %s\r\n"),ctl, txt));
//				if( !_tcsncmp(txt,_T("static"),6) )
//				{
//					GetWindowText(ctl, txt, 14);
//					RETAILMSG(0,(_T("%x, %s\r\n"),ctl, txt));
//					if( !_tcsncmp(txt,_T("of continuous"), 13) ||
//						!_tcsncmp(txt,_T("Turn"), 4)			)
//					{
//						ShowWindow(ctl, SW_HIDE);
//					}
//				}
//			}
//			SetWindowLong(hFirst, GWL_STYLE, WS_GROUP | style );
/////////////////////////////////////////////////////////////////////
			//LoadCB(hBatteryCB, rgCBOnBattery);
			//LoadCB(hExternalCB, rgCBOnBattery);
			//LoadCB(hExternalCB, rgCBOnExternal);
			
			LoadCBStr(hBatteryCB, rgCBOnBattery);
			LoadCBStr(hExternalCB, rgCBOnExternal);

			// Read registry & init initial selection
			CReg reg(HKEY_CURRENT_USER, RK_CONTROLPANEL_BACKLIGHT);

			BOOL bEnableBatTime = reg.ValueDW(RV_USEBATTERY, TRUE);
			BOOL bEnableExtTime = reg.ValueDW(RV_USEAC, TRUE);

			if (bEnableBatTime)
			{
				iBatteryTime = reg.ValueDW(RV_BATTERYTIMEOUT, DEF_BATTERYTIMEOUT);
			}
			else 	
			{
				iBatteryTime = reg.ValueDW(RV_OLD_BATTERYTIMEOUT, DEF_BATTERYTIMEOUT);
			}

			if (bEnableExtTime)
			{
				iExternalTime = reg.ValueDW(RV_ACTIMEOUT, DEF_EXTTIMEOUT);
			}
			else
			{
				iExternalTime = reg.ValueDW(RV_OLD_ACTIMEOUT, DEF_EXTTIMEOUT);
			}
			
			ComboBox_SetCurSel(hBatteryCB, ComboBox_FindData(hBatteryCB, iBatteryTime));
			ComboBox_SetCurSel(hExternalCB, ComboBox_FindData(hExternalCB, iExternalTime));

			// enable/disable CB & set checkbox state
			EnableWindow(hBatteryCB, bEnableBatTime);
			Button_SetCheck(DI(IDC_BBATTERY), bEnableBatTime);
			EnableWindow(hExternalCB, bEnableExtTime);
			Button_SetCheck(DI(IDC_BEXTERNAL), bEnableExtTime);

			// see if "Advanced" button needs to be shown
			LPCTSTR pszFileName = reg.ValueSZ(RV_ADVANCEDCPL);
			hAdvanced = NULL;
			pfnAdvanced = NULL;

			if(pszFileName && (hAdvanced = LoadLibrary(pszFileName)))
			{
				pfnAdvanced = (ADVANCEDPROC)GetProcAddress(hAdvanced, FUNCNAME_BACKLIGHTADVAPPLET);
			}
			ShowWindow(DI(IDC_BADVANCED), (pfnAdvanced ? SW_SHOW : SW_HIDE));
			AygInitDialog( hDlg, SHIDIF_SIPDOWN );
			return 1;
		}
		
		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) 
			{
				// enable/disable CBs based on checkbox
				case IDC_BBATTERY:
					EnableWindow(DI(IDC_BBATTERY_TIME), Button_GetCheck(DI(IDC_BBATTERY)));
					return TRUE;

				case IDC_BEXTERNAL:
					EnableWindow(DI(IDC_BEXTERNAL_TIME), Button_GetCheck(DI(IDC_BEXTERNAL)));
					return TRUE;

				// launch advnaced DLL if present
				case IDC_BADVANCED:
					ASSERT(pfnAdvanced);
					
					if (pfnAdvanced)
					{
						if (pfnAdvanced(hDlg))
						{
							HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, EVENTNAME_BACKLIGHTCHANGEEVENT);
							if (hEvent)
							{
								SetEvent(hEvent);
								CloseHandle(hEvent);
							}
						}
					}
					return TRUE;

				// save registry settings
				case IDOK:
				{
					int iBatteryTime = 0;
					int iExternalTime = 0;
					CReg reg;
					reg.Create(HKEY_CURRENT_USER, RK_CONTROLPANEL_BACKLIGHT);

					iBatteryTime = ComboBox_GetItemData(DI(IDC_BBATTERY_TIME), ComboBox_GetCurSel(DI(IDC_BBATTERY_TIME)));
   				iExternalTime = ComboBox_GetItemData(DI(IDC_BEXTERNAL_TIME), ComboBox_GetCurSel(DI(IDC_BEXTERNAL_TIME)));
					ASSERT(iBatteryTime>=0 && iExternalTime>=0);
					
					BOOL bEnableBatTime, bEnableExtTime;
					if (bEnableBatTime = Button_GetCheck(DI(IDC_BBATTERY)))
					{
						reg.SetDW(RV_USEBATTERY, TRUE);
						reg.SetDW(RV_BATTERYTIMEOUT, iBatteryTime);
					}    
				   else
					{
						reg.DeleteValue(RV_BATTERYTIMEOUT);
					   reg.SetDW(RV_USEBATTERY, FALSE);
					   reg.SetDW(RV_OLD_BATTERYTIMEOUT, iBatteryTime);
					}
			      
					if (bEnableExtTime = Button_GetCheck(DI(IDC_BEXTERNAL)))
			      {
						reg.SetDW(RV_USEAC, TRUE);
						reg.SetDW(RV_ACTIMEOUT, iExternalTime);
					}
					else
					{
						reg.DeleteValue(RV_ACTIMEOUT);
						reg.SetDW(RV_USEAC, FALSE);
						reg.SetDW(RV_OLD_ACTIMEOUT, iExternalTime);
					}
/*					
					//Synchronize power timeouts///////////////////////////////////////////////////////////////
					DWORD BattUserTimeout, AcUserTimeout;
			        GetPowerRegTimeouts( BattUserTimeout, AcUserTimeout );

					if( 0 != BattUserTimeout ) 
					{
						if( bEnableBatTime )
							BattUserTimeout = iBatteryTime;
					}

					if( 0 != AcUserTimeout )
					{
						if( bEnableExtTime )
							AcUserTimeout = iExternalTime;
					}
					SetPowerRegTimeouts( BattUserTimeout, AcUserTimeout );
					NotifyPowerMgr();
*/
					// signal the driver to update
					HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, EVENTNAME_BACKLIGHTCHANGEEVENT);
					if (hEvent)
					{
						SetEvent(hEvent);
						CloseHandle(hEvent);
					}
					return TRUE;
				}
			}
			break;
		}//		case WM_COMMAND:

		case WM_DESTROY:
		{	
			MyFreeLib(hAdvanced);
			pfnAdvanced = 0;
		}
		break;
	}
    return (FALSE);
}

static BOOL s_fTile;
static HBITMAP s_hbm;		// must be freed on exit or new bitmap load
static WNDPROC s_pfnOldBmpBoxProc;

// Imaging related
static BOOL s_bImagingBmp = FALSE;
static IImagingFactory * s_pImagingFactory = NULL;
static IImage * s_pImage = NULL;
static LPDECODERSINFO s_pDecodersInfo = NULL;
static UINT s_uDecodersCount = 0;

extern "C" BOOL APIENTRY BackgroundDlgProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	// these statics behave a little like "members" of this WndProc, hence the m_ prefix :-)
	static BOOL m_fChanged;
	static LPCTSTR	m_pszPath;	// always points to strings owned & freed elsewhere
	static CFileCombo* m_pCombo;

	switch (message)
	{
	    case WM_INITDIALOG:
	    {
			// Subclass the background bitmap static control for bitmap painting. Save prev fn ptr
			s_pfnOldBmpBoxProc = (WNDPROC)SetWindowLong(DI(IDC_BOX),  GWL_WNDPROC, (LONG)BmpBoxWndProc);

	    	// Load registry settings & init statics
	    	CReg reg(HKEY_CURRENT_USER, RK_CONTROLPANEL_DESKTOP);
			s_fTile = reg.ValueDW(RV_TILE);
			m_pszPath = reg.ValueSZ(RV_WALLPAPER); // string owned & freed by reg class, NOT us	
			m_fChanged = FALSE;

			// Init checkbox
			Button_SetCheck(DI(IDC_TILEIMAGE), s_fTile);
			
			InitImaging();
					
			// Init the combo box class
			m_pCombo = new CFileCombo(DI(IDC_WALLLIST));						
			if (m_pCombo)
			{
				// Add all *.2bp files and *.bmp files if no imaging is provided
				m_pCombo->AddFileSpec(PATH_WINDOWS, s_szExtEx[ext2bp]);

				if (s_pImagingFactory)
				{
					// Add files with .BMP, .GIF or .JPG extensions
					for (UINT i = 0; i < s_uDecodersCount; i++)
					{						
						m_pCombo->AddFileSpec(PATH_WINDOWS, s_pDecodersInfo[i].szExtension);					
					}
				}

				if (!s_pImagingFactory || (s_pImagingFactory && !s_bImagingBmp))
				{
					m_pCombo->AddFileSpec(PATH_WINDOWS, s_szExtEx[extbmp]);
				}
					
				// Add the 'none' option. Add & select it
				m_pCombo->InsertItem(CELOADSZ(IDS_NONE), EMPTY_STRING, 0);
			}

			MyDeleteObject(s_hbm);
			s_hbm = NULL;
			if (s_pImage)
			{
				s_pImage->Release();
				s_pImage = NULL;
			}

			if (IsDecoderAvailable(m_pszPath))
			{								
				s_pImagingFactory->CreateImageFromFile(m_pszPath, &s_pImage);
			}
			else
			{
				// .*.2bp | no decoders | no .bmp decoder								
				s_hbm = LoadWallpaperBitmap(hDlg, m_pszPath, s_fTile);
			}
			
			// Disable the checkbox if there is no image
			if (!s_hbm && !s_pImage)
			{
				EnableWindow(DI(IDC_TILEIMAGE), FALSE);
			}
			
			if (m_pCombo)
			{
				// Select the currently selected image in the CB (add it if it's missing)
				m_pCombo->SelectByPath(m_pszPath);
			
				// For m_pszPath = NULL case (if registry entry is missing | m_pszPath is out of sync 
				// with CB selection (if registry key is a bogus path/file)) 
				// Get the current CB selection
				m_pszPath = (LPTSTR)ComboBox_GetItemData(DI(IDC_WALLLIST), ComboBox_GetCurSel(DI(IDC_WALLLIST))); // string owned & freed by CFileCombo
				ASSERT(m_pszPath);
				
				// Save current selection to be able to restore it in case of errors
				m_pCombo->SaveSelection();
			}

			InvalidateRect(DI(IDC_BOX), NULL, TRUE);  // force to repaint
			AygInitDialog( hDlg, SHIDIF_SIPDOWN );

			return 1;
		}
		 
		case WM_COMMAND:
	   {
      switch (LOWORD(wParam))
      {
		case IDC_WALLLIST:
		{
			if (CBN_SELCHANGE != HIWORD(wParam))
			{
				return FALSE;
			}
						
			int iSel;

			if (CB_ERR == (iSel = ComboBox_GetCurSel(DI(IDC_WALLLIST))))
			{
				return FALSE;
			}
			
			HBITMAP hbmNew = NULL;			
			IImage* pImageNew = NULL;
			LPTSTR pNewPath = (LPTSTR)ComboBox_GetItemData(DI(IDC_WALLLIST), iSel);
			ASSERT(pNewPath);

			// Enable/disable the Tile checkbox, based on if we have "none" or a real image			
			if (pNewPath[0])
			{
				HRESULT hr = S_OK;
				if (IsDecoderAvailable(pNewPath))
				{
					hr = s_pImagingFactory->CreateImageFromFile(pNewPath, &pImageNew);
					if (SUCCEEDED(hr) && !pImageNew)
					{
						hr = E_FAIL;
					}
				}
				else
				{
					// .*.2bp | no decoders | no .bmp decoder
					hbmNew = LoadWallpaperBitmap(hDlg, pNewPath, s_fTile);
					if(!(hbmNew))
					{
						hr = E_FAIL;
					}
				}				
				
				if (FAILED(hr))
				{
					// Couldn't load an image. Restore previous selection
					ASSERT(FALSE);
					m_pCombo->RestoreSelection();
					return TRUE;
				}

				EnableWindow(DI(IDC_TILEIMAGE), TRUE);
			}
			else
			{
				// if pNewPath is "" that means "(None)" was selected
				s_fTile = 0;
				
				Button_SetCheck(DI(IDC_TILEIMAGE), 0); // uncheck
				EnableWindow(DI(IDC_TILEIMAGE), FALSE);
			}
			
			m_fChanged = TRUE;

			// Clean up previous settings
			MyDeleteObject(s_hbm);
			s_hbm = NULL;
			if (s_pImage)
			{
				s_pImage->Release();
				s_pImage = NULL;
			}

			// Update wallpaper settings
			if (hbmNew)
			{
				s_hbm = hbmNew;
			}
			else
			{				
				s_pImage = pImageNew;				
			}
			m_pszPath = pNewPath;	// string is owned & freed by the CFileCombo class
			m_pCombo->SaveSelection();
			InvalidateRect(DI(IDC_BOX), NULL, TRUE);	// force to repaint
			return TRUE;
		}

		case IDC_TILEIMAGE:
			m_fChanged = TRUE;
			s_fTile = Button_GetCheck(DI(IDC_TILEIMAGE));
			InvalidateRect(DI(IDC_BOX), NULL, TRUE);	// force to repaint
			return TRUE;

		case IDC_BROWSEWALL:
      {					
			TCHAR szPath[MAX_PATH+2];
         			
	      if(BrowseForWallpaper(hDlg, szPath, MAX_PATH))				
	      {
				HBITMAP hbmNew = NULL;
				HRESULT hr = S_OK;
				IImage * pImageNew = NULL;

				if (IsDecoderAvailable(szPath))
				{
					hr = s_pImagingFactory->CreateImageFromFile(szPath, &pImageNew);
				}
				else
				{
					hbmNew = LoadWallpaperBitmap(hDlg, szPath, s_fTile);
				}

				// To successfully proceed: either hbmNew != NULL or pImageNew != NULL
				if (SUCCEEDED(hr) && (hbmNew || pImageNew))
				{
					int iIndex = m_pCombo->SelectByPath(szPath);
					m_pCombo->SaveSelection();
					m_fChanged = TRUE;

					// Clean up previous settings
					MyDeleteObject(s_hbm);
					s_hbm = NULL;
					if (s_pImage)
					{
						s_pImage->Release();
						s_pImage = NULL;
					}
					
					// Update wallpaper settings
					if (hbmNew)
					{
						s_hbm = hbmNew;
					}
					else
					{						
						s_pImage = pImageNew;
					}
					
					m_pszPath = (LPTSTR)ComboBox_GetItemData(DI(IDC_WALLLIST), iIndex); // string owned & freed by CFileCombo
					ASSERT(m_pszPath);
					
					InvalidateRect(DI(IDC_BOX), NULL, TRUE);	// Force to repaint
								
					// Enable the Tile checkbox
					EnableWindow(DI(IDC_TILEIMAGE), TRUE);					
				}
			}
			SetFocus(DI(IDC_BROWSEWALL));
         return TRUE;
		}

		case IDOK:
		{
			if(m_fChanged)
			{
				s_fTile = Button_GetCheck(DI(IDC_TILEIMAGE));
				CReg reg;
				reg.Create(HKEY_CURRENT_USER, RK_CONTROLPANEL_DESKTOP);
				reg.SetDW(RV_TILE, s_fTile);
				ASSERT(m_pszPath);
				reg.SetSZ(RV_WALLPAPER, m_pszPath);
				PostMessage(HWND_BROADCAST, WM_SETTINGCHANGE, SPI_SETDESKWALLPAPER, 0);
			}
			return TRUE;
		}
		}
		break;
		}
			
		case WM_DESTROY:
		{
			MyDeleteObject(s_hbm);
			s_hbm = NULL;
			UnInitImaging();
			break;
		}
	}
	return (FALSE);
}

void InitImaging()
{
	BOOL bRes = FALSE;
		
	if (s_pImagingFactory)
	{
		s_pImagingFactory->Release();
		s_pImagingFactory = NULL;		
	}

	if (s_pDecodersInfo)
	{
		LocalFree(s_pDecodersInfo);
		s_pDecodersInfo = NULL;
	}

	if (s_uDecodersCount)
	{
		s_uDecodersCount = 0;
	}
	
	if (s_bImagingBmp)
	{
		s_bImagingBmp = FALSE;
	}
	
	if (SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
	{
		HRESULT hr = CoCreateInstance(CLSID_ImagingFactory, NULL, CLSCTX_INPROC_SERVER,
					IID_IImagingFactory, (void**) &s_pImagingFactory);
				
		if (SUCCEEDED(hr) && s_pImagingFactory)
		{
			UINT uDecodCount = 0;
			ImageCodecInfo * pDecoders = NULL;

			hr = s_pImagingFactory->GetInstalledDecoders(&uDecodCount, &pDecoders);
			if (SUCCEEDED(hr) && pDecoders && uDecodCount > 0)
			{
				UINT i, uCount = 0;

				for (i = 0; i < uDecodCount; i++)
				{
					if (wcsstr(CharUpper((LPTSTR)pDecoders[i].FilenameExtension), s_szExtEx[extbmp]))
					{
						// BMP decoder is available
						s_bImagingBmp = TRUE;
						uCount++;
						continue;
					}

					if (wcsstr(CharUpper((LPTSTR)pDecoders[i].FilenameExtension), s_szExtEx[extgif]) ||
						wcsstr(CharUpper((LPTSTR)pDecoders[i].FilenameExtension), s_szExtEx[extjpg]))
					{
						uCount++;
					}
				}

				// Get only decoders info (extensions and format) which Control Panel is interested in
				if (0 < uCount && uCount <= uDecodCount && uCount < 4)
				{
					// uCount = 0: there are no decoders we EXPECT to handle under the Control Panel
					
					UINT uCountOut = uCount;
					s_pDecodersInfo = (LPDECODERSINFO)LocalAlloc(LPTR, uCountOut * sizeof(DECODERSINFO));
					if (s_pDecodersInfo)
					{						
						for (i = 0; i < uDecodCount; i++)
						{
							if (wcsstr(CharUpper((LPTSTR)pDecoders[i].FilenameExtension), s_szExtEx[extbmp]))
							{
								if ((uCount - 1) >= 0)
								{									
									if (SUCCEEDED(StringCchCopyEx(s_pDecodersInfo[(uCount - 1)].szExtension, 
														ARRAYSIZE(s_pDecodersInfo[(uCount - 1)].szExtension), 
														s_szExtEx[extbmp], NULL, NULL, STRSAFE_NULL_ON_FAILURE)))
									{
										if (SUCCEEDED(StringCchCopyEx(s_pDecodersInfo[(uCount - 1)].szFormatDesc, 
														ARRAYSIZE(s_pDecodersInfo[(uCount - 1)].szFormatDesc), 
														pDecoders[i].FormatDescription, NULL, NULL, STRSAFE_NULL_ON_FAILURE)))
										{
											uCount--;
											continue;
										}
									}
								}								
							}

							if (wcsstr(CharUpper((LPTSTR)pDecoders[i].FilenameExtension), s_szExtEx[extgif]))
							{
								if ((uCount - 1) >= 0)
								{
									if (SUCCEEDED(StringCchCopyEx(s_pDecodersInfo[(uCount - 1)].szExtension, 
														ARRAYSIZE(s_pDecodersInfo[(uCount - 1)].szExtension), 
														s_szExtEx[extgif], NULL, NULL, STRSAFE_NULL_ON_FAILURE)))
									{
										if (SUCCEEDED(StringCchCopyEx(s_pDecodersInfo[(uCount - 1)].szFormatDesc, 
														ARRAYSIZE(s_pDecodersInfo[(uCount - 1)].szFormatDesc), 
														pDecoders[i].FormatDescription, NULL, NULL, STRSAFE_NULL_ON_FAILURE)))
										{
											uCount--;
											continue;
										}
									}
								}								
							}

							if (wcsstr(CharUpper((LPTSTR)pDecoders[i].FilenameExtension), s_szExtEx[extjpg]))
							{
								if ((uCount - 1) >= 0)
								{
									if (SUCCEEDED(StringCchCopyEx(s_pDecodersInfo[(uCount - 1)].szExtension, 
														ARRAYSIZE(s_pDecodersInfo[(uCount - 1)].szExtension), 
														s_szExtEx[extjpg], NULL, NULL, STRSAFE_NULL_ON_FAILURE)))
									{
										if (SUCCEEDED(StringCchCopyEx(s_pDecodersInfo[(uCount - 1)].szFormatDesc, 
														ARRAYSIZE(s_pDecodersInfo[(uCount - 1)].szFormatDesc), 
														pDecoders[i].FormatDescription, NULL, NULL, STRSAFE_NULL_ON_FAILURE)))
										{
											uCount--;											
										}
									}
								}								
							}
						}

						if (uCount)
						{
							// Clean up in case of failure
							LocalFree(s_pDecodersInfo);
							s_pDecodersInfo = NULL;
							s_bImagingBmp = FALSE;
						}
						else
						{							
							s_uDecodersCount = uCountOut;							
							bRes = TRUE;
						}
					}
				}

				CoTaskMemFree(pDecoders);
			}
			
			// Clean up in case of failures
			if (!bRes)
			{
				s_pImagingFactory->Release();
				s_pImagingFactory = NULL;
			}
		}
	}
}

void UnInitImaging()
{
	if (s_pImagingFactory)
	{
		s_pImagingFactory->Release();
		s_pImagingFactory = NULL;	
	}
	
	if (s_pImage)
	{
		s_pImage->Release();
		s_pImage = NULL;
	}
	
	if (s_pDecodersInfo)
	{
		LocalFree(s_pDecodersInfo);
		s_pDecodersInfo = NULL;	
	}
	
	s_bImagingBmp = FALSE;
	s_uDecodersCount = 0;
	CoUninitialize();
}

BOOL IsDecoderAvailable(LPCTSTR pszPath)
{
	BOOL bRes = TRUE;

	// IsDecoderAvailable returns FALSE if s_pImagingFactory is NULL
	if (wcsstr(CharLower((LPTSTR)pszPath), s_szExt[ext2bp]) || !s_pImagingFactory)
	{
		// pszPath has .2bp extesntion | no imaging support
		bRes = FALSE;
	}
	else
	{
		// Imaging is supported
		if (wcsstr(CharUpper((LPTSTR)pszPath), s_szExt[extbmp]) && !s_bImagingBmp)
		{
			// pszPath has .bmp extension and imaging does not have bmp decoder
			bRes = FALSE;
		}		
	}

	return bRes;
}

HBITMAP LoadWallpaperBitmap(HWND hDlg, LPCTSTR pszPath, BOOL fTile)
{
	HBITMAP hbm = NULL;
	if(pszPath && pszPath[0]) 
	{	
		DEBUGMSG(ZONE_SCREEN, (L"Calling  SHLoadDIBitmap('%s')\r\n", pszPath));
		hbm = SHLoadDIBitmap(pszPath);
		if (!hbm)
		{
			MessageBox(hDlg, 
				CELOADSZ((GetLastError()==ERROR_OUTOFMEMORY) ? IDS_SCREEN_MEMORY_ERROR : IDS_SCREEN_IMAGE_ERROR),
				CELOADSZ(IDS_SCREEN_ERRTITLE), MB_OK | MB_ICONEXCLAMATION);
		}
	}
	return hbm;
}

BOOL BrowseForWallpaper(HWND hDlg, LPTSTR pszPath, int iLenPath)
{
    BOOL bRes = FALSE;
    OPENFILENAME ofn;
    TCHAR szFilter[MAX_PATH];
    
    pszPath[0] = '\0';
    szFilter[0] = '\0';
    
    //
    // Load filter from .res file and convert \1 to \0
    // szFileter has following format: description; extension; double \1 in the end
    //
    int iFilterLen = LOADSZ(IDS_SCREEN_FILTER, szFilter);
    
    if (iFilterLen < 2)
    {
        // This means we have a badly formed filter string or its missing.
        goto Leave;
    }
    
    if (s_pDecodersInfo)
    {		
        // Update szFilter with extensions supported by Imaging and not equal to *.bmp,
        // becuase IDS_SCREEN_FILTER already has *.bmp
        
        // Remove second \1 in the end
        szFilter[iFilterLen - 2] = TEXT('\0');
        for (UINT i = 0; i < s_uDecodersCount; i++)
        {
            if ((0 == wcsncmp(s_pDecodersInfo[i].szExtension, s_szExtEx[extgif], EXTENSION)) ||
                (0 == wcsncmp(s_pDecodersInfo[i].szExtension, s_szExtEx[extjpg], EXTENSION)))
            {
                if (FAILED(StringCchCatEx(szFilter, ARRAYSIZE(szFilter), s_pDecodersInfo[i].szFormatDesc, NULL, NULL, STRSAFE_NULL_ON_FAILURE)) || 
                    FAILED(StringCchCatEx(szFilter, ARRAYSIZE(szFilter), TEXT("\1"), NULL, NULL, STRSAFE_NULL_ON_FAILURE)) ||
                    FAILED(StringCchCatEx(szFilter, ARRAYSIZE(szFilter), s_pDecodersInfo[i].szExtension, NULL, NULL, STRSAFE_NULL_ON_FAILURE)) ||
                    FAILED(StringCchCatEx(szFilter, ARRAYSIZE(szFilter), TEXT("\1"), NULL, NULL, STRSAFE_NULL_ON_FAILURE)))
                {
                    goto Leave;					
                }
            }
        }
        
        // Add (restore) \1 to the end
        if (FAILED(StringCchCatEx(szFilter, MAX_PATH, TEXT("\1"), NULL, NULL, STRSAFE_NULL_ON_FAILURE)))
        {
            goto Leave;
        }
    }
    
    iFilterLen = wcslen(szFilter);
    for (int i = 0; i < iFilterLen; i++)
    {
        if (TEXT('\1') == szFilter[i])
        {
            szFilter[i] = TEXT('\0');
        }
    }
    
    ZEROMEM(&ofn);
    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hDlg;
    ofn.lpstrFilter       = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nFilterIndex      = 1;
    ofn.lpstrFile         = pszPath;
    ofn.nMaxFile          = iLenPath;
    ofn.lpstrInitialDir   = PATH_WINDOWS;
    ofn.lpstrTitle        = CELOADSZ(IDS_BROWSE);
    ofn.Flags             = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
    ofn.lpfnHook          = OFNHookProc;
    ofn.lpstrDefExt       = TEXT("bmp");
    
    bRes = GetOpenFileName(&ofn);
Leave: ;
       return bRes;
}

UINT APIENTRY OFNHookProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
	OFNOTIFY *lpofnotify = (OFNOTIFY *)lp;

	if(WM_NOTIFY == msg && lpofnotify && CDN_TYPECHANGE == lpofnotify->hdr.code)
	{
		TCHAR szExt[EXTENSION] = TEXT("");
		LPTSTR pszFileterTmp = (LPTSTR)lpofnotify->lpOFN->lpstrFilter;		
		int iIndex = lpofnotify->lpOFN->nFilterIndex;
		
		// Find the extension which corresponds to the iIndex
		for (int i = 0; i < (2 * (iIndex - 1) + 1); i++)
		{
			while (TEXT('\0') != *pszFileterTmp)
			{
				pszFileterTmp++;
			}
			pszFileterTmp++;			
		}
		
		HRESULT hr = StringCchCopy(szExt, EXTENSION, pszFileterTmp);

		if (SUCCEEDED(hr) && (iIndex >= 1) && (iIndex <= 4))
		{
			CommDlg_OpenSave_SetDefExt(hdlg, szExt);
		}
		else
		{
			CommDlg_OpenSave_SetDefExt(hdlg, NULL);
		}
	}
	return 0; 	// NOTE: Always return 0
}

// To paint a background image
LRESULT CALLBACK BmpBoxWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (WM_PAINT == uMsg)
	{
		PAINTSTRUCT ps = {0};
		RECT rcBox = {0};
      
		HDC hdc = BeginPaint(hwnd, &ps);		
		GetClientRect(hwnd, &rcBox);

		HBRUSH hBkBrush = CreateSolidBrush(GetSysColor(COLOR_BACKGROUND));
		if (hBkBrush)
		{
			FillRect(hdc, &rcBox, hBkBrush);
			DeleteObject(hBkBrush);
		}

		if (s_hbm)
		{
			// Image is a *.2bp || *.bmp (if Imaging is not supported)			
			PaintDesktopBitmap(hwnd, hdc, s_hbm, s_fTile);
		}
		else
		{
			if (s_pImage) 
			{				
				PaintWithImaging(hwnd, hdc, s_pImage, s_fTile);
			}
		}

      EndPaint(hwnd, &ps);
		return 0;
	}
	return CallWindowProc(s_pfnOldBmpBoxProc, hwnd, uMsg, wParam, lParam);
}

static void PaintWithImaging(HWND hBox, HDC hdc, IImage* pImage, BOOL fTile)
{
	ImageInfo ii = {0};
	RECT rcBox = {0};
	
	GetClientRect(hBox, &rcBox);	
	if (SUCCEEDED(pImage->GetImageInfo(&ii)) && (0 < ii.Width && 0 < ii.Height))
	{
		// Get the desktop dimentions
		int iCXScreen = GetSystemMetrics(SM_CXSCREEN);
		int iCYScreen = GetSystemMetrics(SM_CYSCREEN);

		if (iCXScreen && iCYScreen)
		{
			// Scale the image
			IImage* pImageThumb = NULL;
			UINT uScaledWidth = (ii.Width * (rcBox.right - rcBox.left)) / (iCXScreen);
			UINT uScaledHeight = (ii.Height * (rcBox.bottom - rcBox.top)) / (iCYScreen);

			if (uScaledWidth && uScaledHeight &&
				SUCCEEDED(pImage->GetThumbnail(uScaledWidth, uScaledHeight, &pImageThumb))
				&& pImageThumb)
			{
				RECT rcDest = {0};
				SetRect(&rcDest, 0, 0, uScaledWidth, uScaledHeight);
				
				HDC hdcSrc = CreateCompatibleDC(hdc);
				HBITMAP hbmSrc = CreateCompatibleBitmap(hdc, uScaledWidth, uScaledHeight);
				HGDIOBJ horg = SelectObject(hdcSrc, hbmSrc);
				
				if (hdcSrc && hbmSrc && horg && SUCCEEDED(pImageThumb->Draw(hdcSrc, &rcDest, NULL)))
				{
					if (fTile)
					{
						HBRUSH hBrush = CreatePatternBrush(hbmSrc);
						if (hBrush)
						{
							HBRUSH hBrushOld = (HBRUSH) SelectObject(hdc, hBrush);
							PatBlt(hdc, 0, 0, (rcBox.right - rcBox.left), (rcBox.bottom - rcBox.top),  PATCOPY);

							if (hBrushOld)
							{
								SelectObject(hdc, hBrushOld);
							}
							DeleteObject(hBrush);							
						}
					}
					else
					{
						// Center in the rcBox
						int horzOffset = (rcBox.right - rcBox.left)/2 - (uScaledWidth/2);
						int vertOffset = (rcBox.bottom - rcBox.top)/2 - (uScaledHeight/2);
						OffsetRect(&rcDest, horzOffset, vertOffset);
						BitBlt(hdc, rcDest.left,  rcDest.top, uScaledWidth, uScaledHeight,  hdcSrc, 0, 0, SRCCOPY);
					}
				}

				SelectObject(hdcSrc, horg);
				if (hbmSrc)
				{
					DeleteObject(hbmSrc);
				}

				if (hdcSrc)
				{
					DeleteDC(hdcSrc);
				}

				pImageThumb->Release();
				pImageThumb = NULL;
			}
		}
	}
}

static void PaintDesktopBitmap(HWND hBox, HDC hdc, HBITMAP hbm, BOOL fTile)
{
	BITMAP bm = {0};
	HBITMAP hbmpScaled;
	HDC hdcBmp, hdcScaled;
	HGDIOBJ hOldBmp, hOldScaledBmp;	
	INT iTop = 0, iLeft = 0;
	INT iScaledBmpWth = 0, iScaledBmpHgt = 0;
	RECT rcBox = {0};
	
	if (!hbm)
	{
		return;
	}
	
	GetClientRect(hBox, &rcBox);

	// Scale and display the background bitmap
	GetObject(hbm, sizeof(BITMAP), &bm);
   iScaledBmpWth = bm.bmWidth * 4 / 10;
   iScaledBmpHgt = bm.bmHeight * 4 / 10;
	hdcBmp  = CreateCompatibleDC(hdc);	
	if (hdcBmp)
	{
		hOldBmp = SelectObject(hdcBmp, hbm);
	}

	// Scale the bitmap down to 40%
	hdcScaled  = CreateCompatibleDC(hdc);
   hbmpScaled = CreateCompatibleBitmap(hdc, iScaledBmpWth, iScaledBmpHgt);
   hOldScaledBmp = SelectObject(hdcScaled, hbmpScaled);
   StretchBlt(hdcScaled, 0, 0, iScaledBmpWth, iScaledBmpHgt, 
					hdcBmp, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
	if (fTile)
	{
      INT iBltTop = 0;
      INT iBltLeft = 0;

      while (iBltTop < rcBox.bottom && iScaledBmpHgt)
		{
			INT iBltHeight = (iScaledBmpHgt > rcBox.bottom - iBltTop) ?
									rcBox.bottom - iBltTop : iScaledBmpHgt;

			while (iBltLeft < rcBox.right && iScaledBmpWth)
			{
				INT iBltWidth = (iScaledBmpWth > rcBox.right - iBltLeft) ?
									rcBox.right - iBltLeft : iScaledBmpWth;

				BitBlt(hdc, iBltLeft, iBltTop, iBltWidth, iBltHeight, hdcScaled, 0, 0, SRCCOPY);
				iBltLeft += iBltWidth;
			}
			iBltTop += iBltHeight;
			iBltLeft = 0;
		}
	}
   else
	{
		// Center the bitmap in the box
	   if (iScaledBmpHgt < rcBox.bottom)
		{
			iTop    += (rcBox.bottom - iScaledBmpHgt) / 2;
		}
		if (iScaledBmpWth < rcBox.right)
		{
			iLeft   += (rcBox.right - iScaledBmpWth) / 2;
		}
		    
		BitBlt(hdc, iLeft, iTop, rcBox.right, rcBox.bottom, hdcScaled, 0, 0, SRCCOPY);
	}

	// Recycle drawing resources
	if (hdcBmp)
	{
		SelectObject(hdcBmp, hOldBmp);
		DeleteDC(hdcBmp);
	}

	if (hdcScaled)
	{
		SelectObject(hdcScaled, hOldScaledBmp);
		DeleteDC(hdcScaled);
	}

	DeleteObject(hbmpScaled);

	DEBUGMSG(ZONE_SCREEN, (L"WM_PAINT: PaintDesktopBitmap iLeft=%d, iTop=%d, s_hbmCurrWallPaper=%d hdc=%d Tiling =%d\r\n", iLeft, iTop, hbm, hdc, fTile));
}

/***
void PaintWallpaperBitmap(HWND hDlg, HBITMAP hbm, BOOL fTile, int idcBox, HDC hdc)
{
	RECT rc = {0}, rc1 = {0};
		
	if (!hbm)
	{
		return;
	}

	GetWindowRect(hDlg, &rc1);
	GetWindowRect(GetDlgItem(hDlg, idcBox), &rc);

	if (fTile)
	{
		OffsetRect(&rc, -rc1.left, -rc1.top);
		FillRect(hdc, &rc, (HBRUSH)hbm);
	}
	else
	{
		BITMAP bm;
		HGDIOBJ horg;
		HDC hdcMem;
		int w, h, x, y;

		x = rc.left - rc1.left; 
		y = rc.top  - rc1.top; 

		GetObject(hbm, sizeof(BITMAP), &bm);	

		hdcMem = CreateCompatibleDC(hdc);
		horg = SelectObject(hdcMem, hbm);
		
		// Center
		w = rc.right - rc.left;
		h = rc.bottom - rc.top;
		if (bm.bmHeight < h)
		{
			y += (h - bm.bmHeight) / 2;
		}
		if (bm.bmWidth < w)
		{
			x += (w - bm.bmWidth) / 2;
		}
			
		BitBlt(hdc, x,  y, w, h, hdcMem, 0, 0, SRCCOPY);
		SelectObject(hdcMem, horg);
		DeleteDC(hdcMem);
	}
}
***/
