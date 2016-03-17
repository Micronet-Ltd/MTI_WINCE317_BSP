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

#include "recbinview.h"
#include "recbin.h"
#include "resource.h"
#include "guid.h"
#include "idlist.h"
#include "shelldialogs.h"
#include "shdisp.h"

//////////////////////////////////////////////////
// CRecycleBinView

short CRecycleBinView::s_ColumnWidths[4] = {180, 100, 100, 120};

// Shared Menu
static INSMENUDATA const filemenuData[] = {
    {0, MF_BYPOSITION | MF_STRING, IDC_BITBUCKET_RESTORE, IDS_MENU_FILE_RESTORE},
    {1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL},
    {2, MF_BYPOSITION | MF_STRING, IDC_BITBUCKET_EMPTY, IDS_MENU_FILE_EMPTYRECBIN}
};
#define FILE_MENU_ITEMS   (sizeof(filemenuData)/sizeof(filemenuData[0]))

static INSMENUDATA const editmenuData[] = {
    {0, MF_BYPOSITION | MF_STRING, IDC_BITBUCKET_RESTORE, IDS_MENU_EDIT_UNDODEL}
};
#define EDIT_MENU_ITEMS   (sizeof(editmenuData)/sizeof(editmenuData[0]))

CRecycleBinView::CRecycleBinView(IShellFolder * pFolder, LPCITEMIDLIST pidlFQ) :
   CDefShellView(pFolder, pidlFQ)
{
}

HRESULT CRecycleBinView::HandleChangeNotify(ChangeNotifyEventData * pChangeNotifyEventData)
{
   if (NULL == pChangeNotifyEventData)
   {
      return E_FAIL;
   }

   // The caller has not created a view yet so there's no need to notify
   if (NULL == m_pShellListView)
   {
      delete pChangeNotifyEventData;
      return S_FALSE;
   }
      
   switch (pChangeNotifyEventData->lEvent)
   {
      case SHCNE_CREATE:
         if (pChangeNotifyEventData->pidl1)
            m_pShellListView->ReflectAdd(pChangeNotifyEventData->pidl1);            
      break;

      case SHCNE_DELETE:
         if (pChangeNotifyEventData->pidl1)
            m_pShellListView->ReflectRemove(pChangeNotifyEventData->pidl1);
      break;

      default:
         Refresh();
   }

   // Clean up the ChangeNotifyEventData object
   delete pChangeNotifyEventData;

   return S_OK;
}

LRESULT CRecycleBinView::HandleCommand(DWORD dwCmd)
{
   switch (dwCmd)
   {
      case IDC_BITBUCKET_RESTORE:
      {
         UINT uItemCount = 0;
         LPCITEMIDLIST * aPidls = m_pShellListView->GetSelected(&uItemCount);

         if (aPidls && uItemCount)
         {            
            for (UINT i = 0; i < uItemCount; i++)
            {
               g_pRecBin->BeginRecycle();
               g_pRecBin->RestoreFile(aPidls[i]);
               g_pRecBin->EndRecycle();
            }            
            g_pShellMalloc->Free(aPidls);
         }
      }
      break;

      case IDC_BITBUCKET_RESTORE_ALL:
      {
         g_pRecBin->BeginRecycle();
         g_pRecBin->RestoreAll();
         g_pRecBin->EndRecycle();
      }
      break;

      case IDC_BITBUCKET_EMPTY:
      {
         BOOL fConfirmDel = TRUE;

         g_pRecBin->BeginRecycle();                 
         g_pRecBin->DeleteAll(fConfirmDel);
         g_pRecBin->EndRecycle();
      }
      break;

      case IDC_FILE_DELETE:
      {
         UINT uItemCount = 0;
         LPCITEMIDLIST * aPidls = m_pShellListView->GetSelected(&uItemCount);

         if (aPidls && uItemCount)
         {
            HRESULT hr = NOERROR;
            int result = IDYES;

            if (1 == uItemCount)
            {
               TCHAR szTmp[MAX_PATH] = TEXT("");

               g_pRecBin->BeginRecycle();
               LPOLESTR str = g_pRecBin->GetFileOrigName(aPidls[0]);
               g_pRecBin->EndRecycle();

               if (str)
               {
                  hr = StringCchCopy(szTmp, MAX_PATH, str);
                  g_pShellMalloc->Free(str);
               }

               if (SUCCEEDED(hr))
               {
                  hr = ShellDialogs::Confirm::DeleteForRecycleBin(szTmp, NULL, FALSE, &result);
               }
            }
            else
            {
               hr = ShellDialogs::Confirm::DeleteMultiple(uItemCount, NULL,
                                                             TRUE, &result);
            }
            
            if (SUCCEEDED(hr) && IDYES == result)
            {
               for (UINT i = 0; i < uItemCount; i++)
               {
                  g_pRecBin->BeginRecycle();
                  g_pRecBin->DeleteFileFolder(aPidls[i]);
                  g_pRecBin->EndRecycle();
               }
            }
                        
            g_pShellMalloc->Free(aPidls);
         }
      }
      break;

      // This should be unavailable in the bitbucket view
      case IDC_FILE_NEWFOLDER:
      case IDC_FILE_OPEN:
      case IDC_FILE_RENAME:
         ASSERT(0);
      break;

      default:
         return CDefShellView::HandleCommand(dwCmd);
   }

   return 0;
}

