//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//

//
// This module contains routines for keeping track of devices and 
// managing device power.
//

#include <pmimpl.h>

// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)

// This routine determines how to map a device power state that the
// PM wants to use into the device power states that a particular
// device actually supports.
CEDEVICE_POWER_STATE
MapDevicePowerState(CEDEVICE_POWER_STATE newDx, UCHAR ucSupportedDx)
{
    CEDEVICE_POWER_STATE reqDx;
    SETFNAME(_T("MapDevicePowerState"));

    DEBUGCHK(newDx >= D0 && newDx <= D4);

    // map the power level to whatever the device actually supports.  All devices
    // are required to support D0 so we won't even check for it.
    reqDx = newDx;
    if(reqDx == D3 && (ucSupportedDx & (1 << reqDx)) == 0) reqDx = D4;
    if(reqDx == D4) {
        if((ucSupportedDx & (1 << reqDx)) == 0) reqDx = D3;
        if((ucSupportedDx & (1 << reqDx)) == 0) reqDx = D2;
    }
    if(reqDx == D2 && (ucSupportedDx & (1 << reqDx)) == 0) reqDx = D1;
    if(reqDx == D1 && (ucSupportedDx & (1 << reqDx)) == 0) reqDx = D0;

    PMLOGMSG(ZONE_DEVICE, (_T("%s: mapping D%d to D%d\r\n"), pszFname, newDx, reqDx));
    return reqDx;
}
    

// This routine actually tells a device to update its current power state.  It 
// returns TRUE if successful, FALSE otherwise.  Note that devices don't always
// update their power state to the level that the PM wants.  Some devices may
// not support all power states, for example.  Devices are responsible for mapping
// PM requests to device power levels that they actually support.
DWORD
SetDevicePower(PDEVICE_STATE pds, CEDEVICE_POWER_STATE newDx, BOOL fForceSet = FALSE)
{
    DWORD dwStatus = ERROR_SUCCESS;
    static DWORD dwStaticRefCount = 0; 
    DWORD dwCurRefCount = 0;
    CEDEVICE_POWER_STATE reqDx;
    CEDEVICE_POWER_STATE oldActualDx, oldCurDx;
    POWER_RELATIONSHIP pr;
    PPOWER_RELATIONSHIP ppr = NULL;
    BOOL fDoSet;
    BOOL fOpenDevice = FALSE;
    HANDLE hDevice = INVALID_HANDLE_VALUE;
    SETFNAME(_T("SetDevicePower"));

    PREFAST_DEBUGCHK(pds != NULL);
    DEBUGCHK( newDx >= D0 && newDx <= D4);
    DEBUGCHK(pds->pInterface != NULL);

    // map the power level to whatever the device actually supports
    reqDx = MapDevicePowerState(newDx, pds->caps.DeviceDx);
    PMLOGMSG(ZONE_DEVICE, (_T("%s: setting '%s' to D%d (mapped to D%d), fForceSet is %d\r\n"), 
        pszFname, pds->pszName, newDx, reqDx, fForceSet));

    // make a last check to see if we really need to send the device an update
    PMLOCK();
    DEBUGCHK(pds->dwNumPending == 0 || pds->pendingDx != PwrDeviceUnspecified);
    DEBUGCHK(pds->dwNumPending != 0 || pds->pendingDx == PwrDeviceUnspecified);
    if(reqDx != pds->actualDx || pds->dwNumPending != 0 || fForceSet) {
        fDoSet = TRUE;

        // record what we're trying to set, visible to other threads
        pds->pendingDx = reqDx;
        pds->dwNumPending++;
        // Re-Enter Checking Count.
        dwStaticRefCount ++ ;
        dwCurRefCount = dwStaticRefCount;
        // remember what we're trying to set in this thread
        oldCurDx = pds->curDx;
        oldActualDx = pds->actualDx;

        // get a handle to the device to make the request
        hDevice = pds->hDevice;
        if(hDevice != INVALID_HANDLE_VALUE) {
            fOpenDevice = FALSE;    // already have a handle
        } else {
            fOpenDevice = TRUE;     // need to open a handle
        }
    } else {
        fDoSet = FALSE;
    }
    PMUNLOCK();

    // are we doing an update?
    if(!fDoSet) {
        PMLOGMSG(ZONE_DEVICE, (_T("%s: device is already at D%d\r\n"), pszFname, reqDx));
    } else {
        // initialize parameters
        memset(&pr, 0, sizeof(pr));
        if(pds->pParent != NULL) {
            PMLOGMSG(ZONE_DEVICE, (_T("%s: parent of '%s' is '%s'\r\n"), 
                pszFname, pds->pszName, pds->pParent->pszName));
            pr.hParent = (HANDLE) pds->pParent;
            pr.pwsParent = pds->pParent->pszName;
            pr.hChild = (HANDLE) pds;
            pr.pwsChild = pds->pszName;
            ppr = &pr;
        }
        
        // get a handle to the device
        if(fOpenDevice) {
            DEBUGCHK(pds->pInterface->pfnOpenDevice != NULL);
            hDevice = pds->pInterface->pfnOpenDevice(pds);
        }

        // did we get a handle?
        if(hDevice == INVALID_HANDLE_VALUE) {
            PMLOGMSG(ZONE_WARN, (_T("%s: couldn't open '%s'\r\n"), pszFname,
                pds->pszName));
            dwStatus = ERROR_INVALID_HANDLE;
        } else {
            DWORD dwBytesReturned;
            DEBUGCHK(pds->pInterface->pfnRequestDevice != NULL);

            BOOL fOk = pds->pInterface->pfnRequestDevice(hDevice, IOCTL_POWER_SET, ppr, 
                ppr == NULL ? 0 : sizeof(*ppr), &reqDx, sizeof(reqDx), 
                &dwBytesReturned);

            if(fOk) {
                // Check for races to update the driver -- it is possible for the device to call
                // DevicePowerNotify() when another thread is calling SetDevicePower(), 
                // SetSystemPowerState(), or Set(/Release)PowerRequirement().
                PMLOCK();
                if(pds->pendingDx == reqDx) {
                    // record the new values
                    pds->curDx = newDx;
                    pds->actualDx = reqDx;
                } 
                else if (dwCurRefCount != dwStaticRefCount || pds->dwNumPending > 1 ) {
                    PMLOGMSG(ZONE_DEVICE, (_T("%s: race detected on '%s', returning ERROR_RETRY\r\n"),
                        pszFname, pds->pszName));
                    dwStatus = ERROR_RETRY;
                }
                else { // This condition indicate no SetDevicePower called by others. It must be wrong value return from device driver
                    RETAILMSG(1, (_T("%s: Wrong Return Value from '%s', returning ERROR_GEN_FAILURE\r\n"),
                        pszFname, pds->pszName));
                    ASSERT(FALSE);
                    dwStatus = ERROR_GEN_FAILURE;
                }
                dwStaticRefCount ++ ;
                PMUNLOCK();
            } else {
                dwStatus = GetLastError();
                if(dwStatus == ERROR_SUCCESS) {
                    dwStatus = ERROR_GEN_FAILURE;
                }
                PMLOGMSG(ZONE_WARN, (_T("%s: '%s' failed IOCTL_POWER_SET D%d, status is %d\r\n"),
                    pszFname, pds->pszName, newDx, dwStatus));
                
                PMLOCK();
                // the set operation failed -- if the device power state appears unchanged,
                // restore our state variables.  In general, devices should never fail a
                // set request.
                if(reqDx == oldActualDx) {
                    pds->curDx = oldCurDx;
                    pds->actualDx = oldActualDx;
                }
                PMUNLOCK();
            }
            
            // close the device handle if we opened it in this routine
            if(fOpenDevice) {
                DEBUGCHK(pds->pInterface->pfnCloseDevice != NULL);
                pds->pInterface->pfnCloseDevice(hDevice);
            }
        }

        // update reference counts
        PMLOCK();
        DEBUGCHK(pds->dwNumPending != 0);
        DEBUGCHK(pds->pendingDx != PwrDeviceUnspecified);
        pds->dwNumPending--;
        if(pds->dwNumPending == 0) {
            pds->pendingDx = PwrDeviceUnspecified;
        }
        PMUNLOCK();
    }

    return dwStatus;
}

