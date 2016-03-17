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
#include "defshellview.h"
#include <imaging.h>

class CDesktopView :
   public CDefShellView
{
protected:
   WNDPROC m_wndprocListView;
   HBITMAP m_hbmpDesktop;
   IImagingFactory * m_pImagingFactory;
   bool m_bIsComInit;
	
   static const TCHAR WC_DESKTOPVIEW[];
   
   static LRESULT CALLBACK SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

   HBITMAP ConvertImageToScreenRes(HBITMAP hbmSrc);
   void InitImaging();
   BOOL IsDecoderAvailable(LPCWSTR pwszFileName);
   HBRUSH LoadDIBitmapBrush(LPCWSTR wszFileName);
   void PaintBackground(HDC hdc) const;
   void UnInitImaging();
	
   // CDefShellView
   virtual void GetDisplayInfo(NMLVDISPINFO * lpdi);
   virtual LRESULT HandleCommand(DWORD dwCmd);
   virtual LRESULT HandleInitMenuPopup(HMENU hmenu, PopupMenuInfo * pPopupMenuInfo);
   virtual LRESULT HandleSettingChange(DWORD flag, LPCWSTR pwszSectionName);
   virtual void HandleSysColorChange();
   virtual HRESULT PerformAction(OLECMDID cmd);
   virtual void Resize(DWORD width, DWORD height);
   virtual BOOL UpdateBrowserMenus(UINT uState);
   virtual BOOL UpdateBrowserToolbar(UINT uState);
   virtual void UpdateStatusBar(LPCITEMIDLIST *aPidls , UINT uItemCount, BOOL fInitialize);

public:
   CDesktopView(IShellFolder * pFolder, LPCITEMIDLIST pidlFQ);
   virtual ~CDesktopView();

   // IShellView methods
   STDMETHOD (CreateViewWindow) (LPSHELLVIEW, LPCFOLDERSETTINGS, LPSHELLBROWSER, LPRECT, HWND*);

   // IOleCommandTarget
   STDMETHOD (QueryStatus)(const GUID*, ULONG, OLECMD[], OLECMDTEXT*);
};

