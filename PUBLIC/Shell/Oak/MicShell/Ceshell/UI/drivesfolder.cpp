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

#include "drivesfolder.h"
#include "defshellview.h"
#include "filechangemgr.h"
#include "filefolder.h"
#include "guid.h"
#include "idlist.h"
#include "nsenum.h"
#include "resource.h"
#include "shelldialogs.h"

//////////////////////////////////////////////////
// CDrivesFolder

IShellFolder * CDrivesFolder::m_psfDrivesDirectory = NULL;

CDrivesFolder::CDrivesFolder() :
   m_ObjRefCount(0)
{
// g_DllRefCount++;

   HRESULT hr = S_OK;
   hr = CreateNameSpacePidl(CLSID_CEShellDrives, &m_pidl);
   ASSERT(SUCCEEDED(hr));

   if (NULL == m_psfDrivesDirectory)
   {
      m_psfDrivesDirectory = new CFileFolder();
   }

   m_ObjRefCount = 1;
}

CDrivesFolder::~CDrivesFolder()
{
   if (m_pidl)
      ILFree(m_pidl);

// g_DllRefCount--;
}


//////////////////////////////////////////////////
// IUnknown

STDMETHODIMP CDrivesFolder::QueryInterface(REFIID riid,
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

STDMETHODIMP_(ULONG) CDrivesFolder::AddRef(VOID)
{
   return ++m_ObjRefCount;
}

STDMETHODIMP_(ULONG) CDrivesFolder::Release(VOID)
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

STDMETHODIMP CDrivesFolder::BindToObject(LPCITEMIDLIST pidl,
                                         LPBC pbc,
                                         REFIID riid,
                                         LPVOID * ppvOut)
{
   ASSERT(pidl);
   ASSERT(ppvOut);
   if (!pidl || !ppvOut)
      return E_FAIL;
   *ppvOut = NULL;

   HRESULT hr = NOERROR;
   IShellFolder * pFolder = NULL;
   CLSID clsid;

   if (ILIsNameSpacePidl(pidl))
   {
      hr = CLSIDFromString((LPWSTR)ILParsingName(pidl), &clsid);
      if (SUCCEEDED(hr))
      {
         hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER,
                               IID_IShellFolder, (VOID**) &pFolder);
      }
   }
   else
   {
      // Try "\"
      pFolder = (IShellFolder *) new CFileFolder();
      if (pFolder)
      {
         IPersistFolder * pPersistFolder = NULL;
         hr = pFolder->QueryInterface(IID_IPersistFolder, (VOID**) &pPersistFolder);
         if (SUCCEEDED(hr))
         {
            LPITEMIDLIST pidlRoot = ILCopy(pidl, 1);
            if (pidlRoot)
            {
               hr = pPersistFolder->Initialize(pidlRoot);
               ILFree(pidlRoot);
            }
            else
            {
               hr = E_OUTOFMEMORY;
            }

            pPersistFolder->Release();
         }
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
   }

   if (SUCCEEDED(hr))
   {
      pidl = ILNext(pidl);
      if (pidl)
         hr = pFolder->BindToObject(pidl, pbc, riid, ppvOut);
      else
         hr = pFolder->QueryInterface(riid, ppvOut);
   }

   if (pFolder)
      pFolder->Release();

   return hr;
}

STDMETHODIMP CDrivesFolder::BindToStorage(LPCITEMIDLIST pidl,
                                          LPBC pbcReserved,
                                          REFIID riid,
                                          LPVOID * ppvObj)
{
   return E_NOTIMPL;
}

STDMETHODIMP CDrivesFolder::CompareIDs(LPARAM lParam,
                                       LPCITEMIDLIST pidl1,
                                       LPCITEMIDLIST pidl2)
{
   ASSERT(pidl1);
   ASSERT(pidl2);
   if (!pidl1 || !pidl2)
      return E_INVALIDARG;

   short severity = SEVERITY_SUCCESS;
   int result = 0;

   if (ILIsNameSpacePidl(pidl1) && ILIsNameSpacePidl(pidl2))
   {
      int cmp = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                              ILDisplayName(pidl1), -1,
                              ILDisplayName(pidl2), -1);
      if (0 != cmp)
      {
         result = (cmp - 2); // See docs on CompareString
      }
      else
      {
         severity = SEVERITY_ERROR;
      }
   }
   else if (ILIsNameSpacePidl(pidl1) && !ILIsNameSpacePidl(pidl2))
   {
      result = -1;
   }
   else if (!ILIsNameSpacePidl(pidl1) && ILIsNameSpacePidl(pidl2))
   {
      result = 1;
   }
   else
   {
      IShellFolder * pFolder = m_psfDrivesDirectory;
      if (pFolder)
      {
         HRESULT hr = pFolder->CompareIDs(lParam, pidl1, pidl2);
         severity = (short) HRESULT_SEVERITY(hr);
         result = HRESULT_CODE(hr);
      }
      else
      {
         severity = SEVERITY_ERROR;
      }
   }

   return MAKE_HRESULT(severity, 0, (USHORT) result);
}

