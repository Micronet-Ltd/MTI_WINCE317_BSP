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
#include "taskbar.hxx"
#include "..\inc\resource.h"
#include "power.h"
#include "taskbarhelpers.h"

TCHAR const c_szPower[] = TEXT("Software\\Microsoft\\Power");
TCHAR const c_szShowPower[] = TEXT("ShowIcon");
TCHAR const c_szShowPowerWarnings[] = TEXT("ShowWarnings");

BOOL WINAPI Shell_NotifyIconI(DWORD dwMsg, PNOTIFYICONDATA pNID, DWORD cbNID);

void GetDisplayPowerSettings( BOOL *pShowPowerIcon, BOOL *pShowWarnings )
{
    HKEY hkey;
    LONG l;
    DWORD dw = 0;
    DWORD dwSize = sizeof(DWORD);

    // Open up the registry and mark this change
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szPower, 0,0,&hkey))
    {
        l = RegQueryValueEx(hkey, c_szShowPower, NULL, NULL, (LPBYTE) &dw, &dwSize);
        *pShowPowerIcon = ( ( l == ERROR_SUCCESS ) && ( dw != 0 ) );

        dwSize = sizeof(DWORD);
        l = RegQueryValueEx( hkey, c_szShowPowerWarnings, NULL, NULL, (LPBYTE) &dw, &dwSize );
        RegCloseKey( hkey );
        *pShowWarnings = ( ( l == ERROR_SUCCESS ) && ( dw != 0 ) );
    }
    else
    {
        // default to showing the power icon and warnings
        *pShowPowerIcon = TRUE;
        *pShowWarnings = TRUE;
    }
}

PowerManagerUI::PowerManagerUI()
{
    GetDisplayPowerSettings(&m_fShowPower, &m_fShowWarning);
    m_hInstance = NULL;
    m_hwndBBL = NULL;
    m_hwndBBVL = NULL;
    m_hwndMBVL = NULL;
    m_uID = -1;
    m_fShowingIcon = FALSE;
    m_fShowingBackupBatteryIcon = FALSE;
}

PowerManagerUI::PowerManagerUI(HINSTANCE hInstance)
{
    m_hInstance = hInstance;
    GetDisplayPowerSettings(&m_fShowPower, &m_fShowWarning);
    m_hwndBBL = NULL;
    m_hwndBBVL = NULL;
    m_hwndMBVL = NULL;
    m_uID = -1;
    m_fShowingIcon = FALSE;
    m_fShowingBackupBatteryIcon = FALSE;
}

PowerManagerUI::~PowerManagerUI()
{
}

void PowerManagerUI::CreatePowerTrayIcon(HWND hwndTB, UINT uID)
{
    if (!m_hInstance)
    {
        return;
    }

    m_uID = uID;
    if (m_fShowPower)
    {
        NOTIFYICONDATA nid;
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwndTB;
        nid.uID = m_uID;
        nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | HHTBF_DESTROYICON;
;       nid.uCallbackMessage = POWER_NOTIFYICON;
        nid.hIcon = (HICON)LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_POWER),
                                     IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
        LoadString(m_hInstance, IDS_POWER_TRAY, nid.szTip, 64); // 64 is the documented size in MSDN
        Shell_NotifyIconI(NIM_ADD, &nid, sizeof(nid));
        m_fShowingIcon = TRUE;
    }
}

