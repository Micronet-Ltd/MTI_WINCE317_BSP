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

Abstract: Code specific to the Volume tab of the Sounds CPL
**/

#include "cplpch.h"

#define CLICK_LOUD 			0x10000
#define QUIET 				1
#define LOUD 				2

#define MUTE_NOTIFICATION	0x01
#define MUTE_APPS			0x02
#define MUTE_EVENTS		 	0x04

#define MAX_VOLUME			5
#define MAX_SPEAKER_VOLUME	0x0ffff

// control IDs of (1) The "Key clicks" line (2) The "Screen taps" line
const int rgClickTap[2][3] =
{
 	IDC_CHK_KEY,IDC_RADIO1,IDC_RADIO2,
    IDC_CHK_TAP,IDC_RADIO3,IDC_RADIO4,
};

#define SHOW_KEYCLICKS	5	// bit 0 or bit 2
#define SHOW_SCREENTAPS	2	// bit 1
#define SHOW_HWBUTTONS	4	// bit 2

class CVolSettings
{

private:
	HWND hDlg;
	
	// save settings for restore on cancel
	int old_keyclick_volume;
	int old_touchclick_volume;
	int old_muting;
	ULONG old_volume;
	// current settings
	int keyclick_volume;
	int touchclick_volume;
	int muting;
	ULONG volume;
public:
	BOOL 	fVertical; // whether vol trackbar is vertical or horiz
	DWORD	dwClickTap; // Bit0=have keybd, Bit1=have touch screen Bit2==have HW buttons

private:
	int GetSliderPos() {
		int pos = ((volume & 0x0ffff) * MAX_VOLUME)/ MAX_SPEAKER_VOLUME;
		ASSERT(pos>=0 && pos<=MAX_VOLUME);
		if(fVertical) 
			pos = MAX_VOLUME - pos; // if vertical, invert slider scale
		return pos;
	}

	void EnableWindows(HWND hDlg)
	{
		EnableWindow(DI(IDC_RADIO1), LOWORD(keyclick_volume));
		EnableWindow(DI(IDC_RADIO2), LOWORD(keyclick_volume));
		EnableWindow(DI(IDC_RADIO3), LOWORD(touchclick_volume));
		EnableWindow(DI(IDC_RADIO4), LOWORD(touchclick_volume));
		EnableWindow(DI(IDC_CHK_NOT), (muting & MUTE_APPS));
	}

public:
	CVolSettings(HWND h)
	{
		ZEROMEM(this);
		hDlg = h;
	}
	
	void LoadSettings(void)
	{
		UINT uErr;
		CReg reg(HKEY_CURRENT_USER, RK_CONTROLPANEL_VOLUME);
		old_keyclick_volume = keyclick_volume = reg.ValueDW(RV_KEY);
		old_touchclick_volume = touchclick_volume = reg.ValueDW(RV_SCREEN);
		old_muting = muting = reg.ValueDW(RV_MUTE);
		volume = 0; // if waveOutGetVolume fails, then this should be zero
		//volume = reg.ValueDW(RV_VOLUME);
    	if(uErr = waveOutGetVolume(0, &volume))
			RETAILMSG(1, (L"waveOutGetVolume error:%d \r\n", uErr));
		old_volume = volume;
		CReg reg2(HKEY_LOCAL_MACHINE, RK_CONTROLPANEL);
		dwClickTap = reg2.ValueDW(RV_INPUTCONFIG);
		DEBUGMSG(ZONE_SOUNDS, (L"Read from registry keyclick_volume=%x touchclick_volume=%x muting=%x volume=%x dwClickTap=%x\r\n",
					keyclick_volume, touchclick_volume, muting, volume, dwClickTap));
	}

