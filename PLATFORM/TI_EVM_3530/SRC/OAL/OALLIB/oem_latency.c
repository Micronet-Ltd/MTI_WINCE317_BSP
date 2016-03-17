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
//  File: oem_latency.c
//
#include <windows.h>
#include <nkintr.h>
#include <pkfuncs.h>

#include <bsp.h>
#include <dvfs.h>
#include <oal_prcm.h>
#include <oal_sr.h>

//-----------------------------------------------------------------------------
#define MAX_OPM                 kOpm6
#ifndef MAX_INT
#define MAX_INT                 0x7FFFFFFF
#endif

#define STATE_NOCPUIDLE         (-1)

//-----------------------------------------------------------------------------
// CSWR = Clock Stopped With Retention
// OSWR = Off State With Retention
#define LATENCY_STATE_CHIP_OFF      0   // CORE+MPU+OTHER = OFF
#define LATENCY_STATE_CHIP_OSWR     1   // CORE+OTHER = OSWR, MPU = CSWR
#define LATENCY_STATE_CHIP_CSWR     2   // CORE+OTHER = CSWR, MPU = CSWR
#define LATENCY_STATE_CORE_CSWR     3   // OTHER=OFF/OSWR/CSWR/INACTIVE, CORE = CSWR, MPU=CSWR
#define LATENCY_STATE_CORE_INACTIVE 4   // OTHER=OFF/OSWR/CSWR/INACTIVE, CORE = INACTIVE, MPU=CSWR
#define LATENCY_STATE_MPU_INACTIVE  5   // OTHER=OFF/OSWR/CSWR/INACTIVE, CORE+MPU = INACTIVE
#define LATENCY_STATE_COUNT         6

//-----------------------------------------------------------------------------
#define RNG_RESET_DELAY             (1620 * BSP_ONE_MILLISECOND_TICKS)    // l4 @ 41.5 MHZs

//-----------------------------------------------------------------------------
const DWORD                     k32khzFrequency = 32768;

//-----------------------------------------------------------------------------
// externals

//  Reference to all PRCM-PRM registers. Initialized in PrcmInit
//
extern OMAP_PRCM_PRM           *g_pPrcmPrm;

//-----------------------------------------------------------------------------
//
//  Global:  g_wakeupLatencyConstraintTickCount
//
//  latency time, in 32khz ticks, associated with current latency state
//
extern INT g_wakeupLatencyConstraintTickCount;


//-----------------------------------------------------------------------------
// typedefs and structs

typedef struct {
    float                       fixedOffset;
    float                       chipOffOffset;
    float                       chipOSWROffset;
    float                       chipCSWROffset;
    float                       coreCSWROffset;
    float                       coreInactiveOffset;
    float                       mpuInactiveOffset;
} LatencyOffsets;

typedef struct {
    DWORD                       ticks;
    float                       us;
} LatencyEntry;


//-----------------------------------------------------------------------------
// global variables

// Wakeup offset table - allows customization in the offset calculation from
// various sleep states
//
static
LatencyOffsets _rgLatencyOffsetTable[] = {
    {
      0.000335f, 0.004944f,   0.000174f,   0.000008f,  0.000045f,  0.000045f,  0.000045f       // OPM0
    },{
      0.000335f, 0.004699f,   0.000130f,   0.000008f,  0.000045f,  0.000045f,  0.000045f       // OPM1
    },{
      0.000335f, 0.002807f,   0.000100f,   0.000008f,  0.000045f,  0.000045f,  0.000045f       // OPM2
    },{
      0.000335f, 0.002807f,   0.000069f,   0.000008f,  0.000045f,  0.000045f,  0.000045f       // OPM3
    },{
      0.000335f, 0.002807f,   0.000069f,   0.000008f,  0.000045f,  0.000045f,  0.000045f       // OPM4
    },{
      0.000335f, 0.002807f,   0.000069f,   0.000008f,  0.000045f,  0.000045f,  0.000045f       // OPM5
    },{
      0.000335f, 0.002807f,   0.000069f,   0.000008f,  0.000045f,  0.000045f,  0.000045f       // OPM6
    },{
      0.0f,      0.0f,        0.0f,        0.0f,       0.0f,       0.0f,       0.0f            // OPM7
    },{
      0.0f,      0.0f,        0.0f,        0.0f,       0.0f,       0.0f,       0.0f            // OPM8
    }
};

// This table defines the possible transition states from an initial state
//
static 
int _mapLatencyTransitionTable [LATENCY_STATE_COUNT][LATENCY_STATE_COUNT + 1] = {
    {
       LATENCY_STATE_CHIP_OFF,      LATENCY_STATE_CHIP_OSWR,        LATENCY_STATE_CHIP_CSWR,        LATENCY_STATE_CORE_CSWR,        LATENCY_STATE_CORE_INACTIVE,    LATENCY_STATE_MPU_INACTIVE,     STATE_NOCPUIDLE
    }, {
       LATENCY_STATE_CHIP_OSWR,     LATENCY_STATE_CHIP_CSWR,        LATENCY_STATE_CORE_CSWR,        LATENCY_STATE_CORE_INACTIVE,    LATENCY_STATE_MPU_INACTIVE,     STATE_NOCPUIDLE
    }, {
       LATENCY_STATE_CHIP_CSWR,     LATENCY_STATE_CORE_CSWR,        LATENCY_STATE_CORE_INACTIVE,    LATENCY_STATE_MPU_INACTIVE,     STATE_NOCPUIDLE
    }, {
       LATENCY_STATE_CORE_CSWR,     LATENCY_STATE_CORE_INACTIVE,    LATENCY_STATE_MPU_INACTIVE,     STATE_NOCPUIDLE
    }, {
       LATENCY_STATE_CORE_INACTIVE, LATENCY_STATE_MPU_INACTIVE,     STATE_NOCPUIDLE
    }, {
       LATENCY_STATE_MPU_INACTIVE,  STATE_NOCPUIDLE
    }
};

