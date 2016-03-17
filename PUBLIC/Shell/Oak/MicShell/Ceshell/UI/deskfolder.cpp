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

#include "deskfolder.h"
#include "desktopview.h"
#include "drivesfolder.h"
#include "filechangemgr.h"
#include "filefolder.h"
#include "fsdrop.h"
#include "guid.h"
#include "idlist.h"
#include "netfolder.h"
#include "nsenum.h"
#include "recbinmenu.h"
#include "recyclefolder.h"
#include "recbindrop.h"
#include "resource.h"
#include "shelldialogs.h"

//////////////////////////////////////////////////
// CDesktopFolder

CDesktopFolder::CDesktopFolder() :
   m_ObjRefCount(0),
   m_pidl(NULL)
{
// g_DllRefCount++;

   m_pidl = CreateRootPidl();

   m_ObjRefCount = 1;
}

CDesktopFolder::~CDesktopFolder()
{
   if (m_pidl)
      ILFree(m_pidl);

// g_DllRefCount--;
}

LPITEMIDLIST CDesktopFolder::CreateRootPidl()
{
   LPITEMIDLIST pidl = NULL;

   CreateNameSpacePidl(CLSID_CEShellDesktop, &pidl);

   return pidl;
}

STDMETHODIMP CDesktopFolder::GetDesktop(LPITEMIDLIST * ppidlDesktop,
                                        IShellFolder ** ppDesktopFolder)
{
   HRESULT hr = NOERROR;
   LPITEMIDLIST pidl = NULL;
   WCHAR wszDesktop[MAX_PATH];

   if (ppidlDesktop)
      *ppidlDesktop = NULL;
   else
      ppidlDesktop = &pidl;

   if (ppDesktopFolder)
      *ppDesktopFolder = NULL;

   // Get a ShellFolder representing the filesystem path to the desktop folder
   if (SHGetSpecialFolderPath(NULL, wszDesktop, CSIDL_DESKTOPDIRECTORY, TRUE))
   {
      hr = ParseDisplayName(NULL, NULL, wszDesktop, NULL, ppidlDesktop, NULL);
      if (SUCCEEDED(hr))
      {
         if (ppDesktopFolder)
         {
            hr = BindToObject(*ppidlDesktop, NULL, IID_IShellFolder,
                              (VOID**) ppDesktopFolder);
         }

         // Free the pidl if the user doesn't want it back
         if (ppidlDesktop == &pidl)
            ILFree(pidl);
      }
   }
   else
   {
      hr = E_FAIL;
   }

   if (FAILED(hr))
   {
      if (ppidlDesktop && *ppidlDesktop)
      {
         ILFree(*ppidlDesktop);
         *ppidlDesktop = NULL;
      }

      if (ppDesktopFolder && *ppDesktopFolder)
      {
         (*ppDesktopFolder)->Release();
         *ppDesktopFolder = NULL;
      }
   }

   return hr;
}


//////////////////////////////////////////////////
// IUnknown

STDMETHODIMP CDesktopFolder::QueryInterface(REFIID riid, LPVOID FAR* ppobj)
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

STDMETHODIMP_(ULONG) CDesktopFolder::AddRef(VOID)
{
   return ++m_ObjRefCount;
}

STDMETHODIMP_(ULONG) CDesktopFolder::Release(VOID)
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

STDMETHODIMP CDesktopFolder::BindToObject(LPCITEMIDLIST pidl,
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
   IShellFolder * pFolder = NULL;

   if (ILIsNameSpacePidl(pidl))
   {
      CLSID clsid;

      hr = CLSIDFromString((LPWSTR)ILParsingName(pidl), &clsid);
      if (SUCCEEDED(hr))
      {
         IClassFactory * pClassFactory = NULL;

         // Try to get the ClassFactory from the local DLL first
         hr = DllGetClassObject(clsid, IID_IClassFactory, (VOID**) &pClassFactory);

         // Next try to locate the item in the registry
         if (FAILED(hr) && (hr == CLASS_E_CLASSNOTAVAILABLE))
         {
            hr = CoGetClassObject(clsid, CLSCTX_INPROC_SERVER, NULL,
                                  IID_IClassFactory, (VOID**) &pClassFactory);
         }

         if (SUCCEEDED(hr))
         {
            // Query the ClassFactory for the correct clsid
            hr = pClassFactory->CreateInstance(NULL, IID_IShellFolder,
                                               (VOID**) &pFolder);
            pClassFactory->Release();

            if (SUCCEEDED(hr))
            {
               // Now pFolder points to the correct type of namespace, bind it
               // to anything in the namepace if there is more to the pidl
               pidl = ILNext(pidl);
               if (pidl)
                  hr = pFolder->BindToObject(pidl, pbc, riid, ppvOut);
               else
                  hr = pFolder->QueryInterface(riid, ppvOut);
             }
         }
      }
   }
   else
   {
      // Try the desktop filesystem folder
      hr = GetDesktop(NULL, &pFolder);
      if (SUCCEEDED(hr))
         hr = pFolder->BindToObject(pidl, pbc, riid, ppvOut);
   }

   if (pFolder)
      pFolder->Release();

   return hr;
}

