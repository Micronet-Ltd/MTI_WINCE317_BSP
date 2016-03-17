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
#include <Csync.h>
#include <cRegEdit.h>
#include <pwsdef.h>
#include <notify.h>
#include <oal.h>
#include <oal_memory.h>
#include <oalex.h>

typedef enum {
    NoTimeoutItem,
    SuspendTimeout,
    SystemActivityTimeout,
    UserActivityTimeout,
} TIMEOUT_ITEM, *PTIMEOUT_ITEM;


    
#define STRING_ON _T("on")
#define STRING_USERIDLE _T("useridle")
#define STRING_SYSTEMIDLE _T("systemidle")
#define STRING_RESUMING _T("resuming")
#define STRING_SUSPEND _T("suspend")
#define STRING_ColdReboot _T("coldreboot")
#define STRING_Reboot _T("reboot")
#define STRING_ShutDown _T("ShutDown")
#define STRING_NOTIFY_EVENT _T("\\\\.\\Notifications\\NamedEvents\\____Notify_Wake_Event")

// platform-specific default values
#define DEF_TIMEOUTTOUSERIDLE           60          // in seconds, 0 to disable
#define DEF_TIMEOUTTOSYSTEMIDLE         300         // in seconds, 0 to disable
#define DEF_TIMEOUTTOSUSPEND            600         // in seconds, 0 to disable
#define DEF_TIMEOUTTOSHUTDOWN           3600        // in seconds, 0 to disable
#define DEF_TIMEOUTTORESUMETEST         30          // in seconds, 0 to disable

#define DEF_RESUMETEST_WAIT_TIMEOUT     1000        // im milliseconds



extern DWORD RegReadStateTimeout(HKEY hk, LPCTSTR pszName, DWORD dwDefault);
// need "C" linkage for compatibility with C language PDD implementations
extern "C" extern POWER_BROADCAST_POWER_INFO gSystemPowerStatus;
//extern "C" extern BOOL                       gWakeForResumeTest;

extern void SetGWESPowerUp();

