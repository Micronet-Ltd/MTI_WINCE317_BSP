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
//-----------------------------------------------------------------------------
// File: DDEx2.CPP
//
// Desc: Direct Draw example program 2.  Adds functionality to 
//       example program 1.
//
//       Reads a bitmap file from disk and copies it into the 
//       back buffer and then slowly flips between the primary
//       surface and the back buffer.  Press F12 to exit the program.
//
//-----------------------------------------------------------------------------

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------
#include <windows.h>
#include <ddraw.h>
#include "resource.h"
#include "ddutil.h"

//-----------------------------------------------------------------------------
// Local definitions
//-----------------------------------------------------------------------------
#define NAME                TEXT("DDExample2")
#define TITLE               TEXT("Direct Draw Example 2")

//-----------------------------------------------------------------------------
// Default settings
//-----------------------------------------------------------------------------
#define TIMER_ID            1
#define TIMER_RATE          500

//-----------------------------------------------------------------------------
// Global data
//-----------------------------------------------------------------------------
LPDIRECTDRAW                g_pDD = NULL;        // DirectDraw object
LPDIRECTDRAWSURFACE         g_pDDSPrimary = NULL;// DirectDraw primary surface
LPDIRECTDRAWSURFACE         g_pDDSBack = NULL;   // DirectDraw back surface
BOOL                        g_bActive = FALSE;   // Is application active?

//-----------------------------------------------------------------------------
// Local data
//-----------------------------------------------------------------------------
static TCHAR                szBackground[] = TEXT("BACK");
static TCHAR                szMsg[] = TEXT("Page Flipping Test: Press F12 to exit");
static TCHAR                szFrontMsg[] = TEXT("Front buffer (F12 to quit)");
static TCHAR                szBackMsg[] = TEXT("Back buffer (F12 to quit)");
static HINSTANCE            hInstance;



//-----------------------------------------------------------------------------
// Name: ReleaseAllObjects()
// Desc: Finished with all objects we use; release them
//-----------------------------------------------------------------------------
static void
ReleaseAllObjects(void)
{
    if (g_pDDSBack != NULL) {
        g_pDDSBack->Release();
        g_pDDSBack = NULL;
    }

    if (g_pDDSPrimary != NULL) {
        g_pDDSPrimary->Release();
        g_pDDSPrimary = NULL;
    }

    if (g_pDD != NULL) {
        g_pDD->Release();
        g_pDD = NULL;
    }
}




//-----------------------------------------------------------------------------
// Name: InitFail()
// Desc: This function is called if an initialization function fails
//-----------------------------------------------------------------------------
#define PREFIX      TEXT("DDEX2: ")
#define PREFIX_LEN  7

HRESULT
InitFail(HWND hWnd, HRESULT hRet, LPCTSTR szError,...)
{
    TCHAR                       szBuff[128] = PREFIX;
    va_list                     vl;

    va_start(vl, szError);
    StringCchVPrintf(szBuff + PREFIX_LEN, (128-PREFIX_LEN), szError, vl);
    ReleaseAllObjects();
    OutputDebugString(szBuff);
    DestroyWindow(hWnd);
    va_end(vl);
    return hRet;
}

#undef PREFIX_LEN
#undef PREFIX


//-----------------------------------------------------------------------------
// Name: UpdateFrame()
// Desc: Displays the proper text for the page
//-----------------------------------------------------------------------------
static void
UpdateFrame(HWND hWnd)
{
    static BYTE phase = 0;
    HDC hdc;
    RECT rc;
    SIZE size;
    int nMsg;

    // The back buffer already has a loaded bitmap, so don't clear it

    if (g_pDDSBack->GetDC(&hdc) == DD_OK)
    {
        SetBkColor(hdc, RGB(0, 0, 255));
        SetTextColor(hdc, RGB(255, 255, 0));
        GetClientRect(hWnd, &rc);
        if (phase)
        {
	        nMsg = lstrlen(szMsg);
            GetTextExtentPoint(hdc, szMsg, nMsg, &size);
            ExtTextOut(hdc, 
		       (rc.right - size.cx) / 2, 
		       (rc.bottom - size.cy) / 2,
		       0,                        // fuOptions
		       NULL,                     // lprc
               szMsg, 
		       nMsg,
		       NULL);                    // lpDx

            nMsg = lstrlen(szFrontMsg);
            GetTextExtentPoint(hdc, szFrontMsg, nMsg, &size);
            ExtTextOut(hdc, 
		       (rc.right - size.cx) / 2, // Center horz. for tv reasons
		       0, 
		       0,                        // fuOptions
		       NULL,                     // lprc
		       szFrontMsg, 
		       nMsg,
		       NULL);                    // lpDx
            phase = 0;
        }
        else
        {
            nMsg = lstrlen(szBackMsg);
            GetTextExtentPoint(hdc, szBackMsg, nMsg, &size);
            ExtTextOut(hdc, 
		       (rc.right - size.cx) / 2, // Center horz. for tv reasons 
		       0,  
		       0,                        // fuOptions
		       NULL,                     // lprc
		       szBackMsg, 
		       nMsg,
		       NULL);                    // lpDx
            phase = 1;
        }
        g_pDDSBack->ReleaseDC(hdc);
    }
}




