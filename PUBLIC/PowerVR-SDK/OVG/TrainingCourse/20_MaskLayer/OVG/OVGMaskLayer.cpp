/******************************************************************************

 @File         OVGMaskLayer.cpp

 @Title        OVGMaskLayer

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     Independent

 @Description  Shows how to use mask layers in OpenVG

******************************************************************************/
#include "PVRShell.h"
#include "OVGTools.h"
#include <math.h>

/****************************************************************************
** Constants
****************************************************************************/
const char c_szMask1File[] = "mask1.pvr";
const char c_szMask2File[] = "mask2.pvr";

/****************************************************************************
** Class OVGMaskLayer
****************************************************************************/
class OVGMaskLayer : public PVRShell
{
private:
	// The paths and paints
	VGPath m_avgPath[4];
	VGPaint m_avgColourPaint[3];

	// PrintVG
	CPVRTPrintVG m_PrintVG;

	// Our two mask layers
	VGMaskLayer m_vgMaskLayer[2];

	// The screen width and height
	unsigned long m_ui32ScreenWidth;
	unsigned long m_ui32ScreenHeight;

public:
	OVGMaskLayer() {}
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
	void TileImageInMask(VGImage &vgImg, VGMaskOperation eOperation);
};

/*******************************************************************************
 * Function Name  : CreatePath
 * Description    : Creates two paths to be affected by the Mask
 *******************************************************************************/
void OVGMaskLayer::CreatePath()
{
	// Create the paths that will be used in the scene.

	m_avgPath[0] = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
							1.0f, 0.0f, 5, 4, (unsigned int)VG_PATH_CAPABILITY_ALL);

	VGubyte aui8PathSegments[5] = {
		VG_MOVE_TO,
		VG_LINE_TO,
		VG_LINE_TO,
		VG_LINE_TO,
		VG_CLOSE_PATH
	};

	VGfloat afPoints[8] = {
			0.05f, 0.35f,
			0.95f, 0.35f,
			0.95f, 0.6f,
			0.05f, 0.6f
	};

	vgAppendPathData(m_avgPath[0], 5, aui8PathSegments, afPoints);

	m_avgPath[1] = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
							1.0f, 0.0f, 5, 4, (unsigned int)VG_PATH_CAPABILITY_ALL);

	VGfloat afPoints2[8] = {
			0.2f, 0.1f,
			0.4f, 0.1f,
			0.4f, 0.9f,
			0.2f, 0.9f
	};

	vgAppendPathData(m_avgPath[1], 5, aui8PathSegments, afPoints2);

	m_avgPath[2] = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
							1.0f, 0.0f, 5, 4, (unsigned int)VG_PATH_CAPABILITY_ALL);

	VGfloat afPoints3[8] = {
			0.6f, 0.1f,
			0.8f, 0.1f,
			0.8f, 0.9f,
			0.6f, 0.9f
	};

	vgAppendPathData(m_avgPath[2], 5, aui8PathSegments, afPoints3);

	m_avgPath[3] = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
							1.0f, 0.0f, 4, 3, (unsigned int)VG_PATH_CAPABILITY_ALL);

	VGubyte aui8PathSegments2[4] = {
		VG_MOVE_TO,
		VG_LINE_TO,
		VG_LINE_TO,
		VG_CLOSE_PATH,
	};
	VGfloat afPoints4[6] = {
			0.1f, 0.3f,
			0.9f, 0.3f,
			0.5f, 0.7f
	};

	vgAppendPathData(m_avgPath[3], 4, aui8PathSegments2, afPoints4);
}
/*******************************************************************************
 * Function Name  : InitApplication
 * Returns        : true if no error occured
 * Description    : Code in InitApplication() will be called by the Shell ONCE per
 *					run, early on in the execution of the program.
 *					Used to initialise variables that are not dependent on the
 *					rendering context (e.g. external modules, loading meshes, etc.)
 *******************************************************************************/
