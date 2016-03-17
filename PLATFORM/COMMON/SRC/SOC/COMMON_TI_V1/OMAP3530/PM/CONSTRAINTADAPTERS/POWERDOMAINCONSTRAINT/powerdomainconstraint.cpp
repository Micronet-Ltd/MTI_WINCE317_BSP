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
//  File:  powerdomainconstraint.cpp
//
//
#include "omap.h"
#include <windows.h>
#include <oal.h>
#include <oalex.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <indexlist.h>
#include "_constants.h"
#include "powerdomainconstraint.h"

//-----------------------------------------------------------------------------
//  registry key names
//
#define POWERDOMAIN_FORCE           (0x80000000)

#define REGEDIT_POWERDOMAIN_CEILING (_T("Ceiling"))
#define REGEDIT_POWERDOMAIN_FLOOR   (_T("Floor"))

//-----------------------------------------------------------------------------
//  external routines
//
extern void NotifyDomainCallbacks(UINT powerDomain, UINT state, DWORD idContext);

//-----------------------------------------------------------------------------
// 
//  Function:  UpdatePowerDomainConstraint
//
//  Updates the power state of a power domain
//
BOOL
PowerDomainConstraint::
UpdatePowerDomainConstraint(
    DWORD idContext
    )
{
    int i;
    DWORD newState = (DWORD) PwrDeviceUnspecified;

    UNREFERENCED_PARAMETER(idContext);
    
    // first check if there is a constraint being forced
    if (m_powerDomainForce != -1)
        {
        DWORD *pDataNode;
        pDataNode = m_IndexList.GetIndex(m_powerDomainForce);
        if (pDataNode != NULL)
            {
            newState = *pDataNode;
            }
        }

    // get highest operating mode if not forced
    if (newState == PwrDeviceUnspecified)
        {
        newState = m_powerDomainFloor;
        for (i = (signed)m_powerDomainCeiling; i <= (signed)m_powerDomainFloor; ++i)
            {
            if (m_rgPowerDomainConstraints[i] > 0)
                {
                newState = (DWORD)i;
                break;
                }
            }
        }

    // save new state
    m_powerDomainCurrent = newState;
        
    return TRUE;
}

//-----------------------------------------------------------------------------
// 
//  Function:  InitializePowerDomain
//
//  initializes the power domain
//
BOOL
PowerDomainConstraint::
InitializePowerDomain(
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
    if (RegQueryValueEx(hKey, REGEDIT_POWERDOMAIN_CEILING, 0, 0, (BYTE*)&state, &size) == ERROR_SUCCESS)
        {
        m_powerDomainCeiling = max(m_powerDomainCeiling, state);
        }
    
    // get floor value
    size = sizeof(DWORD);
    if (RegQueryValueEx(hKey, REGEDIT_POWERDOMAIN_FLOOR, 0, 0, (BYTE*)&state, &size) == ERROR_SUCCESS)
        {
        m_powerDomainFloor = min(m_powerDomainFloor, state);
        }

    // make sure floor (ex. D2) is not less than the ceiling (ex. D3)
    if (m_powerDomainFloor < m_powerDomainCeiling)
        {
        m_powerDomainCeiling = m_powerDomainFloor;
        }
 
    m_powerDomainCurrent = m_powerDomainFloor;

    RegCloseKey(hKey);

cleanUp:
    return rc;
} 

//-----------------------------------------------------------------------------
// 
//  Function:  UpdatePowerDomainConstraint
//
//  Constraint update
//
BOOL
PowerDomainConstraint::
UpdatePowerDomainConstraint(
    HANDLE hConstraintContext, 
    DWORD msg, 
    DWORD newState, 
    UINT  size,
    DWORD idContext
    )
{  
    BOOL rc = FALSE;
    DWORD *pDataNode;
    DWORD id = (DWORD)hConstraintContext - 1;
    
    // validate parameters
    if (size != sizeof(DWORD)) goto cleanUp;
    if (id > m_IndexList.MaxIndex()) goto cleanUp;

    // get data node
    pDataNode = m_IndexList.GetIndex(id);
    if (pDataNode == NULL) goto cleanUp;

    // determine new state
    if (CONSTRAINT_STATE_NULL == newState)
        {
        newState = (DWORD) PwrDeviceUnspecified;
        }
    else if (CONSTRAINT_STATE_FLOOR == newState)
        {
        newState = m_powerDomainFloor;
        }
    else 
        {
        newState = max(min(newState, m_powerDomainFloor), m_powerDomainCeiling);
        }

    // serialize access
    Lock();
           
    // process constraint message
    switch (msg)
        {
        case POWERDOMAIN_FORCE:
            m_powerDomainForce = id;

            //fall-through
            
        case CONSTRAINT_MSG_POWERDOMAIN_REQUEST:
            // update new constraint
            if (*pDataNode != PwrDeviceUnspecified)
                {
                m_rgPowerDomainConstraints[*pDataNode] -= 1;
                }

            if (newState != PwrDeviceUnspecified)
                {
                m_rgPowerDomainConstraints[newState] += 1;
                }
            *pDataNode = newState;
            break;
        }
    
    // update operating mode
    rc = UpdatePowerDomainConstraint(
            idContext
            );
    
    Unlock();

cleanUp:
    return rc;
} 

