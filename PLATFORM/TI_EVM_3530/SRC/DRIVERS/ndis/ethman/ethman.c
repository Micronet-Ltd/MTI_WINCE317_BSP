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

    ethman.c

Abstract:

    This file contains the code managing ethernet connections from a systray icon.  It makes
    calls into NetUI.dll to handle the UI.

Environment:

    WinCE

*/



#include <windows.h>
#include <ndis.h>
#include <msgqueue.h>
#include <prsht.h>
#include <windev.h>
#include "ethman.h"
#include "iptypes.h"
#include "nuiouser.h"
#include "iphlpapi.h"
#include <winsock2.h>
#include <ws2tcpip.h>

// Global data
HINSTANCE                       v_hInst                             = NULL;
PTCHAR                          g_ptcDriverName                     = NULL;
Adapter*                        g_AdapterHead                       = NULL;
HANDLE                          g_hNetUILibInst                     = NULL;
CRITICAL_SECTION                g_csEthmanLock;
HANDLE                          g_hNdisUio                          = NULL;
PFN_AddNetUISystrayIcon         g_pfnAddNetUISystrayIcon            = NULL;
PFN_RemoveNetUISystrayIcon      g_pfnRemoveNetUISystrayIcon         = NULL;
PFN_IsPropSheetDialogMessage    g_pfnIsPropSheetDialogMessage       = NULL;
PFN_UpdateConnectionStatus      g_pfnUpdateConnectionStatus         = NULL;
PFN_ClosePropSheetDialogIfReady g_pfnClosePropSheetDialogIfReady    = NULL;



/*++

Routine Name: 

    DllEntry

Routine Description:

    Entry point for the DLL

Arguments:

    hInstDll:    The DLL Instance
    dwReason:    Reson for calling function
    lpReserved: 

Return Value:

    BOOL

--*/

BOOL DllEntry(HINSTANCE hInstDll, DWORD dwReason, LPVOID lpReserved)
{    
    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:

            DEBUGREGISTER(hInstDll);
            v_hInst = hInstDll;
            DisableThreadLibraryCalls ((HMODULE)hInstDll);
            break;
            
        case DLL_PROCESS_DETACH:

            DEBUGMSG(ZONE_INIT, (TEXT("ETHMAN: ethman.dll called with DLL_PROCESS_DETACH reason")));
            if(g_hNetUILibInst)
            {
                FreeLibrary(g_hNetUILibInst);
            }
            
            break;
            
        default:
            
            break;
    }

    return TRUE;
    
} // DllEntry



/*++

Routine Name: 

    DoRealInitThread

Routine Description:

    This thread initializes netui.dll, but waits until shell is ready.

Arguments:

    pv: 

Return Value:

    DWORD

--*/

DWORD WINAPI DoRealInitThread(LPVOID pv)
{
    BOOL        fRet = TRUE;
    HRESULT     hr = S_OK;

    // wait for shell
    WaitForAPIReady(SH_SHELL, INFINITE);

    DEBUGMSG(ZONE_INIT, (TEXT("ETHMAN: ethman.dll called with DLL_PROCESS_ATTACH reason")));

    // Global Initialization
    g_ptcDriverName = (PTCHAR)NDISUIO_DEVICE_NAME;
                
    hr = ProcessNdisUioNotifications();
    if(FAILED(hr))
    {
        fRet = FALSE;
        DEBUGMSG(ZONE_INIT, (TEXT("ETHMAN: ethman failed to initialize Ndis")));
        goto exit;
    }

exit:
    return 0x00;
}



/*++

Routine Name: 

    ETM_Init

Routine Description:

    Initialize Ethman module

--*/

DWORD ETM_Init(DWORD index)
{
    CloseHandle(CreateThread(NULL,0,DoRealInitThread,NULL,0,NULL));
    return TRUE; 
}



/*++

Routine Name: 

    LoadNetui

Routine Description:

    Load the netui dynamic link library

Return Value:

    HRESULT to indicate return status

--*/

