// All rights reserved ADENEO EMBEDDED 2010
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
//  File: prcm_domain.c
//
#include "omap.h"
#include "omap_prof.h"
#include "omap3530.h"
#include <nkintr.h>
#include <pkfuncs.h>
#include "oalex.h"
#include "prcm_priv.h"


//-----------------------------------------------------------------------------

ClockDomainInfo_t _CoreClockDomain = { 
    3, 
    {   
        {
        CLOCKDOMAIN_L3, CLKSTCTRL_DISABLED >> CLKSTCTRL_SHIFT,  0
        }, {
        CLOCKDOMAIN_L4, CLKSTCTRL_DISABLED >> CLKSTCTRL_SHIFT,  CLKSTCTRL_CORE_L4_SHIFT
        }, {
        CLOCKDOMAIN_D2D, CLKSTCTRL_DISABLED >> CLKSTCTRL_SHIFT,  CLKSTCTRL_CORE_D2D_SHIFT
        }
    }};

ClockDomainInfo_t _PeripheralClockDomain = { 
    1, 
    {
        {
        CLOCKDOMAIN_PERIPHERAL, CLKSTCTRL_DISABLED >> CLKSTCTRL_SHIFT,  0
        }
    }};

ClockDomainInfo_t _UsbHostClockDomain = { 
    1, 
    {   
        {
        CLOCKDOMAIN_USBHOST, CLKSTCTRL_DISABLED >> CLKSTCTRL_SHIFT,  0
        }
    }};

ClockDomainInfo_t _EmulationClockDomain = { 
    1, 
    {   
        {
        CLOCKDOMAIN_EMULATION, CLKSTCTRL_DISABLED >> CLKSTCTRL_SHIFT,  0
        }
    }};    

ClockDomainInfo_t _MpuClockDomain = { 
    1, 
    {
        {
        CLOCKDOMAIN_MPU, CLKSTCTRL_DISABLED >> CLKSTCTRL_SHIFT,  0
        }
    }};

ClockDomainInfo_t _DssClockDomain = { 
    1, 
    {
        {
        CLOCKDOMAIN_DSS, CLKSTCTRL_DISABLED >> CLKSTCTRL_SHIFT,  0
        }
    }};

ClockDomainInfo_t _NeonClockDomain = { 
    1, 
    {
        {
        CLOCKDOMAIN_NEON, CLKSTCTRL_DISABLED >> CLKSTCTRL_SHIFT,  0
        }
    }};

ClockDomainInfo_t _Iva2ClockDomain = { 
    1, 
    {
        {
        CLOCKDOMAIN_IVA2, CLKSTCTRL_DISABLED >> CLKSTCTRL_SHIFT,  0
        }
    }};

ClockDomainInfo_t _CameraClockDomain = { 
    1, 
    {
        {
        CLOCKDOMAIN_CAMERA, CLKSTCTRL_DISABLED >> CLKSTCTRL_SHIFT,  0
        }
    }};

ClockDomainInfo_t _SgxClockDomain = { 
    1, 
    {
        {
        CLOCKDOMAIN_SGX, CLKSTCTRL_DISABLED >> CLKSTCTRL_SHIFT,  0
        }
    }};

//-----------------------------------------------------------------------------
PowerDomainState_t _WakeupPowerDomain = {
    POWERSTATE_ON >> POWERSTATE_SHIFT,
    LOGICRETSTATE_LOGICRET_DOMAINRET,
    0,
    0
    };

PowerDomainState_t _CorePowerDomain = {
    POWERSTATE_ON >> POWERSTATE_SHIFT,
    LOGICRETSTATE_LOGICRET_DOMAINRET,
    0,
    0
    };

PowerDomainState_t _PeripheralPowerDomain = {
    POWERSTATE_ON >> POWERSTATE_SHIFT,
    LOGICRETSTATE_LOGICRET_DOMAINRET,
    0,
    WKDEP_EN_WKUP | WKDEP_EN_IVA2 | WKDEP_EN_MPU | WKDEP_EN_CORE
    };

PowerDomainState_t _UsbHostPowerDomain = {
    POWERSTATE_ON >> POWERSTATE_SHIFT,
    LOGICRETSTATE_LOGICRET_DOMAINRET,
    0,
    WKDEP_EN_WKUP | WKDEP_EN_IVA2 | WKDEP_EN_MPU | WKDEP_EN_CORE
    };

PowerDomainState_t _MpuPowerDomain = {
    POWERSTATE_ON >> POWERSTATE_SHIFT,
    LOGICRETSTATE_LOGICRET_DOMAINRET,
    0,
    WKDEP_EN_PER | WKDEP_EN_DSS | WKDEP_EN_IVA2 | WKDEP_EN_CORE
    };

PowerDomainState_t _DssPowerDomain = {
    POWERSTATE_ON >> POWERSTATE_SHIFT,
    LOGICRETSTATE_LOGICRET_DOMAINRET,
    0,
    WKDEP_EN_WKUP | WKDEP_EN_IVA2 | WKDEP_EN_MPU
    };

PowerDomainState_t _NeonPowerDomain = {
    POWERSTATE_ON >> POWERSTATE_SHIFT,
    LOGICRETSTATE_LOGICRET_DOMAINRET,
    0,
    WKDEP_EN_MPU
    };

PowerDomainState_t _Iva2PowerDomain = {
    POWERSTATE_ON >> POWERSTATE_SHIFT,
    LOGICRETSTATE_LOGICRET_DOMAINRET,
    0,
    WKDEP_EN_PER | WKDEP_EN_DSS | WKDEP_EN_WKUP | WKDEP_EN_MPU | WKDEP_EN_CORE
    };

