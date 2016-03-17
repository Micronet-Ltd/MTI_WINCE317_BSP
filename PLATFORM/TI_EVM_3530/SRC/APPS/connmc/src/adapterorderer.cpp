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
#include "adapterorderer.h"
#include <buttonview.hpp>
#include <buttoncommon.hpp>
#include "list.hxx"

// Borrow the image list from connmc.cpp
extern HIMAGELIST g_himlSmall;

AdapterOrderer::AdapterOrderer(HINSTANCE hInstance, HWND hParent) :
   m_hInstance(hInstance),
   m_hParent(hParent),
   m_hDialog(NULL),
   m_wndprocButton(NULL),
   m_fVisible(FALSE)
{
   m_hInitEvt = ::CreateEvent(NULL, TRUE, FALSE, NULL);
   ASSERT(m_hInitEvt);

   m_hKillEvt = ::CreateEvent(NULL, TRUE, FALSE, NULL);
   ASSERT(m_hKillEvt);

   if (m_hInitEvt && m_hKillEvt)
   {
      // Spin off the UI thread
      m_hThread = ::CreateThread(NULL, 0, AdapterThreadProc, this, 0, NULL);
      ASSERT(m_hThread);

      // Wait for the UI thread it initalize before returning
      if (m_hThread)
         ::WaitForSingleObject(m_hInitEvt, INFINITE);
      ::CloseHandle(m_hInitEvt);
      m_hInitEvt = NULL;
   }
}

AdapterOrderer::~AdapterOrderer()
{
   if (m_hThread)
   {
      ASSERT(m_hDialog);
      ::SetEvent(m_hKillEvt);
      ::WaitForSingleObject(m_hThread, INFINITE);
      ::CloseHandle(m_hKillEvt);
      ::CloseHandle(m_hThread);
   }
}

DWORD WINAPI AdapterOrderer::AdapterThreadProc(LPVOID lpParameter)
{
   AdapterOrderer * pThis = reinterpret_cast<AdapterOrderer *>(lpParameter);
   ASSERT(pThis);
   if (pThis)
   {
      ::CreateDialogParam(pThis->m_hInstance,
                          MAKEINTRESOURCE(g_fPortrait?IDD_ADAPTER_ORDERER_P:IDD_ADAPTER_ORDERER),
                          pThis->m_hParent, AdapterOrdererProc,
                          reinterpret_cast<LPARAM>(pThis));

      // Signal that this object is ready to handle user requests
      ::SetEvent(pThis->m_hInitEvt);

      // Wait for the termiate event to be signaled from the main thread
      // it is done this way because only this thread can call DestroyWindow
      while (WAIT_OBJECT_0 != ::MsgWaitForMultipleObjects(1, &(pThis->m_hKillEvt),
                                                          FALSE, INFINITE,
                                                          QS_ALLINPUT))
      {
         MSG msg = {0};
         while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
         {
            if (!::IsDialogMessage(pThis->m_hDialog, &msg))
            {
               ::TranslateMessage(&msg);
               ::DispatchMessage(&msg);
            }
         }
      }

      ::DestroyWindow(pThis->m_hDialog);
      return 0;
   }

   return static_cast<DWORD>(-1);
}

int CALLBACK AdapterOrderer::SortCallback(LPARAM lParam1, LPARAM lParam2,
                                          LPARAM lParamSort)
{
   AOItem * pAOItem1 = reinterpret_cast<AOItem *>(lParam1);
   AOItem * pAOItem2 = reinterpret_cast<AOItem *>(lParam2);

   ASSERT(pAOItem1);
   ASSERT(pAOItem2);

   if (!pAOItem1 || !pAOItem2)
      return 0;

   if (pAOItem1->dwAdjustedRouteMetric > pAOItem2->dwAdjustedRouteMetric)
      return 1;
   else if (pAOItem1->dwAdjustedRouteMetric < pAOItem2->dwAdjustedRouteMetric)
      return -1;
   else
      return 0;
}

