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
// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
//
//  File:  power.c
//
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>
#include <interrupt_struct.h>
#include <bus.h>
#include <oal_prcm.h>

//------------------------------------------------------------------------------
//
//  External:  g_pIntr
//
//  contains gpio and interrupt information.  Initialized in OALIntrInit()
//
extern OMAP_INTR_CONTEXT const *g_pIntr;
extern OMAP_SYSC_PADCONFS_WKUP_REGS *g_pSyscPadWkupRegs;

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
//  External:  _gpioClkId
//
//  device id's of all GPIO lines.
//
static 
OMAP_DEVICE _gpioClkId[] = {
    OMAP_DEVICE_GPIO1, 
    OMAP_DEVICE_GPIO2,
    OMAP_DEVICE_GPIO3,
    OMAP_DEVICE_GPIO4,
    OMAP_DEVICE_GPIO5,
    OMAP_DEVICE_GPIO6
};

//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptPending
//
//  This function returns true when given sysIntr interrupt is pending.
//
BOOL OEMInterruptPending(DWORD sysIntr)
{
    BOOL pending = 0;
    const UINT32 *pIrqs;
    UINT32 ix, count;

    if(OALIntrTranslateSysIntr(sysIntr, &count, &pIrqs))
	{
        for(ix = 0; ix < count && !pending; ix++)
		{
            pending = OALIntrIsIrqPending(pIrqs[ix]);
			OALMSGS(pending, (L"%S: %d pending\r\n", __FUNCTION__, pIrqs[ix]));
		}
	}

    return pending;
}

