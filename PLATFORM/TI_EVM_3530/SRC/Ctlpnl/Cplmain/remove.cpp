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

Abstract: Code specific to the remove-programs CPL
**/

#include "cplpch.h"
#include "unload.h"


void LoadUpdateRemoveList(HWND hDlg) 
{
	TCHAR szKey[MAX_PATH];
	CReg reg1(HKEY_LOCAL_MACHINE, RK_SOFTWAREAPPS);
	
   	HWND hwndLB = DI(IDC_LISTBOX);
   	
	ListBox_ResetContent(hwndLB);

    while(reg1.EnumKey(szKey, CCHSIZEOF(szKey))) {
        
        CReg reg2(reg1, szKey);

        if(reg2.ValueDW(RV_INSTALLED))
			ListBox_AddString(hwndLB, szKey);
   	}
   	
	EnableWindow(DI(IDC_REMOVE), FALSE);
	
}

void DisplayRemoveError(HWND hDlg, LPARAM lParam)
{
	int idsErr;
	TCHAR szTemp[MAX_PATH * 2];
    ZEROMEM(szTemp);
	
	switch (lParam)  {
	case 0:
		return;

	case ERR_REGISTRY_APP_NAME:			// can't find AppName in regsitry
		idsErr = IDS_REMOVEERR_REG; 
		break;
		
	case ERR_UNLOAD_OPEN:				// can't open unload script file
	case ERR_UNLOAD_COMMAND: 			// illegal unload script file comand		
	case ERR_UNLOAD_NO_COMMAND:         // no command found in unload script file 
	case ERR_REG_COMMAND:				// Illegal registry command
		idsErr = IDS_REMOVEERR_SCRIPT; 
		break;
		
	case ERR_FILE_IN_USE:
		DEBUGCHK(FALSE);	// this error shouldnt propagate this far
		// fall through
	case ERR_ILLEGAL_APP_NAME:		// unused
	case ERR_REGISTRY_INSTALLED_DIR:// unused
	case ERR_UNLOAD_TEMP_OPEN:		// unused
	case ERR_WRITE_TEMP_UNLOAD:		// unused
	case ERR_FILE_NOT_FOUND:		// unused
	case ERR_OUT_OF_MEMORY:			// unused
	default:
		idsErr = IDS_REMOVEERRDEFAULT;
		return;
	}			 
	lstrcpy(szTemp, CELOADSZ(idsErr));
	lstrcat(szTemp, CELOADSZ(IDS_REMOVEERRCOMMON));
	MessageBox(hDlg,  szTemp, CELOADSZ(IDS_REMOVETITLEERR), MB_ICONEXCLAMATION|MB_OK);
}

HWND	g_hwndRemoveDlg;
TCHAR	g_szRemoveApp[MAX_PATH];

DWORD UnloadThread(PVOID pv)
{
	return UninstallApplication(g_hInst, g_hwndRemoveDlg, g_szRemoveApp);
}


extern "C" BOOL CALLBACK RemoveDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	int iSel;
	switch (message) {
    case WM_INITDIALOG:		   
		LoadUpdateRemoveList(hDlg);
		return FALSE; // we set our own focus

    case WM_COMMAND:      
		switch (LOWORD(wParam)) {
		case IDC_REMOVE_FLAG:
			DEBUGMSG(ZONE_REMOVE, (L"Remove: Got IDC_REMOVE_FLAG lParam=%d\r\n", lParam));
			DisplayRemoveError(hDlg, lParam);
			LoadUpdateRemoveList(hDlg);
			return TRUE;
 		
		case IDC_LISTBOX: 
			if (HIWORD(wParam) == LBN_SELCHANGE)
				EnableWindow(DI(IDC_REMOVE), TRUE);        
			return TRUE;
					
		case IDC_REMOVE:
			if((iSel = ListBox_GetCurSel(DI(IDC_LISTBOX))) < 0)
				break;
			ListBox_GetText(DI(IDC_LISTBOX), iSel, g_szRemoveApp);

			if(IDYES == MessageBox(hDlg, CELOADSZ(IDS_REMOVECONFIRM), CELOADSZ(IDS_REMOVE_TITLE), 
					MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2))
			{
				g_hwndRemoveDlg = hDlg;
				MyCreateThread(UnloadThread, NULL);
			}
			LoadUpdateRemoveList(hDlg);
			return TRUE;
			
		case IDOK:
			// Nothing to do
			return TRUE;
		}
	}
	return FALSE;
}

