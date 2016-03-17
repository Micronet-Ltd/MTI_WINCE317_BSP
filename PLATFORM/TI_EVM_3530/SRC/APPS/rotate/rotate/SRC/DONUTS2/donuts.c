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
/*==========================================================================
 *
 *
 *  File:       donuts.c
 *  Content:    Shoot-em-up game
 *
 *
 ***************************************************************************/
#define INITGUID
#include "donuts.h"
#include <strsafe.h>


LPDIRECTDRAW            lpDD = NULL;
LPDIRECTDRAWSURFACE     lpFrontBuffer = NULL;
LPDIRECTDRAWSURFACE     lpBackBuffer = NULL;
LPDIRECTDRAWSURFACE     lpDonut = NULL;
LPDIRECTDRAWSURFACE     lpPyramid = NULL;
LPDIRECTDRAWSURFACE     lpCube = NULL;
LPDIRECTDRAWSURFACE     lpSphere = NULL;
LPDIRECTDRAWSURFACE     lpShip = NULL;
LPDIRECTDRAWSURFACE     lpNum = NULL;
LPDIRECTDRAWPALETTE     lpArtPalette = NULL;
LPDIRECTDRAWPALETTE     lpSplashPalette = NULL;
BOOL                    bSoundEnabled = FALSE;
BOOL                    bPlayIdle = FALSE;
BOOL                    bPlayBuzz = FALSE;
BOOL                    bPlayRev = FALSE;
BOOL                    lastThrust = FALSE;
BOOL                    lastShield = FALSE;
int                     showDelay = 0;
HWND                    hWndMain;
HACCEL                  hAccel;
HINSTANCE               hInst;
BOOL                    bShowFrameCount=TRUE;
BOOL                    bIsActive;
BOOL                    bMouseVisible;
DWORD                   dwFrameCount;
DWORD                   dwFrameTime;
DWORD                   dwFrames;
DWORD                   dwFramesLast;
BOOL                    bTest=FALSE;
BOOL                    bStress=FALSE;
DWORD                   dwTransType;
RGBQUAD                 SPalette[256];
DWORD                   lastTickCount;
int                     score;
int                     ProgramState;
int                     level;
int                     restCount;
DWORD                   dwFillColor;
BOOL                    bSpecialEffects = FALSE;
int                     iForceErase = 0;
DWORD                   ShowLevelCount = 3000;
DWORD                   ScreenX;
DWORD                   ScreenY;
DWORD                   ScreenBpp;
BOOL                    bWantSound = TRUE;  //global flag to turn off sound

int getint(char**p, int def);

LPCTSTR                 cszAppName = TEXT("DONUTS");
TCHAR                   DebugBuf[256];

DBLNODE                 DL;             // Display List

HSNDDEVICE              hSoundDevice;
HSNDOBJ                 hsoBeginLevel     = NULL;
HSNDOBJ                 hsoEngineIdle     = NULL;
HSNDOBJ                 hsoEngineRev      = NULL;
HSNDOBJ                 hsoSkidToStop     = NULL;
HSNDOBJ                 hsoShieldBuzz     = NULL;
HSNDOBJ                 hsoShipExplode    = NULL;
HSNDOBJ                 hsoFireBullet     = NULL;
HSNDOBJ                 hsoShipBounce     = NULL;
HSNDOBJ                 hsoDonutExplode   = NULL;
HSNDOBJ                 hsoPyramidExplode = NULL;
HSNDOBJ                 hsoCubeExplode    = NULL;
HSNDOBJ                 hsoSphereExplode  = NULL;

// timeGetTime currently not supported on WinCE.
#define timeGetTime GetTickCount

// DirectInput work-around.
DWORD GlobalInput = 0;


void setup_game(void)
{
    restCount = GetTickCount();
    initLevel( ++level );
    // set the palette
    if (NULL != lpArtPalette)
        lpFrontBuffer->lpVtbl->SetPalette( lpFrontBuffer, lpArtPalette );
}

/*
 *
 *  AppPause
 *
 */
void AppPause(void)
{
    return;
}

/*
 *
 *  AppUnpause
 *
 *  Reset the various time counters so the donuts don't suddenly
 *  jump halfways across the screen and so the frame rate remains accurate.
 *
 */
void AppUnpause(void)
{
    iForceErase = 2;
    lastTickCount = dwFrameTime = timeGetTime();
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
        pSurface->lpVtbl->Release( pSurface );
        return DDENUMRET_CANCEL;
    }
}


/*
 * MainWndproc
 *
 * Callback for all Windows messages
 */
long FAR PASCAL MainWndproc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    UINT        cmd;

    switch( message )
    {
    case WM_ACTIVATE:
        bIsActive = (BOOL) wParam;
        if( bIsActive )
        {
            bMouseVisible = FALSE;
            lastTickCount = GetTickCount();
            bSpecialEffects = FALSE;
        }
        else
        {
            bMouseVisible = TRUE;
       // DirectInput automatically unacquires for us in FOREGROUND mode
        }
        break;

    case WM_SETCURSOR:
        // Turn off the cursor since this is a full-screen app
        SetCursor(NULL);
        return TRUE;

    case WM_COMMAND:
        cmd = GET_WM_COMMAND_ID(wParam, lParam);

          switch (cmd) {

        case IDC_FRAMERATE:
            bShowFrameCount = !bShowFrameCount;
            if( bShowFrameCount )
            {
                dwFrameCount = 0;
                dwFrameTime = timeGetTime();
            }
            break;

        case IDC_STARTGAME:
            if( ProgramState == PS_SPLASH )
            {
                ProgramState = PS_BEGINREST;
                setup_game();
            }
            break;

        case IDC_QUIT:
            PostMessage( hWnd, WM_CLOSE, 0, 0 );
            return 0;

        case IDC_AUDIO:
            if(bWantSound)
            {
                if( bSoundEnabled )
                {
                    DestroySound();
                }
                else
                {
                    InitializeSound();
                }
            }
            break;

        case IDC_TRAILS:
            bSpecialEffects = !bSpecialEffects;
            break;
        }
        break;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
        BeginPaint( hWnd, &ps );
        EndPaint( hWnd, &ps );
        return 0;

    case WM_CLOSE:
        DestroyGame();
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage( 0 );
        return 0;

      // Work-Around the fact that we don't have D-Input.

    case WM_KEYDOWN:
      switch (wParam) {
      case VK_NUMPAD8:
      case VK_UP:
        GlobalInput |= KEY_UP;
        break;
      case VK_NUMPAD2:
      case VK_DOWN:
        GlobalInput |= KEY_DOWN;
        break;
      case VK_NUMPAD4:
      case VK_LEFT:
        GlobalInput |= KEY_LEFT;
        break;
      case VK_NUMPAD6:
      case VK_RIGHT:
        GlobalInput |= KEY_RIGHT;
        break;
      case VK_SPACE:
        GlobalInput |= KEY_FIRE;
        break;
      case VK_NUMPAD5:
      case VK_CLEAR:
        GlobalInput |= KEY_STOP;
        break;
      case VK_NUMPAD7:
      case VK_HOME:
        GlobalInput |= KEY_SHIELD;
        break;
      }
      break;

    case WM_KEYUP:
      switch (wParam) {
      case VK_NUMPAD8:
      case VK_UP:
        GlobalInput &= ~KEY_UP;
        break;
      case VK_NUMPAD2:
      case VK_DOWN:
        GlobalInput &= ~KEY_DOWN;
        break;
      case VK_NUMPAD4:
      case VK_LEFT:
        GlobalInput &= ~KEY_LEFT;
        break;
      case VK_NUMPAD6:
      case VK_RIGHT:
        GlobalInput &= ~KEY_RIGHT;
        break;
      case VK_SPACE:
        GlobalInput &= ~KEY_FIRE;
        break;
      case VK_NUMPAD5:
      case VK_CLEAR:
        GlobalInput &= ~KEY_STOP;
        break;
      case VK_NUMPAD7:
      case VK_HOME:
        GlobalInput &= ~KEY_SHIELD;
        break;
      }
      break; 

    default:
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);

} /* MainWndproc */

