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

#include "filemenu.h"
#include "mainwnd.h"
#include "utils.h"

extern const TBBUTTON s_tbBtns[] =
{
    {0, ID_GO_BACK,            TBSTATE_ENABLED, 0,  { 0,0 } , IDS_IE_TTSTR,  0},
    {3, ID_VIEW_REFRESH,            TBSTATE_ENABLED, 0,   {0,0}, IDS_IE_TTSTR3,  0},
    {4, ID_GO_HOME,       TBSTATE_ENABLED, 0, {0,0}, IDS_IE_TTSTR4,  0},
    {5, ID_FAV_ADD,   TBSTATE_ENABLED, 0,  {0,0},  IDS_IE_TTSTR6,  0},
};

extern const int s_iBtns = sizeof(s_tbBtns)/sizeof(TBBUTTON);
extern const UINT s_idrMenu = IDR_MAINMENU_PIE;

extern HINSTANCE   g_hInstance;
extern BOOL        g_bQVGAEnabled;

LRESULT CALLBACK PropertiesDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);


HRESULT CMainWnd::FindString()
{
    return E_NOTIMPL;
}



BOOL CMainWnd::HandleMenuPopup(WPARAM wParam, LPARAM lParam)
{
    HMENU hmenu = CommandBar_GetMenu(_hWndMenuBar, 0);
    HMENU hSubMenu;
    int id = 0, iMenuCount = 0;

    OLECMD rgcmd[] = {
        { OLECMDID_CUT, 0 },
        { OLECMDID_COPY, 0 },
        { OLECMDID_PASTE, 0 }
    };

    IOleCommandTarget* pcmdt;
    iMenuCount = GetMenuItemCount(hmenu);

    // need to check for all the commands which do not belong to explorer
    HandleSharedMenuPopup((HMENU)wParam);

    hSubMenu = GetSubMenu(hmenu, 0);
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

        GetMenuItemInfo((HMENU)wParam, 0, TRUE, &mii);
        if (mii.wID == ID_GO_FORWARD)
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

                    GetMenuItemInfo((HMENU)wParam, 8, TRUE, &mii);

                    HMENU hmenuOldFonts = GetSubMenu((HMENU)wParam, 8);
                    RemoveMenu((HMENU)wParam, 8, MF_BYPOSITION);
                    DestroyMenu(hmenuOldFonts);
                    InsertMenu((HMENU)wParam, 8, MF_POPUP|MF_BYPOSITION, (UINT)hmenuFonts, szTitle);
                }

                //Check if fit to screen is enabled
                hr = pcmdt->Exec(&CGID_ShellDocView, OLECMDID_FITTOSCREEN, 0, NULL, &v);
                if (SUCCEEDED(hr) && v.vt == VT_BOOL)
                {
                    CheckMenuItem((HMENU)wParam, IDM_FITTOSCREEN, MF_BYCOMMAND | ( (v.boolVal == VARIANT_TRUE) ? MF_CHECKED : MF_UNCHECKED) );
                }

                //Check if images are shown
                hr = pcmdt->Exec(&CGID_ShellDocView, IDM_SHOWPICTURES, 0, NULL, &v);
                if (SUCCEEDED(hr) && v.vt == VT_BOOL)
                {
                    CheckMenuItem((HMENU)wParam, IDM_SHOWPICTURES, MF_BYCOMMAND | ( (v.boolVal == VARIANT_TRUE) ? MF_CHECKED : MF_UNCHECKED) );
                }

                pcmdt->Release();
            }
        }

        goto Exit;
    }

    hSubMenu = GetSubMenu(hmenu, 1);
    // Favorites/edit commands
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
    }

    if (g_bQVGAEnabled && iMenuCount > 2)
    {
        hSubMenu = GetSubMenu(hmenu, iMenuCount-1);
    }

    if (hSubMenu == (HMENU)wParam)
    {
        // favorites 2-level submenu
        HMENU hmenuFav = GetSubMenu((HMENU)wParam, 0);
        if (_Favorites && hmenuFav)
        {
            // make sure this is really the favorites menu
            id = GetMenuItemID((HMENU)hmenuFav, 0);
            if (id == ID_FAV_PLACEHOLDER || id == ID_FAV_ADD)
            {
                _Favorites->CreateFavoritesMenu((HMENU)hmenuFav);
                _bInMenuLoop = TRUE;
            }
            goto Exit;
        }
    }

    // for a PocketIE UI, other commands could potentially be added as a shell menu
    if (iMenuCount > 2)
    {
        // View commands
        hSubMenu = GetSubMenu(hmenu, 2);
        if (hSubMenu == (HMENU)wParam)
        {
            CheckMenuItem((HMENU)wParam, ID_ADDRESSBAR_TOGGLE, MF_BYCOMMAND | ((_bShowAddrBar) ? MF_CHECKED : MF_UNCHECKED));
            CheckMenuItem((HMENU)wParam, IDM_STATUSBAR_TOGGLE, MF_BYCOMMAND | ((_bShowStatusBar) ? MF_CHECKED : MF_UNCHECKED));
        }

        // Favorites menu
        hSubMenu = GetSubMenu(hmenu, iMenuCount-1);
        if (_Favorites && (hSubMenu == (HMENU)wParam))
        {
            // make sure this is the favorites menu before changing its contents
            int idFavoritesMenu = GetMenuItemID((HMENU)wParam, 0);
            if (idFavoritesMenu == ID_FAV_PLACEHOLDER || idFavoritesMenu == ID_FAV_ADD)
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
    }

 Exit:
    return TRUE;
}

