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

Abstract: Functions specific to the Keyboard CPL
**/

#include "cplpch.h"
#include <aygshell.h>

typedef struct tagKeybdSettings
{
	int	org_rpt;
	int org_dly;
	int org_disp_rpt;
	int dly_tick;
	int rpt_tick;
	int dly_min;
	int dly_max;
	int rpt_min;
	int rpt_max;
}
KEYBDSETTINGS;

typedef struct KBDI_AUTOREPEAT_INFO KEYBDINFO;

//
// prototypes
//
void GetKbdDevInfo(KEYBDINFO& Info, KEYBDSETTINGS& ks);
void InitSliders(HWND hDlg, KEYBDSETTINGS& ks);
void ShowEnableKeybdControls(HWND hDlg, BOOL fKBEnabled);
void UpdateKeybdSettings(HWND hDlg, KEYBDSETTINGS& ks);
void RestoreKeybdSettings(int pos1, int pos2, int disppos2);
LRESULT CALLBACK MyEditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

extern "C" BOOL APIENTRY KeybdDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	static KEYBDSETTINGS ks;
	static KEYBDINFO 	 Info;
	static BOOL      	 fKBEnabled = TRUE;

    switch (message)
    {
	    case WM_INITDIALOG:
		{	// sub class edit control. save prev wndproc in GWL_USERDATA
			WNDPROC lpfnOldEditProc = (WNDPROC) GetWindowLong(DI(IDC_EDIT), GWL_WNDPROC);
		    SetWindowLong(DI(IDC_EDIT), GWL_USERDATA, (LONG)lpfnOldEditProc);
		    SetWindowLong(DI(IDC_EDIT), GWL_WNDPROC, (LONG)MyEditProc);

			// get keybd driver info & init controls
			GetKbdDevInfo(Info, ks);

			// check if we have a keybd or not
			if (ks.rpt_max <= 0)
				fKBEnabled = FALSE;

			InitSliders(hDlg, ks);
			Button_SetCheck(DI(IDC_CHK_BOX), (ks.org_rpt > 0));	  // if > 0 then repeat is enabled
			SetFocus(GetDlgItem(hDlg, IDC_CHK_BOX));
			ShowEnableKeybdControls(hDlg, fKBEnabled);
			UpdateKeybdSettings(hDlg, ks);
            AygInitDialog( hDlg, SHIDIF_SIPDOWN );

			return 0; // don't let gwe set focus
		}
		case WM_HSCROLL:			// track bar message
			switch LOWORD(wParam) {
				case TB_BOTTOM:
				case TB_THUMBPOSITION:
				case TB_LINEUP:
				case TB_LINEDOWN:
				case TB_PAGEUP:
				case TB_PAGEDOWN:
				case TB_TOP:
					UpdateKeybdSettings(hDlg, ks);
					return TRUE;
				}
			break;

    	case WM_DRAWITEM:
    		DrawCPLButton((DRAWITEMSTRUCT*)lParam,
				((LOWORD(wParam)==IDC_BTN_LONG || LOWORD(wParam)==IDC_BTN_SLOW) ? IDB_LEFT : IDB_RIGHT));
            break;

	    case WM_COMMAND:
            switch (LOWORD(wParam)) 
            {
				case IDC_CHK_BOX:
					ShowEnableKeybdControls(hDlg, fKBEnabled);
					UpdateKeybdSettings(hDlg, ks);
					return TRUE;
					
				case IDC_BTN_LONG:
				case IDC_BTN_SHORT:
				{
					HWND hwndSlider1 = GetDlgItem(hDlg, IDC_TRACKBAR_DLY);
					int pos1 = SendMessage(hwndSlider1, TBM_GETPOS, 0, 0);
					SendMessage( hwndSlider1, TBM_SETPOS, TRUE, pos1 + (LOWORD(wParam) == IDC_BTN_SHORT ? 1:-1));
					UpdateKeybdSettings(hDlg, ks);
					return TRUE;
				}
				case IDC_BTN_FAST:
				case IDC_BTN_SLOW:
				{
					HWND hwndSlider2= GetDlgItem(hDlg, IDC_TRACKBAR_RPT);
					int pos2 = SendMessage(hwndSlider2, TBM_GETPOS, 0, 0);
					SendMessage( hwndSlider2, TBM_SETPOS, TRUE, pos2 + (LOWORD(wParam) == IDC_BTN_FAST ? 1:-1));
					UpdateKeybdSettings(hDlg, ks);
					return TRUE;
				}
				case IDOK:
					UpdateKeybdSettings(hDlg, ks);
					return TRUE;
					
				case IDCANCEL:	
					RestoreKeybdSettings(ks.org_dly, ks.org_rpt, ks.org_disp_rpt);
					return TRUE;
			}
		case WM_SYSCOLORCHANGE:
			// fwd the syscolorchange to our trackbars
			SendMessage(DI(IDC_TRACKBAR_RPT), message, wParam, lParam);
			SendMessage(DI(IDC_TRACKBAR_DLY), message, wParam, lParam);
			break; // return FALSE anyway so DefDlgProc handles it for ourselves
	}
    return FALSE;
}

