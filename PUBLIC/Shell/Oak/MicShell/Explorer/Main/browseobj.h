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

#ifndef _BROWSEOBJ_H
#define _BROWSEOBJ_H

#include "mshtmhst.h"
#include "interned.h"
#include "shlobj.h"


class CBrowseObj  :
    public IOleContainer,
    public IOleClientSite,
    public IOleInPlaceSite,
    public IOleControlSite,
    public IServiceProvider,
    public DWebBrowserEvents2,
    public IDocHostUIHandler,
    public IDocHostShowUI,              // msgbox, help window
    public IHTMLOMWindowServices,   // for window move, resize events
    public IShellBrowser
{

public:

    inline void * __cdecl operator new(size_t cb) { return LocalAlloc(LPTR, cb); } 
    inline void * __cdecl operator new[](size_t cb) { return LocalAlloc(LPTR, cb); } 
    inline void __cdecl operator delete(void * pv) { LocalFree(pv); }

    CBrowseObj();
    virtual ~CBrowseObj();

//IUnknown methods
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppv)
    {
        if ((riid == IID_IOleContainer) || (riid == IID_IUnknown))
        {
            *ppv = (IOleContainer *) this;
        }
        else if (riid == IID_IOleClientSite)
        {
            *ppv = (IOleClientSite *)this;
        }
        else if (riid == IID_IOleInPlaceSite)
        {
            *ppv = (IOleInPlaceSite *)this;
        }
        else if (riid == IID_IOleControlSite)
        {
            *ppv = (IOleControlSite *)this;
        }
        else if (riid == IID_IOleWindow)
        {
            *ppv = (IOleWindow *)(IOleInPlaceSite *)this;
        }
        else if ((riid == DIID_DWebBrowserEvents2) || (riid == IID_IDispatch))
        {
            *ppv = (DWebBrowserEvents2 *)this;
        }
        else if(riid == IID_IServiceProvider)
        {
            *ppv = (IServiceProvider *)this;
        }
        else if (riid == IID_IDocHostUIHandler)
        {
            *ppv = (IDocHostUIHandler *)this;
        }
        else if (riid == IID_IDocHostShowUI)
        {
            *ppv = (IDocHostShowUI *)this;
        }
        else if (riid == IID_IHTMLOMWindowServices)
        {
            *ppv = (IHTMLOMWindowServices *)this;
        }
        else if(riid == IID_IShellBrowser)
        {
            *ppv = (IShellBrowser *) this;
        }
        else
        {
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

    // IOleContainer methods
    STDMETHOD(ParseDisplayName)(IBindCtx *, LPOLESTR, ULONG *, IMoniker **) { return E_NOTIMPL;}
    STDMETHOD(EnumObjects)(DWORD, IEnumUnknown **)
    {
        return E_NOTIMPL;
    }
    STDMETHOD(LockContainer)(BOOL)                          { return S_OK;}

    // IOleClientSite methods
    STDMETHOD(SaveObject) (void)                            { return E_NOTIMPL;}
    STDMETHOD(GetMoniker) (DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER * ppmk) { return E_NOTIMPL;}
    STDMETHOD(GetContainer) (LPOLECONTAINER * ppContainer)
    {
        return E_NOINTERFACE;
    }
    STDMETHOD(ShowObject) (void)                            { return E_NOTIMPL;}
    STDMETHOD(OnShowWindow) (BOOL fShow)                    { return E_NOTIMPL;}
    STDMETHOD(RequestNewObjectLayout) (void)                { return E_NOTIMPL;}

    // IOleWindow methods
    STDMETHOD(GetWindow)(HWND *phwnd)
    {
        *phwnd = _hWnd;
        return S_OK;
    }
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode)        { return E_NOTIMPL;}

    // IOleInPlaceSite methods
    STDMETHOD(CanInPlaceActivate)   (void)                  { return S_OK;}
    STDMETHOD(OnInPlaceActivate)    (void)
    {
        _bInPlaceActive = TRUE;
        return S_OK;
    }

    STDMETHOD(OnUIActivate)         (void)                  { return E_NOTIMPL;}
    STDMETHOD(GetWindowContext) (
                                LPOLEINPLACEFRAME FAR *     lplpFrame,
                                LPOLEINPLACEUIWINDOW FAR *  lplpDoc,
                                LPRECT                      lprcPosRect,
                                LPRECT                      lprcClipRect,
                                LPOLEINPLACEFRAMEINFO       lpFrameInfo)
    {

       RECT rcWnd;
       GetClientRect(_hWnd, &rcWnd);

       if(lprcPosRect)
            *lprcPosRect = rcWnd;
       if(lprcClipRect)
            *lprcClipRect = rcWnd;
       
        return S_OK;
    }

    STDMETHOD(Scroll)               (SIZE scrollExtent)     { return E_NOTIMPL;}
    STDMETHOD(OnUIDeactivate)       (BOOL fUndoable)        { return E_NOTIMPL;}
    STDMETHOD(OnInPlaceDeactivate)  (void)
    {
        _bInPlaceActive = FALSE;
        return S_OK;
    }

    STDMETHOD(DiscardUndoState)     (void)                  { return E_NOTIMPL;}
    STDMETHOD(DeactivateAndUndo)    (void)                  { return E_NOTIMPL;}
    STDMETHOD(OnPosRectChange)      (LPCRECT lprcPosRect)   { return E_NOTIMPL;}

    // IOleControlSite methods
    STDMETHOD(OnControlInfoChanged)( void) { return E_NOTIMPL; }
    STDMETHOD(LockInPlaceActive)(BOOL fLock) { return E_NOTIMPL; }
    STDMETHOD(GetExtendedControl)(IDispatch **ppDisp) { return E_NOTIMPL; }

    STDMETHOD(TransformCoords)(POINTL *pPtlHimetric, POINTF *pPtfContainer, DWORD dwFlags) {return E_NOTIMPL; }

    STDMETHOD(TranslateAccelerator)( 
           MSG *pMsg,
           DWORD grfModifiers) { return E_NOTIMPL; }

    STDMETHOD(OnFocus)( BOOL fGotFocus) { return E_NOTIMPL; }
    STDMETHOD(ShowPropertyFrame)( void) { return E_NOTIMPL; }

    // IServiceProvider
    STDMETHOD(QueryService)(REFGUID guidService, REFIID riid, void **ppvObj)
    {
                if(guidService == IID_IHTMLOMWindowServices)
                {
                    return QueryInterface(riid, ppvObj);
                }
                else
                    return E_FAIL;
    }

    //DWebBrowserEvents
    //IDispatch methods
    STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo)          { return E_NOTIMPL;}

    STDMETHOD(GetTypeInfo)(UINT itinfo,LCID lcid,ITypeInfo FAR* FAR* pptinfo)   { return E_NOTIMPL;}

    STDMETHOD(GetIDsOfNames)(REFIID riid,OLECHAR FAR* FAR* rgszNames,UINT cNames,
                             LCID lcid, DISPID FAR* rgdispid)                  { return E_NOTIMPL;}

    STDMETHOD(Invoke)(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags,
                      DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult,
                      EXCEPINFO FAR* pexcepinfo,UINT FAR* puArgErr) { return DISP_E_MEMBERNOTFOUND; }

    // IDocHostUIHandler methods
    STDMETHOD(EnableModeless)(BOOL fEnable) { return E_NOTIMPL; }
    STDMETHOD(FilterDataObject)(IDataObject *pDO, IDataObject **ppDORet) { return E_NOTIMPL; }
    STDMETHOD(GetDropTarget)(IDropTarget *pDropTarget, IDropTarget **ppDropTarget) { return E_NOTIMPL; }
    STDMETHOD(GetExternal)(IDispatch **ppDispatch) { return E_NOTIMPL; }
    STDMETHOD(GetHostInfo)(DOCHOSTUIINFO *pInfo) { return E_NOTIMPL; }
    STDMETHOD(GetOptionKeyPath)(LPOLESTR *pchKey, DWORD dw) { return E_NOTIMPL; }
    STDMETHOD(HideUI)(void) { return E_NOTIMPL; }
    STDMETHOD(OnDocWindowActivate)(BOOL fActivate) { return E_NOTIMPL; }
    STDMETHOD(OnFrameWindowActivate)(BOOL fActivate) { return E_NOTIMPL; }
    STDMETHOD(ResizeBorder)(LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fFrameWindow) { return E_NOTIMPL; }
    STDMETHOD(ShowContextMenu)(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved) { return E_NOTIMPL; }
    STDMETHOD(ShowUI)(DWORD dwID, IOleInPlaceActiveObject *pActiveObject,
            IOleCommandTarget *pCommandTarget, IOleInPlaceFrame *pFrame,
            IOleInPlaceUIWindow *pDoc) { return E_NOTIMPL; }
    STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, const GUID *pgudCmdGroup, DWORD nCmdID) { return E_NOTIMPL; }
    STDMETHOD(TranslateUrl)(DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut) { return E_NOTIMPL; }
    STDMETHOD(UpdateUI)(void) { return E_NOTIMPL; }

    // IDocHostShowUI methods
    STDMETHOD(ShowHelp)( HWND hwnd, LPOLESTR pszHelpFile, UINT uCommand,
                        DWORD dwData, POINT ptMouse, IDispatch *pDispatchObjectHit)
                        { return E_NOTIMPL; }


    STDMETHOD(ShowMessage)( HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption, DWORD dwType,
                            LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult)
    {
        int res = MessageBox(hwnd, lpstrText, lpstrCaption, dwType);
        if(plResult)
        *plResult = res;
        return S_OK;
    }

   // IHTMLOMWindowServices methods
    STDMETHOD(moveTo)( LONG x, LONG y) { return E_NOTIMPL; }  
    STDMETHOD(moveBy)( LONG x, LONG y) { return E_NOTIMPL; }
    STDMETHOD(resizeTo)( LONG x,LONG y) { return E_NOTIMPL; }
    STDMETHOD(resizeBy)( LONG x, LONG y) { return E_NOTIMPL; }

    // IOleInPlaceUIWindow methods
    STDMETHOD(GetBorder)(LPRECT lprectBorder){return E_NOTIMPL;}
    STDMETHOD(RequestBorderSpace)(LPCBORDERWIDTHS pborderwidths){return E_NOTIMPL;}
    STDMETHOD(SetBorderSpace)(LPCBORDERWIDTHS pborderwidths){return E_NOTIMPL;}
    STDMETHOD(SetActiveObject)(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName){return E_NOTIMPL;}

    // IShellBrowser
    STDMETHOD(BrowseObject)(LPCITEMIDLIST pidl, UINT wFlags){return E_NOTIMPL;}
    STDMETHOD(EnableModelessSB)(BOOL fEnable){return E_NOTIMPL;}
    STDMETHOD(GetControlWindow)(UINT id, HWND* lphwnd){return E_NOTIMPL;}
    STDMETHOD(GetViewStateStream)(DWORD grfMode, IStream** ppStrm){return E_NOTIMPL;}
    STDMETHOD(InsertMenusSB)(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths){return E_NOTIMPL;}
    STDMETHOD(OnViewWindowActive)(IShellView* ppshv){return E_NOTIMPL;}
    STDMETHOD(QueryActiveShellView)(IShellView** ppshv){return E_NOTIMPL;}
    STDMETHOD(RemoveMenusSB)(HMENU hmenuShared){return E_NOTIMPL;}
    STDMETHOD(SendControlMsg)(UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pret){return E_NOTIMPL;}    
    STDMETHOD(SetMenuSB)(HMENU hmenuShared, HOLEMENU holemenuRes, HWND hwndActiveObject){return E_NOTIMPL;}
    STDMETHOD(SetStatusTextSB)(LPCOLESTR lpszStatusText){return E_NOTIMPL;}
    STDMETHOD(SetToolbarItems)(LPTBBUTTONSB lpButtons, UINT nButtons, UINT uFlags){return E_NOTIMPL;}
    STDMETHOD(TranslateAcceleratorSB)(LPMSG lpmsg, WORD wID){return E_NOTIMPL;}


   static LRESULT  CALLBACK s_BrowseWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    virtual BOOL _LoadBrowserHelperObjects(void) { return TRUE; }
    virtual BOOL Create( DWORD dwStyle = 0 );
    virtual HWND   CreateBrowser();
    virtual BOOL CreateCommandBar() { return TRUE; }
    virtual BOOL CreateStatusBar() { return TRUE; }
    virtual void Close();
    
    
    HRESULT               Activate(IOleObject *pObject);
    HRESULT               InitEvents();
    
public:
    IWebBrowser2       *_pBrowser;
    HANDLE                  hEvent;

    virtual LRESULT BrowseWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
             return DefWindowProc(hwnd, message, wParam, lParam);
    }
    
protected:

    ULONG                     _ulRefs;            //reference count for the interfaces supported by the container
    IOleObject              *_pObject;
    IConnectionPoint        *_pCP;
    IOleInPlaceActiveObject *_pIPActiveObj;
    BOOL                    _bInPlaceActive;
    DWORD                   _dwEventCookie;
    HWND                    _hWndBrowser;       //handle to the browser window
    HWND                      _hWnd;                //the main window
    RECT                     _rcWnd;
    
};

#endif
