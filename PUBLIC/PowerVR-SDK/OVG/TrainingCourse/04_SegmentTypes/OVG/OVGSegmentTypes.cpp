/******************************************************************************

 @File         OVGSegmentTypes.cpp

 @Title        OpenVG Segment Types using PVRShell

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     Independent

 @Description  Simple example showing the different segment types in OpenVG

******************************************************************************/
#include "OVGTools.h"
#include "PVRShell.h"

/****************************************************************************
** Constants
****************************************************************************/

// We have 11 path segment types to demonstrate
const int g_i32NumSegmentTypes = 11;

/*
This struct simplifies definition of the paths
i32StartSegment and i32StartCoord index into two global lists, one for
path segment types (g_aui8PathSegments) and one for point coordinates
(g_afPathCoords), respecively.
i32NumSegments and i32NumCoord indicate the number of segments and
coordinates for each path.
*/

struct SPathDescription
{
	char* pszName;
	int i32StartSegment;
	int i32NumSegments;
	int i32StartCoord;
	int i32NumCoord;
};

SPathDescription g_asPaths[g_i32NumSegmentTypes] = {
	"line\nVG_LINE_TO",										0,  2,  0,  4,
	"horizontal line\nVG_HLINE_TO",							2,  2,  4,  3,
	"vertical line\nVG_VLINE_TO",							4,  2,  7,  3,
	"quadratic Bezier\nVG_QUAD_TO",							6,  2, 10,  6,
	"cubic Bezier\nVG_CUBIC_TO",							8,  2, 16,  8,
	"smooth quadratic Bezier\nVG_QUAD_TO and\nVG_SQUAD_TO",	10, 3, 24,  8,
	"smooth cubic Bezier\nVG_CUBIC_TO and\nVG_SCUBIC_TO",	13, 3, 32, 12,
	"small counter-clockwise arc\nVG_SCCWARC_TO",			18, 2, 44,  7,
	"large clockwise arc\nVG_LCWARC_TO",					20, 2, 44,  7,
	"small clockwise arc\nVG_SCWARC_TO",					16, 2, 44,  7,
	"large counter-clockwise arc\nVG_LCCWARC_TO",			22, 2, 44,  7,
};

VGubyte g_aui8PathSegments[] = {
	VG_MOVE_TO_ABS, VG_LINE_TO_ABS,
	VG_MOVE_TO_ABS, VG_HLINE_TO_ABS,
	VG_MOVE_TO_ABS, VG_VLINE_TO_ABS,
	VG_MOVE_TO_ABS, VG_QUAD_TO_ABS,
	VG_MOVE_TO_ABS, VG_CUBIC_TO_ABS,
	VG_MOVE_TO_ABS, VG_QUAD_TO_ABS, VG_SQUAD_TO_ABS,
	VG_MOVE_TO_ABS, VG_CUBIC_TO_ABS, VG_SCUBIC_TO_ABS,
	VG_MOVE_TO_ABS, VG_SCWARC_TO_ABS,
	VG_MOVE_TO_ABS, VG_SCCWARC_TO_ABS,
	VG_MOVE_TO_ABS, VG_LCWARC_TO_ABS,
	VG_MOVE_TO_ABS, VG_LCCWARC_TO_ABS,
};

VGfloat g_afPathCoords[] = {
		0.1f, 0.2f,		0.9f, 0.8f,
		0.1f, 0.5f,		0.9f,
		0.5f, 0.2f,		0.8f,
		0.1f, 0.8f,		0.5f, 0.0f,		0.9f, 0.7f,
		0.1f, 0.8f,		0.4f, 0.1f,		0.6f, 0.9f,		0.9f, 0.2f,
		0.1f, 0.4f,		0.3f, 0.3f,		0.5f, 0.5f,		0.9f, 0.5f,
		0.1f, 0.5f,		0.2f, 0.3f,		0.4f, 0.7f,		0.5f, 0.5f,		0.8f, 0.9f,		0.9f, 0.5f,
		0.4f, 0.3f,		0.3f, 0.25f,	0.1f,			0.6f, 0.7f,
};

/****************************************************************************
** Class CSegmentTypes
****************************************************************************/
class CSegmentTypes : public PVRShell
{
private:
	VGPath m_avgPaths[g_i32NumSegmentTypes];

	CPVRTPrintVG m_PrintVg;

	int m_ui32StartTime;

public:
	CSegmentTypes() {}

	/* PVRShell functions */
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	/****************************************************************************
	** Function Definitions
	****************************************************************************/

