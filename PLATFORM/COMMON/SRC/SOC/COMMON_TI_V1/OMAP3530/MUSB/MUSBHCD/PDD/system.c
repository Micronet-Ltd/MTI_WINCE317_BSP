// All rights reserved ADENEO EMBEDDED 2010
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  
    system.c
    
Abstract:  
    Device dependant part of the Inventra MUSBMHDRC.

Notes: 
--*/

#pragma warning(push)
#pragma warning(disable :4115 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <ddkreg.h>
#include <devload.h>
#include <giisr.h>
#include <hcdddsi.h>
#include <omap3530_musbotg.h>
#include <omap3530_musbhcd.h>
#include <dvfs.h>
#pragma warning(pop)

#define REG_PHYSICAL_PAGE_SIZE TEXT("PhysicalPageSize")
#define REG_MAX_CURRENT        TEXT("MaxCurrent")

// Amount of memory to use for HCD buffer
#define gcTotalAvailablePhysicalMemory (0x20000) // 128K
#define gcHighPriorityPhysicalMemory (gcTotalAvailablePhysicalMemory/4)   // 1/4 as high priority

extern DWORD Host_ResumeIRQ (PVOID pHSMUSBContext);
extern DWORD Host_ProcessEP0(PVOID pHSMUSBContext);
extern DWORD Host_ProcessEPxRx(PVOID pHSMUSBContext, DWORD endpoint);
extern DWORD Host_ProcessEPxTx(PVOID pHSMUSBContext, DWORD endpoint);
extern DWORD Host_Connect(PVOID pHSMUSBContext);
extern DWORD Host_Disconnect(PVOID pHSMUSBContext);
extern DWORD Host_Suspend(PVOID pHSMUSBContext);
extern DWORD Host_ProcessDMA(PVOID pHSMUSBContext, UCHAR channel);
extern void HcdMdd_SignalExternalHub(LPVOID lpvHcdObject);
volatile UINT32 maxPower = 100;

MUSB_FUNCS gc_MUsbFuncs = 
{
    NULL,
    &Host_ResumeIRQ,
    &Host_ProcessEP0,
    &Host_ProcessEPxRx,
    &Host_ProcessEPxTx,
    &Host_Connect,
    &Host_Disconnect,
    &Host_Suspend,
    &Host_ProcessDMA
};

DWORD dwMaxCurrent = 100;

#define UnusedParameter(x)  x = x


/* HcdPdd_DllMain
 * 
 *  DLL Entry point.
 *
 * Return Value:
 */
extern BOOL HcdPdd_DllMain(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    UnusedParameter(hinstDLL);
    UnusedParameter(dwReason);
    UnusedParameter(lpvReserved);

    return TRUE;
}
static BOOL
GetRegistryPhysicalMemSize(
    LPCWSTR RegKeyPath,         // IN - driver registry key path
    DWORD * lpdwPhyscialMemSize)       // OUT - base address
{
    HKEY hKey;
    DWORD dwData;
    DWORD dwSize;
    DWORD dwType;
    BOOL  fRet=FALSE;
    DWORD dwRet;
    // Open key
    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,RegKeyPath,0,0,&hKey);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,(TEXT("!EHCD:GetRegistryConfig RegOpenKeyEx(%s) failed %d\r\n"),
                             RegKeyPath, dwRet));
        return FALSE;
    }

    // Read base address, range from registry and determine IOSpace
    dwSize = sizeof(dwData);
    dwRet = RegQueryValueEx(hKey, REG_PHYSICAL_PAGE_SIZE, 0, &dwType, (PUCHAR)&dwData, &dwSize);
    if (dwRet == ERROR_SUCCESS) {
        if (lpdwPhyscialMemSize)
            *lpdwPhyscialMemSize = dwData;
        fRet=TRUE;
    }
    RegCloseKey(hKey);
    return fRet;
}

