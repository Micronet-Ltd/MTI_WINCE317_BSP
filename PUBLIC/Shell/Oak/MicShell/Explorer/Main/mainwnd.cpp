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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  mainwnd.cpp

Abstract:  Implements the main window, the container for the webbrowser

Functions:

Notes: Most of the code resides here. The container and its interaction with the webbrowser control,
        commandbar, statusbar etc.


--*/
#include <windows.h>
#include <intsafe.h>
// Include the automation definitions...
#include <exdisp.h>
#include <exdispid.h>
#include <mshtmdid.h> // AMBIENT_DLCONTROL

#include <objbase.h>

#include <tchar.h>

#include <wininet.h>
#include <urlmon.h>
#include <afdfunc.h>    //ras stuff
#include <pkfuncs.h>    // GetOwnerProcess
#include <mshtml.h>
#include <commctrl.h>
#include <commdlg.h>
#include "resource.h"
#include "filemenu.h"
#include "curlfile.h"
#include "MainWnd.h"
#include "utils.h"
#include "animthrd.h"

#include "desktop.h"
#include "taskbar.hxx" // taskbar

#include "mshtmcid.h"
#include "shdisp.h"

#include <hshell.h>
#include <shellsdk.h>

#include "aygutils.h"
#include "urlmonutils.hpp"
#include "explorerlist.hpp"

#ifndef _PREFAST_
#pragma warning(disable:4068)
#endif // _PREFAST_

enum {
    SHDVID_GETSYSIMAGEINDEX = 18
};

extern "C" {
HWND
WINAPI
CommandBar_GetItemWindow(HWND hwndCB, int iButton);

BOOL IEUsesDCOM();
}


static HRESULT FindString();
HRESULT HandleNewWindow2(LPWSTR lpszUrl, LPCITEMIDLIST pidl, DISPPARAMS FAR* pdparams);
BOOL RegisterMainWnd();
BOOL RegisterDesktopWnd();

BOOL HandleBrowse(HWND hwndOwner, WCHAR *szURL);

extern LONG        glThreadCount;
extern HANDLE      ghExitEvent;
extern HINSTANCE   g_hInstance;
extern DWORD       g_dwMainWindowStackSize;
extern BOOL        g_bBrowseInPlace;
extern BOOL        g_bQVGAEnabled;

//EXTERN_C const GUID CGID_CEShell;     in shdisp.h
EXTERN_C const GUID CGID_MSHTML;
EXTERN_C const GUID CGID_ShellDocView;

extern "C" BOOL APIENTRY OpenURLDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam);

// Registry Keys
static const WCHAR CmdBands_RegKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\CmdBands";
static const WCHAR BandID_RegValue[] = L"ID";
static const WCHAR BandStyle_RegValue[] = L"Break";
static const WCHAR BandWidth_RegValue[] = L"Width";
static const WCHAR BandMax_RegValue[] = L"Max";
static const WCHAR StatusBar_RegKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StatusBar";
static const WCHAR ShowStatusBar_RegValue[] = L"ShowStatusBar";
static const WCHAR TypedUrls_RegKey[] = L"Software\\Microsoft\\Internet Explorer\\TypedURLs";
//static const WCHAR szGUIDs[] = L"\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\CurrentGUIDs";     in utils.cpp

extern const TBBUTTON s_tbBtns[];
extern const int s_iBtns;
extern const UINT s_idrMenu;

DWORD CMainWnd::_TTRefCount = 0;
LPWSTR *CMainWnd::_pszTooltips = 0;


// Shared Menu
static MENUDATA const filemenuData[] = {
        {MF_STRING, ID_CLOSE, IDS_FILE_CLOSE}
};
#define FILE_MENU_ITEMS   (sizeof(filemenuData)/sizeof(filemenuData[0]))

static MENUDATA const viewmenuData[] = {
        {MF_STRING, ID_VIEW_REFRESH, IDS_VIEW_REFRESH},
        {MF_SEPARATOR, 0, 0},
        {MF_STRING, ID_ADDRESSBAR_TOGGLE, IDS_VIEW_ADDRESSBAR},
        {MF_STRING, IDM_STATUSBAR_TOGGLE, IDS_VIEW_STATUSBAR}
};
#define VIEW_MENU_ITEMS   (sizeof(viewmenuData)/sizeof(viewmenuData[0]))

static MENUDATA const gomenuData[] = {
    {MF_STRING, ID_GO_BACK, IDS_GO_BACK},
    {MF_STRING, ID_GO_FORWARD, IDS_GO_FORWARD},
};
#define GO_MENU_ITEMS   (sizeof(gomenuData)/sizeof(gomenuData[0]))

static MENUDATA const favmenuData[] = {
    {MF_STRING, ID_FAV_PLACEHOLDER, IDS_EMPTY},
};
#define FAV_MENU_ITEMS   (sizeof(favmenuData)/sizeof(favmenuData[0]))

DWORD WINAPI NewWindow(LPVOID pParam)
{

    CMainWnd *pWnd = (CMainWnd *)pParam;
    MSG msg;
    BOOL bRet;

    HRESULT hr;

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    bRet = pWnd->Create();

    if (IEUsesDCOM() && pWnd->_pBrowser)
    {
         hr = CoMarshalInterThreadInterfaceInStream(
                        IID_IDispatch,
                        (LPUNKNOWN)pWnd->_pBrowser,
                        &pWnd->_pStrm );
        if (hr != S_OK)
        {
            pWnd->_pStrm = NULL;
        }
    }

    SetEvent(pWnd->hEvent);

    if (!bRet)
    {
        pWnd->_pBrowser = NULL;
        return 0;
    }

    pWnd->ResizeBrowserWindow();

    while (GetMessage( &msg, NULL, 0, 0 ) )
    {
        if (msg.message == WM_QUIT)
        {
            break;
        }

        if (!pWnd->PreTranslateMessage(&msg) && !(msg.message == WM_CHAR && msg.wParam == VK_TAB))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    pWnd->Release();

    CoUninitialize();

    InterlockedDecrement(&glThreadCount);
    SetEvent(ghExitEvent);

    return msg.wParam;
}

HRESULT HandleNewWindow2(LPWSTR lpszUrl, LPCITEMIDLIST pidl, DISPPARAMS FAR* pdparams)
{
    HANDLE hThread;
    CMainWnd *pNewWnd;
    IDispatch *pDispatch;
    HRESULT hr = S_OK;
    int cch;

    pNewWnd = new CMainWnd;
    if (!pNewWnd)
    {
        hr = E_OUTOFMEMORY;
        goto Leave;
    }

    if (lpszUrl)
    {
        cch = _tcslen(lpszUrl);
        
        if (cch >= MAX_URL) cch = MAX_URL-1;

        pNewWnd->_lpszUrl = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * (cch+1));

        if (!pNewWnd->_lpszUrl ||
            StringCchCopy(pNewWnd->_lpszUrl, cch+1, lpszUrl) != S_OK )
        {
            delete pNewWnd;
            hr = E_FAIL;
            goto Leave;
        }
    }
    pNewWnd->_pidl = (LPITEMIDLIST)pidl;

    if (!pdparams)
    {
        pNewWnd->_bEmpty = TRUE;
    }

    pNewWnd->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (!pNewWnd->hEvent)
    {
        delete pNewWnd;
        hr = E_OUTOFMEMORY;
        goto Leave;
    }

    InterlockedIncrement(&glThreadCount);

    hThread = CreateThread(NULL, g_dwMainWindowStackSize, NewWindow, (LPVOID)pNewWnd, STACK_SIZE_PARAM_IS_A_RESERVATION, NULL);

    if (!hThread)
    {
        delete pNewWnd;
        InterlockedDecrement(&glThreadCount);
        hr = E_OUTOFMEMORY;
        goto Leave;
    }

    WaitForSingleObject(pNewWnd->hEvent, INFINITE);
    CloseHandle(hThread);
    if (pdparams)
    {
        if (IEUsesDCOM() && pNewWnd->_pStrm)
        {
            CoGetInterfaceAndReleaseStream(
                            pNewWnd->_pStrm,
                            IID_IDispatch,
                            (LPVOID *)&pDispatch);

            pNewWnd->_pStrm = NULL;
        }
        else if (pNewWnd->_pBrowser)
        {
            hr = pNewWnd->_pBrowser->QueryInterface(IID_IDispatch, (LPVOID *)(&pDispatch));
        }
        else
        {
            hr = E_FAIL;
            pDispatch = NULL;
        }

        *(pdparams->rgvarg[0].pboolVal) = 0;
        *(pdparams->rgvarg[1].ppdispVal) = pDispatch;
    }

Leave:
    if (hr == E_OUTOFMEMORY)
    {
        SHShowOutOfMemory(NULL, 0);
    }
    return hr;
}



BOOL CMainWnd::_LoadBrowserHelperObjects(void)
{
    BOOL bRet = FALSE;
    HKEY hkey;
    if (_pBrowser &&
        RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects"),0,0,  &hkey) == ERROR_SUCCESS)
    {
        WCHAR szGUID[64];
        DWORD cb = ARRAYSIZE(szGUID);
        for (int i = 0;
             RegEnumKeyEx(hkey, i, szGUID, &cb, NULL, NULL, NULL, NULL) == ERROR_SUCCESS;
             i++)
        {
            CLSID clsid;
            IObjectWithSite *pows;
            if ( GUIDFromString(szGUID, &clsid) &&
                SUCCEEDED(CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IObjectWithSite, (void **)&pows)))
            {
                pows->SetSite(_pBrowser);    // give the poinetr to IWebBrowser2

                BSTR strClsid;
                // now register this object so that it can be found through automation.
                strClsid = SysAllocString(szGUID);

                VARIANT varUnknown = {0};
                varUnknown.vt = VT_UNKNOWN;
                varUnknown.punkVal = pows;
                _pBrowser->PutProperty(strClsid, varUnknown);

                pows->Release(); // Instead of calling variantClear()

                bRet = TRUE;
            }
            cb = ARRAYSIZE(szGUID);
        }
        RegCloseKey(hkey);
    }
    return bRet;
}




CMainWnd::CMainWnd()
{
    _lpszUrl = NULL;
    _bFullScreen = FALSE;
    _bEmpty = FALSE;
    _szTitle[0] = 0;
    _bShowCmdBar = TRUE;
    _bShowAddrBar = TRUE;
    _bShowStatusBar = FALSE;
    _bClosing = FALSE;
    _bInWebBrowserMode = TRUE;
    _wZoom = 2; // default zoom
    _wDLCCounter = 0; // counter for Download Completes
    _pInternetSecurityManager = NULL;
    _dwZone = 3;
    _pthrdAnimation = NULL;
    _bShowAnimation = FALSE;
    _hWndCmdband= NULL;
    _hWndAddressCombo= NULL;
    _himlCmdBand = NULL;

}


CMainWnd::~CMainWnd()
{
    if (_lpszUrl)
    {
        LocalFree(_lpszUrl);
    }

    if (_Favorites)
    {
        delete _Favorites;
    }

}

BOOL CMainWnd::IsWebBrowser()
{
    IDispatch *pDisp = NULL;
    IHTMLDocument2 *pDoc = NULL;
    BOOL bRet = FALSE;

    _pBrowser->get_Document(&pDisp);
    if (pDisp)
    {
        pDisp->QueryInterface(IID_IHTMLDocument2, (LPVOID *)&pDoc);
        pDisp->Release();
    }

    if (pDoc)
    {
        bRet = TRUE;
        pDoc->Release();
    }

    return bRet;
}

BOOL CMainWnd::UpdateZoneInfo()
{
    BSTR bstrUrl = NULL;
    HRESULT hr = E_FAIL;
    DWORD dwZone;
    
    if (!_pBrowser || !_pInternetSecurityManager)
    {
        goto leave;
    }
    
    hr = _pBrowser->get_LocationURL(&bstrUrl);
    if (FAILED(hr))
    {
        goto leave;
    }
        
    hr = _pInternetSecurityManager->MapUrlToZone((LPWSTR)bstrUrl, &dwZone, 0 );
    if (FAILED(hr))
    {
        goto leave;
    }

    _dwZone = dwZone;

leave:
    if (bstrUrl)
    {
        SysFreeString(bstrUrl);
    }

    return SUCCEEDED(hr);
}


void CMainWnd::ShowZoneInfo()
{
    if (_dwZone > 4)
    {
        SendMessage( _hWndStatus, SB_SETICON, (WPARAM)4, (LPARAM)NULL);
        SendMessage(_hWndStatus,
                        SB_SETTEXT,
                        (WPARAM)4,
                        (LPARAM)LoadString(g_hInstance, IDS_UNKNOWN, NULL, 0));
        return;
    }

    if (!_bInWebBrowserMode)
    {
        BSTR bstrUrl = NULL;
        BOOL IsNetworkPath = FALSE;

        _pBrowser->get_LocationName(&bstrUrl);
        if (bstrUrl)
        {
            IsNetworkPath = (0 == wcsncmp((LPCTSTR)bstrUrl, L"\\\\", 2));
            SysFreeString(bstrUrl);
        }

        if (!IsNetworkPath)
        {
            // don't override zone set by the view
            return;
        }
    }

    if (_bShowStatusBar)
    {
        SendMessage( _hWndStatus,
                        SB_SETICON,
                        (WPARAM)4,
                        (LPARAM)UrlmonUtils_t::GetStatusIcon(_dwZone));

        SendMessage(_hWndStatus,
                        SB_SETTEXT,
                        (WPARAM)4,
                        (LPARAM)UrlmonUtils_t::GetStatusZone(_dwZone));
    }
}

