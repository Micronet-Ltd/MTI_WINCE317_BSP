/******************************************************************************

 @File         OVGTransforms.cpp

 @Title        OpenVG Transformations using PVRShell

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     Independent

 @Description  Simple example showing path user-to-surface transformations in
               OpenVG

******************************************************************************/
#include "PVRShell.h"
#include "OVGTools.h"

/****************************************************************************
** Constants
****************************************************************************/

/****************************************************************************
** Class CTransforms
****************************************************************************/
class CTransforms : public PVRShell
{
private:
	VGPath  m_avgPaths[2];
	VGPaint m_vgPaint;

	CPVRTPrintVG m_PrintVG;

	unsigned int m_ui32StartTime;
	unsigned int m_ui32AbsTime;

public:
	CTransforms() {}

	/* PVRShell functions */
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	/****************************************************************************
	** Function Definitions
	****************************************************************************/
	void CreatePaths();

	void DoTranslate();
	void DoScaleCentered();
	void DoScaleOrigin();
	void DoRotateCentered();
	void DoRotateOrigin();
	void DoShearCentered();
	void DoShearOrigin();
};

/*******************************************************************************
 * Function Name  : DoTranslate
 * Description    : Demonstrate the effect of translations. Each path is
 *                  translated separately
 *******************************************************************************/
void CTransforms::DoTranslate()
{
	// Make sure we're operating on the path user-to-surface matrix
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);

	// Load Identity matrix. This clears all previous transformations
	vgLoadIdentity();

	// To be independent of screen resolution, we need to scale the
	// coordinates so everything in the range [0, 1] will be visible
	vgScale((float) PVRShellGet(prefWidth), (float) PVRShellGet(prefHeight));

	// Unlike OpenGL, OpenVG does not maintain a matrix stack. So instead of
	// pushing the current matrix to the stack, we have to store it ourselves
	float afUnitMatrix[3 * 3];
	vgGetMatrix(afUnitMatrix);

	// turn time(ms) into a clipped periodic triangle function
	int i32Zigzag1 = m_ui32AbsTime % 2000;

	if(i32Zigzag1 > 1000)
		i32Zigzag1 = 2000 - i32Zigzag1;

	i32Zigzag1 = PVRT_MAX(250, PVRT_MIN(750, i32Zigzag1)) - 500;

	// and again, now with shifted phase
	int i32Zigzag2 = (m_ui32AbsTime + 500) % 2000;

	if(i32Zigzag2 > 1000)
		i32Zigzag2 = 2000 - i32Zigzag2;

	i32Zigzag2 = PVRT_MAX(250, PVRT_MIN(750, i32Zigzag2)) - 250;

	// translation for first path
	vgTranslate(-0.001f * i32Zigzag1, -0.001f * i32Zigzag2);

	// draw first path
	vgDrawPath(m_avgPaths[0], VG_STROKE_PATH | VG_FILL_PATH);

	// restore the unit matrix ([0, 1] visible)
	vgLoadMatrix(afUnitMatrix);

	// translation for second path
	vgTranslate(0.001f * i32Zigzag1, 0.001f * i32Zigzag2);

	// draw second path
	vgDrawPath(m_avgPaths[1], VG_STROKE_PATH | VG_FILL_PATH);
}

/*******************************************************************************
 * Function Name  : DoScaleCentered
 * Description    : Demonstrate the effect of scaling from the centre of a
 *                  shape. Each path is transformed separately.
 *******************************************************************************/
