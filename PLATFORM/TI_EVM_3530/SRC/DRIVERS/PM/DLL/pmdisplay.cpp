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
// This module implements the APIs the power manager uses to send power
// requests to display devices.  It's defined in the MDD, but OEMs can
// override it or remove it from their image by not referencing 
// gDisplayInterface in their code -- that will keep it from being pulled 
// in by the linker.
//

#include <pmimpl.h>
#include <wingdi.h>

// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)

typedef HDC (WINAPI *PFN_CreateDCW)(LPCWSTR, LPCWSTR , LPCWSTR , CONST DEVMODEW *);
typedef BOOL (WINAPI *PFN_DeleteDC)(HDC);
typedef int (WINAPI *PFN_ExtEscape)(HDC, int, int, LPCSTR, int, LPSTR);

// global function pointers
static PFN_CreateDCW gpfnCreateDCW = NULL;
static PFN_DeleteDC gpfnDeleteDC = NULL;
static PFN_ExtEscape gpfnExtEscape = NULL;
static HANDLE ghevGwesAPISetReady = NULL;
static BOOL gfGwesReady = FALSE;

// This routine arranges for initialization of the API used to communicate
// with devices of this class.  It may be called more than once.  If 
// successful it returns TRUE.  Otherwise it returns FALSE.
static BOOL
InitDisplayDeviceInterface(VOID)
{
    BOOL fOk = FALSE;
    SETFNAME(_T("InitDisplayDeviceInterface"));

    if(gpfnCreateDCW == NULL) {
        HMODULE hmCore = (HMODULE) LoadLibrary(_T("coredll.dll"));
        PMLOGMSG(ZONE_WARN && hmCore == NULL, (_T("%s: LoadLibrary() failed %d\r\n"),
            pszFname, GetLastError()));
        if(hmCore != NULL) {
            // get procedure addresses
            gpfnCreateDCW = (PFN_CreateDCW) GetProcAddress(hmCore, _T("CreateDCW"));
            gpfnDeleteDC = (PFN_DeleteDC) GetProcAddress(hmCore, _T("DeleteDC"));
            gpfnExtEscape = (PFN_ExtEscape) GetProcAddress(hmCore, _T("ExtEscape"));
            if(gpfnCreateDCW == NULL || gpfnExtEscape == NULL || gpfnDeleteDC == NULL) {
                PMLOGMSG(ZONE_WARN, 
                    (_T("%s: can't get APIs: pfnCreateDCW 0x%08x, pfnDeleteDC 0x%08x, pfnExtEscape 0x%08x\r\n"),
                    pszFname, gpfnCreateDCW, gpfnDeleteDC, gpfnExtEscape));
                gpfnCreateDCW = NULL;
                gpfnDeleteDC = NULL;
                gpfnExtEscape = NULL;
                FreeLibrary(hmCore);
            }
        }
    }

    // do we have the endpoints we need?
    if(gpfnCreateDCW != NULL && gpfnExtEscape != NULL && gpfnDeleteDC != NULL) {
        // yes, get a handle to the GWES API set event
        if(ghevGwesAPISetReady == NULL) {
            ghevGwesAPISetReady = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T("SYSTEM/GweApiSetReady"));
            DEBUGCHK(ghevGwesAPISetReady != NULL);      // shouldn't happen since we have GWES APIs
            if(ghevGwesAPISetReady == NULL) {
                PMLOGMSG(ZONE_WARN, (_T("%s: can't open GWES API set ready event handle\r\n"),
                    pszFname));
            }
        }
    }

    // did we get all the resources we'll need to eventually access
    // devices of this type?
    if(ghevGwesAPISetReady != NULL) {
        // yes, return a good status
        fOk = TRUE;
    }
    
    PMLOGMSG(ZONE_INIT || (ZONE_WARN && !fOk), (_T("%s: returning %d\r\n"), pszFname, fOk));
    return fOk;
}

// This routine opens a handle to a given device, or to the parent device
// that is its proxy.  The return value is the device's handle, or 
// INVALID_HANDLE_VALUE if there's an error.
static HANDLE 
OpenDisplayDevice(PDEVICE_STATE pds)
{
    SETFNAME(_T("OpenDisplayDevice"));
    HANDLE hRet = NULL;

    // Get a handle to the client device.  The client's name will generally be "DISPLAY" for 
    // the default display, but it could be anything that CreateDC supports.
    PREFAST_DEBUGCHK(gpfnCreateDCW != NULL);
    PREFAST_DEBUGCHK(gpfnDeleteDC != NULL);
    PREFAST_DEBUGCHK(gpfnExtEscape != NULL);

    // Make sure GWES is ready for us to access this device
    if(!gfGwesReady) {
        // We need to wait for GWES to finish -- since this is happening
        // at boot time, we shouldn't have to wait for long.
        DEBUGCHK(ghevGwesAPISetReady != NULL);
        PMLOGMSG(ZONE_INIT, (_T("%s: waiting for GWES to initialize\r\n"),
            pszFname));
        DWORD dwStatus = WaitForSingleObject(ghevGwesAPISetReady, INFINITE);
        if(dwStatus != WAIT_OBJECT_0) {
            PMLOGMSG(ZONE_WARN, 
                (_T("%s: WaitForSingleObject() returned %d (error %d) on GWES ready event\r\n"),
                pszFname, GetLastError()));
        } else {
            PMLOGMSG(ZONE_INIT, (_T("%s: GWES API sets are ready\r\n"), pszFname));
            // we don't need to access the event, but keep the handle value non-NULL
            // in case more than one display registers itself
            CloseHandle(ghevGwesAPISetReady);
            gfGwesReady = TRUE;
        }
    }

    if(gfGwesReady) {
        hRet = gpfnCreateDCW(pds->pszName, NULL, NULL, NULL);
        if(hRet == NULL) {
            PMLOGMSG(ZONE_WARN || ZONE_IOCTL, (_T("%s: CreateDC('%s') failed %d (0x%08x)\r\n"), pszFname,
                pds->pszName, GetLastError(), GetLastError()));
            hRet = INVALID_HANDLE_VALUE;
        } else {
            // determine whether the display driver really supports the PM IOCTLs
            DWORD dwIoctl[] = { IOCTL_POWER_CAPABILITIES, IOCTL_POWER_SET, IOCTL_POWER_GET };
            int i;
            for(i = 0; i < dim(dwIoctl); i++) {
                if(gpfnExtEscape((HDC) hRet, QUERYESCSUPPORT, sizeof(dwIoctl[i]), (LPCSTR) &dwIoctl[i], 0, NULL) <= 0) {
                    break;
                }
            }
            if(i < dim(dwIoctl)) {
                PMLOGMSG(ZONE_WARN, (_T("%s: '%s' doesn't support all power manager control codes\r\n"),
                    pszFname, pds->pszName));
                gpfnDeleteDC((HDC) hRet);
                hRet = INVALID_HANDLE_VALUE;
            }
        }
    }
    
    PMLOGMSG(ZONE_DEVICE || ZONE_IOCTL, (_T("%s: handle to '%s' is 0x%08x\r\n"), \
        pszFname, pds->pszName, hRet));

    return hRet;
}

