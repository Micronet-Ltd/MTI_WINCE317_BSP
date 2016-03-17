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
//
//  File:  power.c
//
#include "omap.h"
#include "oalex.h"
#include "omap_prof.h"
#include "am3517.h"
#include "omap_led.h"
#include "interrupt_struct.h"
#include "oal_prcm.h"
#include "oal_gptimer.h"
#include "oal_clock.h"
#include "bsp_cfg.h"

#include <nkintr.h>

//------------------------------------------------------------------------------
//
//  Global: g_oalTimer
//
//  This is global instance of timer control block.
//
extern OMAP_TIMER_CONTEXT      g_oalTimerContext;
//-----------------------------------------------------------------------------
//
//  Global:  g_pPrcmPrm
//
//  Reference to all PRCM-PRM registers. Initialized in PrcmInit
//
extern OMAP_PRCM_PRM   *g_pPrcmPrm;

//------------------------------------------------------------------------------
//
//  External:  g_pIntr
//
//  contains gpio and interrupt information.  Initialized in OALIntrInit()
//
extern OMAP_INTR_CONTEXT const *g_pIntr;

extern VOID PrcmDumpSavedRefCounts();
extern void DumpPrcmRegsSnapshot();
extern BOOL g_PrcmDebugSuspendResume;

//------------------------------------------------------------------------------
//
//  External:  OALIntrIsIrqPending
//
//  Checks if the given interrupt is pending.
//
BOOL OALIntrIsIrqPending(UINT32 irq);


