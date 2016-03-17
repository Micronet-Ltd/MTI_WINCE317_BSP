/******************************************************************************

 @File         OGLESPhantomMask.cpp

 @Title        PhantomMask

 @Copyright    Copyright (C) 2005 - 2008 by Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows Spherical Harmonics Lighting using an IMG Vertex Program
               Requires the PVRShell.

******************************************************************************/
#include <string.h>
#include "PVRShell.h"
#include "OGLESTools.h"

// Vertex Programs
#include "DIF_VGPARMVP.h"
#include "SHL_VGPARMVP.h"
#include "DIF_VGP.h"
#include "SHL_VGP.h"
#include "DIF_VGPLITE.h"
#include "SHL_VGPLITE.h"

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szMaskMainTexFile[]  = "MaskMain.pvr";
const char c_szRoomStillTexFile[] = "RoomStill.pvr";

// POD File
#ifdef PVRT_FIXED_POINT_ENABLE
const char c_szSceneFile[]	= "PhantomMask_fixed.pod";
#else
const char c_szSceneFile[]	= "PhantomMask_float.pod";
#endif

/****************************************************************************
 ** DEFINES                                                                **
 ****************************************************************************/

// Number of Vertex Programs
const unsigned int g_ui32ProgramNo = 2;

// Assuming a 4:3 aspect ratio:
const VERTTYPE g_fCameraAspect	= f2vt(1.333333333f);
const VERTTYPE g_fCameraNear	= f2vt(50.0f);
const VERTTYPE g_fCameraFar		= f2vt(5000.0f);

// Influences the animation speed - tweak this for HW/SW usage - too slow/fast
const VERTTYPE g_fAnimationFactor = f2vt(0.686f);

/****************************************************************************
** Class: OGLESPhantomMask
****************************************************************************/
class OGLESPhantomMask : public PVRShell
{
    // Animation Related
	VERTTYPE m_fFrame;

	// Texture IDs
	GLuint m_ui32TexMask;
	GLuint m_ui32TexBackground;

	// Vertex Program IDs
	GLuint m_ui32Programs[g_ui32ProgramNo];

	// Print3D, Extension and POD Class Objects
	CPVRTPrint3D 		m_Print3D;
	CPVRTModelPOD		m_Scene;
	CPVRTglesExt		m_Extensions;

	// View and Projection Matrices
	PVRTMat4	m_mView, m_mProjection;

	// Render Options
	bool	m_bEnableSH;

