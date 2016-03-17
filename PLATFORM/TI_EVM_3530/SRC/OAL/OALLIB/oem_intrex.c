/*
===============================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
===============================================================================
*/
//
//  File:  oem_intrex.c
//
#include <windows.h>
#include <bsp.h>
#include <dvfs.h>
#include <oal_prcm.h>
#include <omap_prof.h>
#include <Profiler.h>

//------------------------------------------------------------------------------
// defines
#define HISTORY_COUNT           4
#define HISTORY_MASK            0x03        // must correlate with HISTORY_COUNT
#define HISTORY_PERIOD          (32768)     // 1 second history

//------------------------------------------------------------------------------
// typedefs
typedef struct {
   DWORD         rgTickCount[HISTORY_COUNT];
   DWORD         rgIdleCount[HISTORY_COUNT];
} TICK_LOG;

//------------------------------------------------------------------------------
// static variables

static DWORD s_swInterruptMask = 0;

// used for cpu load monitoring
static DWORD s_idleSum;
static DWORD s_tickSum;
static DWORD s_idleCount;
static DWORD s_tcrrCount;
static DWORD s_idleThreshold;           // 0.8 fixed point number (% value)
static DWORD s_maxBucketSize;

// history tracker
static DWORD    s_logIndex = 0;
static TICK_LOG s_TickLog;

//------------------------------------------------------------------------------
//
//  Function: OALSWIntrSetDataIrq
//
void
OALSWIntrSetDataIrq(
    UINT32 irq, 
    LPVOID pvData, 
    DWORD cbData
    )
{
    DWORD dwVal;
    
    if (irq == IRQ_SW_CPUMONITOR)
        {
        // value is in cpu_load_%, convert to idle_load_%
        dwVal = min(100, *(DWORD*)pvData);
        dwVal = 100 - dwVal;

        // convert idle threshold to 0.8f
        s_idleThreshold = (DWORD)((float)(256*dwVal)/100.0f);
        s_maxBucketSize = HISTORY_PERIOD / HISTORY_COUNT;
        }
}

