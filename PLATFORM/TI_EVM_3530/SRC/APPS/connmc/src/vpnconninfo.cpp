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
#include "conninfo.h"
#include "winsock2.h"
#include "ws2tcpip.h"

#include "vpnconninfo.h"
#include "connmc.h" // For RNM_SHOWERROR
#include "l2tpcfg.h" 

#define MAX_NETBIOS_NAME    15
#define MAX_PRESHARED_KEY   128
static const TCHAR c_szStars[] = TEXT("******");

static const WCHAR wszGoodNetBIOSSymbols[] = L"!@#$%^&\')(.-_{}~";
static const WCHAR wszConnTCPIPParmsRegistryPath[] = L"Comm\\TCPIP\\Parms";
static const WCHAR wszVPNStrictDNS[] = L"VPNStrictDNSNamesOnly";

////////////////////////
// VpnConnInfo
////////////////////////

UINT VpnConnInfo::iconID = (UINT) -1;

VpnConnInfo::VpnConnInfo(TCHAR * pszName, BOOL fL2TP) :
   RasConnInfo(pszName)
{
   VpnConnInfo::fL2TP = fL2TP;
   // This means that we are not a VPN, make ourselves one here
   if (_tcscmp(RasEntry.szDeviceType, RASDT_Vpn))
   {
      LPRASDEVINFO pRasDevInfo = NULL;
      PCTSTR pszDeviceName = (fL2TP ? L2TPDeviceNamePrefix : PPTPDeviceNamePrefix);
      DWORD dwRasDevices = GetDevices(&pRasDevInfo);
      for (DWORD i=0; (i < dwRasDevices) && pRasDevInfo; i++)
      {
         if (!_tcscmp(pRasDevInfo[i].szDeviceType, RASDT_Vpn)
            && !_tcsncmp(pRasDevInfo[i].szDeviceName, pszDeviceName, _tcslen(pszDeviceName)))
         {
            _tcscpy(RasEntry.szDeviceName, pRasDevInfo[i].szDeviceName);
            _tcscpy(RasEntry.szDeviceType, RASDT_Vpn);
            RasEntry.dwfOptions = RASEO_RequireMsEncryptedPw |
                                  RASEO_ProhibitEAP          | 
                                  RASEO_ProhibitPAP          |
                                  RASEO_ProhibitCHAP;
            if (!fL2TP)
                RasEntry.dwfOptions |=  RASEO_RequireDataEncryption;

            break;
         }
      }
      if (pRasDevInfo)
         delete [] pRasDevInfo;
   }
}

void VpnConnInfo::getTypeAsDisplayString(HINSTANCE hInstance, TCHAR * pszType,
                                         DWORD cchType) const
{
   ::LoadString(hInstance, IDS_RAS_VPN, pszType, cchType);
}

BOOL VpnConnInfo::ConfigureL2TP(HWND hParent, HINSTANCE hInstance)
{
    PROPSHEETPAGE psp[1] = {0};
    PROPSHEETHEADER psh = {0};
    TCHAR szTitle[30];

    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].hInstance = hInstance;
    psp[0].pszTemplate = MAKEINTRESOURCE(g_fPortrait?IDD_CONFIG_L2TP_P:IDD_CONFIG_L2TP);
    psp[0].pfnDlgProc = VpnConnInfo::L2TPDlgProc;
    psp[0].lParam = (LPARAM) this;

    LoadString(hInstance, IDS_L2TP_TITLE, szTitle,
              sizeof(szTitle)/sizeof(*szTitle));
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE;
    psh.hwndParent = hParent;
    psh.hInstance = hInstance;
    psh.pszCaption = szTitle;
    psh.nPages = 1;
    psh.ppsp = psp;

    return CallCommCtrlFunc(PropertySheetW)(&psh);
}