	// OpenGL handles VBOs
	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

public:
	OGLESPhantomMask() : m_fFrame(0),
						 m_bEnableSH(true),
						 m_puiVbo(0),
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
	void ComputeAndSetSHIrradEnvMapConstants( float* pSHCoeffsRed, float* pSHCoeffsGreen, float* pSHCoeffsBlue );
	void SetupVGPConstants(bool light0, bool light1, bool envlight);
	bool InitIMG_vertex_program();
	bool LoadTextures();
	bool LoadVertexPrograms(const void *pSHL, int nSzSHL, const void *pDIF, int nSzDIF);
	void CameraGetMatrix();
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
bool OGLESPhantomMask::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Load Geometry Data
	if(m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load PhantomMask_*.pod.\n");
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
bool OGLESPhantomMask::QuitApplication()
{
	// Release basic data structures
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
bool OGLESPhantomMask::InitView()
{
	/******************************
	** IMG VERTEX PROGRAM        **
	*******************************/
	if(!InitIMG_vertex_program())
		return false;

	/******************************
	** Create Textures           **
	*******************************/
	if(!LoadTextures())
		return false;

	/******************************
	** Create VBOs				 **
	*******************************/
	LoadVbos();

	/******************************
	** Projection Matrix         **
	*******************************/

	// Get Camera info from POD file
	CameraGetMatrix();

	/******************************
	** GENERIC RENDER STATES     **
	*******************************/

	// The Type Of Depth Test To Do
	glDepthFunc(GL_LEQUAL);

	// Enables Depth Testing
	glEnable(GL_DEPTH_TEST);

	// Enables Smooth Color Shading
	glShadeModel(GL_SMOOTH);

	// Enables texturing
	glEnable(GL_TEXTURE_2D);

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLESPhantomMask::LoadVbos()
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


/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESPhantomMask::ReleaseView()
{
	// Release all Textures
	glDeleteTextures(1, &m_ui32TexMask);
	glDeleteTextures(1, &m_ui32TexBackground);

	// Release Vertex Programs
	m_Extensions.glDeleteProgramsARB(g_ui32ProgramNo, m_ui32Programs);

	// Release the Print3D textures and windows
	m_Print3D.DeleteAllWindows();
	m_Print3D.ReleaseTextures();

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
bool OGLESPhantomMask::RenderScene()
{
	PVRTMat4	mTrans, mWorld;
	SPODNode	*pNode;
	
	if(PVRShellIsKeyPressed(PVRShellKeyNameACTION1))
		m_bEnableSH = !m_bEnableSH;

	// Animation Setup
	m_Scene.SetFrame(m_fFrame); // Set current frame

	// Default render states - mainly recovery from the changes that Print3D does at the end of the frame
	myglClearColor(f2vt(0.5f), f2vt(0.5f), f2vt(0.5f), f2vt(1.0f));

	myglColor4(f2vt(1.0f), f2vt(1.0f), f2vt(1.0f), f2vt(1.0f));

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set Z compare properties
	glEnable(GL_DEPTH_TEST);

	// Disable Blending and Lighting
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);

	// Set Constants and Matrices based on current mode.
	glMatrixMode (GL_MATRIX0_ARB);
	glLoadIdentity();

	// Create model view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Bind the VGP progtam for Vertex Lighting or for Spherical Harmonics Lighting
	m_Extensions.glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_bEnableSH ? m_ui32Programs[0] : m_ui32Programs[1]);
	
	// Render the geometry with the correct texture

	//  Enable States
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	for(unsigned int i = 0; i < m_Scene.nNumMeshNode; ++i)
	{
		pNode = &m_Scene.pNode[i];	// POD node pointer (e.g. position of object)

		SPODMesh& Mesh = m_Scene.pMesh[pNode->nIdx];

		// bind the VBO for the mesh
		glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[pNode->nIdx]);

		// bind the index buffer, won't hurt if the handle is 0
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[pNode->nIdx]);

		// Texture Setup
		// Layer 0 - default layer

		// Bind correct Texture
		if(pNode->nIdx == 0)
			glBindTexture(GL_TEXTURE_2D, m_ui32TexMask);
		else
			glBindTexture(GL_TEXTURE_2D, m_ui32TexBackground);

		// Get pointers for this object
		m_Scene.GetWorldMatrix(mWorld, *pNode);
		mTrans =  m_mView * mWorld;

		glMatrixMode(GL_MODELVIEW);
		myglLoadMatrix(mTrans.f);

		glMatrixMode(GL_MATRIX0_ARB);

		// Enable Lighting only for the Mask
		if(pNode->nIdx == 0)
		{
            if(m_bEnableSH)
			{
				myglLoadMatrix(mWorld.f);
				myglRotate(f2vt(60.0f), f2vt(0), f2vt(1), f2vt(0));
			}
			else
			{
				myglLoadMatrix(mWorld.f);
			}

			// Enable Vertex Program
			glEnable(GL_VERTEX_PROGRAM_ARB);

			// Projection matrix
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			if(PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen))
			{
#if defined(UNDER_CE) && defined(WIN32_PLATFORM_WFSP)
				{
					PVRTMat4 m( 0		, f2vt(1), 0		, 0,
								f2vt(1)	, 0		 , 0		, 0,
								0		, 0		 , f2vt(1)	, 0,
								0		, 0		 , 0		, f2vt(1));

					myglMultMatrix(m.f);
				}
#endif
				myglRotate(f2vt(90.0f), f2vt(0.0f), f2vt(0.0f), f2vt(1.0f));
			}

			myglMultMatrix(m_mProjection.f);
		}
		else
		{
			glLoadIdentity();

			// Disable Vertex Program
			glDisable(GL_VERTEX_PROGRAM_ARB);
			glDisableClientState(GL_COLOR_ARRAY);

			myglColor4(f2vt(1.0f), f2vt(1.0f), f2vt(1.0f), f2vt(1.0f));

			// Projection matrix
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			if(PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen))
				myglRotate(f2vt(90.0f), f2vt(0.0f), f2vt(0.0f), f2vt(1.0f));

			myglMultMatrix(m_mProjection.f);
		}

