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

Abstract: Code specific to the PASSWD CPL
**/

#include "cplpch.h"

BOOL fScreenSaverIsCheckedWhenEnabled;

void EnablePwdCheckbox(HWND hDlg)
{
	TCHAR sz1[PASSWORD_LENGTH+2], sz2[PASSWORD_LENGTH+1];
	
	GetDlgItemText(hDlg, IDC_EDIT_NEW_PASS, sz1, PASSWORD_LENGTH+1);
	GetDlgItemText(hDlg, IDC_EDIT_CONF_PASS, sz2, PASSWORD_LENGTH+1);
	
    if (*sz1 && *sz2 && 0==_tcsicmp(sz1, sz2)) 
		{
		EnableWindow(DI(IDC_CHK_PASSWRD), TRUE);
		if (BST_CHECKED == SendMessage(DI(IDC_CHK_PASSWRD), BM_GETCHECK, 0, 0))
			{
			Button_SetCheck(DI(IDC_CHK_PASSWRDSCREENSAVER), fScreenSaverIsCheckedWhenEnabled);
			EnableWindow(DI(IDC_CHK_PASSWRDSCREENSAVER), TRUE);
			}
		} 
	else 
		{
		EnableWindow(DI(IDC_CHK_PASSWRD), FALSE);
		Button_SetCheck(DI(IDC_CHK_PASSWRDSCREENSAVER), FALSE);
		EnableWindow(DI(IDC_CHK_PASSWRDSCREENSAVER), FALSE);

    	// if both boxes are emty no power up passwrd and no screen saver passwrd
		if(!sz1[0] && !sz2[0])      
			{
			Button_SetCheck(DI(IDC_CHK_PASSWRD), FALSE);
			}
		}
}

