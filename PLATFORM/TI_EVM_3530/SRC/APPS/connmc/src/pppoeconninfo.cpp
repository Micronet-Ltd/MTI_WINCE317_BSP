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
#include "pppoeconninfo.h"
#include "connmc.h" // For RNM_SHOWERROR

////////////////////////
// PPPOEConnInfo
////////////////////////

UINT PPPOEConnInfo::iconID = (UINT) -1;

PPPOEConnInfo::PPPOEConnInfo(TCHAR * pszName) :
   RasConnInfo(pszName)
{
   // This means that we are not PPPoE, make ourselves one here
   if (_tcscmp(RasEntry.szDeviceType, RASDT_PPPoE))
   {
      LPRASDEVINFO pRasDevInfo = NULL;
      DWORD dwRasDevices = GetDevices(&pRasDevInfo);

      for (DWORD i=0; i < dwRasDevices; i++)
      {
         if (!_tcscmp(pRasDevInfo[i].szDeviceType, RASDT_PPPoE))
         {
            _tcscpy(RasEntry.szDeviceName, pRasDevInfo[i].szDeviceName);
            _tcscpy(RasEntry.szDeviceType, RASDT_PPPoE);
            RasEntry.dwfOptions = RASEO_RequireMsEncryptedPw |
                                  RASEO_RequireDataEncryption|
                                  RASEO_ProhibitEAP          | 
                                  RASEO_ProhibitPAP          |
                                  RASEO_ProhibitCHAP;
            break;
         }
      }
      if (pRasDevInfo)
         delete [] pRasDevInfo;

   }
   
}

void PPPOEConnInfo::getTypeAsDisplayString(HINSTANCE hInstance, TCHAR * pszType,
                                         DWORD cchType) const
{
   ::LoadString(hInstance, IDS_RAS_PPPOE, pszType, cchType);
}

HPROPSHEETPAGE PPPOEConnInfo::getPropPage(UINT uPage, HINSTANCE hInstance)
{
   if (!pfnCreatePropertySheetPage)
      return NULL;

   PROPSHEETPAGE psp = {0};
   psp.dwSize = sizeof(PROPSHEETPAGE);
   psp.hInstance = hInstance;
   switch (uPage)
   {
      case 0:
         psp.pszTemplate = MAKEINTRESOURCE(g_fPortrait?IDD_PPPOE_0_P:IDD_PPPOE_0);
         psp.pfnDlgProc = PPPOEConnInfo::PPPOEConnPropSheetProc;
      break;
      default: return NULL;
   }
   psp.lParam = reinterpret_cast<LPARAM>(this);

   return (*pfnCreatePropertySheetPage)(&psp);
}

