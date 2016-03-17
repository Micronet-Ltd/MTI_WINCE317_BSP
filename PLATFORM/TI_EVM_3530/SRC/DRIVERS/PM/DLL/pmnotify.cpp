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
// This routine keeps track of notification requests from applications.
//

#include <pmimpl.h>
#include <msgqueue.h>

// This routine sends a notification to a specific listener.  The caller
// of this routine must hold the PM lock. The dwLen parameter is the total
// size, in bytes, of the message being sent.
VOID
SendNotification(PPOWER_NOTIFICATION ppn, PPOWER_BROADCAST ppb, DWORD dwLen)
{
    SETFNAME(_T("SendNotification"));

    PMLOGMSG(ZONE_NOTIFY, 
        (_T("%s: sending notification to 0x%08x (owner 0x%08x)\r\n"),
        pszFname, ppn, ppn->hOwner));
    if(!WriteMsgQueue(ppn->hMsgQ, ppb, dwLen, 0, 0)) {
        PMLOGMSG(ZONE_WARN, (_T("%s: WriteMsgQueue(0x%08x, 0x%08x, 0x%x) failed %d\r\n"),
            pszFname, ppn->hMsgQ, ppb, dwLen, GetLastError()));
    }        
}

// This routine sends a notification to all interested listeners.
VOID
GenerateNotifications(PPOWER_BROADCAST ppb) 
{
    PPOWER_NOTIFICATION ppn;
    DWORD dwLen;
    SETFNAME(_T("GenerateNotifications"));

    PREFAST_DEBUGCHK(ppb != NULL);

    // Calculate the amount of data to send, since these are variable length
    // messages.  Always send at least sizeof(POWER_BROADCAST) bytes so that
    // the receiver doesn't have to worry about structure alignment and size
    // issues when they receive the message.
    dwLen = ppb->Length + (3 * sizeof(DWORD));
    if(dwLen < sizeof(POWER_BROADCAST)) {
        dwLen = sizeof(POWER_BROADCAST);
    }

    PMLOGMSG(ZONE_NOTIFY, (_T("%s: sending type %d notifications (%d bytes)\r\n"),
        pszFname, ppb->Message, dwLen));

    PMLOCK();
    for(ppn = gpPowerNotifications; ppn != NULL; ppn = ppn->pNext) {
        // is this a message type for which the client has registered?
        if((ppn->dwFlags & ppb->Message) != 0) {
            // yes, send the message
            SendNotification(ppn, ppb, dwLen);
        }
    }
    PMUNLOCK();

    PMLOGMSG(ZONE_NOTIFY, (_T("%s: done sending type %d notifications\r\n"),
        pszFname, ppb->Message));
}

// This routine deletes all notification structures associated with a
// particular process.  It should be called when the process exits.
VOID
DeleteProcessNotifications(HANDLE hProcess)
{
    BOOL fDone = FALSE;
    SETFNAME(_T("DeleteProcessNotifications"));

    PMLOCK();

    // remove all notifications belonging to this process.  If we
    // remove one from the list, start again -- we will eventually run
    // out of notifications for this process.
    while(!fDone) {
        PPOWER_NOTIFICATION ppn;
        for(ppn = gpPowerNotifications; ppn != NULL; ppn = ppn->pNext) {
            if(ppn->hOwner == hProcess) {
                PMLOGMSG(ZONE_NOTIFY, (_T("%s: deleting notification 0x%08x\r\n"),
                    pszFname, ppn));
                PowerNotificationRemList(&gpPowerNotifications, ppn);
                break;
            }
        }

        // did we get through the whole list?
        if(ppn == NULL) {
            // yes, no need to re-iterate
            fDone = TRUE;
        }
    }

    PMUNLOCK();
}

// Applications can call this routine to get various kinds of notifications.
// This routine returns an opaque pointer to a notification handle if
// successful and sets the system error code to:
//      ERROR_SUCCESS - request registered ok.
//      ERROR_INVALID_PARAMETER - bad message queue or flags values
// Processes can have more than one notification; this allows multiple threads
// to listen for events.
EXTERN_C HANDLE WINAPI 
PmRequestPowerNotifications(HANDLE hMsgQ, DWORD dwFlags)
{
    HANDLE hOwner = GetCallerProcess();
    DWORD dwStatus = ERROR_SUCCESS;
    PPOWER_NOTIFICATION ppn = NULL;
    SETFNAME(_T("PmRequestPowerNotifications"));

    PMLOGMSG(ZONE_NOTIFY || ZONE_API, 
        (_T("%s: creating notification type 0x%08x for 0x%08x\r\n"),
        pszFname, dwFlags, hOwner));

    // check parameters
    if(hMsgQ == NULL) {
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    // create the structure if parameters are all right
    if(dwStatus == ERROR_SUCCESS) {
        ppn = PowerNotificationCreate(hMsgQ, hOwner);
        if(ppn != NULL) {
            ppn->dwFlags = dwFlags;

            PMLOCK();
            // add the notification structure to the list
            PowerNotificationAddList(&gpPowerNotifications, ppn);

            // if the notification requires any platform-specific action,
            // the platform can do it now
            PlatformSendInitialNotifications(ppn, dwFlags);
            PMUNLOCK();
        } else {
            dwStatus = GetLastError();  // set by whatever failed in PowerNotificationCreate()
            DEBUGCHK(dwStatus != ERROR_SUCCESS);
        }
    }

    PMLOGMSG(ZONE_NOTIFY || ZONE_API || (dwStatus != ERROR_SUCCESS && ZONE_WARN),
        (_T("%s: returning 0x%08x, status %d\r\n"), pszFname,
        ppn, dwStatus));
    SetLastError(dwStatus);
    return ppn;
}

// This routine deregisters a notification request created with
// PmRequestPowerNotifications().  It returns:
//      ERROR_SUCCESS - notification deleted
//      ERROR_INVALID_PARAMETER - bad handle value
//      ERROR_FILE_NOT_FOUND - handle doesn't exist
EXTERN_C DWORD WINAPI 
PmStopPowerNotifications(HANDLE h)
{
    DWORD dwStatus = ERROR_INVALID_PARAMETER;
    PPOWER_NOTIFICATION ppn = (PPOWER_NOTIFICATION) h;
    SETFNAME(_T("PmStopPowerNotifications"));

    PMLOGMSG(ZONE_NOTIFY || ZONE_API, (_T("%s: handle is 0x%08x\r\n"), pszFname, h));
    if(ppn != NULL) {
        PMLOCK();
        BOOL fFound = PowerNotificationRemList(&gpPowerNotifications, ppn);
        if(fFound) {
            dwStatus = ERROR_SUCCESS;
        } else {
            dwStatus = ERROR_FILE_NOT_FOUND;
        }
        PMUNLOCK();
    }

    PMLOGMSG(ZONE_NOTIFY || ZONE_API || (dwStatus != ERROR_SUCCESS && ZONE_WARN),
        (_T("%s: returning status %d\r\n"), pszFname, dwStatus));
    return dwStatus;
}


