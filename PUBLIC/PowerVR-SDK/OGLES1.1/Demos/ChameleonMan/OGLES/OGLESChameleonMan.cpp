/******************************************************************************

 @File         OGLESChameleonMan.cpp

 @Title        ChameleonMan

 @Copyright    Copyright (C) 2005 - 2008 by Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows Skinning and DOT3 Lighting using an IMG Vertex Program.
               Requires the PVRShell. Requires VGP support.

******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "PVRShell.h"
#include "OGLESTools.h"

// Vertex Programs
#include "DOT3_Skinning_VGPARMVP.h"
#include "VertexLit_Skinning_VGPARMVP.h"
#include "DOT3_Skinning_VGP.h"
#include "VertexLit_Skinning_VGP.h"
#include "DOT3_Skinning_VGPLITE.h"
#include "VertexLit_Skinning_VGPLITE.h"

/******************************************************************************
 Content file names
******************************************************************************/

// PVR texture files
// Base Textures
const char c_szFinalChameleonManHeadBodyTexFile[] = "FinalChameleonManHeadBody.pvr";
const char c_szFinalChameleonManLegsTexFile[]	= "FinalChameleonManLegs.pvr";
const char c_szLampTexFile[]	= "lamp.pvr";
const char c_szChameleonBeltTexFile[]	= "ChameleonBelt.pvr";

const char c_szSkylineTexFile[] = "skyline.pvr";
const char c_szWallDiffuseBakedTexFile[]	= "Wall_diffuse_baked.pvr";

// Tangent Space BumpMap Textures
const char c_szTang_space_BodyMapTexFile[]	= "Tang_space_BodyMap.pvr";
const char c_szTang_space_LegsMapTexFile[]	= "Tang_space_LegsMap.pvr";
const char c_szTang_space_BeltMapTexFile[]	= "Tang_space_BeltMap.pvr";

// POD File
#ifdef PVRT_FIXED_POINT_ENABLE
const char c_szSceneFile[]	= "ChameleonScene_Fixed.pod";
#else
const char c_szSceneFile[]	= "ChameleonScene.pod";
#endif

/****************************************************************************
 ** Consts                                                                 **
 ****************************************************************************/

// Number of Vertex Programs
const unsigned int g_ui32ProgramNo = 2;

// Max number of bones to influence a vertex
const unsigned int g_ui32MaxBoneInf = 2;

// Camera related defines
const VERTTYPE g_fCameraAspect = f2vt(1.333333333f);			// Assuming a 4:3 aspect ratio:
const VERTTYPE g_fCameraNear = f2vt(10.0f);
const VERTTYPE g_fCameraFar = f2vt(30000.0f);

// Influences the animation speed - tweak this for HW/SW usage - too slow/fast
const VERTTYPE g_fAnimationFactor = f2vt(0.486f);

/****************************************************************************
 ** Enums                                                                 **
 ****************************************************************************/
enum EMeshes
{
	eBody,
	eLegs,
	eBelt,
	eWall,
	eBackground,
	eLights
};

/****************************************************************************
** Structures
****************************************************************************/

/****************************************************************************
** Class: OGLESChameleonMan
****************************************************************************/

class OGLESChameleonMan : public PVRShell
{
    // Animation related variables
	VERTTYPE	m_fFrame, m_fLightPos, m_fWallPos, m_fBackgroundPos;

	// Texture IDs
	GLuint m_ui32TexHeadBody;
	GLuint m_ui32TexLegs;
	GLuint m_ui32TexBeltNormalMap;
	GLuint m_ui32TexHeadNormalMap;
	GLuint m_ui32TexLegsNormalMap;
	GLuint m_ui32TexSkyLine;
	GLuint m_ui32TexWall;
	GLuint m_ui32TexLamp;
	GLuint m_ui32TexBelt;

	// Vertex Program Related Vars
	GLuint m_ui32Programs[g_ui32ProgramNo];

	// Print3D Class Object
	CPVRTPrint3D	m_Print3D;

	// POD Class Object
	CPVRTModelPOD	m_Scene;

