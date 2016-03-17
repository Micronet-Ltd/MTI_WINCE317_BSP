/******************************************************************************

 @File         OVGHelloTriangle_Glut.cpp

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

VGPath vgTriangle;
VGPaint vgFillPaint;
/****************************************************************************
** Declarations
****************************************************************************/

void GlutDisplayHandler()
{
	if(i32Frame > 1000)
		exit(0);

	// Clear the whole surface with the clear color
	vgClear(0, 0, i32WindowWidth, i32WindowHeight);

	// Set the current fill paint...
	vgSetPaint(vgFillPaint, VG_FILL_PATH);

	// Draw the triangle!
	vgDrawPath(vgTriangle, VG_FILL_PATH);

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
	Step 6 - Destroy resources
	--------------------------

	OpenVG resources like paths and paints need to be destroyed
	when they are no longer needed.
	*/
	vgDestroyPath(vgTriangle);
	vgDestroyPaint(vgFillPaint);

	// Terminate OpenVG
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

	/* Enter the main glut loop to render it.*/
	glutMainLoop();

	// Say goodbye
	printf("%s finished.", pszAppName);
	return 0;
}

/*****************************************************************************
 End of file (Initialization.cpp)
*****************************************************************************/