class PowerStateManager: public DefaultPowerStateManager {
public:
    PowerStateManager(PVOID pParam) 
        : DefaultPowerStateManager (pParam)
    {
        m_fActiveManagement = TRUE;
		m_hNotify = NULL;
	 // create a named event
	    m_hNotifyWaitEvent = CreateEvent(NULL, FALSE, FALSE, _T("____Notify_Wake_Event"));
		m_hRTCWakeupEvent  = CreateEvent(NULL, FALSE, FALSE, _T("SYSTEM/RTC_Wakeup_Event"));
		if(m_hRTCWakeupEvent != INVALID_HANDLE_VALUE && GetLastError() == ERROR_ALREADY_EXISTS)
			m_hRTCWakeupEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE,_T("SYSTEM/RTC_Wakeup_Event"));
		m_dwSuspendShutdownState = 0;
		m_dwAlarmResolution = 10000;
		if(!KernelIoControl(IOCTL_KLIB_GETALARMRESOLUTION, 0, 0, (DWORD *)&m_dwAlarmResolution, sizeof(m_dwAlarmResolution), 0))
			m_dwAlarmResolution = 10000; 
    }
    ~PowerStateManager() {
        while (m_pPowerStateList!=NULL) {
            PowerState * pNextState = m_pPowerStateList->GetNextPowerState();
            delete m_pPowerStateList;
            m_pPowerStateList = pNextState;
        }
		if(m_hNotify)
		{
			CeClearUserNotification(m_hNotify);
			m_hNotify = NULL;
		}
		if(m_hNotifyWaitEvent)
		{
			CloseHandle(m_hNotifyWaitEvent);
			m_hNotifyWaitEvent = NULL;
		}
		if(m_hRTCWakeupEvent)
		{
			CloseHandle(m_hRTCWakeupEvent);
			m_hRTCWakeupEvent = NULL;
		}
    }


    BOOL Init() {
        if (DefaultPowerStateManager::Init() && m_pSystemActivity && m_pUserActivity) { // This is required by Default PM.
            PlatformLoadTimeouts();
            ReInitTimeOuts();
			ReInitWakeupTimeOuts();
            return CreatePowerStateList();
        }
        return FALSE;
    }

    virtual void ResetUserIdleTimeout(BOOL fIdle) {
        m_dwCurUserIdleTimeout = (fIdle? GetUserIdleTimeOut():INFINITE);
        if (m_dwCurUserIdleTimeout == 0 )
            m_dwCurUserIdleTimeout = INFINITE ;
       
    }

    virtual void ResetSystemIdleTimeTimeout(BOOL fIdle) {
       m_dwCurTimeoutToSystemIdle = (fIdle? GetSystemIdleTimeOut(): INFINITE);
        if (m_dwCurTimeoutToSystemIdle == 0)
            m_dwCurTimeoutToSystemIdle = INFINITE ;
    }

	virtual void ResetSuspendTimeTimeout(BOOL fIdle) {
        m_dwCurSuspendTimeout = (fIdle? GetSuspendTimeOut(): INFINITE);
        if (m_dwCurSuspendTimeout == 0)
            m_dwCurSuspendTimeout = INFINITE ;
    }

	virtual void ResetShutdownTimeout(BOOL flag ) {
		 m_dwCurShutdownTimeout = (flag? GetShutdownTimeOut(): INFINITE);
        if (m_dwCurShutdownTimeout == 0)
            m_dwCurShutdownTimeout = INFINITE ;

    }

	virtual void ResetResumeTestTimeout(BOOL flag) {

		m_dwCurResumeTestTimeout = (flag? GetResumeTestTimeOut(): INFINITE);
        if (m_dwCurResumeTestTimeout == 0)
            m_dwCurResumeTestTimeout = INFINITE ;
    }

    virtual void SubtractTimeout(DWORD dwTicks) {
        if (m_dwCurSuspendTimeout != INFINITE )
            m_dwCurSuspendTimeout = (m_dwCurSuspendTimeout>dwTicks? m_dwCurSuspendTimeout- dwTicks: 0 );
        if (m_dwCurTimeoutToSystemIdle != INFINITE)
            m_dwCurTimeoutToSystemIdle = (m_dwCurTimeoutToSystemIdle>dwTicks? m_dwCurTimeoutToSystemIdle- dwTicks: 0 );
        if (m_dwCurUserIdleTimeout!= INFINITE)
            m_dwCurUserIdleTimeout =  (m_dwCurUserIdleTimeout> dwTicks? m_dwCurUserIdleTimeout - dwTicks: 0 );
    }

	virtual void SubtractWakeupTimeout(DWORD dwTicks) {
        if (m_dwCurShutdownTimeout != INFINITE )
            m_dwCurShutdownTimeout = (m_dwCurShutdownTimeout>dwTicks? m_dwCurShutdownTimeout- dwTicks: 0 );
    }

	virtual DWORD GetCurShutdownTimeOut() {
		return m_dwCurShutdownTimeout;
    }

	virtual void SetSuspendShutdownState (BOOL flag)
	{
		if(flag)
			m_dwSuspendShutdownState = 1;
		else
			m_dwSuspendShutdownState = 0;
	}

	virtual DWORD GetSuspendShutdownState ()
	{
		return m_dwSuspendShutdownState;
	}


    virtual DWORD   ThreadRun() {
        SETFNAME(_T("PowerStateManager::ThreadRun"));
        // Assume First state is PowerStateOn. So please put POwerStateOn on the header.
        if (m_pPowerStateList) {
            // We need get first SetSystemPower from device to make initial power state correct.
            PowerState * pCurPowerState = GetFirstPowerState();
            if (pCurPowerState != NULL) {
                Lock();
                m_pCurPowerState =  pCurPowerState;
                pCurPowerState->EnterState();
                BOOL fDone = FALSE;
                // Create Legacy Registry modify notification event array.
                while (!fDone && pCurPowerState) {
                    PLATFORM_ACTIVITY_STATE curState = (PLATFORM_ACTIVITY_STATE) pCurPowerState->GetState();
                    PLATFORM_ACTIVITY_STATE newState = curState;
                    Unlock();
					RETAILMSG(0, (_T("%s: %s %d\r\n"  ), pszFname, (pCurPowerState)?pCurPowerState->GetStateString():L"<null>", GetTickCount())) ;
                    PLATFORM_ACTIVITY_EVENT activityEvent = pCurPowerState->WaitForEvent(INFINITE,0 ,NULL);
                    PMLOGMSG(ZONE_PLATFORM, (_T("%s: activityEvent = %d  \r\n"  ), pszFname,activityEvent)) ;
                    Lock();
                    if (pCurPowerState != m_pCurPowerState) {
                        if (activityEvent!= SystemPowerStateAPI) { 
                            PMLOGMSG(ZONE_ERROR|| ZONE_PLATFORM,(_T("Multiple Event happens during SentSystemPowerState %d, Event will evaluated in new state \r\n"),newState));
                        }
                        pCurPowerState = m_pCurPowerState;
                        newState = curState = (PLATFORM_ACTIVITY_STATE) pCurPowerState->GetState();
                        if ((PLATFORM_ACTIVITY_STATE) pCurPowerState->GetState() == Resuming 
                                || (PLATFORM_ACTIVITY_STATE)pCurPowerState->GetState() == Suspend) {
                            // Application call SetSystemPowerState set to Resuming or Suspend in other thread.
                            // This thread come back, calculate time elapsed and update it. So timeout is wrong 
                            // we need re-intialize timeout paramenter
                            ReInitTimeOuts( ) ;
                        }
                    }
                    switch (activityEvent) {
                    case PmShutDown :
                        fDone = TRUE;
                        break;
                    case PmReloadActivityTimeouts:
						PlatformLoadTimeouts(); // No break we need run ReInitTimeouts.
					case PowerSourceChange:
						ReInitTimeOuts();
						ReInitWakeupTimeOuts();
                        pCurPowerState->DefaultEventHandle( activityEvent ) ;
						break;
                    case RestartTimeouts:
                    case SystemPowerStateChange:
                    case PowerButtonPressed: 
                    case AppButtonPressed:
					case SuspButtonPressed:
                        ReInitTimeOuts();
                        pCurPowerState->DefaultEventHandle( activityEvent ) ;
                        break;
                    case SystemPowerStateAPI:
                        break;
                    default:
                        pCurPowerState->DefaultEventHandle( activityEvent ) ;
                        break;
                    }
                    pCurPowerState = SetSystemState(pCurPowerState ) ;
                    ASSERT(pCurPowerState!=NULL);
                    m_pCurPowerState =  pCurPowerState; // Update current state.
                }
                Unlock();
            }
            else
                ASSERT(FALSE);
        }
        return 0;
    }
    virtual void ReAdjustTimeOuts( )  {
        if (m_dwCurSuspendTimeout > GetSuspendTimeOut() ) {
            if ((m_dwCurSuspendTimeout=GetSuspendTimeOut()) == 0 )
                m_dwCurSuspendTimeout = INFINITE ;
        }
        if (m_dwCurTimeoutToSystemIdle > GetSystemIdleTimeOut() ) {
            if ((m_dwCurTimeoutToSystemIdle=GetSystemIdleTimeOut()) == 0 )
                m_dwCurTimeoutToSystemIdle = INFINITE ;
        }
        if (m_dwCurUserIdleTimeout > GetUserIdleTimeOut() ) {
            if ((m_dwCurUserIdleTimeout=GetUserIdleTimeOut()) == 0)
                m_dwCurUserIdleTimeout = INFINITE ;
        }
    }
    
    virtual DWORD GetSmallestTimeout(PTIMEOUT_ITEM pTimeoutItem) {
        DWORD dwReturn = INFINITE;
        TIMEOUT_ITEM activeEvent = NoTimeoutItem;
        if (dwReturn > m_dwCurSuspendTimeout) {
            dwReturn = m_dwCurSuspendTimeout;
            activeEvent = SuspendTimeout ;
        }
        if (dwReturn > m_dwCurTimeoutToSystemIdle) {
            dwReturn = m_dwCurTimeoutToSystemIdle;
            activeEvent = SystemActivityTimeout ;
        }
        if (dwReturn > m_dwCurUserIdleTimeout ) {
            dwReturn = m_dwCurUserIdleTimeout;
            activeEvent = UserActivityTimeout;
        }
        if (pTimeoutItem) {
            *pTimeoutItem = activeEvent;
        }
        return dwReturn;
    }


	virtual DWORD GetSmallestWakeupTimeout() {

		DWORD ret = INFINITE;

		if(m_dwCurShutdownTimeout !=INFINITE && m_dwCurResumeTestTimeout !=INFINITE)
		{      
			ret =  m_dwCurShutdownTimeout > m_dwCurResumeTestTimeout ? m_dwCurResumeTestTimeout:m_dwCurShutdownTimeout;
		}
		else if (m_dwCurShutdownTimeout!=INFINITE)
			ret = m_dwCurShutdownTimeout;
		else
			ret = m_dwCurResumeTestTimeout;

		return ret;


    }
	virtual FILETIME GetSuspendStartTime()
	{
		return m_fSuspendStartTime;
	}

	virtual void SetSuspendStartTime(FILETIME time)
	{
		memcpy(&m_fSuspendStartTime,&time, sizeof(time));
	}


    void DisableUserIdleTimeout() { m_dwCurUserIdleTimeout = INFINITE; };
    void DisableSuspendTimeout() { m_dwCurSuspendTimeout = INFINITE; };
    void DisableSystemIdleTimeout() { m_dwCurTimeoutToSystemIdle = INFINITE; };
	void DisableShutdownTimeout() { m_dwCurShutdownTimeout = INFINITE; };
	void DisableResumeTestTimeout() { m_dwCurResumeTestTimeout = INFINITE; };

    // Timer Function.
    virtual void ReInitTimeOuts() {
        m_dwCurSuspendTimeout = GetSuspendTimeOut();
        m_dwCurTimeoutToSystemIdle = GetSystemIdleTimeOut();
        m_dwCurUserIdleTimeout = GetUserIdleTimeOut() ;
        // If timer is not set. It is not supported.
        if (m_dwCurUserIdleTimeout == 0 )
            m_dwCurUserIdleTimeout = INFINITE ;
        if (m_dwCurTimeoutToSystemIdle == 0)
            m_dwCurTimeoutToSystemIdle = INFINITE ;
        if (m_dwCurSuspendTimeout == 0)
            m_dwCurSuspendTimeout = INFINITE ;
    }

	 // Timer Function.
    virtual void ReInitWakeupTimeOuts() {
		RETAILMSG(1, (_T("ReInitWakeupTimeOuts started\r\n")));
        m_dwCurShutdownTimeout = GetShutdownTimeOut();
		m_dwCurResumeTestTimeout = GetResumeTestTimeOut();
        if (m_dwCurShutdownTimeout == 0 )
            m_dwCurShutdownTimeout = INFINITE ;
        if (m_dwCurResumeTestTimeout == 0)
            m_dwCurResumeTestTimeout = INFINITE ;
    }



    virtual void   PlatformResumeSystem(BOOL fSuspened) {
        SETFNAME(_T("PowerStateManager::PlatformResumeSystem"));
        TCHAR szResumeState[MAX_PATH];
        DWORD dwStatus;
        HANDLE hevActivityReset = NULL;
        
        PMLOGMSG(ZONE_RESUME, (_T("+%s: suspend flag is %d\r\n"), pszFname, fSuspened));
		RETAILMSG(1, (_T("+%s: suspend flag is %d\r\n"), pszFname, fSuspened));
        
        // Was this an unexpected resume event?  If so, there may be a thread priority problem
        // or some piece of software suspended the system without calling SetSystemPowerState().
        DEBUGCHK(fSuspened);
        if(!fSuspened) {
            // Unexpected resume -- turn everything back on.  OEMs may choose to customize this
            // routine to dynamically determine which system power state is most appropriate rather
            // than using PlatformMapPowerStateHint().
            RETAILMSG(0, (_T("%s: WARNING: unexpected resume!\r\n"), pszFname));
            dwStatus = PlatformMapPowerStateHint(POWER_STATE_ON, szResumeState, dim(szResumeState));
            DEBUGCHK(dwStatus == ERROR_SUCCESS);
            
            // Go into the new state.  OEMs that choose to support unexpected resumes may want to
            // lock PM variables with PMLOCK(), then set the curDx and actualDx values for all
            // devices to PwrDeviceUnspecified before calling PmSetSystemPowerState_I().  This will
            // force an update IOCTL to all devices.
            dwStatus = PmSetSystemPowerState_I(szResumeState, 0, POWER_FORCE, TRUE);
            DEBUGCHK(dwStatus == ERROR_SUCCESS);
        } else if(m_fActiveManagement) {
            DWORD dwWakeSource, dwBytesReturned;
            BOOL fOk;
            
            // get the system wake source to help determine which power state we resume into
            fOk = KernelIoControl(IOCTL_HAL_GET_WAKE_SOURCE, NULL, 0, &dwWakeSource, sizeof(dwWakeSource), &dwBytesReturned);
            if(fOk) {
                // ioctl succeeded (not all platforms necessarily support it), but sanity check
                // the return value, just in case.
                if(dwBytesReturned != sizeof(dwWakeSource)) {
                    PMLOGMSG(ZONE_WARN, (_T("%s: KernelIoControl() returned an invalid size %d\r\n"),
                        pszFname, dwBytesReturned));
                } else {
                    // look for an activity timer corresponding to this wake source
                    PACTIVITY_TIMER pat = ActivityTimerFindByWakeSource(dwWakeSource);
                    if(pat != NULL) {
                        PMLOGMSG(ZONE_RESUME || ZONE_TIMERS, (_T("%s: signaling '%s' activity at resume\r\n"),
                            pszFname, pat->pszName));
                        hevActivityReset = pat->hevReset;
                    }
                }
            }
            
            // did we find an activity timer?
            if(hevActivityReset == NULL) {
                RETAILMSG(0, (_T("%s: assuming user activity\r\n"), pszFname));
                hevActivityReset = (m_pUserActivity!=NULL ?m_pUserActivity->hevReset: NULL);
            }
        }
        
        // is there an activity timer we need to reset?
        if(hevActivityReset != NULL) {
            // found a timer, elevate the timer management priority thread so that it 
            // executes before the suspending thread
            DWORD dwOldPriority = CeGetThreadPriority(ghtActivityTimers);
            DWORD dwNewPriority = (CeGetThreadPriority(GetCurrentThread()) - 1);
            DEBUGCHK(dwNewPriority >= 0);
            SetEvent(hevActivityReset);
            CeSetThreadPriority(ghtActivityTimers, dwNewPriority);
            CeSetThreadPriority(ghtActivityTimers, dwOldPriority);
        }
        
        PMLOGMSG(ZONE_RESUME, (_T("-%s\r\n"), pszFname));
    }

	
	PLATFORM_ACTIVITY_STATE   WakeUpSourceToPlatformState() 
	{ 
		PACTIVITY_TIMER pat = NULL;
		PLATFORM_ACTIVITY_STATE state = On;
	    
		PMLOGMSG(ZONE_RESUME, (_T("+WakeUpSourceToPlatformState\r\n")));

		if(m_fActiveManagement) {
			DWORD dwWakeSource, dwBytesReturned;
			BOOL fOk;
	        
			// get the system wake source to help determine which power state we resume into
			fOk = KernelIoControl(IOCTL_HAL_GET_WAKE_SOURCE, 0, 0, &dwWakeSource, sizeof(dwWakeSource), &dwBytesReturned);
			if(fOk)
			{
				// ioctl succeeded (not all platforms necessarily support it), but sanity check
				// the return value, just in case.
				RETAILMSG(0, (_T("WakeUpSourceToPlatformState: Wakeup source(%d)\r\n"), dwWakeSource));
				if(dwBytesReturned != sizeof(dwWakeSource))
				{
					RETAILMSG(1, (_T("WakeUpSourceToPlatformState: KernelIoControl() returned an invalid size %d\r\n"), dwBytesReturned));
				}
				else
				{
					// look for an activity timer corresponding to this wake source
					pat = ActivityTimerFindByWakeSource(dwWakeSource);
				}
			}

			if (pat == m_pSystemActivity)
				state = UserIdle;
			RETAILMSG(0, (_T("WakeUpSourceToPlatformState: Power state (%d, %s)\r\n"), state, (pat && pat->pszName)?pat->pszName:L"<nul>"));
		}

		PMLOGMSG(ZONE_RESUME, (_T("-WakeUpSourceToPlatformState\r\n")));

    return state;
	}


	 // Timeout Functions.

	DWORD GetSuspendTimeOut () {
        return (gSystemPowerStatus.bACLineStatus == AC_LINE_OFFLINE? m_dwBattSuspendTimeout: m_dwACSuspendTimeout);
    }
    DWORD GetSystemIdleTimeOut() {
        return (gSystemPowerStatus.bACLineStatus == AC_LINE_OFFLINE? m_dwBattTimeoutToSystemIdle: m_dwACTimeoutToSystemIdle);
    }
    DWORD GetUserIdleTimeOut() {
        return (gSystemPowerStatus.bACLineStatus == AC_LINE_OFFLINE? m_dwBattUserIdleTimeout: m_dwACUserIdleTimeout);
    }
	DWORD GetShutdownTimeOut() {
        return (gSystemPowerStatus.bACLineStatus == AC_LINE_OFFLINE? m_dwBattShutdownTimeout: m_dwACShutdownTimeout);
    }

	DWORD GetResumeTestTimeOut() {
        return (gSystemPowerStatus.bACLineStatus == AC_LINE_OFFLINE? m_dwBattResumeTestTimeout: m_dwACResumeTestTimeout);
    }


	DWORD EnableWakeUp(DWORD dwSeconds)
	{
		
		CE_NOTIFICATION_TRIGGER sTrigger;
	
		SYSTEMTIME curSysTime = {0};
		FILETIME fileTime;

		if(!m_hNotifyWaitEvent)
		{
			RETAILMSG(1, (_T("EnableWakeUp invalid m_hNotifyWaitEvent params \r\n")));
			return 0;
		}


		RETAILMSG(1, (_T("EnableWakeUp: szNotifyEventName=%s, seconds = %d \r\n" ), STRING_NOTIFY_EVENT,dwSeconds));


		sTrigger.dwSize = sizeof(CE_NOTIFICATION_TRIGGER);
		//Time based notification
		sTrigger.dwType = CNT_TIME;     
		sTrigger.dwEvent = 0;  

		// this will be a named event
		sTrigger.lpszApplication = STRING_NOTIFY_EVENT;  
		sTrigger.lpszArguments = NULL;



		GetLocalTime(&curSysTime);
		/*
		RETAILMSG(1, (_T("The start local time is: %04d.%02d.%02d %02d:%02d:%02d.%03d \r\n"),
						curSysTime.wYear, curSysTime.wMonth, curSysTime.wDay,
						curSysTime.wHour, curSysTime.wMinute, curSysTime.wSecond, curSysTime.wMilliseconds));
						*/

		SystemTimeToFileTime(&curSysTime, &fileTime);
		unsigned __int64 t,t_new;
		memcpy(&t, &fileTime, sizeof(t));
		t += ( __int64 )dwSeconds * ( __int64 )10000000;
		t = t/10000;
		//t = (t*(__int64)m_dwAlarmResolution + (m_dwAlarmResolution/2))/(__int64)(m_dwAlarmResolution);
		t_new = (t/(__int64)m_dwAlarmResolution) *(__int64)m_dwAlarmResolution;
		if(t_new<t)
			t_new += m_dwAlarmResolution;

		t_new = t_new*10000;
		memcpy(&fileTime, &t_new, sizeof(t_new));
		FileTimeToSystemTime(&fileTime, &curSysTime);

		
		RETAILMSG(0, (_T("The end local time is: %04d.%02d.%02d %02d:%02d:%02d.%03d \r\n"),
						curSysTime.wYear, curSysTime.wMonth, curSysTime.wDay,
						curSysTime.wHour, curSysTime.wMinute, curSysTime.wSecond, curSysTime.wMilliseconds));
						


		sTrigger.stStartTime = curSysTime;
		sTrigger.stEndTime   = curSysTime;

		// create user notification
		m_hNotify = CeSetUserNotificationEx( m_hNotify, &sTrigger, NULL );

		return 1;
	}

	void DisableWakeUp()
	{

		RETAILMSG(0, (_T("DisableWakeUp \r\n")));
		if (m_hNotify)
		{
			CeClearUserNotification(m_hNotify);
			m_hNotify = NULL;
		}
	}

	BOOL IsWakedForResumeTest()
	{
		BOOL ret = FALSE, need_to_sleep = 0;

		if(!m_hNotifyWaitEvent ||!m_hRTCWakeupEvent)
		{
			RETAILMSG(1, (_T("IsWakedForResumeTest invalid m_hNotifyWaitEvent value \r\n")));
			return FALSE;
		}
		if(WaitForSingleObject(m_hRTCWakeupEvent, 0) != WAIT_OBJECT_0)
		{
			RETAILMSG(0, (_T("IsWakedForResumeTest: is not RTC\r\n")));
			return FALSE;
		}
		if(GetEventData(m_hRTCWakeupEvent))
		{
			RETAILMSG(0, (_T("IsWakedForResumeTest: Wake for user alarm\r\n")));
			return TRUE;
		}
		RETAILMSG(0, (_T("IsWakedForResumeTest: Wake for Battery check %x\r\n"), GetLastError()));
//RTC event without RTC data
		SYSTEMTIME curSysTime = {0};
		unsigned __int64 nowTime, evTime;
		HANDLE	hn;
		DWORD	dwHowMany;
		DWORD	*buff = 0;
		BOOL	fOtherEv = 0, nRes;
	
		if(!CeGetUserNotificationHandles(0, 0, &dwHowMany))
		{
			RETAILMSG(1, (_T("IsWakedForResumeTest: CeGetUserNotificationHandles failed\r\n")));
			return ret;
		}
		buff = (DWORD *)PmAlloc(dwHowMany*sizeof(HANDLE));
		
		if(!buff)
		{
			RETAILMSG(1, (_T("IsWakedForResumeTest: allocation failed %x\r\n"), buff));
			return ret;
		}

		if(!IsNamedEventSignaled(NOTIFICATION_EVENTNAME_API_SET_READY  , 0))
		{
			RETAILMSG(1, (_T("IsWakedForResumeTest: NOTIFICATION API SET NOT READY\r\n")));
		}

		if(!CeGetUserNotificationHandles ((HANDLE *)buff, dwHowMany, &dwHowMany))
		{
			RETAILMSG(1, (_T("IsWakedForResumeTest: CeGetUserNotificationHandles failed, alloc %x\r\n"), buff));
			PmFree(buff);
			return ret;
		}
		
		for(DWORD i=0; i < dwHowMany; i++)
		{
			DWORD *buf = 0;
			DWORD need;
			hn = (HANDLE)buff[i];

			CeGetUserNotification(hn, 0, &need, 0);
			if(ERROR_INVALID_PARAMETER == GetLastError())
			{
				RETAILMSG(1, (_T("IsWakedForResumeTest: CeGetUserNotification failed\r\n")));
				continue;
			}
			buf = (DWORD*)PmAlloc(need);
			if(buf)
			{
				nRes = CeGetUserNotification(hn, need, &need, (unsigned char *)buf);
				CE_NOTIFICATION_INFO_HEADER *head = (CE_NOTIFICATION_INFO_HEADER *)buf;
				if(nRes && (head->dwStatus == CNS_SIGNALLED))
				{
					RETAILMSG(1, (_T("IsWakedForResumeTest: The notification is currently active\r\n")));
					RETAILMSG(head->pcent,(L"%d. hn 0x%x 0x%x st %d ty %d ev %d %s\r\n%d/%d/%d  %d:%d:%d\r\n", i + 1,
								hn, 
								head->hNotification,
								head->dwStatus, 
								head->pcent->dwType,
								head->pcent->dwEvent,
								head->pcent->lpszApplication, 
								head->pcent->stStartTime.wDay,
								head->pcent->stStartTime.wMonth,
								head->pcent->stStartTime.wYear,
								head->pcent->stStartTime.wHour,
								head->pcent->stStartTime.wMinute,
								head->pcent->stStartTime.wSecond
								));
				}
				else if(nRes && head->pcent)
				{
					RETAILMSG(1,(L"%d. hn 0x%x 0x%x st %d ty %d ev %d %s\r\n%d/%d/%d  %d:%d:%d\r\n", i + 1,
								hn, 
								head->hNotification,
								head->dwStatus, 
								head->pcent->dwType,
								head->pcent->dwEvent,
								head->pcent->lpszApplication, 
								head->pcent->stStartTime.wDay,
								head->pcent->stStartTime.wMonth,
								head->pcent->stStartTime.wYear,
								head->pcent->stStartTime.wHour,
								head->pcent->stStartTime.wMinute,
								head->pcent->stStartTime.wSecond
								));
					if(CNT_TIME == head->pcent->dwType && head->pcent->lpszApplication)
					{
						GetLocalTime(&curSysTime);
						SystemTimeToFileTime(&curSysTime, (FILETIME*)&nowTime);
						SystemTimeToFileTime(&head->pcent->stStartTime, (FILETIME*)&evTime);

						if(evTime <= nowTime + m_dwAlarmResolution * 10000)
						{
							if(0 == wcscmp(head->pcent->lpszApplication, STRING_NOTIFY_EVENT))
							{
								RETAILMSG(1,(L"IsWakedForResumeTest: Battery check found %d\r\n", GetTickCount()));
								need_to_sleep = TRUE;
							
								if(head->hNotification != m_hNotify)
									CeClearUserNotification(head->hNotification);
							}
							else
							{
								fOtherEv = TRUE;
								RETAILMSG(1,(L"IsWakedForResumeTest: other alarm found %d\r\n", GetTickCount()));
							}
						}
					}
				}
				PmFree(buf);

				if(fOtherEv)
					break;
			}
		}

		PmFree(buff);

		if(WAIT_OBJECT_0 == WaitForSingleObject(m_hNotifyWaitEvent ,0))
		{
			m_hNotify		= NULL;
			need_to_sleep	= TRUE;
		}

		if(fOtherEv)
			ret = 0;
		else if(need_to_sleep)
			ret = 1;
		
//		SYSTEMTIME curSysTime = {0};
//		GetLocalTime(&curSysTime);
                   
		RETAILMSG(0, (_T("The system time is: %04d.%02d.%02d %02d:%02d:%02d \r\n"),
						curSysTime.wYear, curSysTime.wMonth, curSysTime.wDay,
						curSysTime.wHour, curSysTime.wMinute, curSysTime.wSecond));

		return ret;
	}