	// Extension Class Object
	CPVRTglesExt	m_Extensions;

	// View and Projection Matrices
	PVRTMat4	m_mView, m_mProj;

	// Status Vars
	bool	m_bEnableDOT3;

	// Vertex Buffer Object (VBO) handles
	GLuint*	m_pui32Vbo;
	GLuint*	m_pui32IndexVbo;

	// FPS Related Calculation
	GLuint	m_uiNumberFrames;
	GLuint	m_uiPreviousTime;
	float 	m_fFPS;
	

public:
	OGLESChameleonMan() : m_bEnableDOT3(true),
						  m_pui32Vbo(0),
						  m_pui32IndexVbo(0)
	{
	}

	/****************************************************************************
	** PROTOTYPES                                                             **
	****************************************************************************/

	// PVRShell functions
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

	// function definitions
	bool InitIMG_vertex_program();
	bool LoadTextures();
	bool ValidateMesh();
	bool LoadVertexPrograms(const void *pSkinningProg,int nSzSkinningProg,const void *pVertLitProg,int nSzVertLitProg);
	void CameraGetMatrix();
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
bool OGLESChameleonMan::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Load POD File Data
	if(m_Scene.ReadFromFile(c_szSceneFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load POD scene!");
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
bool OGLESChameleonMan::QuitApplication()
{
	// Release Base Data Structure Data
	m_Scene.Destroy();

	delete[] m_pui32Vbo;
	delete[] m_pui32IndexVbo;

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
bool OGLESChameleonMan::InitView()
{
	// Initialise variables
	m_fFrame		 = 0;
	m_fLightPos		 = 0;
	m_fWallPos		 = 0;
	m_fBackgroundPos = 0;
	m_uiNumberFrames = 0;
	m_fFPS			 = 0;

	// IMG_vertex_program
	if(!InitIMG_vertex_program())
		return false;

	// Load Textures
	if(!LoadTextures())
		return false;

	// Validate the mesh to make sure it is suitable for VGP skinning
	if(!ValidateMesh())
		return false;

	//	Initialize VBO data
	LoadVbos();

	/******************************
	** Projection Matrix         **
	*******************************/

	// Get Camera Data
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

	// Enable texturing
	glEnable(GL_TEXTURE_2D);

	return true;
}

/*!****************************************************************************
 @Function		LoadVbos
 @Description	Loads the mesh data required for this training course into
				vertex buffer objects
******************************************************************************/
void OGLESChameleonMan::LoadVbos()
{
	if(!m_pui32Vbo)
		m_pui32Vbo = new GLuint[m_Scene.nNumMesh];

	if(!m_pui32IndexVbo)
		m_pui32IndexVbo = new GLuint[m_Scene.nNumMesh];

	/*
		Load vertex data of all meshes in the scene into VBOs

		The meshes have been exported with the "Interleave Vectors" option,
		so all data is interleaved in the buffer at pMesh->pInterleaved.
		Interleaving data improves the memory access pattern and cache efficiency,
		thus it can be read faster by the hardware.
	*/

	glGenBuffers(m_Scene.nNumMesh, m_pui32Vbo);

	for(unsigned int i = 0; i < m_Scene.nNumMesh; ++i)
	{
		// Load vertex data into buffer object
		SPODMesh& Mesh = m_Scene.pMesh[i];
		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;

		glBindBuffer(GL_ARRAY_BUFFER, m_pui32Vbo[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);

		// Load index data into buffer object if available
		m_pui32IndexVbo[i] = 0;

		if(Mesh.sFaces.pData)
		{
			glGenBuffers(1, &m_pui32IndexVbo[i]);
			uiSize = PVRTModelPODCountIndices(Mesh) * sizeof(GLshort);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_pui32IndexVbo[i]);
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
bool OGLESChameleonMan::ReleaseView()
{
	// Release all Textures
	glDeleteTextures(1, &m_ui32TexLegs);
	glDeleteTextures(1, &m_ui32TexBeltNormalMap);
	glDeleteTextures(1, &m_ui32TexHeadNormalMap);
	glDeleteTextures(1, &m_ui32TexLegsNormalMap);
	glDeleteTextures(1, &m_ui32TexSkyLine);
	glDeleteTextures(1, &m_ui32TexWall);
	glDeleteTextures(1, &m_ui32TexLamp);
	glDeleteTextures(1, &m_ui32TexBelt);

	// Release Vertex Programs
	m_Extensions.glDeleteProgramsARB(g_ui32ProgramNo, m_ui32Programs);

	// Release the Print3D textures and windows
	m_Print3D.ReleaseTextures();

	return true;
}


/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool OGLESChameleonMan::RenderScene()
{
	if(PVRShellIsKeyPressed(PVRShellKeyNameACTION1))
		m_bEnableDOT3 = !m_bEnableDOT3;

	PVRTMat4	mTrans, mWorld, mSetRot, mWorldWall;
	int			j;
	unsigned int i;
	PVRTVec4	LightPosition;

	bool bShowDiffuse;
	bool bSecondLayer;

	PVRTMat4	mBoneWorld;
	char			cModifiedTitle[50];
	GLuint		uiDeltaTime, uiTime;

	// Set default render states - mainly recovery from the changes that Print3D does at the end of the frame
	// Buffer clears
	myglClearColor(f2vt(0.5f), f2vt(0.5f), f2vt(0.5f), f2vt(1.0f));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set Z compare properties
	glEnable(GL_DEPTH_TEST);

	// Disable Blending and Lighting
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);

	// Update Light Position and related VGP Program constant
	LightPosition.x = f2vt(200.0f);
	LightPosition.y = f2vt(350.0f);
	LightPosition.z = VERTTYPEMUL(f2vt(200.0f), PVRTABS(PVRTSIN(VERTTYPEDIV(PVRT_PIx, f2vt(4.0f)) + m_fLightPos)));
	LightPosition.w = f2vt(1.0f);

	m_Extensions.myglProgramEnvParameter4v(GL_VERTEX_PROGRAM_ARB,0,LightPosition.ptr());

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if(PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen))
	{
#if defined(UNDER_CE) && defined(WIN32_PLATFORM_WFSP)
		{
			PVRTMat4 m(0, 1, 0, 0,
					   1, 0, 0, 0,
					   0, 0, 1, 0,
					   0, 0, 0, 1);

			myglMultMatrix(m.f);
		}
#endif
		myglRotate(f2vt(90), f2vt(0), f2vt(0), f2vt(1));
	}

	myglMultMatrix(m_mProj.f);	// Set Camera Based Data

	// Set Constants and Matrices based on current mode
	// Set model view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Bind the VGP program for Vertex Lighting or for DOT3 Lighting
	m_Extensions.glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_bEnableDOT3 ? m_ui32Programs[0] : m_ui32Programs[1]);

	// Enable Vertex Program
	glEnable(GL_VERTEX_PROGRAM_ARB);

	// Render the geometry with the correct texture - Vertex Lighting does not use the DOT3 Maps
    bShowDiffuse = !m_bEnableDOT3;
	bSecondLayer = m_bEnableDOT3;

	// Enable and Set Default Client States and Pointers: Vertex, Normal and UVs
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Enable those Generic Vertex Attribs
	m_Extensions.glEnableVertexAttribArrayARB(1);
	m_Extensions.glEnableVertexAttribArrayARB(2);

	m_Extensions.glEnableVertexAttribArrayARB(3);
	m_Extensions.glEnableVertexAttribArrayARB(4);

	// Set current frame
	m_Scene.SetFrame(m_fFrame);

	// Render skinned meshes
	for(i = 0; i < 3; ++i)
	{
		SPODNode &Node = m_Scene.pNode[i];
		SPODMesh &Mesh = m_Scene.pMesh[Node.nIdx];

		// Texture Setup
		// Layer 0
		glActiveTexture(GL_TEXTURE0);

		// Bind correct Texture
		switch(i)
		{
			case eBody: glBindTexture(GL_TEXTURE_2D, bShowDiffuse ? m_ui32TexHeadBody : m_ui32TexHeadNormalMap); break;
			case eLegs: glBindTexture(GL_TEXTURE_2D, bShowDiffuse ? m_ui32TexLegs : m_ui32TexLegsNormalMap); break;
			default:    glBindTexture(GL_TEXTURE_2D, bShowDiffuse ? m_ui32TexBelt : m_ui32TexBeltNormalMap); break;
		}

		// TexEnv for Layer 0
		if(!bShowDiffuse)
		{
			myglTexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			myglTexEnv(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB);
			myglTexEnv(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
			myglTexEnv(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
		}
		else
		{
			myglTexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, (VERTTYPE)(m_bEnableDOT3 ? GL_REPLACE : GL_MODULATE));
			myglTexEnv(GL_TEXTURE_ENV, GL_SOURCE0_RGB, (VERTTYPE)(m_bEnableDOT3 ? GL_PREVIOUS : GL_TEXTURE));
			myglTexEnv(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
		}

		// Layer 1
		glActiveTexture(GL_TEXTURE1);

		// Texture Options
		if(bSecondLayer)
		{
			// Bind Correct Texture
			switch(i)
			{
				case eBody: glBindTexture(GL_TEXTURE_2D, m_ui32TexHeadBody); break;
				case eLegs: glBindTexture(GL_TEXTURE_2D, m_ui32TexLegs); break;
				default:    glBindTexture(GL_TEXTURE_2D, m_ui32TexBelt); break;
			}

			glEnable(GL_TEXTURE_2D);
			myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			// TexEnv for Layer 1
			myglTexEnv(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			myglTexEnv(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
			myglTexEnv(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
		}
		else
		{
			// Layer Not Active
			glDisable(GL_TEXTURE_2D);
		}

		// When skinning only the View Matrix is required, no need to use a World Matrix for the Obj
		glMatrixMode(GL_MODELVIEW);
		myglLoadMatrix(m_mView.f);

		// bind the VBO for the mesh
		glBindBuffer(GL_ARRAY_BUFFER, m_pui32Vbo[Node.nIdx]);
		// bind the index buffer, won't hurt if the handle is 0
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_pui32IndexVbo[Node.nIdx]);

		//	Render object
		glVertexPointer(3, VERTTYPEENUM,  Mesh.sVertex.nStride,  Mesh.sVertex.pData);
		glNormalPointer(VERTTYPEENUM,	  Mesh.sNormals.nStride, Mesh.sNormals.pData);
		glTexCoordPointer(2,VERTTYPEENUM, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);

		// Define Generic Vertex Attribs to supply Blend Weight and Matrix Index
		m_Extensions.glVertexAttribPointerARB(1, Mesh.sBoneWeight.n, GL_UNSIGNED_BYTE_NORM_IMG, GL_FALSE, Mesh.sBoneWeight.nStride, Mesh.sBoneWeight.pData);
		m_Extensions.glVertexAttribPointerARB(2, Mesh.sBoneIdx.n, GL_BYTE, GL_FALSE, Mesh.sBoneIdx.nStride, Mesh.sBoneIdx.pData);

		// Define Generic Vertex Attribs to supply BiNormal and Tangent
		m_Extensions.glVertexAttribPointerARB(3, Mesh.sBinormals.n, VERTTYPEENUM, GL_FALSE, Mesh.sBinormals.nStride, Mesh.sBinormals.pData);
		m_Extensions.glVertexAttribPointerARB(4, Mesh.sTangents.n, VERTTYPEENUM, GL_FALSE, Mesh.sTangents.nStride, Mesh.sTangents.pData);

       	for(int i32Batch = 0; i32Batch < Mesh.sBoneBatches.nBatchCnt; ++i32Batch)
		{
			// Bone Data Setup
			for(j = 0; j < Mesh.sBoneBatches.pnBatchBoneCnt[i32Batch]; ++j)
			{
				// Get the Node of the bone
				int i32NodeID = Mesh.sBoneBatches.pnBatches[i32Batch * Mesh.sBoneBatches.nBatchBoneMax + j];

				// Get the World transformation matrix for this bone
				mBoneWorld = m_Scene.GetBoneWorldMatrix(Node, m_Scene.pNode[i32NodeID]);

				glMatrixMode(GL_MATRIX0_ARB + j);
				myglLoadMatrix(mBoneWorld.f);
			}

			int i32Tris;

			if(i32Batch + 1 < Mesh.sBoneBatches.nBatchCnt)
				i32Tris = Mesh.sBoneBatches.pnBatchOffset[i32Batch + 1] - Mesh.sBoneBatches.pnBatchOffset[i32Batch];
			else
				i32Tris = Mesh.nNumFaces - Mesh.sBoneBatches.pnBatchOffset[i32Batch];

			// Render Geometry
			glDrawElements(GL_TRIANGLES, i32Tris * 3, GL_UNSIGNED_SHORT, &((unsigned short*) 0)[3 * Mesh.sBoneBatches.pnBatchOffset[i32Batch]]);
		}
	}


	// Disable Vertex Program
	glDisable(GL_VERTEX_PROGRAM_ARB);

	// Clean-up

	// Disable the Various Vertex Attribs
	m_Extensions.glDisableVertexAttribArrayARB(1);
	m_Extensions.glDisableVertexAttribArrayARB(2);

	m_Extensions.glDisableVertexAttribArrayARB(3);
	m_Extensions.glDisableVertexAttribArrayARB(4);

	glDisableClientState(GL_NORMAL_ARRAY);

	glClientActiveTexture(GL_TEXTURE1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if(PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen))
		myglRotate(f2vt(90), f2vt(0), f2vt(0), f2vt(1));

	myglMultMatrix(m_mProj.f);	// Set Camera Based Data

	// Decide on the rotation used for the wall and lights
	mSetRot = PVRTMat4::RotationY(m_fWallPos);

	// Set states
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);

	myglTexEnv(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
	myglTexEnv(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
	myglTexEnv(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);

	glClientActiveTexture(GL_TEXTURE0);

	// Render non-skinned meshes
	for(i = 3; i < m_Scene.nNumMesh; ++i)
	{
		SPODNode &Node = m_Scene.pNode[i];
		SPODMesh &Mesh = m_Scene.pMesh[Node.nIdx];

		switch(i)
		{
			case eWall:
				glBindTexture(GL_TEXTURE_2D, m_ui32TexWall);

				m_Scene.GetWorldMatrix(mWorldWall, Node);
				mTrans =  m_mView * mWorldWall * mSetRot;
				break;
			case eBackground:
				glBindTexture(GL_TEXTURE_2D, m_ui32TexSkyLine);

				m_Scene.GetWorldMatrix(mWorld, Node);
				mTrans = m_mView * mWorld;

				// Background has texture animation - simple sideways translation
				glMatrixMode(GL_TEXTURE);
				glLoadIdentity();

				myglTranslate(m_fBackgroundPos, f2vt(0.0f), f2vt(0.0f));
				break;
			case eLights:
				glBindTexture(GL_TEXTURE_2D, m_ui32TexLamp);

				m_Scene.GetWorldMatrix(mWorld, Node);

				mTrans = m_mView * mWorldWall * mSetRot * mWorldWall.inverse() * mWorld;
				break;
			default:
			break;
		};

		glMatrixMode(GL_MODELVIEW);
		myglLoadMatrix(mTrans.f);

		// bind the VBO for the mesh
		glBindBuffer(GL_ARRAY_BUFFER, m_pui32Vbo[Node.nIdx]);
		// bind the index buffer, won't hurt if the handle is 0
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_pui32IndexVbo[Node.nIdx]);

		glVertexPointer(3, VERTTYPEENUM, Mesh.sVertex.nStride, Mesh.sVertex.pData);
		glTexCoordPointer(2,VERTTYPEENUM, Mesh.psUVW[0].nStride, Mesh.psUVW[0].pData);

		glDrawElements(GL_TRIANGLES, Mesh.nNumFaces * 3, GL_UNSIGNED_SHORT, 0);

		if(i == eBackground)
		{
			// Reset Texture Matrix Translation
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
		}
	}

	// Disable states
	glDisableClientState(GL_VERTEX_ARRAY);

	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	// unbind the VBOs
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Print text on screen	
	uiTime = this->PVRShellGetTime();
	uiDeltaTime = uiTime - m_uiPreviousTime;

	if(uiDeltaTime >= 1000)
	{
		m_fFPS = 1000.0f * (float) m_uiNumberFrames / (float) uiDeltaTime;
		m_uiNumberFrames = 0;
		m_uiPreviousTime = uiTime;
	}
	
	sprintf(cModifiedTitle, "%s FPS: %.1f", "Chameleon Man ", m_fFPS);
	if(m_bEnableDOT3)
        m_Print3D.DisplayDefaultTitle(cModifiedTitle, "VGP Skinning with DOT3 Per Pixel Lighting", ePVRTPrint3DSDKLogo);
	else
		m_Print3D.DisplayDefaultTitle("Chameleon Man", "VGP Skinning with Vertex Lighting", ePVRTPrint3DSDKLogo);

	m_Print3D.Flush();

	// Increment the counter sto make sure our animation works
	m_fLightPos	+= f2vt(0.073339703f);
	m_fWallPos	+= f2vt(0.005f);
	m_fBackgroundPos += f2vt(-0.0005f);
	m_fFrame	+= g_fAnimationFactor;
	m_uiNumberFrames++;

	// Wrap the Animation of the Character back to the Start
	// Frame 16 is equal to Frame 0 in the animation data
	if(m_fLightPos >= PVRT_TWO_PI)
		m_fLightPos -= PVRT_TWO_PI;

	if(m_fWallPos >= PVRT_TWO_PI)
		m_fWallPos -= PVRT_TWO_PI;

	if(m_fBackgroundPos <= 0)
		m_fBackgroundPos += f2vt(1.0f);

	if(m_fFrame >= f2vt(16.0f))
		m_fFrame -= f2vt(16.0f);

	return true;
}

/*******************************************************************************
 * Function Name  : InitIMG_vertex_program
 * Returns        : true if no error occured
 * Description    : Function to intialise and load data for GL extension IMG_vertex_program
  *******************************************************************************/
bool OGLESChameleonMan::InitIMG_vertex_program()
{
	char *pszExtensions;
	bool bError;

	/******************************
	** IMG VERTEX PROGRAM        **
	*******************************/

	// Check Extension is Available

	pszExtensions = (char*)glGetString(GL_EXTENSIONS);

	if(!strstr(pszExtensions, "GL_IMG_vertex_program"))
	{
		// Extension not available so quit the app and write a debug message
		PVRShellSet(prefExitMessage, "ERROR: GL_IMG_vertex_program is NOT supported by this platform\n\n");
		return false;
	}

	// Extension is available so :
	// 1) Init the function calls using our tools function

	m_Extensions.LoadExtensions();

	// 2) Load our program(s) for usage

	const char *pszRendererString;

	// Retrieve pointer to renderer string
	pszRendererString = (const char*) glGetString(GL_RENDERER);

	bError = false; // No VGP

	// PowerVR hardware present?
	if(strstr(pszRendererString, "PowerVR"))
	{
		// VGP present?
		if(strstr(pszRendererString, "VGPLite"))
		{
			// VGPLite present
			bError = LoadVertexPrograms(vgp_DOT3_Skinning_VGPLITE,sizeof(vgp_DOT3_Skinning_VGPLITE),vgp_VertexLit_Skinning_VGPLITE,sizeof(vgp_VertexLit_Skinning_VGPLITE));
		}
		else // Assume VGP is present
		{
			bError = LoadVertexPrograms(vgp_DOT3_Skinning_VGP,sizeof(vgp_DOT3_Skinning_VGP),vgp_VertexLit_Skinning_VGP,sizeof(vgp_VertexLit_Skinning_VGP));

			if(!bError)
			{
				// Try loading ARM VP Vertex Program instead
				bError = LoadVertexPrograms(vgp_DOT3_Skinning_VGPARMVP,sizeof(vgp_DOT3_Skinning_VGPARMVP),vgp_VertexLit_Skinning_VGPARMVP,sizeof(vgp_VertexLit_Skinning_VGPARMVP));
			}
		}
    }

	if(!bError)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load VGP binaries.\n");
		return false;
	}

	// 3) Setup some base constants
	PVRTVec4 fValue(f2vt(1.0f));

	m_Extensions.myglProgramEnvParameter4v(GL_VERTEX_PROGRAM_ARB, 0, fValue.ptr());
	return true;
}

/*******************************************************************************
 * Function Name  : ValidateMesh
 * Returns        : true if no error occured
 * Description    : Normalizes weights, alters the indices and checks for the
					correct no of bones.
  *******************************************************************************/
bool OGLESChameleonMan::ValidateMesh()
{
	unsigned int k;
	unsigned char *pData, *pWeightData, *pNewData, *pNewWeightData;

	for(unsigned int i = 0; i < m_Scene.nNumMesh; ++i)
	{
		SPODMesh &Mesh = m_Scene.pMesh[i];

		// If the mesh is interleaved then de-interleave it
		if(Mesh.pInterleaved)
			PVRTModelPODToggleInterleaved(Mesh);

		pData = (unsigned char*) Mesh.sBoneIdx.pData;
		pWeightData = (unsigned char*) Mesh.sBoneWeight.pData;

		if(pData && pWeightData)
		{
			pNewData = Mesh.sBoneIdx.n != g_ui32MaxBoneInf ? (unsigned char*) malloc(sizeof(*pNewData) * g_ui32MaxBoneInf * Mesh.nNumVertex) : pData;
			pNewWeightData = Mesh.sBoneWeight.n != g_ui32MaxBoneInf ? (unsigned char*) malloc(sizeof(*pNewWeightData) * g_ui32MaxBoneInf * Mesh.nNumVertex) : pWeightData;

			for(unsigned int j = 0; j < Mesh.nNumVertex; ++j)
			{
				// Fix the per vertex IDX to ASM mode (ASM indexes each row, HighLevel languages index per matrix)
				for(k = 0 ; k < g_ui32MaxBoneInf; ++k)
				{
					if(k < Mesh.sBoneIdx.n)
						pNewData[(j * g_ui32MaxBoneInf) + k] = pData[(j * Mesh.sBoneIdx.n) + k] * 4;
					else
						pNewData[(j * g_ui32MaxBoneInf) + k] = 0;
				}

				// Normalize the vertex weights in case they aren't
				float fTotal = 0.0f;

				for(k = 0; k < Mesh.sBoneWeight.n; ++k)
					fTotal += (float) pWeightData[(j * Mesh.sBoneWeight.n) + k];

				fTotal = 255.0f / fTotal;

				for(k = 0; k < g_ui32MaxBoneInf; ++k)
				{
					if(k < Mesh.sBoneWeight.n)
						pNewWeightData[(j * g_ui32MaxBoneInf) + k] = (unsigned char) (pWeightData[(j * Mesh.sBoneWeight.n) + k] * fTotal);
					else
						pNewWeightData[(j * g_ui32MaxBoneInf) + k] = 0;
				}
			}

			if(pData != pNewData)
			{
				FREE(Mesh.sBoneIdx.pData);
				Mesh.sBoneIdx.pData = pNewData;
				Mesh.sBoneIdx.n = g_ui32MaxBoneInf;
				Mesh.sBoneIdx.nStride = sizeof(*pNewData) * g_ui32MaxBoneInf;
			}

			if(pWeightData != pNewWeightData)
			{
				FREE(Mesh.sBoneWeight.pData);
				Mesh.sBoneWeight.pData = pNewWeightData;
				Mesh.sBoneWeight.n = g_ui32MaxBoneInf;
				Mesh.sBoneWeight.nStride = sizeof(*pNewWeightData) * g_ui32MaxBoneInf;
			}
		}

		// Convert the mesh so it is interleaved
		PVRTModelPODToggleInterleaved(Mesh);
	}

	return true;
}

/*******************************************************************************
 * Function Name  : LoadTextures
 * Returns        : true if no error occured
 * Description    : Function to load the textures used by this App
  *******************************************************************************/
bool OGLESChameleonMan::LoadTextures()
{
	SPVRTContext	sContext;

	// Init Print3D to display text on screen
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(&sContext,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR:: Cannot initialise Print3D\n");
		return false;
	}

	// Load Textures
	if(PVRTTextureLoadFromPVR(c_szFinalChameleonManHeadBodyTexFile, &m_ui32TexHeadBody) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load texture for Upper Body.\n");
		return false;
	}

	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szFinalChameleonManLegsTexFile, &m_ui32TexLegs) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load texture for Legs.\n");
		return false;
	}

	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szTang_space_BodyMapTexFile, &m_ui32TexHeadNormalMap) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load normalmap texture for Upper Body.\n");
		return false;
	}

	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szTang_space_LegsMapTexFile, &m_ui32TexLegsNormalMap) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load normalmap texture for Legs.\n");
		return false;
	}

	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szTang_space_BeltMapTexFile, &m_ui32TexBeltNormalMap) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load normalmap texture for Belt.\n");
		return false;
	}

	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szSkylineTexFile, &m_ui32TexSkyLine) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load texture for SkyLine.\n");
		return false;
	}

	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szWallDiffuseBakedTexFile, &m_ui32TexWall) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load texture for Wall.\n");
		return false;
	}

	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szLampTexFile, &m_ui32TexLamp) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load texture for Lamps.\n");
		return false;
	}

	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(PVRTTextureLoadFromPVR(c_szChameleonBeltTexFile, &m_ui32TexBelt) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to load texture for Belt.\n");
		return false;
	}

	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

