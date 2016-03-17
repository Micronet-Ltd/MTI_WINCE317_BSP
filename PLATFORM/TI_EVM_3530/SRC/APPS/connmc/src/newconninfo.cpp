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
#include "newconninfo.h"
#include "rnaconninfo.h"
#include "dccconninfo.h"
#include "vpnconninfo.h"
#include "pppoeconninfo.h"
#include "connmc.h" // For RNM_SHOWERROR


////////////////////////
// NewConnInfo
////////////////////////

UINT NewConnInfo::iconID = (UINT) -1;

NewConnInfo::NewConnInfo(TCHAR * pszName)
{
   if (!pszName)
      return;

   this->pszName = new TCHAR[lstrlen(pszName)+1];
   if (this->pszName)
      lstrcpy(this->pszName, pszName);
}

BOOL NewConnInfo::getDefaultActionString(HINSTANCE hInstance, TCHAR * pszDefaultAction, UINT * pError)
{
   if (!pszDefaultAction)
   {
      if (pError)
         *pError = IDS_ERROR;
      return FALSE;
   }

   ::LoadString(hInstance, IDS_NEW_NEW, pszDefaultAction, 128);
   return TRUE;
}


BOOL NewConnInfo::defaultAction(HINSTANCE hInstance, HWND hParent, UINT * pError)
{
   if (!pfnPropertySheet)
   {
      if (pError) *pError = IDS_ERROR;
      return FALSE;
   }

   PROPSHEETHEADER psh = {0};
   HPROPSHEETPAGE phpsp[1] = {0};
   phpsp[0] = getPropPage(0, hInstance);
   if (!phpsp[0])
   {
      if (pError)
         *pError = IDS_ERROR;
      return FALSE;
   }

   psh.dwSize = sizeof(psh);
   psh.dwFlags = PSP_HASHELP | PSH_NOAPPLYNOW | PSH_WIZARD;
   psh.hwndParent = hParent;
   psh.hInstance = hInstance;
   psh.nPages = 1;
   psh.phpage = phpsp;

   (*pfnPropertySheet)(&psh);

   return TRUE;
}

BOOL NewConnInfo::allowRemove(BOOL fInGroup, UINT * pReason) const
{
   if (pReason)
      *pReason = IDS_ERR_DEL_NEW;
   return FALSE;
}

BOOL NewConnInfo::allowRename(BOOL fInGroup, UINT * pReason) const
{
   if (pReason)
      *pReason = IDS_ERR_RENAME_NEW;
   return FALSE;
}

BOOL NewConnInfo::allowShortcut(BOOL fInGroup, UINT * pReason) const
{
   if (pReason)
      *pReason = IDS_ERR_SHORTCUT_NEW;
   return FALSE;
}

HPROPSHEETPAGE NewConnInfo::getPropPage(UINT uPage, HINSTANCE hInstance)
{
   if (!pfnCreatePropertySheetPage)
      return NULL;

   PROPSHEETPAGE psp = {0};
   psp.dwSize = sizeof(PROPSHEETPAGE);
   psp.hInstance = hInstance;
   switch (uPage)
   {
      case 0: psp.pszTemplate = MAKEINTRESOURCE(g_fPortrait? IDD_WIZ_NEW_P:IDD_WIZ_NEW); break;
      default: return NULL;
   }
   psp.pfnDlgProc = NewConnInfo::NewConnPropSheetProc;
   psp.lParam = reinterpret_cast<LPARAM>(this);

   return (*pfnCreatePropertySheetPage)(&psp);
}

BOOL NewConnInfo::allowProperties(BOOL fInGroup, UINT * pReason) const
{
   if (pReason)
      *pReason = IDS_ERROR;
   return FALSE;
}


