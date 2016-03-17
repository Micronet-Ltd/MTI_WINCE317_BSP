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
// This module contains routines for handling power relationships.  A
// device such as NDIS or a bus driver can register as a parent of another
// device.  The parent will act as a proxy for all requests going to the 
// child device.  Parent devices can register with the PM when they handle
// the IOCTL_REGISTER_POWER_RELATIONSHIP message.
//

#include <pmimpl.h>

// This routine sets up a proxy relationship between a parent device
// and a child device.  This will overwrite any other proxy relationships
// created for the child device.  Only one level of relationships is
// supported; that is, a parent device cannot itself be a child device.
// This routine passes back an handle to the relationship and sets the
// global error status to:
//      ERROR_SUCCESS - relationship set up ok
//      ERROR_INVALID_PARAMETER - bad parameter of some sort
//      ERROR_FILE_EXISTS - child device already registered
// Note that in this implementation, a device cannot register itself
// (using AdvertiseInterface()) and then have a parent register to 
// proxy for it.
EXTERN_C HANDLE WINAPI 
PmRegisterPowerRelationship(PVOID pvParent, PVOID pvChild, 
                            PPOWER_CAPABILITIES pCaps, DWORD dwFlags)
{
    PDEVICEID pdiParent = NULL, pdiChild = NULL;
    PDEVICE_LIST pdlParent, pdlChild;
    PDEVICE_STATE pdsParent = NULL;
    PDEVICE_STATE pdsChild = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    SETFNAME(_T("PmRegisterPowerRelationship"));

    PMLOGMSG(ZONE_API, (_T("+%s\r\n"), pszFname));

    // sanity check parameters
    if(pvParent == NULL 
    || pvChild == NULL
    || (pdiParent = DeviceIdParseNameString((LPCTSTR) pvParent, dwFlags)) == NULL
    || (pdiChild = DeviceIdParseNameString((LPCTSTR) pvChild, dwFlags)) == NULL) {
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    // parameters ok so far?
    if(dwStatus == ERROR_SUCCESS) {
        pdlChild = GetDeviceListFromClass(pdiChild->pGuid);
        if(dwStatus == ERROR_SUCCESS) {
            // Look up device lists for parent and child, plus the parent
            // and child device structures.  The child cannot already exist.
            pdlParent = GetDeviceListFromClass(pdiParent->pGuid);
            if(pdlParent == NULL || pdlChild == NULL) {
                PMLOGMSG(ZONE_WARN, (_T("%s: can't find class for parent or child\r\n"),
                    pszFname));
                dwStatus = ERROR_INVALID_PARAMETER;
            } else if((pdsChild = DeviceStateFindList(pdlChild, pdiChild->pszName)) != NULL) {
                PMLOGMSG(ZONE_WARN, (_T("%s: child '%s' already exists\r\n"),
                    pszFname, pdiChild->pszName));
                DeviceStateDecRef(pdsChild);
                dwStatus = ERROR_FILE_EXISTS;
            } else {
                pdsParent = DeviceStateFindList(pdlParent, pdiParent->pszName);
                if(pdsParent == NULL) {
                    PMLOGMSG(ZONE_WARN, (_T("%s: can't find parent '%s'\r\n"), 
                        pszFname, pdiParent->pszName));
                    dwStatus = ERROR_INVALID_PARAMETER;
                } else if(pdsParent->pParent != NULL) {
                    PMLOGMSG(ZONE_WARN, (_T("%s: parent '%s' can't also be a child\r\n"),
                        pszFname, pdsParent->pszName));
                }
            }
        }
    }

    // if parameters were ok, proceed
    if(dwStatus == ERROR_SUCCESS) {
        // create and/or initialize the new device
        AddDevice(pdiChild->pGuid, pdiChild->pszName, pdsParent, pCaps);

        // get the return value
        pdsChild = DeviceStateFindList(pdlChild, pdiChild->pszName);
        if(pdsChild != NULL) {
            // we only want the pointer value for now
            DeviceStateDecRef(pdsChild);
        } else {
            // couldn't create the child device for some reason
            dwStatus = ERROR_GEN_FAILURE;
        }
    }

    // release resources
    if(pdsParent != NULL) DeviceStateDecRef(pdsParent);
    if(pdiParent != NULL) DeviceIdDestroy(pdiParent);
    if(pdiChild != NULL) DeviceIdDestroy(pdiChild);

    PMLOGMSG((dwStatus != ERROR_SUCCESS && ZONE_WARN) || ZONE_API, 
        (_T("%s: returning 0x%08x, status is %d\r\n"), pszFname, pdsChild,
        dwStatus));
    SetLastError(dwStatus);
    return (HANDLE) pdsChild;
}

// This routine releases a power relationship that was created with 
// PmRegisterPowerRelationship().  It returns
//      ERROR_SUCCESS - relationship removed
//      ERROR_INVALID_PARAMETER - bad handle
// Deregistering a power relationship has the side effect of deregistering
// the child device with the PM.  Note that if the child exits while
// the relationship is outstanding, the caller will get ERROR_INVALID_PARAMETER
// when they attempt to release the relationship handle.
EXTERN_C DWORD WINAPI 
PmReleasePowerRelationship(HANDLE h)
{
    PDEVICE_STATE pds = (PDEVICE_STATE) h;
    DWORD dwStatus = ERROR_INVALID_PARAMETER;
    SETFNAME(_T("PmReleasePowerRelationship"));

    PMLOGMSG(ZONE_API, (_T("%s: releasing 0x%08x\r\n"), pszFname, h));

    // make sure that this pointer is a child node with a parent
    if(pds != NULL) {
        BOOL fExists = CheckDevicePointer(pds); // increments refcnt if TRUE
        if(fExists) {
            // delete the device
            PREFAST_DEBUGCHK(pds->pListHead != NULL);
            RemoveDevice(pds->pListHead->pGuid, pds->pszName);

            // done with the pointer
            DeviceStateDecRef(pds);

            // return a good status
            dwStatus = ERROR_SUCCESS;
        }
    }

    PMLOGMSG(ZONE_API || (dwStatus != ERROR_SUCCESS && ZONE_WARN), (_T("%s: returning %d\r\n"), 
        pszFname, dwStatus));
    return dwStatus;
}
