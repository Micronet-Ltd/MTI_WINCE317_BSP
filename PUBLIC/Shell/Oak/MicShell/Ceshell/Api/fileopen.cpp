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

#include "ceshpriv.h"
#include "fileopen.hpp"
#include "ceshlobj.h"
#include "guid.h"
#include <shdisp.h>
#include "shellcmds.h"
#include "resource.h"
#include "api.h"
#include "idlist.h"
#include "aygutils.h"
#include <shellsdk.h>
#include "shelldialogs.h"

#ifndef _PREFAST_
#pragma warning(disable:4068)
#endif // _PREFAST_

extern "C" HWND WINAPI CommandBar_InsertControl(HWND hwndCB,
                                                 HINSTANCE hInstance,
                                                 LPWSTR wszControlClass,
                                                 int  iWidth,
                                                 UINT dwStyle,
                                                 WORD idControl,
                                                 WORD iButton);

extern IMalloc * g_pShellMalloc;


//////////////////////////////////////////////////////////////////////////////

extern "C"  BOOL SHGetOpenFileName(LPOPENFILENAME lpofn, BOOL bSaveDlg)
{
    BOOL bRet = FALSE;

    FileOpenDlg_t* pGetOpenFileNameDlg = new FileOpenDlg_t(bSaveDlg);
    if (!pGetOpenFileNameDlg)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        // SetExtendedError(CDERR_MEMALLOCFAILURE);
        goto leave;
    }

    bRet = pGetOpenFileNameDlg->Run(lpofn);
    delete pGetOpenFileNameDlg;

leave:
    return bRet;
}

//////////////////////////////////////////////////////////////////////////////


static const short s_ColumnWidthsVGA[] = { 116, 86, 87, 134 };
static const short s_ColumnWidthsQVGA[] = { 110, 86, 81, 134 };

