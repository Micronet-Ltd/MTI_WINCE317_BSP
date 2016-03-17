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

Module Name:  mainwnd.h

Abstract:  Implements the main window, the container for the webbrowser

Functions:

Notes: This class implements the container and its interaction with the webbrowser control,
        commandbar, statusbar etc.


--*/

#ifndef _MAINWND_H
#define _MAINWND_H_

#include "mshtmhst.h"
#include "interned.h"
#include "shlobj.h"
#include "browseobj.h"

#define MAX_URL  2060

#define MAXURLS             25
#define ID_COMBOEDIT        1001        //ID for the edit box of a combo box
#define PROGRESSPANEWIDTH   100
#define MESSAGEPANEWIDTH    110
#define ICONPANEWIDTH       20
#define SECURITYPANEWIDTH   30


enum SSL_SECURITY_STATE
{
    SSL_SECURITY_UNSECURE,    // Unsecure (no lock)
    SSL_SECURITY_MIXED,       // Mixed security (broken lock?)
    SSL_SECURITY_SECURE,      // Uniform security
    SSL_SECURITY_SECURE_40,   // Uniform security of >= 40 bits
    SSL_SECURITY_SECURE_56,   // Uniform security of >= 56 bits
    SSL_SECURITY_FORTEZZA,    // Fortezza with Skipjack @80 bits
    SSL_SECURITY_SECURE_128   // Uniform security of >= 128 bits
};


#define MAX_TTSTR 80
extern const int s_iBtns;

#define CMDBANDS 3

typedef struct tagMENUDATA {
    UINT uFlags;
    UINT uIDNewItem;
    UINT idszMenuItem;
}MENUDATA, FAR *LPMENUDATA;

class CAnimThread;

interface IInternetSecurityManager;