//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The Main Window Procedure
//-----------------------------------------------------------------------------
long FAR PASCAL
WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HRESULT                     hRet;

    switch (message)
    {
#ifdef UNDER_CE
        case WM_ACTIVATE:
#else
        case WM_ACTIVATEAPP:
#endif
            // Pause if minimized or not the top window
            g_bActive = (wParam == WA_ACTIVE) || (wParam == WA_CLICKACTIVE);
            return 0L;

        case WM_DESTROY:
            // Clean up and close the app
            ReleaseAllObjects();
            PostQuitMessage(0);
            return 0L;

        case WM_KEYDOWN:
            // Handle any non-accelerated key commands
            switch (wParam)
            {
                case VK_ESCAPE:
                case VK_F12:
                    PostMessage(hWnd, WM_CLOSE, 0, 0);
                    return 0L;
            }
            break;

        case WM_SETCURSOR:
            // Turn off the cursor since this is a full-screen app
            SetCursor(NULL);
            return TRUE;

        case WM_TIMER:
            // Update and flip surfaces
            if (g_bActive && TIMER_ID == wParam)
            {
                UpdateFrame(hWnd);
                while (TRUE)
                {
                    hRet = g_pDDSPrimary->Flip(NULL, 0);
                    if (hRet == DD_OK)
                        break;
                    if (hRet == DDERR_SURFACELOST)
                    {
                        hRet = g_pDDSPrimary->Restore();
                        if (hRet != DD_OK)
                            break;
                        hRet = DDReLoadBitmap(hInstance, g_pDDSBack, szBackground);
                        if (hRet != DD_OK)
                            break;
                    }
                    if (hRet != DDERR_WASSTILLDRAWING)
                        break;
                }
            }
            break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}


//-----------------------------------------------------------------------------
// Name: EnumFunction()
// Desc: Enumeration callback for surfaces in flipping chain. We expect this
//          function to be called once with the surface interface pointer of
//          our back buffer (we only ask for a single back buffer.)
//-----------------------------------------------------------------------------
static HRESULT PASCAL
EnumFunction(LPDIRECTDRAWSURFACE pSurface,
             LPDDSURFACEDESC lpSurfaceDesc,
             LPVOID  lpContext)
{
    static BOOL bCalled = FALSE;

    if (!bCalled) {

        *((LPDIRECTDRAWSURFACE *)lpContext) = pSurface;
        bCalled = TRUE;
        return DDENUMRET_OK;
    }
    else {

        OutputDebugString(L"DDEX2: Enumerated more than surface?");
        pSurface->Release();
        return DDENUMRET_CANCEL;
    }
}


//-----------------------------------------------------------------------------
// Name: InitApp()
// Desc: Do work required for every instance of the application:
//          Create the window, initialize data
//-----------------------------------------------------------------------------
static HRESULT
InitApp(int nCmdShow)
{
    HWND                        hWnd;
    WNDCLASS                    wc;
    DDSURFACEDESC               ddsd;
    HRESULT                     hRet;

    // Set up and register window class
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH )GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = NAME;
    RegisterClass(&wc);

    // Create a window
    hWnd = CreateWindowEx(WS_EX_TOPMOST,
                          NAME,
                          TITLE,
                          WS_POPUP,
                          0,
                          0,
                          GetSystemMetrics(SM_CXSCREEN),
                          GetSystemMetrics(SM_CYSCREEN),
                          NULL,
                          NULL,
                          hInstance,
                          NULL);
    if (!hWnd)
        return FALSE;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    SetFocus(hWnd);

    ///////////////////////////////////////////////////////////////////////////
    // Create the main DirectDraw object
    ///////////////////////////////////////////////////////////////////////////
    hRet = DirectDrawCreate(NULL, &g_pDD, NULL);
    if (hRet != DD_OK)
        return InitFail(hWnd, hRet, TEXT("DirectDrawCreate FAILED"));

    // Get exclusive mode
    hRet = g_pDD->SetCooperativeLevel(hWnd, DDSCL_FULLSCREEN);
    if (hRet != DD_OK)
        return InitFail(hWnd, hRet, TEXT("SetCooperativeLevel FAILED"));

    // Create the primary surface with 1 back buffer
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
                          DDSCAPS_FLIP;
    ddsd.dwBackBufferCount = 1;
    hRet = g_pDD->CreateSurface(&ddsd, &g_pDDSPrimary, NULL);
    if (hRet != DD_OK)
    {
        if (hRet == DDERR_NOFLIPHW)
            return InitFail(hWnd, hRet, TEXT("******** Display driver doesn't support flipping surfaces. ********"));
  
        return InitFail(hWnd, hRet, TEXT("CreateSurface FAILED"));
    }

    // Get a pointer to the back buffer
    hRet = g_pDDSPrimary->EnumAttachedSurfaces(&g_pDDSBack, EnumFunction);
    if (hRet != DD_OK)
        return InitFail(hWnd, hRet, TEXT("EnumAttachedSurfaces FAILED"));

    // Load a bitmap into the back buffer. This will StretchBlt
    // the bitmap in, so we don't have to worry about screen
    // resolution (other than the bitmap looking whack.)
    hRet = DDReLoadBitmap(hInstance, g_pDDSBack, szBackground);
    if (hRet != DD_OK)
        return InitFail(hWnd, hRet, TEXT("DDReLoadBitmap FAILED"));

    // Create a timer to flip the pages
    if (TIMER_ID != SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL))
        return InitFail(hWnd, hRet, TEXT("SetTimer FAILED"));

    return DD_OK;
}




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Initialization, message loop
//-----------------------------------------------------------------------------
int PASCAL
WinMain(HINSTANCE hCurInstance,
        HINSTANCE hPrevInstance,
#ifdef UNDER_CE
        LPWSTR lpCmdLine,
#else
        LPSTR lpCmdLine,
#endif
        int nCmdShow)
{
    MSG msg;

    hInstance = hCurInstance;

    if (InitApp(nCmdShow) != DD_OK)
        return FALSE;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