		//	Render object

		// Set Data Pointers
		glVertexPointer(3, VERTTYPEENUM, Mesh.sVertex.nStride, Mesh.sVertex.pData);
		glNormalPointer(VERTTYPEENUM, Mesh.sNormals.nStride, Mesh.sNormals.pData);
		glTexCoordPointer(2, VERTTYPEENUM, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);


		// Indexed Triangle list
		glDrawElements(GL_TRIANGLES, Mesh.nNumFaces*3, GL_UNSIGNED_SHORT, 0);
	}

	// Disable Vertex Program
	glDisable(GL_VERTEX_PROGRAM_ARB);

	// Clean-up
	myglColor4(f2vt(1.0f), f2vt(1.0f), f2vt(1.0f), f2vt(1.0f));

	// Disable the Various Vertex Attribs
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	// unbind the vertex buffers as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Print text on screen

	// Shadow
	m_Print3D.Print3D(0.0f, 1.0f, 1.205f,  PVRTRGBA(0,0,0,255), "PhantomMask");

	if(m_bEnableSH)
	{
		// Shadow
		m_Print3D.Print3D(0.0f, 8.0f, 0.905f,  PVRTRGBA(0,0,0,255), "VGP Spherical Harmonics Lighting");

		// Base
        m_Print3D.DisplayDefaultTitle("PhantomMask", "VGP Spherical Harmonics Lighting", ePVRTPrint3DSDKLogo);
	}
	else
	{
		// Shadow
		m_Print3D.Print3D(0.0f, 8.0f, 0.905f,  PVRTRGBA(0,0,0,255), "VGP Vertex Lighting");

		// Base
		m_Print3D.DisplayDefaultTitle("PhantomMask", "VGP Vertex Lighting", ePVRTPrint3DSDKLogo);
	}

	m_Print3D.Flush();

	// Increment the framecounter to make sure our animation works
	m_fFrame = m_fFrame + g_fAnimationFactor;

	// Loop animation
	if(m_fFrame >= f2vt(m_Scene.nNumFrame - 1))
		m_fFrame = m_fFrame - f2vt(m_Scene.nNumFrame - 1);

	return true;
}