/*
 * initApplication
 *
 * Do that Windows initialization stuff...
 */
static BOOL initApplication( HANDLE hInstance, int nCmdShow )
{
    WNDCLASS    wc;
    BOOL        rc;

    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = MainWndproc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE(DONUTS_ICON));
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = GetStockObject( BLACK_BRUSH );
    wc.lpszMenuName = NULL;
    wc.lpszClassName = TEXT("DonutsClass");
    rc = RegisterClass( &wc );
    if( !rc )
    {
        return FALSE;
    }

    hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(DONUTS_ACCEL));
    if ( !hAccel )
    {
        return FALSE;
    }


    hWndMain = CreateWindowEx(WS_EX_TOPMOST,
        wc.lpszClassName,
        TEXT("Donuts"),
        WS_VISIBLE | // so we don't have to call ShowWindow
        WS_POPUP,   // non-app window
        0,
        0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        NULL,
        NULL,
        hInstance,
        NULL );

    if( !hWndMain )
    {
        return FALSE;
    }

    SetFocus(hWndMain);

    return TRUE;

} /* initApplication */

/*
 * WinMain
 */
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine,
                        int nCmdShow )
{
    MSG     msg;
    int     retcode = 0;

    // save off application instance
    hInst = hInstance;

    while( lpCmdLine[0] == '-' )
    {
        lpCmdLine++;

        switch (*lpCmdLine++)
        {
        case 't':
            bTest = TRUE;
            break;
        case 'S':
            bWantSound = FALSE;
            break;
        case 'x':
            bStress= TRUE;
            bTest = TRUE;
            break;
        }
        while( IS_SPACE(*lpCmdLine) )
        {
            lpCmdLine++;
        }
    }

    if( !initApplication(hInstance, nCmdShow) )
    {
        return FALSE;
    }

    if( !InitializeGame() )
    {
        DestroyWindow( hWndMain );
        return FALSE;
    }

    dwFrameTime = timeGetTime();

    while( 1 )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {

            if (msg.message == WM_QUIT)
            {
                retcode = (int)msg.wParam;
                break;
            }
            if ( !TranslateAccelerator( hWndMain, hAccel, &msg ) ) 
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else if ( bIsActive )
        {
            UpdateFrame();
        }
    }

    return retcode;
} /* WinMain */


void DestroyGame( void )
{
  // Undo what has been done...

    DestroySound();

    if( lpDonut != NULL )
        lpDonut->lpVtbl->Release( lpDonut );

    if( lpPyramid != NULL )
        lpPyramid->lpVtbl->Release( lpPyramid );

    if( lpCube != NULL )
        lpCube->lpVtbl->Release( lpCube );

    if( lpSphere != NULL )
        lpSphere->lpVtbl->Release( lpSphere );

    if( lpShip != NULL )
        lpShip->lpVtbl->Release( lpShip );

    if( lpNum != NULL )
        lpNum->lpVtbl->Release( lpNum );

    if( lpBackBuffer != NULL )
        lpBackBuffer->lpVtbl->Release( lpBackBuffer );

    if( lpFrontBuffer != NULL )
        lpFrontBuffer->lpVtbl->Release( lpFrontBuffer );

    if( lpArtPalette != NULL )
        lpArtPalette->lpVtbl->Release( lpArtPalette );

    if( lpSplashPalette != NULL )
        lpSplashPalette->lpVtbl->Release( lpSplashPalette );

    if( lpDD != NULL )
        lpDD->lpVtbl->Release( lpDD );
    
    if (DL.next != NULL)
    {
        while( DL.next != &DL )
        {
            DeleteFromList( DL.next );
        }
    }
}

