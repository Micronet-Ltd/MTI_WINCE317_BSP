/******************************************************************************

 @File         OGLESShadowTechniques.cpp

 @Title        Introducing the POD 3d file format

 @Copyright    Copyright (C) 2000 - 2008 by Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to load POD files and play the animation with basic
               lighting

******************************************************************************/
#include <string.h>
#include <GLES/egl.h>

#include "PVRShell.h"
#include "OGLESTools.h"
#include <math.h>


/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szKettleTexFile[]		= "Kettle.pvr";
const char c_szTableCoverTexFile[]	= "TableCover.pvr";
const char c_szBlobTexFile[]		= "Blob.pvr";

// POD File
/*
	We exported the scene from 3DSMax into two .pod files.
	One for the Common (floating point) profile, the other one for CommonLite (fixed point) profile.
*/
#ifdef PVRT_FIXED_POINT_ENABLE
const char c_szSceneFile[]	= "Scene_fixed.pod";
#else
const char c_szSceneFile[]	= "Scene_float.pod";
#endif

/******************************************************************************
 Defines
******************************************************************************/
const float	g_fCharWidth   = 45.0f;	// Used to draw the basic blob shadow
const float g_fFloorHeight = 0.5f;

// Camera constants. Used for making the projection matrix
const float g_fCameraNear = 10.0f;
const float g_fCameraFar  = 1000.0f;

const unsigned int g_ui32TextureSize = 128;

enum ESceneObjects
{
	eGround,
	eLight,
	eShadowCaster
};

enum ShadowModes
{
	BASEBLOBMODE,
	ADVANCEDBLOBMODE,
	PROJGEOMMODE,
	R2TEXMODE
};
/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESShadowTechniques : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// Texture handle
	GLuint	m_uiTableCover, m_uiKettle, m_uiBlobMap, m_uiShadow;

	// VBO Handles
	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// Projection and Model View matrices
	PVRTMat4		m_mProjection, m_mView;

	// Array to lookup the textures for each material in the scene
	GLuint*		m_puiTextures;

	PVRTVec3	m_fLightPos;

	PVRTMat4	m_mfloorShadow;
	PVRTMat4    m_mLightView;
	PVRTMat4	m_mObjectRotation;

	PVRTVec4 m_fPlane;
	PVRTVec3 m_fObjectCentre;
	unsigned int m_ui32Mode;
	unsigned int m_ui32Time;
	unsigned int m_ui32SwitchTime;

	EGLSurface m_PBufferSurface;

	// Render contexts, etc
	EGLDisplay m_CurrentDisplay;
	EGLContext m_CurrentContext;
	EGLSurface m_CurrentSurface;

	VERTTYPE m_fAngle;
	VERTTYPE m_fObjectAngle;

public:
	OGLESShadowTechniques() : m_puiVbo(0),
							  m_puiIndexVbo(0)
	{
	}

	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	bool DrawShadowTexture();
	void DrawMesh(SPODNode* pNode, bool bProjectTexture = false);
	void DrawBaseBlob(PVRTVec3 fCentre);
	void DrawAdvancedBlobShadow();
	void shadowMatrix(PVRTMat4 &shadowMat, const PVRTVec4 &vPlane, const PVRTVec4 &vlightPos);
	void findPlane(PVRTVec4 &plane, const PVRTVec3 &v0, const PVRTVec3 &v1, const PVRTVec3 &v2);
	void DrawProjectedShadow(SPODNode* pNode);
	bool RenderToTexture(SPODNode *pNode);
	bool RenderFromLightsView();
	EGLConfig SelectEGLConfig();
	void LoadVbos();
};


/*!****************************************************************************
 @Function		InitApplication
 @Return		bool		true if no error occured
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependant on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLESShadowTechniques::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Request PBuffer support
	PVRShellSet(prefPBufferContext, true);

	//Loads the scene from the .pod file into a CPVRTModelPOD object.
	if(m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load Scene_*.pod!");
		return false;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if(m_Scene.nNumCamera == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: The scene does not contain a camera\n");
		return false;
	}

	return true;
}

/*!****************************************************************************
 @Function		QuitApplication
 @Return		bool		true if no error occured
 @Description	Code in QuitApplication() will be called by PVRShell once per
				run, just before exiting the program.
				If the rendering context is lost, QuitApplication() will
				not be called.
******************************************************************************/
bool OGLESShadowTechniques::QuitApplication()
{
	// Frees the memory allocated for the scene
	m_Scene.Destroy();

	delete[] m_puiVbo;
	delete[] m_puiIndexVbo;

    return true;
}