void PowerManagerUI::UpdatePower(HWND hwndTB, PPOWER_BROADCAST pPowerInfo)
{
    static BYTE ACLineStatus, BatteryFlag, BackupBatteryFlag;
    NOTIFYICONDATA nid;

    if (!pPowerInfo)
    {
        return;
    }

    PPOWER_BROADCAST_POWER_INFO ppbpi = (PPOWER_BROADCAST_POWER_INFO) pPowerInfo->SystemPowerState;
    if (!ppbpi)
    {
        return;
    }

    // ASSUMPTION: This call shouldn't be made if both m_fShowPower and m_fShowPowerWarnings are FALSE
    ASSERT(m_fShowPower || m_fShowWarning);

    // update the icon
    if (m_fShowPower)
    {
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwndTB;
        nid.uFlags = NIF_MESSAGE | NIF_ICON;
        nid.uCallbackMessage = POWER_NOTIFYICON;
        nid.hIcon = NULL;
        nid.szTip[0] = 0;
        nid.uID = m_uID;

        if (ACLineStatus != ppbpi->bACLineStatus)
        {
            ACLineStatus  = ppbpi->bACLineStatus;
            if ((ACLineStatus == AC_LINE_ONLINE) && !(ppbpi->bBatteryFlag & BATTERY_FLAG_CHARGING))
            {
                nid.hIcon = (HICON)LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_POWER),
                                             IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
                nid.uFlags |= HHTBF_DESTROYICON;
                if (!m_fShowingIcon)
                {
                    Shell_NotifyIconI(NIM_ADD, &nid, sizeof(nid));
                    m_fShowingIcon = TRUE;
                }
                else
                {
                    Shell_NotifyIconI(NIM_MODIFY, &nid, sizeof(nid));
                }

            }

            // NOTE: reset to force a change below
            BatteryFlag=0;
        }

        if (BatteryFlag != ppbpi->bBatteryFlag)
        {
            // the battery status has changed, reset the warning message interval
            BatteryFlag  = ppbpi->bBatteryFlag;

            if (ACLineStatus == AC_LINE_ONLINE)
            {
                if (ppbpi->bBatteryFlag & BATTERY_FLAG_CHARGING)
                {
                    nid.hIcon = (HICON)LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_CHARGING),
                                                 IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
                }
                else
                {
                    nid.hIcon = (HICON)LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_POWER),
                                                 IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
                }
                nid.uFlags |= HHTBF_DESTROYICON;
                if (!m_fShowingIcon)
                {
                    Shell_NotifyIconI(NIM_ADD, &nid, sizeof(nid));
                    m_fShowingIcon = TRUE;
                }
                else
                {
                    Shell_NotifyIconI(NIM_MODIFY, &nid, sizeof(nid));
                }
            }
            else
            {
                BOOL bHandled = FALSE;

                if ((ppbpi->bBatteryFlag & BATTERY_FLAG_HIGH) || (ppbpi->bBatteryFlag == BATTERY_FLAG_UNKNOWN))
                {
                    Shell_NotifyIconI(NIM_DELETE, &nid, sizeof(nid));
                    if (m_hwndMBVL)
                    {
                        PostMessage(m_hwndMBVL, WM_COMMAND, IDOK, 0);
                    }
                    bHandled = TRUE;
                    m_fShowingIcon = FALSE;
                }

                if (!bHandled && (ppbpi->bBatteryFlag & BATTERY_FLAG_LOW))
                {
                    nid.hIcon = (HICON)LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_LOW),
                                                 IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

                    nid.uFlags |= HHTBF_DESTROYICON;
                    if (!m_fShowingIcon)
                    {
                        Shell_NotifyIconI(NIM_ADD, &nid, sizeof(nid));
                        m_fShowingIcon = TRUE;
                    }
                    else
                    {
                        Shell_NotifyIconI(NIM_MODIFY, &nid, sizeof(nid));
                    }
                    bHandled = TRUE;
                }


                if (!bHandled && ((ppbpi->bBatteryFlag & BATTERY_FLAG_CRITICAL) || (ppbpi->bBatteryFlag & BATTERY_FLAG_NO_BATTERY)))
                {
                    nid.hIcon = (HICON)LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_VERYLOW),
                                                 IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

                    nid.uFlags |= HHTBF_DESTROYICON;
                    if (!m_fShowingIcon)
                    {
                        Shell_NotifyIconI(NIM_ADD, &nid, sizeof(nid));
                        m_fShowingIcon = TRUE;
                    }
                    else
                    {
                        Shell_NotifyIconI(NIM_MODIFY, &nid, sizeof(nid));
                    }

                    // show the warning dialog
                    PostMessage(hwndTB, WM_COMMAND, IDM_TASKBAR_WARNING, IDD_MAIN_VLOW);

                    bHandled = TRUE;
                }

                if ( !bHandled )
                {
                    nid.hIcon = (HICON)LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_VERYLOW),
                                                 IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

                    nid.uFlags |= HHTBF_DESTROYICON;
                    if (!m_fShowingIcon)
                    {
                        Shell_NotifyIconI(NIM_ADD, &nid, sizeof(nid));
                        m_fShowingIcon = TRUE;
                    }
                    else
                    {
                        Shell_NotifyIconI(NIM_MODIFY, &nid, sizeof(nid));
                    }

                    if (BATTERY_FLAG_CRITICAL == ppbpi->bBatteryFlag)
                    {
                        // show the warning dialog
                        PostMessage(hwndTB, WM_COMMAND, IDM_TASKBAR_WARNING, IDD_MAIN_VLOW);
                    }
                }
            }
        }

        if (BackupBatteryFlag != ppbpi->bBackupBatteryFlag)
        {
            nid.uID = m_uID+1;

            BOOL bHandled = FALSE;

            if ((ppbpi->bBackupBatteryFlag == BATTERY_FLAG_UNKNOWN) || (ppbpi->bBackupBatteryFlag & BATTERY_FLAG_HIGH))
            {
                Shell_NotifyIconI(NIM_DELETE, &nid, sizeof(nid));
                if (m_hwndBBL)
                {
                    PostMessage(m_hwndBBL, WM_COMMAND, IDOK, 0);
                }
                if (m_hwndBBVL)
                {
                    PostMessage(m_hwndMBVL, WM_COMMAND, IDOK, 0);
                }
                bHandled = TRUE;
                m_fShowingBackupBatteryIcon = FALSE;
            }

            if (!bHandled && (ppbpi->bBackupBatteryFlag & BATTERY_FLAG_LOW))
            {
                // Backup Battery Low
                nid.hIcon = (HICON)LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_LOWBKUP),
                                             IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
                nid.uFlags |= HHTBF_DESTROYICON;
                if (!m_fShowingBackupBatteryIcon)
                {
                    Shell_NotifyIconI(NIM_ADD, &nid, sizeof(nid));
                    m_fShowingBackupBatteryIcon = TRUE;
                }
                else
                {
                    Shell_NotifyIconI(NIM_MODIFY, &nid, sizeof(nid));
                }

                // show the warning dialog
                PostMessage(hwndTB, WM_COMMAND, IDM_TASKBAR_WARNING, IDD_BACKUP_LOW);

                bHandled = TRUE;
            }

            if (!bHandled && ((ppbpi->bBackupBatteryFlag & BATTERY_FLAG_CRITICAL) || (ppbpi->bBackupBatteryFlag & BATTERY_FLAG_NO_BATTERY)))
            {
                nid.hIcon = (HICON)LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_VLOWBKUP),
                                             IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
                nid.uFlags |= HHTBF_DESTROYICON;
                if (!m_fShowingBackupBatteryIcon)
                {
                    Shell_NotifyIconI(NIM_ADD, &nid, sizeof(nid));
                    m_fShowingBackupBatteryIcon = TRUE;
                }
                else
                {
                    Shell_NotifyIconI(NIM_MODIFY, &nid, sizeof(nid));
                }

                // show the warning dialog
                PostMessage(hwndTB, WM_COMMAND, IDM_TASKBAR_WARNING, IDD_BACKUP_VLOW);

                bHandled = TRUE;
            }
        }

        BackupBatteryFlag = ppbpi->bBackupBatteryFlag;
    }
    else
    {
        if (ACLineStatus != ppbpi->bACLineStatus)
        {
            ACLineStatus  = ppbpi->bACLineStatus;

            // NOTE: Reset this in order to force a change below.
            BatteryFlag = 0;
        }

        if (BatteryFlag != ppbpi->bBatteryFlag)
        {
            BatteryFlag  = ppbpi->bBatteryFlag;

            if (ACLineStatus != AC_LINE_ONLINE)
            {
                BOOL bHandled = FALSE;
                if ((ppbpi->bBatteryFlag & BATTERY_FLAG_HIGH) || (ppbpi->bBatteryFlag == BATTERY_FLAG_UNKNOWN))
                {
                    if (m_hwndMBVL)
                    {
                        PostMessage(m_hwndMBVL, WM_COMMAND, IDOK, 0);
                    }
                    bHandled = TRUE;
                }
                if (!bHandled && ((ppbpi->bBatteryFlag & BATTERY_FLAG_CRITICAL) || (ppbpi->bBatteryFlag & BATTERY_FLAG_NO_BATTERY)))
                {
                    // send a warning dialog to the user
                    PostMessage(hwndTB, WM_COMMAND, IDM_TASKBAR_WARNING, IDD_MAIN_VLOW);
                    bHandled = TRUE;
                }
            }

            if (BackupBatteryFlag != ppbpi->bBackupBatteryFlag)
            {
                BOOL bHandled = FALSE;

                if ((ppbpi->bBackupBatteryFlag & BATTERY_FLAG_HIGH) || (ppbpi->bBackupBatteryFlag == BATTERY_FLAG_UNKNOWN))
                {
                    // Backup Battery High
                    if (m_hwndBBL)
                    {
                        PostMessage(m_hwndBBL, WM_COMMAND, IDOK, 0);
                    }
                    if (m_hwndBBVL)
                    {
                        PostMessage(m_hwndBBVL, WM_COMMAND, IDOK, 0);
                    }
                    bHandled = TRUE;
                }

                if (!bHandled && ( ppbpi->bBackupBatteryFlag & BATTERY_FLAG_LOW))
                {
                    // Backup Battery Low, show the user a warning
                    PostMessage(hwndTB, WM_COMMAND, IDM_TASKBAR_WARNING, IDD_BACKUP_LOW);
                    bHandled = TRUE;
                }

                if (!bHandled && ((ppbpi->bBackupBatteryFlag & BATTERY_FLAG_CRITICAL) ||
                                  (ppbpi->bBackupBatteryFlag & BATTERY_FLAG_NO_BATTERY)))
                {
                    // show the user a warning
                    PostMessage(hwndTB, WM_COMMAND, IDM_TASKBAR_WARNING, IDD_BACKUP_VLOW);
                }
            }

            BackupBatteryFlag = ppbpi->bBackupBatteryFlag;
        }
    }
}

