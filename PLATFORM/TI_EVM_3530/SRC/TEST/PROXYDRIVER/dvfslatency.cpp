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
//  File: dvfslatency.cpp
//
#include <windows.h>
#include <oal.h>
#include <oalex.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap35xx.h>
#include <omap_prof.h>
#include <dvfs.h>
#include "proxyapi.h"
#include <Profiler.h>

#ifndef MAX_INT
#define MAX_INT                         0x7FFFFFFF
#endif

#define OPP_LATENCY_TEST_LOOPCOUNT      (100)

//------------------------------------------------------------------------------
// table defining opp setup 
typedef struct {
    IOCTL_OPP_REQUEST_IN    envOppInfo;
    IOCTL_OPP_REQUEST_IN    oppAInfo;
    IOCTL_OPP_REQUEST_IN    oppBInfo;
    DWORD                   minUp;
    DWORD                   maxUp;
    DWORD                   sumUp;
    DWORD                   minDown;
    DWORD                   maxDown;
    DWORD                   sumDown;
} OPP_LATENCY_SETUP;

OPP_LATENCY_SETUP _rgOppLatencySetupTable[] = {
    {   
        {sizeof(IOCTL_OPP_REQUEST_IN), 1, {DVFS_MPU1_OPP}, {kOpp1}},
        {sizeof(IOCTL_OPP_REQUEST_IN), 1, {DVFS_CORE1_OPP}, {kOpp1}},
        {sizeof(IOCTL_OPP_REQUEST_IN), 1, {DVFS_CORE1_OPP}, {kOpp2}}
    }, {
        {sizeof(IOCTL_OPP_REQUEST_IN), 1, {DVFS_MPU1_OPP}, {kOpp2}},
        {sizeof(IOCTL_OPP_REQUEST_IN), 1, {DVFS_CORE1_OPP}, {kOpp1}},
        {sizeof(IOCTL_OPP_REQUEST_IN), 1, {DVFS_CORE1_OPP}, {kOpp2}}
    }, {
        {sizeof(IOCTL_OPP_REQUEST_IN), 1, {DVFS_MPU1_OPP}, {kOpp3}},
        {sizeof(IOCTL_OPP_REQUEST_IN), 1, {DVFS_CORE1_OPP}, {kOpp1}},
        {sizeof(IOCTL_OPP_REQUEST_IN), 1, {DVFS_CORE1_OPP}, {kOpp2}}
    }, {
        {sizeof(IOCTL_OPP_REQUEST_IN), 1, {DVFS_MPU1_OPP}, {kOpp4}},
        {sizeof(IOCTL_OPP_REQUEST_IN), 1, {DVFS_CORE1_OPP}, {kOpp1}},
        {sizeof(IOCTL_OPP_REQUEST_IN), 1, {DVFS_CORE1_OPP}, {kOpp2}}
    }, {
        {sizeof(IOCTL_OPP_REQUEST_IN), 1, {DVFS_MPU1_OPP}, {kOpp5}},
        {sizeof(IOCTL_OPP_REQUEST_IN), 1, {DVFS_CORE1_OPP}, {kOpp1}},
        {sizeof(IOCTL_OPP_REQUEST_IN), 1, {DVFS_CORE1_OPP}, {kOpp2}}
    }, {
        {0, 0, {0}, {0}},
        {0, 0, {0}, {0}},
        {0, 0, {0}, {0}}
    }
};

