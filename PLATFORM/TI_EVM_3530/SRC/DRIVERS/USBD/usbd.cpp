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

usbd.cpp

Abstract:

Functions:

Notes:


--*/
#pragma warning (3 : 4100 4101 4705 4706)

#include <windows.h>

#include "usbdi.h"
#include "hcdi.h"

#include "usbdinc.hpp"
#include "usbdobj.hpp"
#include "usbd.hpp"

LPCWSTR gcszUsbRegKey = L"Drivers\\USB";
LPCWSTR gcszDriverIDs = L"ClientDrivers";
LPCWSTR gcszLoadClients = L"LoadClients";
LPCWSTR gcszDefaultDriver = L"Default";
LPCWSTR gcszDllName = L"DLL";
LPCWSTR gcszUSBDeviceAttach = L"USBDeviceAttach";
LPCWSTR gcszUSBInstallDriver = L"USBInstallDriver";

static USB_FUNCS gc_UsbFuncs =
{
    sizeof(USB_FUNCS),      //DWORD                   dwCount;

    &GetUSBDVersion,                // LPGET_USBD_VERSION
    &OpenClientRegistryKey,         // LPOPEN_CLIENT_REGISTRY_KEY
    &RegisterNotificationRoutine,   // LPREGISTER_NOTIFICATION_ROUTINE
    &UnRegisterNotificationRoutine, // LPUN_REGISTER_NOTIFICATION_ROUTINE
    &LoadGenericInterfaceDriver,    // LPLOAD_GENERIC_INTERFACE_DRIVER
    &TranslateStringDescr,          // LPTRANSLATE_STRING_DESCR
    &FindInterface,                 // LPFIND_INTERFACE
    &GetFrameNumber,                // LPGET_FRAME_NUMBER
    &GetFrameLength,                // LPGET_FRAME_LENGTH
    &TakeFrameLengthControl,        // LPTAKE_FRAME_LENGTH_CONTROL
    &ReleaseFrameLengthControl,     // LPRELEASE_FRAME_LENGTH_CONTROL
    &SetFrameLength,                // LPSET_FRAME_LENGTH
    &GetDeviceInfo,                 // LPGET_DEVICE_INFO
    &IssueVendorTransfer,           // LPISSUE_VENDOR_TRANSFER
    &GetInterface,                  // LPGET_INTERFACE
    &SetInterface,                  // LPSET_INTERFACE
    &GetDescriptor,                 // LPGET_DESCRIPTOR
    &SetDescriptor,                 // LPSET_DESCRIPTOR
    &SetFeature,                    // LPSET_FEATURE
    &ClearFeature,                  // LPCLEAR_FEATURE
    &GetStatus,                     // LPGET_STATUS
    &SyncFrame,                     // LPSYNC_FRAME
    &OpenPipe,                      // LPOPEN_PIPE
    &AbortPipeTransfers,            // LPABORT_PIPE_TRANSFERS
    &ResetPipe,                     // LPRESET_PIPE
    &ClosePipe,                     // LPCLOSE_PIPE
    &IsPipeHalted,                  // LPIS_PIPE_HALTED
    &IssueControlTransfer,          // LPISSUE_CONTROL_TRANSFER
    &IssueBulkTransfer,             // LPISSUE_BULK_TRANSFER
    &IssueInterruptTransfer,        // LPISSUE_INTERRUPT_TRANSFER
    &IssueIsochTransfer,            // LPISSUE_ISOCH_TRANSFER
    &IsTransferComplete,            // LPIS_TRANSFER_COMPLETE
    &GetTransferStatus,             // LPGET_TRANSFER_STATUS
    &GetIsochResults,               // LPGET_ISOCH_RESULTS
    &AbortTransfer,                 // LPABORT_TRANSFER
    &CloseTransfer,                 // LPCLOSE_TRANSFER

    // These functions were added with USBDI version 1.1
    &ResetDefaultPipe,              // LPRESET_DEFAULT_PIPE
    &IsDefaultPipeHalted,            // LPIS_DEFAULT_PIPE_HALTED
    // This Function were added with USBDI version 1.2
    &DisableDevice,                 //LPDISABLE_DEVICE 
    &SuspendDevice,                 //LPSUSPEND_DEVICE
    &ResumeDevice,                  //LPRESUME_DEVICE
    // This Function were added with USBDI version 1.3   
    &GetClientRegistryPath          // LPGET_CLIENT_REGISTRY_PATH
};


