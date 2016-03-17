/******************************************************************************

 @File         OGLESSkybox.cpp

 @Title        Skybox

 @Copyright    Copyright (C) 2005 - 2008 by Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows Skyboxes with PVRTC compression. In the case of OGLES Lite
               we have to convert the data to fixed point format. So we allocate
               extra buffers for the data.

******************************************************************************/
#include <math.h>
#include <string.h>
#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szBalloonTexFile[]  = "balloon.pvr";

// POD File
#ifdef PVRT_FIXED_POINT_ENABLE
const char c_szSceneFile[]	= "Balloon_fixed.pod";
#else
const char c_szSceneFile[]	= "Balloon_float.pod";
#endif

/****************************************************************************
 ** DEFINES                                                                **
 ****************************************************************************/

// Assuming a 4:3 aspect ratio
const float g_fCameraNear = 4.0f;
const float g_fCameraFar = 500.0f;

const float g_fSkyboxZoom = 150.0f;
const bool  g_bSkyboxAdjustUVs = true;

/****************************************************************************
** Class: OGLESSkybox
****************************************************************************/
class OGLESSkybox : public PVRShell
{
	// OpenGL handles for textures and VBOs
	GLuint m_ui32BalloonTex;
	GLuint m_ui32SkyboxTex[6];

	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

	// Print3D, POD Scene and Extensions
	CPVRTPrint3D 		m_Print3D;
	CPVRTModelPOD		m_Scene;
	CPVRTglesExt		m_Extensions;

	// View and Projection Matrices
	PVRTMat4	m_mView, m_mProj;

	// Skybox
	VERTTYPE* m_pSkyboxVertices;
	VERTTYPE* m_pSkyboxUVs;

	// View Variables
	VERTTYPE m_fViewDistance, m_fViewAmplitude, m_fViewUpDownAmplitude;
	float m_fViewAngle;
	float m_fViewAmplitudeAngle;
	float m_fViewUpDownAngle;

	// Vectors for calculating the view matrix and saving the camera position
	PVRTVec3 m_fCameraTo, m_fCameraUp, m_fCameraPos;

public:
	OGLESSkybox() : m_puiVbo(0),
					m_puiIndexVbo(0),
					m_fViewDistance(f2vt(100.0f)),
					m_fViewAmplitude(f2vt(60.0f)),
					m_fViewUpDownAmplitude(f2vt(50.0f)),
					m_fViewAngle(PVRT_PI_OVER_TWOf),
					m_fViewAmplitudeAngle(f2vt(0.0f)),
					m_fViewUpDownAngle(f2vt(0.0f))
	{
		m_fCameraTo = PVRTVec3(f2vt(0), f2vt(0), f2vt(0));
		m_fCameraUp = PVRTVec3(f2vt(0), f2vt(1), f2vt(0));
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
	void ComputeViewMatrix();
	void DrawSkybox();
	void DrawBalloon();
	void LoadVbos();
};


/*******************************************************************************
 * Function Name  : InitApplication
 * Inputs		  : argc, *argv[], uWidth, uHeight
 * Returns        : true if no error occured
 * Description    : Code in InitApplication() will be called by the Shell ONCE per
 *					run, early on in the execution of the program.
 *					Used to initialize variables that are not dependant on the
 *					rendering context (e.g. external modules, loading meshes, etc.)
 *******************************************************************************/
bool OGLESSkybox::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	//	Loads the scene from the .pod file into a CPVRTModelPOD object.

	// Load the mesh
	if(m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load Balloon_*.pod!");
		return false;
	}

	return true;
}


/*******************************************************************************
 * Function Name  : QuitApplication
 * Returns        : true if no error occured
 * Description    : Code in QuitApplication() will be called by the Shell ONCE per
 *					run, just before exiting the program.
 *******************************************************************************/
bool OGLESSkybox::QuitApplication()
{
	m_Scene.Destroy();

	delete[] m_puiVbo;
	delete[] m_puiIndexVbo;

    return true;
}

/*******************************************************************************
 * Function Name  : InitView
 * Inputs		  : uWidth, uHeight
 * Returns        : true if no error occured
 * Description    : Code in InitView() will be called by the Shell upon a change
 *					in the rendering context.
 *					Used to initialize variables that are dependant on the rendering
 *					context (e.g. textures, vertex buffers, etc.)
 *******************************************************************************/
