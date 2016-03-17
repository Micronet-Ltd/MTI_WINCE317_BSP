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

usbddrv.cpp

Abstract:

This file contains code for the USBD module of the Universal Serial
Bus driver for Windows CE.  The USBD driver is responsible for loading
client drivers, and providing an interface to the USB host controller
to client drivers.

  +--------------+
  | USB Client   |
  |   Driver     |
  +--------------+
        /|\
         |       USBDI Interface
        \|/
  +--------------+
  |  USBD        |
  |  Driver      |
  +--------------+
        /|\
         |       HCD Interface
        \|/
  +--------------+
  |  HCD (OHCD or|
  |  UHCD) driver|
  +--------------+
        /|\
         |       HC Interface
        \|/
  +--------------------+               +------------------------+
  | Host controller HW |               | USB Device (function)  |
  +--------------------+               +------------------------+
        /|\                                        /|\
         |                                          |
         +------------------------------------------+

Functions:

TranslateStringDescr
LoadGenericInterfaceDriver
OpenClientRegistryKey
GetClientRegistryPath
FindInterface
TakeFrameLengthControl
ReleaseFrameLengthControl
SetFrameLength
GetFrameNumber
GetFrameLength
OpenPipe
ResetPipe
AbortPipeTransfers
GetInterface
SetInterface
GetDescriptor
SetDescriptor
SetFeature
ClearFeature
GetStatus
SyncFrame
IssueVendorTransfer
IssueControlTransfer
IssueBulkTransfer
IssueInterruptTransfer
IssueIsochTransfer
CloseTransfer
AbortTransfer
ClosePipe
IsTransferComplete
GetTransferError
RegisterNotificationRoutine
UnRegisterNotificationRoutine
GetTransferStatus
GetIsochResults
GetDeviceInfo
GetUSBDVersion
IsPipeHalted

Notes:

@doc DRIVERS
--*/
#pragma warning (3 : 4100 4101 4705 4706)

#include <windows.h>
#include <netui.h>
#include <windev.h>

#include "usbdi.h"
#include "hcdi.h"

#include "usbdinc.hpp"
#include "usbd.hpp"
#include "usbdinc.hpp"
#include "usbdobj.hpp"

#define WAIT_DURATION_USB_TIMEOUT 4000

#ifdef DEBUG
#define DBG_INIT     0x0001
#define DBG_LOADER   0x0002
#define DBG_CONFIG   0x0004
#define DBG_PIPE     0x0008
#define DBG_VENDOR   0x0010
#define DBG_CONTROL  0x0020
#define DBG_BULK     0x0040
#define DBG_INTR     0x0080
#define DBG_ISOCH    0x0100
#define DBG_API      0x0200
#define DBG_WARN     0x4000
#define DBG_ERROR    0x8000
DBGPARAM dpCurSettings = {
    TEXT("USBD"), {
        TEXT("Init"),TEXT("Loader"),TEXT("Config"),TEXT("Pipe"),
        TEXT("Vendor"),TEXT("Control"),TEXT("Bulk"),TEXT("Interrupt"),
        TEXT("Isoch"),TEXT("API"),TEXT("Unused"),TEXT("Unused"),
        TEXT("Unused"),TEXT("Unused"),TEXT("Warning"),TEXT("Error")},
    DBG_INIT | DBG_PIPE | DBG_ERROR
};

// For decoding endpoint descriptor attributes
static const TCHAR *aszTypeStrings[] =
{
    TEXT("Control"),TEXT("Isoch"),TEXT("Bulk"),TEXT("Interrupt")
};

#endif
LPCWSTR gcszUsbConfigureEntry = L"bConfigurationValue";
LPCWSTR gcszDonotPromptUser = L"DoNotPromptUser";

extern "C" BOOL WINAPI DllMain(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    UnusedParameter(hinstDLL);
    UnusedParameter(dwReason);
    UnusedParameter(lpvReserved);
    if (dwReason == DLL_PROCESS_ATTACH) {
      DEBUGREGISTER((HINSTANCE)hinstDLL);
      DEBUGMSG(ZONE_INIT,(TEXT("USBD: DLL attach\r\n")));
      DisableThreadLibraryCalls((HMODULE) hinstDLL);
    }
    return TRUE;
}


extern "C" BOOL
HcdAttach(
    LPVOID lpvHcd,
    LPCHCD_FUNCS lpHcdFuncs,
    LPLPVOID lppvContext)
{
    *lppvContext = NULL;
    if (lpHcdFuncs && lpHcdFuncs->dwCount >= sizeof(HCD_FUNCS)) {
        SHcd * pHcd = new SHcd;
        if (pHcd == NULL)
            return FALSE;
        memset(pHcd,0,sizeof(SHcd));

        pHcd->pvHcd = lpvHcd;
        pHcd->pHcdFuncs = lpHcdFuncs;

        InitializeCriticalSection(&pHcd->csFrameLengthControl);

        *lppvContext = (LPVOID)pHcd;
        DEBUGMSG(ZONE_INIT,(TEXT("+USBD:HcdAttach, hcd: 0x%X\r\n"),pHcd));
        return TRUE;
    }
    else {
        DEBUGMSG(ZONE_ERROR,(TEXT("!!!Error: USBD:HcdAttach,lpHcdFuncs is NULL or Wrong Version : 0x%X\r\n"), lpHcdFuncs));
        return FALSE;
    }
        
}


extern "C" BOOL HcdDetach(LPVOID lpvContext)
{
    ASSERT(lpvContext);

    SHcd * pHcd = (SHcd *)lpvContext;

    DeleteCriticalSection(&pHcd->csFrameLengthControl);

    delete pHcd;

    return TRUE;
}

// Call dialog box in NETUI to get driver name from user.  If platform doesn't
// have UI, or user cancels dialog, this will return FALSE, otherwise returns
// the driver name entered by the user.
static BOOL
GetClientDriverName(LPWSTR szDriverName, int BufSize)
{
   HANDLE hEvent;
   // We share dialog with PCMCIA
   GETDRIVERNAMEPARMS DriverNameParms;
   DriverNameParms.Socket = 0;
   DriverNameParms.PCCardType = PCCARDTYPE_USB;

   // In case we're booting, wait for window manager to come up
    hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, (TEXT("SYSTEM/GweApiSetReady")));
    if (hEvent == NULL) return FALSE;
    WaitForSingleObject(hEvent, INFINITE);

   wcsncpy(DriverNameParms.DriverName,szDriverName,
           sizeof(DriverNameParms.DriverName)/sizeof(DriverNameParms.DriverName[0]));
   BOOL fRet = CallGetDriverName(NULL,&DriverNameParms);
   if (!fRet) {
      DEBUGMSG(ZONE_LOADER,(TEXT("!USBD:GetClientDriverName, error in CallGetDriverName\r\n")));
      return FALSE;
   }
   wcsncpy(szDriverName,DriverNameParms.DriverName,BufSize);
   return TRUE;
}

/*
 * @func   BOOL | HcdSelectConfiguration | Select Configuration according registry setting..
 * @rdesc  Return TRUE if successful, FALSE if registry is not found.
 * @comm   If return failse, the default configuration should be used.
 *         1.2 or later.
 */
extern "C" BOOL HcdSelectConfiguration(LPCUSB_DEVICE lpDeviceInfo, LPBYTE lpbConfigure)
{
    if (lpbConfigure && lpDeviceInfo) {
        BOOL bReturn = FALSE;
        TCHAR RegPath[MAX_PATH];    
        HKEY hClientRegKey;
        DWORD dwData = 0;
        if (!bReturn && ConvertToClientRegistry(RegPath,MAX_PATH,lpDeviceInfo,NULL,TRUE,TRUE,FALSE,NULL)) {
            DWORD dwType = 0;
            DWORD dwLength = sizeof(DWORD);
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,RegPath,0,0,&hClientRegKey) == ERROR_SUCCESS ) { // We found Registry 
                if (RegQueryValueEx( hClientRegKey,gcszUsbConfigureEntry, NULL,&dwType, (LPBYTE)&dwData, &dwLength) == ERROR_SUCCESS)
                    bReturn = TRUE;
                RegCloseKey( hClientRegKey );
            }
        }
        if (!bReturn && ConvertToClientRegistry(RegPath,MAX_PATH,lpDeviceInfo,NULL,TRUE,FALSE,FALSE,NULL)) {
            DWORD dwType = 0;
            DWORD dwLength = sizeof(DWORD);
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,RegPath,0,0,&hClientRegKey) == ERROR_SUCCESS ) { // We found Registry 
                if (RegQueryValueEx( hClientRegKey,gcszUsbConfigureEntry, NULL,&dwType, (LPBYTE)&dwData, &dwLength) == ERROR_SUCCESS)
                    bReturn = TRUE;
                RegCloseKey( hClientRegKey );
            }
        }
        if (!bReturn && ConvertToClientRegistry(RegPath,MAX_PATH,lpDeviceInfo,NULL,FALSE,TRUE,FALSE,NULL)) {
            DWORD dwType = 0;
            DWORD dwLength = sizeof(DWORD);
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,RegPath,0,0,&hClientRegKey) == ERROR_SUCCESS ) { // We found Registry 
                if (RegQueryValueEx( hClientRegKey,gcszUsbConfigureEntry, NULL,&dwType, (LPBYTE)&dwData, &dwLength) == ERROR_SUCCESS)
                    bReturn = TRUE;
                RegCloseKey( hClientRegKey );
            }
        }
        if (bReturn) { // We have Return. We need search for the index.
	     for (BYTE bIndex= 0; lpDeviceInfo->lpConfigs != NULL && bIndex < lpDeviceInfo->Descriptor.bNumConfigurations ; bIndex ++) {
	         if (lpDeviceInfo->lpConfigs[bIndex].Descriptor.bConfigurationValue== (BYTE)dwData) {
	              *lpbConfigure = bIndex;
	              return TRUE;
	         }
	                
	     }
        }
        
    }
    return FALSE;
}

extern "C" BOOL HcdDeviceAttached(LPVOID lpvContext, UINT iDevice,
        UINT iEndpointZero, LPCUSB_DEVICE lpDeviceInfo,
        LPLPVOID lppvDeviceDetach)
{
    BOOL fRet = TRUE, fLoaded = FALSE;
    WCHAR szDllName[USB_MAX_LOAD_STRING] = {0};

    *lppvDeviceDetach = NULL;

    SHcd * pHcd = (SHcd *)lpvContext;
    SDevice * pDev = new SDevice;
    if (pDev == NULL)
        return FALSE;
    memset(pDev,0,sizeof(SDevice));
    pDev->dwSig = VALID_DEVICE_SIG;
    pDev->pHcd = pHcd;
    pDev->iDevice = iDevice;
    pDev->pDeviceInfo = lpDeviceInfo;

    InitializeCriticalSection(&pDev->csPipeLock);
    InitializeCriticalSection(&pDev->csSerializeNotifyRoutine);
    InitializeCriticalSection(&pDev->csLibList);

    for(UINT iIndex = 0 ; iIndex < gcMaxPipes ; ++iIndex) {
        pDev->apPipes[iIndex] = NULL;
        pDev->rgbInterfaceIndex[iIndex] = INVALID_INTERFACE_INDEX;
        pDev->rgbInterfaceFlag[iIndex] = 0;
    }

    SPipe * pPipe = GetPipeObject(pDev);
    if (pPipe) {

        pDev->apPipes[gcEndpointZero] = pPipe;

        pPipe->iEndpointIndex = iEndpointZero; //value we use to the HCD

        DEBUGMSG(ZONE_LOADER,(TEXT("+USBD:HcdDeviceAttached, Interfaces:%u\r\n"),
                              pDev->pDeviceInfo->lpActiveConfig->Descriptor.bNumInterfaces));

        LPCUSB_INTERFACE pInterface =pDev->pDeviceInfo->lpActiveConfig->lpInterfaces;
        UINT iPos = 0;
        for (iIndex =0 ; iIndex < pDev->pDeviceInfo->lpActiveConfig->dwNumInterfaces && iPos < gcMaxPipes ; ++iIndex) {
            if (pInterface && pInterface->Descriptor.bAlternateSetting == 0 ) { // No Alternate setting interface
                pDev->rgbInterfaceIndex[iPos] = pInterface->Descriptor.bInterfaceNumber;
                ASSERT(pInterface->Descriptor.bInterfaceNumber != INVALID_INTERFACE_INDEX);
                iPos ++ ;
            }
            pInterface ++;
        }
        while (fRet && !fLoaded) {
           // Attempt to load client driver based on registry settings
           fRet = LoadDeviceDrivers(pDev, &fLoaded);
           if (fRet && !fLoaded) {
              // Ask the registry whether we should prompt the user for a USB
              // client driver
              TCHAR RegPath[MAX_PATH];
              HRESULT hr = StringCchPrintf(RegPath, MAX_PATH, TEXT("%s\\%s"),
                  gcszUsbRegKey, gcszLoadClients);
              if (FAILED(hr)) {
                  return FALSE;
              }

              HKEY hRegKey;
              hr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegPath, 0, 0, &hRegKey);
              if(ERROR_SUCCESS != hr) {
                  RegCloseKey(hRegKey);
                  return FALSE;
              }

              DWORD dwRegType = 0;
              DWORD dwNoPrompt = 0;
              DWORD dwBufSize = sizeof(DWORD);
              
              hr = RegQueryValueEx(hRegKey, 
                                      gcszDonotPromptUser, 
                                      NULL, 
                                      &dwRegType, 
                                      (BYTE*)&dwNoPrompt, 
                                      &dwBufSize);
              if(ERROR_SUCCESS != hr || REG_DWORD != dwRegType || 
                    sizeof(DWORD) != dwBufSize)
              {
                  dwNoPrompt = 0;
              }

              RegCloseKey(hRegKey);
              
              if(!dwNoPrompt) {
              // No driver found, prompt user for driver name
                if (!GetClientDriverName(szDllName, USB_MAX_LOAD_STRING))
                    break;
                // Call client install proc, then retry load procedure
                else if (!InstallClientDriver(szDllName))
                    // "Error installing USB driver <driver name>"
                    CallNetMsgBox(NULL,NMB_FL_OK|NMB_FL_TITLEUSB,
                                  NETUI_GETNETSTR_USB_INSTALL_FAILURE,szDllName);
              }
              else {
                break;
              }
           }
        }
        if (!fLoaded ) { // We does not load all the driver.
            // Check at least one drivers is loaded before free everything
            for (DWORD dwIndex=0;dwIndex<gcMaxPipes;dwIndex++)
                if (pDev->rgbInterfaceIndex[dwIndex] != INVALID_INTERFACE_INDEX) {
                    if (pDev->rgbInterfaceFlag[dwIndex] & INTERFACE_FLAG_ACTIVATE) {
                        fLoaded=TRUE;
                        break;
                    }
                }
        }

        // Consider it an error if we couldn't get a driver for this device
        if (!fLoaded)
           fRet = FALSE;

        if(fRet)
        {
            *lppvDeviceDetach = (LPVOID)pDev;
            return TRUE;
        }
    }
    RETAILMSG(1,(TEXT("!USBD: Could not load driver for attached device\r\n")));
    if (pPipe)
        FreePipeObjectMem(pPipe);

    DeleteCriticalSection(&pDev->csPipeLock);
    DeleteCriticalSection(&pDev->csSerializeNotifyRoutine);
    DeleteCriticalSection(&pDev->csLibList);
    delete pDev;
    return FALSE;
}