BOOL PowerManagerUI::ShowPowerWarning(HWND hwnd, UINT nID, DLGPROC dlgproc)
/*---------------------------------------------------------------------------*\
 *
 \*---------------------------------------------------------------------------*/
{
    if (!m_fShowWarning)
    {
        return FALSE;
    }

    switch(nID)
    {
        case IDD_BACKUP_LOW:
            if (m_hwndBBL)
            {
                SetForegroundWindow(m_hwndBBL);
            }
            else
            {
                m_hwndBBL = CreateDialogParam(m_hInstance, MAKEINTRESOURCE(IDD_BACKUP_LOW), NULL,
                                              dlgproc, (LPARAM)IDD_BACKUP_LOW);
                ShowWindow(m_hwndBBL, SW_SHOWNORMAL);
            }
            return TRUE;

        case IDD_BACKUP_VLOW:
            if (m_hwndBBVL)
            {
                SetForegroundWindow(m_hwndBBVL);
            }
            else
            {
                m_hwndBBVL = CreateDialogParam(m_hInstance, MAKEINTRESOURCE(IDD_BACKUP_VLOW), NULL,
                                               dlgproc, (LPARAM)IDD_BACKUP_VLOW);
                ShowWindow(m_hwndBBVL, SW_SHOWNORMAL);
            }
            return TRUE;

        case IDD_MAIN_VLOW:
            if (m_hwndMBVL)
            {
                SetForegroundWindow(m_hwndMBVL);
            }
            else
            {
                m_hwndMBVL = CreateDialogParam(m_hInstance, MAKEINTRESOURCE(IDD_MAIN_VLOW), NULL,
                                               dlgproc, (LPARAM)IDD_MAIN_VLOW);
                ShowWindow(m_hwndMBVL, SW_SHOWNORMAL);
            }
            return TRUE;

        default:
            return FALSE;
    }

} /* ShowPowerWarning()
   */

