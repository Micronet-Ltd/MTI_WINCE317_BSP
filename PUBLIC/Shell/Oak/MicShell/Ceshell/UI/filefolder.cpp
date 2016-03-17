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

#include "filefolder.h"
#include "datatrans.h"
#include "defshellview.h"
#undef SHCNF_PATH // extfile.h redefines this
#include <extfile.h>
#include "filechangemgr.h"
#include "filemenu.h"
#include "fsdrop.h"
#include "fsenum.h"
#include "idlist.h"
#include "netrootview.h"
#include "resource.h"
#include "shelldialogs.h"
#include "usersettings.h"

//////////////////////////////////////////////////
// CFileFolder

CFileFolder::CFileFolder() :
   m_pidl(NULL),
   m_pidlRoot(NULL),
   m_cchPath(0),
   m_fTreatAsUNC(FALSE)
{
// g_DllRefCount++;

   HRESULT hr = S_OK;
   hr = SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &m_pidlRoot);
   ASSERT(SUCCEEDED(hr));

   m_ObjRefCount = 1;
}

CFileFolder::~CFileFolder()
{
   if (m_pidl)
      ILFree(m_pidl);

   if (m_pidlRoot)
      ILFree(m_pidlRoot);

// g_DllRefCount--;
}

LPOLESTR CFileFolder::GetPath(size_t * pcch) const
{
   WCHAR wszPath[MAX_PATH];
   LPWSTR pwszPath = wszPath;
   int cch = lengthof(wszPath);

   if (m_pidl)
   {
      // If it's a UNC add a leading \ to get \\ at the beginning
      if (m_fTreatAsUNC)
      {
        *pwszPath = L'\\';
         pwszPath++;
      }

      LPITEMIDLIST pidlCurrent = m_pidl;
      LPITEMIDLIST pidl = NULL;
      while (pidlCurrent)
      {
         pidl = ILCopy(pidlCurrent, 1);
         if (!pidl)
            return FALSE;

         *pwszPath = L'\\';
         pwszPath++;

         cch = (lengthof(wszPath)-(pwszPath-wszPath)); // The number of characters left in wszPath
         ASSERT(0 < cch);
         if ((0 >= cch) || FAILED(StringCchCopy(pwszPath, cch, ILParsingName(pidl))))
         {
            ILFree(pidl);
            return FALSE;
         }

         pwszPath = pwszPath + ::wcslen(pwszPath);

         ILFree(pidl);
         pidlCurrent = ILNext(pidlCurrent);
      }
   }

   // NULL terminate the path string
   *pwszPath = L'\0';

   // Create the string to return
   cch = pwszPath-wszPath; // Total characters in the string (excluding null)
   pwszPath = (LPOLESTR) g_pShellMalloc->Alloc((cch+1)*sizeof(WCHAR));
   if (pwszPath)
   {
      memcpy(pwszPath, wszPath, (cch+1)*sizeof(WCHAR));
      if (pcch)
         *pcch = cch;
   }
   else
   {
      if (pcch)
         *pcch = 0;
   }

   return ((LPOLESTR) pwszPath);
}

HRESULT CFileFolder::InitializeNoVerify(LPCITEMIDLIST pidl)
{
   ASSERT(pidl);
   if (!pidl)
      return E_INVALIDARG;

   HRESULT hr = NOERROR;

   if (m_pidl)
      ILFree(m_pidl);

   m_pidl = ILCopy(pidl, IL_ALL);
   if (!m_pidl)
      hr = E_OUTOFMEMORY;

   if (FAILED(hr))
   {
      if (m_pidl)
      {
         ILFree(m_pidl);
         m_pidl = NULL;
      }
   }

   return hr;
}

HRESULT CFileFolder::TreatAsUNC(BOOL fTreatAsUNC)
{
   HRESULT hr = NOERROR;

   if (m_fTreatAsUNC != fTreatAsUNC)
   {
      if (m_pidlRoot)
      {
         ILFree(m_pidlRoot);
         m_pidlRoot = NULL;
      }

      if (fTreatAsUNC)
         hr = SHGetSpecialFolderLocation(NULL, CSIDL_NETWORK, &m_pidlRoot);
      else
         hr = SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &m_pidlRoot);

      ASSERT(SUCCEEDED(hr));
      if (SUCCEEDED(hr))
         m_fTreatAsUNC = fTreatAsUNC;
   }

   return hr;
}


//////////////////////////////////////////////////
// IUnknown

