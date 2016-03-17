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

#include "idlist.h"
#include "api.h"
#include "filechangemgr.h"
#include "iniutil.h"
#include "guid.h"

extern NameSpacePidlCache * g_pNameSpacePidlCache;

LPITEMIDLIST ILCreate(LPCWSTR pwszParsingName, LPCWSTR pwszDisplayName, WORD flags)
{
   return ILCreateWithCache(pwszParsingName, pwszDisplayName, flags, NULL, NULL);
}

LPITEMIDLIST ILCreateWithCache(LPCWSTR pwszParsingName, LPCWSTR pwszDisplayName, WORD flags,
                               LPCWSTR pwszFilePath, WIN32_FIND_DATA * pfd)
{
   LPITEMIDLIST pidl = NULL;
   size_t cchParsing = 0;
   size_t cchDisplay = 0;

   if (SUCCEEDED(StringCchLength(pwszParsingName, MAX_PATH, &cchParsing)))
   {
      size_t cb = 0;
      size_t offset = 0;
      size_t cacheOffset = 0;

      if ((pwszParsingName == pwszDisplayName) ||
          FAILED(StringCchLength(pwszDisplayName, MAX_PATH, &cchDisplay)))
      {
         pwszDisplayName = NULL;
      } 

      cb += sizeof(pidl->mkid.cb);
      cb += sizeof(WORD); // flags
      ASSERT(0 == cb%4); // Verify that the address starts DWORD aligned
      cb += sizeof(WORD); // offset of DisplayName from the beginning of the pidl
      cb += sizeof(WORD); // offset of Cache from the beginning of the pidl
      ASSERT(0 == cb%4); // Verify that the string starts DWORD aligned
      offset = cb;
      cb += ((cchParsing+1)*sizeof(WCHAR)); // The ParsingName
      cb += ((0 < cb%4) ? (4-(cb%4)) : 0); // DWORD align the DisplayName
      ASSERT(0 == cb%4); // Verify that the string starts DWORD aligned
      if (pwszDisplayName)
      {
         offset = cb;
         cb += ((cchDisplay+1)*sizeof(WCHAR)); // The DisplayName
         cb += ((0 < cb%4) ? (4-(cb%4)) : 0); // DWORD align the DisplayName
         ASSERT(0 == cb%4); // Verify that the item ends DWORD aligned
      }
      cacheOffset = cb;

      SHFILEINFO shfi = {0};               
      size_t cchType = 0;
      if(pfd)
      {
         cb += sizeof(FILETIME); // The Date
         ASSERT(0 == cb%4); // Verify that the address is DWORD aligned
         if (!((CEIDLIST_FILESYSTEM & flags) && (CEIDLIST_FS_DIRECTORY & flags)))
         {
            cb += sizeof(ULARGE_INTEGER); // The Size
            ASSERT(0 == cb%4); // Verify that the string starts DWORD aligned
            if (pwszFilePath && SHGetFileInfo(pwszFilePath, 0, &shfi, sizeof(shfi), SHGFI_TYPENAME))
            {
               if (SUCCEEDED(StringCchLength(shfi.szTypeName, lengthof(shfi.szTypeName), &cchType)))
               {
                  cb += ((cchType+1)*sizeof(WCHAR)); // The Type
                  cb += ((0 < cb%4) ? (4-(cb%4)) : 0); // DWORD align the Type
                  ASSERT(0 == cb%4); // Verify that the item ends DWORD aligned
               }
            }
         }
      }

      LPBYTE p = (LPBYTE) LocalAlloc(LPTR, cb+2); // 2 byte NULL ends the pidl
      if (p)
      {
         flags &= ~CEIDLIST_FS_CACHEMASK;

         IL_CB(p) = (USHORT) cb;
         IL_SET_DISPLAYNAME_OFFSET(p,offset);
         IL_SET_CACHE_OFFSET(p,cacheOffset);

         wcscpy(IL_PARSINGNAME(p), pwszParsingName);
         if (pwszDisplayName)
            wcscpy(IL_DISPLAYNAME(p), pwszDisplayName);

         if(pfd)
         {
            memcpy(IL_DATECACHED(p), &pfd->ftLastWriteTime, sizeof(FILETIME));
            flags |= CEIDLIST_FS_DATECACHED;

            if (!((CEIDLIST_FILESYSTEM & flags) && (CEIDLIST_FS_DIRECTORY & flags)))
            {
               ULARGE_INTEGER uliSize ;
               uliSize.HighPart = pfd->nFileSizeHigh;
               uliSize.LowPart = pfd->nFileSizeLow;
               memcpy(IL_SIZECACHED(p), &uliSize, sizeof(ULARGE_INTEGER));
               flags |= CEIDLIST_FS_SIZECACHED;

               if(cchType)
               {
                  wcscpy(IL_TYPECACHED(p), shfi.szTypeName);
                  flags |= CEIDLIST_FS_TYPECACHED;
               }
            }
         }
         IL_SET_FLAGS(p,flags);

         pidl = (LPITEMIDLIST) p;
      }
   }

   return pidl;
}

