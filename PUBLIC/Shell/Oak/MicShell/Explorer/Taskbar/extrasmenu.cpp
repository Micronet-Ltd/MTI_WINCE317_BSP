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
#include "ExtrasMenu.h"
#include "..\inc\resource.h"
#include "taskbarappbutton.h"

extern HINSTANCE g_hInstance;
extern CTaskBar *g_TaskBar;

#define MAX_TEXT_WIDTH      200
#define DT_END_ELLIPSIS 0x00008000
#define CX_CASCADE_WIDGET   8

#define CCH_EXTRAS 84
struct ExtrasMenuData
{
        WCHAR szDisplay[CCH_EXTRAS];
        HICON  hicon;
        HWND   hwndApp;
        DWORD index;
        WORD   width;
        WORD   height;

        ExtrasMenuData() {
                szDisplay[0] = 0;
                hicon = NULL;
                hwndApp = NULL;
                index = 0;
                width = 0;
                height = 0;
        }

        ~ExtrasMenuData()
        {
        }
};

ExtrasMenuButton::ExtrasMenuButton(HICON hIcon, UINT uFlags, RECT inRect, HWND hwnd, LPWSTR pwszText) :
    TaskbarMenuButton(hIcon, uFlags, inRect, hwnd, pwszText)
{
    ASSERT(g_hInstance);
    m_hIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_SPINNER), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
}

ExtrasMenuButton::ExtrasMenuButton(RECT inRect) :
    TaskbarMenuButton(NULL, HHTBS_BUTTON, inRect, HHTB_EXTRA, TEXT(""))
{
    ASSERT(g_hInstance);
    m_hIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_SPINNER), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
}

ExtrasMenuButton::~ExtrasMenuButton()
{
    if (m_hIcon)
    {
        DestroyIcon(m_hIcon);
    }
}

int ExtrasMenuButton::GetMyIndex()
{
    if (!g_TaskBar)
    {
        return -1;
    }

    return g_TaskBar->FindItemIndex(g_TaskBar->GetWindow(), HHTB_EXTRA);
}

void ExtrasMenuButton::LaunchMenu(HWND hwndTaskBar, POINT pt)
{
    MENUITEMINFO mii;
    ExtrasMenuData *pData = NULL;
    HMENU hmenuExtras;
    HMENU hmenu;
    HDC hdc;

    if (!hwndTaskBar)
    {
        return;
    }

    if (!m_fInMenu)
    {
        ClientToScreen(hwndTaskBar, &pt);

        hmenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDM_CONTEXT_TASKBAR_EXTRAS));
        if (!hmenu)
        {
            return;
        }

        hmenuExtras = GetSubMenu(hmenu, 0);
        if (!hmenuExtras)
        {
            return;
        }

        m_fInMenu = TRUE;
        hdc = GetDC(hwndTaskBar);
        DrawItem(hwndTaskBar, hdc, NULL, TRUE, TRUE);

        TrackPopupMenu(hmenuExtras, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x - 4, pt.y, 0, hwndTaskBar, NULL);

        DrawItem(hwndTaskBar, hdc, NULL, TRUE, FALSE);
        ReleaseDC(hwndTaskBar, hdc);
        //ForceButtonUp(m_hwndTaskBar, index);

        m_fInMenu = FALSE;

        // remove all items
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_DATA|MIIM_ID;

        while(GetMenuItemInfo(hmenuExtras, 0, MF_BYPOSITION, &mii))
        {
            DeleteMenu(hmenuExtras, 0, MF_BYPOSITION);
            if (mii.wID != IDM_TASKBAR_EXTRAS_PLACEHOLDER)
            {
                pData = (ExtrasMenuData *)mii.dwItemData;
                delete pData;
            }
        }

        DestroyMenu(hmenuExtras);
    }
    else
    {
        SetCapture(hwndTaskBar);
        ReleaseCapture();
    }
}

