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
#include "connmc.h"
#include "adapterorderer.h"
#include ".\ipaddr.h"
#include <nuiouser.h>
#include <msgqueue.h>

// Sets up the button maps for g_hCommandBar
const static TBBUTTON g_tbLandscape[] =
{
   {0,                       0,                  TBSTATE_ENABLED, TBSTYLE_SEP,        0, 0, 0, -1},
   {STD_DELETE,              ID_FILE_DELETE,     TBSTATE_ENABLED, TBSTYLE_BUTTON,     0, 0, 0, -1},
   {STD_PROPERTIES,          ID_FILE_PROPERTIES, TBSTATE_ENABLED, TBSTYLE_BUTTON,     0, 0, 0, -1},
   {0,                       0,                  TBSTATE_ENABLED, TBSTYLE_SEP,        0, 0, 0, -1},
   {VIEW_LARGEICONS+STD_MAX, ID_VIEW_LARGEICON,  TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, -1},
   {VIEW_SMALLICONS+STD_MAX, ID_VIEW_SMALLICON,  TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, -1},
   {VIEW_DETAILS+STD_MAX,    ID_VIEW_DETAILS,    TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, -1}
};

const static TBBUTTON g_tbPortrait[] =
{
   {0,                       0,                  TBSTATE_ENABLED, TBSTYLE_SEP,        0, 0, 0, -1},
   {STD_MAX,                 ID_DEFAULT_ACTION,  TBSTATE_ENABLED, TBSTYLE_BUTTON,     0, 0, 0, -1},
   {0,                       0,                  TBSTATE_ENABLED, TBSTYLE_SEP,        0, 0, 0, -1},
   {STD_DELETE,              ID_FILE_DELETE,     TBSTATE_ENABLED, TBSTYLE_BUTTON,     0, 0, 0, -1},
   {STD_PROPERTIES,          ID_FILE_PROPERTIES, TBSTATE_ENABLED, TBSTYLE_BUTTON,     0, 0, 0, -1}
};

const TCHAR g_szBadCharList[] = { _T('\\'), _T('/'), _T(':'),
                                  _T('*'), _T('?'), _T('\"'),
                                  _T('<'), _T('>'), _T('|') };


// Global data
BOOL               g_fPortrait = FALSE;
HINSTANCE          g_hInstance = NULL;
HWND               g_hMainWindow = NULL;
HWND               g_hCommandBar = NULL;
HWND               g_hListView = NULL;
HIMAGELIST         g_himlLarge = NULL; // image list for icon view
HIMAGELIST         g_himlSmall = NULL; // image list for other views
CRITICAL_SECTION   g_hRefreshCS = {0}; // Locks the refresh of hte listview
HANDLE             g_hNdisAccess = INVALID_HANDLE_VALUE; // Used for ndis notify framework
HANDLE             g_hNdisNotifyMsgQueue = NULL; // Used for ndis notify framework
HANDLE             g_hNdisNotifyThread = NULL; // Used for ndis notify framework
HANDLE             g_hNdisNotifyTermiateEvent = NULL; // Used for ndis notify framework
AdapterOrderer *   g_pAdapterOrderer = NULL; // The UI used to orderer adapters
BOOL               g_bPropertiesDialogActive = FALSE;
BOOL               g_bDeferredDeviceChangeNotification = FALSE;

#ifdef USE_SIP
HINSTANCE          g_hSipLib = NULL;
LPFNSIP            g_pfnSipGetInfo = NULL;
LPFNSIP            g_pfnSipSetInfo = NULL;
LPFNSIPSTATUS      g_pfnSipStatus = NULL;
#endif
BOOL               g_fInRename = FALSE;

DECLARE_COMMCTRL_TABLE; // Necessary for common control

// Global function definitions
LRESULT CALLBACK ConnMCWndProc(HWND hwnd, UINT msg,
                               WPARAM wParam, LPARAM lParam);