extern "C" BOOL
HcdDeviceDetached(LPVOID lpvDeviceDetach)
{
    BOOL fRet;

    SDevice * pDev = (SDevice *)lpvDeviceDetach;

    if(!pDev)
        return TRUE;

    SPipe * pPipe, *pDfltPipe;

    DEBUGMSG(ZONE_LOADER,(TEXT("+USBD:HcdDeviceDetached, pDev:0x%X"),pDev));

    EnterCriticalSection(&pDev->csPipeLock);
    pDfltPipe = pDev->apPipes[gcEndpointZero];
    LeaveCriticalSection(&pDev->csPipeLock);

    // Close all pipes
    DEBUGMSG(ZONE_LOADER,(TEXT("USBD:HcdDeviceDetached: Closing all pipes\r\n")));
    for (UINT iPipe = gcEndpointZero; iPipe < gcMaxPipes ; ++iPipe)
    {
        if (pDev->apPipes[iPipe])
            ClosePipe(pDev->apPipes[iPipe]);
    }

    // Notify client drivers of device removal
    fRet = CloseUSBDevice(pDev,USB_CLOSE_DEVICE);

    // Free client Dlls
    while(pDev->pDriverLibs)
    {
        SDriverLibs * pNext;

        if(pDev->pDriverLibs->hDriver) {
            DEBUGMSG(ZONE_LOADER,(TEXT("USBD: Unloading client DLL...\r\n")));
            FreeLibrary(pDev->pDriverLibs->hDriver);
            DEBUGMSG(ZONE_LOADER,(TEXT("USBD: Client DLL unloaded\r\n")));
        }
        pNext = pDev->pDriverLibs->pNext;
        delete pDev->pDriverLibs;
        pDev->pDriverLibs = pNext;
    }

    // all client drivers are removed so we don't need to lock the notfication
    while (pDev->pNotifyList)
    {
        SNotifyList * pNext;

        pNext = pDev->pNotifyList->pNext;
        delete pDev->pNotifyList;
        pDev->pNotifyList = pNext;
    }

    // Free all pipe structures in our free list
    while (pDev->pFreePipeList) {
        pPipe = pDev->pFreePipeList;
        pDev->pFreePipeList = pPipe->pNext;
        FreePipeObjectMem(pPipe);
    }

    // if we hold frame length control then we release it here!
    ReleaseFrameLengthControl(pDev);

    DeleteCriticalSection(&pDev->csPipeLock);
    DeleteCriticalSection(&pDev->csSerializeNotifyRoutine);
    DeleteCriticalSection(&pDev->csLibList);

    pDev->dwSig = 0;
    delete pDev;

    DEBUGMSG(ZONE_LOADER,(TEXT("-USBD:HcdDeviceDetached, pDev:0x%X\r\n"),pDev));
    return fRet;
}

extern "C" BOOL
HcdDeviceSuspendeResumed(LPVOID lpvDeviceDetach, BOOL fResumed)
{

    SDevice * pDev = (SDevice *)lpvDeviceDetach;
    if(!pDev)
        return FALSE;
    BOOL fRet = CloseUSBDevice(pDev, fResumed? USB_RESUMED_DEVICE: USB_SUSPENDED_DEVICE );
    return fRet;
}

/*
 * @func   BOOL | TranslateStringDescr | Translate a USB string descriptor into a NULL terminated string.
 * @rdesc  Returns TRUE if lpStringDescr points to a valid USB string descriptor.
 * @comm   If the string to be translated would overflow the supplied buffer,
 *         it is truncated to fit.
 */
extern "C" BOOL
TranslateStringDescr(
    LPCUSB_STRING_DESCRIPTOR lpStringDescr,  // @parm [IN] - Pointer to USB string descriptor
    LPWSTR szString,                         // @parm [OUT]- Buffer to receive string data
    DWORD cchStringLength)                   // @parm [IN] - Size of string buffer (characters)
{
    ASSERT(sizeof(szString[0]) == sizeof(lpStringDescr->bString[0]));

    szString[0] = 0;

    if(lpStringDescr->bDescriptorType != USB_STRING_DESCRIPTOR_TYPE)
        return FALSE;

    UINT cchString = (lpStringDescr->bLength - sizeof(USB_COMMON_DESCRIPTOR)) /
            sizeof(lpStringDescr->bString[0]);
    --cchStringLength; // normalize to actual # of characters we can copy

    if(cchString > cchStringLength)
        cchString = cchStringLength;

    memcpy(szString, &lpStringDescr->bString, cchString * sizeof(szString[0]));

    szString[cchString] = 0;

    return TRUE;
}

/*
 * @func   BOOL | LoadGenericInterfaceDriver | Called by clients to load driver for an interface
 * @rdesc  Returns TRUE if a driver was successfully loaded for interface, otherwise FALSE.
 * @comm   If a client accepts control of a device, this function must be called
 *         to load a driver for any uncontrolled interfaces.
 */
extern "C" BOOL
LoadGenericInterfaceDriver(
    USB_HANDLE hDevice,           // @parm [IN] - Handle to USB device (passed in <f USBDeviceAttach>)
    LPCUSB_INTERFACE lpInterface) // @parm [IN] - Pointer to USB_INTERFACE struct for interface
{
    SDevice * pDev = (SDevice *)hDevice;

    BOOL fRet = TRUE;
    BOOL fLoaded = FALSE;
    DEBUGMSG(ZONE_LOADER,(TEXT("+USBD:LoadGenericInterfaceDriver\r\n")));

    fRet = LoadUSBClient(pDev, &fLoaded, lpInterface);

    if(fLoaded == FALSE)
        fRet = FALSE;
    DEBUGMSG(ZONE_LOADER,(TEXT("-USBD:LoadGenericInterfaceDriver\r\n")));
    return fRet;
}

/*
 * @func  BOOL | RegisterClientDriverID | Called to register a unique string to identify a client driver.
 * @rdesc Return TRUE if client id was successfully registered, FALSE if an error occurred.
 */
extern "C" BOOL
RegisterClientDriverID(
    LPCWSTR szUniqueDriverId)
{
    BOOL fRet = FALSE;
    DWORD dwGarbage;

    HKEY hkUsb;

    if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, gcszUsbRegKey, 0, NULL, 0, 0, NULL,
            &hkUsb, &dwGarbage) == ERROR_SUCCESS)
    {
        HKEY hkDrivers;

        if(RegCreateKeyEx(hkUsb, gcszDriverIDs, 0, NULL, 0, 0, NULL,
                &hkDrivers, &dwGarbage) == ERROR_SUCCESS)
        {
            HKEY hkThisDriver;

            if(RegCreateKeyEx(hkDrivers, szUniqueDriverId, 0, NULL, 0, 0,
                    NULL, &hkThisDriver, &dwGarbage) == ERROR_SUCCESS)
            {
                fRet = TRUE;

                RegCloseKey(hkThisDriver);
            }
            RegCloseKey(hkDrivers);
        }
        RegCloseKey(hkUsb);
    }

    return fRet;
}

extern "C" BOOL UnRegisterClientDriverID(LPCWSTR szUniqueDriverId)
{
    BOOL fRet = FALSE;

    HKEY hkUsb;

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, gcszUsbRegKey, 0, 0, &hkUsb)
            == ERROR_SUCCESS)
    {
        HKEY hkDrivers;

        if(RegOpenKeyEx(hkUsb, gcszDriverIDs, 0, 0, &hkDrivers)
                == ERROR_SUCCESS)
        {
            if(RegDeleteKey(hkDrivers, szUniqueDriverId) == ERROR_SUCCESS)
            {
                fRet = TRUE;
            }
            RegCloseKey(hkDrivers);
        }
        RegCloseKey(hkUsb);
    }

    return fRet;
}

/*
 * @func   HKEY | OpenClientRegistryKey | Opens registry key associated with client driver.
 * @rdesc  Return handle to open key, or NULL if key doesn't exist, or other error occurs.
 * @comm   Client key is created in <f RegisterClientDriverId>.
 * @xref   <f RegisterClientDriverId>
 */
extern "C" HKEY
OpenClientRegistryKey(
    LPCWSTR szUniqueDriverId)  // @parm [IN] - Unique driver Id string
{
    HKEY hkUsb;
    HKEY hkDriverKey = NULL;

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, gcszUsbRegKey, 0, 0, &hkUsb)
            == ERROR_SUCCESS)
    {
        HKEY hkDrivers;

        if(RegOpenKeyEx(hkUsb, gcszDriverIDs, 0, 0, &hkDrivers)
                == ERROR_SUCCESS)
        {
            if(RegOpenKeyEx(hkDrivers, szUniqueDriverId, 0, 0, &hkDriverKey)
                    != ERROR_SUCCESS)
            {
                hkDriverKey = NULL;
            }
            RegCloseKey(hkDrivers);
        }
        RegCloseKey(hkUsb);
    }

    return hkDriverKey;
}
/*
 * @func   BOOL | GetClientRegistryPath | reutrn registry path associated with client driver.
 * @rdesc  Return handle to open key, or NULL if key doesn't exist, or other error occurs.
 * @comm   Client key is created in <f RegisterClientDriverId>.
 * @xref   <f RegisterClientDriverId>
 */
extern "C" BOOL
GetClientRegistryPath(LPWSTR szRegistryPath, DWORD dwRegPathUnit, LPCWSTR szUniqueDriverId)
{
    if (szRegistryPath && dwRegPathUnit && szUniqueDriverId) {
        HRESULT rResult;
        if (SUCCEEDED(rResult=StringCchCopy(szRegistryPath,dwRegPathUnit,gcszUsbRegKey)) &&
                SUCCEEDED(rResult=StringCchCat(szRegistryPath,dwRegPathUnit,TEXT("\\"))) &&                
                SUCCEEDED(rResult=StringCchCat(szRegistryPath,dwRegPathUnit,gcszDriverIDs)) &&                
                SUCCEEDED(rResult=StringCchCat(szRegistryPath,dwRegPathUnit,TEXT("\\"))) &&                
                SUCCEEDED(rResult=StringCchCat(szRegistryPath,dwRegPathUnit,szUniqueDriverId)) ) {
            return TRUE;
        }
        else {
            SetLastError(HRESULT_CODE(rResult));
        }
    }
    else
        SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}

/*
 * @func   BOOL | RegisterClientSettings | Called to set up client driver load settings.
 * @rdesc  Returns TRUE if settings were successfully configured, otherwise FALSE.
 * @comm   This function should be called by client drivers in their installation routine.
 *         It sets up appropriate registry keys based on lpDriverSettings so that the
 *         driver will be loaded whenever the device is attached.
 * @xref   <t USB_DRIVER_SETTINGS>  <f UnRegisterClientSettings>
 */
extern "C" BOOL
RegisterClientSettings(
    LPCWSTR szDriverLibFile,                // @parm [IN] - Client driver DLL name
    LPCWSTR szUniqueDriverId,               // @parm [IN] - Unique client driver id string
    LPCWSTR szReserved,                     // @parm [IN] - Should be set to 0 to ensure compatibility
                                            //              with future versions of Windows CE
    LPCUSB_DRIVER_SETTINGS lpDriverSettings)// @parm [IN] - Specifies how driver is to be loaded
{
    UnusedParameter(szReserved);

    BOOL fRet = FALSE;
    WCHAR szTemp[gcMaxDriverString];
    DWORD dwGarbage;

    HKEY hkUsb;

    if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, gcszUsbRegKey, 0, NULL, 0, 0, NULL,
            &hkUsb, &dwGarbage) == ERROR_SUCCESS)
    {
        HKEY hkLoadOrder;

        if(RegCreateKeyEx(hkUsb, gcszLoadClients, 0, NULL, 0, 0, NULL,
                &hkLoadOrder, &dwGarbage) == ERROR_SUCCESS)
        {
            HKEY hkGroup1;

            GetSettingString(szTemp, gcMaxDriverString, lpDriverSettings->dwVendorId,
                    lpDriverSettings->dwProductId,
                    lpDriverSettings->dwReleaseNumber);

            if(RegCreateKeyEx(hkLoadOrder, szTemp, 0, NULL, 0, 0, NULL,
                    &hkGroup1, &dwGarbage) == ERROR_SUCCESS)
            {
                HKEY hkGroup2;

                GetSettingString(szTemp, gcMaxDriverString, lpDriverSettings->dwDeviceClass,
                        lpDriverSettings->dwDeviceSubClass,
                        lpDriverSettings->dwDeviceProtocol);

                if(RegCreateKeyEx(hkGroup1, szTemp, 0, NULL, 0, 0, NULL,
                        &hkGroup2, &dwGarbage) == ERROR_SUCCESS)
                {
                    HKEY hkGroup3;

                    GetSettingString(szTemp, gcMaxDriverString, lpDriverSettings->dwInterfaceClass,
                            lpDriverSettings->dwInterfaceSubClass,
                            lpDriverSettings->dwInterfaceProtocol);

                    if(RegCreateKeyEx(hkGroup2, szTemp, 0, NULL, 0, 0, NULL,
                            &hkGroup3, &dwGarbage) == ERROR_SUCCESS)
                    {
                        HKEY hkSettings;

                        if(RegCreateKeyEx(hkGroup3, szUniqueDriverId, 0, NULL,
                                0, 0, NULL, &hkSettings, &dwGarbage)
                                == ERROR_SUCCESS)
                        {
                            if(RegSetValueEx(hkSettings, gcszDllName, 0,
                                    REG_SZ, (LPBYTE)szDriverLibFile,
                                    (lstrlen(szDriverLibFile) + 1) *
                                    sizeof(szDriverLibFile[0]))
                                    == ERROR_SUCCESS)
                            {
                                fRet = TRUE;
                            }

                            RegCloseKey(hkSettings);
                        }

                        RegCloseKey(hkGroup3);
                    }

                    RegCloseKey(hkGroup2);
                }

                RegCloseKey(hkGroup1);
            }
            RegCloseKey(hkLoadOrder);
        }
        RegCloseKey(hkUsb);
    }
    return fRet;
}


/*
 * @func   BOOL | UnRegisterClientSettings | Deregister client driver settings.
 * @rdesc  Returns TRUE if settings were successfully removed, otherwise FALSE.
 * @comm   This function should be called by client drivers in their installation routine.
 *         It sets up appropriate registry keys based on lpDriverSettings param so that the
 *         driver will be loaded whenever the device is attached.
 * @xref   <t USB_DRIVER_SETTINGS>  <f RegisterClientSettings>
 */
extern "C" BOOL
UnRegisterClientSettings(
    LPCWSTR szUniqueDriverId,                // @parm [IN] - Client driver DLL name
    LPCWSTR szReserved,                      // @parm [IN] - Should be set to 0 to ensure compatibility
                                             //              with future versions of Windows CE
    LPCUSB_DRIVER_SETTINGS lpDriverSettings) // @parm [IN] - Must match the settings the client was registered with
{
    UnusedParameter(szReserved);
    // REVIEW this procedure has not been tested!

    BOOL fRet = FALSE;
    WCHAR szTemp[gcMaxDriverString];

    LONG lEnum;
    DWORD cchEnum;

    HKEY hkUsb;

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, gcszUsbRegKey, 0, 0, &hkUsb)
            == ERROR_SUCCESS)
    {
        HKEY hkLoadOrder;

        if(RegOpenKeyEx(hkUsb, gcszLoadClients, 0, 0, &hkLoadOrder)
                == ERROR_SUCCESS)
        {
            HKEY hkGroup1;

            GetSettingString(szTemp, gcMaxDriverString, 
                    lpDriverSettings->dwVendorId,
                    lpDriverSettings->dwProductId,
                    lpDriverSettings->dwReleaseNumber);

            if(RegOpenKeyEx(hkLoadOrder, szTemp, 0, 0, &hkGroup1)
                    == ERROR_SUCCESS)
            {
                HKEY hkGroup2;

                GetSettingString(szTemp, gcMaxDriverString, 
                        lpDriverSettings->dwDeviceClass,
                        lpDriverSettings->dwDeviceSubClass,
                        lpDriverSettings->dwDeviceProtocol);

                if(RegOpenKeyEx(hkGroup1, szTemp, 0, 0, &hkGroup2)
                        == ERROR_SUCCESS)
                {
                    HKEY hkGroup3;

                    GetSettingString(szTemp, gcMaxDriverString, 
                            lpDriverSettings->dwInterfaceClass,
                            lpDriverSettings->dwInterfaceSubClass,
                            lpDriverSettings->dwInterfaceProtocol);

                    if(RegOpenKeyEx(hkGroup2, szTemp, 0, 0, &hkGroup3)
                            == ERROR_SUCCESS)
                    {
                        if(RegDeleteKey(hkGroup3, szUniqueDriverId)
                                == ERROR_SUCCESS)
                        {
                                fRet = TRUE;
                        }
                        cchEnum = sizeof(szTemp) / sizeof(szTemp[0]);
                        lEnum = RegEnumKeyEx(hkGroup3, 0, szTemp,
                                &cchEnum, NULL, NULL, NULL, NULL);

                        RegCloseKey(hkGroup3);

                        if(lEnum == ERROR_NO_MORE_ITEMS)
                        {
                            GetSettingString(szTemp, gcMaxDriverString,
                                    lpDriverSettings->dwInterfaceClass,
                                    lpDriverSettings->dwInterfaceSubClass,
                                    lpDriverSettings->dwInterfaceProtocol);

                            RegDeleteKey(hkGroup2, szTemp);
                        }
                    }
                    cchEnum = sizeof(szTemp) / sizeof(szTemp[0]);
                    lEnum = RegEnumKeyEx(hkGroup2, 0, szTemp,
                            &cchEnum, NULL, NULL, NULL, NULL);

                    RegCloseKey(hkGroup2);

                    if(lEnum == ERROR_NO_MORE_ITEMS)
                    {
                        GetSettingString(szTemp, gcMaxDriverString,
                                lpDriverSettings->dwDeviceClass,
                                lpDriverSettings->dwDeviceSubClass,
                                lpDriverSettings->dwDeviceProtocol);

                        RegDeleteKey(hkGroup1, szTemp);
                    }
                }

                cchEnum = sizeof(szTemp) / sizeof(szTemp[0]);
                lEnum = RegEnumKeyEx(hkGroup1, 0, szTemp,
                        &cchEnum, NULL, NULL, NULL, NULL);

                RegCloseKey(hkGroup1);

                if(lEnum == ERROR_NO_MORE_ITEMS)
                {
                    GetSettingString(szTemp, gcMaxDriverString, 
                            lpDriverSettings->dwVendorId,
                            lpDriverSettings->dwProductId,
                            lpDriverSettings->dwReleaseNumber);

                    RegDeleteKey(hkLoadOrder, szTemp);
                }
            }
            RegCloseKey(hkLoadOrder);
        }
        RegCloseKey(hkUsb);
    }
    return fRet;
}



