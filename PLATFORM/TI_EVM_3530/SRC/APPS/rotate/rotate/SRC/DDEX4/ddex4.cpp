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
// File: DDEx4.CPP
//
// Desc: Direct Draw example program 4.  Adds functionality to 
//       example program 3.  Creates a flipping surface and loads
//       a bitmap image into an offscreen surface.  Uses BltFast to
//       copy portions of the offscreen surface to the back buffer
//       to generate an animation.  Illustrates watching return
//       code from BltFast to prevent image tearing.  This program
//       requires 1.2 Meg of video ram.
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

extern "C" IDirectDrawSurface *
DDLoadBitmapScaled(HINSTANCE hInstance, IDirectDraw * pdd, LPCTSTR szBitmap, 
    DWORD dwWidth, DWORD dwHeight);

//-----------------------------------------------------------------------------
// Local definitions
//-----------------------------------------------------------------------------
#define NAME                TEXT("DDExample4")
#define TITLE               TEXT("Direct Draw Example 4")

//-----------------------------------------------------------------------------
// Default settings
//-----------------------------------------------------------------------------
#define TIMER_ID            1
#define TIMER_RATE          18

//-----------------------------------------------------------------------------
// Global data
//-----------------------------------------------------------------------------
LPDIRECTDRAW                g_pDD = NULL;            // DirectDraw object
LPDIRECTDRAWSURFACE         g_pDDSPrimary = NULL;    // DirectDraw primary surface
LPDIRECTDRAWSURFACE         g_pDDSBack = NULL;       // DirectDraw back surface
LPDIRECTDRAWSURFACE         g_pDDSBackground = NULL; // Offscreen surface 1
LPDIRECTDRAWSURFACE         g_pDDSSprite = NULL;     // Offscreen surface 1
BOOL                        g_bActive = FALSE;       // Is application active?

//-----------------------------------------------------------------------------
// Local data
//-----------------------------------------------------------------------------
static LPCTSTR              szBackground = TEXT("BACKGROUND");
static LPCTSTR              szSprite = TEXT("SPRITE");
static HINSTANCE            hInstance;
static DWORD                g_dwWidth, 
                            g_dwHeight;
static int                  g_xpos[3] = {288, 190, 416};
static int                  g_ypos[3] = {128, 300, 256};


//-----------------------------------------------------------------------------
// Name: ReleaseAllObjects()
// Desc: Finished with all objects we use; release them
//-----------------------------------------------------------------------------
static void
ReleaseAllObjects(void)
{
    if (g_pDDSBack != NULL)
    {
        g_pDDSBack->Release();
        g_pDDSBack = NULL;
    }
    if (g_pDDSPrimary != NULL)
    {
        g_pDDSPrimary->Release();
        g_pDDSPrimary = NULL;
    }
    if (g_pDDSBackground != NULL)
    {
        g_pDDSBackground->Release();
        g_pDDSBackground = NULL;
    }
    if (g_pDDSSprite != NULL)
    {
        g_pDDSSprite->Release();
        g_pDDSSprite = NULL;
    }
    if (g_pDD != NULL)
    {   
        g_pDD->Release();
        g_pDD = NULL;
    }
}




//-----------------------------------------------------------------------------
// Name: InitFail()
// Desc: This function is called if an initialization function fails
//-----------------------------------------------------------------------------
#define PREFIX      TEXT("DDEX4: ")
#define PREFIX_LEN  7