BOOL CALLBACK PPPOEConnInfo::PPPOEConnPropSheetProc(HWND hDlg, UINT uMsg,
                                                WPARAM wParam, LPARAM lParam)
{
   PROPSHEETPAGE * ppsp = reinterpret_cast<PROPSHEETPAGE *>(GetWindowLong(hDlg, GWL_USERDATA));
   PPPOEConnInfo * pPPPOEConn = NULL;
   if (ppsp) 
      pPPPOEConn = reinterpret_cast<PPPOEConnInfo *>(ppsp->lParam);

   switch (uMsg)
   {
      case WM_INITDIALOG:
      {
         ::SetWindowLong(hDlg, GWL_USERDATA, lParam);
         ppsp = reinterpret_cast<PROPSHEETPAGE *>(lParam);
         if (!ppsp)
         {
            ASSERT(0);
            return FALSE;
         }
         
         pPPPOEConn = reinterpret_cast<PPPOEConnInfo *>(ppsp->lParam);

         if (!pPPPOEConn)
         {
            ASSERT(0);
            return FALSE;
         }

		 //
		 //	Fill in the PPPoE lines in the list box.
		 //
		
		 LPRASDEVINFO pRasDevInfo = NULL;
		 DWORD dwRasDevices = GetDevices(&pRasDevInfo);
		 for (DWORD i=0; i < dwRasDevices; i++)
		 {
		 	 if (!_tcscmp(pRasDevInfo[i].szDeviceType, RASDT_PPPoE))
			 {
			 	 ::SendMessage(GetDlgItem (hDlg, IDC_DEVICE),
				 	 CB_ADDSTRING, 0,
					 (LPARAM) pRasDevInfo[i].szDeviceName);
			 }
		 }
		 
		 if (pRasDevInfo)
	 		 delete [] pRasDevInfo;

		 // Try to find the modem in the list.
		 LRESULT lResult = SendMessage(::GetDlgItem(hDlg, IDC_DEVICE),
							 	 CB_FINDSTRINGEXACT,
								 0, (LPARAM) pPPPOEConn->RasEntry.szDeviceName);
		 
		 ::SendMessage(::GetDlgItem(hDlg, IDC_DEVICE), CB_SETCURSEL,
										(CB_ERR != lResult) ? lResult : 0, 0);

		 // List box gets focus by default
		 ::SetFocus(::GetDlgItem(hDlg, IDC_DEVICE));
				

         // Limit the phone number length
         ::SendMessage(::GetDlgItem(hDlg, IDC_HOSTNAME), EM_LIMITTEXT,
                       RAS_MaxPhoneNumber, 0);

         // Set the phone number here
         ::SetDlgItemText(hDlg, IDC_HOSTNAME,
                          pPPPOEConn->RasEntry.szLocalPhoneNumber);         

         // Pnone number box gets focus by default
         ::SetFocus(::GetDlgItem(hDlg, IDC_HOSTNAME));

         return FALSE; // We will set our own focused control
      }
      break;

      case WM_COMMAND:
      {
         if (!pPPPOEConn)
         {
            ASSERT(0);
            return FALSE;
         }
         
         switch (LOWORD(wParam))
         {
            case IDC_TCPSETTINGS:
               pPPPOEConn->ConfigureTcpIp(hDlg, ppsp->hInstance);
            break;

            case IDC_SECSETTINGS:
               pPPPOEConn->ConfigureSecurity(hDlg, ppsp->hInstance);
            break;
         }
      }
      break;

      case WM_NOTIFY:
      {
         LONG lReturn = 0;
         PSHNOTIFY * ppshn = (PSHNOTIFY *) lParam;

         if (!pPPPOEConn)
         {
             ASSERT(0);
             return FALSE;
         }

         switch (ppshn->hdr.code)
         {
            case PSN_SETACTIVE:
            {
               PropSheet_SetWizButtons(::GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);

               // Set the connection name.
               ::SetWindowText(::GetDlgItem(hDlg, IDC_CONNLABEL), pPPPOEConn->pszName);
            }
            break;

            case PSN_WIZFINISH: // WIZFINISH doesn't send a KILLACTIVE so simulate here
            case PSN_KILLACTIVE:
            {                
			   ::GetWindowText(::GetDlgItem(hDlg, IDC_DEVICE),
								pPPPOEConn->RasEntry.szDeviceName,
								RAS_MaxDeviceName+1);
				
               ::GetDlgItemText(hDlg, IDC_HOSTNAME,
                                pPPPOEConn->RasEntry.szLocalPhoneNumber,
                                RAS_MaxPhoneNumber+1);
            }
            if (PSN_WIZFINISH != ppshn->hdr.code)
               break;

            case PSN_APPLY:
            {
                //                
                //  Service name can be anything including NULL.                
                //              
                //  The following examles are all valid:
                // 
                //  AC Name\                          -> Connect to the default service on the specified AC
                //  Service Name                      -> Connect to the specified service on any AC
                //  AC Name\Service Name              -> Connect to the specified service on the specified AC
                //  [Blank]                           -> Connect to the default service on any AC
                //    
                
               pPPPOEConn->writeChanges();
            }
            break;
         }
         ::SetWindowLong(hDlg, DWL_MSGRESULT, lReturn);
      }
      break;

      default:
         return FALSE;
   }

   return TRUE;
}