/*!****************************************************************************
 @Function		InitIMG_vertex_program
 @Return		bool		true if no error occured
 @Description	Initialises the vertex programs
******************************************************************************/
bool OGLESPhantomMask::InitIMG_vertex_program()
{
	/******************************
	** IMG VERTEX PROGRAM        **
	*******************************/

	// Check Extension Available
	char* pExtensions = (char*) glGetString(GL_EXTENSIONS);

	if(!strstr(pExtensions, "GL_IMG_vertex_program"))
	{
		// Extension not available so quit the app and write a debug message
		PVRShellSet(prefExitMessage, "ERROR: GL_IMG_vertex_program is NOT supported by this platform.\n");
		return false;
	}

	// Extension is available so :
	// 1) Init the function calls using our tools function

	m_Extensions.LoadExtensions();

	// 2) Load our program(s) for usage

	// Retrieve pointer to renderer string
	const char* pszRendererString = (const char*) glGetString(GL_RENDERER);

	bool bSuccess = false;

	// PowerVR hardware present? 
	if(strstr(pszRendererString, "PowerVR"))
	{
		// VGP present?
		if(strstr(pszRendererString, "VGPLite"))
		{
			// VGPLite present
			bSuccess = LoadVertexPrograms(vgp_SHL_VGPLITE,sizeof(vgp_SHL_VGPLITE),vgp_DIF_VGPLITE,sizeof(vgp_DIF_VGPLITE));
		}
		else // Assume VGP is present
		{
			bSuccess = LoadVertexPrograms(vgp_SHL_VGP,sizeof(vgp_SHL_VGP),vgp_DIF_VGP,sizeof(vgp_DIF_VGP));
			
			if(!bSuccess)
			{
				// Try loading ARM VP Vertex Program instead
				bSuccess = LoadVertexPrograms(vgp_SHL_VGPARMVP,sizeof(vgp_SHL_VGPARMVP),vgp_DIF_VGPARMVP,sizeof(vgp_DIF_VGPARMVP));
			}
		}
    }

	if(!bSuccess)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load VGP binary programs.\n");
		return false;
	}

	// 3) Setup some base constants
	SetupVGPConstants(false, false, true);

	return true;
}

bool OGLESPhantomMask::LoadTextures()
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
	if(PVRTTextureLoadFromPVR(c_szRoomStillTexFile, &m_ui32TexBackground) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load texture for Background.\n");
		return false;
	}

	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szMaskMainTexFile, &m_ui32TexMask) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load texture for Mask.\n");
		return false;
	}

	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

/*!****************************************************************************
 @Function		LoadVertexPrograms
 @Return		true if no error occured
 @Description	Function to load Vertex Programs used by this App
******************************************************************************/
bool OGLESPhantomMask::LoadVertexPrograms(const void *pSHL, int nSzSHL, const void *pDIF, int nSzDIF)
{
	// Generate Vertex Program IDs
	m_Extensions.glGenProgramsARB(2, m_ui32Programs);

	// Bind and Load Program
	m_Extensions.glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_ui32Programs[0]);
	m_Extensions.glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_BINARY_IMG, nSzSHL, pSHL);

	// Check for problems 
	if(glGetError() != GL_NO_ERROR)
		return false;

	// Bind and Load Program
	m_Extensions.glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_ui32Programs[1]);
	m_Extensions.glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_BINARY_IMG, nSzDIF, pDIF);

	// Check for problems
	if(glGetError()!=GL_NO_ERROR)
		return false;

	return true;
}

/*******************************************************************************
  @Function    CameraGetMatrix
  @Description Function to setup camera position
*******************************************************************************/
void OGLESPhantomMask::CameraGetMatrix()
{
	PVRTVec3 vFrom, vTo, vUp;
	VERTTYPE fFOV = 0;

	vUp = PVRTVec3(f2vt(0.0f), f2vt(1.0f), f2vt(0.0f));

	if(m_Scene.nNumCamera)
	{
		// Get Camera data from POD Geometry File
		fFOV = m_Scene.GetCameraPos(vFrom, vTo, 0);
		fFOV = VERTTYPEMUL(fFOV, f2vt(0.75f));		// Convert from horizontal FOV to vertical FOV (0.75 assumes a 4:3 aspect ratio)
	}

	// View
	m_mView = PVRTMat4::LookAtRH(vFrom, vTo, vUp);

	// Projection
	m_mProjection = PVRTMat4::PerspectiveFovRH(fFOV, g_fCameraAspect, g_fCameraNear, g_fCameraFar, PVRTMat4::OGL);
}

/*******************************************************************************
 @Function ComputeAndSetSHIrradEnvMapConstants
 @Description Function to pre-calculate and setup the Spherical Harmonics Constants
 *******************************************************************************/