DWORD WINAPI SignalEventFunc(LPVOID lpvNotifyParameter)
{
    HANDLE * pEvent = (HANDLE *)lpvNotifyParameter;

    SetEvent(*pEvent);

    return 0;
}

/*
 * @func   LPCUSB_INTERFACE | FindInterface | Search for a specific interface on a device.
 * @rdesc  Returns pointer to USB_INTERFACE structure if interface is found,
 *         otherwise NULL.
 * @comm   Alternate settings are used to supply alternate interfaces that
 *         may be selected by client software.  Not all devices will have
 *         alternate settings, a value of 0 should be used in this case.
 * @xref   <f GetDeviceInfo>
 */
extern "C" LPCUSB_INTERFACE
FindInterface(
   LPCUSB_DEVICE lpDeviceInfo, // @parm [IN] - Pointer to device info struct (returned
                               //              from <f GetDeviceInfo>)
   UCHAR bInterfaceNumber,     // @parm [IN] - Interface number
   UCHAR bAlternateSetting)    // @parm [IN] - Alternate interface setting
{
    UINT cInterfaces =
            lpDeviceInfo->lpActiveConfig->dwNumInterfaces;

    PCUSB_INTERFACE pInterface =
            lpDeviceInfo->lpActiveConfig->lpInterfaces;
    DEBUGMSG(ZONE_API,(TEXT("+USBD:FindInterface, if:%u, Alt:%u\r\n"),
               bInterfaceNumber,bAlternateSetting));
    for(UINT iInterface = 0 ; iInterface < cInterfaces ; ++iInterface)
    {
        if(pInterface->Descriptor.bInterfaceNumber == bInterfaceNumber &&
                pInterface->Descriptor.bAlternateSetting == bAlternateSetting)
        {
            DEBUGMSG(ZONE_API,(TEXT("-USBD:FindInterface\r\n")));
            return pInterface;
        }
        ++pInterface;
    }
    DEBUGMSG(ZONE_API,(TEXT("-USBD:FindInterface, interface not found\r\n")));
    return NULL;
}

/*
 * @func   BOOL | TakeFrameLengthControl | Register for exclusive access to control the USB frame length.
 * @rdesc  Return TRUE if frame length control is given, FALSE if another driver
 *         has already obtained control of USB frame length.
 * @comm   Use with care - changing the USB frame length may affect other devices on the bus.
 * @xref   <f SetFrameLength> <f ReleaseFrameLengthControl>
 */
extern "C" BOOL
TakeFrameLengthControl(
    USB_HANDLE hDevice)  // @parm [IN] - USB device handle
{
    SDevice * pDev = (SDevice *)hDevice;
    if (!ValidateDeviceHandle(pDev))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:TakeFrameLengthControl - Invalid device handle\r\n")));
        return FALSE;
    }
    SHcd * pHcd = pDev->pHcd;
    BOOL fRet = FALSE;
    DEBUGMSG(ZONE_API,(TEXT("+USBD:TakeFrameLengthControl\r\n")));
    EnterCriticalSection(&pHcd->csFrameLengthControl);

    if(!pHcd->pFrameControlOwner)
    {
        pHcd->pFrameControlOwner = pDev;
        fRet = TRUE;
    }

    LeaveCriticalSection(&pHcd->csFrameLengthControl);
    DEBUGMSG(ZONE_API,(TEXT("-USBD:TakeFrameLengthControl\r\n")));
    return fRet;
}

/*
 * @func    BOOL | ReleaseFrameLengthControl | Release exclusive access to USB frame length control
 * @rdesc   Return TRUE if successful, FALSE if client has not previously obtained frame length control
 * @xref   <f SetFrameLength> <f TakeFrameLengthControl>
 */
extern "C" BOOL
ReleaseFrameLengthControl(
    USB_HANDLE hDevice)   // @parm [IN] - USB device handle
{
    SDevice * pDev = (SDevice *)hDevice;
    if (!ValidateDeviceHandle(pDev))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:ReleaseFrameLengthControl - Invalid device handle\r\n")));
        return FALSE;
    }
    SHcd * pHcd = pDev->pHcd;
    BOOL fRet = FALSE;
    DEBUGMSG(ZONE_API,(TEXT("+USBD:ReleaseFrameLengthControl\r\n")));

    // once we have somebody own frame length control then only he
    // can release it... and only once at that!
    if(pHcd->pFrameControlOwner == pDev)
    {
        fRet=(*pHcd->pHcdFuncs->lpStopAdjustingFrame)(pHcd->pvHcd);
        pHcd->pFrameControlOwner = NULL;
    }
    DEBUGMSG(ZONE_API,(TEXT("-USBD:ReleaseFrameLengthControl\r\n")));
    return fRet;
}

/*
 * @func    BOOL | SetFrameLength | Change USB frame length
 * @rdesc   Return TRUE if successful, FALSE if error.
 * @comm    This function should be used infrequently by client drivers, as
 *          changing the USB frame length may affect other devices on the bus.
 *          Must obtain frame control access by calling <f TakeFrameLengthControl>
 *          before attempting to change the frame length.  To enable devices to
 *          adjust to the new frame length, USB requires that frame length changes
 *          be done gradually, over a period of frames, instead of just jumping to
 *          the target frame length.  The client driver passes in an event handle,
 *          which will be signalled once the frame length has reached the target.
 * @xref  <f TakeFrameLengthControl> <f ReleaseFrameLengthControl>
 */
extern "C" BOOL
SetFrameLength(
    USB_HANDLE hDevice, // @parm [IN] - Handle to USB device
    HANDLE hEvent,      // @parm [IN] - Event to signal when frame length reaches target value
    USHORT uFrameLength)// @parm [IN] - Desired frame length
{
    SDevice * pDev = (SDevice *)hDevice;
    if (!ValidateDeviceHandle(pDev))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:SetFrameLength - Invalid device handle\r\n")));
        return FALSE;
    }
    SHcd * pHcd = pDev->pHcd;
    BOOL fRet = FALSE;
    DEBUGMSG(ZONE_API,(TEXT("+USBD:SetFrameLength\r\n")));

    // it is illegal to release and adjust at the same time, so this
    // device will have frame length control now and until we leave this
    // function
    if(pHcd->pFrameControlOwner == pDev)
    {
        fRet = (*pHcd->pHcdFuncs->lpSetFrameLength)(pHcd->pvHcd,
                hEvent, uFrameLength);
    }
    DEBUGMSG(ZONE_API,(TEXT("-USBD:SetFrameLength\r\n")));
    return fRet;
}


/*
 * @func   BOOL | GetFrameNumber | Get current USB frame number
 * @rdesc  Return TRUE if successful, FALSE if error
 */
extern "C" BOOL
GetFrameNumber(
    USB_HANDLE hDevice,       // @parm [IN] - USB device handle
    LPDWORD lpdwFrameNumber)  // @parm [OUT]- Filled in with frame number
{
    SDevice * pDev = (SDevice *)hDevice;
    if (!ValidateDeviceHandle(pDev))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:GetFrameNumber - Invalid device handle\r\n")));
        return FALSE;
    }
    SHcd * pHcd = pDev->pHcd;
    DEBUGMSG(ZONE_API,(TEXT("+USBD:GetFrameNumber\r\n")));
    return (*pHcd->pHcdFuncs->lpGetFrameNumber)(pHcd->pvHcd, lpdwFrameNumber);
}
/*
 * @func   BOOL | GetFrameLength | Get current USB frame length
 * @rdesc  Return TRUE if successful, FALSE if error.
 */
extern "C" BOOL
GetFrameLength(
    USB_HANDLE hDevice,      // @parm [IN] - USB device handle
    LPUSHORT lpuFrameLength) // @parm [OUT]- Filled in with current frame length
{
    SDevice * pDev = (SDevice *)hDevice;
    if (!ValidateDeviceHandle(pDev))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:GetFrameLength - Invalid device handle\r\n")));
        return FALSE;
    }
    SHcd * pHcd = pDev->pHcd;
    DEBUGMSG(ZONE_API,(TEXT("+USBD:GetFrameLength\r\n")));

    return (*pHcd->pHcdFuncs->lpGetFrameLength)(pHcd->pvHcd, lpuFrameLength);
}

/*
 * @func   USB_PIPE | OpenPipe | Open a pipe for communication with USB device.
 * @rdesc  Return USB pipe handle, or NULL if error.  If an error occurs, call
 *         <f GetLastError> for extended error information.  The following Win32
 *         error codes are applicable: <nl>
 *                 ERROR_INVALID_HANDLE   -- The device handle is invalid <nl>
 *                 ERROR_OUTOFMEMORY      -- Could not obtain required memory for operation <nl>
 *                 ERROR_BUSY             -- Could not allocate sufficient bandwidth for pipe <nl>
 *
 * @comm   Only 16 pipes may be open simultaneously on a device (including
 *         default endpoint 0 pipe).
 * @xref   <f ClosePipe>
 */
extern "C" USB_PIPE
OpenPipe(
    USB_HANDLE hDevice,          // @parm [IN] - USB device handle
    LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor) // @parm [IN] - Pointer to endpoint descriptor
{
    BOOL fRet;

    SDevice * pDev = (SDevice *)hDevice;
    if (!ValidateDeviceHandle(pDev))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:OpenPipe - Invalid device handle\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return NULL;
    }
    SHcd * pHcd = pDev->pHcd;
    LPHCD_OPEN_PIPE             pFunc = pHcd->pHcdFuncs->lpOpenPipe;
    SPipe * pPipe = GetPipeObject(pDev);
    if (pPipe == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return NULL;
    }
    pPipe->wMaxPktSize = lpEndpointDescriptor->wMaxPacketSize;

    DEBUGMSG(ZONE_API|ZONE_PIPE,
             (TEXT("+USBD:OpenPipe, EP: 0x%x, MaxPkt: %u, Type: %s\r\n"),
              lpEndpointDescriptor->bEndpointAddress,lpEndpointDescriptor->wMaxPacketSize,
              aszTypeStrings[lpEndpointDescriptor->bmAttributes&3]));

    fRet = (*pFunc)(pHcd->pvHcd, pDev->iDevice, lpEndpointDescriptor,
                    &pPipe->iEndpointIndex);

    if (fRet) {
        UINT iFreePipe = gcMaxPipes; // this value is deliberately out-of-bounds
        EnterCriticalSection(&pDev->csPipeLock);
        for (UINT iPipe = 0 ; iPipe < gcMaxPipes ; ++iPipe) {
            SPipe *pTmp = pDev->apPipes[iPipe];
            if (pTmp) {
                if (pTmp->iEndpointIndex == pPipe->iEndpointIndex) {
                    DEBUGMSG(ZONE_WARNING|ZONE_PIPE, (TEXT("-USBD:OpenPipe - pipe is already open\n")));
                    SetLastError(ERROR_BUSY);
                    iFreePipe = gcMaxPipes;
                    break;
                }
            } else
                if (iFreePipe == gcMaxPipes)
                    iFreePipe = iPipe;
        }
        if (iFreePipe < gcMaxPipes) {
            pDev->apPipes[iFreePipe] = pPipe;
            LeaveCriticalSection(&pDev->csPipeLock);
            DEBUGMSG(ZONE_API|ZONE_PIPE, (TEXT("-USBD:OpenPipe success, hPipe = %X\n"), pPipe));
            return (USB_PIPE)pPipe;
        }
        LeaveCriticalSection(&pDev->csPipeLock);
    }
    // Some error occurred (error code set in HCD)
    FreePipeObject(pPipe);
    DEBUGMSG(ZONE_ERROR,(TEXT("!USBD:OpenPipe returning error\r\n")));
    return NULL;
}

/*
 * @func   BOOL | ClosePipe | Close an open pipe handle.
 * @rdesc  Return TRUE if successful, FALSE if error.
 * @comm   Abort and close all transfers in progress on pipe, and free all
 *         resources associated with pipe.  If transfers are in
 *         progress, will block until aborts complete.
 */
extern "C" BOOL
ClosePipe(
    USB_PIPE hPipe)  // @parm [IN] - Open pipe handle
{
    BOOL fRet = TRUE;
    SPipe * pPipe = (SPipe *)hPipe;
    if (!ReferencePipeHandle(pPipe))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:ClosePipe - Invalid handle\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    SDevice * pDev = pPipe->pDevice;
    SHcd * pHcd = pDev->pHcd;
    DEBUGMSG(ZONE_API|ZONE_PIPE,(TEXT("+USBD:ClosePipe, hPipe: 0x%X\r\n"),hPipe));

    EnterCriticalSection(&pDev->csPipeLock);

#ifdef NEVER 

    DEBUGCHK( pDev->apPipes[pPipe->iEndpointIndex] == pPipe );
#endif

    for (UINT iPipe = 0; iPipe < gcMaxPipes; ++iPipe)
        if (pDev->apPipes[iPipe] == pPipe)
            break;
    if (iPipe == gcMaxPipes) {
        DEBUGMSG(ZONE_WARNING|ZONE_API|ZONE_PIPE, (TEXT("-USBD:ClosePipe, pipe is not open.\n")));
        LeaveCriticalSection(&pDev->csPipeLock);
        DereferencePipeHandle(pPipe);
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    pDev->apPipes[iPipe] = NULL;

    LeaveCriticalSection(&pDev->csPipeLock);

    // Mark pipe as going away - this will prevent any new transfers from
    // being started, and will prevent any transfers in progress from blocking
    // (since AddTransfer() will fail, and if AddTransfer() has already been called,
    // we'll unblock the transfer in CloseTransfer()).
    EnterCriticalSection(&pPipe->csTransferLock);
    pPipe->dwSig = CLOSING_PIPE_SIG;

    if (pPipe->apTransfers)
    {
        for (UINT iTransfer = 0 ; iTransfer < pPipe->cAllocatedTransfers ;
            ++iTransfer)
        {
            STransfer * pTransfer = pPipe->apTransfers[iTransfer];
            pPipe->apTransfers[iTransfer] = NULL;
            if (pTransfer) {
                for (DWORD dwCount=0;dwCount<1000 && (!pTransfer->fComplete) ;dwCount++) {
                    if (AbortTransfer(pTransfer, 0))
                        break;
                    else
                        Sleep(1);
                }
                ASSERT(pTransfer->fComplete);
                
                LeaveCriticalSection(&pPipe->csTransferLock);
                FreeTransferObject(pTransfer);
                EnterCriticalSection(&pPipe->csTransferLock);
            }
            
        }

        delete [] pPipe->apTransfers; // all transfers inside should be NULL
        pPipe->apTransfers = NULL;
        pPipe->cAllocatedTransfers = 0;
    }

    // Free all transfers on our free list
    while (pPipe->pFreeTransferList)
    {
        STransfer *pTransfer = pPipe->pFreeTransferList;
        pPipe->pFreeTransferList = pTransfer->pNext;
        // Release memory and resources used by transfer struct
        FreeTransferObjectMem(pTransfer);
    }
    LeaveCriticalSection(&pPipe->csTransferLock);

    UINT iEp = pPipe->iEndpointIndex;

    // Put pipe struct on free list
    DereferencePipeHandle(pPipe);
    FreePipeObject(pPipe);

    // Finally, call in to the HCD to clean up.
    fRet = (*pHcd->pHcdFuncs->lpClosePipe)(pHcd->pvHcd, pDev->iDevice, iEp);

    DEBUGMSG(ZONE_API|ZONE_PIPE,(TEXT("-USBD:ClosePipe\r\n")));
    return fRet;
}

/*
 * @func   BOOL | ResetPipe | Reset an open USB pipe
 * @rdesc  Return TRUE if successful, or FALSE if error.
 * @comm   This request will clear the halted state of a pipe within the USB stack, and
 *         reset the data toggle state of the endpoint to DATA0.  It does not reset the stalled
 *         state of the endpoint on the device; the device driver must accomplish this by
 *         sending the appropriate FEATURE request on the default pipe.  To determine whether
 *         the endpoint is stalled on the device side, use the <f GetStatus> call.
 *
 * @xref   <f IsPipeHalted> <f ClearFeature> <f GetStatus>
 */
extern "C" BOOL
ResetPipe(
    USB_PIPE hPipe) // @parm [IN] - Open USB pipe handle
{
    SPipe * pPipe = (SPipe *)hPipe;
    BOOL fRet;
    if (!ReferencePipeHandle(pPipe))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:ResetPipe - Invalid handle\r\n")));
        return FALSE;
    }
    SDevice * pDev = pPipe->pDevice;
    SHcd * pHcd = pDev->pHcd;
    LPHCD_RESET_PIPE pFunc = pHcd->pHcdFuncs->lpResetPipe;
    DEBUGMSG(ZONE_API|ZONE_PIPE,(TEXT("+USBD:ResetPipe\r\n")));

    fRet = (*pFunc)(pHcd->pvHcd, pDev->iDevice, pPipe->iEndpointIndex);
    DereferencePipeHandle(pPipe);
    return fRet;
}


