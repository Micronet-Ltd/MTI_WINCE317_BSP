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
// This modules contains various utility routines for creating, deleting
// and accessing power manager data.
//

#include <pmimpl.h>
#include <msgqueue.h>
#include <nkintr.h>

// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)

#ifdef DEBUG
// turns on some memory garbling code -- adds overhead but hopefully helps catch bugs
#define DEBUGALLOC
#endif  // DEBUG


// ------------------------ REGISTRY FUNCTIONS ------------------------

// This routine reads data from the registry and verifies that it's
// of the proper type.  It returns ERROR_SUCCESS if the data can be
// read and is the right type or a registry error if it can't be
// read.  If the data is the wrong type, this routine returns
// ERROR_INVALID_DATA.
DWORD
RegQueryTypedValue(HKEY hk, LPCWSTR pszValue, PVOID pvData, 
                   LPDWORD pdwSize, DWORD dwType)
{
    DWORD dwActualType;
    SETFNAME(_T("RegQueryTypedValue"));

    DWORD dwStatus = RegQueryValueEx(hk, pszValue, NULL, &dwActualType, 
        (PBYTE) pvData, pdwSize);
    if(dwStatus == ERROR_SUCCESS && dwActualType != dwType) {
        PMLOGMSG(ZONE_WARN, (_T("%s: wrong type for '%s'\r\n"),
            pszFname, pszValue));
        dwStatus = ERROR_INVALID_DATA;
    }

    return dwStatus;
}

BOOL
GetPMThreadPriority(LPCTSTR pszValue, LPINT piThreadPriority)
{
    BOOL fOk = FALSE;
    HKEY hkPm = NULL;
    DWORD dwStatus;
    SETFNAME(_T("GetPMThreadPriority"));

    // open the PM key
    dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, PWRMGR_REG_KEY, 0, 0, &hkPm);
    if(dwStatus == ERROR_SUCCESS) {
        DWORD dwValue;
        DWORD dwSize = sizeof(dwValue);

        // read the requested value
        dwStatus = RegQueryTypedValue(hkPm, pszValue, &dwValue, &dwSize, REG_DWORD);
        if(dwStatus == ERROR_SUCCESS) {
            *piThreadPriority = (INT) dwValue;
            PMLOGMSG(ZONE_REGISTRY, (_T("%s: priority for '%s' is %d\r\n"),
                pszFname, pszValue, dwValue));
            fOk = TRUE;
        }

        // close the key
        RegCloseKey(hkPm);
    }

    PMLOGMSG(!fOk && ZONE_REGISTRY, (_T("%s: no setting for '%s'\r\n"),
        pszFname, pszValue));

    return fOk;
}

// ------------------------ SERIALIZATION ------------------------

// serialize access to PM list variables and structure members
VOID
PmLock(VOID)
{
    EnterCriticalSection(&gcsPowerManager);
}

// release synchronization objects obtained with PmLock()
VOID
PmUnlock(VOID)
{
    LeaveCriticalSection(&gcsPowerManager);
}

// serialize access to appliation APIs that cause device updates
VOID
PmEnterUpdate(VOID)
{
    EnterCriticalSection(&gcsDeviceUpdateAPIs);
}

// release synchronization objects obtained with PmEnterUpdate()
VOID
PmLeaveUpdate(VOID)
{
    LeaveCriticalSection(&gcsDeviceUpdateAPIs);
}

// ------------------------ MEMORY STATE MANAGEMENT ------------------------

#define HEAPSIGNATURE       0xFA124301      // randomly chosen number
#define HEAPHEADERSIZE      (4 * sizeof(DWORD))

#ifdef DEBUG
// debug globals
LONG glTotalObjects = 0;
#endif  // DEBUG

// this routine allocates memory and returns a pointer to it, or returns
// NULL.
PVOID
PmAlloc(DWORD dwSize)
{
    SETFNAME(_T("PmAlloc"));

    DEBUGCHK(dwSize != 0);

#ifdef DEBUGALLOC
    // prepend a header so we can track post-free accesses of the memory
    dwSize += HEAPHEADERSIZE;
#endif  // DEBUGALLOC

    PVOID pvRet = HeapAlloc(ghPmHeap, 0, dwSize);
    if(pvRet == NULL) {
        PMLOGMSG(ZONE_WARN, (_T("%s: HeapAlloc(%d) failed %d\r\n"), pszFname,
            dwSize, GetLastError()));
    }
#ifdef DEBUG
    else {
        InterlockedIncrement(&glTotalObjects);
    }
#endif  // DEBUG

#ifdef DEBUGALLOC
    // fill memory with a diagnostic pattern for debugging
    if(pvRet != NULL) {
        memset(pvRet, 0xAA, dwSize);
        PDWORD pdwRet = (PDWORD) pvRet;
        pdwRet[0] = HEAPSIGNATURE;  // record a signature
        pdwRet[1] = dwSize;         // and the real size of the buffer
        pvRet = &pdwRet[HEAPHEADERSIZE / sizeof(DWORD)]; // return a pointer to a buffer of the requested size
    }
#endif  // DEBUGALLOC

#ifdef DEBUG
    PMLOGMSG(ZONE_ALLOC, (_T("%s: alloc %d bytes returned 0x%08x (%d total objects allocated)\r\n"), 
        pszFname, dwSize, pvRet, glTotalObjects));
#else   // DEBUG
    PMLOGMSG(ZONE_ALLOC, (_T("%s: alloc %d bytes returned 0x%08x\r\n"), 
        pszFname, dwSize, pvRet));
#endif  // DEBUG
    return pvRet;
}

// this routine frees memory allocated with PmAlloc()
BOOL
PmFree(PVOID pvMemory)
{
    SETFNAME(_T("PmFree"));

    PMLOGMSG(ZONE_ALLOC, (_T("%s: freeing 0x%08x\r\n"), pszFname, pvMemory));

    DEBUGCHK(pvMemory != NULL);

#ifdef DEBUGALLOC
    // fill 
    if(pvMemory != NULL) {
        PDWORD pdwMemory = (PDWORD) ((DWORD) pvMemory - HEAPHEADERSIZE);
        DEBUGCHK(pdwMemory[0] == HEAPSIGNATURE && pdwMemory[1] > HEAPHEADERSIZE);
        memset(pdwMemory, 0xDD, pdwMemory[1]);
        pvMemory = pdwMemory;
    }
#endif  // DEBUGALLOC

    BOOL fOk = HeapFree(ghPmHeap, 0, pvMemory);
    if(!fOk) {
        PMLOGMSG(ZONE_WARN, (_T("%s: HeapFree(0x%08x) failed %d\r\n"), pszFname,
            pvMemory, GetLastError()));
    }
#ifdef DEBUG
    else {
        InterlockedDecrement(&glTotalObjects);
        DEBUGCHK(glTotalObjects >= 0);
    }
#endif  // DEBUG
    return fOk;
}