HRESULT
InitFail(HWND hWnd, HRESULT hRet, LPCTSTR szError,...)
{
    TCHAR   szBuff[128] = PREFIX;
    va_list vl;

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
// Name: RestoreAll()
// Desc: Restore all lost objects
//-----------------------------------------------------------------------------
HRESULT 
RestoreAll(void)
{
    HRESULT                     hRet;

    hRet = g_pDDSPrimary->Restore();
    if (SUCCEEDED(hRet))
    {
        if (SUCCEEDED(hRet = g_pDDSBackground->Restore()))
        {
            DDReLoadBitmap(hInstance, g_pDDSBackground, szBackground);
        }
        if (SUCCEEDED(hRet = g_pDDSSprite->Restore()))
        {
            DDReLoadBitmap(hInstance, g_pDDSSprite, szSprite);
        }
    }
    return hRet;
}




//-----------------------------------------------------------------------------
// Name: UpdateFrame()
// Desc: Decide what needs to be blitted next, wait for flip to complete,
//       then flip the buffers.
//-----------------------------------------------------------------------------
void 
UpdateFrame(void)
{
    HRESULT                     hRet;


    static DWORD                lastTickCount[3] =  {0, 0, 0};
    static int                  currentFrame[3] =   {0, 0, 0};
    DWORD                       delay[3] =          {50, 78, 13};
    int                         i;
    DWORD                       thisTickCount;
    RECT                        rcSrcRect;
    RECT                        rcDestRect;

    // Decide which frame will be blitted next
    thisTickCount = GetTickCount();
    for (i = 0; i < 3; i++)
    {
        if ((thisTickCount - lastTickCount[i]) > delay[i])
        {
            // Move to next frame;
            lastTickCount[i] = thisTickCount;
            currentFrame[i]++;
            if (currentFrame[i] > 59)
                currentFrame[i] = 0;
        }
    }

    // Blit the stuff for the next frame

    while (TRUE)
    {
        hRet = g_pDDSBack->Blt(NULL, g_pDDSBackground, NULL, DDBLT_WAITNOTBUSY, NULL);

        if (SUCCEEDED(hRet))
            break;
        if (hRet == DDERR_SURFACELOST)
        {
            if (FAILED(RestoreAll()))
                return;
        }
        if (hRet != DDERR_WASSTILLDRAWING)
            return;
    }
    if (FAILED(hRet))
        return;

    for (i = 0; i < 3; i++)
    {
        rcSrcRect.left = currentFrame[i] % 10 * 64;
        rcSrcRect.top = currentFrame[i] / 10 * 64;
        rcSrcRect.right = currentFrame[i] % 10 * 64 + 64;
        rcSrcRect.bottom = currentFrame[i] / 10 * 64 + 64;

        rcDestRect.left = g_xpos[i];
        rcDestRect.top = g_ypos[i];
        rcDestRect.right = rcDestRect.left + (rcSrcRect.right - rcSrcRect.left);
        rcDestRect.bottom = rcDestRect.top + (rcSrcRect.bottom - rcSrcRect.top);

        while (TRUE)
        {
            hRet = g_pDDSBack->Blt(&rcDestRect, g_pDDSSprite, &rcSrcRect, 
                        DDBLT_WAITNOTBUSY | DDBLT_KEYSRC, NULL);
            if (SUCCEEDED(hRet))
                break;
            if (hRet == DDERR_SURFACELOST)
            {
                if (FAILED(RestoreAll()))
                    return;
            }
            if (hRet != DDERR_WASSTILLDRAWING)
                return;
        }
    }

    // Flip the surfaces
    while (TRUE)
    {
        hRet = g_pDDSPrimary->Flip(NULL, 0);
        if (SUCCEEDED(hRet))
            break;
        if (hRet == DDERR_SURFACELOST)
        {
            if (FAILED(RestoreAll()))
                break;
        }
        if (hRet != DDERR_WASSTILLDRAWING)
            break;
    }
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

        OutputDebugString(L"DDEX4: Enumerated more than surface?");
        pSurface->Release();
        return DDENUMRET_CANCEL;
    }
}


