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
#ifndef _QUICKCONFIGUI_H_
#define _QUICKCONFIGUI_H__

#include <windows.h>
#include <windowsx.h>
#include <types.h>
#include "iptypes.h"
#include "iphlpapi.h"
#include "resource.h"
#include "netui.h"
#include "netuip.h"
#include "commctrl.h"
#include "wcommctl.h"
#include "wzcuip.h"
#include "memory.h"
#include "ndis.h"
#include "nuiouser.h"
#include "msgqueue.h"
#include "wzcmsq.h"
#include "wzcsapi.h"

#define WM_WZCSYSTRAYICON				(WM_APP+100)
#define WZCQC_RFSH_TIMEOUT				2000
#define WZC_RFSH_TIMEOUT				3000
#define WZC_RFSH_ASSOCIATED_TIMEOUT		30000
#define IPQC_RFSH_TIMEOUT				2000
#define	WZC_SIGNAL_STRENGTH_TIMEOUT		1000
#define WZC_STATUS_UPDATE				500
#define WZC_SIGNAL_STRENGTH_SAMPLES		5
#define STRING_BUFFER_MAX				512
#define ASSOCIATE_WZC_MAX_RETRIES		24
#define MAX_IP_FIELD_WIDTH				30
#define MAX_WZC_SIGNAL_LEVEL_STR		32
#define MAX_WZCSSID_STR					50
#define MAX_WZCPHYSADDR_STR				40
#define MAX_WZCIPCFGMISC_STR			50
#define MAX_ADAPTERNAME_STR             50
#define MAX_WZC_STATUS					(35+MAX_SSID_LEN)
#define WZCQCFG_LOCK					EnterCriticalSection(&g_csLock)
#define WZCQCFG_UNLOCK					LeaveCriticalSection(&g_csLock)
#define QCFG_WIRELESS					0x2
#define QCFG_WIRED						0x4
#define DEFAULT_MAX_MESSAGES			16
#define WZC_MSGQUEUE_NAME				TEXT("WzcEventLoggingQueue")
#define MAX_PREFERRED_STR				15
#define MAX_ADDNEW_STR					15
#define MAX_CONTEXT_STR					15
#define MAX_SSID_LEN_UI					(MAX_SSID_LEN + MAX_PREFERRED_STR + 1)

#define NUM_THREADS						3
#define THREAD_UPDATE_STATUS			0
#define THREAD_SIGNAL_STRENGTH			1
#define THREAD_REFRESH_WZC				2


// Systray Icon struct for Quick Config dialog
typedef struct _SystrayIcon
{
	TCHAR 				ptcDeviceName[MAX_PATH];			// Name of the device
	TCHAR 				ptcFriendlyName[MAX_PATH];			// Name of the device
	PTCHAR				ptcLastWZCStatus;					// The last status string of zero config
	TCHAR				szActiveWZCNetwork[MAX_SSID_LEN+1];	// Last active network
	DWORD				dwActiveWZCNetworkInfraMode;		// Last active network's infrastructure mode
	DWORD				dwWZCState;							// Current state of zero config
	BOOL				fIPState;							// Current state of IP (connected or not)
	WLAN_INFO 			WLanInfo;							// Zero config structure
	DWORD				dwUIThread;							// Thread ID of the adapter
	HWND				hMainWnd;							// Window that gets notifications
	HWND				hPropSheet;							// Property Sheet
	HWND 				hIPPage;							// Property page HWND for IP page
	HWND				hWZCPage;							// Property page HWND for ZC page
	BOOL				fWireless;	 			 			// Whether the systray icon represents a wireless device
	BOOL				fDialogVisible;						// Dialog's visibility
	HANDLE				hThreads[NUM_THREADS];				// Thread handles
	HANDLE				hCloseWZCDialogEvent;				// Event fired when dialog is closed
	NOTIFYICONDATA		*IconData;     						// Icon notification data
	struct _SystrayIcon	*next;          			  		// Next item in the list
}SystrayIcon;


/* ZeroConfig Quick Config Message Queue for logging */

#define LOG_STATUS_SIZE			(MAX_WZC_STATUS + 60)
#define WZCLOG_ENTRIES			15
#define WZCLOG_INACTIVE			0x0
#define WZCLOG_ACTIVE			0x1

typedef struct _WZCLogMsgQueue
{
	TCHAR				pszList[WZCLOG_ENTRIES][LOG_STATUS_SIZE];
	DWORD				dwOldMsgIdx;
	DWORD				dwNewMsgIdx;
	DWORD				dwState;
	CRITICAL_SECTION	Lock;
	HANDLE				hThread;
}
WZCLogMsgQueue;


// ZeroConfig Logging dialog functions
BOOL CALLBACK WzcLoggingDialogProc(HWND hwndDlg, UINT 	uMsg, WPARAM	wParam, LPARAM	lParam);
HRESULT AddLoggingMsg(TCHAR* ptcDeviceName, UCHAR* pucMacAddr, FILETIME ft, TCHAR* ptcMessage);
DWORD WINAPI WzcLoggingThread(LPVOID pv);
HRESULT	InitializeLoggingQueue();
void UnloadLoggingQueue();
void UpdateWZCLogView();
DWORD GetWZCLogState();
void SaveStateToRegistry();
void LoadStateFromRegistry();
void InitLoggingCS(void);
void DeleteLoggingCS(void);
void OpenLoggingWindow(void);
    

