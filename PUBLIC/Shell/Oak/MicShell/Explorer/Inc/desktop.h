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
#ifndef _DESKTOP_H_
#define _DESKTOP_H_

#include "mshtmhst.h"
#include "interned.h"
#include <shobjidl.h>
#include <shlobj.h>


class CDesktopWnd :
    public IShellBrowser
{
    public:

        inline void * __cdecl operator new(size_t cb) { return LocalAlloc(LPTR, cb); } 
        inline void * __cdecl operator new[](size_t cb) { return LocalAlloc(LPTR, cb); } 
        inline void __cdecl operator delete(void * pv) { LocalFree(pv); }

        CDesktopWnd();
        ~CDesktopWnd();
        
        BOOL Create();
        
//IUnknown methods
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppv)
    {
        if ((riid == IID_IShellBrowser) || (riid == IID_IUnknown))
        {
            *ppv = (IShellBrowser *) this;
        }
        else
        {
            RETAILMSG(1, (L"Desktop Window QI, Unknown interface"));
            *ppv = NULL;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

    STDMETHOD_(ULONG, AddRef) (void)
    {
        InterlockedIncrement((LONG*)&_ulRefs);
        return _ulRefs;
    }

    STDMETHOD_(ULONG, Release) (void)
    {
        ULONG ulRefs = _ulRefs;
        if (InterlockedDecrement((LONG*)&_ulRefs) == 0)
        {
            delete this;
            return 0;
        }
        return ulRefs - 1;
    }

    // inline accessor   
        HWND GetWindow() { return _hWnd; }

    // IOleWindow
        STDMETHOD(GetWindow)(HWND *phwnd);  
        STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode) { return E_NOTIMPL; }


   // IShellBrowser
        STDMETHOD(InsertMenusSB)( 
             HMENU hmenuShared,
             LPOLEMENUGROUPWIDTHS lpMenuWidths) { return E_NOTIMPL; }
        
        STDMETHOD(SetMenuSB)( 
             HMENU hmenuShared,
             HOLEMENU holemenuRes,
             HWND hwndActiveObject) { return S_OK; }
        
        STDMETHOD(RemoveMenusSB)( 
             HMENU hmenuShared) { return E_NOTIMPL; }
        
        STDMETHOD(SetStatusTextSB)( 
            LPCOLESTR pszStatusText) { return E_NOTIMPL; }
        
        STDMETHOD(EnableModelessSB)( 
             BOOL fEnable) { return E_NOTIMPL; }
        
        STDMETHOD(TranslateAcceleratorSB)( 
             MSG  *pmsg,
             WORD wID);
        
        STDMETHOD(BrowseObject)( 
             LPCITEMIDLIST pidl,
             UINT wFlags);
        
        STDMETHOD(GetViewStateStream)( 
             DWORD grfMode,
             IStream  **ppStrm) { return E_NOTIMPL; }
        
        STDMETHOD(GetControlWindow)( 
             UINT id,
            HWND  *lphwnd) { return E_NOTIMPL; }
        
        STDMETHOD(SendControlMsg)( 
             UINT id,
             UINT uMsg,
             WPARAM wParam,
             LPARAM lParam,
             LRESULT  *pret) { return E_NOTIMPL; }
        
        STDMETHOD(QueryActiveShellView)( 
             IShellView  **ppshv) { return E_NOTIMPL; }
        
        STDMETHOD(OnViewWindowActive)( 
             IShellView  *pshv) { return E_NOTIMPL; }
        
        STDMETHOD(SetToolbarItems)( 
             LPTBBUTTONSB lpButtons,
             UINT nButtons,
             UINT uFlags) { return E_NOTIMPL; }
        
    
        LRESULT  DesktopWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
        static       LRESULT CALLBACK s_DesktopWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
 
private:
    HWND        _hWnd;

    IShellView * _psv;
    
    ULONG       _ulRefs;
};

#endif