void GetKbdDevInfo(KEYBDINFO& Info, KEYBDSETTINGS& ks)
{
	int		DelaySettings;
	int		RepeatSettings;
	long	*pInitialDelaySettings = NULL;
	long	*pRepeatRateSettings;
	
	// Get the autorepeat info
	KeybdGetDeviceInfo(KBDI_AUTOREPEAT_INFO_ID, &Info);

	DEBUGMSG(ZONE_KEYBD, (L"KeybdGetDeviceInfo: cInitialDelaysSelectable=%d cRepeatRatesSelectable=%d \r\n", Info.cInitialDelaysSelectable, Info.cRepeatRatesSelectable));
	DEBUGMSG(ZONE_KEYBD, (L"KeybdGetDeviceInfo: current Delay=%d current repeat=%d \r\n", Info.CurrentInitialDelay, Info.CurrentRepeatRate));

	// Allocate buffer for autorepeat settings
	if ( Info.cInitialDelaysSelectable == -1 )
		DelaySettings = 2;
	else
		DelaySettings = Info.cInitialDelaysSelectable;

	if ( Info.cRepeatRatesSelectable == -1 )
		RepeatSettings = 2;
	else
		RepeatSettings = Info.cRepeatRatesSelectable;

	DEBUGMSG(ZONE_KEYBD, (L"DelaySettings=%d RepeatSettings=%d \r\n", DelaySettings, RepeatSettings));

	pInitialDelaySettings = (long*)LocalAlloc(LMEM_FIXED, (DelaySettings + RepeatSettings) * sizeof(INT32));
	pRepeatRateSettings = NULL;

	if(pInitialDelaySettings) {			//Prefix

		pRepeatRateSettings = pInitialDelaySettings + DelaySettings;

		// Get the allowable settings.
		KeybdGetDeviceInfo(KBDI_AUTOREPEAT_SELECTIONS_INFO_ID, pInitialDelaySettings /*&SelInfo*/);

		DEBUGMSG(ZONE_KEYBD, (L"KeybdGetDeviceInfo: cInitialDelaysSelectable=%d cRepeatRatesSelectable=%d \r\n", Info.cInitialDelaysSelectable, Info.cRepeatRatesSelectable));

		// Display and get user settings

		if ( Info.cInitialDelaysSelectable == -1 )
		{
			ks.dly_min = *pInitialDelaySettings;
			ks.dly_max = *(pInitialDelaySettings+1);
			DEBUGMSG(ZONE_KEYBD, (L"dly_min=%d dly_max=%d \r\n", ks.dly_min, ks.dly_max));
		}
		else
		{
			//*pInitialDelaySettings through *(pInitialDelaySettings+DelaySettings-1) are the allowable settings.
			ks.dly_min = *pInitialDelaySettings;
			ks.dly_max = *(pInitialDelaySettings+DelaySettings-1);
			DEBUGMSG(ZONE_KEYBD, (L"2 dly_min=%d dly_max=%d \r\n", ks.dly_min, ks.dly_max));
		}
	}

	if(pRepeatRateSettings) {			//Prefix
		// Similar for repeat rate settings.
		if ( Info.cRepeatRatesSelectable == -1 )
		{
			ks.rpt_min = *pRepeatRateSettings;
			ks.rpt_max = *(pRepeatRateSettings+1);
			DEBUGMSG(ZONE_KEYBD, (L"rpt_min=%d rpt_max=%d \r\n", ks.rpt_min, ks.rpt_max));
		}
		else
		{
			//*pInitialDelaySettings through *(pInitialDelaySettings+DelaySettings-1) are the allowable settings.
			ks.rpt_min = *pRepeatRateSettings;
			ks.rpt_max = *(pRepeatRateSettings+RepeatSettings-1);
			DEBUGMSG(ZONE_KEYBD, (L"2 rpt_min=%d rpt_max=%d \r\n", ks.rpt_min, ks.rpt_max));
		}
	}
	ks.dly_tick = (ks.dly_max- ks.dly_min) / 15;
	ks.rpt_tick = (ks.rpt_max - ks.rpt_min) / 10;	     // 15 tick marks

	// get current settings
	ks.org_dly = Info.CurrentInitialDelay;
	ks.org_disp_rpt = ks.org_rpt = Info.CurrentRepeatRate;
			
	//  get repeat display from registry, if repeat rate is 0
	if (!ks.org_rpt)
	{
		CReg reg(HKEY_CURRENT_USER, RK_CONTROLPANEL_KEYBD);
		ks.org_disp_rpt = reg.ValueDW(RV_DISPDLY);
	}	
	DEBUGMSG(ZONE_KEYBD, (L"org_dly=%d dly_min=%d dly_max=%d  \r\n", ks.org_dly, ks.dly_min, ks.dly_max));
	DEBUGMSG(ZONE_KEYBD, (L"org_disp_rpt=%d \r\n", ks.org_disp_rpt));
	DEBUGMSG(ZONE_KEYBD, (L"org_rpt=%d rpt_min=%d rpt_max=%d  \r\n", ks.org_rpt, ks.rpt_min, ks.rpt_max));


	if (pInitialDelaySettings)
		LocalFree(pInitialDelaySettings);
}

