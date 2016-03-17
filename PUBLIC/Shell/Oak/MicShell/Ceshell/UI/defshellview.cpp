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

#include "defshellview.h"
#include "cundo.h"
#include "datatrans.h"
#include "filechangemgr.h"
#include "guid.h"
#include "idlist.h"
#include "recbin.h"
#include "resource.h"
#include "shelldialogs.h"
#include "usersettings.h"
#include <docobj.h>
#include <shdisp.h>
#include <shellsdk.h> // for GN_CONTEXTMENU

#define WM_DEFSHELLVIEW_SELECTION_UPDATED (WM_USER+0x150)
#define WM_DEFSHELLVIEW_CHANGENOTIFYEVENT (WM_USER+0x151)


//////////////////////////////////////////////////
// CDefShellView::ChangeNotifyEventData

CDefShellView::ChangeNotifyEventData::ChangeNotifyEventData()
{
   ZeroMemory(this, sizeof(*this));
}

CDefShellView::ChangeNotifyEventData::~ChangeNotifyEventData()
{
   ILFree(pidl1);
   ILFree(pidl2);
}


//////////////////////////////////////////////////
// CDefShellView

const WCHAR CDefShellView::WC_DEFSHELLVIEW[] = TEXT("DefShellView");
const UINT CDefShellView::IN_LABEL_EDIT = 0x0001;
const UINT CDefShellView::IN_REFRESH = 0x0002;
const UINT CDefShellView::BACK_ENABLED = 0x0004;
const UINT CDefShellView::FWD_ENABLED = 0x0008;
const UINT CDefShellView::SORT_DECENDING = 0x0010;
const UINT CDefShellView::PERSIST_VIEWMODE = 0x0020;
const UINT CDefShellView::IN_LABEL_APPLY = 0x0040;
const UINT CDefShellView::SELECTION_UPDATE_NEEDED = 0x0080;

UINT CDefShellView::s_CurrentUndoStr = IDS_MENU_EDIT_UNDO;
short CDefShellView::s_ColumnWidths[4] = {180, 80, 130, 150};
DWORD CDefShellView::s_TTRefCount = 0;
LPWSTR CDefShellView::s_pszTooltips[TOOLTIPS] = {0};
DWORD CDefShellView::s_FSRefCount = 0;
LPWSTR CDefShellView::s_pszDRootName = NULL;
HICON CDefShellView::s_hDRootIcon = NULL;

// Shared Menu
static INSMENUDATA const filemenuData[] = {
    {0, MF_BYPOSITION | MF_STRING, IDC_FILE_OPEN, IDS_MENU_FILE_OPEN},
    {1, MF_BYPOSITION | MF_STRING, IDC_FILE_NEWFOLDER, IDS_MENU_FILE_NEWFOLDER},
    {2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL},
    {3, MF_BYPOSITION | MF_STRING, IDC_FILE_DELETE, IDS_MENU_FILE_DELETE},
    {4, MF_BYPOSITION | MF_STRING, IDC_FILE_RENAME, IDS_MENU_FILE_RENAME},
    {5, MF_BYPOSITION | MF_STRING, IDC_FILE_PROPERTIES, IDS_MENU_FILE_PROPERTIES},
    {6, MF_BYPOSITION | MF_SEPARATOR, 0, NULL},
    {7, MF_BYPOSITION | MF_SEPARATOR, 0, NULL}
};
#define FILE_MENU_ITEMS   (sizeof(filemenuData)/sizeof(filemenuData[0]))

static INSMENUDATA const editmenuData[] = {
    {1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL},
    {2, MF_BYPOSITION | MF_STRING, IDC_EDIT_CUT, IDS_MENU_EDIT_CUT},
    {3, MF_BYPOSITION | MF_STRING, IDC_EDIT_COPY, IDS_MENU_EDIT_COPY},
    {4, MF_BYPOSITION | MF_STRING, IDC_EDIT_PASTE, IDS_MENU_EDIT_PASTE},
    {5, MF_BYPOSITION | MF_STRING, IDC_EDIT_PASTESHORTCUT, IDS_MENU_EDIT_PASTESHORTCUT},
    {6, MF_BYPOSITION | MF_SEPARATOR, 0, NULL},
    {7, MF_BYPOSITION | MF_STRING, IDC_EDIT_SELECTALL, IDS_MENU_EDIT_SELECTALL}
};
#define EDIT_MENU_ITEMS   (sizeof(editmenuData)/sizeof(editmenuData[0]))

static INSMENUDATA const viewmenuData[] = {
    {0, MF_BYPOSITION | MF_STRING, IDC_VIEW_ICONS, IDS_MENU_VIEW_LARGE},
    {1, MF_BYPOSITION | MF_STRING, IDC_VIEW_SMALL, IDS_MENU_VIEW_SMALL},
    {2, MF_BYPOSITION | MF_STRING, IDC_VIEW_DETAILS, IDS_MENU_VIEW_DETAILS},
    {3, MF_BYPOSITION | MF_SEPARATOR, 0, NULL},
    {4, MF_BYPOSITION | MF_SEPARATOR, 0, NULL},
    {6, MF_BYPOSITION | MF_STRING, IDC_VIEW_OPTIONS, IDS_MENU_VIEW_OPTIONS}
};
#define VIEW_MENU_ITEMS   (sizeof(viewmenuData)/sizeof(viewmenuData[0]))

static INSMENUDATA const gomenuData[] = {
    {(UINT)-1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL},
    {(UINT)-1, MF_BYPOSITION | MF_STRING, IDC_GO_MYDOCUMENTS, IDS_MENU_GO_MYDOCUMENTS},
};
#define GO_MENU_ITEMS   (sizeof(gomenuData)/sizeof(gomenuData[0]))

// toolbar
static TBBUTTON tbButton[] = {
  {0, IDC_GO_BACK, 0, TBSTYLE_BUTTON, 0, 0, 0, -1},
  {1, IDC_GO_FORWARD, 0, TBSTYLE_BUTTON, 0, 0, 0, -1},
  {0, 0, 0, TBSTYLE_SEP, 0, 0, 0, -1},
  {VIEW_PARENTFOLDER+31, IDC_GO_FOLDERUP, TBSTATE_ENABLED, TBSTYLE_BUTTON,  0, 0, 0, -1},
  {0, 0, 0, TBSTYLE_SEP, 0, 0, 0, -1},
  {STD_DELETE+16, IDC_FILE_DELETE, 0, TBSTYLE_BUTTON, 0, 0, 0, -1},
  {STD_PROPERTIES+16, IDC_FILE_PROPERTIES, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
  {0, 0, 0, TBSTYLE_SEP, 0, 0, 0, -1},
  {VIEW_DETAILS+31, IDC_VIEW_TYPE, TBSTATE_ENABLED, TBSTYLE_DROPDOWN,   0, 0, 0, -1},

};
#define TOOLBAR_ITEMS   (sizeof(tbButton)/sizeof(tbButton[0]))



CDefShellView::CDefShellView(IShellFolder * pFolder, LPCITEMIDLIST pidlFQ) :
   m_ObjRefCount(0),
   m_pidlFQ(NULL),
   m_hwnd(NULL),
   m_hIcon(NULL),
   m_hMenu(NULL),
   m_pszFilter(NULL),
   m_pColumnWidths(NULL),
   m_pShellListView(NULL),
   m_ArrangeBy(0),
   m_Fmt(0),
   m_uWatchID(0),
   m_uFocus(SVUIA_DEACTIVATE),
   m_uState(PERSIST_VIEWMODE),
   m_fFlags(0),
   m_ViewMode(FVM_ICON),
   m_pDragData(NULL),
   m_pShellFolder(pFolder),
   m_pShellBrowser(NULL),
   m_pTopShellBrowser(NULL),
   m_pOleInPlaceUIWindow(NULL)
{
   m_hAccels = LoadAccelerators(HINST_CESHELL, MAKEINTRESOURCE(IDA_DEFVIEW));
   m_pidlFQ = ILCopy(pidlFQ, IL_ALL);
   m_pShellListView = new CShellListView();
   m_ObjRefCount = 1;
}

CDefShellView::~CDefShellView()
{
   if (m_pShellListView)
   {
      delete m_pShellListView;
   }

   ASSERT(!m_hwnd);
   if (m_hwnd)
   {
      DestroyViewWindow();
   }

   ASSERT(!m_pDragData);
   if (m_pDragData)
   {
      if (m_pDragData->m_pCurrentDropTarget)
      {
         m_pDragData->m_pCurrentDropTarget->Release();
      }
      delete m_pDragData;
   }

   if (m_pidlFQ)
   {
      ILFree(m_pidlFQ);
   }

   if (m_pShellFolder)
   {
      m_pShellFolder->Release();
   }

   if (m_pszFilter)
   {
      g_pShellMalloc->Free(m_pszFilter);
   }

   if (m_pColumnWidths)
   {
      g_pShellMalloc->Free(m_pColumnWidths);
   }
}

LRESULT CALLBACK CDefShellView::WndProc(HWND hwnd,
                                        UINT uMsg,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
   CDefShellView * pThis = (CDefShellView*) ::GetWindowLong(hwnd, GWL_USERDATA);
   LRESULT result = 0;

   if (!pThis && (uMsg != WM_CREATE))
   {
      goto DoDefault;
   }

   switch (uMsg)
   {
      case WM_ACTIVATE:
         pThis->m_uFocus = SVUIA_ACTIVATE_FOCUS;
         pThis->UpdateBrowserMenus(SVUIA_ACTIVATE_FOCUS);
      break;

      case WM_COMMAND:
         result = pThis->HandleCommand(GET_WM_COMMAND_ID(wParam, lParam));
      break;

      case WM_CREATE:
      {
         LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
         pThis = (CDefShellView *) (lpcs->lpCreateParams);
         PREFAST_ASSERT(pThis);
         result = pThis->Create(hwnd, lpcs->cx, lpcs->cy);
      }
      break;

      case WM_DESTROY:
         pThis->Destroy();
      break;

      case WM_INITMENUPOPUP:
      {
         HMENU hmenu = (HMENU) wParam;
         MENUITEMINFO mii = {0};
         mii.cbSize = sizeof(mii);
         mii.fMask = MIIM_DATA;
         if (::GetMenuItemInfo(hmenu, 0, TRUE, &mii))
         {
            result = pThis->HandleInitMenuPopup(hmenu, (PopupMenuInfo *) mii.dwItemData);
         }
         else
         {
            result = 1;
         }
      }
      break;

      case WM_NOTIFY:
         result = pThis->HandleNotify((LPNMHDR) lParam);
      break;

      case WM_SETFOCUS:
         // Tell the browser one of our windows has received the focus. This should
         // always be done before merging menus if one of our windows has the focus.
         PREFAST_ASSERT(pThis->m_pShellBrowser);
         pThis->m_pShellBrowser->OnViewWindowActive(pThis);
         ::SetFocus(*(pThis->m_pShellListView));
      break;

      // case WM_FONTCHANGE:

      case WM_SETTINGCHANGE: // WM_WININICHANGE
         result = pThis->HandleSettingChange(wParam, (LPCTSTR)lParam);
      break;

      case WM_SIZE:
         pThis->Resize(LOWORD(lParam), HIWORD(lParam));
      break;

      case WM_SYSCOLORCHANGE:
         pThis->HandleSysColorChange();
      break;

      case WM_HELP:
         result = pThis->HandleCommand(IDC_HELP_TOPICS);
      break;

      case WM_DEFSHELLVIEW_SELECTION_UPDATED:
		 pThis->UpdateSelectionMenu();
      break;

      case WM_DEFSHELLVIEW_CHANGENOTIFYEVENT:
         result = static_cast<LRESULT>(pThis->HandleChangeNotify((ChangeNotifyEventData*)lParam));
      break;

DoDefault:
      default:
         result = ::DefWindowProc(hwnd, uMsg, wParam, lParam);
   }

   return result;
}

int CALLBACK CDefShellView::CompareItems(LPARAM lParam1,
                                         LPARAM lParam2,
                                         LPARAM lParamSort)
{
   LPCITEMIDLIST pidl1 = NULL;
   LPCITEMIDLIST pidl2 = NULL;
   CDefShellView * pThis = (CDefShellView *) lParamSort;
   HRESULT hr = NOERROR;
   int cmp = 0;

   if (SORT_DECENDING & pThis->m_uState)
   {
      pidl1 = (LPCITEMIDLIST) lParam2;
      pidl2 = (LPCITEMIDLIST) lParam1;
   }
   else
   {
      pidl1 = (LPCITEMIDLIST) lParam1;
      pidl2 = (LPCITEMIDLIST) lParam2;
   }

   PREFAST_ASSERT(pThis->m_pShellFolder);
   hr = pThis->m_pShellFolder->CompareIDs(pThis->m_ArrangeBy, pidl1, pidl2);
   if (SUCCEEDED(hr))
   {
      cmp = (int) ((short) HRESULT_CODE(hr));
   }

   return cmp;
}

DWORD CDefShellView::TTAddRef()
{
   if (s_TTRefCount == 0)
   {
      for(int i=0; i<TOOLTIPS; i++)
      {
         ASSERT(!s_pszTooltips[i]);
         s_pszTooltips[i] = LOAD_STRING(IDS_TTSTR1+i);
      }
   }
   return ++s_TTRefCount;
}

DWORD CDefShellView::TTRelease()
{
   if (--s_TTRefCount == 0)
   {
      for (int i = 0; i < TOOLTIPS; i++)
      {
         s_pszTooltips[i] = NULL;
      }
   }
   return s_TTRefCount;
}

DWORD CDefShellView::FSAddRef()
{
   if (s_FSRefCount == 0)
   {
      LPITEMIDLIST pidlDrivesRoot = NULL;
      HRESULT hr;

      hr = SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlDrivesRoot);
      if (SUCCEEDED(hr))
      {
         IShellFolder * pFolder = NULL;
         LPCITEMIDLIST pidlLast = NULL;

         if (SUCCEEDED(SHBindToParent(pidlDrivesRoot, IID_IShellFolder,
                                      (VOID **) &pFolder, &pidlLast)))
         {
            SHFILEINFO sfi = {0};
            WCHAR wszDisplayName[MAX_PATH];
            STRRET str = {0};

            ASSERT (ILIsNameSpacePidl(pidlLast));

            if (SUCCEEDED(pFolder->GetDisplayNameOf(pidlLast, SHGDN_NORMAL | SHGDN_FORPARSING, &str)) &&
                SUCCEEDED(StrRetToBuf(&str, pidlLast, wszDisplayName, lengthof(wszDisplayName))))
            {
               LPWSTR pwszDisplayName = wszDisplayName;
               UINT uFlags = SHGFI_ICON | SHGFI_SMALLICON;

               pwszDisplayName = ::wcsrchr(wszDisplayName, L'{');
               ASSERT(pwszDisplayName);
               uFlags |= SHGFI_USEFILEATTRIBUTES;

               if (SHGetFileInfo(pwszDisplayName, 0, &sfi, sizeof(SHFILEINFO), uFlags))
               {
                  s_hDRootIcon = sfi.hIcon;
               }
            }

            if (SUCCEEDED(pFolder->GetDisplayNameOf(pidlLast, SHGDN_NORMAL, &str)))
            {
               if (str.uType == STRRET_WSTR)
               {
                  s_pszDRootName = str.pOleStr;
               }
            }
            pFolder->Release();
            ILFree(pidlLast);
         }
         ILFree(pidlDrivesRoot);
      }
   }
   return ++s_FSRefCount;
}

