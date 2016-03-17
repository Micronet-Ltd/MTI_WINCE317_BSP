// rotate.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "rotate.h"
#include <windows.h>
#include <commctrl.h>

#define NAME                TEXT("MosquitoWndClass")
#define TITLE               TEXT("Mosquito")

#define BUG_WIDTH           320
#define BUG_HEIGHT          200
#define TIMER_ID            1
#define TIMER_RATE          200

#ifdef UNDER_CE
#define RAND_INT(x) (Random() % x)
#else
#define RAND_INT(x) (rand()*x/RAND_MAX)
#endif
#define RANDOM_VELOCITY() (int)(((RAND_INT(5)+3)*2))

#define PF_TABLE_SIZE (sizeof(ddpfOverlayFormats) / sizeof(ddpfOverlayFormats[0]))

LPDIRECTDRAW                g_pDD = NULL;        // DirectDraw object
LPDIRECTDRAWSURFACE         g_pDDSPrimary = NULL; // Primary Surface.
LPDIRECTDRAWSURFACE         g_pDDSOverlay = NULL; // The overlay primary.
BOOL                        g_bActive = FALSE;   // Is application active?

// Overlay position and velocity data.

int g_nOverlayXPos, g_nOverlayYPos;
int g_nOverlayXVel, g_nOverlayYVel;
int g_nOverlayWidth, g_nOverlayHeight;
DWORD g_dwOverlayXPositionAlignment;

int                         g_RotationAngles = 0; // Supported rotation angles.
int                         g_CurrentAngle = 0;   // Current rotation angle.
static TCHAR                szImg1[] = TEXT("IDB_BUGIMAGE1");
static TCHAR                szImg2[] = TEXT("IDB_BUGIMAGE2");
static TCHAR                szImg3[] = TEXT("IDB_BUGIMAGE3");

// These are the pixel formats this app supports.  Most display adapters
// with overlay support will recognize one or more of these formats.
// We start with YUV format, then work down to RGB. (All 16 bpp.)
static DDPIXELFORMAT ddpfOverlayFormats[] = {
    //{sizeof(DDPIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('Y','U','Y','V'),0,0,0,0,0},  // YUYV
    //{sizeof(DDPIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('U','Y','V','Y'),0,0,0,0,0},  // UYVY
    {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16,  0x7C00, 0x03e0, 0x001F, 0},        // 16-bit RGB 5:5:5
    {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16,  0xF800, 0x07e0, 0x001F, 0}         // 16-bit RGB 5:6:5
};


HINSTANCE g_hInstance;
static RECT rs;
static RECT rd;


#define PREFIX      TEXT("MOSQUITO: ")
#define PREFIX_LEN  10

static void ReleaseAllObjects(void)
{
    if (g_pDDSOverlay != NULL)
    {
        // Use UpdateOverlay() with the DDOVER_HIDE flag to remove an overlay 
	    // from the display.
	    g_pDDSOverlay->UpdateOverlay(NULL, g_pDDSPrimary, NULL, DDOVER_HIDE, NULL);
        g_pDDSOverlay->Release();
        g_pDDSOverlay = NULL;
    }

    if (g_pDDSPrimary != NULL)
    {
        g_pDDSPrimary->Release();
        g_pDDSPrimary = NULL;
    }

    if (g_pDD != NULL)
    {
        g_pDD->Release();
        g_pDD = NULL;
    }
}

static HRESULT InitFail(HWND hWnd, HRESULT hRet, LPCTSTR szError,...)
{
    TCHAR                       szBuff[128] = PREFIX;
    va_list                     vl;

    va_start(vl, szError);
    StringCchVPrintf(szBuff + PREFIX_LEN, (128-PREFIX_LEN), szError, vl);    
    size_t len = wcslen(szBuff);
    StringCchPrintf(szBuff + len, 128 - len, TEXT("\r\n"));
    ReleaseAllObjects();
    OutputDebugString(szBuff);
    DestroyWindow(hWnd);
    va_end(vl);
    return hRet;
}
#undef PREFIX_LEN
#undef PREFIX