HRESULT CreateFileSystemPidl(LPCWSTR pwszPathPart, LPITEMIDLIST * ppidl)
{
   ASSERT(pwszPathPart);
   ASSERT(ppidl);
   if (!pwszPathPart || !ppidl)
      return E_INVALIDARG;

   HRESULT hr = NOERROR;

   *ppidl = ILCreate(pwszPathPart, NULL, CEIDLIST_FILESYSTEM);
   if (!*ppidl)
      hr = E_OUTOFMEMORY;

   return hr;
}

HRESULT CreateNameSpacePidl(REFCLSID rclsid, LPITEMIDLIST * ppidl)
{
   ASSERT(ppidl);
   if (!ppidl)
      return E_INVALIDARG;

   HRESULT hr = NOERROR;
   LPOLESTR str = NULL;

   *ppidl = NULL;

   if (NULL != g_pNameSpacePidlCache)
   {
      // Try to find the item in the cache
      *ppidl = const_cast<LPITEMIDLIST>(g_pNameSpacePidlCache->Get(rclsid));
   }

   if (NULL == *ppidl)
   {
      hr = StringFromCLSID(rclsid, &str);
      if (SUCCEEDED(hr))
      {
         LONG lResult = ERROR_SUCCESS;
         HKEY hKey = NULL;
         WCHAR sz[MAX_PATH] = TEXT("CLSID\\");
         DWORD cb = sizeof(sz);
         LPWSTR pwszDisplayName = NULL;

         hr = StringCchCat(sz, lengthof(sz), str);
         if (SUCCEEDED(hr))
         {
            lResult = RegOpenKeyEx(HKEY_CLASSES_ROOT, sz, 0, 0, &hKey);
            if (ERROR_SUCCESS == lResult)
            {
               lResult = RegQueryValueEx(hKey, TEXT("DisplayName"),
                                         NULL, NULL, (LPBYTE) sz, &cb);

               if (ERROR_SUCCESS == lResult)
                  pwszDisplayName = sz;

               RegCloseKey(hKey);
            }

            *ppidl = ILCreate(str, pwszDisplayName, CEIDLIST_NAMESPACE);
            if (NULL != *ppidl)
            {
               if (NULL != g_pNameSpacePidlCache)
               {
                  // Cache the namespace item
                  g_pNameSpacePidlCache->Cache(rclsid, *ppidl);
               }
            }
            else
            {
               hr = E_OUTOFMEMORY;
            }
         }
         CoTaskMemFree(str);
      }
   }

   return hr;
}