	void CreatePaths();
	void DestroyPaths();
};

/*******************************************************************************
 * Function Name  : CreatePaths
 * Description    : Creates OpenVG paths demonstrating different segment types.
 *                  This function will be called by InitView once.
 *******************************************************************************/
void CSegmentTypes::CreatePaths()
{
	/*
	Create one path for every segment type
	*/
	for(int i = 0; i < g_i32NumSegmentTypes; ++i)
	{
		// Create a path handle...
		m_avgPaths[i] = vgCreatePath(
			VG_PATH_FORMAT_STANDARD,
			VG_PATH_DATATYPE_F,
			1.0f, 0.0f,
			g_asPaths[i].i32NumSegments,
			g_asPaths[i].i32NumCoord,
			VG_PATH_CAPABILITY_APPEND_TO);

		// ... and populate it with data
		vgAppendPathData(
			m_avgPaths[i],
			g_asPaths[i].i32NumSegments,
			&g_aui8PathSegments[g_asPaths[i].i32StartSegment],
			&g_afPathCoords[g_asPaths[i].i32StartCoord]);

		/*
		Path capabilities should be removed when no longer needed. The OpenVG
		implementation might work more efficiently if it knows that path data
		will not change.
		*/
		vgRemovePathCapabilities(m_avgPaths[i], VG_PATH_CAPABILITY_APPEND_TO);
	}
}


/*******************************************************************************
 * Function Name  : DestroyPaths
 * Description    : Destroys OpenVG paths created by CreatePaths.
 *                  This function will be called by ReleaseView once.
 *******************************************************************************/
void CSegmentTypes::DestroyPaths()
{
	for(int i = 0; i < g_i32NumSegmentTypes; ++i)
	{
		vgDestroyPath(m_avgPaths[i]);
		m_avgPaths[i] = 0;
	}
}

/*******************************************************************************
 * Function Name  : InitApplication
 * Returns        : true if no error occured
 * Description    : Code in InitApplication() will be called by the Shell ONCE per
 *					run, early on in the execution of the program.
 *					Used to initialise variables that are not dependent on the
 *					rendering context (e.g. external modules, loading meshes, etc.)
 *******************************************************************************/
bool CSegmentTypes::InitApplication()
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
bool CSegmentTypes::QuitApplication()
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
bool CSegmentTypes::InitView()
{
	/*
	Initially, the OpenVG coordinate system is based on the output resolution.
	To get a device independent coordinate system, we need to apply a
	transformation. Scaling by the output resolution means that coordinates
	between (0, 0) and (1, 1) will be visible on screen.

	It should be noted, however, that different aspect ratios usually require
	special attention regarding the layout of elements on screen.
	*/
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
	vgLoadIdentity();
	vgScale((float)PVRShellGet(prefWidth), (float)PVRShellGet(prefHeight));

	/*
	Create paths that represent different segment types
	*/
	CreatePaths();

	vgSetf(VG_STROKE_LINE_WIDTH, 3.0f / PVRShellGet(prefHeight));

	/*
	The clear colour will be used whenever calling vgClear(). The colour is given
	as non-premultiplied sRGBA.
	*/
	VGfloat afClearColour[] = { 0.6f, 0.8f, 1.0f, 1.0f };
	vgSetfv(VG_CLEAR_COLOR, 4, afClearColour);

	/*
	Initialise custom text drawing
	*/
	m_PrintVg.Initialize(PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	m_ui32StartTime = PVRShellGetTime();

	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns		  : true if no error occured
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool CSegmentTypes::ReleaseView()
{
	DestroyPaths();

	m_PrintVg.Terminate();

	return true;
}


/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool CSegmentTypes::RenderScene()
{
	// Clear the screen with clear color.
	vgClear(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	unsigned int ui32TimeSinceStart = PVRShellGetTime() - m_ui32StartTime;
	// toggle segment type every 3 seconds
	int i32SegmentType = (ui32TimeSinceStart % (3000 * g_i32NumSegmentTypes)) / 3000;

	vgDrawPath(m_avgPaths[i32SegmentType], VG_STROKE_PATH);

	m_PrintVg.DisplayDefaultTitle("SegmentTypes",g_asPaths[i32SegmentType].pszName, ePVRTPrint3DLogoIMG);
	return true;
}


/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialise a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new CSegmentTypes();
}

/*****************************************************************************
 End of file (OVGSegmentTypes.cpp)
*****************************************************************************/