// Maps all combination of opp in Vdd1/Vdd2 to an kOpm value
//
static
DWORD _mapLatencyIndex[kOppCount][kOppCount] = {
    /*
    Vdd2 = 
    kOpp1   kOpp2   kOpp3   kOpp4   kOpp5   kOpp6   kOpp7   kOpp8   kOpp9
    -----   -----   -----   -----   -----   -----   -----   -----   -----   */
    {
      0,      1,      1,      1,      1,      1,      1,      1,      1     // Vdd1 = kOpp1
    }, {
      2,      2,      2,      2,      2,      2,      2,      2,      2     // Vdd1 = kOpp2
    }, {
      3,      3,      3,      3,      3,      3,      3,      3,      3     // Vdd1 = kOpp3
    }, {
      4,      4,      4,      4,      4,      4,      4,      4,      4     // Vdd1 = kOpp4
    }, {
      5,      5,      5,      5,      5,      5,      5,      5,      5     // Vdd1 = kOpp5
    }, {
      6,      6,      6,      6,      6,      6,      6,      6,      6     // Vdd1 = kOpp6
    }, {
      6,      6,      6,      6,      6,      6,      6,      6,      6     // Vdd1 = kOpp7
    }, {
      6,      6,      6,      6,      6,      6,      6,      6,      6     // Vdd1 = kOpp8
    }, {
      6,      6,      6,      6,      6,      6,      6,      6,      6     // Vdd1 = kOpp9
    }
};


// Latency table  
//
static LatencyEntry _rgLatencyTable[kOpmCount][LATENCY_STATE_COUNT];

// off mode method
static DWORD _bOffMode_SignalMode = TRUE;

// maintains current OPP's for all voltage domains
//
static Dvfs_OperatingPoint_e _vddCore = kOpp1;
static Dvfs_OperatingPoint_e _vddMpu = kOpp3;

// maintains current power domain state for mpu and core domains
//
static CRITICAL_SECTION _csLatency;

static DWORD _suspendState = LATENCY_STATE_CHIP_OFF; /*LATENCY_STATE_CHIP_OSWR;*/
static DWORD _domainMpu = POWERSTATE_ON;
static DWORD _domainCore = POWERSTATE_ON;
static DWORD _logicCore = LOGICRETSTATE_LOGICRET_DOMAINRET;
static DWORD _domainInactiveMask = 0;
static DWORD _domainRetentionMask = 0;

// _coreDevice and _otherDevice is used even before OALWakeupLatency_Initialize
// so make sure it is initialized
static DWORD _coreDevice = 0;
static DWORD _otherDevice = 0;

static DWORD _powerDomainCoreState;
static DWORD _powerDomainMpuState;
static DWORD _powerDomainPerState;
static DWORD _powerDomainNeonState;

// Tick count on Wake-up from OFF mode, used for RNG Reset completiion delay
static DWORD s_coreOffWaitTickCount = 0;
static BOOL  s_tickRollOver = FALSE;

//-----------------------------------------------------------------------------
// prototypes
//
extern BOOL IsSmartReflexMonitoringEnabled(UINT channel);
//#define DEBUG_PRCM_SUSPEND_RESUME 1
#undef DEBUG_PRCM_SUSPEND_RESUME
#ifdef DEBUG_PRCM_SUSPEND_RESUME
    static DWORD DeviceEnabledCount[OMAP_DEVICE_COUNT];
    static DWORD SavedDeviceEnabledCount[OMAP_DEVICE_COUNT];
    static DWORD SavedSuspendState;

    static PTCHAR SavedSuspendStateName[] = {
        L"LATENCY_STATE_CHIP_OFF      0, CORE+MPU+OTHER = OFF",
        L"LATENCY_STATE_CHIP_OSWR     1, CORE+OTHER = OSWR, MPU = CSWR",
        L"LATENCY_STATE_CHIP_CSWR     2, CORE+OTHER = CSWR, MPU = CSWR",
        L"LATENCY_STATE_CORE_CSWR     3, OTHER=OFF/OSWR/CSWR/INACTIVE, CORE = CSWR, MPU=CSWR",
        L"LATENCY_STATE_CORE_INACTIVE 4, OTHER=OFF/OSWR/CSWR/INACTIVE, CORE = INACTIVE, MPU=CSWR",
        L"LATENCY_STATE_MPU_INACTIVE  5, OTHER=OFF/OSWR/CSWR/INACTIVE, CORE+MPU = INACTIVE"
    };

    extern PTCHAR DeviceNames[];
