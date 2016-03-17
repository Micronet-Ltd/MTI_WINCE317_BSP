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
// This module contains a sample battery driver for windows CE.  It
// exposes its interface to the system via a set of IOCTLs which device.exe
// invokes on its behalf when one of the battery APIs are invoked.  The
// following APIs are modeled in this way:
//
//      GetSystemPowerStatusEx
//      GetSystemPowerStatusEx2
//      BatteryGetLifeTimeInfo
//      BatteryDrvrSupportsChangeNotification
//      BatteryDrvrGetLevels
//      BatteryNotifyOfTimeChange
//
// When it initializes, this driver is responsible for setting the battery
// API ready event to indicate it is ready to receive IOCTLs in response
// to these APIs.
// 
// This example driver comes with a stubbed-out implementation of the
// low-level battery interfaces.  OEMs that wish to produce their own
// battery driver can write a module containing the low-level entry points
// in their platform and simply link with the library containing this module.
// The linker will bring in the rest of the driver as an MDD.
//
// The low-level interfaces necessary that must be overridden to produce
// a platform-specific PDD are:
//
//      BatteryPDDInitialize
//      BatteryPDDDeinitialize
//      BatteryPDDGetStatus
//      BatteryPDDGetLevels
//      BatteryPDDSupportsChangeNotification
//      BatteryPDDPowerHandler
//      BatteryPDDResume
//

#define BATTERY_DATA_DEF    // global battery driver variables defined here

#include <battimpl.h>
#include <nkintr.h>
#include <devload.h>
#include <pmpolicy.h>

// 5 Seconds for average delay of 2.5 seconds
#define DEF_BATTERYPOLLTIMEOUT          (5*1000)         // in milliseconds
#define DEF_BATTERYCRITICALPOLLTIMEOUT  (2*1000)

// this structure contains startup parameters for the battery monitor thread
typedef struct _batteryContext_tag {
    DWORD       dwPollTimeout;
	DWORD       dwCritPollTimeout;
    int         iPriority;
    SYSTEM_POWER_STATUS_EX2 st;
} BATTERY_CONTEXT, * PBATTERY_CONTEXT;

// global variables
CRITICAL_SECTION gcsBattery;
PFN_BATTERY_PDD_IOCONTROL gpfnBatteryPddIOControl;
HANDLE ghevResume;
HANDLE ghtBattery;
BOOL gfExiting;
BATTERY_CONTEXT gBatteryContext;
DWORD gdwPddBufferSize;
CRITICAL_SECTION gcsPddBuffer;
BYTE *gpPddBuffer;
BYTE *gpPddBufferTemp;

