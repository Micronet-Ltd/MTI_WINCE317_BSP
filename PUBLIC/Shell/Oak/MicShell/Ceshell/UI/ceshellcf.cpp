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

#include "ceshellcf.h"
#include "deskfolder.h"
#include "drivesfolder.h"
#include "guid.h"
#include "netfolder.h"
#include "recyclefolder.h"
#include "taskbarlist.hpp"


STDAPI DllCanUnloadNow()
{
   return S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID * ppv)
{
   HRESULT hr = NOERROR; 
 
   if (IsEqualGUID(CLSID_CEShellBitBucket, rclsid) ||
       IsEqualGUID(CLSID_CEShellDesktop, rclsid) ||
       IsEqualGUID(CLSID_CEShellDrives, rclsid) ||
       IsEqualGUID(CLSID_CEShellNetwork, rclsid) ||
       IsEqualGUID(CLSID_TaskbarList, rclsid))
   {
      CShellClassFactory * pShellClassFactory = new CShellClassFactory(rclsid);
      if (pShellClassFactory)
      { 
         hr = pShellClassFactory->QueryInterface(riid, ppv);
         pShellClassFactory->Release();
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
   }
   else
   {
      hr = CLASS_E_CLASSNOTAVAILABLE;
   }

   return hr;
}


//////////////////////////////////////////////////
// CShellClassFactory

CShellClassFactory::CShellClassFactory(REFCLSID rclsid) :
   m_ObjRefCount(1),
   m_clsid(rclsid)
{
// g_DllRefCount++;
}

CShellClassFactory::~CShellClassFactory()
{
// g_DllRefCount--;
}


//////////////////////////////////////////////////
// IUnknown

STDMETHODIMP CShellClassFactory::QueryInterface(REFIID riid,
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

   if (IsEqualIID(riid, IID_IUnknown)) // IUnknown
   {
      *ppobj = this;
   }
   else if (IsEqualIID(riid, IID_IClassFactory)) // IClassFactory
   {
      *ppobj = (IClassFactory *) this;
   }

   if (*ppobj)
   {
      (*(LPUNKNOWN*)ppobj)->AddRef();
      hr = S_OK;
   }

leave:
   return hr;
}

STDMETHODIMP_(ULONG) CShellClassFactory::AddRef(VOID)
{
   return ++m_ObjRefCount;
}

STDMETHODIMP_(ULONG) CShellClassFactory::Release(VOID)
{
   if (--m_ObjRefCount == 0)
   {
      delete this;
      return 0;
   }
   
   return m_ObjRefCount;
}


//////////////////////////////////////////////////
// IClassFactory

STDMETHODIMP CShellClassFactory::CreateInstance(IUnknown * pUnkOuter,
                                                REFIID riid,
                                                void ** ppv)
{
   if (pUnkOuter)
      return CLASS_E_NOAGGREGATION;

   HRESULT hr = NOERROR;
   IShellFolder * pShellFolder = NULL;
   ITaskbarList * pTaskbarList = NULL;

   if (IsEqualGUID(CLSID_CEShellBitBucket, m_clsid))
   {
      pShellFolder = new CRecycleBinFolder();
   }
   else if (IsEqualGUID(CLSID_CEShellDesktop, m_clsid))
   {
      pShellFolder = new CDesktopFolder();
   }
   else if (IsEqualGUID(CLSID_CEShellDrives, m_clsid))
   {
      pShellFolder = new CDrivesFolder();
   }
   else if (IsEqualGUID(CLSID_CEShellNetwork, m_clsid))
   {
      pShellFolder = new CNetworkFolder();
   }
   else if (IsEqualGUID(CLSID_TaskbarList, m_clsid))
   {
      pTaskbarList = new TaskbarList_t();
   }
   else
   {
      hr = E_NOINTERFACE;
   }

   if (SUCCEEDED(hr))
   {
      if (pShellFolder)
      {
         hr = pShellFolder->QueryInterface(riid, ppv);
         pShellFolder->Release();
      }
      else if (pTaskbarList)
      {
         hr = pTaskbarList->QueryInterface(riid, ppv);
         pTaskbarList->Release();
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
   }

   return hr;
}

STDMETHODIMP CShellClassFactory::LockServer(BOOL fLock)
{
   return NOERROR;
}