HRESULT RenameNamespacePidl(LPCWSTR pwszNewDisplayName, LPITEMIDLIST * ppidl)
{
   ASSERT(pwszNewDisplayName);
   ASSERT(ppidl);
   if (!pwszNewDisplayName || !ppidl)
      return E_INVALIDARG;

   HRESULT hr = NOERROR;
   WCHAR sz[MAX_PATH] = TEXT("CLSID\\");
   size_t cb = 0;

   hr = StringCbLength(pwszNewDisplayName, sizeof(sz), &cb);
   if (SUCCEEDED(hr))
   {
      cb += sizeof(WCHAR); // RegSetValueEx requires that the '\0' is accounted for

      hr = StringCchCat(sz, lengthof(sz), IL_PARSINGNAME(*ppidl));
      if (SUCCEEDED(hr))
      {
         HKEY hKey = NULL;
         LONG lResult = RegOpenKeyEx(HKEY_CLASSES_ROOT, sz, 0, 0, &hKey);
         if (ERROR_SUCCESS == lResult)
         {
            lResult = RegSetValueEx(hKey, TEXT("DisplayName"), NULL,
                                    REG_SZ, (LPBYTE) pwszNewDisplayName, cb);
            if (ERROR_SUCCESS == lResult)
            {
               LPITEMIDLIST pidl = ILCreate(IL_PARSINGNAME(*ppidl),
                                            pwszNewDisplayName,
                                            CEIDLIST_NAMESPACE);
               if (pidl)
               {
                  if (NULL != g_pNameSpacePidlCache)
                  {
                     // Update the cache
                     g_pNameSpacePidlCache->Update(pidl);
                  }

                  ILFree(*ppidl);
                  *ppidl = pidl;
               }
               else
               {
                  hr = E_OUTOFMEMORY;
               }
            }
            else
            {
               hr = HRESULT_FROM_WIN32_I(GetLastError());
            }

            RegCloseKey(hKey);
         }
         else
         {
            hr = HRESULT_FROM_WIN32_I(GetLastError());
         }
      }
   }

   if (SUCCEEDED(hr))
      g_pFileChangeManager->RefreshAllViews();

   return hr;
}

HRESULT MakeDesktopIniString(__inout_ecount(MAX_PATH) LPTSTR psz)
{
    HRESULT hr;
    size_t cch;

    ASSERT(NULL != psz); // Internal function, we can be trust the input

    hr = StringCchLength(psz, MAX_PATH, &cch);
    if (SUCCEEDED(hr))
    {
        if ((0 == cch) || (TEXT('\\') != psz[cch-1]))
        {
            hr = StringCchCat(psz, MAX_PATH, TEXT("\\desktop.ini"));
        }
        else
        {
            hr = StringCchCat(psz, MAX_PATH, TEXT("desktop.ini"));
        }
    }

    return hr;
}

HRESULT UpdateFileSystemPidl(LPCITEMIDLIST pidlParent, LPITEMIDLIST * ppidl)
{
   ASSERT(pidlParent);
   ASSERT(ppidl);
   if (!pidlParent || !ppidl)
      return E_INVALIDARG;

   ASSERT(*ppidl);
   ASSERT(!ILIsEmpty(*ppidl));
   ASSERT(ILIsFileSystemPidl(*ppidl));
   ASSERT(1 == ILCount(*ppidl));
   if (ILIsEmpty(*ppidl) || !ILIsFileSystemPidl(*ppidl) || 1 != ILCount(*ppidl))
      return E_INVALIDARG;

   HRESULT hr = NOERROR;
   LPITEMIDLIST pidlFQ = NULL;
   WCHAR wszPath[MAX_PATH];

   pidlFQ = ILConcatenate(pidlParent, *ppidl);
   if (pidlFQ)
   {
      if (SHGetPathFromIDList(pidlFQ, wszPath))
      {
         WIN32_FIND_DATA fd = {0};
         HANDLE hFind = ::FindFirstFile(wszPath, &fd);
         if (INVALID_HANDLE_VALUE != hFind)
         {
            LPCWSTR pwszPath = wszPath;
            LPCWSTR pwszParsingName = IL_PARSINGNAME(*ppidl);
            LPCWSTR pwszDisplayName = IL_DISPLAYNAME(*ppidl);
            WORD flags = IL_GET_FLAGS(*ppidl);
            WCHAR wsz[MAX_PATH];

            if (FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes)
            {
               // Cache the directory attribute
               flags |= CEIDLIST_FS_DIRECTORY;

               // This will be used to get the type info, but folders don't need it
               pwszPath = NULL;

               // Set the "display name" if a desktop.ini exists
               _tcscpy(wsz, wszPath); // safe since we control wszPath
               hr = MakeDesktopIniString(wsz);
               if (SUCCEEDED(hr))
               {
                  if (ReadIni(L"[.ShellClassInfo]", L"LocalizedResourceName",
                              wsz, lengthof(wsz), wsz) &&
                      SUCCEEDED(SHLoadIndirectString(wsz, wsz,
                                                     lengthof(wsz),
                                                     NULL)) &&
                      0 != ::wcscmp(pwszDisplayName, wsz))
                  {
                     pwszDisplayName = wsz;
                  }
               }
            }
            else
            {
               if (PathIsLink(wszPath))
               {
                  // This is a link file
                  flags |= CEIDLIST_FS_LINK;

                  if (SHGetPathFromIDList(pidlParent, wsz))
                  {
                     // Set the "display name" if a desktop.ini exists
                     hr = MakeDesktopIniString(wsz);
                     if (SUCCEEDED(hr))
                     {
                        if (ReadIni(L"[LocalizedFileNames]", pwszParsingName,
                                    wsz, lengthof(wsz), wsz) &&
                            SUCCEEDED(SHLoadIndirectString(wsz, wsz,
                                                           lengthof(wsz),
                                                           NULL)))
                        {
                           pwszDisplayName = wsz;
                        }
                     }
                  }
               }
            }

            // Cache the details
            ASSERT(!(CEIDLIST_FS_CACHEMASK & flags));
            if(!(CEIDLIST_FS_CACHEMASK & flags))
            {
               LPITEMIDLIST pidl = ILCreateWithCache(pwszParsingName, 
                                                     pwszDisplayName,
                                                     flags, pwszPath, &fd);
               if(pidl)
               {
                  ILFree(*ppidl);
                  *ppidl = pidl;
               }
               else
               {
                  hr = E_OUTOFMEMORY;
               }
            }

            ::FindClose(hFind);
         }
         else
         {
            hr = E_FAIL;
         }
      }
      else
      {
         hr = E_FAIL;
      }
      ILFree(pidlFQ);
   }
   else
   {
      hr = E_OUTOFMEMORY;
   }

   return hr;
}


