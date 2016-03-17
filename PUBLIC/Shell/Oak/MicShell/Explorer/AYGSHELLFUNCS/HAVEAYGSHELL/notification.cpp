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
#include "taskbaritem.h"
#include "taskbarbubble.h"
#include "utils.h"
#include <aygshell.h>
#include "resource.h"

// REVIEW: move this into this directory later
#include "bubble.h"


/*
 * SHNotifcation APIs
 */

extern CTaskBar *g_TaskBar;

const int c_csMinShowTime = 1;  // 1 second minimum
const int c_csDurationDefault = 5;  // 5 second default timeout

LRESULT WINAPI SHNotificationAddII(SHNOTIFICATIONDATA *pndAdd, DWORD cbndAdd, LPTSTR pszTitle, LPTSTR pszHTML)
{
    __try
    {
        COPYDATASTRUCT cds = {0};
        NOTIFYCOPYSTRUCT ncs = {0};
        SHNOTIFICATIONDATA  ndCopy;

        BOOL fRet = TRUE;

        if ((NULL == pndAdd) ||
             (sizeof(SHNOTIFICATIONDATA) != pndAdd->cbStruct) ||
             (FALSE == IsWindow(pndAdd->hwndSink)) ||
             ((pndAdd->npPriority == SHNP_INFORM) && (pszHTML == NULL)) ||
             ((pszHTML == NULL) && (pszTitle == NULL)))
        {
            return ERROR_INVALID_PARAMETER;
        }

        if (!g_TaskBar)
        {
            return ERROR_NOT_READY;
        }

        memcpy(&ndCopy, pndAdd, sizeof(ndCopy));

        ndCopy.pszTitle = pszTitle;
        ndCopy.pszHTML = pszHTML;

        if ((ndCopy.npPriority == SHNP_ICONIC) || (ndCopy.npPriority == SHNP_INFORM))
        {
            if (ndCopy.csDuration == 0)
            {
                if (ndCopy.npPriority == SHNP_ICONIC)
                {
                    ndCopy.csDuration = INFINITE;
                }
                else if (ndCopy.npPriority == SHNP_INFORM)
                {
                    ndCopy.csDuration = c_csDurationDefault;
                }

            }

            // add it to the taskbar notifcation tray
            // Set up the copy data struct
            ncs.pData = &ndCopy;

            cds.cbData = sizeof(NOTIFYCOPYSTRUCT);
            cds.lpData = &ncs;
            cds.dwData = NIM_BUBBLE_ADD;

            HWND hWnd = g_TaskBar->GetWindow();
            return SendMessage(hWnd, WM_COPYDATA, NULL, (LPARAM) &cds) ? ERROR_SUCCESS : ERROR_INVALID_DATA;
        }

        // invalid npPriority
        return ERROR_INVALID_PARAMETER;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        ASSERT(FALSE);
        return FALSE;
    }
}


LRESULT WINAPI SHNotificationUpdateII(DWORD grnumUpdateMask, SHNOTIFICATIONDATA *pndNew, DWORD cbndNew, LPTSTR pszTitle, LPTSTR pszHTML)
{
    __try
    {
        COPYDATASTRUCT cds = {0};
        NOTIFYCOPYSTRUCT ncs = {0};
        BOOL fRet = TRUE;
        SHNOTIFICATIONDATA ndCopy;
        LRESULT lReturn;

        if ((NULL == pndNew) ||
             (sizeof(SHNOTIFICATIONDATA) != pndNew->cbStruct))
        {
           return ERROR_INVALID_PARAMETER;
        }

        if (!g_TaskBar)
        {
            return ERROR_NOT_READY;
        }

        memcpy(&ndCopy, pndNew, sizeof(ndCopy));

        // copy over the data that gets lost in the cross process transition
        ndCopy.pszTitle = pszTitle;
        ndCopy.pszHTML = pszHTML;

        // set the flag bits in the copy so that we can use WM_COPYDATA
        ndCopy.grfFlags = grnumUpdateMask;

        // Set up the copy data struct
        ncs.pData = &ndCopy;

        cds.cbData = sizeof(NOTIFYCOPYSTRUCT);
        cds.lpData = &ncs;
        cds.dwData = NIM_BUBBLE_MODIFY;

        HWND hWnd = g_TaskBar->GetWindow();

        lReturn = SendMessage(hWnd, WM_COPYDATA, NULL, (LPARAM) &cds) ? ERROR_SUCCESS : ERROR_INVALID_DATA;
    
        return lReturn;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        ASSERT(FALSE);
        return FALSE;
    }
}