static const TBBUTTON s_tbButtonVGA[] = {
  {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0, 0, 0, -1},
  {VIEW_PARENTFOLDER, IDC_GO_FOLDERUP, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
  {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0, 0, 0, -1},
  {VIEW_NEWFOLDER, IDC_FILE_NEWFOLDER, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
  {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0, 0, 0, -1},
  {VIEW_LIST, IDC_VIEW_LIST, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, -1},
  {VIEW_DETAILS, IDC_VIEW_DETAILS, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, -1},
};

static const TBBUTTON s_tbButtonQVGA[] = {
  {VIEW_PARENTFOLDER, IDC_GO_FOLDERUP, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
  {VIEW_NEWFOLDER, IDC_FILE_NEWFOLDER, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
};


DWORD FileOpenDlg_t::s_TTRefCount = 0;
LPWSTR FileOpenDlg_t::s_pwszTooltips[4] = {0};

const WCHAR FileOpenDlg_t::s_szDesktopCLSID[] = L"{00021400-0000-0000-C000-000000000046}";
const WCHAR FileOpenDlg_t::s_szBitBucketCLSID[] = L"{000214A1-0000-0000-C000-000000000046}";
const WCHAR FileOpenDlg_t::s_RegGUIDs[] = L"\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\CurrentGUIDs";
const WCHAR FileOpenDlg_t::s_RegDisplayName[] = L"DisplayName";

FileOpenDlg_t::FileOpenDlg_t(BOOL bFileSave) :
    m_pidlFQ(NULL),
    m_pIShellFolder(NULL),
    m_pIShellView(NULL),
    m_pIShellListView(NULL),
    m_pIEnumFilter(NULL),
    m_pIOleCommandTarget(NULL),
    m_bRetVal(FALSE)
{
    m_bFileSave = !!bFileSave;
    m_ObjRefCount = 1;
}

FileOpenDlg_t::~FileOpenDlg_t()
{
}

void FileOpenDlg_t::ActivatePendingView()
{
    IShellFolder* psfParent = NULL;
    LPITEMIDLIST pidlParent = NULL;
    STRRET str = {0};
    DWORD dwStyle;
    RECT rc;

    // Update path
    if (ILIsEmpty(m_pidlFQ) || ILIsNameSpace(m_pidlFQ, CLSID_CEShellDesktop))
    {
        SHGetSpecialFolderPath(NULL, m_wszDirPath, CSIDL_DESKTOPDIRECTORY, TRUE);
    }
    else
    {
        SHGetPathFromIDList(m_pidlFQ, m_wszDirPath);
    }

    pidlParent = ILCopy(m_pidlFQ, IL_ALL);
    ILRemoveLast(pidlParent);

    if (SUCCEEDED(SHBindToObject(m_pIShellDesktopFolder,
                    IID_IShellFolder,
                    pidlParent,
                    (void**)&psfParent)))
    {
        if (SUCCEEDED(psfParent->GetDisplayNameOf(ILFindLast(m_pidlFQ),
                        SHGDN_NORMAL | SHGDN_FORADDRESSBAR,
                        &str)))
        {
            StrRetToBuf(&str, m_pidlFQ, m_wszCurrDir, lengthof(m_wszCurrDir));
        }
        psfParent->Release();
    }

    if (pidlParent)
    {
        ILFree(pidlParent);
    }

    UpdateToolbar(m_hwndToolbar);

    GetClientRect(m_hwndDialog, &rc);
    rc.top += CommandBar_Height(m_hwndToolbar);
    DrawPathBanner(NULL, &rc);

    // Set additional styles
    dwStyle = ::GetWindowLong(m_hwndView, GWL_STYLE);
    dwStyle |= WS_BORDER | WS_TABSTOP;
    ::SetWindowLong(m_hwndView, GWL_STYLE, dwStyle);

    ASSERT(!m_pfnViewWndProc);
    m_pfnViewWndProc = (WNDPROC)::SetWindowLong(m_hwndView,
                    GWL_WNDPROC,
                    (DWORD)FileOpenDlg_t::ViewSubclassProc);

    m_pIShellView->UIActivate(SVUIA_ACTIVATE_NOFOCUS);

}

// using rcVisibleDesktop which is bogus unless SIP is docked
BOOL FileOpenDlg_t::CenterWindowSIPAware(HWND hwnd, BOOL fInitial)
{
    BOOL bRet = FALSE;
    SIPINFO si;
    RECT rcWnd;
    RECT rcWorkArea;
    int cx, cy;

    // Get the size of window
    GetWindowRect(hwnd, &rcWnd);

    // Get the size of SIP-less area OR if no SIP, then screen workarea
    memset(&si, 0, sizeof(SIPINFO));
    si.cbSize = sizeof(SIPINFO);

    if (m_sip.m_pSipGetInfo  && (*m_sip.m_pSipGetInfo)(&si))
    {
        rcWorkArea = si.rcVisibleDesktop;
    }
    else
    {
        SystemParametersInfo(SPI_GETWORKAREA, FALSE, &rcWorkArea, 0);
    }

    // if we're positioning after a INICHANGE, then only do something on SIP-up, not SIP-down
    if (!fInitial && !(si.fdwFlags & SIPF_ON))
    {
        goto leave;
    }

    // Calc space left on each side (may be negative is window is too big)
    cx = ((rcWorkArea.right - rcWorkArea.left) - (rcWnd.right - rcWnd.left))/2;
    cy = ((rcWorkArea.bottom - rcWorkArea.top) - (rcWnd.bottom - rcWnd.top))/2;

    DEBUGMSG(ZONE_INFO,
                    (L"VisibleRect=(top=%d, bottom=%d, left=%d, right=%d) \
                    Work=(top=%d, bottom=%d, left=%d, right=%d) \
                    Wnd=(top=%d, bottom=%d, left=%d, right=%d) \
                    cx=%d cy=%d\r\n",
                    si.rcVisibleDesktop.top,
                    si.rcVisibleDesktop.bottom,
                    si.rcVisibleDesktop.left,
                    si.rcVisibleDesktop.right,
                    rcWorkArea.top,
                    rcWorkArea.bottom,
                    rcWorkArea.left,
                    rcWorkArea.right,
                    rcWnd.top,
                    rcWnd.bottom,
                    rcWnd.left,
                    rcWnd.right,
                    cx,
                    cy));

    // if X is negative reset to 0 (so we fall off right edge only, not both)
    if (cx < 0)
    {
        cx = 0;
    }

    // if Y is negative reset to 0 (so we fall off bottom only, not top & bottom)
    if (cy < 0)
    {
        cy = 0;
    }

    bRet = SetWindowPos(hwnd, NULL, rcWorkArea.left+cx, rcWorkArea.top+cy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

leave:
    return bRet;
}

inline int BUTTONSIZE(const TBBUTTON* ptbButtons, int cButtons, BOOL fHelp) 
{
    int i;
    int iButtonsSize = 0;

#define DEFAULTBUTTONX 24 // corresponds to DEFAULTBUTTONX in the toolbar source code.
#define BUTTONSEPX 6 // corresponds to g_dxButtonSep in the toolbar source code.

    // The values of DEFAULTBUTTONX and BUTTONSEPX depend on the values of
    // DEFAULTBUTTONX and g_dxButtonSep in the toolbar source code. If the
    // values in the toolbar source code change, the contants here must also be
    // changed.

    // Calc width of buttons in the command bar -- count the OK & X, and ? if present
    // count a seperator between each of them & a seperator before that block
    // count other buttons & seperators based on table above

    iButtonsSize += (DEFAULTBUTTONX*((fHelp) ? 3 : 2));
    iButtonsSize += (BUTTONSEPX*((fHelp) ? 3: 2));

    if (!ptbButtons)
    {
        goto leave;
    }

    for (i = 0; i < cButtons; i++)
    {
        if (ptbButtons[i].fsStyle & TBSTYLE_SEP)
        {
            iButtonsSize += BUTTONSEPX;
        }
        else
        {
            iButtonsSize += DEFAULTBUTTONX;
        }
    }

leave:
    return iButtonsSize;
}

BOOL FileOpenDlg_t::CreateCommandBar()
{
    BOOL bRet = FALSE;
    HWND hwndCommandBar;
    HFONT hfontOld = NULL;
    HDC hdc = NULL;
    SIZE size;
    LPWSTR pwszTruncatedTitle = NULL;
    RECT rc;
    int nMaxTitleSize, nTitleLen, nFit, n;

    hdc = CreateCompatibleDC(NULL);
    hfontOld = (HFONT)SelectObject(hdc, m_hFont);

    // Check if the dialog title is too long. If it is too long it
    // will push the commandbar buttons off the visible portion of the
    // command bar. Truncate the title and append "..." if it is too long.

    // Determine the maxiumum length (in pixels) that the dialog title can be.
    GetWindowRect(m_hwndDialog, &rc);
    nMaxTitleSize = rc.right - rc.left // width of dialog
                    - BUTTONSIZE(m_ptbButtons, m_cButtons, m_bHelpAvailable) // width of buttons on cmdbar
                    - 5;               // extra width for the static control added
                                       // in CommandBar_InsertControl call below.

    // Determine how many characters of the title can fit.
    nTitleLen = ::wcslen(m_pwszTitle);
    GetTextExtentExPoint(hdc,
                    m_pwszTitle,
                    nTitleLen,
                    nMaxTitleSize,
                    &nFit,
                    NULL,
                    &size);

    // If the length of the title is greater than the maximum space available,
    // create a truncated title with '...' at the end.
    if (nTitleLen > nFit)
    {
        pwszTruncatedTitle = (LPWSTR)g_pShellMalloc->Alloc((nTitleLen+1)*sizeof(WCHAR));
        if (!pwszTruncatedTitle)
        {
            DEBUGMSG(ZONE_INFO, (L"CreateCommandBar: Allocation failed.\r\n"));
            SetLastError(ERROR_OUTOFMEMORY);
            goto leave;
        }

        if (SUCCEEDED(::StringCchCopy(pwszTruncatedTitle, (nTitleLen+1), m_pwszTitle)))
        {
            pwszTruncatedTitle[nFit] = L'\0';
            for (n = 1; (n <= 3 && n <= nFit); n++)
            {
                pwszTruncatedTitle[nFit-n] = L'.';
            }
        }
        size.cx = nMaxTitleSize;
    }

    hwndCommandBar = CommandBar_Create(HINST_CESHELL, m_hwndDialog, IDC_FILEOPENCMDBAR);
    m_hwndToolbar = hwndCommandBar;

    m_hwndStatic = CommandBar_InsertControl(hwndCommandBar, HINST_CESHELL, L"STATIC",  size.cx + 5, SS_LEFT, 1050, 0);
    m_pfnStaticWndProc = (WNDPROC)::SetWindowLong(m_hwndStatic,
                    GWL_WNDPROC,
                    (DWORD)FileOpenDlg_t::StaticSubclassProc);

    if (pwszTruncatedTitle)
    {
        SetWindowText(m_hwndStatic, pwszTruncatedTitle);
        SetWindowText(m_hwndDialog, pwszTruncatedTitle);
        g_pShellMalloc->Free(pwszTruncatedTitle);
    }
    else
    {
        SetWindowText(m_hwndStatic, m_pwszTitle);
        SetWindowText(m_hwndDialog, m_pwszTitle);
    }

    CommandBar_AddBitmap(hwndCommandBar,
                    HINST_COMMCTRL,
                    IDB_VIEW_SMALL_COLOR,
                    0,
                    16,
                    16);
    CommandBar_AddButtons(hwndCommandBar,
                    m_cButtons,
                    m_ptbButtons);

    CommandBar_AddAdornments(hwndCommandBar,
                    CMDBAR_OK |(m_bHelpAvailable ? STD_HELP : 0),
                    0);

    ::SendMessage(hwndCommandBar,
                    TB_CHECKBUTTON,
                    IDC_VIEW_LIST,
                    MAKELONG(TRUE, 0));

    if (s_pwszTooltips[0] && s_pwszTooltips[1] && s_pwszTooltips[2] && s_pwszTooltips[3])
    {
        CommandBar_AddToolTips(hwndCommandBar, lengthof(s_pwszTooltips), s_pwszTooltips);
    }

    bRet = (hwndCommandBar != NULL);

leave:
    if (hfontOld)
    {
        SelectObject(hdc, hfontOld);
    }

    if (hdc)
    {
        DeleteDC(hdc);
    }
    return bRet;
}

void FileOpenDlg_t::DestroyActiveView()
{
    m_wszDirPath[0] = L'\0';
    m_wszCurrDir[0] = L'\0';

    if (m_pIEnumFilter)
    {
        m_pIEnumFilter->Release();
        m_pIEnumFilter = NULL;
    }

    if (m_pIShellListView)
    {
        m_pIShellListView->Release();
        m_pIShellListView = NULL;
    }

    if (m_pIOleCommandTarget)
    {
        m_pIOleCommandTarget->Release();
        m_pIOleCommandTarget = NULL;
    }

    if (m_pIShellView)
    {
        m_pIShellView->UIActivate(SVUIA_DEACTIVATE);

        m_pIShellView->DestroyViewWindow();
        m_hwndView = NULL;

        m_pIShellView->Release();
        m_pIShellView = NULL;
    }

    if (m_pfnViewWndProc)
    {
        ::SetWindowLong(m_hwndView,
                        GWL_WNDPROC,
                        (DWORD)m_pfnViewWndProc);
        m_pfnViewWndProc = NULL;
    }

    if (m_pIShellFolder)
    {
        m_pIShellFolder->Release();
        m_pIShellFolder = NULL;
    }

    if (m_pidlFQ)
    {
        ILFree(m_pidlFQ);
        m_pidlFQ = NULL;
    }
}

void FileOpenDlg_t::DrawPathBanner(HDC hdc, RECT* pRect)
{
    WCHAR wszTempPath[MAX_PATH];
    HRESULT hr;
    HDC hdcLocal = NULL;
    HFONT hfontOld = NULL;
    COLORREF clrTextOld = CLR_INVALID;
    RECT rc;
    POINT pt[2];

    if (!hdc)
    {
        hdcLocal = GetDC(m_hwndDialog);
        if (!hdcLocal)
        {
            return;
        }
        hdc = hdcLocal;
    }

    PREFAST_ASSERT(pRect);
    CopyRect(&rc, pRect);

    FillRect(hdc, &rc, m_hBGBrush);

    hfontOld = (HFONT)SelectObject(hdc, m_hFont);
    clrTextOld = SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));

    SetBkMode(hdc, TRANSPARENT);
    SelectObject(hdc, GetStockObject(WHITE_PEN));

    pt[0].x = rc.left;
    pt[0].y = rc.top + 1;

    pt[1].x = rc.right;
    pt[1].y = rc.top + 1;

    Polyline(hdc, pt, 2);
    rc.right -= 48;

    rc.bottom = rc.top + 22;

    if (m_hiconFolderSM)
    {
        DrawIconEx(hdc, rc.left + 6, rc.top + 3, m_hiconFolderSM, 16, 16, 0, NULL, DI_NORMAL);
        rc.left += 26;
    }

    // Note that StringCchCopy always null-terminates the buffer
    hr = ::StringCchCopy(wszTempPath, lengthof(wszTempPath), m_wszCurrDir);
    ASSERT(SUCCEEDED(hr)); // Failure is acceptable

    PathCompactPath(hdc, wszTempPath, rc.right - rc.left);
    DrawText(hdc,
                    wszTempPath,
                    -1,
                    &rc,
                    DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_VCENTER);

    if (clrTextOld != CLR_INVALID)
    {
        SetTextColor(hdc, clrTextOld);
    }

    if (hfontOld)
    {
        SelectObject(hdc, hfontOld);
    }

    if (hdcLocal)
    {
        ReleaseDC(m_hwndDialog, hdcLocal);
    }
}

BOOL FileOpenDlg_t::HandleCommand(WPARAM wParam, LPARAM lParam)
{
    BOOL bRet = FALSE;

     // Asume standard group command
    const GUID* pguidCmdGroup = NULL;

    switch (GET_WM_COMMAND_ID(wParam, lParam))
    {
        case IDOK:
            // If the combo box is dropped then select the current item
            if (SendDlgItemMessage(m_hwndDialog, IDC_FILEOPENTYPE,
                                   CB_GETDROPPEDSTATE, 0, 0))
            {
                SendDlgItemMessage(m_hwndDialog, IDC_FILEOPENTYPE,
                    WM_KEYDOWN, VK_RETURN, 0);
                break;
            }

            // OK button is clicked during item label editing
            if ((GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED) && m_bInLabelEdit)
            {
                HandleOK();
                break;
            }

            if ((GET_WM_COMMAND_CMD(wParam, lParam) != BN_CLICKED) || !HandleOK())
            {
                //ListView_SetItemState(m_hwndView, -1, 0, LVIS_CUT | LVIS_SELECTED);
                break;
            }

            bRet = EndDialog(m_hwndDialog, GET_WM_COMMAND_ID(wParam, lParam));
            break;

        case IDCANCEL:
            if (!GET_WM_COMMAND_HWND(wParam, lParam))
            {
                if (m_bInLabelEdit && m_pIOleCommandTarget)
                {
                    m_pIOleCommandTarget->Exec(&CGID_CEShell,
                                    IDC_DISMISS_EDIT,
                                    0,
                                    NULL,
                                    NULL);
                }

                bRet = EndDialog(m_hwndDialog, GET_WM_COMMAND_ID(wParam, lParam));
            }
            break;

        case IDC_FILEOPENHELP:
            if (m_bHelpAvailable)
            {
                CreateProcess(L"peghelp",
                                (m_bFileSave ?
                                        L"file:wince.htm#Save_As_dialog_box" :
                                        L"file:wince.htm#Open_dialog_box"),
                                NULL,
                                NULL,
                                FALSE,
                                0,
                                NULL,
                                NULL,
                                NULL,
                                NULL);
            }
            break;

        case IDC_FILEOPENNAME:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
            {
                m_bSelected = FALSE;
            }
            break;

        case IDC_FILEOPENTYPE:
            //wait for the CBN_CLOSEUP notification before processing a change in the current selection.
            // This is particularly important if a significant amount of processing is required.
            if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_CLOSEUP)
            {
                int iComboIndex = ComboBox_GetCurSel(GET_WM_COMMAND_HWND(wParam, lParam));
                if (m_pIOleCommandTarget &&
                    ((DWORD)(iComboIndex+1) != m_pOpenFileName->nFilterIndex) &&
                    SUCCEEDED(::StringCchCopy(
                                    m_wszCurrentFilter,
                                    lengthof(m_wszCurrentFilter),
                                    (LPWSTR)ComboBox_GetItemData(GET_WM_COMMAND_HWND(wParam, lParam), iComboIndex))))
                {
                    // Update parent as well
                    // since populating the list could take several seconds
                    UpdateWindow(m_hwndDialog);
                    UpdateWindow(GetParent(m_hwndDialog));
                   
                    m_pOpenFileName->nFilterIndex = iComboIndex + 1;

                    // Note that SetFilterString() will reset EnumFilter on failure
                    m_pIEnumFilter->SetFilterString(m_wszCurrentFilter);

                    m_bChangeUpdateDisabled = TRUE;
                    m_pIOleCommandTarget->Exec(NULL, OLECMDID_REFRESH, 0, NULL, NULL);
                    m_bChangeUpdateDisabled = FALSE;

                    // NOTE: the notification is a one's based index
                    //       but the combobox is zero based.
                    NotifyTypeChange();
                }
            }
            break;

        case IDC_FILE_DELETE:
        case IDC_FILE_NEWFOLDER:
        case IDC_FILE_PROPERTIES:
        case IDC_FILE_RENAME :
        case IDC_VIEW_DETAILS:
        case IDC_VIEW_ICONS:
        case IDC_VIEW_LIST:
        case IDC_VIEW_SMALL:
        case IDC_ARRANGE_BYNAME:
        case IDC_ARRANGE_BYSIZE:
        case IDC_ARRANGE_BYTYPE:
        case IDC_ARRANGE_BYDATE:
            pguidCmdGroup = &CGID_CEShell;
            // fall through

        case OLECMDID_REFRESH:
            if (m_pIOleCommandTarget)
            {
                bRet = SUCCEEDED(m_pIOleCommandTarget->Exec(pguidCmdGroup,
                                GET_WM_COMMAND_ID(wParam, lParam),
                                0,
                                NULL,
                                NULL));
            }
            break;

        case IDC_GO_FOLDERUP:
            BrowseObject(NULL, SBSP_SAMEBROWSER | SBSP_PARENT);
            break;

        default:
            break;
    }

    return bRet;
}


BOOL FileOpenDlg_t::HandleListViewNotify(WPARAM wParam, LPARAM lParam, LRESULT* plResult)
{
    BOOL bHandled = TRUE;
    LRESULT lResult = 0;
    NM_LISTVIEW *pnm = (NM_LISTVIEW *)lParam;

    switch (pnm->hdr.code)
    {
        case LVN_ITEMCHANGED:
            if (!m_bChangeUpdateDisabled &&
                (pnm->uChanged & LVIF_STATE))
            {
                if(pnm->uNewState & LVIS_SELECTED)
                {
                    LPCITEMIDLIST* aPidls = NULL;
                    UINT uItemCount = 0;

                    if (!(pnm->uOldState & LVIS_SELECTED) &&
                        SUCCEEDED(m_pIShellListView->GetSelectedItems(&aPidls, &uItemCount)) &&
                        (uItemCount == 1) &&
                        !IsBrowsableItem(NULL, aPidls[0], NULL))
                    {
                        HWND hwndEdit = GetDlgItem(m_hwndDialog, IDC_FILEOPENNAME);
                        WCHAR wszFileName[MAX_PATH];
                        STRRET str = {0};

                        wszFileName[0] = L'\0';
                        if (SUCCEEDED(m_pIShellFolder->GetDisplayNameOf(aPidls[0],
                                        SHGDN_NORMAL | SHGDN_FORADDRESSBAR,
                                        &str)))
                        {
                            StrRetToBuf(&str, aPidls[0], wszFileName, lengthof(wszFileName));
                        }

                        SetWindowText(hwndEdit, PathFindFileName(wszFileName));
                    }

                    if (aPidls)
                    {
                        g_pShellMalloc->Free(aPidls);
                    }

                    m_bSelected = TRUE;
                }
                else
                {
                   // Something was unselected
                   m_bSelected = FALSE;
                }
            }
            break;

        case LVN_BEGINLABELEDIT:
            m_bInLabelEdit = TRUE;
            bHandled = FALSE;
            break;

        case LVN_ENDLABELEDIT:
            m_bInLabelEdit = FALSE;
            bHandled = FALSE;
            break;

        case NM_RETURN:
        case NM_DBLCLK:
            if (pnm->iItem != -1)
            {
                PostMessage(m_hwndDialog, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), 0);
            }
            break;

        case LVN_KEYDOWN:
        {
            LV_KEYDOWN *plvkd = (LV_KEYDOWN *)lParam;

            switch (plvkd->wVKey)
            {
                case VK_BACK:
                    PostMessage(m_hwndDialog, WM_COMMAND, IDC_GO_FOLDERUP, 0);
                    break;

                case VK_DELETE:
                    PostMessage(m_hwndDialog, WM_COMMAND, IDC_FILE_DELETE, 0);
                    break;

                case VK_RETURN:
                    PostMessage(m_hwndDialog, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), 0);
                    break;

                case VK_F1:
                    PostMessage(m_hwndDialog, WM_COMMAND, IDC_FILEOPENHELP, 0);
                    break;

                case VK_F5:
                    PostMessage(m_hwndDialog, WM_COMMAND, OLECMDID_REFRESH, 0);
                    break;

                default:
                    break;
            }
            break;
        }

        default:
            bHandled = FALSE;
    }

    if (bHandled && plResult)
    {
        *plResult = lResult;
    }

    return bHandled;
}

