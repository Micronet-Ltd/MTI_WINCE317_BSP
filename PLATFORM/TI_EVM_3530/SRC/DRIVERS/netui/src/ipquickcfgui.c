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

	IpQuickCfgUi.c

Abstract:

	This file contains the code for the Ip Config Property Page.

Environment:

	WinCE

*/


#include <winsock2.h>
#include <ws2tcpip.h>
#include <ntddip6.h>
#include "QuickConfigUI.h"



/*++

Routine Name: 

	NetDetailsDialogProc

Routine Description:

	Dialog Proc for the Network details dialog.  This function handles all the
	messages encountered on the dialog.

Arguments:

	hwndDlg:	HWND for the dialog
	uMsg:		Message encountered on the dialog
	wParam:		Additional data passed to the dialog
	lParam:		Additional data passed to the dialog

Return Value:

	BOOL (TRUE/FALSE)

--*/

BOOL CALLBACK NetDetailsDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL        fRetVal = FALSE;
    WPARAM      wCmdId;

    switch (uMsg) 
    {
	    case WM_INITDIALOG:

	        OnInitNetDetailsDialog(hwndDlg);
	        fRetVal = TRUE;
	        break;

	    case WM_CLOSE:

	        DestroyWindow(hwndDlg);
	        fRetVal = FALSE;
	        break;

		case WM_DESTROY:

			PostQuitMessage(0);
			fRetVal = FALSE;
			break;

	    case WM_COMMAND:

	        wCmdId  = LOWORD(wParam);
	        switch (wCmdId)
	        {
		        case IDCANCEL:
				case IDOK:

		            DestroyWindow(hwndDlg);
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

} // NetDetailsDialogProc

AddToListControl(HWND hList, int iIndex, LPWSTR szFirst, LPWSTR szSecond)
{
    LV_ITEM lvi = {0};
    lvi.mask = LVIF_PARAM;
    lvi.lParam = 0;

    lvi.iItem = iIndex;
    ListView_InsertItem(hList, &lvi);
    
    ListView_SetItemText(hList, 
                        iIndex, 
                        0, 
                        szFirst);

    ListView_SetItemText(hList,
                        iIndex,
                        1,
                        szSecond);
}


/*++

Routine Name: 

    OnInitNetDetailsDialog

Routine Description:

    Called when the Net Details dialog is being initialized.

Arguments:

    hwndDlg:	HWND of the dialog

--*/

void OnInitNetDetailsDialog(HWND hwndDlg)
{
    PIP_ADAPTER_INFO    pAdapterInfo = NULL, pAdapterInfoHead = NULL;
    PFIXED_INFO         pNetworkParams = NULL;
    PIP_ADDR_STRING pDnsServer = NULL;
    ULONG               ulBufferSize = 0;
    DWORD               dwRetVal;
    HRESULT             hr = S_OK;
    DWORD               i;
    WCHAR               szIpAddr[MAX_IP_FIELD_WIDTH];
    WCHAR               szGateway[MAX_IP_FIELD_WIDTH];
    WCHAR               szSubnetMsk[MAX_IP_FIELD_WIDTH];
    WCHAR               szDHCPServer[MAX_IP_FIELD_WIDTH];
    WCHAR               szPhysicalAddr[MAX_WZCPHYSADDR_STR];
    WCHAR               szLeaseObtained[MAX_WZCIPCFGMISC_STR];
    WCHAR               szLeaseExpires[MAX_WZCIPCFGMISC_STR];
    WCHAR               szDNS1[MAX_IP_FIELD_WIDTH];
    WCHAR               szDNS2[MAX_IP_FIELD_WIDTH];
    WCHAR               szWINS1[MAX_IP_FIELD_WIDTH];
    WCHAR               szWINS2[MAX_IP_FIELD_WIDTH];
    WCHAR*				pTime;
    FILETIME            LeaseObtained, LeaseObtainedLocal;
    SYSTEMTIME          SysLeaseObtained;
    FILETIME            LeaseExpires, LeaseExpiresLocal;
    SYSTEMTIME          SysLeaseExpires;
    LV_COLUMN           lvCol = {0};
    RECT                rect;
    int                 iParamColWidth = 0; 
    TCHAR               szProperty[MAX_WZCIPCFGMISC_STR];
    int                 iIndex = 0;
    HWND                hList = GetDlgItem(hwndDlg, IDC_LIST_IPCFG);

    // Get IP adapter info
    hr = GetIPAdapterInfo(hwndDlg, &pAdapterInfoHead, &pAdapterInfo);
    if (FAILED(hr)) {
        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Could not get the adapter info")));
        goto exit;
    }

    // Get IP data from iphelper

    wsprintf(szPhysicalAddr, TEXT("%02X"), pAdapterInfo->Address[0]);
    for (i=1;i<pAdapterInfo->AddressLength;i++) {
        wsprintf(szPhysicalAddr, TEXT("%s %02X"), szPhysicalAddr, pAdapterInfo->Address[i]);
    }

    mbstowcs(szIpAddr, pAdapterInfo->IpAddressList.IpAddress.String, strlen(pAdapterInfo->IpAddressList.IpAddress.String)+1);
    mbstowcs(szGateway, pAdapterInfo->GatewayList.IpAddress.String, strlen(pAdapterInfo->GatewayList.IpAddress.String)+1);
    mbstowcs(szSubnetMsk, pAdapterInfo->IpAddressList.IpMask.String, strlen(pAdapterInfo->IpAddressList.IpMask.String)+1);
    if (pAdapterInfo->DhcpEnabled) {
        mbstowcs(szDHCPServer, pAdapterInfo->DhcpServer.IpAddress.String, strlen(pAdapterInfo->DhcpServer.IpAddress.String)+1);
    } else {
        szDHCPServer[0] = 0;
    }
    mbstowcs(szWINS1, pAdapterInfo->PrimaryWinsServer.IpAddress.String, strlen(pAdapterInfo->PrimaryWinsServer.IpAddress.String)+1);
    mbstowcs(szWINS2, pAdapterInfo->SecondaryWinsServer.IpAddress.String, strlen(pAdapterInfo->SecondaryWinsServer.IpAddress.String)+1);

    if (pAdapterInfo->DhcpEnabled) {
        // Get system time for lease obtained
        TimeToFileTime(pAdapterInfo->LeaseObtained, &LeaseObtained);
        FileTimeToLocalFileTime(&LeaseObtained, &LeaseObtainedLocal);    
        FileTimeToSystemTime(&LeaseObtainedLocal, &SysLeaseObtained);
        
        // Format string for lease obtained
        GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &SysLeaseObtained, NULL, szLeaseObtained, MAX_WZCIPCFGMISC_STR);
        wcscat(szLeaseObtained, TEXT(" "));
        pTime = szLeaseObtained + wcslen(szLeaseObtained); // Advance pointer to append time
        GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOTIMEMARKER, &SysLeaseObtained, NULL, pTime, (MAX_WZCIPCFGMISC_STR - wcslen(pTime)));
        
        // Get system time for lease expiry
        TimeToFileTime(pAdapterInfo->LeaseExpires, &LeaseExpires);
        FileTimeToLocalFileTime(&LeaseExpires, &LeaseExpiresLocal);    
        FileTimeToSystemTime(&LeaseExpiresLocal, &SysLeaseExpires);
        
        // Format string for lease expiry
        GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &SysLeaseExpires, NULL, szLeaseExpires, MAX_WZCIPCFGMISC_STR);
        wcscat(szLeaseExpires, TEXT(" "));
        pTime = szLeaseExpires + wcslen(szLeaseExpires); // Advance pointer to append time
        GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOTIMEMARKER, &SysLeaseExpires, NULL, pTime, (MAX_WZCIPCFGMISC_STR - wcslen(pTime)));
    } else {
        szLeaseObtained[0] = 0;
        szLeaseExpires[0] = 0;
    }

    // Get Network params struct for DNS servers
    dwRetVal = GetNetworkParams(pNetworkParams, &ulBufferSize);
    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        pNetworkParams = (PFIXED_INFO)malloc(ulBufferSize);
        if (pNetworkParams == NULL) {
            DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Could not allocate memory for network params")));
            goto exit;
        }

        dwRetVal = GetNetworkParams(pNetworkParams, &ulBufferSize);
        if (dwRetVal != ERROR_SUCCESS) {
            DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Could not get network params")));
            goto exit;
        }
    } else if (dwRetVal != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Could not get network params")));
        goto exit;
    }

    if (pNetworkParams == NULL) {
        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Could not get network params")));
        goto exit;
    }

    // Get DNS Servers
    pDnsServer = &(pNetworkParams->DnsServerList);
    mbstowcs(szDNS1, pDnsServer->IpAddress.String, strlen(pDnsServer->IpAddress.String)+1);
    pDnsServer = pDnsServer->Next;
    if (pDnsServer != NULL) {
        mbstowcs(szDNS2, pDnsServer->IpAddress.String, strlen(pDnsServer->IpAddress.String)+1); 
    } else {
        szDNS2[0] = 0;
    }

    // Update UI
    mbstowcs(szProperty,pAdapterInfo->AdapterName,strlen(pAdapterInfo->AdapterName)+1);
    SetWindowText(hwndDlg,szProperty);

    GetClientRect(hList, &rect);
    iParamColWidth = (rect.right/2);
    
    lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT ;
    lvCol.fmt = LVCFMT_LEFT;   // left-align column
    lvCol.cx = iParamColWidth;

    if (LoadString(v_hInst, IDS_IPCFG_PRAMETER,szProperty,sizeof(szProperty)/sizeof(szProperty[0]))) {
        lvCol.pszText = szProperty;
        iIndex = ListView_InsertColumn(hList, iIndex, &lvCol);
        iIndex++;
    }

    if (LoadString(v_hInst, IDS_IPCFG_VALUE,szProperty,sizeof(szProperty)/sizeof(szProperty[0]))) {
        lvCol.cx      = rect.right - iParamColWidth;
        lvCol.pszText = szProperty;
        iIndex = ListView_InsertColumn(hList, iIndex, &lvCol);
    }

    ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT);

    iIndex=0;
    if (LoadString(v_hInst,IDS_IPCFG_PH_ADDR,szProperty,sizeof(szProperty)/sizeof(szProperty[0])))
        AddToListControl(hList, iIndex++, szProperty, szPhysicalAddr);
    if (LoadString(v_hInst,IDS_IPCFG_IPADDR,szProperty,sizeof(szProperty)/sizeof(szProperty[0])))
        AddToListControl(hList, iIndex++, szProperty, szIpAddr);
    if (LoadString(v_hInst,IDS_IPCFG_SUBNET,szProperty,sizeof(szProperty)/sizeof(szProperty[0])))
        AddToListControl(hList, iIndex++, szProperty, szSubnetMsk);
    if (LoadString(v_hInst,IDS_IPCFG_DEFGW,szProperty,sizeof(szProperty)/sizeof(szProperty[0])))
        AddToListControl(hList, iIndex++, szProperty, szGateway);
    if (LoadString(v_hInst,IDS_IPCFG_DHCP,szProperty,sizeof(szProperty)/sizeof(szProperty[0])))
        AddToListControl(hList, iIndex++, szProperty, szDHCPServer);
    if (LoadString(v_hInst,IDS_IPCFG_LEASE_OBT,szProperty,sizeof(szProperty)/sizeof(szProperty[0])))
        AddToListControl(hList, iIndex++, szProperty, szLeaseObtained);
    if (LoadString(v_hInst,IDS_IPCFG_LEASE_EXP,szProperty,sizeof(szProperty)/sizeof(szProperty[0])))
        AddToListControl(hList, iIndex++, szProperty, szLeaseExpires);
    if (LoadString(v_hInst,IDS_IPCFG_DNSSERVERS,szProperty,sizeof(szProperty)/sizeof(szProperty[0]))) {
        AddToListControl(hList, iIndex++, szProperty, szDNS1);
        AddToListControl(hList, iIndex++, L"", szDNS2);
    }
    if (LoadString(v_hInst,IDS_IPCFG_WINSSERVERS,szProperty,sizeof(szProperty)/sizeof(szProperty[0]))) {
        AddToListControl(hList, iIndex++, szProperty, szWINS1);
        AddToListControl(hList, iIndex++, L"", szWINS2);
    }

    exit:
    if (pAdapterInfoHead) {
        free(pAdapterInfoHead);
    }
    if (pNetworkParams) {
        free(pNetworkParams);
    }

    return; 

} // OnInitNetDetailsDialog

