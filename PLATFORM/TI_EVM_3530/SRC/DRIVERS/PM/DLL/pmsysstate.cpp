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
#include "PmSysReg.h"

// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)

// This routine enumerates device power restrictions in the registry
// and adds them to the list of existing restrictions.  It returns a pointer
// to the new list.
PDEVICE_POWER_RESTRICTION
RegReadClassDeviceRestrictions(RegKey * pRegKey, LPCGUID pGuidClass, 
                               PDEVICE_POWER_RESTRICTION pdprCurrent)
{
    TCHAR szDevName[MAX_PATH];
    DWORD dwNameChars, dwType, dwValue, dwValSize, dwIndex, dwStatus;
    DEVICEID devId;
    SETFNAME(_T("RegReadClassDeviceRestrictions"));

    PMLOGMSG(ZONE_REGISTRY, 
        (_T("%s: reading information for class {%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x}\r\n"),
        pszFname,
        pGuidClass->Data1, pGuidClass->Data2, pGuidClass->Data3,
        (pGuidClass->Data4[0] << 8) + pGuidClass->Data4[1], pGuidClass->Data4[2], pGuidClass->Data4[3], 
        pGuidClass->Data4[4], pGuidClass->Data4[5], pGuidClass->Data4[6], pGuidClass->Data4[7]));

    devId.pGuid = pGuidClass;
    dwIndex = 0;
    do {
        dwNameChars = dim(szDevName);
        dwValSize = sizeof(dwValue);
        dwStatus = pRegKey->RegEnumValue( dwIndex, szDevName, &dwNameChars);
        if (dwStatus == ERROR_SUCCESS)
            dwStatus =  pRegKey->RegFindValue(szDevName,(LPBYTE) &dwValue, &dwValSize,&dwType);
        switch(dwStatus) {
        case ERROR_SUCCESS:
            // make sure that the type of this data is a dword and isn't 
            // the reserved word "flags".
            if(dwType != REG_DWORD) {
                PMLOGMSG(ZONE_WARN || ZONE_REGISTRY, 
                    (_T("%s: read wrong type %d\r\n"), pszFname, dwType));
            } else if(_tcsicmp(szDevName, _T("Flags")) == 0) {
                PMLOGMSG(ZONE_REGISTRY, (_T("%s: skipping flags 0x%08x\r\n"),
                    pszFname, dwValue));
            } else if(dwValue < D0 || dwValue > D4) {
                PMLOGMSG(ZONE_WARN || ZONE_REGISTRY, 
                    (_T("%s: invalid device state value %d\r\n"), pszFname,
                    dwValue));
            } else {
                PDEVICE_POWER_RESTRICTION pdprNew;

                // if this is the "default" value it applies to all
                // devices of this class
                if(_tcsicmp(szDevName, _T("default")) == 0) {
                    devId.pszName = NULL;
                } else {
                    devId.pszName = szDevName;
                }
                pdprNew = PowerRestrictionCreate(&devId, 0, (CEDEVICE_POWER_STATE) dwValue, 
                    NULL, 0);
                if(pdprNew != NULL) {
                    PMLOGMSG(ZONE_REGISTRY,  (_T("%s: device '%s' restricted to D%d\r\n"),
                        pszFname, szDevName, dwValue));
                    pdprNew->pNext = pdprCurrent;
                    pdprCurrent = pdprNew;
                }
            }
            break;
        case ERROR_NO_MORE_ITEMS:
            break;
        default:
            // maybe a value of the wrong type or something, treat it as
            // non fatal so that we get all of the device names.
            PMLOGMSG(ZONE_WARN || ZONE_REGISTRY, 
                (_T("%s: RegEnumValue() returned %d\r\n"), pszFname, dwStatus));
            break;
        }
        dwIndex++;
    } while(dwStatus != ERROR_NO_MORE_ITEMS);


    return pdprCurrent;
}