//------------------------------------------------------------------------------
static OMAP_PRCM_PER_CM_REGS   *_pOmap_Prcm_Per_Cm = NULL;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
RunOppLatencyProfiler(
    int     nSamples
    )
{
    int i;
    int idx = 0;
    DWORD minUp, maxUp, sumUp;
    DWORD minDown, maxDown, sumDown;
    DWORD temp;
    ProfilerControlEx profilerControlEx;
    WCHAR szBuffer[MAX_PATH];

    // initialize variables
    OPP_LATENCY_SETUP  *pLatencySetup = &_rgOppLatencySetupTable[idx++];
    memset(&profilerControlEx, 0, sizeof(ProfilerControlEx));

    profilerControlEx.dwVersion = 1;
    profilerControlEx.OEM.dwControlSize = sizeof(ProfilerControlEx) - sizeof(ProfilerControl);
    profilerControlEx.OEM.dwCount = 2;
    profilerControlEx.OEM.rgTargets[0] = PROFILE_CORE1_DVFS_BEGIN;
    profilerControlEx.OEM.rgTargets[1] = PROFILE_CORE1_DVFS_END;

    // set source clock to sys clock
    if (_pOmap_Prcm_Per_Cm == NULL)
        {
        PHYSICAL_ADDRESS PhysAddr;

        PhysAddr.QuadPart = OMAP_PRCM_PER_CM_REGS_PA;
        _pOmap_Prcm_Per_Cm = (OMAP_PRCM_PER_CM_REGS*)MmMapIoSpace(PhysAddr, OMAP_PRCM_PER_CM_REGS_SIZE, FALSE);
        }

    // use gptimer3
    SETREG32(&_pOmap_Prcm_Per_Cm->CM_CLKSEL_PER, CLKSEL_GPT3);

    // start oem profiler
    profilerControlEx.dwOptions = PROFILE_STARTPAUSED;
    //ProfileStartEx((ProfilerControl*)&profilerControlEx);
    profilerControlEx.dwOptions = PROFILE_START;
    KernelIoControl(IOCTL_HAL_OEM_PROFILER, 
                &profilerControlEx, 
                sizeof(ProfilerControlEx),
                NULL, 
                0, 
                NULL
                );

    // loop through each test
    while (pLatencySetup->envOppInfo.size != 0)
        {
        // initialize some counters
        pLatencySetup->minUp = pLatencySetup->minDown = minUp = minDown = -1;
        pLatencySetup->maxUp = pLatencySetup->maxDown = maxUp = maxDown = 0;
        pLatencySetup->sumUp = pLatencySetup->sumDown = sumUp = sumDown = 0;
        
        // set current opp to the run environment opp
        KernelIoControl(IOCTL_OPP_REQUEST, &pLatencySetup->envOppInfo, 
            sizeof(IOCTL_OPP_REQUEST_IN), 0, 0, 0
            );

        for (i = 0; i < nSamples * 2; ++i)
            {
            if (i & 1)
                {
                // change OPP
                KernelIoControl(IOCTL_OPP_REQUEST, &pLatencySetup->oppAInfo, 
                    sizeof(IOCTL_OPP_REQUEST_IN), 0, 0, 0
                    );

                // get data
                profilerControlEx.dwOptions = PROFILE_OEMDEFINED;
                KernelIoControl(IOCTL_HAL_OEM_PROFILER, 
                    &profilerControlEx, 
                    sizeof(ProfilerControlEx),
                    NULL, 
                    0, 
                    NULL
                    );
                
                // save off info
                temp = profilerControlEx.OEM.rgValues[1] - profilerControlEx.OEM.rgValues[0];
                sumDown += temp;
                if (temp > maxDown) maxDown = temp;
                if (temp < minDown) minDown = temp;
                }
            else
                {
                // change OPP
                KernelIoControl(IOCTL_OPP_REQUEST, &pLatencySetup->oppBInfo, 
                    sizeof(IOCTL_OPP_REQUEST_IN), 0, 0, 0
                    );

                // get data
                profilerControlEx.dwOptions = PROFILE_OEMDEFINED;
                KernelIoControl(IOCTL_HAL_OEM_PROFILER, 
                    &profilerControlEx, 
                    sizeof(ProfilerControlEx),
                    NULL, 
                    0, 
                    NULL
                    );

                // save off info
                temp = profilerControlEx.OEM.rgValues[1] - profilerControlEx.OEM.rgValues[0];
                sumUp += temp;
                if (temp > maxUp) maxUp = temp;
                if (temp < minUp) minUp = temp;
                }

            }

        // save logs
        pLatencySetup->minUp = minUp;
        pLatencySetup->maxUp = maxUp;
        pLatencySetup->sumUp = sumUp;

        pLatencySetup->minDown = minDown;
        pLatencySetup->maxDown = maxDown;
        pLatencySetup->sumDown = sumDown;
        
        // get next latency setup
        pLatencySetup = &_rgOppLatencySetupTable[idx++];
        }

    // stop profiling
    profilerControlEx.dwOptions = PROFILE_STARTPAUSED;
    //ProfileStop();
    profilerControlEx.dwOptions = PROFILE_STOP;
    KernelIoControl(IOCTL_HAL_OEM_PROFILER, 
                &profilerControlEx, 
                sizeof(ProfilerControlEx),
                NULL, 
                0, 
                NULL
                );

    // output results
    OutputDebugString(L"SDRC Stall in usec:\r\n");
    OutputDebugString(L"OPP Transition \tmin\tmax\tavg\t\r\n");
    OutputDebugString(L"---------------------------------------------\r\n");

    // loop through and output results
    idx = 0;
    pLatencySetup = &_rgOppLatencySetupTable[idx++];
    do
        {
        // show mpu opp settings
        _stprintf(szBuffer, L"MPU OPP%d:\r\n", pLatencySetup->envOppInfo.rgOpps[0] + 1);
        OutputDebugString(szBuffer);

        // show up transition
        _stprintf(szBuffer, L"OPP1->OPP2:\t%.2f\t%.2f\t%.2f\r\n",
            (float)pLatencySetup->minUp/26.0f,
            (float)pLatencySetup->maxUp/26.0f,
            (float)pLatencySetup->sumUp/26.0f/(float)nSamples
            );
        OutputDebugString(szBuffer);

        // show down transition
        _stprintf(szBuffer, L"OPP2->OPP1:\t%.2f\t%.2f\t%.2f\r\n",
            (float)pLatencySetup->minDown/26.0f,
            (float)pLatencySetup->maxDown/26.0f,
            (float)pLatencySetup->sumDown/26.0f/(float)nSamples
            );
        OutputDebugString(szBuffer);

        pLatencySetup = &_rgOppLatencySetupTable[idx++];
        }    
        while (pLatencySetup->envOppInfo.size != 0);

        
}


