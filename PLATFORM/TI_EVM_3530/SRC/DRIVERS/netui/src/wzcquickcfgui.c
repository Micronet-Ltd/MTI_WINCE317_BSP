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
/*
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:

	WzcQuickCfgUi.c

Abstract:

	This file contains the code for the Zero Config Property Page.

Environment:

	WinCE

*/



#include "QuickConfigUI.h"

#define WM_REFRESH_WZC		(WM_USER + 10)
#define WM_SETWZCSTATUS		(WM_USER + 11)
#define WM_SETWZCSIGSTRNTH	(WM_USER + 12)
#define WM_REFRESH_BUTTONS	(WM_USER + 13)


// Global data
HANDLE 	g_hStopUpdateStatus;
BOOL	g_fConnecting;
BOOL	g_fConfiguring;
BOOL	g_fPopup;
WCHAR	g_wszPreferred[MAX_PREFERRED_STR];
DWORD	g_cchPreferred;
WCHAR	g_wszAddNew[MAX_ADDNEW_STR];
WCHAR	g_wszConnect[MAX_CONTEXT_STR];
WCHAR	g_wszConfigure[MAX_CONTEXT_STR];
WCHAR	g_wszDelete[MAX_CONTEXT_STR];
BOOL	g_fCheckAdhocPackets = FALSE;

#define ARRAY_SIZE(a)       (sizeof(a) / sizeof((a)[0]))

/*++

Routine Name: 

	WZCQuickCfgDialogProc

Routine Description:

	Dialog Proc for the Zero Config property page.  This function handles all the
	messages encountered on the property page.

Arguments:

	hwndDlg:	HWND of the property page
	uMsg:		The message which was generated
	wParam:		Additional data passed to the dialog
	lParam:		Additional data passed to the dialog

Return Value:

	BOOL (TRUE/FALSE)

--*/

BOOL CALLBACK WZCQuickCfgDialogProc(HWND hwndDlg,
									   UINT uMsg,
									   WPARAM wParam,
									   LPARAM lParam)
{
	WPARAM					wCmdId;
	BOOL 					fRetVal 				= FALSE;
	HRESULT					hr 						= S_OK;
	SystrayIcon				*pSysIcon 				= NULL;
	NMHDR					*header 				= NULL;
		
	switch(uMsg)
	{
		case WM_INITDIALOG:
				
			DEBUGMSG(ZONE_MISC, (TEXT("NetUIQC - InitDialog")));
			OnInitWZCDialog(hwndDlg);

			fRetVal = TRUE;
			break;

		case WM_REFRESH_WZC:

			WZCQCFG_LOCK;

			// Get the systray icon for the given window
			hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
			if(FAILED(hr) || !pSysIcon)
			{
				WZCQCFG_UNLOCK;
				fRetVal = FALSE;
				break;
			}

			hr = RefreshWirelessInfo(pSysIcon);
			if(hr != S_OK)
			{
				DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Error refreshing wireless info")));
			}

			WZCQCFG_UNLOCK;

			fRetVal = FALSE;
			break;

		case WM_SETWZCSIGSTRNTH:

			fRetVal = SetWZCSignalStrength(GetParent(hwndDlg), (INT)lParam);
			if(fRetVal == FALSE)
			{
				DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Error setting signal strength")));
			}

			fRetVal = FALSE;
			break;

		case WM_SETWZCSTATUS:

			DisplayWZCStatus(hwndDlg, (PTCHAR)lParam);
			LocalFree((PTCHAR)lParam);

			fRetVal = FALSE;
			break;

		case WM_REFRESH_BUTTONS:

			hr = RefreshWZCButtons(hwndDlg);
			if(FAILED(hr))
			{
				DEBUGMSG(ZONE_MISC, (TEXT("NetUIQC - Error refreshing buttons for WZC property page.")));
			}

			WZCQCFG_LOCK;

			// If we were refreshing buttons after a connect then let's also make sure
			// the associated network is visible
			if(g_fConnecting)
			{
				g_fConnecting = FALSE;

				// Get the systray icon for the given window
				hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
				if(FAILED(hr) || !pSysIcon)
				{
					WZCQCFG_UNLOCK;
					fRetVal = FALSE;
					break;
				}
				
				SelectAssociatedNetwork(pSysIcon->WLanInfo.hwndVLV);
			}

			WZCQCFG_UNLOCK;

			fRetVal = FALSE;
			break;

		case WM_TIMER:

			// Stop the timer and do not restart it until end of refresh thread
			KillTimer(hwndDlg, 1);

			WZCQCFG_LOCK;

			// Get the systray icon for the given window
			hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
			if(FAILED(hr) || !pSysIcon)
			{
				WZCQCFG_UNLOCK;
				fRetVal = FALSE;
				break;
			}

			// Wait for the current refresh thread to exit before creating it again
			if(pSysIcon->hThreads[THREAD_REFRESH_WZC])
			{
				if(WAIT_TIMEOUT == WaitForSingleObject(pSysIcon->hThreads[THREAD_REFRESH_WZC], 0))
				{
					WZCQCFG_UNLOCK;
					SetTimer(hwndDlg, 1, WZCQC_RFSH_TIMEOUT, 0);
					fRetVal = FALSE;
					break;
				}

				CloseHandle(pSysIcon->hThreads[THREAD_REFRESH_WZC]);
			}
			
			pSysIcon->hThreads[THREAD_REFRESH_WZC] = CreateThread(NULL, 0, RefreshZeroConfigThread, (LPVOID)hwndDlg, 0, NULL);
			if(pSysIcon->hThreads[THREAD_REFRESH_WZC] == NULL)
			{
				DEBUGMSG(ZONE_ERROR, (TEXT("NetUIQC - Error creating thread")));
				fRetVal = FALSE;
				break;
			}

			WZCQCFG_UNLOCK;

			fRetVal = FALSE;
			break;	

		case WM_NOTIFY:
			header = (NMHDR*) lParam;

			switch(header->code)
			{
				case LVN_ITEMCHANGED:

					hr = RefreshWZCButtons(hwndDlg);
					if(FAILED(hr))
					{
						DEBUGMSG(ZONE_MISC, (TEXT("NetUIQC - Error refreshing buttons for WZC property page.")));
					}
					
					fRetVal = FALSE;
					break;

				case NM_RCLICK:

					OnListRClick(hwndDlg);
					
					fRetVal = FALSE;
					break;

				case NM_CLICK:
					
					if (GetKeyState(VK_MENU) < 0)
						OnListRClick(hwndDlg);

					fRetVal = FALSE;
					break;

				case NM_DBLCLK:

					OnListDblClick(hwndDlg);
					
					fRetVal = FALSE;
					break;

				case PSN_APPLY:

					OnWZCOk(hwndDlg);
					// Fall through to PSN_RESET

				case PSN_RESET:
					AddListConfigsToCache(hwndDlg);
					OnCloseWZCDialog(hwndDlg);

					fRetVal = FALSE;
					break;
					

				default:
					fRetVal = FALSE;
					break;
			}

			break;

		case WM_COMMAND:

			wCmdId  = LOWORD(wParam);
			switch(wCmdId)
			{
				case IDM_WZC_CONFIGURE:

					OnConfigureNetwork(hwndDlg);
					break;

				case IDM_WZC_DELETE:

					OnDeleteNetwork(hwndDlg);
					break;
				
				case IDC_WZCQCFG_ADVANCED:
					
					OnAdvanced(hwndDlg);
					break;
										
				case IDM_WZC_CONNECT:
				case IDC_WZCQCFG_CONNECT:

					OnConnect(hwndDlg);
					break;

				case IDC_WZCQCFG_LOGGING:
					
					OpenLoggingWindow();
					break;

				default:
					break;
			}

			fRetVal = FALSE;
			break;

		default:
			fRetVal = FALSE;
			break;
	}

	return fRetVal;

} // WZCQuickCfgDialogProc



/*++

Routine Name:

	RefreshZeroConfigThread

Routine Description:

	This thread refreshes ZeroConfig and queries it for updated networks.

Arguments:

	pv:	HWND of the wireless dialog whose adapter should be queried

--*/

DWORD WINAPI RefreshZeroConfigThread(LPVOID pv)
{
	HWND 					hwndDlg 				= (HWND) pv;
	HRESULT					hr						= S_OK;
	DWORD					dwOutFlags				= 0;
	INT						i 						= 0;
	SystrayIcon*			pSysIcon				= NULL;
	BOOL					fAssociated 			= FALSE;
	WLAN_CONFIG_LISTENT		*pConfig 				= NULL;
	DWORD					dwWait;
	HANDLE					h;

	WZCQCFG_LOCK;

	DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: --- Refresh Zero Config ---")));

	// Get the systray icon for the given window
	hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
	if(FAILED(hr) || !pSysIcon)
	{
		goto exit;
	}

	// Refresh Zero config
	hr = pfnWZCRefreshInterfaceEx(
                            NULL, 
                            INTF_LIST_SCAN,
                            &pSysIcon->WLanInfo.IntfEntryEx,
                            &dwOutFlags);
	if(hr != ERROR_SUCCESS)
	{
		goto exit;
	}

	h = pSysIcon->hCloseWZCDialogEvent;

	WZCQCFG_UNLOCK;
	dwWait = WaitForSingleObject(h, WZC_RFSH_TIMEOUT);
	WZCQCFG_LOCK;

	if (dwWait != WAIT_TIMEOUT)
	{
		goto exit;
	}

	// If during the above sleep, the adapter was removed (since we were unlocked)
	// then this call will fail and we will abort.
	pSysIcon = NULL;
	hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
	if(FAILED(hr) || !pSysIcon)
	{
		goto exit;
	}

exit:

	PostMessage(hwndDlg, WM_REFRESH_WZC, 0, 0);

	// Make sure the window still exists (after sleeping) before setting the timer
	if(IsWindow(hwndDlg))
	{
		if(pSysIcon)
		{
			if(pSysIcon->dwWZCState == WZC_PREFERED_LIST_EXHAUSTED)
			{
				SetTimer(hwndDlg, 1, WZCQC_RFSH_TIMEOUT, NULL);	
			}	
			else
			{
				SetTimer(hwndDlg, 1, WZC_RFSH_ASSOCIATED_TIMEOUT, NULL);
			}
		}
		else
		{
			SetTimer(hwndDlg, 1, WZCQC_RFSH_TIMEOUT, NULL);	
		}
	}
	
	WZCQCFG_UNLOCK;
	
	return 0;

} // RefreshZeroConfigThread



/*++

Routine Name:

	GetInfraMode

Routine Description:

	Get the Infrastructure Mode of a config based on the list view image

Arguments:

	iImage:	Image index in the list view

Return Value:

	The infrastructure mode

--*/

NDIS_802_11_NETWORK_INFRASTRUCTURE GetInfraMode(INT iImage)
{
	NDIS_802_11_NETWORK_INFRASTRUCTURE InfraMode;
	
	if(iImage == WZCIMG_INFRA_ACTIVE || 
		iImage == WZCIMG_INFRA_AIRING || 
		iImage == WZCIMG_INFRA_SILENT)
	{
		InfraMode = Ndis802_11Infrastructure;
	}
	else
	{
		InfraMode = Ndis802_11IBSS;
	}

	return InfraMode;
}



/*++

Routine Name:

	IsConfigInLV

Routine Description:

	Checks if the specified config is currently visible in the specified list view and
	returns the index if so.

Arguments:

	pWLanInfo:	Wireless info struct
	pCrt:		Wireless network config

Return Value:

	Index in the list or -1 for not found

--*/

INT IsConfigInLV(PWLAN_INFO pWLanInfo, PWLAN_CONFIG_LISTENT pCrt)
{
	INT 	iListSize;
	INT 	iIndex = -1;
	INT 	j;
	WCHAR	wszConfig[MAX_SSID_LEN_UI+1];
	DWORD 	cchConfig;

	cchConfig = MultiByteToWideChar(CP_ACP, 0, 
						pCrt->wzcConfig.Ssid.Ssid,
						pCrt->wzcConfig.Ssid.SsidLength,
						wszConfig,
						MAX_SSID_LEN_UI);
	wszConfig[cchConfig] = '\0';
	
	iListSize = ListView_GetItemCount(pWLanInfo->hwndVLV);
	for(j=1;j<iListSize;j++)
	{
		NDIS_802_11_NETWORK_INFRASTRUCTURE InfraMode;
		LVITEM  lviCurr = {0};
		DWORD	clviText;
		WCHAR 	buf[MAX_SSID_LEN_UI];

		lviCurr.mask		= LVIF_IMAGE | LVIF_TEXT;
		lviCurr.iItem 		= j;
		lviCurr.pszText 	= buf;
		lviCurr.cchTextMax 	= MAX_SSID_LEN_UI;
		ListView_GetItem(pWLanInfo->hwndVLV, &lviCurr);

		clviText = wcslen(lviCurr.pszText);

		// Get infrastructure mode based on the list image used
		InfraMode = GetInfraMode(lviCurr.iImage);
		if(pCrt->wzcConfig.InfrastructureMode == InfraMode)
		{
		    // Check if first part of string in LV is equal to SSID.
        	if(!wcsncmp(wszConfig, lviCurr.pszText, cchConfig))
			{
    			if(clviText == cchConfig + g_cchPreferred)
    			{
    				// Check if preferred string is appended to end of list view
    				if(!wcscmp(&lviCurr.pszText[clviText - g_cchPreferred], g_wszPreferred))
    				{					
    					iIndex 	= j;
    					break;
    				}
    			}
    			else if(clviText == cchConfig)
    			{
    				iIndex 	= j;
    				break;
    			}
			}
		}
	}

	return iIndex;
}


/*++

Routine Name: 

	AddConfigToLV

Routine Description:

	Adds a config to the list view if it does not already exist in the list view and will update the item
	if a corresponding entry does exist.

Arguments:

	pSysIcon:	Info about data associated with systray icon (or wireless network card)
	pCrt:		List Config
	fPreferred:	Add as a preferred entry

Return Value:

	DWORD to indicate return status

--*/

