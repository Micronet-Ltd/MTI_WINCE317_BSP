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
/*++

	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	PARTICULAR PURPOSE.


	Module Name:  

	sip2.cpp

--*/

#include <windows.h>
#include <commctrl.h>
#include <windef.h>

#include <sipapi.h>
#include "resource.h"

//
// externals
//
BOOL pfnSipShowIM( DWORD dwFlag );
BOOL pfnSipGetCurrentIM( CLSID* pClsid );
BOOL pfnSipSetCurrentIM( CLSID* pClsid );
int pfnSipEnumIM( IMENUMPROC pEnumIMProc );
BOOL pfnSipGetInfo( SIPINFO *pSipInfo );
BOOL pfnSipSetInfo( SIPINFO *pSipInfo );
BOOL pfnSipRegisterNotification( HWND hWnd );
BOOL pfnSipSetDefaultRect( RECT *pRect );

//
// globals
//

IMENUMINFO **g_ppimei = NULL;
unsigned int g_nIms = 0, g_nCurIm;
CLSID g_clsidImCur = CLSID_NULL;
TCHAR const c_szSipRegKey[] = TEXT("ControlPanel\\Sip");
TCHAR const c_szDragStyleRegValue[] = TEXT("DragStyle");

//
// Im list defines and variables.
//


void SIP_FreeImEnums( void );

//
// Sort the IMs alphabetically.
//
__inline static
BOOL WINAPI
SIP_SortImArray( IMENUMINFO **ppimei, int nIms )
{
    //
    // If there aren't any IMs, we're done.
    //
    if( !nIms ) {
        return TRUE;
    }

    //
    // Now sort it.
    //

#define COMPARE_LESS    1
#define SWAP(a,b) { IMENUMINFO *t = a; a = b; b = t; }

    int i, j, low;

    for( i = 0; i < nIms - 1; i++ ) {
        low = i;
        for( j = i + 1; j < nIms; j++ ) {
            if( COMPARE_LESS == CompareString(
                                    LOCALE_USER_DEFAULT,
                                    NORM_IGNORECASE,
                                    ppimei[j]->szName,
                                    -1,
                                    ppimei[low]->szName,
                                    -1 ) ) {
                low = j;
            }
        }
        SWAP( g_ppimei[low], g_ppimei[i] );
    }

    return TRUE;
}


//
// Called by the OS for each installed IM, in response to SipEnumIm.
//
int ImEnumProc( IMENUMINFO *pimei )
{
    if( g_nCurIm >= g_nIms ) {
        return FALSE;
    }

    if( !(g_ppimei[g_nCurIm] = (IMENUMINFO *)LocalAlloc(LMEM_FIXED,
                                                sizeof(IMENUMINFO) )) ) {
        return FALSE;
    }

    *g_ppimei[g_nCurIm++] = *pimei;

    return TRUE;
}


//
// Free IMENUMINFO array
//
void SIP_FreeImEnums( void )
{
    unsigned int i;

    for( i = 0; i < g_nCurIm; i++ ) 
    {
        LocalFree( g_ppimei[i] );
    }
    LocalFree( g_ppimei );
    g_ppimei = NULL;

    return;
}


//
// Show a popup menu allowing selection of the current SIP.
//
BOOL SIP_InitPopup( HMENU hmenu, HINSTANCE hInst, int idFirst )
{
    unsigned int i;
    int nCheck = -1;
	BOOL ret = FALSE;
    DWORD dwFlags;

    if ( !hmenu )
    {
        return FALSE;
    }

    //
    // If g_ppimei is not NULL, free the allocated memory.
    //
    if ( g_ppimei )
    {
        SIP_FreeImEnums();
    }

    //
    // Get number of IMs
    //
    g_nIms = 0;
    g_nCurIm = 0;
    g_nIms = pfnSipEnumIM( NULL );

    if ( g_nIms == 0 )
    {
        return FALSE;
    }

    //
    // Allocate pointer array
    //
    g_ppimei = (IMENUMINFO **)LocalAlloc(LMEM_FIXED,  
                                        g_nIms * sizeof(IMENUMINFO *));  
    if( !g_ppimei ) {
        return FALSE;
    }


    //
    // Enumerate
    //
    g_nCurIm = 0;
    pfnSipEnumIM( ImEnumProc );
    g_nIms = g_nCurIm;


    //
    // Sort it 
    //
    SIP_SortImArray( g_ppimei, g_nIms );
    

    //
    // Find out which IM is currently selected
    //
	pfnSipGetCurrentIM(&g_clsidImCur);

    //
    // Build up the menu.
    //
    for( i = 0; i < g_nIms; i++ )
    {
        if ( g_clsidImCur == g_ppimei[i]->clsid )
        {
            nCheck = i;
        }

        dwFlags = MF_STRING;

        AppendMenu(
            hmenu,
            dwFlags,
            idFirst + i,
            g_ppimei[i]->szName );
    }

    //
    // Add close menu.
    //
    AppendMenu(
        hmenu,
        MFT_SEPARATOR,
        0,
        NULL );

	TCHAR	closetxt[64];
    LoadString(hInst, IDS_CLOSE_SIP, closetxt, sizeof(closetxt)/sizeof(closetxt[0]));
	dwFlags = MF_STRING;

    SIPINFO sipinfo;
    sipinfo.cbSize = sizeof(SIPINFO);
    sipinfo.dwImDataSize = 0;

    if ( pfnSipGetInfo( &sipinfo ) && !( sipinfo.fdwFlags & SIPF_ON ) )
    {
        dwFlags |= MF_GRAYED;
    }

    AppendMenu(
        hmenu,
        dwFlags,
        idFirst + i,
        closetxt );

    if( -1 != nCheck ) {
		CheckMenuItem(hmenu, nCheck, MF_CHECKED | MF_BYPOSITION);
    }

    return TRUE;
}

