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

#ifndef _TASKBARAPPBUTTON_H_
#define _TASKBARAPPBUTTON_H_

#include "taskbarbutton.h"

class TaskbarApplicationButton :
    public TaskbarButton
{
public:
    bool m_bHibernate;
    bool m_bMarkedFullscreen;
    bool m_bMarkedTopmost;

    TaskbarApplicationButton();
    TaskbarApplicationButton(HWND hwnd);
    virtual ~TaskbarApplicationButton();
};

#endif //_TASKBARAPPBUTTON_H_

