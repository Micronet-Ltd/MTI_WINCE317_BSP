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
// This file contains the entry points for the Power Manager.  These entry
// points are called from device.exe, which implicitly links with the PM dll.
// PM global variables are defined in this module.
//

#define _DEFINE_PM_VARS     // global variables defined in this module

#include <pmimpl.h>
#include "PmSysReg.h"
#include "pmext.h"
#include <devload.h>

// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)

// force C linkage to match external variable declarations
extern "C" {

// define guids
GUID idGenericPMDeviceClass = { 0xA32942B7, 0x920C, 0x486b, { 0xB0, 0xE6, 0x92, 0xA7, 0x02, 0xA9, 0x9B, 0x35 } };
GUID idPMDisplayDeviceClass = { 0xEB91C7C9, 0x8BF6, 0x4A2D, { 0x9A, 0xB8, 0x69, 0x72, 0x4E, 0xED, 0x97, 0xD1 } };

// define list pointers
PDEVICE_POWER_RESTRICTION gpFloorDx;
PDEVICE_POWER_RESTRICTION gpCeilingDx;
PPOWER_NOTIFICATION gpPowerNotifications;
PSYSTEM_POWER_STATE gpSystemPowerState;
PDEVICE_LIST gpDeviceLists;
PPACTIVITY_TIMER gppActivityTimers;

// thread control variables
HANDLE ghevPmShutdown;              // set this event to close threads
HANDLE ghtPnP;                      // handle to the PnP thread
HANDLE ghtResume;                   // handle to the resume thread
HANDLE ghtSystem;                   // handle to the system thread
HANDLE ghtActivityTimers;           // handle to the activity timers thread

// other globals
CRITICAL_SECTION gcsPowerManager;   // serialize access to PM data
CRITICAL_SECTION gcsDeviceUpdateAPIs; // serialize application calls that cause device updates
HINSTANCE ghInst;                   // handle to the PM DLL
HANDLE ghevPowerManagerReady;       // when set, this event means that the PM api is enabled
HANDLE ghevResume;                  // power handler sets this to signal system resume
HANDLE ghevTimerResume;             // power handler sets this to re-enable activity timers
HANDLE ghPmHeap;                    // power manager's memory allocation heap

// PMExt related data
HINSTANCE ghPMExtLib; 
DWORD gdwPMExtContext = 0;
PFN_PMExt_Init gpPMExtInit;
PFN_PMExt_DeInit gpPMExtDeinit;

#if defined(DEBUG) || !defined(_PM_NOCELOGMSG)

// message zone configuration
#define DBG_ERROR           (1 << 0)
#define DBG_WARN            (1 << 1)
#define DBG_INIT            (1 << 2)
#define DBG_RESUME          (1 << 3)
#define DBG_DEVICE          (1 << 4)
#define DBG_IOCTL           (1 << 5)
#define DBG_REFCNT          (1 << 6)
#define DBG_REQUIRE         (1 << 7)
#define DBG_NOTIFY          (1 << 8)
#define DBG_REGISTRY        (1 << 9)
#define DBG_PLATFORM        (1 << 10)
#define DBG_API             (1 << 11)
#define DBG_ALLOC           (1 << 12)
#define DBG_TIMERS          (1 << 13)

DBGPARAM dpCurSettings = {
    _T("PM"), 
    {
        TEXT("Errors"), TEXT("Warnings"), TEXT("Init"), TEXT("Resume"), 
        TEXT("Device"), TEXT("Ioctl"), TEXT("Refcnt"), TEXT("Require"),
        TEXT("Notify"),TEXT("Registry"),TEXT("Platform"),TEXT("API"),
        TEXT("Alloc"),TEXT("Timers"),TEXT(""),TEXT("") 
    },
    DBG_ERROR | DBG_WARN | DBG_INIT    // ulZoneMask
}; 

#endif  // defined(DEBUG) || !defined(_PM_NOCELOGMSG)

};      // extern "C"

// C++ Global for PM registry.
SystemNotifyRegKey * g_pSysRegistryAccess = NULL;