void OGLESPhantomMask::ComputeAndSetSHIrradEnvMapConstants( float* pSHCoeffsRed, float* pSHCoeffsGreen, float* pSHCoeffsBlue )
{
    float* fLight[3] = { pSHCoeffsRed, pSHCoeffsGreen, pSHCoeffsBlue };

    // Lighting environment coefficients
    VERTTYPE vCoefficients[3][4];
	int iChannel;

    // These constants are described in the article by Peter-Pike Sloan titled
    // "Efficient Evaluation of Irradiance Environment Maps" in the book
    // "ShaderX 2 - Shader Programming Tips and Tricks" by Wolfgang F. Engel.

    static const float s_fSqrtPI = 1.772453850905516027298167483341f;
    const float fC0 = 1.0f/(2.0f*s_fSqrtPI);
    const float fC1 = (float)1.7320508075688772935274463415059f/(3.0f*s_fSqrtPI);
    const float fC2 = (float)3.8729833462074168851792653997824f/(8.0f*s_fSqrtPI);
    const float fC3 = (float)2.2360679774997896964091736687313f/(16.0f*s_fSqrtPI);
    const float fC4 = 0.5f*fC2;

    for(iChannel = 0; iChannel < 3; ++iChannel)
    {
        vCoefficients[iChannel][0] = f2vt(-fC1*fLight[iChannel][3]);
        vCoefficients[iChannel][1] = f2vt(-fC1*fLight[iChannel][1]);
        vCoefficients[iChannel][2] = f2vt( fC1*fLight[iChannel][2]);
        vCoefficients[iChannel][3] = f2vt( fC0*fLight[iChannel][0] - fC3*fLight[iChannel][6]);
    }

	m_Extensions.myglProgramEnvParameter4v(GL_VERTEX_PROGRAM_ARB,0,vCoefficients[0]);
	m_Extensions.myglProgramEnvParameter4v(GL_VERTEX_PROGRAM_ARB,1,vCoefficients[1]);
	m_Extensions.myglProgramEnvParameter4v(GL_VERTEX_PROGRAM_ARB,2,vCoefficients[2]);

	for(iChannel = 0; iChannel < 3; ++iChannel)
    {
        vCoefficients[iChannel][0] = f2vt(     fC2*fLight[iChannel][4]);
        vCoefficients[iChannel][1] = f2vt(    -fC2*fLight[iChannel][5]);
        vCoefficients[iChannel][2] = f2vt(3.0f*fC3*fLight[iChannel][6]);
        vCoefficients[iChannel][3] = f2vt(    -fC2*fLight[iChannel][7]);
    }

	m_Extensions.myglProgramEnvParameter4v(GL_VERTEX_PROGRAM_ARB,3,vCoefficients[0]);
	m_Extensions.myglProgramEnvParameter4v(GL_VERTEX_PROGRAM_ARB,4,vCoefficients[1]);
	m_Extensions.myglProgramEnvParameter4v(GL_VERTEX_PROGRAM_ARB,5,vCoefficients[2]);

    vCoefficients[0][0] = f2vt(fC4*fLight[0][8]);
    vCoefficients[0][1] = f2vt(fC4*fLight[1][8]);
    vCoefficients[0][2] = f2vt(fC4*fLight[2][8]);
    vCoefficients[0][3] = f2vt(1.0f);

	m_Extensions.myglProgramEnvParameter4v(GL_VERTEX_PROGRAM_ARB,6,vCoefficients[0]);
}

/*******************************************************************************
 * Function Name  : SetupVGPConstants
 * Input		  : Enabled Light Sources
 * Description    : Function to setup the VGP constants used for Diffuse and SH Lighting
 *
 *******************************************************************************/