//------------------------------------------------------------------------------
#define PROFILE_WAKEUP_LATENCY_MAX_IDX      6
#define PROFILE_WAKEUP_TIMER_MAX_IDX        6

//-----------------------------------------------------------------------------
typedef VOID (*PFN_FmtPuts)(WCHAR *s, ...);
typedef WCHAR* (*PFN_Gets)(WCHAR *s, int cch);

//-----------------------------------------------------------------------------
extern HANDLE GetProxyDriverHandle();

//-----------------------------------------------------------------------------
typedef struct {
    DWORD               domainState_MPU;
    DWORD               domainState_CORE;
    DWORD               domainState_PER;
    DWORD               idxProfile;
    INT                 max;
    INT                 min;
    INT                 total;
    DWORD               count;
    DWORD               sleep;
    LPCTSTR             szDescription;
} DOMAIN_PROFILE_INFO;

//-----------------------------------------------------------------------------
static 
DOMAIN_PROFILE_INFO _rgDomainProfile[] = {
    {
        D4, D4, D4, 0, 0, 0, 0, 0, 200, L"CHIP_OFF (MPU-OFF, CORE-OFF, PER-OFF)"
    }, {
        D2, D3, D4, 1, 0, 0, 0, 0, 200, L"CHIP_OSWR (MPU-CSR, CORE-OSWR, PER-OFF)"
    }, {
        D2, D2, D4, 2, 0, 0, 0, 0, 200, L"CHIP_CSWR (MPU-CSR, CORE-CSWR, PER-OFF)"
    }, {
        D2, D2, D0, 3, 0, 0, 0, 0, 200, L"CORE_CSWR (MPU-CSR, CORE-CSWR, PER-ON)"
    }, {
        D2, D0, D0, 4, 0, 0, 0, 0, 200, L"CORE_INACTIVE (MPU-CSR, CORE-ON, PER-ON)"
    }, {
        D0, D0, D0, 5, 0, 0, 0, 0, 200, L"MPU_INACTIVE (MPU-ON, CORE-ON, PER-ON)"
    }, {
         0,  0,  0, 0, 0, 0, 0, 0, 000, NULL
    }
};

//-----------------------------------------------------------------------------
void
ClearProfileControlData(
    ProfilerControlEx *pProfilerControlEx
    )
{
    for (int i = 0; i < PROFILE_COUNT; ++i)
        {
        pProfilerControlEx->OEM.rgValues[i] = 0;
        }
}

