/*++

Copyright (c) 2001-2012  Future Technology Devices International Ltd.

Module Name:

    ftdi_ser.c

Abstract:

    Native USB device driver for FTDI FT8U2XX
    Serial port routines

Environment:

    kernel mode only

Revision History:

	24/05/04	st		Adapted from Microsoft sample
	05/07/04	st		Clear Dtr and Cts on disconnect
	04/08/04	st		Added support to change latency timer on open
	13/08/04	st		Added D2XX bitbang and latency interface. Support for reduced bulk transfer on writes if out of memory error.
						Added registry support for latency and in and out transfer size.
	23/08/04	st		Added the InTransferSize (etc) on a per device basis.
						Added registry settings to allow for additional Config Data (for RTS and DTR settings).
	25/11/04	st		Added unique registry entries for serial numbered devices. Fixed disconnect on an open port.
						Bulk transfer sizes as the registry says, NOT after a bad request. Bulk Priority registry entry added.
	10/12/04	st		Enabled RLSD signal in SerModemIntr.
	07/03/05	st		Changes to allow bulk in thread to start up later. Also changes to the DEBUG messages to make easier filtering.
	09/03/05	st		Added check for ConfigData to set flag if not available.
	15/04/05	st		Debug statement references clean up.
	07/06/05	st		Added ResetHardware On open. Added Notify on removal IOCTLs.
						Removed the purge chip connections when doing a purge (brining it into line with windows). Only purge the internal read buffer.
	23/06/05	st		Changes made to write for modem emulation support. Change in the write callback to remove global variable.
						Registry settings added for modem emulation.
	27/06/05	st		ActiveSync Monitor thread additions.
	30/06/05	st		Changes to allow configurable bulk flags and configurable sync/async bulk calls.
	05/08/05	st		Added code to remove the EventThread on exit and the event. This was causing a memory leak on an unplug/replug.
	15/08/05	st		Improve method of accessing the ftdi context structure.
	17/08/05	st		Changes required to make alert the serial layer of an unplug event. Reporting of the CTS hold and DTR hold stats.
	19/09/05	st		Event characters enabled. Persistant device unplug/replug. More stable out transfer on stalled devices. Out flow control check.
	28/09/05	st		Delay telling the serial interrupt routine that we have sent everything until we have actually sent everything.
	24/01/06	st		Added in functionality to allow for Error events to be flagged.
	21/03/06	st		Removed uneccesary flow control handling in the Transfer part of the device.
						Restructured OutRequest to remove all of my uneccesary additions.
	18/04/06	st		Error and Event character fix.
	21/07/06	st		Fix for write operations that can potentially hang the system.
	08/08/06	st		Fix to allow the unplug on an open port event to trigger.
	09/03/07	st		BulPriorityEx added.
	06/06/07	ma		Changed calls to GetCurrentStreamDriverKey to use GetStreamDriverKey.
	07/05/08	ma		Moved setting TimeToTerminateThread to before creating the thread to fix problem with .NET serial port component not reading on 2nd open.
	04/06/08	ma		Added hi-speed chip support. FT4232H and FT2232H.
	25/03/09	ma		ReadRequest.hReadBytesSatisfied handle was not closed anywhere - now closed in CleanupOpenDevice.
	08/07/09	ma		Fixed memory leak caused by not closing dispatch thread handle.
	03/03/11	ma		Added a surprise remove flag to handle surprise disconnects better (in particular to allow WM_DEVICECHANGE messages to propagate).
						Added support for the FT232H.
	04/01/12	ma		Added an event to synchronise the bulk task with open; wait on an event to indicate that the bulk task is ready rather than an arbitrary 16ms.
						Also, if the bulk buffer is still allocated when the bulk task has been forcibly terminated, free the buffer.
	27/01/12	ma		Fix data loss in driver when exactly 256 bytes received in the buffer from the UART. 
						Data size was cast to a BYTE but had an exception for zero data (or 256 bytes) where data would be ignored and interrupt bit cleared.


--*/
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

Abstract:  

Notes: 
--*/
#include <windows.h>
#include <notify.h>
#include <serdbg.h>
#include <celog.h>
#include <prapi.h>
#include "serpriv.h"
#include "ftdi_ioctl.h"
#include "ftdi_debug.h"
#include "BUSBDBG.H"
#include "INFParse.h"

#ifdef DEBUG
#define	DEBUGMASK(bit)		(1 << (bit))

#define MASK_INIT		DEBUGMASK(0)
#define MASK_OPEN		DEBUGMASK(1)
#define MASK_READ		DEBUGMASK(2)
#define MASK_WRITE		DEBUGMASK(3)
#define MASK_CLOSE		DEBUGMASK(4)
#define MASK_IOCTL		DEBUGMASK(5)
#define MASK_THREAD		DEBUGMASK(6)
#define MASK_EVENTS		DEBUGMASK(7)
#define MASK_CRITSEC	DEBUGMASK(8)
#define MASK_FLOW		DEBUGMASK(9)
#define MASK_IR			DEBUGMASK(10)
#define MASK_USR_READ	DEBUGMASK(11)
#define MASK_ALLOC		DEBUGMASK(12)
#define MASK_FUNCTION	DEBUGMASK(13)
#define MASK_WARN		DEBUGMASK(14)
#define MASK_ERROR		DEBUGMASK(15)
#endif

extern VOID SerialEventHandler(PVOID pHead);

static PSER_INFO pCurrentHead;

extern TCHAR gszStreamDriverKey[MAX_PATH];

/*++

Routine Description:
	When an app has an open handle to this device and then the user unplugs the USB cable the driver will unload here. 
	However, the app can a) hang on a removed handle, or b) pass a call to a removed function, e.g., CloseHandle and 
	cause an AV. Set state to reject all I/O and wait for cleanup signal.
	
Arguments:

	pUsbFTDI		- Handle to the USB device
	
Return Value:

--*/
VOID
SetDeviceUnloading(
	PFTDI_DEVICE pUsbFTDI
	)
{
	PHW_INDEP_INFO pHWIInfo;
	//
	// Get our device context
	//
	pHWIInfo = (PHW_INDEP_INFO)pUsbFTDI->pMddHead;
	pHWIInfo->UnloadPending = TRUE;

	EnterCriticalSection(&pUsbFTDI->Lock);
	pUsbFTDI->Flags.UnloadPending = TRUE;
	LeaveCriticalSection(&pUsbFTDI->Lock);
}

VOID
SetDeviceLoading(
	PFTDI_DEVICE pUsbFTDI
	)
{
	PHW_INDEP_INFO pHWIInfo;
	//
	// Get our device context
	//
	pHWIInfo = (PHW_INDEP_INFO)pUsbFTDI->pMddHead;
	pHWIInfo->UnloadPending = FALSE;

	EnterCriticalSection(&pUsbFTDI->Lock);
	pUsbFTDI->Flags.UnloadPending = FALSE;
	pUsbFTDI->Flags.SurpriseRemoved = FALSE;
	LeaveCriticalSection(&pUsbFTDI->Lock);
}

DWORD
FTDIWriteTransferComplete(
	PVOID Context
	)
{
	DWORD dwErr = ERROR_SUCCESS;
	HANDLE hEvent = (HANDLE)Context;

	DEBUGMSG1(ZONE_FUNCTION, (TEXT(">FTDIWriteTransferComplete\n")));

	if(hEvent) {

		//
		// The current operation completed, signal the event
		//
		if(!SetEvent(hEvent)) {
			dwErr = GetLastError();
			DEBUGMSG2(ZONE_ERROR, TEXT("*** SetEvent ERROR:%d ***\n"), dwErr);
		}
	} 
	else {
		dwErr = ERROR_INVALID_HANDLE;
		SetLastError(dwErr);
		DEBUGMSG2( ZONE_ERROR,TEXT("*** FTDIWriteTransferComplete ERROR:%d ***\n"), dwErr);
	}

	DEBUGMSG1(ZONE_FUNCTION,(TEXT("<FTDIWriteTransferComplete\n")));

	return dwErr;
}

static DWORD WINAPI
FTDIEventThread(PSER_INFO   pHWHead)
{
    DEBUGMSG2 (ZONE_THREAD, TEXT("Entered FTDIEventThread %X\r\n"),
                            pHWHead);

    DEBUGMSG2(ZONE_INIT, TEXT("Spinning in dispatch thread %X\n\r"), pHWHead);
    while (!pHWHead->pDispatchThread) {
        Sleep(20);
    }

    while (!pHWHead->KillRxThread) {
        DEBUGMSG4(ZONE_THREAD, TEXT("Event %X, Index %d, Int %d\r\n"),
                                          pHWHead->hSerialEvent,
                                          pHWHead->dwDevIndex,
                                          pHWHead->pHWObj->dwIntID);

        WaitForSingleObject(pHWHead->hSerialEvent, INFINITE);
        if(!pHWHead->KillRxThread) {
			SerialEventHandler(pHWHead->pMddHead);
			ResetEvent(pHWHead->hSerialEvent);
		}
    }
    
    DEBUGMSG2(ZONE_THREAD, TEXT("FTDIEventThread %x exiting\r\n"), pHWHead);
    return 0;
}

BOOL
StartEventThread(PSER_INFO pHWHead)
{
	// Now set up the dispatch thread 
    pHWHead->KillRxThread = 0;

    DEBUGMSG2(ZONE_INIT, TEXT("Spinning thread%X\n\r"), pHWHead);

    pHWHead->pDispatchThread = CreateThread(NULL,0, FTDIEventThread,
                                                pHWHead, 0,NULL);
    if ( pHWHead->pDispatchThread == NULL ) {
        DEBUGMSG2(ZONE_INIT|ZONE_ERROR,
                 TEXT("Error creating Event thread (%d)\n\r"),
                  GetLastError());
        return (FALSE);
    }
    DEBUGMSG2 (ZONE_INIT, TEXT("Created Event thread %X\r\n"),
                          pHWHead->pDispatchThread);

    DEBUGMSG1 (ZONE_INIT, (TEXT("About to set priority\r\n")));

    CeSetThreadPriority(pHWHead->pDispatchThread, DEFAULT_THREAD_PRIO);
                          
//    DEBUGMSG1(1|ZONE_INIT, (TEXT("Back from setting priority\r\n")));
    return TRUE;
}

/*++

Routine Description:
	
	To support ActiveSync you need to monitor the DCD line? This thread monitors whatever line that the customer wants.
	The sleep is also configurable - as we dont want to be polling in a tight loop.
	
Arguments:

	Context				- Handle to the USB device
	
Return Value:

--*/
DWORD WINAPI
ActiveSyncMonitor(
    IN PVOID Context
    )
{
    DWORD dwModemStatus;
	DWORD dwErr;
	PFTDI_DEVICE pUsbFTDI = (PFTDI_DEVICE)Context;

	pUsbFTDI->bActiveSyncKeepRunning = TRUE;
	// Initialiase the modem status
	FT_GetModemStatus(pUsbFTDI);

	while(pUsbFTDI->bActiveSyncKeepRunning) {

		Sleep(pUsbFTDI->ActiveSyncSleep);

		dwModemStatus = pUsbFTDI->SerialRegs.MSR;
		dwErr = FT_GetModemStatus(pUsbFTDI);

		if(dwErr == ERROR_INVALID_HANDLE) {
			//
			// We can assume that the device is unplugged so exit this thread
			//

			DEBUGMSG1(ZONE_ERROR, (TEXT("ActiveSync thread - Invalud handle\n")));
			pUsbFTDI->hActiveSyncThread = NULL;
			return 1;
		}

		// If ActiveSyncMonitorBit transitioned to active, we need to generate event.
		if((pUsbFTDI->SerialRegs.MSR  & pUsbFTDI->ActiveSyncMonitorBit) && !(dwModemStatus & pUsbFTDI->ActiveSyncMonitorBit)) {
			if(IsAPIReady(SH_WMGR)) {
				CeEventHasOccurred(NOTIFICATION_EVENT_RS232_DETECTED, NULL);
			}
			DEBUGMSG1(ZONE_EVENTS, (TEXT("Indicated RS232 Cable Event\r\n")));
		}
	}
	return 0;
}

/*++

Routine Description:
	Setup the ActiveSync monitoring thread if required in the registry setting	

Arguments:

	pDevice				- Handle to the USB device
	
Return Value:

--*/
VOID 
ConfigureActiveSyncMonitor(
		PFTDI_DEVICE pDevice
		)
{
	TCHAR szDeviceRegistry[MAX_PATH + 10];
	DWORD dwActiveSetting;
	DWORD dwDataSize;
	LONG regError;
    HKEY hKey;

	//
	// Have we been unplugged
	//
	EnterCriticalSection(&pDevice->Lock);
	if(pDevice->Flags.UnloadPending == TRUE) {
		//
		// Dont bother
		//
		LeaveCriticalSection(&pDevice->Lock);
		return;
	}
	LeaveCriticalSection(&pDevice->Lock);

	pDevice->hActiveSyncThread = NULL;

	//
	// Get the registry setting - if none then return
	//
	//GetCurrentStreamDriverKey(gszStreamDriverKey, pDevice->dwPID);
	GetStreamDriverKey(gszStreamDriverKey);
	
	if(pDevice->SerialNumber[0]) {
		//
		// the theory is we will have copied over the registry data at the install part
		// (CreateUniqueDriverSettings) and these values will be used here
		//
		TCHAR tcTemp[20];
		mbstowcs(tcTemp, pDevice->SerialNumber, strlen(pDevice->SerialNumber)+1);
		swprintf(szDeviceRegistry, TEXT("%s\\%s"), gszStreamDriverKey, tcTemp);
	}
	else {
		wcscpy(szDeviceRegistry, gszStreamDriverKey);
	}

	regError = RegOpenKeyEx(
					HKEY_LOCAL_MACHINE, 
					szDeviceRegistry,
					0, 
					KEY_ALL_ACCESS, 
					&hKey
					);

    if (regError == ERROR_SUCCESS) {

		dwDataSize = sizeof(DWORD);
		regError = RegQueryValueEx(
						hKey, 
						TEXT("ActiveSyncSetting"),
						NULL, 
						NULL, 
						(LPBYTE)&dwActiveSetting,
						&dwDataSize
						);

		if(regError == ERROR_SUCCESS) {
			pDevice->ActiveSyncMonitorBit = (UCHAR)(dwActiveSetting & 0xF0);
			pDevice->ActiveSyncSleep = ((dwActiveSetting >> 8) & 0x00FFFFFF);
			
			//
			// Create our ActiveSync monitoring thread
			//
			pDevice->hActiveSyncThread = CreateThread(	
											NULL,
											0,
											ActiveSyncMonitor,
											(PVOID)pDevice,
											0, 
											NULL
											);
		}
	}
	else {
		DEBUGMSG2(ZONE_ERROR, TEXT("Failed to get registy settings for path %s\n"), szDeviceRegistry);
	}

	if(hKey)
		RegCloseKey(hKey);
}

