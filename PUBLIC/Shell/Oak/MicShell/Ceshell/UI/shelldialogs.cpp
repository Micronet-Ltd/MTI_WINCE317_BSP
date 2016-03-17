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

#include "shelldialogs.h"
#include "aygutils.h"
#include "guid.h"
#include "idlist.h"
#include "recbin.h"
#include "resource.h"
#include <shellsdk.h>
#include "usersettings.h"

HWND ShellDialogs::Properties::hRecBinPropDlg = NULL;

HRESULT ShellDialogs::ShowFileError(HWND hwndOwner, LPCWSTR pszTitle,
                                    LPCWSTR pszMsg, LPCWSTR pszFile,
                                    UINT uFlags)
{
   return ShellDialogs::ShowPathError(hwndOwner,
                                      pszTitle,
                                      pszMsg,
                                      (pszFile ? PathFindFileName(pszFile) : NULL),
                                      uFlags);
}

HRESULT ShellDialogs::ShowPathError(HWND hwndOwner, LPCWSTR pszTitle,
                                    LPCWSTR pszMsg, LPCWSTR pszPath,
                                    UINT uFlags)
{
   HRESULT hr = NOERROR;
   HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_ARROW));

   // Translate resource ids to strings here
   if (pszTitle && !HIWORD(pszTitle))
   {
      pszTitle = LOAD_STRING(LOWORD(pszTitle));
   }

   if (pszMsg && !HIWORD(pszMsg))
   {
      pszMsg = LOAD_STRING(LOWORD(pszMsg));
   }

   // Generate the output
   if (pszMsg &&
      pszPath &&
      _tcschr(pszMsg, TEXT('%')))
   {
      LPWSTR pszOutput = NULL;
      size_t cchBuffer = 0;
      LPCWSTR pszFileName = pszPath;

      cchBuffer = _tcslen(pszMsg)+1;
      if (pszFileName)
      {
         cchBuffer += _tcslen(pszFileName);
      }
      else
      {
         cchBuffer += 3; // *.*
      }

      pszOutput = (LPWSTR) g_pShellMalloc->Alloc(cchBuffer*sizeof(WCHAR));
      if (pszOutput)
      {
         ::wsprintf(pszOutput, pszMsg, pszFileName ? pszFileName : TEXT("*.*"));
         MessageBox(hwndOwner, pszOutput, pszTitle, uFlags | MB_APPLMODAL | MB_SETFOREGROUND);  
         g_pShellMalloc->Free(pszOutput);
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
   }
   else
   {
      MessageBox(hwndOwner, pszMsg, pszTitle, uFlags | MB_APPLMODAL | MB_SETFOREGROUND);  
   }

   SetCursor(hCursor);
   return hr;
}

int CALLBACK ShellDialogs::Confirm::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   // Confirm * pConfirm = (Confirm *) GetWindowLong(hDlg, DWL_USER);

   switch (uMsg)
   {
      case WM_INITDIALOG:
         // SetWindowLong(hDlg, DWL_USER, lParam);
         ((Confirm *) lParam)->InitDialog(hDlg);
      break;

      case WM_DESTROY:
      {
         HICON hicon = NULL;

         hicon = (HICON) SendDlgItemMessage(hDlg, IDC_CONFIRM_ICON, STM_SETIMAGE,
                                            IMAGE_ICON, (WPARAM) NULL);
         if (hicon)
         {
            DestroyIcon(hicon);
         }

         hicon = (HICON) SendDlgItemMessage(hDlg, IDC_FILEICON_OLD, STM_SETIMAGE,
                                            IMAGE_ICON, (WPARAM) NULL);
         if (hicon)
         {
            DestroyIcon(hicon);
         }

         hicon = (HICON) SendDlgItemMessage(hDlg, IDC_FILEICON_NEW, STM_SETIMAGE,
                                            IMAGE_ICON, (WPARAM) NULL);
         if (hicon)
         {
            DestroyIcon(hicon);
         }
      }
      break;

      case WM_COMMAND:
         switch (LOWORD(wParam))
         {
            case IDYES:
            case IDYESTOALL:
            case IDNO:
            case IDCANCEL:
               ::EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
            break;
         }
      break;

      default:
         return FALSE;
   }

   return TRUE;
}