BOOL InitializeGame( void )
{
    DDCAPS          ddcaps;
    HRESULT         ddrval;
    DDSURFACEDESC   ddsd;
    DDSURFACEDESC   DDSurfDesc;

    if (bWantSound)
    {
        if (! InitializeSound()) {
            return FALSE;
        }
    }

    score = 0;
    if( bTest )
        ShowLevelCount = 1000;

    // Initialize the display list to point at itself.  This denotes an empty
    // list.  We need to do this here because if CleanupAndExit/DestroyGame
    // will attempt to walk the display list and delete records.
    DL.next = DL.prev = &DL;            

    ddrval = DirectDrawCreate( NULL, &lpDD, NULL );

    if( ddrval != DD_OK )
        return CleanupAndExit(TEXT("DirectDrawCreate Failed!"));
   
    ddrval = lpDD->lpVtbl->SetCooperativeLevel( lpDD, hWndMain, DDSCL_FULLSCREEN );
    if( ddrval != DD_OK )
        return CleanupAndExit(TEXT("SetCooperativeLevel Failed"));

    // Get mode info.

    memset(&DDSurfDesc, 0, sizeof(DDSurfDesc));
    DDSurfDesc.dwSize = sizeof(DDSurfDesc);

    ddrval = lpDD->lpVtbl->GetDisplayMode( lpDD, &DDSurfDesc );
    if ( ddrval != DD_OK )
        return CleanupAndExit(TEXT("GetDisplayMode Failed!"));

    ScreenX = DDSurfDesc.dwWidth;
    ScreenY = DDSurfDesc.dwHeight;
    ScreenBpp = DDSurfDesc.ddpfPixelFormat.dwRGBBitCount;

    // check the color key hardware capabilities
    dwTransType = DDBLT_KEYSRC;
    ddcaps.dwSize = sizeof( ddcaps );

    // Create surfaces
    memset( &ddsd, 0, sizeof( ddsd ) );
    ddsd.dwSize = sizeof( ddsd );
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
                          DDSCAPS_FLIP;
    ddsd.dwBackBufferCount = 1;
    ddrval = lpDD->lpVtbl->CreateSurface( lpDD, &ddsd, &lpFrontBuffer, NULL );
    if (ddrval != DD_OK)
    {
        if (ddrval == DDERR_NOFLIPHW)
            return CleanupAndExit(TEXT("******** Display driver doesn't support flipping surfaces. ********"));
  
        return CleanupAndExit(TEXT("CreateSurface FrontBuffer Failed!"));
    }

    // Get a pointer to the back buffer
    ddrval = lpFrontBuffer->lpVtbl->EnumAttachedSurfaces( lpFrontBuffer, &lpBackBuffer, EnumFunction );
    if (ddrval != DD_OK)
        return CleanupAndExit(TEXT("EnumAttachedSurfaces FAILED"));

    ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
    ddsd.dwWidth = 320;
    ddsd.dwHeight = 384;
    ddrval = lpDD->lpVtbl->CreateSurface( lpDD, &ddsd, &lpDonut, NULL );
    if( ddrval != DD_OK )
        return CleanupAndExit(TEXT("CreateSurface lpDonut Failed!"));

    ddsd.dwHeight = 128;
    ddrval = lpDD->lpVtbl->CreateSurface( lpDD, &ddsd, &lpPyramid, NULL );
    if( ddrval != DD_OK )
        return CleanupAndExit(TEXT("CreateSurface lpPyramid Failed!"));

    ddsd.dwHeight = 32;
    ddrval = lpDD->lpVtbl->CreateSurface( lpDD, &ddsd, &lpCube, NULL );
    if( ddrval != DD_OK )
        return CleanupAndExit(TEXT("CreateSurface lpCube Failed!"));

    ddsd.dwHeight = 32;
    ddrval = lpDD->lpVtbl->CreateSurface( lpDD, &ddsd, &lpSphere, NULL );
    if( ddrval != DD_OK )
        return CleanupAndExit(TEXT("CreateSurface lpSphere Failed!"));
    // Set the background color fill color

    ddsd.dwHeight = 256;
    ddrval = lpDD->lpVtbl->CreateSurface( lpDD, &ddsd, &lpShip, NULL );
    if( ddrval != DD_OK )
        return CleanupAndExit(TEXT("CreateSurface lpShip Failed!"));

    ddsd.dwHeight = 16;
    ddrval = lpDD->lpVtbl->CreateSurface( lpDD, &ddsd, &lpNum, NULL );
    if( ddrval != DD_OK )
        return CleanupAndExit(TEXT("CreateSurface lpNum Failed!"));

    if( !RestoreSurfaces() )
        return CleanupAndExit(TEXT("RestoreSurfaces Failed!"));

    DL.type = OBJ_SHIP;
    DL.surf = lpShip;
    lastTickCount = GetTickCount();

    if(bTest)
    {
        ProgramState = PS_ACTIVE;
        setup_game();
    }
    else
    {
        ProgramState = PS_SPLASH;
    }

    return TRUE;
}

BOOL CleanupAndExit( LPCTSTR err)
{
// Note: We're using OutputDebugString here instead of RETAILMSG to allow the user
// to read the error message, even when the sample app is running on a retail
// build of the OS. Not seeing a message in the debugger when the app mysteriously
// fails (as in the case when there is no hardware flipping support for example)
// has led to a lot of confusion.
    
    HRESULT hr = StringCchPrintf (
                    DebugBuf, 
                    sizeof(DebugBuf)/sizeof(DebugBuf[0]), 
                    TEXT("%s: CleanupAndExit  err = %s\n"), 
                    cszAppName, 
                    err);
    if (SUCCEEDED(hr))
        OutputDebugString(DebugBuf);
    else
        OutputDebugString(TEXT("DONUTS: Internal application error\n"));    

    DestroyGame();

    return FALSE;
}

void bltSplash( void )
{
    HRESULT     ddrval;
    HBITMAP     hbm;

    hbm = (HBITMAP)LoadImage( hInst, TEXT("SPLASH"), IMAGE_BITMAP, 0, 0, 0 );
    if ( NULL == hbm )
        return;

    // if the surface is lost, DDCopyBitmap will fail and the surface will
    // be restored in FlipScreen.
    ddrval = DDCopyBitmap( lpBackBuffer, hbm, 0, 0, 0, 0 );

    DeleteObject( hbm );

    FlipScreen();
}

//
// play a sound, but first set the panning according to where the
// object is on the screen.  fake 3D sound.
//
void playPanned(HSNDOBJ hSO, DBLNODE *object)
{
    SndObjPlayPanned(hSO, (LONG)((20000.0 * ((object->dst.right + object->dst.left) / 2) / ScreenX) - 10000.0));
}

void UpdateFrame( void )
{
    switch( ProgramState )
    {
        case PS_SPLASH:
            // display the splash screen
            bltSplash();
            return;
        case PS_ACTIVE:
            UpdateDisplayList();
            CheckForHits();
            DrawDisplayList();
            if ( isDisplayListEmpty() )
            {
                if(bWantSound)
                {
                    SndObjStop(hsoEngineIdle);
                    SndObjStop(hsoEngineRev);
                }
                bPlayIdle = FALSE;
                bPlayRev = FALSE;
                lastThrust = lastShield = FALSE;
                ProgramState = PS_BEGINREST;
                restCount = GetTickCount();
                initLevel( ++level );
            }
            return;
        case PS_BEGINREST:
            if(bWantSound)
            {
                SndObjPlay(hsoBeginLevel);
            }
            ProgramState = PS_REST;
            //
            // FALLTHRU
            //
            __fallthrough;
        case PS_REST:
            if( ( GetTickCount() - restCount ) > ShowLevelCount )
            {
                bPlayIdle = TRUE;
                lastTickCount = GetTickCount();
                ProgramState = PS_ACTIVE;
            }
            else
            {
                DisplayLevel();
            }
            return;
    }
}

void DisplayLevel( void )
{
    TCHAR buf[10];

    EraseScreen();
    buf[0] = 10 + TEXT('0');
    buf[1] = 11 + TEXT('0');
    buf[2] = 12 + TEXT('0');
    buf[3] = 11 + TEXT('0');
    buf[4] = 10 + TEXT('0');
    buf[5] = TEXT('\0');
    bltScore( buf, ScreenX/2-64, ScreenY/2-8 );
    buf[0] = level / 100 + TEXT('0');
    buf[1] = level / 10 + TEXT('0');
    buf[2] = level % 10 + TEXT('0');
    buf[3] = TEXT('\0');
    bltScore( buf, ScreenX/2+22, ScreenY/2-8 );
    FlipScreen();
}

