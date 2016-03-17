/******************************************************************************

 @File         OVGFillRule.cpp

 @Title        OpenVG Fill Rules using PVRShell

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     Independent

 @Description  Simple example showing fill rules in OpenVG

******************************************************************************/
#include "PVRShell.h"
#include "OVGTools.h"

/****************************************************************************
** Constants
****************************************************************************/

/****************************************************************************
** Class CFillRule
****************************************************************************/
class CFillRule : public PVRShell
{
private:
	VGPath m_vgPath;
	VGPaint m_vgPaint;

	CPVRTPrintVG m_PrintVg;

	unsigned int m_ui32StartTime;

public:
	CFillRule() {}

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
};

/*******************************************************************************
 * Function Name  : CreatePath
 * Description    : Creates an OpenVG path suited to demonstrate fill rules.
 *                  This function will be called by InitView once.
 *******************************************************************************/
void CFillRule::CreatePath()
{
	/*
	This path is designed to intersect itself multiple times, turning
	the winding around once. Winding is important for filling. One
	winding (clockwise or counter-clockwise) can be considered adding
	a stencil value of one to every closed shape, while the other 
	subtracts one. Then the	fill rule determines whether all closed 
	shapes with a value != 0 (VG_NONZERO) or all closed shapes with 
	an odd value (VG_EVEN_ODD) get filled.
	*/
	static VGubyte aui8PathSegments[] = {
		VG_MOVE_TO_ABS,
		VG_CUBIC_TO_ABS,
		VG_SCUBIC_TO_ABS,
		VG_SCUBIC_TO_ABS,
		VG_SCUBIC_TO_ABS,
		VG_SCUBIC_TO_ABS,
	};
	static VGfloat afPathCoords[] = {
		0.9f, 0.5f, 
		0.9f, 1.0f,		0.1f, 1.0f,		0.1f, 0.5f,
		0.8f, 0.9f,		0.8f, 0.5f,
		0.0f, 1.0f,		0.2f, 0.6f,
		0.4f, 0.8f,		0.2f, 0.4f,
		0.9f, 0.0f,		0.9f, 0.5f,
	};

	// Create a path handle...
	m_vgPath = vgCreatePath(
		VG_PATH_FORMAT_STANDARD, 
		VG_PATH_DATATYPE_F, 
		1.0f, 0.0f, 
		6,
		24,
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
 * Function Name  : InitApplication
 * Returns        : true if no error occured
 * Description    : Code in InitApplication() will be called by the Shell ONCE per
 *					run, early on in the execution of the program.
 *					Used to initialise variables that are not dependent on the
 *					rendering context (e.g. external modules, loading meshes, etc.)
 *******************************************************************************/
bool CFillRule::InitApplication()
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
bool CFillRule::QuitApplication()
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
bool CFillRule::InitView()
{
	/*
	Create path
	*/
	CreatePath();

	/*
	Create paint
	*/
	m_vgPaint = vgCreatePaint();
	vgSetParameteri(m_vgPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetColor(m_vgPaint, PVRTRGBA(255,255,170,255));

	// Scale the coordinate system to [0, 1] on both axes
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
	vgLoadIdentity();
	vgScale((float)PVRShellGet(prefWidth), (float)PVRShellGet(prefHeight));

	// Set line style to a dashed line with round caps & joins
	vgSeti(VG_STROKE_CAP_STYLE, VG_CAP_ROUND);
	vgSeti(VG_STROKE_JOIN_STYLE, VG_JOIN_ROUND);
	static float s_afDashes[] = { 0.02f, 0.025f };
	vgSetfv(VG_STROKE_DASH_PATTERN, 2, s_afDashes);
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
	m_PrintVg.Initialize(PVRShellGet(prefWidth), PVRShellGet(prefHeight),PVRShellGet(prefWidth) / 240.0f, 
		PVRShellGet(prefHeight) / 320.0f);

	m_ui32StartTime = PVRShellGetTime();

	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns		  : true if no error occured
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool CFillRule::ReleaseView()
{
	vgDestroyPath(m_vgPath);
	vgDestroyPaint(m_vgPaint);
	m_PrintVg.Terminate();

	return true;
}


/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool CFillRule::RenderScene()
{
	// Clear the screen with the clear colour.
	vgClear(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	unsigned int ui32TimeSinceStart = PVRShellGetTime() - m_ui32StartTime;
	// toggle fill rule every 4 seconds
	unsigned int ui32FillRule = ((ui32TimeSinceStart % 8000) / 4000);

	// Advance dash phase every frame to make winding apparent
	vgSetf(VG_STROKE_DASH_PHASE, 0.00002f * ui32TimeSinceStart);

	// Set fill paint
	vgSetPaint(m_vgPaint, VG_FILL_PATH);

	// Set fill rule
	if(ui32FillRule == 0)
		vgSeti(VG_FILL_RULE, VG_EVEN_ODD);
	else
		vgSeti(VG_FILL_RULE, VG_NON_ZERO);

	// Draw the self-intersecting path with stroke and fill
	vgDrawPath(m_vgPath, VG_STROKE_PATH | VG_FILL_PATH);

	// Draw user interface
	
	// Mark the stencil value of each closed shape
	m_PrintVg.DrawString(70.0f , 15.0f , 0.5f, "0", PVRTRGBA(0,0,0,255));
	m_PrintVg.DrawString(120.0f, 200.0f, 0.5f, "1", PVRTRGBA(0,0,0,255));
	m_PrintVg.DrawString(160.0f, 152.0f, 0.5f, "0", PVRTRGBA(0,0,0,255));
	m_PrintVg.DrawString(60.0f , 180.0f, 0.5f, "2", PVRTRGBA(0,0,0,255));
	m_PrintVg.DrawString(72.0f , 152.0f, 0.5f, "3", PVRTRGBA(0,0,0,255));

	static char* s_apszFillRules[] = { "Even/odd", "Non-zero" };
	m_PrintVg.DisplayDefaultTitle("FillRule", s_apszFillRules[ui32FillRule], ePVRTPrint3DLogoIMG);
	return true;
}

/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialise a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new CFillRule();
}

/*****************************************************************************
 End of file (OVGFillRule.cpp)
*****************************************************************************/