/*!****************************************************************************
 @Function		InitView
 @Return		bool		true if no error occured
 @Description	Code in InitView() will be called by PVRShell upon
				initialization or after a change in the rendering context.
				Used to initialize variables that are dependant on the rendering
				context (e.g. textures, vertex buffers, etc.)
******************************************************************************/
bool OGLESShadowTechniques::InitView()
{
	m_fAngle = f2vt(0);

	/*
		First check whether PBuffer is available. If it is not then exit.
	*/

	/*
		Get the current display, context and surface so we can switch between the
		PBuffer surface and the main render surface.
	*/

	m_CurrentDisplay = eglGetCurrentDisplay();
	m_CurrentContext = eglGetCurrentContext();
	m_CurrentSurface = eglGetCurrentSurface(EGL_DRAW);

	// Set up a configuration and attribute list used for creating a PBuffer surface.
	EGLConfig eglConfig = SelectEGLConfig();

	EGLint list[]=
	{
		// First we specify the width of the surface...
		EGL_WIDTH, g_ui32TextureSize,
		// ...then the height of the surface...
		EGL_HEIGHT, g_ui32TextureSize,
		/* ... then we specifiy the target for the texture
			that will be created when the pbuffer is created...*/
		EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
		/*..then the format of the texture that will be created
			when the pBuffer is bound to a texture...*/
		EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGB,
		// The final thing is EGL_NONE which signifies the end.
		EGL_NONE
	};

	/*
		Using out attribute list and our egl configuration then set up the
		required PBuffer.
	*/
	m_PBufferSurface = eglCreatePbufferSurface(m_CurrentDisplay, eglConfig, list);

	// If we don't have the surface return false.
	if(m_PBufferSurface == EGL_NO_SURFACE)
	{
		PVRShellSet(prefExitMessage, "ERROR: Unable to create PBuffer surface.\n");
		return false;
	}

	// Start the demo with the advanced blob
	m_ui32Mode = ADVANCEDBLOBMODE;

	// Is the screen rotated?
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	// Initialize Print3D
	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	//	Initialize VBO data 
	LoadVbos();

	// Enables texturing
	glEnable(GL_TEXTURE_2D);

	//	Load the textures from the headers.
	if(PVRTTextureLoadFromPVR(c_szTableCoverTexFile, &m_uiTableCover) != PVR_SUCCESS) 
		return false;

	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szKettleTexFile, &m_uiKettle) != PVR_SUCCESS) 
		return false;

	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szBlobTexFile, &m_uiBlobMap) != PVR_SUCCESS) 
		return false;

	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Generate a texture for the render to texture shadow.
	glGenTextures(1, &m_uiShadow);

	// Bind and Enable Texture ID
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,m_uiShadow);
	glEnable(GL_TEXTURE_2D);

	// If Tex Params are not set glCopyTexImage2D will fail !
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Initialise the Texture
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,g_ui32TextureSize,g_ui32TextureSize,0,GL_RGB,GL_UNSIGNED_BYTE,0);

	/* 
		Bind the texture to the PBuffer surface so the surface is bound to a texture
		the first time we call RenderFromLightsView. That function calls eglReleaseTexImage
		before eglBindTexImage so it would be calling it on an unbound texture.
	*/
	if(!eglBindTexImage(m_CurrentDisplay, m_PBufferSurface, EGL_BACK_BUFFER))
	{
		PVRShellOutputDebug("Failed to bind m_PBufferSurface\n");
	}

	// Enables lighting. See BasicTnL for a detailed explanation
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	PVRTVec3 From, To, Up;

	Up = PVRTVec3(f2vt(0.0f), f2vt(1.0f), f2vt(0.0f));

	// We can get the camera position, target and field of view (fov) with GetCameraPos()
	m_Scene.GetCameraPos(From, To, 0);

	m_mView = PVRTMat4::LookAtRH(From, To, Up);

	// Calculates the projection matrix
	m_mProjection = PVRTMat4::PerspectiveFovRH(f2vt(1.0f), f2vt((float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight)), f2vt(g_fCameraNear), f2vt(g_fCameraFar), PVRTMat4::OGL, bRotate);

	// Reads the eLight direction from the scene.
	PVRTVec4 LightDirection;
	PVRTMat4 mWorld;

	// Set the eLight direction using the position of one of the meshes.
	SPODNode* pNode;

	pNode = &m_Scene.pNode[eLight];
	m_Scene.GetWorldMatrix(mWorld, *pNode);

	PVRTMat4 fRot;
	mWorld = PVRTMat4::RotationY(m_fAngle) * mWorld;

	LightDirection.x = m_fLightPos.x = mWorld.f[12];
	LightDirection.y = m_fLightPos.y = mWorld.f[13];
	LightDirection.z = m_fLightPos.z = mWorld.f[14];
	LightDirection.w = f2vt(0.0f);

	// Specify the eLight direction in world space
	myglLightv(GL_LIGHT0, GL_POSITION, LightDirection.ptr());

	glShadeModel( GL_SMOOTH );

	/*
		Build an array to map the textures within the pod header files
		to the textures we loaded a bit further up.
	*/
	m_puiTextures = new GLuint[m_Scene.nNumMaterial];

	for(unsigned int i = 0; i < m_Scene.nNumMaterial; ++i)
	{
		m_puiTextures[i] = 0;
		SPODMaterial* pMaterial = &m_Scene.pMaterial[i];

		if(!strcmp(pMaterial->pszName, "Material #1"))
			m_puiTextures[i] = m_uiTableCover;
		else if(!strcmp(pMaterial->pszName, "Material #2"))
			m_puiTextures[i] = m_uiKettle;
	}

	glEnable(GL_DEPTH_TEST);

	/*
		Get the centre of the mesh that I have called the shadow caster.
		This is used for the advanced blob.
	*/
	pNode = &m_Scene.pNode[eShadowCaster];
	SPODMesh* pMesh = &m_Scene.pMesh[pNode->nIdx];

	m_fObjectCentre = PVRTVec3(f2vt(0.0f), f2vt(5.0f), f2vt(0.0f));

	/*
		Get the plane for the eGround mesh. Obviously this relys on the
		eGround being flat.
	*/
	pNode = &m_Scene.pNode[eGround];
	pMesh = &m_Scene.pMesh[pNode->nIdx];

	PVRTVec3* pVertices = (PVRTVec3*) (pMesh->pInterleaved + (size_t) pMesh->sVertex.pData);

	// Setup floor plane for projected shadow calculations.
	findPlane(m_fPlane, pVertices[0] , pVertices[1], pVertices[2]);

	// Get the start time.
	m_ui32Time = PVRShellGetTime();
	m_ui32SwitchTime = m_ui32Time;

	m_fObjectAngle = f2vt(0.0f);
	m_mObjectRotation = PVRTMat4::Identity();

	// polygon offset for shadow to avoid ZFighting between the shadow and floor
	myglPolygonOffset(f2vt(-10.0f),f2vt(-25.0f));
	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLESShadowTechniques::LoadVbos()
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
 * Function Name  : findPlane
 * Inputs		  : 3 Points
 * Outputs		  : Plane Equations
 * Description    : Find the plane equation given 3 points.
 *******************************************************************************/
