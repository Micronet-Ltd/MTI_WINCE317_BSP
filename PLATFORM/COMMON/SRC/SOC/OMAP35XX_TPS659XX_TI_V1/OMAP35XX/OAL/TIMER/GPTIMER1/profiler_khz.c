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
//  File:  profiler_khz.c
//
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>

//------------------------------------------------------------------------------
// globals
//
BOOL g_oalProfilerEnabled = FALSE;
UINT32 g_oalPerfTimerIrq = -1;

extern UINT32               g_oalTimerIrq;
extern OMAP_GPTIMER_REGS*   g_pTimerRegs;


//------------------------------------------------------------------------------
//
//  Function: OALPerformanceTimerInit
//
//  Initialize the high performance clock for profiling
//
void OALPerformanceTimerInit(DWORD clock, DWORD flag)
{
    OALMSG(TRUE, (L"--- High Performance Frequecy is 32768 khz---\r\n"));

    // Initialize high resolution timer function pointers
    pQueryPerformanceFrequency = OALTimerQueryPerformanceFrequency;
    pQueryPerformanceCounter = OALTimerQueryPerformanceCounter;

    // Profiler timer is system tick timer
    g_oalPerfTimerIrq = g_oalTimerIrq;
}

//------------------------------------------------------------------------------
//
//  Function: OALQueryPerformanceFrequency
//
//  This function returns the frequency of the high-resolution 
//  performance counter.
//
BOOL OALTimerQueryPerformanceFrequency(LARGE_INTEGER *pFrequency)
{
    pFrequency->QuadPart = 32768;    
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

    CurrentValue = INREG32(&g_pTimerRegs->TCRR);
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
    DWORD interval
    )
{
    //  Enable profiler
    g_oalProfilerEnabled = TRUE;
}

//------------------------------------------------------------------------------

VOID
OEMProfileTimerDisable(
    ) 
{
    //  Disable profiler
    g_oalProfilerEnabled = FALSE;
}

//------------------------------------------------------------------------------

void 
OALProfileTimerHit(
    UINT32 ra
    )
{
    //  Call ProfileHit
    ProfilerHit(ra);
}
