/******************************************************************************

 @File         OGLESOptimizeMesh.cpp

 @Title        OptimizeMesh

 @Copyright    Copyright (C) 2005 - 2008 by Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows OptimizeMeshes with PVRTC compression. In the case of OGLES
               Lite we have to convert the data to fixed point format. So we
               allocate extra buffers for the data.

******************************************************************************/
#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szModelTexFile[] = "model_texture.pvr";

// POD File
#ifdef PVRT_FIXED_POINT_ENABLE
	const char c_szSphere[]		= "Sphere_fixed.pod";
	const char c_szSphereOpt[]	= "SphereOpt_fixed.pod";
#else
	const char c_szSphere[]		= "Sphere_float.pod";
	const char c_szSphereOpt[]	= "SphereOpt_float.pod";
#endif

/*****************************************************************************
** Build options
*****************************************************************************/

/*
	Define this for a demonstration of triangle stripping at load time.
	Normally, an app wouldn't strip meshes at load time - it takes too much
	time. This is purely for demonstration purposes.
*/
//#define ENABLE_LOAD_TIME_STRIP

/****************************************************************************
** Constants
****************************************************************************/
const unsigned int m_ui32VBONo = 2;

#ifdef ENABLE_LOAD_TIME_STRIP
const unsigned int m_ui32IndexVBONo = 3;
const unsigned int m_ui32PageNo		= 3;
#else
const unsigned int m_ui32IndexVBONo = 2;
const unsigned int m_ui32PageNo		= 2;
#endif

const VERTTYPE g_fViewDistance = f2vt(35.0f);

// Times in milliseconds
const unsigned int g_ui32TimeAutoSwitch = 4000;
const unsigned int g_ui32TimeFPSUpdate  = 500;

// Assuming a 4:3 aspect ratio:
const VERTTYPE g_fCameraNear = f2vt(4.0f);
const VERTTYPE g_fCameraFar	= f2vt(5000.0f);

/****************************************************************************
** Class: OGLESOptimizeMesh
****************************************************************************/
class OGLESOptimizeMesh : public PVRShell
{
	// Print3D, Extension and POD Class Objects
	CPVRTPrint3D 		m_Print3D;
	CPVRTModelPOD		m_Model;	// Model
	CPVRTModelPOD		m_ModelOpt;	// Triangle optimized model

#ifdef ENABLE_LOAD_TIME_STRIP
	unsigned short		*m_pNewIdx;		// Optimized model's indices

	// There is some processing to be done once only; this flags marks whether it has been done.
	int	m_i32Init;
#endif

	// OpenGL handles for textures and VBOs
	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;
	GLuint	m_Texture;

	// View and Projection Matrices
	PVRTMATRIX	m_mView, m_mProj;
	VERTTYPE	m_fViewAngle;

	// Used to switch mode (not optimized / optimized) after a while
	unsigned long	m_ui32SwitchTimeDiff;
	int				m_i32Page;

	// Time variables
	unsigned long	m_ui32LastTime, m_ui32TimeDiff;

	// FPS variables
	unsigned long	m_ui32FPSTimeDiff, m_ui32FPSFrameCnt;
	float			m_fFPS;

public:
	OGLESOptimizeMesh() : 	m_puiVbo(0),
							m_puiIndexVbo(0)
	{
	}

	// PVRShell functions
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	/****************************************************************************
	** Function Definitions
	****************************************************************************/
	void DrawModel( int iOptim );
	void LoadVbos();
#ifdef ENABLE_LOAD_TIME_STRIP
	void StripMesh();
#endif
};

/*******************************************************************************
 * Function Name  : InitApplication
 * Inputs		  :
 * Returns        : true if no error occured
 * Description    : Code in InitApplication() will be called by the Shell ONCE per
 *					run, early on in the execution of the program.
 *					Used to initialize variables that are not dependant on the
 *					rendering context (e.g. external modules, loading meshes, etc.)
 *******************************************************************************/