void InitSliders(HWND hDlg, KEYBDSETTINGS& ks)
{
	HWND hwndSlider1 = GetDlgItem(hDlg, IDC_TRACKBAR_DLY);
	HWND hwndSlider2= GetDlgItem(hDlg, IDC_TRACKBAR_RPT);

	SendMessage(hwndSlider1, TBM_SETRANGE, TRUE, MAKELONG(ks.dly_min/ks.dly_tick, ks.dly_max/ks.dly_tick));
	SendMessage(hwndSlider1, TBM_SETTICFREQ, 1, 0L);    // 15 tick marks
	SendMessage(hwndSlider1, TBM_SETPAGESIZE, 0L, 3 );

	SendMessage(hwndSlider2, TBM_SETRANGE, TRUE, MAKELONG(ks.rpt_min/ks.rpt_tick,ks.rpt_max/ks.rpt_tick));
	SendMessage(hwndSlider2, TBM_SETTICFREQ, 1, 0L);
	SendMessage(hwndSlider2, TBM_SETPAGESIZE, 0L, 3);

	SendMessage(hwndSlider1, TBM_SETPOS, TRUE, (ks.dly_max - ks.org_dly + ks.dly_min) / ks.dly_tick);
	SendMessage(hwndSlider2, TBM_SETPOS, TRUE, ks.org_disp_rpt / ks.rpt_tick);
}

const int ShowCtrlTbl[] = { 		// table of all control to be hide/show	if KB repeate is disable/enabled
						IDC_BTN_LONG,
						IDC_BTN_SHORT,
						IDC_BTN_FAST,
						IDC_BTN_SLOW,
						IDC_TRACKBAR_RPT,
						IDC_TRACKBAR_DLY,
						IDI_DELAY,
						IDI_RATE,
						IDC_EDIT,
						IDC_STATIC1,
						IDC_STATIC2,
						IDC_STATIC3,
						IDC_STATIC4,
						IDC_STATIC5,
						IDC_STATIC_L,
						IDC_STATIC_S,
						IDC_STATIC_H,
						IDC_STATIC_F,
						};

