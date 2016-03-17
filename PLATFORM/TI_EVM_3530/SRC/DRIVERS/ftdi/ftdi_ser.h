/*++

Copyright (c) 2001-2004  Future Technology Devices International Ltd.

Module Name:

    ftdi_ser.h

Abstract:

    Native USB device driver for FTDI FT8U2XX
    Serial port routines

Environment:

    kernel mode only

Revision History:

	24/05/04	st		Adapted from Microsoft sample
	28/05/04	st		Add FTDI context structure to SER_INFO.


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
   SER_PDD.h
   
Abstract:  
   Holds definitions for sample 16550  serial interface.
   
Notes: 
--*/
#ifndef __SER_PDD_H__   
#define __SER_PDD_H__

#define PC_REG_DEVINDEX_VAL_NAME TEXT("DeviceArrayIndex") 
#define PC_REG_DEVINDEX_VAL_LEN  sizeof( DWORD )
#define PC_REG_IRQ_VAL_NAME TEXT("IRQ") 
#define PC_REG_IRQ_VAL_LEN  sizeof( DWORD )

#define PC_REG_IOBASE_VAL_NAME TEXT("IoBase") 
#define PC_REG_IOBASE_VAL_LEN  sizeof( DWORD )
#define PC_REG_IOLEN_VAL_NAME TEXT("IoLen") 
#define PC_REG_IOLEN_VAL_LEN  sizeof( DWORD )

//
// Signals from the BULK_IN task
//
#define LINE_STATUS_INT_MASK		(1 << 5)
#define RX_INT_MASK				(1 << 2)
#define TX_INT_MASK				(1 << 1)
#define MODEM_INT_MASK			(1 << 0)

