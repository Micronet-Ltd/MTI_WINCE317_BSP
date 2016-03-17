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
#include "dccconninfo.h"

////////////////////////
// DccConnInfo
////////////////////////

UINT DccConnInfo::iconID = (UINT) -1;

DccConnInfo::DccConnInfo(TCHAR * pszName) :
   RasConnInfo(pszName)
{
   // This means that we are not a Direct connection, make ourselves one here
   if (_tcscmp(RasEntry.szDeviceType, RASDT_Direct))
   {
      LPRASDEVINFO pRasDevInfo = NULL;
      DWORD dwRasDevices = GetDevices(&pRasDevInfo);
      for (DWORD i=0; (i < dwRasDevices) && pRasDevInfo; i++)
      {
         if (!_tcscmp(pRasDevInfo[i].szDeviceType, RASDT_Direct))
         {
            _tcscpy(RasEntry.szDeviceName, pRasDevInfo[i].szDeviceName);
            _tcscpy(RasEntry.szDeviceType, RASDT_Direct);
            break;
         }
      }
      if (pRasDevInfo)
         delete [] pRasDevInfo;
   }
}

void DccConnInfo::getTypeAsDisplayString(HINSTANCE hInstance, TCHAR * pszType,
                                         DWORD cchType) const
{
   ::LoadString(hInstance, IDS_RAS_DIRECT, pszType, cchType);
}

HPROPSHEETPAGE DccConnInfo::getPropPage(UINT uPage, HINSTANCE hInstance)
{
   if (!pfnCreatePropertySheetPage)
      return NULL;

   PROPSHEETPAGE psp = {0};
   psp.dwSize = sizeof(PROPSHEETPAGE);
   psp.hInstance = hInstance;
   switch (uPage)
   {
      case 0:
         psp.pszTemplate = MAKEINTRESOURCE(g_fPortrait? IDD_DCC_0_P:IDD_DCC_0);
         psp.pfnDlgProc = DccConnInfo::DccConnPropSheetProc;
      break;
      default: return NULL;
   }
   psp.lParam = reinterpret_cast<LPARAM>(this);

   return (*pfnCreatePropertySheetPage)(&psp);
}