//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptPending
//
//  This function returns true when given sysIntr interrupt is pending.
//
BOOL OEMInterruptPending(DWORD sysIntr)
{
    BOOL pending = FALSE;
    const UINT32 *pIrqs;
    UINT32 ix, count;

    OALMSG(OAL_INTR&&OAL_VERBOSE, (
        L"+OEMInterruptPending(%d)\r\n", sysIntr
        ));

    if (OALIntrTranslateSysIntr(sysIntr, &count, &pIrqs))
        {
        for (ix = 0; ix < count && !pending; ix++)
            {
            pending = OALIntrIsIrqPending(pIrqs[ix]);
            }            
        }
        
    OALMSG(OAL_INTR&&OAL_VERBOSE, (
        L"-OEMInterruptPending(rc = %d)\r\n", pending
        ));
    return pending;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMPowerOff
//
//  Called when the system is to transition to it's lowest power mode (off)
//
VOID
OEMPowerOff(
    )
{
    DWORD i;
    UINT32 sysIntr;
    UINT intr[3];
    BOOL bPowerOn;

    // UNDONE: verify if this is still necessary
    // Disable hardware watchdog
    OALWatchdogEnable(FALSE);
    
    // Make sure that KITL is powered off
    bPowerOn = FALSE;
    KITLIoctl(IOCTL_KITL_POWER_CALL, &bPowerOn, sizeof(bPowerOn), NULL, 0, NULL);    

    // Disable GPTimer2 (used for high perf/monte carlo profiling)
    EnableDeviceClocks(BSPGetGPTPerfDevice(), FALSE);

    // Give chance to do board specific stuff
    BSPPowerOff();

    //----------------------------------------------
    // capture all enabled interrupts and disable interrupts
    intr[0] = INREG32(&g_pIntr->pICLRegs->INTC_MIR0);
    intr[1] = INREG32(&g_pIntr->pICLRegs->INTC_MIR1);
    intr[2] = INREG32(&g_pIntr->pICLRegs->INTC_MIR2);

    OUTREG32(&g_pIntr->pICLRegs->INTC_MIR_SET0, OMAP_MPUIC_MASKALL);
    OUTREG32(&g_pIntr->pICLRegs->INTC_MIR_SET1, OMAP_MPUIC_MASKALL);
    OUTREG32(&g_pIntr->pICLRegs->INTC_MIR_SET2, OMAP_MPUIC_MASKALL);

    //----------------------------------------------
    // Context Save/Restore       
	// Save state then mask all GPIO interrupts
	for (i=0; i<g_pIntr->nbGpioBank; i++)
    {
		INTR_GPIO_CTXT* pCurrGpioCtxt = &g_pIntr->pGpioCtxt[i];

		// Save current state
		pCurrGpioCtxt->restoreCtxt.IRQENABLE1 = INREG32(&pCurrGpioCtxt->pRegs->IRQENABLE1);
		pCurrGpioCtxt->restoreCtxt.WAKEUPENABLE = INREG32(&pCurrGpioCtxt->pRegs->WAKEUPENABLE);

		// Disable all GPIO interrupts in the bank
        OUTREG32(&pCurrGpioCtxt->pRegs->IRQENABLE1, 0);
        OUTREG32(&pCurrGpioCtxt->pRegs->WAKEUPENABLE, 0);

        OALIntrEnableIrqs(1,&pCurrGpioCtxt->bank_irq);
	}

/*
    //----------------------------------------------
    // Clear all enabled IO PAD wakeups for GPIOs
    for (i = 0; i < g_pIntr->nbGpioBank; ++i) 
        {
        irq = g_pIntr->pGpioCtxt[i].irq_start;
        mask = wkup[i];
        while (mask != 0)
            {
            // If a GPIO was wakeup enabled, then clear the wakeup
            if (mask & 0x1)
                {
                OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), FALSE);
                }
            
            irq++;
            mask >>= 1;    
            }
        }
*/

    //----------------------------------------------
    // Enable wake sources interrupts
    for (sysIntr = SYSINTR_DEVICES; sysIntr < SYSINTR_MAXIMUM; sysIntr++)
        {
        // Skip if sysIntr isn't allowed as wake source
        if (!OALPowerWakeSource(sysIntr)) 
		    continue;

        // Enable it as interrupt
        OEMInterruptEnable(sysIntr, NULL, 0);
        }

    // enter full retention
    PrcmSuspend();
    
    //----------------------------------------------
    // Find wakeup source
    for (sysIntr = SYSINTR_DEVICES; sysIntr < SYSINTR_MAXIMUM; sysIntr++)
        {            
        // Skip if sysIntr isn't allowed as wake source
        if (!OALPowerWakeSource(sysIntr)) 
		    continue;

        // When this sysIntr is pending we find wake source
        if (OEMInterruptPending(sysIntr))
            {
            g_oalWakeSource = sysIntr;
            break;
            }
        }
  
    //----------------------------------------------
    // Context Save/Restore
    // Put GPIO interrupt state back to the way it was before suspend
    for (i=0; i<g_pIntr->nbGpioBank; i++)
    {
		INTR_GPIO_CTXT* pCurrGpioCtxt = &g_pIntr->pGpioCtxt[i];		

        // Write registers with the previously saved values
        OUTREG32(&pCurrGpioCtxt->pRegs->IRQENABLE1, pCurrGpioCtxt->restoreCtxt.IRQENABLE1);
        OUTREG32(&pCurrGpioCtxt->pRegs->WAKEUPENABLE, pCurrGpioCtxt->restoreCtxt.WAKEUPENABLE);

    }

/*
    //-------------------------------------------------------
    // Enable all previously enabled IO PAD wakeups for GPIOs
    for (i = 0; i < g_pIntr->nbGpioBank; ++i) 
        {
        irq = g_pIntr->pGpioCtxt[i].irq_start;
        mask = wkup[i];
        while (mask != 0)
            {
            // If a GPIO was wakeup enabled, then clear the wakeup
            if (mask & 0x1)
                {
                OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), TRUE);
                }
            
            irq++;
            mask >z>= 1;    
            }
        }
*/

    //----------------------------------------------
    // Re-enable interrupts    
    OUTREG32(&g_pIntr->pICLRegs->INTC_MIR_CLEAR0, ~intr[0]);
    OUTREG32(&g_pIntr->pICLRegs->INTC_MIR_CLEAR1, ~intr[1]);
    OUTREG32(&g_pIntr->pICLRegs->INTC_MIR_CLEAR2, ~intr[2]);  
    
    //----------------------------------------------
    // Do board specific stuff    
    BSPPowerOn();    

    // Enable GPTimer (used for high perf/monte carlo profiling)
    EnableDeviceClocks(BSPGetGPTPerfDevice(), TRUE);

    // Reinitialize KITL
    bPowerOn = TRUE;
    KITLIoctl(IOCTL_KITL_POWER_CALL, &bPowerOn, sizeof(bPowerOn), NULL, 0, NULL);    
    
    // Enable hardware watchdog
    OALWatchdogEnable(TRUE);
	
#ifndef SHIP_BUILD
    if (g_PrcmDebugSuspendResume)
	{
        OALMSG(1, (L"Enable wake sources:\r\n"));
        for (sysIntr = SYSINTR_FIRMWARE; sysIntr < SYSINTR_MAXIMUM; sysIntr++)
        {
            if (OALPowerWakeSource(sysIntr)) 
                OALMSG(1, (L"  SYSINTR %d\r\n", sysIntr));
        }

    	OALMSG(1, (L"\r\nWake due to SYSINTR %d\r\n", g_oalWakeSource));
        OALWakeupLatency_DumpSnapshot();
        PrcmDumpSavedRefCounts();
        DumpPrcmRegsSnapshot();
    }
#endif
}

//------------------------------------------------------------------------------
