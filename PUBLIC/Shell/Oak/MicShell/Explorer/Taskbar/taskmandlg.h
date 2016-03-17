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
#ifndef _TASKMANDLG_H_
#define _TASKMANDLG_H_

class CTaskBar;

class TaskManDlg
{
private:
    HWND m_hwndTaskMan;

    LRESULT CALLBACK EndTaskDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK s_EndTaskDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    int CALLBACK TaskManDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static int CALLBACK s_TaskManDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
    TaskManDlg();
    ~TaskManDlg();

    void Create();
    void Show();
    void Destroy();
    BOOL IsTaskManDialogMessage(MSG msg);
};

#endif //_TASKMANDLG_H_