STDMETHODIMP CFileFolder::QueryInterface(REFIID riid,
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
   else if (IsEqualIID(riid, IID_IShellFolder)) // IShellFolder
   {
      *ppobj = (IShellFolder *) this;
   }
   else if (IsEqualIID(riid, IID_IShellDetails)) // IShellDetails
   {
      *ppobj = (IShellDetails *) this;
   }
   else if (IsEqualIID(riid, IID_IPersist)) // IPersist
   {
      *ppobj = (IPersistFolder *) this;
   }
   else if (IsEqualIID(riid, IID_IPersistFolder)) // IPersistFolder
   {
      *ppobj = (IPersistFolder *) this;
   }

   if (*ppobj)
   {
      (*(LPUNKNOWN*)ppobj)->AddRef();
      hr = S_OK;
   }

leave:
   return hr;
}

STDMETHODIMP_(ULONG) CFileFolder::AddRef(VOID)
{
   return ++m_ObjRefCount;
}

STDMETHODIMP_(ULONG) CFileFolder::Release(VOID)
{
   if (0 == --m_ObjRefCount)
   {
      delete this;
      return 0;
   }
   
   return m_ObjRefCount;
}


//////////////////////////////////////////////////
// IShellFolder 

STDMETHODIMP CFileFolder::BindToObject(LPCITEMIDLIST pidl,
                                       LPBC pbc,
                                       REFIID riid,
                                       LPVOID * ppvOut)
{
   HRESULT hr = NOERROR;

   if (!pidl || !ppvOut)
   {
      ASSERT(0);
      hr = E_INVALIDARG;
      goto leave;
   }

   *ppvOut = NULL;

   CFileFolder * pFolder = new CFileFolder();
   if (pFolder)
   {
      LPITEMIDLIST pidlFQ = NULL;

      // Propagate the UNC settings
      pFolder->TreatAsUNC(m_fTreatAsUNC);

      if (m_pidl)
         pidlFQ = ILConcatenate(m_pidl, pidl);
      else
         pidlFQ = ILCopy(pidl, IL_ALL);

      if (pidlFQ)
      {
         hr = pFolder->Initialize(pidlFQ);
         if (SUCCEEDED(hr))
            hr = pFolder->QueryInterface(riid, ppvOut);

         ILFree(pidlFQ);
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }

      pFolder->Release();
   }
   else
   {
      hr = E_OUTOFMEMORY;
   }

leave:
   return hr;
}

STDMETHODIMP CFileFolder::BindToStorage(LPCITEMIDLIST pidl,
                                        LPBC pbcReserved,
                                        REFIID riid,
                                        LPVOID * ppvObj)
{
   return E_NOTIMPL;
}

STDMETHODIMP CFileFolder::CompareIDs(LPARAM lParam,
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
  
   if (ILIsFileSystemDirectoryPidl(pidl1) &&
       !ILIsFileSystemDirectoryPidl(pidl2))
   {
      result = -1;      
   }
   else if (!ILIsFileSystemDirectoryPidl(pidl1) &&
            ILIsFileSystemDirectoryPidl(pidl2))
   {
      result = 1;
   }
   else if(ILIsFileSystemDirectoryPidl(pidl1) &&
            ILIsFileSystemDirectoryPidl(pidl2) && (lParam == 1 || lParam == 2))
   {
      result = 0;
   }
   else
   {
      switch (lParam)
      {
         case 0: // NAME
         {
            int cmp = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                    ILDisplayName(pidl1), -1,
                                    ILDisplayName(pidl2), -1);
            if (0 == cmp)
            {
               severity = SEVERITY_ERROR;
               goto CompareIDs_done;
            }
 
            result = (cmp - 2); // See docs on CompareString
            goto CompareIDs_done;
         }
         break;
         
         case 1: // SIZE
         {
            ULARGE_INTEGER uliSize1, uliSize2;

            if (!ILGetSizeCached(pidl1, &uliSize1) ||
                !ILGetSizeCached(pidl2, &uliSize2))
            {
               severity = SEVERITY_ERROR;
               goto CompareIDs_done;
            }

            if (uliSize1.QuadPart < uliSize2.QuadPart)
               result = -1;
            else if (uliSize1.QuadPart > uliSize2.QuadPart)
               result =  1;
            else
               result = 0;
         }
         break;
         
         case 2: // TYPE
         {
            if (!ILHasTypeCached(pidl1) || !ILHasTypeCached(pidl2))
            {
               severity = SEVERITY_ERROR;
               goto CompareIDs_done;
            }
               
            int cmp = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                    ILGetTypeCached(pidl1), -1,
                                    ILGetTypeCached(pidl2), -1);
            if (0 == cmp)
            {
               severity = SEVERITY_ERROR;
               goto CompareIDs_done;
            }
 
            result = (cmp - 2); // See docs on CompareString
         }
         break;
      
         case 3: // DATE
         {
            FILETIME ft1, ft2;

            if (!ILGetDateCached(pidl1, &ft1) || !ILGetDateCached(pidl2, &ft2))
            {
               severity = SEVERITY_ERROR;
               goto CompareIDs_done;
            }

            result = CompareFileTime(&ft1, &ft2);
         }
         break;
      }

      ASSERT(SEVERITY_SUCCESS == severity);
      if (0 == result)
      {
         // The size, type, or date is same sort by name within the grouping
         int cmp = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                 ILDisplayName(pidl1), -1,
                                 ILDisplayName(pidl2), -1);
         if (0 == cmp)
         {
            severity = SEVERITY_ERROR;
            goto CompareIDs_done;
         }

         result = (cmp - 2); // See docs on CompareString
      }
   }

