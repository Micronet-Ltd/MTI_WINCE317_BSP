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

#include "shelllistview.h"
#include "datatrans.h"
#include "idlist.h"
#include "resource.h"

CShellListView::DragData * CShellListView::m_pDragData = NULL;

CShellListView::CShellListView() :
   m_hwndLV(NULL),
   m_dwColumnCount(0),
   m_iTarget(-1),
   m_pidlMakeEditableOnAdd(NULL)
{
}

CShellListView::~CShellListView()
{
   if (m_pidlMakeEditableOnAdd)
   {
      ILFree(m_pidlMakeEditableOnAdd);
   }

   if (m_hwndLV)
   {
      Destroy();
   }
}

BOOL CShellListView::AppendColumn(LPCTSTR psz, int width, int alignment)
{
   LVCOLUMN lvc = {0};

   lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
   lvc.fmt = alignment;
   lvc.cx = width;
   lvc.pszText = (LPTSTR) psz;
   lvc.iSubItem = m_dwColumnCount;

   if (-1 != ListView_InsertColumn(m_hwndLV, m_dwColumnCount, &lvc))
   {
      m_dwColumnCount++;
      return TRUE;
   }

   return FALSE;
}

void CShellListView::Arrange() const
{
   ListView_Arrange(m_hwndLV, LVA_DEFAULT);
}

