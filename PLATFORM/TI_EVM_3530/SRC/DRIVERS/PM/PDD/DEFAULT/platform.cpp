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
// This module implements a set of states defined by the MS Mobile Devices
// Division as a baseline for Pocket PC devices.  OEMs may choose to customize
// this file to support the hardware specific to their platform.
//
// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//

#include <pmimpl.h>
#include <nkintr.h>
#include <extfile.h>
#include <pmpolicy.h>

#include <pwsdef.h>
#include <ceddkex.h>
#include <notify.h>

//#include <oalex.h>


typedef BOOL (WINAPI * PFN_GwesPowerDown)(void);
typedef void (WINAPI * PFN_GwesPowerUp)(BOOL);
typedef BOOL (WINAPI * PFN_ShowStartupWindow)( void );

#define MAXACTIVITYTIMEOUT              (0xFFFFFFFF / 1000)  // in seconds

// gwes suspend/resume functions
PFN_GwesPowerDown gpfnGwesPowerDown = NULL;
PFN_GwesPowerUp gpfnGwesPowerUp = NULL;
PFN_ShowStartupWindow gpfnShowStartupWindow = NULL;

// this variable is protected by the system power state critical section
BOOL gfSystemSuspended = FALSE;
BOOL gfFileSystemsAvailable = TRUE;
GUID idBlockDevices  = {0x8DD679CE, 0x8AB4, 0x43c8, { 0xA1, 0x4A, 0xEA, 0x49, 0x63, 0xFA, 0xA7, 0x15 } };
GUID idResumeDevices = {0xA32942B8, 0x920C, 0x486b, { 0xB0, 0xE6, 0x92, 0xA7, 0x02, 0xA9, 0xAB, 0x89 } };

#define PMCLASS_RESUME                TEXT("{A32942B8-920C-486b-B0E6-92A702A9AB89}")


HANDLE ghevGwesReady;
BOOL gfGwesReady;
INT giPreSuspendPriority;
INT giSuspendPriority;

BOOL   gfPasswordOn = 0;
BOOL   gfSupportPowerButtonRelease = FALSE;
BOOL   gfPageOutAllModules = FALSE;
BOOL   gfWantStartupScreen = FALSE;

DefaultPowerStateManager *g_pPowerStateManager = NULL;



HANDLE EnableWakeUp(LPCTSTR szEventName, DWORD dwSeconds, HANDLE * pWaitEvent);
void DisableWakeUp(HANDLE hNotify, HANDLE hWaitEvent);
void SetGWESPowerUp();

// need "C" linkage for compatibility with C language PDD implementations
extern "C" {
POWER_BROADCAST_POWER_INFO gSystemPowerStatus;
};

