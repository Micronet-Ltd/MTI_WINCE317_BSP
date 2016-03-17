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

#include "fsenum.h"
#include "guid.h"
#include "idlist.h"
#include "recbin.h"
#include "usersettings.h"

//////////////////////////////////////////////////
// CFileSystemEnum

CFileSystemEnum::CFileSystemEnum(SHCONTF grfFlags) :
   m_ObjRefCount(0),
   m_pidlFQ(NULL),
   m_hfind(INVALID_HANDLE_VALUE),
   m_grfFlags(grfFlags),
   m_pszFilter(NULL),
   m_pszRecycleBin(NULL)
{
   ZeroMemory(&m_FindData, sizeof(m_FindData));

   m_ObjRefCount = 1;
}
 
CFileSystemEnum::~CFileSystemEnum()
{
   if (m_pidlFQ)
      ILFree(m_pidlFQ);

   if ((INVALID_HANDLE_VALUE != m_hfind) && (NULL != m_hfind))
      FindClose(m_hfind);

   if (m_pszFilter)
      g_pShellMalloc->Free(m_pszFilter);
}

BOOL CFileSystemEnum::Initialize(LPCITEMIDLIST pidlFQ)
{
   BOOL bRet = FALSE;
   WCHAR wszPath[MAX_PATH];
   HRESULT hr;

   ASSERT(pidlFQ);
   if (!pidlFQ)
      goto leave;

   if (m_pidlFQ)
      ILFree(m_pidlFQ);
   m_pidlFQ = ILCopy(pidlFQ, IL_ALL);

   if ((INVALID_HANDLE_VALUE != m_hfind) && (NULL != m_hfind))
   {
      FindClose(m_hfind);
      m_hfind = INVALID_HANDLE_VALUE;
   }

   if (!SHGetPathFromIDList(m_pidlFQ, wszPath))
      goto leave;

   if (UserSettings::GetUseRecycleBin())
   {
      g_pRecBin->BeginRecycle();
      m_pszRecycleBin = PathFindFileName(CRecycleBin::m_szBitBucketPath);
      if (m_pszRecycleBin)
      {
         size_t cch = m_pszRecycleBin-CRecycleBin::m_szBitBucketPath-1;

         if (((cch < lengthof(wszPath)) && (TEXT('\0') != wszPath[cch]) && (TEXT('\\') != wszPath[cch])) ||
            (0 != _tcsnicmp(wszPath, CRecycleBin::m_szBitBucketPath, cch)))
         {
            m_pszRecycleBin = NULL;
         }
      }
         
      g_pRecBin->EndRecycle();
   }

   if ((L'\\' == wszPath[0]) && (L'\0' == wszPath[1]))
   {
      // Note that StringCchCopy always null-terminates the buffer
      hr = ::StringCchCopy(wszPath, lengthof(wszPath), L"\\*.*");
   }
   else
   {
      PathRemoveTrailingSlashes(wszPath);
      // Note that StringCchCat always null-terminates the buffer
      hr = ::StringCchCat(wszPath, lengthof(wszPath), L"\\*.*");
   }
   ASSERT(SUCCEEDED(hr)); // Failure is acceptable

   m_hfind = FindFirstFile(wszPath, &m_FindData);

   // If there are no files in this directory set m_hfind as a ended search (see header info on m_hfind)
   if (INVALID_HANDLE_VALUE == m_hfind)
      m_hfind = NULL;

   bRet = TRUE;

leave:
   return bRet;
} 

