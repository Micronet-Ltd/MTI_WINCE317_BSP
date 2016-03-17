/******************************************************************************

 @File         OVGIntroducingSVG.cpp

 @Title        Using the tools it loads a simple svg file

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     Independent

 @Description  This little example shows how to use the Tools library to load a
               simple SVG file.

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
const char c_szSVGFile[] = "tiger.svg";

/****************************************************************************
** Class COVGIntroducingSVG
****************************************************************************/
class COVGIntroducingSVG : public PVRShell
{
public:
	CPVRTSVGObject m_SVGObject;
	CPVRTPrintVG m_PrintVG;

	int m_i32WindowWidth;
	int m_i32WindowHeight;

	COVGIntroducingSVG() {}

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
bool COVGIntroducingSVG::InitApplication()
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
bool COVGIntroducingSVG::QuitApplication()
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
bool COVGIntroducingSVG::InitView()
{
	char *pszOvgFilename = NULL;

	// Get file from command line.
	const SCmdLineOpt* psCmdLineOpts = (const SCmdLineOpt*)PVRShellGet(prefCommandLineOpts);
	int i32CmdLineOpts = PVRShellGet(prefCommandLineOptNum);

	for (int i = 0; i < i32CmdLineOpts; ++i)
	{
		if(strcmp(psCmdLineOpts[i].pArg, "-svg") == 0)
		{
			pszOvgFilename = (char*)psCmdLineOpts[i].pVal;
			break;
		}
	}

	// If no command-line, load tiger.svg file (in the same location where the .exe resides)
	if(pszOvgFilename == NULL)
		pszOvgFilename = (char*) &c_szSVGFile[0];		

	// Get the screen width and height
	m_i32WindowWidth  = PVRShellGet(prefWidth);
	m_i32WindowHeight = PVRShellGet(prefHeight);

	/*
		Load the .svg file into the object using the SVG parser. We need to pass the screen width and height to the load function
	    as the tools code scales the scene based on the viewbox/width & height parameters.
    */
	CPVRTSVGParser SVGParser;

	if(!SVGParser.Load(pszOvgFilename, &m_SVGObject, m_i32WindowWidth, m_i32WindowHeight))
	{
		PVRShellSet(prefExitMessage, "Failed to load *.svg file.");
		return false;
	}

	// Set the clear colour
	VGfloat afClearColour[] = { 0.6f, 0.8f, 1.0f, 1.0f };
	vgSetfv(VG_CLEAR_COLOR, 4, afClearColour);

	/*
		Setup the transformation for the SVGObject. The aim of the transformation is to translate the SVG
		object to the centre of the screen.
	*/

	// Set the object so its centre is at the origin
	m_SVGObject.SetToOrigin();

	// Set the transformation of the object so it is translated from the origin to the centre of the screen.
	PVRTMat3 m_Transformation;
	m_Transformation = PVRTMat3::Translation(m_i32WindowWidth * 0.5f, m_i32WindowHeight * 0.5f);

	m_SVGObject.SetTransformation(&m_Transformation);

	// Initialise PrintVG for the logo and the title
	m_PrintVG.Initialize(m_i32WindowWidth, m_i32WindowHeight);
	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : Nothing
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool COVGIntroducingSVG::ReleaseView()
{
	// free anything print vg created
	m_PrintVG.Terminate();
	return true;
}

/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool COVGIntroducingSVG::RenderScene()
{
	// Clear surface
	vgClear(0, 0, m_i32WindowWidth, m_i32WindowHeight);

	// Draw the SVGobject
	m_SVGObject.Draw();

	// Display the title and logo.
	m_PrintVG.DisplayDefaultTitle("IntroducingSVG", "", ePVRTPrint3DSDKLogo);
	return true;
}


/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialise a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new COVGIntroducingSVG();
}

/*****************************************************************************
 End of file (OVGIntroducingSVG.cpp)
*****************************************************************************/
