/******************************************************************************

 @File         OVGChildImage.cpp

 @Title        OpenVG ChildImage using PVRShell

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     Independent

 @Description  Shows how to use child images in OpenVG

******************************************************************************/
#include "PVRShell.h"
#include "OVGTools.h"

/****************************************************************************
** Constants
****************************************************************************/
// Size of the image we want to create
#define IMG_SIZE		128

/****************************************************************************
** Class CChildImage
****************************************************************************/
class CChildImage : public PVRShell
{
public:
	CChildImage() {}

	VGImage m_vgImage;
	VGImage m_avgChildImages[2];
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
bool CChildImage::InitApplication()
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
bool CChildImage::QuitApplication()
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
bool CChildImage::InitView()
{
	//Create an image
	m_vgImage = vgCreateImage(VG_sRGBA_8888, IMG_SIZE, IMG_SIZE, VG_IMAGE_QUALITY_NONANTIALIASED);

	/*
	Populate the image from memory. A 32bit integer array (8bits per component)
	is created and populated.
	*/

	VGuint* pui32ImgData = new VGuint[IMG_SIZE*IMG_SIZE];

	for(int i = 0; i < IMG_SIZE; ++i)
	{
		for(int j = 0; j < IMG_SIZE; ++j)
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
	vgImageSubData(m_vgImage, pui32ImgData, sizeof(VGuint) * IMG_SIZE, VG_sRGBA_8888, 0, 0, IMG_SIZE, IMG_SIZE);

	// Delete the image data as it is now in OpenVG memory
	delete[] pui32ImgData;
	pui32ImgData = 0;

	//Create child images

	/*
	The first child is a child of m_vgImage and is made up of the region of m_vgImage from (0,0) to (IMG_SIZE / 2, IMG_SIZE / 2).
	Note: The area specified must be in the bounds of the parent.
	*/
	m_avgChildImages[0] = vgChildImage(m_vgImage,  0, 0, (VGint) (IMG_SIZE * 0.5), (VGint) (IMG_SIZE * 0.5));

	//The second child is a clone of the first child.

	//Get the dimensions of the first child..
	int i32ChildWidth = vgGetParameteri(m_avgChildImages[0], VG_IMAGE_WIDTH);
	int i32ChildHeight= vgGetParameteri(m_avgChildImages[0], VG_IMAGE_HEIGHT);

	//..and use them to define the region for creating the second child.
	m_avgChildImages[1] = vgChildImage(m_avgChildImages[0],  0, 0, i32ChildWidth,  i32ChildHeight);

	/*
	Clear a small portion of the bottom left-hand corner of m_avgChildImages[0] to red.
	This change will be seen in all relatives of m_avgChildImages[0], reason being they
	all share the same physical memory.

	Note:

	When clearing you specify the coordinate you want to start from and then how many pixels
	across and up you want to clear.
	*/
	VGfloat afClearColour[] = {1.0f, 0.0f, 0.0f, 1.0f};
	vgSetfv(VG_CLEAR_COLOR, 4, afClearColour);
	vgClearImage(m_avgChildImages[0], 0, 0, 10, 10);

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
bool CChildImage::ReleaseView()
{
	// Cleanup: destroy OpenVG image.
	vgDestroyImage(m_vgImage);
	m_vgImage = 0;

	//Destroy the children as well otherwise the memory isn't reclaimed
	vgDestroyImage(m_avgChildImages[0]);
	m_avgChildImages[0] = 0;

	vgDestroyImage(m_avgChildImages[1]);
	m_avgChildImages[1] = 0;

	m_PrintVG.Terminate();
	return true;
}


/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool CChildImage::RenderScene()
{
	//Clear the screen with the current clear color.
	vgClear(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	//Change the matrix mode to VG_MATRIX_IMAGE_USER_TO_SURFACE so we can transform the images
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);

	//Set the current matrix to the identiy
	vgLoadIdentity();

	//Translate the image to the desired position
	vgTranslate(PVRShellGet(prefWidth) * 0.5f - (IMG_SIZE * 0.5f), PVRShellGet(prefHeight)* 0.5f + 5.0f);

	//Draw the main image
	vgDrawImage(m_vgImage);

	//Reset the current matrix...
	vgLoadIdentity();

	//...Perform a translation to position the first child image...
	int i32Height = vgGetParameteri(m_avgChildImages[0], VG_IMAGE_HEIGHT);
	vgTranslate(PVRShellGet(prefWidth) * 0.5f - (vgGetParameteri(m_avgChildImages[1], VG_IMAGE_WIDTH) * 0.5f), PVRShellGet(prefHeight)* 0.5f - i32Height - 5.0f);

	//...and draw
	vgDrawImage(m_avgChildImages[0]);

	//Reset the current matrix...
	vgLoadIdentity();

	//...Perform a translation to position the second child image...
	i32Height = vgGetParameteri(m_avgChildImages[0], VG_IMAGE_HEIGHT) + vgGetParameteri(m_avgChildImages[1], VG_IMAGE_HEIGHT);
	vgTranslate(PVRShellGet(prefWidth) * 0.5f - (vgGetParameteri(m_avgChildImages[1], VG_IMAGE_WIDTH) * 0.5f), PVRShellGet(prefHeight)* 0.5f - i32Height - 10.0f);

	//...and draw
	vgDrawImage(m_avgChildImages[1]);

	m_PrintVG.DisplayDefaultTitle("ChildImage", "", ePVRTPrint3DLogoIMG);
	return true;
}


/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialise a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new CChildImage();
}

/*****************************************************************************
 End of file (OVGChildImage.cpp)
*****************************************************************************/