	void SaveSettings(BOOL fRestore=FALSE)
	{
		UINT uErr;
		CReg reg;
		if(fRestore) 
		{
			DEBUGMSG(ZONE_SOUNDS, (L"RESTORING to registry\r\n"));
			keyclick_volume = old_keyclick_volume;
			touchclick_volume = old_touchclick_volume;
			muting = old_muting;
			volume = old_volume;
		}
		DEBUGMSG(ZONE_SOUNDS, (L"Saving to registry keyclick_volume=%x touchclick_volume=%x muting=%x volume=%x\r\n",
					keyclick_volume, touchclick_volume, muting, volume));
					
		reg.Create(HKEY_CURRENT_USER, RK_CONTROLPANEL_VOLUME);
		reg.SetDW(RV_KEY, keyclick_volume);
		reg.SetDW(RV_SCREEN, touchclick_volume);
		reg.SetDW(RV_MUTE, muting);
		reg.SetDW(RV_VOLUME, volume);
		
		if ((uErr = waveOutSetVolume(0, volume)))
        	RETAILMSG(1,(L"waveOutSetVolume error:%d \r\n", uErr));

		// Tell Wave driver to reload settings
		AudioUpdateFromRegistry();
	}

	void InitDialog()
	{
      	// init the slider
		HWND hwndSlider = GetDlgItem(hDlg, IDC_TRACKBAR);
		
		// The slider is vertical if it's width is less than it's height
		// must set this *before* calling GetSliderPos
		RECT rc;
		GetWindowRect(hwndSlider, &rc);
		fVertical = ((rc.right-rc.left) < (rc.bottom-rc.top));

		SendMessage(hwndSlider, TBM_SETRANGE, TRUE, MAKELONG(0, MAX_VOLUME));
		SendMessage(hwndSlider, TBM_SETTICFREQ, 1, 0L);
		SendMessage(hwndSlider, TBM_SETPAGESIZE, 0L, 1);
		SendMessage(hwndSlider, TBM_SETPOS, TRUE, GetSliderPos());

		// keyclick
		Button_SetCheck(DI(IDC_CHK_KEY), LOWORD(keyclick_volume));
		Button_SetCheck(DI(IDC_RADIO1), (keyclick_volume & (LOUD|CLICK_LOUD)));
		Button_SetCheck(DI(IDC_RADIO2), !(keyclick_volume & (LOUD|CLICK_LOUD)));

		// touch click
		Button_SetCheck(DI(IDC_CHK_TAP), LOWORD(touchclick_volume));
		Button_SetCheck(DI(IDC_RADIO3), (touchclick_volume & (LOUD|CLICK_LOUD)));
		Button_SetCheck(DI(IDC_RADIO4), !(touchclick_volume & (LOUD|CLICK_LOUD)));

		ShowHide(hDlg, rgClickTap[0], 3, (dwClickTap & SHOW_KEYCLICKS) ? SW_SHOW : SW_HIDE);
		ShowHide(hDlg, rgClickTap[1], 3, (dwClickTap & SHOW_SCREENTAPS)? SW_SHOW : SW_HIDE);
		// if first line is hidden & 2nd line is visible, move the 2nd line up)
		if(!(dwClickTap & SHOW_KEYCLICKS) && (dwClickTap & SHOW_SCREENTAPS))
		{
			POINT pt;
			Controldxdy(hDlg, rgClickTap[1][0], rgClickTap[0][0], &pt);
			MoveControls(hDlg, rgClickTap[1], 3, 0, pt.y);
		}
		// if neither line is visible, hide the heading
		if(!(dwClickTap & SHOW_KEYCLICKS) && !(dwClickTap & SHOW_SCREENTAPS))
			ShowWindow(DI(IDC_ENABLECLICKTAP), SW_HIDE);
		// if we have hardware buttons instead of keybd, change the text
		if(dwClickTap & SHOW_HWBUTTONS)
			SetWindowText(DI(IDC_CHK_KEY), CELOADSZ(IDS_HARDWAREBUTTONS));

		// mute sounds
		Button_SetCheck(DI(IDC_CHK_EVTS), (muting & MUTE_EVENTS));
		Button_SetCheck(DI(IDC_CHK_APPS), (muting & MUTE_APPS));
		Button_SetCheck(DI(IDC_CHK_NOT),  (muting & (MUTE_NOTIFICATION|MUTE_APPS)) == (MUTE_NOTIFICATION|MUTE_APPS));
		EnableWindows(hDlg);
	}