BOOL CFileSystemEnum::MeetsFindCriteria()
{
   BOOL bRet = FALSE;

   ASSERT(INVALID_HANDLE_VALUE != m_hfind);
   ASSERT(NULL != m_hfind);

   if (m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
   {
      if (!(m_grfFlags & SHCONTF_FOLDERS))
         goto leave;
   }
   else if (!(m_grfFlags & SHCONTF_NONFOLDERS))
   {
      goto leave;
   }
   else if (m_pszFilter &&
      !PathMatchSpec(m_FindData.cFileName, m_pszFilter))
   {
      WCHAR wszPath[MAX_PATH];
      size_t cch;

      if (!PathIsLink(m_FindData.cFileName))
         goto leave;

      // Special check for shorcuts
      // a) Skip if ShortcutTarget is a folder
      // b) Match pattern against Target if it is a file

      if (!SHGetPathFromIDList(m_pidlFQ, wszPath))
         goto leave;

      if (FAILED(::StringCchLength(wszPath, lengthof(wszPath), &cch)) ||
         ((L'\\' != wszPath[cch-1]) &&
         FAILED(::StringCchCat(wszPath, lengthof(wszPath), L"\\"))) ||
         FAILED(StringCchCat(wszPath, lengthof(wszPath), m_FindData.cFileName)) ||
         !SHGetShortcutTarget(wszPath, wszPath, lengthof(wszPath)))
      {
         goto leave;
      }

      PathRemoveQuotesAndArgs(wszPath);

      if (!PathIsDirectory(wszPath) &&
         !PathMatchSpec((LPCTSTR)PathFindFileName(wszPath), m_pszFilter))
      {
         goto leave;
      }
   }

   if (!UserSettings::GetShowSystemFiles() &&
       ((m_FindData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) ||
        (m_FindData.dwFileAttributes & FILE_ATTRIBUTE_ROMMODULE)))
   {
      goto leave;
   }

   if (!(m_grfFlags & SHCONTF_INCLUDEHIDDEN) &&
       (m_FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
   {
      goto leave;
   }

   if (m_pszRecycleBin)
   {
      if (0 == _tcsicmp(m_pszRecycleBin, m_FindData.cFileName))
         goto leave;
   }

   bRet = TRUE;

leave:
   return bRet;
}


//////////////////////////////////////////////////
// IUnknown

STDMETHODIMP CFileSystemEnum::QueryInterface(REFIID riid,
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

STDMETHODIMP_(ULONG) CFileSystemEnum::AddRef(VOID)
{
   return ++m_ObjRefCount;
}

STDMETHODIMP_(ULONG) CFileSystemEnum::Release(VOID)
{
   if (--m_ObjRefCount == 0)
   {
      delete this;
      return 0;
   }
   
   return m_ObjRefCount;
}


//////////////////////////////////////////////////
// IEnumIDList 

STDMETHODIMP CFileSystemEnum::Clone(IEnumIDList ** ppenum)
{
   return E_NOTIMPL;
}

STDMETHODIMP CFileSystemEnum::Next(ULONG celt,
                                   LPITEMIDLIST * rgelt,
                                   ULONG * pceltFetched)
{
   HRESULT hr = NOERROR;

   ASSERT(rgelt);
   if (!rgelt)
      hr = E_INVALIDARG;

   if (INVALID_HANDLE_VALUE == m_hfind)
      hr = E_UNEXPECTED;

   if (NULL == m_hfind)
      hr = S_FALSE;

   if (NOERROR == hr)
   {
      while (!MeetsFindCriteria())
      {
         if (!FindNextFile(m_hfind, &m_FindData))
         {
            FindClose(m_hfind);
            m_hfind = NULL;
            hr = S_FALSE;
            break;
         }
      }

      if (NOERROR == hr)
      {
         hr = CreateFileSystemPidl(m_FindData.cFileName, rgelt);
         UpdateFileSystemPidl(m_pidlFQ, rgelt);

         if (pceltFetched)
            *pceltFetched = 1;

         if (!FindNextFile(m_hfind, &m_FindData))
         {
            FindClose(m_hfind);
            m_hfind = NULL;
         }
      }
   }

   if (NOERROR != hr)
   {
      if (pceltFetched)
         *pceltFetched = 0;
   }

   return hr;
}

STDMETHODIMP CFileSystemEnum::Reset()
{
   return E_NOTIMPL;
}

STDMETHODIMP CFileSystemEnum::Skip(ULONG celt)
{
   return E_NOTIMPL;
}


//////////////////////////////////////////////////
// IEnumFilter

STDMETHODIMP CFileSystemEnum::SetFilterString(LPCWSTR pszFilter)
{
   HRESULT hr;
   LPWSTR pszTemp = NULL;
   size_t cch = 0;

   if (!pszFilter)
   {
      hr = NOERROR;
      goto resetFilter;
   }

   hr = StringCchLength(pszFilter, MAX_PATH, &cch);
   if (FAILED(hr))
   {
      goto leave;
   }

   if (!m_pszFilter)
   {
      pszTemp = (LPWSTR) g_pShellMalloc->Alloc((cch+1)*sizeof(WCHAR));
   }
   else
   {
      pszTemp = (LPWSTR) g_pShellMalloc->Realloc(m_pszFilter, (cch+1)*sizeof(WCHAR));
   }

   if (!pszTemp)
   {
      hr = E_OUTOFMEMORY;
      goto leave;
   }

   hr = StringCchCopy(pszTemp, cch+1, pszFilter);
   m_pszFilter = pszTemp;

leave:
   if (FAILED(hr))
   {
resetFilter:
      if (m_pszFilter)
      {
         g_pShellMalloc->Free(m_pszFilter);
         m_pszFilter = NULL;
      }
   }

   return hr;
}