HRESULT 
GetAdapterNameW(HWND hwndDlg, WCHAR *szNameBuf, DWORD cbNameBufLen)
{
    HRESULT hr;
    SystrayIcon *pSysIcon = NULL;

    // Get the systray icon for the given window
    hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
    if (FAILED(hr) || !pSysIcon) {
        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Error getting systray icon struct")));
        return E_FAIL;
    }
    wcsncpy(szNameBuf,pSysIcon->ptcDeviceName,cbNameBufLen/sizeof(WCHAR));

    return S_OK;
}
HRESULT
GetAdapterNameA(HWND hwndDlg, char *szNameBuf, DWORD cbNameBufLen)
{
    WCHAR wszNameBuf[MAX_ADAPTERNAME_STR];

    if (GetAdapterNameW(hwndDlg,wszNameBuf,sizeof(wszNameBuf)) == S_OK) {
        if (wcstombs(szNameBuf,wszNameBuf, cbNameBufLen) == cbNameBufLen)
            szNameBuf[cbNameBufLen] = '\0';
        return S_OK;
    }
    return E_FAIL;
}

/*++

Routine Name: 

    IPQuickCfgDialogProc

Routine Description:

    Dialog Proc for the IP Info property page.  This function handles all the
    messages encountered on the property page.

Arguments:

    hwndDlg:	HWND for the property page
    uMsg:		Message encountered on the property page
    wParam:		Additional data passed to the dialog
    lParam:		Additional data passed to the dialog

Return Value:

    BOOL (TRUE/FALSE)

--*/

