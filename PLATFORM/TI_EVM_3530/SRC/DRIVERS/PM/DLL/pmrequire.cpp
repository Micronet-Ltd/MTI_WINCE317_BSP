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
// This module contains routines for managing device power requirements.
//

#include <pmimpl.h>

// This routine updates devices that are affected by a changed power requirement.
// The device ID may indicate a specific device (both class and name are
// specified) or a class of devices (class is specified, name is NULL).
VOID
UpdateRequirementDevices(PDEVICEID pDeviceId)
{
    SETFNAME(_T("UpdateRequirementDevices"));

    PMLOGMSG(ZONE_REQUIRE, 
        (_T("%s: updating '%s' of class %08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x\r\n"),
        pszFname, pDeviceId->pszName != NULL ? pDeviceId->pszName : _T("<All Devices>"),
        pDeviceId->pGuid->Data1, pDeviceId->pGuid->Data2, pDeviceId->pGuid->Data3,
        (pDeviceId->pGuid->Data4[0] << 8) + pDeviceId->pGuid->Data4[1], 
        pDeviceId->pGuid->Data4[2], pDeviceId->pGuid->Data4[3], 
        pDeviceId->pGuid->Data4[4], pDeviceId->pGuid->Data4[5], 
        pDeviceId->pGuid->Data4[6], pDeviceId->pGuid->Data4[7]));

    // all devices with a matching class
    PDEVICE_LIST pdl = GetDeviceListFromClass(pDeviceId->pGuid);
    PREFAST_DEBUGCHK(pdl != NULL);
    
    // are we updating one device or all devices of a class?
    if(pDeviceId->pszName != NULL) {
        PDEVICE_STATE pds = DeviceStateFindList(pdl, pDeviceId->pszName);
        if(pds != NULL) {
            PMLOGMSG(ZONE_REQUIRE, (_T("%s: notifying 0x%08x ('%s')\r\n"),
                pszFname, pds, pds->pszName));
            UpdateDeviceState(pds);
            DeviceStateDecRef(pds);
        }
    } else {
        BOOL fDeviceRemoved;
        PDEVICE_STATE pds;

        PMLOCK();
        
        // Update all devices in a given class.  If a device is
        // removed from the class list while we are iterating through
        // it, start again.  This should be harmless because once devices
        // have been updated they are at the right power level already.
        // At the worst a device will be set to its current power level, and
        // this will only happen if a device is removed from the list while
        // we are working on it.
        do {
            fDeviceRemoved = FALSE;
            pds = pdl->pList;
            while(!fDeviceRemoved && pds != NULL) {
                PDEVICE_STATE pdsNext;
                
                // keep the device structure from being deallocated while
                // we're using a pointer to it
                DeviceStateAddRef(pds);
                
                // update the device power level if necessary
                PMUNLOCK();
                PMLOGMSG(ZONE_REQUIRE, (_T("%s: notifying 0x%08x ('%s')\r\n"),
                    pszFname, pds, pds->pszName));
                UpdateDeviceState(pds);
                PMLOCK();
                
                // is the device still part of a list?
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
                
                // done with the pointer, although we can still
                // check if it's null
                DeviceStateDecRef(pds);
                
                // on to the next device, unless device removed
                if(!fDeviceRemoved) pds = pdsNext;
            }
        } while(fDeviceRemoved || pds != NULL);
        
        PMUNLOCK();
    }
}

// This routine is called when a process terminates.  It removes any 
// requirements held by the process and updates affected devices.
VOID
DeleteProcessRequirements(HANDLE hProcess)
{
    BOOL fDone = FALSE;
    SETFNAME(_T("DeleteProcessRequirements"));

    PMENTERUPDATE();
    
    while(!fDone) {
        PDEVICE_POWER_RESTRICTION pdpr;
        
        // a process is going away, clean up its resources if there are any
        PMLOCK();
        for(pdpr = gpFloorDx; pdpr != NULL; pdpr = pdpr->pNext) {
            // does the terminating process own this requirement?
            if(pdpr->hOwner == hProcess) {
                // clone the device identifier associated with the requirement
                // so that we can update affected devices after removing the
                // restriction.
                PDEVICEID pDeviceId = DeviceIdClone(pdpr->pDeviceId);
                
                // delete the requirement
                PMLOGMSG(ZONE_REQUIRE, 
                    (_T("%s: deleting requirement 0x%08x for process 0x%08x\r\n"),
                    pszFname, pdpr, hProcess));
                if(PowerRestrictionRemList(&gpFloorDx, pdpr)) {
                    PowerRestrictionDestroy(pdpr);
                }
                
                // update any affected devices
                if(pDeviceId != NULL) {
                    PMUNLOCK();
                    UpdateRequirementDevices(pDeviceId);
                    DeviceIdDestroy(pDeviceId);
                    PMLOCK();
                }
                
                // start again at the start of the list, since we had
                // to unlock to handle the device.  We will eventually
                // run out of requirements imposed by this process.
                break;
            }
        }
        PMUNLOCK();
        
        // did we get to the end of the list?
        if(pdpr == NULL) {
            // yes, we're all done now
            fDone = TRUE;
        }
    }

    PMLEAVEUPDATE();
}