static BOOL 
GetRegistryMaxPower(
    LPCWSTR RegKeyPath,      // IN - driver registry key path
    DWORD * lpdwMaxCurrent)  // OUT - base address
{
    HKEY hKey;
    DWORD dwData;
    DWORD dwSize;
    DWORD dwType;
    BOOL  fRet=FALSE;
    DWORD dwRet;
    // Open key
    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,RegKeyPath,0,0,&hKey);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,(TEXT("!EHCD:GetRegistryConfig RegOpenKeyEx(%s) failed %d\r\n"),
                             RegKeyPath, dwRet));
        return FALSE;
    }

    // Read base address, range from registry and determine IOSpace
    dwSize = sizeof(dwData);
    dwRet = RegQueryValueEx(hKey, REG_MAX_CURRENT, 0, &dwType, (PUCHAR)&dwData, &dwSize);
    if (dwRet == ERROR_SUCCESS) {
        if (lpdwMaxCurrent)
            *lpdwMaxCurrent = dwData;
        fRet=TRUE;
    }
    RegCloseKey(hKey);
    return fRet;
}


//------------------------------------------------------------------------------
//
//  Function:  ConfigureMHDRCCard
//
//  Routine Description:
//
//      This is to configure the MHDRC and it supposed to be done by
//      OTG already. 
//
//  Arguments:
//
//      pPddObject - Pointer to PDD reference context.
//
//  Return values:
//
//      TRUE
//
BOOL
ConfigureMHDRCCard(SMHCDPdd * pPddObject)
{
	UNREFERENCED_PARAMETER(pPddObject);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("ConfigureMHDRCCard\r\n")));
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  InitializeMHDRC
//
//  Routine Description:
//
//      Initialization routine for MHDRC including reading registry,
//      setting etc.      
//
//  Arguments:
//
//      pPddObject        :   Pointer to PDD object
//      szDriverRegKey    :   Registry Path
//
//  Return values:
//
//      TRUE - success
//      FALSE - fail
static BOOL 
InitializeMHDRC(
    SMHCDPdd * pPddObject,    // IN - Pointer to PDD structure
    LPCWSTR szDriverRegKey)   // IN - Pointer to active registry key string
{     
    BOOL fResult = FALSE;
    LPVOID pobMem = NULL;
    LPVOID pobMHCD = NULL;
    DWORD dwHPPhysicalMemSize = 0;
    HKEY    hKey;
    DDKWINDOWINFO dwi;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,szDriverRegKey,0,0,&hKey)!= ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,(TEXT("InitializeMHDRC:GetRegistryConfig RegOpenKeyEx(%s) failed\r\n"),
                             szDriverRegKey));
        return FALSE;
    }

    dwi.cbSize=sizeof(dwi);
    if ( DDKReg_GetWindowInfo(hKey, &dwi ) !=ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,(TEXT("InitializeMHDRC:DDKReg_GetWindowInfo or  DDKReg_GetWindowInfo failed\r\n")));
        goto InitializeMHDRC_Error;
    }
    
	fResult = ConfigureMHDRCCard(pPddObject);
    if (!fResult) {
        goto InitializeMHDRC_Error;
    }

    pPddObject->bDMASupport = FALSE;
    if ( hKey!=NULL)  {
        DWORD dwDMA;
        DWORD dwType;
        DWORD dwLength = sizeof(DWORD);
        if (RegQueryValueEx(hKey, HCD_DMA_SUPPORT, 0, &dwType, (PUCHAR)&dwDMA, &dwLength) == ERROR_SUCCESS)
            pPddObject->bDMASupport = ((dwDMA == 0)? FALSE: TRUE);
    }

    DEBUGMSG(ZONE_VERBOSE, (TEXT("DMA Support = %d\r\n"), pPddObject->bDMASupport));
#if 0
    RETAILMSG(1, (TEXT("Force DMA support to 0\r\n")));
    pPddObject->bDMASupport = 0;