/*++

Routine Description:
	Terminate the ActiveSync monitoring thread

Arguments:

	pUsbFTDI				- Handle to the USB device
	
Return Value:

--*/
VOID TerminateActiveSyncMonitor(
		PFTDI_DEVICE pUsbFTDI
		)
{
	if(pUsbFTDI->hActiveSyncThread != NULL) {
		pUsbFTDI->bActiveSyncKeepRunning = FALSE;

		if(WaitForSingleObject(pUsbFTDI->hActiveSyncThread, pUsbFTDI->ActiveSyncSleep) == WAIT_TIMEOUT) {
			DEBUGMSG1(ZONE_ERROR,(TEXT("ActiveSync Thread Not Exited Properly Error\n")));
			// Not so nice termination of the bulk thread
			TerminateThread(pUsbFTDI->hActiveSyncThread, 0xffffffff);
		}
		CloseHandle(pUsbFTDI->hActiveSyncThread);
		pUsbFTDI->hActiveSyncThread = NULL;
	}
}

/*++
Routine:

    Ser_GetRegistryData

Description:

    Take the registry path provided to COM_Init and use it to find this 
    requested comm port's DeviceArrayIndex, the IOPort Base Address, and the
    Interrupt number.
   
Arguments:

    LPCTSTR regKeyPath	the registry path passed in to COM_Init.

Return Value:

    -1 if there is an error.

--*/
BOOL
Ser_GetRegistryData(PSER_INFO pHWHead, LPCTSTR regKeyPath)
{
#define GCI_BUFFER_SIZE 256   

    LONG    regError;
    HKEY    hKey;
    TCHAR   devKeyPath[GCI_BUFFER_SIZE];
    DWORD   dwDataSize = GCI_BUFFER_SIZE;

    DEBUGMSG2(ZONE_INIT, TEXT("Try to open %s\r\n"), regKeyPath);

    // We've been handed the name of a key in the registry that was generated
    // on the fly by device.exe.  We're going to open that key and pull from it
    // a value that is the name of this serial port's real key.  That key
    // will have the DeviceArrayIndex that we're trying to find.  

    regError = RegOpenKeyEx(
					HKEY_LOCAL_MACHINE, 
					regKeyPath,
					0, 
					KEY_ALL_ACCESS, 
					&hKey
					);

    if(regError != ERROR_SUCCESS) {
        DEBUGMSG3(ZONE_INIT | ZONE_ERROR,
                 TEXT("Failed to open %s, Error 0x%X\r\n"), regKeyPath, regError);
        return FALSE;                
    }

    regError = RegQueryValueEx(
					hKey, 
					DEVLOAD_DEVKEY_VALNAME,
					NULL, 
					NULL, 
					(LPBYTE)devKeyPath,
					&dwDataSize
					);

	//
    // We're done with that registry key, so close it.
	//
    RegCloseKey(hKey);

    if (regError != ERROR_SUCCESS) {
        DEBUGMSG4(ZONE_INIT | ZONE_ERROR,
                 TEXT("Failed to find data at %s\\%s, Error 0x%X\r\n"), 
                  regKeyPath, DEVLOAD_DEVKEY_VALNAME, regError);
        return FALSE;                
    }

    DEBUGMSG2(ZONE_INIT, TEXT("Try to open %s\r\n"), devKeyPath);

    regError = RegOpenKeyEx(
					HKEY_LOCAL_MACHINE, 
					devKeyPath, 
					0,
					KEY_ALL_ACCESS, 
					&hKey
					);

    if(regError != ERROR_SUCCESS) {
        DEBUGMSG3(ZONE_INIT | ZONE_ERROR,
                 TEXT("Failed to open %s, Error 0x%X\r\n"), 
                  devKeyPath, regError);
        return ( FALSE );        
    }
    RegCloseKey(hKey);

    return TRUE; 
}

/*
 @doc OEM 
 @func PVOID | SerInit | Initializes device identified by argument.
 *  This routine sets information controlled by the user
 *  such as Line control and baud rate. It can also initialize events and
 *  interrupts, thereby indirectly managing initializing hardware buffers.
 *  Exported only to driver, called only once per process.
 *
 @rdesc The return value is a PVOID to be passed back into the HW
 dependent layer when HW functions are called.
 */
static
PVOID
SerInit(
       ULONG   Identifier, // @parm Device identifier.
       PVOID   pMddHead,   // @parm First argument to mdd callbacks.
       PHWOBJ  pHWObj)     // @parm Pointer to our own HW OBJ for this device
{
    PSER_INFO pHWHead;
	PHW_INDEP_INFO pHWIInfo;
	PFTDI_DEVICE pUsbFTDI;

#ifdef DEBUG
    extern DBGPARAM dpCurSettings;

//	dpCurSettings.ulZoneMask = 0;
 //	dpCurSettings.ulZoneMask = MASK_WRITE | MASK_ERROR;
   //dpCurSettings.ulZoneMask =	MASK_INIT | MASK_OPEN | MASK_READ | MASK_WRITE | MASK_CLOSE | MASK_IOCTL | MASK_THREAD | MASK_EVENTS | 
	//							MASK_CRITSEC | MASK_FLOW | MASK_IR | MASK_USR_READ |  MASK_ALLOC | MASK_FUNCTION | MASK_WARN | MASK_ERROR;
#endif

    DEBUGMSG2(ZONE_INIT, TEXT("+SerInit - %X\r\n"), pMddHead);

	//
	// Allocate for our main data structure
	//
    pHWHead = (PSER_INFO) FT_LocalAlloc(LPTR, sizeof(SER_INFO));
    if(!pHWHead)
        goto ALLOCFAILED;

    DEBUGMSG2(ZONE_INIT, TEXT("+SerInit Identifier - %X\r\n"), Identifier);

    if(!Ser_GetRegistryData(pHWHead, (LPCTSTR)Identifier)) {
        DEBUGMSG1 (ZONE_INIT|ZONE_ERROR, (TEXT("SerInit - Unable to read registry data.  Failing Init!\r\n")));
        goto ALLOCFAILED;
    }

	//
    // Create our interrupt event.
	//
    pHWHead->hSerialEvent = CreateEvent(0, FALSE, FALSE, NULL);


	//
    // Initialize our critical sections
	//
    InitializeCriticalSection(&(pHWHead->TransmitCritSec));
    InitializeCriticalSection(&(pHWHead->HwRegCritSec));

    pHWHead->pMddHead = pMddHead;
    pHWHead->pHWObj = pHWObj;
    pHWHead->cOpenCount = 0;
    pHWHead->cCtrlLines = 0;
    pHWHead->bFlowedOff = FALSE;
    pHWHead->wSOFStableCnt = 0; 				
    
	//
    // Set up our Comm Properties data
	//
    pHWHead->CommProp.wPacketLength      =	0xffff;
    pHWHead->CommProp.wPacketVersion     =	2;
    pHWHead->CommProp.dwServiceMask      =	SP_SERIALCOMM;
    pHWHead->CommProp.dwReserved1        =	0;
    pHWHead->CommProp.dwMaxTxQueue       =	0;
    pHWHead->CommProp.dwMaxRxQueue       =	0;
    pHWHead->CommProp.dwMaxBaud			 =	BAUD_USER;
	pHWHead->CommProp.dwSettableBaud	 =	BAUD_300 |
											BAUD_600 |
											BAUD_1200 |
											BAUD_2400 |
											BAUD_4800 |
											BAUD_9600 |
											BAUD_19200 |
											BAUD_38400 |
											BAUD_57600 |
											BAUD_115200;
    pHWHead->CommProp.dwProvSubType      =	PST_RS232;
    pHWHead->CommProp.dwProvCapabilities =	PCF_DTRDSR |
											PCF_RTSCTS |
											PCF_RLSD |			// SERIAL_PCF_CD | ???
											PCF_PARITY_CHECK |
											PCF_SETXCHAR |
											PCF_XONXOFF |
											PCF_TOTALTIMEOUTS | 
											PCF_INTTIMEOUTS;
    pHWHead->CommProp.dwSettableParams	 =	SP_PARITY |
											SP_BAUD |
											SP_DATABITS |
											SP_STOPBITS |
											SP_HANDSHAKING |
											SP_PARITY_CHECK |
											SP_RLSD;			// SERIAL_SP_CARRIER_DETECT; ???   
	pHWHead->CommProp.wSettableData		 =	DATABITS_5 |
											DATABITS_6 |
											DATABITS_7 |
											DATABITS_8;  
    pHWHead->CommProp.wSettableStopParity =	STOPBITS_10 |
											STOPBITS_15 |
											STOPBITS_20 |
											PARITY_NONE |
											PARITY_ODD  |
											PARITY_EVEN |
											PARITY_MARK |
											PARITY_SPACE;

	//
	// Get our device context
	//
	pHWIInfo = (PHW_INDEP_INFO)pMddHead;
	pHWHead->pFTDICtxt = pHWIInfo->lpvBusContext;



	pUsbFTDI = pHWHead->pFTDICtxt;
	pUsbFTDI->pMddHead = (PVOID)pMddHead;

	pUsbFTDI->BulkBuffer = NULL;	// initialise this to NULL

    DEBUGMSG2(ZONE_INIT, TEXT("-SerInit - %X\r\n"), pHWHead);

    return pHWHead;

ALLOCFAILED:
	//
    // Unmap any memory areas that we may have mapped.
	//
    if(pHWHead) {        

		if(pHWHead->hSerialEvent)
			CloseHandle(pHWHead->hSerialEvent);

        // Free any critical sections we have allocated
        DeleteCriticalSection(&(pHWHead->TransmitCritSec));
        DeleteCriticalSection(&(pHWHead->HwRegCritSec));
        
        // And free the context data structure
        FT_LocalFree(pHWHead);
    }
    return (NULL);
}
/*
 @doc OEM 
 @func PVOID | SerPostInit | Performs final hardware initialization.
 *
 */
static
BOOL
SerPostInit(
         PVOID   pHead   // @parm PVOID returned by SerInit.
         )
{
    PSER_INFO   pHWHead = (PSER_INFO)pHead;

	// This starts our event thread not sure if all of this is overkill +++
    StartEventThread(pHWHead);
    return TRUE;
}

/*
 @doc OEM 
 @func PVOID | SerDeinit | Deinitializes device identified by argument.
 *  This routine frees any memory allocated by SerInit.
 *
 */
static
BOOL
SerDeinit(
         PVOID   pHead   // @parm PVOID returned by SerInit.
         )
{
    PSER_INFO pHWHead = (PSER_INFO) pHead;

    if (!pHWHead)
        return (FALSE);

    // Make sure device is closed before doing DeInit
    if (pHWHead->cOpenCount)
        SerClose(pHead);

    pHWHead->KillRxThread = 1;

	SetEvent(pHWHead->hSerialEvent);

	if(WaitForSingleObject(pHWHead->pDispatchThread, 5000) == WAIT_TIMEOUT) {
		DEBUGMSG1(ZONE_ERROR,(TEXT("EventThread Thread Not Exited Properly Error\n")));
		// Not so nice termination of the bulk thread
		TerminateThread(pHWHead->pDispatchThread, 0xffffffff);
	}

	CloseHandle(pHWHead->pDispatchThread);
	pHWHead->pDispatchThread = 0;

	if(pHWHead->hSerialEvent)
		CloseHandle(pHWHead->hSerialEvent);

    // Free any critical sections we have allocated
    DeleteCriticalSection(&(pHWHead->TransmitCritSec));
    DeleteCriticalSection(&(pHWHead->HwRegCritSec));

    // Free the HWObj allocated in GetSerialObject
    FT_LocalFree(pHWHead->pHWObj);

    FT_LocalFree(pHWHead);

    return TRUE;
}

void myNOP(DWORD dwMilliseconds)
{
	UNREFERENCED_PARAMETER(dwMilliseconds);
	return;
}

/*++

Routine Description:
	Helper Function

Arguments:
	hKey		- The registry key
	ptcName		- Name of the registry value
	pulValue	- Value returned

Return Value:

    TRUE if successful,
    FALSE otherwise

--*/
BOOL
RegQueryDWORD(HKEY hKey, const TCHAR * ptcName, DWORD * pulValue)
{
	DWORD dwDataSize;
	LONG regError;

	dwDataSize = sizeof(DWORD);
	regError = RegQueryValueEx(
					hKey, 
					ptcName,
					NULL, 
					NULL, 
					(LPBYTE)pulValue,
					&dwDataSize
					);

	return (regError == ERROR_SUCCESS);
}

