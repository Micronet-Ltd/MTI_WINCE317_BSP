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
// This module contains power state exchange.
// managing device power.
//

// This typedef describes the system activity states.  These are independent of
// factors such as AC power vs. battery, in cradle or not, etc.  OEMs may choose
// to add their own activity states if they customize this module.

// This typedef describes activity events such as user activity or inactivity,
// power status changes, etc.  OEMs may choose to factor other events into their
// system power state transition decisions.

#include <Csync.h>
#include <cRegEdit.h>

#ifndef __PWSDEF_H_
#define __PWSDEF_H_

#define PM_SHUTDOWN_EVENT 0
#define PM_RELOAD_ACTIVITY_TIMEOUTS_EVENT 1
#define PM_MSGQUEUE_EVENT 2
#define PM_RESTART_TIMER_EVENT 3
#define PM_USER_ACTIVITY_EVENT 4
#define PM_SYSTEM_ACTIVITY_EVENT 5
#define PM_SYSTEM_API_EVENT 6
#define PM_BOOTPHASE2_EVENT 7

#define PM_BASE_TOTAL_EVENT 8

#define PM_UNKNOWN_POWER_STATE (-1)


// OEMs can define values higher than this (as PPN_OEMBASE+0, +1, etc.)
#define PPN_RESUMETESTEND          PPN_OEMBASE+0
#define PPN_SUSPENDSHUTDOWN        PPN_OEMBASE+1

typedef enum {
    NoActivity,
    UserActivity,
    UserInactivity,
    SystemActivity,
    SystemInactivity,
    EnterUnattendedModeRequest,
    LeaveUnattendedModeRequest,
    Timeout,
    RestartTimeouts,
    PowerSourceChange,
    Resume,
    SystemPowerStateChange,
    SystemPowerStateAPI,
    PmShutDown,
    PmReloadActivityTimeouts,
    PowerButtonPressed,
    AppButtonPressed, 
    ExternedEvent = 0x1000,
	ResumeTestOk,
	ResumeTestFailed,
	SuspendShutdownRequest,
	SuspendShutdownRelease,
    SuspButtonPressed
} PLATFORM_ACTIVITY_EVENT, *PPLATFORM_ACTIVITY_EVENT;

typedef enum { 
    On,                     // system is running normally with UI enabled
    UserIdle,               // User Idle state.
    SystemIdle,
    Resuming,               // system is determining what to do after a resume
    Suspend,                 // system suspended, all devices off (or wake-enabled)
    ColdReboot,
    Reboot,
	ShutDown,
    UnknownState = PM_UNKNOWN_POWER_STATE,       // Unknown
} PLATFORM_ACTIVITY_STATE, *PPLATFORM_ACTIVITY_STATE;

#define MAX_EVENT_ARRAY MAXIMUM_WAIT_OBJECTS 


class DefaultPowerStateManager; 
class PowerState {
public:
    PowerState(DefaultPowerStateManager *pPwrStateMgr, PowerState * pNextPowerState );
    virtual ~PowerState();
    virtual void EnterState();
    virtual BOOL Init() ;        
    virtual PLATFORM_ACTIVITY_EVENT  WaitForEvent(DWORD dwTimeouts = INFINITE , DWORD dwNumOfExternEvent = 0, HANDLE * pExternEventArray = NULL); 
  //  virtual DWORD  DefaultEventHandle(PLATFORM_ACTIVITY_EVENT dwHandleIndex ) { return GetLastNewState(); };
	virtual DWORD  DefaultEventHandle(PLATFORM_ACTIVITY_EVENT dwHandleIndex );
    virtual PLATFORM_ACTIVITY_EVENT   UnwantedEventHandle(DWORD dwIndex) { 
        ASSERT(FALSE);
        return NoActivity; 
    };
    virtual DWORD   GetState() = NULL;
    virtual LPCTSTR GetStateString() = NULL ;
    virtual DWORD   GetStateInitialFlags() { return m_InitFlags; };
    virtual DWORD StateValidateRegistry(DWORD dwDState = 0, DWORD dwFlag = POWER_STATE_ON);
    virtual DWORD  GetLastNewState() { return m_LastNewState; };
    virtual DWORD  SetSystemAPIState(DWORD apiState) {
        if (PM_UNKNOWN_POWER_STATE != apiState) 
            m_LastNewState = apiState;
        return m_LastNewState;
    }
    PowerState * GetNextPowerState () { return m_pNextPowerState; };
    virtual BOOL AppsCanRequestState() { return FALSE; } ;
protected:
    DWORD   m_LastNewState;
    DWORD   m_InitFlags;
    DefaultPowerStateManager * const m_pPwrStateMgr ;
    HANDLE m_hUnsignaledHandle;
    virtual PLATFORM_ACTIVITY_EVENT MsgQueueEvent();
    
    PowerState * const m_pNextPowerState ;
    DWORD  m_dwNumOfEvent;
    HANDLE m_dwEventArray[MAX_EVENT_ARRAY];
};

class DefaultPowerStateManager: public CLockObject {
public:
    DefaultPowerStateManager(PVOID pParam);
    virtual ~DefaultPowerStateManager();
// public function
    virtual BOOL    Init();
    virtual HANDLE  GetEventHandle(DWORD dwIndex);
    virtual void    PlatformResumeSystem(BOOL fSuspened);
    virtual DWORD   PlatformMapPowerStateHint(DWORD dwHint, LPTSTR pszBuf, DWORD dwBufChars);
protected:    
    virtual PowerState * GetFirstPowerState();
    virtual PowerState * SetSystemState(PowerState * pCurPowerState );
public:
    virtual DWORD   SendSystemPowerState(LPCWSTR pwsState, DWORD dwStateHint, DWORD dwOptions);
    virtual HANDLE  GetAPISignalHandle() { return m_hSystemApiCalled; };   
    virtual DWORD   SystemStateToActivityState(LPCTSTR lpState )  ;
    virtual LPCTSTR ActivityStateToSystemState (DWORD platActiveState) ;
    virtual DWORD   ThreadRun() =  0;
    PowerState *    GetPowerStatesList () { return m_pPowerStateList ; };
    PACTIVITY_TIMER GetUserActivityTimer() { return m_pUserActivity; }; 
    PACTIVITY_TIMER GetSystemActivityTimer() { return m_pSystemActivity; }; 
    PowerState *    GetStateObject(DWORD newState);

// Timer Function required.
    virtual void ResetUserIdleTimeout(BOOL fIdle) = 0 ;
    virtual void ResetSystemIdleTimeTimeout(BOOL fIdle) =0 ;
	virtual void ResetSuspendTimeTimeout(BOOL fIdle) =0 ;
    virtual void SubtractTimeout(DWORD dwTicks) = 0 ;

	virtual void PlatformLoadTimeouts() = 0; ////reread timeouts from registry
	virtual void SetSuspendShutdownState(BOOL) = 0; // state when shutdown follow immediately after suspend state 

protected:
    // Created Event
    HANDLE m_hevReloadActivityTimeouts;
    HANDLE m_hevBootPhase2;
    HANDLE m_hevRestartTimers;
    HANDLE m_hSystemApiCalled;
    // Global Event Handle
    HANDLE m_hevPmShutdown;
    HANDLE m_hqNotify;
    
    PACTIVITY_TIMER m_pUserActivity;
    PACTIVITY_TIMER m_pSystemActivity;
    
    PowerState *m_pPowerStateList;
    PowerState *m_pCurPowerState;
};

    
extern DefaultPowerStateManager * CreateDefaultPowerManager(PVOID );

#endif
