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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

Wzcui.c

Abstract:  

    Code for Wireless Zero Config dialogs.  This allows user to set 
    the preferred SSID list for 802.11 networks, and configure various
    parameters such as WEP key.

Functions:


Notes: 

    The zero config service is implemented in two parts -- the wzcsvc.dll which
    runs in device.exe and communicates with wireless miniports, and wczsapi.dll
    which is loaded into the application context and provides APIs to communicate
    with wzcsvc.
    
--*/

#include "QuickConfigUi.h"

// Dynamically load function pointers, as WZC may not be available in all configs.
BOOL					   g_fWZCInit = FALSE;
PFN_WZCQueryInterfaceEx    pfnWZCQueryInterfaceEx;
PFN_WZCSetInterfaceEx      pfnWZCSetInterfaceEx;
PFN_WZCRefreshInterfaceEx  pfnWZCRefreshInterfaceEx;
PFN_WZCEnumEapExtensions   pfnWZCEnumEapExtensions;
PFN_WZCDeleteIntfObjEx     pfnWZCDeleteIntfObjEx;
PFN_WZCQueryContext        pfnWZCQueryContext;
PFN_WZCSetContext          pfnWZCSetContext;
PFN_WZCPassword2Key        pfnWZCPassword2Key;

BOOL
InitZeroConfig(void)
{
	HINSTANCE hWZCLib = NULL;
	LONG lStatus;
	HKEY hRegKey;
	DWORD dwValue;
	DWORD dwSize;
	DWORD dwValType;
	BOOL flag = FALSE;

	lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Drivers\\Builtin\\Ethman"), 0, 0, &hRegKey);
	if (ERROR_SUCCESS == lStatus)
	{	
		dwSize = sizeof(DWORD);
		dwValType = REG_DWORD;

		lStatus = RegQueryValueEx(hRegKey, TEXT("CheckAdhocPackets"), NULL, &dwValType, (LPBYTE)&dwValue, &dwSize);
		if (ERROR_SUCCESS == lStatus)
		{
			if (dwValue == 1)
				flag = TRUE;
		}
		
		RegCloseKey(hRegKey);
	}

	g_fCheckAdhocPackets = flag;

	if (g_fWZCInit) {
		return TRUE; // Already initialized
	}

    // See if zero config API present in system
    if ((hWZCLib = LoadLibrary(L"wzcsapi.dll")) == NULL) {
        DEBUGMSG(ZONE_WARN,(TEXT("!WZCUI: WZCAPI.DLL not present in system\r\n")));
        return FALSE;
    }

    pfnWZCQueryInterfaceEx    = (PFN_WZCQueryInterfaceEx)GetProcAddress(hWZCLib,L"WZCQueryInterfaceEx");
    pfnWZCSetInterfaceEx      = (PFN_WZCSetInterfaceEx)GetProcAddress(hWZCLib,L"WZCSetInterfaceEx");
    pfnWZCRefreshInterfaceEx  = (PFN_WZCRefreshInterfaceEx)GetProcAddress(hWZCLib,L"WZCRefreshInterfaceEx");
    pfnWZCEnumEapExtensions = (PFN_WZCEnumEapExtensions)GetProcAddress(hWZCLib,L"WZCEnumEapExtensions");
    pfnWZCDeleteIntfObjEx     = (PFN_WZCDeleteIntfObjEx)GetProcAddress(hWZCLib,L"WZCDeleteIntfObjEx");
    pfnWZCQueryContext		= (PFN_WZCQueryContext)GetProcAddress(hWZCLib,L"WZCQueryContext");
    pfnWZCSetContext		= (PFN_WZCSetContext)GetProcAddress(hWZCLib,L"WZCSetContext");
    pfnWZCPassword2Key      = (PFN_WZCPassword2Key)GetProcAddress(hWZCLib,L"WZCPassword2Key");

    if ((pfnWZCQueryInterfaceEx == NULL)   ||
        (pfnWZCSetInterfaceEx == NULL)     ||
        (pfnWZCRefreshInterfaceEx == NULL) ||
        (pfnWZCEnumEapExtensions == NULL)||
        (pfnWZCDeleteIntfObjEx == NULL)    ||
        (pfnWZCSetContext == NULL)       ||
        (pfnWZCQueryContext == NULL)) {
        DEBUGMSG(ZONE_ERROR,(TEXT("!WZCUI: WZC APis not present in system\r\n")));
        FreeLibrary(hWZCLib);
        return FALSE;
    }

	g_fWZCInit = TRUE;
    
    return TRUE;
}



//  Given the INTF_80211_CAPABILITIES, determine if the comibination
//  WPA & AES exist.

BOOL IsWPAAESCapable(PINTF_80211_CAPABILITY pCap)
{
    DWORD   i;
    
    if (!pCap)
        return FALSE;

    for (i = 0 ; i < pCap->dwNumOfAuthEncryptPairs ; i++) {

        NDIS_802_11_AUTHENTICATION_MODE Auth    = pCap->AuthEncryptPair[i].AuthModeSupported;
        NDIS_802_11_ENCRYPTION_STATUS   Encrypt = pCap->AuthEncryptPair[i].EncryptStatusSupported;
            
        if ((Auth == Ndis802_11AuthModeWPA || Auth == Ndis802_11AuthModeWPAPSK) &&
            (Encrypt == Ndis802_11Encryption3Enabled)) {            
            return TRUE;
        }            
    }
    return FALSE;

}   //  IsWPAAESCapable()


// Get wireless config info.
BOOL
GetWirelessInfo(WCHAR *szAdapterName, WLAN_INFO *pWLanInfo)
{
	DWORD dwError;
	
	pWLanInfo->IntfEntryEx.wszGuid = szAdapterName;

    // Query zero config info 
    if ((dwError = pfnWZCQueryInterfaceEx(
                    NULL,
                    INTF_ALL,
                    &pWLanInfo->IntfEntryEx,
                    &pWLanInfo->dwOIDFlags)) != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,(TEXT("!WZCUI: WZCQueryInterfaceEx failed 0x%X\r\n"),dwError));
        return FALSE; 
    }

    if ((pWLanInfo->IntfEntryEx.dwCtlFlags & INTFCTL_OIDSSUPP) == 0 ) {
        DEBUGMSG(ZONE_ERROR,(TEXT("!WZCUI: 802.11 OIDs not supported by driver\r\n")));
        pfnWZCDeleteIntfObjEx(&pWLanInfo->IntfEntryEx);
        return FALSE;
    }

    if (pWLanInfo->IntfEntryEx.nAuthMode < 0)
        pWLanInfo->IntfEntryEx.nAuthMode = 0;
    if (pWLanInfo->IntfEntryEx.nInfraMode < 0)
        pWLanInfo->IntfEntryEx.nInfraMode = 0;

    return TRUE;
}



