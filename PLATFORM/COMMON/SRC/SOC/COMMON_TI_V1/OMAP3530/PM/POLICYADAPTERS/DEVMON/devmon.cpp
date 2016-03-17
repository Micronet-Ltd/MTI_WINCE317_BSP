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
//  File:  devmon.cpp
//
//  This power policy adapter is an autonomous policy which listens for
//  device state changes and ensures the operating mode matches the 
//  devices performance requirements.
//
#include "omap.h"
#include <windows.h>
#include <oal.h>
#include <oalex.h>
#include <ceddk.h>
#include <ceddkex.h>
#include "omap_dvfs.h"
#include <indexlist.h>
#include "_constants.h"
#include "devoppmap.h"

//-----------------------------------------------------------------------------
#define DEVMON_COOKIE               'dmon'
#define CONSTRAINT_ID_DOMAIN        L"PWRDOM"
#define CONSTRAINT_ID_DVFS          L"DVFS"
#define SYSTEM_SUSPEND_STATE        L"SuspendState"

#define DEFAULT_ENABLE_DOMAIN_POWERSTATE    D2

//-----------------------------------------------------------------------------
//  local structures
typedef struct
{
    DWORD                           cookie;
    CRITICAL_SECTION                cs;
    DWORD                           domainMask;
    DWORD                           currentOpm;
    HANDLE                          hDvfsConstraint;
    HANDLE                          hDomainConstraint;
    DWORD                           rgOpmCount[kOpmCount];
    DWORD                           rgDomainCount[POWERDOMAIN_COUNT];
} DeviceMonitorPolicyInfo_t;

//-----------------------------------------------------------------------------
//  local variables
static IndexList<DWORD>             s_IndexList;
static DeviceMonitorPolicyInfo_t    s_DeviceMonitorInfo;
static DevicePerformanceMapTbl_t    s_DevPerfTable[OMAP_DEVICE_COUNT] = {DEVICE_OPP_TABLE()};

//-----------------------------------------------------------------------------
// 
//  Function:  Lock
//
//  enter critical section
//
void
Lock()
{
    EnterCriticalSection(&s_DeviceMonitorInfo.cs);
}

//-----------------------------------------------------------------------------
// 
//  Function:  Unlock
//
//  exit critical section
//
void
Unlock()
{
    LeaveCriticalSection(&s_DeviceMonitorInfo.cs);
}

//-----------------------------------------------------------------------------
// 
//  Function:  InitializeSuspendState
//
//  sets the chip state to enter on suspend
//
BOOL
InitializeSuspendState(
    _TCHAR const * szContext
    )
{
    DWORD state;
    DWORD size;
    BOOL rc = TRUE;
    HKEY hKey = NULL;

    // read registry to get initialization information
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szContext, 0, 0, &hKey) != ERROR_SUCCESS)
        {
        goto cleanUp;
        }

    // get ceiling value
    size = sizeof(DWORD);
    if (RegQueryValueEx(hKey, SYSTEM_SUSPEND_STATE, 0, 0, (BYTE*)&state, &size) == ERROR_SUCCESS)
        {
        KernelIoControl(IOCTL_PRCM_SET_SUSPENDSTATE, &state, sizeof(DWORD), NULL, 0, NULL);
        }

    RegCloseKey(hKey);

cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  UpdateConstraint
//
//  Updates the constraint
//
void
UpdateConstraint(
    DevicePerformanceMapTbl_t  *pDeviceMap,
    BOOL                        bPreNotify
    )
{
    DWORD i;
    DWORD opm = (DWORD)kOpm0;
    POWERDOMAIN_CONSTRAINT_INFO domainConstraint;

    // check domain states
    if (pDeviceMap->powerDomain != POWERDOMAIN_NULL)
        {
        if (bPreNotify == TRUE && (s_DeviceMonitorInfo.domainMask & (1 << pDeviceMap->powerDomain)) == 0)
            {
            // apply constraint if we don't already have a constraint
            // on the power domain

            // initialize structure
            domainConstraint.powerDomain = pDeviceMap->powerDomain;
            domainConstraint.state = DEFAULT_ENABLE_DOMAIN_POWERSTATE;
            domainConstraint.size = sizeof(POWERDOMAIN_CONSTRAINT_INFO);

            // apply constraint
            PmxUpdateConstraint(s_DeviceMonitorInfo.hDomainConstraint, 
                CONSTRAINT_MSG_POWERDOMAIN_REQUEST, 
                &domainConstraint, 
                sizeof(POWERDOMAIN_CONSTRAINT_INFO)
                );

            // set domain mask
            s_DeviceMonitorInfo.domainMask |= (1 << pDeviceMap->powerDomain);
            }
        else if (bPreNotify == FALSE && s_DeviceMonitorInfo.rgDomainCount[pDeviceMap->powerDomain] == 0)                 
            {
            // release constraint on domain if we no longer need it

            // initialize structure
            domainConstraint.powerDomain = pDeviceMap->powerDomain;
            domainConstraint.state = CONSTRAINT_STATE_NULL;
            domainConstraint.size = sizeof(POWERDOMAIN_CONSTRAINT_INFO);

            // apply constraint
            PmxUpdateConstraint(s_DeviceMonitorInfo.hDomainConstraint, 
                CONSTRAINT_MSG_POWERDOMAIN_REQUEST, 
                &domainConstraint, 
                sizeof(POWERDOMAIN_CONSTRAINT_INFO)
                );

            // clear domain mask
            s_DeviceMonitorInfo.domainMask &= ~(1 << pDeviceMap->powerDomain);
            }
        }

    // check operating mode
        for (i = (DWORD)kOpm0; i < (DWORD)kOpmCount; ++i)
            {
            if (s_DeviceMonitorInfo.rgOpmCount[i] > 0)
                {
                opm = i;
                }
            }

        // check if the constraint needs to be changed
        if (opm != s_DeviceMonitorInfo.currentOpm)
            {
            PmxUpdateConstraint(s_DeviceMonitorInfo.hDvfsConstraint, 
                CONSTRAINT_MSG_DVFS_REQUEST, 
                &opm, 
                sizeof(DWORD)
                );

            s_DeviceMonitorInfo.currentOpm = opm;
            }

}