#ifdef __cplusplus
extern "C" {
#endif

// Steal the useless IR zone from MDD and use it as EP0 & USB specific stuff
#define ZONE_USB ZONE_IR

// Strict timing requirements at enumeration.  Use a relatively high priority
#define DEFAULT_THREAD_PRIO 100

 // We use a callback for serial events
typedef VOID		(*EVENT_FUNC)(PVOID Arg1, ULONG Arg2);

// Here is the callback for serial events
typedef VOID (*PFN_SER_EVENT) (
    PVOID pHandle,              // PHW_INDEP_INFO, but pdd doesn't know it
    UINT32 events               // What events where encountered?
    );

/*
 * @doc HWINTERNAL
 * @struct SER_INFO | Private structure.
 */

typedef struct __SER_INFO {
     // Keep a copy of DCB since we rely on may of its parms
    DCB         dcb;        // @field Device Control Block (copy of DCB in MDD)

     // And the same thing applies for CommTimeouts
    COMMTIMEOUTS CommTimeouts;  // @field Copy of CommTimeouts structure
    ULONG		CommErrors;		// @field Bitfield representing Win32 comm error status. 
    ULONG		ModemStatus;	// @field Bitfield representing Win32 modem status. 
    CRITICAL_SECTION	HwRegCritSec;    // @field Protects NET2890 registers from non-atomic access (addr/data pairs)
    CRITICAL_SECTION	TransmitCritSec; // @field Protects UART TX FIFO from simultaneous access
    ULONG		OpenCount;	    // @field Count of simultaneous opens. 
    ULONG		DroppedBytes;	// @field Number of dropped bytes 
    COMSTAT		Status; 		// @field Bitfield representing Win32 comm status. 
    HANDLE		FlushDone;		// @field Handle to flush done event.

	// We have our own dispatch thread.
	HANDLE			pDispatchThread;// @field ReceiveThread 
	DWORD			KillRxThread:1;	// @field Flag to terminate NET2890DispatchThread.
	HANDLE          hSerialEvent;   // @field Interrupt event
	
    // now hardware specific goodies
    DWORD       dwIOBase;       // @field IO Base Address - unmapped
    DWORD       dwIOLen;        // @field IO Length
    DWORD       dwIRQ;          // @field Interrupt number for this peripheral
    DWORD       dwDevIndex;     // @field Index of device
	WORD		wSOFStableCnt;  // @field How many iterations without SOF
    BOOL		dReq;			// @field USB endpoint 0 command
    BOOL        bFlowedOff;     // @field Are we spoofing flowed off state?
	BYTE		cCtrlLines;       // Emulated state of DTR & RTS
    BYTE        cIntStat;       // @field Last known interrupt status
    BYTE 		dConfIdx; 		// @field USB Configuration Index
    BYTE 		dInterface; 	// @field USB Interface Index
    BYTE 		dSetting; 		// @field USB Setting Index
    BYTE		dAddress;		// @field USB device Address
    UINT8       cOpenCount;     // @field Count of concurrent opens
    COMMPROP    CommProp;       // @field Pointer to CommProp structure.
    PVOID       pMddHead;       // @field First arg to mdd callbacks.
    PHWOBJ      pHWObj;         // @field Pointer to PDDs HWObj structure
//    PBOOT_ARGS  pBootArgs;      // @field Pointer to global boot args struct
	// start FTDI specific
	PUCHAR		currentPktPtr;	// Pointer to data to send in response to EP0 in token
	ULONG		pktBufferRemain;// Number of EP0 bytes remaining to send
	BOOL		packetQueued;	// Whether the entire packet has been sent
	PVOID		pFTDICtxt;		// FTDI specific context structure (from lpvBusContext passed to COM_Init)
	// end FTDI specific

} SER_INFO, *PSER_INFO;

 // And now, all the function prototypes
PVOID
SerInit(
    ULONG   Identifier,
    PVOID   pMddHead,
    PHWOBJ  pHWObj
    );
BOOL SerPostInit(
    PVOID   pHead 
    );
BOOL SerDeinit(
    PVOID   pHead 
    );
BOOL SerOpen(
    PVOID   pHead 
    );
ULONG SerClose(
    PVOID   pHead
    );
VOID SerClearDTR(
    PVOID   pHead 
    );
VOID SerSetDTR(
    PVOID   pHead 
    );
VOID SerClearRTS(
    PVOID   pHead 
    );
VOID SerSetRTS(
    PVOID   pHead 
    );
VOID SerClearBreak(
    PVOID   pHead 
    );
VOID SerSetBreak(
    PVOID   pHead 
    );
VOID SerClearBreak(
    PVOID   pHead 
    );
VOID SerSetBreak(
    PVOID   pHead
    );
ULONG SerGetByteNumber(
    PVOID   pHead	     
    );
VOID SerDisableXmit(
    PVOID   pHead	
    );
VOID SerEnableXmit(
    PVOID   pHead	
    );
BOOL SerSetDCB(
    PVOID   pHead,	
    LPDCB   lpDCB       //     Pointer to DCB structure
    );
ULONG SerSetCommTimeouts(
    PVOID   pHead,	
    LPCOMMTIMEOUTS   lpCommTimeouts //  Pointer to CommTimeout structure
    );
ULONG SerGetRxBufferSize(
    PVOID pHead
    );
INTERRUPT_TYPE SerGetInterruptType(
    PVOID pHead
    );
ULONG SerRxIntr(
    PVOID pHead,
    PUCHAR pRxBuffer,       // Pointer to receive buffer
    ULONG *pBufflen         //  In = max bytes to read, out = bytes read
    );
VOID SerTxIntr(
    PVOID pHead,
    PUCHAR pTxBuffer,
    ULONG *pBufflen
    );
VOID SerLineIntr(
    PVOID pHead
    );
VOID SerModemIntr(
    PVOID pHead 
    );
ULONG SerGetStatus(
    PVOID	pHead,
    LPCOMSTAT	lpStat	// Pointer to LPCOMMSTAT to hold status.
    );
VOID SerReset(
    PVOID   pHead
    );
VOID SerGetModemStatus(
    PVOID   pHead,
    PULONG  pModemStatus    //  PULONG passed in by user.
    );
VOID SerPurgeComm(
    PVOID   pHead,
    DWORD   fdwAction	    //  Action to take. 
    );
BOOL SerXmitComChar(
    PVOID   pHead,
    UCHAR   ComChar   //  Character to transmit. 
    );
BOOL SerPowerOn(
    PVOID   pHead
    );
BOOL SerPowerOff(
    PVOID   pHead
    );
BOOL SerIoctl(
    PVOID pHead,
    DWORD dwCode,
    PBYTE pBufIn,
    DWORD dwLenIn,
    PBYTE pBufOut,
    DWORD dwLenOut,
    PDWORD pdwActualOut);
DWORD DelayedDeviceDeactivate(
    IN PVOID Context
    );
#ifdef __cplusplus
}
#endif


#endif __SER_PDD_H__