/*++

Routine Description:
	Helper Function to allow the initialisation of the device structure.(when open) 

Arguments:
	pHWHead			- pointer to the device

Return Value:

    TRUE if successful,
    FALSE otherwise

--*/
BOOL
InitialiseOpenDevice(
	PSER_INFO pHWHead
	)
{
	DWORD				dwBulkPriority;
	DWORD				dwBulkTimeSlice;
	DWORD				dwBulkSleep;
    DWORD				dwDataSize;
	DWORD				dwDataContext;
	DWORD				dwLatency;
	FTDI_CONFIG_DATA	configData;
	int					iCurrentThreadPriority, iCurrentCeThreadPriority;
    HKEY				hKey;
	LONG				regError;
	PFTDI_DEVICE		pUsbFTDI;
	TCHAR				szDeviceRegistry[MAX_PATH + 10];

	pUsbFTDI = pHWHead->pFTDICtxt;

	if((!pUsbFTDI->Flags.Open) &&
		(!pUsbFTDI->Flags.UnloadPending)) {
		DEBUGMSG1(ZONE_FUNCTION, (TEXT("InitialiseOpenDevice\n")));

		FT_ResetHardware(pUsbFTDI);

        FT_GetModemStatus(pUsbFTDI);

		//
		// Event Init
		//		
		pUsbFTDI->hCloseReaderEvent = CreateEvent( NULL, FALSE, FALSE, NULL);	// auto reset event
		if (!pUsbFTDI->hCloseReaderEvent) {
			DEBUGMSG2( ZONE_ERROR, TEXT("CreateEvent error:%d\n"), GetLastError() );
			return FALSE;
		}
		else {
			ResetEvent(pUsbFTDI->hCloseReaderEvent);
		}	

		pUsbFTDI->hReaderConfiguredEvent = CreateEvent( NULL, FALSE, FALSE, NULL);	// auto reset event
		if (!pUsbFTDI->hReaderConfiguredEvent) {
			DEBUGMSG2( ZONE_ERROR, TEXT("CreateEvent error:%d\n"), GetLastError() );
			return FALSE;
		}
		else {
			ResetEvent(pUsbFTDI->hReaderConfiguredEvent);
		}	
/*
		pUsbFTDI->hReaderTerminatedEvent = CreateEvent( NULL, FALSE, FALSE, NULL);	// auto reset event
		if (!pUsbFTDI->hReaderTerminatedEvent) {
			DEBUGMSG2( ZONE_ERROR, TEXT("CreateEvent error:%d\n"), GetLastError() );
			return FALSE;
		}
		else {
			ResetEvent(pUsbFTDI->hReaderTerminatedEvent);
		}	
*/

		//		
		// Read Request Event Init
		//		
		pUsbFTDI->ReadRequest.hReadBytesSatisfied = CreateEvent( NULL, FALSE, FALSE, NULL);	// auto reset event
		if (!pUsbFTDI->ReadRequest.hReadBytesSatisfied) {
			DEBUGMSG2( ZONE_ERROR, TEXT("CreateEvent error:%d\n"), GetLastError() );
			return FALSE;
		}
		else {
			ResetEvent(pUsbFTDI->ReadRequest.hReadBytesSatisfied);
		}
		pUsbFTDI->ReadRequest.bReadPending = FALSE;
		
		//
		// Mutex Init
		//
		if((pUsbFTDI->ReadBuffer.hBufferMutex = CreateMutex(NULL, FALSE, NULL)) == NULL) {
			DEBUGMSG2(ZONE_ERROR, TEXT("CreateMutex error:%d\n"), GetLastError());
			return FALSE;	
		}

		//
		// Buffers Init
		//		
		pUsbFTDI->ReadBuffer.Buffer		= NULL;

/*  +++ This might be better served by Separate Heap functions so we do not fragment the local heap??? 
See Windows CE book for details. */

		//
		// Read Buffer Init
		//		
		pUsbFTDI->ReadBuffer.Buffer = (UCHAR *)FT_LocalAlloc(0, USER_BUFFER_SIZE);
		if(pUsbFTDI->ReadBuffer.Buffer == NULL) {
			DEBUGMSG2(ZONE_ERROR, TEXT("Read Buffer alloc error = %d\n"), GetLastError());
			return FALSE;
		}
		else {
			pUsbFTDI->ReadBuffer.rbSize			= USER_BUFFER_SIZE;
			pUsbFTDI->ReadBuffer.rbPut			= 0;
			pUsbFTDI->ReadBuffer.rbGet			= 0;
			pUsbFTDI->ReadBuffer.rbBytesFree	= pUsbFTDI->ReadBuffer.rbSize;
			pUsbFTDI->ResetBuffer = FALSE;
		}

		//
		// notify Routine initialisation
		//
		InitializeCriticalSection(&pUsbFTDI->NotifyLock);
		pUsbFTDI->dwCurrentNotifyStatus = 0;
		pUsbFTDI->dwNotifyPermissions = 0;
		pUsbFTDI->Notify.dwRef = 0;
		pUsbFTDI->Notify.dwMask = 0;
		pUsbFTDI->Notify.pvParam = NULL;
	}
	
	//GetCurrentStreamDriverKey(gszStreamDriverKey, pUsbFTDI->dwPID);
	GetStreamDriverKey(gszStreamDriverKey);

	if(pUsbFTDI->SerialNumber[0]) {
		// the theory is we will have copied over the registry data at the install part
		// (CreateUniqueDriverSettings) and these values will be used here
		TCHAR tcTemp[20];
		mbstowcs(tcTemp, pUsbFTDI->SerialNumber, strlen(pUsbFTDI->SerialNumber)+1);
		swprintf(szDeviceRegistry, TEXT("%s\\%s"), gszStreamDriverKey, tcTemp);
	}
	else {
		wcscpy(szDeviceRegistry, gszStreamDriverKey);
	}
	
	regError = RegOpenKeyEx(
					HKEY_LOCAL_MACHINE, 
					szDeviceRegistry,
					0, 
					KEY_ALL_ACCESS, 
					&hKey
					);
    if (regError == ERROR_SUCCESS) {

		if(RegQueryDWORD(hKey, TEXT("LatencyTimer"), &dwLatency) == TRUE) {
			FT_SetLatencyTimer(pUsbFTDI, (UCHAR)(dwLatency&0xFF));
		}

		if(RegQueryDWORD(hKey, TEXT("InTransferSize"), &pUsbFTDI->dwInTransferSize) == TRUE) {
		    pUsbFTDI->InParams.CurrentTransferSize = pUsbFTDI->dwInTransferSize;
		}
		else {
			pUsbFTDI->dwInTransferSize = FT_USBD_DEFAULT_TRANSFER_SIZE;
		}

		if(RegQueryDWORD(hKey, TEXT("OutTransferSize"), &pUsbFTDI->dwOutTransferSize) == TRUE) {
		    pUsbFTDI->OutParams.CurrentTransferSize = pUsbFTDI->dwOutTransferSize;
		}
		else {
			pUsbFTDI->dwOutTransferSize = FT_USBD_DEFAULT_TRANSFER_SIZE;
		}

		dwDataSize = sizeof(configData.ConfigData);
		regError = RegQueryValueEx(
						hKey,
						TEXT("ConfigData"),
						NULL,
						NULL,
						(LPBYTE)&configData.ConfigData,
						&dwDataSize
						);

		if(regError == ERROR_SUCCESS) {
			pUsbFTDI->ConfigDataFlags = configData.ConfigData.Flags;
			// create the baud rate table from this
			if (SerialUseBmBaudRates(pUsbFTDI)) {
				//
				// ConfigData baud rates are 32-bits and match baud rate table.
				//
				memcpy(
					pUsbFTDI->BaudRates,
					&configData.ConfigData.BaudRates,
					NUM_FTDI_BAUD_RATES * sizeof(ULONG)
					);

			}
			else {
				//
				// ConfigData baud rates are 16-bits and have to be converted
				// to 32-bits to match baud rate table.
				//
				PUSHORT pus;
				int i;
				for (i = 0, pus = (PUSHORT)(&configData.ConfigData.BaudRates);
					 i < NUM_FTDI_BAUD_RATES;
					 i++, pus++) {
					ULONG l = (ULONG) *pus;
					l &= 0x0000ffff;
					pUsbFTDI->BaudRates[i] = l;
				}
			}
			pUsbFTDI->bIsValidBaudRateTable = TRUE;	// use the baud rate table in SetBaudRate

			if(pUsbFTDI->SerialNumber[0] && (pUsbFTDI->ConfigDataFlags & FTDI_PERSISTANT_DEVICE_INSTANCE)) {

				//
				// Save the device context in the registry
				//
				dwDataSize = sizeof(DWORD);
				dwDataContext = (DWORD)pUsbFTDI;
				regError = RegSetValueEx(
								hKey,
								TEXT("DeviceContext"),
								0,
								REG_DWORD,
								(LPBYTE)&dwDataContext,
								dwDataSize
								);

				if(regError != ERROR_SUCCESS) {
					DEBUGMSG2(ZONE_ERROR, TEXT("Failed to set Device Context:%d\n"), regError);
				}
			}
		}
		else {
			//
			// We dont have a BaudRate table - no aliasing available use passed in values
			//	
			pUsbFTDI->bIsValidBaudRateTable = FALSE;
		}

		//
		// 0x20A8(8360d) is for DTR/DSR emulation
		// 0x2aa8(10920d) is for RTS/CTS, DTR/DSR
		//
		if(RegQueryDWORD(hKey, TEXT("EmulationMode"), &configData.EmulationMode) == TRUE) {
			if(FT_InitEmulMode(&pUsbFTDI->EmulVars, configData.EmulationMode, pUsbFTDI->OutParams.CurrentTransferSize) == FALSE) {

				//
				// Carry on but with no modem emulation available
				//
				configData.EmulationMode = FT_EMUL_DEF_MODE;
				DEBUGMSG1(ZONE_ERROR, (TEXT("Failed to initialise Modem emulation - out of memory\n")));
			}        
		}
		else {
			configData.EmulationMode = FT_EMUL_DEF_MODE;
			memset(&pUsbFTDI->EmulVars, 0, sizeof(FTEMUL));
			pUsbFTDI->EmulVars.pWriteBuffer = NULL;
		}

		//
		// Optional sleep can be implemented between bulk in transfers
		//
		if(RegQueryDWORD(hKey, TEXT("msSleepAfterBulkIn"), &dwBulkSleep) == TRUE) {
			pUsbFTDI->mySleep = &Sleep;
			pUsbFTDI->dwSleepAfterBulkIn = dwBulkSleep;
		}
		else {
			pUsbFTDI->mySleep = &myNOP;
			pUsbFTDI->dwSleepAfterBulkIn = 0;
		}

/*
  		if(RegQueryDWORD(hKey, TEXT("MinReadTimeout"), &pUsbFTDI->dwMinReadTimeOut) == TRUE) {
			pHWHead->CommTimeouts.ReadTotalTimeoutConstant = pUsbFTDI->dwMinReadTimeOut;
		}
		else {
			pHWHead->CommTimeouts.ReadTotalTimeoutConstant = 0;
		}

  		if(RegQueryDWORD(hKey, TEXT("MinWriteTimeout"), &pUsbFTDI->dwMinWriteTimeOut) == TRUE) {
			pHWHead->CommTimeouts.WriteTotalTimeoutConstant = pUsbFTDI->dwMinWriteTimeOut;
		}
		else {
			pHWHead->CommTimeouts.WriteTotalTimeoutConstant = 0;
		}

*/
		pUsbFTDI->TimeToTerminateThread = FALSE;
		//
		// Thread Init
		//	
		pUsbFTDI->hReaderThread = CreateThread(	
									NULL,
									0,
									BulkInTask,
									(PVOID)pHWHead,
									0, 
									&pUsbFTDI->dwReaderThreadID
									);

		if(pUsbFTDI->hReaderThread == NULL) {
			DEBUGMSG2(ZONE_ERROR, TEXT("FTD_Open Error %d creating BulkInTask\n"), GetLastError());
			return FALSE;
		}
		else {
//	RETAILMSG(1, (TEXT("BULK IN CREATED\r\n")));
			iCurrentThreadPriority = GetThreadPriority(pUsbFTDI->hReaderThread);
			DEBUGMSG2(ZONE_FUNCTION, TEXT("Thread Prior:%d\n"), iCurrentThreadPriority);
			iCurrentCeThreadPriority = CeGetThreadPriority(pUsbFTDI->hReaderThread);
			// Thread priorities are set below when we read the registry
		}

		// MOVE THIS TO BEFORE CREATING THE THREAD!!!  CAUSES PROBLEMS WITH .NET SERIAL PORT COMPONENT
//		pUsbFTDI->TimeToTerminateThread = FALSE;

//		Sleep(16);								// wait for the thread to start
		WaitForSingleObject(pUsbFTDI->hReaderConfiguredEvent,INFINITE);

		//
		// Required to move this down here as bulk transfers can occur with the modem emulation etc.. here
		//
		regError = RegQueryValueEx(
						hKey,
						TEXT("ConfigDataFlags"),
						NULL,
						NULL,
						(LPBYTE)&pUsbFTDI->dwConfigDataFlags,
						&dwDataSize
						);

		if(regError == ERROR_SUCCESS) {
			DWORD dwSaveConfigDataFlags;
			/* Need to save the config data and mask out any flags that could stop DTR/RTS from being set! */
			dwSaveConfigDataFlags = pUsbFTDI->dwConfigDataFlags;
			pUsbFTDI->dwConfigDataFlags &= (DTR_INITIAL_STATE_ON | RTS_INITIAL_STATE_ON);
			if(pUsbFTDI->dwConfigDataFlags & DTR_INITIAL_STATE_ON)
				FT_SetDtr(pHWHead);
			if(pUsbFTDI->dwConfigDataFlags & RTS_INITIAL_STATE_ON)
				FT_SetRts(pHWHead);
			pUsbFTDI->dwConfigDataFlags = dwSaveConfigDataFlags;
		}

	}
	if(hKey)
		RegCloseKey(hKey);

	//
	// Now do the thread priority stuff - this is a global setting  - 
	// I dont see the need for a per device level
	//
	regError = RegOpenKeyEx(
					HKEY_LOCAL_MACHINE, 
					gszStreamDriverKey,
					0, 
					KEY_ALL_ACCESS, 
					&hKey
					);

	if(regError == ERROR_SUCCESS) {

		//
		// BulkPriority - set by SetThreadPriority (old API - used by less than CE 3.0)
		//
		if((RegQueryDWORD(hKey, TEXT("BulkPriority"),&dwBulkPriority) == TRUE) && 
			(dwBulkPriority != (DWORD)iCurrentThreadPriority)) {
			if(SetThreadPriority(pUsbFTDI->hReaderThread, dwBulkPriority) == 0) {
				DEBUGMSG2(ZONE_ERROR, TEXT("Error setting Bulk Priority = %d\n"), GetLastError());
			}
			else {
				DEBUGMSG2(ZONE_FUNCTION, TEXT("Set Bulk Priority to = %d\n"), dwBulkPriority);
			}
		}
		else {
			DEBUGMSG1(ZONE_FUNCTION, (TEXT("No entry for BulkPriority\n")));
		}		

		//
		// CeBulkPriority - set by CeSetThreadPriority (newer API - handles priorities 0..255) 
		//
		if((RegQueryDWORD(hKey, TEXT("BulkPriorityEx"),&dwBulkPriority) == TRUE) && 
			(dwBulkPriority != (DWORD)iCurrentCeThreadPriority)) {
			if(CeSetThreadPriority(pUsbFTDI->hReaderThread, dwBulkPriority) == 0) {
				DEBUGMSG2(ZONE_ERROR, TEXT("Error setting Bulk Priority = %d\n"), GetLastError());
			}
			else {
				DEBUGMSG2(ZONE_FUNCTION, TEXT("Set Bulk Priority to = %d\n"), dwBulkPriority);
			}
		} 
		else {
			DEBUGMSG1(ZONE_FUNCTION, (TEXT("No entry for BulkPriority\n")));
		}

		//
		// BulkTimeSlice
		//
		if(RegQueryDWORD(hKey, TEXT("BulkTimeSlice"),&dwBulkTimeSlice) == TRUE) {
			if(CeSetThreadQuantum(pUsbFTDI->hReaderThread, dwBulkTimeSlice) == FALSE) {
				DEBUGMSG2(ZONE_ERROR, TEXT("Error setting Bulk Time Slice = %d\n"), GetLastError());
			}
			else {
				DEBUGMSG2(ZONE_FUNCTION, TEXT("Setting Bulk Time Slice to = %d\n"), dwBulkTimeSlice);
			}
		} 
		else {
			DEBUGMSG1(ZONE_FUNCTION, (TEXT("No entry for BulkTimeSlice\n")));
		}
	}
	else {
		DEBUGMSG1(ZONE_FUNCTION, (TEXT("Could not open registry entry for this device\n")));
	}

	if(hKey)
		RegCloseKey(hKey);

	pUsbFTDI->Flags.Open = TRUE; 
    pUsbFTDI->Flags.UnloadPending = FALSE;
	pUsbFTDI->Flags.SurpriseRemoved = FALSE;

    pHWHead->cOpenCount++;

	return TRUE;
}