NameSpacePidlCache::NameSpacePidlCache()
{
    rgCache[DESKTOP].clsid = CLSID_CEShellDesktop;
	rgCache[DESKTOP].pidl = NULL;
    rgCache[DRIVES].clsid = CLSID_CEShellDrives;
	rgCache[DRIVES].pidl = NULL;
    rgCache[BITBUCKET].clsid = CLSID_CEShellBitBucket;
	rgCache[BITBUCKET].pidl = NULL;
    rgCache[NETWORK].clsid = CLSID_CEShellNetwork;
	rgCache[NETWORK].pidl = NULL;
}

NameSpacePidlCache::~NameSpacePidlCache()
{
    for (int i = 0; i < CACHE_COUNT; i++)
    {
        if (NULL != rgCache[i].pidl)
        {
            ILFree(rgCache[i].pidl);
            rgCache[i].pidl = NULL;
        }
    }
}

LPCITEMIDLIST NameSpacePidlCache::Get(REFCLSID rclsid)
{
    LPCITEMIDLIST pidlRet = NULL;

    for (int i = 0; i < CACHE_COUNT; i++)
    {
        if (rclsid == rgCache[i].clsid)
        {
            if (NULL != rgCache[i].pidl)
            {
                pidlRet = ILCopy(rgCache[i].pidl, IL_ALL);
            }

            break;
        }
    }

    return pidlRet;
}

void NameSpacePidlCache::Cache(REFCLSID rclsid, LPCITEMIDLIST pidl)
{
    for (int i = 0; i < CACHE_COUNT; i++)
    {
        if (rclsid == rgCache[i].clsid)
        {
            if (NULL != rgCache[i].pidl)
            {
                ILFree(rgCache[i].pidl);
            }

            rgCache[i].pidl = ILCopy(pidl, IL_ALL);
            break;
        }
    }
}

void NameSpacePidlCache::Update(LPCITEMIDLIST pidl)
{
    for (int i = 0; i < CACHE_COUNT; i++)
    {
        if (ILIsEqualParsingNames(pidl, rgCache[i].pidl))
        {
            ILFree(rgCache[i].pidl);
            rgCache[i].pidl = ILCopy(pidl, IL_ALL);
            break;
        }
    }
}