bool OGLESOptimizeMesh::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Set some parameters in the Shell
	PVRShellSet(prefAppName, "OptimizeMesh");
	PVRShellSet(prefSwapInterval, 0);

	// Load POD File Data
	
	// Load the meshes
	if(m_Model.ReadFromFile(c_szSphere) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load Sphere_*.pod!");
		return false;
	}

	if(m_ModelOpt.ReadFromFile(c_szSphereOpt) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load SphereOpt_*.pod!");
		return false;
	}

#ifdef ENABLE_LOAD_TIME_STRIP
	// Create a stripped version of the mesh at load time
	m_i32Init = 2;
#endif

	// Init values to defaults
	m_i32Page = 0;

	return true;
}


/*******************************************************************************
 * Function Name  : QuitApplication
 * Returns        : true if no error occured
 * Description    : Code in QuitApplication() will be called by the Shell ONCE per
 *					run, just before exiting the program.
 *******************************************************************************/
bool OGLESOptimizeMesh::QuitApplication()
{
	m_Model.Destroy();
	m_ModelOpt.Destroy();

	delete[] m_puiVbo;
	delete[] m_puiIndexVbo;

#ifdef ENABLE_LOAD_TIME_STRIP
	free(m_pNewIdx);
#endif
	return true;
}

/*******************************************************************************
 * Function Name  : InitView
 * Inputs		  :
 * Returns        : true if no error occured
 * Description    : Code in InitView() will be called by the Shell upon a change
 *					in the rendering context.
 *					Used to initialize variables that are dependant on the rendering
 *					context (e.g. textures, vertex buffers, etc.)
 *******************************************************************************/
bool OGLESOptimizeMesh::InitView()
{
	SPVRTContext sContext;

	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Init Print3D to display text on screen
	if(m_Print3D.SetTextures(&sContext, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D.\n");
		return false;
	}

	/******************************
	** Create Textures           **
	*******************************/
	if(PVRTTextureLoadFromPVR(c_szModelTexFile, &m_Texture) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load texture for model.\n");
		return false;
	}

	/*********************************
	** View and Projection Matrices **
	*********************************/

	// Get Camera info from POD file
	PVRTVec3 From, To;
	VERTTYPE fFOV;

	if(m_Model.nNumCamera)
	{
		// Get Camera data from POD Geometry File
		fFOV = m_Model.GetCameraPos(From, To, 0);
		fFOV = VERTTYPEMUL(fFOV, f2vt(0.75f));		// Convert from horizontal FOV to vertical FOV (0.75 assumes a 4:3 aspect ratio)
	}
	else
	{
		fFOV = f2vt(PVRT_PIf / 6);
	}

	// View
	m_mView = PVRTMat4::LookAtRH(From, To, PVRTVec3(f2vt(0.0f), f2vt(1.0f), f2vt(0.0f)));

	// Projection
	m_mProj = PVRTMat4::PerspectiveFovRH(fFOV, f2vt((float) PVRShellGet(prefWidth) / (float) PVRShellGet(prefHeight)), g_fCameraNear, g_fCameraFar, PVRTMat4::OGL, bRotate);

	glMatrixMode(GL_PROJECTION);
	myglLoadMatrix(m_mProj.f);

	/******************************
	** GENERIC RENDER STATES     **
	******************************/

	// The Type Of Depth Test To Do
	glDepthFunc(GL_LEQUAL);

	// Enables Depth Testing
	glEnable(GL_DEPTH_TEST);

	// Enables Smooth Color Shading
	glShadeModel(GL_SMOOTH);

	// Define front faces
	glFrontFace(GL_CW);

	// Sets the clear colour
	myglClearColor(f2vt(0.6f), f2vt(0.8f), f2vt(1.0f), f2vt(1.0f));

	// Reset the model view matrix to position the light
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Setup timing variables
	m_ui32LastTime = PVRShellGetTime();
	m_ui32FPSFrameCnt = 0;
	m_fFPS = 0;
	m_fViewAngle = 0;
	m_ui32SwitchTimeDiff = 0;

#ifndef ENABLE_LOAD_TIME_STRIP
	LoadVbos();
#endif

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLESOptimizeMesh::LoadVbos()
{
	if(!m_puiVbo)
		m_puiVbo = new GLuint[m_ui32VBONo];

	if(!m_puiIndexVbo)
		m_puiIndexVbo = new GLuint[m_ui32IndexVBONo];

	glGenBuffers(m_ui32VBONo, m_puiVbo);
	glGenBuffers(m_ui32IndexVBONo, m_puiIndexVbo);

	// Create vertex buffer for Model

	// Load vertex data into buffer object
	unsigned int uiSize = m_Model.pMesh[0].nNumVertex * m_Model.pMesh[0].sVertex.nStride;

	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[0]);
	glBufferData(GL_ARRAY_BUFFER, uiSize, m_Model.pMesh[0].pInterleaved, GL_STATIC_DRAW);

	// Load index data into buffer object if available
	uiSize = PVRTModelPODCountIndices(m_Model.pMesh[0]) * sizeof(GLshort);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, m_Model.pMesh[0].sFaces.pData, GL_STATIC_DRAW);
	
	// Create vertex buffer for ModelOpt

	// Load vertex data into buffer object
	uiSize = m_ModelOpt.pMesh[0].nNumVertex * m_ModelOpt.pMesh[0].sVertex.nStride;

	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[1]);
	glBufferData(GL_ARRAY_BUFFER, uiSize, m_ModelOpt.pMesh[0].pInterleaved, GL_STATIC_DRAW);

	// Load index data into buffer object if available
	uiSize = PVRTModelPODCountIndices(m_ModelOpt.pMesh[0]) * sizeof(GLshort);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, m_ModelOpt.pMesh[0].sFaces.pData, GL_STATIC_DRAW);

