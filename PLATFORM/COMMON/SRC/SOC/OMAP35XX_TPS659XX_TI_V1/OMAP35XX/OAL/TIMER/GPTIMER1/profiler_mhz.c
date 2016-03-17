// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

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
//  File:  profiler_mhz.c
//
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>
#include <bus.h>
#include <oal_prcm.h>

//------------------------------------------------------------------------------
// globals
//
BOOL g_oalProfilerEnabled = FALSE;
UINT32 g_oalPerfTimerIrq = -1;
UINT32 g_oalProfilerIncrement = 0;
UINT32 g_oalLastSysIntr = SYSINTR_NOP;
OMAP_GPTIMER_REGS   *g_pPerfTimer = NULL;

//------------------------------------------------------------------------------
// staticx
//
static DWORD                s_microSecondOffset;    //16.16
static DWORD                s_Frequency;

// UNDONE:
//  Need to initialize all gpt to select clk src to sys_clk

//------------------------------------------------------------------------------
//
//  Function: OALPerformanceTimerInit
//
//  This function initializes the high resolution counter
//
void OALPerformanceTimerInit(DWORD clock, DWORD flag)
{
    DWORD tclr = 0;
    DWORD sysClkFreq;
    UINT32 sysIntr;
    OMAP_PRCM_CLOCK_CONTROL_PRM_REGS *pPrcmClkPRM;

    pPrcmClkPRM = OALPAtoUA(OMAP_PRCM_CLOCK_CONTROL_PRM_REGS_PA);

    // map PRCM and HighResTimer
    g_pPerfTimer = OALPAtoUA(OMAP_GPTIMER2_REGS_PA);

    // use sys clk 
    PrcmClockSetParent(kGPT2_ALWON_FCLK, kSYS_CLK);

    // Enable GPTimer2 for high perf/monte carlo profiling
    PrcmDeviceEnableClocks(OMAP_DEVICE_GPTIMER2, TRUE);

    // configure performance timer
    //---------------------------------------------------
    // Soft reset GPTIMER and wait until finished
    SETREG32(&g_pPerfTimer->TIOCP, SYSCONFIG_SOFTRESET);
    while ((INREG32(&g_pPerfTimer->TISTAT) & GPTIMER_TISTAT_RESETDONE) == 0);
 
    // Enable smart idle and autoidle
    // Set clock activity - FCLK can be  switched off, 
    // L4 interface clock is maintained during wkup.
    OUTREG32(&g_pPerfTimer->TIOCP, 
        0x200 | SYSCONFIG_SMARTIDLE|SYSCONFIG_ENAWAKEUP|
            SYSCONFIG_AUTOIDLE); 

    // Select posted mode
    SETREG32(&g_pPerfTimer->TSICR, GPTIMER_TSICR_POSTED);

    // clear match register
    OUTREG32(&g_pPerfTimer->TMAR, 0xFFFFFFFF);
    
    // clear interrupts
    OUTREG32(&g_pPerfTimer->TISR, 0x00000000);
    
    // enable match interrupt
    OUTREG32(&g_pPerfTimer->TIER, GPTIMER_TIER_MATCH);
 
    // enable wakeups
    OUTREG32(&g_pPerfTimer->TWER, GPTIMER_TWER_MATCH);

    // Set the load register value.
    OUTREG32(&g_pPerfTimer->TLDR, 0x00000000);
 
    // Trigger a counter reload by writing    
    OUTREG32(&g_pPerfTimer->TTGR, 0xFFFFFFFF);

    // calculate 1 microsecond offset value and timer divisor
    sysClkFreq = INREG32(&pPrcmClkPRM->PRM_CLKSEL);
    switch (sysClkFreq)
        {
        case 0:
            // 12mhz
            s_Frequency = 12000000;         // 12,000,000 mhz
            s_microSecondOffset = 12 << 16; // 16.16
            break;

        case 1:
            // 13mhz
            s_Frequency = 13000000;         // 13,000,000 mhz
            s_microSecondOffset = 13 << 16; // 16.16
            break;

        case 2:
            // 19.2mhz
            s_Frequency = 19200000 >> 2;    // 4,800,000 mhz

            // prescale by 4
            tclr = GPTIMER_TCLR_PRE | (1 << 2);
            s_microSecondOffset = 48 << 15; // 16.16
            break;
            
        case 3:
            // 26mhz
            s_Frequency = 13000000;         // 13,000,000 mhz

            // prescale by 4
            tclr = GPTIMER_TCLR_PRE;
            s_microSecondOffset = 13 << 15; // 16.16
            break;

        case 4:
            // 38.4mhz
            s_Frequency = 38400000 >> 3;    // 4,800,000 mhz

            // prescale by 4
            tclr = GPTIMER_TCLR_PRE | (2 << 2);
            s_microSecondOffset = 48 << 15; // 16.16
            break;

        default:
            s_Frequency = 0;
            return;
        }
    
    OALMSG(1, (L"****Profiler Build****\r\n"));
    OALMSG(1, (L"---High Performance Frequecy is %d hz---\r\n", s_Frequency));
    
    // build tclr mask
    tclr |= GPTIMER_TCLR_AR;
    OUTREG32(&g_pPerfTimer->TCLR,  tclr);
   
    // Initialize high resolution timer function pointers
    pQueryPerformanceFrequency = OALTimerQueryPerformanceFrequency;
    pQueryPerformanceCounter = OALTimerQueryPerformanceCounter;

    // Request SYSINTR for timer IRQ, it is done to reserve it...
    g_oalPerfTimerIrq = IRQ_GPTIMER2;
    sysIntr = OALIntrRequestSysIntr(1, &g_oalPerfTimerIrq, OAL_INTR_FORCE_STATIC);
    g_oalPerfTimerIrq = OAL_INTR_IRQ_UNDEFINED;    

    // Enable System Tick interrupt
    if (!OEMInterruptEnable(sysIntr, NULL, 0))
        {
        OALMSG(OAL_ERROR, (L"ERROR: OALPerformanceTimerInit: "
            L"Interrupt enable for system timer failed"
            ));
        }

    //  Start the timer.  Also set for auto reload
    SETREG32(&g_pPerfTimer->TCLR, GPTIMER_TCLR_ST);
    while ((INREG32(&g_pPerfTimer->TWPS) & GPTIMER_TWPS_TCLR) != 0);
    
}


