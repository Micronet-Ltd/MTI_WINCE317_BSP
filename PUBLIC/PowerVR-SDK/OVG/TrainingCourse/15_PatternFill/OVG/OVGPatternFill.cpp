/******************************************************************************

 @File         OVGPatternFill.cpp

 @Title        OpenVG Pattern Fill using PVRShell

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     Independent

 @Description  Simple example showing how to Pattern fill in OpenVG

******************************************************************************/
#include "OVGTools.h"
#include "PVRShell.h"

/****************************************************************************
** Constants
****************************************************************************/

/****************************************************************************
** Class CPatternFill
****************************************************************************/
class CPatternFill : public PVRShell
{
private:
	VGPath m_vgPath;
	VGPaint m_vgImagePaint;
	VGImage m_vgImage;
	CPVRTPrintVG m_PrintVG;

public:
	CPatternFill() {}

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
 * Description    : Creates an OpenVG path.
 *                  This function will be called by InitView once.
 *******************************************************************************/
void CPatternFill::CreatePath()
{
	/*
	Create the shape to pattern fill
	*/

	static VGubyte aui8PathSegments[] = {
		VG_MOVE_TO,
		VG_LCWARC_TO,
		VG_LCWARC_TO,
		VG_LCWARC_TO,
		VG_LCWARC_TO,
		VG_LCWARC_TO,
		VG_LCWARC_TO
	};

	static VGfloat afPathCoords[] = {
		85.0f, 85.0f,
		1.0f, 1.0f, 0.0f, -30.0f, 115.0f,
		1.0f, 1.0f, 0.0f, -115.0f , 30.0f,
		1.0f, 1.0f, 0.0f, -85.0f, -85.0f ,
		1.0f, 1.0f, 0.0f, 30.0f, -115.0f ,
		1.0f, 1.0f, 0.0f, 115.0f, -30.0f,
		1.0f, 1.0f, 0.0f, 85.0f, 85.0f
	};

	// Create a path handle...
	m_vgPath = vgCreatePath(
		VG_PATH_FORMAT_STANDARD,
		VG_PATH_DATATYPE_F,
		1.0f, 0.0f,
		7,
		7,
		VG_PATH_CAPABILITY_APPEND_TO);

	// ... and populate it with data
	vgAppendPathData(m_vgPath, 7, aui8PathSegments, afPathCoords);
}

/*******************************************************************************
 * Function Name  : CreatePaints
 * Description    : Creates a paint that uses an image as a pattern.
 *                  This function will be called by InitView once.
 *******************************************************************************/
void CPatternFill::CreatePaints()
{
	//Create the image we are going to use as the pattern
	m_vgImage = vgCreateImage(VG_sRGBA_8888, 128, 128, VG_IMAGE_QUALITY_BETTER);

	VGuint* pui32ImgData = new VGuint[128 * 128];

	for (int i = 0; i < 128; ++i)
	{
		for (int j = 0; j < 128; ++j)
		{
			// Fills the data with a fancy pattern
			if ( ((i*j)/8) % 2 )
				pui32ImgData[j*128+i] = PVRTRGBA(255,255,0,255);
			else
				pui32ImgData[j*128+i] = PVRTRGBA(255, 255 - (j * 2), 255 - i, 255 - (i * 2));
		}
	}

	vgImageSubData(m_vgImage,pui32ImgData, sizeof(VGuint) * 128,VG_sRGBA_8888, 0, 0, 128, 128);

	// Delete the image data as it is now in OpenVG memory
	delete[] pui32ImgData;
	pui32ImgData = 0;

	// Create a paint
	m_vgImagePaint = vgCreatePaint();

	// Set its type to pattern
	vgSetParameteri(m_vgImagePaint, VG_PAINT_TYPE, VG_PAINT_TYPE_PATTERN);

	//Set the image for the paint to use as a pattern
	vgPaintPattern(m_vgImagePaint, m_vgImage);

	//Set the tiling mode of the pattern. In this case it is set to repeat endlessly
	vgSetParameteri(m_vgImagePaint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_REPEAT);
}

/*******************************************************************************
 * Function Name  : InitApplication
 * Returns        : true if no error occured
 * Description    : Code in InitApplication() will be called by the Shell ONCE per
 *					run, early on in the execution of the program.
 *					Used to initialise variables that are not dependent on the
 *					rendering context (e.g. external modules, loading meshes, etc.)
 *******************************************************************************/
bool CPatternFill::InitApplication()
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
bool CPatternFill::QuitApplication()
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
bool CPatternFill::InitView()
{
	//Create the path
	CreatePath();

	//Create the paints
	CreatePaints();

	//Set the clear colour
	VGfloat afClearColour[] = { 0.6f, 0.8f, 1.0f, 1.0f };
	vgSetfv(VG_CLEAR_COLOR, 4, afClearColour);

	m_PrintVG.Initialize(PVRShellGet(prefWidth), PVRShellGet(prefHeight));
	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns		  : true if no error occured
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool CPatternFill::ReleaseView()
{
	//destroy the path
	vgDestroyPath(m_vgPath);

	//destroy the paint
	vgDestroyPaint(m_vgImagePaint);

	//destroy the image
	vgDestroyImage(m_vgImage);

	m_PrintVG.Terminate();
	return true;
}


/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool CPatternFill::RenderScene()
{
	// Clear the screen with the clear colour.
	vgClear(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	// Switch the matrix mode to VG_MATRIX_PATH_USER_TO_SURFACE
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
	vgLoadIdentity();

	// Set the paint you would like to fill the shape with
	vgSetPaint(m_vgImagePaint, VG_FILL_PATH);

	vgTranslate(PVRShellGet(prefWidth) * 0.5f, PVRShellGet(prefHeight) * 0.5f);
	// Draw the path with stroke and fill
	vgDrawPath(m_vgPath, VG_STROKE_PATH | VG_FILL_PATH);

	m_PrintVG.DisplayDefaultTitle("PatternFill", "", ePVRTPrint3DLogoIMG);
	return true;
}

/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialise a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new CPatternFill();
}

/*****************************************************************************
 End of file (OVGPatternFill.cpp)
*****************************************************************************/