BOOL PowerManagerUI::HandleIconMessage(HWND hwnd, WPARAM wp, LPARAM lp)
{
    if ((wp == m_uID) || (wp == (m_uID + 1)))
    {
        if (lp == WM_LBUTTONDBLCLK)
        {
            CStringRes sz(m_hInstance);
            ExecSilent(hwnd, (LPTSTR)TEXT("ctlpnl"), (LPTSTR)sz.LoadString(IDS_CTLPNLPOWER));
            return TRUE;
        }
    }
    
    return FALSE;
}

BOOL PowerManagerUI::IsPowerManagerUIDialogMessage(MSG msg)
{
    return !((!m_hwndBBL  || !IsDialogMessage(m_hwndBBL,  &msg)) && (!m_hwndBBVL || !IsDialogMessage(m_hwndBBVL, &msg)));
}

LRESULT PowerManagerUI::HandleWarningDialogMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    switch (msg)
    {
        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wp,lp))
            {
                case IDOK:
                case IDCANCEL:
                    lp = GetWindowLong(hwnd, DWL_USER);
                    switch (lp)
                    {
                        case IDD_BACKUP_LOW:
                            m_hwndBBL = NULL;
                            DestroyWindow(hwnd);
                            return TRUE;

                        case IDD_BACKUP_VLOW:
                            m_hwndBBVL = NULL;
                            DestroyWindow(hwnd);
                            return TRUE;

                        case IDD_MAIN_VLOW:
                            m_hwndMBVL = NULL;
                            DestroyWindow(hwnd);
                            return TRUE;
                    }
                    break;
                default:
                    break;
            }
            break;
    }
    return FALSE;

} /* WarningDlgProc()
   */