DWORD CDefShellView::FSRelease()
{
   if (--s_FSRefCount == 0)
   {
      if (s_hDRootIcon)
      {
         ::DestroyIcon(s_hDRootIcon);
         s_hDRootIcon = NULL;
      }

      if (s_pszDRootName)
      {
         g_pShellMalloc->Free(s_pszDRootName);
         s_pszDRootName = NULL;
      }
   }
   return s_FSRefCount;

}

LRESULT CDefShellView::ApplyLabelEdit(NMLVDISPINFO * lpdi)
{
   ASSERT(IN_LABEL_EDIT & m_uState);
   LRESULT result = 0; // Reject the change

   m_uState |= IN_LABEL_APPLY;

   if (lpdi->item.pszText)
   {
      LPCITEMIDLIST pidl = m_pShellListView->GetItemPidl(lpdi->item.iItem);
      if (pidl)
      {
         HRESULT hr = m_pShellFolder->SetNameOf(m_hwnd, pidl, lpdi->item.pszText,
                                                 SHGDN_INFOLDER | SHGDN_FOREDITING,
                                                 NULL); 
         
         if (SUCCEEDED(hr))
         {
            s_CurrentUndoStr = IDS_MENU_EDIT_UNDOREN;
            result = 1; // Accept the change
         }
         else if (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER) 
         {
            WCHAR wszError[MAX_PATH];
            WCHAR wszDisplayName[MAX_PATH]; 

            if (SUCCEEDED(::StringCchPrintfEx(wszError,
                                  lengthof(wszError),
                                  NULL,
                                  NULL,
                                  STRSAFE_IGNORE_NULLS,
                                  L"%s%s",
                                  LOAD_STRING(IDS_SHFO_ERR_HDR_RENAME),
                                  LOAD_STRING(IDS_SHFO_ERR_INVALID))))
            {
               STRRET str = {0};
               if (SUCCEEDED(m_pShellFolder->GetDisplayNameOf(pidl, SHGDN_NORMAL, &str)) &&
                  SUCCEEDED(StrRetToBuf(&str, pidl, wszDisplayName, lengthof(wszDisplayName))))
               {
                  ShellDialogs::ShowFileError(m_hwnd,
                              MAKEINTRESOURCE(IDS_TITLE_RENAMEERROR),
                              wszError,
                              wszDisplayName, 
                              MB_ICONERROR | MB_OK);
               }
            }
         }
      }
   }

   m_uState &= ~(IN_LABEL_EDIT | IN_LABEL_APPLY);
   return result;
}

LRESULT CDefShellView::BeginLabelEdit(NMLVDISPINFO * lpdi)
{
   ASSERT(!(IN_LABEL_EDIT & m_uState));

   if (lpdi)
   {
      LPCITEMIDLIST pidl = m_pShellListView->GetItemPidl(lpdi->item.iItem);
      if (pidl)
      {
         ULONG ulAttrib = SFGAO_CANRENAME;
         if (SUCCEEDED(m_pShellFolder->GetAttributesOf(1, &pidl, &ulAttrib)))
         {
            if (SFGAO_CANRENAME & ulAttrib)
            {
               m_uState |= IN_LABEL_EDIT;
               return 0; // Allow edit
            }
         }
      }
   }

   return 1; // Disallow edit
}