STDMETHODIMP CDesktopFolder::BindToStorage(LPCITEMIDLIST pidl,
                                           LPBC pbcReserved,
                                           REFIID riid,
                                           LPVOID * ppvObj)
{
   return E_NOTIMPL;
}

STDMETHODIMP CDesktopFolder::CompareIDs(LPARAM lParam,
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
      IShellFolder * pFolder = NULL;
      GetDesktop(NULL, &pFolder);
      if (pFolder)
      {
         HRESULT hr = pFolder->CompareIDs(lParam, pidl1, pidl2);
         severity = (short) HRESULT_SEVERITY(hr);
         result = HRESULT_CODE(hr);
         pFolder->Release();
      }
      else
      {
         severity = SEVERITY_ERROR;
      }
   }

   return MAKE_HRESULT(severity, 0, (USHORT) result);
}

STDMETHODIMP CDesktopFolder::CreateViewObject(HWND hwndOwner, 
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
      LPITEMIDLIST pidlDesktop = NULL;
      hr = GetDesktop(&pidlDesktop, NULL);
      if (SUCCEEDED(hr))
      {
         AddRef(); // For the IShellFolder passed to CDefShellView

         CDesktopView * pShellView = new CDesktopView(this, pidlDesktop);
         if (pShellView)
         {
            // Hook up the file notifications
            UINT uWatchID = 0;
            WCHAR wszDesktop[MAX_PATH];
            if (SHGetSpecialFolderPath(NULL, wszDesktop,
                                       CSIDL_DESKTOPDIRECTORY, TRUE))
            {
               IShellChangeNotify * pShellChangeNotify = NULL;
               if (SUCCEEDED(pShellView->QueryInterface(IID_IShellChangeNotify,
                                                        (VOID**)&pShellChangeNotify)))
               {
                  uWatchID = g_pFileChangeManager->AddWatch(wszDesktop,
                                                            pShellChangeNotify);
                  if (0 != uWatchID)
                     pShellView->SetWatchID(uWatchID);

                  pShellChangeNotify->Release();
               }
            }

            hr = pShellView->QueryInterface(riid, ppvOut);

            if (FAILED(hr) && (0 != uWatchID))
               g_pFileChangeManager->RemoveWatch(uWatchID);

            pShellView->Release();
         }
         else
         {
            Release();
            hr = E_OUTOFMEMORY;
         }

         ILFree(pidlDesktop);
      }
   }

leave:
   return hr;
}

STDMETHODIMP CDesktopFolder::EnumObjects(HWND hwndOwner,
                                         DWORD grfFlags,
                                         LPENUMIDLIST * ppenumIDList)
{
   ASSERT(ppenumIDList);
   if (!ppenumIDList)
      return E_INVALIDARG;

   HRESULT hr = NOERROR;
   LPITEMIDLIST pidlDesktop = NULL;

   // Build the enumerator
   CNameSpaceEnum * pEnum = new CNameSpaceEnum(grfFlags);
   if (pEnum)
   {
      GetDesktop(&pidlDesktop, NULL);

      if (pEnum->Initialize(CLSID_CEShellDesktop, pidlDesktop))
      {
         *ppenumIDList = (IEnumIDList *) pEnum;
      }
      else
      {
         pEnum->Release();
         hr = E_FAIL;
      }

      if (pidlDesktop)
         ILFree(pidlDesktop);
   }
   else
   {
      hr = E_OUTOFMEMORY;
   }

   return hr;
}

STDMETHODIMP CDesktopFolder::GetAttributesOf(UINT cidl,
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
   IShellFolder * pDesktopFolder = NULL;

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
   hr = GetDesktop(NULL, &pDesktopFolder);
   if (SUCCEEDED(hr))
   {
      if (0 < cidlFS)
         hr = pDesktopFolder->GetAttributesOf(cidlFS, apidlFS, rgfInOut);

      pDesktopFolder->Release();
   }

   g_pShellMalloc->Free(apidlFS);

   return hr;
}