// This routine reads device power restrictions from the registry.  These
// are subkeys of the key describing a system power state.  It builds a
// list of restrictions and returns a pointer to the list.
PDEVICE_POWER_RESTRICTION
RegReadDeviceRestrictions(RegKey * pRegKey)
{
    PDEVICE_POWER_RESTRICTION pdpr = NULL;
    GUID gClass;
    TCHAR szKeyName[MAX_PATH];
    DWORD dwNameChars, dwIndex, dwStatus;
    SETFNAME(_T("RegReadDeviceRestrictions"));

    // build up the list starting with class subkeys (no subkey is allowed
    // for generic devices).
    dwIndex = 0;
    do {
        dwNameChars = dim(szKeyName);
        dwStatus = pRegKey->RegEnumKeyEx(dwIndex, szKeyName, &dwNameChars);
        switch(dwStatus) {
        case ERROR_SUCCESS:
            // convert the key name to a class
            if(ConvertStringToGuid(szKeyName, &gClass)) {
                // open the key
                RegKey * pChildKey =  pRegKey->RegFindKey(szKeyName);
                if(pChildKey == NULL ) {
                    dwStatus = ERROR_NO_MORE_ITEMS;
                    PMLOGMSG(ZONE_WARN || ZONE_REGISTRY,
                        (_T("%s: RegOpenKeyEx('%s') failed %d\r\n"),
                        pszFname, szKeyName));
                } else {
                    // add the class's restriction data to the list
                    pdpr = RegReadClassDeviceRestrictions(pChildKey, 
                        &gClass, pdpr);
                }
            }
            break;
        case ERROR_NO_MORE_ITEMS:
            break;
        default:
            PMLOGMSG(ZONE_WARN || ZONE_REGISTRY, 
                (_T("%s: RegEnumKeyEx() returned %d\r\n"), pszFname, dwStatus));
            break;
        }
        dwIndex++;
    } while(dwStatus != ERROR_NO_MORE_ITEMS);

    // add values from the root key representing generic devices
    pdpr = RegReadClassDeviceRestrictions(pRegKey, &idGenericPMDeviceClass, pdpr);

    return pdpr;
}
EXTERN_C BOOL PmUpdateSystemPowerStatesIfChanged()
{
    return (g_pSysRegistryAccess!=NULL?g_pSysRegistryAccess->UpdateRegistryChange():FALSE);
}
// This routine reads a system state structure from the registry.  It 
// also loads device power restrictions associated with the power state.
// It returns ERROR_SUCCESS if successful or a Win32 error code otherwise.
// Note that malformed device power values won't cause an error.  The new
// system state information and device power restrictions will be passed
// back via pointers.
// If ppsps is NULL, the state keys are read but not passed back.  If ppdpr
// is NULL, individual device power settings are not read or passed back.
DWORD
RegReadSystemPowerState(LPCTSTR pszName, PPSYSTEM_POWER_STATE ppsps, 
                     PPDEVICE_POWER_RESTRICTION ppdpr)
{
    DWORD dwRetStatus = ERROR_SUCCESS;
    DWORD dwStatus, dwLen = MAX_PATH;
    static BOOL fBootPhase2Reopen = FALSE;
    RegKey *keyHandle;
    TCHAR szPath[MAX_PATH];
    SETFNAME(_T("ReadSystemPowerState"));

    PMLOCK();
    if (!fBootPhase2Reopen) { // We need reopen the key if we migrate to Phase 2.
        HANDLE hevBootPhase2 = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T("SYSTEM/BootPhase2"));
        if (hevBootPhase2) {
            if( WaitForSingleObject(hevBootPhase2, 0) == WAIT_OBJECT_0) { // If Phase2 Signaled.
                if (g_pSysRegistryAccess != NULL) {
                    delete g_pSysRegistryAccess;
                    g_pSysRegistryAccess = NULL;
                }
                fBootPhase2Reopen = TRUE;
            }
            CloseHandle(hevBootPhase2);
        }
        else
            fBootPhase2Reopen = TRUE; // If OpenEvent Failed, It meaning there is no Phase2 state. So We set it to by pass the checking.
    }
    if (g_pSysRegistryAccess==NULL) {
        // format the key name
        StringCbPrintf(szPath, sizeof(szPath), _T("%s\\%s"), PWRMGR_REG_KEY, _T("State"));
        dwLen = (dim(szPath) - 1) - _tcslen(szPath);
        g_pSysRegistryAccess = new SystemNotifyRegKey (HKEY_LOCAL_MACHINE,szPath);
        if (g_pSysRegistryAccess && g_pSysRegistryAccess->Init()== FALSE) {
            delete g_pSysRegistryAccess;
            g_pSysRegistryAccess = NULL;
        }
    }
    PMUNLOCK();
    if (g_pSysRegistryAccess== NULL ) { // false to initialize the structure.
        return ERROR_INVALID_PARAMETER;
    }
    g_pSysRegistryAccess->EnterLock();
    __try {
        // copy the name and make sure it's null terminated
        _tcsncpy(szPath, pszName, dwLen);
        szPath[dim(szPath) - 1] = 0;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        PMLOGMSG(ZONE_WARN, (_T("%s: exception copying state name\r\n"), pszFname));
        dwRetStatus = ERROR_INVALID_PARAMETER;
    }
    
    keyHandle = g_pSysRegistryAccess->RegFindKey(szPath);
    if(keyHandle == NULL ) {
        PMLOGMSG(ZONE_WARN | ZONE_REGISTRY, 
            (_T("%s: RegOpenKeyEx('%s') failed\r\n"), pszFname, szPath));
        dwRetStatus = ERROR_NO_MORE_ITEMS ;
    } else {
        DWORD dwDefaultDx, dwFlags, dwSize, dwType;

        // read the default power state and the flags -- both values
        // must be present
        dwSize = sizeof(dwDefaultDx);
        dwStatus = keyHandle->RegFindValue(_T("Default"), &dwDefaultDx, &dwSize, &dwType);
        if(dwStatus != ERROR_SUCCESS) {
            PMLOGMSG(ZONE_WARN, 
                (_T("%s: can't read default Dx from '%s', status is %d\r\n"),
                pszFname, szPath, dwStatus));
            dwRetStatus = dwStatus;
        }
        if(dwDefaultDx < D0 || dwDefaultDx > D4) {
            PMLOGMSG(ZONE_WARN,
                (_T("%s: invalid Dx value for '%s': %d\r\n"), pszFname,
                pszName, dwDefaultDx));
            dwRetStatus = ERROR_INVALID_DATA;
        }

        if(dwRetStatus == ERROR_SUCCESS) {
            dwSize = sizeof(dwFlags);
            dwStatus =keyHandle->RegFindValue( _T("Flags"), &dwFlags, &dwSize, &dwType);
            if(dwStatus != ERROR_SUCCESS) {
                PMLOGMSG(ZONE_WARN, 
                    (_T("%s: can't read flags from '%s', status is %d\r\n"),
                    pszFname, szPath, dwStatus));
                dwRetStatus = dwStatus;
            }
        }

        PMLOGMSG(dwRetStatus == ERROR_SUCCESS && ZONE_REGISTRY,
            (_T("%s: state '%s' devices max is D%d, flags 0x%08x\r\n"),
            pszFname, pszName, dwDefaultDx, dwFlags));
        if(dwRetStatus == ERROR_SUCCESS && ppsps != NULL) {
            // allocate and initialize a system power state
            PSYSTEM_POWER_STATE psps = SystemPowerStateCreate(pszName);
            if(psps == NULL) {
                dwRetStatus = ERROR_OUTOFMEMORY;
            } else {
                // initialize the structure
                psps->defaultCeilingDx = (CEDEVICE_POWER_STATE) dwDefaultDx;
                psps->dwFlags = dwFlags;
                *ppsps = psps;
            }
        }
    }

    // read device power modifiers for all associated devices
    if(dwRetStatus == ERROR_SUCCESS && ppdpr != NULL) {
        PDEVICE_POWER_RESTRICTION pdpr = RegReadDeviceRestrictions(keyHandle);
        *ppdpr = pdpr;
    }

    g_pSysRegistryAccess->LeaveLock();
    
    return dwRetStatus;
}

