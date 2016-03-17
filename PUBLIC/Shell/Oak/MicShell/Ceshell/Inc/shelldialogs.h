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

#include "ceshhpc.h"

#pragma once

class ShellDialogs
{
public:

   class Confirm
   {
   protected:
      UINT m_uTitle;
      HICON m_hicon;
      TCHAR m_szMessage[256+MAX_PATH];
      BOOL m_fHideYesToAllAndCancel;
      BOOL m_fExtended;
      // Only fill these in if m_fExtented is TRUE
      HICON m_hiconOld;
      HICON m_hiconNew;
      TCHAR m_szFileInfoOld[256];
      TCHAR m_szFileInfoNew[256];

      static int CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

      void InitDialog(HWND hDlg);

   public:
      static HRESULT Move(LPCWSTR pwszFile, HWND hwndOwner, BOOL fHideYesToAllAndCancel, int * pResult);
      static HRESULT Rename(LPCWSTR pszSrc, LPCWSTR pszDst, HWND hwndOwner, int * pResult);
      static HRESULT Delete(LPCWSTR pszPath, HWND hwndOwner, BOOL fAlwaysConfirm, BOOL fForcePermanent, BOOL fHideYesToAllAndCancel, int * pResult);
      static HRESULT DeleteMultiple(DWORD dwCount, HWND hwndOwner, BOOL fForcePermanent, int * pResult);
      static HRESULT DeleteForRecycleBin(LPCWSTR pszPath, HWND hwndOwner, BOOL fFull, int * pResult);
      static HRESULT Merge(LPCWSTR pszPath, HWND hwndOwner, BOOL fHideYesToAllAndCancel, int * pResult);
      static HRESULT Replace(LPCWSTR pszSrc, LPCWSTR pszDst, HWND hwndOwner, int * pResult);
   };

   static HRESULT ShowFileError(HWND hwndOwner, LPCWSTR pszTitle, LPCWSTR pszMsg,
                                LPCWSTR pszFile, UINT uFlags);
   static HRESULT ShowPathError(HWND hwndOwner, LPCWSTR pszTitle, LPCWSTR pszMsg,
                                LPCWSTR pszPath, UINT uFlags);
   static int CALLBACK RecycleBinWarnDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

   class Properties
   {
      protected:
         struct ObjectPropertiesData
         {
            LPCITEMIDLIST pidl;
            IShellFolder * pFolder;
            IShellView * pView;
            LPCITEMIDLIST * aPidls;
            UINT uItemCount;
         };

         struct FolderObjectPropertiesData
         {
            LPCITEMIDLIST pidl;
            IShellFolder * pFolder;
         };

         struct GeneralPropData
         {
            TCHAR szFilename[MAX_PATH];
            TCHAR szType[80];
            TCHAR szLocation[MAX_PATH];
            TCHAR szSize[MAX_PATH];
            TCHAR szModified[MAX_PATH];
            TCHAR szPath[MAX_PATH];
            HICON hicon;
            DWORD dwMinAttributes;
            DWORD dwMaxAttributes;
            BOOL bMultiSelect;
            BOOL bHideAttributes;
            BOOL bRemovableStorage;
            void GetDiskSpace();
         };                                

         struct ShortcutPropData
         {       
            GeneralPropData * pGPD;
            TCHAR szTargetType[128];
            TCHAR szTarget[MAX_PATH];
            TCHAR szTargetSize[32];
            TCHAR szTargetModified[MAX_PATH];
            BOOL bModifyTarget;
            BOOL bNeedToSetTheTarget;
         };

         struct RecyclePropData
         {
            BOOL fUseRecycleBin;
            UINT uPercentage;            
            ULONGLONG ullOSSize;
         };
           
         static HWND hRecBinPropDlg;
         static void GetLocation (LPCITEMIDLIST pidl, LPWSTR pszLocation);                  
         static BOOL SetDlgItemTextEllipses(HWND hDlg, int nDlgItem, LPWSTR pszText);
         static BOOL NeedsEllipses(HDC hdc, LPCWSTR pszText, LPRECT prc, int * pcchDraw, int cxEllipses);       
         static HRESULT UpdateAttributes(IShellFolder * pFolder, LPCITEMIDLIST pidlItem, DWORD dwMinAttrs, DWORD dwMaxAttrs);         
         static void HideDlgItem(HWND hDlg, int nDlgItem);
         
         static int CALLBACK GeneralPropDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);         
         static int CALLBACK RecBinPropDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
         static int CALLBACK RecBinSheetCallback(HWND hDlg, UINT uMsg, LPARAM lParam);
         static int CALLBACK ShortcutPropDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

         static DWORD CALLBACK ObjectPropertiesThread(LPVOID lpParameter);         
         static DWORD CALLBACK FolderObjectPropertiesThread(LPVOID lpParameter);

      public:
         static void ObjectProperties(LPCITEMIDLIST pidl, IShellFolder * pFolder, IShellView * pView, LPCITEMIDLIST * aPidls, UINT uItemCount);         
         static void FolderObjectProperties(LPCITEMIDLIST pidl, IShellFolder * pFolder);
         static void RecycleBinProperties();
   };
};

// From SHFormatDateTime flags
// (FDTF_SHORTDATE and FDTF_LONGDATE are mutually exclusive, as is
// FDTF_SHORTIME and FDTF_LONGTIME.)
#define FDTF_SHORTTIME      0x00000001      // eg, "7:48 PM"
#define FDTF_SHORTDATE      0x00000002      // eg, "3/29/98"
#define FDTF_DEFAULT        (FDTF_SHORTDATE | FDTF_SHORTTIME) // eg, "3/29/98 7:48 PM"
#define FDTF_LONGDATE       0x00000004      // eg, "Monday, March 29, 1998"
#define FDTF_LONGTIME       0x00000008      // eg. "7:48:33 PM"
#define FDTF_LTRDATE        0x00000100      // Left To Right reading order
#define FDTF_RTLDATE        0x00000200      // Right To Left reading order

class ShellFormat
{
public:
   static BOOL ULargeToBuffer(ULARGE_INTEGER uilSize, LPWSTR pszBuffer, size_t cchBuffer);
   static BOOL FileInfo(LPCWSTR pszFile, LPWSTR pszFileInfo, size_t cchFileInfo);
   static BOOL FileTimeInfo(LPCWSTR pszFile, LPWSTR pszFileTimeInfo, size_t cchFileTimeInfo);
   static BOOL FileSizeShort(ULARGE_INTEGER uilSize, LPWSTR pszSize, size_t cchSize);
   static BOOL FolderStatus(UINT uCount, UINT uMsg, LPWSTR pszStatusText, size_t cchStatusText);
   static BOOL FormatDateTime(const FILETIME* pft, DWORD dwFlags, LPWSTR pwszBuffer, size_t cchBuffer);
};

