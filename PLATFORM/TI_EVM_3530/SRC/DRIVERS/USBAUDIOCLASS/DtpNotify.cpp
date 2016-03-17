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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//

#include <windows.h>
#include "DtpNotify.h"

// Device topology notification sink
typedef struct _DTP_NOTIFY_SINK
{
    HANDLE hMsgQueue;
    HANDLE hMsgQueueClientProc;
    HANDLE hClientProc;
    LIST_ENTRY Link;
} DTP_NOTIFY_SINK, *PDTP_NOTIFY_SINK;

//------------------------------------------------------------------------------
//
// Device topology notifications.
//

DtpNotify::DtpNotify()
{
    InitializeCriticalSection(&m_csListLock);
    InitializeListHead(&m_NotifyList);
}

DtpNotify::~DtpNotify()
{
    DeleteNotifyList();
    DeleteCriticalSection(&m_csListLock);
}

DWORD DtpNotify::RegisterDtpMsgQueue( HANDLE hMsgQueue, HANDLE hClientProc )
{
    DWORD mmRet = MMSYSERR_NOERROR;

    if ((NULL == hMsgQueue) || (NULL == hClientProc))
    {
        mmRet = MMSYSERR_INVALPARAM;
        goto Error;
    }

    // Open the message queue for write access.
    MSGQUEUEOPTIONS msgopts;
    ZeroMemory(&msgopts, sizeof(msgopts));
    msgopts.dwSize = sizeof(msgopts);
    msgopts.bReadAccess = FALSE;
    HANDLE hMsgQueueWrite = OpenMsgQueue(hClientProc, hMsgQueue, &msgopts);
    if (NULL == hMsgQueueWrite)
    {
        mmRet = MMSYSERR_ERROR;
        goto Error;
    }

    // Create sink object and add to list.  Keep the client proc msg queue for 
    // unregister.
    PDTP_NOTIFY_SINK pDtpNotifySink = new DTP_NOTIFY_SINK;
    if (NULL == pDtpNotifySink)
    {
        mmRet = MMSYSERR_NOMEM;
        goto Error;
    }

    pDtpNotifySink->hMsgQueue = hMsgQueueWrite;
    pDtpNotifySink->hMsgQueueClientProc = hMsgQueue;
    pDtpNotifySink->hClientProc = hClientProc;

    LockList();
    InsertTailList(&m_NotifyList, &pDtpNotifySink->Link);
    UnlockList();

Error:

    return mmRet;
}

DWORD DtpNotify::UnregisterDtpMsgQueue( HANDLE hMsgQueue, HANDLE hClientProc )
{
    DWORD mmRet = MMSYSERR_NOERROR;

    if ((NULL == hMsgQueue) || (NULL == hClientProc))
    {
        return MMSYSERR_INVALPARAM;
    }

    LockList();

    // Search for the sink object to remove.
    BOOL fUnregistered = FALSE;
    PLIST_ENTRY pListEntry;
    for (pListEntry = m_NotifyList.Flink;
         pListEntry != &m_NotifyList;
         pListEntry = pListEntry->Flink)
    {
        PDTP_NOTIFY_SINK pDtpNotifySink = CONTAINING_RECORD(
                                                pListEntry,
                                                DTP_NOTIFY_SINK,
                                                Link);
        // Compare the client proc message queue handle.
        if ((hMsgQueue == pDtpNotifySink->hMsgQueueClientProc) &&
            (hClientProc == pDtpNotifySink->hClientProc))
        {
            // Remove from list and clean up.
            RemoveEntryList(&pDtpNotifySink->Link);
            CloseHandle(pDtpNotifySink->hMsgQueue);
            delete pDtpNotifySink;

            fUnregistered = TRUE;
            break;
        }
    }

    UnlockList();

    if (!fUnregistered)
    {
        mmRet = MMSYSERR_INVALPARAM;
    }

    return mmRet;
}

DWORD DtpNotify::SendDtpNotifications( const PVOID pvMsg, DWORD cbMsg )
{
    LockList();

    // Send notification to all registered clients.
    PLIST_ENTRY pListEntry;
    for (pListEntry = m_NotifyList.Flink;
         pListEntry != &m_NotifyList;
         pListEntry = pListEntry->Flink)
    {
        PDTP_NOTIFY_SINK pDtpNotifySink = CONTAINING_RECORD(
                                                pListEntry,
                                                DTP_NOTIFY_SINK,
                                                Link);
        WriteMsgQueue(pDtpNotifySink->hMsgQueue, pvMsg, cbMsg, 0, 0);
    }

    UnlockList();

    return MMSYSERR_NOERROR;
}

VOID DtpNotify::DeleteNotifyList()
{
    LockList();

    while (!IsListEmpty(&m_NotifyList))
    {
        // Remove from list and clean up.
        PLIST_ENTRY pListEntry = RemoveHeadList(&m_NotifyList);
        PDTP_NOTIFY_SINK pDtpNotifySink = CONTAINING_RECORD(
                                                pListEntry,
                                                DTP_NOTIFY_SINK,
                                                Link);
        CloseHandle(pDtpNotifySink->hMsgQueue);
        delete pDtpNotifySink;
    }

    UnlockList();
}

