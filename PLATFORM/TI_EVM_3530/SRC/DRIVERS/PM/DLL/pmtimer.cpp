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
// This module contains code to handle activity timers.  Activity timers are
// unsignaled until they count down to 0 (expire), at which point they become
// signaled.  The PM implements activity timers by providing named auto-reset
// events that reset the timers.  Each timer has a pair of named manual-reset 
// events that indicates activity or inactivity.  Both of these timers are reset
// during suspend.
//

#include <pmimpl.h>
#include <nkintr.h>


// This routine initializes the list of activity timers.  It returns ERROR_SUCCESS 
// if successful or a Win32 error code otherwise.
DWORD
ActivityTimerInitList(VOID)
{
    DWORD dwStatus;
    HKEY hk = NULL;
    TCHAR szSources[1024];
    DWORD dwNumTimers = 0;
    PPACTIVITY_TIMER ppatList = NULL;
    SETFNAME(_T("ActivityTimerInitList"));

    wsprintf(szSources, _T("%s\\ActivityTimers"), PWRMGR_REG_KEY);
    dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szSources, 0, 0, &hk);
    if(dwStatus == ERROR_SUCCESS) {
        // figure out how many values are associated with the key
        dwStatus = RegQueryInfoKey(hk, NULL, NULL, NULL, &dwNumTimers, NULL, NULL, NULL,
            NULL, NULL, NULL, NULL);
    } else {
        // no timers configured in the registry
        dwNumTimers = 0;
        dwStatus = ERROR_SUCCESS;
    }

    // if there are any values, allocate an array to hold them
    if(dwStatus == ERROR_SUCCESS) {
        ppatList = (PPACTIVITY_TIMER) PmAlloc((dwNumTimers + 1) * sizeof(PACTIVITY_TIMER));
        if(ppatList == NULL) {
            PMLOGMSG(ZONE_WARN, (_T("%s: couldn't allocate %d timers\r\n"), pszFname,
                dwNumTimers));
            dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        } else {
            memset(ppatList, 0, (dwNumTimers + 1) * sizeof(PACTIVITY_TIMER));
            ppatList[dwNumTimers] = NULL;
        }
    }

    // read list of timers
    if(dwStatus == ERROR_SUCCESS && dwNumTimers != 0) {
        DWORD dwIndex = 0;
        do {
            TCHAR szName[256];
            DWORD cbValueName = dim(szName);
            
            dwStatus = RegEnumKeyEx(hk, dwIndex, szName, &cbValueName, NULL,
                NULL, NULL, NULL);
            if(dwStatus == ERROR_SUCCESS) {
                HKEY hkSubKey = NULL;
                
                // open the subkey
                dwStatus = RegOpenKeyEx(hk, szName, 0, 0, &hkSubKey);
                if(dwStatus == ERROR_SUCCESS) {
                    DWORD dwSize, dwType, dwTimeout;
                    LPTSTR pszValueName;

                    // read the timeout, expressed in seconds
                    dwSize = sizeof(dwTimeout);
                    pszValueName = _T("Timeout");
                    dwStatus = RegQueryValueEx(hkSubKey, pszValueName, NULL, &dwType, (LPBYTE) &dwTimeout, 
                        &dwSize);
                    if(dwStatus == ERROR_SUCCESS) {
                        if(dwType != REG_DWORD || dwTimeout > MAXTIMERINTERVAL) {
                            PMLOGMSG(ZONE_WARN, 
                                (_T("%s: RegQueryValueEx('%s'\'%s') or returned invalid type %d or invalid value %d\r\n"),
                                pszFname, szName, pszValueName, dwType, dwTimeout));
                            dwStatus = ERROR_INVALID_DATA;
                        }

                        // convert timeout to milliseconds
                        dwTimeout *= 1000;
                    } else {
                        // no timeout in seconds, try milliseconds
                        dwSize = sizeof(dwTimeout);
                        pszValueName = _T("TimeoutMs");
                        dwStatus = RegQueryValueEx(hkSubKey, pszValueName, NULL, &dwType, (LPBYTE) &dwTimeout, 
                            &dwSize);
                        if(dwStatus != ERROR_SUCCESS || dwType != REG_DWORD || dwTimeout > (MAXTIMERINTERVAL * 1000)) {
                            PMLOGMSG(ZONE_WARN, 
                                (_T("%s: RegQueryValueEx('%s'\'%s') failed %d (or returned invalid type %d or invalid value %d)\r\n"),
                                pszFname, szName, pszValueName, dwStatus, dwType, dwTimeout));
                            dwStatus = ERROR_INVALID_DATA;
                        }
                    }

                    if(dwStatus == ERROR_SUCCESS) {
                        // get wake sources
                        dwSize = sizeof(szSources);
                        pszValueName = _T("WakeSources");
                        dwStatus = RegQueryValueEx(hkSubKey, pszValueName, NULL, &dwType, (LPBYTE) szSources, 
                            &dwSize);
                        if(dwStatus != ERROR_SUCCESS) {
                            // no wake sources
                            szSources[0] = 0;
                            szSources[1] = 0;
                            dwStatus = ERROR_SUCCESS;
                        } else if(dwType != REG_MULTI_SZ) {
                            PMLOGMSG(ZONE_WARN, (_T("%s: invalid type %d for '%s'\'%s'\r\n"), pszFname, dwType,
                                szName, pszValueName));
                            dwStatus = ERROR_INVALID_DATATYPE;
                        } else {
                            szSources[dim(szSources) -1] = szSources[dim(szSources) -2] = 0; // Terminate MultiSZ
                        }
                    }
                    
                    // did we get the parameters?
                    if(dwStatus == ERROR_SUCCESS) {
                        ppatList[dwIndex] = ActivityTimerCreate(szName, dwTimeout, szSources);
                        if(ppatList[dwIndex] == NULL) {
                            dwStatus = ERROR_NOT_ENOUGH_MEMORY;
                        }
                    }
                }
                
                // release the registry key
                RegCloseKey(hkSubKey);
            }
            
            // update the index
            dwIndex++;
        } while(dwStatus == ERROR_SUCCESS && dwIndex < dwNumTimers);

        // did we read all items ok?
        if(dwStatus == ERROR_NO_MORE_ITEMS) {
            dwStatus = ERROR_SUCCESS;
        }

        // terminate the list with a NULL
        ppatList[dwIndex] = NULL;
    }

    // did we succeed?
    if(dwStatus == ERROR_SUCCESS) {
        PMLOCK();
        gppActivityTimers = ppatList;
        PMUNLOCK();
    } else {
        DWORD dwIndex;
        if(ppatList != NULL) {
            for(dwIndex = 0; dwIndex < dwNumTimers; dwIndex++) {
                if(ppatList[dwIndex] != NULL) {
                    ActivityTimerDestroy(ppatList[dwIndex]);
                }
            }
            PmFree(ppatList);
        }
    }

    // release resources
    if(hk != NULL) RegCloseKey(hk);

    PMLOGMSG(dwStatus != ERROR_SUCCESS && ZONE_WARN,
        (_T("%s: returning %d\r\n"), pszFname, dwStatus));
    return dwStatus;
}

