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
#include <commctrl.h>
#include <commdlg.h>
#include <hshell.h>
#include <shobjidl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <msgqueue.h>
#include <notify.h>
#include "..\inc\resource.h"
#include "list.hpp"
#include "power.h"
#include "taskbar.hxx"

#include "taskbaritem.h"
#include "taskbarbutton.h"
#include "taskbarappbutton.h"
#include "taskbarnotification.h"
#include "taskbarbubble.h"
#include "taskmandlg.h"
#include "extrasmenu.h"

#include "trayp.h"
#include "tbdrop.h"
#include <winuserm.h>
#include <shellsdk.h>
#include "inputlang.hpp"

#include "stmenu.h"
#include "bubble.h"
#include "utils.h"
#include "aygutils.h"
#include "debugzones.h"
#include "taskbarhelpers.h"

extern CTaskBar *g_TaskBar;
extern CDesktopWnd *g_Desktop;

WCHAR const c_szPegHelp[] = L"\\windows\\peghelp.exe";

#ifndef _PREFAST_
#pragma warning(disable:4068)
#endif // _PREFAST_


void DrawArrow (HDC hdc, HBRUSH hbr, int dir, const RECT &rcBounds);
void DrawRect (HDC hdc, HBRUSH hbr, int x1, int y1, int x2, int y2);
BOOL WINAPI Shell_NotifyIconI(DWORD dwMsg, PNOTIFYICONDATA pNID, DWORD cbNID);
BOOL Keymap_ProcessKey(UINT uiMsg, WPARAM wParam, LPARAM lParam);
BOOL SIP_UpdateSipPosition(); // sip2.cpp


HWND Desktop_GetWindow()
{
    return g_Desktop->GetWindow();
}

HWND Taskbar_GetWindow()
{
    return g_TaskBar->GetWindow();
}

BOOL Taskbar_ChangeItemText(HWND hwndApp, LPCWSTR pszText)
{
    return g_TaskBar->ChangeItemText(hwndApp, pszText);
}


// start menu code
#if 0  // commented out pending implementation
// host code, ex: Host_Exec
#define TASKBAR_HOST
// hook for alt-tab
#define TASKBAR_TABHOOK
#endif


#ifdef TASKBAR_TABHOOK

typedef LPTBOBJ (WINAPI *PFNTBGETITEM)(HWND hwndTB, int iItem);
typedef HRESULT (CALLBACK *PFNALTTAB)(HWND hwndTB, int count, PFNTBGETITEM pfnTBGetItem);

HINSTANCE g_hdllAltTabHook;
PFNALTTAB g_pfnAltTabHook;

#endif


// pull in from coredll
extern "C" int GetWindowTextWDirect(HWND, LPWSTR, int);

#define USE_SW_MINIMIZE    1

#define RestoreForegroundWindow(hwnd)    \
    SetForegroundWindow((HWND)(((DWORD)hwnd) | 0x01))
#define IN_MENU(x) (x && x->InMenu())

#define CAPSLOCK_NOTIFYICON           (WM_USER + 2)
#define WM_UPDATECLOCK                (WM_USER + 3)

// Taskbar notification Icons
#define HHTBI_POWER        1
#define HHTBI_CAPSLOCK     3

HBRUSH g_hbrMonoDither;

WCHAR const c_szEllipses[] = L"...";
//#define CCHELLIPSES 3

WCHAR const c_szExplorer[] = L"Explorer";
WCHAR const c_szClock[] = L"Software\\Microsoft\\Clock";
WCHAR const c_szShellKeys[] = L"Software\\Microsoft\\Shell\\Keys";
WCHAR const c_szShellAutoHide[] = L"Software\\Microsoft\\Shell\\AutoHide";
WCHAR const c_szShellOnTop[] = L"Software\\Microsoft\\Shell\\OnTop";
WCHAR const c_szShowClock[] = L"SHOW_CLOCK";
WCHAR const c_szExpandControlPanel[] = L"ExpandControlPanel";
WCHAR const c_szExplore[]  = L"Explore";

#define ADMIN_TIMER_PERIOD            30*1000
#define TASKBAR_SENDMESSAGE_TIMEOUT 2000

//
// sound setting in registry
//
WCHAR const c_szMinimize[] = L"Minimize";
WCHAR const c_szMaximize[] = L"Maximize";
WCHAR const c_szControlPanel[] = L"control.exe";

extern HINSTANCE g_hInstance;


extern BOOL SIP_HaveSip();
extern BOOL SIP_SipRegisterNotification(HWND);
extern BOOL SIP_InitializeSipRect();

CTaskBar::CTaskBar()
{
    m_fShowClock=TRUE;
    BOOL fShowPower = FALSE, fShowPowerWarnings = FALSE;

    GetDisplayPowerSettings(&fShowPower, &fShowPowerWarnings);
    if (fShowPower || fShowPowerWarnings)
    {
        m_pPowerUI = new PowerManagerUI(g_hInstance);
    }
    else
    {
        m_pPowerUI = NULL;
    }

    m_pTaskManDlg = new TaskManDlg();
    m_pExtrasMenu = NULL;

    m_bExpandControlPanel = FALSE;
    m_bTaskBarAbortHide = FALSE;
    m_bTaskBarSlide = TRUE;
    m_bTaskBarTameStartMenu = FALSE;
    m_bInPopupMenu = FALSE;
    m_bInSipMenu = FALSE;

    m_currentNumberOfApps = 0;     // maintained at insertsion/deletion number of apps we are keeping track of
    m_indexFirstApp = 0;           // the first item in the list
    m_firstNotifyToDisplay = 0; // 0-based off of the first notify

    m_fTaskbarDisabled = FALSE;
    m_hwndDisablerWindow = NULL;

    m_fIsSmallScreen = FALSE;

    m_pBubbleMgr = NULL;

    m_ptLastTrayMouseMove.x = -1;
    m_ptLastTrayMouseMove.y = -1;

    m_fIsSmallScreen = IsSmallScreen();

    if (m_fIsSmallScreen)
    {
        m_maxNotifysToDisplay = 2;
        m_maxAppsToDisplay = 1;        // can be zero

        m_hfont = GetTBFont(12);
        m_fExtraAppsButton = TRUE;         // display a button for other apps
        m_fShowDesktopButton = FALSE;
        m_fShowSipButton = TRUE;
        SEPERATOR = 2;
        NUMITEMS = 1;
    }
    else
    {
        m_maxNotifysToDisplay = 16384; // something very large (to prohibit the spinn buttons)
        m_maxAppsToDisplay = 16384;        // some large upper bound

        m_hfont = NULL;
        m_fExtraAppsButton = FALSE;         // don't display a button for other apps
        m_fShowDesktopButton = TRUE;
        m_fShowSipButton = TRUE;
        SEPERATOR = 2;
        NUMITEMS = 3;
    }

    m_fShowSipButton = m_fShowSipButton && SIP_HaveSip();

    InputLang_t::InitializeInputLangList();
}

CTaskBar::~CTaskBar()
{
    Cleanup();

    InputLang_t::UnInitializeInputLangList();
}


LPTBOBJ  CTaskBar::FindItem(HWND hwndItem)
/*---------------------------------------------------------------------------*\
*
\*---------------------------------------------------------------------------*/
{
   LPTBOBJ ptbobj = (LPTBOBJ)m_plistTaskBar->Head();

   while ((ptbobj != NULL) && (ptbobj->ptbi != NULL) && !(ptbobj->ptbi->m_uFlags & HHTBS_NOTIFY))
   {
        if (hwndItem == ptbobj->ptbi->m_hwndMain)
        {
            return ptbobj;
        }
        ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
   }

   return NULL;

} /* FindItem */


BOOL CTaskBar::GetExistingApplications(HWND hwndTB)
/*---------------------------------------------------------------------------*\
*
\*---------------------------------------------------------------------------*/
{
    HWND hwndApp, hwndDesktop = Desktop_GetWindow();

    hwndApp = ::GetWindow(hwndTB, GW_HWNDLAST);

    while (hwndApp)
    {
        if (IsWindowVisible(hwndApp) &&
            (hwndApp != hwndTB) &&
            (hwndApp != hwndDesktop) &&
            !::GetWindow(hwndApp, GW_OWNER) &&
            (GetParent(hwndApp) == NULL) &&
            !(GetWindowLong(hwndApp, GWL_STYLE) & WS_CHILD) &&
            !FindItem(hwndApp) &&
            !(GetWindowLong(hwndApp, GWL_EXSTYLE) & WS_EX_TOOLWINDOW))
        {
            PostMessage(hwndTB, WM_SHELLNOTIFY, SHELLNOTIFY_WINDOWCREATED, (LPARAM)hwndApp);
        }
        hwndApp = ::GetWindow(hwndApp, GW_HWNDPREV);
    }

    return TRUE;

}
/* GetExistingApplications() */

#define TIME_CHANGE_EVENT _T("TaskbarTimeChangeEvent")
#define TIME_CHANGE_NOTIFICATION _T("\\\\.\\Notifications\\NamedEvents\\TaskbarTimeChangeEvent")

DWORD CTaskBar::MessageLoop()
{
    MSG msg;
    HANDLE rghWaits[2] = { NULL, NULL };
    HANDLE hReq = NULL;
    int cEvents = 0;
    DWORD dwTimeEvent = WAIT_OBJECT_0;
    DWORD dwPowerEvent = WAIT_OBJECT_0 + 1;
    BOOL bHaveTimeEvent = FALSE;
    BOOL bHavePowerEvent = FALSE;
    int iPower = 0;
    BYTE pbMsgBuf[sizeof(POWER_BROADCAST) + sizeof(POWER_BROADCAST_POWER_INFO)];
    PPOWER_BROADCAST ppb = (PPOWER_BROADCAST) pbMsgBuf;
    MSGQUEUEOPTIONS msgopts;


    __try
    {
        BOOL bUseOldStyleMessagePump = FALSE;

        RETAILMSG(TRUE, (L"Explorer(V2.0) taskbar thread started.\r\n"));

        RETAILMSG(0, (L"Taskbar: Attempting to use MsgWaitForMultipleObjects.\r\n"));

        CeRunAppAtEvent(TIME_CHANGE_NOTIFICATION, NOTIFICATION_EVENT_NONE);

        // create the time change event
        rghWaits[ cEvents ] = CreateEvent(NULL, FALSE, FALSE, TIME_CHANGE_EVENT);
        if (rghWaits[ cEvents ] != NULL)
        {
            if (!CeRunAppAtEvent(TIME_CHANGE_NOTIFICATION, NOTIFICATION_EVENT_TIME_CHANGE))
            {
                CloseHandle(rghWaits[ cEvents ]);
                rghWaits[ cEvents ] = NULL;
                dwPowerEvent = WAIT_OBJECT_0;
            }
            else
            {
                cEvents++;
                bHaveTimeEvent = TRUE;
            }
        }

        if (m_pPowerUI != NULL)
        {
            // Create our message queue
            memset(&msgopts, 0, sizeof(msgopts));
            msgopts.dwSize = sizeof(msgopts);
            msgopts.dwFlags = 0;
            msgopts.dwMaxMessages = 0;
            msgopts.cbMaxMessage = sizeof(pbMsgBuf);
            msgopts.bReadAccess = TRUE;

            rghWaits[ cEvents ] = CreateMsgQueue(NULL, &msgopts);
            if (!rghWaits[cEvents])
            {
                ERRORMSG(1, (L"Could not create power message queue\r\n"));
                if (cEvents > 0)
                {
                    // can't do power, but we have the time events, so lets use the new message pump
                    goto MSGWAITFORMULTIPLEOBJECTS;
                }
                else
                {
                    bUseOldStyleMessagePump = TRUE;
                    goto NO_MSGWAITFORMULTIPLEOBJECTS;
                }
            }

            // Request notifications
            hReq = RequestPowerNotifications(rghWaits[cEvents], PBT_POWERINFOCHANGE);
            if (!hReq)
            {
                CloseHandle(rghWaits[ cEvents ]);
                ERRORMSG(1, (L"RequestPowerNotifications() failed\r\n"));
                if (cEvents == 0)
                {
                    bUseOldStyleMessagePump = TRUE;
                    goto NO_MSGWAITFORMULTIPLEOBJECTS;
                }
            }
            else
            {
                iPower = cEvents;
                cEvents++;
                bHavePowerEvent = TRUE;
            }
        }

MSGWAITFORMULTIPLEOBJECTS:

        if (cEvents < 1)
        {
            // no events to wait for, don't waste time
            bUseOldStyleMessagePump = TRUE;
            goto NO_MSGWAITFORMULTIPLEOBJECTS;
        }

        RETAILMSG(0, (L"Taskbar: Using MsgWaitForMultipleObjects.\r\n"));

        while (TRUE)
        {
            DWORD dwWaitCode = MsgWaitForMultipleObjectsEx(cEvents, rghWaits, INFINITE, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
            if (dwWaitCode == (WAIT_OBJECT_0 + cEvents))
            {
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    if (msg.message == WM_QUIT)
                    {
                        goto NO_MSGWAITFORMULTIPLEOBJECTS;
                    }

                    if (((m_pTaskManDlg == NULL) || !m_pTaskManDlg->IsTaskManDialogMessage(msg)) &&
                        (!GetRunDlg() || !IsDialogMessage(GetRunDlg(), &msg)) &&
                        ((m_pPowerUI == NULL) || !m_pPowerUI->IsPowerManagerUIDialogMessage(msg)))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }
            }
            else if (bHaveTimeEvent && (dwWaitCode == dwTimeEvent))
            {
                // update the tray clock
                if (m_hwndTaskBar)
                {
                    PostMessage(m_hwndTaskBar, WM_UPDATECLOCK, 0, 0);
                }
            }
            else if (bHavePowerEvent && (dwWaitCode == dwPowerEvent))
            {
                DWORD dwSize, dwFlags;

                if (ReadMsgQueue(rghWaits[iPower], ppb, sizeof(pbMsgBuf), &dwSize, 0, &dwFlags))
                {
                    if (m_pPowerUI != NULL)
                    {
                        m_pPowerUI->UpdatePower(m_hwndTaskBar, ppb);
                    }
                }
                else
                {
                    DEBUGCHK(FALSE); // We should never get here
                }
            }
            else if (dwWaitCode == -1)
            {
                ERRORMSG(1, (L"Error Return Value from Wait For Multiple Objects\r\n"));
                DEBUGCHK(FALSE);
            }
            else
            {
                ERRORMSG(1, (L"Failure from Wait For Multiple Objects %d\r\n", dwWaitCode));
                DEBUGCHK(FALSE);
            }
        }

NO_MSGWAITFORMULTIPLEOBJECTS:

        if (bUseOldStyleMessagePump)
        {
            RETAILMSG(0, (L"Explorer(V2.0) using old style message pump.\r\n"));
            while (GetMessage(&msg, NULL, 0, 0))
            {
                if (((m_pTaskManDlg == NULL) || !m_pTaskManDlg->IsTaskManDialogMessage(msg)) &&
                    (!GetRunDlg() || !IsDialogMessage(GetRunDlg(), &msg)) &&
                    ((m_pPowerUI == NULL) || !m_pPowerUI->IsPowerManagerUIDialogMessage(msg)))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return 0;
    }

    // clean up
    CeRunAppAtEvent(TIME_CHANGE_NOTIFICATION, NOTIFICATION_EVENT_NONE);

    if (hReq)
    {
        StopPowerNotifications(hReq);
    }

    for (int i = 0; i < cEvents; i++)
    {
        CloseHandle(rghWaits[i]);
    }

    return msg.wParam;
}

void CTaskBar::SetAdminTimer()
{
    SetTimer(m_hwndTaskBar, IDT_ADMIN, ADMIN_TIMER_PERIOD, NULL);
}

void CTaskBar::SetHideTimer()
{
    if (m_bTaskBarAutoHide)
    {
        SetTimer(m_hwndTaskBar, IDT_AUTOHIDE, 500, NULL);
    }
}

void CTaskBar::SetUnhideTimer(LONG x, LONG y)
{
    if (m_bTaskBarAutoHide && m_bTaskBarHidden)
    {
        // Figure out how far the mouse has moved since last time.
        LONG dx = x - m_ptLastMouseMove.x;
        LONG dy = y - m_ptLastMouseMove.y;
        LONG rr = dx*dx + dy*dy;

        // RETAILMSG(1, (L"SetUnhideTimer: rr = %d\r\n", rr));

        // If the mouse has moved too far, reset the timer.
        if (rr > 40)
        {
            SetTimer(m_hwndTaskBar, IDT_AUTOUNHIDE, 100, NULL);
            m_ptLastMouseMove.x = x;
            m_ptLastMouseMove.y = y;
        }
    }
}


BOOL CTaskBar::AutoHideTimer(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    DEBUGMSG(ZONE_INFO, (L"AutoHideTimer fired\r\n"));
    if (m_bTaskBarAutoHide && !m_bTaskBarHidden &&
        GetForegroundWindow() != m_hwndTaskBar)
    {
        Hide(m_hwndTaskBar);
    }

    return TRUE;
}


BOOL CTaskBar::AutoUnhideTimer(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    DEBUGMSG(ZONE_INFO, (L"AutoUnhideTimer fired\r\n"));
    KillTimer(m_hwndTaskBar, IDT_AUTOUNHIDE);
    m_ptLastMouseMove.x = 0;
    m_ptLastMouseMove.y = 0;

    if (m_bTaskBarHidden)
    {
        // Make sure the mouse is still in the taskbar.
        POINT pt;
        RECT rc;
        if (GetCursorPos(&pt) &&
            GetWindowRect(m_hwndTaskBar, &rc) &&
            PtInRect(&rc, pt))
        {
            Show(m_hwndTaskBar);
        }
    }

    return TRUE;
}

void  CTaskBar::SlideWindow(HWND hwnd, const RECT &rcNew, BOOL fShow)
{
    if (m_bHasMouse)
    {
        // We do this to refresh the bitmap cache for the image list
        ImageList_DragMove((int) -1, (int) -1);
        ImageList_DragShowNolock(FALSE);
    }

    if (m_bTaskBarSlide)
    {
        int dt = fShow ? 200 : 350;
        int dy, t, tStart, tEnd, prio;
        RECT rcOld, rcMove;
        LONG top;
        HANDLE me = GetCurrentThread();
        prio = GetThreadPriority(me);
        SetThreadPriority(me, THREAD_PRIORITY_ABOVE_NORMAL);

        // Make sure the window is the top of the top.
        if (fShow)
        {
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                         SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
        }

        GetWindowRect(hwnd, &rcOld);

        dy = rcNew.top - rcOld.top;

        tStart = GetTickCount();
        tEnd = tStart + dt;

        rcMove = rcOld;

        while ((t = GetTickCount()) < tEnd)
        {
            if (m_bTaskBarAbortHide && !fShow)
            {
                break;
            }

            top = rcOld.top + (dy * (t - tStart) / dt);
            if (rcMove.top != top)
            {
                rcMove.top = top;
                MoveWindow(hwnd, rcMove.left, rcMove.top, rcMove.right - rcMove.left,
                           rcMove.bottom - rcMove.top, FALSE);  // last param is ignored
                InvalidateRect(hwnd, NULL, TRUE);

                if (!fShow)
                {
                    // Don't call UpdateWindow because the target app could be dead.
                    // UpdateWindow(FindCheckedItem(hwnd));
                    POINT p;
                    p.x = (rcOld.left+rcOld.right)/2;
                    p.y = rcOld.top;
                    UpdateWindow(WindowFromPoint(p));
                }
                UpdateWindow(hwnd);
            }
        }

        SetThreadPriority(me, prio);
    }

    if (m_bTaskBarAbortHide && !fShow)
    {
        return;
    }

    m_bTaskBarHidden = !fShow;

    MoveWindow(hwnd, rcNew.left, rcNew.top, rcNew.right - rcNew.left,
               rcNew.bottom - rcNew.top, FALSE); // last param is ignored
    InvalidateRect(hwnd, NULL, TRUE);


    UpdateWindow(hwnd);

    // Back to the way you were before.
    if (!fShow)
    {
        SetWindowPos(hwnd, m_bTaskBarOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                     0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    }

    if (m_bHasMouse)
    {
        // We do this to refresh the bitmap cache for the image list
        POINT pt;
        GetCursorPos(&pt);
        ImageList_DragShowNolock(TRUE);
        ImageList_DragMove(pt.x, pt.y);
    }
}

void CTaskBar::Show(HWND hwnd)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    KillTimer(m_hwndTaskBar, IDT_AUTOUNHIDE);
    if (m_bTaskBarHidden)
    {
        RECT rc;
        rc.left = 0;
        rc.right = GetSystemMetrics(SM_CXSCREEN);
        rc.bottom = GetSystemMetrics(SM_CYSCREEN);
        rc.top = rc.bottom - TASKBAR_HEIGHT;
        m_bTaskBarHidden = FALSE;
        ShowWindow(m_hwndTray, SW_SHOWNORMAL);

        SlideWindow(m_hwndTaskBar, rc, TRUE);

        // If we aborted a hide, make sure we are still visible.
        if (m_bTaskBarAbortHide)
        {
            m_bTaskBarHidden = FALSE;
            ShowWindow(m_hwndTray, SW_SHOWNORMAL);
        }

        if (m_bHasMouse)
        {
            SetHideTimer();
        }
    }

    m_bTaskBarAbortHide = FALSE;
} /* Show()
   */


void CTaskBar::Hide(HWND hwnd)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    // Don't really hide it if the mouse is still over the taskbar.
    // We need this code here because WM_KILLFOCUS tries to hide the taskbar.
    POINT pt;
    RECT rc;
    // GetCursorPos fails on a mouseless device,
    // so we won't return on touch-screen devices..
    BOOL b1 = GetCursorPos(&pt), b2 = GetWindowRect(hwnd, &rc);
    BOOL b3 = InflateRect(&rc, 6, 6);
    BOOL b4 = PtInRect(&rc, pt);
    DEBUGMSG(ZONE_VERBOSE, (L"HIDE: %d, %d, %d, %d - (%d, %d) - (%d, %d, %d, %d)\r\n",
                  b1, b2, b3, b4,
                  pt.x, pt.y, rc.left, rc.top, rc.right, rc.bottom));
    if (m_bInPopupMenu ||
        (GetCursorPos(&pt) &&
         GetWindowRect(m_hwndTaskBar, &rc) &&
         InflateRect(&rc, 6, 6) &&
         PtInRect(&rc, pt)))
    {
        if (m_bHasMouse)
        {
            SetHideTimer();
        }
        return;
    }

    KillTimer(m_hwndTaskBar, IDT_AUTOUNHIDE);
    KillTimer(m_hwndTaskBar, IDT_AUTOHIDE);
    if (m_bTaskBarAutoHide && !m_bTaskBarHidden)
    {
        rc.left = 0;
        rc.right = GetSystemMetrics(SM_CXSCREEN);
        rc.bottom = GetSystemMetrics(SM_CYSCREEN);
        rc.top = rc.bottom - TASKBAR_HEIGHT_AUTOHIDE;

        SlideWindow(m_hwndTaskBar, rc, FALSE);

        // If the hide was not aborted, finally hide it.
        if (!m_bTaskBarAbortHide)
        {
            m_bTaskBarHidden = TRUE;
            ShowWindow(m_hwndTray, SW_HIDE);
        }
    }

    m_bTaskBarAbortHide = FALSE;
} /* Hide() */

// Taskbar is "active" if
// (a) the foreground window is the taskbar or a window owned by the it, or
// (b) the start/extras/sip menu is showing
BOOL CTaskBar::IsActive()
{
    BOOL fActive = FALSE;
    HWND hwnd = GetForegroundWindow();

    if (hwnd &&
        (hwnd == m_hwndTaskBar|| (GetParent(hwnd) == m_hwndTaskBar)))
    {
        fActive = TRUE;
    }
    else if (m_bInPopupMenu || IN_MENU(m_pExtrasMenu) || m_bInSipMenu)
    {
        fActive = TRUE;
    }

    return fActive;
}

void CTaskBar::SetTaskBarZOrder(HWND hwndZorder)
{
    //default to TOPMOST
    if (hwndZorder == NULL)
    {
        hwndZorder = HWND_TOPMOST;
    }

    // We don't have to worry about the HWND_BOTTOM current case -- it's ok
    // to keep moving ourselves down to the bottom when there's a rude app.

    // Nor do we have to worry about the HWND_TOP current case -- it's ok
    // to keep moving ourselves up to the top when we're active.
    HWND hwndZorderCurrent = (GetWindowLong(m_hwndTaskBar, GWL_EXSTYLE) & WS_EX_TOPMOST) ? HWND_TOPMOST : HWND_NOTOPMOST;

    if (hwndZorder != hwndZorderCurrent)
    {
        // only do this if somehting has changed.
        BOOL bRet = SetWindowPos(m_hwndTaskBar,
                        hwndZorder,
                        0,
                        0,
                        0,
                        0,
                        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

        if (bRet && hwndZorder == HWND_TOPMOST)
        {
            // Confirm that HWND_TOPMOST Z-order got restored
            ASSERT(GetWindowLong(m_hwndTaskBar, GWL_EXSTYLE) & WS_EX_TOPMOST);
        }
    }
}

void CTaskBar::HandleFullScreenOrTopmostApp(TaskbarApplicationButton *pApp)
{
    if (pApp && pApp->m_bMarkedFullscreen)
    {
        m_hwndRudeFullScreen = pApp->m_hwndMain;
    }
    else
    {
        m_hwndRudeFullScreen = NULL;
    }

    // reset the Z-order
    HWND hwndZorder;

    if (m_bTaskBarOnTop && !m_hwndRudeFullScreen)
    {
        if (pApp && pApp->m_bMarkedTopmost)
        {
            hwndZorder = HWND_NOTOPMOST;
        }
        else
        {
            hwndZorder = HWND_TOPMOST;
        }
    }
    else if (IsActive())
    {
        hwndZorder = HWND_TOP;
    }
    else if (m_hwndRudeFullScreen)
    {
        hwndZorder = HWND_BOTTOM;
    }
    else
    {
        hwndZorder = HWND_NOTOPMOST;
    }

    SetTaskBarZOrder(hwndZorder);
}

void CTaskBar::SaveAutoHide(BOOL bAutoHide)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LONG lRet;
    HKEY hkey;
    DWORD dw;

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szShellAutoHide,
                        0, KEY_ALL_ACCESS, &hkey);
    if (lRet != ERROR_SUCCESS)
    {
        lRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szShellAutoHide,
                              0, NULL, 0, KEY_ALL_ACCESS,
                              NULL, &hkey, &dw);
    }

    if (lRet == ERROR_SUCCESS)
    {
        RegSetValueEx(hkey, L"", 0, REG_DWORD, (LPBYTE)&bAutoHide, sizeof(DWORD));
        RegCloseKey(hkey);
    }
}
/* SaveAutoHide() */