// This thread handles resume notifications for the battery driver.  It will
// also poll the battery for status updates at regular intervals, depending
// on the poll interval configuration in the registry.
DWORD WINAPI 
BatteryThreadProc(PVOID pvParam)
{
    BOOL fDone = FALSE;
    BOOL fOk;
	DWORD timeout;
    PBATTERY_CONTEXT pbc = (PBATTERY_CONTEXT) pvParam;
    SETFNAME(_T("Battery: BatteryThreadProc"));
    
    DEBUGCHK(ghevResume != NULL);
    PREFAST_DEBUGCHK(pbc != NULL);
    
    // set my thread priority
    DEBUGMSG(ZONE_RESUME, (_T("%s: thread priority is %d, timeout is %u, ID is 0x%08x\r\n"), 
        pszFname, pbc->iPriority, pbc->dwPollTimeout, GetCurrentThreadId()));
	RETAILMSG(1, (_T("BatteryThreadProc: thread priority is %d, timeout is %u, ID is 0x%08x\r\n"), 
         pbc->iPriority, pbc->dwPollTimeout, GetCurrentThreadId()));
    CeSetThreadPriority(GetCurrentThread(), pbc->iPriority);
    
    // tell the Power Manager to initialize its battery state
    fOk = PowerPolicyNotify(PPN_POWERCHANGE, 0);
    DEBUGCHK(fOk);

	timeout = pbc->dwPollTimeout;
    // wait for the system to resume
    while(!fDone) {
        DWORD dwStatus = WaitForSingleObject(ghevResume, timeout);
		RETAILMSG(0, (_T("BatteryThreadProc: WaitForSingleObject() timeout = %d dwStatus =  %d\r\n"),timeout,dwStatus));
        switch(dwStatus) {
        case WAIT_TIMEOUT:
            {
                SYSTEM_POWER_STATUS_EX2 tmpst;
                
                // update battery status information
                dwStatus = BatteryAPIGetSystemPowerStatusEx2(&tmpst, sizeof(tmpst), TRUE);
                DEBUGCHK(dwStatus == sizeof(pbc->st));
                
                // has anything changed?
                if(memcmp(&tmpst, &pbc->st, sizeof(pbc->st)) != 0) {
                    // yes, update our global variable and notify the power manager
                    pbc->st = tmpst;
                    fOk = PowerPolicyNotify(PPN_POWERCHANGE, 0);
                    DEBUGCHK(fOk);
                }

				timeout = (tmpst.BatteryFlag & BATTERY_FLAG_CRITICAL)?pbc->dwCritPollTimeout : pbc->dwPollTimeout;
				RETAILMSG(0, (_T("BatteryThreadProc: WaitForSingleObject() time = %d\r\n"),GetTickCount()));
            }
            break;
        case WAIT_OBJECT_0:	// system has resumed
            // GetTickCount() stops incrementing during suspends, so we can
            // stop and restart the battery stopwatch during resume.
            DEBUGMSG(ZONE_RESUME, (_T("%s: system has resumed\r\n"), pszFname));
            BatteryAPIStopwatch(FALSE, FALSE);
            BatteryAPIStopwatch(TRUE, FALSE);
            BatteryPDDResume();
            break;
        default:
            DEBUGMSG(ZONE_RESUME, (_T("%s: WaitForSingleObject() returned %d, error %d\r\n"), pszFname,
                dwStatus, GetLastError()));
            fDone = TRUE;
            break;
        }
        
        // check whether we're supposed to exit
        if(gfExiting) fDone = TRUE;
    }
    
    DEBUGMSG(ZONE_RESUME, (_T("%s: exiting\r\n"), pszFname));
    return 0;
}