BOOL CMainWnd::Create()
{
    _hAccelTbl = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
    _Favorites = new CFavoritesMenu();

    UrlmonUtils_t::LoadUrlmonLibrary();

    // get the internet security manager from urlmon
    UrlmonUtils_t::CoInternetCreateSecurityManager(NULL, &_pInternetSecurityManager, 0);

    // Init Tooltips
    TTAddRef();

    LoadToolbarSettings(L"IE");
    LoadToolbarSettings(L"SH");

    HKEY hKey = NULL;
    LRESULT lRet = RegOpenKeyEx(HKEY_CURRENT_USER, StatusBar_RegKey, 0, 0, &hKey);
    if (lRet == ERROR_SUCCESS)
    {
        DWORD cbDataReg = sizeof(DWORD);
        DWORD dwValue;
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, ShowStatusBar_RegValue, NULL, NULL, (BYTE*)&dwValue, &cbDataReg))
        {
            _bShowStatusBar = (dwValue != 0);
        }
        RegCloseKey(hKey);
    }

    _bShowHelp = (GetFileAttributes(L"\\Windows\\peghelp.exe") != -1);

    BOOL bRet;
#ifndef BROWSE_IN_WINDOW
    _bFullScreen = TRUE;
    bRet = CBrowseObj::Create( WS_VISIBLE | WS_MINIMIZEBOX | WS_MAXIMIZEBOX );
#else
    bRet = CBrowseObj::Create();
#endif //BROWSE_IN_WINDOW

    if (_bShowHelp)
    {
        SetWindowLong(_hWnd,
                        GWL_EXSTYLE,
                        WS_EX_CONTEXTHELP | GetWindowLong(_hWnd, GWL_EXSTYLE));
    }

    return bRet;
}

