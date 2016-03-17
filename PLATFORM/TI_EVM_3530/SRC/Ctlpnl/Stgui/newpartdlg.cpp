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
#include "StgUI.H"

// --------------------------------------------------------
DWORD GetSizeField(HWND hDlg)
// --------------------------------------------------------
{
    TCHAR szSize[MAX_PATH] = _T("");    

    // retrieve the size field and convert it to a DWORD
    GetDlgItemText(hDlg, IDC_EDIT_SIZE, szSize, MAX_PATH);
    return _ttoi(szSize);
}

// --------------------------------------------------------
BOOL GetNameField(HWND hDlg, LPTSTR szName, DWORD dwLen)
// --------------------------------------------------------
{
    return GetDlgItemText(hDlg, IDC_EDIT_NAME, szName, dwLen);
}

// --------------------------------------------------------
BOOL CALLBACK NewPartDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
// --------------------------------------------------------
{
    static PNEWPART_INFO pInfo = NULL;
	TCHAR	*pszHelpPath[] = {TEXT("file:ctpnl.htm#create_partition\0")};
    
    switch(wMsg)
    {
        case WM_INITDIALOG:
            
            CenterWindow(hDlg);

            // lparam is new part info struct
            pInfo = (PNEWPART_INFO)lParam;
            
            if(!pInfo)
                EndDialog(hDlg, FALSE);

            // check the "use all disk space box" on startup 
            DLG_DISABLEITEM(hDlg, IDC_EDIT_SIZE);
            DLG_CHECKITEM(hDlg, IDC_CHECKBOX_SPACE);
            
            return TRUE;

        case WM_HELP:
			CreateProcess(TEXT("peghelp"), pszHelpPath[0], 0,0,0,0,0,0,0,0);             
            return TRUE;

        case WM_COMMAND:
            
            switch(LOWORD(wParam))
            {
                case IDC_CHECKBOX_SPACE:
                    
                    if(DLG_ITEMISCHECKED(hDlg, IDC_CHECKBOX_SPACE))
                        DLG_DISABLEITEM(hDlg, IDC_EDIT_SIZE);
                    else
                        DLG_ENABLEITEM(hDlg, IDC_EDIT_SIZE);
                        
                    return TRUE;
                    
                case IDOK:

                    // fill in the size field only if box is not checked
                    if (pInfo)
                    {
                        if(!DLG_ITEMISCHECKED(hDlg, IDC_CHECKBOX_SPACE))
                            pInfo->dwSize = GetSizeField(hDlg);
                        
                        GetNameField(hDlg, pInfo->szName, MAX_PATH);
                    }
                    EndDialog(hDlg, TRUE);
                    
                    return TRUE;

                default:
                    return FALSE;
            }

        case WM_CLOSE:
            EndDialog(hDlg, FALSE);
            return FALSE;

    }
    return FALSE;
}
