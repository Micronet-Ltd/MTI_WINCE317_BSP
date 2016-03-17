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
#ifndef OMAP35XX_MUSBOTG_H
#define OMAP35XX_MUSBOTG_H
#include <omap35xx_musbcore.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OTG_DRIVER (TEXT("MUSBOTG.DLL"))
    
struct _MUSB_FUNCS {
    DWORD (*ResetIRQ)(PVOID pContext);
    DWORD (*ResumeIRQ)(PVOID pContext);
    DWORD (*ProcessEP0)(PVOID pContext);
    DWORD (*ProcessEPx_RX)(PVOID pContext, DWORD endpoint);
    DWORD (*ProcessEPx_TX)(PVOID pContext, DWORD endpoint);
    DWORD (*Connect)(PVOID pContext);
    DWORD (*Disconnect)(PVOID pContext);
    DWORD (*Suspend)(PVOID pContext);
    DWORD (*ProcessDMA)(PVOID pContext, UCHAR channel);
};

typedef struct _MUSB_FUNCS MUSB_FUNCS, * PMUSB_FUNCS;

typedef LPVOID * LPLPVOID;
typedef BOOL (* LPMUSB_ATTACH_PROC)(PMUSB_FUNCS pFuncs, int mode, LPLPVOID lppvContext);
typedef BOOL (* LPMUSB_USBCLOCK_PROC)(BOOL fStart);

#define IDLE_MODE       0
#define DEVICE_MODE     1
#define HOST_MODE       2

#define A_DEVICE        0
#define B_DEVICE        1

#define CONN_CCS        (1<<0)
#define CONN_CSC        (1<<1)
#define CONN_DC         (1<<2)

#define DEFAULT_FIFO_ENDPOINT_0_SIZE    64 // this is according to MUSBMHDRC product spec, pg 13
#define MAX_DMA_CHANNEL 8

typedef struct {
    HANDLE  hReadyEvents[2];    // Event handle for Host Controller & Device
    PMUSB_FUNCS pFuncs[2];      // Function pointer for Host & device controller        
    PVOID       pContext[2];    // Pointer to the context storage for host & device controller
    PCSP_MUSB_OTG_REGS   pUsbOtgRegs;   //  Pointer to USB OTG Reg
    PCSP_MUSB_CSR_REGS   pUsbCsrRegs;   //  Pointer to the CSR registers (0x100 - 0x1FF)
    PCSP_MUSB_GEN_REGS   pUsbGenRegs;   //  Pointer to USB General Reg
    PCSP_MUSB_DMA_REGS   pUsbDmaRegs;   //  Pointer to USB DMA Reg  
    volatile UINT8       *pPadControlRegs;  // Pointer to Pad Control Reg
    volatile DWORD       *pSysControlRegs;  // Pointer to Control Register.
    UINT16              intr_rx;    // current interrupt RX
    UINT16              intr_tx;    // current interrupt TX
    UINT8               intr_usb;   // current control interrupt
    UINT8               operateMode;  // 0 - Idle State, 1 - Device, 2 - Host 
    UINT8               deviceType;  // 0 - A-device, 1 - B-device
    CRITICAL_SECTION    regCS; // Critical section for access control
    // connect_status is a single byte containing current connect status
    // information. Note that it should be update by OTG, not by Host nor Device
    //  D7      D6      D5      D4      D3      D2      D1
    // -----------------------------------------------------
    // |N/A     N/A     N/A     N/A    H_DC     CSC     CCS|
    // -----------------------------------------------------
    // CCS = current connect status
    // CSC = connect status change
    // H_DC = host disconnect complete
    UINT8               connect_status;    
    DWORD               dwSysIntr;
    HANDLE              hSysIntrEvent;
    HANDLE              hPowerEvent;
    HANDLE              hResumeEvent;
    BOOL                bClockStatus;
    DWORD               dwPwrMgmt; // Indicates USBOTG interface/functional clock status
    
} HSMUSB_T, *PHSMUSB_T;

#ifdef __cplusplus
}
#endif

#endif