DWORD AddConfigToLV(SystrayIcon* pSysIcon, PWLAN_CONFIG_LISTENT pCrt, BOOL fPreferred)
{
	DWORD		dwErr 		= ERROR_SUCCESS;
	LVITEM 		lvi 		= {0};
	BOOL		fMatch 		= FALSE;
	BOOL		fNoUpdate 	= FALSE;
	DWORD		cSSID;
	INT			nImgIdx;
	WCHAR 		wszSSID[MAX_SSID_LEN_UI+1];
	CHAR		szListItem[MAX_SSID_LEN_UI];
	BOOL 		fInPrefList;
	DWORD 		cbSSIDLen;


	cSSID = MultiByteToWideChar(CP_ACP, 0,
						pCrt->wzcConfig.Ssid.Ssid,
						pCrt->wzcConfig.Ssid.SsidLength,
						wszSSID,
						MAX_SSID_LEN_UI);
	wszSSID[cSSID] = L'\0';
	
	// get the item's image index
    if (pCrt->wzcConfig.InfrastructureMode == Ndis802_11Infrastructure)
    {
        nImgIdx = (pCrt->dwFlags & WZC_DESCR_ACTIVE) ? WZCIMG_INFRA_ACTIVE :
                    ((pCrt->dwFlags & WZC_DESCR_VISIBLE) ? WZCIMG_INFRA_AIRING : WZCIMG_INFRA_SILENT);
    }
    else
    {
        nImgIdx = (pCrt->dwFlags & WZC_DESCR_ACTIVE) ? WZCIMG_ADHOC_ACTIVE :
                    ((pCrt->dwFlags & WZC_DESCR_VISIBLE) ? WZCIMG_ADHOC_AIRING : WZCIMG_ADHOC_SILENT);
    }
	
	if(wcscmp(wszSSID, L""))
	{
        memset(&lvi,0,sizeof(lvi));
        lvi.iSubItem 	= 0;
        lvi.mask 		= LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
        lvi.iImage 		= nImgIdx;
        lvi.lParam 		= (LPARAM)pCrt;
       	lvi.pszText 	= wszSSID;

        // Determine if the config is in the list view
        lvi.iItem = IsConfigInLV(&pSysIcon->WLanInfo, pCrt);
        if(lvi.iItem == -1)
        {
    		lvi.iItem = 0;
    		fMatch = FALSE;
        }
        else
        {
    		fMatch = TRUE;
        }

    	// Make NULL terminated string for proceeding call
        cbSSIDLen = min(pCrt->wzcConfig.Ssid.SsidLength, (sizeof(szListItem)-1));
    	strncpy(szListItem, pCrt->wzcConfig.Ssid.Ssid, cbSSIDLen);
    	szListItem[cbSSIDLen] = '\0';

    	// Determine what action to take on the list based on if the config has changed lists
    	fInPrefList = IsConfigInList(pSysIcon->WLanInfo.pHdPList, &pCrt->wzcConfig, NULL);
    	if(!fPreferred && !fInPrefList)
    	{
    		// Do nothing
    	}
    	else if(!fPreferred && fInPrefList)
    	{
    		// In this case, don't even update
    		fNoUpdate = TRUE;
    	}
    	else
    	{
    		wcscat(lvi.pszText, g_wszPreferred);
    	}
    	
    	// If the config is not in the list then add it, otherwise modify the current item
    	if(!fMatch)
    	{
    		InsertLVItem(pSysIcon->WLanInfo.hwndVLV, lvi);
    	}
    	else if(!fNoUpdate)
    	{
    		ListView_SetItem(pSysIcon->WLanInfo.hwndVLV, &lvi);
    	}
	}
	
	return dwErr;
}



/*++

Routine Name: 

	RefreshPreferredLV

Routine Description:

	Refresh the entries in the preferred list view

Arguments:

	pWLanInfo:	Info pertaining to wireless networks

--*/

void RefreshPreferredLV(PWLAN_INFO pWLanInfo)
{
	PWLAN_CONFIG_LISTENT	pConfig;
	int 					i, iListSize;

	WZCQCFG_LOCK;

	if(pWLanInfo->hwndPLV)
	{
		iListSize = ListView_GetItemCount(pWLanInfo->hwndPLV);
				
		DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: Refresh Preferred List: %d entries"), iListSize));
		
		for(i = 0; i < iListSize; i++)
		{
			PWLAN_CONFIG_LISTENT 	pOldConfig;
			LVITEM 					lvi={0};
			WCHAR					buf[MAX_SSID_LEN_UI];

			// Get current config from list view
			lvi.mask  		= LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	        lvi.iItem 		= i;
	        lvi.iSubItem 	= 0;
	        lvi.pszText 	= buf;
	        lvi.cchTextMax 	= MAX_SSID_LEN_UI;
			if(!ListView_GetItem(pWLanInfo->hwndPLV, &lvi))
			{
				// Failed to get this item, so let's just move one to next
				ASSERT(0);
				continue;
			}			

			// Get the config currently stored in the list view
			pOldConfig = (PWLAN_CONFIG_LISTENT)lvi.lParam;
			if(pOldConfig == NULL)
			{
				ASSERT(0);
				continue;
			}
			
			// Get the config from the newly refreshed list
			if(!GetListConfigBySsid(pWLanInfo->pHdPList, 
							&pOldConfig->wzcConfig.Ssid, 
							pOldConfig->wzcConfig.InfrastructureMode, 
							&pConfig))
			{
				// Failed to find this config, so let's just move on to next
				ASSERT(0);
				continue;
			}

			ASSERT(pConfig != NULL);

			// Update config in list view
			lvi.lParam = (LPARAM) pConfig;
			ListView_SetItem(pWLanInfo->hwndPLV, &lvi);

			DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: <%s> updated config %X"), lvi.pszText, pConfig));
		}
	}

	WZCQCFG_UNLOCK;
}


/*++

Routine Name:

	RefreshWZCListView

Routine Description:

	Refreshes the list view in the wireless information tab.

Arguments:

	The Systray Icon struct for the list view to refresh

--*/

void RefreshWZCListView(SystrayIcon* pSysIcon)
{
	PWLAN_CONFIG_LISTENT  	pCrt;
	PWLAN_CONFIG_LISTENT  	pLists[2];
	INT						i;
	INT 					iListSize;
	INT						cConfigs;
	INT						cDeletedItems = 0;

	// Delete from list view what is not in config list
	iListSize = ListView_GetItemCount(pSysIcon->WLanInfo.hwndVLV);
	for(i = 1; i < iListSize; i++)
	{
		NDIS_802_11_NETWORK_INFRASTRUCTURE InfraMode;
		PWLAN_CONFIG_LISTENT	pConfig	= NULL;
		LVITEM 					lvi 	= {0};
		BOOL 					fMatch	= FALSE;
		WCHAR					buf[MAX_SSID_LEN_UI];
				
		// Get the list view's text
		lvi.mask 		= LVIF_IMAGE | LVIF_TEXT;
		lvi.iItem		= (i - cDeletedItems);
		lvi.pszText		= buf;
		lvi.cchTextMax	= MAX_SSID_LEN_UI;
		ListView_GetItem(pSysIcon->WLanInfo.hwndVLV, &lvi);

		// Get infrastructure mode based on the list image used
		InfraMode = GetInfraMode(lvi.iImage);

		// Check in preferred list
		if(!fMatch && pSysIcon->WLanInfo.pHdPList)
			fMatch = IsNetworkInList(pSysIcon->WLanInfo.pHdPList, lvi.pszText, InfraMode);
		
		// Check in visible list
		if(!fMatch && pSysIcon->WLanInfo.pHdVList)
			fMatch = IsNetworkInList(pSysIcon->WLanInfo.pHdVList, lvi.pszText, InfraMode);
		
		// If no match was found then delete it from the list
		if(fMatch == FALSE)
		{
			ListView_DeleteItem(pSysIcon->WLanInfo.hwndVLV, (i - cDeletedItems));
			cDeletedItems++;
		}
	}

	cConfigs = iListSize - cDeletedItems;

	// Insert new configs from both lists
	pLists[0] = pSysIcon->WLanInfo.pHdPList;
	pLists[1] = pSysIcon->WLanInfo.pHdVList;
	for(i = 0; i < 2; i++)
	{
		pCrt = pLists[i];
		if(pCrt)
		{
			do
			{		
				if(i == 0)
				{
					AddConfigToLV(pSysIcon, pCrt, TRUE);
				}
				else
				{
					AddConfigToLV(pSysIcon, pCrt, FALSE);
				}				

				pCrt = pCrt->pNext;
			}
			while(pCrt != pLists[i]);
		}
	}
} // RefreshWZCListView



/*++

Routine Name: 

	IsNetworkInList

Routine Description:

	Checks if a network config exists in the specified list.

Arguments:

	pHdList:		List of configs
	szConfigName:	Config name
	InfraMode:		Infrastructure mode of the config

Return Value:

	TRUE:	It exists
	FALSE:	It does not exist

--*/

BOOL IsNetworkInList(WLAN_CONFIG_LISTENT *pHdList, WCHAR* wszNetworkName, NDIS_802_11_NETWORK_INFRASTRUCTURE InfraMode)
{
	PWLAN_CONFIG_LISTENT  	pCrt = pHdList;
	DWORD					cchNetworkName;
	BOOL					fChkPref;
	BOOL					fRetVal = FALSE;

	if(pHdList == NULL)
	{
		return FALSE;
	}

	cchNetworkName = wcslen(wszNetworkName);

	do
	{
		DWORD cchConfigName;
		WCHAR wszConfigName[MAX_SSID_LEN+1];

		cchConfigName = MultiByteToWideChar(CP_ACP, 0,
									pCrt->wzcConfig.Ssid.Ssid,
									pCrt->wzcConfig.Ssid.SsidLength,
									wszConfigName,
									MAX_SSID_LEN);
		wszConfigName[cchConfigName] = '\0';

		fChkPref = FALSE;
		
		// If configs match exactly or match with appended preferred string not considered
		if(InfraMode != pCrt->wzcConfig.InfrastructureMode)
		{
			// Not a match, go to next element
		}
		else if(cchNetworkName == cchConfigName &&
			!wcsncmp(wszConfigName, wszNetworkName, cchConfigName))
		{
			// Config strings match exactly
			fRetVal = TRUE;
			goto exit;
		}
		else if(cchNetworkName == cchConfigName + g_cchPreferred &&
			!wcsncmp(wszConfigName, wszNetworkName, cchConfigName))
		{
			// Preferred Config found, let's make sure preferred string is appended
			fChkPref = TRUE;
		}

		if(fChkPref)
		{
			// Check for preferred string on the end
			if(!wcsncmp(&wszNetworkName[cchNetworkName - g_cchPreferred], g_wszPreferred, g_cchPreferred))
			{
				return TRUE;
			}
		}

		pCrt = pCrt->pNext;
	}
	while(pCrt != pHdList);

exit:
	return fRetVal;
}



/*++

Routine Name: 

	InsertLVItem

Routine Description:

	Insert an item into the list view

Arguments:

	hwndDlg:	HWND of the given window
	lvi:		Item to insert

Return Value:

	DWORD to indicate return status

--*/

DWORD InsertLVItem(HWND hwndLV, LVITEM lvi)
{
	DWORD 	dwErr = ERROR_SUCCESS;
	TCHAR	szItem[MAX_SSID_LEN_UI];
	int		cItems;
	int		iIndex;
	int		i, iLow, iMid, iHigh;
	
	/* Determine index to insert item at */

	// Set binary search variables
	cItems = ListView_GetItemCount(hwndLV);
	iLow = 1;
	iHigh = cItems - 1;

	// Binary search for items to insert next to
	while(iHigh > iLow)
	{
		iMid = (iHigh + iLow) / 2;

		ListView_GetItemText(hwndLV, iMid, 0, szItem, MAX_SSID_LEN_UI);
		if(CSTR_LESS_THAN == CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, lvi.pszText, -1, szItem, -1))
		{
			iHigh = iMid - 1;
		}
		else
		{
			iLow = iMid + 1;
		}
	}

	// Let's figure out if item goes before or after iMid
	iMid = (iHigh + iLow) / 2;
	ListView_GetItemText(hwndLV, iMid, 0, szItem, MAX_SSID_LEN_UI);

    if(CSTR_GREATER_THAN == CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, lvi.pszText, -1, szItem, -1)) // Insert at end
	{
		iIndex = iMid + 1;	
	}
	else
	{
		iIndex = iMid;
	}

	// If index is 0, then set to 1 since "Add New..." must be first entry
	if(iIndex == 0)
	{
		iIndex++;
	}

	// Offset all of the following indexes
	for(i = 0; i < (cItems - iIndex + 1); i++)
	{
		LVITEM lviTmp = {0};

		// Start from last item
		lviTmp.iItem = cItems - 1 - i;
		ListView_GetItem(hwndLV, &lviTmp);
		lviTmp.iItem++;
		ListView_SetItem(hwndLV, &lviTmp);
	}
	
	// Insert the item
	lvi.iItem = iIndex;
	ListView_InsertItem(hwndLV, &lvi);

	return dwErr;
}



/*++

Routine Name: 

	RefreshWirelessInfo

Routine Description:

	This function refreshes the wireless struct from zero config

Arguments:

	hwndDlg:	HWND of the given window

Return Value:

	HRESULT to indicate return status

--*/

HRESULT RefreshWirelessInfo(SystrayIcon* pSysIcon)
{
	HRESULT					hr 			= S_OK;
	DWORD					dwError		= ERROR_SUCCESS;

	WZCQCFG_LOCK;

	DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: --- Refresh Wireless Info ---")));

	// If the preferred list is currently being viewed, then we do not want to sync
	// with zero config until the advanced dialog is closed
	if(pSysIcon->WLanInfo.hwndPLV != NULL)
	{
		DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: Advanced dialog is open, wireless info refresh is delayed")));
		goto exit;
	}

	// If we are currently configuring to a netowrk, it is possible that the message pump for
	// a child window processes a refresh message.  So let's ignore these messages.
	if(g_fConfiguring)
	{
		goto exit;
	}

	// If the zero config data has already been deleted then do not bother to 
	// delete it.  This should not really ever get called but let's check just in
	// case.  If it ever does get hit, the refresh thread will exit quietly and
	// things should resume ok the next time around.
	if(pSysIcon->WLanInfo.IntfEntryEx.bInitialized == FALSE)
	{
		hr = E_FAIL;
		goto exit;
	}
	
	pfnWZCDeleteIntfObjEx(&pSysIcon->WLanInfo.IntfEntryEx);

	pSysIcon->WLanInfo.IntfEntryEx.wszGuid = pSysIcon->ptcDeviceName;

	// GetWireless Info
	if ((dwError = pfnWZCQueryInterfaceEx(
                    NULL,
                    INTF_ALL,
                    &pSysIcon->WLanInfo.IntfEntryEx,
                    &pSysIcon->WLanInfo.dwOIDFlags)) != ERROR_SUCCESS) 
	{
	        DEBUGMSG(ZONE_ERROR,(TEXT("NETUIQC: WZCQueryInterfaceEx failed 0x%X\r\n"),dwError));
	        hr = E_FAIL;
	        goto exit;
	}	

	FillVisibleList(&pSysIcon->WLanInfo,(PWZC_802_11_CONFIG_LIST)pSysIcon->WLanInfo.IntfEntryEx.rdBSSIDList.pData);
	FillPreferredList(&pSysIcon->WLanInfo,(PWZC_802_11_CONFIG_LIST)pSysIcon->WLanInfo.IntfEntryEx.rdStSSIDList.pData);
	FillCurrentConfig(&pSysIcon->WLanInfo,&pSysIcon->WLanInfo.IntfEntryEx);

	// Refresh List Views since configs may have changed
	if(pSysIcon->fDialogVisible && IsWindow(pSysIcon->hWZCPage))
	{
		RefreshWZCListView(pSysIcon);
		AddListConfigsToCache(pSysIcon->hWZCPage);
	}
	
exit:

	WZCQCFG_UNLOCK;
	
	return hr;
	
} // RefreshWirelessInfo



/*++

Routine Name:

	RefreshWZCButtons

Routine Description:

	This function refreshes the Wireless Info page's buttons based on whether an item is selected
	in the list view.

Arguments:

	hwndDlg:	HWND of the property sheet

Return Value:

	HRESULT to indicate return status

--*/