BOOL
SetWirelessInfo(WLAN_INFO *pWLanInfo)
{
    DWORD dwError;
    DWORD dwOutFlags;

    // Apply settings
    if ((dwError = pfnWZCSetInterfaceEx(NULL, INTF_ALL_FLAGS | INTF_PREFLIST,&pWLanInfo->IntfEntryEx,&dwOutFlags)) != ERROR_SUCCESS) {
        DEBUGMSG(1,(TEXT("WZCUI: WZCSetInterfaceEx failed 0x%X\r\n"),dwError));
        return FALSE;
    }
    return TRUE;
}

PWLAN_CONFIG_LISTENT 
AllocAndInitConfigListEnt(DWORD dwFlags, PWZC_WLAN_CONFIG pwzcConfig)
{
    PWLAN_CONFIG_LISTENT pListEnt;
    if ((pListEnt = (PWLAN_CONFIG_LISTENT)LocalAlloc(LPTR,sizeof(WLAN_CONFIG_LISTENT))) == NULL)
        return NULL;
    pListEnt->dwFlags = dwFlags;
    memcpy(&pListEnt->wzcConfig,pwzcConfig,sizeof(WZC_WLAN_CONFIG));
    pListEnt->pPrev = pListEnt->pNext = pListEnt;
    pListEnt->nListIndex = -1;
    return pListEnt;
}

void 
FreeConfigListEnt(PWLAN_CONFIG_LISTENT pListEnt)
{
    pListEnt->pPrev->pNext = pListEnt->pNext;
    pListEnt->pNext->pPrev = pListEnt->pPrev;

	#ifdef DEBUG
	pListEnt->pNext = NULL;
	pListEnt->pPrev = NULL;
	#endif // DEBUG
    
    LocalFree(pListEnt);
}

BOOL
ConfigMatch(PWZC_WLAN_CONFIG pCfg1, PWZC_WLAN_CONFIG pCfg2)
{
    BOOL bMatch;

    // check whether the InfrastructureMode matches
    bMatch = (pCfg1->InfrastructureMode == pCfg2->InfrastructureMode);
    // check whether the SSIDs are of the same length
    bMatch = bMatch && (pCfg1->Ssid.SsidLength == pCfg2->Ssid.SsidLength);
    if (bMatch && pCfg1->Ssid.SsidLength != 0)
    {
        // in case of Non empty SSIDs, check if they're the same
        bMatch = (memcmp(pCfg1->Ssid.Ssid,
                         pCfg2->Ssid.Ssid,
                         pCfg1->Ssid.SsidLength)) == 0;
    }

    return bMatch;
}

BOOL
IsConfigInList(WLAN_CONFIG_LISTENT *pHdList, PWZC_WLAN_CONFIG pwzcConfig, WLAN_CONFIG_LISTENT **ppMatchingConfig)
{
    BOOL bYes = FALSE;

    if (pHdList != NULL)
    {
        WLAN_CONFIG_LISTENT    *pConfig;

        pConfig = pHdList;
        do
        {
            if (ConfigMatch(&pConfig->wzcConfig, pwzcConfig))
            {
                if (ppMatchingConfig != NULL)
                    *ppMatchingConfig = pConfig;

                bYes = TRUE;
                break;
            }
            pConfig = pConfig->pNext;
         }   while(pConfig != pHdList);
    }

    return bYes;
}



DWORD
AddConfigToListView(PWLAN_CONFIG_LISTENT pConfig,HWND hwndLV, INT nPos)
{
    DWORD   dwErr = ERROR_SUCCESS;
    // SSID can't exceed 32 chars (see NDIS_802_11_SSID from ntddndis.h)
    WCHAR   wszSSID[sizeof(WCHAR)*33];
    UINT    nLenSSID = 0;

    // convert the LPSTR (original SSID format) to LPWSTR (needed in List Ctrl)
    if (strcmp(pConfig->wzcConfig.Ssid.Ssid, "") != 0) 
    {
        nLenSSID = MultiByteToWideChar(
                                      CP_ACP,
                                      0,
                                      (LPCSTR)pConfig->wzcConfig.Ssid.Ssid,
                                      pConfig->wzcConfig.Ssid.SsidLength,
                                      wszSSID,
                                      ARRAYSIZE(wszSSID));
        if (nLenSSID == 0) 
            dwErr = GetLastError();
    }

    if (dwErr == ERROR_SUCCESS) 
    {
        LVITEM 					lvi = {0};
        UINT  				 	nImgIdx;
		UINT					i;
        
        // put the null terminator
        wszSSID[nLenSSID]=L'\0';
        for (i=0;i<nLenSSID;i++) {
            if (wszSSID[i] != L' ')
                break;
        }

        // get the item's image index
        if (pConfig->wzcConfig.InfrastructureMode == Ndis802_11Infrastructure)
        {
            nImgIdx = (pConfig->dwFlags & WZC_DESCR_ACTIVE) ? WZCIMG_INFRA_ACTIVE :
                        ((pConfig->dwFlags & WZC_DESCR_VISIBLE) ? WZCIMG_INFRA_AIRING : WZCIMG_INFRA_SILENT);
        }
        else
        {
            nImgIdx = (pConfig->dwFlags & WZC_DESCR_ACTIVE) ? WZCIMG_ADHOC_ACTIVE :
                        ((pConfig->dwFlags & WZC_DESCR_VISIBLE) ? WZCIMG_ADHOC_AIRING : WZCIMG_ADHOC_SILENT);
        }
        memset(&lvi,0,sizeof(lvi));
        lvi.iSubItem = 0;
        lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
        lvi.pszText = wszSSID;
        lvi.iImage = nImgIdx;
        lvi.lParam = (LPARAM)pConfig;
		lvi.iItem = nPos;
		
		pConfig->nListIndex = ListView_InsertItem(hwndLV, &lvi);
		
        ASSERT(pConfig->nListIndex != -1);
        DEBUGMSG(ZONE_NETWORK,(TEXT("AddConfigToListView: Added %s idx %d hwnd0x%X\r\n"),wszSSID,pConfig->nListIndex,hwndLV));
    }

    return dwErr;
}