BOOL AddTransfer(SPipe * pPipe, STransfer * pTransfer)
{
    EnterCriticalSection(&pPipe->csTransferLock);

    // Check if we're being removed
    if (pPipe->dwSig != VALID_PIPE_SIG)
    {
        LeaveCriticalSection(&pPipe->csTransferLock);
        return FALSE;
    }

    for(UINT iTransfer = 0 ; iTransfer < pPipe->cAllocatedTransfers ;
            ++iTransfer)
    {
        if(pPipe->apTransfers[iTransfer] == NULL)
            break;
    }

    if(iTransfer == pPipe->cAllocatedTransfers)
    {
        STransfer * *   apNewTransfers;
        UINT        cAllocated =
                pPipe->cAllocatedTransfers + gcTransferAllocateChunk;

        apNewTransfers = new STransfer * [cAllocated];
        if (apNewTransfers == NULL)
        {
            LeaveCriticalSection(&pPipe->csTransferLock);
            return FALSE;
        }

        for(UINT iCopy = 0 ; iCopy < pPipe->cAllocatedTransfers ; ++iCopy)
            apNewTransfers[iCopy] = pPipe->apTransfers[iCopy];

        for(iCopy += 1 ; iCopy < cAllocated ; ++iCopy)
            apNewTransfers[iCopy] = NULL;

        if(pPipe->apTransfers)
            delete [] pPipe->apTransfers;
        pPipe->apTransfers = apNewTransfers;
        pPipe->cAllocatedTransfers = cAllocated;
    }

    pTransfer->iTransfer = iTransfer;
    pPipe->apTransfers[iTransfer] = pTransfer;

    LeaveCriticalSection(&pPipe->csTransferLock);

    return TRUE;
}

void GetSettingString(LPTSTR pszString, DWORD cchString, DWORD dw1, DWORD dw2, DWORD dw3)
{
    LPCTSTR pszFormat = NULL;

    if (dw3 != USB_NO_INFO) {
        ASSERT( (dw1 != USB_NO_INFO) && (dw2 != USB_NO_INFO) );
        pszFormat = TEXT("%u_%u_%u");
    }
    else if (dw2 != USB_NO_INFO) {
        ASSERT(dw1 != USB_NO_INFO);
        pszFormat = TEXT("%u_%u");
    }
    else if (dw1 != USB_NO_INFO) {
        pszFormat = TEXT("%u");
    }

    if (pszFormat == NULL) {
        // if there are no specific settings then return a default setting
        StringCchCopy(pszString, cchString, gcszDefaultDriver);
    }
    else {
        StringCchPrintf(pszString, cchString, pszFormat, dw1, dw2, dw3);
    }
}


BOOL AddDriverLib(SDevice * pDev, HINSTANCE hDriver)
{
    EnterCriticalSection(&pDev->csLibList);

    SDriverLibs * * ppLib = &pDev->pDriverLibs;

    while(*ppLib)
        ppLib = &((*ppLib)->pNext);

    *ppLib = new SDriverLibs;

    if (*ppLib == NULL) {
        LeaveCriticalSection(&pDev->csLibList);
        return FALSE;
    }
    
    (*ppLib)->hDriver = hDriver;
    (*ppLib)->pNext = NULL;

    LeaveCriticalSection(&pDev->csLibList);

    return TRUE;
}


