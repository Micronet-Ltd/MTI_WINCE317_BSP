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
//
#include "windows.h"
#include <exdisp.h>
#include <exdispid.h>
#include <objbase.h>
#include "resource.h"
#include <wininet.h>

#include <commctrl.h>
#include <commdlg.h>

#include "filemenu.h"
#include "mainwnd.h"
#include "utils.h"

extern const TBBUTTON s_tbBtns[] =
{  {0, ID_GO_BACK, TBSTATE_ENABLED, 0, {0,0}, IDS_IE_TTSTR, 0},
    {1, ID_GO_FORWARD, TBSTATE_ENABLED, 0, {0,0},IDS_IE_TTSTR1, 0},
    {2, ID_VIEW_STOP, TBSTATE_ENABLED, 0, {0,0},IDS_IE_TTSTR2, 0},
    {3, ID_VIEW_REFRESH, TBSTATE_ENABLED, 0, {0,0},IDS_IE_TTSTR3, 0},
    {0, IDC_STATIC, 0, TBSTYLE_SEP, 0, 0, 0, -1},
    {4, ID_GO_HOME, TBSTATE_ENABLED, 0, {0,0},IDS_IE_TTSTR4, 0},
    {7, ID_GO_SEARCH, TBSTATE_ENABLED, 0, {0,0}, IDS_IE_TTSTR5, 0},
};

extern const int s_iBtns = sizeof(s_tbBtns)/sizeof(TBBUTTON);
extern const UINT s_idrMenu = IDR_MAINMENU;

extern BOOL        g_bQVGAEnabled;

HRESULT CMainWnd::FindString()
{
    LPDISPATCH pDisp = NULL;
    LPOLECOMMANDTARGET pCmdTarg = NULL;
    HRESULT sts = S_OK;
    VARIANTARG var;

#define GOERROR_S(bNotCond)  {if (!(bNotCond)) goto errorS; }

    if (!_pBrowser)
    {
        return S_OK;
    }

    sts = _pBrowser->get_Document(&pDisp);
    GOERROR_S(pDisp);
    sts = pDisp->QueryInterface(IID_IOleCommandTarget, (LPVOID*)&pCmdTarg);
    GOERROR_S(pCmdTarg);

    var.vt   = VT_I4;
    var.lVal = 0;
    sts = pCmdTarg->Exec(
                        NULL,
                        OLECMDID_FIND,
                        MSOCMDEXECOPT_PROMPTUSER,
                        NULL,
                        &var);

    errorS:
    if (pCmdTarg)
    {
        pCmdTarg->Release(); // release document's command target
    }

    if (pDisp)
    {
        pDisp->Release();    // release document's dispatch interface
    }
    return sts;
}