PowerDomainState_t _CameraPowerDomain = {
    POWERSTATE_ON >> POWERSTATE_SHIFT,
    LOGICRETSTATE_LOGICRET_DOMAINRET,
    0,
    WKDEP_EN_WKUP | WKDEP_EN_IVA2 | WKDEP_EN_MPU
    };

PowerDomainState_t _SgxPowerDomain = {
    POWERSTATE_ON >> POWERSTATE_SHIFT,
    LOGICRETSTATE_LOGICRET_DOMAINRET,
    0,
    WKDEP_EN_WKUP | WKDEP_EN_IVA2 | WKDEP_EN_MPU
    };

//-----------------------------------------------------------------------------
DomainMap s_DomainTable = {
    {   // POWERDOMAIN_WAKEUP
        0,                  
        0,  
        &_WakeupPowerDomain,        
        NULL,
        { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF }
        
    }, {// POWERDOMAIN_CORE
        0,  
        DOMAIN_UPDATE_POWERSTATE | DOMAIN_UPDATE_CLOCKSTATE,
        &_CorePowerDomain,          
        &_CoreClockDomain ,
        { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF }          
        
    }, {// POWERDOMAIN_PERIPHERAL
        0,  
        DOMAIN_UPDATE_ALL,
        &_PeripheralPowerDomain,    
        &_PeripheralClockDomain,
        { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF }     
        
    }, {// POWERDOMAIN_USBHOST
        0,  
        DOMAIN_UPDATE_ALL,
        &_UsbHostPowerDomain,       
        &_UsbHostClockDomain,
        { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF }        
        
    }, {// POWERDOMAIN_EMULATION
        0,  
        DOMAIN_UPDATE_CLOCKSTATE,
        NULL,                       
        &_EmulationClockDomain,
        { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF }      
        
    }, {// POWERDOMAIN_MPU
        0,  
        DOMAIN_UPDATE_POWERSTATE | DOMAIN_UPDATE_CLOCKSTATE | DOMAIN_UPDATE_WKUPDEP,
        &_MpuPowerDomain,           
        &_MpuClockDomain,
        { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF }  
        
    }, {// POWERDOMAIN_DSS
        0, 
        DOMAIN_UPDATE_ALL,
        &_DssPowerDomain,           
        &_DssClockDomain,
        { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF }  
        
    }, {// POWERDOMAIN_NEON
        0,  
        DOMAIN_UPDATE_POWERSTATE | DOMAIN_UPDATE_CLOCKSTATE | DOMAIN_UPDATE_WKUPDEP,
        &_NeonPowerDomain,          
        &_NeonClockDomain,
        { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF }  
        
    }, {// POWERDOMAIN_IVA2
        0,  
        DOMAIN_UPDATE_POWERSTATE | DOMAIN_UPDATE_CLOCKSTATE | DOMAIN_UPDATE_WKUPDEP,
        &_Iva2PowerDomain,          
        &_Iva2ClockDomain,
        { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF }  
        
    }, {// POWERDOMAIN_CAMERA      
        0,  
        DOMAIN_UPDATE_ALL,
        &_CameraPowerDomain,        
        &_CameraClockDomain,
        { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF }        
        
    }, {// POWERDOMAIN_SGX
        0,  
        DOMAIN_UPDATE_ALL,
        &_SgxPowerDomain,           
        &_SgxClockDomain,
        { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF }  
        
    }, {// POWERDOMAIN_EFUSE
        0,  
        0,
        NULL,                       
        NULL,
        { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF }                
        
    }, {// POWERDOMAIN_SMARTREFLEX    
        0,  
        0,
        NULL,                       
        NULL,
        { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF }
    }
};

//-----------------------------------------------------------------------------
static
BOOL
_DomainInitialize(
    PowerDomainState_t *pDomain,
    UINT                pm_pwstctrl,
    UINT                pm_wkdep,
    UINT                cm_sleepdep    
    )
{
    BOOL rc = TRUE;
    OALMSG(OAL_FUNC, (L"+_DomainInitialize("
        L"pDomain=0x%08X, pm_pwstctrl=0x%08X"
        L"pm_wkdep=0x%08X, cm_sleepdep=0x%08X)\r\n", 
        pDomain, pm_pwstctrl, cm_sleepdep)
        );

    // all values are normalized and then cached in SDRAM
    if (pDomain == NULL) goto cleanUp;
    
    // domain power state
    pDomain->powerState = (pm_pwstctrl & POWERSTATE_MASK) >> POWERSTATE_SHIFT;

    // wake and sleep dependencies
    pDomain->wakeDependency = (pm_wkdep & WKDEP_MASK) >> WKDEP_SHIFT;
    pDomain->sleepDependency = (cm_sleepdep & SLEEPDEP_MASK) >> SLEEPDEP_SHIFT;

cleanUp:
    OALMSG(OAL_FUNC, (L"-_DomainInitialize()=%d\r\n", rc));
    return rc;    
}

