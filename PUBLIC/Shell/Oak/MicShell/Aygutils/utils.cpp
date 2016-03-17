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
#include <shellsdk.h>

#ifndef _PREFAST_
#pragma warning(disable:4068)
#endif // _PREFAST_

HINSTANCE        g_hInstAygshell = NULL;
BOOL             g_bCalledSHInitExtraControls = FALSE;
BOOL             g_bLoadedAygshellLibrary = FALSE;
int              g_iAygshellRefCount = 0;
CRITICAL_SECTION g_csAygshell;
BOOL             g_bInitialized = FALSE;

BOOL InitializeAygUtils()
{
    InitializeCriticalSection( &g_csAygshell );
    g_bInitialized = TRUE;
    return TRUE;
}

BOOL UnInitializeAygUtils()
{
    DeleteCriticalSection( &g_csAygshell );
    g_bInitialized = FALSE;
    return TRUE;
}

BOOL LoadAygshellLibrary()
{
    BOOL bReturn = FALSE;
    if ( !g_bInitialized )
    {
        return FALSE;
    }

    EnterCriticalSection( &g_csAygshell );

    if ( g_hInstAygshell )
    {
        ASSERT( g_iAygshellRefCount > 0 );
        g_iAygshellRefCount++;
        bReturn = TRUE;
    }
    else
    {
        ASSERT( g_iAygshellRefCount == 0 );
#pragma prefast(suppress:321, "Not loading dll via relative path") 
        g_hInstAygshell = LoadLibrary( TEXT( "aygshell.dll" ) );
        if ( g_hInstAygshell )
        {
            g_iAygshellRefCount++;
            g_bLoadedAygshellLibrary = TRUE;
            bReturn = TRUE;
        }
    }

    LeaveCriticalSection( &g_csAygshell );

    return bReturn;
}

BOOL FreeAygshellLibrary()
{
    if ( !g_bInitialized || !g_bLoadedAygshellLibrary )
    {
        return FALSE;
    }

    ASSERT( g_hInstAygshell );
    ASSERT( g_iAygshellRefCount > 0 );

    EnterCriticalSection( &g_csAygshell );

    g_iAygshellRefCount--;

    if ( g_iAygshellRefCount < 1 )
    {
        FreeLibrary( g_hInstAygshell );
        g_hInstAygshell = NULL;
        g_bCalledSHInitExtraControls = FALSE;
    }

    LeaveCriticalSection( &g_csAygshell );

    return TRUE;
}


BOOL AygInitDialog( HWND hwnd, DWORD dwFlags )
{
    BOOL bReturn = FALSE;
    SHINITDLGINFO shidi = { 0 };
    BOOL (*PFN_SHInitDialog)(PSHINITDLGINFO) = NULL;

    if ( !g_bInitialized )
    {
        return FALSE;
    }

    if ( !LoadAygshellLibrary() )
    {
        return FALSE;
    }
    
    shidi.dwMask = SHIDIM_FLAGS;
    shidi.hDlg = hwnd;
    shidi.dwFlags = dwFlags;
    PFN_SHInitDialog = (BOOL (*)(PSHINITDLGINFO)) GetProcAddress( g_hInstAygshell, TEXT("SHInitDialog") );
    if (PFN_SHInitDialog)
    {
        bReturn = PFN_SHInitDialog( &shidi );
    }

    if ( !FreeAygshellLibrary() )
    {
        return FALSE;
    }

    return bReturn;
}

BOOL AygInitExtraControls()
{
    BOOL bReturn = FALSE;
    BOOL (*PFN_SHInitExtraControls)() = NULL;

    if ( !g_bInitialized || !g_hInstAygshell )
    {
        return FALSE;
    }

    if ( g_bCalledSHInitExtraControls )
    {
        return TRUE;
    }

    PFN_SHInitExtraControls = (BOOL (*)()) GetProcAddress( g_hInstAygshell, TEXT( "SHInitExtraControls" ) );
    if ( PFN_SHInitExtraControls )
    {
        bReturn = PFN_SHInitExtraControls();
    }

    g_bCalledSHInitExtraControls = bReturn;

    return bReturn;
}

BOOL AygAddSipprefControl( HWND hwnd )
{
    if ( !g_bInitialized || !g_hInstAygshell || !AygInitExtraControls() )
    {
        return FALSE;
    }

    WNDCLASS wc;
    if (GetClassInfo(g_hInstAygshell, WC_SIPPREF, &wc))
    {
       CreateWindow(WC_SIPPREF, NULL, WS_CHILD, -10, -10,
                    5, 5, hwnd, NULL, g_hInstAygshell, NULL);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}