#ifdef ENABLE_LOAD_TIME_STRIP
	// Creat index data for the load time stripping
	uiSize = PVRTModelPODCountIndices(m_Model.pMesh[0]) * sizeof(GLshort);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, m_pNewIdx, GL_STATIC_DRAW);
#endif

	// Unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : Nothing
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool OGLESOptimizeMesh::ReleaseView()
{
	// Release all Textures
	glDeleteTextures(1, &m_Texture);

	// Release the Print3D textures and windows
	m_Print3D.DeleteAllWindows();
	m_Print3D.ReleaseTextures();
	return true;
}

/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool OGLESOptimizeMesh::RenderScene()
{
	unsigned long ui32Time;

	// Clear the depth and frame buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef ENABLE_LOAD_TIME_STRIP
	/*
		Show a message on-screen then generate the necessary data on the
		second frame.
	*/
	if(m_i32Init)
	{
		--m_i32Init;

		if(m_i32Init)
		{
			m_Print3D.DisplayDefaultTitle("OptimizeMesh", "Generating data...", ePVRTPrint3DSDKLogo);
			m_Print3D.Flush();
			return true;
		}

		StripMesh();
		LoadVbos();
	}
#endif

	//	Time
	ui32Time = PVRShellGetTime();
	m_ui32TimeDiff = ui32Time - m_ui32LastTime;
	m_ui32LastTime = ui32Time;

	// FPS
	++m_ui32FPSFrameCnt;
	m_ui32FPSTimeDiff += m_ui32TimeDiff;

	if(m_ui32FPSTimeDiff >= g_ui32TimeFPSUpdate)
	{
		m_fFPS = m_ui32FPSFrameCnt * 1000.0f / (float) m_ui32FPSTimeDiff;
		m_ui32FPSFrameCnt = 0;
		m_ui32FPSTimeDiff = 0;
	}

	// Change mode when necessary
	m_ui32SwitchTimeDiff += m_ui32TimeDiff;

	if((m_ui32SwitchTimeDiff > g_ui32TimeAutoSwitch) || PVRShellIsKeyPressed(PVRShellKeyNameACTION1))
	{
		m_ui32SwitchTimeDiff = 0;
		++m_i32Page;

		if(m_i32Page >= (int) m_ui32PageNo)
			m_i32Page = 0;
	}

	PVRTVec3 From;
	VERTTYPE fFactor;

	From.x = VERTTYPEMUL(g_fViewDistance, PVRTCOS(m_fViewAngle));
	From.y = f2vt(0.0f);
	From.z = VERTTYPEMUL(g_fViewDistance, PVRTSIN(m_fViewAngle));

	// Increase the rotation
	fFactor = f2vt(0.005f * (float) m_ui32TimeDiff);
	m_fViewAngle += fFactor;

	// Ensure it doesn't grow huge and lose accuracy over time
	if(m_fViewAngle > PVRT_PI)
		m_fViewAngle -= PVRT_TWO_PI;

	// Compute and set the matrix
	m_mView = PVRTMat4::LookAtRH(From, PVRTVec3(0,0,0), PVRTVec3(0,f2vt(1),0));

	glMatrixMode(GL_MODELVIEW);
	myglLoadMatrix(m_mView.f);

	// Draw the model
	DrawModel(m_i32Page);

	// Display the frame rate
	char pTitle[512];
	char *pDesc;

	sprintf(pTitle, "Optimize Mesh %.1ffps", m_fFPS);

	// Print text on screen
	switch(m_i32Page)
	{
	default:
		pDesc = "Indexed Triangle List: Unoptimized";
		break;
	case 1:
		pDesc = "Indexed Triangle List: Optimized (at export time)";
		break;
#ifdef ENABLE_LOAD_TIME_STRIP
	case 2:
		pDesc = "Indexed Triangle List: Optimized (at load time)";
		break;
#endif
	}

	m_Print3D.DisplayDefaultTitle(pTitle, pDesc, ePVRTPrint3DSDKLogo);

	// Flush all Print3D commands
	m_Print3D.Flush();

	return true;
}

