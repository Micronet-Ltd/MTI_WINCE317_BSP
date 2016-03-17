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
#include <windows.h>
#include <bsp.h>
#include <dvfs.h>
#include <oal_prcm.h>
#include <omap_prof.h>
#include <Profiler.h>

//------------------------------------------------------------------------------
#define GPTIMER_ADDRESS             (OMAP_GPTIMER3_REGS_PA)
#define GPTIMER_DEVICE              (OMAP_DEVICE_GPTIMER3)

//------------------------------------------------------------------------------
// globals
//
static DWORD                        _frequency = 0;
static DWORD                        _preScale = -1;
static BOOL                         _profilerEnabled = FALSE;
static OMAP_GPTIMER_REGS           *_pProfTimer = NULL;
static DWORD                        _rgMarkers[PROFILE_COUNT];

//------------------------------------------------------------------------------
//
//  External:  OmapProfilerMark
//
//  sets a profile marker.
//
void
OmapProfilerMark(
    DWORD id,
    void *pv
    )
{
    // validate index
    if (id >= PROFILE_COUNT) return;
    
    switch (id)
        {
        // core DVFS latency info
        case PROFILE_CORE1_DVFS_BEGIN:
        case PROFILE_CORE1_DVFS_END:
            if (_profilerEnabled != FALSE)
                {
                _rgMarkers[id] = INREG32(&_pProfTimer->TCRR);
                }
            break;
        
        // calculate latency 
        case PROFILE_WAKEUP_LATENCY_CHIP_OFF:
        case PROFILE_WAKEUP_LATENCY_CHIP_OSWR:
        case PROFILE_WAKEUP_LATENCY_CHIP_CSWR:
        case PROFILE_WAKEUP_LATENCY_CORE_CSWR:
        case PROFILE_WAKEUP_LATENCY_CORE_INACTIVE:
        case PROFILE_WAKEUP_LATENCY_MPU_INACTIVE:
            _rgMarkers[id] = (DWORD)pv - _rgMarkers[PROFILE_WAKEUP_TIMER_MATCH];
            break;

        default:
            _rgMarkers[id] = (DWORD)pv;
            break;
            
        }
}

//------------------------------------------------------------------------------
//
//  External:  OmapProfilerStart
//
//  starts the profiler.
//
void
OmapProfilerStart(
    )
{
    // initialize gptimer
    OMAP_PRCM_CLOCK_CONTROL_PRM_REGS *pPrcmClkPRM;

    pPrcmClkPRM = OALPAtoUA(OMAP_PRCM_CLOCK_CONTROL_PRM_REGS_PA);
    if (_pProfTimer == NULL)
        {
        // map gptimer and turn it on
        _pProfTimer = OALPAtoUA(GPTIMER_ADDRESS);
        PrcmDeviceEnableClocks(GPTIMER_DEVICE, TRUE);

        // configure performance timer
        //---------------------------------------------------
        // Soft reset GPTIMER and wait until finished
        SETREG32(&_pProfTimer->TIOCP, SYSCONFIG_SOFTRESET);
        while ((INREG32(&_pProfTimer->TISTAT) & GPTIMER_TISTAT_RESETDONE) == 0);
     
        // Enable smart idle and autoidle
        // Set clock activity - FCLK can be  switched off, 
        // L4 interface clock is maintained during wkup.
        OUTREG32(&_pProfTimer->TIOCP, 
            SYSCONFIG_CLOCKACTIVITY_I_ON | SYSCONFIG_SMARTIDLE | 
            SYSCONFIG_ENAWAKEUP | SYSCONFIG_AUTOIDLE
            ); 

        // Select posted mode
        SETREG32(&_pProfTimer->TSICR, GPTIMER_TSICR_POSTED);

        // clear match register
        OUTREG32(&_pProfTimer->TMAR, 0xFFFFFFFF);
        
        // clear interrupts
        OUTREG32(&_pProfTimer->TISR, 0x00000000);

        // Set the load register value.
        OUTREG32(&_pProfTimer->TLDR, 0x00000000);

        // calculate 1 microsecond offset value and timer divisor
        switch (INREG32(&pPrcmClkPRM->PRM_CLKSEL))
            {
            case 0:
                // 12mhz
                _frequency = 12000000;          // 12,000,000 mhz
                break;

            case 1:
                // 13mhz
                _frequency = 13000000;          // 13,000,000 mhz
                break;

            case 2:
                // 19.2mhz
                _frequency = 19200000;          // 19,200,000 mhz
                break;
                
            case 3:
                // 26mhz
                _frequency = 26000000;          // 26,000,000 mhz
                break;

            case 4:
                // 38.4mhz
                _frequency = 38400000;          // 38,400,000 mhz
                break;

            default:
                _frequency = 0;
                return;
            }                
        }
    else
        {
        PrcmDeviceEnableClocks(GPTIMER_DEVICE, TRUE);
        }

    // set prescaler
    if (_preScale != -1)
        {
        OUTREG32(&_pProfTimer->TCLR, 
            GPTIMER_TLCR_PRE | _preScale << GPTIMER_TLCR_PTV_SHIFT
            );
        }

    // clear all markers
    memset(_rgMarkers, 0, sizeof(DWORD) * PROFILE_COUNT);

    // Trigger a counter reload by writing    
    OUTREG32(&_pProfTimer->TTGR, 0xFFFFFFFF);
    
    //  Start the timer.  Also set for auto reload
    SETREG32(&_pProfTimer->TCLR, GPTIMER_TCLR_ST);
    while ((INREG32(&_pProfTimer->TWPS) & GPTIMER_TWPS_TCLR) != 0);
        
    _profilerEnabled = TRUE;
}

