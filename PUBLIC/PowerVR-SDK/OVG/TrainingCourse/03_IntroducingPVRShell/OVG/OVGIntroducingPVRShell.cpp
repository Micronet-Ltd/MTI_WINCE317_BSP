/******************************************************************************

 @File         OVGIntroducingPVRShell.cpp

 @Title        Hello Triangle for OVG, using the PowerVR Shell.

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     Independent

 @Description  OVG Hello triangle app, using the PowerVR Shell.

******************************************************************************/
#include "PVRShell.h"
#include "vg/openvg.h"


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

#define PVRTRGBA(r, g, b, a)   ((VGuint) (((r) << 24) | ((g) << 16) | ((b) << 8) | (a)))

/*!****************************************************************************
 To use the shell, you have to inherit a class from PVRShell
 and implement the five virtual functions which describe how your application
 initializes, runs and releases the ressources.
******************************************************************************/
class CIntroducingPVRShell : public PVRShell
{
public:
	CIntroducingPVRShell() {}

	VGPath m_vgPath;
	VGPaint m_vgFillPaint;

	/* PVRShell functions */
	bool InitApplication();
	bool QuitApplication();
	bool InitView();
	bool ReleaseView();
	bool RenderScene();

	/****************************************************************************
	** Function Definitions
	****************************************************************************/

	// This demo defines no additional methods
};

/****************************************************************************
** Demo class Implementation
****************************************************************************/

/****************************************************************************
** InitApplication() is called by PVRShell to enable user to initialise    **
** the application														   **
****************************************************************************/
bool CIntroducingPVRShell::InitApplication()
{
	// This sets up PVRShell to use an OpenVG context
	PVRShellSet(prefOpenVGContext, true);
	return true;
}

/****************************************************************************
** QuitApplication() is called by PVRShell to enable user to release      **
** any memory before quitting the program.								   **
****************************************************************************/
bool CIntroducingPVRShell::QuitApplication()
{
	///* Release any memory or instances allocated in InitApplication() */
	PVRShellOutputDebug("leaving...");
	return true;
}

/****************************************************************************
** InitView() is called by PVRShell each time a rendering variable is changed
** in the Shell menu (Z-Buffer On/Off, resolution change, buffering mode...)
** In this function one should initialise all variables that are dependant on
** general rendering variables (screen mode, 3D device, etc...)
****************************************************************************/
bool CIntroducingPVRShell::InitView()
{
	/*
	Initially, the OpenVG coordinate system is based on the output resolution.
	To get a device independent coordinate system, we need to apply a
	transformation. Scaling by the output resolution means that coordinates
	between (0, 0) and (1, 1) will be visible on screen.

	It should be noted, however, that different aspect ratios usually require
	special attention regarding the layout of elements on screen.
	*/
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
	vgLoadIdentity();
	vgScale((float)PVRShellGet(prefWidth), (float)PVRShellGet(prefHeight));

	/*
	Drawing shapes with OpenVG requires a path which represents a series of
	line and curve segments describing the outline of the shape. The shape
	does not need to be closed, but for now we will start with a simple
	triangle.
	First we create a path object, then we append segment and point data.
	*/
	m_vgPath = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
							1.0f, 0.0f, 4, 3, (unsigned int)VG_PATH_CAPABILITY_ALL);

	VGubyte aui8PathSegments[4] = {
		VG_MOVE_TO_ABS,
		VG_LINE_TO_ABS,
		VG_LINE_TO_ABS,
		VG_CLOSE_PATH,
	};
	VGfloat afPoints[6] = {
			0.3f, 0.3f,
			0.7f, 0.3f,
			0.5f, 0.7f,
	};
	vgAppendPathData(m_vgPath, 4, aui8PathSegments, afPoints);

	/*
	To fill a shape, we need a paint that describes how to fill it: a gradient,
	pattern, or single colour. Here we choose a simple opaque red.
	*/
	m_vgFillPaint = vgCreatePaint();
	vgSetParameteri(m_vgFillPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetColor(m_vgFillPaint, PVRTRGBA(255,255,170,255));

	/*
	The clear colour will be used whenever calling vgClear(). The colour is given
	as non-premultiplied sRGBA.
	*/
	VGfloat afClearColour[] = { 0.6f, 0.8f, 1.0f, 1.0f };
	vgSetfv(VG_CLEAR_COLOR, 4, afClearColour);

	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : true if no error occured
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool CIntroducingPVRShell::ReleaseView()
{
	// Cleanup: destroy OpenVG path and paint
	vgDestroyPath(m_vgPath);
	m_vgPath = 0;
	vgDestroyPaint(m_vgFillPaint);
	m_vgFillPaint = 0;

	return true;
}

/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool CIntroducingPVRShell::RenderScene()
{
	/*
	Clear the screen with clear colour. Clear is pixel based, so we need
	the dimensions of the screen.
	*/
	vgClear(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	// Set the current fill paint...
	vgSetPaint(m_vgFillPaint, VG_FILL_PATH);

	// ... and draw a filled triangle
	vgDrawPath(m_vgPath, VG_FILL_PATH);

	return true;
}

/*!***************************************************************************
 @Function		NewDemo
 @Return		The demo supplied by the user
 @Description	This function is required by the shell framework, and must
				return a class derived from PVRShell.
*****************************************************************************/
PVRShell* NewDemo()
{
	return new CIntroducingPVRShell;
}

/*****************************************************************************
 End of file (OVGIntroducingPVRShell.cpp)
*****************************************************************************/
