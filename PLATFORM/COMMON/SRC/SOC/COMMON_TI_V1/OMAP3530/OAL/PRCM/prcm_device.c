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
//  File: prcm_device.c
//
#include "omap.h"
#include "omap_prof.h"
#include "omap3530.h"
#include <nkintr.h>
#include <pkfuncs.h>
#include "oalex.h"
#include "prcm_priv.h"
#include "prcm_device.h"

#define SPECIAL_DEBUG_MESSAGE(s, d)       {}
#define SPECIAL_DEBUG_MESSAGE2(s, d, b)   {}
#define SPECIAL_DEBUG_MESSAGE_DONE(s, d)  {}
//#define SPECIAL_DEBUG_MESSAGE(s, d)       {if (d >= OMAP_DEVICE_HSUSB1 && d <= OMAP_DEVICE_USBHOST3) {OALMSG(1, (L"PRCM %s devId=0x%x\r\n", TEXT(#s), d));}}
//#define SPECIAL_DEBUG_MESSAGE2(s, d, b)       {if (d >= OMAP_DEVICE_HSUSB1 && d <= OMAP_DEVICE_USBHOST3) {OALMSG(1, (L"PRCM %s devId=0x%x enable=%d\r\n", TEXT(#s), d, b));}}
//#define SPECIAL_DEBUG_MESSAGE_DONE(s, d)    {if (d >= OMAP_DEVICE_HSUSB1 && d <= OMAP_DEVICE_USBHOST3) {DumpPrcmRegs();}}

extern void DumpPrcmRegs();

//------------------------------------------------------------------------------
//
//  Local:  s_rgClockManagementRoutines
//
//  table of function pointers to clock management routines
//
typedef BOOL fnPrcmEnableDeviceClock(OMAP_DEVICE devId, BOOL bEnable);
typedef BOOL fnPrcmEnableDeviceAutoIdle(OMAP_DEVICE devId, BOOL bEnable);
typedef BOOL fnPrcmSetSourceDeviceClock(OMAP_DEVICE devId, UINT count, UINT rgClocks[]);
typedef struct
{
    fnPrcmEnableDeviceClock*     PrcmDeviceEnableIClock;
    fnPrcmEnableDeviceClock*     PrcmDeviceEnableFClock;    
    fnPrcmEnableDeviceAutoIdle*  PrcmDeviceEnableAutoIdle;
    fnPrcmSetSourceDeviceClock*  PrcmSetSourceDeviceClock;       
} ClockManagementRoutines;

BOOL _PrcmDeviceEnableIClock(OMAP_DEVICE devId, BOOL bEnable);
BOOL _PrcmDeviceEnableFClock(OMAP_DEVICE devId, BOOL bEnable);
BOOL _PrcmDeviceEnableAutoIdle(OMAP_DEVICE devId, BOOL bEnable);
BOOL _PrcmDeviceSetSourceClock(OMAP_DEVICE devId, UINT count, UINT rgClocks[]);

BOOL _PrcmDeviceEnableIClock_DSS(OMAP_DEVICE devId, BOOL bEnable);
BOOL _PrcmDeviceEnableFClock_DSS(OMAP_DEVICE devId, BOOL bEnable);
BOOL _PrcmDeviceSetSourceClock_DSS(OMAP_DEVICE devId, UINT count, UINT rgClocks[]);

BOOL _PrcmDeviceEnableIClock_USB(OMAP_DEVICE devId, BOOL bEnable);
BOOL _PrcmDeviceEnableFClock_USB(OMAP_DEVICE devId, BOOL bEnable);
BOOL _PrcmDeviceSetSourceClock_USB(OMAP_DEVICE devId, UINT count, UINT rgClocks[]);

// WARNING: table rows must be same order as POWERDOMAIN_... enum
static ClockManagementRoutines s_rgClockManagementRoutines[] =
{
    {_PrcmDeviceEnableIClock,   _PrcmDeviceEnableFClock,    _PrcmDeviceEnableAutoIdle,  _PrcmDeviceSetSourceClock   },   // POWERDOMAIN_WAKEUP
    {_PrcmDeviceEnableIClock,   _PrcmDeviceEnableFClock,    _PrcmDeviceEnableAutoIdle,  _PrcmDeviceSetSourceClock   },   // POWERDOMAIN_CORE
    {_PrcmDeviceEnableIClock,   _PrcmDeviceEnableFClock,    _PrcmDeviceEnableAutoIdle,  _PrcmDeviceSetSourceClock   },   // POWERDOMAIN_PERIPHERAL
    {_PrcmDeviceEnableIClock_USB,_PrcmDeviceEnableFClock_USB,_PrcmDeviceEnableAutoIdle,  _PrcmDeviceSetSourceClock_USB}, // POWERDOMAIN_USBHOST
    {NULL /*EMULATION*/,        NULL,                       NULL,                       NULL                        },   // POWERDOMAIN_EMULATION
    {NULL /*MPU*/,              NULL,                       NULL,                       NULL                        },   // POWERDOMAIN_MPU
    {_PrcmDeviceEnableIClock_DSS,_PrcmDeviceEnableFClock_DSS,_PrcmDeviceEnableAutoIdle,  _PrcmDeviceSetSourceClock_DSS}, // POWERDOMAIN_DSS
    {NULL /*NEON*/,             NULL,                       NULL,                       NULL                        },   // POWERDOMAIN_NEON
    {NULL,                      _PrcmDeviceEnableFClock,    NULL,                       NULL                        },   // POWERDOMAIN_IVA2
    {_PrcmDeviceEnableIClock,   _PrcmDeviceEnableFClock ,   _PrcmDeviceEnableAutoIdle,  _PrcmDeviceSetSourceClock   },   // POWERDOMAIN_CAMERA
    {_PrcmDeviceEnableIClock,   _PrcmDeviceEnableFClock ,   NULL,                       _PrcmDeviceSetSourceClock   },   // POWERDOMAIN_SGX
    {NULL /*EFUSE*/,            NULL,                       NULL,                       NULL                        },   // POWERDOMAIN_EFUSE
    {NULL /*SMARTREFLEX*/,      NULL,                       NULL,                       NULL                        },   // POWERDOMAIN_SMARTREFLEX
};