LRESULT CDefShellView::Create(HWND hwnd, DWORD width, DWORD height)
{
   LRESULT lResult = -1; // failure

   ASSERT(hwnd);
   m_hwnd = hwnd;
   if (!m_hwnd)
   {
     return -1;
   }

   ::SetWindowLong(m_hwnd, GWL_USERDATA, (LONG) this);

   ASSERT(m_pShellListView);
   if (!m_pShellListView || !m_pShellListView->Create(m_hwnd, m_fFlags))
   {
      return -1; // failure
   }

   if (PERSIST_VIEWMODE & m_uState)
   {
      m_ViewMode = UserSettings::GetListviewStyle();
   }

   m_ViewMode = m_pShellListView->SetViewMode(m_ViewMode);
   m_UndoStr = s_CurrentUndoStr;

   // Get the proper RTL bits to pass on to our child windows
   m_Fmt = 0;

   // Be sure that the OS is supporting the flags DATE_LTRREADING and DATE_RTLREADING
   {
      // Get the date format reading order
      LANGID userLangID = GetUserDefaultUILanguage();

      if (LANG_ARABIC == PRIMARYLANGID(userLangID))
      {
         // Get the real list view windows ExStyle.
         if (::GetWindowLong(m_hwnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
         {
            m_Fmt = LVCFMT_RIGHT_TO_LEFT;
         }
         else
         {
            m_Fmt = LVCFMT_LEFT_TO_RIGHT;
         }
      }
   }

   LoadColumnHeaders();

   HandleSysColorChange();

   m_pShellListView->Resize(width, height);

   if (SUCCEEDED(Refresh()))
   {
      IDropTarget * pDropTarget = NULL;
      if (SUCCEEDED(QueryInterface(IID_IDropTarget, (VOID **) &pDropTarget)))
      {
         ASSERT(pDropTarget);
         if (SUCCEEDED(RegisterDragDrop(*m_pShellListView, pDropTarget)))
         {
            lResult = 0; // success
         }
         Release(); // RegisterDragDrop will AddRef get rid of the reference from the QI above
      }
   }

   if (-1 == lResult)
   {
      Destroy();
   }

   return lResult;
}

void CDefShellView::Destroy()
{
   if ((0 != m_uWatchID) && g_pFileChangeManager->RemoveWatch(m_uWatchID))
   {
      m_uWatchID = 0;
   }

   if (m_pShellListView)
   {
      RevokeDragDrop(*m_pShellListView);
      m_pShellListView->Destroy();
   }

   ::SetWindowLong(m_hwnd, GWL_USERDATA, (LONG) NULL);
   m_hwnd = NULL;
}

void CDefShellView::DoContextMenu(POINTL pt)
{
   ASSERT(m_pShellListView);
   if (!m_pShellListView)
   {
      return;
   }

   PopupMenuInfo popupMenuInfo = {0};
   HMENU hmenu = LoadPopupMenuTemplate();
   HMENU hmenuTrack = NULL;
   UINT uFlags = TPM_RETURNCMD;
   UINT uCmd = 0;

   if (!hmenu)
   {
      return;
   }

   popupMenuInfo.m_aPidls = m_pShellListView->GetSelected(&popupMenuInfo.m_uPidlCount);
   popupMenuInfo.m_pidlSelected = m_pShellListView->GetPidlAt(pt);
   if (popupMenuInfo.m_aPidls && popupMenuInfo.m_pidlSelected)
   {
      m_pShellFolder->GetUIObjectOf(m_hwnd, 1, &(popupMenuInfo.m_pidlSelected),
                                    IID_IContextMenu, NULL,
                                    (LPVOID *) &(popupMenuInfo.m_pContextMenu));
   }

   if (pt.x < (::GetSystemMetrics(SM_CXVIRTUALSCREEN) / 2))
   {
      uFlags |= TPM_LEFTALIGN;
   }
   else
   {
      uFlags |= TPM_RIGHTALIGN;
   }

   if (::GetWindowLong(m_hwnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
   {
      uFlags ^= TPM_RIGHTALIGN;
   }

   if (pt.y < (::GetSystemMetrics(SM_CYVIRTUALSCREEN) / 2))
   {
      uFlags |= TPM_TOPALIGN;
   }
   else
   {
      uFlags |= TPM_BOTTOMALIGN;
   }

   if (popupMenuInfo.m_aPidls && popupMenuInfo.m_pidlSelected)
   {
      popupMenuInfo.offset = ITEM_MENU_OFFSET;
   }
   else
   {
      popupMenuInfo.offset = FOLDER_MENU_OFFSET;
   }

   hmenuTrack = ::GetSubMenu(hmenu, popupMenuInfo.offset);
   if (hmenuTrack)
   {
      MENUITEMINFO mii = {0};
      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_DATA;
      mii.dwItemData = (DWORD) &popupMenuInfo;
      ::SetMenuItemInfo(hmenuTrack, 0, TRUE, &mii);
      uCmd = ::TrackPopupMenu(hmenuTrack, uFlags, pt.x, pt.y, 0, m_hwnd, NULL);

      if (0 != uCmd)
      {
         // If this is a user defined menu item pass the command back to the user
         if ((IDC_USER_MENU_ITEMS_BEGIN <= uCmd) &&
             (IDC_USER_MENU_ITEMS_END >= uCmd))
         {
            if (popupMenuInfo.m_pContextMenu)
            {
               CMINVOKECOMMANDINFO ici = {0};
               ici.cbSize = sizeof(ici);
               ici.hwnd = m_hwnd;
               ici.lpVerb = (LPCSTR) MAKEINTRESOURCE(uCmd);
               popupMenuInfo.m_pContextMenu->InvokeCommand(&ici);
            }
         }
         else
         {
            ::SendMessage(m_hwnd, WM_COMMAND, (WPARAM) uCmd, NULL);
         }
      }
   }

   if (popupMenuInfo.m_pContextMenu)
   {
      popupMenuInfo.m_pContextMenu->Release();
   }

   if (popupMenuInfo.m_aPidls)
   {
      g_pShellMalloc->Free(popupMenuInfo.m_aPidls);
   }

   ::DestroyMenu(hmenu);
}

void CDefShellView::DoDragDrop(NMLISTVIEW * pnmListView)
{
   HRESULT hr = S_OK;
   IDataObject * pDataObject = NULL;
   UINT uItemCount = 0;
   LPCITEMIDLIST * aPidls = NULL;

   aPidls = m_pShellListView->GetSelected(&uItemCount);
   if (aPidls)
   {
      hr = m_pShellFolder->GetUIObjectOf(*m_pShellListView,
                                         uItemCount,
                                         aPidls,
                                         IID_IDataObject,
                                         NULL,
                                         (LPVOID *) &pDataObject);

      if (SUCCEEDED(hr) && pDataObject)
      {
         IDropSource * pDropSource = NULL;
         if (SUCCEEDED(QueryInterface(IID_IDropSource, (VOID **) &pDropSource)))
         {
            DWORD dwOKEffect = DROPEFFECT_NONE | DROPEFFECT_COPY |
                               DROPEFFECT_MOVE | DROPEFFECT_LINK;
            DWORD dwEffect = 0;

            m_pShellListView->BeginDrag(pnmListView);
            ::DoDragDrop(pDataObject, pDropSource, dwOKEffect, &dwEffect);
            m_pShellListView->EndDrag();

            pDropSource->Release();
         }

         pDataObject->Release();
      }

      g_pShellMalloc->Free(aPidls);
   }
}

void CDefShellView::DoNewFolder()
{
   WCHAR wszPath[MAX_PATH];
   LPWSTR pwszFolder = LOAD_STRING(IDS_NEWFOLDER);
   UINT uMessage = 0;

   if (SHGetPathFromIDList(m_pidlFQ, wszPath) && pwszFolder)
   {
      HRESULT hr = NOERROR;
      WCHAR wszPathUnique[MAX_PATH];
      size_t cch;

      hr = ::StringCchLength(wszPath, lengthof(wszPath), &cch);
      if (FAILED(hr) || (0 == cch))
      {
         goto ErrorBuffer;
      }

      if (L'\\' != wszPath[cch-1])
      {
         hr = ::StringCchCat(wszPath, lengthof(wszPath), L"\\");
         if (FAILED(hr))
         {
            goto ErrorBuffer;
         }
      }

      hr = ::StringCchCat(wszPath, lengthof(wszPath), pwszFolder);
      if (FAILED(hr))
      {
         goto ErrorBuffer;
      }

      if (PathMakeUniqueName(wszPath, NULL, NULL, FALSE, wszPathUnique, lengthof(wszPathUnique)))
      {
         LPITEMIDLIST pidl = NULL;

         pwszFolder = PathFindFileName(wszPathUnique);
         if (pwszFolder &&  SUCCEEDED(CreateFileSystemPidl(pwszFolder, &pidl)))
         {
            m_pShellListView->MakeEditableOnAdd(pidl);
         }

         if (!::CreateDirectory(wszPathUnique, NULL))
         {
            switch (GetLastError())
            {
               case ERROR_FILE_NOT_FOUND:
               case ERROR_ACCESS_DENIED:
                  uMessage = IDS_NEWFOLDER_ERR_ACCESS;
               break;

               case ERROR_HANDLE_DISK_FULL:
               case ERROR_DISK_FULL:
                  uMessage = IDS_NEWFOLDER_ERR_DISKFULL;
               break;

               default:
                  uMessage = IDS_NEWFOLDER_ERR;
            }
         }

         if (pidl)
         {
            ILFree(pidl);
         }
      }
      else
      {
ErrorBuffer:
         uMessage = IDS_NEWFOLDER_ERR_BUFFER;
      }

      if (0 != uMessage)
      {
         ShellDialogs::ShowFileError(*m_pShellListView,
                                     MAKEINTRESOURCE(IDS_NEWFOLDER),
                                     MAKEINTRESOURCE(uMessage),
                                     wszPath,
                                     MB_ICONERROR | MB_OK);
      }
   }
   else
   {
      ::MessageBeep((UINT)-1);
   }
}

void CDefShellView::DoProperties()
{
   SHELLEXECUTEINFO shinfo;
   UINT uItemCount = 0;
   LPCITEMIDLIST * aPidls = m_pShellListView->GetSelected(&uItemCount);

   shinfo.cbSize = sizeof(shinfo);
   shinfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
   shinfo.lpVerb = NULL;
   shinfo.lpDirectory = NULL;
   shinfo.nShow = SW_SHOW;
   shinfo.hInstApp = NULL;
   shinfo.lpFile = TEXT("ctlpnl.exe");

   if (aPidls)
   {
      if (1 < uItemCount)
      {
         ShellDialogs::Properties::ObjectProperties(m_pidlFQ, m_pShellFolder, this, aPidls, uItemCount);
      }
      else
      {
         if (1 == uItemCount)
         {
            if (ILIsNameSpace(*aPidls, CLSID_CEShellBitBucket))
            {
               ShellDialogs::Properties::RecycleBinProperties();
            }
            else
            {
               if (ILIsNameSpace(*aPidls, CLSID_CEShellDrives))
               {
                  //My Computer: Launch "System Properties" CPL applet.
                  shinfo.lpParameters = TEXT("cplmain.cpl,6");
                  ShellExecuteEx( &shinfo );
               }
               else
               {
                  ShellDialogs::Properties::ObjectProperties(m_pidlFQ, m_pShellFolder, this, aPidls, uItemCount);
               }
            }
         }
         else
         {
            ASSERT(FALSE);
         }
      }
      g_pShellMalloc->Free(aPidls);
   }
   else
   {  //No objects selected
      if (ILIsNameSpace(m_pidlFQ, CLSID_CEShellBitBucket))
      {
         ShellDialogs::Properties::RecycleBinProperties();
      }
      else
      {
         IShellFolder * pParentFolder = NULL;
         LPCITEMIDLIST pidlLast = NULL;
         STRRET str = {0};

         if (SUCCEEDED(SHBindToParent(m_pidlFQ, IID_IShellFolder,
                                   (VOID **) &pParentFolder, &pidlLast)))
         {
            if (ILIsNameSpace(pidlLast, CLSID_CEShellDrives))
            {
               //My Computer: Launch "System Properties" CPL applet.
               shinfo.lpParameters = TEXT("cplmain.cpl,6");
               ShellExecuteEx( &shinfo );
            }
            else
            {
               TCHAR szTmp[MAX_PATH], szDesktop[MAX_PATH];

               pParentFolder->GetDisplayNameOf(pidlLast, SHGDN_NORMAL | SHGDN_FORPARSING, &str);
               StrRetToBuf(&str, pidlLast, szTmp, lengthof(szTmp));

               // Get a ShellFolder representing the filesystem path to the desktop folder
               if (SHGetSpecialFolderPath(NULL, szDesktop, CSIDL_DESKTOPDIRECTORY, TRUE))
               {
                  if (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                                  szDesktop, -1, szTmp, -1))
                  {
                     //Desktop: Launch "Display Properties" CPL applet
                     shinfo.lpParameters = TEXT("cplmain.cpl,7");
                     ShellExecuteEx( &shinfo );
                  }
                  else
                  {
                     //Under other folder object
                     ShellDialogs::Properties::FolderObjectProperties(m_pidlFQ, m_pShellFolder);
                  }
               }
            }
         }

         if (pidlLast)
         {
            ILFree(pidlLast);
         }

         if (pParentFolder)
         {
            pParentFolder->Release();
         }
      }
   }
}

void CDefShellView::GetDisplayInfo(NMLVDISPINFO * lpdi)
{
   if (!lpdi)
   {
      return;
   }

   LPITEMIDLIST pidl = (LPITEMIDLIST) lpdi->item.lParam;

   if (0 == lpdi->item.iSubItem)
   {
      // Get The icon information
      if (LVIF_IMAGE & lpdi->item.mask)
      {
         SHFILEINFO sfi = {0};
         WCHAR wszDisplayName[MAX_PATH];
         STRRET str = {0};

         // Tell the listview to store the icon data
         lpdi->item.mask |= LVIF_DI_SETITEM;

         if (SUCCEEDED(m_pShellFolder->GetDisplayNameOf(pidl, SHGDN_NORMAL | SHGDN_FORPARSING, &str)) &&
             SUCCEEDED(StrRetToBuf(&str, pidl, wszDisplayName, lengthof(wszDisplayName))))
         {
            LPWSTR pwszDisplayName = wszDisplayName;
            UINT uFlags = SHGFI_SYSICONINDEX | SHGFI_LARGEICON;

            if (ILIsNameSpacePidl(pidl))
            {
               // Walk over the :: that is prepended to the last
               // guid i.e. ::{12345678-1234-1234-1234-123456789ABC}
               pwszDisplayName = ::wcsrchr(wszDisplayName, L'{');
               ASSERT(pwszDisplayName);
               uFlags |= SHGFI_USEFILEATTRIBUTES;
            }
            else
            {
               // To determine if it is a link
               uFlags |= SHGFI_ATTRIBUTES;
            }

            if (SHGetFileInfo(pwszDisplayName, 0, &sfi, sizeof(SHFILEINFO), uFlags))
            {
               if (ILIsNameSpace(pidl, CLSID_CEShellBitBucket))
               {
                  g_pRecBin->BeginRecycle();

                  if (!g_pRecBin->IsEmpty())
                  {
                     sfi.iIcon++;
                  }

                  g_pRecBin->EndRecycle();

                  lpdi->item.mask &= ~LVIF_DI_SETITEM;
               }
            }

            if (SFGAO_LINK & sfi.dwAttributes)
            {
               lpdi->item.mask |= LVIF_STATE;
               lpdi->item.stateMask = LVIS_OVERLAYMASK;
               lpdi->item.state = INDEXTOOVERLAYMASK(1);
            }
         }

         lpdi->item.iImage = sfi.iIcon; // 0 on error which is the default icon
      }
   }

   // Bail if they aren't asking for text
   if (!(LVIF_TEXT & lpdi->item.mask))
   {
      return;
   }

   // Set to an empty string on failure
   if (lpdi->item.cchTextMax)
   {
      *lpdi->item.pszText = L'\0';
   }

   // Query the details for the name
   IShellDetails * pShellDetails = NULL;
   if (SUCCEEDED(m_pShellFolder->QueryInterface(IID_IShellDetails,
                                                (VOID**) &pShellDetails)))
   {
      SHELLDETAILS sd = {0};
      sd.fmt = m_Fmt;
      if (SUCCEEDED(pShellDetails->GetDetailsOf(pidl, lpdi->item.iSubItem, &sd)))
      {
         StrRetToBuf(&sd.str, pidl, lpdi->item.pszText, lpdi->item.cchTextMax);
      }
      pShellDetails->Release();
   }
}

HRESULT CDefShellView::HandleChangeNotify(ChangeNotifyEventData * pChangeNotifyEventData)
{
   LPITEMIDLIST pidl1Updated = NULL;
   LPITEMIDLIST pidl2Updated = NULL;

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

   if (pChangeNotifyEventData->pidl1)
   {
      pidl1Updated = ILCopy(pChangeNotifyEventData->pidl1, IL_ALL);
      UpdateFileSystemPidl(m_pidlFQ, &pidl1Updated);
   }

   if (pChangeNotifyEventData->pidl2)
   {
      pidl2Updated = ILCopy(pChangeNotifyEventData->pidl2, IL_ALL);
      UpdateFileSystemPidl(m_pidlFQ, &pidl2Updated);
   }

   switch (pChangeNotifyEventData->lEvent)
   {
      case SHCNE_ASSOCCHANGED:
         Refresh();
      break;

      case SHCNE_ATTRIBUTES:
         if (pidl1Updated)
         {
            HRESULT hr = S_OK;
            LPCITEMIDLIST pidlOld = NULL;

            pidlOld = m_pShellListView->GetItemPidl(pidl1Updated);
            if (pidlOld)
            {
               pidlOld = ILCopy(pidlOld, IL_ALL);
               if (pidlOld)
               {
                  if (m_pShellListView->UpdateItemPidl(pidlOld, pidl1Updated))
                  {
                     // Refresh the size if it changed
                     hr = m_pShellFolder->CompareIDs(1, pidlOld, pidl1Updated);
                     if (SUCCEEDED(hr) && (0 != HRESULT_CODE(hr)))
                     {
                        m_pShellListView->InvalidateItemColumn(pidl1Updated, 1);
                     }

                     ::UpdateWindow(*m_pShellListView);
                  }

                  ILFree(pidlOld);
               }
            }
         }
      break;

      case SHCNE_CREATE:
         if (pidl1Updated)
         {
            WCHAR wszPath[MAX_PATH];
            LPITEMIDLIST pidlCat = ILConcatenate(m_pidlFQ, pidl1Updated);

            if (pidlCat)
            {
               BOOL fAddPidl = TRUE;

               if (SHGetPathFromIDList(pidlCat, wszPath))
               {
                  DWORD dwAttrib = GetFileAttributes(wszPath);

                  if (-1 != dwAttrib)
                  {
                     if (!UserSettings::GetShowSystemFiles() &&
                         ((dwAttrib & FILE_ATTRIBUTE_SYSTEM) ||
                          (dwAttrib & FILE_ATTRIBUTE_ROMMODULE)))
                     {
                        fAddPidl = FALSE;
                     }

                     if (!UserSettings::GetShowHiddenFiles() &&
                         (dwAttrib & FILE_ATTRIBUTE_HIDDEN))
                     {
                        fAddPidl = FALSE;
                     }
                  }
               }

               if (fAddPidl)
               {
                  m_pShellListView->ReflectAdd(pidl1Updated);
               }

               ILFree(pidlCat);
            }
         }
      break;

      case SHCNE_DELETE:
         if (pidl1Updated)
         {
            m_pShellListView->ReflectRemove(pidl1Updated);
         }
      break;

      case SHCNE_RENAMEITEM:
         if (pidl1Updated && pidl2Updated)
         {
            HRESULT hr = S_OK;
            LPCITEMIDLIST pidlOld = NULL;

            pidlOld = m_pShellListView->GetItemPidl(pidl1Updated);
            if (pidlOld)
            {
               pidlOld = ILCopy(pidlOld, IL_ALL);
               if (pidlOld)
               {
                  if (m_pShellListView->UpdateItemPidl(pidl1Updated, pidl2Updated))
                  {
                     // Refresh the name and icon
                     m_pShellListView->InvalidateItemColumn(pidl2Updated, 0);

                     // Refresh the type if it changed
                     hr = m_pShellFolder->CompareIDs(2, pidlOld, pidl2Updated);
                     if (SUCCEEDED(hr) && (0 != HRESULT_CODE(hr)))
                     {
                        m_pShellListView->InvalidateItemColumn(pidl2Updated, 2);
                     }

                     ::UpdateWindow(*m_pShellListView);
                  }

                  ILFree(pidlOld);
               }
            }
         }
      break;

      case SHCNE_RMDIR: // Handle special case where we need to close our own window
         if (m_pShellBrowser)
         {
            HRESULT hr;
            IOleCommandTarget * pTarget;

            m_pShellBrowser->AddRef(); // AddRef since it's possible to kill the view on another thread
            hr = m_pShellBrowser->QueryInterface(IID_IOleCommandTarget, (VOID **) &pTarget);
            m_pShellBrowser->Release();

            if (SUCCEEDED(hr))
            {
               pTarget->Exec(NULL, OLECMDID_CLOSE, 0, NULL, NULL);
               pTarget->Release();
            }
         }
      break;

      case SHCNE_UPDATEITEM:
         if (pidl1Updated)
         {
            HRESULT hr = S_OK;
            LPCITEMIDLIST pidlOld = NULL;

            pidlOld = m_pShellListView->GetItemPidl(pidl1Updated);
            if (pidlOld)
            {
               pidlOld = ILCopy(pidlOld, IL_ALL);
               if (pidlOld)
               {
                  if (m_pShellListView->UpdateItemPidl(pidlOld, pidl1Updated))
                  {
                     // Refresh the name and icon
                     m_pShellListView->InvalidateItemColumn(pidl1Updated, 0);

                     // Refresh the size if it changed
                     hr = m_pShellFolder->CompareIDs(1, pidlOld, pidl1Updated);
                     if (SUCCEEDED(hr) && (0 != HRESULT_CODE(hr)))
                     {
                        m_pShellListView->InvalidateItemColumn(pidl1Updated, 1);
                     }

                     // Refresh the date stamp if it changed
                     hr = m_pShellFolder->CompareIDs(3, pidlOld, pidl1Updated);
                     if (SUCCEEDED(hr) && (0 != HRESULT_CODE(hr)))
                     {
                        m_pShellListView->InvalidateItemColumn(pidl1Updated, 3);
                     }

                     ::UpdateWindow(*m_pShellListView);
                  }

                  ILFree(pidlOld);
               }
            }
         }
      break;

      default:
         Refresh();
   }

   // The View is being destroyed for SHCNE_RMDIR event,
   // so do not call UpdateStatusBar
   if (pChangeNotifyEventData->lEvent != SHCNE_RMDIR)
   {
      UpdateStatusBar(NULL, 0, FALSE);
   }

   if (pidl1Updated)
   {
      ILFree(pidl1Updated);
   }

   if (pidl2Updated)
   {
      ILFree(pidl2Updated);
   }

   // Clean up the ChangeNotifyEventData object
   delete pChangeNotifyEventData;

   return S_OK;
}

LRESULT CDefShellView::HandleCommand(DWORD dwCmd)
{
   LRESULT lRet = 0;

   switch (dwCmd)
   {
      case IDC_EDIT_COPY:
         if (!OnDefviewEditCommand())
         {
            PerformAction(OLECMDID_COPY);
         }
      break;

      case IDC_EDIT_CUT:
         if (!OnDefviewEditCommand())
         {       
            PerformAction(OLECMDID_CUT);
         }
      break;

      case IDC_EDIT_PASTE:
         if (!OnDefviewEditCommand())
         {
            PerformAction(OLECMDID_PASTE);
         }
      break;

      case IDC_EDIT_PASTESHORTCUT:
         if (!OnDefviewEditCommand())
         {
            CShellClipboard::Paste(*m_pShellListView, m_pidlFQ, TRUE);
            s_CurrentUndoStr = IDS_MENU_EDIT_UNDO;
         }
      break;

      case IDC_EDIT_SELECTALL:
         ::SetFocus(*m_pShellListView);
         m_pShellListView->SelectAll();
      break;

      case IDC_EDIT_UNDO:
         PerformAction(OLECMDID_UNDO);
      break;

      case IDC_FILE_DELETE:
         if (!OnDefviewEditCommand())
         {
            ShellDataTransfer sdt;
            sdt.m_ppidlItems = m_pShellListView->GetSelected(&(sdt.m_uItems));
            if (sdt.m_ppidlItems)
            {
               ULONG uFlags = SFGAO_CANDELETE;
               if (SUCCEEDED(m_pShellFolder->GetAttributesOf(sdt.m_uItems,
                                                             sdt.m_ppidlItems,
                                                             &uFlags)))
               {
                  if (SFGAO_CANDELETE & uFlags)
                  {
                     sdt.m_hwndOwnerLV = *m_pShellListView;
                     sdt.m_pidlFolder = m_pidlFQ;
                     sdt.TransferTo(ShellDataTransfer::DELE, NULL);
                     s_CurrentUndoStr = IDS_MENU_EDIT_UNDODEL;
                  }
               }
               g_pShellMalloc->Free(sdt.m_ppidlItems);
            }
         }
      break;

      case IDC_FILE_NEWFOLDER:
         ::SetFocus(*m_pShellListView);
         DoNewFolder();
      break;

      case IDC_FILE_OPEN:
         PerformAction(OLECMDID_OPEN);
      break;

      case IDC_FILE_PROPERTIES:
         if (!OnDefviewEditCommand())
         {
            DoProperties();
         }
      break;

      case IDC_FILE_RENAME:
         m_pShellListView->RenameSelected();
      break;

      case IDC_FILE_SENDTO_DESKTOP:
      {
         TCHAR szDesktopDirectory[MAX_PATH];
         if (SHGetSpecialFolderPath(*m_pShellListView, szDesktopDirectory,
                                    CSIDL_DESKTOPDIRECTORY, TRUE))
         {
            ShellDataTransfer sdt;
            sdt.m_ppidlItems = m_pShellListView->GetSelected(&(sdt.m_uItems));
            if (sdt.m_ppidlItems)
            {
               sdt.m_hwndOwnerLV = *m_pShellListView;
               sdt.m_pidlFolder = m_pidlFQ;
               sdt.TransferTo(ShellDataTransfer::LINK, szDesktopDirectory);
               g_pShellMalloc->Free(sdt.m_ppidlItems);
            }
         }
      }
      break;

      case IDC_FILE_SENDTO_MYDOCUMENTS:
      {
         TCHAR szMyDocuments[MAX_PATH];
         if (SHGetSpecialFolderPath(*m_pShellListView, szMyDocuments,
                                    CSIDL_PERSONAL, TRUE))
         {
            ShellDataTransfer sdt;
            sdt.m_ppidlItems = m_pShellListView->GetSelected(&(sdt.m_uItems));
            if (sdt.m_ppidlItems)
            {
               sdt.m_hwndOwnerLV = *m_pShellListView;
               sdt.m_pidlFolder = m_pidlFQ;
               sdt.TransferTo(ShellDataTransfer::COPY, szMyDocuments);
               g_pShellMalloc->Free(sdt.m_ppidlItems);
            }
         }
      }
      break;

      case IDC_HELP_TOPICS:
         CreateProcess(L"\\windows\\peghelp.exe",
                                    L"wince.htm#Windows_Explorer_Help",
                                    0,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0);
         lRet = TRUE;
      break;

      case IDC_REFRESH:
         PerformAction(OLECMDID_REFRESH);
      break;

      case IDC_ARRANGE_AUTO:
      {
         BOOL fIsAutoArrangeEnabled = m_pShellListView->IsAutoArrangeEnabled();
         m_pShellListView->ToggleAutoArrange(!fIsAutoArrangeEnabled);
      }
      break;

      case IDC_ARRANGE_BYDATE:
         // 3 is date (see msdn docs)
         if (3 == m_ArrangeBy)
         {
            m_uState ^= SORT_DECENDING;
         }
         else
         {
            m_ArrangeBy = 3;
            m_uState &= ~SORT_DECENDING;
         }

         if (m_pShellListView)
         {
            m_pShellListView->Sort(CompareItems, (LPARAM) this);
         }
      break;

      case IDC_ARRANGE_BYNAME:
         // 0 is name (see msdn docs)
         if (0 == m_ArrangeBy)
         {
            m_uState ^= SORT_DECENDING;
         }
         else
         {
            m_ArrangeBy = 0;
            m_uState &= ~SORT_DECENDING;
         }

         if (m_pShellListView)
         {
            m_pShellListView->Sort(CompareItems, (LPARAM) this);
         }
      break;

      case IDC_ARRANGE_BYSIZE:
         // 1 is size (see msdn docs)
         if (1 == m_ArrangeBy)
         {
            m_uState ^= SORT_DECENDING;
         }
         else
         {
            m_ArrangeBy = 1;
            m_uState &= ~SORT_DECENDING;
         }

         if (m_pShellListView)
         {
            m_pShellListView->Sort(CompareItems, (LPARAM) this);
         }
      break;

      case IDC_ARRANGE_BYTYPE:
         // 2 is type (see msdn docs)
         if (2 == m_ArrangeBy)
         {
            m_uState ^= SORT_DECENDING;
         }
         else
         {
            m_ArrangeBy = 2;
            m_uState &= ~SORT_DECENDING;
         }

         if (m_pShellListView)
         {
            m_pShellListView->Sort(CompareItems, (LPARAM) this);
         }
      break;

      case IDC_VIEW_DETAILS:
         if (FVM_DETAILS != m_ViewMode)
         {
            m_ViewMode = m_pShellListView->SetViewMode(FVM_DETAILS);
            if (PERSIST_VIEWMODE & m_uState)
            {
               UserSettings::SetListviewStyle(m_ViewMode);
            }
         }
      break;

      case IDC_VIEW_LIST:
         if (FVM_LIST != m_ViewMode)
         {
            m_ViewMode = m_pShellListView->SetViewMode(FVM_LIST);
            if (PERSIST_VIEWMODE & m_uState)
            {
               UserSettings::SetListviewStyle(m_ViewMode);
            }
         }
      break;

      case IDC_VIEW_ICONS:
         if (FVM_ICON != m_ViewMode)
         {
            m_ViewMode = m_pShellListView->SetViewMode(FVM_ICON);
            if (PERSIST_VIEWMODE & m_uState)
               UserSettings::SetListviewStyle(m_ViewMode);
         }
      break;

      case IDC_VIEW_SMALL:
         if (FVM_SMALLICON != m_ViewMode)
         {
            m_ViewMode = m_pShellListView->SetViewMode(FVM_SMALLICON);
            if (PERSIST_VIEWMODE & m_uState)
            {
               UserSettings::SetListviewStyle(m_ViewMode);
            }
         }
      break;

      case IDC_VIEW_OPTIONS:
         UserSettings::ShowFolderOptions();
      break;

      case IDC_VIEW_TYPE:
      {
         HWND hwndToolbar;

         // Get toolbar handle
         if (m_pTopShellBrowser && 
            (m_pTopShellBrowser->GetControlWindow(FCW_TOOLBAR, &hwndToolbar) == S_OK))
         {
             HMENU hmenuTemp, hmenuTrack;
             RECT rc;

             int iIndex = ::SendMessage(hwndToolbar, TB_COMMANDTOINDEX, IDC_VIEW_TYPE, 0);
             ::SendMessage(hwndToolbar, TB_GETITEMRECT, (WPARAM)iIndex, (LPARAM)&rc);
             ::MapWindowPoints(hwndToolbar, NULL, (LPPOINT)&rc, 2);

             hmenuTemp = ::LoadMenu(HINST_CESHELL, MAKEINTRESOURCE(IDM_DEF_POPUP_MENUS));
             hmenuTrack = ::GetSubMenu(hmenuTemp, FOLDER_MENU_OFFSET);
             hmenuTrack = ::GetSubMenu(hmenuTrack, FOLDER_VIEW_MENU_OFFSET);

             ::TrackPopupMenu(hmenuTrack,
                              TPM_LEFTALIGN | TPM_TOPALIGN,
                              (::GetWindowLong(::GetParent(m_hwnd), GWL_EXSTYLE) & WS_EX_LAYOUTRTL) ?
                                               rc.right :
                                               rc.left,
                              rc.bottom,
                              0,
                              m_hwnd,
                              NULL);
             ::DestroyMenu(hmenuTemp);
         }
      }
      break;

      case IDC_GO_MYDOCUMENTS:
      {
         LPITEMIDLIST pidlMyDocuments = NULL;
         if (m_pShellBrowser &&
             SUCCEEDED(SHGetSpecialFolderLocation(*m_pShellListView,
                                                  CSIDL_PERSONAL,
                                                  &pidlMyDocuments)))
         {
            m_pShellBrowser->BrowseObject(pidlMyDocuments,
                                          SBSP_DEFBROWSER | SBSP_DEFMODE | SBSP_ABSOLUTE);
            ILFree(pidlMyDocuments);
         }
      }
      break;

      case IDC_GO_FOLDERUP:
         if (m_pShellBrowser)
         {
            m_pShellBrowser->BrowseObject(NULL, SBSP_DEFBROWSER | SBSP_DEFMODE | SBSP_PARENT);
         }
      break;

      case IDC_DISMISS_EDIT:
      case ID_ESCAPE:
         if (m_pShellListView)
         {
            m_pShellListView->HandleEscape();
         }
      break;

      case ID_CONTEXTMENU:
      {
         LPARAM point = m_pShellListView->GetContextMenuPoint();
         POINTL pt = { GET_X_LPARAM(point), GET_Y_LPARAM(point) };
         DoContextMenu(pt);
      }
      break;

      default:
         if ((IDC_USER_MENU_ITEMS_BEGIN <= dwCmd) &&
             (IDC_USER_MENU_ITEMS_END >= dwCmd))
         {
            MENUITEMINFO mii = {0};
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_DATA;
            if (::GetMenuItemInfo(m_hMenu, 0, TRUE, &mii) && mii.dwItemData)
            {
               CMINVOKECOMMANDINFO ici = {0};
               ici.cbSize = sizeof(ici);
               ici.hwnd = m_hwnd;
               ici.lpVerb = (LPCSTR) MAKEINTRESOURCE(dwCmd);
               ((IContextMenu*)mii.dwItemData)->InvokeCommand(&ici);
            }
            break;
         }
         lRet = 1;
   }

   return lRet;
}

LRESULT CDefShellView::HandleInitMenuPopup(HMENU hmenu, PopupMenuInfo * pPopupMenuInfo)
{
   LRESULT result = 0; // Handled

   if (!pPopupMenuInfo)
   {
      // Do the main menu (or view and arrange-by menu)
      OLECMD oleCmd;
      MENUITEMINFO mii;
      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_ID;

      UpdateUndoMenuItem(hmenu);

      int nPos = 0;
      while(GetMenuItemInfo(hmenu, nPos, TRUE, &mii))
      {
         oleCmd.cmdID= mii.wID;
         QueryStatus(&CGID_CEShell, 1, &oleCmd, NULL);

         if (oleCmd.cmdf & OLECMDF_ENABLED)
         {
            UINT uCheck = MF_BYCOMMAND;
            if (oleCmd.cmdf & OLECMDF_LATCHED)
            {
               uCheck |= MF_CHECKED;
            }
            else
            {
               uCheck |= MF_UNCHECKED;
            }
            ::EnableMenuItem(hmenu, oleCmd.cmdID, MF_BYCOMMAND | MF_ENABLED);
            ::CheckMenuItem(hmenu, oleCmd.cmdID, uCheck);
         }
         else if (oleCmd.cmdf & OLECMDF_SUPPORTED)
         {
            ::EnableMenuItem(hmenu, oleCmd.cmdID, MF_BYCOMMAND | MF_GRAYED);
         }
         nPos++;
      }
   }
   else
   {
      //Do the context menu
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
            UINT mfPaste = MF_BYCOMMAND;
            UINT mfPasteShortcut = MF_BYCOMMAND;
            
            if (CShellClipboard::HasData())
            {
               mfPaste |= MF_ENABLED;
            }
            else
            {
                mfPaste |= MF_GRAYED;
            }

            if (ShellDataTransfer::CONTAINS_COPY_DATA &
               CShellClipboard::GetContentType())
            {
               mfPasteShortcut |= MF_ENABLED;
            }
            else
            {
               mfPasteShortcut |= MF_GRAYED;
            }

            ::EnableMenuItem(hmenu, IDC_EDIT_PASTE, mfPaste);
            ::EnableMenuItem(hmenu, IDC_EDIT_PASTESHORTCUT, mfPasteShortcut);
         }
         break;

         case ITEM_MENU_OFFSET:
         {
            ASSERT(pPopupMenuInfo->m_aPidls);

            ULONG uFlags = (SFGAO_CANMOVE | SFGAO_CANCOPY | SFGAO_CANDELETE | SFGAO_CANRENAME);
            UINT mfOpen = MF_BYCOMMAND;
            UINT mfCut = MF_BYCOMMAND;
            UINT mfCopy = MF_BYCOMMAND;
            UINT mfDelete = MF_BYCOMMAND;
            UINT mfRename = MF_BYCOMMAND;

            if (SUCCEEDED(m_pShellFolder->GetAttributesOf(pPopupMenuInfo->m_uPidlCount,
                                                          pPopupMenuInfo->m_aPidls,
                                                          &uFlags)))
            {
               mfOpen |= MF_ENABLED;
               mfCut |= ((SFGAO_CANMOVE & uFlags) ? MF_ENABLED : MF_GRAYED);
               mfCopy |= ((SFGAO_CANCOPY & uFlags) ? MF_ENABLED : MF_GRAYED);
               mfDelete |= ((SFGAO_CANDELETE & uFlags) ? MF_ENABLED : MF_GRAYED);
               if (1 < pPopupMenuInfo->m_uPidlCount)
               {
                  mfRename |= MF_GRAYED;
               }
               else
               {
                  mfRename |= ((SFGAO_CANRENAME & uFlags) ? MF_ENABLED : MF_GRAYED);
               }
            }
            else
            {
               mfOpen |= MF_GRAYED;
               mfCut |= MF_GRAYED;
               mfCopy |= MF_GRAYED;
               mfDelete |= MF_GRAYED;
               mfRename |= MF_GRAYED;
            }

            ::EnableMenuItem(hmenu, IDC_FILE_OPEN, mfOpen);
            ::EnableMenuItem(hmenu, IDC_EDIT_CUT, mfCut);
            ::EnableMenuItem(hmenu, IDC_EDIT_COPY, mfCopy);
            ::EnableMenuItem(hmenu, IDC_FILE_DELETE, mfDelete);
            ::EnableMenuItem(hmenu, IDC_FILE_RENAME, mfRename);
         }
         break;

         default:
            result = 1; // Not handled
      }
   }

   return result;
}

LRESULT CDefShellView::HandleNotify(LPNMHDR lpnmh)
{
   ASSERT(lpnmh);
   if (!lpnmh)
   {
      return 0;
   }

   switch(lpnmh->code)
   {
      case HDN_ITEMCHANGED:
      case HDN_ENDTRACK:
      {
         NMHEADER * lpnmhdr = (NMHEADER *) lpnmh;
         if ((4 > lpnmhdr->iItem) && (HDI_WIDTH & lpnmhdr->pitem->mask))
         {
            s_ColumnWidths[lpnmhdr->iItem] = (short)lpnmhdr->pitem->cxy;
         }
      }
      break;

      case LVN_BEGINDRAG:
         DoDragDrop((NMLISTVIEW *) lpnmh);
      break;

      case LVN_BEGINLABELEDIT:
         return BeginLabelEdit((NMLVDISPINFO *) lpnmh);
      break;

      case LVN_COLUMNCLICK:
         switch (((NMLISTVIEW *) lpnmh)->iSubItem)
         {
            case 0: HandleCommand(IDC_ARRANGE_BYNAME); break; // Name
            case 1: HandleCommand(IDC_ARRANGE_BYSIZE); break; // Size
            case 2: HandleCommand(IDC_ARRANGE_BYTYPE); break; // Type
            case 3: HandleCommand(IDC_ARRANGE_BYDATE); break; // Date
         }
      break;

      case LVN_DELETEITEM:
         ILFree(m_pShellListView->GetItemPidl(((NMLISTVIEW *) lpnmh)->iItem));
      break;

      case LVN_ENDLABELEDIT:
         return ApplyLabelEdit((NMLVDISPINFO *) lpnmh);
      break;

      case LVN_GETDISPINFO:
         GetDisplayInfo((NMLVDISPINFO *) lpnmh);
      break;

      case LVN_KEYDOWN:
      {
         switch (((LV_KEYDOWN*)lpnmh)->wVKey)
         {
            case VK_ESCAPE:
               CShellClipboard::HandleEscape(*m_pShellListView);
               s_CurrentUndoStr = IDS_MENU_EDIT_UNDO;
            break;
         }
      }
      break;

      case LVN_ITEMCHANGED:
      {
         LPNMLISTVIEW lpnmLV = (NMLISTVIEW *)lpnmh;
         if (m_pTopShellBrowser &&
            (lpnmLV->uChanged & LVIF_STATE) &&
            ((lpnmLV->uNewState & LVIS_SELECTED) != (lpnmLV->uOldState & LVIS_SELECTED)))
         {
			 OnSelectionChanged();
         }
      }
      break;

      case NM_DBLCLK:
      case NM_RETURN:
         PerformAction(OLECMDID_OPEN);
      break;

      case NM_CLICK:
      case NM_RCLICK:
      case NM_RDBLCLK:
      {
         // Check for Alt-Tap
         if ((NM_CLICK == lpnmh->code) && !GetAsyncKeyState(VK_MENU))
         {
            break;
         }
      }
      // fall through...

      case GN_CONTEXTMENU:
      {
         DWORD dwPoint = ::GetMessagePos();
         POINTL pt = { GET_X_LPARAM(dwPoint), GET_Y_LPARAM(dwPoint) };
         DoContextMenu(pt);
      }
      break;

      case NM_SETFOCUS:
         UpdateStatusBar(NULL, 0, FALSE);
      break;
   }

   return 0;
}

LRESULT CDefShellView::HandleSettingChange(DWORD flag, LPCTSTR pszSectionName)
{
   LRESULT result = 1;

   // Ignore the setting change
   return result;
}

void CDefShellView::HandleSysColorChange()
{
   if (!m_pShellListView)
   {
      return;
   }

   m_pShellListView->SetColors(GetSysColor(COLOR_WINDOW),
                               GetSysColor(COLOR_WINDOWTEXT),
                               GetSysColor(COLOR_WINDOW));
}

void CDefShellView::LoadColumnHeaders()
{
   ASSERT(m_pShellListView);
   if (!m_pShellListView)
   {
      return;
   }

   LPCTSTR pszHeader = NULL;
   short* pColumnWidths = s_ColumnWidths;

   // override default settings upon user request
   if (m_pColumnWidths)
   {
      pColumnWidths = m_pColumnWidths;
   }

   pszHeader = LOAD_STRING(IDS_HDR_DEFVIEW_NAME);
   if (pszHeader)
   {
      m_pShellListView->AppendColumn(pszHeader, pColumnWidths[0], LVCFMT_LEFT);
   }

   pszHeader = LOAD_STRING(IDS_HDR_DEFVIEW_SIZE);
   if (pszHeader)
   {
      m_pShellListView->AppendColumn(pszHeader, pColumnWidths[1], LVCFMT_RIGHT);
   }

   pszHeader = LOAD_STRING(IDS_HDR_DEFVIEW_TYPE);
   if (pszHeader)
   {
      m_pShellListView->AppendColumn(pszHeader, pColumnWidths[2], LVCFMT_LEFT);
   }

   pszHeader = LOAD_STRING(IDS_HDR_DEFVIEW_DATE);
   if (pszHeader)
   {
      m_pShellListView->AppendColumn(pszHeader, pColumnWidths[3], LVCFMT_LEFT);
   }
}

HMENU CDefShellView::LoadPopupMenuTemplate()
{
   return ::LoadMenu(HINST_CESHELL, MAKEINTRESOURCE(IDM_DEF_POPUP_MENUS));
}

BOOL CDefShellView::MergeSubMenu(HMENU hmenuSub, LPINSMENUDATA lpInsMenuData, int nItems, BOOL focusActive)
{
   for (int i = 0; i < nItems; i++)
   {
      ::InsertMenu(hmenuSub, lpInsMenuData->uPosition, lpInsMenuData->uFlags,
                   lpInsMenuData->uIDMNewItem, LOAD_STRING(lpInsMenuData->idszMenuItem));

      // Move pointer to next structure
      lpInsMenuData = (LPINSMENUDATA)((LPBYTE)lpInsMenuData+sizeof(INSMENUDATA));
   }

   return TRUE;
}

BOOL CDefShellView::OnDefviewEditCommand()
{
   BOOL bRet = FALSE;

   if (IN_LABEL_EDIT & m_uState)
   {
      ::MessageBeep((UINT)-1);
      bRet = TRUE;
   }

   return bRet;
}

HRESULT CDefShellView::PerformAction(OLECMDID cmd)
{
   HRESULT hr = NOERROR;

   switch (cmd)
   {
      case OLECMDID_OPEN:
      {
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

             BOOL fBrowsed = FALSE;
             for (UINT i = 0; i < uItemCount; i++)
             {
                SFGAOF sfgaof = SFGAO_FOLDER | SFGAO_BROWSABLE;
                m_pShellFolder->GetAttributesOf(1, &aPidls[i], &sfgaof);
                if ((SFGAO_FOLDER | SFGAO_BROWSABLE) & sfgaof)
                {
                   if (!fBrowsed)
                   {
                      m_pShellBrowser->BrowseObject(aPidls[i], SBSP_DEFBROWSER | SBSP_DEFMODE | SBSP_RELATIVE);
                      fBrowsed = TRUE;
                   }
                   continue;
                }

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

      case OLECMDID_COPY:
      {
         UINT uItemCount = 0;
         LPCITEMIDLIST * aPidls = m_pShellListView->GetSelected(&uItemCount);
         if (aPidls)
         {
            ULONG uFlags = SFGAO_CANCOPY;

            if (SUCCEEDED(m_pShellFolder->GetAttributesOf(uItemCount,
                                                          aPidls,
                                                          &uFlags)))
            {
               if (SFGAO_CANCOPY & uFlags)
               {
                  CShellClipboard::Copy(*m_pShellListView, m_pShellFolder,
                                        uItemCount, aPidls);
               }
            }
            g_pShellMalloc->Free(aPidls);
         }
      }
      break;

      case OLECMDID_CUT:
      {
         UINT uItemCount = 0;
         LPCITEMIDLIST * aPidls = m_pShellListView->GetSelected(&uItemCount);
         if (aPidls)
         {
            ULONG uFlags = SFGAO_CANMOVE;

            if (SUCCEEDED(m_pShellFolder->GetAttributesOf(uItemCount,
                                                          aPidls,
                                                          &uFlags)))
            {
               if (SFGAO_CANMOVE & uFlags)
               {
                  if (CShellClipboard::Cut(*m_pShellListView, m_pShellFolder,
                                           uItemCount, aPidls))
                  {
                     m_pShellListView->ShowSelectedAsCut();
                  }
               }
            }
            g_pShellMalloc->Free(aPidls);
         }
      }
      break;

      case OLECMDID_PASTE:
      {
         UINT undoStr;
         DWORD dwContentType = CShellClipboard::GetContentType();

         if (ShellDataTransfer::CONTAINS_COPY_DATA & dwContentType)
         {
            undoStr = IDS_MENU_EDIT_UNDOCPY;
         }
         else if (ShellDataTransfer::CONTAINS_CUT_DATA & dwContentType)
         {
            undoStr = IDS_MENU_EDIT_UNDOMOVE;
         }
         else
         {
            undoStr = IDS_MENU_EDIT_UNDOPASTE;
         }

         if (CShellClipboard::Paste(*m_pShellListView, m_pidlFQ))
         {
            s_CurrentUndoStr = undoStr;
         }
      }
      break;

      case OLECMDID_REFRESH:
      {
         UserSettings::Query();
         Refresh();
      }
      break;

      case OLECMDID_UNDO:
      {
         if (g_pUndo && g_pUndo->HasData())
         {
            g_pUndo->Undo(m_hwnd);
         }
         s_CurrentUndoStr = IDS_MENU_EDIT_UNDO;
      }
      break;

      default:
         hr = OLECMDERR_E_NOTSUPPORTED;
   }

   return hr;
}


void CDefShellView::Resize(DWORD width, DWORD height)
{
   if (m_pShellListView)
   {
      m_pShellListView->Resize(width, height);
   }
}

BOOL CDefShellView::UpdateBrowserMenus(UINT uState)
{
   BOOL bRet;

   if (!m_pTopShellBrowser)
   {
      bRet = FALSE;
      goto leave;
   }

   // OnDeactivate
   if (SVUIA_DEACTIVATE != m_uFocus)
   {
      if (m_hMenu)
      {
         HWND hwndTB;
         // Need to save current Back/Forward states
         if (SUCCEEDED(m_pTopShellBrowser->GetControlWindow(FCW_TOOLBAR, &hwndTB)))
         {
            if (SendMessage(hwndTB, TB_GETSTATE, IDC_GO_BACK, 0) & TBSTATE_ENABLED)
            {
               m_uState |= BACK_ENABLED;
            }
            else
            {
               m_uState &= ~BACK_ENABLED;
            }

            if (SendMessage(hwndTB, TB_GETSTATE, IDC_GO_FORWARD, 0) & TBSTATE_ENABLED)
            {
               m_uState |= FWD_ENABLED;
            }
            else
            {
               m_uState &= ~FWD_ENABLED;
            }
         }

        // Release IContextMenu interface if there is one
        MENUITEMINFO mii = {0};
        mii.cbSize = sizeof(MENUITEMINFO);

        mii.fMask = MIIM_DATA;
        if (::GetMenuItemInfo(m_hMenu, 0, TRUE, &mii) && mii.dwItemData)
        {
           ((IContextMenu*)mii.dwItemData)->Release();
        }

         m_pTopShellBrowser->RemoveMenusSB(m_hMenu);
         // We must not call ::DestroyMenu(m_hMenu) explicitly.
         // It will get destroyed for us in previous call to RemoveMenusSB()
         // when the TopFrame destroys the CommandBar and restores its own menus
         m_hMenu = NULL;
      }
      m_uFocus = SVUIA_DEACTIVATE;
   }

   // OnActivate
   //only do this if we are active
   if (uState != SVUIA_DEACTIVATE)
   {
      ASSERT(!m_hMenu);

      //merge the menus
      m_hMenu = CreateMenu();

      if (m_hMenu)
      {
         HMENU hmenuSub, hmenuTemp, hmenuTrack;
         BOOL fActivateFocus= (BOOL)(SVUIA_ACTIVATE_FOCUS == uState);
         OLEMENUGROUPWIDTHS   omw = {0, 0, 0, 0, 0, 0};

         m_pTopShellBrowser->InsertMenusSB(m_hMenu, &omw);

         //get the view menu so we can merge with it

         //merge our items into the File menu
         if ((hmenuSub = ::GetSubMenu(m_hMenu, FILE_MENU_OFFSET)) != NULL)
         {
            MergeSubMenu(hmenuSub, (LPINSMENUDATA)filemenuData, FILE_MENU_ITEMS, fActivateFocus);

            hmenuTemp= ::LoadMenu(HINST_CESHELL, MAKEINTRESOURCE(IDM_FILE_SENDTO_MENU));
            hmenuTrack = ::GetSubMenu(hmenuTemp, 0);
            ::RemoveMenu(hmenuTemp, 0, MF_BYPOSITION);
            ::DestroyMenu(hmenuTemp);

            if (!::InsertMenu(hmenuSub, 7, MF_BYPOSITION | MF_POPUP, (UINT)hmenuTrack, LOAD_STRING(IDS_MENU_FILE_SENDTO)))
            {
               ::DestroyMenu(hmenuTrack);
            }
         }

         //merge our items into the Edit menu
         if ((hmenuSub = ::GetSubMenu(m_hMenu, EDIT_MENU_OFFSET)) != NULL)
         {
            // special case for Edit-Undo command
            m_UndoStr = s_CurrentUndoStr;
            ::InsertMenu(hmenuSub, 0, MF_BYPOSITION | MF_STRING, IDC_EDIT_UNDO, LOAD_STRING(s_CurrentUndoStr));

            MergeSubMenu(hmenuSub, (LPINSMENUDATA)&editmenuData, EDIT_MENU_ITEMS, fActivateFocus);
         }

         //merge our items into the View menu
         if ((hmenuSub = ::GetSubMenu(m_hMenu, VIEW_MENU_OFFSET)) != NULL)
         {
            MergeSubMenu(hmenuSub, (LPINSMENUDATA)&viewmenuData, VIEW_MENU_ITEMS, fActivateFocus);

            hmenuTemp = ::LoadMenu(HINST_CESHELL, MAKEINTRESOURCE(IDM_DEF_POPUP_MENUS));
            hmenuTrack = ::GetSubMenu(hmenuTemp, FOLDER_MENU_OFFSET);
            HMENU hmenuArrange = ::GetSubMenu(hmenuTrack, FOLDER_ARRANGEBY_MENU_OFFSET);
            ::RemoveMenu(hmenuTrack, FOLDER_ARRANGEBY_MENU_OFFSET, MF_BYPOSITION);
            ::DestroyMenu(hmenuTemp);

            if (!::InsertMenu(hmenuSub, 4, MF_BYPOSITION | MF_POPUP, (UINT)hmenuArrange, LOAD_STRING(IDS_MENU_VIEW_ARRANGEICONS)))
            {
               ::DestroyMenu(hmenuArrange);
            }
         }

         //merge our items into the Go Menu
         if ((hmenuSub = ::GetSubMenu(m_hMenu, GO_MENU_OFFSET)) != NULL)
         {
            MergeSubMenu(hmenuSub,(LPINSMENUDATA)&gomenuData, GO_MENU_ITEMS, fActivateFocus);
         }

         //add the items that should only be added if we have the focus
         if (SVUIA_ACTIVATE_FOCUS == uState)
         {
         }

         if (FAILED(m_pTopShellBrowser->SetMenuSB(m_hMenu, NULL, m_hwnd)))
         {
              ASSERT(0);
              // SHELL: SetMenu Failed
         }
      }
      else
      {
         ASSERT(0);
         // SHELL: CreateMenu Failed
      }

      UpdateBrowserToolbar(uState);
   }

   bRet = TRUE;

leave:
    return bRet;
}

BOOL CDefShellView::UpdateBrowserToolbar(UINT uState)
{
   BOOL bRet;
   BOOL bMirrored;

   if (!m_pTopShellBrowser)
   {
      bRet = FALSE;
      goto leave;
   }

   // Restore Back/Forward states on the new structure that is about to be set
   tbButton[0].fsState = (BYTE)(LOWORD((m_uState & BACK_ENABLED) ? TBSTATE_ENABLED:0));
   tbButton[1].fsState = (BYTE)(LOWORD((m_uState & FWD_ENABLED) ? TBSTATE_ENABLED:0));

#define SWAP(x,y, _type)  { _type i; i = x; x = y; y = i; }

   bMirrored = ::GetWindowLong(::GetParent(m_hwnd), GWL_EXSTYLE) & WS_EX_LAYOUTRTL;
   if (bMirrored)
   {
      // Swap images for Back and Forward buttons
      SWAP(tbButton[0].iBitmap, tbButton[1].iBitmap, int);
   }

   HRESULT hr = m_pTopShellBrowser->SetToolbarItems(tbButton, TOOLBAR_ITEMS, FCT_MERGE);

   HWND hwndTB;
   if (SUCCEEDED(m_pTopShellBrowser->GetControlWindow(FCW_TOOLBAR, &hwndTB)))
   {
      CommandBar_AddToolTips(hwndTB, TOOLTIPS, s_pszTooltips);
   }

   if (bMirrored)
   {
      SWAP(tbButton[0].iBitmap, tbButton[1].iBitmap, int);
   }

   bRet = SUCCEEDED(hr);

leave:
   return bRet;
}

void CDefShellView::UpdateCurrentDropTarget(DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{
   PREFAST_ASSERT(m_pDragData);

   HRESULT hr = NOERROR;
   LPCITEMIDLIST pidl = m_pShellListView->GetPidlAt(pt);
   IDropTarget * pDropTarget = NULL;

   // Find the drop target for the current item or folder
   if (pidl)
   {
      m_pDragData->m_fOverItem = TRUE;
      hr = m_pShellFolder->GetUIObjectOf(*m_pShellListView, 1, &pidl, IID_IDropTarget,
                                         NULL, (VOID **) &pDropTarget);
   }
   else
   {
      IShellFolder * pFolder = NULL;
      LPCITEMIDLIST pidlLast = NULL;

      m_pDragData->m_fOverItem = FALSE;

      hr = SHBindToParent(m_pidlFQ, IID_IShellFolder,
                          (VOID **) &pFolder, &pidlLast);
      if (SUCCEEDED(hr))
      {
         hr = pFolder->GetUIObjectOf(*m_pShellListView, 1, &pidlLast, IID_IDropTarget,
                                     NULL, (VOID **) &pDropTarget);
         pFolder->Release();
         ILFree(pidlLast);
      }
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
      {
         pDropTarget->Release();
      }

      m_pDragData->m_pCurrentDropTarget = NULL;
      *pdwEffect = DROPEFFECT_NONE;
   }
}

void CDefShellView::UpdateStatusBar(LPCITEMIDLIST *aPidls , UINT uItemCount, BOOL fInitialize)
{
   WCHAR wszStatus[128];
   WCHAR wszTemp[MAX_PATH];
   UINT uMsg;
   LPCITEMIDLIST * aPidlsInternal = NULL;
   IShellBrowser* pTopShellBrowser = m_pTopShellBrowser;

   if (!pTopShellBrowser)
   {
      return;
   }

   pTopShellBrowser->AddRef(); // AddRef since it's possible to kill the view on another thread

   if (fInitialize)
   {
      int nParts = 0, i;
      HICON hSatusIcon = s_hDRootIcon;
      LPWSTR pszStatusZone = s_pszDRootName;

      pTopShellBrowser->SendControlMsg(FCW_STATUS, SB_GETPARTS, 0, 0, (LRESULT*)&nParts);
      ASSERT(nParts > STATUS_PART_ICON);

      for (i =0; i<nParts; i++)
      {
         pTopShellBrowser->SendControlMsg(FCW_STATUS, SB_SETTEXT, i, (LPARAM)TEXT(""), NULL);
         pTopShellBrowser->SendControlMsg(FCW_STATUS, SB_SETICON, i, NULL, NULL);
      }

      // Reset zone for network folders
      if (SHGetPathFromIDList(m_pidlFQ, wszTemp) &&
        PathMatchSpec(wszTemp, L"\\\\*"))
      {
         hSatusIcon = NULL;
         pszStatusZone = NULL;
      }

      pTopShellBrowser->SendControlMsg(FCW_STATUS,
                               SB_SETTEXT,
                               STATUS_PART_ICON,
                               (LPARAM)pszStatusZone,
                               NULL);

      pTopShellBrowser->SendControlMsg(FCW_STATUS,
                               SB_SETICON,
                               STATUS_PART_ICON,
                               (LPARAM)hSatusIcon,
                               NULL);
   }

   if (!aPidls)
   {
       aPidlsInternal = m_pShellListView->GetSelected(&uItemCount);
       aPidls = aPidlsInternal;
   }

   switch (uItemCount)
   {
      case 0:
         // No objects selected; show total item count
         uItemCount = ListView_GetItemCount(*m_pShellListView);
         uMsg = IDS_STATUS_BASE;
         break;

      case 1:
      {
         ULONG uFlags = SFGAO_FILESYSTEM | SFGAO_FOLDER;
         if (SUCCEEDED(m_pShellFolder->GetAttributesOf(uItemCount,
                                  aPidls,
                                  &uFlags)))
         {
            if (uFlags != SFGAO_FILESYSTEM)
            {
                goto DoDefault;
            }

            wcscpy(wszTemp, L": ");
            NMLVDISPINFO nmlvdi;
            size_t cchRemaining = lengthof(wszStatus);

            LPWSTR pwszStatus = wszStatus;

            nmlvdi.item.iItem = m_pShellListView->FindItemFromPidl(*aPidls);
            nmlvdi.item.lParam = (LPARAM)m_pShellListView->GetItemPidl(nmlvdi.item.iItem);
            if (!nmlvdi.item.lParam)
            {
               goto DoDefault;
            }

            nmlvdi.item.mask = LVIF_TEXT;
            nmlvdi.item.pszText = wszTemp+2;
            nmlvdi.item.cchTextMax = lengthof(wszTemp)-2;

            static int iDetails[3][2] = {
                {2, IDS_HDR_DEFVIEW_TYPE},
                {3, IDS_HDR_DEFVIEW_DATE},
                {1, IDS_HDR_DEFVIEW_SIZE},
            };

            for (int i=0; i<3; i++)
            {
               StringCchCopyEx(pwszStatus,
                                        cchRemaining,
                                        LOAD_STRING(iDetails[i][1]),
                                        &pwszStatus,
                                        &cchRemaining,
                                        STRSAFE_NULL_ON_FAILURE
                                        );

               nmlvdi.item.iSubItem = iDetails[i][0];
               GetDisplayInfo(&nmlvdi);

               StringCchCopyEx(pwszStatus,
                                        cchRemaining,
                                        wszTemp,
                                        &pwszStatus,
                                        &cchRemaining,
                                        STRSAFE_NULL_ON_FAILURE
                                        );
               if (cchRemaining > 0)
               {
                  *pwszStatus = L' ';
                  pwszStatus++;
                  cchRemaining--;
               }
            }

            pwszStatus--;
            *pwszStatus = L'\0';
            goto SetText;
         }
         // FALL THROUGH
      }

DoDefault:
      default:
         uMsg = IDS_STATUS_SELECTED;
         break;
   }

   ShellFormat::FolderStatus(uItemCount, uMsg, wszStatus, lengthof(wszStatus));

SetText:
   pTopShellBrowser->SendControlMsg(FCW_STATUS,
                            SB_SETTEXT,
                            STATUS_PART_TEXT,
                            (LPARAM)wszStatus,
                            NULL);

   pTopShellBrowser->Release();

   if (aPidlsInternal)
   {
      g_pShellMalloc->Free(aPidlsInternal);
   }
}

void CDefShellView::UpdateUndoMenuItem(HMENU hmenu)
{
   if (s_CurrentUndoStr != m_UndoStr)
   {
      WCHAR szText[MAX_PATH];

      if (::LoadString(HINST_CESHELL, s_CurrentUndoStr, szText, MAX_PATH))
      {
         MENUITEMINFO mii;
         mii.cbSize = sizeof(MENUITEMINFO);
         mii.fMask = MIIM_TYPE;
         mii.fType = MFT_STRING;
         mii.dwTypeData = szText;
         mii.cch = wcslen(szText);

         if (::SetMenuItemInfo(hmenu, IDC_EDIT_UNDO, FALSE, &mii))
         {
            m_UndoStr = s_CurrentUndoStr;
         }
      }
   }
}


//////////////////////////////////////////////////
// IUnknown

STDMETHODIMP CDefShellView::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
   HRESULT hr = E_NOINTERFACE;

   if (!ppReturn)
   {
      ASSERT(0);
      hr = E_INVALIDARG;
      goto leave;
   }

   *ppReturn = NULL;

   if (IsEqualIID(riid, IID_IUnknown)) // IUnknown
   {
      *ppReturn = this;
   }
   else if (IsEqualIID(riid, IID_IOleWindow)) // IOleWindow
   {
      *ppReturn = (IOleWindow *)(IShellView *) this;
   }
   else if (IsEqualIID(riid, IID_IShellView)) // IShellView
   {
      *ppReturn = (IShellView *) this;
   }
   else if (IsEqualIID(riid, IID_IOleCommandTarget)) // IOleCommandTarget
   {
      *ppReturn = (IOleCommandTarget *) this;
   }
   else if (IsEqualIID(riid, IID_IOleInPlaceActiveObject)) // IOleInPlaceActiveObject
   {
      *ppReturn = (IOleInPlaceActiveObject *) this;
   }
   else if (IsEqualIID(riid, IID_IShellChangeNotify)) // IShellChangeNotify
   {
      *ppReturn = (IShellChangeNotify *) this;
   }
   else if (IsEqualIID(riid, IID_IDropSource)) // IDropSource
   {
      *ppReturn = (IDropSource *) this;
   }
   else if (IsEqualIID(riid, IID_IDropTarget)) // IDropTarget
   {
      *ppReturn = (IDropTarget *) this;
   }
   else if (IsEqualIID(riid, IID_IEnumFilter)) // IEnumFilter
   {
      *ppReturn = (IEnumFilter *) this;
   }
   else if (IsEqualIID(riid, IID_IShellListView)) // IShellListView
   {
      *ppReturn = (IShellListView *) this;
   }

   if (*ppReturn)
   {
      (*(LPUNKNOWN*)ppReturn)->AddRef();
      hr = S_OK;
   }

leave:
   return hr;
}

STDMETHODIMP_(DWORD) CDefShellView::AddRef()
{
   return ++m_ObjRefCount;
}

STDMETHODIMP_(DWORD) CDefShellView::Release()
{
   if (--m_ObjRefCount == 0)
   {
      delete this;
      return 0;
   }

   return m_ObjRefCount;
}


//////////////////////////////////////////////////
// IOleWindow

STDMETHODIMP CDefShellView::ContextSensitiveHelp(BOOL fEnterMode)
{
   return E_NOTIMPL;
}

STDMETHODIMP CDefShellView::GetWindow(HWND * phwnd)
{
   HRESULT hr;

   ASSERT(phwnd);
   if (!phwnd)
   {
      hr = E_INVALIDARG;
      goto leave;
   }

   *phwnd = m_hwnd;
   hr = S_OK;

leave:
   return hr;
}


//////////////////////////////////////////////////
// IShellView

STDMETHODIMP CDefShellView::AddPropertySheetPages(DWORD dwReserved,
                                                  LPFNADDPROPSHEETPAGE lpfn,
                                                  LPARAM lParam)
{
   return E_NOTIMPL;
}

STDMETHODIMP CDefShellView::CreateViewWindow(LPSHELLVIEW pPrevView,
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
   *phwnd = NULL;

   // Should the caller be creating another view window without first destroying the old one?
   ASSERT(!m_hwnd);
   if (m_hwnd)
   {
      DestroyViewWindow();
   }

   WNDCLASS wc = {0};
   HWND hwndParent = NULL;

   // Generate the class
   if (!GetClassInfo(HINST_CESHELL, WC_DEFSHELLVIEW, &wc))
   {
      wc.style = CS_HREDRAW | CS_VREDRAW;
      wc.lpfnWndProc = (WNDPROC) WndProc;
      wc.hInstance = HINST_CESHELL;
      wc.hCursor = LoadCursor(NULL, IDC_ARROW);
      wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
      wc.lpszClassName = WC_DEFSHELLVIEW;

      if (!::RegisterClass(&wc))
      {
         return E_FAIL;
      }
   }

   // Set our internal parameters
   m_pShellBrowser = psb;

   // AddRef because the docs tell us to
   m_pShellBrowser->AddRef();

   m_ViewMode = lpfs->ViewMode;
   m_fFlags = (FWF_SHOWSELALWAYS | lpfs->fFlags); // Force on FWF_SHOWSELALWAYS

   // Create the window
   m_pShellBrowser->GetWindow(&hwndParent);
   *phwnd = ::CreateWindow(WC_DEFSHELLVIEW, NULL,
                           WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                           prcView->left, prcView->top,
                           prcView->right - prcView->left,
                           prcView->bottom - prcView->top,
                           hwndParent, NULL, HINST_CESHELL, (LPVOID) this);
   if (!(*phwnd))
   {
      return E_FAIL;
   }

   // Add reference to the Tooltips
   TTAddRef();

   // Add reference to DrivesRoot's icon and displayname
   FSAddRef();

   // Get the IShellBrowser for explorer
   ASSERT(!m_pTopShellBrowser);
   IServiceProvider * pServiceProvider;
   HRESULT hr = m_pShellBrowser->QueryInterface(IID_IServiceProvider,
                                                (void**) &pServiceProvider);
   if (SUCCEEDED(hr))
   {
      hr = pServiceProvider->QueryService(SID_QIClientSite,
                                          IID_IShellBrowser,
                                          (void**) &m_pTopShellBrowser);
      pServiceProvider->Release();
   }

   // Get IOleInPlaceUIWindow and set our IOleInPlaceActiveObject
   ASSERT(!m_pOleInPlaceUIWindow);
   if (SUCCEEDED(m_pShellBrowser->QueryInterface(IID_IOleInPlaceUIWindow,
                                                (void**) &m_pOleInPlaceUIWindow)))
   {
      m_pOleInPlaceUIWindow->SetActiveObject((IOleInPlaceActiveObject *) this, NULL);
   }

   ::ShowWindow(*phwnd, SW_SHOW);

   // Get the icon for this window
   if (m_pTopShellBrowser)
   {
      IShellFolder * pFolder = NULL;
      LPCITEMIDLIST pidlLast = NULL;

      if (SUCCEEDED(SHBindToParent(m_pidlFQ, IID_IShellFolder,
                                   (VOID **) &pFolder, &pidlLast)))
      {
         SHFILEINFO sfi = {0};
         WCHAR wszDisplayName[MAX_PATH];
         STRRET str = {0};

         if (SUCCEEDED(pFolder->GetDisplayNameOf(pidlLast, SHGDN_NORMAL | SHGDN_FORPARSING, &str)) &&
             SUCCEEDED(StrRetToBuf(&str, pidlLast, wszDisplayName, lengthof(wszDisplayName))))
         {
            LPWSTR pwszDisplayName = wszDisplayName;
            UINT uFlags = SHGFI_ICON | SHGFI_SMALLICON;

            // Walk over the :: that is prepended to the last guid i.e. ::{12345678-1234-1234-1234-123456789ABC}
            if (ILIsNameSpacePidl(pidlLast))
            {
               pwszDisplayName = ::wcsrchr(wszDisplayName, L'{');
               ASSERT(pwszDisplayName);
               uFlags |= SHGFI_USEFILEATTRIBUTES;
            }

            if (SHGetFileInfo(pwszDisplayName, 0, &sfi, sizeof(SHFILEINFO), uFlags))
            {
               HWND hwndFrame = NULL;
               m_pTopShellBrowser->GetWindow(&hwndFrame);
               ::SendMessage(hwndFrame, WM_SETICON, ICON_SMALL, (WPARAM)sfi.hIcon);
               m_hIcon = sfi.hIcon;
            }
         }

         pFolder->Release();
         ILFree(pidlLast);
      }
   }

   UpdateStatusBar(NULL, 0, TRUE);

   return S_OK;
}

STDMETHODIMP CDefShellView::DestroyViewWindow()
{
   // Make sure that the UI is deactivated
   UIActivate(SVUIA_DEACTIVATE);

   // Release Tooltips
   TTRelease();

   // Release DrivesRoot's icon and displayname
   FSRelease();

   ASSERT(!m_hMenu);
   if (m_hMenu)
   {
      ASSERT(m_pTopShellBrowser);

      // Release IContextMenu interface if there is one
      MENUITEMINFO mii = {0};
      mii.cbSize = sizeof(MENUITEMINFO);

      mii.fMask = MIIM_DATA;
      if (::GetMenuItemInfo(m_hMenu, 0, TRUE, &mii) && mii.dwItemData)
      {
         ((IContextMenu*)mii.dwItemData)->Release();
      }

      if (m_pTopShellBrowser)
      {
         m_pTopShellBrowser->RemoveMenusSB(m_hMenu);
      }

      // We must not call ::DestroyMenu(m_hMenu) explicitly.
      // It will get destroyed for us in previous call to RemoveMenusSB()
      // when the TopFrame destroys the CommandBar and restores its own menus
   }

   if (m_hwnd)
   {
      ::DestroyWindow(m_hwnd);
   }

   if (m_pOleInPlaceUIWindow)
   {
      // The doc tell us to do the following call, but shdocview does not do anything
      // m_pOleInPlaceUIWindow->SetActiveObject(NULL, NULL);
      m_pOleInPlaceUIWindow->Release();
      m_pOleInPlaceUIWindow = NULL;
   }

   if (m_pShellBrowser)
   {
      m_pShellBrowser->Release();
      m_pShellBrowser = NULL;
   }

   if (m_pTopShellBrowser)
   {
      HWND hwndFrame = NULL;
      HICON hiconSmall = NULL;

      // Clean the icon for this window
      m_pTopShellBrowser->GetWindow(&hwndFrame);
      hiconSmall = (HICON) ::SendMessage(hwndFrame, WM_GETICON, ICON_SMALL, NULL);

      // There may be another active View,
      // if current icon is different from the one initially set.
      if (hiconSmall && (hiconSmall == m_hIcon))
      {
         ::SendMessage(hwndFrame, WM_SETICON, ICON_SMALL, NULL);

         // Clean used parts for Statusbar
         m_pTopShellBrowser->SendControlMsg(FCW_STATUS,
                                  SB_SETTEXT,
                                  STATUS_PART_TEXT,
                                  (LPARAM)TEXT(""),
                                  NULL);
         m_pTopShellBrowser->SendControlMsg(FCW_STATUS,
                                  SB_SETTEXT,
                                  STATUS_PART_ICON,
                                  (LPARAM)TEXT(""),
                                  NULL);
         m_pTopShellBrowser->SendControlMsg(FCW_STATUS,
                                  SB_SETICON,
                                  STATUS_PART_ICON,
                                  NULL,
                                  NULL);
      }

      m_pTopShellBrowser->Release();
      m_pTopShellBrowser = NULL;
   }

   if (m_hIcon)
   {
      ::DestroyIcon(m_hIcon);
   }

   return S_OK;
}

STDMETHODIMP CDefShellView::EnableModeless(BOOL fEnable)
{
   return E_NOTIMPL;
}

STDMETHODIMP CDefShellView::GetCurrentInfo(LPFOLDERSETTINGS lpfs)
{
   HRESULT hr;

   if (!lpfs)
   {
      ASSERT(0);
      hr = E_INVALIDARG;
      goto leave;
   }

   lpfs->ViewMode = m_ViewMode;
   lpfs->fFlags = m_fFlags;

   hr = S_OK;

leave:
   return hr;
}

STDMETHODIMP CDefShellView::GetItemObject(UINT uItem,
                                          REFIID riid,
                                          LPVOID *ppvOut)
{
   HRESULT hr = E_NOTIMPL;

   if (!ppvOut)
   {
      ASSERT(0);
      hr = E_INVALIDARG;
      goto leave;
   }

   *ppvOut = NULL;

leave:
   return hr;
}

STDMETHODIMP CDefShellView::Refresh()
{
   HRESULT hr = NOERROR;

   if (IN_REFRESH & m_uState)
   {
      return S_FALSE;
   }

   if (IN_LABEL_APPLY & m_uState)
   {
      // RenameNamespacePidl calls refresh on all views. 
      // Delay the refresh for the view on ApplyLabelEdit.
      ::PostMessage(m_hwnd, WM_COMMAND, IDC_REFRESH, NULL);
      goto leave;
   }

   m_uState |= IN_REFRESH;

   if (m_pShellListView && m_pShellListView->Clear())
   {
      SHCONTF grfFlags = SHCONTF_NONFOLDERS | SHCONTF_FOLDERS;
      if (UserSettings::GetShowHiddenFiles())
      {
         grfFlags |= SHCONTF_INCLUDEHIDDEN;
      }

      LPENUMIDLIST pEnumIDList = NULL;
      hr = m_pShellFolder->EnumObjects(m_hwnd, grfFlags, &pEnumIDList);
      if (SUCCEEDED(hr))
      {
         IEnumFilter* pIEnumFilter;
         if (m_pszFilter && 
            SUCCEEDED(pEnumIDList->QueryInterface(IID_IEnumFilter, (void**)&pIEnumFilter)))
         {
            // Note that SetFilterString() will reset EnumFilter on failure
            pIEnumFilter->SetFilterString(m_pszFilter);
            pIEnumFilter->Release();
         }

         m_pShellListView->SetRedraw(FALSE);
         if (m_pShellListView->Populate(pEnumIDList))
         {
            m_pShellListView->Sort(CompareItems, (LPARAM) this);
            ListView_SetItemState(*m_pShellListView, 0, LVIS_FOCUSED, LVIS_FOCUSED);
         }
         m_pShellListView->SetRedraw(TRUE);
         pEnumIDList->Release();

         UpdateStatusBar(NULL, 0, FALSE);
      }
   }
   else
   {
      hr = E_FAIL;
   }

   m_uState &= ~IN_REFRESH;

leave:
   return hr;
}

STDMETHODIMP CDefShellView::SaveViewState()
{
   return E_NOTIMPL;
}

STDMETHODIMP CDefShellView::SelectItem(LPCITEMIDLIST pidlItem, UINT uFlags)
{
   return E_NOTIMPL;
}

STDMETHODIMP CDefShellView::TranslateAccelerator(LPMSG pmsg)
{
   if (IN_LABEL_EDIT & m_uState)
   {
      if ((WM_KEYFIRST <= pmsg->message) && (WM_KEYLAST >= pmsg->message))
      {
         if (!(pmsg->message == WM_KEYDOWN && pmsg->wParam == VK_TAB))
         {
            ::TranslateMessage(pmsg);
            ::DispatchMessage(pmsg);
         }
         return S_OK;
      }
      else
      {
         return S_FALSE;
      }
   }
   else
   {
      HRESULT hr = S_FALSE;

      if (m_pShellBrowser && (pmsg->hwnd == m_hwnd || m_hwnd == ::GetParent(pmsg->hwnd)))
      {
         if (pmsg->message == WM_KEYDOWN && pmsg->wParam == VK_TAB)
         {
            // Get the IOleControlSite for explorer
            IOleControlSite *   pCtrlSite;
            IServiceProvider * pServiceProvider;

            if (SUCCEEDED(m_pShellBrowser->QueryInterface(IID_IServiceProvider, (void**)&pServiceProvider)))
            {
               if (SUCCEEDED(pServiceProvider->QueryService(SID_QIClientSite, IID_IOleControlSite, (void**)&pCtrlSite)))
               {
                  DWORD grfModifiers = 0;
                  if (GetKeyState(VK_SHIFT) & 0x8000)
                  {
                     grfModifiers += 1;  // KEYMOD_SHIFT
                  }

                  if (GetKeyState(VK_CONTROL) & 0x8000)
                  {
                     grfModifiers += 2;  // KEYMOD_CONTROL
                  }

                  if (GetKeyState(VK_MENU) & 0x8000)
                  {
                     grfModifiers += 4;  // KEYMOD_ALT
                  }

                  hr = pCtrlSite->TranslateAccelerator(pmsg, grfModifiers);
                  pCtrlSite->Release();
               }
               pServiceProvider->Release();
            }
         }
      }
      else if (pmsg->message == WM_KEYDOWN && pmsg->wParam == VK_TAB)
      {
         m_pShellListView->OnSetFocus();
         hr = S_OK;
      }

      if (hr != S_FALSE)
      {
         return hr;
      }
      else
      {
         return (::TranslateAccelerator(m_hwnd, m_hAccels, pmsg) ? S_OK : S_FALSE);
      }
   }
}

STDMETHODIMP CDefShellView::UIActivate(UINT uState)
{
   if (uState == m_uFocus)
   {
      return S_OK;
   }

   UpdateBrowserMenus(uState);
   m_uFocus = uState;

   return S_OK;
}


//////////////////////////////////////////////////
// IOleCommandTarget

STDMETHODIMP CDefShellView::QueryStatus(const GUID * pguidCmdGroup,
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

   if (!m_pShellListView)
   {
      return E_FAIL;
   }

   HRESULT hr = S_OK;

   if (!pguidCmdGroup)
   {
      // the standard group
      if (pCmdText && (OLECMDTEXTF_NONE != pCmdText->cmdtextf))
      {
         if (OLECMDTEXTF_NAME == pCmdText->cmdtextf)
         {
            // Name
            hr = StringCchCopyW(pCmdText->rgwz, pCmdText->cwBuf, TEXT("TODO"));
            if (SUCCEEDED(hr))
            {
               pCmdText->cwActual = (wcslen(pCmdText->rgwz)+1);
            }
         }
         else if (OLECMDTEXTF_STATUS == pCmdText->cmdtextf)
         {
            // Status
            hr = StringCchCopyW(pCmdText->rgwz, pCmdText->cwBuf, TEXT("TODO"));
            if (SUCCEEDED(hr))
            {
               pCmdText->cwActual = (wcslen(pCmdText->rgwz)+1);
            }
         }
      }
   }
   else if (IsEqualGUID(CGID_CEShell, *pguidCmdGroup))
   {
      // Shell cmd group
      ULONG uFlags = (SFGAO_CANMOVE | SFGAO_CANCOPY | SFGAO_CANDELETE | SFGAO_CANRENAME);
      UINT uItemCount = 0;
      LPCITEMIDLIST * aPidls = m_pShellListView->GetSelected(&uItemCount);
      BOOL fSelection = FALSE;

      if (aPidls)
      {
         fSelection =  SUCCEEDED(m_pShellFolder->GetAttributesOf(uItemCount, aPidls, &uFlags));
      }

      for (ULONG i = 0; i < cCmds; i++)
      {
         prgCmds[i].cmdf= OLECMDF_SUPPORTED;

         switch(prgCmds[i].cmdID)
         {
             case IDC_FILE_OPEN:
             {
                // Do dynamic Filemenu items
                MENUITEMINFO mii = {0};
                mii.cbSize = sizeof(MENUITEMINFO);

                // Release IContextMenu interface if there is one
                mii.fMask = MIIM_DATA;
                if (::GetMenuItemInfo(m_hMenu, 0, TRUE, &mii) && mii.dwItemData)
                {
                    ((IContextMenu*)mii.dwItemData)->Release();
                    mii.dwItemData = 0;
                    ::SetMenuItemInfo(m_hMenu, 0, TRUE, &mii);
                }

                HMENU hmenuSub = ::GetSubMenu(m_hMenu, FILE_MENU_OFFSET);
                if (hmenuSub)
                {
                   // Clean previous User items
                   mii.fMask = MIIM_ID;
                   while(::GetMenuItemInfo(hmenuSub, USER_INSERT_OFFSET, TRUE, &mii))
                   {
                      if (mii.wID < IDC_USER_MENU_ITEMS_BEGIN ||
                            mii.wID > IDC_USER_MENU_ITEMS_END )
                      {
                         break;
                      }
                      ::RemoveMenu(hmenuSub, USER_INSERT_OFFSET, MF_BYPOSITION);
                   }
                }

                if (fSelection)
                {
                    prgCmds[i].cmdf |= OLECMDF_ENABLED;

                    IContextMenu* pIContextMenu = NULL;
                    if (SUCCEEDED(m_pShellFolder->GetUIObjectOf(m_hwnd, 1, aPidls,
                                                  IID_IContextMenu, NULL,
                                                  (LPVOID*) &(pIContextMenu))))
                    {
                       if (hmenuSub)
                       {
                          pIContextMenu->QueryContextMenu(hmenuSub, USER_INSERT_OFFSET,
                                                        IDC_USER_MENU_ITEMS_BEGIN,
                                                        IDC_USER_MENU_ITEMS_END,
                                                        CMF_NORMAL);
                       }
                       // Release IContextMenu when dynamic item is removed
                       mii.fMask = MIIM_DATA;
                       mii.dwItemData = (DWORD) pIContextMenu;
                       ::SetMenuItemInfo(m_hMenu, 0, TRUE, &mii);
                    }
                }
             }
             break;

             case IDC_FILE_SENDTO_DESKTOP:
             case IDC_FILE_SENDTO_MYDOCUMENTS:
                if (fSelection)
                {
                   prgCmds[i].cmdf |= OLECMDF_ENABLED;
                }
             break;

             case IDC_FILE_DELETE:
                if (fSelection && (SFGAO_CANDELETE & uFlags))
                {
                   prgCmds[i].cmdf |= OLECMDF_ENABLED;
                }
             break;

             case IDC_FILE_RENAME:
                if (fSelection && (SFGAO_CANRENAME & uFlags) && (1 == uItemCount))
                {
                   prgCmds[i].cmdf |= OLECMDF_ENABLED;
                }
             break;

             case IDC_EDIT_UNDO:
                if (g_pUndo && g_pUndo->HasData())
                {
                   prgCmds[i].cmdf |= OLECMDF_ENABLED;
                }
                UpdateUndoMenuItem(GetSubMenu(m_hMenu, EDIT_MENU_OFFSET));
             break;

             case IDC_EDIT_CUT:
                if (fSelection && (SFGAO_CANMOVE & uFlags))
                {
                   prgCmds[i].cmdf |= OLECMDF_ENABLED;
                }
             break;

             case IDC_EDIT_COPY:
                if (fSelection && (SFGAO_CANCOPY & uFlags))
                {
                   prgCmds[i].cmdf |= OLECMDF_ENABLED;
                }
             break;

             case IDC_EDIT_PASTE:
                if (CShellClipboard::HasData())
                {
                   prgCmds[i].cmdf |= OLECMDF_ENABLED;
                }
             break;

             case IDC_EDIT_PASTESHORTCUT:
                if (ShellDataTransfer::CONTAINS_COPY_DATA &
                   CShellClipboard::GetContentType())
                {
                   prgCmds[i].cmdf |= OLECMDF_ENABLED;
                }
             break;

             case IDC_VIEW_ICONS:
                prgCmds[i].cmdf |= OLECMDF_ENABLED;
                if (m_ViewMode == FVM_ICON)
                {
                   prgCmds[i].cmdf |= OLECMDF_LATCHED;
                }
             break;

             case IDC_VIEW_SMALL:
                prgCmds[i].cmdf |= OLECMDF_ENABLED;
                if (m_ViewMode == FVM_SMALLICON)
                {
                   prgCmds[i].cmdf |= OLECMDF_LATCHED;
                }
             break;

             case IDC_VIEW_LIST:
                prgCmds[i].cmdf |= OLECMDF_ENABLED;
                if (m_ViewMode == FVM_LIST)
                {
                   prgCmds[i].cmdf |= OLECMDF_LATCHED;
                }
             break;

             case IDC_VIEW_DETAILS:
                prgCmds[i].cmdf |= OLECMDF_ENABLED;
                if (m_ViewMode == FVM_DETAILS)
                {
                   prgCmds[i].cmdf |= OLECMDF_LATCHED;
                }
             break;

             case IDC_FILE_NEWFOLDER:
             case IDC_FILE_PROPERTIES:
             case IDC_EDIT_SELECTALL:
             case IDC_VIEW_OPTIONS:
             case IDC_DISMISS_EDIT:
             case IDC_GO_FOLDERUP:
             case IDC_GO_MYDOCUMENTS:
                prgCmds[i].cmdf |= OLECMDF_ENABLED;
             break;

             case IDC_ARRANGE_BYNAME:
                prgCmds[i].cmdf |= OLECMDF_ENABLED;
                if (m_ArrangeBy == 0)
                {
                   prgCmds[i].cmdf |= OLECMDF_LATCHED;
                }
             break;

             case IDC_ARRANGE_BYSIZE:
                prgCmds[i].cmdf |= OLECMDF_ENABLED;
                if (m_ArrangeBy == 1)
                {
                   prgCmds[i].cmdf |= OLECMDF_LATCHED;
                }
             break;

             case IDC_ARRANGE_BYTYPE:
                prgCmds[i].cmdf |= OLECMDF_ENABLED;
                if (m_ArrangeBy == 2)
                {
                   prgCmds[i].cmdf |= OLECMDF_LATCHED;
                }
             break;

             case IDC_ARRANGE_BYDATE:
                prgCmds[i].cmdf |= OLECMDF_ENABLED;
                if (m_ArrangeBy == 3)
                {
                   prgCmds[i].cmdf |= OLECMDF_LATCHED;
                }
             break;

             case IDC_ARRANGE_AUTO:
                prgCmds[i].cmdf |= OLECMDF_ENABLED;
                if (m_pShellListView->IsAutoArrangeEnabled())
                {
                   prgCmds[i].cmdf |= OLECMDF_LATCHED;
                }
             break;

             default:
                prgCmds[i].cmdf = 0;
         }

      }

      if (aPidls)
      {
         g_pShellMalloc->Free(aPidls);
      }
   }
   else
   {
       hr= OLECMDERR_E_UNKNOWNGROUP;
   }

   return hr;
}

STDMETHODIMP CDefShellView::Exec(const GUID * pguidCmdGroup,
                                 DWORD nCmdID,
                                 DWORD nCmdExecOpt,
                                 VARIANTARG * pvaIn,
                                 VARIANTARG * pvaOut)
{
   if (!pguidCmdGroup)
   {
       // Standard Command Group
       return PerformAction((OLECMDID) nCmdID/*, nCmdExecOpt*/);
   }

   if (IsEqualGUID(CGID_CEShell, *pguidCmdGroup))
   {
       return HandleCommand(nCmdID);
   }
   else
   {
       return OLECMDERR_E_UNKNOWNGROUP;
   }
}


//////////////////////////////////////////////////
// IOleInPlaceActiveObject

STDMETHODIMP CDefShellView::OnFrameWindowActivate(BOOL fActivate)
{
   if (m_pShellListView && fActivate)
   {
      ::SetFocus(*m_pShellListView);
   }
   return S_OK;
};

STDMETHODIMP CDefShellView::OnDocWindowActivate(BOOL fActivate)
{
   return E_NOTIMPL;
}

STDMETHODIMP CDefShellView::ResizeBorder(LPCRECT prcBorder,
                                 IOleInPlaceUIWindow* pUIWindow,
                                 BOOL fFrameWindow)
{
   return E_NOTIMPL;
}


//////////////////////////////////////////////////
// IShellChangeNotify

STDMETHODIMP CDefShellView::OnChange(LONG lEvent,
                                     LPCITEMIDLIST pidl1,
                                     LPCITEMIDLIST pidl2)
{
    HRESULT hr = S_OK;
    ChangeNotifyEventData * pChangeNotifyEventData = new ChangeNotifyEventData();

    if (NULL != pChangeNotifyEventData)
    {
        pChangeNotifyEventData->lEvent = lEvent;

        if (NULL != pidl1)
        {
            pChangeNotifyEventData->pidl1 = ILCopy(pidl1, IL_ALL);
            if (NULL == pChangeNotifyEventData->pidl1)
            {
                hr = E_OUTOFMEMORY;
                goto leave;
            }
        }

        if (NULL != pidl2)
        {
            pChangeNotifyEventData->pidl2 = ILCopy(pidl2, IL_ALL);
            if (NULL == pChangeNotifyEventData->pidl2)
            {
                hr = E_OUTOFMEMORY;
                goto leave;
            }
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

leave:
    if (SUCCEEDED(hr))
    {
        // We handle this with a PostMessage because we need the event to be
        // processed on the main thread and CDefShellView::OnChange gets called
        // on the FileChangeManager thread.
        if (0 == ::PostMessage(m_hwnd, WM_DEFSHELLVIEW_CHANGENOTIFYEVENT, 0, reinterpret_cast<LPARAM>(pChangeNotifyEventData)))
        {
            hr = E_FAIL;
        }
    }

    if (FAILED(hr))
    {
        if (NULL != pChangeNotifyEventData)
        {
            delete pChangeNotifyEventData;
        }
    }

    return hr;
}


//////////////////////////////////////////////////
// IDropSource

STDMETHODIMP CDefShellView::QueryContinueDrag(BOOL fEsc,
                                              DWORD dwKeyState)
{
   if (fEsc)
   {
      return DRAGDROP_S_CANCEL;
   }

   // Make sure the left mouse button is still down
   if (!(dwKeyState & MK_LBUTTON))
   {
      return DRAGDROP_S_DROP;
   }

   return NOERROR;
}

STDMETHODIMP CDefShellView::GiveFeedback(DWORD dwEffect)
{
   HRESULT hr = NOERROR;

   if (!m_pShellListView->GiveFeedback(dwEffect))
   {
      hr = DRAGDROP_S_USEDEFAULTCURSORS;
   }

   return hr;
}


//////////////////////////////////////////////////
// IDropTarget

STDMETHODIMP CDefShellView::DragEnter(LPDATAOBJECT pDataObj,
                                      DWORD dwKeyState,
                                      POINTL pt,
                                      LPDWORD pdwEffect)
{
   ASSERT(pDataObj);
   ASSERT(pdwEffect);
   if (!pDataObj || !pdwEffect)
   {
      return E_INVALIDARG;
   }

   ASSERT(!m_pDragData);
   m_pDragData = new DragData();
   if (!m_pDragData)
   {
      return E_OUTOFMEMORY;
   }

   HRESULT hr = NOERROR;

   // Set up the DragData structure
   m_pDragData->m_dwDropEffect = DROPEFFECT_NONE;
   m_pDragData->m_pCurrentDropTarget = NULL;
   m_pDragData->m_pDataObject = NULL;
   m_pDragData->m_hwndSource = NULL;
   m_pDragData->m_fContainsNamespace = FALSE;

   m_pDragData->m_pDataObject = pDataObj;
   m_pDragData->m_pDataObject->AddRef();

   FORMATETC fe = {0};
   STGMEDIUM sm = {0};
   ShellDataTransfer sdt;
   fe.cfFormat = (CLIPFORMAT) RegisterClipboardFormat(CESHELL_DATA_TRANSFER);
   fe.ptd = NULL;
   fe.dwAspect = DVASPECT_CONTENT;
   fe.lindex = -1;
   fe.tymed = TYMED_HGLOBAL;
   if (SUCCEEDED(m_pDragData->m_pDataObject->GetData(&fe, &sm)) &&
       ShellDataTransfer::Unpack(sm.hGlobal, &sdt))
   {
      // Force lnk on for namespaces
      m_pDragData->m_hwndSource = sdt.m_hwndOwnerLV;
      if (ShellDataTransfer::CONTAINS_NAMESPACE_OBJECT & sdt.m_dwFlags)
      {
         m_pDragData->m_fContainsNamespace = TRUE;
      }
      sdt.Free();
   }

   // Do the enter
   m_pShellListView->DragEnter(pt);
   UpdateCurrentDropTarget(dwKeyState, pt, pdwEffect);
   hr = DragOver(dwKeyState, pt, pdwEffect);

   if (FAILED(hr))
   {
      DragLeave();
   }

   return hr;
}

STDMETHODIMP CDefShellView::DragOver(DWORD dwKeyState,
                                     POINTL pt,
                                     LPDWORD pdwEffect)
{
   ASSERT(pdwEffect);
   if (!pdwEffect)
   {
      return E_INVALIDARG;
   }

   if (!m_pDragData)
   {
      return E_UNEXPECTED;
   }

   BOOL fScrolled;

   // Test for rollover or rollout and update the drop target
   if (m_pShellListView->DragOver(pt, &fScrolled))
   {
      UpdateCurrentDropTarget(dwKeyState, pt, pdwEffect);
   }

   // Do the drag over
   if (m_pDragData->m_pCurrentDropTarget)
   {
      m_pDragData->m_pCurrentDropTarget->DragOver(dwKeyState, pt, pdwEffect);
   }
   else
   {
      *pdwEffect = DROPEFFECT_NONE;
   }

   // Override the current effect if necessary
   if (DROPEFFECT_NONE != *pdwEffect)
   {
      m_pShellListView->HiliteCurrentTarget();

      if (m_pDragData->m_fContainsNamespace)
      {
         if ((*m_pShellListView != m_pDragData->m_hwndSource) ||     // Not the exact same window
             (m_pDragData->m_fOverItem) ||                           // Over an item in the view
             ((MK_CONTROL & dwKeyState) || (MK_SHIFT & dwKeyState))) // A special key is down
         {
            *pdwEffect = DROPEFFECT_LINK;
         }
      }
      else
      {
         if (MK_CONTROL & dwKeyState)
         {
            if (MK_SHIFT & dwKeyState)
            {
               *pdwEffect = DROPEFFECT_LINK;
            }
            else
            {
               *pdwEffect = DROPEFFECT_COPY;
            }
         }
         else
         {
            if (MK_SHIFT & dwKeyState)
            {
               *pdwEffect = DROPEFFECT_MOVE;
            }
         }
      }

      if (fScrolled)
      {
         *pdwEffect |= DROPEFFECT_SCROLL;
      }
   }

   // Stash the drop effect
   m_pDragData->m_dwDropEffect = *pdwEffect;

   return NOERROR;
}

STDMETHODIMP CDefShellView::DragLeave()
{
   // Since it's possible that the user closed the window before the drop was done
   if (m_pShellListView)
   {
      m_pShellListView->DragLeave();
   }

   if (m_pDragData)
   {
      if (m_pDragData->m_pCurrentDropTarget)
      {
         m_pDragData->m_pCurrentDropTarget->DragLeave();
         m_pDragData->m_pCurrentDropTarget->Release();
      }

      if (m_pDragData->m_pDataObject)
      {
         m_pDragData->m_pDataObject->Release();
      }

      delete m_pDragData;
      m_pDragData = NULL;
   }

   return NOERROR;
}

STDMETHODIMP CDefShellView::Drop(LPDATAOBJECT pDataObj,
                                 DWORD dwKeyState,
                                 POINTL pt,
                                 LPDWORD pdwEffect)
{
   ASSERT(pDataObj);
   ASSERT(pdwEffect);
   if (!pDataObj || !pdwEffect)
   {
      return E_INVALIDARG;
   }

   if (!m_pDragData)
   {
      return E_UNEXPECTED;
   }

   if (!m_pDragData->m_pCurrentDropTarget)
   {
      return E_FAIL;
   }

   HRESULT hr = NOERROR;
   FORMATETC fe = {0};
   STGMEDIUM sm = {0};
   ShellDataTransfer sdt;

   fe.cfFormat = (CLIPFORMAT) RegisterClipboardFormat(CESHELL_DATA_TRANSFER);
   fe.ptd = NULL;
   fe.dwAspect = DVASPECT_CONTENT;
   fe.lindex = -1;
   fe.tymed = TYMED_HGLOBAL;

   hr = pDataObj->GetData(&fe, &sm);
   if (SUCCEEDED(hr) && ShellDataTransfer::Unpack(sm.hGlobal, &sdt))
   {
      if ((*m_pShellListView != sdt.m_hwndOwnerLV) ||        // The src and dest are not the exact same
          (m_pDragData->m_fOverItem) ||                      // Over an item in the current view
          (DROPEFFECT_LINK & m_pDragData->m_dwDropEffect) || // Link in the current view
          (DROPEFFECT_COPY & m_pDragData->m_dwDropEffect))   // Copy in the current view
      {
         // Do a drop onto the current drop target
         hr = m_pDragData->m_pCurrentDropTarget->Drop(pDataObj, dwKeyState, pt,
                                                      &(m_pDragData->m_dwDropEffect));
         if (SUCCEEDED(hr))
         {
            s_CurrentUndoStr = IDS_MENU_EDIT_UNDOMOVE;
         }
      }
      else
      {
         // Move the items in the listview
         m_pShellListView->DropTo(pt);
         hr = S_FALSE;
      }

      sdt.Free();
   }
   else
   {
      hr = E_FAIL;
   }

   if (FAILED(hr))
   {
      m_pDragData->m_dwDropEffect = DROPEFFECT_NONE;
   }

   *pdwEffect = m_pDragData->m_dwDropEffect;
   DragLeave();

   return hr;
}


//////////////////////////////////////////////////
// IEnumFilter

STDMETHODIMP CDefShellView::SetFilterString(LPCWSTR pszFilter)
{
   HRESULT hr;
   LPWSTR pszTemp = NULL;
   size_t cch = 0;

   if (!pszFilter)
   {
      hr = NOERROR;
      goto resetFilter;
   }

   hr = StringCchLength(pszFilter, MAX_PATH, &cch);
   if (FAILED(hr))
   {
      goto leave;
   }

   if (!m_pszFilter)
   {
      pszTemp = (LPWSTR) g_pShellMalloc->Alloc((cch+1)*sizeof(WCHAR));
   }
   else
   {
      pszTemp = (LPWSTR) g_pShellMalloc->Realloc(m_pszFilter, (cch+1)*sizeof(WCHAR));
   }

   if (!pszTemp)
   {
      hr = E_OUTOFMEMORY;
      goto leave;
   }

   hr = StringCchCopy(pszTemp, cch+1, pszFilter);
   m_pszFilter = pszTemp;

leave:
   if (FAILED(hr))
   {
resetFilter:
      if (m_pszFilter)
      {
         g_pShellMalloc->Free(m_pszFilter);
         m_pszFilter = NULL;
      }
   }

   return hr;
}


//////////////////////////////////////////////////
// IShellListView

STDMETHODIMP CDefShellView::EnablePersistentViewMode(BOOL fEnable)
{
   if (fEnable)
   {
      m_uState |= PERSIST_VIEWMODE;
   }
   else
   {
      m_uState &= ~PERSIST_VIEWMODE;
   }

   return NOERROR;
}

STDMETHODIMP CDefShellView::GetSelectedItems(LPCITEMIDLIST** paPidls, UINT* puCount)
{
   HRESULT hr;

   if (!paPidls || !puCount)
   {
      hr = E_INVALIDARG;
      goto leave;
   }

   if (!m_pShellListView)
   {
      hr = E_FAIL;
      goto leave;
   }

   hr = NOERROR;
   *paPidls = m_pShellListView->GetSelected(puCount);

leave:
    return hr;
}

STDMETHODIMP CDefShellView::SetColumnWidths(int* pWidths, int cItems)
{
   HRESULT hr;
   int i;
   short* pNewWidths = NULL;

   if (!pWidths || (UINT)cItems > 4)
   {
      hr = E_INVALIDARG;
      goto leave;
   }

   if (!m_pColumnWidths)
   {
      pNewWidths = (short*)g_pShellMalloc->Alloc(cItems*sizeof(short));
   }
   else
   {
      pNewWidths = (short*)g_pShellMalloc->Realloc(m_pColumnWidths, cItems*sizeof(short));
   }

   if (!pNewWidths)
   {
      hr = E_OUTOFMEMORY;
      goto leave;
   }

   hr = NOERROR;
   for (i = 0; i < cItems; i++)
   {
      pNewWidths[i] = (short)pWidths[i];
   }
   m_cColumns = (short)cItems;
   m_pColumnWidths = pNewWidths;

leave:
   if (FAILED(hr) && m_pColumnWidths)
   {
      g_pShellMalloc->Free(m_pColumnWidths);
      m_pColumnWidths = NULL;
      m_cColumns = 0;
   }

   return hr;
}

void CDefShellView::OnSelectionChanged()
{
	if (!(SELECTION_UPDATE_NEEDED & m_uState))
	{
		::PostMessage(m_hwnd, WM_DEFSHELLVIEW_SELECTION_UPDATED, NULL, NULL);
		m_uState |= SELECTION_UPDATE_NEEDED;
	}

}

void CDefShellView::UpdateSelectionMenu()
{
	if (SELECTION_UPDATE_NEEDED & m_uState)
	{
		HWND hwndTB;
		UINT uItemCount = 0;
		LPCITEMIDLIST * aPidls = m_pShellListView->GetSelected(&uItemCount);
		
		// update toolbar buttons
		if (SUCCEEDED(m_pTopShellBrowser->GetControlWindow(FCW_TOOLBAR, &hwndTB)))
		{
			LRESULT fState = 0;
			
			ULONG uFlags = SFGAO_CANDELETE;
			if (aPidls && SUCCEEDED(m_pShellFolder->GetAttributesOf(uItemCount, aPidls, &uFlags)))
			{
				if (SFGAO_CANDELETE & uFlags)
				{
					fState = TBSTATE_ENABLED;
				}
			}
			SendMessage(hwndTB, TB_SETSTATE, IDC_FILE_DELETE, (LPARAM)MAKELONG(fState,0));
		}
		
		UpdateStatusBar(aPidls, uItemCount, FALSE);
		
		if (aPidls)
		{
			g_pShellMalloc->Free(aPidls);
		}

		m_uState &= ~SELECTION_UPDATE_NEEDED;
	}
}