LRESULT WINAPI SHNotificationRemoveII(const CLSID *pclsid, DWORD cbCLSID, DWORD dwID)
{
    __try
    {
        SHNOTIFICATIONDATA nd = {0};
        SHNOTIFICATIONDATA ndOld = {0};
        NOTIFYCOPYSTRUCT ncs = {0};
        COPYDATASTRUCT cds = {0};

        if (NULL == pclsid)
        {
           return ERROR_INVALID_PARAMETER;
        }

        if (!g_TaskBar)
        {
            return ERROR_NOT_READY;
        }

        if (g_TaskBar->GetNotificationData(pclsid, dwID, &ndOld, NULL, NULL, NULL))
        {
            if (ndOld.npPriority == SHNP_ICONIC)
            {
                nd.clsid = *pclsid;
                nd.dwID = dwID;

                ncs.pData = &nd;

                // Set up the copy data struct
                cds.cbData = sizeof(NOTIFYCOPYSTRUCT);
                cds.lpData = &ncs;
                cds.dwData = NIM_BUBBLE_DELETE;

                HWND hWnd;
                
                hWnd = g_TaskBar->GetWindow();
                return SendMessage(hWnd, WM_COPYDATA, NULL, (LPARAM) &cds) ? ERROR_SUCCESS : ERROR_INVALID_DATA;
            }
            else
            {
                return g_TaskBar->RemoveBubble(pclsid, dwID) ? ERROR_SUCCESS : ERROR_INVALID_DATA;
            }
        }

        // REVIEW: handle removing open bubble
        return ERROR_INVALID_DATA;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        ASSERT(FALSE);
        return FALSE;
    }
}


LRESULT WINAPI SHNotificationGetDataII(const CLSID *pclsid, DWORD cbclsid, DWORD dwID, SHNOTIFICATIONDATA *pndBuffer, DWORD cbBuffer,
                                        LPTSTR pszTitle, DWORD cbTitle, LPTSTR pszHTML, DWORD cbHTML, DWORD *pdwHTMLLength)
{
    __try
    {
        if (g_TaskBar->GetNotificationData(pclsid, dwID, pndBuffer, pszTitle, pszHTML, pdwHTMLLength))
        {
            return ERROR_SUCCESS;
        }

        return ERROR_INVALID_DATA;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        ASSERT(FALSE);
        return FALSE;
    }
}

/* Taskbar functions */
void CTaskBar::SetNotificationIconUpdateTimer()
{
    // find the shortest timer and use it
    UINT uiShortestTime = -1;

    LPTBOBJ ptbobj = (LPTBOBJ)m_plistTaskBar->Head();

    while (ptbobj != NULL)
    {
        if ((ptbobj->ptbi->m_uFlags & HHTBS_BUBBLE_NOTIFY) &&
             ((ptbobj->ptbi->m_uFlags & HHTBF_DESTROYICON) == 0))
        {
            TaskbarBubble *pBubble = (TaskbarBubble *) ptbobj->ptbi;
            if ((pBubble->m_csDuration * 1000) < uiShortestTime)
            {
                uiShortestTime = (pBubble->m_csDuration * 1000) - ((pBubble->m_uiStartTime > 0) ? (GetTickCount() - pBubble->m_uiStartTime) : 0);
            }
        }
        ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
    }

    if (uiShortestTime < (UINT) -1)
    {
        SetTimer(m_hwndTaskBar, IDT_NOTIFICATION_ICON_UPDATE, uiShortestTime, NULL);
    }
    else
    {
        KillTimer(m_hwndTaskBar, IDT_NOTIFICATION_ICON_UPDATE);
    }
}

