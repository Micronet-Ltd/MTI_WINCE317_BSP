/******************************************************************************

 @File         OVGImageFilter.cpp

 @Title        OpenVG Image Filters using PVRShell

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     Independent

 @Description  Shows how to use image filters in OpenVG

******************************************************************************/
#include "OVGTools.h"
#include "PVRShell.h"

/****************************************************************************
** Constants
****************************************************************************/
// Size of the image we want to create
#define IMG_SIZE		128
//The number of images we require
#define IMG_NO			5
/****************************************************************************
** Class CImageFilter
****************************************************************************/
class CImageFilter : public PVRShell
{
public:
	CImageFilter() {}

	VGImage m_avgImages[IMG_NO];

	CPVRTPrintVG m_PrintVg;
	int m_ui32StartTime;

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
bool CImageFilter::InitApplication()
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
bool CImageFilter::QuitApplication()
{
	PVRShellOutputDebug("leaving...");
	return true;
}

/*******************************************************************************
 * Function Name  : InitView
 * Returns        : true if no error occured
 * Description    : Code in InitView() will be called by the Shell upon a change
 *					in the rendering context.
 *					Used to initialise variables that are dependant on the rendering
 *					context (e.g. textures, vertex buffers, etc.)
 *******************************************************************************/
bool CImageFilter::InitView()
{
	int i;

	//Create a set of images
	for(i = 0; i < IMG_NO; ++i)
	{
		m_avgImages[i] = vgCreateImage(VG_sRGBA_8888, IMG_SIZE, IMG_SIZE, VG_IMAGE_QUALITY_NONANTIALIASED);
	}

	/*
	The first image will be populated from memory. A 32bit integer array (8bits per component)
	is created and populated.
	*/

	VGuint* pui32ImgData = new VGuint[IMG_SIZE*IMG_SIZE];

	for(i = 0; i < IMG_SIZE; ++i)
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
	vgImageSubData(m_avgImages[0],pui32ImgData, sizeof(VGuint) * IMG_SIZE,VG_sRGBA_8888, 0, 0, IMG_SIZE, IMG_SIZE);

	// Delete the image data as it is now in OpenVG memory
	delete[] pui32ImgData;
	pui32ImgData = 0;

	/*
		Gaussian
		The second image will be created by applying a guassian blur to the first image.
		To do this we use vgGaussianBlur.
	*/

	vgGaussianBlur(m_avgImages[1], m_avgImages[0],1.0f,1.0f,VG_TILE_REPEAT);

	/*
		Convolve

		The third image is created by applying a convolution filter to the first image.

		The first stage is to create a Kernel, in this case I am using a Laplacean filter.
	*/

	VGshort ai16Kernel[] = {-1,-1,-1,
						 -1, 8,-1,
						 -1,-1,-1};

	/*
	Note: The Kernel is in Column-major order, so

				1  2  3
				4  5  6

	would be stored as

				1  4  2  5  3  6
	*/

	//Then we use vgConvolve to apply the kernals

	vgConvolve(m_avgImages[2], m_avgImages[0],3, 3, 0, 0, ai16Kernel, 1.0f, 0.0f, VG_TILE_REPEAT);

	/*
		Separable Convolve

		The fourth image is created by applying a separable filter to the first image

		The first stage is to create the two required kernels
	*/

	VGshort ai16KernelX[] = {-1,0,1};
	VGshort ai16KernelY[] = { 1,2,1};

	//Then we use vgSeparableConvolve to apply the kernels
	vgSeparableConvolve(m_avgImages[3], m_avgImages[0], 3, 3, 0, 0, ai16KernelX, ai16KernelY, 1.0f, 0.0f, VG_TILE_REPEAT);

	/*
		Image Lookup

		The fifth image is created by using a lookup table on the first image.
	*/

	/*
	First stage is to create the lookup tables for the colours. The colour value from the first picture
	for each pixel will be used to index into these to get the new colour. The lookup tables being created
	in this case are going to invert the colours of the first image.
	*/
	VGubyte aui8RedLUT[256];
	VGubyte aui8GreenLUT[256];
	VGubyte aui8BlueLUT[256];
	VGubyte aui8AlphaLUT[256];

	for(i = 0; i < 256; ++i)
	{
		aui8RedLUT[i]   = 255 - i;
		aui8GreenLUT[i] = 255 - i;
		aui8BlueLUT[i]  = 255 - i;
		aui8AlphaLUT[i] = i;
	}

	vgLookup(m_avgImages[4], m_avgImages[0], aui8RedLUT, aui8GreenLUT, aui8BlueLUT, aui8AlphaLUT, (VGboolean) true, (VGboolean) false);

	//Init Font
	m_PrintVg.Initialize(PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	//Set the clear colour for clearing the sceen
	VGfloat afClearColour[] = { 0.6f, 0.8f, 1.0f, 1.0f };
	vgSetfv(VG_CLEAR_COLOR, 4, afClearColour);

	//Get the start time.
	m_ui32StartTime = PVRShellGetTime();
	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : true if no error occured
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool CImageFilter::ReleaseView()
{
	// Cleanup: destroy OpenVG images
	for(int i = 0 ; i < IMG_NO; ++i)
	{
		vgDestroyImage(m_avgImages[i]);
		m_avgImages[i] = 0;
	}

	m_PrintVg.Terminate();
	return true;
}


/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/

bool CImageFilter::RenderScene()
{
	//Clear the screen with the current clear color.
	vgClear(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	//Change the matrix mode to VG_MATRIX_IMAGE_USER_TO_SURFACE so we can transform the images
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);

	//Set the current matrix to the identiy
	vgLoadIdentity();

	//Translate the image to the desired position
	vgTranslate((PVRShellGet(prefWidth) - IMG_SIZE) * 0.5f, (PVRShellGet(prefHeight) - IMG_SIZE) * 0.5f);

	// we switch to the next image every few seconds
	int i32CurrentImage = ((PVRShellGetTime() - m_ui32StartTime)% (4000 * IMG_NO)) / 4000;

	vgDrawImage(m_avgImages[i32CurrentImage]);

	static char* apszPaints[] = {
		"None",
		"Guassian",
		"Convolve",
		"Separable Convolve",
		"Image Lookup"
	};

	//draw text
	m_PrintVg.DisplayDefaultTitle("ImageFilter", apszPaints[i32CurrentImage], ePVRTPrint3DLogoIMG);
	return true;
}


/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialise a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new CImageFilter();
}

/*****************************************************************************
 End of file (OVGImageFilter.cpp)
*****************************************************************************/