void CTransforms::DoScaleCentered()
{
	// Make sure we're operating on the path user-to-surface matrix
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);

	// Load Identity matrix. This clears all previous transformations
	vgLoadIdentity();

	// To be independent of screen resolution, we need to scale the
	// coordinates so everything in the range [0, 1] will be visible
	vgScale((float) PVRShellGet(prefWidth), (float) PVRShellGet(prefHeight));

	// Unlike OpenGL, OpenVG does not maintain a matrix stack. So instead of
	// pushing the current matrix to the stack, we have to store it ourselves
	float afUnitMatrix[3*3];
	vgGetMatrix(afUnitMatrix);

	// turn time(ms) into a periodic triangle function
	int i32Zigzag = m_ui32AbsTime % 2000;

	if(i32Zigzag > 1000)
		i32Zigzag = 2000 - i32Zigzag;

	float fScaleFactor = 0.5f + (i32Zigzag * 0.001f);

	// Scaling a shape from its center is identical to moving it to the
	// origin, scaling it there, and moving it back where it was.
	//
	// IMPORTANT:
	// Since OpenVG right-multiplies matrices, you conceptually need to
	// call the transformation functions in backwards order.
	vgTranslate(0.5f, 0.75f);
	vgScale(fScaleFactor, fScaleFactor);
	vgTranslate(-0.5f, -0.75f);

	// draw first path
	vgDrawPath(m_avgPaths[0], VG_STROKE_PATH | VG_FILL_PATH);

	// restore the unit matrix ([0, 1] visible)
	vgLoadMatrix(afUnitMatrix);

	// transformation for second path
	fScaleFactor = 2 - fScaleFactor;
	vgTranslate(0.5f, 0.25f);
	vgScale(fScaleFactor, fScaleFactor);
	vgTranslate(-0.5f, -0.25f);

	// draw second path
	vgDrawPath(m_avgPaths[1], VG_STROKE_PATH | VG_FILL_PATH);
}

/*******************************************************************************
 * Function Name  : DoScaleOrigin
 * Description    : Demonstrate the effect of scaling from the origin.
 *******************************************************************************/
void CTransforms::DoScaleOrigin()
{
	// Make sure we're operating on the path user-to-surface matrix
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);

	// Load Identity matrix. This clears all previous transformations
	vgLoadIdentity();

	// To be independent of screen resolution, we need to scale the
	// coordinates so everything in the range [0, 1] will be visible
	vgScale((float) PVRShellGet(prefWidth), (float) PVRShellGet(prefHeight));

	// turn time(ms) into a periodic triangle function
	int i32Zigzag = m_ui32AbsTime % 2000;

	if(i32Zigzag > 1000)
		i32Zigzag = 2000 - i32Zigzag;

	i32Zigzag = PVRT_MAX(100, PVRT_MIN(900, i32Zigzag));

	float fScaleFactor = 0.3f + (i32Zigzag * 0.0009f);

	// Scaling a scene from the origin means that objects will
	// either get pulled towards the origin or move away from it
	// along with being resized.
	vgScale(fScaleFactor, fScaleFactor);

	// draw first path
	vgDrawPath(m_avgPaths[0], VG_STROKE_PATH | VG_FILL_PATH);
	// draw second path
	vgDrawPath(m_avgPaths[1], VG_STROKE_PATH | VG_FILL_PATH);
}

/*******************************************************************************
 * Function Name  : DoRotateCentered
 * Description    : Demonstrate the effect of rotating around the centre of a
 *                  shape. Each path is transformed separately.
 *******************************************************************************/
void CTransforms::DoRotateCentered()
{
	// Make sure we're operating on the path user-to-surface matrix
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);

	// Load Identity matrix. This clears all previous transformations
	vgLoadIdentity();

	// To be independent of screen resolution, we need to scale the
	// coordinates so everything in the range [0, 1] will be visible
	vgScale((float)PVRShellGet(prefWidth), (float)PVRShellGet(prefHeight));

	// Unlike OpenGL, OpenVG does not maintain a matrix stack. So instead of
	// pushing the current matrix to the stack, we have to store it ourselves
	float afUnitMatrix[3*3];
	vgGetMatrix(afUnitMatrix);

	float fRotationAngle = m_ui32AbsTime * 0.03f;

	// Scaling a shape from its center is identical to moving it to the
	// origin, scaling it there, and moving it back where it was.
	//
	// IMPORTANT:
	// Since OpenVG right-multiplies matrices, you conceptually need to
	// call the transformation functions in backwards order.
	vgTranslate(0.5f, 0.75f);
	vgRotate(fRotationAngle);
	vgTranslate(-0.5f, -0.75f);

	// draw first path
	vgDrawPath(m_avgPaths[0], VG_STROKE_PATH | VG_FILL_PATH);

	// restore the unit matrix ([0, 1] visible)
	vgLoadMatrix(afUnitMatrix);

	// transformation for second path
	vgTranslate(0.5f, 0.25f);
	vgRotate(-fRotationAngle);
	vgTranslate(-0.5f, -0.25f);

	// draw second path
	vgDrawPath(m_avgPaths[1], VG_STROKE_PATH | VG_FILL_PATH);
}