// This routine releases power requirements imposed by applications.
// It returns:
//      ERROR_SUCCESS - requirement released OK.
//      ERROR_INVALID_HANDLE - couldn't find the requirement
EXTERN_C DWORD WINAPI 
PmReleasePowerRequirement(HANDLE h, DWORD dwFlags)
{
    DWORD dwStatus = ERROR_INVALID_HANDLE;
    PDEVICE_POWER_RESTRICTION pdpr = (PDEVICE_POWER_RESTRICTION) h;
    PDEVICEID pDeviceId = NULL;
    SETFNAME(_T("PmReleasePowerRequirement"));

    UnusedParameter(dwFlags);

    PMLOGMSG(ZONE_REQUIRE || ZONE_API, (_T("%s: releasing requirement 0x%08x\r\n"), 
        pszFname, h));

    PMENTERUPDATE();
    PMLOCK();

    // look for the entry.  If it's valid make sure that it has the same owner.
    BOOL fFound = PowerRestrictionCheckList(gpFloorDx, pdpr);
    if(fFound) {
        // don't allow the caller to delete somebody else's requirement
        if(pdpr->hOwner == GetCallerProcess()) {
            // clone the device identifier associated with the requirement
            // so that we can update affected devices after removing the
            // restriction.
            pDeviceId = DeviceIdClone(pdpr->pDeviceId);

            // delete the restriction
            fFound = PowerRestrictionRemList(&gpFloorDx, pdpr);
            DEBUGCHK(fFound);
            if(fFound) {
                PowerRestrictionDestroy(pdpr);
            }
        }
    }

    PMUNLOCK();

    // update any affected devices
    if(pDeviceId) {
        UpdateRequirementDevices(pDeviceId);
        DeviceIdDestroy(pDeviceId);
        dwStatus = ERROR_SUCCESS;
    }

    PMLEAVEUPDATE();

    PMLOGMSG((dwStatus != ERROR_SUCCESS && ZONE_WARN) || ZONE_API, 
        (_T("%s: returning %d\r\n"), pszFname, dwStatus));
    return dwStatus;
}

// Applications can use this routine to inform the power manager
// that they would like to have a specific device or class of devices
// operate at a certain power level.  If successful, this routine returns
// an opaque handle to the power requirement.  If there is a problem,
// this routine returns NULL.  GetLastError() will return the following:
//      ERROR_SUCCESS - operation completed successfully
//      ERROR_INVALID_PARAMETER - invalid device id or reqDx
//      ERROR_NOT_ENOUGH_MEMORY - error creating a new requirement data structure
EXTERN_C HANDLE WINAPI 
PmSetPowerRequirement(PVOID pvDevice, CEDEVICE_POWER_STATE reqDx, 
                      ULONG dwDeviceFlags, PVOID pvSystemState, 
                      ULONG dwStateFlags)
{
    HANDLE hRequirement = NULL;
    BOOL fUpdate = FALSE;
    PDEVICEID pdi = NULL;
    HANDLE hOwner = GetCallerProcess();
    DWORD dwStatus = ERROR_SUCCESS;
    SETFNAME(_T("PmSetPowerRequirement"));

    UnusedParameter(dwStateFlags);

    PMLOGMSG(ZONE_REQUIRE || ZONE_API, (_T("%s: creating device state D%d requirement\r\n"),
        pszFname, reqDx));

    PMENTERUPDATE();

    // sanity check parameters
    if(pvDevice == NULL 
    || (reqDx < D0 || reqDx > D4)
    || (pdi = DeviceIdParseNameString((LPCTSTR) pvDevice, dwDeviceFlags)) == NULL
    || GetDeviceListFromClass(pdi->pGuid) == NULL) {
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    // if parameters were ok then proceed
    if(dwStatus == ERROR_SUCCESS) {
        // mask off flags we don't need to keep
        dwDeviceFlags &= POWER_FORCE;

        PMLOCK();
        
        // create a new restriction structure
        PDEVICE_POWER_RESTRICTION pdpr = PowerRestrictionCreate(pdi, hOwner, reqDx, 
            (LPCTSTR) pvSystemState, dwDeviceFlags);
        if(pdpr == NULL) {
            dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        } else {
            PowerRestrictionAddList(&gpFloorDx, pdpr);
        }
        hRequirement = (HANDLE) pdpr;
        
        // should we update the device's power state?
        if(pdpr !=NULL && 
                (pdpr->pszSystemState == NULL || _tcscmp(pdpr->pszSystemState, gpSystemPowerState->pszName) == 0)) {
            fUpdate = TRUE;
        }
        
        PMUNLOCK();
        
        // if the update flag is set, we should update the device power for
        // all matching devices.  Whether this works or not doesn't affect the
        // return code from this API, since the restriction has been successfully
        // put into place.
        if(fUpdate) {
            UpdateRequirementDevices(pdi);
        }
    }

    PMLEAVEUPDATE();

    // release resources
    if(pdi != NULL) DeviceIdDestroy(pdi);

    // return handle and set global status
    PMLOGMSG(ZONE_REQUIRE || ZONE_API || (dwStatus != ERROR_SUCCESS && ZONE_WARN),
        (_T("%s: returning 0x%08x, status is %d\r\n"), pszFname,
        hRequirement, dwStatus));
    SetLastError(dwStatus);
    return hRequirement;
}