CompareIDs_done:

   return MAKE_HRESULT(severity, 0, (USHORT) result);
}

STDMETHODIMP CFileFolder::CreateViewObject(HWND hwndOwner, 
                                           REFIID riid, 
                                           LPVOID * ppvOut)
{
   ASSERT(ppvOut);
   if (!ppvOut)
      return E_INVALIDARG;
   *ppvOut = NULL;

   HRESULT hr = E_NOTIMPL;

   if (IsEqualIID(riid, IID_IShellView))
   {
      LPITEMIDLIST pidlFQ = NULL;
      LPWSTR pwszPath = GetPath(NULL);

      if (!pwszPath)
         return E_OUTOFMEMORY;

      if (m_pidl)
         pidlFQ = ILConcatenate(m_pidlRoot, m_pidl);
      else
         pidlFQ = ILCopy(m_pidlRoot, IL_ALL);

      if (pidlFQ)
      {
         CEOIDINFO info = {0};
         CDefShellView * pShellView = NULL;

         AddRef(); // For the IShellFolder passed to CDefShellView

         if (m_fTreatAsUNC && (1 == ILCount(m_pidl)))
         {
            // Special case for the root of network folder (\\foo)
            pShellView = (CDefShellView *) new CNetworkRootView(this, pidlFQ);
         }
         else if (::CeOidGetInfo(OIDFROMAFS(AFS_ROOTNUM_NETWORK), &info) &&
                  (CSTR_EQUAL == ::CompareString(LOCALE_USER_DEFAULT,
                                                 NORM_IGNORECASE,
                                                 info.infDirectory.szDirName, -1,
                                                 pwszPath, -1)))
         {
            // Special case for network folder (\network)
            pShellView = (CDefShellView *) new CNetworkRootView(this, pidlFQ);
         }
         else
         {
            // Regular default shell view
            pShellView = (CDefShellView *) new CDefShellView(this, pidlFQ);
         }
         
         if (pShellView)
         {
            // Hook up the file notifications
            UINT uWatchID = 0;
            IShellChangeNotify * pShellChangeNotify = NULL;
            if (SUCCEEDED(pShellView->QueryInterface(IID_IShellChangeNotify,
                                                        (VOID**)&pShellChangeNotify)))
            {
               uWatchID = g_pFileChangeManager->AddWatch(pwszPath,
                                                         pShellChangeNotify);
               if (0 != uWatchID)
                  pShellView->SetWatchID(uWatchID);

               pShellChangeNotify->Release();
            }

            hr = pShellView->QueryInterface(riid, ppvOut);

            if (FAILED(hr) && (0 != uWatchID))
               g_pFileChangeManager->RemoveWatch(uWatchID);

            pShellView->Release();
         }
         else
         {
            hr = E_OUTOFMEMORY;
         }

         if (FAILED(hr))
            Release();

         ILFree(pidlFQ);
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }

      g_pShellMalloc->Free(pwszPath);
   }

   return hr;
}

STDMETHODIMP CFileFolder::EnumObjects(HWND hwndOwner,
                                      DWORD grfFlags,
                                      LPENUMIDLIST * ppenumIDList)
{
   if (!ppenumIDList)
      return E_INVALIDARG;
   *ppenumIDList = NULL;

   HRESULT hr = NOERROR;
   LPITEMIDLIST pidlFQ = NULL;
   if (m_pidl)
      pidlFQ = ILConcatenate(m_pidlRoot, m_pidl);
   else
      pidlFQ = ILCopy(m_pidlRoot, IL_ALL);

   if (pidlFQ)
   {   
      // Build the enumerator
      CFileSystemEnum * pEnum = new CFileSystemEnum(grfFlags);
      if (pEnum)
      {
         if (pEnum->Initialize(pidlFQ))
         {
            *ppenumIDList = (IEnumIDList *) pEnum;
         }
         else
         {
            pEnum->Release();
            hr = E_FAIL;
         }
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }

      ILFree(pidlFQ);
   }
   else
   {
      hr = E_OUTOFMEMORY;
   }

   return hr;
}