/*******************************************************************************
 * Function Name  : DoRotateOrigin
 * Description    : Demonstrate the effect of rotating around the origin.
 *******************************************************************************/
void CTransforms::DoRotateOrigin()
{
	// Make sure we're operating on the path user-to-surface matrix
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);

	// Load Identity matrix. This clears all previous transformations
	vgLoadIdentity();

	// To be independent of screen resolution, we need to scale the
	// coordinates so everything in the range [0, 1] will be visible
	vgScale((float)PVRShellGet(prefWidth), (float)PVRShellGet(prefHeight));

	// turn time(ms) into a periodic triangle function
	int i32Zigzag = m_ui32AbsTime % 2000;

	if(i32Zigzag > 1000)
		i32Zigzag = 2000 - i32Zigzag;

	i32Zigzag = PVRT_MAX(100, PVRT_MIN(900, i32Zigzag));

	vgRotate((i32Zigzag - 350) / 15.0f);

	// draw first path
	vgDrawPath(m_avgPaths[0], VG_STROKE_PATH | VG_FILL_PATH);
	// draw second path
	vgDrawPath(m_avgPaths[1], VG_STROKE_PATH | VG_FILL_PATH);
}

/*******************************************************************************
 * Function Name  : DoShearCentered
 * Description    : Demonstrate the effect of shearing centred on a
 *                  shape. Each path is transformed separately.
 *******************************************************************************/
void CTransforms::DoShearCentered()
{
	// Make sure we're operating on the path user-to-surface matrix
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);

	// Load Identity matrix. This clears all previous transformations
	vgLoadIdentity();

	// To be independent of screen resolution, we need to scale the
	// coordinates so everything in the range [0, 1] will be visible
	vgScale((float)PVRShellGet(prefWidth), (float)PVRShellGet(prefHeight));

	// Unlike OpenGL, OpenVG does not maintain a matrix stack. So instead of
	// pushing the current matrix to the stack, we have to store it ourselves
	float afUnitMatrix[3 * 3];
	vgGetMatrix(afUnitMatrix);

	// turn time(ms) into a clipped periodic triangle function
	int i32Zigzag1 = m_ui32AbsTime % 2000;

	if(i32Zigzag1 > 1000)
		i32Zigzag1 = 2000 - i32Zigzag1;

	i32Zigzag1 = PVRT_MAX(250, PVRT_MIN(750, i32Zigzag1)) - 500;

	// and again, now with shifted phase
	int i32Zigzag2 = (m_ui32AbsTime + 500) % 2000;

	if(i32Zigzag2 > 1000)
		i32Zigzag2 = 2000 - i32Zigzag2;

	i32Zigzag2 = PVRT_MAX(250, PVRT_MIN(750, i32Zigzag2)) - 500;

	// Scaling a shape from its center is identical to moving it to the
	// origin, scaling it there, and moving it back where it was.
	//
	// IMPORTANT:
	// Since OpenVG right-multiplies matrices, you conceptually need to
	// call the transformation functions in backwards order.
	vgTranslate(0.5f, 0.75f);
	vgShear(0.001f * i32Zigzag1, 0);
	vgTranslate(-0.5f, -0.75f);

	// draw first path
	vgDrawPath(m_avgPaths[0], VG_STROKE_PATH | VG_FILL_PATH);

	// restore the unit matrix ([0, 1] visible)
	vgLoadMatrix(afUnitMatrix);

	// transformation for second path
	vgTranslate(0.5f, 0.25f);
	vgShear(0.001f * i32Zigzag2, 0);
	vgTranslate(-0.5f, -0.25f);

	// draw second path
	vgDrawPath(m_avgPaths[1], VG_STROKE_PATH | VG_FILL_PATH);
}

