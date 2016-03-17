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

    QuickConfigUi.c

Abstract:

    This file contains functions to handle the systray icon associated
    with an ethernet device.  It also handles the dialogs which are created
    when double-clicking the systray icon.

Environment:

    WinCE

*/



#include <winsock2.h>
#include <ws2tcpip.h>
#include "QuickConfigUI.h"

#define REG_ADAPTER_NAME_MAPPINGS	_T("Comm\\AdapterNameMappings")


CRITICAL_SECTION        g_csLock;
SystrayIcon*			g_pSystrayIconsHead;
HANDLE                  g_hNdisUio = NULL;


const	WCHAR ConnTCPIPParmsRegistryPath[] = L"Comm\\TCPIP\\Parms";
const	WCHAR DisableMsgName[] = L"MicronetDisableCachedLeaseMsg";
DWORD	g_DisableCachedLeaseDialog	= 0;

/*++

Routine Name:

    TimeToFileTime

Routine Description:

    Converts a time_t struct

Arguments:

    t:
    pft:

--*/

void TimeToFileTime( time_t t, LPFILETIME pft )
{
    unsigned __int64 ui64 = (unsigned __int64)t * 10000000 + 116444736000000000;
    *pft = *(FILETIME *)&ui64;
    return;
}



/*++

Routine Name:

    InitializeWZCQCFGCriticalSection

Routine Description:

    Initialize the critical section for the quick config dialogs.

--*/

void InitializeWZCQCFG(void)
{
    InitializeCriticalSection(&g_csLock);

    InitLoggingCS();

    if(g_hNdisUio == NULL)
    {
		HKEY	hKey = 0;
		BOOL	fRetVal = FALSE;
		DWORD	dwSize, dwType = 0;
		LONG	hRes;

		hRes = RegOpenKeyEx (HKEY_LOCAL_MACHINE, ConnTCPIPParmsRegistryPath, 0, 0, &hKey);
		if (ERROR_SUCCESS == hRes) 
		{
			dwSize = sizeof(DWORD);
			hRes = RegQueryValueEx(hKey, DisableMsgName, 0, &dwType, (LPBYTE)&g_DisableCachedLeaseDialog, &dwSize);
			if(ERROR_SUCCESS != hRes)
			{
				g_DisableCachedLeaseDialog = 0;
			}
		}
		RETAILMSG(1,(L"NETUI: InitializeWZCQCFG g_DisableCachedLeaseDialog = %d\r\n", g_DisableCachedLeaseDialog));

        g_hNdisUio = CreateFile(
                           NDISUIO_DEVICE_NAME,                            //	Object name.
                           0x00,                                           //	Desired access.
                           0x00,                                           //	Share Mode.
                           NULL,                                           //	Security Attr
                           OPEN_EXISTING,                                  //	Creation Disposition.
                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,   //	Flag and Attributes..
                           (HANDLE)INVALID_HANDLE_VALUE);
    }
}



/*++

Routine Name:

    DeleteWZCQCFGCriticalSection

Routine Description:

    Delete the critical section for the quick config dialogs.

--*/

void DeleteWZCQCFG(void)
{
    DeleteCriticalSection(&g_csLock);
    DeleteLoggingCS();
    if(g_hNdisUio)
        CloseHandle(g_hNdisUio);
    if(g_hStopUpdateStatus)
        CloseHandle(g_hStopUpdateStatus);
}



/*++

Routine Name:

    QuickCfgProcPage

Routine Description:

    Dialog proc for the Property Sheet

Arguments:

    hwndDlg:	HWND of the dialog
    uMsg:		Message type
    wParam:		wParam
    lParam:		Additional data

Return Value:

    int

--*/

int CALLBACK QuickCfgProcPage(
                             HWND hwndDlg,  // handle to the property sheet dialog box
                             UINT uMsg,     // message identifier
                             LPARAM lParam)  // message parameter
{
   return 0;
} // QuickCfgProcPage