//-----------------------------------------------------------------------------
BOOL
ProfileInterruptLatency(
    HANDLE  hDomainConstraint,
    int     nTargetSamples
    )
{
    int prof;
    int loopCount;
    int nSamples;
    WCHAR szBuffer[MAX_PATH];
    PHYSICAL_ADDRESS PhysAddr;
    int val, min, max, total;
    ProfilerControlEx profilerControlEx;
    POWERDOMAIN_CONSTRAINT_INFO constraintInfo;
    OMAP_PRCM_MPU_PRM_REGS *pOMAP_PRCM_MPU_PRM;

    // get access to PRM registers directly
    PhysAddr.QuadPart = OMAP_PRCM_MPU_PRM_REGS_PA;
    pOMAP_PRCM_MPU_PRM = (OMAP_PRCM_MPU_PRM_REGS*)MmMapIoSpace(PhysAddr, OMAP_PRCM_MPU_PRM_REGS_SIZE, FALSE);

    // open a power domain constraint handle to change the power domain 
    // floors
    constraintInfo.size = sizeof(POWERDOMAIN_CONSTRAINT_INFO);
        
    // initialize variables
    memset(&profilerControlEx, 0, sizeof(ProfilerControlEx));
    profilerControlEx.dwVersion = 1;
    profilerControlEx.dwOptions = PROFILE_OEMDEFINED;
    profilerControlEx.OEM.dwControlSize = sizeof(ProfilerControlEx) - sizeof(ProfilerControl);
    profilerControlEx.OEM.dwCount = PROFILE_WAKEUP_LATENCY_MAX_IDX;
    profilerControlEx.OEM.rgTargets[0] = PROFILE_WAKEUP_LATENCY_CHIP_OFF;
    profilerControlEx.OEM.rgTargets[1] = PROFILE_WAKEUP_LATENCY_CHIP_OSWR;
    profilerControlEx.OEM.rgTargets[2] = PROFILE_WAKEUP_LATENCY_CHIP_CSWR;
    profilerControlEx.OEM.rgTargets[3] = PROFILE_WAKEUP_LATENCY_CORE_CSWR;
    profilerControlEx.OEM.rgTargets[4] = PROFILE_WAKEUP_LATENCY_CORE_INACTIVE;
    profilerControlEx.OEM.rgTargets[5] = PROFILE_WAKEUP_LATENCY_MPU_INACTIVE;

    // loop through entire profile list
    prof = 0;
    while (_rgDomainProfile[prof].szDescription != NULL)
        {
        _stprintf(szBuffer, L"profiling %s\r\n", _rgDomainProfile[prof].szDescription);
        OutputDebugString(szBuffer);
        
        // initialize variables
        _rgDomainProfile[prof].min = min = MAX_INT;
        _rgDomainProfile[prof].max = max = 0;
        _rgDomainProfile[prof].total = total = 0;
        _rgDomainProfile[prof].count = 0;

        nSamples = 0;
        loopCount = nTargetSamples * 2;
        
        // put the mpu in the correct state
        constraintInfo.powerDomain = POWERDOMAIN_MPU;
        constraintInfo.state = _rgDomainProfile[prof].domainState_MPU;
        PmxUpdateConstraint(hDomainConstraint, 
            CONSTRAINT_MSG_POWERDOMAIN_REQUEST,
            (void*)&constraintInfo, 
            sizeof(POWERDOMAIN_CONSTRAINT_INFO)
            );
        
        // put the core in the correct state
        constraintInfo.powerDomain = POWERDOMAIN_CORE;
        constraintInfo.state = _rgDomainProfile[prof].domainState_CORE;
        PmxUpdateConstraint(hDomainConstraint, 
            CONSTRAINT_MSG_POWERDOMAIN_REQUEST,
            (void*)&constraintInfo, 
            sizeof(POWERDOMAIN_CONSTRAINT_INFO)
            );

        // put the per in the correct state
        constraintInfo.powerDomain = POWERDOMAIN_PERIPHERAL;
        constraintInfo.state = _rgDomainProfile[prof].domainState_PER;
        PmxUpdateConstraint(hDomainConstraint,
            CONSTRAINT_MSG_POWERDOMAIN_REQUEST,
            (void*)&constraintInfo,
            sizeof(POWERDOMAIN_CONSTRAINT_INFO)
            );

        // clear any residual profile information        
        KernelIoControl(IOCTL_HAL_OEM_PROFILER, 
            (void*)&profilerControlEx, 
            sizeof(ProfilerControlEx), 
            NULL,
            0, 
            NULL
            );

        ClearProfileControlData(&profilerControlEx);

        // loop through requested number of samples        
        while (nSamples < nTargetSamples && loopCount > 0)
            {
            // wait for wake-up
            Sleep(_rgDomainProfile[prof].sleep);

            // get profiled information
            KernelIoControl(IOCTL_HAL_OEM_PROFILER, 
                (void*)&profilerControlEx, 
                sizeof(ProfilerControlEx), 
                NULL,
                0, 
                NULL
                );

            // log wake-up latency
            if (profilerControlEx.OEM.rgValues[_rgDomainProfile[prof].idxProfile] != 0)
                {
                val = profilerControlEx.OEM.rgValues[_rgDomainProfile[prof].idxProfile];
                if (val < min || nSamples == 0) min = val;
                if (val > max || nSamples == 0) max = val;
                total += val;
                ++nSamples;  
                }
                      
            ClearProfileControlData(&profilerControlEx);
            --loopCount;
            }

        // log information
        _rgDomainProfile[prof].min = min;
        _rgDomainProfile[prof].max = max;
        _rgDomainProfile[prof].total = total;
        _rgDomainProfile[prof].count = nSamples;

        ++prof;
        }

    // check for non-entered states
    prof = 0;
    while (_rgDomainProfile[prof].szDescription != NULL)
        {
        if (_rgDomainProfile[prof].count == 0)
            {
            _rgDomainProfile[prof].min = 0;
            _rgDomainProfile[prof].max = 0;
            }
        ++prof;
        }

    // output results
    OutputDebugString(L"Transition Results in usec:\r\n");
    OutputDebugString(L"Sleep Desc.\t\tMIN\tMAX\tAVG\tSUM\tCOUNT\r\n");
    OutputDebugString(L"-------------------------------------------------------\r\n");
    for (prof = 0; _rgDomainProfile[prof].szDescription != NULL; prof++)
        {
        _stprintf(szBuffer, L"%s\t%.2f\t%.2f\t%.2f\t%.2f\t%d\r\n",
            _rgDomainProfile[prof].szDescription,
            (float)_rgDomainProfile[prof].min/32768.0f * 1000000.0f,
            (float)_rgDomainProfile[prof].max/32768.0f * 1000000.0f,
            _rgDomainProfile[prof].count == 0 ? 0.0f : ((float)_rgDomainProfile[prof].total/(float)_rgDomainProfile[prof].count/32768.0f) * 1000000.0f,
            (float)_rgDomainProfile[prof].total/32768.0f * 1000000.0f,
            _rgDomainProfile[prof].count            
            );

        OutputDebugString(szBuffer);
        }
        
    return TRUE;
}