// This routine reads the registry to determine what type of device interfaces
// we will be monitoring.  The default PM GUID is ignored if present in the
// registry and is always added last (so it's first in the list).
BOOL
DeviceListsInit(VOID)
{
    BOOL fOk = TRUE;
    PDEVICE_LIST pdl;
    DWORD dwStatus;
    HKEY hk;
    TCHAR szBuf[MAX_PATH];
    SETFNAME(_T("DeviceListsInit"));

    // enumerate all the device classes
    StringCchPrintf(szBuf,_countof(szBuf), _T("%s\\Interfaces"), PWRMGR_REG_KEY);
    dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szBuf, 0, 0, &hk);
    if(dwStatus == ERROR_SUCCESS) {
        DWORD dwIndex = 0;
        do {
            DWORD cbValueName = dim(szBuf), dwType;
            GUID idInterface;

            dwStatus = RegEnumValue(hk, dwIndex, szBuf, &cbValueName, NULL,
                &dwType, NULL, NULL);
            if(dwStatus == ERROR_SUCCESS) {
                if(dwType != REG_SZ) {
                    PMLOGMSG(ZONE_WARN, (_T("%s: invalid type for value '%s'\r\n"),
                        pszFname, szBuf));
                } else if(!ConvertStringToGuid(szBuf, &idInterface)) {
                    PMLOGMSG(ZONE_WARN, (_T("%s: can't convert '%s' to GUID\r\n"),
                        pszFname, szBuf));
                } else if(idInterface == idGenericPMDeviceClass) {
                    PMLOGMSG(ZONE_INIT, (_T("%s: default GUID found in registry as expected\r\n"), 
                        pszFname));
                } else if((pdl = DeviceListCreate(&idInterface)) == NULL) {
                    PMLOGMSG(ZONE_WARN, (_T("%s: DeviceListCreate() failed\r\n"), 
                        pszFname));
                } else if(PlatformDeviceListInit(pdl) == FALSE) {
                    PMLOGMSG(ZONE_WARN, (_T("%s: PlatformDeviceListInit() failed\r\n"), 
                        pszFname));
                    DeviceListDestroy(pdl);
                } else {
                    // add the new entry to the list
                    pdl->pNext = gpDeviceLists;
                    gpDeviceLists = pdl;
                }

                // update the index
                dwIndex++;
            }
        } while(dwStatus == ERROR_SUCCESS);

        // check for abnormal termination of the loop
        if(dwStatus != ERROR_NO_MORE_ITEMS) {
            fOk = FALSE;
        }

        // close the registry handle
        RegCloseKey(hk);
    }

    // add the default list last
    if(fOk) {
        fOk = FALSE;
        pdl = DeviceListCreate(&idGenericPMDeviceClass);
        if(pdl != NULL) {
            if(PlatformDeviceListInit(pdl) == FALSE) {
                PMLOGMSG(ZONE_INIT || ZONE_WARN, 
                    (_T("%s: PlatformDeviceListInit() failed for default class\r\n"),
                    pszFname));
                DeviceListDestroy(pdl);
            } else {
                pdl->pNext = gpDeviceLists;
                gpDeviceLists = pdl;
                fOk = TRUE;
            }
        }
    }

    // clean up if necessary
    if(!fOk) {
        PMLOGMSG(ZONE_WARN, (_T("%s: error during list initialization\r\n"), 
            pszFname));
        while(gpDeviceLists != NULL) {
            pdl = gpDeviceLists;
            gpDeviceLists = pdl->pNext;
            pdl->pNext = NULL;
            DeviceListDestroy(pdl);
        }
    }

    return fOk;
}