// This routine initializes the battery driver.  Since it should only be
// loaded once, it returns an error if this routine is called more than
// once.
DWORD
Init(
         PVOID Context
         )
{
    DWORD dwHandle = 0;     // assume failure
    SETFNAME(_T("BattDrvr: Init"));
    
    DEBUGMSG(ZONE_INIT, (_T("%s: invoked w/ context 0x%08x\r\n"), pszFname, Context));

    // have we already been loaded?
    if(ghevResume == NULL) {
        // get a handle to our API event
        HANDLE hevReady = OpenEvent(EVENT_ALL_ACCESS, FALSE, BATTERY_API_EVENT_NAME);
        if(hevReady == NULL) {
            DEBUGMSG(ZONE_ERROR || ZONE_INIT, (_T("%s: fatal error: can't open API event\r\n"), pszFname));
            goto done;
        }
        
        // no, initialize global variables
        InitializeCriticalSection(&gcsBattery);
        InitializeCriticalSection(&gcsPddBuffer);
        gpfnBatteryPddIOControl = NULL;     // must be initialized by the PDD
        gdwPddBufferSize = sizeof(SYSTEM_POWER_STATUS_EX2);
        gpPddBuffer = NULL;
        gpPddBufferTemp = NULL;
        ghevResume = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(ghevResume == NULL) {
            DEBUGMSG(ZONE_ERROR || ZONE_INIT, (_T("%s: CreateEvent() failed\r\n"), pszFname));
        } else {
            if(!BatteryPDDInitialize(Context)) {
                DEBUGMSG(ZONE_ERROR || ZONE_INIT, (_T("%s: BatteryPDDInitialize() failed\r\n"), pszFname));
            } else {
                HKEY hk;
                DWORD dwLen;
                
                // init defaults
                gBatteryContext.dwPollTimeout     = DEF_BATTERYPOLLTIMEOUT;
				gBatteryContext.dwCritPollTimeout = DEF_BATTERYCRITICALPOLLTIMEOUT;
                gBatteryContext.iPriority = 249;		// THREAD_PRIORITY_HIGHEST
                memset(&gBatteryContext.st, 0xFF, sizeof(gBatteryContext.st));
                
                // get my thread priority configuration
                hk = OpenDeviceKey(Context);
                if(hk != NULL) {
                    DWORD dwValue, dwType, dwSize, dwStatus;
                    
                    // get the thread priority
                    dwSize = sizeof(dwValue);
                    dwStatus = RegQueryValueEx(hk, _T("PollPriority256"), NULL, &dwType, (LPBYTE) &dwValue, &dwSize);
                    if(dwStatus == ERROR_SUCCESS && dwType == REG_DWORD) {
                        gBatteryContext.iPriority = (INT) dwValue;
                    }
                    
                    // get the polling interval
                    dwSize = sizeof(dwValue);
                    dwStatus = RegQueryValueEx(hk, _T("PollInterval"), NULL, &dwType, (LPBYTE) &dwValue, &dwSize);
                    if(dwStatus == ERROR_SUCCESS && dwType == REG_DWORD) {
                        gBatteryContext.dwPollTimeout = dwValue;
                    }


					// get the polling interval when in battery critical
                    dwSize = sizeof(dwValue);
                    dwStatus = RegQueryValueEx(hk, _T("CriticalPollInterval"), NULL, &dwType, (LPBYTE) &dwValue, &dwSize);
                    if(dwStatus == ERROR_SUCCESS && dwType == REG_DWORD) {
                        gBatteryContext.dwCritPollTimeout = dwValue;
                    }

                    
                    // get the Pdd buffer size
                    dwSize = sizeof(dwValue);
                    dwStatus = RegQueryValueEx(hk, _T("PddBufferSize"), NULL, &dwType, (LPBYTE) &dwValue, &dwSize);
                    if(dwStatus == ERROR_SUCCESS && dwType == REG_DWORD) {
                        if( dwValue > gdwPddBufferSize )
                        {
                            gdwPddBufferSize = dwValue;
                        }
                    }
                    
                    RegCloseKey(hk);
                }

                gpPddBuffer = (BYTE*)LocalAlloc( LPTR, gdwPddBufferSize );
                if( gpPddBuffer == NULL )
                {
                    DEBUGMSG(ZONE_ERROR || ZONE_INIT, (_T("%s: BatteryPDDInitialize() failed to allocate the Pdd buffer.\r\n"), pszFname));
                    goto done;
                }

                gpPddBufferTemp = (BYTE*)LocalAlloc( LPTR, gdwPddBufferSize );
                if( gpPddBufferTemp == NULL )
                {
                    DEBUGMSG(ZONE_ERROR || ZONE_INIT, (_T("%s: BatteryPDDInitialize() failed to allocate the Pdd temp buffer.\r\n"), pszFname));
                    goto done;
                }

                // initialize battery update variables
                dwLen = BatteryAPIGetSystemPowerStatusEx2 (&gBatteryContext.st, sizeof(gBatteryContext.st), TRUE);
                DEBUGCHK(dwLen == sizeof(gBatteryContext.st));
                
                // start the battery monitor/resume thread
                ghtBattery = CreateThread(NULL, 0, BatteryThreadProc, &gBatteryContext, 0, NULL);
                if(ghtBattery == NULL) {
                    DEBUGMSG(ZONE_ERROR || ZONE_INIT, (_T("%s: CreateThread() failed %d\r\n"), pszFname,
                        GetLastError()));
                } else {
                    // notify the world that we're up and running
                    SetEvent(hevReady);
                    CloseHandle(hevReady);
                    
                    // return success
                    dwHandle = 1;
                }
            }
        }
        
done:
        // clean up if something went wrong
        if(dwHandle == 0) {
            if(gpPddBuffer != NULL) {
                LocalFree( gpPddBuffer );
                gpPddBuffer = NULL;
            }
            if(gpPddBufferTemp != NULL) {
                LocalFree( gpPddBufferTemp );
                gpPddBufferTemp = NULL;
            }
            if(ghtBattery != NULL) {
                DEBUGCHK(ghevResume != NULL);
                gfExiting = TRUE;
                SetEvent(ghevResume);
                WaitForSingleObject(ghtBattery, INFINITE);
                CloseHandle(ghtBattery);
                ghtBattery = NULL;
            }
            if(ghevResume != NULL) {
                CloseHandle(ghevResume);
                ghevResume = NULL;
            }
            DeleteCriticalSection(&gcsBattery);
            DeleteCriticalSection(&gcsPddBuffer);
        }
    }
    
    DEBUGMSG(ZONE_INIT, (_T("%s: returning %d\r\n"), pszFname, dwHandle));
    return dwHandle;
}