//------------------------------------------------------------------------------
#define CONSTRAINT_MSG_DVFS_FORCE   (0x80000000)
#define MAX_OPM kOpm6

//------------------------------------------------------------------------------
typedef struct {
    DWORD       tickMin;
    DWORD       tickMax;
    DWORD       tickTotal;
    DWORD       count;
    DWORD       failCount;
} DVFS_STRESS_TRANSITION_DATA;


// timing matrix to profile transition times
DVFS_STRESS_TRANSITION_DATA _rgTransitionTime[MAX_OPM + 1][MAX_OPM + 1];

//------------------------------------------------------------------------------
DWORD
operator-(
    LARGE_INTEGER a,
    LARGE_INTEGER b
    )
{
    return (a.LowPart - b.LowPart);
}

//------------------------------------------------------------------------------
//This function changes form Opm and stores the time tooked by te constraint adapter to change
//from one Opm to another
BOOL  
ToggleOPM(
    HANDLE                          hDvfsConstraint,
    DVFS_STRESS_TEST_PARAMETERS    *pTestParam
    )
{
    DWORD newOpm;
    DWORD delta;
    DWORD currentOpm;
    LARGE_INTEGER startTime, stopTime;
    BOOL rc = FALSE;

    currentOpm = pTestParam->_currentOpm;
    if (pTestParam->_random == TRUE)
        {
        newOpm = Random() % (MAX_OPM + 1);
        }
    else
        {
        if (currentOpm == pTestParam->_hiopm)
            {
            newOpm = pTestParam->_lowopm;
            }
        else
            {
            newOpm = pTestParam->_hiopm;
            }
        
        }

    // transition operating modes
    QueryPerformanceCounter(&startTime);
    if (PmxUpdateConstraint(hDvfsConstraint, 
        CONSTRAINT_MSG_DVFS_FORCE, 
        (void*)&newOpm, 
        sizeof(DWORD)
        ) == FALSE)
        {
        _rgTransitionTime[currentOpm][newOpm].failCount++;
        return TRUE;
        }

    QueryPerformanceCounter(&stopTime);

    delta = stopTime - startTime;

    // log profile information
    if (_rgTransitionTime[currentOpm][newOpm].tickMin > delta || 
        _rgTransitionTime[currentOpm][newOpm].count == 0)
        {
        _rgTransitionTime[currentOpm][newOpm].tickMin = delta;
        }

    if (_rgTransitionTime[currentOpm][newOpm].tickMax < delta || 
        _rgTransitionTime[currentOpm][newOpm].count == 0)
        {
        _rgTransitionTime[currentOpm][newOpm].tickMax = delta;
        }

    _rgTransitionTime[currentOpm][newOpm].tickTotal += delta;
    _rgTransitionTime[currentOpm][newOpm].count++;

    // update opm information
    pTestParam->_currentOpm = newOpm;
    return rc;
}