STDMETHODIMP CDesktopFolder::GetDisplayNameOf(LPCITEMIDLIST pidl,
                                              DWORD uFlags,
                                              LPSTRRET lpName)
{
   ASSERT(lpName);
   if (!lpName)
      return E_INVALIDARG;

   HRESULT hr = NOERROR;
   WCHAR wszDisplayName[MAX_PATH];

   // NULL pidls get mapped to the desktop namespace
   if (ILIsEmpty(pidl))
      pidl = m_pidl;   

   // Validate
   if (1 < ILCount(pidl))
      return E_UNEXPECTED;

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
   }
   else
   {
      // Try the filesystem folder
      IShellFolder * pDesktopFolder = NULL;
      hr = GetDesktop(NULL, &pDesktopFolder);
      if (SUCCEEDED(hr))
      {
         hr = pDesktopFolder->GetDisplayNameOf(pidl, uFlags, lpName);
         pDesktopFolder->Release();
      }
   }

   return hr;
}

STDMETHODIMP CDesktopFolder::GetUIObjectOf(HWND hwndOwner,
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

   IShellFolder * pFolder = NULL;
   HRESULT hr = GetDesktop(NULL, &pFolder);   
   if (FAILED(hr))
      return hr;

   PREFAST_ASSERT(pFolder);

   if (IsEqualIID(riid, IID_IDataObject)) // IDataObject
   {
      // Defer this call to the filesystem folder representing the Desktop
      hr = pFolder->GetUIObjectOf(hwndOwner, cidl, apidl,
                                  riid, prgfInOut, ppvOut);
   }
   else if (IsEqualIID(riid, IID_IDropTarget)) // IDropTarget
   {
      ASSERT(1 == cidl);
      ASSERT(1 == ILCount(*apidl));

      if (ILIsNameSpace(*apidl, CLSID_CEShellDrives))
      {
         CFileSystemDropTarget * pDropTarget = new CFileSystemDropTarget();
         if (pDropTarget)
         {
            if (pDropTarget->Initialize(L"\\"))
               hr = pDropTarget->QueryInterface(riid, ppvOut);
            else
               hr = E_FAIL;
            pDropTarget->Release();
         }
         else
         {
            hr = E_OUTOFMEMORY;
         }
      }
      else if (ILIsNameSpace(*apidl, CLSID_CEShellBitBucket))
      {
         CRecBinDropTarget * pDropTarget = new CRecBinDropTarget();
         if (pDropTarget)
         {            
            if (pDropTarget->Initialize())
               hr = pDropTarget->QueryInterface(riid, ppvOut);
            else
               hr = E_FAIL;
            pDropTarget->Release();
         }
         else
         {
            hr = E_OUTOFMEMORY;
         }
      }
      else
      {
         // Defer this call to the filesystem folder representing the Desktop
         hr = pFolder->GetUIObjectOf(hwndOwner, cidl, apidl,
                                     riid, prgfInOut, ppvOut);
      }
   }
   else if (IsEqualIID(riid, IID_IContextMenu)) // IContextMenu
   {
      if (ILIsNameSpace(*apidl, CLSID_CEShellDrives))
      {
         // No special menu yet
         hr = E_NOTIMPL;
      }
      else if (ILIsNameSpace(*apidl, CLSID_CEShellBitBucket))
      {
         CRecycleBinMenu * pRecycleBinMenu = new CRecycleBinMenu();
         if (pRecycleBinMenu)
         {
            hr = pRecycleBinMenu->QueryInterface(riid, ppvOut);
            pRecycleBinMenu->Release();
         }
         else
         {
            hr = E_OUTOFMEMORY;
         }
      }
      else
      {
         // Defer this call to the filesystem folder representing the Desktop
         hr = pFolder->GetUIObjectOf(hwndOwner, cidl, apidl,
                                     riid, prgfInOut, ppvOut);
      }
   }

   pFolder->Release();

   return hr;
}