BOOL FileOpenDlg_t::HandleOK()
{
    BOOL bRet = FALSE;
    HRESULT hr;

    HWND hwndEdit;
    LPWSTR pwszExt;
    LPWSTR pwszExtCopy = NULL;

    WCHAR wszPath[MAX_PATH] = L"::";
    WCHAR wszTemp[MAX_PATH];
    size_t cch;

    UINT uError = 0;
    LPCWSTR pwszFileError = NULL;

    DWORD dwAttrib;

    DECLAREWAITCURSOR;
    SetWaitCursor();

    hwndEdit = GetDlgItem(m_hwndDialog, IDC_FILEOPENNAME);

    // make a copy of the default extension
    if (m_pwszDefExt)
    {
        pwszExtCopy = (LPWSTR)g_pShellMalloc->Alloc((1+::wcslen(m_pwszDefExt))*sizeof(WCHAR));
        if (pwszExtCopy)
        {
            ::wcscpy(pwszExtCopy, m_pwszDefExt);
        }
    }

    // set the 'default' extension based on the filterbox selection
    SetDefaultExtension();

    // get the target of our affections. If the last action was selecting something
    // that's what we want. If it was typing in teh edit box, then that's the one
    if (m_bSelected)
    {
        LPCITEMIDLIST* aPidls = NULL;
        UINT uItemCount = 0;

        wszPath[0] = L'\0';
        hr = E_FAIL;
        if (SUCCEEDED(m_pIShellListView->GetSelectedItems(&aPidls, &uItemCount)) &&
            (uItemCount == 1))
        {
            LPITEMIDLIST pidlTarget = NULL;
            if (IsBrowsableItem(NULL, aPidls[0], &pidlTarget))
            {
                // This is an enter press just after a directory selection...
                if (pidlTarget)
                {
                    hr = BrowseObject(pidlTarget, SBSP_SAMEBROWSER | SBSP_ABSOLUTE);
                    ILFree(pidlTarget);
                }
                else
                {
                    hr = BrowseObject(aPidls[0], SBSP_SAMEBROWSER | SBSP_RELATIVE);
                }
            }
            else
            {
                // Enter pressed after file selection
                STRRET str = {0};

                if (SUCCEEDED(m_pIShellFolder->GetDisplayNameOf(aPidls[0],
                                SHGDN_NORMAL | SHGDN_FORPARSING,
                                &str)) &&
                    SUCCEEDED(StrRetToBuf(&str, aPidls[0], wszTemp, lengthof(wszTemp))))
                {
                    LPWSTR pwszFormat = L"%s\\%s";

                    if (0 == ::wcscmp(m_wszDirPath, L"\\"))
                    {
                        pwszFormat = L"%s%s";
                    }
                    
                    hr = ::StringCchPrintf(wszPath,
                                lengthof(wszPath),
                                pwszFormat,
                                m_wszDirPath,
                                PathFindFileName(wszTemp));
                }
            }
        }

        if (aPidls)
        {
            g_pShellMalloc->Free(aPidls);
        }

        if (wszPath[0])
        {
            goto doFileChecking;
        }
        goto leave;
    }

    // Process Text in the FileName box.
    GetWindowText(hwndEdit, wszTemp, lengthof(wszTemp));

    // Validity checking
    if (wszTemp[0] == L'\0')
    {
        DEBUGMSG(ZONE_INFO, (L"HandleOK FAILED--null path\n"));
        goto leave;
    }

    cch = lengthof(wszPath) -2;
    if (GetParsingName(wszTemp, wszPath+2, &cch))
    {
        LPITEMIDLIST pidl = NULL;

        if (SUCCEEDED(NavigateToFolder(wszPath)))
        {
            goto leave;
        }

        if (SUCCEEDED(m_pIShellDesktopFolder->ParseDisplayName(NULL,
                        NULL,
                        (LPOLESTR)wszPath,
                        NULL,
                        &pidl,
                        NULL)))
        {
            SHGetPathFromIDList(pidl, wszTemp);
            ILFree(pidl);
        }
    }

    PathRemoveBlanks(wszTemp);
    if (::wcspbrk(wszTemp, L"/:\"<>|"))
    {
        DEBUGMSG(ZONE_INFO, (L"HandleOK Error, invalid filename\n"));
        uError = IDS_SHGOFN_ERR_INVALIDFILENAME;
        pwszFileError = wszTemp;
        goto showError;
    }

    // check for wildcards. Note wildcard cannot contain a \  char
    if (::wcspbrk(wszTemp, L"*?") && !::wcschr(wszTemp, L'\\'))
    {
        if (m_pIEnumFilter &&
            m_pIOleCommandTarget &&
            SUCCEEDED(::StringCchCopy(m_wszCurrentFilter,
                        lengthof(m_wszCurrentFilter),
                        wszTemp)))
        {
            // Note that SetFilterString() will reset EnumFilter on failure
            m_pIEnumFilter->SetFilterString(m_wszCurrentFilter);

            m_bChangeUpdateDisabled = TRUE;
            m_pIOleCommandTarget->Exec(NULL, OLECMDID_REFRESH, 0, NULL, NULL);
            m_bChangeUpdateDisabled = FALSE;
        }
        DEBUGMSG(ZONE_INFO, (L"HandleOK. Got wild-cards--DONE\n"));
        goto leave;
    }

    // Construct proper path in wszPath
    DEBUGMSG(ZONE_INFO, (L"HandleOk. Calling ProcessDots on '%s'\n", wszTemp));
    if (!ProcessDots(m_wszDirPath, wszPath, wszTemp))
    {
        uError = IDS_SHGOFN_ERR_PATHTOOLONG;
        goto showError;
    }

doFileChecking:
    // Check for the file's existence, exactly as typed & if it's a dir
    dwAttrib = GetFileAttributes(wszPath);
    pwszExt = PathFindExtension(wszPath);

    DEBUGMSG(ZONE_INFO, (L"HandleOK. Path(%s) Attribs(%d) Ext(%s)\n", wszPath, dwAttrib, pwszExt));

    // Check if we have to dereference a link
    if (PathIsLink(wszPath) &&
        !(m_pOpenFileName->Flags & OFN_NODEREFERENCELINKS) &&
        SHGetShortcutTarget(wszPath, wszTemp, lengthof(wszTemp)))
    {
        PathRemoveQuotesAndArgs(wszTemp);

        DWORD dwAttribTarget = GetFileAttributes(wszTemp);
        if (-1 != dwAttribTarget)
        {
            DEBUGMSG(ZONE_INFO, (L"HandleOK. DEREF Link (%s)==>(%s)\n", wszPath, wszTemp));

            // Note that StringCchCopy always null-terminates the buffer
            hr = ::StringCchCopy(wszPath, lengthof(wszPath), wszTemp);
            ASSERT(SUCCEEDED(hr)); // Failure is acceptable

            dwAttrib = dwAttribTarget;
            pwszExt = PathFindExtension(wszPath);

            if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
            {
                SetWindowText(hwndEdit, NULL);
            }
            DEBUGMSG(ZONE_INFO, (L"HandleOK. Target Path(%s) Attribs(%d) Ext(%s)\n", wszPath, dwAttrib, pwszExt));
        }
    }

    // if dir, try to change to it
    if (dwAttrib != -1 && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
    {
        m_bChangeUpdateDisabled = TRUE;
        hr = NavigateToFolder(wszPath);
        m_bChangeUpdateDisabled = FALSE;

        if (SUCCEEDED(hr))
        {
            DEBUGMSG(ZONE_INFO, (L"HandleOK done--changed dir. New Dir(%s)\n", m_wszCurrDir));

            GetWindowText(hwndEdit, m_pOpenFileName->lpstrFile, m_pOpenFileName->nMaxFile);
            PathRemoveTrailingSlashes(m_pOpenFileName->lpstrFile);
            SetWindowText(hwndEdit, PathFindFileName(m_pOpenFileName->lpstrFile));
            goto leave;
        }
    }

    // If file doesn't exist as is, check for the name + the default extension
    if ((dwAttrib == -1) && (m_pwszDefExt))
    {
        DEBUGMSG(ZONE_INFO, (L"HandleOK, checking for default extn(%s)\n", m_pwszDefExt));
        if (pwszExt[0])
        {
            HKEY hKey;

            if (0 == ::wcsicmp(m_pwszDefExt, (pwszExt + 1)))
            {
                goto doneCheckingExtension;
            }
            else if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, pwszExt, 0, 0, &hKey))
            {
                RegCloseKey(hKey);
                goto doneCheckingExtension;
            }
        }

        if (::wcscmp(m_wszCurrentFilter, L"*.*") ||  L'\0' == pwszExt[0])
        {
            //Type != All Files || no extension
            hr = ::StringCchLength(wszPath, lengthof(wszPath), &cch);
            if (SUCCEEDED(hr))
            {
                pwszExt = (wszPath + cch);
                pwszExt[0] = L'.';
            }

            // according to the docs, only the first three characters are appended
            if (FAILED(hr) ||
                FAILED(::StringCchCopyN(pwszExt+1,
                            (lengthof(wszPath) - cch - 1),
                            m_pwszDefExt,
                            3)))
            {
                // path is too long
                uError = IDS_SHGOFN_ERR_PATHTOOLONG;
                goto showError;
            }
        }

        dwAttrib = GetFileAttributes(wszPath);
        DEBUGMSG(ZONE_INFO, (L"HandleOK. WITH EXT Path(%s) Attribs(%d) Ext(%s)\n", wszPath, dwAttrib, pwszExt));
    }

