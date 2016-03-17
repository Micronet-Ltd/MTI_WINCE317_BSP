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

#include "desktopview.h"
#include "idlist.h"
#include "resource.h"
#include "recbin.h"
#include "usersettings.h"
#include "shdisp.h"
#include "imgguids.h"

//////////////////////////////////////////////////
// CDesktopView

const TCHAR CDesktopView::WC_DESKTOPVIEW[] = TEXT("DesktopExplorerWindow");

CDesktopView::CDesktopView(IShellFolder * pFolder, LPCITEMIDLIST pidlFQ) :
   CDefShellView(pFolder, pidlFQ),
   m_wndprocListView(NULL),
   m_hbmpDesktop(NULL),
   m_pImagingFactory(NULL),
   m_bIsComInit(FALSE)
{
}

CDesktopView::~CDesktopView()
{
   if (m_hbmpDesktop)
   {
      DeleteObject(m_hbmpDesktop);
   }
   UnInitImaging();
}

LRESULT CALLBACK CDesktopView::SubclassProc(HWND hwnd,
                                            UINT uMsg,
                                            WPARAM wParam,
                                            LPARAM lParam)
{
   HWND hwndParent = ::GetParent(hwnd);
   ASSERT(hwndParent);
   CDesktopView * pThis = (CDesktopView*) ::GetWindowLong(hwndParent, GWL_USERDATA);
   LRESULT result = 0;

   switch (uMsg)
   {
      case WM_ERASEBKGND:
         pThis->PaintBackground((HDC) wParam);
      break;

      case WM_KEYDOWN:
         switch (wParam)
         {
            case VK_TAB:
               if ((FWF_DESKTOP & pThis->m_fFlags) && !(IN_LABEL_EDIT & pThis->m_uState))
               {
                  SetForegroundWindow(FindWindow(L"HHTaskBar", NULL));
                  goto leave;
               }
            break;

            case VK_F1:
            {
               // forward F1 to the taskbar to launch help
               HWND hwndTaskbar = ::FindWindow(L"HHTaskBar", NULL);
               if (hwndTaskbar)
               {
                  ::PostMessage(hwndTaskbar, uMsg, wParam, lParam);
                  goto leave;
               }
            }
            break;
         }
         // fall through...

      default:
         result = ::CallWindowProc(pThis->m_wndprocListView,
                                   hwnd, uMsg, wParam, lParam);
   }

leave:
   return result;
}

HBITMAP CDesktopView::ConvertImageToScreenRes(HBITMAP hbmSrc)
{
   if (NULL == hbmSrc)
   {
      return NULL;
   }

   HBITMAP hbmDst = NULL;
   BITMAP bm = {0};

   // Get a HDC to the screen.
   HDC hdcScreen = CreateDC(L"DISPLAY", NULL, NULL, NULL);

   GetObject(hbmSrc, sizeof(BITMAP), &bm);

   HDC hdcSrc = CreateCompatibleDC(hdcScreen);

   SelectObject(hdcSrc, hbmSrc);
   HDC hdcDst =  CreateCompatibleDC(hdcScreen);

   if (UserSettings::GetTileBackgroundImage())
   {
      int iHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
      int iWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);

      hbmDst = CreateCompatibleBitmap(hdcScreen, iWidth, iHeight);

      SelectObject(hdcDst, hbmDst);
      POINT pt = {0};

      if (m_pShellListView && ListView_GetOrigin(*m_pShellListView, &pt))
      {
         SetBrushOrgEx(hdcSrc, pt.x, pt.y, NULL);
      }

      HBRUSH hBrush = CreatePatternBrush(hbmSrc);
      HBRUSH hBrushOld = (HBRUSH) SelectObject(hdcDst, hBrush);
      PatBlt(hdcDst, 0, 0, iWidth, iHeight,  PATCOPY);
      SelectObject(hdcDst, hBrushOld);
      DeleteObject(hBrush);
   }
   else
   {
      hbmDst = CreateCompatibleBitmap(hdcScreen, bm.bmWidth, bm.bmHeight);
      SelectObject(hdcDst, hbmDst);
      BitBlt(hdcDst, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);
   }

   DeleteDC(hdcSrc);
   DeleteDC(hdcDst);
   DeleteDC(hdcScreen);

   return hbmDst;
}