BOOL CTaskBar::GetAutoHide(BOOL *pbAutoHide)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    BOOL bRet = FALSE;
    HKEY hkey;
    LONG lRet;
    DWORD dw = sizeof(DWORD);
    DWORD dwType;

    // Open up the registry and mark this change
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szShellAutoHide, 0,0,&hkey))
    {
        lRet = RegQueryValueEx(hkey, L"", 0, &dwType,
                      (LPBYTE) pbAutoHide, &dw);
        RegCloseKey(hkey);
        bRet = (BOOL)(lRet == ERROR_SUCCESS);
    }

    return bRet;
}
/* GetAutoHide() */

void CTaskBar::SaveExpandControlPanel(BOOL bExpand)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LONG lRet;
    HKEY hkey;
    DWORD dw;

    // Open up the registry and mark this change
    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szExplorer,
                        0, KEY_ALL_ACCESS, &hkey);
    if (lRet != ERROR_SUCCESS)
    {
        lRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szExplorer,
                              0, NULL, 0, KEY_ALL_ACCESS,
                              NULL, &hkey, &dw);
    }

    if (lRet == ERROR_SUCCESS)
    {
        RegSetValueEx(hkey, c_szExpandControlPanel, 0, REG_DWORD, (LPBYTE)&bExpand, sizeof(DWORD));
        RegCloseKey(hkey);
    }
} /* HHTaskBar_SaveExpandControlPanel() */

BOOL CTaskBar::GetExpandControlPanel(BOOL *pbExpand)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    BOOL bRet = FALSE;
    HKEY hkey;
    LONG lRet;
    DWORD dw = sizeof(DWORD);
    DWORD dwType;

    // Open up the registry and retrieve the value
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szExplorer, 0,0,&hkey))
    {
        lRet = RegQueryValueEx(hkey, c_szExpandControlPanel, 0, &dwType, (LPBYTE)pbExpand, &dw);
        RegCloseKey(hkey);
        bRet = (BOOL)(lRet == ERROR_SUCCESS);
    }

    return bRet;
} /* HHTaskBar_GetExpandControlPanel() */


BOOL CTaskBar::GetSlide(BOOL *pbSlide)
{
    HKEY hkey;
    DWORD dw = sizeof(DWORD);
    DWORD dwType;

    *pbSlide = TRUE;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                    L"Software\\Microsoft\\Shell\\SlideTaskBar", 0,0,&hkey))
    {
        DEBUGMSG(ZONE_VERBOSE, (L"EXPLORER: Opened key ok\r\n"));
        DWORD dwRet = RegQueryValueEx(hkey, L"", 0, &dwType, (LPBYTE) pbSlide, &dw);
        DEBUGMSG(ZONE_VERBOSE, (L"EXPLORER: dwRet = %d, pbSlide = %d\r\n", dwRet, *pbSlide));
        RegCloseKey(hkey);
    }

    return TRUE;
}

BOOL CTaskBar::GetTameStartMenu(BOOL *pbTameStartMenu)
{
    HKEY hkey;
    DWORD dw = sizeof(DWORD);
    DWORD dwType;

    *pbTameStartMenu = FALSE;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                    L"Software\\Microsoft\\Shell\\TameStartMenu", 0,0,&hkey))
    {
        DEBUGMSG(ZONE_VERBOSE, (L"EXPLORER: Opened key ok\r\n"));
        DWORD dwRet = RegQueryValueEx(hkey, L"", 0, &dwType, (LPBYTE) pbTameStartMenu, &dw);
        DEBUGMSG(ZONE_VERBOSE, (L"EXPLORER: dwRet = %d, pbTameStartMenu = %d\r\n", dwRet, *pbTameStartMenu));
        RegCloseKey(hkey);
    }

    return TRUE;
}

void CTaskBar::SaveOnTop(BOOL bOnTop)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LONG lRet;
    HKEY hkey;
    DWORD dw;

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szShellOnTop,
                        0, KEY_ALL_ACCESS, &hkey);
    if (lRet != ERROR_SUCCESS)
    {
        lRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szShellOnTop,
                              0, NULL, 0, KEY_ALL_ACCESS,
                              NULL, &hkey, &dw);
    }

    if (lRet == ERROR_SUCCESS)
    {
        RegSetValueEx(hkey, L"", 0, REG_DWORD, (LPBYTE)&bOnTop, sizeof(DWORD));
        RegCloseKey(hkey);
    }

}
/* SaveOnTop() */


BOOL CTaskBar::GetOnTop(BOOL *pbOnTop)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    BOOL bRet = FALSE;
    HKEY hkey;
    LONG lRet;
    DWORD dw = sizeof(DWORD);
    DWORD dwType;

    // Open up the registry and mark this change
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szShellOnTop, 0,0,&hkey))
    {
        lRet = RegQueryValueEx(hkey, L"", 0, &dwType,
                      (LPBYTE) pbOnTop, &dw);
        RegCloseKey(hkey);
        bRet = (BOOL)(lRet == ERROR_SUCCESS);
    }

    return bRet;
}
/* GetOnTop() */

void CTaskBar::ShowClock(HWND hwnd, BOOL bShow)
/*---------------------------------------------------------------------------*\
*
\*---------------------------------------------------------------------------*/
{
    HKEY hkey;
    LONG lRet;

    // Open up the registry and mark this change
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szClock, 0,0,&hkey))
    {
        lRet = RegSetValueEx(hkey, c_szShowClock, 0, REG_DWORD,
                        (LPBYTE) &bShow, sizeof(bShow));
        RegCloseKey(hkey);
    }
    CheckShowClock();
} /* HHTaskBar_ShowClock()
  */


void CTaskBar::SetWorkArea(void)
/*---------------------------------------------------------------------------*\
*
\*---------------------------------------------------------------------------*/
{
    RECT rcWorkArea = {0}, rcWorkAreaNew = {0};

    SystemParametersInfo(SPI_GETWORKAREA, 0, (void*)&rcWorkArea, 0);

    // if the taskbar isn't on top, the workarea is the screen
    SetRect(&rcWorkAreaNew, 0, 0, GetSystemMetrics(SM_CXSCREEN),
               GetSystemMetrics(SM_CYSCREEN));
    if (m_bTaskBarOnTop)
    {
        if (m_bTaskBarAutoHide)
        {
            rcWorkAreaNew.bottom -= TASKBAR_HEIGHT_AUTOHIDE;
        }
        else
        {
            rcWorkAreaNew.bottom -= TASKBAR_HEIGHT;
        }
    }

    // currently, only the bottom will change
    if (rcWorkArea.bottom != rcWorkAreaNew.bottom)
    {
        SystemParametersInfo(SPI_SETWORKAREA, 0, (void*)&rcWorkAreaNew, SPIF_SENDCHANGE);
    }

    SIP_InitializeSipRect();
}
/* SetWorkArea() */

BOOL CALLBACK CTaskBar::s_ResizeCallBack(HWND hwnd, LPARAM lparam)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    POINT *ppt = (POINT*)lparam;
    DWORD dwStyle;
    RECT rc;

    if ((hwnd != Taskbar_GetWindow()) &&
        (hwnd != Desktop_GetWindow()))
    {
        dwStyle = GetWindowLong(hwnd, GWL_STYLE);
        GetWindowRect(hwnd, &rc);

        // resize all non popup windows that are full screen
        if (!(dwStyle & WS_POPUP) && ((rc.left == 0) && (rc.top == 0) &&
                (rc.bottom == ppt->y) && (rc.right == ppt->x)))
        {
            MoveWindow(hwnd, 0, 0, ppt[1].x, ppt[1].y, FALSE); // last param is ignored
            InvalidateRect(hwnd, NULL, TRUE);

            return TRUE;
        }
        else if (rc.bottom > ppt[1].y)
        {
            LONG y = rc.top - (rc.bottom - ppt[1].y);
            if (y < 0)
            {
                y = 0;
            }
            SetWindowPos(hwnd, 0, rc.left, y, 0, 0,
                         SWP_NOZORDER | SWP_NOSIZE);
        }
    }

    return TRUE;
}
/* ResizeCallBack */


void CTaskBar::ResizeAppWindows(BOOL bTaskBarVisible, BOOL bTaskBarOnTop)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    POINT pt[2]; // 0 - old size, 1 - new size
    RECT rc = {0};

    pt[1].x = GetSystemMetrics(SM_CXSCREEN);
    pt[1].y = GetSystemMetrics(SM_CYSCREEN);

    SystemParametersInfo(SPI_GETWORKAREA, 0, (void*)&rc, 0);

    pt[0].x = rc.right - rc.left;
    pt[0].y = rc.bottom - rc.top;

    if (bTaskBarOnTop)
    {
        if (bTaskBarVisible)
        {
            pt[1].y -= TASKBAR_HEIGHT;
        }
        else
        {
            pt[1].y -= TASKBAR_HEIGHT_AUTOHIDE;
        }
    }

    EnumWindows(s_ResizeCallBack, (LPARAM)pt);
} /* HHTaskBar_ResizeAppWindows()
   */


void CTaskBar::ApplyProperties(HWND hwnd, BOOL bGeneral)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    BOOL bExpandControlPanel = m_bExpandControlPanel;
    BOOL bAutoHideSave = m_bTaskBarAutoHide;
    BOOL bOnTopSave = m_bTaskBarOnTop;
    BOOL bShowClockSave = m_fShowClock;
    HWND hwndCtrl;

    if (bGeneral)
    {
        hwndCtrl = GetDlgItem(hwnd, IDC_TASKBAR_AUTOHIDE);
        m_bTaskBarAutoHide = (SendMessage(hwndCtrl, BM_GETCHECK, 0, 0) ==
                                   BST_CHECKED);

        hwndCtrl = GetDlgItem(hwnd, IDC_TASKBAR_ONTOP);
        m_bTaskBarOnTop = (SendMessage(hwndCtrl, BM_GETCHECK, 0, 0) ==
                           BST_CHECKED);

        hwndCtrl = GetDlgItem(hwnd, IDC_TASKBAR_SHOWCLOCK);
        m_fShowClock = (SendMessage(hwndCtrl, BM_GETCHECK, 0, 0) ==
                                BST_CHECKED);

        if (m_bTaskBarAutoHide != bAutoHideSave)
        {
            if (m_bTaskBarAutoHide)
            {
                Hide(m_hwndTaskBar);
            }
            else
            {
                // This is in a different thread than the taskbar, so an autohide
                // could currently be in progress.  If so we need to abort it.
                m_bTaskBarAbortHide = TRUE;
                Show(m_hwndTaskBar);
            }
            SaveAutoHide(m_bTaskBarAutoHide);
        }
        ResizeAppWindows(!m_bTaskBarAutoHide, m_bTaskBarOnTop);

        // Fix for (Taskbar: Does not hide on full screen apps after using auto hide)
        if (bOnTopSave != m_bTaskBarOnTop ||
            (m_bTaskBarAutoHide != bAutoHideSave && FALSE == m_bTaskBarOnTop))
        {
            SetWindowPos(m_hwndTaskBar,
                         m_bTaskBarOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                         0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            SaveOnTop(m_bTaskBarOnTop);
        }

        if (bShowClockSave != m_fShowClock)
        {
            ShowClock(m_hwndTaskBar, m_fShowClock);
        }

        // now set the workarea
        SetWorkArea();
    }
    else
    {
        hwndCtrl = GetDlgItem(hwnd, IDC_EXPAND);
        m_bExpandControlPanel = (SendMessage(hwndCtrl, BM_GETCHECK, 0, 0) ==
                                   BST_CHECKED);
        if (m_bExpandControlPanel != bExpandControlPanel)
        {
            SaveExpandControlPanel(m_bExpandControlPanel);
        }
    }
} /* HHTaskBar_ApplyProperties
   */


void CTaskBar::ReloadSettings()
{
    // Stash away the old settings.
    BOOL bTaskBarAutoHide = m_bTaskBarAutoHide;
    BOOL bTaskBarOnTop = m_bTaskBarOnTop;
    BOOL bTaskBarSlide = m_bTaskBarSlide;
    BOOL bTaskBarTameStartMenu = m_bTaskBarTameStartMenu;

    // Get the current settings
    GetAutoHide(&m_bTaskBarAutoHide);
    GetOnTop(&m_bTaskBarOnTop);
    GetSlide(&m_bTaskBarSlide);
    GetTameStartMenu(&m_bTaskBarTameStartMenu);

    // See if we need to apply the changes.
    // TODO: This is copied from ApplyProperties, we really only should have one copy.

    if (bTaskBarAutoHide != m_bTaskBarAutoHide)
    {
        if (m_bTaskBarAutoHide)
        {
            Hide(m_hwndTaskBar);
        }
        else
        {
            // This is in a different thread than the taskbar, so an autohide
            // could currently be in progress.  If so we need to abort it.
            m_bTaskBarAbortHide = TRUE;
            Show(m_hwndTaskBar);
        }
    }

    if (bTaskBarOnTop != m_bTaskBarOnTop)
    {
        SetWindowPos(m_hwndTaskBar,
                     m_bTaskBarOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                     0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    }

    CheckShowClock();

    if (bTaskBarAutoHide != m_bTaskBarAutoHide ||
        bTaskBarOnTop != m_bTaskBarOnTop)
    {
        ResizeAppWindows(!m_bTaskBarAutoHide, m_bTaskBarOnTop);
        SetWorkArea();
    }
}

DWORD CTaskBar::GetRecentDocsCount()
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LPSHELLFOLDER psfDesktop = NULL;
    LPITEMIDLIST pidlRecentDocs = NULL;
    LPSHELLFOLDER psfRecentDocs = NULL;
    LPENUMIDLIST peilRecentDocs = NULL;
    LPMALLOC pMalloc = NULL;
    DWORD dwFileCount = 0;
    LPITEMIDLIST pidlItem = NULL;
    ULONG celt = 0;

    if (FAILED(SHGetMalloc(&pMalloc)))
    {
        return 0;
    }

    if (SUCCEEDED(SHGetDesktopFolder(&psfDesktop)))
    {
        if (SUCCEEDED(SHGetSpecialFolderLocation(NULL,
                        CSIDL_RECENT,
                        &pidlRecentDocs)))
        {
            if (SUCCEEDED(psfDesktop->BindToObject(pidlRecentDocs,
                            NULL,
                            IID_IShellFolder,
                            (void**)&psfRecentDocs)))
         {
                if (SUCCEEDED(psfRecentDocs->EnumObjects(NULL,
                                SHCONTF_NONFOLDERS,
                                &peilRecentDocs)))
                {
                    while ((S_OK == peilRecentDocs->Next(1, &pidlItem, &celt)) && (0 != celt))
                    {
                        dwFileCount += celt;
                        pMalloc->Free(pidlItem);
                    }
                    peilRecentDocs->Release();
                }
                psfRecentDocs->Release();
            }
            pMalloc->Free(pidlRecentDocs);
        }
        psfDesktop->Release();
    }
    pMalloc->Release();

    return dwFileCount;
} /* GetRecentDocsCount()
   */


BOOL CTaskBar::PropertiesThread(HANDLE hEvent)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    __try
    {
        PROPSHEETPAGE psp[2];
        PROPSHEETHEADER psh;
        MSG msg;
        HWND hwndProperties;

        // setup the header
        psh.dwSize = sizeof(PROPSHEETHEADER);
        psh.dwFlags = PSH_PROPSHEETPAGE | PSH_MODELESS | PSH_USECALLBACK;
        psh.hwndParent = 0;
        psh.hInstance = g_hInstance;
        psh.pszIcon = NULL;
        psh.pszCaption = MAKEINTRESOURCE(IDS_TASKBAR_PROPERTIES);
        psh.nStartPage = 0;
        psh.ppsp = (LPCPROPSHEETPAGE) psp;
        psh.pfnCallback = s_PropSheetProc;
        psh.nPages = 2;

        // setup the page
        psp[0].dwSize = sizeof(PROPSHEETPAGE);
        psp[0].dwFlags = PSP_USETITLE;
        psp[0].hInstance = g_hInstance;
        psp[0].pszTemplate = MAKEINTRESOURCE(IDD_TASKBAR_OPTIONS);
        psp[0].pszIcon = NULL;
        psp[0].pfnDlgProc = s_PropertiesDlgProc;
        psp[0].pszTitle = MAKEINTRESOURCE(IDS_TASKBAR_OPTIONS);
        psp[0].lParam = (LPARAM) 0;
        psp[0].pfnCallback = NULL;

        // setup the page
        psp[1].dwSize = sizeof(PROPSHEETPAGE);
        psp[1].dwFlags = PSP_USETITLE;
        psp[1].hInstance = g_hInstance;
        psp[1].pszTemplate = MAKEINTRESOURCE(IDD_TASKBAR_DOCUMENTS);
        psp[1].pszIcon = NULL;
        psp[1].pfnDlgProc = s_DocumentMenuDlgProc;
        psp[1].pszTitle = MAKEINTRESOURCE(IDS_STARTMENU);
        psp[1].lParam = (LPARAM) 0;
        psp[1].pfnCallback = NULL;

        SetEvent(hEvent);
        hwndProperties = (HWND)PropertySheet(&psh);
        g_TaskBar->m_hwndTaskbarProperties = hwndProperties;

        if (-1 == (LONG)hwndProperties)
        {
            ASSERT(-1 != (LONG)hwndProperties);
            return FALSE;
        }

        LONG l = SetWindowLong(hwndProperties, DWL_DLGPROC,
                               (LONG) s_PropertiesSubclass);
        SetWindowLong(hwndProperties, GWL_USERDATA, l);

        while (GetMessage(&msg, NULL, 0, 0))
        {
            if (!PropSheet_IsDialogMessage(hwndProperties, &msg))
            {
                if (hwndProperties &&
                   !PropSheet_GetCurrentPageHwnd(hwndProperties))
                {
                    // destroy the propsheet
                    DestroyWindow(hwndProperties);
                    PostQuitMessage(0);
                    hwndProperties = NULL;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        SetEvent(hEvent);
    }

    g_TaskBar->m_hwndTaskbarProperties = 0;
    return TRUE;
}


void CTaskBar::Properties(HWND hwndTB)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    if (g_TaskBar->m_hwndTaskbarProperties)
    {
        SetForegroundWindow(g_TaskBar->m_hwndTaskbarProperties);
    }
    else
    {
        HANDLE hThread;
        HANDLE hEvent;
        DWORD dwThreadID;

        hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (!hEvent)
        {
            return;
        }

        hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PropertiesThread,
                               (LPVOID)hEvent, 0, &dwThreadID);
        if (!hThread)
        {
            CloseHandle(hEvent);
            return;
        }
        CloseHandle(hThread);

        WaitForSingleObject(hEvent, INFINITE);
        CloseHandle(hEvent);
    }
    return;

} /* HHTaskBar_Properties()
   */


