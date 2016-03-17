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

#pragma once

#include "ceshpriv.h"


class TaskbarList_t : public ITaskbarList2
{
protected:
    DWORD m_ObjRefCount;
    HWND m_hwndTaskbarList;
    UINT WM_ShellHook;


    ~TaskbarList_t();
    HWND GetTaskbarWindow(void);

public:
    TaskbarList_t();

    // IUnknown Methods
    STDMETHOD  (QueryInterface)(REFIID, void **);
    STDMETHOD_ (DWORD, AddRef)();
    STDMETHOD_ (DWORD, Release)();

    // ITaskbarList Methods
    STDMETHOD (HrInit)(void);
    STDMETHOD (AddTab)(HWND hwnd);
    STDMETHOD (DeleteTab)(HWND hwnd);
    STDMETHOD (ActivateTab)(HWND hwnd);
    STDMETHOD (SetActiveAlt)(HWND hwnd);

    // ITaskbarList2 Methods
    STDMETHOD (MarkFullscreenWindow)(HWND hwnd, BOOL fFullscreen);
};


