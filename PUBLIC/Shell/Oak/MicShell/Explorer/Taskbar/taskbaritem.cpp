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
#include "taskbaritem.h"
#include "taskbarhelpers.h"
#include "taskbar.hxx"
#include "utils.h"

extern HBRUSH g_hbrMonoDither;

TaskbarItem::TaskbarItem(HICON hIcon, UINT uFlags, RECT inRect, HWND hwnd, LPWSTR pwszText)
{
    m_hIcon = hIcon;
    m_uFlags = uFlags;
    m_rc = inRect;
    m_hwndMain = hwnd;
    StringCchCopy(m_wszItem, ARRAYSIZE(m_wszItem), pwszText);
}

TaskbarItem::TaskbarItem()
{
    m_hIcon = NULL;
    m_uFlags = 0;
    m_rc.top = m_rc.bottom = m_rc.left = m_rc.right = 0;
    m_hwndMain = NULL;
    m_wszItem[0] = L'\0';
}

TaskbarItem::~TaskbarItem()
{
}

void TaskbarItem::CopyFrom(TaskbarItem *pItem)
{
    if (pItem != this)
    {
        m_hIcon = pItem->m_hIcon;
        m_uFlags = pItem->m_uFlags;
        m_rc = pItem->m_rc;
        m_hwndMain = pItem->m_hwndMain;
        StringCchCopy(m_wszItem, ARRAYSIZE(m_wszItem), pItem->m_wszItem);
    }
}

