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
#include "windows.h"
#include "taskbar.hxx"
#include <aygshell.h>
#include <shellsdkguids.h>
#include "TaskbarBubble.h"
#include "bubble.h"

#ifndef _PREFAST_
#pragma warning(disable:4068)
#endif // _PREFAST_

extern HINSTANCE g_hInstance;
extern LRESULT SendBubbleNotificationMessage(HWND hwnd, NMSHN *pNMSHN, WCHAR *pszTarget);

#define SAFE_RELEASE(p) if (p) { p->Release(); p = NULL; }

CHtmlBubble::CHtmlBubble()
{
    m_hwndBubble = NULL;
    m_hwndParent = NULL;
    m_hwndHTML = NULL;
    m_hinstIECTL = NULL;
    m_fInited = 0;
    m_ptbBubble = NULL;
    m_pishnc = NULL;
}

CHtmlBubble::~CHtmlBubble()
{
    if (m_hwndBubble)
    {
        PopDown();
    }

    if (m_hinstIECTL)
    {
        FreeLibrary(m_hinstIECTL);
    }

    SAFE_RELEASE(m_pishnc);
}

typedef HRESULT (*IECTLINIT)(HINSTANCE);
BOOL CHtmlBubble::Init(HWND hwndParent)
{
    if (0 == m_fInited)
    {
        WNDCLASS wc;
        
        memset(&wc, 0, sizeof(wc));
        wc.lpfnWndProc = (WNDPROC)CHtmlBubble::s_BubbleWndProc;   
        wc.hInstance = g_hInstance;
        wc.lpszClassName = SZ_BUBBLE_WNDCLASS;
        wc.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_WINDOW);
        
        if (!RegisterClass(&wc))
        {
            goto Exit;
        }

#pragma prefast(suppress:321, "Not loading dll via relative path") 
        m_hinstIECTL = LoadLibrary(L"iectl");
        if (m_hinstIECTL)
        {
            IECTLINIT pfnInit = (IECTLINIT)GetProcAddress(m_hinstIECTL, L"InitHTMLControl");
            if (pfnInit && S_OK == pfnInit(g_hInstance))
            {
                m_fInited = 1;
                goto Exit;
            }
        }
        m_fInited = 2;  // mark so we don't keep trying if we've failed. 
    }
Exit:
    m_hwndParent = hwndParent;
    return (1 == m_fInited);
    
}

BOOL CHtmlBubble::IsBeingShown(CLSID clsid, DWORD dwID)
{
    if (!m_ptbBubble)
    {
        return false;
    }
    return (((DWORD) m_ptbBubble->m_uID == dwID) && IsEqualCLSID(m_ptbBubble->m_clsid, clsid));
}

BOOL CHtmlBubble::PopUp(TaskbarBubble *pBubble, BOOL bShowingTrayBubble)
{
    BOOL bRet = FALSE;

    if (!pBubble || !pBubble->m_pwszHTML || m_fInited != 1)
    {   
        return FALSE;
    }

    if (bShowingTrayBubble)
    {
        PopDown();
    }
    else
    {
        if (m_hwndBubble)
        {
            return FALSE;
        }
    }

    m_ptbBubble = pBubble;
 
    if (m_hwndBubble && m_hwndHTML)
    {              
        bRet = TRUE;
    }
    else
    {
        if (CreateBubbleWindow() && CreateBubbleControl())
        {
            bRet = TRUE;
        }
    }
    
    if (bRet)
    {
        SendMessage(m_hwndHTML, WM_SETTEXT, 0, (LPARAM)pBubble->m_pwszHTML);
        ShowBubble();
        if (pBubble->m_csDuration)
        {
            SetTimer(m_hwndBubble, TIMER_POPBUBBLE, bShowingTrayBubble ? INFINITE : pBubble->m_csDuration * 1000, NULL);
        }
    }

    // send a message to the window, in our case the bubble points to nothing
    NMSHN nmshn;
    nmshn.hdr.hwndFrom = NULL;
    nmshn.hdr.idFrom = (DWORD) pBubble->m_uID;
    nmshn.hdr.code = SHNN_SHOW;
    nmshn.pt.x = 0;
    nmshn.pt.y = 0;
    nmshn.lParam = pBubble->m_lParam;
    SendNotifyToSink(pBubble->m_hwndMain, &nmshn);

    return bRet;
}