BOOL CALLBACK AdapterOrderer::UpDownButtonSubcass(HWND hwnd, UINT msg,
                                                  WPARAM wParam, LPARAM lParam)
{
   AdapterOrderer * pThis =
      reinterpret_cast<AdapterOrderer *>(GetWindowLong(hwnd, GWL_USERDATA));
   ASSERT(pThis);

   if (WM_PAINT == msg ||
       BM_SETSTATE == msg ||
       BM_SETCHECK == msg)
   {
      ::CallWindowProc(pThis->m_wndprocButton, hwnd, msg, wParam, lParam);

      HDC hdc = GetDC(hwnd);
      if (hdc)
      {
         UINT uIcon = 0;
         HICON hIcon = NULL;

         if (::GetDlgItem(pThis->m_hDialog, IDC_ADAPTER_UP) == hwnd)
         {
            if (::IsWindowEnabled(hwnd))
               uIcon = IDI_ADAPTER_UP_ENABLED;
            else
               uIcon = IDI_ADAPTER_UP_DISABLED;
         }
         else
         {
            if (::IsWindowEnabled(hwnd))
               uIcon = IDI_ADAPTER_DOWN_ENABLED;
            else
               uIcon = IDI_ADAPTER_DOWN_DISABLED;
         }

         hIcon = static_cast<HICON>(::LoadImage(pThis->m_hInstance,
                                                MAKEINTRESOURCE(uIcon),
                                                IMAGE_ICON, 0, 0, 0));
         RECT rc = {0};
         ::GetClientRect(hwnd, &rc);
         ::DrawIcon(hdc, (rc.right-UP_DOWN_ICON_WIDTH)/2,
                    (rc.bottom-UP_DOWN_ICON_HEIGHT)/2, hIcon);
         ::DestroyIcon(hIcon);
         ::ReleaseDC(hwnd, hdc);
      }

      return 0;
   }

   return CallWindowProc(pThis->m_wndprocButton, hwnd, msg, wParam, lParam);

}

BOOL CALLBACK AdapterOrderer::AdapterOrdererProc(HWND hwnd, UINT msg,
                                                 WPARAM wParam, LPARAM lParam)
{
   AdapterOrderer * pThis =
      reinterpret_cast<AdapterOrderer *>(GetWindowLong(hwnd, GWL_USERDATA));

   switch (msg)
   {
      case WM_INITDIALOG:
      {
         ASSERT(lParam);
         ::SetWindowLong(hwnd, GWL_USERDATA, lParam);
         reinterpret_cast<AdapterOrderer *>(lParam)->m_hDialog = hwnd;
         reinterpret_cast<AdapterOrderer *>(lParam)->init();
      }
      break;

      case WM_COMMAND:
      {
         if ((IDOK == LOWORD(wParam)) &&
             (BN_CLICKED == HIWORD(wParam)))
         {
            pThis->writeChanges();
            PostMessage(hwnd, WM_CLOSE, NULL, NULL);
         }
         else if ((IDCANCEL == LOWORD(wParam)) &&
                  (BN_CLICKED == HIWORD(wParam)))
         {                    
            PostMessage(hwnd, WM_CLOSE, NULL, NULL);
         }
         else if ((IDC_ADAPTER_UP == LOWORD(wParam)) &&
                  (BN_CLICKED == HIWORD(wParam)))
         {                    
            pThis->handleSelectionUp();
         }
         else if ((IDC_ADAPTER_DOWN == LOWORD(wParam)) &&
                  (BN_CLICKED == HIWORD(wParam)))
         {                    
            pThis->handleSelectionDown();
         }
         else
         {
            return FALSE;
         }
      }
      break;

      case WM_NOTIFY:
      {
         LPNMHDR lpnmhdr = reinterpret_cast<LPNMHDR>(lParam);
         ASSERT(lpnmhdr);

         if ((IDC_ADAPTER_LIST == lpnmhdr->idFrom) &&
             (LVN_ITEMCHANGED == lpnmhdr->code))
         {
            LPNMLISTVIEW lpnmlv = reinterpret_cast<LPNMLISTVIEW>(lParam);
            if (LVIF_STATE & lpnmlv->uChanged)
               pThis->handleSelectionChange();
         }
         else if ((IDC_ADAPTER_LIST == lpnmhdr->idFrom) &&
                  (LVN_DELETEITEM == lpnmhdr->code))
         {
            LPNMLISTVIEW lpnmlv = reinterpret_cast<LPNMLISTVIEW>(lParam);
            AdapterOrderer::AOItem * pAOItem = reinterpret_cast<AdapterOrderer::AOItem *>(lpnmlv->lParam);
            if (pAOItem)
               delete pAOItem;
         }
         else if ((IDC_ADAPTER_LIST == lpnmhdr->idFrom) &&
                  (LVN_GETDISPINFO == lpnmhdr->code))
         {
            NMLVDISPINFO * plvdi = reinterpret_cast<LV_DISPINFO *>(lParam);

            AOItem * pAOItem  = reinterpret_cast<AOItem  *>(plvdi->item.lParam);
            ASSERT(pAOItem);
            if (pAOItem && pAOItem->pLanConnInfo)
            {
               pAOItem->pLanConnInfo->getDisplayInfo(0, pThis->m_hInstance,
                                                     plvdi->item.pszText,
                                                     plvdi->item.cchTextMax,
                                                     NULL);
               ::_tcscat(plvdi->item.pszText, _T(" ("));
               ::_itot(pAOItem->dwAdjustedRouteMetric,
                       plvdi->item.pszText+::_tcslen(plvdi->item.pszText), 10);
               ::_tcscat(plvdi->item.pszText, _T(")"));
            }
         }
         else
         {
            return FALSE;
         }
      }
      break;

      case WM_CHAR:
      {
         if ((VK_ESCAPE == wParam) && (0 == lParam))
            pThis->hide();
      }
      break;

      case WM_CLOSE:
      {
         pThis->hide();
      }
      break;

      default:
      {
         return FALSE;
      }
   }

   return TRUE;
}