void ShellDialogs::Confirm::InitDialog(HWND hDlg)
{
   HICON hicon = NULL;

   // Set the title
   ::SetWindowText(hDlg, LOAD_STRING(m_uTitle));

   AygInitDialog( hDlg, SHIDIF_SIPDOWN );
 
   // Load the icon
   ASSERT(m_hicon);
   hicon = (HICON) ::SendDlgItemMessage(hDlg, IDC_CONFIRM_ICON, STM_SETIMAGE,
                                        IMAGE_ICON, (WPARAM) m_hicon);
   if (hicon)
   {
      DestroyIcon(hicon);
   }

   // Set the message text
   ::SetDlgItemText(hDlg, IDC_CONFIRM_MESSAGE, m_szMessage);

   if (m_fExtended)
   {
      // The old icon and info
      ASSERT(m_hiconOld);
      hicon = (HICON) ::SendDlgItemMessage(hDlg, IDC_FILEICON_OLD, STM_SETIMAGE,
                                           IMAGE_ICON, (WPARAM) m_hiconOld);
      if (hicon)
      {
         ::DestroyIcon(hicon);
      }
      ::SetDlgItemText(hDlg, IDC_FILEINFO_OLD, m_szFileInfoOld);

      // The new icon and info
      ASSERT(m_hiconNew);
      hicon = (HICON) ::SendDlgItemMessage(hDlg, IDC_FILEICON_NEW, STM_SETIMAGE,
                                           IMAGE_ICON, (WPARAM) m_hiconNew);
      if (hicon)
      {
         ::DestroyIcon(hicon);
      }
      ::SetDlgItemText(hDlg, IDC_FILEINFO_NEW, m_szFileInfoNew);
   }
   else
   {
      // Move the buttons so the dialog doesn't look too big or too small
      HWND hwnd = ::GetDlgItem(hDlg, IDC_CONFIRM_MESSAGE);
      HDC hdc = ::GetDC(hwnd);
      DWORD dwStyle = DT_LEFT | DT_EXPANDTABS | DT_WORDBREAK | DT_NOPREFIX | DT_CALCRECT;
      UINT buttons[] = { IDYES, IDYESTOALL, IDNO, IDCANCEL };
      RECT rc = {0};
      int dy = 0;
      int dyMax = 0;

      // Figure out how much to move to buttons
      ::GetClientRect(hwnd, &rc);
      dy = rc.bottom;
      ::DrawText(hdc, m_szMessage, -1, &rc, dwStyle);
      ::ReleaseDC(hwnd, hdc);
      dy -= rc.bottom;

      // Don't allow the buttons to move above the bottom of the icon
      ::GetClientRect(hwnd, &rc);
      ::MapWindowRect(NULL, hDlg, &rc);
      dyMax = rc.bottom;
      ::GetClientRect(::GetDlgItem(hDlg, IDC_CONFIRM_ICON), &rc);
      ::MapWindowRect(NULL, hDlg, &rc);
      dyMax -= rc.bottom;
      if (dyMax < dy)
      {
         dy = dyMax;
      }

      for (UINT i = 0; i < (sizeof(buttons)/sizeof(*buttons)); i++)
      {
         hwnd = GetDlgItem(hDlg, buttons[i]);
         if (hwnd)
         {
            ::GetWindowRect(hwnd, &rc);
            ::MapWindowRect(NULL, hDlg, &rc);
            ::SetWindowPos(hwnd, NULL, rc.left, rc.top-dy, 0, 0,
                           SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
         }
      }

      ::GetWindowRect(hDlg, &rc);
      ::SetWindowPos(hDlg, NULL, rc.left, rc.top+(dy/2),
                     rc.right-rc.left, rc.bottom-rc.top-dy,
                     SWP_NOZORDER | SWP_NOACTIVATE);
   }

   if (m_fHideYesToAllAndCancel)
   {
      RECT rc = {0};
      int dy = 0;

      // Get the postion of the button before the move
      ::GetWindowRect(::GetDlgItem(hDlg, IDYES), &rc);
      ::MapWindowRect(NULL, hDlg, &rc);
      dy = rc.top;

      // IDYES -> IDNO
      ::GetWindowRect(::GetDlgItem(hDlg, IDNO), &rc);
      ::MapWindowRect(NULL, hDlg, &rc);
      ::SetWindowPos(::GetDlgItem(hDlg, IDYES), NULL, rc.left, rc.top, 0, 0,
                     SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

      // IDNO -> IDCANCEL
      ::GetWindowRect(::GetDlgItem(hDlg, IDCANCEL), &rc);
      ::MapWindowRect(NULL, hDlg, &rc);
      ::SetWindowPos(::GetDlgItem(hDlg, IDNO), NULL, rc.left, rc.top, 0, 0,
                     SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

      // Destroy IDYESTOALL and IDCANCEL
      ::DestroyWindow(::GetDlgItem(hDlg, IDYESTOALL));
      ::DestroyWindow(::GetDlgItem(hDlg, IDCANCEL));

      // Expand the text area since the the buttons have been shifted down
      dy = rc.top - dy;
      ::GetWindowRect(::GetDlgItem(hDlg, IDC_CONFIRM_MESSAGE), &rc);
      ::MapWindowRect(NULL, hDlg, &rc);
      ::SetWindowPos(::GetDlgItem(hDlg, IDC_CONFIRM_MESSAGE), NULL,
                     rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top+dy,
                     SWP_NOZORDER | SWP_NOACTIVATE);
   }
}

HRESULT ShellDialogs::Confirm::Move(LPCWSTR pszPath, HWND hwndOwner, BOOL fHideYesToAllAndCancel, int * pResult)
{
   HRESULT hr = S_OK;
   DWORD dwAttrib = (DWORD) -1;
   Confirm confirm;
   HCURSOR hcursor = NULL;

   if (!pszPath || !pResult)
   {
      return E_INVALIDARG;
   }

   dwAttrib = GetFileAttributes(pszPath);

   if (-1 == dwAttrib)
   {
      return E_FAIL;
   }

   if (FILE_ATTRIBUTE_DIRECTORY & dwAttrib)
   {
      *pResult = IDOK;
      return S_OK; // Do nothing
   }
   else if (FILE_ATTRIBUTE_SYSTEM & dwAttrib)
   {
      hr = ::StringCchPrintfEx(confirm.m_szMessage,
                           lengthof(confirm.m_szMessage),
                           NULL,
                           NULL,
                           STRSAFE_IGNORE_NULLS,
                           LOAD_STRING(IDS_CONFIRM_MOV_SYSTEM),
                           PathFindFileName(pszPath));
   }
   else if (FILE_ATTRIBUTE_READONLY & dwAttrib)
   {
      hr = ::StringCchPrintfEx(confirm.m_szMessage,
                           lengthof(confirm.m_szMessage),
                           NULL,
                           NULL,
                           STRSAFE_IGNORE_NULLS,
                           LOAD_STRING(IDS_CONFIRM_MOV_READONLY),
                           PathFindFileName(pszPath));
   }
   else
   {
      *pResult = IDOK;
      return S_OK; // Do nothing
   }

   if (FAILED(hr))
   {
      return hr;
   }

   confirm.m_uTitle = IDS_TITLE_CONFIRMMOVE;
   confirm.m_hicon = (HICON) LoadImage(HINST_CESHELL,
                                       MAKEINTRESOURCE(IDI_CONFIRMMOVE),
                                       IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
   confirm.m_fHideYesToAllAndCancel = fHideYesToAllAndCancel;
   confirm.m_fExtended = FALSE;

   hcursor = SetCursor(LoadCursor(NULL, IDC_ARROW));
   *pResult = DialogBoxParam(HINST_CESHELL, MAKEINTRESOURCE(IDD_CONFIRM_YESNOALLCANCEL),
                             hwndOwner, DlgProc, (LPARAM) &confirm);
   SetCursor(hcursor);

   return NOERROR;
}

HRESULT ShellDialogs::Confirm::Rename(LPCWSTR pszSrc, LPCWSTR pszDst,
                                      HWND hwndOwner, int * pResult)
{
   HRESULT hr = S_OK;
   DWORD dwAttrib = (DWORD) -1;
   Confirm confirm;
   HCURSOR hcursor = NULL;

   if (!pszSrc || !pszDst || !pResult)
   {
      return E_INVALIDARG;
   }

   dwAttrib = GetFileAttributes(pszSrc);

   if (-1 == dwAttrib)
   {
      return E_FAIL;
   }

   if (FILE_ATTRIBUTE_DIRECTORY & dwAttrib)
   {
      *pResult = IDOK;
      return S_OK; // Do nothing
   }
   else if (FILE_ATTRIBUTE_SYSTEM & dwAttrib)
   {
      hr = ::StringCchPrintfEx(confirm.m_szMessage,
                           lengthof(confirm.m_szMessage),
                           NULL,
                           NULL,
                           STRSAFE_IGNORE_NULLS,
                           LOAD_STRING(IDS_CONFIRM_REN_SYSTEM),
                           PathFindFileName(pszSrc),
                           PathFindFileName(pszDst));
   }
   else if (FILE_ATTRIBUTE_READONLY & dwAttrib)
   {
      hr = ::StringCchPrintfEx(confirm.m_szMessage,
                           lengthof(confirm.m_szMessage),
                           NULL,
                           NULL,
                           STRSAFE_IGNORE_NULLS,
                           LOAD_STRING(IDS_CONFIRM_REN_READONLY),
                           PathFindFileName(pszSrc),
                           PathFindFileName(pszDst));
   }
   else
   {
      *pResult = IDOK;
      return S_OK; // Do nothing
   }

   if (FAILED(hr))
   {
      return hr;
   }

   confirm.m_uTitle = IDS_TITLE_CONFIRMRENAME;
   confirm.m_hicon = (HICON) LoadImage(HINST_CESHELL,
                                       MAKEINTRESOURCE(IDI_CONFIRMRENAME),
                                       IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
   confirm.m_fHideYesToAllAndCancel = FALSE;
   confirm.m_fExtended = FALSE;

   hcursor = SetCursor(LoadCursor(NULL, IDC_ARROW));
   *pResult = DialogBoxParam(HINST_CESHELL, MAKEINTRESOURCE(IDD_CONFIRM_YESNO),
                             hwndOwner, DlgProc, (LPARAM) &confirm);
   SetCursor(hcursor);

   return NOERROR;
}

HRESULT ShellDialogs::Confirm::Delete(LPCWSTR pszPath, HWND hwndOwner, BOOL fAlwaysConfirm,
                                      BOOL fForcePermanent, BOOL fHideYesToAllAndCancel, int * pResult)
{
   HRESULT hr = S_OK;
   DWORD dwAttrib = (DWORD) -1;
   Confirm confirm;
   WCHAR szBuffer[lengthof(confirm.m_szMessage)];
   HCURSOR hcursor = NULL;
   LPCWSTR pszFormat = NULL;

   if (!pszPath || !pResult)
   {
      return E_INVALIDARG;
   }

   dwAttrib = GetFileAttributes(pszPath);

   if (-1 == dwAttrib)
   {
      return E_FAIL;
   }

   if (FILE_ATTRIBUTE_DIRECTORY & dwAttrib)
   {
      // These messages should be constructed to reflect the fact that
      // the user is deleting a folder, but this will do for now
      if (fAlwaysConfirm)
      {
         *confirm.m_szMessage = TEXT('\0');
      }
      else
      {
         *pResult = IDOK;
         return S_OK; // Do nothing
      }
   }
   else if (FILE_ATTRIBUTE_SYSTEM & dwAttrib)
   {
      hr = ::StringCchPrintfEx(confirm.m_szMessage,
                           lengthof(confirm.m_szMessage),
                           NULL,
                           NULL,
                           STRSAFE_IGNORE_NULLS,
                           LOAD_STRING(IDS_CONFIRM_DEL_SYSTEM),
                           PathFindFileName(pszPath));
   }
   else if (FILE_ATTRIBUTE_READONLY & dwAttrib)
   {
      hr =  ::StringCchPrintfEx(confirm.m_szMessage,
                           lengthof(confirm.m_szMessage),
                           NULL,
                           NULL,
                           STRSAFE_IGNORE_NULLS,
                           LOAD_STRING(IDS_CONFIRM_DEL_READONLY),
                           PathFindFileName(pszPath));
   }
   else if (PathIsExe(pszPath))
   {
      hr = ::StringCchPrintfEx(confirm.m_szMessage,
                           lengthof(confirm.m_szMessage),
                           NULL,
                           NULL,
                           STRSAFE_IGNORE_NULLS,
                           LOAD_STRING(IDS_CONFIRM_DEL_EXE),
                           PathFindFileName(pszPath));
   }
   else
   {
      if (fAlwaysConfirm)
      {
         *confirm.m_szMessage = TEXT('\0');
      }
      else
      {
         *pResult = IDOK;
         return S_OK; // Do nothing
      }
   }

   if (FAILED(hr))
   {
      return hr;
   }

   if (UserSettings::GetUseRecycleBin() && !fForcePermanent)
   {
      pszFormat = LOAD_STRING(IDS_CONFIRM_DEL_RECBIN);
   }
   else
   {
      pszFormat = LOAD_STRING(IDS_CONFIRM_DEL);
   }

   hr = ::StringCchPrintfEx(szBuffer,
                        lengthof(szBuffer),
                        NULL,
                        NULL,
                        STRSAFE_IGNORE_NULLS,
                        pszFormat,
                        PathFindFileName(pszPath));
   if (FAILED(hr))
   {
      return hr;
   }

   hr = StringCchCat(confirm.m_szMessage, lengthof(confirm.m_szMessage), szBuffer);
   if (FAILED(hr))
   {
      return hr;
   }

   confirm.m_uTitle = IDS_TITLE_CONFIRMDELETE;
   if (UserSettings::GetUseRecycleBin() && !fForcePermanent)
   {
      confirm.m_hicon = (HICON) LoadImage(HINST_CESHELL,
                                          MAKEINTRESOURCE(IDI_BITBUCKET_FULL),
                                          IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
   }
   else
   {
      confirm.m_hicon = (HICON) LoadImage(HINST_CESHELL,
                                          MAKEINTRESOURCE(IDI_DELETE_PERMANENTLY),
                                          IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);   
   }
   confirm.m_fHideYesToAllAndCancel = fHideYesToAllAndCancel;
   confirm.m_fExtended = FALSE;

   hcursor = SetCursor(LoadCursor(NULL, IDC_ARROW));
   *pResult = DialogBoxParam(HINST_CESHELL, MAKEINTRESOURCE(IDD_CONFIRM_YESNOALLCANCEL),
                             hwndOwner, DlgProc, (LPARAM) &confirm);
   SetCursor(hcursor);

   return NOERROR;
}

HRESULT ShellDialogs::Confirm::DeleteMultiple(DWORD dwCount, HWND hwndOwner, BOOL fForcePermanent, int * pResult)
{
   HRESULT hr = S_OK;
   Confirm confirm;
   HCURSOR hcursor = NULL;
   LPCWSTR pszFormat = NULL;

   if ((2 > dwCount) || !pResult)
   {
      return E_INVALIDARG;
   }

   if (UserSettings::GetUseRecycleBin() && !fForcePermanent)
   {
      pszFormat = LOAD_STRING(IDS_CONFIRM_DEL_MUL_RECBIN); 
   }
   else
   {
      pszFormat = LOAD_STRING(IDS_CONFIRM_DEL_MUL);
   }

   hr = ::StringCchPrintfEx(confirm.m_szMessage,
                        lengthof(confirm.m_szMessage),
                        NULL,
                        NULL,
                        STRSAFE_IGNORE_NULLS,
                        pszFormat,
                        dwCount);
   if (FAILED(hr))
   {
      return hr;
   }

   confirm.m_uTitle = IDS_TITLE_CONFIRMDELETE;
   if (UserSettings::GetUseRecycleBin() && !fForcePermanent)
   {
      confirm.m_hicon = (HICON) LoadImage(HINST_CESHELL,
                                          MAKEINTRESOURCE(IDI_BITBUCKET_FULL),
                                          IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
   }
   else
   {
      confirm.m_hicon = (HICON) LoadImage(HINST_CESHELL,
                                          MAKEINTRESOURCE(IDI_DELETE_PERMANENTLY),
                                          IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);   
   }
   confirm.m_fHideYesToAllAndCancel = FALSE;
   confirm.m_fExtended = FALSE;

   hcursor = SetCursor(LoadCursor(NULL, IDC_ARROW));
   *pResult = DialogBoxParam(HINST_CESHELL, MAKEINTRESOURCE(IDD_CONFIRM_YESNO),
                             hwndOwner, DlgProc, (LPARAM) &confirm);
   SetCursor(hcursor);

   return NOERROR;
}

HRESULT ShellDialogs::Confirm::DeleteForRecycleBin(LPCWSTR pszPath, HWND hwndOwner, BOOL fFull, int * pResult)
{
   HRESULT hr = S_OK;
   Confirm confirm;
   WCHAR szBuffer[lengthof(confirm.m_szMessage)];
   HCURSOR hcursor = NULL;
   LPCWSTR pszFormat = NULL;

   if (!pszPath || !pResult)
   {
      return E_INVALIDARG;
   }

   if (fFull)
   {
      hr = ::StringCchPrintfEx(szBuffer,
                           lengthof(szBuffer),
                           NULL,
                           NULL,
                           STRSAFE_IGNORE_NULLS,
                           LOAD_STRING(IDS_CONFIRM_DEL_BIG),
                           PathFindFileName(pszPath));
   }
   else
   {
      *szBuffer = TEXT('\0');
   }

   if (FAILED(hr))
   {
      return hr;
   }

   hr = StringCchCopy(confirm.m_szMessage, lengthof(confirm.m_szMessage), szBuffer);
   if (FAILED(hr))
   {
      return hr;
   }

   if (fFull)
   {
      pszFormat = LOAD_STRING(IDS_CONFIRM_DELCANCEL);
   }
   else
   {
      pszFormat = LOAD_STRING(IDS_CONFIRM_DEL);
   }

   hr = ::StringCchPrintfEx(szBuffer,
                        lengthof(szBuffer),
                        NULL,
                        NULL,
                        STRSAFE_IGNORE_NULLS,
                        pszFormat,
                        PathFindFileName(pszPath));
   if (FAILED(hr))
   {
      return hr;
   }

   hr = StringCchCat(confirm.m_szMessage, lengthof(confirm.m_szMessage), szBuffer);
   if (FAILED(hr))
   {
      return hr;
   }

   confirm.m_uTitle = IDS_TITLE_CONFIRMDELETE;
   confirm.m_hicon = (HICON) LoadImage(HINST_CESHELL,
                                       MAKEINTRESOURCE(IDI_DELETE_PERMANENTLY),
                                       IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);   
   confirm.m_fHideYesToAllAndCancel = FALSE;
   confirm.m_fExtended = FALSE;

   hcursor = SetCursor(LoadCursor(NULL, IDC_ARROW));
   *pResult = DialogBoxParam(HINST_CESHELL, MAKEINTRESOURCE(IDD_CONFIRM_YESNO),
                              hwndOwner, DlgProc, (LPARAM) &confirm);
   SetCursor(hcursor);

   return hr;
}

HRESULT ShellDialogs::Confirm::Merge(LPCWSTR pszPath, HWND hwndOwner, BOOL fHideYesToAllAndCancel, int * pResult)
{
   HRESULT hr = S_OK;
   DWORD dwAttrib = (DWORD) -1;
   Confirm confirm;
   HCURSOR hcursor = NULL;

   if (!pszPath || !pResult)
   {
      return E_INVALIDARG;
   }

   dwAttrib = GetFileAttributes(pszPath);
   if (-1 == dwAttrib)
   {
      return E_FAIL;
   }

   hr = ::StringCchPrintfEx(confirm.m_szMessage,
                        lengthof(confirm.m_szMessage),
                        NULL,
                        NULL,
                        STRSAFE_IGNORE_NULLS,
                        LOAD_STRING(IDS_CONFIRM_MRG_FOLDER),
                        PathFindFileName(pszPath));
   if (FAILED(hr))
   {
      return hr;
   }

   confirm.m_uTitle = IDS_TITLE_CONFIRMMERGE;
   confirm.m_hicon = (HICON) LoadImage(HINST_CESHELL,
                                       MAKEINTRESOURCE(IDI_CONFIRMREPLACE),
                                       IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
   confirm.m_fHideYesToAllAndCancel = fHideYesToAllAndCancel;
   confirm.m_fExtended = FALSE;

   hcursor = SetCursor(LoadCursor(NULL, IDC_ARROW));
   *pResult = DialogBoxParam(HINST_CESHELL, MAKEINTRESOURCE(IDD_CONFIRM_YESNOALLCANCEL),
                             hwndOwner, DlgProc, (LPARAM) &confirm);
   SetCursor(hcursor);

   return NOERROR;
}

HRESULT ShellDialogs::Confirm::Replace(LPCWSTR pszSrc, LPCWSTR pszDst, HWND hwndOwner, int * pResult)
{
   HRESULT hr = S_OK;
   DWORD dwAttribSrc = (DWORD) -1;
   DWORD dwAttribDst = (DWORD) -1;
   Confirm confirm;
   HCURSOR hcursor = NULL;
   LPCWSTR pszFormat = NULL;

   if (!pszSrc || !pszDst || !pResult)
   {
      return E_INVALIDARG;
   }

   dwAttribSrc = GetFileAttributes(pszSrc);
   dwAttribDst = GetFileAttributes(pszDst);

   if ((-1 == dwAttribSrc) || (-1 == dwAttribDst))
   {
      return E_FAIL;
   }

   if (FILE_ATTRIBUTE_SYSTEM & dwAttribDst)
   {
      pszFormat = LOAD_STRING(IDS_CONFIRM_REP_SYSTEM);
   }
   else if (FILE_ATTRIBUTE_READONLY & dwAttribDst)
   {
      pszFormat = LOAD_STRING(IDS_CONFIRM_REP_READONLY);
   }
   else
   {
      ASSERT(!(FILE_ATTRIBUTE_DIRECTORY & dwAttribDst));
      pszFormat = LOAD_STRING(IDS_CONFIRM_REP);
   }

   hr = ::StringCchPrintfEx(confirm.m_szMessage,
                        lengthof(confirm.m_szMessage),
                        NULL,
                        NULL,
                        STRSAFE_IGNORE_NULLS,
                        pszFormat,
                        PathFindFileName(pszDst));
   if (FAILED(hr))
   {
      return hr;
   }

   confirm.m_uTitle = IDS_TITLE_CONFIRMREPLACE;
   confirm.m_hicon = (HICON) LoadImage(HINST_CESHELL,
                                       MAKEINTRESOURCE(IDI_CONFIRMREPLACE),
                                       IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
   confirm.m_fHideYesToAllAndCancel = FALSE;
   confirm.m_fExtended = TRUE;

   // This is the extended data
   ShellFormat::FileInfo(pszDst, confirm.m_szFileInfoOld,
                         lengthof(confirm.m_szFileInfoOld));

   ShellFormat::FileInfo(pszSrc, confirm.m_szFileInfoNew,
                         lengthof(confirm.m_szFileInfoNew));

   SHFILEINFO shfi = {0};
   if (SHGetFileInfo(pszDst, 0, &shfi, sizeof(shfi), SHGFI_ICON|SHGFI_LARGEICON))
   {
      confirm.m_hiconOld = shfi.hIcon;
   }

   if (SHGetFileInfo(pszSrc, 0, &shfi, sizeof(shfi), SHGFI_ICON|SHGFI_LARGEICON))
   {
      confirm.m_hiconNew = shfi.hIcon;
    }

   hcursor = SetCursor(LoadCursor(NULL, IDC_ARROW));
   *pResult = DialogBoxParam(HINST_CESHELL, MAKEINTRESOURCE(IDD_CONFIRM_REPLACE),
                             hwndOwner, DlgProc, (LPARAM) &confirm);
   SetCursor(hcursor);

   return NOERROR;
}

BOOL ShellFormat::ULargeToBuffer(ULARGE_INTEGER uilSize, LPWSTR pszBuffer, size_t cchBuffer)
{
   WCHAR szNumber[30];
   HRESULT hr = ::StringCchPrintf(szNumber,
                                lengthof(szNumber),
                                TEXT("%I64u"),
                                uilSize.QuadPart);

   if (SUCCEEDED(hr))
   {
      WCHAR szTemp[30];
      NUMBERFMT nfmt = {0};

      if (0 != GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING,
                             szTemp, lengthof(szTemp)))
      {
         nfmt.Grouping = _ttoi(szTemp);

         if (0 != GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND,
                                szTemp, lengthof(szTemp)))
         {
            nfmt.lpDecimalSep = nfmt.lpThousandSep = szTemp;

            if (0 != GetNumberFormat(LOCALE_USER_DEFAULT, 0, szNumber,
                                     &nfmt, pszBuffer, cchBuffer))
            {
               return TRUE;
            }
         }
      }
   }

   return FALSE;
}

BOOL ShellFormat::FileInfo(LPCWSTR pszFile, LPWSTR pszFileInfo, size_t cchFileInfo)
{
   if (!pszFile)
   {
      return FALSE;
   }

   WIN32_FIND_DATA fd;
   HANDLE hfind = FindFirstFile(pszFile, &fd);
   ULARGE_INTEGER uliSize;
   WCHAR szSize[32];
   FILETIME ftLocal;
   SYSTEMTIME stLocal;
   WCHAR szDate[64];
   WCHAR szTime[64];
   LPWSTR pszModified;

   if (INVALID_HANDLE_VALUE == hfind)
   {
      return FALSE;
   }
   FindClose(hfind);

   // Get the file size
   uliSize.HighPart = fd.nFileSizeHigh;
   uliSize.LowPart = fd.nFileSizeLow;
   FileSizeShort(uliSize, szSize, lengthof(szSize));

   // Get the date & time
   FileTimeToLocalFileTime(&fd.ftLastWriteTime, &ftLocal);
   FileTimeToSystemTime(&ftLocal, &stLocal);
   GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &stLocal, NULL, szDate, lengthof(szDate));
   GetTimeFormat(LOCALE_USER_DEFAULT, 0, &stLocal, NULL, szTime, lengthof(szTime));

   // Build the file info string
   pszModified = LOAD_STRING(IDS_FORMAT_MODIFIED);
   return SUCCEEDED(::StringCchPrintfEx(pszFileInfo,
                 cchFileInfo,
                 NULL,
                 NULL,
                 STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                 TEXT("%s\r\n%s %s, %s"),
                 szSize,
                 pszModified,
                 szDate,
                 szTime));
}

BOOL ShellFormat::FileTimeInfo(LPCWSTR pszFile, LPWSTR pszFileTimeInfo, size_t cchFileTimeInfo)
{   
   FILETIME ft = {0};
   SYSTEMTIME st = {0};
   WIN32_FIND_DATA fd = {0};
   BOOL fReturn = FALSE;

   ASSERT(pszFile);
   ASSERT(pszFileTimeInfo);
   ASSERT(cchFileTimeInfo);

   if (!pszFile || ! pszFileTimeInfo || !cchFileTimeInfo)
   {
      return fReturn;
   }

   HANDLE hfind = FindFirstFile(pszFile, &fd);
   if (INVALID_HANDLE_VALUE != hfind)
   {          
      FindClose(hfind);

      // Get the date & time
      FileTimeToLocalFileTime(&fd.ftLastWriteTime, &ft);
      FileTimeToSystemTime(&ft, &st);

      GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, pszFileTimeInfo, cchFileTimeInfo);
      StringCchCatEx(pszFileTimeInfo, cchFileTimeInfo, L" ", &pszFileTimeInfo, NULL, STRSAFE_NULL_ON_FAILURE);
      GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, pszFileTimeInfo, cchFileTimeInfo);
      fReturn = TRUE;
   }

   return fReturn;
}

BOOL ShellFormat::FileSizeShort(ULARGE_INTEGER uilSize, LPWSTR pszSize, size_t cchSize)
{
   enum                   {     ORDER_BYTES,     ORDER_KB,     ORDER_MB,     ORDER_GB,     ORDER_TB };
   const UINT uOrders[] = { IDS_ORDER_BYTES, IDS_ORDER_KB, IDS_ORDER_MB, IDS_ORDER_GB, IDS_ORDER_TB };

   ULARGE_INTEGER uliTemp = uilSize;
   ULARGE_INTEGER uliInteger;
   DWORD dwDecimal = 0;
   UINT uOrder = 0;
   WCHAR szTemp[30];
   BOOL twoDigitsPrecision = TRUE;

   if (1000L > uilSize.QuadPart)
   {
      ::StringCbPrintfW(szTemp, sizeof(szTemp), L"%d", uilSize.LowPart);
      goto doFormat;
   }

   // Shift until it is a printable number
   uOrder = ORDER_KB;
   while ((uliTemp.QuadPart >= (1000L * 1024L)) && (lengthof(uOrders)-1 > uOrder))
   {
      uliTemp.QuadPart >>= 10;
      uOrder++;
   }

   uliInteger.QuadPart = uliTemp.QuadPart >> 10;

   // Include fractional part for two-digits sizes or 1MB and higher orders
   if (100 > uliInteger.LowPart || uOrder > ORDER_KB)
   {
      DWORD dwTmp;

      uliTemp.QuadPart = (uliTemp.QuadPart - (uliInteger.QuadPart << 10));

      // At this point, dwDecimal should be between 0 and 1000
      // we want get the top one (or two) digits.
      dwDecimal = uliTemp.LowPart * 1000 / 1024;
      dwTmp = dwDecimal;
      dwDecimal /= 10;

      // one-digit precision for orders less than 1GB
      // if the number of digits before the decimal point is 2 or more
      // e.g.: 10.1KB - one digit after the point
      //       3.05KB - two digits after the point because there is only one digit before the point
      if (uliInteger.LowPart >= 10 && uOrder < ORDER_GB)
      {
         twoDigitsPrecision = FALSE;
         dwTmp = dwDecimal;
         dwDecimal /= 10;
      }

      if (dwDecimal)
      {
         // Round properly
         dwTmp %= 10;                  
         if (5 < dwTmp)
         {
            if (9 == dwDecimal || 99 == dwDecimal)
            {
                uliInteger.LowPart += 1;
                dwDecimal = 0;
            }
            else
            {
                dwDecimal += 1;
            }  
         }
      }
   }

   if (!ULargeToBuffer(uliInteger, szTemp, lengthof(szTemp)))
   {
      return FALSE;
   }

   if (dwDecimal)
   {
      size_t len = ::wcslen(szTemp);

      if (0 != GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL,
                    szTemp+len, lengthof(szTemp)-len))
      {
          len = wcslen(szTemp);
          StringCchPrintf(szTemp + len
                        , lengthof(szTemp) - len
                        , twoDigitsPrecision ? L"%02d" : L"%d"
                        , dwDecimal);

          ASSERT(twoDigitsPrecision || dwDecimal < 10
                  && "error formatting file size: "
                     "if we have only one digit, dwDecimal should be less than 10");
      }
   }

doFormat:
   return SUCCEEDED(::StringCchPrintfEx(pszSize,
                 cchSize,
                 NULL,
                 NULL,
                 STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                 LOAD_STRING(uOrders[uOrder]),
                 szTemp));
}

BOOL ShellFormat::FolderStatus(UINT uCount, UINT uMsg, LPWSTR pszStatusText, size_t cchStatusText)
{
   ULARGE_INTEGER uliTemp;
   WCHAR szNumber[30];

   uliTemp.QuadPart = uCount;
   if (!ShellFormat::ULargeToBuffer(uliTemp, szNumber, lengthof(szNumber)))
   {
      return FALSE;
   }

   return SUCCEEDED(::StringCchPrintfEx(pszStatusText,
                 cchStatusText,
                 NULL,
                 NULL,
                 STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                 LOAD_STRING(uMsg),
                 szNumber));
}

/*-------------------------------------------------------------------------
Purpose: Constructs a displayname form of the file time.

         dwFlags may be 0, in which case FDTF_DEFAULT is assumed.  Other
         valid flags are:

            FDTF_DEFAULT    "3/29/98 7:48 PM"
            FDTF_SHORTTIME  "7:48 PM"
            FDTF_SHORTDATE  "3/29/98"
            FDTF_LONGDATE   "Monday, March 29, 1998"
            FDTF_LONGTIME   "7:48:33 PM"
*/
BOOL ShellFormat::FormatDateTime(const FILETIME* pftInput, DWORD dwFlags, LPWSTR pwszBuffer, size_t cchBuffer)
{
#define LRM 0x200E // UNICODE Left-to-right mark control character
#define RLM 0x200F // UNICODE Left-to-right mark control character

    BOOL bRet = FALSE;
    FILETIME ftLocal;
    SYSTEMTIME st;
    DWORD dwDateFlags;
    DWORD dwTimeFlags;
    size_t cchRemaining;

    if (!pftInput || !pwszBuffer || !cchBuffer)
    {
        goto leave;
    }

    cchRemaining = cchBuffer;

    if (!dwFlags)
    {
        // Assume short date and short time
        dwFlags = FDTF_DEFAULT;
    }

    ::FileTimeToLocalFileTime(pftInput, &ftLocal);
    ::FileTimeToSystemTime(&ftLocal, &st);

    dwDateFlags = DATE_SHORTDATE;     // default
    dwTimeFlags = TIME_NOSECONDS;     // default

    // Initialize the flags we're going to use
    if (dwFlags & FDTF_LONGDATE)
    {
        dwDateFlags = DATE_LONGDATE;
    }

    if (dwFlags & FDTF_LTRDATE)
    {
        dwDateFlags |= DATE_LTRREADING;
    }
    else if(dwFlags & FDTF_RTLDATE)
    {
        dwDateFlags |= DATE_RTLREADING;
    }

    if (dwFlags & FDTF_LONGTIME)
    {
        dwTimeFlags &= ~TIME_NOSECONDS;
    }

    if (dwFlags & (FDTF_LONGDATE | FDTF_SHORTDATE))
    {
        // Get the date
        ::GetDateFormat(LOCALE_USER_DEFAULT,
                    dwDateFlags,
                    &st,
                    NULL,
                    pwszBuffer,
                    cchBuffer);

        // Are we tacking on the time too?
        if (dwFlags & (FDTF_SHORTTIME | FDTF_LONGTIME))
        {
            HRESULT hr;
            LPCWSTR pwszSeparator;

            // Yes; for long dates, separate with a comma, otherwise
            // separate with a space.
            if (dwFlags & FDTF_LONGDATE)
            {
                pwszSeparator = L", ";
            }
            else
            {
                pwszSeparator = L" ";
            }

            hr = ::StringCchCatEx(pwszBuffer,
                            cchBuffer,
                            pwszSeparator,
                            &pwszBuffer,
                            &cchRemaining,
                            STRSAFE_NULL_ON_FAILURE);
            if (FAILED(hr))
            {
                goto leave;
            }

            // need to insert strong a Unicode control character to simulate
            // a strong run in the opposite base direction to enforce
            // correct display of concatinated string in all cases
            if (dwFlags & FDTF_RTLDATE)
            {
                if (cchRemaining >= 2)
                {
                    *pwszBuffer++ = LRM; // simulate an opposite run
                    *pwszBuffer++ = RLM; // force RTL display of the time part.
                    *pwszBuffer = 0;
                    cchRemaining -= 2;
                }
            }
            else if (dwFlags & FDTF_LTRDATE)
            {
                if (cchRemaining >= 2)
                {
                    *pwszBuffer++ = RLM; // simulate an opposite run
                    *pwszBuffer++ = LRM; // force LTR display of the time part.
                    *pwszBuffer = 0;
                    cchRemaining -= 2;
                }
            }
        }
    }

    if (dwFlags & (FDTF_SHORTTIME | FDTF_LONGTIME))
    {
        // Get the time
        ::GetTimeFormat(LOCALE_USER_DEFAULT,
                    dwTimeFlags,
                    &st,
                    NULL,
                    pwszBuffer,
                    cchRemaining);
    }

    bRet = TRUE;

leave:
    return bRet;
}

DWORD ShellDialogs::Properties::ObjectPropertiesThread(LPVOID lpParameter)
{   
   BOOL fMultTypes = FALSE;
   BOOL fGetTargetAgain = FALSE;
   BOOL fUnderRecBin = FALSE;
   DWORD dwAttrib = 0;  
   GeneralPropData * pGPD = NULL;
   HRESULT hr;

   ObjectPropertiesData * pOPD = (ObjectPropertiesData *) lpParameter;
   IShellFolder * pFolder = pOPD->pFolder;
   IShellView * pView = pOPD->pView;
   LPCITEMIDLIST * aPidls = pOPD->aPidls;
   LPCITEMIDLIST pidl = pOPD->pidl;
   LPITEMIDLIST * ppidl = (LPITEMIDLIST *)aPidls;
   LPITEMIDLIST pidltmp = NULL, pidlLink;
   
   PROPSHEETPAGE psp[2] = {0};
   PROPSHEETHEADER psh = {0};
   SHFILEINFO sfi = {0};
   ShortcutPropData * pSPD = NULL;
   WCHAR szDisplayName[MAX_PATH];
   WCHAR szMsg[MAX_PATH];
   WCHAR szTmp[MAX_PATH];   
   UINT i = 0;
   UINT uDatabsCount = 0;
   UINT uFilesCount = 0;
   UINT uFoldersCount = 0;
   UINT uItemCount = pOPD->uItemCount;
   ULARGE_INTEGER uliTotalSize;

   if (!pidl || !pFolder || !aPidls || !uItemCount)
   {
      goto ObjectPropertiesThread_done;
   }

   pGPD = (GeneralPropData *)LocalAlloc(LPTR, sizeof(GeneralPropData));
   if (!pGPD)
   {
      goto ObjectPropertiesThread_done;
   }

   pSPD = (ShortcutPropData *)LocalAlloc(LPTR, sizeof(ShortcutPropData));
   if (!pSPD)
   {
      goto ObjectPropertiesThread_done;
   }
   
   pSPD->pGPD = pGPD;
   pGPD->dwMinAttributes = 0xffff;
   pGPD->dwMaxAttributes = 0;
   pGPD->bHideAttributes = FALSE;
   *pGPD->szType = 0;
   
   if (ILIsNameSpace(pidl, CLSID_CEShellBitBucket))
   {
      fUnderRecBin = TRUE;      
   }
   
   psp[0].dwSize = sizeof(PROPSHEETPAGE);
   psp[0].dwFlags = PSP_USETITLE;
   psp[0].hInstance = HINST_CESHELL;
   psp[0].pszTemplate = MAKEINTRESOURCE(IDD_PROPERTY);
   psp[0].pfnDlgProc = GeneralPropDlgProc;
   psp[0].pszTitle = MAKEINTRESOURCE(IDS_TITLE_PROPERTIES);
   psp[0].lParam = (LPARAM)(LPVOID)pGPD;

   psh.nPages = 1; //Increment for a shortcut
   psh.dwSize = sizeof(PROPSHEETHEADER);
   psh.dwFlags = PSH_PROPSHEETPAGE | PSH_PROPTITLE;
   psh.hInstance = HINST_CESHELL;
   psh.pszCaption = L"";
   psh.ppsp = (LPCPROPSHEETPAGE) psp;

   //Gather some information on all selected items.      
   IShellFolder * pDesktopFolder;
   hr = SHGetDesktopFolder(&pDesktopFolder);
   if (FAILED(hr))
   {
      ASSERT(FALSE);
   }

   uliTotalSize.LowPart = 0;
   uliTotalSize.HighPart = 0;

   for (i = 0; i < uItemCount; i++)
   {   
      STRRET str = {0};     
      szDisplayName[0] = TEXT('\0');
      pidltmp = *ppidl;
      UINT uFlags = SHGFI_LARGEICON | SHGFI_ICON | SHGFI_TYPENAME;
             
      if (fUnderRecBin)
      {   
         g_pRecBin->BeginRecycle();
         hr = pFolder->GetDisplayNameOf(pidltmp, SHGDN_NORMAL | SHGDN_FORPARSING, &str);
         g_pRecBin->EndRecycle();
      }
      else
      {
         //Get the name which is relative to the desktop 
         hr = pFolder->GetDisplayNameOf(pidltmp, SHGDN_NORMAL | SHGDN_FORPARSING, &str);     
      }
     
      StrRetToBuf(&str, pidltmp, szDisplayName, lengthof(szDisplayName));
      SHGetFileInfo(szDisplayName, 0, &sfi, sizeof(SHFILEINFO), uFlags);

      if (!fMultTypes)
      {
         if (*pGPD->szType)
         {
            if (CSTR_EQUAL != CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                            pGPD->szType, -1, sfi.szTypeName, -1))
            {
               fMultTypes = TRUE;   
            }
         }
         else
         {
            ::wcscpy(pGPD->szType, sfi.szTypeName);
         }
      }

      if (ILIsFileSystemPidl(pidltmp))
      {                              
         dwAttrib = GetFileAttributes(szDisplayName);

         if (-1 != dwAttrib)
         {
            pGPD->dwMaxAttributes |= dwAttrib;
            pGPD->dwMinAttributes &= dwAttrib;
         }


         ULONG ulAttrs = SFGAO_FOLDER;
         hr = pFolder->GetAttributesOf(1, (LPCITEMIDLIST*) &pidltmp, &ulAttrs);  
         if (SUCCEEDED(hr) && (ulAttrs & SFGAO_FOLDER))
         {
            ++uFoldersCount;
         } 
         else
         {
            ++uFilesCount;
            
            WIN32_FILE_ATTRIBUTE_DATA attribEx = {0};    
            if (GetFileAttributesEx(szDisplayName, GetFileExInfoStandard, &attribEx))
            {
               ULARGE_INTEGER uliSize;
               uliSize.LowPart = attribEx.nFileSizeLow;
               uliSize.HighPart = attribEx.nFileSizeHigh;
                              
               uliTotalSize.LowPart += uliSize.LowPart;
               uliTotalSize.HighPart += uliSize.HighPart;
            }                  
         }         
      }
      else
      {
         ++uFoldersCount;
      }

      ppidl++;
   }
   
   ShellFormat::FileSizeShort(uliTotalSize, pGPD->szSize, lengthof(pGPD->szSize));

   if (0 == uFilesCount && 0 == uFoldersCount)
   {
      //No objects selected
      pGPD->bMultiSelect = FALSE;
   } 
   else if (1 == (uFilesCount + uFoldersCount)) 
   {
      //One object is selected
      pGPD->bMultiSelect = FALSE;      
      pGPD->hicon = sfi.hIcon;

      if(ILIsNameSpacePidl(pidltmp))
      {
         *pGPD->szSize = 0;
         *pGPD->szModified = 0;
         pGPD->bHideAttributes = TRUE;                  
         StringCchCopyEx(pGPD->szFilename, MAX_PATH,
                         PathFindFileName(szDisplayName),
                         NULL, NULL, STRSAFE_NULL_ON_FAILURE);
      }
      else 
      {            
         StringCchCopyEx(pGPD->szPath, MAX_PATH, szDisplayName,
                         NULL, NULL, STRSAFE_NULL_ON_FAILURE);
                  
         *pGPD->szModified = 0;
         ShellFormat::FileTimeInfo(pGPD->szPath, pGPD->szModified, lengthof(pGPD->szModified));

         if (PathIsLink(pGPD->szPath) && SHGetShortcutTarget(pGPD->szPath, pSPD->szTarget, MAX_PATH))
         {                        
            ULONG ulEaten;
                        
            pSPD->bModifyTarget = (fUnderRecBin) ? FALSE : TRUE;

            PathRemoveQuotesAndArgs(pSPD->szTarget);

            if (SUCCEEDED(PathIsGUID(pSPD->szTarget))) // PathIsGUID returns S_OK if TRUE
            {
               // Prepend the guid with :: so the shell recognizes it
               for (int j = _tcslen(pSPD->szTarget); 0 < j; j--)
                  pSPD->szTarget[j+1] = pSPD->szTarget[j-1];
               pSPD->szTarget[1] = TEXT(':');
               pSPD->szTarget[0] = TEXT(':');
            }

            hr = pDesktopFolder->ParseDisplayName(NULL, NULL, pSPD->szTarget, &ulEaten, &pidlLink, NULL);
            if (SUCCEEDED(hr))
            {
               //Handle the Shortcut target
               *pSPD->szTargetSize = 0;
               *pSPD->szTargetModified = 0;

               if (ILIsNameSpacePidl(ILFindLast(pidlLink)))
               { 
                  fGetTargetAgain = TRUE;
                  StringCchCopyEx(pSPD->szTarget, lengthof(pSPD->szTarget),
                                  ILDisplayName(pidl), NULL, NULL,
                                  STRSAFE_NULL_ON_FAILURE);

                  LoadString(HINST_CESHELL, IDS_PROPERTIES_SYSTEMFOLDER,
                             pSPD->szTargetType, lengthof(pSPD->szTargetType));
                     
                  pSPD->bModifyTarget = FALSE;
               } 
               else
               {                                    
                  //Get the target path                  
                  SHGetPathFromIDList(pidlLink, szTmp);
                  SHGetFileInfo(szTmp, 0, &sfi, sizeof(SHFILEINFO), SHGFI_TYPENAME);
                  
                  ::wcscpy(pSPD->szTargetType, sfi.szTypeName);

                  dwAttrib = GetFileAttributes(szTmp);                                    
                  if ((-1 != dwAttrib) && !(FILE_ATTRIBUTE_DIRECTORY & dwAttrib))
                  {
                     //Get the size of the Target
                     WIN32_FIND_DATA fd;
                     HANDLE hFile = FindFirstFile(szTmp, &fd);
                     if (INVALID_HANDLE_VALUE != hFile)
                     {                                    
                        ULARGE_INTEGER uliSize;
                        uliSize.HighPart = fd.nFileSizeHigh;
                        uliSize.LowPart = fd.nFileSizeLow;
                        FindClose(hFile);
                                              
                        ShellFormat::FileSizeShort(uliSize, pSPD->szTargetSize, lengthof(pSPD->szTargetSize));                        
                     }

                     ShellFormat::FileTimeInfo(szTmp, pSPD->szTargetModified, lengthof(pSPD->szTargetModified));
                  }
               }

               ILFree(pidlLink);
            }
            else
            {
               LoadString(HINST_CESHELL, IDS_PROPERTIES_ERR_NOTARGET, pSPD->szTargetType, lengthof(pSPD->szTargetType));
            }

            if (!fGetTargetAgain)
            {
               //Put back any quotes or arguments               
               SHGetShortcutTarget(pGPD->szPath, pSPD->szTarget, MAX_PATH);
            }

            psp[1].dwSize = sizeof(PROPSHEETPAGE);
            psp[1].dwFlags = PSP_USETITLE;
            psp[1].hInstance = HINST_CESHELL;
            psp[1].pszTemplate = MAKEINTRESOURCE(IDD_SHORTCUT);
            psp[1].pfnDlgProc = ShortcutPropDlgProc;
            psp[1].pszTitle = MAKEINTRESOURCE(IDS_TITLE_SHORTCUT);
            psp[1].lParam = (LPARAM)(LPVOID)pSPD;
            psh.nPages++;
         }
         else if (PathIsDirectory(pGPD->szPath))
         {
            *pGPD->szSize = 0;
            *pGPD->szModified = 0;
            dwAttrib = GetFileAttributes(pGPD->szPath);
            if ((-1 != dwAttrib) && (FILE_ATTRIBUTE_TEMPORARY & dwAttrib))
            {                              
               pGPD->GetDiskSpace();
               pGPD->bRemovableStorage = TRUE;
            }
            pGPD->bHideAttributes = TRUE;
         }
         else if (0 < uDatabsCount)
         {
            pGPD->bHideAttributes = TRUE;
         }

         if (ILIsNameSpace(pidl, CLSID_CEShellBitBucket) && fUnderRecBin)
         {
            *pGPD->szModified = 0;
            pGPD->bHideAttributes = TRUE;

            g_pRecBin->BeginRecycle();
            LPOLESTR str = g_pRecBin->GetFileOrigName(pidltmp);
            g_pRecBin->EndRecycle();
            if (str)
            {
               // Set empty string on failure
               StringCchCopyEx(pGPD->szFilename, MAX_PATH, (LPCWSTR)str, NULL, NULL, STRSAFE_NULL_ON_FAILURE);
               
               if (g_pShellMalloc)
               {
                  g_pShellMalloc->Free(str);
               }
            }

            if ((!UserSettings::GetShowExtensions() && !PathIsDirectory(pGPD->szFilename)) ||
                PathIsLink(pGPD->szFilename))
            {
               PathRemoveExtension(pGPD->szFilename);
            }
         }
         else
         {
            StringCchCopyEx(szTmp, MAX_PATH, pGPD->szPath, NULL, NULL, STRSAFE_NULL_ON_FAILURE);
            
            if ((!UserSettings::GetShowExtensions() && !PathIsDirectory(szTmp)) ||
                PathIsLink(szTmp))
            {
               PathRemoveExtension(szTmp);
            }

            ::wcscpy(pGPD->szFilename, PathFindFileName(szTmp));
         }
      }
    
      if (fUnderRecBin)
      {
         StringCchCopyEx(pGPD->szLocation, MAX_PATH, TEXT("Recycle Bin"), NULL, NULL, STRSAFE_NULL_ON_FAILURE);         
      }
      else
      {
         GetLocation(pidl, pGPD->szLocation);
      }
   }
   else
   {
      //Multiple selected items    
      pGPD->bMultiSelect = TRUE;
      LoadString(HINST_CESHELL, IDS_PROPERTIES_NUMOFFILES, szMsg, MAX_PATH);
      ::wsprintf(pGPD->szFilename, szMsg, uFilesCount, uFoldersCount);
      
      if (uFilesCount && uFoldersCount)
      {
         fMultTypes = TRUE;
      }

      if (fMultTypes)
      {
         LoadString(HINST_CESHELL, IDS_PROPERTIES_TYPEMULTI, pGPD->szType, lengthof(pGPD->szType));
      }
      else
      {    
         szTmp[0] = TEXT('\0');
         LoadString(HINST_CESHELL, IDS_PROPERTIES_TYPEALL, szMsg, MAX_PATH);
         ::wcscpy(szTmp, pGPD->szType);
         ::wsprintf(pGPD->szType, szMsg, szTmp);
      }

      pGPD->hicon = LoadIcon(HINST_CESHELL, MAKEINTRESOURCE(IDI_MULTIPLEFILES));
      LoadString(HINST_CESHELL, IDS_PROPERTIES_ALLIN, szMsg, MAX_PATH);
       
      if (fUnderRecBin)
      {  
         StringCchCopyEx(szTmp, MAX_PATH, TEXT("Recycle Bin"), NULL, NULL, STRSAFE_NULL_ON_FAILURE);         
      }
      else
      {
         szTmp[0] = TEXT('\0');  
         GetLocation(pidl, szTmp);
      }
      ::wsprintf(pGPD->szLocation, szMsg, szTmp);

      *pGPD->szModified = 0;         

      if (0 == uFilesCount)
      {
         pGPD->bHideAttributes = TRUE;
         *pGPD->szSize = 0;
      }

      if ((0 < uDatabsCount) || fUnderRecBin)
      {
         pGPD->bHideAttributes = TRUE;
      }            
   }

   pDesktopFolder->Release();

   //Set dialog caption
   psh.pszCaption = pGPD->szFilename;

   // Initialize the extra common controls since IDD_SHORTCUT requires it
   if ( LoadAygshellLibrary() )
   {
       AygInitExtraControls();
   }
   
   if (PropertySheet(&psh))
   {
      // Check if we have to set any attributes.
      if (pGPD->dwMaxAttributes)
      {
         BOOL fRefreshView = FALSE;
         ppidl = (LPITEMIDLIST *)aPidls;
         for (i = 0; i < uItemCount; i++)
         {
            pidltmp = *ppidl;

            if (SUCCEEDED(UpdateAttributes(pFolder, pidltmp, pGPD->dwMinAttributes, pGPD->dwMaxAttributes)))
            {
               fRefreshView = TRUE;
            }
            else
            {
               break;
            }

            ppidl++;
         }

         if (fRefreshView)
         {
            pView->Refresh();
         }
      }

      if (1 < psh.nPages)
      {
         if (pSPD->bModifyTarget && pSPD->bNeedToSetTheTarget)
         {    
            //This case is only for one selected item
            ppidl = (LPITEMIDLIST *)aPidls;
            if (1 == uItemCount)
            {
               STRRET str = {0}; 
               pidltmp = *ppidl;
                    
               *pGPD->szPath = 0;

               //Get the name which is relative to the desktop 
               if (SUCCEEDED(pFolder->GetDisplayNameOf(pidltmp, SHGDN_NORMAL | SHGDN_FORPARSING, &str)) &&
                  SUCCEEDED(StrRetToBuf(&str, pidltmp, pGPD->szPath, lengthof(pGPD->szPath))))
               {   
                  dwAttrib = GetFileAttributes(pGPD->szPath);
                  if ((-1 != dwAttrib) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
                  {
                     SetFileAttributes(pGPD->szPath, 0);
                  }

                  DeleteFile(pGPD->szPath);    
                  SHCreateShortcut(pGPD->szPath, pSPD->szTarget);
               }
               
            }      
         }
      }
   }
   FreeAygshellLibrary();   

ObjectPropertiesThread_done:

   if (pSPD)
   {
      LocalFree(pSPD);
   }

   if (pGPD)
   {
      if (pGPD->hicon)
      {
         DestroyIcon(pGPD->hicon);  
      }
      LocalFree(pGPD);
   }

   if (pidl)
   {
      ILFree(pidl);
   }

   if (pFolder)
   {
      pFolder->Release();
   }

   if (pView)
   {
      pView->Release();
   }

   if (aPidls)
   {
      for (i = 0; i < uItemCount; i++)
      {
         ILFree(aPidls[i]);
      }
      LocalFree(aPidls);
   }

   if (pOPD)
   {
      LocalFree(pOPD);
   }

   return 0;
}

DWORD ShellDialogs::Properties::FolderObjectPropertiesThread(LPVOID lpParameter)
{
   FolderObjectPropertiesData * pFOPD = (FolderObjectPropertiesData *) lpParameter;
   LPCITEMIDLIST pidl = pFOPD->pidl;
   IShellFolder * pFolder = pFOPD->pFolder;
   PROPSHEETPAGE psp = {0};
   PROPSHEETHEADER psh = {0};
   GeneralPropData * pGPD = NULL;   
   SHFILEINFO sfi = {0};
   DWORD dwAttrib;
   IShellFolder * pParentFolder = NULL;
   LPCITEMIDLIST pidlLast = NULL;
   STRRET str = {0};
   
   ASSERT(pidl);
   ASSERT(pFolder);
   if (!pidl || !pFolder)
   {
      goto FolderObjectPropertiesThread_done;
   }

   pGPD = (GeneralPropData *)LocalAlloc(LPTR, sizeof(GeneralPropData));
   if (NULL == pGPD)
   {
      goto FolderObjectPropertiesThread_done;
   }
      
   psp.dwSize = sizeof(PROPSHEETPAGE);
   psp.dwFlags = PSP_USETITLE;
   psp.hInstance = HINST_CESHELL;
   psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPERTY);
   psp.pfnDlgProc = GeneralPropDlgProc;
   psp.pszTitle = MAKEINTRESOURCE(IDS_TITLE_GENERAL);
   psp.lParam = (LPARAM)(LPVOID)pGPD;

   psh.nPages = 1; 
   psh.dwSize = sizeof(PROPSHEETHEADER);
   psh.dwFlags = PSH_PROPSHEETPAGE;
   psh.hInstance = HINST_CESHELL;
   psh.pszCaption = MAKEINTRESOURCE(IDS_TITLE_PROPERTIES);
   psh.ppsp = (LPCPROPSHEETPAGE) &psp;

   pGPD->bMultiSelect = FALSE;
       
   if (SUCCEEDED(SHBindToParent(pidl, IID_IShellFolder, (VOID **) &pParentFolder, &pidlLast)))   
   {                      
      if (SUCCEEDED(pParentFolder->GetDisplayNameOf(pidlLast, SHGDN_NORMAL | SHGDN_FORPARSING, &str)))
      {
         StrRetToBuf(&str, pidlLast, pGPD->szPath, lengthof(pGPD->szPath));                                          
      }
   }

   if (pParentFolder)
   {
      pParentFolder->Release();
   }

   if (pidlLast)
   {
      ILFree(pidlLast);
   }

   if (PathIsDirectory(pGPD->szPath))
   {
      UINT uFlags = SHGFI_LARGEICON | SHGFI_ICON | SHGFI_TYPENAME;
      SHGetFileInfo(pGPD->szPath, 0, &sfi, sizeof(SHFILEINFO), uFlags);

      pGPD->hicon = sfi.hIcon;
      ::wcscpy(pGPD->szType, sfi.szTypeName);

      *pGPD->szSize = 0;
      *pGPD->szModified = 0;
      pGPD->bHideAttributes = TRUE;
   
      dwAttrib = GetFileAttributes(pGPD->szPath);
      if ((-1 != dwAttrib) && (FILE_ATTRIBUTE_TEMPORARY & dwAttrib))
      {            
         pGPD->GetDiskSpace();
         pGPD->bRemovableStorage = TRUE;
      }

      StringCchCopyEx(pGPD->szFilename, MAX_PATH, PathFindFileName(pGPD->szPath), NULL, NULL, STRSAFE_NULL_ON_FAILURE);

      UINT uCount = ILCount(pidl);
      if (uCount)
      {  
         LPCITEMIDLIST pidlParent = ILCopy(pidl, uCount-1);
         if (SUCCEEDED(SHBindToParent(pidlParent, IID_IShellFolder, (VOID **) &pParentFolder, &pidlLast)))   
         {                      
            if (SUCCEEDED(pParentFolder->GetDisplayNameOf(pidlLast, SHGDN_NORMAL | SHGDN_FOREDITING, &str)))
            {
               StrRetToBuf(&str, pidlLast, pGPD->szLocation, lengthof(pGPD->szLocation));                                          
            }
         }

         if (pParentFolder)
         {
            pParentFolder->Release();
         }

         if (pidlLast)
         {
            ILFree(pidlLast);
         }

         if (pidlParent)
         {
            ILFree(pidlParent);   
         }
      }
   }
   else
   {
      ASSERT(FALSE);
   }

   PropertySheet(&psh);

FolderObjectPropertiesThread_done:

   if (pGPD)
   {
      if (pGPD->hicon)
      {
         DestroyIcon(pGPD->hicon);  
      }
      LocalFree(pGPD);
   }

   if (pidl)
   {
      ILFree(pidl);
   }

   if (pFolder)
   {
      pFolder->Release();
   }

   if (pFOPD)
   {
      LocalFree(pFOPD);
   }

   return 0;
}

void ShellDialogs::Properties::ObjectProperties(LPCITEMIDLIST pidl, IShellFolder * pFolder, IShellView * pView, LPCITEMIDLIST * aPidls, UINT uItemCount)
{
   if (!pidl || !pFolder || !pView || !aPidls || !uItemCount)
   {
      return;
   }

   ObjectPropertiesData * popd = (ObjectPropertiesData *) LocalAlloc(LPTR, sizeof(ObjectPropertiesData));
   if (popd)
   {
      popd->pidl = ILCopy(pidl, IL_ALL);
      popd->pFolder = pFolder;
      popd->pFolder->AddRef();
      popd->pView = pView;
      popd->pView->AddRef();
      popd->aPidls = (LPCITEMIDLIST *) g_pShellMalloc->Alloc(uItemCount * sizeof(LPITEMIDLIST));
      if (popd->aPidls)
      {
         for (UINT i = 0; i < uItemCount; i++)
         {
            if (aPidls[i])
            {
               popd->aPidls[i] = ILCopy(aPidls[i], IL_ALL);
            }
         }
      }
      popd->uItemCount = uItemCount;

      HANDLE hThread = ::CreateThread(NULL, 0, ObjectPropertiesThread, popd, 0, NULL);
      if (hThread)
      {
         ::CloseHandle(hThread);
      }
   }
}
       
void ShellDialogs::Properties::FolderObjectProperties(LPCITEMIDLIST pidl, IShellFolder * pFolder)
{
   if (!pidl || !pFolder)
   {
      return;
   }

   FolderObjectPropertiesData * pfopd = (FolderObjectPropertiesData *) LocalAlloc(LPTR, sizeof(FolderObjectPropertiesData));
   if (pfopd)
   {
      pfopd->pidl = ILCopy(pidl, IL_ALL);
      pfopd->pFolder = pFolder;
      pfopd->pFolder->AddRef();

      HANDLE hThread = ::CreateThread(NULL, 0, FolderObjectPropertiesThread, pfopd, 0, NULL);
      if (hThread)
      {
         ::CloseHandle(hThread);
      }
   }
}

int CALLBACK ShellDialogs::Properties::GeneralPropDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{   
   GeneralPropData * pGPD = (GeneralPropData *)GetWindowLong(hDlg, DWL_USER);

   switch(uMsg)
   {
      case WM_INITDIALOG:
      {
         HWND hwndCtrl;
         UINT fCheck;
         LPPROPSHEETPAGE ppsh = (LPPROPSHEETPAGE)lParam;         
         pGPD = (GeneralPropData *)ppsh->lParam;

         SetWindowLong(hDlg, DWL_USER, (LONG)pGPD);

         SendDlgItemMessage(hDlg, IDC_PICON, STM_SETIMAGE, 
                        (WPARAM) IMAGE_ICON, (LPARAM) pGPD->hicon);
         SetDlgItemTextEllipses(hDlg, IDC_FILENAME, pGPD->szFilename);
         SetDlgItemTextEllipses(hDlg, IDC_TYPE, pGPD->szType);
         SetDlgItemTextEllipses(hDlg, IDC_LOCATION, pGPD->szLocation);

         AygInitDialog( hDlg, SHIDIF_SIPDOWN );

         if (pGPD->bRemovableStorage)
         {
            WCHAR szLabel[30];      
            LoadString(HINST_CESHELL, IDS_PROPERTIES_USED, szLabel, 30);        
            SetDlgItemText(hDlg, IDC_SIZELABEL, szLabel);
            
            LoadString(HINST_CESHELL, IDS_PROPERTIES_FREE, szLabel, 30);
            SetDlgItemText(hDlg, IDC_MODIFIEDLABEL, szLabel);
         }

         if(!::wcslen(pGPD->szSize))
         {
            HideDlgItem(hDlg, IDC_SIZELABEL);
            HideDlgItem(hDlg, IDC_PSIZE);
         }
         else
         {
            SetDlgItemTextEllipses(hDlg, IDC_PSIZE, pGPD->szSize);
         }

         if(!::wcslen(pGPD->szModified))
         {
            HideDlgItem(hDlg, IDC_MODIFIEDLABEL);
            HideDlgItem(hDlg, IDC_MODIFIED);
         }
         else
         {
            SetDlgItemText(hDlg, IDC_MODIFIED, pGPD->szModified);
         }

         if(pGPD->bHideAttributes)
         {            
            HideDlgItem(hDlg, IDC_READONLY);
            HideDlgItem(hDlg, IDC_HIDDEN);
            HideDlgItem(hDlg, IDC_ARCHIVE);
            HideDlgItem(hDlg, IDC_SYSTEM);
            HideDlgItem(hDlg, IDC_GROUPBOX);
         }
         else
         {
            fCheck = (pGPD->dwMaxAttributes & FILE_ATTRIBUTE_READONLY) ? 1 : 0;
            hwndCtrl = GetDlgItem(hDlg, IDC_READONLY);

            if(pGPD->bMultiSelect && fCheck && !(pGPD->dwMinAttributes & FILE_ATTRIBUTE_READONLY))
            {
               fCheck = 2;
            }
            else
            {
               PostMessage(hwndCtrl, BM_SETSTYLE, BS_AUTOCHECKBOX, 0);
            }

            PostMessage(hwndCtrl, BM_SETCHECK, fCheck, 0);

            fCheck = (pGPD->dwMaxAttributes & FILE_ATTRIBUTE_HIDDEN) ? 1 : 0;
            hwndCtrl = GetDlgItem(hDlg, IDC_HIDDEN);
            if(pGPD->bMultiSelect && fCheck && !(pGPD->dwMinAttributes & FILE_ATTRIBUTE_HIDDEN))
            {
               fCheck = 2;
            }
            else
            {
               PostMessage(hwndCtrl, BM_SETSTYLE, BS_AUTOCHECKBOX, 0);
            }

            PostMessage(hwndCtrl, BM_SETCHECK, fCheck, 0);

            fCheck = (pGPD->dwMaxAttributes & FILE_ATTRIBUTE_ARCHIVE) ? 1 : 0;
            hwndCtrl = GetDlgItem(hDlg, IDC_ARCHIVE);
            if(pGPD->bMultiSelect && fCheck && !(pGPD->dwMinAttributes & FILE_ATTRIBUTE_ARCHIVE))
            {
               fCheck = 2;
            }
            else
            {
               PostMessage(hwndCtrl, BM_SETSTYLE, BS_AUTOCHECKBOX, 0);
            }

            PostMessage(hwndCtrl, BM_SETCHECK, fCheck, 0);

            fCheck = (pGPD->dwMaxAttributes & FILE_ATTRIBUTE_SYSTEM) ? 1 : 0;
            hwndCtrl = GetDlgItem(hDlg, IDC_SYSTEM);
            if(pGPD->bMultiSelect && fCheck && !(pGPD->dwMinAttributes & FILE_ATTRIBUTE_SYSTEM))
            {
               fCheck = 2;
            }
            else
            {
               PostMessage(hwndCtrl, BM_SETSTYLE, BS_AUTOCHECKBOX, 0);
            }

            PostMessage(hwndCtrl, BM_SETCHECK, fCheck, 0);            
         }
         return -1;
      }

      case WM_DESTROY:
      {
         DWORD dwStartMaxAttributes = pGPD->dwMaxAttributes, 
              dwStartMinAttributes = pGPD->dwMinAttributes;

         pGPD->dwMaxAttributes = 0;  //On the outbound, max is used for a mask
         pGPD->dwMinAttributes = 0;  //and min is used for the values.

         if (!pGPD->bHideAttributes)
         {
            UINT fCheck, fNewCheck;
            HWND hwndCtrl;

            fCheck = (dwStartMaxAttributes & FILE_ATTRIBUTE_READONLY) ? 1 : 0;
            if (pGPD->bMultiSelect && fCheck && !(dwStartMinAttributes & FILE_ATTRIBUTE_READONLY)) 
            {
               fCheck = 2;
            }

            hwndCtrl = GetDlgItem(hDlg, IDC_READONLY);
            fNewCheck = SendMessage(hwndCtrl, BM_GETCHECK, 0, 0);
            if (fNewCheck != fCheck)
            {
               pGPD->dwMaxAttributes |= FILE_ATTRIBUTE_READONLY;
               if (fNewCheck)
               {
                  pGPD->dwMinAttributes |= FILE_ATTRIBUTE_READONLY;
               }
            }

            fCheck = (dwStartMaxAttributes & FILE_ATTRIBUTE_HIDDEN) ? 1 : 0;
            if (pGPD->bMultiSelect && fCheck && !(dwStartMinAttributes & FILE_ATTRIBUTE_HIDDEN)) 
            {
               fCheck = 2;
            }

            hwndCtrl = GetDlgItem(hDlg, IDC_HIDDEN);
            fNewCheck = SendMessage(hwndCtrl, BM_GETCHECK, 0, 0);
            if (fNewCheck != fCheck)
            {
               pGPD->dwMaxAttributes |= FILE_ATTRIBUTE_HIDDEN;
               if (fNewCheck)
               {
                  pGPD->dwMinAttributes |= FILE_ATTRIBUTE_HIDDEN;
               }
            }

            fCheck = (dwStartMaxAttributes & FILE_ATTRIBUTE_ARCHIVE) ? 1 : 0;
            if (pGPD->bMultiSelect && fCheck && !(dwStartMinAttributes & FILE_ATTRIBUTE_ARCHIVE)) 
            {
               fCheck = 2;
            }

            hwndCtrl = GetDlgItem(hDlg, IDC_ARCHIVE);
            fNewCheck = SendMessage(hwndCtrl, BM_GETCHECK, 0, 0);
            if (fNewCheck != fCheck)
            {
               pGPD->dwMaxAttributes |= FILE_ATTRIBUTE_ARCHIVE;
               if (fNewCheck)
               {
                  pGPD->dwMinAttributes |= FILE_ATTRIBUTE_ARCHIVE;
               }
            }

            fCheck = (dwStartMaxAttributes & FILE_ATTRIBUTE_SYSTEM) ? 1 : 0;
            if (pGPD->bMultiSelect && fCheck && !(dwStartMinAttributes & FILE_ATTRIBUTE_SYSTEM)) 
            {
               fCheck = 2;
            }

            hwndCtrl = GetDlgItem(hDlg, IDC_SYSTEM);
            fNewCheck = SendMessage(hwndCtrl, BM_GETCHECK, 0, 0);
            if (fNewCheck != fCheck)
            {
               pGPD->dwMaxAttributes |= FILE_ATTRIBUTE_SYSTEM;
               if (fNewCheck)
               {
                  pGPD->dwMinAttributes |= FILE_ATTRIBUTE_SYSTEM;
               }
            }
         }
         return 0;
      }
      default:
         return 0;
   }
}

int CALLBACK ShellDialogs::Properties::ShortcutPropDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{   
   ShortcutPropData * pSPD = (ShortcutPropData *)GetWindowLong(hDlg, DWL_USER);

   switch(uMsg)
   {
      case WM_INITDIALOG:
      {
         // Initalize the SipPref control if necessary
         LoadAygshellLibrary();
         AygAddSipprefControl( hDlg );
         
         LPPROPSHEETPAGE ppsh = (LPPROPSHEETPAGE)lParam;         
         pSPD = (ShortcutPropData *)ppsh->lParam;      
         SetWindowLong(hDlg, DWL_USER, (LONG)pSPD);

         SendDlgItemMessage(hDlg, IDC_PICON, STM_SETIMAGE, 
                        (WPARAM) IMAGE_ICON, (LPARAM) pSPD->pGPD->hicon);
         SetDlgItemTextEllipses(hDlg, IDC_FILENAME, pSPD->pGPD->szFilename);

         SetDlgItemTextEllipses(hDlg, IDC_TARGETTYPE, pSPD->szTargetType);
         SetDlgItemText(hDlg, IDC_TARGET, pSPD->szTarget);
         SendMessage(GetDlgItem(hDlg, IDC_TARGET), EM_LIMITTEXT, MAX_PATH-4, 0);
         if (!pSPD->bModifyTarget)
         {
            HWND hwndCtrl = GetDlgItem(hDlg, IDC_TARGET);
            EnableWindow(hwndCtrl, FALSE);
         }

         if(::wcslen(pSPD->szTargetModified))
         {
            SetDlgItemTextEllipses(hDlg, IDC_MODIFIED, pSPD->szTargetModified);
         }
         else
         {
            HideDlgItem(hDlg, IDC_MODIFIEDLABEL);
            HideDlgItem(hDlg, IDC_MODIFIED);
         }

         if(::wcslen(pSPD->szTargetSize))
         {
            SetDlgItemTextEllipses(hDlg, IDC_PSIZE, pSPD->szTargetSize);
         }
         else
         {
            HideDlgItem(hDlg, IDC_SIZELABEL);
            HideDlgItem(hDlg, IDC_PSIZE);
         }

         AygInitDialog( hDlg, SHIDIF_SIZEDLG );

         return -1;
      }

      case WM_DESTROY:
      {
         WCHAR szPath[MAX_PATH];
         pSPD->bNeedToSetTheTarget = FALSE;
          GetDlgItemText(hDlg, IDC_TARGET, szPath, MAX_PATH);

         if (CSTR_EQUAL != CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                         szPath, -1, pSPD->szTarget, -1))
         {
            ::wcscpy(pSPD->szTarget, szPath);
            pSPD->bNeedToSetTheTarget = TRUE;
         }
         FreeAygshellLibrary();
         return 0;
      }

      default:
        return 0;
   }
}

HRESULT ShellDialogs::Properties::UpdateAttributes(IShellFolder * pFolder, LPCITEMIDLIST pidlItem, 
                                                   DWORD dwMinAttrs, DWORD dwMaxAttrs)
{   
   WCHAR szPath[MAX_PATH];
   STRRET str = {0};
   
   if (ILIsFileSystemPidl(pidlItem))
   {
      //Not a virtual object.
      //Get the name which is relative to the desktop       
      if (SUCCEEDED(pFolder->GetDisplayNameOf(pidlItem, SHGDN_NORMAL | SHGDN_FORPARSING, &str)) &&      
         SUCCEEDED(StrRetToBuf(&str, pidlItem, szPath, lengthof(szPath))))
      {                                              
         DWORD dwAttrib = GetFileAttributes(szPath);
         if (-1 != dwAttrib)
         {                
            dwAttrib &= ~dwMaxAttrs;
            dwAttrib |= dwMinAttrs;
         }
               
         if (!SetFileAttributes(szPath, dwAttrib))
         {
            if (!PathIsDirectory(szPath))
            {                              
               ShellDialogs::ShowFileError(NULL,
                                           PathFindFileName(szPath),
                                           MAKEINTRESOURCE(IDS_PROPERTIES_ERR_ACCESS),
                                           szPath,
                                           MB_ICONERROR | MB_OK);
               return E_FAIL;
            }
         }
      }
   }

   return NOERROR;
}

void ShellDialogs::Properties::HideDlgItem(HWND hDlg, int nDlgItem)
{
   HWND hwndCtrl = GetDlgItem(hDlg, nDlgItem);
   ShowWindow(hwndCtrl, SW_HIDE);
}

BOOL ShellDialogs::Properties::SetDlgItemTextEllipses(HWND hDlg, int nDlgItem, LPWSTR pszText)
{   
   HDC hdc;
   SIZE size;
   BOOL fNeedEllipses;
   RECT rc = {0};
   int cchText;
   WCHAR szTemp[MAX_PATH];
   const WCHAR cszEllipses[] = TEXT("...");
   HWND hwndCtrl = GetDlgItem(hDlg, nDlgItem);
   
   ASSERT(pszText);
   if (!pszText)
   {
      return FALSE;
   }

   hdc = GetDC(hwndCtrl);

   if(!hwndCtrl || !hdc)
   {
      return FALSE;
   }

   GetClientRect(hwndCtrl, &rc);

   _tcsncpy(szTemp, pszText, MAX_PATH - 1);
   szTemp[MAX_PATH - 1] = 0;

   // 3d param is the number of characters in the string. 
   GetTextExtentPoint(hdc, cszEllipses, lengthof(cszEllipses) - 1, &size);
   //NOTE: Since we are putting this text into a Static control,
   //      we need to take into account the WinCE implementation
   //      that will shrink the text rect by 1 on each side so
   //      the text isn't right next to the border.
   size.cx += 2;
   
   fNeedEllipses = NeedsEllipses(hdc, pszText, &rc, &cchText, size.cx);
   ReleaseDC(hwndCtrl, hdc);
   
   if (fNeedEllipses)
   {
      ::wcscpy(szTemp + cchText, cszEllipses);
   }
   
   SetWindowText(hwndCtrl, szTemp);

   return TRUE;
}

BOOL ShellDialogs::Properties::NeedsEllipses(HDC hdc, LPCWSTR pszText, LPRECT prc, int * pcchDraw, int cxEllipses)
{
   int cchText;
   int cxRect;
   int ichMin, ichMax, ichMid;
   SIZE size;
   
   cxRect = prc->right - prc->left;   
   cchText = ::wcslen(pszText);

   if (!cchText)
   {
      *pcchDraw = cchText;
      return FALSE;
   }
   
   GetTextExtentPoint(hdc, pszText, cchText, &size);   
   if (size.cx <= cxRect)
   {
      *pcchDraw = cchText;
      return FALSE;
   }

   cxRect -= cxEllipses;   
   ichMax = 1;
   if (0 < cxRect)
   {   
      ichMin = 0;
      ichMax = cchText;
      
      while (ichMin < ichMax)
      {
         ichMid = (ichMin + ichMax + 1) / 2;
         
         GetTextExtentPoint(hdc, &pszText[ichMin], ichMid - ichMin, &size);
         
         if (size.cx < cxRect)
         {
            ichMin = ichMid;
            cxRect -= size.cx;            
         }
         else if (size.cx > cxRect)
         {
            ichMax = ichMid - 1;            
         }
         else
         {
            ichMax = ichMid;
            break;
         }
      }
      
      if (ichMax < 1)
      {
         ichMax = 1;
      }
   }
   
   *pcchDraw = ichMax;
   return TRUE;
}

void ShellDialogs::Properties::GeneralPropData::GetDiskSpace()
{
   ULARGE_INTEGER uliFreeToCaller, uliTotal, uliFree;

   //Get the free space information.
   if (GetDiskFreeSpaceEx(szPath, &uliFreeToCaller, &uliTotal, &uliFree))
   {      
      WCHAR szFormat[32], szBytes[64], szTemp[64];
      ULARGE_INTEGER uli;
      
      LoadString(HINST_CESHELL, IDS_ORDER_BYTES, szFormat, 32);
      uli.QuadPart = uliTotal.QuadPart - uliFree.QuadPart;
      szBytes[0] = 0;
      if (ShellFormat::ULargeToBuffer(uli, szTemp, lengthof(szTemp)))
      {
         // Set to an empty string on failure
         StringCchPrintfEx(szBytes,
                       lengthof(szBytes),
                       NULL,
                       NULL,
                       STRSAFE_NULL_ON_FAILURE,
                       szFormat,
                       szTemp);        
      }
                
      ShellFormat::FileSizeShort(uli, szTemp, lengthof(szTemp));

      if (*szBytes)
      {
         StringCchPrintf(szSize, lengthof(szSize), L"%s (%s)", szTemp, szBytes);
      }
      else
      {
         StringCchCopy(szSize, lengthof(szSize), szTemp);
      }
      
      // Display the number of bytes if it isn't too big
      szBytes[0] = 0;
      if (ShellFormat::ULargeToBuffer(uliFree, szTemp, lengthof(szTemp)))
      {
         StringCchPrintfEx(szBytes,
                       lengthof(szBytes),
                       NULL,
                       NULL,
                       STRSAFE_NULL_ON_FAILURE,
                       szFormat,
                       szTemp);
      }
      
      ShellFormat::FileSizeShort(uliFree, szTemp, lengthof(szTemp));

      if (*szBytes)
      {
         StringCchPrintf(szModified, lengthof(szModified), L"%s (%s)", szTemp, szBytes);
      }
      else
      {
         StringCchCopy(szSize, lengthof(szSize), szTemp);
      }
   }

   return;
}

void ShellDialogs::Properties::GetLocation(LPCITEMIDLIST pidl, LPWSTR pszLocation)
{
   //Get IShellFolder pointer on the parent object to get the name of pidl object.
   IShellFolder * pParentFolder = NULL;
   LPCITEMIDLIST pidlLast = NULL;
   
   ASSERT(pidl);
   ASSERT(pszLocation);
   if (!pidl || !pszLocation)
   {
      return;
   }
     
   if (SUCCEEDED(SHBindToParent(pidl, IID_IShellFolder, (VOID **) &pParentFolder, &pidlLast)))   
   {
      STRRET str = {0};
      WCHAR szTmp[MAX_PATH];      
        
      if (SUCCEEDED(pParentFolder->GetDisplayNameOf(pidlLast, SHGDN_NORMAL | SHGDN_FOREDITING, &str)))
      {
         if (SUCCEEDED(StrRetToBuf(&str, pidlLast, szTmp, lengthof(szTmp))))
         {
            WCHAR szDesktop[MAX_PATH];
            BOOL fRet = SHGetSpecialFolderPath(NULL, szDesktop, CSIDL_DESKTOPDIRECTORY, TRUE);

            if (fRet && (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT,
                                                     NORM_IGNORECASE,
                                                     szDesktop, -1, szTmp, -1)))
            {
               if (SUCCEEDED(pParentFolder->GetDisplayNameOf(pidlLast, SHGDN_INFOLDER | SHGDN_FOREDITING, &str)))
                  StrRetToBuf(&str, pidlLast, szTmp, lengthof(szTmp));
            }     
             
            StringCchCopyEx(pszLocation, MAX_PATH, szTmp, NULL, NULL, STRSAFE_NULL_ON_FAILURE);
         }
      }

      pParentFolder->Release();
      ILFree(pidlLast);
   }

   return;
}

