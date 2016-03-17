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

wzcuip.h

Abstract:  
    Private defs for handling wireless LAN adapter config.
Functions:


Notes: 


--*/

#include "eapol.h"
#include "wzcsapi.h"

// flags used to select state & item images
#define WZCIMG_PREFR_NOSEL     0    // empty check box
#define WZCIMG_PREFR_SELECT    1    // checked check box
#define WZCIMG_INFRA_AIRING    2    // infra icon
#define WZCIMG_INFRA_ACTIVE    3    // infra icon + blue circle
#define WZCIMG_INFRA_SILENT    4    // infra icon + red cross
#define WZCIMG_ADHOC_AIRING    5    // adhoc icon
#define WZCIMG_ADHOC_ACTIVE    6    // adhoc icon + blue circle
#define WZCIMG_ADHOC_SILENT    7    // adhoc icon + red cross

// flags indicating various operational actions.
// flags are used in:
//   AddUniqueConfig()
//   RefreshListView()
#define WZCADD_HIGROUP     0x00000001   // add in front of its group
#define WZCADD_OVERWRITE   0x00000002   // overwrite data
#define WZCOP_VLIST        0x00000004   // operate on the visible list
#define WZCOP_PLIST        0x00000008   // operate on the preferred list
#define WZCOP_SELCONNECTED 0x00000010   // select the associated network when finished

typedef struct _WLAN_CONFIG_LISTENT {
    struct _WLAN_CONFIG_LISTENT *pPrev, *pNext;
    INT                 nListIndex;           // index of the entry in the list
    DWORD               dwFlags;              // WZC_DESCR* flags
    WZC_WLAN_CONFIG     wzcConfig;            // all WZC configuration
} WLAN_CONFIG_LISTENT, *PWLAN_CONFIG_LISTENT;

// flags used in dwFlags
// the entry is preferred (user defined)
#define WZC_DESCR_PREFRD    0x00000001
// the entry is currently visible in the air
#define WZC_DESCR_VISIBLE   0x00000002
// the entry is currently active (the one plumbed to the adapter)
#define WZC_DESCR_ACTIVE    0x00000004

typedef struct _WLAN_INFO {
    INTF_ENTRY_EX  IntfEntryEx;
    DWORD       dwOIDFlags;

    HWND    hwndVLV;      // list ctrl holding the visible configurations
    HWND    hwndPLV;      // list ctrl holding the preferred configurations
    HWND    hbtnConnect;  // "Connect" button
    HWND	hbtnViewLog;  // "View Log" button
    HWND    hbtnRem;      // "Remove" button
    HWND    hbtnUp;       // "Up" button
    HWND    hbtnDown;     // "Down" button
    HWND    hbtnAdvanced; // "Advanced" button
    HWND	hwndNetAccess;// "Network Access" combo box
    HWND	hwndPopupChk; // Check box for poping up ethman

    // Handle to the images
    HIMAGELIST  hImgs;    // list items images

    // Current infrastructure mode
    DWORD       dwCtlFlags;

    // internal lists
    PWLAN_CONFIG_LISTENT pHdVList;  // list of visible configs
    PWLAN_CONFIG_LISTENT pHdPList;  // list of preferred configs

} WLAN_INFO, *PWLAN_INFO;

typedef DWORD (*PFN_WZCQueryInterfaceEx)(LPWSTR pSrvAddr,DWORD dwInFlags,PINTF_ENTRY_EX pIntf,LPDWORD pdwOutFlags);
typedef DWORD (*PFN_WZCSetInterfaceEx)(LPWSTR pSrvAddr,DWORD dwInFlags,PINTF_ENTRY_EX pIntf,LPDWORD pdwOutFlags);
typedef DWORD (*PFN_WZCRefreshInterfaceEx)(LPWSTR pSrvAddr,DWORD dwInFlags,PINTF_ENTRY_EX pIntf,LPDWORD pdwOutFlags);
typedef DWORD (*PFN_WZCEnumEapExtensions)(DWORD *pdwNumOfExtensions,PEAP_EXTENSION_INFO *ppEapExtensions);
typedef DWORD (*PFN_WZCDeleteIntfObjEx)(PINTF_ENTRY_EX pIntf);
typedef DWORD (*PFN_WZCQueryContext)(LPWSTR pSrvAddr,DWORD dwInFlags,PWZC_CONTEXT pContext,LPDWORD pdwOutFlags);
typedef DWORD (*PFN_WZCSetContext)(LPWSTR pSrvAddr,DWORD dwInFlags,PWZC_CONTEXT pContext,LPDWORD pdwOutFlags);
typedef VOID  (*PFN_WZCPassword2Key)(PWZC_WLAN_CONFIG pwzcConfig, LPCSTR cszPassword);


extern PFN_WZCQueryInterfaceEx  pfnWZCQueryInterfaceEx;
extern PFN_WZCSetInterfaceEx      pfnWZCSetInterfaceEx;
extern PFN_WZCRefreshInterfaceEx  pfnWZCRefreshInterfaceEx;
extern PFN_WZCEnumEapExtensions pfnWZCEnumEapExtensions;
extern PFN_WZCDeleteIntfObjEx     pfnWZCDeleteIntfObjEx;
extern PFN_WZCQueryContext		pfnWZCQueryContext;
extern PFN_WZCSetContext		pfnWZCSetContext;
extern PFN_WZCPassword2Key      pfnWZCPassword2Key;