#endif
	
//-----------------------------------------------------------------------------
//
//  Function: _OALWakeupLatency_Lock
//
//  Desc:
//
static
void
_OALWakeupLatency_Lock()
{    
    // check if constraint needs to be updated
    EnterCriticalSection(&_csLatency);
}

//-----------------------------------------------------------------------------
//
//  Function: _OALWakeupLatency_Unlock
//
//  Desc:
//
static
void
_OALWakeupLatency_Unlock()
{    
    // check if constraint needs to be updated
    LeaveCriticalSection(&_csLatency);
}

//-----------------------------------------------------------------------------
//
//  Function: OALWakeupLatency_Initialize
//
//  Desc:
//      initializes the wake-up latency table for the platform.
//
BOOL
OALWakeupLatency_Initialize(
    )
{
    int i;
    DWORD voltSetup1;
    float latencyTime;
    float fixedOffset;
    float clkSetupTime;
    float clkOffSetupTime;
    float voltRetentionSetupTime;    
    float voltOffsetTime;    
    float voltOffSetupTime;    

    #ifdef DEBUG_PRCM_SUSPEND_RESUME
        for (i = 0; i < OMAP_DEVICE_COUNT; i++)
    	    DeviceEnabledCount[i] = 0;
    #endif
			
    // initialize synchronization objects
    InitializeCriticalSection(&_csLatency);

    // get sys_clk setup times for offmode and other sleep modes
    clkSetupTime = ((float)BSP_PRM_CLKSETUP)/(float)k32khzFrequency;
    clkOffSetupTime = ((float)BSP_PRM_CLKSETUP_OFFMODE)/(float)k32khzFrequency;

    // get vdd setup offset for off and sleep modes
    voltOffsetTime = (float)INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VOLTOFFSET);
    voltOffsetTime = voltOffsetTime/(float)k32khzFrequency;

    // get vdd setup time for retention mode
    voltSetup1 = HIWORD(BSP_PRM_VOLTSETUP1_INIT) + LOWORD(BSP_PRM_VOLTSETUP1_INIT);
    voltRetentionSetupTime = ((float)voltSetup1 * 8.0f)/(float)PrcmClockGetSystemClockFrequency();

    // get vdd setup time for OFF mode
    if (INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VOLTCTRL) & SEL_OFF_SIGNALLINE)
        {
        _bOffMode_SignalMode = TRUE;
        voltOffSetupTime = ((float)BSP_PRM_VOLTSETUP2)/(float)k32khzFrequency;
        }
    else
        {
        // get vdd setup time for i2c based off mode
        _bOffMode_SignalMode = FALSE;
        voltSetup1 = HIWORD(BSP_PRM_VOLTSETUP1_OFF_MODE) + LOWORD(BSP_PRM_VOLTSETUP1_OFF_MODE);
        voltOffSetupTime = ((float)voltSetup1 * 8.0f)/(float)PrcmClockGetSystemClockFrequency();
        }
    
    // Loop through each opm entry in the latency table
    for (i = 0; i <= MAX_OPM; i++)
        {
        // global latency added to all latency entries
        fixedOffset = _rgLatencyOffsetTable[i].fixedOffset;

        // chip off latency
        latencyTime = _rgLatencyOffsetTable[i].chipOffOffset + fixedOffset;
        latencyTime += voltOffsetTime + max(voltOffSetupTime,clkOffSetupTime);
        _rgLatencyTable[i][LATENCY_STATE_CHIP_OFF].us = latencyTime;
        _rgLatencyTable[i][LATENCY_STATE_CHIP_OFF].ticks = (DWORD)((latencyTime) * (float)k32khzFrequency);

        // chip oswr latency
        latencyTime = _rgLatencyOffsetTable[i].chipOSWROffset + fixedOffset;
        latencyTime += voltRetentionSetupTime + voltOffsetTime + clkSetupTime;
        _rgLatencyTable[i][LATENCY_STATE_CHIP_OSWR].us = latencyTime;
        _rgLatencyTable[i][LATENCY_STATE_CHIP_OSWR].ticks = (DWORD)((latencyTime) * (float)k32khzFrequency);

        // chip cswr latency
        latencyTime = _rgLatencyOffsetTable[i].chipCSWROffset + fixedOffset;
        latencyTime += voltRetentionSetupTime + voltOffsetTime + clkSetupTime;
        _rgLatencyTable[i][LATENCY_STATE_CHIP_CSWR].us = latencyTime;
        _rgLatencyTable[i][LATENCY_STATE_CHIP_CSWR].ticks = (DWORD)((latencyTime) * (float)k32khzFrequency);

        // core cswr latency
        latencyTime = _rgLatencyOffsetTable[i].coreCSWROffset + fixedOffset;
        _rgLatencyTable[i][LATENCY_STATE_CORE_CSWR].us = latencyTime;
        _rgLatencyTable[i][LATENCY_STATE_CORE_CSWR].ticks = (DWORD)((latencyTime) * (float)k32khzFrequency);

        // core inactive latency
        latencyTime = _rgLatencyOffsetTable[i].coreInactiveOffset + fixedOffset;
        _rgLatencyTable[i][LATENCY_STATE_CORE_INACTIVE].us = latencyTime;
        _rgLatencyTable[i][LATENCY_STATE_CORE_INACTIVE].ticks = (DWORD)((latencyTime) * (float)k32khzFrequency);

        // mpu inactive latency
        latencyTime = _rgLatencyOffsetTable[i].mpuInactiveOffset + fixedOffset;
        _rgLatencyTable[i][LATENCY_STATE_MPU_INACTIVE].us = latencyTime;
        _rgLatencyTable[i][LATENCY_STATE_MPU_INACTIVE].ticks = (DWORD)((latencyTime) * (float)k32khzFrequency);

        OALMSG(OAL_INFO, (L"opm%d: chipOffOffset:%d,%d chipOSWROffset:%d,%d chipCSWROffset:%d,%d "
                   L"coreCSWROffset:%d,%d coreInactiveOffset:%d,%d mpuInactiveOffset:%d,%d\r\n",
            i,
            (DWORD)(_rgLatencyTable[i][LATENCY_STATE_CHIP_OFF].us * 1000000.0f), _rgLatencyTable[i][LATENCY_STATE_CHIP_OFF].ticks,
            (DWORD)(_rgLatencyTable[i][LATENCY_STATE_CHIP_OSWR].us * 1000000.0f), _rgLatencyTable[i][LATENCY_STATE_CHIP_OSWR].ticks,
            (DWORD)(_rgLatencyTable[i][LATENCY_STATE_CHIP_CSWR].us * 1000000.0f), _rgLatencyTable[i][LATENCY_STATE_CHIP_CSWR].ticks,
            (DWORD)(_rgLatencyTable[i][LATENCY_STATE_CORE_CSWR].us * 1000000.0f), _rgLatencyTable[i][LATENCY_STATE_CORE_CSWR].ticks,
            (DWORD)(_rgLatencyTable[i][LATENCY_STATE_CORE_INACTIVE].us * 1000000.0f), _rgLatencyTable[i][LATENCY_STATE_CORE_INACTIVE].ticks,
            (DWORD)(_rgLatencyTable[i][LATENCY_STATE_MPU_INACTIVE].us * 1000000.0f), _rgLatencyTable[i][LATENCY_STATE_MPU_INACTIVE].ticks
            ));
        }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: OALWakeupLatency_SetOffModeConstraint