BOOL
InstallClientDriver(LPCWSTR szDriverName)
{
   BOOL fRet = FALSE;

   HINSTANCE hClientInstance = LoadLibrary(szDriverName);

    if (hClientInstance) {
        LPCLIENT_INSTALL_PROC pInstallProc =
        (LPCLIENT_INSTALL_PROC) GetProcAddress(hClientInstance,gcszUSBInstallDriver);
      if (pInstallProc) {
          __try {
                if ((*pInstallProc)(szDriverName)) {
                    DEBUGMSG(ZONE_LOADER,(TEXT("USBD:InstallClientDriver(%s), Install completed successfully\r\n"),szDriverName));
                    fRet = TRUE;
                } else {
                    DEBUGMSG(ZONE_ERROR,(TEXT("!USBD:InstallClientDriver(%s), error in client driver install\r\n"),szDriverName));
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                DEBUGMSG(ZONE_ERROR,(TEXT("!USBD: Exception in install proc\r\n")));
            }
        } else {
         DEBUGMSG(ZONE_ERROR,(TEXT("!USBD:InstallClientDriver, GetProcAddr(%s,%s) failed: %u\r\n"),
                              szDriverName,gcszUSBInstallDriver,GetLastError()));
      }
   }
   else {
      DEBUGMSG(ZONE_ERROR,(TEXT("!USBD:InstallClientDriver, error in LoadLibrary(%s): %u\r\n"),
                           szDriverName,GetLastError()));
   }

   if (hClientInstance) {
      FreeLibrary(hClientInstance);
   }
   return fRet;
}

BOOL LoadRegisteredDriver(HKEY hkReg, SDevice * pDev,
        LPCUSB_INTERFACE lpInterface, LPBOOL pfLoaded,
        LPCUSB_DRIVER_SETTINGS lpDriverSettings)
{
    UINT iSubKey = 0;
    DWORD cchDriverId;
    WCHAR szDriverId[gcMaxDriverIdSize];
    HKEY hkSetting;

    cchDriverId = sizeof(szDriverId) / sizeof(szDriverId[0]);

    while(RegEnumKeyEx(hkReg, iSubKey++, szDriverId, &cchDriverId, NULL,
            NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        if(RegOpenKeyEx(hkReg, szDriverId, 0, 0, &hkSetting) == ERROR_SUCCESS)
        {
            WCHAR szDll[USB_MAX_LOAD_STRING];
            DWORD cbDll = sizeof(szDll);
            DWORD dwType;

            RegQueryValueEx(hkSetting, gcszDllName, NULL, &dwType,
                    (LPBYTE)szDll, &cbDll);

            if(cbDll && dwType == REG_SZ)
            {
                // We use LoadDriver here instead of LoadLibrary so that client drivers will
                // be locked into RAM.  This is in case a client wishes to register with device
                // to receive power notifications - you cannot take a page fault in a power handler
                // routine.
                HINSTANCE hClientInstance = LoadDriver(szDll);

                if(hClientInstance)
                {
                    LPCLIENT_ATTACH_PROC pAttachProc = (LPCLIENT_ATTACH_PROC)
                            GetProcAddress(hClientInstance,
                            gcszUSBDeviceAttach);

                    if(pAttachProc)
                    {
                        DEBUGMSG(ZONE_LOADER,(TEXT("USBD: Calling client attach proc for %s\r\n"),szDriverId));
                        __try {
                            (*pAttachProc)((USB_HANDLE)pDev, &gc_UsbFuncs,
                                           lpInterface, szDriverId, pfLoaded,
                                           lpDriverSettings, 0);
                        } __except(EXCEPTION_EXECUTE_HANDLER) {
                              DEBUGMSG(ZONE_ERROR,(TEXT("!USBD: Exception in attach proc\r\n")));
                              *pfLoaded = FALSE;
                        }
                        DEBUGMSG(ZONE_LOADER,(TEXT("USBD: Client %s control of device\r\n"),
                                              (*pfLoaded)? TEXT("accepted"):TEXT("did not accept")));
                    }
                    else
                        DEBUGMSG(ZONE_ERROR,(TEXT("!LoadRegisteredDriver: No attach proc for client driver %s\r\n"),szDriverId));
                    if(!*pfLoaded)
                        FreeLibrary(hClientInstance);
                }
                else  {
                    DEBUGMSG(ZONE_ERROR,(TEXT("!USBD: Error in LoadDriver(%s): %u\r\n"),
                                         szDll,GetLastError()));
                }
                if(*pfLoaded)
                    AddDriverLib(pDev, hClientInstance);
            }
            RegCloseKey(hkSetting);
        }
        cchDriverId = sizeof(szDriverId) / sizeof(szDriverId[0]);
    }

    return TRUE;
}

BOOL ConvertToClientRegistry(LPTSTR lpStr, DWORD cchStr, LPCUSB_DEVICE pDeviceInfo,LPCUSB_INTERFACE pInterface,BOOL fGroup1, BOOL fGroup2, BOOL fGroup3,PUSB_DRIVER_SETTINGS pSetting)
{
    USB_DRIVER_SETTINGS DriverSettings;
    DEBUGMSG(ZONE_LOADER,(TEXT("USBD:ConvertToClientRegistry for fGroup1=%d,fGroup2=%d,fGroup=%d\r\n"),fGroup1, fGroup2, fGroup3));
    DriverSettings.dwCount = sizeof(DriverSettings);
    
    // Reset Valid setting signaure before the loop
    BOOL fWork1=TRUE;
    if (fGroup1 && pDeviceInfo ) { // Default
        DriverSettings.dwVendorId = pDeviceInfo->Descriptor.idVendor;
        DriverSettings.dwProductId = pDeviceInfo->Descriptor.idProduct;
        DriverSettings.dwReleaseNumber = pDeviceInfo->Descriptor.bcdDevice;
    }
    else {
        DriverSettings.dwVendorId = USB_NO_INFO;
        DriverSettings.dwProductId = USB_NO_INFO;
        DriverSettings.dwReleaseNumber = USB_NO_INFO;
    }
    // do while will allow each deafult at least hit once.
    do { 
        
        BOOL fWork2 = TRUE;
        if (fGroup2 && pDeviceInfo) {
            DriverSettings.dwDeviceClass = pDeviceInfo->Descriptor.bDeviceClass;
            DriverSettings.dwDeviceSubClass = pDeviceInfo->Descriptor.bDeviceSubClass;
            DriverSettings.dwDeviceProtocol =pDeviceInfo->Descriptor.bDeviceProtocol;
        }
        else {
            DriverSettings.dwDeviceClass = USB_NO_INFO;
            DriverSettings.dwDeviceSubClass = USB_NO_INFO;
            DriverSettings.dwDeviceProtocol = USB_NO_INFO;
        }
        do {
            
            BOOL fWork3 = TRUE ;
            if ( fGroup3 && pInterface) {
                DriverSettings.dwInterfaceClass = pInterface->Descriptor.bInterfaceClass;
                DriverSettings.dwInterfaceSubClass =  pInterface->Descriptor.bInterfaceSubClass;
                DriverSettings.dwInterfaceProtocol =  pInterface->Descriptor.bInterfaceProtocol;
            }
            else {
                DriverSettings.dwInterfaceClass = USB_NO_INFO;
                DriverSettings.dwInterfaceSubClass =  USB_NO_INFO;
                DriverSettings.dwInterfaceProtocol =  USB_NO_INFO;
            }
            do {
                // Convert Setting Signature to Registry PATH.
                if (lpStr == NULL) {
                    return FALSE;
                }
                
                TCHAR szDevice[gcMaxDriverString];
                GetSettingString(szDevice, gcMaxDriverString, DriverSettings.dwVendorId,DriverSettings.dwProductId, DriverSettings.dwReleaseNumber);
                
                TCHAR szDeviceClass[gcMaxDriverString];
                GetSettingString(szDeviceClass, gcMaxDriverString, DriverSettings.dwDeviceClass,DriverSettings.dwDeviceSubClass,DriverSettings.dwDeviceProtocol);
                
                TCHAR szInterfaceClass[gcMaxDriverString];
                GetSettingString(szInterfaceClass, gcMaxDriverString, DriverSettings.dwInterfaceClass,DriverSettings.dwInterfaceSubClass, DriverSettings.dwInterfaceProtocol);

                HRESULT hr = StringCchPrintf(lpStr, cchStr, TEXT("%s\\%s\\%s\\%s\\%s"),
                    gcszUsbRegKey, gcszLoadClients, szDevice, szDeviceClass,
                    szInterfaceClass);
                if (FAILED(hr)) {
                    return FALSE;
                }
                
                HKEY hClientRegKey;
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,lpStr,0,0,&hClientRegKey) == ERROR_SUCCESS ) { 
                    // We found a available Registry .
                    if (pSetting!=NULL)
                        memcpy(pSetting , &DriverSettings, sizeof(USB_DRIVER_SETTINGS));
                    DEBUGMSG(ZONE_LOADER,(TEXT("USBD:ConvertToClientRegistry, Registry %s is exist\r\n"),lpStr));
                    RegCloseKey(hClientRegKey);
                    return TRUE;
                }
                else
                    DEBUGMSG(ZONE_LOADER,(TEXT("USBD:ConvertToClientRegistry, Registry %s is not exist\r\n"),lpStr));

                // Reduce length of setting signature.
                if (fGroup3) { 
                    if (DriverSettings.dwInterfaceProtocol != USB_NO_INFO)
                        DriverSettings.dwInterfaceProtocol = USB_NO_INFO;
                    else if  (DriverSettings.dwInterfaceSubClass != USB_NO_INFO)
                        DriverSettings.dwInterfaceSubClass = USB_NO_INFO;
                    else 
                        fWork3 = FALSE;
                }
                else
                    fWork3 = FALSE;
            }while (fWork3);
            
            // Reduce length of setting signature.
            if (fGroup2) {
                if (DriverSettings.dwDeviceProtocol != USB_NO_INFO)
                    DriverSettings.dwDeviceProtocol = USB_NO_INFO;
                else if (DriverSettings.dwDeviceSubClass != USB_NO_INFO)
                    DriverSettings.dwDeviceSubClass = USB_NO_INFO;
                else 
                    fWork2 = FALSE;
            } 
            else fWork2 = FALSE;
        }while (fWork2);
        // Reduce length of setting signature.
        if (fGroup1) {
            if(DriverSettings.dwReleaseNumber != USB_NO_INFO)
                DriverSettings.dwReleaseNumber = USB_NO_INFO ;
            else if (DriverSettings.dwProductId != USB_NO_INFO)
                DriverSettings.dwProductId = USB_NO_INFO;
            else 
                fWork1 = FALSE;
        }
        else
            fWork1 = FALSE;
    }while( fWork1);
    DEBUGMSG(ZONE_LOADER,(TEXT("-USBD:ConvertToClientRegistry, Can not find registry\r\n")));
    return FALSE;
}


