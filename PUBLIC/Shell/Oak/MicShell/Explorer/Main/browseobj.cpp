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

#include "windows.h"
#include "browseobj.h"
#include "resource.h"

#define INITGUID
#include "initguid.h"

extern HINSTANCE g_hInstance;

DEFINE_GUID(CLSID_WebBrowser,       0x8856F961L, 0x340A, 0x11D0, 0xA9, 0x6B, 0x00, 0xC0, 0x4F, 0xD7, 0x05, 0xA2);
DEFINE_GUID(IID_IWebBrowser,        0xEAB22AC1L, 0x30C1, 0x11CF, 0xA7, 0xEB, 0x00, 0x00, 0xC0, 0x5B, 0xAE, 0x0B);
DEFINE_GUID(IID_IWebBrowser2,       0xD30C1661L, 0xCDAF, 0x11D0, 0x8A, 0x3E, 0x00, 0xC0, 0x4F, 0xC9, 0xE2, 0x6E);
DEFINE_GUID(DIID_DWebBrowserEvents, 0xEAB22AC2L, 0x30C1, 0x11CF, 0xA7, 0xEB, 0x00, 0x00, 0xC0, 0x5B, 0xAE, 0x0B);
DEFINE_GUID(DIID_DWebBrowserEvents2, 0x34A715A0L, 0x6587, 0x11D0, 0x92, 0x4A, 0x00, 0x20, 0xAF, 0xC7, 0xAC, 0x4D);
DEFINE_GUID(IID_IWebBrowserApp,     0x0002DF05L, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

const GUID SID_SDocHost = { 0xc6504990, 0xd43e, 0x11cf, { 0x89, 0x3b, 0x00, 0xaa, 0x00, 0xbd, 0xce, 0x1a}};

#ifdef USE_REGISTRY_FOR_FULLSCREEN
TCHAR const c_szShellKey[] = TEXT("Software\\Microsoft\\Shell");
TCHAR const c_szShowFullScreen[] = TEXT("ShowBrowserFullScreen");

BOOL ShowFullScreen()
{
    HKEY hKey;
    DWORD  dwValue;
    DWORD dwSize = sizeof( dwValue );
    // defaults to true
    BOOL bRet = TRUE;

    if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szShellKey, 0, 0, &hKey))
    {
        goto exit;
    }

    // get the current scheme name
    if(ERROR_SUCCESS != RegQueryValueEx(hKey, c_szShowFullScreen, NULL, NULL, (LPBYTE) &dwValue, &dwSize ))
    {
        goto exit;
    }

    bRet = ( dwValue != 0 );

exit:

    RegCloseKey(hKey);
    return bRet;
}
#endif // USE_REGISTRY_FOR_FULLSCREEN

BOOL RegisterBrowserWnd()
{
        static BOOL bRegistered = FALSE;

        WNDCLASS        wc;

        if(!bRegistered)
        {
            wc.style         = 0;
            wc.lpfnWndProc   = (WNDPROC)CBrowseObj::s_BrowseWndProc;
            wc.cbClsExtra    = 0;
            wc.cbWndExtra    = 0;
            wc.hInstance     = g_hInstance;
            wc.hIcon         = NULL; // LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_IE));
            wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
            wc.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_WINDOW);
            wc.lpszMenuName  = NULL;
            wc.lpszClassName = TEXT("Explore");

            if (RegisterClass(&wc))
                bRegistered = TRUE;
        }
        return bRegistered;

}

CBrowseObj::CBrowseObj()
{
    _ulRefs = 1;
    _hWnd = NULL;
    _pBrowser = NULL;
    _pObject = NULL;
    _pCP = NULL;

    hEvent = NULL;       
}

CBrowseObj::~CBrowseObj()
{           
    if(_pBrowser)
    {
        _pBrowser->Release();
    }

    if (hEvent)
    {
        CloseHandle(hEvent);
        hEvent = NULL;
    }
}   


BOOL CBrowseObj::Create( DWORD dwStyle )
{
    RECT rcArea;

    DWORD dwStyleDefault =
        WS_VISIBLE | WS_OVERLAPPED | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
        /*|WS_OVERLAPPED|WS_BORDER|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX*/

    DWORD dwExStyle = 0/*WS_EX_OVERLAPPEDWINDOW*/;

    HMENU hMenu = NULL;

    if ( dwStyle == 0 )
    {
        dwStyle = dwStyleDefault;
    }

    if(!RegisterBrowserWnd())
        return FALSE;
        
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcArea, 0);

#ifdef USE_REGISTRY_FOR_FULLSCREEN
    if ( !ShowFullScreen() )
    {
        rcArea.left += 20;
        rcArea.top += 20;
        rcArea.right -= 40;
        rcArea.bottom -= 30;
    }       
#endif USE_REGISTRY_FOR_FULLSCREEN

    LANGID userLangID = GetUserDefaultUILanguage();

    if (LANG_ARABIC == PRIMARYLANGID(userLangID) ||
        LANG_FARSI == PRIMARYLANGID(userLangID) ||
        LANG_HEBREW == PRIMARYLANGID(userLangID) ||
        LANG_URDU == PRIMARYLANGID(userLangID))
    {
        dwExStyle |= WS_EX_LAYOUTRTL;
    }

    _hWnd = ::CreateWindowEx(dwExStyle,
                             TEXT("Explore"),
                             TEXT(""),
                             dwStyle,
                             rcArea.left,
                             rcArea.top,
                             rcArea.right - rcArea.left,
                             rcArea.bottom - rcArea.top,
                             NULL, hMenu, g_hInstance, 0);
    if (!_hWnd)
        return FALSE;

    SetWindowLong(_hWnd, GWL_USERDATA, (DWORD)this);
    GetWindowRect(_hWnd, &_rcWnd);

    if (!CreateCommandBar())
        return FALSE; 

    if(!CreateStatusBar())
        return FALSE;
        
    if (!(_hWndBrowser = CreateBrowser()))
        return FALSE;

 
    SetFocus(_hWnd);

    return TRUE;
}