//------------------------------------------------------------------------------
//In this function the opm change priod is generated and also the test final results are displayed
BOOL
ProfileDVFSLatency(
    HANDLE                          hDvfsConstraint,
    DVFS_STRESS_TEST_PARAMETERS    *pTestParam
    )
{
    BOOL rc = TRUE;
    DWORD newOpm;
    DWORD tickStop;
    DWORD tickStart;    
    DWORD tickDelta;
    LARGE_INTEGER perfCounterFreq;
    DWORD tickCount = pTestParam->_duration;
    WCHAR szBuffer[MAX_PATH];

    // transition to known operating mode
    if (pTestParam->_random == TRUE)
        {
        newOpm = Random() % (MAX_OPM + 1);
        }
    else
        {
        newOpm = pTestParam->_hiopm;
        }

    if (PmxUpdateConstraint(hDvfsConstraint, 
            CONSTRAINT_MSG_DVFS_FORCE, 
            (void*)&newOpm, 
            sizeof(DWORD)
            ) == FALSE)
        {
        _stprintf(szBuffer, L"Failed to transition to initial operating mode kOpm%d", newOpm);
        OutputDebugString(szBuffer);
        return FALSE;
        }
    
    pTestParam->_currentOpm = newOpm;
    

    // loop through and perform test
    while (tickCount > 0)
        {
        tickStart = GetTickCount();
        rc = ToggleOPM(hDvfsConstraint, pTestParam);
        Sleep(pTestParam->_period);
        tickStop = GetTickCount();

        tickDelta = tickStop - tickStart;
        if (tickCount < tickDelta)
            {
            tickCount = 0;
            }
        else
            {
            tickCount -= tickDelta;
            }
        }

     // output results
     QueryPerformanceFrequency(&perfCounterFreq);
     OutputDebugString(L"Transition Results in ticks:\r\n");
     OutputDebugString(L"\tOPM(i)\tOPM(f)\tMIN\tMAX\tAVG\tCOUNT\tSUM\tFAILED:\r\n");
     OutputDebugString(L"-------------------------------------------------------\r\n");
     for (int i = 0; i <= MAX_OPM; i++)
        {
        for (int j = 0; j <= MAX_OPM; j++)
            {
            if (i != j && _rgTransitionTime[i][j].count > 0)
                {
                _stprintf(szBuffer, L"%d\t\t%d\t%d\t%d\t%d\t%d\t%d\t%d\r\n",
                    i ,
                    j ,
                    _rgTransitionTime[i][j].tickMin,
                    _rgTransitionTime[i][j].tickMax,
                    _rgTransitionTime[i][j].tickTotal/_rgTransitionTime[i][j].count,                    
                    _rgTransitionTime[i][j].count,
                    _rgTransitionTime[i][j].tickTotal,                    
                    _rgTransitionTime[i][j].failCount
                    );
                OutputDebugString(szBuffer);
                }
            }
        }

     OutputDebugString(L"Transition Results in usec:\r\n");
     OutputDebugString(L"\tOPM(i)\tOPM(f)\tMIN\tMAX\tAVG\tCOUNT\tSUM\tFAILED:\r\n");
     OutputDebugString(L"-------------------------------------------------------\r\n");
     for (int i = 0; i <= MAX_OPM; i++)
        {
        for (int j = 0; j <= MAX_OPM; j++)
            {
            if (i != j && _rgTransitionTime[i][j].count > 0)
                {
                _stprintf(szBuffer, L"%d\t\t%d\t%.2f\t%.2f\t%.2f\t%d\t%.2f\t%d\r\n",
                    i ,
                    j ,
                    (float)(_rgTransitionTime[i][j].tickMin)/(float)(perfCounterFreq.LowPart) * 1000000.0f,
                    (float)(_rgTransitionTime[i][j].tickMax)/(float)(perfCounterFreq.LowPart) * 1000000.0f,
                    ((float)(_rgTransitionTime[i][j].tickTotal)/(float)(_rgTransitionTime[i][j].count))/(float)(perfCounterFreq.LowPart) * 1000000.0f,
                    _rgTransitionTime[i][j].count,
                    (float)(_rgTransitionTime[i][j].tickTotal)/(float)(perfCounterFreq.LowPart) * 1000000.0f,                    
                    _rgTransitionTime[i][j].failCount
                    );
                OutputDebugString(szBuffer);
                }
            }
        }
    
    return TRUE;
}