BOOL CTaskBar::UpdateTimedNotificationIcons()
{
    LPTBOBJ ptbobj = (LPTBOBJ)m_plistTaskBar->Tail();
    UINT uiCurrentTime = GetTickCount();

    while (ptbobj != NULL)
    {
        BOOL bDelete = FALSE;
        int iIndex = -1;
        if (ptbobj->ptbi->m_uFlags & HHTBS_BUBBLE_NOTIFY)
        {
            TaskbarBubble *pBubble = (TaskbarBubble *) ptbobj->ptbi;
            if (pBubble->m_csDuration != INFINITE)
            {
                if ((pBubble->m_uiStartTime + (pBubble->m_csDuration * 1000)) < uiCurrentTime)
                {
                    // remove this icon
                    ptbobj->ptbi->m_uFlags |= HHTBF_DESTROYICON;
                    iIndex = m_plistTaskBar->GetObjectIndex(&ptbobj->lpObj);
                    bDelete = TRUE;
                }
            }
        }
        ptbobj = (LPTBOBJ)m_plistTaskBar->Prev(&ptbobj->lpObj);
        if (bDelete)
        {
            DeleteItem(m_hwndTaskBar, iIndex, TRUE);
            bDelete = FALSE;
        }
    }

    RecalcItems(m_hwndTaskBar);
    SetNotificationIconUpdateTimer();

    return TRUE;
}

/*
 * NOTIFICATION BUBBLE SUPPORT
 */

UINT CTaskBar::FindNotify(HWND hwndTB, PSHNOTIFICATIONDATA pND)
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
        if (ptbobj->ptbi->m_uFlags & HHTBS_BUBBLE_NOTIFY)
        {
            TaskbarBubble *pBubble = (TaskbarBubble *) ptbobj->ptbi;

            if (IsEqualCLSID(pND->clsid, pBubble->m_clsid) && (pND->dwID == pBubble->m_uID))
            {
                return (UINT)m_plistTaskBar->GetObjectIndex(&ptbobj->lpObj);
            }
        }
        ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
    }

    return (UINT)-1;
}

BOOL CTaskBar::NotifyTagDestroyIcon(HWND hwndTB, PSHNOTIFICATIONDATA pND)
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
        if (ptbobj->ptbi->m_uFlags & HHTBS_BUBBLE_NOTIFY)
        {
            TaskbarBubble *pBubble = (TaskbarBubble *) ptbobj->ptbi;

            if (IsEqualCLSID(pND->clsid, pBubble->m_clsid) && (pND->dwID == pBubble->m_uID))
            {
                ptbobj->ptbi->m_uFlags |= HHTBF_DESTROYICON;
                return TRUE;
            }
        }
        ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
    }

    return FALSE;
}


BOOL CTaskBar::InsertNotify(HWND hwndTB, PSHNOTIFICATIONDATA pND)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LPTBOBJ ptbobj;
    TaskbarBubble *pBubble = new TaskbarBubble();
    RECT rc;

    if (!pBubble || !pND)
    {
        return FALSE;
    }

    rc.top = 7;
    rc.bottom = 23;
    ptbobj = GetItem(hwndTB, (int)HHTB_FIRSTNOTIFY);
    rc.right = ptbobj->ptbi->m_rc.left - SEPERATOR;
    rc.left = rc.right - 16;

    // change this to a constructor?
    pBubble->m_hIcon = CopyIcon(pND->hicon);
    pBubble->m_uFlags = HHTBS_NOTIFY | HHTBS_BUBBLE_NOTIFY;
    pBubble->m_rc = rc;
    pBubble->m_hwndMain = pND->hwndSink;
    pBubble->m_uID = pND->dwID;
    pBubble->m_csDuration = pND->csDuration;
    pBubble->m_npPriority = pND->npPriority;
    pBubble->m_grfFlags = pND->grfFlags;
    pBubble->m_clsid = pND->clsid;
    pBubble->m_lParam = pND->lParam;
    pBubble->m_pwszHTML = NULL;
    pBubble->m_bWaitingForShow = FALSE;
    pBubble->m_uiStartTime = 0;

    if (SHNUM_TITLE & (pND->grfFlags))
    {
        if (StringCbCopy((LPWSTR)pBubble->m_wszItem, CCHMAXTBLABEL * sizeof(TCHAR), pND->pszTitle) != S_OK)
        {
            pBubble->m_wszItem[ 0 ] = _T('\0');
        }
    }

    if (SHNUM_HTML & (pND->grfFlags))
    {
        DWORD dwLen = (_tcslen(pND->pszHTML) + 1) * sizeof(TCHAR);
        pBubble->m_pwszHTML = (LPCTSTR) LocalAlloc(LMEM_FIXED, dwLen);
        if (pBubble->m_pwszHTML)
        {
            if (StringCbCopy((LPTSTR) pBubble->m_pwszHTML, dwLen, pND->pszHTML) != S_OK)
            {
                LocalFree((HLOCAL) pBubble->m_pwszHTML);
                pBubble->m_pwszHTML = NULL;
            }
        }
    }

    if ((ptbobj = (LPTBOBJ) LocalAlloc(LMEM_FIXED, sizeof(TBOBJ))) != NULL)
    {
        ptbobj->ptbi = pBubble;
        if (AddBubbleToTray(hwndTB, ptbobj))
        {
            return TRUE;
        }

        delete pBubble;
        LocalFree(ptbobj);
    }
    return FALSE;
}

