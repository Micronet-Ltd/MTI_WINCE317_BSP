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
#include <windows.h>
#include <nkintr.h>
#include <pkfuncs.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>
#include <initguid.h>
#include <bus.h>
#include <bsp_cfg.h>

// duplicate of definition in PLATFORM\COMMON\SRC\SOC\OMAP35XX_TPS659XX_TI_V1\omap35xx\OAL\prcm\prcm_priv.h
typedef struct {
    ClockDomain_e           clockDomain;
    DWORD                   requiredWakeupDependency;
    DWORD                   requiredSleepDependency;
} DomainDependencyRequirement;

//-----------------------------------------------------------------------------
DomainDependencyRequirement s_rgClockDomainDependency[POWERDOMAIN_COUNT] =
{
    {
        CLOCKDOMAIN_NULL,       0xFFFFFFFF,             0xFFFFFFFF                  // POWERDOMAIN_WAKEUP
    }, {
        CLOCKDOMAIN_NULL,       0xFFFFFFFF,             0xFFFFFFFF                  // POWERDOMAIN_CORE
    }, {
        CLOCKDOMAIN_PERIPHERAL, CM_WKDEP_PER_INIT,      CM_SLEEPDEP_PER_INIT        // POWERDOMAIN_PERIPHERAL
    }, {
        CLOCKDOMAIN_USBHOST,    CM_WKDEP_USBHOST_INIT,  CM_SLEEPDEP_USBHOST_INIT    // POWERDOMAIN_USBHOST
    }, {
        CLOCKDOMAIN_EMULATION,  0xFFFFFFFF,             0xFFFFFFFF                  // POWERDOMAIN_EMULATION
    }, {
        CLOCKDOMAIN_MPU,        CM_WKDEP_MPU_INIT,      0xFFFFFFFF                  // POWERDOMAIN_MPU
    }, {
        CLOCKDOMAIN_DSS,        CM_WKDEP_DSS_INIT,      CM_SLEEPDEP_DSS_INIT        // POWERDOMAIN_DSS
    }, {
        CLOCKDOMAIN_NEON,       CM_WKDEP_NEON_INIT,     0xFFFFFFFF                  // POWERDOMAIN_NEON
    }, {
        CLOCKDOMAIN_IVA2,       CM_WKDEP_IVA2_INIT,     0xFFFFFFFF                  // POWERDOMAIN_IVA2
    }, {
        CLOCKDOMAIN_CAMERA,     CM_WKDEP_CAM_INIT,      CM_SLEEPDEP_CAM_INIT        // POWERDOMAIN_CAMERA
    }, {
        CLOCKDOMAIN_SGX,        CM_WKDEP_SGX_INIT,      CM_SLEEPDEP_SGX_INIT        // POWERDOMAIN_SGX
    }, {
        CLOCKDOMAIN_NULL,       0xFFFFFFFF,             0xFFFFFFFF                  // POWERDOMAIN_EFUSE
    }, {
        CLOCKDOMAIN_NULL,       0xFFFFFFFF,             0xFFFFFFFF                  // POWERDOMAIN_SMARTREFLEX
    }
};
//-----------------------------------------------------------------------------