void OGLESShadowTechniques::findPlane(PVRTVec4 &plane, const PVRTVec3 &v0, const PVRTVec3 &v1, const PVRTVec3 &v2)
{
	PVRTVec3 vec0, vec1;

	// Need 2 vectors to find cross product.
	vec0 = v1 - v0;
	vec0.normalize();

	vec1 = v2 - v0;
	vec1.normalize();

	// find cross product to get A, B, and C of plane equation
	plane.x = VERTTYPEMUL(vec0.y, vec1.z)   - VERTTYPEMUL(vec0.z, vec1.y);
	plane.y = -(VERTTYPEMUL(vec0.x, vec1.z) - VERTTYPEMUL(vec0.z, vec1.x));
	plane.z = VERTTYPEMUL(vec0.x, vec1.y)   - VERTTYPEMUL(vec0.y, vec1.x);

	plane.w = -(VERTTYPEMUL(plane.x, v0.x) + VERTTYPEMUL(plane.y, v0.y) + VERTTYPEMUL(plane.z, v0.z));
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESShadowTechniques::ReleaseView()
{
	// Frees the texture lookup array
	delete[] m_puiTextures;

	// Frees the texture
	glDeleteTextures(1, &m_uiKettle);
	glDeleteTextures(1, &m_uiTableCover);

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	// Destroy the PBuffer surface
	eglDestroySurface(m_CurrentDisplay,m_PBufferSurface);

	glDeleteTextures(1, &m_uiShadow);

	m_Scene.Destroy();
	return true;
}

/*!****************************************************************************
 @Function		RenderToTexture
 @Return		bool		true if no error occured
 @Description	Renders the mesh in pNode to texture from the cameras point
				of view.
******************************************************************************/
bool OGLESShadowTechniques::RenderToTexture(SPODNode *pNode)
{
	PVRTMat4 mWorld;
	PVRTMat4 mModelView;

	// Gets the node model matrix
	m_Scene.GetWorldMatrix(mWorld, *pNode);

	// Set the Shadow Color and Alpha
	myglColor4(f2vt(0.25f), f2vt(0.25f), f2vt(0.25f), f2vt(1.0f));

	mWorld = m_mObjectRotation * mWorld;

	// Multiply the view matrix by the model (mWorld) matrix to get the model-view matrix
	glMatrixMode(GL_MODELVIEW);

	mModelView = m_mLightView * mWorld;
	myglLoadMatrix(mModelView.f);

	glEnableClientState(GL_VERTEX_ARRAY);

	glDisable(GL_TEXTURE_2D);

	/*
		For some reason on some platforms the rendering to a PBuffer fails if culling is enabled.
		Therefore I'm disabling culling before I render to the PBuffer.
	*/
	glDisable(GL_CULL_FACE);

	DrawMesh(pNode);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiShadow);

	glEnable(GL_CULL_FACE);
	return true;
}

