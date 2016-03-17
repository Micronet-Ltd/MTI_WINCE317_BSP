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

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <shellsdk.h>
#include <shlwapi.h>
#include "taskbar.hxx"
#include "..\inc\resource.h"
#include "aygutils.h"
#include "utils.h"

extern HINSTANCE g_hInstance;
extern CTaskBar *g_TaskBar;

TCHAR const c_szRunHistory[] = TEXT("Explorer\\RunHistory");


HRESULT SHRegQuerySZEx(HKEY hkeyRoot,
                       LPCTSTR lpszKey,
                       LPCTSTR lpszValue,
                       LPTSTR lpszVal,
                       UINT cch)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    DWORD dwType, dw;
    LONG lRet;

    if (lpszKey == NULL) {
        return E_INVALIDARG;
    }

    dw = cch * sizeof(TCHAR);
    lRet = RegQueryValueEx(hkeyRoot, lpszValue, (LPDWORD)lpszKey, &dwType,
                           (LPBYTE)lpszVal, &dw);
    return (ERROR_SUCCESS == lRet) ? NOERROR: E_FAIL;

} /* SHRegQuerySZEx()
   */


HWND CTaskBar::GetRunDlg(VOID)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    return m_hwndRunDlg;
} /* GetRunDlg()
   */


BOOL CTaskBar::ShowRunDlg(VOID)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    if (m_hwndRunDlg == NULL) {
        m_hwndRunDlg = CreateDialogParam(g_hInstance, MAKEINTRESOURCE(DLG_RUN),
                                    NULL, (DLGPROC)s_RunDlg, (LPARAM)this);
    }

    SetForegroundWindow(m_hwndRunDlg);

    ShowWindow(m_hwndRunDlg, SW_SHOW);
    UpdateWindow(m_hwndRunDlg);
    return TRUE;

} /* TaskBar_ShowRunDlg()
   */


BOOL CTaskBar::GetRunDlgHistory(HWND hwndCB)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    TCHAR szPath[MAX_PATH], szValue[18];
    int i=0;

    if (!hwndCB) {
        return FALSE;
    }

    szPath[0] = 0;

    wsprintf(szValue, TEXT("%d"), i);
    while (SHRegQuerySZEx(HKEY_LOCAL_MACHINE, c_szRunHistory, szValue,
                          szPath, MAX_PATH) == NOERROR)
    {
        ComboBox_AddString(hwndCB, szPath);
        StringCbPrintf(szValue, sizeof(szValue), TEXT("%d"), ++i);
    }

    ComboBox_GetLBText(hwndCB, 0, szPath);
    SetWindowText(hwndCB, szPath);
    return TRUE;

} /* GetRunDlgHistory()
   */


BOOL CTaskBar::UpdateRunDlgHistory(HWND hwndCB, LPCTSTR lpszLastCmd)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    TCHAR szPath[MAX_PATH], szValue[18];
    DWORD dw;
    HKEY hKey;
    LONG lRet;
    int i;

    if ((i = ComboBox_FindStringExact(hwndCB, -1, lpszLastCmd)) != CB_ERR) {
        ComboBox_DeleteString(hwndCB, i);
    }
    ComboBox_InsertString(hwndCB, 0, lpszLastCmd);

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRunHistory,0,0, &hKey);
    if (ERROR_SUCCESS != lRet) {
        return FALSE;
    }

    for (i=0; i<ComboBox_GetCount(hwndCB); i++) {
        ComboBox_GetLBText(hwndCB, i, szPath);
        dw = (lstrlen(szPath) + 1) * sizeof(TCHAR);

        StringCbPrintf(szValue, sizeof(szValue), TEXT("%d"), i);
        RegSetValueEx(hKey, szValue, 0, REG_SZ, (LPBYTE)szPath, dw);
    }

    RegCloseKey(hKey);
    return TRUE;

} /* UpdateRunDlgHistory()
   */


