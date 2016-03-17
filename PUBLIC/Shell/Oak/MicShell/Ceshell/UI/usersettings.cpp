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

#include "usersettings.h"
#include "filechangemgr.h"
#include "resource.h"
#include <aygshell.h>

const TCHAR UserSettings::DESKTOP[] = TEXT("ControlPanel\\Desktop");
const TCHAR UserSettings::EXPLORER[] = TEXT("Explorer");
const TCHAR UserSettings::BACKGROUND_IMAGE_LOCATION[] = TEXT("wallpaper");
const TCHAR UserSettings::LISTVIEW_STYLE[] = TEXT("ListStyle");
const TCHAR UserSettings::RECYCLE_BIN_PERCENTAGE[] = TEXT("RecycleBinSize");
const TCHAR UserSettings::SHOW_EXTENSIONS[] = TEXT("ShowExt");
const TCHAR UserSettings::SHOW_HIDDEN_FILES[] = TEXT("ViewAll");
const TCHAR UserSettings::SHOW_SYSTEM_FILES[] = TEXT("ShowSys");
const TCHAR UserSettings::TILE_BACKGROUND_IMAGE[] = TEXT("tile");
const TCHAR UserSettings::USE_2ND_CLIPBOARD[] = TEXT("Use2ndClipboard");
const TCHAR UserSettings::USE_COMPATIBLE_BACKGROUND_IMAGE[] = TEXT("UseCompatibleBGImage");
const TCHAR UserSettings::USE_RECYCLE_BIN[] = TEXT("UseRecycleBin");

const DWORD UserSettings::DEFAULT_LISTVIEW_STYLE = LVS_ICON;
const DWORD UserSettings::DEFAULT_RECYCLE_BIN_SIZE = 10;
const BOOL UserSettings::DEFAULT_SHOW_EXTENSIONS = FALSE;
const BOOL UserSettings::DEFAULT_SHOW_HIDDEN_FILES = FALSE;
const BOOL UserSettings::DEFAULT_SHOW_SYSTEM_FILES = FALSE;
const BOOL UserSettings::DEFAULT_TILE_BACKGROUND_IMAGE = FALSE;
const BOOL UserSettings::DEFAULT_USE_2ND_CLIPBOARD = FALSE;
const BOOL UserSettings::DEFAULT_USE_COMPATIBLE_BACKGROUND_IMAGE = TRUE;
const BOOL UserSettings::DEFAULT_USE_RECYCLE_BIN = TRUE;

TCHAR UserSettings::szBackgoundImageLocation[] = TEXT("");
DWORD UserSettings::dwRecycleBinSize = DEFAULT_RECYCLE_BIN_SIZE;
BOOL UserSettings::fShowExtensions = DEFAULT_SHOW_EXTENSIONS;
BOOL UserSettings::fShowHiddenFiles = DEFAULT_SHOW_HIDDEN_FILES;
BOOL UserSettings::fShowSystemFiles = DEFAULT_SHOW_SYSTEM_FILES;
BOOL UserSettings::fTileBackgroundImage = DEFAULT_TILE_BACKGROUND_IMAGE;
BOOL UserSettings::fUse2ndClipboard = DEFAULT_USE_2ND_CLIPBOARD;
BOOL UserSettings::fUseCompatibleBackgroundImage = DEFAULT_USE_COMPATIBLE_BACKGROUND_IMAGE;
BOOL UserSettings::fUseRecycleBin = DEFAULT_USE_RECYCLE_BIN;
HWND UserSettings::hFolderOptionsDlg = NULL;

BOOL UserSettings::GetBOOL(HKEY hkeyRoot, LPCTSTR pszPath, LPCTSTR pszKey, BOOL defVal)
{
   HKEY hKey = NULL;
   BOOL retVal = defVal;

   if (ERROR_SUCCESS == RegOpenKeyEx(hkeyRoot, pszPath, 0, 0, &hKey))
   {
      LPBYTE p = (LPBYTE) &retVal;
      DWORD dw = sizeof(retVal);
      if (ERROR_SUCCESS != RegQueryValueEx(hKey, pszKey, 0, NULL, p, &dw))
      {
         retVal = defVal;
      }

      RegCloseKey(hKey);
   }
   else
   {
      retVal = defVal;
   }

   return retVal;
}

