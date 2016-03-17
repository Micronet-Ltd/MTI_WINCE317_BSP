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

#ifndef _H_CBUBBLE_
#define _H_CBUBBLE_

typedef struct tagNM_HTMLVIEW { 
    NMHDR hdr; 
    LPSTR szTarget;
    LPSTR szData;
    DWORD dwCookie;
} NM_HTMLVIEW;

typedef struct tagNM_BUBBLEDONEINFO {
    CLSID clsid;
    DWORD dwID;
} NM_BUBBLEDONEINFO;

#define NM_HOTSPOT    (WM_USER+101)

#define NM_BUBBLEDONE (WM_USER+102)

#define SZ_HTMLCTL_WNDCLASS TEXT("htmctl32")
#define SZ_BUBBLE_WNDCLASS TEXT("bubble_parent")

#define CX_POPUP  200
#define CY_POPUP  200
#define CX_PADDING 10
#define CY_PADDING 10

#define TIMER_POPBUBBLE    100

class CHtmlBubble 
{

public:
    CHtmlBubble();
    ~CHtmlBubble();
    
    BOOL Init(HWND hwndParent);
    BOOL PopUp(TaskbarBubble *ptbb, BOOL bShowingTrayBubble = FALSE);
    BOOL PopDown(BOOL bSendMessage = FALSE, BOOL bTimerExpired = FALSE);
    void HideBubble();
    void ShowBubble();

    LRESULT CALLBACK BubbleWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK s_BubbleWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    BOOL IsBeingShown(CLSID clsid, DWORD dwID);

private:
    BOOL CreateBubbleWindow();
    BOOL CreateBubbleControl();
    void HandleBubbleLink(LPWSTR pwszUrl);    
    void HandleBubbleCommand(DWORD dwCmdId);
    BOOL GetCallbackInterface(IShellNotificationCallback **ppishnc);
    void OnBubbleDismissed(BOOL bTimerExpired = FALSE);
    LRESULT SendNotifyToSink(HWND hwndSink, NMSHN *pnmshnOriginal);

    HWND m_hwndParent;
    HWND m_hwndBubble;
    HWND m_hwndHTML;
    HINSTANCE m_hinstIECTL; 
    UINT m_fInited;
    TaskbarBubble *m_ptbBubble;

    IShellNotificationCallback *m_pishnc;
};

#endif