void CDesktopView::InitImaging()
{
   if (m_bIsComInit)
   {
      return;
   }
   
   // Initialize COM once
   if (SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
   {
      m_bIsComInit = TRUE;
      CoCreateInstance(CLSID_ImagingFactory, NULL, CLSCTX_INPROC_SERVER,
         IID_IImagingFactory, (void**) &m_pImagingFactory);
   }
}

void CDesktopView::UnInitImaging()
{
   if (m_pImagingFactory)
   {
      m_pImagingFactory->Release();
      m_pImagingFactory = NULL;
   }
   
   if (m_bIsComInit)
   {
      CoUninitialize();
      m_bIsComInit = FALSE;
   }
}

BOOL CDesktopView::IsDecoderAvailable(LPCWSTR pwszFileName)
{
   BOOL bRet = FALSE;

   if (!pwszFileName || !pwszFileName[0])
   {
      goto Leave;
   }

   LPWSTR pwszExtension = PathFindExtension(pwszFileName);
   if (!pwszExtension || !(*pwszExtension))
   {
      goto Leave;
   }
   
   // Perform a lowercase comparison of strings.
   if (m_pImagingFactory && (0 != ::_wcsicmp(pwszExtension, L".2bp")))
   {
      // Imaging is availbale & pwszFileName does not have .2bp extension
      ImageCodecInfo * pDecoders = NULL;
      UINT uDecodCount = 0;

      HRESULT hr = m_pImagingFactory->GetInstalledDecoders(&uDecodCount, &pDecoders);
      if (SUCCEEDED(hr) && pDecoders && 0 < uDecodCount)
      {
         // Find the decoder to handle pwszExtension
         for (UINT i = 0; i < uDecodCount; i++)
         {
            if (::wcsstr(CharUpper((LPTSTR)pDecoders[i].FilenameExtension), CharUpper(pwszExtension)))
            {
               bRet = TRUE;
               break;
            }
         }

         CoTaskMemFree(pDecoders);
      }
   }

Leave: ;
   return bRet;
}

HBRUSH CDesktopView::LoadDIBitmapBrush(LPCWSTR wszFileName)
{
   HBRUSH hbrush = NULL;
   HANDLE hFile;

   hFile = ::CreateFile(wszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
   if (INVALID_HANDLE_VALUE != hFile)
   {
      DWORD dwFileSize = GetFileSize(hFile, NULL);
      DWORD dwRead = 0;
      BITMAPFILEHEADER bmfh;

      if (!::ReadFile(hFile, (LPVOID)&bmfh, sizeof(BITMAPFILEHEADER), &dwRead, NULL))
      {
         if (bmfh.bfType == MAKEWORD('B','M'))
         {
            LPBYTE pdib = (LPBYTE)::VirtualAlloc(NULL, (dwFileSize-dwRead),
                                                 MEM_COMMIT|MEM_RESERVE,
                                                 PAGE_READWRITE);
            if (NULL != pdib)
            {
               if (!::ReadFile(hFile, (LPVOID)pdib, (dwFileSize-dwRead), &dwRead, NULL))
               {
                  hbrush = CreateDIBPatternBrushPt(pdib, DIB_RGB_COLORS);
               }

               ::VirtualFree(pdib, 0, MEM_RELEASE);
            }
         }
      }

      ::CloseHandle(hFile);
   }

   return hbrush;
}

void CDesktopView::PaintBackground(HDC hdc) const
{
   ASSERT(m_pShellListView);
   if (!m_pShellListView)
   {
      return;
   }

   BOOL fUseCompatibleBackgroundImage = UserSettings::GetUseCompatibleBackgroundImage();
   BOOL fTileBackgroundImage = UserSettings::GetTileBackgroundImage();
   COLORREF crBk = ::GetSysColor(COLOR_BACKGROUND);
   COLORREF crFg = ::GetSysColor(COLOR_STATIC);
   RECT rc = {0};
   RECT rcBlt = {0};

   ::GetClientRect(*m_pShellListView, &rc);

   // If bitmap is mono, map to system colors.
   ::SetBkColor(hdc, crBk);
   if (crBk == crFg)
   {
     crFg = ::GetSysColor(COLOR_STATICTEXT);
   }
   ::SetTextColor(hdc, crFg);

   if (m_hbmpDesktop)
   {
      if (!fUseCompatibleBackgroundImage && fTileBackgroundImage)
      {
         // Tile the old way
         POINT pt;
         if (ListView_GetOrigin(*m_pShellListView, &pt))
         {
            ::SetBrushOrgEx(hdc, pt.x, pt.y, NULL);
         }

         ::FillRect(hdc, &rc, (HBRUSH) m_hbmpDesktop);
      }
      else // (g_fUseCompatibleBM || !m_fTile)
      {
         BITMAP bm = {0};
         ::GetObject(m_hbmpDesktop, sizeof(BITMAP), &bm);
         HDC hdcMem = CreateCompatibleDC(hdc);
         HGDIOBJ horg = SelectObject(hdcMem, m_hbmpDesktop);

         if (!fTileBackgroundImage)
         {
             int horzOffset = ::GetSystemMetrics(SM_CXSCREEN)/2 - (bm.bmWidth/2);
             int vertOffset = ::GetSystemMetrics(SM_CYSCREEN)/2 - (bm.bmHeight/2);
             ::SetRect(&rcBlt, 0, 0, bm.bmWidth, bm.bmHeight);
             ::OffsetRect(&rcBlt, horzOffset, vertOffset);
             ::BitBlt(hdc, rcBlt.left,  rcBlt.top, bm.bmWidth,
                      bm.bmHeight,  hdcMem, 0, 0, SRCCOPY);
         }
         else
         {
             ::BitBlt(hdc, 0,0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
         }

         ::SelectObject(hdcMem, horg);
         ::DeleteDC(hdcMem);

         //NOTE: Lets clip the area we just were drawing
         HRGN hrgn = ::CreateRectRgnIndirect(&rc);
         ::SelectClipRgn(hdc, hrgn);
         ::DeleteObject(hrgn);

         if (!fTileBackgroundImage)
         {
            ::ExcludeClipRect(hdc, rcBlt.left, rcBlt.top,
                              rcBlt.right, rcBlt.bottom);
            ::FillRect(hdc, &rc, ::GetSysColorBrush(COLOR_BACKGROUND));
            ::SelectClipRgn(hdc, NULL);
         }
      }
   }
   else
   {
      ::FillRect(hdc, &rc, ::GetSysColorBrush(COLOR_BACKGROUND));
   }

   #ifndef SHIP_BUILD
   WCHAR wszVersion[256];
   OSVERSIONINFO VerInfo;
   VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

   ::GetVersionEx(&VerInfo);
   ::wsprintf(wszVersion, L"Windows Embedded CE v%d.%02d (Build %d on %s) ",
              VerInfo.dwMajorVersion, VerInfo.dwMinorVersion, VerInfo.dwBuildNumber, TEXT(__DATE__));
   if (m_hbmpDesktop && !fTileBackgroundImage)
   {
     rc.top = rcBlt.bottom + 5;
   }
   else
   {
     rc.top = rc.top + (rc.bottom - rc.top) / 2 - 8;
   }
   ::DrawText(hdc, wszVersion, -1, &rc, DT_SINGLELINE | DT_CENTER | DT_TOP);
   #endif
}

void CDesktopView::GetDisplayInfo(NMLVDISPINFO * lpdi)
{
   CDefShellView::GetDisplayInfo(lpdi);

   if (FWF_DESKTOP & m_fFlags)
   {
      // Always hide extensions on desktop
      if (UserSettings::GetShowExtensions() &&
        (lpdi && (LVIF_TEXT & lpdi->item.mask)))
      {
         LPITEMIDLIST pidl = (LPITEMIDLIST) lpdi->item.lParam;
         ASSERT(pidl);

         if (pidl && !ILIsFileSystemDirectoryPidl(pidl))
         {
            LPTSTR pszExtension = PathFindExtension(lpdi->item.pszText);
            *pszExtension = TEXT('\0');
         }
      }
   }
}

LRESULT CDesktopView::HandleCommand(DWORD dwCmd)
{
   LRESULT result = 1;

   if (FWF_DESKTOP & m_fFlags)
   {
      // Legacy set of wallpaper and desktop tile
      if ((1001 == dwCmd) || (1105 == dwCmd))
      {
         HandleSettingChange(SPI_SETDESKWALLPAPER, NULL);
         result = 0;
      }
      else if (IDC_GO_FOLDERUP == dwCmd)
      {
         // Do nothing
         result = 0;
      }
      else
      {
         result = CDefShellView::HandleCommand(dwCmd);
      }
   }
   else
   {
      result = CDefShellView::HandleCommand(dwCmd);
   }

   return result;
}

LRESULT CDesktopView::HandleInitMenuPopup(HMENU hmenu, PopupMenuInfo * pPopupMenuInfo)
{
   LRESULT result = CDefShellView::HandleInitMenuPopup(hmenu, pPopupMenuInfo);

   if ((0 == result) && (FWF_DESKTOP & m_fFlags))
   {
      if (pPopupMenuInfo && (FOLDER_MENU_OFFSET == pPopupMenuInfo->offset))
      {
         ::RemoveMenu(hmenu, FOLDER_VIEW_MENU_OFFSET, MF_BYPOSITION); // "View" sub-menu
         ::RemoveMenu(hmenu, FOLDER_VIEW_MENU_OFFSET, MF_BYPOSITION); // "View" separator
      }
   }

   return result;
}

LRESULT CDesktopView::HandleSettingChange(DWORD flag, LPCWSTR pwszSectionName)
{
   LRESULT result = 1;

   if (FWF_DESKTOP & m_fFlags)
   {
      if (0 == flag) // Application sent settings change
      {
         // Ignore the setting change
      }
      else if (SETTINGCHANGE_RESET == flag) // The screen was rotated
      {
         ::MoveWindow(m_hwnd, 0, 0,
                      GetSystemMetrics(SM_CXVIRTUALSCREEN),
                      GetSystemMetrics(SM_CYVIRTUALSCREEN), TRUE);
         HandleSettingChange(SPI_SETWORKAREA, NULL);
         HandleSettingChange(SPI_SETDESKWALLPAPER, NULL);
      }
      else if (SPI_SETDESKWALLPAPER == flag) // The desktop wallpaper changed
      {
         WCHAR wszDesktopImage[MAX_PATH];

         UserSettings::Query();

         UserSettings::GetBackgroundImageLocation(wszDesktopImage,
                                                  lengthof(wszDesktopImage));

         if (m_hbmpDesktop)
         {
            DeleteObject(m_hbmpDesktop);
            m_hbmpDesktop = NULL;
         }

         InitImaging();

         if (UserSettings::GetUseCompatibleBackgroundImage())
         {
            HBITMAP hbm = NULL;

            if (IsDecoderAvailable(wszDesktopImage))
            {
               IImage * pImage = NULL;

               if (SUCCEEDED(m_pImagingFactory->CreateImageFromFile(wszDesktopImage, &pImage)) && pImage)
               {
                  ImageInfo ii = {0};
                  
                  if (SUCCEEDED(pImage->GetImageInfo(&ii)) && (0 < ii.Width && 0 < ii.Height))
                  {
                     RECT rcDest = {0};
                     ::SetRect(&rcDest, 0, 0, ii.Width, ii.Height);
                     HDC hdc = ::GetDC(m_hwnd);
                     if (hdc)
                     {
                        HDC hdcMem = CreateCompatibleDC(hdc);
                        if (hdcMem)
                        {
                           hbm = CreateCompatibleBitmap(hdc, ii.Width, ii.Height);
                           if (hbm)
                           {
                              HGDIOBJ horg = ::SelectObject(hdcMem, hbm);
                              if (horg && SUCCEEDED(pImage->Draw(hdcMem, &rcDest, NULL)))
                              {
                                 hbm = (HBITMAP)SelectObject(hdcMem, horg);
                              }
                              else
                              {
                                 ::DeleteObject(hbm);
                                 hbm = NULL;
                              }
                           }
                           ::DeleteDC(hdcMem);
                        }
                        ::ReleaseDC(m_hwnd, hdc);
                     }
                  }

                  pImage->Release();
               }
            }
            else
            {
               hbm = SHLoadDIBitmap(wszDesktopImage);
            }
            
            if (hbm)
            {
               m_hbmpDesktop = ConvertImageToScreenRes(hbm);
               ::DeleteObject(hbm);
            }
         }
         else
         {
            if (UserSettings::GetTileBackgroundImage())
            {
               m_hbmpDesktop = (HBITMAP) LoadDIBitmapBrush(wszDesktopImage);
            }
            else
            {
               m_hbmpDesktop = SHLoadDIBitmap(wszDesktopImage);
            }
         }

         result = (SUCCEEDED(PerformAction(OLECMDID_REFRESH)) ? 0 : 1);
      }
      else if (SPI_SETWORKAREA == flag) // The work area changed
      {
         // Since the taskbar may have moved recalulate the workarea now
         RECT rcPrimary = {0};
         HWND hwndTaskbar = NULL;

         GetClientRect(*m_pShellListView, &rcPrimary);
         hwndTaskbar = FindWindow(TEXT("HHTaskBar"), NULL);
         if (hwndTaskbar)
         {
            RECT rcTaskbar = {0};

            GetClientRect(hwndTaskbar, &rcTaskbar);
            rcPrimary.bottom -= (rcTaskbar.bottom-rcTaskbar.top);
            if (m_pShellListView)
            {
               ListView_SetWorkAreas(*m_pShellListView, 1, &rcPrimary);
               m_pShellListView->SnapToGrid();
            }
         }
         result = 0;
      }
      else
      {
         // Ignore the setting change
      }
   }

   return result;
}

void CDesktopView::HandleSysColorChange()
{
   if (!m_pShellListView)
   {
      return;
   }

   if (FWF_DESKTOP & m_fFlags)
   {
      // If this is the desktop window don't use the WINDOWTEXT color for the icon
      // labels, instead compute a black or white color based on the brightness of
      // the desktop background color.  We might want to consider just using the
      // COLOR_STATIC as the text color here like the bg bitmap does.
      COLORREF rgbBkColor = GetSysColor(COLOR_BACKGROUND);
      COLORREF rgbTextColor;
      int nIntensity = GetRValue(rgbBkColor) +
                       GetGValue(rgbBkColor) +
                       GetBValue(rgbBkColor);

      if (0 < nIntensity)
      {
         nIntensity /= 3;
      }

      if (128 < nIntensity)
      {
         rgbTextColor = RGB(0,0,0);
      }
      else
      {
         rgbTextColor = RGB(255,255,255);
      }

      m_pShellListView->SetColors(rgbBkColor, rgbTextColor, rgbBkColor);
   }
   else
   {
      CDefShellView::HandleSysColorChange();
   }
}

HRESULT CDesktopView::PerformAction(OLECMDID cmd)
{
   HRESULT hr = NOERROR;

   switch (cmd)
   {
      case OLECMDID_OPEN:
      {
         if (!(FWF_DESKTOP & m_fFlags))
         {
            goto DoDefault;
         }

         UINT uItemCount = 0;
         LPCITEMIDLIST * aPidls = m_pShellListView->GetSelected(&uItemCount);
         if (aPidls)
         {
             SHELLEXECUTEINFO sei = {0};
             STRRET str = {0};
             WCHAR sz[MAX_PATH];

             sei.cbSize = sizeof(sei);
             sei.hwnd = m_hwnd;
             sei.nShow = SW_SHOWNORMAL;

             for (UINT i = 0; i < uItemCount; i++)
             {
                m_pShellFolder->GetDisplayNameOf(aPidls[i],
                                                 SHGDN_NORMAL | SHGDN_FORPARSING,
                                                 &str);
                if (SUCCEEDED(StrRetToBuf(&str, aPidls[i], sz, lengthof(sz))))
                {
                   sei.lpFile = sz;
                   ShellExecuteEx(&sei);
                }
             }
             g_pShellMalloc->Free(aPidls);
         }
      }
      break;

DoDefault:
      default:
         hr = CDefShellView::PerformAction(cmd);
   }

   return hr;
}

void CDesktopView::Resize(DWORD width, DWORD height)
{
   if (!m_pShellListView)
   {
      return;
   }

   CDefShellView::Resize(width, height);
   if (FWF_DESKTOP & m_fFlags)
   {
      m_pShellListView->Arrange();
   }
}

BOOL CDesktopView::UpdateBrowserMenus(UINT uState)
{
   if (FWF_DESKTOP & m_fFlags)
   {
      return TRUE;
   }
   else
   {
      return CDefShellView::UpdateBrowserMenus(uState);
   }
}

BOOL CDesktopView::UpdateBrowserToolbar(UINT uState)
{
   BOOL bRet;

   if (FWF_DESKTOP & m_fFlags)
   {
      bRet = TRUE;
      goto leave;
   }

   if (!m_pTopShellBrowser)
   {
      bRet = FALSE;
      goto leave;
   }
    
   bRet = CDefShellView::UpdateBrowserToolbar(uState);
   HWND hwndToolbar;

   // Get toolbar handle
   if (SUCCEEDED(m_pTopShellBrowser->GetControlWindow(FCW_TOOLBAR, &hwndToolbar)))
   {
      ::SendMessage(hwndToolbar, TB_ENABLEBUTTON, IDC_GO_FOLDERUP, FALSE);
   }

leave:
   return bRet;
}

void CDesktopView::UpdateStatusBar(LPCITEMIDLIST *aPidls , UINT uItemCount, BOOL fInitialize)
{
   IShellBrowser* pTopShellBrowser = m_pTopShellBrowser;

   if (FWF_DESKTOP & m_fFlags)
   {
      return;
   }

   if (!pTopShellBrowser)
   {
      return;
   }

   pTopShellBrowser->AddRef(); // AddRef since it's possible to kill the view on another thread

   CDefShellView::UpdateStatusBar(aPidls, uItemCount, fInitialize);

   if (fInitialize)
   {
      pTopShellBrowser->SendControlMsg(FCW_STATUS,
                               SB_SETTEXT,
                               STATUS_PART_ICON,
                               (LPARAM)TEXT(""),
                               NULL);

      pTopShellBrowser->SendControlMsg(FCW_STATUS,
                               SB_SETICON,
                               STATUS_PART_ICON,
                               NULL,
                               NULL);
   }

   pTopShellBrowser->Release();
}


//////////////////////////////////////////////////
// IShellView

STDMETHODIMP CDesktopView::CreateViewWindow(LPSHELLVIEW pPrevView,
                                            LPCFOLDERSETTINGS lpfs,
                                            LPSHELLBROWSER psb,
                                            LPRECT prcView,
                                            HWND * phwnd)
{
   ASSERT(lpfs);
   ASSERT(psb);
   ASSERT(prcView);
   ASSERT(phwnd);
   if (!lpfs || !psb || !prcView || !phwnd)
   {
      return E_INVALIDARG;
   }

   HRESULT hr = NOERROR;
   *phwnd = NULL;

   // This is a normal view of the desktop (not a special case window)
   if (!(FWF_DESKTOP & lpfs->fFlags))
   {
      return CDefShellView::CreateViewWindow(pPrevView, lpfs, psb, prcView, phwnd);
   }

   // Should the caller be creating another view window without first destroying the old one?
   ASSERT(!m_hwnd);
   if (m_hwnd)
   {
      DestroyViewWindow();
   }

   WNDCLASS wc = {0};

   // Can not create two instances of the desktop
   if (::GetClassInfo(HINST_CESHELL, WC_DEFSHELLVIEW, &wc))
   {
      return E_FAIL;
   }

   // Generate the class
   wc.style = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc = (WNDPROC) WndProc;
   wc.hInstance = HINST_CESHELL;
   wc.hCursor = LoadCursor(NULL, IDC_ARROW);
   wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
   wc.lpszClassName = WC_DESKTOPVIEW;

   if (!::RegisterClass(&wc))
   {
      return E_FAIL;
   }

   // Set our internal parameters
   m_pShellBrowser = psb;
   m_fFlags = lpfs->fFlags;

   DWORD dwExStyle = 0;
   LANGID userLangID = GetUserDefaultUILanguage();

   if (LANG_ARABIC == PRIMARYLANGID(userLangID) ||
      LANG_FARSI == PRIMARYLANGID(userLangID) ||
      LANG_HEBREW == PRIMARYLANGID(userLangID) ||
      LANG_URDU == PRIMARYLANGID(userLangID))
   {
      dwExStyle |= WS_EX_LAYOUTRTL;
   }

   // Create the window
   *phwnd = ::CreateWindowEx(dwExStyle, WC_DESKTOPVIEW, NULL,
                           WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                           prcView->left, prcView->top,
                           prcView->right - prcView->left,
                           prcView->bottom - prcView->top,
                           NULL, NULL, HINST_CESHELL, (LPVOID) this);

   if (*phwnd)
   {
      // AddRef because the docs tell us to
      m_pShellBrowser->AddRef();

      // Enforce Icon view for the desktop
      m_ViewMode = m_pShellListView->SetViewMode(FVM_ICON);

      // Hook up the subclassing for the desktop
      m_wndprocListView = (WNDPROC) ::SetWindowLong(*m_pShellListView, GWL_WNDPROC,
                                                    (LONG) SubclassProc);

      if (!m_wndprocListView)
         hr = E_FAIL;
   }
   else
   {
      hr = E_FAIL;
   }

   if (SUCCEEDED(hr))
   {
      // Initialize the refresh window for the recycle bin
      g_pRecBin->CRecycleBin::BeginRecycle();
      g_pRecBin->SetDesktopWindow(*m_pShellListView);
      g_pRecBin->CRecycleBin::EndRecycle();

      // Initalize the wallpaper and show the window
      HandleSettingChange(SPI_SETDESKWALLPAPER, NULL);
      ::ShowWindow(*phwnd, SW_SHOW);
   }
   else
   {
      ::UnregisterClass(WC_DESKTOPVIEW, HINST_CESHELL);
   }

   return S_OK;
}


//////////////////////////////////////////////////
// IOleCommandTarget

STDMETHODIMP CDesktopView::QueryStatus(const GUID * pguidCmdGroup,
                                           ULONG cCmds,
                                           OLECMD prgCmds[],
                                           OLECMDTEXT * pCmdText)
{
   ASSERT(cCmds);
   ASSERT(prgCmds);
   if (!prgCmds)
   {
      return E_POINTER;
   }

   HRESULT hr = CDefShellView::QueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText);

   if (!pguidCmdGroup)
   {
      // the standard group
   }
   else if (IsEqualGUID(CGID_CEShell, *pguidCmdGroup))
   {
      // Shell cmd group
      for (ULONG i = 0; i < cCmds; i++)
      {
         // override info for the following commands
         switch(prgCmds[i].cmdID)
         {
            case IDC_GO_FOLDERUP:
               prgCmds[i].cmdf = 0;
            break;
         }
      }
   }
   
   return hr;
}