bool OVGMaskLayer::InitApplication()
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
bool OVGMaskLayer::QuitApplication()
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
bool OVGMaskLayer::InitView()
{
	// Get screen dimensions
	m_ui32ScreenWidth = PVRShellGet(prefWidth);
	m_ui32ScreenHeight= PVRShellGet(prefHeight);

	// Create the paths so we have something to look at.
	CreatePath();

	// Set the render quality so the stroke borders have some form of anti-aliasing
	vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_BETTER);

	// Create the paints that the paths will use
	m_avgColourPaint[0] = vgCreatePaint();
	vgSetParameteri(m_avgColourPaint[0], VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetColor(m_avgColourPaint[0], PVRTRGBA(255,255,15,255));

	m_avgColourPaint[1] = vgCreatePaint();
	vgSetParameteri(m_avgColourPaint[1], VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetColor(m_avgColourPaint[1], PVRTRGBA(255,50,0, 255));

	m_avgColourPaint[2] = vgCreatePaint();
	vgSetParameteri(m_avgColourPaint[2], VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetColor(m_avgColourPaint[2], PVRTRGBA(50,250,15, 255));

	/*
		Load the images we're going to use to modify the mask layer.

		For more details on masking please refer to our OVGMasking training course
	*/

	// Create the VGImages.
	VGImage vgMaskImg, vgMaskImg2;

	// Using the PVR Tools we're going to load the mask data from a pvr file
	if(PVRTImageLoadFromPVR(c_szMask1File, &vgMaskImg) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "Error: Failed to open mask1.pvr.");
		return false;
	}

	if(PVRTImageLoadFromPVR(c_szMask2File, &vgMaskImg2) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "Error: Failed to open mask2.pvr.");
		return false;
	}

	/*
		Create a mask layer

		A VGMaskLayer is an object that allows you to store and manipulate the drawing surface's mask layer
	*/

	m_vgMaskLayer[0] = vgCreateMaskLayer(m_ui32ScreenWidth, m_ui32ScreenHeight);

	if(m_vgMaskLayer[0] == 0)
	{
		PVRShellSet(prefExitMessage, "Error: Failed to create mask layer.");
		return false;
	}

	// Tile the first image in the drawing surface's masking layer
	TileImageInMask(vgMaskImg, VG_SET_MASK);

	/*
		Copy the contents of the drawing surface mask layer into our mask layer object

		vgCopyMask has the following parameters

		VGMaskLayer maskLayer, VGint dx, VGint dy, VGint sx, VGint sy, VGint width, VGint height)

		where

		masklayer is the masklayer to copy to
		dx, dy are the coordinates to start the copy at in the masklayer
		sx, sy are the coordinates to start the copy from in the source mask layer
		width and the height are the width and height of the region you wish to copy.

		In our case we're copying the full mask layer.
	*/

	vgCopyMask(m_vgMaskLayer[0], 0, 0, 0, 0, m_ui32ScreenWidth, m_ui32ScreenHeight);

	// Create the second mask layer
	m_vgMaskLayer[1] = vgCreateMaskLayer(m_ui32ScreenWidth, m_ui32ScreenHeight);

	if(m_vgMaskLayer[1] == 0)
	{
		PVRShellSet(prefExitMessage, "Error: Failed to create mask layer.");
		return false;
	}

	// Replace the contents of the mask by tiling the second image
	TileImageInMask(vgMaskImg2, VG_SET_MASK);

	// Copy the contents of the mask into the second mask layer
	vgCopyMask(m_vgMaskLayer[1], 0, 0, 0, 0, m_ui32ScreenWidth, m_ui32ScreenHeight);

	// Destroy the images as they are no longer needed
	vgDestroyImage(vgMaskImg);
	vgDestroyImage(vgMaskImg2);

	// Set the mask to ones
	vgMask(VG_INVALID_HANDLE, VG_FILL_MASK, 0, 0, m_ui32ScreenWidth, m_ui32ScreenHeight);

	// Init PrintVG
	m_PrintVG.Initialize(m_ui32ScreenWidth, m_ui32ScreenHeight);

	// Setup the transformation to scale the paths to fit the screen
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
	vgLoadIdentity();

	vgScale((float) m_ui32ScreenWidth, (float) m_ui32ScreenHeight);

	// Reduce the stroke size to compensate for our scaling
	vgSetf(VG_STROKE_LINE_WIDTH, 1.0f / m_ui32ScreenHeight);

	//Create and set the clear colour
	VGfloat afClearColour[] = { 0.6f, 0.8f, 1.0f, 1.0f };
	vgSetfv(VG_CLEAR_COLOR, 4, afClearColour);

	return true;
}