BOOL CMainWnd::HandleMenuPopup(WPARAM wParam, LPARAM lParam)
{
    HMENU hmenu = CommandBar_GetMenu(_hWndMenuBar, 0);
    HMENU hSubMenu;

    OLECMD rgcmd[] = {
        { OLECMDID_CUT, 0 },
        { OLECMDID_COPY, 0 },
        { OLECMDID_PASTE, 0 }
    };

    IOleCommandTarget* pcmdt;
    // check/uncheck the offline state
    int iMenuCount = GetMenuItemCount(hmenu);

    // for WM_INITMENUPOPUP

    // need to check for all the commands which do not belong to explorer
    HandleSharedMenuPopup((HMENU)wParam);

    hSubMenu = GetSubMenu(hmenu, 1);
    if (hSubMenu == (HMENU)wParam)
    {
        // enable / disable edit menu commands
        if (GetFocus()==_hWndAddressEdit)
        {
            DWORD dwStart, dwEnd;

            SendMessage(_hWndAddressEdit, EM_GETSEL, (WPARAM)(&dwStart), (LPARAM)(&dwEnd));
            if (dwEnd - dwStart >= 1)
            {
                EnableMenuItem((HMENU)wParam, ID_EDIT_CUT, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem((HMENU)wParam, ID_EDIT_COPY, MF_BYCOMMAND | MF_ENABLED);
            }
            else
            {
                EnableMenuItem((HMENU)wParam, ID_EDIT_CUT, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem((HMENU)wParam, ID_EDIT_COPY, MF_BYCOMMAND | MF_GRAYED);
            }
            if (IsClipboardFormatAvailable(CF_UNICODETEXT) || IsClipboardFormatAvailable(CF_TEXT))
            {
                EnableMenuItem((HMENU)wParam, ID_EDIT_PASTE, MF_BYCOMMAND | MF_ENABLED);
            }
            else
            {
                EnableMenuItem((HMENU)wParam, ID_EDIT_PASTE, MF_BYCOMMAND | MF_GRAYED);
            }
        }
        else
        {
            if (!_pBrowser)
            {
                goto Exit;
            }

            if (SUCCEEDED(_pBrowser->QueryInterface(IID_IOleCommandTarget, (LPVOID*) & pcmdt)))
            {
                pcmdt->QueryStatus(NULL, sizeof(rgcmd)/sizeof(OLECMD), rgcmd, NULL);
                pcmdt->Release();
            }
            for (int i=0; i<sizeof(rgcmd)/sizeof(OLECMD); i++)
            {
                EnableMenuItem((HMENU)wParam, ID_EDIT_CUT + i, rgcmd[i].cmdf & OLECMDF_ENABLED ?
                    (MF_BYCOMMAND | MF_ENABLED) : ( MF_BYCOMMAND| MF_GRAYED));
            }
        }

        goto Exit;
    }

    hSubMenu = GetSubMenu(hmenu, 2);
    // provide encoding menu
    if (hSubMenu == (HMENU)wParam)
    {
        CheckMenuItem((HMENU)wParam, ID_ADDRESSBAR_TOGGLE, MF_BYCOMMAND | ((_bShowAddrBar) ? MF_CHECKED : MF_UNCHECKED));
        CheckMenuItem((HMENU)wParam, IDM_STATUSBAR_TOGGLE, MF_BYCOMMAND | ((_bShowStatusBar) ? MF_CHECKED : MF_UNCHECKED));

        if (!_pBrowser)
        {
            goto Exit;
        }

        // if this is not the view menu, bail out early
        MENUITEMINFO mii;
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_ID;

        GetMenuItemInfo((HMENU)wParam, 3, TRUE, &mii);
        if (mii.wID == ID_VIEW_STOP)
        {
            if (SUCCEEDED(_pBrowser->QueryInterface(IID_IOleCommandTarget, (LPVOID*) & pcmdt)))
            {
                VARIANTARG v = {0};
                HRESULT hr;

                hr = pcmdt->Exec(&CGID_ShellDocView, 27/*SHDVID_GETMIMECSETMENU*/, 0, NULL, &v);
                if (S_OK == hr)
                {
                    UINT uItem = 0;
                    HMENU hmenuFonts = (HMENU)v.lVal;
                    WCHAR szTitle[50];

                    mii.fMask = MIIM_TYPE;
                    mii.fType = MFT_STRING;
                    mii.dwTypeData = szTitle;
                    mii.cch = 50;

                    GetMenuItemInfo((HMENU)wParam, 1, TRUE, &mii);

                    HMENU hmenuOldFonts = GetSubMenu((HMENU)wParam, 1);
                    RemoveMenu((HMENU)wParam, 1, MF_BYPOSITION);
                    DestroyMenu(hmenuOldFonts);
                    InsertMenu((HMENU)wParam, 1, MF_POPUP|MF_BYPOSITION, (UINT)hmenuFonts, szTitle);
                }

                pcmdt->Release();
            }
        }

        goto Exit;
    }

    if (g_bQVGAEnabled)
    {
        hSubMenu = GetSubMenu(hmenu, iMenuCount-1);
        if (hSubMenu == (HMENU)wParam)
        {
            // favorites 2-level submenu
            HMENU hmenuFav = GetSubMenu((HMENU)wParam, 0);
            if (_Favorites && hmenuFav)
            {
                // make sure this is really the favorites menu
                int id = GetMenuItemID((HMENU)hmenuFav, 0);
                if (id == ID_FAV_PLACEHOLDER || id == ID_FAV_ADD)
                {
                    _Favorites->CreateFavoritesMenu((HMENU)hmenuFav);
                    _bInMenuLoop = TRUE;
                }
                goto Exit;
            }
        }
    }

    hSubMenu = GetSubMenu(hmenu, iMenuCount-1);
    // Favorites
    if (_Favorites && (hSubMenu == (HMENU)wParam))
    {
        // make sure this is the favorites menu before changing its contents
        int id = GetMenuItemID((HMENU)wParam, 0);
        if (id == ID_FAV_PLACEHOLDER || id == ID_FAV_ADD)
        {
            _Favorites->CreateFavoritesMenu((HMENU)wParam);
            _bInMenuLoop = TRUE;
        }
    }
    else
    {
        if (_bInMenuLoop)
        {
            PREFAST_ASSERT(_Favorites);
            _Favorites->HandleInitMenuPopup((HMENU)wParam);
        }
    }

 Exit:
    return TRUE;
}


// called when the first download starts
void CMainWnd::EnterDownload()
{
    return;
}

// called when the last download is finished
void CMainWnd::ExitDownload()
{
    return;
}

// return 1 if the command is handled here
UINT  CMainWnd::ExtHandleCommand(WPARAM wParam, LPARAM lParam)
{
    UINT nID = LOWORD(wParam);
    switch (nID)
    {
#if 0
        case ID_FILE_PRINT:
        {
            _pBrowser->ExecWB(OLECMDID_PRINT, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
            return 0;
        }

        case ID_FILE_PAGESETUP:
        {
            HandlePageSetup(_hWnd);
            return 0;
        }
#endif
        case ID_FILE_FIND:
            FindString();
            return 0;

        default:
            break;
    }
    return 1;
}