void bltScore( LPCTSTR num, int x, int y )
{
    LPCTSTR c;
    RECT    src;
    RECT    dest;
    int     i;
    HRESULT ddrval;

    for(c=num; *c != TEXT('\0'); c++)
    {
        while( 1 )
        {
            i = *c - TEXT('0');

            src.left = i*16;
            src.top = 0;
            src.right = src.left + 16;
            src.bottom = src.top + 16;

            dest.left = x;
            dest.top  = y;
            dest.right = dest.left + (src.right - src.left);
            dest.bottom = dest.top + (src.bottom - src.top);

            ddrval = lpBackBuffer->lpVtbl->Blt( lpBackBuffer, &dest, lpNum, &src, dwTransType, NULL );
            if( ddrval == DD_OK )
            {
                break;
            }
            if( ddrval == DDERR_SURFACELOST )
            {
                if( !RestoreSurfaces() )
                    return;
            }
            if( ddrval != DDERR_WASSTILLDRAWING )
            {
                return;
            }
        }
        x += 16;
    }
}

void CheckForHits( void )
{
    LPDBLNODE   bullet, target, save;
    int         frame, x, y, l, t;
    BOOL        hit;

    // update screen rects
    target = &DL;
    do
    {
        frame = (DWORD)target->frame;
        switch( target->type )
        {
            case OBJ_DONUT:
                target->dst.left = (DWORD)target->posx;
                target->dst.top = (DWORD)target->posy;
                target->dst.right = target->dst.left + 64;
                target->dst.bottom = target->dst.top + 64;
                target->src.left = 64 * (frame % 5);
                target->src.top = 64 * (frame /5);
                target->src.right = target->src.left + 64;
                target->src.bottom = target->src.top + 64;
                break;
            case OBJ_PYRAMID:
                target->dst.left = (DWORD)target->posx;
                target->dst.top = (DWORD)target->posy;
                target->dst.right = target->dst.left + 32;
                target->dst.bottom = target->dst.top + 32;
                target->src.left = 32 * (frame % 10);
                target->src.top = 32 * (frame /10);
                target->src.right = target->src.left + 32;
                target->src.bottom = target->src.top + 32;
                break;
            case OBJ_SPHERE:
                target->dst.left = (DWORD)target->posx;
                target->dst.top = (DWORD)target->posy;
                target->dst.right = target->dst.left + 16;
                target->dst.bottom = target->dst.top + 16;
                target->src.left = 16 * (frame % 20);
                target->src.top = 16 * (frame /20);
                target->src.right = target->src.left + 16;
                target->src.bottom = target->src.top + 16;
                break;
            case OBJ_CUBE:
                target->dst.left = (DWORD)target->posx;
                target->dst.top = (DWORD)target->posy;
                target->dst.right = target->dst.left + 16;
                target->dst.bottom = target->dst.top + 16;
                target->src.left = 16 * (frame % 20);
                target->src.top = 16 * (frame /20);
                target->src.right = target->src.left + 16;
                target->src.bottom = target->src.top + 16;
                break;
            case OBJ_SHIP:
                target->dst.left = (DWORD)target->posx;
                target->dst.top = (DWORD)target->posy;
                target->dst.right = target->dst.left + 32;
                target->dst.bottom = target->dst.top + 32;
                if( lastShield )
                    target->src.top = 32 * (frame / 10) + 128;
                else
                    target->src.top = 32 * (frame /10);
                target->src.left = 32 * (frame % 10);
                target->src.right = target->src.left + 32;
                target->src.bottom = target->src.top + 32;
                break;
            case OBJ_BULLET:
                frame = (DWORD)target->frame/20 % 4;
                target->dst.left = (DWORD)target->posx;
                target->dst.top = (DWORD)target->posy;
                target->dst.right = target->dst.left + 3;
                target->dst.bottom = target->dst.top + 3;
                target->src.left = BULLET_X + frame*4;
                target->src.top = BULLET_Y;
                target->src.right = target->src.left + 3;
                target->src.bottom = target->src.top + 3;
                break;
        }
        target = target->next;
    }
    while( target != &DL );

    bullet=&DL;
    do
    {
        hit = FALSE;
        if((bullet->type != OBJ_BULLET) && (bullet != &DL))
        {
            bullet = bullet->next;
            continue;
        }

        x = (bullet->dst.left + bullet->dst.right) / 2;
        y = (bullet->dst.top + bullet->dst.bottom) / 2;
        for(target=DL.next; target != &DL; target = target->next)
        {
            if( ( target->type != OBJ_DONUT ) &&
                ( target->type != OBJ_PYRAMID ) &&
                ( target->type != OBJ_SPHERE ) &&
                ( target->type != OBJ_CUBE ) )
                continue;

            if( (x >= target->dst.left) &&
                (x <  target->dst.right) &&
                (y >= target->dst.top) &&
                (y <  target->dst.bottom) )
            {
                if ((bullet != &DL) || !lastShield)
                {
                    // the bullet hit the target
                    switch( target->type )
                    {
                    case OBJ_DONUT:
                        if(bWantSound)
                        {
                            playPanned(hsoDonutExplode, target);
                        }
                        addObject( OBJ_PYRAMID, target->dst.left,
                            target->dst.top, -1.0, -1.0 );
                        addObject( OBJ_PYRAMID, target->dst.left,
                            target->dst.top, -1.0, -1.0 );
                        addObject( OBJ_PYRAMID, target->dst.left,
                            target->dst.top, -1.0, -1.0 );
                        score += 10;
                        break;
                    case OBJ_PYRAMID:
                        if(bWantSound)
                        {
                            playPanned(hsoPyramidExplode, target);
                        }
                        addObject( OBJ_SPHERE, target->dst.left,
                            target->dst.top, -1.0, -1.0 );
                        addObject( OBJ_CUBE, target->dst.left,
                            target->dst.top, -1.0, -1.0 );
                        addObject( OBJ_CUBE, target->dst.left,
                            target->dst.top, -1.0, -1.0 );
                        score += 20;
                        break;
                    case OBJ_CUBE:
                        if(bWantSound)
                        {
                            playPanned(hsoCubeExplode, target);
                        }
                        addObject( OBJ_SPHERE, target->dst.left,
                            target->dst.top, -1.0, -1.0 );
                        addObject( OBJ_SPHERE, target->dst.left,
                            target->dst.top, -1.0, -1.0 );
                        score += 40;
                        break;
                    case OBJ_SPHERE:
                        if(bWantSound)
                        {
                            playPanned(hsoSphereExplode, target);
                        }
                        score += 20;
                    }

                    l = target->dst.left;
                    t = target->dst.top;
                    DeleteFromList( target );
                }

                hit = TRUE;
            }

            if( hit )
            {
                if( bullet == &DL )
                {
                    hit = FALSE;
                    if (!lastShield && !showDelay && !bTest)
                    {
                        if(bWantSound)
                        {
                            playPanned(hsoShipExplode, bullet);
                            SndObjStop(hsoEngineIdle);
                        }
                        score -= 150;
                        if (score < 0)
                            score = 0;

                        addObject( OBJ_SPHERE, l, t, -1.0, -1.0 );
                        addObject( OBJ_SPHERE, l, t, -1.0, -1.0 );
                        addObject( OBJ_SPHERE, l, t, -1.0, -1.0 );
                        addObject( OBJ_SPHERE, l, t, -1.0, -1.0 );
                        addObject( OBJ_BULLET, l, t,
                            randDouble( -0.5, 0.5 ), randDouble( -0.5, 0.5 ) );
                        addObject( OBJ_BULLET, l, t,
                            randDouble( -0.5, 0.5 ), randDouble( -0.5, 0.5 ) );
                        addObject( OBJ_BULLET, l, t,
                            randDouble( -0.5, 0.5 ), randDouble( -0.5, 0.5 ) );
                        addObject( OBJ_BULLET, l, t,
                            randDouble( -0.5, 0.5 ), randDouble( -0.5, 0.5 ) );
                        addObject( OBJ_BULLET, l, t,
                            randDouble( -0.5, 0.5 ), randDouble( -0.5, 0.5 ) );
                        addObject( OBJ_BULLET, l, t,
                            randDouble( -0.5, 0.5 ), randDouble( -0.5, 0.5 ) );
                        addObject( OBJ_BULLET, l, t,
                            randDouble( -0.5, 0.5 ), randDouble( -0.5, 0.5 ) );
                        addObject( OBJ_BULLET, l, t,
                            randDouble( -0.5, 0.5 ), randDouble( -0.5, 0.5 ) );
                        addObject( OBJ_BULLET, l, t,
                            randDouble( -0.5, 0.5 ), randDouble( -0.5, 0.5 ) );
                        addObject( OBJ_BULLET, l, t,
                            randDouble( -0.5, 0.5 ), randDouble( -0.5, 0.5 ) );
                        initShip(TRUE);
                    }
                }

                break;
            }
        }

        if( hit )
        {
            save = bullet;
            bullet = bullet->next;
            
            if (save != &DL)
                DeleteFromList( save );
        }
        else
        {
            bullet = bullet->next;
        }

    } while (bullet != &DL);
}

