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


#include <pmimpl.h>
#include <nkintr.h>
#include <extfile.h>
#include <pmpolicy.h>
#include <pwsdef.h>
//#include <notify.h>
//#define STRING_NOTIFY_EVENT _T("\\\\.\\Notifications\\NamedEvents\\____Notify_Wake_Event")



PowerState::PowerState(DefaultPowerStateManager *pPwrStateMgr, PowerState * pNextPowerState)
:   m_pPwrStateMgr (pPwrStateMgr)
,   m_pNextPowerState (pNextPowerState)
{
    memset (m_dwEventArray,0,sizeof(m_dwEventArray)) ;
    m_hUnsignaledHandle = CreateEvent(NULL, FALSE, FALSE,NULL);
    PREFAST_ASSERT(pPwrStateMgr!=NULL);
    m_dwEventArray [ PM_SHUTDOWN_EVENT ] = pPwrStateMgr->GetEventHandle(PM_SHUTDOWN_EVENT );
    m_dwEventArray [ PM_RELOAD_ACTIVITY_TIMEOUTS_EVENT ] =  pPwrStateMgr->GetEventHandle( PM_RELOAD_ACTIVITY_TIMEOUTS_EVENT);
    m_dwEventArray [ PM_MSGQUEUE_EVENT ] = pPwrStateMgr->GetEventHandle( PM_MSGQUEUE_EVENT );
    m_dwEventArray [ PM_RESTART_TIMER_EVENT ] = pPwrStateMgr->GetEventHandle( PM_RESTART_TIMER_EVENT );
    
 //   m_dwEventArray [ PM_USER_ACTIVITY_EVENT ] = (pPwrStateMgr->GetUserActivityTimer()!=NULL? pPwrStateMgr->GetUserActivityTimer()->hevActive:NULL);
 //   m_dwEventArray [ PM_SYSTEM_ACTIVITY_EVENT ] = (pPwrStateMgr->GetSystemActivityTimer()!=NULL? pPwrStateMgr->GetSystemActivityTimer()->hevActive:NULL);

    m_dwEventArray [ PM_USER_ACTIVITY_EVENT ] =   (pPwrStateMgr->GetUserActivityTimer()!=NULL ? pPwrStateMgr->GetUserActivityTimer()->hevReset:NULL);
    m_dwEventArray [ PM_SYSTEM_ACTIVITY_EVENT ] = (pPwrStateMgr->GetSystemActivityTimer()!=NULL? pPwrStateMgr->GetSystemActivityTimer()->hevReset:NULL);
    
    m_dwEventArray [ PM_SYSTEM_API_EVENT] = pPwrStateMgr->GetEventHandle(PM_SYSTEM_API_EVENT);
    m_dwEventArray [ PM_BOOTPHASE2_EVENT] = pPwrStateMgr->GetEventHandle(PM_BOOTPHASE2_EVENT);
    
    m_dwNumOfEvent = PM_BASE_TOTAL_EVENT;
    for (DWORD dwIndex=0 ; dwIndex < PM_BASE_TOTAL_EVENT; dwIndex ++ ) {
        if (m_dwEventArray[dwIndex] == NULL) { // Use dummy one.
            m_dwEventArray[dwIndex]  = m_hUnsignaledHandle;
        }
    }
    m_InitFlags = 0;
    m_LastNewState = PM_UNKNOWN_POWER_STATE ;
}
PowerState::~PowerState()
{
    if (m_hUnsignaledHandle!=NULL)
        CloseHandle(m_hUnsignaledHandle);
}
BOOL PowerState::Init()
{
    SETFNAME(_T("PowerState::Init"));
    if (m_pPwrStateMgr && m_hUnsignaledHandle ) {
        for (DWORD dwIndex=0; dwIndex < m_dwNumOfEvent; dwIndex++)
            if ( m_dwEventArray[dwIndex] == NULL) {
                ASSERT(FALSE);
                return FALSE;
            }
        m_LastNewState = GetState(); // Point to itself
        DWORD dwReturn = StateValidateRegistry();
        if (dwReturn != ERROR_SUCCESS) {
            PMLOGMSG(ZONE_PLATFORM, (_T("%s: StateValidateRegistry return (0x%08x) fails\r\n"), pszFname, dwReturn));    
            ASSERT(FALSE);
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}
PLATFORM_ACTIVITY_EVENT PowerState::MsgQueueEvent()
{
    SETFNAME(_T("PowerState::MsgQueueEvent"));
    PLATFORM_ACTIVITY_EVENT activeEvent = NoActivity ;
    POWERPOLICYMESSAGE ppm;
    DWORD dwStatus = PmPolicyReadNotificationQueue( m_dwEventArray [ PM_MSGQUEUE_EVENT ], &ppm, sizeof(ppm));
    if(dwStatus == ERROR_SUCCESS) {
        PMLOGMSG(ZONE_PLATFORM, (_T("%s: got request 0x%04x (data 0x%08x) from process 0x%08x\r\n"),
            pszFname, ppm.dwMessage, ppm.dwData, ppm.hOwnerProcess));
        switch(ppm.dwMessage) {
        case PPN_POWERCHANGE:
            if(PmUpdatePowerStatus()){
                activeEvent = PowerSourceChange;
            }
            break;
        case PPN_SUSPENDKEYPRESSED:
			RETAILMSG(1, (L" PowerState::MsgQueueEvent: %s\r\n",  (ppm.dwData)?L"Standby":L"Shutdown" ));
			if(ppm.dwData == 0)
				activeEvent = PowerButtonPressed;
			else
				activeEvent = SuspButtonPressed;
            break;
        case PPN_APPBUTTONPRESSED:
            activeEvent = AppButtonPressed;
            break;
        case PPN_UNATTENDEDMODE:
            // somebody wants to enter or leave unattended mode
            if(ppm.dwData != FALSE) {
                activeEvent = EnterUnattendedModeRequest;
            } else {
                activeEvent = LeaveUnattendedModeRequest;
            }
			break;
		case PPN_RESUMETESTEND:
			 if(ppm.dwData != FALSE) {
                activeEvent = ResumeTestOk;
            } else {
                activeEvent = ResumeTestFailed;
            }
            break;
		case PPN_SUSPENDSHUTDOWN:
			 if(ppm.dwData != FALSE) {
                activeEvent = SuspendShutdownRequest;
            } else {
                activeEvent = SuspendShutdownRelease;
            }
            break;

        default:
            // unhandled notification type, ignore it
            PMLOGMSG(ZONE_WARN, (_T("%s: unhandled policy notification 0x%04x (data 0x%08x)\r\n"), 
                pszFname, ppm.dwMessage, ppm.dwData));
            break;
        }
    }
    PMLOGMSG(ZONE_PLATFORM, (_T("%s: return ActiveEvent = 0x%08x\r\n"), pszFname, activeEvent));    
    return activeEvent;
}
void PowerState::EnterState()
{
    SETFNAME(_T("PowerState::EnterState"));
	RETAILMSG(0, (_T("%s: %s %d\r\n"  ), pszFname, GetStateString(), GetTickCount())) ;
    PmSetSystemPowerState_I ( GetStateString(), 0 , 0, TRUE);
    m_LastNewState = GetState(); // Point to itself
    // Initial Timeout to Active First.
	


    if (m_pPwrStateMgr->GetUserActivityTimer())
        m_dwEventArray [ PM_USER_ACTIVITY_EVENT ] = m_pPwrStateMgr->GetUserActivityTimer()->hevReset;
	else
		 m_dwEventArray [ PM_USER_ACTIVITY_EVENT ] =m_hUnsignaledHandle;

    if (m_pPwrStateMgr->GetSystemActivityTimer())
        m_dwEventArray [ PM_SYSTEM_ACTIVITY_EVENT ] = m_pPwrStateMgr->GetSystemActivityTimer()->hevReset;
	else
		 m_dwEventArray [ PM_SYSTEM_ACTIVITY_EVENT ] = m_hUnsignaledHandle;
		
    //TODO Do not forget to implement this in vender specific code. m_pPwrStateMgr->ReAdjustTimeOuts( );
}
PLATFORM_ACTIVITY_EVENT PowerState::WaitForEvent(DWORD dwTimeouts, DWORD dwNumOfExternEvent, HANDLE * pExternEventArray)
{
    DWORD dwNumOfEvent = m_dwNumOfEvent ;
    if (dwNumOfExternEvent!=0 && pExternEventArray!=NULL ) {
        for ( ; dwNumOfExternEvent!=0 &&  dwNumOfEvent < MAX_EVENT_ARRAY ; dwNumOfEvent++  )
            if (*pExternEventArray != NULL ) {
                m_dwEventArray [ dwNumOfEvent ] = *pExternEventArray ;
                pExternEventArray ++;
                dwNumOfExternEvent --;    
            }
            else  {
                ASSERT(FALSE);
                return NoActivity;
            }
    }
    DWORD dwTicksElapsed = GetTickCount();
    DWORD dwReturn = WaitForMultipleObjects ( min(dwNumOfEvent,MAX_EVENT_ARRAY), m_dwEventArray, FALSE, dwTimeouts);
	RETAILMSG(0, (_T("PowerState::WaitForEvent (%d)\r\n"), dwReturn));
    dwTicksElapsed = GetTickCount() - dwTicksElapsed;
    m_pPwrStateMgr->SubtractTimeout(dwTicksElapsed);
    if (dwReturn == WAIT_TIMEOUT) 
        return Timeout;
    else if (dwReturn >= WAIT_OBJECT_0 + m_dwNumOfEvent) { // Extern Event            
        return (PLATFORM_ACTIVITY_EVENT)(ExternedEvent + dwReturn - (WAIT_OBJECT_0 + m_dwNumOfEvent));
    }
    else {
        PLATFORM_ACTIVITY_EVENT platEvent = NoActivity;
        switch(dwReturn - WAIT_OBJECT_0 ) {
            case PM_SHUTDOWN_EVENT:
                platEvent = PmShutDown;
                break;
             case PM_RELOAD_ACTIVITY_TIMEOUTS_EVENT:
                platEvent = PmReloadActivityTimeouts;
                break;
            case PM_MSGQUEUE_EVENT:
                platEvent = MsgQueueEvent();
                break;
            case PM_RESTART_TIMER_EVENT:
                platEvent =  RestartTimeouts;
                break;
            case PM_USER_ACTIVITY_EVENT:
                if (m_pPwrStateMgr->GetUserActivityTimer()) {
					/*
                    if (m_dwEventArray [ PM_USER_ACTIVITY_EVENT ] == m_pPwrStateMgr->GetUserActivityTimer()->hevActive ) { // Active Signaled. Same State and Wait For Inactive
                        m_dwEventArray [ PM_USER_ACTIVITY_EVENT ] = m_pPwrStateMgr->GetUserActivityTimer()->hevInactive;
                        m_pPwrStateMgr->ResetUserIdleTimeout(FALSE);
                        m_pPwrStateMgr->ResetSystemIdleTimeTimeout(FALSE); // User Activity will tread as System Actiivty
                        platEvent = UserActivity ;
                    }
                    else  { // Inactive Signaled . Start to wait on Active.
                        ASSERT(m_dwEventArray [ PM_USER_ACTIVITY_EVENT ]  == m_pPwrStateMgr->GetUserActivityTimer()->hevInactive);
                        m_dwEventArray [ PM_USER_ACTIVITY_EVENT ]  = m_pPwrStateMgr->GetUserActivityTimer()->hevActive ;
                        m_pPwrStateMgr->ResetUserIdleTimeout(TRUE);
                        m_pPwrStateMgr->ResetSystemIdleTimeTimeout(TRUE);
                        platEvent = UserInactivity ;
                    }
					*/
					m_pPwrStateMgr->ResetUserIdleTimeout(TRUE);
					m_pPwrStateMgr->ResetSystemIdleTimeTimeout(TRUE);
					m_pPwrStateMgr->ResetSuspendTimeTimeout(TRUE);
					platEvent = UserActivity ;
				}
                else
                    ASSERT(FALSE);
                break;
            case PM_SYSTEM_ACTIVITY_EVENT:
				if (m_pPwrStateMgr->GetSystemActivityTimer()){
					/* 
                    if (m_dwEventArray [ PM_SYSTEM_ACTIVITY_EVENT ] == m_pPwrStateMgr->GetSystemActivityTimer()->hevActive ) { // Active Signaled. Same State and Wait For Inactive
                        m_dwEventArray [ PM_SYSTEM_ACTIVITY_EVENT ] = m_pPwrStateMgr->GetSystemActivityTimer()->hevInactive;
                        m_pPwrStateMgr->ResetSystemIdleTimeTimeout(FALSE); // User Activity will tread as System Actiivty
                        platEvent = SystemActivity ;
                    }
                    else  { // Inactive Signaled . Start to wait on Active.
                        ASSERT(m_dwEventArray [ PM_SYSTEM_ACTIVITY_EVENT ]  == m_pPwrStateMgr->GetSystemActivityTimer()->hevInactive);
                        m_dwEventArray [ PM_SYSTEM_ACTIVITY_EVENT ]  = m_pPwrStateMgr->GetSystemActivityTimer()->hevActive ;
                        m_pPwrStateMgr->ResetSystemIdleTimeTimeout(TRUE);
                        platEvent = SystemInactivity ;
                    }
					*/
					m_pPwrStateMgr->ResetSystemIdleTimeTimeout(TRUE);
					m_pPwrStateMgr->ResetSuspendTimeTimeout(TRUE);
                    platEvent = SystemActivity;
                }
                break;
            case PM_SYSTEM_API_EVENT:
                platEvent = SystemPowerStateAPI;
                break;
            case PM_BOOTPHASE2_EVENT: // This event only signal once.
			{	
				static int first = 0;
				RETAILMSG(0, (_T("PM: PM_BOOTPHASE2_EVENT\r\n")));
				if(!first)
				{
					m_pPwrStateMgr->PlatformLoadTimeouts();//once reread timeouts from registry
					first = 1;

					// remove all old notifications
/*
					HANDLE hn;
					DWORD dwHowMany;
					DWORD *buff = 0;

					CeGetUserNotificationHandles (0, 0, &dwHowMany);
					buff = (DWORD *)PmAlloc(dwHowMany*sizeof(HANDLE));

					CeGetUserNotificationHandles ((HANDLE *)buff, dwHowMany, &dwHowMany);
					for(int i=0; i < dwHowMany; i++)
					{
						DWORD *buf = 0;
						DWORD need;
						hn = (HANDLE)buff[i];
						int nRes = CeGetUserNotification(hn, 0, &need, 0);
						if(1)
						{
							buf = (DWORD *)PmAlloc(need);
							if(buf)
							{
								nRes = CeGetUserNotification(hn, need, &need, (unsigned char *)buf);
								CE_NOTIFICATION_INFO_HEADER *head;
								head = (CE_NOTIFICATION_INFO_HEADER *)buf;
								if(nRes && head && head->pcent && head->pcent->lpszApplication && (0 == wcscmp(STRING_NOTIFY_EVENT, head->pcent->lpszApplication)))
									CeClearUserNotification(head->hNotification);
								PmFree(buf);
							}
						}
					}
					PmFree(buff);
*/
				}

				platEvent = RestartTimeouts;
                m_dwEventArray [ PM_BOOTPHASE2_EVENT ] = m_hUnsignaledHandle;
			}
			break;
            default:
                platEvent = UnwantedEventHandle( dwReturn);
                break;
        }
 		RETAILMSG(0, (_T("-PowerState::WaitForEvent (%d) ev = %x\r\n"), dwReturn, platEvent));
       return platEvent;
    }

}
//TODO do not forget to implemet this in vender specific.
DWORD  PowerState::DefaultEventHandle(PLATFORM_ACTIVITY_EVENT platActivityEvent )
{
    switch (platActivityEvent) {
        case AppButtonPressed:
            m_LastNewState = On;
            break;
		case PowerButtonPressed:
			m_LastNewState = ShutDown;
			break;
		case SuspButtonPressed:
			m_LastNewState = Suspend;
			break;
		case SuspendShutdownRequest:
			m_pPwrStateMgr->SetSuspendShutdownState(TRUE);
			break;
		case SuspendShutdownRelease:
			m_pPwrStateMgr->SetSuspendShutdownState(FALSE);
			break;
        default:
            break;
    }
    return GetLastNewState();
}

DWORD PowerState::StateValidateRegistry(DWORD dwDState , DWORD dwFlag )
{
    HKEY hkPM = NULL, hkSubkey;
    TCHAR pszSubKey[MAX_PATH];
    DWORD dwDisposition;
    SETFNAME(_T("PowerState::StateValidateRegistry"));
    // open the PM registry key
    DWORD dwStatus = RegCreateKeyEx(HKEY_LOCAL_MACHINE, PWRMGR_REG_KEY, 0, NULL, 0, 0, NULL,  &hkPM, &dwDisposition);
    if(dwStatus != ERROR_SUCCESS) {
        PMLOGMSG(ZONE_ERROR, (_T("%s: can't open '%s', error is %d\r\n"), pszFname, PWRMGR_REG_KEY, dwStatus));
    } 
    // verify the On system state
    if(dwStatus == ERROR_SUCCESS) {
        StringCchPrintf(pszSubKey,MAX_PATH,_T("State\\%s"), GetStateString());
        dwStatus = RegCreateKeyEx(hkPM, pszSubKey, 0, NULL, 0, 0, NULL, &hkSubkey, &dwDisposition);
        if(dwStatus == ERROR_SUCCESS) {
            if(dwDisposition == REG_CREATED_NEW_KEY) {
                // allow devices to go to any power level
                DWORD dwValue = dwDState;      // D State
                dwStatus = RegSetValueEx(hkSubkey, NULL, 0, REG_DWORD, (LPBYTE) &dwValue, sizeof(dwValue));
                
                // write the flags value
                if(dwStatus == ERROR_SUCCESS) {
                    dwValue = dwFlag ;
                    m_InitFlags = dwFlag;
                    dwStatus = RegSetValueEx(hkSubkey, _T("Flags"), 0, REG_DWORD, (LPBYTE) &dwValue, sizeof(dwValue));
                }
            }
            else {
                DWORD dwValue = 0;
                DWORD dwLen = sizeof(DWORD);
                DWORD dwType = 0 ;
                if (RegQueryValueEx( hkSubkey, _T("Flags"),NULL, &dwType, (PBYTE)&dwValue, &dwLen ) == ERROR_SUCCESS ) {
                    m_InitFlags = dwValue ;
                }
            }
            RegCloseKey(hkSubkey);
        }
        PMLOGMSG(dwStatus != ERROR_SUCCESS && ZONE_ERROR, 
            (_T("%s: error %d while creating or writing values in '%s\\%s'\r\n"), pszFname, dwStatus,PWRMGR_REG_KEY, pszSubKey));
    }
    // release resources
    if(hkPM != NULL) RegCloseKey(hkPM);
    return dwStatus;
}


DefaultPowerStateManager::DefaultPowerStateManager(PVOID /*pParam*/)
{
    // create events
    m_hevReloadActivityTimeouts = CreateEvent(NULL, FALSE, FALSE, _T("PowerManager/ReloadActivityTimeouts"));
    m_hevBootPhase2 = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T("SYSTEM/BootPhase2"));

    m_hevRestartTimers = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hqNotify = PmPolicyCreateNotificationQueue();
    // Using Global Event
    m_hevPmShutdown = ghevPmShutdown;
        
    m_pPowerStateList = NULL;    
    m_pCurPowerState = NULL;
    
    m_hSystemApiCalled = CreateEvent(NULL, FALSE, FALSE,NULL) ;

    m_pUserActivity = ActivityTimerFindByName(_T("UserActivity"));;
    m_pSystemActivity = ActivityTimerFindByName(_T("SystemActivity"));
}
DefaultPowerStateManager::~DefaultPowerStateManager()
{
    if (m_hevReloadActivityTimeouts)
        CloseHandle(m_hevReloadActivityTimeouts);
    if (m_hevBootPhase2)
        CloseHandle(m_hevBootPhase2) ;
        
    if (m_hevRestartTimers)
        CloseHandle(m_hevRestartTimers);
    if (m_hqNotify)
        PmPolicyCloseNotificationQueue(m_hqNotify);
    
    if (m_hSystemApiCalled)
        CloseHandle(m_hSystemApiCalled);

}
BOOL  DefaultPowerStateManager::Init()
{
    SETFNAME(_T("PowerStateManager::Init"));
    if(m_hevReloadActivityTimeouts == NULL 
        || m_hevRestartTimers == NULL
        || m_hqNotify == NULL ) {
        PMLOGMSG(ZONE_WARN, (_T("%s: CreateEvent() failed for system event\r\n"), pszFname));
        return FALSE;
    }
    if (!m_hSystemApiCalled) {
        PMLOGMSG(ZONE_WARN, (_T("%s:  PMSystemAPI::Init() failed\r\n"), pszFname));
        return FALSE;
    }
    // check that all of our activity events exist
    if(m_pUserActivity && (m_pUserActivity->hevActive == NULL || m_pUserActivity->hevInactive == NULL)) {
        PMLOGMSG(ZONE_WARN, (_T("%s: UserActivity timer events not found\r\n"), pszFname));
        return FALSE;
    }
    if(m_pSystemActivity && ( m_pSystemActivity->hevActive == NULL || m_pSystemActivity->hevInactive == NULL)) {
        // SystemActivity is optional. If it present. Both hevActive && hevInactive has to valid.
        PMLOGMSG(ZONE_WARN, (_T("%s: SystemActivity timer events not found\r\n"), pszFname));
        return FALSE;
    }
    if(m_hqNotify == NULL) {
        PMLOGMSG(ZONE_WARN, (_T("%s: PmPolicyCreateNotificationQueue() failed\r\n"), pszFname));
        return FALSE;
    }
    return TRUE;
        
    
};
HANDLE  DefaultPowerStateManager::GetEventHandle(DWORD dwIndex)
{
    switch (dwIndex) {
        case PM_SHUTDOWN_EVENT:
            return ghevPmShutdown;

        case PM_RELOAD_ACTIVITY_TIMEOUTS_EVENT:
            return m_hevReloadActivityTimeouts;

        case PM_MSGQUEUE_EVENT:
            return m_hqNotify;

        case PM_RESTART_TIMER_EVENT:
            return m_hevRestartTimers;

        case PM_SYSTEM_API_EVENT:
            return GetAPISignalHandle();

        case PM_BOOTPHASE2_EVENT:
            return m_hevBootPhase2;
    }
    return NULL;

}

void    DefaultPowerStateManager::PlatformResumeSystem(BOOL fSuspened)
{
    SETFNAME(_T("DefaultPowerStateManager::PlatformResumeSystem"));
    if (fSuspened) {
        // Unexpected resume -- go to the resuming state.  This should not happen unless
        // somebody is illegally calling PowerOffSystem() directly.
        RETAILMSG(0, (_T("%s: WARNING: unexpected resume!\r\n"), pszFname));
        
        // Go into the new state.  OEMs that choose to support unexpected resumes may want to
        // lock PM variables with PMLOCK(), then set the curDx and actualDx values for all
        // devices to PwrDeviceUnspecified before calling PmSetSystemPowerState_I().  This will
        // force an update IOCTL to all devices.
        DEBUGCHK(m_hevRestartTimers != NULL);
        SetEvent(m_hevRestartTimers);
    }
    PMLOGMSG(ZONE_RESUME, (_T("-%s\r\n"), pszFname));
}
DWORD CountBit(DWORD dwBit)
{
    DWORD dwReturn = 0;
    while (dwBit) {
        dwBit = ((dwBit - 1 ) & dwBit);
        dwReturn ++ ;
    }
    return dwReturn;
}
DWORD   DefaultPowerStateManager::PlatformMapPowerStateHint(DWORD dwHint, LPTSTR pszBuf, DWORD dwBufChars)
{
    DWORD dwStatus = ERROR_FILE_NOT_FOUND;
    LPTSTR pszMappedStateName = NULL;
    SETFNAME(_T("DefaultPowerStateManager::PlatformMapPowerStateHint"));
    if (dwHint && pszBuf && dwBufChars) {
        PowerState * bestState = NULL;
        DWORD bestMatchBit = 0;
        PowerState * curState = m_pPowerStateList ;
        while (curState) {
            PSYSTEM_POWER_STATE psps=NULL;
            if (curState->AppsCanRequestState() 
                    && RegReadSystemPowerState( curState->GetStateString(), &psps, NULL) == ERROR_SUCCESS
                    && psps!=NULL) { // Here we do not use state intial flags because someone can change it on live.
                DWORD dwFlagsBit = CountBit(psps->dwFlags & dwHint );
                if (dwFlagsBit > bestMatchBit ) {
                    bestMatchBit = dwFlagsBit;
                    bestState = curState;
                }
            }
            if (psps)
                SystemPowerStateDestroy(psps);
            curState = curState->GetNextPowerState();
        }

        // if we have one, copy the name back to the caller
        if(bestState != NULL) {
            if(dwBufChars < (_tcslen(bestState->GetStateString()) + 1)) {
                dwStatus = ERROR_INSUFFICIENT_BUFFER;
            } else {
                _tcscpy(pszBuf, bestState->GetStateString());
                dwStatus = ERROR_SUCCESS;
                PMLOGMSG(ZONE_PLATFORM,(_T("%s: returning state \"%s\"\r\n"), pszFname, pszBuf));
            }
        }
    }
    DEBUGCHK(dwStatus == ERROR_SUCCESS);
    PMLOGMSG(dwStatus != ERROR_SUCCESS && ZONE_WARN,(_T("%s: returning %d\r\n"), pszFname, dwStatus));
    return dwStatus;
}
PowerState * DefaultPowerStateManager::GetFirstPowerState()
{
    SETFNAME(_T("PowerStateManager::GetFirstPowerState"));
    PowerState * curState = NULL;
    HANDLE hEvents[2];
    hEvents[0] = ghevPmShutdown;
    hEvents[1] = GetAPISignalHandle();
    DWORD dwStatus = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
    switch(dwStatus) {
    case (WAIT_OBJECT_0 + 0):
        PMLOGMSG(ZONE_INIT || ZONE_WARN, (_T("%s: shutdown event signaled, exiting\r\n"), pszFname));
        break;
    case (WAIT_OBJECT_0 + 1): {
        curState = m_pCurPowerState ;
        PMLOGMSG(ZONE_INIT, (_T("%s: initialization complete\r\n"), pszFname));
    }
        break;
    default:
        PMLOGMSG(ZONE_INIT || ZONE_WARN, (_T("%s: WaitForMultipleObjects() returned %d, exiting\r\n"),
            pszFname, dwStatus));
        break;
    }
    return curState;
}
PowerState * DefaultPowerStateManager::SetSystemState(PowerState * pCurPowerState )
{
    SETFNAME(_T("PowerStateManager::SetSystemState"));
    if (pCurPowerState!=NULL) {
        DWORD curState = pCurPowerState->GetState();
        DWORD newState = curState;
        do { //Switch to new stable state.
            newState = pCurPowerState->GetLastNewState();
            if (newState!= curState) {
                PowerState * pNewPowerState = GetStateObject(newState);
                if (pNewPowerState == NULL ) { // Wrong state all unspported state.
                    PMLOGMSG(ZONE_WARN || ZONE_PLATFORM,(_T("Unsported state %d \r\n"),newState));
                    ASSERT(FALSE);
                    newState = curState;
                }
                else if ( pNewPowerState!=pCurPowerState) {
                    PMLOGMSG(ZONE_INIT || ZONE_PLATFORM, (_T("%s: state change from \"%s\" to \"%s\" \r\n"  ), 
                        pszFname,pCurPowerState->GetStateString(),pNewPowerState->GetStateString())) ;
                    pCurPowerState = pNewPowerState;
                    pCurPowerState ->EnterState();
                    // Update to new state.
                    curState = newState;
                    newState = pCurPowerState->GetLastNewState();
                }
                else
                    newState = curState;
            }
        } while (newState!= curState) ; // Change to stable state.
        ASSERT(pCurPowerState!=NULL);
    }
    return pCurPowerState ;
}
DWORD   DefaultPowerStateManager::SendSystemPowerState(LPCWSTR pwsState, DWORD dwStateHint, DWORD dwOptions )
{
    TCHAR   szStateName[MAX_PATH];
    DWORD   dwStatus = ERROR_SUCCESS;
    int		prior;
    szStateName[0] = 0;
    SETFNAME(_T("PowerStateManager::SendSystemPowerState"));
    PMLOGMSG(ZONE_API, (_T("+%s: name %s, hint 0x%08x, options 0x%08x, fInternal %d\r\n"),
            pszFname, pwsState != NULL ? pwsState : _T("<NULL>"), dwStateHint, dwOptions));
    if (pwsState) {
        __try {
            StringCchCopy(szStateName, MAX_PATH, pwsState) ;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            PMLOGMSG(ZONE_WARN, (_T("%s: exception copying state name\r\n"), 
                pszFname));
            szStateName[0] = 0 ;
            dwStatus = ERROR_INVALID_PARAMETER;
        }
    } 
    else {
        // try to match the hint flag to a system state
        dwStatus = PlatformMapPowerStateHint(dwStateHint, szStateName, dim(szStateName));
    }
    
    PMLOGMSG(ZONE_API, (_T("+%s: name \"%s\", hint 0x%08x, options 0x%08x, fInternal %d\r\n"),
            pszFname,szStateName, dwStateHint, dwOptions));

    // go ahead and do the update?
    if(dwStatus == ERROR_SUCCESS) {
        Lock();
        DWORD activeState = SystemStateToActivityState(szStateName );
        if (activeState == PM_UNKNOWN_POWER_STATE ) {
            dwStatus = ERROR_INVALID_PARAMETER;
        }
        else {
            PowerState * pNewPowerState = GetStateObject(activeState);
            if (pNewPowerState && pNewPowerState->AppsCanRequestState()) {
                if ((dwOptions & POWER_DUMPDW)!=0) {
                    CaptureDumpFileOnDevice (GetCurrentProcessId (), GetCurrentThreadId (), NULL);
                }
				////////
				prior = CeGetThreadPriority(GetCurrentThread());
				CeSetThreadPriority(GetCurrentThread(), 219);
                pNewPowerState->EnterState();
                pNewPowerState = SetSystemState(pNewPowerState ) ;
                m_pCurPowerState = pNewPowerState;
                SetEvent(m_hSystemApiCalled);
				Sleep(0);
				CeSetThreadPriority(GetCurrentThread(), prior);
				///////
            }
            else 
                dwStatus = ERROR_INVALID_PARAMETER; 
        }
        Unlock();
    }
    return dwStatus;

}
DWORD   DefaultPowerStateManager::SystemStateToActivityState(LPCTSTR lpState )
{
    if (lpState == NULL)
        return PM_UNKNOWN_POWER_STATE;
    PowerState * curState = m_pPowerStateList ;
    while (curState) {
        if (_tcsicmp(curState->GetStateString(),lpState)==0)
            return curState->GetState();
        else
            curState = curState->GetNextPowerState();
    }
    return PM_UNKNOWN_POWER_STATE;

}
LPCTSTR DefaultPowerStateManager::ActivityStateToSystemState (DWORD platActiveState)
{
    PowerState * curState = m_pPowerStateList ;
    while (curState) {
        if (curState->GetState() == platActiveState )
            return curState->GetStateString() ;
        else
            curState = curState->GetNextPowerState();
    }
    return NULL;

}
PowerState * DefaultPowerStateManager::GetStateObject(DWORD newState)
{
    if (m_pPowerStateList != NULL) {
        PowerState * pCurState = m_pPowerStateList;
        while (pCurState) {
            if (pCurState-> GetState() ==  newState ) {
                return pCurState ;
            }
            pCurState = pCurState->GetNextPowerState();
        }
    }
    return NULL;
}