//------------------------------------------------------------------------------
//
//  Function:  _PrcmDeviceAddrefDeviceDomain
//
//  incremented domain device refcount
//
static 
void
_PrcmDeviceAddrefDeviceDomain(UINT powerDomain)
{
    // UNDONE:
    //  Potential deadlock
    //
    Lock(Mutex_DeviceClock);
    s_rgActiveDomainDeviceCount[powerDomain]++;
    if (s_rgActiveDomainDeviceCount[powerDomain] == 1)
        {
        if ((s_rgClockDomainDependency[powerDomain].requiredSleepDependency & WKDEP_EN_MPU) == 0)
            {
            // if sleep dependency is dynamic then associate sleep dependency
            PrcmDomainSetSleepDependency(powerDomain, WKDEP_EN_MPU, TRUE);
            }

        // check if domain dependency is fixed or not
        if ((s_rgClockDomainDependency[powerDomain].requiredWakeupDependency & WKDEP_EN_MPU) == 0)
            {
            // if wake-up dependency is dynamic then associate wake-up dependency
            PrcmDomainSetWakeupDependency(powerDomain, WKDEP_EN_MPU, TRUE);

            // generate a software supervised wake-up to enable clock domain
            PrcmDomainSetClockState(powerDomain,
                            s_rgClockDomainDependency[powerDomain].clockDomain,
                            CLKSTCTRL_WAKEUP
                            );

            PrcmDomainSetClockState(powerDomain,
                            s_rgClockDomainDependency[powerDomain].clockDomain,
                            CLKSTCTRL_AUTOMATIC
                            );
            }
        }
    Unlock(Mutex_DeviceClock);    
}

//------------------------------------------------------------------------------
//
//  Function:  _PrcmDeviceReleaseDeviceDomain
//
//  decrement domain device refcount
//
static 
void
_PrcmDeviceReleaseDeviceDomain(UINT powerDomain)
{
    // UNDONE:
    //  Potential deadlock
    //
    Lock(Mutex_DeviceClock);
    if (s_rgActiveDomainDeviceCount[powerDomain] != 0)
        {
        s_rgActiveDomainDeviceCount[powerDomain]--;
        if (s_rgActiveDomainDeviceCount[powerDomain] == 0)
            {
            if ((s_rgClockDomainDependency[powerDomain].requiredWakeupDependency & WKDEP_EN_MPU) == 0)
                {
                // if wake-up dependency is dynamic then dis-associate wake-up dependency
                PrcmDomainSetWakeupDependency(powerDomain, WKDEP_EN_MPU, FALSE);
                }

            if ((s_rgClockDomainDependency[powerDomain].requiredSleepDependency & WKDEP_EN_MPU) == 0)
                {
                // if sleep dependency is dynamic then dis-associate sleep dependency
                PrcmDomainSetSleepDependency(powerDomain, WKDEP_EN_MPU, FALSE);

                // generate a software supervised sleep to disable clock domain
                PrcmDomainSetClockState(powerDomain,
                                s_rgClockDomainDependency[powerDomain].clockDomain,
                                CLKSTCTRL_SLEEP
                                );

                PrcmDomainSetClockState(powerDomain,
                                s_rgClockDomainDependency[powerDomain].clockDomain,
                                CLKSTCTRL_AUTOMATIC
                                );
                }
            }
        }
    Unlock(Mutex_DeviceClock); 
}

//------------------------------------------------------------------------------
//
//  Function:  _PrcmDeviceSetAutoidle
//
//  sets the autoidle bit for a given device
//
BOOL
_PrcmDeviceHwUpdateAutoidle(
    UINT devId
    )
{
    UINT mask;
    BOOL rc = TRUE;  
    OMAP_CM_REGS *pPrcmCm;
    volatile unsigned int *pautoidle;
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+_PrcmDeviceHwUpdateAutoidle(devId=%d)\r\n", devId));

    SPECIAL_DEBUG_MESSAGE(_PrcmDeviceHwUpdateAutoidle, devId)

    if (s_rgDeviceLookupTable[devId].pautoidle == NULL) goto cleanUp;

    mask = s_rgDeviceLookupTable[devId].pautoidle->mask;
    pPrcmCm = GetCmRegisterSet(s_rgDeviceLookupTable[devId].powerDomain);
    pautoidle = (volatile unsigned int*)((UCHAR*)pPrcmCm + s_rgDeviceLookupTable[devId].pautoidle->offset);

    if (s_rgDeviceLookupTable[devId].pautoidle->enabled == TRUE)
        {
        SETREG32(pautoidle, mask);
        }
    else
        {
        CLRREG32(pautoidle, mask);
        }
    
    SPECIAL_DEBUG_MESSAGE_DONE(_PrcmDeviceHwUpdateAutoidle, devId)
    rc = TRUE;

cleanUp:
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-_PrcmDeviceHwUpdateAutoidle()=%d\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  _PrcmDeviceEnableAutoIdle
//
//  Enables AUTOIDLE for the appropriate clock
//
BOOL
_PrcmDeviceEnableAutoIdle(
    OMAP_DEVICE devId, 
    BOOL bEnable
    )
{
    BOOL rc = TRUE;    
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+_PrcmDeviceEnableAutoIdle(devId=%d, %d)\r\n", devId, bEnable));

    SPECIAL_DEBUG_MESSAGE2(_PrcmDeviceEnableAutoIdle, devId, bEnable)

    // update flag
    if (s_rgDeviceLookupTable[devId].pautoidle)
        {
        s_rgDeviceLookupTable[devId].pautoidle->enabled = bEnable;
        _PrcmDeviceHwUpdateAutoidle(devId);
        }
    rc = TRUE;
    
    SPECIAL_DEBUG_MESSAGE_DONE(_PrcmDeviceEnableAutoIdle, devId)

    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-_PrcmDeviceEnableAutoIdle()=%d\r\n", rc));
    return rc;
}
    
//------------------------------------------------------------------------------
//
//  Function:  _PrcmUpdateDeviceClockSource
//
//  updates the device source clocks
//
__inline
void 
_PrcmUpdateDeviceClockSource(
    SourceDeviceClocks_t *pSrcClocks,
    BOOL bEnable
    )
{
    DWORD i;
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+_PrcmUpdateDeviceClockSource"
            L"(pSrcClocks=0x%08X, bEnable=%d)\r\n", pSrcClocks, bEnable)
            );
    
    if (pSrcClocks == NULL) return;

    for (i = 0; i < pSrcClocks->size; ++i)
        {
        ClockUpdateParentClock(pSrcClocks->rgSourceClocks[i], bEnable);
        }

    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+_PrcmUpdateDeviceClockSource()\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  _PrcmDeviceWaitForDeviceAccess