LRESULT CRecycleBinView::HandleInitMenuPopup(HMENU hmenu, PopupMenuInfo * pPopupMenuInfo)
{   
   LRESULT result = 0; // Handled

   if (!pPopupMenuInfo)
   {
       // Do the main menu (or view and arrange-by menu)
      result = CDefShellView::HandleInitMenuPopup(hmenu, pPopupMenuInfo);
   }
   else
   {
      if (pPopupMenuInfo->m_pContextMenu)
      {
         pPopupMenuInfo->m_pContextMenu->QueryContextMenu(hmenu, USER_INSERT_OFFSET,
                                                          IDC_USER_MENU_ITEMS_BEGIN,
                                                          IDC_USER_MENU_ITEMS_END,
                                                          CMF_NORMAL);
      }

      switch (pPopupMenuInfo->offset)
      {
         case FOLDER_MENU_OFFSET:
         {
            g_pRecBin->BeginRecycle();
            if (g_pRecBin->IsEmpty())
               ::EnableMenuItem(hmenu, IDC_BITBUCKET_EMPTY, MF_BYCOMMAND | MF_GRAYED);
            
            g_pRecBin->EndRecycle();

            ::EnableMenuItem(hmenu, IDC_EDIT_PASTE, MF_BYCOMMAND | MF_GRAYED);
         }
         break;

         default:
            result = 1; // Not handled
      }
   }

   return result;
}

LRESULT CRecycleBinView::HandleNotify(LPNMHDR lpnmh)
{
   ASSERT(lpnmh);
   if (!lpnmh)
      return 0;

   switch(lpnmh->code)
   {
      case HDN_ITEMCHANGED:
      case HDN_ENDTRACK:
      {
         NMHEADER * lpnmhdr = (NMHEADER *) lpnmh;
         if (4 > (lpnmhdr->iItem) && (HDI_WIDTH & lpnmhdr->pitem->mask))
            s_ColumnWidths[lpnmhdr->iItem] = (short)lpnmhdr->pitem->cxy;
      }
      break;

      default:
         return CDefShellView::HandleNotify(lpnmh);
   }
   
   return 0;
}

void CRecycleBinView::LoadColumnHeaders()
{
   ASSERT(m_pShellListView);
   if (!m_pShellListView)
      return;

   LPCTSTR pszHeader = NULL;

   pszHeader = LOAD_STRING(IDS_HDR_DEFVIEW_NAME);
   if (pszHeader)
      m_pShellListView->AppendColumn(pszHeader, s_ColumnWidths[0], LVCFMT_LEFT);

   pszHeader = LOAD_STRING(IDS_HDR_DEFVIEW_SIZE);
   if (pszHeader)
      m_pShellListView->AppendColumn(pszHeader, s_ColumnWidths[1], LVCFMT_RIGHT);

   pszHeader = LOAD_STRING(IDS_HDR_RECBINVIEW_LOCATION);
   if (pszHeader)
      m_pShellListView->AppendColumn(pszHeader, s_ColumnWidths[2], LVCFMT_LEFT);

   pszHeader = LOAD_STRING(IDS_HDR_RECBINVIEW_DATE);
   if (pszHeader)
      m_pShellListView->AppendColumn(pszHeader, s_ColumnWidths[3], LVCFMT_LEFT);
}

HMENU CRecycleBinView::LoadPopupMenuTemplate()
{
   return ::LoadMenu(HINST_CESHELL, MAKEINTRESOURCE(IDM_RECBIN_POPUP_MENUS));
}