BOOL ConnMCHandleNotify(NMLISTVIEW * plv);
BOOL ConnMCHandleCommand(WORD code);
BOOL CreateConnMCWindow(TCHAR * pszTitle, HINSTANCE hInstance, int nCmdShow);
BOOL CreateConnMCCommandBar();
BOOL CreateConnMCListView();
BOOL RefreshListView();
void RefreshMainMenu(HMENU hMenu);
void RefreshViewMenu(DWORD viewType);
void ShowContextMenu();
#ifdef USE_SIP
void PositionSIP(int nSipState);
#endif
int ConnMCMessageBox(UINT idBody, UINT idTitle, UINT flags);
BOOL LaunchNdisNotificationThread();
DWORD NdisNotificationThreadProc(LPVOID lpVoid);
void CleanupNdisNotificationThread();
void ParseConnMCCmdLine(LPWSTR pszCmdLine);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev,
                   LPWSTR pszCmdLine, int nCmdShow)
{
   TCHAR szTitle[128];
   MSG msg = {0};
   HACCEL hAccelTable = NULL;

   g_hInstance = hInstance;

   if (GetSystemMetrics(SM_CXSCREEN) < 480)
      g_fPortrait = TRUE;

#ifdef USE_SIP
   if((g_hSipLib = LoadLibrary(_T("coredll.dll"))) &&
      (g_pfnSipStatus = (LPFNSIPSTATUS)GetProcAddress(g_hSipLib, _T("SipStatus"))) &&
      (g_pfnSipGetInfo = (LPFNSIP)GetProcAddress(g_hSipLib, _T("SipGetInfo"))) &&
      (g_pfnSipSetInfo = (LPFNSIP)GetProcAddress(g_hSipLib, _T("SipSetInfo"))) &&
      (SIP_STATUS_AVAILABLE == g_pfnSipStatus()))
   {
      DEBUGMSG(1, (L"ConnMC: Using SIP!\r\n"));
   }
   else
   {
      g_pfnSipStatus = NULL;
      g_pfnSipGetInfo = NULL;
      g_pfnSipSetInfo = NULL;
      if(g_hSipLib)
         FreeLibrary(g_hSipLib);
   }
#endif


#ifdef DEBUG
   ParseConnMCCmdLine(pszCmdLine);
#endif

   if (g_fPortrait)
      LoadString(hInstance, IDS_TITLE_PORTRAIT, szTitle,
                 sizeof(szTitle)/sizeof(*szTitle));
   else
      LoadString(hInstance, IDS_TITLE_LANDSCAPE, szTitle,
                 sizeof(szTitle)/sizeof(*szTitle));

   // Only launch ConnMC once
   HWND hPrevWnd = FindWindow(szTitle, szTitle);
   if (hPrevWnd)
   {
      SetForegroundWindow((HWND) ((ULONG) hPrevWnd | 0x00000001));
      return FALSE;
   }

   if (!InitCommCtrlTable())
   {
// ZONE - DEBUGMSG(ZONE_ERROR, _T("Unable to load commctrl.dll\r\n"));
      return FALSE;
   }

   InitCommonControls();
   RegisterIPClass(hInstance);

   WNDCLASS wc = {0};
   wc.lpfnWndProc = ConnMCWndProc;
   wc.hInstance = hInstance;
   wc.hbrBackground = (HBRUSH) GetStockObject(LTGRAY_BRUSH);
   wc.lpszClassName = szTitle;
   if (!RegisterClass(&wc))
      goto ConnMCWinMainExit;

   if (!CreateConnMCWindow(szTitle, hInstance, nCmdShow))
      goto ConnMCWinMainExit;

   RefreshMainMenu(CallCommCtrlFunc(CommandBar_GetMenu)(g_hCommandBar, 0));

   // Launch the thread that will listen for ndis notifications
   LaunchNdisNotificationThread();

   hAccelTable = LoadAccelerators(hInstance, TEXT("CONNMC_ACCEL"));
   while (GetMessage(&msg, NULL, 0, 0))
   {
      if (g_fInRename || !TranslateAccelerator(g_hMainWindow, hAccelTable, &msg))
      {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
   }

   CleanupNdisNotificationThread();

ConnMCWinMainExit:
   UnregisterIPClass(hInstance);
   return msg.wParam;
}

LRESULT CALLBACK ConnMCWndProc(HWND hwnd, UINT msg,
                               WPARAM wParam, LPARAM lParam)
{
   TCHAR cmdLine[256];

   switch (msg)
   {
      case WM_HELP:
      {
         RETAILMSG(1, (TEXT("PegHelp file:rnetw.htm#Main_Contents\r\n")));
         _tcscpy(cmdLine,TEXT("file:rnetw.htm#Main_Contents"));
         CreateProcess(TEXT("peghelp"),cmdLine,
                       NULL,NULL,FALSE,0,NULL,NULL,NULL,NULL);
      }
      break;

      case WM_NETCONNECT:
         RefreshListView();
         break;

      case WM_ACTIVATE:
      {
         if (WA_INACTIVE != LOWORD(wParam))
            SetFocus(g_hListView);
      }
      break;

      case WM_SIZE:
      {
         RECT rc = {0};
         GetClientRect(g_hMainWindow, &rc);
         rc.top += CallCommCtrlFunc(CommandBar_Height)(g_hCommandBar);
         MoveWindow(g_hListView, rc.left, rc.top, rc.right-rc.left,
                    rc.bottom-rc.top, TRUE);
      }
      break;

	  case WM_CONTEXTMENU:
		  {
			  ShowContextMenu();
			  return TRUE;
		  }break;

      case WM_NOTIFY:
      {
         NMLISTVIEW * plv = reinterpret_cast<NM_LISTVIEW *>(lParam);
         ASSERT(plv);

         if (plv && (plv->hdr.hwndFrom == g_hListView))
            return ConnMCHandleNotify(plv);
      }
      break;

      case WM_COMMAND:
      {
         return ConnMCHandleCommand(LOWORD(wParam));
      }
      break;

#ifdef USE_SIP
      case WM_WININICHANGE:
      {
         if (g_pfnSipGetInfo && (SPI_SETSIPINFO == wParam))
         {
            // SIP most likely either was raised or lowered
            SIPINFO si = {0};
            si.cbSize = sizeof(si);
            if ((*g_pfnSipGetInfo)(&si) )
            {
               // Resize the main window and the listview
               SetWindowPos(g_hMainWindow, NULL, si.rcVisibleDesktop.left, si.rcVisibleDesktop.top,
                            si.rcVisibleDesktop.right - si.rcVisibleDesktop.left,
                            si.rcVisibleDesktop.bottom - si.rcVisibleDesktop.top,
                            SWP_NOACTIVATE | SWP_NOZORDER);

               // And the listview--must convert to client coordinates here
               si.rcVisibleDesktop.top += CallCommCtrlFunc(CommandBar_Height)(g_hCommandBar);
               MapWindowRect(NULL, GetParent(g_hListView), &si.rcVisibleDesktop);
               SetWindowPos(g_hListView, NULL, si.rcVisibleDesktop.left, si.rcVisibleDesktop.top,
                            si.rcVisibleDesktop.right - si.rcVisibleDesktop.left,
                            si.rcVisibleDesktop.bottom - si.rcVisibleDesktop.top,
                            SWP_NOACTIVATE | SWP_NOZORDER);
            }
         }
      }
      break;
#endif

      case WM_SYSCOLORCHANGE:
      {
          SendMessage(g_hListView, msg, wParam, lParam);
          SendMessage(g_hCommandBar, msg, wParam, lParam);
      }
      break;

      case WM_DEVICECHANGE:
      {
         if (g_bPropertiesDialogActive)
         {
            g_bDeferredDeviceChangeNotification = TRUE;
         }
         else
         {
            if (g_pAdapterOrderer)
               g_pAdapterOrderer->refresh();
            RefreshListView();
         }
      }
      break;

      case RNM_SHOWERROR:
      {
         ConnMCMessageBox(lParam, wParam, MB_OK | MB_ICONWARNING);
      }
      break;

      case WM_DESTROY:
      {
         // Clean up the image list
         DestroyWindow(g_hListView);
         DeleteCriticalSection(&g_hRefreshCS);
         ImageList_Destroy(g_himlLarge);
         ImageList_Destroy(g_himlSmall);
         PostQuitMessage(0);
      }
      break;

      default:
         return DefWindowProc(hwnd, msg, wParam, lParam);
   }

   return FALSE;
}

BOOL ConnMCHandleNotify(NMLISTVIEW * plv)
{
	if (!plv)
	{
		ASSERT(plv);
		return FALSE;
	}

   switch (plv->hdr.code)
   {
      case LVN_DELETEITEM:
      {
         if (plv->lParam)
         {
            // ZONE - DEBUGMSG (ZONE_MISC, (TEXT("Got LVN_DELETEITEM %d\r\n"), plv->iItem));
            delete (reinterpret_cast<ConnInfo *>(plv->lParam));
         }
      }
      break;

      case LVN_GETDISPINFO:
      {
         NMLVDISPINFO * plvdi = reinterpret_cast<LV_DISPINFO *>(plv);

         ConnInfo * pConnInfo = reinterpret_cast<ConnInfo *>(plvdi->item.lParam);
         ASSERT(pConnInfo);
         if (pConnInfo)
            pConnInfo->getDisplayInfo(plvdi->item.iSubItem, g_hInstance,
                                      plvdi->item.pszText, plvdi->item.cchTextMax, NULL);
      }
      break;

      case LVN_ITEMCHANGED:
      {
         RefreshMainMenu(CallCommCtrlFunc(CommandBar_GetMenu)(g_hCommandBar, 0));
      }

      case LVN_COLUMNCLICK:
      {
         // TODO: We could sort the item list here
      }
      break;

      case LVN_BEGINLABELEDIT:
      {
         NMLVDISPINFO * plvdi = reinterpret_cast<LV_DISPINFO *>(plv);
         ConnInfo * pConnInfo = reinterpret_cast<ConnInfo *>(plvdi->item.lParam);
         ASSERT(pConnInfo);
         if (!pConnInfo)
            return TRUE;

         g_fInRename = TRUE;
#ifdef USE_SIP
         PositionSIP(SIP_UP);
#endif
         ListView_EnsureVisible(g_hListView, plvdi->item.iItem, FALSE);

         UINT uError = 0;
         if (!pConnInfo->allowRename(FALSE, &uError))
         {
            ConnMCMessageBox(uError, NULL, MB_OK | MB_ICONWARNING);
            return TRUE;
         }

         HWND hEdit = ListView_GetEditControl(g_hListView);
         if (hEdit)
            SendMessage(hEdit, EM_LIMITTEXT, RAS_MaxEntryName, 0);

         return FALSE; // Allow the edit
      }
      break;

      case LVN_ENDLABELEDIT:
      {
         g_fInRename = FALSE;
#ifdef USE_SIP
         PositionSIP(SIP_DOWN);
#endif

         NMLVDISPINFO * plvdi = reinterpret_cast<LV_DISPINFO *>(plv);
         ConnInfo * pConnInfo = reinterpret_cast<ConnInfo *>(plvdi->item.lParam);
         ASSERT(pConnInfo);
         if (!pConnInfo || !plvdi->item.pszText)
            return FALSE; // Don't change the label

         UINT uError = 0;
         if (!pConnInfo->rename(plvdi->item.pszText, &uError))
         {
            ConnMCMessageBox(uError, NULL, MB_OK | MB_ICONWARNING);
            PostMessage(g_hListView, LVM_EDITLABEL, (WPARAM)plvdi->item.iItem, 0);
            return FALSE;
         }

         ListView_SetItemState(g_hListView, plvdi->item.iItem,
                               LVIS_SELECTED | LVIS_FOCUSED,
                               LVIS_SELECTED | LVIS_FOCUSED);
         SetFocus(g_hListView);

         return TRUE; // The edit was successful
      }
      break;

      case NM_RCLICK:
      {
         ShowContextMenu();
      }
      break;

      case NM_CLICK:
      {
         if (GetKeyState(VK_MENU) < 0)
            ShowContextMenu();
      }
      break;

      case NM_DBLCLK:
      {
         if (GetKeyState(VK_MENU) >= 0)
         {
            // Here we have to special case LAN connections because double click doesn't
            // do the default action, it really launches the properties dialog
            LV_ITEM lvi = {0};
            lvi.mask = LVIF_PARAM;
            lvi.iItem = ListView_GetNextItem(g_hListView, -1,
                                             LVNI_SELECTED | LVIS_FOCUSED);
            ListView_GetItem(g_hListView, &lvi);
            if (-1 == lvi.iItem)
               break; // Nothing is highlighted

            ConnInfo * pConnInfo = reinterpret_cast<ConnInfo *>(lvi.lParam);
            ASSERT(pConnInfo);
            if (pConnInfo && pConnInfo->isType(_T("LanConnInfo")))
               SendMessage(g_hMainWindow, WM_COMMAND, ID_FILE_PROPERTIES, 0);
            else
               SendMessage(g_hMainWindow, WM_COMMAND, ID_DEFAULT_ACTION, 0);
         }
         else
         {
            ShowContextMenu();
         }
      }
      break;

      case LVN_KEYDOWN:
      {
         NMLVKEYDOWN * plvkd = reinterpret_cast<LV_KEYDOWN *>(plv);
         if (VK_RETURN == plvkd->wVKey)
         {
            // Here we have to special case LAN connections because double click doesn't
            // do the default action, it really launches the properties dialog
            LV_ITEM lvi = {0};
            lvi.mask = LVIF_PARAM;
            lvi.iItem = ListView_GetNextItem(g_hListView, -1,
                                             LVNI_SELECTED | LVIS_FOCUSED);
            ListView_GetItem(g_hListView, &lvi);
            if (-1 == lvi.iItem)
               break; // Nothing is highlighted

            ConnInfo * pConnInfo = reinterpret_cast<ConnInfo *>(lvi.lParam);
            ASSERT(pConnInfo);
            if (pConnInfo && pConnInfo->isType(_T("LanConnInfo")))
               SendMessage(g_hMainWindow, WM_COMMAND, ID_FILE_PROPERTIES, 0);
            else
               SendMessage(g_hMainWindow, WM_COMMAND, ID_DEFAULT_ACTION, 0);
         }
         else if ((VK_CONTROL == plvkd->wVKey) && GetAsyncKeyState(VK_MENU))
         {
            ShowContextMenu();
         }
         else
         {
            return FALSE; // Let the LV message handler take care of this
         }
      }
      break;

      default:
         return FALSE;
   }

   return TRUE;
}

BOOL ConnMCHandleCommand(WORD code)
{
   LVITEM lvi = {0};

   switch (code)
   {
      // Help commands
      case ID_HELP:
      {
         if (!g_fPortrait)
            ConnMCMessageBox(IDS_COPYRIGHT, IDS_HELPABOUT, MB_OK);
      }
      break;

      // Logic commands
      case ID_DEFAULT_ACTION:
      {
         ConnInfo * pConnInfo = NULL;
         lvi.mask = LVIF_PARAM;
         lvi.iItem = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
         while (-1 != lvi.iItem)
         {
            if (ListView_GetItem(g_hListView, &lvi))
            {
               pConnInfo = reinterpret_cast<ConnInfo *>(lvi.lParam);
               ASSERT(pConnInfo);
               if (pConnInfo)
               {
                  pConnInfo->defaultAction(g_hInstance, g_hMainWindow, NULL);
               }
            }
            lvi.iItem = ListView_GetNextItem(g_hListView, lvi.iItem,
                                             LVNI_SELECTED);
         }

         RefreshListView();
      }
      break;

      case ID_CONN_NEW:
      {
         ConnInfo * pConnInfo = NULL;
         lvi.mask = LVIF_PARAM;
         lvi.iItem = 0;
         while (ListView_GetItem(g_hListView, &lvi))
         {
            pConnInfo = reinterpret_cast<ConnInfo *>(lvi.lParam);
            if (pConnInfo)
            {
               if (pConnInfo->isType(_T("NewConnInfo")))
               {
                  pConnInfo->defaultAction(g_hInstance, g_hMainWindow, NULL);
                  break; // We are guarenteed to have a NewConnInfo around
               }
               lvi.iItem++;
            }
         }

         RefreshListView();
      }
      break;

      case ID_FILE_DELETE:
      {
         UINT selectedCount = ListView_GetSelectedCount(g_hListView);
         ASSERT(0 < selectedCount);

         lvi.mask = LVIF_PARAM;
         lvi.iItem = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
         ListView_GetItem(g_hListView, &lvi);
         ConnInfo * pConnInfo = reinterpret_cast<ConnInfo *>(lvi.lParam);

         // Check to make sure the user really want to delete
         if (1 == selectedCount)
         {
            UINT uError = 0;
            if (!pConnInfo->allowRemove(FALSE, &uError))
            {
               ConnMCMessageBox(uError, NULL, MB_ICONWARNING | MB_OK);
               break;
            }
            else
            {
               if (IDYES != ConnMCMessageBox(IDS_DELCON, IDS_CONFDEL,
                                             MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2))
               {
                  // ZONE
                  break;
               }
            }
         }
         else // (1 < selectedCount)
         {
            if (IDYES != ConnMCMessageBox(IDS_DELCONS, IDS_CONFDEL,
                                          MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2))
            {
               // ZONE
               break;
            }
         }

         // User confirmed delete, start deleting
         BOOL fAllDeleted = TRUE;
         lvi.mask = LVIF_PARAM;
         lvi.iItem = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
         while (-1 != lvi.iItem)
         {
            if (ListView_GetItem(g_hListView, &lvi))
            {
               pConnInfo = reinterpret_cast<ConnInfo *>(lvi.lParam);
               ASSERT(pConnInfo);
               if (pConnInfo)
               {
                  if (pConnInfo->remove(NULL))
                     ListView_DeleteItem(g_hListView, lvi.iItem);
                  else
                     fAllDeleted = FALSE;
               }
            }
            lvi.iItem = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
         }

         if (!fAllDeleted)
            ConnMCMessageBox(IDS_ERR_NOT_ALL_DEL, NULL, MB_ICONWARNING | MB_OK);

         SetFocus(g_hListView);
      }
      break;

      case ID_FILE_CREATESHORTCUT:
      {
         if (g_fPortrait)
            break;

         lvi.mask = LVIF_PARAM;
         lvi.iItem = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
         while (-1 != lvi.iItem)
         {
            if (ListView_GetItem(g_hListView, &lvi))
            {
               ConnInfo * pConnInfo = reinterpret_cast<ConnInfo *>(lvi.lParam);
               ASSERT(pConnInfo);
               if (pConnInfo)
               {
                  UINT uError = 0;
                  if (!pConnInfo->allowShortcut(TRUE, &uError) ||
                      !pConnInfo->shortcut(g_hInstance, &uError))
                  {
                     ConnMCMessageBox(uError, NULL, MB_APPLMODAL | MB_OK |
                                                    MB_ICONEXCLAMATION);
                  }
               }
            }
            lvi.iItem = ListView_GetNextItem(g_hListView, lvi.iItem,
                                             LVNI_SELECTED);
         }
      }
      break;

      case ID_FILE_PROPERTIES:
      {
         if (1 != ListView_GetSelectedCount(g_hListView))
         {
            ConnMCMessageBox(IDS_ERR_TOOMANYENT, IDS_ERROR, MB_OK|MB_ICONWARNING);
            break;
         }

         lvi.mask = LVIF_PARAM;
         lvi.iItem = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
         ASSERT(-1 != lvi.iItem);
         ListView_GetItem(g_hListView, &lvi);
         ConnInfo * pConnInfo = reinterpret_cast<ConnInfo *>(lvi.lParam);
         ASSERT(pConnInfo);
         if (pConnInfo)
         {
            UINT uError;

			//
			// Set a flag while the (modal) connection properties dialog is displayed
			// to prevent a WM_DEVICECHANGE notification from deleting our connection
			// objects.
			//
			g_bPropertiesDialogActive = TRUE;

#ifdef USE_SIP
            PositionSIP(SIP_UP);
#endif

            if (!pConnInfo->showProperties(g_hInstance, g_hMainWindow, &uError))
               ConnMCMessageBox(uError, IDS_ERROR, MB_OK|MB_ICONWARNING);
#ifdef USE_SIP
            PositionSIP(SIP_DOWN);
#endif
			//
			// If a WM_DEVICECHANGE notification was deferred during the display
			// of the connection properties dialog, process it now.
			//
			g_bPropertiesDialogActive = FALSE;
			if (g_bDeferredDeviceChangeNotification)
			{
				g_bDeferredDeviceChangeNotification = FALSE;
                RefreshListView();
			}
         }
      }
      break;

      case ID_EDIT_CREATECOPY:
      {
         if (g_fPortrait)
            break;

         BOOL fAllCopied = TRUE;
         lvi.mask = LVIF_PARAM;
         lvi.iItem = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
         while (-1 != lvi.iItem)
         {
            if (ListView_GetItem(g_hListView, &lvi))
            {
               ConnInfo * pConnInfo = reinterpret_cast<ConnInfo *>(lvi.lParam);
               ASSERT(pConnInfo);
               if (pConnInfo)
               {
                  if (!pConnInfo->copy(g_hInstance, NULL))
                     fAllCopied = FALSE;
               }
            }
            lvi.iItem = ListView_GetNextItem(g_hListView, lvi.iItem,
                                             LVNI_SELECTED);
         }

         if (!fAllCopied)
            ConnMCMessageBox(IDS_ERR_NOT_ALL_COPY, NULL, MB_ICONWARNING | MB_OK);

         RefreshListView();
      }
      break;

      case ID_ADVANCED_ADVANCED:
      {
         g_pAdapterOrderer = new AdapterOrderer(g_hInstance, g_hMainWindow);
         if (g_pAdapterOrderer)
         {
            g_pAdapterOrderer->refresh();
            g_pAdapterOrderer->show();

            delete g_pAdapterOrderer;
            g_pAdapterOrderer = NULL;

            RefreshListView(); // Pick up any changes that happened while ordering
         }
      }
      break;

      case ID_CONN_SET_AS_DEFAULT:
      {
         if (1 != ListView_GetSelectedCount(g_hListView))
         {
            ConnMCMessageBox(IDS_ERR_TOOMANYENT, IDS_ERROR, MB_OK|MB_ICONWARNING);
            break;
         }

         lvi.mask = LVIF_PARAM;
         lvi.iItem = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
         ASSERT(-1 != lvi.iItem);
         ListView_GetItem(g_hListView, &lvi);
         ConnInfo * pConnInfo = reinterpret_cast<ConnInfo *>(lvi.lParam);
         ASSERT(pConnInfo);
         if (pConnInfo)
         {
            UINT uError;
            if (pConnInfo->setAsDefault(!pConnInfo->isSetAsDefault(), &uError))
               RefreshListView();
            else
               ConnMCMessageBox(uError, IDS_ERROR, MB_OK|MB_ICONWARNING);
         }
      }
      break;

      // UI commands
      case ID_FILE_RENAME:
      {
         if (1 != ListView_GetSelectedCount(g_hListView))
         {
            ConnMCMessageBox(IDS_ERR_TOOMANYENT, IDS_ERROR, MB_OK|MB_ICONWARNING);
            break;
         }

         lvi.iItem = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
         ASSERT(-1 != lvi.iItem);
         SetFocus(g_hListView);
         ListView_EditLabel(g_hListView, lvi.iItem);
      }
      break;

      case ID_EDIT_SELECTALL:
      {
         ListView_SetItemState(g_hListView, -1, LVIS_SELECTED, LVIS_SELECTED);
      }
      break;

      case ID_VIEW_LARGEICON:
      {
         if (!g_fPortrait)
         {
            RefreshViewMenu(ID_VIEW_LARGEICON);
            SetWindowLong(g_hListView, GWL_STYLE,
                          (GetWindowLong (g_hListView, GWL_STYLE) &
                          ~LVS_TYPEMASK) | LVS_ICON);
         }
      }
      break;

      case ID_VIEW_SMALLICON:
      {
         if (!g_fPortrait)
         {
            RefreshViewMenu(ID_VIEW_SMALLICON);
            SetWindowLong(g_hListView, GWL_STYLE,
                          (GetWindowLong (g_hListView, GWL_STYLE) &
                          ~LVS_TYPEMASK) | LVS_SMALLICON);
         }
      }
      break;

      case ID_VIEW_DETAILS:
      {
         if (!g_fPortrait)
         {
            RefreshViewMenu(ID_VIEW_DETAILS);
            SetWindowLong(g_hListView, GWL_STYLE,
                          (GetWindowLong (g_hListView, GWL_STYLE) &
                          ~LVS_TYPEMASK) | LVS_REPORT);
         }
      }
      break;

      case ID_VIEW_REFRESH:
      {
         RefreshListView();
      }
      break;

      case ID_FILE_EXIT:
      {
         // TODO - Could there be a better way to do this
         DestroyWindow(g_hMainWindow);
      }
      break;

      default:
         // ZONE - debug zone info DEBUGMSG(ZONE_MISC, (TEXT("Got WM_COMMAND=%d\r\n"), LOWORD(wParam)));
         return FALSE;
   }
   return TRUE;
}

BOOL CreateConnMCWindow(TCHAR * pszTitle, HINSTANCE hInstance, int nCmdShow)
{
   ASSERT(pszTitle);
   int nWidth, nHeight, x, y;

   // Figure out the size of the window based on SIP
   nWidth = nHeight = CW_USEDEFAULT;
   x = y = CW_USEDEFAULT;
#ifdef USE_SIP
   SIPINFO si = {0};
   si.cbSize = sizeof(si);
   if (g_pfnSipGetInfo && (*g_pfnSipGetInfo)(&si))
   {
      nWidth = si.rcVisibleDesktop.right - si.rcVisibleDesktop.left;
      nHeight = si.rcVisibleDesktop.bottom - si.rcVisibleDesktop.top;
      x = si.rcVisibleDesktop.left;
      y = si.rcVisibleDesktop.top;
   }
#endif

   InitializeCriticalSection(&g_hRefreshCS);

   g_hMainWindow = CreateWindow(pszTitle, pszTitle, WS_CLIPCHILDREN, x, y,
                                nWidth, nHeight, NULL, NULL, g_hInstance, NULL);
   if (!g_hMainWindow)
      return FALSE;

   // Set ConnMC icon for the task bar
   HICON hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_REMOTENW),
                                  IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
   if (hIcon)
      SendMessage(g_hMainWindow, WM_SETICON, FALSE, (WPARAM) hIcon);

   if (!CreateConnMCCommandBar())
      return FALSE;

   InitializeCriticalSection(&g_hRefreshCS);

   if (!CreateConnMCListView())
      return FALSE;

   ShowWindow(g_hMainWindow, SW_SHOW);
   UpdateWindow(g_hMainWindow);

   SetFocus(g_hListView);
   return TRUE;
}

