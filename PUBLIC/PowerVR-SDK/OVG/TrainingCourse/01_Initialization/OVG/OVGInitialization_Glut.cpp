/******************************************************************************

 @File         OVGInitialization_Glut.cpp

 @Title        OpenVG Intitalization Tutorial

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     .

 @Description  Basic Tutorial that shows step-by-step how to initialize OpenVG,
               use it for drawing a line, and terminate it.

******************************************************************************/
#include "vg/openvg.h"
#include "EGL/egl.h"
#include <gl/glut.h>

// No C++ Standard Library or exception handling on Symbian platform
#include <stdio.h>

/****************************************************************************
** Constants
****************************************************************************/
const char pszAppName[] = "OpenVG Intialization Tutorial";
int i32Frame = 0;
EGLDisplay			eglDisplay	= 0;
EGLSurface			eglSurface	= 0;
int i32WindowWidth = 240;
int i32WindowHeight= 320;
/****************************************************************************
** Declarations
****************************************************************************/

void GlutDisplayHandler()
{
	if(i32Frame > 1000)
		exit(0);

	// Initialize the clear color to opaque red
	VGfloat afClearColor[4];

	// Change clear clour between clear Blue and clear Yellow
	if (i32Frame&64)
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
		Drawing is double buffered, so you never see any intermediate
		results of the drawing. When you have finished drawing
		you have to call eglSwapBuffers to make the results appear on
		screen.
	*/

	eglSwapBuffers(eglDisplay, eglSurface);
	glutSwapBuffers();

	++i32Frame;
	glutPostRedisplay();
}

void CleanupOnExit()
{
	/*
		Step 10 - Terminate OpenVG

		eglTerminate takes care of destroying any context or surface created
		with this display, so we don't need to call eglDestroySurface or
		eglDestroyContext here.
	*/
	eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglTerminate(eglDisplay);
}
/****************************************************************************
** Application entry point
****************************************************************************/
int main(int argc, char *argv[])
{
	EGLConfig			eglConfig	= 0;
	EGLContext			eglContext	= 0;
	int i32NumConfigs, i32MajorVersion, i32MinorVersion;

	/*
	Step 0 - Create a window that we can use for OpenVG output

	Window creation is platform specific so it should be done by
	a separate function.
	*/

	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(240, 320);
	glutInitWindowPosition(100, 100);

    int m_glutWindow = glutCreateWindow("");

	if(!m_glutWindow)
	{
		return 1;
	}

	glutDisplayFunc(GlutDisplayHandler);
	atexit(CleanupOnExit);

	/*
	Step 1 - Get the default display

	EGL uses the concept of a "display" which in most environments
	corresponds to a single physical screen. Since we usually want
	to draw to the main screen or only have a single screen to begin
	with, we let EGL pick the default display.

	Querying other displays is platform specific.
	*/
	eglDisplay = (NativeDisplayType)eglGetDisplay(EGL_DEFAULT_DISPLAY);

	/*
	Step 2 - Initialize EGL

	EGL has to be initialized with the display obtained in the
	previous step. We cannot use other EGL functions except
	eglGetDisplay and eglGetError before eglInitialize has been
	called.
	If we're not interested in the EGL version number we can just
	pass NULL for the second and third parameters.
	*/
	if(!eglInitialize(eglDisplay, &i32MajorVersion, &i32MinorVersion))
	{
		printf("Error: eglInitialize() failed.\n");
		return 1;
	}

	/*
	Step 3 - Make OpenVG the current API

	EGL provides ways to set up OpenGL ES and OpenVG contexts
	(and possibly other graphics APIs in the future), so we need
	to specify the "current API".
	*/
	eglBindAPI(EGL_OPENVG_API);

	/*
	Step 4 - Specify the required configuration attributes

	An EGL "configuration" describes the pixel format and type of
	surfaces that can be used for drawing.
	For now we just want to use a 16 bit RGB surface that is a
	window surface, i.e. it will be visible on screen. The list
	has to contain key/value pairs, terminated with EGL_NONE.
	*/
	static const int ai32ConfigAttribs[] =
	{
		EGL_RED_SIZE,       5,
		EGL_GREEN_SIZE,     6,
		EGL_BLUE_SIZE,      5,
		EGL_ALPHA_SIZE,     0,
		EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT,
		EGL_NONE
	};

	/*
	Step 5 - Find a config that matches all requirements

	eglChooseConfig provides a list of all available configurations
	that meet or exceed the requirements given as the second
	argument. In most cases we just want the first config that meets
	all criteria, so we can limit the number of configs returned to 1.
	*/
	if(!eglChooseConfig(eglDisplay, ai32ConfigAttribs, &eglConfig, 1, &i32NumConfigs) || (i32NumConfigs != 1))
	{
		printf("Error: eglChooseConfig() failed.\n");
		exit(0);
	}

	/*
	Step 6 - Create a window surface to draw to

	Use the config picked in the previous step and the native window
	handle to create a window surface. A window surface is one that
	will be visible on screen inside the native window (or
	fullscreen if there is no window system).

	Pixmaps and pbuffers are surfaces which only exist in off-screen
	memory.
	*/
	eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig,(void*) m_glutWindow, NULL);
	if((eglGetError() != EGL_SUCCESS) || (eglSurface == EGL_NO_SURFACE))
	{
		printf("Error: eglCreateWindowSurface() failed.\n");
		exit(0);
	}

	/*
	Step 7 - Create a context

	EGL has to create a context for OpenVG. Our OpenVG resources
	like paths and images will only be valid inside this context
	(or shared contexts)
	*/
	eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, NULL);
	if((eglGetError() != EGL_SUCCESS) || (eglContext == EGL_NO_CONTEXT))
	{
		printf("Error: eglCreateContext() failed.\n");
		exit(0);
	}

	/*
	Step 8 - Bind the context to the current thread and use our
	window surface for drawing and reading

	Contexts are bound to a thread. This means you don't have to
	worry about other threads and processes interfering with your
	OpenVG application.
	We need to specify a surface that will be the target of all
	subsequent drawing operations, and one that will be the source
	of read operations. They can be the same surface.
	*/
	eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
	if(eglGetError() != EGL_SUCCESS)
	{
		printf("Error: eglMakeCurrent() failed.\n");
		exit(0);
	}

	/*
	Step 9 - Draw something with OpenVG

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

	eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH, &i32WindowWidth);
	eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &i32WindowHeight);

	glutMainLoop();

	// Say goodbye
	printf("%s finished.", pszAppName);
	return 0;
}

/*****************************************************************************
 End of file (Initialization.cpp)
*****************************************************************************/