// This routine sends an IOCTL_POWER_GET to a device to obtain its current
// device power state.  It returns ERROR_SUCCESS and fills in pCurDx if 
// successful.  Otherwise it returns an error code.
DWORD
GetDevicePower(PDEVICE_STATE pds, PCEDEVICE_POWER_STATE pCurDx)
{
    DWORD dwStatus = ERROR_GEN_FAILURE;
    POWER_RELATIONSHIP pr;
    PPOWER_RELATIONSHIP ppr = NULL;
    BOOL fOpenDevice;
    HANDLE hDevice;
    SETFNAME(_T("GetDevicePower"));

    PREFAST_DEBUGCHK(pds != NULL );
    PREFAST_DEBUGCHK(pCurDx != NULL);
    PREFAST_DEBUGCHK(pds->pInterface != NULL);

    // initialize parameters
    memset(&pr, 0, sizeof(pr));
    if(pds->pParent != NULL) {
        PMLOGMSG(ZONE_DEVICE, (_T("%s: parent of '%s' is '%s'\r\n"), 
            pszFname, pds->pszName, pds->pParent->pszName));
        pr.hParent = (HANDLE) pds->pParent;
        pr.pwsParent = pds->pParent->pszName;
        pr.hChild = (HANDLE) pds;
        pr.pwsChild = pds->pszName;
        ppr = &pr;
    }
    
    // get a handle to the device
    hDevice = pds->hDevice;
    if(hDevice != INVALID_HANDLE_VALUE) {
        fOpenDevice = FALSE;    // already have a handle
    } else {
        fOpenDevice = TRUE;     // need to open a handle
        DEBUGCHK(pds->pInterface->pfnOpenDevice != NULL);
        hDevice = pds->pInterface->pfnOpenDevice(pds);
    }

    // did we get a handle?
    if(hDevice == INVALID_HANDLE_VALUE) {
        PMLOGMSG(ZONE_WARN, (_T("%s: couldn't open '%s'\r\n"), pszFname,
            pds->pszName));
    } else {
        CEDEVICE_POWER_STATE tmpDx = PwrDeviceUnspecified;
        DWORD dwBytesReturned;
        DEBUGCHK(pds->pInterface->pfnRequestDevice != NULL);
        BOOL fOk = pds->pInterface->pfnRequestDevice(hDevice, IOCTL_POWER_GET, ppr, 
            ppr == NULL ? 0 : sizeof(*ppr), &tmpDx, sizeof(tmpDx), 
            &dwBytesReturned);
        
        // update variables and clear the requestor thread ID
        if(fOk) {
            if(VALID_DX(tmpDx)) {
                *pCurDx = tmpDx;
                dwStatus = ERROR_SUCCESS;
            } else {
                PMLOGMSG(ZONE_WARN, 
                    (_T("%s: '%s' IOCTL_POWER_GET returned invalid state %u\r\n"),
                    pszFname, pds->pszName, tmpDx));
                dwStatus = ERROR_INVALID_DATA;
            }
        } else {
            dwStatus = GetLastError();
        }
        
        PMLOGMSG(!fOk && ZONE_WARN, 
            (_T("%s: '%s' failed IOCTL_POWER_GET, status is %d\r\n"),
            pszFname, pds->pszName, GetLastError()));
        
        // close the device handle if we opened it in this routine
        if(fOpenDevice) {
            DEBUGCHK(pds->pInterface->pfnCloseDevice != NULL);
            pds->pInterface->pfnCloseDevice(hDevice);
        }
    }

    return dwStatus;
}

// This routine determines the mapping from a requested device power level
// to an actual new device power level.  This is based on whether or not
// a device power level has been set by the user and what the current 
// floor and ceiling device power levels are.  This routine returns the new
// device power level, or PwrDeviceUnspecified if the device is already
// at the desired power level.
CEDEVICE_POWER_STATE
GetNewDeviceDx(CEDEVICE_POWER_STATE reqDx, 
               CEDEVICE_POWER_STATE curDx, 
               CEDEVICE_POWER_STATE setDx, 
               CEDEVICE_POWER_STATE floorDx, 
               CEDEVICE_POWER_STATE ceilingDx)
{
    CEDEVICE_POWER_STATE newDx = reqDx;

    DEBUGCHK(reqDx != PwrDeviceUnspecified);
    DEBUGCHK(VALID_DX(reqDx));

    // determine which power state state to put the device into as a result
    // of this update.  The device may already be in a valid state, in which
    // case we don't need to do anything.
    if(setDx != PwrDeviceUnspecified) {
        newDx = setDx;
    } else {
        // no explicitly set power level, compare current values against
        // boundaries
        newDx = reqDx;                  // assume this setting is ok
        if(newDx < ceilingDx) {
            // lower power to the level of the ceiling
            newDx = ceilingDx;
        }
        if(floorDx < newDx) {
            // raise power to the level of the floor
            newDx = floorDx;
        }
    }

    // return no change if the new state is unchanged or is invalid
    // (for example, if DevicePowerNotify() while processing 
    // IOCTL_POWER_CAPABILITIES)
    if(curDx == newDx || ! VALID_DX(newDx)) {
        newDx = PwrDeviceUnspecified;
    }

    return newDx;
}

