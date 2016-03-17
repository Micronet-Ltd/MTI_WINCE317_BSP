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

Abstract: Accessibility Options\ General tab. 
**/
#include "cplpch.h"
#include "accglobl.h"

extern ACCESSTIMEOUT TimeOutParam;	
extern BOOL InitKeyboard, InitSound, InitDisplay, InitMouse;

BOOL SoundOnActivation	= TRUE;		// Is in use under Keyboard, Display and Mouse tabs.
BOOL InitGeneral = FALSE;	

bool InitGeneralVals(void);
void SaveGeneralVals(void);

#define MAXCOMBO	6

/************************************************************************************
Routine Description:
    The dialog procedure for processing the General accessibility tab.
	
	ACCESSTIMEOUT contains information about the time-out period associated 
	with the Win32 accessibility features. The accessibility time-out period is the length of time 
	that must pass without keyboard and mouse input before the operating system automatically 
	turns off accessibility features. 

	The accessibility features affected by the time-out are the ToggleKeys, StickyKeys, MouseKeys and 
	the high contrast mode setting.
   
Arguments:
    Regular DlgProc
    
Return Value:
    BOOL value based on processed message 
************************************************************************************/
extern "C" BOOL CALLBACK AccessGeneralDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i;
	DWORD dwIndex;
	bool 	bRet = false;
			
	switch(message)
	{	
		case WM_INITDIALOG:

			bRet = InitGeneralVals();

			InitGeneral = TRUE;			
			
			// If the other pages were not launched make sure that:
			// - sound global stuff is correct;
			// - all registry settings are correct.			
			if(!InitKeyboard) {
				InitKeyboardVals();
			}
			if(!InitSound) {
				InitSoundVals();
			}
			if(!InitDisplay) {
				InitDisplayVals();
			}
			if(!InitMouse) {
				InitMouseVals();
			}
			
			CheckDlgButton(hDlg, IDC_ACC_IDLECHK, (TimeOutParam.dwFlags & ATF_TIMEOUTON));
			CheckDlgButton(hDlg, IDC_ACC_SOUNDONOFFCHK, SoundOnActivation);
			
			for (i = 0; i < MAXCOMBO ; i++)			
			{
				TCHAR szTemp[MAX_PATH];
				StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%d"), (( i + 1 ) * 5));
				ComboBox_AddString(GetDlgItem(hDlg, IDC_ACC_GENCOMBO), szTemp);
			}

			//Get zero-based index. 
			dwIndex = TimeOutParam.iTimeOutMSec / (1000L * 60L * 5L);
			dwIndex--;
			if(dwIndex < 0 || dwIndex > 5) {
					dwIndex = 0;
			}
			ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_ACC_GENCOMBO), dwIndex); 

			//If ATF_TIMEOUTON flag is set, a time-out period has been set for accessibility features.
			//If the flag is not set, the features will not time out even though a time-out period is specified.
			if (!(TimeOutParam.dwFlags & ATF_TIMEOUTON)) {
				EnableWindow(GetDlgItem(hDlg, IDC_ACC_GENCOMBO),FALSE);
			}
			
			//TimeOutParam.iTimeOutMSec should not be 0, otherwise settings are unchecked
			//because of the event under AccessibilityKeyProcessor
			if(!TimeOutParam.iTimeOutMSec) {										
				i = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_ACC_GENCOMBO));
				TimeOutParam.iTimeOutMSec = (ULONG)((long)(( i + 1 ) * 5) * 60L * 1000L);
			}
					
			if (!bRet)
			{				
				EnableWindow(GetDlgItem(hDlg, IDC_ACC_IDLECHK),FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_ACC_GENCOMBO),FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_ACC_SOUNDONOFFCHK),FALSE);
			}

            AygInitDialog( hDlg, SHIDIF_SIPDOWN );
			return TRUE;

    	case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) 
			{
				case IDC_ACC_SOUNDONOFFCHK:
					SoundOnActivation = !SoundOnActivation;
					
					return TRUE;

				case IDC_ACC_IDLECHK:
					TimeOutParam.dwFlags ^= ATF_TIMEOUTON;
					EnableWindow(GetDlgItem(hDlg, IDC_ACC_GENCOMBO), (TimeOutParam.dwFlags & ATF_TIMEOUTON));
	    	      					
					return TRUE;
										
				case IDC_ACC_GENCOMBO:
					switch(HIWORD(wParam))
					{
						case CBN_CLOSEUP:							
							i = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_ACC_GENCOMBO));
							TimeOutParam.iTimeOutMSec = (ULONG)((long)((i + 1) * 5) * 60L * 1000L);
							
							return TRUE;
					}
					break;

				case IDOK:
					if(InitGeneral) {							
						SaveGeneralVals();
					}
					return TRUE;
			}
			break;
	}
	return FALSE;
}
/************************************************************************************
Routine Description:
    Initialization for AccessGeneralDlgProc.
   
Arguments:
    
Return Value:
    indicates if the ACCESSTIMEOUT feature is present 
************************************************************************************/
bool 
InitGeneralVals(
	void
	)
{
	TimeOutParam.cbSize = sizeof(TimeOutParam);                                                  
	return(SystemParametersInfo(SPI_GETACCESSTIMEOUT, sizeof(TimeOutParam), &TimeOutParam, 0) ? true : false);
}
/************************************************************************************
Routine Description:
    Saves settings for the General tab.
   
Arguments:
    
Return Value:
************************************************************************************/
void SaveGeneralVals()
{
	HKEY hKey;
	DWORD dwDisposition, dwVal;

	if(SoundOnActivation) {
		//ATF_ONOFFFEEDBACK: If this flag is set, the operating system plays a descending siren sound 
		//when the time-out period elapses and the accessibility features are turned off. 
		TimeOutParam.dwFlags	|= ATF_ONOFFFEEDBACK;						
	}
	else {
		TimeOutParam.dwFlags	&= ~ATF_ONOFFFEEDBACK;						
	}
									
	TimeOutParam.cbSize = sizeof(TimeOutParam);																		
	SystemParametersInfo(SPI_SETACCESSTIMEOUT, sizeof(TimeOutParam), &TimeOutParam, SPIF_SENDCHANGE);
		
	// Set Accessibility registry: Sound on/off 
	if(ERROR_SUCCESS == 
		RegCreateKeyEx(HKEY_CURRENT_USER, RK_ACCESSIB, 0, NULL, 0, 0, NULL, &hKey, &dwDisposition))
	{			
	 	dwVal = SoundOnActivation;	 		
		RegSetValueEx(hKey, RV_SOUNDONACTIV, 0, REG_DWORD, (LPBYTE)&dwVal, sizeof(DWORD));

		RegCloseKey(hKey);
	}
	
	// If the other pages have not been launched (but we got their settings under WM_INITDIALOG above) then
	// save the settings.
    if(!InitKeyboard) {
		SaveKeyboardVals();
	}
	if(!InitSound) {
		SaveSoundVals();
	}
	if(!InitDisplay) {
		SaveDisplayVals();
	}
	if(!InitMouse) {
		SaveMouseVals();
	}
	
}
