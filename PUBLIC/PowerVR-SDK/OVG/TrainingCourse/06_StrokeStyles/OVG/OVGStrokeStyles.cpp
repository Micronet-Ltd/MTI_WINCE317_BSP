/******************************************************************************

 @File         OVGStrokeStyles.cpp

 @Title        OpenVG Stroke Styles using PVRShell

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     Independent

 @Description  Simple example showing the different stroke styles in OpenVG

******************************************************************************/
#include "OVGTools.h"
#include "PVRShell.h"

/****************************************************************************
** Constants
****************************************************************************/

/****************************************************************************
** Class StrokeStyles
****************************************************************************/
class CStrokeStyles : public PVRShell
{
private:
	VGPath m_vgPath;
	CPVRTPrintVG m_PrintVG;

	int m_i32JoinStyle;
	int m_i32CapStyle;
	int m_i32DashStyle;
	int m_i32Selected;
	VGfloat m_fMiterLimit;
	VGfloat m_fDashPhase;

public:
	CStrokeStyles() :
		m_i32JoinStyle(0),
		m_i32CapStyle(0),
		m_i32DashStyle(0),
	    m_i32Selected(0),
		m_fMiterLimit(1),
		m_fDashPhase(0)
	{}

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
 * Description    : Code in CreatePath() will be called by InitView() and is
 *					used to set up the path that will be used.
 *******************************************************************************/
void CStrokeStyles::CreatePath()
{
	static VGubyte aui8PathSegments[] = {
		VG_MOVE_TO_ABS,
		VG_VLINE_TO_ABS,
		VG_CUBIC_TO_ABS,
		VG_VLINE_TO_ABS
	};
	static VGfloat afPathCoords[] = {
		0.1f, 0.1f,
		0.5f,
		0.38f, 0.0f, 0.62f, 0.7f, 0.9f, 0.5f,
		0.1f
	};

	m_vgPath = vgCreatePath(
		VG_PATH_FORMAT_STANDARD,
		VG_PATH_DATATYPE_F,
		1.0f, 0.0f,
		4,
		10,
		VG_PATH_CAPABILITY_APPEND_TO);

	vgAppendPathData(
		m_vgPath,
		4,
		aui8PathSegments,
		afPathCoords);

	vgRemovePathCapabilities(m_vgPath, VG_PATH_CAPABILITY_APPEND_TO);
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
bool CStrokeStyles::InitApplication()
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
bool CStrokeStyles::QuitApplication()
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
bool CStrokeStyles::InitView()
{
	/*
	Create path
	*/
	CreatePath();

	/*
	The clear colour will be used whenever calling vgClear(). The colour is given
	as non-premultiplied sRGBA.
	*/
	VGfloat afClearColour[] = { 0.6f, 0.8f, 1.0f, 1.0f };
	vgSetfv(VG_CLEAR_COLOR, 4, afClearColour);

	/*
	Initialise custom text drawing
	*/
	m_PrintVG.Initialize(PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : Nothing
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool CStrokeStyles::ReleaseView()
{
	vgDestroyPath(m_vgPath);

	m_PrintVG.Terminate();

	return true;
}


/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool CStrokeStyles::RenderScene()
{
	/*
		If the left or right arrow keys are pressed then change the
		CapStyle or JoinStyle or DashStyle depending on which one is
		selected.
	*/
	if(PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
	{
		switch(m_i32Selected)
		{
			case 0: m_i32CapStyle  = (m_i32CapStyle +  2) % 3; break;
			case 1: m_i32JoinStyle = (m_i32JoinStyle + 2) % 3; break;
			case 2: m_i32DashStyle = (m_i32DashStyle + 2) % 3; break;
		}
	}
	if(PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
	{
		switch(m_i32Selected)
		{
			case 0: m_i32CapStyle  = (m_i32CapStyle +  1) % 3; break;
			case 1: m_i32JoinStyle = (m_i32JoinStyle + 1) % 3; break;
			case 2: m_i32DashStyle = (m_i32DashStyle + 1) % 3; break;
		}
	}

	/*
		If the up or down arrow is pressed then change which item is
		selected.
	*/
	if(PVRShellIsKeyPressed(PVRShellKeyNameUP))
		m_i32Selected = (m_i32Selected + 2) % 3;

	if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
		m_i32Selected = (m_i32Selected + 1) % 3;

	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
	vgLoadIdentity();
	vgScale((float)PVRShellGet(prefWidth), (float)PVRShellGet(prefHeight));

	// Clear the screen with clear colour.
	vgClear(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	// Draw the path with the stroke styles that we want
	vgSeti(VG_STROKE_CAP_STYLE  , VG_CAP_BUTT   + m_i32CapStyle);
	vgSeti(VG_STROKE_JOIN_STYLE , VG_JOIN_MITER + m_i32JoinStyle);
	vgSetf(VG_STROKE_MITER_LIMIT, m_fMiterLimit * PVRShellGet(prefHeight));

	if(m_i32DashStyle > 0)
	{
		vgSetf(VG_STROKE_DASH_PHASE, m_fDashPhase);

		static float s_afDashes[] = { 0.1f, 0.15f, 0.23f, 0.11f };
		vgSetfv(VG_STROKE_DASH_PATTERN, 4, s_afDashes);

		if(m_i32DashStyle == 2)
			m_fDashPhase += 0.01f;
	}
	else
	{
		vgSetfv(VG_STROKE_DASH_PATTERN, 0, NULL);
	}

	vgSetf(VG_STROKE_LINE_WIDTH, 20.0f / PVRShellGet(prefHeight));

	vgDrawPath(m_vgPath, VG_STROKE_PATH);

	/*	Draw the text.

		If one of the pieces of text is currently selected then it will be
		drawn in yellow.
	*/

	static char* apszCapStrings[]  = { "Butt", "Round", "Square" };
	static char* apszJoinStrings[] = { "Miter", "Round", "Bevel" };
	static char* apszDashStrings[] = { "None", "Pattern", "Moving" };

	m_PrintVG.DisplayDefaultTitle("StrokeStyles", "", ePVRTPrint3DLogoIMG);

	float fHeight = PVRShellGet(prefHeight) - 40.0f;

	/*
		Draw the Cap text.
	*/
	m_PrintVG.DrawString(2.0f , fHeight, 0.6f, "Cap:", (int) PVRTRGBA(204,204,204,255));
	m_PrintVG.DrawShadowString(65.0f, fHeight, 0.6f, apszCapStrings[m_i32CapStyle],
			m_i32Selected == 0 ? (int) PVRTRGBA(255,255,0,255) : (int) PVRTRGBA(255,255,255,255));

	/*
		Draw the Join text.
	*/
	fHeight -= 20.0f;
	m_PrintVG.DrawString(2.0f, fHeight, 0.6f, "Join:", (int) PVRTRGBA(204,204,204,255));
	m_PrintVG.DrawShadowString(65.0f, fHeight, 0.6f, apszJoinStrings[m_i32JoinStyle],
			m_i32Selected == 1 ? (int) PVRTRGBA(255,255,0,255) : (int) PVRTRGBA(255,255,255,255));

	/*
		Draw the Dash text.
	*/
	fHeight -= 20.0f;
	m_PrintVG.DrawString(2.0f,fHeight, 0.6f, "Dash:", (int) PVRTRGBA(204,204,204,255));
	m_PrintVG.DrawShadowString(65.0f, fHeight, 0.6f, apszDashStrings[m_i32DashStyle],
			m_i32Selected == 2 ? (int) PVRTRGBA(255,255,0,255) : (int) PVRTRGBA(255,255,255,255));

	return true;
}


/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialise a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new CStrokeStyles();
}

/*****************************************************************************
 End of file (OVGStrokeStyles.cpp)
*****************************************************************************/