#endif
    pPddObject->dwDMAMode = 1;
    DEBUGMSG(ZONE_VERBOSE, (TEXT("DMA Mode = %d\r\n"), pPddObject->dwDMAMode));


    pPddObject->nDVFSOrder = 150;
    if ( hKey!=NULL)  {
        DWORD dwDVFSOrder = 0;
        DWORD dwType;
        DWORD dwLength = sizeof(DWORD);
        if (RegQueryValueEx(hKey, HCD_DVFS_ORDER, 0, &dwType, (PUCHAR)&dwDVFSOrder, &dwLength) == ERROR_SUCCESS)
            if (dwDVFSOrder != 0)
                pPddObject->nDVFSOrder = dwDVFSOrder;
    }
    DEBUGMSG(ZONE_VERBOSE, (TEXT("InitializeMHDRC pPddObject[0x%x], order[%d]\r\n"), pPddObject, pPddObject->nDVFSOrder));

    // get max current that can be supplied to devices inserted into the OTG port
    GetRegistryMaxPower(szDriverRegKey, &dwMaxCurrent);

    // The PDD can supply a buffer of contiguous physical memory here, or can let the 
    // MDD try to allocate the memory from system RAM.  We will use the HalAllocateCommonBuffer()
    // API to allocate the memory and bus controller physical addresses and pass this information
    // into the MDD.
    if (GetRegistryPhysicalMemSize(szDriverRegKey,&pPddObject->dwPhysicalMemSize)) {
        // A quarter for High priority Memory.
        dwHPPhysicalMemSize = pPddObject->dwPhysicalMemSize/4;
        // Align with page size.        
        pPddObject->dwPhysicalMemSize = (pPddObject->dwPhysicalMemSize + PAGE_SIZE -1) & ~(PAGE_SIZE -1);
        dwHPPhysicalMemSize = ((dwHPPhysicalMemSize +  PAGE_SIZE -1) & ~(PAGE_SIZE -1));
    }
    else 
        pPddObject->dwPhysicalMemSize=0;
    
    if (pPddObject->dwPhysicalMemSize<gcTotalAvailablePhysicalMemory) { // Setup Minimun requirement.
        pPddObject->dwPhysicalMemSize = gcTotalAvailablePhysicalMemory;
        dwHPPhysicalMemSize = gcHighPriorityPhysicalMemory;
    }

    pPddObject->AdapterObject.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);
    pPddObject->AdapterObject.InterfaceType = dwi.dwInterfaceType;
    pPddObject->AdapterObject.BusNumber = dwi.dwBusNumber;
    if ((pPddObject->pvVirtualAddress = HalAllocateCommonBuffer(&pPddObject->AdapterObject, pPddObject->dwPhysicalMemSize, &pPddObject->LogicalAddress, FALSE)) == NULL) {
        goto InitializeMHDRC_Error;
    }

	pobMem = HcdMdd_CreateMemoryObject(pPddObject->dwPhysicalMemSize, dwHPPhysicalMemSize, (PUCHAR) pPddObject->pvVirtualAddress, (PUCHAR) pPddObject->LogicalAddress.LowPart);
    if (!pobMem) {
        goto InitializeMHDRC_Error;
    }

	pobMHCD = HcdMdd_CreateHcdObject(pPddObject, pobMem, szDriverRegKey, 0x00, 0x00);
    if (!pobMHCD) {
        goto InitializeMHDRC_Error;
    }

    pPddObject->lpvMemoryObject = pobMem;
    pPddObject->lpvMHCDMddObject = pobMHCD;
    DEBUGMSG(ZONE_VERBOSE, (TEXT("InitializeMHDRC with Base(0x%x), MddObject(0x%x)\r\n"), pPddObject, pPddObject->lpvMHCDMddObject));
    _tcsncpy(pPddObject->szDriverRegKey, szDriverRegKey, MAX_PATH);    
    
    if ( hKey!=NULL)  {
        RegCloseKey(hKey);
    }

    DEBUGMSG(ZONE_VERBOSE, (TEXT("InitializeMHDRC success\r\n")));
    return TRUE;

InitializeMHDRC_Error:
    if (pPddObject->IsrHandle) {
        FreeIntChainHandler(pPddObject->IsrHandle);
        pPddObject->IsrHandle = NULL;
    }
    
    if (pobMHCD)
        HcdMdd_DestroyHcdObject(pobMHCD);
    if (pobMem)
        HcdMdd_DestroyMemoryObject(pobMem);
    if(pPddObject->pvVirtualAddress)
        HalFreeCommonBuffer(&pPddObject->AdapterObject, pPddObject->dwPhysicalMemSize, pPddObject->LogicalAddress, pPddObject->pvVirtualAddress, FALSE);

    pPddObject->lpvMemoryObject = NULL;
    pPddObject->lpvMHCDMddObject = NULL;
    pPddObject->pvVirtualAddress = NULL;
    if ( hKey!=NULL) 
        RegCloseKey(hKey);

    return FALSE;
}


