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

#include "iconcache.h"
#undef SHCNF_PATH // extfile.h redefines this
#include <extfile.h>

const DWORD CIconCache::HIBERNATE_FREEZE_CACHE  = 0x10000; //  64KB
const DWORD CIconCache::LCID_ENGLISH = MAKELCID(MAKELANGID(LANG_ENGLISH,
                                                           SUBLANG_ENGLISH_US),
                                                SORT_DEFAULT);
CIconCache * CIconCache::pIconCache = NULL;

CIconCache::CacheInfo::CacheInfo() :
   m_pszItem(NULL),
   m_iImageIndex(-1),
   m_iImageAltIndex(-1),
   m_fFreeType(FALSE),
   m_pszType(NULL)
{
}

CIconCache::CacheInfo::~CacheInfo()
{
   if (m_pszItem)
   {
      delete [] m_pszItem;
   }

   if (m_fFreeType && m_pszType)
   {
      delete [] m_pszType;
   }
};

CIconCache::CIconCache()
{
   m_himlSmall = ImageList_Create(16, 16, TRUE, 1, 0);
   m_himlLarge = ImageList_Create(32, 32, TRUE, 1, 0); 
}

CIconCache::~CIconCache()
{
   CacheInfo * pCacheInfo = NULL;
   CacheInfo * pCacheInfoNext = NULL;

   m_list.Lock(TRUE);
   pCacheInfo = (CacheInfo *) m_list.Head();
   while (pCacheInfo)
   {
      pCacheInfoNext = (CacheInfo *) m_list.Next(pCacheInfo);
      m_list.Disconnect(pCacheInfo);
      delete pCacheInfo;
      pCacheInfo = pCacheInfoNext;
   }
   m_list.Lock(FALSE);

   if (m_himlSmall)
   {
      ImageList_Destroy(m_himlSmall);
   }
 
   if (m_himlLarge)
   {
      ImageList_Destroy(m_himlLarge);
   }
}