static BOOL CopyBitmapToYUVSurface(LPDIRECTDRAWSURFACE lpDDSurf, HBITMAP hbm)
{
    HDC                 hdcImage;
    HRESULT             ddrval;
    DDSURFACEDESC       ddsd;
    DWORD               x, y, dwWidth, dwHeight;
    DWORD               dwPitch;
    LPBYTE              pSurf;
    DWORD               dwBytesInRow;
    COLORREF            color;
    BYTE                R,G,B, Y0,Y1,U,V;
    BOOL                bRet = FALSE;

    if (hbm == NULL || lpDDSurf == NULL)
	return FALSE;

    //
    //  select bitmap into a memoryDC so we can use it.
    //
    hdcImage = CreateCompatibleDC(NULL);
    SelectObject(hdcImage, hbm);

    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    // Lock down the surface so we can modify it's contents.
    ddrval=lpDDSurf->Lock( NULL, &ddsd, DDLOCK_WAITNOTBUSY, NULL);
    if (FAILED(ddrval))
	    goto CleanUp;

    dwWidth=ddsd.dwWidth;
    dwHeight=ddsd.dwHeight;
    dwPitch=ddsd.lPitch;
    pSurf=(LPBYTE)ddsd.lpSurface;
	dwBytesInRow=ddsd.dwWidth*2;

    // Go through the image 2 pixels at a time and convert to YUV
    for(y=0; y<dwHeight; y++)
    {
	for(x=0; x<dwWidth; x+=2)
		{
	    // The equations for color conversion used here, probably aren't 
	    // exact, but they seem to do an OK job.
			color=GetPixel(hdcImage, x,y);
			R=GetRValue(color);
			G=GetGValue(color);
			B=GetBValue(color);
		    Y0= (BYTE)(0.29*R + 0.59*G + 0.14*B);
			U= (BYTE)(128.0 - 0.14*R - 0.29*G + 0.43*B);

			color=GetPixel(hdcImage, x+1,y);
			R=GetRValue(color);
			G=GetGValue(color);
			B=GetBValue(color);
			Y1= (BYTE)(0.29*R + 0.57*G + 0.14*B);
			V= (BYTE)(128.0 + 0.36*R - 0.29*G - 0.07*B);

			switch (ddsd.ddpfPixelFormat.dwFourCC)
			{
				case MAKEFOURCC('Y','U','Y','V'): 
					*(pSurf++) = Y0;
					*(pSurf++) = U;
					*(pSurf++) = Y1;
					*(pSurf++) = V;
					break;
				case MAKEFOURCC('U','Y','V','Y'): 
					*(pSurf++) = U;
					*(pSurf++) = Y0;
					*(pSurf++) = V;
					*(pSurf++) = Y1;
					break;
			}                       
		}
		pSurf+=(dwPitch-dwBytesInRow);
    }

    lpDDSurf->Unlock(NULL);     

CleanUp:
    if(hdcImage)
	DeleteDC(hdcImage);

	return TRUE;
}

