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

#ifndef _TASKBARMENUBUTTON_H_
#define _TASKBARMENUBUTTON_H_

#include "taskbaritem.h"

class TaskbarMenuButton : public TaskbarItem
{
protected:
	BOOL m_fInMenu;

public:
    TaskbarMenuButton() : TaskbarItem() { m_fInMenu = FALSE; }
    TaskbarMenuButton(HICON icon, UINT flags, RECT inrect, HWND hwnd, TCHAR *pText) : TaskbarItem(icon, flags, inrect, hwnd, pText) { m_fInMenu = FALSE; }
	virtual ~TaskbarMenuButton() {}

	BOOL InMenu() { return m_fInMenu; }
	virtual LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) { return 0; }
	virtual void LaunchMenu(HWND hwndTaskBar, POINT pt) {}
};
#endif //_TASKBARMENUBUTTON_H_