// This routine is called to check the consistency of the system's power 
// management registry settings.  It is called during during power manager
// initialization.  If no registry settings are found, OEMs can use this
// routine to set them up.  The routine returns FALSE if some fatal error
// is discovered and the registry is unusable.  This will halt PM 
// initialization.  If the registry is OK (or can be initialized/repaired)
// this routine returns ERROR_SUCCESS, otherwise it returns an error code.
EXTERN_C DWORD WINAPI
PlatformValidatePMRegistry(VOID)
{
    HKEY hkPM = NULL, hkSubkey;
    LPTSTR pszSubKey;
    DWORD dwStatus = ERROR_GEN_FAILURE;
    DWORD dwDisposition;
    SETFNAME(_T("PlatformValidatePMRegistry"));

    PMLOGMSG(ZONE_INIT, (_T("+%s\r\n"), pszFname));

    // open the PM registry key
    dwStatus = RegCreateKeyEx(HKEY_LOCAL_MACHINE, PWRMGR_REG_KEY, 0, NULL, 0, 0, NULL, 
        &hkPM, &dwDisposition);
    if(dwStatus != ERROR_SUCCESS) {
        PMLOGMSG(ZONE_ERROR, (_T("%s: can't open '%s', error is %d\r\n"), pszFname, 
            PWRMGR_REG_KEY, dwStatus));
    } 
    if (dwStatus== ERROR_SUCCESS  && dwDisposition != REG_CREATED_NEW_KEY ) { // Exit Key.
        DWORD dwValue =  0;
        DWORD dwSize = sizeof(DWORD);
        if (RegQueryTypedValue(hkPM, PM_SUPPORT_PB_RELEASE, &dwValue, &dwSize, REG_DWORD)==ERROR_SUCCESS) {
            gfSupportPowerButtonRelease = (dwValue!=0);
        }
        dwSize = sizeof(dwSize);
        if(RegQueryTypedValue(hkPM, L"PageOutAllModules", &dwValue, &dwSize, REG_DWORD) == ERROR_SUCCESS
                && dwValue!=0 ) 
            gfPageOutAllModules = TRUE;
        else
            gfPageOutAllModules = FALSE;
    }

    // verify interface guids
    if(dwStatus == ERROR_SUCCESS) {
        pszSubKey = _T("Interfaces");
        dwStatus = RegCreateKeyEx(hkPM, pszSubKey, 0, NULL, 0, 0, NULL, &hkSubkey,
            &dwDisposition);
        if(dwStatus == ERROR_SUCCESS) {
            if(dwDisposition == REG_CREATED_NEW_KEY) {
                LPTSTR pszName = PMCLASS_GENERIC_DEVICE;
                LPTSTR pszValue = _T("Generic power-manageable devices");
                dwStatus = RegSetValueEx(hkSubkey, pszName, 0, REG_SZ, (LPBYTE) pszValue, 
                    (_tcslen(pszValue) + 1) * sizeof(*pszValue));
                if(dwStatus == ERROR_SUCCESS) {
                    pszName = PMCLASS_BLOCK_DEVICE;
                    pszValue = _T("Power-manageable block devices");
                    dwStatus = RegSetValueEx(hkSubkey, pszName, 0, REG_SZ, (LPBYTE) pszValue, 
                        (_tcslen(pszValue) + 1) * sizeof(*pszValue));
                }
                if(dwStatus == ERROR_SUCCESS) {
                    pszName = PMCLASS_DISPLAY;
                    pszValue = _T("Power-manageable display drivers");
                    dwStatus = RegSetValueEx(hkSubkey, pszName, 0, REG_SZ, (LPBYTE) pszValue, 
                        (_tcslen(pszValue) + 1) * sizeof(*pszValue));
                }
            }
            RegCloseKey(hkSubkey);
        }

        PMLOGMSG(dwStatus != ERROR_SUCCESS && ZONE_ERROR, 
            (_T("%s: error %d while creating or writing values in '%s\\%s'\r\n"), pszFname, dwStatus,
            PWRMGR_REG_KEY, pszSubKey));
    }

    // release resources
    if(hkPM != NULL) RegCloseKey(hkPM);

    PMLOGMSG(ZONE_INIT, (_T("-%s: returning %d\r\n"), pszFname, dwStatus));

    return dwStatus;
}