HRESULT LoadNetui(void)
{
    HRESULT hr = S_OK;
    DWORD dw = 0;
 
    g_hNetUILibInst = LoadLibrary(TEXT("netui.dll"));
    if(g_hNetUILibInst == NULL)
    {
        dw = GetLastError();
        DEBUGMSG(ZONE_INIT, (TEXT("ETHMAN: Could not load netui.dll from ethman.dll")));
        goto exit;
    }
 
    g_pfnAddNetUISystrayIcon = (PFN_AddNetUISystrayIcon) GetProcAddress(g_hNetUILibInst, TEXT("AddNetUISystrayIcon"));
    if(g_pfnAddNetUISystrayIcon == NULL)
    {
        dw = GetLastError();
        DEBUGMSG(ZONE_INIT, (TEXT("ETHMAN: Could not GetProcAddress in netui.dll")));
        goto exit;
    }
 
    g_pfnRemoveNetUISystrayIcon = (PFN_RemoveNetUISystrayIcon) GetProcAddress(g_hNetUILibInst, TEXT("RemoveNetUISystrayIcon"));
    if(g_pfnRemoveNetUISystrayIcon == NULL)
    {
        dw = GetLastError();
        DEBUGMSG(ZONE_INIT, (TEXT("ETHMAN: Could not GetProcAddress in netui.dll")));
        goto exit;
    }
 
    g_pfnUpdateConnectionStatus = (PFN_UpdateConnectionStatus) GetProcAddress(g_hNetUILibInst, TEXT("UpdateConnectionStatus"));
    if(g_pfnUpdateConnectionStatus == NULL)
    {
        dw = GetLastError();
        DEBUGMSG(ZONE_INIT, (TEXT("ETHMAN: Could not GetProcAddress in netui.dll")));
        goto exit;
    }
 
    g_pfnIsPropSheetDialogMessage = (PFN_IsPropSheetDialogMessage) GetProcAddress(g_hNetUILibInst, TEXT("IsPropSheetDialogMessage"));
    if(g_pfnIsPropSheetDialogMessage == NULL)
    {
        dw = GetLastError();
        DEBUGMSG(ZONE_INIT, (TEXT("ETHMAN: Could not GetProcAddress in netui.dll")));
        goto exit;
    }
 
    g_pfnClosePropSheetDialogIfReady = (PFN_ClosePropSheetDialogIfReady) GetProcAddress(g_hNetUILibInst, TEXT("ClosePropSheetDialogIfReady"));
    if(g_pfnClosePropSheetDialogIfReady == NULL)
    {
        dw = GetLastError();
        DEBUGMSG(ZONE_INIT, (TEXT("ETHMAN: Could not GetProcAddress in netui.dll")));
        goto exit;
    }
 
exit:
 
    hr = HRESULT_FROM_WIN32(dw);
 
    return hr;
}




/*++

Routine Name: 

    UnloadNetui

Routine Description:

    Unloads netui dynamic link library

--*/

void UnloadNetui(void)
{
    g_pfnAddNetUISystrayIcon = NULL;
    g_pfnRemoveNetUISystrayIcon = NULL;
    g_pfnUpdateConnectionStatus = NULL;
    g_pfnIsPropSheetDialogMessage = NULL;
    g_pfnClosePropSheetDialogIfReady = NULL;

    if (g_hNetUILibInst)
    {
        FreeLibrary(g_hNetUILibInst);
        g_hNetUILibInst = NULL;
    }
}



/*++

Routine Name: 

    ProcessNdisUioNotifications

Routine Description:

    Process notification from NDISUIO.  Essentially just start a thread that registers
    and then waits for notifications.

Return Value:

    HRESULT to indicate return status

--*/

HRESULT ProcessNdisUioNotifications(void)
{
    HRESULT hr = S_OK;
    HANDLE     hThread;

    InitializeCriticalSection(&g_csEthmanLock);

    hThread = CreateThread(NULL,0,DoWaitNotifThread,NULL,0,NULL);
    if(hThread == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ETHMAN: Could not create Notification thread")));
        hr = E_FAIL;
        goto exit;
    }

    CloseHandle(hThread);
        
exit:
    return hr;
    
} // ProcessNdisUioNotifications