BOOL CTaskBar::AddBubbleToTray(HWND hwndTB, LPTBOBJ ptbobj)
{
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

        ((TaskbarBubble *) ptbobj->ptbi)->m_uiStartTime = GetTickCount();
        SetNotificationIconUpdateTimer();

        RecalcItems(hwndTB);
        UpdateWindow(m_hwndTray);
        return TRUE;
    }

    return FALSE;
}


BOOL CTaskBar::AddBubble(PSHNOTIFICATIONDATA pND)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LPTBOBJ ptbobj;

    if (!m_plistBubbles)
    {
        return FALSE;
    }

    TaskbarBubble *pBubble = new TaskbarBubble(pND);
    if (!pBubble)
    {
        return FALSE;
    }

    if ((ptbobj = (LPTBOBJ) LocalAlloc(LMEM_FIXED, sizeof(TBOBJ))) != NULL)
    {
        ptbobj->ptbi = pBubble;
        if (AddBubble(ptbobj))
        {
            return TRUE;
        }

        delete pBubble;
        LocalFree(ptbobj);
    }
    else
    {
        delete pBubble;
    }

    return FALSE;
}

BOOL CTaskBar::AddBubble(LPTBOBJ ptbobj)
{
    if (ptbobj && m_plistBubbles->AddHead(&ptbobj->lpObj))
    {
        // ignore return val
        LaunchNotifyBubble((TaskbarBubble *) ptbobj->ptbi);
        return TRUE;
    }

    return FALSE;
}

BOOL CTaskBar::RemoveBubble(const CLSID *pclsid, DWORD dwID)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    if (!m_plistBubbles)
    {
        return FALSE;
    }

    LPTBOBJ ptbobj = (LPTBOBJ)m_plistBubbles->Head();

    while (ptbobj != NULL)
    {
        TaskbarBubble *pBubble = (TaskbarBubble *) ptbobj->ptbi;

        if (IsEqualCLSID(*pclsid, pBubble->m_clsid) && dwID == pBubble->m_uID)
        {
            // close it if it is open
            if (m_pBubbleMgr && m_pBubbleMgr->IsBeingShown(pBubble->m_clsid, (DWORD) pBubble->m_uID))
            {
                m_pBubbleMgr->PopDown();
            }
            ptbobj = (LPTBOBJ) m_plistBubbles->Disconnect(&ptbobj->lpObj);
            delete ptbobj->ptbi;
            LocalFree(ptbobj);
            return TRUE;
        }
        ptbobj = (LPTBOBJ)m_plistBubbles->Next(&ptbobj->lpObj);
    }

    return FALSE;
}

UINT CTaskBar::FindBubble(CLSID clsid, DWORD dwID)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    if (!m_plistBubbles)
    {
        return (UINT)-1;
    }

    LPTBOBJ ptbobj = (LPTBOBJ)m_plistBubbles->Head();

    while (ptbobj != NULL)
    {
        TaskbarBubble *pBubble = (TaskbarBubble *) ptbobj->ptbi;

        if (IsEqualCLSID(clsid, pBubble->m_clsid) && (dwID == pBubble->m_uID))
        {
            return (UINT)m_plistBubbles->GetObjectIndex(&ptbobj->lpObj);
        }
        ptbobj = (LPTBOBJ)m_plistBubbles->Next(&ptbobj->lpObj);
    }

    return (UINT)-1;
}