protected:        
    virtual void    PlatformLoadTimeouts() {
        DWORD dwStatus;
        TCHAR szPath[MAX_PATH];
        HKEY hk;
        SETFNAME(_T("PowerStateManager::PlatformLoadTimeouts"));

        // assume default values
        m_dwACSuspendTimeout = DEF_TIMEOUTTOSUSPEND * 1000;
        m_dwACTimeoutToSystemIdle = DEF_TIMEOUTTOSYSTEMIDLE * 1000 ;
        m_dwACUserIdleTimeout = DEF_TIMEOUTTOUSERIDLE * 1000;
		m_dwACShutdownTimeout = DEF_TIMEOUTTOSHUTDOWN * 1000;
		m_dwACResumeTestTimeout = DEF_TIMEOUTTORESUMETEST * 1000;
        
        m_dwBattSuspendTimeout = DEF_TIMEOUTTOSUSPEND * 1000;
        m_dwBattTimeoutToSystemIdle = DEF_TIMEOUTTOSYSTEMIDLE * 1000 ;
        m_dwBattUserIdleTimeout = DEF_TIMEOUTTOUSERIDLE * 1000;
		m_dwBattShutdownTimeout = DEF_TIMEOUTTOSHUTDOWN * 1000;
		m_dwBattResumeTestTimeout = DEF_TIMEOUTTORESUMETEST * 1000;

        // get timeout thresholds for transitions between states
        StringCchPrintf(szPath, MAX_PATH, _T("%s\\%s"), PWRMGR_REG_KEY, _T("Timeouts"));
        dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPath, 0, 0, &hk);
        if(dwStatus == ERROR_SUCCESS) {
            // read system power state timeouts
            m_dwACSuspendTimeout = RegReadStateTimeout(hk, _T("ACSuspend"), DEF_TIMEOUTTOSUSPEND);
            m_dwACTimeoutToSystemIdle = RegReadStateTimeout(hk, _T("ACSystemIdle"), DEF_TIMEOUTTOSYSTEMIDLE);
            m_dwACUserIdleTimeout = RegReadStateTimeout(hk, _T("ACUserIdle"), DEF_TIMEOUTTOUSERIDLE);
			m_dwACShutdownTimeout = RegReadStateTimeout(hk, _T("ACOff"), DEF_TIMEOUTTOSHUTDOWN);
			m_dwACResumeTestTimeout =RegReadStateTimeout(hk, _T("ACResumeTest"), DEF_TIMEOUTTORESUMETEST);
             
            m_dwBattSuspendTimeout = RegReadStateTimeout(hk, _T("BattSuspend"), DEF_TIMEOUTTOUSERIDLE);
            m_dwBattTimeoutToSystemIdle = RegReadStateTimeout(hk, _T("BattSystemIdle"), DEF_TIMEOUTTOSYSTEMIDLE);
            m_dwBattUserIdleTimeout = RegReadStateTimeout(hk, _T("BattUserIdle"), DEF_TIMEOUTTOSUSPEND);
			m_dwBattShutdownTimeout = RegReadStateTimeout(hk, _T("BattOff"), DEF_TIMEOUTTOSHUTDOWN);
			m_dwBattResumeTestTimeout =RegReadStateTimeout(hk, _T("BattResumeTest"), DEF_TIMEOUTTORESUMETEST);
             
            // release resources
            RegCloseKey(hk);
        }
        // Update Legacy if we find them.
        PMLOGMSG(ZONE_INIT || ZONE_PLATFORM, 
            (_T("%s: ACSuspendTimeout %d, ACTimeoutToSystemIdle %d, ACUserIdleTimeout %d \r\n"), pszFname, m_dwACSuspendTimeout, m_dwACTimeoutToSystemIdle, m_dwACUserIdleTimeout));
        PMLOGMSG(ZONE_INIT || ZONE_PLATFORM, 
            (_T("%s: BattSuspendTimeout %d,BattTimeoutToSystemIdle %d, BattUserIdleTimeout%d \r\n"  ),pszFname, m_dwBattSuspendTimeout,m_dwBattTimeoutToSystemIdle, m_dwBattUserIdleTimeout));
    }
    
    virtual BOOL    CreatePowerStateList(); // Implement this at last.
    