BOOL
Deinit(
           DWORD dwContext
           )
{
    DEBUGMSG(ZONE_INIT, (_T("Battery: Deinit: invoked w/ context 0x%08x\r\n"),
        dwContext));
    
    DEBUGCHK(dwContext == 1);
    DEBUGCHK(ghevResume != NULL);
    
    // notify the PDD
    BatteryPDDDeinitialize();
    
    // clean up global variables
    if(ghtBattery != NULL) {
        gfExiting = TRUE;
        SetEvent(ghevResume);
        WaitForSingleObject(ghtBattery, INFINITE);
        CloseHandle(ghtBattery);
        ghtBattery = NULL;
    }
    if(ghevResume != NULL) {
        CloseHandle(ghevResume);
        ghevResume = NULL;
    }
    if(gpPddBuffer != NULL) {
        LocalFree( gpPddBuffer );
        gpPddBuffer = NULL;
    }
    if(gpPddBufferTemp != NULL) {
        LocalFree( gpPddBufferTemp );
        gpPddBufferTemp = NULL;
    }
    DeleteCriticalSection(&gcsBattery);
    DeleteCriticalSection(&gcsPddBuffer);
    
    DEBUGMSG(ZONE_INIT, (_T("Battery: Deinit: all done\r\n")));
    return TRUE;
}