// This routine initializes the power manager and notifies the system that
// its api set is ready to be used.  It returns TRUE if successful and FALSE
// if there's a problem.
EXTERN_C BOOL WINAPI
PmInit(VOID)
{
    BOOL fOk = TRUE;
    HANDLE hevPnPReady = NULL,  hevSystemReady = NULL;
 //   HANDLE hevActivityTimersReady = NULL, hevResumeReady = NULL;
	HANDLE hevDummy = NULL;
    SETFNAME(_T("PmInit"));

    PMLOGMSG(ZONE_INIT || ZONE_API, (_T("+%s\r\n"), pszFname));

    // set up globals
    InitializeCriticalSection(&gcsPowerManager);
    InitializeCriticalSection(&gcsDeviceUpdateAPIs);
    gpFloorDx = NULL;
    gpCeilingDx = NULL;
    gpPowerNotifications = NULL;
    gpSystemPowerState = NULL;
    ghPmHeap = GetProcessHeap();
    gpDeviceLists = NULL;
    gppActivityTimers = NULL;
    ghevPowerManagerReady = NULL;
    ghevResume = NULL;
    ghevTimerResume = NULL;
    ghtPnP = NULL;
    ghtResume = NULL;
    ghtActivityTimers = NULL;
    ghtSystem = NULL;

    // cleanup event (hopefully never used)
    ghevPmShutdown = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(ghevPmShutdown == NULL) {
        PMLOGMSG(ZONE_ERROR, (_T("%s: CreateEvent() failed for shutdown event\r\n"), pszFname));
        fOk = FALSE;
    } 

    // validate the power management registry settings.  OEM code should use this
    // routine to make sure that registry settings are present for all the power
    // states they expect to support.  If the registry is not configured, the OEM
    // code can treat it as a fatal error or perform its own initialization.
    if(fOk) {
        DWORD dwStatus = PlatformValidatePMRegistry();
        if(dwStatus != ERROR_SUCCESS) {
            PMLOGMSG(ZONE_ERROR, (_T("%s: PlatformValidatePMRegistry() failed %d\r\n"), 
                pszFname, dwStatus));
            fOk = FALSE;
        } else {
            // read the list of interface types we will monitor
            fOk = DeviceListsInit();
        }
    }

    // create events
    if(fOk) {
        ghevPowerManagerReady = CreateEvent(NULL, TRUE, FALSE, _T("SYSTEM/PowerManagerReady"));
        ghevResume = CreateEvent(NULL, FALSE, FALSE, NULL);
        ghevTimerResume = CreateEvent(NULL, FALSE, FALSE, NULL);
        hevPnPReady = CreateEvent(NULL, FALSE, FALSE, NULL);
      //  hevResumeReady = CreateEvent(NULL, FALSE, FALSE, NULL);
        hevSystemReady = CreateEvent(NULL, FALSE, FALSE, NULL);
       // hevActivityTimersReady = CreateEvent(NULL, FALSE, FALSE, NULL);
        hevDummy = CreateEvent(NULL, FALSE, FALSE, NULL);

        // check everything
        if(hevPnPReady == NULL
       // || hevResumeReady == NULL 
        || hevSystemReady == NULL
        //|| hevActivityTimersReady == NULL
        || hevDummy == NULL
        || ghevPowerManagerReady == NULL
        || ghevTimerResume == NULL
        || ghevResume == NULL) {
            PMLOGMSG(ZONE_ERROR, (_T("%s: event creation failure\r\n"), pszFname));
            fOk = FALSE;
        }
    }

    // start threads
    if(fOk) {
        ghtPnP = CreateThread(NULL, 0, PnpThreadProc, (LPVOID) hevPnPReady, 0, NULL);
   //     ghtResume = CreateThread(NULL, 0, ResumeThreadProc, (LPVOID) hevResumeReady, 0, NULL);
  //      ghtActivityTimers = CreateThread(NULL, 0, ActivityTimersThreadProc, 
   //         (LPVOID) hevActivityTimersReady, 0, NULL);
        
        // check everything
        if(ghtPnP == NULL 
       // || ghtResume == NULL 
       // || ghtActivityTimers == NULL
		) {
            PMLOGMSG(ZONE_ERROR, (_T("%s: thread creation failure\r\n"), pszFname));
            fOk = FALSE;
        }
    }

    // wait for threads to initialize (or fail to initialize)
    #define NUM_OF_READY_EXIT_PAIR 1
    HANDLE hEvents[] = { hevPnPReady, /*hevResumeReady,hevActivityTimersReady, */
        ghtPnP,/* ghtResume,ghtActivityTimers*/ };
    int iReady = 0;
    while( iReady < NUM_OF_READY_EXIT_PAIR  && fOk) {
        DWORD dwStatus = WaitForMultipleObjects(dim(hEvents), hEvents, FALSE, INFINITE);
        switch(dwStatus) {
        // thread ready events
        case (WAIT_OBJECT_0 + 0):   // pnp ready
      //  case (WAIT_OBJECT_0 + 1):   // resume ready
      //  case (WAIT_OBJECT_0 + 2):   // activity timers ready
            // don't watch for the thread exiting now -- some may do
            // so if they don't have work to do.
            hEvents[dwStatus - WAIT_OBJECT_0 + NUM_OF_READY_EXIT_PAIR] = hevDummy;
            iReady++;
            break;

        // thread exiting events
        case (WAIT_OBJECT_0 + 1):   // pnp exited
      //  case (WAIT_OBJECT_0 + 4):   // resume exited
     //   case (WAIT_OBJECT_0 + 5):   // activity timers exited
            PMLOGMSG(ZONE_ERROR, (_T("%s: thread initialization failure\r\n"), pszFname));
            fOk = FALSE;
            break;
        default:
            PMLOGMSG(ZONE_ERROR, (_T("%s: WaitForMultipleObjects() returnd %d, status is %d\r\n"),
                pszFname, GetLastError()));
            fOk = FALSE;
            break;
        }
    }

    // load PMExt DLL, call init
    if (fOk) {
        TCHAR DevDll[DEVDLL_LEN];
        DWORD cbData;
        DWORD Flags;
        HKEY hKey;

        gpPMExtInit = NULL;
        gpPMExtDeinit = NULL;;

        // Note: TEXT("\\Omap") is appended to the PMExt_Registry_Root because this is the only PMExt that we support
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, PMExt_Registry_Root TEXT("\\Omap"), 0, 0, &hKey)) {
            cbData = sizeof(DevDll);
            if (ERROR_SUCCESS == RegQueryValueEx(hKey, DEVLOAD_DLLNAME_VALNAME, NULL, NULL, (LPBYTE) DevDll, &cbData)) {
                cbData = sizeof(Flags);
                if (ERROR_SUCCESS != RegQueryValueEx(hKey, DEVLOAD_FLAGS_VALNAME, NULL, NULL, (LPBYTE) &Flags, &cbData)) {
                    Flags = DEVFLAGS_NONE;
				}
                ghPMExtLib = (Flags & DEVFLAGS_LOADLIBRARY) ? LoadLibrary(DevDll) : LoadDriver(DevDll);
                if (!ghPMExtLib) {
                    PMLOGMSG(ZONE_ERROR, (_T("%s: couldn't load PMExt \"%s\" -- error %d\r\n"), pszFname, DevDll, GetLastError()));
                } 
                else {
                    gpPMExtInit = (PFN_PMExt_Init) GetProcAddress(ghPMExtLib, PMExt_Init_NAME);
                    if (!gpPMExtInit)
					    PMLOGMSG(ZONE_ERROR, (_T("%s: \"%s\" can't GetProcAddress\r\n"), pszFname, PMExt_Init_NAME));

                    gpPMExtDeinit = (PFN_PMExt_DeInit) GetProcAddress(ghPMExtLib, PMExt_DeInit_NAME);
                }
            }
			else {
                PMLOGMSG(ZONE_INIT, (_T("%s: can't get value \"%s\" in key \"%s\"\r\n"), pszFname, DEVLOAD_DLLNAME_VALNAME, PMExt_Registry_Root TEXT("\\Omap")));
			}
            if (gpPMExtInit && gpPMExtDeinit) {
#ifdef DEBUG
               gdwPMExtContext = gpPMExtInit(HKEY_LOCAL_MACHINE, PMExt_Registry_Root TEXT("\\Omap"));
#else
                __try {
                    gdwPMExtContext = gpPMExtInit(HKEY_LOCAL_MACHINE, PMExt_Registry_Root TEXT("\\Omap"));
				}
                __except(EXCEPTION_EXECUTE_HANDLER) {
                    gdwPMExtContext = 0;
                }
#endif
                if (!gdwPMExtContext)
                    PMLOGMSG(ZONE_ERROR, (_T("%s: \"%s\" failed\r\n"), pszFname, PMExt_Init_NAME));
                else
                    PMLOGMSG(ZONE_INIT, (_T("%s: \"%s\" success\r\n"), pszFname, PMExt_Init_NAME));
            }
            RegCloseKey(hKey);
		}
		else {
            PMLOGMSG(ZONE_INIT, (_T("%s: can't open key [HKEY_LOCAL_MACHINE\\%s]\r\n"), pszFname, PMExt_Registry_Root TEXT("\\Omap")));
		}
	}

    // if everything is initialized, we're ready start the system thread
    if(fOk) {
        // check that the thread started
        ghtSystem = CreateThread(NULL, 0, SystemThreadProc, (LPVOID) hevSystemReady, 0, NULL);
        if(ghtSystem == NULL) {
            PMLOGMSG(ZONE_ERROR, (_T("%s: system thread creation failure\r\n"), pszFname));
            fOk = FALSE;
        }
        
        // wait for it to initialize or exit
        HANDLE hSystemEvents[] = { hevSystemReady, ghtSystem };
        DWORD dwStatus = WaitForMultipleObjects(dim(hSystemEvents), hSystemEvents, FALSE, INFINITE);
        if(dwStatus != WAIT_OBJECT_0) {
            PMLOGMSG(ZONE_ERROR, (_T("%s: system thread initialization failure\r\n"), pszFname));
            fOk = FALSE;
        }
    } 
    
    // should we signal that our API is ready?
    if(fOk) {
        // yes, the PM is initialized
        PmSetSystemPowerState(NULL, POWER_STATE_ON, POWER_FORCE);
        SetEvent(ghevPowerManagerReady);
    } else {
        // no, clean up
        if(ghevPmShutdown != NULL) {
            // tell threads to shut down
            SetEvent(ghevPmShutdown);
        }

        // wait for threads to exit
        if(ghtPnP != NULL) {
            PMLOGMSG(ZONE_ERROR, (_T("%s: shutting down PnP thread\r\n"), pszFname));
            WaitForSingleObject(ghtPnP, INFINITE);
            CloseHandle(ghtPnP);
        }
		/*
        if(ghtResume != NULL) {
            PMLOGMSG(ZONE_ERROR, (_T("%s: shutting down resume thread\r\n"), pszFname));
            WaitForSingleObject(ghtResume, INFINITE);
            CloseHandle(ghtResume);
        }
		*/
        if(ghtSystem != NULL) {
            PMLOGMSG(ZONE_ERROR, (_T("%s: shutting down system thread\r\n"), pszFname));
            WaitForSingleObject(ghtSystem, INFINITE);
            CloseHandle(ghtSystem);
        }
		/*
        if(ghtActivityTimers != NULL) {
            PMLOGMSG(ZONE_ERROR, (_T("%s: shutting down activity timers thread\r\n"), pszFname));
            WaitForSingleObject(ghtActivityTimers, INFINITE);
            CloseHandle(ghtActivityTimers);
        }
		*/

        PMLOGMSG(ZONE_ERROR, (_T("%s: closing handles\r\n"), pszFname));
        if(ghevPmShutdown != NULL) CloseHandle(ghevPmShutdown);
        if(ghevPowerManagerReady != NULL) CloseHandle(ghevPowerManagerReady);
        if(ghevResume != NULL) CloseHandle(ghevResume);
        if(ghevTimerResume != NULL) CloseHandle(ghevTimerResume);
    }

    // clean up status handles
    if(hevPnPReady != NULL) CloseHandle(hevPnPReady);
  //  if(hevResumeReady != NULL) CloseHandle(hevResumeReady);
    if(hevSystemReady != NULL) CloseHandle(hevSystemReady);
   // if(hevActivityTimersReady != NULL) CloseHandle(hevActivityTimersReady);
    if(hevDummy != NULL) CloseHandle(hevDummy);

    PMLOGMSG((!fOk && ZONE_ERROR) || ZONE_INIT || ZONE_API, 
        (_T("-%s: returning %d\r\n"), pszFname, fOk));

    DEBUGCHK(fOk);      // make sure we raise a red flag if the init fails

    return fOk;
}