// This routine examines a device's current power state information with
// regards to system power state and floor and ceiling restrictions.  It
// returns TRUE if successful and FALSE otherwise.  If successful, new 
// values for the device's current, floor, and ceiling power states are
// passed back via pointers.  If the caller is not interested in the
// new values for floor and ceiling power levels, it may pass in null
// for these parameters.  The caller should hold the PM lock.
BOOL
GetNewDeviceStateInfo(PCEDEVICE_POWER_STATE pNewFloorDx, 
                      PCEDEVICE_POWER_STATE pNewCeilingDx,
                      PDEVICE_STATE pds, PSYSTEM_POWER_STATE psps, 
                      PDEVICE_POWER_RESTRICTION pFloorDxList,
                      PDEVICE_POWER_RESTRICTION pCeilingDxList)
{
    BOOL fOk = TRUE;
    LPCGUID pGuid = pds->pListHead->pGuid;
    DEVICEID devId;
    PDEVICE_POWER_RESTRICTION pdpr;
    CEDEVICE_POWER_STATE newFloorDx, newCeilingDx;
    SETFNAME(_T("GetNewDeviceStateInfo"));

    PREFAST_DEBUGCHK(psps != NULL);
    PREFAST_DEBUGCHK(pNewFloorDx != NULL);
    PREFAST_DEBUGCHK(pNewCeilingDx != NULL);

    // Assume the default ceiling state.  Since power ceilings are defined in
    // the registry, there should be at most one that matches the device's 
    // class and one that matches the device exactly.
    newCeilingDx = psps->defaultCeilingDx;

    // look for power restrictions that match this device's class
    devId.pGuid = pds->pListHead->pGuid;
    devId.pszName = NULL;
    if((pdpr = PowerRestrictionFindList(pCeilingDxList, &devId, NULL)) != NULL) {
        newCeilingDx = pdpr->devDx;
    }

    // look for power restrictions that match this device exactly
    devId.pszName = pds->pszName;
    if((pdpr = PowerRestrictionFindList(pCeilingDxList, &devId, NULL)) != NULL) {
        newCeilingDx = pdpr->devDx;
    }

    // Assume the default floor (device turned off).  There may be multiple
    // power floors specified in the registry so we will have to look for
    // matches of several different types.
    newFloorDx = D4;

    // are we ignoring application requirements in this system power state?
    if((psps->dwFlags & (POWER_STATE_CRITICAL | POWER_STATE_OFF)) == 0) {
        BOOL fNeedForce;

        // is the default for this power state to ignore application requirements?
        if((psps->dwFlags & POWER_STATE_SUSPEND) != 0) {
            fNeedForce = TRUE;
        } else {
            fNeedForce = FALSE;
        }

        // no, look for the least restricted floor based on device class for any
        // system power state
        devId.pszName = NULL;
        pdpr = pFloorDxList;
        while((pdpr = PowerRestrictionFindList(pdpr, &devId, NULL)) != NULL) {
            if(pdpr->devDx < newFloorDx
            && (fNeedForce == FALSE || (pdpr->dwFlags & POWER_FORCE) != 0)) {
                newFloorDx = pdpr->devDx;
            }
            pdpr = pdpr->pNext;
        }
        
        // look for the least restricted floor based on device class for
        // this system power state
        devId.pszName = NULL;
        pdpr = pFloorDxList;
        while((pdpr = PowerRestrictionFindList(pdpr, &devId, psps->pszName)) 
            != NULL) {
            if(pdpr->devDx < newFloorDx
            && (fNeedForce == FALSE || (pdpr->dwFlags & POWER_FORCE) != 0)) {
                newFloorDx = pdpr->devDx;
            }
            pdpr = pdpr->pNext;
        }
        
        // look for the least restricted floor based on an exact 
        // match with this device for any system power state
        devId.pszName = pds->pszName;
        pdpr = pFloorDxList;
        while((pdpr = PowerRestrictionFindList(pdpr, &devId, NULL)) != NULL) {
            if(pdpr->devDx < newFloorDx
            && (fNeedForce == FALSE || (pdpr->dwFlags & POWER_FORCE) != 0)) {
                newFloorDx = pdpr->devDx;
            }
            pdpr = pdpr->pNext;
        }
        
        // look for the least restricted floor based on an exact 
        // match with this device for this system power state
        devId.pszName = pds->pszName;
        pdpr = pFloorDxList;
        while((pdpr = PowerRestrictionFindList(pdpr, &devId, psps->pszName)) 
            != NULL) {
            if(pdpr->devDx < newFloorDx
            && (fNeedForce == FALSE || (pdpr->dwFlags & POWER_FORCE) != 0)) {
                newFloorDx = pdpr->devDx;
            }
            pdpr = pdpr->pNext;
        }
    }

    // pass back values
    if(fOk) {
        *pNewCeilingDx = newCeilingDx;
        *pNewFloorDx = newFloorDx;
    }

    return fOk;
}
// This routine updates a device's power state variables.  This routine
// should be called whenever a device is added to the system or whenever
// a device power restriction is created, modified, or destroyed.
BOOL
UpdateDeviceState(PDEVICE_STATE pds)
{
    BOOL fOk = TRUE;
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwRetryCount = 0 ;
    SETFNAME(_T("UpdateDeviceState"));

    do {
        CEDEVICE_POWER_STATE newDx = PwrDeviceUnspecified;
        CEDEVICE_POWER_STATE oldLastReqDx;
        BOOL fForce = FALSE;
        
        PMLOCK();
        // get the new device power state (based on the last device power state
        // the device wanted -- this value is initialized to D0 in case the 
        // device never makes any power requests on its own.
        fOk = GetNewDeviceStateInfo(&pds->floorDx, &pds->ceilingDx,
            pds, gpSystemPowerState, gpFloorDx, gpCeilingDx);
        if(fOk) {
            // We have to monitor changes to pds->lastReqDx, since multiple threads can be 
            // in the PM at once calling DevicePowerNotify().
            newDx = GetNewDeviceDx(pds->lastReqDx, pds->curDx, pds->setDx, pds->floorDx, pds->ceilingDx);
            if(newDx == PwrDeviceUnspecified && (pds->dwNumPending != 0 || dwStatus == ERROR_RETRY)) {
                RETAILMSG(0, (_T("%s: reentrant set for '%s', setting %d\r\n"),
					pszFname, pds->pszName, pds->curDx));
                newDx = pds->curDx;
                fForce = TRUE;
            }
            RETAILMSG(1, 
                (_T("%s: new state for '%s' is %d (current %d, req %d, set %d, floor %d, ceiling %d, actual %d)\r\n"),
                L"UpdateDeviceState", pds->pszName, newDx, pds->curDx, pds->lastReqDx, pds->setDx, pds->floorDx, pds->ceilingDx, pds->actualDx));
            oldLastReqDx = pds->lastReqDx;
            dwStatus = ERROR_SUCCESS;
        }
        PMUNLOCK();

        // do we need to update the device?
        if(fOk && newDx != PwrDeviceUnspecified) {
            // yes, set its power state to the new value
            dwStatus = SetDevicePower(pds, newDx, fForce);
            if(dwStatus == ERROR_SUCCESS) {
                PMLOCK();
                RETAILMSG(0, 
                    (_T("%s: updated state for '%s' is current %d, req %d, set %d, floor %d, ceiling %d, actual %d\r\n"),
                    pszFname, pds->pszName, pds->curDx, pds->lastReqDx, pds->setDx, pds->floorDx, pds->ceilingDx, pds->actualDx));
                if(pds->lastReqDx != oldLastReqDx) {
                    RETAILMSG(0, (_T("%s: race detected on '%s', setting ERROR_RETRY\r\n"),
                        pszFname, pds->pszName));
                    dwStatus = ERROR_RETRY;
                }
                PMUNLOCK();
            }
            
            if(dwStatus == ERROR_RETRY) {
                dwRetryCount++;
                RETAILMSG(0, (_T("%s: retrying(%d) SetDevicePower('%s', D%d)\r\n"), 
                    pszFname,dwRetryCount, pds->pszName, newDx));
            } else if(dwStatus != ERROR_SUCCESS) {
                RETAILMSG(0, (_T("%s: SetDevicePower('%s', D%d) failed %d\r\n"), 
                    pszFname, pds->pszName, newDx, dwStatus));
                fOk = FALSE;
            }
        }
    } while(fOk && dwStatus == ERROR_RETRY);

    return fOk;
}