//
//  Desc:
//
//       Set the wait period for CORE to go OFF again
//       Delay for RNG reset completion in HS Device
//
VOID
OALWakeupLatency_SetOffModeConstraint(
    DWORD tcrr
    )
{
    // CORE can go OFF only after RNG Reset Complete (2 ^ 26 * L4 Clock cycles)         
    s_coreOffWaitTickCount = tcrr + RNG_RESET_DELAY;

    if (s_coreOffWaitTickCount < tcrr)
        {
        s_tickRollOver = TRUE;
        }
} 

//-----------------------------------------------------------------------------
//
//  Function: OALWakeupLatency_PushState
//
//  Desc:
//      Updates the hw and sw to put the device into a specific
//  wake-up interrupt state.  Saves the previous state into a private
//  stack structure.  This routine should only be called in OEMIdle.
//
DWORD
OALWakeupLatency_PushState(
    DWORD state
    )
{
    DWORD mpuState;
    DWORD coreState;
    DWORD perState;
    DWORD neonState;
    DWORD clkSetup;
    DWORD voltSetup1;
    DWORD voltSetup2;

    // UNDONE:
    // Need to dynamically update vdd1 setup time based on predicted sleep states

    // save current states
    _powerDomainCoreState = INREG32(&g_pPrcmPrm->pOMAP_CORE_PRM->PM_PWSTCTRL_CORE);
    _powerDomainMpuState = INREG32(&g_pPrcmPrm->pOMAP_MPU_PRM->PM_PWSTCTRL_MPU);
    _powerDomainPerState = INREG32(&g_pPrcmPrm->pOMAP_PER_PRM->PM_PWSTCTRL_PER);
    _powerDomainNeonState = INREG32(&g_pPrcmPrm->pOMAP_NEON_PRM->PM_PWSTCTRL_NEON);

    coreState = _powerDomainCoreState & ~POWERSTATE_MASK;
    mpuState = _powerDomainMpuState & ~POWERSTATE_MASK;
    perState = _powerDomainPerState & ~POWERSTATE_MASK;
    neonState = _powerDomainNeonState & ~POWERSTATE_MASK;

    // For Idle mode (other than OFF)
    clkSetup = BSP_PRM_CLKSETUP;
    voltSetup1 = BSP_PRM_VOLTSETUP1_INIT;
    voltSetup2 = 0;
    
    OALMSG(0, (L"OALWakeupLatency_PushState(%d)\r\n", state));
    // Clear the wkup status
    OUTREG32(&g_pPrcmPrm->pOMAP_WKUP_PRM->PM_WKST_WKUP,
    INREG32(&g_pPrcmPrm->pOMAP_WKUP_PRM->PM_WKST_WKUP) | CM_CLKEN_IO);

    if ((!IsSmartReflexMonitoringEnabled(kSmartReflex_Channel1)) &&
        (!IsSmartReflexMonitoringEnabled(kSmartReflex_Channel2)))
        {
        if (state == LATENCY_STATE_CHIP_OFF )
            {
            // Enable Auto OFF
            PrcmVoltSetAutoControl(AUTO_OFF_ENABLED | AUTO_SLEEP_DISABLED | AUTO_RET_DISABLED,
                                    AUTO_OFF | AUTO_SLEEP | AUTO_RET);
            }
        else if (state <= LATENCY_STATE_CHIP_CSWR )
            {
            // Enable Auto RET
            PrcmVoltSetAutoControl(AUTO_OFF_DISABLED | AUTO_SLEEP_DISABLED | AUTO_RET_ENABLED,
                                    AUTO_OFF | AUTO_SLEEP | AUTO_RET);
            }
        else
            {
            // Enable Auto sleep
            PrcmVoltSetAutoControl(AUTO_OFF_DISABLED | AUTO_SLEEP_ENABLED | AUTO_RET_DISABLED, 
                                    AUTO_OFF | AUTO_SLEEP | AUTO_RET);
            }
        }
    
    switch (state)
        {
        case LATENCY_STATE_CHIP_OFF:

            // For Off mode, the clksetup need to count for oscillator startup
            clkSetup = BSP_PRM_CLKSETUP_OFFMODE;
            voltSetup1 = BSP_PRM_VOLTSETUP1_OFF_MODE;
            voltSetup2 = BSP_PRM_VOLTSETUP2;

            coreState |= POWERSTATE_OFF;
            mpuState |= POWERSTATE_OFF;
            perState |= POWERSTATE_OFF;
            neonState |= POWERSTATE_OFF;
            OALEnableIOPadWakeup();
            OALEnableIOWakeupDaisyChain();

            // reset the RNG reset variables 
            if (dwOEMHighSecurity == OEM_HIGH_SECURITY_HS)
                {
                s_tickRollOver = FALSE;
                s_coreOffWaitTickCount = 0;
                }
            break;

        case LATENCY_STATE_CHIP_OSWR:
            // UNDONE : OSWR is not yet supported in HS device
            if (dwOEMHighSecurity == OEM_HIGH_SECURITY_GP)
                {
                coreState &= ~LOGICRETSTATE_LOGICRET_DOMAINRET;
                coreState |= POWERSTATE_RETENTION;
                mpuState |= POWERSTATE_RETENTION;
                perState |= POWERSTATE_RETENTION;
                neonState |= POWERSTATE_RETENTION;
                OALEnableIOPadWakeup(); 
                break;
                }

        case LATENCY_STATE_CHIP_CSWR:
            coreState |= LOGICRETSTATE_LOGICRET_DOMAINRET;
            coreState |= POWERSTATE_RETENTION;
            mpuState |= POWERSTATE_RETENTION;
            perState |= POWERSTATE_RETENTION;
            neonState |= POWERSTATE_RETENTION;
            break;

        case LATENCY_STATE_CORE_CSWR:
            coreState |= LOGICRETSTATE_LOGICRET_DOMAINRET;
            coreState |= POWERSTATE_RETENTION;
            mpuState |= _powerDomainMpuState & POWERSTATE_MASK;
            perState |= POWERSTATE_ON;
            neonState |= _powerDomainNeonState & POWERSTATE_MASK;
            break;

        case LATENCY_STATE_CORE_INACTIVE:
            coreState |= POWERSTATE_ON;
            mpuState |= _powerDomainMpuState & POWERSTATE_MASK;
            perState |= POWERSTATE_ON;
            neonState |= _powerDomainNeonState & POWERSTATE_MASK;
            break;

        case LATENCY_STATE_MPU_INACTIVE:
            coreState |= POWERSTATE_ON;
            mpuState |= POWERSTATE_ON;
            perState |= POWERSTATE_ON;
            neonState |= POWERSTATE_ON;
            break;
        }

    OUTREG32(&g_pPrcmPrm->pOMAP_CORE_PRM->PM_PWSTCTRL_CORE, coreState);
    OUTREG32(&g_pPrcmPrm->pOMAP_MPU_PRM->PM_PWSTCTRL_MPU, mpuState);
    OUTREG32(&g_pPrcmPrm->pOMAP_PER_PRM->PM_PWSTCTRL_PER, perState);
    OUTREG32(&g_pPrcmPrm->pOMAP_NEON_PRM->PM_PWSTCTRL_NEON, neonState);

    // update clock setup and vdd setup times
    OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_CLKSETUP, clkSetup);
    OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VOLTSETUP2, voltSetup2);
    if (_bOffMode_SignalMode == FALSE)
        {
        OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VOLTSETUP1, voltSetup1);
        }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: OALWakeupLatency_PopState