/*
 * @func   BOOL | ResetDefaultPipe | Reset default (EP0) USB pipe
 * @rdesc  Return TRUE if successful, or FALSE if error.
 * @comm   This request will clear the halted state of the default pipe within the USB stack.
 *         Note: this function is only available in USBDI version 1.1 or later.
 *
 * @xref   <f IsDefaultPipeHalted>
 */
extern "C" BOOL
ResetDefaultPipe(
    USB_HANDLE hDevice) // @parm [IN] -  USB device handle
{

    SDevice *pDev = (SDevice *)hDevice;
    if (!ValidateDeviceHandle(pDev))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:ResetDefaultPipe - Invalid device handle\r\n")));
        return FALSE;
    }
    SPipe *pDfltPipe;
    EnterCriticalSection(&pDev->csPipeLock);
    pDfltPipe = pDev->apPipes[gcEndpointZero];
    LeaveCriticalSection(&pDev->csPipeLock);

    return ResetPipe(pDfltPipe);
}

/*
 * @func   BOOL | AbortPipeTransfers | Abort all transfers on an open USB pipe
 * @rdesc  Return TRUE if successful, FALSE if error.
 * @comm   Will block until all transfers have been successfully cancelled on the
 *         endpoint, unless USB_NO_WAIT is specified, in which case, the transfers
 *         are flagged to be cancelled, but the function returns immediately.
 */
extern "C" BOOL
AbortPipeTransfers(
    USB_PIPE hPipe,  // @parm [IN] - Open USB pipe handle
    DWORD dwFlags)   // @parm [IN] - USB_NO_WAIT, or 0
{
    SPipe * pPipe = (SPipe *)hPipe;
    if (!ReferencePipeHandle(pPipe))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:AbortPipeTransfers - Invalid handle\r\n")));
        return FALSE;
    }
    SDevice * pDev = pPipe->pDevice;
    SHcd * pHcd = pDev->pHcd;
    UINT iTransfer;
    STransfer * pTransfer;

    DEBUGMSG(ZONE_API|ZONE_PIPE,
             (TEXT("+USBD:AbortPipeTransfers, hPipe: 0x%X, flags: 0x%X, AllocatedTrans:%u\r\n"),
              hPipe,dwFlags,pPipe->cAllocatedTransfers));

    EnterCriticalSection(&pPipe->csTransferLock);
    for (iTransfer=0;iTransfer<pPipe->cAllocatedTransfers;iTransfer++) {
        pTransfer = pPipe->apTransfers[iTransfer];
        LeaveCriticalSection(&pPipe->csTransferLock);
        if (pTransfer)
            AbortTransfer(pTransfer, dwFlags);
        EnterCriticalSection(&pPipe->csTransferLock);
    }
    LeaveCriticalSection(&pPipe->csTransferLock);

    DEBUGMSG(ZONE_PIPE,(TEXT("-USBD:AbortPipeTransfers\r\n")));
    DereferencePipeHandle(pPipe);
    return TRUE;
}

/* @topic USB Transfer functions | USB client drivers communicate with devices by issuing
 *        transfers.  There are four basic type of transfers: Control, Bulk, Interrupt, and
 *        Isochronous.  Several routines are also provided to issue control transfers for common
 *        device configuration and setup requests.
 *
 *        All transfer routines have an optional callback parameter.  If specified, the transfer
 *        routine will return immediately, and the callback function will be called when
 *        the transfer has completed, or been cancelled.  If no callback is specified, then
 *        the function will block until the transfer is finished, unless the USB_NO_WAIT flag
 *        is specified.  In this case, the function will return immediately, and no indication
 *        will be given when the transfer completes. This might be used by a client driver to
 *        queue up a list of requests, and only wait for the last one.  Note that the client is
 *        still responsible for closing all transfer handles in this case.
 *
 *        All transfer routines return a handle to a transfer, or NULL if an error occurs.  In
 *        the latter case, client drivers may obtain further error information by calling the
 *        standard <f GetLastError> function.  The following Win32 error codes are applicable to
 *        the USB transfer functions: <nl>
 *
 *               ERROR_INVALID_HANDLE      -- The pipe handle is invalid <nl>
 *               ERROR_OUTOFMEMORY         -- Could not obtain required memory for operation <nl>
 *               ERROR_INVALID_USER_BUFFER -- Client buffer pointer is invalid  <nl>
 *               ERROR_INVALID_PARAMETER   -- (isoch only) The specified starting frame is invalid <nl>
 *
 *        The USB (OHCI) hardware assumes a host page size of 4K bytes.  Under Windows CE, many
 *        supported processors do not necessarily follow this assumption (1K page size is common).
 *        The issue here is that the underlying hardware can use scatter/gather DMA, but
 *        can handle at most 1 page transition per transfer.  However, client buffers on systems
 *        with smaller page sizes may be scattered across several physically disjoint pages.
 *
 *        In order to make this restriction transparent to client drivers, OHCD.DLL allocates
 *        a buffer from the system which is known to be contiguous within 4K sections.
 *        Then, DMA is done to the private buffers, and copied to the client buffer
 *        once the transfer is complete.  For some client drivers, this extra copy may
 *        impose an unacceptable performance hit, however.  So, the Windows CE transfer functions
 *        allow the client to pass in an optional physical memory address which is used directly
 *        for the DMA to/from the HC hardware.  Because the HC accesses this buffer directly,
 *        the following two restrictions must be rigidly adhered to:
 *
 *        1) Physical buffer must be contiguous within 4K segments.   How the client allocates
 *           this memory is beyond the scope of this document (e.g. may be through a platform
 *           specific call). The LockPages() call can be used to get physical page information.
 *
 *        2) Client must not access or free the buffer before the transfer has completed.
 *
 *
 * @xref  <f GetInterface> <f SetInterface> <f GetDescriptor> <f SetDescriptor> <f SetFeature>
 *        <f ClearFeature> <f GetStatus> <f SyncFrame> <f IssueVendorTransfer>
 *        <f IssueControlTransfer> <f IssueBulkTransfer> <f IssueInterruptTransfer>
 *        <f IssueIsochTransfer>
 */

/*
 * @func   USB_TRANSFER | GetInterface | Send a GET_INTERFACE request to USB device
 * @rdesc  Returns a USB_TRANSFER handle, or NULL if an error occurs.
 * @comm   Initiates a control transfer to USB device requesting the alternate
 *         setting selected for the given interface.
 * @xref   <f SetInterface> <f IsTransferComplete> <f GetTransferStatus> <f AbortTransfer>
 */
extern "C" USB_TRANSFER
GetInterface(
    USB_HANDLE hDevice,           // @parm [IN] - USB device handle
    LPTRANSFER_NOTIFY_ROUTINE lpStartAddress, // @parm [IN] - Address of callback routine (may be NULL)
    LPVOID lpvNotifyParameter,    // @parm [IN] - Parameter to pass to callback routine
    DWORD dwFlags,                // @parm [IN] - USB_NO_WAIT, or 0
    UCHAR bInterfaceNumber,       // @parm [IN] - Interface number
    PUCHAR lpbAlternateSetting)   // @parm [OUT]- Filled in with current alternate setting value
{
    USB_DEVICE_REQUEST DeviceRequest;
    DEBUGMSG(ZONE_API|ZONE_CONFIG,(TEXT("+USBD:GetInterface\r\n")));

    DeviceRequest.bmRequestType = USB_REQUEST_DEVICE_TO_HOST |
            USB_REQUEST_STANDARD | USB_REQUEST_FOR_INTERFACE;
    DeviceRequest.bRequest = USB_REQUEST_GET_INTERFACE;
    DeviceRequest.wValue = 0;
    DeviceRequest.wIndex = bInterfaceNumber;
    DeviceRequest.wLength = 1;

    dwFlags |= USB_IN_TRANSFER;

    return IssueVendorTransfer(hDevice, lpStartAddress, lpvNotifyParameter,
            dwFlags, &DeviceRequest, lpbAlternateSetting, 0);
}

/*
 * @func   USB_TRANSFER | SetInterface | Send a SET_INTERFACE request to USB device
 * @rdesc  Returns a USB_TRANSFER handle, or NULL if an error occurs.
 * @comm   Initiates a control transfer to USB device specifying the alternate
 *         setting to use for the given interface.
 * @xref   <f GetInterface> <f IsTransferComplete> <f GetTransferStatus> <f AbortTransfer>
 */
extern "C" USB_TRANSFER
SetInterface(
    USB_HANDLE hDevice,           // @parm [IN] - USB device handle
    LPTRANSFER_NOTIFY_ROUTINE lpStartAddress, // @parm [IN] - Address of callback routine (should be NULL unless the USB_NO_WAIT flag is set)
    LPVOID lpvNotifyParameter,    // @parm [IN] - Parameter to pass to callback routine
    DWORD dwFlags,                // @parm [IN] - USB_NO_WAIT, or 0
    UCHAR bInterfaceNumber,       // @parm [IN] - Interface number
    UCHAR bAlternateSetting)      // @parm [IN] - Alternate setting number
{
    USB_DEVICE_REQUEST DeviceRequest;
    DEBUGMSG(ZONE_API|ZONE_CONFIG,(TEXT("+USBD:SetInterface\r\n")));

    DeviceRequest.bmRequestType = USB_REQUEST_HOST_TO_DEVICE |
            USB_REQUEST_STANDARD | USB_REQUEST_FOR_INTERFACE;
    DeviceRequest.bRequest = USB_REQUEST_SET_INTERFACE;
    DeviceRequest.wValue = bAlternateSetting;
    DeviceRequest.wIndex = bInterfaceNumber;
    DeviceRequest.wLength = 0;

    dwFlags &= ~USB_IN_TRANSFER;
    dwFlags &= ~USB_SHORT_TRANSFER_OK;

    return IssueVendorTransfer(hDevice, lpStartAddress, lpvNotifyParameter,
            dwFlags, &DeviceRequest, NULL, 0);
}
/*
 * @func   USB_TRANSFER | GetDescriptor | Send a GET_DESCRIPTOR request to USB device.
 * @rdesc  Returns a USB_TRANSFER handle, or NULL if an error occurs.
 * @comm   Initiates a control transfer to USB device requesting device
 *         descriptor information.  The device will return the contents of the
 *         device descriptor starting at the index specified.
 * @xref   <f SetDescriptor> <f IsTransferComplete> <f GetTransferStatus> <f AbortTransfer>
 */
extern "C" USB_TRANSFER
GetDescriptor(
    USB_HANDLE hDevice,           // @parm [IN] - USB device handle
    LPTRANSFER_NOTIFY_ROUTINE lpStartAddress, // @parm [IN] - Address of callback routine (may be NULL)
    LPVOID lpvNotifyParameter,    // @parm [IN] - Parameter to pass to callback routine
    DWORD dwFlags,                // @parm [IN] - USB_NO_WAIT, or 0
    UCHAR bType,                  // @parm [IN] - Descriptor type (one of
                                  //              USB_DEVICE_DESCRIPTOR_TYPE,
                                  //              USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                  //              USB_STRING_DESCRIPTOR_TYPE, or vendor
                                  //              specific descriptor type)
    UCHAR bIndex,                 // @parm [IN] - Index within descriptor
    WORD wLanguage,               // @parm [IN] - Language Id for string descriptors, 0 for others
    WORD wLength,                 // @parm [IN] - Size of buffer
    LPVOID lpvBuffer)             // @parm [IN] - Buffer for descriptor data
{
    USB_DEVICE_REQUEST DeviceRequest;
    DEBUGMSG(ZONE_API|ZONE_CONFIG,(TEXT("+USBD:GetDescriptor\r\n")));

    DeviceRequest.bmRequestType = USB_REQUEST_DEVICE_TO_HOST |
            USB_REQUEST_STANDARD | USB_REQUEST_FOR_DEVICE;
    DeviceRequest.bRequest = USB_REQUEST_GET_DESCRIPTOR;
    DeviceRequest.wValue = ((WORD)bType) << 8 | bIndex;
    DeviceRequest.wIndex = wLanguage;
    DeviceRequest.wLength = wLength;

    dwFlags |= USB_IN_TRANSFER;

    return IssueVendorTransfer(hDevice, lpStartAddress, lpvNotifyParameter,
            dwFlags, &DeviceRequest, lpvBuffer, 0);
}
/*
 * @func   USB_TRANSFER | SetDescriptor | Send a SET_DESCRIPTOR request to USB device.
 * @rdesc  Returns a USB_TRANSFER handle, or NULL if an error occurs.
 * @comm   Initiates a control transfer to USB device setting device
 *         descriptor information.
 * @xref   <f GetDescriptor> <f IsTransferComplete> <f GetTransferStatus> <f AbortTransfer>
 */
extern "C" USB_TRANSFER
SetDescriptor(
    USB_HANDLE hDevice,           // @parm [IN] - USB device handle
    LPTRANSFER_NOTIFY_ROUTINE lpStartAddress, // @parm [IN] - Address of callback routine (may be NULL)
    LPVOID lpvNotifyParameter,    // @parm [IN] - Parameter to pass to callback routine
    DWORD dwFlags,                // @parm [IN] - USB_NO_WAIT, or 0
    UCHAR bType,                  // @parm [IN] - Descriptor type (one of
                                  //              USB_DEVICE_DESCRIPTOR_TYPE,
                                  //              USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                  //              or USB_STRING_DESCRIPTOR_TYPE, or vendor
                                  //              specific descriptor type)
    UCHAR bIndex,                 // @parm [IN] - Index within descriptor
    WORD wLanguage,               // @parm [IN] - Language Id for string descriptors, 0 for others
    WORD wLength,                 // @parm [IN] - Descriptor length
    LPVOID lpvBuffer)             // @parm [IN] - Buffer for descriptor data
{
    USB_DEVICE_REQUEST DeviceRequest;
    DEBUGMSG(ZONE_API|ZONE_CONFIG,(TEXT("+USBD:SetDescriptor\r\n")));

    DeviceRequest.bmRequestType = USB_REQUEST_HOST_TO_DEVICE |
            USB_REQUEST_STANDARD | USB_REQUEST_FOR_DEVICE;
    DeviceRequest.bRequest = USB_REQUEST_SET_DESCRIPTOR;
    DeviceRequest.wValue = ((WORD)bType) << 8 | bIndex;
    DeviceRequest.wIndex = wLanguage;
    DeviceRequest.wLength = wLength;

    dwFlags &= ~USB_IN_TRANSFER;
    dwFlags &= ~USB_SHORT_TRANSFER_OK;

    return IssueVendorTransfer(hDevice, lpStartAddress, lpvNotifyParameter,
            dwFlags, &DeviceRequest, lpvBuffer, 0);
}