BOOL CreateConnMCCommandBar()
{
   g_hCommandBar = CallCommCtrlFunc(CommandBar_Create)(g_hInstance,
                                                       g_hMainWindow, 1);
   if (!g_hCommandBar)
      return FALSE;

   if (g_fPortrait)
   {
      // Add the Menu
      CallCommCtrlFunc(CommandBar_InsertMenubar)(g_hCommandBar, g_hInstance,
                                                 IDM_PORTRAIT, 0);

      // Add the standard (delete/properties) and view bitmap buttons
      CallCommCtrlFunc(CommandBar_AddBitmap)(g_hCommandBar, HINST_COMMCTRL,
                                             IDB_STD_SMALL_COLOR, 0, 16, 16);
      CallCommCtrlFunc(CommandBar_AddBitmap)(g_hCommandBar, g_hInstance,
                                             IDB_TOOLBAR, 1, 16, 16);

      // Associate the bitmaps with button actions
      CommandBar_AddButtons(g_hCommandBar,
                            sizeof(g_tbPortrait)/sizeof(*g_tbPortrait),
                            g_tbPortrait);

      /* TODO - add tooltips
      LoadString(g_hInstance, IDS_TLTP_NON, nonStr, sizeof(nonStr));
      LoadString(g_hInstance, IDS_TLTP_CONNECT, largeIconStr, sizeof(largeIconStr));
      LoadString(g_hInstance, IDS_TLTP_EDIT, smallIconStr, sizeof(smallIconStr));
      LoadString(g_hInstance, IDS_TLTP_DELETEP, propertiesStr, sizeof(propertiesStr));
      CommandBar_AddToolTips(g_hCommandBar, 4, (LPARAM)ToolTipsTblPortrait);
      */
   }
   else
   {
      // Add the Menu
      CallCommCtrlFunc(CommandBar_InsertMenubar)(g_hCommandBar, g_hInstance,
                                                 IDM_LANDSCAPE, 0);

      // Add the standard (delete/properties) and view bitmap buttons
      CallCommCtrlFunc(CommandBar_AddBitmap)(g_hCommandBar, HINST_COMMCTRL,
                                             IDB_STD_SMALL_COLOR, 0, 16, 16);
      CallCommCtrlFunc(CommandBar_AddBitmap)(g_hCommandBar, HINST_COMMCTRL,
                                             IDB_VIEW_SMALL_COLOR, 0, 16, 16);

      // Associate the bitmaps with button actions
      CommandBar_AddButtons(g_hCommandBar,
                            sizeof(g_tbLandscape)/sizeof(*g_tbLandscape),
                            g_tbLandscape);

      /* TODO - add tooltips
      LoadString(g_hInstance, IDS_TLTP_NON, nonStr, sizeof(nonStr));
      LoadString(g_hInstance, IDS_TLTP_DELETE, deleteStr, sizeof(deleteStr));
      LoadString(g_hInstance, IDS_TLTP_PRPTY, propertiesStr, sizeof(propertiesStr));
      LoadString(g_hInstance, IDS_TLTP_LARGE, largeIconStr, sizeof(largeIconStr));
      LoadString(g_hInstance, IDS_TLTP_SMOLL, smallIconStr, sizeof(smallIconStr));
      LoadString(g_hInstance, IDS_TLTP_DETAILS, detailsStr, sizeof(detailsStr));
      CommandBar_AddToolTips(g_hCommandBar, 6, (LPARAM)ToolTipsTbl);
      */

      // Set Large Icon view as the default
      SendMessage(g_hCommandBar, TB_CHECKBUTTON, ID_VIEW_LARGEICON,
                  MAKELONG(TRUE, 0));
      HMENU hMenu = CallCommCtrlFunc(CommandBar_GetMenu)(g_hCommandBar, 0);
      CheckMenuRadioItem(hMenu, ID_VIEW_LARGEICON, ID_VIEW_DETAILS,
                         ID_VIEW_LARGEICON, MF_BYCOMMAND);
   }

   // Add exit and help buttons
   CallCommCtrlFunc(CommandBar_AddAdornments)(g_hCommandBar, CMDBAR_HELP, 0);

   return TRUE;
}