//
//  Desc:
//      Updates the hw and sw to rollback the device from a wake-up interrupt
//  state.
//
void
OALWakeupLatency_PopState()
{
    // change states back to previous states.  This should be done
    // by hitting the hardware registers directly so that the prcm library
    // structures are not modified.

    // this is only called in OEMIdle and OEMPowerOff
    OUTREG32(&g_pPrcmPrm->pOMAP_CORE_PRM->PM_PWSTCTRL_CORE, _powerDomainCoreState);
    OUTREG32(&g_pPrcmPrm->pOMAP_MPU_PRM->PM_PWSTCTRL_MPU, _powerDomainMpuState);
    OUTREG32(&g_pPrcmPrm->pOMAP_PER_PRM->PM_PWSTCTRL_PER, _powerDomainPerState);
    OUTREG32(&g_pPrcmPrm->pOMAP_NEON_PRM->PM_PWSTCTRL_NEON, _powerDomainNeonState);
    
    if (OALIOPadWakeupEnableStatus())
        {
        // Disable IO PAD wakeup 
        OALDisableIOWakeupDaisyChain();
        OALDisableIOPadWakeup();
        }
    
    // restore vdd setup time if in i2c mode
    if (_bOffMode_SignalMode == FALSE)
        {
        OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VOLTSETUP1, 
            BSP_PRM_VOLTSETUP1_INIT
            );
        }
}

