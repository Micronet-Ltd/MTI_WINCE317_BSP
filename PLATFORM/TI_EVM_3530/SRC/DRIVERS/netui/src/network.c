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

Network.c

Abstract:  

Functions:


Notes: 


--*/
#include <windows.h>
#include <tchar.h>
#include "string.h"
#include "memory.h"
#include "commctrl.h"
#include "resource.h"
#include "netui.h"
#include "netuip.h"
#include "wzcuip.h"
#include "wcommctl.h"

typedef struct _NETWORK_ADPT_INFO {
	LPTSTR	szAdapterName;
	BOOL	fUseDHCP;
	BOOL	fSaved;
	DWORD	IPAddr;
	DWORD	SubnetMask;
	DWORD	Gateway;
	DWORD	DNSAddr;
	DWORD	DNSAltAddr;
	DWORD	WINSAddr;
	DWORD	WINSAltAddr;
	TCHAR	szDisplayName[256];
} NETWORK_ADPT_INFO, *PNETWORK_ADPT_INFO;

#define net_long(x) (((((ulong)(x))&0xffL)<<24) | \
                     ((((ulong)(x))&0xff00L)<<8) | \
                     ((((ulong)(x))&0xff0000L)>>8) | \
                     ((((ulong)(x))&0xff000000L)>>24))
                     
#define INADDR_ANY              0x00000000
#define INADDR_LOOPBACK         0x7f000001
#define INADDR_BROADCAST        0xffffffff

