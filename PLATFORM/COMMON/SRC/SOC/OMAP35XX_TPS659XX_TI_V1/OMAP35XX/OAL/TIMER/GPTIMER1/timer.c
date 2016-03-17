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
//  File:  timer.c
//
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>
#include <bus.h>
#include <oal_kitlex.h>
#include <oal_prcm.h>
#include <oal_i2c.h>
#include <omap_prof.h>

//-----------------------------------------------------------------------------
#define DELTA_TIME              20          // In TICK
#ifndef MAX_INT
#define MAX_INT                 0x7FFFFFFF
#endif

//#define INTR_TEST_POINT

// ##### Michael. Init GPIO_62 for test point purposes
// ##### Define GPIO_62 as GPIO in 'platform.c' of Xloader
#ifdef INTR_TEST_POINT
 #include <ceddkex.h>
 #include <gpio.h>
 #include "omap35xx.h"
 #include <interrupt_struct.h>

 #define GPIO_BITS_PER_BANK      (0x1F)
 #define GPIO_BANK(x)            (x >> 5)
 #define GPIO_BIT(x)             (x & GPIO_BITS_PER_BANK)
 #define TEST_BIT                GPIO_BIT(GPIO_62)
 #define TEST_BANK               GPIO_BANK(GPIO_62)
 #define TEST_MASK               (1 << (TEST_BIT))

 extern OMAP_INTR_CONTEXT const    *g_pIntr;

 #define TEST_GPIO_LOW           (CLRREG32(&g_pIntr->pGPIORegs[TEST_BANK]->DATAOUT, 1 << (TEST_BIT)))
 #define TEST_GPIO_HIGH          (SETREG32(&g_pIntr->pGPIORegs[TEST_BANK]->DATAOUT, 1 << (TEST_BIT)))
#else
 #define TEST_GPIO_LOW            
 #define TEST_GPIO_HIGH           
#endif /* INTR_TEST_POINT */



//------------------------------------------------------------------------------
//
//  typedef: OMAP_TIMER_CONTEXT
//
//  Structure maintaining system timer and tick count values
//

typedef struct OMAP_TIMER_CONTEXT {

    UINT32 maxPeriodMSec;               // Maximal timer period in MSec
    UINT32 margin;                      // Margin of time need to reprogram timer interrupt

    volatile UINT64 curCounts;          // Counts at last system tick
    volatile UINT32 base;               // Timer value at last interrupt
    volatile UINT32 match;              // Timer match value for next interrupt

} OMAP_TIMER_CONTEXT;

//------------------------------------------------------------------------------
//
//  External: g_oalTimerIrq
//
//  This variable is defined in interrupt module and it is used in interrupt
//  handler to distinguish system timer interrupt.
//
extern UINT32           g_oalTimerIrq;

//------------------------------------------------------------------------------
//
//  Global:  dwOEMMaxIdlePeriod
//
//  maximum idle period during OS Idle in milliseconds
//
extern DWORD dwOEMMaxIdlePeriod;

//------------------------------------------------------------------------------
//
//  Global: g_pTimerRegs
//
//  This is global instance of timer registers
//
OMAP_GPTIMER_REGS*      g_pTimerRegs;

//------------------------------------------------------------------------------
//
//  Global: g_oalTimer
//
//  This is global instance of timer control block.
//
OMAP_TIMER_CONTEXT      g_oalTimerContext;

//-----------------------------------------------------------------------------
//
//  Global:  g_pPrcmPrm
//
//  Reference to all PRCM-PRM registers. Initialized in PrcmInit
//
extern OMAP_PRCM_PRM   *g_pPrcmPrm;

//-----------------------------------------------------------------------------
//
//  Global:  g_pPrcmCm
//
//  Reference to all PRCM-CM registers. Initialized in PrcmInit
//
extern OMAP_PRCM_CM    *g_pPrcmCm;

//-----------------------------------------------------------------------------
//
//  Global:  g_wakeupLatencyConstraintTickCount
//
//  latency time, in 32khz ticks, associated with current latency state
//
INT g_wakeupLatencyConstraintTickCount = MAX_INT;

//------------------------------------------------------------------------------
//
//  Define: MSEC / TICK conversions

//  Conversions for 32kHz clock