// ------------------------ DEVICE ID MANAGEMENT ------------------------

static BOOL 
HexStringToDword(LPCWSTR FAR& lpsz, DWORD FAR& Value,
                 int cDigits, WCHAR chDelim)
{
    int Count;
    
    Value = 0;
    for (Count = 0; Count < cDigits; Count++, lpsz++)
    {
        if (*lpsz >= '0' && *lpsz <= '9')
            Value = (Value << 4) + *lpsz - '0';
        else if (*lpsz >= 'A' && *lpsz <= 'F')
            Value = (Value << 4) + *lpsz - 'A' + 10;
        else if (*lpsz >= 'a' && *lpsz <= 'f')
            Value = (Value << 4) + *lpsz - 'a' + 10;
        else
            return(FALSE);
    }
    
    if (chDelim != 0)
        return *lpsz++ == chDelim;
    else
        return TRUE;
}


static BOOL 
wUUIDFromString(LPCWSTR lpsz, LPGUID pguid)
{
    DWORD dw;
    
    if (!HexStringToDword(lpsz, pguid->Data1, sizeof(DWORD)*2, '-'))
        return FALSE;
    
    if (!HexStringToDword(lpsz, dw, sizeof(WORD)*2, '-'))
        return FALSE;
    
    pguid->Data2 = (WORD)dw;
    
    if (!HexStringToDword(lpsz, dw, sizeof(WORD)*2, '-'))
        return FALSE;
    
    pguid->Data3 = (WORD)dw;
    
    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
        return FALSE;
    
    pguid->Data4[0] = (BYTE)dw;
    
    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, '-'))
        return FALSE;
    
    pguid->Data4[1] = (BYTE)dw;
    
    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
        return FALSE;
    
    pguid->Data4[2] = (BYTE)dw;
    
    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
        return FALSE;
    
    pguid->Data4[3] = (BYTE)dw;
    
    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
        return FALSE;
    
    pguid->Data4[4] = (BYTE)dw;
    
    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
        return FALSE;
    
    pguid->Data4[5] = (BYTE)dw;
    
    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
        return FALSE;
    
    pguid->Data4[6] = (BYTE)dw;
    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
        return FALSE;
    
    pguid->Data4[7] = (BYTE)dw;
    
    return TRUE;
}

static BOOL 
GUIDFromString(LPCWSTR lpsz, LPGUID pguid)
{
    if (*lpsz++ != '{' )
        return FALSE;
    
    if(wUUIDFromString(lpsz, pguid) != TRUE)
        return FALSE;
    
    lpsz +=36;
    
    if (*lpsz++ != '}' )
        return FALSE;
    
    return TRUE;
}

// this routine converts a text string to a GUID if possible.
BOOL 
ConvertStringToGuid (LPCTSTR pszGuid, GUID *pGuid)
{
    BOOL fOk = FALSE;
    SETFNAME(_T("ConvertStringToGuid"));

    DEBUGCHK(pGuid != NULL && pszGuid != NULL);
    __try {
        if (! GUIDFromString(pszGuid, pGuid)) {
            PMLOGMSG(ZONE_WARN, (_T("%s: couldn't parse '%s'\r\n"), pszFname,
                pszGuid));
        }
        fOk = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        PMLOGMSG(ZONE_WARN, (_T("%s: error parsing guid\r\n"), pszFname));
    }

    return fOk;
}

