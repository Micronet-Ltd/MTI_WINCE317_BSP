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
#include "taskbar.hxx"
#include <aygshell.h>

/*
 * SHNotifcation APIs
 */
LRESULT WINAPI SHNotificationAddII( SHNOTIFICATIONDATA *pndAdd, LPTSTR pszTitle, LPTSTR pszHTML )
{
    UNREFERENCED_PARAMETER(pndAdd);
    UNREFERENCED_PARAMETER(pszTitle);
    UNREFERENCED_PARAMETER(pszHTML);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);

    return ERROR_CALL_NOT_IMPLEMENTED;
}


LRESULT WINAPI SHNotificationUpdateII(DWORD grnumUpdateMask, SHNOTIFICATIONDATA *pndNew, LPTSTR pszTitle, LPTSTR pszHTML)
{
    UNREFERENCED_PARAMETER(grnumUpdateMask);
    UNREFERENCED_PARAMETER(pndNew);
    UNREFERENCED_PARAMETER(pszTitle);
    UNREFERENCED_PARAMETER(pszHTML);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);

    return ERROR_CALL_NOT_IMPLEMENTED;
}

LRESULT WINAPI SHNotificationRemoveII(const CLSID *pclsid, DWORD dwID)
{
    UNREFERENCED_PARAMETER(pclsid);
    UNREFERENCED_PARAMETER(dwID);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);

    return ERROR_CALL_NOT_IMPLEMENTED;
}


LRESULT WINAPI SHNotificationGetDataII( const CLSID *pclsid, DWORD dwID, SHNOTIFICATIONDATA *pndBuffer,
									    LPTSTR pszTitle, LPTSTR pszHTML, DWORD *pdwHTMLLength )
{
    UNREFERENCED_PARAMETER(pclsid);
    UNREFERENCED_PARAMETER(dwID);
    UNREFERENCED_PARAMETER(pndBuffer);
    UNREFERENCED_PARAMETER(pszTitle);
    UNREFERENCED_PARAMETER(pszHTML);
    UNREFERENCED_PARAMETER(pdwHTMLLength);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);

    return ERROR_CALL_NOT_IMPLEMENTED;
}

void CTaskBar::SetNotificationIconUpdateTimer()
{
}

BOOL CTaskBar::UpdateTimedNotificationIcons()
{
    return TRUE;
}

/*
 * NOTIFICATION BUBBLE SUPPORT
 */

UINT CTaskBar::FindNotify(HWND hwndTB, PSHNOTIFICATIONDATA pND)
{
    UNREFERENCED_PARAMETER(hwndTB);
    UNREFERENCED_PARAMETER(pND);

    return (UINT)-1;
}

BOOL CTaskBar::NotifyTagDestroyIcon(HWND hwndTB, PSHNOTIFICATIONDATA pND)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    UNREFERENCED_PARAMETER(hwndTB);
    UNREFERENCED_PARAMETER(pND);

    return FALSE;
}


BOOL CTaskBar::InsertNotify(HWND hwndTB, PSHNOTIFICATIONDATA pND)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    UNREFERENCED_PARAMETER(hwndTB);
    UNREFERENCED_PARAMETER(pND);

    return FALSE;
}

BOOL CTaskBar::AddBubbleToTray( HWND hwndTB, LPTBOBJ ptbobj )
{
    UNREFERENCED_PARAMETER(hwndTB);
    UNREFERENCED_PARAMETER(ptbobj);

    return FALSE;
}


BOOL CTaskBar::AddBubble( PSHNOTIFICATIONDATA pND )
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    UNREFERENCED_PARAMETER(pND);

    return FALSE;
}

BOOL CTaskBar::AddBubble(LPTBOBJ ptbobj)
{
    UNREFERENCED_PARAMETER(ptbobj);

    return FALSE;
}

BOOL CTaskBar::RemoveBubble( const CLSID *pclsid, DWORD dwID )
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    UNREFERENCED_PARAMETER(pclsid);
    UNREFERENCED_PARAMETER(dwID);

    return FALSE;
}

UINT CTaskBar::FindBubble( CLSID clsid, DWORD dwID )
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    UNREFERENCED_PARAMETER(clsid);
    UNREFERENCED_PARAMETER(dwID);

    return (UINT)-1;
}

BOOL CTaskBar::UpdateBubble( DWORD grnumUpdateMask, SHNOTIFICATIONDATA *pndNew )
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    UNREFERENCED_PARAMETER(grnumUpdateMask);
    UNREFERENCED_PARAMETER(pndNew);

    return FALSE;
}

BOOL CTaskBar::GetNotificationData( const CLSID *pclsid, DWORD dwID, SHNOTIFICATIONDATA *pndBuffer,
								    LPTSTR pszTitle, LPTSTR pszHTML, DWORD *pdwHTMLLength )
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    UNREFERENCED_PARAMETER(pclsid);
    UNREFERENCED_PARAMETER(dwID);
    UNREFERENCED_PARAMETER(pndBuffer);
    UNREFERENCED_PARAMETER(pszTitle);
    UNREFERENCED_PARAMETER(pszHTML);
    UNREFERENCED_PARAMETER(pdwHTMLLength);

    return FALSE;
}

BOOL CTaskBar::NotifyMouseEventBubble(HWND hwndTB, UINT iItem, UINT uMsg, WPARAM wParam, LPARAM lParam)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    UNREFERENCED_PARAMETER(hwndTB);
    UNREFERENCED_PARAMETER(iItem);
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

	return FALSE;
}

BOOL CTaskBar::LaunchNotifyBubble(TaskbarBubble *pBubble, BOOL bFromTray)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    UNREFERENCED_PARAMETER(pBubble);
    UNREFERENCED_PARAMETER(bFromTray);

    return FALSE;
}

BOOL CTaskBar::FireNextBubble()
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    return FALSE;
}
/*
 * END NOTIFICATION BUBBLE SUPPORT
 */

LRESULT CALLBACK CTaskBar::DisabledTaskBarWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(msg);
    UNREFERENCED_PARAMETER(wp);
    UNREFERENCED_PARAMETER(lp);

    return FALSE;
}

LRESULT CTaskBar::HandleNimBubbleMessage( NOTIFYCOPYSTRUCT *pNCS, DWORD dwMsg )
{
    UNREFERENCED_PARAMETER(pNCS);
    UNREFERENCED_PARAMETER(dwMsg);

    return FALSE;
}