//-----------------------------------------------------------------------------
// 
//  Function:  UpdatePowerDomainConstraint
//
//  Updates the power state of a power domain
//
BOOL
MpuPowerDomainConstraint::
UpdatePowerDomainConstraint(
    DWORD idContext
    )
{
    BOOL rc = TRUE;
    DWORD powerDomainOld = GetPowerDomainConstraint();

    // determine new power domain state
    PowerDomainConstraint::UpdatePowerDomainConstraint(idContext);

    // private mapping to power domain state
    if (powerDomainOld != GetPowerDomainConstraint())
        {
        rc = FlushDomainPowerState();

        NotifyDomainCallbacks(POWERDOMAIN_MPU, 
            GetPowerDomainConstraint(),
            idContext
            );
        }

    return rc;
}

//-----------------------------------------------------------------------------
// 
//  Function:  FlushDomainPowerState
//
//  flush the power domain state
//
BOOL
MpuPowerDomainConstraint::
FlushDomainPowerState(
    )
{
    BOOL rc;
    IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN domainPowerState;

    // initialize structure
    domainPowerState.size = sizeof(IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN);
    domainPowerState.powerDomain = POWERDOMAIN_MPU;
    
    switch (GetPowerDomainConstraint())
        {
        case D0:
        case D1:
            // set power domain to on
            domainPowerState.powerState = POWERSTATE_ON;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;
            
        case D2:
            // set close switch retention
            domainPowerState.powerState = POWERSTATE_RETENTION;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;

        case D3:
            // set open switch retention
            domainPowerState.powerState = POWERSTATE_RETENTION;
            domainPowerState.logicState = LOGICRETSTATE_LOGICOFF_DOMAINRET;
            break;

        case D4:
            // set power domain to off mode
            domainPowerState.powerState = POWERSTATE_OFF;
            domainPowerState.logicState = LOGICRETSTATE_LOGICOFF_DOMAINRET;
            break;
        }
    
    // send ioctl
    rc = KernelIoControl(IOCTL_PRCM_DOMAIN_SET_POWERSTATE, &domainPowerState, 
            sizeof(IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN), 0, 0, 0
            );

    return rc;
}

//-----------------------------------------------------------------------------
// 
//  Function:  UpdatePowerDomainConstraint
//
//  Updates the power state of a power domain
//
BOOL
IvaPowerDomainConstraint::
UpdatePowerDomainConstraint(
    DWORD idContext
    )
{
    BOOL rc = TRUE;
    DWORD powerDomainOld = GetPowerDomainConstraint();

    // determine new power domain state
    PowerDomainConstraint::UpdatePowerDomainConstraint(idContext);

    // private mapping to power domain state
    if (powerDomainOld != GetPowerDomainConstraint())
        {
        rc = FlushDomainPowerState();

        NotifyDomainCallbacks(POWERDOMAIN_IVA2, 
            GetPowerDomainConstraint(),
            idContext
            );
        }
                  
    return rc;
}

//-----------------------------------------------------------------------------
// 
//  Function:  FlushDomainPowerState
//
//  flush the power domain state
//
BOOL
IvaPowerDomainConstraint::
FlushDomainPowerState(
    )
{
    BOOL rc;
    IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN domainPowerState;

    // initialize structure
    domainPowerState.size = sizeof(IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN);
    domainPowerState.powerDomain = POWERDOMAIN_IVA2;
    
    switch (GetPowerDomainConstraint())
        {
        case D0:
        case D1:
            // set power domain to on
            domainPowerState.powerState = POWERSTATE_ON;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;
            
        case D2:
            // set close switch retention
            domainPowerState.powerState = POWERSTATE_RETENTION;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;

        case D3:
            // set open switch retention
            domainPowerState.powerState = POWERSTATE_RETENTION;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;

        case D4:
            // set power domain to off mode
            domainPowerState.powerState = POWERSTATE_OFF;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;
        }
    
    // send ioctl
    rc = KernelIoControl(IOCTL_PRCM_DOMAIN_SET_POWERSTATE, &domainPowerState, 
            sizeof(IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN), 0, 0, 0
            );

    return rc;
}