protected:
    
    // TimeOut Parameter.
    DWORD      m_dwACSuspendTimeout;
    DWORD      m_dwACTimeoutToSystemIdle ;
    DWORD      m_dwACUserIdleTimeout;
	DWORD      m_dwACShutdownTimeout;
	DWORD      m_dwACResumeTestTimeout;
    
    DWORD      m_dwBattSuspendTimeout ;
    DWORD      m_dwBattTimeoutToSystemIdle ;
    DWORD      m_dwBattUserIdleTimeout;
	DWORD      m_dwBattShutdownTimeout;
	DWORD      m_dwBattResumeTestTimeout;
        
    DWORD	   m_dwCurSuspendTimeout;
    DWORD      m_dwCurTimeoutToSystemIdle ;
    DWORD      m_dwCurUserIdleTimeout;
	DWORD      m_dwCurShutdownTimeout;	
	DWORD      m_dwCurResumeTestTimeout;
	DWORD      m_dwSuspendShutdownState;
	DWORD      m_dwAlarmResolution;

	FILETIME   m_fSuspendStartTime;
    BOOL       m_fActiveManagement;
	HANDLE     m_hNotify;
	HANDLE     m_hNotifyWaitEvent;
	HANDLE     m_hRTCWakeupEvent;

};


class PowerStateOn : public PowerState {
public:
    PowerStateOn(PowerStateManager *pPwrStateMgr, PowerState * pNextPowerState = NULL )      
    :   PowerState(pPwrStateMgr,pNextPowerState)
    {;  };
    virtual void EnterState() {
        PowerState::EnterState();

		if (m_pPwrStateMgr->GetUserActivityTimer()&& ((PowerStateManager *)m_pPwrStateMgr)->GetUserIdleTimeOut()!=0)
			m_dwEventArray [ PM_USER_ACTIVITY_EVENT ] = m_pPwrStateMgr->GetUserActivityTimer()->hevReset;
		else
		    m_dwEventArray [ PM_USER_ACTIVITY_EVENT ] = m_hUnsignaledHandle;

        // to not wait for system activity event in this state
		m_dwEventArray [ PM_SYSTEM_ACTIVITY_EVENT ] = m_hUnsignaledHandle;

        ((PowerStateManager *)m_pPwrStateMgr)->ReInitTimeOuts( );
    }