#define FLAGS_VALID_MASK (SHNUM_PRIORITY | SHNUM_DURATION | SHNUM_ICON | SHNUM_HTML | SHNUM_TITLE)

// move this into taskbarbubble?
BOOL CTaskBar::UpdateBubble(DWORD grnumUpdateMask, SHNOTIFICATIONDATA *pndNew)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    UINT uiIndex = (UINT)-1;
    LPTBOBJ ptbobj = NULL;
    BOOL bAddToList = FALSE;
    TaskbarBubble *pBubble = NULL;

    if (!m_plistBubbles || !m_plistTaskBar || (grnumUpdateMask & ~FLAGS_VALID_MASK) || (grnumUpdateMask == 0))
    {
        return FALSE;
    }

    uiIndex = FindBubble(pndNew->clsid, pndNew->dwID);
    if (uiIndex == (UINT)-1)
    {
        uiIndex = FindNotify(m_hwndTaskBar, pndNew);
        if (uiIndex != (UINT)-1)
        {
            ptbobj = (LPTBOBJ)m_plistTaskBar->GetAtIndex((int) uiIndex);
        }
    }
    else
    {
        ptbobj = (LPTBOBJ)m_plistBubbles->GetAtIndex((int) uiIndex);
    }

    if (NULL == ptbobj)
    {
        ASSERT(ptbobj);
        return FALSE;
    }    

    pBubble = (TaskbarBubble *) ptbobj->ptbi;
    if ((grnumUpdateMask & SHNUM_PRIORITY) && (pndNew->npPriority != pBubble->m_npPriority))
    {
        // changing from ICONIC to INFORM or the other way around
        // remove from the old list
        if (pBubble->m_npPriority == SHNP_ICONIC)
        {
            DeleteItem(m_hwndTaskBar, uiIndex, FALSE);
        }
        else
        {
            m_plistBubbles->DisconnectAtIndex(uiIndex);
        }

        // update the priority
        pBubble->m_npPriority = pndNew->npPriority;

        bAddToList = TRUE;
    }

    if (grnumUpdateMask & SHNUM_DURATION)
    {
        //change the duration
        pBubble->m_csDuration = pndNew->csDuration;
        //REVIEW: update the timer or whatever
    }

    if (grnumUpdateMask & SHNUM_ICON && pndNew->hicon)
    {
        if (pBubble->m_hIcon)
        {
            DestroyIcon(pBubble->m_hIcon);
        }
        pBubble->m_hIcon = CopyIcon(pndNew->hicon);

        InvalidateRect(m_hwndTray, NULL, TRUE);
        UpdateWindow(m_hwndTray);
    }

    if (grnumUpdateMask & SHNUM_TITLE)
    {
        // if we are ICONIC, have to recreate the tooltip
        if (StringCbCopy(pBubble->m_wszItem, sizeof(pBubble->m_wszItem), pndNew->pszTitle) != S_OK)
        {
            pBubble->m_wszItem[ 0 ] = _T('\0');
        }
    }

    if (grnumUpdateMask & SHNUM_HTML)
    {
        // alloc the memory
        int iLen = _tcslen(pndNew->pszHTML) + 1;
        if (pBubble->m_pwszHTML)
        {
            LPCTSTR pstr = (LPCTSTR) LocalReAlloc((void *) pBubble->m_pwszHTML, iLen * sizeof(TCHAR), LMEM_MOVEABLE);
            if (pstr)
            {
                pBubble->m_pwszHTML = pstr;
            }
            else
            {
                iLen = _tcslen(pBubble->m_pwszHTML) + 1;
            }
        }
        else
        {
            pBubble->m_pwszHTML = (LPCTSTR) LocalAlloc(LMEM_FIXED, iLen * sizeof(TCHAR));
        }

        if (pBubble->m_pwszHTML)
        {
            _tcsncpy((LPTSTR) pBubble->m_pwszHTML, pndNew->pszHTML, iLen);
            // if the bubble is open, need to update it
            if (m_pBubbleMgr && m_pBubbleMgr->IsBeingShown(pBubble->m_clsid, (DWORD) pBubble->m_uID))
            {
                m_pBubbleMgr->PopDown();
                m_pBubbleMgr->PopUp(pBubble, (pBubble->m_npPriority == SHNP_ICONIC));
            }
        }
    }

    if (bAddToList)
    {
        // add to the new list
        if (pBubble->m_npPriority == SHNP_ICONIC)
        {
            // add it to the tray
            return AddBubbleToTray(m_hwndTaskBar, ptbobj);
        }
        else
        {
            // add it to the bubble list
            return AddBubble(ptbobj);
        }
    }

    return TRUE;
}

