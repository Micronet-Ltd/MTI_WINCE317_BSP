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

#include "recyclefolder.h"
#include "datatrans.h"
#include "filechangemgr.h"
#include "guid.h"
#include "idlist.h"
#include "recbin.h"
#include "recbindrop.h"
#include "recbinview.h"
#include "shelldialogs.h"

//////////////////////////////////////////////////
// CRecycleBinFolder

CRecycleBinFolder::CRecycleBinFolder() :
   m_ObjRefCount(0)
{
// g_DllRefCount++;

   m_ObjRefCount = 1;
}

CRecycleBinFolder::~CRecycleBinFolder()
{
// g_DllRefCount--;
}


//////////////////////////////////////////////////
// IUnknown

STDMETHODIMP CRecycleBinFolder::QueryInterface(REFIID riid,
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
   else if(IsEqualIID(riid, IID_IShellDetails)) // IShellDetails
   {
      *ppobj = (IShellDetails *) this;
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

STDMETHODIMP_(ULONG) CRecycleBinFolder::AddRef(VOID)
{
   return ++m_ObjRefCount;
}

STDMETHODIMP_(ULONG) CRecycleBinFolder::Release(VOID)
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

STDMETHODIMP CRecycleBinFolder::BindToObject(LPCITEMIDLIST pidl,
                                             LPBC pbc,
                                             REFIID riid,
                                             LPVOID * ppvOut)
{
   return E_NOTIMPL;
}

STDMETHODIMP CRecycleBinFolder::BindToStorage(LPCITEMIDLIST pidl,
                                              LPBC pbcReserved,
                                              REFIID riid,
                                              LPVOID * ppvObj)
{
   return E_NOTIMPL;
}

STDMETHODIMP CRecycleBinFolder::CompareIDs(LPARAM lParam,
                                           LPCITEMIDLIST pidl1,
                                           LPCITEMIDLIST pidl2)
{
   ASSERT(pidl1);
   ASSERT(pidl2);
   if (!pidl1 || !pidl2)
      return E_INVALIDARG;

   short severity = SEVERITY_SUCCESS;
   int result = 0;

   ASSERT(!ILIsNameSpacePidl(pidl1));
   ASSERT(!ILIsNameSpacePidl(pidl2));

   switch (lParam)
   {
      case 0: // NAME
      {
         STRRET str = {0};
         WCHAR wszPath1[MAX_PATH];
         WCHAR wszPath2[MAX_PATH];
         
         if (FAILED(GetDisplayNameOf(pidl1, SHGDN_INFOLDER | SHGDN_FORADDRESSBAR, &str)))
            goto CompareIDs_done;
         StrRetToBuf(&str, pidl1, wszPath1, lengthof(wszPath1));

         if (FAILED(GetDisplayNameOf(pidl2, SHGDN_INFOLDER | SHGDN_FORADDRESSBAR, &str)))
            goto CompareIDs_done;
         StrRetToBuf(&str, pidl2, wszPath2, lengthof(wszPath2));

         int cmp = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                 wszPath1, -1,
                                 wszPath2, -1);
         if (0 == cmp)
         {
            severity = SEVERITY_ERROR;
            goto CompareIDs_done;
         }
 
         result = (cmp - 2); // See docs on CompareString
      }
      break;
         
      case 1: // SIZE
      {
         g_pRecBin->BeginRecycle();
         
         ULONGLONG ullSize1 = 0, ullSize2 = 0;
         if (g_pRecBin->GetFileSize(pidl1, &ullSize1) && 
            g_pRecBin->GetFileSize(pidl2, &ullSize2))
         {
            if (ullSize1 < ullSize2)
               result = -1;
            else if (ullSize1 > ullSize2)
               result =  1;
            else
               result = 0;
         }
         else
         {
            severity = SEVERITY_ERROR;
            goto CompareIDs_done;
         }
         
         g_pRecBin->EndRecycle();
      }
      break;
         
      case 2: // ORIGINAL LOCATION
      {
         g_pRecBin->BeginRecycle();                        
         LPOLESTR str1 = g_pRecBin->GetFileOrigLoc(pidl1);
         if (str1)
         {
            LPOLESTR str2 = g_pRecBin->GetFileOrigLoc(pidl2);
            if (str2)
            {
               int cmp = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                       (LPCTSTR)str1, -1,
                                       (LPCTSTR)str2, -1);
               g_pShellMalloc->Free(str1);
               g_pShellMalloc->Free(str2);
               
               if (0 == cmp)
               {
                  severity = SEVERITY_ERROR;
                  goto CompareIDs_done;
               }
 
               result = (cmp - 2); // See docs on CompareString
            }
            else
               g_pShellMalloc->Free(str1);
         }
         else
         {
            severity = SEVERITY_ERROR;
            goto CompareIDs_done;
         }

         g_pRecBin->EndRecycle();
      }
      break;
      
      case 3: // DATE DELETED
      {
         FILETIME ft1, ft2;

         g_pRecBin->BeginRecycle();

         if (g_pRecBin->GetFileDateDel(pidl1, &ft1) &&
            g_pRecBin->GetFileDateDel(pidl2, &ft2))
         {
            result = CompareFileTime(&ft1, &ft2);
         }
         else
         {
            severity = SEVERITY_ERROR;
            goto CompareIDs_done;
         }

         g_pRecBin->EndRecycle();
      }
      break;
   }
   
   CompareIDs_done:

   return MAKE_HRESULT(severity, 0, (USHORT) result);
}