DWORD UserSettings::GetDWORD(HKEY hkeyRoot, LPCTSTR pszPath, LPCTSTR pszKey, DWORD defVal)
{
   HKEY hKey = NULL;
   DWORD retVal = defVal;

   if (ERROR_SUCCESS == RegOpenKeyEx(hkeyRoot, pszPath, 0, 0, &hKey))
   {
      LPBYTE p = (LPBYTE) &retVal;
      DWORD dw = sizeof(retVal);
      if (ERROR_SUCCESS != ::RegQueryValueEx(hKey, pszKey, 0, NULL, p, &dw))
      {
         retVal = defVal;
      }

      ::RegCloseKey(hKey);
   }
   else
   {
      retVal = defVal;
   }

   return retVal;
}

BOOL UserSettings::GetSZ(HKEY hkeyRoot, LPCTSTR pszPath, LPCTSTR pszKey, LPTSTR psz, size_t cch, LPCTSTR defVal)
{
   HKEY hKey = NULL;
   BOOL fSuccess = FALSE;

   if (psz && (0 < cch))
   {
      if (ERROR_SUCCESS == RegOpenKeyEx(hkeyRoot, pszPath, 0, 0, &hKey))
      {
         LPBYTE p = (LPBYTE) psz;
         DWORD dw = cch*sizeof(TCHAR);
         if (ERROR_SUCCESS == ::RegQueryValueEx(hKey, pszKey, 0, NULL, p, &dw))
         {
            fSuccess = TRUE;
         }

         ::RegCloseKey(hKey);
      }
   }

   if (!fSuccess)
   {
      if (psz && (0 < cch))
      {
         if (defVal)
         {
            if (FAILED(StringCchCopy(psz, cch, defVal)))
               *psz = TEXT('\0');
         }
         else
         {
            *psz = TEXT('\0');
         }
      }
   }

   return fSuccess;
}

BOOL UserSettings::SetBOOL(HKEY hkeyRoot, LPCTSTR pszPath, LPCTSTR pszKey, BOOL setVal)
{
   BOOL fSuccess = FALSE;
   HKEY hKey = NULL;

   if (ERROR_SUCCESS == ::RegCreateKeyEx(hkeyRoot, pszPath, 0,
                                         L"", 0, 0, NULL, &hKey, NULL))
   {
      LPBYTE p = (LPBYTE) &setVal;
      if (ERROR_SUCCESS == ::RegSetValueEx(hKey, pszKey, 0, REG_DWORD,
                                           p, sizeof(setVal)))
      {
         fSuccess = TRUE;
      }

      ::RegCloseKey(hKey);
   }

   return fSuccess;
}

BOOL UserSettings::SetDWORD(HKEY hkeyRoot, LPCTSTR pszPath, LPCTSTR pszKey, DWORD setVal)
{
   BOOL fSuccess = FALSE;
   HKEY hKey = NULL;

   if (ERROR_SUCCESS == ::RegCreateKeyEx(hkeyRoot, pszPath, 0,
                                         L"", 0, 0, NULL, &hKey, NULL))
   {
      LPBYTE p = (LPBYTE) &setVal;
      if (ERROR_SUCCESS == ::RegSetValueEx(hKey, pszKey, 0, REG_DWORD,
                                           p, sizeof(setVal)))
      {
         fSuccess = TRUE;
      }

      ::RegCloseKey(hKey);
   }

   return fSuccess;
}

BOOL UserSettings::SetSZ(HKEY hkeyRoot, LPCTSTR pszPath, LPCTSTR pszKey, LPCTSTR psz, size_t cch)
{
   BOOL fSuccess = FALSE;
   HKEY hKey = NULL;

   if (ERROR_SUCCESS == ::RegCreateKeyEx(hkeyRoot, pszPath, 0,
                                         L"", 0, 0, NULL, &hKey, NULL))
   {
      LPBYTE p = (LPBYTE) psz;
      if (ERROR_SUCCESS == ::RegSetValueEx(hKey, pszKey, 0, REG_SZ, p, cch*sizeof(TCHAR)))
      {
         fSuccess = TRUE;
      }

      ::RegCloseKey(hKey);
   }

   return fSuccess;
}

BOOL UserSettings::GetBackgroundImageLocation(LPTSTR psz, size_t cch)
{
   return SUCCEEDED(StringCchCopy(psz, cch, szBackgoundImageLocation));
}