LPTBOBJ  CTaskBar::GetItem(HWND hwndTB, int iItem)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LPTBOBJ ptbobj=NULL;

    switch  (iItem)
    {
        case (int)HHTB_START:
            return (LPTBOBJ)m_plistTaskBar->Head();

        case (int)HHTB_FIRSTNOTIFY:
            ptbobj = (LPTBOBJ)m_plistTaskBar->Head();

            while (ptbobj != NULL)
            {
                if (ptbobj->ptbi->m_uFlags & HHTBS_NOTIFY)
                {
                    return ptbobj;
                }
                ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
            }
            // If we don't find it, fall through and return the tray.

        case HHTB_TRAY:
        case HHTB_DESKTOP:
        case HHTB_SIP:
        case HHTB_EXTRA:
              ptbobj = (LPTBOBJ)m_plistTaskBar->Tail();
              if ((int)HHTB_EXTRA == iItem)
              {
                    return ptbobj;
              }
              ptbobj = (LPTBOBJ)m_plistTaskBar->Prev(&ptbobj->lpObj);
              if ((int)HHTB_SIP == iItem)
              {
                    return ptbobj;
              }
              ptbobj = (LPTBOBJ)m_plistTaskBar->Prev(&ptbobj->lpObj);
              if ((int)HHTB_DESKTOP == iItem)
              {
                    return ptbobj;
              }
              ptbobj = (LPTBOBJ)m_plistTaskBar->Prev(&ptbobj->lpObj);
              return ptbobj;

        case TRAY_SPINNER_DOWN:
        case TRAY_SPINNER_UP:
            return NULL;

        default:
            return (LPTBOBJ)m_plistTaskBar->GetAtIndex(iItem);
    }

    return NULL;

} /* HHTaskBar_GetItem()
   */



BOOL CTaskBar::CloseAllWindows(HWND hwndTB)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LPTBOBJ ptbobj, lptbStart, lptbTray;

    lptbStart = GetItem(hwndTB, (int)HHTB_START);
    lptbTray  = GetItem(hwndTB, (int)HHTB_FIRSTNOTIFY);

    // First, find the currently checked one.
    ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&lptbStart->lpObj);
    while (ptbobj && ptbobj != lptbTray)
    {
        if (IsWindow(ptbobj->ptbi->m_hwndMain))
        {
            SendMessage(ptbobj->ptbi->m_hwndMain, WM_CLOSE, 0, 0);
        }
        ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
    }

    return TRUE;

} /* HHTaskBar_CloseAllWindows()
   */


BOOL  CTaskBar::GetTrayRect(HWND hwndTB, LPRECT lprc)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LPTBOBJ lptbTray, lptbNotify;

    if (!lprc)
    {
        return FALSE;
    }

    lptbTray = GetItem(hwndTB, (int)HHTB_TRAY);
    lptbNotify = GetItem(hwndTB, (int)HHTB_FIRSTNOTIFY);

    CopyRect(lprc, &lptbTray->ptbi->m_rc);
    lprc->left = lptbNotify->ptbi->m_rc.left - (2*SEPERATOR);
    if (m_firstNotifyToDisplay > 0)
    {
        lprc->left  -= (TRAY_SPINNERWIDTH + SEPERATOR);
    }

    return TRUE;

} /* HHTaskBar_GetTrayRect()
   */


BOOL CTaskBar::IsClockVisible(HWND hwndTB)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    HKEY hkey;
    DWORD dw = sizeof(DWORD), dwType;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szClock,0,0, &hkey))
    {
        RegQueryValueEx(hkey, c_szShowClock, 0, &dwType,
                        (LPBYTE)&m_fShowClock, &dw);
        RegCloseKey(hkey);
    }
    return m_fShowClock;

} /* HHTaskBar_IsClockVisible()
   */

BOOL CTaskBar::CalcSizeClock(LPRECT lprc)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    WCHAR szBuf[64];
    HDC hdc;
    HFONT hfontOld;
    SIZE size;
    LPTBOBJ lptbobj = GetItem(m_hwndTaskBar, (int)HHTB_DESKTOP);

    lprc->right = lptbobj->ptbi->m_rc.left -1;
    lprc->left  = lprc->right - 4;

    if (m_fShowClock)
    {
        hdc = GetDC(m_hwndTaskBar);

        if (m_hfont)
        {
            hfontOld = (HFONT)SelectObject(hdc, m_hfont);
        }

        if (GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, NULL, NULL, szBuf, 64))
        {
            GetTextExtentExPoint(hdc, szBuf, lstrlen(szBuf), 1000, NULL, NULL, &size);

            // the +1 in small screen mode is for the border, 12 is a more generous spacing for screens that can afford it
            lprc->left = lprc->right - size.cx - (m_fIsSmallScreen ? (SEPERATOR*2 + 1) : 12);
        }
        else
        {
            ASSERT(FALSE);
        }

        if (m_hfont)
        {
            SelectObject(hdc, hfontOld);
        }

        ReleaseDC(m_hwndTaskBar, hdc);
    }
    return TRUE;

} /* HHTaskBar_CalcSizeClock()
   */


BOOL CTaskBar::RecalcButtons()
{
    // get the last item
    LPTBOBJ  lpobj =  NULL;
    int cx = GetSystemMetrics(SM_CXSCREEN);
    lpobj = GetItem(m_hwndTaskBar, (int)HHTB_EXTRA);

    if (!lpobj)
    {
        return FALSE;
    }

    lpobj->ptbi->m_rc.right = cx;
    if (m_fExtraAppsButton)
    {
        lpobj->ptbi->m_rc.left = cx - EXTRAS_BUTTON_WIDTH;
        InvalidateRect(m_hwndTaskBar, &lpobj->ptbi->m_rc, FALSE);
    }
    else
    {
        lpobj->ptbi->m_rc.left = lpobj->ptbi->m_rc.right;
    }

     cx = lpobj->ptbi->m_rc.left - 1;

     lpobj = (LPTBOBJ)m_plistTaskBar->Prev(&lpobj->lpObj);
     if (!lpobj)
     {
         return FALSE;
     }

     lpobj->ptbi->m_rc.right = cx;
     if (m_fShowSipButton)
     {
         lpobj->ptbi->m_rc.left = cx - SIP_BUTTON_WIDTH;
         InvalidateRect(m_hwndTaskBar, &lpobj->ptbi->m_rc, FALSE);
     }
     else
     {
          lpobj->ptbi->m_rc.left = lpobj->ptbi->m_rc.right;
     }

    cx = lpobj->ptbi->m_rc.left - 1;

    lpobj = (LPTBOBJ)m_plistTaskBar->Prev(&lpobj->lpObj);
    if (!lpobj)
    {
        return FALSE;
    }

     lpobj->ptbi->m_rc.right = cx;
     if (m_fShowDesktopButton)
     {
        lpobj->ptbi->m_rc.left = cx - DESKTOP_BUTTON_WIDTH;
        InvalidateRect(m_hwndTaskBar, &lpobj->ptbi->m_rc, FALSE);
     }
     else
     {
         lpobj->ptbi->m_rc.left = lpobj->ptbi->m_rc.right;
     }

     return TRUE;
}

BOOL CTaskBar::CheckShowClock()
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    TaskbarItem *pItem = NULL;

    RecalcButtons();

    pItem = GetItemInfo(m_hwndTaskBar, (UINT)HHTB_TRAY);
    if (pItem != NULL)
    {
        IsClockVisible(m_hwndTaskBar);
        CalcSizeClock(&pItem->m_rc);
        UpdateItem(m_hwndTaskBar, (UINT)HHTB_TRAY, pItem);
        RecalcItems(m_hwndTaskBar);

        UpdateWindow(m_hwndTray);
    }
    return TRUE;

} /* HHTaskBar_CheckShowClock()
   */


BOOL CTaskBar::RecalcShowClock()
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    TaskbarItem *pItem = NULL;

    RecalcButtons();

    pItem = GetItemInfo(m_hwndTaskBar, (UINT)HHTB_TRAY);
    if (pItem != NULL)
    {
        IsClockVisible(m_hwndTaskBar);
        CalcSizeClock(&pItem->m_rc);
        SetWindowPos(
                        m_hwndTray,
                        0,
                        pItem->m_rc.left,
                        pItem->m_rc.top,
                        pItem->m_rc.right - pItem->m_rc.left,
                        pItem->m_rc.bottom - pItem->m_rc.top,
                        SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE
);
        UpdateItem(m_hwndTaskBar, (UINT)HHTB_TRAY, pItem);
        RecalcItems(m_hwndTaskBar);

        UpdateWindow(m_hwndTray);
    }
    return TRUE;

}

void CTaskBar::WinIniChange(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (wParam == SETTINGCHANGE_START)
    {
        RECT rcScreen = {0};
        rcScreen.right = GetSystemMetrics(SM_CXSCREEN);
        rcScreen.bottom = GetSystemMetrics(SM_CYSCREEN);
        rcScreen.top = rcScreen.bottom - TASKBAR_HEIGHT;

        MoveWindow(Taskbar_GetWindow(),
                   rcScreen.left, rcScreen.top,
                   rcScreen.right - rcScreen.left,
                   rcScreen.bottom - rcScreen.top, FALSE);

        RecalcShowClock();
        SetWorkArea();
        ResizeAppWindows(!m_bTaskBarAutoHide, m_bTaskBarOnTop);

        UpdateWindow(Taskbar_GetWindow());
        if (m_bTaskBarAutoHide && m_bTaskBarHidden)
        {
            Hide(m_hwndTaskBar);
        }

        return;
    }

    if (wParam == SPI_SETSIPINFO)
    {
        UpdateSipButton();
    }

    if (wParam == SPI_SETWORKAREA)
    {
        SIP_UpdateSipPosition();
    }

    switch(lParam)
    {
        case INI_INTL:
        {
            HKL hklCurrent = InputLang_t::GetInputLocale();
            InputLang_t::InitializeInputLangList();
            UpdateInputLangIcon(hklCurrent);

            m_fLocaleChange = TRUE;
            break;
        }

        case 5000:
            ReloadSettings();
            break;

        default:
            break;
    }
}


void CTaskBar::Cleanup()
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    SHFILEINFO sfi;
    HIMAGELIST himl;

    KillTimer(m_hwndTaskBar, IDT_ADMIN);
    KillTimer(m_hwndTaskBar, IDT_AUTOHIDE);
    KillTimer(m_hwndTaskBar, IDT_AUTOUNHIDE);

    if (m_pTaskManDlg)
    {
        delete m_pTaskManDlg;
        m_pTaskManDlg = NULL;
    }

    if (m_hwndRunDlg)
    {
        DestroyWindow(m_hwndRunDlg);
        m_hwndRunDlg = NULL;
    }

    if (Desktop_GetWindow())
    {
        DestroyWindow(Desktop_GetWindow());
    }

    if (m_plistTaskBar)
    {
        delete m_plistTaskBar;
        m_plistTaskBar = NULL;
    }

    if (m_plistBubbles)
    {
        delete m_plistBubbles;
        m_plistBubbles = NULL;
    }

    if (m_hfont)
    {
        DeleteObject(m_hfont);
        m_hfont = NULL;
    }

    if (m_hiconDefault)
    {
        DestroyIcon(m_hiconDefault);
        m_hiconDefault = NULL;
    }

    if (g_hbrMonoDither)
    {
        DeleteObject(g_hbrMonoDither);
        g_hbrMonoDither = NULL;
    }

    if (m_pExtrasMenu)
    {
        delete m_pExtrasMenu;
        m_pExtrasMenu = NULL;
    }

    himl = (HIMAGELIST)SHGetFileInfo(L"", 0, &sfi, sizeof(SHFILEINFO),
                                     SHGFI_SYSICONINDEX | SHGFI_LARGEICON);
    ImageList_Destroy(himl);

    himl = (HIMAGELIST)SHGetFileInfo(L"", 0, &sfi, sizeof(SHFILEINFO),
                                     SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
    ImageList_Destroy(himl);
    //SetEvent(g_hTerminalEvent);

#ifdef TASKBAR_TABHOOK
    if (g_hdllAltTabHook)
    {
        FreeLibrary(g_hdllAltTabHook);
    }
    g_hdllAltTabHook = NULL;
    g_pfnAltTabHook = NULL;
#endif
} /* HHTaskbar_Cleanup()
   */


BOOL  CTaskBar::Register(HINSTANCE hInstance)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    WNDCLASS wc;
    BOOL fOk;

    if (!g_TaskBar)
    {
        g_TaskBar = this;
    }

    InitDitherBrush();

    wc.style = /*CS_HREDRAW | CS_VREDRAW | */CS_DBLCLKS;
    wc.lpfnWndProc = (WNDPROC)s_TaskBarWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = HHTASKBARDATA;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockBrush(LTGRAY_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = HHTASKBARCLASSNAME;

    // Register the window class and return success/failure code.
    fOk = RegisterClass(&wc);

    // Register the tray window class
    wc.lpfnWndProc = (WNDPROC)s_TrayWndProc;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)COLOR_MENU;
    wc.lpszClassName = HHTRAYCLASSNAME;
    fOk = RegisterClass(&wc);

    if (fOk)
    {
#if 0
        m_hiconDefault = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_APPLICATION),
                                          IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
#endif
        m_hiconDefault = NULL;  // Desktop does not provide default icons.

        fOk = SetupStandardButtons(hInstance);
    }

    if (fOk)
    {
        m_hwndTaskBar = Create(NULL);

        LPDROPTARGET pDropTarget = CTaskbarDropTarget_Create(m_hwndTaskBar, m_plistTaskBar);
        if (pDropTarget)
        {
           RegisterDragDrop(m_hwndTaskBar, pDropTarget);
           pDropTarget->Release();
        }

        // set shell hook
        WM_ShellHook = RegisterWindowMessage(L"SHELLHOOK");

        fOk = (m_hwndTaskBar != NULL);
    }

    return fOk;

} /* HHTaskBar_Register()
   */

BOOL CTaskBar::SetupStandardButtons(HINSTANCE hInstance)
{
    HDC hdc;
    HFONT hfontOld = NULL;
    SIZE size;

    if ((m_plistTaskBar != NULL) || (hInstance == NULL))
    {
        return FALSE;
    }

    m_plistTaskBar = new CList();

    if (!m_plistTaskBar)
    {
        ASSERT(m_plistTaskBar);
        return FALSE;
    }

    RECT rcButton = {0, TOPLINE, 52, BOTTOMLINE};
    RECT rcTray = {0, TOPLINE, 0, BOTTOMLINE};

    //Create the Start Button
    TaskbarItem *pItem = new TaskbarItem(NULL, HHTBS_BUTTON, rcButton, HHTB_START, L"");
    LPTBOBJ ptbobj = (LPTBOBJ) LocalAlloc(LMEM_FIXED, sizeof(TBOBJ));
    ASSERT(pItem);
    ASSERT(ptbobj);
    if (!pItem || !ptbobj)
    {
        return FALSE;
    }

    if (!m_fIsSmallScreen)
    {
        LoadString(hInstance, IDS_START, pItem->m_wszItem, CCHMAXTBLABEL);
        hdc = GetDC(NULL);
        if (m_hfont)
        {
            hfontOld = (HFONT)SelectObject(hdc, m_hfont);
        }

        if (GetTextExtentExPoint(hdc, pItem->m_wszItem, lstrlen(pItem->m_wszItem), 1000, NULL, NULL, &size)) {
            pItem->m_rc.right = pItem->m_rc.left + size.cx + 30;
        }

        if (m_hfont)
        {
            SelectObject(hdc, hfontOld);
        }

        ReleaseDC(NULL, hdc);
    }
    else
    {
        pItem->m_rc.right = pItem->m_rc.left + 24;
    }
    pItem->m_hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_FLAG), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    ptbobj->ptbi = pItem;
    m_plistTaskBar->AddTail(&ptbobj->lpObj);
    m_indexFirstApp = 1; // after the start menu

    //Create the Tray
    pItem = new TaskbarItem(NULL, HHTBS_STATUS, rcTray, HHTB_TRAY, L"");
    ptbobj = (LPTBOBJ) LocalAlloc(LMEM_FIXED, sizeof(TBOBJ));
    ASSERT(pItem);
    ASSERT(ptbobj);
    if (!pItem || !ptbobj)
    {
        return FALSE;
    }
    ptbobj->ptbi = pItem;
    m_plistTaskBar->AddTail(&ptbobj->lpObj);

    //Create the Desktop Button
    pItem = new TaskbarItem(NULL, HHTBS_BUTTON, rcButton, HHTB_DESKTOP, L"");
    ptbobj = (LPTBOBJ) LocalAlloc(LMEM_FIXED, sizeof(TBOBJ));
    ASSERT(pItem);
    ASSERT(ptbobj);
    if (!pItem || !ptbobj)
    {
        return FALSE;
    }
    pItem->m_hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_DESKTOP), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    ptbobj->ptbi = pItem;
    m_plistTaskBar->AddTail(&ptbobj->lpObj);

    //Create the SIP Button
    pItem = new TaskbarItem(NULL, HHTBS_BUTTON, rcButton, HHTB_SIP, L"");
    ptbobj = (LPTBOBJ) LocalAlloc(LMEM_FIXED, sizeof(TBOBJ));
    ASSERT(pItem);
    ASSERT(ptbobj);
    if (!pItem || !ptbobj)
    {
        return FALSE;
    }
    pItem->m_hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_SIP), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    m_hiSipUpIcon = pItem->m_hIcon;
    m_hiSipDownIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_SIP2), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    m_hiSipCurrentIcon = m_hiSipDownIcon;
    ptbobj->ptbi = pItem;
    m_plistTaskBar->AddTail(&ptbobj->lpObj);

    //Create the extra apps button
    if (m_pExtrasMenu)
    {
        ASSERT(!m_pExtrasMenu);
        delete m_pExtrasMenu;
        m_pExtrasMenu = NULL;
    }
    m_pExtrasMenu = new ExtrasMenuButton(rcButton);
    ptbobj = (LPTBOBJ) LocalAlloc(LMEM_FIXED, sizeof(TBOBJ));
    ASSERT(m_pExtrasMenu);
    ASSERT(ptbobj);
    if (!m_pExtrasMenu || !ptbobj)
    {
        return FALSE;
    }
    ptbobj->ptbi = m_pExtrasMenu;
    m_plistTaskBar->AddTail(&ptbobj->lpObj);

    return TRUE;
}


HWND  CTaskBar::Create(HWND hwndParent)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    DWORD dwStyle = WS_POPUP;
    POINT pt;
    RECT rc;
    TOOLINFO ti = {0};
    LPTBOBJ ptbobj;

    // Figure out if the device has a mouse.  This fails if there is none.
    m_bHasMouse = GetCursorPos(&pt);
    DEBUGMSG(ZONE_VERBOSE, (L"\r\nExplorer: HasMouse = %s\r\n", m_bHasMouse ? L"TRUE" : L"FALSE"));

    if (hwndParent != NULL)
    {
        dwStyle |= WS_CHILD;
        GetClientRect(hwndParent, &rc);
    }
    else
    {
        SetRect(&rc, 0, 0, GetSystemMetrics(SM_CXSCREEN),
                GetSystemMetrics(SM_CYSCREEN));
    }

    m_plistBubbles = new CList();

    if (!m_plistBubbles)
    {
        ASSERT(m_plistBubbles);
        return NULL;
    }

    DWORD dwExStyle = 0;
    LANGID userLangID = GetUserDefaultUILanguage();

    if (LANG_ARABIC == PRIMARYLANGID(userLangID) ||
        LANG_FARSI == PRIMARYLANGID(userLangID) ||
        LANG_HEBREW == PRIMARYLANGID(userLangID) ||
        LANG_URDU == PRIMARYLANGID(userLangID))
    {
        dwExStyle |= WS_EX_LAYOUTRTL;
    }

    rc.top = rc.bottom - TASKBAR_HEIGHT;
    m_hwndTaskBar = CreateWindowEx(dwExStyle,HHTASKBARCLASSNAME, L"", dwStyle, rc.left, rc.top,
                          rc.right-rc.left, rc.bottom-rc.top, hwndParent,
                          NULL, g_hInstance, NULL);


    GetExistingApplications(m_hwndTaskBar);

    dwStyle = WS_CHILD | WS_VISIBLE;
    GetTrayRect(m_hwndTaskBar, &rc);
    m_hwndTray = CreateWindowEx(WS_EX_NOACTIVATE,
                                HHTRAYCLASSNAME,
                                L"",
                                dwStyle,
                                rc.left,
                                rc.top,
                                rc.right-rc.left,
                                rc.bottom-rc.top,
                                m_hwndTaskBar,
                                NULL, g_hInstance, NULL);

    CheckShowClock();

    // Move these into a separate function?

    // Add a tooltip to the taskbar
    m_hwndTaskBarTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
                                     WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     m_hwndTaskBar, NULL, g_hInstance, NULL);

    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_SUBCLASS;
    ti.lpszText = LPSTR_TEXTCALLBACK;
    ti.hwnd = m_hwndTaskBar;
    // This tool handles the Start button
    ptbobj = GetItem(m_hwndTaskBar, (int) HHTB_START);
    if (ptbobj)
    {
        ti.uId = (WPARAM) ptbobj;
        ti.rect = ptbobj->ptbi->m_rc;
        SendMessage(m_hwndTaskBarTT, TTM_ADDTOOL, NULL, (LPARAM) &ti);
    }
    // This tool handles the Desktop button
    ptbobj = GetItem(m_hwndTaskBar, (int) HHTB_DESKTOP);
    if (ptbobj)
    {
        ti.uId = (WPARAM) ptbobj;
        ti.rect = ptbobj->ptbi->m_rc;
        SendMessage(m_hwndTaskBarTT, TTM_ADDTOOL, NULL, (LPARAM) &ti);
    }

    if (m_fShowSipButton)
    {
        // This tool handles the sip button
        ptbobj = GetItem(m_hwndTaskBar, (int) HHTB_SIP);
        if (ptbobj)
        {
            ti.uId = (WPARAM) ptbobj;
            ti.rect = ptbobj->ptbi->m_rc;
            SendMessage(m_hwndTaskBarTT, TTM_ADDTOOL, NULL, (LPARAM) &ti);
        }

        // call SipRegisterNotification
        SIP_SipRegisterNotification(m_hwndTaskBar);
    }

    if (m_fExtraAppsButton)
    {
        // this tool handles the apps button
        ptbobj = GetItem(m_hwndTaskBar, (int) HHTB_EXTRA);
        if (ptbobj)
        {
            ti.uId = (WPARAM) ptbobj;
            ti.rect = ptbobj->ptbi->m_rc;
            SendMessage(m_hwndTaskBarTT, TTM_ADDTOOL, NULL, (LPARAM) &ti);
        }
    }

    // This tool handles the clock (but uses the tray ptbobj)
    ptbobj = GetItem(m_hwndTaskBar, (int) HHTB_TRAY);
    if (ptbobj)
    {
        ti.hwnd = m_hwndTray;
        ti.uId = (WPARAM) ptbobj;
        CalcSizeClock(&(ti.rect));
        MapWindowPoints(m_hwndTaskBar, m_hwndTray, (LPPOINT) &(ti.rect), 2);
        SendMessage(m_hwndTaskBarTT, TTM_ADDTOOL, NULL, (LPARAM) &ti);
    }

    if (m_pPowerUI)
    {
        m_pPowerUI->CreatePowerTrayIcon(m_hwndTaskBar, HHTBI_POWER);
    }

    // Use default keyboard layout as the active keyboard
    HKL hklDefault;
    ::SystemParametersInfo(SPI_GETDEFAULTINPUTLANG, 0, (void*)&hklDefault, 0);
    UpdateInputLangIcon(hklDefault);

    if (m_hwndTaskBar)
    {
#ifdef TASKBAR_TABHOOK
        WCHAR szDll[MAX_PATH];

        DWORD lRet, dwType, dwLen;
        dwLen = MAX_PATH;
        szDll[0] = L'\0';

        lRet = RegQueryValueEx(HKEY_LOCAL_MACHINE, L"AltTabHook",
                               (LPDWORD) L"Explorer", &dwType, (LPBYTE) szDll, &dwLen);
        if (lRet == ERROR_SUCCESS && dwType == REG_SZ && szDll[0] != L'\0')
        {
            g_hdllAltTabHook = LoadLibrary(szDll);
            if (g_hdllAltTabHook != NULL)
            {
                DEBUGMSG(ZONE_VERBOSE, (L"TASKBAR: AltTabHook loaded!\n"));
                g_pfnAltTabHook =
                    (PFNALTTAB) GetProcAddress(g_hdllAltTabHook, L"AltTabHook");
            }
        }
#endif
        // It is now going off every 5 seconds !!!

        SetAdminTimer();
        TimerAdmin(m_hwndTaskBar, WM_TIMER, 0, 0);

        m_bTaskBarOnTop = TRUE;
        GetExpandControlPanel(&m_bExpandControlPanel);
        GetAutoHide(&m_bTaskBarAutoHide);
        GetOnTop(&m_bTaskBarOnTop);
        GetSlide(&m_bTaskBarSlide);
        GetTameStartMenu(&m_bTaskBarTameStartMenu);

        SetWorkArea();
        SetWindowPos(m_hwndTaskBar,
                     m_bTaskBarOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                     0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        ResizeAppWindows(!m_bTaskBarAutoHide, m_bTaskBarOnTop);
        if (m_bTaskBarAutoHide)
        {
            m_bTaskBarHidden = FALSE; // force hide
            Hide(m_hwndTaskBar);
        }
        else
        {
            m_bTaskBarHidden = TRUE; // force show
            Show(m_hwndTaskBar);
        }
        InvalidateRect(m_hwndTaskBar, 0, TRUE);

        ShowWindow(m_hwndTaskBar, SW_SHOWNORMAL);
        UpdateWindow(m_hwndTaskBar);
    }

    m_pBubbleMgr = new CHtmlBubble;
    if (m_pBubbleMgr)
    {
        m_pBubbleMgr->Init(m_hwndTaskBar);
    }

    if (m_pTaskManDlg)
    {
        m_pTaskManDlg->Create();
    }

    // SetFocus to the Taskbar since TaskManDlg got it on creation
    SetFocus(m_hwndTaskBar);

    return m_hwndTaskBar;

} /* Create()
   */


