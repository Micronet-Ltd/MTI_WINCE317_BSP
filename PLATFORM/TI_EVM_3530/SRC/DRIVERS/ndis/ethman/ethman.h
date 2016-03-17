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
#ifdef DEBUG

DBGPARAM dpCurSettings = 
{
    TEXT("ETHMAN"), 
 {
  TEXT("Init"),
  TEXT("NdisUIO"),
  TEXT("WZC"),  
  TEXT("Undefined"),
  TEXT("Undefined"),
  TEXT("Undefined"),
  TEXT("Undefined"),
  TEXT("Undefined"),
  TEXT("Undefined"),
  TEXT("Undefined"),  
  TEXT("Undefined"),
  TEXT("Undefined"),
  TEXT("Undefined"),  
  TEXT("Undefined"),  
  TEXT("Warning"),
  TEXT("Error") 
 },
 
    0x0000C000
};


// Debug ZONE defines
#define ZONE_INIT			DEBUGZONE(0)
#define ZONE_UIO			DEBUGZONE(1)
#define ZONE_WZC			DEBUGZONE(2)
#define ZONE_WARN			DEBUGZONE(14)
#define ZONE_ERROR			DEBUGZONE(15)

#endif

#define WM_WZCSYSTRAYICON		(WM_APP+100)
#define	MAX_DEVICE_NAME			50

typedef HRESULT (*PFN_AddNetUISystrayIcon) (PTCHAR ptcDeviceName, BOOL fWireless, HWND *phWnd);
typedef HRESULT (*PFN_RemoveNetUISystrayIcon) (PTCHAR ptcDeviceName);
typedef BOOL (*PFN_IsPropSheetDialogMessage) (HWND hWnd, MSG* pMsg);
typedef void (*PFN_UpdateConnectionStatus)(PTCHAR ptcDeviceName, BOOL fConnected);
typedef HRESULT (*PFN_ClosePropSheetDialogIfReady) (HWND hWnd);


typedef struct _Adapter
{
    TCHAR               ptcDeviceName[MAX_DEVICE_NAME];
    HANDLE              hAlive;
    HANDLE              hUIThread;
    HANDLE              hIPThread;
    BOOL                fWireless;
    DWORD               dwRefCount;
    struct _Adapter*    pNext;
    struct _Adapter*    pPrev;
}Adapter;

// Function prototypes
HRESULT HandleCurrentDevices(BOOL fWirelessModuleReady);
HRESULT ProcessNdisUioNotifications(void);
DWORD WINAPI DoWaitNotifThread(LPVOID pv);
DWORD WINAPI SystrayIconThread(LPVOID pv);
DWORD WINAPI IPNotificationThread(LPVOID pv);
void UpdateConnectionStatus(PTCHAR ptcDeviceName);
HRESULT RemoveSystrayIcon(PTCHAR pszDeviceName);
BOOL IsDeviceWireless(PTCHAR ptcDeviceName);
BOOL IsDeviceBTPAN(PTCHAR ptcDeviceName);
HRESULT AddSystrayIcon(PTCHAR ptcDeviceName, BOOL fWireless);
HRESULT AddAdapter(Adapter** ppAdapter, PTCHAR ptcDeviceName, BOOL fWireless);
void RemoveAdapter(Adapter* pAdapter);


// Definitions for Ndisuio
#define	ERROR_INVALID_ARGUMENT			0xE0000001
#define	ERROR_INVALID_MAC_ADDR			0xE0000002
#define	ERROR_MAC_ADDRESS_MUST_FOLLOW	0xE0000003
#define	ERROR_INVALID_NO_OF_PACKETS		0xE0000004
#define	ERROR_NEED_NO_OF_PACKETS		0xE0000005
#define	ERROR_NUMBER_OF_PACKET_TOO_BIG	0xE0000006
#define	ERROR_INVALID_PACKET_SIZE		0xE0000007
#define	ERROR_PACKET_SIZE_TOO_BIG		0xE0000008
#define	ERROR_NEED_PACKET_SIZE			0xE0000009
#define	ERROR_FAILED_QUERY_MAC			0xE000000A
#define	ERROR_INVALID_ETHER_TYPE		0xE000000B
#define	ERROR_MULTIPLE_DEVICE_NAME		0xE000000C
#define	ERROR_NEED_DEVICE_NAME			0xE000000D
#define	ERROR_SETTING_ETHER_TYPE		0xE000000E
#define	ERROR_NO_RX_MEM					0xE000000F
#define	ERROR_READFILE					0xE0000010
#define	ERROR_CREATE_FILE				0xE0000011
#define	ERROR_OPEN_DEVICE				0xE0000012
#define	ERROR_SETTING_FILTER			0xE0000013
#define	ERROR_NO_MEM_FOR_WRITE			0xE0000014
#define	ERROR_NEED_DST_MAC				0xE0000015
#define	ERROR_WRITE_FILE					0xE0000016
#define	ERROR_OPEN_WRITE_DEVICE			0xE0000017
#define	ERROR_NO_COMMAND_GIVEN			0xE0000018
#define	ERROR_FAILED_CREATEMSGQUEUE		0xE0000019
#define	ERROR_REQUEST_NOTIFICATION		0xE000001A
#define	ERROR_NIC_STATISTICS_FAILED		0xE000001B
#define	ERROR_OID_MUST_FOLLOW			0xE000001C
#define	ERROR_MISSING_VALUE_TO_SET		0xE000001D



/* 
Functions necessary to export
*/

#define VALID_DEVICE_CONTEXT	0x12345678


BOOL ETM_Deinit(DWORD hDeviceContext)
{
    return TRUE;
}

DWORD ETM_Open (DWORD hDeviceContext , DWORD dwAccessCode , DWORD dwShareMode)
{
    return VALID_DEVICE_CONTEXT;
}

BOOL ETM_Close (DWORD hOpenContext)
{
    return TRUE;
}

DWORD ETM_Write (DWORD hOpenContext , LPCVOID pSourceBytes , DWORD dwNumberOfBytes)
{
    return -1;
}

DWORD ETM_Read (DWORD hOpenContext, LPVOID pBuffer, DWORD dwCount)
{
    return 0;
}

DWORD ETM_Seek (DWORD hOpenContext , long lAmount, WORD wType)
{
    return -1;
}

BOOL ETM_IOControl(DWORD hOpenContext, DWORD	dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE	pBufOut,	DWORD dwLenOut, PDWORD pdwActualOut)
{
    return TRUE;
}

