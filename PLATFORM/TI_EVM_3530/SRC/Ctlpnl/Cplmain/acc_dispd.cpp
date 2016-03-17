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

Abstract: Accessibility Options\ Display tab.
**/

#include "cplpch.h"
#include "accglobl.h"

extern	BOOL SoundOnActivation;
HIGHCONTRAST HighContrastParam;
TCHAR		 szCurHCScheme[MAX_PATH] = {0};
BOOL		 InitDisplay = FALSE;
		
extern "C" BOOL CALLBACK HighContrastDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
const TCHAR szHighContrastWhite[] = TEXT("High Contrast White");   
const TCHAR	szHighContrastBlack[] = TEXT("High Contrast Black");
/************************************************************************************
Routine Description:
    The dialog procedure for processing the Display accessibility tab.
   
Arguments:
    Regular DlgProc
    
Return Value:
    BOOL value based on processed message 
************************************************************************************/
extern "C" BOOL CALLBACK AccessDisplayDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{       
	HIGHCONTRAST TmpHC = {0};

    switch (message)     
    {
        case WM_INITDIALOG:
		{			
			InitDisplayVals();
			InitDisplay = TRUE;
			
			CheckDlgButton(hDlg, IDC_ACC_USEHIGHCONT, (HighContrastParam.dwFlags & HCF_HIGHCONTRASTON));

			if (!(HighContrastParam.dwFlags & HCF_AVAILABLE)) {
				EnableWindow(GetDlgItem(hDlg, IDC_ACC_DISPSETTINGS), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_ACC_USEHIGHCONT), FALSE);				
			}
			
            AygInitDialog( hDlg, SHIDIF_SIPDOWN );

            return TRUE;
		}

    	case WM_COMMAND:
			switch (GET_WM_COMMAND_ID(wParam, lParam)) 
			{				
				case IDC_ACC_USEHIGHCONT:
					HighContrastParam.dwFlags ^= HCF_HIGHCONTRASTON;																
					return TRUE;

				case IDC_ACC_DISPSETTINGS:					
					TmpHC = HighContrastParam;
					if(IDCANCEL == DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ACCD_DISPSETTINGS), hDlg, HighContrastDlg))
					{
						HighContrastParam = TmpHC;
					}					
					return TRUE;

				case IDOK:
					if(InitDisplay) {
						SaveDisplayVals();
					}
					return TRUE; 

				default:
					break;
			}
			break;
	}
	return FALSE;
}
/************************************************************************************
Routine Description:
    Initialization for AccessDisplayDlgProc.
   
Arguments:
    
Return Value:   
************************************************************************************/
void InitDisplayVals()
{	
	HKEY hKey;
	DWORD dwDisposition;
	DWORD dwType, dwSize;
	TCHAR szBuf[MAX_PATH] = {0};
	
	HighContrastParam.cbSize			= sizeof(HighContrastParam);
	HighContrastParam.lpszDefaultScheme = szCurHCScheme;
 	SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(HighContrastParam), &HighContrastParam, 0);	

	if(!szCurHCScheme[0]) 
	{		
		if(	ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, RK_ACCESSIBHC, 0, NULL, 0, 0, NULL, &hKey, &dwDisposition))
		{
			dwSize = MAX_PATH;
			if(ERROR_SUCCESS == RegQueryValueEx(hKey, RV_HCSCHEME, NULL, (LPDWORD)&dwType, (LPBYTE)szBuf, &dwSize))
			{				
				lstrcpy(szCurHCScheme, szBuf);
			}
			else
			{
				// Set white on black color scheme.				
				lstrcpy(szCurHCScheme, szHighContrastBlack);
			}					
			RegCloseKey(hKey);
		}
		else {			
			// Set white on black color scheme.
			lstrcpy(szCurHCScheme, szHighContrastBlack);
		}
	}
}
/************************************************************************************
Routine Description:
    Saves settings for the Display tab.
   
Arguments:
    
Return Value:
************************************************************************************/
void SaveDisplayVals()
{
	HKEY hKey;
	
	//The accessibility time-out affects the high contrast mode setting here.
	if(SoundOnActivation) {		// Set under General tab
        HighContrastParam.dwFlags	|= HCF_HOTKEYSOUND;
	}
    else {
        HighContrastParam.dwFlags	&= ~HCF_HOTKEYSOUND;
	}
	
	// Save the "High Contrast Scheme" under registry.
	if(TEXT('\0') != szCurHCScheme[0] &&
		ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, RK_ACCESSIBHC, 0, 0, &hKey))
	{
		RegSetValueEx(hKey, RV_HCSCHEME, 0, (DWORD)REG_SZ, (BYTE *)szCurHCScheme, 
							sizeof(szCurHCScheme)/sizeof(szCurHCScheme[0]));                        
		RegCloseKey(hKey);
	}
	else {		
		// Set white on black color scheme to have something to set.
		lstrcpy(szCurHCScheme, szHighContrastBlack);
	}
	
	HighContrastParam.cbSize = sizeof(HighContrastParam);	
	SystemParametersInfo(SPI_SETHIGHCONTRAST, sizeof(HighContrastParam), &HighContrastParam, SPIF_SENDCHANGE);
	
}
/************************************************************************************
Routine Description:
    "Settings for High Contrast" dialog procedure.
   
Arguments:
    Regular DlgProc
    
Return Value:
    BOOL value based on processed message   
************************************************************************************/
extern "C" BOOL CALLBACK HighContrastDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{	
	TCHAR szLastHCScheme[MAX_PATH] = {0};

	switch(message)
	{
		case WM_INITDIALOG:			
			CheckDlgButton(hDlg, IDC_ACCD_HIGHSHORT, (HighContrastParam.dwFlags & HCF_HOTKEYACTIVE));		
			
			// Set white on black color scheme.
			CheckRadioButton(hDlg, IDC_ACCD_HCWHITEBLACK, IDC_ACCD_HCBLACKWHITE, IDC_ACCD_HCWHITEBLACK);

			if(0 != lstrcmp(szCurHCScheme, szHighContrastBlack))
			{		
				// Set black on white color scheme.				
				CheckRadioButton(hDlg, IDC_ACCD_HCWHITEBLACK, IDC_ACCD_HCBLACKWHITE, IDC_ACCD_HCBLACKWHITE);				
			}

			lstrcpy(szLastHCScheme, szCurHCScheme);	//Keep the scheme to restore it if necessary.
			break;
	
		case WM_COMMAND:
      	switch (GET_WM_COMMAND_ID(wParam, lParam)) 
			{
				case IDC_ACCD_HIGHSHORT:					
					HighContrastParam.dwFlags ^=  HCF_HOTKEYACTIVE;
					break;

				case IDC_ACCD_HCWHITEBLACK:					
					CheckRadioButton(hDlg, IDC_ACCD_HCWHITEBLACK, IDC_ACCD_HCBLACKWHITE, IDC_ACCD_HCWHITEBLACK);					
					lstrcpy(szCurHCScheme, szHighContrastBlack);
					break;

				case IDC_ACCD_HCBLACKWHITE:										
					CheckRadioButton(hDlg, IDC_ACCD_HCWHITEBLACK, IDC_ACCD_HCBLACKWHITE, IDC_ACCD_HCBLACKWHITE);					
					lstrcpy(szCurHCScheme, szHighContrastWhite);
					break;
								 				
				case IDOK:					
					EndDialog(hDlg, IDOK);
					break;

				case IDCANCEL:					
					lstrcpy(szCurHCScheme, szLastHCScheme);
					EndDialog(hDlg, IDCANCEL);
			}
			break;
					
		default:
			return FALSE;
	}
	return TRUE;
}

