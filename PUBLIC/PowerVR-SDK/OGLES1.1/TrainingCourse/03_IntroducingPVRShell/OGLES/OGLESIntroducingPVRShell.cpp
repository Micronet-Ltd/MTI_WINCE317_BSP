/******************************************************************************

 @File         OGLESIntroducingPVRShell.cpp

 @Title        Introduces PVRShell

 @Copyright    Copyright (C) 2000 - 2008 by Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to use the PowerVR framework for initialization. This
               framework allows platform abstraction so applications using it
               will work on any MBX enabled device.

******************************************************************************/
#include <GLES/egl.h>

#include "PVRShell.h"

	/*
	*	Lesson 3: The PowerVR Shell
	*	===========================

	The PowerVR shell handles all OS specific initialisation code, and is
	extremely convenient for writing portable applications. It also has
	several built in command line features, which allow you to specify
	attributes like the backbuffer size, vsync and antialiasing modes.

	The code is constructed around a "PVRShell" superclass. You must define
	your app using a class which inherits from this, which should implement
	the following five methods, (which at execution time are essentially
	called in the order in which they are listed):


	InitApplication:	This is called before any API initialisation has
	taken place, and can be used to set up any application data which does
	not require API calls, for example object positions, or arrays containing
	vertex data, before they are uploaded.

	InitView:	This is called after the API has initialised, and can be
	used to do any remaining initialisation which requires API functionality.
	In this app, it is used to upload the vertex data.

	RenderScene:	This is called repeatedly to draw the geometry. Returning
	false from this function instructs the app to enter the quit sequence:

	ReleaseView:	This function is called before the API is released, and
	is used to release any API resources. In this app, it releases the
	vertex buffer.

	QuitApplication:	This is called last of all, after the API has been
	released, and can be used to free any leftover user allocated memory.


	The shell framework starts the application by calling a "NewDemo" function,
	which must return an instance of the PVRShell class you defined. We will
	now use the shell to create a "Hello triangle" app, similar to the previous
	one.

	*/

/******************************************************************************
 Defines
******************************************************************************/

// Defines to abstract float/fixed data for Common/CommonLite profiles
#ifdef PVRT_FIXED_POINT_ENABLE
#define VERTTYPE		GLfixed
#define VERTTYPEENUM	GL_FIXED
#define f2vt(x)			((int)((x)*65536))
#define myglLoadMatrix	glLoadMatrixx
#define myglClearColor	glClearColorx
#else
#define VERTTYPE		GLfloat
#define VERTTYPEENUM	GL_FLOAT
#define f2vt(x)			(x)
#define myglLoadMatrix	glLoadMatrixf
#define myglClearColor	glClearColor
#endif

/*!****************************************************************************
 To use the shell, you have to inherit a class from PVRShell
 and implement the five virtual functions which describe how your application
 initializes, runs and releases the ressources.
******************************************************************************/
class OGLESIntroducingPVRShell : public PVRShell
{
public:
	// Vertex Buffer Object (VBO) handle
	GLuint	m_ui32Vbo;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();
};


/*!****************************************************************************
 @Function		InitApplication
 @Return		bool		true if no error occured
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependant on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLESIntroducingPVRShell::InitApplication()
{
	return true;
}

/*!****************************************************************************
 @Function		QuitApplication
 @Return		bool		true if no error occured
 @Description	Code in QuitApplication() will be called by PVRShell once per
				run, just before exiting the program.
				If the rendering context is lost, QuitApplication() will
				not be called.
******************************************************************************/
bool OGLESIntroducingPVRShell::QuitApplication()
{
    return true;
}

/*!****************************************************************************
 @Function		InitView
 @Return		bool		true if no error occured
 @Description	Code in InitView() will be called by PVRShell upon
				initialization or after a change in the rendering context.
				Used to initialize variables that are dependant on the rendering
				context (e.g. textures, vertex buffers, etc.)
******************************************************************************/
bool OGLESIntroducingPVRShell::InitView()
{
	// Sets the clear color
	myglClearColor(f2vt(0.6f), f2vt(0.8f), f2vt(1.0f), f2vt(1.0f));

	// Create VBO for the triangle from our data

	// Interleaved vertex data
	VERTTYPE afVertices[] = {	f2vt(-0.4f),f2vt(-0.4f),f2vt(0.0f), // Position
								f2vt(1.0f),f2vt(1.0f),f2vt(0.66f),f2vt(1.0f), // Colour
								f2vt(+0.4f),f2vt(-0.4f),f2vt(0.0f),
								f2vt(1.0f),f2vt(1.0f),f2vt(0.66f),f2vt(1.0f),
								f2vt(0.0f),f2vt(0.4f),f2vt(0.0f),
								f2vt(1.0f),f2vt(1.0f),f2vt(0.66f),f2vt(1.0f)};

	glGenBuffers(1, &m_ui32Vbo);

	unsigned int uiSize = 3 * (sizeof(VERTTYPE) * 7); // 3 vertices * stride (7 verttypes per vertex (3 pos + 4 colour))

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

	// Set the buffer's data
	glBufferData(GL_ARRAY_BUFFER, uiSize, afVertices, GL_STATIC_DRAW);

	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESIntroducingPVRShell::ReleaseView()
{
	return true;
}

/*!****************************************************************************
 @Function		RenderScene
 @Return		bool		true if no error occured
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				eglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important OS events.
				Will also manage relevent OS events. The user has access to
				these events through an abstraction layer provided by PVRShell.
******************************************************************************/
bool OGLESIntroducingPVRShell::RenderScene()
{
	/*
		Clears the color buffer.
	*/
	glClear(GL_COLOR_BUFFER_BIT);

	/*
		Draw a triangle
	*/

	// bind the VBO for the triangle
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

	// Enable vertex arrays
	glEnableClientState(GL_VERTEX_ARRAY);

	// Points to the vertex position data in the vertex buffer
	glVertexPointer(3,VERTTYPEENUM,sizeof(VERTTYPE) * 7, 0);

	// Set color data in the same way
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4,VERTTYPEENUM,sizeof(VERTTYPE) * 7, (unsigned char*) (sizeof(VERTTYPE) * 3));

	// Draws a non-indexed triangle array from the pointers previously given.
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// unbind the vertex buffer as we don't need it bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

/*!****************************************************************************
 @Function		NewDemo
 @Return		PVRShell*		The demo supplied by the user
 @Description	This function must be implemented by the user of the shell.
				The user should return its PVRShell object defining the
				behaviour of the application.
******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLESIntroducingPVRShell();
}

/******************************************************************************
 End of file (OGLESIntroducingPVRShell.cpp)
******************************************************************************/
