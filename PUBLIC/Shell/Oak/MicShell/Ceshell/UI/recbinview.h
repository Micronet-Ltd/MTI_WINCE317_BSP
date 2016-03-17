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

class CRecycleBinView :
   public CDefShellView
{
protected:
   static short s_ColumnWidths[4];

   virtual HRESULT HandleChangeNotify(ChangeNotifyEventData * pChangeNotifyEventData);
   virtual LRESULT HandleCommand(DWORD dwCmd);
   virtual LRESULT HandleInitMenuPopup(HMENU hmenu, PopupMenuInfo * pPopupMenuInfo);
   virtual LRESULT HandleNotify(LPNMHDR lpnmh);
   virtual void LoadColumnHeaders();
   virtual HMENU LoadPopupMenuTemplate();
   virtual HRESULT PerformAction(OLECMDID cmd);
   virtual void UpdateCurrentDropTarget(DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect);

public:
   CRecycleBinView(IShellFolder * pFolder, LPCITEMIDLIST pidlFQ);

   // IShellView methods
   STDMETHOD (CreateViewWindow) (LPSHELLVIEW, LPCFOLDERSETTINGS, LPSHELLBROWSER, LPRECT, HWND*);

   // IOleCommandTarget
   STDMETHOD (QueryStatus)(const GUID*, ULONG, OLECMD[], OLECMDTEXT*);

   // IDropSource methods
   STDMETHOD (QueryContinueDrag)(BOOL, DWORD);
   STDMETHOD (GiveFeedback)(DWORD);

   virtual BOOL UpdateBrowserMenus(UINT uState);
   virtual void UpdateStatusBar(LPCITEMIDLIST *aPidls , UINT uItemCount, BOOL fInitialize);

};