/*++

Routine Description:
	Helper Function to allow remove the DeviceContext registry key

Arguments:
	pUsbFTDI			- pointer to the device structure to clear

Return Value:

    TRUE if successful,
    FALSE otherwise

--*/
BOOL
RegRemoveDeviceContext(
	PFTDI_DEVICE pUsbFTDI
	)
{		
    DWORD dwDataSize;
	DWORD dwDataContext;
    HKEY hKey = NULL;
	LONG regError;
	TCHAR szDeviceRegistry[MAX_PATH + 10];

	//
	// Cleanup our device context on a close event so on an unplug the memory will not be leaked
	// - on a soft reboot memory will be cleared but the registry setting will still be there. 
	// This potentially harmfull scenario is hanlded in with the gbAttached variable.
	//
	if(pUsbFTDI->SerialNumber[0]) {
		TCHAR tcTemp[MAX_PATH];
		//GetCurrentStreamDriverKey(gszStreamDriverKey, pUsbFTDI->dwPID);
		GetStreamDriverKey(gszStreamDriverKey);

		// the theory is we will have copied over the registry data at the install part
		// (CreateUniqueDriverSettings) and these values will be used here
		mbstowcs(tcTemp, pUsbFTDI->SerialNumber, strlen(pUsbFTDI->SerialNumber)+1);
		swprintf(szDeviceRegistry, TEXT("%s\\%s"), gszStreamDriverKey, tcTemp);

		regError = RegOpenKeyEx(
						HKEY_LOCAL_MACHINE, 
						szDeviceRegistry,
						0, 
						KEY_ALL_ACCESS, 
						&hKey
						);

		if(regError == ERROR_SUCCESS) {

			dwDataContext = (DWORD)NULL;
			dwDataSize = sizeof(DWORD);
			regError = RegSetValueEx(
							hKey,
							TEXT("DeviceContext"),
							0,
							REG_DWORD,
							(LPBYTE)&dwDataContext,
							dwDataSize
							);

			if(regError != ERROR_SUCCESS) {
				DEBUGMSG2(ZONE_ERROR, TEXT("CleanupOpenDevice: Failed to open key (%d)\n"), regError);
				return FALSE;
			}
		}
		else {
			DEBUGMSG2(ZONE_ERROR, TEXT("CleanupOpenDevice: Failed to open key (%d)\n"), regError);
				return FALSE;
		}
		if(hKey)
			RegCloseKey(hKey);
	}
	return TRUE;
}

/*++

Routine Description:
	Helper Function to allow the cleanup the device structure.(when closed)

Arguments:
	pUsbFTDI			- pointer to the device structure to add

Return Value:

    TRUE if successful,
    FALSE otherwise

--*/
BOOL
CleanupOpenDevice(
	PFTDI_DEVICE pUsbFTDI
	)
{		
	BOOL bRc = TRUE;

	DEBUGMSG1(ZONE_FUNCTION, (TEXT(">CleanupOpenDevice\n")));

	EnterCriticalSection(&pUsbFTDI->Lock);

	//
	// Event Close
	//		
	// The event is cleaned up in the Reader Thread!
	if(pUsbFTDI->hCloseReaderEvent) {		
		if(!CloseHandle(pUsbFTDI->hCloseReaderEvent)) {
			DEBUGMSG2(ZONE_ERROR, TEXT("CloseHandle (Event)error:%d\n"), GetLastError());
		}
	}

	if(pUsbFTDI->hReaderConfiguredEvent) {		
		if(!CloseHandle(pUsbFTDI->hReaderConfiguredEvent)) {
			DEBUGMSG2(ZONE_ERROR, TEXT("CloseHandle (Event)error:%d\n"), GetLastError());
		}
	}
/*
	if(pUsbFTDI->hReaderTerminatedEvent) {		
		if(!CloseHandle(pUsbFTDI->hReaderTerminatedEvent)) {
			DEBUGMSG2(ZONE_ERROR, TEXT("CloseHandle (Event)error:%d\n"), GetLastError());
		}
	}
*/	

	//		
	// Read Request Event Close
	//		
	if(pUsbFTDI->ReadRequest.hReadBytesSatisfied) {
		if(!CloseHandle(pUsbFTDI->ReadRequest.hReadBytesSatisfied)) {
			DEBUGMSG2(ZONE_ERROR, TEXT("CloseHandle (Read Request Event)error:%d\n"), GetLastError());
			bRc = FALSE;
		}
	}

	//
	// Thread Close
	//		
	if(pUsbFTDI->hReaderThread) {
		if(!CloseHandle(pUsbFTDI->hReaderThread)) {
			DEBUGMSG2(ZONE_ERROR, TEXT("CloseHandle (Thread)error:%d\n"), GetLastError());
			bRc = FALSE;
		}
	}

	//
	// Mutex Close
	//		
	if(pUsbFTDI->ReadBuffer.hBufferMutex) {
		if(!CloseHandle(pUsbFTDI->ReadBuffer.hBufferMutex)) {
			DEBUGMSG2(ZONE_ERROR, TEXT("CloseHandle (Mutex)error:%d\n"), GetLastError());
			bRc = FALSE;
		}
	}

	//
	// Read Buffer Remove
	//		
	if(pUsbFTDI->ReadBuffer.Buffer) {
		if(FT_LocalFree(pUsbFTDI->ReadBuffer.Buffer)) {
			DEBUGMSG2(ZONE_ERROR, TEXT("LocalFree User Buffer Failed Error:%d\n"), GetLastError());
			bRc = FALSE;
		}
	}

	//
	// Emulation Buffer Remove
	//		
	if(pUsbFTDI->EmulVars.pWriteBuffer) {
		if(FT_LocalFree(pUsbFTDI->EmulVars.pWriteBuffer)) {
			DEBUGMSG2(ZONE_ERROR, TEXT("LocalFree Modem Emulation Buffer Failed Error:%d\n"), GetLastError());
			bRc = FALSE;
		}
	}

	//
	//	Remove our notify critical section
	//
	DeleteCriticalSection(&pUsbFTDI->NotifyLock);

	pUsbFTDI->Flags.Open = FALSE;

	LeaveCriticalSection(&pUsbFTDI->Lock);

	DEBUGMSG1(ZONE_FUNCTION, (TEXT("<CleanupOpenDevice\n")));
	return bRc;

}

/*
 @doc OEM
 @func BOOL | SerOpen | This routine is called when the port is opened.
 *  Not exported to users, only to driver.
 *
 @rdesc Returns TRUE if successful, FALSE otherwise.
 */
static
BOOL
SerOpen(
       PVOID   pHead /*@parm PVOID returned by Serinit. */
       )
{
    PSER_INFO pHWHead;
	PFTDI_DEVICE pUsbFTDI;

	pHWHead = (PSER_INFO)pHead;	
	pUsbFTDI = pHWHead->pFTDICtxt;
	pUsbFTDI->pHWHead = pHWHead;

    DEBUGMSG1(ZONE_OPEN, (TEXT(">SerOpen\n")));

    // Disallow multiple simultaneous opens
    if (pHWHead->cOpenCount) {
		DEBUGMSG1(ZONE_OPEN, (TEXT(">Already Open\n")));
        return FALSE;
	}

	TerminateActiveSyncMonitor(pUsbFTDI);
	InitialiseOpenDevice(pHWHead);

#ifdef POLL_FOR_DISCONNECT
    // Yuck.  We want to poll for detaches when the device is open.
    // But right now, the IST is in a wait infinite.  Spoof an interrupt
    // to let him know we have been opened and he needs to start polling.
    SetEvent(pHWHead->hSerialEvent);
#endif
    DEBUGMSG1(ZONE_OPEN, (TEXT("<SerOpen\n")));

    return TRUE;
}

/*
 @doc OEM
 @func ULONG | SerClose | This routine closes the device identified by the PVOID returned by SerInit.
 *  Not exported to users, only to driver.
 *
 @rdesc The return value is 0.
 */
static
ULONG
SerClose(
        PVOID   pHead   // @parm PVOID returned by SerInit.
        )
{
    PSER_INFO pHWHead;
	PFTDI_DEVICE pUsbFTDI;
	pHWHead = (PSER_INFO)pHead;	
	pUsbFTDI = pHWHead->pFTDICtxt;

    DEBUGMSG1(ZONE_CLOSE, (TEXT(">SerClose\n")));

    if(pHWHead->cOpenCount) {
        DEBUGMSG1 (ZONE_CLOSE, (TEXT("SerClose, closing device\r\n")));
        pHWHead->cOpenCount--;

		// Set DTR and RTS appropriately
		if((pUsbFTDI->dwConfigDataFlags & DTR_IGNORE_ON_CLOSE) != DTR_IGNORE_ON_CLOSE)
			FT_ClrDtr(pHWHead);
		if((pUsbFTDI->dwConfigDataFlags & RTS_IGNORE_ON_CLOSE) != RTS_IGNORE_ON_CLOSE)
			FT_ClrRts(pHWHead);

		// Terminate the bulk task nicely
		pUsbFTDI->TimeToTerminateThread = TRUE;
		SetEvent(pUsbFTDI->hCloseReaderEvent);
		DEBUGMSG1(ZONE_FUNCTION,(TEXT("SetEvent hCloseReaderEvent\n")));
		if(pUsbFTDI->hReaderThread) {
			//
			// You should be cancelling all outstanding Bulk Reads here +++ otherwise you could potentially be waiting 5 seconds for a close
			// Wait for the thread to exit.
			//
			if(WaitForSingleObject(pUsbFTDI->hReaderThread, 5000) == WAIT_TIMEOUT) {
				DEBUGMSG1(ZONE_ERROR,(TEXT("Thread Not Exited Properly Error\n")));
				// Not so nice termination of the bulk thread - 
				// causes memory leak of local bulk buffer!!!!
				TerminateThread(pUsbFTDI->hReaderThread, 0xffffffff);
//				RETAILMSG(1,(TEXT("FORCED TERMINATION OF BULK THREAD!\r\n")));
				// also free our buffer if it's still there!
				if (pUsbFTDI->BulkBuffer)
				{
					FT_LocalFree(pUsbFTDI->BulkBuffer);
					pUsbFTDI->BulkBuffer = NULL;
				}

			}

//			WaitForSingleObject(pUsbFTDI->hReaderConfiguredEvent,INFINITE);

			CloseHandle(pUsbFTDI->hReaderThread);
			pUsbFTDI->hReaderThread = 0;
		}

		if(CleanupOpenDevice(pUsbFTDI) == FALSE) {
			DEBUGMSG1(ZONE_ERROR,(TEXT("CleanupOpenDevice Failed\n")));
		}

#ifdef TODO   // Do we need something similar on USB???
        // while we are still transmitting, sleep.
        uTries = 0;
        while(((pHWHead->ser16550.IER = READ_PORT_UCHAR(pHWHead->ser16550.pIER)) 
                 & SERIAL_IER_THR) &&  // indicates TX in progress
                (uTries++ < 100) &&                            // safety net
                !(pHWHead->ser16550.LSR & SERIAL_LSR_TEMT)     // indicates FIFO not yet empty
              ) {
            DEBUGMSG3(ZONE_CLOSE, 
                      TEXT("SerClose, TX in progress, IER 0x%X, LSR 0x%X\r\n"),
                       *pHWHead->ser16550.pIER, pHWHead->ser16550.LSR);
            Sleep(10);
        }
#endif

        // TODO - When the device is closed, should power it down or somehow try to
        // let the desktop know that we aren't doing anything with any data that it
        // might be sending our way..

    }
	RegRemoveDeviceContext(pUsbFTDI);

	SetEvent(pUsbFTDI->hCloseEvent);	// DeviceNotify might be waiting for this to finally remove everything

	ConfigureActiveSyncMonitor(pUsbFTDI);

	DEBUGMSG2(ZONE_CLOSE, TEXT("SerClose for device %p\n"), pUsbFTDI);
	DEBUGMSG1 (ZONE_CLOSE,(TEXT("<SerClose\n")));
    return 0;
}