void ShellDialogs::Properties::RecycleBinProperties()
{
   PROPSHEETPAGE psp[1];
   PROPSHEETHEADER psh;
   RecyclePropData pd;

   pd.fUseRecycleBin = UserSettings::GetUseRecycleBin();

   g_pRecBin->BeginRecycle();
   pd.uPercentage = g_pRecBin->GetPercentage();

   //Get the size of the Object Store or the Storage for the Recycle Bin   
   pd.ullOSSize = g_pRecBin->StoreSize();
   g_pRecBin->EndRecycle();

   psp[0].dwSize = sizeof(PROPSHEETPAGE);
   psp[0].dwFlags = PSP_USETITLE;
   psp[0].hInstance = HINST_CESHELL;
   psp[0].pszTemplate = MAKEINTRESOURCE(IDD_RECYCLEBIN);
   psp[0].pszIcon = NULL;
   psp[0].pfnDlgProc = RecBinPropDlgProc;
   psp[0].pszTitle = MAKEINTRESOURCE(IDS_TITLE_INTERNAL);
   psp[0].lParam = (LPARAM) &pd;
   psp[0].pfnCallback = NULL;

   psh.dwSize = sizeof(PROPSHEETHEADER);
   psh.dwFlags = PSH_PROPSHEETPAGE | PSH_USECALLBACK;
   psh.hwndParent = NULL;
   psh.hInstance = HINST_CESHELL;
   psh.pszIcon = NULL;
   psh.pszCaption = MAKEINTRESOURCE(IDS_TITLE_RECYCLEBIN);
   psh.nStartPage = 0;
   psh.ppsp = (LPCPROPSHEETPAGE) psp;
   psh.pfnCallback = RecBinSheetCallback;
   psh.nPages = 1;

   if (!hRecBinPropDlg)
   {
      if(-1 != PropertySheet(&psh))
      {         
         if (pd.fUseRecycleBin != UserSettings::GetUseRecycleBin())
         {
            UserSettings::SetUseRecycleBin(pd.fUseRecycleBin);
         }

         g_pRecBin->BeginRecycle();
         if (pd.uPercentage != g_pRecBin->GetPercentage())
         {
            g_pRecBin->SetPercentage(pd.uPercentage);
            if (pd.fUseRecycleBin && !g_pRecBin->IsEmpty() && g_pRecBin->IsFull())
            {                         
               //Warn about the storage:
               //Pop up a dialog to allow the user to empty the RecycleBin            
               HWND hWndWarning = FindWindow(TEXT("Dialog"), LOAD_STRING(IDS_PROPERTIES_RECBINMAX));
               if (hWndWarning)
               {
                  SetForegroundWindow(hWndWarning);
               }
               else
               {
                  LPWSTR pszStr = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR));
                  if (pszStr)
                  {
                     *pszStr = TEXT('\0');

                     HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_ARROW));
                     DialogBoxParam(HINST_CESHELL, MAKEINTRESOURCE(IDD_RECYCLEBINFULL), NULL,
                        ShellDialogs::RecycleBinWarnDlgProc, (LPARAM) pszStr);

                     SetCursor(hCursor);
                     LocalFree((HLOCAL)pszStr);
                  }
               }            
            }   
         }

         g_pRecBin->EndRecycle();
         hRecBinPropDlg = NULL;
      }
   }
   else
   {            
      SetForegroundWindow(hRecBinPropDlg);
   }

   return;
}