class CMainWnd :
    public CBrowseObj
{

public:
    CMainWnd();
    ~CMainWnd();

    BOOL Create();

    STDMETHOD(GetWindowContext) (
                    LPOLEINPLACEFRAME FAR *     lplpFrame,
                    LPOLEINPLACEUIWINDOW FAR *  lplpDoc,
                    LPRECT                      lprcPosRect,
                    LPRECT                      lprcClipRect,
                    LPOLEINPLACEFRAMEINFO       lpFrameInfo)
    {
        RECT rcWnd;
        GetClientRect(_hWnd, &rcWnd);

        if(_bShowCmdBar && _hWndCmdband)
            rcWnd.top += CommandBands_Height(_hWndCmdband);
        if(_bShowStatusBar && _hWndStatus)
        {
            RECT rc;
            GetClientRect(_hWndStatus, &rc);
            rcWnd.bottom -= (rc.bottom - rc.top);
        }

        if(lprcPosRect)
            *lprcPosRect = rcWnd;
        if(lprcClipRect)
            *lprcClipRect = rcWnd;

        return S_OK;
    }

    // IHTMLOMWindowServices virtual methods
    STDMETHOD(moveTo)( LONG x, LONG y);
    STDMETHOD(moveBy)( LONG x, LONG y);
    STDMETHOD(resizeTo)( LONG x,LONG y);
    STDMETHOD(resizeBy)( LONG x, LONG y);

    // IDispatch virtual
    STDMETHOD(Invoke)(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags,
                    DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult,
                    EXCEPINFO FAR* pexcepinfo,UINT FAR* puArgErr);

    // IDocHostUIHandler virtual
    STDMETHOD(GetHostInfo)(DOCHOSTUIINFO *pInfo);

    // IShellBrowser
    STDMETHOD(GetControlWindow)(UINT id, HWND* lphwnd);
    STDMETHOD(InsertMenusSB)(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
    STDMETHOD(RemoveMenusSB)(HMENU hmenuShared);
    STDMETHOD(SendControlMsg)(UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pret);
    STDMETHOD(SetMenuSB)(HMENU hmenuShared, HOLEMENU holemenuRes, HWND hwndActiveObject);
    STDMETHOD(SetToolbarItems)(LPTBBUTTONSB lpButtons, UINT nButtons, UINT uFlags);

    // IOleControlSite methods
    STDMETHOD(LockInPlaceActive)(BOOL fLock) { return S_OK; }
    STDMETHOD(TranslateAccelerator)(MSG *pMsg, DWORD grfModifiers);
    STDMETHOD(OnFocus)(BOOL fGotFocus) { return S_OK; }

    // CBrowseObj virtual
    HWND CMainWnd::CreateBrowser();
    BOOL CreateCommandBar();
    BOOL AdjustCommandBar();
    BOOL CreateStatusBar();

    BOOL CreateIECommandBar(BOOL fCloseBox, TBBUTTON *tbBtns, int numButtons, long idMenu);
    BOOL CreateAddressBar(UINT fStyle, UINT cxWidth);
    BOOL CreateMenuBar(LONG idMenu, UINT fStyle, UINT cxWidth);
    BOOL CreateToolBar(TBBUTTON *tbBtns, int numButtons, UINT fStyle, UINT cxWidth);
    BOOL ShowAddressBar(BOOL fShow);
    BOOL ShowStatusBar(BOOL fShow);
    VOID ResizeBrowserWindow();
    BOOL LoadToolbarSettings(LPCWSTR szHost);
    BOOL SaveToolbarSettings(LPCWSTR szHost, BOOL fUseRegistry);
    VOID LoadTypedUrls();
    VOID SaveTypedUrls();
    BOOL _LoadBrowserHelperObjects(void);
    VOID UpdateUrlList(LPCTSTR lpszUrl);
    VOID SelChange();
    VOID SetCurrentLocation();

    VOID EnterDownload();
    VOID ExitDownload();
    UINT ExtHandleCommand(WPARAM wParam, LPARAM lParam);


public:
    BOOL PreTranslateMessage(LPMSG pMsg);
    VOID Close();
    HWND GetWindow() { return _hWnd;}
    LRESULT HandleCommand(WPARAM wParam, LPARAM lParam);
    LRESULT HandleNotify(WPARAM wParam, LPARAM lParam);
    VOID HandleSettingChange(WPARAM wParam, LPARAM lParam);
    BOOL IsWebBrowser();
    void ShowZoneInfo();
    BOOL UpdateZoneInfo();

    LRESULT BrowseWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK AddressEditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    WNDPROC _lpfnAddressEditProc;
    LPWSTR _lpszUrl;
    LPITEMIDLIST _pidl;
    bool _bEmpty:1;
    bool _bFullScreen:1;

    LPSTREAM _pStrm;  // for new window marshaling
    IInternetSecurityManager *_pInternetSecurityManager;


protected:

    // Added for Explorer ToolTips
    static DWORD _TTRefCount;
    static LPWSTR *_pszTooltips;  // IE TT strings
    static DWORD TTAddRef();
    static DWORD TTRelease();

    //Added for HTML Find Dialogs
    HRESULT FindString();
    VOID ChangeFontSize(BOOL fInit);
    BOOL RegisterMainWnd();
    BOOL HandleMenuPopup(WPARAM wParam, LPARAM lParam);
    LRESULT ToggleFullScreen();

    //Added for Creation of Shared Menu
    HMENU CreateSubMenu(LPMENUDATA lpMenuData, int nItems, LONG* plInsertedItems);
    void HandleSharedMenuPopup(HMENU hMenu);


    HACCEL _hAccelTbl;    //a handle to the accelerator table

    CFavoritesMenu *_Favorites;
    bool _bInMenuLoop:1;    // for populating the favorites menu

    CAnimThread *_pthrdAnimation;    //the animation thread
    bool _bShowAnimation:1;

    DWORD _dwZone;

// Command Bar windows
    HWND _hWndCmdband;
    HWND _hWndMenuBar;
    HWND _hWndAddrBar;
    HWND _hWndToolBar;
    HWND _hWndStatus;
    HWND _hWndAddressCombo;    //the combo box for typed urls
    HWND _hWndAddressEdit;    //the edit box of that combo box
    HWND _hWndProgress;    // progress bar
    HIMAGELIST _himlCmdBand;
    COMMANDBANDSRESTOREINFO _cbRestoreInfo[2][CMDBANDS]; // IE & Shared Bands
// Command Bar

    bool _bTyped:1;
    bool _bSelected:1;
    bool _bCheckedOffline:1;
    bool _bShowCmdBar:1;
    bool _bShowAddrBar:1;
    bool _bShowStatusBar:1;
    bool _bInWebBrowserMode:1;
    bool _bShowHelp:1;
    bool _bClosing:1;
    
    WCHAR _szTitle[MAX_URL];   //title of the current document

    int _wZoom;
    int _wDLCCounter;
};

#endif	//_MAINWND_H_