#ifdef PM_SUPPORTS_DEVICE_QUERIES

// NOTE -- Devices are not required to implement IOCTL_POWER_QUERY, nor is the 
// PM required to pay attention to the value that devices return in response to the
// query.  If the POWER_FORCE flag is set, the PM is not required to make the request.
// This means that device drivers cannot count on IOCTL_POWER_QUERY being invoked
// at all, so by default, the PM does not use IOCTL_POWER_QUERY at all.  
//
// However, some OEMs may choose to query drivers on every system power state
// transition and to always honor the device's response.  Such OEMs may 
// find the code inside this #ifdef useful.  This code may not be present
// in future source updates to the Power Manager.

// This routine queries a driver as to whether it is ready to handle a device
// power state transition.  It returns TRUE or FALSE according to the response;
// FALSE may also indicate an error talking to the device.
BOOL
QueryDevicePowerUpdate(PDEVICE_STATE pds, CEDEVICE_POWER_STATE newDx)
{
    BOOL fOk = TRUE;
    POWER_RELATIONSHIP pr;
    PPOWER_RELATIONSHIP ppr = NULL;
    CEDEVICE_POWER_STATE reqDx;
    BOOL fDoSet;
    BOOL fOpenDevice = FALSE;
    HANDLE hDevice = INVALID_HANDLE_VALUE;
    SETFNAME(_T("QueryDevicePower"));
    
    DEBUGCHK(pds != NULL && newDx >= D0 && newDx <= D4);
    DEBUGCHK(pds->pInterface != NULL);

    // map the power level to whatever the device actually supports
    reqDx = MapDevicePowerState(newDx, pds->caps.DeviceDx);
    PMLOGMSG(ZONE_DEVICE, (_T("%s: setting '%s' to D%d (mapped to D%d)\r\n"), 
        pszFname, pds->pszName, newDx, reqDx));

    // make a last check to see if we really need to send the device an update
    PMLOCK();
    if(reqDx != pds->actualDx) {
        fDoSet = TRUE;
        hDevice = pds->hDevice;
        if(hDevice != INVALID_HANDLE_VALUE) {
            fOpenDevice = FALSE;    // already have a handle
        } else {
            fOpenDevice = TRUE;     // need to open a handle
        }
    } else {
        fDoSet = FALSE;
    }
    PMUNLOCK();

    // are we doing an update?
    if(!fDoSet) {
        PMLOGMSG(ZONE_DEVICE, (_T("%s: device is already at D%d\r\n"), pszFname,
            reqDx));
    } else {
        // initialize parameters
        memset(&pr, 0, sizeof(pr));
        if(pds->pParent != NULL) {
            PMLOGMSG(ZONE_DEVICE, (_T("%s: parent of '%s' is '%s'\r\n"), 
                pszFname, pds->pszName, pds->pParent->pszName));
            pr.hParent = (HANDLE) pds->pParent;
            pr.pwsParent = pds->pParent->pszName;
            pr.hChild = (HANDLE) pds;
            pr.pwsChild = pds->pszName;
            ppr = &pr;
        }

        // get a handle to the device
        if(fOpenDevice) {
            DEBUGCHK(pds->pInterface->pfnOpenDevice != NULL);
            hDevice = pds->pInterface->pfnOpenDevice(pds);
        }

        // do we have one?
        if(hDevice == INVALID_HANDLE_VALUE) {
            PMLOGMSG(ZONE_WARN, (_T("%s: couldn't open '%s'\r\n"), pszFname,
                pds->pszName));
            fOk = FALSE;
        } else {
            CEDEVICE_POWER_STATE tmpDx = newDx;
            DWORD dwBytesReturned;
            DEBUGCHK(pds->pInterface->pfnRequestDevice != NULL);
            fOk = pds->pInterface->RequestDevice(hDevice, IOCTL_POWER_QUERY, ppr, 
                ppr == NULL ? 0 : sizeof(*ppr), &tmpDx, sizeof(tmpDx), 
                &dwBytesReturned);
            if(!fOk) {
                PMLOGMSG(ZONE_WARN, 
                    (_T("%s: '%s' failed IOCTL_POWER_QUERY D%d, status is %d\r\n"),
                    pszFname, pds->pszName, newDx, GetLastError()));
            }
            
            // close the device handle if we opened it in this routine
            if(fOpenDevice) {
                DEBUGCHK(pds->pInterface->pfnCloseDevice != NULL);
                pds->pInterface->pfnCloseDevice(hDevice);
            }
            
            // if the device permitted the ioctl, determine its response to the 
            // query.
            if(fOk) {
                if(tmpDx == PwrDeviceUnspecified) {
                    PMLOGMSG(ZONE_WARN, (_T("%s: '%s' rejected IOCTL_POWER_QUERY D%d\r\n"),
                        pszFname, pds->pszName, newDx));
                    fOk = FALSE;
                }
            }
        }
    }

    return fOk;
}