//-----------------------------------------------------------------------------
// 
//  Function:  UpdatePowerDomainConstraint
//
//  Updates the power state of a power domain
//
BOOL
CorePowerDomainConstraint::
UpdatePowerDomainConstraint(
    DWORD idContext
    )
{
    BOOL rc = TRUE;
    DWORD powerDomainOld = GetPowerDomainConstraint();

    // determine new power domain state
    PowerDomainConstraint::UpdatePowerDomainConstraint(idContext);

    // private mapping to power domain state
    if (powerDomainOld != GetPowerDomainConstraint())
        {
        rc = FlushDomainPowerState();

        NotifyDomainCallbacks(POWERDOMAIN_CORE, 
            GetPowerDomainConstraint(),
            idContext
            );
        }
                  
    return rc;
}

//-----------------------------------------------------------------------------
// 
//  Function:  FlushDomainPowerState
//
//  flush the power domain state
//
BOOL
CorePowerDomainConstraint::
FlushDomainPowerState(
    )
{   
    BOOL rc;
    IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN domainPowerState;

    // initialize structure
    domainPowerState.size = sizeof(IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN);
    domainPowerState.powerDomain = POWERDOMAIN_CORE;
    
    switch (GetPowerDomainConstraint())
        {
        case D0:
        case D1:
            // set power domain to on
            domainPowerState.powerState = POWERSTATE_ON;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;
            
        case D2:
            // set close switch retention
            domainPowerState.powerState = POWERSTATE_RETENTION;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;

        case D3:
            // set open switch retention
            domainPowerState.powerState = POWERSTATE_RETENTION;
            domainPowerState.logicState = LOGICRETSTATE_LOGICOFF_DOMAINRET;
            break;

        case D4:
            // set power domain to off mode
            domainPowerState.powerState = POWERSTATE_OFF;
            domainPowerState.logicState = LOGICRETSTATE_LOGICOFF_DOMAINRET;
            break;
        }
    
    // send ioctl
    rc = KernelIoControl(IOCTL_PRCM_DOMAIN_SET_POWERSTATE, &domainPowerState, 
            sizeof(IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN), 0, 0, 0
            );

    return rc;
}

//-----------------------------------------------------------------------------
// 
//  Function:  UpdatePowerDomainConstraint
//
//  Updates the power state of a power domain
//
BOOL
PeripheralPowerDomainConstraint::
UpdatePowerDomainConstraint(
    DWORD idContext
    )
{
    BOOL rc = TRUE;
    DWORD powerDomainOld = GetPowerDomainConstraint();

    // determine new power domain state
    PowerDomainConstraint::UpdatePowerDomainConstraint(idContext);

    // private mapping to power domain state
    if (powerDomainOld != GetPowerDomainConstraint())
        {
        rc = FlushDomainPowerState();

        NotifyDomainCallbacks(POWERDOMAIN_PERIPHERAL, 
            GetPowerDomainConstraint(),
            idContext
            );
        }
                  
    return rc;
}

//-----------------------------------------------------------------------------
// 
//  Function:  FlushDomainPowerState
//
//  flush the power domain state
//
BOOL
PeripheralPowerDomainConstraint::
FlushDomainPowerState(
    )
{
    BOOL rc;
    IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN domainPowerState;

    // initialize structure
    domainPowerState.size = sizeof(IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN);
    domainPowerState.powerDomain = POWERDOMAIN_PERIPHERAL;
    
    switch (GetPowerDomainConstraint())
        {
        case D0:
        case D1:
            // set power domain to on
            domainPowerState.powerState = POWERSTATE_ON;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;
            
        case D2:
        case D3:
            // set close switch retention
            domainPowerState.powerState = POWERSTATE_RETENTION;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;

        case D4:
            // set power domain to off mode
            domainPowerState.powerState = POWERSTATE_OFF;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;
        }
    
    // send ioctl
    rc = KernelIoControl(IOCTL_PRCM_DOMAIN_SET_POWERSTATE, &domainPowerState, 
            sizeof(IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN), 0, 0, 0
            );

    return rc;
}