AddUniqueConfig(
    PWLAN_INFO          pWLanInfo,
    DWORD               dwOpFlags,
    DWORD               dwEntryFlags,
    PWZC_WLAN_CONFIG    pwzcConfig,
    WLAN_CONFIG_LISTENT **ppNewNode)
{
    DWORD       			dwErr    = ERROR_SUCCESS;
    PWLAN_CONFIG_LISTENT  	pHdList;

	WZCQCFG_LOCK;

	pHdList = (dwEntryFlags & WZC_DESCR_PREFRD) ? pWLanInfo->pHdPList : pWLanInfo->pHdVList;

    // if the list is currently empty, create the first entry as the head of the list
    if (pHdList == NULL)
    {
        pHdList = AllocAndInitConfigListEnt(dwEntryFlags,pwzcConfig);

        if (pHdList == NULL)
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

        // if the caller wants, return the pointer to the newly created object
        if (ppNewNode != NULL)
            *ppNewNode = pHdList;
    }
    else
    {
        // else the list already contains at least one element
        PWLAN_CONFIG_LISTENT pCrt;

        // scan the list (keep in mind it is ordered descendingly on IM)
        pCrt = pHdList;
        do
        {
            // if found an identical entry (same SSID and same InfraMode)
            // signal the DUPLICATE_TAG error
            if (ConfigMatch(&pCrt->wzcConfig, pwzcConfig))
            {
                // merge the flags first
                pCrt->dwFlags |= dwEntryFlags;

                // if requested, copy the "authentication" value
                // hidden in "authentication mode" attribute of the WZC_WLAN_CONFIG structure
                if (dwOpFlags & WZCADD_OVERWRITE)
                {
                    //NWB_SET_AUTHMODE(&(pCrt->m_wzcConfig), NWB_GET_AUTHMODE(pwzcConfig));
                    pCrt->wzcConfig.AuthenticationMode = pwzcConfig->AuthenticationMode;
                    pCrt->wzcConfig.Privacy = pwzcConfig->Privacy;

                    // If the HIGROUP is specified let's move the config to the top
	                if (dwOpFlags & WZCADD_HIGROUP)
	                {
						if (pCrt != pHdList)
						{
							pCrt->pPrev->pNext = pCrt->pNext;
							pCrt->pNext->pPrev = pCrt->pPrev;
							pCrt->pNext = pHdList;
							pCrt->pPrev = pHdList->pPrev;
							pHdList->pPrev->pNext = pCrt;
							pHdList->pPrev = pCrt;
							pHdList = pCrt;
						}
	                }
                }

                // if the caller wants, return the pointer to the matching entry
                if (ppNewNode != NULL)
                    *ppNewNode = pCrt;

                // signal there is already a matching config
                dwErr = ERROR_DUPLICATE_TAG;
            }
            pCrt = pCrt->pNext;
        } while (dwErr == ERROR_SUCCESS && pCrt != pHdList);

        // if dwErr is unchanged, this means a new node has to be added ahead of pCrt node
        if (dwErr == ERROR_SUCCESS)
        {
            // create the new config and insert it ahead of this node.
            PWLAN_CONFIG_LISTENT pNewConfig;

            pNewConfig = AllocAndInitConfigListEnt(dwEntryFlags, pwzcConfig);
            if (pNewConfig != NULL)
            {
                // Insert at head if requested
                if (dwOpFlags & WZCADD_HIGROUP)
                    pCrt = pHdList;

                pNewConfig->pPrev = pCrt->pPrev;
                pNewConfig->pNext = pCrt;
                pCrt->pPrev->pNext = pNewConfig;
                pCrt->pPrev = pNewConfig;

                if (dwOpFlags & WZCADD_HIGROUP)
                {
                    pHdList = pNewConfig;
                }
            }
            else
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
            }
            // if the caller wants, return the pointer to the newly created object
            if (ppNewNode != NULL)
                *ppNewNode = pNewConfig;
        }
    }

    if (dwEntryFlags & WZC_DESCR_PREFRD)
    {
        pWLanInfo->pHdPList = pHdList;
    }
    else
    {
        pWLanInfo->pHdVList = pHdList;
    }

	WZCQCFG_UNLOCK;

    return dwErr;
}

//+---------------------------------------------------------------------
// FillVisibleList - fills in the configs from the WZC_802_11_CONFIG_LIST object
// into the list of visible configs
DWORD
FillVisibleList(PWLAN_INFO pWLanInfo, PWZC_802_11_CONFIG_LIST pwzcVList)
{
    DWORD   dwErr = ERROR_SUCCESS;
    UINT    i;

	WZCQCFG_LOCK;

	DEBUGMSG (ZONE_NETWORK,(TEXT(" FillVisibleList: %u entries\r\n"),pwzcVList ? pwzcVList->NumberOfItems:0));

    // cleanup whatever we might already have in the visible list
    if (pWLanInfo->pHdVList != NULL)
    {
        while (pWLanInfo->pHdVList->pNext != pWLanInfo->pHdVList)
        {
            FreeConfigListEnt(pWLanInfo->pHdVList->pNext);
        }
        FreeConfigListEnt(pWLanInfo->pHdVList);
        pWLanInfo->pHdVList = NULL;
    }

    if (pwzcVList != NULL)
    {
        for (i = 0; i < pwzcVList->NumberOfItems; i++)
        {

#ifdef DEBUG
            {
                TCHAR szSSID[33];
                PWZC_WLAN_CONFIG    pwzcVConfig = &(pwzcVList->Config[i]);
                if (! pwzcVConfig->Ssid.SsidLength ||
                    (mbstowcs(szSSID,pwzcVConfig->Ssid.Ssid,pwzcVConfig->Ssid.SsidLength) <= 0)) {
                    _tcscpy(szSSID,TEXT("<none>"));
                }
                szSSID[pwzcVConfig->Ssid.SsidLength]=0;
                DEBUGMSG (ZONE_NETWORK,(TEXT("Preferred SSID: <%s>\r\n"),szSSID));
            }
#endif
            dwErr = AddUniqueConfig(
                        pWLanInfo,
                        0,                  // no op flags
                        WZC_DESCR_VISIBLE,  // this is a visible entry
                        &(pwzcVList->Config[i]),
                        NULL);

            // reset the error if config was just duplicated
            if (dwErr == ERROR_DUPLICATE_TAG)
                dwErr = ERROR_SUCCESS;
        }
    }

	WZCQCFG_UNLOCK;

    return dwErr;
}