int CALLBACK ShellDialogs::Properties::RecBinPropDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   RecyclePropData *pPD = (RecyclePropData *)GetWindowLong(hDlg, DWL_USER);
   WCHAR szBuf[64];
   HWND hwndCtrl;

   switch(uMsg)
   {
      case WM_INITDIALOG:
      {
         LPPROPSHEETPAGE ppsh = (LPPROPSHEETPAGE)lParam;
         pPD = (RecyclePropData *)ppsh->lParam;
         SetWindowLong(hDlg, DWL_USER, (LONG)pPD);

         AygInitDialog( hDlg, SHIDIF_SIPDOWN );

         hwndCtrl = GetDlgItem(hDlg, IDC_PERCENTAGESLIDER);
         SendMessage(hwndCtrl, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 100));  
         SendMessage(hwndCtrl, TBM_SETTICFREQ, 10, 10);  
         SendMessage(hwndCtrl, TBM_SETPAGESIZE, 0, 10);  
         SendMessage(hwndCtrl, TBM_SETPOS, TRUE, pPD->uPercentage);
         
         hwndCtrl = GetDlgItem(hDlg, IDC_USERECYCLEBIN);
         SendMessage(hwndCtrl, BM_SETCHECK, 
                  pPD->fUseRecycleBin ? BST_UNCHECKED : BST_CHECKED, 0);

         hwndCtrl = GetDlgItem(hDlg, IDC_PERCENTAGE);
         ::StringCbPrintfW(szBuf, sizeof(szBuf), L"%d%%", pPD->uPercentage);
         SetWindowText(hwndCtrl, szBuf);
         
         hwndCtrl = GetDlgItem(hDlg, IDC_TOTALSTORAGE);

         ULARGE_INTEGER uliTmp;
         uliTmp.QuadPart = pPD->ullOSSize;
         ShellFormat::FileSizeShort(uliTmp, szBuf, lengthof(szBuf));        
         SetWindowText(hwndCtrl, szBuf);
   
         hwndCtrl = GetDlgItem(hDlg, IDC_MAXRECYCLE);
         
         uliTmp.QuadPart = (pPD->ullOSSize * (ULONGLONG)pPD->uPercentage)/(ULONGLONG)100;
         ShellFormat::FileSizeShort(uliTmp, szBuf, lengthof(szBuf));
         SetWindowText(hwndCtrl, szBuf);
         
         if (!pPD->fUseRecycleBin)
         {
            hwndCtrl = GetDlgItem(hDlg, IDC_PERCENTAGESLIDER);
            EnableWindow(hwndCtrl, FALSE);
            hwndCtrl = GetDlgItem(hDlg, IDC_PERCENTAGE);
            EnableWindow(hwndCtrl, FALSE);
            hwndCtrl = GetDlgItem(hDlg, IDC_MAXLABEL);
            EnableWindow(hwndCtrl, FALSE);
            hwndCtrl = GetDlgItem(hDlg, IDC_GROUPBOX);
            EnableWindow(hwndCtrl, FALSE);
            hwndCtrl = GetDlgItem(hDlg, IDC_USERECYCLEBIN);
            SetFocus(hwndCtrl);
         }
         else
         {
            hwndCtrl = GetDlgItem(hDlg, IDC_PERCENTAGESLIDER);
            SetFocus(hwndCtrl);
         }         
      }
      break;

      case WM_HSCROLL:
      {
         DWORD dwPos;
         hwndCtrl = GetDlgItem(hDlg, IDC_PERCENTAGESLIDER);
         dwPos = SendMessage(hwndCtrl, TBM_GETPOS, 0, 0);
         pPD->uPercentage = dwPos;

         hwndCtrl = GetDlgItem(hDlg, IDC_PERCENTAGE);
         ::StringCbPrintfW(szBuf, sizeof(szBuf), L"%d%%", pPD->uPercentage);
         SetWindowText(hwndCtrl, szBuf);

         hwndCtrl = GetDlgItem(hDlg, IDC_TOTALSTORAGE);
                  
         ULARGE_INTEGER uliTmp;
         uliTmp.QuadPart = pPD->ullOSSize;
         ShellFormat::FileSizeShort(uliTmp, szBuf, lengthof(szBuf));

         SetWindowText(hwndCtrl, szBuf);
   
         hwndCtrl = GetDlgItem(hDlg, IDC_MAXRECYCLE);
                  
         uliTmp.QuadPart = (pPD->ullOSSize * (ULONGLONG)pPD->uPercentage)/(ULONGLONG)100;
         ShellFormat::FileSizeShort(uliTmp, szBuf, lengthof(szBuf));

         SetWindowText(hwndCtrl, szBuf);         
      }
      break;

      case WM_COMMAND:
         switch (GET_WM_COMMAND_ID(wParam,lParam))
         {            
            case IDC_USERECYCLEBIN:
            {
               if (BN_CLICKED == HIWORD(wParam))
               { 
                  pPD->fUseRecycleBin = !pPD->fUseRecycleBin;
                  
                  if (!pPD->fUseRecycleBin)
                  {
                     hwndCtrl = GetDlgItem(hDlg, IDC_PERCENTAGESLIDER);
                     EnableWindow(hwndCtrl, FALSE);
                     hwndCtrl = GetDlgItem(hDlg, IDC_PERCENTAGE);
                     EnableWindow(hwndCtrl, FALSE);
                     hwndCtrl = GetDlgItem(hDlg, IDC_MAXLABEL);
                     EnableWindow(hwndCtrl, FALSE);
                     hwndCtrl = GetDlgItem(hDlg, IDC_GROUPBOX);
                     EnableWindow(hwndCtrl, FALSE);
                  }
                  else
                  {
                     hwndCtrl = GetDlgItem(hDlg, IDC_PERCENTAGESLIDER);
                     EnableWindow(hwndCtrl, TRUE);
                     hwndCtrl = GetDlgItem(hDlg, IDC_PERCENTAGE);
                     EnableWindow(hwndCtrl, TRUE);
                     hwndCtrl = GetDlgItem(hDlg, IDC_MAXLABEL);
                     EnableWindow(hwndCtrl, TRUE);
                     hwndCtrl = GetDlgItem(hDlg, IDC_GROUPBOX);
                     EnableWindow(hwndCtrl, TRUE);
                  }
               }               
            }           
         }
         break;

      default:
         return FALSE;
   }   
   return TRUE;
}