//-----------------------------------------------------------------------------
// 
//  Function:  UpdatePowerDomainConstraint
//
//  Updates the power state of a power domain
//
BOOL
DssPowerDomainConstraint::
UpdatePowerDomainConstraint(
    DWORD idContext
    )
{
    BOOL rc = TRUE;
    DWORD powerDomainOld = GetPowerDomainConstraint();

    // determine new power domain state
    PowerDomainConstraint::UpdatePowerDomainConstraint(idContext);

    // private mapping to power domain state
    if (powerDomainOld != GetPowerDomainConstraint())
        {
        rc = FlushDomainPowerState();

        NotifyDomainCallbacks(POWERDOMAIN_DSS, 
            GetPowerDomainConstraint(),
            idContext
            );
        }

    return rc;
}

//-----------------------------------------------------------------------------
// 
//  Function:  FlushDomainPowerState
//
//  flush the power domain state
//
BOOL
DssPowerDomainConstraint::
FlushDomainPowerState(
    )
{
    BOOL rc;
    IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN domainPowerState;

    // initialize structure
    domainPowerState.size = sizeof(IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN);
    domainPowerState.powerDomain = POWERDOMAIN_DSS;
    
    switch (GetPowerDomainConstraint())
        {
        case D0:
        case D1:
            // set power domain to on
            domainPowerState.powerState = POWERSTATE_ON;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;
            
        case D2:
        case D3:
            // set close switch retention
            domainPowerState.powerState = POWERSTATE_RETENTION;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;

        case D4:
            // set power domain to off mode
            domainPowerState.powerState = POWERSTATE_OFF;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;
        }
    
    // send ioctl
    rc = KernelIoControl(IOCTL_PRCM_DOMAIN_SET_POWERSTATE, &domainPowerState, 
            sizeof(IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN), 0, 0, 0
            );

    return rc;
}

//-----------------------------------------------------------------------------
// 
//  Function:  UpdatePowerDomainConstraint
//
//  Updates the power state of a power domain
//
BOOL
SgxPowerDomainConstraint::
UpdatePowerDomainConstraint(
    DWORD idContext
    )
{
    BOOL rc = TRUE;
    DWORD powerDomainOld = GetPowerDomainConstraint();

    // determine new power domain state
    PowerDomainConstraint::UpdatePowerDomainConstraint(idContext);

    // private mapping to power domain state
    if (powerDomainOld != GetPowerDomainConstraint())
        {
        rc = FlushDomainPowerState();

        NotifyDomainCallbacks(POWERDOMAIN_SGX, 
            GetPowerDomainConstraint(),
            idContext
            );
        }
                  
    return rc;
}

//-----------------------------------------------------------------------------
// 
//  Function:  FlushDomainPowerState
//
//  flush the power domain state
//
BOOL
SgxPowerDomainConstraint::
FlushDomainPowerState(
    )
{
    BOOL rc;
    IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN domainPowerState;

    // initialize structure
    domainPowerState.size = sizeof(IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN);
    domainPowerState.powerDomain = POWERDOMAIN_SGX;
    
    switch (GetPowerDomainConstraint())
        {
        case D0:
        case D1:
            // set power domain to on
            domainPowerState.powerState = POWERSTATE_ON;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;
            
        case D2:
        case D3:
            // currently SGX doesn't support retention
            domainPowerState.powerState = POWERSTATE_ON;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;

        case D4:
            // set power domain to off mode
            domainPowerState.powerState = POWERSTATE_OFF;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;
        }
    
    // send ioctl
    rc = KernelIoControl(IOCTL_PRCM_DOMAIN_SET_POWERSTATE, &domainPowerState, 
            sizeof(IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN), 0, 0, 0
            );

    return rc;
}

//-----------------------------------------------------------------------------
// 
//  Function:  UpdatePowerDomainConstraint
//
//  Updates the power state of a power domain
//
BOOL
CameraPowerDomainConstraint::
UpdatePowerDomainConstraint(
    DWORD idContext
    )
{
    BOOL rc = TRUE;
    DWORD powerDomainOld = GetPowerDomainConstraint();
    
    // determine new power domain state
    PowerDomainConstraint::UpdatePowerDomainConstraint(idContext);

    // private mapping to power domain state
    if (powerDomainOld != GetPowerDomainConstraint())
        {
        rc = FlushDomainPowerState();
        
        NotifyDomainCallbacks(POWERDOMAIN_CAMERA, 
            GetPowerDomainConstraint(),
            idContext
            );                      
        }
   
    return rc;
}