void EraseScreen( void )
{
    DDBLTFX     ddbltfx;
    HRESULT     ddrval;

    // iForceErase forces us to erase so we can get of stray GDI pixels
    if (iForceErase)
    {
        iForceErase--;
    }
    else if( bSpecialEffects )   // cool looking screen with no colorfill
    {
        return;
    }

    // Erase the background
    memset (&ddbltfx, 0, sizeof (ddbltfx));
    ddbltfx.dwSize = sizeof( ddbltfx );
#ifdef NONAMELESSUNION
    ddbltfx.u5.dwFillColor = dwFillColor;
#else
    ddbltfx.dwFillColor = dwFillColor;
#endif
    while( 1 )
    {
        ddrval = lpBackBuffer->lpVtbl->Blt( lpBackBuffer, NULL, NULL,
                 NULL, DDBLT_COLORFILL, &ddbltfx );

        if( ddrval == DD_OK )
        {
            break;
        }
        if( ddrval == DDERR_SURFACELOST )
        {
            if( !RestoreSurfaces() )
                return;
        }
        if( ddrval != DDERR_WASSTILLDRAWING )
        {
            return;
        }
    }
}

void FlipScreen( void )
{
    HRESULT     ddrval;

    // Flip the surfaces
    while( 1 )
    {
        ddrval = lpFrontBuffer->lpVtbl->Flip( lpFrontBuffer, NULL, 0 );
        if( ddrval == DD_OK )
        {
            break;
        }
        if( ddrval == DDERR_SURFACELOST )
        {
            if( !RestoreSurfaces() )
            {
                return;
            }
        }
        if( ddrval != DDERR_WASSTILLDRAWING )
        {
            break;
        }
    }
}

void DrawDisplayList( void )
{
    LPDBLNODE   this;
    LPDBLNODE   last;
    HRESULT     ddrval;
    TCHAR        scorebuf[11];
    int         rem;

    // blt everything in reverse order if we are doing destination transparency
    // calculate score string
    scorebuf[0] = score/10000000 + TEXT('0');
    rem = score % 10000000;
    scorebuf[1] = rem/1000000 + TEXT('0');
    rem = score % 1000000;
    scorebuf[2] = rem/100000 + TEXT('0');
    rem = score % 100000;
    scorebuf[3] = rem/10000 + TEXT('0');
    rem = score % 10000;
    scorebuf[4] = rem/1000 + TEXT('0');
    rem = score % 1000;
    scorebuf[5] = rem/100 + TEXT('0');
    rem = score % 100;
    scorebuf[6] = rem/10 + TEXT('0');
    rem = score % 10;
    scorebuf[7] = rem + TEXT('0');
    if( bSoundEnabled )
    {
        scorebuf[8] = 14 + TEXT('0');
        scorebuf[9] = 13 + TEXT('0');
        scorebuf[10] = TEXT('\0');
    }
    else
    {
        scorebuf[8] = TEXT('\0');
    }

    EraseScreen();
    if( dwTransType == DDBLT_KEYDEST )
    {
        bltScore(scorebuf, 10, ScreenY-26);

        if( bShowFrameCount )
            DisplayFrameRate();

        this = DL.next; // start with the topmost bitmap
        last = DL.next; // don't blt it twice

        if (showDelay)
            last = &DL;
    }
    else
    {
        this = &DL;     // start with the bottommost bitmap (the ship)
        last = &DL;     // don't blt it twice

        if (showDelay)
            this = this->prev;
    }

    do
    {
        while (!IsRectEmpty(&this->src))
        {
            ddrval = lpBackBuffer->lpVtbl->Blt( lpBackBuffer, &(this->dst), this->surf, &(this->src), dwTransType, NULL );

            if( ddrval == DD_OK )
            {
                break;
            }
            if( ddrval == DDERR_SURFACELOST )
            {
                if( !RestoreSurfaces() )
                    return;
            }
            if( ddrval != DDERR_WASSTILLDRAWING )
            {
                return;
            }
        }
        if( dwTransType != DDBLT_KEYDEST )
        {
            this = this->prev;
        }
        else
        {
            this = this->next;
        }
    }
    while( this != last );

    if( dwTransType != DDBLT_KEYDEST )
    {
        bltScore(scorebuf, 10, ScreenY-26);

        if( bShowFrameCount )
            DisplayFrameRate();
    }

    FlipScreen();
}