STDMETHODIMP CRecycleBinFolder::CreateViewObject(HWND hwndOwner, 
                                                 REFIID riid, 
                                                 LPVOID * ppvOut)
{  HRESULT hr = NOERROR;

   ASSERT(ppvOut);
   if (!ppvOut)
      return E_INVALIDARG;

   *ppvOut = NULL;

   if(IsEqualIID(riid, IID_IShellView))
   {
      AddRef(); // For the IShellFolder passed to CDefShellView

      LPITEMIDLIST pidl = NULL;
      hr = CreateNameSpacePidl(CLSID_CEShellBitBucket, &pidl);
      if (FAILED(hr))
         return hr;

      CDefShellView * pShellView = new CRecycleBinView(this, pidl);             
      ILFree(pidl);

      if (!pShellView)
      {
         Release();
         return E_OUTOFMEMORY;
      }

      // Hook up the file notifications
      UINT uWatchID = 0;
      IShellChangeNotify * pShellChangeNotify = NULL;
      if (SUCCEEDED(pShellView->QueryInterface(IID_IShellChangeNotify,
                                             (VOID**)&pShellChangeNotify)))
      {               
         uWatchID = g_pFileChangeManager->AddWatch((LPWSTR)CRecycleBin::m_szBitBucketPath, pShellChangeNotify);
         if (0 != uWatchID)
            pShellView->SetWatchID(uWatchID);

         pShellChangeNotify->Release();
      }
      
      hr = pShellView->QueryInterface(riid, ppvOut);

      if (FAILED(hr) && (0 != uWatchID))
         g_pFileChangeManager->RemoveWatch(uWatchID);

      pShellView->Release();
   }

   return hr;
}

STDMETHODIMP CRecycleBinFolder::EnumObjects(HWND hwndOwner,
                                            DWORD grfFlags,
                                            LPENUMIDLIST * ppenumIDList)
{
   if (!ppenumIDList)
      return E_INVALIDARG;
   *ppenumIDList = NULL;

   HRESULT hr = NOERROR;

   g_pRecBin->BeginRecycle();
   g_pRecBin->EndRecycle();
   
   // defshellview.cpp will release pRecBin
   CRecycleBin * pRecBin = new CRecycleBin(grfFlags);
   if (pRecBin)
   {
      pRecBin->Initialize();
      *ppenumIDList = (IEnumIDList *)pRecBin;
   }
   else
   {
      hr = E_OUTOFMEMORY;
   }

   return hr;
}

STDMETHODIMP CRecycleBinFolder::GetAttributesOf(UINT cidl,
                                                LPCITEMIDLIST * apidl,
                                                ULONG * rgfInOut)
{
   ASSERT(apidl);
   ASSERT(rgfInOut);
   if (!apidl || !rgfInOut)
      return E_INVALIDARG;

   HRESULT hr = NOERROR;
   STRRET str = {0};
   WCHAR wszPath[MAX_PATH];
   DWORD dwAttrib = (DWORD) -1;
   ULONG rgfTotal = (ULONG) -1;
   ULONG rgf = 0;

   for (UINT i = 0; i < cidl; i++)
   {
      hr = GetDisplayNameOf(apidl[i], SHGDN_NORMAL | SHGDN_FORPARSING, &str);
      if (FAILED(hr))
      {
         break;
      }
      StrRetToBuf(&str, apidl[i], wszPath, lengthof(wszPath));

      dwAttrib = GetFileAttributes(wszPath);
      if (-1 == dwAttrib)
      {
         hr = HRESULT_FROM_WIN32_I(::GetLastError());
         break;
      }

      rgf = 0;

      // Miscellaneous Flags
      if (SFGAO_FOLDER & *rgfInOut)
      {
         if (FILE_ATTRIBUTE_DIRECTORY & dwAttrib)
            rgf |= SFGAO_FOLDER;
      }

      rgfTotal &= rgf;
   }

   if (FAILED(hr) || (0 == cidl))
   {
      (*rgfInOut) = 0;
   }
   else
   {
      rgfTotal |= (SFGAO_CANDELETE | SFGAO_READONLY);
      (*rgfInOut) &= rgfTotal;
   }

   return hr;
}