	void UpdateVolume(int iIncr=0)
	{
		UINT uErr;
		int pos = SendMessage(DI(IDC_TRACKBAR), TBM_GETPOS, 0, 0);
		if(iIncr)
		{
			if(fVertical)
				iIncr = -iIncr; // if vertical, invert slider incr
				
			int newpos = pos + iIncr;
			if(newpos>=0 && newpos<=MAX_VOLUME)
			{
				pos += iIncr;
				SendMessage(DI(IDC_TRACKBAR), TBM_SETPOS, TRUE, pos);
			}
		}
		ASSERT(pos>=0 && pos<=MAX_VOLUME);
		if(fVertical)
			pos = MAX_VOLUME - pos;  // if vertical, invert slider scale
		volume = pos * MAX_SPEAKER_VOLUME / MAX_VOLUME;
		volume = MAKELONG(LOWORD(volume), LOWORD(volume)); // left & right channels
		DEBUGMSG(ZONE_SOUNDS, (L"Setting Volume to: %x pos=%d\r\n", volume, pos));
		if ((uErr = waveOutSetVolume(0, volume)))
		{
			volume = 0;
			waveOutGetVolume(0, &volume);
			SendMessage(DI(IDC_TRACKBAR), TBM_SETPOS, TRUE, GetSliderPos());
			RETAILMSG(1, (L"waveOutSetVolume error:%d \r\n", uErr));
		}
		else
		{
			if (!sndPlaySound(TEXT("SystemDefault"), SND_ALIAS | SND_ASYNC))
				RETAILMSG(1, (L"sndPlaySound return FALSE\r\n"));
		}
	}

	void RefreshVolume(void)
	{
		//Prefix
	  	ULONG temp = 0;
		DEBUGMSG(ZONE_SOUNDS, (L"Vol CPL: set focus\r\n"));
	   	if(0==waveOutGetVolume(0, &temp) && temp!=volume)
	   	{
			DEBUGMSG(ZONE_SOUNDS, (L"Vol CPL: vol changed under us %d-->%d\r\n", volume, temp));
			volume = temp;
			SendMessage(DI(IDC_TRACKBAR), TBM_SETPOS, TRUE, GetSliderPos());
		}
	}

	void ProcessMuteEvent(int idc)
	{
		int mute_mask=0;
		
		switch(idc)
		{
		default: ASSERT(FALSE); break;
		case IDC_CHK_EVTS: mute_mask = MUTE_EVENTS; break;
		case IDC_CHK_APPS: mute_mask = MUTE_APPS; break;
		case IDC_CHK_NOT:  mute_mask = MUTE_NOTIFICATION; break;
		}
			
 		if(Button_GetCheck(DI(idc)))
			muting |= mute_mask;
		else
			muting &= ~mute_mask;
		EnableWindows(hDlg);
		SaveSettings();
	}

	// fKey == key or touch event
	// fCheck == checkbox or radio event
	// iRadio == 0 or 1 for 1st & 2nd radio buttons
	void ProcessKeyTouchEvent(int idc)
	{
		// which volume are we munging? KeyClick or TouchClick?
		BOOL fKey = (idc==IDC_CHK_KEY || idc==IDC_RADIO1 || idc==IDC_RADIO2);
		int* pVol = (fKey ? &keyclick_volume : &touchclick_volume);

		if(idc==IDC_CHK_KEY || idc==IDC_CHK_TAP)
		{
			if(!Button_GetCheck(DI(idc)))
			{
				// turn OFF this type of click, by zapping low 16 bits
				*pVol &= 0xffff0000;
			}
			else
			{
				// set volume based on which radio button is selected
				*pVol = Button_GetCheck(DI(fKey ? IDC_RADIO1 : IDC_RADIO3)) ?
							(LOUD | CLICK_LOUD) : QUIET;			 
			}
		}
		else
		{
			if(idc==IDC_RADIO2 || idc==IDC_RADIO4)
			{
				*pVol = QUIET;
			}
			else
			{
				ASSERT(idc==IDC_RADIO1 || idc==IDC_RADIO3);
				*pVol = (LOUD | CLICK_LOUD);
			}
		}
		EnableWindows(hDlg);
		SaveSettings();
	}
};