// This routine performs platform-specific initialization on a device list.
// Primarily this involves determining what routines should be used to communicate
// with the class.
EXTERN_C BOOL WINAPI
PlatformDeviceListInit(PDEVICE_LIST pdl)
{
    BOOL fOk = FALSE;
    PDEVICE_INTERFACE pInterface;
    SETFNAME(_T("PlatformDeviceListInit"));

    PREFAST_DEBUGCHK(pdl != NULL);
    DEBUGCHK(pdl->pGuid != NULL);

    if(*pdl->pGuid == idPMDisplayDeviceClass) {
        PMLOGMSG(ZONE_INIT || ZONE_PLATFORM, 
            (_T("%s: using display interface to access class %08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x\r\n"), 
            pszFname, pdl->pGuid->Data1, pdl->pGuid->Data2, pdl->pGuid->Data3,
            (pdl->pGuid->Data4[0] << 8) + pdl->pGuid->Data4[1], pdl->pGuid->Data4[2], pdl->pGuid->Data4[3], 
            pdl->pGuid->Data4[4], pdl->pGuid->Data4[5], pdl->pGuid->Data4[6], pdl->pGuid->Data4[7]));
#ifndef NODISPLAYINTERFACE
        // Use the display driver interface to get to the device.  To remove
        // display code from the link, edit or conditionally compile this code out
        // of the PM.
        extern DEVICE_INTERFACE gDisplayInterface;      // defined in the MDD
        pInterface = &gDisplayInterface;
#else   // NODISPLAYINTERFACE
        PMLOGMSG(ZONE_INIT || ZONE_WARN, (_T("%s: warning: display interface not supported\r\n"), pszFname));
#endif  // NODISPLAYINTERFACE
    } else {
        // use the standard stream interface to get to the device
        PMLOGMSG(ZONE_INIT || ZONE_PLATFORM, 
            (_T("%s: using stream interface to access class %08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x\r\n"), 
            pszFname, pdl->pGuid->Data1, pdl->pGuid->Data2, pdl->pGuid->Data3,
            (pdl->pGuid->Data4[0] << 8) + pdl->pGuid->Data4[1], pdl->pGuid->Data4[2], pdl->pGuid->Data4[3], 
            pdl->pGuid->Data4[4], pdl->pGuid->Data4[5], pdl->pGuid->Data4[6], pdl->pGuid->Data4[7]));
        extern DEVICE_INTERFACE gStreamInterface;       // defined in the MDD
        pInterface = &gStreamInterface;
    }

    // try to initialize the interface
    if(pInterface != NULL) {
        if(pInterface->pfnInitInterface() == FALSE) {
            PMLOGMSG(ZONE_WARN, (_T("%s: warning: pfnInitInterface() failed for interface\r\n"), pszFname));
        } else {
            // pass back the pointer
            pdl->pInterface = pInterface;
            fOk = TRUE;
        }
    }

    return fOk;
}
// This routine is responsible for mapping system power state hint
// values to known system power states that we maintain in the registry.
EXTERN_C DWORD WINAPI
PlatformMapPowerStateHint(DWORD dwHint, LPTSTR pszBuf, DWORD dwBufChars)
{
    DWORD dwStatus = ERROR_FILE_NOT_FOUND;
    SETFNAME(_T("PlatformMapPowerStateHint"));
    if (g_pPowerStateManager) {
        dwStatus = g_pPowerStateManager->PlatformMapPowerStateHint(dwHint, pszBuf, dwBufChars);
    }
    PMLOGMSG(dwStatus != ERROR_SUCCESS && ZONE_WARN,
        (_T("%s: returning %d\r\n"), pszFname, dwStatus));
    return dwStatus;
}
ULONG WINAPI unsafe_shut_down(void *arg)
{
	void *vp = OpenWatchDogTimer(L"PM Platform OFF", 0);

	RETAILMSG(1, (L"PM: unsafe_shut_down :  Wait for action (%x, %d)\r\n", vp, GetTickCount()));
	if(vp)
	{
		if(WAIT_OBJECT_0 == WaitForSingleObject(vp, INFINITE))
		{
			RETAILMSG(1, (L"PM: unsafe_shut_down :  Urgent shutdown (%d)\r\n", GetTickCount()));
		#if defined (BSP_DEBUG_SERIAL)
			Sleep(50); // for debugging purposes only
		#endif
			KernelLibIoControl((HANDLE)KMOD_OAL, IOCTL_HAL_SHUTDOWN, 0, 0, 0, 0, 0);
		}
		CloseHandle(vp);
	}
	RETAILMSG(1, (L"PM: unsafe_shut_down :  Leave (%d, %d)\r\n", GetLastError(), GetTickCount()));


	return 0;
}
// This routine reads and verifies system power state information from
// the registry, then updates all devices appropriately.  The caller of this
// routine should hold the system power state critical section.  The fInternal
// flag indicates whether the call originated within the PM or outside it.
EXTERN_C DWORD WINAPI
PlatformSetSystemPowerState(LPCTSTR pszName, BOOL fForce, BOOL fInternal)
{
    DWORD dwStatus = ERROR_SUCCESS;
    PSYSTEM_POWER_STATE pNewSystemPowerState = NULL;
    PDEVICE_POWER_RESTRICTION pNewCeilingDx = NULL;
    BOOL fDoTransition = FALSE;
    INT iPreSuspendPriority = 0;
    static BOOL fFirstCall = TRUE;
	//TCHAR  szNotifyEventName[255]= _T("_____Wake_Event");

    SETFNAME(_T("PlatformSetSystemPowerState"));
    
    // read system power state variables and construct new lists
    if (gfFileSystemsAvailable)
        PmUpdateSystemPowerStatesIfChanged();
    dwStatus = RegReadSystemPowerState(pszName, &pNewSystemPowerState, &pNewCeilingDx);

	RETAILMSG(1,(_T("%s: %s\r\n"), pszFname,pszName));
        
    // did we get registry information about the new power state?			
    if(dwStatus == ERROR_SUCCESS) {
        BOOL fSuspendSystem = FALSE;
      //  static BOOL fWantStartupScreen = FALSE;
        DWORD dwNewStateFlags = pNewSystemPowerState->dwFlags;
        BOOL  fPasswordOn = ((dwNewStateFlags & POWER_STATE_PASSWORD)!=0) ;
        
        // assume we will update the system power state
        fDoTransition = TRUE;
        
        // Are we going to suspend the system as a whole?
        if((dwNewStateFlags & (POWER_STATE_SUSPEND | POWER_STATE_OFF | POWER_STATE_CRITICAL | POWER_STATE_RESET)) != 0) {
            fSuspendSystem = TRUE;
        }
        
        // A "critical" suspend might mean we have totally lost battery power and need
        // to suspend really quickly.  Depending on the platform, OEMs may be able
        // to bypass driver notification entirely and rely on xxx_PowerDown() notifications
        // to suspend gracefully.  Or they may be able to implement a critical suspend
        // kernel ioctl.  This sample implementation is very generic and simply sets the 
        // POWER_FORCE flag, which is not used.
        if(dwNewStateFlags & (POWER_STATE_CRITICAL | POWER_STATE_OFF | POWER_STATE_RESET)) {
            fForce = TRUE;
        }
                
        // if everything seems ok, do the set operation
        if(fDoTransition) {
            POWER_BROADCAST_BUFFER pbb;
            PDEVICE_LIST pdl;
            BOOL fResumeSystem = FALSE;
            
            // send out system power state change notifications
            pbb.Message = PBT_TRANSITION;
            pbb.Flags = pNewSystemPowerState->dwFlags;
            pbb.Length = _tcslen(pNewSystemPowerState->pszName) + 1; // char count not byte count for now
            if(pbb.Length > MAX_PATH) {
                // truncate the system power state name -- note, we actually have MAX_PATH + 1
                // characters available.
                pbb.Length = MAX_PATH;
            }
            _tcsncpy(pbb.SystemPowerState, pNewSystemPowerState->pszName, pbb.Length);
            pbb.Length *= sizeof(pbb.SystemPowerState[0]);      // convert to byte count
            GenerateNotifications((PPOWER_BROADCAST) &pbb);
            
            // is GWES ready?
            if(!gfGwesReady) {
                if(ghevGwesReady != NULL && WaitForSingleObject(ghevGwesReady, 0) == WAIT_OBJECT_0) {
                    gfGwesReady = TRUE;
                    CloseHandle(ghevGwesReady);
                    ghevGwesReady = NULL;
                }
            }
            // are we suspending?
            if(fSuspendSystem && gpfnGwesPowerDown != NULL) {
                
                // Start the process of suspending GWES
                if(gfGwesReady) {
                    gfWantStartupScreen = gpfnGwesPowerDown();
                }
            }
            
            // update global system state variables
            PMLOCK();
            PSYSTEM_POWER_STATE pOldSystemPowerState = gpSystemPowerState;
            PDEVICE_POWER_RESTRICTION pOldCeilingDx = gpCeilingDx;
            if(gpSystemPowerState != NULL
                && (gpSystemPowerState->dwFlags & (POWER_STATE_SUSPEND | POWER_STATE_OFF | POWER_STATE_CRITICAL)) != 0) {
                // we are exiting a suspended state
                fResumeSystem = TRUE;
            }
            gpSystemPowerState = pNewSystemPowerState;
            gpCeilingDx = pNewCeilingDx;
            PMUNLOCK();
            
            // are we suspending, resuming, or neither?
            if(fSuspendSystem) {
                INT iCurrentPriority;
				void *vp;

                // Notify the kernel that we are about to suspend.  This gives the
                // kernel an opportunity to clear wake source flags before we initiate
                // the suspend process.  If we don't do this and a wake source interrupt
                // occurs between the time we call PowerOffSystem() and the time 
                // OEMPowerOff() is invoked it is hard for the kernel to know whether or
                // not to suspend.
				BOOL bShutdown = ((dwNewStateFlags & POWER_STATE_OFF) != 0)? TRUE:FALSE;

                KernelIoControl(IOCTL_HAL_PRESUSPEND, &bShutdown, sizeof(bShutdown), NULL, 0, NULL);
				RETAILMSG(1, (_T("%s: IOCTL_HAL_PRESUSPEND:%s\r\n"), pszFname, (bShutdown)?L"Shutdown":L"Suspend"));


				if(bShutdown)
				{
					vp = CreateWatchDogTimer(L"PM Platform OFF", 8000, 100, WDOG_RESET_DEVICE, 0, 0);
					if(vp)
					{
						void *vpt;
						StartWatchDogTimer(vp, 0);
						vpt = CreateThread(0, 0, unsafe_shut_down, 0, 0, 0);
						CeSetThreadPriority(vpt, CeGetThreadPriority(GetCurrentThread()) - 1);
						CloseHandle(vpt);
					}
				}
				// we're suspending; update all devices other than block devices,
                // in case any of them need to access the registry or write files.
                RETAILMSG(1, (_T("%s: suspending - notifying non-block drivers\r\n"), pszFname));
                for(pdl = gpDeviceLists; pdl != NULL; pdl = pdl->pNext) {
                    if(*pdl->pGuid != idBlockDevices) {
                        UpdateClassDeviceStates(pdl);
                    }
                }
                

                iCurrentPriority = CeGetThreadPriority(GetCurrentThread());
                DEBUGCHK(iCurrentPriority != THREAD_PRIORITY_ERROR_RETURN);
                if(iCurrentPriority != THREAD_PRIORITY_ERROR_RETURN) {
					RETAILMSG(1, (_T("%s: PRESUSPEND: rescshedular %d\r\n"), pszFname, giPreSuspendPriority));
                    CeSetThreadPriority(GetCurrentThread(), giPreSuspendPriority);
                    Sleep(0);
                    CeSetThreadPriority(GetCurrentThread(), iCurrentPriority);
                }

                // Notify file systems that their block drivers will soon go away.  After making
                // this call, this thread is the only one that can access the file system
                // (including registry and device drivers) without blocking.  Unfortunately,
                // this API takes and holds the file system critical section, so other threads
                // attempting to access the registry or files may cause priority inversions.
                // To avoid priority problem that may starve the PM, we may raise our own priority
                // to a high level.  Do this if giSuspendPriority is non-zero.
                if(giSuspendPriority != 0) {
                    iPreSuspendPriority = CeGetThreadPriority(GetCurrentThread());
					RETAILMSG(1, (_T("%s: SUSPEND priority %d->%d\r\n"), pszFname, iPreSuspendPriority, giSuspendPriority));
                    DEBUGCHK(iPreSuspendPriority != THREAD_PRIORITY_ERROR_RETURN);
                    PMLOGMSG(ZONE_PLATFORM, 
                        (_T("%s: suspending: raising thread priority for 0x%08x from %d to %d\r\n"),
                        pszFname, GetCurrentThreadId(), iPreSuspendPriority, giSuspendPriority));
                    CeSetThreadPriority(GetCurrentThread(), giSuspendPriority);
                }
				RETAILMSG(1, (_T("%s: FSNOTIFY_POWER_OFF\r\n"), pszFname));
                FileSystemPowerFunction(FSNOTIFY_POWER_OFF);
                gfFileSystemsAvailable = FALSE;

                // update block device power states
                RETAILMSG(1, (_T("%s: suspending - notifying block drivers\r\n"), pszFname));
                pdl = GetDeviceListFromClass(&idBlockDevices);
                if(pdl != NULL) {
                    UpdateClassDeviceStates(pdl);
                }
				if(bShutdown)
				{
					if(vp)
					{
						StopWatchDogTimer(vp, 0);
						CloseHandle(vp);
					}
				}
                
				//RETAILMSG(TRUE, (_T("PM: PlatformSetSystemPowerState: dwNewStateFlags = 0x%x \r\n"),dwNewStateFlags));
                // Handle resets and shutdowns here, after flushing files.  Since Windows CE does
                // not define a standard mechanism for handling shutdown (via POWER_STATE_OFF),
                // OEMs will need to fill in the appropriate code here.  Similarly, if an OEM does
                // not support IOCTL_HAL_REBOOT, they should not support POWER_STATE_RESET.
                if((dwNewStateFlags & POWER_STATE_RESET) != 0) {
                    // is this to be a cold boot?
                    if(_tcscmp(pszName, _T("coldreboot")) == 0) {
                        SetCleanRebootFlag();
                    }

                    // should not return from this call, but if we do just suspend the system
                    KernelLibIoControl((HANDLE)KMOD_OAL, IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, NULL);
                    RETAILMSG(TRUE, (_T("PM: PlatformSetSystemPowerState: KernelIoControl(IOCTL_HAL_REBOOT) returned!\r\n")));
                    DEBUGCHK(FALSE);        // break into the debugger
				}/*
				else if((dwNewStateFlags & POWER_STATE_OFF) != 0) {
                   
					RETAILMSG(TRUE, (_T("PM: PlatformSetSystemPowerState: KernelIoControl(IOCTL_HAL_SHUTDOWN) started!\r\n")));
                    // should not return from this call, but if we do just suspend the system
                    KernelLibIoControl((HANDLE)KMOD_OAL, IOCTL_HAL_SHUTDOWN, NULL, 0, NULL, 0, NULL);
                    RETAILMSG(TRUE, (_T("PM: PlatformSetSystemPowerState: KernelIoControl(IOCTL_HAL_SHUTDOWN) returned!\r\n")));
                    DEBUGCHK(FALSE);        // break into the debugger
                } */
            } else if(fResumeSystem) {
                // we're waking up from a resume -- update block device power states
                // so we can access the registry and/or files.
                PMLOGMSG(ZONE_PLATFORM || ZONE_RESUME, 
                    (_T("%s: resuming - notifying block drivers\r\n"), pszFname));
                pdl = GetDeviceListFromClass(&idBlockDevices);
                if(pdl != NULL) {
                    UpdateClassDeviceStates(pdl);
                }
                
                // Notify file systems that their block drivers are back.
                FileSystemPowerFunction(FSNOTIFY_POWER_ON);
                gfFileSystemsAvailable = TRUE;

				
			        
                // update all devices other than block devices
                PMLOGMSG(ZONE_PLATFORM || ZONE_RESUME, 
                    (_T("%s: resuming - notifying block drivers\r\n"), pszFname));

				
				pdl = GetDeviceListFromClass(&idResumeDevices);
                if(pdl != NULL) {
                    UpdateClassDeviceStates(pdl);
                }
				
                // tell gwes to wake up
				//SetGWESPowerUp();//moved after	GenerateNotifications
				
	
                // send out resume notification
                pbb.Message = PBT_RESUME;
                pbb.Flags = 0;
                pbb.Length = 0;
                pbb.SystemPowerState[0] = 0;
                GenerateNotifications((PPOWER_BROADCAST) &pbb);
                // tell gwes to wake up
				SetGWESPowerUp();
            } else {
                // update all devices without any particular ordering
                UpdateAllDeviceStates();
            }
            
            // release the old state information
			RETAILMSG(0, (_T("%s: Destroy old power state\r\n"), pszFname));
            SystemPowerStateDestroy(pOldSystemPowerState);
            while(pOldCeilingDx != NULL) {
                PDEVICE_POWER_RESTRICTION pdpr = pOldCeilingDx->pNext;
                PowerRestrictionDestroy(pOldCeilingDx);
                pOldCeilingDx = pdpr;
            }
            
            // are we suspending?
            if(fSuspendSystem) {
                // Once file systems are disabled, discard code pages from drivers.  This
                // is a diagnostic tool to forcibly expose paging-related bugs that could 
                // cause apparently random system crashes or hangs.  Optionally, OEMs can
                // disable this for production systems to speed up resume times.
                if(gfPageOutAllModules) {
					RETAILMSG(1, (_T("%s: discard code pages\r\n"), pszFname));
                    BOOL fOk = PageOutModule(GetCurrentProcess(), PAGE_OUT_ALL_DEPENDENT_DLL);
                    DEBUGCHK(fOk);                    
                }

                // set a flag to notify the resume thread that this was a controlled
                // suspend
                gfSystemSuspended = TRUE;
				
                
                RETAILMSG(1, (_T("%s: calling PowerOffSystem()\r\n"), L"PlatformSetSystemPowerState"));
				//RETAILMSG(1, (_T("%s: calling PowerOffSystem()\r\n"), pszFname));
                PowerOffSystem();       // sets a flag in the kernel for the scheduler
                Sleep(0);               // so we force the scheduler to run
                RETAILMSG(1, (_T("%s: back from PowerOffSystem()\r\n"), L"PlatformSetSystemPowerState"));
                
                // clear the suspend flag
                gfSystemSuspended = FALSE;
                gfPasswordOn = 0 ;
            }
            else {
                PMLOCK();
                if ( fPasswordOn != gfPasswordOn ) {
                    if (gfGwesReady && gpfnShowStartupWindow && fPasswordOn )
                        gpfnShowStartupWindow();
                    gfPasswordOn = fPasswordOn;
                }
                PMUNLOCK();
            }
        } else {
            // release the unused new state information
            SystemPowerStateDestroy(pNewSystemPowerState);
            while(pNewCeilingDx != NULL) {
                PDEVICE_POWER_RESTRICTION pdpr = pNewCeilingDx->pNext;
                PowerRestrictionDestroy(pNewCeilingDx);
                pNewCeilingDx = pdpr;
            }
            
        }
    }

    // restore our priority if we updated it during a suspend transition
    if(giSuspendPriority != 0 && iPreSuspendPriority != 0) {
        PMLOGMSG(ZONE_PLATFORM, (_T("%s: restoring thread priority to %d\r\n"), 
            pszFname, iPreSuspendPriority));
        CeSetThreadPriority(GetCurrentThread(), iPreSuspendPriority);
    }
    
    return dwStatus;
}