// called when the first download starts
void CMainWnd::EnterDownload()
{
    TBBUTTONINFO tbbi;
    if (_bShowCmdBar && _hWndToolBar)
    {
        tbbi.cbSize = sizeof(TBBUTTONINFO);
        tbbi.dwMask = TBIF_IMAGE|TBIF_COMMAND;
        tbbi.idCommand = ID_VIEW_STOP;
        tbbi.iImage = 2;
        SendMessage(_hWndToolBar, TB_SETBUTTONINFO, (WPARAM)ID_VIEW_REFRESH, (LPARAM)&tbbi);
    }
}

// called when the last download is finished
void CMainWnd::ExitDownload()
{
    TBBUTTONINFO tbbi;
    if (_bShowCmdBar && _hWndToolBar)
    {
        tbbi.cbSize = sizeof(TBBUTTONINFO);
        tbbi.dwMask = TBIF_IMAGE|TBIF_COMMAND;
        tbbi.idCommand = ID_VIEW_REFRESH;
        tbbi.iImage = 3;
        SendMessage(_hWndToolBar, TB_SETBUTTONINFO, (WPARAM)ID_VIEW_STOP, (LPARAM)&tbbi);
    }
}

UINT  CMainWnd::ExtHandleCommand(WPARAM wParam, LPARAM lParam)
{
    UINT nID = LOWORD(wParam);

    switch (nID)
    {
        case IDM_FILE_PROPERTIES:
            _pBrowser->AddRef();
            DialogBoxParam(g_hInstance,
                            MAKEINTRESOURCE(IDD_PAGEPROPERTIES),
                            _hWnd,
                            (DLGPROC)PropertiesDialogProc,
                            (LPARAM)_pBrowser);

            _pBrowser->Release();
        return 0;

        case IDM_SHOWPICTURES:
        case IDM_FITTOSCREEN:
        {
            HMENU hmenu = CommandBar_GetMenu(_hWndMenuBar, 0);
            HMENU hSubMenu = GetSubMenu(hmenu, 0);

            if (hSubMenu)
            {
                IOleCommandTarget* pcmdt;

                if (SUCCEEDED(_pBrowser->QueryInterface(IID_IOleCommandTarget, (LPVOID*) & pcmdt)))
                {
                    MENUITEMINFO mii;
                    mii.cbSize = sizeof(mii);
                    mii.fMask = MIIM_STATE;

                    GetMenuItemInfo(hSubMenu, nID, FALSE, &mii);

                    VARIANTARG v;
                    v.vt = VT_BOOL;
                    v.boolVal = (mii.fState == MFS_CHECKED) ? VARIANT_FALSE : VARIANT_TRUE;

                    DWORD cmdID = (nID == IDM_FITTOSCREEN) ? OLECMDID_FITTOSCREEN : IDM_SHOWPICTURES;

                    if (SUCCEEDED(pcmdt->Exec(&CGID_ShellDocView, cmdID, 0, &v, NULL)))
                    {
                        CheckMenuItem(hSubMenu, nID, MF_BYCOMMAND | ( (v.boolVal == VARIANT_TRUE) ? MF_CHECKED : MF_UNCHECKED) );
                    }

                    pcmdt->Release();
                }
            }
            return 0;
        }

        default:
            break;

    }
    return 1;
}