BOOL CALLBACK IPQuickCfgDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL        fRetVal = FALSE;
    HRESULT     hr = S_OK;
    WPARAM      wCmdId;
    SystrayIcon *pSysIcon = NULL;
    TCHAR       szRenew[STRING_BUFFER_MAX];
    HWND		hPropSheet;

    switch (uMsg) {
    case WM_INITDIALOG:

        SetTimer(hwndDlg, 1, IPQC_RFSH_TIMEOUT, NULL);
        fRetVal = TRUE;
        break;

    case WM_TIMER:
        RefreshIPCfgDialog(hwndDlg, FALSE);
        fRetVal = FALSE;
        break;

    case WM_COMMAND:

        wCmdId  = LOWORD(wParam);
        switch (wCmdId) {
        case IDC_WZCQCFG_DETAILS:

            // Get the systray icon struct
            DEBUGMSG(ZONE_MISC, (TEXT("NetUIQC - Details button event")));
            
            WZCQCFG_LOCK;
            hr = GetDeviceSystrayIcon(GetParent(hwndDlg), &pSysIcon);
            if (SUCCEEDED(hr) && pSysIcon) {
                BOOL fPortrait = (GetSystemMetrics(SM_CXSCREEN) < 480)? TRUE:FALSE;
            	HWND hDlg;
            	
            	
                hDlg = CreateDialog(v_hInst, MAKEINTRESOURCE(fPortrait?IDD_NETDETAILS_N:IDD_NETDETAILS_W), pSysIcon->hPropSheet, NetDetailsDialogProc);
                if(hDlg)
                {
                	hPropSheet = pSysIcon->hPropSheet;
                	
                	EnableWindow(hPropSheet, FALSE);

                	WZCQCFG_UNLOCK;
                	WZCMiniMsgPump(hDlg);
                	WZCQCFG_LOCK;

					// Since critical section was unlocked, we must check the window is still valid.
                	if(IsWindow(hPropSheet))
	                	EnableWindow(hPropSheet, TRUE);
                }
                
            }
            WZCQCFG_UNLOCK;
            
            break;

        case IDC_WZCQCFG_RENEW:

            // Renew button pushed
            DEBUGMSG(ZONE_MISC, (TEXT("NetUIQC - Renew button event")));
            hr = RefreshIPCfgDialog(hwndDlg, TRUE);
            if (FAILED(hr)) {
                LoadString(v_hInst, IDS_QC_IPRENEWERR, szRenew, sizeof(szRenew)/sizeof(szRenew[0]));
                NetMsgBox(hwndDlg, NMB_FL_OK | NMB_FL_EXCLAIM, szRenew);
            }
            break;

        default:
            break;
        }

        fRetVal = 0;
        break;

    default:
        fRetVal = FALSE;
    }

    return fRetVal;

} // IPQuickCfgDialogProc



