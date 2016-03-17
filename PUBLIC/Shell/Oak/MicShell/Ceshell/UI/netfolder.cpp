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

#include "netfolder.h"
#include "drivesfolder.h"
#include "filefolder.h"
#include "guid.h"
#include "idlist.h"
#include "resource.h"
#include "shelldialogs.h"

//////////////////////////////////////////////////
// CNetworkFolder

CNetworkFolder::CNetworkFolder() :
   m_ObjRefCount(0)
{
// g_DllRefCount++;

   m_ObjRefCount = 1;
}

CNetworkFolder::~CNetworkFolder()
{
// g_DllRefCount--;
}

void CNetworkFolder::ShowNetworkError(HRESULT hr, LPWSTR pwsz)
{
   if (FACILITY_WIN32 != HRESULT_FACILITY(hr))
      return;

   UINT uError = 0;

   switch (HRESULT_CODE(hr))
   {
      case ERROR_ACCESS_DENIED:
         if (pwsz)
            uError = IDS_NET_ERR_ACCESS_DENIED;
         else
            uError = IDS_NET_ERR_ACCESS_GENERIC;
      break;

      case ERROR_BAD_NETPATH:
         uError = IDS_NET_ERR_BAD_NETPATH;
      break;

      case ERROR_NO_NETWORK:
         uError = IDS_NET_ERR_NO_NETWORK;
      break;

      default:
         return;
   }

   ShellDialogs::ShowFileError(NULL, MAKEINTRESOURCE(IDS_TITLE_NETWORKING),
                               MAKEINTRESOURCE(uError), pwsz,
                               MB_ICONWARNING | MB_OK);
}


//////////////////////////////////////////////////
// IUnknown

STDMETHODIMP CNetworkFolder::QueryInterface(REFIID riid,
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
   else if(IsEqualIID(riid, IID_IShellFolder)) // IShellFolder
   {
      *ppobj = (IShellFolder *) this;
   }
   else if(IsEqualIID(riid, IID_IPersistFolder)) // IPersistFolder
   {
      *ppobj = (IPersistFolder *) this;
   }

   if(*ppobj)
   {
      (*(LPUNKNOWN*)ppobj)->AddRef();
      hr = S_OK;
   }

leave:
   return hr;
}

STDMETHODIMP_(ULONG) CNetworkFolder::AddRef(VOID)
{
   return ++m_ObjRefCount;
}

STDMETHODIMP_(ULONG) CNetworkFolder::Release(VOID)
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

STDMETHODIMP CNetworkFolder::BindToObject(LPCITEMIDLIST pidl,
                                          LPBC pbc,
                                          REFIID riid,
                                          LPVOID * ppvOut)
{
   ASSERT(pidl);
   ASSERT(ppvOut);
   if (!pidl || !ppvOut)
      return E_INVALIDARG;
   *ppvOut = NULL;

   HRESULT hr = NOERROR;

   LPITEMIDLIST pidlRoot = ILCopy(pidl, 1);
   if (pidlRoot)
   {
      if (ILIsFileSystemPidl(pidlRoot))
      {
         CFileFolder * pFileFolder = new CFileFolder();
         if (pFileFolder)
         {
            LPITEMIDLIST pidlNext = ILNext(pidl);

            pFileFolder->TreatAsUNC(TRUE);

            if (pidlNext)
            {
               hr = pFileFolder->InitializeNoVerify(pidlRoot);
               if (SUCCEEDED(hr))
                  hr = pFileFolder->BindToObject(pidlNext, pbc, riid, ppvOut);
            }
            else
            {
               hr = pFileFolder->Initialize(pidlRoot);
               if (SUCCEEDED(hr))
                  hr = pFileFolder->QueryInterface(riid, ppvOut);
            }

            // Handle network errors here before handing
            // the error off to the container
            if (FAILED(hr))
               ShowNetworkError(hr, NULL);

            pFileFolder->Release();
         }
         else
         {
            hr =  E_OUTOFMEMORY;
         }
      }
      else
      {
         hr = E_FAIL;
      }

      ILFree(pidlRoot);
   }
   else
   {
      hr =  E_OUTOFMEMORY;
   }

   return hr;
}

STDMETHODIMP CNetworkFolder::BindToStorage(LPCITEMIDLIST pidl,
                                           LPBC pbcReserved,
                                           REFIID riid,
                                           LPVOID * ppvObj)
{
   return E_NOTIMPL;
}

STDMETHODIMP CNetworkFolder::CompareIDs(LPARAM lParam,
                                        LPCITEMIDLIST pidl1,
                                        LPCITEMIDLIST pidl2)
{
   return E_NOTIMPL;
}

STDMETHODIMP CNetworkFolder::CreateViewObject(HWND hwndOwner, 
                                              REFIID riid, 
                                              LPVOID * ppvOut)
{
   return E_NOTIMPL;
}

STDMETHODIMP CNetworkFolder::EnumObjects(HWND hwndOwner,
                                         DWORD grfFlags,
                                         LPENUMIDLIST * ppenumIDList)
{
   return E_NOTIMPL;
}

STDMETHODIMP CNetworkFolder::GetAttributesOf(UINT cidl,
                                             LPCITEMIDLIST * apidl,
                                             ULONG * rgfInOut)
{
   ASSERT(rgfInOut);
   if (!rgfInOut)
      return E_INVALIDARG;

   (*rgfInOut) &= (SFGAO_BROWSABLE | SFGAO_FILESYSTEM | SFGAO_HASSUBFOLDER);

   return NOERROR;
}

