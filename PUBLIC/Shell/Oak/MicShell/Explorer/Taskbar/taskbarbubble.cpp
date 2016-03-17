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
#include "taskbarbubble.h"
#include "utils.h"

TaskbarBubble::TaskbarBubble() : TaskbarNotification()
{
    m_uID = 0;
    m_csDuration = 0;
    memset(&m_npPriority, 0, sizeof(SHNP));
    m_grfFlags = 0;
    memset(&m_clsid, 0, sizeof(CLSID));
    m_lParam = 0;
    m_uiStartTime = 0;
    m_bWaitingForShow = FALSE;
    m_pwszHTML = NULL;
}

TaskbarBubble::TaskbarBubble(PSHNOTIFICATIONDATA pND) : TaskbarNotification()
{
    m_hIcon = CopyIcon(pND->hicon);
    m_uFlags = HHTBS_NOTIFY | HHTBS_BUBBLE_NOTIFY;
    m_hwndMain = pND->hwndSink;
    m_uID = pND->dwID;
    m_csDuration = pND->csDuration;
    m_npPriority = pND->npPriority;
    m_grfFlags = pND->grfFlags;
    m_clsid = pND->clsid;
    m_lParam = pND->lParam;
    m_pwszHTML = NULL;
    m_bWaitingForShow = FALSE;
    m_uiStartTime = 0;
    SetRect(&m_rc, 0, 0, 0, 0);

    if (SHNUM_TITLE & pND->grfFlags)
    {
        if (StringCbCopy((LPWSTR) m_wszItem, CCHMAXTBLABEL * sizeof(WCHAR), pND->pszTitle) != S_OK)
        {
            m_wszItem[0] = L'\0';
        }
    }

    if (SHNUM_HTML & pND->grfFlags)
    {
        DWORD dwLen = (_tcslen(pND->pszHTML) + 1) * sizeof(WCHAR);
        m_pwszHTML = (LPCWSTR) LocalAlloc(LMEM_FIXED, dwLen);
        if ((m_pwszHTML != NULL) && (StringCbCopy((LPWSTR) m_pwszHTML, dwLen, pND->pszHTML) != S_OK))
        {
            LocalFree((HLOCAL) m_pwszHTML);
            m_pwszHTML = NULL;
        }
    }
}

TaskbarBubble::~TaskbarBubble()
{
    if (m_pwszHTML)
    {
        LocalFree((HLOCAL) m_pwszHTML);
    }
}