extern VOID BSPEnablePWRONSource(INT32 en);
//------------------------------------------------------------------------------
//
//  Function:  OEMPowerOff
//
//  Called when the system is to transition to it's lowest power mode (off)
//
VOID OEMPowerOff()
{
    int i = 0;
    UINT irq = 0;
    UINT32 sysIntr;
    UINT intr[3];
    UINT gpio[OMAP_GPIO_BANK_COUNT];
    UINT wkup[OMAP_GPIO_BANK_COUNT];
    UINT32 mask;
    BOOL bPowerOn;
    BOOL bPrevIntrState;

    // disable interrupts (note: this should not be needed)
	// Vladimir: already disabled
    bPrevIntrState = INTERRUPTS_ENABLE(0);

	OALMSGS(0, (L"OEMPowerOff: disable WD\r\n"));
    // UNDONE: verify if this is still necessary
    // Disable hardware watchdog
    OALWatchdogEnable(FALSE);
    
    // Make sure that KITL is powered off
    bPowerOn = 0;
    KITLIoctl(IOCTL_KITL_POWER_CALL, &bPowerOn, sizeof(bPowerOn), NULL, 0, NULL);    

    // Disable GPTimer2 (used for high perf/monte carlo profiling)
    PrcmDeviceEnableClocks(OMAP_DEVICE_GPTIMER2, FALSE);

    // Give chance to do board specific stuff
    BSPPowerOff();

	OALMSGS(1, (L"%S: mask interrupts\r\n", __FUNCTION__));
    //----------------------------------------------
    // capture all enabled interrupts and disable interrupts
    intr[0] = INREG32(&g_pIntr->pICLRegs->INTC_MIR0);
    intr[1] = INREG32(&g_pIntr->pICLRegs->INTC_MIR1);
    intr[2] = INREG32(&g_pIntr->pICLRegs->INTC_MIR2);

    OUTREG32(&g_pIntr->pICLRegs->INTC_MIR_SET0, OMAP_MPUIC_MASKALL);
    OUTREG32(&g_pIntr->pICLRegs->INTC_MIR_SET1, OMAP_MPUIC_MASKALL);
    OUTREG32(&g_pIntr->pICLRegs->INTC_MIR_SET2, OMAP_MPUIC_MASKALL);

	OALMSGS(0, (L"OEMPowerOff mask GPIO interrupts\r\n"));
    //----------------------------------------------
    // Context Save/Restore
    //  Mask all GPIO interrupts
    for(i = 0; i < OMAP_GPIO_BANK_COUNT; ++i)
	{
        gpio[i] = INREG32(&g_pIntr->pGPIORegs[i]->IRQENABLE1);
        wkup[i] = INREG32(&g_pIntr->pGPIORegs[i]->WAKEUPENABLE);
        OUTREG32(&g_pIntr->pGPIORegs[i]->IRQENABLE1, 0);
        OUTREG32(&g_pIntr->pGPIORegs[i]->WAKEUPENABLE, 0);
    }

	OALMSGS(0, (L"OEMPowerOff enable wake IO PADs\r\n"));
    //----------------------------------------------
    // Clear all enabled IO PAD wakeups for GPIOs
    for(i = 0; i < OMAP_GPIO_BANK_COUNT; ++i) 
	{
        irq = IRQ_GPIO_0 + (i * 32);
        mask = wkup[i];
        while(mask)
		{
            // If a GPIO was wakeup enabled, then clear the wakeup
            if(mask & 0x1)
                OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), 0);
            
            irq++;
            mask >>= 1;
		}
	}

	OALMSGS(0, (L"%S: enable wake sources\r\n", __FUNCTION__));
    //----------------------------------------------
    // Enable wake sources interrupts
    for(sysIntr = SYSINTR_FIRMWARE; sysIntr < SYSINTR_MAXIMUM; sysIntr++)
	{
        // Skip if sysIntr isn't allowed as wake source
        if (!OALPowerWakeSource(sysIntr)) 
		    continue;

        // Enable it as interrupt
        OEMInterruptEnable(sysIntr, 0, 0);

		OALMSGS(1, (L"%S: %d wake enabled\r\n", __FUNCTION__, sysIntr));
	}

    // enter full retention
	do
	{
		BSPEnablePWRONSource(1);
		PrcmSuspend();
		BSPEnablePWRONSource(0);
    
		OALMSGS(0, (L"%S: resuming... What is a reason?\r\n", __FUNCTION__));

		//----------------------------------------------
		// Find wakeup source
		for(sysIntr = SYSINTR_FIRMWARE; sysIntr < SYSINTR_MAXIMUM; sysIntr++)
		{            
			// Skip if sysIntr isn't allowed as wake source
			if(!OALPowerWakeSource(sysIntr)) 
				continue;

			// When this sysIntr is pending we find wake source
			if(OEMInterruptPending(sysIntr))
			{
				g_oalWakeSource = sysIntr;
				OALMSGS(1, (L"%S: (%d) is reason of resume\r\n", __FUNCTION__, g_oalWakeSource));
				break;
			}
		}

		if(sysIntr == SYSINTR_MAXIMUM)
		{
			OALMSGS(1, (L"OEMPowerOff Cannot find wake sources\r\n"));
		}
	}
	while(sysIntr == SYSINTR_MAXIMUM);

	OALMSGS(1, (L"%S: resume PERIPHERAL domain\r\n", __FUNCTION__));
   
	OALMSGS(0, (L"%S: un-mask GPIO interrupts\r\n", __FUNCTION__));
    //----------------------------------------------
    // Context Save/Restore
    //  Mask all GPIO interrupts
    for(i = 0; i < OMAP_GPIO_BANK_COUNT; ++i)
	{
        OUTREG32(&g_pIntr->pGPIORegs[i]->IRQENABLE1, gpio[i]);
        OUTREG32(&g_pIntr->pGPIORegs[i]->WAKEUPENABLE, wkup[i]);
	}

	OALMSGS(0, (L"%S: reenable IO PADs interrupts\r\n", __FUNCTION__));
    //-------------------------------------------------------
    // Enable all previously enabled IO PAD wakeups for GPIOs
    for(i = 0; i < OMAP_GPIO_BANK_COUNT; ++i) 
	{
        irq = IRQ_GPIO_0 + (i * 32);
        mask = wkup[i];
        while(mask)
		{
            // If a GPIO was wakeup enabled, then clear the wakeup
            if(mask & 0x1)
                OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), 1);
            
            irq++;
            mask >>= 1;
		}
	}
    
	OALMSGS(1, (L"%S: ummask interrupts\r\n", __FUNCTION__));
    //----------------------------------------------
    // Re-enable interrupts    
    OUTREG32(&g_pIntr->pICLRegs->INTC_MIR_CLEAR0, ~intr[0]);
    OUTREG32(&g_pIntr->pICLRegs->INTC_MIR_CLEAR1, ~intr[1]);
    OUTREG32(&g_pIntr->pICLRegs->INTC_MIR_CLEAR2, ~intr[2]);  
    
    //----------------------------------------------
    // Do board specific stuff    
    BSPPowerOn();    

    // Enable GPTimer2 (used for high perf/monte carlo profiling)
    PrcmDeviceEnableClocks(OMAP_DEVICE_GPTIMER2, TRUE);

    // Reinitialize KITL
    bPowerOn = 1;
    KITLIoctl(IOCTL_KITL_POWER_CALL, &bPowerOn, sizeof(bPowerOn), NULL, 0, NULL);    
    
	OALMSGS(0, (L"%S: enable WD\r\n", __FUNCTION__));
    // Enable hardware watchdog
    OALWatchdogEnable(1);
	
#ifndef SHIP_BUILD
#ifdef DEBUG_PRCM_SUSPEND_RESUME
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
#endif

    // restore interrupts
    INTERRUPTS_ENABLE(bPrevIntrState);
}

//------------------------------------------------------------------------------