// This routine determines what power state a device would enter in a new
// system power state and asks the device whether it is ready to enter
// that state.  It returns TRUE or FALSE depending on the device's response;
// false may also indicate an error.    
BOOL
QueryDeviceStateUpdate(PDEVICE_STATE pds, PSYSTEM_POWER_STATE psps, 
                       PDEVICE_POWER_RESTRICTION pFloorDx, 
                       PDEVICE_POWER_RESTRICTION pCeilingDx)
{
    BOOL fOk = TRUE;
    CEDEVICE_POWER_STATE floorDx, ceilingDx;
    CEDEVICE_POWER_STATE newDx = PwrDeviceUnspecified;
    SETFNAME(_T("QueryDeviceState"));

    PMLOCK();
    // get the new device power state (based on the last device power state
    // the device wanted -- this value is initialized to D0 in case the 
    // device never makes any power requests on its own.
    fOk = GetNewDeviceStateInfo(&floorDx, &ceilingDx, pds, psps, pFloorDx, pCeilingDx);
    if(fOk) {
        newDx = GetNewDeviceDx(pds->lastReqDx, pds->curDx, pds->setDx, floorDx, ceilingDx);
        PMLOGMSG(ZONE_DEVICE, 
            (_T("%s: new state for '%s' is %d (current %d, req %d, set %d, floor %d, ceiling %d, actual %d)\r\n"),
            pszFname, pds->pszName, newDx, pds->curDx, pds->lastReqDx, pds->setDx, pds->floorDx, pds->ceilingDx, pds->actualDx));
    }
    PMUNLOCK();

    // do we need to update the device?
    if(fOk && newDx != PwrDeviceUnspecified) {
        // yes, see if the device is able to update its power state to the new value
        fOk = QueryDevicePowerUpdate(pds, newDx);
    }

    return fOk;
}

// This routine asks all registered devices if they are ready to make device
// power state transitions to a new system power state.  It returns TRUE if all
// responses are positive, FALSE otherwise.
BOOL 
QueryClassDevices(PDEVICE_LIST pdl, PSYSTEM_POWER_STATE psps, PDEVICE_POWER_RESTRICTION pdprCeiling,
                     PDEVICE_POWER_RESTRICTION pdprFloor)
{
    BOOL fOk = TRUE;
    BOOL fDeviceRemoved;
    PDEVICE_STATE pds;
    SETFNAME(_T("QueryClassDevices"));

    DEBUGCHK(pdl != NULL);

    PMLOCK();

    // Since it's possible that the device list may be modified
    // while we are not holding the PM critical section we need
    // to watch out for the device we are working on getting removed.
    // We increment the reference count to keep the device data 
    // structure from being deallocated out from under us.  Once
    // the device update is complete and we have the critical section
    // again, we check to see if it's still part of the device list.
    // If it's not, we start again at the top of the list.
    do {
        fDeviceRemoved = FALSE;
        pds = pdl->pList;
        while(fOk && !fDeviceRemoved && pds != NULL) {
            PDEVICE_STATE pdsNext;
            DeviceStateAddRef(pds);     // keep the device pointer alive
            
            PMUNLOCK();
            fOk = QueryDeviceStateUpdate(pds, psps, pdprFloor, pdprCeiling);
            PMLOCK();
            
            // is the device still on the list?
            if(pds->pListHead == NULL) {
                // device disappeared while we were accessing it
                PMLOGMSG(ZONE_WARN || ZONE_DEVICE, 
                    (_T("%s: device '%s' removed during update\r\n"), pszFname,
                    pds->pszName));
                fDeviceRemoved = TRUE;
            } else {
                // save the next pointer so we don't try to
                // dereference pds after decrementing its use count.
                pdsNext = pds->pNext;
            }
            DeviceStateDecRef(pds);     // done with the device pointer
            if(!fDeviceRemoved) pds = pdsNext;  // on to the next
        }
    } while(fOk && (fDeviceRemoved || pds != NULL));

    PMUNLOCK();

    PMLOGMSG(!fOk && ZONE_WARN, (_T("%s: returning %d\r\n"), pszFname, fOk));
    return fOk;
}

// This routine asks all registered devices if they are ready to make device
// power state transitions to a new system power state.  It returns TRUE if all
// responses are positive, FALSE otherwise.
BOOL 
QueryAllDevices(PSYSTEM_POWER_STATE psps, PDEVICE_POWER_RESTRICTION pdprCeiling,
                     PDEVICE_POWER_RESTRICTION pdprFloor)
{
    PDEVICE_LIST pdl;
    BOOL fOk = TRUE;
    SETFNAME(_T("QueryAllDevices"));

    // update all devices of all classes
    for(pdl = gpDeviceLists; fOk && pdl != NULL; pdl = pdl->pNext) {
        fOk = QueryClassDevices(pdl, psps, pdprCeiling, pdprFloor);
    }

    PMLOGMSG(!fOk && ZONE_WARN, (_T("%s: returning %d\r\n"), pszFname, fOk));
    return fOk;
}

#endif  // PM_SUPPORTS_DEVICE_QUERIES

// This routine updates state for all devices.  It should be called during
// system power state transitions so that device power states can be
// adjusted appropriately.
VOID
UpdateClassDeviceStates(PDEVICE_LIST pdl)
{
    BOOL fDeviceRemoved;
    PDEVICE_STATE pds;
    SETFNAME(_T("UpdateClassDeviceStates"));

    PREFAST_DEBUGCHK(pdl != NULL);

    PMLOCK();

    // Since it's possible that the device list may be modified
    // while we are not holding the PM critical section we need
    // to watch out for the device we are working on getting removed.
    // We increment the reference count to keep the device data 
    // structure from being deallocated out from under us.  Once
    // the device update is complete and we have the critical section
    // again, we check to see if it's still part of the device list.
    // If it's not, we start again at the top of the list.  This should
    // be harmless since devices that are at the right power state
    // already won't be updated.
    do {
        fDeviceRemoved = FALSE;
        pds = pdl->pList;
        while(!fDeviceRemoved && pds != NULL) {
            PDEVICE_STATE pdsNext;
            DeviceStateAddRef(pds);     // keep the device pointer alive
            
            PMUNLOCK();
            UpdateDeviceState(pds);
            PMLOCK();
            
            // is the device still on the list?
            if(pds->pListHead == NULL) {
                // device disappeared while we were accessing it
                PMLOGMSG(ZONE_WARN || ZONE_DEVICE, 
                    (_T("%s: device '%s' removed during update\r\n"), pszFname,
                    pds->pszName));
                fDeviceRemoved = TRUE;
            } else {
                // save the next pointer so we don't try to
                // dereference pds after decrementing its use count.
                pdsNext = pds->pNext;
            }
            DeviceStateDecRef(pds);     // done with the device pointer
            if(!fDeviceRemoved) pds = pdsNext;  // on to the next
        }
    } while(fDeviceRemoved || pds != NULL);

    PMUNLOCK();
}

// This routine updates state for all devices of all classes.  It can be called during
// system power state transitions so that device power states can be
// adjusted appropriately.
VOID
UpdateAllDeviceStates(VOID)
{
    PDEVICE_LIST pdl;

    // update all devices of all classes
    for(pdl = gpDeviceLists; pdl != NULL; pdl = pdl->pNext) {
        UpdateClassDeviceStates(pdl);
    }
}