//-----------------------------------------------------------------------------
//
//  Function:  OALSWIntrEnableIrq
//
//  sw interrupt handler
//
BOOL
OALSWIntrEnableIrq(
    UINT32 irq
    )
{   
    if (irq == IRQ_SW_CPUMONITOR)
        {
        // reset cpu monitor counters
        s_tickSum = 0;
        s_idleSum = 0;
        s_logIndex = 0;        
        memset(&s_TickLog, 0, sizeof(TICK_LOG));        
        }
    s_swInterruptMask |= (1 << (irq - IRQ_SW_RTC_QUERY));
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  OALSWIntrDisableIrq
//
//  sw interrupt handler
//
VOID
OALSWIntrDisableIrq(
    UINT32 irq
    )
{
    if (irq == IRQ_SW_CPUMONITOR)
        {
        // reset cpu monitor counters
        s_tickSum = 0;
        s_idleSum = 0;
        s_logIndex = 0;
        memset(&s_TickLog, 0, sizeof(TICK_LOG));
        }
    s_swInterruptMask &= ~(1 << (irq - IRQ_SW_RTC_QUERY));
}

//-----------------------------------------------------------------------------
//
//  Function:  OALSWIntrDoneIrq
//
//  sw interrupt handler
//
VOID
OALSWIntrDoneIrq(
    UINT32 irq
    )
{    
    s_swInterruptMask |= (1 << (irq - IRQ_SW_RTC_QUERY));
}

//-----------------------------------------------------------------------------
//
//  Function:  OALTickTimerIntr
//
//  Pre-process gptimer1 (tick timer) interrupt.  Wake-up cpu monitor
//  if the cpu load exceeds threshold.
//
UINT32
OALTickTimerIntr()
{
    DWORD ratio;    
    DWORD idleTime;
    DWORD tickTime;
    UINT64 preDelta;
    DWORD remainder;
    DWORD idleDelta;
    DWORD bucketSpace;
    DWORD currentTick;
    DWORD idleTotalTime;
    DWORD tickTotalTime;
    DWORD idleRemove = 0;
    DWORD tickRemove = 0;
    BOOL bRatioCalculated = FALSE;
    UINT32 sysIntr = SYSINTR_NOP;
   
    currentTick = OALTimerGetCount();
    if (s_swInterruptMask & (1 << (IRQ_SW_CPUMONITOR - IRQ_SW_RTC_QUERY)))
        {
        // calculate idle time and active time since last gptimer interrupt
        idleTime = curridlelow - s_idleCount;
        tickTime = currentTick - s_tcrrCount;

        // typically cpuload is calculated as 
        // idle_threshold >= idleTime/activeTime
        //
        // to remove division us following eq. instead
        //  idle_threshold * activeTime >= idleTime
        //
        idleTotalTime = idleTime + s_idleSum;
        tickTotalTime = tickTime + s_tickSum;
        tickTotalTime *= s_idleThreshold;    //24.8f
        tickTotalTime >>= 8;                 //32.0f

        if (idleTotalTime <= tickTotalTime && s_tickSum > 0)
            {
            OALSWIntrDisableIrq(IRQ_SW_CPUMONITOR);
            sysIntr = OALIntrTranslateIrq(IRQ_SW_CPUMONITOR);
            }
        else 
            {
            // Need to check if ticktime exceeds maximum history time.
            if (tickTime > HISTORY_PERIOD)
                {                
                ratio = (DWORD)(((UINT64)idleTime << 16) / (UINT64)tickTime); // u0.16
                bRatioCalculated = TRUE;

                // update idleTime and tickTime to maximum allowed period
                idleTime -= (((tickTime - HISTORY_PERIOD) * ratio) >> 16);
                tickTime = HISTORY_PERIOD;
                }
            
            // update sum with the amount of tick and idle time that is
            // being added to the sum
            //
            remainder = 0x8000;
            s_tickSum = s_tickSum + tickTime;
            s_idleSum = s_idleSum + idleTime;
            
            while (tickTime > 0)
                {
                bucketSpace = s_maxBucketSize - s_TickLog.rgTickCount[s_logIndex];
                if (bucketSpace > 0)
                    {
                    // check if current tick count will fit into bucket
                    if (tickTime <= bucketSpace)
                        {
                        s_TickLog.rgTickCount[s_logIndex] += tickTime;
                        s_TickLog.rgIdleCount[s_logIndex] += idleTime;                       
                        break;
                        }

                    if (bRatioCalculated == FALSE)
                        { 
                        ratio = (DWORD)(((UINT64)idleTime << 16) / (UINT64)tickTime); // u0.16
                        bRatioCalculated = TRUE;
                        }

                    if (ratio == 0)
                        {
                        idleDelta = 0;
                        }
                    else
                        {
                        // calculate how many idle and tick counts to add into bucket   
                        preDelta = (UINT64)bucketSpace * (UINT64)ratio;             // u32.16
                        idleDelta = (DWORD)((preDelta + (UINT64)remainder) >> 16);  // u32                 
                        remainder = ((DWORD)preDelta + remainder) & 0xFFFF;         // u0.16
                        idleDelta = min(idleDelta, idleTime);
                        }

                    // update buckets and leftover times
                    s_TickLog.rgTickCount[s_logIndex] += bucketSpace;
                    s_TickLog.rgIdleCount[s_logIndex] += idleDelta;
                    tickTime -= bucketSpace;
                    idleTime -= idleDelta;
                    }
                
                // update with next log                
                s_logIndex = (s_logIndex + 1) & HISTORY_MASK;
                tickRemove += s_TickLog.rgTickCount[s_logIndex];
                idleRemove += s_TickLog.rgIdleCount[s_logIndex];
                s_TickLog.rgTickCount[s_logIndex] = 0;
                s_TickLog.rgIdleCount[s_logIndex] = 0;
                }

            // update sum with the amount of tick and idle time that
            // is being removed
            s_tickSum -= tickRemove;
            s_idleSum -= idleRemove;
            }       
        }

    // keep track of timer information
    s_tcrrCount = currentTick;
    s_idleCount = curridlelow;

    return sysIntr;
}

//-----------------------------------------------------------------------------
