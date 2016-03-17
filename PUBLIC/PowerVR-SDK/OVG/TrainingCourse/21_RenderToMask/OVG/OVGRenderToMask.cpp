/******************************************************************************

 @File         OVGRenderToMask.cpp

 @Title        OpenVG Masking using PVRShell

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     Independent

 @Description  Shows how to use masking in OpenVG

******************************************************************************/
#include "PVRShell.h"
#include "OVGTools.h"
#include <math.h>

/****************************************************************************
** Constants
****************************************************************************/

/****************************************************************************
** Class OVGRenderToMask
****************************************************************************/
class OVGRenderToMask : public PVRShell
{
private:
	VGPath m_avgPath[3];
	VGPaint m_avgColourPaint[3];
	CPVRTPrintVG m_PrintVG;

public:
	OVGRenderToMask() {}

	// PVRShell functions
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	/****************************************************************************
	** Function Definitions
	****************************************************************************/
	void CreatePath();
};

/*******************************************************************************
 * Function Name  : CreatePath
 * Description    : Creates two paths to be affected by the Mask
 *******************************************************************************/
void OVGRenderToMask::CreatePath()
{
	/*
	Create the two paths that will be used in the scene. The first is a triangle and
	the second is a square.
	*/

	m_avgPath[0] = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
							1.0f, 0.0f, 4, 3, (unsigned int)VG_PATH_CAPABILITY_ALL);

	VGubyte aui8PathSegments[4] = {
		VG_MOVE_TO,
		VG_LINE_TO,
		VG_LINE_TO,
		VG_CLOSE_PATH,
	};
	VGfloat afPoints[6] = {
			0.17f, 0.34f,
			0.83f, 0.34f,
			0.5f , 0.74f
	};

	vgAppendPathData(m_avgPath[0], 4, aui8PathSegments, afPoints);

	m_avgPath[1] = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
							1.0f, 0.0f, 5, 4, (unsigned int)VG_PATH_CAPABILITY_ALL);

	VGubyte aui8PathSegments2[5] = {
		VG_MOVE_TO,
		VG_LINE_TO,
		VG_LINE_TO,
		VG_LINE_TO,
		VG_CLOSE_PATH
	};

	VGfloat afPoints2[8] = {
			0.08f, 0.39f,
			0.92f, 0.39f,
			0.92f, 0.56f,
			0.08f, 0.56f
	};

	vgAppendPathData(m_avgPath[1], 5, aui8PathSegments2, afPoints2);

	m_avgPath[2] = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
							1.0f, 0.0f, 0, 0, (unsigned int)VG_PATH_CAPABILITY_ALL);

	vguEllipse(m_avgPath[2], 0.5f, 0.5f, 0.17f, 0.125f);
}
/*******************************************************************************
 * Function Name  : InitApplication
 * Returns        : true if no error occured
 * Description    : Code in InitApplication() will be called by the Shell ONCE per
 *					run, early on in the execution of the program.
 *					Used to initialise variables that are not dependent on the
 *					rendering context (e.g. external modules, loading meshes, etc.)
 *******************************************************************************/
bool OVGRenderToMask::InitApplication()
{
	// This sets up PVRShell to use an OpenVG context
	PVRShellSet(prefOpenVGContext, true);
	return true;
}

/*******************************************************************************
 * Function Name  : QuitApplication
 * Returns        : true if no error occured
 * Description    : Code in QuitApplication() will be called by the Shell ONCE per
 *					run, just before exiting the program.
 *******************************************************************************/
bool OVGRenderToMask::QuitApplication()
{
	return true;
}

/*******************************************************************************
 * Function Name  : InitView
 * Returns        : true if no error occured
 * Description    : Code in InitView() will be called by the Shell upon a change
 *					in the rendering context.
 *					Used to initialise variables that are dependent on the rendering
 *					context (e.g. textures, vertex buffers, etc.)
 *******************************************************************************/
bool OVGRenderToMask::InitView()
{
	//Create the paths so we have something to look at.
	CreatePath();

	// Setup the transformation to scale the paths to fit the screen
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
	vgLoadIdentity();

	vgScale((float) PVRShellGet(prefWidth ), (float) PVRShellGet(prefHeight));

	// Reduce the stroke size to compensate for our scaling
	vgSetf(VG_STROKE_LINE_WIDTH, 1.0f / PVRShellGet(prefHeight));

	//Set the render quality so the stroke borders have some form of anti-aliasing
	vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_BETTER);

	// Create the paints that the paths will use
	m_avgColourPaint[0] = vgCreatePaint();
	vgSetParameteri(m_avgColourPaint[0], VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetColor(m_avgColourPaint[0], PVRTRGBA(255,255,170,255));

	m_avgColourPaint[1] = vgCreatePaint();
	vgSetParameteri(m_avgColourPaint[1], VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetColor(m_avgColourPaint[1], PVRTRGBA(255,136,136, 255));

	m_avgColourPaint[2] = vgCreatePaint();
	vgSetParameteri(m_avgColourPaint[2], VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetColor(m_avgColourPaint[2], PVRTRGBA(0,0,0, 128));

	/*
		vgRenderToMask modifies the drawing surface's mask by applying
		the given operation to the region covered by the rendering of
		the given path.

		The second parameter determines the paint mode of the path. Also,
		the rendering of the path is affected by the current OpenVG state
		settings (e.g. scissor rects, stroke parameters etc.). However,
		paint settings (e.g., paint matrices) are ignored.

		The third parameter is the masking operation. Please see the
		OVGMasking training course for more detail.
	*/

	vgSetPaint(m_avgColourPaint[2], VG_FILL_PATH);
	vgRenderToMask(m_avgPath[2], VG_STROKE_PATH | VG_FILL_PATH, VG_SET_MASK);

	//Destroy the path as it is no longer needed.
	vgDestroyPath(m_avgPath[2]);

	// Initialise printVG
	m_PrintVG.Initialize(PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	//Create and set the clear colour
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
bool OVGRenderToMask::ReleaseView()
{
	// Cleanup: destroy the paths
	vgDestroyPath(m_avgPath[0]);
	vgDestroyPath(m_avgPath[1]);

	m_PrintVG.Terminate();
	return true;
}

/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool OVGRenderToMask::RenderScene()
{
	//Clear the screen with the current clear colour.
	vgClear(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	// Set the paint and draw the path
	vgSetPaint(m_avgColourPaint[1], VG_FILL_PATH);
	vgDrawPath(m_avgPath[1], VG_STROKE_PATH | VG_FILL_PATH);

	//Enable masking
	vgSeti(VG_MASKING, VG_TRUE);

	// Set the paint and draw the path
	vgSetPaint(m_avgColourPaint[0], VG_FILL_PATH);
	vgDrawPath(m_avgPath[0], VG_STROKE_PATH | VG_FILL_PATH);

	//Disable masking
	vgSeti(VG_MASKING, VG_FALSE);

	m_PrintVG.DisplayDefaultTitle("RenderToMask", "", ePVRTPrint3DSDKLogo);
	return true;
}

/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialise a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new OVGRenderToMask();
}

/*****************************************************************************
 End of file (OVGRenderToMask.cpp)
*****************************************************************************/