void DisplayFrameRate( void )
{
    DWORD               time2;
    TCHAR               buff[256];

    dwFrameCount++;
    time2 = timeGetTime() - dwFrameTime;
    if( time2 > 1000 )
    {
        dwFrames = (dwFrameCount*1000)/time2;
        dwFrameTime = timeGetTime();
        dwFrameCount = 0;
    }
    if( dwFrames == 0 )
    {
        return;
    }

    if (dwFrames != dwFramesLast)
    {
        dwFramesLast = dwFrames;
    }

    if( dwFrames > 99 )
    {
        dwFrames = 99;
    }
    buff[0] = (TCHAR)((dwFrames / 10) + TEXT('0'));
    buff[1] = (TCHAR)((dwFrames % 10) + TEXT('0'));
    buff[2] = TEXT('\0');
    bltScore(buff, ScreenX/2-25, 10);
}

void DeleteFromList( LPDBLNODE this )
{
    this->next->prev = this->prev;
    this->prev->next = this->next;
    LocalFree( this );
}

void UpdateEnginePan(void)
{
    double pan = (2 * DL.posx / (double) ScreenX) - 1.0;
    double hack = 0.2;
    SndObjSetPan(hsoEngineIdle, hack * pan);
}
void UpdateEngineIdle(void)
{
    double absvel = sqrt(DL.vely*DL.vely + DL.velx * DL.velx);
    double rate = 1.0 + absvel; //log10(10.0 + absvel);
    SndObjSetRate(hsoEngineIdle, rate);
}


void UpdateDisplayList( void )
{
    LPDBLNODE   this;
    LPDBLNODE   save;
    DWORD       thisTickCount = GetTickCount();
    DWORD       tickDiff = thisTickCount - lastTickCount;
    double      maxx, maxy;
    double      maxframe;
    DWORD       input;
    BOOL        event = FALSE;

    lastTickCount = thisTickCount;

    if( bTest )
    {
        input = (KEY_RIGHT | KEY_FIRE);
    }
    else 
    {
        input = GlobalInput;
    }

    if (showDelay)
    {
        showDelay -= (int)tickDiff;
        if (showDelay < 0)
        {
            showDelay = 0;
            lastShield = FALSE;
            initShip( FALSE );
        }
    }

    // update the ship
    if( !showDelay )
    {
        DL.posx += DL.velx * (double)tickDiff;
        DL.posy += DL.vely * (double)tickDiff;
    }
    if( DL.posx > MAX_SHIP_X )
    {
        DL.posx = MAX_SHIP_X;
        DL.velx = -DL.velx;
        event = TRUE;
    }
    else if ( DL.posx < 0 )
    {
        DL.posx =0;
        DL.velx = -DL.velx;
        event = TRUE;
    }
    if( DL.posy > MAX_SHIP_Y )
    {
        DL.posy = MAX_SHIP_Y;
        DL.vely = -DL.vely;
        event = TRUE;
    }
    else if ( DL.posy < 0 )
    {
        DL.posy =0;
        DL.vely = -DL.vely;
        event = TRUE;
    }
    if (event)
    {
        if(bWantSound)
        {
            playPanned(hsoShipBounce, &DL);
            UpdateEngineIdle();
        }
        event = FALSE;
    }
    UpdateEnginePan();

    if ((event = (showDelay || ((input & KEY_SHIELD) == KEY_SHIELD))) !=
        lastShield)
    {
        if (event && !showDelay)
                {
                    if(bWantSound)
                    {
                        SndObjPlay(hsoShieldBuzz);
                    }
                    bPlayBuzz = TRUE;
                }
        else
                {
                if(bWantSound)
                {
                    SndObjStop(hsoShieldBuzz);
                }
                bPlayBuzz = FALSE;
            }
        lastShield = event;
    }
    if (event)
    {
        input &= ~(KEY_FIRE);
    }

    if (input & KEY_FIRE)
    {
        if( !showDelay )
        {
            // add a bullet to the scene
            score--;
            if(score < 0)
                score = 0;

            if(bWantSound)
            {
                SndObjPlay(hsoFireBullet);
            }
            addObject( OBJ_BULLET, Dirx[(int)DL.frame]*6.0 + 16.0 + DL.posx,
                                   Diry[(int)DL.frame]*6.0 + 16.0 + DL.posy,
                                   Dirx[(int)DL.frame]*500.0/1000.0,
                                   Diry[(int)DL.frame]*500.0/1000.0 );
        }
    }

    event = FALSE;
    if( input & KEY_LEFT )
    {
            DL.frame -= 1.0;
            if( DL.frame < 0.0 )
                DL.frame += MAX_SHIP_FRAME;
    }
    if( input & KEY_RIGHT )
    {
            DL.frame += 1.0;
            if( DL.frame >= MAX_SHIP_FRAME)
                DL.frame -= MAX_SHIP_FRAME;
    }
    if( input & KEY_UP )
    {
            DL.velx += Dirx[(int)DL.frame] * 10.0/1000.0;
            DL.vely += Diry[(int)DL.frame] * 10.0/1000.0;
            event = TRUE;
    }
    if( input & KEY_DOWN )
    {
            DL.velx -= Dirx[(int)DL.frame] * 10.0/1000.0;
            DL.vely -= Diry[(int)DL.frame] * 10.0/1000.0;
            event = TRUE;
    }

    // setrate
    if (event) {
        UpdateEngineIdle();
    }

    if (event != lastThrust)
    {
        if (event)
        {
            input &= ~KEY_STOP;
            if(bWantSound)
            {
                SndObjStop(hsoSkidToStop);
                SndObjPlay(hsoEngineRev);
            }
            bPlayRev = TRUE;
        }
        else
        {
            if(bWantSound)
            {
                SndObjStop(hsoEngineRev);
            }
            bPlayRev = FALSE;
        }

        lastThrust = event;
    }

    if( input & KEY_STOP )
    {
        if(bWantSound)
        {
            if (DL.velx || DL.vely)
                playPanned(hsoSkidToStop, &DL);
        }

        DL.velx = 0;
        DL.vely = 0;

        UpdateEngineIdle();
    }

    this = DL.next;
    do
    {
        this->posx += this->velx * (double)tickDiff;
        this->posy += this->vely * (double)tickDiff;
        this->frame += this->delay * (double)tickDiff;
        switch( this->type )
        {
            case OBJ_DONUT:
                maxx = (double)MAX_DONUT_X;
                maxy = (double)MAX_DONUT_Y;
                maxframe = (double)MAX_DONUT_FRAME;
                break;
            case OBJ_PYRAMID:
                maxx = (double)MAX_PYRAMID_X;
                maxy = (double)MAX_PYRAMID_Y;
                maxframe = (double)MAX_PYRAMID_FRAME;
                break;
            case OBJ_SPHERE:
                maxx = (double)MAX_SPHERE_X;
                maxy = (double)MAX_SPHERE_Y;
                maxframe = (double)MAX_SPHERE_FRAME;
                break;
            case OBJ_CUBE:
                maxx = (double)MAX_CUBE_X;
                maxy = (double)MAX_CUBE_Y;
                maxframe = (double)MAX_CUBE_FRAME;
                break;
            case OBJ_BULLET:
                maxx = (double)MAX_BULLET_X;
                maxy = (double)MAX_BULLET_Y;
                maxframe = (double)MAX_BULLET_FRAME;
                if( this->frame >= (double)MAX_BULLET_FRAME )
                {
                    save = this;
                    this = this->next;
                    DeleteFromList( save );
                    continue;
                }
                break;
        }
        if( this != &DL )
        {
            if( this->posx > maxx )
            {
                this->posx = maxx;
                this->velx = -this->velx;
            }
            else if ( this->posx < 0 )
            {
                this->posx =0;
                this->velx = -this->velx;
            }
            if( this->posy > maxy )
            {
                this->posy = maxy;
                this->vely = -this->vely;
            }
            else if ( this->posy < 0 )
            {
                this->posy =0;
                this->vely = -this->vely;
            }
            if( this->frame >= maxframe )
            {
                this->frame -= maxframe;
            }
            this = this->next;
        }
    }
    while( this != &DL );
}