#if 0
// This routine calculates the next timeout interval for the various 
// activity timers.  This routine doesn't sort timers, on the assumption
// that a given system will have relatively few inactivity timers.
DWORD
GetNextInactivityTimeout(DWORD dwElapsed)
{
    DWORD dwTimeout = INFINITE;
    DWORD dwIndex;
    PACTIVITY_TIMER pat;

    PMLOCK();
    for(dwIndex = 0; (pat = gppActivityTimers[dwIndex]) != NULL; dwIndex++) {
        DWORD dwTimeLeft = pat->dwTimeLeft;
        if(dwTimeLeft != INFINITE) {
            // subtract elapsed time
            if(dwTimeLeft < dwElapsed) {
                dwTimeLeft = 0;
            } else {
                dwTimeLeft -= dwElapsed;
            }

            // update the timeout value
            if(dwTimeout == INFINITE || dwTimeLeft < dwTimeout) {
                dwTimeout = dwTimeLeft;
            }

            // update the timer
            pat->dwTimeLeft = dwTimeLeft;
        }
    }
    PMUNLOCK();

    return dwTimeout;
}
// this thread handles activity timer events
DWORD WINAPI 
ActivityTimersThreadProc(LPVOID lpvParam)
{
    DWORD dwStatus, dwNumEvents, dwWaitInterval;
    HANDLE hevReady = (HANDLE) lpvParam;
    HANDLE hEvents[MAXIMUM_WAIT_OBJECTS];
    BOOL fDone = FALSE;
    HANDLE hevDummy = NULL;
    INT iPriority;
    const DWORD cdwTimerBaseIndex = 2;
    SETFNAME(_T("ActivityTimersThreadProc"));

    PMLOGMSG(ZONE_INIT, (_T("+%s: thread 0x%08x\r\n"), pszFname, GetCurrentThreadId()));

    // set the thread priority
    if(!GetPMThreadPriority(_T("TimerPriority256"), &iPriority)) {
        iPriority = DEF_ACTIVITY_TIMER_THREAD_PRIORITY;
    }
    CeSetThreadPriority(GetCurrentThread(), iPriority);

    // initialize the list of activity timers
    if(ActivityTimerInitList() != ERROR_SUCCESS) {
        PMLOGMSG(ZONE_WARN, (_T("%s: ActivityTimerInitList() failed\r\n"), pszFname));
        goto done;
    }

    // create a dummy event that's never signaled
    hevDummy = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(hevDummy == NULL) {
        PMLOGMSG(ZONE_WARN, (_T("%s: Couldn't create dummy event\r\n"), pszFname));
        goto done;
    }

    // set up the list of events
    dwNumEvents = 0;
    hEvents[dwNumEvents++] = ghevPmShutdown;
    hEvents[dwNumEvents++] = ghevTimerResume;
    PMLOCK();
	if(gppActivityTimers[0] == NULL) {
		// no activity timers defined
		PmFree(gppActivityTimers);
		gppActivityTimers = NULL;
	} 
	/*
	else {
		// copy activity timer events into the event list
		while(dwNumEvents < dim(hEvents) && gppActivityTimers[dwNumEvents - cdwTimerBaseIndex] != NULL) {
			hEvents[dwNumEvents] = gppActivityTimers[dwNumEvents - cdwTimerBaseIndex]->hevReset;
			dwNumEvents++;
		}
	}*/
    PMUNLOCK();

    // we're up and running
    SetEvent(hevReady);

	/*
    // are there actually any timers to wait on?
    if(dwNumEvents <= cdwTimerBaseIndex) {
        // no timers defined, so we don't need this thread to wait on them.
        PMLOGMSG(ZONE_INIT || ZONE_WARN, (_T("%s: no activity timers defined, exiting\r\n"), pszFname));
        Sleep(1000);            // don't want PM initialization to fail when we exit
        goto done;
    }
   */
	
    // wait for these events to get signaled
    PMLOGMSG(ZONE_TIMERS, (_T("%s: entering wait loop, %d timers total\r\n"),
        pszFname, dwNumEvents - cdwTimerBaseIndex));
    dwWaitInterval = 0;
    while(!fDone) {
        DWORD dwTimeout = GetNextInactivityTimeout(dwWaitInterval);
        DWORD dwWaitStart = GetTickCount();

        PMLOGMSG(ZONE_TIMERS, 
            (_T("%s: waiting %u (0x%08x) ms for next event, wait interval was %d\r\n"), pszFname,
            dwTimeout, dwTimeout, dwWaitInterval));
       // dwStatus = WaitForMultipleObjects(dwNumEvents, hEvents, FALSE, dwTimeout);
		dwStatus = WaitForMultipleObjects(dwNumEvents, hEvents, FALSE, INFINITE);
        dwWaitInterval = GetTickCount() - dwWaitStart;

        // figure out what caused the wakeup
        if(dwStatus == (WAIT_OBJECT_0 + 0)) {
            PMLOGMSG(ZONE_WARN, (_T("%s: shutdown event set\r\n"), pszFname));
            fDone = TRUE;
        } else if(dwStatus == (WAIT_OBJECT_0 + 1)) {
            DWORD dwIndex;
            PACTIVITY_TIMER pat;

            // we've resumed, so re-enable all activity timers that can be reset
            PMLOGMSG(ZONE_TIMERS, (_T("%s: resume event set\r\n"), pszFname));
            PMLOCK();
            for(dwIndex = 0; (pat = gppActivityTimers[dwIndex]) != NULL; dwIndex++) {
                DWORD dwEventIndex = dwIndex + cdwTimerBaseIndex;
                if(hEvents[dwEventIndex] == hevDummy) {
                    hEvents[dwEventIndex] = pat->hevReset;
                }
                pat->dwTimeLeft = pat->dwTimeout + dwWaitInterval;
            }
            PMUNLOCK();
        } 
		/*else if(dwStatus == WAIT_TIMEOUT) {
            DWORD dwIndex;
            PACTIVITY_TIMER pat;

            // figure out which event(s) timed out
            PMLOCK();
            for(dwIndex = 0; (pat = gppActivityTimers[dwIndex]) != NULL; dwIndex++) {
                if(pat->dwTimeLeft <= dwWaitInterval  && pat->dwTimeLeft != INFINITE) {
                    // has the timer really expired?
                    if(WaitForSingleObject(pat->hevReset, 0) == WAIT_OBJECT_0) {
                        // The timer was reset while we weren't looking at it, so we'll look
                        // at it again later.  Calculate the new timeout, compensating for the update
                        // that will occur in GetNextInactivityTimeout().
                        PMLOGMSG(ZONE_TIMERS, (_T("%s: timer '%s' reset after timeout\r\n"), pszFname,
                            pat->pszName));
                        pat->dwTimeLeft = pat->dwTimeout + dwWaitInterval;
                        pat->dwResetCount++;
                    } else {
                        // the timer has really expired, update events appropriately
                        PMLOGMSG(ZONE_TIMERS, (_T("%s: timer '%s' has expired\r\n"), pszFname,
                            pat->pszName));
                        ResetEvent(pat->hevActive);
                        SetEvent(pat->hevInactive);

                        // start looking at the reset event for this timer again
                        hEvents[dwIndex + cdwTimerBaseIndex] = pat->hevReset;

                        // update counts
                        pat->dwTimeLeft = INFINITE;
                        pat->dwExpiredCount++;
                    }
                }
            }
            PMUNLOCK();
        } else if(dwStatus > (WAIT_OBJECT_0 + 0) && dwStatus < (WAIT_OBJECT_0 + dwNumEvents)) {
            PACTIVITY_TIMER pat;
            DWORD dwEventIndex = dwStatus - WAIT_OBJECT_0;

            PMLOCK();
            
            // get a pointer to the timer
            pat = gppActivityTimers[dwEventIndex - cdwTimerBaseIndex];

            // handle its events
            DEBUGCHK(pat != NULL);
            if(pat->dwTimeout == 0) {
                // we're not using the event, so ignore it
                pat->dwTimeLeft = INFINITE;
            } else {
                PMLOGMSG(ZONE_TIMERS, (_T("%s: timer '%s' reset\r\n"), pszFname, pat->pszName));

                // set events appropriately
                ResetEvent(pat->hevInactive);
                SetEvent(pat->hevActive);

                // don't look at this event again until it's about ready to time out
                hEvents[dwEventIndex] = hevDummy;

                // update time left on the timer, compensating for the update
                // that will occur in GetNextInactivityTimeout().
                pat->dwTimeLeft = pat->dwTimeout + dwWaitInterval;
            }
            pat->dwResetCount++;
            PMUNLOCK();
        }
		*/else {
            PMLOGMSG(ZONE_WARN, (_T("%s: WaitForMultipleObjects() returned %d, status is %d\r\n"),
                pszFname, dwStatus, GetLastError())); 
            fDone = TRUE;
        }
    }
	

done:
    // release resources
    if(hevDummy != NULL) CloseHandle(hevDummy);
	PMLOCK();
    if(gppActivityTimers != NULL) {
        DWORD dwIndex = 0;
        while(gppActivityTimers[dwIndex] != NULL) {
            ActivityTimerDestroy(gppActivityTimers[dwIndex]);
            dwIndex++;
        }
        PmFree(gppActivityTimers);
		gppActivityTimers = NULL;
    }
	PMUNLOCK();

    PMLOGMSG(ZONE_INIT | ZONE_WARN, (_T("-%s: exiting\r\n"), pszFname));
    return 0;
}


#endif