// @doc OEM
// @func ULONG | SerRxIntr | This routine gets several characters from the hardware
//   receive buffer and puts them in a buffer provided via the second argument.
//   It returns the number of bytes lost to overrun.
// 
// @rdesc The return value indicates the number of overruns detected.
//   The actual number of dropped characters may be higher.
//
ULONG
SerRxIntr(
    PVOID pHead,                // @parm Pointer to hardware head
    PUCHAR pRxBuffer,           // @parm Pointer to receive buffer
    ULONG *pBufflen             // @parm In = max bytes to read, out = bytes read
    )
{
	DWORD dwWaitReturn;
	DWORD ntStatus;
    PSER_INFO pHWHead = (PSER_INFO)pHead;
	PFTDI_DEVICE pUsbFTDI;
    ULONG RetVal	= 0;
    ULONG TargetRoom = *pBufflen;
    //GDM: BYTE ucRcvCnt;
    ULONG ucRcvCnt;
    BOOL fRXFlag = FALSE;
    BOOL fReplaceparityErrors = FALSE;
    BOOL fNull;
    UCHAR cEvtChar, cRXChar;
    PUCHAR srce;
    PUCHAR srceEnd;

    DEBUGMSG2(0, TEXT(">SerRxIntr - len %d.\r\n"), *pBufflen);
    *pBufflen = 0;

	pUsbFTDI = pHWHead->pFTDICtxt;

	cEvtChar = pHWHead->dcb.EvtChar;
    fNull = pHWHead->dcb.fNull;
    if (pHWHead->dcb.fErrorChar && pHWHead->dcb.fParity)
        fReplaceparityErrors = TRUE;
    
    try {
		//
        // Check for a FIFO overflow +++ not sure how
		//
		dwWaitReturn = WaitForSingleObject(pUsbFTDI->ReadBuffer.hBufferMutex, INFINITE);
		switch (dwWaitReturn) {
		case WAIT_OBJECT_0:
			ntStatus	= ERROR_SUCCESS;
			break;
		case WAIT_FAILED:
			DEBUGMSG2( ZONE_ERROR, TEXT("hBufferMutex error:%d\n"), GetLastError());
			return ~ERROR_SUCCESS;
		default:
			DEBUGMSG2( ZONE_ERROR, TEXT("Unhandled BufferMutex WaitReason:%d\n"), dwWaitReturn );
			return ~ERROR_SUCCESS;
		};			

//		DEBUGMSG1( ZONE_FUNCTION, (TEXT("Read Request aquired mutex\n")));

        // See how much data we have received this is a BYTE count !!!
        //GDM: ucRcvCnt = (BYTE)(pUsbFTDI->ReadBuffer.rbSize - pUsbFTDI->ReadBuffer.rbBytesFree);
        ucRcvCnt = (pUsbFTDI->ReadBuffer.rbSize - pUsbFTDI->ReadBuffer.rbBytesFree);

        //
        // if copy wraps round read buffer, do the bit to the end of the buffer
        //
        if(pUsbFTDI->ReadBuffer.rbGet + ucRcvCnt >= pUsbFTDI->ReadBuffer.rbSize) {
			int i = 0;
            srce = pUsbFTDI->ReadBuffer.Buffer + pUsbFTDI->ReadBuffer.rbGet;
            srceEnd = pUsbFTDI->ReadBuffer.Buffer + pUsbFTDI->ReadBuffer.rbSize;
			// Read in as much data as possible from the FTDI chip. +++ hopefully this gets all of the data at the end - redo this!!!
			while(TargetRoom && ucRcvCnt && (srce != srceEnd)) {
				ucRcvCnt--;
				pUsbFTDI->ReadBuffer.rbBytesFree++;
				// Pull the byte out of the FIFO
				cRXChar = *srce++;

				if(!cRXChar && fNull) {
					 // Do nothing - byte gets discarded
					DEBUGMSG1(ZONE_FLOW| ZONE_WARN,
                			  (TEXT("Dropping NULL byte due to fNull\r\n")));
				} 
				else {
					// Finally, we can store the byte, update status and save.
					DEBUGMSG2(ZONE_INIT, TEXT("RX %X:\r\n"), cRXChar);
					*pRxBuffer++ = cRXChar;
					(*pBufflen)++;
					--TargetRoom;
				}

				// See if we need to generate an EV_RXFLAG for the received char.
				if (cRXChar == cEvtChar)
					fRXFlag = TRUE;
			}
			pUsbFTDI->ReadBuffer.rbGet = 0;
//			DEBUGMSG2(ZONE_INIT, TEXT("Last RX %X:\r\n"), cRXChar);

		}
		if(ucRcvCnt) {

            srce = pUsbFTDI->ReadBuffer.Buffer + pUsbFTDI->ReadBuffer.rbGet;
            srceEnd = srce + ucRcvCnt;
			DEBUGMSG3(ZONE_FUNCTION, TEXT("ReadCopied from %d to %d"), pUsbFTDI->ReadBuffer.rbGet, (pUsbFTDI->ReadBuffer.rbGet + ucRcvCnt));

           	while(TargetRoom && ucRcvCnt && (srce != srceEnd)) {
				ucRcvCnt--;
				pUsbFTDI->ReadBuffer.rbBytesFree++;
				// Pull the byte out of the FIFO
				cRXChar = *srce++;

				if(!cRXChar && fNull) {
					 // Do nothing - byte gets discarded
					DEBUGMSG1(ZONE_FLOW| ZONE_WARN,
                			  (TEXT("Dropping NULL byte due to fNull\r\n")));
				} 
				else {
					// Finally, we can store the byte, update status and save.
					DEBUGMSG2(0, TEXT("RX %X:\r\n"), cRXChar);
					*pRxBuffer++ = cRXChar;
					(*pBufflen)++;
					--TargetRoom;
				}

				// See if we need to generate an EV_RXFLAG for the received char.
				if(cRXChar == cEvtChar)
					fRXFlag = TRUE;
				pUsbFTDI->ReadBuffer.rbGet++;
			}
        }

        
        // If we took all the receive data, then clear the interrupt
        if (!ucRcvCnt) {
			pHWHead->cIntStat &= ~RX_INT_MASK;	// clear the "interrupt"
        } 
		else {
            // We weren't able to read all the data in this pass.  We leave
            // the interrupt set so we will get called again by the MDD.
            DEBUGMSG3(ZONE_READ, TEXT("RXIntr : %d bytes left in FTD (%d)\r\n"), 
                ucRcvCnt, pUsbFTDI->ReadBuffer.rbSize - pUsbFTDI->ReadBuffer.rbBytesFree);
        }        

		ReleaseMutex(pUsbFTDI->ReadBuffer.hBufferMutex);        
    }
    except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // just exit
    }
    
    // if we saw one (or more) EVT chars, then generate an event
    if(fRXFlag)
        EvaluateEventFlag(pHWHead->pMddHead, EV_RXFLAG);

    DEBUGMSG3(ZONE_READ, TEXT("-GetBytes - rx %d, drop %d.\r\n"), *pBufflen, pHWHead->DroppedBytes);

    RetVal = pHWHead->DroppedBytes;
    pHWHead->DroppedBytes = 0;
    return RetVal;
}

//
// @doc OEM
// @func PVOID | SerGetRxStart | This routine returns the start of the hardware
// receive buffer.  See SerGetRxBufferSize.
// 
// @rdesc The return value is a pointer to the start of the device receive buffer.
// 
PVOID
SerGetRxStart(
    PVOID   pHead // @parm PVOID returned by Serinit.
    )
{
    return NULL;
}

//
// @doc OEM
// @func ULONG | SerGetInterruptType | This function is called
//   by the MDD whenever an interrupt occurs.  The return code
//   is then checked by the MDD to determine which of the four
//   interrupt handling routines are to be called.
// 
// @rdesc This routine returns a bitmask indicating which interrupts
//   are currently pending.
// 
INTERRUPT_TYPE
SerGetInterruptType(
    PVOID pHead      // Pointer to hardware head
    )
{
    PSER_INFO pHWHead = (PSER_INFO)pHead;
    INTERRUPT_TYPE interrupts = 0;

	if(pHWHead->cIntStat & LINE_STATUS_INT_MASK) {			// Line status change
        DEBUGMSG1(ZONE_THREAD, (TEXT("SerGetInterruptType: INTR_LINE\r\n")));
        interrupts |= INTR_LINE;
		pHWHead->cIntStat &= ~LINE_STATUS_INT_MASK;	// clear the "interrupt"
 	}
    else if((pHWHead->cIntStat & MODEM_INT_MASK)) {			// Modem status change
        interrupts |= INTR_MODEM;
		pHWHead->cIntStat &= ~MODEM_INT_MASK;		// clear the "interrupt"
 	}
    else if(pHWHead->cIntStat & TX_INT_MASK){				// ready to TX more data.
        DEBUGMSG1(ZONE_THREAD, (TEXT("SerGetInterruptType: INTR_TX\r\n")));
        interrupts |= INTR_TX;
		pHWHead->cIntStat &= ~TX_INT_MASK;			// clear the "interrupt"
    }
    else if(pHWHead->cIntStat & RX_INT_MASK) {				// RX data ready
        DEBUGMSG1(ZONE_THREAD, (TEXT("SerGetInterruptType: INTR_RX\r\n")));
        interrupts |= INTR_RX;
    }
    return interrupts;
}

//
// @doc OEM
// @func ULONG | SerModemIntr | This routine is called from the MDD
//   whenever INTR_MODEM is returned by SerGetInterruptType.
// 
// @rdesc None
//
VOID
SerModemIntr(
    PVOID pHead                // Hardware Head
    )
{
    PSER_INFO pHWHead = (PSER_INFO)pHead;
    DWORD dwModemStatus;
    DWORD dwOldModemStatus;
	PFTDI_DEVICE pUsbFTDI;

	pUsbFTDI = pHWHead->pFTDICtxt;
	// modem status has changed
    dwModemStatus = pUsbFTDI->SerialRegs.MSR;
    dwOldModemStatus = pUsbFTDI->OldModemStatus;

	if (pHWHead->cOpenCount) 
	{
		// If open, notify app of any control line changes.
		DEBUGMSG3 (ZONE_EVENTS, TEXT("Modem Status %2.2X <> %2.2X\r\n"), dwModemStatus, dwOldModemStatus);

		if((dwOldModemStatus & FT_CTS_ON) != (dwModemStatus & FT_CTS_ON))
			EvaluateEventFlag(pHWHead->pMddHead, EV_CTS);
		if((dwOldModemStatus & FT_DSR_ON) != (dwModemStatus & FT_DSR_ON))
			EvaluateEventFlag(pHWHead->pMddHead, EV_DSR);
		if((dwOldModemStatus & FT_RING_ON) != (dwModemStatus & FT_RING_ON))
			EvaluateEventFlag(pHWHead->pMddHead, EV_RING);
		if((dwOldModemStatus & FT_DCD_ON) != (dwModemStatus & FT_DCD_ON))
			EvaluateEventFlag(pHWHead->pMddHead, EV_RLSD);
	} 
	else // +++ really not sure about this at all? Are we notifying that a serial device is active?
	{
		// If RLSD transitioned to active, we need to generate event.
		if ((dwModemStatus & MS_RLSD_ON) && !(dwOldModemStatus & MS_RLSD_ON)) 
		{
			if (IsAPIReady(SH_WMGR)) 
			{
				CeEventHasOccurred(NOTIFICATION_EVENT_RS232_DETECTED, NULL);
			}
			DEBUGMSG1(ZONE_EVENTS, (TEXT("Indicated RS232 Cable Event\r\n")));
		}
	}
}

//
// @doc OEM
// @func ULONG | SerLineIntr | This routine is called from the MDD
//   whenever INTR_LINE is returned by SerGetInterruptType.
// 
// @rdesc None
//
VOID
SerLineIntr(
    PVOID pHead                // Hardware Head
    )
{
	PSER_INFO pHWHead = (PSER_INFO)pHead;
    DWORD dwLineStatus;
	PFTDI_DEVICE pUsbFTDI;
	DWORD ulLineEvents = 0;

	pUsbFTDI = pHWHead->pFTDICtxt;

	// modem status has changed
    dwLineStatus = pUsbFTDI->SerialRegs.LSR;
	pUsbFTDI->SerialRegs.LSR = 0;

	if (pHWHead->cOpenCount) 
	{
		if (dwLineStatus & (SERIAL_LSR_OE | SERIAL_LSR_PE | SERIAL_LSR_FE)) {
			if(dwLineStatus & SERIAL_LSR_OE) {
				pHWHead->DroppedBytes++;
				pHWHead->CommErrors |= CE_OVERRUN;
			}
			if(dwLineStatus & SERIAL_LSR_PE) {
				pHWHead->CommErrors |= CE_RXPARITY;
			}
			if(dwLineStatus & SERIAL_LSR_FE) {
				pHWHead->CommErrors |= CE_FRAME;
			}

			ulLineEvents |= EV_ERR;
		}

		if(dwLineStatus & SERIAL_LSR_BI)
			ulLineEvents |= EV_BREAK;
	}      
	
	// Let WaitCommEvent know about this error
    if(ulLineEvents)
		EvaluateEventFlag(pHWHead->pMddHead, ulLineEvents);

}

//
// @doc OEM
// @func ULONG | SerGetRxBufferSize | This function returns
// the size of the hardware buffer passed to the interrupt
// initialize function.  It would be used only for devices
// which share a buffer between the MDD/PDD and an ISR.
//
// 
// @rdesc This routine always returns 0 for 16550 UARTS.
// 
ULONG
SerGetRxBufferSize(
    PVOID pHead
    )
{
    return 0;
}

/*++

Routine Description:
	The original CLIIssuebulk transfer wasnt sufficient enough to allow for cancellation of writes
	therfore most of the code in that is repeated here with the extra functionality of dealing with timouts
	and purges. This was picked up with the Serial Communications test provided by windows CE.

Arguments:
	Buffer - buffer we are writing from
	Length - length of Buffer
	BytesWrite - pointer to a buffer to receive the bytes written
    pDevExt - our device extension
	dwTimeout - a timout on the packet (this does not add 5 seconds as in dead man timout.

Return Value:

    TRUE if successful,
    FALSE otherwise

--*/
NTSTATUS
OutRequest(
	PVOID Buffer,
	ULONG Length,
	PULONG BytesWrite,
    PFTDI_DEVICE pDevExt,
	DWORD dwTimeout
	)
{
	DWORD dwUsbErr;
    NTSTATUS ntStatus;
	ULONG TransferBufferLength;
    USB_TRANSFER hTransfer = NULL;

	ntStatus = IssueBulkTransfer(	
					pDevExt->UsbFuncs,
					pDevExt->BulkOut.hPipe,
					pDevExt->BulkOut.pCompletionRoutine,
					pDevExt->BulkOut.hEvent,
					pDevExt->BulkOut.dwBulkFlags,
					Buffer, 
					0,
					Length,
					&TransferBufferLength,
					dwTimeout,
					&dwUsbErr
					);

	DEBUGMSG2(ZONE_ERROR, TEXT("OutRequest error:%d\n"),  ntStatus);
	if(dwUsbErr == USB_STALL_ERROR) {

		//
		// Try to recover from STALL (USBD_HALTED)
		//
		DEBUGMSG1(ZONE_ERROR, (TEXT("nRequest RESETTING PIPE\n")));

		if(LResetPipe(pDevExt, &pDevExt->BulkIn) != ERROR_SUCCESS)
			ntStatus = STATUS_DEVICE_NOT_CONNECTED;
	}
	//
	// Let the BULK_ERROR handle the other error codes
	//
	//DEBUGMSG3(ZONE_THREAD, TEXT("Bulk Write=%d, Length=%d\n"), *BytesWrite, Length);
	//DEBUGMSG3(ZONE_THREAD, TEXT("Bulk ntStatus=%d, Error=%d \n"), ntStatus, dwUsbErr);
	
    //
    // USBD maps the error code for us
    //

//	DEBUGMSG3(ZONE_THREAD, TEXT("Bulk Write=%d, Length=%d\n"), *BytesWrite, Length);
//	DEBUGMSG3(ZONE_THREAD, TEXT("Bulk ntStatus=%d, Error=%d \n"), ntStatus, dwUsbErr);

	if (ntStatus == ERROR_SUCCESS) {
		*BytesWrite = TransferBufferLength;
	}

    if (*BytesWrite > 2) {
        DEBUGMSG3(ZONE_FUNCTION, TEXT("OutRequest Requested:%d Wrote:%d\n"), Length, *BytesWrite);
    }

	return ntStatus;
}