//#define TICKS_PER_MSEC      67043   // s6.13 (32.768 * 2^13)
//#define MSEC_TO_TICK(msec)  (((msec) * TICKS_PER_MSEC) >> 13) // s12.0 * s6.13 ==> s18.13 >> 13 ==> s18.0
#define MSEC_TO_TICK(msec)  (((msec) << 12)/125 + 1)      // msec * 32.768
#define TICK_TO_MSEC(tick)     (((tick) * 1000) >> 15)    // msec / 32.768

//------------------------------------------------------------------------------
UINT32
OEMGetTickCount(
    );

VOID
UpdatePeriod(
    UINT32 periodMSec
    );

//------------------------------------------------------------------------------
//
//  Function:  OALTimerSetCompare
//
__inline VOID OALTimerSetCompare(UINT32 compare)
{

    OUTREG32(&g_pTimerRegs->TMAR, compare);
    while ((INREG32(&g_pTimerRegs->TWPS) & GPTIMER_TWPS_TMAR) != 0);

    // make sure we don't set next timer interrupt to the past
    //
    if (compare < INREG32(&g_pTimerRegs->TCRR)) UpdatePeriod(1);
}


//------------------------------------------------------------------------------
//
//  Function:  UpdatePeriod
//
VOID
UpdatePeriod(
    UINT32 periodMSec
    )
{
    UINT32 period, match;
    INT32 delta;
    UINT64 offsetMSec = periodMSec;
    UINT64 tickCount = OALGetTickCount();
//    INT nDelay;

    // Calculate count difference
    period = (UINT32)MSEC_TO_TICK(offsetMSec);

//    nDelay = min(period, DELTA_TIME);
    // This is compare value
//    match = ((UINT32)MSEC_TO_TICK(tickCount)) + nDelay;
    match = ((UINT32)MSEC_TO_TICK(tickCount)) + period + DELTA_TIME;

    delta = (INT32)(OALTimerGetCount()+ g_oalTimerContext.margin - match);

    // If we are behind, issue interrupt as soon as possible
    if (delta > 0)
    {
        match += MSEC_TO_TICK(1);
    }

    // Save off match value
    g_oalTimerContext.match = match;

    // Set timer match value
    OALTimerSetCompare(match);
}