HRESULT	RefreshWZCButtons(HWND hwndDlg)
{
	SystrayIcon*	pSysIcon 	= NULL;
	HRESULT			hr 			= S_OK;
	INT				iSelected 	= -1;
	HWND			hwndLV;
	HWND			hwndBtn;

	WZCQCFG_LOCK;

	// Get the systray icon for the given window
	hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
	if(FAILED(hr) || !pSysIcon)
	{
		goto exit;
	}

	// If Zero Config is disabled, then quietly exit
	if(! (pSysIcon->WLanInfo.dwCtlFlags & INTFCTL_ENABLED))
	{
		goto exit;
	}

	// Get dialog ids
	hwndLV 	= GetDlgItem(hwndDlg, IDC_WZCQCFG_NETWORKS);
	hwndBtn = GetDlgItem(hwndDlg, IDC_WZCQCFG_CONNECT);
	if(!hwndLV || !hwndBtn)
	{
		hr = E_FAIL;
		goto exit;
	}

	// If we are currently disabled, do not refresh
	if(!IsWindowEnabled(hwndLV) && !IsWindowEnabled(hwndBtn))
	{
		goto exit;
	}
	
	// Get the selected item from the visible list
	iSelected = ListView_GetNextItem(hwndLV, -1, LVNI_SELECTED);
	if (iSelected < 1) // Use 1 since item 0 is the "Add New" string
	{						
		EnableWindow(hwndBtn, FALSE);
	}
	else if(pSysIcon->WLanInfo.dwCtlFlags & INTFCTL_ENABLED)
	{
		WLAN_CONFIG_LISTENT* pConfig = NULL;
		if(GetSelectedConfig(pSysIcon->WLanInfo.hwndVLV, &pConfig, NULL))
		{
			// Check if it is valid to connect to this type of network
			if ((pSysIcon->WLanInfo.IntfEntryEx.dwCtlFlags & INTFCTL_CM_MASK) != Ndis802_11AutoUnknown &&
		        (pSysIcon->WLanInfo.IntfEntryEx.dwCtlFlags & INTFCTL_CM_MASK) != (DWORD)pConfig->wzcConfig.InfrastructureMode)
			{
				EnableWindow(hwndBtn, FALSE);
			}
			else 
			{
				EnableWindow(hwndBtn, TRUE);
			}
		}
	}

exit:
	WZCQCFG_UNLOCK;
	
	return hr;

} // RefreshWZCButtons



/*++

Routine Name:

	OnWZCOk

Routine Description:

	Ok button was pressed from property sheet

Arguments:

	hwndDlg:	HWND of the dialog

--*/

void OnWZCOk(HWND hwndDlg)
{
	SystrayIcon* 	pSysIcon = NULL;
	HRESULT			hr;

	WZCQCFG_LOCK;
	
	// Get the systray icon for the given window
	hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
	if(FAILED(hr) || !pSysIcon)
	{
		goto exit;
	}

	g_fPopup = (BST_CHECKED == SendMessage(pSysIcon->WLanInfo.hwndPopupChk, BM_GETCHECK, 0, 0));
	SetDialogPopup(g_fPopup, 0);

exit:
	WZCQCFG_UNLOCK;
	
	return;
}



/*++

Routine Name:

	AddListConfigsToCache

Routine Description:

	Add the configs from the list view to the cache

Arguments:

	hwndDlg:	HWND of the dialog

Return Value:

	DWORD to indicate return status
	
--*/

DWORD AddListConfigsToCache(HWND hwndDlg)
{
	SystrayIcon* 	pSysIcon 	= NULL;
	DWORD			dwRetVal 	= ERROR_SUCCESS;
	HRESULT			hr;

	WZCQCFG_LOCK;

	// If popup is turned off, don't do anything here
	if(!g_fPopup)
	{
		goto exit;
	}
	
	// Get the systray icon for the given window
	hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
	if(FAILED(hr) || !pSysIcon)
	{
		dwRetVal = E_FAIL;
		goto exit;
	}

	// If our current state is failed, then add the configs, otherwise ignore
	if(pSysIcon->dwWZCState == WZC_FAILED_ASSOCIATION || 
		pSysIcon->dwWZCState == WZC_CANCEL_AUTH || 
		pSysIcon->dwWZCState == WZC_FAILED_AUTH_NO_RETRY ||
		pSysIcon->dwWZCState == WZC_FAILED_AUTH_WILL_RETRY || 
		pSysIcon->dwWZCState == WZC_PREFERED_LIST_EXHAUSTED)
	{
		INT		iListSize;
		INT		i;

		// Loop through the list and add each item to the cache
		iListSize = ListView_GetItemCount(pSysIcon->WLanInfo.hwndVLV);
		for(i = 1; i < iListSize; i++)
		{
			PWLAN_CONFIG_LISTENT 	pConfig;
			LVITEM 					lvi = {0};
			CHAR					szListItem[MAX_SSID_LEN+1];

			lvi.mask 	= LVIF_PARAM;
			lvi.iItem	= i;
			ListView_GetItem(pSysIcon->WLanInfo.hwndVLV, &lvi);

			pConfig = (PWLAN_CONFIG_LISTENT)lvi.lParam;
			ASSERT(pConfig);
			ASSERT(pConfig->wzcConfig.Ssid.SsidLength <= ARRAY_SIZE(szListItem));
            
			strncpy(szListItem, pConfig->wzcConfig.Ssid.Ssid, pConfig->wzcConfig.Ssid.SsidLength);
			szListItem[pConfig->wzcConfig.Ssid.SsidLength] = '\0';
			
			dwRetVal = WZCCacheAdd(szListItem, (DWORD)pConfig->wzcConfig.InfrastructureMode);
			if(dwRetVal != ERROR_SUCCESS)
			{
				goto exit;
			}
		}
	}

exit:
	WZCQCFG_UNLOCK;
	
	return dwRetVal;
}
	


/*++

Routine Name:

	AreNetworksCached

Routine Description:

	Check if all the visible networks are currently cached

Arguments:

	pWLanInfo:	Wireless info struct

Return Value:

	TRUE: 	All networks are currently cached
	FALSE:	At least one network is not cached
	
--*/

BOOL AreNetworksCached(PWLAN_INFO pWLanInfo)
{
	BOOL 					fRetVal = TRUE;
	PWLAN_CONFIG_LISTENT	pCurr;

	WZCQCFG_LOCK;

	if(pWLanInfo->pHdVList)
	{
		pCurr = pWLanInfo->pHdVList;

		do
		{
			CHAR szNetwork[MAX_SSID_LEN+1];
			DWORD cbSSIDLen;

			if (!pCurr) {
			    ASSERT(FALSE);
			    fRetVal = FALSE;
			    break;
			}

			ASSERT(pCurr->wzcConfig.Ssid.SsidLength <= ARRAY_SIZE(szNetwork));
			cbSSIDLen = min(pCurr->wzcConfig.Ssid.SsidLength, MAX_SSID_LEN);
			strncpy(szNetwork, pCurr->wzcConfig.Ssid.Ssid, cbSSIDLen);
			szNetwork[cbSSIDLen] = '\0';
			
			if(!IsInWZCCache(szNetwork, (DWORD)pCurr->wzcConfig.InfrastructureMode))
			{
				fRetVal = FALSE;
				break;
			}
			
			pCurr = pCurr->pNext;
		}
		while(pCurr != pWLanInfo->pHdVList);
	}

	WZCQCFG_UNLOCK;

	return fRetVal;
}



/*++

Routine Name: 

	OnAdvanced

Routine Description:

	Handles advanced dialog being displayed.

Arguments:

	hwndDlg:	Handle to parent window

--*/

void OnAdvanced(HWND hwndDlg)
{
	SystrayIcon* 	pSysIcon = NULL;
	HRESULT 		hr;
	HWND			hDlg;
	HWND			hPropSheet;

	WZCQCFG_LOCK;

	hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
	if(FAILED(hr) || !pSysIcon)
	{
		goto exit;
	}
	
	// Check if we need to refresh before opening dialog
	if((pSysIcon->WLanInfo.IntfEntryEx.dwCtlFlags & INTFCTL_ENABLED) && 
		(pSysIcon->WLanInfo.dwOIDFlags & INTF_BSSIDLIST) == 0)
	{
		SetCursor(LoadCursor(NULL, IDC_WAIT));			
		RefreshWirelessInfo(pSysIcon);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}

	hPropSheet = pSysIcon->hPropSheet;

	hDlg = CreateDialogParam(v_hInst,
					MAKEINTRESOURCE(IDD_LAN_WZC_ADVANCED), 
					hwndDlg, 
					NetworkWLanAdvancedDlgProc,
					(LPARAM)&pSysIcon->WLanInfo);
	if(hDlg)
    {
    	EnableWindow(hPropSheet, FALSE);

   		WZCQCFG_UNLOCK;
    	WZCMiniMsgPump(hDlg);
   		WZCQCFG_LOCK;

   		// Since critical section was unlocked, we must check the window is still valid.
    	if(IsWindow(hPropSheet))
	    	EnableWindow(hPropSheet, TRUE);

		pSysIcon = NULL;
    	hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
		if(FAILED(hr) || !pSysIcon)
		{
			goto exit;
		}

		pSysIcon->WLanInfo.hwndPLV = NULL;
		RefreshWirelessInfo(pSysIcon);

		if(pSysIcon->WLanInfo.dwCtlFlags & INTFCTL_ENABLED)
		{
			EnableWindow(pSysIcon->WLanInfo.hbtnConnect, TRUE);
			EnableWindow(pSysIcon->WLanInfo.hbtnViewLog, TRUE);
		}
		else
		{
			EnableWindow(pSysIcon->WLanInfo.hbtnConnect, FALSE);
			EnableWindow(pSysIcon->WLanInfo.hbtnViewLog, FALSE);
		}
    }
	
exit:
	WZCQCFG_UNLOCK;
}



/*++

Routine Name: 

	OnAddNew

Routine Description:

	Adds a new dialog to the list and presents user with config dialog to do so.

Arguments:

	hPropSheet:	The structure containing info regarding to specific systray icon

--*/

void OnAddNew(HWND hPropSheet)
{
	HRESULT					hr;
	SystrayIcon*			pSysIcon = NULL;
	WLAN_CONFIG_LISTENT*	pConfig = NULL;
	WLAN_CFG_PROPS 			CfgProps;
	HWND					hDlg;
	BOOL					fAddConfig;
    BOOL fPortrait = (GetSystemMetrics(SM_CXSCREEN) < 480)? TRUE:FALSE;
    UINT nDlgId    = fPortrait? IDC_WZC_DLG_PROPS_N:IDC_WZC_DLG_PROPS_W;

	WZCQCFG_LOCK;

	hr = GetDeviceSystrayIcon(hPropSheet, &pSysIcon);
	if(FAILED(hr))
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Failed to get systray icon struct")));
		goto exit;
	}

	// If Zero Config is disabled, then quietly exit
	if(! (pSysIcon->WLanInfo.dwCtlFlags & INTFCTL_ENABLED))
	{
		goto exit;
	}

	InitializeCfgProps(&CfgProps, WZCDLG_PROPS_RWALL|WZCDLG_PROPS_NODUPS|WZCDLG_PROPS_DEFOK|WZCDLG_PROPS_ADD,nDlgId);

	if((pSysIcon->WLanInfo.IntfEntryEx.dwCtlFlags & INTFCTL_CM_MASK) != Ndis802_11AutoUnknown)
	{
		CfgProps.wzcConfig.InfrastructureMode = (NDIS_802_11_NETWORK_INFRASTRUCTURE)(pSysIcon->WLanInfo.IntfEntryEx.dwCtlFlags & INTFCTL_CM_MASK);
		CfgProps.dwFlags &= ~WZCDLG_PROPS_RWINFR;
	}

	CfgProps.dwNicCapabilities = pSysIcon->WLanInfo.IntfEntryEx.dwCapabilities;
    CfgProps.bWPAAESCapable    = IsWPAAESCapable((PINTF_80211_CAPABILITY)pSysIcon->WLanInfo.IntfEntryEx.rdNicCapabilities.pData);    
	CfgProps.wzcConfig.Privacy = pSysIcon->WLanInfo.IntfEntryEx.dwCapabilities & INTFCAP_MAX_CIPHER_MASK;
	CfgProps.pConfigList = pSysIcon->WLanInfo.pHdPList;
	CfgProps.fDlgOk = FALSE;

	hPropSheet = pSysIcon->hPropSheet;

	hDlg = CreateDialogParam(v_hInst, 
				MAKEINTRESOURCE(nDlgId), 
				pSysIcon->hWZCPage, 
				NetworkWLanPropsDlgProc,
				(LPARAM)&CfgProps);
	if(hDlg)
    {
    	EnableWindow(hPropSheet, FALSE);

   		WZCQCFG_UNLOCK;
    	WZCMiniMsgPump(hDlg);
   		WZCQCFG_LOCK;

   		// Since critical section was unlocked, we must check the window is still valid.
    	if(IsWindow(hPropSheet))
	    	EnableWindow(hPropSheet, TRUE);

		// We unlocked so need to reobtain systray icon context
		pSysIcon = NULL;
    	hr = GetDeviceSystrayIcon(hPropSheet, &pSysIcon);
		if(FAILED(hr))
		{
			DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Failed to get systray icon struct")));
			goto exit;
		}
    }

	// Ok button was pressed
	if(CfgProps.fDlgOk)
	{
		DWORD 	dwFlags = WZC_DESCR_PREFRD;

		// Check if the config exists in the visible list
		if(IsConfigInList(pSysIcon->WLanInfo.pHdVList, &CfgProps.wzcConfig, NULL))
		{
			dwFlags |= WZC_DESCR_VISIBLE;
		}

		// Add the config to the preferred list
		pConfig = NULL; // Reset pointer
		fAddConfig = (AddUniqueConfig(&pSysIcon->WLanInfo,
						WZCADD_OVERWRITE | WZCADD_HIGROUP,
						dwFlags,
						&CfgProps.wzcConfig,
						&pConfig) == ERROR_SUCCESS);

		SavePreferredConfigs(&pSysIcon->WLanInfo, &pSysIcon->WLanInfo.IntfEntryEx);
		SetWirelessInfo(&pSysIcon->WLanInfo);
	}

	// Config was added successfully
	if(CfgProps.fDlgOk && fAddConfig)
	{
		// Add a preferred config
		AddConfigToLV(pSysIcon, pConfig, TRUE);
	}

exit:
	WZCQCFG_UNLOCK;
	return;
}



/*++

Routine Name: 

	OnListDblClick

Routine Description:

	Handles double-click on list view event

Arguments:

	hwndDlg:	Handle to parent window

--*/

void OnListDblClick(HWND hwndDlg)
{
	SystrayIcon* 	pSysIcon 	= NULL;
	HRESULT 		hr;
	int 			iSelected;

	WZCQCFG_LOCK;
	
	hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
	if(FAILED(hr))
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Failed to get systray icon struct")));
		goto exit;
	}
	
	iSelected = ListView_GetNextItem(pSysIcon->WLanInfo.hwndVLV, -1, LVNI_SELECTED);
	if(iSelected == 0)
	{
		WZCQCFG_UNLOCK;
		OnAddNew(GetParent(hwndDlg));
		WZCQCFG_LOCK;
	}
	else if(iSelected > 0)
	{
		PWLAN_CONFIG_LISTENT	pConfig		= NULL;
		LVITEM					lvi = {0};
		
		lvi.mask	= LVIF_PARAM;
		lvi.iItem	= iSelected;
		ListView_GetItem(pSysIcon->WLanInfo.hwndVLV, &lvi);

		pConfig = (PWLAN_CONFIG_LISTENT)lvi.lParam;
		ASSERT(pConfig != NULL);
		
		// If config is in preferred list then show props, otherwise try to connect
		if(IsConfigInList(pSysIcon->WLanInfo.pHdPList, &pConfig->wzcConfig, NULL))
		{
			WZCQCFG_UNLOCK;
			OnConfigureNetwork(hwndDlg);
			WZCQCFG_LOCK;
		}
		else
		{
			WZCQCFG_UNLOCK;
			OnConnect(hwndDlg);
			WZCQCFG_LOCK;
		}
	}
	
exit:
	WZCQCFG_UNLOCK;
	return;
}



/*++

Routine Name: 

	OnListRClick

Routine Description:

	Handles right click on list view event

Arguments:

	hwndDlg:	Handle to parent window

--*/

