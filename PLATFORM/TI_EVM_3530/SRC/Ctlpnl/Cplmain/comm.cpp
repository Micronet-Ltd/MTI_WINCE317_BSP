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

Abstract: Code for COMM & Dialing CPLs
**/

#include "cplpch.h"
#include "ras.h"

//
// DIALING CPL has only these 2 functions
//

void WINAPI lineCallback(DWORD hDevice, DWORD dwMessage, DWORD dwInstance, 
                    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    DEBUGMSG (ZONE_COMM, (L"COMM.CPL - Line Callback\r\n"));
    //return (0);
}

extern "C" int WINAPI ShowLineTranslateDlg(void)
{
#define tapiVersionCur          0x10005     
    DWORD       NumDev;
    LONG        lResult;
    DWORD         dwAPIVersion = tapiVersionCur;
    HLINEAPP    hLineApp = NULL;

    DEBUGMSG (ZONE_COMM, (L"About to call lineInitialize\r\n"));
    lResult = lineInitialize (&hLineApp, g_hInst, lineCallback, NULL, &NumDev);

    DEBUGMSG (ZONE_COMM, (L"lineInitialize ret=%d NumDev=%d\r\n", lResult, NumDev));

    // Let the user modify the dialing tab
    lResult = lineTranslateDialog(hLineApp, 0, dwAPIVersion, 0, NULL);

    // and finally, shut down the line
    lResult = lineShutdown(hLineApp);
    if ( lResult != 0 ) {
        DEBUGMSG (ZONE_ERROR|ZONE_COMM, (L"lineShutdown failed (x%X)\r\n", lResult ));
        dwAPIVersion = 0;
    }
    return (0);
}


//
// COMM CPL RAS window proc & associated functions
//
#define MAX_CONNECT        128

void RasSetConnectText(HWND hDlg, LPCTSTR pszConnect);
void RasGetConnectText(HWND hDlg, LPTSTR szConnect);
void EnableRASWindows(HWND hDlg);
extern "C" BOOL APIENTRY RasChangeDlgProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
void LoadConnectoidList(HWND hwndCB);

extern "C" BOOL APIENTRY CommRASDlgProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    CReg reg;
    TCHAR szBuf[MAX_CONNECT+1];
    DWORD dwAuto;
    LPTSTR pszConnect;
    
    switch (message)
    {
    case WM_INITDIALOG:
        DEBUGMSG(ZONE_COMM, (L"CommRASDlgProc: Got WM_INITDLG....\r\n"));
        SetDeviceDependentText(hDlg, IDC_CONNECTION, IDS_COMM_CONNECTION);
        reg.Open(HKEY_CURRENT_USER, RK_CONTROLPANEL_COMM);
        //Prefix
        //reg.ValueSZ(RV_CNCT, szBuf, CCHSIZEOF(szBuf));
        if( reg.ValueSZ(RV_CNCT, szBuf, CCHSIZEOF(szBuf)) ) {
            RasSetConnectText(hDlg, szBuf);
        }
        Button_SetCheck(DI(IDC_AUTO), reg.ValueDW(RV_AUTOCNCT));
        SetFocus(DI(IDC_AUTO)); 
        EnableRASWindows(hDlg);

        AygInitDialog( hDlg, SHIDIF_SIPDOWN );

        return 0; // don't let GWE set focus

    case WM_COMMAND:     
        switch (LOWORD(wParam))
        {
        case IDC_CHANGE:
            // Need to pass the current connection name to RasChangeDlgProc (after stripping 's)
            RasGetConnectText(hDlg, szBuf);
            pszConnect = (LPTSTR)DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_CHANGE_CONN), hDlg, (DLGPROC)RasChangeDlgProc, (LPARAM)szBuf);
            if(pszConnect && (DWORD)pszConnect!= -1)
            {
                RasSetConnectText(hDlg, pszConnect);
                // free the string
                MyFree(pszConnect);
            }
            // fall through
            
        case IDC_AUTO:
            EnableRASWindows(hDlg);
            return TRUE;

        case IDOK:
            DEBUGMSG(ZONE_COMM, (L"CommRASDlgProc: Got IDOK....\r\n"));
            // Get current connection & strip the surounding ' 's
            RasGetConnectText(hDlg, szBuf);
            dwAuto = Button_GetCheck(DI(IDC_AUTO));
            reg.Create(HKEY_CURRENT_USER, RK_CONTROLPANEL_COMM);
            reg.SetSZ(RV_CNCT, szBuf); 
            reg.SetDW(RV_AUTOCNCT, dwAuto);
            return TRUE;
        }
    }
    return (FALSE);
}

void RasSetConnectText(HWND hDlg, LPCTSTR pszConnect)
{
    TCHAR szBuf[MAX_CONNECT+3];
    
    if(pszConnect[0])
    {
        EnableWindow(DI(IDC_STATIC_DIRECT), TRUE);

        StringCchPrintf(szBuf, MAX_CONNECT+3, _T("'%s'"), pszConnect);

        SetWindowText(DI(IDC_STATIC_DIRECT), szBuf);
    }
}