BOOL LoadGroupDriver(SDevice * pDev, PBOOL pfLoaded,
        LPCUSB_INTERFACE lpInterface,
        BOOL fGroup1, BOOL fGroup2, BOOL fGroup3)
{
    *pfLoaded = FALSE;
    BOOL fRet = TRUE;
    HKEY hClientRegKey;
    USB_DRIVER_SETTINGS CurSettings;
    TCHAR RegPath[MAX_PATH];
    
    if (ConvertToClientRegistry(RegPath,MAX_PATH,pDev?pDev->pDeviceInfo:NULL,lpInterface,fGroup1,fGroup2,fGroup3,&CurSettings)) {
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,RegPath,0,0,&hClientRegKey)== ERROR_SUCCESS ) {
            fRet = LoadRegisteredDriver(hClientRegKey, pDev, lpInterface, pfLoaded, &CurSettings);
            RegCloseKey( hClientRegKey );
        }
        else
            DEBUGMSG(ZONE_LOADER,(TEXT("USBD:LoadGroupDriver(): no key LoadClients\\%s\r\n"),RegPath));
    }
    return fRet;
}

BOOL LoadUSBClient(SDevice * pDev, BOOL * pfLoaded,
        LPCUSB_INTERFACE pInterface)
{
    *pfLoaded = FALSE;

    BOOL fRet = TRUE;

    if(!*pfLoaded)
    {
        LoadGroupDriver( pDev, pfLoaded,
                pInterface,  TRUE, TRUE, pInterface != NULL);
    }
    if(!*pfLoaded)
    {
        LoadGroupDriver(pDev, pfLoaded,
                pInterface, TRUE, FALSE, pInterface != NULL);
    }
    if(!*pfLoaded)
    {
        LoadGroupDriver( pDev, pfLoaded,
                pInterface,  FALSE, TRUE, pInterface != NULL);
    }
    if(!*pfLoaded &&  pInterface != NULL)
    {
        LoadGroupDriver( pDev, pfLoaded,
                pInterface, FALSE, FALSE, TRUE ); 
        
        if (!*pfLoaded ) //Make sure this is absolute last one to call.
            LoadGroupDriver( pDev, pfLoaded,
                pInterface, FALSE, FALSE, FALSE );
    }

    return fRet;
}