HWND CBrowseObj::CreateBrowser()
{
    HRESULT hr;
    IUnknown *pUnk = NULL;
    IOleObject *pObject = NULL;
    
    if (!_pBrowser)
    {
        hr = CoCreateInstance(CLSID_WebBrowser, NULL,
                              CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
                              IID_IUnknown, (LPVOID *)(&pUnk));
        if (hr != S_OK)
            return FALSE;

        hr = pUnk->QueryInterface(IID_IOleObject, (LPVOID *)(&pObject));
        if (hr != S_OK)
            goto Cleanup;

        DWORD dwFlags;
        hr = pObject->GetMiscStatus(DVASPECT_CONTENT, &dwFlags);
        if (hr != S_OK)
            goto Cleanup;

        if (dwFlags & OLEMISC_SETCLIENTSITEFIRST)
        {
            IOleClientSite *pClientSite;
            hr = QueryInterface(IID_IOleClientSite, (LPVOID *)(&pClientSite));
            if (hr != S_OK)
                goto Cleanup;

            hr = pObject->SetClientSite(pClientSite);
            pClientSite->Release();
            if (hr != S_OK)
                goto Cleanup;
        }

        hr = Activate(pObject);
        if (hr != S_OK)
            goto Cleanup;

        hr = _pObject->QueryInterface(IID_IWebBrowser2, (void **)&_pBrowser);
        if (hr != S_OK)
            goto Cleanup;


        // do we care if this fails to load BHOs??
        _LoadBrowserHelperObjects();
        
        // See if there might be a url in lpszUrl
        hr = pUnk->QueryInterface(IID_IOleInPlaceActiveObject, (LPVOID *)(&_pIPActiveObj));
        if (S_OK!=hr)
            _pIPActiveObj = NULL;

        hr = InitEvents();
        
    }

Cleanup:
    if (pUnk)
        pUnk->Release();
    if (pObject)
        pObject->Release();

    IOleWindow *pWnd = NULL;
    HWND hwndBrowser = NULL;

    if (_pBrowser)
    {
        hr = _pBrowser->QueryInterface(IID_IOleWindow, (LPVOID *)(&pWnd));
        if (hr != S_OK)
            return NULL;
    }

    if (pWnd)
    {
        hr = pWnd->GetWindow(&hwndBrowser);
        pWnd->Release();
    }
    return hwndBrowser;
}

HRESULT CBrowseObj::InitEvents()
{
    HRESULT                     hr;
    IConnectionPointContainer  *pCPCont = NULL;
    DWebBrowserEvents          *pEvents = NULL;

    if (!_pBrowser)
        return S_FALSE;
    hr = _pBrowser->QueryInterface(IID_IConnectionPointContainer, (LPVOID *)&pCPCont);
    if (hr != S_OK)
        return S_FALSE;
    hr = pCPCont->FindConnectionPoint(DIID_DWebBrowserEvents2, &_pCP);
    if (hr != S_OK)
    {
        _pCP = NULL;
        goto Cleanup;
    }

    hr = QueryInterface(DIID_DWebBrowserEvents2, (LPVOID *)(&pEvents));
    if (hr != S_OK)
        goto Cleanup;
    hr = _pCP->Advise(pEvents, &(_dwEventCookie));
    if (hr != S_OK)
        goto Cleanup;


    Cleanup:
    if (pCPCont)
        pCPCont->Release();
    if (pEvents)
        pEvents->Release();
    return hr;
}


HRESULT CBrowseObj::Activate(IOleObject *pObject)
{
    _pObject = pObject;
    _pObject->AddRef();

    RECT rc;
    ::GetClientRect(_hWnd, &rc);

    HRESULT hr;
    hr    =    _pObject->DoVerb(   OLEIVERB_UIACTIVATE,    NULL,    this,    0,    _hWnd,    &rc);

    if (hr != S_OK)
        goto Cleanup;

    Cleanup:
    return hr;
}


void CBrowseObj::Close()
{

    // Tell the shell browser to die off
    SendMessage(_hWndBrowser, WM_CLOSE, 0,0);

    if(_pIPActiveObj)
    {
        _pIPActiveObj->Release();
    }
    _pIPActiveObj = NULL;
    if(_pCP)
    {
        _pCP->Unadvise(_dwEventCookie);
        _pCP->Release();
    }
    _pCP = NULL;
    if(_pObject)
    {
        _pObject->Close(FALSE);
        _pObject->Release();
    }
    _pObject = NULL;
    if(_pBrowser)
        _pBrowser->Release();
    _pBrowser = NULL;

}

LRESULT  CALLBACK CBrowseObj::s_BrowseWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CBrowseObj *pBrowseObj = (CBrowseObj*)GetWindowLong(hwnd, GWL_USERDATA);

    if(pBrowseObj)
        return pBrowseObj->BrowseWndProc(hwnd, message, wParam, lParam);
    else
        return DefWindowProc(hwnd, message, wParam, lParam);
}

