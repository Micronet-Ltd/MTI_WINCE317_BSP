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
// File: Mosquito.cpp
//
// Desc: Mosquito is a DirectDraw sample application that demonstates the
//       use of video overlay. It creates a flipable overlay, loads a small
//       animation into the various back buffers, then flips the buffers as
//       it moves the overlay around the screen. Press F12 or ESC to quit.
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

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
void ReleaseAllObjects(void);
HRESULT InitFail(HWND, HRESULT, LPCTSTR, ...);
HRESULT RestoreAllSurfaces();
void MoveOverlay();
long FAR PASCAL WindowProc(HWND, UINT, WPARAM, LPARAM);
BOOL CopyBitmapToYUVSurface(LPDIRECTDRAWSURFACE, HBITMAP);
HRESULT WINAPI EnumSurfacesCallback(LPDIRECTDRAWSURFACE, LPDDSURFACEDESC, LPVOID);
HRESULT LoadBugImages();
HRESULT InitApp(HINSTANCE hInstance, int nCmdShow);


//-----------------------------------------------------------------------------
// Name: RestoreAllSurfaces
// Desc: Called in case we lose our surface's vram.
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Name: MoveOverlay()
// Desc: Called on the timer, this function moves the overlay around the
//       screen, periodically calling flip to animate the mosquito.
//-----------------------------------------------------------------------------