    // This state does not need Resume Time out.
    virtual PLATFORM_ACTIVITY_EVENT  WaitForEvent(DWORD dwTimeouts = INFINITE , DWORD dwNumOfExternEvent = 0, HANDLE * pExternEventArray = NULL) {
        TIMEOUT_ITEM TimeoutItem;
        ((PowerStateManager *) m_pPwrStateMgr)->DisableSuspendTimeout();
        ((PowerStateManager *) m_pPwrStateMgr)->DisableSystemIdleTimeout();
        DWORD dwTimeout =((PowerStateManager *) m_pPwrStateMgr)->GetSmallestTimeout(&TimeoutItem);
        PLATFORM_ACTIVITY_EVENT activeEvent = PowerState::WaitForEvent(dwTimeout ,dwNumOfExternEvent, pExternEventArray) ;
        switch (activeEvent) {
            case Timeout: {
                switch (TimeoutItem) {
                    case UserActivityTimeout:
                        m_LastNewState = UserIdle;
                        break;
                    default:
                        ASSERT(FALSE);
                        break;
                }
                break;
            }
        }
        return activeEvent;
    }
    virtual DWORD  GetState() { return (DWORD)On; };
    virtual LPCTSTR GetStateString() { return STRING_ON ; };
    virtual DWORD StateValidateRegistry(DWORD  , DWORD  ) {
        return PowerState::StateValidateRegistry(0, POWER_STATE_ON|POWER_STATE_PASSWORD);
    }
    virtual BOOL AppsCanRequestState() { return TRUE; }
    
};