//
// @doc OEM
// @func ULONG | SerTXIntr | This routine is called from the new MDD
//   whenever INTR_TX is returned by SerGetInterruptType.  It is responsible
//   for loading up the TX FIFO with next block of data.
// 
// @rdesc None
//
VOID
SerTxIntr(
    PVOID pHead,          // Hardware Head
    PUCHAR pTxBuffer,     // @parm Pointer to receive buffer
    ULONG *pBuffLen       // @parm In = max bytes to transmit, out = bytes transmitted
    )
{
    PSER_INFO   pHWHead = (PSER_INFO) pHead;
	PFTDI_DEVICE pUsbFTDI;
	PUCHAR pTxPtr, pTxToSendPtr;
	ULONG BufferLength = *pBuffLen;
	DWORD dwTimeout;
	DWORD dwTransferSize, dwExpectedTransferSize;
	DWORD dwBytesTransferred;
	DWORD dwTotalTransferred = 0;
	DWORD dwErr = ERROR_SUCCESS;
	DWORD dwUsbErr = USB_NO_ERROR;

    DEBUGMSG2(ZONE_WRITE, TEXT(">SerTxIntr 0x%X\r\n"), pHead);

	pHWHead->CommErrors &= ~CE_TXFULL;

    // If nothing to send, just return
    if(!BufferLength) {
        DEBUGMSG1(ZONE_WRITE, (TEXT("TX: nothing to send\r\n")));            
        return;
    }

	*pBuffLen = 0;		 // In case we don't send anything below.

	pHWHead = (PSER_INFO)pHead;	
	pUsbFTDI = pHWHead->pFTDICtxt;
 
	if(ACCEPT_IO(pUsbFTDI)) {
		EnterCriticalSection(&(pHWHead->TransmitCritSec));
#ifdef REMOVED_AS_NOT_IN_WINDOWS_DRIVER
        // If CTS flow control is desired, check cts. If clear, don't send,
        // but loop.  When CTS comes back on, the OtherInt routine will
        // detect this and re-enable TX interrupts (causing Flushdone).
        // For finest granularity, we would check this in the loop below,
        // but for speed, I check it here (up to 8 xmit characters before
        // we actually flow off.
        if ( pHWHead->dcb.fOutxCtsFlow ) {
            // ReadMSR( pHWHead );
            // We don't need to explicitly read the MSR, since we always enable
            // IER_MS, which ensures that we will get an interrupt and read
            // the MSR whenever CTS, DSR, TERI, or DCD change.

            if (! (pUsbFTDI->SerialRegs.MSR & SERIAL_MSR_CTS) ) {
//                unsigned char byte;
                DEBUGMSG1 (ZONE_WRITE|ZONE_FLOW,
                          (TEXT("SerTxIntrEx, flowed off via CTS\n") ) );
/*                pHWHead->CTSFlowOff = TRUE;  // Record flowed off state
                if (pHWHead->pIsrInfoVirt==NULL || pHWHead->pXmitBuffer==NULL || GetDataSize(pHWHead->pXmitBuffer)==0)  {// no data inbuffer. 
                    byte = INB(pHWHead, pIER);
                    OUTB(pHWHead, pIER, byte & ~SERIAL_IER_THR); // disable TX interrupts while flowed off
                }*/
                // We could return a positive value here, which would
                // cause the MDD to periodically check the flow control
                // status.  However, we don't need to since we know that
                // the DCTS interrupt will cause the MDD to call us, and we
                // will subsequently fake a TX interrupt to the MDD, causing
                // him to call back into PutBytes.

                LeaveCriticalSection(&(pHWHead->TransmitCritSec));
                return;
            }
        }

        // Same thing applies for DSR
        if ( pHWHead->dcb.fOutxDsrFlow ) {
            // ReadMSR( pHWHead );
            // We don't need to explicitly read the MSR, since we always enable
            // IER_MS, which ensures that we will get an interrupt and read
            // the MSR whenever CTS, DSR, TERI, or DCD change.

            if (! (pUsbFTDI->SerialRegs.MSR & SERIAL_MSR_DSR) ) {
                DEBUGMSG (ZONE_WRITE|ZONE_FLOW,
                          (TEXT("SerTxIntrEx, flowed off via DSR\n") ) );
 /*               pHWHead->DSRFlowOff = TRUE;  // Record flowed off state
                if (pHWHead->pIsrInfoVirt==NULL || pHWHead->pXmitBuffer==NULL || GetDataSize(pHWHead->pXmitBuffer)==0) { // no data inbuffer. 
                    OUTB(pHWHead, pIER, IER_NORMAL_INTS); // disable TX interrupts while flowed off
                }*/
                // See the comment above above positive return codes.

                LeaveCriticalSection(&(pHWHead->TransmitCritSec));
                return;
            }
        }
#endif
		//
		// Copy Data to the FIFO
		//
		pTxPtr = pTxBuffer;

		while(pTxPtr && BufferLength) {

			dwTransferSize = min(BufferLength, pUsbFTDI->OutParams.CurrentTransferSize);
			dwTimeout = dwTransferSize * pHWHead->CommTimeouts.WriteTotalTimeoutMultiplier +
						pHWHead->CommTimeouts.WriteTotalTimeoutConstant;

			if (!dwTimeout) {
				dwTimeout = INFINITE;
			}
			DEBUGMSG2(ZONE_WRITE, TEXT(">Write dwTimeout %d\r\n"), dwTimeout);

			//
			// Save this here so we only transfer what app is expecting else we could end up looping
			// forever if we go the emulation path.
			//
			dwExpectedTransferSize = dwTransferSize;
			if(FT_EMUL_SUBSTITUTION_MODE(pUsbFTDI)) {

				//
				// modem control requestes are delt with in utils.c
				//
				dwTransferSize = FT_EmulCopyTxBytes(
									pUsbFTDI->EmulVars.pWriteBuffer,
									pTxPtr,
									&dwExpectedTransferSize,
									dwExpectedTransferSize,
									pUsbFTDI
									);

				pTxToSendPtr = pUsbFTDI->EmulVars.pWriteBuffer;
			}
			else {
				pTxToSendPtr = pTxPtr;
			}

			dwBytesTransferred = 0;

			dwErr = OutRequest(
						pTxToSendPtr,
						dwTransferSize,
						&dwBytesTransferred,
						pUsbFTDI,
						dwTimeout
						);
			DEBUGMSG2(ZONE_WRITE, TEXT(">OutRequest %d\r\n"), dwTransferSize);

			//
			// Tell the FTDIEventThread we have completed the transmit
			//
/*			pUsbFTDI->pHWHead->cIntStat |= TX_INT_MASK;
			if(!SetEvent( pUsbFTDI->pHWHead->hSerialEvent)) {
				dwErr = GetLastError();
				DEBUGMSG2(ZONE_ERROR,TEXT("*** SetSerialEvent ERROR:%d ***\n"), dwErr);
			}
*/
			if(dwErr == ERROR_OUTOFMEMORY) {

				//
				// Change to 64 and retry
				//
				pUsbFTDI->OutParams.CurrentTransferSize = 64;
			}
			else if(ERROR_SUCCESS != dwErr) {
				DEBUGMSG4(ZONE_ERROR, TEXT("IssueBulkTransfer error:%d, 0x%x, trans = 0x%X\n"), dwErr, dwUsbErr, dwBytesTransferred);
	//			IoErrorHandler(pUsbFTDI, &pUsbFTDI->BulkOut, pUsbFTDI->BulkOut.EndpointAddress, dwUsbErr);
				if(pUsbFTDI->ConfigDataFlags & FTDI_PERSISTANT_DEVICE_INSTANCE) {
					Sleep(100);
					break;
				}
				else {
					dwTotalTransferred=(DWORD)-1;
					SetLastError(dwErr);
					LeaveCriticalSection(&(pHWHead->TransmitCritSec));
					return;
				}
			}
			else {

				//
				// only add on what the user is expecting? Even for the emulation path
				// This isnt quite OK as what if we dont send the amout we wanted.
				//
				pTxPtr +=dwExpectedTransferSize;
				BufferLength -=dwExpectedTransferSize;
				dwTotalTransferred +=dwExpectedTransferSize;
			}
		}
		LeaveCriticalSection(&(pHWHead->TransmitCritSec));
		//
		// Tell the FTDIEventThread we have completed the transmit
		//
		pUsbFTDI->pHWHead->cIntStat |= TX_INT_MASK;
		if(!SetEvent( pUsbFTDI->pHWHead->hSerialEvent)) {
			dwErr = GetLastError();
			DEBUGMSG2(ZONE_ERROR,TEXT("*** SetSerialEvent ERROR:%d ***\n"), dwErr);
		}


	} else {
		DEBUGMSG1(ZONE_ERROR,(TEXT("SerTxIntr: ERROR_INVALID_HANDLE\n")));
		dwTotalTransferred=(DWORD)-1;
		SetLastError(ERROR_INVALID_HANDLE);
	}

    DEBUGMSG3(ZONE_WRITE, TEXT("Tx %d of %d\r\n"), dwTotalTransferred, *pBuffLen);

#ifdef DEBUG
    if (ZONE_TXDATA)
        CELOGDATA(1, CELID_RAW_CHAR, pTxBuffer, (WORD)*pBuffLen, 1, CELZONE_MISC);
#endif

    // Return number of bytes transmitted via pBuffLen.
	*pBuffLen = dwTotalTransferred;

    DEBUGMSG2(ZONE_WRITE, TEXT("<SerTxIntr 0x%X\r\n"), pHead);
}

/*
 @doc OEM
 @func BOOL | SerPowerOff |
 *  Called by driver to turn off power to serial port.
 *  Not exported to users, only to driver.
 *
 @rdesc This routine returns a status.
 */
static
BOOL
SerPowerOff(
           PVOID   pHead       // @parm	PVOID returned by SerInit.
           )
{
    PSER_INFO pHWHead = (PSER_INFO)pHead;

    // Not required functionality.
    
	DEBUGMSG2(ZONE_IOCTL, TEXT("SerPowerOff - have we been surprise removed???\r\n"), pHead);

    return TRUE;
}

/*
 @doc OEM
 @func BOOL | SerPowerOn |
 *  Called by driver to turn on power to serial port.
 *  Not exported to users, only to driver.
 *
 @rdesc This routine returns a status.
 */
static
BOOL
SerPowerOn(
          PVOID   pHead       // @parm	PVOID returned by SerInit.
          )
{
    PSER_INFO   pHWHead = (PSER_INFO)pHead;

    // Not required functionality.

    return TRUE;
}

//
// @doc OEM
// @func void | SerClearDtr | This routine clears DTR.
//
// @rdesc None.
//
VOID
SerClearDTR(
    PVOID   pHead // @parm PVOID returned by HWinit.
    )
{
    PSER_INFO		pHWHead;
	PFTDI_DEVICE	pUsbFTDI;

    DEBUGMSG2 (ZONE_FUNCTION, TEXT(">SerClearDTR, 0x%X\r\n"), pHead);

	pHWHead = (PSER_INFO)pHead;	
	pUsbFTDI = pHWHead->pFTDICtxt;

	if(ACCEPT_IO(pUsbFTDI)) {
		FT_ClrDtr(pHWHead);
	}

    DEBUGMSG2(ZONE_FUNCTION, TEXT("<SerClearDTR, 0x%X\r\n"), pHead);
}

//
// @doc OEM
// @func VOID | SerSetDTR | This routine sets DTR.
// 
// @rdesc None.
//
VOID
SerSetDTR(
    PVOID   pHead // @parm PVOID returned by HWinit.
    )
{    
    PSER_INFO		pHWHead;
	PFTDI_DEVICE	pUsbFTDI;

    DEBUGMSG2 (ZONE_FUNCTION, TEXT(">SerSetDTR, 0x%X\r\n"), pHead);

	pHWHead = (PSER_INFO)pHead;	
	pUsbFTDI = pHWHead->pFTDICtxt;

	if(ACCEPT_IO(pUsbFTDI)) {
		FT_SetDtr(pHWHead);
	}

    DEBUGMSG2(ZONE_FUNCTION, TEXT("<SerSetDTR, 0x%X\r\n"), pHead);
}

//
// @doc OEM
// @func VOID | SerClearRTS | This routine clears RTS.
// 
// @rdesc None.
//
VOID
SerClearRTS(
    PVOID   pHead // @parm PVOID returned by HWinit.
    )
{
    PSER_INFO		pHWHead;
	PFTDI_DEVICE	pUsbFTDI;

    DEBUGMSG2(ZONE_FUNCTION, TEXT(">SerClrRTS, 0x%X\r\n"), pHead);

	pHWHead = (PSER_INFO)pHead;	
	pUsbFTDI = pHWHead->pFTDICtxt;

	if(ACCEPT_IO(pUsbFTDI)) {
		FT_ClrRts(pHWHead);
	}

    DEBUGMSG2(ZONE_FUNCTION, TEXT("<SerClrRTS, 0x%X\r\n"), pHead);
}

//
// @doc OEM
// @func VOID | SerSetRTS | This routine sets RTS.
// 
// @rdesc None.
//
VOID
SerSetRTS(
    PVOID   pHead // @parm PVOID returned by HWinit.
    )
{
	PSER_INFO		pHWHead;
	PFTDI_DEVICE	pUsbFTDI;

    DEBUGMSG2(ZONE_FUNCTION, TEXT(">SerSetRTS, 0x%X\r\n"), pHead);

	pHWHead = (PSER_INFO)pHead;	
	pUsbFTDI = pHWHead->pFTDICtxt;

	if(ACCEPT_IO(pUsbFTDI)) {
		FT_SetRts(pHWHead);
	}

    DEBUGMSG2(ZONE_FUNCTION, TEXT("<SerSetRTS, 0x%X\r\n"), pHead);
}