// This routine reads a power state timeout value from the registry.  The
// timeout value should be expressed in seconds; this routine returns 
// the timeout value in milliseconds.
DWORD
RegReadStateTimeout(HKEY hk, LPCTSTR pszName, DWORD dwDefault)
{
    DWORD dwValue, dwSize, dwStatus;
    
    dwSize = sizeof(dwValue);
    dwStatus = RegQueryTypedValue(hk, pszName, &dwValue, &dwSize, REG_DWORD);
    if(dwStatus != ERROR_SUCCESS) {
        dwValue = dwDefault;
    } else {
        if(dwValue == 0) {
            dwValue = INFINITE;
        } else {
            if(dwValue > MAXACTIVITYTIMEOUT) {
                dwValue = MAXACTIVITYTIMEOUT;
            }
        }
    }

    // convert to milliseconds
    if(dwValue != INFINITE) {
        dwValue *= 1000;
    }

    return dwValue;
}
// This routine implements the main power manager event loop.  This loop implements
// the power manager's response to external events, such as being docked in a cradle,
// running low on battery power, going on or off AC power, etc.  This sample implementation
// simply monitors battery level changes and generates the appropriate notifications.
EXTERN_C VOID WINAPI 
PlatformManageSystemPower(HANDLE hevReady)
{
    BOOL fDone = FALSE;
    HANDLE hqNotify = NULL;
    HMODULE hmCoreDll = NULL;
    SETFNAME(_T("PlatformManageSystemPower"));
    
    PMLOGMSG(ZONE_INIT || ZONE_PLATFORM, (_T("+%s\r\n"), pszFname));
        
    // determine thread priority settings while we're suspending (in case
    // of priority inversion)
    if(!GetPMThreadPriority(_T("PreSuspendPriority256"), &giPreSuspendPriority)) {
        giPreSuspendPriority = DEF_PRESUSPEND_THREAD_PRIORITY;
    }
    if(!GetPMThreadPriority(_T("SuspendPriority256"), &giSuspendPriority)) {
        giSuspendPriority = 51; //DEF_SUSPEND_THREAD_PRIORITY;
    }
    
    // Get pointers to GWES's suspend/routine APIs.  These require GWES, so the OEM may
    // not have them on this platform.  Also get battery level APIs, which require a 
    // battery driver and may not be present.
    hmCoreDll = (HMODULE) LoadLibrary(_T("coredll.dll"));
    gfGwesReady = FALSE;
    PmInitPowerStatus(hmCoreDll);
    if(hmCoreDll != NULL) {
        gpfnGwesPowerDown = (PFN_GwesPowerDown) GetProcAddress(hmCoreDll, _T("GwesPowerDown"));
        gpfnGwesPowerUp = (PFN_GwesPowerUp) GetProcAddress(hmCoreDll, _T("GwesPowerUp"));
        gpfnShowStartupWindow = (PFN_ShowStartupWindow) GetProcAddress(hmCoreDll, _T("ShowStartupWindow"));
        
        // do we have both gwes suspend/resume APIs?
        if(gpfnGwesPowerDown == NULL || gpfnGwesPowerUp == NULL) {
            // no, ignore GWES
            gpfnGwesPowerDown = NULL;
            gpfnGwesPowerUp = NULL;
        } else {
            // monitor GWES apis
            ghevGwesReady = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T("SYSTEM/GweApiSetReady"));
            DEBUGCHK(ghevGwesReady != NULL);
        }
    }
            
    if (!fDone) {
        DefaultPowerStateManager *pPowerStateManager = CreateDefaultPowerManager( NULL );
        if (pPowerStateManager && pPowerStateManager->Init()) {
            g_pPowerStateManager = pPowerStateManager ;
            // we're up and running
            SetEvent(hevReady);
            g_pPowerStateManager->ThreadRun() ;
        }
        else {
            ASSERT(FALSE);
            if (pPowerStateManager)
                delete pPowerStateManager;
            PMLOGMSG(ZONE_INIT || ZONE_ERROR, (_T("%s: PowerStateManager Intialization Failed!!!\r\n"), pszFname));
        }
        if (g_pPowerStateManager) {
            delete g_pPowerStateManager;
            g_pPowerStateManager = NULL;
        }
    }
    if(hmCoreDll != NULL) FreeLibrary(hmCoreDll);
    
    PMLOGMSG(ZONE_PLATFORM, (_T("-%s: exiting\r\n"), pszFname));
}