//------------------------------------------------------------------------------
//
//  Function:  CheckAndHaltAllDma
//
//  Called by dma read/write routines to synchronize dma activity
//  with current DVFS transitions.
//
static void CheckAndHaltAllDma(SMHCDPdd *pPdd, BOOL bHalt)
{    
    DEBUGMSG(ZONE_FUNCTION, (
        L"+CheckAndHaltAllDma(0x%08x, %d)\r\n", 
        pPdd,bHalt
        ));

    EnterCriticalSection(&pPdd->csDVFS);

    if (pPdd->nActiveDmaCount == 0 && bHalt == TRUE)
        {
        DEBUGMSG(1, (L"*** USBHCD: CheckAndHaltAllDma: set hDVFSAckEvent\r\n"));
        SetEvent(pPdd->hDVFSAckEvent);
        }  
    
    LeaveCriticalSection(&pPdd->csDVFS);

    DEBUGMSG(ZONE_FUNCTION, (L"-CheckAndHaltAllDma()\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  CopyDVFSHandles
//
//  Called by dma read/write routines to synchronize dma activity
//  with current DVFS transitions.
//
static void CopyDVFSHandles(SMHCDPdd *pPdd, UINT processId, HANDLE hAckEvent, HANDLE hActivityEvent)
{
    DEBUGMSG(ZONE_FUNCTION, (
        L"+CopyDVFSHandles(0x%08x, 0x%08X, 0x%08X)\r\n", 
        pPdd, hAckEvent, hActivityEvent
    ));
        
    // make copies of event handles
    DuplicateHandle((HANDLE)processId, hAckEvent, 
        GetCurrentProcess(), &pPdd->hDVFSAckEvent, 
        0, FALSE, DUPLICATE_SAME_ACCESS
        );

    DuplicateHandle((HANDLE)processId, hActivityEvent, 
        GetCurrentProcess(), &pPdd->hDVFSActivityEvent, 
        0, FALSE, DUPLICATE_SAME_ACCESS
        );

    DEBUGMSG(ZONE_FUNCTION, (
        L"-CopyDVFSHandles()\r\n"
    ));
}

//------------------------------------------------------------------------------
//
//  Function:  PreDmaActivation
//
//  Called by dma read/write routines to synchronize dma activity
//  with current DVFS transitions.
//
BOOL HcdPdd_PreTransferActivation(SMHCDPdd *pPdd)
{    
    BOOL retVal=TRUE;
    DEBUGMSG(ZONE_FUNCTION, (
        L"+PreDmaActivation\r\n"
    ));

    for (;;)
    {
    
        // this operation needs to be atomic to handle a corner case
        EnterCriticalSection(&pPdd->csDVFS);
        
        // check and wait for DVFS activity to complete
        if (pPdd->bDVFSActive == TRUE)
        {
            DWORD dwRet;

            DEBUGMSG(ZONE_FUNCTION, (L"*** DVFS in progress wait before doing DMA\r\n"));
            dwRet = WaitForSingleObject(pPdd->hDVFSActivityEvent, 0);
            if (dwRet != WAIT_TIMEOUT)
            {
                InterlockedIncrement(&pPdd->nActiveDmaCount);
                LeaveCriticalSection(&pPdd->csDVFS);
                break;
            }
        }
        else
        {
            InterlockedIncrement(&pPdd->nActiveDmaCount);
            LeaveCriticalSection(&pPdd->csDVFS);
            break;
        }
        LeaveCriticalSection(&pPdd->csDVFS);  // hDVFSActivityEvent not signaled

    }

    DEBUGMSG(ZONE_HCD, (
        L"PreDmaActivation() with count %d\r\n", pPdd->nActiveDmaCount
        ));

    return retVal;
}

//------------------------------------------------------------------------------
//
//  Function:  HcdPdd_PostTransferDeactivation
//
//  Called by dma read/write routines to synchronize dma activity
//  with current DVFS transitions.
//
void HcdPdd_PostTransferDeactivation(SMHCDPdd *pPdd)
{
        
    DEBUGMSG(ZONE_HCD, (
        L"+PostDmaDeactivation\r\n"
    ));
    
    ASSERT(pPdd->nActiveDmaCount > 0);

    // this operation needs to be atomic to handle a corner case
    EnterCriticalSection(&pPdd->csDVFS);
    
    // check if all dma's are inactive and signal ack event if so
    InterlockedDecrement(&pPdd->nActiveDmaCount);
    if (pPdd->bDVFSActive == TRUE && pPdd->nActiveDmaCount <= 0)
        {
        DEBUGMSG(ZONE_HCD, (L"***USBHCD:finished all Dma's set hDVFSAckEvent\r\n"));
        SetEvent(pPdd->hDVFSAckEvent);
        }

    LeaveCriticalSection(&pPdd->csDVFS);

    DEBUGMSG(ZONE_HCD, (
        L"PostDmaDeactivation() with count %d\r\n", pPdd->nActiveDmaCount
        ));
}

//------------------------------------------------------------------------------------
//  Function: HcdPdd_Init
//
//  Routine description:
//
//      PDD Entry point - called at system init to detect and configure EHCI card.
//
//
//  Argument:
//
//      dwContext - Pointer to the registry entry
//
//  Return Value:
//
//      Return pointer to PDD specific data structure, or NULL if error.
//
extern DWORD 
HcdPdd_Init(
    DWORD dwContext)  // IN - Pointer to context value. For device.exe, this is a string 
                      //      indicating our active registry key.
{
    SMHCDPdd *  pPddObject = malloc(sizeof(SMHCDPdd));
    BOOL        fRet = FALSE;
    HMODULE m_hOTGInstance;
    LPMUSB_ATTACH_PROC lphAttachProc;
    PHSMUSB_T pOTG;


    DEBUGMSG(ZONE_FUNCTION, (TEXT("HcdPdd_Init+\r\n")));
    if (pPddObject) {
        pPddObject->pvVirtualAddress = NULL;
        InitializeCriticalSection(&pPddObject->csPdd);
        pPddObject->IsrHandle = NULL;
        pPddObject->hParentBusHandle = CreateBusAccessHandle((LPCWSTR)g_dwContext); 
        
        fRet = InitializeMHDRC(pPddObject, (LPCWSTR)dwContext);
        if (!fRet)
            goto END;
    }
    else
    {
        DEBUGMSG(ZONE_WARNING, (TEXT("HcdPdd_Init cannot allocate pPddObject\r\n")));
        goto END;
    }


   InitializeCriticalSection(&pPddObject->csDVFS);

    // initialize dvfs variables
    pPddObject->bDVFSActive = FALSE;
    pPddObject->nActiveDmaCount = 0;
    pPddObject->hDVFSAckEvent = NULL;
    pPddObject->hDVFSActivityEvent = NULL;    

    // Now we need to register for the OTG to acknowledge it is ready.
    // Get the OTG module handle
    m_hOTGInstance = GetModuleHandle(OTG_DRIVER);
    if (m_hOTGInstance == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Failure to load %s\r\n"), OTG_DRIVER));
        goto END;
    }

    pPddObject->m_lpUSBClockProc = (LPMUSB_USBCLOCK_PROC)GetProcAddress(m_hOTGInstance, TEXT("OTGUSBClock"));
    if (pPddObject->m_lpUSBClockProc == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Failure to get OTGUSBClock\r\n")));
        goto END;
    }

    lphAttachProc = (LPMUSB_ATTACH_PROC)GetProcAddress(m_hOTGInstance, TEXT("OTGAttach"));
    if (lphAttachProc == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Failure to get OTGAttach\r\n")));
        goto END;
    }

    fRet = (*lphAttachProc)(&gc_MUsbFuncs, HOST_MODE, (LPLPVOID)&pOTG);
    if (fRet == FALSE)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Error in performing the attach procedure\r\n")));
        goto END;
    }

    pOTG->pContext[HOST_MODE-1] = pPddObject;

    SetEvent(pOTG->hReadyEvents[HOST_MODE-1]);
END:
    if(!fRet)
    {
        if (pPddObject && pPddObject->hParentBusHandle)
             CloseBusAccessHandle(pPddObject->hParentBusHandle);
            
        DeleteCriticalSection(&pPddObject->csPdd);
        free(pPddObject);
        pPddObject = NULL;
    }
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("HcdPdd_Init-\r\n")));
    return (DWORD)pPddObject;
}