BOOL CTaskBar::GetNotificationData(const CLSID *pclsid, DWORD dwID, SHNOTIFICATIONDATA *pndBuffer,
                                    LPTSTR pszTitle, LPTSTR pszHTML, DWORD *pdwHTMLLength)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    // need to search on clsid and id
    if (!pclsid)
    {
        return FALSE;
    }

    LPTBOBJ ptbobj = NULL;
    TaskbarBubble *pBubble = NULL;
    BOOL bFound = FALSE;
    
    if (m_plistTaskBar)
    {
        ptbobj = (LPTBOBJ)m_plistTaskBar->Head();

        while ((ptbobj != NULL) && !bFound)
        {
            if (ptbobj->ptbi->m_uFlags & HHTBS_BUBBLE_NOTIFY)
            {
                pBubble = (TaskbarBubble *) ptbobj->ptbi;

                if (IsEqualCLSID(*pclsid, pBubble->m_clsid) && (dwID == pBubble->m_uID))
                {
                    bFound = TRUE;
                    break;
                }
            }
            ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
            pBubble = NULL;
        }
    }

    if (m_plistBubbles && !bFound)
    {
        ptbobj = (LPTBOBJ)m_plistBubbles->Head();

        while ((ptbobj != NULL) && !bFound)
        {
            pBubble = (TaskbarBubble *) ptbobj->ptbi;

            if (IsEqualCLSID(*pclsid, pBubble->m_clsid) && (dwID == pBubble->m_uID))
            {
                bFound = TRUE;
                break;
            }

            ptbobj = (LPTBOBJ)m_plistTaskBar->Next(&ptbobj->lpObj);
            pBubble = NULL;
        }
    }

    if (bFound && (pBubble != NULL))
    {
        // copy the data to pndBuffer,
        if (pndBuffer)
        {
            pndBuffer->cbStruct = sizeof(SHNOTIFICATIONDATA);
            pndBuffer->dwID = pBubble->m_uID;
            pndBuffer->npPriority = pBubble->m_npPriority;
            pndBuffer->csDuration = pBubble->m_csDuration;
            pndBuffer->hicon = pBubble->m_hIcon;
            pndBuffer->grfFlags = pBubble->m_grfFlags;
            pndBuffer->clsid = pBubble->m_clsid;
            pndBuffer->hwndSink = pBubble->m_hwndMain;
            pndBuffer->lParam = pBubble->m_lParam;
        }

        if (pszTitle)
        {
            // assumes that pszTitle has length at least of CCHMAXTBLABEL
            _tcscpy(pszTitle, pBubble->m_wszItem);
        }

        if (pszHTML)
        {
            if (pdwHTMLLength)
            {
                _tcsncpy(pszHTML, pBubble->m_pwszHTML, *pdwHTMLLength - 1);
                pszHTML[*pdwHTMLLength - 1] = _T('\0');
            }
            else
            {
                if (StringCbCopy(pszHTML, sizeof(*pBubble->m_pwszHTML), pBubble->m_pwszHTML) != S_OK)
                {
                    pszHTML[0] = _T('\0');
                }
            }
        }
        else
        {
            if (pdwHTMLLength)
            {
                if (pBubble->m_pwszHTML != NULL)
                {
                    *pdwHTMLLength = _tcslen(pBubble->m_pwszHTML) + 1;
                }
                else
                {
                    *pdwHTMLLength = 0;
                }
            }
        }

        return TRUE;
    }

    return FALSE;
}