doneCheckingExtension:

    if (dwAttrib != -1)
    {
        // File exists
        if (m_bFileSave)
        {
            if (m_pOpenFileName->Flags & OFN_OVERWRITEPROMPT)
            {
                // Here's where we put up the replace dialog.
                ::StringCchPrintfEx(wszTemp,
                                lengthof(wszTemp),
                                NULL,
                                NULL,
                                STRSAFE_IGNORE_NULLS,
                                LOAD_STRING(IDS_SHGOFN_ERR_REPLACEPROMPT),
                                wszPath);
                if (IDYES != MessageBox(m_hwndDialog,
                                    wszTemp,
                                    m_pwszTitle,
                                    MB_YESNO | MB_ICONEXCLAMATION | MB_APPLMODAL))
                {
                    goto leave;
                }
            }
        }
    }
    else
    {
        // File does not exist
        if (m_pOpenFileName->Flags & OFN_PATHMUSTEXIST)
        {
            if (SUCCEEDED(::StringCchCopy(wszTemp,
                            lengthof(wszTemp),
                            wszPath)))
            {
                PathRemoveFileSpec(wszTemp);
            }

            if (!PathFileExists(wszTemp))
            {
                GetWindowText(hwndEdit, wszTemp, lengthof(wszTemp));
                uError = IDS_SHGOFN_ERR_PATHDOESNOTEXIST;
                pwszFileError = wszTemp;
                goto showPathError;
            }
        }

        if (m_pOpenFileName->Flags & OFN_FILEMUSTEXIST)
        {
            uError = IDS_SHGOFN_ERR_FILENOTFOUND;
            pwszFileError = wszPath;
            goto showPathError;
        }

        if ((m_pOpenFileName->Flags & OFN_CREATEPROMPT) && !m_bFileSave)
        {
            ::StringCchPrintfEx(wszTemp,
                            lengthof(wszTemp),
                            NULL,
                            NULL,
                            STRSAFE_IGNORE_NULLS,
                            LOAD_STRING(IDS_SHGOFN_ERR_CREATEPROMPT),
                            wszPath);
            if ( IDYES != MessageBox(m_hwndDialog,
                            wszTemp,
                            m_pwszTitle,
                            MB_YESNO | MB_ICONEXCLAMATION | MB_APPLMODAL))
            {
                goto leave;
            }
        }
    }

    // set the appropriate OFN fields
    // 1) set the file name
    if (FAILED(::StringCchLength(wszPath, lengthof(wszPath), &cch)) ||
        (cch >= m_pOpenFileName->nMaxFile) ||
        FAILED(::StringCchCopy(m_pOpenFileName->lpstrFile,
                        m_pOpenFileName->nMaxFile,
                        wszPath)))
    {
        uError = IDS_SHGOFN_ERR_PATHTOOLONG;
        goto showError;
    }

    // 2) set nFileExtension and OFN_EXTENSIONDIFFERENT
    // we compare the current extension to the default extension that was
    // passed in originally, or else set by CDM_SETDEFEXT, this was copied into pwszExtCopy
    pwszExt = PathFindExtension(wszPath);
    if (*pwszExt && pwszExtCopy && ::wcsicmp(pwszExt+1, pwszExtCopy))
    {
        m_pOpenFileName->Flags |= OFN_EXTENSIONDIFFERENT;
    }
    if (*pwszExt == L'.')
    {
        pwszExt++;
        if (*pwszExt)
        {
            m_pOpenFileName->nFileExtension = (WORD)(pwszExt - wszPath);
        }
        else
        {
            // according to the docs, if the user types . as the last char,
            // this field should be set to 0.
            m_pOpenFileName->nFileExtension = 0;
        }
    }
    else
    {
        m_pOpenFileName->nFileExtension = (WORD)(pwszExt - wszPath);
    }

    // 3) set the file name without path (lpstrFileTitle and nFileOffset)
    pwszExt = PathFindFileName(wszPath);
    m_pOpenFileName->nFileOffset = (WORD)(pwszExt - wszPath);

    if (m_pOpenFileName->lpstrFileTitle &&
        m_pOpenFileName->nMaxFileTitle > 0)
    {
        if (FAILED(::StringCchLength(pwszExt,
                        lengthof(wszPath)-(pwszExt - wszPath),
                        &cch)) ||
            (cch >= m_pOpenFileName->nMaxFileTitle) ||
            FAILED(::StringCchCopy(m_pOpenFileName->lpstrFileTitle,
                            m_pOpenFileName->nMaxFileTitle,
                            pwszExt)))
        {
            uError = IDS_SHGOFN_ERR_PATHTOOLONG;
            goto showError;
        }
    }

    // 4) nFilterIndex
    m_pOpenFileName->nFilterIndex = (ComboBox_GetCurSel(GetDlgItem(m_hwndDialog, IDC_FILEOPENTYPE))+1);

    DEBUGMSG(ZONE_INFO,
        (L"HandleOK Done. Returning lpstrFile(%s), lpstrFileTitle(%s), nFilterIndex(%d), nFileOffset(%d), nFileExtension(%d)\n",
        m_pOpenFileName->lpstrFile,
        m_pOpenFileName->lpstrFileTitle,
        m_pOpenFileName->nFilterIndex,
        m_pOpenFileName->nFileOffset,
        m_pOpenFileName->nFileExtension));

    m_bRetVal = TRUE;
    bRet = TRUE;

leave:
    ResetWaitCursor();

    if (pwszExtCopy)
    {
        g_pShellMalloc->Free(pwszExtCopy);
    }

    m_bSelected = FALSE;

    ::SendMessage(hwndEdit, EM_SETSEL, 0, -1);
    SetFocus(hwndEdit);

    return bRet;

showError:
    ShellDialogs::ShowFileError(m_hwndDialog,
                    m_pwszTitle,
                    MAKEINTRESOURCE(uError),
                    pwszFileError,
                    MB_OK | MB_ICONEXCLAMATION);
    goto leave;

showPathError:
    ShellDialogs::ShowPathError(m_hwndDialog,
                    m_pwszTitle,
                    MAKEINTRESOURCE(uError),
                    pwszFileError,
                    MB_OK | MB_ICONEXCLAMATION);
    goto leave;
}

BOOL FileOpenDlg_t::HaveSIP()
{
    BOOL bRet = FALSE;
    SIPINFO si;

    if (!m_sip.m_pSipGetInfo)
    {
        // no SIP functions
        goto leave;
    }

    memset(&si, 0, sizeof(SIPINFO));
    si.cbSize = sizeof(SIPINFO);
    if (!(*m_sip.m_pSipGetInfo)(&si))
    {
        DEBUGMSG(ZONE_INFO, (L"Fileopen: CoreSIP exists, but no SIP available\r\n"));
        // SIP not available?
        goto leave;
    }

    bRet = TRUE;

leave:
    return bRet;
}

BOOL FileOpenDlg_t::InitDialog(HWND hDlg)
{
    BOOL bRet = FALSE;
    HRESULT hr;
    HWND hwndEdit;

    // Initalize the SipPref control if necessary
    WNDCLASS wc;

    AygInitExtraControls();

    if (GetClassInfo(HINST_CESHELL, L"SIPPREF", &wc))
    {
        CreateWindow(L"SIPPREF",
                        NULL,
                        WS_CHILD,
                        -10,
                        -10,
                        5,
                        5,
                        hDlg,
                        NULL,
                        HINST_CESHELL,
                        NULL);
    }

    // size for the sip
    AygInitDialog(hDlg, SHIDIF_SIZEDLG);

#ifdef DEBUG
    RECT rc;
    GetWindowRect(hDlg, &rc);
    DEBUGMSG(ZONE_INFO, (L"FileOpen Window: %d %d %d %d\r\n", rc.left, rc.top, rc.right, rc.bottom));
#endif

    m_hwndDialog = hDlg;
    m_hwndView = GetDlgItem(hDlg, IDC_FILEOPENLIST);

    GetWindowRect(m_hwndView, &m_rcView);
    MapWindowRect(NULL, hDlg, &m_rcView);
    m_rcView.top += 20;

    DestroyWindow(m_hwndView);
    m_hwndView = NULL;

    m_bInLabelEdit = FALSE;
    m_bSelected = FALSE;

    m_bChangeUpdateDisabled = TRUE;

    CreateCommandBar();
    InitFilterBox();

    hwndEdit = GetDlgItem(m_hwndDialog, IDC_FILEOPENNAME);
    SetWindowText(hwndEdit, PathFindFileName(m_pOpenFileName->lpstrFile));

    m_bChangeUpdateDisabled = FALSE;

    // according to the docs, default is system root directory.
    if (m_pOpenFileName->lpstrInitialDir && PathIsDirectory(m_pOpenFileName->lpstrInitialDir))
    {
        hr = NavigateToFolder(m_pOpenFileName->lpstrInitialDir);
    }
    else
    {
        hr = NavigateToFolder(L"\\");
    }

    if (FAILED(hr))
    {
        ::SetWindowLong(m_hwndStatic, GWL_WNDPROC, (DWORD)m_pfnStaticWndProc);
        bRet = EndDialog(hDlg, IDCANCEL);
        goto leave;
    }

    ::SendMessage(GetDlgItem(hDlg, IDC_FILEOPENNAME), EM_LIMITTEXT, MAX_PATH-1, 0);

    CenterWindowSIPAware(hDlg, TRUE);

    ::SendMessage(hwndEdit, EM_SETSEL, 0, -1);
    SetFocus(hwndEdit);

leave:
    return bRet;
}

