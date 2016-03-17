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

#include "nsenum.h"
#include "fsenum.h"
#include "guid.h"
#include "idlist.h"

//////////////////////////////////////////////////
// CNameSpaceEnum

CNameSpaceEnum::CNameSpaceEnum(SHCONTF grfFlags) :
   m_ObjRefCount(0),
   m_hkey(NULL),
   m_dwIndex(0),
   m_pidlFS(NULL),
   m_pEnumFS(NULL),
   m_grfFlags(grfFlags),
   m_bUsingFilter(FALSE)
{
   ZeroMemory(&m_clsid, sizeof(m_clsid));
   m_ObjRefCount = 1;
}
 
CNameSpaceEnum::~CNameSpaceEnum()
{
   Close();

   if (m_pidlFS)
      ILFree(m_pidlFS);
}

BOOL CNameSpaceEnum::Initialize(REFCLSID clsid, LPCITEMIDLIST pidlFS)
{
   m_clsid = clsid;

   if (pidlFS)
   {
      m_pidlFS = ILCopy(pidlFS, IL_ALL);
      if (!m_pidlFS)
         return FALSE;
   }

   return SUCCEEDED(Reset());
} 

void CNameSpaceEnum::Close()
{
   if (NULL != m_hkey)
   {
      RegCloseKey(m_hkey);
      m_hkey = NULL;
      m_dwIndex = 0;
   }

   if (m_pEnumFS)
   {
      m_pEnumFS->Release();
      m_pEnumFS = NULL;
   }
}


//////////////////////////////////////////////////
// IUnknown

STDMETHODIMP CNameSpaceEnum::QueryInterface(REFIID riid, LPVOID FAR* ppobj)
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
   else if(IsEqualIID(riid, IID_IEnumIDList)) // IEnumIDList
   {
      *ppobj = (IEnumIDList *) this;
   }
   else if (IsEqualIID(riid, IID_IEnumFilter)) // IEnumFilter
   {
      *ppobj = (IEnumFilter *) this;
   }

   if(*ppobj)
   {
      (*(LPUNKNOWN*)ppobj)->AddRef();
      hr = S_OK;
   }

leave:
   return hr;
}

STDMETHODIMP_(ULONG) CNameSpaceEnum::AddRef(VOID)
{
   return ++m_ObjRefCount;
}

STDMETHODIMP_(ULONG) CNameSpaceEnum::Release(VOID)
{
   if (--m_ObjRefCount == 0)
   {
      delete this;
      return 0;
   }
   
   return m_ObjRefCount;
}


//////////////////////////////////////////////////
// IShellFolder 

STDMETHODIMP CNameSpaceEnum::Clone(IEnumIDList ** ppenum)
{
   return E_NOTIMPL;
}

STDMETHODIMP CNameSpaceEnum::Next(ULONG celt,
                                  LPITEMIDLIST * rgelt,
                                  ULONG * pceltFetched)
{
   HRESULT hr = NOERROR;

   ASSERT(rgelt);
   if (!rgelt)
      hr = E_INVALIDARG;

   if (!m_hkey && !m_pEnumFS)
      hr = E_UNEXPECTED;

   if (NOERROR == hr)
   {
nextNameSpace:
      WCHAR szCLSID[MAX_CLSID_STR_LEN+1];
      DWORD cchCLSID = lengthof(szCLSID);
      CLSID clsid = {0};
      LONG lReturn = ERROR_SUCCESS;

      if (m_hkey)
      {
         lReturn = RegEnumValue(m_hkey, m_dwIndex++, szCLSID, &cchCLSID,
                                NULL, NULL, NULL, NULL);
      }
      else
      {
         lReturn = ERROR_NO_MORE_ITEMS;
      }

      if (ERROR_SUCCESS == lReturn)
      {
         hr = CLSIDFromString(szCLSID, &clsid);
         if (m_bUsingFilter &&
            IsEqualGUID(CLSID_CEShellBitBucket, clsid))
         {
            // Do not show Recycle bin to match desktop behavior
            goto nextNameSpace;
         }

         if (SUCCEEDED(hr))
         {
            hr = CreateNameSpacePidl(clsid, rgelt);
            if (SUCCEEDED(hr))
            {
               if (pceltFetched)
                  *pceltFetched = 1;
            }
         }
      }
      else if (ERROR_NO_MORE_ITEMS == lReturn)
      {
         if (m_pEnumFS)
            hr = m_pEnumFS->Next(celt, rgelt, pceltFetched);
         else
            hr = S_FALSE;
      }
      else
      {
         hr = E_FAIL;
      }
   }

   if (NOERROR != hr)
   {
      if (pceltFetched)
         *pceltFetched = 0;
   }

   return hr;
}

STDMETHODIMP CNameSpaceEnum::Reset()
{
   HRESULT hr = NOERROR;

   Close();

   if (IsEqualCLSID(m_clsid, CLSID_CEShellDesktop))
   {
      if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                        L"Explorer\\Desktop",
                                        0, NULL, &m_hkey))
      {
         m_hkey = NULL;
      }
   }
   else if(IsEqualCLSID(m_clsid, CLSID_CEShellDrives))
   {
      if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                        L"Explorer\\My Computer",
                                        0, NULL, &m_hkey))
      {
         m_hkey = NULL;
      }
   }
   else
   {
      // Unknown shell CLSID!
      hr = E_FAIL;
   }

   if (m_pidlFS)
   {
      CFileSystemEnum * pFileSystemEnum = new CFileSystemEnum(m_grfFlags);
      if (pFileSystemEnum)
      {
         if (pFileSystemEnum->Initialize(m_pidlFS))
            m_pEnumFS = pFileSystemEnum;
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
   }

   if (FAILED(hr))
      Close();

   return hr;
}

STDMETHODIMP CNameSpaceEnum::Skip(ULONG celt)
{
   return E_NOTIMPL;
}


//////////////////////////////////////////////////
// IEnumFilter

STDMETHODIMP CNameSpaceEnum::SetFilterString(LPCWSTR pszFilter)
{
   HRESULT hr = E_FAIL;

   m_bUsingFilter = FALSE;

   if (!pszFilter)
   {
      hr = NOERROR;
      goto leave;
   }
    
   if (!m_pEnumFS)
   {
      goto leave;
   }

   IEnumFilter* pIEnumFilter;
   if (SUCCEEDED(m_pEnumFS->QueryInterface(IID_IEnumFilter, (void**)&pIEnumFilter)))
   {
      hr = pIEnumFilter->SetFilterString(pszFilter);
      pIEnumFilter->Release();
   }

    m_bUsingFilter = SUCCEEDED(hr);

leave:
   return hr;
}