/*++

Routine Name: 

    DoWaitNotifThread

Routine Description:

    Wait for Notifications from NDISUIO.

Arguments:

    pv:        NULL void pointer

Return Value:

    DWORD

--*/

DWORD WINAPI DoWaitNotifThread(void *pv)
{
    HANDLE                         hMsgQueue = NULL;
    HRESULT                        hr = S_OK;
    MSGQUEUEOPTIONS                sOptions;
    NDISUIO_DEVICE_NOTIFICATION    sDeviceNotification;
    DWORD                          dwBytesReturned;
    DWORD                          dwFlags;
	BOOL						   fWirelessModuleReady = FALSE;
	BOOL						   fDeviceWireless;
	
	void *vp = CreateEvent(0, 1, 0, L"___TIWLNAPI1_ran");

    DEBUGMSG(ZONE_UIO, (TEXT("ETHMAN: Entered NdisUIO notification thread")));

    g_hNdisUio = CreateFile(
                g_ptcDriverName,                                  //    Object name.
                0x0,											  //    Desired access.
                0x00,                                             //    Share Mode.
                NULL,                                             //    Security Attr
                OPEN_EXISTING,                                    //    Creation Disposition.
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,     //    Flag and Attributes..
                (HANDLE)INVALID_HANDLE_VALUE);    

    do
    {
        //
        //    First stop create the msg queue..
        //

        sOptions.dwSize            = sizeof(MSGQUEUEOPTIONS);
        sOptions.dwFlags            = MSGQUEUE_ALLOW_BROKEN;
        sOptions.dwMaxMessages    = 0;
        sOptions.cbMaxMessage        = sizeof(NDISUIO_DEVICE_NOTIFICATION);
        sOptions.bReadAccess        = TRUE;
    
        hMsgQueue = CreateMsgQueue(
                        _T("NDISPWR_QUEUE"),
                        &sOptions);


        if (hMsgQueue == NULL)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("ETHMAN: Could not create message queue for Ndis notifications - breaking from notification loop")));
            break;
        }
        

        while (WaitForSingleObject(hMsgQueue, INFINITE) == WAIT_OBJECT_0)
        {
            while (ReadMsgQueue(
                        hMsgQueue,
                        &sDeviceNotification,
                        sizeof(NDISUIO_DEVICE_NOTIFICATION),
                        &dwBytesReturned,
                        1,
                        &dwFlags))
            {
                PTCHAR     ptcDeviceName = NULL;
                
                // The device name should be in uppercase to query NdisUIO
                ptcDeviceName = _wcsdup(sDeviceNotification.ptcDeviceName);
                if (!ptcDeviceName)
                {
                    DEBUGMSG(ZONE_ERROR, (TEXT("ETHMAN: Could not allocate memory for string \"%s\"\r\n"), 
                        sDeviceNotification.ptcDeviceName));
                    continue;
                }

                _wcsupr(ptcDeviceName);
            
                //
                //    Okay, we have notification..
                //

				//RS Temporary 
				fWirelessModuleReady = TRUE;

                DEBUGMSG(ZONE_UIO, (TEXT("ETHMAN: Received an NdisUIO notification")));

                switch(sDeviceNotification.dwNotificationType)
                {
                    case NDISUIO_NOTIFICATION_BIND:

							fDeviceWireless = IsDeviceWireless(ptcDeviceName);

							if (!fWirelessModuleReady && (fDeviceWireless || IsDeviceBTPAN(ptcDeviceName)))
								break;
	                        
							AddSystrayIcon(ptcDeviceName, fDeviceWireless);
							break;

                    case NDISUIO_NOTIFICATION_UNBIND:

							RemoveSystrayIcon(ptcDeviceName);
							break;

					case NDISUIO_NOTIFICATION_DEVICE_POWER_UP:
							SetEvent(vp);
							fWirelessModuleReady = TRUE;
							break;
                }

                if (ptcDeviceName)
                    LocalFree(ptcDeviceName);
                    
            }    
        
        }    

    }
    while (FALSE);

    if(hMsgQueue)
        CloseMsgQueue(hMsgQueue);    

	CloseHandle(vp);

    DeleteCriticalSection(&g_csEthmanLock);