BOOL  CTaskBar::TimerAdmin(HWND hwndTB, UINT msg, WPARAM wParam, LPARAM lParam)
/*---------------------------------------------------------------------------*\
 * This timer goes off every 30 seconds.  The notification code depends
 * on this interval when it sets iNotificationSoundDelayCount; so if
 * you change the interval you'll want to change NUTILS.C too.
\*---------------------------------------------------------------------------*/
{
#define DELAY_INTERVAL 300000    // 5 minute, time in ms

    static WORD wHour=(WORD)-1, wMinute=(WORD)-1;
    static BOOL fCleanUpHibernate;
    static DWORD cbHibernateBelow;

    SYSTEMTIME st;
    TaskbarItem *pItem = NULL;
    BOOL fDone=FALSE, fLowMemory=FALSE;
    HWND hwndApp;
    LPTBOBJ ptbobj;
    MEMORYSTATUS ms;
    RECT rcOld;

    if (0 == cbHibernateBelow)
    {
         SYSTEM_INFO sysinfo;
         GetSystemInfo(&sysinfo);
         cbHibernateBelow = max (HIBERNATE_BELOW,
                                 HIBERNATE_BELOW_PAGES * sysinfo.dwPageSize);
    }

#if defined(USE_ALLOC_STAT)
    WalkUnfreedMemory(L"TimerAdmin");
#endif

    ms.dwLength = sizeof(MEMORYSTATUS);
    GlobalMemoryStatus(&ms);
    if (ms.dwAvailPhys < cbHibernateBelow)
    {
        if (!fCleanUpHibernate)
        {
            fCleanUpHibernate = TRUE;
        }

#ifdef MONITOR_HIBERNATE
        DEBUGMSG(ZONE_VERBOSE, (L"Determining who to post WM_HIBERNATE\r\n"));
#endif

        hwndApp = ::GetWindow(hwndTB, GW_HWNDLAST);
        do
        {
            ptbobj = FindItem(hwndApp);
            // ASSERT (ptbobj); // This window doesn't have to be on the taskbar

            if (ptbobj && !((TaskbarApplicationButton *) ptbobj->ptbi)->m_bHibernate)
            {
                fDone = TRUE;
            }

            hwndApp = ::GetWindow(hwndApp, GW_HWNDPREV);

        } while (hwndApp && !fDone);

        if (fDone)
        {
#ifdef MONITOR_HIBERNATE
            LPWSTR sz;

            if (sz = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*MAX_PATH))
            {
                GetWindowTextWDirect(ptbobj->ptbi->m_hwndMain, sz, MAX_PATH);
                DEBUGMSG(ZONE_VERBOSE, (L"Posting WM_HIBERNATE to 0x%.8x '%s'\r\n",
                              ptbobj->ptbi->m_hwndMain, sz));
                LocalFree(sz);
            }
#endif
            ((TaskbarApplicationButton *) ptbobj->ptbi)->m_bHibernate = true;
            PostMessage(ptbobj->ptbi->m_hwndMain, WM_HIBERNATE, 0, 0);
        }
#ifdef MONITOR_HIBERNATE
        else
        {
            DEBUGMSG(ZONE_VERBOSE, (L"Not posting WM_HIBERNATE.\r\n"));
        }
#endif

    }
    else
    {
        if (fCleanUpHibernate)
        {
            LPTBOBJ lptbStart, lptbTray;

            fCleanUpHibernate = FALSE;
            lptbStart = GetItem(hwndTB, (int)HHTB_START);
            lptbTray  = GetItem(hwndTB, (int)HHTB_FIRSTNOTIFY);

            // First, find the currently checked one.
            ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&lptbStart->lpObj);
            while (ptbobj != lptbTray)
            {
                ((TaskbarApplicationButton *) ptbobj->ptbi)->m_bHibernate = false;
                ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
            }
        }
    }

    if (m_fShowClock)
    {
        GetLocalTime(&st);

        if ((st.wHour != wHour) || (st.wMinute != wMinute) || m_fLocaleChange)
        {
            wHour = st.wHour;
            wMinute = st.wMinute;

            pItem = GetItemInfo(hwndTB, (UINT)HHTB_TRAY);
            GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL,
                          pItem->m_wszItem, CCHMAXTBLABEL);

            if (m_fLocaleChange)
            {
                IsClockVisible(hwndTB);
            }

            rcOld = pItem->m_rc;
            CalcSizeClock(&pItem->m_rc);
            UpdateItem(hwndTB, (UINT)HHTB_TRAY, pItem);

            if (m_fLocaleChange || !EqualRect(&rcOld, &pItem->m_rc))
            {
                RecalcItems(hwndTB);
                UpdateWindow(m_hwndTray);
            }
            m_fLocaleChange = FALSE;
        }
    }

    return 0;
} /* TimerAdmin()
   */

// Returns the index at which the item is inserted, or -1 otherwise
//
int CTaskBar::InsertItem(HWND hwndApp)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    int insertIndex = -1;
    LPTBOBJ ptbobj;
    TaskbarApplicationButton *pAppItem = NULL;

    if (!IsWindow(hwndApp) ||
        (GetWindowLong(hwndApp, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) ||
        (hwndApp == this->m_hwndTaskBar) ||
        (hwndApp == this->m_hwndTaskBarTT) ||
        (hwndApp == this->m_hwndTray))
    {
        goto leave;
    }

    ptbobj = (LPTBOBJ) LocalAlloc(LPTR, sizeof(TBOBJ));
    if (!ptbobj)
    {
        goto leave;
    }
    pAppItem = new TaskbarApplicationButton(hwndApp);
    if (!pAppItem)
    {
        LocalFree(ptbobj);
        goto leave;
    }
    ptbobj->ptbi = pAppItem;

    if (!ptbobj->ptbi->m_hIcon || ptbobj->ptbi->m_hIcon == (HICON)E_FAIL)
    {
        ptbobj->ptbi->m_hIcon = m_hiconDefault;
    }

    SetRect(&ptbobj->ptbi->m_rc, 0, 0, 0, 0);

    if (m_fExtraAppsButton && m_currentNumberOfApps >= m_maxAppsToDisplay)
    {
        insertIndex = m_indexFirstApp;
    }
    else
    {
        insertIndex = m_plistTaskBar->GetObjectIndex(&(GetItem(m_hwndTaskBar, (int)HHTB_FIRSTNOTIFY)->lpObj));
    }

    if (m_plistTaskBar->InsertBeforeIndex(&ptbobj->lpObj, insertIndex))
    {
        // Add a tooltip for this new item
        TOOLINFO ti = {0};
        ti.cbSize = sizeof(TOOLINFO);
        ti.uFlags = TTF_SUBCLASS;
        ti.hwnd = m_hwndTaskBar;
        ti.lpszText = LPSTR_TEXTCALLBACK;
        ti.uId = (WPARAM) ptbobj;
        SendMessage(m_hwndTaskBarTT, TTM_ADDTOOL, NULL, (LPARAM) &ti);

        m_currentNumberOfApps++;
        RecalcItems(m_hwndTaskBar);
    }
    else
    {
        // Failed to insert in TaskbarList
        if (ptbobj->ptbi)
        {
            delete ptbobj->ptbi;
        }
        insertIndex = -1;
        LocalFree(ptbobj);
    }

leave:
    return insertIndex;
} /* InsertItem()
   */

BOOL CTaskBar::DeleteItem(HWND hwnd, int iItem, BOOL bFree)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    DEBUGMSG(ZONE_VERBOSE, (L"CTaskBar::DeleteItem\r\n"));
    if (iItem <= 0 ||
        m_plistTaskBar->GetAtIndex(iItem) == m_plistTaskBar->Head())
    {
        return FALSE;
    }

    LPTBOBJ ptbobj = (LPTBOBJ)m_plistTaskBar->DisconnectAtIndex(iItem);
    if (!(ptbobj->ptbi->m_uFlags & HHTBS_NOTIFY) && (int)ptbobj->ptbi->m_hwndMain > 0)
    {
        m_currentNumberOfApps--;
    }
    else if (ptbobj->ptbi->m_uFlags & HHTBS_NOTIFY)
    {
        m_currentNumberOfNotifys--;
        if (m_firstNotifyToDisplay >=  m_currentNumberOfNotifys)
        {
            m_firstNotifyToDisplay = m_currentNumberOfNotifys - m_maxNotifysToDisplay;
            if (m_firstNotifyToDisplay < 0)
            {
                m_firstNotifyToDisplay = 0;
            }
        }

    }
    ASSERT(m_currentNumberOfApps >= 0);
    ASSERT(m_currentNumberOfNotifys >= 0);

    if (ptbobj != NULL)
    {
        if ((ptbobj->ptbi->m_uFlags & HHTBF_DESTROYICON) && ptbobj->ptbi->m_hIcon && bFree)
        {

            //if (ptbobj->ptbi->m_hIcon == (HICON)E_FAIL) DebugBreak();
            DestroyIcon(ptbobj->ptbi->m_hIcon);
        }

        // Remove the tooltip with the item
        TOOLINFO ti = {0};
        ti.cbSize = sizeof(TOOLINFO);
        ti.hwnd = ((ptbobj->ptbi->m_uFlags & HHTBS_NOTIFY) ? m_hwndTray : m_hwndTaskBar);
        ti.uId = (WPARAM) ptbobj;
        SendMessage(m_hwndTaskBarTT, TTM_DELTOOL, NULL, (LPARAM) &ti);

        RecalcItems(hwnd);
        if (bFree)
        {
            delete ptbobj->ptbi;
            LocalFree(ptbobj);
        }
    }

    return TRUE;
} /* DeleteItem()
   */

HWND CTaskBar::FindCheckedItem(HWND hwndTB)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LPTBOBJ ptbobj, lptbStart, lptbTray;

    lptbStart = GetItem(hwndTB, (int)HHTB_START);
    lptbTray  = GetItem(hwndTB, (int)HHTB_FIRSTNOTIFY);

    // First, find the currently checked one.
    ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&lptbStart->lpObj);
    while (ptbobj && ptbobj != lptbTray)
    {
        if (ptbobj->ptbi->m_uFlags & HHTBS_CHECKED)
        {
            return ptbobj->ptbi->m_hwndMain;
        }
        ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
    }

    // NOTE: Nothing is checked so I should return the desktop HWND
    return Desktop_GetWindow();
} /* FindCheckedItem()
   */



int CTaskBar::FindItemIndex(HWND hwndTB, HWND hwndItem)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LPTBOBJ ptbobj = (LPTBOBJ)m_plistTaskBar->Head();

    while (ptbobj != NULL)
    {
        if (hwndItem == ptbobj->ptbi->m_hwndMain &&
            !(ptbobj->ptbi->m_uFlags & HHTBS_NOTIFY))
        {
            return m_plistTaskBar->GetObjectIndex(&ptbobj->lpObj);
        }
        ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
    }

    return -1;
} /* FindItemIndex()
   */


UINT CTaskBar::FindNotify(HWND hwndTB, PNOTIFYICONDATA pNID)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    if (!m_plistTaskBar)
    {
        return (UINT)-1;
    }

    LPTBOBJ ptbobj = (LPTBOBJ)m_plistTaskBar->Head();

    while (ptbobj != NULL)
    {
        if ((ptbobj->ptbi->m_uFlags & HHTBS_NOTIFY) && !(ptbobj->ptbi->m_uFlags & HHTBS_BUBBLE_NOTIFY))
        {
            TaskbarNotification *pNotify = (TaskbarNotification *) ptbobj->ptbi;

            if (pNID->hWnd == pNotify->m_hwndMain && pNID->uID == pNotify->m_uID)
            {
                return (UINT)m_plistTaskBar->GetObjectIndex(&ptbobj->lpObj);
            }
        }
        ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
    }

    return (UINT)-1;
} /* FindNofify()
   */


BOOL CTaskBar::NotifyTagDestroyIcon(HWND hwndTB, PNOTIFYICONDATA pNID)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    if (!m_plistTaskBar)
    {
        return FALSE;
    }

    LPTBOBJ ptbobj = (LPTBOBJ)m_plistTaskBar->Head();

    while (ptbobj != NULL)
    {
        if ((ptbobj->ptbi->m_uFlags & HHTBS_NOTIFY) && !(ptbobj->ptbi->m_uFlags & HHTBS_BUBBLE_NOTIFY))
        {
            TaskbarNotification *pNotify = (TaskbarNotification *) ptbobj->ptbi;

            if (pNID->hWnd == pNotify->m_hwndMain && pNID->uID == pNotify->m_uID)
            {
                ptbobj->ptbi->m_uFlags |= HHTBF_DESTROYICON;
                return TRUE;
            }
        }
        ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
    }

    return FALSE;
} /* NotifyTagDestroyIcon()
   */


BOOL CTaskBar::ModifyNotify(HWND hwndTB, PNOTIFYICONDATA pnid, int iItem)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LPTBOBJ ptbobj;

    ptbobj = (LPTBOBJ) m_plistTaskBar->GetAtIndex(iItem);

    if (NULL == ptbobj)
    {
        ASSERT(ptbobj);
        return FALSE;
    }

    if (!(ptbobj->ptbi->m_uFlags & HHTBS_NOTIFY))
    {
        return FALSE;
    }

    TaskbarNotification *pNotify = (TaskbarNotification *) ptbobj->ptbi;
    BOOL updated = pNotify->Update(pnid);
    if (updated & (pnid->uFlags & NIF_ICON && pnid->hIcon))
    {
        InvalidateRect(m_hwndTray, NULL, TRUE);
        UpdateWindow(m_hwndTray);
    }

    return updated;
} /* ModifyNofify()
   */

BOOL CTaskBar::InsertNotify(HWND hwndTB, PNOTIFYICONDATA pnid)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    DEBUGMSG(ZONE_VERBOSE, (L"CTaskBar::InsertNotify\r\n"));
    LPTBOBJ ptbobj;
    TaskbarNotification *pNotify = new TaskbarNotification(pnid);
    RECT rc;
    ASSERT(pNotify);
    if (!pNotify)
    {
        return FALSE;
    }

    rc.top = 7;
    rc.bottom = 23;
    rc.right = GetItem(hwndTB, (int)HHTB_FIRSTNOTIFY)->ptbi->m_rc.left - SEPERATOR;
    rc.left = rc.right - 16;
    pNotify->m_rc = rc;

    if (ptbobj = (LPTBOBJ) LocalAlloc(LMEM_FIXED, sizeof(TBOBJ)))
    {
        ptbobj->ptbi = pNotify;
        if (m_plistTaskBar->InsertBeforeIndex(&ptbobj->lpObj,
                                              (UINT)m_plistTaskBar->GetObjectIndex(&(GetItem(hwndTB, (int)HHTB_FIRSTNOTIFY)->lpObj))))
        {
            m_currentNumberOfNotifys++;

            // make sure this new notify is displayed
            if (m_firstNotifyToDisplay > 0)
            {
                m_firstNotifyToDisplay = 0;
            }

            // Add a tooltip for this new item
            TOOLINFO ti = {0};
            ti.cbSize = sizeof(TOOLINFO);
            ti.uFlags = TTF_SUBCLASS;
            ti.hwnd = m_hwndTray;
            ti.lpszText = LPSTR_TEXTCALLBACK;
            ti.uId = (WPARAM) ptbobj;
            SendMessage(m_hwndTaskBarTT, TTM_ADDTOOL, NULL, (LPARAM) &ti);

            RecalcItems(hwndTB);
            UpdateWindow(m_hwndTray);
            return TRUE;
        }

        delete pNotify;
        LocalFree(ptbobj);
    }
    else
    {
        delete pNotify;
    }

    return FALSE;
} /* InsertNotify()
   */

BOOL CTaskBar::UpdateInputLangIcon(HKL hklIdentifier)
{
    BOOL bRet = FALSE;
    BOOL bShowInputLang;
    RECT rc;
    InputLangNotification *pNotify = NULL;

    if (!m_plistTaskBar ||
        !InputLang_t::UpdateInputLocale(hklIdentifier))
    {
        goto leave;
    }

    bShowInputLang = InputLang_t::ShowIcon();

    LPTBOBJ ptbobj = (LPTBOBJ)m_plistTaskBar->Head();
    while (ptbobj)
    {
        if (ptbobj->ptbi->m_uFlags & HHTBS_INPUTLANG_NOTIFY)
        {
            pNotify = (InputLangNotification *) &ptbobj->ptbi;
            break;
        }
        ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
    }

    if (!ptbobj)
    {
        // Insert Taskbar icon
        if (!bShowInputLang)
        {
            goto leave;
        }

        ptbobj = (LPTBOBJ) LocalAlloc(LMEM_FIXED, sizeof(TBOBJ));
        if (!ptbobj)
        {
            goto leave;
        }

        rc.top = TOPLINE + 4;;
        rc.bottom = BOTTOMLINE + 4;
        rc.right = GetItem(m_hwndTaskBar, (int)HHTB_FIRSTNOTIFY)->ptbi->m_rc.left - SEPERATOR;
        rc.left = rc.right;
        pNotify = new InputLangNotification(rc, m_hwndTray);
        if (!pNotify)
        {
            LocalFree(ptbobj);
            goto leave;
        }
        ptbobj->ptbi = pNotify;

        if (!m_plistTaskBar->InsertBeforeIndex(&ptbobj->lpObj,
                        (UINT)m_plistTaskBar->GetObjectIndex(&(GetItem(m_hwndTaskBar, (int)HHTB_FIRSTNOTIFY)->lpObj))))
        {
            delete ptbobj->ptbi;
            LocalFree(ptbobj);
            goto leave;
        }

        m_currentNumberOfNotifys++;

        // make sure this new notify is displayed
        if (m_firstNotifyToDisplay > 0)
        {
            m_firstNotifyToDisplay = 0;
        }

        // Add a tooltip for this new item
        TOOLINFO ti = {0};
        ti.cbSize = sizeof(TOOLINFO);
        ti.uFlags = TTF_SUBCLASS;
        ti.hwnd = m_hwndTray;
        ti.lpszText = LPSTR_TEXTCALLBACK;
        ti.uId = (WPARAM) ptbobj;
        SendMessage(m_hwndTaskBarTT, TTM_ADDTOOL, NULL, (LPARAM) &ti);
    }
    else
    {
        if (!bShowInputLang &&
            m_plistTaskBar->Disconnect(&ptbobj->lpObj))
        {
            delete ptbobj->ptbi;
            LocalFree(ptbobj);
            RecalcItems(m_hwndTaskBar);
            UpdateWindow(m_hwndTray);
            goto leave;
        }

        rc = ptbobj->ptbi->m_rc;
        MapWindowPoints(m_hwndTaskBar, m_hwndTray, (LPPOINT)&rc, 2);
        InvalidateRect(m_hwndTray, &rc, FALSE);
    }

    if (FAILED(StringCbCopy(ptbobj->ptbi->m_wszItem,
                            sizeof(ptbobj->ptbi->m_wszItem),
                            InputLang_t::GetInputLocaleDescription())))
    {
        ptbobj->ptbi->m_wszItem[0] = L'\0';
    }
    else
    {
        LPWSTR pwszSeparator;
        pwszSeparator = ::wcschr(ptbobj->ptbi->m_wszItem, L'$');
        if (pwszSeparator)
        {
            *pwszSeparator = L'\0';
        }
    }

    ((InputLangNotification *) ptbobj->ptbi)->CalcSizeInputLang(m_hwndTaskBar);

    RecalcItems(m_hwndTaskBar);
    UpdateWindow(m_hwndTray);

    bRet = TRUE;

leave:
    return bRet;
}