void AdapterOrderer::init()
{
   // Set up the adapter list
   HWND hAdapterList = ::GetDlgItem(m_hDialog, IDC_ADAPTER_LIST);
   ListView_SetImageList(hAdapterList, g_himlSmall, LVSIL_SMALL);

   LVCOLUMN lvc = {0};
   RECT rc = {0};
   GetClientRect(hAdapterList, &rc);
   lvc.mask = LVCF_FMT;
   lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
   ListView_InsertColumn(hAdapterList, 0, &lvc);
   ListView_SetExtendedListViewStyle(hAdapterList, LVS_EX_FULLROWSELECT);

   // Subclass the button to draw on them later
   LONG lGWL = ::GetWindowLong(::GetDlgItem(m_hDialog, IDC_ADAPTER_UP),
                               GWL_WNDPROC);
   m_wndprocButton = reinterpret_cast<WNDPROC>(lGWL);

   ::SetWindowLong(::GetDlgItem(m_hDialog, IDC_ADAPTER_UP), GWL_WNDPROC,
                   reinterpret_cast<LONG>(UpDownButtonSubcass));
   ::SetWindowLong(::GetDlgItem(m_hDialog, IDC_ADAPTER_UP),
                   GWL_USERDATA, reinterpret_cast<LPARAM>(this));

   ::SetWindowLong(::GetDlgItem(m_hDialog, IDC_ADAPTER_DOWN), GWL_WNDPROC,
                   reinterpret_cast<LONG>(UpDownButtonSubcass));
   ::SetWindowLong(::GetDlgItem(m_hDialog, IDC_ADAPTER_DOWN),
                   GWL_USERDATA, reinterpret_cast<LPARAM>(this));

   // Adjust the column width to be the entire control,
   // it looks more like a list box this way
   GetClientRect(hAdapterList, &rc);
   ListView_SetColumnWidth(hAdapterList, 0, rc.right);
}

void AdapterOrderer::handleSelectionChange()
{
   HWND hAdapterList = ::GetDlgItem(m_hDialog, IDC_ADAPTER_LIST);
   HWND hAdapterUp = ::GetDlgItem(m_hDialog, IDC_ADAPTER_UP);
   HWND hAdapterDown = ::GetDlgItem(m_hDialog, IDC_ADAPTER_DOWN);
   int iItem = ListView_GetNextItem(hAdapterList, -1, LVNI_SELECTED);
   int iItems = ListView_GetItemCount(hAdapterList);

   if (1 < iItems)
   {
      if (0 == iItem) // top item
      {
         ::EnableWindow(hAdapterUp, FALSE);
         ::EnableWindow(hAdapterDown, TRUE);
      }
      else if ((iItems-1) == iItem) // bottom item
      {
         ::EnableWindow(hAdapterUp, TRUE);
         ::EnableWindow(hAdapterDown, FALSE);
      }
      else if ((0 < iItem) && (iItem < (iItems-1))) // in the middle
      {
         ::EnableWindow(hAdapterUp, TRUE);
         ::EnableWindow(hAdapterDown, TRUE);
      }
      else // error or nothing selected
      {
         ::EnableWindow(hAdapterUp, FALSE);
         ::EnableWindow(hAdapterDown, FALSE);
      }
   }
   else
   {
      ::EnableWindow(hAdapterUp, FALSE);
      ::EnableWindow(hAdapterDown, FALSE);
   }

   // Refresh the buttons to get the new icons
   ::InvalidateRect(hAdapterUp, NULL, TRUE);
   ::InvalidateRect(hAdapterDown, NULL, TRUE);
   ::UpdateWindow(hAdapterUp);
   ::UpdateWindow(hAdapterDown);
}

