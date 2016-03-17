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
// File: ddutil.cpp
//
// Desc: Routines for loading bitmap and palettes from resources
//
//
//-----------------------------------------------------------------------------

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------
#include <windows.h>
#include <windowsx.h>
#include <ddraw.h>
#include "ddutil.h"

//-----------------------------------------------------------------------------
// Name: DDGetBitmapHandle
// Desc: Attempts to load bitmap from resource, then from disk. Returns
//       handle.
//-----------------------------------------------------------------------------
extern "C" HBITMAP
DDGetBitmapHandle(HINSTANCE hInstance, LPCTSTR szBitmap)
{
  HBITMAP hbm;

  hbm = (HBITMAP) LoadImage(hInstance, szBitmap, IMAGE_BITMAP, 0, 0, 0);
  if (hbm == NULL)
     hbm = (HBITMAP) LoadImage(NULL, szBitmap, IMAGE_BITMAP, 0, 0, 0);
  return (hbm);
}

//-----------------------------------------------------------------------------
// Name: DDLoadBitmap()
// Desc: Create a DirectDrawSurface from a bitmap resource.
//-----------------------------------------------------------------------------
extern "C" IDirectDrawSurface *
DDLoadBitmap(HINSTANCE hInstance, IDirectDraw * pdd, LPCTSTR szBitmap)
{
    HRESULT hr;
    HBITMAP hbm;
    BITMAP bm;
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
    // Get size of the bitmap
    //
    GetObject(hbm, sizeof(bm), &bm);

    //
    // Create a DirectDrawSurface for this bitmap
    //
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.dwWidth = bm.bmWidth;
    ddsd.dwHeight = bm.bmHeight;
    if (pdd->CreateSurface(&ddsd, &pdds, NULL) != DD_OK)
        return NULL;

    hr = DDCopyBitmap(pdds, hbm, 0, 0, 0, 0);
    if (hr != DD_OK) {
      OutputDebugString(TEXT("DDUTIL: Unable to copy bitmap bits.\n"));
      pdds->Release();
      pdds = NULL;
    }

    DeleteObject(hbm);
    return pdds;
}

//-----------------------------------------------------------------------------
// Name: DDReLoadBitmap()
// Desc: Load a bitmap from a file or resource into a directdraw surface.
//       normaly used to re-load a surface after a restore.
//-----------------------------------------------------------------------------
extern "C" HRESULT
DDReLoadBitmap(HINSTANCE hInstance, IDirectDrawSurface * pdds, LPCTSTR szBitmap)
{
    HBITMAP hbm;
    HRESULT hr;

    hbm = DDGetBitmapHandle(hInstance,szBitmap);
    if (hbm == NULL) {
      OutputDebugString(TEXT("DDUTIL: Unable to obtain handle to bitmap.\n"));
      return E_FAIL;
    }

    hr = DDCopyBitmap(pdds, hbm, 0, 0, 0, 0);
    if (hr != DD_OK) {
      OutputDebugString(TEXT("DDUTIL: Unable to copy bitmap bits.\n"));
    }
    
    DeleteObject(hbm);
    return hr;
}

//-----------------------------------------------------------------------------
// Name: DDCopyBitmap()
// Desc: Draw a bitmap into a DirectDrawSurface
//-----------------------------------------------------------------------------
extern "C" HRESULT
DDCopyBitmap(IDirectDrawSurface * pdds, HBITMAP hbm, int x, int y,
             int dx, int dy)
{
    HDC hdcImage;
    HDC hdc;
    BITMAP bm;
    DDSURFACEDESC ddsd;
    HRESULT hr;

    if (hbm == NULL || pdds == NULL)
        return E_FAIL;

    //
    // Select bitmap into a memoryDC so we can use it.
    //
    hdcImage = CreateCompatibleDC(NULL);
    if (!hdcImage) {
        OutputDebugString(TEXT("DDHEL: CreateCompatibleDC failed.\n"));
	    return E_FAIL;
    }
    SelectObject(hdcImage, hbm);

    //
    // Get size of the bitmap
    //
    GetObject(hbm, sizeof(bm), &bm);
    dx = dx == 0 ? bm.bmWidth : dx;     // Use the passed size, unless zero
    dy = dy == 0 ? bm.bmHeight : dy;

    //
    // Get size of surface.
    //
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
    pdds->GetSurfaceDesc(&ddsd);

    if ((hr = pdds->GetDC(&hdc)) == DD_OK)
    {
        if (!StretchBlt(hdc, 
			0, 0, 
			ddsd.dwWidth, 
			ddsd.dwHeight, 
			hdcImage, 
			x, y,
			dx, dy, 
			SRCCOPY)) hr = E_FAIL;
        pdds->ReleaseDC(hdc);
    }
    DeleteDC(hdcImage);
    return hr;
}