//+---------------------------------------------------------------------
// FillPreferredList - fills in the configs from the WZC_802_11_CONFIG_LIST object
// into the list of preferred configs
DWORD
FillPreferredList(PWLAN_INFO pWLanInfo,PWZC_802_11_CONFIG_LIST pwzcPList)
{
    DWORD   dwErr = ERROR_SUCCESS;
    UINT    i;

	WZCQCFG_LOCK;

    DEBUGMSG (ZONE_NETWORK,(TEXT(" FillPreferredList: %u entries\r\n"),pwzcPList ? pwzcPList->NumberOfItems:0));

    // cleanup whatever we might already have in the preferred list
    if (pWLanInfo->pHdPList != NULL)
    {
        while (pWLanInfo->pHdPList ->pNext != pWLanInfo->pHdPList)
        {
            FreeConfigListEnt(pWLanInfo->pHdPList->pNext);
        }
        FreeConfigListEnt(pWLanInfo->pHdPList);
        pWLanInfo->pHdPList = NULL;
    }

    if (pwzcPList != NULL)
    {
        for (i = 0; i < pwzcPList->NumberOfItems; i++)
        {
            PWZC_WLAN_CONFIG    pwzcPConfig = &(pwzcPList->Config[i]);
            DWORD               dwFlags = WZC_DESCR_PREFRD;

            // check whether this preferred is also visible and adjust dwFlags if so
            if (IsConfigInList(pWLanInfo->pHdVList, pwzcPConfig,NULL))
                dwFlags |= WZC_DESCR_VISIBLE;
#ifdef DEBUG
            {
                TCHAR szSSID[33];
                if (! pwzcPConfig->Ssid.SsidLength ||
                    (mbstowcs(szSSID,pwzcPConfig->Ssid.Ssid,pwzcPConfig->Ssid.SsidLength) <= 0)) {
                    _tcscpy(szSSID,TEXT("<none>"));
                }
                szSSID[pwzcPConfig->Ssid.SsidLength]=0;
                DEBUGMSG (ZONE_NETWORK,(TEXT("Preferred SSID: <%s>\r\n"),szSSID));
            }
#endif

            dwErr = AddUniqueConfig(
                        pWLanInfo,
                        WZCADD_OVERWRITE,   // preferred entries cause info to be overwritten
                        dwFlags,
                        pwzcPConfig,
                        NULL);

            // reset the error if config was just duplicated
            if (dwErr == ERROR_DUPLICATE_TAG)
                dwErr = ERROR_SUCCESS;
        }
    }

	WZCQCFG_UNLOCK;

    return dwErr;
}

//+---------------------------------------------------------------------------
// Fill in the current configuration settings for this adapter
DWORD
FillCurrentConfig(PWLAN_INFO pWLanInfo, PINTF_ENTRY_EX pIntf)
{
    DWORD           dwErr = ERROR_SUCCESS;
    WZC_WLAN_CONFIG wzcCurrent = {0};
    WLAN_CONFIG_LISTENT  *pConfig = NULL;

	WZCQCFG_LOCK;

    wzcCurrent.InfrastructureMode = (NDIS_802_11_NETWORK_INFRASTRUCTURE)pIntf->nInfraMode;
    if (pIntf->rdSSID.dwDataLen > sizeof(wzcCurrent.Ssid.Ssid)) {
        ASSERT(0); 
        wzcCurrent.Ssid.SsidLength = sizeof(wzcCurrent.Ssid.Ssid);
    } else {
        wzcCurrent.Ssid.SsidLength = pIntf->rdSSID.dwDataLen;
    }
    CopyMemory(wzcCurrent.Ssid.Ssid, pIntf->rdSSID.pData, wzcCurrent.Ssid.SsidLength);
    // another bit of a hack. Code in the authentication mode for this adapter in the highest
    // of the two reserved bits from WZC_WLAN_CONFIG
    //NWB_SET_AUTHMODE(&wzcCurrent, pIntf->nAuthMode);
    wzcCurrent.AuthenticationMode = (NDIS_802_11_AUTHENTICATION_MODE)pIntf->nAuthMode;
    // set the privacy field based on the adapter's WEP status.
    wzcCurrent.Privacy = pIntf->nWepStatus;

    if (IsConfigInList(pWLanInfo->pHdVList, &wzcCurrent, &pConfig))
        pConfig->dwFlags |= WZC_DESCR_ACTIVE;

    if (IsConfigInList(pWLanInfo->pHdPList, &wzcCurrent, &pConfig))
        pConfig->dwFlags |= WZC_DESCR_ACTIVE;

	WZCQCFG_UNLOCK;

    return dwErr;
}

// ------------------------------------------------------------------
// Fill the preferred list view with configs
// ------------------------------------------------------------------

DWORD
FillPreferredLV(PWLAN_INFO pWLanInfo)
{
    DWORD       			dwErr = ERROR_SUCCESS;
    HWND					hwndLV;
    PWLAN_CONFIG_LISTENT  	pActive = NULL;
    PWLAN_CONFIG_LISTENT	pHdList;

	WZCQCFG_LOCK;

	// Preferred list
	pHdList = pWLanInfo->pHdPList;
	hwndLV	= pWLanInfo->hwndPLV;

	ListView_DeleteAllItems(hwndLV);

    if (pHdList != NULL)
    {
        PWLAN_CONFIG_LISTENT  	pCrt;
        INT						i;
        
        pCrt = pHdList;
        i = 0;            
        do
        {
            // add in the list all the entries if AutoMode or we're filling the
            // visible list.
            // Otherwise (!AutoMode & Preferred list) put in just the entries for
            // the corresponding infrastructure mode
            if ((pWLanInfo->dwCtlFlags & INTFCTL_CM_MASK) == Ndis802_11AutoUnknown ||
                (pWLanInfo->dwCtlFlags & INTFCTL_CM_MASK) == (DWORD)pCrt->wzcConfig.InfrastructureMode)
            {
            	pCrt->nListIndex = i;
                AddConfigToListView(pCrt,hwndLV, i++);
            }
            else
            {
                pCrt->nListIndex = -1;
            }
            pCrt = pCrt->pNext;
        } while (pCrt != pHdList);
    }

	WZCQCFG_UNLOCK;

    return dwErr;
}