// This routine closes a handle opened with OpenDisplayDevice().  It returns TRUE
// if successful, FALSE if there's a problem.
static BOOL
CloseDisplayDevice(HANDLE hDevice)
{
    SETFNAME(_T("CloseDisplayDevice"));

    DEBUGCHK(gpfnCreateDCW != NULL);
    DEBUGCHK(gpfnDeleteDC != NULL);
    DEBUGCHK(gpfnExtEscape != NULL);
    DEBUGCHK(hDevice != NULL && hDevice != INVALID_HANDLE_VALUE);
    DEBUGCHK(gfGwesReady);

    PMLOGMSG(ZONE_DEVICE || ZONE_IOCTL, (_T("%s: closing 0x%08x\r\n"), pszFname, hDevice));
    BOOL fOk = gpfnDeleteDC((HDC) hDevice);
    PMLOGMSG(!fOk && (ZONE_WARN || ZONE_IOCTL), (_T("%s: DeleteDC(0x%08x) failed %d (0x%08x)\r\n"), pszFname,
        hDevice, GetLastError(), GetLastError()));
    return fOk;
}

// This routine issues a request to a device.  It returns TRUE if successful,
// FALSE if there's a problem.
static BOOL
RequestDisplayDevice(HANDLE hDevice, DWORD dwRequest, LPVOID pInBuf, DWORD dwInSize, 
              LPVOID pOutBuf, DWORD dwOutSize, LPDWORD pdwBytesRet)
{
    BOOL fOk = FALSE;
    int iStatus;
    SETFNAME(_T("RequestDisplayDevice"));

    DEBUGCHK(gpfnCreateDCW != NULL);
    PREFAST_DEBUGCHK(gpfnDeleteDC != NULL);
    PREFAST_DEBUGCHK(gpfnExtEscape != NULL);
    DEBUGCHK(hDevice != NULL && hDevice != INVALID_HANDLE_VALUE);
    DEBUGCHK(gfGwesReady);

    __try {
        PMLOGMSG(ZONE_IOCTL, (_T("%s: calling ExtEscape(0x%08x) w/ request %d ('%s')\r\n"),
            pszFname, hDevice, dwRequest, 
            dwRequest == IOCTL_POWER_CAPABILITIES ? _T("IOCTL_POWER_CAPABILITIES") 
            : dwRequest == IOCTL_POWER_GET ? _T("IOCTL_POWER_GET") 
            : dwRequest == IOCTL_POWER_SET ? _T("IOCTL_POWER_SET") 
            : dwRequest == IOCTL_POWER_QUERY ? _T("IOCTL_POWER_QUERY") 
            : dwRequest == IOCTL_REGISTER_POWER_RELATIONSHIP ? _T("IOCTL_REGISTER_POWER_RELATIONSHIP")
            : _T("<UNKNOWN>")));
        iStatus = gpfnExtEscape((HDC) hDevice, dwRequest, dwInSize, (LPCSTR) pInBuf, 
            dwOutSize, (LPSTR) pOutBuf);
        if(iStatus > 0) {
            fOk = TRUE;
            if(pdwBytesRet != NULL) {
                // need to fill this in with something
                *pdwBytesRet = dwOutSize;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        PMLOGMSG(ZONE_WARN || ZONE_IOCTL, (_T("%s: exception in DeviceIoControl(%d)\r\n"), pszFname,
            dwRequest));
        fOk = FALSE;
    }

    PMLOGMSG(!fOk && (ZONE_WARN || ZONE_IOCTL), 
        (_T("%s: DeviceIoControl(%d) to 0x%08x failed %d (0x%08x)\r\n"), pszFname,
        dwRequest, hDevice, GetLastError(), GetLastError()));
    return fOk;
}

// create a data structure to encapsulate this interface
EXTERN_C DEVICE_INTERFACE gDisplayInterface = {
    InitDisplayDeviceInterface,
    OpenDisplayDevice,
    CloseDisplayDevice,
    RequestDisplayDevice
};

