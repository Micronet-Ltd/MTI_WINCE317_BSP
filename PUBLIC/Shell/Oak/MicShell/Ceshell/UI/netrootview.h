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

class CNetworkRootView :
   public CDefShellView
{
protected:
   // CDefShellView
   virtual LRESULT HandleInitMenuPopup(HMENU hmenu, PopupMenuInfo * pPopupMenuInfo);
   virtual BOOL UpdateBrowserToolbar(UINT uState);
   virtual void UpdateStatusBar(LPCITEMIDLIST *aPidls , UINT uItemCount, BOOL fInitialize);

public:
   CNetworkRootView(IShellFolder * pFolder, LPCITEMIDLIST pidlFQ);
   virtual ~CNetworkRootView();

   // IOleCommandTarget
   STDMETHOD (QueryStatus)(const GUID*, ULONG, OLECMD[], OLECMDTEXT*);
};