//exit:
    return 0;

} // DoWaitNotif



/*++

Routine Name: 

    IsDeviceWireless

Routine Description:

    Given the device name returns whether the device is wireless

Arguments:

    ptcDeviceName:    Device name string

Return Value:

    BOOL (TRUE/FALSE)

--*/

BOOL IsDeviceWireless(PTCHAR ptcDeviceName)
{
    NIC_STATISTICS          NicStatistics = {0};
    DWORD                   dwBytesWritten = 0;
    BOOL                    fWireless = FALSE;

    NicStatistics.ptcDeviceName = ptcDeviceName;

    if(DeviceIoControl(
            g_hNdisUio,
            IOCTL_NDISUIO_NIC_STATISTICS,
            NULL,
            0,
            &NicStatistics,
            sizeof(NIC_STATISTICS),
            &dwBytesWritten,
            NULL))
    {
        if(NicStatistics.PhysicalMediaType == NdisPhysicalMediumWirelessLan)
        {
            fWireless = TRUE;
        }
    }

    return fWireless;
    
} // IsDeviceWireless


BOOL IsDeviceBTPAN(PTCHAR ptcDeviceName)
{
	return (0 == _tcsnicmp(ptcDeviceName, TEXT("BTPAN1"), 6));
} 


/*++

Routine Name: 

    HandleCurrentDevices

Routine Description:

    This function checks to see if there are any devices which were loaded before
    NDISUIO notifications were registered for.  It adds an icon to the systray for
    these devices

Return Value:

    HRESULT to indicate return status

--*/

HRESULT HandleCurrentDevices(void)
{
    HRESULT hr = S_OK;
    DWORD   DesiredAccess = 0x0;
    PTCHAR  ptcDeviceName = NULL;

    g_hNdisUio = CreateFile(
                g_ptcDriverName,                                  //    Object name.
                DesiredAccess,                                    //    Desired access.
                0x00,                                             //    Share Mode.
                NULL,                                             //    Security Attr
                OPEN_EXISTING,                                    //    Creation Disposition.
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,     //    Flag and Attributes..
                (HANDLE)INVALID_HANDLE_VALUE);    

    if (g_hNdisUio != INVALID_HANDLE_VALUE)
    {
        DWORD                     dwBytesWritten = 0;
        CHAR                     Buf[1024];
        DWORD                    i = 0;
        DWORD                    dwBufLength = sizeof(Buf);
        PNDISUIO_QUERY_BINDING    pQueryBinding;

        pQueryBinding = (PNDISUIO_QUERY_BINDING)Buf;

        // Iterate through each device
        for(pQueryBinding->BindingIndex = i; /*Nothing*/; pQueryBinding->BindingIndex = ++i)
        {
            // Get device name
            if (DeviceIoControl(
                g_hNdisUio,
                IOCTL_NDISUIO_QUERY_BINDING,
                pQueryBinding,
                sizeof(NDISUIO_QUERY_BINDING),
                Buf,
                dwBufLength,
                &dwBytesWritten,
                NULL))
            {
                // Get Device name
                ptcDeviceName = (PTCHAR)((PUCHAR)pQueryBinding + pQueryBinding->DeviceNameOffset);        

                // Add icon to systray
                AddSystrayIcon(ptcDeviceName, IsDeviceWireless(ptcDeviceName));
                
                DEBUGMSG(ZONE_UIO, (TEXT("ETHMAN: Ethernet device detected")));
            }
            else
            {
                break;
            }
        }
    }

    return hr;

}    //    OpenNdisDevice()


/*++

Routine Name:

    AddrefAdapter

Routine Description:

    This method adds a ref to the specified adapter object    

Arguments:

    pAdapter:    Adapter object to addref
    
--*/

void AddrefAdapter(Adapter* pAdapter)
{
    EnterCriticalSection(&g_csEthmanLock);
    pAdapter->dwRefCount++;
    LeaveCriticalSection(&g_csEthmanLock);
}


/*++

Routine Name:

    DerefAdapter

Routine Description:

    This method dereferences the specified adapter object
    and removes the adapter object if the ref count is zero.

Arguments:

    pAdapter:    Adapter object to deref
    
--*/

