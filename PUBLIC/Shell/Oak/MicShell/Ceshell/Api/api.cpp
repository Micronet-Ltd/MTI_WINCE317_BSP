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

#include "api.h"
#include "guid.h"
#include "iconcache.h"
#include "idlist.h"
#include "malloc.h"
#include "resource.h"
#include <windev.h>

SHSTDAPI_(BOOL) SHGetDocumentsFolder(LPCTSTR pszVolume, LPTSTR pszDocs)
{
   BOOL fReturn = FALSE;

   if (0 == wcscmp(pszVolume, TEXT("\\")))
   {
      // asking about the root RAM volume
      fReturn = SHGetSpecialFolderPath(NULL, pszDocs, CSIDL_PERSONAL, TRUE);
      if (!fReturn)
         *pszDocs = TEXT('\0');
   }
    else
    {
        TCHAR szTemp[MAX_PATH];
        DWORD dwAttribs = GetFileAttributes(pszVolume);
        if (dwAttribs != (DWORD)-1)
        {
#ifdef DEBUG
            // to simulate storage card, accept CF*
            static const TCHAR szDebugStorageDir[] = TEXT("CF");
            if ((dwAttribs & FILE_ATTRIBUTE_DIRECTORY) && ! wcsncmp(pszVolume + 1, szDebugStorageDir, wcslen(szDebugStorageDir)))
                dwAttribs = (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_TEMPORARY);
#endif
            // there is such a beast to begin with
            if ((dwAttribs & (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_TEMPORARY)) == (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_TEMPORARY))
            {
                wsprintf(szTemp, TEXT("%s\\ignore_my_docs"), pszVolume);
                if (GetFileAttributes(szTemp) != (DWORD)-1)
                {
                    // this special file exists...  it tells us to ignore any mydocs folder.
                    // we will use the storage card volume as the root my docs path
                    if (pszDocs != pszVolume)
                        lstrcpy(pszDocs, pszVolume);
                }
                else
                {
                    // it's a storage card!
                    // (the My Documents folder is not localized nor is it (until now)
                    // available via API for storage cards
                    wsprintf(szTemp, TEXT("%s\\My Documents"), pszVolume);

                    if (GetFileAttributes(szTemp) != (DWORD)-1)
                    {
                        // there's a My Docs directory already here (likely from a Rapier device)
                        // use it.
                        //
                        lstrcpy(pszDocs, szTemp);
                    }
                    else
                    {
                        HANDLE hFile;
                        // there's no my docs folder.
                        // use the root as the mydocs
                        if (pszDocs != pszVolume)
                            lstrcpy(pszDocs, pszVolume);

                        // also mark this as a card to forever more ignore mydocs
                        wsprintf(szTemp, TEXT("%s\\ignore_my_docs"), pszVolume);
                        hFile = CreateFile(szTemp, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
#ifdef DEBUG
                                           0,
#else
                                           FILE_ATTRIBUTE_HIDDEN,
#endif
                                           NULL);
                        // just create it and close it.  can't do much if it fails, so don't worry about it
                        CloseHandle(hFile);
                    }
                }
                fReturn = TRUE;
            }
        }
   }

   return fReturn;
}

SHSTDAPI SHGetMalloc(LPMALLOC * ppMalloc)
{
   ASSERT(ppMalloc);
   if (!ppMalloc)
      return E_INVALIDARG;

   // Should we just use CoGetMalloc instead?
   *ppMalloc = (LPMALLOC) new CShellMalloc();

   return ((*ppMalloc) ? NOERROR : E_OUTOFMEMORY);
}