// -------------------------------------------------------------------
// Proc to refresh dialog buttons.
// -------------------------------------------------------------------

DWORD 
RefreshButtons(HWND hDlg, PWLAN_INFO pWLanInfo)
{
	int		iSelected;
	int		iCount;
	BOOL	fWZCEnabled;

	WZCQCFG_LOCK;

	//
	// Get the number of items in the list and the current selection.  From this, we can
	// determine the state of the Up/Down buttons.
	//
	
	iSelected = ListView_GetNextItem(pWLanInfo->hwndPLV, -1, LVNI_SELECTED);
	iCount = ListView_GetItemCount(pWLanInfo->hwndPLV);

	fWZCEnabled = (IsDlgButtonChecked(hDlg, IDC_WZC_CHK_ENABLEWZC) == BST_CHECKED);

	EnableWindow(pWLanInfo->hbtnRem, fWZCEnabled && (iSelected >= 0));
	EnableWindow(pWLanInfo->hbtnUp, fWZCEnabled && (iSelected > 0));
	EnableWindow(pWLanInfo->hbtnDown, fWZCEnabled && (iSelected >= 0) && (iSelected < iCount - 1));

	WZCQCFG_UNLOCK;

	return ERROR_SUCCESS;
}


DWORD
SwapConfigsInListView(PWLAN_INFO pWLanInfo, INT nIdx1, INT nIdx2, WLAN_CONFIG_LISTENT **ppConfig1, WLAN_CONFIG_LISTENT **ppConfig2)
{
    DWORD   dwErr = ERROR_SUCCESS;
    LVITEM  lvi1 = {0};
    LVITEM  lvi2 = {0};
    WCHAR   wszSSID1[sizeof(WCHAR)*33];
    WCHAR   wszSSID2[sizeof(WCHAR)*33];

	WZCQCFG_LOCK;

    // since we take all what is known about an item this includes
    // images indices and selection state
    // get the first item
    lvi1.iItem = nIdx1;
    lvi1.iSubItem = 0;
    lvi1.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE | LVIF_PARAM;
    lvi1.stateMask = (UINT)-1;
    lvi1.pszText = wszSSID1;
    lvi1.cchTextMax = ARRAYSIZE(wszSSID1);
    if (!ListView_GetItem(pWLanInfo->hwndPLV, &lvi1))
    {
        dwErr = ERROR_GEN_FAILURE;
        goto exit;
    }
    *ppConfig1 = (WLAN_CONFIG_LISTENT*)lvi1.lParam;

    // get the second item
    lvi2.iItem = nIdx2;
    lvi2.iSubItem = 0;
    lvi2.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE | LVIF_PARAM;
    lvi2.stateMask = (UINT)-1;
    lvi2.pszText = wszSSID2;
    lvi2.cchTextMax = ARRAYSIZE(wszSSID2);
    if (!ListView_GetItem(pWLanInfo->hwndPLV, &lvi2))
    {
        dwErr = ERROR_GEN_FAILURE;
        goto exit;
    }
    *ppConfig2 = (WLAN_CONFIG_LISTENT*)lvi2.lParam;

    // swap the indices and reset the items at their new positions
    lvi1.iItem = nIdx2;
    lvi2.iItem = nIdx1;
    if (!ListView_SetItem(pWLanInfo->hwndPLV, &lvi1) ||
        !ListView_SetItem(pWLanInfo->hwndPLV, &lvi2))
    {
        dwErr = ERROR_GEN_FAILURE;
        goto exit;
    }
    // if everything went fine, swap the indices in the objects
    (*ppConfig1)->nListIndex = nIdx2;
    (*ppConfig2)->nListIndex = nIdx1;
    // make visible the selected entry
    ListView_EnsureVisible(pWLanInfo->hwndPLV, nIdx1, FALSE);

exit:
	WZCQCFG_UNLOCK;
	
    return dwErr;
}


DWORD
SavePreferredConfigs(PWLAN_INFO pWLanInfo, PINTF_ENTRY_EX pIntf)
{
    DWORD       dwErr = ERROR_SUCCESS;
    WLAN_CONFIG_LISTENT  *pCrt = NULL;
    UINT        nPrefrd = 0;

	WZCQCFG_LOCK;

    if (pWLanInfo->pHdPList != NULL)
    {
        // count first the number of preferred entries in the list
        pCrt = pWLanInfo->pHdPList;
        do
        {
            nPrefrd++;
            pCrt = pCrt->pNext;
        } while(pCrt != pWLanInfo->pHdPList);
    }

    if (nPrefrd > 0)
    {
        PWZC_802_11_CONFIG_LIST pwzcPrefrdList;
        UINT                    nwzcPrefrdSize;

        nwzcPrefrdSize = sizeof(WZC_802_11_CONFIG_LIST)+ (nPrefrd-1)*sizeof(WZC_WLAN_CONFIG);

        // allocate as much memory as needed for storing all the preferred SSIDs
        pwzcPrefrdList = (PWZC_802_11_CONFIG_LIST)LocalAlloc(LPTR,nwzcPrefrdSize);
        if (pwzcPrefrdList == NULL)
        {
            dwErr = GetLastError();
        }
        else
        {
            pwzcPrefrdList->NumberOfItems = 0; 
            // we have now all we need - start copying the preferred 
            pCrt = pWLanInfo->pHdPList;
            do
            {
                PWZC_WLAN_CONFIG    pPrefrdConfig;

                pPrefrdConfig = &(pwzcPrefrdList->Config[pwzcPrefrdList->NumberOfItems++]);
                CopyMemory(pPrefrdConfig, &pCrt->wzcConfig, sizeof(WZC_WLAN_CONFIG));
                pCrt = pCrt->pNext;
            } while(pwzcPrefrdList->NumberOfItems < nPrefrd && pCrt != pWLanInfo->pHdPList);

            pIntf->rdStSSIDList.dwDataLen = nwzcPrefrdSize;
            pIntf->rdStSSIDList.pData = (LPBYTE)pwzcPrefrdList;
        }
    }
    else
    {
        pIntf->rdStSSIDList.dwDataLen = 0;
        pIntf->rdStSSIDList.pData = NULL;
    }

	WZCQCFG_UNLOCK;

    return dwErr;
}



