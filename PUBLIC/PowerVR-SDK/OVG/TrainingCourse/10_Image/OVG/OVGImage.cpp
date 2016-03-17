/******************************************************************************

 @File         OVGImage.cpp

 @Title        OpenVG Image using PVRShell

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     Independent

 @Description  Shows how to use images in OpenVG

******************************************************************************/
#include "PVRShell.h"
#include "OVGTools.h"

/****************************************************************************
** Constants
****************************************************************************/
// Size of the image we want to create
#define IMG_SIZE		128

/****************************************************************************
** Class CImage
****************************************************************************/
class CImage : public PVRShell
{
public:
	CImage() {}

	VGImage m_avgImage[2];
	CPVRTPrintVG m_PrintVG;
	/* PVRShell functions */
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
 * Function Name  : InitApplication
 * Returns        : true if no error occured
 * Description    : Code in InitApplication() will be called by the Shell ONCE per
 *					run, early on in the execution of the program.
 *					Used to initialise variables that are not dependent on the
 *					rendering context (e.g. external modules, loading meshes, etc.)
 *******************************************************************************/
bool CImage::InitApplication()
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
bool CImage::QuitApplication()
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
bool CImage::InitView()
{
	//Create a pair of images
	m_avgImage[0] = vgCreateImage(VG_sRGBA_8888, IMG_SIZE, IMG_SIZE, VG_IMAGE_QUALITY_NONANTIALIASED);
	m_avgImage[1] = vgCreateImage(VG_sRGBA_8888, IMG_SIZE, IMG_SIZE, VG_IMAGE_QUALITY_NONANTIALIASED);

	/*
	The first image will be populated from memory. A 32bit integer array (8bits per component)
	is created and populated.
	*/

	VGuint* pui32ImgData = new VGuint[IMG_SIZE * IMG_SIZE];

	for (int i = 0; i < IMG_SIZE; ++i)
	{
		for (int j = 0; j < IMG_SIZE; ++j)
		{
			// Fills the data with a fancy pattern
			if ( ((i*j)/8) % 2 )
				pui32ImgData[j*IMG_SIZE+i] = PVRTRGBA(255,255,0,255);
			else
				pui32ImgData[j*IMG_SIZE+i] = PVRTRGBA(255, 255 - (j * 2), 255 - i, 255 - (i * 2));
		}
	}

	/*
	The data in the array is then copied to the portion of the image starting at (0,0) through to (IMG_SIZE, IMG_SIZE), in
	this case that is the whole image. The coordinate system of the image places (0,0) at the bottom left-hand corner and
	(IMG_SIZE, IMG_SIZE) at the top right-hand corner.
	*/
	vgImageSubData(m_avgImage[0],pui32ImgData, sizeof(VGuint) * IMG_SIZE,VG_sRGBA_8888, 0, 0, IMG_SIZE, IMG_SIZE);

	// Delete the image data as it is now in OpenVG memory
	delete[] pui32ImgData;
	pui32ImgData = 0;

	/*
	The second image will initially be cleared to a single colour and then a part of the first image
	will be copied to it.
	*/

	//The colour to clear the image to is taken from the currently set VG_CLEAR_COLOR.
	VGfloat afClearColour[] = { 1.0f, 0.8f, 0.6f, 1.0f };
	vgSetfv(VG_CLEAR_COLOR, 4, afClearColour);

	//Clear the part of the image in the range (0,0) to (IMG_SIZE, IMG_SIZE), in this case that is the whole image.
	vgClearImage(m_avgImage[1], 0, 0, IMG_SIZE, IMG_SIZE);

	/*
	Copy the bottom left-hand corner ((0,0) to (IMG_SIZE / 2, IMG_SIZE / 2)) of the first image
	into the the second image starting at (IMG_SIZE / 4, IMG_SIZE / 4).
	*/
	int i32ImgSizeQuarter = (int) (IMG_SIZE * 0.25);

	vgCopyImage(m_avgImage[1], i32ImgSizeQuarter, i32ImgSizeQuarter,
				m_avgImage[0], 0,0,
				(VGint) (IMG_SIZE * 0.5), (VGint) (IMG_SIZE * 0.5),
				(VGboolean) false);

	//Set the clear colour for clearing the sceen
	afClearColour[0] = 0.6f;
	afClearColour[1] = 0.8f;
	afClearColour[2] = 1.0f;
	afClearColour[3] = 1.0f;

	vgSetfv(VG_CLEAR_COLOR, 4, afClearColour);

	m_PrintVG.Initialize(PVRShellGet(prefWidth), PVRShellGet(prefHeight));
	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : true if no error occured
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool CImage::ReleaseView()
{
	// Cleanup: destroy OpenVG images
	vgDestroyImage(m_avgImage[0]);
	m_avgImage[0] = 0;

	vgDestroyImage(m_avgImage[1]);
	m_avgImage[1] = 0;

	m_PrintVG.Terminate();
	return true;
}


/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool CImage::RenderScene()
{
	//Clear the screen with the current clear colour.
	vgClear(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	//Change the matrix mode to VG_MATRIX_IMAGE_USER_TO_SURFACE so we can transform the images
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);

	//Set the current matrix to the identity
	vgLoadIdentity();

	//Translate the image to the desired position
	vgTranslate(PVRShellGet(prefWidth) * 0.5f - (IMG_SIZE * 0.5f), PVRShellGet(prefHeight)* 0.5f + 5);

	//Draw the first image
	vgDrawImage(m_avgImage[0]);

	//Reset the current matrix...
	vgLoadIdentity();

	//...Perform a translation to position the second image...
	vgTranslate(PVRShellGet(prefWidth) * 0.5f - (IMG_SIZE * 0.5f), PVRShellGet(prefHeight)* 0.5f - IMG_SIZE - 5);

	//...and draw
	vgDrawImage(m_avgImage[1]);

	m_PrintVG.DisplayDefaultTitle("Image", "", ePVRTPrint3DLogoIMG);
	return true;
}


/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialise a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new CImage();
}

/*****************************************************************************
 End of file (OVGImage.cpp)
*****************************************************************************/