void OGLESPhantomMask::SetupVGPConstants(bool light0, bool light1, bool envlight)
{
	// SH Data Sets
	// Not all are used

	float SHCoeffsLight1Red[9] = {0.83409595f, -1.4446964f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, -0.93254757f, 0.00000000f, -1.6152197f};
	float SHCoeffsLight1Green[9] = {0.83409595f, -1.4446964f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, -0.93254757f, 0.00000000f, -1.6152197f};
	float SHCoeffsLight1Blue[9] = {0.83409595f, -1.4446964f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, -0.93254757f, 0.00000000f, -1.6152197f};

	float SHCoeffsLight2Red[9] = {0.83409595f, -1.2120811f, -0.24892779f, -0.74568230f, -1.3989232f, -0.46699628f, -0.84948879f, 0.28729999f, -0.70663643f};
	float SHCoeffsLight2Green[9] = {0.83409595f, -1.2120811f, -0.24892779f, -0.74568230f, -1.3989232f, -0.46699628f, -0.84948879f, 0.28729999f, -0.70663643f};
	float SHCoeffsLight2Blue[9] = {0.83409595f, -1.2120811f, -0.24892779f, -0.74568230f, -1.3989232f, -0.46699628f, -0.84948879f, 0.28729999f, -0.70663643f};

	float SHCoeffsLightEnvRed[9] = {1.2961891f, -0.42659417f, -0.10065936f, -8.4035477e-005f, -0.00021227333f, 0.10019236f, 0.011847760f, 0.00016783635f, -0.10584830f};
	float SHCoeffsLightEnvGreen[9] = {1.2506844f, -0.12775756f, 0.33325988f, -8.7283181e-005f, -0.00015105936f, -0.025249202f, -0.048718069f, 0.00026852929f, -0.28519103f};
	float SHCoeffsLightEnvBlue[9] = {1.6430428f, 0.098693930f, 0.071262904f, 0.00044371662f, 0.00027166531f, 0.056100018f, -0.23762819f, -0.00015725456f, -0.49318397f};

	float SHCoeffsLightSideRed[9] = {	0.83409595f, 0.00000000f, 0.00000000f, -1.4446964f, 0.00000000f, 0.00000000f, -0.93254757f, 0.00000000f, 1.6152197f};
	float SHCoeffsLightSideGreen[9] = {	0.83409595f, 0.00000000f, 0.00000000f, -1.4446964f, 0.00000000f, 0.00000000f, -0.93254757f, 0.00000000f, 1.6152197f};
	float SHCoeffsLightSideBlue[9] = {	0.83409595f, 0.00000000f, 0.00000000f, -1.4446964f, 0.00000000f, 0.00000000f, -0.93254757f, 0.00000000f, 1.6152197f};

	float SHCoeffsLightEnvGraceCrossRed[9] = {10.153550f, -5.0607910f, -4.3494077f, 3.7619650f, -1.4272760f, 3.3470039f, -2.0500889f, -7.1480651f, 2.7244451f};
	float SHCoeffsLightEnvGraceCrossGreen[9] = {5.6218147f, -4.4867749f, -2.3315217f, 0.71724868f, -0.65607071f, 2.8644383f, -1.2423282f, -2.7321301f, -0.70176142f};
	float SHCoeffsLightEnvGraceCrossBlue[9] = {6.9620109f, -7.7706318f, -3.4473803f, -0.12024292f, -1.5760463f, 6.0764866f, -1.9274533f, -1.7631743f, -3.9185245f};

	float	SHCoeffsLightSummedRed[9];
	float	SHCoeffsLightSummedGreen[9];
	float	SHCoeffsLightSummedBlue[9];

	VERTTYPE fValue[4];
	float LIGHT1WEIGHT,LIGHT2WEIGHT,LIGHTENVWEIGHT,LIGHTSIDEWEIGHT,LIGHTGRACECROSSWEIGHT;

	int i;

	// SH Weights
	LIGHT1WEIGHT=0.0f;
	LIGHT2WEIGHT=0.0f;
	LIGHTENVWEIGHT=0.0f;
	LIGHTSIDEWEIGHT=0.0f;
	LIGHTGRACECROSSWEIGHT=0.0f;

	// Set weights based on scene info

	if(light0 && light1 && envlight)
	{
		LIGHT1WEIGHT=0.3f;
		LIGHT2WEIGHT=0.3f;
		LIGHTENVWEIGHT=1.0f;
	}
	else if(!light0 && !light1 && envlight)
	{
		LIGHTENVWEIGHT=1.0f;
	}

	// Calculate the final SH coefs using the different lights and weights

	for (i = 0; i < 9; ++i)
	{
		SHCoeffsLightSummedRed[i]   = LIGHT1WEIGHT*SHCoeffsLight1Red[i]+LIGHT2WEIGHT*SHCoeffsLight2Red[i]+LIGHTENVWEIGHT*SHCoeffsLightEnvRed[i]+LIGHTSIDEWEIGHT*SHCoeffsLightSideRed[i]+LIGHTGRACECROSSWEIGHT*SHCoeffsLightEnvGraceCrossRed[i];
		SHCoeffsLightSummedGreen[i] = LIGHT1WEIGHT*SHCoeffsLight1Green[i]+LIGHT2WEIGHT*SHCoeffsLight2Green[i]+LIGHTENVWEIGHT*SHCoeffsLightEnvGreen[i]+LIGHTSIDEWEIGHT*SHCoeffsLightSideGreen[i]+LIGHTGRACECROSSWEIGHT*SHCoeffsLightEnvGraceCrossGreen[i];
		SHCoeffsLightSummedBlue[i]  = LIGHT1WEIGHT*SHCoeffsLight1Blue[i]+LIGHT2WEIGHT*SHCoeffsLight2Blue[i]+LIGHTENVWEIGHT*SHCoeffsLightEnvBlue[i]+LIGHTSIDEWEIGHT*SHCoeffsLightSideBlue[i]+LIGHTGRACECROSSWEIGHT*SHCoeffsLightEnvGraceCrossBlue[i];
	}

	ComputeAndSetSHIrradEnvMapConstants(SHCoeffsLightSummedRed, SHCoeffsLightSummedGreen, SHCoeffsLightSummedBlue);

	/************************************************************************/
	/*            VERTEX LIGHTING CONSTANTS                                 */
	/************************************************************************/

	// Light Position 1 : TOP
	fValue[0]=f2vt(0.0f);
    fValue[1]=f2vt(0.5f);
	fValue[2]=f2vt(0.0f);
	fValue[3]=f2vt(0.0f);

	m_Extensions.myglProgramEnvParameter4v(GL_VERTEX_PROGRAM_ARB,7,fValue);

	// Light Position 2 : BOTTOM
    fValue[0]=f2vt(0.0f);
	fValue[1]=f2vt(-0.5f);
	fValue[2]=f2vt(0.0f);
	fValue[3]=f2vt(0.0f);

	m_Extensions.myglProgramEnvParameter4v(GL_VERTEX_PROGRAM_ARB,8,fValue);

	// Light Position 3 : LEFT
    fValue[0]=f2vt(-0.5f);
	fValue[1]=f2vt(0.0f);
	fValue[2]=f2vt(0.0f);
	fValue[3]=f2vt(0.0f);

	m_Extensions.myglProgramEnvParameter4v(GL_VERTEX_PROGRAM_ARB,9,fValue);

	// Light Position 4 : RIGHT
    fValue[0]=f2vt(0.5f);
	fValue[1]=f2vt(0.0f);
	fValue[2]=f2vt(0.0f);
	fValue[3]=f2vt(0.0f);

	m_Extensions.myglProgramEnvParameter4v(GL_VERTEX_PROGRAM_ARB,10,fValue);

	// Ambient Light
	fValue[0]=f2vt(0.05f);
    fValue[1]=f2vt(0.05f);
	fValue[2]=f2vt(0.05f);
	fValue[3]=f2vt(0.05f);

	m_Extensions.myglProgramEnvParameter4v(GL_VERTEX_PROGRAM_ARB,11,fValue);
}

/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialize a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLESPhantomMask();
}

/*****************************************************************************
 End of file (OGLESPhantomMask.cpp)
*****************************************************************************/
