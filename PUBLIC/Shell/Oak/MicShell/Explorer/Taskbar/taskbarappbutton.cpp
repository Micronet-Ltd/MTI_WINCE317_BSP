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
#include "taskbarappbutton.h"
#include "taskbar.hxx"
#include "utils.h"

// pull in from coredll
extern "C" int GetWindowTextWDirect(HWND, LPWSTR, int);

TaskbarApplicationButton::TaskbarApplicationButton() :
    TaskbarButton()
{
    m_bHibernate = FALSE;
    m_bMarkedFullscreen = FALSE;
    m_bMarkedTopmost = FALSE;
}

TaskbarApplicationButton::TaskbarApplicationButton(HWND hwnd) :
    TaskbarButton()
{
    m_hwndMain = hwnd;
    m_bHibernate = FALSE;
    m_bMarkedFullscreen = FALSE;
    m_bMarkedTopmost = FALSE;

    if (m_hwndMain)
    {
        m_bMarkedTopmost = ((GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0);

        if (!GetWindowTextWDirect(m_hwndMain, m_wszItem, ARRAYSIZE(m_wszItem)))
        {
            //DEBUGMSG(ZONE_VERBOSE, (TEXT("SHELLNOTIFY_WINDOWCREATED: No title\r\n")));
            m_wszItem[0] = L'\0';
        }

        m_uFlags = HHTBS_BUTTON;

        if (!::SendMessageTimeout(m_hwndMain, WM_GETICON, 0, 0, SMTO_NORMAL, 2000, (DWORD*)&m_hIcon))
        {
            m_hIcon = NULL;
        }
    }
}

TaskbarApplicationButton::~TaskbarApplicationButton()
{
}