// ---------------------------------------------------------------------------
// Function for the initializing the advanced dialog.
// ---------------------------------------------------------------------------

void OnInitWZCAdvancedDialog(HWND hDlg, PWLAN_INFO pWLanInfo, NDIS_802_11_NETWORK_INFRASTRUCTURE* pMode)
{
	WCHAR 								szAll[50];
	WCHAR 								szAPs[50];
	WCHAR 								szAdhoc[50];
	HIMAGELIST							hImgs;
	LVCOLUMN							lvc = {0};
	DWORD								dwStyle;
	RECT								rc;

	ASSERT(pMode && pWLanInfo);

	WZCQCFG_LOCK;

	// Get dialog controls
	pWLanInfo->hwndNetAccess	= GetDlgItem(hDlg, IDC_WZC_NETACCESS_COMBO);
	pWLanInfo->hwndPLV			= GetDlgItem(hDlg, IDC_WZC_PREF_LIST);
	pWLanInfo->hbtnUp			= GetDlgItem(hDlg, IDC_WZC_PREF_UP);
	pWLanInfo->hbtnDown			= GetDlgItem(hDlg, IDC_WZC_PREF_DOWN);
	pWLanInfo->hbtnRem			= GetDlgItem(hDlg, IDC_WZC_PREF_DELETE);
	
	pWLanInfo->dwCtlFlags = pWLanInfo->IntfEntryEx.dwCtlFlags;

	/*** Combo Box ***/

	// Load strings for combo box
	LoadString(v_hInst, IDS_WZCADV_ALLAVAIL, szAll, 50);
	LoadString(v_hInst, IDS_WZCADV_ACCESSPTS, szAPs, 50);
	LoadString(v_hInst, IDS_WZCADV_ADHOC, szAdhoc, 50);
	
	// Add strings to combo box
	ComboBox_AddString(pWLanInfo->hwndNetAccess, szAll);
	ComboBox_AddString(pWLanInfo->hwndNetAccess, szAPs);
	ComboBox_AddString(pWLanInfo->hwndNetAccess, szAdhoc);

	// Select correct item
    *pMode = (NDIS_802_11_NETWORK_INFRASTRUCTURE)(pWLanInfo->dwCtlFlags & INTFCTL_CM_MASK);
    switch(*pMode)
    {
    	case Ndis802_11IBSS:
            // Computer-to-computer
            SendMessage(pWLanInfo->hwndNetAccess, CB_SETCURSEL, 2, 0);
            break;
        case Ndis802_11Infrastructure:
            // infrastructure (access point) network
            SendMessage(pWLanInfo->hwndNetAccess, CB_SETCURSEL, 1, 0);
            break;
        case Ndis802_11AutoUnknown:
        default:
            // Any network (access point preferred)
            SendMessage(pWLanInfo->hwndNetAccess, CB_SETCURSEL, 0, 0);
    }

	/*** List View ***/

	// Set Image List
	dwStyle = GetWindowLong(pWLanInfo->hwndPLV, GWL_STYLE);
    SetWindowLong(pWLanInfo->hwndPLV, GWL_STYLE, (dwStyle | LVS_SHAREIMAGELISTS));

    hImgs = ImageList_LoadBitmap(v_hInst,
                          MAKEINTRESOURCE(IDB_WZCSTATE),
                          16,0,CLR_NONE);
    ListView_SetImageList(pWLanInfo->hwndPLV, hImgs, LVSIL_SMALL);

	// Set other configs
    lvc.mask = LVCF_FMT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;

    GetClientRect(pWLanInfo->hwndPLV, &rc);
    lvc.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL);
    //lvc.cx = rc.right;
    ListView_InsertColumn(pWLanInfo->hwndPLV, 0, &lvc);

    ListView_SetExtendedListViewStyleEx(pWLanInfo->hwndPLV, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

    EnableWindow(pWLanInfo->hwndPLV, TRUE);

	// Fill the preferred list with configs
	FillPreferredLV(pWLanInfo);

	/*** Check boxes ***/

    SendDlgItemMessage(hDlg, IDC_WZC_CHK_FALLBACK, BM_SETCHECK, (pWLanInfo->dwCtlFlags & INTFCTL_FALLBACK) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_WZC_CHK_ENABLEWZC, BM_SETCHECK, (pWLanInfo->dwCtlFlags & INTFCTL_ENABLED) ? BST_CHECKED : BST_UNCHECKED, 0);
	
	/*** Other controls ***/

	RefreshButtons(hDlg, pWLanInfo);
	RefreshPreferredLV(pWLanInfo);

	// If Zero Config is disabled, disable all controls
	if(IsDlgButtonChecked(hDlg,IDC_WZC_CHK_ENABLEWZC) == BST_UNCHECKED)
	{
		EnableWindow(pWLanInfo->hwndPLV, FALSE);
		EnableWindow(pWLanInfo->hwndNetAccess, FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_WZC_CHK_FALLBACK), FALSE);
	}

	WZCQCFG_UNLOCK;
}


// ---------------------------------------------------------------------------
// Proc for swapping networks in the preferred list.
// ---------------------------------------------------------------------------

void MoveWZCNetwork(HWND hDlg, PWLAN_INFO pWLanInfo, BOOL fDirection)
{
	INT         iSelected;
	INT         iOther;
	WLAN_CONFIG_LISTENT  *pConfig, *pOther;

	// no matter what, get the selected item from the list
	iSelected = ListView_GetNextItem(pWLanInfo->hwndPLV, -1, LVNI_SELECTED);

	// since we are here it means there is another entry up/down side
	// with which the selected one needs to change place
	// first delete the entry from its current position in the list
	iOther = (!fDirection)? iSelected+1 : iSelected-1;

	WZCQCFG_LOCK;

	// swap first the visual elements (entries in the List View)
	// This returns the WLAN_CONFIG_LISTENT pointers with their indices already
	// adjusted
	SwapConfigsInListView(pWLanInfo,iSelected, iOther, &pConfig, &pOther);

	ASSERT(pOther && pConfig);

	// if need to go down one hop..
	if(!fDirection)
	{
		// swap positions in the list 
		// remove the entry from its current position
		pConfig->pNext->pPrev = pConfig->pPrev;
		pConfig->pPrev->pNext = pConfig->pNext;
		// and put it back down to its successor
		pConfig->pPrev = pOther;
		pConfig->pNext = pOther->pNext;
		pOther->pNext->pPrev = pConfig;
		pOther->pNext = pConfig;
		// fix the m_pHdPList if needed;
		if (pWLanInfo->pHdPList == pConfig)
			pWLanInfo->pHdPList = pOther;
	}
	// if need to go up one hop..
	else
	{
		// swap positions in the list
		// remove the entry from its current position
		pConfig->pPrev->pNext = pConfig->pNext;
		pConfig->pNext->pPrev = pConfig->pPrev;
		// and put it back in front of its predecessor
		pConfig->pNext = pOther;
		pConfig->pPrev = pOther->pPrev;
		pOther->pPrev->pNext = pConfig;
		pOther->pPrev = pConfig;
		// fix the pHdPList if needed
		if (pWLanInfo->pHdPList == pOther)
			pWLanInfo->pHdPList = pConfig;
	}


	// need to refresh the buttons such that the "Up"/"Down" buttons
	// get updated for the new position of the selection
	RefreshPreferredLV(pWLanInfo);
	RefreshButtons(hDlg, pWLanInfo);

	WZCQCFG_UNLOCK;
}