//------------------------------------------------------------------------------
//
//  Function: OALTimerQueryPerformanceFrequency
//
//  This function returns the frequency of the high-resolution 
//  performance counter.
// 
BOOL OALTimerQueryPerformanceFrequency(LARGE_INTEGER *pFrequency)
{    
    pFrequency->QuadPart = s_Frequency;    
    return TRUE;
}

 
//------------------------------------------------------------------------------
//
//  Function: OALQueryPerformanceCounter
//
//  This function returns the current value of the high-resolution 
//  performance counter.
//  
BOOL OALTimerQueryPerformanceCounter(LARGE_INTEGER *pCounter)
{
    static LARGE_INTEGER PreviousValue = {0, 0};
    DWORD CurrentValue;
    BOOL bInterruptsEnabled;
    
    bInterruptsEnabled = INTERRUPTS_ENABLE(FALSE);

    CurrentValue = INREG32(&g_pPerfTimer->TCRR);
    if (CurrentValue < PreviousValue.LowPart)
    {
        // rollover, increment upper 32 bits
        PreviousValue.HighPart++;
    }
    PreviousValue.LowPart = CurrentValue;
    pCounter->QuadPart = PreviousValue.QuadPart;            

    INTERRUPTS_ENABLE(bInterruptsEnabled);

    return TRUE;
}


//------------------------------------------------------------------------------

VOID
OEMProfileTimerEnable(
    DWORD interval /*usually 200us*/
    )
{
    DWORD tcrr;
    UINT64 val;
    BOOL enabled;
    UINT64 interval64 = interval;    
    
    // calculate interrupt intervals
    interval64 <<= 16; // 16.16
    val = (UINT64)interval64 * (UINT64)s_microSecondOffset;
    g_oalProfilerIncrement = (UINT32)(val >> 32);

    OALLED(LED_IDX_PROFILER_INC, g_oalProfilerIncrement);

    // Following code should not be interrupted
    enabled = INTERRUPTS_ENABLE(FALSE);

    // currently using gptimer2 for high performance clocking
    g_oalPerfTimerIrq = IRQ_GPTIMER2;
    
    // set interrupts at requested intervals
    tcrr = INREG32(&g_pPerfTimer->TCRR) + g_oalProfilerIncrement;
    OUTREG32(&g_pPerfTimer->TMAR, tcrr);
    while ((INREG32(&g_pPerfTimer->TWPS) & GPTIMER_TWPS_TMAR) != 0);
    
    // enable high perf interrupt
    g_oalProfilerEnabled = TRUE;
    SETREG32(&g_pPerfTimer->TCLR, GPTIMER_TCLR_CE);

    // Enable profiling interrupt
    OALIntrEnableIrqs(1, &g_oalPerfTimerIrq);

    // Enable interrupts
    INTERRUPTS_ENABLE(enabled);
    
}

//------------------------------------------------------------------------------

VOID
OEMProfileTimerDisable(
    ) 
{
    BOOL enabled;
    
    if (g_oalProfilerEnabled == FALSE) return;

    // Following code should not be interrupted
    enabled = INTERRUPTS_ENABLE(FALSE);

    // Disable the profile timer interrupt
    OALIntrDisableIrqs(1, &g_oalPerfTimerIrq);
    
    // Inform interrupt module
    g_oalPerfTimerIrq = OAL_INTR_IRQ_UNDEFINED;

    // Reset flag
    g_oalProfilerEnabled = FALSE;
    CLRREG32(&g_pPerfTimer->TCLR, GPTIMER_TCLR_CE);

    // Enable interrupts
    INTERRUPTS_ENABLE(enabled);
}

//------------------------------------------------------------------------------

void 
OALProfileTimerHit(
    UINT32 ra
    )
{
    UINT32  tcrr;

    //  Call ProfileHit
    ProfilerHit(ra);

    // get current time
    tcrr = INREG32(&g_pPerfTimer->TCRR);
    OALLED(LED_IDX_PROFILER_TICK, tcrr);
    
    // clear interrupt
    OUTREG32(&g_pPerfTimer->TISR, 
        GPTIMER_TISR_MAT | GPTIMER_TISR_OVF | GPTIMER_TISR_TCAR
        );

    // setup for next interrupt        
    OUTREG32(&g_pPerfTimer->TMAR, tcrr + g_oalProfilerIncrement);

    // clear interrupt status
    OALIntrDoneIrqs(1, &g_oalPerfTimerIrq);
}