LRESULT CALLBACK NetworkWLanPropsDlgProc (HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK NetworkWLanAdvancedDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


BOOL IsWPAAESCapable(PINTF_80211_CAPABILITY pCap);
BOOL InitZeroConfig(void);
BOOL GetWirelessInfo(WCHAR *szAdapterName, WLAN_INFO *pWLanInfo);
BOOL SetWirelessInfo(WLAN_INFO *pWLanInfo);

BOOL ConfigMatch(PWZC_WLAN_CONFIG pCfg1, PWZC_WLAN_CONFIG pCfg2);

// ZeroConfigUI prototypes used by Quick config dialog
DWORD FillVisibleList(PWLAN_INFO pWLanInfo, PWZC_802_11_CONFIG_LIST pwzcVList);
DWORD FillPreferredList(PWLAN_INFO pWLanInfo,PWZC_802_11_CONFIG_LIST pwzcPList);
DWORD FillCurrentConfig(PWLAN_INFO pWLanInfo, PINTF_ENTRY_EX pIntf);
AddUniqueConfig(PWLAN_INFO pWLanInfo, DWORD dwOpFlags, DWORD dwEntryFlags, PWZC_WLAN_CONFIG pwzcConfig, WLAN_CONFIG_LISTENT **ppNewNode);
DWORD SavePreferredConfigs(PWLAN_INFO pWLanInfo, PINTF_ENTRY_EX pIntf);
void FreeConfigListEnt(PWLAN_CONFIG_LISTENT pListEnt);
BOOL IsConfigInList(WLAN_CONFIG_LISTENT *pHdList, PWZC_WLAN_CONFIG pwzcConfig, WLAN_CONFIG_LISTENT **ppMatchingConfig);


////////////////////////////////////////////////////////////////////////
// WLAN_CFG_PROPS related stuff
//
// flags used for WLAN_CFG_PROPS dwFlags
#define		WZCDLG_PROPS_RWALL    0x000000ff    // enable all settings for writing
#define		WZCDLG_PROPS_RWSSID   0x00000001    // enable SSID for writing
#define		WZCDLG_PROPS_RWINFR   0x00000002    // enable Infrastructure Mode for writing
#define		WZCDLG_PROPS_RW8021X  0x00000020    // enable 802.1x parameters for editing
#define		WZCDLG_PROPS_ADD      0x00000100    // Adding a new network
#define		WZCDLG_PROPS_CONF     0x00000200    // Configure a network
#define		WZCDLG_PROPS_CONNECT  0x00000400    // First time connecting to this network
#define		WZCDLG_PROPS_NODUPS   0x00001000    // check and prevent duplicates (m_pConfigList must be set in advance)
#define		WZCDLG_PROPS_DEFOK    0x00002000    // "OK" = defpushbutton (otherwise, "Cancel"=defpushbutton)


// utility macro to convert a hexa digit into its value
#define HEX(c)  ((c)<='9'?(c)-'0':(c)<='F'?(c)-'A'+0xA:(c)-'a'+0xA)

// defines used for the valid WEP Key indices
#define WZC_WEPKIDX_NDIGITS 16  // more than we ever need
#define WZC_WEPKIDX_MIN     1
#define WZC_WEPKIDX_MAX     4

typedef struct _WLAN_CFG_PROPS 
{
    UINT IDD; 
    BOOL bKMatTouched; // indicator whether the wep key material has been touched

    // handle to the dialog
    HWND        hwndDlg;
    // Handles to all the UI controls 
    HWND        hwndEdSSID;   // "Service Set Identifier:" edit
    HWND        hwndChkAdhoc; // "Adhoc" vs "Infra" checkbox
    HWND        hwndCbAuthenticate; // Authenication combo box
    HWND        hwndCbKLen;   // "Key length" combo
    HWND        hwndCbKFmt;   // "Key format" combo
    HWND        hwndCbEncrypt;    // Encryption combo box
    // wep key related controls
    HWND        hwndUseHardwarePW; // "Use password from network hardware" check box
    HWND        hwndWEPKMatBtn; // "Edit WEP key" button
    HWND        hwndEdKMat;   // "Key material" edit
    HWND        hwndEdKIdx;   // "Key index" edit

    // 802.1x controls
    HWND        hwndChkEnable8021x; // "Enable 802.1x auth"
    HWND        hwndCbEAPType;      // "EAP Type combobox"
    HWND        hwndBtnEAPProps;     // EAP Provider properties

	// Did the user press ok or cancel.  This is used to signal modeless dialogs.
    BOOL		fDlgOk;

    // Internal flags
    DWORD       dwFlags;

    // Nic capabilities
    DWORD		            dwNicCapabilities;
    BOOL                    bWPAAESCapable; // TRUE when WPA supports AES.

    // 802.1x state 
    DWORD                   dwNumEapExt;    // number of EAP extensions in system
    PEAP_EXTENSION_INFO     pEapExtInfo;    // Pointer to array describing available EAP extensions

    // wzc configuration settings
    WZC_WLAN_CONFIG         wzcConfig;
    
    // List of configs to be checked if WZCDLG_PROPS_NODUPS is provided
    WLAN_CONFIG_LISTENT    *pConfigList;

} WLAN_CFG_PROPS, *PWLAN_CFG_PROPS;


void InitializeCfgProps(WLAN_CFG_PROPS *pWCP, DWORD dwFlags, UINT nDlgId);

// Max string for combo box text, etc
#define MAX_WZC_UI_STR 512