/*******************************************************************************
 * Function Name  : DoShearOrigin
 * Description    : Demonstrate the effect of shearing from the origin.
 *******************************************************************************/
void CTransforms::DoShearOrigin()
{
	// Make sure we're operating on the path user-to-surface matrix
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);

	// Load Identity matrix. This clears all previous transformations
	vgLoadIdentity();

	// To be independent of screen resolution, we need to scale the
	// coordinates so everything in the range [0, 1] will be visible
	vgScale((float)PVRShellGet(prefWidth), (float)PVRShellGet(prefHeight));

	// turn time(ms) into a periodic triangle function
	int i32Zigzag = m_ui32AbsTime % 2000;

	if(i32Zigzag > 1000)
		i32Zigzag = 2000 - i32Zigzag;

	i32Zigzag = PVRT_MAX(100, PVRT_MIN(900, i32Zigzag)) - 500;

	vgShear(i32Zigzag * 0.001f, 0);

	// draw first path
	vgDrawPath(m_avgPaths[0], VG_STROKE_PATH | VG_FILL_PATH);
	// draw second path
	vgDrawPath(m_avgPaths[1], VG_STROKE_PATH | VG_FILL_PATH);
}

/*******************************************************************************
 * Function Name  : CreatePaths
 * Description    : Create two simple OpenVG paths. One is centred around
 *                  (0.5, 0.25), the other around (0.5, 0.75).
 *                  This function will be called by InitView once.
 *******************************************************************************/
void CTransforms::CreatePaths()
{
	/*
	This is also a simple demonstration of relative path segments. Since
	we want both shapes to be identical, just in different locations, we
	can use the same data for both and just skip the first MOVE_TO_ABS
	when creating the second shape.
	*/
	static VGubyte aui8PathSegments[] = {
		VG_MOVE_TO_ABS,
		VG_MOVE_TO_REL,
		VG_VLINE_TO_REL,
		VG_HLINE_TO_REL,
		VG_VLINE_TO_REL,
		VG_CLOSE_PATH
	};

	static VGfloat afPathCoords[] = {
		0.0f, 0.5f,
		0.4f, 0.15f,
		0.2f,
		0.2f,
		-0.2f,
	};

	// Create a path handle...
	m_avgPaths[0] = vgCreatePath(
		VG_PATH_FORMAT_STANDARD,
		VG_PATH_DATATYPE_F,
		1.0f, 0.0f,
		6,
		7,
		VG_PATH_CAPABILITY_APPEND_TO);

	// ... and populate it with data
	vgAppendPathData(m_avgPaths[0], 6, aui8PathSegments, afPathCoords);

	// Do it a second time...
	m_avgPaths[1] = vgCreatePath(
		VG_PATH_FORMAT_STANDARD,
		VG_PATH_DATATYPE_F,
		1.0f, 0.0f,
		5,
		7,
		VG_PATH_CAPABILITY_APPEND_TO);

	// ... but skip the first MOVE_TO_ABS segment, effectively
	// starting at the origin
	vgAppendPathData(m_avgPaths[1], 5, &aui8PathSegments[1], &afPathCoords[2]);


	/*
	Path capabilities should be removed when no longer needed. The OpenVG
	implementation might work more efficiently if it knows that path data
	will not change.
	*/
	vgRemovePathCapabilities(m_avgPaths[0], VG_PATH_CAPABILITY_APPEND_TO);
	vgRemovePathCapabilities(m_avgPaths[1], VG_PATH_CAPABILITY_APPEND_TO);
}