//------------------------------------------------------------------------------
//
//  Function:     OEMIdle
//
//  This function is called by the kernel when there are no threads ready to
//  run. The CPU should be put into a reduced power mode if possible and halted.
//  It is important to be able to resume execution quickly upon receiving an
//  interrupt.
//
//  Interrupts are disabled when OEMIdle is called and when it returns.
//
//  This implementation doesn't change system tick. It is intend to be used
//  with variable tick implementation. However it should work with fixed
//  variable tick implementation also (with lower efficiency because maximal
//  idle time is 1 ms).
//
//  WARNING: This function is called from deep within the kernel, it cannot make
//  any system calls, use any critical sections, or debug messages.
//
VOID OEMIdle(DWORD idleParam)
{
    static UINT _max = 0;
    static UINT _count = 0;

    INT delta;
    UINT tcrrTemp;
    UINT tcrrEnter, tcrrExit;
    UINT idleDelta, newIdleLow;
    INT wakeupDelay;
    INT maxDelay;
    DWORD latencyState;
    DWORD wakeStatus;
    DWORD prevCoreState;
    DWORD prevMpuState;
    DWORD prevPerState;
    DWORD prevChipState;

    OUTREG32(&g_pPrcmPrm->pOMAP_CORE_PRM->PM_PREPWSTST_CORE, 0);
    OUTREG32(&g_pPrcmPrm->pOMAP_MPU_PRM->PM_PREPWSTST_MPU, 0);
    OUTREG32(&g_pPrcmPrm->pOMAP_PER_PRM->PM_PREPWSTST_PER, 0);
    OUTREG32(&g_pPrcmPrm->pOMAP_CAM_PRM->PM_PREPWSTST_CAM, 0);
    OUTREG32(&g_pPrcmPrm->pOMAP_USBHOST_PRM->PM_PREPWSTST_USBHOST, 0);
    OUTREG32(&g_pPrcmPrm->pOMAP_SGX_PRM->PM_PREPWSTST_SGX, 0);
    OUTREG32(&g_pPrcmPrm->pOMAP_IVA2_PRM->PM_PREPWSTST_IVA2, 0);
    OUTREG32(&g_pPrcmPrm->pOMAP_DSS_PRM->PM_PREPWSTST_DSS, 0);

    // How far are we from next timer interrupt
    // If we are really near to timer interrupt do nothing...
    latencyState = OALWakeupLatency_GetCurrentState();
    tcrrEnter = INREG32(&g_pTimerRegs->TCRR);
    delta = g_oalTimerContext.match - tcrrEnter;
    if (delta < (INT32)g_oalTimerContext.margin) goto cleanUp;

    // get latency time...
    //
    // check if current latency is greater than current requirements
    maxDelay = min(delta, g_wakeupLatencyConstraintTickCount);
    wakeupDelay = OALWakeupLatency_GetDelayInTicks(latencyState);
    if (maxDelay < wakeupDelay)
        {
        // check if current state meets timing constraint
        latencyState = OALWakeupLatency_FindStateByMaxDelayInTicks(maxDelay);
        wakeupDelay = OALWakeupLatency_GetDelayInTicks(latencyState);
        }

    // check one last time to make sure we aren't going to sleep longer than
    if (maxDelay >= wakeupDelay)
        {
        //  Indicate in idle
        OALLED(LED_IDX_IDLE, 1);
        
        if (OALWakeupLatency_IsChipOff(latencyState))
            {
            if (!OALContextSave())
                {
                // Context Save Failed
                goto cleanUp;
                }
            }

        // account for wakeup latency
        OALWakeupLatency_PushState(latencyState);
        g_oalTimerContext.match -= wakeupDelay;
        OALTimerSetCompare(g_oalTimerContext.match);
        }
    else
        {
        goto cleanUp;
        }

    // Move SoC/CPU to idle mode    
//	TEST_GPIO_HIGH;
    fnOALCPUIdle(g_pCPUInfo);
//    INTERRUPTS_ON( );
//	TEST_GPIO_LOW;
    
	// ##### ~ 150us PLL clocks locking, but it is faster then early before changing
	// ##### CM_CLKEN_PLL register from 0x007f007f to 0x047f047e
	// Problems with shutdown (3 keys) ???



	//TEST_GPIO_HIGH;
    // restore latency state
    OALWakeupLatency_PopState();
    
    // capture previous power state and clear it
    wakeStatus = INREG32(&g_pPrcmPrm->pOMAP_WKUP_PRM->PM_WKST_WKUP);
    prevCoreState = INREG32(&g_pPrcmPrm->pOMAP_CORE_PRM->PM_PREPWSTST_CORE);
    prevMpuState = INREG32(&g_pPrcmPrm->pOMAP_MPU_PRM->PM_PREPWSTST_MPU);
    prevPerState = INREG32(&g_pPrcmPrm->pOMAP_PER_PRM->PM_PREPWSTST_PER);
   
	// ~ 30 us
	//TEST_GPIO_LOW;

    // Restore the System Context
	// ~ 3-4 us
    OALContextRestore(prevMpuState, prevCoreState, prevPerState);


	//TEST_GPIO_HIGH;
    // the rest ~ 70 us

    // get current TCRR value: workaround for errata 1.35
    INREG32(&g_pTimerRegs->TCRR);
    tcrrExit = INREG32(&g_pTimerRegs->TCRR);

    PrcmProcessPostMpuWakeup();

    // ERRATA 1.31 workaround
    do
        {
        tcrrTemp = INREG32(&g_pTimerRegs->TCRR);
        }
    while (tcrrTemp == tcrrExit);

    // display previous power states
    // capture chip state
    prevChipState = prevCoreState | prevMpuState | prevPerState;
    prevChipState |= INREG32(&g_pPrcmPrm->pOMAP_CAM_PRM->PM_PREPWSTST_CAM);
    prevChipState |= INREG32(&g_pPrcmPrm->pOMAP_USBHOST_PRM->PM_PREPWSTST_USBHOST);
    prevChipState |= INREG32(&g_pPrcmPrm->pOMAP_SGX_PRM->PM_PREPWSTST_SGX);
    prevChipState |= INREG32(&g_pPrcmPrm->pOMAP_IVA2_PRM->PM_PREPWSTST_IVA2);
    prevChipState |= INREG32(&g_pPrcmPrm->pOMAP_DSS_PRM->PM_PREPWSTST_DSS);
    
    // profile some data
    if ((wakeStatus & CM_IDLEST_ST_GPT1) == CM_IDLEST_ST_GPT1)
        {
        OMAP_PROFILE_MARK(PROFILE_WAKEUP_TIMER_MATCH, INREG32(&g_pTimerRegs->TMAR));
        OMAP_PROFILE_MARK(PROFILE_WAKEUP_TIMER_MATCH_ORIGINAL, (INREG32(&g_pTimerRegs->TMAR) + wakeupDelay));

        // Profile wake-up latencies
        if ((prevChipState & POWERSTATE_MASK) == POWERSTATE_OFF)
            {
            OMAP_PROFILE_MARK(PROFILE_WAKEUP_LATENCY_CHIP_OFF, tcrrTemp);
            OMAP_PROFILE_MARK(PROFILE_WAKEUP_TIMER_CHIP_OFF, tcrrTemp);
            }
        else if ((prevChipState & POWERSTATE_MASK) == POWERSTATE_RETENTION)
            {
            if ((prevCoreState & LOGICRETSTATE_LOGICRET_DOMAINRET) == LOGICRETSTATE_LOGICRET_DOMAINRET)
                {
                OMAP_PROFILE_MARK(PROFILE_WAKEUP_LATENCY_CHIP_CSWR, tcrrTemp);
                OMAP_PROFILE_MARK(PROFILE_WAKEUP_TIMER_CHIP_CSWR, tcrrTemp);
                }
            else
                {
                OMAP_PROFILE_MARK(PROFILE_WAKEUP_LATENCY_CHIP_OSWR, tcrrTemp);
                OMAP_PROFILE_MARK(PROFILE_WAKEUP_TIMER_CHIP_OSWR, tcrrTemp);
                }
            }
        else if ((prevCoreState & POWERSTATE_MASK) == POWERSTATE_RETENTION)
            {
            OMAP_PROFILE_MARK(PROFILE_WAKEUP_LATENCY_CORE_CSWR, tcrrTemp);
            OMAP_PROFILE_MARK(PROFILE_WAKEUP_TIMER_CORE_CSWR, tcrrTemp);
            }
        else if ((prevMpuState & POWERSTATE_MASK) == POWERSTATE_RETENTION)
            {
            OMAP_PROFILE_MARK(PROFILE_WAKEUP_LATENCY_CORE_INACTIVE, tcrrTemp);
            OMAP_PROFILE_MARK(PROFILE_WAKEUP_TIMER_CORE_INACTIVE, tcrrTemp);
            }
        else
            {
            OMAP_PROFILE_MARK(PROFILE_WAKEUP_LATENCY_MPU_INACTIVE, tcrrTemp);
            OMAP_PROFILE_MARK(PROFILE_WAKEUP_TIMER_MPU_INACTIVE, tcrrTemp);
            }
        }

    OALLED(LED_IDX_MPU_PREV_STATE, prevMpuState);
    OALLED(LED_IDX_CORE_PREV_STATE, prevCoreState);
    OALLED(LED_IDX_CHIP_PREV_STATE, prevChipState);

    // Update idle counter
    idleDelta = INREG32(&g_pTimerRegs->TCRR) - tcrrEnter;
    newIdleLow = curridlelow + idleDelta;
    if (newIdleLow < curridlelow) 
	    curridlehigh++;
    curridlelow = newIdleLow;

cleanUp:
	//TEST_GPIO_LOW;
    OALLED(LED_IDX_IDLE, 0);
    return;
}