class PowerStateUserIdle : public PowerState {
public:
    PowerStateUserIdle(PowerStateManager *pPwrStateMgr, PowerState * pNextPowerState = NULL )     
    :   PowerState(pPwrStateMgr,pNextPowerState)
    {;  };
    virtual void EnterState() {
        PowerState::EnterState();

		if (m_pPwrStateMgr->GetUserActivityTimer()&& ((PowerStateManager *)m_pPwrStateMgr)->GetUserIdleTimeOut()!=0)
			m_dwEventArray [ PM_USER_ACTIVITY_EVENT ] = m_pPwrStateMgr->GetUserActivityTimer()->hevReset;
		else
		    m_dwEventArray [ PM_USER_ACTIVITY_EVENT ] = m_hUnsignaledHandle;


		if (m_pPwrStateMgr->GetSystemActivityTimer()&& ((PowerStateManager *)m_pPwrStateMgr)->GetSystemIdleTimeOut()!=0)
			m_dwEventArray [ PM_SYSTEM_ACTIVITY_EVENT ] = m_pPwrStateMgr->GetSystemActivityTimer()->hevReset;
		else
		    m_dwEventArray [ PM_SYSTEM_ACTIVITY_EVENT ] = m_hUnsignaledHandle;

        

        ((PowerStateManager *)m_pPwrStateMgr)->ReInitTimeOuts( );
    }
    // This state does not need Resume Time out.
    virtual PLATFORM_ACTIVITY_EVENT  WaitForEvent(DWORD dwTimeouts = INFINITE , DWORD dwNumOfExternEvent = 0, HANDLE * pExternEventArray = NULL) {
        TIMEOUT_ITEM TimeoutItem;
        ((PowerStateManager *)m_pPwrStateMgr)->DisableUserIdleTimeout();
        ((PowerStateManager *) m_pPwrStateMgr)->DisableSuspendTimeout();
        DWORD dwTimeout = ((PowerStateManager *)m_pPwrStateMgr)->GetSmallestTimeout(&TimeoutItem);
        PLATFORM_ACTIVITY_EVENT activeEvent = PowerState::WaitForEvent(dwTimeout ,dwNumOfExternEvent, pExternEventArray) ;
        switch (activeEvent) {
            case UserActivity: 
                m_LastNewState = On;
                break;
            case Timeout: {
                switch (TimeoutItem) {
                    case SystemActivityTimeout:
                        m_LastNewState = SystemIdle;
                        break;
                    default:
                        ASSERT(FALSE);
                        break;
                }
                break;
            }
        }
        return activeEvent;
    }
    virtual DWORD  GetState() { return (DWORD)UserIdle; };
    virtual LPCTSTR GetStateString() { return STRING_USERIDLE; };
    virtual DWORD StateValidateRegistry(DWORD dwDState , DWORD dwFlag ) {
        return PowerState::StateValidateRegistry(1, POWER_STATE_PASSWORD|POWER_STATE_USERIDLE );
    }
    virtual BOOL AppsCanRequestState() { return TRUE; }
    
};

class PowerStateSystemIdle : public PowerState {
public:
    PowerStateSystemIdle(PowerStateManager *pPwrStateMgr, PowerState * pNextPowerState = NULL )     
    :   PowerState(pPwrStateMgr,pNextPowerState)
    {;  };
    virtual void EnterState() {
        PowerState::EnterState();

		if (m_pPwrStateMgr->GetUserActivityTimer()&& ((PowerStateManager *)m_pPwrStateMgr)->GetUserIdleTimeOut()!=0)
			m_dwEventArray [ PM_USER_ACTIVITY_EVENT ] = m_pPwrStateMgr->GetUserActivityTimer()->hevReset;
		else
		    m_dwEventArray [ PM_USER_ACTIVITY_EVENT ] = m_hUnsignaledHandle;


		if (m_pPwrStateMgr->GetSystemActivityTimer()&& ((PowerStateManager *)m_pPwrStateMgr)->GetSystemIdleTimeOut()!=0)
			m_dwEventArray [ PM_SYSTEM_ACTIVITY_EVENT ] = m_pPwrStateMgr->GetSystemActivityTimer()->hevReset;
		else
		    m_dwEventArray [ PM_SYSTEM_ACTIVITY_EVENT ] = m_hUnsignaledHandle;


        ((PowerStateManager *)m_pPwrStateMgr)->ReInitTimeOuts( );
    }
    // This state does not need Resume Time out.
    virtual PLATFORM_ACTIVITY_EVENT  WaitForEvent(DWORD dwTimeouts = INFINITE , DWORD dwNumOfExternEvent = 0, HANDLE * pExternEventArray = NULL) {
        TIMEOUT_ITEM TimeoutItem;
        ((PowerStateManager *)m_pPwrStateMgr)->DisableUserIdleTimeout();
        ((PowerStateManager *)m_pPwrStateMgr)->DisableSystemIdleTimeout();
        DWORD dwTimeout = ((PowerStateManager *)m_pPwrStateMgr)->GetSmallestTimeout(&TimeoutItem);
        PLATFORM_ACTIVITY_EVENT activeEvent = PowerState::WaitForEvent(dwTimeout ,dwNumOfExternEvent, pExternEventArray) ;
        switch (activeEvent) {
            case UserActivity: 
                m_LastNewState = On;
                break;

			case SystemActivity: 
                m_LastNewState = UserIdle;
                break;
            case Timeout: {
                switch (TimeoutItem) {
                    case SuspendTimeout:
                        m_LastNewState = Suspend;
                        break;
                    default:
                        ASSERT(FALSE);
                }
                break;
            }
        }
        return activeEvent;
    }
    virtual DWORD  GetState() { return (DWORD)SystemIdle; };
    virtual LPCTSTR GetStateString() { return STRING_SYSTEMIDLE; };
    virtual DWORD StateValidateRegistry(DWORD dwDState , DWORD dwFlag ) {
        return PowerState::StateValidateRegistry(2, 0 );
    }
    
};
class PowerStateSuspended : public PowerState {
public:
    PowerStateSuspended (PowerStateManager *pPwrStateMgr, PowerState * pNextPowerState = NULL )
    :   PowerState(pPwrStateMgr,pNextPowerState)
    { 
	};
    virtual void EnterState() {
		if(!((PowerStateManager *)m_pPwrStateMgr)->GetSuspendShutdownState())
		{
			DWORD timeout = ((PowerStateManager *)m_pPwrStateMgr)->GetSmallestWakeupTimeout();
			RETAILMSG(1, (_T("Enter suspend  state, timeout = %d \r\n"),timeout));
			if (timeout != INFINITE)
			{
				/*
				DWORD nAlarm = 1000;
				if(!KernelIoControl(IOCTL_KLIB_GETALARMRESOLUTION, 0, 0, (DWORD *)&nAlarm, sizeof(nAlarm), 0))
					nAlarm = 1000;

				timeout = (timeout/nAlarm)*nAlarm;
				RETAILMSG(1, (_T("Enter suspend  state1, timeout = %d \r\n"),timeout));
				*/

				
				DWORD dwSeconds = timeout/1000;
				//((PowerStateManager *)m_pPwrStateMgr)->SubtractWakeupTimeout(timeout);
			
				SYSTEMTIME curSysTime = {0};
				FILETIME fileTime;

				GetLocalTime(&curSysTime);
				SystemTimeToFileTime(&curSysTime, &fileTime);

				((PowerStateManager *)m_pPwrStateMgr)->SetSuspendStartTime(fileTime);
			

				((PowerStateManager *)m_pPwrStateMgr)->EnableWakeUp(dwSeconds);
			}
			PmSetSystemPowerState_I ( GetStateString() , 0 , 0, TRUE);
			// Because it wakeup by wakeup source So it automatic enter Resuming State.
			m_LastNewState = Resuming; // Point to Resuming.
		}
		else
			m_LastNewState = ShutDown; // Shutdown should be performed instead of suspend
      
    }

