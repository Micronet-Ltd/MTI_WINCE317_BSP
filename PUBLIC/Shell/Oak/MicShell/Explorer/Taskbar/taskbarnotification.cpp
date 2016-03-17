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
#include "taskbarnotification.h"

TaskbarNotification::TaskbarNotification() :
    TaskbarItem()
{
}

TaskbarNotification::TaskbarNotification(PNOTIFYICONDATA pnid) :
    TaskbarItem()
{
    m_hIcon = pnid->hIcon;
    m_uFlags = HHTBS_NOTIFY;
    m_hwndMain = pnid->hWnd;
    if (NIF_TIP & (pnid->uFlags))
    {
        if (StringCbCopy(m_wszItem, sizeof(m_wszItem), pnid->szTip) != S_OK)
        {
            m_wszItem[0] = L'\0';
        }
    }
    else
    {
        m_wszItem[0] = L'\0';
    }
    m_uID = pnid->uID;
    m_uCallbackMessage = pnid->uCallbackMessage;
}

TaskbarNotification::TaskbarNotification(HICON hIcon, UINT uFlags, RECT inRect, HWND hwnd, LPWSTR pwszText, UINT id, UINT callback) :
    TaskbarItem(hIcon, uFlags, inRect, hwnd, pwszText)
{
    m_uID = id;
    m_uCallbackMessage = callback;
}

TaskbarNotification::~TaskbarNotification()
{
}

BOOL TaskbarNotification::Update(PNOTIFYICONDATA pnid)
{
    if ((pnid->uFlags & NIF_ICON) && pnid->hIcon)
    {
        if ((m_uFlags & HHTBF_DESTROYICON) && m_hIcon)
        {
            DestroyIcon(m_hIcon);
        }
        m_hIcon = pnid->hIcon;
    }

    if (pnid->uFlags & NIF_MESSAGE)
    {
        m_uCallbackMessage = pnid->uCallbackMessage;
    }

    if (pnid->uFlags & NIF_TIP)
    {
        if (StringCbCopy(m_wszItem, sizeof(m_wszItem), pnid->szTip) != S_OK)
        {
            m_wszItem[0] = L'\0';
        }
    }

    return TRUE;
}

BOOL TaskbarNotification::DrawItem(HWND hwndTaskBar, HWND hwndTray, HDC hdc)
{
    if (m_rc.right > m_rc.left)
    {
        int yOffset = (((m_rc.bottom - m_rc.top) - 16) / 2);

        //NOTE: The tray needs to be drawn in tray window,
        //      so the rectangle is in wrong coordinates.
        RECT rcTray;
        CopyRect(&rcTray, &m_rc);
        MapWindowPoints(hwndTaskBar, hwndTray, (LPPOINT)&rcTray, 2);

        DrawIconEx(hdc, rcTray.left, rcTray.top + yOffset, m_hIcon,16, 16, 0, NULL, DI_NORMAL);
    }

    return TRUE;
}