//-----------------------------------------------------------------------------
// 
//  Function:  FlushDomainPowerState
//
//  flush the power domain state
//
BOOL
CameraPowerDomainConstraint::
FlushDomainPowerState(
    )
{
    BOOL rc;
    IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN domainPowerState;

    // initialize structure
    domainPowerState.size = sizeof(IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN);
    domainPowerState.powerDomain = POWERDOMAIN_CAMERA;
    
    switch (GetPowerDomainConstraint())
        {
        case D0:
        case D1:
            // set power domain to on
            domainPowerState.powerState = POWERSTATE_ON;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;
            
        case D2:
        case D3:
            // camera only supports CSWR
            domainPowerState.powerState = POWERSTATE_RETENTION;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;

        case D4:
            // set power domain to off mode
            domainPowerState.powerState = POWERSTATE_OFF;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;
        }
    
    // send ioctl
    rc = KernelIoControl(IOCTL_PRCM_DOMAIN_SET_POWERSTATE, &domainPowerState, 
            sizeof(IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN), 0, 0, 0
            );

    return rc;
}

//-----------------------------------------------------------------------------
// 
//  Function:  UpdatePowerDomainConstraint
//
//  Updates the power state of a power domain
//
BOOL
UsbHostPowerDomainConstraint::
UpdatePowerDomainConstraint(
    DWORD idContext
    )
{
    BOOL rc = TRUE;
    DWORD powerDomainOld = GetPowerDomainConstraint();

    // determine new power domain state
    PowerDomainConstraint::UpdatePowerDomainConstraint(idContext);

    // private mapping to power domain state
    if (powerDomainOld != GetPowerDomainConstraint())
        {
        rc = FlushDomainPowerState();

        NotifyDomainCallbacks(POWERDOMAIN_USBHOST, 
            GetPowerDomainConstraint(),
            idContext
            );
        }
                  
    return rc;
}

//-----------------------------------------------------------------------------
// 
//  Function:  FlushDomainPowerState
//
//  flush the power domain state
//
BOOL
UsbHostPowerDomainConstraint::
FlushDomainPowerState(
    )
{
    BOOL rc;
    IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN domainPowerState;

    // initialize structure
    domainPowerState.size = sizeof(IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN);
    domainPowerState.powerDomain = POWERDOMAIN_USBHOST;
    
    switch (GetPowerDomainConstraint())
        {
        case D0:
        case D1:
            // set power domain to on
            domainPowerState.powerState = POWERSTATE_ON;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;
            
        case D2:
        case D3:
            // usbhost only supports CSWR
            domainPowerState.powerState = POWERSTATE_RETENTION;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;

        case D4:
            // set power domain to off mode
            domainPowerState.powerState = POWERSTATE_OFF;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;
        }
    
    // send ioctl
    rc = KernelIoControl(IOCTL_PRCM_DOMAIN_SET_POWERSTATE, &domainPowerState, 
            sizeof(IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN), 0, 0, 0
            );

    return rc;
}

//-----------------------------------------------------------------------------
// 
//  Function:  UpdatePowerDomainConstraint
//
//  Updates the power state of a power domain
//
BOOL
NeonPowerDomainConstraint::
UpdatePowerDomainConstraint(
    DWORD idContext
    )
{
    BOOL rc = TRUE;
    DWORD powerDomainOld = GetPowerDomainConstraint();

    // determine new power domain state
    PowerDomainConstraint::UpdatePowerDomainConstraint(idContext);

    // private mapping to power domain state
    if (powerDomainOld != GetPowerDomainConstraint())
        {
        rc = FlushDomainPowerState();

        NotifyDomainCallbacks(POWERDOMAIN_NEON, 
            GetPowerDomainConstraint(),
            idContext
            );
        }
                  
    return rc;
}

//-----------------------------------------------------------------------------
// 
//  Function:  FlushDomainPowerState
//
//  flush the power domain state
//
BOOL
NeonPowerDomainConstraint::
FlushDomainPowerState(
    )
{
    BOOL rc;
    IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN domainPowerState;

    // initialize structure
    domainPowerState.size = sizeof(IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN);
    domainPowerState.powerDomain = POWERDOMAIN_NEON;
    
    switch (GetPowerDomainConstraint())
        {
        case D0:
        case D1:
            // set power domain to on
            domainPowerState.powerState = POWERSTATE_ON;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;
            
        case D2:
        case D3:
            // currently only supports CSWR
            domainPowerState.powerState = POWERSTATE_RETENTION;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;

        case D4:
            // set power domain to off mode
            domainPowerState.powerState = POWERSTATE_OFF;
            domainPowerState.logicState = LOGICRETSTATE_LOGICRET_DOMAINRET;
            break;
        }
    
    // send ioctl
    rc = KernelIoControl(IOCTL_PRCM_DOMAIN_SET_POWERSTATE, &domainPowerState, 
            sizeof(IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN), 0, 0, 0
            );

    return rc;
}

//------------------------------------------------------------------------------
