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
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
#ifndef OMAP35XX_MUSBHCD_H
#define OMAP35XX_MUSBHCD_H

#define USB_ENDPOINT(p) (p&0xf)
#define HOST_MAX_EP         31 // EP0 - EP15, only EP0 is bi-dir, EP1-15 has IN, OUT
#define DMA_MAX_CHANNEL     8
#define HOST_MAX_EPNUM      16
#define HCD_DMA_SUPPORT (TEXT("Dma"))
#define HCD_DVFS_ORDER  (TEXT("DVFSOrder"))

enum {
    MUSB_READ_SUCCESS = 0,
    MUSB_READ_MORE_DATA,
    MUSB_WAIT_DMA_COMPLETE
};
typedef struct _SMHCDPdd
{
    LPVOID lpvMemoryObject;
    LPVOID lpvMHCDMddObject;
    PVOID pvVirtualAddress;                        // DMA buffers as seen by the CPU
    DWORD dwPhysicalMemSize;
    PHYSICAL_ADDRESS LogicalAddress;        // DMA buffers as seen by the DMA controller and bus interfaces
    DMA_ADAPTER_OBJECT AdapterObject;
    TCHAR szDriverRegKey[MAX_PATH];
    PUCHAR ioPortBase;
    DWORD dwSysIntr;
    CRITICAL_SECTION csPdd;                     // serializes access to the PDD object
    HANDLE          IsrHandle;
    HANDLE hParentBusHandle;    
    BOOL    bDMASupport;
    DWORD   dwDMAMode;
    LPMUSB_USBCLOCK_PROC m_lpUSBClockProc;
// DVFS variables
    UINT nDVFSOrder;
    BOOL bDVFSActive;
    LONG  nActiveDmaCount;
    HANDLE hDVFSAckEvent;
    HANDLE hDVFSActivityEvent;
    CRITICAL_SECTION csDVFS;

} SMHCDPdd;


#endif