HPROPSHEETPAGE VpnConnInfo::getPropPage(UINT uPage, HINSTANCE hInstance)
{
   if (!pfnCreatePropertySheetPage)
      return NULL;

   PROPSHEETPAGE psp = {0};
   psp.dwSize = sizeof(PROPSHEETPAGE);
   psp.hInstance = hInstance;
   switch (uPage)
   {
      case 0:
         psp.pszTemplate = MAKEINTRESOURCE(g_fPortrait?IDD_VPN_0_P:IDD_VPN_0);
         psp.pfnDlgProc = VpnConnInfo::VpnConnPropSheetProc;
      break;
      default: return NULL;
   }
   psp.lParam = reinterpret_cast<LPARAM>(this);

   return (*pfnCreatePropertySheetPage)(&psp);
}

static L2TP_CONFIG_DATA g_L2TPConfigDefault = {1, L2TP_IPSEC_AUTH_SIGNATURE, 0};
BOOL CALLBACK VpnConnInfo::L2TPDlgProc(HWND hDlg, UINT uMsg,
                                                WPARAM wParam, LPARAM lParam)
{
    PROPSHEETPAGE * ppsp = reinterpret_cast<PROPSHEETPAGE *>(GetWindowLong(hDlg, GWL_USERDATA));
    VpnConnInfo * pVpnConn = NULL;
    TCHAR szPresharedKey[MAX_PRESHARED_KEY+1];
    PL2TP_CONFIG_DATA pConfigData;
    
    if (ppsp) pVpnConn = reinterpret_cast<VpnConnInfo *>(ppsp->lParam);
    
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            ::SetWindowLong(hDlg, GWL_USERDATA, lParam);
            ppsp = reinterpret_cast<PROPSHEETPAGE *>(lParam);
            ASSERT(ppsp);
            pVpnConn = reinterpret_cast<VpnConnInfo *>(ppsp->lParam);
            ASSERT(pVpnConn);
            
            // Limit the phone number length
            ::SendMessage(::GetDlgItem(hDlg, IDC_PRESHAREDKEY), EM_LIMITTEXT,
                MAX_PRESHARED_KEY, 0);
            // 
            // 
            //
            pConfigData = (PL2TP_CONFIG_DATA)pVpnConn->pDevConfig;
            if (!pConfigData || pVpnConn->cbDevConfig < sizeof(L2TP_CONFIG_DATA))
                pConfigData = &g_L2TPConfigDefault;
            if (pConfigData)
            {
                if (pConfigData->dwVersion == 1)
                {
                    if (pConfigData->dwAuthType & L2TP_IPSEC_AUTH_PRESHAREDKEY)
                    {
                        Button_SetCheck(::GetDlgItem(hDlg, IDC_RB_PRESHAREDKEY), BST_CHECKED);
                        ::EnableWindow(::GetDlgItem(hDlg, IDC_PRESHAREDKEY), TRUE);
                        if (pConfigData->cbKey && (pConfigData->dwOffsetKey + pConfigData->cbKey <= pVpnConn->cbDevConfig))
                        {
                            // don't show the preshared key
                            ::SetDlgItemTextW(hDlg, IDC_PRESHAREDKEY, c_szStars);
                            //::SetDlgItemTextW(hDlg, IDC_PRESHAREDKEY,
                            //  (PWCHAR)((PBYTE)pConfigData + pConfigData->dwOffsetKey));         
                        }
                    }
                    else
                    {
                        Button_SetCheck(::GetDlgItem(hDlg, IDC_RB_CERTAUTH), BST_CHECKED);
                    }
                }
            }
            
            return FALSE; // We will set our own focused control
        }
        break;
        
    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case IDC_RB_PRESHAREDKEY:
            case IDC_RB_CERTAUTH:
                ::EnableWindow(::GetDlgItem(hDlg, IDC_PRESHAREDKEY), 
                    Button_GetCheck(::GetDlgItem(hDlg, IDC_RB_PRESHAREDKEY)) == BST_CHECKED);
                break;
                
            default:
                break;
            }
        }
        break;
        
    case WM_NOTIFY:
        {
            DWORD cbKey;
            DWORD dwAuthType;
            PWCHAR pszPresharedKey = NULL;
            
            switch((reinterpret_cast<NMHDR *>(lParam))->code) {
            case PSN_APPLY:
                //
                // This event occurs when the user presses the "OK" button on
                // the "IPSEC Settings" dialog.
                //
                if (!pVpnConn)
                {
                    // pRasConn shouldn't be NULL, but if it is, return FALSE
                    return FALSE;
                }

                if (Button_GetCheck(::GetDlgItem(hDlg, IDC_RB_PRESHAREDKEY)) == BST_CHECKED)
                {
                    // read the pre-shared key field from the dialog
                    ::GetDlgItemTextW(hDlg, IDC_PRESHAREDKEY, szPresharedKey, MAX_PRESHARED_KEY+1);
                    if (0 == _tcscmp(szPresharedKey, c_szStars))
                    {
                        // No new preshared key was entered, so use the previously configured one.

                        // Note that pVpnConn->pDevConfig may be NULL, for example if
                        // this is a brand new connectoid being created for which there
                        // is no DevConfig yet.
                        pConfigData = (PL2TP_CONFIG_DATA)pVpnConn->pDevConfig;
                        if (pConfigData == NULL)
                        {
                            // No previously configured pre-shared key. Fail.
                            return FALSE;
                        }

                        if ((pConfigData->dwOffsetKey > pVpnConn->cbDevConfig)
                        ||  (pConfigData->cbKey > pVpnConn->cbDevConfig)
                        ||  (pConfigData->dwOffsetKey + pConfigData->cbKey > pVpnConn->cbDevConfig))
                        {
                            // The previous config data is invalid/corrupted. Fail.
                            return FALSE;
                        }

                        pszPresharedKey = (PWCHAR)((PBYTE)pConfigData + pConfigData->dwOffsetKey);
                        cbKey = pConfigData->cbKey;
                    }
                    else
                    {
                        // The user entered a new preshared key, use that.
                        pszPresharedKey = &szPresharedKey[0];
                        cbKey =   (wcslen(szPresharedKey))*sizeof(WCHAR);
                    }
                    
                    dwAuthType = L2TP_IPSEC_AUTH_PRESHAREDKEY;
                    
                }
                else
                {
                    // User wants to use certificate based authentication, not PSK.
                    cbKey = 0;
                    dwAuthType = L2TP_IPSEC_AUTH_SIGNATURE;
                }
                pConfigData = (PL2TP_CONFIG_DATA)new BYTE [sizeof(L2TP_CONFIG_DATA)+ cbKey];
                if (pConfigData)
                {
                    ZeroMemory(pConfigData, sizeof(L2TP_CONFIG_DATA));
                    pConfigData->dwVersion = 1;
                    pConfigData->dwAuthType = dwAuthType;
                    pConfigData->cbKey = cbKey;
                    pConfigData->dwOffsetKey = sizeof(L2TP_CONFIG_DATA);
                    if (cbKey)
                        memcpy((PBYTE)pConfigData+pConfigData->dwOffsetKey, pszPresharedKey, cbKey);
                    
                    if (pVpnConn->pDevConfig)
                        ZeroMemory(pVpnConn->pDevConfig,pVpnConn->cbDevConfig);
                    delete [] pVpnConn->pDevConfig;
                    pVpnConn->pDevConfig = (PBYTE)pConfigData;
                    pVpnConn->cbDevConfig = sizeof(L2TP_CONFIG_DATA) + cbKey;
                }
                SecureZeroMemory(szPresharedKey, sizeof(szPresharedKey));
                
                break;
                
            case PSN_QUERYCANCEL:
                ;
                break;
            }
        }
        
        break;
        
    default:
        return FALSE;
   }
   return TRUE;
}