BOOL CreateConnMCListView()
{
   RECT rc = {0};
   HICON hiconItem = NULL;      // icon for list view items
   int iDefaultOverlay = 0;

   GetClientRect(g_hMainWindow, &rc);
   rc.top += CallCommCtrlFunc(CommandBar_Height)(g_hCommandBar);
   g_hListView = CreateWindow(WC_LISTVIEW, TEXT(""), WS_VISIBLE | WS_CHILD |
                              WS_VSCROLL | LVS_SHOWSELALWAYS | LVS_AUTOARRANGE |
                              0x0040 | LVS_EDITLABELS | LVS_ICON | LVS_SHAREIMAGELISTS,
                              rc.left, rc.top,  rc.right - rc.left,
                              rc.bottom - rc.top, g_hMainWindow,
                              NULL, g_hInstance, NULL);
   if (!g_hListView)
      return FALSE;

   // Create the full-sized and small icon image lists.
   g_himlLarge = ImageList_Create(GetSystemMetrics(SM_CXICON),
                                  GetSystemMetrics(SM_CYICON),
                                  ILC_MASK, 4, 1);
   ASSERT(g_himlLarge);
   g_himlSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
                                  GetSystemMetrics(SM_CYSMICON),
                                  ILC_MASK, 4, 1);
   ASSERT(g_himlSmall);

   if (!g_himlLarge || !g_himlSmall)
      return FALSE;

   // Add IDI_NEWCONN icon
   hiconItem = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_NEWCONN));
   if (hiconItem)
   {
      UINT iconID = ImageList_AddIcon(g_himlLarge, hiconItem);
      if (-1 != iconID)
      {
         NewConnInfo::setIconID(iconID);
         hiconItem = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_NEWCONN),
                                      IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
         if (hiconItem)
         {
            ImageList_AddIcon(g_himlSmall, hiconItem);
            DestroyIcon(hiconItem);
         }
      }
   }

   // Add IDI_RNA icon
   hiconItem = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_RNA));
   if (hiconItem)
   {
      UINT iconID = ImageList_AddIcon(g_himlLarge, hiconItem);
      if (-1 != iconID)
      {
         RnaConnInfo::setIconID(iconID);
         hiconItem = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_RNA),
                                      IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
         if (hiconItem)
         {
            ImageList_AddIcon(g_himlSmall, hiconItem);
            DestroyIcon(hiconItem);
         }
      }
   }

   // Add IDI_DCC icon
   hiconItem = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_DCC));
   if (hiconItem)
   {
      UINT iconID = ImageList_AddIcon(g_himlLarge, hiconItem);
      if (-1 != iconID)
      {
         DccConnInfo::setIconID(iconID);
         hiconItem = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_DCC),
                                      IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
         if (hiconItem)
         {
            ImageList_AddIcon(g_himlSmall, hiconItem);
            DestroyIcon(hiconItem);
         }
      }
   }

   // Add IDI_VPN icon
   hiconItem = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_VPN));
   if (hiconItem)
   {
      UINT iconID = ImageList_AddIcon(g_himlLarge, hiconItem);
      if (-1 != iconID)
      {
         VpnConnInfo::setIconID(iconID);
         hiconItem = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_VPN),
                                      IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
         if (hiconItem)
         {
            ImageList_AddIcon(g_himlSmall, hiconItem);
            DestroyIcon(hiconItem);
         }
      }
   }


   //
   // use IDI_VPN icon for pppoe connection..
   //
   hiconItem = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_VPN));
   if (hiconItem)
   {
      UINT iconID = ImageList_AddIcon(g_himlLarge, hiconItem);
      if (-1 != iconID)
      {
         PPPOEConnInfo::setIconID(iconID);
         hiconItem = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_VPN),
                                      IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
         if (hiconItem)
         {
            ImageList_AddIcon(g_himlSmall, hiconItem);
            DestroyIcon(hiconItem);
         }
      }
   }

   // Add LAN (enabled/disabled) icons
   hiconItem = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_LAN_ENABLED));
   if (hiconItem)
   {
      UINT iconID = ImageList_AddIcon(g_himlLarge, hiconItem);
      if (-1 != iconID)
      {
         LanConnInfo::setEnabledLanIconID(iconID);
         hiconItem = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_LAN_ENABLED),
                                      IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
         if (hiconItem)
         {
            ImageList_AddIcon(g_himlSmall, hiconItem);
            DestroyIcon(hiconItem);
         }
      }
   }
   hiconItem = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_LAN_DISABLED));
   if (hiconItem)
   {
      UINT iconID = ImageList_AddIcon(g_himlLarge, hiconItem);
      if (-1 != iconID)
      {
         LanConnInfo::setDisabledLanIconID(iconID);
         hiconItem = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_LAN_DISABLED),
                                      IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
         if (hiconItem)
         {
            ImageList_AddIcon(g_himlSmall, hiconItem);
            DestroyIcon(hiconItem);
         }
      }
   }

   // Add WLAN (enabled/disabled) icons
   hiconItem = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_WLAN_ENABLED));
   if (hiconItem)
   {
      UINT iconID = ImageList_AddIcon(g_himlLarge, hiconItem);
      if (-1 != iconID)
      {
         LanConnInfo::setEnabledWLanIconID(iconID);
         hiconItem = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_WLAN_ENABLED),
                                      IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
         if (hiconItem)
         {
            ImageList_AddIcon(g_himlSmall, hiconItem);
            DestroyIcon(hiconItem);
         }
      }
   }
   hiconItem = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_WLAN_DISABLED));
   if (hiconItem)
   {
      UINT iconID = ImageList_AddIcon(g_himlLarge, hiconItem);
      if (-1 != iconID)
      {
         LanConnInfo::setDisabledWLanIconID(iconID);
         hiconItem = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_WLAN_DISABLED),
                                      IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
         if (hiconItem)
         {
            ImageList_AddIcon(g_himlSmall, hiconItem);
            DestroyIcon(hiconItem);
         }
      }
   }

   // Add the default check overlay image
   hiconItem = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_DEFAULTOVERLAY),
                                IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
   if (hiconItem)
   {
      iDefaultOverlay = ImageList_AddIcon(g_himlLarge, hiconItem);
      DestroyIcon(hiconItem);
      if (-1 != iDefaultOverlay)
      {
         hiconItem = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_DEFAULTOVERLAY),
                                      IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
         if (hiconItem)
         {
            ImageList_AddIcon(g_himlSmall, hiconItem);
            DestroyIcon(hiconItem);
         }

         ImageList_SetOverlayImage(g_himlLarge, iDefaultOverlay, 1);
         ImageList_SetOverlayImage(g_himlSmall, iDefaultOverlay, 1);
      }
   }

   // Assign the image lists to the list view control.
   ListView_SetImageList(g_hListView, g_himlLarge, LVSIL_NORMAL);
   ListView_SetImageList(g_hListView, g_himlSmall, LVSIL_SMALL);

   // Add the columns.
   TCHAR szColText[256];
   LV_COLUMN lvc = {0};
   lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
   lvc.fmt = LVCFMT_LEFT;
   lvc.pszText = szColText;
   for (int i = 0; i < LV_COLUMN_COUNT; i++)
   {
      lvc.iSubItem = i;
      switch (i)
      {
         case 0:  lvc.cx = 150; break;
         case 2:  lvc.cx = 225; break;
         default: lvc.cx = 100;
      }

      LoadString(g_hInstance, IDS_LV_COLUMN_0+i, szColText,
                 sizeof(szColText)/sizeof(*szColText));
      ListView_InsertColumn(g_hListView, i, &lvc);
   }

   return RefreshListView();
}