    virtual PLATFORM_ACTIVITY_EVENT  WaitForEvent(DWORD dwTimeouts = INFINITE , DWORD dwNumOfExternEvent = 0, HANDLE * pExternEventArray = NULL) {
        // Suspend is no wait
        return NoActivity;
    }
    virtual DWORD  GetState() { return (DWORD)Suspend; };
    virtual LPCTSTR GetStateString()  { return STRING_SUSPEND; };
    virtual DWORD StateValidateRegistry(DWORD  , DWORD  ) {
        return PowerState::StateValidateRegistry(3, POWER_STATE_SUSPEND );
    }
    virtual BOOL AppsCanRequestState() { return TRUE; }

};




class PowerStateResuming : public PowerState {
public:
    PowerStateResuming  (PowerStateManager *pPwrStateMgr, PowerState * pNextPowerState = NULL )
    :   PowerState(pPwrStateMgr,pNextPowerState)
    { 
		m_WakedForResumeTest = 0;
	};
    virtual void EnterState() {
		RETAILMSG(1, (_T("Enter resume state %d\r\n"), GetTickCount()));
        PowerState::EnterState();
		RETAILMSG(1, (_T("Enter resume state1 %d\r\n"), GetTickCount()));
		m_WakedForResumeTest = ((PowerStateManager *)m_pPwrStateMgr)->IsWakedForResumeTest()?1:0;

		m_TakePS = CreateEvent(0, 0, 0, L"Give Me Power Source");
		SetEvent(m_TakePS);
		CloseHandle(m_TakePS);

		((PowerStateManager *)m_pPwrStateMgr)->DisableWakeUp();

		if(!m_WakedForResumeTest)
		{
			if(WAIT_OBJECT_0 == WaitForAPIReady(SH_WMGR, 100))
				CeEventHasOccurred(NOTIFICATION_EVENT_TIME_CHANGE, 0);
             //SetGWESPowerUp();
			((PowerStateManager *)m_pPwrStateMgr)->ResetShutdownTimeout(TRUE);
			m_LastNewState = ((PowerStateManager *)m_pPwrStateMgr)->WakeUpSourceToPlatformState();
		}

        ((PowerStateManager *)m_pPwrStateMgr)->ReInitTimeOuts();

		//m_LastNewState = ((PowerStateManager *)m_pPwrStateMgr)->WakeUpSourceToPlatformState();
		//m_LastNewState = ShutDown;
        // Initial Timeout to Active First.

		/*  // already done in PowerState::EnterState()
        if (m_pPwrStateMgr->GetUserActivityTimer()!=NULL)
            m_dwEventArray [ PM_USER_ACTIVITY_EVENT ] = m_pPwrStateMgr->GetUserActivityTimer()->hevActive;
        if (m_pPwrStateMgr->GetSystemActivityTimer()!=NULL)
            m_dwEventArray [ PM_SYSTEM_ACTIVITY_EVENT ] = m_pPwrStateMgr->GetSystemActivityTimer()->hevActive;
			*/
        // m_pPwrStateMgr->ResetSystemIdleTimeTimeout(TRUE); // This will at least do not folling into suspend directly.
    }
    virtual PLATFORM_ACTIVITY_EVENT  WaitForEvent(DWORD dwTimeouts = INFINITE , DWORD dwNumOfExternEvent = 0, HANDLE * pExternEventArray = NULL) 
	{

		PLATFORM_ACTIVITY_EVENT activeEvent = NoActivity;
		RETAILMSG(0, (_T("Resume state:WaitForEvent \r\n")));
		if(m_WakedForResumeTest)
		{
			RETAILMSG(0, (_T("Resume state:IsWakedForResumeTest =  TRUE \r\n")));
			 m_dwEventArray [ PM_USER_ACTIVITY_EVENT ] = m_hUnsignaledHandle;
			 m_dwEventArray [ PM_SYSTEM_ACTIVITY_EVENT ] = m_hUnsignaledHandle;


			activeEvent = PowerState::WaitForEvent( DEF_RESUMETEST_WAIT_TIMEOUT,dwNumOfExternEvent, pExternEventArray) ;
			switch (activeEvent){
				case Timeout: 
					{
					//SetGWESPowerUp();
					((PowerStateManager *)m_pPwrStateMgr)->ResetShutdownTimeout(TRUE);
					m_LastNewState = ((PowerStateManager *)m_pPwrStateMgr)->WakeUpSourceToPlatformState();
					}
					break;
				case  ResumeTestOk:
					{

						//((PowerStateManager *)m_pPwrStateMgr)->SubtractWakeupTimeout(timeout);
			
						SYSTEMTIME curSysTime = {0};
						FILETIME curFileTime,startSuspendFileTime;
						unsigned __int64 curTime,startSuspendTime;

						DWORD    tickLasted;
						startSuspendFileTime = ((PowerStateManager *)m_pPwrStateMgr)->GetSuspendStartTime();

						GetLocalTime(&curSysTime);
						SystemTimeToFileTime(&curSysTime, &curFileTime);
						memcpy(&curTime, &curFileTime, sizeof(curTime));
						memcpy(&startSuspendTime, &startSuspendFileTime, sizeof(startSuspendTime));

						tickLasted = (DWORD)((( __int64 )(curTime - startSuspendTime))/( __int64 )10000);
						((PowerStateManager *)m_pPwrStateMgr)->SubtractWakeupTimeout(tickLasted);

	

						RETAILMSG(1, (_T("Resume state: ResumeTestOk, shutdown timeout = %d ,tickLasted = %d, prior = %d\r\n"),
							((PowerStateManager *)m_pPwrStateMgr)->GetCurShutdownTimeOut(),tickLasted, CeGetThreadPriority(GetCurrentThread())));
						if(((PowerStateManager *)m_pPwrStateMgr)->GetCurShutdownTimeOut()==0)
						{
							m_LastNewState = ShutDown;
						}
						else
						{
							m_LastNewState = Suspend;
						}
					}
					break;
				case  ResumeTestFailed:
					m_LastNewState = ShutDown;
					break;
				default:
					RETAILMSG(1, (_T("Default activeEvent = %x \r\n"),activeEvent));
			}				
			
        
		}
		

		/*
        PLATFORM_ACTIVITY_EVENT activeEvent = PowerState::WaitForEvent(0 ,dwNumOfExternEvent, pExternEventArray) ;
        switch (activeEvent) {
            case  UserActivity:
                m_LastNewState = On;
                break;
            case  SystemActivity:
                m_LastNewState = UserIdle;
                break;
            case Timeout: {
                m_LastNewState = On;                
                break;
            }
        }

        return activeEvent;
		*/
		return activeEvent;
    }
    virtual DWORD  GetState() { return (DWORD)Resuming; };
    virtual LPCTSTR GetStateString() { return STRING_RESUMING; };
    virtual DWORD StateValidateRegistry(DWORD , DWORD  ) {
        return PowerState::StateValidateRegistry(2, 0 );
    }
/*
	virtual DWORD  DefaultEventHandle(PLATFORM_ACTIVITY_EVENT platActivityEvent )
	{
		RETAILMSG(1, (_T("Resume state:DefaultEventHandle platActivityEvent = %d \r\n"),platActivityEvent));
		if(m_WakedForResumeTest)
		{
			switch (platActivityEvent) {
				case  ResumeTestOk:
					{
						RETAILMSG(1, (_T("Resume state: ResumeTestOk, shutdown timeout = %d \r\n"),((PowerStateManager *)m_pPwrStateMgr)->GetCurShutdownTimeOut()));
						if(((PowerStateManager *)m_pPwrStateMgr)->GetCurShutdownTimeOut()==0)
						{
							m_LastNewState = ShutDown;
						}
						else
						{
							m_LastNewState = Suspend;
						}
					}
					break;
				case  ResumeTestFailed:
					m_LastNewState = ShutDown;
					break;
				default:
					break;
			}
		 return GetLastNewState();
		}

		else
		{
			return PowerState::DefaultEventHandle(platActivityEvent);
		}
	}
	*/

