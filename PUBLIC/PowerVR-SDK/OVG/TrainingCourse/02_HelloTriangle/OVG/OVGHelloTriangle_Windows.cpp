/******************************************************************************

 @File         OVGHelloTriangle_Windows.cpp

 @Title        OpenVG HelloTriangle Tutorial

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     .

 @Description  

******************************************************************************/
#include <stdio.h>
#include <windows.h>
#include <TCHAR.H>

#include "vg/openvg.h"
#include "EGL/egl.h"

/******************************************************************************
 Defines
******************************************************************************/
//#define NO_GDI 1 /* Remove the GDI functions */

#ifndef NO_GDI
// Windows class name to register
#define	WINDOW_CLASS _T("PVRShellClass")

#endif

/******************************************************************************
 Global variables
******************************************************************************/

// Variable set in the message handler to finish the demo
bool	g_bDemoDone = false;

/*!****************************************************************************
 @Function		WndProc
 @Input			hWnd		Handle to the window
 @Input			message		Specifies the message
 @Input			wParam		Additional message information
 @Input			lParam		Additional message information
 @Return		LRESULT		result code to OS
 @Description	Processes messages for the main window
******************************************************************************/
#ifndef NO_GDI
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		// Handles the close message when a user clicks the quit icon of the window
		case WM_CLOSE:
			g_bDemoDone = true;
			PostQuitMessage(0);
			return 1;

		default:
			break;
	}

	// Calls the default window procedure for messages we did not handle
	return DefWindowProc(hWnd, message, wParam, lParam);
}
#endif
/*!****************************************************************************
 @Function		TestEGLError
 @Input			pszLocation		location in the program where the error took
								place. ie: function name
 @Return		bool			true if no EGL error was detected
 @Description	Tests for an EGL error and prints it
******************************************************************************/
bool TestEGLError(HWND hWnd, char* pszLocation)
{
	/*
		eglGetError returns the last error that has happened using egl,
		not the status of the last called function. The user has to
		check after every single egl call or at least once every frame.
	*/
	EGLint iErr = eglGetError();
	if (iErr != EGL_SUCCESS)
	{
#ifndef NO_GDI
		TCHAR pszStr[256];
		_stprintf(pszStr, _T("%s failed (%d).\n"), pszLocation, iErr);
		MessageBox(hWnd, pszStr, _T("Error"), MB_OK|MB_ICONEXCLAMATION);
#endif
		return false;
	}

	return true;
}