//------------------------------------------------------------------------------
BOOL
ProfileWakupAccuracy(
    HANDLE hDomainConstraint,
    WAKEUPACCURACY_TEST_PARAMETERS *pTestParam
    )
{
    int prof;
    int loopCount;
    DWORD nSamples;
    WCHAR szBuffer[MAX_PATH];
    PHYSICAL_ADDRESS PhysAddr;
    int val, min, max, total;
    ProfilerControlEx profilerControlEx;
    ProfilerControlEx profilerControlExTMar;
    POWERDOMAIN_CONSTRAINT_INFO constraintInfo;
    OMAP_PRCM_MPU_PRM_REGS *pOMAP_PRCM_MPU_PRM;

    // get access to PRM registers directly
    PhysAddr.QuadPart = OMAP_PRCM_MPU_PRM_REGS_PA;
    pOMAP_PRCM_MPU_PRM = (OMAP_PRCM_MPU_PRM_REGS*)MmMapIoSpace(PhysAddr, OMAP_PRCM_MPU_PRM_REGS_SIZE, FALSE);

    // open a power domain constraint handle to change the power domain
    // floors
    constraintInfo.size = sizeof(POWERDOMAIN_CONSTRAINT_INFO);

    // initialize variables
    memset(&profilerControlEx, 0, sizeof(ProfilerControlEx));
    profilerControlEx.dwVersion = 1;
    profilerControlEx.dwOptions = PROFILE_OEMDEFINED;
    profilerControlEx.OEM.dwControlSize = sizeof(ProfilerControlEx) - sizeof(ProfilerControl);
    profilerControlEx.OEM.dwCount = PROFILE_WAKEUP_TIMER_MAX_IDX;
    profilerControlEx.OEM.rgTargets[0] = PROFILE_WAKEUP_TIMER_CHIP_OFF;
    profilerControlEx.OEM.rgTargets[1] = PROFILE_WAKEUP_TIMER_CHIP_OSWR;
    profilerControlEx.OEM.rgTargets[2] = PROFILE_WAKEUP_TIMER_CHIP_CSWR;
    profilerControlEx.OEM.rgTargets[3] = PROFILE_WAKEUP_TIMER_CORE_CSWR;
    profilerControlEx.OEM.rgTargets[4] = PROFILE_WAKEUP_TIMER_CORE_INACTIVE;
    profilerControlEx.OEM.rgTargets[5] = PROFILE_WAKEUP_TIMER_MPU_INACTIVE;

    memset(&profilerControlExTMar, 0, sizeof(ProfilerControlEx));
    profilerControlExTMar.dwVersion = 1;
    profilerControlExTMar.dwOptions = PROFILE_OEMDEFINED;
    profilerControlExTMar.OEM.dwCount = 1;
    profilerControlExTMar.OEM.dwControlSize = sizeof(ProfilerControlEx) - sizeof(ProfilerControl);
    profilerControlExTMar.OEM.rgTargets[0] = PROFILE_WAKEUP_TIMER_MATCH_ORIGINAL;

    // loop through entire profile list
    prof = 0;
    while (_rgDomainProfile[prof].szDescription != NULL)
        {
        _stprintf(szBuffer, L"timer match accuracy profiling %s\r\n", _rgDomainProfile[prof].szDescription);
        OutputDebugString(szBuffer);

        // initialize variables
        _rgDomainProfile[prof].min = min = MAX_INT;
        _rgDomainProfile[prof].max = max = 0x80000000;
        _rgDomainProfile[prof].total = total = 0;
        _rgDomainProfile[prof].count = 0;

        nSamples = 0;
        loopCount = pTestParam->_numberOfSamples * 4;

        // put the mpu in the correct state
        constraintInfo.powerDomain = POWERDOMAIN_MPU;
        constraintInfo.state = _rgDomainProfile[prof].domainState_MPU;
        PmxUpdateConstraint(hDomainConstraint,
            CONSTRAINT_MSG_POWERDOMAIN_REQUEST,
            (void*)&constraintInfo,
            sizeof(POWERDOMAIN_CONSTRAINT_INFO)
            );

        // put the core in the correct state
        constraintInfo.powerDomain = POWERDOMAIN_CORE;
        constraintInfo.state = _rgDomainProfile[prof].domainState_CORE;
        PmxUpdateConstraint(hDomainConstraint,
            CONSTRAINT_MSG_POWERDOMAIN_REQUEST,
            (void*)&constraintInfo,
            sizeof(POWERDOMAIN_CONSTRAINT_INFO)
            );

        // put the per in the correct state
        constraintInfo.powerDomain = POWERDOMAIN_PERIPHERAL;
        constraintInfo.state = _rgDomainProfile[prof].domainState_PER;
        PmxUpdateConstraint(hDomainConstraint,
            CONSTRAINT_MSG_POWERDOMAIN_REQUEST,
            (void*)&constraintInfo,
            sizeof(POWERDOMAIN_CONSTRAINT_INFO)
            );

        // clear any residual profile information
        KernelIoControl(IOCTL_HAL_OEM_PROFILER,
            (void*)&profilerControlEx,
            sizeof(ProfilerControlEx),
            NULL,
            0,
            NULL
            );

        KernelIoControl(IOCTL_HAL_OEM_PROFILER,
            (void*)&profilerControlExTMar,
            sizeof(ProfilerControlEx),
            NULL,
            0,
            NULL
            );

        ClearProfileControlData(&profilerControlEx);
        ClearProfileControlData(&profilerControlExTMar);

        // loop through requested number of samples
        while (nSamples < pTestParam->_numberOfSamples && loopCount > 0)
            {
            // wait for wake-up
            Sleep(pTestParam->_sleepPeriod);

            // get profiled information
            KernelIoControl(IOCTL_HAL_OEM_PROFILER,
                (void*)&profilerControlEx,
                sizeof(ProfilerControlEx),
                NULL,
                0,
                NULL
                );

            KernelIoControl(IOCTL_HAL_OEM_PROFILER,
                (void*)&profilerControlExTMar,
                sizeof(profilerControlExTMar),
                NULL,
                0,
                NULL
                );

            // log timer match accuracy
            if ((profilerControlEx.OEM.rgValues[_rgDomainProfile[prof].idxProfile] != 0) &&
                (profilerControlExTMar.OEM.rgValues[0] != 0))
                {
                val = profilerControlEx.OEM.rgValues[_rgDomainProfile[prof].idxProfile];
                val = val - (INT)profilerControlExTMar.OEM.rgValues[0];
                if (val < min || nSamples == 0) min = val;
                if (val > max || nSamples == 0) max = val;
                total += val;
                ++nSamples;
                }

            ClearProfileControlData(&profilerControlEx);
            ClearProfileControlData(&profilerControlExTMar);

            --loopCount;
            }

        // log information
        _rgDomainProfile[prof].min = min;
        _rgDomainProfile[prof].max = max;
        _rgDomainProfile[prof].total = total;
        _rgDomainProfile[prof].count = nSamples;

        ++prof;
        }

    // check for non-entered states
    prof = 0;
    while (_rgDomainProfile[prof].szDescription != NULL)
        {
        if (_rgDomainProfile[prof].count == 0)
            {
            _rgDomainProfile[prof].min = 0;
            _rgDomainProfile[prof].max = 0;
            }
        ++prof;
        }

    // output results
    OutputDebugString(L"Timer match results in ticks:\r\n");
    OutputDebugString(L"Sleep Desc.\t\tMIN\tMAX\tAVG\tSUM\tCOUNT\r\n");
    OutputDebugString(L"-------------------------------------------------------\r\n");
    for (prof = 0; _rgDomainProfile[prof].szDescription != NULL; prof++)
        {
        _stprintf(szBuffer, L"%s\t%5d\t%5d\t%5.2f\t%5d\t%5d\r\n",
            _rgDomainProfile[prof].szDescription,
            _rgDomainProfile[prof].min,
            _rgDomainProfile[prof].max,
            _rgDomainProfile[prof].count == 0 ? 0.0f : ((float)_rgDomainProfile[prof].total/(float)_rgDomainProfile[prof].count),
            _rgDomainProfile[prof].total,
            _rgDomainProfile[prof].count
            );

        OutputDebugString(szBuffer);
        }

    return TRUE;
}