BOOL CALLBACK VpnConnInfo::VpnConnPropSheetProc(HWND hDlg, UINT uMsg,
                                                WPARAM wParam, LPARAM lParam)
{
    PROPSHEETPAGE * ppsp = reinterpret_cast<PROPSHEETPAGE *>(GetWindowLong(hDlg, GWL_USERDATA));
    VpnConnInfo * pVpnConn = NULL;
    if (ppsp)
        pVpnConn = reinterpret_cast<VpnConnInfo *>(ppsp->lParam);
    
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            ::SetWindowLong(hDlg, GWL_USERDATA, lParam);
            ppsp = reinterpret_cast<PROPSHEETPAGE *>(lParam);
            ASSERT(ppsp);
            pVpnConn = reinterpret_cast<VpnConnInfo *>(ppsp->lParam);
            ASSERT(pVpnConn);
            
            // Limit the VPN hostname length.
            // Because we're storing this information in a
            // (ras.h)RASENTRY, the maximum hostname length is restricted
            // to 128 characters.  However, the maximum length for a DNS
            // FQDN is 255 bytes, so that means that some long hostnames
            // cannot be entered by the user.
            ::SendMessage(::GetDlgItem(hDlg, IDC_HOSTNAME), EM_LIMITTEXT,
                RAS_MaxPhoneNumber, 0);
            
            // Set the phone number here
            ::SetDlgItemText(hDlg, IDC_HOSTNAME,
                pVpnConn->RasEntry.szLocalPhoneNumber);         
            
            // Pnone number box gets focus by default
            ::SetFocus(::GetDlgItem(hDlg, IDC_HOSTNAME));
            
            // enable Configure button if L2TP
            ::EnableWindow(::GetDlgItem(hDlg, IDC_CONFIG), pVpnConn->fL2TP);
            
            return FALSE; // We will set our own focused control
        }
        break;
        
    case WM_COMMAND:
        {
            if (!pVpnConn)
            {
                break;
            }
            switch (LOWORD(wParam))
            {
            case IDC_CONFIG:
                pVpnConn->ConfigureL2TP(hDlg, ppsp->hInstance);
                break;
            case IDC_TCPSETTINGS:
                pVpnConn->ConfigureTcpIp(hDlg, ppsp->hInstance);
                break;
                
            case IDC_SECSETTINGS:
                pVpnConn->ConfigureSecurity(hDlg, ppsp->hInstance);
                break;
            }
        }
        break;
        
    case WM_NOTIFY:
        {
            if (!pVpnConn)
            {
                break;
            }
            LONG lReturn = 0;
            PSHNOTIFY * ppshn = (PSHNOTIFY *) lParam;
            switch (ppshn->hdr.code)
            {
            case PSN_SETACTIVE:
                {
                    PropSheet_SetWizButtons(::GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
                    
                    // Set the connection name.
                    ::SetWindowText(::GetDlgItem(hDlg, IDC_CONNLABEL), pVpnConn->pszName);
                }
                break;
                
            case PSN_WIZFINISH: // WIZFINISH doesn't send a KILLACTIVE so simulate here
            case PSN_KILLACTIVE:
                {
                    // (Actual size of buffer is RAS_MaxPhoneNumber+1)
                    ::GetDlgItemText(hDlg, IDC_HOSTNAME,
                        pVpnConn->RasEntry.szLocalPhoneNumber,
                        RAS_MaxPhoneNumber+1);
                }
                if (PSN_WIZFINISH != ppshn->hdr.code)
                    break;
                
            case PSN_APPLY:
                {
                    if (!pVpnConn) {
                        // pVpnConn shouldn't be NULL
                        return FALSE;
                    }
                    
                    // Validate the host/ip address
                    if (_T('\0') == pVpnConn->RasEntry.szLocalPhoneNumber[0])
                    {
                        ::EnableWindow(::GetParent(hDlg), FALSE);
                        ::SendMessage(::GetParent(::GetParent(hDlg)), RNM_SHOWERROR,
                            NULL, IDS_ERR_HOSTNAMEREQ);
                        ::EnableWindow(::GetParent(hDlg), TRUE);
                        ::SetFocus(::GetDlgItem(hDlg, IDC_HOSTNAME));
                        lReturn = 1;
                        break;
                    }
                    
                    if (!isValidVPNInfoString(pVpnConn->RasEntry.szLocalPhoneNumber))
                    {
                        ::EnableWindow(::GetParent(hDlg), FALSE);
                        ::SendMessage(::GetParent(::GetParent(hDlg)), RNM_SHOWERROR,
                            NULL, IDS_ERR_BADHOSTNAME);
                        ::EnableWindow(::GetParent(hDlg), TRUE);
                        ::SetFocus(::GetDlgItem(hDlg, IDC_HOSTNAME));
                        lReturn = 1;
                        break;
                    }
                    
                    pVpnConn->writeChanges();
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


BOOL VpnConnInfo::isValidVPNInfoString(WCHAR * wszValidate)
{
    BOOL bValid = FALSE;
    
    if (!wszValidate)
        return FALSE;

    // First, check for a valid numeric format.
    // (The really annoying thing about this is that the first thing
    // ::getaddrinfo does is MultiByteToWideChar(CP_ACP,...).)

    char szTargetString[NI_MAXHOST];
    if (0 != ::WideCharToMultiByte(CP_ACP, 0, wszValidate, -1,
                                   szTargetString, sizeof(szTargetString),
                                   NULL, NULL))
    {
        ADDRINFO hints, *ai;
        ::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_flags = AI_NUMERICHOST;
        bValid = NO_ERROR == ::getaddrinfo(szTargetString, "", &hints, &ai);
        ::freeaddrinfo(ai);
    }

    // Unfortunately can't use getaddrinfo to check for a valid name
    // because it does a DNS lookup which might fail for various
    // reasons even if there is a valid name.  It is too bad, because
    // presumably the stack knows how to validate DNS names &
    // NetBIOS names.

    // So see the MSDN article "Complying with Name Restrictions for
    // Hosts and Domains" for the rules for valid names.

    // If the name is 15 chars or less, then check against the valid
    // NetBIOS characters:  characters, numbers, white space, and the
    // symbols !@#$%^&')(.-_{}~

    if (!bValid && ::wcslen(wszValidate) <= MAX_NETBIOS_NAME)
    {
        WCHAR * wsz = wszValidate;
        WCHAR wc;
        while (0 != (wc = *wsz++))
        {
            if (::iswctype(wc, _ALPHA|_DIGIT|_SPACE)) continue;
            if (::wcschr(wszGoodNetBIOSSymbols, wc))  continue;
            // invalid NetBIOS character
            break;
        }
        bValid = 0 == wc;
    }
    
    // Strict DNS:  A-Za-z0-9-.
    // (except not checking maximum 63 bytes/label)
    // (and not checking that '_' only at first character in a label)

    if (!bValid)
    {
        WCHAR * wsz = wszValidate;
        WCHAR wc;
        while (0 != (wc = *wsz++))
        {
            if (L'a' <= wc && wc <= L'z') continue;
            if (L'A' <= wc && wc <= L'Z') continue;
            if (L'0' <= wc && wc <= L'9') continue;
            if (L'-' == wc)               continue;
            if (L'.' == wc)               continue;
            if (L'_' == wc)               continue;
            // invalid strict DNS character
            break;
        }
        bValid = 0 == wc;
    }

    // Slack DNS rules: Windows can be configured so that it is
    // possible to use any UTF-8 character in a name.  Maximum length
    // is 255 bytes in UTF-8.

    // Determine if non-standard slackDNS check should be done by
    // checking the registry.  Default (registry key missing) is to do
    // the slack check.  If the value is non-zero skip the slack check.

    if (!bValid)
    {
        HKEY  hKey = NULL;
        DWORD dwVPNStrictDNS = 0;
        DWORD cbVPNStrictDNS = sizeof(dwVPNStrictDNS);
        if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszConnTCPIPParmsRegistryPath, NULL, KEY_READ, &hKey))
        {
            (void)::RegQueryValueEx(hKey, wszVPNStrictDNS, NULL, NULL, (LPBYTE)&dwVPNStrictDNS, &cbVPNStrictDNS);
        }
        if (NULL != hKey)
        {            
            ::RegCloseKey(hKey);
        }
        if (0 == dwVPNStrictDNS)
        {
            // (But ::WideCharToMultiByte will never return an error on
            // conversion due to invalid characters (it will even eat misused
            // surrogates).  So there's no point in this check.)

            int cb = ::WideCharToMultiByte(CP_UTF8, 0, wszValidate, -1, NULL, 0, NULL, NULL);
                // (CE: Doesn't support default char on UTF8 conversion)
            bValid = 0 < cb && cb <= 255;
        }
    }

    return bValid;
}