// This routine allows applications to determine what the current system
// power state name is.  It also passes back flag bits which provide some
// information about the state.
EXTERN_C DWORD WINAPI 
PmGetSystemPowerState(LPWSTR pBuffer, DWORD dwBufChars, PDWORD pdwFlags)
{
    DWORD dwStatus = ERROR_INVALID_PARAMETER;
    SETFNAME(_T("PmGetSystemPowerState"));
    
    PMLOGMSG(ZONE_API, (_T("+%s: buf 0x%08x, size %d, pflags 0x%08x\r\n"),
        pszFname, pBuffer, dwBufChars, pdwFlags));

    PMLOCK();

    __try {
        if(dwBufChars < (_tcslen(gpSystemPowerState->pszName) + 1)) {
            dwStatus = ERROR_INSUFFICIENT_BUFFER;
        } else {
            StringCchCopy(pBuffer, dwBufChars, gpSystemPowerState->pszName);
            *pdwFlags = gpSystemPowerState->dwFlags;
            dwStatus = ERROR_SUCCESS;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    PMUNLOCK();

    PMLOGMSG(ZONE_API, (_T("-%s: returning %d\r\n"), pszFname, dwStatus));

    return dwStatus;
}

// This routine wraps system power state transitions.  It can be called
// from within the PM or (indirectly) by applications.  The fInternal
// flag indicates whether the invocation is from the PM or an application.
// If apps don't know the name of the state in which they're interested,
// the PM will find a mapping for them.  This routine serializes system power
// state changes using a critical section.  Drivers should not change system 
// power states while handling device power state transitions.  If they do, 
// they may deadlock the power manager.
DWORD
PmSetSystemPowerState_I(LPCWSTR pwsState, DWORD dwStateHint, DWORD dwOptions, 
                        BOOL fInternal)
{
    TCHAR szStateName[MAX_PATH];
    DWORD dwStatus = ERROR_SUCCESS;
    SETFNAME(_T("PmSetSystemPowerState_I"));

    PMLOGMSG(ZONE_API, (_T("+%s: name %s, hint 0x%08x, options 0x%08x, fInternal %d\r\n"),
        pszFname, pwsState != NULL ? pwsState : _T("<NULL>"), dwStateHint, dwOptions,
        fInternal));

    PMENTERUPDATE();

    // if the user passes a null state name, use the hints flag to try
    // to find a match.
    if(pwsState != NULL) {
        // copy the parameter but watch out for bad pointers
        __try {
            _tcsncpy(szStateName, pwsState, dim(szStateName) - 1);
            szStateName[dim(szStateName) - 1] = 0;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            PMLOGMSG(ZONE_WARN, (_T("%s: exception copying state name\r\n"), 
                pszFname));
            dwStatus = ERROR_INVALID_PARAMETER;
        }
    } else {
        // try to match the hint flag to a system state
        dwStatus = PlatformMapPowerStateHint(dwStateHint, szStateName, dim(szStateName));
    }
    
    // go ahead and do the update?
    if(dwStatus == ERROR_SUCCESS) {
        BOOL fForce;
        
        if((dwOptions & POWER_FORCE) != 0) {
            fForce = TRUE;
        } else {
            fForce = FALSE;
        }
        dwStatus = PlatformSetSystemPowerState(szStateName, fForce, fInternal);
    }

    PMLEAVEUPDATE();

    PMLOGMSG(ZONE_API, (_T("-%s: returning dwStatus %d\r\n"), pszFname, dwStatus));

    return dwStatus;
}

// Applications can call this routine to modify the system power state.
// If they don't know the name of the state in which they're interested,
// the PM will find a mapping for them.  Drivers should not change system 
// power states while handling device power state transitions.  If they do, 
// they may deadlock the power manager.
EXTERN_C DWORD WINAPI 
PmSetSystemPowerState(LPCWSTR pwsState, DWORD dwStateHint, DWORD dwOptions)
{
    TCHAR szState[MAX_PATH];
    LPTSTR pszStateName = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    SETFNAME(_T("PmSetSystemPowerState"));

    PMLOGMSG(ZONE_API, (_T("+%s: name %s, hint 0x%08x, options 0x%08x\r\n"),
        pszFname, pwsState != NULL ? pwsState : _T("<NULL>"), dwStateHint, dwOptions));

    // if the user passes a null state name, use the hints flag to try
    // to find a match.
    if(pwsState != NULL) {
        // copy the parameter but watch out for bad pointers
        __try {
            DWORD dwIndex;
            for(dwIndex = 0; pwsState[dwIndex] != 0 && dwIndex < (dim(szState) - 1); dwIndex++) {
                szState[dwIndex] = _totlower(pwsState[dwIndex]);
            }
            szState[dwIndex] = 0;
            pszStateName = szState;

            // don't pass on truncated system power state names
            if(pwsState[dwIndex] != 0) {
                PMLOGMSG(ZONE_WARN, 
                    (_T("%s: system power state name '%s' exceeds %d characters\r\n"), 
                    pszFname, pwsState, dim(szState) - 1));
                dwStatus = ERROR_INVALID_PARAMETER;
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            PMLOGMSG(ZONE_WARN, (_T("%s: exception copying state name\r\n"), 
                pszFname));
            dwStatus = ERROR_INVALID_PARAMETER;
        }
    }

    // carry out the power state change
    if(dwStatus == ERROR_SUCCESS) {
        dwStatus = PlatformSendSystemPowerState(pszStateName, dwStateHint, dwOptions) ;
    }

    PMLOGMSG(ZONE_API, (_T("-%s: returning dwStatus %d\r\n"), pszFname, dwStatus));

    return dwStatus;
}