extern "C" BOOL APIENTRY PasswdDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static BOOL fEditsAreUntouched;
    static LPTSTR pszOldPass;
    DWORD dwStatus;
    
    switch (message)
    {
   	case WM_INITDIALOG:
        // in our case, lParam is the password
        ASSERT(lParam);
        pszOldPass = (LPTSTR)lParam;
        Edit_LimitText(DI(IDC_EDIT_NEW_PASS), PASSWORD_LENGTH);
        Edit_LimitText(DI(IDC_EDIT_CONF_PASS), PASSWORD_LENGTH);
        SetDlgItemText(hDlg, IDC_EDIT_NEW_PASS, pszOldPass);
        SetDlgItemText(hDlg, IDC_EDIT_CONF_PASS,pszOldPass);
        SetFocus(DI(IDC_EDIT_NEW_PASS));
        Edit_SetSel(DI(IDC_EDIT_NEW_PASS), 0, -1);
        
        dwStatus = GetPasswordStatus();
        Button_SetCheck(DI(IDC_CHK_PASSWRD), dwStatus & PASSWORD_STATUS_ACTIVE);
        EnableWindow(DI(IDC_CHK_PASSWRDSCREENSAVER), dwStatus & PASSWORD_STATUS_ACTIVE);
        fScreenSaverIsCheckedWhenEnabled = dwStatus & PASSWORD_STATUS_SCREENSAVERPROTECT;
        Button_SetCheck(DI(IDC_CHK_PASSWRDSCREENSAVER), fScreenSaverIsCheckedWhenEnabled);
        EnablePwdCheckbox(hDlg);
        
        AygInitDialog( hDlg, SHIDIF_SIPDOWN );
        
        fEditsAreUntouched = TRUE;
        return 0; // dont let gwe change focus
        
        
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_EDIT_NEW_PASS:
        case IDC_EDIT_CONF_PASS:
            // The first time they hit eitehr edit control, clear the other 
            if(HIWORD(wParam)==EN_CHANGE)
            {
                if(fEditsAreUntouched)
                {
                    fEditsAreUntouched = FALSE;
                    SetDlgItemText(hDlg, (LOWORD(wParam)==IDC_EDIT_NEW_PASS ? IDC_EDIT_CONF_PASS : IDC_EDIT_NEW_PASS), EMPTY_STRING);
                }
                EnablePwdCheckbox(hDlg);
            }
            return TRUE;
            
        case IDC_CHK_PASSWRD:
            if (BN_CLICKED == HIWORD(wParam))
            {
                if (BST_CHECKED == SendMessage(DI(IDC_CHK_PASSWRD), BM_GETCHECK, 0, 0))
                {
                    Button_SetCheck(DI(IDC_CHK_PASSWRDSCREENSAVER), fScreenSaverIsCheckedWhenEnabled);
                    EnableWindow(DI(IDC_CHK_PASSWRDSCREENSAVER), TRUE);
                }
                else
                {
                    Button_SetCheck(DI(IDC_CHK_PASSWRDSCREENSAVER), FALSE);
                    EnableWindow(DI(IDC_CHK_PASSWRDSCREENSAVER), FALSE);
                }
            }
            break;
            
        case IDC_CHK_PASSWRDSCREENSAVER:
            if (BN_CLICKED == HIWORD(wParam))
            {
                if (BST_CHECKED == SendMessage(DI(IDC_CHK_PASSWRDSCREENSAVER), BM_GETCHECK, 0, 0))
                    fScreenSaverIsCheckedWhenEnabled = TRUE;
                else
                    fScreenSaverIsCheckedWhenEnabled = FALSE;
            }
            break;
            
        case IDOK:
            {
                TCHAR sz1[PASSWORD_LENGTH+2] = L"", sz2[PASSWORD_LENGTH+1] = L"";
                TCHAR *pszOld = NULL, *pszNew = NULL;
                BOOL fSet = FALSE;
                size_t iNewPassLen = 0;
                
                GetDlgItemText(hDlg, IDC_EDIT_NEW_PASS, sz1, PASSWORD_LENGTH+1);
                GetDlgItemText(hDlg, IDC_EDIT_CONF_PASS, sz2, PASSWORD_LENGTH+1);
                
                if(wcsicmp(sz1, sz2))
                {
                    MessageBox(hDlg, CELOADSZ(IDS_PASSWDERR_NOTMATCH), CELOADSZ(IDS_PASSWDERR_TITLE), MB_ICONINFORMATION | MB_OK );
                    SetFocus(GetDlgItem(hDlg, IDC_EDIT_CONF_PASS));
                    Edit_SetSel(DI(IDC_EDIT_CONF_PASS), 0L, -1);   
                    return FALSE;
                }
                
                //
                // NOTE: The Control Panels & the StartUI & Welcome dialogs all force passwds to
                // lowercase so as to be case-insensitive, despite the fact that the underlying
                // OS password APIs are case sensitive.
                //
                _wcslwr(sz1);
                
                if (!CheckPassword(NULL))   // If NULL doesn't pass we have an old password set
                {
                    pszOld = pszOldPass;
                }
                
                StringCchLength(sz1, PASSWORD_LENGTH+2, &iNewPassLen);

                if (iNewPassLen)
                {
                    pszNew = sz1;
                }
                
                fSet = SetPassword(pszOld, pszNew);
                ASSERT(fSet);
                
                if (fSet)
                {
                    dwStatus = (fSet && Button_GetCheck(DI(IDC_CHK_PASSWRD))) ? PASSWORD_STATUS_ACTIVE : 0;
                    if (dwStatus)
                        dwStatus |= (fSet && Button_GetCheck(DI(IDC_CHK_PASSWRDSCREENSAVER))) ? PASSWORD_STATUS_SCREENSAVERPROTECT : 0;
                    
                    fSet = SetPasswordStatus(dwStatus, pszNew);
                    ASSERT(fSet);
                    
                    if (fSet)
                    {
                        WCHAR szUserName[128];
                        DWORD cchUser = ARRAYSIZEOF(szUserName);
                        if (GetUserNameExW(NameWindowsCeLocal, szUserName, &cchUser ))
                        {
                            // set the user secret used for CryptProtectData APIs only if power on password is checked
                            // Since we don't store the password on the device, the user needs to provide it after a warm reset
                            // so as that SetCurrentUser can succeed.
                            if (dwStatus) 
                                SetUserData((PBYTE)pszNew, iNewPassLen*sizeof(TCHAR));
                            else
                                SetUserData(NULL, 0);   // no password used for CryptProtectData
                        }
                    }
                }
                
                return TRUE;
            }
        }
    }
    return FALSE;                                           
}


