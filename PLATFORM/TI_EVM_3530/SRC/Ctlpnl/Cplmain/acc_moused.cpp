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

Abstract: Accessibility Options\ Mouse tab.
**/
#include "cplpch.h"
#include "accglobl.h"

#define MAXSPEED 9

extern BOOL SoundOnActivation;
MOUSEKEYS MouseKeyParam;
BOOL InitMouse = FALSE;

// Pixels per second. Valid values are from 10 to 360.
UINT SpeedTable[MAXSPEED] = {
	{ 10  },
	{ 20  },
	{ 30  },
	{ 40  },
	{ 60  },
	{ 80  },
	{ 120 },
	{ 180 },
	{ 360 }, };

extern "C" BOOL CALLBACK MouseKeyDlg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/************************************************************************************
Routine Description:
    The dialog procedure for processing the Mouse accessibility tab.
   
Arguments:
    Regular DlgProc
    
Return Value:
    BOOL value based on processed message 
************************************************************************************/
extern "C" BOOL CALLBACK AccessMouseDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{       
	MOUSEKEYS TmpMK;
	
    switch (message)     
    {
        case WM_INITDIALOG:
		{			
			InitMouseVals();
			InitMouse = TRUE;

			CheckDlgButton(hDlg, IDC_ACC_USEMOUSEKEYS, (MouseKeyParam.dwFlags & MKF_MOUSEKEYSON));

			if (!(MouseKeyParam.dwFlags & MKF_AVAILABLE)) {
				EnableWindow(GetDlgItem(hDlg, IDC_ACC_MSETTINGS),FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_ACC_USEMOUSEKEYS),FALSE);
			}

            AygInitDialog( hDlg, SHIDIF_SIPDOWN );

            return TRUE;
		}
		
		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID(wParam, lParam)) 
			{
				case IDC_ACC_USEMOUSEKEYS:					
					MouseKeyParam.dwFlags ^= MKF_MOUSEKEYSON;

					return TRUE;

				case IDC_ACC_MSETTINGS:
					TmpMK = MouseKeyParam;
					if(IDCANCEL == DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ACCD_MOUSESETTINGS), hDlg, MouseKeyDlg))
					{
						MouseKeyParam = TmpMK;
					}
					
					return TRUE;

				case IDOK:
					if(InitMouse) {
						SaveMouseVals();									
					}
					return TRUE;
			}
			break;
	}
	return FALSE;
}
/************************************************************************************
Routine Description:
    Initialization for AccessMouseDlgProc.
   
Arguments:
    
Return Value:   
************************************************************************************/
void InitMouseVals()
{
	MouseKeyParam.cbSize = sizeof(MouseKeyParam);
	SystemParametersInfo(SPI_GETMOUSEKEYS, sizeof(MouseKeyParam), &MouseKeyParam, 0);
}
/************************************************************************************
Routine Description:
    Saves settings for the Mouse tab.
   
Arguments:
    
Return Value:
************************************************************************************/
void SaveMouseVals()
{
	//Otherwise MouseKeyParam settings will not be saved.
	if( 10 > MouseKeyParam.iMaxSpeed || 360 < MouseKeyParam.iMaxSpeed) {
		MouseKeyParam.iMaxSpeed = 10;
	}

	if( 1000 > MouseKeyParam.iTimeToMaxSpeed || 5000 < MouseKeyParam.iTimeToMaxSpeed) {
		MouseKeyParam.iTimeToMaxSpeed = 1000;
	}

	//The accessibility time-out affects the MouseKeys setting here.
	if(SoundOnActivation) {		// Set under General tab
        MouseKeyParam.dwFlags	|= MKF_HOTKEYSOUND;
	}
    else {
        MouseKeyParam.dwFlags	&= ~MKF_HOTKEYSOUND;
	}

	MouseKeyParam.cbSize = sizeof(MouseKeyParam);
	SystemParametersInfo(SPI_SETMOUSEKEYS, sizeof(MouseKeyParam), &MouseKeyParam, SPIF_SENDCHANGE);
}
/************************************************************************************
Routine Description:
    "Settings for MouseKeys" dialog procedure.
   
Arguments:
    Regular DlgProc
    
Return Value:
    BOOL value based on processed message   
************************************************************************************/
extern "C" BOOL CALLBACK MouseKeyDlg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int iPos;

	switch(msg)
	{
		case WM_INITDIALOG:			
			CheckDlgButton(hDlg, IDC_ACCD_MOUSESHORT, (MouseKeyParam.dwFlags & MKF_HOTKEYACTIVE));
			CheckDlgButton(hDlg, IDC_ACCD_MHOLDKEYS, (MouseKeyParam.dwFlags & MKF_MODIFIERS));

			if(MouseKeyParam.dwFlags & MKF_REPLACENUMBERS) {
				CheckRadioButton(hDlg, IDC_ACCD_NUMOFF, IDC_ACCD_NUMON, IDC_ACCD_NUMON);
			}
			else {
				CheckRadioButton(hDlg, IDC_ACCD_NUMOFF, IDC_ACCD_NUMON, IDC_ACCD_NUMOFF);
			}

			//iMaxSpeed: specifies the maximum speed the mouse cursor attains when an arrow key is held down.
			for (iPos = 0; iPos < MAXSPEED; iPos++)
			{
				if(MouseKeyParam.iMaxSpeed == SpeedTable[iPos]) {	
					break;
				}
			}

			if(MAXSPEED != iPos) { // Not in the SpeedTable
				iPos ++;
			}

			SendDlgItemMessage(hDlg, IDC_ACCD_MTOPSPEED, TBM_SETRANGE, TRUE, MAKELONG(1,MAXSPEED));
			SendDlgItemMessage(hDlg, IDC_ACCD_MTOPSPEED, TBM_SETPOS, TRUE, iPos);

			//iTimeToMaxSpeed specifies the length of time, in milliseconds, 
			//that it takes for the mouse cursor to reach maximum speed when an arrow key is held down. 
			//Valid values are from 1000 to 5000. 
			iPos = (MouseKeyParam.iTimeToMaxSpeed - 500)/500;
			iPos = (MAXSPEED + 1) - iPos;
			if( iPos > MAXSPEED) {
				iPos = MAXSPEED;
			}
			if( iPos <= 0) {
				iPos = 1;
			}
			
			SendDlgItemMessage(hDlg,IDC_ACCD_MACCEL, TBM_SETRANGE, TRUE, MAKELONG(1,MAXSPEED));
			SendDlgItemMessage(hDlg,IDC_ACCD_MACCEL, TBM_SETPOS, TRUE, iPos);

			//iCtrlSpeed specifies the multiplier to apply to the mouse cursor speed when the 
			//user holds down the CTRL key while using the arrow keys to move the cursor.
			MouseKeyParam.iCtrlSpeed = GetSystemMetrics(SM_CXSCREEN)/16;
			break;
			
		case WM_HSCROLL:			
			
			switch (LOWORD(wParam))
			{
				//TB_BOTTOM, TB_LINEDOWN, TB_LINEUP, and TB_TOP: the user interacts with a trackbar by using the keyboard
				case TB_LINEUP:		// VK_LEFT
				case TB_LINEDOWN:	// VK_RIGHT
				case TB_BOTTOM:		// VK_END
				case TB_TOP:		// VK_HOME

				//TB_THUMBPOSITION and TB_THUMBTRACK: the user is using the mouse. 
				case TB_THUMBTRACK:		// Slider movement (the user dragged the slider)
				case TB_THUMBPOSITION:	// WM_LBUTTONUP following a TB_THUMBTRACK notification message

				//TB_ENDTRACK, TB_PAGEDOWN, and TB_PAGEUP: keyboard, mouse
				case TB_PAGEUP:			// VK_PRIOR (the user clicked the channel to the left of the slider).	  	
				case TB_PAGEDOWN:		// VK_NEXT (the user clicked the channel to the right of the slider)

					iPos = SendMessage((HWND)(lParam), TBM_GETPOS,0,0);
					if(iPos < 1) iPos = 1;		
					else
						if(iPos > MAXSPEED) iPos = MAXSPEED;
					
					SendMessage((HWND)(lParam), TBM_SETPOS, TRUE, iPos);

					if ((HWND)(lParam) == GetDlgItem(hDlg, IDC_ACCD_MTOPSPEED)) {
						iPos--;		//0 Index based
						MouseKeyParam.iMaxSpeed = SpeedTable[iPos];
					}
					else {				
						MouseKeyParam.iTimeToMaxSpeed =  ((MAXSPEED + 1 - iPos) * 500) + 500;	
					}
			}
			break;

		case WM_COMMAND:
      	switch (GET_WM_COMMAND_ID(wParam, lParam)) 
			{
				case IDC_ACCD_MOUSESHORT:				
					MouseKeyParam.dwFlags ^= MKF_HOTKEYACTIVE;
					break;
				
				case IDC_ACCD_MHOLDKEYS:
					MouseKeyParam.dwFlags ^= MKF_MODIFIERS;
					break;

				case IDC_ACCD_NUMOFF:
					MouseKeyParam.dwFlags &= ~MKF_REPLACENUMBERS;
					CheckRadioButton(hDlg, IDC_ACCD_NUMOFF, IDC_ACCD_NUMON, IDC_ACCD_NUMOFF);
					break;

				case IDC_ACCD_NUMON:
					MouseKeyParam.dwFlags |= MKF_REPLACENUMBERS;
					CheckRadioButton(hDlg, IDC_ACCD_NUMOFF, IDC_ACCD_NUMON, IDC_ACCD_NUMON);
					break;

				case IDOK:
					EndDialog(hDlg, IDOK);
					break;

				case IDCANCEL:
					EndDialog(hDlg, IDCANCEL);
			}
			break;
					
		default:
			return FALSE;
	}

	return TRUE;
}
