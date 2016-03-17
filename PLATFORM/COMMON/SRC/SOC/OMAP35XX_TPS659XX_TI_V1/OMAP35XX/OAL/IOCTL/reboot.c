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
//
//  File:  reboot.c
//
//  This file implement OMAP35XX SoC specific OALIoCtlHalReboot function.
//
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>
#include <oalex.h>
#include <oal_prcm.h>

extern OMAP_PRCM_PRM              *g_pPrcmPrm;

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalReboot
//
BOOL OALIoCtlHalReboot(
    UINT32 code, 
    VOID *pInpBuffer, 
    UINT32 inpSize, 
    VOID *pOutBuffer,
    UINT32 outSize, 
    UINT32 *pOutSize
    )
{
    OMAP_PRCM_GLOBAL_PRM_REGS *pGlobalPrmRegs = OALPAtoUA(OMAP_PRCM_GLOBAL_PRM_REGS_PA);
    OMAP_CONTEXT_RESTORE_REGS *pContextRestoreRegs = OALPAtoUA(OMAP_CONTEXT_RESTORE_REGS_PA);
    BOOL    bPowerOn = FALSE;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalReboot\r\n"));

    // Perform a global SW reset
    OALMSG(TRUE, (L"*** RESET ***\r\n"));

    // Disable KITL
#if (_WINCEOSVER<600)
    OALKitlPowerOff();
#else    
    KITLIoctl(IOCTL_KITL_POWER_CALL, &bPowerOn, sizeof(bPowerOn), NULL, 0, NULL);    
#endif

    // Clear context registers
    OUTREG32(&pContextRestoreRegs->BOOT_CONFIG_ADDR, 0);
    OUTREG32(&pContextRestoreRegs->PUBLIC_RESTORE_ADDR, 0);
    OUTREG32(&pContextRestoreRegs->SECURE_SRAM_RESTORE_ADDR, 0);
    OUTREG32(&pContextRestoreRegs->SDRC_MODULE_SEMAPHORE, 0);
    OUTREG32(&pContextRestoreRegs->PRCM_BLOCK_OFFSET, 0);
    OUTREG32(&pContextRestoreRegs->SDRC_BLOCK_OFFSET, 0);
    OUTREG32(&pContextRestoreRegs->OEM_CPU_INFO_DATA_PA, 0);
    OUTREG32(&pContextRestoreRegs->OEM_CPU_INFO_DATA_VA, 0);

    // Flush the cache
    OEMCacheRangeFlush( NULL, 0, CACHE_SYNC_ALL );

    // Do warm reset
    OUTREG32(&pGlobalPrmRegs->PRM_RSTCTRL, /*RSTCTRL_RST_DPLL3|*/ RSTCTRL_RST_GS);

    // Should never get to this point...
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalReboot\r\n"));
    return TRUE;
}

//------------------------------------------------------------------------------