/*******************************************************************************
 * Function Name  : InitApplication
 * Inputs		  : argc, *argv[], uWidth, uHeight
 * Returns        : true if no error occured
 * Description    : Code in InitApplication() will be called by the Shell ONCE per
 *					run, early on in the execution of the program.
 *					Used to initialise variables that are not dependent on the
 *					rendering context (e.g. external modules, loading meshes, etc.)
 *******************************************************************************/
bool CTransforms::InitApplication()
{
	// This sets up PVRShell to use an OpenVG context
	PVRShellSet(prefOpenVGContext, true);

	// Set the AA mode to best
	PVRShellSet(prefFSAAMode, 2);
	return true;
}

/*******************************************************************************
 * Function Name  : QuitApplication
 * Returns        : true if no error occured
 * Description    : Code in QuitApplication() will be called by the Shell ONCE per
 *					run, just before exiting the program.
 *******************************************************************************/
bool CTransforms::QuitApplication()
{
	PVRShellOutputDebug("leaving...");
	return true;
}

/*******************************************************************************
 * Function Name  : InitView
 * Inputs		  : uWidth, uHeight
 * Returns        : true if no error occured
 * Description    : Code in InitView() will be called by the Shell upon a change
 *					in the rendering context.
 *					Used to initialise variables that are dependent on the rendering
 *					context (e.g. textures, vertex buffers, etc.)
 *******************************************************************************/
bool CTransforms::InitView()
{
	// Create paths
	CreatePaths();

	// Create paint
	m_vgPaint = vgCreatePaint();
	vgSetParameteri(m_vgPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetColor(m_vgPaint, PVRTRGBA(255, 255, 170, 255));

	vgSeti(VG_STROKE_JOIN_STYLE, VG_JOIN_ROUND);
	vgSetf(VG_STROKE_LINE_WIDTH, 2.5f / PVRShellGet(prefHeight));

	/*
	The clear colour will be used whenever calling vgClear(). The colour is given
	as non-premultiplied sRGBA.
	*/
	VGfloat afClearColour[] = { 0.6f, 0.8f, 1.0f, 1.0f };
	vgSetfv(VG_CLEAR_COLOR, 4, afClearColour);

	// Initialise custom text drawing
	m_PrintVG.Initialize(PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	m_ui32StartTime = PVRShellGetTime();

	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : Nothing
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool CTransforms::ReleaseView()
{
	// Cleanup: destroy paths and paint
	vgDestroyPath(m_avgPaths[0]);
	vgDestroyPath(m_avgPaths[1]);
	vgDestroyPaint(m_vgPaint);
	// Cleanup for custom text drawing
	m_PrintVG.Terminate();

	return true;
}


/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool CTransforms::RenderScene()
{
	m_ui32AbsTime = PVRShellGetTime();

	// Clear the screen with clear color.
	vgClear(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	// Set fill paint
	vgSetPaint(m_vgPaint, VG_FILL_PATH);

	unsigned int ui32TimeSinceStart = PVRShellGetTime() - m_ui32StartTime;
	// toggle segment type every 3 seconds
	int ui32ActiveTransform = (ui32TimeSinceStart % (3000 * 7)) / 3000;

	switch(ui32ActiveTransform)
	{
		case 0: DoTranslate(); break;
		case 1: DoScaleCentered(); break;
		case 2: DoScaleOrigin(); break;
		case 3: DoRotateCentered(); break;
		case 4: DoRotateOrigin(); break;
		case 5: DoShearCentered(); break;
		case 6: DoShearOrigin(); break;
	}

	// Draw user interface
	static char* apszTransforms[] = {
		"Translation",
		"Scaling (centered on object)",
		"Scaling (from origin)",
		"Rotate (centered on object)",
		"Rotate (around origin)",
		"Shear (centered on object)",
		"Shear (from origin)",
	};

	m_PrintVG.DisplayDefaultTitle("Transforms", apszTransforms[ui32ActiveTransform], ePVRTPrint3DLogoIMG);
	return true;
}

/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialise a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new CTransforms();
}

/*****************************************************************************
 End of file (OVGTransforms.cpp)
*****************************************************************************/
