/******************************************************************************

 @File         OVGColorTransform.cpp

 @Title        Introducing the PVG PowerVR Vector Graphics file format

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     Independent

 @Description  This little example shows how to use the Tools library to load a
               PVG file exported from adobe Illustrator.

******************************************************************************/
#include "OVGTools.h"
#include "PVRShell.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

/****************************************************************************
** Constants
****************************************************************************/
const char c_szPVGFile[] = "Example.pvg";

/****************************************************************************
** Class OVGColorTransform
****************************************************************************/
class OVGColorTransform : public PVRShell
{
public:
	CPVRTPVGObject* m_pPVGObj;

	CPVRTPrintVG m_PrintVG;

	bool m_bKeepAspect;

	int m_i32WindowWidth;
	int m_i32WindowHeight;

	float m_fCounter;

	OVGColorTransform() { }

	/* PVRShell functions */
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();
};


/*******************************************************************************
 * Function Name  : InitApplication
 * Inputs		  : argc, *argv[], uWidth, uHeight
 * Returns        : true if no error occured
 * Description    : Code in InitApplication() will be called by the Shell ONCE per
 *					run, early on in the execution of the program.
 *					Used to initialise variables that are not dependent on the
 *					rendering context (e.g. external modules, loading meshes, etc.)
 *******************************************************************************/
bool OVGColorTransform::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*) PVRShellGet(prefReadPath));

	PVRShellSet(prefOpenVGContext, true);
	PVRShellSet(prefFSAAMode, 1); // Set the anti-alias mode to 'faster'
	return true;
}

/*******************************************************************************
 * Function Name  : QuitApplication
 * Returns        : true if no error occured
 * Description    : Code in QuitApplication() will be called by the Shell ONCE per
 *					run, just before exiting the program.
 *******************************************************************************/
bool OVGColorTransform::QuitApplication()
{
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
bool OVGColorTransform::InitView()
{
	char *pszOvgFilename = NULL;

	// Get file from command line.
	const SCmdLineOpt* psCmdLineOpts = (const SCmdLineOpt*)PVRShellGet(prefCommandLineOpts);
	int i32CmdLineOpts = PVRShellGet(prefCommandLineOptNum);

	for (int i = 0; i < i32CmdLineOpts; ++i)
	{
		if(strcmp(psCmdLineOpts[i].pArg, "-pvg") == 0)
		{
			pszOvgFilename = (char*)psCmdLineOpts[i].pVal;
			break;
		}
	}

	// If no command-line, load Example.pvg file
	if(pszOvgFilename == NULL)
		pszOvgFilename = (char*) &c_szPVGFile[0];

	m_pPVGObj = CPVRTPVGObject::FromFile(pszOvgFilename);

	if (m_pPVGObj == NULL)
	{
		PVRShellSet(prefExitMessage, "Could not find example file. Please use the -pvg=<path to file> command line option to point to a PVG file.");
		return false;
	}

	m_bKeepAspect = true;

	m_i32WindowWidth = PVRShellGet(prefWidth);
	m_i32WindowHeight = PVRShellGet(prefHeight);

	VGfloat afClearColour[] = { 0.6f, 0.8f, 1.0f, 1.0f };
	vgSetfv(VG_CLEAR_COLOR, 4, afClearColour);

	// Initialise PrintVG for the logo and the title
	m_PrintVG.Initialize(m_i32WindowWidth, m_i32WindowHeight);

	// Init the counter to a rnd value
	srand(PVRShellGetTime());
	m_fCounter = (rand() % 1000) * 0.01f;

	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : Nothing
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool OVGColorTransform::ReleaseView()
{
	delete m_pPVGObj;
	m_pPVGObj = NULL;

	return true;
}

/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool OVGColorTransform::RenderScene()
{
	// Handle keypad input
	if(PVRShellIsKeyPressed(PVRShellKeyNameLEFT) || PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
	{
		m_bKeepAspect = !m_bKeepAspect;
	}

	// Clear surface
	vgClear(0, 0, m_i32WindowWidth, m_i32WindowHeight);

	// Setup transformations
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
	vgLoadIdentity();

	m_pPVGObj->SetupScaleToSize((float)m_i32WindowWidth, (float)m_i32WindowHeight, m_bKeepAspect);
	m_pPVGObj->SetupTranslateToOrigin();

	float fSin = (float) sin(m_fCounter);
	float fCos = (float) cos(m_fCounter);

	/*
		If the colour transform is enabled the colour
		of anything drawn will be multiplied by a
		per-channel scale and a per-channel bias is
		added:

			R'= R * Sr + Br
			G'= G * Sg + Bg
			B'= B * Sb + Bb
			A'= A * Sa + Ba
	*/

	/*
		To set the per-channel scales and
		biases we uses vgSetfv with the
		VG_COLOR_TRANSFORM_VALUES parameter.
		We pass an array of 8 floats as the value.
	*/

	VGfloat values[8] = { fSin + 1.0f,	/* Sr */
						 fCos + 1.0f,	/* Sg */
						 1.0f,			/* Sb */
						 1.0f,			/* Sa */
						 fCos * 0.3f,	/* Br */
						 fSin * 0.3f,	/* Bg */
						 0.0f,			/* Bb */
						 0.0f};			/* Ba */

	vgSetfv(VG_COLOR_TRANSFORM_VALUES, 8, values);

	// Enable the colour transform
	vgSeti(VG_COLOR_TRANSFORM, VG_TRUE);

	// Draw object
	m_pPVGObj->Draw();

	// Disable the colour transform
	vgSeti(VG_COLOR_TRANSFORM, VG_FALSE);

	m_fCounter += 0.01f;

	// Display the title and logo.
	m_PrintVG.DisplayDefaultTitle("ColorTransform", "", ePVRTPrint3DSDKLogo);
	return true;
}


/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialise a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new OVGColorTransform();
}

/*****************************************************************************
 End of file (OVGColorTransform.cpp)
*****************************************************************************/