//-----------------------------------------------------------------------------
// 
//  Function:  DEVMON_InitPolicy
//
//  Policy  initialization
//
HANDLE
DEVMON_InitPolicy(
    _TCHAR const *szContext
    )
{
    DWORD dwLevel = CONSTRAINT_STATE_NULL;
    POWERDOMAIN_CONSTRAINT_INFO domainConstraint;
    
    // initializt global structure
    memset(&s_DeviceMonitorInfo, 0, sizeof(DeviceMonitorPolicyInfo_t));

    s_DeviceMonitorInfo.currentOpm = kOpm0;
    s_DeviceMonitorInfo.cookie = DEVMON_COOKIE;
    InitializeCriticalSection(&s_DeviceMonitorInfo.cs);

    s_DeviceMonitorInfo.hDvfsConstraint = PmxSetConstraintById(
        CONSTRAINT_ID_DVFS, 
        CONSTRAINT_MSG_DVFS_REQUEST, 
        &dwLevel,
        sizeof(DWORD)
        );

    // initialize domain constraint
    domainConstraint.powerDomain = POWERDOMAIN_CORE;
    domainConstraint.size = sizeof(POWERDOMAIN_CONSTRAINT_INFO);
    domainConstraint.state = CONSTRAINT_STATE_NULL;
    
    s_DeviceMonitorInfo.hDomainConstraint = PmxSetConstraintById(
        CONSTRAINT_ID_DOMAIN, 
        CONSTRAINT_MSG_POWERDOMAIN_REQUEST, 
        (void*)&domainConstraint, 
        sizeof(POWERDOMAIN_CONSTRAINT_INFO)
        );
    
    // initialize suspend state
    InitializeSuspendState(szContext);

    return (HANDLE)&s_DeviceMonitorInfo;
} 

//-----------------------------------------------------------------------------
// 
//  Function:  DEVMON_DeinitPolicy
//
//  Policy uninitialization
//
BOOL
DEVMON_DeinitPolicy(
    HANDLE hPolicyAdapter
    )
{
    BOOL rc = FALSE;

    // validate parameters
    if (hPolicyAdapter != (HANDLE)&s_DeviceMonitorInfo) goto cleanUp;

    // reset structure
    if (s_DeviceMonitorInfo.hDvfsConstraint != NULL)
        {
        PmxReleaseConstraint(s_DeviceMonitorInfo.hDvfsConstraint);
        }

    DeleteCriticalSection(&s_DeviceMonitorInfo.cs);

    rc = TRUE;

cleanUp:
    return rc;
} 

//-----------------------------------------------------------------------------
// 
//  Function:  DEVMON_PreDeviceStateChange
//
//  device state change handler
//
BOOL 
DEVMON_PreDeviceStateChange(
    HANDLE hPolicyAdapter,
    UINT dev, 
    UINT oldState, 
    UINT newState
    )
{
    // validate parameters
    if (hPolicyAdapter != (HANDLE)&s_DeviceMonitorInfo) return FALSE;

    // record the new device state for the device
    if (dev >= OMAP_DEVICE_GENERIC) return TRUE;

    UPDATE_DEVICE_STATE(s_DevPerfTable, dev, newState);
        
    // D3 is the inflection point
    if (newState < (UINT)D3 && oldState >= (UINT)D3)
        {
        // apply any constraints defined for device
        //
        s_DeviceMonitorInfo.rgOpmCount[s_DevPerfTable[dev].opm] += 1;

        // update power domain states
        if (s_DevPerfTable[dev].powerDomain != POWERDOMAIN_NULL)
            {
            s_DeviceMonitorInfo.rgDomainCount[s_DevPerfTable[dev].powerDomain] += 1;
            }
        UpdateConstraint(&s_DevPerfTable[dev], TRUE);
        }
    else if (newState >= (UINT)D3 && oldState < (UINT)D3)
        {
        // release any constraints defined for device
        //
        s_DeviceMonitorInfo.rgOpmCount[s_DevPerfTable[dev].opm] -= 1;

        // update power domain states
        if (s_DevPerfTable[dev].powerDomain != POWERDOMAIN_NULL)
            {
            s_DeviceMonitorInfo.rgDomainCount[s_DevPerfTable[dev].powerDomain] -= 1;
            }
        UpdateConstraint(&s_DevPerfTable[dev], FALSE);
        }
    
    return TRUE;
}

//-----------------------------------------------------------------------------
// 
//  Function:  DEVMON_PostDeviceStateChange
//
//  device state change handler
//
BOOL 
DEVMON_PostDeviceStateChange(
    HANDLE hPolicyAdapter,
    UINT dev, 
    UINT oldState, 
    UINT newState
    )
{
    UNREFERENCED_PARAMETER(hPolicyAdapter);
    UNREFERENCED_PARAMETER(dev);
    UNREFERENCED_PARAMETER(oldState);
    UNREFERENCED_PARAMETER(newState);
    return FALSE;
}
//------------------------------------------------------------------------------
