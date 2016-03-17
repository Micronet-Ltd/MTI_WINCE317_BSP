/******************************************************************************

 @File         OVGPaints.cpp

 @Title        OpenVG Paints using PVRShell

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     Independent

 @Description  Simple example showing paints in OpenVG

******************************************************************************/
#include "OVGTools.h"
#include "PVRShell.h"

/****************************************************************************
** Constants
****************************************************************************/

/****************************************************************************
** Class CPaints
****************************************************************************/
class CPaints : public PVRShell
{
private:
	VGPath m_vgPath;
	VGPaint m_vgColourPaint;
	VGPaint m_vgLinearGradientPaint;
	VGPaint m_vgRadialGradientPaint;

	CPVRTPrintVG m_PrintVg;

	int m_ui32StartTime;

public:
	CPaints() {}

	/* PVRShell functions */
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	/****************************************************************************
	** Function Definitions
	****************************************************************************/
	void CreatePath();
	void CreatePaints();
};

/*******************************************************************************
 * Function Name  : CreatePath
 * Description    : Creates a simple OpenVG path.
 *                  This function will be called by InitView once.
 *******************************************************************************/
void CPaints::CreatePath()
{
	static VGubyte aui8PathSegments[] = {
		VG_MOVE_TO_ABS,
		VG_CUBIC_TO_ABS,
		VG_SCUBIC_TO_ABS,
		VG_SCUBIC_TO_ABS,
		VG_SCUBIC_TO_ABS,
		VG_CLOSE_PATH,
	};
	static VGfloat afPathCoords[] = {
		0.3f, 0.1f,
		0.1f,-0.1f,		0.3f, 0.5f,		0.1f, 0.7f,
		0.5f, 0.7f,		0.7f, 0.9f,
		0.7f, 0.5f,		0.9f, 0.3f,
		0.5f, 0.3f,		0.3f, 0.1f,
	};

	// Create a path handle...
	m_vgPath = vgCreatePath(
		VG_PATH_FORMAT_STANDARD,
		VG_PATH_DATATYPE_F,
		1.0f, 0.0f,
		6,
		20,
		VG_PATH_CAPABILITY_APPEND_TO);

	// ... and populate it with data
	vgAppendPathData(m_vgPath, 6, aui8PathSegments, afPathCoords);

	/*
	Path capabilities should be removed when no longer needed. The OpenVG
	implementation might work more efficiently if it knows that path data
	will not change.
	*/
	vgRemovePathCapabilities(m_vgPath, VG_PATH_CAPABILITY_APPEND_TO);
}

/*******************************************************************************
 * Function Name  : CreatePaints
 * Description    : Creates colour, gradient, and pattern paints.
 *                  This function will be called by InitView once.
 *******************************************************************************/
void CPaints::CreatePaints()
{
	// Create a paint
	m_vgColourPaint = vgCreatePaint();
	// Set its type to colour
	vgSetParameteri(m_vgColourPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);

	// The easiest way to set the paint colour is to call vgSetColor.
	// The colour is given as non-premultiplied sRGBA.
	vgSetColor(m_vgColourPaint, PVRTRGBA(255,255,170,255));
	// As an alternative to vgSetColor, you could call this:
	// float afColour[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	// vgSetParameterfv(m_vgColourPaint, VG_PAINT_COLOR, 4, afColour);


	// Create another paint, now for linear gradient
	m_vgLinearGradientPaint = vgCreatePaint();
	vgSetParameteri(m_vgLinearGradientPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_LINEAR_GRADIENT);

	// A linear gradient needs start and end points that describe orientation
	// and length of the gradient.
	float afLinearGradientPoints[4] = {
		0.4f, 0.4f,
		0.6f, 0.6f
	};
	vgSetParameterfv(m_vgLinearGradientPaint, VG_PAINT_LINEAR_GRADIENT, 4, afLinearGradientPoints);

	// We set the colour ramp spread mode to reflect. This is very similar to
	// the OpenGL texture wrap mode MIRROR.
	vgSetParameteri(m_vgLinearGradientPaint, VG_PAINT_COLOR_RAMP_SPREAD_MODE, VG_COLOR_RAMP_SPREAD_REFLECT);

	// Now we have to specify the colour ramp. It is described by "stops" that
	// are given as premultiplied sRGBA colour at a position between 0 and 1.
	// Between these stops, the colour is linearly interpolated.
	// This colour ramp goes from red to green to blue, all opaque.
	float afColourRampStops[] = {
		0.0f,	1.0f, 0.0f, 0.0f, 1.0f,
		0.5f,	0.0f, 1.0f, 0.0f, 1.0f,
		1.0f,	0.0f, 0.0f, 1.0f, 1.0f,
	};
	vgSetParameterfv(m_vgLinearGradientPaint, VG_PAINT_COLOR_RAMP_STOPS, 15, afColourRampStops);


	// Create another paint, now for radial gradient
	m_vgRadialGradientPaint = vgCreatePaint();
	vgSetParameteri(m_vgRadialGradientPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_RADIAL_GRADIENT);

	// A radial gradient needs a center point, a focal point and a radius.
	// Center point and radius determine the outline of the circle where the
	// gradient "ends", while the focal point is where the gradient "starts".
	// The gradient colour at any given point is determined through linear
	// interpolation between the focal point and the closest point on the
	// circle.
	float afRadialGradientPoints[5] = {
		0.5f, 0.5f,
		0.6f, 0.6f,
		0.25f
	};
	vgSetParameterfv(m_vgRadialGradientPaint, VG_PAINT_RADIAL_GRADIENT, 5, afRadialGradientPoints);

	// Again, set spread mode to reflect
	vgSetParameteri(m_vgRadialGradientPaint, VG_PAINT_COLOR_RAMP_SPREAD_MODE, VG_COLOR_RAMP_SPREAD_REFLECT);

	// we can reuse the color ramp stops from the linear gradient here
	vgSetParameterfv(m_vgRadialGradientPaint, VG_PAINT_COLOR_RAMP_STOPS, 15, afColourRampStops);
}