static BOOL LoadImageOntoSurface(LPDIRECTDRAWSURFACE lpdds, LPCTSTR lpstrResID)
{
    HBITMAP hbm = NULL;
    HDC     hdcImage = NULL;
    HDC     hdcSurf = NULL;
    BOOL bRetVal = FALSE;
    HRESULT ddrval;
    DDSURFACEDESC ddsd;

    if (!lpdds) return FALSE;

    //
    // get surface size and format.
    //
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
    ddrval = lpdds->GetSurfaceDesc(&ddsd);
    if (FAILED(ddrval))
	    goto Exit;

	// Load the bitmap resource.  We'll use LoadImage() since it'll scale the 
    // image to fit our surface, and maintain the color information in the
    // bitmap.
    hbm = (HBITMAP)LoadImage(g_hInstance, lpstrResID, IMAGE_BITMAP, 0, 0, 0);
	if (hbm == NULL)
	    goto Exit;


    // If our surface is a FOURCC YUV format, we need to do a little work to convert
    // our RGB resource bitmap into the appropriate YUV format.
    if (ddsd.ddpfPixelFormat.dwFlags == DDPF_FOURCC)
    {
	if (!CopyBitmapToYUVSurface(lpdds, hbm))
	    goto Exit;        
    }
    else  //Looks like we're just using a standard RGB surface format, let GDI do the work.
    {
	    // Create a DC and associate the bitmap with it.
	    hdcImage = CreateCompatibleDC(NULL);
	    SelectObject(hdcImage, hbm);
   
	    ddrval = lpdds->GetDC(&hdcSurf);
	    if (FAILED(ddrval))
	        goto Exit;
    
	    if (BitBlt(hdcSurf, 0, 0, ddsd.dwWidth, ddsd.dwHeight, hdcImage, 0, 0, SRCCOPY) == FALSE)
	        goto Exit;
    }

    bRetVal = TRUE;
    
Exit:
    if (hdcSurf)
	    lpdds->ReleaseDC(hdcSurf);
    if (hdcImage)
	    DeleteDC(hdcImage);
    if (hbm)
	    DeleteObject(hbm);

    return bRetVal;
}

static HRESULT WINAPI EnumSurfacesCallback(LPDIRECTDRAWSURFACE lpDDSurface,  
                     LPDDSURFACEDESC lpDDSurfaceDesc,  
                     LPVOID lpContext)
{
    int * CallCount = (int *)lpContext;
    HRESULT hr = (HRESULT)DDENUMRET_OK;
    LPCTSTR ResName;

    // Load the Bug Image appropriate...

    if (*CallCount == 0) {
        ResName = szImg2;
    }
    else if (*CallCount == 1) {
        ResName = szImg3;
    }
    else {
        // Eh?
        hr = (HRESULT)DDENUMRET_CANCEL;
        goto exit;
    }

    if (!LoadImageOntoSurface(lpDDSurface, ResName)) {
        hr = (HRESULT)DDENUMRET_CANCEL;
        goto exit;
    }

    // Bump the count.

    (*CallCount)++;

exit:
    lpDDSurface->Release();
    return hr;
}

static HRESULT LoadBugImages()
{
    HRESULT hRet;
    int CallCount = 0;

    // Put the first bug image onto the first buffer of our complex surface.
    if (!LoadImageOntoSurface(g_pDDSOverlay, szImg1))
	    return (E_FAIL);

    // Use the enumeration attachment function to load the other images.
    hRet = g_pDDSOverlay->EnumAttachedSurfaces((LPVOID)&CallCount,EnumSurfacesCallback);
    return (hRet);
}

static HRESULT RestoreAllSurfaces()
{
    HRESULT hRet;

    // Try Restoring the primary surface.

    hRet = g_pDDSPrimary->Restore();
    if (hRet != DD_OK)
	    return hRet;

    // Try Restoring the overlay surface.

    hRet = g_pDDSOverlay->Restore();
    if (hRet != DD_OK)
	    return hRet;

    // Reload the images.
    hRet = LoadBugImages();
    if (hRet != DD_OK)
        return hRet;

    // Show the overlay.

    hRet = g_pDDSOverlay->UpdateOverlay(&rs, g_pDDSPrimary, &rd, DDOVER_SHOW, NULL);
    return hRet;
}

