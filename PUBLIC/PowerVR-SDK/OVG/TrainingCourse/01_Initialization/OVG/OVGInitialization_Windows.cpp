/******************************************************************************

 @File         OVGInitialization_Windows.cpp

 @Title        OpenVG Initialization Tutorial

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     .

 @Description  Basic Tutorial that shows step-by-step how to initialize OpenGL ES
               1.x, use it for clearing the screen with different colours and
               terminate it.

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

	// Initialize the clear color to opaque red
	VGfloat afClearColor[] = { 1, 0, 0, 1 };

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
	hWnd = CreateWindow( WINDOW_CLASS, _T("Initialization"), WS_VISIBLE | WS_SYSMENU,
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
		Step 1 - Get the default display.
		EGL uses the concept of a "display" which in most environments
		corresponds to a single physical screen. Since we usually want
		to draw to the main screen or only have a single screen to begin
		with, we let EGL pick the default display.
		Querying other displays is platform specific.
	*/
	eglDisplay = eglGetDisplay((NativeDisplayType)hDC);

    if(eglDisplay == EGL_NO_DISPLAY)
         eglDisplay = eglGetDisplay((NativeDisplayType) EGL_DEFAULT_DISPLAY);

    iErr = eglGetError();

	/*
		Step 2 - Initialize EGL.
		EGL has to be initialized with the display obtained in the
		previous step. We cannot use other EGL functions except
		eglGetDisplay and eglGetError before eglInitialize has been
		called.
		If we're not interested in the EGL version number we can just
		pass NULL for the second and third parameters.
	*/
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

	/*
		Step 3 - Specify the required configuration attributes.
		An EGL "configuration" describes the pixel format and type of
		surfaces that can be used for drawing.
		For now we just want to use a 16 bit RGB surface that is a
		Window surface, i.e. it will be visible on screen. The list
		has to contain key/value pairs, terminated with EGL_NONE.
	*/
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

	/*
		Step 4 - Find a config that matches all requirements.
		eglChooseConfig provides a list of all available configurations
		that meet or exceed the requirements given as the second
		argument. In most cases we just want the first config that meets
		all criteria, so we can limit the number of configs returned to 1.
	*/
	int iConfigs;
	if (!eglChooseConfig(eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &iConfigs) || (iConfigs != 1))
	{
#ifndef NO_GDI
		MessageBox(0, _T("eglChooseConfig() failed."), _T("Error"), MB_OK|MB_ICONEXCLAMATION);
#endif
		goto cleanup;
	}


	/*
		Step 6 - Create a context.
		EGL has to create a context for OpenGL ES. Our OpenGL ES resources
		like textures will only be valid inside this context
		(or shared contexts)
	*/
	eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, NULL);
	if (!TestEGLError(hWnd, "eglCreateContext"))
	{
		goto cleanup;
	}

	/*
		Step 5 - Create a surface to draw to.
		Use the config picked in the previous step and the native window
		handle when available to create a window surface. A window surface
		is one that will be visible on screen inside the native display (or
		fullscreen if there is no windowing system).
		Pixmaps and pbuffers are surfaces which only exist in off-screen
		memory.
	*/
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


	/*
		Step 7 - Bind the context to the current thread and use our
		window surface for drawing and reading.
		Contexts are bound to a thread. This means you don't have to
		worry about other threads and processes interfering with your
		OpenGL ES application.
		We need to specify a surface that will be the target of all
		subsequent drawing operations, and one that will be the source
		of read operations. They can be the same surface.
	*/
	eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
	if (!TestEGLError(hWnd, "eglMakeCurrent"))
	{
		goto cleanup;
	}

	/*
	Step 8 - Draw something with OpenVG

	At this point everything is initialized and we're ready to use
	OpenVG to draw something on the screen.

	Usually an application would create OpenVG resources like paths
	and images once and then call a draw function either in response
	to a repaint request by the window system or in an idle loop to
	facilitate smooth animation.

 	For demonstration purposes we only clear the surface
	1000 times while interpolating the clear color from red to blue.
	*/

	/*
	OpenVG's coordinate system is pixel based, so we first determine
	the size of the surface we're drawing to.

	Note that if the window was resized, EGL will automatically resize
	the window surface when you call eglSwapBuffers.
	So when you handle a resize event, it is a good idea to call
	eglSwapBuffers and update all variables that depend on the surface
	size.
	*/
	int i32WindowWidth, i32WindowHeight;
	eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH, &i32WindowWidth);
	eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &i32WindowHeight);

	// Clears the screen in different colours (640 frames)
	for(i = 0; i < 64*10; ++i)
	{
		// Check if the message handler finished the demo
		if (g_bDemoDone) break;

		// Change clear clour between clear Blue and clear Yellow
		if (i&64)
		{
			afClearColor[0] = 0.6f; // red
			afClearColor[1] = 0.8f; // green
			afClearColor[2] = 1.0f; // blue
			afClearColor[3] = 1.0f; // alpha
		}
		else
		{
			afClearColor[0] = 1.0f; // red
			afClearColor[1] = 1.0f; // green
			afClearColor[2] = 0.66f;// blue
			afClearColor[3] = 1.0f; // alpha
		}

		// Set the new clear color
		vgSetfv(VG_CLEAR_COLOR, 4, afClearColor);

		// Clear the whole surface with the clear color
		vgClear(0, 0, i32WindowWidth, i32WindowHeight);

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
 End of file (OGLESInitialization.cpp)
******************************************************************************/