STDMETHODIMP CDrivesFolder::CreateViewObject(HWND hwndOwner, 
                                             REFIID riid, 
                                             LPVOID * ppvOut)
{
   HRESULT hr = E_NOINTERFACE;

   if (!ppvOut)
   {
      ASSERT(0);
      hr = E_INVALIDARG;
      goto leave;
   }

   *ppvOut = NULL;

   if(IsEqualIID(riid, IID_IShellView))
   {
      LPITEMIDLIST pidlFQ = NULL;
      hr = SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlFQ);
      if (SUCCEEDED(hr))
      {
         AddRef(); // For the IShellFolder passed to CDefShellView

         CDefShellView * pShellView = new CDefShellView(this, pidlFQ);

         ILFree(pidlFQ);

         if(!pShellView)
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
            uWatchID = g_pFileChangeManager->AddWatch(L"\\", pShellChangeNotify);
            if (0 != uWatchID)
               pShellView->SetWatchID(uWatchID);

            pShellChangeNotify->Release();
         }

         hr = pShellView->QueryInterface(riid, ppvOut);

         if (FAILED(hr) && (0 != uWatchID))
            g_pFileChangeManager->RemoveWatch(uWatchID);

         pShellView->Release();
      }
   }

leave:
   return hr;
}

STDMETHODIMP CDrivesFolder::EnumObjects(HWND hwndOwner,
                                        DWORD grfFlags,
                                        LPENUMIDLIST * ppenumIDList)
{
   if (!ppenumIDList)
      return E_INVALIDARG;
   *ppenumIDList = NULL;

   HRESULT hr = NOERROR;

   CNameSpaceEnum * pEnumNS = new CNameSpaceEnum(grfFlags);
   if (pEnumNS)
   {
      LPITEMIDLIST pidl = NULL;
      CreateNameSpacePidl(CLSID_CEShellDrives, &pidl);

      if (pEnumNS->Initialize(CLSID_CEShellDrives, pidl))
         *ppenumIDList = (LPENUMIDLIST) pEnumNS;
      else
         hr = E_FAIL;

      if (pidl)
         ILFree(pidl);
   }
   else
   {
      hr = E_OUTOFMEMORY;
   }

   return hr;
}