// ---------------------------------------------------------------------------
// Proc for the delete button action.
// ---------------------------------------------------------------------------

void OnWZCPrefDelete(HWND hDlg, PWLAN_INFO pWLanInfo)
{
	LVITEM      			lvi = {0};
	PWLAN_CONFIG_LISTENT	pConfig;
	PWLAN_CONFIG_LISTENT	pCrt;
	INT     				iSelected;

	WZCQCFG_LOCK;

	if(GetSelectedConfig(pWLanInfo->hwndPLV, &pConfig, &iSelected) == FALSE)
	{
		// If no config was selected, then abort
		goto exit;
	}

	// adjust the list indices for all the entries that follow
	// the selected one
	for (pCrt = pConfig->pNext; pCrt != pWLanInfo->pHdPList; pCrt = pCrt->pNext)
	{
		if (pCrt->nListIndex != -1)
		pCrt->nListIndex--;
	}
	
	// determine first which entry gets the selection
	// the selection moves down if there is any other entry down, or up otherwise
	pCrt = (pConfig->pNext == pWLanInfo->pHdPList) ? pConfig->pPrev : pConfig->pNext;

	// if after that the selection still points to the same object, it means
	// it is the only one in the list so the head and selection are set to NULL
	if (pCrt == pConfig)
	{
		pWLanInfo->pHdPList = pCrt = NULL;
	}
	// otherwise, if it is the head of the list which gets removed, the head
	// moves down to the next entry
	else if (pWLanInfo->pHdPList == pConfig)
	{
		pWLanInfo->pHdPList = pConfig->pNext;
	}

	// delete now the selected entry from the list
	ListView_DeleteItem(pWLanInfo->hwndPLV, iSelected);
	// and destroy its entry (free function takes care of list removal)
	FreeConfigListEnt(pConfig);

	// set the new selection if any
	if (pCrt != NULL)
	{
		ListView_SetItemState(pWLanInfo->hwndPLV, pCrt->nListIndex, LVIS_SELECTED, LVIS_SELECTED);
		ListView_EnsureVisible(pWLanInfo->hwndPLV, pCrt->nListIndex, FALSE);
	}

	// refresh the buttons' state
	RefreshButtons(hDlg, pWLanInfo);

exit:
	WZCQCFG_UNLOCK;
}

// ---------------------------------------------------------------------------
// Proc to handle ok button events
// ---------------------------------------------------------------------------

void OnWZCAdvancedOK(HWND hDlg, PWLAN_INFO pWLanInfo)
{
	WZCQCFG_LOCK;
	
	// Set the "fallback to visible networks" flag
    pWLanInfo->dwCtlFlags &= ~INTFCTL_FALLBACK;
    if (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_WZC_CHK_FALLBACK, BM_GETCHECK, 0, 0)) {
        pWLanInfo->dwCtlFlags |= INTFCTL_FALLBACK;
    }

    if(IsDlgButtonChecked(hDlg,IDC_WZC_CHK_ENABLEWZC) == BST_CHECKED)
    	pWLanInfo->dwCtlFlags |= INTFCTL_ENABLED;
    else
		pWLanInfo->dwCtlFlags &= ~INTFCTL_ENABLED;

	pWLanInfo->IntfEntryEx.dwCtlFlags = pWLanInfo->dwCtlFlags;
	
	SavePreferredConfigs(pWLanInfo, &pWLanInfo->IntfEntryEx);
	SetWirelessInfo(pWLanInfo);

	DestroyWindow(hDlg);

	WZCQCFG_UNLOCK;
}

// --------------------------------------------------------------------------
// Proc to handle enable/disable zero config
// --------------------------------------------------------------------------

void OnWZCEnableChk(HWND hDlg, PWLAN_INFO pWLanInfo)
{
	BOOL bEnable;
	
	WZCQCFG_LOCK;

	bEnable = (IsDlgButtonChecked(hDlg,IDC_WZC_CHK_ENABLEWZC) == BST_CHECKED);
	EnableWindow(pWLanInfo->hwndPLV, bEnable);
	EnableWindow(pWLanInfo->hwndNetAccess, bEnable);
	EnableWindow(GetDlgItem(hDlg, IDC_WZC_CHK_FALLBACK), bEnable);

	RefreshButtons(hDlg, pWLanInfo);

	WZCQCFG_UNLOCK;
}

// --------------------------------------------------------------------------
// Proc to handle right click on list view
// --------------------------------------------------------------------------

void OnAdvListRClick(HWND hDlg)
{
	HMENU hMenu = CreatePopupMenu();
	if(hMenu)
	{
		POINT 	p;

		AppendMenu(hMenu, 0, IDM_WZC_CONFIGURE, g_wszConfigure);
		AppendMenu(hMenu, 0, IDM_WZC_DELETE, g_wszDelete);

		GetCursorPos(&p);
		TrackPopupMenu(hMenu, TPM_LEFTALIGN, p.x, p.y, 0, hDlg, NULL);									
		DestroyMenu(hMenu);
	}
}

// -------------------------------------------------------------------------
// Proc to configure a network from advanced dialog
// -------------------------------------------------------------------------