/*++

Routine Name: 

    GetIPAdapterInfo

Routine Description:

    Get the IP Adapter info given the adapter name

Arguments:

    hwndDlg:			HWND of the Property Sheet or main dialog
    ppAdapterInfo:		Adapter info struct
    pszAdapterName:		Adapter name

Return Value:

    HRESULT to indicate return status

--*/

HRESULT GetIPAdapterInfo(HWND hwndDlg, PIP_ADAPTER_INFO *ppAdapterInfoHead, PIP_ADAPTER_INFO *ppAdapterInfo)
{
    HRESULT     hr = S_OK;
    DWORD       dwRetVal;
    ULONG       ulBufferSize = 0;
    char        szAdapterName[MAX_ADAPTERNAME_STR];

    *ppAdapterInfoHead = NULL;

    dwRetVal = GetAdaptersInfo( *ppAdapterInfo, &ulBufferSize );
    if ( dwRetVal == ERROR_BUFFER_OVERFLOW) {
        if (!(*ppAdapterInfo = (PIP_ADAPTER_INFO)malloc(ulBufferSize))) {
            DEBUGMSG(ZONE_ERROR, (TEXT("Insufficient Memory for IPAdapter allocation.")));
            hr = E_FAIL;
            goto exit;
        }

        dwRetVal = GetAdaptersInfo( *ppAdapterInfo, &ulBufferSize);
        if (dwRetVal != ERROR_SUCCESS) {
            DEBUGMSG(ZONE_ERROR, (TEXT("Could not get adapter info")));
            hr = E_FAIL;
            goto exit;
        }
    } else if (dwRetVal != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (TEXT("Could not get adapter info")));
        hr = E_FAIL;
        goto exit;
    }

    if (*ppAdapterInfo == NULL) {
        DEBUGMSG(ZONE_ERROR, (TEXT("No IP Interface present")));
        hr = E_FAIL;
        goto exit;
    }

    *ppAdapterInfoHead = *ppAdapterInfo;

    if (GetAdapterNameA(hwndDlg,szAdapterName,sizeof(szAdapterName)) != S_OK) {
        DEBUGMSG(ZONE_ERROR, (TEXT("Error getting adapter name")));
        hr = E_FAIL;
        goto exit;
    }

    // Advance adapter pointer to the correct adapter in the list
    while (strcmp(szAdapterName,(*ppAdapterInfo)->AdapterName) != 0) {
        *ppAdapterInfo = (*ppAdapterInfo)->Next;
        if (*ppAdapterInfo == NULL) {
            DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Could not get ip info for the specified adapter")));
            hr = E_FAIL;
            goto exit;
        }
    }

    exit:
    return hr;

} // GetIPAdapterInfo



/*++

Routine Name: 

    RefreshIPCfgDialog

Routine Description:

    Refresh the info in the IP Config dialog

Arguments:

    hwndDlg:	HWND of the property page
    fRenew:		Whether the function should also renew the IP info

Return Value:

    HRESULT to indicate return status

--*/