BOOL isDisplayListEmpty( void )
{
    LPDBLNODE ptr;

    for(ptr=DL.next; ptr != &DL; ptr = ptr->next)
    {
        if(ptr->type != OBJ_BULLET)
            return FALSE;
    }
    return TRUE;
}

void initShip( BOOL delay )
{
    DL.posx = (double)(ScreenX/2-16);       // center the ship
    DL.posy = (double)(ScreenY/2-16);
    DL.frame = 0.0;
    if( bTest )
    {
        DL.velx = 0.25;
        DL.vely = 0.5;
    }
    else
    {
        DL.velx = DL.vely = 0.0;        // not moving
    }
    if( !bTest && delay ) {
        showDelay = DEF_SHOW_DELAY;
    }
    else {
        UpdateEngineIdle();
        SndObjPlay(hsoEngineIdle);
    }
}

void initLevel( int level )
{
    int     i;

    // clear any stray bullets out of the display list
    while( DL.next != &DL )
    {
        DeleteFromList( DL.next );
    }
    for(i=0; i<(2*level-1); i++)
    {
        addObject( OBJ_DONUT, -1.0, -1.0, -1.0, -1.0 );
    }
    initShip(TRUE);
}

void addObject( SHORT type, double x, double y, double vx, double vy )
{
    LPDBLNODE   new;

    new = (LPDBLNODE) LocalAlloc( LPTR, sizeof( DBLNODE ) );
    if( new == NULL)
        return;

    new->type = type;
    switch( type )
    {
        case OBJ_DONUT:
            if( x < 0.0) // no position specified?
            {
                new->posx = randDouble( 0.0, (double)MAX_DONUT_X );
                new->posy = randDouble( 0.0, (double)MAX_DONUT_Y );
            }
            else
            {
                new->posx = x;
                new->posy = y;
            }
            new->velx = randDouble( -50.0/1000.0, 50.0/1000.0 );
            new->vely = randDouble( -50.0/1000.0, 50.0/1000.0 );
            new->frame = randDouble( 0, 30 );
            new->delay = 30.0*randDouble( 0.1, 0.4 )/1000.0;
            new->surf = lpDonut;
            linkObject( new );
            break;
        case OBJ_PYRAMID:
            if( x < 0) // no position specified?
            {
                new->posx = randDouble( 0.0, (double)MAX_PYRAMID_X );
                new->posy = randDouble( 0.0, (double)MAX_PYRAMID_Y );
            }
            else
            {
                new->posx = x;
                new->posy = y;
            }
            new->velx = 1.5*randDouble( -50.0/1000.0, 50.0/1000.0 );
            new->vely = 1.5*randDouble( -50.0/1000.0, 50.0/1000.0 );
            new->frame = randDouble( 0, 30 );
            new->delay = 40.0*randDouble( 0.3, 1.0 )/1000.0;
            new->surf = lpPyramid;
            linkObject( new );
            break;
        case OBJ_SPHERE:
            if( x < 0) // no position specified?
            {
                new->posx = randDouble( 0.0, (double)MAX_SPHERE_X );
                new->posy = randDouble( 0.0, (double)MAX_SPHERE_Y );
            }
            else
            {
                new->posx = x;
                new->posy = y;
            }
            new->velx = 3.0*randDouble( -50.0/1000.0, 50.0/1000.0 );
            new->vely = 3.0*randDouble( -50.0/1000.0, 50.0/1000.0 );
            new->frame = randDouble( 0, 30 );
            new->delay = 40.0*randDouble( 1.5, 2.0 )/1000.0;
            new->surf = lpSphere;
            linkObject( new );
            break;
        case OBJ_CUBE:
            if( x < 0) // no position specified?
            {
                new->posx = randDouble( 0.0, (double)MAX_CUBE_X );
                new->posy = randDouble( 0.0, (double)MAX_CUBE_Y );
            }
            else
            {
                new->posx = x;
                new->posy = y;
            }
            new->velx = 4.0*randDouble( -50.0/1000.0, 50.0/1000.0 );
            new->vely = 4.0*randDouble( -50.0/1000.0, 50.0/1000.0 );
            new->frame = randDouble( 0, 30 );
            new->delay = 40.0*randDouble( 0.8, 2.0 )/1000.0;
            new->surf = lpCube;
            linkObject( new );
            break;
        case OBJ_BULLET:
            new->posx = x;
            new->posy = y;
            new->velx = vx;
            new->vely = vy;
            new->frame = 0.0;
            new->delay = 1.0;
            new->surf = lpNum;
            linkObject( new );
            break;
    }
}

void linkObject( LPDBLNODE new )
{
    new->next = DL.next;
    new->prev = &DL;
    DL.next->prev = new;
    DL.next = new;
}

void linkLastObject( LPDBLNODE new )
{
    new->prev = DL.prev;
    new->next = &DL;
    DL.prev->next = new;
    DL.prev = new;
}