DWORD FileOpenDlg_t::InitFilterBox()
{
    DWORD nIndex = 0;
    HRESULT hr;
    size_t cch;
    HWND hwndDialog = m_hwndDialog;
    HWND hwndComboBox = GetDlgItem(hwndDialog, IDC_FILEOPENTYPE);
    LPCWSTR lpwszFilter = m_pOpenFileName->lpstrFilter;

    m_wszCurrentFilter[0] = L'\0';

    if (!hwndComboBox)
    {
        goto leave;
    }

    if (!lpwszFilter)
    {
        EnableWindow(hwndComboBox, FALSE);
        goto leave;
    }

    while (*lpwszFilter)
    {
        //  First string put in as string to show.
        nIndex = ComboBox_AddString(hwndComboBox, lpwszFilter);

        cch = ::wcslen(lpwszFilter) + 1;
        lpwszFilter += cch;

        //  Second string put in as itemdata.
        ComboBox_SetItemData(hwndComboBox, nIndex, lpwszFilter);

        //  Advance to next element.
        cch = ::wcslen(lpwszFilter) + 1;
        lpwszFilter += cch;
    }

    ComboBox_SetCurSel(hwndComboBox, m_pOpenFileName->nFilterIndex-1);

    // Note that StringCchCopy always null-terminates the buffer
    hr = ::StringCchCopy(m_wszCurrentFilter,
                    lengthof(m_wszCurrentFilter),
                    (LPWSTR)ComboBox_GetItemData(hwndComboBox, m_pOpenFileName->nFilterIndex-1));
    ASSERT(SUCCEEDED(hr)); // Failure is acceptable

    //Set the extended UI so that the user can drop the CB using the arrow keys
    ComboBox_SetExtendedUI(hwndComboBox,TRUE);

leave:
    return nIndex;
}

BOOL FileOpenDlg_t::InitGlobals()
{
    BOOL bRet = FALSE;
    HRESULT hr;
    LOGFONT lf;
    INITCOMMONCONTROLSEX icce;

    // Get a shell folder for the desktop
    hr = SHGetDesktopFolder(&m_pIShellDesktopFolder);
    if (FAILED(hr) || !m_pIShellDesktopFolder)
    {
        goto leave;
    }

    // init common controls
    icce.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icce.dwICC  = ICC_LISTVIEW_CLASSES|ICC_BAR_CLASSES;
    if (!InitCommonControlsEx(&icce))
    {
        RETAILMSG(1, (L"FileOpen/SaveDialog--InitCommonControls failed!\n"));
        SetLastError(ERROR_DLL_INIT_FAILED);
        goto leave;
    }

    bRet = TRUE;

    // icon
    m_hiconFolderSM = (HICON)LoadImage(HINST_CESHELL, MAKEINTRESOURCE(IDI_EXPLORER2), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

    // see if PEGHELP.EXE is present and try to find the help files
    if (-1 != GetFileAttributes(L"\\windows\\peghelp.exe"))
    {
        m_bHelpAvailable = TRUE;
    }
    else
    {
        m_bHelpAvailable = FALSE;
        RETAILMSG(1, (L"FileOpen: No Help Available\n"));
    }

    // font for current directory banner
    GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &lf);
    lf.lfHeight = 16;
    lf.lfWidth = 0;
    lf.lfWeight = FW_BOLD;
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
#ifndef UNDER_CE
    ::wcscpy(lf.lfFaceName, L"MS Sans Serif");
#endif
    m_hFont = CreateFontIndirect(&lf);
    m_hBGBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

    // Check if we have SIP functionality
#pragma prefast(suppress:321, "Not loading dll via relative path") 
    m_sip.m_hCoreDll = LoadLibrary(L"coredll.dll");
    if (m_sip.m_hCoreDll)
    {
        m_sip.m_pSipGetInfo = (LPFNSIP)GetProcAddress(m_sip.m_hCoreDll, L"SipGetInfo");
        if (m_sip.m_pSipGetInfo)
        {
            DEBUGMSG(ZONE_INFO, (L"FILEOPEN: CoreSIP is present!\r\n"));
        }
        else
        {
            DEBUGMSG(ZONE_INFO, (L"Fileopen: CoreSIP component not present\r\n"));
            m_sip.m_pSipGetInfo = NULL;
        }
    }

leave:
    return bRet;
}

// If the item is a shortcut and target is browsable, function returns
// an  item identifier list for the latter object on ppidlOut.
// PIDL is to be relative to the root of the namespace (the desktop).
// ppidlOut is set to NULL otherwise.
BOOL FileOpenDlg_t::IsBrowsableItem(IShellFolder* psfParent, LPCITEMIDLIST pidlRelative, LPITEMIDLIST *ppidlOut)
{
    BOOL bRet = FALSE;
    ULONG ulAttrib = SFGAO_FOLDER | SFGAO_BROWSABLE | SFGAO_LINK;
    LPITEMIDLIST pidlTarget = NULL;

    if (!psfParent)
    {
        // Assume current folder is the parent
        psfParent = m_pIShellFolder;
        if (!psfParent)
        {
            ASSERT(0);
            goto leave;
        }
    }

    if (FAILED(psfParent->GetAttributesOf(1, &pidlRelative, &ulAttrib)))
    {
        goto leave;
    }

    if ((SFGAO_FOLDER | SFGAO_BROWSABLE) & ulAttrib)
    {
        bRet = TRUE;
        goto leave;
    }

    if ((SFGAO_LINK & ulAttrib) &&
        !(m_pOpenFileName->Flags & OFN_NODEREFERENCELINKS))
    {
        STRRET str = {0};
        WCHAR wszPath[MAX_PATH];

        if (SUCCEEDED(psfParent->GetDisplayNameOf(pidlRelative,
                        SHGDN_NORMAL | SHGDN_FORPARSING,
                        &str)) &&
            SUCCEEDED(StrRetToBuf(&str, pidlRelative, wszPath, lengthof(wszPath))) &&
            SHGetShortcutTarget(wszPath, wszPath, lengthof(wszPath)))
        {
            PathRemoveQuotesAndArgs(wszPath);

            IShellFolder* psfParentTarget = NULL;
            if (SUCCEEDED(m_pIShellDesktopFolder->ParseDisplayName(NULL,
                            NULL,
                            (LPOLESTR)wszPath,
                            NULL,
                            &pidlTarget,
                            NULL)) &&
                SUCCEEDED(SHBindToParent(pidlTarget,
                            IID_IShellFolder,
                            (void**)&psfParentTarget,
                            NULL)) &&
                IsBrowsableItem(psfParentTarget, ILFindLast(pidlTarget), NULL))
            {
                bRet = TRUE;
            }

            if (psfParentTarget)
            {
                psfParentTarget->Release();
            }
        }
    }

leave:
    if (ppidlOut && bRet)
    {
        *ppidlOut = ILCopy(pidlTarget, IL_ALL);
    }

    if (pidlTarget)
    {
        ILFree(pidlTarget);
    }

    return bRet;
}

HRESULT FileOpenDlg_t::NavigateToFolder(LPCWSTR pwszPath)
{
    HRESULT hr;
    LPITEMIDLIST pidl = NULL;
    LPITEMIDLIST pidlRelative = NULL;
    IShellFolder* psfParent = NULL;
    WCHAR wszPath[MAX_PATH];

    ULONG ulAttrib = SFGAO_FOLDER | SFGAO_BROWSABLE | SFGAO_FILESYSTEM;

    // cannot be empty path
    if (!pwszPath || !*pwszPath)
    {
        hr = E_INVALIDARG;
        goto leave;
    }

    hr = ::StringCchCopy(wszPath, lengthof(wszPath), pwszPath);
    if (FAILED(hr))
    {
        goto leave;
    }
    PathRemoveTrailingSlashes(wszPath);

    if (SUCCEEDED(m_pIShellDesktopFolder->ParseDisplayName(NULL,
                        NULL,
                        (LPOLESTR)wszPath,
                        NULL,
                        &pidl,
                        NULL)) &&
        SUCCEEDED(SHBindToParent(pidl,
                        IID_IShellFolder,
                        (void**)&psfParent,
                        (LPCITEMIDLIST*)&pidlRelative)) &&
        SUCCEEDED(psfParent->GetAttributesOf(1,
                        (LPCITEMIDLIST*)&pidlRelative,
                        &ulAttrib)) &&
        (ulAttrib & (SFGAO_FOLDER | SFGAO_BROWSABLE)))
    {
        hr = BrowseObject(pidl, SBSP_SAMEBROWSER | SBSP_ABSOLUTE);
    }
    else
    {
        hr = E_FAIL;
    }

    if (FAILED(hr))
    {
        goto leave;
    }

leave:
    if (pidl)
    {
        ILFree(pidl);
    }

    if (pidlRelative)
    {
        ILFree(pidlRelative);
    }

    if (psfParent)
    {
        psfParent->Release();
    }

    return hr;
}

int FileOpenDlg_t::NotifyTypeChange()
{
    int iRet = 0;
    OFNOTIFY openFileNotify;

    if (!m_pOpenFileName->lpfnHook)
    {
        goto leave;
    }

    // we are sending back the original structure (m_pOpenFileName)

    openFileNotify.hdr.hwndFrom = m_hwndDialog;
    openFileNotify.hdr.idFrom = 0;
    openFileNotify.hdr.code = CDN_TYPECHANGE;
    openFileNotify.lpOFN = m_pOpenFileName;
    openFileNotify.pszFile = NULL;

    m_pOpenFileName->lpfnHook(m_hwndDialog,
                    WM_NOTIFY,
                    0,
                    (LPARAM)&openFileNotify);

leave:
    return iRet;;
}