STDMETHODIMP CFileFolder::GetAttributesOf(UINT cidl,
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

      // Capability Flags
      if (SFGAO_CANCOPY & *rgfInOut)
      {
         rgf |= SFGAO_CANCOPY;
      }
      if (SFGAO_CANDELETE & *rgfInOut)
      {
         if (!(FILE_ATTRIBUTE_INROM & dwAttrib))
            rgf |= SFGAO_CANDELETE;
      }
      if (SFGAO_CANLINK & *rgfInOut)
      {
         rgf |= SFGAO_CANLINK;
      }
      if (SFGAO_CANMOVE & *rgfInOut)
      {
         if (!(FILE_ATTRIBUTE_INROM & dwAttrib))
            rgf |= SFGAO_CANMOVE;
      }
      if (SFGAO_CANRENAME & *rgfInOut)
      {
         if (!(FILE_ATTRIBUTE_INROM & dwAttrib))
            rgf |= SFGAO_CANRENAME;
      }

      // Display Flags
      if (SFGAO_LINK & *rgfInOut)
      {
         if (PathIsLink(wszPath))
            rgf |= SFGAO_LINK;
      }
      if (SFGAO_READONLY & *rgfInOut)
      {
         if ((FILE_ATTRIBUTE_INROM | FILE_ATTRIBUTE_READONLY) & dwAttrib)
            rgf |= SFGAO_READONLY;
      }

      // Contents Flags
      if (SFGAO_HASSUBFOLDER & *rgfInOut)
      {
         WCHAR * pwszFFF = NULL;

         if ((FILE_ATTRIBUTE_DIRECTORY & dwAttrib) &&
             (SUCCEEDED(StringCchCatExW(wszPath, lengthof(wszPath),
                                        L"\\*.*", &pwszFFF, NULL, 0))))
         {
            WIN32_FIND_DATA fd = {0};
            HANDLE hFind = ::FindFirstFile(wszPath, &fd);

            if (INVALID_HANDLE_VALUE != hFind)
            {
               do
               {
                  ASSERT(-1 != fd.dwFileAttributes);
                  if (FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes)
                  {
                     rgf |= SFGAO_HASSUBFOLDER;
                     break;
                  }
               } while (::FindNextFile(hFind, &fd));

               ::FindClose(hFind);
            }

            pwszFFF -= 4; // Walk just before the "\*.*"
            ASSERT(L'\\' == *pwszFFF);
            *pwszFFF = L'\0';
         }
      }

      // Miscellaneous Flags
      if (SFGAO_FILESYSTEM & *rgfInOut)
      {
         rgf |= SFGAO_FILESYSTEM;
      }
      if (SFGAO_FILESYSANCESTOR & *rgfInOut)
      {
         rgf |= SFGAO_FILESYSANCESTOR;
      }
      if (SFGAO_FOLDER & *rgfInOut)
      {
         if (FILE_ATTRIBUTE_DIRECTORY & dwAttrib)
            rgf |= SFGAO_FOLDER;
      }
      if (SFGAO_REMOVABLE & *rgfInOut)
      {
         if (PathIsRemovableDevice(wszPath))
            rgf |= SFGAO_REMOVABLE;
      }

      rgfTotal &= rgf;
   }

   if (FAILED(hr) || (0 == cidl))
      (*rgfInOut) = 0;
   else
      (*rgfInOut) &= rgfTotal;

   return hr;
}