CIconCache::CacheInfo * CIconCache::CacheItem(LPCWSTR pszFilePath)
{
   ASSERT(pszFilePath);
   if (!pszFilePath)
   {
      return NULL;
   }

   CacheInfo * pCacheInfo = new CacheInfo();
   LPCWSTR pszFileName = PathFindFileName(pszFilePath);
   HICON hiconSmall = NULL;
   HICON hiconLarge = NULL;
   HICON hiconAltSmall = NULL;
   HICON hiconAltLarge = NULL;
   HKEY hKey = NULL;
   size_t cch;
   PREFAST_ASSERT(pszFileName); // Will never be NULL (see PathFindFileName)

   if (!pCacheInfo)
   {
      return NULL;
   }

   // We will never cache directories
   ASSERT(!PathIsDirectory(pszFilePath));

   if (SUCCEEDED(PathIsGUID(pszFileName))) // It's a namespace path
   {
      // ITEM: Copy the guid into the item variable
      cch = _tcslen(pszFileName)+1;
      pCacheInfo->m_pszItem = new WCHAR[cch];
      if (pCacheInfo->m_pszItem)
      {
         StringCchCopy(pCacheInfo->m_pszItem, cch, pszFileName);
      }

      // TYPE: Pull the type information from the resources
      pCacheInfo->m_pszType = LOAD_STRING(IDS_FILETYPE_SYSTEM),
      pCacheInfo->m_fFreeType = FALSE;

      // ICON: Get the requested icons
      WCHAR sz[MAX_PATH] = TEXT("CLSID\\");
      if (SUCCEEDED(::StringCchCat(sz, lengthof(sz), pszFileName)))
      {
         LONG lResult = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, sz, 0, 0, &hKey);
         if (ERROR_SUCCESS == lResult)
         {
            DWORD cb = sizeof(sz);
            lResult = ::RegQueryValueEx(hKey, TEXT("DefaultIcon"),
                                        NULL, NULL, (LPBYTE) sz, &cb);
            if (ERROR_SUCCESS == lResult)
            {
               IExtractIcon(sz, &hiconSmall, &hiconLarge);
            }

            cb = sizeof(sz);
            lResult = ::RegQueryValueEx(hKey, TEXT("AltIcon"),
                                        NULL, NULL, (LPBYTE) sz, &cb);
            if (ERROR_SUCCESS == lResult)
            {
               IExtractIcon(sz, &hiconAltSmall, &hiconAltLarge);
            }

            ::RegCloseKey(hKey);
         }
      }
   }
   else // It's a file system file
   {
      if (PathIsExe(pszFileName))
      {
         // Exes are treated differently

         // ITEM: Each exe had it's own icon, since it is pulled from the binary
         cch = _tcslen(pszFilePath)+1;
         pCacheInfo->m_pszItem = new WCHAR[cch];
         if (pCacheInfo->m_pszItem)
         {
            StringCchCopy(pCacheInfo->m_pszItem, cch, pszFilePath);
         }

         // TYPE: Pull the type information from the resources
         pCacheInfo->m_pszType = LOAD_STRING(IDS_FILETYPE_APPLICATION),
         pCacheInfo->m_fFreeType = FALSE;

         // ICON: Get the icon from the binary
         ::ExtractIconEx(pszFilePath, 0, &hiconLarge, &hiconSmall, 1);
      }
      else
      {
         LPCWSTR pszExtension = PathFindExtension(pszFileName);
         WCHAR szExtensionLookup[256] = TEXT("");
         BOOL fHasExtension = FALSE;

         if ((NULL != pszExtension) && (_T('\0') != *pszExtension))
         {
            ASSERT(_T('.') == *pszExtension);

            // If the character after the . is not null, this has an extension
            fHasExtension = (_T('\0') != pszExtension[1]);
         }

         if (fHasExtension)
         {
            // ITEM: The item identifier is just the extension
            cch = _tcslen(pszExtension)+1;
            pCacheInfo->m_pszItem = new WCHAR[cch];
            if (pCacheInfo->m_pszItem)
            {
               StringCchCopy(pCacheInfo->m_pszItem, cch, pszExtension);
            }

            // This will convert .foo into foofile, it is needed for type and icon
            LONG lResult = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, pszExtension, 0, 0, &hKey);
            if (ERROR_SUCCESS == lResult)
            {
               DWORD cb = sizeof(szExtensionLookup);
               lResult = ::RegQueryValueEx(hKey, NULL, NULL, NULL,
                                           (LPBYTE) szExtensionLookup, &cb);
               if (ERROR_SUCCESS != lResult)
               {
                  szExtensionLookup[0] = TEXT('\0');
               }

               ::RegCloseKey(hKey);
               hKey = NULL;
            }

            // TYPE: Figure out the type information
            if (TEXT('\0') != szExtensionLookup[0])
            {
               lResult = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, szExtensionLookup, 0, 0, &hKey);
               if (ERROR_SUCCESS == lResult)
               {
                  WCHAR sz[MAX_PATH];
                  DWORD cb = sizeof(sz);
                  lResult = ::RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE) sz, &cb);

                  // NULL terminate the string
                  sz[MAX_PATH-1] = 0;

                  if (ERROR_SUCCESS == lResult)
                  {
                     cch = cb/sizeof(WCHAR);
                     pCacheInfo->m_pszType = new WCHAR[cch];
                     if (pCacheInfo->m_pszType)
                     {
                        StringCchCopy(pCacheInfo->m_pszType, cch, sz);
                     }
                     pCacheInfo->m_fFreeType = TRUE;
                  }
                  else
                  {
                     pCacheInfo->m_pszType = LOAD_STRING(IDS_FILETYPE_UNKNOWN);
                     pCacheInfo->m_fFreeType = FALSE;
                  }

                  ::RegCloseKey(hKey);
               }
               else
               {
                  pCacheInfo->m_pszType = LOAD_STRING(IDS_FILETYPE_UNKNOWN);
                  pCacheInfo->m_fFreeType = FALSE;
               }
            }
            else
            {
               pszExtension++; // move beyond the .
               WCHAR wszLocalExtension[64];
               size_t cchExtension = _tcslen(pszExtension);
               size_t cchFileType = _tcslen(LOAD_STRING(IDS_FILETYPE_FILE))-2; // -2 for the %s

               // Copy extension to local buffer for conversion to uppercase.
               if (SUCCEEDED(::StringCchCopy(wszLocalExtension, lengthof(wszLocalExtension), pszExtension)))
               {
                  ::CharUpperBuff(wszLocalExtension, min(cchExtension, (lengthof(wszLocalExtension) - 1)));
                  pszExtension = wszLocalExtension;
               }

               pCacheInfo->m_pszType = new WCHAR[cchExtension+cchFileType+1];
               if (pCacheInfo->m_pszType)
               {
                  _stprintf(pCacheInfo->m_pszType, LOAD_STRING(IDS_FILETYPE_FILE),
                            pszExtension);
               }
               pCacheInfo->m_fFreeType = TRUE;
            }
         }
         else
         {
            // ITEM: The has no extension, use the full path
            cch = _tcslen(pszFilePath)+1;
            pCacheInfo->m_pszItem = new WCHAR[cch];
            if (pCacheInfo->m_pszItem)
            {
               StringCchCopy(pCacheInfo->m_pszItem, cch, pszFilePath);
            }

            // TYPE: This is just the generic type
            pCacheInfo->m_pszType = LOAD_STRING(IDS_FILETYPE_FILE_GENERIC);
            pCacheInfo->m_fFreeType = FALSE;
         }

         // ICON: Get the requested icons
         if (TEXT('\0') != szExtensionLookup[0])
         {
            WCHAR sz[MAX_PATH];
            DWORD cb = sizeof(sz);

            if (SUCCEEDED(::StringCchPrintf(sz, lengthof(sz), TEXT("%s\\%s"),
                                            szExtensionLookup, TEXT("DefaultIcon"))))
            {
               LONG lResult = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, sz, 0, 0, &hKey);
               if (ERROR_SUCCESS == lResult)
               {
                  cb = sizeof(sz);
                  lResult = ::RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE) sz, &cb);

                  // NULL terminate the string
                  sz[MAX_PATH-1] = 0;

                  if (ERROR_SUCCESS == lResult)
                  {
                     if (0 == _tcscmp(sz, TEXT("%1")))
                     {
                        ::ExtractIconEx(pszFilePath, 0, &hiconLarge, &hiconSmall, 1);
                     }
                     else
                     {
                        IExtractIcon(sz, &hiconSmall, &hiconLarge);
                     }
                  }

                  ::RegCloseKey(hKey);
               }
            }
         }
      }
   }

   if (pCacheInfo->m_pszItem && pCacheInfo->m_pszType)
   {
      if (hiconLarge)
      {
         int iIndexLarge = -1;
         int iIndexSmall = -1;

         iIndexLarge = ImageList_AddIcon(m_himlLarge, hiconLarge);
         if (hiconSmall)
         {
            iIndexSmall = ImageList_AddIcon(m_himlSmall, hiconSmall);
         }
         else
         {
            iIndexSmall = ImageList_AddIcon(m_himlSmall, hiconLarge);
         }
         ASSERT(iIndexLarge == iIndexSmall);
         pCacheInfo->m_iImageIndex = iIndexLarge;

         if (hiconAltLarge)
         {
            iIndexLarge = ImageList_AddIcon(m_himlLarge, hiconAltLarge);
            if (hiconAltSmall)
            {
               iIndexSmall = ImageList_AddIcon(m_himlSmall, hiconAltSmall);
            }
            else
            {
               iIndexSmall = ImageList_AddIcon(m_himlSmall, hiconAltLarge);
            }
            ASSERT(iIndexLarge == iIndexSmall);
            pCacheInfo->m_iImageAltIndex = iIndexLarge;
         }
      }
      else
      {
         pCacheInfo->m_iImageIndex = GetDefaultImageIndex(pszFilePath, 0);
      }
   }
   else
   {
      // Without the itema and type there's no use caching the item
      delete pCacheInfo;
      pCacheInfo = NULL;
   }

   if (hiconSmall)
   {
      ::DestroyIcon(hiconSmall);
   }

   if (hiconLarge)
   {
      ::DestroyIcon(hiconLarge);
   }

   if (hiconAltSmall)
   {
      ::DestroyIcon(hiconAltSmall);
   }

   if (hiconAltLarge)
   {
      ::DestroyIcon(hiconAltLarge);
   }

   return pCacheInfo;
}