/*
 * @func   USB_TRANSFER | SetFeature | Send a SET_FEATURE request to USB device.
 * @rdesc  Returns a USB_TRANSFER handle, or NULL if an error occurs.
 * @comm   Initiates a control transfer to USB device requesting specified
 *         feature be enabled.
 * @xref   <f ClearFeature> <f IsTransferComplete> <f GetTransferStatus> <f AbortTransfer>
 */
extern "C" USB_TRANSFER
SetFeature(
    USB_HANDLE hDevice,           // @parm [IN] - USB device handle
    LPTRANSFER_NOTIFY_ROUTINE lpStartAddress, // @parm [IN] - Address of callback routine (may be NULL)
    LPVOID lpvNotifyParameter,    // @parm [IN] - Parameter to pass to callback routine
    DWORD dwFlags,                // @parm [IN] - USB_NO_WAIT or 0, plus one of the following:
                                  //              USB_SEND_TO_DEVICE:    Request for device
                                  //              USB_SEND_TO_INTERFACE: Request for interface
                                  //              USB_SEND_TO_ENDPOINT:  Request for endpoint (default)
    WORD wFeature,                // @parm [IN] - Feature selector - one of USB_FEATURE_xxx defs in usb100.h
    UCHAR bIndex)                 // @parm [IN] - 0 for DEVICE, or interface/endpoint number
{
    USB_DEVICE_REQUEST DeviceRequest;
    DEBUGMSG(ZONE_API|ZONE_CONFIG,(TEXT("+USBD:SetFeature\r\n")));

    DeviceRequest.bmRequestType = USB_REQUEST_HOST_TO_DEVICE |
            USB_REQUEST_STANDARD;
    if(dwFlags & USB_SEND_TO_DEVICE)
        DeviceRequest.bmRequestType |= USB_REQUEST_FOR_DEVICE;
    else if(dwFlags & USB_SEND_TO_INTERFACE)
        DeviceRequest.bmRequestType |= USB_REQUEST_FOR_INTERFACE;
    else
        DeviceRequest.bmRequestType |= USB_REQUEST_FOR_ENDPOINT;

    DeviceRequest.bRequest = USB_REQUEST_SET_FEATURE;
    DeviceRequest.wValue = wFeature;
    DeviceRequest.wIndex = dwFlags & USB_SEND_TO_DEVICE ? 0 : bIndex;
    DeviceRequest.wLength = 0;

    dwFlags &= ~USB_IN_TRANSFER;
    dwFlags &= ~USB_SHORT_TRANSFER_OK;

    return IssueVendorTransfer(hDevice, lpStartAddress, lpvNotifyParameter,
            dwFlags, &DeviceRequest, NULL, 0);
}

/*
 * @func   USB_TRANSFER | ClearFeature | Send a CLEAR_FEATURE request to USB device
 * @rdesc  Returns a USB_TRANSFER handle, or NULL if an error occurs.
 * @comm   Initiates a control transfer to USB device requesting specified
 *         feature be disabled.
 * @xref   <f SetFeature> <f IsTransferComplete> <f GetTransferStatus> <f AbortTransfer>
 */
extern "C" USB_TRANSFER
ClearFeature(
    USB_HANDLE hDevice,           // @parm [IN] - USB device handle
    LPTRANSFER_NOTIFY_ROUTINE lpStartAddress, // @parm [IN] - Address of callback routine (may be NULL)
    LPVOID lpvNotifyParameter,    // @parm [IN] - Parameter to pass to callback routine
    DWORD dwFlags,                // @parm [IN] - USB_NO_WAIT or 0, plus one of the following:
                                  //              USB_SEND_TO_DEVICE:    Request for device
                                  //              USB_SEND_TO_INTERFACE: Request for interface
                                  //              USB_SEND_TO_ENDPOINT:  Request for endpoint (default)
    WORD wFeature,                // @parm [IN] - Feature selector - one of USB_FEATURE_xxx defs in usb100.h
    UCHAR bIndex)                 // @parm [IN] - 0 for DEVICE, or interface/endpoint number
{
    USB_DEVICE_REQUEST DeviceRequest;
    DEBUGMSG(ZONE_API|ZONE_CONFIG,(TEXT("+USBD:ClearFeature\r\n")));

    DeviceRequest.bmRequestType = USB_REQUEST_HOST_TO_DEVICE |
            USB_REQUEST_STANDARD;
    if(dwFlags & USB_SEND_TO_DEVICE)
        DeviceRequest.bmRequestType |= USB_REQUEST_FOR_DEVICE;
    else if(dwFlags & USB_SEND_TO_INTERFACE)
        DeviceRequest.bmRequestType |= USB_REQUEST_FOR_INTERFACE;
    else
        DeviceRequest.bmRequestType |= USB_REQUEST_FOR_ENDPOINT;

    DeviceRequest.bRequest = USB_REQUEST_CLEAR_FEATURE;
    DeviceRequest.wValue = wFeature;
    DeviceRequest.wIndex = dwFlags & USB_SEND_TO_DEVICE ? 0 : bIndex;
    DeviceRequest.wLength = 0;

    dwFlags &= ~USB_IN_TRANSFER;
    dwFlags &= ~USB_SHORT_TRANSFER_OK;

    return IssueVendorTransfer(hDevice, lpStartAddress, lpvNotifyParameter,
            dwFlags, &DeviceRequest, NULL, 0);
}

/*
 * @func   USB_TRANSFER | GetStatus | Send a GET_STATUS request to USB device.
 * @rdesc  Returns a USB_TRANSFER handle, or NULL if an error occurs.
 * @comm   Initiates a control transfer to USB device requesting device,
 *         interface, or endpoint status.
 * @xref   <f GetStatus> <f IsTransferComplete> <f GetTransferStatus> <f AbortTransfer>
 */
extern "C" USB_TRANSFER
GetStatus(
    USB_HANDLE hDevice,           // @parm [IN] - USB device handle
    LPTRANSFER_NOTIFY_ROUTINE lpStartAddress, // @parm [IN] - Address of callback routine (may be NULL)
    LPVOID lpvNotifyParameter,    // @parm [IN] - Parameter to pass to callback routine
    DWORD dwFlags,                // @parm [IN] - USB_NO_WAIT or 0, plus one of the following:
                                  //              USB_SEND_TO_DEVICE:    Request for device
                                  //              USB_SEND_TO_INTERFACE: Request for interface
                                  //              USB_SEND_TO_ENDPOINT:  Request for endpoint (default)
    UCHAR bIndex,                 // @parm [IN] - 0 for DEVICE, or interface/endpoint number
    LPWORD lpwStatus)             // @parm [OUT]- Filled in with status word
{
    USB_DEVICE_REQUEST DeviceRequest;
    DEBUGMSG(ZONE_API|ZONE_CONFIG,(TEXT("+USBD:GetStatus\r\n")));

    DeviceRequest.bmRequestType = USB_REQUEST_DEVICE_TO_HOST |
            USB_REQUEST_STANDARD;
    if(dwFlags & USB_SEND_TO_DEVICE)
        DeviceRequest.bmRequestType |= USB_REQUEST_FOR_DEVICE;
    else if(dwFlags & USB_SEND_TO_INTERFACE)
        DeviceRequest.bmRequestType |= USB_REQUEST_FOR_INTERFACE;
    else
        DeviceRequest.bmRequestType |= USB_REQUEST_FOR_ENDPOINT;

    DeviceRequest.bRequest = USB_REQUEST_GET_STATUS;
    DeviceRequest.wValue = 0;
    DeviceRequest.wIndex = dwFlags & USB_SEND_TO_DEVICE ? 0 : bIndex;
    DeviceRequest.wLength = 2;

    dwFlags |= USB_IN_TRANSFER;

    return IssueVendorTransfer(hDevice, lpStartAddress, lpvNotifyParameter,
            dwFlags, &DeviceRequest, lpwStatus, 0);
}
/*
 * @func   USB_TRANSFER | SyncFrame | Send a SYNC_FRAME request to USB device.
 * @rdesc  Returns a USB_TRANSFER handle, or NULL if an error occurs.
 * @comm   Initiates a control transfer to USB device requesting a Sync
 *         Frame be sent (used by isoch endpoints to synchronize a data
 *         stream).
 * @xref   <f IsTransferComplete> <f GetTransferStatus> <f AbortTransfer>
 */
extern "C" USB_TRANSFER
SyncFrame(
    USB_HANDLE hDevice,           // @parm [IN] - USB device handle
    LPTRANSFER_NOTIFY_ROUTINE lpStartAddress, // @parm [IN] - Address of callback routine (may be NULL)
    LPVOID lpvNotifyParameter,    // @parm [IN] - Parameter to pass to callback routine
    DWORD dwFlags,                // @parm [IN] - USB_NO_WAIT, or 0
    UCHAR bEndpoint,              // @parm [IN] - Endpoint number
    LPWORD lpwFrame)              // @parm [OUT]- Frame number
{
    USB_DEVICE_REQUEST DeviceRequest;
    DEBUGMSG(ZONE_API|ZONE_CONFIG,(TEXT("+USBD:SyncFrame\r\n")));

    DeviceRequest.bmRequestType = USB_REQUEST_HOST_TO_DEVICE |
            USB_REQUEST_STANDARD | USB_REQUEST_FOR_ENDPOINT;
    DeviceRequest.bRequest = USB_REQUEST_SYNC_FRAME;
    DeviceRequest.wValue = 0;
    DeviceRequest.wIndex = bEndpoint;
    DeviceRequest.wLength = 2;

    dwFlags |= USB_IN_TRANSFER;

    return IssueVendorTransfer(hDevice, lpStartAddress, lpvNotifyParameter,
            dwFlags, &DeviceRequest, lpwFrame, 0);
}

/*
 * @func   USB_TRANSFER | IssueVendorTransfer | Send a vendor specific control transfer to USB device.
 * @rdesc  Returns a USB_TRANSFER handle, or NULL if an error occurs.
 * @comm   Initiates a control transfer to USB device on the default
 *         endpoint (0). Intended for vendor specific transfers (for standard
 *         transfers, use corresponding function (e.g. <f GetInterface>,
 *         <f SetInterface>, etc).
 * @xref   <f IsTransferComplete> <f GetTransferStatus> <f AbortTransfer>
 */
extern "C" USB_TRANSFER
IssueVendorTransfer(
    USB_HANDLE hDevice,           // @parm [IN] - USB device handle
    LPTRANSFER_NOTIFY_ROUTINE lpStartAddress, // @parm [IN] - Address of callback routine (may be NULL)
    LPVOID lpvNotifyParameter,    // @parm [IN] - Parameter to pass to callback routine
    DWORD dwFlags,                // @parm [IN] - Transfer flags (see usbtypes.h)
    LPCUSB_DEVICE_REQUEST lpControlHeader, // @parm [IN] - Pointer to device request header
    LPVOID lpvBuffer,             // @parm [IN] - Data buffer (if physical buffer address is
                                  //              specified, this must contain the virtual
                                  //              address of the buffer)
    ULONG uBufferPhysicalAddress) // @parm [IN] - Physical address of data buffer (may be NULL)
{
    SDevice * pDev = (SDevice *)hDevice;
    if (lpControlHeader == NULL) { 
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:IssueVendorTransfer - Invalid Parameter\r\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }
    
    if (!ValidateDeviceHandle(pDev))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:IssueVendorTransfer - Invalid device handle\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return NULL;
    }
    DEBUGMSG(ZONE_API|ZONE_VENDOR,(TEXT("+USBD:IssueVendorTransfer, dwFlags:0x%X, Control:0x%X, vBuf:0x%X, pBuf:0x%X\r\n"),
                                   dwFlags,lpControlHeader,lpvBuffer,uBufferPhysicalAddress));

    EnterCriticalSection(&pDev->csPipeLock);
    SPipe * pPipe = pDev->apPipes[gcEndpointZero];
    LeaveCriticalSection(&pDev->csPipeLock);
    if (!ReferencePipeHandle(pPipe))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:IssueVendorTransfer - Invalid EP0 handle\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return NULL;
    }

    SHcd * pHcd = pDev->pHcd;
    LPHCD_ISSUE_TRANSFER pFunc = pHcd->pHcdFuncs->lpIssueTransfer;
    BOOL fWait = FALSE;
    SWait * pWait;
    BOOL fRet;

    if(!lpStartAddress)
        fWait = dwFlags & USB_NO_WAIT ? FALSE : TRUE;

    STransfer * pTransfer = GetTransferObject(pPipe,0);
    if (pTransfer == NULL)
    {
        DereferencePipeHandle(pPipe);
        SetLastError(ERROR_OUTOFMEMORY);
        return NULL;
    }
    // Initialize the Pointer.
    pTransfer->dwFlags = dwFlags;
    pTransfer->lpOrigControlHeader = lpControlHeader;
    if (!SUCCEEDED(CeAllocAsynchronousBuffer((PVOID *)&pTransfer->lpMappedControlHeader,(PVOID)lpControlHeader,sizeof(USB_DEVICE_REQUEST),ARG_I_PTR|MARSHAL_FORCE_ALIAS))) {
        pTransfer->lpMappedControlHeader = NULL;
        DEBUGMSG (ZONE_ERROR,(TEXT("IssueVendorTransfer CeAllocAsynchronousBuffer Error (%d)\r\n"),GetLastError()));
        ASSERT(FALSE);
    }
    pTransfer->dwUserBufferSize = lpControlHeader->wLength ;
    pTransfer->lpvOrigUserBuffer = lpvBuffer;
    pTransfer->pvBuffer = NULL;
    if (lpvBuffer!=NULL && pTransfer->dwUserBufferSize!=0) {
        if (!SUCCEEDED(CeAllocAsynchronousBuffer((PVOID *)&pTransfer->pvBuffer,(PVOID)lpvBuffer,pTransfer->dwUserBufferSize,(((dwFlags& USB_IN_TRANSFER)!=0)?ARG_O_PTR:ARG_I_PTR)|MARSHAL_FORCE_ALIAS))) {
            pTransfer->pvBuffer = NULL;
            DEBUGMSG (ZONE_ERROR,(TEXT("IssueVendorTransfer CeAllocAsynchronousBuffer Error (%d)\r\n"),GetLastError()));
            ASSERT(FALSE);
        }
    }
    
    pTransfer->lpdwMappedLengths = NULL;

    if(fWait)
    {
        pWait = GetWaitObject();
        lpStartAddress = &SignalEventFunc;
        lpvNotifyParameter = &pWait->hEvent;
    }

    fRet = (*pFunc)(pHcd->pvHcd, pDev->iDevice, pPipe->iEndpointIndex,
            lpStartAddress, lpvNotifyParameter, dwFlags, pTransfer->lpMappedControlHeader, 0, 0,
            NULL, lpControlHeader->wLength, pTransfer->pvBuffer, uBufferPhysicalAddress,
            pTransfer, NULL, NULL, &pTransfer->fComplete,
            &pTransfer->dwBytesTransfered, &pTransfer->dwError);

    if (fRet && AddTransfer(pPipe,pTransfer))
    {
        if (fWait)
		{
            if ( WAIT_OBJECT_0 != WaitForSingleObject(pWait->hEvent, WAIT_DURATION_USB_TIMEOUT))
			{
				FreeTransferObject(pTransfer);
				pTransfer = NULL;
			}
		}
    }
    else
    {
        FreeTransferObject(pTransfer);
        pTransfer = NULL;
    }
    if(fWait)
        FreeWaitObject(pWait);

    DereferencePipeHandle(pPipe);

    DEBUGMSG(ZONE_API|ZONE_VENDOR,(TEXT("-USBD:IssueVendorTransfer, pTransfer:0x%X\r\n"),pTransfer));
    return (USB_TRANSFER)pTransfer;
}

/*
 * @func   USB_TRANSFER | IssueControlTransfer | Initiate control transfer with USB device.
 * @rdesc  Returns a USB_TRANSFER handle, or NULL if an error occurs.
 * @comm   Initiates a control transfer to USB device on the specified
 *         endpoint.  Note that control transfers to the default endpoint (0) are issued
 *         using <f IssueVendorTransfer>.
 * @xref   <f IsTransferComplete> <f GetTransferStatus> <f AbortTransfer>
 */