/*
    Driver Loading Strategy

    Client drivers are loaded based on registry keys off of Drivers/BuiltIn/USB/LoadClients.
    The key is formatted as follows:

    ...LoadClients/<Group1 Id>/<Group2 Id>/<Group3 Id>/<Driver Name>

    Where each of the group Id strings may be "Default", indicating it should be
    checked for all devices, or may be of the form X, X_Y, or X_Y_Z, where the X, Y, and Z
    depend on the group:


    Group 1)    X = Device Vendor ID
                Y = Device Product ID
                Z = Device Release #

    Group 2)    X = Device Class Code
                Y = Device Sub Class Code
                Z = Device Protocol Code

    Group 3)    X = Interface Class Code
                Y = Interface Sub Class Code
                Z = Interface Protocol Code

    Algorithm:

        Search for the default/default/default driver (this driver loaded for every device)
        Search for the most general group 1 driver
        Search for the most general group 1 + 2 driver
        Search for the most general group 2 driver

        For each interface now try:
        Search for the most general group 1 + 2 + 3 driver
        Search for the most general group 1 + 3 driver
        Search for the most general group 2 + 3 driver
        Search for the most general group 3 driver

        Once the most general driver is found, the drivers attach process is called.
        If the driver accepts control of the device, then no more drivers are loaded
        automatically.  It is the responsibility of the client driver to
        load all other lower edge drivers, and all interface drivers.

*/
BOOL LoadDeviceDrivers(SDevice * pDev, BOOL * pfLoaded)
{
    *pfLoaded = FALSE;

    BOOL fRet = TRUE;

    DEBUGMSG(ZONE_LOADER,(TEXT("USBD:LoadDeviceDrivers - Device VendorId: %u, ProductId: %u, Release: %u\r\n"),
                          pDev->pDeviceInfo->Descriptor.idVendor, pDev->pDeviceInfo->Descriptor.idProduct,
                          pDev->pDeviceInfo->Descriptor.bcdDevice));
    fRet = LoadUSBClient(pDev, pfLoaded, NULL);

    if(!*pfLoaded)
    {
        LPCUSB_CONFIGURATION pConfig = pDev->pDeviceInfo->lpActiveConfig;
        LPCUSB_INTERFACE pInterface = pConfig->lpInterfaces;

        DWORD dwCurIndex = 0;
        for(UINT iInterface = 0 ; 
                iInterface < pConfig->dwNumInterfaces && dwCurIndex < gcMaxPipes &&  fRet && !*pfLoaded; 
                iInterface++, pInterface++) {   
                    
            if (pInterface->Descriptor.bAlternateSetting!=0) { // Alternate setting. Skip it.
                continue;
            }
            ASSERT(pDev->rgbInterfaceIndex[dwCurIndex] == pInterface->Descriptor.bInterfaceNumber);
            if ( (pDev->rgbInterfaceFlag[dwCurIndex] & INTERFACE_FLAG_ACTIVATE) ==0) { // This interface has not been activated yet
                BOOL bLoaded=FALSE;
                DEBUGMSG(ZONE_LOADER,(TEXT("USBD:LoadDeviceDrivers loading driver for interface %u\r\n"),iInterface));
                fRet = LoadUSBClient(pDev, &bLoaded, pInterface);
                if (fRet && bLoaded) { // Driver Loaded for this interface.
                    pDev->rgbInterfaceFlag[dwCurIndex] |= INTERFACE_FLAG_ACTIVATE;
                }
            }
            dwCurIndex++;
        }
        // Check there is interface loaded.
        for(dwCurIndex = 0 ;  dwCurIndex < gcMaxPipes  ; dwCurIndex++) {
            if ((pDev->rgbInterfaceFlag[dwCurIndex] & INTERFACE_FLAG_ACTIVATE) !=0) {
                *pfLoaded=TRUE;
                fRet = TRUE;
            }
        }
    }

    return fRet;
}