HRESULT CRecycleBinView::PerformAction(OLECMDID cmd)
{
   HRESULT hr = NOERROR;

   switch (cmd)
   {
      case OLECMDID_OPEN:
      {
         UINT uItemCount = 0;

         LPCITEMIDLIST * aPidls = m_pShellListView->GetSelected(&uItemCount);

         if (aPidls && uItemCount)   // Launch properties for one item (as for the desktop).
         {
            LPCITEMIDLIST pidl = NULL;

            hr = CreateNameSpacePidl(CLSID_CEShellBitBucket, (LPITEMIDLIST *) &pidl);
            if (SUCCEEDED(hr))
            {
               ShellDialogs::Properties::ObjectProperties(pidl, m_pShellFolder, this, aPidls, 1);
               ILFree(pidl);
            }

            g_pShellMalloc->Free(aPidls);
         }
      }
      break;

      case OLECMDID_COPY:
      case OLECMDID_CUT:
      case OLECMDID_PASTE:
         break;

      default:
         return CDefShellView::PerformAction(cmd);
   }

   return hr;
}

BOOL CRecycleBinView::UpdateBrowserMenus(UINT uState)
{
   CDefShellView::UpdateBrowserMenus(uState);
   
// OnActivate
//only do this if we are active
   if (SVUIA_DEACTIVATE != uState)
   {
      if (m_hMenu)
      {
         HMENU hmenuSub, hmenuTrack;
         BOOL fActivateFocus= (BOOL)(SVUIA_ACTIVATE_FOCUS == uState);

         // Insert RecycleBin items into the File menu
         if (NULL != (hmenuSub=::GetSubMenu(m_hMenu, FILE_MENU_OFFSET)))
         {
            // Remove SendTo Popup menu
            if (NULL != (hmenuTrack= ::GetSubMenu(hmenuSub, 7)))
            {
               ::RemoveMenu(hmenuSub, 7, MF_BYPOSITION);
               ::DestroyMenu(hmenuTrack);
            }
            // Remove Separator
            ::RemoveMenu(hmenuSub, 7, MF_BYPOSITION);

            // Remove 
            ::RemoveMenu(hmenuSub, 1, MF_BYPOSITION);
            ::RemoveMenu(hmenuSub, 0, MF_BYPOSITION);
            
            CDefShellView::MergeSubMenu(hmenuSub, (LPINSMENUDATA)filemenuData, FILE_MENU_ITEMS, fActivateFocus);
         }

         // Insert RecycleBin items into the Edit menu
         if(NULL != (hmenuSub=::GetSubMenu(m_hMenu, EDIT_MENU_OFFSET)))
         {
            // Remove standard Undo
            ::RemoveMenu(hmenuSub, 0, MF_BYPOSITION);
            
            CDefShellView::MergeSubMenu(hmenuSub, (LPINSMENUDATA)editmenuData, EDIT_MENU_ITEMS, fActivateFocus);
         }

         // Insert RecycleBin items into the View menu
         if(NULL != (hmenuSub=::GetSubMenu(m_hMenu, VIEW_MENU_OFFSET)))
         {            
            HMENU hmenuLoad, hmenuTemp, hmenuIns;

            // Remove "Arrange Icons By"
            if (NULL != (hmenuTemp=::GetSubMenu(hmenuSub, 4)))
            {
               ::RemoveMenu(hmenuSub, 4, MF_BYPOSITION);
               ::DestroyMenu(hmenuTemp);
            }

            // Add "Arrange Icons" specific for the Recycle Bin
            if (NULL != (hmenuLoad = ::LoadMenu(HINST_CESHELL, MAKEINTRESOURCE(IDM_RECBIN_POPUP_MENUS))))
            {
               hmenuTemp = ::GetSubMenu(hmenuLoad, FOLDER_MENU_OFFSET);
               hmenuIns = ::GetSubMenu(hmenuTemp, 3);
              
               ::RemoveMenu(hmenuTemp, 3, MF_BYPOSITION);
               ::DestroyMenu(hmenuLoad);
            
               if (!::InsertMenu(hmenuSub, 4, MF_BYPOSITION | MF_POPUP, (UINT)hmenuIns, LOAD_STRING(IDS_MENU_VIEW_ARRANGEICONS)))
               {
                  ::DestroyMenu(hmenuIns);
               }
            }
         }        
      }
   }

   return TRUE;
}