HRESULT RefreshIPCfgDialog(HWND hwndDlg, BOOL fRenew)
{
    PIP_ADAPTER_INFO    pAdapterInfo = NULL, pAdapterInfoHead = NULL;
    PIP_INTERFACE_INFO  pBuffer = NULL;
    HRESULT             hr = S_OK;
    ULONG               ulBufferSize = 0;
    DWORD               dwRetVal;
    HWND                hAddrTypeLbl;
    HWND                hIPAddrLbl;
    HWND                hGatewayLbl;
    HWND                hSubnetMskLbl;
    WCHAR               szIpAddr[MAX_IP_FIELD_WIDTH];
    WCHAR               szGateway[MAX_IP_FIELD_WIDTH];
    WCHAR               szSubnetMsk[MAX_IP_FIELD_WIDTH];
    WCHAR               szOldIpAddr[MAX_IP_FIELD_WIDTH];
    WCHAR               szOldGateway[MAX_IP_FIELD_WIDTH];
    WCHAR               szOldSubnetMsk[MAX_IP_FIELD_WIDTH];
    WCHAR               szOldDhcpEnabled[STRING_BUFFER_MAX];
    INT                 i;
    TCHAR               szStatic[STRING_BUFFER_MAX];
    TCHAR               szDhcp[STRING_BUFFER_MAX];
    WCHAR               szAdapterName[MAX_ADAPTERNAME_STR];

    // Get UI components
    hAddrTypeLbl        = GetDlgItem(hwndDlg, IDC_ADDRTYPE_LBL_INF);
    hIPAddrLbl          = GetDlgItem(hwndDlg, IDC_IPADDR_LBL_INF);
    hGatewayLbl         = GetDlgItem(hwndDlg, IDC_GATEWAY_LBL_INF);
    hSubnetMskLbl       = GetDlgItem(hwndDlg, IDC_SUBNETMSK_LBL_INF);

    // Get IP adapter info
    hr = GetIPAdapterInfo(hwndDlg, &pAdapterInfoHead, &pAdapterInfo);
    if (FAILED(hr)) {
        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Could not get the adapter info")));
        goto exit;
    }

    // If renew IP Info is specified
    if (fRenew) {
        ulBufferSize = 0;

        // Initialize interface
        dwRetVal = GetInterfaceInfo(pBuffer, &ulBufferSize);
        if (dwRetVal == ERROR_INSUFFICIENT_BUFFER) {
            pBuffer = (PIP_INTERFACE_INFO)malloc(ulBufferSize);
            if (!pBuffer) {
                DEBUGMSG(ZONE_ERROR, (TEXT("Insufficient Memory for IPInterface allocation.")));
                hr = E_FAIL;
                goto exit;
            }
            dwRetVal = GetInterfaceInfo(pBuffer, &ulBufferSize);
        }

        if (dwRetVal != NO_ERROR) {
            DEBUGMSG(ZONE_ERROR, (TEXT("NetUIQC: Could not get interface for renewing IP info")));
            hr = E_FAIL;
            goto exit;
        }

        // If no adapter interface is present
        if (ulBufferSize == 0) {
            hr = E_FAIL;
            goto exit;
        }

        SetCursor(LoadCursor(NULL, IDC_WAIT));

        if (GetAdapterNameW(hwndDlg,szAdapterName,sizeof(szAdapterName)) != S_OK) {
            hr = E_FAIL;
            goto exit;
        }

        // Loop through adpaters to find the current adapter
        for (i=0;i<pBuffer->NumAdapters;i++) {
            if (!wcscmp(pBuffer->Adapter[i].Name, szAdapterName)) {
                // Release the IP address of the specified adpater
                dwRetVal = IpReleaseAddress(&(pBuffer->Adapter[i]));

                // Reset labels based on a successful Release
                if (dwRetVal == NO_ERROR) {
                    SetWindowText(hIPAddrLbl, L"0.0.0.0");
                    SetWindowText(hSubnetMskLbl, L"0.0.0.0");
                    SetWindowText(hGatewayLbl, L"");
                    Sleep(500); // Sleep for dramatic effect
                }

                // Renew the IP address of the specified adapter
                IpRenewAddress(&(pBuffer->Adapter[i]));
            }
        }

        SetCursor(LoadCursor(NULL, IDC_ARROW));
    } else {
        // Get IP data from iphelper
        mbstowcs(szIpAddr, pAdapterInfo->IpAddressList.IpAddress.String, strlen(pAdapterInfo->IpAddressList.IpAddress.String)+1);
        mbstowcs(szGateway, pAdapterInfo->GatewayList.IpAddress.String, strlen(pAdapterInfo->GatewayList.IpAddress.String)+1);
        mbstowcs(szSubnetMsk, pAdapterInfo->IpAddressList.IpMask.String, strlen(pAdapterInfo->IpAddressList.IpMask.String)+1);

        LoadString(v_hInst, IDS_QC_DHCP, szDhcp, sizeof(szDhcp)/sizeof(szDhcp[0]));
        LoadString(v_hInst, IDS_QC_STATIC_IP, szStatic, sizeof(szStatic)/sizeof(szStatic[0]));

        // Set the data in the UI components if it has changed

        GetWindowText(hAddrTypeLbl, szOldDhcpEnabled, STRING_BUFFER_MAX);
        if (wcscmp(((pAdapterInfo->DhcpEnabled) ? szDhcp : szStatic), szOldDhcpEnabled) != 0) {
            SetWindowText(hAddrTypeLbl, (pAdapterInfo->DhcpEnabled) ? szDhcp : szStatic);
        }

        GetWindowText(hIPAddrLbl, szOldIpAddr, MAX_IP_FIELD_WIDTH);
        if (wcscmp(szOldIpAddr, szIpAddr) != 0) {
            SetWindowText(hIPAddrLbl, szIpAddr);
        }

        GetWindowText(hGatewayLbl, szOldGateway, MAX_IP_FIELD_WIDTH);
        if (wcscmp(szOldGateway, szGateway) != 0) {
            SetWindowText(hGatewayLbl, szGateway);
        }

        GetWindowText(hSubnetMskLbl, szOldSubnetMsk, MAX_IP_FIELD_WIDTH);
        if (wcscmp(szOldSubnetMsk, szSubnetMsk) != 0) {
            SetWindowText(hSubnetMskLbl, szSubnetMsk);
        }
    }

    exit:

    if (pBuffer) {
        free(pBuffer);
    }
    if (pAdapterInfoHead) {
        free(pAdapterInfoHead);
    }

    return hr;

} // RefreshIPCfgDialog



void
GetIPv6AddressString(TCHAR *szBuffer, PSOCKADDR_IN6 pAddr)
{
    DWORD dwBufLen = INET6_ADDRSTRLEN;

    if (WSAAddressToString(
                          (struct sockaddr *)pAddr,
                          sizeof(SOCKADDR_IN6),
                          NULL,
                          szBuffer,
                          &dwBufLen)) {
        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Error from WSAAddressToString():0x%X"),WSAGetLastError()));
        LoadString(v_hInst, IDS_QC_V6_ADDR_UNAVAIL, szBuffer, INET6_ADDRSTRLEN);
    }
}