STDMETHODIMP CDrivesFolder::GetAttributesOf(UINT cidl,
                                            LPCITEMIDLIST * apidl,
                                            ULONG * rgfInOut)
{
   ASSERT(apidl);
   ASSERT(rgfInOut);
   if (!apidl || !rgfInOut)
      return E_INVALIDARG;

   HRESULT hr = NOERROR;
   UINT cidlFS = 0;
   LPCITEMIDLIST * apidlFS = (LPCITEMIDLIST *) g_pShellMalloc->Alloc(cidl*sizeof(LPCITEMIDLIST));

   if (!apidlFS)
      return E_OUTOFMEMORY;

   // Seperate the namespace pidls from the filesystem pidls
   for (UINT i = 0; i < cidl; i++)
   {
      if (ILIsNameSpacePidl(apidl[i]))
      {
         (*rgfInOut) &= (SFGAO_BROWSABLE | SFGAO_CANLINK |
                         SFGAO_FILESYSANCESTOR | SFGAO_HASSUBFOLDER |
                         SFGAO_CANRENAME);
      }
      else
      {
         apidlFS[cidlFS++] = apidl[i];
      }
   }

   // Pass the pidl list on to the file system directory
   if (0 < cidlFS)
   {
      IShellFolder * pDrivesDirectory = m_psfDrivesDirectory;
      if (pDrivesDirectory)
      {
         hr = pDrivesDirectory->GetAttributesOf(cidlFS, apidlFS, rgfInOut);
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
   }

   g_pShellMalloc->Free(apidlFS);

   return hr;
}

STDMETHODIMP CDrivesFolder::GetDisplayNameOf(LPCITEMIDLIST pidl,
                                             DWORD uFlags,
                                             LPSTRRET lpName)
{
   ASSERT(pidl);
   ASSERT(lpName);

   if (!pidl || !lpName)
      return E_INVALIDARG;

   HRESULT hr = NOERROR;
   WCHAR wszDisplayName[MAX_PATH];

   // Validate
   if (1 != ILCount(pidl))
      return E_FAIL;

   if (ILIsNameSpacePidl(pidl))
   {
      if (SHGDN_FORPARSING & uFlags)
      {
         ::wcscpy(wszDisplayName, L"::");
         hr = StringCchCat(wszDisplayName, lengthof(wszDisplayName),
                           ILParsingName(pidl));
      }
      else
      {
         hr = StringCchCopy(wszDisplayName, lengthof(wszDisplayName),
                            ILDisplayName(pidl));
      }
   }
   else
   {
      LPWSTR pwszName = wszDisplayName;
      if (!(SHGDN_INFOLDER & uFlags))
      {
         *pwszName = L'\\';
         pwszName++;
      }
      if (ILIsNameSpacePidl(pidl) && (SHGDN_FORPARSING & uFlags))
      {
         *pwszName = L':';
         pwszName++;
         *pwszName = L':';
         pwszName++;
      }
      *pwszName = L'\0';

      if (SHGDN_FORPARSING & uFlags)
      {
         hr = StringCchCat(wszDisplayName, lengthof(wszDisplayName),
                           ILParsingName(pidl));
      }
      else
      {
         hr = StringCchCat(wszDisplayName, lengthof(wszDisplayName),
                           ILDisplayName(pidl));
      }
   }

   if (SUCCEEDED(hr))
   {
      size_t cb = (::wcslen(wszDisplayName)+1)*sizeof(WCHAR);

      lpName->pOleStr = (LPWSTR) g_pShellMalloc->Alloc(cb);
      if (lpName->pOleStr)
      {
         ::wcscpy(lpName->pOleStr, wszDisplayName);
         lpName->uType = STRRET_WSTR;
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
   }

   return hr;
}

STDMETHODIMP CDrivesFolder::GetUIObjectOf(HWND hwndOwner,
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
   IShellFolder * pFolder = m_psfDrivesDirectory;

   if (!pFolder)
      return E_OUTOFMEMORY;

   if (IsEqualIID(riid, IID_IDataObject)) // IDataObject
   {
      // Defer this call to the filesystem folder representing My Computer
      hr = pFolder->GetUIObjectOf(hwndOwner, cidl, apidl,
                                  riid, prgfInOut, ppvOut);
   }
   else if (IsEqualIID(riid, IID_IDropTarget)) // IDropTarget
   {
      ASSERT(1 == cidl);
      ASSERT(1 == ILCount(*apidl));

      // Defer this call to the filesystem folder representing My Computer
      hr = pFolder->GetUIObjectOf(hwndOwner, cidl, apidl,
                                  riid, prgfInOut, ppvOut);
   }
   else if (IsEqualIID(riid, IID_IContextMenu)) // IContextMenu
   {
      // Defer this call to the filesystem folder representing My Computer
      hr = pFolder->GetUIObjectOf(hwndOwner, cidl, apidl,
                                  riid, prgfInOut, ppvOut);
   }

   return hr;
}

STDMETHODIMP CDrivesFolder::ParseDisplayName(HWND hwndOwner,
                                             LPBC pbc,
                                             LPOLESTR pwszDisplayName,
                                             ULONG * pchEaten,
                                             LPITEMIDLIST * ppidl,
                                             ULONG * pdwAttributes)
{
   ASSERT(ppidl);
   if (!ppidl)
      return E_INVALIDARG;

   HRESULT hr = NOERROR;
   *ppidl = NULL;

   if (0 == wcsncmp(pwszDisplayName, L"::{", 3))
   {
      CLSID clsid;
      hr = CLSIDFromString(pwszDisplayName, &clsid);
      if (SUCCEEDED(hr))
      {
         // TODO: validate that the clsid belongs in My Computer (do reg query)
         hr = CreateNameSpacePidl(clsid, ppidl);
      }
      // TODO: verify that there isn't anymore work to do
   }
   else
   {
      if ((L'\0' == *pwszDisplayName) || (L'\\' == *pwszDisplayName))
      {
         // The FS root and UNC names need to be parsed by the desktop namespace
         hr = E_FAIL;
      }
      else // FileSystem path
      {
         LPITEMIDLIST pidlRoot = NULL;
         LPWSTR pwszRoot = NULL;
         size_t cch = 0;

         hr = StringCchLength(pwszDisplayName, MAX_PATH, &cch);
         if (SUCCEEDED(hr))
         {
            pwszRoot = ::wcschr(pwszDisplayName, L'\\');
            if (pwszRoot)
               cch = pwszRoot-pwszDisplayName;

            pwszRoot = (LPWSTR) g_pShellMalloc->Alloc((cch+1)*sizeof(WCHAR));
            if (pwszRoot)
            {
               memcpy(pwszRoot, pwszDisplayName, cch*sizeof(WCHAR));
               pwszRoot[cch] = L'\0';

               hr = CreateFileSystemPidl(pwszRoot, &pidlRoot);
               UpdateFileSystemPidl(m_pidl, &pidlRoot);
               g_pShellMalloc->Free(pwszRoot);

               if (SUCCEEDED(hr))
               {
                  pwszDisplayName += cch;
                  if (L'\0' != *pwszDisplayName)
                  {
                     IShellFolder * pFolder = NULL;

                     ASSERT(L'\\' == *pwszDisplayName);
                     pwszDisplayName++; // Walk over the \ 

                     hr = BindToObject(pidlRoot, NULL, IID_IShellFolder, (VOID**) &pFolder);
                     if (SUCCEEDED(hr))
                     {
                        LPITEMIDLIST pidlTail = NULL;

                        hr = pFolder->ParseDisplayName(hwndOwner, pbc, pwszDisplayName,
                                                       pchEaten, &pidlTail, pdwAttributes);
                        if (SUCCEEDED(hr))
                        {
                           if (NULL != pidlTail)
                           {
                              *ppidl = ILConcatenate(pidlRoot, pidlTail);
                              ILFree(pidlTail);
                           }
                           else
                           {
                              *ppidl = ILCopy(pidlRoot, IL_ALL);
                           }

                           if (!*ppidl)
                              hr = E_OUTOFMEMORY;
                        }


                        pFolder->Release();
                     }
                  }
                  else
                  {
                     *ppidl = ILCopy(pidlRoot, IL_ALL);
                     if (!*ppidl)
                        hr = E_OUTOFMEMORY;
                  }

                  ILFree(pidlRoot);
               }
            }
            else
            {
               hr = E_OUTOFMEMORY;
            }
         }
      }
   }

   return hr;
}

STDMETHODIMP CDrivesFolder::SetNameOf(HWND hwndOwner,
                                      LPCITEMIDLIST pidl,
                                      LPCOLESTR lpszName,
                                      DWORD uFlags,
                                      LPITEMIDLIST * ppidlOut)
{
   HRESULT hr = NOERROR;

   ASSERT(pidl);
   ASSERT(lpszName);
   if (!pidl || !lpszName)
   {
      hr = E_INVALIDARG;
      goto leave;
   }

   ASSERT(1 == ILCount(pidl));

   if (ILIsNameSpacePidl(pidl))
   {
      size_t cch = 0;
      WCHAR wszNewName[MAX_PATH];
      WCHAR wszError[MAX_PATH];
      LPWSTR pwszMsg = NULL;
      LPCWSTR pwszFileName = NULL;

      hr = StringCchCopy(wszNewName, lengthof(wszNewName), lpszName);
      if (SUCCEEDED(hr))
      {
         PathRemoveBlanks(wszNewName);
         hr = StringCchLengthW(wszNewName, lengthof(wszNewName), &cch);
      }

      if (FAILED(hr))
      {
         goto leave;
      }

      if (0 == cch)
      {
         pwszMsg = MAKEINTRESOURCE(IDS_RENAME_NONAME);
         hr = E_FAIL;
      }
      else if (!PathIsValidFileName(PathFindFileName(wszNewName)))
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

      LPITEMIDLIST pidlNew = ILCopy(pidl, IL_ALL);
      if (pidlNew)
      {
         hr = RenameNamespacePidl(wszNewName, &pidlNew);
         if (SUCCEEDED(hr))
         {
            if (ppidlOut)
            {
               *ppidlOut = pidlNew;
            }
            else
            {
               ILFree(pidlNew);
            }
         }
         else
         {
            ILFree(pidlNew);
         }
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
   }
   else
   {
      IShellFolder * pDrivesDirectory = m_psfDrivesDirectory;
      if (pDrivesDirectory)
      {
         hr = pDrivesDirectory->SetNameOf(hwndOwner, pidl, lpszName, uFlags, ppidlOut);
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
   }

leave:
   return hr;
}


//////////////////////////////////////////////////
// IShellDetails

STDMETHODIMP CDrivesFolder::ColumnClick(UINT iColumn)
{
   return S_FALSE;
}

STDMETHODIMP CDrivesFolder::GetDetailsOf(LPCITEMIDLIST pidl,
                                         UINT iColumn,
                                         LPSHELLDETAILS pDetails)
{
   ASSERT(pidl);
   ASSERT(pDetails);
   if (!pidl || !pDetails)
      return E_INVALIDARG;

   ASSERT(1 == ILCount(pidl));

   HRESULT hr = NOERROR;

   if (ILIsNameSpacePidl(pidl))
   {
      switch (iColumn)
      {
         // Name
         case 0:
            hr = GetDisplayNameOf(pidl, SHGDN_INFOLDER | SHGDN_FOREDITING, &pDetails->str);
         break;

         // Size
         case 1:
         {
         }
         break;

         // Type
         case 2:
         {
         }
         break;

         // Modified
         case 3:
         {
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
   }
   else
   {
      IShellFolder * pDrivesDirectory = m_psfDrivesDirectory;
      if (pDrivesDirectory)
      {
         IShellDetails * pShellDetails = NULL;
         hr = pDrivesDirectory->QueryInterface(IID_IShellDetails,
                                               (VOID**) &pShellDetails);
         if (SUCCEEDED(hr))
         {
            hr = pShellDetails->GetDetailsOf(pidl, iColumn, pDetails);
            pShellDetails->Release();
         }
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
   }

   return hr;
}


//////////////////////////////////////////////////
// IPersist

STDMETHODIMP CDrivesFolder::GetClassID(LPCLSID lpClassID)
{
   return E_NOTIMPL;
}


//////////////////////////////////////////////////
// IPersistFolder

STDMETHODIMP CDrivesFolder::Initialize(LPCITEMIDLIST pidlFQ)
{
   // TODO: assert that this is really just "my computer" pidlFQ
   return NOERROR;
}