void CRecycleBinView::UpdateCurrentDropTarget(DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{
   PREFAST_ASSERT(m_pDragData);

   HRESULT hr = NOERROR;
   IDropTarget * pDropTarget = NULL;
   LPCITEMIDLIST pidl = NULL;

   hr = CreateNameSpacePidl(CLSID_CEShellBitBucket, (LPITEMIDLIST *) &pidl);
   if (SUCCEEDED(hr))
   {
      hr = m_pShellFolder->GetUIObjectOf(*m_pShellListView, 1, &pidl, IID_IDropTarget,
                                         NULL, (VOID **) &pDropTarget);
      ILFree(pidl);
   }

   // Clean up current drop target
   if (m_pDragData->m_pCurrentDropTarget)
   {
      m_pDragData->m_pCurrentDropTarget->DragLeave();
      m_pDragData->m_pCurrentDropTarget->Release();
   }

   // Set the new drop target
   if (SUCCEEDED(hr))
   {
      ASSERT(pDropTarget);
      m_pDragData->m_pCurrentDropTarget = pDropTarget;
      m_pDragData->m_pCurrentDropTarget->DragEnter(m_pDragData->m_pDataObject,
                                                   dwKeyState, pt, pdwEffect);
   }
   else
   {
      ASSERT(!pDropTarget);
      if (pDropTarget)
         pDropTarget->Release();

      m_pDragData->m_pCurrentDropTarget = NULL;
      *pdwEffect = DROPEFFECT_NONE;
   }
}

void CRecycleBinView::UpdateStatusBar(LPCITEMIDLIST *aPidls , UINT uItemCount, BOOL fInitialize)
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
// IShellView

STDMETHODIMP CRecycleBinView::CreateViewWindow(LPSHELLVIEW pPrevView, 
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
      return E_INVALIDARG;
   *phwnd = NULL;

   FOLDERSETTINGS fs = *lpfs;
   fs.ViewMode = FVM_DETAILS;
   return CDefShellView::CreateViewWindow(pPrevView, &fs, psb, prcView, phwnd);
}

//////////////////////////////////////////////////
// IOleCommandTarget

STDMETHODIMP CRecycleBinView::QueryStatus(const GUID * pguidCmdGroup,
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
      if (aPidls)
         g_pShellMalloc->Free(aPidls);
      
      for (ULONG i = 0; i < cCmds; i++)
      {
         // override info for the following commands
         switch(prgCmds[i].cmdID)
         {
            case IDC_FILE_DELETE:
            case IDC_BITBUCKET_RESTORE:
               if(uItemCount)
                  prgCmds[i].cmdf = OLECMDF_ENABLED;
               else
                  prgCmds[i].cmdf = OLECMDF_SUPPORTED;
            break;

            case IDC_BITBUCKET_EMPTY:
            {
               g_pRecBin->BeginRecycle();

               if (g_pRecBin->IsEmpty())
                  prgCmds[i].cmdf = OLECMDF_SUPPORTED;
               else
                  prgCmds[i].cmdf = OLECMDF_ENABLED;

               g_pRecBin->EndRecycle();
            }
            break;

            case IDC_FILE_RENAME:
            case IDC_EDIT_UNDO:
            case IDC_EDIT_CUT:
            case IDC_EDIT_COPY:
            case IDC_EDIT_PASTE:
            case IDC_EDIT_PASTESHORTCUT:
               prgCmds[i].cmdf = OLECMDF_SUPPORTED;
            break;              
         }
      }
   }
   
   return hr;
}

//IDropSource interface contains methods used in any application used as a data source in 
//a drag-and-drop operation.

//QueryContinueDrag determines whether a drag-and-drop operation should be continued, canceled, or completed. 
STDMETHODIMP CRecycleBinView::QueryContinueDrag(BOOL fEsc,
                                              DWORD dwKeyState)
{
   return E_NOTIMPL;
}

//GiveFeedback enables a source application to give visual feedback to the end user during 
//a drag-and-drop operation 
//GiveFeedback is responsible for changing the cursor shape or for changing the highlighted source 
//based on the value of the dwEffect parameter. If you are using default cursors, you can 
//return DRAGDROP_S_USEDEFAULTCURSORS, which causes OLE to update the cursor for you, using its defaults. 
STDMETHODIMP CRecycleBinView::GiveFeedback(DWORD dwEffect)
{
   return E_NOTIMPL;
}