BOOL RefreshListView()
{
   if (!g_hListView)
      return FALSE;

   EnterCriticalSection(&g_hRefreshCS);

   LVITEM lvi = {0};
   ConnInfo * pConnInfo = NULL;

   lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
   lvi.pszText = LPSTR_TEXTCALLBACK;

   ListView_DeleteAllItems(g_hListView);

   //////////////////////////////////////////////
   // Insert the New Connection icon
   //////////////////////////////////////////////
   TCHAR szTemp[512];
   LoadString(g_hInstance, IDS_MAKE_NEW,
              szTemp, sizeof(szTemp)/sizeof(*szTemp));
   pConnInfo = new NewConnInfo(szTemp);
   if (!pConnInfo)
   {
      LeaveCriticalSection(&g_hRefreshCS);
      return FALSE;
   }
   lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
   lvi.state = LVIS_SELECTED | LVIS_FOCUSED;
   lvi.lParam = (LPARAM) pConnInfo;
   lvi.iImage = pConnInfo->getIconID();
   ListView_InsertItem(g_hListView, &lvi);

   //////////////////////////////////////////////
   // Now insert the RAS icons
   //////////////////////////////////////////////
   pConnInfo = RasConnInfo::FindFirst();
   while (pConnInfo)
   {
      lvi.iItem++;
      lvi.iImage = pConnInfo->getIconID();
      lvi.lParam = (LPARAM) pConnInfo; // save item data
      if (((RasConnInfo *) pConnInfo)->isSetAsDefault())
      {
         lvi.stateMask = LVIS_OVERLAYMASK;
         lvi.state = INDEXTOOVERLAYMASK(1);
      }
      else
      {
         lvi.stateMask = 0;
         lvi.state = 0;
      }
      ListView_InsertItem(g_hListView, &lvi);

      pConnInfo = RasConnInfo::FindNext();
   }
   lvi.stateMask = 0;
   lvi.state = 0;

   //////////////////////////////////////////////
   // Add the LAN adapters here
   //////////////////////////////////////////////
   pConnInfo = LanConnInfo::FindFirst();
   while (pConnInfo)
   {
      lvi.iItem++;
      lvi.iImage = pConnInfo->getIconID();
      lvi.lParam = (LPARAM) pConnInfo; // save item data
      ListView_InsertItem(g_hListView, &lvi);

      pConnInfo = LanConnInfo::FindNext();
   }

   LeaveCriticalSection(&g_hRefreshCS);

   return TRUE;
}