void DerefAdapter(Adapter* pAdapter)
{
    EnterCriticalSection(&g_csEthmanLock);
    pAdapter->dwRefCount--;
    if (pAdapter->dwRefCount == 0)
    {
        DEBUGMSG(ZONE_INIT, (TEXT("ETHMAN: DerefAdapter is removing the adapter instance 0x%08X.\n"), pAdapter));
        RemoveAdapter(pAdapter);
    }
    LeaveCriticalSection(&g_csEthmanLock);
}


/*++

Routine Name:

    RemoveAdapter

Routine Description:

    Removes the specified adapter struct from the list

Arguments:

    pAdapter:    Adapter to remove

--*/

void RemoveAdapter(Adapter* pAdapter)
{
    // Assert locked
    ASSERT(g_csEthmanLock.OwnerThread == (HANDLE)GetCurrentThreadId());
        
    // Remove the adapter
    if(pAdapter)
    {
        if(pAdapter == g_AdapterHead)
        {
            g_AdapterHead = g_AdapterHead->pNext;
            if(g_AdapterHead == NULL)
            {
                UnloadNetui();
            }
        }
    
        if(pAdapter->pNext != NULL)
        {
            pAdapter->pNext->pPrev = pAdapter->pPrev;
        }
        if(pAdapter->pPrev != NULL)
        {
            pAdapter->pPrev->pNext = pAdapter->pNext;
        }
        if(NULL != pAdapter->hAlive) 
        {
            CloseHandle(pAdapter->hAlive);
        }

        if(NULL != pAdapter->hUIThread)
        {
            CloseHandle(pAdapter->hUIThread);
        }

        if(NULL != pAdapter->hIPThread)
        {
            CloseHandle(pAdapter->hIPThread);
        }
        
        LocalFree(pAdapter);
    }
}



/*++

Routine Name:

    AddAdapter

Routine Description:

    This function create the adapter struct and adds it to the list.

Arguments:

    ppAdapter:        Adapter to add
    ptcDeviceName:    Name of the device to add
    
Return Value:

    HRESULT to indicate return status

--*/

HRESULT AddAdapter(Adapter** ppAdapter, PTCHAR ptcDeviceName, BOOL fWireless)
{
    HRESULT    hr = S_OK;

    // Fill up the adapter struct and pass it to 
    Adapter* pAdapter = (Adapter*)LocalAlloc(LMEM_FIXED, sizeof(Adapter));
    if(pAdapter == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Ethman: Out of memory")));
        hr = E_OUTOFMEMORY;
        goto exit;
    }else{
        ZeroMemory(pAdapter, sizeof(Adapter));
    }

    pAdapter->dwRefCount = 1;
    _tcsncpy(pAdapter->ptcDeviceName, ptcDeviceName, MAX_DEVICE_NAME);
    pAdapter->fWireless = fWireless;
    pAdapter->hAlive = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    // Insert the element into the adapters list
    
    EnterCriticalSection(&g_csEthmanLock);

    if (g_AdapterHead == NULL)
    {
        // Need to load netui.dll
        hr = LoadNetui();
        if(FAILED(hr))
        {
            LeaveCriticalSection(&g_csEthmanLock);
            goto exit;
        }
    }

    pAdapter->pNext         = g_AdapterHead;
    g_AdapterHead           = pAdapter;
    pAdapter->pPrev         = NULL;

    if(pAdapter->pNext != NULL)
    {
        pAdapter->pNext->pPrev = pAdapter;
    }
    
    LeaveCriticalSection(&g_csEthmanLock);

exit:
    *ppAdapter = pAdapter;
    
    return hr;
}



/*++

Routine Name:

    GetAdapter

Routine Description:

    Get the adapter struct from the list of adapters

Arguments:

    ppAdapter:        Out param for the adapter
    ptcDeviceName:    Name of the device to get

Return Value:

    HRESULT to indicate return status

--*/