extern "C" USB_TRANSFER
IssueControlTransfer(
    USB_PIPE hPipe,               // @parm [IN] - Open USB pipe handle
    LPTRANSFER_NOTIFY_ROUTINE lpStartAddress, // @parm [IN] - Address of callback routine (may be NULL)
    LPVOID lpvNotifyParameter,    // @parm [IN] - Parameter to pass to callback routine
    DWORD dwFlags,                // @parm [IN] - Transfer flags (see usbtypes.h)
    LPCVOID lpvControlHeader,     // @parm [IN] - Pointer to control header (8 bytes)
    DWORD dwBufferSize,           // @parm [IN] - Size of data buffer
    LPVOID lpvBuffer,             // @parm [IN] - Data buffer (if physical buffer address is
                                  //              specified, this must contain the virtual
                                  //              address of the buffer)
    ULONG uBufferPhysicalAddress) // @parm [IN] - Physical address of data buffer (may be NULL)
{
    SPipe * pPipe = (SPipe *)hPipe;
    if (!ReferencePipeHandle(pPipe))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:IssueControlTransfer - Invalid handle\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return NULL;
    }
    SDevice * pDev = pPipe->pDevice;
    SHcd * pHcd = pDev->pHcd;
    LPHCD_ISSUE_TRANSFER pFunc = pHcd->pHcdFuncs->lpIssueTransfer;
    BOOL fWait = FALSE;
    SWait * pWait;
    BOOL fRet;
    DEBUGMSG(ZONE_API|ZONE_CONTROL,
             (TEXT("+USBD:IssueControlTransfer, Pipe:0x%X, dwFlags:0x%X, Control:0x%X, Bufsize: %u vBuf:0x%X, pBuf:0x%X\r\n"),
              pPipe,dwFlags,lpvControlHeader,dwBufferSize,lpvBuffer,uBufferPhysicalAddress));

    if(!lpStartAddress)
        fWait = dwFlags & USB_NO_WAIT ? FALSE : TRUE;

    STransfer * pTransfer = GetTransferObject(pPipe,0);
    if (pTransfer==NULL)
    {
        DereferencePipeHandle(pPipe);
        SetLastError(ERROR_OUTOFMEMORY);
        return NULL;
    }
    // Initialize the Pointer.
    pTransfer->dwFlags = dwFlags;
    pTransfer->lpOrigControlHeader = (LPCUSB_DEVICE_REQUEST)lpvControlHeader;
    if (!SUCCEEDED(CeAllocAsynchronousBuffer((PVOID *)&pTransfer->lpMappedControlHeader,(PVOID)lpvControlHeader,sizeof(USB_DEVICE_REQUEST), ARG_I_PTR|MARSHAL_FORCE_ALIAS))) {
        pTransfer->lpMappedControlHeader = NULL;
        DEBUGMSG (ZONE_ERROR,(TEXT("IssueControlTransfer CeAllocAsynchronousBuffer Error (%d)\r\n"),GetLastError()));
        ASSERT(FALSE);
    }
    pTransfer->dwUserBufferSize = dwBufferSize ;
    pTransfer->lpvOrigUserBuffer = lpvBuffer;
    pTransfer->pvBuffer = NULL;
    if (lpvBuffer!=NULL && pTransfer->dwUserBufferSize!=0) {
        if (!SUCCEEDED(CeAllocAsynchronousBuffer((PVOID *)&pTransfer->pvBuffer,(PVOID)lpvBuffer,pTransfer->dwUserBufferSize,(((dwFlags& USB_IN_TRANSFER)!=0)?ARG_O_PTR:ARG_I_PTR)|MARSHAL_FORCE_ALIAS))) {
            pTransfer->pvBuffer = NULL;
            DEBUGMSG (ZONE_ERROR,(TEXT("IssueControlTransfer CeAllocAsynchronousBuffer Error (%d)\r\n"),GetLastError()));
            ASSERT(FALSE);
        }
    }
    
    pTransfer->lpdwMappedLengths = NULL;

    if(fWait)
    {
        pWait = GetWaitObject();
        lpStartAddress = &SignalEventFunc;
        lpvNotifyParameter = &pWait->hEvent;
    }

    fRet = (*pFunc)(pHcd->pvHcd, pDev->iDevice, pPipe->iEndpointIndex,
            lpStartAddress, lpvNotifyParameter, dwFlags, pTransfer->lpMappedControlHeader, 0, 0,
            NULL, dwBufferSize, pTransfer->pvBuffer, uBufferPhysicalAddress,
            pTransfer, NULL, NULL, &pTransfer->fComplete,
            &pTransfer->dwBytesTransfered, &pTransfer->dwError);

    if (fRet && AddTransfer(pPipe,pTransfer))
    {
        if (fWait)
		{
            if ( WAIT_OBJECT_0 != WaitForSingleObject(pWait->hEvent, WAIT_DURATION_USB_TIMEOUT))
			{
				FreeTransferObject(pTransfer);
				pTransfer = NULL;
			}
		}
    }
    else
    {
        FreeTransferObject(pTransfer);
        pTransfer = NULL;
    }
    if(fWait)
        FreeWaitObject(pWait);

    DereferencePipeHandle(pPipe);

    DEBUGMSG(ZONE_API|ZONE_CONTROL,(TEXT("-USBD:IssueControlTransfer, pTransfer:0x%X\r\n"),pTransfer));
    return (USB_TRANSFER)pTransfer;
}

/*
 * @func   USB_TRANSFER | IssueBulkTransfer | Initiate bulk transfer with USB device.
 * @rdesc  Returns a USB_TRANSFER handle, or NULL if an error occurs (Call <f GetLastError>
 *         for extended error information).
 * @comm   Initiates a bulk transfer to USB device on the specified
 *         endpoint.
 * @xref   <f IsTransferComplete> <f GetTransferStatus> <f AbortTransfer>
 */
extern "C" USB_TRANSFER
IssueBulkTransfer(
    USB_PIPE hPipe,               // @parm [IN] - Open USB pipe handle
    LPTRANSFER_NOTIFY_ROUTINE lpStartAddress, // @parm [IN] - Address of callback routine (may be NULL)
    LPVOID lpvNotifyParameter,    // @parm [IN] - Parameter to pass to callback routine
    DWORD dwFlags,                // @parm [IN] - Transfer flags (see usbtypes.h)
    DWORD dwBufferSize,           // @parm [IN] - Size of data buffer
    LPVOID lpvBuffer,             // @parm [IN/OUT]  - Data buffer (if physical buffer address is
                                  //                   specified, this must contain the virtual
                                  //                   address of the buffer)
    ULONG uBufferPhysicalAddress) // @parm [IN/OUT] - Physical address of data buffer (may be NULL)
{
    SPipe * pPipe = (SPipe *)hPipe;
    if (!ReferencePipeHandle(pPipe))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:IssueBulkTransfer - Invalid handle\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return NULL;
    }
    SDevice * pDev = pPipe->pDevice;
    SHcd * pHcd = pDev->pHcd;
    LPHCD_ISSUE_TRANSFER pFunc = pHcd->pHcdFuncs->lpIssueTransfer;
    BOOL fWait = FALSE;
    SWait * pWait;
    BOOL fRet;
    DEBUGMSG(ZONE_API|ZONE_BULK,
             (TEXT("+USBD:IssueBulkTransfer, pipe:%X, dwFlags:0x%X, Bufsize: %u vBuf:0x%X, pBuf:0x%X\r\n"),
              pPipe,dwFlags,dwBufferSize,lpvBuffer,uBufferPhysicalAddress));

    if(!lpStartAddress)
        fWait = dwFlags & USB_NO_WAIT ? FALSE : TRUE;

    STransfer * pTransfer = GetTransferObject(pPipe,0);
    if (pTransfer == NULL)
    {
        DereferencePipeHandle(pPipe);
        SetLastError(ERROR_OUTOFMEMORY);
        return NULL;
    }
    pTransfer->dwFlags = dwFlags;
    pTransfer->lpMappedControlHeader =NULL;
    pTransfer->dwUserBufferSize = dwBufferSize ;
    pTransfer->lpvOrigUserBuffer = lpvBuffer;
    pTransfer->pvBuffer = NULL;
    if (lpvBuffer!=NULL && pTransfer->dwUserBufferSize!=0) {
        if (!SUCCEEDED(CeAllocAsynchronousBuffer((PVOID *)&pTransfer->pvBuffer,(PVOID)lpvBuffer,pTransfer->dwUserBufferSize,(((dwFlags& USB_IN_TRANSFER)!=0)?ARG_O_PTR:ARG_I_PTR)|MARSHAL_FORCE_ALIAS))) {
            pTransfer->pvBuffer = NULL;
            DEBUGMSG (ZONE_ERROR,(TEXT("IssueBulkTransfer CeAllocAsynchronousBuffer Error (%d)\r\n"),GetLastError()));
            ASSERT(FALSE);
        }
    }
    
    pTransfer->lpdwMappedLengths = NULL;

    if(fWait)
    {
        pWait = GetWaitObject();
        lpStartAddress = &SignalEventFunc;
        lpvNotifyParameter = &pWait->hEvent;
    }

    fRet = (*pFunc)(pHcd->pvHcd, pDev->iDevice, pPipe->iEndpointIndex,
            lpStartAddress, lpvNotifyParameter, dwFlags, NULL, 0, 0,
            NULL, dwBufferSize, pTransfer->pvBuffer, uBufferPhysicalAddress, pTransfer,
            NULL, NULL, &pTransfer->fComplete, &pTransfer->dwBytesTransfered,
            &pTransfer->dwError);

    if (fRet && AddTransfer(pPipe,pTransfer))
    {
        if (fWait)
		{
            if ( WAIT_OBJECT_0 != WaitForSingleObject(pWait->hEvent, WAIT_DURATION_USB_TIMEOUT))
			{
				FreeTransferObject(pTransfer);
				pTransfer = NULL;
			}
		}
    }
    else
    {
        FreeTransferObject(pTransfer);
        pTransfer = NULL;
    }
    if(fWait)
        FreeWaitObject(pWait);

    DereferencePipeHandle(pPipe);

    DEBUGMSG(ZONE_API|ZONE_BULK,(TEXT("-USBD:IssueBulkTransfer, pTransfer:0x%X\r\n"),pTransfer));
    return (USB_TRANSFER)pTransfer;
}

/*
 * @func   USB_TRANSFER | IssueInterruptTransfer | Initiate interrupt transfer with USB device.
 * @rdesc  Returns a USB_TRANSFER handle, or NULL if an error occurs.
 * @comm   Initiates an interrupt transfer to USB device on the specified
 *         endpoint.
 * @xref   <f IsTransferComplete> <f GetTransferStatus> <f AbortTransfer>
 */
extern "C" USB_TRANSFER
IssueInterruptTransfer(
    USB_PIPE hPipe,               // @parm [IN] - Open USB pipe handle
    LPTRANSFER_NOTIFY_ROUTINE lpStartAddress, // @parm [IN] - Address of callback routine (may be NULL)
    LPVOID lpvNotifyParameter,    // @parm [IN] - Parameter to pass to callback routine
    DWORD dwFlags,                // @parm [IN] - Transfer flags (see usbtypes.h)
    DWORD dwBufferSize,           // @parm [IN] - Size of data buffer
    LPVOID lpvBuffer,             // @parm [IN] - Data buffer (if physical buffer address is
                                  //              specified, this must contain the virtual
                                  //              address of the buffer)
    ULONG uBufferPhysicalAddress) // @parm [IN] - Physical address of data buffer (may be NULL)
{
    SPipe * pPipe = (SPipe *)hPipe;
    if (!ReferencePipeHandle(pPipe))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:IssueInterruptTransfer - Invalid handle\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return NULL;
    }
    SDevice * pDev = pPipe->pDevice;
    SHcd * pHcd = pDev->pHcd;
    LPHCD_ISSUE_TRANSFER pFunc = pHcd->pHcdFuncs->lpIssueTransfer;
    BOOL fWait = FALSE;
    SWait * pWait;
    BOOL fRet;
    DEBUGMSG(ZONE_API|ZONE_INTERRUPT,
             (TEXT("+USBD:IssueInterruptTransfer, pipe:%X, dwFlags:0x%X, Bufsize: %u vBuf:0x%X, pBuf:0x%X\r\n"),
              pPipe,dwFlags,dwBufferSize,lpvBuffer,uBufferPhysicalAddress));

    if(!lpStartAddress)
        fWait = dwFlags & USB_NO_WAIT ? FALSE : TRUE;

    STransfer * pTransfer = GetTransferObject(pPipe,0);
    if (pTransfer == NULL)
    {
        DereferencePipeHandle(pPipe);
        SetLastError(ERROR_OUTOFMEMORY);
        return NULL;
    }
    pTransfer->dwFlags = dwFlags;
    pTransfer->lpMappedControlHeader =NULL;
    pTransfer->dwUserBufferSize = dwBufferSize ;
    pTransfer->lpvOrigUserBuffer = lpvBuffer;
    pTransfer->pvBuffer = NULL;
    if (lpvBuffer!=NULL && pTransfer->dwUserBufferSize!=0) {
        if (!SUCCEEDED(CeAllocAsynchronousBuffer((PVOID *)&pTransfer->pvBuffer,(PVOID)lpvBuffer,pTransfer->dwUserBufferSize,(((dwFlags& USB_IN_TRANSFER)!=0)?ARG_O_PTR:ARG_I_PTR)|MARSHAL_FORCE_ALIAS))) {
            pTransfer->pvBuffer = NULL;
            DEBUGMSG (ZONE_ERROR,(TEXT("IssueInterruptTransfer CeAllocAsynchronousBuffer Error (%d)\r\n"),GetLastError()));
            ASSERT(FALSE);
        }
    }
    
    pTransfer->lpdwMappedLengths = NULL;

    if(fWait)
    {
        pWait = GetWaitObject();
        lpStartAddress = &SignalEventFunc;
        lpvNotifyParameter = &pWait->hEvent;
    }

    fRet = (*pFunc)(pHcd->pvHcd, pDev->iDevice, pPipe->iEndpointIndex,
            lpStartAddress, lpvNotifyParameter, dwFlags, NULL, 0, 0,
            NULL, dwBufferSize, pTransfer->pvBuffer, uBufferPhysicalAddress, pTransfer,
            NULL, NULL, &pTransfer->fComplete, &pTransfer->dwBytesTransfered,
            &pTransfer->dwError);

    if (fRet && AddTransfer(pPipe,pTransfer))
    {
        if (fWait)
		{
            if ( WAIT_OBJECT_0 != WaitForSingleObject(pWait->hEvent, WAIT_DURATION_USB_TIMEOUT))
			{
				FreeTransferObject(pTransfer);
				pTransfer = NULL;
			}
		}
    }
    else
    {
        FreeTransferObject(pTransfer);
        pTransfer = NULL;
    }
    if(fWait)
        FreeWaitObject(pWait);

    DereferencePipeHandle(pPipe);

    DEBUGMSG(ZONE_API|ZONE_INTERRUPT,(TEXT("-USBD:IssueInterruptTransfer, pTransfer:0x%X\r\n"),pTransfer));
    return (USB_TRANSFER)pTransfer;
}

/*
 * @func   USB_TRANSFER | IssueIsochTransfer | Initiate isochronous transfer with USB device.
 * @rdesc  Returns a USB_TRANSFER handle, or NULL if an error occurs.
 * @comm   Initiates an isochronous transfer to USB device on the specified
 *         endpoint.   Since isochronous transfers must be executed continuously, a
 *         sequence of buffers is provided, one of which will be transferred each frame.
 *         There is no limit on the number of buffers that may be provided, though each
 *         one must be less than or equal to the maximum packet size for the endpoint.
 * @xref   <f GetIsochResults> <f IsTransferComplete> <f GetTransferStatus> <f AbortTransfer>
 */
