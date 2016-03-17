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

#pragma once

#include "ceshhpc.h"

class UserSettings
{
protected:   
   static const TCHAR DESKTOP[];
   static const TCHAR EXPLORER[];
   static const TCHAR BACKGROUND_IMAGE_LOCATION[];
   static const TCHAR LISTVIEW_STYLE[];
   static const TCHAR RECYCLE_BIN_PERCENTAGE[];
   static const TCHAR SHOW_EXTENSIONS[];
   static const TCHAR SHOW_HIDDEN_FILES[];
   static const TCHAR SHOW_SYSTEM_FILES[];
   static const TCHAR TILE_BACKGROUND_IMAGE[];
   static const TCHAR USE_2ND_CLIPBOARD[];
   static const TCHAR USE_COMPATIBLE_BACKGROUND_IMAGE[];
   static const TCHAR USE_RECYCLE_BIN[];

   static const DWORD DEFAULT_LISTVIEW_STYLE;
   static const DWORD DEFAULT_RECYCLE_BIN_SIZE;
   static const BOOL DEFAULT_SHOW_EXTENSIONS;
   static const BOOL DEFAULT_SHOW_HIDDEN_FILES;
   static const BOOL DEFAULT_SHOW_SYSTEM_FILES;
   static const BOOL DEFAULT_TILE_BACKGROUND_IMAGE;
   static const BOOL DEFAULT_USE_2ND_CLIPBOARD;
   static const BOOL DEFAULT_USE_COMPATIBLE_BACKGROUND_IMAGE;
   static const BOOL DEFAULT_USE_RECYCLE_BIN;

   static TCHAR szBackgoundImageLocation[MAX_PATH];
   static DWORD dwRecycleBinSize;
   static BOOL fShowExtensions;
   static BOOL fShowHiddenFiles;
   static BOOL fShowSystemFiles;
   static BOOL fTileBackgroundImage;
   static BOOL fUse2ndClipboard;
   static BOOL fUseCompatibleBackgroundImage;
   static BOOL fUseRecycleBin;

   static HWND hFolderOptionsDlg;
   static BOOL GetBOOL(HKEY hkeyRoot, LPCTSTR pszPath, LPCTSTR pszKey, BOOL defVal);
   static DWORD GetDWORD(HKEY hkeyRoot, LPCTSTR pszPath, LPCTSTR pszKey, DWORD defVal);
   static BOOL GetSZ(HKEY hkeyRoot, LPCTSTR pszPath, LPCTSTR pszKey, LPTSTR psz, size_t cch, LPCTSTR defVal);
   static BOOL SetBOOL(HKEY hkeyRoot, LPCTSTR pszPath, LPCTSTR pszKey, BOOL setVal);
   static BOOL SetDWORD(HKEY hkeyRoot, LPCTSTR pszPath, LPCTSTR pszKey, DWORD setVal);
   static BOOL SetSZ(HKEY hkeyRoot, LPCTSTR pszPath, LPCTSTR pszKey, LPCTSTR psz, size_t cch);

   static int CALLBACK FolderOptionsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
   static BOOL GetBackgroundImageLocation(LPTSTR psz, size_t cch);
   static DWORD GetListviewStyle();
   static DWORD GetRecycleBinSize();
   static BOOL GetShowExtensions();
   static BOOL GetShowHiddenFiles();
   static BOOL GetShowSystemFiles();
   static BOOL GetTileBackgroundImage();
   static BOOL GetUse2ndClipboard();
   static BOOL GetUseCompatibleBackgroundImage();
   static BOOL GetUseRecycleBin();

   static BOOL SetBackgroundImageLocation(LPCTSTR psz, size_t cch);
   static BOOL SetListviewStyle(DWORD dwStyle);
   static BOOL SetRecycleBinSize(DWORD dwSize);
   static BOOL SetShowExtensions(BOOL fShow);
   static BOOL SetShowHiddenFiles(BOOL fShow);
   static BOOL SetShowSystemFiles(BOOL fShow);
   static BOOL SetTileBackgroundImage(BOOL fTile);
   static BOOL SetUseCompatibleBackgroundImage(BOOL fUse);
   static BOOL SetUseRecycleBin(BOOL fUse);

   static void Query();
   static void ShowFolderOptions();
};