BOOL CHtmlBubble::PopDown(BOOL bSendMessage, BOOL bTimerExpired)
{
    if (m_hwndBubble)
    {
        KillTimer(m_hwndBubble, TIMER_POPBUBBLE);
        HideBubble();
        DestroyWindow(m_hwndBubble);
        m_hwndBubble = NULL;
        m_hwndHTML = NULL;
        if (bSendMessage)
        {
            // means user or timer closed bubble instead of us
            OnBubbleDismissed(bTimerExpired);
        }
    }

    if (bSendMessage && m_hwndParent)
    {
        NM_BUBBLEDONEINFO *pNM = (NM_BUBBLEDONEINFO *) LocalAlloc(sizeof(NM_BUBBLEDONEINFO), LMEM_FIXED);
        if (pNM)
        {
            pNM->clsid = m_ptbBubble->m_clsid;
            pNM->dwID = (DWORD) m_ptbBubble->m_uID;
        }
        PostMessage(m_hwndParent, NM_BUBBLEDONE, (WPARAM) 0, (LPARAM) pNM);
    }

    m_ptbBubble = NULL;
    SAFE_RELEASE(m_pishnc);

    return TRUE;
}

void CHtmlBubble::HideBubble()
{
    if (m_hwndBubble)
    {
        ShowWindow(m_hwndBubble, SW_HIDE);
    }
   
}

void CHtmlBubble::ShowBubble()
{
    if (m_hwndBubble && m_hwndHTML)
    {
        SetWindowPos(m_hwndBubble, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);    
    }
}


BOOL CHtmlBubble::CreateBubbleWindow()
{
    RECT rcArea;

    if (m_hwndBubble)
    {
        return TRUE;
    }

    SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, 0);

    m_hwndBubble = CreateWindowEx(WS_EX_TOOLWINDOW,
                                   SZ_BUBBLE_WNDCLASS, 
                                   L"", 
                                   WS_BORDER|WS_POPUP, 
                                   rcArea.right - CX_POPUP - CX_PADDING, 
                                   rcArea.bottom - CY_POPUP - CY_PADDING, 
                                   CX_POPUP, CY_POPUP, 
                                   NULL, // m_hwndParent, 
                                   NULL, g_hInstance, (void *)this);

    if (m_hwndBubble)
    {                
        SetWindowLong(m_hwndBubble, GWL_USERDATA, (LONG)this);
        return TRUE;
    }

    return FALSE;

}