LRESULT CALLBACK PropertiesDialogProc
(
    HWND hDlg,
    UINT message,
    UINT wParam,
    LONG lParam
)
{

    // Start of the *real* message processing
    switch (message)
    {
        case WM_INITDIALOG:
        {
            // Do the Done button thing, and size for me please (fullscreen)
#if 0 // TODO
            SHINITDLGINFO shidi = { 0 };
            shidi.dwMask    = SHIDIM_FLAGS;
            shidi.dwFlags   = SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN;
            shidi.hDlg      = hDlg;
            SHInitDialog(&shidi);
            SetFocus(GetDlgItem(hDlg, IDC_TITLE));
#endif

            IWebBrowser2 *pBrowser = (IWebBrowser2 *)lParam;

            // Strings...
            WCHAR *szTemp = (LPWSTR)LoadString(g_hInstance, IDS_UNKNOWN, NULL, 0);

            int nID [] = {IDC_PROP_TITLE, IDC_PROP_PROTOCOL, IDC_PROP_TYPE,
                          IDC_PROP_ITEMSIZE, IDC_PROP_SECURITY, IDC_PROP_ADDRESS};

            if (szTemp)
            {
                for (int ix = 0; ix < ARRAYSIZE(nID); ix++)
                {
                    SetDlgItemText(hDlg, nID[ix], szTemp);
                }
            }

            IOleCommandTarget *pcmdt = NULL;
            VARIANTARG v = {0};

            if (pBrowser &&
                SUCCEEDED(pBrowser->QueryInterface(IID_IOleCommandTarget, (void**)&pcmdt)) &&
                SUCCEEDED(pcmdt->Exec(&CGID_ShellDocView, OLECMDID_PROPERTIES, 0, NULL, &v)))
            {
                if (v.vt == (VT_ARRAY | VT_BSTR) &&
                    v.parray &&
                    v.parray->rgsabound[0].cElements >= ARRAYSIZE(nID))
                {
                    BSTR *pbstr;

                    if (SUCCEEDED(SafeArrayAccessData(v.parray, (void HUGEP **)&pbstr)))
                    {
                        for (int ix = 0; ix < ARRAYSIZE(nID); ix++)
                        {
                            if (pbstr[ix])
                            {
                                SetDlgItemText(hDlg, nID[ix], pbstr[ix]);
                            }
                        }
                        SafeArrayUnaccessData(v.parray);
                    }
                }

                VariantClear(&v);
            }

            if (pcmdt)
            {
                pcmdt->Release();
            }

            // Don't let Windows set focus on the edit box...
            return FALSE;
        }

        case WM_SIZE:
#if 0
            // Now for rearranging the child windows...
            // Note that, yes, the following code does implicitly REQUIRE a
            // Left-To-Right reading order.
            HDC hdc;
            HWND hwndLabelCtrl;
            HWND hwndNameCtrl;
            hdc = GetDC(hDlg);
            if (!hdc)
            {
                break;
            }

            // REVIEW: BeginDefer/EndDeferWindowPos? [jaym]

            hwndLabelCtrl = GetDlgItem(hDlg, IDC_PROTOCOL_LABEL);
            hwndNameCtrl = GetDlgItem(hDlg, IDC_PROTOCOL);
            if (hwndLabelCtrl && hwndNameCtrl)
            {
                PositionLabeledTextWindows(hdc, hwndLabelCtrl, hwndNameCtrl);
            }
            hwndLabelCtrl = GetDlgItem(hDlg, IDC_TYPE_LABEL);
            hwndNameCtrl = GetDlgItem(hDlg, IDC_TYPE);
            if (hwndLabelCtrl && hwndNameCtrl)
            {
                PositionLabeledTextWindows(hdc, hwndLabelCtrl, hwndNameCtrl);
            }
            hwndLabelCtrl = GetDlgItem(hDlg, IDC_ITEMSIZE_LABEL);
            hwndNameCtrl = GetDlgItem(hDlg, IDC_ITEMSIZE);
            if (hwndLabelCtrl && hwndNameCtrl)
            {
                PositionLabeledTextWindows(hdc, hwndLabelCtrl, hwndNameCtrl);
            }
            hwndLabelCtrl = GetDlgItem(hDlg, IDC_SECURITY_LABEL);
            hwndNameCtrl = GetDlgItem(hDlg, IDC_SECURITY);
            if (hwndLabelCtrl && hwndNameCtrl)
            {
                PositionLabeledTextWindows(hdc, hwndLabelCtrl, hwndNameCtrl);
            }

            // For the Address EditField, we need to do a little extra work
            hwndLabelCtrl = GetDlgItem(hDlg, IDC_ADDRESS_LABEL);
            hwndNameCtrl = GetDlgItem(hDlg, IDC_ADDRESS);
            if (hwndLabelCtrl && hwndNameCtrl)
            {
                PositionLabeledTextWindows(hdc, hwndLabelCtrl, hwndNameCtrl, FALSE);
                RECT rc = {0};
                GetWindowRect(hwndNameCtrl, &rc);
                SetWindowPos(hwndNameCtrl, NULL, 0, 0,
                                LOWORD(lParam) - rc.left -4, // 4 =right spacing
                                rc.bottom - rc.top,
                                SWP_NOMOVE | SWP_NOZORDER);
            }

            // Release the HDC
            ReleaseDC(hDlg, hdc);

            // Always break or return...
            break;

        case WM_HELP:
            {
                LaunchHelp(IDS_HELP_PROPERTIES);
                break;
            }

        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc;

                hdc = BeginPaint(hDlg, &ps);
                if (hdc != NULL)
                {
                    SHHdrGrpSepLineDraw(hDlg, NULL, hdc, SHD_HEADER);
                }

                EndPaint(hDlg, &ps);
                break;
            }

        case WM_DESTROY:
            // Free the font manager object.
            if (s_pvFontManager)
            {
                SHFontMgrDestroy(s_pvFontManager);
                s_pvFontManager = NULL;
            }
            break;

        case WMP_FORCECLOSE:
            // Force this dialog to close without doing anything.
            // This is how we signal that nothing should happen...
            EndDialog(hDlg, TRUE);
            break;
#endif

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hDlg, TRUE);
                break;
            }
            break;

        default:
            break;
    }

    return FALSE;
} // PropertiesDialogProc()


