/******************************************************************************

 @File         OVGBlendModes.cpp

 @Title        OpenVG BlendModes using PVRShell

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     Independent

 @Description  Simple example showing the blend modes in OpenVG

******************************************************************************/
#include "OVGTools.h"
#include "PVRShell.h"

/****************************************************************************
** Constants
****************************************************************************/

/****************************************************************************
** Class CBlendModes
****************************************************************************/
class CBlendModes : public PVRShell
{
private:
	VGPath m_vgPath[6];
	VGPaint m_vgColourPaint;

	CPVRTPrintVG m_PrintVg;

public:
	CBlendModes() {}

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
 * Description    : Creates six OpenVG paths.
 *                  This function will be called by InitView once.
 *******************************************************************************/
void CBlendModes::CreatePath()
{
	/*
	This function creates six paths, where each path is one of the sides of
	the cube.
	*/
	static VGubyte aui8PathSegments[] = {
		VG_MOVE_TO,
		VG_LINE_TO,
		VG_LINE_TO,
		VG_LINE_TO,
		VG_CLOSE_PATH
	};

	static VGfloat afPathCoords1[] = {
		5.0f, 15.0f, 
		5.0f, 65.0f,		
		55.0f, 55.0f,
		55.0f, 5.0f
	};

	static VGfloat afPathCoords2[] = {
		55.0f, 5.0f, 
		20.0f, -15.0f,		
	   -30.0f, -5.0f,
		5.0f, 15.0f
	};

	static VGfloat afPathCoords3[] = {
		5.0f,15.0f, 
		5.0f,65.0f,		
	   -30.0f, 45.0f,
	   -30.0f, -5.0f
	};

	static VGfloat afPathCoords4[] = {
		55.0f, 5.0f, 
		55.0f, 55.0f,		
		20.0f, 35.0f,
		20.0f, -15.0f
	};

	static VGfloat afPathCoords5[] = {
		-30.0f, 45.0f, 
		-30.0f, -5.0f,		
		20.0f, -15.0f,
		20.0f, 35.0f
	};

	static VGfloat afPathCoords6[] = {
		5.0f, 65.0f, 
		55.0f, 55.0f,		
		20.0f, 35.0f,
		-30.0f, 45.0f
	};

	// Create the path handles...
	m_vgPath[0] = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 5,4, VG_PATH_CAPABILITY_APPEND_TO);
	m_vgPath[1] = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 5,4, VG_PATH_CAPABILITY_APPEND_TO);
	m_vgPath[2] = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 5,4, VG_PATH_CAPABILITY_APPEND_TO);
	m_vgPath[3] = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 5,4, VG_PATH_CAPABILITY_APPEND_TO);
	m_vgPath[4] = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 5,4, VG_PATH_CAPABILITY_APPEND_TO);
	m_vgPath[5] = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 5,4, VG_PATH_CAPABILITY_APPEND_TO);

	// ... and populate them with data...
	vgAppendPathData(m_vgPath[0], 5, aui8PathSegments, afPathCoords1);
	vgAppendPathData(m_vgPath[1], 5, aui8PathSegments, afPathCoords2);
	vgAppendPathData(m_vgPath[2], 5, aui8PathSegments, afPathCoords3);
	vgAppendPathData(m_vgPath[3], 5, aui8PathSegments, afPathCoords4);
	vgAppendPathData(m_vgPath[4], 5, aui8PathSegments, afPathCoords5);
	vgAppendPathData(m_vgPath[5], 5, aui8PathSegments, afPathCoords6);
	
	/*
	...then remove the path capabilities.
	*/
	vgRemovePathCapabilities(m_vgPath[0], VG_PATH_CAPABILITY_APPEND_TO);
	vgRemovePathCapabilities(m_vgPath[1], VG_PATH_CAPABILITY_APPEND_TO);
	vgRemovePathCapabilities(m_vgPath[2], VG_PATH_CAPABILITY_APPEND_TO);
	vgRemovePathCapabilities(m_vgPath[3], VG_PATH_CAPABILITY_APPEND_TO);
	vgRemovePathCapabilities(m_vgPath[4], VG_PATH_CAPABILITY_APPEND_TO);
	vgRemovePathCapabilities(m_vgPath[5], VG_PATH_CAPABILITY_APPEND_TO);
}

/*******************************************************************************
 * Function Name  : CreatePaints
 * Description    : Creates a colour paint.
 *                  This function will be called by InitView once.
 *******************************************************************************/