// This routine is called when the system has resumed.  It is responsible for
// determining why the system woke up and initiating the appropriate system
// power state transition.  This routine is invoked in the context of the PM's
// resume thread.  
// Note:  this thread must be running at a higher priority than any thread
// that might suspend the system using SetSystemPowerState().  Otherwise it cannot
// tell whether it needs to update the system power state on its own.  In general,
// OEMs should not suspend the system without calling SetSystemPowerState() -- the
// code in this routine that handles unexpected suspends is present only as a fallback.
EXTERN_C VOID WINAPI
PlatformResumeSystem(void)
{
    SETFNAME(_T("PlatformResumeSystem"));
    
    PMLOGMSG(ZONE_RESUME, (_T("+%s: suspend flag is %d\r\n"), pszFname, gfSystemSuspended));
    
    // Was this an unexpected resume event?  If so, there may be a thread priority problem
    // or some piece of software suspended the system without calling SetSystemPowerState().
    DEBUGCHK(gfSystemSuspended);
    if (g_pPowerStateManager) {
        g_pPowerStateManager->PlatformResumeSystem(gfSystemSuspended);
    }
    PMLOGMSG(ZONE_RESUME, (_T("-%s\r\n"), pszFname));
}

DWORD WINAPI PlatformSendSystemPowerState(LPCWSTR pwsState, DWORD dwStateHint, DWORD dwOptions)
{
    DWORD dwReturn = ERROR_GEN_FAILURE;
    if (g_pPowerStateManager) {
        dwReturn = g_pPowerStateManager->SendSystemPowerState(pwsState, dwStateHint, dwOptions );
    }
    return dwReturn;
}



	//------------------------------------------------------------------------------
	// Use this function to setup wake up event at specified interval.
	// Input Params
	// szEventName: Name of the event. This function will create this named event for you.
	// dwSeconds: Specify how many seconds later the current time this event should be set.
	// Output Params
	// Returned Value: Function returns handle to this notify process. This is different than
	//                 handle returned by pWaitEvent. Use this handle to clear off this wake up process.
	//                 If this function fails then it returns NULL.
	// pWaitEvent: Handle to named event which one can wait on
	// 