void AdapterOrderer::handleSelectionUp()
{
   HWND hAdapterList = ::GetDlgItem(m_hDialog, IDC_ADAPTER_LIST);
   LVITEM lvi1 = {0}; // This is the item to move
   LVITEM lvi2 = {0}; // This is the item which will get shifted
   lvi1.iItem = ListView_GetNextItem(hAdapterList, -1, LVNI_SELECTED);
   lvi1.mask = LVIF_PARAM;
   lvi2.iItem = lvi1.iItem-1;
   lvi2.mask = LVIF_PARAM;

   if (-1 != lvi1.iItem)
   {
      if (ListView_GetItem(hAdapterList, &lvi1) &&
          ListView_GetItem(hAdapterList, &lvi2))
      {
         AOItem * pAOItem1 = reinterpret_cast<AOItem *>(lvi1.lParam);
         AOItem * pAOItem2 = reinterpret_cast<AOItem *>(lvi2.lParam);
         if (pAOItem1 && pAOItem2)
         {
            if (pAOItem1->dwAdjustedRouteMetric != pAOItem2->dwAdjustedRouteMetric)
            {
               DWORD dwTempAdjustedRouteMetric = pAOItem1->dwAdjustedRouteMetric;
               pAOItem1->dwAdjustedRouteMetric = pAOItem2->dwAdjustedRouteMetric;
               pAOItem2->dwAdjustedRouteMetric = dwTempAdjustedRouteMetric;
            }
            else
            {
               if (1 < pAOItem1->dwAdjustedRouteMetric)
                  pAOItem1->dwAdjustedRouteMetric--;
            }

            ListView_SortItems(hAdapterList, AdapterOrderer::SortCallback, NULL);
            handleSelectionChange();
         }
      }
   }
}

void AdapterOrderer::handleSelectionDown()
{
   HWND hAdapterList = ::GetDlgItem(m_hDialog, IDC_ADAPTER_LIST);
   LVITEM lvi1 = {0}; // This is the item to move
   LVITEM lvi2 = {0}; // This is the item which will get shifted
   lvi1.iItem = ListView_GetNextItem(hAdapterList, -1, LVNI_SELECTED);
   lvi1.mask = LVIF_PARAM;
   lvi2.iItem = lvi1.iItem+1;
   lvi2.mask = LVIF_PARAM;

   if (-1 != lvi1.iItem)
   {
      if (ListView_GetItem(hAdapterList, &lvi1) &&
          ListView_GetItem(hAdapterList, &lvi2))
      {
         AOItem * pAOItem1 = reinterpret_cast<AOItem *>(lvi1.lParam);
         AOItem * pAOItem2 = reinterpret_cast<AOItem *>(lvi2.lParam);
         if (pAOItem1 && pAOItem2)
         {
            if (pAOItem1->dwAdjustedRouteMetric != pAOItem2->dwAdjustedRouteMetric)
            {
               DWORD dwTempAdjustedRouteMetric = pAOItem1->dwAdjustedRouteMetric;
               pAOItem1->dwAdjustedRouteMetric = pAOItem2->dwAdjustedRouteMetric;
               pAOItem2->dwAdjustedRouteMetric = dwTempAdjustedRouteMetric;
            }
            else
            {
               pAOItem1->dwAdjustedRouteMetric++;
            }

            ListView_SortItems(hAdapterList, AdapterOrderer::SortCallback, NULL);
            handleSelectionChange();
         }
      }
   }
}

BOOL AdapterOrderer::addAdapter(LanConnInfo * pLanConnInfo)
{
   ASSERT(pLanConnInfo);
   ASSERT(m_hDialog);
   if (!pLanConnInfo || !m_hDialog)
      return FALSE;

   DWORD dwRouteMetric = pLanConnInfo->getDefaultGatewayRouteMetric();
   if (static_cast<DWORD>(-1) == dwRouteMetric)
      return FALSE;

   AOItem * pAOItem = new AOItem();
   if (!pAOItem)
      return FALSE;

   pAOItem->pLanConnInfo = pLanConnInfo;
   pAOItem->dwAdjustedRouteMetric = dwRouteMetric;

   HWND hAdapterList = ::GetDlgItem(m_hDialog, IDC_ADAPTER_LIST);
   LVITEM lvi = {0};
   lvi.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT;
   lvi.iItem = ListView_GetItemCount(hAdapterList);
   lvi.pszText = LPSTR_TEXTCALLBACK;
   lvi.iImage = pLanConnInfo->getIconID();
   lvi.lParam = reinterpret_cast<LPARAM>(pAOItem);
   if (-1 == ListView_InsertItem(hAdapterList, &lvi))
   {
      delete pAOItem;
      return FALSE;
   }

   return TRUE;
}