BOOL TaskbarItem::DrawItem(HWND hwndTaskbar, HDC hdc, HFONT hFont, BOOL bIsSmallScreen, BOOL bPressed)
{
    // don't bother trying to draw it if it isn't supposed to be drawn
    if (m_rc.right < m_rc.left)
    {
        return TRUE;
    }

    int  cchText, cxEdge, xOffset = 1, yOffset = 1;
    WCHAR ach[CCHMAXTBLABEL + CCHELLIPSES];
    BOOL bNeedEllipses;
    HDC hdcMem;
    HBITMAP hbmMem;
    HGDIOBJ horg;
    RECT rcTemp;
    SIZE siz;
    COLORREF clrText, clrBack;
    HFONT hfontOld = NULL;
    UINT edge = bPressed ? EDGE_SUNKEN : EDGE_RAISED;
    HBRUSH hbr = GetSysColorBrush(COLOR_3DFACE);
    BOOL bForceEdge = (m_hwndMain == HHTB_START || m_hwndMain == HHTB_EXTRA || m_hwndMain == HHTB_SIP || bIsSmallScreen);

    if (m_uFlags & HHTBS_CHECKED)
    {
        if (bForceEdge)
        {
            hbr = g_hbrMonoDither;
        }
        edge = EDGE_SUNKEN;
    }

    cxEdge = GetSystemMetrics(SM_CXEDGE);
    CopyRect(&rcTemp, &m_rc);
    OffsetRect(&rcTemp, -rcTemp.left, -rcTemp.top);

    yOffset = (((rcTemp.bottom - rcTemp.top) - 16) / 2) - cxEdge;

    hdcMem = CreateCompatibleDC(hdc);

    hbmMem = CreateBitmap(rcTemp.right - rcTemp.left, rcTemp.bottom - rcTemp.top, 1,
                          GetDeviceCaps(hdc, BITSPIXEL),  NULL);

    horg = SelectObject(hdcMem, hbmMem);

    SelectObject(hdcMem, hbr);
    clrText = SetTextColor(hdcMem, GetSysColor(COLOR_BTNHIGHLIGHT)); // 0 -> 0
    clrBack = SetBkColor(hdcMem, GetSysColor(COLOR_BTNFACE));        // 1 -> 1

    // only draw the dither brush where the mask is 1's
    PatBlt(hdcMem, rcTemp.left, rcTemp.top, rcTemp.right - rcTemp.left, rcTemp.bottom - rcTemp.top, PATCOPY);
    if (!bIsSmallScreen || bForceEdge)
    {
        RECT rc2;
        GetClientRect(hwndTaskbar, &rc2);
        rc2.left = rcTemp.left;
        rc2.right = rcTemp.right;
        if (bIsSmallScreen && bForceEdge && (m_hwndMain == HHTB_START))
        {
            DrawEdge(hdcMem, &rc2, EDGE_ETCHED, BF_RIGHT);
        }
        else if (bIsSmallScreen && bForceEdge && (m_hwndMain == HHTB_EXTRA))
        {
            DrawEdge(hdcMem, &rc2, EDGE_ETCHED, BF_LEFT);
        }
        else if (bIsSmallScreen && bForceEdge && (m_hwndMain == HHTB_SIP))
        {
            DrawEdge(hdcMem, &rc2, EDGE_ETCHED, BF_LEFT );
        }
        else if (bIsSmallScreen)
        {
            DrawEdge(hdcMem, &rcTemp, EDGE_ETCHED, BF_RECT );
        }
        else
        {
            DrawEdge(hdcMem, &rcTemp, edge, BF_RECT | BF_SOFT);
        }
    }
    SetTextColor(hdcMem, clrText);
    SetBkColor(hdcMem, clrBack);

    InflateRect(&rcTemp, -cxEdge, -cxEdge);
    if (!bIsSmallScreen && edge == EDGE_SUNKEN)
    {
        xOffset += 1;
        yOffset += 2;
    }

    if (m_hIcon)
    {
        if (m_hIcon == (HICON)E_FAIL)
        {
            DebugBreak();
        }

        DrawIconEx(hdcMem,
                        (rcTemp.left + xOffset),
                        (rcTemp.top + yOffset),
                        m_hIcon,
                        16,
                        16,
                        0,
                        NULL,
                        DI_NORMAL);

        rcTemp.left += 18;
    }

    rcTemp.left += xOffset;
    rcTemp.top  += (yOffset - 2); //so that the text is not drawing too low!

    StringCchCopy(ach, ARRAYSIZE(ach), m_wszItem);

    if (hFont)
    {
        hfontOld = (HFONT)SelectObject(hdcMem, hFont);
    }

    GetTextExtentPoint(hdcMem, c_szEllipses, CCHELLIPSES, &siz);
    bNeedEllipses = TaskbarNeedsEllipses(hdcMem, m_wszItem, &rcTemp, &cchText, siz.cx);
    if (bNeedEllipses)
    {
        StringCchCopy((ach + cchText), (ARRAYSIZE(ach) - cchText), c_szEllipses);
        cchText += CCHELLIPSES;
    }
    else
    {
        if (FAILED(StringCchLength(ach, ARRAYSIZE(ach), (size_t*)&cchText)))
        {
            cchText = 0;
        }
    }

    SetTextColor(hdcMem, GetSysColor(COLOR_BTNTEXT) /*RGB(0,0,0)*/);
    SetBkMode(hdcMem, TRANSPARENT);
    DrawText(hdcMem,
                    ach,
                    cchText,
                    &rcTemp,
                    DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_VCENTER);

    if (hfontOld)
    {
        SelectObject(hdcMem, hfontOld);
    }
    CopyRect(&rcTemp, &m_rc);
    BitBlt(hdc, rcTemp.left, rcTemp.top, rcTemp.right - rcTemp.left, rcTemp.bottom - rcTemp.top, hdcMem, 0, 0, SRCCOPY);

    if ((m_hwndMain == HHTB_START) && (GetFocus() == hwndTaskbar))
    {
        InflateRect(&rcTemp, -3, -3);
        DrawFocusRect(hdc, &rcTemp);
    }

    SelectObject(hdcMem, horg);
    DeleteDC(hdcMem);
    DeleteObject(hbmMem);

    return TRUE;
}

void TaskbarItem::Animate(HWND hwnd, BOOL bMaximize)
{
    RECT rcTB;
    RECT rcButton;

    GetWindowRect (hwnd, &rcTB);
    rcButton = m_rc;
    rcButton.top    += rcTB.top;
    rcButton.bottom += rcTB.top;
    RectangleAnimation (m_hwndMain, &rcButton,  bMaximize);
}

BOOL TaskbarItem::GetTooltipText(LPWSTR pwszTT, int cchTT)
{
    if (m_wszItem[0] && pwszTT && (cchTT > 2))
    {
        StringCchCopy(pwszTT, cchTT, m_wszItem);
        return TRUE;
    }

    return FALSE;
}