void OnListRClick(HWND hwndDlg)
{
	SystrayIcon* 	pSysIcon = NULL;
	HRESULT			hr;

	hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
	if(SUCCEEDED(hr) && pSysIcon)
	{	
		PWLAN_CONFIG_LISTENT 	pConfig;
		INT						iSelected = 0;
		
		WZCQCFG_LOCK;

		GetSelectedConfig(pSysIcon->WLanInfo.hwndVLV, &pConfig, &iSelected);
		if(iSelected > 0)
		{
			LVITEM 					lvi = {0};
			HMENU					hMenu;

			ASSERT(pConfig);
		
			hMenu = CreatePopupMenu();
			if(hMenu)
			{
				POINT 	p;

				// If Zero Config is disabled, do not enable items
				if(pSysIcon->WLanInfo.dwCtlFlags & INTFCTL_ENABLED)
				{
					AppendMenu(hMenu, 0, IDM_WZC_CONNECT, g_wszConnect);

					// If config is preferred, then enable Configure and Delete
					if(IsConfigInList(pSysIcon->WLanInfo.pHdPList, &pConfig->wzcConfig, NULL))
					{
						AppendMenu(hMenu, 0, IDM_WZC_CONFIGURE, g_wszConfigure);
						AppendMenu(hMenu, 0, IDM_WZC_DELETE, g_wszDelete);
					}
					else
					{
						AppendMenu(hMenu, MF_GRAYED, IDM_WZC_CONFIGURE, g_wszConfigure);
						AppendMenu(hMenu, MF_GRAYED, IDM_WZC_DELETE, g_wszDelete);
					}
				}
				else
				{
					AppendMenu(hMenu, MF_GRAYED, IDM_WZC_CONNECT, g_wszConnect);
					AppendMenu(hMenu, MF_GRAYED, IDM_WZC_CONFIGURE, g_wszConfigure);
					AppendMenu(hMenu, MF_GRAYED, IDM_WZC_DELETE, g_wszDelete);
				}

				GetCursorPos(&p);

				WZCQCFG_UNLOCK;
				TrackPopupMenu(hMenu, TPM_LEFTALIGN, p.x, p.y, 0, hwndDlg, NULL);									
				WZCQCFG_LOCK;

				DestroyMenu(hMenu);
			}
		}
		
		WZCQCFG_UNLOCK;
	}
}



/*++

Routine Name: 

	OnInitWZCDialog

Routine Description:

	This function is called when the zero config property page is initialized.

Arguments:

	hwndDlg:	HWND of the property page

--*/

void OnInitWZCDialog(HWND hwndDlg)
{
	HRESULT			hr = S_OK;
	SystrayIcon		*pSysIcon = NULL;
	LV_COLUMN		lvc = {0};
	DWORD			dwStyle;
	RECT			rc;
	LVITEM			lvi;
	int				iNewItemID = -1;
	WZCQCFG_LOCK;

	// Reset state flags
	g_fConnecting	= FALSE;
	g_fConfiguring	= FALSE;

	// Load string resources
	LoadString(v_hInst, IDS_QC_CONNECT, g_wszConnect, MAX_CONTEXT_STR);
	LoadString(v_hInst, IDS_QC_CONFIGURE, g_wszConfigure, MAX_CONTEXT_STR);
	LoadString(v_hInst, IDS_QC_DELETE, g_wszDelete, MAX_CONTEXT_STR);
	LoadString(v_hInst, IDS_QC_ADDNEW, g_wszAddNew, MAX_ADDNEW_STR);
	g_cchPreferred = LoadString(v_hInst, IDS_QC_PREFERRED, g_wszPreferred, MAX_PREFERRED_STR);	

	// Get Systray Icon for the given window
	hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
	if(FAILED(hr))
	{
		goto exit;
	}

	// Set the prop page hwnd in the systray icon struct
	pSysIcon->hWZCPage = hwndDlg;

	ResetEvent(pSysIcon->hCloseWZCDialogEvent);

	// Set WLanInfo controls
	pSysIcon->WLanInfo.hwndVLV 		= GetDlgItem(hwndDlg, IDC_WZCQCFG_NETWORKS);
	pSysIcon->WLanInfo.hbtnAdvanced = GetDlgItem(hwndDlg, IDC_WZCQCFG_ADVANCED);
	pSysIcon->WLanInfo.hbtnConnect 	= GetDlgItem(hwndDlg, IDC_WZCQCFG_CONNECT);
	pSysIcon->WLanInfo.hwndPopupChk	= GetDlgItem(hwndDlg, IDC_WZC_SHOWCFG);
	pSysIcon->WLanInfo.hbtnViewLog	= GetDlgItem(hwndDlg, IDC_WZCQCFG_LOGGING);
	pSysIcon->WLanInfo.hwndPLV 		= NULL;

	// initialize the image list styles
	dwStyle = GetWindowLong(pSysIcon->WLanInfo.hwndVLV, GWL_STYLE);
	SetWindowLong(pSysIcon->WLanInfo.hwndVLV, GWL_STYLE, (dwStyle | LVS_SHAREIMAGELISTS));

	// Create state image lists
	pSysIcon->WLanInfo.hImgs = ImageList_LoadBitmap(v_hInst, MAKEINTRESOURCE(IDB_WZCSTATE), 16, 0, CLR_NONE);
	ListView_SetImageList(pSysIcon->WLanInfo.hwndVLV, pSysIcon->WLanInfo.hImgs, LVSIL_SMALL);
	lvc.mask = LVCF_FMT | LVCF_WIDTH;
	lvc.fmt = LVCFMT_LEFT;
	GetClientRect(pSysIcon->WLanInfo.hwndVLV, &rc);
	lvc.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL);
	ListView_InsertColumn(pSysIcon->WLanInfo.hwndVLV, 0, &lvc);
	ListView_SetExtendedListViewStyleEx(pSysIcon->WLanInfo.hwndVLV, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

	// Set list view item for "Add New" entry
	memset(&lvi,0,sizeof(lvi));
	lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    lvi.pszText = g_wszAddNew;
    lvi.iImage = 8;
    lvi.lParam = 0;

	iNewItemID = ListView_InsertItem(pSysIcon->WLanInfo.hwndVLV, &lvi);
	
	if (-1 != iNewItemID)
		ListView_SetItemState(pSysIcon->WLanInfo.hwndVLV, iNewItemID, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
  

	// Query Interface now if it was not done when card was inserted
	if(! pSysIcon->WLanInfo.IntfEntryEx.bInitialized)
	{
		if(!GetWirelessInfo(pSysIcon->ptcDeviceName, &pSysIcon->WLanInfo))
		{
			DEBUGMSG(ZONE_ERROR, (_T("NETUIQC: Error querying Zero Config")));
			goto exit;
		}
	}

	pSysIcon->WLanInfo.dwCtlFlags = pSysIcon->WLanInfo.IntfEntryEx.dwCtlFlags;

	if(pSysIcon->WLanInfo.dwOIDFlags & INTF_BSSIDLIST)
	{
		// add the list of visible configs for this adapter
		FillVisibleList(&pSysIcon->WLanInfo,(PWZC_802_11_CONFIG_LIST)pSysIcon->WLanInfo.IntfEntryEx.rdBSSIDList.pData);
		// add the list of preferred configs for this adapter
		FillPreferredList(&pSysIcon->WLanInfo,(PWZC_802_11_CONFIG_LIST)pSysIcon->WLanInfo.IntfEntryEx.rdStSSIDList.pData);
		// fill in the list view
		FillCurrentConfig(&pSysIcon->WLanInfo,&pSysIcon->WLanInfo.IntfEntryEx);
		RefreshWZCListView(pSysIcon);

		EnableWindow(pSysIcon->WLanInfo.hwndVLV, TRUE);

		SetTimer(hwndDlg, 1, WZCQC_RFSH_TIMEOUT, NULL);
	}
	else
	{
		ASSERT(pSysIcon->hThreads[THREAD_REFRESH_WZC] == NULL);
		
		// If zero config info was not ready, then spin off this thread to handle it
		pSysIcon->hThreads[THREAD_REFRESH_WZC] = CreateThread(NULL, 0, RefreshZeroConfigThread, (LPVOID)hwndDlg, 0, NULL);
		if(pSysIcon->hThreads[THREAD_REFRESH_WZC] == NULL)
		{
			DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Error creating RefreshZeroConfigThread thread")));
			goto exit;
		}
	}

	// Check/Uncheck the popup button
	SendMessage(pSysIcon->WLanInfo.hwndPopupChk, BM_SETCHECK, g_fPopup ? BST_CHECKED : BST_UNCHECKED, 0);

	RefreshWZCButtons(hwndDlg);

	// If Zero Config is not enabled, disable some controls
	if(! (pSysIcon->WLanInfo.dwCtlFlags & INTFCTL_ENABLED))
	{
		EnableWindow(pSysIcon->WLanInfo.hbtnConnect, FALSE);
		EnableWindow(pSysIcon->WLanInfo.hbtnViewLog, FALSE);
	}

	SelectAssociatedNetwork(pSysIcon->WLanInfo.hwndVLV);
	
	// If the last status is not the empty string
	if(_tcscmp(pSysIcon->ptcLastWZCStatus, TEXT("")) != 0)
	{
		DisplayWZCStatus(hwndDlg, pSysIcon->ptcLastWZCStatus);
	}

	DisplayInitialSignalStrength(GetParent(hwndDlg));

	// Create thread for updating signal strength
	pSysIcon->hThreads[THREAD_SIGNAL_STRENGTH] = CreateThread(NULL, 0, SignalStrengthThread, (LPVOID)GetParent(hwndDlg), 0, NULL);
	if(pSysIcon->hThreads[THREAD_SIGNAL_STRENGTH] == NULL)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Error creating SignalStrengthThread thread")));
		goto exit;
	}

exit:
	WZCQCFG_UNLOCK;
	
} // OnInitWZCDialog



/*++

Routine Name: 

	OnCloseWZCDialog

Routine Description:

	This function is called when the zero config property page is closed.

Arguments:

	hwndDlg:	HWND of the property page

--*/

void OnCloseWZCDialog(HWND hwndDlg)
{
	HRESULT hr = S_OK;
	SystrayIcon* pSysIcon = NULL;
	HANDLE hSignalStrength;
	HANDLE hRefreshWZC;
	
	WZCQCFG_LOCK;

	// Get Systray Icon for the given window
	hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
	if(FAILED(hr))
	{
		WZCQCFG_UNLOCK;
		goto exit;
	}

	if(pSysIcon->WLanInfo.hImgs)
	{
		ImageList_Destroy(pSysIcon->WLanInfo.hImgs);
		pSysIcon->WLanInfo.hImgs = NULL;
	}

	SetEvent(pSysIcon->hCloseWZCDialogEvent);

	hSignalStrength = pSysIcon->hThreads[THREAD_SIGNAL_STRENGTH];
	hRefreshWZC = pSysIcon->hThreads[THREAD_REFRESH_WZC];

	pSysIcon->hThreads[THREAD_SIGNAL_STRENGTH] = 0;
	pSysIcon->hThreads[THREAD_REFRESH_WZC] = 0;

	WZCQCFG_UNLOCK;

	//
	// This will wait until threads exit (this must be fast!)
	//	
	if(hSignalStrength)
	{
		WaitForSingleObject(hSignalStrength, INFINITE);
		CloseHandle(hSignalStrength);
	}		
	if(hRefreshWZC)
	{
		WaitForSingleObject(hRefreshWZC, INFINITE);
		CloseHandle(hRefreshWZC);		
	}

exit:
	return;
}


/*++

Routine Name: 

	SelectAssociatedNetwork

Routine Description:

	Select the list view item corresponding to the network which is currently associated
	(if that network exists).

Arguments:

	hwndLV:		Handle to the list view

Return Value:

	TRUE:	Network exists and was selected
	FALSE:	No associated networks exist

--*/

BOOL SelectAssociatedNetwork(HWND hwndLV)
{
	BOOL 	fRetVal = FALSE;
	LVITEM 	lvi 	= {0};
	INT 	i;
	INT 	iItemCount;

	iItemCount = ListView_GetItemCount(hwndLV);
	for(i=1;i<iItemCount;i++)
	{
		PWLAN_CONFIG_LISTENT pConfig;
		
		lvi.mask  = LVIF_PARAM;
		lvi.iItem = i;
		ListView_GetItem(hwndLV, &lvi);
		
		pConfig = (PWLAN_CONFIG_LISTENT) lvi.lParam;
		if(pConfig && (pConfig->dwFlags & WZC_DESCR_ACTIVE))
		{
			ListView_EnsureVisible(hwndLV, i, FALSE);
			fRetVal = TRUE;
			break;
		}
	}

	return fRetVal;
}



/*++

Routine Name: 

	GetSelectedConfig

Routine Description:

	Gets the config of the network currently selected in the List View.

Arguments:

	hwndListView:	List View handle
	ppConfig:		Config to return
	piSelected:		Returns selection index in list view

Return Value:

	BOOL to indicate return status

--*/

BOOL GetSelectedConfig(HWND hwndListView, PWLAN_CONFIG_LISTENT* ppConfig, INT* piSelected)
{	
	BOOL	fRetVal = TRUE;
	int		iSelected;
	LVITEM	lvi 	= {0};

	ASSERT(hwndListView && ppConfig);

	// Get the selected network
	iSelected = ListView_GetNextItem(hwndListView, -1, LVNI_SELECTED);
	if(iSelected == -1)
	{
		fRetVal = FALSE;
		goto exit;
	}

	lvi.mask 	= LVIF_PARAM;
	lvi.iItem 	= iSelected;
	ListView_GetItem(hwndListView, &lvi);

	*ppConfig = (PWLAN_CONFIG_LISTENT)lvi.lParam;

exit:

	if(piSelected)
	{
		*piSelected = iSelected;
	}
	
	return fRetVal;
}



/*++

Routine Name: 

	OnDeleteNetwork

Routine Description:

	Handles deleting a network from the list view

Arguments:

	hwndDlg:	Handle to the parent window

--*/

void OnDeleteNetwork(HWND hwndDlg)
{
	HRESULT					hr;
	SystrayIcon* 			pSysIcon = NULL;
	LVITEM					lvi = {0};
	PWLAN_CONFIG_LISTENT	pConfig;
		
	WZCQCFG_LOCK;

	// Get the systray icon for the given window
	hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
	if(FAILED(hr) || !pSysIcon)
	{
		goto exit;
	}

	// If Zero Config is disabled, then quietly exit
	if(! (pSysIcon->WLanInfo.dwCtlFlags & INTFCTL_ENABLED))
	{
		goto exit;
	}

	// Get list selection
	if(!GetSelectedConfig(pSysIcon->WLanInfo.hwndVLV, &pConfig, NULL))
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: No config was selected in list")));
		goto exit;
	}

	ASSERT(pConfig && pConfig->pNext != NULL && pConfig->pPrev != NULL);

	if(pConfig == pConfig->pNext && pConfig == pConfig->pPrev)
	{
		// One item left
		pSysIcon->WLanInfo.pHdPList = NULL;
	}
	else if(pSysIcon->WLanInfo.pHdPList == pConfig)
	{
		// Head of list is being deleted
		pSysIcon->WLanInfo.pHdPList = pConfig->pNext;
	}

	FreeConfigListEnt(pConfig);
	
	SavePreferredConfigs(&pSysIcon->WLanInfo, &pSysIcon->WLanInfo.IntfEntryEx);
	SetWirelessInfo(&pSysIcon->WLanInfo);
	
	RefreshWZCListView(pSysIcon);

exit:
	WZCQCFG_UNLOCK;
}