DWORD UserSettings::GetListviewStyle()
{
   DWORD dwListStyle;
   DWORD dwValue = GetDWORD(HKEY_LOCAL_MACHINE, EXPLORER,
                               LISTVIEW_STYLE,
                               DEFAULT_LISTVIEW_STYLE);

   switch (dwValue)
   {
      case LVS_SMALLICON:
         dwListStyle = FVM_SMALLICON;
         break;

      case LVS_LIST:
         dwListStyle = FVM_LIST;
      break;

      case LVS_REPORT:
         dwListStyle = FVM_DETAILS;
      break;

      case LVS_ICON:
      default:
         dwListStyle = FVM_ICON;
      break;
   }

   return dwListStyle;
}

DWORD UserSettings::GetRecycleBinSize()
{
   return dwRecycleBinSize;
}

BOOL UserSettings::GetShowExtensions()
{
   return fShowExtensions;
}

BOOL UserSettings::GetShowHiddenFiles()
{
   return fShowHiddenFiles;
}

BOOL UserSettings::GetShowSystemFiles()
{
   return fShowSystemFiles;
}

BOOL UserSettings::GetTileBackgroundImage()
{
   return fTileBackgroundImage;
}

BOOL UserSettings::GetUse2ndClipboard()
{
   return fUse2ndClipboard;
}

BOOL UserSettings::GetUseCompatibleBackgroundImage()
{
   return fUseCompatibleBackgroundImage;
}

BOOL UserSettings::GetUseRecycleBin()
{
   return fUseRecycleBin;
}

BOOL UserSettings::SetBackgroundImageLocation(LPCTSTR psz, size_t cch)
{
   BOOL fSuccess = SetSZ(HKEY_CURRENT_USER, DESKTOP, BACKGROUND_IMAGE_LOCATION, psz, cch);

   // Update the cached value
   if (fSuccess)
   {
      fSuccess = SUCCEEDED(StringCchCopy(szBackgoundImageLocation,
                                         lengthof(szBackgoundImageLocation),
                                         psz));
   }

   return fSuccess;
}

BOOL UserSettings::SetListviewStyle(DWORD dwStyle)
{
   DWORD dwValue;

   switch (dwStyle)
   {
      case FVM_SMALLICON:
         dwValue = LVS_SMALLICON;
         break;

      case FVM_LIST:
         dwValue = LVS_LIST;
      break;

      case FVM_DETAILS:
         dwValue = LVS_REPORT;
      break;

      case FVM_ICON:
      default:
         dwValue = LVS_ICON;
      break;
   }

   return SetDWORD(HKEY_LOCAL_MACHINE, EXPLORER, LISTVIEW_STYLE, dwValue);
}

BOOL UserSettings::SetRecycleBinSize(DWORD dwSize)
{
   BOOL fSuccess = SetDWORD(HKEY_LOCAL_MACHINE, EXPLORER, RECYCLE_BIN_PERCENTAGE, dwSize);

   // Update the cached value
   if (fSuccess)
      dwRecycleBinSize = dwSize;

   return fSuccess;
}

BOOL UserSettings::SetShowExtensions(BOOL fShow)
{
   BOOL fSuccess = SetBOOL(HKEY_LOCAL_MACHINE, EXPLORER, SHOW_EXTENSIONS, fShow);

   // Update the cached value
   if (fSuccess)
      fShowExtensions = fShow;

   return fSuccess;
}

BOOL UserSettings::SetShowHiddenFiles(BOOL fShow)
{
   BOOL fSuccess = SetBOOL(HKEY_LOCAL_MACHINE, EXPLORER, SHOW_HIDDEN_FILES, fShow);

   // Update the cached value
   if (fSuccess)
      fShowHiddenFiles = fShow;

   return fSuccess;
}

BOOL UserSettings::SetShowSystemFiles(BOOL fShow)
{
   BOOL fSuccess = SetBOOL(HKEY_LOCAL_MACHINE, EXPLORER, SHOW_SYSTEM_FILES, fShow);

   // Update the cached value
   if (fSuccess)
      fShowSystemFiles = fShow;

   return fSuccess;
}

BOOL UserSettings::SetTileBackgroundImage(BOOL fTile)
{
   BOOL fSuccess = SetBOOL(HKEY_CURRENT_USER, DESKTOP, SHOW_SYSTEM_FILES, fTile);

   // Update the cached value
   if (fSuccess)
      fTileBackgroundImage = fTile;

   return fSuccess;
}

BOOL UserSettings::SetUseCompatibleBackgroundImage(BOOL fUse)
{
   BOOL fSuccess = SetBOOL(HKEY_LOCAL_MACHINE, EXPLORER, USE_COMPATIBLE_BACKGROUND_IMAGE, fUse);

   // Update the cached value
   if (fSuccess)
      fUseCompatibleBackgroundImage = fUse;

   return fSuccess;
}