// pwszCurrentDir is a string indicating the current directory.
//      It must start with a slash. i.e. \windows\programs
// pwszInput is what the user has given us for input. i.e. ..\myfile
// pwszPath points to an output buffer which will hold the constructed path
//      i.e \windows\myfile
// RETURNS: TRUE on success, FALSE if unable to construct path
BOOL FileOpenDlg_t::ProcessDots(LPCWSTR pwszCurrentDir, LPWSTR pwszPath, LPCWSTR pwszInput)
{
    BOOL bRet = FALSE;

    LPWSTR pwszPathEnd;
    LPWSTR pwszDot;
    LPWSTR pwszSlash;
    WCHAR wChar;
    int numDots;
    size_t cch;

    PREFAST_DEBUGCHK(pwszCurrentDir);
    DEBUGCHK((::wcslen(pwszCurrentDir) > 0) && pwszCurrentDir[0] == L'\\');
    PREFAST_DEBUGCHK(pwszPath);
    PREFAST_DEBUGCHK(pwszInput);

    pwszPath[0] = L'\0';

    // check if there is enough room to create path
    cch = ::wcslen(pwszInput);
    if (pwszInput[0] != L'\\')
    {
        cch += ::wcslen(pwszCurrentDir) + 1;
    }

    if (cch >= MAX_PATH)
    {
        goto leave;
    }

    if (pwszInput[0] == L'\0')
    {
        ::wcscpy(pwszPath, pwszCurrentDir);
        bRet = TRUE;
        goto leave;
    }

    if (pwszInput[0] == L'\\')
    {
        pwszInput++;
        ::wcscpy(pwszPath, L"\\");
    }
    else
    {
        ::wcscpy(pwszPath, pwszCurrentDir);
        cch = ::wcslen(pwszPath);
        if ((cch > 0) &&  (L'\\' != pwszPath[cch-1]))
        {
            ::wcscat(pwszPath, L"\\");
        }
    }
    pwszPathEnd = pwszPath + ::wcslen(pwszPath);

    // check for dots
    while ((pwszDot = (LPWSTR)::wcschr(pwszInput, L'.')) != NULL)
    {
        // character before . should be beginning of line or slash
        if (pwszDot != pwszInput && *(pwszDot - 1) != L'\\')
        {
            goto unknownDotSequence;
        }

        // append to pwszPath all of pwszInput up until the dot
        if (pwszDot > pwszInput)
        {
            DEBUGCHK(pwszDot[0] == L'.');
            pwszDot[0] = L'\0';
            ::wcscpy(pwszPathEnd, pwszInput);
            pwszDot[0] = L'.';
            pwszPathEnd += (pwszDot - pwszInput);

            DEBUGCHK(pwszPathEnd[0] == L'\0');
            pwszInput = pwszDot;
        }

        // count dots
        for (numDots = 0; pwszDot[0] == L'.'; pwszDot++, numDots++)
            ;

        // check char after dots
        if (pwszDot[0] == L'\0')
        {
            // have (dots)EOL
            pwszInput = pwszDot;
            // continue on to remove slashes
        }
        else if (pwszDot[0] == L'\\')
        {
            // have (dots)SLASH
            pwszInput = pwszDot + 1;
            // continue on to remove slashes
        }
        else
        {
unknownDotSequence:
            // have (dots)? - append everything until the next slash
            pwszSlash = ::wcschr(pwszDot, L'\\');
            if (pwszSlash == NULL)
            {
                break;
            }

            pwszSlash++;
            DEBUGCHK(pwszDot >= pwszInput && pwszSlash > pwszDot);
            // copy everything up to and including the slash
            wChar = pwszSlash[0];
            pwszSlash[0] = L'\0';
            ::wcscpy(pwszPathEnd, pwszInput);
            pwszSlash[0] = wChar;

            // increment pointers and keep looking for dots
            pwszPathEnd += (pwszSlash - pwszInput);
            DEBUGCHK(pwszPathEnd[0] == 0);
            pwszInput = pwszSlash;
            continue;
        }

        // remove the same number of slashes as dots.
        // we assume pwszPath starts with a slash, and there will always
        // be a slash in pwszPath as long as it is non-zero length
        DEBUGCHK(pwszPath[0] == L'\\');
        DEBUGCHK(pwszPathEnd > pwszPath);
        while (numDots-- > 0)
        {
            pwszSlash = ::wcsrchr(pwszPath, L'\\');
            // stop if there are no more slashes or we have a network path
            if (!pwszSlash || pwszSlash == (pwszPath + 1))
            {
                break;
            }
            pwszPathEnd = pwszSlash;
            pwszPathEnd[0] = L'\0';
        }

        // restore trailing slash
        ::wcscpy(pwszPathEnd, L"\\");
        pwszPathEnd++;
        DEBUGCHK(pwszPathEnd[0] == L'\0');
    }

    // append the rest of pwszInput to pwszPath
    ::wcscpy(pwszPathEnd, pwszInput);

    PathRemoveTrailingSlashes(pwszPath);
    bRet = TRUE;

leave:
    return bRet;
}

BOOL FileOpenDlg_t::Run(LPOPENFILENAME lpofn)
{
    BOOL bRetVal = FALSE;
    HANDLE hDlgTemplate = NULL;

    ASSERT(lpofn);
    if (!lpofn || lpofn->lStructSize != sizeof(OPENFILENAME) || !lpofn->lpstrFile)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto leave;
    }

    DEBUGMSG(ZONE_INFO,
                    (L"Entering FileOpen (Dir=%s, Filter=%s Ext=%s File=%s)\n",
                    lpofn->lpstrInitialDir,
                    lpofn->lpstrFilter,
                    lpofn->lpstrDefExt,
                    lpofn->lpstrFile));

    // ignore lpofn->lpstrCustomFilter
    // ignore lpofn->nMaxCustFilter
    // ignore lpofn->lCustData
    // ignore lpofn->lpfnHook (used by CE internally)

    m_pOpenFileName = lpofn;
    m_pwszTitle = (LPCWSTR)lpofn->lpstrTitle;
    m_pwszDefExt = NULL;

    if (lpofn->lpstrDefExt)
    {
        m_pwszDefExt = (LPWSTR)g_pShellMalloc->Alloc((1+::wcslen(lpofn->lpstrDefExt))*sizeof(WCHAR));
        if (!m_pwszDefExt)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto leave;
        }
        ::wcscpy(m_pwszDefExt, lpofn->lpstrDefExt);
    }

    m_wszDirPath[0] = L'\0';
    m_FolderSettings.ViewMode = FVM_LIST;
    m_FolderSettings.fFlags = 0;

    if (!InitGlobals())
    {
        goto exit;
    }

    if (!m_pwszTitle)
    {
        m_pwszTitle = (LPCWSTR)LOAD_STRING(m_bFileSave ? IDS_SHGOFN_SAVE : IDS_SHGOFN_OPEN);
    }

    if (m_pOpenFileName->nFilterIndex < 1)
    {
        m_pOpenFileName->nFilterIndex = 1;
    }

    m_pColumnWidths = s_ColumnWidthsVGA;
    m_ptbButtons = s_tbButtonVGA;
    m_cButtons = lengthof(s_tbButtonVGA);

    // choose our template
    if (lpofn->Flags & OFN_ENABLETEMPLATE)
    {
        // Both custom instance handle and the dialog template name are
        // user specified. Locate the dialog resource in the specified
        // instance block and load it.
        HANDLE hRes = FindResource(lpofn->hInstance, lpofn->lpTemplateName, RT_DIALOG);
        ASSERT(hRes);
        if (!hRes)
        {
            goto exit;
        }
        hDlgTemplate = LoadResource(lpofn->hInstance, (HRSRC) hRes);
    }
    else if (lpofn->Flags & OFN_ENABLETEMPLATEHANDLE)
    {
        // A handle to the pre-loaded resource has been specified.
        hDlgTemplate = lpofn->hInstance;
    }
    else
    {
        HANDLE hRes;
        int idd;

        BOOL IsQVGAEnabled = FALSE;
        hRes = FindResource(HINST_CESHELL, MAKEINTRESOURCE(IDR_QVGAENABLED), L"BOOL");
        if (hRes)
        {
            IsQVGAEnabled = *((BOOL*)LoadResource(HINST_CESHELL, (HRSRC)hRes));
        }

        idd = IDD_GETOPENFILENAME;

        // if SIP in use, always raise it, not just on Gryphon
        // but only switch templates if on Gryphon
        if (HaveSIP() && IsQVGAEnabled)
        {
            idd = IDD_GETOPENFILENAME_SIP;
        }

        if (IsQVGAEnabled)
        {
            m_pColumnWidths = s_ColumnWidthsQVGA;
            m_ptbButtons = s_tbButtonQVGA;
            m_cButtons = lengthof(s_tbButtonQVGA);
        }

        // Load the resource
        hRes = FindResource(HINST_CESHELL, MAKEINTRESOURCE(idd), RT_DIALOG);
        ASSERT(hRes);
        if (!hRes)
        {
            goto exit;
        }
        hDlgTemplate = LoadResource(HINST_CESHELL, (HRSRC)hRes);
    }

    ASSERT(hDlgTemplate);
    if (!hDlgTemplate || !LockResource(hDlgTemplate))
    {
        goto exit;
    }

    // Load aygshell for SipPref
    LoadAygshellLibrary();
    TTAddRef();

    DialogBoxIndirectParam(HINST_CESHELL, (LPDLGTEMPLATE) hDlgTemplate,
                           lpofn->hwndOwner, (DLGPROC)FileOpenDlg_t::DlgProc,
                           (LPARAM)this);

    TTRelease();
    FreeAygshellLibrary();

    bRetVal = m_bRetVal;

exit:
    UnInitGlobals();

    if (m_pwszDefExt)
    {
        g_pShellMalloc->Free(m_pwszDefExt);
    }

leave:
    return bRetVal;
}

BOOL FileOpenDlg_t::SetDefaultExtension()
{
    BOOL bRet = FALSE;
    LPWSTR pwszFilter;

    // get us a new default extension
    if (!m_pwszDefExt)
    {
        goto leave;
    }

    // look for a '.' in the filter string, we use the remaining chars as an extension.
    pwszFilter = m_wszCurrentFilter;
    while(*pwszFilter && *pwszFilter != L'.')
    {
        pwszFilter++;
    }

    if (!*pwszFilter)
    {
        goto leave;
    }

    // jump over the dot
    pwszFilter++;
    if (*pwszFilter &&
        (*pwszFilter != L'*' && *pwszFilter != L'?'))
    {
        // find the size of the filter string
        int count;
        for(count = 0; pwszFilter[count] && pwszFilter[count] != L';'; count++)
            ;

        LPWSTR pwszTmp =(LPWSTR)g_pShellMalloc->Realloc(m_pwszDefExt,
                        (1+count)*sizeof(WCHAR));
        if (pwszTmp)
        {
            // we successfully resized the pointer, so fill it.
            m_pwszDefExt = pwszTmp;
            memcpy(m_pwszDefExt, pwszFilter, count*sizeof(WCHAR));
            m_pwszDefExt[count] = L'\0';
        }
        else
        {
            g_pShellMalloc->Free(m_pwszDefExt);
            m_pwszDefExt = NULL;
            goto leave;
        }
    }

    bRet = TRUE;

leave:
    return bRet;
}

void FileOpenDlg_t::UnInitGlobals()
{
    if (m_pIShellDesktopFolder)
    {
        m_pIShellDesktopFolder->Release();
        m_pIShellDesktopFolder = NULL;
    }

    if (m_hiconFolderSM)
    {
        DestroyIcon(m_hiconFolderSM);
        m_hiconFolderSM = NULL;
    }

    if (m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = NULL;
    }

    if (m_hBGBrush)
    {
        DeleteObject(m_hBGBrush);
        m_hBGBrush = NULL;
    }

    if (m_sip.m_hCoreDll)
    {
        FreeLibrary(m_sip.m_hCoreDll);
        m_sip.m_hCoreDll = NULL;
        memset(&(m_sip), 0, sizeof(m_sip));
    }
}

void FileOpenDlg_t::UpdateToolbar(HWND hwndToolbar)
{
    OLECMD rgCmds[] = {
        {IDC_FILE_NEWFOLDER, 0},
        {IDC_VIEW_LIST, 0},
        {IDC_VIEW_DETAILS, 0},
        {IDC_GO_FOLDERUP, 0}
    };

    PREFAST_ASSERT(m_pIOleCommandTarget);
    m_pIOleCommandTarget->QueryStatus(&CGID_CEShell, lengthof(rgCmds), rgCmds, NULL);

    ::SendMessage(hwndToolbar,
                    TB_ENABLEBUTTON,
                    IDC_FILE_NEWFOLDER,
                    (rgCmds[0].cmdf & OLECMDF_ENABLED));

    ::SendMessage(hwndToolbar,
                    TB_CHECKBUTTON,
                    IDC_VIEW_LIST,
                    (rgCmds[1].cmdf & OLECMDF_LATCHED));

    ::SendMessage(hwndToolbar,
                    TB_CHECKBUTTON,
                    IDC_VIEW_DETAILS,
                    (rgCmds[2].cmdf & OLECMDF_LATCHED));

    ::SendMessage(hwndToolbar,
                    TB_ENABLEBUTTON,
                    IDC_GO_FOLDERUP,
                    (rgCmds[3].cmdf & OLECMDF_ENABLED));
}