/*++

Routine Name: 

	OnConfigureNetwork

Routine Description:

	Handles configure network event

Arguments:

	hwndDlg:	Handle to the parent window

--*/

void OnConfigureNetwork(HWND hwndDlg)
{
	HRESULT					hr;
	SystrayIcon* 			pSysIcon = NULL;
	WLAN_CONFIG_LISTENT*	pConfig = NULL;
	WLAN_CFG_PROPS 			CfgProps;
	HWND					hPropSheet;
	HWND					hDlg;
    BOOL                    fPortrait = (GetSystemMetrics(SM_CXSCREEN) < 480)? TRUE:FALSE;
    UINT                    nDlgId    = fPortrait? IDC_WZC_DLG_PROPS_N:IDC_WZC_DLG_PROPS_W;

	WZCQCFG_LOCK;

	// Get the systray icon for the given window
	hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
	if(FAILED(hr) || !pSysIcon)
	{
		goto exit;
	}

	// If Zero Config is disabled, then quietly exit
	if(! (pSysIcon->WLanInfo.dwCtlFlags & INTFCTL_ENABLED))
	{
		goto exit;
	}

	InitializeCfgProps(&CfgProps, WZCDLG_PROPS_RWALL|WZCDLG_PROPS_NODUPS|WZCDLG_PROPS_DEFOK|WZCDLG_PROPS_CONF,nDlgId);

	if(GetSelectedConfig(pSysIcon->WLanInfo.hwndVLV, &pConfig, NULL) == FALSE)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Could not get the selected config from the list view")));
		goto exit;
	}

	// Check if it is valid to connect to this type of network
	if ((pSysIcon->WLanInfo.IntfEntryEx.dwCtlFlags & INTFCTL_CM_MASK) != Ndis802_11AutoUnknown &&
        (pSysIcon->WLanInfo.IntfEntryEx.dwCtlFlags & INTFCTL_CM_MASK) != (DWORD)pConfig->wzcConfig.InfrastructureMode)
	{
		DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: Cannot configure this type of network")));
		goto exit;
	}

	CopyMemory(&CfgProps.wzcConfig, &pConfig->wzcConfig, sizeof(WZC_WLAN_CONFIG));

	g_fConfiguring = TRUE;

	CfgProps.dwNicCapabilities = pSysIcon->WLanInfo.IntfEntryEx.dwCapabilities;
    CfgProps.bWPAAESCapable    = IsWPAAESCapable((PINTF_80211_CAPABILITY)pSysIcon->WLanInfo.IntfEntryEx.rdNicCapabilities.pData);
	CfgProps.fDlgOk = FALSE;
	hPropSheet = pSysIcon->hPropSheet;

	// Display config dialog
	hDlg = CreateDialogParam(v_hInst, 
                              MAKEINTRESOURCE(nDlgId), 
                              hwndDlg, 
                              NetworkWLanPropsDlgProc,
                              (LPARAM)&CfgProps);
	if(hDlg)
    {
    	EnableWindow(hPropSheet, FALSE);

   		WZCQCFG_UNLOCK;
    	WZCMiniMsgPump(hDlg);
   		WZCQCFG_LOCK;

   		g_fConfiguring = FALSE;

   		// Since critical section was unlocked, we must check the window is still valid.
    	if(IsWindow(hPropSheet))
	    	EnableWindow(hPropSheet, TRUE);

		// We unlocked so need to reobtain systray icon context
		pSysIcon = NULL;
    	hr = GetDeviceSystrayIcon(hPropSheet, &pSysIcon);
		if(FAILED(hr))
		{
			DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Failed to get systray icon struct")));
			goto exit;
		}
    }
	
	g_fConfiguring = FALSE;
	
	if(CfgProps.fDlgOk)
	{
		// Save configs
		pConfig->wzcConfig = CfgProps.wzcConfig;
		
		SavePreferredConfigs(&pSysIcon->WLanInfo, &pSysIcon->WLanInfo.IntfEntryEx);
		SetWirelessInfo(&pSysIcon->WLanInfo);
	}

exit:
	WZCQCFG_UNLOCK;
}



/*++

Routine Name: 

	OnConnect

Routine Description:

	This function is called when the connect button is pushed.

Arguments:

	hwndDlg:	Handle to the parent window

--*/

void OnConnect(HWND hwndDlg)
{
	HRESULT					hr 					= S_OK;
	BOOL					fVisibleListChanged = FALSE;
	BOOL 					bOk					= FALSE;
	BOOL					fConfigInPrefList	= FALSE;
	WLAN_CONFIG_LISTENT*	pConfig 			= NULL;
	WLAN_CONFIG_LISTENT*	pPrefConfig 		= NULL;
	WLAN_CONFIG_LISTENT*	pTail 				= NULL;
	WLAN_CFG_PROPS 			CfgProps;
	SystrayIcon*			pSysIcon 			= NULL;
	TCHAR					szStatus[MAX_WZC_STATUS];
	DWORD 					dwFlags;
    BOOL                    fPortrait = (GetSystemMetrics(SM_CXSCREEN) < 480)? TRUE:FALSE;
    UINT                    nDlgId    = fPortrait? IDC_WZC_DLG_PROPS_N:IDC_WZC_DLG_PROPS_W;

	// This flag signals the refresh thread to finish up.  We do not want the user to 
	// wait for a long time when they press connect.

	WZCQCFG_LOCK;

	// Get the systray icon for the given window
	hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
	if(FAILED(hr) || !pSysIcon)
	{
		goto exit;
	}

	// If Zero Config is disabled, then quietly exit
	if(! (pSysIcon->WLanInfo.dwCtlFlags & INTFCTL_ENABLED))
	{
		goto exit;
	}

	InitializeCfgProps(&CfgProps, WZCDLG_PROPS_RWALL|WZCDLG_PROPS_NODUPS|WZCDLG_PROPS_DEFOK|WZCDLG_PROPS_CONF|WZCDLG_PROPS_CONNECT,nDlgId);

	if(GetSelectedConfig(pSysIcon->WLanInfo.hwndVLV, &pConfig, NULL) == FALSE)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Could not get the selected config from the list view")));
		goto exit;
	}

	// Check if it is valid to connect to this type of network
	if ((pSysIcon->WLanInfo.IntfEntryEx.dwCtlFlags & INTFCTL_CM_MASK) != Ndis802_11AutoUnknown &&
        (pSysIcon->WLanInfo.IntfEntryEx.dwCtlFlags & INTFCTL_CM_MASK) != (DWORD)pConfig->wzcConfig.InfrastructureMode)
	{
		DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: Cannot connect to this type of network")));
		goto exit;
	}

	CopyMemory(&CfgProps.wzcConfig, &pConfig->wzcConfig, sizeof(WZC_WLAN_CONFIG));

	// Check if config is in preferred list.
	fConfigInPrefList = GetListConfigBySsid(pSysIcon->WLanInfo.pHdPList, 
									&pConfig->wzcConfig.Ssid, 
									pConfig->wzcConfig.InfrastructureMode,
									&pPrefConfig);
	if(!fConfigInPrefList)
	{
		HWND hPropSheet;
		HWND hDlg;
		
		// if the mode is not "auto", freeze it in the dialog
        if ((pSysIcon->WLanInfo.IntfEntryEx.dwCtlFlags & INTFCTL_CM_MASK) != Ndis802_11AutoUnknown)
        {
            CfgProps.wzcConfig.InfrastructureMode = (NDIS_802_11_NETWORK_INFRASTRUCTURE)(pSysIcon->WLanInfo.IntfEntryEx.dwCtlFlags & INTFCTL_CM_MASK);
            CfgProps.dwFlags &= ~WZCDLG_PROPS_RWINFR;
        }

		CfgProps.dwNicCapabilities = pSysIcon->WLanInfo.IntfEntryEx.dwCapabilities;
        CfgProps.bWPAAESCapable    = IsWPAAESCapable((PINTF_80211_CAPABILITY)pSysIcon->WLanInfo.IntfEntryEx.rdNicCapabilities.pData);            
        CfgProps.pConfigList = pSysIcon->WLanInfo.pHdPList;
        CfgProps.fDlgOk = FALSE;
		hPropSheet = pSysIcon->hPropSheet;

		g_fConfiguring = TRUE;
		
		hDlg = CreateDialogParam(v_hInst, 
                              MAKEINTRESOURCE(nDlgId), 
                              hwndDlg, 
                              NetworkWLanPropsDlgProc,
                              (LPARAM)&CfgProps);
		if(hDlg)
	    {
	    	EnableWindow(hPropSheet, FALSE);

	   		WZCQCFG_UNLOCK;
	    	WZCMiniMsgPump(hDlg);
	   		WZCQCFG_LOCK;

	    	g_fConfiguring = FALSE;

	   		// Since critical section was unlocked, we must check the window is still valid.
	    	if(IsWindow(hPropSheet))
		    	EnableWindow(hPropSheet, TRUE);

			// We unlocked so need to reobtain systray icon context
			pSysIcon = NULL;
	    	hr = GetDeviceSystrayIcon(hPropSheet, &pSysIcon);
			if(FAILED(hr))
			{
				DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Failed to get systray icon struct")));
				goto exit;
			}
	    }

		g_fConfiguring = FALSE;
        
		// If we selected "Cancel" from the dialog then exit quietly.
        if(CfgProps.fDlgOk == FALSE)
    	{
    		hr = S_OK;
    		goto exit;
    	}
	}

	// Display string to indicate we are scanning
	LoadString(v_hInst, IDS_QC_SCANNING, szStatus, sizeof(szStatus)/sizeof(szStatus[0]));
	DisplayWZCStatus(pSysIcon->hWZCPage, szStatus);
	StringCchCopy(pSysIcon->ptcLastWZCStatus, MAX_WZC_STATUS, szStatus);

	// Add the config to the preferred list
	dwFlags = WZC_DESCR_PREFRD;
	pConfig = NULL; // Reset pointer
	AddUniqueConfig(&pSysIcon->WLanInfo,
				WZCADD_OVERWRITE | WZCADD_HIGROUP,
				dwFlags,
				&CfgProps.wzcConfig,
				&pConfig);

	SavePreferredConfigs(&pSysIcon->WLanInfo, &pSysIcon->WLanInfo.IntfEntryEx);
	SetWirelessInfo(&pSysIcon->WLanInfo);

	// Disable the WZC page
	EnableWindow(pSysIcon->WLanInfo.hbtnConnect, FALSE);
	EnableWindow(pSysIcon->WLanInfo.hbtnAdvanced, FALSE);
	EnableWindow(pSysIcon->WLanInfo.hwndVLV, FALSE);
	g_fConnecting = TRUE;
	
exit:

	WZCQCFG_UNLOCK;

} // OnConnect



/*++

Routine Name: 

	DisplayInitialSignalStrength

Routine Description:

	Displays the signal strength on the dialog in a quick manner (i.e. doesn't poll several times).

Arguments:

	pv: 	HWND of the dialog

--*/

DWORD DisplayInitialSignalStrength(HWND hwnd)
{
	HRESULT 		hr;
	SystrayIcon*	pSysIcon = NULL;
	INT				iSignalStrength = 0;
	DWORD			dwRetVal = E_FAIL;
	
	// Get the systray icon for the given window
	hr = GetDeviceSystrayIcon(hwnd, &pSysIcon);
	if(FAILED(hr) || !pSysIcon)
	{
		goto exit;
	}

	hr = GetSignalStrength(pSysIcon->ptcDeviceName, &iSignalStrength);
	if(FAILED(hr))
	{
		goto exit;
	}

	if(iSignalStrength < -90)
		iSignalStrength = 0;
	else if(iSignalStrength < -81)
		iSignalStrength = 1;
	else if(iSignalStrength < -71)
		iSignalStrength = 2;
	else if(iSignalStrength < -67)
		iSignalStrength = 3;
	else if(iSignalStrength < -57)
		iSignalStrength = 4;
	else
		iSignalStrength = 5;

	if(!SetWZCSignalStrength(hwnd, iSignalStrength))
	{
		goto exit;
	}

	dwRetVal = ERROR_SUCCESS;

exit:
	return dwRetVal;		
}


/*++

Routine Name: 

	SignalStrengthThread

Routine Description:

	This thread is spun off to update the signal strength.  It goes away when the dialog is closed.

Arguments:

	pv: 	HWND of the dialog

--*/

DWORD WINAPI SignalStrengthThread(LPVOID pv)
{
	HWND			hwnd 			= (HWND) pv;
	SystrayIcon*	pSysIcon 		= NULL;
	INT				iSignalStrength;
	HRESULT			hr 				= S_OK;
	INT				i				= 0;
	INT				iTmp			= 0;
	DWORD			cdwSamples		= 0;
	TCHAR			ptcDeviceName[MAX_SSID_LEN_UI];
	HANDLE			h;

	WZCQCFG_LOCK;
	
	// Get the systray icon for the given window
	hr = GetDeviceSystrayIcon(hwnd, &pSysIcon);
	if(FAILED(hr) || !pSysIcon)
	{
		WZCQCFG_UNLOCK;
		goto exit;
	}

	// Copy the string to a new buffer.  If the adapter is removed the string
	// will be freed.
	_tcsncpy(ptcDeviceName, pSysIcon->ptcDeviceName, MAX_SSID_LEN_UI);
	ptcDeviceName[MAX_SSID_LEN_UI-1] = '\0';

	h = pSysIcon->hCloseWZCDialogEvent;

	WZCQCFG_UNLOCK;

	while(1)
	{
		cdwSamples		= 0;
		iSignalStrength = 0;

		// Loop through x number of times to get a good number of samples.  You will keep
		// executing in this loop until NDISUIO succeeds n times (where n is the number of samples)
		for(i=0;i<WZC_SIGNAL_STRENGTH_SAMPLES;i++)
		{
			// Total sleep time for all iterations of the loop is
			// approximately timeout * samples
			if(WaitForSingleObject(h, WZC_SIGNAL_STRENGTH_TIMEOUT) != WAIT_TIMEOUT)
			{
				goto exit;
			}

			// Get Signal strength
			hr = GetSignalStrength(ptcDeviceName, &iTmp);
			if(hr == S_OK)
			{
				cdwSamples++;
				iSignalStrength += iTmp;
			}
		}

		// Get the average signal strength for all the samples taken
		iSignalStrength = (int) ((double)iSignalStrength / (double)cdwSamples);
		
		// Convert RSSI data to int representing 6 states (no signal, very low, low, good, very good, excellent)

		if(iSignalStrength < -90)
			iSignalStrength = 0;
		else if(iSignalStrength < -81)
			iSignalStrength = 1;
		else if(iSignalStrength < -71)
			iSignalStrength = 2;
		else if(iSignalStrength < -67)
			iSignalStrength = 3;
		else if(iSignalStrength < -57)
			iSignalStrength = 4;
		else
			iSignalStrength = 5;

		WZCQCFG_LOCK;

		// Get the systray icon for the given window
		pSysIcon = NULL;
		hr = GetDeviceSystrayIcon(hwnd, &pSysIcon);
		if(FAILED(hr) || !pSysIcon)
		{
			WZCQCFG_UNLOCK;
			goto exit;
		}

		if(pSysIcon->fDialogVisible && IsWindow(pSysIcon->hWZCPage))
		{
			PostMessage(pSysIcon->hWZCPage, WM_SETWZCSIGSTRNTH, 0, (LPARAM)iSignalStrength);
		}

		h = pSysIcon->hCloseWZCDialogEvent;

		WZCQCFG_UNLOCK;		
	}

exit:

	DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: Exiting SignalStrengthThread")));
	
	return 0;
	
} // SignalStrengthThread