BOOL SIP_SelectSIP( int iChoice )
{
    BOOL bRet = FALSE;

    //
    // Check for valid selection and select IM if so.
    //
	if(iChoice >= 0 && iChoice < (int) g_nIms)
    {
        if( g_clsidImCur != g_ppimei[iChoice]->clsid )
        {
		    pfnSipSetCurrentIM(&g_ppimei[iChoice]->clsid);
            g_clsidImCur = g_ppimei[iChoice]->clsid;
        }
		pfnSipShowIM(SIPF_ON);
        bRet = TRUE;
	}
    else if(iChoice == g_nIms)
    {
		pfnSipShowIM(SIPF_OFF);
		bRet = TRUE;
	}

	SIP_FreeImEnums();

    return bRet;
}

BOOL SIP_SipRegisterNotification( HWND hWnd )
{
    return pfnSipRegisterNotification( hWnd );
}

BOOL SIP_HaveSip()
{
    return( pfnSipEnumIM( NULL ) > 0 );
}

BOOL SIP_IsSipOn()
{
    SIPINFO sipinfo;
    sipinfo.cbSize = sizeof(SIPINFO);
    sipinfo.dwImDataSize = 0;

    if ( !pfnSipGetInfo( &sipinfo ) )
    {
        return FALSE;
    }
    
    return ( sipinfo.fdwFlags & SIPF_ON );
}

// should be called after the taskbar initializes the workarea
BOOL SIP_InitializeSipRect()
{
    RECT rc, rcDefault;
    int cxBorder = 0;
    int cyBorder = 0;
    HKEY hkey;
    LONG l;
    DWORD dw = 0;
    DWORD dwSize = sizeof(DWORD);

    // check the registry to see if we need room for the borders
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, c_szSipRegKey, 0,0,&hkey))
    {
        l = RegQueryValueEx(hkey, c_szDragStyleRegValue, NULL, NULL, (LPBYTE) &dw, &dwSize);
        if ( ( l == ERROR_SUCCESS ) && ( dw != 0 ) )
        {
            cxBorder = 2;
            cyBorder = GetSystemMetrics(SM_CYCAPTION) + 2;
        }
        RegCloseKey( hkey );
    }
   
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, FALSE);
    rcDefault.left = rc.right - (240 + cxBorder);
    rcDefault.right = rc.right;
    rcDefault.top = rc.bottom - (80 + cyBorder);
    rcDefault.bottom = rc.bottom;

    return pfnSipSetDefaultRect( &rcDefault );
}

// make sure the sip isn't off the screen
BOOL SIP_UpdateSipPosition()
{
    BOOL fMove = FALSE;

    // Obtain current window resolution
    RECT rcwnd;
    SIPINFO sipinfo;
    memset(&sipinfo, 0, sizeof(SIPINFO));
    sipinfo.cbSize = sizeof(SIPINFO);

    if (!pfnSipGetInfo(&sipinfo))
    {
        return FALSE;
    }

    rcwnd = sipinfo.rcSipRect;

    int x       = rcwnd.left;
    int y       = rcwnd.top;
    int width   = rcwnd.right - rcwnd.left;
    int height  = rcwnd.bottom - rcwnd.top;

    // Obtain new work area resolution
    RECT rcDisp = sipinfo.rcVisibleDesktop;

    // If the right or bottom edge of the client area is off screen,
    // push it so that edge is on screen
    int dy = rcwnd.bottom - rcDisp.bottom;
    if (dy > 0 && dy < y)
    {
        y    -= dy;
        fMove = TRUE;
    }

    int dx = rcwnd.right - rcDisp.right;
    if (dx > 0 && dx < x)
    {
        x    -= dx;
        fMove = TRUE;
    }

    if (fMove)
    {
        sipinfo.cbSize = sizeof(SIPINFO);
        sipinfo.rcSipRect.left = x;
        sipinfo.rcSipRect.right = x + width;
        sipinfo.rcSipRect.top = y;
        sipinfo.rcSipRect.bottom = y + height;
        return pfnSipSetInfo(&sipinfo);
    }

    return FALSE;
}