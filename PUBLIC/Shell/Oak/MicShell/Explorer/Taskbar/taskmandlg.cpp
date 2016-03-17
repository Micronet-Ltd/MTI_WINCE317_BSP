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
#include <shellsdk.h>
#include "taskbar.hxx"
#include "taskmandlg.h"
#include "taskbaritem.h"
#include "aygutils.h"
#include "..\inc\resource.h"
#include "taskbarappbutton.h"
#include "taskbarhelpers.h"
#include "utils.h"

extern HINSTANCE g_hInstance;
extern CTaskBar *g_TaskBar;
TaskManDlg *g_TaskManDlg = NULL;

#define RestoreForegroundWindow(hwnd)    \
    SetForegroundWindow((HWND)(((DWORD)hwnd) | 0x01))

TaskManDlg::TaskManDlg()
{
    ASSERT(!g_TaskManDlg);
    if (!g_TaskManDlg)
    {
        g_TaskManDlg = this;
    }
}


TaskManDlg::~TaskManDlg()
{
    if (m_hwndTaskMan)
    {
        Destroy();
    }
}


void TaskManDlg::Create()
{
    m_hwndTaskMan = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_TASK_MANAGER),
                                 NULL, s_TaskManDlgProc);
} /* TaskMan_Create()
   */


void TaskManDlg::Show()
{
    if (m_hwndTaskMan)
    {
        SetForegroundWindow(m_hwndTaskMan);
        ShowWindow(m_hwndTaskMan, SW_SHOWNORMAL);
    }
}


void TaskManDlg::Destroy()
{
    if (m_hwndTaskMan)
    {
        DestroyWindow(m_hwndTaskMan);
        m_hwndTaskMan = NULL;
    }
} /* TaskMan_Destroy()
   */


int CALLBACK TaskManDlg::s_TaskManDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (g_TaskManDlg)
    {
        return g_TaskManDlg->TaskManDlgProc(hwnd, msg, wParam, lParam);
    }
    else
    {
        return 0;
    }
}