/*++

Routine Name: 

	RefreshSignalStrength

Routine Description:

	This thread constantly polls NDISUIO for the wireless devices signal strength
	and updates the NETUI dialog.

Arguments:

	pv:		A device info struct which contains the device name and HWND

Return Value:

	DWORD

--*/


long MapWirelessSignalStrengthToRSSI(IN  ULONG ulWss) 
{
    long lRSSI;
    	
    switch(ulWss) {
        case 0:
            lRSSI = -91;
            break;

        case 1:
            lRSSI = -82;
            break;

        case 2:
        	lRSSI = -72;
        	break;

        case 3:
        	lRSSI = - 68;
        	break;

        case 4:
            lRSSI = -58;
            break;

        case 5:
        	lRSSI = -56;
        	break;

        default:
            lRSSI = -92;
            break;
    }

    return (lRSSI);
}




BOOL IsNativeWifiDevice(PTCHAR ptcDeviceName, INT *piSignalStrength)
{
#ifdef NATIVEWIFI

    NATIVEWIFI_STATISTICS   NwifiStatistics = {0};
    DWORD                   dwBytesWritten = 0;
    BOOL                    fWireless = FALSE;

    NwifiStatistics.ptcDeviceName = ptcDeviceName;

    if(DeviceIoControl(
            g_hNdisUio,
            IOCTL_NDISUIO_NATIVEWIFI_STATISTICS,
            NULL,
            0,
            &NwifiStatistics,
            sizeof(NATIVEWIFI_STATISTICS),
            &dwBytesWritten,
            NULL))
    {
        if(NwifiStatistics.PhysicalMediaType == NdisPhysicalMediumNativeWifi && NwifiStatistics.bStationMode)
        {
            DWORD   dwSignalLevel;
            
            fWireless = TRUE;

            RETAILMSG (1, (TEXT("Nwifi Strength = [%d]\r\n"),
                NwifiStatistics.ulRSSI));            

            dwSignalLevel = ((NwifiStatistics.ulRSSI - STA_MIN_QUALITY) * 5 ) / (STA_MAX_QUALITY - STA_MIN_QUALITY);

            if (piSignalStrength)
                *piSignalStrength = MapWirelessSignalStrengthToRSSI(dwSignalLevel);
        }
    }

    return fWireless;

#else

    return  FALSE;

#endif
    
} // IsDeviceWireless


HRESULT	GetSignalStrength(PTCHAR ptcDeviceName, INT* piSignalStrength)
{
	BOOL				fRetVal 		= TRUE;
	HRESULT				hr 				= S_OK;
	PNDISUIO_QUERY_OID	pQueryOid		= NULL;
	DWORD				dwBytesReturned = 0;
	UCHAR				QueryBuffer[sizeof(NDISUIO_QUERY_OID)+sizeof(DWORD)];

    ASSERT(piSignalStrength);

	if (IsNativeWifiDevice(ptcDeviceName, piSignalStrength))
	{
	    //
	    //  Use Nativewifi API to get the RSSI.
	    //

        DEBUGMSG (0, (TEXT("SignalStrength = [%d]\r\n"),
            *piSignalStrength));
	    
        hr = S_OK;
	}
	else
	{    	
    	// Get Signal strength
    	pQueryOid = (PNDISUIO_QUERY_OID)&QueryBuffer[0];
    	pQueryOid->ptcDeviceName = ptcDeviceName;
    	pQueryOid->Oid = OID_802_11_RSSI;

    	fRetVal = DeviceIoControl(
    				g_hNdisUio,
    				IOCTL_NDISUIO_QUERY_OID_VALUE,
    				(LPVOID) pQueryOid,
    				sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
    				(LPVOID) pQueryOid,
    				sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
    				&dwBytesReturned,
    				NULL);
    	if(fRetVal == TRUE && piSignalStrength)
    	{
    		hr = S_OK;
    		*piSignalStrength = *(DWORD *)&pQueryOid->Data;
    	}
    	else
    	{
    		hr = E_FAIL;
    	}
    }    	

	return hr;
	
} // GetSignalStrength

void CheckAdhocPackets(PTCHAR ptcDeviceName, UCHAR *pSSID)
{
	BOOL				fRetVal 		= TRUE;
	PNDISUIO_QUERY_OID	pQueryOid		= NULL;
	DWORD				dwBytesReturned = 0;
	UCHAR				*pQueryBuffer;
	NDIS_802_11_BSSID_LIST_EX	*pBSSIDList;
	NDIS_WLAN_BSSID_EX	*pBSSID;
	ULONG				ulNumOfItems;
	HANDLE				hProcessHeap;
	int					nQueryBufferSize = 4096; /*sizeof(NDISUIO_QUERY_OID) + sizeof(NDIS_802_11_BSSID_LIST_EX) * 2;*/

#define MAX_QUERY_SIZE 32 * 1024

#ifndef SHIP_BUILD
	char				buf[32];
#endif

	hProcessHeap = GetProcessHeap();
	if (!hProcessHeap)
	{
		RETAILMSG(1, (TEXT("NETUIQC: CheckAdhocPackets -> GetProcessHeap failed, GetLastError = %d\r\n"), GetLastError()));
		return;
	}

	do
	{
		pQueryBuffer = HeapAlloc(hProcessHeap, 0, nQueryBufferSize);
		if (!pQueryBuffer)
		{
			RETAILMSG(1, (TEXT("NETUIQC: CheckAdhocPackets -> HeapAlloc failed, GetLastError = %d\r\n"), GetLastError()));
			fRetVal = FALSE;
			break;
		}

		pQueryOid = (PNDISUIO_QUERY_OID)pQueryBuffer;
		pQueryOid->ptcDeviceName = ptcDeviceName;
		pQueryOid->Oid = OID_802_11_BSSID_LIST;

		fRetVal = DeviceIoControl(g_hNdisUio, IOCTL_NDISUIO_QUERY_OID_VALUE, 
			(LPVOID) pQueryOid, nQueryBufferSize, (LPVOID) pQueryOid, nQueryBufferSize, &dwBytesReturned, NULL);

		if (!fRetVal)
		{
			if (GetLastError() == ERROR_INVALID_USER_BUFFER)
			{
				HeapFree(hProcessHeap, 0, pQueryBuffer);
				nQueryBufferSize *= 2;
				RETAILMSG(1, (TEXT("NETUIQC: CheckAdhocPackets -> DeviceIoControl failed -> ERROR_INVALID_USER_BUFFER, try nQueryBufferSize = %d\r\n"), nQueryBufferSize));
			}
			else
				break;
		}
	}
	while (!fRetVal && nQueryBufferSize < MAX_QUERY_SIZE);

	if (fRetVal)
	{
		pBSSIDList = (NDIS_802_11_BSSID_LIST_EX *)pQueryOid->Data;
		pBSSID = &pBSSIDList->Bssid[0];
		RETAILMSG(1, (TEXT("NETUIQC: CheckAdhocPackets -> pBSSIDList->NumberOfItems = %d\r\n"), pBSSIDList->NumberOfItems));

		for (ulNumOfItems = pBSSIDList->NumberOfItems; ulNumOfItems; ulNumOfItems--)
		{
#ifndef SHIP_BUILD
			memset(buf, 0, sizeof(buf));
			strncpy(buf, pBSSID->Ssid.Ssid, pBSSID->Ssid.SsidLength);
			RETAILMSG(1, (TEXT("NETUIQC: CheckAdhocPackets -> Ssid = '%S', IELength = %d\r\n"), buf, pBSSID->IELength));
#endif
			if (strlen(pSSID) == pBSSID->Ssid.SsidLength && strncmp(pSSID, pBSSID->Ssid.Ssid, pBSSID->Ssid.SsidLength) == 0)
			{
				if (pBSSID->IELength == 0)
				{
					pQueryOid->Oid = OID_802_11_DISASSOCIATE;
					fRetVal = DeviceIoControl(
					g_hNdisUio,
					IOCTL_NDISUIO_SET_OID_VALUE,
					(LPVOID) pQueryOid,
					sizeof(NDISUIO_QUERY_OID),
					NULL,
					0,
					NULL,
					NULL);

					if (!fRetVal)
					{
						RETAILMSG(1, (TEXT("NETUIQC: CheckAdhocPackets -> OID_802_11_DISASSOCIATE failed\r\n")));
					}
				}

				break;
			}

			pBSSID = (NDIS_WLAN_BSSID_EX *)((PBYTE)pBSSID + pBSSID->Length);
		}
	}

	if (pQueryBuffer)
		HeapFree(hProcessHeap, 0, pQueryBuffer);
} 


/*++

Routine Name: 

	SetWZCSignalStrength

Routine Description:

	Set the Signal strength of the wireless device

Arguments:

	hWnd:				HWND of the property sheet or main dialog
	iSignalStrength:	The signal strength

Return Value:

	TRUE:	Success
	FALSE:	Failure
	
--*/

BOOL SetWZCSignalStrength(HWND hWnd, INT iSignalStrength)
{
	BOOL 		fSuccess = TRUE;
	HRESULT		hr = S_OK;
	HWND		hSignalStrengthLbl;
	SystrayIcon	*pSysIcon = NULL;
	TCHAR		ptcStatus[MAX_WZC_STATUS];
	TCHAR		ptcNotConnected[MAX_WZC_STATUS];
	HWND		hStatus;
	TCHAR		ptcSignalLevel[MAX_WZC_SIGNAL_LEVEL_STR];
	TCHAR		ptcPrevSignalLevel[MAX_WZC_SIGNAL_LEVEL_STR];
	TCHAR		szSignal[MAX_WZC_SIGNAL_LEVEL_STR];

	WZCQCFG_LOCK;

	DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: Set signal strength")));
	
	// Get the systray icon for the given window
	hr = GetDeviceSystrayIcon(hWnd, &pSysIcon);
	if(FAILED(hr) || !pSysIcon)
	{
		fSuccess = FALSE;
		goto exit;
	}

	// If dialog is not visible then return FALSE
	if(pSysIcon->fDialogVisible == FALSE)
	{
		fSuccess = FALSE;
		goto exit;
	}

	// If our status is currently "Not connected" then display "No signal"
	LoadString(v_hInst, IDS_QC_NOT_CONNECTED, ptcNotConnected, MAX_WZC_STATUS);
	hStatus = GetDlgItem(pSysIcon->hWZCPage, IDC_WZCQCFG_STATUS_LBL_INF);
	GetWindowText(hStatus, ptcStatus, MAX_WZC_STATUS);
	if(_tcscmp(ptcStatus, ptcNotConnected) == 0)
	{
		iSignalStrength = 0;
	}
	
	// Get the UI component to display the signal strength
	hSignalStrengthLbl = GetDlgItem(pSysIcon->hWZCPage, IDC_WZCQCFG_SIGNAL_LBL_INF);
	if(hSignalStrengthLbl)
	{
		// Determine the text representation of the signal strength
		if(iSignalStrength == 0)
		{
			LoadString(v_hInst, IDS_QC_SIGNAL0, szSignal, sizeof(szSignal)/sizeof(szSignal[0]));
			_tcscpy(ptcSignalLevel, szSignal);
		}
		else if(iSignalStrength == 1)
		{
			LoadString(v_hInst, IDS_QC_SIGNAL1, szSignal, sizeof(szSignal)/sizeof(szSignal[0]));
			_tcscpy(ptcSignalLevel, szSignal);
		}
		else if(iSignalStrength == 2)
		{
			LoadString(v_hInst, IDS_QC_SIGNAL2, szSignal, sizeof(szSignal)/sizeof(szSignal[0]));
			_tcscpy(ptcSignalLevel, szSignal);
		}
		else if(iSignalStrength == 3)
		{
			LoadString(v_hInst, IDS_QC_SIGNAL3, szSignal, sizeof(szSignal)/sizeof(szSignal[0]));
			_tcscpy(ptcSignalLevel, szSignal);
		}
		else if(iSignalStrength == 4)
		{
			LoadString(v_hInst, IDS_QC_SIGNAL4, szSignal, sizeof(szSignal)/sizeof(szSignal[0]));
			_tcscpy(ptcSignalLevel, szSignal);
		}
		else
		{
			LoadString(v_hInst, IDS_QC_SIGNAL5, szSignal, sizeof(szSignal)/sizeof(szSignal[0]));
			_tcscpy(ptcSignalLevel, szSignal);
		}

		// Only refresh the text if it is different
		GetWindowText(hSignalStrengthLbl, ptcPrevSignalLevel, MAX_WZC_SIGNAL_LEVEL_STR);
		if(_tcscmp(ptcPrevSignalLevel, ptcSignalLevel) != 0)
		{
			SetWindowText(hSignalStrengthLbl, ptcSignalLevel);
		}
	}

exit:
	WZCQCFG_UNLOCK;
	
	return fSuccess;

} // SetWZCSignalStrength



/*++

Routine Name: 

	HandleWZCMessage

Routine Description:

	This function handles log entry generated by the Zeroconfig message queue.

Arguments:

	pLogEntry:	Log entry from ZeroConfig Message Queue

--*/