static void MoveOverlay()
{
    HRESULT         hRet;
    DWORD           dwXAligned;

    // Add the current velocity vectors to the position.

    g_nOverlayXPos += g_nOverlayXVel;
    g_nOverlayYPos += g_nOverlayYVel;

    // Check to see if this new position puts the overlay off the edge of the screen.
    // SetOverlayPosition() won't like that.

    // Have we gone off the left edge?

    if (g_nOverlayXPos < 0) {
	    g_nOverlayXPos = 0;
	    g_nOverlayXVel = RANDOM_VELOCITY();
    }

    // Have we gone off the right edge?

    if ((g_nOverlayXPos+g_nOverlayWidth) >  GetSystemMetrics(SM_CXSCREEN)){
	    g_nOverlayXPos = GetSystemMetrics(SM_CXSCREEN) - g_nOverlayWidth;
	    g_nOverlayXVel = -RANDOM_VELOCITY();
    }

    // Have we gone off the top edge?

    if (g_nOverlayYPos < 0) {
	    g_nOverlayYPos = 0;
	    g_nOverlayYVel = RANDOM_VELOCITY();
    }

    // Have we gone off the bottom edge?

    if ( (g_nOverlayYPos+g_nOverlayHeight) >  GetSystemMetrics(SM_CYSCREEN)) {
	    g_nOverlayYPos = GetSystemMetrics(SM_CYSCREEN) - g_nOverlayHeight;
	    g_nOverlayYVel = -RANDOM_VELOCITY();
    }

    // We need to check for any alignment restrictions on the X position.

    if (g_dwOverlayXPositionAlignment)
	    dwXAligned = g_nOverlayXPos - g_nOverlayXPos % g_dwOverlayXPositionAlignment;
    else
	    dwXAligned = g_nOverlayXPos;

    // Set the overlay to it's new position.

    hRet = g_pDDSOverlay->SetOverlayPosition(dwXAligned, g_nOverlayYPos);
    if (hRet == DDERR_SURFACELOST)
    {
	    if (FAILED(RestoreAllSurfaces())) 
	        return;
    }

    // Flip.

    while (TRUE)
    {
        hRet = g_pDDSOverlay->Flip(NULL, 0);
        if (hRet == DD_OK)
            break;
        if (hRet == DDERR_SURFACELOST)
        {
            hRet = RestoreAllSurfaces();
            if (hRet != DD_OK)
                break;
        }
        if (hRet != DDERR_WASSTILLDRAWING)
            break;
    }
}