//------------------------------------------------------------------------------
//
//  External:  OmapProfilerStop
//
//  stops the profiler.
//
void
OmapProfilerStop(
    )
{
    CLRREG32(&_pProfTimer->TCLR, GPTIMER_TCLR_ST);
    while ((INREG32(&_pProfTimer->TWPS) & GPTIMER_TWPS_TCLR) != 0);
    _profilerEnabled = FALSE;

    PrcmDeviceEnableClocks(GPTIMER_DEVICE, FALSE);
}

//------------------------------------------------------------------------------
//
//  External:  OmapProfilerQuery
//
//  captures profiling information.
//
void
OmapProfilerQuery(
    DWORD count,
    DWORD target[],
    DWORD value[]
    )
{
    DWORD i;
    
    for (i = 0; i < count; ++i)
        {
        value[i] = _rgMarkers[target[i]];
        _rgMarkers[target[i]] = 0;
        }
}

//-----------------------------------------------------------------------------
//
//  Function:  OALIoCtlHALProfiler
//
//  processes the IOCTL_HAL_OEM_PROFILER ioctl
//
BOOL 
OALIoCtlHALProfiler(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    )
{
    BOOL rc;
    ProfilerControl    *pProfilerControl;
    ProfilerControlEx  *pProfilerControlEx;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHALProfiler\r\n"));

    // validate parameters
    //
    rc = FALSE;
    if (pInBuffer == NULL)
        {
        goto cleanUp;
        }

    pProfilerControl = (ProfilerControl*)pInBuffer;
    if (inSize != sizeof(ProfilerControl) + pProfilerControl->OEM.dwControlSize)
        {
        goto cleanUp;
        }

    // determine command
    if (pProfilerControl->dwOptions & (PROFILE_STARTPAUSED | PROFILE_CONTINUE | PROFILE_START))
        {
        OmapProfilerStart();
        rc = TRUE;
        }
    else if (pProfilerControl->dwOptions & (PROFILE_PAUSE | PROFILE_STOP))
        {
        OmapProfilerStop();
        rc = TRUE;
        }
    else if (pProfilerControl->dwOptions & (PROFILE_OEMDEFINED))
        {
        if (inSize != sizeof(ProfilerControlEx)) goto cleanUp;

        // get data
        pProfilerControlEx = (ProfilerControlEx*)pProfilerControl;
        OmapProfilerQuery(pProfilerControlEx->OEM.dwCount, 
            pProfilerControlEx->OEM.rgTargets, 
            pProfilerControlEx->OEM.rgValues
            );
        
        rc = TRUE;
        }
    
cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIoCtlHALProfiler(rc = %d)\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------