//-----------------------------------------------------------------------------
// Name: DDLoadPalette()
// Desc: Create a DirectDraw palette object from a bitmap resource
//       if the resource does not exist or NULL is passed create a
//       default 332 palette.
//-----------------------------------------------------------------------------
extern "C" IDirectDrawPalette *
DDLoadPalette(HINSTANCE hInstance, IDirectDraw * pdd, LPCTSTR szBitmap)
{
    IDirectDrawPalette * ddpal;
    int i;
    int n;
    HBITMAP hbm;
    DIBSECTION ds;
    PALETTEENTRY ape[256];
    RGBQUAD * prgb;
    
    //
    // Get a handle to the bitmap.
    //
    hbm = DDGetBitmapHandle(hInstance,szBitmap);
    if (hbm == NULL) {
      OutputDebugString(TEXT("DDUTIL: Unable to obtain handle to bitmap.\n"));
      return NULL;
    }

    //
    // Get the DIB Section associated with this bitmap.
    //
    
    GetObject(hbm, sizeof(ds), &ds);

    if (ds.dsBmih.biBitCount <= 8) {

      //
      // Bitmap should have a palette. Load it.
      //

      prgb = (RGBQUAD *) ((BYTE *) &(ds.dsBmih) + ds.dsBmih.biSize);
      if (ds.dsBmih.biClrUsed == 0) n = 1 << ds.dsBmih.biBitCount;
      else n = ds.dsBmih.biClrUsed;
      if(n > 256) n = 256; // avoid array overflow

      for (i = 0; i < n; i++) {
	    ape[i].peRed = prgb[i].rgbRed;
        ape[i].peGreen = prgb[i].rgbGreen;
        ape[i].peBlue = prgb[i].rgbBlue;
        ape[i].peFlags = (BYTE) 0;
      }

    }
    else {

      //
      // Build a 332 palette as the default.
      //
      for (i = 0; i < 256; i++) {
	    ape[i].peRed = (BYTE) (((i >> 5) & 0x07) * 255 / 7);
        ape[i].peGreen = (BYTE) (((i >> 2) & 0x07) * 255 / 7);
        ape[i].peBlue = (BYTE) (((i >> 0) & 0x03) * 255 / 3);
        ape[i].peFlags = (BYTE) 0;
      }
    }

    pdd->CreatePalette(0, ape, &ddpal, NULL);
    DeleteObject(hbm);
    return ddpal;
}

//-----------------------------------------------------------------------------
// Name: DDColorMatch()
// Desc: Convert a RGB color to a pysical color.
//       We do this by leting GDI SetPixel() do the color matching
//       then we lock the memory and see what it got mapped to.
//-----------------------------------------------------------------------------
extern "C" DWORD
DDColorMatch(IDirectDrawSurface * pdds, COLORREF rgb)
{
    COLORREF                rgbT;
    HDC                     hdc;
    DWORD                   dw = CLR_INVALID;
    DDSURFACEDESC           ddsd;
    HRESULT                 hres;

    //
    //  Use GDI SetPixel to color match for us
    //
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        rgbT = GetPixel(hdc, 0, 0);     // Save current pixel value
        SetPixel(hdc, 0, 0, rgb);       // Set our value
        pdds->ReleaseDC(hdc);
    }
    else
    {
        return dw;
    }
    //
    // Now lock the surface so we can read back the converted color
    //
    ddsd.dwSize = sizeof(ddsd);
    while ((hres = pdds->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)
        ;
    if (hres == DD_OK)
    {
        dw = *(DWORD *) ddsd.lpSurface;                 // Get DWORD
        if (ddsd.ddpfPixelFormat.dwRGBBitCount < 32)
            dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount) - 1;  // Mask it to bpp
        pdds->Unlock(NULL);
    }
    //
    //  Now put the color that was there back.
    //
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        SetPixel(hdc, 0, 0, rgbT);
        pdds->ReleaseDC(hdc);
    }
    return dw;
}

//-----------------------------------------------------------------------------
// Name: DDSetColorKey()
// Desc: Set a color key for a surface, given a RGB.
//       If you pass CLR_INVALID as the color key, the pixel
//       in the upper-left corner will be used.
//-----------------------------------------------------------------------------
extern "C" HRESULT
DDSetColorKey2(IDirectDrawSurface * pdds, COLORREF rgb)
{
    DDCOLORKEY ddck;

    ddck.dwColorSpaceLowValue = DDColorMatch(pdds, rgb);
    ddck.dwColorSpaceHighValue = ddck.dwColorSpaceLowValue;
    return pdds->SetColorKey(DDCKEY_SRCBLT, &ddck);
}

extern "C" HRESULT
DDSetColorKey(IDirectDrawSurface * pdds, COLORREF rgb)
{
    DDCOLORKEY ddck;

    ddck.dwColorSpaceLowValue = 0;
    ddck.dwColorSpaceHighValue = ddck.dwColorSpaceLowValue;
    return pdds->SetColorKey(DDCKEY_SRCBLT, &ddck);
}