extern "C" USB_TRANSFER
IssueIsochTransfer(
    USB_PIPE hPipe,               // @parm [IN] - Open USB pipe handle
    LPTRANSFER_NOTIFY_ROUTINE lpStartAddress, // @parm [IN] - Address of callback routine (may be NULL)
    LPVOID lpvNotifyParameter,    // @parm [IN] - Parameter to pass to callback routine
    DWORD dwFlags,                // @parm [IN] - Transfer flags (see usbtypes.h). The following
                                  //              flags are specific to isoch transfers:
                                  // USB_START_ISOCH_ASAP: Start transfer as soon as possible,
                                  //                       instead of specifying a starting frame.
                                  // USB_COMPRESS_ISOCH:   For IN transfers - will guarantee that
                                  //                       data is copied contiguously to user buffer,
                                  //                       as opposed to being fragmented if the
                                  //                       amount of data is less than the maximum requested
                                  //                       for a particular frame.
    DWORD dwStartingFrame,        // @parm [IN] - Frame number on which to start transfer
    DWORD dwFrames,               // @parm [IN] - Number of frames in transfer
    LPCDWORD lpdwLengths,         // @parm [IN/OUT] - Array of lengths (one entry per frame)
    LPVOID lpvBuffer,             // @parm [IN/OUT] - Data buffer (if physical buffer address is
                                  //                  specified, this must contain the virtual
                                  //                  address of the buffer)
    ULONG uBufferPhysicalAddress) // @parm [IN/OUT] - Physical address of data buffer (may be NULL).
{
    SPipe * pPipe = (SPipe *)hPipe;
    if (!ReferencePipeHandle(pPipe))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:IssueIsochTransfer - Invalid handle\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return NULL;
    }
    SDevice * pDev = pPipe->pDevice;
    SHcd * pHcd = pDev->pHcd;
    LPHCD_ISSUE_TRANSFER pFunc = pHcd->pHcdFuncs->lpIssueTransfer;
    BOOL fWait = FALSE;
    SWait * pWait;
    BOOL fRet;
    DEBUGMSG(ZONE_API|ZONE_ISOCH,
             (TEXT("+USBD:IssueIsochTransfer, pipe:%X, dwFlags:0x%X, StartFrame: %u, dwFrames: %u, vBuf:0x%X, pBuf:0x%X\r\n"),
              pPipe,dwFlags,dwStartingFrame,dwFrames,lpvBuffer,uBufferPhysicalAddress));

    if(!lpStartAddress)
        fWait = dwFlags & USB_NO_WAIT ? FALSE : TRUE;

    STransfer * pTransfer = GetTransferObject(pPipe,dwFrames);
    if (pTransfer == NULL)
    {
        DereferencePipeHandle(pPipe);
        SetLastError(ERROR_OUTOFMEMORY);
        return NULL;
    }

    DWORD dwBufferSize = 0;
    for(UINT iFrame = 0 ; iFrame < dwFrames ; ++iFrame)
    {
        pTransfer->adwIsochErrors[iFrame]  =  USB_NOT_COMPLETE_ERROR;
        pTransfer->adwIsochLengths[iFrame] = 0;
        if (lpdwLengths[iFrame] > pPipe->wMaxPktSize) {
            FreeTransferObject(pTransfer);
            DereferencePipeHandle(pPipe);
            SetLastError(ERROR_INVALID_PARAMETER);
            return NULL;
        }
        dwBufferSize += lpdwLengths[iFrame];
    }
    // Added Pointer for mapping.
    pTransfer->dwFlags = dwFlags;
    pTransfer->lpMappedControlHeader = NULL;
    pTransfer->dwUserBufferSize = dwBufferSize ;
    pTransfer->lpvOrigUserBuffer= lpvBuffer;
    pTransfer->pvBuffer = NULL;
    if ( lpvBuffer!=NULL && pTransfer->dwUserBufferSize!=0) {
        if (!SUCCEEDED(CeAllocAsynchronousBuffer((PVOID *)&pTransfer->pvBuffer,(PVOID)lpvBuffer,pTransfer->dwUserBufferSize,(((dwFlags& USB_IN_TRANSFER)!=0)?ARG_O_PTR:ARG_I_PTR)|MARSHAL_FORCE_ALIAS))) {
            pTransfer->pvBuffer = NULL;
            DEBUGMSG (ZONE_ERROR,(TEXT("IssueIsochTransfer CeAllocAsynchronousBuffer Error (%d)\r\n"),GetLastError()));
            ASSERT(FALSE);
        }
    }
    
    pTransfer->lpdwOrigLengths =(LPDWORD) lpdwLengths ;
    pTransfer->lpdwMappedLengths = NULL;
    if ( lpdwLengths!=NULL && dwFrames!=0  ) {
        if (!SUCCEEDED(CeAllocAsynchronousBuffer((PVOID *)&pTransfer->lpdwMappedLengths,(PVOID)lpdwLengths,dwFrames*sizeof(LPDWORD),ARG_I_PTR|MARSHAL_FORCE_ALIAS))) {
            pTransfer->lpdwMappedLengths = NULL;
            DEBUGMSG (ZONE_ERROR,(TEXT("IssueIsochTransfer CeAllocAsynchronousBuffer Error (%d)\r\n"),GetLastError()));
            ASSERT(FALSE);
        }
    }
    
    if(fWait)
    {
        pWait = GetWaitObject();
        lpStartAddress = &SignalEventFunc;
        lpvNotifyParameter = &pWait->hEvent;
    }

    fRet = (*pFunc)(pHcd->pvHcd, pDev->iDevice, pPipe->iEndpointIndex,
            lpStartAddress, lpvNotifyParameter, dwFlags, NULL, dwStartingFrame,
            dwFrames, pTransfer->lpdwMappedLengths, dwBufferSize, pTransfer->pvBuffer,
            uBufferPhysicalAddress, pTransfer, pTransfer->adwIsochErrors,
            pTransfer->adwIsochLengths, &pTransfer->fComplete,
            &pTransfer->dwBytesTransfered, &pTransfer->dwError);

    if (fRet && AddTransfer(pPipe,pTransfer))
    {
        if (fWait)
		{
            if ( WAIT_OBJECT_0 != WaitForSingleObject(pWait->hEvent, WAIT_DURATION_USB_TIMEOUT))
			{
				FreeTransferObject(pTransfer);
				pTransfer = NULL;
			}
		}
    }
    else
    {
        FreeTransferObject(pTransfer);
        pTransfer = NULL;
    }
    if(fWait)
        FreeWaitObject(pWait);

    DereferencePipeHandle(pPipe);

    DEBUGMSG(ZONE_API|ZONE_VENDOR,(TEXT("-USBD:IssueIsochTransfer, pTransfer:0x%X\r\n"),pTransfer));
    return (USB_TRANSFER)pTransfer;
}

/*
 * @func   BOOL | CloseTransfer | Close an active transfer handle
 * @rdesc  Return TRUE if successful, FALSE if error.
 * @comm   Will abort transfer if still in progress, and free resources
 *         associated with the transfer.  Note that the client driver must
 *         close a transfer handle even if an error occurs during the transfer.
 */
extern "C" BOOL
CloseTransfer(
    USB_TRANSFER hTransfer)  // @parm [IN] - USB transfer handle
{
    STransfer * pTransfer = (STransfer *)hTransfer;
    DWORD dwCount;
    if (! ReferenceTransferHandle(pTransfer))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:CloseTransfer, invalid handle\r\n")));
        return FALSE;
    }

    SPipe * pPipe = pTransfer->pPipe;
    SDevice * pDev = pPipe->pDevice;
    SHcd * pHcd =  pDev->pHcd;
    LPHCD_ABORT_TRANSFER pFunc = pHcd->pHcdFuncs->lpAbortTransfer;
    LPTRANSFER_NOTIFY_ROUTINE lpStartAddress = NULL;
    LPVOID lpvNotifyParameter = NULL;
    BOOL fRet = TRUE;

    DEBUGMSG(ZONE_API,(TEXT("+USBD:CloseTransfer, pTransfer:0x%X\r\n"),pTransfer));

    // we could be allocating and enlarging the current array of pipes
    // so we need a critical section
    EnterCriticalSection(&pPipe->csTransferLock);
    pPipe->apTransfers[pTransfer->iTransfer] = NULL;
    LeaveCriticalSection(&pPipe->csTransferLock);

    // we support multiple aborts on a single transfer, so let's abort it
    for (dwCount=0;dwCount<1000 && (!pTransfer->fComplete);dwCount++)
    {
        if (AbortTransfer(hTransfer, 0))
            break;
        else
            Sleep(1);
    }

    ASSERT(pTransfer->fComplete);

    DereferenceTransferHandle(pTransfer);

    // Free transfer memory.  This will block until the ref count goes to 0.
    FreeTransferObject(pTransfer);

    DEBUGMSG(ZONE_API,(TEXT("-USBD:CloseTransfer\r\n")));
    return fRet;
}

/*
 * @func   BOOL | AbortTransfer | Abort an active transfer.
 * @rdesc  Return TRUE if successful, FALSE if error.
 * @comm   Halt transfer in progress.  Will block until transfer
 *         has been successfully cancelled unless USB_NO_WAIT is
 *         specified, in which case, transfer is flagged for cancellation
 *         and routine returns immediately.  Completion routine for the transfer
 *         will be called, if one has been specified, and the transfer error code
 *         will be set to USB_CANCELLED_ERROR.
 */
extern "C" BOOL
AbortTransfer(
    USB_TRANSFER hTransfer, // @parm [IN] - USB transfer handle
    DWORD dwFlags)          // @parm [IN] - USB_NO_WAIT, or 0
{
    STransfer * pTransfer = (STransfer *)hTransfer;
    if (! ReferenceTransferHandle(pTransfer))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:AbortTransfer, invalid handle\r\n")));
        return FALSE;
    }
    SPipe * pPipe = pTransfer->pPipe;
    SDevice * pDev = pPipe->pDevice;
    SHcd * pHcd =  pDev->pHcd;
    LPHCD_ABORT_TRANSFER pFunc = pHcd->pHcdFuncs->lpAbortTransfer;
    LPTRANSFER_NOTIFY_ROUTINE lpStartAddress = NULL;
    LPVOID lpvNotifyParameter = NULL;
    BOOL fRet = TRUE;
    BOOL fIssueCancelRequest = FALSE;
    SWait * pWait = NULL;
    BOOL fWait = dwFlags & USB_NO_WAIT ? FALSE : TRUE;

    DEBUGMSG(ZONE_API,(TEXT("+USBD:AbortTransfer, pTransfer:%X, Wait:%u\r\n"),
                       hTransfer,!(dwFlags&USB_NO_WAIT)));

    if(!pTransfer->fComplete)
    {
        pWait = GetWaitObject();

        EnterCriticalSection(&pTransfer->csWaitObject);

        // Only issue abort if transfer is not already being cancelled
        if(!pTransfer->pWait)
        {
            pTransfer->pWait = pWait;
            fIssueCancelRequest = TRUE;
        }

        LeaveCriticalSection(&pTransfer->csWaitObject);

        if(fIssueCancelRequest)
        {
            lpStartAddress = &SignalEventFunc;
            lpvNotifyParameter = &pTransfer->pWait->hEvent;

            fRet =(*pFunc)(pHcd->pvHcd, pDev->iDevice, pPipe->iEndpointIndex,
                    lpStartAddress, lpvNotifyParameter, pTransfer);
        }
        else
        {
            FreeWaitObject(pWait);
        }

        if(!fRet)
            SetEvent(pTransfer->pWait->hEvent);

        if(fWait && fRet)
        {
            if (WaitForSingleObject(pTransfer->pWait->hEvent, 10000) != WAIT_OBJECT_0) {
               DEBUGMSG(ZONE_ERROR,(TEXT("!USBD:AbortTransfer Error in WFSO\r\n")));
               fRet = FALSE;
               ASSERT(FALSE);  // This should never happen
            }
        }
        else if (fWait) {
            for (DWORD dwTicks =0 ; dwTicks< 10*1000 && pTransfer->fComplete==FALSE ;) {
                Sleep(10);
                dwTicks +=10;
            }
            
        }
        DEBUGMSG(ZONE_API,(TEXT("-USBD:AbortTransfer, fRet:%u\r\n"),fRet));
    }
    else
    {
      DEBUGMSG(ZONE_API,(TEXT("-USBD:AbortTransfer, already complete\r\n")));
    }
    DereferenceTransferHandle(pTransfer);
    return fRet;
}


/*
 * @func   BOOL | IsTransferComplete | Check whether transfer has completed.
 * @rdesc  Return TRUE if transfer has completed, FALSE if not.
 */
extern "C" BOOL
IsTransferComplete(
    USB_TRANSFER hTransfer) // @parm [IN] - Transfer handle
{
    STransfer * pTransfer = (STransfer *)hTransfer;
    BOOL fRet;
    if (! ReferenceTransferHandle(pTransfer))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:IsTransferComplete, invalid handle\r\n")));
        return FALSE;
    }
    fRet = pTransfer->fComplete;
    DEBUGMSG(ZONE_API,(TEXT("+USBD:IsTransferComplete, ret %u\r\n"),fRet));
    DereferenceTransferHandle(pTransfer);
    return fRet;
}

/*
 * @func   BOOL | GetTransferError | Return error code associated with USB transfer.
 * @rdesc  Return TRUE if hTransfer is a valid transfer, FALSE if not.
 * @comm   Transfer error codes are defined in usbtypes.h.  If the transfer has not
 *         been completed, lpdwError is set to USB_NOT_COMPLETE_ERROR.
 */
extern "C" BOOL
GetTransferError(
    USB_TRANSFER hTransfer,  // @parm [IN] - Transfer handle
    LPDWORD lpdwError)       // @parm [OUT]- Filled in with error code
{
    STransfer * pTransfer = (STransfer *)hTransfer;
    if (! ReferenceTransferHandle(pTransfer))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:GetTransferError, invalid handle\r\n")));
        return FALSE;
    }
    DEBUGMSG(ZONE_API,(TEXT("+USBD:GetTransferError: %u\r\n"),pTransfer->dwError));

    if(lpdwError)
        *lpdwError = (pTransfer->fComplete)? pTransfer->dwError: USB_NOT_COMPLETE_ERROR;
    DereferenceTransferHandle(pTransfer);
    return TRUE;
}

/*
 * @func   BOOL | RegisterNotificationRoutine | Register a callback for device notifications.
 * @rdesc  Return TRUE if notification successfully registered, FALSE if error.
 * @comm   Currently the only notification is USB_CLOSE_DEVICE.
 * @xref   <f UnRegisterNotificationRoutine>
 */
extern "C" BOOL
RegisterNotificationRoutine(
    USB_HANDLE hDevice,                      // @parm [IN] - USB device handle
    LPDEVICE_NOTIFY_ROUTINE lpNotifyRoutine, // @parm [IN] - Address of notification callback
    LPVOID lpvNotifyParameter)               // @parm [IN] - Parameter to pass to lpNotifyRoutine
{
    SDevice * pDev = (SDevice *)hDevice;
    if (!ValidateDeviceHandle(pDev))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:RegisterNotificationRoutine - Invalid device handle\r\n")));
        return FALSE;
    }
    SNotifyList * pNotifyList = new SNotifyList;
    if (pNotifyList == NULL) {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:RegisterNotificationRoutine - Out of Memory\r\n")));
        return FALSE;
    }
    DEBUGMSG(ZONE_API,(TEXT("+USBD:RegisterNotificationRoutine\r\n")));

    pNotifyList->lpNotifyRoutine = lpNotifyRoutine;
    pNotifyList->lpvNotifyParameter = lpvNotifyParameter;

    EnterCriticalSection(&pDev->csSerializeNotifyRoutine);

    pNotifyList->pNext = pDev->pNotifyList;
    pDev->pNotifyList = pNotifyList;

    LeaveCriticalSection(&pDev->csSerializeNotifyRoutine);
    DEBUGMSG(ZONE_API,(TEXT("-USBD:RegisterNotificationRoutine\r\n")));
    return TRUE;
}

/*
 * @func   BOOL | UnRegisterNotificationRoutine | Remove device notification callback.
 * @rdesc  Return TRUE if notification successfully unregistered, FALSE if error.
 * @xref   <f RegisterNotificationRoutine>
 */
