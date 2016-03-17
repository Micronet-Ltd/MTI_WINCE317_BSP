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

#include "netrootview.h"
#include "resource.h"
#include "shdisp.h"

//////////////////////////////////////////////////
// CNetworkRootView

CNetworkRootView::CNetworkRootView(IShellFolder * pFolder, LPCITEMIDLIST pidlFQ) :
   CDefShellView(pFolder, pidlFQ)
{
}

CNetworkRootView::~CNetworkRootView()
{
}

LRESULT CNetworkRootView::HandleInitMenuPopup(HMENU hmenu, PopupMenuInfo * pPopupMenuInfo)
{
   LRESULT result = CDefShellView::HandleInitMenuPopup(hmenu, pPopupMenuInfo);

   if ((0 == result) && pPopupMenuInfo)
   {
      switch (pPopupMenuInfo->offset)
      {
         case FOLDER_MENU_OFFSET:
         {
            // Don't allow pastes in the root of a network (\\foo)
            ::EnableMenuItem(hmenu, IDC_EDIT_PASTE, MF_BYCOMMAND | MF_GRAYED);
            ::EnableMenuItem(hmenu, IDC_EDIT_PASTESHORTCUT, MF_BYCOMMAND | MF_GRAYED);
            ::EnableMenuItem(hmenu, IDC_FILE_NEWFOLDER, MF_BYCOMMAND | MF_GRAYED);
            ::EnableMenuItem(hmenu, IDC_FILE_PROPERTIES, MF_BYCOMMAND | MF_GRAYED);
         }
         break;

         case ITEM_MENU_OFFSET:
         {
            ::EnableMenuItem(hmenu, IDC_EDIT_CUT, MF_BYCOMMAND | MF_GRAYED);
            ::EnableMenuItem(hmenu, IDC_FILE_DELETE, MF_BYCOMMAND | MF_GRAYED);
            ::EnableMenuItem(hmenu, IDC_FILE_RENAME, MF_BYCOMMAND | MF_GRAYED);
         }
         break;
      }  
   }

   return result;
}

BOOL CNetworkRootView::UpdateBrowserToolbar(UINT uState)
{
   BOOL bRet;

   if (!m_pTopShellBrowser)
   {
      bRet = FALSE;
      goto leave;
   }

   bRet = CDefShellView::UpdateBrowserToolbar(uState);

   HWND hwndTB;   
   if (SUCCEEDED(m_pTopShellBrowser->GetControlWindow(FCW_TOOLBAR, &hwndTB)))
   {
      ::SendMessage(hwndTB, TB_SETSTATE, IDC_FILE_DELETE, (LPARAM)0);
      ::SendMessage(hwndTB, TB_SETSTATE, IDC_FILE_PROPERTIES, (LPARAM)0);
      ::SendMessage(hwndTB, TB_SETSTATE, IDC_GO_FOLDERUP, (LPARAM)0);
   }

leave:
   return bRet;
}

void CNetworkRootView::UpdateStatusBar(LPCITEMIDLIST *aPidls , UINT uItemCount, BOOL fInitialize)
{
   IShellBrowser* pTopShellBrowser = m_pTopShellBrowser;

   if (!pTopShellBrowser)
      return;

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
// IOleCommandTarget

STDMETHODIMP CNetworkRootView::QueryStatus(const GUID * pguidCmdGroup,
                                           ULONG cCmds,
                                           OLECMD prgCmds[],
                                           OLECMDTEXT * pCmdText)
{
   ASSERT(cCmds);
   ASSERT(prgCmds);
   if (!prgCmds)
      return E_POINTER;

   HRESULT hr = CDefShellView::QueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText);

   if (!pguidCmdGroup)
   {
      // the standard group
   }
   else if (IsEqualGUID(CGID_CEShell, *pguidCmdGroup))
   {
      // Shell cmd group
      UINT uItemCount = 0;
      LPCITEMIDLIST * aPidls = m_pShellListView->GetSelected(&uItemCount);
      
      for (ULONG i = 0; i < cCmds; i++)
      {
         // override info for the following commands
         switch(prgCmds[i].cmdID)
         {
            case IDC_FILE_NEWFOLDER:
            case IDC_FILE_DELETE:
            case IDC_FILE_RENAME:
            case IDC_EDIT_CUT:
            case IDC_EDIT_PASTE:
            case IDC_EDIT_PASTESHORTCUT:
            case IDC_GO_FOLDERUP:
               prgCmds[i].cmdf = OLECMDF_SUPPORTED;
            break;

            case IDC_FILE_PROPERTIES:
               if (0 == uItemCount)
               {
                  prgCmds[i].cmdf = OLECMDF_SUPPORTED;
               }
            break;

            case IDC_FILE_SENDTO_MYDOCUMENTS:
               if (0 != uItemCount)
               {
                  prgCmds[i].cmdf = OLECMDF_SUPPORTED;
               }
            break;
         }
      }

      if(aPidls)
         g_pShellMalloc->Free(aPidls);
   }
   
   return hr;
}

