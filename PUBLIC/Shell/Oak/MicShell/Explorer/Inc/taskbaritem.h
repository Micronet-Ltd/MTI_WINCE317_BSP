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

#ifndef _TASKBARITEM_H_
#define _TASKBARITEM_H_

class TaskbarItem
{
public:
    HICON m_hIcon;
    UINT m_uFlags;
    RECT m_rc;
    HWND m_hwndMain;
    WCHAR m_wszItem[MAX_PATH];

    TaskbarItem();
    TaskbarItem(HICON hIcon, UINT uFlags, RECT inRect, HWND hwnd, LPWSTR pwszText);
    virtual ~TaskbarItem();

    void CopyFrom(TaskbarItem *pItem);
    virtual BOOL DrawItem(HWND hwndTaskbar, HDC hdc, HFONT hFont, BOOL bSmallScreen, BOOL bPressed);
    void Animate(HWND hwnd, BOOL bMaximize);
    virtual BOOL GetTooltipText(LPWSTR pwszTT, int cchTT);
};
#endif //_TASKBARITEM_H_