extern "C" BOOL
UnRegisterNotificationRoutine(
    USB_HANDLE hDevice,                      // @parm [IN] - USB device handle
    LPDEVICE_NOTIFY_ROUTINE lpNotifyRoutine, // @parm [IN] - Address of notification callback
    LPVOID lpvNotifyParameter)               // @parm [IN] - Must be the same as that passed to
                                             //              <f RegisterNotificationRoutine>
{
    BOOL fRet = FALSE;

    SDevice * pDev = (SDevice *)hDevice;
    if (!ValidateDeviceHandle(pDev))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:UnRegisterNotificationRoutine - Invalid device handle\r\n")));
        return FALSE;
    }
    SNotifyList * * ppNotifyList = &pDev->pNotifyList;
    SNotifyList * pTemp;
    DEBUGMSG(ZONE_API,(TEXT("+USBD:UnRegisterNotificationRoutine\r\n")));

    EnterCriticalSection(&pDev->csSerializeNotifyRoutine);

    while(*ppNotifyList && !fRet)
    {
        pTemp = *ppNotifyList;
        if(pTemp->lpNotifyRoutine == lpNotifyRoutine &&
                pTemp->lpvNotifyParameter == lpvNotifyParameter)
        {
            *ppNotifyList = pTemp->pNext;
            delete pTemp;
            fRet = TRUE;
            break;
        }
        ppNotifyList = &pTemp->pNext;
    }

    LeaveCriticalSection(&pDev->csSerializeNotifyRoutine);
    DEBUGMSG(ZONE_API,(TEXT("-USBD:UnRegisterNotificationRoutine\r\n")));
    return fRet;
}

/*
 * @func   BOOL | GetTransferStatus | Get status of active transfer.
 * @rdesc  Return TRUE if hTransfer is a valid transfer, FALSE otherwise.
 * @comm   Used to get the results of a completed transfer.  Note that even
 *         transfers which have errors may have valid data indicated.
 */
extern "C" BOOL
GetTransferStatus(
    USB_TRANSFER hTransfer,     // @parm [IN] - Transfer handle
    LPDWORD lpdwBytesTransfered,// @parm [OUT]- Filled in with # of bytes transferred (may be NULL)
    LPDWORD lpdwError)          // @parm [OUT]- Filled in with transfer error code or
                                //              USB_NO_ERROR (may be NULL)
{
    STransfer * pTransfer = (STransfer *)hTransfer;
    if (! ReferenceTransferHandle(pTransfer))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:GetTransferStatus, invalid handle\r\n")));
        return FALSE;
    }
    DEBUGMSG(ZONE_API,(TEXT("+USBD:GetTransferStatus\r\n")));

    if(pTransfer->fComplete)
    {
        if(lpdwBytesTransfered)
            *lpdwBytesTransfered = pTransfer->dwBytesTransfered;
        if(lpdwError)
            *lpdwError = pTransfer->dwError;
    }
    else
    {
        if(lpdwBytesTransfered)
            *lpdwBytesTransfered = 0;
        if(lpdwError)
            *lpdwError = USB_NO_ERROR;
    }
    DEBUGMSG(ZONE_API,(TEXT("-USBD:GetTransferStatus\r\n")));
    DereferenceTransferHandle(pTransfer);
    return TRUE;
}


/*
 * @func   BOOL | GetIsochResults | Get status of active isoch transfer.
 * @rdesc  Return TRUE if hTransfer is a completed isoch transfer, FALSE otherwise.
 * @comm   The supplied arrays are filled out with the error codes and amounts
 *         of data transferred for each frame.  Note that there may have been
 *         data transferred on a frame, even if an error is returned.
 */
extern "C" BOOL
GetIsochResults(
    USB_TRANSFER hTransfer,      // @parm [IN] - Isoch transfer handle
    DWORD cFrames,               // @parm [IN] - Number of frames to return results for
    LPDWORD lpdwBytesTransfered, // @parm [OUT]- Filled in with array of lengths (may be NULL)
    LPDWORD lpdwErrors)          // @parm [OUT]- Filled in with array of error codes (may be NULL)
{
    STransfer * pTransfer = (STransfer *)hTransfer;
    BOOL fRet = TRUE;
    if (! ReferenceTransferHandle(pTransfer))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:GetIsochResults, invalid handle\r\n")));
        return FALSE;
    }
    DWORD cMinFrames = cFrames <= pTransfer->cFrames ?
            cFrames : pTransfer->cFrames;
    DEBUGMSG(ZONE_API,(TEXT("+USBD:GetIsochResults\r\n")));

    if(lpdwBytesTransfered)
        memset(lpdwBytesTransfered, 0, cFrames * sizeof(*lpdwBytesTransfered));
    if(lpdwErrors)
        memset(lpdwErrors, 0, cFrames * sizeof(*lpdwErrors));

    if(pTransfer->fComplete)
    {
        if(lpdwBytesTransfered)
            memcpy(lpdwBytesTransfered, pTransfer->adwIsochLengths, cMinFrames * sizeof(*lpdwBytesTransfered));
        if(lpdwErrors)
            memcpy(lpdwErrors, pTransfer->adwIsochErrors, cMinFrames * sizeof(*lpdwErrors));
    }
    else
        fRet = FALSE;

    DereferenceTransferHandle(pTransfer);
    return fRet;
}

/*
 * @func   LPCUSB_DEVICE | GetDeviceInfo | Get pointer to device information structure.
 * @rdesc  Returns a pointer to USB_DEVICE structure for device.
 */
extern "C" LPCUSB_DEVICE
GetDeviceInfo(
    USB_HANDLE hDevice)  // @parm [IN] - USB device handle
{
    SDevice * pDev = (SDevice *)hDevice;
    if (!ValidateDeviceHandle(pDev))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:GetDeviceInfo - Invalid device handle\r\n")));
        return NULL;
    }
    DEBUGMSG(ZONE_API,(TEXT("+USBD:GetDeviceInfo\r\n")));
    return pDev->pDeviceInfo;
}

/*
 * @func   VOID | GetUSBDVersion | Get USBD version.
 * @rdesc  none
 * @comm   This function may be used to determine the current version
 *         of the USBDI interface.
 */
extern "C" VOID
GetUSBDVersion(
    LPDWORD lpdwMajorVersion,   // @parm [OUT] - Filled in with USBD major version number.
    LPDWORD lpdwMinorVersion)   // @parm [OUT] - Filled in with USBD minor version number.
{
    *lpdwMajorVersion = gcMajorUSBDVersion;
    *lpdwMinorVersion = gcMinorUSBDVersion;

    return;
}

/*
 * @func   BOOL | IsPipeHalted | Check if pipe is in halted state.
 * @rdesc  Return TRUE if successful, FALSE if error.
 * @comm   This function may be used to determine whether a pipe is
 *         halted or not.
 */
extern "C" BOOL
IsPipeHalted(
    USB_PIPE hPipe,     // @parm [IN] - USB pipe handle
    LPBOOL lpbHalted)   // @parm [OUT]- Set to TRUE if pipe is halted, FALSE if not
{
    SPipe * pPipe = (SPipe *)hPipe;
    if (!ReferencePipeHandle(pPipe))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:IsPipeHalted - Invalid handle\r\n")));
        return FALSE;
    }
    SDevice * pDev = pPipe->pDevice;
    SHcd * pHcd = pDev->pHcd;
    LPHCD_IS_PIPE_HALTED pFunc = pHcd->pHcdFuncs->lpIsPipeHalted;
    BOOL fRet;
    DEBUGMSG(ZONE_API,(TEXT("+USBD:IsPipeHalted\r\n")));
    fRet = (*pFunc)(pHcd->pvHcd, pDev->iDevice, pPipe->iEndpointIndex,
            lpbHalted);
    DereferencePipeHandle(pPipe);
    DEBUGMSG(ZONE_API,(TEXT("-USBD:IsPipeHalted: ret %u\r\n"),fRet));
    return fRet;
}


/*
 * @func   BOOL | IsDefaultPipeHalted | Check if EP0 pipe is in halted state.
 * @rdesc  Return TRUE if successful, FALSE if error.
 * @comm   This function may be used to determine whether a pipe is
 *         halted or not.  Note: this function is only available in USBDI version
 *         1.1 or later.
 */
extern "C" BOOL
IsDefaultPipeHalted(
    USB_HANDLE hDevice, // @parm [IN] - USB device handle
    LPBOOL lpbHalted)   // @parm [OUT]- Set to TRUE if pipe is halted, FALSE if not
{
    SDevice *pDev = (SDevice *)hDevice;
    if (!ValidateDeviceHandle(pDev))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:IsDefaultPipeHalted - Invalid device handle\r\n")));
        return FALSE;
    }
    SPipe *pDfltPipe;
    EnterCriticalSection(&pDev->csPipeLock);
    pDfltPipe = pDev->apPipes[gcEndpointZero];
    LeaveCriticalSection(&pDev->csPipeLock);

    return IsPipeHalted(pDfltPipe, lpbHalted);
}
// Help Function Set Device Bit.
BOOL SetDeviceBit(SDevice * pDev, BYTE bInterfaceNumber, BOOL fSet, BYTE bBit)
{
    BOOL fReturn = FALSE;
    if (pDev) {
        for (int iIndex=0; iIndex<gcMaxPipes; iIndex++) {
            if (pDev->rgbInterfaceIndex[iIndex]!= INVALID_INTERFACE_INDEX &&
                    pDev->rgbInterfaceIndex[iIndex] == bInterfaceNumber ) {
                if (fSet)
                    pDev->rgbInterfaceFlag[iIndex] |= bBit;
                else
                    pDev->rgbInterfaceFlag[iIndex] &= ~bBit;
                fReturn = TRUE;
                break;
            }
        }
    }
    return fReturn;
}
// Help Function Is All inteface Bit Set?
BOOL IsAllBitSet(SDevice * pDev, BYTE bBit)
{
    BOOL fReturn = TRUE;
    if (pDev) {
        for (int iIndex=0; iIndex<gcMaxPipes; iIndex++) {
            if (pDev->rgbInterfaceIndex[iIndex]!= INVALID_INTERFACE_INDEX &&
                    (pDev->rgbInterfaceFlag[iIndex] & INTERFACE_FLAG_ACTIVATE)!= 0 ) { // If it is activated device
                if ((bBit & pDev->rgbInterfaceFlag[iIndex]) != bBit) {
                    fReturn = FALSE;
                    break;
                }
            }
        }
    }
    return fReturn;
}
// Help Function Is One inteface bit set?
BOOL IsOneBitSet(SDevice * pDev, BYTE bBit)
{
    BOOL bReturn = FALSE;
    if (pDev) {
        for (int iIndex=0; iIndex<gcMaxPipes; iIndex++) {
            if (pDev->rgbInterfaceIndex[iIndex]!= INVALID_INTERFACE_INDEX &&
                    (pDev->rgbInterfaceFlag[iIndex] & INTERFACE_FLAG_ACTIVATE)!= 0 ) { // If it is activated device
                if ((bBit & pDev->rgbInterfaceFlag[iIndex]) == bBit) {
                    bReturn = TRUE;
                    break;
                }
            }
        }
    }
    return bReturn;
    
}
/*
 * @func   BOOL | DisableDevice | Disable the device that reference by hDevice.
 * @rdesc  Return TRUE if successful, FALSE if error.
 * @comm   if this device has multi-inteface, Device only be disable after DisableDevice called 
 *         on all the interface
 *         1.2 or later.
 */
extern "C" BOOL
DisableDevice(
    USB_HANDLE hDevice, // @parm [IN] - USB device handle
    BOOL    fResetDevice, // @parm [IN] - Optional Reset device. If it is true, device will be re-enumerate
    BYTE    bInterfaceNumber // @parm [IN] - Interface index number in Inteface Descriptor
    )
{
    SDevice *pDev = (SDevice *)hDevice;
    if (!ValidateDeviceHandle(pDev))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:DisableDevice - Invalid device handle\r\n")));
        return FALSE;
    }
    DEBUGMSG(ZONE_API,(TEXT("+USBD:DisableDevice\r\n")));
    EnterCriticalSection(&pDev->csPipeLock);
    SHcd * pHcd = pDev->pHcd;
    BYTE bBit = INTERFACE_FLAG_DISABLE;
    bBit |= (fResetDevice?INTERFACE_FLAG_RESET:0);
    BOOL bReturn = FALSE;
    PREFAST_ASSERT(pHcd!=NULL);
    PREFAST_ASSERT(pHcd->pHcdFuncs!=NULL);
    if (pHcd->pHcdFuncs->lpDisableDevice) {
        bReturn = SetDeviceBit(pDev, bInterfaceNumber, TRUE, bBit);
        if (bReturn && IsAllBitSet(pDev,INTERFACE_FLAG_DISABLE)) {
            fResetDevice = (IsOneBitSet(pDev,INTERFACE_FLAG_RESET) || fResetDevice);
            bReturn = (*(pHcd->pHcdFuncs->lpDisableDevice))(pHcd->pvHcd, pDev->iDevice, fResetDevice);
        }
    }
    LeaveCriticalSection(&pDev->csPipeLock);
    DEBUGMSG(ZONE_API,(TEXT("-USBD:DisableDevice(bReturn = %d\r\n"),bReturn));
    return bReturn;
}
/*
 * @func   BOOL | SuspendDevice | Suspend the device that reference by hDevice.
 * @rdesc  Return TRUE if successful, FALSE if error.
 * @comm   if this device has multi-inteface, Device only be suspend after Suspend Device called 
 *         on all the interface
 *         1.2 or later.
 */
extern "C" BOOL
SuspendDevice(
    USB_HANDLE hDevice, // @parm [IN] - USB device handle
    BYTE    bInterfaceNumber // @parm [IN] - Interface index number in Inteface Descriptor
    )
{
    SDevice *pDev = (SDevice *)hDevice;
    if (!ValidateDeviceHandle(pDev))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:SuspendDevice - Invalid device handle\r\n")));
        return FALSE;
    }
    DEBUGMSG(ZONE_API,(TEXT("+USBD:SuspendDevice\r\n")));
    EnterCriticalSection(&pDev->csPipeLock);
    SHcd * pHcd = pDev->pHcd;
    PREFAST_ASSERT(pHcd!=NULL);
    PREFAST_ASSERT(pHcd->pHcdFuncs!=NULL);
    BOOL bReturn = FALSE;
    if (pHcd->pHcdFuncs->lpSuspendResume) {
        bReturn= SetDeviceBit(pDev,bInterfaceNumber, TRUE,INTERFACE_FLAG_SUSPEND);
        if (bReturn && IsAllBitSet(pDev,INTERFACE_FLAG_SUSPEND) ) {
            bReturn = (*(pHcd->pHcdFuncs->lpSuspendResume))(pHcd->pvHcd, pDev->iDevice, TRUE);
        }
    }
    LeaveCriticalSection(&pDev->csPipeLock);
    DEBUGMSG(ZONE_API,(TEXT("-USBD:SuspendDevice (bReturn = %d\r\n"),bReturn));
    return bReturn;
}
 
/*
 * @func   BOOL | ResumeDevice | Suspend the device that reference by hDevice.
 * @rdesc  Return TRUE if successful, FALSE if error.
 * @comm   Device will be resume after this call.
 *         1.2 or later.
 */
extern "C" BOOL
ResumeDevice(
    USB_HANDLE hDevice, // @parm [IN] - USB device handle
    BYTE    bInterfaceNumber // @parm [IN] - Interface index number in Inteface Descriptor
    )
{
    SDevice *pDev = (SDevice *)hDevice;
    if (!ValidateDeviceHandle(pDev))
    {
        DEBUGMSG(ZONE_WARNING,(TEXT("!USBD:ResumeDevice - Invalid device handle\r\n")));
        return FALSE;
    }
    DEBUGMSG(ZONE_API,(TEXT("+USBD:ResumeDevice\r\n")));
    EnterCriticalSection(&pDev->csPipeLock);
    SHcd * pHcd = pDev->pHcd;
    PREFAST_ASSERT(pHcd!=NULL);
    PREFAST_ASSERT(pHcd->pHcdFuncs!=NULL);
    BOOL bReturn = FALSE;
    if (pHcd->pHcdFuncs->lpSuspendResume) {
        bReturn = SetDeviceBit(pDev,bInterfaceNumber, FALSE,INTERFACE_FLAG_SUSPEND);
        if (bReturn  ) {
            bReturn = (*(pHcd->pHcdFuncs->lpSuspendResume))(pHcd->pvHcd, pDev->iDevice, FALSE);
        }
        else
            bReturn = FALSE;
    }
    LeaveCriticalSection(&pDev->csPipeLock);
    DEBUGMSG(ZONE_API,(TEXT("-USBD:ResumeDevice (bReturn = %d\r\n"),bReturn));
    return bReturn;
}