BOOL CALLBACK NewConnInfo::NewConnPropSheetProc(HWND hDlg, UINT uMsg,
                                                WPARAM wParam, LPARAM lParam)
{
   PROPSHEETPAGE * ppsp = reinterpret_cast<PROPSHEETPAGE *>(GetWindowLong(hDlg, GWL_USERDATA));
   NewConnInfo * pNewConn = NULL;
   if (ppsp) pNewConn = reinterpret_cast<NewConnInfo *>(ppsp->lParam);
   static ConnInfo * pTempConnInfo = NULL;
   static DWORD dwPageCount = 0;

   switch (uMsg)
   {
      case WM_INITDIALOG:
      {
         ::SetWindowLong(hDlg, GWL_USERDATA, lParam);
         ppsp = reinterpret_cast<PROPSHEETPAGE *>(lParam);
         ASSERT(ppsp);
         pNewConn = reinterpret_cast<NewConnInfo *>(ppsp->lParam);
         ASSERT(pNewConn);

         // Set the connection name.
         TCHAR szConnName[RAS_MaxEntryName+1];
         RasConnInfo::GetUnusedRasName(ppsp->hInstance, szConnName);
         ::SendMessage(::GetDlgItem (hDlg, IDC_REMNAME), EM_LIMITTEXT,
                       RAS_MaxEntryName, 0);
         ::SetWindowText(::GetDlgItem(hDlg, IDC_REMNAME), szConnName);
         ::SendMessage(::GetDlgItem (hDlg, IDC_REMNAME), EM_SETSEL, 0, -1);

         // Enable or disable the radio buttons based on what devices are in the OS
         // and check the default button
         BOOL fHasRNA   = RasConnInfo::HasDevices(RASDT_Modem);
         BOOL fHasDCC   = RasConnInfo::HasDevices(RASDT_Direct);
         BOOL fHasPPTPVPN   = RasConnInfo::HasDevices(RASDT_Vpn, PPTPDeviceNamePrefix);
         BOOL fHasL2TPVPN   = RasConnInfo::HasDevices(RASDT_Vpn, L2TPDeviceNamePrefix);
         BOOL fHasPPPOE = RasConnInfo::HasDevices(RASDT_PPPoE);         

         EnableWindow(GetDlgItem(hDlg, IDC_NEW_RNA),   fHasRNA);
         EnableWindow(GetDlgItem(hDlg, IDC_NEW_DCC),   fHasDCC);
         EnableWindow(GetDlgItem(hDlg, IDC_NEW_VPN),   fHasPPTPVPN);
         EnableWindow(GetDlgItem(hDlg, IDC_NEW_L2TPVPN), fHasL2TPVPN);
         EnableWindow(GetDlgItem(hDlg, IDC_NEW_PPPOE), fHasPPPOE);

         if (fHasRNA)
            ::CheckRadioButton(hDlg, IDC_NEW_CONN_MIN, IDC_NEW_CONN_MAX, IDC_NEW_RNA);
         else if (fHasDCC)
            ::CheckRadioButton(hDlg, IDC_NEW_CONN_MIN, IDC_NEW_CONN_MAX, IDC_NEW_DCC);
         else if (fHasPPTPVPN)
            ::CheckRadioButton(hDlg, IDC_NEW_CONN_MIN, IDC_NEW_CONN_MAX, IDC_NEW_VPN);
         else if (fHasL2TPVPN)
            ::CheckRadioButton(hDlg, IDC_NEW_CONN_MIN, IDC_NEW_CONN_MAX, IDC_NEW_L2TPVPN);
         else if (fHasPPPOE)
            ::CheckRadioButton(hDlg, IDC_NEW_CONN_MIN, IDC_NEW_CONN_MAX, IDC_NEW_PPPOE);

            ;// No available connection types, check nothing

         // Set up the page counter
         dwPageCount = 1;

         // Edit box gets focus by default
         ::SetFocus(::GetDlgItem(hDlg, IDC_REMNAME));
         return FALSE; // We will set our own focused control
      }
      break;

      case WM_NOTIFY:
      {
         LONG lReturn = 0;
         PSHNOTIFY * ppshn = (PSHNOTIFY *) lParam;
         switch (ppshn->hdr.code)
         {
            case PSN_SETACTIVE:
            {
               // Only enable next if there is at least one kind of device available
               if (RasConnInfo::HasDevices(RASDT_Modem) ||
                   RasConnInfo::HasDevices(RASDT_Direct) ||
                   RasConnInfo::HasDevices(RASDT_Vpn)    ||
                   RasConnInfo::HasDevices(RASDT_PPPoE))
               {
                  PropSheet_SetWizButtons(::GetParent(hDlg), PSWIZB_NEXT);
               }
               else
               {
                  PropSheet_SetWizButtons(::GetParent(hDlg), 0);
               }
            }
            break;

            case PSN_WIZNEXT:
            {
               // Create the new temp object to store user data in, or keep the
               // old object if the user is going into the same page
               BOOL fRefreshPropSheetPages = TRUE;
               TCHAR szName[RAS_MaxEntryName+1] = _T("");
               ::GetDlgItemText(hDlg, IDC_REMNAME, szName,
                                sizeof(szName)/sizeof(*szName));

				// validate the name the user entered
               UINT uError = 0;
			   ValidateName(szName, &uError); // will also trim leading / trailing spaces
               if (0 != uError)
               {
                  ::EnableWindow(::GetParent(hDlg), FALSE);
                  ::SendMessage(::GetParent(::GetParent(hDlg)),
                                RNM_SHOWERROR, NULL, uError);
                  ::EnableWindow(::GetParent(hDlg), TRUE);
                  ::SetFocus(::GetDlgItem(hDlg, IDC_REMNAME));

				   // Put the possibly trimmed string back into the dialog
				  ::SetDlgItemText(hDlg, IDC_REMNAME, szName);

                  lReturn = 1;
                  break;
			   }
			   // Put the possibly trimmed string back into the dialog
			  ::SetDlgItemText(hDlg, IDC_REMNAME, szName);


               if (BST_CHECKED == ::SendMessage(GetDlgItem(hDlg, IDC_NEW_RNA),
                                                BM_GETCHECK, NULL, NULL))
               {
                  if (!pTempConnInfo)
                  {
                     pTempConnInfo = new RnaConnInfo(szName);
                  }
                  else if (pTempConnInfo->isType(_T("RnaConnInfo")))
                  {
                     pTempConnInfo->unsafeRename(szName);
                     fRefreshPropSheetPages = (2 > dwPageCount); // Keep any work the user did, if any
                  }
                  else
                  {
                     delete pTempConnInfo;
                     pTempConnInfo = new RnaConnInfo(szName);
                  }
               }
               else if (BST_CHECKED == ::SendMessage(GetDlgItem(hDlg, IDC_NEW_DCC),
                                                     BM_GETCHECK, NULL, NULL))
               {
                  if (!pTempConnInfo)
                  {
                     pTempConnInfo = new DccConnInfo(szName);
                  }
                  else if (pTempConnInfo->isType(_T("DccConnInfo")))
                  {
                     pTempConnInfo->unsafeRename(szName);
                     fRefreshPropSheetPages = (2 > dwPageCount); // Keep any work the user did, if any
                  }
                  else
                  {
                     delete pTempConnInfo;
                     pTempConnInfo = new DccConnInfo(szName);
                  }
               }
               else if ((BST_CHECKED == ::SendMessage(GetDlgItem(hDlg, IDC_NEW_VPN),
                                                     BM_GETCHECK, NULL, NULL))
                          ||(BST_CHECKED == ::SendMessage(GetDlgItem(hDlg, IDC_NEW_L2TPVPN),
                                                     BM_GETCHECK, NULL, NULL)))
               {
                  BOOL fL2TP = (BST_CHECKED == ::SendMessage(GetDlgItem(hDlg, IDC_NEW_L2TPVPN),
                                                     BM_GETCHECK, NULL, NULL));
                  if (!pTempConnInfo)
                  {
                     pTempConnInfo = new VpnConnInfo(szName, fL2TP);
                  }
                  else if (pTempConnInfo->isType(_T("VpnConnInfo")))
                  {
                     pTempConnInfo->unsafeRename(szName);
                     fRefreshPropSheetPages = (2 > dwPageCount); // Keep any work the user did, if any
                  }
                  else
                  {
                     delete pTempConnInfo;
                     pTempConnInfo = new VpnConnInfo(szName, fL2TP);
                  }
               }
               else if (BST_CHECKED == ::SendMessage(GetDlgItem(hDlg, IDC_NEW_PPPOE),
                                                     BM_GETCHECK, NULL, NULL))
               {
                  if (!pTempConnInfo)
                  {
                     pTempConnInfo = new PPPOEConnInfo(szName);
                  }
                  else if (pTempConnInfo->isType(_T("PPPOEConnInfo")))
                  {
                     pTempConnInfo->unsafeRename(szName);
                     fRefreshPropSheetPages = (2 > dwPageCount); // Keep any work the user did, if any
                  }
                  else
                  {
                     delete pTempConnInfo;
                     pTempConnInfo = new PPPOEConnInfo(szName);
                  }
               }

               // Check for out of memory
               ASSERT(pTempConnInfo);
               if (!pTempConnInfo)
               {
                  ::EnableWindow(::GetParent(hDlg), FALSE);
                  ::SendMessage(::GetParent(::GetParent(hDlg)), RNM_SHOWERROR,
                                NULL, IDS_ERR_OOM);
                  ::EnableWindow(::GetParent(hDlg), TRUE);
                  ::SetFocus(::GetDlgItem(hDlg, IDC_REMNAME));
                  lReturn = 1;
                  break;
               }

               // Do error checking on the state of the object
               pTempConnInfo->isValid(&uError);
               if (IDS_ERR_DOES_NOT_EXIST != uError)
               {
                  ::EnableWindow(::GetParent(hDlg), FALSE);
                  ::SendMessage(::GetParent(::GetParent(hDlg)),
                                RNM_SHOWERROR, NULL, uError);
                  ::EnableWindow(::GetParent(hDlg), TRUE);
                  ::SetFocus(::GetDlgItem(hDlg, IDC_REMNAME));
                  lReturn = 1;
                  break;
               }

               if (fRefreshPropSheetPages)
               {
                  // Clean up any pages that already exist (user pressed back at some point)
                  while (1 < dwPageCount)
                  {
                     PropSheet_RemovePage(::GetParent(hDlg), dwPageCount-1, NULL);
                     dwPageCount--;
                  }

                  // Add the new pages to the end of the list
                  HPROPSHEETPAGE hpsp = NULL;
                  for (DWORD i = 0; TRUE; i++)
                  {
                     hpsp = pTempConnInfo->getPropPage(i, ppsp->hInstance);
                     if (!hpsp)
                        break;
                     PropSheet_AddPage(::GetParent(hDlg), hpsp);
                     dwPageCount++;
                  }
               }
            }
            break;
         }
         ::SetWindowLong(hDlg, DWL_MSGRESULT, lReturn);
      }
      break;

      case WM_DESTROY:
      {
         // Get rid of the temp data here, the listview refresh will create it again
         if (pTempConnInfo)
         {
            delete pTempConnInfo;
            pTempConnInfo = NULL;
         }
      }
      break;

      default:
         return FALSE;
   }

   return TRUE;
}