BOOL CTaskBar::RecalcItems(HWND hwndTB)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    DEBUGMSG(ZONE_VERBOSE, (L"CTaskBar::RecalcItems\r\n"));

    LPTBOBJ lptbStart, lptbTray, lptbFirstNotify, ptbobj;
    int fLeftO = 1, nLeftO, nWidth, nOffset=0;
    int nItems = 0;
    RECT rc;
    TOOLINFO ti = {0};

    lptbStart = GetItem(hwndTB, (int)HHTB_START);
    lptbTray  = GetItem(hwndTB, (int)HHTB_TRAY);

    lptbFirstNotify = GetItem(hwndTB, (int)HHTB_FIRSTNOTIFY);
    ptbobj = (LPTBOBJ) m_plistTaskBar->Prev(&lptbTray->lpObj);

    nItems = m_currentNumberOfApps;

    int nRightEdge = lptbTray->ptbi->m_rc.left;
    int nCurrentNotify = m_currentNumberOfNotifys - 1;
    if (m_currentNumberOfNotifys -  m_maxNotifysToDisplay > m_firstNotifyToDisplay)
    {
        nRightEdge -= (TRAY_SPINNERWIDTH + SEPERATOR);
    }

    while (ptbobj && (ptbobj->ptbi->m_uFlags & HHTBS_NOTIFY))
    {
        if (nCurrentNotify >=  m_firstNotifyToDisplay && nCurrentNotify < (m_firstNotifyToDisplay + m_maxNotifysToDisplay))
        {
            ptbobj->ptbi->m_rc.right =  nRightEdge - SEPERATOR;
            if (ptbobj->ptbi->m_uFlags & HHTBS_INPUTLANG_NOTIFY)
            {
                ((InputLangNotification *) ptbobj->ptbi)->CalcSizeInputLang(m_hwndTaskBar);
            }
            else
            {
                ptbobj->ptbi->m_rc.left = ptbobj->ptbi->m_rc.right - 16;
            }
            // reset
            nRightEdge = ptbobj->ptbi->m_rc.left;
        }
        else
        {
            ptbobj->ptbi->m_rc.right = nRightEdge;
            ptbobj->ptbi->m_rc.left =  ptbobj->ptbi->m_rc.right;
        }
        ptbobj->ptbi->m_rc.top = TOPLINE + 4;
        ptbobj->ptbi->m_rc.bottom = BOTTOMLINE - 3;

        nCurrentNotify--;
        ptbobj = (LPTBOBJ)m_plistTaskBar->Prev(&ptbobj->lpObj);
    }


    if (m_maxAppsToDisplay <= 0)
    {
        nWidth = 0;
    }
    else
    {
        if (nItems < NUMITEMS)
        {
            fLeftO = FALSE;
            nItems = NUMITEMS; // items can't be bigger the 1/NUMITEMS of remaining
        }

        if (nItems > m_maxAppsToDisplay)
        {
            nItems = m_maxAppsToDisplay;
        }

        // available width is between the right edge of the start menu and the left edge of the
        // extra apps button
        nWidth  = lptbFirstNotify->ptbi->m_rc.left - lptbStart->ptbi->m_rc.right; // width of the running apps area
        nWidth -= SEPERATOR*3;        // subtract 3 separators (1 for the border and one on each side)

        if (m_firstNotifyToDisplay > 0)
        {
            nWidth -= TRAY_SPINNERWIDTH;
        }

        nLeftO = fLeftO ? nWidth % nItems : 0;  // width of left over space
        nWidth = nWidth / nItems;

        rc.left = lptbStart->ptbi->m_rc.right;
        rc.top = TOPLINE;
        rc.right = rc.left + nWidth;
        rc.bottom = BOTTOMLINE;

        ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&lptbStart->lpObj);
        while (ptbobj != lptbFirstNotify)
        {
            nItems--;

            if (nLeftO)
            {
                CopyRect(&ptbobj->ptbi->m_rc, &rc);
                ptbobj->ptbi->m_rc.right ++;

                OffsetRect(&ptbobj->ptbi->m_rc, nOffset, 0);

                nOffset += (nWidth + 1);
                nLeftO --;
            }
            else
            {
                CopyRect(&ptbobj->ptbi->m_rc, &rc);

                OffsetRect(&ptbobj->ptbi->m_rc, nOffset, 0);
                nOffset += nWidth;
            }
            ptbobj->ptbi->m_rc.left += SEPERATOR;
            // hiding a non-visible window may only be useful when we only display one item
            if (nItems < 0 || (m_fIsSmallScreen && !IsWindowVisible(ptbobj->ptbi->m_hwndMain))) // hide these items
            {
                ptbobj->ptbi->m_rc.right = ptbobj->ptbi->m_rc.left;
            }

            // Resize the tooltip rect for taskbar buttons
            ti.cbSize = sizeof(TOOLINFO);
            ti.hwnd = m_hwndTaskBar;
            ti.uId = (WPARAM) ptbobj;
            ti.rect = ptbobj->ptbi->m_rc;
            SendMessage(m_hwndTaskBarTT, TTM_NEWTOOLRECT, NULL, (LPARAM) &ti);

            ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
        }
    }

    GetTrayRect(hwndTB, &rc);
    MoveWindow(m_hwndTray, rc.left, rc.top, rc.right-rc.left,
               rc.bottom - rc.top, FALSE);  // MoveWindow(..., TRUE) last param is ignored.
    InvalidateRect(m_hwndTray, NULL, TRUE);

    ptbobj = (LPTBOBJ)lptbFirstNotify;
    while (ptbobj && (ptbobj->ptbi->m_uFlags & HHTBS_NOTIFY))
    {
        // Resize the tool rect for NOTIFY (tray) icons
        ti.cbSize = sizeof(TOOLINFO);
        ti.hwnd = m_hwndTray;
        ti.uId = (WPARAM) ptbobj;
        ti.rect = ptbobj->ptbi->m_rc;
        MapWindowPoints(m_hwndTaskBar, m_hwndTray, (LPPOINT) &(ti.rect), 2);
        SendMessage(m_hwndTaskBarTT, TTM_NEWTOOLRECT, NULL, (LPARAM) &ti);

        ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
    }

    ptbobj = GetItem(hwndTB, (int)HHTB_TRAY);
    if (ptbobj)
    {
        ti.cbSize = sizeof(TOOLINFO);
        ti.hwnd = m_hwndTray;
        ti.uId = (WPARAM) ptbobj;
        CalcSizeClock(&(ti.rect));
        MapWindowPoints(m_hwndTaskBar, m_hwndTray, (LPPOINT) &(ti.rect), 2);
        SendMessage(m_hwndTaskBarTT, TTM_NEWTOOLRECT, NULL, (LPARAM) &ti);
    }

    InvalidateRect(hwndTB, NULL, TRUE);
    return TRUE;
} /* RecalcItems()
   */


int CTaskBar::ItemCount(HWND hwnd)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    return (int)m_plistTaskBar->GetCount();
} /* ItemCount()
   */



BOOL CTaskBar::Draw(HWND hwnd, HDC hdc, LPCRECT lprcUpdate)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LPTBOBJ ptbobj = (LPTBOBJ)m_plistTaskBar->Tail();
    UINT nItem=ItemCount(hwnd)-1;
    RECT rc = {0};

    if (m_bTaskBarHidden)
    {
        return TRUE;
    }

    while (ptbobj != NULL)
    {
        if (ptbobj->ptbi->m_uFlags & (HHTBS_STATUS | HHTBS_NOTIFY))
        {
            if (hwnd == m_hwndTray)
            {
                RECT rcClient = ptbobj->ptbi->m_rc;
                MapWindowPoints(m_hwndTaskBar, m_hwndTray, (LPPOINT) &rcClient, 2);
                
                if (IntersectRect(&rc, &rcClient, lprcUpdate))
                {
                    DrawItem(hwnd, hdc, nItem, FALSE);
                }
            }
        }
        else
        {
            if (hwnd == m_hwndTaskBar)
            {
                if (IntersectRect(&rc, &ptbobj->ptbi->m_rc, lprcUpdate))
                {
                    DrawItem(hwnd, hdc, nItem, FALSE);
                }
            }
        }
        ptbobj = (LPTBOBJ)m_plistTaskBar->Prev(&ptbobj->lpObj);
        nItem --;
    }

    return TRUE;
} /* Draw()
   */


BOOL CTaskBar::DrawItem(HWND hwnd, HDC hdc, UINT nItem, BOOL fPressed)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    if (m_bTaskBarHidden)
    {
        return TRUE;
    }

    LPTBOBJ ptbobj = GetItem(hwnd, nItem);

    if (ptbobj == NULL)
    {
        return FALSE;
    }

    if (hwnd == m_hwndTray)
    {
        if (ptbobj->ptbi->m_uFlags & HHTBS_STATUS)
        {
#if 0 // draw the tray all the time. Instead, don't frame it if it is empty.
            if (m_fShowClock || (GetItem(m_hwndTaskBar, (UINT)HHTB_FIRSTNOTIFY) !=
                                 GetItem(m_hwndTaskBar, (UINT)HHTB_TRAY)))
#endif
            {
                DrawTray(hdc, ptbobj->ptbi, BDR_SUNKENOUTER, GetSysColorBrush(COLOR_MENU));
            }
        }
        else if (ptbobj->ptbi->m_uFlags & HHTBS_NOTIFY)
        {
            return ((TaskbarNotification *) ptbobj->ptbi)->DrawItem(m_hwndTaskBar, m_hwndTray, hdc);
        }
    }
    else if (hwnd == m_hwndTaskBar)
    {
        ptbobj->ptbi->DrawItem(
                                m_hwndTaskBar,
                                hdc,
                                m_hfont,
                                m_fIsSmallScreen,
                                fPressed);
    }

    return TRUE;
} /* DrawItem()
   */

BOOL CTaskBar::UpdateItem(HWND hwnd, UINT nItem, TaskbarItem *pItem)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LPTBOBJ ptbobj = GetItem(hwnd, nItem);

    if (ptbobj == NULL || pItem == NULL)
    {
        return FALSE;
    }

    ptbobj->ptbi->CopyFrom(pItem);

    if (ptbobj->ptbi->m_uFlags & (HHTBS_NOTIFY|HHTBS_STATUS))
    {
        RECT rcTray;

        CopyRect(&rcTray, &pItem->m_rc);
        MapWindowPoints(m_hwndTaskBar, m_hwndTray, (LPPOINT)&rcTray, 2);
        InvalidateRect(m_hwndTray, &rcTray, FALSE);
        UpdateWindow(m_hwndTray);
    }
    else
    {
        InvalidateRect(m_hwndTaskBar, &pItem->m_rc, FALSE);
        UpdateWindow(m_hwndTaskBar);
    }

    return TRUE;
} /* UpdateItem()
   */


UINT CTaskBar::HitTest(HWND hwnd, POINT pt, UINT FAR *lpgrfFlags)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    UINT i=0;
    LPTBOBJ ptbobj = (LPTBOBJ)m_plistTaskBar->Head();

    while (ptbobj)
    {
        if (PtInRect(&ptbobj->ptbi->m_rc, pt))
        {
            if (lpgrfFlags)
            {
                *lpgrfFlags = ptbobj->ptbi->m_uFlags;
                return i;
            }
        }

        i++;
        ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
    }

    return (UINT)-1;
} /* HitTest()
   */


TaskbarItem * CTaskBar::GetItemInfo(HWND hwnd, UINT nItem)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LPTBOBJ ptbobj = GetItem(hwnd, nItem);

    if (ptbobj == NULL)
    {
        return NULL;
    }

    return ptbobj->ptbi;
} /* GetItemInfo()
   */


BOOL CTaskBar::Destroy(HWND hwndTB)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    // Free up used resources
    DestroyWindow(hwndTB);
    return TRUE;
} /* Destroy()
   */



BOOL CTaskBar::NotifyMouseEvent(HWND hwndTB, UINT iItem, UINT uMsg, WPARAM wParam, LPARAM lParam)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LPTBOBJ ptobj = (LPTBOBJ) GetItem(hwndTB, iItem);

    if (NULL == ptobj)
    {
        BOOL bRet = FALSE;
        if (iItem == TRAY_SPINNER_DOWN)
        {
            if (m_firstNotifyToDisplay -1 >=  0)
            {
                m_firstNotifyToDisplay--;
                bRet = TRUE;
            }
        }
        else if (iItem == TRAY_SPINNER_UP)
        {
            if (m_firstNotifyToDisplay +1  <=  m_currentNumberOfNotifys - m_maxNotifysToDisplay)
            {
                m_firstNotifyToDisplay++;
                bRet = TRUE;
            }
        }
        else
        {
            ASSERT(ptobj);
        }

        if (bRet) RecalcItems(hwndTB);

        return bRet;
    }

    if (ptobj->ptbi->m_uFlags & HHTBS_BUBBLE_NOTIFY)
    {
        return NotifyMouseEventBubble(hwndTB, iItem, uMsg, wParam, lParam);
    }

    if (((TaskbarNotification *) ptobj->ptbi)->m_uCallbackMessage)
    {
        if (IsWindow(ptobj->ptbi->m_hwndMain))
        {
            PostMessage(
                        ptobj->ptbi->m_hwndMain,
                        ((TaskbarNotification *) ptobj->ptbi)->m_uCallbackMessage,
                        ((TaskbarNotification *) ptobj->ptbi)->m_uID,
                        uMsg);
        }
        else if (iItem != (UINT) -1)
        {
            // Remove the item from the taskbar if it doesn't really exist.
            DeleteItem(hwndTB, iItem);
            RecalcItems(hwndTB);
        }
        return TRUE;
    }

    return FALSE;
} /* NotifyMouseEvent()
   */


void CTaskBar::ForceButtonUp(HWND hwnd, int index)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    MSG msg;
    POINT pt;
    UINT grfFlags;

    if (PeekMessage(&msg, hwnd, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_NOREMOVE))
    {
        pt.x = (int)LOWORD(msg.lParam);
        pt.y = (int)HIWORD(msg.lParam);

        if ((int)HitTest(hwnd, pt, &grfFlags) == index)
        {
            PeekMessage(&msg, hwnd, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_REMOVE);
        }
    }
} /* ForceStartButtonUp()
   */


void CTaskBar::StartMenu(HWND hwnd)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    HDC hdc;

    // Prevent the start menu from coming up twice...
    if (!m_bInStartMenu && !IN_MENU(m_pExtrasMenu))
    {
        BOOL fWasHidden = m_bTaskBarHidden;
        Show(hwnd);
        m_bInStartMenu = TRUE;

        SetForegroundWindow(hwnd);

        if (!(m_bTaskBarAutoHide && fWasHidden && m_bTaskBarTameStartMenu))
        {
            hdc = GetDC(hwnd);
            DrawItem(hwnd, hdc, 0, TRUE);

            TaskbarItem *pItem = GetItemInfo(hwnd, 0);
            RECT rc = pItem->m_rc;

            ClientToScreen(hwnd, (LPPOINT)&rc);
            ClientToScreen(hwnd, (LPPOINT)&rc.right);

            StartMenu_Track(TPM_LEFTALIGN | TPM_BOTTOMALIGN, rc.left, rc.top - 1,hwnd);

            DrawItem(hwnd, hdc, 0, FALSE);
            ReleaseDC(hwnd, hdc);
            ForceButtonUp(hwnd, 0);
        }

        m_bInStartMenu = FALSE;
    }
    else
    {
        SetCapture(hwnd);
        ReleaseCapture();
    }
} /* StartMenu()
   */


void CTaskBar::SipMenu(POINT pt, int index)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    MENUITEMINFO mii;
    HMENU hmenuSip;
    HMENU hmenu;
    HDC hdc;

    if (!m_bInSipMenu)
    {
        ClientToScreen(m_hwndTaskBar, &pt);

        hmenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDM_CONTEXT_TASKBAR_SIP));
        if (!hmenu)
        {
            return;
        }

        hmenuSip = GetSubMenu(hmenu, 0);
        if (!hmenuSip)
        {
            return;
        }

        m_bInSipMenu = TRUE;
        hdc = GetDC(m_hwndTaskBar);
        DrawItem(m_hwndTaskBar, hdc, index, TRUE);

        // Moved over the SIP menu so that it wouldn't go outside the
        // screen boundary in multimonitor scenario
        TrackPopupMenu(hmenuSip, TPM_BOTTOMALIGN | TPM_RIGHTALIGN,       // changed from LEFTALIGN
                              pt.x + 12, pt.y, 0, m_hwndTaskBar, NULL);   // changed from x-4 to x+12

        DrawItem(m_hwndTaskBar, hdc, index, FALSE);
        ReleaseDC(m_hwndTaskBar, hdc);
        ForceButtonUp(m_hwndTaskBar, index);

        m_bInSipMenu = FALSE;

        // remove all items
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_DATA|MIIM_ID;

        while(GetMenuItemInfo(hmenuSip, 0, MF_BYPOSITION, &mii))
        {
            DeleteMenu(hmenuSip, 0, MF_BYPOSITION);
        }

        DestroyMenu(hmenuSip);
    }
    else
    {

        SetCapture(m_hwndTaskBar);
        ReleaseCapture();
    }
} /* SipMenu()
   */


LPTBOBJ CTaskBar::GetActive(HWND hwnd)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LPTBOBJ lptbStart, lptbTray, ptbobj;

    lptbStart = GetItem(hwnd, (int)HHTB_START);
    lptbTray  = GetItem(hwnd, (int)HHTB_FIRSTNOTIFY);

    if (NULL ==  (ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&lptbStart->lpObj)))
    {
        ASSERT(ptbobj);
        return NULL;
    }

    while (ptbobj != lptbTray)
    {
        if (ptbobj->ptbi->m_uFlags & HHTBS_CHECKED)
        {
            return ptbobj;
        }
        ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
    }

    return NULL;
} /* GetActive()
   */


BOOL CTaskBar::ActivateNextItem(HWND hwnd)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LPTBOBJ ptbobj, lptbStart;

    if (ptbobj = GetActive(hwnd))
    {
        ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
    }

    if (!ptbobj || ptbobj == GetItem(hwnd, (int)HHTB_FIRSTNOTIFY))
    {
        lptbStart = GetItem(hwnd, (int)HHTB_START);
        ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&lptbStart->lpObj);

        if (ptbobj == GetItem(hwnd, (int)HHTB_FIRSTNOTIFY))
        {
            return FALSE;
        }
    }

    if (ptbobj)
    {
        CheckApp(hwnd,
                           FindItemIndex(hwnd, ptbobj->ptbi->m_hwndMain),
                           TRUE);
    }

    return TRUE;
} /* ActivateNextItem()
   */


BOOL CTaskBar::ExecuteKey(DWORD key, DWORD modifiers)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    HKEY hkey;
    LONG lRet;
    WCHAR CmdLine[MAX_PATH];
    DWORD dw = MAX_PATH;
    DWORD dwType;

    // Open up the registry

    // use cmdline as a temp for keyname creation
    StringCbPrintfW(CmdLine, sizeof(CmdLine), L"%s\\%2.2X%2.2X", c_szShellKeys, modifiers, key);
    DEBUGMSG(ZONE_VERBOSE,(L"Shell execute macro key %s\\%2.2X%2.2X\r\n",
                 c_szShellKeys, modifiers, key));
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, CmdLine, 0,0,&hkey))
    {
        lRet = RegQueryValueEx(hkey, L"", 0, &dwType,
                        (LPBYTE)CmdLine, &dw);
        RegCloseKey(hkey);
        if (lRet == ERROR_SUCCESS)
        {
            //make sure that CmdLine is terminated
            CmdLine[MAX_PATH-1] = '\0';

            DEBUGMSG(ZONE_VERBOSE,(L">> Found command %s\r\n", CmdLine));

#ifdef TASKBAR_HOST
            Host_Exec(NULL, CmdLine, NULL);
#else
            ExecSilent(NULL, CmdLine, NULL);
#endif
            return TRUE;
        }
    }

    return FALSE;
} /* ExecuteKey
   */


void CTaskBar::OnWmShellNotify(
    HWND    hwndTaskBar,
    int     NotifyCode,
    LPARAM  lParam,
    int*    pItem
)
{
    TaskbarItem *pTBItem = NULL;
    int iItem;
    HWND hwndApp = (HWND)lParam;

    DEBUGMSG(ZONE_VERBOSE,(L"WM_SHELLNOTIFY hwnd=%x lParam=%d\r\n", hwndApp, lParam));

    if (hwndApp == m_hwndTaskBar)
    {
        return;
    }
    else if (hwndApp == Desktop_GetWindow())
    {
        CheckApp(hwndTaskBar, 0, FALSE);
        return;
    }

    iItem = FindItemIndex(hwndTaskBar, hwndApp);

    switch (NotifyCode)
    {
        case SHELLNOTIFY_WINDOWCREATED:
            DEBUGMSG(ZONE_VERBOSE, (L"WINDOWCREATED: %d\r\n", GetWindowLong(hwndApp, GWL_EXSTYLE)));
            if (iItem == -1)
            {
                ASSERT(hwndTaskBar == m_hwndTaskBar);
                InsertItem(hwndApp);
            }
            break;

        case SHELLNOTIFY_WINDOWDESTROYED:
            if (iItem != -1)
            {
                DeleteItem(hwndTaskBar, iItem);
                // reset nItem since the list has changed
                *pItem = -1;
#ifdef PEGMETRICS
                NKDbgPrintfW(L"%10d uSec to TaskBar item destroyed.\r\n",ProfileStart(-1,FALSE));
#endif
            }
            break;

        case SHELLNOTIFY_WINDOWACTIVATED:
            if (iItem != -1)
            {
                CheckApp(hwndTaskBar, iItem, FALSE);
            }
            break;

        case SHELLNOTIFY_REDRAW:
            if (iItem != -1)
            {
                pTBItem = GetItemInfo(hwndTaskBar, iItem);
                if (!GetWindowTextWDirect(hwndApp, pTBItem->m_wszItem, MAX_PATH))
                {
                    DEBUGMSG(ZONE_VERBOSE, (L"SHELLNOTIFY_REDRAW: No title\r\n"));
                    pTBItem->m_wszItem[0] = L'\0';
                }

                if ((pTBItem->m_uFlags & HHTBF_DESTROYICON) && pTBItem->m_hIcon)
                {
                    DestroyIcon(pTBItem->m_hIcon);
                    pTBItem->m_uFlags &= ~HHTBF_DESTROYICON;
                }

                if (!::SendMessageTimeout(hwndApp,
                                WM_GETICON,
                                0,
                                0,
                                SMTO_NORMAL,
                                TASKBAR_SENDMESSAGE_TIMEOUT,
                                (DWORD*)&pTBItem->m_hIcon))
                {
                    pTBItem->m_hIcon = NULL;
                }

                if (pTBItem->m_hIcon == (HICON)E_FAIL)
                {
                    pTBItem->m_hIcon = NULL;
                }

                if (pTBItem->m_hIcon == NULL)
                {
                    pTBItem->m_hIcon = m_hiconDefault;
                }

                UpdateItem(hwndTaskBar, iItem, pTBItem);
            }
            break;

        case SHELLNOTIFY_CAPSLOCK:
            {
                NOTIFYICONDATA nid;

                nid.cbSize = sizeof(NOTIFYICONDATA);
                nid.hWnd = m_hwndTaskBar;
                nid.uFlags = NIF_ICON;
                nid.uCallbackMessage = CAPSLOCK_NOTIFYICON;
                nid.hIcon = NULL;
                nid.szTip[0] = 0;

                if (lParam)
                {
                    nid.uFlags = NIF_ICON;
                    nid.uID = HHTBI_CAPSLOCK;
                    nid.hIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_CAPSLOCK),
                                                 IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
                    Shell_NotifyIconI(NIM_ADD, &nid, sizeof(nid));
                    NotifyTagDestroyIcon(m_hwndTaskBar, &nid);
                }
                else
                {
                    nid.uID = HHTBI_CAPSLOCK;
                    Shell_NotifyIconI(NIM_DELETE, &nid, sizeof(nid));
                }
                break;
            }

        case SHELLNOTIFY_LANGUAGE:
            UpdateInputLangIcon((HKL)lParam);
            break;

        default:
            break;
        }

    return;
}