BOOL CloseUSBDevice(SDevice * pDev, DWORD dwCode)
{
    EnterCriticalSection(&pDev->csSerializeNotifyRoutine);

    SNotifyList * pNotify = pDev->pNotifyList, *pNext;

    while(pNotify)
    {
        // It is possible that the client will call UnregisterNotificationRoutine(),
        // so don't touch pNotify after calling notify routine.
        pNext = pNotify->pNext;

        if (pNotify->lpNotifyRoutine) {
           DEBUGMSG(ZONE_LOADER,(TEXT("USBD: Calling client device detach, suspend, resume proc\r\n")));
            (*pNotify->lpNotifyRoutine)(pNotify->lpvNotifyParameter,
                                        dwCode, NULL, NULL, NULL, NULL);
            DEBUGMSG(ZONE_LOADER,(TEXT("USBD: Returned from client device suspend, resume, detach proc\r\n")));
        }
        pNotify = pNext;
    }

    LeaveCriticalSection(&pDev->csSerializeNotifyRoutine);

    return TRUE;
}

LPCUSB_ENDPOINT FindEndpoint(LPCUSB_INTERFACE lpInterface,
        UCHAR bEndpointAddress)
{
    UINT cEndpoints = lpInterface->Descriptor.bNumEndpoints;

    PCUSB_ENDPOINT pEndpoint = lpInterface->lpEndpoints;

    for(UINT iEndpoint = 0 ; iEndpoint < cEndpoints ; ++iEndpoint)
    {
        if(pEndpoint->Descriptor.bEndpointAddress == bEndpointAddress)
        {
            return pEndpoint;
        }
        ++pEndpoint;
    }

    return NULL;
}


// Routines to check for valid handles.  Since we don't have system handles,
// just pointers, we have to be careful about freeing memory when clients might
// potentially still hang on to the invalid pointers.  For example, a client
// driver might do a ClosePipe() and then later try to issue a transfer on
// the pipe.  It would be better if we returned an error saying the pipe was
// invalid, rather than partying on freed memory.
// To attempt to accomplish this, we use signatures in our handle structures,
// and place pipe and handle structures on free lists, and only free the memory
// when the device is removed, and the client driver has been unloaded.  Note
// that we are still open to the possibility of the handles being reused off of
// the free list -- this may cause strange behavior, but should not crash USBD.
//
// Additionally, keep a refcnt for each handle, which will be incremented while
// a handle is actively being used by USBD (i.e. when in a USBD call only).
BOOL ReferencePipeHandle(SPipe *pPipe)
{
    if (!pPipe)
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD: NULL Pipe handle.\r\n")));
        return FALSE;
    }
    __try {
        if (pPipe->dwSig != VALID_PIPE_SIG) {
            DEBUGMSG(ZONE_WARNING,(TEXT("!USBD: Invalid Pipe handle 0x%X (sig 0x%X)\r\n"),
                                 pPipe,pPipe->dwSig));
            return FALSE;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
          DEBUGMSG(ZONE_ERROR,(TEXT("!USBD: Exception checking Pipe handle 0x%X\r\n"),pPipe));
          return FALSE;
    }
    InterlockedIncrement((long *)&pPipe->iRefCnt);
    return TRUE;
}
void DereferencePipeHandle(SPipe *pPipe)
{
    ASSERT(pPipe->iRefCnt > 0);
    InterlockedDecrement((long *)&pPipe->iRefCnt);
}