/*******************************************************************************
 * Function Name  : InitApplication
 * Returns        : true if no error occured
 * Description    : Code in InitApplication() will be called by the Shell ONCE per
 *					run, early on in the execution of the program.
 *					Used to initialise variables that are not dependent on the
 *					rendering context (e.g. external modules, loading meshes, etc.)
 *******************************************************************************/
bool CPaints::InitApplication()
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
bool CPaints::QuitApplication()
{
	PVRShellOutputDebug("leaving...");
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
bool CPaints::InitView()
{
	/*
	Create path
	*/
	CreatePath();
	CreatePaints();

	// Scale the coordinate system to [0, 1] on both axes
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
	vgLoadIdentity();
	vgScale((float)PVRShellGet(prefWidth), (float)PVRShellGet(prefHeight));

	// Set stroke width
	vgSetf(VG_STROKE_LINE_WIDTH, 2.5f / PVRShellGet(prefHeight));

	/*
	The clear colour will be used whenever calling vgClear(). The colour is given
	as non-premultiplied sRGBA.
	*/
	VGfloat afClearColour[] = { 0.6f, 0.8f, 1.0f, 1.0f };
	vgSetfv(VG_CLEAR_COLOR, 4, afClearColour);

	/*
	Initialise custom text drawing
	*/
	m_PrintVg.Initialize(PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	m_ui32StartTime = PVRShellGetTime();

	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns		  : true if no error occured
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool CPaints::ReleaseView()
{
	vgDestroyPath(m_vgPath);
	vgDestroyPaint(m_vgColourPaint);
	vgDestroyPaint(m_vgLinearGradientPaint);
	vgDestroyPaint(m_vgRadialGradientPaint);

	m_PrintVg.Terminate();

	return true;
}


/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool CPaints::RenderScene()
{
	// Clear the screen with clear colour.
	vgClear(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	// we switch to the next paint every 4 seconds
	int i32CurrentPaint = ((PVRShellGetTime() - m_ui32StartTime)% 12000) / 4000;

	// Set fill paint
	switch(i32CurrentPaint)
	{
		case 0: vgSetPaint(m_vgColourPaint, VG_FILL_PATH); break;
		case 1: vgSetPaint(m_vgLinearGradientPaint, VG_FILL_PATH); break;
		case 2: vgSetPaint(m_vgRadialGradientPaint, VG_FILL_PATH); break;
	}

	// Draw the path with stroke and fill
	vgDrawPath(m_vgPath, VG_STROKE_PATH | VG_FILL_PATH);

	// Draw user interface
	static char* apszPaints[] = {
		"Single colour",
		"Linear gradient",
		"Radial gradient",
	};

	m_PrintVg.DisplayDefaultTitle("Paints", apszPaints[i32CurrentPaint], ePVRTPrint3DLogoIMG);
	return true;
}

/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialise a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new CPaints();
}

/*****************************************************************************
 End of file (OVGPaints.cpp)
*****************************************************************************/