STDMETHODIMP CRecycleBinFolder::GetDisplayNameOf(LPCITEMIDLIST pidl,
                                                 DWORD uFlags,
                                                 LPSTRRET lpName)
{  LPOLESTR str = NULL;
   HRESULT hr = NOERROR;
   size_t strLen = 0;

   ASSERT(pidl);
   ASSERT(lpName);

   if (!pidl || !lpName)
      return E_INVALIDARG;

   if (!(SHGDN_INFOLDER & uFlags))
   {
      TCHAR szRecName[MAX_PATH];
      if (SUCCEEDED(StringCchCopy(szRecName, lengthof(szRecName), ILDisplayName(pidl))))
      {
         strLen = _tcslen(CRecycleBin::m_szBitBucketPath) + _tcslen(szRecName) + 1; //+1 for \
         
         lpName->pOleStr = (LPOLESTR) g_pShellMalloc->Alloc((strLen + 1) * sizeof(WCHAR));         
         if (lpName->pOleStr)
         {            
            hr = StringCchCopy(lpName->pOleStr, strLen + 1, CRecycleBin::m_szBitBucketPath);
            if (SUCCEEDED(hr))
            {
               hr = StringCchCat(lpName->pOleStr, strLen + 1, TEXT("\\"));
               if (SUCCEEDED(hr))
               {
                  hr = StringCchCat(lpName->pOleStr, strLen + 1, szRecName);
                  if (SUCCEEDED(hr))
                     lpName->uType = STRRET_WSTR;
               }
            }
         }
         else
         {
            hr = E_OUTOFMEMORY;
         }
      }
      else
      {
         hr = E_FAIL;
      }
   }
   else
   {
      g_pRecBin->BeginRecycle();

      str = g_pRecBin->GetFileOrigName(pidl);
      if (str)
      {
         strLen = wcslen((LPWSTR)str);
         lpName->pOleStr = (LPOLESTR) g_pShellMalloc->Alloc((strLen + 1) * sizeof(WCHAR));
         if (lpName->pOleStr)
         {
            hr = StringCchCopy(lpName->pOleStr, strLen + 1, str);
            lpName->uType = STRRET_WSTR;
         }
         else
         {
            hr = E_OUTOFMEMORY;
         }

         g_pShellMalloc->Free(str);
      }
      else
      {
         hr = E_FAIL;
      }
      
      g_pRecBin->EndRecycle();
   }
      
   return hr;
}