// This routine returns the number of bytes that would need to be allocated to hold
// a copy of the device ID being passed in as a parameter.  It returns zero if there
// is an error OR if the device id pointer is NULL.
DWORD
DeviceIdSize(PDEVICEID pId)
{
    DWORD dwSize;
    SETFNAME(_T("DeviceIdSize"));

    if(pId != NULL) {
        // calculate the amount of memory to allocate
        __try {
            dwSize = sizeof(*pId);
            if(pId->pszName != NULL) {
                dwSize += (_tcslen(pId->pszName) + 1) * sizeof(pId->pszName[0]);
            }
            if(pId->pGuid != NULL) {
                dwSize += sizeof(GUID);
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            PMLOGMSG(ZONE_WARN, (_T("%s: exception calculating size\r\n"), pszFname));
            dwSize = 0;;
        }
    } else {
        dwSize = 0;
    }

    return dwSize;
}

// This routine clones a source device ID into a pre-allocated buffer.  The 
// pre-allocation enables callers to reduce the number of heap allocations
// they need to make when embedding device IDs into larger structures.
// This routine returns pIdDst, or NULL in case of error.  Errors include
// insufficient buffer size or exceptions while copying.
PDEVICEID
DeviceIdCloneIntoBuffer(PDEVICEID pIdSrc, PDEVICEID pIdDst, DWORD cbDst)
{
    DWORD dwSize;
    SETFNAME(_T("DeviceIdCloneIntoBuffer"));

    PREFAST_DEBUGCHK(pIdSrc != NULL);
    DEBUGCHK(pIdSrc->pGuid != NULL || pIdSrc->pszName != NULL);
    PREFAST_DEBUGCHK(pIdDst != NULL);
    DEBUGCHK(cbDst >= sizeof(DEVICEID));

    // make sure we have enough room
    dwSize = DeviceIdSize(pIdSrc);
    if(cbDst < dwSize) {
        PMLOGMSG(ZONE_WARN, 
            (_T("%s: destination buffer size %d too small, %d bytes needed\r\n"),
            pszFname, cbDst, dwSize));
        pIdDst = NULL;
    } else {
        memset(pIdDst, 0, dwSize);
        pIdDst->pGuid = NULL;
        pIdDst->pszName = NULL;
        __try {
            // copy class field if present
            DWORD dwNameOffset = sizeof(*pIdDst);
            
            if(pIdSrc->pGuid != NULL) {
                LPGUID pGuid = (LPGUID) ((LPBYTE) pIdDst + sizeof(*pIdDst));
                *pGuid = *pIdSrc->pGuid;
                pIdDst->pGuid = pGuid;
                dwNameOffset += sizeof(GUID);
            }

            // copy name field if present
            if(pIdSrc->pszName != NULL) {
                LPTSTR pszName = (LPTSTR) ((LPBYTE) pIdDst + dwNameOffset);
                _tcscpy(pszName, pIdSrc->pszName);
                pIdDst->pszName = pszName;
            }            
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            PMLOGMSG(ZONE_WARN, (_T("%s: exception during member copy\r\n"), 
                pszFname));
            pIdDst = NULL;
        }
    }

    return pIdDst;
}

// This routine creates a copy of a device id and returns its pointer.  If
// there's a problem it returns NULL.
PDEVICEID
DeviceIdClone(PDEVICEID pId)
{
    PDEVICEID pIdClone;
    DWORD dwSize;
    SETFNAME(_T("DeviceIdClone"));

    DEBUGCHK(pId != NULL);

    // calculate the amount of memory to allocate
    dwSize = DeviceIdSize(pId);
    if(dwSize != 0) {
        pIdClone = (PDEVICEID) PmAlloc(dwSize);
    } else {
        pIdClone = NULL;
    }

    // copy data into the buffer
    if(pIdClone != NULL) {
        PDEVICEID pIdTmp = DeviceIdCloneIntoBuffer(pId, pIdClone, dwSize);
        if(pIdTmp == NULL) {
            PmFree(pIdClone);
            pIdClone = NULL;
        }
    }

    DEBUGCHK(pIdClone == NULL || pIdClone->pGuid != NULL || pIdClone->pszName != NULL);

    return pIdClone;
}

// This routine parses a device name string and returns a pointer to an
// allocated device ID.  The caller is responsible for freeing this pointer
// when they are done with it.  If the name cannot be parsed, this routine
// returns NULL.
PDEVICEID
DeviceIdParseNameString(LPCTSTR pszDevName, DWORD dwFlags)
{
    GUID idClass;
    LPCTSTR pszName = pszDevName;
    PDEVICEID pdi = NULL;
    DEVICEID di;
    SETFNAME(_T("DeviceIdParseNameString"));

    // sanity check the parameters
    PREFAST_DEBUGCHK(pszName != NULL);
    if((dwFlags & POWER_NAME) == 0) {
        PMLOGMSG(ZONE_WARN, 
            (_T("%s: don't know how to parse name without POWER_NAME flag\r\n"), 
            pszFname));
        return NULL;
    }

    // is the name prefixed with a guid?
    __try {
        BOOL fOk = TRUE;
        if(*pszName == '{') {
            if(!ConvertStringToGuid(pszName, &idClass)) {
                // indicate an error later
                pszName = NULL;
            } else {
                // skip the guid, we are guaranteed to find the
                // closing brace because the guid parsed ok.
                while(*pszName != '}' && *pszName != 0) {
                    pszName++;
                }

                // skip the trailing curly-brace and backslash
                if(*pszName != '}') {
                    fOk = FALSE;
                } else {
                    pszName++;      // go to the char after the close brace
                    if(*pszName != _T('\\')) {
                        fOk = FALSE;
                    } else {
                        pszName++;
                    }
                }
            }
        } else {
            // use the default device class
            idClass = idGenericPMDeviceClass;
        }
        
        // do we have a name?
        if(fOk && pszName != NULL && *pszName != 0) {
            DWORD dwLen;
            DWORD cchName;

            // yes, save it and convert to lower case
            di.pGuid = &idClass;
            cchName = _tcslen(pszName);
            dwLen = (cchName + 1) * sizeof(*pszName);
            __try {
                DWORD dwIndex;
                LPTSTR pszTempName = (LPTSTR) _alloca(dwLen);
                for(dwIndex = 0; dwIndex < cchName; dwIndex++) {
                    pszTempName[dwIndex] = _totlower(pszName[dwIndex]);
                }
                pszTempName[cchName] = 0;
                di.pszName = pszTempName;
            } 
            __except(EXCEPTION_EXECUTE_HANDLER) {
                di.pszName = NULL;
            }
            if(di.pszName != NULL) {
                pdi = DeviceIdClone(&di);
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        PMLOGMSG(ZONE_WARN, (_T("%s: exception parsing name\r\n"), pszFname));
    }

    PMLOGMSG(pdi == NULL && ZONE_WARN, (_T("%s: returning 0x%08x\r\n"), pszFname, pdi));
    return pdi;
}
    
// This routine deallocates a device ID that was created with DeviceIdClone().
// Do not call this routine on a device ID that may have statically allocated
// members.
VOID
DeviceIdDestroy(PDEVICEID pId)
{
    if(pId) {
        PmFree(pId);
    }
}

// This routine returns TRUE if two device IDs match exactly, FALSE otherwise.
// Note that exact matches are expected to be case sensitive, so "FOO1:" will
// not match "foo1:".
BOOL
DeviceIdsAreEqual(PDEVICEID pId1, PDEVICEID pId2)
{
    BOOL fEqual = TRUE;

    PREFAST_DEBUGCHK(pId1 != NULL);
    PREFAST_DEBUGCHK(pId2 != NULL);
    // check name strings first
    if(pId1->pszName == NULL) {
        if(pId2->pszName != NULL) {
            fEqual = FALSE;
        }
    } else {
        if(pId2->pszName == NULL || _tcscmp(pId1->pszName, pId2->pszName) != 0) {
            fEqual = FALSE;
        }
    }

    // check guid values
    if(fEqual == TRUE) {
        if(pId1->pGuid == NULL) {
            if(pId2->pGuid != NULL) {
                fEqual = FALSE;
            }
        } else {
            if(pId2->pGuid == NULL || *pId1->pGuid != *pId2->pGuid) {
                fEqual = FALSE;
            }
        }
    }

    return fEqual;
}

// ------------------------ DEVICE STATE MANAGEMENT ------------------------

// this routine creates a data structure describing a device.  Its initial
// reference count is one.
PDEVICE_STATE
DeviceStateCreate(LPCTSTR pszName)
{
    BOOL fOk = FALSE;
    PDEVICE_STATE pds = NULL;
    SETFNAME(_T("DeviceStateCreate"));
    
    PREFAST_DEBUGCHK(pszName != NULL);

    __try {
        DWORD dwSize = sizeof(*pds) + ((_tcslen(pszName) + 1) * sizeof(pszName[0]));
        pds = (PDEVICE_STATE) PmAlloc(dwSize);
        if(pds != NULL) {
            LPTSTR pszNameCopy = (LPTSTR) ((LPBYTE) pds + sizeof(*pds));
            memset(pds, 0, sizeof(*pds));
            _tcscpy(pszNameCopy, pszName);
            pds->pszName = pszNameCopy;
            pds->curDx = D0;
            pds->floorDx = PwrDeviceUnspecified;
            pds->ceilingDx = PwrDeviceUnspecified;
            pds->setDx = PwrDeviceUnspecified;
            pds->lastReqDx = D0;
            pds->actualDx = D0;
            pds->pendingDx = PwrDeviceUnspecified;
            pds->dwNumPending = 0;
            pds->pParent = NULL;
            pds->dwRefCount = 1;
            pds->hDevice = INVALID_HANDLE_VALUE;
            pds->pListHead = NULL;
            pds->pNext = NULL;
            pds->pPrev = NULL;
            PMLOGMSG(ZONE_REFCNT, (_T("%s: created 0x%08x (name '%s'), refcnt is %d\r\n"),
                pszFname, pds, pszName, pds->dwRefCount));
            fOk = TRUE;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        PMLOGMSG(ZONE_WARN, (_T("%s: exception initializing structure\r\n"), pszFname));
    }

    if(!fOk) {
        if(pds != NULL) {
            PmFree(pds);
            pds = NULL;
        }
    }

    PMLOGMSG(pds == NULL && ZONE_WARN, (_T("%s: couldn't create structure for '%s'\r\n"),
        pszFname, pszName));
    return pds;
}

// this routine deallocates a device state structure.  It returns TRUE if 
// successful, FALSE otherwise.
BOOL
DeviceStateDestroy(PDEVICE_STATE pds)
{
    BOOL fOk = TRUE;
    SETFNAME(_T("DeviceStateDestroy"));

    PMLOGMSG(ZONE_REFCNT, (_T("%s: deleting 0x%08x\r\n"), pszFname, pds));
    if(pds != NULL) {
        if(pds->pParent != NULL) DeviceStateDecRef(pds->pParent);
        if(pds->hDevice != INVALID_HANDLE_VALUE) {
            PREFAST_DEBUGCHK(pds->pInterface != NULL);
            pds->pInterface->pfnCloseDevice(pds->hDevice);
        }
        PmFree(pds);
    }

    return fOk;
}

// this routine updates a device's reference count
VOID
DeviceStateAddRef(PDEVICE_STATE pds)
{
    SETFNAME(_T("DeviceStateAddRef"));

    PREFAST_DEBUGCHK(pds != NULL);

    PMLOCK();
    pds->dwRefCount++;
    PMLOGMSG(ZONE_REFCNT, (_T("%s: refcnt for 0x%08x set to %d\r\n"), pszFname,
        pds, pds->dwRefCount));
    PMUNLOCK();
}

// this routine decrements a device's reference count and frees the device's
// state if the reference count goes to zero.
VOID
DeviceStateDecRef(PDEVICE_STATE pds)
{
    BOOL fDestroy = FALSE;
    SETFNAME(_T("DeviceStateDecRef"));

    PREFAST_DEBUGCHK(pds != NULL);

    PMLOCK();
    DEBUGCHK(pds->dwRefCount > 0);
    pds->dwRefCount--;
    PMLOGMSG(ZONE_REFCNT, (_T("%s: refcnt for 0x%08x set to %d\r\n"), pszFname,
        pds, pds->dwRefCount));
    if(pds->dwRefCount == 0) {
        DEBUGCHK(pds->pListHead == NULL && pds->pNext == NULL && pds->pPrev == NULL);
        fDestroy = TRUE;
    }
    PMUNLOCK();

    // is it time to get rid of the device?
    if(fDestroy) {
        DeviceStateDestroy(pds);
    }
}

// this routine adds a device state structure to a list and increments its
// reference count.  It returns TRUE if successful, FALSE otherwise.
BOOL
DeviceStateAddList(PDEVICE_LIST pdl, PDEVICE_STATE pdsDevice)
{
    BOOL fOk = TRUE;
    SETFNAME(_T("DeviceStateAddList"));

    PREFAST_DEBUGCHK(pdl != NULL);
    PREFAST_DEBUGCHK(pdsDevice != NULL);
    DEBUGCHK(pdsDevice->pNext == NULL && pdsDevice->pPrev == NULL);
    DEBUGCHK(pdsDevice->dwRefCount == 1);
    DEBUGCHK(pdsDevice->pListHead == NULL);

    PMLOGMSG(ZONE_REFCNT | ZONE_DEVICE, 
        (_T("%s: adding 0x%08x ('%s') to list 0x%08x\r\n"),
        pszFname, pdsDevice, pdsDevice->pszName, pdl));

    // put the new device at the head of the list
    PMLOCK();
    pdsDevice->pListHead = pdl;
    pdsDevice->pNext = pdl->pList;
    pdsDevice->pPrev = NULL;
    if(pdl->pList != NULL) {
        pdl->pList->pPrev = pdsDevice;
    }
    pdl->pList = pdsDevice;

    // copy interface method pointers from the device class
    DEBUGCHK(pdl->pInterface != NULL);
    pdsDevice->pInterface = pdl->pInterface;

    DeviceStateAddRef(pdsDevice);
    PMUNLOCK();

    return fOk;
}

// this routine removes a device state structure from a list and decrements
// its reference count.
BOOL
DeviceStateRemList(PDEVICE_STATE pds)
{
    BOOL fOk = TRUE;
    SETFNAME(_T("DeviceStateRemList"));
    PREFAST_DEBUGCHK(pds != NULL);
    PREFAST_DEBUGCHK(pds->pListHead != NULL);
    
    PMLOGMSG(ZONE_REFCNT | ZONE_DEVICE, 
        (_T("%s: removing 0x%08x ('%s') from list 0x%08x\r\n"),
        pszFname, pds, pds->pszName, pds->pListHead));

    PMLOCK();

    // are we at the head of the list?
    if(pds->pPrev != NULL) {
        pds->pPrev->pNext = pds->pNext;
    } else {
        // not that if pNext == NULL && pPrev == NULL we are the only list
        // member and the assignment to the list head will set it to NULL (meaning
        // empty).
        if(pds->pNext != NULL) {
            pds->pNext->pPrev = NULL;
        }
        pds->pListHead->pList = pds->pNext;
    }
    
    // are we at the tail of the list?
    if(pds->pNext != NULL) {
        pds->pNext->pPrev = pds->pPrev;
    } else {
        if(pds->pPrev != NULL) {
            pds->pPrev->pNext = NULL;
        }
    }

    // clear list pointers
    pds->pNext = NULL;
    pds->pPrev = NULL;
    pds->pListHead = NULL;

    // decrement the reference count
    DeviceStateDecRef(pds);
        
    PMUNLOCK();

    return fOk;
}

// This routine looks for a device on a list.  If it finds the device, it
// increments its reference counter and returns a pointer to it.  The caller
// should decrement the reference counter when it is done with the pointer.
// Note that the search is case sensitive.
PDEVICE_STATE
DeviceStateFindList(PDEVICE_LIST pdl, LPCTSTR pszName)
{
    PDEVICE_STATE pds = NULL;
    SETFNAME(_T("DeviceStateFindList"));

    PMLOCK();

    __try {
        // look for a match
        for(pds = pdl->pList; pds != NULL; pds = pds->pNext) {
            if(_tcscmp(pds->pszName, pszName) == 0) {
                // increment the reference count and exit
                DeviceStateAddRef(pds);
                break;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        PMLOGMSG(ZONE_WARN, (_T("%s: exception searching list\r\n"), pszFname));
        pds = NULL;
    }

    PMUNLOCK();

    return pds;
}


// ------------------------ DEVICE POWER RESTRICTION MANAGEMENT ----------

// this routine creates a data structure describing a device.  Its initial
// reference count is one.
PDEVICE_POWER_RESTRICTION
PowerRestrictionCreate(PDEVICEID pDeviceId, HANDLE hOwner, CEDEVICE_POWER_STATE Dx, 
                       LPCTSTR pszSystemState, DWORD dwFlags)
{
    PDEVICE_POWER_RESTRICTION pdpr = NULL;
    BOOL fOk = TRUE;
    SETFNAME(_T("PowerRestrictionCreate"));

    __try {
        LPTSTR pszSystemStateCopy;
        PDEVICEID pDeviceIdCopy = NULL;
        DWORD dwDeviceIdSize = DeviceIdSize(pDeviceId);
        DWORD cchSystemState;

        // allocate resources
        DWORD dwSize = sizeof(*pdpr) + dwDeviceIdSize;
        if(pszSystemState != NULL) {
            cchSystemState = _tcslen(pszSystemState);
            dwSize += ((cchSystemState + 1) * sizeof(pszSystemState[0]));
        }
        pdpr = (PDEVICE_POWER_RESTRICTION) PmAlloc(dwSize);
        if(pdpr == NULL) {
            fOk = FALSE;
        }

        if(fOk) {
            // copy the device ID
            if(pDeviceId != NULL) {
                pDeviceIdCopy = (PDEVICEID) ((LPBYTE) pdpr + sizeof(*pdpr));
                pDeviceIdCopy = DeviceIdCloneIntoBuffer(pDeviceId, pDeviceIdCopy, dwDeviceIdSize);
                if(pDeviceIdCopy == NULL) {
                    fOk = FALSE;
                }
            }
        }

        if(fOk) {
            // copy the system power state name, if it's present
            if(pszSystemState != NULL) {
                // get a pointer to the aggregated buffer we just allocated
                PREFAST_SUPPRESS(12009,"The Size has been pre-calculated");
                pszSystemStateCopy = (LPTSTR) ((LPBYTE) pdpr + sizeof(*pdpr) + dwDeviceIdSize);
                
                // Do the copy here because once it's assigned to the structure
                // our allocated buffer will be treated as const.  Convert to lowercase
                // so that we avoid doing localized comparisons when accessing the requirement
                // data.
                for (DWORD dwIndex = 0; dwIndex < cchSystemState; dwIndex++) {
                    pszSystemStateCopy[dwIndex] = _totlower(pszSystemState[dwIndex]);
                }
                pszSystemStateCopy[cchSystemState] = 0;
            } else {
                pszSystemStateCopy = NULL;
            }
        }
        
        // initialize the structure
        if(fOk) { 
            memset(pdpr, 0, sizeof(*pdpr));
            pdpr->pDeviceId = pDeviceIdCopy;
            pdpr->hOwner = hOwner;
            pdpr->devDx = Dx;
            pdpr->pszSystemState = pszSystemStateCopy;
            pdpr->dwFlags = dwFlags;
            pdpr->pNext = NULL;
            pdpr->pPrev = NULL;

        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        PMLOGMSG(ZONE_WARN, (_T("%s: exception during structure initialization\r\n"),
            pszFname));
        fOk = FALSE;
    }

    // clean up if necessary
    if(!fOk) {
        if(pdpr != NULL) {
            PmFree(pdpr);
            pdpr = NULL;
        }
    }

    PMLOGMSG(pdpr == NULL && ZONE_WARN, (_T("%s: couldn't create structure for '%s'\r\n"),
        pszFname, pDeviceId->pszName != NULL ? pDeviceId->pszName : _T("<NULL>")));
    return pdpr;
}

// this routine deallocates a device state structure.  It returns TRUE if 
// successful, FALSE otherwise.
BOOL
PowerRestrictionDestroy(PDEVICE_POWER_RESTRICTION pdpr)
{
    BOOL fOk = TRUE;

    if(pdpr != NULL) {
        PmFree(pdpr);
    }

    return fOk;
}

// this routine adds a device state structure to a list and increments its
// reference count.  It returns TRUE if successful, FALSE otherwise.
BOOL
PowerRestrictionAddList(PPDEVICE_POWER_RESTRICTION ppListHead, PDEVICE_POWER_RESTRICTION pdpr)
{
    BOOL fOk = TRUE;

    PREFAST_DEBUGCHK(ppListHead != NULL);
    PREFAST_DEBUGCHK(pdpr != NULL);
    DEBUGCHK(pdpr->pNext == NULL && pdpr->pPrev == NULL);
    DEBUGCHK(ppListHead != NULL);

    // put the new device at the head of the list
    PMLOCK();
    pdpr->pNext = *ppListHead;
    pdpr->pPrev = NULL;
    if(*ppListHead != NULL) {
        (*ppListHead)->pPrev = pdpr;
    }
    *ppListHead = pdpr;
    PMUNLOCK();

    return fOk;
}

// this routine removes a device state structure from a list and decrements
// its reference count.
BOOL
PowerRestrictionRemList(PPDEVICE_POWER_RESTRICTION ppListHead, PDEVICE_POWER_RESTRICTION pdpr)
{
    BOOL fOk = TRUE;

    PREFAST_DEBUGCHK(ppListHead != NULL);

    PMLOCK();

    // are we at the head of the list?
    if(pdpr->pPrev != NULL) {
        pdpr->pPrev->pNext = pdpr->pNext;
    } else {
        // not that if pNext == NULL && pPrev == NULL we are the only list
        // member and the assignment to the list head will set it to NULL (meaning
        // empty).
        if(pdpr->pNext != NULL) {
            pdpr->pNext->pPrev = NULL;
        }
        *ppListHead = pdpr->pNext;
    }
    
    // are we at the tail of the list?
    if(pdpr->pNext != NULL) {
        pdpr->pNext->pPrev = pdpr->pPrev;
    } else {
        if(pdpr->pPrev != NULL) {
            pdpr->pPrev->pNext = NULL;
        }
    }

    // clear list pointers
    pdpr->pNext = NULL;
    pdpr->pPrev = NULL;

    // delete the entry
    PMUNLOCK();

    return fOk;
}

// This routine looks for a power restriction on a list.  If it finds the entry, it
// increments its reference counter and returns a pointer to it.  The caller
// should decrement the reference counter when it is done with the pointer.  The system
// power state name is expected to be lower-case, to avoid doing localized string comparisons.
PDEVICE_POWER_RESTRICTION
PowerRestrictionFindList(PDEVICE_POWER_RESTRICTION pList, 
                         PDEVICEID pDeviceId, LPCTSTR pszSystemState)
{
    PDEVICE_POWER_RESTRICTION pdpr;
    SETFNAME(_T("PowerRestrictionFindList"));

    PMLOCK();

    __try {
        // look for a match
        for(pdpr = pList; pdpr != NULL; pdpr = pdpr->pNext) {
            if(DeviceIdsAreEqual(pdpr->pDeviceId, pDeviceId)
                && (pszSystemState == NULL && pdpr->pszSystemState == NULL
                || (pszSystemState != NULL && pdpr->pszSystemState != NULL 
                    && _tcscmp(pszSystemState, pdpr->pszSystemState) == 0))) {
                break;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        PMLOGMSG(ZONE_WARN, (_T("%s: exception while searching list\r\n"),
            pszFname));
        pdpr = NULL;
    }

    PMUNLOCK();

    return pdpr;
}

// This routine checks that a particular power restriction exists on a list.
// It returns TRUE if it does, FALSE if it can't be matched.  The comparison
// is based on pointer value, not on the contents of the pointer.
BOOL
PowerRestrictionCheckList(PDEVICE_POWER_RESTRICTION pList, 
                         PDEVICE_POWER_RESTRICTION pdpr)
{
    PDEVICE_POWER_RESTRICTION pdprTraveller;
    BOOL fFound = FALSE;

    PMLOCK();

    // look for a match
    for(pdprTraveller = pList; pdprTraveller != NULL; pdprTraveller = pdprTraveller->pNext) {
        if(pdprTraveller == pdpr) {
            fFound = TRUE;
            break;
        }
    }

    PMUNLOCK();

    return fFound;
}

// ------------------------ POWER NOTIFICATION MANAGEMENT ----------

// This routine creates a data structure describing a power notification.
// It returns a pointer to the structure, or NULL if there's an
// error.
PPOWER_NOTIFICATION
PowerNotificationCreate(HANDLE hMsgQ, HANDLE hOwner)
{
    SETFNAME(_T("PowerNotificationCreate"));

    // allocate resources
    PPOWER_NOTIFICATION ppn = (PPOWER_NOTIFICATION) PmAlloc(sizeof(*ppn));
    if(ppn != NULL ) {
        // initialize the structure
        memset(ppn, 0, sizeof(*ppn));
        ppn->hOwner = hOwner;
        ppn->pNext = NULL;
        ppn->pPrev = NULL;

        // now try to open the message queue
        MSGQUEUEOPTIONS msgopts;
        memset(&msgopts, 0, sizeof(msgopts));
        msgopts.dwSize = sizeof(MSGQUEUEOPTIONS);
        msgopts.bReadAccess = FALSE;
        ppn->hMsgQ = OpenMsgQueue(hOwner, hMsgQ, &msgopts);
        if(ppn->hMsgQ == NULL) {
            PMLOGMSG(ZONE_WARN, 
                (_T("%s: OpenMsgQueue() failed %d (owner 0x%08x, source q 0x%08x)\r\n"),
                pszFname, GetLastError(), hOwner, hMsgQ));
            PmFree(ppn);
            ppn = NULL;
        }
    }

    PMLOGMSG(ppn == NULL && ZONE_WARN, (_T("%s: couldn't create structure\r\n"), pszFname));
    return ppn;
}

// this routine deallocates a device state structure.  It returns TRUE if 
// successful, FALSE otherwise.
BOOL
PowerNotificationDestroy(PPOWER_NOTIFICATION ppn)
{
    BOOL fOk = TRUE;

    if(ppn != NULL) {
        if(ppn->hMsgQ != NULL) CloseMsgQueue(ppn->hMsgQ);
        PmFree(ppn);
    }

    return fOk;
}

// This routine adds a power notification to a list.
BOOL
PowerNotificationAddList(PPPOWER_NOTIFICATION ppListHead, PPOWER_NOTIFICATION ppn)
{
    BOOL fOk = TRUE;

    PREFAST_DEBUGCHK(ppListHead != NULL);
    PREFAST_DEBUGCHK(ppn != NULL);
    DEBUGCHK(ppn->pNext == NULL && ppn->pPrev == NULL);
    DEBUGCHK(ppListHead != NULL);

    // put the new device at the head of the list
    PMLOCK();
    ppn->pNext = *ppListHead;
    ppn->pPrev = NULL;
    if(*ppListHead != NULL) {
        (*ppListHead)->pPrev = ppn;
    }
    *ppListHead = ppn;
    PMUNLOCK();

    return fOk;
}

// this routine removes a power notification from a list and deletes it.
BOOL
PowerNotificationRemList(PPPOWER_NOTIFICATION ppListHead, PPOWER_NOTIFICATION ppn)
{
    BOOL fOk = TRUE;

    PREFAST_DEBUGCHK(ppListHead != NULL);

    PMLOCK();
    // We have to check that ppn is in the ppListHead before we can delete it.
    PPOWER_NOTIFICATION pCurNotification = *ppListHead;
    
    while (pCurNotification) {
        if (pCurNotification == ppn)
            break;
        else
            pCurNotification = pCurNotification->pNext ;
    };
    if (pCurNotification) {
        
        // are we at the head of the list?
        if(ppn->pPrev != NULL) {
            ppn->pPrev->pNext = ppn->pNext;
        } else {
            // not that if pNext == NULL && pPrev == NULL we are the only list
            // member and the assignment to the list head will set it to NULL (meaning
            // empty).
            if(ppn->pNext != NULL) {
                ppn->pNext->pPrev = NULL;
            }
            *ppListHead = ppn->pNext;
        }
        
        // are we at the tail of the list?
        if(ppn->pNext != NULL) {
            ppn->pNext->pPrev = ppn->pPrev;
        } else {
            if(ppn->pPrev != NULL) {
                ppn->pPrev->pNext = NULL;
            }
        }

        // clear list pointers
        ppn->pNext = NULL;
        ppn->pPrev = NULL;

        // delete the structure and its members
        PowerNotificationDestroy(ppn);
    }
    else
        fOk = FALSE ;
    
    PMUNLOCK();

    return fOk;
}

// ------------------------ DEVICE LISTS ---------------------

// This routine creates a device list entry and returns a pointer to it, or NULL
// if there's an error.
PDEVICE_LIST
DeviceListCreate(LPCGUID pClass)
{
    PDEVICE_LIST pdl = NULL;
    HANDLE hMsgQ = NULL;
    MSGQUEUEOPTIONS msgOptions;
    SETFNAME(_T("DeviceListCreate"));

    PMLOGMSG(ZONE_INIT || ZONE_DEVICE, 
        (_T("%s: creating list entry for class %08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x\r\n"), 
        pszFname, pClass->Data1, pClass->Data2, pClass->Data3,
        (pClass->Data4[0] << 8) + pClass->Data4[1], pClass->Data4[2], pClass->Data4[3], 
        pClass->Data4[4], pClass->Data4[5], pClass->Data4[6], pClass->Data4[7]));
    
    // allocate structure members
    pdl = (PDEVICE_LIST) PmAlloc(sizeof(*pdl) + sizeof(GUID));

    // create message queues
    memset(&msgOptions, 0, sizeof(msgOptions));
    msgOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
    msgOptions.dwFlags = 0;
    msgOptions.cbMaxMessage = PNP_QUEUE_SIZE;
    msgOptions.bReadAccess = TRUE;
    hMsgQ = CreateMsgQueue(NULL, &msgOptions);
    if(hMsgQ == NULL) {
        DWORD dwStatus = GetLastError();
        PMLOGMSG(ZONE_ERROR, (_T("%s: CreateMsgQueue() failed %d\r\n"), pszFname, dwStatus));
    }

    // initialize the structure
    if(pdl != NULL && hMsgQ != NULL) {
        LPGUID pGuid = (LPGUID) ((LPBYTE) pdl + sizeof(*pdl));
        memset(pdl, 0, sizeof(pdl));
        *pGuid = *pClass;
        pdl->pGuid = pGuid;
        pdl->hMsgQ = hMsgQ;
        pdl->hnClass = NULL;
        pdl->pInterface = NULL;
        pdl->pList = NULL;
        pdl->pNext = NULL;
    } else {
        // some kind of error, release resources
        if(pdl != NULL) PmFree(pdl);
        if(hMsgQ != NULL) CloseMsgQueue(hMsgQ);
        pdl = NULL;
    }

    PMLOGMSG(pdl == NULL && ZONE_WARN, (_T("%s: returning 0x%08x\r\n"), pszFname, pdl));
    return pdl;
}

// This routine frees a device list entry.  The list must be empty
// when this routine is called, and it must not point to another
// list (ie, pNext must be null).
VOID
DeviceListDestroy(PDEVICE_LIST pdl)
{
    SETFNAME(_T("DeviceListDestroy"));

    PMLOGMSG(ZONE_INIT || ZONE_DEVICE, 
        (_T("%s: deleting list entry for class %08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x\r\n"), 
        pszFname, pdl->pGuid->Data1, pdl->pGuid->Data2, pdl->pGuid->Data3,
        (pdl->pGuid->Data4[0] << 8) + pdl->pGuid->Data4[1], pdl->pGuid->Data4[2], pdl->pGuid->Data4[3], 
        pdl->pGuid->Data4[4], pdl->pGuid->Data4[5], pdl->pGuid->Data4[6], pdl->pGuid->Data4[7]));
    
    DEBUGCHK(pdl->pNext == NULL);
    DEBUGCHK(pdl->pList == NULL);
    DEBUGCHK(pdl->hnClass == NULL);

    if(pdl->hMsgQ) CloseMsgQueue(pdl->hMsgQ);
    PmFree(pdl);
}

// this routine determines to which device list a particular device class
// corresponds
PDEVICE_LIST
GetDeviceListFromClass(LPCGUID guidDevClass)
{
    PDEVICE_LIST pdl;
    SETFNAME(_T("GetDeviceListFromClass"));

    PREFAST_DEBUGCHK(guidDevClass != NULL);

    // look for a match
    __try {
        for(pdl = gpDeviceLists; pdl != NULL; pdl = pdl->pNext) {
			RETAILMSG(0, (_T("%s: %08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x\r\n"), 
				pszFname, pdl->pGuid->Data1, pdl->pGuid->Data2, pdl->pGuid->Data3,
				(pdl->pGuid->Data4[0] << 8) + pdl->pGuid->Data4[1], pdl->pGuid->Data4[2], pdl->pGuid->Data4[3], 
				pdl->pGuid->Data4[4], pdl->pGuid->Data4[5], pdl->pGuid->Data4[6], pdl->pGuid->Data4[7]));

            if(*pdl->pGuid == *guidDevClass) {
                break;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        PMLOGMSG(TRUE, (_T("%s: exception accessing guidDevClass 0x%08x\r\n"),
            pszFname, guidDevClass));
        pdl = NULL;
    }

    return pdl;
}

// ------------------------ SYSTEM POWER STATES ---------------------

// this routine allocates a structure describing a system power state.
PSYSTEM_POWER_STATE
SystemPowerStateCreate(LPCTSTR pszName)
{
    PSYSTEM_POWER_STATE psps;
    SETFNAME(_T("SystemPowerStateCreate"));

    PREFAST_DEBUGCHK(pszName != NULL);

    DWORD cchName;

    __try {
        cchName = _tcslen(pszName);
        DWORD dwSize = sizeof(*psps) + ((cchName + 1) * sizeof(pszName[0]));
        psps = (PSYSTEM_POWER_STATE) PmAlloc(dwSize);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        PMLOGMSG(ZONE_WARN, (_T("%s: exception calculating size\r\n"), pszFname));
        psps = NULL;
    }

    if(psps != NULL) {
        memset(psps, 0, sizeof(*psps));
        __try {
            LPTSTR pszNameCopy = (LPTSTR) ((LPBYTE) psps + sizeof(*psps));
            // Copy the name, converting to lower case.  We want to avoid doing
            // case-sensitive comparisons in the platform code while file system
            // access may be disabled.
            for (DWORD dwIndex = 0; dwIndex < cchName; dwIndex++) {
                pszNameCopy[dwIndex] = _totlower(pszName[dwIndex]);
            }
            pszNameCopy[cchName] = 0;
            psps->pszName = pszNameCopy;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            PMLOGMSG(ZONE_WARN, (_T("%s: exception copying name\r\n"),
                pszFname));
            PmFree(psps);
            psps = NULL;
        }
    }

    PMLOGMSG(psps == NULL && ZONE_WARN, 
        (_T("%s: couldn't allocate structure\r\n"), pszFname));

    return psps;
}

// this routine deallocates a system power state structure
BOOL
SystemPowerStateDestroy(PSYSTEM_POWER_STATE psps)
{
    BOOL fOk = TRUE;

    if(psps != NULL) {
        PmFree(psps);
    }

    return fOk;
}

// ------------------------ ACTIVITY TIMER LISTS ---------------------

// This routine allocates and initializes an activity timer structure.
// The structure has a name and several events, one of which is a public
// (ie named) event whose name is derived from the activity timer name.
// It also contains an array of wake sources that are associated with
// the activity timer.  This is constructed from a REG_MULTI_SZ value,
// each component string of which is a wake source number represented in
// decimal.  The wake source values must be compatible with 
// IOCTL_HAL_GET_WAKE_SOURCE.
PACTIVITY_TIMER
ActivityTimerCreate(LPTSTR pszName, DWORD dwTimeout, LPTSTR pszSources)
{
    PACTIVITY_TIMER pat = NULL;
    DWORD dwWakeSources, dwSize;
    LPTSTR pszSource;
    TCHAR szPath[MAX_PATH];
    BOOL fOk = TRUE;
    SETFNAME(_T("ActivityTimerCreate"));

    PREFAST_DEBUGCHK(pszName != NULL );
    PREFAST_DEBUGCHK(pszSources != NULL);
    DEBUGCHK(*pszName != 0);

    // calculate the number of wake sources and allocate an array for them
    dwWakeSources = 0;
    pszSource = pszSources;
    while(*pszSource != 0) {
        pszSource += _tcslen(pszSource) + 1;
        dwWakeSources++;
    }

    // allocate and initialize structure and members
    dwSize = sizeof(*pat) + ((dwWakeSources + 1) * sizeof(pat->pdwWakeSources[0])) 
        + ((_tcslen(pszName) + 1) * sizeof(*pszName));
    pat = (PACTIVITY_TIMER) PmAlloc(dwSize);
    if(pat != NULL) {
        memset(pat, 0, sizeof(*pat));
        pat->dwTimeout = dwTimeout;                 // timeout in milliseconds
        if(pat->dwTimeout == 0) {
            pat->dwTimeLeft = INFINITE;
        } else {
            pat->dwTimeLeft = pat->dwTimeout;
        }
        pat->pdwWakeSources = (LPDWORD) ((LPBYTE) pat + sizeof(*pat));
        pat->pszName = (LPTSTR) ((LPBYTE) pat->pdwWakeSources 
            + ((dwWakeSources + 1) * sizeof(pat->pdwWakeSources[0])));

        // create the reset event
        if(fOk &&
            _sntprintf(szPath, dim(szPath), _T("PowerManager/ActivityTimer/%s"), pszName) == (-1)) {
            PMLOGMSG(ZONE_WARN, (_T("%s: couldn't format event name for activity timer '%s'\r\n"),
                pszFname, pszName));
            fOk = FALSE;
        } else {
            szPath[dim(szPath)-1] = 0;
            pat->hevReset = CreateEvent(NULL, FALSE, FALSE, szPath);
        }

        // create the event indicating activity
        if(fOk &&
            _sntprintf(szPath, dim(szPath), _T("PowerManager/%s_Active"), pszName) == (-1)) {
            PMLOGMSG(ZONE_WARN, (_T("%s: couldn't format event name for activity timer '%s'\r\n"),
                pszFname, pszName));
            fOk = FALSE;
        } else {
            szPath[dim(szPath)-1] = 0;
            pat->hevActive = CreateEvent(NULL, TRUE, FALSE, szPath);
        }

        // create the event indicating inactivity
        if(fOk &&
            _sntprintf(szPath, dim(szPath), _T("PowerManager/%s_Inactive"), pszName) == (-1)) {
            PMLOGMSG(ZONE_WARN, (_T("%s: couldn't format event name for activity timer '%s'\r\n"),
                pszFname, pszName));
            fOk = FALSE;
        } else {
            szPath[dim(szPath)-1] = 0;
            pat->hevInactive = CreateEvent(NULL, TRUE, FALSE, szPath);
        }

        // did we get everything we needed?
        if(pat->hevReset == NULL || pat->hevActive == NULL || pat->hevInactive == NULL) {
            PMLOGMSG(ZONE_WARN, (_T("%s: resource allocation failed for '%s'\r\n"), 
                pszFname, pszName));
            fOk = FALSE;
        } else {
            DWORD dwIndex;

            // init activity name
            _tcscpy(pat->pszName, pszName);

            // init wake sources
            dwIndex = 0;
            pszSource = pszSources;
            while(fOk && *pszSource != 0 && dwIndex < dwWakeSources) {
                LPTSTR pszEnd;
                pat->pdwWakeSources[dwIndex] = _tcstol(pszSource, &pszEnd, 0);
                if(*pszEnd != 0) {
                    PMLOGMSG(ZONE_WARN, 
                        (_T("%s: can't parse wake source '%s' for timer '%s'\r\n"),
                        pszFname, pszSource, pszName));
                    fOk = FALSE;
                }
                pszSource += _tcslen(pszSource) + 1;
                dwIndex++;
            }

            // terminate the list with SYSINTR_NOP
            pat->pdwWakeSources[dwIndex] = SYSINTR_NOP;
        }
    }


    // clean up if necessary
    if(!fOk && pat != NULL) {
        if(pat->hevReset != NULL) CloseHandle(pat->hevReset);
        if(pat->hevActive != NULL) CloseHandle(pat->hevActive);
        if(pat->hevInactive != NULL) CloseHandle(pat->hevInactive);
        PmFree(pat);
        pat = NULL;
    }

    PMLOGMSG(ZONE_INIT || (pat == NULL && ZONE_WARN), 
        (_T("%s: ActivityTimerCreate('%s') returning 0x%08x\r\n"), pszFname, pszName));
    return pat;
}

// This routine deallocates an activity timer structure and frees any resources
// associated with it.
BOOL
ActivityTimerDestroy(PACTIVITY_TIMER pat)
{
    BOOL fOk = TRUE;

    // free the structure and its members
    if(pat != NULL) {
        if(pat->hevReset != NULL) CloseHandle(pat->hevReset);
        if(pat->hevActive != NULL) CloseHandle(pat->hevActive);
        if(pat->hevInactive != NULL) CloseHandle(pat->hevInactive);
        PmFree(pat);
    }

    return fOk;
}

// This routine looks up an activity timer by name and returns a pointer to
// its corresponding structure.  If it can't find the timer, it returns NULL.
PACTIVITY_TIMER
ActivityTimerFindByName(LPCTSTR pszName)
{
    PACTIVITY_TIMER pat = NULL;
    SETFNAME(_T("ActivityTimerFindByName"));

    PMLOCK();
    if(gppActivityTimers != NULL) {
        DWORD dwTimerIndex = 0;
        while((pat = gppActivityTimers[dwTimerIndex]) != NULL) {
            if(_tcscmp(pat->pszName, pszName) == 0) {
                break;
            }
            dwTimerIndex++;
        }
    }
    PMUNLOCK();

    PMLOGMSG(ZONE_TIMERS, (_T("%s: search for '%s' returning 0x%08x\r\n"), pszFname,
        pszName, pat));

    return pat;
}

// This routine looks up the activity timer associcated with a wake source
// and returns a pointer to its corresponding structure.  If it can't 
// find the timer, it returns NULL.
PACTIVITY_TIMER
ActivityTimerFindByWakeSource(DWORD dwWakeSource)
{
    PACTIVITY_TIMER pat = NULL;
    BOOL fDone = FALSE;
    SETFNAME(_T("ActivityTimerFindByWakeSource"));

    PMLOCK();
    if(gppActivityTimers != NULL) {
        DWORD dwTimerIndex = 0;
        while(!fDone && (pat = gppActivityTimers[dwTimerIndex]) != NULL) {
            DWORD dwSourceIndex = 0;
            while(pat->pdwWakeSources[dwSourceIndex] != 0) {
                if(pat->pdwWakeSources[dwSourceIndex] == dwWakeSource) {
                    fDone = TRUE;
                    break;
                }
                dwSourceIndex++;
            }
            dwTimerIndex++;
        }
    }
    PMUNLOCK();

    PMLOGMSG(ZONE_TIMERS, (_T("%s: search for %d (0x%x) returning 0x%08x\r\n"), pszFname,
        dwWakeSource, dwWakeSource, pat));

    return pat;
}
