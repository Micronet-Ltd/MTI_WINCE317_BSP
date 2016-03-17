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

#include "taskbarlist.hpp"
#include "trayp.h"
//#include <shlobj.h>
#include <shobjidl.h>


// From "taskbar.hxx"
#ifndef HHTASKBARCLASSNAME
#define HHTASKBARCLASSNAME     L"HHTaskBar"
#endif


//////////////////////////////////////////////////////////////////////////////
// TaskbarList_t

TaskbarList_t::TaskbarList_t() :
    m_ObjRefCount(0),
    m_hwndTaskbarList(NULL)
{
    WM_ShellHook = RegisterWindowMessage(L"SHELLHOOK");
    m_ObjRefCount =1;
}       


TaskbarList_t::~TaskbarList_t()
{
    // should always have zero
    ASSERT(m_ObjRefCount == 0);
}    

HWND TaskbarList_t::GetTaskbarWindow(void)
{
    HWND hwnd = m_hwndTaskbarList;

    if (hwnd && IsWindow(hwnd))
    {
        goto leave;
    }

    hwnd = FindWindow(HHTASKBARCLASSNAME, NULL);
    if (hwnd)
    {
        m_hwndTaskbarList = hwnd;
    }

leave:
    return hwnd;
}


//////////////////////////////////
//
// IUnknown Methods...
//
STDMETHODIMP TaskbarList_t::QueryInterface(REFIID riid, void **ppReturn)
{
    HRESULT hr = E_NOINTERFACE;
    
    if (!ppReturn)
    {
        goto leave;
    }

    *ppReturn = NULL;

    if (IsEqualIID(riid, IID_IUnknown)) // IUnknown
    {
        *ppReturn = this;
    }
    else if (IsEqualIID(riid, IID_ITaskbarList)) // ITaskbarList
    {
        *ppReturn = (ITaskbarList *)this;
    }
    else if (IsEqualIID(riid, IID_ITaskbarList2)) // ITaskbarList2
    {
        *ppReturn = (ITaskbarList2 *)this;
    }

    if (*ppReturn)
    {
        (*(LPUNKNOWN*)ppReturn)->AddRef();
        hr = S_OK;
    }

leave:
    return hr;
}

STDMETHODIMP_(DWORD) TaskbarList_t::AddRef()
{
   return ++m_ObjRefCount;
}

STDMETHODIMP_(DWORD) TaskbarList_t::Release()
{
   if (--m_ObjRefCount == 0)
   {
      delete this;
      return 0;
   }
   
   return m_ObjRefCount;
}


//////////////////////////////////
//
// ITaskbarList Methods...
//
STDMETHODIMP TaskbarList_t::HrInit(void)
{
    HWND hwndTL = GetTaskbarWindow();

    if (hwndTL == NULL)
    {
        return E_NOTIMPL;
    }

    return S_OK;
}

STDMETHODIMP TaskbarList_t::AddTab(HWND hwnd)
{
    HRESULT hr = E_FAIL;
    LRESULT lRes = 0;
    HWND hwndTL = GetTaskbarWindow();

    if (!hwnd || !IsWindow(hwnd))
    {
        hr = E_INVALIDARG;
        goto leave;
    }

    if (hwndTL)
    {
        lRes = SendMessage(hwndTL, WM_ShellHook, HSHELL_WINDOWCREATED, (LPARAM) hwnd);
    }

    hr = (!lRes ? E_FAIL:S_OK);

leave:
    return hr;
}

STDMETHODIMP TaskbarList_t::DeleteTab(HWND hwnd)
{
    HRESULT hr = E_FAIL;
    LRESULT lRes = 0;
    HWND hwndTL = GetTaskbarWindow();

    if (!hwnd || !IsWindow(hwnd))
    {
        hr = E_INVALIDARG;
        goto leave;
    }

    if (hwndTL)
    {
        lRes = SendMessage(hwndTL, WM_ShellHook, HSHELL_WINDOWDESTROYED, (LPARAM) hwnd);
    }

    hr = (!lRes ? E_FAIL:S_OK);

leave:
    return hr;
}

STDMETHODIMP TaskbarList_t::ActivateTab(HWND hwnd)
{
    HRESULT hr = E_FAIL;
    LRESULT lRes = 0;
    HWND hwndTL = GetTaskbarWindow();

    if (!hwnd || !IsWindow(hwnd))
    {
        hr = E_INVALIDARG;
        goto leave;
    }

    if (hwndTL)
    {
        SendMessage(hwndTL, WM_ShellHook, HSHELL_WINDOWACTIVATED, (LPARAM) hwnd);
        lRes = SendMessage(hwndTL, TBC_SETACTIVEALT , 0, (LPARAM) hwnd);
    }

    hr = (!lRes ? E_FAIL:S_OK);

leave:
    return hr;
}


STDMETHODIMP TaskbarList_t::SetActiveAlt(HWND hwnd)
{
    HRESULT hr = E_FAIL;
    LRESULT lRes = 0;
    HWND hwndTL = GetTaskbarWindow();

    if (!hwnd || !IsWindow(hwnd))
    {
        hr = E_INVALIDARG;
        goto leave;
    }

    if (hwndTL)
    {
        lRes = SendMessage(hwndTL, TBC_SETACTIVEALT , 0, (LPARAM) hwnd);
    }

    hr = (!lRes ? E_FAIL:S_OK);

leave:
    return hr;
}

STDMETHODIMP TaskbarList_t::MarkFullscreenWindow(HWND hwnd, BOOL fFullscreen)
{
    HRESULT hr = E_FAIL;
    LRESULT lRes = 0;
    HWND hwndTL = GetTaskbarWindow();

    if (!hwnd || !IsWindow(hwnd))
    {
        hr = E_INVALIDARG;
        goto leave;
    }

    if (hwndTL)
    {
        lRes = SendMessage(hwndTL, TBC_MARKFULLSCREEN, (WPARAM) fFullscreen, (LPARAM) hwnd);
    }

    hr = (!lRes ? E_FAIL:S_OK);

leave:
    return hr;
};