/*
SHGetSpecialFolderLocation
   CSIDL_BITBUCKET
   CSIDL_CONTROLS
   CSIDL_DESKTOP
   CSIDL_DRIVES

SHGetSpecialFolderPath
   CSIDL_DESKTOPFOLDER
   CSIDL_FAVORITES
   CSIDL_FONTS (?)
   CSIDL_PERSONAL
   CSIDL_PROFILE
   CSIDL_PROGRAMS
   CSIDL_RECENT
   CSIDL_STARTUP
*/
SHSTDAPI SHGetSpecialFolderLocation(HWND hwndOwner, int nFolder, LPITEMIDLIST * ppidl)
{
   ASSERT(ppidl);
   if (!ppidl)
      return E_INVALIDARG;
   *ppidl = NULL;

   HRESULT hr = NOERROR;
   BOOL fCreate = (CSIDL_FLAG_CREATE & nFolder);
   nFolder &= ~CSIDL_FLAG_CREATE;

   // In 4.20 CSIDL_DESKTOP became CSIDL_DESKTOPDIRECTORY
   if (CSIDL_DESKTOP == nFolder)
   {
      DWORD dwVersion = GetProcessVersion(GetCurrentProcessId());
      if ((4 > HIWORD(dwVersion)) || ((4 == HIWORD(dwVersion)) &&
                                      (20 > LOWORD(dwVersion))))
      {
         nFolder = CSIDL_DESKTOPDIRECTORY;
      }
   }

   switch (nFolder)
   {
      // Virtual Folders
      case CSIDL_BITBUCKET:
         hr = CreateNameSpacePidl(CLSID_CEShellBitBucket, ppidl);
      break;

      case CSIDL_DESKTOP:
         hr = CreateNameSpacePidl(CLSID_CEShellDesktop, ppidl);
      break;

      case CSIDL_DRIVES:
         hr = CreateNameSpacePidl(CLSID_CEShellDrives, ppidl);
      break;

      case CSIDL_NETWORK:
         hr = CreateNameSpacePidl(CLSID_CEShellNetwork, ppidl);
      break;

      // File System Folders
      default:
      {
         WCHAR szPath[MAX_PATH];
         if (SHGetSpecialFolderPath(hwndOwner, szPath, nFolder, fCreate))
         {
            IShellFolder * pFolder = NULL;
            hr = SHGetDesktopFolder(&pFolder);
            if (SUCCEEDED(hr))
            {
               hr = pFolder->ParseDisplayName(hwndOwner, NULL, szPath,
                                              NULL, ppidl, NULL);
               pFolder->Release();
            }
         }
         else
         {
            if (ERROR_INVALID_PARAMETER == GetLastError())
               hr = E_INVALIDARG;
            else
               hr = E_FAIL;
         }
      }
   }

   return hr;
}