const int EnableCtrlTbl[] = { 		// table of all control to be hide/show	if KB repeate is disable/enabled
						IDC_BTN_LONG,
						IDC_BTN_SHORT,
						IDC_BTN_FAST,
						IDC_BTN_SLOW,
						IDC_TRACKBAR_RPT,
						IDC_TRACKBAR_DLY,
						IDC_STATIC1,
						IDC_STATIC2,
						IDC_STATIC4,
						IDC_STATIC5,
						IDC_STATIC_L,
						IDC_STATIC_S,
						IDC_STATIC_H,
						IDC_STATIC_F,
						};

void ShowEnableKeybdControls(HWND hDlg, BOOL fKBEnabled)
{
	int i;
	if(!fKBEnabled)
	{
		for (i = 0; i < ARRAYSIZEOF(ShowCtrlTbl); i++)
			ShowWindow(GetDlgItem(hDlg, ShowCtrlTbl[i]), SW_HIDE);
		return;
	}

	BOOL b = Button_GetCheck(DI(IDC_CHK_BOX));
	for (i = 0; i < ARRAYSIZEOF(EnableCtrlTbl); i++)
	{
		EnableWindow(GetDlgItem(hDlg, EnableCtrlTbl[i]), b);
	}
}

void UpdateKeybdSettings(HWND hDlg, KEYBDSETTINGS& ks)
{
	int pos1, pos2, disppos2;
	
	HWND hwndSlider1 = GetDlgItem(hDlg, IDC_TRACKBAR_DLY);
	HWND hwndSlider2= GetDlgItem(hDlg, IDC_TRACKBAR_RPT);

	pos1= ks.dly_tick * SendMessage(hwndSlider1, TBM_GETPOS, 0, 0) ;
	pos2= ks.rpt_tick * SendMessage(hwndSlider2, TBM_GETPOS, 0, 0);
	if (pos1 < ks.dly_min)
		pos1 = ks.dly_min;
	else if (pos1 > ks.dly_max)
		pos1 = ks.dly_max;
					
	pos1 = ks.dly_max - pos1 + ks.dly_min;		// invert

	if (pos2 < ks.rpt_min)
		pos2 = ks.rpt_min;
	else if (pos2 > ks.rpt_max)
		pos2 = ks.rpt_max;

	DEBUGMSG(ZONE_KEYBD, (L"Read TrackBar: rpt=%d dly=%d \r\n", pos2, pos1));
	disppos2 = pos2;

	if(!Button_GetCheck(DI(IDC_CHK_BOX))) {
		// repeat checkbox is disabled
		pos2 = 0;
	}

	CReg reg;
	reg.Create(HKEY_CURRENT_USER, RK_CONTROLPANEL_KEYBD);
	reg.SetDW(RV_INITIALDELAY, pos1);
	reg.SetDW(RV_REPEATRATE, pos2);
	reg.SetDW(RV_DISPDLY, disppos2);
	
	NotifyWinUserSystem(NWUS_KEYBD_REPEAT_CHANGED);
}

void RestoreKeybdSettings(int pos1, int pos2, int disppos2)
{
	CReg reg;
	reg.Create(HKEY_CURRENT_USER, RK_CONTROLPANEL_KEYBD);
	reg.SetDW(RV_INITIALDELAY, pos1);
	reg.SetDW(RV_REPEATRATE, pos2);
	reg.SetDW(RV_DISPDLY, disppos2);
	
	NotifyWinUserSystem(NWUS_KEYBD_REPEAT_CHANGED);
}

// This function is the "WndProc" for our subclassed Edit control
LRESULT CALLBACK MyEditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CHAR:
		if (GetWindowTextLength(hwnd) < 20)
			break;
		// fall through if test is too long		
	case WM_KILLFOCUS:
		SetWindowText(hwnd, EMPTY_STRING);		// erase text
		break;	
	}
    return CallWindowProc((WNDPROC)GetWindowLong(hwnd, GWL_USERDATA), hwnd, message, wParam, lParam);
}