/*******************************************************************************
 * Function Name  : DrawModel
 * Inputs		  : iOptim
 * Description    : Draws the balloon
 *******************************************************************************/
void OGLESOptimizeMesh::DrawModel( int iOptim )
{
	SPODMesh *pMesh;
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	PVRTMATRIX worldMatrix;
	m_Model.GetWorldMatrix(worldMatrix, m_Model.pNode[0]);
	myglMultMatrix(worldMatrix.f);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_Texture);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Enable States
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Set Data Pointers and bing the VBOs
	switch(iOptim)
	{
	default:
		pMesh = m_Model.pMesh;

		glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[0]);
		break;
	case 1:
		pMesh = m_ModelOpt.pMesh;

		glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[1]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[1]);
		break;
#ifdef ENABLE_LOAD_TIME_STRIP
	case 2:
		pMesh = m_Model.pMesh;

		glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[2]);
		break;
#endif
	}

	// Used to display interleaved geometry
	glVertexPointer(3, VERTTYPEENUM, pMesh->sVertex.nStride, pMesh->sVertex.pData);
	glTexCoordPointer(2, VERTTYPEENUM, pMesh->psUVW[0].nStride, pMesh->psUVW[0].pData);

	// Draw
	glDrawElements(GL_TRIANGLES, pMesh->nNumFaces * 3, GL_UNSIGNED_SHORT, 0);

	// Disable States
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	// unbind the vertex buffers as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glPopMatrix();
}

#ifdef ENABLE_LOAD_TIME_STRIP
/*******************************************************************************
 * Function Name  : StripMesh
 * Description    : Generates the stripped-at-load-time list.
 *******************************************************************************/
void OGLESOptimizeMesh::StripMesh()
{
	// Make a copy of the indices as we want to keep the original
	m_pNewIdx = (unsigned short*)malloc(sizeof(unsigned short)*m_Model.pMesh->nNumFaces*3);
	memcpy(m_pNewIdx, m_Model.pMesh->sFaces.pData, sizeof(unsigned short)*m_Model.pMesh->nNumFaces*3);

	PVRTTriStripList(m_pNewIdx, m_Model.pMesh->nNumFaces);
}
#endif

/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialize a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLESOptimizeMesh();
}

/*****************************************************************************
 End of file (OGLESOptimizeMesh.cpp)
*****************************************************************************/