DWORD CTaskBar::s_BrowseForRunFile(LPVOID lpParameter)
{
#define LEN_256    256

    TCHAR szFilter[LEN_256];
    TCHAR szPath[MAX_PATH];
    TCHAR szTemp[MAX_PATH];
    OPENFILENAME ofn = {0};
    LPTSTR lpszTemp = szFilter;
    HWND hwnd = (HWND) lpParameter;
    DWORD dwReturn = 0;

    for(int i=0; i<4; i++) {
        if (LoadString(g_hInstance, IDS_PROGRAMS+i, lpszTemp, (DWORD)LEN_256 + (DWORD)szFilter - (DWORD)lpszTemp)) {
            lpszTemp += lstrlen(lpszTemp)+1;
        } else {
            ASSERT(0 != _T("LoadString failed"));
        }
    }

    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = szFilter;
    *lpszTemp++ = L'\0';
    LoadString(g_hInstance, IDS_BROWSE, lpszTemp, (DWORD)LEN_256 + (DWORD)szFilter - (DWORD)lpszTemp);
    ofn.lpstrTitle = lpszTemp;
    ofn.lpstrFile = szPath;
    ofn.lpstrDefExt = TEXT("exe");
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_FILEMUSTEXIST;
    ofn.nMaxFile = MAX_PATH;
    ofn.hwndOwner = hwnd;
    szPath[0] = 0;

    if (GetOpenFileName(&ofn)) {
        if (_tcschr(szPath, TEXT(' '))) {
            lstrcpy(szTemp, TEXT("\""));
            lstrcat(szTemp, szPath);
            lstrcat(szTemp, TEXT("\""));
            lstrcpy(szPath, szTemp);
        }

        SetDlgItemText(hwnd, IDD_COMMAND, szPath);
        EnableWindow(GetDlgItem(hwnd, IDOK),
                     lstrlen(szPath) ? TRUE : FALSE);
        SendMessage(hwnd, WM_NEXTDLGCTL,
                    (WPARAM)GetDlgItem(hwnd, IDOK), (LPARAM)TRUE);

        dwReturn = 1;
    }

    return dwReturn;
}


LRESULT CALLBACK CTaskBar::s_RunDlg(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (g_TaskBar)
        return g_TaskBar->RunDlg(hwnd,msg,wp,lp);
    else
        return 0;
}