/*++

Routine Name:

    NotifyDialogProc

Routine Description:

    Dialog Proc for the Notification dialog.  This function handles all the
    messages encountered on the dialog.  This is a hidden dialog which serves
    the purpose of catching messages related to the systray icon.  The property
    sheet or property pages could not handle the necessary behaviour so this
    dialog had to be created.

Arguments:

    hWndDlg:	HWND for the dialog
    uMsg:		Message encountered on the dialog
    wParam:		Additional data passed to the dialog
    lParam:		Additional data passed to the dialog

Return Value:

    BOOL (TRUE/FALSE)

--*/

#define WM_PSCLOSE	(WM_USER+51)

BOOL CALLBACK NotifyDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL            fRetVal     = FALSE;
    HRESULT         hr          = S_OK;
    SystrayIcon*    pSysIcon    = NULL;

    switch (uMsg) {
    case WM_INITDIALOG:

        // Called when dialog is initialized
        DEBUGMSG(ZONE_MISC, (TEXT("NetUIQC - WM_INITDIALOG processed on Main Dialog.")));
        fRetVal = TRUE;
        break;

    case WM_CLOSE:

        WZCQCFG_LOCK;
        DestroyWindow(hWndDlg);
        WZCQCFG_UNLOCK;
        
        fRetVal = 0;
        break;

	case WM_PSCLOSE:

		// Destroy Property Sheet associated with this window
		DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: Destroying property sheet for systray icon: %08X"), lParam));
		DestroyWindow((HWND)lParam);
		fRetVal = 0;
		break;

    case WM_DESTROY:

        // Post Quit Message to the message pump
        PostQuitMessage(0);
        fRetVal = 0;
        break;

    case WM_SETTINGCHANGE:

		hr = GetDeviceSystrayIcon(hWndDlg, &pSysIcon);
        if (SUCCEEDED(hr) && pSysIcon)
        {
        	TCHAR	szFriendly[MAX_PATH];
        	if(GetFriendlyDeviceName(pSysIcon->ptcDeviceName, szFriendly) != ERROR_SUCCESS)
        	{
        		fRetVal = 0;
        		break;
        	}

        	_tcsncpy(pSysIcon->ptcFriendlyName, szFriendly, MAX_PATH);

        	// Update window if it is visible
			if(pSysIcon->fDialogVisible)
			{
				SetWindowText(pSysIcon->hPropSheet, pSysIcon->ptcFriendlyName);
			}

			// Update icon tip text
			wcsncpy(pSysIcon->IconData->szTip, pSysIcon->ptcFriendlyName, sizeof(pSysIcon->IconData->szTip)/sizeof(pSysIcon->IconData->szTip[0]));
			Shell_NotifyIcon(NIM_MODIFY, pSysIcon->IconData);
        }
		
		fRetVal = 0;
	 	break;

    case WM_WZCSYSTRAYICON:

        switch (lParam) {
        case WM_LBUTTONDBLCLK:

			WZCQCFG_LOCK;

            // Get the systray icon struct
            hr = GetDeviceSystrayIcon(hWndDlg, &pSysIcon);
            if (SUCCEEDED(hr) && pSysIcon) {
                if (pSysIcon->fDialogVisible == FALSE) {
                    pSysIcon->fDialogVisible = TRUE;

                    // Create the dialog
                    hr = CreatePropertySheet(&(pSysIcon->hPropSheet), hWndDlg, pSysIcon->ptcFriendlyName, pSysIcon->fWireless);
                    if (FAILED(hr)) {
                        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Could not create property sheet")));
                        WZCQCFG_UNLOCK;
                        goto exit;
                    }

                    pSysIcon->hIPPage = PropSheet_GetCurrentPageHwnd(pSysIcon->hPropSheet);
                    if (pSysIcon->fWireless && wParam == 1) {
                        //PropSheet_SetCurSel(pSysIcon->hPropSheet, pSysIcon->hWZCPage, 0);
                        if (! PropSheet_SetCurSel(pSysIcon->hPropSheet, NULL, 2))
                            PropSheet_SetCurSel(pSysIcon->hPropSheet, NULL, 1);
                        pSysIcon->hWZCPage = PropSheet_GetCurrentPageHwnd(pSysIcon->hPropSheet);
                    }

                    // Init the IPcfg dialog and pass it the HWND of he IP property page.  There is
                    // no real danger of PropSheet_GetCurrentPageHwnd returning NULL unless
                    // the user closed the dialog in the short period of time between now and the
                    // last statement.
                    RefreshIPCfgDialog(PropSheet_GetCurrentPageHwnd(pSysIcon->hPropSheet), FALSE);
                } else {
                    SetForegroundWindow(pSysIcon->hPropSheet);
                }
            }

            WZCQCFG_UNLOCK;

            fRetVal = FALSE;
            break;

        default:
            fRetVal = FALSE;
            break;
        }

    default:
        fRetVal = TRUE;
        break;
    }

    exit:
    return fRetVal;

} // NotifyDialogProc