//-----------------------------------------------------------------------------
static
BOOL
_DomainClockRestore(
    UINT powerDomain
    )
{
    UINT i;
    UINT val;
    UINT parentClock;
    SrcClockDivisorTable_t *pDivisors;

    extern SrcClockMap s_SrcClockTable;

    switch (powerDomain)
        {
        case POWERDOMAIN_PERIPHERAL:
            // build mask
            val = s_SrcClockTable[kGPT2_ALWON_FCLK].parentClk == kSYS_CLK ? CLKSEL_GPT2 : 0;
            val |= s_SrcClockTable[kGPT3_ALWON_FCLK].parentClk == kSYS_CLK ? CLKSEL_GPT3 : 0;
            val |= s_SrcClockTable[kGPT4_ALWON_FCLK].parentClk == kSYS_CLK ? CLKSEL_GPT4 : 0;
            val |= s_SrcClockTable[kGPT5_ALWON_FCLK].parentClk == kSYS_CLK ? CLKSEL_GPT5 : 0;
            val |= s_SrcClockTable[kGPT6_ALWON_FCLK].parentClk == kSYS_CLK ? CLKSEL_GPT6 : 0;
            val |= s_SrcClockTable[kGPT7_ALWON_FCLK].parentClk == kSYS_CLK ? CLKSEL_GPT7 : 0;
            val |= s_SrcClockTable[kGPT8_ALWON_FCLK].parentClk == kSYS_CLK ? CLKSEL_GPT8 : 0;
            val |= s_SrcClockTable[kGPT9_ALWON_FCLK].parentClk == kSYS_CLK ? CLKSEL_GPT9 : 0;
            OUTREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER, val);
            break;

        case POWERDOMAIN_DSS:
            // write to hw
            OUTREG32(&g_pPrcmCm->pOMAP_DSS_CM->CM_CLKSEL_DSS,
                CLKSEL_DSS1(s_SrcClockTable[kDSS1_ALWON_FCLK].pDivisors->SourceClock[0].divisor) |
                CLKSEL_TV(s_SrcClockTable[k54M_FCLK].pDivisors->SourceClock[0].divisor));
            break;
 
        case POWERDOMAIN_CAMERA:
            // write to hw
            OUTREG32(&g_pPrcmCm->pOMAP_CAM_CM->CM_CLKSEL_CAM, 
                CLKSEL_CAM(s_SrcClockTable[kCAM_MCLK].pDivisors->SourceClock[0].divisor)
                );
            break;

        case POWERDOMAIN_SGX:
            // verify parent clock is valid       
            pDivisors = s_SrcClockTable[kSGX_FCLK].pDivisors;
            parentClock = s_SrcClockTable[kSGX_FCLK].parentClk;
            for (i = 0; i < pDivisors->count; ++i)
                {
                if (parentClock == pDivisors->SourceClock[i].id)
                    {
                    // write to hw
                    OUTREG32(&g_pPrcmCm->pOMAP_SGX_CM->CM_CLKSEL_SGX, 
                        CLKSEL_SGX(pDivisors->SourceClock[i].divisor)
                        );
                    break;
                    }
                }
            break;
        case POWERDOMAIN_IVA2:
            PrcmClockRestoreDpllState(kDPLL2);
           break;
        }

    return TRUE;
}

//-----------------------------------------------------------------------------
static
BOOL
_PrcmDomainClockInitialize(
    ClockDomainInfo_t  *pClockStates,
    UINT                cm_clkstctrl
    )
{
    UINT i;
    UINT temp;
    BOOL rc = TRUE;
    OALMSG(OAL_FUNC, (L"+_PrcmDomainClockInitialize("
        L"pClockStates=0x%08X, cm_clkstctrl=0x%08X\r\n", 
        pClockStates,  cm_clkstctrl)
        );

    // all values are normalized and then cached in SDRAM
    if (pClockStates == NULL) goto cleanUp;

    for (i = 0; i < pClockStates->count; ++i)
        {  
        temp = cm_clkstctrl >> pClockStates->rgClockDomains[i].clockShift;        
        pClockStates->rgClockDomains[i].clockState = (temp & CLKSTCTRL_MASK) >> CLKSTCTRL_SHIFT; 
        }
    
cleanUp:
    OALMSG(OAL_FUNC, (L"-_PrcmDomainClockInitialize()=%d\r\n", rc));
    return rc;    
}

//-----------------------------------------------------------------------------
static
BOOL
_PrcmDomainHwUpdate(
    UINT powerDomain,
    UINT ffMask
    )
{
    BOOL rc = TRUE;
    UINT pm_wkdep;
    UINT pm_pwstctrl;
    UINT cm_sleepdep;
    UINT cm_clkstctrl;   
    OMAP_CM_REGS   *pCmRegs;
    OMAP_PRM_REGS  *pPrmRegs;
    ClockDomainInfo_t  *pClockStates;    
    PowerDomainState_t *pDomainState;
    
    // update the following hw registers
    // PM_WKDEP_xxx
    // CM_SLEEPDEP_xxx
    // PM_PWSTCTRL_xxx.POWERSTATE
    // CM_CLKSTCTRL_xxx

    pCmRegs = GetCmRegisterSet(powerDomain);
    pPrmRegs = GetPrmRegisterSet(powerDomain);
    pClockStates = s_DomainTable[powerDomain].pClockStates;
    pDomainState = s_DomainTable[powerDomain].pDomainState;
        
    if (pDomainState != NULL)
        {
        if (ffMask & DOMAIN_UPDATE_WKUPDEP)
            {
            pm_wkdep = INREG32(&pPrmRegs->PM_WKDEP_xxx) & ~WKDEP_MASK;
            pm_wkdep |= pDomainState->wakeDependency << WKDEP_SHIFT;
            OUTREG32(&pPrmRegs->PM_WKDEP_xxx, pm_wkdep);
            }

        if (ffMask & DOMAIN_UPDATE_SLEEPDEP)
            {
            cm_sleepdep = INREG32(&pCmRegs->CM_SLEEPDEP_xxx) & ~SLEEPDEP_MASK;
            cm_sleepdep |= pDomainState->sleepDependency << SLEEPDEP_SHIFT;
            OUTREG32(&pCmRegs->CM_SLEEPDEP_xxx, cm_sleepdep);
            }

        if (ffMask & DOMAIN_UPDATE_POWERSTATE)
            {
            pm_pwstctrl = INREG32(&pPrmRegs->PM_PWSTCTRL_xxx) & ~(POWERSTATE_MASK | LOGICRETSTATE_MASK);
            pm_pwstctrl |= pDomainState->powerState << POWERSTATE_SHIFT;
            pm_pwstctrl |= pDomainState->logicState;
            OUTREG32(&pPrmRegs->PM_PWSTCTRL_xxx, pm_pwstctrl);
            }
        }

    if (pClockStates != NULL)
        {
        if (ffMask & DOMAIN_UPDATE_CLOCKSTATE)
            {
            UINT i;
            cm_clkstctrl = INREG32(&pCmRegs->CM_CLKSTCTRL_xxx);
            for (i = 0; i < pClockStates->count; ++i)
                {
                cm_clkstctrl &= ~(CLKSTCTRL_MASK << pClockStates->rgClockDomains[i].clockShift);
                cm_clkstctrl |= (pClockStates->rgClockDomains[i].clockState << pClockStates->rgClockDomains[i].clockShift) << CLKSTCTRL_SHIFT;
                }
            OUTREG32(&pCmRegs->CM_CLKSTCTRL_xxx, cm_clkstctrl);

            // save context
            if (powerDomain == POWERDOMAIN_MPU)
                {
                OUTREG32(&g_pPrcmRestore->CM_CLKSTCTRL_MPU, cm_clkstctrl);
                }
            else if (powerDomain == POWERDOMAIN_CORE)
                {
                OUTREG32(&g_pPrcmRestore->CM_CLKSTCTRL_CORE, cm_clkstctrl);
                }
            }
        }
    
    return rc;
}

