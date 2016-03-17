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

Abstract: Accessibility Options\ Keyboard tab.
**/
#include "cplpch.h"
#include "accglobl.h"

extern BOOL SoundOnActivation;
ACCESSTIMEOUT	TimeOutParam;			
STICKYKEYS		StickyKeyParam;			
TOGGLEKEYS		ToggleKeyParam;
BOOL InitKeyboard = FALSE;

extern "C" BOOL CALLBACK StickyKeyDlg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern "C" BOOL CALLBACK ToggleKeyDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

/************************************************************************************
Routine Description:
    The dialog procedure for processing the Keyboard accessibility tab.
   
Arguments:
    Regular DlgProc
    
Return Value:
    BOOL value based on processed message 
************************************************************************************/
extern "C" BOOL CALLBACK AccessKeybdDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{   	
	STICKYKEYS TmpStickK;			
	TOGGLEKEYS TmpTogK;
		
    switch (message)     
    {
        case WM_INITDIALOG:
		{
			InitKeyboardVals();
			InitKeyboard = TRUE;
			
			CheckDlgButton(hDlg, IDC_ACC_USESTICKY, (StickyKeyParam.dwFlags & SKF_STICKYKEYSON));
						
			if (!(StickyKeyParam.dwFlags & SKF_AVAILABLE)) {
				EnableWindow(GetDlgItem(hDlg, IDC_ACC_STICKYSETTINGS),FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_ACC_USESTICKY),FALSE);				
			}
							
			CheckDlgButton(hDlg, IDC_ACC_USETOGGLE, (ToggleKeyParam.dwFlags & TKF_TOGGLEKEYSON));
			
			if (!(ToggleKeyParam.dwFlags & TKF_AVAILABLE)) {
				EnableWindow(GetDlgItem(hDlg, IDC_ACC_TOGGLESETTINGS),FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_ACC_USETOGGLE),FALSE);
			}

            AygInitDialog( hDlg, SHIDIF_SIPDOWN );
			
			return TRUE;			
		}

		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID(wParam, lParam)) 
			{				
				case IDC_ACC_USESTICKY:									
					StickyKeyParam.dwFlags ^= SKF_STICKYKEYSON;
					
					return TRUE;

				case IDC_ACC_USETOGGLE:					
			 		ToggleKeyParam.dwFlags ^= TKF_TOGGLEKEYSON;
					
					return TRUE;
				
				case IDC_ACC_STICKYSETTINGS:					
					TmpStickK = StickyKeyParam;
					if (IDCANCEL == DialogBox(g_hInst,MAKEINTRESOURCE(IDD_ACCD_STICKYSETTINGS), hDlg, StickyKeyDlg))
					{
						StickyKeyParam = TmpStickK;
					}
										
					return TRUE;

				case IDC_ACC_TOGGLESETTINGS:				
					TmpTogK = ToggleKeyParam;
					if(IDCANCEL == DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ACCD_TOGGLESETTINGS), hDlg, ToggleKeyDlg))
					{
						ToggleKeyParam = TmpTogK;
					}
									
					return TRUE;

				case IDOK:
					if(InitKeyboard) {
						SaveKeyboardVals();
					}

					return TRUE;
			}
			break;
	}
	return FALSE;
}
/************************************************************************************
Routine Description:
    Initialization for AccessKeybdDlgProc.
   
Arguments:
    
Return Value:   
************************************************************************************/
void InitKeyboardVals()
{
	HKEY hKey;
	DWORD dwDisposition, dwSize, dwType, dwVal;

	// Get Accessibility registry settings: Sound on/off.
	// Sound on/off feature MUST be initialized before we start to work with Keyboard and General tabs.
	if(ERROR_SUCCESS == 
		RegCreateKeyEx(HKEY_CURRENT_USER, RK_ACCESSIB, 0, NULL, 0, 0, NULL, &hKey, &dwDisposition))
	{		
		dwSize = sizeof(DWORD);		
		dwVal = 0;
		if(ERROR_SUCCESS ==
			RegQueryValueEx(hKey, RV_SOUNDONACTIV, NULL, (LPDWORD)&dwType, (LPBYTE)&dwVal, &dwSize))
		{
			SoundOnActivation  = dwVal;
		}
		else {
			SoundOnActivation  = TRUE;
		}

		RegCloseKey(hKey);
	}

	// TimeOutParam can be retrieved again if General tab is launched.
	TimeOutParam.cbSize = sizeof(TimeOutParam);
	SystemParametersInfo(SPI_GETACCESSTIMEOUT, sizeof(TimeOutParam), &TimeOutParam, 0);

	if(!TimeOutParam.iTimeOutMSec) {										
		//Set minimum otherwise Keyboard tab settings will not be saved.
		TimeOutParam.iTimeOutMSec = (ULONG)(5L * 60L * 1000L);

		SystemParametersInfo(SPI_SETACCESSTIMEOUT, sizeof(TimeOutParam), &TimeOutParam, SPIF_SENDCHANGE);
	}
	
	
	ToggleKeyParam.cbSize = sizeof(ToggleKeyParam);
	SystemParametersInfo(SPI_GETTOGGLEKEYS, sizeof(ToggleKeyParam), &ToggleKeyParam, 0);
	
	StickyKeyParam.cbSize = sizeof(StickyKeyParam);
	SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(StickyKeyParam), &StickyKeyParam, 0);
	
}
/************************************************************************************
Routine Description:
    Saves settings for the Keyboard tab.
   
Arguments:
    
Return Value:
************************************************************************************/
void SaveKeyboardVals()
{
    StickyKeyParam.cbSize = sizeof(StickyKeyParam);
    ToggleKeyParam.cbSize = sizeof(ToggleKeyParam);

	//The accessibility time-out affects the ToggleKeys and StickyKeys setting here.
    if(SoundOnActivation)		// Set under General tab
    {
		//SKF_HOTKEYSOUND (TKF_HOTKEYSOUND): system plays a siren sound 
		//when the user turns the StickyKeys (ToggleKeys) feature on or off by using the hot key.
        StickyKeyParam.dwFlags	|= SKF_HOTKEYSOUND;        
        ToggleKeyParam.dwFlags	|= TKF_HOTKEYSOUND;

		//ATF_ONOFFFEEDBACK: the OS plays a descending siren sound 
		//when the time-out period elapses and the accessibility features are turned off. 
        TimeOutParam.dwFlags	|= ATF_ONOFFFEEDBACK;
    }
    else
    {
        StickyKeyParam.dwFlags	&= ~SKF_HOTKEYSOUND;        
        ToggleKeyParam.dwFlags	&= ~TKF_HOTKEYSOUND;
        TimeOutParam.dwFlags	&= ~ATF_ONOFFFEEDBACK;
    }
      
	SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(ToggleKeyParam), &ToggleKeyParam, SPIF_SENDCHANGE);
	SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(StickyKeyParam), &StickyKeyParam, SPIF_SENDCHANGE);
	
}
/************************************************************************************
Routine Description:
    "Settings for StickyKeys" dialog procedure.
   
Arguments:
    Regular DlgProc
    
Return Value:
    BOOL value based on processed message   
************************************************************************************/
extern "C" BOOL CALLBACK StickyKeyDlg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
			//SKF_HOTKEYACTIVE: if this flag is set, the user can turn 
			//the StickyKeys feature on and off by pressing the SHIFT key five times.
			CheckDlgButton(hDlg, IDC_ACCD_STICKYSHORT, (StickyKeyParam.dwFlags & SKF_HOTKEYACTIVE));

			//SKF_TRISTATE: if the flag is set then pressing a modifier key twice in a row 
			//locks down the key until the user presses it a third time.			
			CheckDlgButton(hDlg, IDC_ACCD_STKLOCK, (StickyKeyParam.dwFlags & SKF_TRISTATE));

			//SKF_AUDIBLEFEEDBACK: if the flag is set then the system plays a sound when 
			//the user latches, locks, or releases modifier keys using the StickyKeys feature.		
			CheckDlgButton(hDlg, IDC_ACCD_STKSOUNDMOD, (StickyKeyParam.dwFlags & SKF_AUDIBLEFEEDBACK));

			break;
      
		case WM_COMMAND:
      	switch (GET_WM_COMMAND_ID(wParam, lParam)) 
			{								
				case IDC_ACCD_STICKYSHORT:
					StickyKeyParam.dwFlags ^= SKF_HOTKEYACTIVE;					
					break;

				case IDC_ACCD_STKLOCK:
					StickyKeyParam.dwFlags ^= SKF_TRISTATE;                
					break;
				
				case IDC_ACCD_STKSOUNDMOD:
					StickyKeyParam.dwFlags ^= SKF_AUDIBLEFEEDBACK;
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

/************************************************************************************
Routine Description:
    "Settings for ToggleKeys" dialog procedure.
   
Arguments:
    Regular DlgProc
    
Return Value:
    BOOL value based on processed message   
************************************************************************************/
extern "C" BOOL CALLBACK ToggleKeyDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{		
		case WM_INITDIALOG:	
			CheckDlgButton(hDlg, IDC_ACCD_TOGGLESHORT, (ToggleKeyParam.dwFlags & TKF_HOTKEYACTIVE));	
			break;

    	case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) 
		{											
				case IDC_ACCD_TOGGLESHORT:				
					ToggleKeyParam.dwFlags ^= TKF_HOTKEYACTIVE;					
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
	return(TRUE);
}