BOOL CShellListView::BeginDrag(NMLISTVIEW * pnmListView)
{
   ASSERT(!m_pDragData);
   ASSERT(m_hwndLV);

   UINT uCount = ListView_GetSelectedCount(m_hwndLV);
   if (!pnmListView || (0 == uCount))
   {
      return FALSE;
   }

   m_pDragData = new DragData();
   if (!m_pDragData)
   {
      return FALSE;
   }
   ZeroMemory(m_pDragData, sizeof(DragData));

   HIMAGELIST himlDrag = NULL;
   POINT ptViewOrigin = {0};
   POINT ptItemPostion = {0};
   ListView_GetOrigin(m_hwndLV, &ptViewOrigin);

   if (1 < uCount)
   {
      himlDrag = ImageList_Create(32, 32, ILC_MASK, 1, 1);
      if (!himlDrag)
      {
         return FALSE;
      }

      HICON hicon = (HICON) ::LoadImage(HINST_CESHELL,
                                        MAKEINTRESOURCE(IDI_MULTIPLEFILES),
                                        IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
      if (!hicon)
      {
         ImageList_Destroy(himlDrag);
         return FALSE;
      }

      ImageList_AddIcon(himlDrag, hicon);
      ::DestroyIcon(hicon);

      ListView_GetItemPosition(m_hwndLV, pnmListView->iItem, &ptItemPostion);
      // Convert from view coords to hwnd coords
      ptItemPostion.x -= ptViewOrigin.x;
      ptItemPostion.y -= ptViewOrigin.y;
   }
   else
   {
      ASSERT(ListView_GetNextItem(m_hwndLV, -1, LVNI_SELECTED) == pnmListView->iItem);

      himlDrag = ListView_CreateDragImage(m_hwndLV, pnmListView->iItem, &ptItemPostion);
      if (!himlDrag)
      {
         return FALSE;
      }
   }

   BOOL fReturn = ImageList_BeginDrag(himlDrag, 0,
                                      pnmListView->ptAction.x-ptItemPostion.x,
                                      pnmListView->ptAction.y-ptItemPostion.y);

   if (fReturn)
   {
      // Fill in the drag masks
      m_pDragData->m_himlDragMask = ImageList_Create(32, 32, ILC_MASK, 4, 1);
      HICON hIcon = NULL;
      for (UINT i = 0; (m_pDragData->m_himlDragMask && (i < 4)); i++)
      {
         hIcon = (HICON) LoadImage(HINST_CESHELL, MAKEINTRESOURCE(IDI_DRAGBASE+i),
                                   IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
         if (hIcon)
         {
            ImageList_AddIcon(m_pDragData->m_himlDragMask, hIcon);
            DestroyIcon(hIcon);
         }
         else
         {
            ImageList_Destroy(m_pDragData->m_himlDragMask);
            m_pDragData->m_himlDragMask = NULL;
         }
      }

      // Fill in the data specific to the drag source
      m_pDragData->m_hwndSourceLV = m_hwndLV;
      m_pDragData->m_ptDragOrigin.x = ptViewOrigin.x+pnmListView->ptAction.x;
      m_pDragData->m_ptDragOrigin.y = ptViewOrigin.y+pnmListView->ptAction.y;
      m_pDragData->m_pExcludeList = new int[uCount+1]; // +1 is room for the -1 termiator
      if (m_pDragData->m_pExcludeList)
      {
         LVITEM lvi = {0};
         UINT uListItem = 0;
         lvi.mask = LVIF_PARAM;
         lvi.iItem = ListView_GetNextItem(m_hwndLV, -1, LVNI_SELECTED);
         while (uListItem < uCount)
         {
            m_pDragData->m_pExcludeList[uListItem] = lvi.iItem;
            lvi.iItem = ListView_GetNextItem(m_hwndLV, lvi.iItem, LVNI_SELECTED);
            uListItem++;
         }
         m_pDragData->m_pExcludeList[uListItem] = -1; // Terminate with a -1
      }
   }

   ImageList_Destroy(himlDrag);
   return fReturn;
}

BOOL CShellListView::Clear()
{
   return ListView_DeleteAllItems(m_hwndLV);
}

void CShellListView::DragEnter(POINTL pt)
{
   m_AutoScroll.Init();

   ImageList_DragEnter(NULL, pt.x, pt.y);

   ASSERT(-1 == m_iTarget);
}

void CShellListView::DragLeave()
{
   ImageList_DragLeave(NULL);

   if ((-1 != m_iTarget) &&
       (LVIS_DROPHILITED == ListView_GetItemState(m_hwndLV,
                                                  m_iTarget,
                                                  LVIS_DROPHILITED)))
   {
      ListView_SetItemState(m_hwndLV, m_iTarget, 0, LVIS_DROPHILITED);
      UpdateWindow(m_hwndLV);
   }

   m_iTarget = -1;
}

BOOL CShellListView::Create(HWND hwndParent, UINT fFlags)
{
   INITCOMMONCONTROLSEX iccex;
   iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
   iccex.dwICC = ICC_LISTVIEW_CLASSES;
   InitCommonControlsEx(&iccex);

   ASSERT(!m_hwndLV);
   if (m_hwndLV)
   {
      Destroy();
   }

   // Figure out the requested style
   DWORD dwStyleEx = 0;
   DWORD dwStyle = WS_CHILD | WS_TABSTOP | WS_VISIBLE |
                   LVS_EDITLABELS | LVS_SHAREIMAGELISTS;

   if (FWF_AUTOARRANGE & fFlags)
   {
      dwStyle |= LVS_AUTOARRANGE;
   }

   if (FWF_SINGLESEL & fFlags)
   {
      dwStyle |= LVS_SINGLESEL;
   }

   // ? FWF_NOSUBFOLDERS

   if (FWF_NOCLIENTEDGE & fFlags)
   {
      dwStyleEx &= ~WS_EX_CLIENTEDGE;
   }

   if (FWF_NOSCROLL & fFlags)
   {
      dwStyle |= LVS_NOSCROLL;
   }
   else
   {
      dwStyle |= (WS_HSCROLL | WS_VSCROLL);
   }

   if (FWF_ALIGNLEFT & fFlags)
   {
      dwStyle |= LVS_ALIGNLEFT;
   }

   // ? FWF_NOICONS

   if (FWF_SHOWSELALWAYS & fFlags)
   {
      dwStyle |= LVS_SHOWSELALWAYS;
   }

   // ? FWF_HIDEFILENAMES

   // If the parent window is mirrored then the listview window will inherit the mirroring flag
   // And we need the reading order to be Left to right, which is the right to left in the mirrored mode.

   if (::GetWindowLong(hwndParent, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
   {
      // This means left to right reading order because this window will be mirrored.
      dwStyleEx |= WS_EX_RTLREADING;
   }

   m_hwndLV = CreateWindowEx(dwStyleEx, WC_LISTVIEW, NULL, dwStyle, 0, 0,
                             0, 0, hwndParent, NULL, HINST_CESHELL, NULL);
   if (m_hwndLV)
   {
      SHFILEINFO sfi = {0};
      HIMAGELIST himlLarge = NULL;
      HIMAGELIST himlSmall = NULL;

      himlLarge = (HIMAGELIST) SHGetFileInfo(TEXT(""), 0, &sfi, sizeof(SHFILEINFO),
                                             SHGFI_SYSICONINDEX | SHGFI_LARGEICON |
                                             SHGFI_USEFILEATTRIBUTES);
      if (himlLarge)
      {
         ListView_SetImageList(m_hwndLV, himlLarge, LVSIL_NORMAL);
      }

      himlSmall = (HIMAGELIST) SHGetFileInfo(TEXT(""), 0, &sfi, sizeof(SHFILEINFO),
                                             SHGFI_SYSICONINDEX | SHGFI_SMALLICON |
                                             SHGFI_USEFILEATTRIBUTES);
      if (himlSmall)
      {
         ListView_SetImageList(m_hwndLV, himlSmall, LVSIL_SMALL);
      }
   }

   return (m_hwndLV ? TRUE : FALSE);
}

void CShellListView::Destroy()
{
   ASSERT(!m_pDragData);
   if (m_pDragData)
   {
      if (m_pDragData->m_himlDragMask)
      {
         ImageList_Destroy(m_pDragData->m_himlDragMask);
      }

      if (m_pDragData->m_pExcludeList)
      {
         delete [] m_pDragData->m_pExcludeList;
      }

      delete m_pDragData;
      m_pDragData = NULL;
   }

   if (m_hwndLV)
   {
      ::DestroyWindow(m_hwndLV);
      m_hwndLV = NULL;
   }
}


BOOL CShellListView::DragOver(POINTL pt, BOOL * pfScrolled)
{
   LVHITTESTINFO lvhti = {0};
   LVITEM lvi = {0};
   BOOL fSelectionChanged = FALSE;
   BOOL fScrolled = m_AutoScroll.AutoScroll(m_hwndLV, &pt);

   if (pfScrolled)
      *pfScrolled = fScrolled;

   lvhti.pt.x = pt.x;
   lvhti.pt.y = pt.y;
   ::ScreenToClient(m_hwndLV, &(lvhti.pt));
   lvi.iItem = ListView_HitTest(m_hwndLV, &lvhti);

/*
   // Override default behavior if over a header (-2 means over a header)
   HWND hwndHdr = ListView_GetHeader(pThis->m_ed.hwndListView);
   if ((IsWindowVisible(hwndHdr)) &&
      (hwndHdr == ChildWindowFromPoint(pThis->m_ed.hwndListView, dpt)))
   {
      iTarget = TARGET_IS_HEADER;
   }
*/

   ImageList_DragMove(pt.x, pt.y);

   if (m_iTarget != lvi.iItem)
   {
      if ((-1 != m_iTarget) &&
          (LVIS_DROPHILITED == ListView_GetItemState(m_hwndLV,
                                                     m_iTarget,
                                                     LVIS_DROPHILITED)))
      {
         ImageList_DragLeave(NULL);
         ListView_SetItemState(m_hwndLV, m_iTarget, 0, LVIS_DROPHILITED);
         UpdateWindow(m_hwndLV);
         ImageList_DragEnter(NULL, pt.x, pt.y);
      }

      // Set the new drop target but unset it if it is in the exclude list
      m_iTarget = lvi.iItem;
      if (m_pDragData && (m_hwndLV == m_pDragData->m_hwndSourceLV))
      {
         int iListItem = 0;
         while (-1 != m_pDragData->m_pExcludeList[iListItem])
         {
            // The item is in the exclude list
            if (lvi.iItem == m_pDragData->m_pExcludeList[iListItem])
            {
               m_iTarget = -1;
               break;
            }
            iListItem++;
         }   
      }

      fSelectionChanged = TRUE;
   }

   return fSelectionChanged;
}

void CShellListView::DropTo(POINTL pt)
{
   POINT ptViewOrigin = {0};
   POINT ptClient = { pt.x, pt.y };
   ::ScreenToClient(m_hwndLV, &ptClient);
   ListView_GetOrigin(m_hwndLV, &ptViewOrigin);

   LONG dx = (ptViewOrigin.x+ptClient.x);
   LONG dy = (ptViewOrigin.y+ptClient.y);
   if (m_pDragData && (m_hwndLV == m_pDragData->m_hwndSourceLV))
   {
      dx -= m_pDragData->m_ptDragOrigin.x;
      dy -= m_pDragData->m_ptDragOrigin.y;
   }

   POINTL ptItemPos = {0};
   LVITEM lvi = {0};
   lvi.mask = LVIF_PARAM;
   lvi.iItem = ListView_GetNextItem(m_hwndLV, -1, LVNI_SELECTED);
   while (-1 != lvi.iItem)
   {
      if (ListView_GetItemPosition(m_hwndLV, lvi.iItem, &ptItemPos))
      {
         ListView_SetItemPosition(m_hwndLV, lvi.iItem,
                                  ptItemPos.x+dx, ptItemPos.y+dy);
      }

      lvi.iItem = ListView_GetNextItem(m_hwndLV, lvi.iItem, LVNI_SELECTED);
   }
}

void CShellListView::EndDrag()
{
   ASSERT(-1 == m_iTarget);
   if (-1 != m_iTarget)
      DragLeave();

   ImageList_EndDrag();

   // Clean up the global drag data
   if (m_pDragData)
   {
      if (m_pDragData->m_himlDragMask)
      {
         ImageList_Destroy(m_pDragData->m_himlDragMask);
      }

      if (m_pDragData->m_pExcludeList)
      {
         delete [] m_pDragData->m_pExcludeList;
      }

      delete m_pDragData;
      m_pDragData = NULL;
   }
}

int CShellListView::FindItemFromPidl(LPCITEMIDLIST pidl) const
{
   ASSERT(pidl);
   if (!pidl)
   {
      return -1;
   }

   int iItemCount = ListView_GetItemCount(m_hwndLV);
   int iItem = -1;
   LVITEM lvItem = {0};
   lvItem.mask = LVIF_PARAM;
   LPITEMIDLIST pidlCmp = NULL;

   if (0 == iItemCount)
   {
      return -1;
   }

   while ((lvItem.iItem < iItemCount) && (ListView_GetItem(m_hwndLV, &lvItem)))
   {
      pidlCmp = (LPITEMIDLIST) lvItem.lParam;
      if (ILIsEqualParsingNames(pidl, pidlCmp))
      {
         iItem = lvItem.iItem;
         break;
      }

      lvItem.iItem++;
   }

   return iItem;
}

LPARAM CShellListView::GetContextMenuPoint() const
{
   POINT pt = {0};
   int iItem = ListView_GetNextItem(m_hwndLV, -1, LVNI_SELECTED);

   if (-1 != iItem)
   {
      RECT rc = {0};
      if (ListView_GetItemRect(m_hwndLV, iItem, &rc, LVIR_ICON))
      {
         pt.x = (rc.left+rc.right)/2;
         pt.y = (rc.top+rc.bottom)/2;;
      }
   }

   ::MapWindowPoints(m_hwndLV, NULL, &pt, 1);
   return MAKELPARAM(pt.x, pt.y);
}

LPCITEMIDLIST CShellListView::GetItemPidl(int iItem) const
{
   LPCITEMIDLIST pidl = NULL;
   LVITEM lvItem = {0};
   lvItem.mask = LVIF_PARAM;
   lvItem.iItem = iItem;

   if ((-1 != lvItem.iItem) && ListView_GetItem(m_hwndLV, &lvItem))
      pidl = (LPCITEMIDLIST) lvItem.lParam;

   return pidl;
}

LPCITEMIDLIST CShellListView::GetItemPidl(LPCITEMIDLIST pidlItem) const
{
   LPCITEMIDLIST pidl = NULL;
   LVITEM lvItem = {0};
   lvItem.mask = LVIF_PARAM;
   lvItem.iItem = FindItemFromPidl(pidlItem);

   if ((-1 != lvItem.iItem) && ListView_GetItem(m_hwndLV, &lvItem))
   {
      pidl = (LPCITEMIDLIST) lvItem.lParam;
   }

   return pidl;
}

LPCITEMIDLIST CShellListView::GetPidlAt(POINTL pt) const
{
   LVHITTESTINFO lvhti = {0};
   LVITEM lvi = {0};
   LPCITEMIDLIST pidl = NULL;

   lvhti.pt.x = pt.x;
   lvhti.pt.y = pt.y;
   ::ScreenToClient(m_hwndLV, &(lvhti.pt));
   lvi.iItem = ListView_HitTest(m_hwndLV, &lvhti);

   // If the item is in the exclude list just return a NULL
   if (m_pDragData && (m_hwndLV == m_pDragData->m_hwndSourceLV))
   {
      int iListItem = 0;
      while (-1 != m_pDragData->m_pExcludeList[iListItem])
      {
         // The item is in the exclude list
         if (lvi.iItem == m_pDragData->m_pExcludeList[iListItem])
         {
            lvi.iItem = -1;
            break;
         }
         iListItem++;
      }   
   }

   if (-1 != lvi.iItem)
   {
      lvi.mask = LVIF_PARAM;
      if (ListView_GetItem(m_hwndLV, &lvi))
      {
         pidl = (LPCITEMIDLIST) lvi.lParam;
      }
   }

   return pidl;
}

LPCITEMIDLIST * CShellListView::GetSelected(UINT * puCount) const
{
   LPITEMIDLIST * aPidls = NULL;
   UINT uCount = ListView_GetSelectedCount(m_hwndLV);

   if (uCount)
   {
      aPidls = (LPITEMIDLIST *) g_pShellMalloc->Alloc(uCount * sizeof(LPITEMIDLIST));
      if (aPidls)
      {
         LPITEMIDLIST * ppidl = &aPidls[uCount-1];
         LVITEM lvi = {0};
         lvi.mask = LVIF_PARAM;
         lvi.iItem = ListView_GetNextItem(m_hwndLV, -1, LVNI_SELECTED);
         while (-1 != lvi.iItem)
         {
            if (ListView_GetItem(m_hwndLV, &lvi))
            {
               *ppidl = (LPITEMIDLIST) lvi.lParam;
            }

            ppidl--;
            lvi.iItem = ListView_GetNextItem(m_hwndLV, lvi.iItem, LVNI_SELECTED);
         }
      }
      else
      {
         uCount = 0;
      }
   }

   if (puCount)
   {
      *puCount = uCount;
   }

   return (LPCITEMIDLIST *) aPidls;
}

BOOL CShellListView::GiveFeedback(DWORD dwEffect)
{
   if (!m_pDragData || !m_pDragData->m_himlDragMask)
   {
      return FALSE;
   }

   int iIcon;
   if (DROPEFFECT_MOVE == (dwEffect & DROPEFFECT_MOVE))
   {
      iIcon = IDI_DRAGMOVE-IDI_DRAGBASE;
   }
   else if (DROPEFFECT_COPY == (dwEffect & DROPEFFECT_COPY))
   {
      iIcon = IDI_DRAGCOPY-IDI_DRAGBASE;
   }
   else if (DROPEFFECT_LINK == (dwEffect & DROPEFFECT_LINK))
   {
      iIcon = IDI_DRAGLINK-IDI_DRAGBASE;
   }
   else
   {
      iIcon = IDI_DRAGNONE-IDI_DRAGBASE;
   }

   ImageList_SetDragCursorImage(m_pDragData->m_himlDragMask, iIcon, 0, 0);

   return TRUE;
}

void CShellListView::HandleEscape()
{
   HWND hwndEdit = ListView_GetEditControl(m_hwndLV);
   ::SendMessage((hwndEdit ? hwndEdit : m_hwndLV), WM_KEYDOWN, VK_ESCAPE, 0);
}

void CShellListView::HiliteCurrentTarget()
{
   if ((-1 != m_iTarget) &&
       (LVIS_DROPHILITED != ListView_GetItemState(m_hwndLV,
                                                  m_iTarget,
                                                  LVIS_DROPHILITED)))
   {
      POINT pt = {0};
      if (GetCursorPos(&pt))
      {
         ImageList_DragLeave(NULL);
         ListView_SetItemState(m_hwndLV, m_iTarget,
                               LVIS_DROPHILITED, LVIS_DROPHILITED);
         UpdateWindow(m_hwndLV);
         ImageList_DragEnter(NULL, pt.x, pt.y);
      }
   }
}

BOOL CShellListView::IsAutoArrangeEnabled() const
{
   return (LVS_AUTOARRANGE & ::GetWindowLong(m_hwndLV, GWL_STYLE));
}

void CShellListView::InvalidateItemColumn(LPCITEMIDLIST pidl, int iColumn)
{
   LVITEM lvItem = {0};
   lvItem.iItem = FindItemFromPidl(pidl);
   if (-1 != lvItem.iItem)
   {
      RECT rc = {0};
      BOOL fInvalidate = FALSE;

      if (0 == iColumn)
      {
         // This will redraw the whole item, there is no way to get the listview
         // to recalc without redrawing the whole thing
         ListView_Update(m_hwndLV, lvItem.iItem);

         // Do this to force update of the text and icon
         lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
         lvItem.pszText = LPSTR_TEXTCALLBACK;
         lvItem.iImage = I_IMAGECALLBACK;
         ListView_SetItem(m_hwndLV, &lvItem);

         fInvalidate = ListView_GetItemRect(m_hwndLV, lvItem.iItem, &rc,
                                            LVIR_SELECTBOUNDS);
      }
      else
      {
         // Do this to force update of the text
         lvItem.iSubItem = iColumn;
         lvItem.mask = LVIF_TEXT;
         lvItem.pszText = LPSTR_TEXTCALLBACK;
         ListView_SetItem(m_hwndLV, &lvItem);

         fInvalidate = ListView_GetSubItemRect(m_hwndLV, lvItem.iItem, iColumn,
                                               LVIR_BOUNDS, &rc);
      }

      if (fInvalidate)
      {
         ::InvalidateRect(m_hwndLV, &rc, FALSE);
      }
   }
}

void CShellListView::MakeEditableOnAdd(LPCITEMIDLIST pidl)
{
   if (m_pidlMakeEditableOnAdd)
   {
      ILFree(m_pidlMakeEditableOnAdd);
   }

   m_pidlMakeEditableOnAdd = ILCopy(pidl, IL_ALL);
}

void CShellListView::OnSetFocus()
{
   int index = ListView_GetSelectionMark(m_hwndLV);

   if (index == -1)
   {
      LVITEM lvItem = {0};
      lvItem.mask = LVIF_STATE;
      lvItem.stateMask = LVIS_FOCUSED;
      lvItem.state= 1;

      ListView_SetItem(m_hwndLV, &lvItem);
   }
   
   SetFocus(m_hwndLV);
}

BOOL CShellListView::Populate(LPENUMIDLIST pEnumIDList)
{
   ASSERT(pEnumIDList);
   if (!pEnumIDList)
   {
      return FALSE;
   }

   BOOL fReturn = TRUE;
   DWORD dwFetched = 0;
   LPITEMIDLIST pidl = NULL;
   LVITEM lvItem = {0};
   MSG msg = {0};
   HCURSOR hCursor = ::SetCursor(LoadCursor(NULL, IDC_WAIT));

   lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
   lvItem.pszText = LPSTR_TEXTCALLBACK;
   lvItem.iImage = I_IMAGECALLBACK;
   lvItem.iItem = -1;

   while ((S_OK == pEnumIDList->Next(1, &pidl, &dwFetched)) && dwFetched)
   {
      fReturn = ReflectAdd(pidl);
      ILFree(pidl);

      if (::PeekMessage(&msg, NULL, WM_CLOSE, WM_CLOSE, PM_NOREMOVE))
      {
         fReturn = FALSE;
      }

      if (!fReturn)
      {
         break;
      }

      // Don't lag the visual input
      if (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_REMOVE) ||
          ::PeekMessage(&msg, NULL, WM_SIZE, WM_SIZE, PM_REMOVE) ||
          ::PeekMessage(&msg, NULL, WM_MOVE, WM_MOVE, PM_REMOVE))
      {
         ::TranslateMessage(&msg);
         ::DispatchMessage(&msg);
      }
   }

   if (!fReturn)
   {
      ListView_DeleteAllItems(m_hwndLV);
   }

   ::SetCursor(hCursor);

   return fReturn;
}

BOOL CShellListView::ReflectAdd(LPCITEMIDLIST pidl)
{
   ASSERT(pidl);
   if (!pidl)
   {
      return FALSE;
   }

   BOOL fAdded = FALSE;

   LVITEM lvItem = {0};
   lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
   lvItem.pszText = LPSTR_TEXTCALLBACK;
   lvItem.iImage = I_IMAGECALLBACK;
   lvItem.iItem = ListView_GetItemCount(m_hwndLV);
   lvItem.lParam = (LPARAM) ILCopy(pidl, 1);

   // If the item is cut onto the clipboard show that here
   if (CShellClipboard::ShouldHiliteAsCut(m_hwndLV, pidl))
   {
      lvItem.mask |= LVIF_STATE;
      lvItem.stateMask = LVIS_CUT;
      lvItem.state = LVIS_CUT; 
   }

   lvItem.iItem = ListView_InsertItem(m_hwndLV, &lvItem);
   if (-1 != lvItem.iItem)
   {
      fAdded = TRUE;
   }

   if ((-1 != lvItem.iItem) && m_pidlMakeEditableOnAdd)
   {
      if (ILIsEqualParsingNames(m_pidlMakeEditableOnAdd, pidl))
      {
         ListView_EditLabel(m_hwndLV, lvItem.iItem);
         ILFree(m_pidlMakeEditableOnAdd);
         m_pidlMakeEditableOnAdd = NULL;
      }
   }

   return fAdded;
}

BOOL CShellListView::ReflectRemove(LPCITEMIDLIST pidl)
{
   ASSERT(pidl);
   if (!pidl)
   {
      return FALSE;
   }

   BOOL fRemoved = FALSE;

   int iItem = FindItemFromPidl(pidl);
   if (-1 != iItem)
   {
      if (ListView_DeleteItem(m_hwndLV, iItem))
      {
         fRemoved = TRUE;
      }
   }

   return fRemoved;
}

void CShellListView::Resize(DWORD width, DWORD height)
{
   ::MoveWindow(m_hwndLV, 0, 0, width, height, TRUE);
}

void CShellListView::RenameSelected()
{
   // Don't allow rename on more than one item
   if (1 < ListView_GetSelectedCount(m_hwndLV))
   {
      return;
   }

   // Don't allow rename if we're already renaming
   if (ListView_GetEditControl(m_hwndLV))
   {
      return;
   }

   int iItem = ListView_GetNextItem(m_hwndLV, -1, LVNI_SELECTED);
   ASSERT(-1 != iItem);
   ListView_EditLabel(m_hwndLV, iItem);
}

void CShellListView::SelectAll()
{
   // REVIEW: should we just deny select all while editing?
   if (ListView_GetEditControl(m_hwndLV))
   {
      SetFocus(m_hwndLV);
   }

   ListView_SetItemState(m_hwndLV, -1, LVIS_SELECTED, LVIS_SELECTED);
}

void CShellListView::SetColors(COLORREF rgbBkColor, COLORREF rgbTextColor, COLORREF rgbTextBkColor)
{
   ListView_SetBkColor(m_hwndLV, rgbBkColor);
   ListView_SetTextColor(m_hwndLV, rgbTextColor);
   ListView_SetTextBkColor(m_hwndLV, rgbTextBkColor);
}

void CShellListView::SetRedraw(BOOL fRedraw)
{
   ::SendMessage(m_hwndLV, WM_SETREDRAW, fRedraw, 0);
}

UINT CShellListView::SetViewMode(UINT ViewMode)
{
   UINT newViewMode;

   DWORD dwStyle = ::GetWindowLong(m_hwndLV, GWL_STYLE);
   dwStyle &= ~LVS_TYPEMASK;

   switch (ViewMode)
   {
      case FVM_ICON:
         dwStyle |= LVS_ICON;
         newViewMode = FVM_ICON;
      break;

      case FVM_SMALLICON:
         dwStyle |= LVS_SMALLICON;
         newViewMode = FVM_SMALLICON;
      break;

      case FVM_LIST:
         dwStyle |= LVS_LIST;
         newViewMode = FVM_LIST;
      break;

      case FVM_DETAILS:
         dwStyle |= LVS_REPORT;
         newViewMode = FVM_DETAILS;
      break;

      default:
         dwStyle |= LVS_ICON;
         newViewMode = FVM_ICON; 
   }

   ::SetWindowLong(m_hwndLV, GWL_STYLE, dwStyle);
   return newViewMode;
}

void CShellListView::ShowSelectedAsCut()
{
   int iItem = ListView_GetNextItem(m_hwndLV, -1, LVNI_SELECTED);
   while (-1 != iItem)
   {
      ListView_SetItemState(m_hwndLV, iItem, LVIS_CUT, LVIS_CUT);
      iItem = ListView_GetNextItem(m_hwndLV, iItem, LVNI_SELECTED);
   }
}

void CShellListView::SnapToGrid() const
{
   ListView_Arrange(m_hwndLV, LVA_SNAPTOGRID);
}

void CShellListView::Sort(PFNLVCOMPARE pfnCompare, LPARAM lParamSort)
{
   if (!pfnCompare)
   {
      return;
   }

   HCURSOR hCursor = ::SetCursor(LoadCursor(NULL, IDC_WAIT));
   ListView_SortItems(m_hwndLV, pfnCompare, lParamSort);
   ::SetCursor(hCursor);
}

void CShellListView::ToggleAutoArrange(BOOL fAutoArrange)
{
   DWORD dwStyle = ::GetWindowLong(m_hwndLV, GWL_STYLE);

   if (fAutoArrange)
   {
      dwStyle |= LVS_AUTOARRANGE;
   }
   else
   {
      dwStyle &= ~LVS_AUTOARRANGE;
   }

   ::SetWindowLong(m_hwndLV, GWL_STYLE, dwStyle);
}

BOOL CShellListView::UpdateItemPidl(LPCITEMIDLIST pidlOld, LPCITEMIDLIST pidlNew)
{
   ASSERT(pidlOld);
   ASSERT(pidlNew);
   if (!pidlOld || !pidlNew)
   {
      return FALSE;
   }

   BOOL fUpdated = FALSE;
   LVITEM lvItem = {0};

   lvItem.mask = LVIF_PARAM;
   lvItem.iItem = FindItemFromPidl(pidlOld);
   if ((-1 != lvItem.iItem) && ListView_GetItem(m_hwndLV, &lvItem))
   {
      ILFree((LPITEMIDLIST) lvItem.lParam);
      lvItem.lParam = (LPARAM) ILCopy(pidlNew, 1);
      if (ListView_SetItem(m_hwndLV, &lvItem))
      {
         fUpdated = TRUE;
      }
   }

   return fUpdated;
}

const int AutoScroll_t::s_ScrollInterval = 250;
const int AutoScroll_t::s_MinScrollVelocity = 20;

void AutoScroll_t::Init()
{
   m_IdxNextSample = 0;
   m_TimeLastScroll = 0;
}

bool AutoScroll_t::AddSample(const POINTL * ppt, DWORD dwTime)
{
   m_PointSamples[m_IdxNextSample % s_CntSamples] = *ppt;
   m_TimeSamples[m_IdxNextSample % s_CntSamples] = dwTime;
   m_IdxNextSample++;
   return m_IdxNextSample >= s_CntSamples;
}

int AutoScroll_t::CurrentVelocity()
{
   int i;
   int iStart;
   int iNext;
   int dx;
   int dy;
   int Distance = 0;
   DWORD Time = 1;      //   Avoid divide by zero

   i = iStart = m_IdxNextSample % s_CntSamples;

   do
   {
      iNext = (i + 1) % s_CntSamples;

      dx = abs(m_PointSamples[i].x - m_PointSamples[iNext].x);
      dy = abs(m_PointSamples[i].y - m_PointSamples[iNext].y);
      Distance += (dx + dy);
      Time += abs(m_TimeSamples[i] - m_TimeSamples[iNext]);

      i = iNext;

   } while (i != iStart);

   // scale this so we don't loose accuracy
   return(Distance * 1024) / Time;
}

bool AutoScroll_t::CanScroll(HWND hwnd, Direction_t Direction)
{
   bool ScrollOk = false;
   SCROLLINFO si;
   int Code;

   if ( ( Direction == s_Up ) ||
       ( Direction == s_Down ) )
   {
      Code = SB_VERT;
   }
   else
   {
      Code = SB_HORZ;
   }

   si.cbSize = sizeof(SCROLLINFO);
   si.fMask = SIF_ALL;
   GetScrollInfo(hwnd, Code, &si);

   if ( ( Direction == s_Down ) ||
       ( Direction == s_Right ) )
   {
      if ( si.nPage )
      {
         si.nMax -= si.nPage - 1;
      }
      ScrollOk = si.nPos < si.nMax;
   }
   else
   {
      ScrollOk = si.nPos > si.nMin;
   }

   return ScrollOk;
}

void AutoScroll_t::DragScrollDirection(HWND hwnd, const POINTL * ppt,
                                       bool * pUp, bool * pDown,
                                       bool * pLeft, bool * pRight)
{
   RECT rc;
   RECT rcOuter;
   DWORD dwStyle;

   GetClientRect(hwnd, &rc);
   dwStyle = GetWindowLong(hwnd, GWL_STYLE);
   if ( dwStyle & WS_HSCROLL )
   {
      rc.bottom -= GetSystemMetrics(SM_CYHSCROLL);
   }

   if ( dwStyle & WS_VSCROLL )
   {
      rc.right -= GetSystemMetrics(SM_CXVSCROLL);
   }

   //   The explorer forwards us drag/drop things outside of our client area
   //   so we need to explictly test for that before we do things
   rcOuter = rc;
   InflateRect(&rcOuter, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON));

   InflateRect(&rc, -GetSystemMetrics(SM_CXICON), -GetSystemMetrics(SM_CYICON));

   POINT pt = { ppt->x, ppt->y };
   if ( !PtInRect(&rc, pt) && PtInRect(&rcOuter, pt) )
   {
      if ( dwStyle & WS_HSCROLL )
      {
         if ( ppt -> x < rc.left )
         {
            *pLeft = CanScroll(hwnd, s_Left);
         }
         else if ( ppt->x > rc.right )
         {
            *pRight = CanScroll(hwnd, s_Right);
         }
      }

      if ( dwStyle & WS_VSCROLL )
      {
         if ( ppt->y < rc.top )
         {
            *pUp = CanScroll(hwnd, s_Up);
         }
         else if (ppt->y > rc.bottom)
         {
            *pDown = CanScroll(hwnd, s_Down);
         }
      }
   }

   return;
}

bool AutoScroll_t::AutoScroll(HWND hwnd, const POINTL * pptNow)
{
   DWORD dwTimeNow = GetTickCount();
   bool Up = false;
   bool Down = false;
   bool Left = false;
   bool Right = false;

   if ( AddSample(pptNow, dwTimeNow) )
   {
      if ( CurrentVelocity() <= s_MinScrollVelocity )
      {
         if ( (dwTimeNow - m_TimeLastScroll) >= s_ScrollInterval )
         {
            DragScrollDirection(hwnd, pptNow, &Up, &Down, &Left, &Right);
            if ( Up | Down | Left | Right )
            {
               ImageList_DragShowNolock(FALSE);

               if ( Up || Down )
               {
                  FORWARD_WM_VSCROLL(
                     hwnd,
                     NULL,
                     Up ? SB_LINEUP : SB_LINEDOWN,
                     1,
                     SendMessage
                     );
               }

               if ( Left || Right )
               {
                  FORWARD_WM_HSCROLL(
                     hwnd,
                     NULL,
                     Left ? SB_LINELEFT : SB_LINERIGHT,
                     1,
                     SendMessage
                     );
               }

               ImageList_DragShowNolock(TRUE);
               m_TimeLastScroll = dwTimeNow;
            }
         }
      }
   }
   else
   {
      DragScrollDirection(hwnd, pptNow, &Up, &Down, &Left, &Right);
   }

   return Up | Down | Left | Right;
}