// this routine checks to see if a device pointer is actually on a 
// list someplace.  If it finds it, it increments its reference
// count and returns true.
BOOL
CheckDevicePointer(PDEVICE_STATE pds)
{
    PDEVICE_LIST pdl;
    BOOL fFound = FALSE;

    // look for a match
    DEBUGCHK(pds != NULL);
    PMLOCK();
    for(pdl = gpDeviceLists; pdl != NULL && !fFound; pdl = pdl->pNext) {
        PDEVICE_STATE pdsTraveller;
        for(pdsTraveller = pdl->pList; pdsTraveller != NULL; 
        pdsTraveller = pdsTraveller->pNext) {
            if(pdsTraveller == pds) {
                fFound = TRUE;
                break;
            }
        }
    }

    // did we find the device?
    if(fFound) {
        // yes, update its reference count
        DeviceStateAddRef(pds);
    }
    PMUNLOCK();

    return fFound;
}

// This routine adds a device to the list associated with its device class.
// This routine does not return a value; it will either create a new
// device state structure and add it to a list or it will not.  If the new
// device duplicates an existing one this routine won't create a new node.
// This routine executes in the context of the PnP thread, which handles
// device interface additions and removals.
VOID
AddDevice(LPCGUID guidDevClass, LPCTSTR pszName, PDEVICE_STATE pdsParent, 
          PPOWER_CAPABILITIES pCaps)
{
    SETFNAME(_T("AddDevice"));

    PMLOGMSG(ZONE_DEVICE, 
        (_T("%s: adding '%s', pdsParent 0x%08x, pCaps 0x%08x to class %08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x\r\n"), 
        pszFname, pszName, pdsParent, pCaps,
        guidDevClass->Data1, guidDevClass->Data2, guidDevClass->Data3,
        (guidDevClass->Data4[0] << 8) + guidDevClass->Data4[1], guidDevClass->Data4[2], guidDevClass->Data4[3], 
        guidDevClass->Data4[4], guidDevClass->Data4[5], guidDevClass->Data4[6], guidDevClass->Data4[7]));

    RETAILMSG(1, 
        (_T("%s: adding '%s', pdsParent 0x%08x, pCaps 0x%08x to class %08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x\r\n"), 
		L"PM:AddDevice", pszName, pdsParent, pCaps,
        guidDevClass->Data1, guidDevClass->Data2, guidDevClass->Data3,
        (guidDevClass->Data4[0] << 8) + guidDevClass->Data4[1], guidDevClass->Data4[2], guidDevClass->Data4[3], 
        guidDevClass->Data4[4], guidDevClass->Data4[5], guidDevClass->Data4[6], guidDevClass->Data4[7]));

    // figure out onto which list this device should be added
    PDEVICE_LIST pdl = GetDeviceListFromClass(guidDevClass);
    
    // did we find the list?
    if(pdl != NULL) {
        // check for duplicates
        PDEVICE_STATE pds = DeviceStateFindList(pdl, pszName);
        
        // create the device if it doesn't already exist
        if(pds == NULL) {
            BOOL fOk = FALSE;
            pds = DeviceStateCreate(pszName);
            if(pds != NULL) {
                // if we are passed the device's capabilities, just copy them
                // into the structure
                if(pCaps != NULL) {
                    __try {
                        pds->caps = *pCaps;
                    }
                    __except(EXCEPTION_EXECUTE_HANDLER) {
                        PMLOGMSG(ZONE_WARN, 
                            (_T("%s: exception during capabilities copy from 0x%08x\r\n"),
                            pszFname, pCaps));
                        pCaps = NULL;
                    }
                }
                
                // update the device's parent pointer
                if(pdsParent != NULL) {
                    DeviceStateAddRef(pdsParent);
                }
                pds->pParent = pdsParent;
                
                // add the new device to its class list
                if(!DeviceStateAddList(pdl, pds)) {
                    // deallocate the node, reference count isn't incremented
                    DeviceStateDecRef(pds);
                    pds = NULL;
                } else {
                    PREFAST_DEBUGCHK(pds->pInterface != NULL);
                    PREFAST_DEBUGCHK(pds->pInterface->pfnOpenDevice != NULL);
                    PREFAST_DEBUGCHK(pds->pInterface->pfnRequestDevice != NULL);
                    PREFAST_DEBUGCHK(pds->pInterface->pfnCloseDevice != NULL);
                    pds->hDevice = pds->pInterface->pfnOpenDevice(pds);
                    if(pds->hDevice == INVALID_HANDLE_VALUE) {
                        PMLOGMSG(ZONE_WARN, (_T("%s: couldn't open device '%s'\r\n"),
                            pszFname, pszName != NULL ? _T("<NULL>") : pszName));
                    } else {
                        // do we need to request capabilities?
                        fOk = TRUE;             // assume success
                        if(pCaps == NULL) {
                            DWORD dwBytesReturned;
                            POWER_RELATIONSHIP pr;
                            PPOWER_RELATIONSHIP ppr = NULL;
                            memset(&pr, 0, sizeof(pr));
                            if(pds->pParent != NULL) {
                                PMLOGMSG(ZONE_DEVICE, (_T("%s: parent of '%s' is '%s'\r\n"), 
                                    pszFname, pds->pszName, pds->pParent->pszName));
                                pr.hParent = (HANDLE) pds->pParent;
                                pr.pwsParent = pds->pParent->pszName;
                                pr.hChild = (HANDLE) pds;
                                pr.pwsChild = pds->pszName;
                                ppr = &pr;
                            }                        
                            
                            // get the device's capabilities structure
                            fOk = pds->pInterface->pfnRequestDevice(pds->hDevice, IOCTL_POWER_CAPABILITIES, 
                                ppr, ppr == NULL ? 0 : sizeof(*ppr), 
                                &pds->caps, sizeof(pds->caps), &dwBytesReturned);
                            
                            // sanity check the size in case a device is just returning
                            // a good status on all ioctls for some reason
                            if(fOk && dwBytesReturned != sizeof(pds->caps)) {
                                PMLOGMSG(ZONE_WARN, 
                                    (_T("%s: invalid size returned from IOCTL_POWER_CAPABILITIES\r\n"),
                                    pszFname));
                                fOk = FALSE;
                            }
                        }

                        // any problems so far?
                        if(fOk) {
                            // determine whether we should request power relationships from a parent device
                            if((pds->caps.Flags & POWER_CAP_PARENT) != 0) {
                                pds->pInterface->pfnRequestDevice(pds->hDevice, IOCTL_REGISTER_POWER_RELATIONSHIP,
                                    NULL, 0, NULL, 0, NULL);
                            }
                        }
                    }
                }
                
                // have we read all the configuration information we need from 
                // the new device
                if (pds != NULL ) {
                    if(!fOk) {
                        // no, delete the device
                        DeviceStateRemList(pds);
                    } else {
                        // See if the device supports multiple handles.  Power manageable devices
                        // should allow multiple open handles, but if they don't we will have to open
                        // one before each access.
                        HANDLE hDevice = pds->pInterface->pfnOpenDevice(pds);
                        if(hDevice == INVALID_HANDLE_VALUE) {
                            PMLOGMSG(ZONE_WARN, (_T("%s: WARNING: '%s' does not support multiple handles\r\n"),
                                pszFname, pds->pszName));
                            pds->pInterface->pfnCloseDevice(pds->hDevice);
                            pds->hDevice = INVALID_HANDLE_VALUE;
                        } else {
                            // close the second handle, since we don't need it
                            pds->pInterface->pfnCloseDevice(hDevice);
                        }
                        
                        // initialize the new device's power state variables
                        UpdateDeviceState(pds);
                    }
                }
            }
        }
        
        // we are done with the device pointer
        if(pds != NULL) {
            DeviceStateDecRef(pds);
        }
    } else {
        PMLOGMSG(ZONE_WARN, (_T("%s: class for device '%s' not supported\r\n"),
            pszFname, pszName));
    }
}