int CIconCache::GetDefaultImageIndex(LPCWSTR pszFilePath, DWORD dwAttrib) const
{
   ASSERT(m_himlSmall && (0 < ImageList_GetImageCount(m_himlSmall)));
   ASSERT(m_himlLarge && (0 < ImageList_GetImageCount(m_himlLarge)));

   if (-1 == dwAttrib)
   {
      // Unknown attributes, make a best guess based on the filename
      LPCWSTR pszFileName = PathFindFileName(pszFilePath);
      ASSERT(pszFileName);

      if (SUCCEEDED(PathIsGUID(pszFileName)))
      {
         return FOLDER_ICON; // default folder since it's an unknown namespace
      }
      else
      {
         LPCWSTR pszExtension = PathFindExtension(pszFileName);
         if (pszExtension)
         {
            if (TEXT('\0') != *pszExtension)
            {
               if (PathIsExe(pszExtension))
               {
                  return APPLICATION_ICON; // default executable
               }
               else
               {
                  return DOCUMENT_ICON; // default document
               }
            }
            else
            {
               return FOLDER_ICON; // default folder
            }
         }
         else
         {
            return DOCUMENT_ICON; // default document
         }
      }
   }
   else if (FILE_ATTRIBUTE_DIRECTORY & dwAttrib)
   {
      // Directory, determine if it is a special type or just a regular folder
      if (FILE_ATTRIBUTE_TEMPORARY & dwAttrib)
      {
         CEOIDINFO info = {0};

         if ((L'\\' == pszFilePath[0]) && (L'\\' == pszFilePath[1]))
         {
            return NETWORKFOLDER_ICON; // network share (\\foo)
         }
         else if (CSTR_EQUAL == ::CompareString(LCID_ENGLISH, NORM_IGNORECASE,
                                                TEXT("\\release"), -1,
                                                pszFilePath, -1))
         {
            return NETWORKFOLDER_ICON; // kitl share (\release)
         }
         else if (::CeOidGetInfo(OIDFROMAFS(AFS_ROOTNUM_NETWORK), &info) &&
                  (CSTR_EQUAL == ::CompareString(LOCALE_USER_DEFAULT,
                                                 NORM_IGNORECASE,
                                                 info.infDirectory.szDirName, -1,
                                                 pszFilePath, -1)))
         {
            return NETWORKFOLDER_ICON; // network folder (\network)
         }
         else
         {
            return STORAGECARD_ICON; // storage card
         }
      }
      else
      {
         return FOLDER_ICON; // default folder
      }
   }
   else
   {
      // Either an executable or a document
      if (PathIsExe(pszFilePath))
      {
         return APPLICATION_ICON; // default executable
      }
      else
      {
         return DOCUMENT_ICON; // default document
      }
   }
}