LRESULT ExtrasMenuButton::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!m_fInMenu && (msg != WM_COMMAND))
    {
        return 0;
    }

    switch(msg)
    {
        case WM_COMMAND:
            return ProcessCommand(hwnd, wParam, lParam);

        case WM_MEASUREITEM:
            return MeasureItem((MEASUREITEMSTRUCT *)lParam);

        case WM_DRAWITEM:
            return DrawMenuItem((DRAWITEMSTRUCT *)lParam);

        case WM_INITMENUPOPUP:
             return InitPopup((HMENU)wParam);
    }

    return 0;
}

LRESULT ExtrasMenuButton::ProcessCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    if (!g_TaskBar)
    {
        return 0;
    }

    int index = GET_WM_COMMAND_ID(wParam, lParam) - IDM_EXTRAS_MENU_FIRST__;

    if (index >= (g_TaskBar->GetFirstVisibleApplicationIndex() + g_TaskBar->GetApplicationCount()))
    {
        return 0;
    }

    if (GET_WM_COMMAND_ID(wParam, lParam) == IDM_DESKTOP)
    {
        g_TaskBar->CheckApp(hwnd, 0, TRUE);
        return 1;
    }
    else if (index > 0)
    {
        return g_TaskBar->CheckApp(hwnd, index, TRUE);
    }

    return 0;
}

LRESULT ExtrasMenuButton::InitPopup(HMENU hmenu)
{
    if (!g_TaskBar)
    {
        return 0;
    }

    ExtrasMenuData *pData = NULL;
    int cchLen = 0;
    int pos = 0;
    int id = IDM_EXTRAS_MENU_FIRST__ + g_TaskBar->GetApplicationCount() + g_TaskBar->GetFirstVisibleApplicationIndex() - 1;  // start at the end
    MENUITEMINFO mii;
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_DATA|MIIM_ID;
    HRESULT hr = S_OK;
    TaskbarApplicationButton *app = NULL;

    // remove all items
    while(GetMenuItemInfo(hmenu, 0, MF_BYPOSITION, &mii))
    {
        DeleteMenu(hmenu, 0, MF_BYPOSITION);
        if (mii.wID != IDM_TASKBAR_EXTRAS_PLACEHOLDER)
        {
            pData = (ExtrasMenuData *)mii.dwItemData;
            delete pData;
        }
    }

    // populate with current processes
    // get the 'last' app and go backwards (so that the top of the menu is the last process)
    app = g_TaskBar->GetPrevAppButton(NULL);

    while(app)
    {
        // a visible rectangle means it's displayed on the taskbar
        // skip it
        if (app->m_rc.right > app->m_rc.left) goto Skip;

        // fill in the menu data for this item
        pData = new ExtrasMenuData;
        if (pData)
        {
            pData->hicon = app->m_hIcon;
            pData->hwndApp = app->m_hwndMain;
            cchLen = wcslen(app->m_wszItem);
            hr = StringCchCopy(pData->szDisplay, CCH_EXTRAS-3, app->m_wszItem);

            if ((hr == S_OK) && (cchLen >= CCH_EXTRAS-3))
            {
                hr = StringCchCat(pData->szDisplay, CCH_EXTRAS, c_szEllipses);
                if (hr != S_OK)
                {
                    // revert the string
                    hr = StringCchCopy(pData->szDisplay, CCH_EXTRAS-3, app->m_wszItem);
                }
            }

            if ( hr == S_OK )
            {
                InsertMenu(hmenu, pos++, MF_BYPOSITION | MF_OWNERDRAW, id--, (LPCTSTR)pData);
            }
            else
            {
                delete pData;
            }
            pData = NULL; // the menu will own it
        }
Skip:
        // back up
        app = g_TaskBar->GetPrevAppButton(app);
    }

    if (!g_TaskBar->m_fShowDesktopButton)
    {
        LPTBOBJ ptbobj = g_TaskBar->GetItem(g_TaskBar->GetWindow(), (int)HHTB_DESKTOP);
        pData = new ExtrasMenuData;
        if (pData)
        {
            pData->hicon = ptbobj->ptbi->m_hIcon;
            pData->hwndApp = ptbobj->ptbi->m_hwndMain;
            LoadString(g_hInstance, IDS_DESKTOP, pData->szDisplay, CCH_EXTRAS);

            InsertMenu(hmenu, pos++, MF_BYPOSITION | MF_OWNERDRAW, IDM_DESKTOP, (LPCTSTR)pData);
            pData = NULL; // the menu will own it
        }
    }

    return TRUE;
}