void OnAdvConfigureNetwork(HWND hwndParent, PWLAN_INFO pWLanInfo)
{
	WLAN_CONFIG_LISTENT*	pConfig = NULL;
	WLAN_CFG_PROPS 			CfgProps;
	HWND					hDlg;
    BOOL                    fPortrait = (GetSystemMetrics(SM_CXSCREEN) < 480)? TRUE:FALSE;
    UINT                    nDlgId    = fPortrait? IDC_WZC_DLG_PROPS_N:IDC_WZC_DLG_PROPS_W;

	WZCQCFG_LOCK;
	
	InitializeCfgProps(&CfgProps, WZCDLG_PROPS_RWALL|WZCDLG_PROPS_NODUPS|WZCDLG_PROPS_DEFOK|WZCDLG_PROPS_CONF,nDlgId);

	GetSelectedConfig(pWLanInfo->hwndPLV, &pConfig, NULL);
	if (pConfig) 
	{	
		CopyMemory(&CfgProps.wzcConfig, &pConfig->wzcConfig, sizeof(WZC_WLAN_CONFIG));

		CfgProps.dwNicCapabilities = pWLanInfo->IntfEntryEx.dwCapabilities;
        CfgProps.bWPAAESCapable    = IsWPAAESCapable((PINTF_80211_CAPABILITY)pWLanInfo->IntfEntryEx.rdNicCapabilities.pData);            
		CfgProps.fDlgOk = FALSE;

		// Display config dialog
		hDlg = CreateDialogParam(v_hInst, 
	                              MAKEINTRESOURCE(nDlgId), 
	                              hwndParent, 
	                              NetworkWLanPropsDlgProc,
	                              (LPARAM)&CfgProps);
		if(hDlg)
	    {
	    	EnableWindow(hwndParent, FALSE);

	   		WZCQCFG_UNLOCK;
	    	WZCMiniMsgPump(hDlg);
	   		WZCQCFG_LOCK;

	   		// Since critical section was unlocked, we must check the window is still valid.
	    	if(IsWindow(hwndParent))
		    	EnableWindow(hwndParent, TRUE);
	    }
		
		if(CfgProps.fDlgOk)
		{
			// Save configs
			pConfig->wzcConfig = CfgProps.wzcConfig;
		}
	}
	
	WZCQCFG_UNLOCK;
}

// ---------------------------------------------------------------------------
// Dialog proc for the "advanced" button.
// ---------------------------------------------------------------------------

LRESULT CALLBACK 
NetworkWLanAdvancedDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static PWLAN_INFO s_pWLanInfo;
	static NDIS_802_11_NETWORK_INFRASTRUCTURE s_mode;
	
	BOOL 				fRetVal = TRUE;
	NMHDR*				header 	= NULL;
	
	switch(message)
	{
		case WM_INITDIALOG:
		{			
			// Get WLanInfo
			s_pWLanInfo = (PWLAN_INFO) lParam;
			if(!s_pWLanInfo)
			{
				fRetVal = FALSE;
				break;
			}

			OnInitWZCAdvancedDialog(hDlg, s_pWLanInfo, &s_mode);
			
			fRetVal = FALSE;
			break;
		}
		case WM_NOTIFY:
			
			header = (NMHDR*) lParam;
			switch(header->code)
			{
				case LVN_ITEMCHANGED:

					RefreshButtons(hDlg, s_pWLanInfo);

					fRetVal = FALSE;
					break;

				case NM_RCLICK:

					OnAdvListRClick(hDlg);
					
					fRetVal = FALSE;
					break;

				case NM_CLICK:
					
					if (GetKeyState(VK_MENU) < 0)
						OnAdvListRClick(hDlg);

					fRetVal = FALSE;
					break;

				case NM_DBLCLK:

					OnAdvConfigureNetwork(hDlg, s_pWLanInfo);
					
					fRetVal = FALSE;
					break;
			}

			fRetVal = FALSE;
			break;
			
		case WM_COMMAND:
			
			switch(LOWORD(wParam))
			{
				case IDM_WZC_CONFIGURE:

					OnAdvConfigureNetwork(hDlg, s_pWLanInfo);
					break;
					
				case IDOK:

					OnWZCAdvancedOK(hDlg, s_pWLanInfo);
					break;

				case IDCANCEL:

					DestroyWindow(hDlg);
					break;

				case IDC_WZC_CHK_ENABLEWZC:
				
					OnWZCEnableChk(hDlg, s_pWLanInfo);
					fRetVal = FALSE;
					break;

				case IDC_WZC_PREF_UP:
					
					MoveWZCNetwork(hDlg, s_pWLanInfo, TRUE);
					
					fRetVal = FALSE;
					break;
					
				case IDC_WZC_PREF_DOWN:

					MoveWZCNetwork(hDlg, s_pWLanInfo, FALSE);
					
					fRetVal = FALSE;
					break;
					
				case IDM_WZC_DELETE:	
				case IDC_WZC_PREF_DELETE:

					OnWZCPrefDelete(hDlg, s_pWLanInfo);
					
					fRetVal = FALSE;
					break;

				case IDC_WZC_NETACCESS_COMBO:
				{
					int iSelection;
					
					iSelection = SendMessage(s_pWLanInfo->hwndNetAccess, CB_GETCURSEL, 0, 0);
					ASSERT(iSelection != CB_ERR);

	                // See what type of network connectivity the user selected
	                if(iSelection == 2)
	                {
	                    // Computer-to-computer
	                    s_mode = Ndis802_11IBSS;
	                }
	                else if(iSelection == 1)
	                {
	                    // infrastructure (access point) network
	                    s_mode = Ndis802_11Infrastructure;
	                }
	                else if(iSelection == 0)\
	                {
	                    // Any network (access point preferred)
	                    s_mode = Ndis802_11AutoUnknown;
	                }

					s_pWLanInfo->dwCtlFlags &= ~INTFCTL_CM_MASK;
	                s_pWLanInfo->dwCtlFlags |= (((DWORD) s_mode) & INTFCTL_CM_MASK);
	                
					FillPreferredLV(s_pWLanInfo);
				}
					fRetVal = FALSE;
					break;					
			}

			fRetVal = FALSE;
			break;

		case WM_CLOSE:
			
			DestroyWindow(hDlg);
			fRetVal = FALSE;
			break;

		case WM_DESTROY:

			PostQuitMessage(0);
			fRetVal = FALSE;
			break;


		default:
			fRetVal = FALSE;
			break;
	}
	
	return fRetVal;
}