//-----------------------------------------------------------------------------------
//  Function: HcdPdd_CheckConfigPower
//
//  Routine Description:
//
//    Check power required by specific device configuration and return whether it
//    can be supported on this platform.  For CEPC, this is trivial, just limit to
//    the 500mA requirement of USB.  For battery powered devices, this could be 
//    more sophisticated, taking into account current battery status or other info.
//
//  Arguments:
//
//    bPort - Port Number to be checked
//    dwCfgPower - Power required by configuration
//    dwTotalPower - Total power currently in use on port
//
//  Return Value:
//
//    Return TRUE if configuration can be supported, FALSE if not.
//
extern BOOL HcdPdd_CheckConfigPower(
    UCHAR bPort,         // IN - Port number
    DWORD dwCfgPower,    // IN - Power required by configuration
    DWORD dwTotalPower)  // IN - Total power currently in use on port
{
	UNREFERENCED_PARAMETER(bPort);

    return ((dwCfgPower + dwTotalPower) > dwMaxCurrent) ? FALSE : TRUE;
}

//-----------------------------------------------------------------------------
//  Function:   HcdPdd_PowerUp
//
//  Routine Description:
//
//      Power Up routine for system resume
//
//  Argument:
//
//      hDeviceContext - Pointer to SMHCDPDD context
//
//  Return:
//
//      NULL
//
extern void HcdPdd_PowerUp(DWORD hDeviceContext)
{
#if 0
    SMHCDPdd * pPddObject = (SMHCDPdd *)hDeviceContext;
    HcdMdd_PowerUp(pPddObject->lpvMHCDMddObject);
#else
	UNREFERENCED_PARAMETER(hDeviceContext);
#endif
    return;
}