//-----------------------------------------------------------------------------
BOOL
PrcmDomainSetPowerStateInternal(
    UINT        powerDomain,
    UINT        powerState,
    UINT        logicState,
    BOOL        bNotify
    )
{
    BOOL rc = FALSE;
    PowerDomainState_t *pDomainState;
    
    if (powerDomain >= POWERDOMAIN_COUNT) goto cleanUp;
    if ((s_DomainTable[powerDomain].ffValidationMask & DOMAIN_UPDATE_POWERSTATE) == 0) goto cleanUp;

    // update internal state information 
    pDomainState = s_DomainTable[powerDomain].pDomainState;
    if (pDomainState == NULL) goto cleanUp;
    
    // POWERSTATE_OFF                      0
    // POWERSTATE_RETENTION                1
    // POWERSTATE_INACTIVE                 2
    // POWERSTATE_ON                       3

    // LOGICRETSTATE_LOGICOFF_DOMAINRET    1
    // LOGICRETSTATE_LOGICRET_DOMAINRET    4

    //OALMSG(1, (L"Domain %d -> P%d L%d\r\n", powerDomain, powerState, logicState));

    Lock(Mutex_Domain);
    powerState &= POWERSTATE_MASK;
    powerState >>= POWERSTATE_SHIFT;
    pDomainState->powerState = powerState;
    pDomainState->logicState = logicState & LOGICRETSTATE;
    rc = _PrcmDomainHwUpdate(powerDomain, DOMAIN_UPDATE_POWERSTATE);

    // check if we need to notify of a power state change
    if (bNotify == TRUE)
        {
        // update latency information
        OALWakeupLatency_UpdateDomainState(powerDomain, 
            pDomainState->powerState, 
            pDomainState->logicState
            );
        }
    Unlock(Mutex_Domain);
    
cleanUp:    
    
    return rc;
}

//-----------------------------------------------------------------------------
BOOL
DomainGetDeviceContextState(
    UINT                powerDomain,
    ClockOffsetInfo_2  *pInfo,
    BOOL                bSet
    )
{
    int idx;
    BOOL rc;
    OMAP_PRM_REGS *pPrmRegs;
    
    // Get array index 
    idx = pInfo->offset - cm_offset(CM_ICLKEN1_xxx);
    Lock(Mutex_Domain);
    
    // get current power state of domain
    pPrmRegs = GetPrmRegisterSet(powerDomain);
    if ((INREG32(&pPrmRegs->PM_PWSTST_xxx) & POWERSTATE_MASK) == POWERSTATE_OFF)
        {
        // domain is off
        
        // clear device context table for power domain
        s_DomainTable[powerDomain].rgDeviceContextState[0] = 0;
        s_DomainTable[powerDomain].rgDeviceContextState[1] = 0;
        s_DomainTable[powerDomain].rgDeviceContextState[2] = 0;
        rc = 0;
        }
    else
        {
        // get device context state
        rc = s_DomainTable[powerDomain].rgDeviceContextState[idx] & pInfo->mask;
        }

    // update device context state
    if (bSet == TRUE)
        {
        s_DomainTable[powerDomain].rgDeviceContextState[idx] |= pInfo->mask;        
        }
    Unlock(Mutex_Domain);
    
    return rc != 0;
}

