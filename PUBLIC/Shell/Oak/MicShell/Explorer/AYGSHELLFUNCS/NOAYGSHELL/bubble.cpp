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
#include <windows.h>
#include <aygshell.h>
#include "taskbar.hxx"
#include "bubble.h"

CHtmlBubble::CHtmlBubble()
{
}

CHtmlBubble::~CHtmlBubble()
{
}

typedef HRESULT (*IECTLINIT)(HINSTANCE);
BOOL CHtmlBubble::Init(HWND hwndParent)
{
    UNREFERENCED_PARAMETER( hwndParent );
    return FALSE;
}

BOOL CHtmlBubble::IsBeingShown( CLSID clsid, DWORD dwID )
{
    UNREFERENCED_PARAMETER( clsid );
    UNREFERENCED_PARAMETER( dwID );

    return FALSE;
}

BOOL CHtmlBubble::PopUp(TaskbarBubble *pBubble, BOOL bShowingTrayBubble)
{
    UNREFERENCED_PARAMETER(pBubble);
    UNREFERENCED_PARAMETER( bShowingTrayBubble );

    return FALSE;
}

BOOL CHtmlBubble::PopDown( BOOL bSendMessage, BOOL bTimerExpired )
{
    UNREFERENCED_PARAMETER( bSendMessage );
    UNREFERENCED_PARAMETER( bTimerExpired );

    return FALSE;
}

void CHtmlBubble::HideBubble()
{
}

void CHtmlBubble::ShowBubble()
{
}


BOOL CHtmlBubble::CreateBubbleWindow()
{
    return FALSE;
}

BOOL CHtmlBubble::CreateBubbleControl()
{
    return FALSE;
}

BOOL CHtmlBubble::GetCallbackInterface( IShellNotificationCallback **ppishnc )
{
    UNREFERENCED_PARAMETER( ppishnc );
    return FALSE;
}

void CHtmlBubble::OnBubbleDismissed( BOOL bTimerExpired )
{
    UNREFERENCED_PARAMETER( bTimerExpired );
}

void CHtmlBubble::HandleBubbleLink(WCHAR *pszUrl)
{
    UNREFERENCED_PARAMETER( pszUrl );
}

void CHtmlBubble::HandleBubbleCommand( DWORD dwCmdId )
{
    UNREFERENCED_PARAMETER( dwCmdId );
}

LRESULT CHtmlBubble::SendNotifyToSink(HWND hwndSink, NMSHN* pnmshnOriginal)
{
    UNREFERENCED_PARAMETER( hwndSink );
    UNREFERENCED_PARAMETER( pnmshnOriginal );
    
    return FALSE;
}

LRESULT CALLBACK CHtmlBubble::s_BubbleWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    UNREFERENCED_PARAMETER( hwnd );
    UNREFERENCED_PARAMETER( msg );
    UNREFERENCED_PARAMETER( wp );
    UNREFERENCED_PARAMETER( lp );
    
    return FALSE;
}

LRESULT CALLBACK CHtmlBubble::BubbleWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)   
{
    UNREFERENCED_PARAMETER( hwnd );
    UNREFERENCED_PARAMETER( msg );
    UNREFERENCED_PARAMETER( wp );
    UNREFERENCED_PARAMETER( lp );
    
    return FALSE;
}