// this routine removes a device from the list of managed devices.  When
// the device's reference count goes to zero it will be deallocated.
VOID
RemoveDevice(LPCGUID guidDevClass, LPCTSTR pszName)
{
    SETFNAME(_T("RemoveDevice"));

    PMLOGMSG(ZONE_DEVICE, (_T("%s: removing '%s' from %08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x\r\n"), 
        pszFname, pszName, guidDevClass->Data1, guidDevClass->Data2, guidDevClass->Data3,
        (guidDevClass->Data4[0] << 8) + guidDevClass->Data4[1], guidDevClass->Data4[2], guidDevClass->Data4[3], 
        guidDevClass->Data4[4], guidDevClass->Data4[5], guidDevClass->Data4[6], guidDevClass->Data4[7]));

    // figure out onto which list this device should be added
    PDEVICE_LIST pdl = GetDeviceListFromClass(guidDevClass);

    // did we find the list?
    if(pdl != NULL) {
        // check for duplicates
        PDEVICE_STATE pds = DeviceStateFindList(pdl, pszName);
        if(pds == NULL) {
            PMLOGMSG(ZONE_WARN, (_T("%s: '%s' not in class list\r\n"),
                pszFname, pszName));
        } else {
            // disconnect the device from the list
            DeviceStateRemList(pds);
            
            // if we are a child device, decrement the parent pointer's
            // reference count
            if(pds->pParent != NULL) {
                DeviceStateDecRef(pds->pParent);
                pds->pParent = NULL;
            }
            
            // if we are a parent device, remove child devices (if any)
            if((pds->caps.Flags & POWER_CAP_PARENT) != 0) {
                // look for a match across all lists, since the child may not
                // be of the same class
                for(pdl = gpDeviceLists; pdl != NULL; pdl = pdl->pNext) {
                    PDEVICE_STATE pCur, pNext;
                    pCur = pdl->pList;
                    while(pCur != NULL) {
                        pNext = pCur->pNext;
                        if(pCur->pParent == pds) {
                            RemoveDevice(pdl->pGuid, pCur->pszName);
                        }
                        pCur = pNext;
                    }
                }
            }
            
            // we're done with the device pointer
            DeviceStateDecRef(pds);
        }
    }
}

// This routine looks up a given device and passes back its current power state
// via the pCurDx pointer.  It returns the following error codes:
//      ERROR_SUCCESS - device found, *pCurDx updated successfully
//      ERROR_FILE_NOT_FOUND - device not found
//      ERROR_INVALID_PARAMETER - bad device ID, flags, or pCurDx pointer.
EXTERN_C DWORD WINAPI
PmGetDevicePower(PVOID pvDevice, DWORD dwDeviceFlags, PCEDEVICE_POWER_STATE pCurDx)
{
    DWORD dwStatus = ERROR_FILE_NOT_FOUND;
    PDEVICEID pdi = NULL;
    SETFNAME(_T("PmGetDevicePower"));

    PMLOGMSG(ZONE_API || ZONE_DEVICE, (_T("+%s: device %s, flags 0x%08x\r\n"),
        pszFname, (dwDeviceFlags & POWER_NAME) != 0 ? (LPTSTR) pvDevice : _T("<UNKNOWN>"),
        dwDeviceFlags));

    // sanity check parameters
    if(pvDevice == NULL 
    || pCurDx == NULL
    || (pdi = DeviceIdParseNameString((LPCTSTR) pvDevice, dwDeviceFlags)) == NULL) {
        dwStatus = ERROR_INVALID_PARAMETER;
    } else {
        PDEVICE_LIST pdl;
        
        // look for the device list and the device
        pdl = GetDeviceListFromClass(pdi->pGuid);
        if(pdl != NULL) {
            PDEVICE_STATE pds = DeviceStateFindList(pdl, pdi->pszName);
            if(pds != NULL) {
                CEDEVICE_POWER_STATE curDx = PwrDeviceUnspecified;

                // how are we supposed to obtain the data?
                if((dwDeviceFlags & POWER_FORCE) == 0) {
                    // use our cached value
                    PMLOCK();
                    curDx = pds->curDx;
                    PMUNLOCK();
                    dwStatus = ERROR_SUCCESS;
                } else {
                    // request the device's power status
                    dwStatus = GetDevicePower(pds, &curDx);
                }

                // do we have a value to pass back?
                if(dwStatus == ERROR_SUCCESS) {
                    __try {
                        // yes, update the caller's parameter
                        *pCurDx = curDx;
                    }
                    __except(EXCEPTION_EXECUTE_HANDLER) {
                        PMLOGMSG(ZONE_WARN, (_T("%s: exception updating pCurDx 0x%08x\r\n"),
                            pszFname, pCurDx));
                        dwStatus = ERROR_INVALID_PARAMETER;
                    }
                }
                
                // done with its pointer
                DeviceStateDecRef(pds);
            }
        }
    }

    // release resources
    if(pdi != NULL) DeviceIdDestroy(pdi);

    PMLOGMSG(ZONE_API || ZONE_DEVICE || (dwStatus != ERROR_SUCCESS && ZONE_WARN), 
        (_T("-%s: returning dwStatus %d\r\n"), pszFname, dwStatus));

    // return our status
    return dwStatus;
}

// This routine looks up a given device and sets its device power state to the
// value in dwState.  It returns the following values:
//      ERROR_SUCCESS - device found and power state updated
//      ERROR_FILE_NOT_FOUND - device not found
//      ERROR_WRITE_FAULT - attempt to update the device power failed
//      ERROR_INVALID_PARAMETER - bad device ID, flags, or dwState value.
//      ERROR_ACCESS_DENIED - caller doesn't have permission to call this routine.

