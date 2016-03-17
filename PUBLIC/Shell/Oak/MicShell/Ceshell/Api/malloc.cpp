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

#include "malloc.h"

//////////////////////////////////////////////////
// CShellMalloc

CShellMalloc::CShellMalloc() :
   m_ObjRefCount(0)
{
   m_ObjRefCount = 1;
}

CShellMalloc::~CShellMalloc()
{
}


//////////////////////////////////////////////////
// IUnknown

STDMETHODIMP CShellMalloc::QueryInterface(REFIID riid,
                                          LPVOID FAR * ppobj)
{
   HRESULT hr = E_NOINTERFACE;

   if (!ppobj)
   {
      ASSERT(0);
      hr = E_INVALIDARG;
      goto leave;
   }

   *ppobj = NULL;

   if(IsEqualIID(riid, IID_IUnknown)) // IUnknown
   {
      *ppobj = this;
   }
   else if(IsEqualIID(riid, IID_IMalloc)) // IMalloc
   {
      *ppobj = (IMalloc *) this;
   }

   if(*ppobj)
   {
      (*(LPUNKNOWN*)ppobj)->AddRef();
      hr = S_OK;
   }

leave:
   return hr;
}

STDMETHODIMP_(ULONG) CShellMalloc::AddRef(VOID)
{
   return ++m_ObjRefCount;
}

STDMETHODIMP_(ULONG) CShellMalloc::Release(VOID)
{
   if (--m_ObjRefCount == 0)
   {
      delete this;
      return 0;
   }
   
   return m_ObjRefCount;
}


//////////////////////////////////////////////////
// IMalloc

STDMETHODIMP_(LPVOID) CShellMalloc::Alloc(ULONG cb)
{
   return LocalAlloc(LMEM_FIXED, cb);
}

STDMETHODIMP_(LPVOID) CShellMalloc::Realloc(void * pv,
                                         ULONG cb)
{
   return LocalReAlloc(pv, cb, 0);
}


STDMETHODIMP_(VOID) CShellMalloc::Free(void * pv)
{
   LocalFree(pv);
}


STDMETHODIMP_(ULONG) CShellMalloc::GetSize(void * pv)
{
   return LocalSize(pv);
}


STDMETHODIMP_(int) CShellMalloc::DidAlloc(void * pv)
{
   return -1;
}

STDMETHODIMP_(VOID) CShellMalloc::HeapMinimize()
{
}