/*******************************************************************************
 * Function Name  : LoadVertexPrograms
 * Returns        : true if no error occured
 * Description    : Function to load the Vertex Programs used by this App
  *******************************************************************************/
bool OGLESChameleonMan::LoadVertexPrograms(const void *pSkinningProg,int nSzSkinningProg,const void *pVertLitProg,int nSzVertLitProg)
{
	// Generate IDs for 2 Vertex Programs
	m_Extensions.glGenProgramsARB(2, m_ui32Programs);

	// Bind and Load first program
	m_Extensions.glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_ui32Programs[0]);
	m_Extensions.glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_BINARY_IMG, nSzSkinningProg, pSkinningProg);

	// Check for problems
	if(glGetError() != GL_NO_ERROR)
		return false;

	// Bind and Load second program
	m_Extensions.glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_ui32Programs[1]);
	m_Extensions.glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_BINARY_IMG, nSzVertLitProg, pVertLitProg);

	// Check for problems
	if(glGetError() != GL_NO_ERROR)
		return false;

	return true;
}

/*******************************************************************************
 * Function Name  : CameraGetMatrix
 * Returns        :
 * Description    : Function to Setup the Camera Matrices
  *******************************************************************************/
void OGLESChameleonMan::CameraGetMatrix()
{
	PVRTVec3	fFrom, fTo, fUp;
	VERTTYPE	fFOV = f2vt(3.1415f / 6);

	fUp = PVRTVec3(f2vt(0.0f), f2vt(1.0f), f2vt(0.0f));

	// Grab Camera Data from POD file
	if(m_Scene.nNumCamera)
	{
		fFOV = m_Scene.GetCameraPos(fFrom, fTo, 0);
		fFOV = VERTTYPEMUL(fFOV, f2vt(0.65f));		// Convert from horizontal FOV to vertical FOV (0.75 assumes a 4:3 aspect ratio)
		fTo.y = fTo.y - f2vt(30.0f);
	}

	// Generate View
	m_mView = PVRTMat4::LookAtRH(fFrom, fTo, fUp);

	// Generate Projection
	m_mProj = PVRTMat4::PerspectiveFovRH(fFOV, g_fCameraAspect, g_fCameraNear, g_fCameraFar, PVRTMat4::OGL);
}

/*******************************************************************************
 * Function Name  : NewDemo
 * Description    : Called by the Shell to initialize a new instance to the
 *					demo class.
 *******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLESChameleonMan();
}