STDMETHODIMP CDesktopFolder::ParseDisplayName(HWND hwndOwner,
                                              LPBC pbc,
                                              LPOLESTR pwszDisplayName,
                                              ULONG * pchEaten,
                                              LPITEMIDLIST * ppidl,
                                              ULONG *pdwAttributes)
{
   ASSERT(ppidl);
   if (!ppidl)
      return E_INVALIDARG;

   HRESULT hr = NOERROR;
   *ppidl = NULL;
   
   IShellFolder * pFolder = NULL;
   LPITEMIDLIST pidlRoot = NULL;
   LPITEMIDLIST pidlTail = NULL;
   LPWSTR pwsz = NULL, pwszBegin = NULL, pwszEnd = NULL;
   size_t cch = 0;

   if (0 == wcsncmp(pwszDisplayName, L"::{", 3))
   {
      pwszBegin = pwszDisplayName + 2;
      hr = StringCchLength(pwszBegin, MAX_PATH, &cch); // TODO: MAX_PATH is not valid here
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
      if (pwsz)
      {
         CLSID clsid;

         memcpy(pwsz, pwszBegin, cch*sizeof(WCHAR));
         pwsz[cch] = L'\0';

         hr = CLSIDFromString(pwsz, &clsid);
         if (SUCCEEDED(hr))
         {
            // TODO: validate that the clsid belongs on the desktop (do reg query)
            hr = CreateNameSpacePidl(clsid, &pidlRoot);
         }
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }

      if (FAILED(hr))
         goto ParseDisplayName_done;

      if (pwszEnd)
      {
          pwszDisplayName = pwszEnd+1;
      }
      else
      {
         *ppidl = ILCopy(pidlRoot, IL_ALL);
         if (!*ppidl)
            hr = E_OUTOFMEMORY;
      }
   }
   else
   {
      // Desktop is responsible for parsing filesystem and UNC paths
      if (L'\\' == *pwszDisplayName)
      {
         switch (pwszDisplayName[1])
         {
            case L'\0': // Just "My Computer"
               hr = CreateNameSpacePidl(CLSID_CEShellDrives, ppidl);
            break;

            case L'\\': // UNC
               hr = CreateNameSpacePidl(CLSID_CEShellNetwork, &pidlRoot);
            break;

            default: // Probably a fully qualified path
               if (PathFileExists(pwszDisplayName))
               {
                  // Since My Computer doubles as c: on CE make sure the path
                  // is relative to the filesystem root
                  pwszDisplayName++;
                  hr = CreateNameSpacePidl(CLSID_CEShellDrives, &pidlRoot);
               }
               else
               {
                  hr = HRESULT_FROM_WIN32_I(::GetLastError());
               }
         }
      }
      else // Special folder
      {
         hr = E_FAIL;
      }
   }
   
   // See if there is still more work to do
   if (SUCCEEDED(hr) && !*ppidl)
   {
      ASSERT(pidlRoot);

      hr = BindToObject(pidlRoot, NULL, IID_IShellFolder, (VOID**) &pFolder);
      if (SUCCEEDED(hr))
      {
         hr = pFolder->ParseDisplayName(hwndOwner, pbc, pwszDisplayName,
                                        pchEaten, &pidlTail, pdwAttributes);
         if (SUCCEEDED(hr))
         {
            ASSERT(pidlTail);

            *ppidl = ILConcatenate(pidlRoot, pidlTail);
            if (!*ppidl)
               hr = E_OUTOFMEMORY;

            ILFree(pidlTail);
         }

         pFolder->Release();
      }

   }

ParseDisplayName_done:

   if (pidlRoot)
      ILFree(pidlRoot);

   if (pwsz)
      g_pShellMalloc->Free(pwsz);

   return hr;
}

STDMETHODIMP CDesktopFolder::SetNameOf(HWND hwndOwner,
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
      IShellFolder * pDesktopFolder = NULL;
      hr = GetDesktop(NULL, &pDesktopFolder);
      if (SUCCEEDED(hr))
      {
         hr = pDesktopFolder->SetNameOf(hwndOwner, pidl, lpszName, uFlags, ppidlOut);
         pDesktopFolder->Release();
      }
   }

leave:
   return hr;
}


//////////////////////////////////////////////////
// IShellDetails

STDMETHODIMP CDesktopFolder::ColumnClick(UINT iColumn)
{
   return S_FALSE;
}

STDMETHODIMP CDesktopFolder::GetDetailsOf(LPCITEMIDLIST pidl,
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
      IShellFolder * pDesktopFolder = NULL;
      hr = GetDesktop(NULL, &pDesktopFolder);
      if (SUCCEEDED(hr))
      {
         IShellDetails * pShellDetails = NULL;
         hr = pDesktopFolder->QueryInterface(IID_IShellDetails,
                                             (VOID**) &pShellDetails);
         if (SUCCEEDED(hr))
         {
            hr = pShellDetails->GetDetailsOf(pidl, iColumn, pDetails);
            pShellDetails->Release();
         }
         pDesktopFolder->Release();
      }
   }


   return hr;
}


//////////////////////////////////////////////////
// IPersist

STDMETHODIMP CDesktopFolder::GetClassID(LPCLSID lpClassID)
{
   return E_NOTIMPL;
}


//////////////////////////////////////////////////
// IPersistFolder

STDMETHODIMP CDesktopFolder::Initialize(LPCITEMIDLIST pidlFQ)
{
   // How can you bind the desktop folder??
   return E_UNEXPECTED;
}