int CIconCache::GetImageIndex(LPCWSTR pszFilePath, DWORD dwAttrib, BOOL fGetAltImageIndex)
{
   ASSERT(pszFilePath);
   if (!pszFilePath || (TEXT('\0') == *pszFilePath))
   {
      return -1;
   }

   int iIndex = 0;
   WCHAR szTargetPath[MAX_PATH];

   if (FAILED(::StringCchCopy(szTargetPath, lengthof(szTargetPath), pszFilePath)))
   {
      return GetDefaultImageIndex(pszFilePath, dwAttrib);
   }

   // Get the target
   if (PathIsLink(pszFilePath))
   {
      do
      {
         if (!SHGetShortcutTarget(szTargetPath, szTargetPath, lengthof(szTargetPath)))
         {
            return GetDefaultImageIndex(pszFilePath, dwAttrib);
         }
         PathRemoveQuotesAndArgs(szTargetPath);
      } while (PathIsLink(szTargetPath));

      dwAttrib = ::GetFileAttributes(szTargetPath);
   }

   if ((-1 != dwAttrib) && (FILE_ATTRIBUTE_DIRECTORY & dwAttrib)) // Directory
   {
      iIndex = GetDefaultImageIndex(szTargetPath, dwAttrib);
   }
   else // Cached item
   {
      CacheInfo * pCacheInfo = GetItem(szTargetPath);
      if (pCacheInfo)
      {
         if (fGetAltImageIndex && (-1 != pCacheInfo->m_iImageAltIndex))
         {
            iIndex = pCacheInfo->m_iImageAltIndex;
         }
         else
         {
            iIndex = pCacheInfo->m_iImageIndex;
         }
      }
      else
      {
         iIndex = GetDefaultImageIndex(szTargetPath, dwAttrib);
      }
   }

   return iIndex;
}