/*++

Routine Name: 

    GetIPv6DefaultGateway

Routine Description:

    Retrieve the IPv6 default gateway for a given adapter.  This uses the IPv6 IOCTL interface to 
    query the route table.  In future releases, this should be done through IPHLPAPI, since the IOCTL
    interface can change from release to release.  However, v6 compatible routines for retrieving route
    table info have not been defined yet...

Arguments:

    pAA:	  Adapter address struct returned from GetAdaptersAddresses()
    szAddr:	  String buffer to hold address (must be INET6_ADDRSTRLEN chars)

Return Value:

    TRUE if successful, FALSE if default gateway could not be retrieved.

--*/
BOOL
GetIPv6DefaultGateway(PIP_ADAPTER_ADDRESSES pAA, TCHAR *szAddr)
{
    HANDLE                  hIPv6;
    IPV6_QUERY_ROUTE_TABLE  QueryRouteTable, NextQueryRouteTable;
    IPV6_INFO_ROUTE_TABLE   RTE;
    ULONG                   ulBytesReturned;
    SOCKADDR_IN6            Addr;
    BOOL                    bRet = FALSE;


    hIPv6 = CreateFileW(
                       DD_IPV6_DEVICE_NAME,
                       GENERIC_WRITE,                      // access mode
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,                               // security attributes
                       OPEN_EXISTING,
                       0,                                  // flags & attributes
                       NULL);                              // template file

    if (hIPv6 == INVALID_HANDLE_VALUE) {
        DEBUGMSG(ZONE_ERROR,(TEXT("!Error opening IPv6 driver to get default gateway\r\n")));
        return FALSE;
    }

    NextQueryRouteTable.Neighbor.IF.Index = 0;

    for (;;) {
        QueryRouteTable = NextQueryRouteTable;

        if (!DeviceIoControl(
                            hIPv6, 
                            IOCTL_IPV6_QUERY_ROUTE_TABLE,
                            &QueryRouteTable, 
                            sizeof(QueryRouteTable),
                            &RTE, 
                            sizeof(RTE), 
                            &ulBytesReturned,
                            NULL)) {
            DEBUGMSG(ZONE_ERROR,(TEXT("!Error querying IPv6 driver to get default gateway\r\n")));
            break;
        }

        NextQueryRouteTable = RTE.Next;

        if (QueryRouteTable.Neighbor.IF.Index != 0) {
            RTE.This = QueryRouteTable;

            if ((RTE.This.PrefixLength == 0) &&
                (pAA->Ipv6IfIndex == RTE.This.Neighbor.IF.Index)) {
                // Found a valid router
                Addr.sin6_family   = AF_INET6;
                Addr.sin6_addr     = RTE.This.Neighbor.Address;
                Addr.sin6_port     = 0;

                if (IN6_IS_ADDR_LINKLOCAL(&(Addr.sin6_addr))) {
                    Addr.sin6_scope_id = pAA->ZoneIndices[ScopeLevelLink];
                } else
                    if (IN6_IS_ADDR_SITELOCAL(&(Addr.sin6_addr))) {
                    Addr.sin6_scope_id = pAA->ZoneIndices[ScopeLevelSite];
                } else {
                    Addr.sin6_scope_id = 0;
                }

                GetIPv6AddressString(szAddr,&Addr);
                bRet = TRUE;
                break;
            }

        }

        if (NextQueryRouteTable.Neighbor.IF.Index == 0)
            break;
    }

    CloseHandle(hIPv6);
    return bRet;
}



/*++

Routine Name: 

    UpdateIPv6AddressField

Routine Description:

    Update text in IPv6 adddress field.  Don't touch if unchanged (to avoid flickering)

Arguments:

    hwndDlg:	HWND of the property page
    ulLabelId:  ID of the field
    szAddr:     Address string (at most INET6_ADDRSTRLEN bytes)

Return Value:

    none

--*/
static BOOL bListViewInit = FALSE;
#define NUM_IPV6ITEMS 8

struct _ColInfo
{
	int StringID;
	int CtrlID;
	int RowID;
} ViewInfo[NUM_IPV6ITEMS] = {
	
	{IDS_IPV6_DEFAULT_GATEWAY,	IDC_V6GATEWAY_LBL_INF, -1},
	{IDS_IPV6_GLOBAL,			IDC_V6PREFGLOBADDR_LBL_INF, -1},
	{IDS_IPV6_SITE_LOCAL,		IDC_V6PREFSITEADDR_LBL_INF, -1},
	{IDS_IPV6_LINK_LOCAL,		IDC_V6PREFLINKADDR_LBL_INF, -1},
	{IDS_IPV6_6TO4,				IDC_V66TO4ADDR_LBL_INF,		-1},
	{IDS_IPV6_AUTO_TUNNEL,		IDC_V6TUNNELADDR_LBL_INF1,	-1},
	{IDS_IPV6_AUTO_TUNNEL2,		IDC_V6TUNNELADDR_LBL_INF2,	-1},
	{IDS_IPV6_AUTO_TUNNEL3,		IDC_V6TUNNELADDR_LBL_INF3,	-1}
};



BOOL InitListView(HWND hwndDlg)
{
    HWND	hListViewWnd = GetDlgItem(hwndDlg, IDC_IPV6_LISTBOX);

	// Need to the add two columns first
	LVCOLUMN lvc = {0};
    LVITEM 	lvi = {0};
    TCHAR  szBuffer[INET6_ADDRSTRLEN];
	int	i;

	lvc.mask = LVCF_TEXT;

    LoadString(v_hInst, IDS_IPV6_COL_PROPERTY, szBuffer, INET6_ADDRSTRLEN);
	lvc.pszText = szBuffer;
	ListView_InsertColumn(hListViewWnd, 0, &lvc);

    LoadString(v_hInst, IDS_IPV6_COL_VALUE, szBuffer, INET6_ADDRSTRLEN);
	lvc.pszText = szBuffer;
    ListView_InsertColumn(hListViewWnd, 1, &lvc);


	// add the rows to the list
	memset(&lvi,0,sizeof(lvi));
	lvi.mask = LVIF_PARAM | LVIF_TEXT;

	for (i = 0; i < NUM_IPV6ITEMS; i++)
	{
		lvi.iItem = i;
	    LoadString(v_hInst, ViewInfo[i].StringID, szBuffer, INET6_ADDRSTRLEN);
		lvi.pszText = szBuffer;
		lvi.lParam = (LPARAM)ViewInfo[i].CtrlID;
		
		ViewInfo[i].RowID = ListView_InsertItem(hListViewWnd, &lvi);
		
		// add second column data (empty to start
	    LoadString(v_hInst, IDS_QC_V6_ADDR_UNAVAIL, szBuffer, INET6_ADDRSTRLEN);
		ListView_SetItemText(hListViewWnd, ViewInfo[i].RowID , 1, szBuffer);
		
	}

	// adjust column widths
	ListView_SetColumnWidth(hListViewWnd, 0, LVSCW_AUTOSIZE);
	ListView_SetColumnWidth(hListViewWnd, 1, 300);

	bListViewInit = TRUE;

	return TRUE;
}