BOOL CHtmlBubble::CreateBubbleControl()
{
    if (m_hwndBubble)
    {
        m_hwndHTML = CreateWindow(SZ_HTMLCTL_WNDCLASS,
                                   L"",
                                   WS_CHILD,
                                   0,0,
                                   CX_POPUP, CY_POPUP,
                                   m_hwndBubble,
                                   NULL,
                                   g_hInstance,
                                   NULL);
        if (m_hwndHTML)
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CHtmlBubble::GetCallbackInterface(IShellNotificationCallback **ppishnc)
{
    HRESULT hr = S_OK;

    if (m_pishnc)
    {
        // already have it
        *ppishnc = m_pishnc;
        return TRUE;
    }

    // need to cocreate it
    if (IsEqualCLSID(m_ptbBubble->m_clsid, CLSID_NULL))
    {
        *ppishnc = NULL;
        return FALSE;
    }

    hr = CoCreateInstance(m_ptbBubble->m_clsid, NULL, CLSCTX_ALL,
        IID_IShellNotificationCallback,
        (void **)&m_pishnc);

    if (NULL != m_pishnc)
    {
        *ppishnc = m_pishnc;
    }

    if (SUCCEEDED(hr))
    {
        return TRUE;
    }

    return FALSE;
}

void CHtmlBubble::OnBubbleDismissed(BOOL bTimerExpired)
{
    IShellNotificationCallback *pishnc;

    // try the COM object callback
    if (GetCallbackInterface(&pishnc) && (NULL != pishnc))
    {
        // call the callback
        pishnc->OnDismiss((DWORD) m_ptbBubble->m_uID, bTimerExpired, m_ptbBubble->m_lParam);
    }

    // try the window-based callback

    NMSHN nmshn;
    nmshn.hdr.hwndFrom = NULL;
    nmshn.hdr.idFrom = (DWORD) m_ptbBubble->m_uID;
    nmshn.hdr.code = SHNN_DISMISS;
    nmshn.fTimeout = bTimerExpired;
    nmshn.lParam = m_ptbBubble->m_lParam;
    SendNotifyToSink(m_ptbBubble->m_hwndMain, &nmshn);
}

void CHtmlBubble::HandleBubbleLink(LPWSTR pwszUrl)
{
    // m_tbiiBubble has the information from the initial popup
    if (pwszUrl)
    {
        IShellNotificationCallback *pishnc;
        if (GetCallbackInterface(&pishnc) && pishnc)
        {
            // call the callback
            pishnc->OnLinkSelected((DWORD) m_ptbBubble->m_uID, pwszUrl, m_ptbBubble->m_lParam);
        }
    }

    if (m_ptbBubble->m_hwndMain)
    {
        NMSHN nmshn;
        nmshn.hdr.hwndFrom = NULL;
        nmshn.hdr.idFrom = (DWORD) m_ptbBubble->m_uID;
        nmshn.hdr.code = SHNN_LINKSEL;
        nmshn.lParam = m_ptbBubble->m_lParam;
        nmshn.dwReturn = 0; //?

        SendBubbleNotificationMessage(m_ptbBubble->m_hwndMain, &nmshn, pwszUrl);
    }
}

void CHtmlBubble::HandleBubbleCommand(DWORD dwCmdId)
{
     // m_tbiiBubble has the information from the initial popup
    IShellNotificationCallback *pishnc;

    if (dwCmdId != IDCANCEL)
    {
        // try the COM object callback
        DWORD dwID = (DWORD) m_ptbBubble->m_uID;

        if (GetCallbackInterface(&pishnc) && (NULL != pishnc))
        {
            // call the callback
            pishnc->OnCommandSelected(dwID, (int) dwCmdId);
        }

        if (NULL != m_ptbBubble->m_hwndMain)
        {
            // Post the window-based callback
            PostMessage(m_ptbBubble->m_hwndMain, WM_COMMAND, MAKEWPARAM(dwCmdId, 0), (LPARAM)dwID);
        }
    }
}

// this will send a notify message to a sink.
// it'll marshal the notify structure, but assumes there are no pointers in the struct
LRESULT CHtmlBubble::SendNotifyToSink(HWND hwndSink, NMSHN* pnmshnOriginal)
{
    return SendBubbleNotificationMessage(hwndSink, pnmshnOriginal, NULL);
}

LRESULT CALLBACK CHtmlBubble::s_BubbleWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CHtmlBubble *bubble = (CHtmlBubble *)GetWindowLong(hwnd, GWL_USERDATA);

    // handle WM_CREATE here with bubble = lParam
    if (NULL != bubble)
    {
        return bubble->BubbleWndProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK CHtmlBubble::BubbleWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)   
{
    switch(msg)
    {
        case WM_COMMAND:
            if (m_hwndHTML == (HWND)lParam)
            {
                HandleBubbleCommand(LOWORD(wParam));
                PopDown(TRUE);
            }
            break;
        case WM_TIMER:
            if (wParam == TIMER_POPBUBBLE)
            {
                PopDown(TRUE, TRUE);
            }
            break;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE)
            {
                PopDown(TRUE);
            }
            break;
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                PopDown(TRUE);
            }
            break;
        case WM_NOTIFY:
            {
                NM_HTMLVIEW *pnmh = (NM_HTMLVIEW *)lParam;

                if (pnmh->hdr.code == NM_HOTSPOT)
                {
                    HandleBubbleLink((WCHAR *)pnmh->szTarget);
                    PopDown(TRUE);
                }
            }
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