BOOL
IOControl(
              DWORD  dwContext,
              DWORD  Ioctl,
              PUCHAR pInBuf,
              DWORD  InBufLen, 
              PUCHAR pOutBuf,
              DWORD  OutBufLen,
              PDWORD pdwBytesTransferred
              )
{
    DWORD  dwErr = ERROR_INVALID_PARAMETER;
    BOOL   bRc = FALSE;
    SETFNAME(_T("IOControl"));

    DEBUGMSG(ZONE_FUNCTION, (_T("%s: IOCTL:0x%x, InBuf:0x%x, InBufLen:%d, OutBuf:0x%x, OutBufLen:0x%x)\r\n"),
        pszFname, Ioctl, pInBuf, InBufLen, pOutBuf, OutBufLen));

    switch (Ioctl) {
    case IOCTL_BATTERY_GETSYSTEMPOWERSTATUSEX2:
        // sanity check parameters
        if(pOutBuf != NULL && OutBufLen >= sizeof(SYSTEM_POWER_STATUS_EX2)
            && pInBuf != NULL && InBufLen == sizeof(BOOL) && pdwBytesTransferred != NULL) {
            BOOL fOk = FALSE;
            BOOL fForce;

            // get the force parameter
            __try {
                fForce = *((BOOL *) pInBuf);
                fOk = TRUE;
            } 
            __except(EXCEPTION_EXECUTE_HANDLER) {
                DEBUGMSG(ZONE_WARN, 
                    (_T("%s: exception reading fForce in IOCTL_BATTERY_GETSYSTEMPOWERSTATUSEX\r\n"), pszFname));
            }

            // did we get the force parameter?
            if(fOk) {
                DWORD dwStatus = 0;
                DEBUGCHK(fForce == TRUE || fForce == FALSE);

                // get the data
                __try {
                    dwStatus = BatteryAPIGetSystemPowerStatusEx2((PSYSTEM_POWER_STATUS_EX2) pOutBuf, OutBufLen, fForce);
                }
                __except(EXCEPTION_EXECUTE_HANDLER) {
                    DEBUGMSG(ZONE_WARN, 
                        (_T("%s: exception reading status in IOCTL_BATTERY_GETSYSTEMPOWERSTATUSEX2\r\n"), pszFname));
                    dwErr = ERROR_GEN_FAILURE;
                }

                // pass back return values
                __try {
                    *pdwBytesTransferred = dwStatus;
                    if(dwStatus == 0) {
                        dwErr = GetLastError();
                    } else {
                        dwErr = ERROR_SUCCESS;
                    }
                }
                __except(EXCEPTION_EXECUTE_HANDLER) {
                    DEBUGMSG(ZONE_WARN, 
                        (_T("%s: exception writing status in IOCTL_BATTERY_GETSYSTEMPOWERSTATUSEX2\r\n"), pszFname));
                }
            }
        }
        break;
        
    case IOCTL_BATTERY_GETSYSTEMPOWERSTATUSEX:
        // sanity check parameters
        if(pOutBuf != NULL && OutBufLen == sizeof(SYSTEM_POWER_STATUS_EX)
            && pInBuf != NULL && InBufLen == sizeof(BOOL) && pdwBytesTransferred != NULL) {
            BOOL fOk = FALSE;
            BOOL fForce;

            // get the force parameter
            __try {
                fForce = *((BOOL *) pInBuf);
                fOk = TRUE;
            } 
            __except(EXCEPTION_EXECUTE_HANDLER) {
                DEBUGMSG(ZONE_WARN, 
                    (_T("%s: exception reading fForce in IOCTL_BATTERY_GETSYSTEMPOWERSTATUSEX\r\n"), pszFname));
            }

            // did we get the force parameter?
            if(fOk) {
                DEBUGCHK(fForce == TRUE || fForce == FALSE);

                // get the data            
                __try {
                    fOk = BatteryAPIGetSystemPowerStatusEx((PSYSTEM_POWER_STATUS_EX) pOutBuf, fForce);
                }
                __except(EXCEPTION_EXECUTE_HANDLER) {
                    DEBUGMSG(ZONE_WARN, 
                        (_T("%s: exception reading status in IOCTL_BATTERY_GETSYSTEMPOWERSTATUSEX\r\n"), pszFname));
                    dwErr = ERROR_GEN_FAILURE;
                }

                // pass back return values
                __try {
                    if(fOk) {
                        *pdwBytesTransferred = sizeof(SYSTEM_POWER_STATUS_EX);
                        dwErr = ERROR_SUCCESS;
                    } else {
                        *pdwBytesTransferred = 0;
                        dwErr = GetLastError();
                    }
                }
                __except(EXCEPTION_EXECUTE_HANDLER) {
                    DEBUGMSG(ZONE_WARN, 
                        (_T("%s: exception writing status in IOCTL_BATTERY_GETSYSTEMPOWERSTATUSEX\r\n"), pszFname));
                }
            }
        }
        break;
        
    case IOCTL_BATTERY_GETLIFETIMEINFO:
        // sanity check parameters
        if(pOutBuf != NULL && OutBufLen == sizeof(BATTERYLIFETIMEINFO)) {
            SYSTEMTIME stLastChange = { 0 };
            DWORD cmsCpuUsage = 0, cmsPreviousCpuUsage = 0;
            
            DEBUGCHK(pInBuf == NULL);
            DEBUGCHK(InBufLen == 0);

            // assume a good return value from this function, even though it's void
            SetLastError(ERROR_SUCCESS);

            // get the data
            LOCKBATTERY();
            BatteryAPIGetLifeTimeInfo(&stLastChange, &cmsCpuUsage, &cmsPreviousCpuUsage);
            UNLOCKBATTERY();

            // pass back return values
            __try {
                PBATTERYLIFETIMEINFO pblti = (PBATTERYLIFETIMEINFO) pOutBuf;
                pblti->stLastChange = stLastChange;
                pblti->cmsCpuUsage = cmsCpuUsage;
                pblti->cmsPreviousCpuUsage = cmsPreviousCpuUsage;
                if( pdwBytesTransferred != NULL )
                {
                    *pdwBytesTransferred = sizeof(PBATTERYLIFETIMEINFO);
                }
                dwErr = GetLastError();
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                DEBUGMSG(ZONE_WARN, 
                    (_T("%s: exception in IOCTL_BATTERY_GETLIFETIMEINFO\r\n"), pszFname));
                dwErr = ERROR_INVALID_PARAMETER;
            }
        }
        break;
        
    case IOCTL_BATTERY_GETLEVELS:
        // sanity check parameters
        if(pOutBuf != NULL && OutBufLen == sizeof(LONG) && pdwBytesTransferred != NULL) {
            DWORD dwLevels;
            
            DEBUGCHK(pInBuf == NULL);
            DEBUGCHK(InBufLen == 0);

            // get the data
            LOCKBATTERY();
            dwLevels = BatteryPDDGetLevels();
            UNLOCKBATTERY();

            // pass back return values
            __try {
                *((PLONG) pOutBuf) = (LONG) dwLevels;
                *pdwBytesTransferred = sizeof(LONG);
                dwErr = ERROR_SUCCESS;
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                DEBUGMSG(ZONE_WARN, 
                    (_T("%s: exception in IOCTL_BATTERY_GETLEVELS\r\n"), pszFname));
                dwErr = ERROR_INVALID_PARAMETER;
            }
        }
        break;
        
    case IOCTL_BATTERY_SUPPORTSCHANGENOTIFICATION:
        // sanity check parameters
        if(pOutBuf != NULL && OutBufLen == sizeof(BOOL) && pdwBytesTransferred != NULL) {
            BOOL fSupportsChange;
            
            DEBUGCHK(pInBuf == NULL);
            DEBUGCHK(InBufLen == 0);

            // get the data
            LOCKBATTERY();
            fSupportsChange = BatteryPDDSupportsChangeNotification();
            UNLOCKBATTERY();

            // pass back return values
            __try {
                *((PBOOL) pOutBuf) = fSupportsChange;
                *pdwBytesTransferred = sizeof(fSupportsChange);
                dwErr = ERROR_SUCCESS;
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                DEBUGMSG(ZONE_WARN, 
                    (_T("%s: exception in IOCTL_BATTERY_SUPPORTSCHANGENOTIFICATION\r\n"), pszFname));
                dwErr = ERROR_INVALID_PARAMETER;
            }
        }
        break;
        
    case IOCTL_BATTERY_NOTIFYOFTIMECHANGE:
        // sanity check parameters
        if(pInBuf != NULL && InBufLen == sizeof(FILETIME) && pOutBuf != NULL 
            && OutBufLen == sizeof(BOOL)) {
            FILETIME ft;
            BOOL fForward;

            // pass back return values
            __try {
                ft = *((PFILETIME) pInBuf);
                fForward = *((PBOOL) pOutBuf);
                dwErr = ERROR_SUCCESS;
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                DEBUGMSG(ZONE_WARN, 
                    (_T("%s: exception in IOCTL_BATTERY_NOTIFYOFTIMECHANGE\r\n"), pszFname));
            }

            // did we get the parameter ok?
            if(dwErr == ERROR_SUCCESS) {
                // assume a good status, even though the function is void
                SetLastError(ERROR_SUCCESS);

                // get the data
                LOCKBATTERY();
                BatteryAPINotifyOfTimeChange(fForward, &ft);
                UNLOCKBATTERY();

                // get the status code
                dwErr = GetLastError();
            }
        }
        break;
        
    default:
        // Pass through to the PDD if enabled.  Note that the PDD function returns an error code,
        // not a boolean.
        if(gpfnBatteryPddIOControl != NULL) {
            dwErr = gpfnBatteryPddIOControl(dwContext, Ioctl, pInBuf, InBufLen, pOutBuf, OutBufLen, pdwBytesTransferred);
            DEBUGMSG(ZONE_IOCTL || ZONE_WARN, (_T("%s: PDD IOControl(%u) returned %d\r\n"), pszFname, Ioctl, dwErr));
        } else {
            DEBUGMSG(ZONE_IOCTL || ZONE_WARN, (_T("%s: Unsupported IOCTL code %u\r\n"), pszFname, Ioctl));
            dwErr = ERROR_NOT_SUPPORTED;
        }
        break;
    }
    
    // pass back appropriate response codes
    SetLastError(dwErr);
    if(dwErr != ERROR_SUCCESS) {
        bRc = FALSE;
    } else {
        bRc = TRUE;
    }

    DEBUGMSG(ZONE_FUNCTION || (!bRc && ZONE_WARN), (_T("%s: returning %d, error %d\r\n"),
        pszFname, bRc, dwErr));
    return bRc;
}