void RefreshMainMenu(HMENU hMenu)
{
   BOOL enableDefaultAction = TRUE;
   BOOL enableSetAsDefault = TRUE;
   BOOL checkSetAsDefault = FALSE;
   BOOL enableShortcut = !g_fPortrait;
   BOOL enableDelete = TRUE;
   BOOL enableRename = TRUE;
   BOOL enableProperties = TRUE;
   BOOL enableCopy = TRUE;
   DWORD dwSelected = ListView_GetSelectedCount(g_hListView);
   BOOL fInGroup = (1 < dwSelected);
   LV_ITEM lvi = {0};
   lvi.mask = LVIF_PARAM;

   // If nothing is selected just grey everything out and bail
   if (0 < dwSelected)
   {
      // Find the lowest common denominator of what should be highlighted
      ConnInfo * pConnInfo = NULL;
      lvi.iItem = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
      while ((-1 != lvi.iItem) && ListView_GetItem(g_hListView, &lvi))
      {
         pConnInfo = reinterpret_cast<ConnInfo *>(lvi.lParam);
         ASSERT(pConnInfo);
         if (pConnInfo)
         {
            if (enableDefaultAction)
               enableDefaultAction = pConnInfo->allowDefaultAction(fInGroup, NULL);
            if (enableSetAsDefault)
               enableSetAsDefault = pConnInfo->allowSetAsDefault(fInGroup, NULL);
            if (enableShortcut)
               enableShortcut = pConnInfo->allowShortcut(fInGroup, NULL);
            if (enableDelete)
               enableDelete = pConnInfo->allowRemove(fInGroup, NULL);
            if (enableRename)
               enableRename = pConnInfo->allowRename(fInGroup, NULL);
            if (enableProperties)
               enableProperties = pConnInfo->allowProperties(fInGroup, NULL);
            if (enableCopy)
               enableCopy = pConnInfo->allowCopy(fInGroup, NULL);
         }
         lvi.iItem = ListView_GetNextItem(g_hListView, lvi.iItem,
                                          LVNI_SELECTED);
      }
   }
   else
   {
      enableDefaultAction = FALSE;
      enableSetAsDefault = FALSE;
      enableShortcut = FALSE;
      enableDelete = FALSE;
      enableRename = FALSE;
      enableProperties = FALSE;
      enableCopy = FALSE;
   }

   // Find what icon we actually clicked and set the action
   // string and set as default check mark accordingly
   lvi.iItem = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED | LVIS_FOCUSED);
   ListView_GetItem(g_hListView, &lvi);
   if (-1 != lvi.iItem)
   {
      ConnInfo * pConnInfo = reinterpret_cast<ConnInfo *>(lvi.lParam);
      ASSERT(pConnInfo);
      if (pConnInfo)
      {
         TCHAR szMenuText[128] = {0};
         MENUITEMINFO mii = {0};
         if (pConnInfo->getDefaultActionString(g_hInstance, szMenuText, NULL))
         {
            // Special case for New, the command bar menu should never show
            // "New..." as it's action item, because it already has a "New..." item
            if ((1 == dwSelected) && pConnInfo->isType(_T("NewConnInfo")) &&
                (CallCommCtrlFunc(CommandBar_GetMenu)(g_hCommandBar, 0) == hMenu))
            {
               LoadString(g_hInstance, IDS_RAS_CONNECT, szMenuText,
                          sizeof(szMenuText)/sizeof(*szMenuText));
               enableDefaultAction = FALSE;
            }

            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_TYPE;
            mii.fType = MFT_STRING;
            mii.dwTypeData = szMenuText;
            mii.cch = _tcslen(szMenuText)+1;
            SetMenuItemInfo(hMenu, ID_DEFAULT_ACTION, FALSE, &mii);
         }

         checkSetAsDefault = ((1 == dwSelected) && (pConnInfo->isSetAsDefault()));
      }
   }

   // Finally, the enabling/disabling/checking
   // If we are in portrait EnableMenuItem will handle the missing IDs for us
   EnableMenuItem(hMenu, ID_DEFAULT_ACTION, enableDefaultAction ? MF_ENABLED : MF_GRAYED);
   EnableMenuItem(hMenu, ID_CONN_SET_AS_DEFAULT, enableSetAsDefault ? MF_ENABLED : MF_GRAYED);
   CheckMenuItem(hMenu, ID_CONN_SET_AS_DEFAULT, checkSetAsDefault ? (MF_BYCOMMAND | MF_CHECKED) :
                                                                    (MF_BYCOMMAND | MF_UNCHECKED));
   EnableMenuItem(hMenu, ID_FILE_CREATESHORTCUT, enableShortcut ? MF_ENABLED : MF_GRAYED);
   EnableMenuItem(hMenu, ID_FILE_DELETE, enableDelete ? MF_ENABLED : MF_GRAYED);
   EnableMenuItem(hMenu, ID_FILE_RENAME, enableRename ? MF_ENABLED : MF_GRAYED);
   EnableMenuItem(hMenu, ID_FILE_PROPERTIES, enableProperties ? MF_ENABLED : MF_GRAYED);
   EnableMenuItem(hMenu, ID_EDIT_CREATECOPY, enableCopy ? MF_ENABLED : MF_GRAYED);

   // Set the Tool Bar buttons
   if (g_fPortrait)
   {
      SendMessage(g_hCommandBar, TB_ENABLEBUTTON,
                  ID_DEFAULT_ACTION, MAKELONG(enableDefaultAction, 0));
   }
   SendMessage(g_hCommandBar, TB_ENABLEBUTTON,
               ID_FILE_DELETE, MAKELONG(enableDelete, 0));
   SendMessage(g_hCommandBar, TB_ENABLEBUTTON,
               ID_FILE_PROPERTIES, MAKELONG(enableProperties, 0));
}