void AdapterOrderer::writeChanges()
{
   HWND hAdapterList = ::GetDlgItem(m_hDialog, IDC_ADAPTER_LIST);
   const int itemCount = ListView_GetItemCount(hAdapterList);
   LVITEM lvi = {0};
   lvi.mask = LVIF_PARAM;
   lvi.iItem = ListView_GetItemCount(hAdapterList);

   for (lvi.iItem = 0; lvi.iItem < itemCount; lvi.iItem++)
   {
      if (ListView_GetItem(hAdapterList, &lvi))
      {
         AOItem * pAOItem = reinterpret_cast<AOItem *>(lvi.lParam);
         ASSERT(pAOItem);
         LanConnInfo * pLanConnInfo = pAOItem->pLanConnInfo;
         ASSERT(pLanConnInfo);
         pLanConnInfo->setDefaultGatewayRouteMetric(pAOItem->dwAdjustedRouteMetric);
      }
   }
}

BOOL AdapterOrderer::show()
{
   if (!m_hDialog)
      return FALSE;

   if (::IsWindowVisible(m_hDialog))
   {
      ASSERT(m_fVisible);
      return TRUE;
   }

   HWND hAdapterList = ::GetDlgItem(m_hDialog, IDC_ADAPTER_LIST);
   if (!hAdapterList)
      return FALSE;

   HCURSOR hCursor = ::SetCursor(LoadCursor(NULL, IDC_WAIT));
   ListView_SetItemState(hAdapterList, 0,
                         LVIS_SELECTED | LVIS_FOCUSED,
                         LVIS_SELECTED | LVIS_FOCUSED);

   ::ShowWindow(m_hDialog, SW_SHOW);
   ::UpdateWindow(m_hDialog);
   m_fVisible = TRUE;

   if (m_hParent)
      ::EnableWindow(m_hParent, FALSE);
   ::SetCursor(hCursor);

   MSG msg = {0};
   while (m_fVisible)
   {
      if (::GetMessage(&msg, m_hParent, WM_PAINT, WM_PAINT))
         ::DispatchMessage(&msg);
   }

   if (m_hParent)
      ::EnableWindow(m_hParent, TRUE);

   return TRUE;
}

BOOL AdapterOrderer::hide()
{
   ::ShowWindow(m_hDialog, SW_HIDE);
   m_fVisible = FALSE;
   return TRUE;
}



BOOL
AdapterOrderer::IsSameAdapter(LanConnInfo *a, LanConnInfo *b)
{
   WCHAR     szNameA[512], szNameB[512];
   
    a->getDisplayInfo(0x00, 0x00, szNameA, 512, NULL);
    b->getDisplayInfo(0x00, 0x00, szNameB, 512, NULL);

    return (wcscmp(szNameA, szNameB) == 0x00);
    
}   //  IsSameAdapter()


//
//  Method determines if a given pLanConnInfo is alrady in the list view.
//
BOOL AdapterOrderer::ItemIsInListView(LanConnInfo * pTargetLanConnInfo)
{

   HWND         hAdapterList = ::GetDlgItem(m_hDialog, IDC_ADAPTER_LIST);
   const int    itemCount    = ListView_GetItemCount(hAdapterList);
   LVITEM       lvi = {0};

   lvi.mask  = LVIF_PARAM;
   lvi.iItem = ListView_GetItemCount(hAdapterList);

   for (lvi.iItem = 0; lvi.iItem < itemCount; lvi.iItem++)
   {
      if (ListView_GetItem(hAdapterList, &lvi))
      {
         AOItem * pAOItem = reinterpret_cast<AOItem *>(lvi.lParam);
         ASSERT(pAOItem);
         LanConnInfo * pLanConnInfo = pAOItem->pLanConnInfo;
         ASSERT(pLanConnInfo); 
     
         if (IsSameAdapter(pLanConnInfo, pTargetLanConnInfo))
            return TRUE;          
      }
   }

   //
   //   Get here means we did not find the item.
   //

   return FALSE;
   
}   //  ItemIsInListView()