void
UpdateIPv6AddressField(HWND hwndDlg, ULONG ulLabelId, TCHAR *szAddr)
{
	
    HWND	hListViewWnd = GetDlgItem(hwndDlg, IDC_IPV6_LISTBOX);
    int		iCount = 0;
	DWORD	dwID = 0;
	LVITEM	lvi = {0};
	int		i;
	BOOL	bFound = FALSE;

    TCHAR	szOldVal[INET6_ADDRSTRLEN];

	if (!bListViewInit)
	{
		InitListView(hwndDlg);
	}
	
	// Locate the line that contains the info to be updated
	for (i = 0; i < NUM_IPV6ITEMS; i++)
	{
		if (ulLabelId == ViewInfo[i].CtrlID)
		{
			// found the one we want to update
			ListView_GetItemText(hListViewWnd, ViewInfo[i].RowID, 1, szOldVal, INET6_ADDRSTRLEN);

			if (wcscmp(szOldVal, szAddr))
			{
				ListView_SetItemText(hListViewWnd, ViewInfo[i].RowID, 1, szAddr);
			}
			break;
        }
	}
}


__inline int IN6_IS_ADDR_6TO4(const struct in6_addr *a)
{
    return((a->s6_bytes[0] == 0x20) && (a->s6_bytes[1] == 0x02));
}

__inline int IN6_IS_ADDR_ISATAP(const struct in6_addr *a)
{
    return(((a->s6_words[4] & 0xfffd) == 0) && (a->s6_words[5] == 0xfe5e));
}

/*++

Routine Name: 

    RefreshIPCfgDialog

Routine Description:

    Refresh the info in the IP Config dialog

Arguments:

    hwndDlg:	HWND of the property page
    fRenew:		Whether the function should also renew the IP info

Return Value:

    HRESULT to indicate return status

--*/

