/******************************************************************************

 @File         OGLESIntroducingPOD.cpp

 @Title        Introducing the POD 3d file format

 @Copyright    Copyright (C) 2000 - 2008 by Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to load POD files and play the animation with basic
               lighting

******************************************************************************/
#include <string.h>

#include "PVRShell.h"
#include "OGLESTools.h"

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
const char c_szArmTexFile[]	= "tex_arm.pvr";
const char c_szBaseTexFile[]= "tex_base.pvr";

// Scene
/*
	We exported the scene from 3DSMax into two .pod files.
	One for the Common (floating point) profile, the other one for CommonLite (fixed point) profile.
*/
#ifdef PVRT_FIXED_POINT_ENABLE
const char c_szSceneFile[] = "IntroducingPOD_fixed.pod";
#else
const char c_szSceneFile[] = "IntroducingPOD_float.pod";
#endif

/******************************************************************************
 Defines
******************************************************************************/

// Camera constants. Used for making the projection matrix
#define CAM_NEAR	(4.0f)
#define CAM_FAR		(5000.0f)

#define DEMO_FRAME_RATE	(1.0f / 30.0f)

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESIntroducingPOD : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D	m_Print3D;

	// Texture handle
	GLuint			m_uiTex_base, m_uiTex_arm;

	// Vertex Buffer Object (VBO) handles
	GLuint*	m_puiVbo;
	GLuint*	m_puiIndexVbo;

	// 3D Model
	CPVRTModelPOD	m_Scene;

	// Projection and Model View matrices
	PVRTMat4		m_mProjection, m_mView;

	// Array to lookup the textures for each material in the scene
	GLuint*			m_puiTextures;

	// Variables to handle the animation in a time-based manner
	int				m_iTimePrev;
	VERTTYPE		m_fFrame;