extern "C" BOOL APIENTRY VolumeDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static CVolSettings *pVolSettings = NULL;


	if ((NULL == pVolSettings) && (WM_INITDIALOG != message) && (WM_SYSCOLORCHANGE != message))
	{
		// Initdialog creates the settings object and color change doesn't use it
		// but all other messages have to have the settings pointer or they will crash
		DEBUGMSG(ZONE_ERROR, (L"VolumeDlgProc non init message recived when pVolSettings is NULL"));
		return FALSE;
	}


    switch (message)
    {
    case WM_INITDIALOG:
        if( waveOutGetNumDevs() == 0 )
        {
            WCHAR wszError[ MAXERRORLENGTH ];
            waveOutGetErrorText( MMSYSERR_NODRIVER, wszError, MAXERRORLENGTH );
            MessageBox( hDlg, wszError, NULL, MB_OK | MB_ICONEXCLAMATION );
            DestroyWindow( hDlg );
        }
        else
        {
      	    // create our context object & load current settings
		    pVolSettings = new CVolSettings(hDlg);
		    //Prefix
		    if(pVolSettings) {
			    pVolSettings->LoadSettings();
			    // init dialog based on settings
			    pVolSettings->InitDialog();
		    }
		    //
        }
        return 1;

	case WM_HSCROLL:			// track bar message
	case WM_VSCROLL:			// track bar message
		switch LOWORD(wParam)
		{
		case TB_BOTTOM:
		case TB_THUMBPOSITION:
		case TB_LINEUP:
		case TB_LINEDOWN:
		case TB_PAGEUP:
		case TB_PAGEDOWN:
		case TB_TOP:
			pVolSettings->UpdateVolume();
			break;
		}
		break;
		
	case WM_DRAWITEM:
		if(pVolSettings->fVertical)
      		DrawCPLButton((LPDRAWITEMSTRUCT) lParam, (LOWORD(wParam)==IDC_HIGH) ? IDB_UP : IDB_DOWN);
      	else
      		DrawCPLButton((LPDRAWITEMSTRUCT) lParam, (LOWORD(wParam)==IDC_HIGH) ? IDB_RIGHT : IDB_LEFT);
      	break;

	case WM_COMMAND:
    	switch (LOWORD(wParam))
    	{
		case IDC_LOW:
		case IDC_HIGH:
			pVolSettings->UpdateVolume((LOWORD(wParam) == IDC_HIGH) ? 1: -1);
			return TRUE;

		case IDC_CHK_EVTS:
		case IDC_CHK_APPS:
		case IDC_CHK_NOT:
			pVolSettings->ProcessMuteEvent(LOWORD(wParam));
			return TRUE;
					
		case IDC_CHK_KEY:
		case IDC_CHK_TAP:
		case IDC_RADIO1:               
		case IDC_RADIO2:               
		case IDC_RADIO3:          
		case IDC_RADIO4:              
			pVolSettings->ProcessKeyTouchEvent(LOWORD(wParam));
			return TRUE;

		case IDOK:
			pVolSettings->SaveSettings();
			return TRUE;
				
		case IDCANCEL:	
			pVolSettings->SaveSettings(TRUE); // restore original
			return TRUE;
		}
		break;
	case WM_DESTROY:
		delete pVolSettings;
		pVolSettings = NULL;
		break;
	case WM_SYSCOLORCHANGE:
		// fwd the syscolorchange to our trackbar
		SendMessage(DI(IDC_TRACKBAR), message, wParam, lParam);
		break; // return FALSE anyway so DefDlgProc handles it for ourselves

	case WM_ACTIVATE:
		if(LOWORD(wParam) != WA_INACTIVE)
			pVolSettings->RefreshVolume();
		break;
    }
    return (FALSE);
}