HRESULT RefreshIPv6CfgDialog(HWND hwndDlg)
{
    HRESULT             hr = S_OK;
    ULONG               ulBufferSize = 0;
    DWORD               dwRetVal;

    TCHAR               szAddr[INET6_ADDRSTRLEN];
    TCHAR               szAddrNotAvail[INET6_ADDRSTRLEN];
    BOOL                bDefaultGateway = FALSE;
    BOOL                bPrefGlobal = FALSE;
    BOOL                bPrefLinkLocal = FALSE;
    BOOL                bPrefSiteLocal = FALSE;
    BOOL                b6to4 = FALSE;
    BOOL                bAuto1 = FALSE;
    BOOL                bAuto2 = FALSE;
    BOOL                bAuto3 = FALSE;

    PIP_ADAPTER_ADDRESSES pAdapterAddresses = NULL, pThisAdapterAddresses;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicastAddresses;
    char                szAdapterName[MAX_ADAPTERNAME_STR];

    // Get IP address info
    dwRetVal = GetAdaptersAddresses(AF_INET6,GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST,
                                    NULL,NULL,&ulBufferSize);

    if (dwRetVal != ERROR_BUFFER_OVERFLOW) {
        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Error from GetAdaptersAddresses():%u"),dwRetVal));
        hr = E_FAIL;
        goto exit;
    }

    pAdapterAddresses = (PIP_ADAPTER_ADDRESSES)malloc(ulBufferSize);
    if (pAdapterAddresses == NULL) {
        hr = E_FAIL;
        goto exit;
    }

    dwRetVal = GetAdaptersAddresses(AF_INET6,GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST,
                                    NULL,pAdapterAddresses,&ulBufferSize);
    if (dwRetVal != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Error from GetAdaptersAddresses():%u"),dwRetVal));
        hr = E_FAIL;
        goto exit;
    }
    // Get our adapter name
    if (GetAdapterNameA(hwndDlg,szAdapterName,sizeof(szAdapterName)) != S_OK) {
        DEBUGMSG(ZONE_ERROR, (TEXT("Error getting adapter name")));
        hr = E_FAIL;
        goto exit;
    }

    // Walk adapter list
    for (pThisAdapterAddresses = pAdapterAddresses; 
        pThisAdapterAddresses;
        pThisAdapterAddresses = pThisAdapterAddresses->Next) {

        BOOL bCurrentAdapter;

        DEBUGMSG(ZONE_MISC,(TEXT("GetAdaptersAddresses adapter <%s>\n"),pThisAdapterAddresses->FriendlyName));

        if (! strcmp(szAdapterName,pThisAdapterAddresses->AdapterName)) {

            // IPv6 stack will return address info even if media disconnected.  Don't display
            // addresses in this case.
            if (!(pThisAdapterAddresses->OperStatus & IfOperStatusUp)) {
                DEBUGMSG(ZONE_MISC,(TEXT("GetAdaptersAddresses -- Media Disconnected\n")));
                break;
            }

            bCurrentAdapter = TRUE;
            if (GetIPv6DefaultGateway(pThisAdapterAddresses, szAddr)) {
                UpdateIPv6AddressField(hwndDlg, IDC_V6GATEWAY_LBL_INF, szAddr);
                bDefaultGateway = TRUE;
            }
        }
        else {
            bCurrentAdapter = FALSE;
            if (pThisAdapterAddresses->IfType != IF_TYPE_TUNNEL) {
                // Skip other non tunneling adapters 
                continue;
            }
            // TBD - For tunneling interfaces, we want to choose the one matching the v4 address on this interface

        }

        for (pUnicastAddresses = pThisAdapterAddresses->FirstUnicastAddress ;
            pUnicastAddresses;
            pUnicastAddresses = pUnicastAddresses->Next) {
            PSOCKADDR_IN6 pSA = (PSOCKADDR_IN6)pUnicastAddresses->Address.lpSockaddr;
            ULONG AddrFieldLbl;

            GetIPv6AddressString(szAddr, pSA);

            DEBUGMSG(ZONE_MISC,(TEXT("   GetAdaptersAddresses return addr <%s>\n"),szAddr));

            AddrFieldLbl = 0;

            if (IN6_IS_ADDR_SITELOCAL(&pSA->sin6_addr)) {
                if (bCurrentAdapter) {
                    AddrFieldLbl = IDC_V6PREFSITEADDR_LBL_INF;
                    bPrefSiteLocal = TRUE;
                } else if (IN6_IS_ADDR_ISATAP(&pSA->sin6_addr)) {
                    AddrFieldLbl = IDC_V6TUNNELADDR_LBL_INF2;
                    bAuto2 = TRUE;
                }
            } else if (IN6_IS_ADDR_LINKLOCAL(&pSA->sin6_addr)) {
                if (bCurrentAdapter) {
                    AddrFieldLbl = IDC_V6PREFLINKADDR_LBL_INF;
                    bPrefLinkLocal = TRUE;
                } else if (IN6_IS_ADDR_ISATAP(&pSA->sin6_addr)) {
                    AddrFieldLbl = IDC_V6TUNNELADDR_LBL_INF3;
                    bAuto3 = TRUE;
                }
            } else {
                if (bCurrentAdapter) {
                    AddrFieldLbl = IDC_V6PREFGLOBADDR_LBL_INF;
                    bPrefGlobal = TRUE;
                } else if (IN6_IS_ADDR_6TO4(&pSA->sin6_addr)) {
                    AddrFieldLbl = IDC_V66TO4ADDR_LBL_INF;
                    b6to4 = TRUE;
                } else if (IN6_IS_ADDR_ISATAP(&pSA->sin6_addr)) {
                    AddrFieldLbl = IDC_V6TUNNELADDR_LBL_INF1;
                    bAuto1 = TRUE;
                }
            }

            if (AddrFieldLbl)
                UpdateIPv6AddressField(hwndDlg, AddrFieldLbl, szAddr);
        }

    }


    exit:
    // Display address not available for fields we couldn't populate

    LoadString(v_hInst, IDS_QC_V6_ADDR_UNAVAIL, szAddrNotAvail, INET6_ADDRSTRLEN);
    if (!bDefaultGateway)
        UpdateIPv6AddressField(hwndDlg, IDC_V6GATEWAY_LBL_INF, szAddrNotAvail);
    if (!bPrefGlobal)
        UpdateIPv6AddressField(hwndDlg, IDC_V6PREFGLOBADDR_LBL_INF, szAddrNotAvail);
    if (!bPrefSiteLocal)
        UpdateIPv6AddressField(hwndDlg, IDC_V6PREFSITEADDR_LBL_INF, szAddrNotAvail);
    if (!bPrefLinkLocal)
        UpdateIPv6AddressField(hwndDlg, IDC_V6PREFLINKADDR_LBL_INF, szAddrNotAvail);
    if (!b6to4)
        UpdateIPv6AddressField(hwndDlg, IDC_V66TO4ADDR_LBL_INF, szAddrNotAvail);
    if (!bAuto1)
        UpdateIPv6AddressField(hwndDlg, IDC_V6TUNNELADDR_LBL_INF1, szAddrNotAvail);
    if (!bAuto2)
        UpdateIPv6AddressField(hwndDlg, IDC_V6TUNNELADDR_LBL_INF2, szAddrNotAvail);
    if (!bAuto3)
        UpdateIPv6AddressField(hwndDlg, IDC_V6TUNNELADDR_LBL_INF3, szAddrNotAvail);

    if (pAdapterAddresses) {
        free(pAdapterAddresses);
    }
    return hr;
}

/*++

Routine Name: 

    IPv6QuickCfgDialogProc

Routine Description:

    Dialog Proc for the IP Info property page.  This function handles all the
    messages encountered on the property page.

Arguments:

    hwndDlg:	HWND for the property page
    uMsg:		Message encountered on the property page
    wParam:		Additional data passed to the dialog
    lParam:		Additional data passed to the dialog

Return Value:

    BOOL (TRUE/FALSE)

--*/

BOOL CALLBACK IPv6QuickCfgDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL        fRetVal = FALSE;
    HRESULT     hr = S_OK;

    switch (uMsg) {
    case WM_INITDIALOG:
		InitListView(hwndDlg);
        RefreshIPv6CfgDialog(hwndDlg);
        SetTimer(hwndDlg, 1, IPQC_RFSH_TIMEOUT, NULL);
        fRetVal = TRUE;
        break;

    case WM_TIMER:
        RefreshIPv6CfgDialog(hwndDlg);
        fRetVal = FALSE;
        break;

    case WM_COMMAND:

        fRetVal = 0;
        break;

	case WM_DESTROY:
		bListViewInit = FALSE;
		break;

    default:
        fRetVal = FALSE;
    }

    return fRetVal;

} // IPQuickCfgDialogProc