BOOL CTaskBar::NotifyMouseEventBubble(HWND hwndTB, UINT iItem, UINT uMsg, WPARAM wParam, LPARAM lParam)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    LPTBOBJ ptbobj = (LPTBOBJ) GetItem(hwndTB, iItem);

    if (NULL == ptbobj)
    {
        // should have been caught already
        return FALSE;
    }

    if (!IsWindow(ptbobj->ptbi->m_hwndMain) && (iItem != (UINT) -1))
    {
        DeleteItem(hwndTB, iItem);
        RecalcItems(hwndTB);
        return TRUE;
    }

    if (uMsg == WM_LBUTTONDOWN)
    {
        return LaunchNotifyBubble((TaskbarBubble *) ptbobj->ptbi, TRUE);
    }


    return FALSE;
}

BOOL CTaskBar::LaunchNotifyBubble(TaskbarBubble *pBubble, BOOL bFromTray)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    BOOL bRet = FALSE;

    if (m_pBubbleMgr)
    {
        bRet = m_pBubbleMgr->PopUp(pBubble, bFromTray);
    }

    pBubble->m_bWaitingForShow = !bRet;

    return bRet;
}

BOOL CTaskBar::FireNextBubble()
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    // see if the are any bubbles waiting
    if (!m_plistBubbles)
    {
        return FALSE;
    }

    LPTBOBJ ptbobj = (LPTBOBJ)m_plistBubbles->Head();

    while (ptbobj != NULL)
    {
        TaskbarBubble *pBubble = (TaskbarBubble *) ptbobj->ptbi;

        if (pBubble->m_bWaitingForShow)
        {
            LaunchNotifyBubble(pBubble);
        }

        ptbobj = (LPTBOBJ)m_plistBubbles->Next(&ptbobj->lpObj);
    }

    return FALSE;
}

/*
 * END NOTIFICATION BUBBLE SUPPORT
 */

LRESULT CALLBACK CTaskBar::DisabledTaskBarWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    // filter out the messages that the taskbar shouldn't handle in a disabled state
    if (!m_fTaskbarDisabled)
    {
        return FALSE;
    }

    // special check so that a rogue app can't disable the taskbar forever.
    if (!IsWindow(m_hwndDisablerWindow))
    {
        m_fTaskbarDisabled = FALSE;
        m_hwndDisablerWindow = NULL;
        return FALSE;
    }

    switch (msg)
    {
        case WM_SYSKEYUP:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_MENUCHAR:
        case WM_KEYDOWN:
        case WM_RBUTTONDOWN:
        case WM_MOUSEMOVE:
        case WM_LBUTTONDBLCLK:
        case WM_NOTIFY:
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wp, lp))
            {
                case IDM_TASKBAR_WARNING:
                case IDM_TASKBAR_CLOSE:
                    return FALSE;
            }
            return TRUE;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
            // these messages get forwarded to m_hwndDisablerWindow
            SendMessage(m_hwndDisablerWindow, msg, wp, lp);
            return TRUE;
    }

    return FALSE;
}

LRESULT CTaskBar::HandleNimBubbleMessage(NOTIFYCOPYSTRUCT *pNCS, DWORD dwMsg)
{
    SHNOTIFICATIONDATA *pSHND = NULL;
    
    if ((!pNCS) || (!pNCS->pData))
    {
        return ERROR_INVALID_DATA;
    }

    pSHND = pNCS->pData;

    switch(dwMsg)
    {
        case NIM_BUBBLE_ADD:
        {
            BOOL bRet = TRUE;
            if (pSHND->npPriority == SHNP_ICONIC)
            {
                if ((UINT) -1 == FindNotify(m_hwndTaskBar, pSHND))
                {
                    bRet = InsertNotify(m_hwndTaskBar, pSHND);
                }
                else
                {
                    bRet = FALSE;
                }
            }
            else if (pSHND->npPriority == SHNP_INFORM)
            {
                bRet = AddBubble(pSHND);
            }
            return bRet;
            break;
        }

        case NIM_BUBBLE_MODIFY:
        {
            BOOL bRet = UpdateBubble(pSHND->grfFlags, pSHND);
            return bRet;
            break;
        }

        case NIM_BUBBLE_DELETE:
        {
            BOOL bReturn = TRUE;
            int nIcon = FindNotify(m_hwndTaskBar, pSHND);

            if (nIcon < 0)
            {
                bReturn = FALSE;
            }

            if (bReturn)
            {
                bReturn = DeleteItem(m_hwndTaskBar, nIcon);
            }
            return bReturn;
            break;
        }

        default:
            return ERROR_INVALID_DATA;
    }
}
