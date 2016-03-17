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

#include "ceshpriv.h"

SHSTDAPI SHGetDesktopFolder(LPSHELLFOLDER *ppshf)
{
   if (ppshf)
      *ppshf = NULL;

   return E_NOTIMPL;
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
      case CSIDL_DESKTOP:
      case CSIDL_DRIVES:
      case CSIDL_NETWORK:
         hr = E_NOTIMPL;
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

WINSHELLAPI int WINAPI SHFileOperation(LPSHFILEOPSTRUCT lpFileOp)
{
   return 1; // Failure
}

extern "C" LPITEMIDLIST WINAPI SHBrowseForFolder(LPBROWSEINFO lpbi)
{
   return NULL; // Error
}

BOOL ILIsNameSpace(LPCITEMIDLIST pidl, REFCLSID rclsid)
{
   return FALSE; // Namespaces not supported
}

