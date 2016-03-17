/******************************************************************************

 @File         OVGScissors.cpp

 @Title        OpenVG Scissors using PVRShell

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     Independent

 @Description  Shows how to use scissors in OpenVG

******************************************************************************/
#include "PVRShell.h"
#include "OVGTools.h"
#include <math.h>

/****************************************************************************
** Constants
****************************************************************************/

/****************************************************************************
** Class CScissors
****************************************************************************/
class CScissors : public PVRShell
{
private:
	VGPath m_vgPath;
	VGPath m_avgBox[2];
	VGPaint m_vgColorPaint;
	double m_dAngle;
	unsigned int m_ui32Time;
	CPVRTPrintVG m_PrintVG;

public:
	CScissors() {}
	void CreatePath();

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

void CScissors::CreatePath()
{
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
 * Function Name  : InitApplication
 * Returns        : true if no error occured
 * Description    : Code in InitApplication() will be called by the Shell ONCE per
 *					run, early on in the execution of the program.
 *					Used to initialise variables that are not dependent on the
 *					rendering context (e.g. external modules, loading meshes, etc.)
 *******************************************************************************/
bool CScissors::InitApplication()
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
bool CScissors::QuitApplication()
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
bool CScissors::InitView()
{
	//Create a path so we have something to look at.
	CreatePath();

	// Create a paint so the path can be filled.
	m_vgColorPaint = vgCreatePaint();
	vgSetParameteri(m_vgColorPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetColor(m_vgColorPaint, PVRTRGBA(255,255,170,255));

	/*
	Set up two more paths, these ones will act as borders to the scissor rectangles so we can see where
	they are.
	*/
	static VGubyte s_aui8PathSegments[] = {
		VG_MOVE_TO,
		VG_LINE_TO,
		VG_LINE_TO,
		VG_LINE_TO,
		VG_CLOSE_PATH
	};

	static VGfloat s_afPathCoords[] = {
		-50,-50,
		-50, 50,
		 50, 50,
		 50,-50
	};

	// Create a path handle...
	m_avgBox[0] = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 5,4,VG_PATH_CAPABILITY_APPEND_TO);

	// ... and populate it with data
	vgAppendPathData(m_avgBox[0], 5, s_aui8PathSegments, s_afPathCoords);

	static VGfloat s_afPathCoords2[] = {
		-25.0f,-25.0f,
		-25.0f, 25.0f,
		 25.0f, 25.0f,
		 25.0f,-25.0f
	};

	// Create a path handle...
	m_avgBox[1] = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 5,4,VG_PATH_CAPABILITY_APPEND_TO);

	// ... and populate it with data
	vgAppendPathData(m_avgBox[1], 5, s_aui8PathSegments, s_afPathCoords2);

	//initialise dAngle
	m_dAngle = 0.0;
	m_ui32Time = PVRShellGetTime();

	m_PrintVG.Initialize(PVRShellGet(prefWidth), PVRShellGet(prefHeight));
	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : true if no error occured
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool CScissors::ReleaseView()
{
	// Cleanup: destroy the paths
	vgDestroyPath(m_avgBox[0]);
	vgDestroyPath(m_avgBox[1]);
	vgDestroyPath(m_vgPath);

	m_PrintVG.Terminate();
	return true;
}

/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool CScissors::RenderScene()
{
	unsigned int ui32Time = PVRShellGetTime();

	//increment the angle so the midpoints keep moving.
	if(ui32Time - m_ui32Time > 10)
	{
		m_ui32Time = ui32Time;
		m_dAngle += 0.15;
	}

	/*
	Set up the scissor rectangle. This doesn't have to be done every
	frame but since we are animating it we do.
	*/

	//Create some midpoints to build the rectangles around
	int i32X  = (int) ((PVRShellGet(prefWidth ) * 0.5f) + 70  * cos(m_dAngle));
	int i32Y  = (int) ((PVRShellGet(prefHeight) * 0.5f) + 70  * sin(m_dAngle));
	int i32X2 = (int) ((PVRShellGet(prefWidth ) * 0.5f) + 90  * cos(m_dAngle + 3.1415));
	int i32Y2 = (int) ((PVRShellGet(prefHeight) * 0.5f) + -50 * sin(m_dAngle + 3.1415));

	/*
	Create the scissor rectangles. To do this we put all the scissor rectangles into an int array.
	There will be 4 ints per rectangle. The first two values define the position of the bottom
	left hand corner. the second two describe the width and the height.
	*/
	VGint ai32Scissor[8] = { i32X  - 50, i32Y  - 50, 100, 100, //Scissor Rectangle 1
							i32X2 - 25, i32Y2 - 25, 50 , 50 };//Scissor Rectangle 2

	/*
	Set the scissor rects. As we are creating 2 scissor rects the second parameter is 8. If you were just
	setting 1 then this value would be 4.
	*/
	vgSetiv(VG_SCISSOR_RECTS, 8, ai32Scissor);

	//Create and set the clear colour
	VGfloat afClearColour[] = { 0.6f, 0.8f, 1.0f, 1.0f };
	vgSetfv(VG_CLEAR_COLOR, 4, afClearColour);

	//Clear the screen with the current clear colour.
	vgClear(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	// Set the paint you would like to fill the shape with
	vgSetPaint(m_vgColorPaint, VG_FILL_PATH);

	//Enable Scissoring
	vgSeti(VG_SCISSORING, VG_TRUE);

	//Set the clear colour
	afClearColour[0] = 0.8f;
	afClearColour[1] = 0.6f;
	afClearColour[2] = 1.0f;
	afClearColour[3] = 1.0f;
	vgSetfv(VG_CLEAR_COLOR, 4, afClearColour);

	/*
	Clear the whole screen with the current clear colour. As the scissor rects are enabled only their
	inards will be cleared.
	*/

	vgClear(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	//Switch to the path matrix mode
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);

	/*
	Draw the path with stroke and fill. As the scissor rects are enabled only the parts that fall
	within those rectangles will be visible.
	*/
	vgLoadIdentity();
	vgTranslate(PVRShellGet(prefWidth) * 0.5f, PVRShellGet(prefHeight) * 0.5f);
	vgDrawPath(m_vgPath, VG_STROKE_PATH | VG_FILL_PATH);

	//Disable Scissoring so the borders are visible
	vgSeti(VG_SCISSORING, VG_FALSE);

	//Switch to the path matrix mode
	vgLoadIdentity();

	//translate the path to the first scissor rectangle so we can have a nice border.
	vgTranslate((VGfloat) i32X, (VGfloat)i32Y);

	//draw path
	vgDrawPath(m_avgBox[0], VG_STROKE_PATH);

	//translate the path to the second scissor rectangle so we can have a nice border.
	vgLoadIdentity();
	vgTranslate((VGfloat) i32X2, (VGfloat) i32Y2);
	vgDrawPath(m_avgBox[1], VG_STROKE_PATH);

	//Reset the path2 matrix so the other path doesn't get affected
	vgLoadIdentity();

	m_PrintVG.DisplayDefaultTitle("Scissors", "", ePVRTPrint3DLogoIMG);
	return true;
}


/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialise a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new CScissors();
}

/*****************************************************************************
 End of file (OVGScissors.cpp)
*****************************************************************************/