//-----------------------------------------------------------------------------
BOOL
DomainInitialize()
{
    UINT            i;
    BOOL            rc = TRUE;
    UINT            cm_clkstctrl;
    UINT            pm_pwstctrl;
    UINT            pm_wkdep;
    UINT            cm_sleepdep;
    OMAP_CM_REGS   *pCmRegs;
    OMAP_PRM_REGS  *pPrmRegs;
    
    
    OALMSG(OAL_FUNC, (L"+DomainInitialize()\r\n"));

    for (i = 0; i < POWERDOMAIN_COUNT; ++i)
        {        
        pCmRegs = GetCmRegisterSet(i);
        pPrmRegs = GetPrmRegisterSet(i);

        pm_wkdep = 0;
        if (s_DomainTable[i].ffValidationMask & DOMAIN_UPDATE_WKUPDEP)
            {
            pm_wkdep = INREG32(&pPrmRegs->PM_WKDEP_xxx);
            }

        cm_sleepdep = 0;
        if (s_DomainTable[i].ffValidationMask & DOMAIN_UPDATE_SLEEPDEP)
            {
            cm_sleepdep = INREG32(&pCmRegs->CM_SLEEPDEP_xxx);
            }

        pm_pwstctrl = 0;
        cm_clkstctrl = 0;
        if (s_DomainTable[i].ffValidationMask & DOMAIN_UPDATE_POWERSTATE)
            {
            pm_pwstctrl = INREG32(&pPrmRegs->PM_PWSTCTRL_xxx);
            }

        if (s_DomainTable[i].ffValidationMask & DOMAIN_UPDATE_CLOCKSTATE)
            {
            cm_clkstctrl = INREG32(&pCmRegs->CM_CLKSTCTRL_xxx);
            }

        _DomainInitialize(s_DomainTable[i].pDomainState, 
            pm_pwstctrl, pm_wkdep, cm_sleepdep
            );

        _PrcmDomainClockInitialize(s_DomainTable[i].pClockStates, 
            cm_clkstctrl
            );
        }


    OALMSG(OAL_FUNC, (L"-DomainInitialize()=%d\r\n", rc));
    return rc;    
}

//-----------------------------------------------------------------------------
// WARNING: This function is called from OEMPowerOff - no system calls, critical 
// sections, OALMSG, etc., may be used by this function or any function that it calls.
//------------------------------------------------------------------------------
BOOL
PrcmRestoreDomain(
    UINT powerDomain
    )
{
    UINT i;
    BOOL rc = TRUE;
    UINT cm_sleepdep;
    UINT cm_clkstctrl;   
    OMAP_CM_REGS   *pCmRegs;
    OMAP_PRM_REGS  *pPrmRegs;
    ClockDomainInfo_t  *pClockStates;    
    PowerDomainState_t *pDomainState;
    
    // initialize variables
    pCmRegs = GetCmRegisterSet(powerDomain);
    pPrmRegs = GetPrmRegisterSet(powerDomain);
    pClockStates = s_DomainTable[powerDomain].pClockStates;
    pDomainState = s_DomainTable[powerDomain].pDomainState;
        
    // restore clk src dividers
    _DomainClockRestore(powerDomain);

    if (s_DomainTable[powerDomain].ffValidationMask & DOMAIN_UPDATE_SLEEPDEP)
        {
        // restore sleep dependencies
        cm_sleepdep = INREG32(&pCmRegs->CM_SLEEPDEP_xxx) & ~SLEEPDEP_MASK;
        cm_sleepdep |= pDomainState->sleepDependency << SLEEPDEP_SHIFT;
        OUTREG32(&pCmRegs->CM_SLEEPDEP_xxx, cm_sleepdep);
        }

    // restore clock state
    cm_clkstctrl = 0;
    for (i = 0; i < pClockStates->count; ++i)
        {
        cm_clkstctrl |= (pClockStates->rgClockDomains[i].clockState << pClockStates->rgClockDomains[i].clockShift) << CLKSTCTRL_SHIFT;
        }
    OUTREG32(&pCmRegs->CM_CLKSTCTRL_xxx, cm_clkstctrl);

    return rc;
}

//-----------------------------------------------------------------------------
BOOL
PrcmDomainSetWakeupDependency(
    UINT        powerDomain,
    UINT        ffDependency,
    BOOL        bEnable
    )
{
    BOOL rc = FALSE;
    PowerDomainState_t *pDomainState;
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+PrcmDomainSetWakeupDependency"
            L"(powerDomain=%d, ffDependency=0x%08X, bEnable=%d)\r\n", 
            powerDomain, ffDependency, bEnable));
    
    if (powerDomain >= POWERDOMAIN_COUNT) goto cleanUp;
    if ((s_DomainTable[powerDomain].ffValidationMask & DOMAIN_UPDATE_WKUPDEP) == 0) goto cleanUp;

    // update internal state information 
    pDomainState = s_DomainTable[powerDomain].pDomainState;
    if (pDomainState == NULL) goto cleanUp;

    Lock(Mutex_Domain);
    ffDependency &= WKDEP_MASK;
    ffDependency >>= WKDEP_SHIFT;
    if (bEnable != FALSE)
        {
        pDomainState->wakeDependency |= ffDependency; 
        }
    else
        {
        pDomainState->wakeDependency &= ~ffDependency; 
        }

    rc = _PrcmDomainHwUpdate(powerDomain, DOMAIN_UPDATE_WKUPDEP);
    Unlock(Mutex_Domain);
    