// Validate device handle.  We don't need to refcnt these, since client drivers
// can't close them.
BOOL ValidateDeviceHandle(SDevice *pDev)
{
    __try {
        if (pDev->dwSig != VALID_DEVICE_SIG) {
            DEBUGMSG(ZONE_WARNING,(TEXT("!USBD: Invalid device handle 0x%X (sig 0x%X)\r\n"),
                                 pDev,pDev->dwSig));
            return FALSE;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
          DEBUGMSG(ZONE_ERROR,(TEXT("!USBD: Exception checking device handle 0x%X\r\n"),pDev));
          return FALSE;
    }
    return TRUE;
}

// Validate and lock transfer handle
BOOL ReferenceTransferHandle(STransfer *pTransfer)
{
    if (pTransfer == NULL ) 
        return FALSE;
    
    __try {
        if (pTransfer->dwSig != VALID_TRANSFER_SIG) {
            DEBUGMSG(ZONE_WARNING,(TEXT("!USBD: Invalid transfer handle 0x%X (sig 0x%X)\r\n"),
                                 pTransfer,pTransfer->dwSig));
            return FALSE;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
          DEBUGMSG(ZONE_ERROR,(TEXT("!USBD: Exception checking transfer handle 0x%X\r\n"),pTransfer));
          return FALSE;
    }
    InterlockedIncrement((long *)&pTransfer->iRefCnt);
    return TRUE;
}

void DereferenceTransferHandle(STransfer *pTransfer)
{
    ASSERT(pTransfer->iRefCnt > 0);
    InterlockedDecrement((long *)&pTransfer->iRefCnt);
}

// Get a pipe handle off of the device free list, or allocate and init a new one.
SPipe *GetPipeObject(SDevice *pDev)
{
    SPipe *pPipe = NULL;
    EnterCriticalSection(&pDev->csPipeLock);
    if (pDev->pFreePipeList) {
        pPipe = pDev->pFreePipeList;
        pDev->pFreePipeList = pPipe->pNext;
    }
    LeaveCriticalSection(&pDev->csPipeLock);
    if (pPipe == NULL) {
        pPipe = new SPipe;
        if (pPipe == NULL)
            return NULL;
        memset(pPipe,0,sizeof(SPipe));
        pPipe->pDevice = pDev;
        InitializeCriticalSection(&pPipe->csTransferLock);
    }
    pPipe->dwSig = VALID_PIPE_SIG;
    ASSERT(pPipe->iRefCnt == 0);
    return pPipe;
}
// Put a pipe handle back on the free list for the device. First, make
// sure no one is still referencing it.
void FreePipeObject(SPipe *pPipe)
{
    ULONG iTry=0;
    SDevice *pDev = pPipe->pDevice;
    pPipe->dwSig = FREE_PIPE_SIG;

    // Wait for refcnt to go to 0
    while (pPipe->iRefCnt && (iTry++<10)) {
        DEBUGMSG(ZONE_WARNING,
                 (TEXT("USBD:FreePipeObject(0x%X), waiting for refcnt:%u, try:%u\r\n"),
                 pPipe,pPipe->iRefCnt,iTry));
        Sleep(500);
    }
    if (pPipe->iRefCnt)
    {
        // Someone won't let go of resource, don't stick it on free list...
        DEBUGMSG(ZONE_ERROR,(TEXT("!USBD:FreePipeObject(0x%X) timed out waiting for refcnt!\r\n"),pPipe));
        ASSERT(FALSE);
        return;
    }

    EnterCriticalSection(&pDev->csPipeLock);
    pPipe->pNext = pDev->pFreePipeList;
    pDev->pFreePipeList = pPipe;
    LeaveCriticalSection(&pDev->csPipeLock);
}

// Free memory and system resources associated with a pipe handle
void FreePipeObjectMem(SPipe *pPipe)
{
    DeleteCriticalSection(&pPipe->csTransferLock);
    delete pPipe;
}

// Get a transfer handle off of the pipe free list.
STransfer * GetTransferObject(SPipe *pPipe, DWORD dwIsochFrames)
{
    STransfer *pTransfer=NULL;

    EnterCriticalSection(&pPipe->csTransferLock);
    if (pPipe->pFreeTransferList) {
        pTransfer = pPipe->pFreeTransferList;
        pPipe->pFreeTransferList = pTransfer->pNext;
    }
    LeaveCriticalSection(&pPipe->csTransferLock);

    if (pTransfer == NULL) {
        // Allocate a new transfer struct and set it up
        pTransfer = new STransfer;
        if (pTransfer == NULL)
            return NULL;
        memset(pTransfer,0,sizeof(STransfer));
        InitializeCriticalSection(&pTransfer->csWaitObject);
        pTransfer->pPipe = pPipe;
    }

    ASSERT(pTransfer->iRefCnt == 0);

    // If array lengths aren't big enough, free old arrays and allocate
    // new ones. Typically, isoch transfers on the same pipe will
    // use the same # of frames, so this path should usually only
    // be taken for newly allocated transfer structs.
    if (pTransfer->cFrames < dwIsochFrames) {
        if(pTransfer->adwIsochErrors) {
            delete [] pTransfer->adwIsochErrors;
            pTransfer->adwIsochErrors = NULL;
        }
        if(pTransfer->adwIsochLengths) {
            delete [] pTransfer->adwIsochLengths;
            pTransfer->adwIsochLengths = NULL;
        }
        pTransfer->cFrames = dwIsochFrames;
        if (dwIsochFrames) {
            pTransfer->adwIsochErrors = new DWORD[dwIsochFrames];
            pTransfer->adwIsochLengths = new DWORD[dwIsochFrames];
            if ((pTransfer->adwIsochErrors == NULL) ||
                (pTransfer->adwIsochLengths == NULL)) {
                FreeTransferObjectMem(pTransfer);
                return NULL;
            }
        }
    }
    // OK, flag handle as valid
    pTransfer->dwSig = VALID_TRANSFER_SIG;
    return pTransfer;
}

// Put transfer handle on pipe free list
void FreeTransferObject(STransfer *pTransfer)
{
    ULONG iTry=0;
    SPipe *pPipe = pTransfer->pPipe;
    // Mark transfer as freed for ReferenceTransferHandle().  Watch for race condition
    // if two threads are trying to close simultaneously (typically happens in device
    // detach processing).
    EnterCriticalSection(&pPipe->csTransferLock);
    if (pTransfer->dwSig != VALID_TRANSFER_SIG) {
        DEBUGMSG(ZONE_WARNING,(TEXT("USBD:FreeTransferObject(0x%X), already free, refcnt:%u\r\n"),
                               pTransfer,pTransfer->iRefCnt));
        LeaveCriticalSection(&pPipe->csTransferLock);
        return;
    }
    pTransfer->dwSig = FREE_TRANSFER_SIG;
    LeaveCriticalSection(&pPipe->csTransferLock);

    // Wait for refcnt to go to 0
    while (pTransfer->iRefCnt && (++iTry < 10)) {
        DEBUGMSG(ZONE_WARNING,
                 (TEXT("USBD:FreeTransferObject(0x%X), waiting for refcnt:%u, try:%u\r\n"),
                 pTransfer,pTransfer->iRefCnt,iTry));
        Sleep(500);
    }

    if (pTransfer->iRefCnt)
    {
        // Someone won't let go of resource, don't stick it on free list...
        DEBUGMSG(ZONE_ERROR,(TEXT("!USBD:FreeTransferObject(0x%X) timed out waiting for refcnt!\r\n"),pTransfer));
        ASSERT(FALSE);
        return;
    }

    if (pTransfer->pWait)
    {
        FreeWaitObject(pTransfer->pWait);
        pTransfer->pWait = NULL;
    }
    EnterCriticalSection(&pPipe->csTransferLock);
    if (pTransfer->pvBuffer) {
        CeFreeAsynchronousBuffer(pTransfer->pvBuffer,pTransfer->lpvOrigUserBuffer,pTransfer->dwUserBufferSize,(((pTransfer->dwFlags& USB_IN_TRANSFER)!=0)?ARG_O_PTR:ARG_I_PTR)|MARSHAL_FORCE_ALIAS);
        pTransfer->pvBuffer = NULL;
    }
    if (pTransfer->lpMappedControlHeader) {
        CeFreeAsynchronousBuffer((PVOID)pTransfer->lpMappedControlHeader, (PVOID)pTransfer->lpOrigControlHeader,sizeof(USB_DEVICE_REQUEST),ARG_I_PTR|MARSHAL_FORCE_ALIAS);
        pTransfer->lpMappedControlHeader = NULL;
    }
    if (pTransfer->lpdwMappedLengths) {
        CeFreeAsynchronousBuffer(pTransfer->lpdwMappedLengths, pTransfer->lpdwOrigLengths, pTransfer->cFrames*sizeof(LPDWORD),ARG_I_PTR|MARSHAL_FORCE_ALIAS);
        pTransfer->lpdwMappedLengths = NULL;
    }
    pTransfer->pNext = pPipe->pFreeTransferList;
    pPipe->pFreeTransferList = pTransfer;
    LeaveCriticalSection(&pPipe->csTransferLock);
}

// Free memory and resources held by transfer object.  Must
// have been fully initialized in GetTransferObject().
void FreeTransferObjectMem(STransfer *pTransfer)
{
    DeleteCriticalSection(&pTransfer->csWaitObject);
    if(pTransfer->adwIsochErrors)
        delete [] pTransfer->adwIsochErrors;
    if(pTransfer->adwIsochLengths)
        delete [] pTransfer->adwIsochLengths;
    delete pTransfer;
}

SWait * GetWaitObject()
{
    SWait * pWait = new SWait;

    if (pWait == NULL)
        return NULL;

    pWait->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if(pWait->hEvent == NULL)
    {
        delete pWait;
        pWait = NULL;
    }
    return pWait;
}


BOOL FreeWaitObject(SWait * pWait)
{
    if(pWait)
    {
        if(pWait->hEvent)
            CloseHandle(pWait->hEvent);
        delete pWait;
    }

    return TRUE;
}

