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

#ifndef _TASKBARBUBBLE_H_
#define _TASKBARBUBBLE_H_

#include "taskbarnotification.h"
#include <aygshell.h>

class TaskbarBubble :
    public TaskbarNotification
{
public:
    DWORD m_csDuration;
    SHNP m_npPriority;
    DWORD m_grfFlags;
    CLSID m_clsid;
    LPCWSTR m_pwszHTML;
    LPARAM m_lParam;
    UINT m_uiStartTime;
    bool m_bWaitingForShow;

    TaskbarBubble();
    TaskbarBubble(PSHNOTIFICATIONDATA pND);
    virtual ~TaskbarBubble();
};

#endif //_TASKBARBUBBLE_H_