BOOL UserSettings::SetUseRecycleBin(BOOL fUse)
{
   BOOL fSuccess = SetBOOL(HKEY_LOCAL_MACHINE, EXPLORER, USE_RECYCLE_BIN, fUse);

   // Update the cached value
   if (fSuccess)
      fUseRecycleBin = fUse;

   return fSuccess;
}

void UserSettings::Query()
{
   // Initialize the cache

   GetSZ(HKEY_CURRENT_USER, DESKTOP, BACKGROUND_IMAGE_LOCATION,
         szBackgoundImageLocation, lengthof(szBackgoundImageLocation),
         TEXT(""));
   dwRecycleBinSize = GetDWORD(HKEY_LOCAL_MACHINE, EXPLORER,
                               RECYCLE_BIN_PERCENTAGE,
                               DEFAULT_RECYCLE_BIN_SIZE);
   fShowExtensions = GetBOOL(HKEY_LOCAL_MACHINE, EXPLORER, SHOW_EXTENSIONS,
                             DEFAULT_SHOW_EXTENSIONS);
   fShowHiddenFiles = GetBOOL(HKEY_LOCAL_MACHINE, EXPLORER, SHOW_HIDDEN_FILES,
                              DEFAULT_SHOW_HIDDEN_FILES);
   fShowSystemFiles = GetBOOL(HKEY_LOCAL_MACHINE, EXPLORER, SHOW_SYSTEM_FILES,
                              DEFAULT_SHOW_SYSTEM_FILES);
   fTileBackgroundImage = GetBOOL(HKEY_CURRENT_USER, DESKTOP,
                                  TILE_BACKGROUND_IMAGE,
                                  DEFAULT_TILE_BACKGROUND_IMAGE);
   fUse2ndClipboard = GetBOOL(HKEY_LOCAL_MACHINE, EXPLORER, USE_2ND_CLIPBOARD,
                              DEFAULT_USE_2ND_CLIPBOARD);
   fUseCompatibleBackgroundImage = GetBOOL(HKEY_LOCAL_MACHINE, EXPLORER,
                                           USE_COMPATIBLE_BACKGROUND_IMAGE,
                                           DEFAULT_USE_COMPATIBLE_BACKGROUND_IMAGE);
   fUseRecycleBin = GetBOOL(HKEY_LOCAL_MACHINE, EXPLORER, USE_RECYCLE_BIN,
                            DEFAULT_USE_RECYCLE_BIN);
}

int CALLBACK UserSettings::FolderOptionsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
      case WM_INITDIALOG:
      {
         AygInitDialog( hDlg, SHIDIF_SIPDOWN );

         ::CheckDlgButton(hDlg, IDC_HIDEFILEEXTENSIONS,
                          GetShowExtensions() ? BST_UNCHECKED : BST_CHECKED);

         ::CheckDlgButton(hDlg, IDC_HIDEHIDDENFILES,
                          GetShowHiddenFiles() ? BST_UNCHECKED : BST_CHECKED);

         ::CheckDlgButton(hDlg, IDC_HIDESYSTEMFILES,
                          GetShowSystemFiles() ? BST_UNCHECKED : BST_CHECKED);

         // Have only one instance of the Options Dialog launched.
         hFolderOptionsDlg = hDlg;
      }
      break;

      case WM_COMMAND:
      {
         switch (LOWORD(wParam))
         {
            case IDOK:
               SetShowExtensions(BST_CHECKED != ::IsDlgButtonChecked(hDlg, IDC_HIDEFILEEXTENSIONS));
               SetShowHiddenFiles(BST_CHECKED != ::IsDlgButtonChecked(hDlg, IDC_HIDEHIDDENFILES));
               SetShowSystemFiles(BST_CHECKED != ::IsDlgButtonChecked(hDlg, IDC_HIDESYSTEMFILES));
               g_pFileChangeManager->RefreshAllViews();
               ::EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
            break;

            case IDCANCEL:
               ::EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
            break;

            default:
               return FALSE;
         }
      }
      break;

      default:
         return FALSE;
   }

   return TRUE;
}

void UserSettings::ShowFolderOptions()
{
   if (!hFolderOptionsDlg)
   {
      ::DialogBox(HINST_CESHELL, MAKEINTRESOURCE(IDD_FOLDER_OPTIONS),
                  NULL, FolderOptionsDlgProc);

      hFolderOptionsDlg = NULL;
   }
   else
   {            
      SetForegroundWindow(hFolderOptionsDlg);
   }
}