void HandleWZCMessage(LOG_ENTRY* pLogEntry, BOOL fShowStatus)
{
    HRESULT			hr			= S_OK;
    SystrayIcon*	pSysIcon 	= NULL;
    TCHAR			szStatus[MAX_WZC_STATUS]; // array large enough to hold any status string
    TCHAR*			pszStatusMsg; // array large enough to hold the entire message
    WCHAR			wszSSID[MAX_SSID_LEN+1];
    BOOL            bSetConnected = FALSE;

    WZCQCFG_LOCK;
    
    // Get systray icon struct for the given adapter
    hr = GetDeviceSystrayIconByName(pLogEntry->ptcAdapterName, &pSysIcon);
    if(FAILED(hr))
    {
        goto exit;
    }
    
    // Get the SSID
    MultiByteToWideChar(CP_ACP, 0,
        pLogEntry->pucSSID,
        strlen(pLogEntry->pucSSID)+1,
        wszSSID,
        MAX_SSID_LEN+1);    // This will pickup the end NULL also.

    // Allocate memory for the status message
    pszStatusMsg = LocalAlloc(0, MAX_WZC_STATUS*sizeof(TCHAR));
    if(pszStatusMsg == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Out of memory")));
        goto exit;
    }
    
    wcscpy(pszStatusMsg, L"");
    
    // If the dialog is not visible, then don't bother showing status
    // even if the caller wanted to
    if(pSysIcon->fDialogVisible == FALSE)
    {
        fShowStatus = FALSE;
    }
    
    // Handle notification			
    switch (pLogEntry->dwLogId)
    {
    case WZC_VISIBLE_NETWORK:
        {
            // A new network has appeared and Zero Config has failed to connect
            DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: ZeroConfig MsgQueue -> Visible Network appeared, while not connected")));
            
            // Do not do anything here unless popup is enabled, the dialog is not visible, and we are currently
            // in a failed state
            if(g_fPopup && 
                !pSysIcon->fDialogVisible &&
                pSysIcon->dwWZCState != WZC_ASSOCIATING &&
                pSysIcon->dwWZCState != WZC_SUCCESSFUL_ASSOCIATION &&
                pSysIcon->dwWZCState != WZC_AUTHENTICATING &&
                pSysIcon->dwWZCState != WZC_AUTHENTICATED &&
                pSysIcon->dwWZCState != WZC_REAUTHENTICATING)
            {
                // Refresh wireless info to get updated info on visible network
                RefreshWirelessInfo(pSysIcon);
                
                if(AreNetworksCached(&pSysIcon->WLanInfo) == FALSE)
                {
                    PostMessage(pSysIcon->hMainWnd, WM_WZCSYSTRAYICON, 1, WM_LBUTTONDBLCLK);
                }
            }
        }
        break;
        
    case WZC_ASSOCIATING:
        
        DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: ZeroConfig MsgQueue -> Associating with %s"), wszSSID));
        
        // Get the string to display
        LoadString(v_hInst, IDS_QC_ASSOCIATING, szStatus, sizeof(szStatus)/sizeof(szStatus[0]));
        if(pLogEntry->dwInfrastructureMode == Ndis802_11IBSS)
        {
            TranslateMsgToAdhoc(szStatus);
        }
        
        // Fill in the network and display the string
        StringCchPrintf(pszStatusMsg, MAX_WZC_STATUS,  szStatus, wszSSID);
        
        AddLoggingMsg(pLogEntry->ptcAdapterName, NULL, pLogEntry->ftTimeStamp, pszStatusMsg);
        pSysIcon->dwWZCState = pLogEntry->dwLogId;
        
        if(fShowStatus)
        {
            PostMessage(pSysIcon->hWZCPage, WM_REFRESH_WZC, 0, 0);
        }
        
        break;
        
    case WZC_FAILED_ASSOCIATION:
        
        DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: ZeroConfig MsgQueue -> Failed to associate with %s"), wszSSID));
        
        // Update status field
        LoadString(v_hInst, IDS_QC_FAILED_ASSOCIATE, szStatus, sizeof(szStatus)/sizeof(szStatus[0]));
        if(pLogEntry->dwInfrastructureMode == Ndis802_11IBSS)
        {
            TranslateMsgToAdhoc(szStatus);
        }
        
        // Fill in the network and display the string
        StringCchPrintf(pszStatusMsg, MAX_WZC_STATUS,  szStatus, wszSSID);
        
        AddLoggingMsg(pLogEntry->ptcAdapterName, NULL, pLogEntry->ftTimeStamp,pszStatusMsg);
        pSysIcon->dwWZCState = pLogEntry->dwLogId;
        
        if(fShowStatus)
        {
            PostMessage(pSysIcon->hWZCPage, WM_REFRESH_WZC, 0, 0);
            
            // Enable Controls
            EnableWindow(pSysIcon->WLanInfo.hbtnAdvanced, TRUE);
            EnableWindow(pSysIcon->WLanInfo.hwndVLV, TRUE);
            PostMessage(pSysIcon->hWZCPage, WM_REFRESH_BUTTONS, 0, 0);
        }
        
        break;
        
    case WZC_ASSOCIATED_REQUIRES_8021X:
    case WZC_ASSOCIATED_NO_8021X:
        
        DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: ZeroConfig MsgQueue -> Associated with %s"), wszSSID));
        
        // Update status field
        LoadString(v_hInst, IDS_QC_ASSOCIATED, szStatus, sizeof(szStatus)/sizeof(szStatus[0]));
        if(pLogEntry->dwInfrastructureMode == Ndis802_11IBSS)
        {
            TranslateMsgToAdhoc(szStatus);
        }
        
        // Fill in the network and display the string
        StringCchPrintf(pszStatusMsg, MAX_WZC_STATUS,  szStatus, wszSSID);
        
        AddLoggingMsg(pLogEntry->ptcAdapterName, pLogEntry->pucBSSID, pLogEntry->ftTimeStamp, pszStatusMsg);
        pSysIcon->dwWZCState = pLogEntry->dwLogId;
        
        // If we have associated then clear the cache
        WZCCacheClear();
        
        if(fShowStatus)
        {
            DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: --- Refreshing UI based on successful association ---"), wszSSID));
            
            PostMessage(pSysIcon->hWZCPage, WM_REFRESH_WZC, 0, 0);
            
            // Enable Controls
            EnableWindow(pSysIcon->WLanInfo.hbtnAdvanced, TRUE);
            EnableWindow(pSysIcon->WLanInfo.hwndVLV, TRUE);
            PostMessage(pSysIcon->hWZCPage, WM_REFRESH_BUTTONS, 0, 0);
        }
        
        wcscpy(pSysIcon->szActiveWZCNetwork, wszSSID);
        pSysIcon->dwActiveWZCNetworkInfraMode = pLogEntry->dwInfrastructureMode;
        
        // If we are associated (and do not need to authenticate) and our IP is valid, let's display connected
        if((pLogEntry->dwLogId == WZC_ASSOCIATED_NO_8021X) && pSysIcon->fIPState)
        {
            bSetConnected = TRUE;
        }

		if (g_fCheckAdhocPackets && pLogEntry->dwInfrastructureMode == Ndis802_11IBSS)
        {
			CheckAdhocPackets(pSysIcon->ptcDeviceName, pLogEntry->pucSSID);
        }
        
        break;
        
    case WZC_AUTHENTICATING:
        
        DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: ZeroConfig MsgQueue -> Authenticating with %s"), wszSSID));
        
        // Update status field
        LoadString(v_hInst, IDS_QC_AUTHENTICATING, szStatus, sizeof(szStatus)/sizeof(szStatus[0]));
        if(pLogEntry->dwInfrastructureMode == Ndis802_11IBSS)
        {
            TranslateMsgToAdhoc(szStatus);
        }
        
        // Fill in the network and display the string
        StringCchPrintf(pszStatusMsg, MAX_WZC_STATUS,  szStatus, wszSSID);
        
        AddLoggingMsg(pLogEntry->ptcAdapterName, pLogEntry->pucBSSID, pLogEntry->ftTimeStamp, pszStatusMsg);
        pSysIcon->dwWZCState = pLogEntry->dwLogId;
        
        if(fShowStatus)
        {
            PostMessage(pSysIcon->hWZCPage, WM_REFRESH_WZC, 0, 0);
        }
        
        break;
        
    case WZC_AUTHENTICATED:
        
        DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: ZeroConfig MsgQueue -> Authenticated with %s"), wszSSID));
        
        // Update status field
        LoadString(v_hInst, IDS_QC_AUTHENTICATED, szStatus, sizeof(szStatus)/sizeof(szStatus[0]));
        if(pLogEntry->dwInfrastructureMode == Ndis802_11IBSS)
        {
            TranslateMsgToAdhoc(szStatus);
        }
        
        // Fill in the network and display the string
        StringCchPrintf(pszStatusMsg, MAX_WZC_STATUS,  szStatus, wszSSID);
        
        AddLoggingMsg(pLogEntry->ptcAdapterName, pLogEntry->pucBSSID, pLogEntry->ftTimeStamp, pszStatusMsg);
        
        if(fShowStatus)
        {
            // If we just switched from reauthenticating to authenticating, then do not
            // bother to display the "Authenticated" message.  Just skip to "Connected".
            if(pSysIcon->dwWZCState == WZC_REAUTHENTICATING && pSysIcon->fIPState)
            {
                // Update the wireless status field
                bSetConnected = TRUE;
            }
            
            PostMessage(pSysIcon->hWZCPage, WM_REFRESH_WZC, 0, 0);
        }
        
        pSysIcon->dwWZCState = pLogEntry->dwLogId;
        wcscpy(pSysIcon->szActiveWZCNetwork, wszSSID);
        pSysIcon->dwActiveWZCNetworkInfraMode = pLogEntry->dwInfrastructureMode;
        
        // If we have an IP let's display connected
        if(pSysIcon->fIPState)
        {
            // Update the wireless status field
            bSetConnected = TRUE;
        }
        
        break;
        
    case WZC_CANCEL_AUTH:
        
        DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: ZeroConfig MsgQueue -> Cancelled authentication with %s"), wszSSID));
        
        // Update status field
        LoadString(v_hInst, IDS_QC_CANCEL_AUTHENTICATION, szStatus, sizeof(szStatus)/sizeof(szStatus[0]));
        if(pLogEntry->dwInfrastructureMode == Ndis802_11IBSS)
        {
            TranslateMsgToAdhoc(szStatus);
        }
        
        // Fill in the network and display the string
        StringCchPrintf(pszStatusMsg, MAX_WZC_STATUS,  szStatus, wszSSID);
        
        AddLoggingMsg(pLogEntry->ptcAdapterName, NULL, pLogEntry->ftTimeStamp, pszStatusMsg);
        pSysIcon->dwWZCState = pLogEntry->dwLogId;
        
        if(fShowStatus)
        {
            PostMessage(pSysIcon->hWZCPage, WM_REFRESH_WZC, 0, 0);
        }
        
        break;
        
    case WZC_FAILED_AUTH_NO_RETRY:
        
        DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: ZeroConfig MsgQueue -> Failed to authenticate with %s"), wszSSID));
        
        // Update status field
        LoadString(v_hInst, IDS_QC_FAILED_AUTHENTICATION, szStatus, sizeof(szStatus)/sizeof(szStatus[0]));
        if(pLogEntry->dwInfrastructureMode == Ndis802_11IBSS)
        {
            TranslateMsgToAdhoc(szStatus);
        }
        
        // Fill in the network and display the string
        StringCchPrintf(pszStatusMsg, MAX_WZC_STATUS,  szStatus, wszSSID, pLogEntry->dwArg);
        
        AddLoggingMsg(pLogEntry->ptcAdapterName, NULL, pLogEntry->ftTimeStamp, pszStatusMsg);
        pSysIcon->dwWZCState = pLogEntry->dwLogId;
        
        if(fShowStatus)
        {
            PostMessage(pSysIcon->hWZCPage, WM_REFRESH_WZC, 0, 0);
        }
        
        break;
        
    case WZC_FAILED_AUTH_WILL_RETRY:
        
        DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: ZeroConfig MsgQueue -> Failed to authenticate (will retry) with %s"), wszSSID));
        
        // Update status field
        LoadString(v_hInst, IDS_QC_FAILED_AUTHENTICATION, szStatus, sizeof(szStatus)/sizeof(szStatus[0]));
        if(pLogEntry->dwInfrastructureMode == Ndis802_11IBSS)
        {
            TranslateMsgToAdhoc(szStatus);
        }
        
        // Fill in the network and display the string
        StringCchPrintf(pszStatusMsg, MAX_WZC_STATUS,  szStatus, wszSSID, pLogEntry->dwArg);
        
        AddLoggingMsg(pLogEntry->ptcAdapterName, NULL, pLogEntry->ftTimeStamp, pszStatusMsg);
        pSysIcon->dwWZCState = pLogEntry->dwLogId;
        
        if(fShowStatus)
        {
            PostMessage(pSysIcon->hWZCPage, WM_REFRESH_WZC, 0, 0);
        }
        
        break;
        
    case WZC_REAUTHENTICATING:
        
        DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: ZeroConfig MsgQueue -> Reauthenticating with %s"), wszSSID));
        
        // Update status field
        LoadString(v_hInst, IDS_QC_REAUTHENTICATING, szStatus, sizeof(szStatus)/sizeof(szStatus[0]));
        if(pLogEntry->dwInfrastructureMode == Ndis802_11IBSS)
        {
            TranslateMsgToAdhoc(szStatus);
        }
        
        // Fill in the network and display the string
        StringCchPrintf(pszStatusMsg, MAX_WZC_STATUS, szStatus, wszSSID);
        
        AddLoggingMsg(pLogEntry->ptcAdapterName, pLogEntry->pucBSSID, pLogEntry->ftTimeStamp, pszStatusMsg);
        pSysIcon->dwWZCState = pLogEntry->dwLogId;
        
        if(fShowStatus)
        {
            PostMessage(pSysIcon->hWZCPage, WM_REFRESH_WZC, 0, 0);
        }
        
        break;			
        
    case WZC_PREFERED_LIST_EXHAUSTED:
        
        DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: ZeroConfig MsgQueue -> Preffered List exhausted")));
        
        // Update status field
        LoadString(v_hInst, IDS_QC_NOT_CONNECTED, pszStatusMsg, MAX_WZC_STATUS);
        pSysIcon->dwWZCState = pLogEntry->dwLogId;
        
        if(fShowStatus)
        {
            PostMessage(pSysIcon->hWZCPage, WM_REFRESH_WZC, 0, 0);
            
            // If we get this message, let's ensure the timer goes off soon so
            // we can update with zero config
            SetTimer(pSysIcon->hWZCPage, 1, (WZC_RFSH_TIMEOUT + WZCQC_RFSH_TIMEOUT), NULL);
        }
        
        // Display a different message to the advanced logging dialog
        LoadString(v_hInst, IDS_QC_PREF_LIST_EXHAUSTED, szStatus, sizeof(szStatus)/sizeof(szStatus[0]));
        AddLoggingMsg(pLogEntry->ptcAdapterName, NULL, pLogEntry->ftTimeStamp, szStatus);
        
        wcscpy(pSysIcon->szActiveWZCNetwork, TEXT(""));
        pSysIcon->dwActiveWZCNetworkInfraMode = 0;
        
        break;
        
        // Cases only displayed to advanced logging dialog
    case WZC_STARTED:
        
        LoadString(v_hInst, IDS_QC_WZCSTARTED, szStatus, sizeof(szStatus)/sizeof(szStatus[0]));
        AddLoggingMsg(pLogEntry->ptcAdapterName, NULL, pLogEntry->ftTimeStamp, szStatus);
        
        break;
        
    case WZC_CLOSE_EAPOL_SESSION:
        
        LoadString(v_hInst, IDS_QC_CLOSE_EAPOL_SESSION, szStatus, sizeof(szStatus)/sizeof(szStatus[0]));
        AddLoggingMsg(pLogEntry->ptcAdapterName, NULL, pLogEntry->ftTimeStamp, szStatus);        
        break;

    case WZC_CONFIGURED:
        if(fShowStatus)
        {
            // Enable Controls
            EnableWindow(pSysIcon->WLanInfo.hbtnAdvanced, TRUE);
            EnableWindow(pSysIcon->WLanInfo.hwndVLV, TRUE);
            PostMessage(pSysIcon->hWZCPage, WM_REFRESH_BUTTONS, 0, 0);
            bSetConnected = TRUE;

        }            
        break;
        
    default:
        break;
        
    }


    // Set the connected status.
    if (bSetConnected) 
    {
        // If we have an IP let's display connected
        if(pSysIcon->fIPState)
        {
            // Update the wireless status field
            LoadString(v_hInst, IDS_QC_CONNECTED, szStatus, sizeof(szStatus)/sizeof(szStatus[0]));
            if(pLogEntry->dwInfrastructureMode == Ndis802_11IBSS)
            {
                TranslateMsgToAdhoc(szStatus);
            }
            
            StringCchPrintf(pszStatusMsg, MAX_WZC_STATUS,  szStatus, pSysIcon->szActiveWZCNetwork);
        }
    }        
    
    // Store the last message in the SystrayIcon struct
    if(_tcscmp(pszStatusMsg, L"") != 0)
    {
        StringCchCopy(pSysIcon->ptcLastWZCStatus, MAX_WZC_STATUS, pszStatusMsg);
        if(fShowStatus)
        {
            // This handler of this messages, will call LocalFree()
            PostMessage(pSysIcon->hWZCPage, WM_SETWZCSTATUS, 0, (LPARAM)pszStatusMsg);
        }
        else
        {
            LocalFree(pszStatusMsg);
        }
    }
    else
    {
        LocalFree(pszStatusMsg);
    }
    
exit:
    
    WZCQCFG_UNLOCK;
    
    return;
    
} // HandleWZCMessage



/*++

Routine Name: 

	UpdateStatusThread

Routine Description:

	This thread is spun off to update the status field on the wireless dialog.  It waits for messages
	from ZeroConfig and handles them.

--*/