STDMETHODIMP CFileFolder::GetDisplayNameOf(LPCITEMIDLIST pidl,
                                           DWORD uFlags,
                                           LPSTRRET lpName)
{
   ASSERT(pidl);
   ASSERT(lpName);

   if (!pidl || !lpName)
      return E_INVALIDARG;

   HRESULT hr = NOERROR;
   size_t cchRemaining = MAX_PATH;
   LPWSTR pwszCurrent = NULL;
   LPITEMIDLIST pidlCurrent = (LPITEMIDLIST) pidl;
   LPITEMIDLIST pidlNext = NULL;
   lpName->uType = STRRET_WSTR;

   lpName->pOleStr = (LPOLESTR) g_pShellMalloc->Alloc(cchRemaining*sizeof(WCHAR));
   if (!lpName->pOleStr)
      return E_OUTOFMEMORY;
   pwszCurrent = lpName->pOleStr;

   if (!(SHGDN_INFOLDER & uFlags))
   {
      size_t cch = 0;
      LPOLESTR pwszFolder = GetPath(&cch);
      if (pwszFolder)
      {
         memcpy(pwszCurrent, pwszFolder, cch*sizeof(WCHAR));
         g_pShellMalloc->Free(pwszFolder);

         if (2 < (cchRemaining-cch))
         {
            pwszCurrent += cch;
            *pwszCurrent = L'\\';
            pwszCurrent++;
            cchRemaining -= (pwszCurrent-lpName->pOleStr);
         }
         else
         {
            hr = HRESULT_FROM_WIN32_I(ERROR_BUFFER_OVERFLOW);
         }
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
   }

   if (FAILED(hr))
      pidlCurrent = NULL;

   while (pidlCurrent)
   {
      if (SHGDN_FORPARSING & uFlags)
      {
         hr = StringCchCopyEx(pwszCurrent, cchRemaining,
                              ILParsingName(pidlCurrent),
                              &pwszCurrent, &cchRemaining, 0);
      }
      else
      {
         hr = StringCchCopyEx(pwszCurrent, cchRemaining,
                              ILDisplayName(pidlCurrent),
                              &pwszCurrent, &cchRemaining, 0);
      }

      if (FAILED(hr))
         break;

      pidlNext = ILNext(pidlCurrent);
      if (pidlNext)
      {
         ASSERT(ILIsFileSystemDirectoryPidl(pidlCurrent));

         if (2 < cchRemaining)
         {
            *pwszCurrent = L'\\';
            pwszCurrent++;
            cchRemaining--;
         }
         else
         {
            hr = HRESULT_FROM_WIN32_I(ERROR_BUFFER_OVERFLOW);
            pidlNext = NULL; // Don't append anything else since there is no more room
         }
      }
      else
      {
         if (!(SHGDN_FORPARSING & uFlags) && // The string is for display
             !ILIsFileSystemDirectoryPidl(pidlCurrent) && // pidlCurrent is a file
             !UserSettings::GetShowExtensions()) // The user does not want to show extensions
         {
            LPTSTR pszExtension = PathFindExtension(lpName->pOleStr);
            *pszExtension = TEXT('\0');
         }
      }

      pidlCurrent = pidlNext;
   }

   if (FAILED(hr))
   {
      g_pShellMalloc->Free(lpName->pOleStr);
      lpName->pOleStr = NULL;
   }

   return hr;
}

STDMETHODIMP CFileFolder::GetUIObjectOf(HWND hwndOwner,
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

   if(IsEqualIID(riid, IID_IDataObject)) // IDataObject
   {
      CShellDataObject * pDataObject = new CShellDataObject();
      if (pDataObject)
      {
         LPITEMIDLIST pidlFQ = NULL;
         FORMATETC fe = {0};
         STGMEDIUM sm = {0};
         UINT uFormat = ::RegisterClipboardFormat(CESHELL_DATA_TRANSFER);

         if (m_pidl)
            pidlFQ = ILConcatenate(m_pidlRoot, m_pidl);
         else
            pidlFQ = ILCopy(m_pidlRoot, IL_ALL);

         if (pidlFQ)
         {
            ShellDataTransfer sdt;

            for (UINT i = 0; i < cidl; i++)
            {
               if (ILIsNameSpacePidl(apidl[i]))
               {
                  sdt.m_dwFlags |= ShellDataTransfer::CONTAINS_NAMESPACE_OBJECT;
                  break;
               }
            }

            sdt.m_hwndOwnerLV = hwndOwner;
            sdt.m_pidlFolder = pidlFQ;
            sdt.m_uItems = cidl;
            sdt.m_ppidlItems = apidl;

            sm.hGlobal = ShellDataTransfer::Pack(&sdt);
            if (sm.hGlobal)
            {
               fe.cfFormat = (CLIPFORMAT) uFormat;
               fe.ptd = NULL;
               fe.dwAspect = DVASPECT_CONTENT;
               fe.lindex = -1;
               fe.tymed = sm.tymed = TYMED_HGLOBAL;

               hr = pDataObject->SetData(&fe, &sm, TRUE);
            }
            ILFree(pidlFQ);
         }
         else
         {
            hr = E_OUTOFMEMORY;
         }

         if (SUCCEEDED(hr))
         {
            hr = pDataObject->QueryInterface(riid, ppvOut);
            pDataObject->Release();
         }
         else
         {
            delete pDataObject;
            *ppvOut = NULL;
         }
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
   }
   else if (IsEqualIID(riid, IID_IDropTarget)) // IDropTarget
   {
      ASSERT(1 == cidl);
      ASSERT(1 == ILCount(*apidl));

      CFileSystemDropTarget * pDropTarget = new CFileSystemDropTarget();
      if (pDropTarget)
      {
         STRRET StrRet = {0};
         hr = GetDisplayNameOf(*apidl, SHGDN_NORMAL | SHGDN_FORPARSING, &StrRet);
         if (SUCCEEDED(hr))
         {
            WCHAR wszPath[MAX_PATH];
            hr = StrRetToBuf(&StrRet, *apidl, wszPath, lengthof(wszPath));
            if (SUCCEEDED(hr))
            {
               if (pDropTarget->Initialize(wszPath))
               {
                  hr = pDropTarget->QueryInterface(riid, ppvOut);
                  pDropTarget->Release();
               }
               else
               {
                  hr = E_FAIL;
               }
            }
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
   else if (IsEqualIID(riid, IID_IContextMenu)) // IContextMenu
   {
      CFileMenu * pFileMenu = new CFileMenu();
      if (pFileMenu)
      {
         if (pFileMenu->Initialize(this, cidl, apidl))
            hr = pFileMenu->QueryInterface(riid, ppvOut);
         else
            hr = E_FAIL;

         pFileMenu->Release();
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
   }

   return hr;
}

STDMETHODIMP CFileFolder::ParseDisplayName(HWND hwndOwner,
                                           LPBC pbc,
                                           LPOLESTR pwszDisplayName,
                                           ULONG * pchEaten,
                                           LPITEMIDLIST * ppidl,
                                           ULONG * pdwAttributes)
{
   ASSERT(pwszDisplayName);
   ASSERT(ppidl);
   if (!pwszDisplayName || !ppidl)
      return E_INVALIDARG;

   HRESULT hr = NOERROR;
   WCHAR wszPath[MAX_PATH];
   size_t cch = 0;
   LPWSTR pwszPath = GetPath(&cch);
   *ppidl = NULL;

   if (!pwszPath)
      return E_OUTOFMEMORY;

   hr = StringCchPrintf(wszPath, lengthof(wszPath), TEXT("%s\\%s"),
                        pwszPath, pwszDisplayName);
   g_pShellMalloc->Free(pwszPath);

   if (SUCCEEDED(hr))
   {
      if (PathFileExists(wszPath))
      {
         LPITEMIDLIST pidl = NULL;
         LPITEMIDLIST pidlFQ = NULL;
         LPITEMIDLIST pidlCat = NULL;
         LPWSTR pszPathSegB;
         LPWSTR pszPathSegE;

         if (m_pidl)
         {
            pidlFQ = ILConcatenate(m_pidlRoot, m_pidl);
         }
         else
         {
            pidlFQ = ILCopy(m_pidlRoot, IL_ALL);
         }

         pszPathSegB = wszPath + cch;
         if (L'\\' == *pszPathSegB)
         {
             pszPathSegB++;
             ASSERT(L'\\' != *pszPathSegB); // There can't be \\ in the middle of a path
         }

         pszPathSegE = wcschr(pszPathSegB, L'\\');
         while (pszPathSegE)
         {
            *pszPathSegE = L'\0';

            CreateFileSystemPidl(pszPathSegB, &pidl);

            // This must be a directory, cache anything in the desktop.ini
            UpdateFileSystemPidl(pidlFQ, &pidl);

            if (!*ppidl)
            {
               *ppidl = pidl;
 
               pidlCat = ILConcatenate(pidlFQ, pidl);
               ASSERT(pidlCat);
               ILFree(pidlFQ);
               pidlFQ = pidlCat;
            }
            else
            {
               pidlCat = ILConcatenate(*ppidl, pidl);
               ASSERT(pidlCat);
               ILFree(*ppidl);
               *ppidl = pidlCat;
 
               pidlCat = ILConcatenate(pidlFQ, pidl);
               ASSERT(pidlCat);
               ILFree(pidlFQ);
               pidlFQ = pidlCat;
 
               ILFree(pidl);
            }
            pidl = NULL;

            pszPathSegB = pszPathSegE+1;
            pszPathSegE = wcschr(pszPathSegB, L'\\');
         }

         if (L'\0' != *pszPathSegB)
         {
            CreateFileSystemPidl(pszPathSegB, &pidl);

            // Cache anything in the desktop.ini if this is a directory
            DWORD dwAttrib = ::GetFileAttributes(wszPath);
            if ((-1 != dwAttrib) && (FILE_ATTRIBUTE_DIRECTORY & dwAttrib))
            {
               UpdateFileSystemPidl(pidlFQ, &pidl);
            }
         }

         if (NULL != *ppidl)
         {
            if (NULL != pidl)
            {
               pidlCat = ILConcatenate(*ppidl, pidl);
               ILFree(pidl);
               ILFree(*ppidl);
               *ppidl = pidlCat;
            }
            else
            {
               // Nothing to add to *ppidl, just leave it as is
            }
         }
         else
         {
            *ppidl = pidl;
         }
 
         ILFree(pidlFQ);
      }
      else
      {
         hr = HRESULT_FROM_WIN32_I(::GetLastError());
      }
   }

   return hr;
}

STDMETHODIMP CFileFolder::SetNameOf(HWND hwndOwner,
                                    LPCITEMIDLIST pidl,
                                    LPCOLESTR lpszName,
                                    DWORD uFlags,
                                    LPITEMIDLIST * ppidlOut)
{
   HRESULT hr;
   WCHAR wszNewName[MAX_PATH];
   WCHAR wszFromPath[MAX_PATH];
   WCHAR wszToPath[MAX_PATH];
   LPWSTR pwszToFileName = NULL;
   size_t cch = 0;
   STRRET str;
   SHFILEOPSTRUCT shfos;

   ASSERT(pidl);
   ASSERT(lpszName);
   if (!pidl || !lpszName)
   {
      hr = E_INVALIDARG;
      goto leave;
   }

   if (ppidlOut)
   {
      *ppidlOut = NULL;
   }

   // Validate the name
   hr = StringCchCopy(wszNewName, lengthof(wszNewName), lpszName);
   if (SUCCEEDED(hr))
   {
      WCHAR wszError[MAX_PATH];
      LPWSTR pwszMsg = NULL;
      LPCWSTR pwszFileName = NULL;

      PathRemoveBlanks(wszNewName);

      hr = StringCchLengthW(wszNewName, MAX_PATH, &cch);

      if (FAILED(hr))
      {
         goto leave;
      }

      if ((0 == cch) || (wszNewName[0] == L'.'))
      {
         pwszMsg = MAKEINTRESOURCE(IDS_RENAME_NONAME);
         hr = E_FAIL;
      }
      else if (!PathIsValidFileName(wszNewName))
      {
         if (SUCCEEDED(::StringCchPrintfEx(wszError,
                                   lengthof(wszError),
                                   NULL,
                                   NULL,
                                   STRSAFE_IGNORE_NULLS,
                                   TEXT("%s%s"),
                                   LOAD_STRING(IDS_SHFO_ERR_HDR_RENAME),
                                   LOAD_STRING(IDS_SHFO_ERR_BADNAME))))
         {
            pwszMsg = wszError;
            pwszFileName = ILDisplayName(pidl);
         }
         else
         {
            pwszMsg = LOAD_STRING(IDS_SHFO_ERR_BADNAME);
         }
         hr = E_FAIL;
      }

      if (FAILED(hr))
      {
         ShellDialogs::ShowFileError(hwndOwner,
                                     MAKEINTRESOURCE(IDS_TITLE_RENAMEERROR),
                                     pwszMsg,
                                     pwszFileName,
                                     MB_ICONERROR | MB_OK);
         goto leave;
      }
   }

   // Figure out the "old" name
   memset(&str, 0, sizeof(str));
   hr = GetDisplayNameOf(pidl, SHGDN_NORMAL | SHGDN_FORPARSING, &str);
   if (SUCCEEDED(hr))
   {
      hr = StrRetToBuf(&str, pidl, wszFromPath, lengthof(wszFromPath));
   }

   if (FAILED(hr))
   {
      goto leave;
   }

   wszFromPath[wcslen(wszFromPath)+1] = L'\0'; // 2x null terminate for SHFileOperation

   // Figure out the "new" name
   if (SHGDN_INFOLDER & uFlags)
   {
      LPOLESTR pwszFolder = GetPath(NULL);
      if (pwszFolder)
      {
         hr = StringCchCopy(wszToPath, lengthof(wszToPath), pwszFolder);
         if (SUCCEEDED(hr))
         {
            size_t cchRemaining = 0;
            hr = StringCchCatEx(wszToPath, lengthof(wszToPath), L"\\",
                                &pwszToFileName, &cchRemaining, 0);

            if (SUCCEEDED(hr))
            {
               hr = StringCchCat(pwszToFileName, cchRemaining, wszNewName);
            }
         }

         g_pShellMalloc->Free(pwszFolder);
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
   }
   else
   {
      hr = StringCchCopy(wszToPath, lengthof(wszToPath), wszNewName);
      pwszToFileName = PathFindFileName(wszToPath);
   }

   if (FAILED(hr))
   {
      goto leave;
   }

   ASSERT(pwszToFileName);

   if (!(SHGDN_FORPARSING & uFlags) &&
      !ILIsFileSystemDirectoryPidl(pidl) &&
      !UserSettings::GetShowExtensions())
   {
      LPCWSTR pwszExtension = PathFindExtension(wszFromPath);
      if (TEXT('\0') != *pwszExtension)
      {
         hr = StringCchCat(wszToPath, lengthof(wszToPath), pwszExtension);
      }
   }

   if (FAILED(hr))
   {
      goto leave;
   }

   memset(&shfos, 0, sizeof(shfos));
   shfos.wFunc = FO_RENAME;
   shfos.hwnd = hwndOwner;
   shfos.fFlags = FOF_ALLOWUNDO;
   shfos.pFrom = wszFromPath;
   shfos.pTo = wszToPath;

   if (0 == SHFileOperation(&shfos))
   {
      // Since the user can cancel the file operation and SHFileOperation will
      // still succeed we need this check
      if (PathFileExists(wszFromPath))
      {
         hr = E_FAIL;
      }
   }
   else
   {
      hr = E_FAIL;
   }

   if (SUCCEEDED(hr) && ppidlOut)
   {
      LPITEMIDLIST pidlFQ = NULL;
      if (m_pidl)
      {
         pidlFQ = ILConcatenate(m_pidlRoot, m_pidl);
      }
      else
      {
         pidlFQ = ILCopy(m_pidlRoot, IL_ALL);
      }
      hr = CreateFileSystemPidl(pwszToFileName, ppidlOut);
      UpdateFileSystemPidl(pidlFQ, ppidlOut);
      ILFree(pidlFQ);
   }

leave:
   return hr;
}


//////////////////////////////////////////////////
// IShellDetails

STDMETHODIMP CFileFolder::ColumnClick(UINT iColumn)
{
   return S_FALSE;
}

STDMETHODIMP CFileFolder::GetDetailsOf(LPCITEMIDLIST pidl,
                                       UINT iColumn,
                                       LPSHELLDETAILS pDetails)
{
   ASSERT(pidl);
   ASSERT(pDetails);
   if (!pidl || !pDetails)
      return E_INVALIDARG;

   ASSERT(1 == ILCount(pidl));

   HRESULT hr = NOERROR;

   switch (iColumn)
   {
      // Name
      case 0:
         hr = GetDisplayNameOf(pidl, SHGDN_INFOLDER | SHGDN_FOREDITING, &pDetails->str);
      break;

      // Size
      case 1:
      {
         size_t cch = 30;
         pDetails->str.pOleStr = (LPOLESTR) g_pShellMalloc->Alloc(cch*sizeof(WCHAR));
         if (pDetails->str.pOleStr)
         {
            pDetails->str.uType = STRRET_WSTR;

            if (ILIsFileSystemDirectoryPidl(pidl))
            {
               pDetails->str.pOleStr[0] = L'\0';
            }
            else 
            {
               ULARGE_INTEGER uliSize = {0};
               if (!ILGetSizeCached(pidl, &uliSize) ||
                   !ShellFormat::FileSizeShort(uliSize, pDetails->str.pOleStr, cch))
               {
                  g_pShellMalloc->Free(pDetails->str.pOleStr);
                  hr = E_FAIL;
               }
            }
         }
         else
         {
            hr = E_OUTOFMEMORY;
         }
      }
      break;

      // Type
      case 2:
      {
         LPCWSTR pwszType = NULL;
         
         if (ILIsFileSystemDirectoryPidl(pidl))
         {
            pwszType = LOAD_STRING(IDS_DISPLAY_FSFOLDER);
         }
         else
         {
            pwszType = ILGetTypeCached(pidl);
         }         

         size_t cch = 0;
         if(pwszType && SUCCEEDED(StringCchLengthW(pwszType, MAX_PATH, &cch)))
         {               
            pDetails->str.pOleStr = (LPOLESTR) g_pShellMalloc->Alloc((cch+1)*sizeof(WCHAR));
            if (pDetails->str.pOleStr)
            {
               pDetails->str.uType = STRRET_WSTR;
               if(FAILED(StringCchCopy(pDetails->str.pOleStr, cch+1, pwszType)))
               {
                  g_pShellMalloc->Free(pDetails->str.pOleStr);
                  hr = E_FAIL;
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
      break;

      // Modified
      case 3:
      {
         size_t cch = 64;
         FILETIME ftCached = {0};

         if(ILGetDateCached(pidl, &ftCached))
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
               ShellFormat::FormatDateTime(&ftCached, dwFlags, pDetails->str.pOleStr, cch);
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

      // Attributes
      case 4:
      {
      }
      break;

      default:
         hr = E_FAIL;
   }

   return hr;
}


//////////////////////////////////////////////////
// IPersist

STDMETHODIMP CFileFolder::GetClassID(LPCLSID lpClassID)
{
   return E_NOTIMPL;
}


//////////////////////////////////////////////////
// IPersistFolder

STDMETHODIMP CFileFolder::Initialize(LPCITEMIDLIST pidl)
{
   ASSERT(pidl);
   if (!pidl)
      return E_INVALIDARG;

   HRESULT hr = InitializeNoVerify(pidl);
   if (SUCCEEDED(hr))
   {
      LPWSTR pwszPath = GetPath(&m_cchPath);
      if (pwszPath)
      {
         if (-1 == ::GetFileAttributes(pwszPath))
            hr = HRESULT_FROM_WIN32_I(::GetLastError());

         g_pShellMalloc->Free(pwszPath);
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
   }

   if (FAILED(hr))
   {
      if (m_pidl)
      {
         ILFree(m_pidl);
         m_pidl = NULL;
      }

      if (m_pidlRoot)
      {
         ILFree(m_pidlRoot);
         m_pidlRoot = NULL;
      }
   }

   return hr;
}