// General Quick Config dialog functions
HRESULT GetDeviceSystrayIcon(HWND hWnd, SystrayIcon **pSysIcon);
HRESULT GetDeviceSystrayIconByName(PTCHAR ptcDeviceName, SystrayIcon **pSysIcon);
HRESULT CreatePropertySheet(HWND *phWnd, HWND hWndParent, PTCHAR ptcDeviceName, BOOL fWireless);
HRESULT ClosePropSheetDialog(HWND hWnd);
HRESULT WINAPI ClosePropSheetDialogIfReady(HWND hWnd);
HRESULT WINAPI AddNetUISystrayIcon(PTCHAR ptcDeviceName, BOOL fWireless, HWND *phWnd);
HRESULT WINAPI RemoveNetUISystrayIcon(PTCHAR ptcDeviceName);
BOOL WINAPI IsPropSheetDialogMessage(HWND hWnd, MSG* pMsg);
void InitializeWZCQCFG(void);
void DeleteWZCQCFG(void);
void WZCMiniMsgPump(HWND hWnd);
DWORD GetFriendlyDeviceName(TCHAR* szDeviceName, TCHAR* szFriendlyName);


// Zero Config dialog functions
BOOL CALLBACK WZCQuickCfgDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI SignalStrengthThread(LPVOID pv);
DWORD WINAPI RefreshZeroConfigThread(LPVOID pv);
DWORD WINAPI UpdateStatusThread(LPVOID pv);
void OnConnect(HWND hwndDlg);
void OnConfigureNetwork(HWND hwndDlg);
void OnDeleteNetwork(HWND hwndDlg);
void OnListRClick(HWND hwndDlg);
void OnListDblClick(HWND hwndDlg);
void OnAdvanced(HWND hwndDlg);
BOOL SetWZCSignalStrength(HWND hWnd, INT iSignalStrength);
HRESULT RefreshWirelessInfo(SystrayIcon* pSysIcon);
void OnAddNewThread(SystrayIcon* pSysIcon);
HRESULT	RefreshWZCButtons(HWND hwndDlg);
void HandleWZCMessage(LOG_ENTRY* pLogEntry, BOOL fShowStatus);
void OnInitWZCDialog(HWND hwndDlg);
void OnCloseWZCDialog(hwndDlg);
void RefreshWZCListView(SystrayIcon* pSysIcon);
void WINAPI UpdateConnectionStatus(PTCHAR ptcDeviceName, BOOL fConnected);
HRESULT	GetSignalStrength(PTCHAR ptcDeviceName, INT* piSignalStrength);
BOOL GetListConfigBySsid(PWLAN_CONFIG_LISTENT pHdList, NDIS_802_11_SSID *pSsid, NDIS_802_11_NETWORK_INFRASTRUCTURE NetInfra, WLAN_CONFIG_LISTENT **ppMatchingConfig);
void TranslateMsgToAdhoc(PWCHAR szMessage);
void DisplayWZCStatus(HWND hwndDlg, PTCHAR ptcStatus);
BOOL IsPreferredListEmpty(SystrayIcon* pSysIcon);
void UpdateToConnected();
BOOL GetSelectedConfig(HWND hwndListView, PWLAN_CONFIG_LISTENT* pConfig, INT* piSelected);
DWORD InsertLVItem(HWND hwndLV, LVITEM lvi);
void RefreshPreferredLV(PWLAN_INFO pWLanInfo);
DWORD DisplayInitialSignalStrength(HWND hwnd);
BOOL SelectAssociatedNetwork(HWND hwndLV);
BOOL IsNetworkInList(WLAN_CONFIG_LISTENT *pHdList, WCHAR* szConfigName, NDIS_802_11_NETWORK_INFRASTRUCTURE InfraMode);
INT IsConfigInLV(PWLAN_INFO pWLanInfo, PWLAN_CONFIG_LISTENT pCrt);
NDIS_802_11_NETWORK_INFRASTRUCTURE GetInfraMode(INT iImage);
void OnWZCOk(HWND hwndDlg);
DWORD AddListConfigsToCache(HWND hwndDlg);
BOOL AreNetworksCached(PWLAN_INFO pWLanInfo);

// WzcPopup.c dialog functions
DWORD SetDialogPopup(BOOL fPopup, DWORD dwTimeoutSecs);
DWORD GetDialogPopup(BOOL* pfPopup, DWORD* pdwTimeoutSecs);
DWORD WZCCacheAdd(CHAR* szNetwork, DWORD dwInfraMode);
BOOL IsInWZCCache(CHAR* szNetwork, DWORD dwInfraMode);
void RefreshWZCCache();
void WZCCacheClear();


// IPCfg dialog functions
BOOL CALLBACK IPQuickCfgDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK IPv6QuickCfgDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void OnInitNetDetailsDialog(HWND hwndDlg);
HRESULT	GetIPAdapterInfo(HWND hwndDlg, PIP_ADAPTER_INFO *ppAdapterInfoHead, PIP_ADAPTER_INFO *ppAdapterInfo);
HRESULT RefreshIPCfgDialog(HWND hwndDlg, BOOL fRenew);
void TimeToFileTime( time_t t, LPFILETIME pft );

// Global data
extern CRITICAL_SECTION 	g_csLock;
extern SystrayIcon*			g_pSystrayIconsHead;
extern HANDLE				g_hNdisUio;
extern HANDLE				g_hStopUpdateStatus;
extern WCHAR				g_wszConnect[MAX_CONTEXT_STR];
extern WCHAR				g_wszConfigure[MAX_CONTEXT_STR];
extern WCHAR				g_wszDelete[MAX_CONTEXT_STR];
extern BOOL					g_fPopup;
extern BOOL					g_fCheckAdhocPackets;

#endif
