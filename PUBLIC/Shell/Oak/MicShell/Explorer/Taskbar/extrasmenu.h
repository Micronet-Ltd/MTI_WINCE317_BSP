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

#ifndef _EXTRASMENUBUTTON_H_
#define _EXTRASMENUBUTTON_H_

#include "taskbarmenubutton.h"

class ExtrasMenuButton :
    public TaskbarMenuButton
{
protected:
    int GetMyIndex();

public:
    ExtrasMenuButton() : TaskbarMenuButton() {}
    ExtrasMenuButton(HICON hIcon, UINT uFlags, RECT inRect, HWND hwnd, LPWSTR pwszText);
    ExtrasMenuButton(RECT inRect);
    virtual ~ExtrasMenuButton();

    void    LaunchMenu(HWND hwndTaskBar, POINT pt);
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT ProcessCommand(HWND hwnd, WPARAM wParam, LPARAM lParam);
    LRESULT MeasureItem(MEASUREITEMSTRUCT *mis);
    LRESULT DrawMenuItem(DRAWITEMSTRUCT *dis);
    LRESULT InitPopup(HMENU hMenu);

    virtual BOOL GetTooltipText(LPWSTR pwszTT, int cchTT);

};
#endif //_EXTRASMENUBUTTON_H_

