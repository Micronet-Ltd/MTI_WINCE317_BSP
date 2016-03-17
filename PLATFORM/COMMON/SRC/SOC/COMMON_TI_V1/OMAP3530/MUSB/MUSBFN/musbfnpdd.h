// All rights reserved ADENEO EMBEDDED 2010
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//------------------------------------------------------------------------------
//
//  File:  musbfnpdd.h
//
//  This file contains USB function PDD header.
//
#ifndef __MUSBFNPDD_H
#define __MUSBFNPDD_H

#include <dvfs.h>

// Setup command to support for Mass Storage Devices
// Feature request for Endpoints 
#define ENDPOINT_SET_FEATURE_HALT     0       /* Force Endpoint STALL */
#define ENDPOINT_SET_FEATURE_DIR_IN   0x80

#define DEVICE_CONTEXT  (DEVICE_MODE-1)
#define HOST_CONTEXT    (HOST_MODE-1)

// ep0State values:  EP0 state
enum {
    EP0_ST_SETUP,
    EP0_ST_SETUP_PROCESSED,
    EP0_ST_STATUS_IN,
    EP0_ST_STATUS_OUT,
    EP0_ST_DATA_RX,
    EP0_ST_DATA_TX,
    EP0_ST_ERROR
};

// endPoint transfer type
enum {
    EP_TYPE_CTRL = 0,
    EP_TYPE_ISOCH,
    EP_TYPE_BULK,
    EP_TYPE_INTERRUPT
};
    
// endPoint transfer type
typedef enum _USBFN_STATE {
    USBFN_IDLE = 0,
    USBFN_ACTIVE
}USBFN_STATE;



// DMA channel assignment for endpoints
#define MUSB_TX_DMA_CHN  0
#define MUSB_RX_DMA_CHN  1


// PDD internal devState flags    
#define MUSB_DEVSTAT_B_HNP_ENABLE   (1 << 9)        // Host Enable HNP
#define MUSB_DEVSTAT_A_HNP_SUPPORT  (1 << 8)        // Host Support HNP
#define MUSB_DEVSTAT_A_ALT_HNP_SUPPORT (1 << 7)     // Host Support HNP on Alter Port.
#define MUSB_DEVSTAT_R_WK_OK        (1 << 6)        // Remote wakeup enabled
#define MUSB_DEVSTAT_USB_RESET      (1 << 5)        // USB device reset
#define MUSB_DEVSTAT_SUS            (1 << 4)        // Device suspended
#define MUSB_DEVSTAT_CFG            (1 << 3)        // Device configured
#define MUSB_DEVSTAT_ADD            (1 << 2)        // Device addresses       
#define MUSB_DEVSTAT_DEF            (1 << 1)        // Device defined
#define MUSB_DEVSTAT_ATT            (1 << 0)        // Device attached

#define DVFS_NODMA     0
#define DVFS_PREDMA    1
#define DVFS_POSTDMA   2


//------------------------------------------------------------------------------
//
//  Type:  MusbFnEp_t
//
//  Internal PDD structure which holds info about endpoint direction,
//  max packet size and active transfer.
//
typedef struct {
    WORD maxPacketSize;
    BOOL dirRx;
    BOOL zeroLength;
    BOOL bMassStore;

    BOOL dmaEnabled;
    BOOL stalled;
    BOOL bZeroLengthSent;
    DWORD dwRemainBuffer;
    STransfer *pTransfer;
    DWORD dwRxDMASize;
    BOOL bTxDMAShortPacket;
    BYTE  dmaDVFSstate;

    BOOL bLastRxUsedDMA;    
} MUsbFnEp_t;

enum {
    MODE_DMA = 0,    
    MODE_FIFO
};
//------------------------------------------------------------------------------
//
//  Type:  MUsbFnPdd_t
//
//  Internal PDD context structure.
//
typedef struct {

    DWORD memBase;
    DWORD memLen;
    DWORD enableDMA;
    DWORD rx0DmaEp;
    DWORD tx0DmaEp;
    DWORD rx0DmaBufferSize;
    DWORD tx0DmaBufferSize;
    DWORD rx0DmaState;

    PHSMUSB_T pUSBContext;
    
    VOID *pMddContext;
    PFN_UFN_MDD_NOTIFY pfnNotify;
    LPMUSB_USBCLOCK_PROC pfnEnUSBClock;

    HANDLE hParentBus;

    CRITICAL_SECTION dmaCS;
    UCHAR* pDmaRx0Buffer;
    DWORD  paDmaRx0Buffer;
    UCHAR* pDmaTx0Buffer;
    DWORD  paDmaTx0Buffer;

    PVOID  pCachedDmaTx0Buffer;

    DWORD sysIntr;

    CEDEVICE_POWER_STATE pmPowerState;
    CEDEVICE_POWER_STATE selfPowerState;
    CEDEVICE_POWER_STATE actualPowerState;

    DWORD devState;
    BOOL selfPowered;
    WORD setupCount;

    CRITICAL_SECTION epCS;

    CRITICAL_SECTION powerStateCS;
    HANDLE hPowerDownEvent;
    BOOL   fPowerDown;
    USBFN_STATE dwUSBFNState;

    MUsbFnEp_t ep[USBD_EP_COUNT];

    DWORD ep0State;    
    DWORD intr_rx_data_avail ;
    UCHAR   ucAddress;
    BOOL    bSetAddress;
    // Add for DVFS
    DWORD   nDVFSOrder;
    BOOL bDVFSActive;
    BOOL bDVFSAck;
    LONG  nActiveDmaCount;
    HANDLE hDVFSAckEvent;
    HANDLE hDVFSActivityEvent;
    CRITICAL_SECTION csDVFS;
    HANDLE  hVbusChargeEvent;

    // Keep track of endpoints for mass storage devices
    WORD FsEpMassStoreFlags;
    WORD HsEpMassStoreFlags;

    BOOL bDMAForRX;
    BOOL bTXIsUsingUsbDMA;
    BOOL bRXIsUsingUsbDMA;
} MUsbFnPdd_t;

//-------------------------------------------------------------------------

#endif

