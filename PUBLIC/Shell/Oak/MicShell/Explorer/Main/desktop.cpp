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
// Include the automation definitions...
#include <exdisp.h>
#include <exdispid.h>
#include <mshtmdid.h> // AMBIENT_DLCONTROL

#include "desktop.h"
#include "resource.h"

extern HINSTANCE g_hInstance;

HRESULT HandleNewWindow2(LPTSTR lpszUrl, LPCITEMIDLIST pidl, DISPPARAMS FAR* pdparams);

CDesktopWnd::CDesktopWnd()
{
    _ulRefs = 1;
    _hWnd = NULL;
    _psv = NULL;
}

CDesktopWnd::~CDesktopWnd()
{
    if(_psv)
        _psv->Release();

    if(_hWnd)
        DestroyWindow(_hWnd); 
 
}


HRESULT CDesktopWnd::GetWindow(HWND *phwnd)
{
    if(phwnd)
        *phwnd = _hWnd;
       
    return S_OK;
}


HRESULT CDesktopWnd::BrowseObject( 
             LPCITEMIDLIST pidl,
             UINT wFlags)
{

    return HandleNewWindow2(NULL, pidl, NULL);

}

HRESULT CDesktopWnd::TranslateAcceleratorSB( MSG  *pmsg, WORD wID)
{

    if(_psv)
       return  _psv->TranslateAccelerator(pmsg);
    else
       return  E_NOTIMPL;

}

BOOL CDesktopWnd::Create()
{

    IShellFolder    *pSHF;
    FOLDERSETTINGS fs;
    RECT rc;
    HRESULT hr = E_FAIL;
    
    // Get a shell folder for the desktop
    hr = SHGetDesktopFolder(&pSHF);
    if(hr || !pSHF)
        goto Cleanup;

    // create a shell view for it
    hr = pSHF->CreateViewObject(NULL, IID_IShellView, (LPVOID *)&_psv);
    if(hr || !_psv)
        goto Cleanup;

    fs.ViewMode = FVM_ICON;
    fs.fFlags = FWF_DESKTOP | FWF_ALIGNLEFT | FWF_NOSCROLL;

    SetRect(&rc, 0, 0, GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN));
    
    // create the desktop's view window (no need to AddRef since CreateViewWindow does it)
    hr = _psv->CreateViewWindow(NULL,  &fs, (IShellBrowser *)this, &rc, &_hWnd);
    if(hr || !_hWnd)
    {
        Release();
        goto Cleanup;
    }

    RegisterDesktop(_hWnd);

Cleanup:
    if(pSHF)
        pSHF->Release();

   return (hr == S_OK);    
}