// This routine receives notifications from the system about various system
// events, including processes exiting.  Some of the messages have the same
// names as DllEntry() messages, but the resemblance is superficial.  Note 
// that unlike DLL entry point message handlers, these notifications allow
// the message handler to use synchronization objects without deadlocking
// the system.
EXTERN_C VOID WINAPI 
PmNotify(DWORD dwFlags, HPROCESS hProcess, HTHREAD hThread)
{
    SETFNAME(_T("PmNotify"));

    UnusedParameter(hThread);

    switch(dwFlags) {
    case DLL_PROCESS_DETACH:
        // release any resources held by this process
        PMLOGMSG(ZONE_API, (_T("+%s: hProcess 0x%08x exiting\r\n"), pszFname, hProcess));
        DeleteProcessRequirements(hProcess);
        DeleteProcessNotifications(hProcess);
        PMLOGMSG(ZONE_API, (_T("-%s\r\n"), pszFname));
        break;
    default:
        break;
    }
}
// dll entry point
STDAPI_(BOOL) WINAPI
DllEntry(HANDLE hInst, DWORD  dwReason, LPVOID lpvReserved) 
{
    BOOL bRc = TRUE;
    SETFNAME(_T("Power Manager"));
    
    UnusedParameter(hInst);
    UnusedParameter(lpvReserved);
    
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        ghInst = (HINSTANCE) hInst;
        RETAILREGISTERZONES(ghInst);    // zones in both debug and retail builds
        PMLOGMSG(ZONE_INIT,(TEXT("*** %s: DLL_PROCESS_ATTACH - Current Process: 0x%x, ID: 0x%x ***\r\n"),
            pszFname, GetCurrentProcess(), GetCurrentProcessId()));
        DisableThreadLibraryCalls((HMODULE) hInst);
        break;
        
    case DLL_PROCESS_DETACH:
        PMLOGMSG(ZONE_INIT,(TEXT("*** %s: DLL_PROCESS_DETACH - Current Process: 0x%x, ID: 0x%x ***\r\n"),
            pszFname, GetCurrentProcess(), GetCurrentProcessId()));
        if (g_pSysRegistryAccess != NULL)
            delete g_pSysRegistryAccess;
        g_pSysRegistryAccess = NULL;
        break;
        
    default:
        break;
    }
   
    return bRc;
}