//------------------------------------------------------------------------------
//
//  Function: OALTimerInit
//
//  General purpose timer 1 is used for system tick. It supports
//  count/compare mode on 32kHz clock
//
BOOL
OALTimerInit(
    UINT32 sysTickMSec,
    UINT32 countsPerMSec,
    UINT32 countsMargin
    )
{
    BOOL rc = FALSE;
    UINT32 sysIntr;

    OALMSG(OAL_TIMER&&OAL_FUNC, (
        L"+OALTimerInit(%d, %d, %d)\r\n", sysTickMSec, countsPerMSec,
        countsMargin
        ));

    //  Initialize timer state information
    g_oalTimerContext.maxPeriodMSec = dwOEMMaxIdlePeriod;   // Maximum period the timer will interrupt on, in mSec
    g_oalTimerContext.margin =        DELTA_TIME;           // Time needed to reprogram the timer interrupt
    g_oalTimerContext.curCounts =     0;
    g_oalTimerContext.base =          0;
    g_oalTimerContext.match =         0xFFFFFFFF;

    // Set idle conversion constant and counters
    idleconv     = MSEC_TO_TICK(1);
    curridlehigh = 0;
    curridlelow  = 0;

    // Use variable system tick
    pOEMUpdateRescheduleTime = OALTimerUpdateRescheduleTime;

    // Get virtual addresses for hardware
    g_pTimerRegs = OALPAtoUA(OMAP_GPTIMER1_REGS_PA);


    // enable gptimer1
    PrcmDeviceEnableClocks(OMAP_DEVICE_GPTIMER1, TRUE);

    // stop timer
    OUTREG32(&g_pTimerRegs->TCLR, 0);

    // Soft reset GPTIMER
    OUTREG32(&g_pTimerRegs->TIOCP, SYSCONFIG_SOFTRESET);

    // While until done
    while ((INREG32(&g_pTimerRegs->TISTAT) & GPTIMER_TISTAT_RESETDONE) == 0);

    // Set smart idle
    OUTREG32(
        &g_pTimerRegs->TIOCP,
            SYSCONFIG_SMARTIDLE|SYSCONFIG_ENAWAKEUP|
            SYSCONFIG_AUTOIDLE
        );

    // Enable posted mode
    OUTREG32(&g_pTimerRegs->TSICR, GPTIMER_TSICR_POSTED);

    // Set match register to avoid unwanted interrupt
    OUTREG32(&g_pTimerRegs->TMAR, 0xFFFFFFFF);

    // Enable match interrupt
    OUTREG32(&g_pTimerRegs->TIER, GPTIMER_TIER_MATCH);

    // Enable match wakeup
    OUTREG32(&g_pTimerRegs->TWER, GPTIMER_TWER_MATCH);

    // Enable timer in auto-reload and compare mode
    OUTREG32(&g_pTimerRegs->TCLR, GPTIMER_TCLR_CE|GPTIMER_TCLR_AR|GPTIMER_TCLR_ST);

    // Wait until write is done
    while ((INREG32(&g_pTimerRegs->TWPS) & GPTIMER_TWPS_TCLR) != 0);

    // Set global variable to tell interrupt module about timer used
    g_oalTimerIrq = IRQ_GPTIMER1;

    // Request SYSINTR for timer IRQ, it is done to reserve it...
    sysIntr = OALIntrRequestSysIntr(1, &g_oalTimerIrq, OAL_INTR_FORCE_STATIC);

    // Enable System Tick interrupt
    if (!OEMInterruptEnable(sysIntr, NULL, 0))
        {
        OALMSG(OAL_ERROR, (
            L"ERROR: OALTimerInit: Interrupt enable for system timer failed"
            ));
        goto cleanUp;
        }

    // Initialize timer to maximum period
    UpdatePeriod(g_oalTimerContext.maxPeriodMSec);

    // Done
    rc = TRUE;

cleanUp:
    OALMSG(OAL_TIMER && OAL_FUNC, (L"-OALTimerInit(rc = %d)\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OALTimerUpdateRescheduleTime
//
//  This function is called by kernel to set next reschedule time.
//
VOID
OALTimerUpdateRescheduleTime(
    DWORD timeMSec
    )
{
    UINT32 baseMSec, periodMSec;
    INT32 delta;

    // Get current system timer counter
    baseMSec = CurMSec;

    // How far we are from next tick
    delta = (INT32)(g_oalTimerContext.match - OALTimerGetCount());

    if( delta < 0 )
    {
        UpdatePeriod(0);
        goto cleanUp;
    }

    // If timer interrupts occurs, or we are within 1 ms of the scheduled
    // interrupt, just return - timer ISR will take care of it.
    if ((baseMSec != CurMSec) || (delta < MSEC_TO_TICK(1))) goto cleanUp;

    // Calculate the distance between the new time and the last timer interrupt
      periodMSec = timeMSec - OEMGetTickCount();


    // Trying to set reschedule time prior or equal to CurMSec - this could
    // happen if a thread is on its way to sleep while preempted before
    // getting into the Sleep Queue
    if ((INT32)periodMSec < 0)
        {
        periodMSec = 0;
        }
    else if (periodMSec > g_oalTimerContext.maxPeriodMSec)
        {
        periodMSec = g_oalTimerContext.maxPeriodMSec;
        }

    // Now we find new period, so update timer
    UpdatePeriod(periodMSec);

cleanUp:
    return;
}



//------------------------------------------------------------------------------
//
//  Function: OALTimerIntrHandler
//
//  This function implement timer interrupt handler. It is called from common
//  ARM interrupt handler.
//
UINT32 OALTimerIntrHandler()
{
    UINT32 sysIntr = SYSINTR_NOP;
    UINT32 count;
    INT32 period, delta;

    if (g_oalILT.active) {
        g_oalILT.counter--;
        if (g_oalILT.counter == 0) {
            g_oalILT.interrupts = 0;
            g_oalILT.isrTime1 = OALTimerCountsSinceSysTick();
        }            
    }        

    // allow bsp to process timer interrupt first
//    sysIntr = OALTickTimerIntr();
//    if(sysIntr != SYSINTR_NOP)
//		return sysIntr;

    // Clear interrupt
    OUTREG32(&g_pTimerRegs->TISR, GPTIMER_TIER_MATCH);

    // How far from interrupt we are?
    count = OALTimerGetCount();
    delta = count - g_oalTimerContext.match;

    // If delta is negative, timer fired for some reason
    // To be safe, reprogram the timer for minimum delta
    if (delta < 0)
    {
        delta = 0;
        goto cleanUp;
    }

    // Find how long period was
    period = count - g_oalTimerContext.base;
    g_oalTimerContext.curCounts += period;
    g_oalTimerContext.base += period;

    // Calculate actual CurMSec
    CurMSec = (UINT32) TICK_TO_MSEC(g_oalTimerContext.curCounts);

    OALLED(LED_IDX_TIMER, CurMSec >> 10);

    // Reschedule?
    delta = dwReschedTime - CurMSec;
    if (delta <= 0)
        {
        sysIntr = SYSINTR_RESCHED;
        delta = g_oalTimerContext.maxPeriodMSec;
        }

cleanUp:
    // Set new period
    UpdatePeriod(delta);

    if (g_oalILT.active) {
        if (g_oalILT.counter == 0) {
            sysIntr = SYSINTR_TIMING;
            g_oalILT.counter = g_oalILT.counterSet;
            g_oalILT.isrTime2 = OALTimerCountsSinceSysTick();
        }
    }        

    return sysIntr;
}


//------------------------------------------------------------------------------
//
//  Function:  OALTimerGetCount
//
UINT32 OALTimerGetCount()
{
    //  Return the timer value
    return INREG32(&g_pTimerRegs->TCRR);
}

//------------------------------------------------------------------------------
//
//  Function:  OALTimerCountsSinceSysTick
//
INT32 OALTimerCountsSinceSysTick()
{
    // Return timer ticks since last interrupt
    return (INT32)(INREG32(&g_pTimerRegs->TCRR) - g_oalTimerContext.base);
}

//------------------------------------------------------------------------------
//
//  Function:  OALGetTickCount
//
//  This function returns number of 1 ms ticks which elapsed since system boot
//  or reset (absolute value isn't important). The counter can overflow but
//  overflow period should not be shorter then approx 30 seconds. Function
//  is used in  system boot so it must work before interrupt subsystem
//  is active.
//
UINT32
OALGetTickCount(
    )
{
    UINT64 tickCount = INREG32(&g_pTimerRegs->TCRR);
    //  Returns number of 1 msec ticks
    return (UINT32) TICK_TO_MSEC(tickCount);
}

//------------------------------------------------------------------------------
//
//  Function: OEMGetTickCount
//
//  This returns the number of milliseconds that have elapsed since Windows
//  CE was started. If the system timer period is 1ms the function simply
//  returns the value of CurMSec. If the system timer period is greater then
//  1 ms, the HiRes offset is added to the value of CurMSec.
//
UINT32
OEMGetTickCount(
    )
{
    UINT64 baseCounts;
    UINT32 offset;

    // This code adjusts the accuracy of the returned value to the nearest
    // MSec when the system tick exceeds 1 ms. The following code checks if
    // a system timer interrupt occurred between reading the CurMSec value
    // and the call to fetch the HiResTicksSinceSysTick. If so, the value of
    // CurMSec and Offset is re-read, with the certainty that a system timer
    // interrupt will not occur again.
    do
        {
        baseCounts = g_oalTimerContext.curCounts;
        offset = OALTimerGetCount() - g_oalTimerContext.base;
        }
    while (baseCounts != g_oalTimerContext.curCounts);


   //  Update CurMSec (kernel uses both CurMSec and GetTickCount() at different places) and return msec tick count
    CurMSec = (UINT32)TICK_TO_MSEC(baseCounts + offset);

    return CurMSec;

}



//------------------------------------------------------------------------------