BOOL CALLBACK FileOpenDlg_t::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL bHandled = FALSE;
    HRESULT hr;
    size_t cch;

    FileOpenDlg_t* pThis = (FileOpenDlg_t*)::GetWindowLong(hDlg, DWL_USER);

    if (!pThis && uMsg != WM_INITDIALOG)
    {
        goto leave;
    }

    switch (uMsg)
    {
        case WM_INITDIALOG:
            pThis = (FileOpenDlg_t*)lParam;
            ::SetWindowLong(hDlg, DWL_USER, (LONG)pThis);
            bHandled = pThis->InitDialog(hDlg);
            break;

        case WM_SETTINGCHANGE:
            if (wParam == SPI_SETSIPINFO)
            {
                pThis->CenterWindowSIPAware(hDlg, FALSE);
            }
            break;

        case WM_CTLCOLORSTATIC:
            SetTextColor((HDC)wParam, GetSysColor(COLOR_BTNTEXT));
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (LPARAM)pThis->m_hBGBrush;

        case WM_ERASEBKGND:
        {
            RECT rc;
            GetClientRect(pThis->m_hwndDialog, &rc);
            rc.top += CommandBar_Height(pThis->m_hwndToolbar);

            pThis->DrawPathBanner((HDC)wParam, &rc);

            bHandled = TRUE;
            break;
        }

        case WM_HELP:
            pThis->HandleCommand(IDC_FILEOPENHELP, NULL);
            bHandled = TRUE;
            break;

        case WM_COMMAND:
            ASSERT(hDlg == pThis->m_hwndDialog);
            bHandled = pThis->HandleCommand(wParam, lParam);
            break;

        case CDM_GETSPEC:
            {
                WCHAR tempName[MAX_PATH];

                cch = Edit_GetText(GetDlgItem(pThis->m_hwndDialog, IDC_FILEOPENNAME),
                                tempName,
                                lengthof(tempName));

                // is the request buffer is too small, inform the caller how big to make it
                if (lParam && (cch < wParam))
                {
                    // Note that StringCchCopy always null-terminates the buffer
                    hr = ::StringCchCopy((LPWSTR)lParam, wParam, tempName);
                    ASSERT(SUCCEEDED(hr)); // Failure is acceptable
                }
                ::SetWindowLong(hDlg, DWL_MSGRESULT, (LONG)cch+1);
                bHandled = TRUE;
                break;
            }

        case CDM_GETFOLDERPATH:
            // wParam is the maximum buffer size
            // lParam is the buffer
            {
                cch = 0;

                // is the request buffer is too small, inform the caller how big to make it
                if (SUCCEEDED(::StringCchLength(pThis->m_wszDirPath,
                                lengthof(pThis->m_wszDirPath),
                                &cch)) &&
                    (cch < wParam))
                {
                    // Note that StringCchCopy always null-terminates the buffer
                    hr = ::StringCchCopy((LPWSTR)lParam, wParam, pThis->m_wszDirPath);
                    ASSERT(SUCCEEDED(hr)); // Failure is acceptable
                }
                ::SetWindowLong(hDlg, DWL_MSGRESULT, (LONG)cch+1);
                bHandled = TRUE;
            }
            break;

        case CDM_SETCONTROLTEXT:
            // wParam is the control ID
            // lParam is a pointer to the text
            // let's support only the filename text
            switch(wParam)
            {
                case edt1:
                    if (lParam)
                    {
                        RETAILMSG(0, (L"CDM_SETCONTROLTEXT '%s':\r\n", lParam));
                        // set the edit text
                        Edit_SetText(GetDlgItem(hDlg, IDC_FILEOPENNAME), (LPWSTR)lParam);
                        UpdateWindow(GetDlgItem(hDlg, IDC_FILEOPENNAME));

                    }
                    break;
                default:
                    break;
           }
           break;

        case CDM_SETDEFEXT:
            // we can get this from the lpfnHook callback
            // Copying the string since we had to make a buffer for it
            // if the value passed in was null, kill our own string
            if (!lParam)
            {
                if (pThis->m_pwszDefExt)
                {
                    g_pShellMalloc->Free(pThis->m_pwszDefExt);
                    pThis->m_pwszDefExt = NULL;
                }
                break;
            }

            // if the value was non-nul, we can realloc, or crate a new string
            cch = ::wcslen((LPWSTR)lParam);

            if (pThis->m_pwszDefExt)
            {
                LPWSTR pwszTmp;

                pwszTmp = (LPWSTR)g_pShellMalloc->Realloc(pThis->m_pwszDefExt,
                                (cch+1)*sizeof(WCHAR));
                if (!pwszTmp)
                {
                    // Allocation fails
                    g_pShellMalloc->Free(pThis->m_pwszDefExt);
                }

                pThis->m_pwszDefExt = pwszTmp;
            }
            else
            {
                pThis->m_pwszDefExt = (LPWSTR)g_pShellMalloc->Alloc((cch+1)*sizeof(WCHAR));
            }

            // if the mem op was successful, copy the string
            if (pThis->m_pwszDefExt)
            {
                ::wcscpy(pThis->m_pwszDefExt, (LPWSTR)lParam);
            }
            RETAILMSG(0, (L"CDM_SETDEFEXT '%s':\r\n", pThis->m_pwszDefExt));
            break;

        case WM_NOTIFY:
            break;

        case WM_DESTROY:
            ::SetWindowLong(pThis->m_hwndStatic, GWL_WNDPROC, (DWORD)pThis->m_pfnStaticWndProc);
            pThis->DestroyActiveView();
            ::SetWindowLong(hDlg, DWL_USER, 0);
            break;
    }

leave:
    return bHandled;
}

LRESULT CALLBACK FileOpenDlg_t::StaticSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    HWND hwndDialog = GetParent(GetParent(hwnd));
    FileOpenDlg_t* pThis = (FileOpenDlg_t*)::GetWindowLong(hwndDialog, DWL_USER);

    if (!pThis)
    {
        ASSERT(0);
        goto leave;
    }

    switch (uMsg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            WCHAR wszCaption[MAX_PATH];
            HFONT hfontOld = NULL;
            RECT rc;

            BeginPaint(hwnd, &ps);
            if (ps.hdc)
            {
                if (pThis->m_hFont)
                {
                    hfontOld = (HFONT)SelectObject(ps.hdc, pThis->m_hFont);
                }

                GetClientRect(hwnd, &rc);
                FillRect(ps.hdc, &rc, pThis->m_hBGBrush);
                rc.left += 3;
                rc.top += 3;

                SetTextColor(ps.hdc, GetSysColor(COLOR_BTNTEXT));
                SetBkMode(ps.hdc, TRANSPARENT);

                GetWindowText(hwnd, wszCaption, MAX_PATH);
                ExtTextOut (ps.hdc,
                                rc.left,
                                rc.top,
                                0,
                                NULL,
                                wszCaption,
                                ::wcslen(wszCaption),
                                NULL);

                if (hfontOld)
                {
                    SelectObject(ps.hdc, hfontOld);
                }
            }
            EndPaint(hwnd, &ps);
            break;
        }

        default:
            lResult = CallWindowProc(pThis->m_pfnStaticWndProc, hwnd, uMsg, wParam, lParam);
    }

leave:
    return lResult;
}

LRESULT CALLBACK FileOpenDlg_t::ViewSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    HWND hwndDialog = GetParent(hwnd);
    FileOpenDlg_t* pThis = (FileOpenDlg_t*)::GetWindowLong(hwndDialog, DWL_USER);

    if (!pThis)
    {
        ASSERT(0);
        goto leave;
    }

    switch (uMsg)
    {
        case WM_NOTIFY:
            if (pThis->HandleListViewNotify(wParam, lParam, &lResult))
            {
                break;
            }
            // fall through if this was not handled

        default:
            lResult = CallWindowProc(pThis->m_pfnViewWndProc, hwnd, uMsg, wParam, lParam);
    }

    // If it is a ViewMode change, then update toolbar buttons
    if (uMsg == WM_COMMAND)
    {
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
            case IDC_VIEW_DETAILS:
            case IDC_VIEW_ICONS:
            case IDC_VIEW_LIST:
            case IDC_VIEW_SMALL:
                pThis->UpdateToolbar(pThis->m_hwndToolbar);
        }
    }

leave:
    return lResult;
}

DWORD FileOpenDlg_t::TTAddRef()
{
    if (s_TTRefCount == 0)
    {
        s_pwszTooltips[0] = (LPWSTR) LOAD_STRING(IDS_SHGOFN_TTSTR1); //Up
        s_pwszTooltips[1] = (LPWSTR) LOAD_STRING(IDS_NEWFOLDER); // NewFolder
        s_pwszTooltips[2] = (LPWSTR) LOAD_STRING(IDS_SHGOFN_TTSTR2); // List
        s_pwszTooltips[3] = (LPWSTR) LOAD_STRING(IDS_SHGOFN_TTSTR3); // Details
    }
    return ++s_TTRefCount;
}

DWORD FileOpenDlg_t::TTRelease()
{
    if (--s_TTRefCount == 0)
    {
        for (int i = 0; i < 4; i++)
        {
            s_pwszTooltips[i] = NULL;
        }
    }
    return s_TTRefCount;
}


//////////////////////////////////////////////////
// IUnknown

STDMETHODIMP FileOpenDlg_t::QueryInterface(REFIID riid, void** ppReturn)
{
    HRESULT hr = E_NOINTERFACE;

    if (!ppReturn)
    {
        ASSERT(0);
        hr = E_INVALIDARG;
        goto leave;
    }

    *ppReturn = NULL;

    if (IsEqualIID(riid, IID_IUnknown)) // IUnknown
    {
        *ppReturn = this;
    }
    else if (IsEqualIID(riid, IID_IShellBrowser)) // IShellBrowser
    {
        *ppReturn = (IShellBrowser *) this;
    }
    else if (IsEqualIID(riid, IID_IOleCommandTarget)) // IOleCommandTarget
    {
        *ppReturn = (IOleCommandTarget *) this;
    }

    if (*ppReturn)
    {
        (*(LPUNKNOWN*)ppReturn)->AddRef();
        hr = S_OK;
    }

leave:
    return hr;
}

STDMETHODIMP_(DWORD) FileOpenDlg_t::AddRef()
{
    return ++m_ObjRefCount;
}

STDMETHODIMP_(DWORD) FileOpenDlg_t::Release()
{
    if (--m_ObjRefCount == 0)
    {
        delete this;
        return 0;
    }

    return m_ObjRefCount;
}

//////////////////////////////////////////////////
// IOleWindow

STDMETHODIMP FileOpenDlg_t::GetWindow(HWND* phwnd)
{
    HRESULT hr;

    if (!phwnd)
    {
        hr = E_INVALIDARG;
        goto leave;
    }

    *phwnd = m_hwndDialog;
    hr = S_OK;

leave:
    return hr;
}


//////////////////////////////////////////////////
// IShellBrowser methods