//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The Main Window Procedure
//-----------------------------------------------------------------------------
long FAR PASCAL
WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
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
	        // Shoot another frame.
	        UpdateFrame();
	        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
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
    if (FAILED(hRet = DirectDrawCreate(NULL, &g_pDD, NULL)))
        return InitFail(hWnd, hRet, TEXT("DirectDrawCreate FAILED"));

    // Get exclusive mode
    if (FAILED(hRet = g_pDD->SetCooperativeLevel(hWnd, DDSCL_FULLSCREEN)))
        return InitFail(hWnd, hRet, TEXT("SetCooperativeLevel FAILED"));

    // Create the primary surface with 1 back buffer
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
                          DDSCAPS_FLIP;
    ddsd.dwBackBufferCount = 1;
    if (FAILED(hRet = g_pDD->CreateSurface(&ddsd, &g_pDDSPrimary, NULL)))
    {
        if (hRet == DDERR_NOFLIPHW)
            return InitFail(hWnd, hRet, TEXT("******** Display driver doesn't support flipping surfaces. ********"));
  
        return InitFail(hWnd, hRet, TEXT("CreateSurface FAILED"));
    }

    // Get a pointer to the back buffer
    hRet = g_pDDSPrimary->EnumAttachedSurfaces(&g_pDDSBack, EnumFunction);
    if (hRet != DD_OK)
        return InitFail(hWnd, hRet, TEXT("EnumAttachedSurfaces FAILED"));
    
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
    if (FAILED(hRet = g_pDDSBack->GetSurfaceDesc(&ddsd)))
    {
        return InitFail(hWnd, hRet, TEXT("GetSurfaceDesc FAILED"));
    }
    g_dwWidth = ddsd.dwWidth;
    g_dwHeight = ddsd.dwHeight;

    // Load the background bitmap, scaling to fit dimensions of back buffer
    g_pDDSBackground = DDLoadBitmapScaled(hInstance, g_pDD, szBackground, g_dwWidth, g_dwHeight);
    if (g_pDDSBackground == NULL)
        return InitFail(hWnd, hRet, TEXT("DDLoadBitmap FAILED"));

    // Load the sprite (donut) bitmap
    g_pDDSSprite = DDLoadBitmap(hInstance, g_pDD, szSprite);
    if (g_pDDSSprite == NULL)
        return InitFail(hWnd, hRet, TEXT("DDLoadBitmap FAILED"));

    // Set the color key for this bitmap (black)
    DDSetColorKey(g_pDDSSprite, RGB(0, 0, 0));

    // Create a timer to flip the pages
    if (TIMER_ID != SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL))
        return InitFail(hWnd, hRet, TEXT("SetTimer FAILED"));

    // for Smartphone and PPC, adjust donut positions:
    if (g_dwWidth <= 320 || g_dwHeight <= 240)
    {   
        g_xpos[0] = 0; g_ypos[0] = 0; 
        g_xpos[1] = g_dwWidth-64; g_ypos[1] = g_dwHeight/2-32; 
        g_xpos[2] = g_dwWidth/2-32; g_ypos[2] = g_dwHeight-64; 
    }

    return DD_OK;
}

//-----------------------------------------------------------------------------
// Name: DDLoadBitmapScaled()
// Desc: Create a DirectDrawSurface from a bitmap resource, scaling the bitmap
//       to the specified width and height.
//-----------------------------------------------------------------------------
extern "C" IDirectDrawSurface *
DDLoadBitmapScaled(HINSTANCE hInstance, IDirectDraw * pdd, LPCTSTR szBitmap, 
    DWORD dwWidth, DWORD dwHeight)
{
    HRESULT hr;
    HBITMAP hbm;
    DDSURFACEDESC ddsd;
    IDirectDrawSurface * pdds;

    //
    // Get a handle to the bitmap.
    //
    hbm = DDGetBitmapHandle(hInstance,szBitmap);
    if (hbm == NULL) {
      OutputDebugString(TEXT("DDUTIL: Unable to obtain handle to bitmap.\n"));
      return NULL;
    }

    //
    // Create a DirectDrawSurface for this bitmap
    //
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.dwWidth = dwWidth;
    ddsd.dwHeight = dwHeight;
    if (FAILED(pdd->CreateSurface(&ddsd, &pdds, NULL)))
        return NULL;

    hr = DDCopyBitmap(pdds, hbm, 0, 0, 0, 0);
    if (FAILED(hr)) {
      OutputDebugString(TEXT("DDUTIL: Unable to copy bitmap bits.\n"));
      pdds->Release();
      pdds = NULL;
    }

    DeleteObject(hbm);
    return pdds;
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
    MSG                         msg;

    hInstance = hCurInstance;

    if (FAILED(InitApp(nCmdShow)))
        return FALSE;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