BOOL CALLBACK DccConnInfo::DccConnPropSheetProc(HWND hDlg, UINT uMsg,
                                                WPARAM wParam, LPARAM lParam)
{
	PROPSHEETPAGE * ppsp = reinterpret_cast<PROPSHEETPAGE *>(GetWindowLong(hDlg, GWL_USERDATA));
	DccConnInfo * pDccConn = NULL;
	if (ppsp)
		pDccConn = reinterpret_cast<DccConnInfo *>(ppsp->lParam);
	
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			::SetWindowLong(hDlg, GWL_USERDATA, lParam);
			ppsp = reinterpret_cast<PROPSHEETPAGE *>(lParam);
			if (!ppsp)
			{
				ASSERT(ppsp);
				return FALSE;
			}
			
			pDccConn = reinterpret_cast<DccConnInfo *>(ppsp->lParam);
			if (!pDccConn)
			{
				ASSERT(pDccConn);
				return FALSE;
			}
			
			// Fill in the modem list box
			LPRASDEVINFO pRasDevInfo = NULL;
			DWORD dwRasDevices = GetDevices(&pRasDevInfo);
			for (DWORD i=0; i < dwRasDevices; i++)
			{
				if (!_tcscmp(pRasDevInfo[i].szDeviceType, RASDT_Direct))
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
				0, (LPARAM) pDccConn->RasEntry.szDeviceName);
			
			// It is possible to have an entry that doesn't match a device name if it was created
			// using an outside tool (EG RasEntry).  This is not an error.
			//ASSERT(CB_ERR != lResult);
			
			::SendMessage(::GetDlgItem(hDlg, IDC_DEVICE), CB_SETCURSEL,
				(CB_ERR != lResult) ? lResult : 0, 0);
			
			// Modem combo box gets focus by default
			::SetFocus(::GetDlgItem(hDlg, IDC_DEVICE));
			
			return FALSE; // We will set our own focused control
		}
		break;
		
	case WM_COMMAND:
		{
			if (!pDccConn) {
				// pDccConn shouldn't be NULL
				return FALSE;
			}

			switch (LOWORD(wParam))
			{
            case IDC_CONFIG:
				pDccConn->handleDevConfig(hDlg);
				break;
				
            case IDC_TCPSETTINGS:
				pDccConn->ConfigureTcpIp(hDlg, ppsp->hInstance);
				break;
				
            case IDC_SECSETTINGS:
				pDccConn->ConfigureSecurity(hDlg, ppsp->hInstance);
				break;
				
            case IDC_DEVICE:
				{
					WCHAR wszNewDeviceName[RAS_MaxDeviceName+1];
					
					wszNewDeviceName[0] = L'\0';
					::GetWindowText(::GetDlgItem(hDlg, IDC_DEVICE),
						wszNewDeviceName,
						RAS_MaxDeviceName+1);
					if (0 != (wcscmp(pDccConn->RasEntry.szDeviceName, wszNewDeviceName)))
					{
						// If the device name changed then any prior devconfig
						// info is no longer valid, so free it.
						//
						// Note that if connmc wanted to be really smart, it could maintain
						// a cache of the devconfig info on a per-DeviceName basis. That way,
						// if the user changes the DeviceName back to one he previously
						// configured, the devconfig info for that device could be restored.
						//
						wcscpy(pDccConn->RasEntry.szDeviceName, wszNewDeviceName);
						delete [] pDccConn->pDevConfig;
						pDccConn->pDevConfig = NULL;
						pDccConn->cbDevConfig = 0;
					}
					
				}
				break;
			}
		}
		break;
		
	case WM_NOTIFY:
		{
			PSHNOTIFY * ppshn = (PSHNOTIFY *) lParam;
			
			if (!pDccConn) {
				// pDccConn shouldn't be NULL
				return FALSE;
			}
			
			switch (ppshn->hdr.code)
			{
            case PSN_SETACTIVE:
				{
					PropSheet_SetWizButtons(::GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
					
					// Set the connection name.
					::SetWindowText(::GetDlgItem(hDlg, IDC_CONNLABEL), pDccConn->pszName);
				}
				break;
				
            case PSN_WIZFINISH: // WIZFINISH doesn't send a KILLACTIVE so simulate here
            case PSN_KILLACTIVE:
				{
					::GetWindowText(::GetDlgItem(hDlg, IDC_DEVICE),
						pDccConn->RasEntry.szDeviceName,
						RAS_MaxDeviceName+1);
				}
				if (PSN_WIZFINISH != ppshn->hdr.code)
					break;
				
            case PSN_APPLY:
				{
					pDccConn->writeChanges();
				}
				break;
			}
			::SetWindowLong(hDlg, DWL_MSGRESULT, 0);
		}
		break;
		
	case WM_DEVICECHANGE:
		{
			if (!pDccConn) {
				// pDccConn shouldn't be NULL
				return FALSE;
			}

			// Refresh the modem list box
			HCURSOR hCursorSave = SetCursor(LoadCursor(NULL, IDC_WAIT));
			::SendDlgItemMessage(hDlg, IDC_DEVICE, CB_RESETCONTENT, 0, 0);
			LPRASDEVINFO pRasDevInfo = NULL;
			DWORD dwRasDevices = GetDevices(&pRasDevInfo);
			for (DWORD i=0; (i < dwRasDevices) && pRasDevInfo; i++)
			{
				if (!_tcscmp(pRasDevInfo[i].szDeviceType, RASDT_Direct))
				{
					::SendDlgItemMessage(hDlg, IDC_DEVICE, CB_ADDSTRING, 0,
						(LPARAM) pRasDevInfo[i].szDeviceName);
				}
			}
			if (pRasDevInfo)
				delete [] pRasDevInfo;
			
			// Try to find the modem in the list.
			LRESULT lResult = ::SendDlgItemMessage(hDlg, IDC_DEVICE, CB_FINDSTRINGEXACT,
				0, (LPARAM) pDccConn->RasEntry.szDeviceName);
			ASSERT(CB_ERR != lResult);
			::SendMessage(::GetDlgItem(hDlg, IDC_DEVICE), CB_SETCURSEL,
				(CB_ERR != lResult) ? lResult : 0, 0);
			SetCursor(hCursorSave);
		}
		break;
		
	default:
		return FALSE;
   }
   
   return TRUE;
}