DWORD WINAPI UpdateStatusThread(LPVOID pv)
{
	BOOL	            bStatus 			= TRUE;
    MSGQUEUEOPTIONS     sOptions;
    HANDLE				hMsgQueue;
    HANDLE 				hEventHandles[2];
    LOG_ENTRY  			LogEntry 			= {0};
    LOG_ENTRY			TmpLogEntry;
    DWORD      			dwFlags;
    DWORD      			dwBytesReturned;
    DWORD				dwRetVal 			= 0;

	//
	//	Open the message queue..
	//

	sOptions.dwSize					= sizeof(MSGQUEUEOPTIONS);
	sOptions.dwFlags				= MSGQUEUE_ALLOW_BROKEN;
	sOptions.dwMaxMessages			= DEFAULT_MAX_MESSAGES;
	sOptions.cbMaxMessage			= sizeof(LOG_ENTRY);
	sOptions.bReadAccess			= TRUE;
	
	hMsgQueue = CreateMsgQueue(WZC_MSGQUEUE_NAME, &sOptions);
	if (hMsgQueue == NULL)
	{
        goto exit;
	}

	// Read the initial entries in the queue and only keep the latest entry to display
	while (ReadMsgQueue(
					hMsgQueue,
					&TmpLogEntry,
					sizeof(LOG_ENTRY),
					&dwBytesReturned,
					1,
					&dwFlags))
	{
		// Handle the message but do not display to status field (only logging window)
		HandleWZCMessage(&LogEntry, FALSE);
		LogEntry = TmpLogEntry;
	}

	if(LogEntry.dwLogId != 0)
	{
		HandleWZCMessage(&LogEntry, TRUE);
	}

	// Update the status message to say connected if we have a valid IP address
	UpdateToConnected();

    //
    //  Loop retrieving the queue.  If all wireless dialogs are closed there is no point in
    //  continuing.  So close the message queue.
    //

	DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: Entering ZeroConfig MsgQueue loop")));

	hEventHandles[0] = g_hStopUpdateStatus;
    hEventHandles[1] = hMsgQueue;

	while(1) 
	{
		dwRetVal = WaitForMultipleObjects(2, hEventHandles, FALSE, INFINITE);
		if(1 != (dwRetVal - WAIT_OBJECT_0))
		{
			break;
		}

		while (ReadMsgQueue(
					hMsgQueue,
					&LogEntry,
					sizeof(LOG_ENTRY),
					&dwBytesReturned,
					1,
					&dwFlags))
		{
			//
			//	Okay, we have notification..  
			//

			DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: Received ZeroConfig Status Message")));
			HandleWZCMessage(&LogEntry, TRUE);

			// Sleep for a bit to give the user time to read the status
            Sleep(WZC_STATUS_UPDATE);
        }			
	}

    CloseMsgQueue(hMsgQueue);

exit:
	
	DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: Leaving ZeroConfig MsgQueue thread")));

	return 0;
	
} // UpdateStatusThread


/*++

Routine Name: 

	DisplayWZCStatus

Routine Description:

	Displays the specified message in the Zero Config dialog status label

Arguments:

	hwndDlg:	The property page hwnd
	ptcStatus:	The message to display

--*/

void DisplayWZCStatus(HWND hwndDlg, PTCHAR ptcStatus)
{
	HWND 	hStatusLbl;
	RECT	rect;
	SIZE	size;
	DWORD	cdwWidth;
	DWORD	cdwText, cdwTextStart;
	HDC		hDC;
	TCHAR	szNewStatus[MAX_WZC_STATUS];

	DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: Set Zero Config status message")));

	// Check the window is still valid since 
	if(IsWindow(hwndDlg))
	{
		hStatusLbl = GetDlgItem(hwndDlg, IDC_WZCQCFG_STATUS_LBL_INF);

		// Let's make sure we insert "..." if the text is 
		// going off the end of the label

		// Get label width
		GetWindowRect(hStatusLbl, &rect);
		cdwWidth = rect.right - rect.left;

		// Get text length
		hDC = GetWindowDC(hStatusLbl);
		cdwTextStart = _tcslen(ptcStatus);
		cdwText = cdwTextStart;
		GetTextExtentPoint32(hDC, ptcStatus, cdwText, &size);

		// If string is too long, let's keep making it shorter until it is a good size
		while(cdwWidth < (DWORD) size.cx)
		{
			cdwText--;
			GetTextExtentPoint32(hDC, ptcStatus, cdwText, &size);
		}

		// If we need to add dots then make new string and set window text,
		// otherwise just set original string.
		if(cdwTextStart > cdwText)
		{
			DWORD cchStatus = min(cdwText, (sizeof(szNewStatus)/sizeof(TCHAR)-1));
			if (cchStatus >= 3) 
			{
				cchStatus -= 3; // allow for ellipses
				_tcsncpy(szNewStatus, ptcStatus, cchStatus);
				szNewStatus[cchStatus] = _T('\0');
				
				_tcscat(szNewStatus, _T("..."));
				
				SetWindowText(hStatusLbl, szNewStatus);
			}
		}
		else
		{
			SetWindowText(hStatusLbl, ptcStatus);
		}

		ReleaseDC(hStatusLbl, hDC);
	}
	else
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Cannot update Zero Config status - window is no longer valid")));
	}
}



/*++

Routine Name:

	UpdateToConnected

Routine Description:

	If we have just opened the dialog then we should check if we became connected while the ZC message queue
	was down.  If so update the status message.

--*/

void UpdateToConnected()
{
	SystrayIcon* 	pCurr	= NULL;
	WCHAR			szStatus[MAX_WZC_STATUS];
	
	// Get the wireless dialog
	WZCQCFG_LOCK;

	// Get the hwnd of the specified device
	pCurr = g_pSystrayIconsHead;
	while(pCurr != NULL)
	{
		// If one of the dialogs matches then curr points to the correct data
		if(pCurr->fWireless)
		{
			// Check if we are connected.  If so update the status text
			if(pCurr->fIPState == TRUE && 
				(pCurr->dwWZCState == WZC_AUTHENTICATED || pCurr->dwWZCState == WZC_SUCCESSFUL_ASSOCIATION))
			{
				if(wcscmp(pCurr->szActiveWZCNetwork, TEXT("")) != 0)
				{
					CHAR	szSSID[MAX_SSID_LEN+1];
					WCHAR*	szStatusMsg = NULL;

					szStatusMsg = LocalAlloc(0, MAX_WZC_STATUS*sizeof(TCHAR));
					if(szStatusMsg == NULL)
					{
						DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Out of memory")));
						goto exit;
					}

					// Update the wireless status field
					LoadString(v_hInst, IDS_QC_CONNECTED, szStatus, sizeof(szStatus)/sizeof(szStatus[0]));

					WideCharToMultiByte(CP_ACP, 0,
								pCurr->szActiveWZCNetwork,
								wcslen(pCurr->szActiveWZCNetwork)+1,
								szSSID,
								MAX_SSID_LEN+1,
								NULL, NULL);
					
					if(pCurr->dwActiveWZCNetworkInfraMode == Ndis802_11IBSS)
					{
						TranslateMsgToAdhoc(szStatus);
					}
					
					StringCchPrintf(szStatusMsg, MAX_WZC_STATUS, szStatus, pCurr->szActiveWZCNetwork);
					StringCchCopy(pCurr->ptcLastWZCStatus, MAX_WZC_STATUS, szStatusMsg);
					
					PostMessage(pCurr->hWZCPage, WM_SETWZCSTATUS, 0, (LPARAM)szStatusMsg);
				}
			}
		}

		pCurr = pCurr->next;
	}

exit:
	WZCQCFG_UNLOCK;
	
} // UpdateToConnected



/*++

Routine Name: 

	TranslateMsgToAdhoc

Routine Description:

	This function converts a ZeroConfig status message for a regular network to an Adhoc network by inserting
	a string before the network.

Arguments:

	szMessage:	The message to change. Buffer is MAX_WZC_STATUS wide characters.

--*/

void TranslateMsgToAdhoc(PWCHAR szMessage)
{
	DWORD 	i;
	DWORD 	dwIndex 		= -1;
	WCHAR	pszTmpBuffer[MAX_WZC_STATUS];
	WCHAR*	pTmp;
	WCHAR	szAdhoc[10];
	DWORD   cchAdhoc;
	DWORD   cchMessage;

	// Loop through	the message looking for the '%' where we
	// will insert the "adhoc" string
	cchMessage = wcslen(szMessage);
	for(i=cchMessage;i>0;i--)
	{
		if(szMessage[i] == '%')
		{
			dwIndex = i;
			break;
		}
	}

	// If the '%' was found
	if(dwIndex != -1)
	{
		cchAdhoc = LoadString(v_hInst, IDS_WZC_INFRA_IBSS, szAdhoc, sizeof(szAdhoc)/sizeof(szAdhoc[0]));
		if (! cchAdhoc) 
			return;

		// Make sure string still fits in buffer
		if ((cchAdhoc + cchMessage) >= MAX_WZC_STATUS)
			return;

		// Set pointer to the '%' element and copy the string to temp buffer
		pTmp = &szMessage[dwIndex];
		wcscpy(pszTmpBuffer, pTmp);

		// Append the adhoc string
		wcscpy(pTmp, szAdhoc);

		// Append the temp string to the end again
		wcscat(szMessage, L" ");
		wcscat(szMessage, pszTmpBuffer);		
	}
	
} // TranslateMsgToAdhoc



/*++

Routine Name: 

	GetListConfigBySsid

Routine Description:

	Gets the config struct from the specified SSID (device name)

Arguments:

	pHdList:			List to search (preferred or visible)
	pSsid:				Specified SSID
	ppMatchingConfig:	Config to return
	NetInfra:			Infrastructure Mode

Return Value:

	BOOL

--*/

BOOL GetListConfigBySsid(PWLAN_CONFIG_LISTENT pHdList, NDIS_802_11_SSID *pSsid, NDIS_802_11_NETWORK_INFRASTRUCTURE NetInfra, WLAN_CONFIG_LISTENT **ppMatchingConfig)
{
	BOOL bYes = FALSE;

	if (pHdList != NULL)
	{
 		WLAN_CONFIG_LISTENT    *pConfig;

		pConfig = pHdList;
		do
		{
 			if (pSsid->SsidLength == pConfig->wzcConfig.Ssid.SsidLength &&
 				!strncmp(pConfig->wzcConfig.Ssid.Ssid, pSsid->Ssid, pSsid->SsidLength) &&
 				NetInfra == pConfig->wzcConfig.InfrastructureMode)
			{
				if (ppMatchingConfig != NULL)
					*ppMatchingConfig = pConfig;

				bYes = TRUE;
				break;
			}
 			pConfig = pConfig->pNext;
 		}   
		while(pConfig != pHdList);
	}

	return bYes;

} // GetListConfigBySsid



/*++

Routine Name: 

	UpdateConnectionStatus

Routine Description:

	This function updates the connection status and systray icon state.

Arguments:

	ptcDeviceName:	Adapter Name
	fConnected:		TRUE for connected, FALSE for disconnected

--*/

void WINAPI UpdateConnectionStatus(PTCHAR ptcDeviceName, BOOL fConnected)
{
	HICON					hSysIcon;
	HRESULT					hr 			= S_OK;
	SystrayIcon*			pSysIcon 	= NULL;
	WCHAR					szStatus[MAX_WZC_STATUS];
	
	WZCQCFG_LOCK;

	// Get the systray icon for the given window (if it fails then the device has been removed, so quit the thread)
	hr = GetDeviceSystrayIconByName(ptcDeviceName, &pSysIcon);
	if(FAILED(hr) || !pSysIcon)
	{
		goto exit;
	}

	if(fConnected)
	{
		pSysIcon->fIPState = TRUE;
	
		// Show connected icon
		if(pSysIcon->fWireless == TRUE)
		{
			hSysIcon = (HICON) LoadImage(v_hInst, MAKEINTRESOURCE(IDI_CONNECTED), IMAGE_ICON, 16, 16, 0);
			
			if(wcscmp(pSysIcon->szActiveWZCNetwork, TEXT("")) != 0)
			{
				CHAR	szSSID[MAX_SSID_LEN+1];
				WCHAR*	szStatusMsg = NULL;

				szStatusMsg = LocalAlloc(0, MAX_WZC_STATUS*sizeof(TCHAR));
				if(szStatusMsg == NULL)
				{
					DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Out of memory")));
					goto exit;
				}
				
				// Update the wireless status field
				LoadString(v_hInst, IDS_QC_CONNECTED, szStatus, sizeof(szStatus)/sizeof(szStatus[0]));

				WideCharToMultiByte(CP_ACP, 0,
								pSysIcon->szActiveWZCNetwork,
								wcslen(pSysIcon->szActiveWZCNetwork)+1,
								szSSID,
								MAX_SSID_LEN+1,
								NULL, NULL);

				if(pSysIcon->dwActiveWZCNetworkInfraMode == Ndis802_11IBSS)
				{
					TranslateMsgToAdhoc(szStatus);
				}

				StringCchPrintf(szStatusMsg, MAX_WZC_STATUS, szStatus, pSysIcon->szActiveWZCNetwork);
				StringCchCopy(pSysIcon->ptcLastWZCStatus, MAX_WZC_STATUS, szStatusMsg);

				if(pSysIcon->fDialogVisible && IsWindow(pSysIcon->hWZCPage))
				{
					PostMessage(pSysIcon->hWZCPage, WM_SETWZCSTATUS, 0, (LPARAM)szStatusMsg);
				}
				else
    			    LocalFree(szStatusMsg);
			    
			}
		}
		else
		{
			hSysIcon = (HICON) LoadImage(v_hInst, MAKEINTRESOURCE(IDI_LANCONNECT), IMAGE_ICON, 16, 16, 0);		
		}
	}
	else
	{
		pSysIcon->fIPState = FALSE;
	
		// Show disconnected icon
		if(pSysIcon->fWireless == TRUE)
		{
			hSysIcon = (HICON) LoadImage(v_hInst, MAKEINTRESOURCE(IDI_DISCONNECTED), IMAGE_ICON, 16, 16, 0);	
		}
		else
		{
			hSysIcon = (HICON) LoadImage(v_hInst, MAKEINTRESOURCE(IDI_LANDISCONNECT), IMAGE_ICON, 16, 16, 0);	
		}
	}

	// If the state has changed
	if(pSysIcon->IconData->hIcon != hSysIcon)
	{
		pSysIcon->IconData->hIcon = hSysIcon;
			
		if(pSysIcon->IconData->hIcon == NULL)
		{
			DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Could not get NetUI icon")));
			hr = E_FAIL;
			goto exit;
		}

		// Modify systray icon image
		if(Shell_NotifyIcon(NIM_MODIFY, pSysIcon->IconData) == FALSE)
		{
			DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Error adding icon to systray")));
			hr = E_FAIL;
			goto exit;
		}
	}

exit:
	WZCQCFG_UNLOCK;
	
} // UpdateConnectionStatus


/*++

Routine Name: 

	IsPrefferedListEmpty

Routine Description:

	This function checks if the preffered list of wireless networks is empty.

Arguments:

	pSysIcon:	Systray Icon struct representing the current adapter

Return Value:

	TRUE: 	List is empty
	FALSE:	List is not empty

--*/

BOOL IsPreferredListEmpty(SystrayIcon* pSysIcon)
{
	BOOL fRetVal = FALSE;

	WZCQCFG_LOCK;

	if((pSysIcon->WLanInfo.IntfEntryEx.rdStSSIDList.pData) == NULL || 
		((PWZC_802_11_CONFIG_LIST)(pSysIcon->WLanInfo.IntfEntryEx.rdStSSIDList.pData))->NumberOfItems == 0)
	{
		fRetVal = TRUE;
	}

	WZCQCFG_UNLOCK;

	return fRetVal;
	
} // IsPreferredListEmpty