CIconCache::CacheInfo * CIconCache::GetItem(LPCWSTR pszFilePath)
{
   ASSERT(pszFilePath);
   if (!pszFilePath || (TEXT('\0') == *pszFilePath))
   {
      return NULL;
   }

   m_list.Lock(TRUE);

   CacheInfo * pCacheInfo = (CacheInfo *) m_list.Head();
   LPCWSTR pszExtension = NULL;

   while (pCacheInfo)
   {
      // Check to see if we store an explicit item
      if (0 == ::_tcsicmp(pszFilePath, pCacheInfo->m_pszItem))
      {
         break; // Found it
      }

      // Check the extension
      pszExtension = PathFindExtension(pszFilePath);
      if (pszExtension && (TEXT('\0') != *pszExtension))
      {
         if (0 == ::_tcsicmp(pszExtension, pCacheInfo->m_pszItem))
         {
            break;
         }
      }

      pCacheInfo = (CacheInfo *) m_list.Next(pCacheInfo);
   }

   if (!pCacheInfo)
   {
      // Not in the cache, cache it now
      pCacheInfo = CacheItem(pszFilePath);
      if (pCacheInfo)
      {
         m_list.AddTail(pCacheInfo);
      }
   }

   m_list.Lock(FALSE);

   return pCacheInfo;
}

HIMAGELIST CIconCache::GetSystemImageList(UINT uFlags) const
{
   HIMAGELIST himl = NULL;

   switch (uFlags)
   {
      case SHGFI_SMALLICON:
         himl = m_himlSmall;
      break;

      case SHGFI_LARGEICON:
         himl =  m_himlLarge;
      break;
   }

   return himl;
}

BOOL CIconCache::GetType(LPCWSTR pszFilePath, DWORD dwAttrib, LPWSTR pszType, size_t cchType)
{
   ASSERT(pszFilePath);
   ASSERT(pszType);
   if (!pszFilePath || !pszType || (TEXT('\0') == *pszFilePath))
   {
      return FALSE;
   }

   HRESULT hr = E_FAIL;
   LPCWSTR pszFileName = PathFindFileName(pszFilePath);
   ASSERT(pszFileName);

   if ((0 == _tcscmp(TEXT("\\"), pszFilePath)) || // File system root
       (SUCCEEDED(PathIsGUID(pszFileName)))) // Namespace
   {
      hr = ::StringCchCopy(pszType, cchType, LOAD_STRING(IDS_FILETYPE_SYSTEM));
   }
   else if ((-1 != dwAttrib) && (FILE_ATTRIBUTE_DIRECTORY & dwAttrib)) // Directory
   {
      hr = ::StringCchCopy(pszType, cchType, LOAD_STRING(IDS_FILETYPE_FOLDER));
   }
   else if (PathIsExe(pszFileName)) // Executable
   {
      hr = ::StringCchCopy(pszType, cchType, LOAD_STRING(IDS_FILETYPE_APPLICATION));
   }
   else if (PathIsLink(pszFileName)) // Shortcut
   {
      hr = ::StringCchCopy(pszType, cchType, LOAD_STRING(IDS_FILETYPE_SHORTCUT));
   }
   else // Cached item
   {
      CacheInfo * pCacheInfo = GetItem(pszFilePath);
      if (pCacheInfo && pCacheInfo->m_pszType)
      {
         hr = ::StringCchCopy(pszType, cchType, pCacheInfo->m_pszType);
      }
   }

   return SUCCEEDED(hr);
}

