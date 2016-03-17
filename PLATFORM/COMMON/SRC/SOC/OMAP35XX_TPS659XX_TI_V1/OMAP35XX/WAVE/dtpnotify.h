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

#ifndef _DTPNOTIFY_H_
#define _DTPNOTIFY_H_

#if (_MSC_VER >= 1000)
#pragma once
#endif

#include <linklist.h>

//------------------------------------------------------------------------------
//
// Device topology notifications.
//

class DtpNotify
{
public:

    DtpNotify();
    virtual ~DtpNotify();

    DWORD RegisterDtpMsgQueue(HANDLE hMsgQueue, HANDLE hClientProc);
    DWORD UnregisterDtpMsgQueue(HANDLE hMsgQueue, HANDLE hClientProc);
    DWORD SendDtpNotifications(const PVOID pvMsg, DWORD cbMsg);

private:

    VOID DeleteNotifyList();

    VOID LockList() { EnterCriticalSection(&m_csListLock); }
    VOID UnlockList() { LeaveCriticalSection(&m_csListLock); }

    LIST_ENTRY m_NotifyList;
    CRITICAL_SECTION m_csListLock;
};

#endif // _DTPNOTIFY_H_