void RefreshViewMenu(DWORD viewType)
{
   HMENU hMenu = CallCommCtrlFunc(CommandBar_GetMenu)(g_hCommandBar, 0);
   if (!hMenu)
      return;

   ASSERT(ID_VIEW_LARGEICON <= viewType);
   ASSERT(ID_VIEW_DETAILS >= viewType);

   // Check the menu item
   CheckMenuRadioItem(hMenu, ID_VIEW_LARGEICON, ID_VIEW_DETAILS,
                      viewType, MF_BYCOMMAND);

   // Push the Command Bar button
   SendMessage(g_hCommandBar, TB_CHECKBUTTON, ID_VIEW_LARGEICON,
               MAKELONG((ID_VIEW_LARGEICON == viewType) ? TRUE : FALSE, 0));
   SendMessage(g_hCommandBar, TB_CHECKBUTTON, ID_VIEW_SMALLICON,
               MAKELONG((ID_VIEW_SMALLICON == viewType) ? TRUE : FALSE, 0));
   SendMessage(g_hCommandBar, TB_CHECKBUTTON, ID_VIEW_DETAILS,
               MAKELONG((ID_VIEW_DETAILS == viewType) ? TRUE : FALSE, 0));
}

void ShowContextMenu()
{
   HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDM_CONTEXT));
   if (hMenu)
   {
      HMENU hMenuTrack = GetSubMenu(hMenu, 0);
      if (hMenuTrack)
      {
         RefreshMainMenu(hMenuTrack);

         DWORD dwPoint = GetMessagePos();
         TrackPopupMenu(hMenuTrack, TPM_LEFTALIGN, GET_X_LPARAM(dwPoint),
                        GET_Y_LPARAM(dwPoint), 0, g_hMainWindow, NULL);
      }
      DestroyMenu(hMenu);
   }
}

#ifdef USE_SIP
void PositionSIP(int nSipState)
{
   // Do we have the sip function?
   if (g_pfnSipGetInfo && g_pfnSipSetInfo)
   {
      SIPINFO si = {0};
      si.cbSize = sizeof(SIPINFO);

      // See whether the SIP is up or down
      if ((*g_pfnSipGetInfo)(&si))
      {
         // Has the SIP state changed?
         if ((!(si.fdwFlags & SIPF_ON) && SIP_UP == nSipState) ||
             (si.fdwFlags & SIPF_ON && !(SIP_UP == nSipState)))
         {
            si.fdwFlags ^= SIPF_ON;
            (*g_pfnSipSetInfo)(&si);
         }
      }
   }
}
#endif

int ConnMCMessageBox(UINT idBody, UINT idTitle, UINT flags)
{
   TCHAR szTitle[256];
   TCHAR szBody[256];

   LoadString(g_hInstance, idBody, szBody,
              sizeof(szBody)/sizeof(*szBody));

   if (idTitle)
   {
      LoadString(g_hInstance, idTitle, szTitle,
                 sizeof(szTitle)/sizeof(*szTitle));
   }
   else
   {
      if (g_fPortrait)
         LoadString(g_hInstance, IDS_TITLE_PORTRAIT, szTitle,
                    sizeof(szTitle)/sizeof(*szTitle));
      else
         LoadString(g_hInstance, IDS_TITLE_LANDSCAPE, szTitle,
                    sizeof(szTitle)/sizeof(*szTitle));
   }

   return MessageBox(g_hMainWindow, szBody, szTitle, flags);
}

BOOL LaunchNdisNotificationThread()
{
	// Create the ndis file
	g_hNdisAccess = CreateFile(NDISUIO_DEVICE_NAME, 0, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		INVALID_HANDLE_VALUE);
	if (INVALID_HANDLE_VALUE == g_hNdisAccess)
		return FALSE;

	// Create the ndis message queue
	MSGQUEUEOPTIONS msgQueueOptions = {0};
	msgQueueOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
	msgQueueOptions.dwFlags = 0;
	msgQueueOptions.dwMaxMessages = 25;
	msgQueueOptions.cbMaxMessage = sizeof(NDISUIO_DEVICE_NOTIFICATION);
	msgQueueOptions.bReadAccess = TRUE;

	g_hNdisNotifyMsgQueue = CreateMsgQueue(NULL, &msgQueueOptions);
	if (NULL == g_hNdisNotifyMsgQueue)
	{
		CleanupNdisNotificationThread();
		return FALSE;
	}

	// Register our request with ndis
	NDISUIO_REQUEST_NOTIFICATION ndisRequestNotification = {0};
	ndisRequestNotification.hMsgQueue = g_hNdisNotifyMsgQueue;
	ndisRequestNotification.dwNotificationTypes = NDISUIO_NOTIFICATION_BIND | NDISUIO_NOTIFICATION_UNBIND;

	if (!DeviceIoControl(g_hNdisAccess, IOCTL_NDISUIO_REQUEST_NOTIFICATION,
		&ndisRequestNotification,
		sizeof(NDISUIO_REQUEST_NOTIFICATION),
		NULL, 0, NULL, NULL))
	{
		CleanupNdisNotificationThread();
		return FALSE;
	}

	// Create the event that will termate our notify thread
	g_hNdisNotifyTermiateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!g_hNdisNotifyTermiateEvent)
	{
		CleanupNdisNotificationThread();
		return FALSE;
	}

	// Create the thread that will watch for ndis notifications
	g_hNdisNotifyThread = CreateThread(NULL, 0, NdisNotificationThreadProc,
		NULL, 0, NULL);
	if (!g_hNdisNotifyThread)
	{
		CleanupNdisNotificationThread();
		return FALSE;
	}

	return TRUE;
}