VOID
PowerDown(
              DWORD dwContext
              )
{
    // notify the PDD
    BatteryPDDPowerHandler(TRUE);
}


VOID
PowerUp(
            DWORD dwContext
            )
{
    // notify the PDD
    BatteryPDDPowerHandler(FALSE);
    
    // schedule our resume thread
    if(ghevResume != NULL) {
        CeSetPowerOnEvent(ghevResume);
    }
}


DWORD
Open(
         DWORD Context, 
         DWORD Access,
         DWORD ShareMode)
{
    
    DEBUGMSG(ZONE_FUNCTION,(_T("Open(%x, 0x%x, 0x%x)\r\n"),Context, Access, ShareMode));
    
    UNREFERENCED_PARAMETER(Access);
    UNREFERENCED_PARAMETER(ShareMode);
    
    // pass back the device handle
    return Context;     // 0 indicates failure
}


BOOL  
Close(
          DWORD Context
          ) 
{
    DEBUGMSG(ZONE_FUNCTION,(_T("Close(%x)\r\n"), Context));
    
    return TRUE;
}


DWORD
Read(
         DWORD  dwContext,
         LPVOID pBuf,
         DWORD  Len
         ) 
{
    UNREFERENCED_PARAMETER(dwContext);
    UNREFERENCED_PARAMETER(pBuf);
    UNREFERENCED_PARAMETER(Len);
    
    DEBUGMSG(ZONE_ERROR | ZONE_FUNCTION,(_T("Read\r\n")));
    SetLastError(ERROR_INVALID_FUNCTION);
    return  0;
}

