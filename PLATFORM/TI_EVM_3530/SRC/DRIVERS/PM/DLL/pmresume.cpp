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
// This module contains code to handle system suspend/resume events.  Note that
// system suspend/resume is not the same as a system power state transition.  A
// system suspend/resume event occurs when PowerOffSystem() is called, device
// drivers are notified via PowerUp()/PowerDown() entry points, and OEMPowerOff()
// is invoked.  System power state transitions may or may not involve a system
// suspend/resume.  
//
// Devices that support the Power Manager should not allow system suspend/resume 
// events to occur except as part of a system power state transition.  That is,
// all suspends should involve SetSystemPowerState().  PowerOffSystem() should never
// be called directly.
//

#include <pmimpl.h>
#include <nkintr.h>

// this thread is signaled when the system wakes from a suspend state
DWORD WINAPI 
ResumeThreadProc(LPVOID lpvParam)
{
    DWORD dwStatus;
    HANDLE hevReady = (HANDLE) lpvParam;
    HANDLE hEvents[2];
    BOOL fDone = FALSE;
    INT iPriority;
    SETFNAME(_T("ResumeThreadProc"));

    PMLOGMSG(ZONE_INIT, (_T("+%s: thread 0x%08x\r\n"), pszFname, GetCurrentThreadId()));

    // set the thread priority
    if(!GetPMThreadPriority(_T("ResumePriority256"), &iPriority)) {
        iPriority = DEF_RESUME_THREAD_PRIORITY;
    }
    CeSetThreadPriority(GetCurrentThread(), iPriority);

    // we're up and running
    SetEvent(hevReady);

    // wait for new devices to arrive
    hEvents[0] = ghevResume;
    hEvents[1] = ghevPmShutdown;
    while(!fDone) {
        dwStatus = WaitForMultipleObjects(dim(hEvents), hEvents, FALSE, INFINITE);
        switch(dwStatus) {
        case (WAIT_OBJECT_0 + 0):
            PMLOGMSG(ZONE_RESUME, (_T("%s: resume event signaled\r\n"), pszFname));
            PlatformResumeSystem();
            break;
        case (WAIT_OBJECT_0 + 1):
            PMLOGMSG(ZONE_WARN, (_T("%s: shutdown event set\r\n"), pszFname));
            fDone = TRUE;
            break;
        default:
            PMLOGMSG(ZONE_WARN, (_T("%s: WaitForMultipleObjects() returned %d, status is %d\r\n"),
                pszFname, dwStatus, GetLastError())); 
            fDone = TRUE;
            break;
        }
    }

    PMLOGMSG(ZONE_INIT | ZONE_WARN, (_T("-%s: exiting\r\n"), pszFname));
    return 0;
}

// this routine is called in an interrupt context.  It indicates whether
// the system is suspending or resumeing.
EXTERN_C VOID WINAPI
PmPowerHandler(BOOL bOff)
{
    if(!bOff) {
        // we are resuming, signal the resume thread
        if(ghevResume != NULL) CeSetPowerOnEvent(ghevResume);

        // signal the timer thread too
//        if(ghevTimerResume != NULL) CeSetPowerOnEvent(ghevTimerResume);
    }
}