LRESULT  CTaskBar::RunDlg(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    TCHAR szPath[MAX_PATH], szTemp[MAX_PATH];
    BOOL fOK;
    HIMC hIMC;

    switch (msg) {
        case WM_INITDIALOG:
            // Initalize the SipPref control if necessary
            WNDCLASS wc;
            if (GetClassInfo(g_hInstance, WC_SIPPREF, &wc))
            {
                CreateWindow(WC_SIPPREF, NULL, WS_CHILD, -10, -10,
                             5, 5, hwnd, NULL, g_hInstance, NULL);
            }

            AygInitDialog( hwnd, SHIDIF_SIZEDLG );

    #ifdef TASKBAR_HOST
            Host_CenterWindow(hwnd, NULL);
    #endif
            SendMessage(GetDlgItem(hwnd, IDD_COMMAND), CB_LIMITTEXT, MAX_PATH-1, 0);
            GetRunDlgHistory(GetDlgItem(hwnd, IDD_COMMAND));

            GetDlgItemText(hwnd, IDD_COMMAND, szPath, MAX_PATH);
            EnableWindow(GetDlgItem(hwnd, IDOK), lstrlen(szPath) ? TRUE : FALSE);
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wp,lp)) {

                case IDD_COMMAND:
                    if (GET_WM_COMMAND_CMD(wp,lp) == CBN_EDITCHANGE) {
                        GetDlgItemText(hwnd, IDD_COMMAND, szPath, MAX_PATH);
                        EnableWindow(GetDlgItem(hwnd, IDOK),
                                     lstrlen(szPath) ? TRUE : FALSE);

                    }else if (GET_WM_COMMAND_CMD(wp,lp) == CBN_SELENDOK) {
                        EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
                    }
                    break;

                case IDD_BROWSE:
                {
                    // Since we are part of the shell we shouldn't block everybody by displaying
                    // a modal dialog, spin it off on a thread so the shell can still do work
                    HANDLE hThread = NULL;
                    hThread = CreateThread(NULL, 0,
                                           (LPTHREAD_START_ROUTINE) s_BrowseForRunFile,
                                           (LPVOID) hwnd, NULL, NULL);
                    CloseHandle(hThread);
                    break;
                }

                case IDOK:
                {
                    LPTSTR lpszArgs = NULL;

                    if (m_bTaskBarHidden == FALSE) {
                        SetFocus(m_hwndTaskBar);
                    }

                    GetDlgItemText(hwnd, IDD_COMMAND, szPath, MAX_PATH);

                    // PathRemoveArgs can't handle leading spaces, so handle them manually...
                    TCHAR *szPathWithoutLeadingSpaces = szPath;
                    while(*szPathWithoutLeadingSpaces == TEXT(' ')) {
                        szPathWithoutLeadingSpaces++;
                    }
                    lstrcpy(szTemp, szPathWithoutLeadingSpaces);

                    UINT cchBuf = sizeof(szTemp)/sizeof(*szTemp);
                    if (!PathIsURL(szTemp) &&
                        !GetParsingName(szPathWithoutLeadingSpaces, szTemp, &cchBuf))
                    {
                        PathRemoveArgs(szTemp);
                        // Remove any trailing spaces
                        //PathRemoveBlanks(szTemp);
                        int cchLen = wcslen(szTemp) - 1;
                        while(cchLen > 0 && szTemp[cchLen] == L' ')
                            cchLen--;
                        szTemp[cchLen+1] = L'\0';

                        lpszArgs = PathGetArgs(szPathWithoutLeadingSpaces);
                        //PathRemoveBlanks(lpszArgs);
                        cchLen = wcslen(lpszArgs) - 1;
                        while(cchLen > 0 && lpszArgs[cchLen] == L' ')
                            cchLen--;
                        lpszArgs[cchLen+1] = L'\0';
                    }

#ifdef TASKBAR_HOST
                    fOK = SUCCEEDED(Host_Exec(hwnd, szTemp, lpszArgs));
#else
                    SHELLEXECUTEINFO sei = {0};
                    sei.cbSize = sizeof(sei);
                    sei.hwnd = hwnd;
                    sei.lpFile = szTemp;
                    sei.lpParameters = lpszArgs;
                    sei.nShow = SW_SHOWNORMAL;
                    fOK = ShellExecuteEx(&sei);
#endif
                    SetCursor(LoadCursor(NULL, IDC_ARROW));

                    if (fOK) {
                        UpdateRunDlgHistory(GetDlgItem(hwnd, IDD_COMMAND), szPath);
                    }else{
                        SendMessage(hwnd, WM_NEXTDLGCTL,
                                    (WPARAM)GetDlgItem(hwnd, IDD_COMMAND),
                                    (LPARAM)TRUE);
                        return FALSE;
                    }
                }
                // Drop though to the cancel option

                case IDCANCEL:
                    hIMC = ImmGetContext(hwnd);
                    if (hIMC) {
                        if (ImmGetOpenStatus(hIMC)) {
                            if (ImmSetCompositionString(hIMC, SCS_SETSTR, "", 0, NULL, 0))
                                ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
                            else
                                ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
                        }
                        ImmReleaseContext(hwnd, hIMC);
                    }
                    //
                    DestroyWindow(hwnd);
                    m_hwndRunDlg = NULL;

                    if (GET_WM_COMMAND_ID(wp,lp) == IDCANCEL && !m_bTaskBarHidden) {
                        SetFocus(m_hwndTaskBar);
                    }
                    break;

                default:
                    break;
            }
            break;
    }
    return FALSE;

} /* RunDlg()
   */