BOOL CIconCache::IExtractIcon(LPWSTR pszIcon, HICON * phiconSM, HICON * phiconLG) const
{
   BOOL fFound = FALSE;

   if (phiconSM)
   {
      *phiconSM = NULL;
   }

   if (phiconLG)
   {
      *phiconLG = NULL;
   }

   LPWSTR pwsz = wcschr(pszIcon, L',');
   if (pwsz)
   {
      *pwsz = L'\0';
      int iIndex = _ttol(++pwsz);

      if (CSTR_EQUAL == ::CompareString(LCID_ENGLISH, NORM_IGNORECASE,
                                        PathFindFileName(pszIcon),
                                        -1, TEXT("ceshell.dll"), -1))
      {
         if (phiconSM)
         {
            *phiconSM = (HICON) ::LoadImage(HINST_CESHELL,
                                            MAKEINTRESOURCE(-iIndex),
                                            IMAGE_ICON,
                                            ::GetSystemMetrics(SM_CXSMICON),
                                            ::GetSystemMetrics(SM_CYSMICON),
                                            LR_DEFAULTCOLOR);
         }

         if (phiconLG)
         {
            *phiconLG = (HICON) ::LoadImage(HINST_CESHELL,
                                            MAKEINTRESOURCE(-iIndex),
                                            IMAGE_ICON,
                                            ::GetSystemMetrics(SM_CXICON),
                                            ::GetSystemMetrics(SM_CYICON),
                                            LR_DEFAULTCOLOR);
         }
      }
      else
      {
         // if the path is _not_ ceshell.dll
         ::ExtractIconEx(pszIcon, iIndex, phiconLG, phiconSM, 1);
      }

      fFound = TRUE;
   }

   return fFound;
}

BOOL CIconCache::RebuildSystemImageList()
{
   HICON hiconSM;
   HICON hiconLG;
   int iIndex;
   BOOL fSuccess = TRUE;

   // NOTE: We are really bummin if any of this fails, so let us
   //       protect ourselves by bailing if system resources are low.
   MEMORYSTATUS ms = {0};
   ms.dwLength = sizeof(MEMORYSTATUS);
   GlobalMemoryStatus(&ms);
   if (ms.dwAvailPhys < HIBERNATE_FREEZE_CACHE)
   {
      return FALSE;
   }

   // Clean out the image lists
   ASSERT(m_himlSmall);
   ASSERT(m_himlLarge);
   ImageList_Remove(m_himlSmall, -1);
   ImageList_Remove(m_himlLarge, -1);

   // Fill the image lists with the default shell icons
   for (int i = IDI_SYSTEM_ICON_FIRST; i <= IDI_SYSTEM_ICON_LAST; i++)
   {
      hiconSM = (HICON) ::LoadImage(HINST_CESHELL, MAKEINTRESOURCE(i), IMAGE_ICON,
                                    ::GetSystemMetrics(SM_CXSMICON),
                                    ::GetSystemMetrics(SM_CYSMICON),
                                    LR_DEFAULTCOLOR);
      if (!hiconSM)
      {
         fSuccess = FALSE;
         break;
      }

      iIndex = ImageList_AddIcon(m_himlSmall, hiconSM);
      ::DestroyIcon(hiconSM);
      if (-1 == iIndex)
      {
         fSuccess = FALSE;
         break;
      }

      hiconLG = (HICON) ::LoadImage(HINST_CESHELL, MAKEINTRESOURCE(i), IMAGE_ICON,
                                    ::GetSystemMetrics(SM_CXICON),
                                    ::GetSystemMetrics(SM_CYICON),
                                    LR_DEFAULTCOLOR);
      if (!hiconLG)
      {
         fSuccess = FALSE;
         break;
      }

      iIndex = ImageList_AddIcon(m_himlLarge, hiconLG);
      ::DestroyIcon(hiconLG);
      if (-1 == iIndex)
      {
         fSuccess = FALSE;
         break;
      }
   }

   if (fSuccess)
   {
      // Set the overlay image (shortcut arrow)
      ImageList_SetOverlayImage(m_himlSmall, SHORTCUT_ICON, 1);
      ImageList_SetOverlayImage(m_himlLarge, SHORTCUT_ICON, 1);
   }
   else
   {
      // Clean up (free all icons from the list to free up memory)
      ImageList_Remove(m_himlSmall, -1);
      ImageList_Remove(m_himlLarge, -1);
   }

   return fSuccess;
}