int CALLBACK ShellDialogs::Properties::RecBinSheetCallback(HWND hDlg, UINT uMsg, LPARAM lParam)
{
   if (PSCB_INITIALIZED == uMsg)
   {
      hRecBinPropDlg = hDlg;
   }

   return 0;
}

int CALLBACK ShellDialogs::RecycleBinWarnDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   LPWSTR pszData = NULL;
   switch (uMsg)
   {
      case WM_INITDIALOG:                  
         pszData = (LPWSTR)lParam;

         AygInitDialog( hDlg, SHIDIF_SIPDOWN );

         // Set the message text
         ::SetDlgItemText(hDlg, IDC_CONFIRM_MESSAGE, pszData);

         MessageBeep((UINT)-1);
         break;
                  
      case WM_COMMAND:
         switch (GET_WM_COMMAND_ID(wParam,lParam))
         {
            case IDC_EMPTY:
               {
                  BOOL fConfirmDel = FALSE;

                  g_pRecBin->BeginRecycle();                  
                  g_pRecBin->DeleteAll(fConfirmDel);
                  g_pRecBin->EndRecycle();

                  ShowWindow(hDlg, SW_HIDE);
                  ::EndDialog(hDlg, GET_WM_COMMAND_ID(wParam,lParam));                                 
               }
               break;

            case IDCANCEL:
               {                    
                  ::EndDialog(hDlg, GET_WM_COMMAND_ID(wParam,lParam));
               }
               break;                           
         }
         break;

      default:
         return FALSE;
   }
   
   return TRUE;
}