//
// Make sure the specified static IP address is valid
//
BOOL
IsValidIPAddress(
    DWORD IPAddr
    )
{
    switch (IPAddr) {
    case INADDR_ANY:
    case INADDR_LOOPBACK:
    case INADDR_BROADCAST:
        return FALSE;
    }

    return TRUE;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
LRESULT CALLBACK
NetworkIPDlgProc (
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam
   )
{
	PROPSHEETPAGE *psp;
	NMHDR		*header;
	PNETWORK_ADPT_INFO pAdptInfo;
	DWORD	Flag;
	TCHAR	szTitle[256];
	TCHAR	szErrMsg[256];
	DWORD   dwStyle;
	HWND    hParentWnd;
	DEBUGMSG (ZONE_NETWORK && ZONE_MISC,
			  (TEXT("+NetworkIPDlgProc(0x%X, 0x%X, 0x%X, 0x%X)\r\n"),
			   hDlg, message, wParam, lParam));
	
	switch (message) {
	case WM_NOTIFY :
		header = (NMHDR*) lParam;

		if (PSN_APPLY != header->code) {
			break;
		}
		DEBUGMSG (ZONE_NETWORK,
				  (TEXT(" NetworkIPDlgProc:Apply Changes....\r\n")));
		
		pAdptInfo = (PNETWORK_ADPT_INFO) GetWindowLong (hDlg, GWL_USERDATA);
        if (pAdptInfo == NULL) {
            break;
        }

		// Save the data into the struct.
		pAdptInfo->fUseDHCP = SendMessage(GetDlgItem(hDlg, IDC_USEDHCP),
										  BM_GETCHECK, 0, 0);
		if (pAdptInfo->fUseDHCP) {
			// Clear the addresses.
			pAdptInfo->IPAddr = 0;
			pAdptInfo->SubnetMask = 0;
			pAdptInfo->Gateway = 0;
		} else {
			SendMessage(GetDlgItem(hDlg, IDC_IP_ADDR), IP_GETADDRESS, 0,
						(LPARAM) &pAdptInfo->IPAddr);
			SendMessage(GetDlgItem(hDlg, IDC_SUBNET_MASK), IP_GETADDRESS, 0,
						(LPARAM) &pAdptInfo->SubnetMask);
			SendMessage(GetDlgItem(hDlg, IDC_GATEWAY), IP_GETADDRESS, 0,
						(LPARAM) &pAdptInfo->Gateway);

			// Verify that address is correct.
			if (FALSE == IsValidIPAddress(pAdptInfo->IPAddr)) {
				LoadString (v_hInst, IDS_ERR_TITLE, szTitle,
							sizeof(szTitle)/sizeof(TCHAR));
				LoadString (v_hInst, IDS_NETWORK_ERR_BADIP, szErrMsg,
							sizeof(szErrMsg)/sizeof(TCHAR));
				MessageBox (hDlg, szErrMsg, szTitle, MB_ICONEXCLAMATION);
				SetWindowLong (hDlg, DWL_MSGRESULT,
							   PSNRET_INVALID_NOCHANGEPAGE);
				return TRUE;

			}
		}
		pAdptInfo->fSaved = TRUE;

		SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
		return TRUE;
		
	case WM_INITDIALOG :
		// This is always the first window drawn, center the parent now
		hParentWnd = GetParent(hDlg);

		// If invoked directly (i.e. not from a control panel
		// then make sure that we are marked as topmost
		if (!GetWindow(hParentWnd, GW_OWNER)) {
			dwStyle = GetWindowLong (hParentWnd, GWL_EXSTYLE);

			SetWindowLong (hParentWnd, GWL_EXSTYLE, dwStyle | WS_EX_TOPMOST);
		}

		psp = (PROPSHEETPAGE *)lParam;
		
		SetWindowLong (hDlg, GWL_USERDATA, psp->lParam);
		pAdptInfo = (PNETWORK_ADPT_INFO)psp->lParam;
		
		DEBUGMSG (ZONE_NETWORK,
				  (TEXT(" NetworkIPDlgProc: WM_INITDIALOG(lParam=0x%X(%s)\r\n"),
				   lParam, (pAdptInfo->szAdapterName[0]) ?
				   pAdptInfo->szAdapterName : TEXT("<NULL>")));

		// Save the name away.
		SetWindowLong (hDlg, DWL_USER, (LONG)pAdptInfo);

		EnableWindow(GetDlgItem(hDlg, IDC_IP_ADDR), !pAdptInfo->fUseDHCP);
		EnableWindow(GetDlgItem(hDlg, IDC_IPADDRLABEL), !pAdptInfo->fUseDHCP);
		
		EnableWindow(GetDlgItem(hDlg, IDC_SUBNET_MASK), !pAdptInfo->fUseDHCP);
		EnableWindow(GetDlgItem(hDlg, IDC_SUBNETLABEL), !pAdptInfo->fUseDHCP);
		
		EnableWindow(GetDlgItem(hDlg, IDC_GATEWAY), !pAdptInfo->fUseDHCP);
		EnableWindow(GetDlgItem(hDlg, IDC_GATEWAYLABEL), !pAdptInfo->fUseDHCP);
		
		if (pAdptInfo->fUseDHCP) {
			// Set the default state.
			CheckRadioButton (hDlg, IDC_USEDHCP, IDC_DONTUSEDHCP, IDC_USEDHCP);
			
		} else {
			CheckRadioButton (hDlg, IDC_USEDHCP, IDC_DONTUSEDHCP,
							  IDC_DONTUSEDHCP);
			
			if (pAdptInfo->IPAddr) {
			SendMessage (GetDlgItem(hDlg, IDC_IP_ADDR), IP_SETADDRESS, 0,
						 (LPARAM) pAdptInfo->IPAddr);
			}
			
			if (pAdptInfo->SubnetMask) {
				SendMessage (GetDlgItem(hDlg, IDC_SUBNET_MASK), IP_SETADDRESS, 0,
							 (LPARAM) pAdptInfo->SubnetMask);
			}
			if (pAdptInfo->Gateway) {
				SendMessage (GetDlgItem(hDlg, IDC_GATEWAY), IP_SETADDRESS, 0,
							 (LPARAM) pAdptInfo->Gateway);
			}
			
		}

		return TRUE;
	case WM_COMMAND :
		pAdptInfo = (PNETWORK_ADPT_INFO) GetWindowLong (hDlg, GWL_USERDATA);
		
		DEBUGMSG (ZONE_NETWORK,
				  (TEXT(" NetworkIPDlgProc: WM_COMMAND(wParam=0x%X)\r\n"),
				   wParam));
		if ((HIWORD(wParam) == EN_SETFOCUS) &&
			((HWND)lParam == GetDlgItem(hDlg, IDC_SUBNET_MASK))) {
			SendMessage(GetDlgItem(hDlg, IDC_IP_ADDR), IP_GETADDRESS, 0,
						(LPARAM) &pAdptInfo->IPAddr);
			SendMessage(GetDlgItem(hDlg, IDC_SUBNET_MASK), IP_GETADDRESS, 0,
						(LPARAM) &pAdptInfo->SubnetMask);
			if (pAdptInfo->IPAddr && (0 == pAdptInfo->SubnetMask)) {
				// Initialize a subnet mask
				// Get top byte
				Flag = pAdptInfo->IPAddr >> 24;
				if (Flag <= 127) {
					pAdptInfo->SubnetMask = 0xFF000000;
				} else if (Flag <= 191) {
					pAdptInfo->SubnetMask = 0xFFFF0000;
				} else {
					// Ok I'm ignoring class D or E
					pAdptInfo->SubnetMask = 0xFFFFFF00;
				}
				SendMessage (GetDlgItem(hDlg, IDC_SUBNET_MASK),
							 IP_SETADDRESS, 0,
							 (LPARAM) pAdptInfo->SubnetMask);
			}
			break;
		}
        switch( LOWORD( wParam ) ) {
		case IDC_USEDHCP :
		case IDC_DONTUSEDHCP :
			// When either of these change...
			if (SendMessage(GetDlgItem(hDlg, IDC_DONTUSEDHCP),
							BM_GETCHECK, 0, 0)) {
				Flag = TRUE;
			} else {
				Flag = FALSE;
				
				SendMessage(GetDlgItem(hDlg, IDC_IP_ADDR),
							IP_CLEARADDRESS, 0, 0);
				SendMessage(GetDlgItem(hDlg, IDC_SUBNET_MASK),
							IP_CLEARADDRESS, 0, 0);
				SendMessage(GetDlgItem(hDlg, IDC_GATEWAY),
							IP_CLEARADDRESS, 0, 0);

				// Clear the values.
				pAdptInfo->IPAddr = 0;
				pAdptInfo->SubnetMask = 0;
				pAdptInfo->Gateway = 0;
				
			}
			EnableWindow(GetDlgItem(hDlg, IDC_IP_ADDR), Flag);
			EnableWindow(GetDlgItem(hDlg, IDC_IPADDRLABEL), Flag);

			EnableWindow(GetDlgItem(hDlg, IDC_SUBNET_MASK), Flag);
			EnableWindow(GetDlgItem(hDlg, IDC_SUBNETLABEL), Flag);

			EnableWindow(GetDlgItem(hDlg, IDC_GATEWAY), Flag);
			EnableWindow(GetDlgItem(hDlg, IDC_GATEWAYLABEL), Flag);
	
			
			break;
		}
		break;
	default :
		DEBUGMSG (ZONE_NETWORK && ZONE_MISC,
				  (TEXT("-NetworkIPDlgProc: (default) Return FALSE\r\n")));
		return (FALSE);
	}
	DEBUGMSG (ZONE_NETWORK && ZONE_MISC,
			  (TEXT("-NetworkIPDlgProc: Return TRUE\r\n")));
	return TRUE;
	
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
LRESULT CALLBACK
NetworkNameServDlgProc (
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam
   )
{
	PROPSHEETPAGE *psp;
	NMHDR		*header;
	PNETWORK_ADPT_INFO pAdptInfo;
	
	DEBUGMSG (ZONE_NETWORK && ZONE_MISC,
			  (TEXT("+NetworkNameServDlgProc(0x%X, 0x%X, 0x%X, 0x%X)\r\n"),
			   hDlg, message, wParam, lParam));
	
	switch (message) {
	case WM_NOTIFY :
		header = (NMHDR*) lParam;

		if (PSN_APPLY != header->code) {
			break;
		}
		DEBUGMSG (ZONE_NETWORK,
				  (TEXT(" NetworkNameServDlgProc:Apply Changes....\r\n")));
		
		pAdptInfo = (PNETWORK_ADPT_INFO) GetWindowLong (hDlg, GWL_USERDATA);

		SendMessage(GetDlgItem(hDlg, IDC_DNS_ADDR), IP_GETADDRESS, 0,
					(LPARAM) &pAdptInfo->DNSAddr);
		SendMessage(GetDlgItem(hDlg, IDC_DNS_ALTADDR), IP_GETADDRESS, 0,
					(LPARAM) &pAdptInfo->DNSAltAddr);
		SendMessage(GetDlgItem(hDlg, IDC_WINS_ADDR), IP_GETADDRESS, 0,
					(LPARAM) &pAdptInfo->WINSAddr);
		SendMessage(GetDlgItem(hDlg, IDC_WINS_ALTADDR), IP_GETADDRESS, 0,
					(LPARAM) &pAdptInfo->WINSAltAddr);

		DEBUGMSG (ZONE_NETWORK, (TEXT(" NetworkNameServDlgProc:\r\n")
								 TEXT("\tDNSAddr=0x%X DNSAltAddr=0x%X\r\n")
								 TEXT("\tWINSAddr=0x%X WINSAltAddr=0x%X\r\n"),
								 pAdptInfo->DNSAddr, pAdptInfo->DNSAltAddr,
								 pAdptInfo->WINSAddr, pAdptInfo->WINSAltAddr));
		
		SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
		return TRUE;
		
	case WM_INITDIALOG :
		psp = (PROPSHEETPAGE *)lParam;
		
		SetWindowLong (hDlg, GWL_USERDATA, psp->lParam);
		pAdptInfo = (PNETWORK_ADPT_INFO)psp->lParam;
		DEBUGMSG (ZONE_NETWORK,
				  (TEXT(" NetworkNameServDlgProc: WM_INITDIALOG(lParam=0x%X(%s)\r\n"),
				   lParam, (pAdptInfo) ?
				   pAdptInfo->szAdapterName : TEXT("<NULL>")));

		// Save the name away.
		SetWindowLong (hDlg, DWL_USER, (LONG)pAdptInfo);

		if (pAdptInfo->DNSAddr) {
			SendMessage(GetDlgItem(hDlg, IDC_DNS_ADDR), IP_SETADDRESS, 0,
						(LPARAM) pAdptInfo->DNSAddr);
		}
		if (pAdptInfo->DNSAltAddr) {
			SendMessage(GetDlgItem(hDlg, IDC_DNS_ALTADDR), IP_SETADDRESS, 0,
						(LPARAM) pAdptInfo->DNSAltAddr);
		}
		if (pAdptInfo->WINSAddr) {
			SendMessage(GetDlgItem(hDlg, IDC_WINS_ADDR), IP_SETADDRESS, 0,
						(LPARAM) pAdptInfo->WINSAddr);
		}
		if (pAdptInfo->WINSAltAddr) {
			SendMessage(GetDlgItem(hDlg, IDC_WINS_ALTADDR), IP_SETADDRESS, 0,
						(LPARAM) pAdptInfo->WINSAltAddr);
		}
		
		return TRUE;
	case WM_COMMAND :
		pAdptInfo = (PNETWORK_ADPT_INFO) GetWindowLong (hDlg, GWL_USERDATA);
		break;
	default :
		DEBUGMSG (ZONE_NETWORK && ZONE_MISC,
				  (TEXT("-NetworkNameServDlgProc: (default) Return FALSE\r\n")));
		return (FALSE);
	}
	DEBUGMSG (ZONE_NETWORK && ZONE_MISC,
			  (TEXT("-NetworkNameServDlgProc: Return TRUE\r\n")));
	return TRUE;
	
}




int CALLBACK 
AdapterPropSheetProc(
   HWND hwndDlg,  // handle to the property sheet dialog box
   UINT uMsg,     // message identifier
   LPARAM lParam  // message parameter
   )
{
	DEBUGMSG (ZONE_NETWORK,
			  (TEXT("AdapterPropSheetProc(0x%X, 0x%X, 0x%X)\r\n"),
			   hwndDlg, uMsg, lParam));
	return 0;
}
void
IPAddrToStr (LPTSTR szStr, DWORD IPAddr)
{
	wsprintf (szStr, TEXT("%d.%d.%d.%d"),
			  (IPAddr >> 24) & 0xFF, (IPAddr >> 16) & 0xFF,
			  (IPAddr >> 8) & 0xFF,  IPAddr & 0xFF);
}
BOOL StringToAddr(TCHAR *AddressString, DWORD *AddressValue) {
	TCHAR	*pStr = AddressString;
	PUCHAR	AddressPtr = (PUCHAR)AddressValue;
	int		i;
	int		Value;

	// Parse the four pieces of the address.
	for (i=0; *pStr && (i < 4); i++) {
		Value = 0;
		while (*pStr && TEXT('.') != *pStr) {
			if ((*pStr < TEXT('0')) || (*pStr > TEXT('9'))) {
				DEBUGMSG (ZONE_NETWORK,
						  (TEXT("Unable to convert %s to address\r\n"),
						   AddressString));
				return FALSE;
			}
			Value *= 10;
			Value += *pStr - TEXT('0');
			pStr++;
		}
		if (Value > 255) {
			DEBUGMSG (ZONE_NETWORK,
					  (TEXT("Unable to convert %s to address\r\n"),
					   AddressString));
			return FALSE;
		}
		AddressPtr[i] = Value;
		if (TEXT('.') == *pStr) {
			pStr++;
		}
	}

	// Did we get all of the pieces?
	if (i != 4) {
		DEBUGMSG (ZONE_NETWORK,
				  (TEXT("Unable to convert %s to address\r\n"),
				   AddressString));
		return FALSE;
	}

	*AddressValue = net_long (*AddressValue);
	
	DEBUGMSG (ZONE_NETWORK,
			  (TEXT("Converted %s to address %X\r\n"),
			   AddressString, *AddressValue));

	return TRUE;
}	// StringToAddr()

VOID
GetMultiSZAddr (HKEY hKey, LPTSTR szValueName, LPDWORD pAddr1, LPDWORD pAddr2)
{
	TCHAR	szTemp[256];
	LPTSTR	szPtr;
	LONG	hRes;
	DWORD	dwSize, dwType;

	dwSize = sizeof(szTemp);
	hRes = RegQueryValueEx (hKey, szValueName, 0, &dwType, (LPBYTE)szTemp,
							&dwSize); 
	if ((hRes == ERROR_SUCCESS) &&
		((dwType == REG_SZ) || (dwType == REG_MULTI_SZ))) {
		if (pAddr1 && szTemp[0]) {
			StringToAddr (szTemp, pAddr1);
		}
		if (pAddr2 && szTemp[0] && (dwType == REG_MULTI_SZ)) {
			szPtr = szTemp;
			while (*szPtr) {
				szPtr++;
			}
			// Move past the null.
			szPtr++;
			if (*szPtr) {
				StringToAddr (szPtr, pAddr2);
			}
		}
	} else {
		DEBUGMSG (ZONE_NETWORK,
				  (TEXT("GetMultiSZ: '%s' hRes=%d dwType=%d\r\n"),
				   szValueName, hRes, dwType));
	}
	
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
BOOL
AdapterIPProperties (HWND hWndOwner, LPTSTR szAdapterName)
{
	BOOL	fRetVal = FALSE;
	TCHAR	szTemp[256];
	HKEY	hKey;
	LONG	hRes;
	DWORD	dwDisp, dwSize, dwType;
	NETWORK_ADPT_INFO	AdptInfo, OldAdptInfo;
	PROPSHEETPAGE psp[3];
	PROPSHEETHEADER psh;
	TCHAR szCaption1[60];
	TCHAR szCaption2[60];
	TCHAR szTitle[128];
	DWORD	Len;
	DWORD	cxScreen = GetSystemMetrics(SM_CXSCREEN);

    if (szAdapterName == NULL)
        return FALSE;

	DEBUGMSG (ZONE_NETWORK|ZONE_INTERFACE|ZONE_FUNCTION,
			  (TEXT("+AdapterIPProperties(0x%X, 0x%X(%s))\r\n"),
			   hWndOwner, szAdapterName, szAdapterName));

	if (cxScreen >= 480) {
		// We need the common control functions
		if (!ValidCommCtrlFunc(InitCommonControls)) {
			DEBUGMSG (ZONE_ERROR|ZONE_NETWORK,
					  (TEXT("!AdapterIPProperties: Common Controls not loaded\r\n")));
			return FALSE;
		}
	

		// Initialize the common controls.
		CallCommCtrlFunc(InitCommonControls)();
	} else {
		INITCOMMONCONTROLSEX	icce;

		icce.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icce.dwICC  = ICC_TOOLTIP_CLASSES|ICC_CAPEDIT_CLASS;
		
		// We need the common control functions
		if (!ValidCommCtrlFunc(InitCommonControlsEx)) {
			DEBUGMSG (ZONE_ERROR|ZONE_NETWORK,
					  (TEXT("!AdapterIPProperties: Common Controls not loaded\r\n")));
			return FALSE;
		}
	

		// Initialize the common controls.
		CallCommCtrlFunc(InitCommonControlsEx)(&icce);
	}
	
	// I need PropertySheetW as well.
	if (!ValidCommCtrlFunc(PropertySheetW)) {
		DEBUGMSG (ZONE_ERROR|ZONE_NETWORK,
				  (TEXT("!LineConfigEdit: Common Controls not loaded\r\n")));
		return FALSE;
	}
	
	// Ignore errors if it's already registered.
    RegisterIPClass( v_hInst );

	// Initialize the adapter Info.
	memset ((char *)&AdptInfo, 0, sizeof(AdptInfo));

	AdptInfo.szAdapterName = szAdapterName;
	// By default we enable DHCP.
	AdptInfo.fUseDHCP = TRUE;
	
	// Get the current registry data.
	_tcscpy (szTemp, TEXT("Comm\\"));
	StringCchCat (szTemp, ARRAYSIZE(szTemp), szAdapterName);
	hKey = NULL;
	hRes = RegOpenKeyEx (HKEY_LOCAL_MACHINE, szTemp, 0, 0, &hKey);
	if (ERROR_SUCCESS == hRes) {
		AdptInfo.szDisplayName[0] = TEXT('\0');
		dwSize = sizeof(AdptInfo.szDisplayName);
		RegQueryValueEx (hKey, TEXT("DisplayName"), NULL, NULL,
						 (LPBYTE)AdptInfo.szDisplayName,
						 &dwSize);
		RegCloseKey (hKey);
	}
	
	
	// Get the current registry data.
	_tcscpy (szTemp, TEXT("Comm\\"));
	StringCchCat (szTemp, ARRAYSIZE(szTemp), szAdapterName);
	StringCchCat (szTemp, ARRAYSIZE(szTemp), TEXT("\\Parms\\TcpIp"));

	hKey = NULL;
	hRes = RegOpenKeyEx (HKEY_LOCAL_MACHINE, szTemp, 0, 0, &hKey);
	if (ERROR_SUCCESS == hRes) {
		// Read the saved data in.
		dwSize = sizeof(DWORD);
		hRes = RegQueryValueEx (hKey, TEXT("EnableDHCP"), 0, &dwType,
								(LPBYTE)&AdptInfo.fUseDHCP,
								&dwSize);
		if ((ERROR_SUCCESS == hRes) && (!AdptInfo.fUseDHCP)) {
			GetMultiSZAddr (hKey, TEXT("IpAddress"),
							&AdptInfo.IPAddr, NULL);
			GetMultiSZAddr (hKey, TEXT("Subnetmask"),
							&AdptInfo.SubnetMask, NULL);
			GetMultiSZAddr (hKey, TEXT("DefaultGateway"),
							&AdptInfo.Gateway, NULL);
		}
		GetMultiSZAddr (hKey, TEXT("DNS"), &AdptInfo.DNSAddr,
						&AdptInfo.DNSAltAddr);
		GetMultiSZAddr (hKey, TEXT("WINS"), &AdptInfo.WINSAddr,
						&AdptInfo.WINSAltAddr);			
	} else {
		DEBUGMSG (ZONE_NETWORK, (TEXT(" AdapterIPProperties: Unable to open key, using default\r\n")));
	}

	// Save copy of Adptinfo to detect changes
	memcpy(&OldAdptInfo, &AdptInfo, sizeof (AdptInfo));

	// Ok now set up property sheet info.
	psp[0].dwSize = sizeof(PROPSHEETPAGE);
	psp[0].dwFlags = PSP_USETITLE;
	psp[0].hInstance = v_hInst;
	psp[0].pszTemplate = (cxScreen >= 480) ? MAKEINTRESOURCE(IDD_NETWORK_IP) :
						 MAKEINTRESOURCE(IDD_NETWORK_IP2);
	psp[0].pszIcon = NULL;
	psp[0].pfnDlgProc = NetworkIPDlgProc;
	LoadString(v_hInst, IDS_NETWORK_IPADDR, szCaption1,
			   sizeof(szCaption1)/sizeof(szCaption1[0]));
	psp[0].pszTitle = szCaption1;
	psp[0].lParam = (LPARAM)&AdptInfo;
	psp[0].pfnCallback = NULL;

	psp[1].dwSize = sizeof(PROPSHEETPAGE);
	psp[1].dwFlags = PSP_USETITLE;
	psp[1].hInstance = v_hInst;
	psp[1].pszTemplate = (cxScreen >= 480) ?
						 MAKEINTRESOURCE(IDD_NETWORK_NAMESERV) :
						 MAKEINTRESOURCE(IDD_NETWORK_NAMESERV2);
	psp[1].pszIcon = NULL;
	psp[1].pfnDlgProc = NetworkNameServDlgProc;
	LoadString(v_hInst, IDS_NETWORK_NAMESERV, szCaption2,
			   sizeof(szCaption2)/sizeof(szCaption2[0]));
	psp[1].pszTitle = szCaption2;
	psp[1].lParam = (LPARAM)&AdptInfo;
	psp[1].pfnCallback = NULL;

	psh.dwSize     = sizeof(PROPSHEETHEADER);
	psh.dwFlags    = PSH_PROPSHEETPAGE | PSH_USECALLBACK;
	psh.hwndParent = hWndOwner;
	psh.hInstance  = v_hInst;
	psh.pszIcon    = NULL;
	
	LoadString (v_hInst, IDS_NETWORK_IP_TITLE, szTemp,
				ARRAYSIZE(szTemp));
	wsprintf (szTitle, szTemp, AdptInfo.szDisplayName[0] ?
			  AdptInfo.szDisplayName : AdptInfo.szAdapterName);
	psh.pszCaption = (LPTSTR) szTitle;
	psh.nPages     = 2;
	psh.ppsp       = (LPCPROPSHEETPAGE) psp;
	psh.pfnCallback = AdapterPropSheetProc;
	psh.nStartPage = 0;

	CallCommCtrlFunc(PropertySheetW)(&psh);

	// Did they hit OK?	 Also, check whether we had settings before (hKey non NULL)
	// and whether the user changed anything.  If not, return FALSE so that connmc
	// knows not to rebind the interface.
	OldAdptInfo.fSaved = 1;  // To allow memcmp below
	if (AdptInfo.fSaved && 
		((hKey == NULL) || memcmp(&OldAdptInfo,&AdptInfo,sizeof(AdptInfo)))) {
		DEBUGMSG (ZONE_NETWORK,
				  (TEXT(" AdapterIPProperties: Save settings\r\n")));
		// Did key exist before?
		if (hKey == NULL) {
			// Create it.
			_tcscpy (szTemp, TEXT("Comm\\"));
			StringCchCat (szTemp, ARRAYSIZE(szTemp), szAdapterName);
			StringCchCat (szTemp, ARRAYSIZE(szTemp), TEXT("\\Parms\\TcpIp"));
			hRes = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szTemp, 0, NULL,
								   REG_OPTION_NON_VOLATILE, 0, NULL,
								   &hKey, &dwDisp);
			if (hRes != ERROR_SUCCESS) {
				DEBUGMSG (ZONE_ERROR|ZONE_NETWORK,
						  (TEXT("!Unable to create reg key '%s'\r\n"),
						   szTemp));
			}
		}
		// Just in case the create failed.
		if (hKey != NULL) {
			fRetVal = TRUE;
			// I'd like 1 to mean TRUE.
			if (AdptInfo.fUseDHCP) {
				AdptInfo.fUseDHCP = 1;
			}
			RegSetValueEx (hKey, TEXT("EnableDHCP"), 0, REG_DWORD,
						   (char *)&(AdptInfo.fUseDHCP), sizeof(DWORD));

			IPAddrToStr (szTemp, AdptInfo.IPAddr);
			Len = _tcslen(szTemp)+1;
			szTemp[Len++] = TEXT('\0');
			RegSetValueEx (hKey, TEXT("IpAddress"), 0, REG_MULTI_SZ,
						   (char *)szTemp, sizeof(TCHAR)*Len);
			
			IPAddrToStr (szTemp, AdptInfo.SubnetMask);
			Len = _tcslen(szTemp)+1;
			szTemp[Len++] = TEXT('\0');
			RegSetValueEx (hKey, TEXT("Subnetmask"), 0, REG_MULTI_SZ,
						   (char *)szTemp, sizeof(TCHAR)*Len);
			
			IPAddrToStr (szTemp, AdptInfo.Gateway);
			Len = _tcslen(szTemp)+1;
			szTemp[Len++] = TEXT('\0');
			RegSetValueEx (hKey, TEXT("DefaultGateway"), 0, REG_MULTI_SZ,
						   (char *)szTemp, sizeof(TCHAR)*Len);

			Len = 0;
			if (AdptInfo.DNSAddr) {
				IPAddrToStr (szTemp, AdptInfo.DNSAddr);
				Len = _tcslen(szTemp)+1;
				if (AdptInfo.DNSAltAddr) {
					IPAddrToStr (szTemp+Len, AdptInfo.DNSAltAddr);
					Len += _tcslen(szTemp+Len)+1; // allow for NULL character added by IPAddrToStr
				}
				szTemp[Len++] = TEXT('\0');
			} else if (AdptInfo.DNSAltAddr) {
				IPAddrToStr (szTemp, AdptInfo.DNSAltAddr);
				Len = _tcslen(szTemp)+1;
				szTemp[Len++] = TEXT('\0');
			}
			if (Len) {
				RegSetValueEx (hKey, TEXT("DNS"), 0, REG_MULTI_SZ,
							   (char *)szTemp, sizeof(TCHAR)*Len);
			} else {
				RegSetValueEx (hKey, TEXT("DNS"), 0, REG_MULTI_SZ,
							   (char *)TEXT(""), sizeof(TCHAR));
			}
			
			
			Len = 0;
			if (AdptInfo.WINSAddr) {
				IPAddrToStr (szTemp, AdptInfo.WINSAddr);
				Len = _tcslen(szTemp)+1;
				if (AdptInfo.WINSAltAddr) {
					IPAddrToStr (szTemp+Len, AdptInfo.WINSAltAddr);
					Len += _tcslen(szTemp+Len)+1; // allow for NULL character added by IPAddrToStr
				}
				szTemp[Len++] = TEXT('\0');
			} else if (AdptInfo.WINSAltAddr) {
				IPAddrToStr (szTemp, AdptInfo.WINSAltAddr);
				Len = _tcslen(szTemp)+1;
				szTemp[Len++] = TEXT('\0');
			}
			if (Len) {
				RegSetValueEx (hKey, TEXT("WINS"), 0, REG_MULTI_SZ,
							   (char *)szTemp, sizeof(TCHAR)*Len);
			} else {
				RegSetValueEx (hKey, TEXT("WINS"), 0, REG_MULTI_SZ,
							   (char *)TEXT(""), sizeof(TCHAR));
			}	
		}
	} else {
		DEBUGMSG (ZONE_NETWORK, (TEXT("Don't save settings\r\n")));
	}

	if (hKey)
		RegCloseKey (hKey);
	
	DEBUGMSG (ZONE_NETWORK|ZONE_INTERFACE|ZONE_FUNCTION,
			  (TEXT("-AdapterIPProperties: Returning %d\r\n"), fRetVal));
	return fRetVal;
}