SHSTDAPI SHBindToParent(LPCITEMIDLIST pidl, REFIID riid, VOID ** ppv, LPCITEMIDLIST * ppidlLast)
{
   ASSERT(pidl);
   ASSERT(ppv);
   if (!pidl || !ppv)
      return E_INVALIDARG;
   *ppv = NULL;

   HRESULT hr = NOERROR;
   IShellFolder * pDesktop = NULL;
   IShellFolder * pParent = NULL;
   UINT uCount = 0;

   uCount = ILCount(pidl);
   if (0 == uCount)
      return E_FAIL;

   hr = SHGetDesktopFolder(&pDesktop);
   if (FAILED(hr))
      return hr;

   if (1 == uCount)
   {
      hr = pDesktop->QueryInterface(IID_IShellFolder, (VOID **) &pParent);
   }
   else
   {
      LPCITEMIDLIST pidlParent = ILCopy(pidl, uCount-1);
      if (pidlParent)
      {
         hr = pDesktop->BindToObject(pidlParent, NULL, IID_IShellFolder, (VOID **) &pParent);
         ILFree(pidlParent);
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
   }

   pDesktop->Release();

   if (FAILED(hr))
      return hr;

   ASSERT(pParent);
   hr = pParent->QueryInterface(riid, ppv);
   pParent->Release();

   if (SUCCEEDED(hr) && ppidlLast)
   {
      *ppidlLast = ILCopy(ILFindLast(pidl), 1);
      if (!*ppidlLast)
      {
         ((IUnknown *)(*ppv))->Release();
         *ppv = NULL;
         hr = E_OUTOFMEMORY;
      }
   }

   return hr;
}

SHSTDAPI_(BOOL) SHGetPathFromIDList(LPCITEMIDLIST pidl, LPTSTR pszPath)
{
   ASSERT(pidl);
   ASSERT(pszPath);
   if (!pidl || !pszPath)
      return FALSE;

   HRESULT hr = NOERROR;
   IShellFolder * psf = NULL;
   LPCITEMIDLIST pidlLast = NULL;

   hr = SHBindToParent(pidl, IID_IShellFolder, (VOID **) &psf, &pidlLast);
   if (SUCCEEDED(hr))
   {
      if (ILIsNameSpace(pidlLast, CLSID_CEShellDrives))
      {
         // This is a special case for My Computer since it is the same as "\"
         ::wcscpy(pszPath, L"\\");
      }
      else
      {
         STRRET str;
         hr = psf->GetDisplayNameOf(pidlLast, SHGDN_NORMAL | SHGDN_FORPARSING, &str);
         if (SUCCEEDED(hr))
             hr = StrRetToBuf(&str, pidlLast, pszPath, MAX_PATH);

         if (SUCCEEDED(hr))
         {
            DWORD dwAttributes = SFGAO_FILESYSTEM;
            hr = psf->GetAttributesOf(1, (LPCITEMIDLIST *)&pidlLast, &dwAttributes);
            if (SUCCEEDED(hr) && !(dwAttributes & SFGAO_FILESYSTEM))
            {
               hr = E_FAIL;    // not a file system guy, slam it
               *pszPath = 0;
            }
         }
      }

      ILFree(pidlLast);
      psf->Release();
   }

   return SUCCEEDED(hr);
}

LWSTDAPI StrRetToBuf(STRRET * psr, LPCITEMIDLIST pidl, LPWSTR pszBuf, UINT cchBuf)
{
   ASSERT(psr);
   ASSERT(pszBuf);
   if (!psr || !pszBuf)
      return E_INVALIDARG;

   HRESULT hr = E_FAIL;
   ASSERT(STRRET_OFFSET != psr->uType);

   switch (psr->uType)
   {
      case STRRET_WSTR:
      {
         if (psr->pOleStr)
         {
            IMalloc * pShellMalloc = NULL;

            hr = SHGetMalloc(&pShellMalloc);

            if (SUCCEEDED(hr))
            {
               hr = StringCchCopy(pszBuf, cchBuf, psr->pOleStr);

               pShellMalloc->Free(psr->pOleStr);
               pShellMalloc->Release();

               // Make sure no one thinks things are allocated still
               psr->uType = STRRET_CSTR;
               psr->cStr[0] = 0;
            }
         }
      }
      break;

      case STRRET_CSTR:
      {
         mbstowcs(pszBuf, psr->cStr, cchBuf);
         hr = NOERROR;
      }
      break;
   }

   if (FAILED(hr) && cchBuf)
      *pszBuf = 0;

   return hr;
}

DWORD WINAPI SHGetFileInfo(LPCTSTR pszPath,
                           DWORD dwFileAttributes,
                           SHFILEINFO * psfi,
                           UINT cbFileInfo,
                           UINT uFlags)
{
   HRESULT hr = S_OK;
   IShellFolder * pFolder = NULL;
   LPCITEMIDLIST pidl = NULL;
   UINT uSupportedFlags = (
      SHGFI_ATTRIBUTES |
      SHGFI_DISPLAYNAME |
      SHGFI_ICON |
      SHGFI_LARGEICON |
      SHGFI_SMALLICON |
      SHGFI_SYSICONINDEX |
      SHGFI_TYPENAME |
      SHGFI_USEFILEATTRIBUTES);

   // Unsupported flags
   // SHGFI_ATTR_SPECIFIED *
   // SHGFI_ADDOVERLAYS
   // SHGFI_EXETYPE
   // SHGFI_ICONLOCATION
   // SHGFI_LINKOVERLAY *
   // SHGFI_OPENICON
   // SHGFI_OVERLAYINDEX
   // SHGFI_PIDL
   // SHGFI_SELECTED
   // SHGFI_SHELLICONSIZE

   // Parameter checking
   if (!pszPath || !psfi || (sizeof(SHFILEINFO) != cbFileInfo))
   {
      ASSERT(0);
      ::SetLastError(ERROR_INVALID_PARAMETER);
      return 0;
   }

   // Check for CE supported subset of flags
   if (uFlags & (~uSupportedFlags))
   {
      ::SetLastError(ERROR_INVALID_FLAGS);
      return 0;
   }

   // Check for flag collisions
   if ((SHGFI_ATTRIBUTES & uFlags) && (SHGFI_USEFILEATTRIBUTES & uFlags))
   {
      ::SetLastError(ERROR_INVALID_FLAGS);
      return 0;
   }

   if (((SHGFI_LARGEICON | /*SHGFI_LINKOVERLAY |*/ SHGFI_SMALLICON) & uFlags) &&
       !((SHGFI_ICON | SHGFI_SYSICONINDEX) & uFlags))
   {
      ::SetLastError(ERROR_INVALID_FLAGS);
      return 0;
   }

   // Build the system cache if it doesn't already exist
   if (!CIconCache::pIconCache)
   {
      CIconCache * pIconCache = new CIconCache();
      if (!pIconCache || !pIconCache->RebuildSystemImageList())
      {
         ::SetLastError(ERROR_OUTOFMEMORY);
         return 0;
      }

      if (0 != ::InterlockedTestExchange((LPLONG) &CIconCache::pIconCache,
                                         0, (DWORD)pIconCache))
      {
         delete pIconCache; // Already set by another thread
      }
   }

   // Get the file attributes
   if (!(SHGFI_USEFILEATTRIBUTES & uFlags))
   {
      dwFileAttributes = ::GetFileAttributes(pszPath);
      if (-1 == dwFileAttributes)
      {
         if (FAILED(PathIsGUID(pszPath)))
         {
            DWORD err = ::GetLastError();
            if (((TEXT('\\') == pszPath[0]) && (TEXT('\\') == pszPath[1])) &&
                ((ERROR_ACCESS_DENIED == err) || (ERROR_BAD_NETPATH == err) ||
                 (ERROR_CANCELLED == err) || (ERROR_NO_NETWORK == err)))
            {
               // It's probably a non-accessable net path
               dwFileAttributes = (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_TEMPORARY);
               if (SHGFI_ATTRIBUTES & uFlags)
               {
                  psfi->dwAttributes = (SFGAO_FILESYSTEM | SFGAO_FOLDER);
                  uFlags &= ~SHGFI_ATTRIBUTES; // Try not to touch the item again
               }

               // Should we block display name access here?
            }
            else
            {
               // Unknown file
               ::SetLastError(ERROR_MOD_NOT_FOUND);
               return 0;
            }
         }
         else
         {
            dwFileAttributes = 0;
         }
      }
   }

   // Get the type
   if (SHGFI_TYPENAME & uFlags)
   {
      CIconCache::pIconCache->GetType(pszPath, dwFileAttributes,
                                      psfi->szTypeName, lengthof(psfi->szTypeName));
   }

   // Get the icon
   if ((SHGFI_ICON | SHGFI_SYSICONINDEX) & uFlags)
   {
      psfi->iIcon = CIconCache::pIconCache->GetImageIndex(pszPath, dwFileAttributes, FALSE);

      if (SHGFI_ICON & uFlags)
      {
         HIMAGELIST himl = NULL;
         int iFlags = 0;

         if (uFlags & SHGFI_SMALLICON)
            himl = CIconCache::pIconCache->GetSystemImageList(SHGFI_SMALLICON);
         else
            himl = CIconCache::pIconCache->GetSystemImageList(SHGFI_LARGEICON);

         if (PathIsLink(pszPath)) // || (SHGFI_LINKOVERLAY & uFlags))
            iFlags = INDEXTOOVERLAYMASK(1);

         psfi->hIcon = ImageList_GetIcon(himl, psfi->iIcon, iFlags);
      }
   }

   // Get the item pidl and the shell folder (needed for attributes and display name)
   if ((SHGFI_ATTRIBUTES | SHGFI_DISPLAYNAME) & uFlags)
   {
      IShellFolder * pDesktopFolder = NULL;
      hr = SHGetDesktopFolder(&pDesktopFolder);
      if (SUCCEEDED(hr))
      {
         LPITEMIDLIST pidlPath = NULL;
         hr = pDesktopFolder->ParseDisplayName(NULL, NULL, (LPTSTR) pszPath,
                                               NULL, &pidlPath, NULL);
         if (SUCCEEDED(hr))
         {
            hr = SHBindToParent(pidlPath, IID_IShellFolder,
                                (VOID**) &pFolder, &pidl);
            ILFree(pidlPath);
         }

         pDesktopFolder->Release();
      }

      if (FAILED(hr))
      {
         if (FACILITY_WIN32 == HRESULT_FACILITY(hr))
            ::SetLastError(HRESULT_CODE(hr));
         else
            ::SetLastError(ERROR_MOD_NOT_FOUND); // This will have to do
         return 0;
      }
   }

   // Get the SHGAO attributes if requested
   if (SHGFI_ATTRIBUTES & uFlags)
   {
      ULONG flags = (ULONG) -1;
      PREFAST_ASSERT(pFolder);
      hr = pFolder->GetAttributesOf(1, &pidl, &flags);
      if (SUCCEEDED(hr))
         psfi->dwAttributes = flags;
   }

   // Get the display name
   if (SHGFI_DISPLAYNAME & uFlags)
   {
      STRRET str = {0};
      PREFAST_ASSERT(pFolder);
      hr = pFolder->GetDisplayNameOf(pidl, SHGDN_INFOLDER | SHGDN_FORADDRESSBAR, &str);
      if (SUCCEEDED(hr))
         hr = StrRetToBuf(&str, pidl, psfi->szDisplayName, lengthof(psfi->szDisplayName));
   }

   // These are no longer needed
   if (pFolder)
   {
      pFolder->Release();
      pFolder = NULL;
   }
   if (pidl)
   {
      ILFree(pidl);
      pidl = NULL;
   }

   if (FAILED(hr)) // Attributes or display name failed
   {
      if (FACILITY_WIN32 == HRESULT_FACILITY(hr))
         ::SetLastError(HRESULT_CODE(hr));
      else
         ::SetLastError(ERROR_MOD_NOT_FOUND); // This will have to do
      return 0;
   }

   // Get the correct return depending on the flags
   // NOTE: XP only gets the image list for SHGFI_SYSICONINDEX
   if ((SHGFI_ICON | SHGFI_SYSICONINDEX) & uFlags)
   {
      if (SHGFI_SMALLICON & uFlags)
         return (DWORD) CIconCache::pIconCache->GetSystemImageList(SHGFI_SMALLICON);
      else
         return (DWORD) CIconCache::pIconCache->GetSystemImageList(SHGFI_LARGEICON);
   }
   else
   {
      return 1;
   }
}

int WINAPI SHShowOutOfMemory(HWND hwndOwner, UINT grfFlags)
{
   if (WAIT_OBJECT_0 != WaitForAPIReady(SH_WMGR,0))
      return IDOK;

   return ::MessageBox(hwndOwner, LOAD_STRING(IDS_OOM), LOAD_STRING(IDS_OOM_TITLE),
                       MB_ICONSTOP | MB_OK | MB_SETFOREGROUND);
}


/*++
Routine Description:
    "Binds" a pidl to the object it identifies.
    This function is basically a wrapper for IShellFolder::BindToObject

Arguments:
    psf     IShellFolderInterface (if this is NULL, we create one)
    riid    refiidof the interface
    pidl    PIDl or the object we want to bind
    ppvOut  variable to receive the interface pointer (a shellfolder interface)

Return Value:
    S_OK if successful.  Otherwise an HRESULT identifying the error.

--*/
STDAPI SHBindToObject (IShellFolder *psf, REFIID riid, LPCITEMIDLIST pidl, void **ppvOut)
{
    HRESULT hr = E_FAIL;
    IShellFolder *psfRelease = NULL;

    if (!psf)
    {
        hr = SHGetDesktopFolder(&psf);
        if (SUCCEEDED(hr) && psf)
        {
            psfRelease = psf;
        }
        else
        {
            ASSERT(psf);
            return hr;
        }
    }
    else
    {
        psfRelease = NULL;
    }

    if (ILIsEmpty(pidl))
    {
        hr = psf->QueryInterface(riid, ppvOut);
    }
    else
    {
        hr = psf->BindToObject(pidl, NULL, riid, ppvOut);
    }

    if (psfRelease)
    {
        psfRelease->Release();
    }

    if (SUCCEEDED(hr) && (*ppvOut == NULL))
    {
        //Some lame shell extensions (eg WS_FTP on desktop) will return success and a null out pointer
        DEBUGMSG(ZONE_WARNING, (_T("SHBindToObject: BindToObject succeeded but returned null ppvOut!!")));
    }

    return hr;
}

void WINAPI SHAddToRecentDocs(UINT uFlags, LPCVOID pv)
{
    WCHAR szRecentDoc[MAX_PATH];

    if (!SHGetSpecialFolderPath(NULL, szRecentDoc, CSIDL_RECENT, TRUE))
    {
        return;
    }

    if (!pv)
    {
        LPWSTR pszEnd = NULL;
        if (SUCCEEDED(StringCbCatEx(szRecentDoc, sizeof(szRecentDoc)-sizeof(WCHAR),
                        TEXT("\\*.*"), &pszEnd, NULL, 0)))
        {
            *(++pszEnd) = TEXT('\0'); // Double NULL terminate for SHFileOperation

            SHFILEOPSTRUCT shfos = {0};
            shfos.wFunc = FO_DELETE;
            shfos.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI;
            shfos.pFrom = szRecentDoc;
            SHFileOperation(&shfos);
        }
    }
    else
    {
        if ((SHARD_PATH == uFlags) || (SHARD_PIDL == uFlags))
        {
            BOOL fValidString = TRUE;
            WCHAR szPath[MAX_PATH+2] = TEXT("\""); // +2 for the ""

            if (SHARD_PATH == uFlags)
            {
                fValidString = SUCCEEDED(StringCbCat(szPath, sizeof(szPath), (LPCWSTR)pv));
            }
            else
            {
                fValidString = SHGetPathFromIDList((LPCITEMIDLIST) pv, szPath+1);
            }

            if (fValidString)
            {
                if (SUCCEEDED(StringCbCat(szRecentDoc, sizeof(szRecentDoc), TEXT("\\"))) &&
                    SUCCEEDED(StringCbCat(szRecentDoc, sizeof(szRecentDoc), PathFindFileName(szPath))))
                {
                    PathRemoveExtension(szRecentDoc);
                    if (SUCCEEDED(StringCbCat(szRecentDoc, sizeof(szRecentDoc), TEXT(".lnk"))) &&
                        SUCCEEDED(StringCbCat(szPath, sizeof(szPath), TEXT("\""))))
                    {
                        SHCreateShortcut(szRecentDoc, szPath);
                    }
                }
            }
        }
    }
    // TODO: delete some recent docs
}