HRESULT GetAdapter(Adapter** ppAdapter, PTCHAR ptcDeviceName)
{
    HRESULT hr = E_FAIL;
    
    Adapter* pAdapter;

    EnterCriticalSection(&g_csEthmanLock);

    pAdapter = g_AdapterHead;
    while(pAdapter != NULL)
    {
        if(_tcscmp(pAdapter->ptcDeviceName, ptcDeviceName) == 0)
        {
            AddrefAdapter(pAdapter);
            hr = S_OK;
            break;
        }
        pAdapter = pAdapter->pNext;
    }

    LeaveCriticalSection(&g_csEthmanLock);

    *ppAdapter = pAdapter;

    return hr;
    
}


/*++

Routine Name:

    AddSystrayIcon

Routine Description:

    This function starts the appropriate thread to create a systray icon 
    for either a wireless or wired network.

Arguments:

    ptcDeviceName:    Device name string
    fWireless:        Boolean representing wired/wireless

Return Value:

    HRESULT to indicate return status

--*/

HRESULT AddSystrayIcon(PTCHAR ptcDeviceName, BOOL fWireless)
{
    HRESULT     hr                 = S_OK;
    Adapter*    pAdapter        = NULL;

    // Add the adapter to the list
    hr = AddAdapter(&pAdapter, ptcDeviceName, fWireless);
    if(FAILED(hr))
    {
        goto exit;
    }
    
    AddrefAdapter(pAdapter); // Addref before passing to thread
    pAdapter->hUIThread = CreateThread(NULL,0,SystrayIconThread,(LPVOID)pAdapter,0,NULL);
    if (! pAdapter->hUIThread)
    {
        DerefAdapter(pAdapter);
        goto exit;
    }

exit:
    return hr;
}



/*++

Routine Name:

    IPNotificationThread

Routine Description:

    This thread listens for changes in an adapters IP address and changes it if so    

--*/

DWORD WINAPI IPNotificationThread(LPVOID pv)
{
    Adapter*        pAdapter    = (Adapter*) pv;
    HANDLE          phEvents[2] = {0,0};
    DWORD           dwRetVal    = 0;
    WORD            wVersionRequested;
    WSADATA         wsadata;
    int             Status;
    SOCKET          AddrChangeSock = INVALID_SOCKET;
    WSAOVERLAPPED   WSAOverlapped;
    TCHAR           szDeviceName[MAX_DEVICE_NAME];

    phEvents[0] = pAdapter->hAlive;
    wcscpy(szDeviceName, pAdapter->ptcDeviceName);
    
    // Done with adapter object
    DerefAdapter(pAdapter);

    wVersionRequested = MAKEWORD(2,2);    

    Status = WSAStartup (wVersionRequested, &wsadata);

    if (Status != 0) 
    {
        DEBUGMSG (ZONE_ERROR,
            (TEXT("Ethman:: WSAStartup() call failed.   Err:[0x%x]\r\n"),
            WSAGetLastError()));

        goto exit;
    }

    AddrChangeSock = socket(AF_INET, SOCK_STREAM, 0);

    if (AddrChangeSock == INVALID_SOCKET)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("Ethman:: Failed socket() call.\r\n")));

        AddrChangeSock = INVALID_SOCKET;
        WSACleanup();
        goto exit;
    }

    phEvents[1] = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (phEvents[1] == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("Ethman:: Failed CreateEvent() in StartAddressChangeNotification().\r\n")));

        goto exit;
    }  


    //
    //  Now request for the notification..
    //

    memset (&WSAOverlapped, 0x00, sizeof(WSAOVERLAPPED));

    WSAOverlapped.hEvent = phEvents[1];    

    Status = WSAIoctl(
                AddrChangeSock, 
                SIO_ADDRESS_LIST_CHANGE, 
                NULL, 
                0, 
                NULL, 
                0, 
                NULL, 
                &WSAOverlapped, 
                NULL);    

    if (Status != ERROR_SUCCESS && GetLastError() != ERROR_IO_PENDING)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("Ethman:: Failed WSAIoctl() for SIO_ADDRESS_LIST_CHANGE.\r\n")));
            
        goto exit;
    }    

    // We should update connection status once since IP could have already been bound
    UpdateConnectionStatus(szDeviceName);
    
    while(1)
    {
        // Wait for quit event or IP change
        dwRetVal = WaitForMultipleObjects(2, phEvents, FALSE, INFINITE);

        //
        //  Immediately tells winsock to listen for the next one..
        //
        Status = WSAIoctl(
                AddrChangeSock, 
                SIO_ADDRESS_LIST_CHANGE, 
                NULL, 
                0, 
                NULL, 
                0, 
                NULL, 
                &WSAOverlapped, 
                NULL);

        if (Status != ERROR_SUCCESS && GetLastError() != ERROR_IO_PENDING)
        {
            DEBUGMSG(ZONE_ERROR,
                (TEXT("Ethman:: Failed WSAIoctl() for SIO_ADDRESS_LIST_CHANGE.\r\n")));
                
            goto exit;
        }                    
            
        if(1 != dwRetVal - WAIT_OBJECT_0)
        {
            // If the adapter has been removed or this call somehow fails
            // then break from the loop
            break;
        }

        // Create a thread to update the connection status and get back to notify
        // call immediately.
        UpdateConnectionStatus(szDeviceName);
    }