DWORD
Write(
          DWORD  dwContext,
          LPVOID pBuf,
          DWORD  Len
          ) 
{
    UNREFERENCED_PARAMETER(dwContext);
    UNREFERENCED_PARAMETER(pBuf);
    UNREFERENCED_PARAMETER(Len);
    
    DEBUGMSG(ZONE_ERROR | ZONE_FUNCTION,(_T("Read\r\n")));
    SetLastError(ERROR_INVALID_FUNCTION);
    return  0;
}

ULONG
Seek(
         PVOID Context,
         LONG Position,
         DWORD Type
         )
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Position);
    UNREFERENCED_PARAMETER(Type);
    
    return (DWORD)-1;
}

BOOL WINAPI
DllMain(
         HINSTANCE hDllHandle, 
         DWORD  dwReason, 
         LPVOID lpreserved
         ) 
{
    BOOL bRc = TRUE;
    
    UNREFERENCED_PARAMETER(hDllHandle);
    UNREFERENCED_PARAMETER(lpreserved);
    
    switch (dwReason) {
    case DLL_PROCESS_ATTACH: 
        {
            DEBUGREGISTER(hDllHandle);
            DEBUGMSG(ZONE_INIT,(_T("*** DLL_PROCESS_ATTACH - Current Process: 0x%x, ID: 0x%x ***\r\n"),
                GetCurrentProcess(), GetCurrentProcessId()));
            DisableThreadLibraryCalls((HMODULE) hDllHandle);
        } 
        break;
        
    case DLL_PROCESS_DETACH: 
        {
            DEBUGMSG(ZONE_INIT,(_T("*** DLL_PROCESS_DETACH - Current Process: 0x%x, ID: 0x%x ***\r\n"),
                GetCurrentProcess(), GetCurrentProcessId()));
        } 
        break;
        
    default:
        break;
    }
    
    return bRc;
}