/*!****************************************************************************
 @Function		DrawShadowTexture
 @Return		bool		true if no error occured
 @Description	Draws the texture that has been rendered to for the shadow.
******************************************************************************/
bool OGLESShadowTechniques::DrawShadowTexture()
{
	glPushMatrix();
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	float fWidth = (float) PVRShellGet(prefWidth);
	float fHeight= (float) PVRShellGet(prefHeight);
	float fScale = fHeight / fWidth;

	bool bRotate = PVRShellGet(prefIsRotated);

	if(bRotate)
	{
		fScale = fWidth / fHeight;
		myglRotate(f2vt(90.0f), f2vt(0),f2vt(0),f2vt(1));
	}

	myglTranslate(f2vt(-1), f2vt(-1), f2vt(0.5f));
	myglScale(f2vt(fScale),f2vt(1) ,f2vt(1));

	static VERTTYPE	VerticesLeft[] = {
			f2vt(0.02f) , f2vt(0.6f) , f2vt(0.0f),
			f2vt(0.02f) , f2vt(0.02f), f2vt(0.0f),
			f2vt(0.6f)  , f2vt(0.02f), f2vt(0.0f),
			f2vt(0.6f)  , f2vt(0.6f) , f2vt(0.0f),
	};

	static VERTTYPE	UVs[] = {
			f2vt(0.0f), f2vt(1.0f),
			f2vt(0.0f), f2vt(0.0f),
			f2vt(1.0f), f2vt(0.0f),
	 		f2vt(1.0f), f2vt(1.0f)
		};

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,VERTTYPEENUM,0, (VERTTYPE*)&VerticesLeft);

	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2,VERTTYPEENUM,0, (VERTTYPE*)&UVs);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_uiShadow);

	glDrawArrays(GL_TRIANGLE_FAN,0,4);

	glDisableClientState(GL_VERTEX_ARRAY);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	return true;
}