exit:

    DEBUGMSG(ZONE_UIO, (TEXT("Ethman: Exiting IPNotificationThread")));

    if (AddrChangeSock != INVALID_SOCKET)
    {
        closesocket(AddrChangeSock);
        WSACleanup();
    }

    if (phEvents[1])
        CloseHandle(phEvents[1]);
    
    return 0;    
}



/*++

Routine Name:

    UpdateConnectionStatus

Routine Address:

    This function updates the connection status of each of the given adapter.

Arguments:

    ptcDeviceName:    Name of the adapter

--*/

void UpdateConnectionStatus(PTCHAR ptcDeviceName)
{
    DWORD               dwRetVal             = NO_ERROR;
    PIP_ADAPTER_INFO    pAdapterInfo         = NULL;
    PIP_ADAPTER_INFO    pAdapterInfoHead    = NULL;
    ULONG               ulBufferSize        = 0;
    TCHAR*              pszAdapterName        = NULL;
    DWORD               cdwAdapterName        = 0;
    BOOL                fDisConnected        = TRUE;

    // Get IP Adapters Info from IPhelper
    dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulBufferSize);
    if(dwRetVal == ERROR_BUFFER_OVERFLOW)
    {
        pAdapterInfo = (PIP_ADAPTER_INFO)LocalAlloc(LMEM_FIXED, ulBufferSize);
        if(pAdapterInfo == NULL)
        {
             dwRetVal = ERROR_OUTOFMEMORY;
            goto exit;
        }

            // Set a pointer to the head of the list
        pAdapterInfoHead = pAdapterInfo;
      
        dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulBufferSize);
        if(dwRetVal != NO_ERROR)
        {
            goto exit;
        }
    }
    else if(dwRetVal != NO_ERROR)
    {
        goto exit;
    }

    // If no adapter interface is present
    if(ulBufferSize == 0)
    {
        dwRetVal = ERROR_INVALID_DATA;
        goto exit;
    }    

    // Check if the Public Interface Adapter is connected
    do
    {
        DWORD cdwCurrAdapterName = strlen(pAdapterInfo->AdapterName)+1;
        if(cdwCurrAdapterName > cdwAdapterName)
        {
            // Free the existing buffer (if it exists) 
            if(pszAdapterName)
            {
                LocalFree(pszAdapterName);
                pszAdapterName = NULL;
            }
                    
            // Allocate some memory for the adapter name string.  Make this large enough so 
            // that we probably won't have to reallocate more later.
            cdwAdapterName     = cdwCurrAdapterName + 10;
            pszAdapterName    = (WCHAR*)LocalAlloc(LMEM_FIXED, cdwAdapterName*sizeof(TCHAR));
            if(pszAdapterName == NULL)
            {
                dwRetVal = ERROR_OUTOFMEMORY;
                goto exit;
            }
        }
        
        mbstowcs(pszAdapterName, pAdapterInfo->AdapterName, cdwCurrAdapterName);

        if(_tcscmp(ptcDeviceName, pszAdapterName) == 0)
        {
            // Verify that the IP address does not represent an invalid connection
            if(strcmp(pAdapterInfo->IpAddressList.IpAddress.String, "0.0.0.0") == 0)
            {
                g_pfnUpdateConnectionStatus(ptcDeviceName, FALSE);
            }
            else
            {
                g_pfnUpdateConnectionStatus(ptcDeviceName, TRUE);
            }

            dwRetVal            = NO_ERROR;
            fDisConnected        = FALSE;
            break;
        }

        pAdapterInfo = pAdapterInfo->Next;
    }
    while(pAdapterInfo != NULL);