DWORD NdisNotificationThreadProc(LPVOID lpVoid)
{
   HANDLE hWaitList[2];
   DWORD dwWaitObject;
   NDISUIO_DEVICE_NOTIFICATION ndisDeviceNotification = {0};
   DWORD dwBytesReturned = 0;
   DWORD dwFlags = 0;

   // Assert that PM is already initalized
   if (WAIT_OBJECT_0 != WaitForAPIReady(SH_SHELL, 0))
   {
	   DEBUGMSG(ZONE_ERROR, (TEXT("NdisNotificationThreadProc WaitForAPIReady SH_SHELL failed\r\n")));
	   ASSERT(0);
   }

   hWaitList[0] = g_hNdisNotifyMsgQueue;
   hWaitList[1] = g_hNdisNotifyTermiateEvent;

   while (TRUE)
   {
      dwWaitObject = WaitForMultipleObjects(sizeof(hWaitList)/sizeof(*hWaitList),
                                            hWaitList, FALSE, INFINITE);

      if (WAIT_OBJECT_0 == dwWaitObject)
      {
         while (ReadMsgQueue(g_hNdisNotifyMsgQueue, &ndisDeviceNotification,
                sizeof(ndisDeviceNotification), &dwBytesReturned, 1, &dwFlags))
         {
            if ((ndisDeviceNotification.dwNotificationType &
                 NDISUIO_NOTIFICATION_BIND) ||
                (ndisDeviceNotification.dwNotificationType &
                 NDISUIO_NOTIFICATION_UNBIND))
            {
               PostMessage(g_hMainWindow, WM_DEVICECHANGE, NULL, NULL);
            }
         }
      }
      else // Terminate event or error from WaitForMultipleObjects
      {
         break;
      }
   }

   return 0;
}

void CleanupNdisNotificationThread()
{
	if (INVALID_HANDLE_VALUE != g_hNdisAccess)
	{
		CloseHandle(g_hNdisAccess);
		g_hNdisAccess = INVALID_HANDLE_VALUE;
	}

	if (g_hNdisNotifyMsgQueue)
	{
		CloseMsgQueue(g_hNdisNotifyMsgQueue);
		g_hNdisNotifyMsgQueue = NULL;
	}

	if (g_hNdisNotifyThread)
	{
		SetEvent(g_hNdisNotifyTermiateEvent);
		WaitForSingleObject(g_hNdisNotifyTermiateEvent, INFINITE);
		CloseHandle(g_hNdisNotifyTermiateEvent);
		g_hNdisNotifyTermiateEvent = NULL;

		CloseHandle(g_hNdisNotifyThread);
		g_hNdisNotifyThread = NULL;
	}
}


void ParseConnMCCmdLine(LPWSTR pszCmdLine)
{
/* TODO - uncomment
   // Scan command line
   while (*pszCmdLine)
   {
      // Skip leading blanks
      while (' ' == *pszCmdLine) {
         pszCmdLine++;
      }
      if ('\0' == *pszCmdLine) {
         break;
      }
      if ((TEXT('-') == *pszCmdLine) || (TEXT('/') == *pszCmdLine)) {
         pszCmdLine++;

         while ((TEXT(' ') != *pszCmdLine) && (TEXT('\0') != *pszCmdLine)) {
            switch (*pszCmdLine) {
            case 'p' :
            case 'P' :
               g_fPortrait = TRUE;
               break;
            case 'l' :
            case 'L' :
               g_fPortrait = FALSE;
               break;
            case 'd' :
            case 'D' :
               // Get debug flag
               dpCurSettings.ulZoneMask = 0;
               pszCmdLine++;
               while ((TEXT('0') <= *pszCmdLine) && (TEXT('9') >= *pszCmdLine)) {
                  dpCurSettings.ulZoneMask *= 10;
                  dpCurSettings.ulZoneMask += *pszCmdLine - TEXT('0');
                  pszCmdLine++;
               }
               pszCmdLine--;
               DEBUGMSG (1, (TEXT("New ulZoneMask=%d\r\n"),
                          dpCurSettings.ulZoneMask));
               break;
            default :
               DEBUGMSG (1, (TEXT("Unknown switch '%s'\r\n"), pszCmdLine));
               break;
            }
            pszCmdLine++;
         }
      } else {
         DEBUGMSG (1, (TEXT("Bad commandline parm '%s'\r\n"), pszCmdLine));
         break;
      }
   }
*/
}



void TrimR(TCHAR * szSource)
{
	// Parameter check
	if (!szSource)
		return;
	
	// Empty string check
	if (!*szSource)
		return;
	
	TCHAR * pEnd = szSource-1;
	TCHAR * pCur = szSource;
	
	do
	{
		if (' ' != *pCur)
		{
			pEnd = pCur;
		}
	}while(*++pCur);
	
	if (*pEnd)
		*++pEnd = NULL;
}

void TrimL(TCHAR * szSource, size_t StrLen)
{
	// Parameter check
	if (!szSource || !*szSource || (StrLen < 1))
		return;
	
	// Does inplace space trimming on the left of the string
	TCHAR *pStart	= szSource;
	size_t Run = StrLen;
	
	while (*pStart && (' ' == *pStart) && Run > 0)
	{
		pStart++;
		Run--;
	}

	
	if (pStart != szSource)
	{
	
		size_t CopyLen = StrLen - (pStart - szSource);

		while(*pStart && CopyLen > 0)
		{
			*szSource++ = *pStart++;
			CopyLen--;
		}
		
		*szSource = NULL;
	}
}


void TrimLR(TCHAR *szSource)
{
	// param check
	if (!szSource)
		return;
	
	TrimR(szSource);
	if (*szSource)
		TrimL(szSource, _tcslen(szSource));
}


BOOL isValidNameString(TCHAR * pszValidate)
{
	// param check
	if (!pszValidate)
		return FALSE;
	
	for (int i = 0; i < (sizeof(g_szBadCharList)/sizeof(*g_szBadCharList)); i++)
	{
		if (::_tcschr(pszValidate, g_szBadCharList[i]))
			return FALSE;
	}
	
	return TRUE;
}


void ValidateName(TCHAR * pszNewName, UINT * pError)
{
	// param check
	if (!pszNewName || !pError)
		return;
	
	// Empty string check
	if (!*pszNewName)
	{
		*pError = IDS_ERR_NULLNAME;
		return;
	}
	
	
	// All spaces string check
	TrimLR(pszNewName);
	if (!*pszNewName)
	{
		*pError = IDS_ERR_ALL_SPACES;
		return;
	}
	
	
	// check for invalid characters
	BOOL bValidString = isValidNameString(pszNewName);
	
	if (!bValidString)
	{
		*pError = IDS_ERR_BADNAME;
		return;
	}

	// Make sure the name is unique in the list of items.
	LVFINDINFO FindInfo;
	FindInfo.flags = LVFI_STRING;
	FindInfo.psz = pszNewName;
	int Position = ListView_FindItem(g_hListView, -1, &FindInfo);

	if (Position != -1)
	{
		*pError = IDS_ERR_DUPLICATE;
	}
}
