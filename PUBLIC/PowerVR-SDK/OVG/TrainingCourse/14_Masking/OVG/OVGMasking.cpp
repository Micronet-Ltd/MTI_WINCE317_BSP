/******************************************************************************

 @File         OVGMasking.cpp

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
const char c_szPVRFile[] = "mask.pvr";

/****************************************************************************
** Class OVGMasking
****************************************************************************/
class OVGMasking : public PVRShell
{
private:
	VGPath m_avgPath[2];
	VGPaint m_avgColourPaint[2];
	CPVRTPrintVG m_PrintVG;

public:
	OVGMasking() {}
	void CreatePath();

	// PVRShell functions
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	/****************************************************************************
	** Function Definitions
	****************************************************************************/

	// This demo defines no additional methods
};

/*******************************************************************************
 * Function Name  : CreatePath
 * Description    : Creates two paths to be affected by the Mask
 *******************************************************************************/
void OVGMasking::CreatePath()
{
	/*
	Find the midpoint of the screen so we can build 2 paths around that.
	*/

	float fWidth = PVRShellGet(prefWidth ) * 0.5f;
	float fHeight= PVRShellGet(prefHeight) * 0.5f;

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
			fWidth - 80.0f, fHeight - 50.0f,
			fWidth + 80.0f, fHeight - 50.0f,
			fWidth, fHeight + 80.0f
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
			fWidth - 100.0f, fHeight - 35.0f,
			fWidth + 100.0f, fHeight - 35.0f,
			fWidth + 100.0f, fHeight + 18.0f,
			fWidth - 100.0f, fHeight + 18.0f
	};

	vgAppendPathData(m_avgPath[1], 5, aui8PathSegments2, afPoints2);
}
/*******************************************************************************
 * Function Name  : InitApplication
 * Returns        : true if no error occured
 * Description    : Code in InitApplication() will be called by the Shell ONCE per
 *					run, early on in the execution of the program.
 *					Used to initialise variables that are not dependent on the
 *					rendering context (e.g. external modules, loading meshes, etc.)
 *******************************************************************************/
bool OVGMasking::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*) PVRShellGet(prefReadPath));

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
bool OVGMasking::QuitApplication()
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
bool OVGMasking::InitView()
{
	//Create the paths so we have something to look at.
	CreatePath();

	//Set the render quality so the stroke borders have some form of anti-aliasing
	vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_BETTER);

	// Create the two paints that the paths will use
	m_avgColourPaint[0] = vgCreatePaint();
	vgSetParameteri(m_avgColourPaint[0], VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetColor(m_avgColourPaint[0], PVRTRGBA(255,255,170,255));

	m_avgColourPaint[1] = vgCreatePaint();
	vgSetParameteri(m_avgColourPaint[1], VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetColor(m_avgColourPaint[1], PVRTRGBA(255,136,136, 255));

	// A VGImage is used to define the mask (or a part of)
	VGImage vgMaskImg;

	// Using the PVR Tools we're going to load the mask data from a pvr file
	if(PVRTImageLoadFromPVR(c_szPVRFile, &vgMaskImg) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "Error: Failed to open mask.pvr.");
		return false;
	}

	// Get the width and the height of the image.
	unsigned int ui32Width = vgGetParameteri(vgMaskImg, VG_IMAGE_WIDTH);
	unsigned int ui32Height= vgGetParameteri(vgMaskImg, VG_IMAGE_HEIGHT);

	//Get the centre point of the screen minus the width and the height of the image so the image will be centralised.
	VGint i32X = (VGint) ((PVRShellGet(prefWidth)  - ui32Width ) * 0.5);
	VGint i32Y = (VGint) ((PVRShellGet(prefHeight) - ui32Height) * 0.5);

	/*
	Using vgMask we use the VGImage to set the values of the mask.

		void vgMask(VGImage mask, VGMaskOperation operation, VGint x, VGint y, VGint width, VGint height)

	x,y,width and height define the region of the mask you wish to alter. 'operation' defines how you wish
	to change the mask.

	VG_CLEAR_MASK	  - sets all mask alpha values in the region to 0, ignoring the new mask image.
	VG_FILL_MASK	  - sets all mask alpha values in the region to 1, ignoring the new mask image.
	VG_SET_MASK		  - copies alpha values from the image to the region of interest.
	VG_UNION_MASK	  - replaces the previous alpha mask in the region of interest by its union with the new image.
						The new alpha values are always greater than or equal to the old ones.
	VG_INTERSECT_MASK - replaces the previous alpha mask in the region of interest by its intersection with
						the new mask image. The resulting mask values are always less than or equal to their
						previous value.
	VG_SUBTRACT_MASK  - This subtracts the new alpha mask from the previous alpha mask and replaces the previous alpha mask
						in the region of interest by the resulting mask. The resulting alpha values are always less than or
						equal to their previous value.
	*/
	vgMask(vgMaskImg, VG_SET_MASK, i32X, i32Y, ui32Width, ui32Height);

	//Destroy the image as it is no longer needed.
	vgDestroyImage(vgMaskImg);

	m_PrintVG.Initialize(PVRShellGet(prefWidth), PVRShellGet(prefHeight));
	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : true if no error occured
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool OVGMasking::ReleaseView()
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
bool OVGMasking::RenderScene()
{
	//Create and set the clear colour
	VGfloat afClearColour[] = { 0.6f, 0.8f, 1.0f, 1.0f };
	vgSetfv(VG_CLEAR_COLOR, 4, afClearColour);

	//Clear the screen with the current clear color.
	vgClear(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	// Set the paint and draw the path
	vgSetPaint(m_avgColourPaint[1], VG_FILL_PATH);
	vgDrawPath(m_avgPath[1], VG_STROKE_PATH | VG_FILL_PATH);

	//Enable masking for the triangle
	vgSeti(VG_MASKING, VG_TRUE);

	// Set the paint and draw the path
	vgSetPaint(m_avgColourPaint[0], VG_FILL_PATH);
	vgDrawPath(m_avgPath[0], VG_STROKE_PATH | VG_FILL_PATH);

	//Disable masking
	vgSeti(VG_MASKING, VG_FALSE);

	m_PrintVG.DisplayDefaultTitle("Masking", "", ePVRTPrint3DSDKLogo);
	return true;
}


/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialise a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new OVGMasking();
}

/*****************************************************************************
 End of file (OVGMasking.cpp)
*****************************************************************************/