BOOL CMainWnd::CreateStatusBar()
{
    BOOL bRet = FALSE;
    if (!(_hWndStatus = CreateStatusWindow(WS_CHILD|WS_VISIBLE, _T("Ready"), _hWnd, ID_STATUSBAR)))
    {
        goto leave;
    }

    RECT rc;
    GetClientRect(_hWndStatus, &rc);
    int naParts[5];

    naParts[4] = rc.right;
    naParts[3] = MAX(0, rc.right - MESSAGEPANEWIDTH);
    naParts[2] = MAX(0, rc.right - MESSAGEPANEWIDTH - ICONPANEWIDTH);
    naParts[1] = MAX(0, rc.right - MESSAGEPANEWIDTH - ICONPANEWIDTH - SECURITYPANEWIDTH);
    naParts[0] = MAX(0, rc.right - MESSAGEPANEWIDTH - ICONPANEWIDTH - SECURITYPANEWIDTH - PROGRESSPANEWIDTH);

    SendMessage(_hWndStatus, SB_SETPARTS, (WPARAM )5, (LPARAM)naParts);

    SendMessage(_hWndStatus, SB_GETRECT, 1, (LPARAM )&rc);
    InflateRect(&rc, -1,-1);

    _hWndProgress = CreateWindowEx(0,
                    PROGRESS_CLASS,
                    _T(""),
                    WS_CHILD|WS_VISIBLE|PBS_SMOOTH,
                    naParts[0],
                    rc.top,
                    PROGRESSPANEWIDTH,
                    rc.bottom - rc.top,
                    _hWndStatus,
                    NULL,
                    g_hInstance,
                    NULL);
    if (!_hWndProgress)
    {
        goto leave;
    }
    SendMessage(_hWndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

    if (!_bShowStatusBar)
    {
        // make this appear like a toggle
        _bShowStatusBar = TRUE;
        ShowStatusBar(FALSE);
    }

    bRet = TRUE;

leave:
    return bRet;
}

void CMainWnd::ResizeBrowserWindow()
{
    RECT rcStatus, rcWnd;
    int cbHeight = 0;
    int sbHeight = 0;

    if (_bShowStatusBar)
    {
        GetClientRect(_hWndStatus, &rcStatus);
        sbHeight = rcStatus.bottom - rcStatus.top;
    }

    if (_bShowCmdBar)
    {
        cbHeight = CommandBands_Height(_hWndCmdband);
    }

    GetClientRect(_hWnd, &rcWnd);

    SetWindowPos(_hWndBrowser,
                        NULL,
                        0,
                        cbHeight,
                        rcWnd.right-rcWnd.left,
                        (rcWnd.bottom - rcWnd.top)-sbHeight-cbHeight,
                        SWP_NOZORDER);

}

BOOL CMainWnd::ShowAddressBar(BOOL fShow)
{
    BOOL bRet = FALSE;
    REBARBANDINFO rbbi;
    int index = 0;

    if (!fShow == !_bShowAddrBar)
    {
        bRet = TRUE;
        goto Leave;
    }

    rbbi.fMask = RBBIM_STYLE|RBBIM_ID;
    rbbi.cbSize = sizeof(REBARBANDINFO);

    index = SendMessage(_hWndCmdband, RB_IDTOINDEX, (WPARAM)ID_BAND_ADDR, 0);
    SendMessage(_hWndCmdband, RB_GETBANDINFO, (WPARAM)index, (LPARAM)&rbbi);
    if (rbbi.wID == ID_BAND_ADDR)
    {
        if (fShow)
        {
            rbbi.fStyle &= ~RBBS_HIDDEN;
        }
        else
        {
            rbbi.fStyle |= RBBS_HIDDEN;
        }

        if (SendMessage(_hWndCmdband, RB_SETBANDINFO, (WPARAM)index, (LPARAM)&rbbi))
        {
            _bShowAddrBar = !!fShow;
            bRet = TRUE;
        }
    }

Leave:
    return bRet;
}

BOOL CMainWnd::ShowStatusBar(BOOL fShow)
{
    if (!fShow  ^ ! _bShowStatusBar)
    {
        ShowWindow(_hWndStatus, (fShow ? SW_SHOW : SW_HIDE));
        _bShowStatusBar = !!fShow;
    }

    if (fShow)
    {
        ShowZoneInfo();
    }

    return TRUE;
}

BOOL CMainWnd::CreateCommandBar()
{
    BOOL bOK = FALSE;

    if(!_bClosing)
    {
        // Create IE command bar - by default
        bOK= CreateIECommandBar(_bFullScreen, (TBBUTTON *)s_tbBtns, s_iBtns, s_idrMenu);
    }
    return bOK;
}

BOOL CMainWnd::AdjustCommandBar()
{
    ASSERT(_hWndCmdband);

    if (_bFullScreen)
    {
        CommandBands_AddAdornments(_hWndCmdband,
                        g_hInstance,
                        (_bShowHelp ? CMDBAR_HELP : 0),
                        0);
    }
    else
    {
        HWND hwnd;
        UINT index;

        // Remove Addornments from Commandband
        index= SendMessage(_hWndCmdband, RB_IDTOINDEX, (WPARAM)0xFFFFFFFF, 0);
        hwnd= CommandBands_GetCommandBar(_hWndCmdband, index);
        SendMessage(_hWndCmdband, RB_DELETEBAND, (WPARAM)index, (LPARAM)0);
        DestroyWindow(hwnd);
    }

    return TRUE;
}

BOOL CMainWnd::CreateIECommandBar(BOOL fCloseBox, TBBUTTON *tbBtns, int numButtons, long idMenu)
{
    BOOL bRet = FALSE;
    RECT rc;
    HBITMAP hbm;
    LPWSTR pszAddress = NULL;
    TBBUTTON *tbBtns_temp = NULL;
    
    if (_himlCmdBand)
    {
        ImageList_Destroy(_himlCmdBand);
        _himlCmdBand = NULL;
    }

    if (_hWndAddressCombo)
    {
        SaveTypedUrls();

        // Save current address
        pszAddress = (LPWSTR)LocalAlloc(LMEM_FIXED, MAX_URL * sizeof(WCHAR));
        if (pszAddress)
        {
            pszAddress[0] = 0;
            GetWindowText(_hWndAddressCombo, pszAddress, MAX_URL);
        }
    }

    if (_hWndCmdband)
    {
        DestroyWindow(_hWndCmdband);
        _hWndCmdband = NULL;

        // The children got destroyed as well on previous call
        _hWndMenuBar = NULL;
        _hWndToolBar = NULL;
        _hWndAddressCombo = NULL;
        _hWndAddressEdit = NULL;
    }

    _himlCmdBand = ImageList_Create(11, 13, ILC_COLOR, 1, 0);
    hbm = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_COOLBARIMAGES));
    if (hbm)
    {
        ImageList_Add(_himlCmdBand, hbm, NULL);
        DeleteObject(hbm);
    }

    _hWndCmdband = CommandBands_Create(g_hInstance, _hWnd, (UINT)ID_COMMANDBAND,
                                       RBS_SMARTLABELS|RBS_BANDBORDERS|RBS_AUTOSIZE, _himlCmdBand);
    if (!_hWndCmdband)
    {
        goto Leave;
    }



    // Restore information
    int i, iCmd = -1, iTool = -1, iAddr = -1;


    // Check for RTL-ness of the main window
    if (GetWindowLong(_hWnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
    {
        TBBUTTON tbBtn_swap;
        ULONG    cbAlloc;

        int iFwd = -1, iBack = -1;

        if (SUCCEEDED(ULongMult((ULONG)numButtons, sizeof(TBBUTTON), &cbAlloc)))
        {
            // create a temporary local copy of the button array
            tbBtns_temp = (TBBUTTON*)LocalAlloc(0, cbAlloc);
            if(tbBtns_temp)
            {
                // copy the input array
                memcpy(tbBtns_temp, tbBtns, numButtons*sizeof(TBBUTTON));
                // find the FWD/BACK buttons
                for(i=0;i<numButtons;i++)
                {
                    switch(tbBtns_temp[i].idCommand)
                    {
                        case ID_GO_BACK:
                            iBack = i;
                            break;
                        case ID_GO_FORWARD:
                            iFwd = i;
                            break;
                    }
                    if(iBack != -1 && iFwd != -1)
                    {
                        break;
                    }
                }
                // swap them 
                if(iFwd != -1 && iBack != -1)
                {
                    memcpy(&tbBtn_swap, &tbBtns_temp[iFwd], sizeof(TBBUTTON));
                    memcpy(&tbBtns_temp[iFwd], &tbBtns_temp[iBack], sizeof(TBBUTTON));
                    memcpy(&tbBtns_temp[iBack], &tbBtn_swap, sizeof(TBBUTTON));

                    // set tbBtns to the local array for use below
                    tbBtns = tbBtns_temp;
                }
            }
        }
    }    
    
    for (i = 0; i < CMDBANDS; i++)
    {
        switch(_cbRestoreInfo[0][i].wID)
        {
            case ID_BAND_CMD:
                iCmd = i;
                CreateMenuBar(idMenu, _cbRestoreInfo[0][iCmd].fStyle, _cbRestoreInfo[0][iCmd].cxRestored);
                break;

            case ID_BAND_TOOL:
                iTool = i;
                CreateToolBar(tbBtns, numButtons, _cbRestoreInfo[0][iTool].fStyle, _cbRestoreInfo[0][iTool].cxRestored);
                break;

            case ID_BAND_ADDR:
                iAddr = i;
                _bShowAddrBar = (_cbRestoreInfo[0][iAddr].fStyle & RBBS_HIDDEN) ? FALSE:TRUE;
                CreateAddressBar(_cbRestoreInfo[0][iAddr].fStyle, _cbRestoreInfo[0][iAddr].cxRestored);
                break;

            case ID_BAND_INVALID:
                break;
        }
    }

    // If not found, create using default settings
    if (iCmd == -1)
    {
        CreateMenuBar(idMenu, (UINT)-1, (UINT)-1);
    }

    if (iTool == -1)
    {
        CreateToolBar(tbBtns, numButtons, (UINT)-1, (UINT)-1);
    }

    if (iAddr == -1)
    {
        CreateAddressBar((UINT)-1, (UINT)-1);
    }

    // Maximize if needed
    UINT bandIndex;
    if (iCmd != -1 && _cbRestoreInfo[0][iCmd].fMaximized)
    {
        SendMessage(_hWndCmdband, RB_MAXIMIZEBAND, (WPARAM)0, (LPARAM)0);
    }

    if (iTool != -1 && _cbRestoreInfo[0][iTool].fMaximized)
    {
        bandIndex = SendMessage(_hWndCmdband, RB_IDTOINDEX, (WPARAM)ID_BAND_TOOL, (LPARAM)0);
        SendMessage(_hWndCmdband, RB_MAXIMIZEBAND, (WPARAM)bandIndex, (LPARAM)0);
    }

    if (iAddr != -1 && _cbRestoreInfo[0][iAddr].fMaximized)
    {
        bandIndex = SendMessage(_hWndCmdband, RB_IDTOINDEX, (WPARAM)ID_BAND_ADDR, (LPARAM)0);
        SendMessage(_hWndCmdband, RB_MAXIMIZEBAND, (WPARAM)bandIndex, (LPARAM)0);
    }

    // Restore address if any
    if (pszAddress && _hWndAddressCombo)
    {
        SendMessage(_hWndAddressCombo, WM_SETTEXT, 0, (LPARAM)pszAddress);
    }

    // Add Adornments in full screen mode
    if (fCloseBox)
    {
        CommandBands_AddAdornments(_hWndCmdband,
                        g_hInstance,
                        (_bShowHelp ? CMDBAR_HELP : 0),
                        0);
    }

    if (!_pthrdAnimation)
    {
        _pthrdAnimation = new CAnimThread(_hWnd);
        if (!_pthrdAnimation)
        {
            goto Leave;
        }

        _pthrdAnimation->StartThread();
         // event is signaled by StartThread after the window is created
        WaitForSingleObject(_pthrdAnimation->_hEvent, INFINITE);
    }

    GetWindowRect(_hWnd, &rc);
    rc.top += CommandBands_Height(_hWndCmdband);
    _pthrdAnimation->PositionWindow(  &rc );

    _bInWebBrowserMode = TRUE;

    bRet = TRUE;

Leave:
    if(tbBtns_temp)
    {
        LocalFree(tbBtns_temp);
    }   

    if (pszAddress)
    {
        LocalFree(pszAddress);
    }

    return bRet;
}


BOOL CMainWnd::CreateMenuBar(LONG idMenu, UINT fStyle, UINT cxWidth)
{
    BOOL bRet = FALSE;
    REBARBANDINFO rbbi;
    ASSERT(_hWndCmdband);

    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask = RBBIM_ID|RBBIM_IMAGE|RBBIM_STYLE;
    rbbi.fStyle = RBBS_NOGRIPPER;
    rbbi.wID = ID_BAND_CMD;
    rbbi.iImage = 1;

    if (cxWidth != (UINT)-1)
    {
        rbbi.fMask |= RBBIM_SIZE;
        rbbi.cx = cxWidth;
    }

    // Ensure that menu is always on 0 position
    if (!SendMessage(_hWndCmdband, RB_GETBANDCOUNT, (WPARAM)0, (LPARAM)0))
    {
        if (!CommandBands_AddBands(_hWndCmdband, g_hInstance, 1, &rbbi))
        {
            goto leave;
        }
    }
    else
    {
        if (!SendMessage(_hWndCmdband, RB_INSERTBAND, (WPARAM)0, (LPARAM)&rbbi))
        {
            goto leave;
        }
    }

    _hWndMenuBar = CommandBands_GetCommandBar(_hWndCmdband, 0);
    if (!CommandBar_InsertMenubar(_hWndMenuBar, g_hInstance, (UINT)idMenu, 0))
    {
        // delete band
        goto leave;
    }

    if (cxWidth == (UINT)-1)
    {
        RECT rc;
        HWND hwndMenu = CommandBar_GetItemWindow(_hWndMenuBar, 0);
        GetClientRect(hwndMenu, &rc);

        rbbi.cx = rc.right - rc.left + 5;
        rbbi.fMask = RBBIM_SIZE;
        SendMessage(_hWndCmdband, RB_SETBANDINFO, (WPARAM)0, (LPARAM)&rbbi);
    }

    bRet = TRUE;

leave:
    return bRet;
}

BOOL CMainWnd::CreateToolBar(TBBUTTON *tbBtns, int numButtons, UINT fStyle, UINT cxWidth)
{
    BOOL bRet = FALSE;
    REBARBANDINFO rbbi;
    ASSERT(_hWndCmdband);

    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask = RBBIM_ID|RBBIM_IMAGE|RBBIM_SIZE;
    rbbi.wID = ID_BAND_TOOL;
    rbbi.iImage = 0;

    if (cxWidth == (UINT)-1)
    {
        REBARBANDINFO rbbiCmd;
        RECT rc;
        GetClientRect(_hWnd, &rc);

        rbbiCmd.cbSize = sizeof(REBARBANDINFO);
        rbbiCmd.fMask = RBBIM_SIZE;
        SendMessage(_hWndCmdband, RB_GETBANDINFO, (WPARAM)0, (LPARAM)&rbbiCmd);

        rbbi.cx = rc.right - rbbiCmd.cx;
    }
    else
    {
        rbbi.fMask |= RBBIM_STYLE;
        rbbi.fStyle = fStyle;
        rbbi.cx = cxWidth;
    }

    if (!CommandBands_AddBands(_hWndCmdband, g_hInstance, 1, &rbbi))
    {
        goto leave;
    }

     _hWndToolBar = CommandBands_GetCommandBar(_hWndCmdband, 1);
    if ((CommandBar_AddBitmap(_hWndToolBar, g_hInstance, IDB_TBBITMAP, 16, 16, 16)) < 0)
    {
        goto leave;
    }

    if (!CommandBar_AddButtons(_hWndToolBar, numButtons, tbBtns))
    {
        goto leave;
    }

    CommandBar_AddToolTips(_hWndToolBar, s_iBtns, _pszTooltips);
    bRet = TRUE;

leave:
    return bRet;
}

BOOL CMainWnd::CreateAddressBar(UINT fStyle, UINT cxWidth)
{
    BOOL bRet= TRUE;
    DWORD dwExStyle = 0;

    ASSERT(_hWndCmdband);

    // If the parent window is mirrored then the combo box window will inherit the mirroring flag
    // And we need the reading order to be Left to right, which is the right to left in the mirrored mode.

    if (GetWindowLong(_hWnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
    {
        // This means left to right reading order because this window will be mirrored.
        dwExStyle |= WS_EX_RTLREADING;
    }

    _hWndAddressCombo = CreateWindowEx(dwExStyle, TEXT("combobox"), TEXT(""),
                         WS_VISIBLE | WS_CHILD |WS_TABSTOP| CBS_AUTOHSCROLL,
                         0, 0, 20, 110, _hWnd,
                         (HMENU)ID_ADDRESSBAR,
                         g_hInstance, NULL);
    if (_hWndAddressCombo)
    {
        LoadTypedUrls();

        // setup the edit control of the combobox
        _hWndAddressEdit = GetDlgItem(_hWndAddressCombo, ID_COMBOEDIT);
        if (_hWndAddressEdit)
        {
            SetWindowLong(_hWndAddressEdit, GWL_USERDATA, (DWORD)this);

            _lpfnAddressEditProc = (WNDPROC )SetWindowLong(_hWndAddressEdit,
                                                   GWL_WNDPROC, (LONG)AddressEditProc);
            SendMessage(_hWndAddressEdit, EM_LIMITTEXT, MAX_URL-1, 0L);
        }
    }

    REBARBANDINFO rbbi;
    WCHAR szAddressLabel[40];
    RECT rcComboBox;
    GetWindowRect(_hWndAddressCombo, &rcComboBox);

    LoadString(g_hInstance, IDS_ADDRESS, szAddressLabel, 40);
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask = RBBIM_CHILD | RBBIM_TEXT | RBBIM_ID | RBBIM_STYLE |
                RBBIM_CHILDSIZE | RBBIM_IMAGE;
    rbbi.wID = ID_BAND_ADDR;
    rbbi.cxMinChild = 0;
    rbbi.cyMinChild = rcComboBox.bottom - rcComboBox.top;
    rbbi.iImage = 2;
    rbbi.hwndChild = _hWndAddressCombo;
    rbbi.lpText = (LPWSTR)szAddressLabel;

    if (cxWidth == (UINT)-1)
    {
        rbbi.fStyle = RBBS_BREAK;
    }
    else
    {
        rbbi.fMask |= RBBIM_SIZE;
        rbbi.cx = cxWidth;
        rbbi.fStyle = fStyle;
    }

    if (!_bShowAddrBar)
    {
        rbbi.fStyle |= RBBS_HIDDEN;
    }
    else
    {
        rbbi.fStyle &= ~RBBS_HIDDEN;
    }

    AygAddSipprefControl( _hWndAddressCombo );

    if (!CommandBands_AddBands(_hWndCmdband, g_hInstance, 1, &rbbi))
    {
        bRet= FALSE;
        if (_hWndAddressCombo)
        {
            DestroyWindow(_hWndAddressCombo);
            _hWndAddressCombo = NULL;
        }
    }

    return bRet;
}

STDMETHODIMP CMainWnd::GetControlWindow(UINT id, HWND* lphwnd)
{
    HRESULT hr = S_OK;

    if (!lphwnd)
    {
        ASSERT(0);
        hr = E_INVALIDARG;
        goto leave;
    }

    switch(id)
    {
        case FCW_TOOLBAR:
            *lphwnd= _hWndToolBar;
            break;

        case FCW_STATUS:
            *lphwnd= _hWndStatus;
            break;

        case FCW_PROGRESS:
            *lphwnd= _hWndProgress;
            break;

        default:
            hr = E_FAIL;
    }

leave:
    return hr;
}

STDMETHODIMP CMainWnd::InsertMenusSB(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    HRESULT hr = E_FAIL;
    HMENU hmenuSub;
    WCHAR szText[MAX_PATH];

    if (!hmenuShared || !lpMenuWidths)
    {
        ASSERT(0);
        hr = E_INVALIDARG;
        goto leave;
    }

    // Create FileMenu
    hmenuSub= CreateSubMenu((LPMENUDATA)&filemenuData, FILE_MENU_ITEMS, &(lpMenuWidths->width[0]));
    if (hmenuSub)
    {
        LoadString(g_hInstance, IDS_FILE, szText, MAX_PATH);
        if (!AppendMenu(hmenuShared, MF_POPUP, (UINT_PTR)hmenuSub, szText))
        {
            DestroyMenu(hmenuSub);
            lpMenuWidths->width[0]= 0;
            goto leave;
        }
    }

    //Create EditMenu (empty)
    hmenuSub= CreatePopupMenu();
    if (hmenuSub)
    {
        LoadString(g_hInstance, IDS_EDIT, szText, MAX_PATH);
        if (!AppendMenu(hmenuShared, MF_POPUP, (UINT_PTR)hmenuSub, szText))
        {
            DestroyMenu(hmenuSub);
            goto leave;
        }
    }

    // Create ViewMenu
    hmenuSub= CreateSubMenu((LPMENUDATA)&viewmenuData, VIEW_MENU_ITEMS, &(lpMenuWidths->width[2]));
    if (hmenuSub)
    {
        LoadString(g_hInstance, IDS_VIEW, szText, MAX_PATH);
        if (!AppendMenu(hmenuShared, MF_POPUP, (UINT_PTR)hmenuSub, szText))
        {
            DestroyMenu(hmenuSub);
            lpMenuWidths->width[2]= 0;
            goto leave;
        }
    }

    //Create Go Menu
    hmenuSub= CreateSubMenu((LPMENUDATA)&gomenuData, GO_MENU_ITEMS, &(lpMenuWidths->width[3]));
    HMENU hmenuGo = hmenuSub;
    if (hmenuSub)
    {
        LoadString(g_hInstance, IDS_GO, szText, MAX_PATH);
        if (!AppendMenu(hmenuShared, MF_POPUP, (UINT_PTR)hmenuSub, szText))
        {
            DestroyMenu(hmenuSub);
            lpMenuWidths->width[3]= 0;
            goto leave;
        }
    }

    //Create Favorites Menu
    hmenuSub= CreateSubMenu((LPMENUDATA)&favmenuData, FAV_MENU_ITEMS, &(lpMenuWidths->width[4]));
    if (hmenuSub)
    {
        LoadString(g_hInstance, IDS_FAVORITES, szText, MAX_PATH);

        BOOL bInserted;
        if (g_bQVGAEnabled && hmenuGo)
        {
            InsertMenu(hmenuGo, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
            bInserted = InsertMenu(hmenuGo, 0, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hmenuSub, szText);
        }
        else
        {
            bInserted = AppendMenu(hmenuShared, MF_POPUP, (UINT_PTR)hmenuSub, szText);
        }

        if (!bInserted)
        {
            DestroyMenu(hmenuSub);
            lpMenuWidths->width[4]= 0;
            goto leave;
        }
    }

    hr = S_OK;

leave:
    return hr;
}

STDMETHODIMP CMainWnd::RemoveMenusSB(HMENU hmenuShared)
{
    HMENU hmenuSub;
    ASSERT(hmenuShared);

    // Save shared settings in our class member
    SaveToolbarSettings(L"SH", FALSE);

    while ((hmenuSub=GetSubMenu(hmenuShared, 0)) != NULL)
    {
        RemoveMenu(hmenuShared, 0, MF_BYPOSITION);
        DestroyMenu(hmenuSub);
    }

    CreateCommandBar();
    return S_OK;
}

STDMETHODIMP CMainWnd::SendControlMsg(UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plRet)
{
    HRESULT hr = E_FAIL;
    LRESULT lRet = 0;

    switch(id)
    {
        case FCW_TOOLBAR:
            if (!_hWndToolBar)
            {
                goto Leave;
            }
            lRet = SendMessage(_hWndToolBar, uMsg, wParam, lParam);
            break;

        case FCW_STATUS:
            if (!_hWndStatus)
            {
                goto Leave;
            }
            lRet = SendMessage(_hWndStatus, uMsg, wParam, lParam);
            break;

        default:
            goto Leave;
    }

    hr = S_OK;

    if (plRet)
    {
        *plRet = lRet;
    }

Leave:
    return hr;
}

STDMETHODIMP CMainWnd::SetMenuSB(HMENU hmenuShared, HOLEMENU holemenuRes, HWND hwndActiveObject)
{
    HRESULT hr = E_FAIL;
    REBARBANDINFO rbbi;
    RECT rc;
    DWORD dwStyle;

    ASSERT(_hWndCmdband);

    if (!hmenuShared /*||!hwndActiveObject*/)
    {
        goto leave;
    }

    // Save IE settings in our class member
    SaveToolbarSettings(L"IE", FALSE);

    if (_hWndMenuBar)
    {
        SendMessage(_hWndCmdband, RB_DELETEBAND, (WPARAM)0, (LPARAM)0);
        DestroyWindow(_hWndMenuBar);
    }

    // Restore information
    int iCmd;
    for (iCmd= 0; iCmd< CMDBANDS && (ID_BAND_CMD != _cbRestoreInfo[1][iCmd].wID); iCmd++);

    // Create Menubar Window
    dwStyle = WS_CHILD | WS_VISIBLE | TBSTYLE_LIST | CCS_NORESIZE |TBSTYLE_TOOLTIPS;
    _hWndMenuBar = CreateWindow(TOOLBARCLASSNAME,
                    NULL,
                    dwStyle,
                    0,
                    0,
                    0,
                    0,
                    _hWndCmdband,
                    (HMENU)0,
                    g_hInstance,
                    0);
    if (!_hWndMenuBar)
    {
        goto leave;
    }

    SendMessage(_hWndMenuBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

    // Insert the window in the Cmdband.
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask = RBBIM_ID |RBBIM_IMAGE | RBBIM_STYLE |RBBIM_CHILD | RBBIM_CHILDSIZE;
    rbbi.fStyle = RBBS_NOGRIPPER;
    rbbi.cxMinChild = 0;
    rbbi.cyMinChild = 24; // The button height is 22 but the toolbar height is 24
    rbbi.wID = ID_BAND_CMD;
    rbbi.iImage = 1;
    rbbi.hwndChild = _hWndMenuBar;

    if (iCmd != CMDBANDS)
    {
        rbbi.fMask |= RBBIM_SIZE;
        rbbi.cx = _cbRestoreInfo[1][iCmd].cxRestored;
    }

    if (!SendMessage(_hWndCmdband, RB_INSERTBAND, (WPARAM)0, (LPARAM)&rbbi))
    {
        goto leave;
    }

    // Insert Composite menu
    if (!CommandBar_InsertMenubarEx(_hWndMenuBar, NULL, (LPWSTR)hmenuShared, 0))
    {
        goto leave;
    }

    // Compute Menubar size.
    if (iCmd == CMDBANDS)
    {
        HWND hwndMenu = CommandBar_GetItemWindow(_hWndMenuBar, 0);
        GetClientRect(hwndMenu, &rc);

        rbbi.cx = rc.right - rc.left + 5;
        rbbi.fMask = RBBIM_SIZE;
        SendMessage(_hWndCmdband, RB_SETBANDINFO, (WPARAM)0, (LPARAM)&rbbi);
    }

    //OleSetMenuDescriptor(holemenuRes, _hWnd, hwndActiveObject, (IOleInPlaceFrame*)(IShellBrowser*)this, _pIPActiveObj);
    _bInWebBrowserMode = FALSE;

    hr = S_OK;

leave:
    return hr;
}

STDMETHODIMP CMainWnd::SetToolbarItems(LPTBBUTTONSB lpButtons, UINT nButtons, UINT uFlags)
{
    HRESULT hr = E_FAIL;
    REBARBANDINFO rbbi, rbbiTemp;;
    DWORD dwStyle;
    UINT bandIndex;

    ASSERT(_hWndCmdband);

    if (uFlags & FCT_CONFIGABLE)
    {
        //FCT_CONFIGABLE
        goto leave;
    }

    if (_hWndToolBar)
    {
        bandIndex = SendMessage(_hWndCmdband, RB_IDTOINDEX, (WPARAM)ID_BAND_TOOL, (LPARAM)0);
        SendMessage(_hWndCmdband, RB_DELETEBAND, (WPARAM)bandIndex, (LPARAM)0);
        DestroyWindow(_hWndToolBar);
    }

    // Create Toolbar Window
    dwStyle = WS_CHILD | WS_VISIBLE | TBSTYLE_LIST | CCS_NORESIZE |TBSTYLE_TOOLTIPS;
    _hWndToolBar = CreateWindow(TOOLBARCLASSNAME, NULL, dwStyle, 0, 0,
                                 0, 0, _hWndCmdband, (HMENU)0, g_hInstance, 0);

    if (!_hWndToolBar)
    {
        goto leave;
    }
    SendMessage(_hWndToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbiTemp.cbSize = sizeof(REBARBANDINFO);

    // Insert the window in the Cmdband.
    rbbi.fMask = RBBIM_ID |RBBIM_IMAGE|RBBIM_SIZE |RBBIM_CHILD | RBBIM_CHILDSIZE;
    rbbi.wID = ID_BAND_TOOL;
    rbbi.cxMinChild = 0;
    rbbi.cyMinChild = 24; // The button height is 22 but the toolbar height is 24
    rbbi.iImage = 0;
    rbbi.hwndChild = _hWndToolBar;

    // Restore information
    int i, iCmd = -1, iTool = -1, iAddr = -1;
    for (i= 0; i< CMDBANDS; i++)
    {
        switch(_cbRestoreInfo[1][i].wID)
        {
            case ID_BAND_CMD:
                iCmd = i;
                break;

            case ID_BAND_TOOL:
                iTool = i;
                rbbi.fMask |= RBBIM_STYLE;
                rbbi.fStyle = _cbRestoreInfo[1][iTool].fStyle;
                rbbi.cx = _cbRestoreInfo[1][iTool].cxRestored;
                if (iAddr != -1 && iTool > iAddr)
                {
                    if (!CommandBands_AddBands(_hWndCmdband, g_hInstance, 1, &rbbi))
                    {
                        goto leave;
                    }
                }
                else
                {
                    if (!SendMessage(_hWndCmdband, RB_INSERTBAND, (WPARAM)1, (LPARAM)&rbbi))
                    {
                        goto leave;
                    }
                }
                break;

            case ID_BAND_ADDR:
                iAddr = i;
                rbbiTemp.fMask = RBBIM_STYLE | RBBIM_SIZE;
                rbbiTemp.fStyle = _cbRestoreInfo[1][iAddr].fStyle;
                rbbiTemp.cx = _cbRestoreInfo[1][iAddr].cxRestored;

                _bShowAddrBar = (_cbRestoreInfo[1][iAddr].fStyle & RBBS_HIDDEN) ? FALSE:TRUE;

                bandIndex =  SendMessage(_hWndCmdband, RB_IDTOINDEX, (WPARAM)ID_BAND_ADDR, (LPARAM)0);
                SendMessage(_hWndCmdband, RB_SETBANDINFO, bandIndex, (LPARAM)&rbbiTemp);
                break;

            case ID_BAND_INVALID:
                break;
        }
    }

    // If not found, create using default settings
    if (iTool == -1)
    {
        // Compute Toolbar size (Main window size - Menubar size)
        RECT rc;
        GetClientRect(_hWnd, &rc);

        rbbiTemp.fMask = RBBIM_SIZE;
        rbbiTemp.cx = 0;
        SendMessage(_hWndCmdband, RB_GETBANDINFO, (WPARAM)0, (LPARAM)&rbbiTemp);

        rbbi.cx= rc.right - rbbiTemp.cx;

        if (!SendMessage(_hWndCmdband, RB_INSERTBAND, (WPARAM)1, (LPARAM)&rbbi))
        {
            goto leave;
        }
    }

    if (uFlags & FCT_MERGE)
    {
        // Merge the toolbar items with those provided by IE
        // Available bitmaps will be Explorer toolbar, IDB_STD_SMALL_COLOR and IDB_VIEW_SMALL_COLOR
        // Indexes in TBBUTTON structures must reflect appropiate offset
        if ((CommandBar_AddBitmap(_hWndToolBar, g_hInstance, IDB_TBBITMAP, 16, 16, 16)) < 0)
        {
            goto leave;
        }
    }//else FCT_ADDTOEND

    // Add Standar and View bitmaps
    if ((CommandBar_AddBitmap(_hWndToolBar, HINST_COMMCTRL, IDB_STD_SMALL_COLOR, 15, 16, 16)) < 0)
    {
        goto leave;
    }

    if ((CommandBar_AddBitmap(_hWndToolBar, HINST_COMMCTRL, IDB_VIEW_SMALL_COLOR, 12, 16, 16)) < 0)
    {
        goto leave;
    }

    if (!CommandBar_AddButtons(_hWndToolBar, nButtons, lpButtons))
    {
        goto leave;
    }

    // By this time all shared bands are supposed to be in place. Maximize if needed
    if (iCmd != -1 && _cbRestoreInfo[1][iCmd].fMaximized)
    {
        SendMessage(_hWndCmdband, RB_MAXIMIZEBAND, (WPARAM)0, (LPARAM)0);
    }

    if (iTool != -1 && _cbRestoreInfo[1][iTool].fMaximized)
    {
        bandIndex = SendMessage(_hWndCmdband, RB_IDTOINDEX, (WPARAM)ID_BAND_TOOL, (LPARAM)0);
        SendMessage(_hWndCmdband, RB_MAXIMIZEBAND, (WPARAM)bandIndex, (LPARAM)0);
    }

    if (iAddr != -1 && _cbRestoreInfo[1][iAddr].fMaximized)
    {
        bandIndex =  SendMessage(_hWndCmdband, RB_IDTOINDEX, (WPARAM)ID_BAND_ADDR, (LPARAM)0);
        SendMessage(_hWndCmdband, RB_MAXIMIZEBAND, (WPARAM)bandIndex, (LPARAM)0);
    }

    hr = S_OK;

leave:
    return hr;
}

STDMETHODIMP CMainWnd::TranslateAccelerator(  MSG *pMsg, DWORD grfModifiers)
{
    HRESULT hr = S_FALSE;

    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB)
    {
        HWND hwndFocus = GetFocus();
        if (hwndFocus != _hWndAddressEdit)
        {
            SetFocus(_hWndAddressEdit);
        }
        hr = S_OK;
    }
    return hr;
}

HWND CMainWnd::CreateBrowser()
{
    VARIANT varURL;
    HWND hwndBrowser;
    HRESULT hr = S_FALSE;

    V_VT(&varURL) = VT_EMPTY;

    hwndBrowser = CBrowseObj::CreateBrowser();

    ExplorerList_t::AddExplorerWnd(_hWnd);

    if (_bEmpty && _pBrowser)
    {
        int iSize = 0;
        if (_pidl)
        {
            hr = InitVariantFromIDList(&varURL, _pidl);
            _pidl = NULL;
            hr = S_OK;
        }
        else if (_lpszUrl)
        {
            V_VT(&varURL) = VT_BSTR;
            varURL.bstrVal = SysAllocString(_lpszUrl);

            // don't need this anymore
            LocalFree(_lpszUrl);
            _lpszUrl = NULL;
            hr = S_OK;
        }

        if (hr == S_OK)
        {
            _pBrowser->Navigate2(&varURL, NULL, NULL, NULL, NULL);
        }
        else
        {
            _pBrowser->GoHome();
        }

        VariantClear(&varURL);
    }

    return hwndBrowser;
}



VOID CMainWnd::Close()
{

    _bClosing = TRUE;
    // the animation window is a child of the browser window so clean it up first.
    // the destructor will not return until the child thread has terminated. 
    if(_pthrdAnimation)
    {
        delete _pthrdAnimation;
        _pthrdAnimation = NULL;
    }

    // Cleanup favorites menu
    if(_Favorites)
    {
	_Favorites->Cleanup();
    }

    // superclass cleanup
    CBrowseObj::Close();

    ExplorerList_t::RemoveExplorerWnd(_hWnd);

    // Save CmdBar settings on the registry
    SaveToolbarSettings(L"IE", TRUE);
    SaveToolbarSettings(L"SH", TRUE);

    HKEY hKey = NULL;
    DWORD dwDisp;

    LRESULT lRet = RegCreateKeyEx(HKEY_CURRENT_USER, StatusBar_RegKey, 0, NULL, 0, 0, NULL, &hKey, &dwDisp);
    if (lRet == ERROR_SUCCESS)
    {
        DWORD dwTemp = (_bShowStatusBar) ? 1: 0;
        RegSetValueEx(hKey, ShowStatusBar_RegValue, 0, REG_DWORD, (LPBYTE)&dwTemp, sizeof(DWORD));
        RegCloseKey(hKey);
    }

    if (_hWndAddressCombo)
    {
        SaveTypedUrls();
    }

    if (_himlCmdBand)
    {
        ImageList_Destroy(_himlCmdBand);
        _himlCmdBand = NULL;
    }

    if (_hWndCmdband)
    {
        DestroyWindow(_hWndCmdband);
    }

    // DeInit Tooltips
    TTRelease();

    // Release the security manager before freeing up Urlmon
    if (_pInternetSecurityManager)
    {
        _pInternetSecurityManager->Release();
    }

    UrlmonUtils_t::FreeUrlmonLibrary();
}

#if 0 // componentized to extui.cpp
BOOL CMainWnd::HandleMenuPopup(WPARAM wParam, LPARAM lParam)
{
    // for WM_INITMENUPOPUP
    OLECMD rgcmd[] = {
        { OLECMDID_CUT, 0 },
        { OLECMDID_COPY, 0 },
        { OLECMDID_PASTE, 0 }
    };

    HMENU hmenu = CommandBar_GetMenu(_hWndMenuBar, 0);
    HMENU hSubMenu = GetSubMenu(hmenu, 1);
    IOleCommandTarget* pcmdt;

    if (hSubMenu == (HMENU)wParam)
    {
        // enable / disable edit menu commands
        int i;

        if (GetFocus()==_hWndAddressEdit)
        {
            DWORD dwStart, dwEnd;

            SendMessage(_hWndAddressEdit, EM_GETSEL, (WPARAM)(&dwStart), (LPARAM)(&dwEnd));
            if (dwEnd - dwStart > 1)
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

            for (i=0; i<sizeof(rgcmd)/sizeof(OLECMD); i++)
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

    hSubMenu = GetSubMenu(hmenu, 4);
    // Favorites
    if (hSubMenu == (HMENU)wParam)
    {
        if (!_Favorites)
        {
            _Favorites = new CFavoritesMenu;
            if (!_Favorites)
            {
                goto Exit;
            }
        }
        _Favorites->CreateFavoritesMenu((HMENU)wParam);
        _bInMenuLoop = TRUE;
    }
    else
    {
        if (_bInMenuLoop)
        {
            _Favorites->HandleInitMenuPopup((HMENU)wParam);
        }
    }
 Exit:
    return TRUE;
}
#endif

void CMainWnd::HandleSharedMenuPopup(HMENU hMenu)
{
    BOOL bUpdate= FALSE;
    int nItems, iCmd, i, n;
    OLECMD *prgCmds, *pOleCmd;
    IOleCommandTarget* pcmdt;

    if (!_pBrowser)
    {
        return;
    }

    nItems= GetMenuItemCount(hMenu);
    prgCmds = (OLECMD*)LocalAlloc(LPTR, nItems * sizeof(OLECMD));
    if (!prgCmds)
    {
        return;
    }
    pOleCmd= prgCmds;

    if (SUCCEEDED(_pBrowser->QueryInterface(IID_IOleCommandTarget, (LPVOID*) &pcmdt)))
    {
        // Fill olecmd array
        for (i=0, n=0; i<nItems; i++)
        {
            iCmd= GetMenuItemID(hMenu, i);

            // Check if it belongs to shell
            if (IDM_CESHELL__FIRST__ <= iCmd && IDM_CESHELL__LAST__ >= iCmd)
            {
                n++;
                pOleCmd->cmdID= iCmd;
                pOleCmd= (OLECMD*)((LPBYTE)pOleCmd+sizeof(OLECMD));
            }//else try other view group
        }

        // Query status
        if (n && SUCCEEDED(pcmdt->QueryStatus(&CGID_CEShell, n, prgCmds, NULL)))
        {
            bUpdate= TRUE;
        }//else try other view group

        // Enable/Disable if needed
        while (bUpdate && n--)
        {
            pOleCmd= (OLECMD*)((LPBYTE)pOleCmd-sizeof(OLECMD));
            if (pOleCmd->cmdf & OLECMDF_ENABLED)
            {
                UINT uCheck = MF_BYCOMMAND;
                if (pOleCmd->cmdf & OLECMDF_LATCHED)
                {
                    uCheck |= MF_CHECKED;
                }
                else
                {
                    uCheck |= MF_UNCHECKED;
                }
                EnableMenuItem(hMenu, pOleCmd->cmdID, MF_BYCOMMAND | MF_ENABLED);
                ::CheckMenuItem(hMenu, pOleCmd->cmdID, uCheck);
            }
            else if (pOleCmd->cmdf & OLECMDF_SUPPORTED)
            {
                EnableMenuItem(hMenu, pOleCmd->cmdID, MF_BYCOMMAND | MF_GRAYED);
            }
        }

        pcmdt->Release();
    }

    LocalFree(prgCmds);
}

HMENU CMainWnd::CreateSubMenu(LPMENUDATA lpMenuData, int nItems, LONG* plMenuWidth)
{
    HMENU hmenuSub;
    WCHAR szText[MAX_PATH];
    int i;

    hmenuSub= CreatePopupMenu();
    if (hmenuSub)
    {
        // Add SubMenu items
        for (i=0; i<nItems; i++)
        {
            LoadString(g_hInstance, lpMenuData->idszMenuItem, szText, MAX_PATH);
            AppendMenu(hmenuSub, lpMenuData->uFlags, lpMenuData->uIDNewItem, szText);
            // Move pointer to next structure
            lpMenuData= (LPMENUDATA)((LPBYTE)lpMenuData+sizeof(MENUDATA));
        }

        *plMenuWidth= i;
    }

    return hmenuSub;
}


STDMETHODIMP CMainWnd::GetHostInfo(DOCHOSTUIINFO *pInfo)
{
    pInfo->cbSize = sizeof(DOCHOSTUIINFO);
    //pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER|DOCHOSTUIFLAG_FLAT_SCROLLBAR;
    //pInfo->dwFlags |= DOCHOSTUIFLAG_SCROLL_NO;

    return S_OK;
}

STDMETHODIMP CMainWnd::Invoke (
                              DISPID dispidMember,
                              REFIID riid,
                              LCID lcid,
                              WORD wFlags,
                              DISPPARAMS FAR* pdparams,
                              VARIANT FAR* pvarResult,
                              EXCEPINFO FAR* pexcepinfo,
                              UINT FAR* puArgErr
                              )
{
    switch (dispidMember)
    {
#if 0 // this will override the internet settings control panel
    case DISPID_AMBIENT_DLCONTROL:
    if (pvarResult)
    {
        V_VT(pvarResult) = VT_I4;
        // always set these three unless they should be disabled
        pvarResult->lVal = DLCTL_DLIMAGES|DLCTL_VIDEOS|DLCTL_BGSOUNDS;

        // put the browser in download only mode
        // pvarResult->lVal |= DLCTL_DOWNLOADONLY;
    }
    break;
#endif
        case DISPID_STATUSTEXTCHANGE:
            if (_bShowStatusBar)
            {
                if (pdparams && pdparams->rgvarg[0].vt == VT_BSTR)
                {
                    // NULL terminate the string
                    WCHAR szStatusText[128];

                    wcsncpy(szStatusText, pdparams->rgvarg[0].bstrVal, 128);
                    szStatusText[128 - 1] = 0;
                    SendMessage(_hWndStatus, SB_SETTEXT, 0, (LPARAM)szStatusText);
                }
            }
            break;

        case DISPID_SETSECURELOCKICON:
            if(pdparams && pdparams->rgvarg[0].vt == VT_I4)
            {
                HICON hIcon = NULL;
                if (pdparams->rgvarg[0].lVal == SSL_SECURITY_SECURE ||
                    pdparams->rgvarg[0].lVal == SSL_SECURITY_SECURE_40 ||
                    pdparams->rgvarg[0].lVal == SSL_SECURITY_SECURE_56 ||
                    pdparams->rgvarg[0].lVal == SSL_SECURITY_FORTEZZA ||
                    pdparams->rgvarg[0].lVal == SSL_SECURITY_SECURE_128)
                {
                    hIcon = UrlmonUtils_t::GetStatusIcon(5);
                }
                else if (pdparams->rgvarg[0].lVal == SSL_SECURITY_MIXED)
                {
                    hIcon = UrlmonUtils_t::GetStatusIcon(5);
                }

                SendMessage(_hWndStatus, SB_SETICON, 2, (LPARAM)hIcon);
            }
            break;

        case DISPID_PROGRESSCHANGE:
        {
            LONG lProgMax = pdparams->rgvarg[0].lVal;
            LONG lProg = pdparams->rgvarg[1].lVal;
            UINT nPos = (lProg == -1) ? 999 : ((lProg-1)%1000);

            SendMessage(_hWndProgress, PBM_SETPOS, nPos, 0);
        }
        break;

        // notification for file download
        case DISPID_FILEDOWNLOAD:
            break;

        case DISPID_NAVIGATECOMPLETE2:
            if (_bTyped || _bSelected)
            {
                _bTyped = FALSE;
                _bSelected = FALSE;
                ::SetFocus(_hWndAddressCombo);

                if (pdparams && (V_VT(&pdparams->rgvarg[0]) == (VT_VARIANT|VT_BYREF)))
                {
                    if (0 == wcsncmp(pdparams->rgvarg[0].pvarVal->bstrVal, L"::{", 3))
                    {
                        // update GUID/DisplayName table
                        LPWSTR pszPrevDisplayName = NULL;

                        SaveDisplayName(pdparams->rgvarg[0].pvarVal->bstrVal+2, _szTitle, &pszPrevDisplayName);
                        if (pszPrevDisplayName)
                        {
                            int i = SendMessage(_hWndAddressCombo, CB_FINDSTRINGEXACT,
                                    (WPARAM )-1, (LPARAM )pszPrevDisplayName);
                            if (i != CB_ERR)
                            {
                                SendMessage(_hWndAddressCombo, CB_DELETESTRING, i, 0);
                                SendMessage(_hWndAddressCombo, CB_INSERTSTRING, i, (LPARAM )_szTitle);
                            }
                            LocalFree(pszPrevDisplayName);
                        }
                        UpdateUrlList(_szTitle);
                    }
                    else
                    {
                        UpdateUrlList(pdparams->rgvarg[0].pvarVal->bstrVal);
                    }
                }
            }
            else
            {
                HWND hwndIPAO;
                if (_pIPActiveObj && (S_OK == _pIPActiveObj->GetWindow(&hwndIPAO)))
                {
                    ::SetFocus(hwndIPAO);
                }
            }
            SetCurrentLocation();
            UpdateZoneInfo();
            ShowZoneInfo();
            break;

        case DISPID_NAVIGATEERROR:
        {
            _bTyped= FALSE;

            // Display Error
            if (pdparams &&
               pdparams->rgvarg[3].vt == (VT_VARIANT | VT_BYREF) &&
               pdparams->rgvarg[3].pvarVal &&
               pdparams->rgvarg[3].pvarVal->vt == VT_BSTR)
            {
                BOOL bHandled = TRUE;
                HRESULT hr = E_FAIL;
                LPWSTR lpszPath = pdparams->rgvarg[3].pvarVal->bstrVal;

                if (!lpszPath)
                {
                    lpszPath= L"";
                }

                if (pdparams->rgvarg[1].vt == (VT_VARIANT | VT_BYREF) &&
                   pdparams->rgvarg[1].pvarVal &&
                   pdparams->rgvarg[1].pvarVal->vt == VT_I4 &&
                   pdparams->rgvarg[1].pvarVal->lVal)
                {
                    hr = pdparams->rgvarg[1].pvarVal->lVal;
                }

                switch(HRESULT_CODE(hr))
                {
                    case E_INVALIDARG:
                        ASSERT(hr == E_INVALIDARG); // If we ever hit this, look into shdocvw
                        // fall through to E_FAIL

                    case ERROR_FILE_NOT_FOUND:
                    case ERROR_PATH_NOT_FOUND:
                    case E_FAIL:
                    {
                        WCHAR szMsg[MAX_PATH];
                        LPWSTR lpsz;

                        LoadString(g_hInstance, IDS_ERR_GENERIC, szMsg, MAX_PATH);
                        //wcschr(lpszMsg, TEXT('%'));
                        UINT len =  lstrlen(szMsg) + 1;
                        len += lstrlen(lpszPath);

                        lpsz = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * len);
                        if (lpsz)
                        {
                            wsprintf(lpsz, szMsg, lpszPath);
                            MessageBox(_hWnd, lpsz, lpszPath, MB_ICONERROR | MB_OK | MB_SETFOREGROUND);
                            LocalFree(lpsz);
                        }
                    }
                    break;

                    case ERROR_NO_NETWORK:
                    case ERROR_BAD_NETPATH:
                        // These are handled by the view for now
                        break;

                    case E_OUTOFMEMORY:
                        SHShowOutOfMemory(_hWnd, 0);
                        break;

                    default:
                        bHandled = FALSE;
                }

                if (bHandled)
                {
                    // If error was handled, then restore the address bar.
                    // Shdocvw will display its own error page otherwise
                    SetCurrentLocation();
                }

                if (V_VT(&pdparams->rgvarg[0]) == (VT_BOOL |VT_BYREF)
                    && pdparams->rgvarg[0].pboolVal)
                {
                    *(pdparams->rgvarg[0].pboolVal)= bHandled;
                }

            }
            break;
        }

        case DISPID_COMMANDSTATECHANGE:
        {
            DWORD dwState;
            DWORD dwId = -1;
            UINT uEnable;

            switch ((LONG)pdparams->rgvarg[1].lVal)
            {
                case CSC_NAVIGATEFORWARD :
                    dwId = ID_GO_FORWARD;
                    break;
                case CSC_NAVIGATEBACK :
                    dwId = ID_GO_BACK;
                    break;
            }

            if (dwId != -1)
            {
                dwState = SendMessage (_hWndToolBar, TB_GETSTATE, dwId, 0);
                if ((BOOL)pdparams->rgvarg[0].boolVal)
                {
                    dwState |= TBSTATE_ENABLED;
                    uEnable = MF_ENABLED;
                }
                else
                {
                    dwState &= ~(TBSTATE_ENABLED);
                    uEnable = MF_GRAYED;
                }
                SendMessage (_hWndToolBar, TB_SETSTATE, dwId, dwState);
                HMENU hmenu = CommandBar_GetMenu(_hWndMenuBar, 0);
                if (hmenu)
                {
                    ::EnableMenuItem(hmenu, dwId, MF_BYCOMMAND | uEnable);
                }
            }
            break;
        }

        case DISPID_TITLECHANGE:
            if (pdparams && pdparams->rgvarg[0].vt == VT_BSTR)
            {
                LPWSTR pszArgTitle = (LPWSTR) pdparams->rgvarg[0].bstrVal;
                WCHAR szTitle[85];
                int len = wcslen(pszArgTitle);

                int j = len;
                while (j>0 && pszArgTitle[--j] != L'\\');

                j += (pszArgTitle[j] == L'\\') ? 1:0;
                len -= j;

                _tcsncpy(_szTitle, &pszArgTitle[j], MAX_URL-1);
                _tcsncpy(szTitle, &pszArgTitle[j], 80);
                szTitle[80]= 0;

                if (len > 80)
                {
                    _tcscat(szTitle, L"...");
                }
                SetWindowText(_hWnd, szTitle);
            }
            break;

        case DISPID_ONQUIT:
            PostMessage(_hWnd, WM_CLOSE, 0, 0L);
            break;

        case DISPID_DOWNLOADBEGIN:
            _wDLCCounter++;
            if (_wDLCCounter == 1)
            {
                if (_bShowAnimation && _pthrdAnimation)
                {
                    _pthrdAnimation->StartAnimation();
                }
                EnterDownload();
            }
            break;

        case DISPID_DOWNLOADCOMPLETE:
        case DISPID_DOCUMENTCOMPLETE:
            _wDLCCounter--;
            if (_wDLCCounter <= 0)
            {
                ExitDownload();
                if (_bShowAnimation && _pthrdAnimation)
                {
                    _pthrdAnimation->StopAnimation();
                }
                ChangeFontSize(TRUE);
                _wDLCCounter = 0;
                if (_bShowAnimation != _bInWebBrowserMode)
                {
                    _bShowAnimation = _bInWebBrowserMode;
                }
            }
            break;

        case DISPID_NEWWINDOW2:
            return HandleNewWindow2(NULL, NULL, pdparams);

        case DISPID_PROPERTYCHANGE:
        case DISPID_BEFORENAVIGATE2:
            break;

        case DISPID_ADDFAVORITE:
            if (pdparams && pdparams->cArgs == 2 &&
                pdparams->rgvarg[0].vt == VT_BSTR &&
                pdparams->rgvarg[1].vt == VT_BSTR &&
                _Favorites)
            {
                _Favorites->DoAddToFavorites(_hWnd, pdparams->rgvarg[0].bstrVal, pdparams->rgvarg[1].bstrVal);
            }
            break;

        default:
            return DISP_E_MEMBERNOTFOUND;
    }
    return S_OK;
}


STDMETHODIMP CMainWnd::moveTo( LONG x, LONG y)
{
    SetWindowPos(_hWnd, NULL, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
    return S_OK;
}

STDMETHODIMP CMainWnd::moveBy( LONG x, LONG y)
{
    RECT rcWindow;

    GetWindowRect(_hWnd, &rcWindow);

    SetWindowPos(_hWnd, NULL, rcWindow.left + x, rcWindow.top + y, 0, 0,
                               SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
    return S_OK;
}


STDMETHODIMP CMainWnd::resizeTo( LONG x, LONG y)
{
    // We do not want the size to be less then 100 for top level windows in browser
    if (x < 100)
    {
        x = 100;
    }

    if (y < 100)
    {
        y = 100;
    }

    SetWindowPos(_hWnd, NULL, 0, 0, x, y, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
    return S_OK;
}

STDMETHODIMP CMainWnd::resizeBy( LONG x, LONG y)
{
    RECT rcWindow;
    long w, h;

    GetWindowRect(_hWnd, &rcWindow);

    w = rcWindow.right - rcWindow.left + x;
    h = rcWindow.bottom - rcWindow.top + y;

    if (w < 100)
    {
        w = 100;
    }

    if (h < 100)
    {
        h = 100;
    }

    SetWindowPos(_hWnd, NULL, 0, 0, w, h, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
    return S_OK;
}

LRESULT CMainWnd::HandleCommand(WPARAM wParam, LPARAM lParam)
{
    UINT nID = LOWORD(wParam);

        // give  componentized code a shot at the message
    if (!ExtHandleCommand(wParam, lParam))
    {
        return 0;
    }

    switch (nID)
    {
        case IDM_STATUSBAR_TOGGLE:
            ShowStatusBar(!_bShowStatusBar);
            ResizeBrowserWindow();
            break;

        case ID_ADDRESSBAR:
            if (HIWORD(wParam) == CBN_SELENDOK)
            {
                SelChange();
            }
            break;

        case ID_INTERNET_OPTIONS:
        {
            SHELLEXECUTEINFO sei;
            sei.cbSize = sizeof(sei);
            sei.fMask = SEE_MASK_FLAG_NO_UI;
            sei.hwnd = _hWnd;
            sei.lpVerb = NULL;
            sei.lpFile = L"ctlpnl";
            sei.lpParameters = L"inetcpl.cpl";
            sei.lpDirectory = NULL;
            sei.nShow = SW_SHOWNORMAL;

            ShellExecuteEx(&sei);
        }
        break;

        case ID_GO_BACK:
            _pBrowser->GoBack();
            break;

        case ID_GO_FORWARD:
            _pBrowser->GoForward();
            break;

        case ID_GO_HOME:
            _pBrowser->GoHome();
            break;

        case ID_GO_SEARCH:
            _pBrowser->GoSearch();
            break;

        case ID_VIEW_REFRESH:
            _pBrowser->Refresh();
            break;

        case ID_VIEW_STOP:
            _pBrowser->Stop();
            break;

        case ID_ZOOMUP:
            _wZoom++;
            if (_wZoom > 4)
                _wZoom = 4;
            ChangeFontSize(FALSE);
            break;

        case ID_ZOOMDOWN:
            _wZoom--;
            if (_wZoom < 0)
            {
                _wZoom = 0;
            }
            ChangeFontSize(FALSE);
            break;

        case ID_CLOSE:
            _pBrowser->ExecWB(OLECMDID_CLOSE, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
            break;

        case ID_FOCUS_URL:
            if (_hWndAddressEdit)
            {
                SetFocus(_hWndAddressEdit);
            }
            break;

        case ID_POPUP_URL:
            if (_hWndAddressCombo)
            {
                SendMessage(_hWndAddressCombo, CB_SHOWDROPDOWN, (WPARAM)TRUE, 0);
                ::SetFocus(_hWndAddressCombo);
            }
            break;

        case ID_FILE_OPEN:
        case ID_OPEN:
            {
                WCHAR *szURL = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * MAX_URL);
                if (szURL)
                {
                    int nRet = 0;
                    BSTR bstrUrl = NULL;
                    _pBrowser->get_LocationURL(&bstrUrl);
                    if ( StringCchCopy(szURL, MAX_URL, (LPCTSTR)bstrUrl) != S_OK )
                    {
                        LocalFree(szURL);
                        SysFreeString( bstrUrl );
                        break;
                    }
                    SysFreeString(bstrUrl);

                    nRet =  DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_OPEN_DIALOG), _hWnd, OpenURLDlgProc, (long)szURL);
                    if (nRet == 1)
                    {
#pragma prefast(suppress:307, "This is an ok cast")
                        _pBrowser->Navigate(szURL, NULL, NULL, NULL, NULL);
                    }
                    else if (nRet == 2)
                    {
                        HandleNewWindow2(szURL,NULL, NULL);
                    }
                    LocalFree(szURL);
                }
            }
            break;

        case ID_FILE_FIND:
            FindString();
            break;

        case ID_FILE_SAVEAS:
        {
            VARIANT vTitle;
            BSTR bstrTitle = SysAllocString(_szTitle);
            if (bstrTitle)
            {
                WCHAR *pch = bstrTitle;
                // Strip dots. Perhaps find a more locale friendly way of doing this?
                while (*pch)
                {
                    if (*pch == L'.')
                    {
                        *pch = L'_';
                    }
                    pch++;
                }
                vTitle.vt = VT_BSTR;
                vTitle.bstrVal = bstrTitle;
                _pBrowser->ExecWB(OLECMDID_SAVEAS, OLECMDEXECOPT_DODEFAULT, &vTitle, NULL);
                SysFreeString(bstrTitle);
            }
            break;
        }

        case ID_ADDRESSBAR_TOGGLE:
            ShowAddressBar(!_bShowAddrBar);
            break;

        case ID_TOOLBARS_TOGGLE:
            if (_hWndCmdband)
            {
                RECT rc;

                _bShowCmdBar = !_bShowCmdBar;
                CommandBands_Show(_hWndCmdband, _bShowCmdBar);
                GetClientRect(_hWnd, &rc);
                if (_bShowCmdBar)
                {
                    rc.top += CommandBands_Height(_hWndCmdband);
                }

                if (_pthrdAnimation)
                {
                    _pthrdAnimation->PositionWindow(&rc);
                }

                ResizeBrowserWindow();
            }
            break;

        case ID_EDIT_CUT:
            if (GetFocus()== _hWndAddressEdit)
            {
                SendMessage(_hWndAddressEdit, WM_CUT, WPARAM(0), LPARAM(0));
            }
            else
            {
                _pBrowser->ExecWB(OLECMDID_CUT, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
            }
            break;

        case ID_EDIT_COPY:
            if (GetFocus()== _hWndAddressEdit)
            {
                SendMessage(_hWndAddressEdit, WM_COPY, WPARAM(0), LPARAM(0));
            }
            else
            {
                _pBrowser->ExecWB(OLECMDID_COPY, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
            }
            break;

        case ID_EDIT_PASTE:
            if (GetFocus() == _hWndAddressEdit)
            {
                SendMessage(_hWndAddressEdit, WM_PASTE, WPARAM(0), LPARAM(0));
            }
            else
            {
                _pBrowser->ExecWB(OLECMDID_PASTE, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
            }
            break;

        case ID_FAV_ORG:
            if (_Favorites)
            {
                _Favorites->DoOrganizeFavorites(_hWnd);
            }
            break;

        case ID_FAV_ADD:
        {
            BSTR bstrURL = NULL;
            if (_Favorites && S_OK ==_pBrowser->get_LocationURL(&bstrURL))
            {
                _Favorites->DoAddToFavorites(_hWnd, bstrURL, _szTitle);
                SysFreeString(bstrURL);
            }
            break;
        }

        case ID_VIEW_FONTSLARGEST:
        case ID_VIEW_FONTSLARGER:
        case ID_VIEW_FONTSMEDIUM:
        case ID_VIEW_FONTSSMALLER:
        case ID_VIEW_FONTSSMALLEST:
            _wZoom = LOWORD(wParam) - ID_VIEW_FONTSSMALLEST;
            ChangeFontSize(FALSE);
            break;

        case ID_FULLSCREEN:
            return ToggleFullScreen();

        case ID_HELP_TOPICS:
            if (_bShowHelp)
            {
                CreateProcess(L"\\windows\\peghelp.exe",
                                _bInWebBrowserMode ?
                                        L"wince.htm" :
                                        L"wince.htm#Windows_Explorer_Help",
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0);
            }

        default:
            // handle favorite menu items
            if (nID >= IDM_FAVORITES_FIRST__ && nID <= IDM_FAVORITES_LAST__)
            {
                if (_Favorites)
                {
                     HMENU hmenu = CommandBar_GetMenu(_hWndMenuBar, 0);
                    _Favorites->ExecuteCommand(hmenu, nID, _pBrowser, _hWnd);
                }
                return 0;
            }
            // if this is from the encoding popup
            else if (IDM_MIMECSET__FIRST__ <= nID && IDM_MIMECSET__LAST__ >= nID)
            {
                if (_pBrowser)
                {
                    IOleCommandTarget* pcmdt;
                    if (SUCCEEDED(_pBrowser->QueryInterface(IID_IOleCommandTarget, (LPVOID*) &pcmdt)))
                    {
                        pcmdt->Exec(&CGID_MSHTML, nID, 0, NULL, NULL);
                        pcmdt->Release();
                    }
                }
                return 0;
            }
            else if (IDM_CESHELL__FIRST__ <= nID && IDM_CESHELL__LAST__ >= nID)
            {
                if (_pBrowser)
                {
                    IOleCommandTarget* pcmdt;
                    if (SUCCEEDED(_pBrowser->QueryInterface(IID_IOleCommandTarget, (LPVOID*) &pcmdt)))
                    {
                        pcmdt->Exec(&CGID_CEShell, nID, 0, NULL, NULL);
                        pcmdt->Release();
                    }
                }
                return 0;
            }
            return 1;
    }
    return 0;
}

LRESULT CMainWnd::HandleNotify(WPARAM wParam, LPARAM lParam)
{
    if (wParam== ID_COMMANDBAND)
    {
        NMREBAR * pnm = (NMREBAR*)lParam;
        switch(pnm->hdr.code)
        {
            case RBN_LAYOUTCHANGED:
                break;

            case RBN_HEIGHTCHANGE:
            {
                RECT rcMain, rcBrowse, rcStatus;

                int iHeight = SendMessage(pnm->hdr.hwndFrom, RB_GETBARHEIGHT, 0, 0);

                if (_bShowStatusBar && _hWndStatus)
                {
                    GetClientRect(_hWndStatus, &rcStatus);
                }
                else
                {
                    rcStatus.bottom = rcStatus.top =0;
                }
                if (_hWndBrowser)
                {
                    GetClientRect(_hWnd, &rcMain);
                    GetClientRect(_hWndBrowser, &rcBrowse);
                    int iBrowseHeight = (rcMain.bottom - rcMain.top) - iHeight -(rcStatus.bottom - rcStatus.top);

                    if (iBrowseHeight != (rcBrowse.bottom - rcBrowse.top))
                    {
                        SetWindowPos(_hWndBrowser,  NULL, 0, iHeight, rcMain.right-rcMain.left, iBrowseHeight, SWP_NOZORDER);
                    }
                }
            }
            break;
        }
    }
    else
    {
        NMTOOLBAR* pnm = (NMTOOLBAR*)lParam;

        if ((pnm->hdr.hwndFrom == _hWndToolBar) && (pnm->hdr.code == TBN_DROPDOWN))
        {
            if (IDM_CESHELL__FIRST__ <= pnm->iItem && IDM_CESHELL__LAST__ >= pnm->iItem)
            {
                IOleCommandTarget* pcmdt;
                if (_pBrowser && SUCCEEDED(_pBrowser->QueryInterface(IID_IOleCommandTarget, (LPVOID*) &pcmdt)))
                {
                    pcmdt->Exec(&CGID_CEShell, pnm->iItem, 0, NULL, NULL);
                    pcmdt->Release();
                }
            }
        }
    }

    return 0;
}

VOID CMainWnd::HandleSettingChange(WPARAM wParam, LPARAM lParam)
{
    if (wParam == SPI_SETWORKAREA)
    {
        if (_bFullScreen)
        {
            RECT rcWorkArea;
            // no need to change the window style here, just adjust for the new work-area.
            SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0);
            SetWindowPos(_hWnd,
                            NULL,
                            rcWorkArea.left,
                            rcWorkArea.top,
                            rcWorkArea.right-rcWorkArea.left,
                            rcWorkArea.bottom-rcWorkArea.top,
                            SWP_NOZORDER);
        }
    }

    IOleInPlaceObject * pOleInPlaceObject;
    if (SUCCEEDED(_pObject->QueryInterface(IID_IOleInPlaceObject,
                                           (PVOID*) &pOleInPlaceObject)))
    {
        HWND hwnd = NULL;
        if (SUCCEEDED(pOleInPlaceObject->GetWindow(&hwnd)))
        {
            ::SendMessage(hwnd, WM_SETTINGCHANGE, wParam, lParam);
        }
        pOleInPlaceObject->Release();
    }
}

LRESULT  CMainWnd::ToggleFullScreen()
{
    DWORD dwStyle = GetWindowLong(_hWnd, GWL_STYLE);

    if (_bFullScreen)
    {
        dwStyle |= (WS_OVERLAPPED | WS_SYSMENU | WS_THICKFRAME);
        SetWindowLong(_hWnd, GWL_STYLE, dwStyle);
        SetWindowPos(_hWnd,
                        NULL,
                        _rcWnd.left,
                        _rcWnd.top,
                        _rcWnd.right-_rcWnd.left,
                        _rcWnd.bottom-_rcWnd.top,
                        SWP_NOZORDER);
    }
    else
    {
        RECT rcWorkArea;

        SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0);
        dwStyle &= ~(WS_OVERLAPPED | WS_SYSMENU | WS_THICKFRAME);

        SetWindowLong(_hWnd, GWL_STYLE, dwStyle);
        SetWindowPos(_hWnd,
                        NULL,
                        rcWorkArea.left,
                        rcWorkArea.top,
                        rcWorkArea.right-rcWorkArea.left,
                        rcWorkArea.bottom-rcWorkArea.top,
                        SWP_NOZORDER);
    }
    _bFullScreen = !_bFullScreen;

    if (_hWndCmdband)
    {
        AdjustCommandBar();
    }
    else
    {
        CreateCommandBar();
    }

    return 0;
}


LRESULT  CMainWnd::BrowseWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CMainWnd * pMainWnd = this;

    if (pMainWnd)
    {
        switch (message)
        {
            case WM_CLOSE:
            case WM_DESTROY:
                SetWindowLong(hwnd, GWL_USERDATA, (DWORD)0);
                if (pMainWnd)
                {
                    pMainWnd->Close();
                }
                ::PostQuitMessage(0);
                break;

            case WM_ACTIVATE:
            if (pMainWnd->_pIPActiveObj)
            {
                pMainWnd->_pIPActiveObj->OnFrameWindowActivate(LOWORD(wParam) != WA_INACTIVE);
            }
            return 0;

            case WM_SIZE:
            {
                RECT rc,rc2;
                GetClientRect(pMainWnd->_hWnd, &rc);

                pMainWnd->ResizeBrowserWindow();
                if (pMainWnd->_bShowCmdBar)
                {
                    GetClientRect(pMainWnd->_hWndCmdband, &rc2);
                    if ((rc2.right - rc2.left) != (rc.right - rc.left))
                    {
                        rc2.left = rc.left;
                        rc2.right = rc.right;
                        SetWindowPos(pMainWnd->_hWndCmdband, NULL, rc2.left, rc2.top, rc2.right-rc2.left, rc2.bottom-rc2.top, SWP_NOZORDER);
                    }
                }

                if (_pthrdAnimation)
                {
                    rc.top += CommandBands_Height(_hWndCmdband);
                     _pthrdAnimation->PositionWindow(&rc);
                }

                GetClientRect(pMainWnd->_hWndStatus, &rc2);
                rc.top = rc.bottom - (rc2.bottom - rc2.top);

                int naParts[5];
                naParts[4] = rc.right;
                naParts[3] = MAX(0, rc.right - MESSAGEPANEWIDTH);
                naParts[2] = MAX(0, rc.right - MESSAGEPANEWIDTH - ICONPANEWIDTH);
                naParts[1] = MAX(0, rc.right - MESSAGEPANEWIDTH - ICONPANEWIDTH - ICONPANEWIDTH);
                naParts[0] = MAX(0, rc.right - MESSAGEPANEWIDTH - ICONPANEWIDTH - ICONPANEWIDTH - PROGRESSPANEWIDTH);

                SendMessage(pMainWnd->_hWndStatus, SB_SETPARTS, (WPARAM )5, (LPARAM)naParts);
                SetWindowPos(pMainWnd->_hWndStatus, NULL, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, SWP_NOZORDER);

                SendMessage(_hWndStatus, SB_GETRECT, 1, (LPARAM )&rc);
                InflateRect(&rc, -1,-1);

                SetWindowPos(pMainWnd->_hWndProgress, NULL, naParts[0], rc.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
            }
            // FALL THROUGH
            case  WM_MOVE:
                if (!pMainWnd->_bFullScreen)
                {
                    GetWindowRect(hwnd, &pMainWnd->_rcWnd);
                }
                break;

            case WM_COMMAND:
                return pMainWnd->HandleCommand(wParam, lParam);

            case WM_NOTIFY:
                return pMainWnd->HandleNotify(wParam, lParam);

            case WM_GETICON:
                {
                    // the shell view will have set it's own icon, but the web browser needs to do it here
                    // if the active view is MSHTML, the code below will succeed, if not it will fail
                    HRESULT hr = E_FAIL;
                    IOleCommandTarget* pcmdt;

                    if (_pBrowser && SUCCEEDED(_pBrowser->QueryInterface(IID_IOleCommandTarget, (LPVOID*) & pcmdt)))
                    {
                        VARIANTARG v = {0};
                        // S_OK implies this is a web page, E_FAIL if it is not. Ignore v for now.
                        hr = pcmdt->Exec(&CGID_ShellDocView, SHDVID_GETSYSIMAGEINDEX, 0, NULL, &v);
                        pcmdt->Release();
                        if (SUCCEEDED(hr))
                        {
                            // return the hIcon for a webpage
                            return (LONG)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_HTML),IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
                        }
                    }
                }
                break;

            case WM_INITMENUPOPUP:
                pMainWnd->HandleMenuPopup(wParam, lParam);
                break;

            case WM_MEASUREITEM:
                if (pMainWnd->_Favorites)
                {
                    return pMainWnd->_Favorites->HandleMeasureItem((MEASUREITEMSTRUCT *)lParam);
                }
                break;

            case WM_DRAWITEM:
                if (pMainWnd->_Favorites)
                {
                    return pMainWnd->_Favorites->HandleDrawItem((DRAWITEMSTRUCT *)lParam);
                }
                break;

            case WM_MENUCHAR:
                if (pMainWnd->_Favorites)
                {
                    return pMainWnd->_Favorites->HandleMenuChar((HMENU) lParam, (WCHAR)LOWORD(wParam));
                }
                break;

            case WM_EXITMENULOOP:
                if (pMainWnd->_bInMenuLoop)
                {
                    pMainWnd->_bInMenuLoop = FALSE;
                }
                break;

            case WM_SETTINGCHANGE:
                pMainWnd->HandleSettingChange(wParam, lParam);
                break;

            case WM_SYSCOLORCHANGE:
            {
                // forward message to the controls and the View
                SendMessage(_hWndCmdband, message, wParam, lParam);

                HWND hwndIPAO;
                if (_pIPActiveObj && (S_OK == _pIPActiveObj->GetWindow(&hwndIPAO)))
                {
                    SendMessage(hwndIPAO, message, wParam, lParam);
                }
                break;
            }

            case WM_HELP:
            {
                BOOL bHandled = FALSE;
                HWND hwndIPAO;

                // Send to the view object first
                if (_pIPActiveObj && (S_OK == _pIPActiveObj->GetWindow(&hwndIPAO)))
                {
                    bHandled = SendMessage(hwndIPAO, message, wParam, lParam);
                }

                if (!bHandled)
                {
                    pMainWnd->HandleCommand(ID_HELP_TOPICS, 0);
                }
                return TRUE;
            }

            default:
                break;
        }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

BOOL CMainWnd::PreTranslateMessage(LPMSG pMsg)
{
    BOOL bHandled = FALSE;

    if (pMsg->hwnd == _hWndAddressEdit)
    {
        if ((WM_KEYDOWN == pMsg->message) && (VK_TAB == pMsg->wParam))
        {
            HWND hwndIPAO;
            if (_pIPActiveObj && (S_OK == _pIPActiveObj->GetWindow(&hwndIPAO)))
            {
                ::SetFocus(hwndIPAO);
                goto DoAcceleratorTranslations;
            }
        }

        return FALSE;
    }

DoAcceleratorTranslations:

    // Send to the view object first
    if (_pIPActiveObj)
    {
        HRESULT hr = _pIPActiveObj->TranslateAccelerator(pMsg);
        bHandled = (SUCCEEDED(hr) && (S_FALSE != hr));
    }

    // Send to the frame if the view didn't handle it
    if (!bHandled)
    {
        bHandled = ::TranslateAccelerator(_hWnd, _hAccelTbl, pMsg);
    }

    return bHandled;
}

VOID CMainWnd::ChangeFontSize(BOOL fInit)
{
    VARIANT vaSize;
    vaSize.vt = VT_I4;

    if (fInit)
    {
        // Setting initial value
        vaSize.vt = 0;
        HRESULT hr = _pBrowser->ExecWB(OLECMDID_ZOOM, OLECMDEXECOPT_DONTPROMPTUSER,
                                           NULL, &vaSize);
        _wZoom = vaSize.lVal;
    }
    else
    {
        vaSize.lVal = _wZoom;
        HRESULT hr = _pBrowser->ExecWB(OLECMDID_ZOOM, OLECMDEXECOPT_DONTPROMPTUSER,
                                           &vaSize, NULL);
    }

    HMENU hMenu = CommandBar_GetMenu(_hWndMenuBar, 0);
    BOOL bRet = CheckMenuRadioItem(hMenu, ID_VIEW_FONTSSMALLEST ,ID_VIEW_FONTSLARGEST, _wZoom+ID_VIEW_FONTSSMALLEST, MF_BYCOMMAND);
    if (_bShowCmdBar)
    {
        CommandBar_DrawMenuBar(_hWndMenuBar, 0);
    }
}


#if 0 // componentized to extui.cpp
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
#endif

BOOL CMainWnd::LoadToolbarSettings(LPCWSTR pwszHost)
{
    BOOL bRet = FALSE;
    HKEY hKey = NULL;
    LONG lRet;
    int i, index;

    PREFAST_ASSERT(pwszHost);
    index = (wcscmp(pwszHost, L"IE") == 0) ? 0:1;

    // Initialize structure
    for (i = 0; i < CMDBANDS; i++)
    {
        _cbRestoreInfo[index][i].cbSize = sizeof(COMMANDBANDSRESTOREINFO);
        _cbRestoreInfo[index][i].wID = ID_BAND_INVALID;
    }

    lRet = RegOpenKeyEx(HKEY_CURRENT_USER, CmdBands_RegKey, 0, 0, &hKey);
    if (lRet == ERROR_SUCCESS)
    {
        bRet = TRUE;

        for (i = 0; i < CMDBANDS; i++)
        {
            WCHAR wszBand[20];
            HKEY hkeyBand;
            DWORD dwLen, dwMax;

            StringCchPrintfEx(wszBand,
                            ARRAYSIZE(wszBand),
                            NULL,
                            NULL,
                            STRSAFE_IGNORE_NULLS,
                            L"%sBand%d",
                            pwszHost,
                            i);

            lRet = RegOpenKeyEx(hKey, wszBand, 0, 0, &hkeyBand);
            if (lRet == ERROR_SUCCESS)
            {
                dwLen = sizeof(DWORD);
                RegQueryValueEx(hkeyBand, BandID_RegValue, NULL, NULL, (LPBYTE)&_cbRestoreInfo[index][i].wID, &dwLen);
                dwLen = sizeof(DWORD);
                RegQueryValueEx(hkeyBand, BandStyle_RegValue, NULL, NULL, (LPBYTE)&_cbRestoreInfo[index][i].fStyle, &dwLen);
                dwLen = sizeof(DWORD);
                RegQueryValueEx(hkeyBand, BandWidth_RegValue, NULL, NULL, (LPBYTE)&_cbRestoreInfo[index][i].cxRestored, &dwLen);
                dwLen = sizeof(DWORD);
                RegQueryValueEx(hkeyBand, BandMax_RegValue, NULL, NULL, (LPBYTE)&dwMax, &dwLen);

                _cbRestoreInfo[index][i].fMaximized = (BOOL)(dwMax);
                RegCloseKey(hkeyBand);
            }
            else
            {
                bRet = FALSE;
            }
        }
        RegCloseKey(hKey);
    }

    return bRet;
}

BOOL CMainWnd::SaveToolbarSettings(LPCWSTR pwszHost, BOOL fUseRegistry)
{
    BOOL bRet = FALSE;
    HKEY hKey = NULL;
    DWORD dwDisp;
    LONG lRet;
    int i, index;

    PREFAST_ASSERT(pwszHost);
    index = (wcscmp(pwszHost, L"IE") == 0) ? 0:1;

    if (fUseRegistry)
    {
        lRet = RegCreateKeyEx(HKEY_CURRENT_USER, CmdBands_RegKey, 0, NULL, 0, 0, NULL, &hKey, &dwDisp);
        if (lRet == ERROR_SUCCESS)
        {
            bRet = TRUE;
            for (i = 0; i < CMDBANDS; i++)
            {
                HKEY hkeyBand = NULL;
                WCHAR wszBand[20];

                StringCchPrintfEx(wszBand,
                                ARRAYSIZE(wszBand),
                                NULL,
                                NULL,
                                STRSAFE_IGNORE_NULLS,
                                L"%sBand%d",
                                pwszHost,
                                i);

                lRet = RegCreateKeyEx(hKey, wszBand, 0, NULL, 0, 0, NULL, &hkeyBand, &dwDisp);
                if (lRet == ERROR_SUCCESS)
                {
                    DWORD dwTemp;
                    RETAILMSG(0, (L"Explorer: saving %s wID=%d Width=%d Style=%d\r\n", wszBand, _cbRestoreInfo[index][i].wID,
                            _cbRestoreInfo[index][i].cxRestored, (_cbRestoreInfo[index][i].fStyle & RBBS_BREAK)));

                    RegSetValueEx(hkeyBand, BandID_RegValue, 0, REG_DWORD, (LPBYTE)&_cbRestoreInfo[index][i].wID, sizeof(DWORD));
                    RegSetValueEx(hkeyBand, BandStyle_RegValue, 0, REG_DWORD, (LPBYTE)&_cbRestoreInfo[index][i].fStyle , sizeof(DWORD));
                    RegSetValueEx(hkeyBand, BandWidth_RegValue, 0, REG_DWORD, (LPBYTE)&_cbRestoreInfo[index][i].cxRestored, sizeof(DWORD));
                    dwTemp = (_cbRestoreInfo[index][i].fMaximized) ? 1: 0;
                    RegSetValueEx(hkeyBand, BandMax_RegValue, 0, REG_DWORD, (LPBYTE)&dwTemp, sizeof(DWORD));

                    RegCloseKey(hkeyBand);
                }
                else
                {
                    bRet = FALSE;
                }
            }
            RegCloseKey(hKey);
        }

    }
    else if (_hWndCmdband)
    {
        // Store settings on class member
        bRet = TRUE;

        int iBand = 0;
        for (i = 0; i < (CMDBANDS+1) && iBand < CMDBANDS; i++)
        {
            if (!(CommandBands_GetRestoreInformation(_hWndCmdband, i, &_cbRestoreInfo[index][iBand])))
            {
                _cbRestoreInfo[index][iBand].wID = ID_BAND_INVALID;
                _cbRestoreInfo[index][iBand].fStyle = 0;
                _cbRestoreInfo[index][iBand].cxRestored = 0;
                _cbRestoreInfo[index][iBand].fMaximized = FALSE;
            }

            // adornments band might be present, don't save it
            if (0xFFFFFFFF != (UINT)_cbRestoreInfo[index][iBand].wID)
            {
                iBand++;
            }
        }
    }

    return bRet;
}


VOID CMainWnd::LoadTypedUrls()
{
    WCHAR pszValName[10];
    WCHAR *pszValData = NULL;
    DWORD dwType = 0;
    DWORD cbValName,cbData;
    DWORD dwIndex = 0;
    HKEY hKey = NULL;
    LONG lRes = 0;

    pszValData = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * MAX_URL);
    if (!pszValData)
    {
        goto Exit;
    }

    lRes = RegOpenKeyEx(HKEY_CURRENT_USER, TypedUrls_RegKey, 0, 0, &hKey);
    if (hKey && lRes==ERROR_SUCCESS)
    {
        cbValName = 10;
        cbData = sizeof(WCHAR) * MAX_URL;

        while (ERROR_SUCCESS == RegEnumValue(hKey, dwIndex, pszValName, &cbValName, NULL, &dwType, (BYTE *)pszValData, &cbData))
        {
            dwIndex++;
            SendMessage(_hWndAddressCombo, CB_INSERTSTRING, 0, (LPARAM )pszValData);
            cbValName = 10;
            cbData = sizeof(WCHAR) * MAX_URL;
        }
        RegCloseKey(hKey);
    }
    LocalFree(pszValData);

Exit:
    return;
}

VOID CMainWnd::SaveTypedUrls()
{
    HKEY hKey = NULL;
    WCHAR *szString = NULL;
    int cbString = 0;
    DWORD dwDisp;

    LONG lRes = RegCreateKeyEx(HKEY_CURRENT_USER, TypedUrls_RegKey, 0, NULL, 0, 0, 0, &hKey, &dwDisp);
    if (hKey && lRes==ERROR_SUCCESS)
    {
        int nCount = SendMessage(_hWndAddressCombo, CB_GETCOUNT, 0, 0L);
        int nLen;
        WCHAR szValueName[7];
        for (int i=0; i<nCount; i++)
        {
            nLen = SendMessage(_hWndAddressCombo, CB_GETLBTEXTLEN, i, 0L);
            if (nLen==CB_ERR)
            {
                break;
            }

            if (nLen > cbString)
            {
                if (szString)
                {
                    LocalFree(szString);
                }
                szString = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * (nLen+1));
                cbString = nLen;
            }
            // pulled out of the if above, prefix isn't sure if the string is always valid here
            // we'll catch it this way.
            if (!szString)
            {
                break;
            }

            SendMessage(_hWndAddressCombo, CB_GETLBTEXT, i, (LPARAM )szString);
            StringCbPrintfW(szValueName, sizeof(szValueName), L"url%d", i+1);
            lRes = RegSetValueEx(hKey, szValueName, 0, REG_SZ, (BYTE *)szString, (_tcslen(szString)+1)*sizeof(WCHAR));
        }

        RegCloseKey(hKey);
    }

    if (szString)
    {
        LocalFree(szString);
    }
    return;
}

VOID CMainWnd::UpdateUrlList(LPCTSTR lpszUrl)
{

    int i = SendMessage(_hWndAddressCombo, CB_FINDSTRINGEXACT,
                        (WPARAM )-1, (LPARAM )lpszUrl);
    if (i == CB_ERR)
    {
        SendMessage(_hWndAddressCombo, CB_INSERTSTRING, 0, (LPARAM )lpszUrl);
        SendMessage(_hWndAddressCombo, CB_SETCURSEL,
                    (WPARAM)0, (LPARAM)0);
    }
    else
    {
        SendMessage(_hWndAddressCombo, CB_SETCURSEL,
                    (WPARAM)i, (LPARAM)0);
    }

    if ( (SendMessage(_hWndAddressCombo, CB_GETCOUNT, 0, 0L)) > MAXURLS)
    {
        SendMessage(_hWndAddressCombo, CB_DELETESTRING, MAXURLS-1, 0L);
    }
}

VOID CMainWnd::SelChange()
{
    WCHAR *szUrl;
    int nRet, nRet1 = SendMessage(_hWndAddressCombo, CB_GETCURSEL, 0, 0L);
    if (nRet1==CB_ERR)
    {
        return ;
    }

    nRet = SendMessage(_hWndAddressCombo, CB_GETLBTEXTLEN, nRet1, 0L);
    if (nRet==CB_ERR)
    {
        return;
    }

    szUrl = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * (nRet+1));
    if (szUrl)
    {
        BOOL bNavigate = TRUE;
        SendMessage(_hWndAddressCombo, CB_GETLBTEXT, nRet1, (LPARAM )szUrl);

        //check if we need to translate to parsing name
        WCHAR szParsingName[MAX_PATH]= L"::";
        UINT cchBuf = MAX_PATH-2;

        if (GetParsingName(szUrl, szParsingName+2, &cchBuf))
        {
#pragma prefast(suppress:307, "This is an ok cast")
            _pBrowser->Navigate(szParsingName, NULL, NULL, NULL, NULL);
        }
        else
        {
            if (!g_bBrowseInPlace && !PathCanBrowseInPlace(szUrl))
            {
                SHELLEXECUTEINFO sei = {0};
                sei.cbSize = sizeof(sei);
                sei.fMask = 0;
                sei.lpVerb = NULL;
                sei.lpFile = TEXT("explorer.exe");
                sei.lpParameters = szUrl;
                sei.lpDirectory = NULL;
                sei.nShow = SW_SHOWNORMAL;

                if (ShellExecuteEx(&sei))
                {
                    bNavigate = FALSE;
                }
            }

            if (bNavigate)
            {
#pragma prefast(suppress:307, "This is an ok cast")
                _pBrowser->Navigate(szUrl, NULL, NULL, NULL, NULL);
            }
            else
            {
                // ShellExecuteEx succeded above.
                // Restore current location for this browser
                SetCurrentLocation();
            }
        }

        _bSelected = !!bNavigate;
        LocalFree(szUrl);
    }
}

VOID CMainWnd::SetCurrentLocation()
{
    BSTR bstrUrl = NULL;

    if (!_pBrowser)
    {
        return;
    }

    _pBrowser->get_LocationName(&bstrUrl);

    if (bstrUrl)
    {
        ExplorerList_t::UpdateExplorerWndInfo(_hWnd, (LPWSTR)bstrUrl);

        if (_hWndAddressCombo)
        {
            SendMessage(_hWndAddressCombo, WM_SETTEXT, 0, (LPARAM)bstrUrl);
        }

        SysFreeString(bstrUrl);
    }
}

LRESULT CALLBACK CMainWnd::AddressEditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CMainWnd *pMainWnd = (CMainWnd*)GetWindowLong(hwnd, GWL_USERDATA);
    WCHAR szParsingName[MAX_PATH] = _T("::");
    UINT cchBuf = MAX_PATH-2;
    BOOL bNavigate = TRUE;

    if (!pMainWnd)
    {
        return 0;
    }

    if ((message == WM_CHAR) && (wParam == VK_RETURN))
    {
        int cchString = GetWindowTextLength(hwnd);
        LPWSTR szString = NULL;
        LPWSTR pszString = NULL;

        if (0 == cchString)
        {
            goto Leave;
        }

        szString = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * (cchString+1));
        if (!szString)
        {
            goto Leave;
        }

        pszString = szString;

        GetWindowText(hwnd, szString, cchString+1);

        // Remove preceding spaces.
        while (_T(' ') == *pszString)
           pszString++;

        //check if we need to translate to parsing name
        if (GetParsingName(pszString, szParsingName+2, &cchBuf))
        {
#pragma prefast(suppress:307, "This is an ok cast")
            pMainWnd->_pBrowser->Navigate(szParsingName, NULL, NULL, NULL, NULL);
        }
        else
        {
            if (!g_bBrowseInPlace && !PathCanBrowseInPlace(pszString))
            {
                SHELLEXECUTEINFO sei = {0};
                sei.cbSize = sizeof(sei);
                sei.fMask = 0;
                sei.lpVerb = NULL;
                sei.lpFile = TEXT("explorer.exe");
                sei.lpParameters = pszString;
                sei.lpDirectory = NULL;
                sei.nShow = SW_SHOWNORMAL;

                if (ShellExecuteEx(&sei))
                {
                    bNavigate = FALSE;
                }
            }

            if (bNavigate)
            {
#pragma prefast(suppress:307, "This is an ok cast")
                pMainWnd->_pBrowser->Navigate(pszString, NULL, NULL, NULL, NULL);
            }
            else
            {
                // ShellExecuteEx succeded above.
                // Restore current location for this browser
                pMainWnd->SetCurrentLocation();
            }
        }

        pMainWnd->_bTyped = !!bNavigate;

        LocalFree(szString);
Leave:
        return 0;
    }
    else
    {
        return CallWindowProc(pMainWnd->_lpfnAddressEditProc, hwnd, message, wParam, lParam);
    }
}

extern "C" BOOL APIENTRY OpenURLDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    WCHAR *szURL = (WCHAR *)GetWindowLong(hDlg,DWL_USER);
    int nRet = 4;

    switch(message)
    {
        case WM_INITDIALOG:
        {
            // Initalize the SipPref control if necessary
            WNDCLASS wc;
            if (GetClassInfo(g_hInstance, WC_SIPPREF, &wc))
            {
                CreateWindow(WC_SIPPREF, NULL, WS_CHILD, -10, -10,
                                 5, 5, hDlg, NULL, g_hInstance, NULL);
            }

            AygInitDialog( hDlg, SHIDIF_SIZEDLG );

            if (!lParam)
            {
                EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_BROWSE), FALSE);
            }
            else
            {
                szURL = (WCHAR *)lParam;
                SetWindowLong(hDlg, DWL_USER, (LONG)szURL);
                SendMessage(GetDlgItem(hDlg, IDC_URL_EDIT), EM_LIMITTEXT, MAX_URL-1, 0);
                if (szURL[0])
                {
                    SetDlgItemText(hDlg, IDC_URL_EDIT, szURL);
                }
            }
            return TRUE;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    GetDlgItemText(hDlg, IDC_URL_EDIT, szURL, MAX_URL-1);
                    BOOL bNewWnd = SendMessage(GetDlgItem(hDlg, IDC_NEWWINDOW), BM_GETCHECK, 0L, 0L);
                    nRet = (bNewWnd) ? 2 : 1;
                }

               case IDCANCEL:
                    EndDialog(hDlg, nRet);
                    return TRUE;

                case IDC_BROWSE:
                    if (HandleBrowse(hDlg, szURL))
                    {
                        SetDlgItemText(hDlg, IDC_URL_EDIT, szURL);
                    }

                default:
                    return (TRUE);
            }
            break;

        case WM_DESTROY:
            SetWindowLong(hDlg, DWL_USER, 0);
            break;
    }

    return (FALSE);
}


BOOL HandleBrowse(HWND hwndOwner, WCHAR *szURL)
{
    BOOL bRet = FALSE;
    OPENFILENAME ofn;
    WCHAR wchFilter[MAX_PATH];
    WCHAR wchFile[MAX_PATH+8];
    int cbLen;

     // Initialize ofn struct
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndOwner;
    ofn.Flags = OFN_FILEMUSTEXIST |
                    OFN_PATHMUSTEXIST   |
                    OFN_OVERWRITEPROMPT |
                    OFN_HIDEREADONLY;

    cbLen = LoadString(g_hInstance, IDS_BROWSEFILTER,
                       wchFilter, MAX_PATH-2);

    if (cbLen>0)
    {
        for (; cbLen >= 0; cbLen--)
        {
            if (wchFilter[cbLen]== L'@')
            {
                wchFilter[cbLen] = 0;
            }
        }
    }
    else
    {
        goto leave;
    }

    ofn.lpstrFilter = wchFilter;
    ofn.nFilterIndex = 1;
    wcscpy(wchFile, L"file://");

    ofn.lpstrFile = wchFile+wcslen(wchFile);  // prefix the string with "file://"
    ofn.nMaxFile = MAX_PATH;
    if (GetOpenFileName(&ofn))
    {
        wcsncpy(szURL, wchFile, MAX_URL-1);
        bRet = TRUE;
    }

leave:
    return bRet;
}

DWORD CMainWnd::TTAddRef()
{
    BYTE *bTemp;
    if (_TTRefCount == 0)
    {
        bTemp = (BYTE *)LocalAlloc(LMEM_FIXED,  s_iBtns*MAX_TTSTR*sizeof(WCHAR)+s_iBtns*sizeof(LPWSTR));
        if (bTemp)
        {
            _pszTooltips = (LPWSTR *)bTemp;
            for (int i=0; i<s_iBtns; i++)
            {
                   _pszTooltips[i] = (LPWSTR)(bTemp + s_iBtns*sizeof(LPWSTR) + i*MAX_TTSTR*sizeof(WCHAR));
                   LoadString(g_hInstance, s_tbBtns[i].dwData,  _pszTooltips[i], MAX_TTSTR);
            }
         }
    }
    return ++_TTRefCount;
}

DWORD CMainWnd::TTRelease()
{
    if (--_TTRefCount == 0)
    {
        LocalFree(_pszTooltips);
        _pszTooltips = NULL;
    }
    return _TTRefCount;
}