BOOL RestoreSurfaces( void )
{
    HRESULT     ddrval;
    HBITMAP     hbm;

    ddrval = lpFrontBuffer->lpVtbl->Restore(lpFrontBuffer);
    if( ddrval != DD_OK )
        return FALSE;
    ddrval = lpDonut->lpVtbl->Restore(lpDonut);
    if( ddrval != DD_OK )
        return FALSE;
    ddrval = lpPyramid->lpVtbl->Restore(lpPyramid);
    if( ddrval != DD_OK )
        return FALSE;
    ddrval = lpCube->lpVtbl->Restore(lpCube);
    if( ddrval != DD_OK )
        return FALSE;
    ddrval = lpSphere->lpVtbl->Restore(lpSphere);
    if( ddrval != DD_OK )
        return FALSE;
    ddrval = lpShip->lpVtbl->Restore(lpShip);
    if( ddrval != DD_OK )
        return FALSE;
    ddrval = lpNum->lpVtbl->Restore(lpNum);
    if( ddrval != DD_OK )
        return FALSE;

    hbm = (HBITMAP)LoadImage( hInst, TEXT("DONUTS8"), IMAGE_BITMAP, 0, 0, 0 );

    if( NULL == hbm )
        return FALSE;

    ddrval = DDCopyBitmap( lpDonut, hbm, 0, 0, 320, 384 );
    if( ddrval != DD_OK )
    {
        DeleteObject( hbm );
        return FALSE;
    }

    ddrval = DDCopyBitmap( lpPyramid, hbm, 0, 384, 320, 128 );
    if( ddrval != DD_OK )
    {
        DeleteObject( hbm );
        return FALSE;
    }

    ddrval = DDCopyBitmap( lpSphere, hbm, 0, 512, 320, 32 );
    if( ddrval != DD_OK )
    {
        DeleteObject( hbm );
        return FALSE;
    }

    ddrval = DDCopyBitmap( lpCube, hbm, 0, 544, 320, 32 );
    if( ddrval != DD_OK )
    {
        DeleteObject( hbm );
        return FALSE;
    }

    ddrval = DDCopyBitmap( lpShip, hbm, 0, 576, 320, 256 );
    if( ddrval != DD_OK )
    {
        DeleteObject( hbm );
        return FALSE;
    }

    ddrval = DDCopyBitmap( lpNum, hbm, 0, 832, 320, 16 );
    if( ddrval != DD_OK )
    {
        DeleteObject( hbm );
        return FALSE;
    }

    DeleteObject( hbm );

    // Set fill color to black
    dwFillColor = 0;

    // Set the transparent color to black for each sprite
    if (FAILED(DDSetColorKey(lpDonut, RGB(0,0,0))))
        return FALSE;

    if (FAILED(DDSetColorKey(lpPyramid, RGB(0,0,0))))
        return FALSE;

    if (FAILED(DDSetColorKey(lpCube, RGB(0,0,0))))
        return FALSE;

    if (FAILED(DDSetColorKey(lpSphere, RGB(0,0,0))))
        return FALSE;

    if (FAILED(DDSetColorKey(lpShip,  RGB(0,0,0))))
        return FALSE;

    if (FAILED(DDSetColorKey(lpNum, RGB(0,0,0))))
        return FALSE;

    return TRUE;
}

int randInt( int low, int high )
{
    int range = high - low;
    int num = (int)(Random() % range);
    return( num + low );
}

double randDouble( double low, double high )
{
    double range = high - low;
    double num = (double)(range * (double)Random()/0xFFFFFFFF);
    return( num + low );
}

BOOL InitializeSound( void )
{
    bSoundEnabled = FALSE;

    if (!bWantSound) {
        return TRUE;
    }

    hSoundDevice = SndCreateSndDevice(0);
    if (hSoundDevice != NULL) {
        hsoBeginLevel     = SndObjCreate(hSoundDevice, hInst, TEXT("BeginLevel"),     FALSE,  1);
        hsoEngineIdle     = SndObjCreate(hSoundDevice, hInst, TEXT("EngineIdle"),      TRUE,  1);
        hsoEngineRev      = SndObjCreate(hSoundDevice, hInst, TEXT("EngineRev"),       TRUE,  1);
        hsoSkidToStop     = SndObjCreate(hSoundDevice, hInst, TEXT("SkidToStop"),     FALSE,  1);
        hsoShieldBuzz     = SndObjCreate(hSoundDevice, hInst, TEXT("ShieldBuzz"),      TRUE,  1);
        hsoShipExplode    = SndObjCreate(hSoundDevice, hInst, TEXT("ShipExplode"),    FALSE,  1);
        hsoFireBullet     = SndObjCreate(hSoundDevice, hInst, TEXT("Gunfire"),        FALSE, 25);
        hsoShipBounce     = SndObjCreate(hSoundDevice, hInst, TEXT("ShipBounce"),     FALSE,  4);
        hsoDonutExplode   = SndObjCreate(hSoundDevice, hInst, TEXT("DonutExplode"),   FALSE, 10);
        hsoPyramidExplode = SndObjCreate(hSoundDevice, hInst, TEXT("PyramidExplode"), FALSE, 12);
        hsoCubeExplode    = SndObjCreate(hSoundDevice, hInst, TEXT("CubeExplode"),    FALSE, 15);
        hsoSphereExplode  = SndObjCreate(hSoundDevice, hInst, TEXT("SphereExplode"),  FALSE, 10);
        bSoundEnabled = TRUE;
    }
    return TRUE;
}

void DestroySound( void )
{
    if (!bWantSound) {
        return; //No work to be done

    }
    bSoundEnabled = FALSE;
    if (hSoundDevice) {
        SndObjDestroy(hsoBeginLevel);
        hsoBeginLevel = NULL;
        SndObjDestroy(hsoEngineIdle);
        hsoEngineIdle = NULL;
        SndObjDestroy(hsoEngineRev);
        hsoEngineRev = NULL;
        SndObjDestroy(hsoSkidToStop);
        hsoSkidToStop = NULL;
        SndObjDestroy(hsoShieldBuzz);
        hsoShieldBuzz = NULL;
        SndObjDestroy(hsoShipExplode);
        hsoShipExplode = NULL;
        SndObjDestroy(hsoFireBullet);
        hsoFireBullet = NULL;
        SndObjDestroy(hsoShipBounce);
        hsoShipBounce = NULL;
        SndObjDestroy(hsoDonutExplode);
        hsoDonutExplode = NULL;
        SndObjDestroy(hsoPyramidExplode);
        hsoPyramidExplode = NULL;
        SndObjDestroy(hsoCubeExplode);
        hsoCubeExplode = NULL;
        SndObjDestroy(hsoSphereExplode);
        hsoSphereExplode = NULL;

        SndObjCloseDevice(hSoundDevice);
        hSoundDevice = NULL;
    }
}