//-----------------------------------------------------------------------------
//  Function:   HcPdd_PowerDown
//
//  Routine Description:
//
//      Power Down routine for system suspend
//
//  Argument:
//
//      hDeviceContext - Pointer to SMHCDPDD context
//
//  Return:
//
//      NULL
//
extern void HcdPdd_PowerDown(DWORD hDeviceContext)
{
#if 0
    SMHCDPdd * pPddObject = (SMHCDPdd *)hDeviceContext;
    HcdMdd_PowerDown(pPddObject->lpvMHCDMddObject);
#else
	UNREFERENCED_PARAMETER(hDeviceContext);
#endif
    return;
}

//-----------------------------------------------------------------------------
//  Function:   HcdPdd_Deinit
//
//  Routine Description:
//
//      Deinit routine for USB Host Driver
//
//  Argument:
//
//      hDeviceContext - Pointer to SMHCDPDD context
//
//  Return:
//
//      TRUE
//
extern BOOL HcdPdd_Deinit(DWORD hDeviceContext)
{
    SMHCDPdd * pPddObject = (SMHCDPdd *)hDeviceContext;

    if(pPddObject->lpvMHCDMddObject)
        HcdMdd_DestroyHcdObject(pPddObject->lpvMHCDMddObject);
    if(pPddObject->lpvMemoryObject)
        HcdMdd_DestroyMemoryObject(pPddObject->lpvMemoryObject);
    if(pPddObject->pvVirtualAddress)
        HalFreeCommonBuffer(&pPddObject->AdapterObject, pPddObject->dwPhysicalMemSize, pPddObject->LogicalAddress, pPddObject->pvVirtualAddress, FALSE);

    free(pPddObject);
    return TRUE;
    
}