HANDLE EnableWakeUp(LPCTSTR szEventName, DWORD dwSeconds, HANDLE * pWaitEvent)
{
	HANDLE     hWaitEvent=NULL;
	HANDLE     hNotify=NULL;
	CE_NOTIFICATION_TRIGGER sTrigger;
	TCHAR szNotifyEventName[255]= _T("\\\\.\\Notifications\\NamedEvents\\");
	SYSTEMTIME curSysTime = {0};
	FILETIME fileTime;

	if ((!szEventName)||(!pWaitEvent))
	{
		RETAILMSG(1, (_T("invalid input params")));
		return hNotify;
	}

	//Ignoring buffer overflow.
	_tcscat(szNotifyEventName, szEventName);

	RETAILMSG(0, (_T("szNotifyEventName=%s"), szNotifyEventName));
	RETAILMSG(0, (_T("Seconds=%d"), dwSeconds));

	sTrigger.dwSize = sizeof(CE_NOTIFICATION_TRIGGER);
	//Time based notification
	sTrigger.dwType = CNT_TIME;     
	sTrigger.dwEvent = 0;  

	// this will be a named event
	sTrigger.lpszApplication = szNotifyEventName;  
	sTrigger.lpszArguments = NULL;

	// set start and end of notification period
	//sTrigger.stEndTime = curSysTime;
/*
	GetLocalTime(&curSysTime);
	SystemTimeToFileTime(&curSysTime, &fileTime);
	AddSecondsToFileTime(&fileTime, dwSeconds);
	FileTimeToSystemTime(&fileTime, &curSysTime);
*/

	GetLocalTime(&curSysTime);
	SystemTimeToFileTime(&curSysTime, &fileTime);
	unsigned __int64 t;
	memcpy(&t, &fileTime, sizeof(t));
	t += ( __int64 )dwSeconds * ( __int64 )10000000;
	memcpy(&fileTime, &t, sizeof(t));
	FileTimeToSystemTime(&fileTime, &curSysTime);


	sTrigger.stStartTime = curSysTime;
	sTrigger.stEndTime   = curSysTime;

	// create a named event
	hWaitEvent = CreateEvent(NULL, FALSE, FALSE, szEventName);

	// if named event was created
	if(hWaitEvent)
	{
		// create user notification
		hNotify = CeSetUserNotificationEx( 0, &sTrigger, NULL );
		if (!hNotify)
		{
			RETAILMSG(1, (_T("CeSetUserNotificationEx failed.")));
			CloseHandle(hNotify);
		}
	}
	else
		RETAILMSG(1, (_T("CreateEvent failed.")));

	*pWaitEvent = hWaitEvent;
	RETAILMSG(0, (_T(" - EnableWakeUp.")));
	return hNotify;
}

//------------------------------------------------------------------------------
// Use this function to disable wake up event which was setup earlier
// Input Params
// hNotify: Handle to "Notify process". OS will do cleanup based on this handle.
// hWaitEvent: This is wait event on which user app is supposed to wait. This will be closed here.
// Output Params
// Returned Value: None.
// 
void DisableWakeUp(HANDLE hNotify, HANDLE hWaitEvent)
{
	if (hNotify)
		CeClearUserNotification(hNotify);

	if (hWaitEvent)
		CloseHandle(hWaitEvent);

}

void SetGWESPowerUp()
{
	if(gpfnGwesPowerUp != NULL && gfGwesReady) {
            gpfnGwesPowerUp(gfWantStartupScreen);
            gfWantStartupScreen = FALSE;
    }
}