/*!****************************************************************************
 @Function		RenderFromLightsView
 @Return		bool		true if no error occured
 @Description	Renders the teapot from the eLight's view.
******************************************************************************/
bool OGLESShadowTechniques::RenderFromLightsView()
{
	glEnable(GL_DEPTH_TEST);

	glBindTexture(GL_TEXTURE_2D, m_uiShadow);

	if(!eglReleaseTexImage(m_CurrentDisplay, m_PBufferSurface, EGL_BACK_BUFFER))
	{
		PVRShellOutputDebug("Failed to release m_PBufferSurface");
		return false;
	}

	if(!eglMakeCurrent(m_CurrentDisplay, m_PBufferSurface, m_PBufferSurface, m_CurrentContext))
	{
		PVRShellOutputDebug("Unable to make the pbuffer context current");
		return false;
	}
	
	glViewport(0, 0, g_ui32TextureSize, g_ui32TextureSize);
	myglClearColor(f2vt(1), f2vt(1), f2vt(1), f2vt(1));

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	PVRTMat4 proj;

	proj = PVRTMat4::PerspectiveFovRH(f2vt(1.0f), VERTTYPEDIV(g_ui32TextureSize, g_ui32TextureSize), f2vt(g_fCameraNear), f2vt(g_fCameraFar), PVRTMat4::OGL, false);

	glMatrixMode(GL_PROJECTION);
	myglLoadMatrix(proj.f);

	RenderToTexture(&m_Scene.pNode[eShadowCaster]);

	if(!eglBindTexImage(m_CurrentDisplay, m_PBufferSurface, EGL_BACK_BUFFER))
	{
		PVRShellOutputDebug("Failed to bind m_PBufferSurface\n");
	}

	if(!eglMakeCurrent(m_CurrentDisplay, m_CurrentSurface, m_CurrentSurface, m_CurrentContext))
	{
		PVRShellOutputDebug("Unable to make the main context current");
		return false;
	}

	return true;
}
/*!****************************************************************************
 @Function		RenderScene
 @Return		bool		true if no error occured
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				eglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important OS events.
				Will also manage relevent OS events. The user has access to
				these events through an abstraction layer provided by PVRShell.
******************************************************************************/
bool OGLESShadowTechniques::RenderScene()
{
	//	If the time and circumstances are right then switch the mode.
	unsigned int ui32Time = PVRShellGetTime();
	bool bUpdateTexture = false;

	if(ui32Time - m_ui32SwitchTime > 5000)
	{
		++m_ui32Mode;

		if(m_ui32Mode == R2TEXMODE)
			bUpdateTexture = true;

		if(m_ui32Mode > R2TEXMODE)
			m_ui32Mode = ADVANCEDBLOBMODE;

		m_ui32SwitchTime = ui32Time;
	}

	//	Initialise the viewport and stuff
	SPODNode* pNode = 0;
	PVRTMat4 mWorld;
	PVRTMat4 mModelView;
	PVRTMat4 fTransform;

	// If the time is right then update the eLight's angle and the kettle's angle
	if(ui32Time - m_ui32Time > 10)
	{
		m_fAngle += f2vt(0.01f);
		m_ui32Time = ui32Time;

		m_fObjectAngle += f2vt(0.009f);
	
		m_mObjectRotation = PVRTMat4::Translation(0, f2vt(21), 0) * PVRTMat4::RotationX(-m_fObjectAngle) * PVRTMat4::Translation(0, f2vt(-21), 0);

		// Update eLight position
		PVRTVec4 LightDirection;

		pNode = &m_Scene.pNode[eLight];

		m_Scene.GetWorldMatrix(mWorld, *pNode);

		mWorld = PVRTMat4::RotationY(m_fAngle) * mWorld;

		LightDirection.x = m_fLightPos.x = mWorld.f[12];
		LightDirection.y = m_fLightPos.y = mWorld.f[13];
		LightDirection.z = m_fLightPos.z = mWorld.f[14];
		LightDirection.w = f2vt(0.0f);

		// Specify the eLight direction in world space
		myglLightv(GL_LIGHT0, GL_POSITION, LightDirection.ptr());

		PVRTVec3 Up, fPointOfInterest;

		Up = PVRTVec3(f2vt(0.0f), f2vt(1.0f), f2vt(0.0f));

		// The position of the teapot
		fPointOfInterest = PVRTVec3(f2vt(0.0f), f2vt(02.0f), f2vt(0.0f));

		// Model View Matrix
		PVRTMatrixLookAtRH(m_mLightView, m_fLightPos, fPointOfInterest, Up);
		bUpdateTexture = m_ui32Mode == R2TEXMODE;
	}

	// If we are in Render To Texture mode then render the teapot from the eLight's point of view.
	if(bUpdateTexture)
	{
		RenderFromLightsView();
		bUpdateTexture = false;
	}

	glEnable(GL_DEPTH_TEST);
	myglColor4(f2vt(1.0f), f2vt(1.0f), f2vt(1.0f), f2vt(1.0f));

	glViewport(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	myglClearColor(f2vt(0.6f), f2vt(0.8f), f2vt(1.0f), f2vt(1.0f));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Loads the projection matrix
	glMatrixMode(GL_PROJECTION);
	myglLoadMatrix(m_mProjection.f);

	// Specify the view matrix to OpenGL ES
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	// Draw the eLight
	glDisable(GL_LIGHTING);

	pNode = &m_Scene.pNode[eLight];

	m_Scene.GetWorldMatrix(mWorld, *pNode);

	mModelView = m_mView * PVRTMat4::RotationY(m_fAngle) * mWorld;
	myglLoadMatrix(mModelView.f);

	glActiveTexture(GL_TEXTURE0);

	if(pNode->nIdxMaterial == -1)
		glBindTexture(GL_TEXTURE_2D, 0);
	else
		glBindTexture(GL_TEXTURE_2D, m_puiTextures[pNode->nIdxMaterial]);

	DrawMesh(pNode);

	// Draw the eGround
	pNode = &m_Scene.pNode[eGround];

	m_Scene.GetWorldMatrix(mWorld, *pNode);

	mModelView = m_mView * mWorld;
	myglLoadMatrix(mModelView.f);

	glActiveTexture(GL_TEXTURE0);

	if(pNode->nIdxMaterial == -1)
		glBindTexture(GL_TEXTURE_2D, 0);
	else
		glBindTexture(GL_TEXTURE_2D, m_puiTextures[pNode->nIdxMaterial]);

	if(m_ui32Mode == R2TEXMODE)
	{
		/*
			If we are in render to texture mode then draw the eGround with the rendered
			texture applied.
		*/
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_uiShadow);
		glEnable(GL_TEXTURE_2D);

		DrawMesh(pNode, true);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
	else
	{
		DrawMesh(pNode);
	}


	// Draw the shadow caster
	glPushMatrix();
		glEnable(GL_LIGHTING);

		pNode = &m_Scene.pNode[eShadowCaster];

		m_Scene.GetWorldMatrix(mWorld, *pNode);

		mModelView = m_mView * m_mObjectRotation * mWorld;
	
		myglLoadMatrix(mModelView.f);

		glActiveTexture(GL_TEXTURE0);

		if (pNode->nIdxMaterial == -1)
			glBindTexture(GL_TEXTURE_2D, 0);
		else
			glBindTexture(GL_TEXTURE_2D, m_puiTextures[pNode->nIdxMaterial]);

		DrawMesh(pNode);

		glDisable(GL_LIGHTING);
	glPopMatrix();

	// Draw the shadows
	PVRTVec3 fCentre;

	switch(m_ui32Mode)
	{
		case BASEBLOBMODE:
			fCentre = PVRTVec3(f2vt(0.0f), f2vt(0.0f), f2vt(0.0f));

			// Set the modelview without the kettle rotation
			mModelView = m_mView * mWorld;
			myglLoadMatrix(mModelView.f);

			DrawBaseBlob(fCentre);
			m_Print3D.DisplayDefaultTitle("ShadowTechniques", "Base Blob", ePVRTPrint3DSDKLogo);
		break;
		case ADVANCEDBLOBMODE:
			// Set the modelview without the kettle rotation
			mModelView =  m_mView * mWorld;
			myglLoadMatrix(mModelView.f);

			DrawAdvancedBlobShadow();
			m_Print3D.DisplayDefaultTitle("ShadowTechniques", "Dynamic Blob", ePVRTPrint3DSDKLogo);
		break;
		case PROJGEOMMODE:
			myglLoadMatrix(m_mView.f);
			DrawProjectedShadow(pNode);
			m_Print3D.DisplayDefaultTitle("ShadowTechniques", "Projected geometry", ePVRTPrint3DSDKLogo);
		break;
		case R2TEXMODE:
			// This shadow is drawn when the eGround is drawn.
			m_Print3D.DisplayDefaultTitle("ShadowTechniques", "Projected render (Using PBuffer)", ePVRTPrint3DSDKLogo);
			DrawShadowTexture();
		break;
	};


	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			mesh		The mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void OGLESShadowTechniques::DrawMesh(SPODNode* pNode, bool bProjectTexture)
{
	unsigned int ui32MeshID = pNode->nIdx;

	// Get Mesh referenced by the pNode
	SPODMesh& Mesh = m_Scene.pMesh[ui32MeshID];

	// Bind the vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[ui32MeshID]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[ui32MeshID]);

	glVertexPointer(3, VERTTYPEENUM, Mesh.sVertex.nStride, Mesh.sVertex.pData);
	glNormalPointer(VERTTYPEENUM   , Mesh.sNormals.nStride, Mesh.sNormals.pData);

	glClientActiveTexture(GL_TEXTURE0);
	glTexCoordPointer(2, VERTTYPEENUM, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);

	if(bProjectTexture)
	{
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		glLoadIdentity();

		glClientActiveTexture(GL_TEXTURE1);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(3, VERTTYPEENUM, Mesh.sVertex.nStride, Mesh.sVertex.pData);

		glLoadIdentity();

		myglTranslate(f2vt(0.5f), f2vt(0.5f), f2vt(0.0f));
		myglScale(f2vt(0.003f), f2vt(0.003f), f2vt(1.0f));

		myglMultMatrix(m_mLightView.f);
	}

	// Indexed Triangle list
	glDrawElements(GL_TRIANGLES, Mesh.nNumFaces*3, GL_UNSIGNED_SHORT, 0);

	if(bProjectTexture)
	{
		glClientActiveTexture(GL_TEXTURE1);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
	}

	// unbind the vertex buffers as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*!****************************************************************************
 @Function		DrawProjectedShadow
 @Return		void
 @Description	Squish the mesh to the eGround plane and draw it.
******************************************************************************/
void OGLESShadowTechniques::DrawProjectedShadow(SPODNode* pNode)
{
	glPushMatrix();

	// Multiply the view matrix by the model (mWorld) matrix to get the model-view matrix
	PVRTMat4 m_mfloorShadow;
	PVRTVec4 vCurLightPos;

	vCurLightPos = PVRTVec4(m_fLightPos.x, m_fLightPos.y, m_fLightPos.z, f2vt(0.0f));

	shadowMatrix(m_mfloorShadow, m_fPlane, vCurLightPos);
	myglMultMatrix(m_mfloorShadow.f);

	// Enable Polygon offset to avoid ZFighting between floor and shadow
	glEnable(GL_POLYGON_OFFSET_FILL);

	// Disable Blending since alpha blend does not work with projection
	glDisable (GL_BLEND);

	// Disable Texture
	glDisable(GL_TEXTURE_2D);

	// Set the Shadow Color and Alpha
	myglColor4(f2vt(0.0f), f2vt(0.0f), f2vt(0.0f), f2vt(1.0f));

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	// Set the transformation of the kettle
	PVRTMat4 fTransform, mWorld;
	m_Scene.GetWorldMatrix(mWorld, *pNode);

	mWorld *= m_mObjectRotation;
	myglMultMatrix(mWorld.f);

	// Render the objects which will be slammed into the floor plane
	DrawMesh(pNode);

	// Disable Polygon offset to avoid ZFighting between floor and shadow
	glDisable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_TEXTURE_2D);
	glEnable (GL_BLEND);

	glPopMatrix();
}

/*!****************************************************************************
 @Function		shadowMatrix
 @Return		void
 @Description	Create a matrix to squish the mesh.
******************************************************************************/
void OGLESShadowTechniques::shadowMatrix(PVRTMat4 &shadowMat, const PVRTVec4 &vPlane, const PVRTVec4 &vlightPos)
{
	VERTTYPE fDot;

	// Find dot product between eLight position vector and eGround plane normal.
	fDot =
		VERTTYPEMUL(vPlane.x, vlightPos.x) +
		VERTTYPEMUL(vPlane.y, vlightPos.y) +
		VERTTYPEMUL(vPlane.z, vlightPos.z) +
		VERTTYPEMUL(vPlane.w, vlightPos.w);

	shadowMat.f[ 0] = fDot - VERTTYPEMUL(vlightPos.x, vPlane.x);
	shadowMat.f[ 4] = 0   - VERTTYPEMUL(vlightPos.x, vPlane.y);
	shadowMat.f[ 8] = 0   - VERTTYPEMUL(vlightPos.x, vPlane.z);
	shadowMat.f[12] = 0   - VERTTYPEMUL(vlightPos.x, vPlane.w);

	shadowMat.f[ 1] = 0   - VERTTYPEMUL(vlightPos.y, vPlane.x);
	shadowMat.f[ 5] = fDot - VERTTYPEMUL(vlightPos.y, vPlane.y);
	shadowMat.f[ 9] = 0   - VERTTYPEMUL(vlightPos.y, vPlane.z);
	shadowMat.f[13] = 0   - VERTTYPEMUL(vlightPos.y, vPlane.w);

	shadowMat.f[ 2] = 0   - VERTTYPEMUL(vlightPos.z, vPlane.x);
	shadowMat.f[ 6] = 0   - VERTTYPEMUL(vlightPos.z, vPlane.y);
	shadowMat.f[10] = fDot - VERTTYPEMUL(vlightPos.z, vPlane.z);
	shadowMat.f[14] = 0   - VERTTYPEMUL(vlightPos.z, vPlane.w);

	shadowMat.f[ 3] = 0   - VERTTYPEMUL(vlightPos.w, vPlane.x);
	shadowMat.f[ 7] = 0   - VERTTYPEMUL(vlightPos.w, vPlane.y);
	shadowMat.f[11] = 0   - VERTTYPEMUL(vlightPos.w, vPlane.z);
	shadowMat.f[15] = fDot - VERTTYPEMUL(vlightPos.w, vPlane.w);
}

/*!****************************************************************************
 @Function		DrawBaseBlob
 @Return		void
 @Description	Draw a base blob around the input coordinate
******************************************************************************/
void OGLESShadowTechniques::DrawBaseBlob(PVRTVec3 fCentre)
{
	glDisableClientState(GL_NORMAL_ARRAY);

	VERTTYPE	Vertices[] = {
			fCentre.x + f2vt(g_fCharWidth)	, fCentre.y + f2vt(g_fFloorHeight), fCentre.z + f2vt(-g_fCharWidth),
			fCentre.x + f2vt(-g_fCharWidth), fCentre.y + f2vt(g_fFloorHeight), fCentre.z + f2vt(-g_fCharWidth),
			fCentre.x + f2vt(g_fCharWidth)	, fCentre.y + f2vt(g_fFloorHeight), fCentre.z + f2vt(g_fCharWidth) ,
	 		fCentre.x + f2vt(-g_fCharWidth), fCentre.y + f2vt(g_fFloorHeight), fCentre.z + f2vt(g_fCharWidth)
		};

	static VERTTYPE	UVs[] = {
			f2vt(0.0f), f2vt(0.0f),
			f2vt(1.0f), f2vt(0.0f),
			f2vt(0.0f), f2vt(1.0f),
	 		f2vt(1.0f), f2vt(1.0f)
		};

	// Enable Polygon offset to avoid ZFighting between floor and shadow
	glEnable(GL_POLYGON_OFFSET_FILL);

	// Enable Blending for Transparent Blob
	glEnable (GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ZERO);

	// Bind Blob Texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,m_uiBlobMap);

	// Draw Blob - in this case the object is "static" so blob position is "static" as well
	// In a Game the Blob position would be calculated from the Character Position.

	// Enable Client States and Setup Data Pointers
	glVertexPointer(3,VERTTYPEENUM,0,(VERTTYPE*)&Vertices);

	glClientActiveTexture(GL_TEXTURE0);
	glTexCoordPointer(2,VERTTYPEENUM,0,(VERTTYPE*)&UVs);

	// Draw Geometry
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);

	myglColor4(f2vt(1.0f), f2vt(1.0f), f2vt(1.0f), f2vt(1.0f));

	// Disable blending
	glDisable (GL_BLEND);

	// Disable Polygon offset to avoid ZFighting between floor and shadow
	glDisable(GL_POLYGON_OFFSET_FILL);

	glEnableClientState(GL_NORMAL_ARRAY);
}

/*!****************************************************************************
 @Function		DrawAdvancedBlobShadow
 @Return		void
 @Description	Find the intersection point of the ray to the eGround plane
				and place a blob there.
******************************************************************************/
void OGLESShadowTechniques::DrawAdvancedBlobShadow()
{
	PVRTVec3 fRay, fNorm, fInter;

	fRay = m_fObjectCentre - m_fLightPos;
	
	fNorm = m_fPlane;
	
	fRay.normalize();

	VERTTYPE fAlpha = m_fPlane.w - fNorm.dot(m_fLightPos);
	VERTTYPE fK =  fNorm.dot(fRay);

	if(fK != 0.0f)
		fAlpha = VERTTYPEDIV(fAlpha, fK);

	if(fK == 0.0f)
		fInter = PVRTVec3(f2vt(0.0f), f2vt(0.0f), f2vt(0.0f));
	else
	{
		fInter.x = m_fLightPos.x + VERTTYPEMUL(fAlpha, fRay.x);
		fInter.y = 0;
		fInter.z = m_fLightPos.z + VERTTYPEMUL(fAlpha, fRay.z);
	}

	DrawBaseBlob(fInter);
}

/*!****************************************************************************
 @Function		SelectEGLConfig
 @Return		EGLConfig
 @Description	Returns a suitable config for creating a PBuffer.
******************************************************************************/
EGLConfig OGLESShadowTechniques::SelectEGLConfig()
{
	EGLConfig EglConfig = 0;
	EGLint i32ConfigID;
	int i32BufferSize;

	// Get the colour buffer size of the current surface so we can create a PBuffer surface
	// that matches.
	EGLDisplay eglDisplay = eglGetCurrentDisplay();
	eglQueryContext(eglDisplay, eglGetCurrentContext(), EGL_CONFIG_ID, &i32ConfigID);
	eglGetConfigAttrib(eglDisplay, (EGLConfig) i32ConfigID, EGL_BUFFER_SIZE,&i32BufferSize);

	EGLint i32ConfigNo;

	// Setup the configuration list for our surface.
    EGLint conflist[] =
	{
		EGL_CONFIG_CAVEAT, EGL_NONE,
		/*
			Tell it the minimum size we want for our colour buffer and the depth size so
			eglChooseConfig will choose the config that is the closest match.
		*/
		EGL_BUFFER_SIZE, i32BufferSize,
		EGL_DEPTH_SIZE, 16,
		// The PBuffer bit is the important part as it shows we want a PBuffer
		EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
		EGL_BIND_TO_TEXTURE_RGB, EGL_TRUE,
		EGL_NONE
	};

	// Find and return the config
    if(!eglChooseConfig(m_CurrentDisplay, conflist, &EglConfig, 1, &i32ConfigNo) || i32ConfigNo != 1)
	{
		PVRShellOutputDebug("Error: Failed to find a suitable config.\n");
		return 0;
    }

    return EglConfig;
}

/*!****************************************************************************
 @Function		NewDemo
 @Return		PVRShell*		The demo supplied by the user
 @Description	This function must be implemented by the user of the shell.
				The user should return its PVRShell object defining the
				behaviour of the application.
******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLESShadowTechniques();
}

/******************************************************************************
 End of file (OGLESShadowTechniques.cpp)
******************************************************************************/