//-----------------------------------------------------------------------------
//  Function:   HcdPdd_Open
//
//  Routine Description:
//
//      This routine is called when device is opened.
//
//  Argument:
//
//      hDeviceContext - Pointer to SMHCDPDD context
//      AccessCode - the access of this open device
//      ShareMode - share mode
//
//  Return:
//
//      hDeviceContext
//
extern DWORD HcdPdd_Open(DWORD hDeviceContext, DWORD AccessCode,
        DWORD ShareMode)
{    
    UnusedParameter(AccessCode);
    UnusedParameter(ShareMode);

    return hDeviceContext; // we can be opened, but only once!
}

//-----------------------------------------------------------------------------
//  Function:   HcdPdd_Close
//
//  Routine Description:
//
//      Closing routine when device is closed
//
//  Argument:
//
//      hDeviceContext - Pointer to SMHCDPDD context
//
//  Return:
//
//      NULL
//
extern BOOL HcdPdd_Close(DWORD hOpenContext)
{
    UnusedParameter(hOpenContext);

    return TRUE;
}

//-----------------------------------------------------------------------------
//  Function:   HcdPdd_Read
//
//  Routine Description:
//
//      Stream interface for reading the device.
//
//  Argument:
//
//      hOpenDeviceContext - Pointer to SMHCDPDD context
//      pBuffer - Pointer to buffer storing the data to be read from device.
//      Count - size of the buffer
//
//  Return:
//
//      no of bytes to be read
//
extern DWORD HcdPdd_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
    UnusedParameter(hOpenContext);
    UnusedParameter(pBuffer);
    UnusedParameter(Count);

    return (DWORD)-1; // an error occured
}

//-----------------------------------------------------------------------------
//  Function:   HcdPdd_Write
//
//  Routine Description:
//
//      Stream interface for write the device.
//
//  Argument:
//
//      hOpenDeviceContext - Pointer to SMHCDPDD context
//      pSourceBuffer - Pointer to buffer storing the data to be sent to device.
//      NumberOfBytes - size of the data
//
//  Return:
//
//      no of bytes that have been written
//
extern DWORD HcdPdd_Write(DWORD hOpenContext, LPCVOID pSourceBytes,
        DWORD NumberOfBytes)
{
    UnusedParameter(hOpenContext);
    UnusedParameter(pSourceBytes);
    UnusedParameter(NumberOfBytes);

    return (DWORD)-1;
}

//-----------------------------------------------------------------------------
//  Function:   HcdPdd_Seek
//
//  Routine Description:
//
//      Stream interface for seeking the data location the device.
//
//  Argument:
//      
//      Not used
//
//  Return:
//
//      Not used
//
extern DWORD HcdPdd_Seek(DWORD hOpenContext, LONG Amount, DWORD Type)
{
    UnusedParameter(hOpenContext);
    UnusedParameter(Amount);
    UnusedParameter(Type);

    return (DWORD)-1;
}

