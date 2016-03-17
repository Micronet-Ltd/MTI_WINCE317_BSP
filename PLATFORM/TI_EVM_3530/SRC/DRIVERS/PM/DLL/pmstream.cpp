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
// requests to stream devices.  It's defined in the MDD, but OEMs can
// override it or remove it from their image by not referencing 
// gDisplayInterface in their code -- that will keep it from being pulled 
// in by the linker.
//

#include <pmimpl.h>
#include <omap_pmext.h>
#include <devload.h>
// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)

// This routine arranges for initialization of the API used to communicate
// with devices of this class.  It may be called more than once.  If 
// successful it returns TRUE.  Otherwise it returns FALSE.
static BOOL
InitStreamDeviceInterface(VOID)
{
    // nothing special to do for stream devices
    return TRUE;
}

// This routine opens a handle to a given device, or to the parent device
// that is its proxy.  The return value is the device's handle, or 
// INVALID_HANDLE_VALUE if there's an error.
static HANDLE 
OpenStreamDevice(PDEVICE_STATE pds)
{
	HANDLE hRet;

    PDEVICE_STATE pdsReal = pds;
    SETFNAME(_T("OpenStreamDevice"));

    // determine what device to actually open
    if(pds->pParent != NULL) {
        pdsReal = pds->pParent;
    }

    // get a handle to the client
#ifdef DEBUG
	    hRet = CreateFile(pdsReal->pszName, 0, 
        FILE_SHARE_READ|FILE_SHARE_WRITE|DEVACCESS_PM_MODE, NULL, OPEN_EXISTING, 0, NULL);
#else
	__try
	{
     hRet = CreateFile(pdsReal->pszName, DEVACCESS_BUSNAMESPACE, //DEVACCESS_BUSNAMESPACE, DEVACCESS_PMEXT_MODE
        FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	}
	__except(1)
	{
	 RETAILMSG(1, (TEXT("+OpenStreamDevice('%s'), MS DebugCheck issue workaround\r\n"), pdsReal->pszName));
	}
#endif

    if(hRet == INVALID_HANDLE_VALUE) {
        PMLOGMSG(ZONE_WARN || ZONE_IOCTL, (_T("%s: OpenFile('%s') failed %d (0x%08x)\r\n"), pszFname,
            pdsReal->pszName, GetLastError(), GetLastError()));
    }
    PMLOGMSG(ZONE_DEVICE || ZONE_IOCTL, (_T("%s: handle to '%s' on behalf of '%s' is 0x%08x\r\n"), \
        pszFname, pdsReal->pszName, pds->pszName, hRet));

    return hRet;
}

// This routine closes a handle opened with OpenStreamDevice().  It returns TRUE
// if successful, FALSE if there's a problem.
static BOOL
CloseStreamDevice(HANDLE hDevice)
{
    SETFNAME(_T("CloseStreamDevice"));

    PMLOGMSG(ZONE_DEVICE || ZONE_IOCTL, (_T("%s: closing 0x%08x\r\n"), pszFname, hDevice));
    BOOL fOk = CloseHandle(hDevice);
    PMLOGMSG(!fOk && (ZONE_WARN || ZONE_IOCTL), (_T("%s: CloseHandle(0x%08x) failed %d (0x%08x)\r\n"), pszFname,
        hDevice, GetLastError(), GetLastError()));
    return fOk;
}

// This routine issues a request to a device.  It returns TRUE if successful,
// FALSE if there's a problem.
static BOOL
RequestStreamDevice(HANDLE hDevice, DWORD dwRequest, LPVOID pInBuf, DWORD dwInSize, 
              LPVOID pOutBuf, DWORD dwOutSize, LPDWORD pdwBytesRet)
{
    BOOL fOk;
    SETFNAME(_T("RequestStreamDevice"));

    DEBUGCHK(hDevice != INVALID_HANDLE_VALUE);

    __try {
        PMLOGMSG(ZONE_IOCTL, (_T("%s: calling DeviceIoControl(0x%08x) w/ request %d ('%s')\r\n"),
            pszFname, hDevice, dwRequest, 
            dwRequest == IOCTL_POWER_CAPABILITIES ? _T("IOCTL_POWER_CAPABILITIES") 
            : dwRequest == IOCTL_POWER_GET ? _T("IOCTL_POWER_GET") 
            : dwRequest == IOCTL_POWER_SET ? _T("IOCTL_POWER_SET") 
            : dwRequest == IOCTL_POWER_QUERY ? _T("IOCTL_POWER_QUERY") 
            : dwRequest == IOCTL_REGISTER_POWER_RELATIONSHIP ? _T("IOCTL_REGISTER_POWER_RELATIONSHIP")
            : _T("<UNKNOWN>")));
        fOk = DeviceIoControl(hDevice, dwRequest, pInBuf, dwInSize, 
            pOutBuf, dwOutSize, pdwBytesRet, NULL);
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
EXTERN_C DEVICE_INTERFACE gStreamInterface = {
    InitStreamDeviceInterface,
    OpenStreamDevice,
    CloseStreamDevice,
    RequestStreamDevice
};