void RasGetConnectText(HWND hDlg, LPTSTR szConnect)
{
    int iLen;
    TCHAR szBuf[MAX_CONNECT];
    //Prefix
    //GetDlgItemText(hDlg, IDC_STATIC_DIRECT, szBuf, MAX_CONNECT);
    if(GetDlgItemText(hDlg, IDC_STATIC_DIRECT, szBuf, MAX_CONNECT)) {
        // Strip the surounding ' 's
        iLen = lstrlen(szBuf);
        DEBUGCHK(iLen > 0 && szBuf[0]=='\'' && szBuf[iLen-1]=='\'');
        if (iLen > 0)
        {
            szBuf[iLen-1]=0; // strip trailing '
            lstrcpy(szConnect, szBuf+1); // +1 to strip leading '
        }
        else
        {
            szConnect[0] = 0;
        }
    }
}

void EnableRASWindows(HWND hDlg)
{
    static const int EnableCtrlTbl[] = {         // table of all control to be hide/show
        IDC_CHANGE,
        IDC_STATIC_DIRECT,
        IDC_CNCT_BOX,
        IDC_STATIC1
    };
    BOOL b;
    int  i;

    b = Button_GetCheck(DI(IDC_AUTO));
    for (i = 0; i < ARRAYSIZEOF(EnableCtrlTbl); i++)
        EnableWindow(GetDlgItem(hDlg, EnableCtrlTbl[i]), b);
}

extern "C" BOOL APIENTRY RasChangeDlgProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    HWND hwndCB;
    int     index;
    DECLAREWAITCURSOR;

    switch (message)
    {
    case WM_HELP:
        MyCreateProcess(TEXT("peghelp"), CommRasTab.pszHelp);
        break;

    case WM_INITDIALOG:
        DEBUGMSG(ZONE_COMM, (L"RasChangeDlgProc: Got WM_INITDLG....\r\n"));
        CenterWindowSIPAware(hDlg);
        hwndCB = GetDlgItem(hDlg, IDC_CB);
        // ComboBox_SetExtendedUI(hwndCB, TRUE);
        SetWaitCursor();
        LoadConnectoidList(hwndCB);

        // szConnect is passed in in lParam of WM_INITDIALOG
        index = ComboBox_FindString(hwndCB, -1, lParam); 
        if ( index == CB_ERR ) 
            index = 0;

        ComboBox_SetCurSel(hwndCB, index); 
        ResetWaitCursor();
        SetFocus(hwndCB);

        AygInitDialog( hDlg, SHIDIF_SIPDOWN );

        return 0; // don't let GWE set focus

    case WM_COMMAND:      
        switch (LOWORD(wParam))
        {
        case IDOK:
            DEBUGMSG(ZONE_COMM, (L"RasChangeDlgProc: Got IDOK....\r\n"));
            hwndCB = GetDlgItem(hDlg, IDC_CB);
            index = ComboBox_GetCurSel(hwndCB); 
            if ( index != CB_ERR ) {    
                TCHAR szConnect[MAX_CONNECT+1];
                if (ComboBox_GetLBText(hwndCB, index, szConnect) > 0) {
                    // alloc a copy of szConnect & return it
                    // the caller of thsi dlg box will free the string
                    EndDialog(hDlg, (int)MySzDup(szConnect));
                    return TRUE;
                }
            }
            // fall through
        case IDCANCEL:
            EndDialog(hDlg, 0); 
            return TRUE;
        }
    }    
    return FALSE;
}

void LoadConnectoidList(HWND hwndCB)
{
    LPRASENTRYNAME lpRasEntries;    // Pointer to the RasEntries.
    RASENTRY    RasEntry;
    DWORD        cEntries;        // Number of Entries found
    DWORD        index;            
    DWORD        cb;                // Number of bytes in RasEntryName list.
    
    RasEnumEntries (NULL, NULL, NULL, &cb, NULL);
    if (!cb) return;

    // Now we have to do something with the list.
    lpRasEntries = (LPRASENTRYNAME)LocalAlloc (LPTR, cb);
    
    if (lpRasEntries && !RasEnumEntries (NULL, NULL, lpRasEntries, &cb, &cEntries))
    {
        // Walk the list
        DEBUGMSG(ZONE_COMM, (L"RasEnumEntries: cEntries=%d\r\n", cEntries));

        for ( index = 0; index < cEntries; index++)
        {
            DEBUGMSG(ZONE_COMM, (L"RasEnumEntries: get device type for '%s' \r\n", lpRasEntries[index].szEntryName));
            
            // Get the Entry properties.
            cb = sizeof(RASENTRY);
            RasEntry.dwSize = sizeof(RASENTRY);
            if (RasGetEntryProperties (NULL, lpRasEntries[index].szEntryName,
                    &(RasEntry), &cb, NULL, NULL)) 
            {
                // Bad Entry, skip over it.
                ASSERT(FALSE);
                continue;
            }
            DEBUGMSG(ZONE_COMM, (L"RasEnumEntries: device='%s' \r\n", RasEntry.szDeviceType));

            // is it direct ?.
            if (lstrcmp(RasEntry.szDeviceType, RASDT_Direct)) {
                DEBUGMSG(ZONE_COMM, (L"RasEnumEntries: skipping DIRECT entry \r\n"));
                continue;
            }
            DEBUGMSG(ZONE_COMM, (L"RasEnumEntries: Add for '%s' device='%s' \r\n", lpRasEntries[index].szEntryName, RasEntry.szDeviceType));

            // Add the item.
            ComboBox_AddString(hwndCB, (LPTSTR)lpRasEntries[index].szEntryName);
        }
    }
    // Free the RasEntry info
    LocalFree ((HLOCAL)lpRasEntries);        
}