//-----------------------------------------------------------------------------
//
//  Function: OALWakeupLatency_GetCurrentState
//
//  Desc:
//      returns the current state of the wakeup latency
//
DWORD
OALWakeupLatency_GetCurrentState()
{
    DWORD rc;

    if (_domainMpu == POWERSTATE_INACTIVE)
        {
        // LATENCY_STATE_MPU_INACTIVE state
        //
        rc = LATENCY_STATE_MPU_INACTIVE;
        }
    else if (_domainCore == POWERSTATE_INACTIVE || _coreDevice != 0)
        {
        // LATENCY_STATE_CORE_INACTIVE state
        //
        rc = LATENCY_STATE_CORE_INACTIVE;
        }
    else if (_domainInactiveMask != 0 || _otherDevice != 0)
        {
        // LATENCY_STATE_CORE_CSWR,
        //
        rc = LATENCY_STATE_CORE_CSWR;
        }
    else if (_domainCore == POWERSTATE_RETENTION || _domainMpu == POWERSTATE_RETENTION || _domainRetentionMask != 0 || _vddMpu > kOpp2)
        {
        // LATENCY_STATE_CHIP_OSWR, LATENCY_STATE_CHIP_CSWR state
        //
        rc = (_logicCore == LOGICRETSTATE_LOGICRET_DOMAINRET) ? LATENCY_STATE_CHIP_CSWR : LATENCY_STATE_CHIP_OSWR;
        }
    else
        {
        rc = LATENCY_STATE_CHIP_OFF;
        }

    OALMSG(0, (L"OALWakeupLatency_GetCurrentState(%d)\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function: OALWakeupLatency_GetDelayInTicks
//
//  Desc:
//      Returns the wake-up latency time based on ticks
//
INT
OALWakeupLatency_GetDelayInTicks(
    DWORD state
    )
{
    if (state == STATE_NOCPUIDLE) return MAX_INT;
    return _rgLatencyTable[_mapLatencyIndex[_vddMpu][_vddCore]][state].ticks;
}

//-----------------------------------------------------------------------------
//
//  Function: OALWakeupLatency_FindStateByMaxDelayInTicks
//
//  Desc:
//      Returns the best match latency-state based on current state of
//  active devices and mpu/core domains.
//
DWORD
OALWakeupLatency_FindStateByMaxDelayInTicks(
    INT delayTicks
    )
{
    DWORD i;
    int *rgStateTransitions = _mapLatencyTransitionTable[OALWakeupLatency_GetCurrentState()];
    LatencyEntry *rgLatencies = _rgLatencyTable[_mapLatencyIndex[_vddMpu][_vddCore]];
    for (i = 0; rgStateTransitions[i] != STATE_NOCPUIDLE; ++i)
        {
        if ((DWORD)delayTicks >= rgLatencies[rgStateTransitions[i]].ticks)
            {
            return rgStateTransitions[i];
            }
        }
    return STATE_NOCPUIDLE;
}

//-----------------------------------------------------------------------------
//
//  Function: OALWakeupLatency_UpdateOpp
//
//  Desc:
//      updates the latency state machine with the OPP of a
//  voltage domain.
//
BOOL
OALWakeupLatency_UpdateOpp(
    DWORD *rgDomains,
    DWORD *rgOpps,
    DWORD  count
    )
{
    DWORD i;
    
    for (i = 0; i < count; ++i)
        {
        switch (rgDomains[i])
            {
            case DVFS_MPU1_OPP:
                _vddMpu = rgOpps[i];
                break;

            case DVFS_CORE1_OPP:
                _vddCore = rgOpps[i];                
                break;
            }
        }
    OALMSG(0, (L"OALWakeupLatency_UpdateOpp(%d, %d)\r\n", _vddMpu, _vddCore));
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: OALWakeupLatency_UpdateDomainState
//
//  Desc:
//      updates the latency state machine with the state information of a
//  power domain.
//
BOOL
OALWakeupLatency_UpdateDomainState(
    DWORD powerDomain,
    DWORD powerState,
    DWORD logicState
    )
{
    // only care about MPU and CORE power domain states
    OALMSG(0, (L"OALWakeupLatency_UpdateDomainState(%d, %d)\r\n", powerDomain, powerState));
    switch (powerDomain)
        {
        case POWERDOMAIN_MPU:
            if ((powerState == POWERSTATE_RETENTION && logicState != LOGICRETSTATE) ||
                 powerState == POWERSTATE_OFF)
                {
                _domainMpu = POWERSTATE_OFF;
                }
            else
                {
                _domainMpu = powerState == POWERSTATE_ON ? POWERSTATE_INACTIVE : powerState;
                }
            break;

        case POWERDOMAIN_CORE:
            _domainCore = powerState == POWERSTATE_ON ? POWERSTATE_INACTIVE : powerState;
            _logicCore = logicState;
            break;

        default:
            if (powerState == POWERSTATE_ON)
                {
                _domainInactiveMask |= (1 << powerDomain);
                }
            else
                {
                _domainInactiveMask &= ~(1 << powerDomain);
                }

            if (powerState == POWERSTATE_RETENTION)
                {
                _domainRetentionMask |= (1 << powerDomain);
                }
            else
                {
                _domainRetentionMask &= ~(1 << powerDomain);
                }
            break;
        }
  
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: OALWakeupLatency_DeviceEnabled
//
//  Desc:
//      notification from prcm device manager when a hardware device is
//  enabled or disabled.
//
BOOL
OALWakeupLatency_DeviceEnabled(
    DWORD devId,
    BOOL bEnabled
    )
{
   
    OALMSG(0, (L"OALWakeupLatency_DeviceEnabled(%X, %d)\r\n", devId, bEnabled));
    switch (devId)
        {
        case OMAP_DEVICE_I2C1:
        case OMAP_DEVICE_I2C2:
        case OMAP_DEVICE_I2C3:
        case OMAP_DEVICE_MMC1:
        case OMAP_DEVICE_MMC2:
        case OMAP_DEVICE_MMC3:
        case OMAP_DEVICE_USBTLL:
        case OMAP_DEVICE_HDQ:
        case OMAP_DEVICE_MCBSP1:
        case OMAP_DEVICE_MCBSP5:
        case OMAP_DEVICE_MCSPI1:
        case OMAP_DEVICE_MCSPI2:
        case OMAP_DEVICE_MCSPI3:
        case OMAP_DEVICE_MCSPI4:
        case OMAP_DEVICE_UART1:
        case OMAP_DEVICE_UART2:
        case OMAP_DEVICE_TS:
        case OMAP_DEVICE_GPTIMER10:
        case OMAP_DEVICE_GPTIMER11:
        case OMAP_DEVICE_MSPRO:
        case OMAP_DEVICE_EFUSE:
        case OMAP_DEVICE_SR1:
        case OMAP_DEVICE_SR2:
            if (bEnabled == TRUE)
                {
                #ifdef DEBUG_PRCM_SUSPEND_RESUME
                    DeviceEnabledCount[devId]++;
				#endif
                ++_coreDevice;
                }
            else
                {
                #ifdef DEBUG_PRCM_SUSPEND_RESUME
                    if (DeviceEnabledCount[devId] > 0)
                        DeviceEnabledCount[devId]--;
				#endif
                if (_coreDevice > 0) 
				    --_coreDevice;
                }
            break;

        /*
        GPIO clocks not included, always on unless in suspend
        case OMAP_DEVICE_GPIO2:
        case OMAP_DEVICE_GPIO3:
        case OMAP_DEVICE_GPIO4:
        case OMAP_DEVICE_GPIO5:
        case OMAP_DEVICE_GPIO6:
        */
        case OMAP_DEVICE_MCBSP2:          
        case OMAP_DEVICE_MCBSP3:
        case OMAP_DEVICE_MCBSP4:
        case OMAP_DEVICE_GPTIMER2:
        case OMAP_DEVICE_GPTIMER3:
        case OMAP_DEVICE_GPTIMER4:
        case OMAP_DEVICE_GPTIMER5:
        case OMAP_DEVICE_GPTIMER6:
        case OMAP_DEVICE_GPTIMER7: 
        case OMAP_DEVICE_GPTIMER8:
        case OMAP_DEVICE_GPTIMER9:    
        case OMAP_DEVICE_UART3:
        case OMAP_DEVICE_UART4:
        case OMAP_DEVICE_WDT3:       
        case OMAP_DEVICE_DSS: 
        case OMAP_DEVICE_DSS1:
        case OMAP_DEVICE_DSS2:
        case OMAP_DEVICE_TVOUT:
        case OMAP_DEVICE_CAMERA: 
        case OMAP_DEVICE_CSI2: 
        case OMAP_DEVICE_DSP:
        case OMAP_DEVICE_2D: 
        case OMAP_DEVICE_3D:
        case OMAP_DEVICE_SGX:
        case OMAP_DEVICE_HSUSB1: 
        case OMAP_DEVICE_HSUSB2: 
        case OMAP_DEVICE_USBHOST1: 
        case OMAP_DEVICE_USBHOST2:
        case OMAP_DEVICE_USBHOST3:
            if (bEnabled == TRUE)
                {
                #ifdef DEBUG_PRCM_SUSPEND_RESUME
                    DeviceEnabledCount[devId]++;
                #endif
                ++_otherDevice;
                }
            else
                {
                #ifdef DEBUG_PRCM_SUSPEND_RESUME
                    if (DeviceEnabledCount[devId] > 0)
                        DeviceEnabledCount[devId]--;
                #endif
                if (_otherDevice > 0) 
    				--_otherDevice;
                }
            break; 
        }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: OALWakeupLatency_IsChipOff
//
//  Desc:
//      returns CORE OFF status for a particular latency state
//
BOOL
OALWakeupLatency_IsChipOff(
    DWORD latencyState
    )
{
    OALMSG(0, (L"OALWakeupLatency_IsChipOff(%d)\r\n", latencyState));
    return  (latencyState == LATENCY_STATE_CHIP_OFF);
}

//-----------------------------------------------------------------------------
//
//  Function: OALWakeupLatency_GetSuspendState
//
//  Desc:
//      returns the default suspend state
//
DWORD
OALWakeupLatency_GetSuspendState(
    )
{
    DWORD rc = _suspendState;
    DWORD domainMask = _domainInactiveMask | _domainRetentionMask;

    // ignore peripheral and neon power state
    domainMask &= ~(1 << POWERDOMAIN_NEON);
    domainMask &= ~(1 << POWERDOMAIN_PERIPHERAL);

#if defined (DEBUG_PRCM_SUSPEND_RESUME)
	{
		int i;
		OALMSG(1, (L"\r\OALWakeupLatency_GetSuspendState: Enabled devices\r\n"));

		for (i = 0; i < OMAP_DEVICE_COUNT; i++)
			if (DeviceEnabledCount[i])
				OALMSG(1, (L"%s(%d)\r\n", DeviceNames[i], DeviceEnabledCount[i]));
	}
	OALMSG(1, (L"OALWakeupLatency_GetSuspendState: CS(M(%X), C(%X), P(%X), O(%X), IM(%X), RM(%X), (%X %X))\r\n",
		_domainMpu, _domainCore, _coreDevice, _otherDevice, _domainInactiveMask, _domainRetentionMask, _vddMpu, kOpp2));
#endif

    // check if there's something which may prevent a lower sleep state
    if (domainMask != 0 || _otherDevice != 0 || _coreDevice != 0)
        {
        rc = max(rc, OALWakeupLatency_GetCurrentState());
        }

   return rc;
}

//-----------------------------------------------------------------------------
//
//  Function: OALWakeupLatency_SetSuspendState
//
//  Desc:
//      returns the default suspend state
//
BOOL
OALWakeupLatency_SetSuspendState(
    DWORD suspendState
    )
{
    BOOL rc = FALSE;
    if (suspendState < LATENCY_STATE_COUNT)
        {
        _suspendState = suspendState;
        rc = TRUE;
        }

     OALMSG(0, (L"OALWakeupLatency_SetSuspendState(%d)\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  OALIoCtlInterruptLatencyConstraint
//
//  updates the current interrupt latency constraint
//
BOOL 
OALIoCtlInterruptLatencyConstraint(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    )
{
    BOOL rc = FALSE;
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlInterruptLatencyConstraint\r\n"));

    if (pInBuffer == NULL || inSize != sizeof(float)) goto cleanUp;

    // check if pInBuffer is less than 0 which means to release 
    // interrupt constraints
    _OALWakeupLatency_Lock();
    if (*(float*)pInBuffer < 0.0f)
        {
        g_wakeupLatencyConstraintTickCount = MAX_INT;
        }
    else
        {
        g_wakeupLatencyConstraintTickCount = (DWORD)(*(float*)pInBuffer * (float)k32khzFrequency);
        }
    _OALWakeupLatency_Unlock();
    
    rc = TRUE;
    
cleanUp:    
    OALMSG(OAL_INTR&&OAL_FUNC, 
        (L"-OALIoCtlInterruptLatencyConstraint(rc = %d)\r\n", rc)
        );
    
    return rc;
}

//-----------------------------------------------------------------------------

#ifdef DEBUG_PRCM_SUSPEND_RESUME

    // save values for later display
    VOID OALWakeupLatency_SaveSnapshot()
    {
        int i;
	
        SavedSuspendState = OALWakeupLatency_GetSuspendState();

        for (i = 0; i < OMAP_DEVICE_COUNT; i++)
	    {
            // latency ref count
    	    SavedDeviceEnabledCount[i] = DeviceEnabledCount[i];
	    }
    }

    // dump saved values
    VOID OALWakeupLatency_DumpSnapshot()
    {
        int i;
	
        OALMSG(1, (L"\r\nOALWakeupLatency Saved Snapshot::\r\n"));

        OALMSG(1, (L"\r\nSaved Non-Zero Interrupt Latency Device Enabled Counts:\r\n"));
        for (i = 0; i < OMAP_DEVICE_COUNT; i++)
            if (SavedDeviceEnabledCount[i])
    	        OALMSG(1, (L"Saved Interrupt Latency Device Enabled Count %s = %d\r\n", DeviceNames[i], SavedDeviceEnabledCount[i]));

        OALMSG(1, (L"\r\nSaved suspend state: %d, %s\r\n", SavedSuspendState, SavedSuspendStateName[SavedSuspendState]));
    }

#else

    VOID OALWakeupLatency_SaveSnapshot()
    {
    }

    VOID OALWakeupLatency_DumpSnapshot()
    {
    }

#endif