void CBlendModes::CreatePaints()
{
	// Create a paint...
	m_vgColourPaint = vgCreatePaint();
	// ... set its type to colour...
	vgSetParameteri(m_vgColourPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	// ... then set the colour. This paint colour has an alpha value of 128 so it is translucent.
	vgSetColor(m_vgColourPaint, PVRTRGBA(255,255,170, 128));
}

/*******************************************************************************
 * Function Name  : InitApplication
 * Returns        : true if no error occured
 * Description    : Code in InitApplication() will be called by the Shell ONCE per
 *					run, early on in the execution of the program.
 *					Used to initialize variables that are not dependant on the
 *					rendering context (e.g. external modules, loading meshes, etc.)
 *******************************************************************************/
bool CBlendModes::InitApplication()
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
bool CBlendModes::QuitApplication()
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
bool CBlendModes::InitView()
{
	//Create the paths and the paint
	CreatePath();
	CreatePaints();

	// Set the stroke width, cap style and the join style.
	vgSetf(VG_STROKE_LINE_WIDTH, 1.0f);
	vgSeti(VG_STROKE_CAP_STYLE, VG_CAP_SQUARE);
	vgSeti(VG_STROKE_JOIN_STYLE, VG_JOIN_ROUND);

	// Set the render quality to better.
	vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_BETTER);
	
	//Set the background clear colour
	VGfloat afClearColour[] = { 0.6f, 0.8f, 1.0f, 1.0f };
	vgSetfv(VG_CLEAR_COLOR, 4, afClearColour);

	//Initialise printvg so we can draw text to the screen
	m_PrintVg.Initialize(PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns		  : true if no error occured
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool CBlendModes::ReleaseView()
{
	//Tidy up by destroying what we have created
	for(int i = 0; i < 6; ++i)
		vgDestroyPath(m_vgPath[i]);

	vgDestroyPaint(m_vgColourPaint);

	m_PrintVg.Terminate();
	return true;
}


/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool CBlendModes::RenderScene()
{
	// Clear the screen with clear color.
	vgClear(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	//Find the midpoint of the screen
	float fMidX = PVRShellGet(prefWidth ) * 0.5f;
	float fMidY = PVRShellGet(prefHeight) * 0.5f;
	
	for(int i = 0; i < 6; ++i)
	{
		//translate cube 'i' to the correct position
		switch(i)
		{
			case 0:
				//Write a string representing the blend type to the screen
				m_PrintVg.DrawShadowString(fMidX - 118.0f, fMidY + 90.0f, 0.8f, "src_over");
				
				//Translate any paths that will be drawn to the required position.
				vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
				vgLoadIdentity();
				
				vgTranslate((VGfloat) fMidX - 75.0f, (VGfloat)fMidY + 70.0f);

				//Set the blend mode to use.
				vgSeti(VG_BLEND_MODE, VG_BLEND_SRC_OVER);
			break;
			case 1:
				m_PrintVg.DrawShadowString(fMidX + 15.0f, fMidY + 90.0f, 0.8f, "src_in");
				
				vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
				vgLoadIdentity();
				
				vgTranslate((VGfloat) fMidX + 45.0f, (VGfloat)fMidY + 70.0f);
				vgSeti(VG_BLEND_MODE, VG_BLEND_SRC_IN);
			break;
			case 2:
				m_PrintVg.DrawShadowString(fMidX - 112.0f, fMidY - 5.0f, 0.8f, "multiply");
				
				vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
				vgLoadIdentity();
				
				vgTranslate((VGfloat) fMidX - 75.0f, (VGfloat)fMidY - 25.0f);
				vgSeti(VG_BLEND_MODE, VG_BLEND_MULTIPLY);
			break;
			case 3:
				m_PrintVg.DrawShadowString(fMidX + 12.0f, fMidY - 5.0f, 0.8f, "screen");
				
				vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
				vgLoadIdentity();
				
				vgTranslate((VGfloat) fMidX + 45.0f, (VGfloat)fMidY - 25.0f);
				vgSeti(VG_BLEND_MODE, VG_BLEND_SCREEN);	
			break;
			case 4:
				m_PrintVg.DrawShadowString(fMidX - 108.0f, fMidY - 95.0f, 0.8f, "lighten");
				
				vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
				vgLoadIdentity();
				
				vgTranslate((VGfloat) fMidX - 75.0f, (VGfloat)fMidY - 115.0f);
				vgSeti(VG_BLEND_MODE, VG_BLEND_LIGHTEN);	
			break;
			case 5:
				m_PrintVg.DrawShadowString(fMidX + 15.0f, fMidY - 95.0f, 0.8f, "darken");
				
				vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
				vgLoadIdentity();
				
				vgTranslate((VGfloat) fMidX + 47.0f, (VGfloat)fMidY - 115.0f);
				vgSeti(VG_BLEND_MODE, VG_BLEND_DARKEN);
			break;
		};

		// Set the paint
		vgSetPaint(m_vgColourPaint, VG_FILL_PATH);

		// Draw all the sides of the cube
		vgDrawPath(m_vgPath[0], VG_STROKE_PATH | VG_FILL_PATH);
		vgDrawPath(m_vgPath[1], VG_STROKE_PATH | VG_FILL_PATH);
		vgDrawPath(m_vgPath[2], VG_STROKE_PATH | VG_FILL_PATH);
		vgDrawPath(m_vgPath[3], VG_STROKE_PATH | VG_FILL_PATH);
		vgDrawPath(m_vgPath[4], VG_STROKE_PATH | VG_FILL_PATH);
		vgDrawPath(m_vgPath[5], VG_STROKE_PATH | VG_FILL_PATH);
	}

	m_PrintVg.DisplayDefaultTitle("BlendModes", "", ePVRTPrint3DLogoIMG);

	return true;
}

/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialise a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new CBlendModes();
}

/*****************************************************************************
 End of file (OVGBlendModes.cpp)
*****************************************************************************/