//
//  returns for register to be accessible
//
BOOL
_PrcmDeviceWaitForDeviceAccess(
    OMAP_DEVICE devId
    )
{
    int i;
    UINT mask;
    OMAP_CM_REGS *pPrcmCm;
    volatile unsigned int *pidle;
    DeviceLookupEntry const *pEntry = &s_rgDeviceLookupTable[devId];    
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+_PrcmDeviceWaitForDeviceAccess(devId=%d)\r\n", devId));

    SPECIAL_DEBUG_MESSAGE(_PrcmDeviceWaitForDeviceAccess, devId)

    if (pEntry->pidlestatus != NULL)
        {
        mask = pEntry->pidlestatus->mask;    
        pPrcmCm = GetCmRegisterSet(pEntry->powerDomain);
        pidle = (volatile unsigned int*)((UCHAR*)pPrcmCm + pEntry->pidlestatus->offset);  
        for (i = 0; i < MAX_IDLESTATUS_LOOP; ++i)
            {
            if ((INREG32(pidle) & mask) == 0) break;
            OALStall(1);
            }
        }
    
    SPECIAL_DEBUG_MESSAGE_DONE(_PrcmDeviceWaitForDeviceAccess, devId)

    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-_PrcmDeviceWaitForDeviceAccess()=%d\r\n", TRUE));
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  _PrcmDeviceEnableWakeUp
//
//  Enables the wake enable capabilities of a device
//
BOOL
_PrcmDeviceEnableWakeUp(
    OMAP_DEVICE devId, 
    BOOL bEnable
    )
{
    UINT mask;
    OMAP_PRM_REGS *pPrcmPrm;
    volatile unsigned int *pwken;
    DeviceLookupEntry const *pEntry = &s_rgDeviceLookupTable[devId];
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+_PrcmDeviceEnableWakeUp(devId=%d)\r\n", devId));

    SPECIAL_DEBUG_MESSAGE2(_PrcmDeviceEnableWakeUp, devId, bEnable)

    // check if device is wakeup capable
    if (pEntry->pwken != NULL)
        {
        // initialize variables
        mask = pEntry->pwken->mask;    
        pPrcmPrm = GetPrmRegisterSet(pEntry->powerDomain);
        pwken = (volatile unsigned int*)((UCHAR*)pPrcmPrm + pEntry->pwken->offset);
        
        if (bEnable) 
            {
            SETREG32(pwken, mask);
            }
        else
            {
            CLRREG32(pwken, mask);
            }
        }

    SPECIAL_DEBUG_MESSAGE_DONE(_PrcmDeviceEnableWakeUp, devId)

    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-_PrcmDeviceEnableWakeUp()=%d\r\n", TRUE));
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  _PrcmDeviceEnableFClock
//
//  Enables the appropriate functional clock
//
BOOL
_PrcmDeviceEnableFClock(
    OMAP_DEVICE devId, 
    BOOL bEnable
    )
{
    UINT mask;
    OMAP_CM_REGS *pPrcmCm;
    volatile unsigned int *pfclken;
    DeviceLookupEntry const *pEntry = &s_rgDeviceLookupTable[devId]; 
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+_PrcmDeviceEnableFClock(devId=%d, %d)\r\n", devId, bEnable));

    SPECIAL_DEBUG_MESSAGE2(_PrcmDeviceEnableFClock, devId, bEnable)

    if (pEntry->pfclk != NULL)
        {    
        mask = pEntry->pfclk->mask;    
        pPrcmCm = GetCmRegisterSet(pEntry->powerDomain);
        pfclken = (volatile unsigned int*)((UCHAR*)pPrcmCm + pEntry->pfclk->offset);
        
        if (bEnable != FALSE)
            {
            SETREG32(pfclken, mask);
            }
        else
            {
            CLRREG32(pfclken, mask);
            }

        // notify oal of device activity for wakeup latency management
        OALWakeupLatency_DeviceEnabled(devId, bEnable);
        }
    
    SPECIAL_DEBUG_MESSAGE_DONE(_PrcmDeviceEnableFClock, devId)

    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-_PrcmDeviceEnableFClock()=%d\r\n", TRUE));
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  _PrcmDeviceEnableFClock_DSS
//
//  Enables the appropriate functional clock
//
BOOL
_PrcmDeviceEnableFClock_DSS(
    OMAP_DEVICE devId, 
    BOOL bEnable
    )
{
    UINT i;
    UINT mask;
    BOOL rc = TRUE;
    SourceDeviceClocks_t *pSrcClocks;
    volatile unsigned int *pfclken;
    DeviceLookupEntry const *pEntry = &s_rgDeviceLookupTable[devId]; 
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+_PrcmDeviceEnableFClock_DSS(devId=%d)\r\n", devId));
      
    SPECIAL_DEBUG_MESSAGE2(_PrcmDeviceEnableFClock_DSS, devId, bEnable)

    pfclken = &g_pPrcmCm->pOMAP_DSS_CM->CM_FCLKEN_DSS;    
    pSrcClocks = pEntry->pSrcClocks;

    // NOTE:
    //   Disabling DSS1 and/or DSS2 may potential shutdown a clock that
    // is mapped OMAP_DEVICE_DSS.  For right now caller must be cautious
    // not to disable dss1 and/or dss2 while dss is enabled.
    //

    mask = INREG32(pfclken);
    switch (devId)
        {
        case OMAP_DEVICE_DSS1:
            if (bEnable == TRUE)
                mask |= CM_CLKEN_DSS1;
            else
                mask &= ~CM_CLKEN_DSS1;
            OUTREG32(pfclken, mask);
            break;

        case OMAP_DEVICE_DSS2:
            if (bEnable == TRUE)
                mask |= CM_CLKEN_DSS2;
            else
                mask &= ~CM_CLKEN_DSS2;
            OUTREG32(pfclken, mask);
            break;

        case OMAP_DEVICE_TVOUT:
            if (bEnable == TRUE)
                mask |= CM_CLKEN_TV;
            else
                mask &= ~CM_CLKEN_TV;
            OUTREG32(pfclken, mask);
            break;

        case OMAP_DEVICE_DSS:
            for (i = 0; i < pSrcClocks->size; ++i)
                {
                switch (pSrcClocks->rgSourceClocks[i])
                    {
                    case kDSS1_ALWON_FCLK:
                        PrcmDeviceEnableFClock(OMAP_DEVICE_DSS1, bEnable);
                        break;

                    case kDSS2_ALWON_FCLK:
                        PrcmDeviceEnableFClock(OMAP_DEVICE_DSS2, bEnable);
                        break;

                    default:
                        rc = FALSE;
                        goto cleanUp;
                    }        
                }
            break;
            
        default:
            rc = FALSE;
            goto cleanUp;
        }

    // notify oal of device activity for wakeup latency management
    OALWakeupLatency_DeviceEnabled(devId, bEnable);

    SPECIAL_DEBUG_MESSAGE_DONE(_PrcmDeviceEnableFClock_DSS, devId)

cleanUp:    
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-_PrcmDeviceEnableFClock_DSS()=%d\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  _PrcmDeviceEnableFClock_USB
//
//  Enables the appropriate USB Host functional clock
//
BOOL
_PrcmDeviceEnableFClock_USB(
    OMAP_DEVICE devId, 
    BOOL bEnable
    )
{
    BOOL rc = FALSE;
    DeviceLookupEntry const *pEntry;
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+_PrcmDeviceEnableFClock_USB(devId=%d)\r\n", devId));

    SPECIAL_DEBUG_MESSAGE2(_PrcmDeviceEnableFClock_USB, devId, bEnable)

    // determine which fclk to enable/disable
    pEntry = &s_rgDeviceLookupTable[devId];
    switch (devId)
        {
        case OMAP_DEVICE_HSUSB1:    // 48MHz FCLK
        case OMAP_DEVICE_HSUSB2:    // 120MHz FCLK
            // when these device IDs are used, gate final clock
            _PrcmDeviceEnableFClock(devId, bEnable);
            break; 

        case OMAP_DEVICE_USBHOST1:
        case OMAP_DEVICE_USBHOST2:
        case OMAP_DEVICE_USBHOST3:
            // when these device IDs are used, gate entire clock tree
            // USB host has 2 FCLKs, 120MHz and 48MHz
            // always gate 48MHz FCLK
            PrcmDeviceEnableFClock(OMAP_DEVICE_HSUSB1, bEnable);
            // if 120MHz FCLK is included in soruce clocks list, gate it as well
            if (pEntry->pSrcClocks->size == 4 &&
                pEntry->pSrcClocks->rgSourceClocks[3] == kUSBHOST_120M_FCLK)
                {
                // also gate 120MHz FCLK
                PrcmDeviceEnableFClock(OMAP_DEVICE_HSUSB2, bEnable);
                }
            break;
            
        default:
            goto cleanUp;
        }
    
    // notify oal of device activity for wakeup latency management
    OALWakeupLatency_DeviceEnabled(devId, bEnable);
    rc = TRUE;

    SPECIAL_DEBUG_MESSAGE_DONE(_PrcmDeviceEnableFClock_USB, devId)

cleanUp:    
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-_PrcmDeviceEnableFClock_USB()=%d\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  _PrcmDeviceEnableIClock
//
//  Enables the appropriate interface clock
//
BOOL
_PrcmDeviceEnableIClock(
    OMAP_DEVICE devId, 
    BOOL bEnable
    )
{
    UINT mask;
    OMAP_CM_REGS *pPrcmCm;
    volatile unsigned int *piclken;
    DeviceLookupEntry const *pEntry = &s_rgDeviceLookupTable[devId];    
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+_PrcmDeviceEnableIClock(devId=%d, %d)\r\n", devId, bEnable));

    SPECIAL_DEBUG_MESSAGE2(_PrcmDeviceEnableIClock, devId, bEnable)

    if (pEntry->piclk != NULL)
        {
        mask = pEntry->piclk->mask;    
        pPrcmCm = GetCmRegisterSet(pEntry->powerDomain);
        piclken = (volatile unsigned int*)((UCHAR*)pPrcmCm + pEntry->piclk->offset);        

        if (bEnable != FALSE)
            {
            _PrcmDeviceAddrefDeviceDomain(pEntry->powerDomain);
            SETREG32(piclken, mask);
            
            // wait until device is accessible
            if (pEntry->pidlestatus != NULL)
                {
                _PrcmDeviceWaitForDeviceAccess(devId);
                }
            }
        else
            {
            CLRREG32(piclken, mask);
            _PrcmDeviceReleaseDeviceDomain(pEntry->powerDomain);
            }    
        }

    SPECIAL_DEBUG_MESSAGE_DONE(_PrcmDeviceEnableIClock, devId)

    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-_PrcmDeviceEnableIClock()=%d\r\n", TRUE));
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  _PrcmDeviceEnableIClock_DSS
//
//  Enables the appropriate interface clock
//
BOOL
_PrcmDeviceEnableIClock_DSS(
    OMAP_DEVICE devId, 
    BOOL bEnable
    )
{
    UINT mask;
    BOOL rc = TRUE;       
    OMAP_CM_REGS *pPrcmCm;    
    volatile unsigned int *piclken;
    DeviceLookupEntry const *pEntry = &s_rgDeviceLookupTable[devId];
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+_PrcmDeviceEnableIClock_DSS(devId=%d)\r\n", devId));

    SPECIAL_DEBUG_MESSAGE2(_PrcmDeviceEnableIClock_DSS, devId, bEnable)

    if (pEntry->piclk != NULL)
        {
        mask = pEntry->piclk->mask;    
        pPrcmCm = GetCmRegisterSet(pEntry->powerDomain);
        piclken = (volatile unsigned int*)((UCHAR*)pPrcmCm + pEntry->piclk->offset);
    
        if (bEnable != FALSE)
            {
            _PrcmDeviceAddrefDeviceDomain(pEntry->powerDomain);
            SETREG32(piclken, mask);
            
            // wait until device is accessible
            if (pEntry->pidlestatus != NULL)
                {
                PrcmDomainSetClockState(POWERDOMAIN_DSS, 
                        CLOCKDOMAIN_DSS, 
                        CLKSTCTRL_WAKEUP
                        );
                _PrcmDeviceWaitForDeviceAccess(devId);
                PrcmDomainSetClockState(POWERDOMAIN_DSS, 
                        CLOCKDOMAIN_DSS, 
                        CLKSTCTRL_AUTOMATIC
                        );
                }
            }
        else
            {
            CLRREG32(piclken, mask);
            _PrcmDeviceReleaseDeviceDomain(pEntry->powerDomain);
            }    
        }

    SPECIAL_DEBUG_MESSAGE_DONE(_PrcmDeviceEnableIClock_DSS, devId)

    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-_PrcmDeviceEnableIClock_DSS()=%d\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  _PrcmDeviceEnableIClock_USB
//
//  Enables the appropriate USB Host interface clock
//
BOOL
_PrcmDeviceEnableIClock_USB(
    OMAP_DEVICE devId, 
    BOOL bEnable
    )
{
    BOOL rc = FALSE;
    DeviceLookupEntry const *pEntry;
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+_PrcmDeviceEnableIClock_USB(devId=%d)\r\n", devId));

    SPECIAL_DEBUG_MESSAGE2(_PrcmDeviceEnableIClock_USB, devId, bEnable)

    // determine which iclk to enable/disable
    pEntry = &s_rgDeviceLookupTable[devId];
    switch (devId)
        {
        case OMAP_DEVICE_HSUSB1:
        case OMAP_DEVICE_HSUSB2:
            // when these device IDs are used, just gate final clock
            _PrcmDeviceEnableIClock(devId, bEnable);
            break; 

        case OMAP_DEVICE_USBHOST1:
        case OMAP_DEVICE_USBHOST2:
        case OMAP_DEVICE_USBHOST3:
#if 0
            // Enable USB HOST hardware context save/restore for ES3.1
            if (!bEnable && IS_SILICON_ES3_1())
                {
                PrcmDomainSetMemoryState(POWERDOMAIN_USBHOST, 
                                            SAVEANDRESTORE,
                                            SAVEANDRESTORE
                                            );
                }
#endif            
            PrcmDeviceEnableIClock(OMAP_DEVICE_HSUSB1, bEnable);
            break;
            
        default:
            goto cleanUp;
        }
    
    rc = TRUE;

    SPECIAL_DEBUG_MESSAGE_DONE(_PrcmDeviceEnableIClock_USB, devId)

cleanUp:    
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-_PrcmDeviceEnableIClock_USB()=%d\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  _PrcmDeviceSetSourceClock
//
//  Sets the source clock and dividers for a device module
//
BOOL
_PrcmDeviceSetSourceClock(
    OMAP_DEVICE  devId, 
    UINT        count,
    UINT        rgClocks[]
    )
{
    BOOL rc = FALSE;
    SourceDeviceClocks_t *pSrcClocks;

    UNREFERENCED_PARAMETER(count);

    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+_PrcmDeviceSetSourceClock(devId=%d)\r\n", devId));

    pSrcClocks = s_rgDeviceLookupTable[devId].pSrcClocks;    
    rc = PrcmClockSetParent(pSrcClocks->rgSourceClocks[0], rgClocks[0]);
    
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-_PrcmDeviceSetSourceClock()=%d\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  _PrcmDeviceSetSourceClock_DSS
//
//  Sets the source clock and dividers for a device module
//
BOOL
_PrcmDeviceSetSourceClock_DSS(
    OMAP_DEVICE  devId, 
    UINT        count,
    UINT        rgClocks[]
    )
{
    UINT i;
//    UINT val;
    BOOL rc = FALSE;
    BOOL bDss1 = FALSE;
    BOOL bDss2 = FALSE;    
    BOOL bCurrentDss1 = FALSE;
    BOOL bCurrentDss2 = FALSE;

    SourceDeviceClocks_t *pSrcClocks;
    
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+_PrcmDeviceSetSourceClock_DSS(devId=%d)\r\n", devId));

    SPECIAL_DEBUG_MESSAGE(_PrcmDeviceSetSourceClock_DSS, devId)

    if (devId != OMAP_DEVICE_DSS) goto cleanUp;

    pSrcClocks = s_rgDeviceLookupTable[devId].pSrcClocks;

    for (i = 0; i < count; ++i)
        {
        switch (rgClocks[i])
            {
            case kDSS1_ALWON_FCLK:
                bDss1 = TRUE;
                break;

            case kDSS2_ALWON_FCLK:
                bDss2 = TRUE;
                break;

            default:
                goto cleanUp;
            }        

        switch (pSrcClocks->rgSourceClocks[i])
            {
            case kDSS1_ALWON_FCLK:
                bCurrentDss1 = TRUE;
                break;

            case kDSS2_ALWON_FCLK:
                bCurrentDss2 = TRUE;
                break;

            default:
                goto cleanUp;
            }        
        }

    i = 0;
    if (bDss1 == TRUE) 
	    pSrcClocks->rgSourceClocks[i++] = kDSS1_ALWON_FCLK;

    if (bDss2 == TRUE) 
	    pSrcClocks->rgSourceClocks[i++] = kDSS2_ALWON_FCLK;

    if (i > 0)
	    {
        if (s_rgDeviceLookupTable[devId].pfclk->refCount > 0)
	        {
            if (bCurrentDss1 != bDss1)
                PrcmDeviceEnableFClock(OMAP_DEVICE_DSS1, bDss1);

    	    if (bCurrentDss2 != bDss2)
                PrcmDeviceEnableFClock(OMAP_DEVICE_DSS2, bDss2);
            }
    	rc = TRUE;
	    }
			
/*
    i = 0;
    pSrcClocks = s_rgDeviceLookupTable[devId].pSrcClocks;
    if (bDss1 == TRUE) pSrcClocks->rgSourceClocks[i++] = kDSS1_ALWON_FCLK;
    if (bDss2 == TRUE) pSrcClocks->rgSourceClocks[i++] = kDSS2_ALWON_FCLK;

    if (i > 0)
        {
        // update source clock count
        pSrcClocks->size = i;
        
        // update clocks
        if (s_rgDeviceLookupTable[devId].pfclk->refCount > 0)
            {
            // enable both clocks
            val = INREG32(&g_pPrcmCm->pOMAP_DSS_CM->CM_FCLKEN_DSS);
            OUTREG32(&g_pPrcmCm->pOMAP_DSS_CM->CM_FCLKEN_DSS, val | CM_CLKEN_DSS_MASK);
            
            val &= ~CM_CLKEN_DSS_MASK;
            for (i = 0; i < pSrcClocks->size; ++i)
                {
                switch (pSrcClocks->rgSourceClocks[i])
                    {
                    case kDSS1_ALWON_FCLK:
                        val |= CM_CLKEN_DSS1;
                        break;

                    case kDSS2_ALWON_FCLK:
                        val |= CM_CLKEN_DSS2;
                        break;
                    }
                }

            // enable the requested source clock(s)
            OUTREG32(&g_pPrcmCm->pOMAP_DSS_CM->CM_FCLKEN_DSS, val);
            rc = TRUE;
            }
        else
            {
            rc = TRUE;
            }
        }
*/
    
    SPECIAL_DEBUG_MESSAGE_DONE(_PrcmDeviceSetSourceClock_DSS, devId)

cleanUp:    
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-_PrcmDeviceSetSourceClock_DSS()=%d\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  _PrcmDeviceSetSourceClock_USB
//
//  Sets the source clock and dividers for a device module
//  Note: only supports kUSBHOST_48M_FCLK and kUSBHOST_120M_FCLK
//
BOOL
_PrcmDeviceSetSourceClock_USB(
    OMAP_DEVICE  devId, 
    UINT         count,
    UINT         rgClocks[]
    )
{
    BOOL rc = FALSE;
    DeviceLookupEntry const *pEntry;

    SPECIAL_DEBUG_MESSAGE(_PrcmDeviceSetSourceClock_USB, devId)

    // validate parameters
    if (count > 1) goto cleanUp;
    if (rgClocks[0] != kUSBHOST_120M_FCLK && rgClocks[0] != kUSBHOST_48M_FCLK) goto cleanUp;

    // get current source clock/fclk information
    pEntry = &s_rgDeviceLookupTable[devId];
    switch (devId)
        {
        case OMAP_DEVICE_USBHOST1:
        case OMAP_DEVICE_USBHOST2:
        case OMAP_DEVICE_USBHOST3:
            // check source clock to see if it is already on the source clocks list
            if (pEntry->pSrcClocks->rgSourceClocks[3] == (SourceClock_e) rgClocks[0])
                {
                rc = TRUE;
                goto cleanUp;                
                }

            // update new fclk
            pEntry->pSrcClocks->rgSourceClocks[3] = rgClocks[0];
            if (rgClocks[0] == kUSBHOST_120M_FCLK)
                {
                pEntry->pSrcClocks->size = 4;
                if (pEntry->pfclk->refCount > 0)
                    {
                    PrcmDeviceEnableFClock(OMAP_DEVICE_HSUSB2, TRUE);
                    }
                }
            else
                {
                pEntry->pSrcClocks->size = 3;
                if (pEntry->pfclk->refCount > 0)
                    {
                    PrcmDeviceEnableFClock(OMAP_DEVICE_HSUSB2, FALSE);
                    }
                }
            break;
            
        default:
            goto cleanUp;
        }

    rc = TRUE;
    
    SPECIAL_DEBUG_MESSAGE_DONE(_PrcmDeviceSetSourceClock_USB, devId)

cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  PrcmDeviceGetEnabledState
//
//  returns current activity state of the device
//
BOOL
PrcmDeviceGetEnabledState(
    UINT devId,
    BOOL *pbEnable
    )
{
    BOOL rc = FALSE;
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+PrcmDeviceGetEnabledState(devId=%d)\r\n", devId));

    if (devId >= OMAP_DEVICE_GENERIC) goto cleanUp;

    SPECIAL_DEBUG_MESSAGE(PrcmDeviceGetEnabledState, devId)

    *pbEnable = FALSE;
    if (s_rgDeviceLookupTable[devId].pfclk != NULL)
        {
        *pbEnable = s_rgDeviceLookupTable[devId].pfclk->refCount != 0;
        }
    
    rc = TRUE;

    SPECIAL_DEBUG_MESSAGE_DONE(PrcmDeviceGetEnabledState, devId)

cleanUp:
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-PrcmDeviceGetEnabledState()=%d\r\n", rc));
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function:  PrcmDeviceGetAutoIdleState
//
//  returns current autoidle state of the device
//
BOOL
PrcmDeviceGetAutoIdleState(
    UINT devId,
    BOOL *pbEnable
    )
{
    BOOL rc = FALSE;
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+PrcmDeviceGetAutoIdleState(devId=%d)\r\n", devId));

    SPECIAL_DEBUG_MESSAGE(PrcmDeviceGetAutoIdleState, devId)

    if (devId >= OMAP_DEVICE_GENERIC) goto cleanUp;

    *pbEnable = FALSE;
    if (s_rgDeviceLookupTable[devId].pautoidle != NULL)
        {
        *pbEnable = s_rgDeviceLookupTable[devId].pautoidle->enabled;
        }
    
    rc = TRUE;

    SPECIAL_DEBUG_MESSAGE_DONE(PrcmDeviceGetAutoIdleState, devId)

cleanUp:
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-PrcmDeviceGetAutoIdleState()=%d\r\n", rc));
    return rc;
}   



BOOL
SelectSourceClocks(
	 OMAP_DEVICE		devID,
     UINT count,
	 OMAP_CLOCK	clockID[]
)
{
    return PrcmDeviceSetSourceClocks(devID,count,clockID);
}

//-----------------------------------------------------------------------------
//
//  Function:  PrcmDeviceSetSourceClocks
//
//  Sets the clock source for a device
//
BOOL
PrcmDeviceSetSourceClocks(
    UINT devId,
    UINT count,
    UINT rgClocks[]
    )
{
    BOOL rc = FALSE;
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+PrcmDeviceSetSourceClocks(devId=%d)\r\n", devId));

    SPECIAL_DEBUG_MESSAGE(PrcmDeviceSetSourceClocks, devId)

    if (devId >= OMAP_DEVICE_GENERIC) goto cleanUp;

    Lock(Mutex_DeviceClock);
    if (s_rgDeviceLookupTable[devId].pSrcClocks != NULL &&
        s_rgClockManagementRoutines[s_rgDeviceLookupTable[devId].powerDomain].PrcmSetSourceDeviceClock)
        {
        rc = s_rgClockManagementRoutines[s_rgDeviceLookupTable[devId].powerDomain].PrcmSetSourceDeviceClock(devId, count, rgClocks);
        }
    Unlock(Mutex_DeviceClock);    
    
    SPECIAL_DEBUG_MESSAGE_DONE(PrcmDeviceSetSourceClocks, devId)

cleanUp:
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-PrcmDeviceSetSourceClocks()=%d\r\n", rc));
    return rc;
}   


//------------------------------------------------------------------------------
//
//  Function:  PrcmDeviceEnableIClock
//
//  Enables the appropriate interface clock
//
BOOL
PrcmDeviceEnableIClock(
    UINT devId, 
    BOOL bEnable
    )
{
    BOOL rc = TRUE;
    BOOL bUpdateClocks = FALSE;
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+PrcmDeviceEnableIClock(devId=%d, %d)\r\n", devId, bEnable));

    SPECIAL_DEBUG_MESSAGE2(PrcmDeviceEnableIClock, devId, bEnable)

    if (devId >= OMAP_DEVICE_GENERIC) goto cleanUp;
    if (s_rgDeviceLookupTable[devId].piclk == NULL) goto cleanUp;

    if (bEnable != FALSE)
        {
        if (InterlockedIncrement(&s_rgDeviceLookupTable[devId].piclk->refCount) == 1)
            {
            bUpdateClocks = TRUE;
            }
        }
    else if (s_rgDeviceLookupTable[devId].piclk->refCount > 0)
        {
        if (InterlockedDecrement(&s_rgDeviceLookupTable[devId].piclk->refCount) == 0)
            {
            bUpdateClocks = TRUE;
            }
        }

    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L" PrcmDeviceEnableIClock %supdate clocks\r\n", bUpdateClocks ? L"" : L"skip "));
    // update hardware if clock is being enabled and it's not a virtual bit
    if (bUpdateClocks == TRUE && s_rgDeviceLookupTable[devId].piclk->bVirtual == FALSE)
        {
        Lock(Mutex_DeviceClock);
        if (bEnable)
            {
            _PrcmDeviceAddrefDeviceDomain(s_rgDeviceLookupTable[devId].powerDomain);
            }
        else
            {
            _PrcmDeviceReleaseDeviceDomain(s_rgDeviceLookupTable[devId].powerDomain);
            }
        
        _PrcmDeviceEnableWakeUp(devId, bEnable);
        if (s_rgClockManagementRoutines[s_rgDeviceLookupTable[devId].powerDomain].PrcmDeviceEnableIClock)
            {
            rc = s_rgClockManagementRoutines[s_rgDeviceLookupTable[devId].powerDomain].PrcmDeviceEnableIClock(devId, bEnable);
            }
        Unlock(Mutex_DeviceClock);
        }

    SPECIAL_DEBUG_MESSAGE_DONE(PrcmDeviceEnableIClock, devId)

cleanUp:
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-PrcmDeviceEnableIClock()=%d\r\n", rc));

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  PrcmDeviceEnableFClock
//
//  Enables the appropriate functional clock
//
BOOL
PrcmDeviceEnableFClock(
    UINT devId, 
    BOOL bEnable
    )
{
    BOOL rc = TRUE;
    BOOL bUpdateClocks = FALSE;
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+PrcmDeviceEnableFClock(devId=%d, %d)\r\n", devId, bEnable));

    SPECIAL_DEBUG_MESSAGE2(PrcmDeviceEnableFClock, devId, bEnable)

    if (devId >= OMAP_DEVICE_GENERIC) goto cleanUp;
    // update autoidle for device
    _PrcmDeviceHwUpdateAutoidle(devId);
    
    if (s_rgDeviceLookupTable[devId].pfclk == NULL) goto cleanUp;

    if (bEnable != FALSE)
        {
        if (InterlockedIncrement(&s_rgDeviceLookupTable[devId].pfclk->refCount) == 1 )
            {
            bUpdateClocks = TRUE;
            }
        }
    else if (s_rgDeviceLookupTable[devId].pfclk->refCount > 0)
        {
        if (InterlockedDecrement(&s_rgDeviceLookupTable[devId].pfclk->refCount) == 0)
            {
            bUpdateClocks = TRUE;
            }
        }
    
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L" PrcmDeviceEnableFClock %supdate clocks\r\n", bUpdateClocks ? L"" : L"skip "));
    // update hardware if clock is being enabled and it's not a virtual bit
    if (bUpdateClocks == TRUE && s_rgDeviceLookupTable[devId].pfclk->bVirtual == FALSE)
        {
        Lock(Mutex_DeviceClock);
        if (bEnable)
            {
            SPECIAL_DEBUG_MESSAGE2(_PrcmUpdateDeviceClockSource enable, devId, bEnable)
            _PrcmUpdateDeviceClockSource(s_rgDeviceLookupTable[devId].pSrcClocks, bEnable);
            }
        
        if (s_rgClockManagementRoutines[s_rgDeviceLookupTable[devId].powerDomain].PrcmDeviceEnableFClock)
            {
            rc = s_rgClockManagementRoutines[s_rgDeviceLookupTable[devId].powerDomain].PrcmDeviceEnableFClock(devId, bEnable);
            }
        
        if (!bEnable)
            {
            SPECIAL_DEBUG_MESSAGE2(_PrcmUpdateDeviceClockSource disable, devId, bEnable)
            _PrcmUpdateDeviceClockSource(s_rgDeviceLookupTable[devId].pSrcClocks, bEnable);
            }
        PrcmDomainUpdateRefCount(s_rgDeviceLookupTable[devId].powerDomain, bEnable);
        Unlock(Mutex_DeviceClock);
        }

    SPECIAL_DEBUG_MESSAGE_DONE(PrcmDeviceEnableFClock, devId)

cleanUp:
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-PrcmDeviceEnableFClock()=%d\r\n", rc));

    return rc;
}
BOOL
EnableDeviceClocks(
    OMAP_DEVICE		devID,
    BOOL			bEnable
    )
{
    return PrcmDeviceEnableClocks(devID,bEnable);
}
//------------------------------------------------------------------------------
//
//  Function:  PrcmDeviceEnableClocks
//
//  Enables the appropriate functional and interface clocks
//
BOOL
PrcmDeviceEnableClocks(
    UINT devId, 
    BOOL bEnable
    )
{
    UINT oldState = bEnable ? D4 : D0;
    UINT newState = bEnable ? D0 : D4;
    SPECIAL_DEBUG_MESSAGE2(PrcmDeviceEnableClocks, devId, bEnable)

    OALMux_UpdateOnDeviceStateChange(devId, oldState, newState, TRUE);

    PrcmDeviceEnableFClock(devId, bEnable);
    PrcmDeviceEnableIClock(devId, bEnable);

    OALMux_UpdateOnDeviceStateChange(devId, oldState, newState, FALSE);

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  PrcmDeviceEnableClocksKernel
//
//  Enables the appropriate functional and interface clocks, no system calls
//
BOOL
PrcmDeviceEnableClocksKernel(
    UINT devId, 
    BOOL bEnable
    )
{
    UINT oldState = bEnable ? D4 : D0;
    UINT newState = bEnable ? D0 : D4;
    g_bSingleThreaded = TRUE;
    OALMux_UpdateOnDeviceStateChange(devId, oldState, newState, TRUE);
    PrcmDeviceEnableFClock(devId, bEnable);
    PrcmDeviceEnableIClock(devId, bEnable);
    OALMux_UpdateOnDeviceStateChange(devId, oldState, newState, FALSE);
    g_bSingleThreaded = FALSE;
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  PrcmDeviceEnableAutoIdle
//
//  Enables autoidle for a device
//
BOOL
PrcmDeviceEnableAutoIdle(
    UINT devId, 
    BOOL bEnable
    )
{
    BOOL rc = TRUE;
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+PrcmDeviceEnableAutoIdle(devId=%d, %d)\r\n", devId, bEnable));

    SPECIAL_DEBUG_MESSAGE2(PrcmDeviceEnableAutoIdle, devId, bEnable)

    if (devId >= OMAP_DEVICE_GENERIC) goto cleanUp;
    if (s_rgDeviceLookupTable[devId].pautoidle == NULL) goto cleanUp;

    if (s_rgClockManagementRoutines[s_rgDeviceLookupTable[devId].powerDomain].PrcmDeviceEnableAutoIdle)
        {
        Lock(Mutex_DeviceClock);
        rc = s_rgClockManagementRoutines[s_rgDeviceLookupTable[devId].powerDomain].PrcmDeviceEnableAutoIdle(devId, bEnable);
        Unlock(Mutex_DeviceClock);
        }

    SPECIAL_DEBUG_MESSAGE_DONE(PrcmDeviceEnableAutoIdle, devId)

cleanUp:
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-PrcmDeviceEnableAutoIdle()=%d\r\n", rc));
    return rc;
}    


//------------------------------------------------------------------------------
//
//  Function:  PrcmDeviceGetContextState
//
//  retrieves the context state of a device.
//
//  TRUE - context for device is retained
//  FALSE - context for device is reset.
//
BOOL
PrcmDeviceGetContextState(
    UINT devId,
    BOOL bSet
    )
{
    BOOL rc = TRUE;
    OALMSG(OAL_FUNC, (L"+PrcmDeviceGetContextState(devId=%d)\r\n", devId));

    if (devId >= OMAP_DEVICE_GENERIC) goto cleanUp;

    if (s_rgDeviceLookupTable[devId].piclk != NULL)
        {
        rc = DomainGetDeviceContextState(s_rgDeviceLookupTable[devId].powerDomain,
                s_rgDeviceLookupTable[devId].piclk,
                bSet
                );
        }

cleanUp:
    OALMSG(OAL_FUNC, (L"-PrcmDeviceGetContextState()=%d\r\n", rc));
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function:  PrcmDeviceGetSourceClockInfo
//
//  returns clock information for a given device
//
BOOL
PrcmDeviceGetSourceClockInfo(
    UINT devId,
    IOCTL_PRCM_DEVICE_GET_SOURCECLOCKINFO_OUT *pInfo
    )
{
    UINT i;
    BOOL rc = FALSE;
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+PrcmDeviceGetSourceClockInfo(devId=%d)\r\n", devId));

    SPECIAL_DEBUG_MESSAGE(PrcmDeviceGetSourceClockInfo, devId)

    if (devId >= OMAP_DEVICE_GENERIC) goto cleanUp;
    
    if (s_rgDeviceLookupTable[devId].pSrcClocks == NULL) goto cleanUp;
    
    pInfo->count = s_rgDeviceLookupTable[devId].pSrcClocks->size;
    for (i = 0; i < pInfo->count; ++i)
        {
        pInfo->rgSourceClocks[i].nLevel = 1;
        pInfo->rgSourceClocks[i].clockId = s_rgDeviceLookupTable[devId].pSrcClocks->rgSourceClocks[i];
        PrcmClockGetParentClockRefcount(pInfo->rgSourceClocks[i].clockId, 
            pInfo->rgSourceClocks[i].nLevel, 
            &pInfo->rgSourceClocks[i].refCount
            );
        }

    rc = TRUE;

    SPECIAL_DEBUG_MESSAGE_DONE(PrcmDeviceGetSourceClockInfo, devId)

cleanUp:
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-PrcmDeviceGetSourceClockInfo()=%d\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
BOOL
DeviceInitialize()
{
    unsigned int i;
    OMAP_CM_REGS *pPrcmCm;
    volatile unsigned int *preg;
    
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+DeviceInitialize()\r\n"));

    // iterate through all devices and update its state information
    for (i = 0; i < OMAP_DEVICE_COUNT - 1; ++i)
        {
        pPrcmCm = GetCmRegisterSet(s_rgDeviceLookupTable[i].powerDomain);

        // update autoidle information
        if (s_rgDeviceLookupTable[i].pautoidle != NULL)
            {
            preg = (volatile unsigned int*)((UCHAR*)pPrcmCm + s_rgDeviceLookupTable[i].pautoidle->offset);
            if (INREG32(preg) & s_rgDeviceLookupTable[i].pautoidle->mask)
                {
                PrcmDeviceEnableAutoIdle(i, TRUE);
                }            
            }
        
        //Avoid reference counting DSS device to keep bootloader screen on all the way
        //to the display driver initialization
        if(i == OMAP_DEVICE_DSS || i == OMAP_DEVICE_DSS1 || i == OMAP_DEVICE_DSS2)
            {
            continue;
            }

        // update functional clock information
        if (s_rgDeviceLookupTable[i].pfclk != NULL)
            {
            preg = (volatile unsigned int*)((UCHAR*)pPrcmCm + s_rgDeviceLookupTable[i].pfclk->offset);
            if (INREG32(preg) & s_rgDeviceLookupTable[i].pfclk->mask)
                {
                PrcmDeviceEnableFClock(i, TRUE);
                }            
            }

        // update inteface clock information
        if (s_rgDeviceLookupTable[i].piclk != NULL)
            {
            preg = (volatile unsigned int*)((UCHAR*)pPrcmCm + s_rgDeviceLookupTable[i].piclk->offset);
            if (INREG32(preg) & s_rgDeviceLookupTable[i].piclk->mask)
                {
                PrcmDeviceEnableIClock(i, TRUE);
                }            
            }
        }

#if 0
    {   // Enable the save and restore mechanism for the USB Host device
        OMAP_PRCM_USBHOST_PRM_REGS *pOMAP_PRCM_USBHOST_PRM_REGS;
        OMAP_PRCM_CORE_PRM_REGS *pOMAP_PRCM_CORE_PRM_REGS;
        PHYSICAL_ADDRESS pa;

        pa.QuadPart = OMAP_PRCM_USBHOST_PRM_REGS_PA;
        pOMAP_PRCM_USBHOST_PRM_REGS = (OMAP_PRCM_USBHOST_PRM_REGS *)MmMapIoSpace(pa, sizeof(OMAP_PRCM_USBHOST_PRM_REGS), FALSE);
        
        pa.QuadPart = OMAP_PRCM_CORE_PRM_REGS_PA;
        pOMAP_PRCM_CORE_PRM_REGS = (OMAP_PRCM_CORE_PRM_REGS *)MmMapIoSpace(pa, sizeof(OMAP_PRCM_CORE_PRM_REGS), FALSE);
        
        /* 0x4830 74E0 bit4 ASAVEANDRESTORE for USBHOST enable Save and Restore mechanism */
        pOMAP_PRCM_USBHOST_PRM_REGS->PM_PWSTCTRL_USBHOST |= (0x01 << 4); //SAVEANDRESTORE

        /* 0x4830 6AE0 bit4 ASAVEANDRESTORE for USBTLL enable Save and Restore mechanism */
        pOMAP_PRCM_CORE_PRM_REGS->PM_PWSTCTRL_CORE |= (0x01 << 4); //SAVEANDRESTORE

        MmUnmapIoSpace(pOMAP_PRCM_USBHOST_PRM_REGS, sizeof(OMAP_PRCM_USBHOST_PRM_REGS));
        MmUnmapIoSpace(pOMAP_PRCM_CORE_PRM_REGS, sizeof(OMAP_PRCM_CORE_PRM_REGS));
    }
#endif
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-DeviceInitialize()\r\n"));

    return TRUE;
}