EXTERN_C DWORD WINAPI 
PmSetDevicePower(PVOID pvDevice, DWORD dwDeviceFlags, CEDEVICE_POWER_STATE newDx)
{
    DWORD dwStatus = ERROR_FILE_NOT_FOUND;
    PDEVICEID pdi = NULL;
    SETFNAME(_T("PmSetDevicePower"));

    RETAILMSG(1, (_T("+%s: device %s, flags 0x%08x, new Dx %d\r\n"),
        pszFname, (dwDeviceFlags & POWER_NAME) != 0 ? (LPTSTR) pvDevice : _T("<UNKNOWN>"),
        dwDeviceFlags, newDx));
/*
	if(gpSystemPowerState->dwFlags & (POWER_STATE_CRITICAL | POWER_STATE_OFF | POWER_STATE_RESET))
	{
		RETAILMSG(1, (_T("+%s: device %s: impossible set state, the platform is off\r\n"),
			pszFname, (dwDeviceFlags & POWER_NAME) != 0 ? (LPTSTR) pvDevice : _T("<UNKNOWN>")));
		return ERROR_SUCCESS;
	}
*/
    PMENTERUPDATE();

    // sanity check parameters
    if(pvDevice == NULL 
    || ((newDx < D0 || newDx > D4) && newDx != PwrDeviceUnspecified)
    || (pdi = DeviceIdParseNameString((LPCTSTR) pvDevice, dwDeviceFlags)) == NULL) {
        dwStatus = ERROR_INVALID_PARAMETER;
    } else if(PSLGetCallerTrust() != OEM_CERTIFY_TRUST) {
        dwStatus = ERROR_ACCESS_DENIED;
    } else {
        PDEVICE_LIST pdl;
        BOOL fOk = TRUE;

        // look for the device list and the device
		RETAILMSG(0, (_T("%s: look for %08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x\r\n"), 
			pszFname,pdi->pGuid->Data1, pdi->pGuid->Data2, pdi->pGuid->Data3,
			(pdi->pGuid->Data4[0] << 8) + pdi->pGuid->Data4[1], pdi->pGuid->Data4[2], pdi->pGuid->Data4[3], 
			pdi->pGuid->Data4[4], pdi->pGuid->Data4[5], pdi->pGuid->Data4[6], pdi->pGuid->Data4[7]));

        pdl = GetDeviceListFromClass(pdi->pGuid);
        if(pdl != NULL) {
			RETAILMSG(0, (_T("%s: find state %s\r\n"), pszFname, pdi->pszName));
            PDEVICE_STATE pds = DeviceStateFindList(pdl, pdi->pszName);
            if(pds != NULL) {
                // are we imposing a set value or are we removing a set value?
                if(newDx == PwrDeviceUnspecified) {
                    // remove the set value
                    PMLOCK();
                    pds->setDx = PwrDeviceUnspecified;
                    PMUNLOCK();
                    
                    // let the device find its own power level
                    UpdateDeviceState(pds);
                    
                    // we've removed the set constraint, no matter what UpdateDeviceStatus() did
                    dwStatus = ERROR_SUCCESS;
                } else {
                    // found the device, now try to update it
                    PMLOCK();
                    pds->setDx = newDx;
                    PMUNLOCK();
                    fOk = UpdateDeviceState(pds);
                    if(!fOk) {
                        PMLOCK();
                        pds->setDx = PwrDeviceUnspecified;
                        PMUNLOCK();
                        dwStatus = ERROR_WRITE_FAULT;
                    } else {
                        dwStatus = ERROR_SUCCESS;
                    }
                }
                
                // done with the device pointer
                DeviceStateDecRef(pds);
            }
			else
			{
				RETAILMSG(0, (_T("%s: PS not find state %s\r\n"), pszFname, pdi->pszName));
			}
        }
    }

    PMLEAVEUPDATE();

    // release resources
    if(pdi != NULL)
		DeviceIdDestroy(pdi);

    RETAILMSG(1, (_T("-%s: device %s (%d)\r\n"), pszFname,
		(dwDeviceFlags & POWER_NAME) != 0 ? (LPTSTR) pvDevice : _T("<UNKNOWN>"), dwStatus));

    // return our status
    return dwStatus;
}

// This routine is invoked when a device driver wants to have the power manager
// adjust its power state.  Power manager will attempt to honor the request
// within the constraints imposed on it by the current system power state,
// application requirements, and user set operations.
EXTERN_C DWORD WINAPI 
PmDevicePowerNotify(PVOID pvDevice, CEDEVICE_POWER_STATE reqDx, 
                    DWORD dwDeviceFlags)
{
    DWORD dwStatus = ERROR_SUCCESS;
    PDEVICEID pdi = NULL;
    SETFNAME(_T("PmDevicePowerNotify"));

    PMLOGMSG(ZONE_API, (_T("+%s: device '%s' req %d, flags 0x%08x\r\n"), pszFname, 
        (dwDeviceFlags & POWER_NAME) != 0 ? (LPTSTR) pvDevice : _T("<UNKNOWN>"),
        reqDx, dwDeviceFlags));

    // sanity check parameters
    if(pvDevice == NULL 
    || (reqDx < D0 || reqDx > D4)
    || (pdi = DeviceIdParseNameString((LPCTSTR) pvDevice, dwDeviceFlags)) == NULL) {
        dwStatus = ERROR_INVALID_PARAMETER;
    } else {
        PDEVICE_LIST pdl;
        BOOL fOk = TRUE;

        // look for the device list and the device
        pdl = GetDeviceListFromClass(pdi->pGuid);
        if(pdl == NULL) {
            dwStatus = ERROR_FILE_NOT_FOUND;
        } else {
            PDEVICE_STATE pds = DeviceStateFindList(pdl, pdi->pszName);
            if(pds == NULL) {
                dwStatus = ERROR_FILE_NOT_FOUND;
            } else {
                PMLOCK();
                
                PMLOGMSG(ZONE_DEVICE, 
                    (_T("%s: device '%s' request for D%d, curDx D%d, floorDx D%d, ceilingDx D%d, setDx %d\r\n"),
                    pszFname, pds->pszName, reqDx, pds->curDx, pds->floorDx, pds->ceilingDx, pds->setDx));
                
                // record this state change request
                pds->lastReqDx = reqDx;

                PMUNLOCK();

                // change the device power state if necessary
                fOk = UpdateDeviceState(pds);
                if(!fOk) {
                    PMLOGMSG(ZONE_WARN, 
                        (_T("%s: SetDevicePower('%s') failed when requesting D%d\r\n"),
                        pszFname, pds->pszName, reqDx));
                    dwStatus = ERROR_WRITE_FAULT;
                }
                
                // done with the device state pointer
                DeviceStateDecRef(pds);
            }
        }
    }

    // release resources
    if(pdi != NULL) DeviceIdDestroy(pdi);

    PMLOGMSG((dwStatus != ERROR_SUCCESS && ZONE_WARN) || ZONE_API,
        (_T("%s: returning %d\r\n"), pszFname, dwStatus));
    return dwStatus;
}