#if DEBUG
WCHAR *
DisplayName(LanConnInfo *a)
{
    static WCHAR Name[512];

    a->getDisplayInfo(0x00, 0x00, Name, 512, NULL);  
    return Name;
}

WCHAR *
DisplayName2(LanConnInfo *a)
{
    static WCHAR Name[512];

    a->getDisplayInfo(0x00, 0x00, Name, 512, NULL);  
    return Name;
}
#endif


void AdapterOrderer::refresh()
{
    HWND        hAdapterList = ::GetDlgItem(m_hDialog, IDC_ADAPTER_LIST);
    LVITEM      lvi = {0};
    const int   itemCount    = ListView_GetItemCount(hAdapterList);

    typedef ce::list<LanConnInfo *> LAN_CONN_LIST;
    LAN_CONN_LIST   LanConnList;
    LanConnInfo     *pLanConnInfo;
    BOOL            bFound;
    
    if (!hAdapterList)
        return;

    // 
    //  If there is no existing entry, just put all in the list.
    //
    if (0 == ListView_GetItemCount(hAdapterList))
    {
        //
        //  Rebuild the list
        //
        ConnInfo * pConnInfo = LanConnInfo::FindFirst();
        
        while (pConnInfo)
        {
            addAdapter(static_cast<LanConnInfo *>(pConnInfo));
            pConnInfo = LanConnInfo::FindNext();
        }
        
        ListView_SortItems(hAdapterList, AdapterOrderer::SortCallback, NULL);        
        return;
    }        
    

    //
    //  List exists, merge list view and adapter list.
    //  Delete removed adapter and add new adapter.
    //
    //  First collect all the interfaces in the system.
    //

    pLanConnInfo = LanConnInfo::FindFirst();
    
    while (pLanConnInfo)
    {
        LanConnList.push_front(pLanConnInfo);
        pLanConnInfo = LanConnInfo::FindNext();
    }    

    //
    //  Go through the list item and see if there is anything we need to delete.
    //

    lvi.mask  = LVIF_PARAM;
    lvi.iItem = ListView_GetItemCount(hAdapterList);

    for (lvi.iItem = 0; lvi.iItem < itemCount; lvi.iItem++)
    {
        if (ListView_GetItem(hAdapterList, &lvi))
        {
             AOItem * pAOItem = reinterpret_cast<AOItem *>(lvi.lParam);
             ASSERT(pAOItem);
             LanConnInfo * pLanConnInfoTarget = pAOItem->pLanConnInfo;
             ASSERT(pLanConnInfoTarget);

             //
             // Now see if this is in our list of adapter.
             // If it is not then delete this list item.
             //

             bFound = FALSE;
             
             for (LAN_CONN_LIST::iterator it = LanConnList.begin(), itEnd = LanConnList.end();
                  it != itEnd;)
             {
                pLanConnInfo = *it;

                DEBUGMSG (0, (TEXT("Checking [%s] against incoming [%s]\r\n"),
                    DisplayName(pLanConnInfoTarget),
                    DisplayName2(pLanConnInfo)));

                if (IsSameAdapter(pLanConnInfoTarget, pLanConnInfo))
                {
                    //
                    //  Same item, get out of here..
                    //
                    bFound = TRUE;
                    break;
                }             
                it++;        
            }

            if (!bFound)
            {
                //
                //  This will trigger LVN_DELETEITEM where we delete the
                //  AOItem.
                //
                
                if (ListView_DeleteItem(hAdapterList, lvi.iItem) == TRUE)
                {
                    DEBUGMSG (0, 
                        (TEXT("### Deleting existing [%s]\r\n"),
                        DisplayName(pLanConnInfoTarget)));
                }                    
            }
        }                            
    }
 

    //
    //  Finally add the new items to the list view, delete if it already
    //  exists.
    //

    for (LAN_CONN_LIST::iterator it = LanConnList.begin(), itEnd = LanConnList.end();
         it != itEnd;)
    {
        pLanConnInfo = *it;

        if (!ItemIsInListView(pLanConnInfo))
        {
            addAdapter(pLanConnInfo);

            DEBUGMSG (0, (TEXT("--> Adding incoming [%s]\r\n"),
                DisplayName(pLanConnInfo)));
        }            
        else
        {
            DEBUGMSG (0, (TEXT("--- Deleting incoming [%s]\r\n"),
                DisplayName(pLanConnInfo)));
            delete(pLanConnInfo);
        }            
        
        it++;        
    }

    ListView_SortItems(hAdapterList, AdapterOrderer::SortCallback, NULL);
    
}