public:
	OGLESIntroducingPOD() : m_puiVbo(0),
							m_puiIndexVbo(0),
							m_puiTextures(0)
	{
	}

	// PVRShell functions
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	void LoadVbos();
	void DrawMesh(unsigned int ui32MeshID);
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
bool OGLESIntroducingPOD::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	/*
		Loads the scene from the .pod file into a CPVRTModelPOD object.
		We could also export the scene as a header file and
		load it with ReadFromMemory().
	*/

	if(m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Couldn't load the .pod file\n");
		return false;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if(m_Scene.nNumCamera == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: The scene does not contain a camera\n");
		return false;
	}

	// Initialize variables used for the animation
	m_fFrame = 0;
	m_iTimePrev = PVRShellGetTime();

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
bool OGLESIntroducingPOD::QuitApplication()
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
bool OGLESIntroducingPOD::InitView()
{
	/*
		Initialize Print3D
	*/
    bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	// Sets the clear color
	myglClearColor(f2vt(0.6f), f2vt(0.8f), f2vt(1.0f), f2vt(1.0f));

	// Enables texturing
	glEnable(GL_TEXTURE_2D);

	/*
		Loads the texture.
		For a more detailed explanation, see Texturing and IntroducingPVRTools
	*/
	if(PVRTTextureLoadFromPVR(c_szBaseTexFile, &m_uiTex_base) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot load the texture\n");
		return false;
	}

	if(PVRTTextureLoadFromPVR(c_szArmTexFile, &m_uiTex_arm) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot load the texture\n");
		return false;
	}

	// Enables lighting. See BasicTnL for a detailed explanation
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	/*
		Loads the light direction from the scene.
	*/
	// We check the scene contains at least one
	if(m_Scene.nNumLight == 0)
	{
		PVRShellSet(prefExitMessage, "ERROR: The scene does not contain a light\n");
		return false;
	}

	//	Initialize VBO data
	LoadVbos();

	/*
		Initializes an array to lookup the textures
		for each materials in the scene.
	*/
	m_puiTextures = new GLuint[m_Scene.nNumMaterial];

	for(int i = 0; i < (int) m_Scene.nNumMaterial; ++i)
	{
		m_puiTextures[i] = 0;
		SPODMaterial* pMaterial = &m_Scene.pMaterial[i];

		if(!strcmp(pMaterial->pszName, "Mat_Base"))
		{
			m_puiTextures[i] = m_uiTex_base;
		}
		else if(!strcmp(pMaterial->pszName, "Mat_Arm"))
		{
			m_puiTextures[i] = m_uiTex_arm;
		}
	}

	// Enable the depth test
	glEnable(GL_DEPTH_TEST);
	
	// Enable culling
	glEnable(GL_CULL_FACE);
	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLESIntroducingPOD::LoadVbos()
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
bool OGLESIntroducingPOD::ReleaseView()
{
	// Frees the texture lookup array
	delete [] m_puiTextures;

	// Frees the texture
	glDeleteTextures(1, &m_uiTex_arm);
	glDeleteTextures(1, &m_uiTex_base);

	// Release Print3D Textures
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
bool OGLESIntroducingPOD::RenderScene()
{
	// Clears the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Enables lighting
	glEnable(GL_LIGHTING);

	/*
		Calculates the frame number to animate in a time-based manner.
		Uses the shell function PVRShellGetTime() to get the time in milliseconds.
	*/
	int iTime = PVRShellGetTime();

	if(m_iTimePrev > iTime)
		m_iTimePrev = iTime;

	int iDeltaTime = iTime - m_iTimePrev;

	m_iTimePrev	= iTime;
	m_fFrame	+= VERTTYPEMUL(f2vt(iDeltaTime), f2vt(DEMO_FRAME_RATE));

	while(m_fFrame > f2vt(m_Scene.nNumFrame-1))
		m_fFrame -= f2vt(m_Scene.nNumFrame-1);

	// Sets the scene animation to this frame
	m_Scene.SetFrame(m_fFrame);

	{
		PVRTVec3	vFrom, vTo, vUp(f2vt(0.0f), f2vt(1.0f), f2vt(0.0f));
		VERTTYPE	fFOV;

		// We can get the camera position, target and field of view (fov) with GetCameraPos()
		fFOV = m_Scene.GetCameraPos( vFrom, vTo, 0);

		/*
			We can build the model view matrix from the camera position, target and an up vector.
			For this we use PVRTMa4::LookAtRH().
		*/
		m_mView = PVRTMat4::LookAtRH(vFrom, vTo, vUp);

		// Calculates the projection matrix
		bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
		m_mProjection = PVRTMat4::PerspectiveFovRH(fFOV, f2vt((float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight)), f2vt(CAM_NEAR), f2vt(CAM_FAR), PVRTMat4::OGL, bRotate);

		// Loads the projection matrix
		glMatrixMode(GL_PROJECTION);
		myglLoadMatrix(m_mProjection.f);
	}

	// Specify the view matrix to OpenGL ES so we can specify the light in world space
	glMatrixMode(GL_MODELVIEW);
	myglLoadMatrix(m_mView.f);

	{
		// Reads the light direction from the scene.
		PVRTVec4 vLightDirection;
		PVRTVec3 vPos;

		m_Scene.GetLight(vPos, *(PVRTVec3*)&vLightDirection, 0);
		vLightDirection.x = -vLightDirection.x;
		vLightDirection.y = -vLightDirection.y;
		vLightDirection.z = -vLightDirection.z;

		/*
			Set the w component to 0, so when passing it to glLight(), it is
			considered as a directional light (as opposed to a spot light).
		*/
		vLightDirection.w = 0;

		// Specify the light direction in world space
		myglLightv(GL_LIGHT0, GL_POSITION, (VERTTYPE*)&vLightDirection);
	}

	// Enable the vertices, normals and texture coordinates arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	/*
		A scene is composed of nodes. There are 3 types of nodes:
		- MeshNodes :
			references a mesh in the pMesh[].
			These nodes are at the beginning of the pNode[] array.
			And there are nNumMeshNode number of them.
			This way the .pod format can instantiate several times the same mesh
			with different attributes.
		- lights
		- cameras
		To draw a scene, you must go through all the MeshNodes and draw the referenced meshes.
	*/
	for(int i = 0; i < (int)m_Scene.nNumMeshNode; ++i)
	{
		SPODNode& Node = m_Scene.pNode[i];
	
		// Gets the node model matrix
		PVRTMat4 mWorld;
		m_Scene.GetWorldMatrix(mWorld, Node);

		// Multiply the view matrix by the model (mWorld) matrix to get the model-view matrix
		PVRTMat4 mModelView;
		mModelView = m_mView * mWorld;

		myglLoadMatrix(mModelView.f);

		// Loads the correct texture using our texture lookup table
		if(Node.nIdxMaterial == -1)
		{
			// It has no pMaterial defined. Use blank texture (0)
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, m_puiTextures[Node.nIdxMaterial]);
		}

		/*
			Now that the model-view matrix is set and the materials ready,
			call another function to actually draw the mesh.
		*/
		DrawMesh(Node.nIdx);
	}

	// Disable the vertex attribute arrays
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	// Display the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	m_Print3D.DisplayDefaultTitle("IntroducingPOD", "", ePVRTPrint3DSDKLogo);
	m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
 @Function		DrawMesh
 @Input			mesh		The mesh to draw
 @Description	Draws a SPODMesh after the model view matrix has been set and
				the meterial prepared.
******************************************************************************/
void OGLESIntroducingPOD::DrawMesh(unsigned int ui32MeshID)
{
	SPODMesh& Mesh = m_Scene.pMesh[ui32MeshID];

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_puiVbo[ui32MeshID]);
	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiIndexVbo[ui32MeshID]);

	// Setup pointers
	glVertexPointer(3, VERTTYPEENUM, Mesh.sVertex.nStride, Mesh.sVertex.pData);
	glTexCoordPointer(2, VERTTYPEENUM, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);
	glNormalPointer(VERTTYPEENUM, Mesh.sNormals.nStride, Mesh.sNormals.pData);

	/*
		The geometry can be exported in 4 ways:
		- Indexed Triangle list
		- Non-Indexed Triangle list
		- Indexed Triangle strips
		- Non-Indexed Triangle strips
	*/
	if(Mesh.nNumStrips == 0)
	{
		if(m_puiIndexVbo[ui32MeshID])
		{
			// Indexed Triangle list
			glDrawElements(GL_TRIANGLES, Mesh.nNumFaces * 3, GL_UNSIGNED_SHORT, 0);
		}
		else
		{
			// Non-Indexed Triangle list
			glDrawArrays(GL_TRIANGLES, 0, Mesh.nNumFaces * 3);
		}
	}
	else
	{
		for(int i = 0; i < (int) Mesh.nNumStrips; ++i)
		{
			int offset = 0;
			if(m_puiIndexVbo[ui32MeshID])
			{
				// Indexed Triangle strips
				glDrawElements(GL_TRIANGLE_STRIP, Mesh.pnStripLength[i]+2, GL_UNSIGNED_SHORT, &((GLshort*)0)[offset]);
			}
			else
			{
				// Non-Indexed Triangle strips
				glDrawArrays(GL_TRIANGLE_STRIP, offset, Mesh.pnStripLength[i]+2);
			}
			offset += Mesh.pnStripLength[i]+2;
		}
	}

	// unbind the vertex buffers as we don't need them bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
	return new OGLESIntroducingPOD();
}

/******************************************************************************
 End of file (OGLESIntroducingPOD.cpp)
******************************************************************************/