/*++

Routine Name:

	WZCMiniMsgPump

Routine Description:

	This is a message pump used by dialogs which are popped up from the UI.  Since a
	modal dialog box would not get close notifications for when the card is ejected, we
	have to do it this way.

Arguments:

	hWnd:	Handle to the window using the message pump

--*/

void WZCMiniMsgPump(HWND hWnd)
{
	MSG msg;
	
	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(!IsDialogMessage(hWnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}


/*++

Routine Name:

    AddNetUISystrayIcon

Routine Description:

    Add an icon to the systray to indicate that a ethernet device has been added.

Arguments:

    ptcDeviceName:	String representing the name of the device
    fWireless:		Device is wireless
    phWnd:			Out parameter corresponding to the HWND

Return Value:

    HRESULT to indicate return status

--*/

HRESULT AddNetUISystrayIcon(PTCHAR ptcDeviceName, BOOL fWireless, HWND *phWnd)
{
    HRESULT             hr = S_OK;
    NOTIFYICONDATA      *nid = NULL;
    SystrayIcon         *n = NULL;
    BOOL				fWZCInit;

    // Wait for the shell to be ready
   while (WAIT_OBJECT_0 != WaitForAPIReady(SH_SHELL, 500))
            ; 

    WZCQCFG_LOCK;

    // Allocate memory for systray icon
    n = (SystrayIcon *)malloc(sizeof(SystrayIcon));
    if (n == NULL) {
        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Could not allocate memory for systray icon node")));
        hr = E_FAIL;
        goto exit;
    }

	memset(n, 0, sizeof(SystrayIcon));

    // Get Wireless Info (this method also sets the function pointers used for
    // querying zero config).  If this fails we abort everything
    if (fWireless) {
        hr = InitializeLoggingQueue();
        if (FAILED(hr)) {
            goto exit;
        }

		if (!InitZeroConfig()) {
			hr = E_FAIL;
			goto exit;
		}

		memset(&n->WLanInfo, 0, sizeof(WLAN_INFO));

		fWZCInit = TRUE; // Assume true		
        if (!GetWirelessInfo((WCHAR *) ptcDeviceName, &(n->WLanInfo))) {
        	fWZCInit = FALSE;
            DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Warning - failed to get wireless info")));
        }

        // Check if the dialog should pop up
		GetDialogPopup(&g_fPopup, NULL);
    }

    // Allocate memory for systray icon notification
    nid = (NOTIFYICONDATA *)malloc(sizeof(NOTIFYICONDATA));
    if (nid == NULL) {
        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Could not allocate memory for systray icon")));
        hr = E_FAIL;
        goto exit;
    }

    // Allocate memory for the last status message
    n->ptcLastWZCStatus = (TCHAR *)malloc(MAX_WZC_STATUS*sizeof(TCHAR));
    if (n->ptcLastWZCStatus == NULL) {
        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Out of memory")));
        hr = E_FAIL;
        goto exit;
    }

    // Create the dialog (this is an invisible dialog just to handle systray notifications
    n->hMainWnd = CreateDialog(v_hInst, MAKEINTRESOURCE(IDD_QCNOTIF), NULL, NotifyDialogProc);
    if (n->hMainWnd == NULL) {
        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Could not create Main dialog")));
        hr = E_FAIL;
        goto exit;
    }

    // Create the icon
    if (fWireless) {
        nid->hIcon = (HICON) LoadImage(v_hInst, MAKEINTRESOURCE(IDI_DISCONNECTED), IMAGE_ICON, 16, 16, 0);
        if (nid->hIcon == NULL) {
            DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Could not get NetUI icon")));
            hr = E_FAIL;
            goto exit;
        }
    } else {
        nid->hIcon = (HICON) LoadImage(v_hInst, MAKEINTRESOURCE(IDI_LANDISCONNECT), IMAGE_ICON, 16, 16, 0);
        if (nid->hIcon == NULL) {
            DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Could not get NetUI icon")));
            hr = E_FAIL;
            goto exit;
        }
    }

	_tcscpy(n->ptcDeviceName, ptcDeviceName);
	if(GetFriendlyDeviceName(n->ptcDeviceName, n->ptcFriendlyName) != ERROR_SUCCESS)
	{
		_tcscpy(n->ptcFriendlyName, n->ptcDeviceName);
	}

    nid->hWnd = n->hMainWnd;
    nid->cbSize = sizeof(NOTIFYICONDATA);
    wcsncpy(nid->szTip, n->ptcFriendlyName, sizeof(nid->szTip)/sizeof(nid->szTip[0]));
    nid->uCallbackMessage = WM_WZCSYSTRAYICON;
    nid->uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid->uID = 1001;

    // Add the icon to the tray
    if (Shell_NotifyIcon(NIM_ADD, nid) == FALSE) {
        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Error adding icon to systray: %d"), GetLastError()));
        hr = E_FAIL;
        goto exit;
    }

    n->IconData             = nid;
    n->fWireless            = fWireless;
    n->fDialogVisible       = FALSE;
    n->dwUIThread    		= GetCurrentThreadId();
    n->fIPState             = FALSE;

    // Insert the node into the linked list
    n->next = g_pSystrayIconsHead;
    g_pSystrayIconsHead = n;

    if (fWireless) {
        // Initialize the status to "Not Connected".
        LoadString(v_hInst, IDS_QC_NOT_CONNECTED, n->ptcLastWZCStatus, MAX_WZC_STATUS);

        // Initialize the active network
        wcscpy(n->szActiveWZCNetwork, TEXT(""));
        n->dwActiveWZCNetworkInfraMode = 0;

		n->hCloseWZCDialogEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		// Create the ZeroConfig Msg Thread and stop event
		if(g_hStopUpdateStatus == 0)
		{
			// First time we must create event
			g_hStopUpdateStatus	= CreateEvent(NULL, TRUE, FALSE, NULL);
		}
		else
		{
			ResetEvent(g_hStopUpdateStatus);
		}

		n->hThreads[THREAD_UPDATE_STATUS] = CreateThread(NULL, 0, UpdateStatusThread, NULL, 0, NULL);
		
		if(!g_hStopUpdateStatus || !n->hThreads[THREAD_UPDATE_STATUS])
		{
			DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Could not create thread and/or thread stop event")));
			hr = E_FAIL;
			goto exit;
		}
		
        // If the preferred list is empty then show the dialog on startup
        if (fWZCInit && g_fPopup && (IsPreferredListEmpty(n) == TRUE)) {
            PostMessage(n->hMainWnd, WM_WZCSYSTRAYICON, 1, WM_LBUTTONDBLCLK);
        }
    }

    *phWnd = n->hMainWnd;
    goto exit2;

exit:
    if (n != NULL) {
        if (n->ptcLastWZCStatus != NULL)
            free(n->ptcLastWZCStatus);
        if (n->hMainWnd)
    		DestroyWindow(n->hMainWnd);
        if (n->hCloseWZCDialogEvent)
        	CloseHandle(n->hCloseWZCDialogEvent);
	    if (n->hThreads[THREAD_UPDATE_STATUS])
	    	CloseHandle(n->hThreads[THREAD_UPDATE_STATUS]);
        free(n);
    }
    if (nid != NULL) {
    	if (nid->hIcon) {
    		DestroyIcon(nid->hIcon);
    	}
    	free(nid);
    }
    
exit2:
    WZCQCFG_UNLOCK;

    return hr;

} // AddNetUISystrayIcon



/*++

Routine Name:

    RemoveNetUISystrayIcon

Routine Description:

    Remove icon from systray to indicate that the ethernet device has been removed.

Arguments:

    ptcDeviceName:	String representing the name of the device

Return Value:

    HRESULT to indicate return status

--*/

HRESULT RemoveNetUISystrayIcon(PTCHAR ptcDeviceName)
{
    HRESULT         hr = S_OK;
    SystrayIcon     *curr = NULL;
    SystrayIcon     *prev = NULL;
    int 			i;

    WZCQCFG_LOCK;

    // Get the HWND of the specified device
    curr = g_pSystrayIconsHead;
    while (curr != NULL) {
        if (wcscmp(curr->ptcDeviceName, ptcDeviceName) == 0) {
            if (prev == NULL) {
                g_pSystrayIconsHead = curr->next;
            } else {
                prev->next = curr->next;
            }

            break;
        }
        prev = curr;
        curr = curr->next;
    }

	WZCQCFG_UNLOCK;

    // Device name not found
    if (curr == NULL) {
        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Tried to remove a systray icon which does not exist")));
        hr = E_FAIL;
        goto exit;
    }


	if (curr->fWireless)
    {
        // When a wireless card is removed, clear the cache of networks since
        // netui.dll will be unloaded if we get down to zero network adapters 
        // present in the system.
        WZCCacheClear(); 
    
        // Signal ZC Msg thread to stop and wait for it
        SetEvent(g_hStopUpdateStatus);
        WaitForSingleObject(curr->hThreads[THREAD_UPDATE_STATUS], INFINITE);
		CloseHandle(curr->hThreads[THREAD_UPDATE_STATUS]);
		curr->hThreads[THREAD_UPDATE_STATUS] = 0;

        UnloadLoggingQueue();

        if(curr->WLanInfo.IntfEntryEx.bInitialized)
        {
        	pfnWZCDeleteIntfObjEx(&curr->WLanInfo.IntfEntryEx);
        }
        if(curr->WLanInfo.hImgs)
        {
            ImageList_Destroy(curr->WLanInfo.hImgs);
            curr->WLanInfo.hImgs = NULL;
        }

        if (curr->WLanInfo.pHdVList != NULL)
		{
			while (curr->WLanInfo.pHdVList->pNext != curr->WLanInfo.pHdVList)
			{
			    FreeConfigListEnt(curr->WLanInfo.pHdVList->pNext);
			}
			FreeConfigListEnt(curr->WLanInfo.pHdVList);
			curr->WLanInfo.pHdVList = NULL;
		}
        if (curr->WLanInfo.pHdPList != NULL)
		{
			while (curr->WLanInfo.pHdPList->pNext != curr->WLanInfo.pHdPList)
			{
			    FreeConfigListEnt(curr->WLanInfo.pHdPList->pNext);
			}
			FreeConfigListEnt(curr->WLanInfo.pHdPList);
			curr->WLanInfo.pHdPList = NULL;
		}
    }

    // Close property sheet dialog and clean up
    if(curr->fDialogVisible)
    {
		PostMessage(curr->hMainWnd, WM_PSCLOSE, 0, (LPARAM)curr->hPropSheet);
		curr->fDialogVisible = FALSE;
    }

    curr->hPropSheet = 0;

	// Make sure all threads are closed (already did index 0 - THREAD_UPDATE_STATUS)
    for (i = 1; i < NUM_THREADS; i++)
    {
    	if (curr->hThreads[i])
    	{
    		WaitForSingleObject(curr->hThreads[i], INFINITE);
    		CloseHandle(curr->hThreads[i]);
    		curr->hThreads[i] = 0;
    	}
	}

	CloseHandle(curr->hCloseWZCDialogEvent);

    // Delete the icon from the tray
    Shell_NotifyIcon(NIM_DELETE, curr->IconData);
    DestroyIcon(curr->IconData->hIcon);

    PostMessage(curr->hMainWnd, WM_CLOSE, 0, 0);

    // Free data associated with adapter
    free(curr->IconData);
    free(curr->ptcLastWZCStatus);
    free(curr);

	DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: Systray icon removed for device: %s"), ptcDeviceName));

exit:

    return hr;

} // RemoveNetUISystrayIcon



/*++

Routine Name:

	IsPropSheetDialogMessage

Routine Description:

	Checks if the message associated with the invisible window
	is a property sheet message.

Arguments:

	hWnd:	Notification Window Handle
	pMsg:	Message received

Return Value:

	Return value of PropSheet_IsDialogMessage

--*/

BOOL WINAPI IsPropSheetDialogMessage(HWND hWnd, MSG* pMsg)
{
    HRESULT         hr          = S_OK;
    BOOL            fRetVal     = FALSE;
    SystrayIcon*    pSysIcon    = NULL;

	WZCQCFG_LOCK;

    // The the systray icon data for the specified hWnd
    hr = GetDeviceSystrayIcon(hWnd, &pSysIcon);
    if (FAILED(hr)) {
        goto exit;
    }

	if(pSysIcon->fDialogVisible)
	{
		HWND hPropSheet = pSysIcon->hPropSheet;
		
		WZCQCFG_UNLOCK;
    	fRetVal = PropSheet_IsDialogMessage(hPropSheet, pMsg);
    	WZCQCFG_LOCK;
	}

exit:
	WZCQCFG_UNLOCK;
	
    return fRetVal;

} // IsPropSheetDialogMessage



/*++

Routine Name:

    ClosePropSheetDialog

Routine Description:

    Close the Property Sheet which is related to the specified HWND.

Arguments:

    hWnd:	HWND which can represent one of the child property pages or
            the hidden dialog which receives systray icon notifications

Return Value:

    HRESULT to indicate return status

--*/

HRESULT ClosePropSheetDialog(HWND hWnd)
{
    HRESULT         hr          = S_OK;
    SystrayIcon     *pSysIcon   = NULL;
    DWORD           dwFlags     = 0;

	WZCQCFG_LOCK;

    // The the systray icon data for the specified hWnd
    hr = GetDeviceSystrayIcon(hWnd, &pSysIcon);
    if (FAILED(hr))
    {
        hr = S_OK;
        goto exit;
    }

    // Destroy the window and close the handles
    if (pSysIcon->fDialogVisible == TRUE)
    {
        DestroyWindow(pSysIcon->hPropSheet);
    }

    pSysIcon->fDialogVisible = FALSE;
    pSysIcon->hPropSheet = 0;

exit:
	WZCQCFG_UNLOCK;
	
    return hr;

} // ClosePropSheetDialog



/*++

Routine Name:

    ClosePropSheetDialogIfReady

Routine Description:

    This function closes the property sheet if it realizes the user has clicked the
    close or ok buttons

Arguments:

    hWnd:	HWND of the dialog

Return Value:

    HRESULT to indicate return status

--*/

HRESULT WINAPI ClosePropSheetDialogIfReady(HWND hWnd)
{
    HRESULT         hr          = S_OK;
    SystrayIcon*    pSysIcon    = NULL;
    DWORD           dwFlags     = 0;

	WZCQCFG_LOCK;

    // The the systray icon data for the specified hWnd
    hr = GetDeviceSystrayIcon(hWnd, &pSysIcon);
    if (FAILED(hr)) {
        hr = S_OK;
        goto exit;
    }

    // Check if the dialog has already been closed
    if (pSysIcon->fDialogVisible == FALSE) {
        goto exit;
    }

    if (!PropSheet_GetCurrentPageHwnd(pSysIcon->hPropSheet)) {
        // Destroy the window and close the handles
        ClosePropSheetDialog(pSysIcon->hMainWnd);
    }

exit:
	WZCQCFG_UNLOCK;

    return hr;

} // ClosePropSheetDialogIfReady


/*++

Routine Name:

    GetDeviceSystrayIcon

Routine Description:

    Get the systray icon struct associated with the given HWND.  The HWND can be
    the any of the dialogs (property sheet, property pages, Main Dialog)

Arguments:

    hWnd:		The specified HWND
    pSysIcon:	Out paramter to SysIcon struct

Return Value:

    HRESULT to indicate return status

--*/

HRESULT GetDeviceSystrayIcon(HWND hWnd, SystrayIcon **pSysIcon)
{
    HRESULT         hr = S_OK;
    SystrayIcon     *curr = NULL;
    SystrayIcon     *prev = NULL;

	WZCQCFG_LOCK;

    // Treat this as a special case, since some adapters may have an hWnd of zero before being
    // initialized.
    if (hWnd == 0) {
        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Invalid hWnd - could not get systray icon associated with adapter")));
        hr = E_FAIL;
        goto exit;
    }

    // Get the hwnd of the specified device
    curr = g_pSystrayIconsHead;
    while (curr != NULL) {
        // If one of the dialogs matches then curr points to the correct data
        if (curr->hMainWnd == hWnd || curr->hPropSheet == hWnd) {
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    // If HWND was not found return error
    if (curr == NULL) {
        hr = E_FAIL;
        goto exit;
    }

    *pSysIcon = curr;

exit:
	WZCQCFG_UNLOCK;
	
    return hr;

} // GetDeviceSystrayIcon
	


/*++

Routine Name:

    GetDeviceSystrayIconByName

Routine Description:

    Get the systray icon struct associated with the given device name.

Arguments:

    ptcDeviceName:	The specified device name
    pSysIcon:		Out paramter to SysIcon struct

Return Value:

    HRESULT to indicate return status

--*/

HRESULT GetDeviceSystrayIconByName(PTCHAR ptcDeviceName, SystrayIcon **pSysIcon)
{
    HRESULT         hr = S_OK;
    SystrayIcon     *curr = NULL;
    SystrayIcon     *prev = NULL;

	WZCQCFG_LOCK;

    // Get the device name of the specified device
    curr = g_pSystrayIconsHead;
    while (curr != NULL) {
        if (_tcscmp(curr->ptcDeviceName, ptcDeviceName) == 0) {
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    // If device name was not found return error
    if (curr == NULL) {
        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Could not find the systray icon associated with the given device name")));
        hr = E_FAIL;
        goto exit;
    }

    *pSysIcon = curr;

exit:
	WZCQCFG_UNLOCK;

    return hr;

} // GetDeviceSystrayIconByName



/*++

Routine Name:

    GetFriendlyDeviceName

Routine Description:

    Gets the friendly name for a given adapter

Arguments:

	szDeviceName:	Adapter name
	szFriendlyName:	Frienly name for the adapter (out param)

Return Value:

    ERROR_SUCCESS if succeeded

--*/

DWORD GetFriendlyDeviceName(TCHAR* szDeviceName, TCHAR* szFriendlyName)
{
	DWORD 	dwRetVal	= ERROR_SUCCESS;
	DWORD	cdwName		= MAX_PATH;
	HKEY	hKey;

	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, REG_ADAPTER_NAME_MAPPINGS, 0, KEY_READ, &hKey))
	{
		dwRetVal = ERROR_NOT_FOUND;
		goto exit;
	}

	if (ERROR_SUCCESS != RegQueryValueEx(hKey, szDeviceName, NULL, NULL, (LPBYTE)szFriendlyName, &cdwName))
	{
		dwRetVal = ERROR_NOT_FOUND;
		RegCloseKey(hKey);
		goto exit;
	}
	
	RegCloseKey(hKey);

exit:
	return dwRetVal;
}


/*++

Routine Name:

    IPv6Present

Routine Description:

    Determine if IPv6 is available on the system.

Arguments:

Return Value:

    TRUE if IPv6 is configured, FALSE otherwise

--*/
BOOL
IPv6Present()
{
    ULONG ulBufferSize=0;
    // Check if we have any v6 adapters.
    GetAdaptersAddresses(AF_INET6,0,NULL,NULL,&ulBufferSize);
    return(ulBufferSize > 0) ? TRUE : FALSE;
}


/*++

Routine Name:

    CreatePropertySheet

Routine Description:

    Create the property sheet and display it.

Arguments:

    phWnd:			HWND of the property sheet is returned as out parameter
    ptcDeviceName:	String representing the device name

Return Value:

    HRESULT to indicate return status

--*/

HRESULT CreatePropertySheet(HWND *phWnd, HWND hWndParent, PTCHAR ptcDeviceName, BOOL fWireless)
{
    HRESULT             hr = S_OK;
    PROPSHEETPAGE       page[3];
    PROPSHEETHEADER sheet;
    TCHAR               szIPCaption[30];
    TCHAR               szWZCCaption[30];
    TCHAR               szIPv6Caption[30];
    DWORD               index;
    BOOL                fPortrait = (GetSystemMetrics(SM_CXSCREEN) < 480)? TRUE:FALSE;
	int		            iddv4	= fPortrait? IDD_IPQCFG_N:IDD_IPQCFG_W;
    int                 iddv6   = fPortrait? IDD_IPV6DETAILS_N:IDD_IPV6DETAILS_W;
    int                 iddwzc  = fPortrait? IDD_WZCQCFG_N:IDD_WZCQCFG_W;

    // IP Cfg page
    page[0].dwSize = sizeof(PROPSHEETPAGE);
    page[0].dwFlags = PSP_USETITLE;
    page[0].hInstance = v_hInst;
    page[0].pszTemplate = MAKEINTRESOURCE(iddv4);
    page[0].pszIcon = NULL;
    LoadString(v_hInst, IDS_QCIP_TITLE, szIPCaption,
               sizeof(szIPCaption)/sizeof(szIPCaption[0]));
    page[0].pszTitle = szIPCaption;
    page[0].pfnDlgProc = IPQuickCfgDialogProc;
    page[0].lParam = (LPARAM) ptcDeviceName;
    page[0].pfnCallback = NULL;
    page[0].pcRefParent = NULL;

    sheet.nPages = 1;
    sheet.nStartPage = 0;

    index = 0;

    if (IPv6Present()) {
        // IPv6 page
        index++;
        page[index].dwSize = sizeof(PROPSHEETPAGE);
        page[index].dwFlags = PSP_USETITLE;
        page[index].hInstance = v_hInst;
        page[index].pszTemplate = MAKEINTRESOURCE(iddv6);
        page[index].pszIcon = NULL;
        LoadString(v_hInst, IDS_QC_V6_TITLE, szIPv6Caption,
                   sizeof(szIPv6Caption)/sizeof(szIPv6Caption[0]));
        page[index].pszTitle = szIPv6Caption;
        page[index].pfnDlgProc = IPv6QuickCfgDialogProc;
        page[index].lParam = 0;
        page[index].pfnCallback = NULL;
        page[index].pcRefParent = NULL;

        sheet.nPages++;
    }

    if (fWireless) {
        // WZC Cfg page
        index++;
        page[index].dwSize = sizeof(PROPSHEETPAGE);
        page[index].dwFlags = PSP_USETITLE;
        page[index].hInstance = v_hInst;
        page[index].pszTemplate = MAKEINTRESOURCE(iddwzc);
        page[index].pszIcon = NULL;
        LoadString(v_hInst, IDS_QCWZC_TITLE, szWZCCaption,
                   sizeof(szWZCCaption)/sizeof(szWZCCaption[0]));
        page[index].pszTitle = szWZCCaption;
        page[index].pfnDlgProc = WZCQuickCfgDialogProc;
        page[index].lParam = 0;
        page[index].pfnCallback = NULL;
        page[index].pcRefParent = NULL;

        sheet.nPages++;

        // For wireless adapters, show the wireless settings tab
        sheet.nStartPage = index;
    }

    // Property sheet
    sheet.dwSize = sizeof(PROPSHEETHEADER);
    sheet.dwFlags = PSH_PROPSHEETPAGE | PSH_MODELESS | PSH_USECALLBACK;
    //sheet.hwndParent = hWndParent;
    sheet.hwndParent = NULL;
    sheet.hInstance = v_hInst;
    sheet.hIcon =  NULL;
    sheet.pszIcon = NULL;
    sheet.pszCaption = ptcDeviceName;
    // Note leaving this set as 0 for now due to issue retrieving parent window in
    // OnInitWZCDialog.  Currently, code in NotifyDialogProc will select page.
    sheet.nStartPage = 0;
//	sheet.pStartPage = MAKEINTRESOURCE(idd);
    sheet.ppsp = (LPCPROPSHEETPAGE) page;
    sheet.pfnCallback = QuickCfgProcPage;

    // Initialize the common controls.
    CallCommCtrlFunc(InitCommonControls)();

    // Create the property sheet
    *phWnd = (HWND) CallCommCtrlFunc(PropertySheetW)(&sheet);
    if (((INT)*phWnd) == -1) {
        hr = E_FAIL;
        DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Error creating property sheet")));
        goto exit;
    }

exit:
    return hr;

} // CreatePropertySheet