int CALLBACK TaskManDlg::TaskManDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndLB = GetDlgItem(hwnd, IDC_LISTBOX);
    WCHAR szPath[MAX_PATH];
    DWORD dwRet;
    int iIndex;

    switch(msg)
    {
        case WM_INITDIALOG:
            AygInitDialog( hwnd, SHIDIF_SIPDOWN );
            SetFocus(hwndLB);
            return -1;

        case WM_ACTIVATE:
            if (LOWORD(wParam) != WA_INACTIVE)
            {
                TCHAR szClass[32];
                TaskbarApplicationButton *ptbi = NULL;
                TaskbarApplicationButton *ptbiDone = NULL;

                ListBox_ResetContent(hwndLB);

                if (!g_TaskBar)
                {
                    return 0;
                }

                ptbi = g_TaskBar->GetNextAppButton(NULL);

                while (ptbi)
                {
                    if (ptbi->m_uFlags & HHTBS_CHECKED)
                    {
                        ptbiDone = ptbi;
                        break;
                    }
                    ptbi = g_TaskBar->GetNextAppButton(ptbi);
                }

                if (!ptbi)
                {
                    ptbi = g_TaskBar->GetNextAppButton(NULL);
                }

                do
                {
                    if (ptbi != NULL)
                    {
                        GetClassName(ptbi->m_hwndMain, szClass, 31);
                        if (!lstrcmpi(szClass, c_szExplore))
                        {
                            LoadString(g_hInstance, IDS_EXPLORER, szPath, ARRAYSIZE(szPath));
                            lstrcat(szPath, L" - ");
                            lstrcat(szPath, ptbi->m_wszItem);
                            iIndex = ListBox_AddString(hwndLB, szPath);
                        }
                        else
                        {
                            iIndex = ListBox_AddString(hwndLB, ptbi->m_wszItem);
                        }

                        if (iIndex != LB_ERR)
                        {
                            dwRet = ListBox_SetItemData(hwndLB, iIndex, (DWORD)ptbi->m_hwndMain);
                            if (ptbi->m_uFlags & HHTBS_CHECKED)
                            {
                                ListBox_SetCurSel(hwndLB, iIndex);
                            }
                        }

                        ptbi = g_TaskBar->GetNextAppButton(ptbi);
                    }
                    else
                    {
                        ptbi = g_TaskBar->GetNextAppButton(NULL);
                    }
                }
                while (ptbi != ptbiDone);

                // Disable unnecessary buttons
                if (0 == ListBox_GetCount(hwndLB))
                {
                    EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);
                    EnableWindow(GetDlgItem(hwnd, IDC_ENDTASK), FALSE);
                    SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwnd, IDCANCEL), (LPARAM)TRUE);
                }
                else
                {
                    EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
                    EnableWindow(GetDlgItem(hwnd, IDC_ENDTASK), TRUE);
                    SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)hwndLB, (LPARAM)TRUE);

                    // Select the first item if no buttons are checked
                    if (LB_ERR == ListBox_GetCurSel(hwndLB))
                    {
                       ListBox_SetCurSel(hwndLB, 0);
                    }
                }
                break;
            }

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDC_ENDTASK:
                {
                    HWND hwndApp;

                    // 11795 hide the window first instead of last...
                    ShowWindow(hwnd, SW_HIDE);

                    if ((iIndex = ListBox_GetCurSel(hwndLB)) != LB_ERR)
                    {
                        hwndApp = (HWND)ListBox_GetItemData(hwndLB, iIndex);
                        if (IsWindowEnabled(hwndApp))
                        {
                            PostMessage(hwndApp, WM_CLOSE, 0, 0);
                            Sleep(5000);
                        }

                        // If app is still around prompt for "really nuke?"
                        // IMPORTANT: First check if it is not a system process
                        if (IsWindow(hwndApp) && IsOKToKill(hwndApp) && g_TaskBar)
                        {
                            LPTBOBJ ptbobj;

                            ptbobj = g_TaskBar->FindItem(hwndApp);

                            if (ptbobj)
                            {
                                DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_SHUTDOWN), hwnd,
                                               (DLGPROC)s_EndTaskDlgProc, (LPARAM)ptbobj);
                            }
                        }
                    }
                    break;
                }

                case IDC_LISTBOX:
                    if (HIWORD(wParam) == LBN_DBLCLK)
                    {
                        if ((iIndex = ListBox_GetCurSel(hwndLB)) != LB_ERR)
                        {
                            RestoreForegroundWindow(ListBox_GetItemData(hwndLB, iIndex));
                            ShowWindow(hwnd, SW_HIDE);
                            ListBox_ResetContent(hwndLB);
                        }
                    }
                    break;

                case IDOK:
                    if ((iIndex = ListBox_GetCurSel(hwndLB)) != LB_ERR)
                    {
                        RestoreForegroundWindow(ListBox_GetItemData(hwndLB, iIndex));
                        ShowWindow(hwnd, SW_HIDE);
                        ListBox_ResetContent(hwndLB);
                    }
                    break;

                case IDCANCEL:
                    ShowWindow(hwnd, SW_HIDE);
                    ListBox_ResetContent(hwndLB);
                    break;
            }
            break;
    }
    return 0;
} /* TaskMan_DlgProc()
   */


BOOL TaskManDlg::IsTaskManDialogMessage(MSG msg)
{
    return (m_hwndTaskMan && IsDialogMessage(m_hwndTaskMan, &msg));
}


LRESULT CALLBACK TaskManDlg::s_EndTaskDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (g_TaskManDlg)
    {
        return g_TaskManDlg->EndTaskDlgProc(hwnd, msg, wParam, lParam);
    }
    else
    {
        return 0;
    }
}

LRESULT CALLBACK TaskManDlg::EndTaskDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LPTBOBJ ptbobj;
    DWORD dwProcessId;
    HANDLE hProc;

    switch (msg)
    {
        case WM_INITDIALOG:
            ptbobj = (LPTBOBJ)lParam;
            SetWindowLong(hwnd, DWL_USER, (LONG)ptbobj);
            SetWindowText(hwnd, ptbobj->ptbi->m_wszItem);
            AygInitDialog( hwnd, SHIDIF_SIPDOWN );
            MessageBeep((UINT)-1);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:
                    ptbobj = (LPTBOBJ)GetWindowLong(hwnd, DWL_USER);
                    GetWindowThreadProcessId(ptbobj->ptbi->m_hwndMain, &dwProcessId);
                    hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, dwProcessId);
                    TerminateProcess(hProc, (UINT)-1);
                    CloseHandle(hProc);

                case IDCANCEL:
                    EndDialog(hwnd, GET_WM_COMMAND_ID(wParam, lParam));
                    break;

                default:
                    break;
            }
            break;
    }
    return FALSE;

} /* EndTaskDlgProc()
   */