STDMETHODIMP FileOpenDlg_t::BrowseObject(LPCITEMIDLIST pidl, UINT wFlags)
{
    HRESULT hr = E_FAIL;
    LPITEMIDLIST pidlNew = NULL;
    IShellFolder* pIShellFolderNew = NULL;
    IShellView* pIShellViewNew = NULL;
    IOleCommandTarget* pIOleCommandTargetNew = NULL;
    IEnumFilter* pIEnumFilterNew = NULL;
    IShellListView* pIShellListView = NULL;
    HWND hwndViewNew = NULL;
    int iWidths[4];
    int iCol;

    DECLAREWAITCURSOR;
    SetWaitCursor();

    if (!pidl && !(wFlags & SBSP_PARENT))
    {
        hr = E_INVALIDARG;
        goto leave;
    }

    switch (wFlags & (SBSP_ABSOLUTE | SBSP_RELATIVE | SBSP_PARENT))
    {
        case SBSP_RELATIVE:
            if (m_pidlFQ)
                pidlNew = ILConcatenate(m_pidlFQ, pidl);
            break;

        case SBSP_PARENT:
            // Clone parent
            if (m_pidlFQ)
            {
                pidlNew = ILCopy(m_pidlFQ, IL_ALL);
                ILRemoveLast(pidlNew);
            }
            break;

        case SBSP_ABSOLUTE:
            pidlNew = ILCopy(pidl, IL_ALL);
            break;

        default:
            ASSERT(FALSE);
            goto leave;
    }

    if (!pidlNew)
    {
        goto leave;
    }

    // Create the new ShellView
    hr = SHBindToObject(m_pIShellDesktopFolder,
                    IID_IShellFolder,
                    pidlNew,
                    (void**)&pIShellFolderNew);
    if (FAILED(hr))
    {
        goto error;
    }

    hr = pIShellFolderNew->CreateViewObject(m_hwndDialog,
                    IID_IShellView,
                    (void**)&pIShellViewNew);
    if (FAILED(hr))
    {
        goto error;
    }

    hr = pIShellViewNew->QueryInterface(IID_IOleCommandTarget,
                    (void**)&pIOleCommandTargetNew);
    if (FAILED(hr))
    {
        goto error;
    }

    hr = pIShellViewNew->QueryInterface(IID_IEnumFilter,
                    (void**)&pIEnumFilterNew);
    if (FAILED(hr))
    {
        goto error;
    }

    if (FAILED(pIShellViewNew->QueryInterface(IID_IShellListView,
                    (void**)&pIShellListView)))
    {
        goto error;
    }

    // Note that SetFilterString() will reset EnumFilter on failure
    pIEnumFilterNew->SetFilterString(m_wszCurrentFilter);

    // Set column widths for details view
    for (iCol = 0; iCol < 4; iCol++)
    {
        iWidths[iCol] = m_pColumnWidths[iCol];
        if (iCol == 1)
        {
            iWidths[iCol] -= GetSystemMetrics(SM_CXVSCROLL);
        }
    }
    pIShellListView->SetColumnWidths(iWidths, lengthof(iWidths));

    pIShellListView->EnablePersistentViewMode(FALSE);

    if (m_pIShellView)
    {
        m_pIShellView->GetCurrentInfo(&m_FolderSettings);
    }
    m_FolderSettings.fFlags |= FWF_AUTOARRANGE | FWF_SINGLESEL;

    hr = pIShellViewNew->CreateViewWindow(m_pIShellView,
                    &m_FolderSettings,
                    (IShellBrowser*)this,
                    &m_rcView,
                    &hwndViewNew);
    if (FAILED(hr) || !hwndViewNew)
    {
        goto error;
    }

    // Switch views
    DestroyActiveView();

    m_pidlFQ = pidlNew;
    m_hwndView = hwndViewNew;
    m_pIShellFolder = pIShellFolderNew;
    m_pIShellView = pIShellViewNew;
    m_pIShellListView = pIShellListView;
    m_pIEnumFilter = pIEnumFilterNew;
    m_pIOleCommandTarget = pIOleCommandTargetNew;

    ActivatePendingView();

leave:
    ResetWaitCursor();
    return hr;

error:
    if (pidlNew)
    {
        ILFree(pidlNew);
    }

    if (pIShellListView)
    {
        pIShellListView->Release();
    }

    if (pIEnumFilterNew)
    {
        pIEnumFilterNew->Release();
    }

    if (pIOleCommandTargetNew)
    {
        pIOleCommandTargetNew->Release();
    }

    if (pIShellViewNew)
    {
        pIShellViewNew->Release();
    }

    if (pIShellFolderNew)
    {
        pIShellFolderNew->Release();
    }

    goto leave;
}

//////////////////////////////////////////////////
// IOleCommandTarget methods

STDMETHODIMP FileOpenDlg_t::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    HRESULT hr;

    if (pguidCmdGroup)
    {
        hr = OLECMDERR_E_UNKNOWNGROUP;
        goto leave;
    }

    // Handle standard group commands
    switch (nCmdID)
    {
        case OLECMDID_CLOSE:
            PostMessage(m_hwndDialog, WM_CLOSE, 0, 0);
            hr = S_OK;
            break;

        default:
            hr = OLECMDERR_E_NOTSUPPORTED;
            break;
    }

leave:
    return hr;
}

// Helper functions to obtain NameSpaces from the registry given a DisplayName

BOOL FileOpenDlg_t::FindNameInClassesRoot(LPCWSTR pwszNameSpace, LPCWSTR pwszDisplayName, size_t cchToCompare)
{
    BOOL bRet = FALSE;
    WCHAR wszValName[MAX_PATH];
    WCHAR wszCLSID[256] = L"CLSID\\";
    DWORD dwType = 0;
    DWORD cbValName;
    HKEY hKey = NULL;

    if (FAILED(::StringCchCat(wszCLSID, 256, pwszNameSpace)))
    {
        goto Leave;
    }

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, wszCLSID, 0, 0, &hKey))
    {
        cbValName = MAX_PATH * sizeof(WCHAR);
        if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                        s_RegDisplayName,
                        NULL,
                        &dwType,
                        (LPBYTE)wszValName,
                        &cbValName))
        {
            if (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT,
                            NORM_IGNORECASE | NORM_IGNOREWIDTH,
                            pwszDisplayName,
                            cchToCompare,
                            wszValName,
                            -1))
            {
                bRet = TRUE;
            }
        }
        RegCloseKey(hKey);
    }

Leave:
    return bRet;
}


//    GetParsingName
//
//    @parm LPCWSTR | pwszDisplayName | Name for which to retrieve its ParsingName
//    @parm LPWSTR | pwszParsingName | (Out) buffer to store the ParsingName
//    @parm PUINT | pcchOutBuf | (In/Out) pointer to buffer size in characters
//
//    @rdesc If the function finds a match, the return value is TRUE.
//        pcchOutBuf's value is either equal to the number of copied chars or the
//        required buffer size (if pwszParsingName parameter is not large enough).
//        If the function fails, the return value is FALSE. 

BOOL FileOpenDlg_t::GetParsingName(LPCWSTR pwszDisplayName, LPWSTR pwszParsingName, size_t* pcchOutBuf)
{
    static const WCHAR pNameSpaceHolder[2][25]= {
        {L"Explorer\\Desktop"},
        {L"Explorer\\MyDevice"}
    };

    WCHAR wszValName[256];
    LPWSTR pwszValData= NULL;
    LPCWSTR pszEndName= NULL;
    size_t cchInput, cchToCompare , cchToCpy;
    DWORD dwType = 0;
    DWORD cbValName, cbData;
    DWORD dwIndex, i;
    HKEY hKey;
    LONG lRes = !ERROR_SUCCESS;

    if (!pwszDisplayName || !pwszParsingName || !pcchOutBuf)
    {
        goto Leave;
    }

    if (pwszDisplayName[0] == L'\\')
    {
        goto Leave;
    }

    pwszValData= (LPWSTR)g_pShellMalloc->Alloc(sizeof(WCHAR)*MAX_PATH);
    if (!pwszValData)
    {
        goto Leave;
    }

    cchToCpy = *pcchOutBuf;

    // Init to negative number
    cbValName = (DWORD)-1;

    // Strip the possible namespace. Tail will be concatenated to Parsing name in case of a match
    cchInput = cchToCompare = ::wcslen(pwszDisplayName);
    pszEndName= ::wcschr(pwszDisplayName, L'\\');
    if (pszEndName)
    {
        cchToCompare = (pszEndName - pwszDisplayName);
    }

    // Look up in our cache's set first
    hKey = NULL;
    lRes = RegOpenKeyEx(HKEY_CURRENT_USER, s_RegGUIDs, 0, 0, &hKey);
    if (hKey && lRes==ERROR_SUCCESS)
    {
        dwIndex = 0;
        cbValName = 256;
        cbData = sizeof(WCHAR)*MAX_PATH;    
        
        while(ERROR_SUCCESS == (lRes =RegEnumValue(hKey,
                        dwIndex,
                        wszValName,
                        &cbValName,
                        NULL,
                        &dwType,
                        (BYTE *)pwszValData,
                        &cbData)))
        {
            dwIndex++;
            if ((0 != ::wcscmp(wszValName, s_szBitBucketCLSID)) &&
                CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT,
                            NORM_IGNORECASE | NORM_IGNOREWIDTH,
                            pwszDisplayName,
                            cchToCompare,
                            pwszValData,
                            -1))
            {
                cchToCpy = min((cbValName+1), *pcchOutBuf);
                lRes = ::StringCchCopy(pwszParsingName, cchToCpy, wszValName);
                break;
            }
            cbValName = 256;
            cbData = sizeof(WCHAR)*MAX_PATH;
        }
        RegCloseKey(hKey);
    }

    if (lRes == ERROR_SUCCESS)
    {
        // We have a match
        goto ExitParsingName;
    }

    // Compare with the Root
    if (FindNameInClassesRoot(s_szDesktopCLSID, pwszDisplayName, cchToCompare))
    {
        cchToCpy = min(lengthof(s_szDesktopCLSID), *pcchOutBuf);
        lRes = ::StringCchCopy(pwszParsingName, cchToCpy, s_szDesktopCLSID);
        goto ExitParsingName;
    }
    
    // As a last chance try a match on Explorer Namespace Holders
    for(i=0; i<2; i++)
    {
        hKey= NULL;
        lRes= RegOpenKeyEx(HKEY_LOCAL_MACHINE, pNameSpaceHolder[i], 0, 0, &hKey);
        if (hKey && lRes==ERROR_SUCCESS)
        {
            dwIndex= 0;
            cbValName = 256;
            while(ERROR_SUCCESS == (lRes= RegEnumValue(hKey,
                            dwIndex,
                            wszValName,
                            &cbValName,
                            NULL,
                            NULL,
                            NULL,
                            NULL)))
            {
                DWORD cbCLSID = cbValName;

                if ((0 != ::wcscmp(wszValName, s_szBitBucketCLSID)) &&
                    FindNameInClassesRoot(wszValName, pwszDisplayName, cchToCompare))
                {
                    cchToCpy = min((cbCLSID+1), *pcchOutBuf);
                    lRes = ::StringCchCopy(pwszParsingName, cchToCpy, wszValName);
                    cbValName = cbCLSID; // characters copied or needed
                    break;
                }
                dwIndex++;
                cbValName = 256;
            }
            RegCloseKey(hKey);
        }

        if (lRes == ERROR_SUCCESS)
        {
            // We have a match
            goto ExitParsingName;
        }
    }
    
ExitParsingName:
    g_pShellMalloc->Free(pwszValData);

    if (lRes == ERROR_SUCCESS)
    {
        if (pszEndName && (*pcchOutBuf > (cchToCpy + cchInput - cchToCompare)))
        {
            ::wcscat(pwszParsingName, pszEndName);
        }
        
        // characters copied or needed;
        *pcchOutBuf = cbValName+1;
    }

 Leave:
    return !(BOOL)lRes;
}