LRESULT ExtrasMenuButton::DrawMenuItem(DRAWITEMSTRUCT *lpdis)
{
    ExtrasMenuData *pData;
    RECT rc;

    if (!lpdis)
    {
        return FALSE;
    }

    pData = (ExtrasMenuData *)lpdis->itemData;
    if (!pData)
    {
        return FALSE;
    }

    rc = lpdis->rcItem;

    switch (lpdis->itemAction)
    {
        case ODA_DRAWENTIRE:
        case ODA_SELECT:
            if ((lpdis->itemState & ODS_SELECTED) && lpdis->itemID)
            {
                HBRUSH hbrush = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
                SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
                FillRect(lpdis->hDC, &rc, hbrush);
                DeleteObject((HGDIOBJ)hbrush);
            }
            else
            {
                HBRUSH hbrush = CreateSolidBrush(GetSysColor(COLOR_MENU));
                SetTextColor(lpdis->hDC, GetSysColor(COLOR_MENUTEXT));
                FillRect(lpdis->hDC, &rc, hbrush);
                DeleteObject((HGDIOBJ)hbrush);
            }
            break;

        default:
            break;
    }

    RECT rcT = rc;

    if (lpdis->itemID)
    {
        if (pData->hicon)
        {
            DrawIconEx(lpdis->hDC, rc.left + 4, rc.top + 3, pData->hicon, 16, 16, 0, NULL, DI_NORMAL);
        }
        rcT.left += 24; // skip the space, icon, space

        DrawText(
                        lpdis->hDC,
                        pData->szDisplay,
                        wcslen(pData->szDisplay),
                        &rcT,
                        DT_VCENTER | DT_SINGLELINE | DT_NOCLIP | DT_NOPREFIX | DT_END_ELLIPSIS);
    }

    return TRUE;
}

LRESULT ExtrasMenuButton::MeasureItem(MEASUREITEMSTRUCT *lpmis)
{
    ExtrasMenuData *pData;
    SIZE size;
    WCHAR *pwszDisplay;

    pData = (ExtrasMenuData *)lpmis->itemData;

    if (pData->width && pData->height)
    {
        // already measured
        lpmis->itemHeight = pData->height;
        lpmis->itemWidth = pData->width;
        return TRUE;
    }

    size.cx = 94;
    size.cy = 0;

    if (lpmis->itemID)
    {
        // Height of 16 for icon + 3 pixels top & bottom
        lpmis->itemHeight = GetSystemMetrics(SM_CYSMICON) + 6;
    }
    else
    {
        // This means we have a separator
        lpmis->itemHeight = 7;
    }

    pwszDisplay = pData->szDisplay;

    HDC hdc = GetDC(NULL);
    GetTextExtentExPoint(hdc, pwszDisplay, wcslen(pwszDisplay), MAX_TEXT_WIDTH, NULL, NULL, &size);
    if (size.cx > MAX_TEXT_WIDTH)
    {
        size.cx = MAX_TEXT_WIDTH;
    }

    ReleaseDC(NULL, hdc);

    // reset height if need be
    if (size.cy > GetSystemMetrics(SM_CYSMICON))
    {
        lpmis->itemHeight = size.cy + 10;
    }

    // 4 pixel border + icon width + 4 pixel + text length + 8 pixels
    lpmis->itemWidth  = size.cx + GetSystemMetrics(SM_CXSMICON) + 8;

    pData->height = lpmis->itemHeight;
    pData->width = lpmis->itemWidth;

    return TRUE;
}

BOOL ExtrasMenuButton::GetTooltipText(LPWSTR pwszTT, int cchTT)
{
    if (pwszTT && (cchTT > 2))
    {
        LoadString(g_hInstance, IDS_APP_BUTTON_TT, pwszTT, cchTT - 1);
        return TRUE;
    }

    return FALSE;
}

