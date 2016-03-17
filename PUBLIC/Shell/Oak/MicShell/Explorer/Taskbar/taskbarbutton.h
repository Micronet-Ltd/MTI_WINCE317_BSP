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

#ifndef _TASKBARBUTTON_H_
#define _TASKBARBUTTON_H_

#include "taskbaritem.h"

class TaskbarButton : public TaskbarItem
{
#if 0
protected:
    BOOL DrawButton(HDC hdc, UINT edge, HBRUSH hbr, BOOL fForceEdge);

public:
    TaskbarButton(HWND hwnd);
    ~TaskbarButton();

    int HandleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    BOOL DrawItem(HWND hwnd, HDC hdc, BOOL fPressed,  BOOL fSmallScreen);
#else
public:
    TaskbarButton() {}
#endif

};

#endif //_TASKBARBUTTON_H_