STDMETHODIMP CNetworkFolder::GetDisplayNameOf(LPCITEMIDLIST pidl,
                                              DWORD uFlags,
                                              LPSTRRET lpName)
{
   ASSERT(pidl);
   ASSERT(lpName);

   if (!pidl || !lpName)
      return E_INVALIDARG;

   lpName->pOleStr = (LPOLESTR) g_pShellMalloc->Alloc(MAX_PATH * sizeof(WCHAR));
   if (!lpName->pOleStr)
      return E_OUTOFMEMORY;

   lpName->uType = STRRET_WSTR;

   LPWSTR pwszCurrent = lpName->pOleStr;
   size_t cch = 0;

   if (!(SHGDN_INFOLDER & uFlags))
   {
      pwszCurrent[0] = L'\\';
      pwszCurrent[1] = L'\\';
      pwszCurrent += 2;
      cch = 2;
   }

   LPCWSTR pwszName = ((SHGDN_FORPARSING & uFlags) ? ILParsingName(pidl) : ILDisplayName(pidl));

   return StringCchCopy(pwszCurrent, MAX_PATH-cch, pwszName);
}

STDMETHODIMP CNetworkFolder::GetUIObjectOf(HWND hwndOwner,
                                           UINT cidl,
                                           LPCITEMIDLIST * apidl,
                                           REFIID riid,
                                           UINT * prgfInOut,
                                           LPVOID * ppvOut)
{
   return E_NOTIMPL;
}

STDMETHODIMP CNetworkFolder::ParseDisplayName(HWND hwndOwner,
                                              LPBC pbc,
                                              LPOLESTR pwszDisplayName,
                                              ULONG * pchEaten,
                                              LPITEMIDLIST * ppidl,
                                              ULONG * pdwAttributes)
{
   ASSERT(ppidl);
   if (!ppidl)
      return E_INVALIDARG;

   if ((L'\\' != pwszDisplayName[0]) || (L'\\' != pwszDisplayName[1]))
   {
      return E_FAIL;
   }

   HRESULT hr = NOERROR;
   LPITEMIDLIST pidl = NULL;
   LPITEMIDLIST pidlEnd = NULL;
   size_t cch = 0;
   LPWSTR pwsz = NULL;
   LPWSTR pwszBegin = NULL;
   LPWSTR pwszEnd = NULL;
   IShellFolder * pFolder = NULL;

   // Since our UNC navigation is simple, we just use the filesystem folder.
   // To do this we must first trick it into thinking it's drive letter is '\\foo'
   pwszBegin = pwszDisplayName+2; // walk over \\ part of the UNC
   hr = StringCchLength(pwszBegin, MAX_PATH, &cch);
   if (SUCCEEDED(hr))
   {
      pwszEnd = ::wcschr(pwszBegin, L'\\');
      if (pwszEnd)
         cch = pwszEnd-pwszBegin;
      ASSERT(cch);
      if (0 == cch)
         hr = HRESULT_FROM_WIN32_I(ERROR_BUFFER_OVERFLOW);
   }

   if (FAILED(hr))
      goto ParseDisplayName_done;

   pwsz = (LPWSTR) g_pShellMalloc->Alloc((cch+1)*sizeof(WCHAR));
   if (!pwsz)
   {
      hr = E_OUTOFMEMORY;
      goto ParseDisplayName_done;
   }

   memcpy(pwsz, pwszBegin, cch*sizeof(WCHAR));
   pwsz[cch] = L'\0';

   hr = CreateFileSystemPidl(pwsz, &pidl);
   if (FAILED(hr))
      goto ParseDisplayName_done;

   if (pwszEnd)
   {
      pwszEnd++; // Walk over the '\'

      CFileFolder * pFileFolder = new CFileFolder();
      if (!pFileFolder)
      {
         hr = E_OUTOFMEMORY;
         goto ParseDisplayName_done;
      }

      pFileFolder->TreatAsUNC(TRUE);
      pFolder = (IShellFolder *) pFileFolder;

      hr = pFileFolder->InitializeNoVerify(pidl);
      if (FAILED(hr))
         goto ParseDisplayName_done;

      hr = pFolder->ParseDisplayName(hwndOwner, pbc, pwszEnd,
                                     pchEaten, &pidlEnd, pdwAttributes);
      if (FAILED(hr))
         goto ParseDisplayName_done;

      *ppidl = ILConcatenate(pidl, pidlEnd);
      if (!*ppidl)
      {
         hr = E_OUTOFMEMORY;
         goto ParseDisplayName_done;
      }
   }
   else
   {
      *ppidl = ILCopy(pidl, IL_ALL);
      if (!*ppidl)
      {
         hr = E_OUTOFMEMORY;
         goto ParseDisplayName_done;
      }
   }

ParseDisplayName_done:

   // Handle network errors here before handing
   // the error off to the container
   if (FAILED(hr))
      ShowNetworkError(hr, pwszDisplayName);

   if (pidl)
      ILFree(pidl);

   if (pidlEnd)
      ILFree(pidlEnd);

   if (pwsz)
      g_pShellMalloc->Free(pwsz);

   if (pFolder)
      pFolder->Release();

   return hr;
}

STDMETHODIMP CNetworkFolder::SetNameOf(HWND hwndOwner,
                                       LPCITEMIDLIST pidl,
                                       LPCOLESTR lpszName,
                                       DWORD uFlags,
                                       LPITEMIDLIST * ppidlOut)
{
   return E_NOTIMPL;
}


//////////////////////////////////////////////////
// IPersistFolder

STDMETHODIMP CNetworkFolder::GetClassID(LPCLSID lpClassID)
{
   return E_NOTIMPL;
}

STDMETHODIMP CNetworkFolder::Initialize(LPCITEMIDLIST pidlFQ)
{
   return E_NOTIMPL;
}