/*******************************************************************************
 * Function Name  : TileImageInMask
 * Description    : This function tiles the passed in image to fill the mask
					layer with values
 *******************************************************************************/
void OVGMaskLayer::TileImageInMask(VGImage &vgImg, VGMaskOperation eOperation)
{
	unsigned int ui32ImageWidth  = vgGetParameteri(vgImg, VG_IMAGE_WIDTH);
	unsigned int ui32ImageHeight = vgGetParameteri(vgImg, VG_IMAGE_HEIGHT);

	// Tile the image in the mask
	for(int i32X = 0; i32X < (int) m_ui32ScreenWidth; i32X += ui32ImageWidth)
	{
		for(int i32Y = 0; i32Y < (int) m_ui32ScreenHeight; i32Y += ui32ImageHeight)
		{
			int i32W = i32X + ui32ImageWidth  > m_ui32ScreenWidth  ? m_ui32ScreenWidth - i32X : m_ui32ScreenWidth;
			int i32H = i32Y + ui32ImageHeight > m_ui32ScreenHeight ? m_ui32ScreenHeight - i32Y : m_ui32ScreenHeight;

			vgMask(vgImg, eOperation, i32X, i32Y, i32W, i32H);
		}
	}
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : true if no error occured
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool OVGMaskLayer::ReleaseView()
{
	// Cleanup: destroy the paths
	vgDestroyPath(m_avgPath[0]);
	vgDestroyPath(m_avgPath[1]);

	m_PrintVG.Terminate();

	// Destroy the mask layers
	vgDestroyMaskLayer(m_vgMaskLayer[0]);
	vgDestroyMaskLayer(m_vgMaskLayer[1]);
	return true;
}

/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool OVGMaskLayer::RenderScene()
{
	//Clear the screen with the current clear colour
	vgClear(0, 0, m_ui32ScreenWidth, m_ui32ScreenHeight);

	// Set the paint and draw the path
	vgSetPaint(m_avgColourPaint[1], VG_FILL_PATH);
	vgDrawPath(m_avgPath[0], VG_STROKE_PATH | VG_FILL_PATH);

	//Enable masking
	vgSeti(VG_MASKING, VG_TRUE);

	// Set paint
	vgSetPaint(m_avgColourPaint[2], VG_FILL_PATH);

	/*
		Modify the drawing surface's mask layer by loading it with the values from our first mask layer.

		See OVGMasking for more details on vgMask.
	*/
	vgMask(m_vgMaskLayer[0], VG_SET_MASK, 0, 0, m_ui32ScreenWidth, m_ui32ScreenHeight);

	// Draw our path
	vgDrawPath(m_avgPath[1], VG_STROKE_PATH | VG_FILL_PATH);

	// Load the second mask layer into the mask
	vgMask(m_vgMaskLayer[1], VG_SET_MASK, 0, 0, m_ui32ScreenWidth, m_ui32ScreenHeight);

	// Draw our path
	vgDrawPath(m_avgPath[2], VG_STROKE_PATH | VG_FILL_PATH);

	// Change paint for the triangle
	vgSetPaint(m_avgColourPaint[0], VG_FILL_PATH);

	// Merge our first mask layer into the mask (that currently contains the data from our second layer)
	vgMask(m_vgMaskLayer[0], VG_INTERSECT_MASK, 0, 0, m_ui32ScreenWidth, m_ui32ScreenHeight);

	// Draw our path
	vgDrawPath(m_avgPath[3], VG_STROKE_PATH | VG_FILL_PATH);

	// Disable masking
	vgSeti(VG_MASKING, VG_FALSE);

	// Draw title and logo
	m_PrintVG.DisplayDefaultTitle("MaskLayer", "", ePVRTPrint3DSDKLogo);

	return true;
}


/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialise a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new OVGMaskLayer();
}

/*****************************************************************************
 End of file (OVGMaskLayer.cpp)
*****************************************************************************/