BOOL CTaskBar::CheckApp(HWND hwnd, UINT nItem, BOOL fShowItem, BOOL *pbRedraw)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    UINT iIndex=1;
    LPTBOBJ ptbobj=NULL, ptbActive=NULL;
    HWND hwndMinimized=NULL, hwndRestored=NULL;
    RECT rc1, rc2, rc;

    if (pbRedraw)
    {
        *pbRedraw = TRUE;
    }

    SetRect(&rc1, 0, 0, 0, 0);
    SetRect(&rc2, 0, 0, 0, 0);

    // Should not really call with this combination.  -1 means
    // just monkey with the button visuals.  In cases where we
    // can't find the right window, if fShowItem were true, we
    // would blast the desktop to the foreground in the
    // ptbobj == NULL case below.
    if ((nItem == -1) && fShowItem)
    {
        fShowItem = FALSE;
    }

    if (! nItem)
    {
        // NOTE: this means bring the desktop window up and uncheck the active
        //       app if there is one.
        if (ptbActive = GetActive(hwnd))
        {
            ptbActive->ptbi->m_uFlags &= ~HHTBS_CHECKED;
            ptbActive->ptbi->m_uFlags |= HHTBS_BUTTON;

            hwndMinimized = ptbActive->ptbi->m_hwndMain;
            CopyRect(&rc1, &ptbActive->ptbi->m_rc);
        }

        // Restore Z-order
        if (m_hwndRudeFullScreen)
        {
            fShowItem = TRUE;
            HandleFullScreenOrTopmostApp(NULL);
        }
    }
    else if (nItem == (UINT)-1)
    {
        // NOTE: this means find the app which has the foreground window
        // and check its taskbar button.
        HWND hwndApp = GetForegroundWindow();
        HWND hwndParentOwner;

        while (hwndParentOwner = GetParent(hwndApp))    // assignment in while
        {
            hwndApp = hwndParentOwner;
        }

        ptbobj = FindItem(hwndApp);

        if (ptbActive = GetActive(hwnd))
        {
            ptbActive->ptbi->m_uFlags &= ~HHTBS_CHECKED;
            ptbActive->ptbi->m_uFlags |= HHTBS_BUTTON;

            hwndMinimized = ptbActive->ptbi->m_hwndMain;
            CopyRect(&rc1, &ptbActive->ptbi->m_rc);
        }

    }
    else
    {
        // NOTE: If we return here it is because the SHELLNOTIFY_WINDOWACTIVATED
        // happened because we just called SetForegroundWindow() on the window.

        ptbobj = GetItem(hwnd, nItem);
        if (!ptbobj)
        {
            ASSERT(0);
            return FALSE;
        }

        if ((ptbobj->ptbi->m_hwndMain == HHTB_EXTRA) || (ptbobj->ptbi->m_hwndMain == HHTB_SIP))
        {
            // don't want to be here
            return FALSE;
        }

        // validate the window
        if (!IsWindow(ptbobj->ptbi->m_hwndMain) && (ptbobj->ptbi->m_hwndMain != HHTB_DESKTOP))
        {
            DeleteItem(hwnd, nItem);
            UpdateWindow(hwnd);
            if (pbRedraw)
            {
                *pbRedraw = FALSE;
            }
            return FALSE;
        }

        // First see if we consider this window fullscreen
        // assume not fullscreen
        if (ptbobj->ptbi->m_uFlags & HHTBF_ACTIVATE)
        {
            ptbobj->ptbi->m_uFlags &= ~HHTBF_ACTIVATE;
            HandleFullScreenOrTopmostApp((TaskbarApplicationButton *)ptbobj->ptbi);
            return FALSE;
        }

        if (ptbActive = GetActive(hwnd))
        {
            // If it is a popup window on the taskbar, then it must be
            // parented to the desktop and we don't allow minimizing it.
            // we also don't allow minimizing WS_DISABLED windows
            long lStyle = GetWindowLong(ptbobj->ptbi->m_hwndMain, GWL_STYLE);
            if (ptbobj == ptbActive &&
                ((lStyle & WS_POPUP) || (lStyle & WS_DISABLED)))
            {
                return FALSE;
            }

            ptbActive->ptbi->m_uFlags &= ~HHTBS_CHECKED;
            ptbActive->ptbi->m_uFlags |= HHTBS_BUTTON;

            hwndMinimized = ptbActive->ptbi->m_hwndMain;
            CopyRect(&rc1, &ptbActive->ptbi->m_rc);
        }
        else
        {
            //NOTE: Need to reset the desktop button, perhaps.
            ptbActive = GetItem(hwnd, (int)HHTB_DESKTOP);
            if (ptbActive && (ptbActive->ptbi->m_uFlags & HHTBS_CHECKED))
            {
                ptbActive->ptbi->m_uFlags &= ~HHTBS_CHECKED;
                ptbActive->ptbi->m_uFlags |= HHTBS_BUTTON;
                InvalidateRect(hwnd, &ptbActive->ptbi->m_rc, FALSE);

                ptbActive = NULL;
            }
        }

        if (ptbobj->ptbi->m_rc.right == ptbobj->ptbi->m_rc.left)
        {
            ptbobj = (LPTBOBJ)m_plistTaskBar->Disconnect(&ptbobj->lpObj);
            m_plistTaskBar->InsertBeforeIndex(&ptbobj->lpObj, m_indexFirstApp);
            RecalcItems(m_hwndTaskBar);
        }

        if (fShowItem && (ptbobj == ptbActive))
        {
#if USE_SW_MINIMIZE
            ShowWindow(ptbobj->ptbi->m_hwndMain, SW_MINIMIZE);
#else
            SetWindowPos(ptbobj->ptbi->m_hwndMain, HWND_BOTTOM, 0, 0, 0, 0,
                         SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
#endif
            sndPlaySound(c_szMinimize, SND_ALIAS | SND_ASYNC | SND_NODEFAULT);
            CheckApp(hwnd, (UINT)-1, FALSE);
            ptbActive->ptbi->Animate(hwnd, FALSE);
            return FALSE;
        }

        // Finally, let the tray know about any fullscreen windowage
        HandleFullScreenOrTopmostApp((TaskbarApplicationButton *)ptbobj->ptbi);
    }

    // NOTE: from this point on we base everything off of the value
    //       of ptbobj;


    if (ptbobj == NULL)
    {
        if (fShowItem)
        {
            RestoreForegroundWindow(Desktop_GetWindow());
            UpdateWindow(Desktop_GetWindow());
        }
    }
    else
    {
        ptbobj->ptbi->m_uFlags &= ~HHTBS_BUTTON;
        ptbobj->ptbi->m_uFlags |= (HHTBS_CHECKED|HHTBS_STATE);

        hwndRestored = ptbobj->ptbi->m_hwndMain;
        CopyRect(&rc2, &ptbobj->ptbi->m_rc);

        if (fShowItem)
        {
            // NOTE: This bit will get reset when we get the
            // SHELLNOTIFY_WINDOWACTIVATED message;
            ptbobj->ptbi->m_uFlags |= HHTBF_ACTIVATE;

            if (!(GetWindowLong(ptbobj->ptbi->m_hwndMain, GWL_STYLE) & WS_POPUP))
            {
                sndPlaySound(c_szMaximize, SND_ALIAS | SND_ASYNC | SND_NODEFAULT);
                ptbobj->ptbi->Animate(hwnd, TRUE);
            }
            RestoreForegroundWindow(ptbobj->ptbi->m_hwndMain);
            UpdateWindow(ptbobj->ptbi->m_hwndMain);
        }
    }

    if (hwndMinimized == hwndRestored)
    {
        return TRUE;
    }

    if (hwndMinimized)
    {
        GetClientRect(hwndMinimized, &rc);
        InvalidateRect(hwnd, &rc1, FALSE);
    }


    if (hwndRestored)
    {
        if (fShowItem)
        {
            GetClientRect(hwndRestored, &rc);
        }
        InvalidateRect(hwnd, &rc2, FALSE);
    }

    if (hwndMinimized || hwndRestored)
    {
        UpdateWindow(hwnd);
    }

    return TRUE;
} /* CheckApp()
   */


BOOL CTaskBar::DrawTray(HDC hdc, TaskbarItem *pItem, UINT edge, HBRUSH hbr)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    int  cchText, cxEdge, xOffset = 1, yOffset = 1;
    WCHAR ach[CCHMAXTBLABEL + CCHELLIPSES];
    BOOL fNeedEllipses;
    RECT rcEdge, rcText;
    int textLeft = 0;
    SIZE siz;
    HFONT hfontOld;

    cxEdge = GetSystemMetrics(SM_CXEDGE);
    CopyRect(&rcText, &pItem->m_rc);
    //NOTE: The tray needs to be drawn in tray window,
    //      so the rectangle is in wrong coordinates.
    MapWindowPoints(m_hwndTaskBar, m_hwndTray, (LPPOINT)&rcText, 2);
    textLeft = rcText.left;

    GetTrayRect(m_hwndTaskBar, &rcEdge);
    //NOTE: The tray needs to be drawn in tray window,
    //      so the rectangle is in wrong coordinates.
    MapWindowPoints(m_hwndTaskBar, m_hwndTray, (LPPOINT)&rcEdge, 2);

    SelectObject(hdc, hbr);
    SetTextColor(hdc, GetSysColor(COLOR_BTNHIGHLIGHT)); // 0 -> 0
    SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));        // 1 -> 1

    // only draw the dither brush where the mask is 1's
    PatBlt(hdc, rcEdge.left, rcEdge.top, rcEdge.right - rcEdge.left, rcEdge.bottom - rcEdge.top, PATCOPY);

    // draw the frame/edge if the tray is not empty
    if(m_fShowClock || m_currentNumberOfNotifys)
    {
        if (!m_fIsSmallScreen)
        {
            DrawEdge(hdc, &rcEdge, edge, BF_RECT | BF_SOFT);
        }
        else
        {
#if 0
            RECT rc2;
            GetClientRect(m_hwndTaskBar, &rc2);
            rc2.left = rcEdge.left;
            rc2.right = rcEdge.right;
#endif

                DrawEdge(hdc, &rcEdge, EDGE_ETCHED, BF_LEFT);
        }

    }
    InflateRect(&rcText, -cxEdge, -cxEdge);
    if (edge == EDGE_SUNKEN)
    {
        xOffset = 2;
        yOffset = 3;
    }

    rcText.left += xOffset;
    rcText.top  += (yOffset - 2);

    StringCchCopy(ach, ARRAYSIZE(ach), pItem->m_wszItem);

    if (m_hfont)
    {
        hfontOld = (HFONT)SelectObject(hdc, m_hfont);
    }

    GetTextExtentPoint(hdc, c_szEllipses, CCHELLIPSES, &siz);
    fNeedEllipses = TaskbarNeedsEllipses(hdc, pItem->m_wszItem, &rcText,
                                            &cchText, siz.cx);
    if (fNeedEllipses)
    {
        StringCchCopy((ach + cchText), (ARRAYSIZE(ach) - cchText), c_szEllipses);
        cchText += CCHELLIPSES;
    }
    else
    {
        cchText = lstrlen(ach);
    }

    SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT) /*RGB(0,0,0)*/);
    SetBkMode(hdc, TRANSPARENT);
    DrawText(hdc, ach, cchText, &rcText,
             DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_VCENTER);

    if (m_hfont)
    {
        SelectObject(hdc, hfontOld);
    }

    RECT rcButton = rcEdge;
    HBRUSH hbrButton = CreateSolidBrush(RGB(0,0,200));
    InflateRect(&rcButton, 0, -((rcButton.bottom-rcButton.top)/4));
    if (m_firstNotifyToDisplay > 0 && m_currentNumberOfNotifys > 0)
    {
        rcButton.left += (2*SEPERATOR);
        rcButton.right = rcButton.left + TRAY_SPINNERWIDTH;
        DrawArrow(hdc, hbrButton, BG_LEFT, rcButton);
    }

    if (m_currentNumberOfNotifys -  m_maxNotifysToDisplay > m_firstNotifyToDisplay  && m_currentNumberOfNotifys > m_maxNotifysToDisplay)
    {
        rcButton.right = textLeft - SEPERATOR;
        rcButton.left = rcButton.right -  TRAY_SPINNERWIDTH;
        DrawArrow(hdc, hbrButton, BG_RIGHT, rcButton);
    }
    DeleteObject(hbrButton);

    return TRUE;
} /* DrawTray()
   */

LRESULT CALLBACK CTaskBar::s_WarningDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (g_TaskBar)
    {
        return g_TaskBar->WarningDlgProc(hwnd,msg,wParam,lParam);
    }
    else
    {
        return 0;
    }
}

LRESULT CALLBACK CTaskBar::WarningDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    if (m_pPowerUI != NULL)
    {
        if (m_pPowerUI->HandleWarningDialogMessage(hwnd, msg, wParam, lParam))
        {
            return TRUE;
        }
    }

    switch (msg)
    {
        case WM_INITDIALOG:
            SetWindowLong(hwnd, DWL_USER, lParam);
 #ifdef TASKBAR_HOST
            Host_CenterWindow(hwnd, NULL);
#else
            CenterWindow(hwnd, NULL);
 #endif
            AygInitDialog(hwnd, SHIDIF_SIPDOWN);
            MessageBeep((UINT)-1);
            return TRUE;
            break;

        default:
            break;
    }

    return FALSE;
} /* WarningDlgProc()
   */


int CALLBACK CTaskBar::s_DocumentMenuDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (g_TaskBar)
    {
        return g_TaskBar->DocumentMenuDlgProc(hwnd,msg,wParam,lParam);
    }
    else
    {
        return 0;
    }
}


int
CALLBACK
CTaskBar::DocumentMenuDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    HWND hwndCtrl;

    switch(msg)
    {
        case WM_INITDIALOG:
        {
            hwndCtrl = GetDlgItem(hwnd, IDC_EXPAND);
            SendMessage(hwndCtrl, BM_SETCHECK,
                        m_bExpandControlPanel ? BST_CHECKED : BST_UNCHECKED, 0);
        }
        break;

        case WM_NOTIFY:
        {
            LPNMHDR pmn = (LPNMHDR) lParam;
            if (pmn->code == PSN_APPLY)
            {
                ApplyProperties(hwnd, FALSE);
                SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
                PostMessage(NULL, WM_NULL, 0, 0);
            }
            else if (pmn->code == PSN_QUERYCANCEL)
            {
                SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
                PostMessage(NULL, WM_NULL, 0, 0);
            }
            else if (pmn->code == PSN_SETACTIVE)
            {
                BOOL bEnabled = ((0 == GetRecentDocsCount()) ? FALSE : TRUE);
                EnableWindow(GetDlgItem(hwnd, IDOK), bEnabled);
            }
        }
        break;


        case WM_COMMAND:
        {
            switch (GET_WM_COMMAND_ID(wParam,lParam))
            {
                case IDOK:
                    SHAddToRecentDocs(0, 0);
                    EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);
                    hwndCtrl = GetDlgItem(hwnd, IDC_EXPAND);
                    if (hwndCtrl)
                    {
                        SetFocus(hwndCtrl);
                    }
                    break;
            }
            break;

            default:
                break;
        }
        break;

    }

    return 0;
} /* DocumentMenuDlgProc()
   */


int CALLBACK CTaskBar::s_PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam)
{
    if (uMsg == PSCB_PRECREATE)
    {
        ((DLGTEMPLATE *) lParam)->dwExtendedStyle |= WS_EX_TOOLWINDOW;
    }

    return TRUE;
}

int CALLBACK CTaskBar::s_PropertiesSubclass(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_MOVE)
    {
        POINT pt = {0};
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        HMONITOR hMonitor = NULL;
        MONITORINFO mi = {0};
        mi.cbSize = sizeof(mi);

        hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
        if (hMonitor && GetMonitorInfo(hMonitor, &mi))
        {
            RECT rc = {0};
            GetWindowRect(hwnd, &rc);
            if ((rc.top+5) >= mi.rcWork.bottom)
            {
                SetWindowPos(hwnd, NULL, pt.x, mi.rcWork.bottom-(rc.bottom-rc.top),
                             0, 0, SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOZORDER);
            }
        }
    }

    return CallWindowProc((WNDPROC) GetWindowLong(hwnd, GWL_USERDATA),
                          hwnd,  msg, wParam, lParam);
}


int CALLBACK CTaskBar::s_PropertiesDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (g_TaskBar)
    {
        return g_TaskBar->PropertiesDlgProc(hwnd,msg,wParam,lParam);
    }
    else
    {
        return 0;
    }
}


int
CALLBACK
CTaskBar::PropertiesDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    HWND hwndCtrl;

    switch(msg)
    {
        case WM_INITDIALOG:
        {
            AygInitDialog(hwnd, SHIDIF_SIPDOWN);

            hwndCtrl = GetDlgItem(hwnd, IDC_TASKBAR_AUTOHIDE);
            SendMessage(hwndCtrl, BM_SETCHECK,
                        m_bTaskBarAutoHide ? BST_CHECKED : BST_UNCHECKED, 0);

            hwndCtrl = GetDlgItem(hwnd, IDC_TASKBAR_ONTOP);
            SendMessage(hwndCtrl, BM_SETCHECK,
                        m_bTaskBarOnTop ? BST_CHECKED : BST_UNCHECKED, 0);

            hwndCtrl = GetDlgItem(hwnd, IDC_TASKBAR_SHOWCLOCK);
            SendMessage(hwndCtrl, BM_SETCHECK,
                        m_fShowClock ? BST_CHECKED : BST_UNCHECKED, 0);

            hwnd = GetParent(hwnd); // the parent
#ifdef TASKBAR_HOST
            Host_CenterWindow(hwnd, NULL);
#endif
        }
        break;

        case WM_NOTIFY:
        {
            LPNMHDR pmn = (LPNMHDR)lParam;
            if (pmn->code == PSN_APPLY)
            {
                ApplyProperties(hwnd, TRUE);
                SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
                PostMessage(NULL, WM_NULL, 0, 0);
            }
            else if ((pmn->code == PSN_KILLACTIVE) ||
                    (pmn->code == PSN_QUERYCANCEL))
            {
                SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
                PostMessage(NULL, WM_NULL, 0, 0);
            }
        }
        break;
    }

    return 0;
} /* ProperiesDlgProc()
   */

// A rude window is considered "active" if:
// (a) it is in the same thread as the foreground window, or
// (b) it is in the same window hierarchy as the foreground window
BOOL IsRudeWindowActive(HWND hwnd)
{
    BOOL bRet = FALSE;

    HWND hwndFore = GetForegroundWindow();

    DWORD dwID = GetWindowThreadProcessId(hwnd, NULL);
    DWORD dwIDFore = GetWindowThreadProcessId(hwndFore, NULL);

    if (dwID == dwIDFore)
    {
        bRet = TRUE;
    }
    else
    {
        // See if the foreground window is the same as hwnd
        // or a descendant of it
        while (hwndFore)
        {
            if (hwndFore == hwnd)
            {
                bRet = TRUE;
                break;
            }

            hwndFore = GetParent(hwndFore);
        }
    }

    return bRet;
}

LRESULT CTaskBar::HandleShellHook(int iCode, LPARAM lParam)
{
    LRESULT lRes = TRUE;
    HWND hwnd = (HWND)lParam;

    switch (iCode)
    {
        case HSHELL_WINDOWACTIVATED:
            //ITaskbarList::ActivateTab((HWND)lParam);
            SendMessage(m_hwndTaskBar,
                            WM_SHELLNOTIFY,
                            SHELLNOTIFY_WINDOWACTIVATED,
                            lParam);
            break;

        case HSHELL_WINDOWCREATED:
            //ITaskbarList::AddTab((HWND)lParam);
            SendMessage(m_hwndTaskBar,
                            WM_SHELLNOTIFY,
                            SHELLNOTIFY_WINDOWCREATED,
                            lParam);
            break;

        case HSHELL_WINDOWDESTROYED:
            //ITaskbarList::DeleteTab((HWND)lParam);
            SendMessage(m_hwndTaskBar,
                            WM_SHELLNOTIFY,
                            SHELLNOTIFY_WINDOWDESTROYED,
                            lParam);
            break;

        default:
            lRes = 0;
    }

    return lRes;
}

LRESULT CALLBACK CTaskBar::s_TaskBarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   if (g_TaskBar)
   {
       return g_TaskBar->TaskBarWndProc(hwnd,msg,wParam,lParam);
   }
   else
   {
       return 0;
   }
}