exit:

    if(pszAdapterName)
    {
        LocalFree(pszAdapterName);
    }

    if(pAdapterInfoHead)
    {
        LocalFree(pAdapterInfoHead);
    }

    if(fDisConnected == TRUE)
    {
        dwRetVal = ERROR_DEV_NOT_EXIST;
    }

    if(dwRetVal != NO_ERROR)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ETHMAN: Error updating connection status: %d"), GetLastError()));
    }
}




/*++

Routine Name: 

    SystrayIconThread

Routine Description:

    Thread to handle Window messages for the current device systray icon.

Arguments:

    pv:        Void pointer to the device name

Return Value:

    DWORD

--*/

DWORD WINAPI SystrayIconThread(LPVOID pv)
{
    HRESULT     hr             = S_OK;
    HWND        hWnd;
    Adapter*    pAdapter       = (Adapter*) pv;
    MSG         msg;

    DEBUGMSG(ZONE_INIT, (TEXT("\r\n***************\r\nETHMAN: Systray Icon added.\r\n**************")));

    // Add systray icon
    hr = g_pfnAddNetUISystrayIcon(pAdapter->ptcDeviceName, pAdapter->fWireless, &hWnd);
    if(FAILED(hr))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ETHMAN: Failed to add systray icon")));                                
        goto exit;
    }
    
    AddrefAdapter(pAdapter); // Addref before passing to thread
    pAdapter->hIPThread = CreateThread(NULL, 0, IPNotificationThread, (LPVOID)pAdapter, 0, NULL);
    if (! pAdapter->hIPThread)
    {        
        DerefAdapter(pAdapter);
        goto exit;
    }

    // We have successfully added the adapter so it is now safe to remove it
    // if requested.

    while(GetMessage(&msg, NULL, 0, 0))
    {
        g_pfnClosePropSheetDialogIfReady(hWnd);
        if(!g_pfnIsPropSheetDialogMessage(hWnd, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);    
        }
    }

exit:

    // Done with the adapter object on this thread
    DerefAdapter(pAdapter);

    EnterCriticalSection(&g_csEthmanLock);
    if(g_AdapterHead == NULL)
    {
        UnloadNetui();
    }
    LeaveCriticalSection(&g_csEthmanLock);
    
    DEBUGMSG(ZONE_UIO, (TEXT("Ethman: Exiting SystrayIconThread")));
    return 0;
    
} // SystrayIconThread



/*++

Routine Name:

    RemoveSystrayIcon

Routine Description:

    Removes the systray icon and cleans up.

Arguments:

    ptcDeviceName:    Name of the adapter to remove

Return Value:

    HRESULT to indicate return status

--*/

HRESULT RemoveSystrayIcon(PTCHAR ptcDeviceName)
{
    HRESULT     hr             = S_OK;
    Adapter*    pAdapter     = NULL;

    // Set the event to terminate threads associated with this device and wait for them to die
    hr = GetAdapter(&pAdapter, ptcDeviceName);
    if(FAILED(hr))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ETHMAN: Tried to remove adapter that was already removed")));
        goto exit;
    }
    
    SetEvent(pAdapter->hAlive);
    WaitForSingleObject(pAdapter->hIPThread, 5000);

    hr = g_pfnRemoveNetUISystrayIcon(ptcDeviceName);
    if(FAILED(hr))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ETHMAN: Failed to remove systray icon")));
    }
    
    // Deref twice: Once to match the GetAdapter call above and once to
    // remove the adapter instance.
    DerefAdapter(pAdapter);
    DerefAdapter(pAdapter);

exit:        
    return hr;

} // RemoveSystrayIcon