bool OGLESSkybox::InitView()
{
	SPVRTContext sContext;
	char filename[16];

	/******************************
	** Create Textures           **
	*******************************/
	for(int i = 0; i < 6; ++i)
	{
		sprintf(filename, "skybox%d.pvr", (i+1));

		if(PVRTTextureLoadFromPVR(filename, &m_ui32SkyboxTex[i]) != PVR_SUCCESS)
		{
			PVRShellOutputDebug("ERROR: Failed to load texture for skybox.\n");
		}

		myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	if(PVRTTextureLoadFromPVR(c_szBalloonTexFile, &m_ui32BalloonTex) != PVR_SUCCESS)
	{
		PVRShellOutputDebug("ERROR: Failed to load balloon texture.\n");
	}

	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Is the screen rotated?
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Init Print3D to display text on screen
	if(m_Print3D.SetTextures(&sContext, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to initialise Print3D\n");
		return false;
	}

	//	Initialize VBO data
	LoadVbos();

	/*********************/
	/* Create the skybox */
	/*********************/
	PVRTCreateSkybox(g_fSkyboxZoom, g_bSkyboxAdjustUVs, 512, &m_pSkyboxVertices, &m_pSkyboxUVs );

	/**********************
	** Projection Matrix **
	**********************/

	// Projection
	m_mProj = PVRTMat4::PerspectiveFovRH(f2vt(PVRT_PIf / 6), f2vt((float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight)), f2vt(g_fCameraNear), f2vt(g_fCameraFar), PVRTMat4::OGL, bRotate);
	
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

	// Enable texturing
	glEnable(GL_TEXTURE_2D);

	// Define front faces
	glFrontFace(GL_CW);

	// Disable Blending
	glDisable(GL_BLEND);

	// Enables texture clamping
	myglTexParameter( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	myglTexParameter( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	// Set the clear colour
	myglClearColor(f2vt(0.5f), f2vt(0.5f), f2vt(0.5f), f2vt(1.0f));

	// Reset the model view matrix to position the light
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Setup ambiant light
    glEnable(GL_LIGHTING);
	VERTTYPE fLightGlobalAmbient[] = {f2vt(0.4f), f2vt(0.4f), f2vt(0.4f), f2vt(1.0f)};
    myglLightModelv(GL_LIGHT_MODEL_AMBIENT, fLightGlobalAmbient);

	// Setup a directional light source
	VERTTYPE fLightPosition[] = {f2vt(+0.7f), f2vt(+1.0f), f2vt(-0.2f), f2vt(0.0f)};
    VERTTYPE fLightAmbient[]  = {f2vt(0.6f), f2vt(0.6f), f2vt(0.6f), f2vt(1.0f)};
    VERTTYPE fLightDiffuse[]  = {f2vt(1.0f), f2vt(1.0f), f2vt(1.0f), f2vt(1.0f)};
    VERTTYPE fLightSpecular[] = {f2vt(1.0f), f2vt(1.0f), f2vt(1.0f), f2vt(1.0f)};

    glEnable(GL_LIGHT0);
    myglLightv(GL_LIGHT0, GL_POSITION, fLightPosition);
    myglLightv(GL_LIGHT0, GL_AMBIENT,  fLightAmbient);
    myglLightv(GL_LIGHT0, GL_DIFFUSE,  fLightDiffuse);
    myglLightv(GL_LIGHT0, GL_SPECULAR, fLightSpecular);

	// Setup the balloon material
	VERTTYPE fMatAmbient[] = {f2vt(0.7f), f2vt(0.7f), f2vt(0.7f), f2vt(1.0f)};
	VERTTYPE fMatDiffuse[] = {f2vt(1.0f), f2vt(1.0f), f2vt(1.0f), f2vt(1.0f)};
	VERTTYPE fMatSpecular[] = {f2vt(0.0f), f2vt(0.0f), f2vt(0.0f), f2vt(0.0f)};

	myglMaterialv(GL_FRONT_AND_BACK, GL_AMBIENT,  fMatAmbient);
	myglMaterialv(GL_FRONT_AND_BACK, GL_DIFFUSE,  fMatDiffuse);
	myglMaterialv(GL_FRONT_AND_BACK, GL_SPECULAR, fMatSpecular);
	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLESSkybox::LoadVbos()
{
	if(!m_puiVbo)
		m_puiVbo = new GLuint[m_Scene.nNumMesh];

	if(!m_puiIndexVbo)
		m_puiIndexVbo = new GLuint[m_Scene.nNumMesh];

	/*
		Load vertex data of all meshes in the scene into VBOs

		The meshes have been exported with the "Interleave Vectors" option,
		so all data is interleaved in the buffer at pMesh->pInterleaved.
		Interleaving data improves the memory access pattern and cache efficiency,
		thus it can be read faster by the hardware.
	*/

	glGenBuffers(m_Scene.nNumMesh, m_puiVbo);

	for(unsigned int i = 0; i < m_Scene.nNumMesh; ++i)
	{
		// Load vertex data into buffer object
		SPODMesh& Mesh = m_Scene.pMesh[i];
		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;

		glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);

		// Load index data into buffer object if available
		m_puiIndexVbo[i] = 0;

		if(Mesh.sFaces.pData)
		{
			glGenBuffers(1, &m_puiIndexVbo[i]);
			uiSize = PVRTModelPODCountIndices(Mesh) * sizeof(GLshort);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : Nothing
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool OGLESSkybox::ReleaseView()
{
	// Release all Textures
	glDeleteTextures(1, &m_ui32BalloonTex);

	for(int i = 0; i < 6; ++i)
		glDeleteTextures(1, &m_ui32SkyboxTex[i]);

	// Destroy the skybox
	PVRTDestroySkybox(m_pSkyboxVertices, m_pSkyboxUVs);

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
bool OGLESSkybox::RenderScene()
{
	// Clear the depth and frame buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Calculate the model view matrix turning around the balloon
	ComputeViewMatrix();

	// Enable States
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Draw the skybox
	DrawSkybox();

	// The balloon has normals
	glEnableClientState(GL_NORMAL_ARRAY);

	// Draw the balloon
	DrawBalloon();

	// Enable States
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	// Print text on screen
	m_Print3D.DisplayDefaultTitle("Skybox", "Skybox with PVRTC", ePVRTPrint3DSDKLogo);

	// Flush all Print3D commands
	m_Print3D.Flush();
	return true;
}

/*******************************************************************************
 * Function Name  : ComputeViewMatrix
 * Description    : Calculate the view matrix turning around the balloon
 *******************************************************************************/
void OGLESSkybox::ComputeViewMatrix()
{
	// Calculate the distance to balloon
	VERTTYPE fDistance = m_fViewDistance + VERTTYPEMUL(m_fViewAmplitude, f2vt(sin(m_fViewAmplitudeAngle)));
	fDistance = VERTTYPEMUL(fDistance, f2vt(0.2f));
	m_fViewAmplitudeAngle += 0.004f;

	// Calculate the vertical position of the camera
	VERTTYPE fUpdown = VERTTYPEMUL(m_fViewUpDownAmplitude, f2vt(sin(m_fViewUpDownAngle)));
	fUpdown = VERTTYPEMUL(fUpdown, f2vt(0.2f));
	m_fViewUpDownAngle += 0.005f;

	// Calculate the angle of the camera around the balloon
	m_fCameraPos.x = VERTTYPEMUL(fDistance, f2vt(cos(m_fViewAngle)));
	m_fCameraPos.y = fUpdown;
	m_fCameraPos.z = VERTTYPEMUL(fDistance, f2vt(sin(m_fViewAngle)));

	m_fViewAngle += 0.003f;

	// Compute and set the matrix
	m_mView = PVRTMat4::LookAtRH(m_fCameraPos, m_fCameraTo, m_fCameraUp);

	glMatrixMode(GL_MODELVIEW);
	myglLoadMatrix(m_mView.f);
}

/*******************************************************************************
 * Function Name  : DrawSkybox
 * Description    : Draws the skybox
 *******************************************************************************/
void OGLESSkybox::DrawSkybox()
{
	// Only use the texture color
	myglTexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	// Draw the skybox around the camera position
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	myglTranslate(-m_fCameraPos.x, -m_fCameraPos.y, -m_fCameraPos.z);

	// Disable lighting
	glDisable(GL_LIGHTING);

	// Enable backface culling for skybox; need to ensure skybox faces are set up properly
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	for(int i = 0; i < 6; ++i)
	{
		// Set Data Pointers
		glBindTexture(GL_TEXTURE_2D, m_ui32SkyboxTex[i]);
		glVertexPointer(3, VERTTYPEENUM, sizeof(VERTTYPE) * 3, &m_pSkyboxVertices[i*4*3]);
		glTexCoordPointer(2, VERTTYPEENUM, sizeof(VERTTYPE) * 2, &m_pSkyboxUVs[i*4*2]);

		// Draw 
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	glPopMatrix();
}

/*******************************************************************************
 * Function Name  : DrawBalloon
 * Description    : Draws the balloon
 *******************************************************************************/
void OGLESSkybox::DrawBalloon()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	PVRTMat4 World;
	m_Scene.GetWorldMatrix(World, m_Scene.pNode[0]);
	myglMultMatrix(World.f);

	// Modulate with vertex colour
	myglTexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Enable lighting
	glEnable(GL_LIGHTING);

	// Bind the Texture
	glBindTexture(GL_TEXTURE_2D, m_ui32BalloonTex);

	// Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	SPODMesh& Mesh = m_Scene.pMesh[0];

	// Bind the vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[0]);

	// Setup pointers
	glVertexPointer(3, VERTTYPEENUM, Mesh.sVertex.nStride, Mesh.sVertex.pData);
	glTexCoordPointer(2, VERTTYPEENUM, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);
	glNormalPointer(VERTTYPEENUM, Mesh.sNormals.nStride, Mesh.sNormals.pData);

	glDrawElements(GL_TRIANGLES, Mesh.nNumFaces * 3, GL_UNSIGNED_SHORT, 0);

	// unbind the vertex buffers as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glPopMatrix();
}

/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialize a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLESSkybox();
}

/*****************************************************************************
 End of file (OGLESSkybox.cpp)
*****************************************************************************/