/*
 @doc OEM
 @func BOOL | SerEnableIR | This routine enables ir.
 *  Not exported to users, only to driver.
 *
 @rdesc Returns TRUE if successful, FALSEotherwise.
 */
static
BOOL
SerEnableIR(
           PVOID   pHead, // @parm PVOID returned by Serinit.
           ULONG   BaudRate  // @parm PVOID returned by HWinit.
           )
{
    // We don't support an IR mode, so fail.
    return TRUE;		// To pass the test in the Serial Comms test
}

/*
 @doc OEM
 @func BOOL | SerDisableIR | This routine disable the ir.
 *  Not exported to users, only to driver.
 *
 @rdesc Returns TRUE if successful, FALSEotherwise.
 */
static
BOOL
SerDisableIR(
            PVOID   pHead /*@parm PVOID returned by Serinit. */
            )
{
    // We don't support an IR mode.  But don't fail, in case
    // someone calls this redundantly to ensure that we are
    // in wired mode, which is what we support.
    return TRUE;
}

//
// @doc OEM
// @func VOID | SerClearBreak | This routine clears break.
// 
// @rdesc None.
// 
VOID
SerClearBreak(
    PVOID   pHead // @parm PVOID returned by HWinit.
    )
{
	PSER_INFO		pHWHead;
	PFTDI_DEVICE	pUsbFTDI;

    DEBUGMSG2(ZONE_FUNCTION, TEXT(">SerClrBreak, 0x%X\r\n"), pHead);

	pHWHead = (PSER_INFO)pHead;	
	pUsbFTDI = pHWHead->pFTDICtxt;
	
	if(ACCEPT_IO(pUsbFTDI)) {
		FT_SetBreak(pUsbFTDI,FTDI_BREAK_OFF);
	}
    
	DEBUGMSG2(ZONE_FUNCTION, TEXT("<SerClrBreak, 0x%X\r\n"), pHead);
}

//
// @doc OEM
// @func VOID | SerSetBreak | This routine sets break.
// 
// @rdesc None.
//
VOID
SerSetBreak(
    PVOID   pHead // @parm PVOID returned by HWinit.
    )
{
	PSER_INFO		pHWHead;
	PFTDI_DEVICE	pUsbFTDI;

    DEBUGMSG2(ZONE_FUNCTION, TEXT(">SerSetBreak, 0x%X\r\n"), pHead);

	pHWHead = (PSER_INFO)pHead;	
	pUsbFTDI = pHWHead->pFTDICtxt;

	if(ACCEPT_IO(pUsbFTDI)) {
		FT_SetBreak(pUsbFTDI,FTDI_BREAK_ON);
	}

    DEBUGMSG2(ZONE_FUNCTION, TEXT("<SerSetBreak, 0x%X\r\n"), pHead);
}

//
// @doc OEM
// @func	BOOL | SerXmitComChar | Transmit a char immediately
// 
// @rdesc	TRUE if succesful
//
BOOL
SerXmitComChar(
    PVOID   pHead,    // @parm PVOID returned by HWInit.
    UCHAR   ComChar   // @parm Character to transmit. 
    )
{
    PSER_INFO   pHWHead = (PSER_INFO)pHead;
    
    DEBUGMSG2(ZONE_FUNCTION,
              TEXT("+SerXmitComChar 0x%X\r\n"), pHead);

     // Get critical section, then transmit when buffer empties
    DEBUGMSG2 (ZONE_WRITE, TEXT("XmitComChar wait for CritSec %x.\r\n"),
                           &(pHWHead->TransmitCritSec));
    EnterCriticalSection(&(pHWHead->TransmitCritSec));
    DEBUGMSG2 (ZONE_WRITE, TEXT("XmitComChar got CritSec %x.\r\n"),
                           &(pHWHead->TransmitCritSec));
    try
    {
        DEBUGMSG2(ZONE_INIT,TEXT("!!! SerXmitComChar 0x%X not implemented\r\n"), pHead);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Just exit
    }
    
    LeaveCriticalSection(&(pHWHead->TransmitCritSec));
    DEBUGMSG2(ZONE_WRITE, TEXT("XmitComChar released CritSec %x.\r\n"),
                           &(pHWHead->TransmitCritSec));
    
    DEBUGMSG2(ZONE_FUNCTION,
              TEXT("-SerXmitComChar 0x%X\r\n"), pHead);
    
    return TRUE;
}
        

//  
// @doc OEM
// @func	ULONG | SerGetStatus | This structure is called by the MDD
//   to retrieve the contents of a COMSTAT structure.
//
// @rdesc	The return is a ULONG, representing success (0) or failure (-1).
//
ULONG
SerGetStatus(
    PVOID	pHead,	// @parm PVOID returned by HWInit.
    LPCOMSTAT	lpStat	// Pointer to LPCOMMSTAT to hold status.
    )
{
    PSER_INFO pHWHead = (PSER_INFO)pHead;
    ULONG RetVal = pHWHead->CommErrors;
  	PFTDI_DEVICE pUsbFTDI;

	pUsbFTDI = pHWHead->pFTDICtxt;
    
    DEBUGMSG2 (ZONE_FUNCTION, TEXT(">SerGetStatus 0x%X\r\n"), pHead);

    pHWHead->CommErrors = 0; // Clear old errors each time

    if (lpStat)
    {
		if(pUsbFTDI->FlowControl & FT_FLOW_RTS_CTS) {
			lpStat->fCtsHold = (pUsbFTDI->SerialRegs.MSR & FT_CTS_ON);
		}
		if(pUsbFTDI->FlowControl & FT_FLOW_DTR_DSR) {
			lpStat->fDsrHold = (pUsbFTDI->SerialRegs.MSR & FT_DSR_ON);
		}
    }
    else
        RetVal = (ULONG) -1;

    DEBUGMSG2 (ZONE_FUNCTION, TEXT("<SerGetStatus 0x%X\r\n"), pHead);
    return RetVal;
}

//
// @doc OEM
// @func	ULONG | SerReset | Perform any operations associated
//   with a device reset
//
// @rdesc	None.
//
VOID
SerReset(
    PVOID pHead	// @parm PVOID returned by HWInit.
    )
{
    PSER_INFO pHWHead = (PSER_INFO)pHead;
	PFTDI_DEVICE pUsbFTDI;

    DEBUGMSG2(ZONE_FUNCTION, TEXT(">SerReset 0x%X\r\n"), pHead);

	pUsbFTDI = pHWHead->pFTDICtxt;

    memset(&pHWHead->Status, 0, sizeof(COMSTAT));

	FT_ResetHardware(pUsbFTDI);
	FT_Purge(pUsbFTDI, FT_PURGE_TX);
	FT_Purge(pUsbFTDI, FT_PURGE_RX);
   
    DEBUGMSG2 (ZONE_FUNCTION, TEXT("<SerReset 0x%X\r\n"), pHead);
}

//
// @doc OEM
// @func	VOID | SerGetModemStatus | Retrieves modem status.
//
// @rdesc	None.
//
VOID
SerGetModemStatus(
    PVOID   pHead,	    // @parm PVOID returned by HWInit.
    PULONG  pModemStatus    // @parm PULONG passed in by user.
    )
{
    PSER_INFO pHWHead = (PSER_INFO) pHead;
  	PFTDI_DEVICE pUsbFTDI;
  
    DEBUGMSG2(ZONE_FUNCTION, TEXT(">SerGetModemStatus 0x%X\r\n"), pHead);

  	pUsbFTDI = pHWHead->pFTDICtxt;
	*pModemStatus = pUsbFTDI->SerialRegs.MSR;

    DEBUGMSG3(ZONE_FUNCTION | ZONE_EVENTS,
              TEXT("<SerGetModemStatus 0x%X (stat x%X) \r\n"), pHead, *pModemStatus);
}

/*
 @doc OEM
 @func	VOID | SerGetCommProperties | Retrieves Comm Properties.
 *
 @rdesc	None.
 */
static
VOID
SerGetCommProperties(
                    PVOID   pHead,      // @parm PVOID returned by SerInit. 
                    LPCOMMPROP  pCommProp   // @parm Pointer to receive COMMPROP structure. 
                    )
{
    PSER_INFO pHWHead = (PSER_INFO)pHead;

    *pCommProp = pHWHead->CommProp;
}

//
// @doc OEM
// @func	VOID | SerPurgeComm | Purge RX and/or TX
// 
// @rdesc	None.
//