STDMETHODIMP CRecycleBinFolder::GetUIObjectOf(HWND hwndOwner,
                                              UINT cidl,
                                              LPCITEMIDLIST * apidl,
                                              REFIID riid,
                                              UINT * prgfInOut,
                                              LPVOID * ppvOut)
{
   ASSERT(apidl);
   ASSERT(ppvOut);
   if (!apidl || !ppvOut)
      return E_INVALIDARG;
   *ppvOut = NULL;

   HRESULT hr = E_NOTIMPL;

   if (IsEqualIID(riid, IID_IDropTarget)) // IDropTarget
   {  
      ASSERT(1 == cidl);
      ASSERT(1 == ILCount(*apidl));

      CRecBinDropTarget * pDropTarget = new CRecBinDropTarget();
      if (pDropTarget)
      {         
         if (pDropTarget->Initialize())
         {
            hr = pDropTarget->QueryInterface(riid, ppvOut);
            pDropTarget->Release();
         }
         else
         {
            hr = E_FAIL;
         }

         if (FAILED(hr))
         {
            pDropTarget->Release();
            *ppvOut = NULL;
         }
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
   }

   return hr;
}

STDMETHODIMP CRecycleBinFolder::ParseDisplayName(HWND hwndOwner,
                                                 LPBC pbc,
                                                 LPOLESTR pwszDisplayName,
                                                 ULONG * pchEaten,
                                                 LPITEMIDLIST * ppidl,
                                                 ULONG * pdwAttributes)
{
   return E_NOTIMPL;
}

STDMETHODIMP CRecycleBinFolder::SetNameOf(HWND hwndOwner,
                                          LPCITEMIDLIST pidl,
                                          LPCOLESTR lpszName,
                                          DWORD uFlags,
                                          LPITEMIDLIST * ppidlOut)
{
   return E_NOTIMPL;
}

//////////////////////////////////////////////////
// IShellDetails
STDMETHODIMP CRecycleBinFolder::ColumnClick(UINT iColumn)
{
   return S_FALSE;
}

STDMETHODIMP CRecycleBinFolder::GetDetailsOf(LPCITEMIDLIST pidl,
                                             UINT iColumn,
                                             LPSHELLDETAILS pDetails)
{
   LPOLESTR str = NULL;
   HRESULT hr = NOERROR;

   ASSERT(pidl);
   ASSERT(pDetails);
   if (!pidl || !pDetails)
      return E_INVALIDARG;

   ASSERT(1 == ILCount(pidl));
 
   g_pRecBin->BeginRecycle();
   
   switch (iColumn)
   {
      // Name
      case 0:
         str = g_pRecBin->GetFileOrigName(pidl);
         if (str)
         {
            size_t strLen = wcslen((LPWSTR)str);

            pDetails->str.pOleStr = (LPOLESTR) g_pShellMalloc->Alloc((strLen + 1) * sizeof(WCHAR));
            if (pDetails->str.pOleStr)
            {
               hr = StringCchCopyW(pDetails->str.pOleStr, strLen + 1, str);
               ASSERT(SUCCEEDED(hr));

               pDetails->str.uType = STRRET_WSTR;
               pDetails->fmt = LVCFMT_LEFT;
            }
            else
            {
               hr = E_OUTOFMEMORY;
            }

            g_pShellMalloc->Free(str);
         }
       
      break;

      // Size
      case 1:
      {  
         ULONGLONG ullSize = 0;         
         if (g_pRecBin->GetFileSize(pidl, &ullSize))
         {
            pDetails->str.pOleStr = (LPOLESTR) g_pShellMalloc->Alloc(MAX_PATH * sizeof(WCHAR));
            if ((pDetails->str.pOleStr))
            {
               pDetails->fmt = LVCFMT_RIGHT;
               //TODO:
               _ultow((unsigned long)ullSize, pDetails->str.pOleStr, 10);
            }
            else
            {
               hr = E_OUTOFMEMORY;
            }
         }
         else
         {
            hr = E_FAIL;
         }
      }
      break;

      // Original Location
      case 2:
      {
         str = g_pRecBin->GetFileOrigLoc(pidl);
         if (str)
         {
            size_t strLen = wcslen((LPWSTR)str);

            pDetails->str.pOleStr = (LPOLESTR) g_pShellMalloc->Alloc((strLen + 1) * sizeof(WCHAR));
            if ((pDetails->str.pOleStr))
            {
               hr = StringCchCopyW(pDetails->str.pOleStr, strLen + 1, str);
               ASSERT(SUCCEEDED(hr));

               pDetails->str.uType = STRRET_WSTR;
               pDetails->fmt = LVCFMT_LEFT;
            }
            else
            {
               hr = E_OUTOFMEMORY;
            }

            g_pShellMalloc->Free(str);
         }
      }
      break;

      // Date Deleted
      case 3:
      {
         size_t cch = 64;
         FILETIME ftDateDel = {0};

         if (g_pRecBin->GetFileDateDel(pidl, &ftDateDel))
         {  
            DWORD dwFlags = FDTF_DEFAULT;

            switch (pDetails->fmt)
            {
               case LVCFMT_LEFT_TO_RIGHT:
                  dwFlags |= FDTF_LTRDATE;
                  break;

               case LVCFMT_RIGHT_TO_LEFT:
                  dwFlags |= FDTF_RTLDATE;
                  break;
            }

            pDetails->str.pOleStr = (LPOLESTR) g_pShellMalloc->Alloc(cch * sizeof(WCHAR));
            if (pDetails->str.pOleStr)
            {
               pDetails->str.uType = STRRET_WSTR;
               pDetails->fmt = LVCFMT_LEFT;
               ShellFormat::FormatDateTime(&ftDateDel, dwFlags, pDetails->str.pOleStr, cch);
            }
            else
            {
               hr = E_OUTOFMEMORY;
            }

            g_pShellMalloc->Free(str);
         }
      }
      break;

      default:
         hr = E_FAIL;
   }

   g_pRecBin->EndRecycle();

   return hr;
}

//////////////////////////////////////////////////
// IPersistFolder

STDMETHODIMP CRecycleBinFolder::GetClassID(LPCLSID lpClassID)
{
   return E_NOTIMPL;
}

STDMETHODIMP CRecycleBinFolder::Initialize(LPCITEMIDLIST pidlFQ)
{
   return E_NOTIMPL;
}