cleanUp:        
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-PrcmDomainSetWakeupDependency()=%d\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
BOOL
PrcmDomainSetSleepDependency(
    UINT        powerDomain,
    UINT        ffDependency,
    BOOL        bEnable
    )
{
    BOOL rc = FALSE;
    PowerDomainState_t *pDomainState;
    OALMSG(OAL_FUNC, (L"+PrcmDomainSetSleepDependency"
        L"(powerDomain=%d, ffDependency=0x%08X, bEnable=%d)\r\n", 
        powerDomain, ffDependency, bEnable));
    
    if (powerDomain >= POWERDOMAIN_COUNT) goto cleanUp;
    if ((s_DomainTable[powerDomain].ffValidationMask & DOMAIN_UPDATE_SLEEPDEP) == 0) goto cleanUp;

    // update internal state information 
    pDomainState = s_DomainTable[powerDomain].pDomainState;
    if (pDomainState == NULL) goto cleanUp;
    
    Lock(Mutex_Domain);
    ffDependency &= SLEEPDEP_MASK;
    ffDependency >>= SLEEPDEP_SHIFT;
    if (bEnable != FALSE)
        {
        pDomainState->sleepDependency |= ffDependency; 
        }
    else
        {
        pDomainState->sleepDependency &= ~ffDependency; 
        }

    rc = _PrcmDomainHwUpdate(powerDomain, DOMAIN_UPDATE_SLEEPDEP);
    Unlock(Mutex_Domain);
    
cleanUp:        
    OALMSG(OAL_FUNC, (L"-PrcmDomainSetSleepDependency()=%d\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
BOOL
PrcmDomainSetPowerState(
    UINT        powerDomain,
    UINT        powerState,
    UINT        logicState
    )
{
    BOOL rc;
    UINT temp;
    UINT clockState = CLKSTCTRL_DISABLED;
    UINT oldPowerState = (UINT)-1;
    OMAP_CM_REGS *pCmRegs;
    
    OALMSG(OAL_FUNC, (L"+PrcmDomainSetPowerState"
        L"(powerDomain=%d, powerState=0x%08X)\r\n", 
        powerDomain, powerState));
    
    // get old power states to check if power domain needs a sw wakeup
    if (s_DomainTable[powerDomain].pClockStates != NULL)
        {
        clockState = s_DomainTable[powerDomain].pClockStates->rgClockDomains[0].clockState;
        }

    if (s_DomainTable[powerDomain].pDomainState != NULL)
        {
        oldPowerState = s_DomainTable[powerDomain].pDomainState->powerState;
        }
    
    rc = PrcmDomainSetPowerStateInternal(powerDomain, powerState, logicState, TRUE);
    
    // force a sleep to sleep transition through software since it
    // isn't supported in hardware
    if (oldPowerState != -1 && clockState == CLKSTCTRL_AUTOMATIC &&        
        s_DomainTable[powerDomain].refCount == 0 && oldPowerState != powerState)
        {        
        pCmRegs = GetCmRegisterSet(powerDomain);
    
        // force sw wake-up of power domain
        temp = INREG32(&pCmRegs->CM_CLKSTCTRL_xxx);
        temp &= ~CLKSTCTRL_MASK;
        temp |= CLKSTCTRL_WAKEUP;
        OUTREG32(&pCmRegs->CM_CLKSTCTRL_xxx, temp);

        // wait for clock to be activated
        temp = OALGetTickCount() + 1;
        while (INREG32(&pCmRegs->CM_CLKSTST_xxx) == 0 && temp > OALGetTickCount());

        // put clock back to automatic mode
        temp = INREG32(&pCmRegs->CM_CLKSTCTRL_xxx);
        temp &= ~CLKSTCTRL_MASK;
        temp |= CLKSTCTRL_AUTOMATIC;
        OUTREG32(&pCmRegs->CM_CLKSTCTRL_xxx, temp);
        }
    
    OALMSG(OAL_FUNC, (L"+PrcmDomainSetPowerState()=%d\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
BOOL
PrcmDomainSetClockState(
    UINT        powerDomain,
    UINT        clockDomain,
    UINT        clockState
    )
{
    UINT i;
    BOOL rc = FALSE;
    ClockDomainInfo_t *pClockStates;
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+PrcmDomainSetClockState"
            L"(powerDomain=%d, clockDomain=%d, clockState=0x%08X)\r\n", 
            powerDomain, clockDomain, clockState));

    if (powerDomain >= POWERDOMAIN_COUNT) goto cleanUp;
    if ((s_DomainTable[powerDomain].ffValidationMask & DOMAIN_UPDATE_CLOCKSTATE) == 0) goto cleanUp;

    // update internal state information 
    pClockStates = s_DomainTable[powerDomain].pClockStates;
    if (pClockStates == NULL) goto cleanUp;

    Lock(Mutex_Domain);
    for (i = 0; i < pClockStates->count; ++i)
        {
        if (pClockStates->rgClockDomains[i].clockDomain == (ClockDomain_e) clockDomain)
            {
            clockState &= CLKSTCTRL_MASK;
            clockState >>= CLKSTCTRL_SHIFT;
            pClockStates->rgClockDomains[i].clockState = clockState;
            _PrcmDomainHwUpdate(powerDomain, DOMAIN_UPDATE_CLOCKSTATE);
            break;
            }
        }    
    Unlock(Mutex_Domain);
    rc = TRUE;
    
cleanUp:        
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-PrcmDomainSetClockState()=%d\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
// WARNING: This function is called from OEMPowerOff - no system calls, critical 
// sections, OALMSG, etc., may be used by this function or any function that it calls.
//------------------------------------------------------------------------------
BOOL
PrcmDomainSetClockStateKernel(
    UINT        powerDomain,
    UINT        clockDomain,
    UINT        clockState
    )
{
    UINT i;
    BOOL rc = FALSE;
    ClockDomainInfo_t *pClockStates;

    if (powerDomain >= POWERDOMAIN_COUNT) goto cleanUp;
    if ((s_DomainTable[powerDomain].ffValidationMask & DOMAIN_UPDATE_CLOCKSTATE) == 0) goto cleanUp;

    // update internal state information 
    pClockStates = s_DomainTable[powerDomain].pClockStates;
    if (pClockStates == NULL) goto cleanUp;

    for (i = 0; i < pClockStates->count; ++i)
        {
        if (pClockStates->rgClockDomains[i].clockDomain == (ClockDomain_e) clockDomain)
            {
            clockState &= CLKSTCTRL_MASK;
            clockState >>= CLKSTCTRL_SHIFT;
            pClockStates->rgClockDomains[i].clockState = clockState;
            _PrcmDomainHwUpdate(powerDomain, DOMAIN_UPDATE_CLOCKSTATE);
            break;
            }
        }    
    rc = TRUE;
    
cleanUp:        
    return rc;
}

//-----------------------------------------------------------------------------
BOOL
PrcmDomainSetMemoryState(
    UINT        powerDomain,
    UINT        memoryState,
    UINT        memoryStateMask
    )
{
    UINT val;    
    BOOL rc = FALSE;
    OMAP_PRM_REGS *pPrmRegs;
    OALMSG(OAL_FUNC, (L"+PrcmDomainSetMemoryState"
        L"(powerDomain=%d, memoryState=0x%08X, memoryStateMask=0x%08X)\r\n", 
        powerDomain, memoryState, memoryStateMask));

    pPrmRegs = GetPrmRegisterSet(powerDomain);
    if (pPrmRegs == NULL) goto cleanUp;

    Lock(Mutex_Domain);
    // update cached logic state
    if (memoryStateMask & LOGICRETSTATE)
        {
        s_DomainTable[powerDomain].pDomainState->logicState = memoryState & LOGICRETSTATE_MASK;
        }
    
    memoryState &= memoryStateMask;
    val = INREG32(&pPrmRegs->PM_PWSTCTRL_xxx) & ~memoryStateMask;
    val |= memoryState;
    OUTREG32(&pPrmRegs->PM_PWSTCTRL_xxx, val);
    Unlock(Mutex_Domain);
    
    rc = TRUE;
    
cleanUp:        
    OALMSG(OAL_FUNC, (L"-PrcmDomainSetMemoryState()=%d\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
// WARNING: The PrcmDomainClearWakeupStatus function can be called from OEMIdle
// and OEMPowerOff (through PrcmSuspend). It must not use system calls, critical 
// sections, etc.
//-----------------------------------------------------------------------------

BOOL
PrcmDomainClearWakeupStatus(
    UINT        powerDomain
    )
{
    UINT val;    
    BOOL rc = FALSE;
    OMAP_PRM_REGS *pPrmRegs;
    //OALMSG(OAL_FUNC, (L"+PrcmDomainClearWakeupStatus"
    //    L"(powerDomain=%d)\r\n", 
    //    powerDomain)
    //    );

    pPrmRegs = GetPrmRegisterSet(powerDomain);
    if (pPrmRegs == NULL) goto cleanUp;

    // This routine should only be called during system boot-up or
    // from OEMIdle, OEMPowerOff.  Hence, serialization within this routine
    // should not be performed.

    val = INREG32(&pPrmRegs->PM_WKST_xxx);
    OUTREG32(&pPrmRegs->PM_WKST_xxx, val);
    if (powerDomain == POWERDOMAIN_CORE) 
        {
        val = INREG32(&pPrmRegs->PM_WKST3_xxx);
        OUTREG32(&pPrmRegs->PM_WKST3_xxx, val);
        }
    
    rc = TRUE;
    
cleanUp:        
    //OALMSG(OAL_FUNC, (L"-PrcmDomainClearWakeupStatus()=%d\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
void
PrcmDomainUpdateRefCount(
    UINT                    powerDomain,
    UINT                    bEnable
    )
{    
    // update refcount
    if (bEnable)
        {
        InterlockedIncrement((LONG*)&s_DomainTable[powerDomain].refCount);
        }
    else
        {
        InterlockedDecrement((LONG*)&s_DomainTable[powerDomain].refCount);
        }
}

//-----------------------------------------------------------------------------
void
PrcmProcessPostMpuWakeup()
{
    // NOTE:
    //  This routine should only be called in OEMIdle where IRQ's are disabled
    //
    DWORD val;

    // core
    OUTREG32(&g_pPrcmPrm->pOMAP_CORE_PRM->PM_WKST1_CORE,
        INREG32(&g_pPrcmPrm->pOMAP_CORE_PRM->PM_WKST1_CORE)
        );

    OUTREG32(&g_pPrcmPrm->pOMAP_CORE_PRM->PM_WKST3_CORE,
        INREG32(&g_pPrcmPrm->pOMAP_CORE_PRM->PM_WKST3_CORE)
        );

    // peripheral
    OUTREG32(&g_pPrcmPrm->pOMAP_PER_PRM->PM_WKST_PER,
        INREG32(&g_pPrcmPrm->pOMAP_PER_PRM->PM_WKST_PER)
        );

    // usbhost
    OUTREG32(&g_pPrcmPrm->pOMAP_USBHOST_PRM->PM_WKST_USBHOST,
        INREG32(&g_pPrcmPrm->pOMAP_USBHOST_PRM->PM_WKST_USBHOST)
        );

    // clear irq status for mpu
    val = INREG32(&g_pPrcmPrm->pOMAP_OCP_SYSTEM_PRM->PRM_IRQSTATUS_MPU);
    OUTREG32(&g_pPrcmPrm->pOMAP_OCP_SYSTEM_PRM->PRM_IRQSTATUS_MPU, 
        val & PRM_IRQENABLE_WKUP_EN
        );

    // clear device context state table

    // core
    val = INREG32(&g_pPrcmPrm->pOMAP_CORE_PRM->PM_PREPWSTST_CORE);
    if (((val & POWERSTATE_MASK) == POWERSTATE_OFF) || \
        (((val & POWERSTATE_MASK) == POWERSTATE_RETENTION) && \
         ((val & LOGICRETSTATE_MASK) == LOGICRETSTATE_LOGICOFF_DOMAINRET)))
        {
        // clear domain device context state
        s_DomainTable[POWERDOMAIN_CORE].rgDeviceContextState[0] = 0;
        s_DomainTable[POWERDOMAIN_CORE].rgDeviceContextState[1] = 0;
        s_DomainTable[POWERDOMAIN_CORE].rgDeviceContextState[2] = 0;
        }

    // peripheral
    val = INREG32(&g_pPrcmPrm->pOMAP_PER_PRM->PM_PREPWSTST_PER);
    if ((val & POWERSTATE_MASK) == POWERSTATE_OFF)
        {
        // clear domain device context state
        s_DomainTable[POWERDOMAIN_PERIPHERAL].rgDeviceContextState[0] = 0;
        s_DomainTable[POWERDOMAIN_PERIPHERAL].rgDeviceContextState[1] = 0;
        s_DomainTable[POWERDOMAIN_PERIPHERAL].rgDeviceContextState[2] = 0;
        }

    // dss
    val = INREG32(&g_pPrcmPrm->pOMAP_DSS_PRM->PM_PREPWSTST_DSS);
    if ((val & POWERSTATE_MASK) == POWERSTATE_OFF)
        {
        // clear domain device context state
        s_DomainTable[POWERDOMAIN_DSS].rgDeviceContextState[0] = 0;
        s_DomainTable[POWERDOMAIN_DSS].rgDeviceContextState[1] = 0;
        s_DomainTable[POWERDOMAIN_DSS].rgDeviceContextState[2] = 0;
        }

    // usbhost
    val = INREG32(&g_pPrcmPrm->pOMAP_USBHOST_PRM->PM_PREPWSTST_USBHOST);
    if ((val & POWERSTATE_MASK) == POWERSTATE_OFF)
        {
        // clear domain device context state
        s_DomainTable[POWERDOMAIN_USBHOST].rgDeviceContextState[0] = 0;
        s_DomainTable[POWERDOMAIN_USBHOST].rgDeviceContextState[1] = 0;
        s_DomainTable[POWERDOMAIN_USBHOST].rgDeviceContextState[2] = 0;
        }

    // camera
    val = INREG32(&g_pPrcmPrm->pOMAP_CAM_PRM->PM_PREPWSTST_CAM);
    if ((val & POWERSTATE_MASK) == POWERSTATE_OFF)
        {
        // clear domain device context state
        s_DomainTable[POWERDOMAIN_CAMERA].rgDeviceContextState[0] = 0;
        s_DomainTable[POWERDOMAIN_CAMERA].rgDeviceContextState[1] = 0;
        s_DomainTable[POWERDOMAIN_CAMERA].rgDeviceContextState[2] = 0;
        }

    // sgx
    val = INREG32(&g_pPrcmPrm->pOMAP_SGX_PRM->PM_PREPWSTST_SGX);
    if ((val & POWERSTATE_MASK) == POWERSTATE_OFF)
        {
        // clear domain device context state
        s_DomainTable[POWERDOMAIN_SGX].rgDeviceContextState[0] = 0;
        s_DomainTable[POWERDOMAIN_SGX].rgDeviceContextState[1] = 0;
        s_DomainTable[POWERDOMAIN_SGX].rgDeviceContextState[2] = 0;
        }

    // IVA2
    val = INREG32(&g_pPrcmPrm->pOMAP_IVA2_PRM->PM_PREPWSTST_IVA2);
    if ((val & POWERSTATE_MASK) == POWERSTATE_OFF)
        {
        // clear domain device context state
        s_DomainTable[POWERDOMAIN_IVA2].rgDeviceContextState[0] = 0;
        s_DomainTable[POWERDOMAIN_IVA2].rgDeviceContextState[1] = 0;
        s_DomainTable[POWERDOMAIN_IVA2].rgDeviceContextState[2] = 0;
        }

}

//-----------------------------------------------------------------------------
VOID
PrcmDomainClearReset()
{
    // Clear the Resest states
    OUTREG32(&g_pPrcmPrm->pOMAP_CORE_PRM->RM_RSTST_CORE, 
            INREG32(&g_pPrcmPrm->pOMAP_CORE_PRM->RM_RSTST_CORE)); 

    OUTREG32(&g_pPrcmPrm->pOMAP_MPU_PRM->RM_RSTST_MPU, 
            INREG32(&g_pPrcmPrm->pOMAP_MPU_PRM->RM_RSTST_MPU)); 

    OUTREG32(&g_pPrcmPrm->pOMAP_EMU_PRM->RM_RSTST_EMU, 
            INREG32(&g_pPrcmPrm->pOMAP_EMU_PRM->RM_RSTST_EMU)); 

    OUTREG32(&g_pPrcmPrm->pOMAP_PER_PRM->RM_RSTST_PER,
            INREG32(&g_pPrcmPrm->pOMAP_PER_PRM->RM_RSTST_PER)); 

    OUTREG32(&g_pPrcmPrm->pOMAP_IVA2_PRM->RM_RSTST_IVA2,
            INREG32(&g_pPrcmPrm->pOMAP_IVA2_PRM->RM_RSTST_IVA2)); 

    OUTREG32(&g_pPrcmPrm->pOMAP_SGX_PRM->RM_RSTST_SGX,
            INREG32(&g_pPrcmPrm->pOMAP_SGX_PRM->RM_RSTST_SGX)); 

    OUTREG32(&g_pPrcmPrm->pOMAP_DSS_PRM->RM_RSTST_DSS,
            INREG32(&g_pPrcmPrm->pOMAP_DSS_PRM->RM_RSTST_DSS)); 

    OUTREG32(&g_pPrcmPrm->pOMAP_CAM_PRM->RM_RSTST_CAM,
            INREG32(&g_pPrcmPrm->pOMAP_CAM_PRM->RM_RSTST_CAM)); 

    OUTREG32(&g_pPrcmPrm->pOMAP_NEON_PRM->RM_RSTST_NEON,
            INREG32(&g_pPrcmPrm->pOMAP_NEON_PRM->RM_RSTST_NEON)); 

    OUTREG32(&g_pPrcmPrm->pOMAP_USBHOST_PRM->RM_RSTST_USBHOST,
            INREG32(&g_pPrcmPrm->pOMAP_USBHOST_PRM->RM_RSTST_USBHOST)); 
}
//------------------------------------------------------------------------------