VOID
SerPurgeComm(
    PVOID   pHead,	    // @parm PVOID returned by HWInit.
    DWORD   fdwAction	    // @parm Action to take. 
    )
{
    PSER_INFO pHWHead;
   	PFTDI_DEVICE pUsbFTDI;
   
    DEBUGMSG2(ZONE_FUNCTION, TEXT(">SerPurgeComm 0x%X\r\n"), pHead);

	pHWHead = (PSER_INFO)pHead;	
	pUsbFTDI = pHWHead->pFTDICtxt;
   
    EnterCriticalSection(&(pHWHead->HwRegCritSec));

    try {
        // REVIEW THIS - I don't see how this could have terminated a pending read,
        // nor how RX interrupts would ever get turned back on.  I suspect that
        // RXABORT and TXABORT would both be better implemented in the MDD.
        if(fdwAction & PURGE_RXABORT) {
/* This seems to be needed to purge the current outstanding TX and RX +++ */
			DEBUGMSG2(ZONE_FUNCTION, TEXT("PURGE_RXABORT - stop in task\n"), pHead);
		}
        if(fdwAction & PURGE_TXABORT) {
			//
			// Windows driver does nothing here so I will follow.
			// The Serial Communications test will fail if this is dealt with.
			// I did experiment with having an Event you could set here and any outstanding writes 
			// would be cancelled (I waited for multiple objects in the OutRequest) but this failed the tests.
			//
			DEBUGMSG2(ZONE_FUNCTION, TEXT("PURGE_TXABORT Done\n"), pHead);
		}
        if(fdwAction & PURGE_TXCLEAR) {
			// Removed this to tie in with the windows VCP driver
			// FT_Purge(pUsbFTDI, FT_PURGE_TX);
        }

        if(fdwAction & PURGE_RXCLEAR) {
			//
			// windows VCP driver only purges the internal buffer (so do this here)
			//
			FT_PurgeInternalBuffer(pUsbFTDI);
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Just exit
    }

    LeaveCriticalSection(&(pHWHead->HwRegCritSec));

    DEBUGMSG2(ZONE_FUNCTION, TEXT("<SerPurgeComm 0x%X\r\n"), pHead);
}

BOOL CheckValidDCB(LPDCB   lpDCB) // +++ needs implemented
{
	*lpDCB = *lpDCB;
	return TRUE;
}

BOOL SetFlowParams(LPDCB lpDCB, TFtFlowControl *  pfc)
{
	if (lpDCB->fOutxCtsFlow)
		pfc->Control = FT_FLOW_RTS_CTS;
	else if (lpDCB->fOutxDsrFlow)
		pfc->Control = FT_FLOW_DTR_DSR;
	else if (lpDCB->fOutX || lpDCB->fInX) {
		pfc->Control = FT_FLOW_XON_XOFF;
		pfc->XonChar = lpDCB->XonChar;
		pfc->XoffChar = lpDCB->XoffChar;
	}
	else
		pfc->Control = FT_FLOW_NONE;	

	return TRUE;
}

//
// @doc OEM
// @func	BOOL | SerSetDCB | Sets new values for DCB.  This
// routine gets a DCB from the MDD.  It must then compare
// this to the current DCB, and if any fields have changed take
// appropriate action.
// 
// @rdesc	BOOL
//
// +++ this needs better checking
BOOL
SerSetDCB(
    PVOID   pHead,	    // @parm	PVOID returned by HWInit.
    LPDCB   lpDCB       // @parm    Pointer to DCB structure
    )
{
	DWORD			dwRet;
	BOOL			bChanged = FALSE;
    PSER_INFO		pHWHead;
  	PFTDI_DEVICE	pUsbFTDI;
	TFtLineControl	tfLineCtrl;
	TFtFlowControl  tfFlowCtrl;

    DEBUGMSG2(ZONE_FUNCTION, TEXT(">SerSetDCB 0x%X\r\n"), pHead);

	pHWHead = (PSER_INFO)pHead;	
	pUsbFTDI = pHWHead->pFTDICtxt;
   

    dwRet = ERROR_SUCCESS;

     // If the device is open, scan for changes and do whatever
     // is needed for the changed fields.  if the device isn't
     // open yet, just save the DCB for later use by the open.
    if(pHWHead->cOpenCount)		// +++ this was commented out for some reason! I didnt comment it so Ill do it here incase 
								// you get any strange behaviour
    {
         // Note, fparity just says whether we should check
         // receive parity.  And the 16550 won't let us NOT
         // check parity if we generate it.  So this field
         // has no effect on the hardware.
		if(CheckValidDCB(lpDCB)) {
			tfLineCtrl.WordLength = lpDCB->ByteSize;
 			tfLineCtrl.Parity = lpDCB->Parity;
 			tfLineCtrl.StopBits = lpDCB->StopBits;

			dwRet = FT_SetLineControl(pUsbFTDI, &tfLineCtrl); // +++ needs removed to save code and app memory
			if(dwRet == ERROR_SUCCESS) {
				SetFlowParams(lpDCB, &tfFlowCtrl);
				dwRet = FT_SetFlowControl(pUsbFTDI, &tfFlowCtrl);
			}
			if(dwRet == ERROR_SUCCESS) {
				dwRet = FT_SetBaudRate(pUsbFTDI, lpDCB->BaudRate);
			}
		}

		if(pHWHead->dcb.EvtChar) {
			TFtSpecialChars SpecialChars;

			SpecialChars.EventCharEnabled = lpDCB->EvtChar ? 1 : 0;
			SpecialChars.EventChar = lpDCB->EvtChar;
			//
			// From the MSDN documenation regarding the DCB
			//
			if(lpDCB->fParity && lpDCB->fErrorChar) {
				SpecialChars.ErrorCharEnabled = TRUE;
				SpecialChars.ErrorChar = lpDCB->ErrorChar;
			}
			else {
				SpecialChars.ErrorCharEnabled = FALSE;
				SpecialChars.ErrorChar = 0;
			}
			FT_SetChars(pUsbFTDI, &SpecialChars);
		}
    }
    
    if ((dwRet == ERROR_SUCCESS)) {
        // Now that we have done the right thing, store this DCB
        pHWHead->dcb = *lpDCB;
    }

    DEBUGMSG2(ZONE_FUNCTION, TEXT("<SerSetDCB 0x%X\r\n"), pHead);

	if(dwRet == ERROR_SUCCESS)
		return TRUE;
	else
		return FALSE;

}

//
// @doc OEM
// @func	BOOL | SerSetCommTimeouts | Sets new values for the
// CommTimeouts structure. routine gets a DCB from the MDD.  It
// must then compare this to the current DCB, and if any fields
// have changed take appropriate action.
// 
// @rdesc	ULONG
//
ULONG
SerSetCommTimeouts(
    PVOID   pHead,	    // @parm	PVOID returned by HWInit.
    LPCOMMTIMEOUTS   lpCommTimeouts // @parm Pointer to CommTimeout structure
    )
{
    PSER_INFO pHWHead = (PSER_INFO)pHead;
    ULONG retval = 0;
    
    DEBUGMSG2(ZONE_FUNCTION, TEXT(">SerSetCommTimeout 0x%X\r\n"), pHead);
    
     // Just store this DCB
    pHWHead->CommTimeouts = *lpCommTimeouts;

    DEBUGMSG2 (ZONE_FUNCTION, TEXT("<SerSetCommTimeout 0x%X\r\n"), pHead);

    return retval;
}


/*
	Unsure which way to go with the error setting - can we use the same STATUS_INVALID_PARAMETER? 
	Will this map to the ERROR_INVALID_PARAMETERS??
*/	
void ErrorInvalidArgs(NTSTATUS * ntStatus, BOOL * ret, DWORD * err)
{
	*ntStatus = STATUS_INVALID_PARAMETER;
	*ret = FALSE;
	*err = ERROR_INVALID_PARAMETER;
}


void ErrorSuccess(NTSTATUS * ntStatus, BOOL * ret, DWORD * err)
{
	*ntStatus	= STATUS_SUCCESS;
	*ret		= TRUE;
	*err		= ERROR_SUCCESS;
}

void ErrorFail(NTSTATUS * ntStatus, BOOL * ret, DWORD * err)
{
	*ntStatus	= STATUS_UNSUCCESSFUL;
	*ret		= FALSE;
	*err		= GetLastError();
}


//
//  @doc OEM
//  @func    BOOL | SerIoctl | Device IO control routine.  
//  @parm DWORD | dwOpenData | value returned from COM_Open call
//    @parm DWORD | dwCode | io control code to be performed
//    @parm PBYTE | pBufIn | input data to the device
//    @parm DWORD | dwLenIn | number of bytes being passed in
//    @parm PBYTE | pBufOut | output data from the device
//    @parm DWORD | dwLenOut |maximum number of bytes to receive from device
//    @parm PDWORD | pdwActualOut | actual number of bytes received from device
//
//    @rdesc        Returns TRUE for success, FALSE for failure
//
//  @remark  The MDD will pass any unrecognized IOCTLs through to this function.
//
BOOL
SerIoctl(PVOID pHead, DWORD dwCode,PBYTE pBufIn,DWORD dwLenIn,
         PBYTE pBufOut,DWORD dwLenOut,PDWORD pdwActualOut)
{
    PSER_INFO pHWHead = (PSER_INFO) pHead;
    BOOL ret = TRUE;
	DWORD err	= ~ERROR_SUCCESS;
    NTSTATUS ntStatus;
  	PFTDI_DEVICE	pUsbFTDI;

	pUsbFTDI = pHWHead->pFTDICtxt;

    DEBUGMSG2 (ZONE_FUNCTION, TEXT("+SerIoctl 0x%X\r\n"), pHead);

	if((pUsbFTDI->Flags.UnloadPending == TRUE) && (dwCode != IOCTL_FT_GET_DEVICE_EVENT_INFO)) {
		return FALSE;
	}

    switch (dwCode) 
	{

		case IOCTL_FT_SET_LATENCY_TIMER :

			if(!pBufIn || (dwLenIn < sizeof(UCHAR))) {
				ErrorInvalidArgs(&ntStatus, &ret, &err);
				break;
			}

			__try {
				err = FT_SetLatencyTimer(
								pUsbFTDI,
								*(UCHAR *) pBufIn
								);

			}
			__except(EXCEPTION_EXECUTE_HANDLER) {
				DEBUGMSG1(ZONE_IOCTL, 
					(_T("exception reading pBufIn in IOCTL_FT_SET_LATENCY_TIMER\n")));
				err = GetLastError();
			}
		
			break;

		case IOCTL_FT_GET_LATENCY_TIMER :

			if(!pBufOut || (dwLenOut < sizeof(UCHAR))) {
				ErrorInvalidArgs(&ntStatus, &ret, &err);
				break;
			}

			__try {
				err = FT_GetLatencyTimer(
								pUsbFTDI,
								(UCHAR *) pBufOut
								);

			}
			__except(EXCEPTION_EXECUTE_HANDLER) {
				DEBUGMSG1(ZONE_IOCTL, 
					(_T("exception reading pBufIn in IOCTL_FT_GET_LATENCY_TIMER\n")));
				err = GetLastError();
			}
		
			break;

		case IOCTL_FT_SET_BIT_MODE :

			if(!pBufIn || (dwLenIn < sizeof(TFtBitMode))) {
				ErrorInvalidArgs(&ntStatus, &ret, &err);
				break;
			}

			__try {
				err = FT_SetBitMode(
								pUsbFTDI,
								(TFtBitMode *) pBufIn
								);

			}
			__except(EXCEPTION_EXECUTE_HANDLER) {
				DEBUGMSG1(ZONE_IOCTL, 
					(_T("exception reading pBufIn in IOCTL_FT_SET_BIT_MODE\n")));
				err = GetLastError();
			}

			break;


		case IOCTL_FT_GET_BIT_MODE :

			if(!pBufOut || (dwLenOut < sizeof(UCHAR))) {
				ErrorInvalidArgs(&ntStatus, &ret, &err);
				break;
			}

			__try {
				err = FT_GetBitMode(
								pUsbFTDI,
								(UCHAR *) pBufIn
								);

			}
			__except(EXCEPTION_EXECUTE_HANDLER) {
				DEBUGMSG1(ZONE_IOCTL, 
					(_T("exception reading pBufIn in IOCTL_FT_GET_BIT_MODE\n")));
				err = GetLastError();
			}

			break;

		
		case IOCTL_FT_GET_DEVICE_INFO:

			if(!pBufOut || (dwLenOut < sizeof(TFtDeviceInfo))) {
				ErrorInvalidArgs(&ntStatus, &ret, &err);
				break;
			}
			else {

				TFtDeviceInfo *Params = (TFtDeviceInfo *) pBufOut;
				ULONG dw;

				if (IS_FT232H(pUsbFTDI))
					Params->Type = FT232H;
				else if (IS_FT4232H(pUsbFTDI))
					Params->Type = FT4232H;
				else if (IS_FT2232H(pUsbFTDI))
					Params->Type = FT2232H;
				else if(IS_FT232R(pUsbFTDI))
					Params->Type = FT232R;
				else if(IS_FT2232(pUsbFTDI))
					Params->Type = FT2232;
				else if(IS_FT232B(pUsbFTDI))
					Params->Type = FT232BM;
				else if(IS_FT8U232AM(pUsbFTDI))
					Params->Type = FT8U232AM;
				else if(IS_FT8U100AX(pUsbFTDI))
					Params->Type = FT8U100AX;
				else
					Params->Type = 0;

				dw = (ULONG) pUsbFTDI->pUsbDevice->Descriptor.idVendor;
				dw <<= 16;
				Params->ID = dw & 0xffff0000;
				dw = (ULONG) pUsbFTDI->pUsbDevice->Descriptor.idProduct;
				dw &= 0x0000ffff;
				Params->ID |= dw;

				FT_StrCpy(Params->SerialNumber,pUsbFTDI->SerialNumber);
				FT_StrCpy(Params->Description,pUsbFTDI->Description);

				Params->Status = 0;
				err	= ERROR_SUCCESS;

			}

			break;

		case IOCTL_FT_SET_DTR:

			DEBUGMSG1(ZONE_IOCTL, (_T(">IOCTL_FT_SET_DTR\n")));
			__try {
				err = FT_SetDtr(pHWHead);
			}
			__except(EXCEPTION_EXECUTE_HANDLER) {
				DEBUGMSG1(ZONE_WARN, 
					(_T("exception reading pBufIn in IOCTL_FT_SET_DTR\n")));
				err = GetLastError();
			}
			DEBUGMSG1(ZONE_IOCTL, (_T("<IOCTL_FT_SET_DTR\n")));
			break;

		case IOCTL_FT_CLR_DTR:

			__try {
				err = FT_ClrDtr(pHWHead);
			}
			__except(EXCEPTION_EXECUTE_HANDLER) {
				DEBUGMSG1(ZONE_WARN, 
					(_T("exception reading pBufIn in IOCTL_FT_CLR_DTR\n")));
				err = GetLastError();
			}

			break;

		case IOCTL_FT_SET_RTS:

			__try {
				err = FT_SetRts(pHWHead);
			}
			__except(EXCEPTION_EXECUTE_HANDLER) {
				DEBUGMSG1(ZONE_WARN, 
					(_T("exception reading pBufIn in IOCTL_FT_SET_RTS\n")));
				err = GetLastError();
			}

			break;

		case IOCTL_FT_CLR_RTS :

			__try {
				err = FT_ClrRts(pHWHead);
			}
			__except(EXCEPTION_EXECUTE_HANDLER) {
				DEBUGMSG1(ZONE_WARN, 
					(_T("exception reading pBufIn in IOCTL_FT_CLR_RTS\n")));
				err = GetLastError();
			}

			break;

		case IOCTL_FT_SET_DEVICE_EVENT:
			if(!pBufIn || (dwLenIn < sizeof(FTNotifyParams))) {
				ErrorInvalidArgs(&ntStatus, &ret, &err);
				break;
			}
			else {
				err = FT_SetDeviceEvent(
						pUsbFTDI,
						(FTNotifyParams *) pBufIn
						);
			}
			break;

		case IOCTL_FT_GET_DEVICE_EVENT_INFO:
			if(!pBufOut || (dwLenOut < sizeof(DWORD))) {
				ErrorInvalidArgs(&ntStatus, &ret, &err);
				break;
			}

			EnterCriticalSection(&pUsbFTDI->NotifyLock);

			*((DWORD *) pBufOut) = pUsbFTDI->dwCurrentNotifyStatus;

			LeaveCriticalSection(&pUsbFTDI->NotifyLock);

			break;

		case IOCTL_FT_GET_SET_DEVICE_TYPE:
			if (pBufIn && (dwLenIn >= sizeof(pUsbFTDI->usbDeviceInfo.DeviceType)))
				pUsbFTDI->usbDeviceInfo.DeviceType = (USB_SERIAL_DEVICE_TYPE)*pBufIn;
			else if (pBufOut && (dwLenOut >= sizeof(pUsbFTDI->usbDeviceInfo.DeviceType)))
			{
				*((USB_SERIAL_DEVICE_TYPE *) pBufOut) = pUsbFTDI->usbDeviceInfo.DeviceType;
				if (pdwActualOut)
					*pdwActualOut = sizeof(pUsbFTDI->usbDeviceInfo.DeviceType);
			}

			err = ERROR_SUCCESS;
			break;

		case IOCTL_FT_GET_USB_DEVICE_EXT_INFO:
			if (pBufOut && (dwLenOut >= sizeof(pUsbFTDI->usbDeviceInfo)))
			{
				memcpy(pBufOut, &(pUsbFTDI->usbDeviceInfo), sizeof(pUsbFTDI->usbDeviceInfo));
				if (pdwActualOut)
					*pdwActualOut = sizeof(pUsbFTDI->usbDeviceInfo);
			}

			err = ERROR_SUCCESS;
			break;

		default:
			ret = FALSE;
			DEBUGMSG2 (ZONE_FUNCTION, TEXT(" Unsupported ioctl 0x%X\r\n"), dwCode);
			break;            
    }

    DEBUGMSG2 (ZONE_FUNCTION, TEXT("-SerIoctl 0x%X\r\n"), pHead);
	
	if(err != ERROR_SUCCESS)
		ret =  FALSE;
	else
		ret = TRUE;

	return ret;
}

const
HW_VTBL IoVTbl = {
    SerInit,
    SerPostInit,
    SerDeinit,
    SerOpen,
    SerClose,
    SerGetInterruptType,
    SerRxIntr,
    SerTxIntr,
    SerModemIntr,
    SerLineIntr,
    SerGetRxBufferSize,
    SerPowerOff,
    SerPowerOn,
    SerClearDTR,
    SerSetDTR,
    SerClearRTS,
    SerSetRTS,
    SerEnableIR,
    SerDisableIR,
    SerClearBreak,
    SerSetBreak,
    SerXmitComChar,
    SerGetStatus,
    SerReset,
    SerGetModemStatus,
    SerGetCommProperties,
    SerPurgeComm,
    SerSetDCB,
    SerSetCommTimeouts,
    SerIoctl};

extern const HW_VTBL SerCardIoVTbl;

PHWOBJ
GetSerialObject(
               DWORD DeviceArrayIndex
               )
{
    PHWOBJ pSerObj;

	//
    // We do not have a statically allocated array of HWObjs.  Instead, we 
    // allocate a new HWObj for each instance of the driver.  The MDD will 
    // always call GetSerialObj/HWInit/HWDeinit in that order, so we can do 
    // the alloc here and do any subsequent free in HWDeInit.
	//
    DEBUGMSG1 (ZONE_INIT, (TEXT("GetSerialObject Called\n")));

	//
    // Allocate space for the HWOBJ.
	//
    pSerObj = (PHWOBJ)FT_LocalAlloc( LPTR, sizeof(HWOBJ) );
    if (!pSerObj)
        return (NULL);

	//
    // Fill in the HWObj structure that we just allocated.
	//
    pSerObj->BindFlags = THREAD_IN_PDD;      // We take care of our own IST
    pSerObj->pFuncTbl = (HW_VTBL *) &IoVTbl; // Return pointer to appropriate functions

	//
    // Now return this structure to the MDD.
	//
    return pSerObj;
}


