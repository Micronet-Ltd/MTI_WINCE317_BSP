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
// This module contains code to handle notifications of devices being added
// and removed from the system.
//

#include <pmimpl.h>
#include <pnp.h>
#include <msgqueue.h>

// this routine reads device notifications from a message queue and updates
// the PM's internal tables appropriately.
BOOL
ProcessPnPMsgQueue(HANDLE hMsgQ)
{
    BOOL fOk = FALSE;
    UCHAR deviceBuf[PNP_QUEUE_SIZE];
    DWORD iBytesInQueue = 0;
    DWORD dwFlags = 0;
    SETFNAME(_T("ProcessPnPMsgQueue"));

    // read a message from the message queue -- it should be a device advertisement
    memset(deviceBuf, 0, PNP_QUEUE_SIZE);
    if ( !ReadMsgQueue(hMsgQ, deviceBuf, PNP_QUEUE_SIZE, &iBytesInQueue, 0, &dwFlags)) {
        // nothing in the queue
        PMLOGMSG(ZONE_WARN, (_T("%s: ReadMsgQueue() failed %d\r\n"), pszFname,
            GetLastError()));
    } else if(iBytesInQueue >= sizeof(DEVDETAIL)) {
        // process the message
        PDEVDETAIL pDevDetail = (PDEVDETAIL) deviceBuf;
        DWORD dwMessageSize = sizeof(DEVDETAIL) + pDevDetail->cbName;
        
        // check for overlarge names
        if(pDevDetail->cbName > ((PNP_MAX_NAMELEN - 1) * sizeof(pDevDetail->szName[0]))) {
            PMLOGMSG(ZONE_WARN, (_T("%s: device name longer than %d characters\r\n"), 
                pszFname, PNP_MAX_NAMELEN - 1));
        } else {
            // convert the device name to lower case
            int i;
            for(i = 0; i < (PNP_MAX_NAMELEN - 1) && pDevDetail->szName[i] != 0; i++) {
                pDevDetail->szName[i] = _totlower(pDevDetail->szName[i]);
            }
            pDevDetail->szName[i] = 0;
            
            // add or remove the device -- note that a particular interface may be
            // advertised more than once, so these routines must handle that possibility.
            if(pDevDetail->fAttached) {
                AddDevice(&pDevDetail->guidDevClass, pDevDetail->szName, NULL, NULL);
            } else {
                RemoveDevice(&pDevDetail->guidDevClass, pDevDetail->szName);
            }
            fOk = TRUE;
        }
    } else {
        // not enough bytes for a message
        PMLOGMSG(ZONE_WARN, (_T("%s: got runt message (%d bytes)\r\n"), pszFname, 
            iBytesInQueue));
    }

    return fOk;
}

// this thread waits for power manageable devices to be announced.  When
// they arrive they are added to the PM's list of devices and initialized
// appropriately.  If a device goes away, its entry will be removed
// from the list.
extern "C" extern HANDLE ghevPowerManagerReady; 
DWORD WINAPI 
PnpThreadProc(LPVOID lpvParam)
{
    DWORD dwStatus;
    HANDLE hnGeneric = NULL;
    HANDLE hevReady = (HANDLE) lpvParam;
    HANDLE hEvents[MAXIMUM_WAIT_OBJECTS];
    DWORD dwNumEvents = 0;
    BOOL fDone = FALSE;
    BOOL fOk;
    INT iPriority;
    PDEVICE_LIST pdl;
    SETFNAME(_T("PnpThreadProc"));

    PMLOGMSG(ZONE_INIT, (_T("+%s: thread 0x%08x\r\n"), pszFname, GetCurrentThreadId()));

    // set the thread priority
    if(!GetPMThreadPriority(_T("PnPPriority256"), &iPriority)) {
        iPriority = DEF_PNP_THREAD_PRIORITY;
    }
    CeSetThreadPriority(GetCurrentThread(), iPriority);

    // first list entry is the exit event
    hEvents[dwNumEvents++] = ghevPmShutdown;

    // set up device notifications
    for(pdl = gpDeviceLists; pdl != NULL && dwNumEvents < dim(hEvents); pdl = pdl->pNext) {
        hEvents[dwNumEvents++] = pdl->hMsgQ;
        pdl->hnClass = RequestDeviceNotifications(pdl->pGuid, pdl->hMsgQ, TRUE);
        if(pdl->hnClass == NULL) {
            PMLOGMSG(ZONE_WARN, (_T("%s: RequestDeviceNotifications() failed %d\r\n"),
                pszFname, GetLastError()));
        }
    }
    DEBUGCHK(dwNumEvents > 1);
    // we're up and running
    SetEvent(hevReady);
    
    // Wait for Initalization complete.
    HANDLE hInit[2] = {ghevPowerManagerReady, ghevPmShutdown};
    fDone = (WaitForMultipleObjects(_countof(hInit), hInit, FALSE, INFINITE)!= WAIT_OBJECT_0);

    // wait for new devices to arrive
    while(!fDone) {
        dwStatus = WaitForMultipleObjects(dwNumEvents, hEvents, FALSE, INFINITE);
        if(dwStatus == (WAIT_OBJECT_0 + 0)) {
            PMLOGMSG(ZONE_WARN, (_T("%s: shutdown event set\r\n"), pszFname));
            fDone = TRUE;
        } else if(dwStatus > WAIT_OBJECT_0 && dwStatus <= (WAIT_OBJECT_0 + MAXIMUM_WAIT_OBJECTS)) { 
            dwStatus -= WAIT_OBJECT_0;
            fOk = ProcessPnPMsgQueue(hEvents[dwStatus]);
            if(!fOk) {
                PMLOGMSG(ZONE_WARN, (_T("%s: ProcessPnPMsgQueue(0x%08x) failed\r\n"), pszFname,
                    hEvents[dwStatus]));
            }
        } else {
            PMLOGMSG(ZONE_WARN, (_T("%s: WaitForMultipleObjects() returned %d, status is %d\r\n"),
                pszFname, dwStatus, GetLastError())); 
            fDone = TRUE;
            break;
        }
    }

    // release resources
    for(pdl = gpDeviceLists; pdl != NULL; pdl = pdl->pNext) {
        if(pdl->hnClass != NULL) StopDeviceNotifications(pdl->hnClass);
    }

    // all done
    PMLOGMSG(ZONE_INIT | ZONE_WARN, (_T("-%s: exiting\r\n"), pszFname));
    return 0;
}