//-----------------------------------------------------------------------------
//  Function:   HcdPdd_IOControl
//
//  Routine Description:
//
//      Stream interface for I/O Control of the device.
//
//  Argument:
//
//      Not used at this point
//
//  Return:
//
//      -1
//
extern BOOL HcdPdd_IOControl(DWORD context, DWORD dwCode, PBYTE pBufIn,
        DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
    BOOL rc = FALSE;

	UNREFERENCED_PARAMETER(pdwActualOut);
	UNREFERENCED_PARAMETER(dwLenOut);
	UNREFERENCED_PARAMETER(pBufOut);
	UNREFERENCED_PARAMETER(dwLenIn);
	UNREFERENCED_PARAMETER(pBufIn);
	UNREFERENCED_PARAMETER(context);

    DEBUGMSG(ZONE_FUNCTION, (L"HcdPdd_IOControl with context (0x%x)\r\n", context));

    switch (dwCode)
        {
        case IOCTL_DVFS_OPPNOTIFY:
            break;
#if 0        
        case IOCTL_DVFS_OPMNOTIFY:
        {
            IOCTL_DVFS_OPMNOTIFY_IN *pData =(IOCTL_DVFS_OPMNOTIFY_IN*)pBufIn;
            
            DEBUGMSG(ZONE_FUNCTION, (L"HcdPdd: received dvfs notification (%d)\r\n",
                pData->notification)
                );
            
            // this operation should be atomic to handle a corner case
            EnterCriticalSection(&pPdd->csDVFS);
            
            // signal dvfs thread to stall SDRAM access
            if (pData->notification == kPreNotice)
            {
                pPdd->bDVFSActive = TRUE;
                HcdMdd_SignalExternalHub(pPdd->lpvMHCDMddObject);
                
                // check and halt dma if active
                //
                DEBUGMSG(ZONE_FUNCTION, (L"HcdPdd: Halting DMA for DVFS, "
                    L"active dma count=%d\r\n",
                    pPdd->nActiveDmaCount)
                    );
                CheckAndHaltAllDma(pPdd, TRUE);
                DEBUGMSG(ZONE_FUNCTION, (L"HcdPdd: Pre-DVFS transition done\r\n"));
            }
            else if (pData->notification == kPostNotice)
            {
                pPdd->bDVFSActive = FALSE;
                
                DEBUGMSG(ZONE_FUNCTION, (L"HcdPdd: continuing DMA for DVFS\r\n"));
                CheckAndHaltAllDma(pPdd, FALSE);
                DEBUGMSG(ZONE_FUNCTION, (L"HcdPdd: Post-DVFS transition done\r\n"));
            }
            LeaveCriticalSection(&pPdd->csDVFS);
            rc = TRUE;
        }
            break;

        case IOCTL_DVFS_INITINFO:
        {
            IOCTL_DVFS_INITINFO_OUT *pInitInfo =(IOCTL_DVFS_INITINFO_OUT*)pBufOut;
            pInitInfo->notifyMode = kAsynchronous;
            pInitInfo->notifyOrder = pPdd->nDVFSOrder;
            RETAILMSG(1, (TEXT("HSUSBHCD::DVFS_INITINFO return order %d\r\n"), pPdd->nDVFSOrder));
            rc = TRUE;
        }
            break;
        
        case IOCTL_DVFS_OPMINFO:
        {
            IOCTL_DVFS_OPMINFO_IN *pData =(IOCTL_DVFS_OPMINFO_IN*)pBufIn;
            CopyDVFSHandles(pPdd, pData->processId, 
                pData->hAckEvent, pData->hOpmEvent
                );
            RETAILMSG(1, (TEXT("HSUSBHCD::DVFS_OPMINFO\r\n")));
            rc = TRUE;
        }
            break;

        case IOCTL_DVFS_DETACH:
        {
            // close all handles
            if (pPdd->hDVFSAckEvent != NULL)
            {
                CloseHandle(pPdd->hDVFSAckEvent);
                pPdd->hDVFSAckEvent = NULL;
            }
            
            if (pPdd->hDVFSActivityEvent!= NULL)
            {
                CloseHandle(pPdd->hDVFSActivityEvent);
                pPdd->hDVFSActivityEvent = NULL;
            }
            rc = TRUE;
        }
            break;

        case IOCTL_DVFS_HALTMODE:
        {
            if (pPdd->bDVFSActive == FALSE)
            {                
                IOCTL_DVFS_HALTMODE_IN *pData =(IOCTL_DVFS_HALTMODE_IN*)pBufIn;
                pPdd->rxHaltMode = pData->rxMode;
                pPdd->txHaltMode = pData->txMode;
                rc = TRUE;
            }
            break;
        }
#endif        
        }
    return rc;
}


//----------------------------------------------------------------
//  Function: HcdPdd_InitiatePowerUp
//
//  Routine description:
//
//      Manage WinCE suspend/resume events
//
//      This gets called by the MDD's IST when it detects a power resume.
//      By default it has nothing to do.
//
//  Argument:
//      
//      hDeviceContext - Pointer to SMHCDPDD
//
//  Return values:
//
//      Nil
//
extern void HcdPdd_InitiatePowerUp (DWORD hDeviceContext)
{
	UNREFERENCED_PARAMETER(hDeviceContext);
}