long FAR PASCAL WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int                         NewAngle;
    DEVMODE                     DevMode;

    switch (message)
    {
#ifdef UNDER_CE
        case WM_ACTIVATE:
#else
        case WM_ACTIVATEAPP:
#endif
            // Pause if minimized or not the top window
            g_bActive = (wParam == WA_ACTIVE) || (wParam == WA_CLICKACTIVE);
			RegisterHotKey(hWnd, VK_QUIT, 0, VK_F4);
            return 0L;

        case WM_KILLFOCUS:
            // We do not allow anyone else to have the keyboard focus until
            // we are done.
            SetFocus(hWnd);
            return 0L;

        case WM_DESTROY:
            // Clean up and close the app
			UnregisterHotKey(hWnd, VK_QUIT);
            ReleaseAllObjects();
            PostQuitMessage(0);
            return 0L;

		case WM_HOTKEY:
		{
			switch(wParam) 
			{
			
				case VK_QUIT: 
                    PostMessage(hWnd, WM_CLOSE, 0, 0);
					break;
			}
			break;
		}
        case WM_KEYDOWN:
            // Handle any non-accelerated key commands
            switch (wParam)
            {
                case VK_ESCAPE:
                case VK_F12:
                    PostMessage(hWnd, WM_CLOSE, 0, 0);
                    return 0L;

                case VK_SPACE:
                
                    // Rotate to the "next" angle.

                    if (g_CurrentAngle >= 0 && g_RotationAngles >= 0) {

                        NewAngle = g_CurrentAngle;

                        do
                        {
                            NewAngle <<= 1;

                            if (NewAngle == DMDO_0)
                            {
                                NewAngle = DMDO_90;
                            }

                            if (NewAngle > DMDO_270)
                            {
                                NewAngle = DMDO_0;
                            }
                        } while (!(NewAngle & g_RotationAngles) && (NewAngle != DMDO_0));

                        memset(&DevMode, 0, sizeof (DevMode));
                        DevMode.dmSize               = sizeof (DevMode);
                        DevMode.dmFields             = DM_DISPLAYORIENTATION;
                        DevMode.dmDisplayOrientation = NewAngle;

                        if (DISP_CHANGE_SUCCESSFUL == ChangeDisplaySettingsEx(NULL, &DevMode, NULL, CDS_RESET, NULL)) {

                            g_CurrentAngle = NewAngle;

							RestoreAllSurfaces();
                        }
                    }
                    return 0L;
            }
            break;

        case WM_TIMER:
            // Update and flip surfaces
            if (g_bActive && TIMER_ID == wParam)
            {
                MoveOverlay();
            }
            break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

static HRESULT InitApp(HINSTANCE hInstance, int nCmdShow)
{
    HWND                        hWnd;
    WNDCLASS                    wc;
    DDSURFACEDESC               ddsd;
    DDCAPS                      ddcaps;
    HRESULT                     hRet;
    DWORD                       dwUpdateFlags = 0;
    DDOVERLAYFX                 ovfx;
    DEVMODE                     DevMode;

    // Check for rotation support by getting the rotation angles supported.
 
    memset(&DevMode, 0, sizeof(DevMode));
    DevMode.dmSize = sizeof(DevMode);
    DevMode.dmFields = DM_DISPLAYQUERYORIENTATION;

    if (DISP_CHANGE_SUCCESSFUL == ChangeDisplaySettingsEx(NULL, &DevMode, NULL, CDS_TEST, NULL)) {

        g_RotationAngles = DevMode.dmDisplayOrientation;
    }
    else {

        OutputDebugString(L"MOSQUITO: Device does not support any rotation modes. Rotation disabled.");
        g_RotationAngles = -1;
    }

    // Get the current rotation angle.

    memset(&DevMode, 0, sizeof (DevMode));
    DevMode.dmSize = sizeof (DevMode);
    DevMode.dmFields = DM_DISPLAYORIENTATION;

    if (DISP_CHANGE_SUCCESSFUL == ChangeDisplaySettingsEx(NULL, &DevMode, NULL, CDS_TEST, NULL)) {

        g_CurrentAngle = DevMode.dmDisplayOrientation;
    }
    else {

        OutputDebugString(L"MOSQUITO: Unable to read current rotation. Rotation disabled.");
        g_CurrentAngle = -1;
    }

    // Set up and register window class.

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

    // Create a window.

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
    // We never show the window, only set focus to it.
    SetFocus(hWnd);

    // Create the main DirectDraw object

    hRet = DirectDrawCreate(NULL, &g_pDD, NULL);
    if (hRet != DD_OK)
        return InitFail(hWnd, hRet, TEXT("DirectDrawCreate FAILED"));

    // Get normal mode.

    hRet = g_pDD->SetCooperativeLevel(hWnd, DDSCL_NORMAL);
    if (hRet != DD_OK)
        return InitFail(hWnd, hRet, TEXT("SetCooperativeLevel FAILED"));

    // Get a primary surface interface pointer (only needed for init.)

    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    hRet = g_pDD->CreateSurface(&ddsd, &g_pDDSPrimary, NULL);
    if (hRet != DD_OK)
        return InitFail(hWnd, hRet, TEXT("CreateSurface FAILED"));

    // See if we can support overlays.

    memset(&ddcaps, 0, sizeof(ddcaps));
    ddcaps.dwSize = sizeof(ddcaps);
    hRet = g_pDD->GetCaps(&ddcaps, NULL);
    if (hRet != DD_OK)
        return InitFail(hWnd, hRet, TEXT("GetCaps FAILED"));

    if (ddcaps.dwOverlayCaps == 0)
        return InitFail(hWnd, hRet, TEXT("Overlays are not supported in hardware!"));

    // Get alignment info to compute our overlay surface size.

    rs.left = 0;
    rs.top = 0;
    rs.right = BUG_WIDTH;
    rs.bottom = BUG_HEIGHT;
    if (ddcaps.dwAlignSizeSrc != 0)
	    rs.right += rs.right % ddcaps.dwAlignSizeSrc;
    
    // Create the overlay flipping surface. We will attempt the pixel formats
    // in our table one at a time until we find one that jives.

    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_FLIP;
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_BACKBUFFERCOUNT | 
                   DDSD_PIXELFORMAT;
	ddsd.dwWidth = rs.right;
	ddsd.dwHeight = rs.bottom;
    ddsd.dwBackBufferCount = 2;

    int i = 0;
    do {
	    ddsd.ddpfPixelFormat = ddpfOverlayFormats[i];
	    hRet = g_pDD->CreateSurface(&ddsd, &g_pDDSOverlay, NULL);
    } while (hRet != DD_OK && (++i < PF_TABLE_SIZE));

    if (hRet != DD_OK)
        return InitFail(hWnd, hRet, TEXT("Unable to create overlay surface!"));

    // Load the images.

    if (LoadBugImages() != DD_OK)
        return InitFail(hWnd, hRet, TEXT("Unable to load images to overlay surface!"));

    // Finish setting up the overlay.

    int StretchFactor1000 = ddcaps.dwMinOverlayStretch > 1000 ? ddcaps.dwMinOverlayStretch : 1000;

    rd.left=0; 
    rd.top=0;
    // Adding 999 takes care of integer truncation problems.
    rd.right  = (rs.right * StretchFactor1000 + 999) / 1000;
    rd.bottom = rs.bottom * StretchFactor1000 / 1000;
    if (ddcaps.dwAlignSizeDest != 0)
	    rd.right = (int)((rd.right + ddcaps.dwAlignSizeDest - 1)/ ddcaps.dwAlignSizeDest) *
                    ddcaps.dwAlignSizeDest;

    // Set the flags we'll send to UpdateOverlay
    dwUpdateFlags = DDOVER_SHOW;

    // Does the overlay hardware support source color keying?
    // If so, we can hide the black background around the image.
    // This probably won't work with YUV formats
    memset(&ovfx, 0, sizeof(ovfx));
    ovfx.dwSize = sizeof(ovfx);
    if (ddcaps.dwOverlayCaps & DDOVERLAYCAPS_CKEYSRC)
    {
        dwUpdateFlags |= DDOVER_KEYSRCOVERRIDE;

        // Create an overlay FX structure so we can specify a source color key.
        // This information is ignored if the DDOVER_SRCKEYOVERRIDE flag 
        // isn't set.
        ovfx.dckSrcColorkey.dwColorSpaceLowValue=0; // black as the color key
        ovfx.dckSrcColorkey.dwColorSpaceHighValue=0;
    }

    // Update the overlay parameters.

    hRet = g_pDDSOverlay->UpdateOverlay(&rs, g_pDDSPrimary, &rd, dwUpdateFlags, &ovfx);
    if (hRet != DD_OK)
        return InitFail(hWnd, hRet, TEXT("Unable to show overlay surface!"));

    // Set a bunch of position and velocity module vars.

    g_nOverlayXPos = 0;
    g_nOverlayYPos = 0;
    g_nOverlayXVel = RANDOM_VELOCITY();
    g_nOverlayYVel = RANDOM_VELOCITY();
    g_nOverlayWidth = rd.right - rd.left;
    g_nOverlayHeight = rd.bottom - rd.top;
    
    // Set the "destination position alignment" global so we won't have to
    // keep calling GetCaps() everytime we move the overlay surface.

    g_dwOverlayXPositionAlignment = ddcaps.dwAlignBoundaryDest;

    // Create a timer to flip the pages.

    if (TIMER_ID != SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL))
        return InitFail(hWnd, hRet, TEXT("SetTimer FAILED"));

    return DD_OK;
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
    MSG                         msg;

    g_hInstance = hInstance;

    if (InitApp(hInstance, nCmdShow) != DD_OK)
        return FALSE;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