LRESULT CALLBACK CTaskBar::TaskBarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
/*---------------------------------------------------------------------------*\
*
\*---------------------------------------------------------------------------*/
{
    static int s_nItem= -1;
    static BOOL nLast, fFocus, fWindowsKey;

    POINT pt;
    HDC hdc=NULL;
    RECT rc;
    PAINTSTRUCT ps;
    UINT grfFlags;
    TaskbarItem *pItem = NULL;
    LRESULT l = 0;

    if (m_fTaskbarDisabled)
    {
        l = DisabledTaskBarWndProc(hwnd, msg, wParam, lParam);
        if (l != 0)
        {
            return l;
        }
    }

    l = m_pExtrasMenu->HandleMessage(hwnd, msg, wParam, lParam);
    if (l != 0)
    {
        return l;
    }

    switch (msg)
    {
        case WM_CREATE:
        {
            LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
            // SetWindowLong(hwnd, HHTASKBARID, (UINT)lpcs->hMenu);

            // Make this window IME disabled
            ImmAssociateContext(hwnd, NULL);
            break;
        }

        case WM_SETFOCUS:
        case WM_KILLFOCUS:
            pItem = GetItemInfo(hwnd, 0);
            InvalidateRect(hwnd, &pItem->m_rc, FALSE);
            if (msg == WM_KILLFOCUS) Hide(hwnd);
            UpdateWindow(hwnd);
            break;

        case WM_SYSKEYUP:
        case WM_KEYUP:
            switch (wParam)
            {
                case VK_LWIN:
                case VK_RWIN:
                    if (fWindowsKey)
                    {
                        fWindowsKey = FALSE;
                        StartMenu(hwnd);
                    }
                    break;

                default:
                    // if you want other default behaviour, do not ignore
                    // the return value of Keymap_ProcessKey, see other reference
                    // in WM_KEYDOWN
                    Keymap_ProcessKey(msg, wParam, lParam);
                    break;
            }
            break;

        // Hopefully we'll never get anything we shouldn't from WM_SYSKEYDOWN...
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:

            fWindowsKey = (wParam == VK_LWIN || wParam == VK_RWIN) ? TRUE : FALSE;

            switch (wParam)
            {
                case 'E':
                    // NOTE: Open browser of "My Pegasus"
                    if (GetKeyState(VK_LWIN) || GetKeyState(VK_RWIN))
                    {
                        if (m_bInStartMenu)
                        {
                            StartMenu(hwnd);
                        }
#ifdef TASKBAR_HOST
                        Host_Exec(hwnd, L"\\", NULL);
#else
                        ExecSilent(hwnd, L"\\", NULL);
#endif
                    }
                    break;

                case 'R':
                    // NOTE: Open the run dialog
                    if (GetKeyState(VK_LWIN) || GetKeyState(VK_RWIN))
                    {
                        if (m_bInStartMenu)
                        {
                            StartMenu(hwnd);
                        }
                        ShowRunDlg();
                    }
                    break;

                case 'C':
                    // NOTE: Open the control panel
                    if (GetKeyState(VK_LWIN) || GetKeyState(VK_RWIN))
                    {
                        if (m_bInStartMenu)
                        {
                            StartMenu(hwnd);
                        }

   #ifdef OS_CONTROLPANEL
                        MyCreateProcess(c_szControlPanel, NULL);
   #else
   #ifdef TASKBAR_HOST
                        Host_Exec(hwnd, c_szControlPanel, NULL);
   #else
                        ExecSilent(hwnd, (LPWSTR)c_szControlPanel, NULL);
   #endif
   #endif
                    }
                    break;

                case 'K':
                    // NOTE: Open the keyboard applet
                    if (GetKeyState(VK_LWIN) || GetKeyState(VK_RWIN))
                    {
                        if (m_bInStartMenu)
                        {
                            StartMenu(hwnd);
                        }
                        CStringRes sz(g_hInstance);
                        ExecSilent(hwnd, (LPWSTR)L"ctlpnl", (LPWSTR)sz.LoadString(IDS_CTLPNLKEYBOARD));
                    }
                    break;

                case 'I':
                    // NOTE: Open the stylus applet
                    if (GetKeyState(VK_LWIN) || GetKeyState(VK_RWIN))
                    {
                        if (m_bInStartMenu)
                        {
                            StartMenu(hwnd);
                        }
                        CStringRes sz(g_hInstance);
                        ExecSilent(hwnd, (LPWSTR)L"ctlpnl", (LPWSTR)sz.LoadString(IDS_CTLPNLSTYLUS));
                    }
                    break;

                case 'H':
                    // NOTE: Open help
                    if (GetKeyState(VK_LWIN) || GetKeyState(VK_RWIN))
                    {
                        if (m_bInStartMenu)
                        {
                            StartMenu(hwnd);
                        }
                    }
                    break;

                case 'M':
                    if (GetKeyState(VK_LWIN) || GetKeyState(VK_RWIN))
                    {
                        if (GetActive(hwnd))
                        {
                            CheckApp(hwnd, 0, TRUE);
                        }
                    }
                    break;

                case VK_F1:
                    {
                        // Launch help
                        CStringRes sz(g_hInstance);
#pragma prefast(suppress:309, "Argument 9 is supposed to be set to NULL in CE")
                        CreateProcess(c_szPegHelp, sz.LoadString(IDS_HELPFILE), NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
                    }
                    break;

                case VK_DELETE:
                case VK_BACK:
                    if (GetKeyState(VK_CONTROL) && GetKeyState(VK_MENU) && m_pTaskManDlg)
                    {
                        m_pTaskManDlg->Show();
                    }
                    break;

                case VK_TAB:
                    // Until we get the real ALT-Tab, launch the taskmanger
                    if (GetKeyState(VK_MENU))
                    {
#ifdef TASKBAR_TABHOOK
                        if (g_pfnAltTabHook)
                        {
                            HRESULT hr;
                            __try
                            {
                                hr = g_pfnAltTabHook(m_hwndTaskBar,
                                                ItemCount(m_hwndTaskBar),
                                                GetItem);
                            }
                            __except (EXCEPTION_EXECUTE_HANDLER)
                            {
                                hr = E_FAIL;
                            }

                            if (SUCCEEDED(hr))
                            {
                                break;
                            }
                        }
 #endif
                        if (m_pTaskManDlg)
                        {
                            m_pTaskManDlg->Show();
                        }
                    }
                    else
                    {
                        if (GetFocus() == m_hwndTaskBar)
                        {
                            SetForegroundWindow(Desktop_GetWindow());
                        }
                    }
                    break;

                case VK_ESCAPE:
                    if (GetKeyState(VK_CONTROL))
                    {
                        StartMenu(hwnd);
                    }
                    else if (GetKeyState(VK_MENU))
                    {
                        ActivateNextItem(hwnd);
                    }
                    break;

                case VK_SPACE:
                    // NOTE: Open help on the winkeys.
                    if (GetKeyState(VK_LWIN) || GetKeyState(VK_RWIN))
                    {
                        if (m_bInStartMenu)
                        {
                            StartMenu(hwnd);
                        }
                        break;
                    }

                    // NOTE: need to fall through

                case VK_RETURN:
                    StartMenu(hwnd);
                    break;

                default:
                {
                    if (Keymap_ProcessKey(msg, wParam, lParam))
                    {
                        break;
                    }

                    DWORD keyflags = 0;
                    if (GetKeyState(VK_LWIN) || GetKeyState(VK_RWIN))
                    {
                        keyflags |= MK_LWIN;
                    }

                    if (GetKeyState(VK_MENU))
                    {
                        keyflags |= MK_ALT;
                    }

                    if (GetKeyState(VK_CONTROL))
                    {
                        keyflags |= MK_CONTROL;
                    }

                    if (GetKeyState(VK_SHIFT))
                    {
                        keyflags |= MK_SHIFT;
                    }

                    ExecuteKey(wParam, keyflags);
                    break;
                }
                break;
            }
            break;

        case WM_ERASEBKGND:
        {
            LPTBOBJ lptb, ptbobj;
            HRGN hrgn;
            RECT rcClip, rcIntersect;

            if (m_bTaskBarHidden)
            {
                GetClientRect(hwnd, &rcClip);
                FillRect((HDC)wParam, &rcClip, GetSysColorBrush(COLOR_3DFACE));
                return TRUE;
            }

            hdc = (HDC)wParam;

            GetClipBox(hdc, &rcClip);
            hrgn = CreateRectRgnIndirect(&rcClip);

            SelectClipRgn(hdc, hrgn);
            DeleteObject(hrgn);

            m_plistTaskBar->Lock(TRUE);

            lptb   = GetItem(m_hwndTaskBar, (int)HHTB_FIRSTNOTIFY);
            ptbobj = (LPTBOBJ)m_plistTaskBar->Head();

            while (ptbobj != lptb)
            {
                if (IntersectRect(&rcIntersect, &ptbobj->ptbi->m_rc, &rcClip))
                {
                    ExcludeClipRect(hdc,
                                    ptbobj->ptbi->m_rc.left,
                                    ptbobj->ptbi->m_rc.top,
                                    ptbobj->ptbi->m_rc.right,
                                    ptbobj->ptbi->m_rc.bottom);
                }
                ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
            }
            ptbobj = GetItem(m_hwndTaskBar, (int)HHTB_DESKTOP);
            if (IntersectRect(&rcIntersect, &ptbobj->ptbi->m_rc, &rcClip))
            {
                ExcludeClipRect(hdc,
                                ptbobj->ptbi->m_rc.left,
                                ptbobj->ptbi->m_rc.top,
                                ptbobj->ptbi->m_rc.right,
                                ptbobj->ptbi->m_rc.bottom);
            }

            ptbobj = GetItem(m_hwndTaskBar, (int)HHTB_SIP);
            if (m_fShowSipButton && IntersectRect(&rcIntersect, &ptbobj->ptbi->m_rc, &rcClip))
            {
                ExcludeClipRect(hdc,
                                ptbobj->ptbi->m_rc.left,
                                ptbobj->ptbi->m_rc.top,
                                ptbobj->ptbi->m_rc.right,
                                ptbobj->ptbi->m_rc.bottom);
            }

            if (m_fShowClock || (GetItem(m_hwndTaskBar, (UINT)HHTB_FIRSTNOTIFY) !=
                                 GetItem(m_hwndTaskBar, (UINT)HHTB_TRAY)))
            {
                GetTrayRect(hwnd, &rc);
                if (IntersectRect(&rcIntersect, &ptbobj->ptbi->m_rc, &rcClip))
                {
                    ExcludeClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
                }
            }
            m_plistTaskBar->Lock(FALSE);

            FillRect(hdc, &rcClip, GetSysColorBrush(COLOR_3DFACE));

            if (rcClip.top == 0)
            {
                rcClip.bottom = rcClip.top + 1;
                FillRect(hdc, &rcClip, GetSysColorBrush(COLOR_3DLIGHT));
            }

            SelectClipRgn(hdc, NULL);
            return TRUE;
            break;
        }

        case WM_COMMAND:

            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDM_TASKBAR_WARNING:
                    if (m_pPowerUI)
                    {
                        // ignore return val until there is another warning handler
                        m_pPowerUI->ShowPowerWarning(hwnd, lParam, (DLGPROC) s_TrayWndProc);
                    }
                    break;

                case IDM_TASKBAR_CLOSE:
                {
                    if (s_nItem != -1)
                    {
                        LPTBOBJ ptobj = (LPTBOBJ)GetItem(hwnd, s_nItem);
                        if (ptobj != NULL)
                        {
                            PostMessage(ptobj->ptbi->m_hwndMain, WM_CLOSE, 0, 0);
                        }
                    }
                    break;
                }

                case IDM_TASKBAR_PROPERTIES:
                    Properties(hwnd);
                    break;

                case IDM_TASKBAR_TASKMAN:
                    if (m_pTaskManDlg)
                    {
                        m_pTaskManDlg->Show();
                    }
                    break;

                case (IDM_TASKBAR_CLOSE + 1):
                    if ((UINT)LOWORD(lParam) == 0)
                    {
                        LPTBOBJ ptbobj = (LPTBOBJ)GetItem(hwnd, (int)HHTB_DESKTOP);
                        ptbobj->ptbi->m_uFlags &= ~HHTBS_BUTTON;
                        ptbobj->ptbi->m_uFlags |= (HHTBS_CHECKED|HHTBS_STATE);
                        InvalidateRect(hwnd, &ptbobj->ptbi->m_rc, FALSE);
                        UpdateWindow(hwnd);
                    }
                    CheckApp(hwnd, (UINT)LOWORD(lParam), (BOOL)HIWORD(lParam));
                    break;

                case ID_TASKBARCLOCK:
                    CheckShowClock();
                    break;

                case IDM_TASKBAR_DESTROY_STARTMENU:
                    StartMenu_Destroy();
                    break;

                default:
                    if (GET_WM_COMMAND_ID(wParam, lParam))
                    {
                        if (SipMenu_ProcessCommand(hwnd, wParam, lParam))
                        {
                            break;
                        }
                        else if (StartMenu_ProcessCommand(hwnd, wParam, lParam))
                        {
                            break;
                        }
                        else
                        {
                            return DefWindowProc(hwnd, msg, wParam, lParam);
                        }
                    }
            }
            break;

        case WM_MEASUREITEM:
            if (m_bInStartMenu)
            {
                return StartMenu_MeasureItem((MEASUREITEMSTRUCT *)lParam);
            }
            break;

        case WM_DRAWITEM:
            if (m_bInStartMenu)
            {
                return StartMenu_DrawMenuItem((DRAWITEMSTRUCT *)lParam);
            }
            break;

        case WM_MENUCHAR:
            if (m_bInStartMenu)
            {
                return StartMenu_MenuChar((HMENU)lParam, (WCHAR)LOWORD(wParam));
            }
            break;

        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            Draw(hwnd, hdc, &ps.rcPaint);
            EndPaint(hwnd, &ps);
            break;

        case WM_INITMENUPOPUP:
        {

            if (m_bInStartMenu)
            {
                StartMenu_InitPopup((HMENU)wParam);
            }
            else
            if (m_bInSipMenu)
            {
                SipMenu_InitPopup((HMENU)wParam);
            }
            else
            {
                DWORD point;

                point = GetMessagePos();

                pt.x  = LOWORD(point);
                pt.y  = HIWORD(point);

                ScreenToClient(hwnd, &pt);

                if (HitTest(hwnd, pt, &grfFlags) != -1)
                {
                    if (grfFlags & HHTBS_CHECKED)
                    {
                        EnableMenuItem((HMENU)wParam, IDM_TASKBAR_MIN,
                                       MF_BYCOMMAND | MF_ENABLED);
                        EnableMenuItem((HMENU)wParam, IDM_TASKBAR_MAX,
                                       MF_BYCOMMAND | MF_GRAYED);
                    }
                    else
                    {
                        EnableMenuItem((HMENU)wParam, IDM_TASKBAR_MIN,
                                       MF_BYCOMMAND | MF_GRAYED);
                        EnableMenuItem((HMENU)wParam, IDM_TASKBAR_MAX,
                                       MF_BYCOMMAND | MF_ENABLED);
                    }
                }
            }
            break;
        }

        case WM_LBUTTONDOWN:
            if (!GetAsyncKeyState(VK_MENU))
            {
                pt.x = (int)LOWORD(lParam);
                pt.y = (int)HIWORD(lParam);

                SetFocus(hwnd);
                BOOL fWasHidden = m_bTaskBarHidden;
                Show(hwnd);

                if ((s_nItem = HitTest(hwnd, pt, &grfFlags)) == 0)
                {
                    StartMenu(hwnd);
                }
                else if (s_nItem != -1)
                {
                    DEBUGMSG(ZONE_VERBOSE, (L"TASKBAR: Button %d of %d pressed\r\n", s_nItem,
                                  ItemCount(hwnd)));

                    // If the taskbar is appearing (was hidden) and we're clicking on an app
                    // button, bail out now so we don't really activate the button.
                    // ItemCount - 3 is the desktop button, which we want to work.
                    if (m_bTaskBarAutoHide &&
                         fWasHidden &&
                         (s_nItem < (ItemCount(hwnd) - 3)))
                    {
                        break;
                    }

                    LPTBOBJ ptbobj = GetItem(hwnd, s_nItem);
                    if (ptbobj->ptbi->m_hwndMain == HHTB_EXTRA)
                    {
                        // always execute from the top left of the button rect
                        pt.x = ptbobj->ptbi->m_rc.left;
                        pt.y = ptbobj->ptbi->m_rc.top -1;
                        if (m_pExtrasMenu)
                        {
                            m_pExtrasMenu->LaunchMenu(hwnd, pt);
                        }
                        break;
                    }

                    if (m_fShowSipButton && (s_nItem == ItemCount(hwnd) - 2))
                    {
                        // always execute from the top left of the button rect
                        pt.x = ptbobj->ptbi->m_rc.left;
                        pt.y = ptbobj->ptbi->m_rc.top -1;
                        SipMenu(pt, s_nItem);
                        break;
                    }

                    if ((grfFlags & HHTBS_BUTTON) || (grfFlags & HHTBS_STATE))
                    {
                        //SetForegroundWindow(hwnd);
                        SetCapture(hwnd) ;
                        fFocus = TRUE;

                        hdc = GetDC(hwnd);
                        DrawItem(hwnd, hdc, s_nItem, nLast = TRUE);
                        ReleaseDC(hwnd, hdc);
                    }
                    else if (grfFlags & HHTBS_NOTIFY)
                    {
                        NotifyMouseEvent(hwnd, s_nItem, msg, wParam, lParam);
                        nLast = FALSE;
                    }
                }

                break;
            }
            // fall-thru for alt-tap

        case WM_RBUTTONUP:
        {
            pt.x = (int)LOWORD(lParam);
            pt.y = (int)HIWORD(lParam);

            SetFocus(hwnd);
            Show(hwnd);

            s_nItem = HitTest(hwnd, pt, &grfFlags);
            DEBUGMSG(ZONE_VERBOSE, (L"WM_RBUTTONDOWN: Hit nItem = %d\r\n", s_nItem));

            if ((s_nItem != -1) &&
                    (s_nItem != 0)) { // clicked on an application button
                DEBUGMSG(ZONE_INFO, (L"Clicked on application button\r\n"));

                if (s_nItem >= (ItemCount(hwnd) - 3))
                {
                    break;
                }

                HMENU hmenu, hmenuTrack;
                LPTBOBJ ptbobj = (LPTBOBJ)GetItem(hwnd, s_nItem);

                if (NULL != ptbobj)
                {
                    HWND hwndMain = ptbobj->ptbi->m_hwndMain;

                    if (IsWindowEnabled(hwndMain))
                    {
                        RestoreForegroundWindow(hwndMain);
                        ClientToScreen(hwnd, &pt);

                        SetForegroundWindow(hwnd);
                        hmenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDM_CONTEXT_TASKBAR_BUTTON));
                        hmenuTrack = GetSubMenu(hmenu, 0);
                        m_bInPopupMenu = TRUE;
                        TrackPopupMenu(hmenuTrack, TPM_BOTTOMALIGN | TPM_LEFTALIGN,
                                       pt.x - 4, pt.y, 0, hwnd, NULL);
                        m_bInPopupMenu = FALSE;
                        RestoreForegroundWindow(hwndMain);
                        DestroyMenu(hmenu);
                    }
                }
            }
            else
            {
                // clicked on the start menu or the background
                DEBUGMSG(ZONE_INFO, (L"Clicked on Start menu or background\r\n"));
                HMENU hmenu, hmenuTrack;
                hmenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDM_CONTEXT_TASKBAR));
                hmenuTrack = GetSubMenu(hmenu, 0);
                ClientToScreen(hwnd, &pt);
                m_bInPopupMenu = TRUE;
                DEBUGMSG(ZONE_VERBOSE, (L"TASKBAR: In Context Menu\r\n"));
                TrackPopupMenu(hmenuTrack, TPM_BOTTOMALIGN | TPM_LEFTALIGN,
                               pt.x - 4, pt.y, 0, hwnd, NULL);
                m_bInPopupMenu = FALSE;
                DEBUGMSG(ZONE_VERBOSE, (L"TASKBAR: Out Context Menu\r\n"));
                DestroyMenu(hmenu);
            }

            break;
        }

        case WM_MOUSEMOVE:
        {
            int iItem;
            pt.x = (int)LOWORD(lParam);
            pt.y = (int)HIWORD(lParam);

            if (m_bHasMouse)
            {
                SetUnhideTimer(pt.x, pt.y);
            }

            if ((iItem = HitTest(hwnd, pt, &grfFlags)) != (UINT)-1)
            {
                if (grfFlags & HHTBS_NOTIFY)
                {
                    NotifyMouseEvent(hwnd, iItem, msg, wParam, lParam);
                }
            }

            if (fFocus)
            {
                hdc = GetDC(hwnd);
                if (iItem == s_nItem)
                {
                    if (!nLast)
                    {
                        DrawItem(hwnd, hdc, s_nItem, nLast = TRUE);
                    }
                }
                else
                {
                    if (nLast)
                    {
                        DrawItem(hwnd, hdc, s_nItem, nLast = FALSE);
                    }
                }
                ReleaseDC(hwnd, hdc);
            }
            break;
        }

        case WM_LBUTTONUP:
        {
            UINT iItem;
            pt.x = (int)LOWORD(lParam);
            pt.y = (int)HIWORD(lParam);
            BOOL bRedraw = TRUE;

            if ((iItem = HitTest(hwnd, pt, &grfFlags)) != (UINT)-1)
            {
                if (grfFlags & HHTBS_NOTIFY)
                {
                    NotifyMouseEvent(hwnd, iItem, msg, wParam, lParam);
                }
            }

            if (fFocus)
            {
                ReleaseCapture() ;
                fFocus = FALSE;
            }

            hdc = GetDC(hwnd);
            if (nLast)
            {
                LPTBOBJ ptbobj = (LPTBOBJ)GetItem(hwnd, s_nItem);
                if (ptbobj->ptbi->m_hwndMain == HHTB_DESKTOP)
                {
                    if (ptbobj->ptbi->m_uFlags & HHTBS_CHECKED)
                    {
                        ptbobj->ptbi->m_uFlags &= ~HHTBS_CHECKED;
                        ptbobj->ptbi->m_uFlags |= HHTBS_BUTTON;

#if USE_SW_MINIMIZE
                        ShowWindow(Desktop_GetWindow(), SW_MINIMIZE);
#else
                        SetWindowPos(Desktop_GetWindow(), HWND_BOTTOM,0, 0, 0, 0,
                                     SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
#endif
                        // refresh the taskbar position
                        SetWindowPos(hwnd, m_bTaskBarOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                                0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

                        CheckApp(hwnd, (UINT)-1, FALSE);
                    }
                    else
                    {
                        ptbobj->ptbi->m_uFlags &= ~HHTBS_BUTTON;
                        ptbobj->ptbi->m_uFlags |= (HHTBS_CHECKED|HHTBS_STATE);
                        CheckApp(hwnd, 0, TRUE);
                    }
                }
                else
                {
                    if ((s_nItem > 0) &&
                         (s_nItem < ItemCount(hwnd)))
                   {
#ifdef PEGMETRICS
                        NKDbgPrintfW(L"Timing taskbar switch\r\n");
                        ProfileStart(0, TRUE);
#endif
                        CheckApp(hwnd, s_nItem, TRUE, &bRedraw);
                        if (!bRedraw)
                        {
                            s_nItem = -1;
                        }
                    }
                }

                if (bRedraw)
                {
                    DrawItem(hwnd, hdc, s_nItem, nLast = FALSE);
                }
            }
            ReleaseDC(hwnd, hdc);

            s_nItem = -1;

            break;
        }

        case WM_LBUTTONDBLCLK:
        {
            UINT iItem;
            pt.x = (int)LOWORD(lParam);
            pt.y = (int)HIWORD(lParam);

            if ((iItem = HitTest(hwnd, pt, &grfFlags)) != (UINT)-1)
            {
                if (grfFlags & HHTBS_NOTIFY)
                {
                    NotifyMouseEvent(hwnd, iItem, msg, wParam, lParam);
                }
                else
                {
                    LPTBOBJ ptobj = (LPTBOBJ)GetItem(hwnd, iItem);
                    if (ptobj == GetItem(hwnd, (int)HHTB_TRAY))
                    {
                        // if we don't have clock.exe, try ctlpnl clock.cpl
                        if (!ExecSilent(hwnd, L"clock", L"Date & Time"))
                        {
                            ExecSilent(hwnd, L"ctlpnl", L"clock.cpl");
                        }
                    }
                }
            }
            break;
        }

        case WM_WININICHANGE:
            WinIniChange(hwnd, msg, wParam, lParam);
            break;

        case WM_UPDATECLOCK:
            TimerAdmin(hwnd, WM_TIMER, IDT_ADMIN, lParam);
            break;

        case WM_TIMER:
            if (wParam == IDT_ADMIN)
            {
                TimerAdmin(hwnd, msg, wParam, lParam);
            }
            else if (wParam == IDT_AUTOHIDE)
            {
                AutoHideTimer(hwnd, msg, wParam, lParam);
            }
            else if (wParam == IDT_AUTOUNHIDE)
            {
                AutoUnhideTimer(hwnd, msg, wParam, lParam);
            }
            else if (wParam == IDT_NOTIFICATION_ICON_UPDATE)
            {
                UpdateTimedNotificationIcons();
            }
            break;
#if 0
        case WM_FILECHANGEINFO:
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
            while (EDList_NextChange()) {};
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
            break;
#endif
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            Cleanup();
            PostQuitMessage(0);
            break;

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;
            if (pnmh && m_hwndTaskBarTT == pnmh->hwndFrom && TTN_NEEDTEXT == pnmh->code)
            {
                LPNMTTDISPINFO lpnmtdi = (LPNMTTDISPINFO)lParam;

                // Find the location of the mouse
                if (!GetCursorPos(&pt))
                {
                    break;
                }
                MapWindowPoints(NULL, m_hwndTaskBar, &pt, 1);

                // Find the item and display it's tooltip
                UINT iItem = HitTest(hwnd, pt, &grfFlags);
                if ((UINT) -1 != iItem)
                {
                    LPTBOBJ ptobj = (LPTBOBJ) GetItem(hwnd, iItem);
                    if (ptobj == GetItem(hwnd, (int) HHTB_START))
                    {
                        LoadString(g_hInstance, IDS_CLICK_HERE_TO_BEGIN, lpnmtdi->szText, 80); // See commctrl.h for length of szText
                    }
                    else if (ptobj == GetItem(hwnd, (int) HHTB_DESKTOP))
                    {
                        LoadString(g_hInstance, IDS_SHOW_DESKTOP, lpnmtdi->szText, 80); // See commctrl.h for length of szText
                    }
                    else if (ptobj == GetItem(hwnd, (int) HHTB_SIP))
                    {
                        LoadString(g_hInstance, IDS_SIP_DESC, lpnmtdi->szText, 80);
                    }
                    else if (ptobj && ptobj->ptbi)
                    {
                        ptobj->ptbi->GetTooltipText(lpnmtdi->szText, 80); // See commctrl.h for length of szText
                    }
                }
            }

            if (pnmh && (m_hwndTaskBarTT == pnmh->hwndFrom))
            {
                SetWindowPos(m_hwndTaskBarTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            }
            break;
        }

        case POWER_NOTIFYICON:
            if (m_pPowerUI != NULL)
            {
                m_pPowerUI->HandleIconMessage(hwnd, wParam, lParam);
            }
            break;

        case WM_SHELLNOTIFY:
            OnWmShellNotify(hwnd, wParam, lParam, &s_nItem);
            break;

        case NM_BUBBLEDONE:
        {
            NM_BUBBLEDONEINFO *pNM = (NM_BUBBLEDONEINFO *) lParam;
            if (pNM)
            {
                RemoveBubble(&pNM->clsid, pNM->dwID);
            }
            LocalFree(pNM);
            FireNextBubble();
        }
        break;

        case WM_IM_INFO:
            switch(wParam)
            {
                case IM_NARROWIMAGE:
                    if (lParam != 0)
                    {
                        m_hiSipCurrentIcon = (HICON) lParam;
                    }
                    else
                    {
                        m_hiSipCurrentIcon = m_hiSipDownIcon;
                    }
                    // update the sip button icon
                    UpdateSipButton();
                    break;

                default:
                    // can ignore rest of messages?
                    break;
            }
            break;

        case WM_COPYDATA: // This is only used for NOTIFYICONDATA and SHNOTIFICATIONDATA
        {
            PCOPYDATASTRUCT pCDS = (PCOPYDATASTRUCT) lParam;
            PNOTIFYICONDATA pNID = NULL;
            NOTIFYCOPYSTRUCT *pNCS = NULL;
            BOOL bRecalc = TRUE;

            if ((NULL == pCDS) || (NULL == pCDS->lpData))
            {
                ASSERT(0);
                return FALSE;
            }

            if ( (sizeof(NOTIFYICONDATA) != pCDS->cbData) && (sizeof(NOTIFYCOPYSTRUCT) != pCDS->cbData))
            {
                ASSERT(0);
                return FALSE;
            }

            if ((pCDS->dwData >= NIM_ADD) && (pCDS->dwData <= NIM_DELETE))
            {
                 pNID = (PNOTIFYICONDATA) pCDS->lpData;
            }
            else if ((pCDS->dwData >= NIM_BUBBLE_ADD) && (pCDS->dwData <= NIM_BUBBLE_DELETE))
            {
                pNCS = (NOTIFYCOPYSTRUCT *) pCDS->lpData;
            }
            else
            {
                return FALSE;
            }

            switch (pCDS->dwData)
            {
                case NIM_ADD:
                {
                    if ((UINT) -1 != FindNotify(m_hwndTaskBar, pNID))
                    {
                        return FALSE;
                    }

                    if (!InsertNotify(m_hwndTaskBar, pNID))
                    {
                        return FALSE;
                    }
                    bRecalc = FALSE;    // insert calls recalc directly

                    break;
                }

                case NIM_MODIFY:
                {
                    int nIcon = FindNotify(m_hwndTaskBar, pNID);
                    if (nIcon < 0)
                    {
                        return FALSE;
                    }

                    if (!ModifyNotify(m_hwndTaskBar, pNID, nIcon))
                    {
                        return FALSE;
                    }
                    bRecalc = FALSE;
                    break;
                }

                case NIM_DELETE:
                {
                    int nIcon = FindNotify(m_hwndTaskBar, pNID);
                    if (nIcon < 0)
                    {
                        return FALSE;
                    }
                    DeleteItem(m_hwndTaskBar, nIcon);
                    bRecalc = FALSE;                    // delete calls recalc directly

                    break;
                }

                case NIM_BUBBLE_MODIFY:
                    bRecalc = FALSE;
                case NIM_BUBBLE_ADD:
                case NIM_BUBBLE_DELETE:
                    if (!HandleNimBubbleMessage(pNCS, pCDS->dwData))
                    {
                        return FALSE;
                    }
                    break;

                default:
                    return FALSE;
            }

            if (bRecalc)
            {
                RecalcItems(GetWindow());
            }
            return TRUE;
        }

        case WM_POWERBROADCAST:
        {
            const WCHAR g_MessengerWindowClass[] = L"MSGSBlObj";
            HWND hwndMessenger;

            hwndMessenger = FindWindow(g_MessengerWindowClass, NULL);
            if (hwndMessenger)
            {
                // Notify Messenger of idle state
                PostMessage(
                                FindWindow(g_MessengerWindowClass, NULL),
                                msg,
                                wParam,
                                lParam
);
            }
        }
            // Place holder for real screen saver implementation
            switch(wParam)
            {
                case PBT_APMUSERIDLE:
                    // ScreenSaver start
                    RETAILMSG(TRUE, (L"Screen Saver Started.\r\n"));
                    break;

                // Screen saver app should be looking for this message
                case PBT_APMUSERACTIVE:
                    // ScreenSaver end
                    RETAILMSG(TRUE, (L"Screen Saver Ended.\r\n"));
                    break;
            }

            break;

        // AYGSHELL SHFullScreen Messages
#define SPECIAL_HIDE_MESSAGE WM_USER+10
#define SPECIAL_SHOW_MESSAGE WM_USER+11
        case SPECIAL_HIDE_MESSAGE:
            m_hwndDisablerWindow = (HWND) wParam;
            m_fTaskbarDisabled = (m_hwndDisablerWindow != NULL);
            return TRUE;
            break;

        case SPECIAL_SHOW_MESSAGE:
            m_hwndDisablerWindow = NULL;
            m_fTaskbarDisabled = FALSE;
            return TRUE;
            break;

        case TBC_SETACTIVEALT:
            //TODO ITaskbarList::SetActiveAlt((HWND) lParam);
            break;

        case TBC_MARKFULLSCREEN:
        {
            HWND hwndFS = (HWND)lParam;
		
            if(wParam)
            {
                KillTimer(m_hwndTaskBar, IDT_AUTOHIDE);
            }
            else
            {
                SetHideTimer();
            }

            if (IsWindow(hwndFS))
            {
                // look for the item they're talking about
                LPTBOBJ ptobj = FindItem(hwndFS);
                if (ptobj == NULL)
                {
                    // we didn't find it, so insert it now
                    InsertItem(hwndFS);
                    ptobj = FindItem(hwndFS);

                    // if it's still not found, then it's one of the taskbar windows
                    // taskbar can't be fullscreen, so we won't take any action
                }
                if (ptobj)
                {
                    // mark it fullscreen/not fullscreen
                    ((TaskbarApplicationButton *)ptobj->ptbi)->m_bMarkedFullscreen = !!(BOOL)wParam;
                    if (IsRudeWindowActive(hwndFS))
                    {
                        // it's active, so tell the tray to hide/show
                        HandleFullScreenOrTopmostApp((TaskbarApplicationButton *)ptobj->ptbi);
                    }
                }

                return TRUE;
            }
        }
        break;

        default:
            if (msg == WM_ShellHook)
            {
                return HandleShellHook((int)wParam, lParam);
            }
            else
            {
                return DefWindowProc(hwnd, msg, wParam, lParam);
            }
    }

    return 0;
} /* HHTaskBarWndProc()
      */