/*!****************************************************************************
 @Function		WinMain
 @Input			hInstance		Application instance from OS
 @Input			hPrevInstance	Always NULL
 @Input			lpCmdLine		command line from OS
 @Input			nCmdShow		Specifies how the window is to be shown
 @Return		int				result code to OS
 @Description	Main function of the program
******************************************************************************/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, TCHAR *lpCmdLine, int nCmdShow)
{
	int i;

	// Windows variables
	HWND				hWnd	= 0;
	HDC					hDC		= 0;

	// EGL variables
	EGLDisplay			eglDisplay	= 0;
	EGLConfig			eglConfig	= 0;
	EGLSurface			eglSurface	= 0;
	EGLContext			eglContext	= 0;
	NativeWindowType	eglWindow	= 0;
	EGLint				pi32ConfigAttribs[128];
	EGLint				iErr;

	/*
		Step 0 - Create a NativeWindowType that we can use for OpenGL ES output
	*/
#ifndef NO_GDI

	// Register the windows class
	WNDCLASS sWC;
    sWC.style = CS_HREDRAW | CS_VREDRAW;
	sWC.lpfnWndProc = WndProc;
    sWC.cbClsExtra = 0;
    sWC.cbWndExtra = 0;
    sWC.hInstance = hInstance;
    sWC.hIcon = 0;
    sWC.hCursor = 0;
    sWC.lpszMenuName = 0;
	sWC.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    sWC.lpszClassName = WINDOW_CLASS;
	ATOM registerClass = RegisterClass(&sWC);
	if (!registerClass)
	{
		MessageBox(0, _T("Failed to register the window class"), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	// Retrieve the display dimensions
	HMONITOR	hMonitor;
	POINT		p;
	MONITORINFO	sMInfo;

	p.x			= 0;
	p.y			= 0;
	hMonitor	= MonitorFromPoint(p, MONITOR_DEFAULTTOPRIMARY);
	sMInfo.cbSize = sizeof(sMInfo);
	BOOL bRet = GetMonitorInfo(hMonitor, &sMInfo);
	if(!bRet)
	{
		MessageBox(0, _T("Failed to get monitor info"), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	// Create the eglWindow
	RECT	sRect;
	SetRect(&sRect, 0, 0, sMInfo.rcMonitor.right - sMInfo.rcMonitor.left, sMInfo.rcMonitor.bottom - sMInfo.rcMonitor.top);
	AdjustWindowRectEx(&sRect, WS_CAPTION | WS_SYSMENU, false, 0);
	hWnd = CreateWindow( WINDOW_CLASS, _T("HelloTriangle"), WS_VISIBLE | WS_SYSMENU,
		0, 0,
		sMInfo.rcMonitor.right - sMInfo.rcMonitor.left,
		sMInfo.rcMonitor.bottom - sMInfo.rcMonitor.top, NULL, NULL, hInstance, NULL);
	eglWindow = hWnd;

	// Get the associated device context
	hDC = GetDC(hWnd);
	if (!hDC)
	{
		MessageBox(0, _T("Failed to create the device context"), _T("Error"), MB_OK|MB_ICONEXCLAMATION);
		goto cleanup;
	}
#endif
	/*
	Step 0 - Initialize OpenVG
	--------------------------

	The following code up to the next comment block consists of the
	steps 0 to 8 taken straight from the Initialization tutorial.
	*/
	eglDisplay = eglGetDisplay((NativeDisplayType) hDC);

    if(eglDisplay == EGL_NO_DISPLAY)
         eglDisplay = eglGetDisplay((NativeDisplayType) EGL_DEFAULT_DISPLAY);

    iErr = eglGetError();

	EGLint iMajorVersion, iMinorVersion;
	if (!eglInitialize(eglDisplay, &iMajorVersion, &iMinorVersion))
	{
#ifndef NO_GDI
		MessageBox(0, _T("eglInitialize() failed."), _T("Error"), MB_OK|MB_ICONEXCLAMATION);
#endif
		goto cleanup;
	}

	eglBindAPI(EGL_OPENVG_API);
	iErr = eglGetError();

	i = 0;
	pi32ConfigAttribs[i++] = EGL_RED_SIZE;
	pi32ConfigAttribs[i++] = 5;
	pi32ConfigAttribs[i++] = EGL_GREEN_SIZE;
	pi32ConfigAttribs[i++] = 6;
	pi32ConfigAttribs[i++] = EGL_BLUE_SIZE;
	pi32ConfigAttribs[i++] = 5;
	pi32ConfigAttribs[i++] = EGL_ALPHA_SIZE;
	pi32ConfigAttribs[i++] = 0;
	pi32ConfigAttribs[i++] = EGL_SURFACE_TYPE;
	pi32ConfigAttribs[i++] = EGL_WINDOW_BIT;
	pi32ConfigAttribs[i++] = EGL_RENDERABLE_TYPE;
	pi32ConfigAttribs[i++] = EGL_OPENVG_BIT;
	pi32ConfigAttribs[i++] = EGL_NONE;

	int iConfigs;
	if (!eglChooseConfig(eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &iConfigs) || (iConfigs != 1))
	{
#ifndef NO_GDI
		MessageBox(0, _T("eglChooseConfig() failed."), _T("Error"), MB_OK|MB_ICONEXCLAMATION);
#endif
		goto cleanup;
	}

	eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, NULL);
	if (!TestEGLError(hWnd, "eglCreateContext"))
	{
		goto cleanup;
	}

	eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, eglWindow, NULL);

    if(eglSurface == EGL_NO_SURFACE)
    {
        eglGetError(); // Clear error
        eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, NULL, NULL);
	}

    if (!TestEGLError(hWnd, "eglCreateWindowSurface"))
	{
		goto cleanup;
	}

	eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
	if (!TestEGLError(hWnd, "eglMakeCurrent"))
	{
		goto cleanup;
	}

	/*
	Steps 1 to 4 - Prepare OpenVG to draw a triangle
	------------------------------------------------

	At this point we could theoretically start drawing with OpenVG. But
	we have to specify what to draw and how to draw it first.
	*/

	/*
	Step 1 - Set up a device independent coordinate system
	------------------------------------------------------

	Initially, the OpenVG coordinate system is based on the output resolution.
	To get a device independent coordinate system, we need to apply a
	transformation: Scaling by the output resolution means that coordinates
	between (0, 0) and (1, 1) will be visible on screen, with the origin
	in the lower left corner.

	Transformations are described more in-depth in the Transforms tutorial.

	It should be noted that different aspect ratios often require
	special attention regarding the layout of elements on screen.
	*/

	int i32WindowWidth, i32WindowHeight;
	eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH, &i32WindowWidth);
	eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &i32WindowHeight);

	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
	vgLoadIdentity();
	vgScale((float)i32WindowWidth, (float)i32WindowHeight);


	/*
	Step 2 - Create a path
	----------------------
	Drawing shapes with OpenVG requires a path which represents a series of
	line and curve segments describing the outline of the shape. The shape
	does not need to be closed, but for now we will start with a simple
	triangle.
	First we create a path handle, then we append segment and point data.

	Creating a path involves choosing a datatype used for point data (we use
	float here, indicated by VG_PATH_DATATYPE_F) and capabilities that we want
	to use. Picking the right capabilities is important as the OpenVG driver
	can use a more efficient and compact internal representation for paths
	with limited capabilities. We only need two capabilities for this tutorial:
	adding data	to the path and drawing it, with the latter being implicitly
	enabled	for all paths.
	*/
	VGPath vgTriangle;
	vgTriangle = vgCreatePath(
							VG_PATH_FORMAT_STANDARD,
							VG_PATH_DATATYPE_F,
							1.0f, 0.0f, 4, 3,
							(unsigned int)VG_PATH_CAPABILITY_APPEND_TO);

	/*
	The segments of a path are described as a series of commands, represented as
	an array of bytes. You can imagine the commands being performed by a pen:
	First the pen moves to a starting location without drawing, from there it
	draws a line to a second point. Then another line to a third point. After
	that, it closes the shape by drawing a line from the last point to the
	starting location: triangle finished!

	The suffixes _ABS and _REL attached to the commands indicate whether the
	coordinates are to be interpreted as absolute locations (seen from the
	origin)or as being relative to the location of the current point.
	*/
	VGubyte aui8PathSegments[4];
	aui8PathSegments[0] = VG_MOVE_TO_ABS;
	aui8PathSegments[1] = VG_LINE_TO_ABS;
	aui8PathSegments[2] = VG_LINE_TO_ABS;
	aui8PathSegments[3] = VG_CLOSE_PATH;

	/*
	In addition to the array of commands, the path needs a list of points. A
	command can "consume" from 0 to 6 values, depending on its type. MOVE_TO
	and LINE_TO each take two values, CLOSE_PATH takes none.
	A triangle requires 3 2D vertices.
	*/
	VGfloat afPoints[6];
	afPoints[0] = 0.3f;
	afPoints[1] = 0.3f;
	afPoints[2] = 0.7f;
	afPoints[3] = 0.3f;
	afPoints[4] = 0.5f;
	afPoints[5] = 0.7f;

	/*
	When appending data to the path, only the number of segments needs to be
	specified since the number of points used depends on the actual commands.
	*/
	vgAppendPathData(vgTriangle, 4, aui8PathSegments, afPoints);

	/*
	Path capabilities should be removed as soon as they are no longer needed.
	The OpenVG implementation might work more efficiently if it knows that
	path data will not change since it can use an optimized internal
	representation.
	*/
	vgRemovePathCapabilities(vgTriangle, VG_PATH_CAPABILITY_APPEND_TO);


	/*
	Step 3 - Create a paint
	-----------------------
	To fill a shape, we need a paint that describes how to fill it: a gradient,
	pattern, or single color. Here we choose a paint with type COLOR that
	is a simple opaque red. vgSetColor is a shortcut function that takes a
	non-premultiplied sRGBA color encoded as a 32bit integer in RGBA_8888 form.
	*/
	VGPaint vgFillPaint;
	vgFillPaint = vgCreatePaint();
	vgSetParameteri(vgFillPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetColor(vgFillPaint, 0xFFFFAAFF);

	/*
	Step 4 - Prepare the render loop
	--------------------------------

	The clear color will be used whenever calling vgClear(). The color is given
	as non-premultiplied sRGBA, represented by four float values.
	*/
	VGfloat afClearColor[4];
	afClearColor[0] = 0.6f;
	afClearColor[1] = 0.8f;
	afClearColor[2] = 1.0f;
	afClearColor[3] = 1.0f;

	// Set the clear color
	vgSetfv(VG_CLEAR_COLOR, 4, afClearColor);

	/*
	Step 5 - Render loop
	--------------------
	*/

	for(i = 0; i < 64*10; ++i)
	{
		// Check if the message handler finished the demo
		if (g_bDemoDone) break;

		// Clear the whole surface with the clear color
		vgClear(0, 0, i32WindowWidth, i32WindowHeight);

		// Set the current fill paint...
		vgSetPaint(vgFillPaint, VG_FILL_PATH);

		// Draw the triangle!
		vgDrawPath(vgTriangle, VG_FILL_PATH);

		/*
			Swap Buffers.
			Brings to the native display the current render surface.
		*/
		eglSwapBuffers(eglDisplay, eglSurface);
		if (!TestEGLError(hWnd, "eglSwapBuffers"))
		{
			goto cleanup;
		}
#ifndef NO_GDI
		// Managing the window messages
		MSG msg;
		PeekMessage(&msg, hWnd, NULL, NULL, PM_REMOVE);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
#endif
	}

	/*
		Step 9 - Terminate OpenGL ES and destroy the window (if present).
		eglTerminate takes care of destroying any context or surface created
		with this display, so we don't need to call eglDestroySurface or
		eglDestroyContext here.
	*/

cleanup:
	eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglTerminate(eglDisplay);

	/*
		Step 10 - Destroy the eglWindow.
		Again, this is platform specific and delegated to a separate function.
	*/
#ifndef NO_GDI
	// Release the device context
	if (hDC) ReleaseDC(hWnd, hDC);

	// Destroy the eglWindow
	if (hWnd) DestroyWindow(hWnd);
#endif
	return 0;
}

/******************************************************************************
 End of file (OVGHelloTriangle.cpp)
******************************************************************************/