	protected:
		BOOL m_WakedForResumeTest;
		HANDLE 	m_TakePS;
};

class PowerStateColdReboot : public PowerState {
public:
    PowerStateColdReboot (PowerStateManager *pPwrStateMgr, PowerState * pNextPowerState = NULL )
    :   PowerState(pPwrStateMgr,pNextPowerState)
    {; };
    virtual void EnterState() {
        PmSetSystemPowerState_I ( GetStateString() , 0 , 0, TRUE);
        // Because it wakeup by wakeup source So it automatic enter Resuming State.
    }
    virtual PLATFORM_ACTIVITY_EVENT  WaitForEvent(DWORD dwTimeouts = INFINITE , DWORD dwNumOfExternEvent = 0, HANDLE * pExternEventArray = NULL) {
        // Suspend is no wait
        return NoActivity;
    }
    virtual DWORD  GetState() { return (DWORD)ColdReboot; };
    virtual DWORD  GetLastNewState() {  return (DWORD)ColdReboot;; };
    virtual LPCTSTR GetStateString()  { return STRING_ColdReboot; };
    virtual DWORD StateValidateRegistry(DWORD  , DWORD  ) {
        return PowerState::StateValidateRegistry(4, POWER_STATE_RESET );
    }
    virtual BOOL AppsCanRequestState() { return TRUE; }
};

class PowerStateReboot : public PowerState {
public:
    PowerStateReboot (PowerStateManager *pPwrStateMgr, PowerState * pNextPowerState = NULL )
    :   PowerState(pPwrStateMgr,pNextPowerState)
    {; };
    virtual void EnterState() {
        PmSetSystemPowerState_I ( GetStateString() , 0 , 0, TRUE);
        // Because it wakeup by wakeup source So it automatic enter Resuming State.
    }
    virtual PLATFORM_ACTIVITY_EVENT  WaitForEvent(DWORD dwTimeouts = INFINITE , DWORD dwNumOfExternEvent = 0, HANDLE * pExternEventArray = NULL) {
        // Suspend is no wait
        return NoActivity;
    }
    virtual DWORD  GetState() { return (DWORD)Reboot; };
    virtual DWORD  GetLastNewState() {  return (DWORD)Reboot;; };
    virtual LPCTSTR GetStateString()  { return STRING_Reboot; };
    virtual DWORD StateValidateRegistry(DWORD  , DWORD  ) {
        return PowerState::StateValidateRegistry(4, POWER_STATE_RESET );
    }
    virtual BOOL AppsCanRequestState() { return TRUE; }
};



class PowerStateShutDown : public PowerState {
public:
    PowerStateShutDown (PowerStateManager *pPwrStateMgr, PowerState * pNextPowerState = NULL )
    :   PowerState(pPwrStateMgr,pNextPowerState)
    {; };
    virtual void EnterState() {
        PmSetSystemPowerState_I ( GetStateString() , 0 , 0, TRUE);

    }
    virtual PLATFORM_ACTIVITY_EVENT  WaitForEvent(DWORD dwTimeouts = INFINITE , DWORD dwNumOfExternEvent = 0, HANDLE * pExternEventArray = NULL) {
        // Shutdown is no wait
        return NoActivity;
    }
    virtual DWORD  GetState() { return (DWORD)ShutDown; };
    virtual DWORD  GetLastNewState() {  return (DWORD)ShutDown;; };
    virtual LPCTSTR GetStateString()  { return STRING_ShutDown; };
    virtual DWORD StateValidateRegistry(DWORD  , DWORD  ) {
        return PowerState::StateValidateRegistry(4, POWER_STATE_OFF );
    }
    virtual BOOL AppsCanRequestState() { return TRUE; }
};


BOOL PowerStateManager::CreatePowerStateList()
{
    if (m_pPowerStateList == NULL ) {
        m_pPowerStateList = new PowerStateOn( this, new PowerStateUserIdle( this, new PowerStateSystemIdle(this,  
            new PowerStateResuming(this, new PowerStateSuspended(this, new PowerStateReboot(this,new PowerStateColdReboot(this,new PowerStateShutDown(this))))))));
    }
    if (m_pPowerStateList != NULL) {
        PowerState * pCurState = m_pPowerStateList;
        while (pCurState) {
            if (!pCurState->Init()) {
                ASSERT(FALSE);
                return FALSE;
            }
            pCurState = pCurState->GetNextPowerState();
        }
        return TRUE;
    }
    else
        return FALSE;
}

DefaultPowerStateManager * CreateDefaultPowerManager(PVOID pParm)
{
    return (new PowerStateManager(pParm));
}