LRESULT CALLBACK CTaskBar::s_TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (g_TaskBar)
    {
        return g_TaskBar->TrayWndProc(hwnd, msg, wParam, lParam);
    }
    else
    {
        return 0;
    }
}


LRESULT CALLBACK CTaskBar::TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
/*---------------------------------------------------------------------------*\
*
\*---------------------------------------------------------------------------*/
{
    static BOOL nLast;
    UINT iItem;
    POINT pt;
    HDC hdc=NULL;
    PAINTSTRUCT ps;
    UINT grfFlags;

    switch (msg)
    {
        case WM_ERASEBKGND:
            {
                RECT rcClip;
                GetClientRect(hwnd, &rcClip);
                FillRect((HDC)wParam, &rcClip, GetSysColorBrush(COLOR_3DFACE));

                return TRUE;
            }break;

        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            Draw(hwnd, hdc, &ps.rcPaint);
            EndPaint(hwnd, &ps);
            break;

        case WM_LBUTTONDOWN:
            pt.x = (int)LOWORD(lParam);
            pt.y = (int)HIWORD(lParam);

            {
                RECT rcTray;
                RECT rcButton;

                MapWindowPoints(m_hwndTray, m_hwndTaskBar, &pt, 1);
                GetTrayRect(m_hwndTaskBar, &rcTray);

                rcButton = rcTray;
                if (m_firstNotifyToDisplay > 0 && m_currentNumberOfNotifys > 0)
                {
                    rcButton.right = rcButton.left + TRAY_SPINNERWIDTH + SEPERATOR;
                    if (PtInRect(&rcButton, pt))
                    {
                        NotifyMouseEvent(hwnd, TRAY_SPINNER_DOWN, msg, wParam, lParam);
                        break;
                    }
                }

                if (m_currentNumberOfNotifys -  m_maxNotifysToDisplay > m_firstNotifyToDisplay  && m_currentNumberOfNotifys > m_maxNotifysToDisplay)
                {
                    LPTBOBJ pTray = GetItem(m_hwndTaskBar, (int)HHTB_TRAY);
                    if (pTray)
                    {
                        rcButton.right = pTray->ptbi->m_rc.left - SEPERATOR;
                        rcButton.left = rcButton.right - TRAY_SPINNERWIDTH;
                        if (PtInRect(&rcButton, pt))
                        {
                            NotifyMouseEvent(hwnd, TRAY_SPINNER_UP, msg, wParam, lParam);
                            break;
                        }
                    }
                }

            }

            if (((iItem = HitTest(hwnd, pt, &grfFlags)) != (UINT)-1) && (grfFlags & HHTBS_NOTIFY))
            {
                NotifyMouseEvent(hwnd, iItem, msg, wParam, lParam);
            }
            break;

        case WM_MOUSEMOVE:
            pt.x = (int)LOWORD(lParam);
            pt.y = (int)HIWORD(lParam);

            MapWindowPoints(m_hwndTray, m_hwndTaskBar, &pt, 1);

            if (((m_ptLastTrayMouseMove.x - pt.x) != 0) || ((m_ptLastTrayMouseMove.y - pt.y) != 0))
            {
                // this is to get around the issue where CommCtrl does a SetWindowPos
                // message which causes a WM_MOUSEMOVE message even if the mouse hasn't
                // moved
                if (((iItem = HitTest(hwnd, pt, &grfFlags)) != (UINT)-1) && (grfFlags & HHTBS_NOTIFY))
                {
                    NotifyMouseEvent(hwnd, iItem, msg, wParam, lParam);
                }
                m_ptLastTrayMouseMove.x = pt.x;
                m_ptLastTrayMouseMove.y = pt.y;
            }
            break;

        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            pt.x = (int)LOWORD(lParam);
            pt.y = (int)HIWORD(lParam);

            MapWindowPoints(m_hwndTray, m_hwndTaskBar, &pt, 1);

            if ((iItem = HitTest(hwnd, pt, &grfFlags)) != (UINT)-1)
            {
                if (grfFlags & HHTBS_NOTIFY)
                {
                    NotifyMouseEvent(hwnd, iItem, msg, wParam, lParam);
                }
            }
            break;

        case WM_LBUTTONDBLCLK:
            pt.x = (int)LOWORD(lParam);
            pt.y = (int)HIWORD(lParam);
            MapWindowPoints(m_hwndTray, m_hwndTaskBar, &pt, 1);

            if ((iItem = HitTest(hwnd, pt, &grfFlags)) != (UINT)-1)
            {
                if (grfFlags & HHTBS_NOTIFY)
                {
                    NotifyMouseEvent(hwnd, iItem, msg, wParam, lParam);
                }
                else
                {
                    LPTBOBJ ptobj = (LPTBOBJ)GetItem(hwnd, iItem);
                    if (ptobj == GetItem(hwnd, (int)HHTB_TRAY))
                    {
                        if (!ExecSilent(hwnd, L"clock", L"Date & Time"))
                        {
                            ExecSilent(hwnd, L"ctlpnl", L"clock.cpl");
                        }
                    }
                }
            }
            break;

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;
            if (pnmh && m_hwndTaskBarTT == pnmh->hwndFrom && TTN_NEEDTEXT == pnmh->code)
            {
                LPNMTTDISPINFO lpnmtdi = (LPNMTTDISPINFO) lParam;

                // Find the location of the mouse
                if (!GetCursorPos(&pt))
                {
                    break;
                }
                MapWindowPoints(NULL, m_hwndTaskBar, &pt, 1);

                // Find the item and display it's tooltip
                iItem = HitTest(hwnd, pt, &grfFlags);
                if ((UINT) -1 != iItem)
                {
                    LPTBOBJ ptobj = (LPTBOBJ) GetItem(hwnd, iItem);
                    if (ptobj == GetItem(hwnd, (int) HHTB_TRAY))
                    {
                        // 80 is the documented length of lpnmtdi->szText
                        GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE,
                                    NULL, NULL, lpnmtdi->szText, 80);
                    }
                    else
                    {
                        if (ptobj && *ptobj->ptbi->m_wszItem)
                        {
                            StringCchCopy(lpnmtdi->szText, 80, ptobj->ptbi->m_wszItem);
                        }
                    }
                }
                // make sure the tooltip window is above everything else
                SetWindowPos(m_hwndTaskBarTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            }
            break;
        }

        case POWER_NOTIFYICON:
            if (m_pPowerUI)
            {
                m_pPowerUI->HandleIconMessage(hwnd, wParam, lParam);
            }
            break;

        case INPUTLANG_NOTIFYICON:
            if ((lParam == WM_LBUTTONDBLCLK) || (lParam == WM_LBUTTONDOWN))
            {
                LPTBOBJ ptbobj = GetItem(m_hwndTaskBar, (int)HHTB_FIRSTNOTIFY);
                while (ptbobj)
                {
                    if (ptbobj->ptbi->m_uFlags & HHTBS_INPUTLANG_NOTIFY)
                    {
                        break;
                    }
                    ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
                }

                if (ptbobj)
                {
                    HKL hklNew = (HKL)InputLang_t::TrackPopUpMenu(m_hwndTaskBar, &ptbobj->ptbi->m_rc);
                    if (hklNew)
                    {
                        PostMessage(
                                        GetForegroundWindow(),
                                        WM_INPUTLANGCHANGEREQUEST,
                                        0,
                                        (LPARAM)hklNew
                                        );
                    }
                }
            }
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
} /* HHTrayWndProc()
 */

/************************************************************
 *                                                          *
 * SipMenu Handling                                         *
 *                                                          *
 ************************************************************/
extern BOOL SIP_InitPopup(HMENU hmenu, HINSTANCE hInst, int idFirst);
extern BOOL SIP_SelectSIP(int iChoice);
extern BOOL SIP_IsSipOn();

BOOL CTaskBar::UpdateSipButton(HICON hIcon)
{
    LPTBOBJ ptbobj = NULL;

    ptbobj = GetItem(m_hwndTaskBar, (int) HHTB_SIP);
    if (!ptbobj)
    {
        return FALSE;
    }

    ptbobj->ptbi->m_hIcon = (hIcon != NULL) ? hIcon : m_hiSipCurrentIcon;
    UpdateItem(m_hwndTaskBar, (int) HHTB_SIP, ptbobj->ptbi);

    return TRUE;
}

BOOL CTaskBar::SipMenu_ProcessCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    int index = GET_WM_COMMAND_ID(wParam, lParam);
    HICON hNewIcon = NULL;

    if ((index < IDM_SIP_MENU_FIRST__) || (index > IDM_SIP_MENU_LAST__))
    {
        return FALSE;
    }

    if (SIP_SelectSIP(index - IDM_SIP_MENU_FIRST__))
    {
        return UpdateSipButton(m_hiSipCurrentIcon);
    }

    return UpdateSipButton(m_hiSipUpIcon);
}


BOOL CTaskBar::SipMenu_InitPopup(HMENU hmenu)
{
    // remove placeholder
    DeleteMenu(hmenu, 0, MF_BYPOSITION);

    return SIP_InitPopup(hmenu, g_hInstance, IDM_SIP_MENU_FIRST__);
}

/************************************************************
 * SipMenu Handling End                                     *
 ************************************************************/

// Move into TaskbarItem?
BOOL CTaskBar::ChangeItemText(HWND hwndApp, LPCWSTR pwszNewText)
{
    if (!hwndApp || !IsWindow(hwndApp))
    {
        return FALSE;
    }

    int iItem = FindItemIndex(m_hwndTaskBar, hwndApp);
    if (iItem == -1)
    {
        return FALSE;
    }

    TaskbarItem *pItem = NULL;

    pItem = GetItemInfo(m_hwndTaskBar, iItem);
    if (pwszNewText)
    {
        StringCchCopyN(pItem->m_wszItem, ARRAYSIZE(pItem->m_wszItem), pwszNewText, (CCHMAXTBLABEL - 1));
    }
    else
    {
        if (!GetWindowTextWDirect(hwndApp, pItem->m_wszItem, CCHMAXTBLABEL))
        {
            pItem->m_wszItem[0] = L'\0';
        }
    }

    UpdateItem(m_hwndTaskBar, iItem, pItem);

    return TRUE;
}

TaskbarApplicationButton *CTaskBar::GetNextAppButton(TaskbarApplicationButton *lastButton)
{
    LPTBOBJ lptbNotify = GetItem(m_hwndTaskBar, (int)HHTB_FIRSTNOTIFY);
    LPTBOBJ ptbobj = NULL;

    //used to get all the buttons
    if (lastButton == NULL)
    {
        ptbobj = GetItem(m_hwndTaskBar, (int)HHTB_START);
        ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
        if ((ptbobj == lptbNotify) || (ptbobj == NULL))
        {
            return NULL;
        }

        return (TaskbarApplicationButton *)ptbobj->ptbi;
    }

    ptbobj = FindItem(lastButton->m_hwndMain);
    if (ptbobj == NULL)
    {
        return NULL;
    }

    ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
    if ((ptbobj == lptbNotify) || (ptbobj == NULL))
    {
        return NULL;
    }

    return (TaskbarApplicationButton *)ptbobj->ptbi;
}

TaskbarApplicationButton *CTaskBar::GetPrevAppButton(TaskbarApplicationButton *nextButton)
{
    LPTBOBJ lptbNotify = GetItem(m_hwndTaskBar, (int)HHTB_FIRSTNOTIFY);
    LPTBOBJ lptbStart = GetItem(m_hwndTaskBar, (int)HHTB_START);
    LPTBOBJ ptbobj = NULL;

    //used to get all the buttons
    if (nextButton == NULL)
    {
        ptbobj = (LPTBOBJ)m_plistTaskBar->Prev(&lptbNotify->lpObj);
        if ((ptbobj == lptbStart) || (ptbobj == NULL))
        {
            return NULL;
        }

        return (TaskbarApplicationButton *)ptbobj->ptbi;
    }

    ptbobj = FindItem(nextButton->m_hwndMain);
    if (ptbobj == NULL)
    {
        return NULL;
    }

    ptbobj = (LPTBOBJ)m_plistTaskBar->Prev(&ptbobj->lpObj);
    if ((ptbobj == lptbStart) || (ptbobj == NULL))
    {
        return NULL;
    }

    return (TaskbarApplicationButton *)ptbobj->ptbi;
}

