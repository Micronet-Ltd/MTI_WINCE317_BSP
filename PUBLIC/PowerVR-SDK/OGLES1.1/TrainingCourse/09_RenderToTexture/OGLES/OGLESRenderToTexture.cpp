/******************************************************************************

 @File         OGLESRenderToTexture.cpp

 @Title        RenderToTexture

 @Copyright    Copyright (C) 2006 - 2008 by Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows how to render to a pBuffer surface and bind that to a
               texture.

******************************************************************************/
#include <math.h>
#include "PVRShell.h"
#include "OGLESTools.h"
#include <string.h>

/****************************************************************************
 ** STRUCTURES                                                             **
 ****************************************************************************/
enum EVBO
{
	eFeedback,
	eStalk
};

/****************************************************************************
** Class: OGLESRenderToTexture
****************************************************************************/
class OGLESRenderToTexture : public PVRShell
{
	//	We require 2 PBuffer surfaces.
	EGLSurface m_PBufferSurface[2];
	unsigned int m_ui32CurrentBuffer;
	unsigned int m_ui32PreviousBuffer;

	// Render contexts, etc
	EGLDisplay m_CurrentDisplay;
	EGLContext m_CurrentContext;
	EGLSurface m_CurrentSurface;

	// PBuffer Texture IDs
	GLuint m_ui32Texture;

	// Print3D
	CPVRTPrint3D 		m_Print3D;

	int m_i32TexSize;

	float m_fAngle;
	float m_fAngle2;

	unsigned int m_ui32Framenum;
	unsigned int m_ui32Time;

	// Vertex Buffer Object (VBO) handles
	GLuint	m_ui32Vbo[2];

public:
	OGLESRenderToTexture() : 	m_ui32CurrentBuffer(1),
								m_ui32PreviousBuffer(0)
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
	bool DrawScreen();
	bool RenderFractal();
	EGLConfig SelectEGLConfig();
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
bool OGLESRenderToTexture::InitApplication()
{
	// Request PBuffer support
	PVRShellSet(prefPBufferContext, true);

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
bool OGLESRenderToTexture::QuitApplication()
{
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
bool OGLESRenderToTexture::InitView()
{
	//	Find the largest power 2 texture that can be fit in the screen size we have got.
	m_i32TexSize = 1;

	// Set iSize to the value of the dimension that is the smallest
	int iSize = PVRShellGet(prefWidth) < PVRShellGet(prefHeight) ? PVRShellGet(prefWidth): PVRShellGet(prefHeight);

	// While the buffersize is smaller than the smallest dimension multiply it by 2.
	while(m_i32TexSize * 2 < iSize)
		m_i32TexSize *= 2;

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
		EGL_WIDTH, m_i32TexSize,
		// ...then the height of the surface...
		EGL_HEIGHT, m_i32TexSize,
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
		Using out attribute list and our egl configuration then set up the two
		required PBuffers.
	*/
	m_PBufferSurface[0] = eglCreatePbufferSurface(m_CurrentDisplay, eglConfig, list);
	m_PBufferSurface[1] = eglCreatePbufferSurface(m_CurrentDisplay, eglConfig, list);

	// If we don't have both of the surfaces return false.
	if(m_PBufferSurface[0] == EGL_NO_SURFACE || m_PBufferSurface[1] == EGL_NO_SURFACE)
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to create pbuffer.\n");
		return false;
	}

	// Initialize Print3D
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);

	if(m_Print3D.SetTextures(0,PVRShellGet(prefWidth),PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D.\n");
		return false;
	}

	/*
		Initialise the texture
	*/

	// Enable 2D texturing.
	glEnable(GL_TEXTURE_2D);

	// Create a texture ID for this texture
	glGenTextures(1,&m_ui32Texture);

	// Bind the Texture
	glBindTexture(GL_TEXTURE_2D,m_ui32Texture);

	// Setup the texture parameters as if they are not set then glTexImage2D will fail!
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Initialise the Texture
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,m_i32TexSize, m_i32TexSize ,0,GL_RGBA,GL_UNSIGNED_BYTE,0);

	/*
		Setup the first pBuffer surface for the first frame as its contents are going to be used when
		rendering to the second.
	*/

	// Switch the render target to the pBuffer
	if(!eglMakeCurrent(m_CurrentDisplay, m_PBufferSurface[0], m_PBufferSurface[0], m_CurrentContext))
	{
		PVRShellSet(prefExitMessage, "ERROR: Unable to make the pbuffer context current.\n");
		return false;
	}

	// Setup the Viewport to the dimensions of the pBuffer
	glViewport(0, 0, m_i32TexSize, m_i32TexSize);

	// Clear it
	myglClearColor(f2vt(0.0f), f2vt(0.0f), f2vt(0.0f), f2vt(1.0f));
	glClear(GL_COLOR_BUFFER_BIT);

	// Bind the texture to this surface
	if(!eglBindTexImage(m_CurrentDisplay, m_PBufferSurface[0], EGL_BACK_BUFFER))
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to bind m_PBufferSurface.\n");
		return false;
	}

	// We now switch back to the backbuffer for rendering.
	if(!eglMakeCurrent(m_CurrentDisplay, m_CurrentSurface, m_CurrentSurface, m_CurrentContext))
	{
		PVRShellOutputDebug("ERROR: Unable to make the main context current.\n");
		return false;
	}

	/*
		Change the starting point for a random number by using the current
		time as a seed. Once that is done set the frame number to something
		random.
	*/

	srand(PVRShellGetTime());
	m_ui32Framenum = rand() % 5000;

	// Get the initial time
	m_ui32Time = PVRShellGetTime();

	// Create VBOs for the app

	// Set up the quad that we will texture to
	VERTTYPE m_Feedback[] = { f2vt(-0.65f), f2vt(1.3f), f2vt(0.65f), // Position
							f2vt(0.0f), f2vt(1.0f),					 // UV
							f2vt(-0.65f), f2vt(0.0f), f2vt(0.65f),
							f2vt(0.0f), f2vt(0.0f),
							f2vt(0.65f), f2vt(0.0f), f2vt(0.65f),
							f2vt(1.0f), f2vt(0.0f),
							f2vt(0.65f), f2vt(1.3f), f2vt(0.0f),
							f2vt(1.0f), f2vt(1.0f) };

	// Set up the vertices for the stalk which is the basis for the pattern
	VERTTYPE m_Vertices[] = { f2vt(-0.08f), f2vt(-0.4f), f2vt(0.5f), // Position
							f2vt(-0.1f), f2vt(-1.0f), f2vt(0.5f),
							f2vt(0.1f), f2vt(-1.0f), f2vt(0.5f),
							f2vt(0.1f),	f2vt(-1.0f), f2vt(0.5f),
							f2vt(0.08f), f2vt(-0.4f), f2vt(0.5f),
							f2vt(-0.08f), f2vt(-0.4f), f2vt(0.5f),
							f2vt(0.0f), f2vt(-0.3f), f2vt(0.5f),
							f2vt(0.08f), f2vt(-0.4f), f2vt(0.5f),
							f2vt(-0.08f), f2vt(-0.4f), f2vt(0.5f) };

	glGenBuffers(2, &m_ui32Vbo[0]);

	unsigned int uiSize = 4 * (sizeof(VERTTYPE) * 5); // 4 vertices * stride (5 verttypes per vertex)

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo[eFeedback]);

	// Set the buffer's data
	glBufferData(GL_ARRAY_BUFFER, uiSize, m_Feedback, GL_STATIC_DRAW);

	uiSize = 9 * (sizeof(VERTTYPE) * 3); // 9 vertices * stride (3 verttypes per vertex)

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo[eStalk]);

	// Set the buffer's data
	glBufferData(GL_ARRAY_BUFFER, uiSize, m_Vertices, GL_STATIC_DRAW);

	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : Nothing
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool OGLESRenderToTexture::ReleaseView()
{
	// Release the Print3D textures and windows
	m_Print3D.DeleteAllWindows();
	m_Print3D.ReleaseTextures();

	// Delete the textures we created
	glDeleteTextures(1,&m_ui32Texture);

	// Destroy the surfaces we created
	eglDestroySurface(m_CurrentDisplay,m_PBufferSurface[0]);
	eglDestroySurface(m_CurrentDisplay,m_PBufferSurface[1]);

	return true;
}

/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool OGLESRenderToTexture::RenderScene()
{
	// Vary the branch angles on the fractal sinusoidally
	m_fAngle = (float)(sin(0.25*PVRT_PIf*(float)(m_ui32Framenum)/256.0f ))* 70.0f;

	// largeish prime number in the angular frequency here, so the motion's not obviously periodic
	m_fAngle2 = (float)(sin((79.0f/256.0f)*2.0*PVRT_PIf*(float)(m_ui32Framenum)/256.0f  ))*100.0f + 30.0f;

	// Increase the frame count
	if(PVRShellGetTime() - m_ui32Time > 10)
	{
		m_ui32Time = PVRShellGetTime();
		m_ui32Framenum += 2;

		if(m_ui32Framenum > 20000)
			m_ui32Framenum = 0;
	}

	// Disable the depth test as we don't need it
	glDisable(GL_DEPTH_TEST);

	// Disable culling
	glDisable(GL_CULL_FACE);

	// Draw the RenderToTexture
	if(!DrawScreen())
		return false;

	m_Print3D.DisplayDefaultTitle("Render to Texture", "Using PBuffers", ePVRTPrint3DSDKLogo);

	// Flush all Print3D commands
	m_Print3D.Flush();

	return true;
}


/*******************************************************************************
 * Function Name  : DrawScreen
 * Description    : Draws the RenderToTexture
 *******************************************************************************/
bool OGLESRenderToTexture::DrawScreen()
{
	/*
		We're going to do the following steps to create the effect

		Frame 0

			1.	We make surface1 the current rendering context.
			2.	We draw two quads with m_ui32Texture applied.
			3.	We release surface2 from any textures it is bound to.
			4.	We draw a non-textured polygon.
			5.	We bind m_ui32Texture to surface1.
			6.	We make the back buffer current.
			7.	We draw 6 quads with m_ui32Texture applied.

		Frame 1

			8.	We make surface2 the current rendering context.
			9.	We draw two quads with m_ui32Texture (still bound to surface1) applied.
			10.	We release surface1 from any textures it is bound to.
			11.	We draw a non-textured polygon.
			12.	We bind m_ui32Texture to surface2.
			13.	We make the back buffer current.
			14.	We draw 6 quads with m_ui32Texture (bound to surface2) applied.

		Frame 2

			15.	We make surface1 the current rendering context.
			16.	We draw two quads with m_ui32Texture (still bound to surface2) applied.
			17.	We release surface2 from any textures it is bound to.
			18.	We draw a non-textured polygon.
			19.	We bind m_ui32Texture to surface1.
			20.	We make the back buffer current.
			21.	We draw 6 quads with m_ui32Texture (bound to surface1) applied.

			22.	We repeat steps 8 through to 22 for consecutive frames.
	*/

	/*
		Draw the fractal onto m_ui32Texture
	*/

	if(!RenderFractal())
		return false;

	//	Render 6 rotated copies of the fractal to the screen:

	// Set the Viewport to the whole screen.
	glViewport(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));

	// Enable blending.
	glEnable (GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ONE);

	// Enable the vertex and the texture coordinate state.
	glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Enable 2D texturing.
	glEnable(GL_TEXTURE_2D);

	
	// Set the colour of the overal effect.
	myglColor4(f2vt(1.0f), f2vt(1.0f), f2vt(0.0f), f2vt(1.0f));

	// Bind the texture that is currently bound to a PBuffer surface.
	glBindTexture(GL_TEXTURE_2D,m_ui32Texture);

	// Clear the background to a light blue.
	myglClearColor(f2vt(0.6f), f2vt(0.8f), f2vt(1.0f), f2vt(1.0f));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// Scale the scene to fit the screen comfortably.
	if(PVRShellGet(prefWidth) > PVRShellGet(prefHeight))
		myglScale(f2vt(0.8f * (float)PVRShellGet(prefHeight)/(float)PVRShellGet(prefWidth)),f2vt(0.8f),f2vt(0.8f));
	else
		myglScale(f2vt(0.8f),f2vt(0.8f * (float)PVRShellGet(prefWidth)/(float)PVRShellGet(prefHeight)),f2vt(0.8f));

	// bind the VBO for the feedback quad
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo[eFeedback]);

	// Set the vertex and texture coordinate buffers we're going to use.
	glVertexPointer(3, VERTTYPEENUM, sizeof(VERTTYPE) * 5 , 0);
	glTexCoordPointer(2, VERTTYPEENUM, sizeof(VERTTYPE) * 5, (unsigned char*) (sizeof(VERTTYPE) * 3));

	/*
		The pBuffer texture only contains one branch of the effect so what we do is render
		six quads rotated round a point so we end up displaying 6 branches.
	*/
	for(int i = 0; i < 6; ++i)
	{
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		myglRotate(f2vt(60.0), f2vt(0.0), f2vt(0.0), f2vt(1.0) );
	}

	glPopMatrix();

	// Disable the vertex and texture coordinate client states and texturing in 2D.
	glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);

	// unbind the vertex buffer as we don't need it bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Swap the buffers over.
	m_ui32PreviousBuffer = m_ui32CurrentBuffer;
	++m_ui32CurrentBuffer;

	if(m_ui32CurrentBuffer > 1)
		m_ui32CurrentBuffer = 0;

	return true;

}

/*******************************************************************************
 * Function Name  : RenderFractal
 * Description    : Draws the RenderToTexture
 *******************************************************************************/
bool OGLESRenderToTexture::RenderFractal()
{
	// Switch the render target to the pBuffer
	if(!eglMakeCurrent(m_CurrentDisplay, m_PBufferSurface[m_ui32CurrentBuffer], m_PBufferSurface[m_ui32CurrentBuffer], m_CurrentContext))
	{
		PVRShellSet(prefExitMessage, "ERROR: Unable to make the pbuffer context current.\n");
		return false;
	}

	// Render to the PBuffer.

	// Setup the Viewport to the dimensions of the pBuffer
	glViewport(0, 0, m_i32TexSize, m_i32TexSize);

	// Clear the screen by this colour
	myglClearColor(f2vt(0.0f), f2vt(0.0f), f2vt(0.0f), f2vt(1.0f));

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render two rotated copies of the previous PBuffer onto current frame:

	// Bind the texture created on the previous frame:
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,m_ui32Texture);

	// Enable additive blend:
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	// Enable the vertex and texture coordinate client states
	glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// bind the VBO for the feedback quad
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo[eFeedback]);

	// Set up the vertex and texture coordinate buffers we are going to use.
	glVertexPointer(3, VERTTYPEENUM, sizeof(VERTTYPE) * 5 , 0);
	glTexCoordPointer(2, VERTTYPEENUM, sizeof(VERTTYPE) * 5, (unsigned char*) (sizeof(VERTTYPE) * 3));

	/*
	Switch to the modelview matrix and push it onto the stack so we don't make any
	permanent changes.
	*/
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// Translate and rotate the first quad.
	myglTranslate( f2vt(0.0f),f2vt(-0.4f),f2vt(0.0f) );
	myglRotate( f2vt(m_fAngle + m_fAngle2), f2vt(0.0), f2vt(0.0), f2vt(1.0) );

	// Draw the first quad.
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glPopMatrix();

	glPushMatrix();

	// Translate and rotate the second quad.
	myglTranslate( f2vt(0.0f),f2vt(-0.4f),f2vt(0.0f) );
	myglRotate( f2vt(m_fAngle - m_fAngle2), f2vt(0.0), f2vt(0.0), f2vt(1.0) );

	// Draw the second quad
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glPopMatrix();

	/*
		We no longer need the texture bound to the previous surface so we release the previous surface from
		all the textures it is bound to.
	*/

	if(!eglReleaseTexImage(m_CurrentDisplay, m_PBufferSurface[m_ui32PreviousBuffer], EGL_BACK_BUFFER))
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to release m_PBufferSurface.\n");
		return false;
	}

	/*
		Now draw the trunk.

		Firstly disable the texture coordinate state as the
		trunk doesn't have texture coordinates.
	*/

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);

	// bind the VBO for the stalk
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo[eStalk]);

	/*
		Set up the vertex buffer we'll be using.
	*/
	glVertexPointer(3, VERTTYPEENUM, sizeof(VERTTYPE) * 3, 0);

	// Draw the trunk
	glDrawArrays(GL_TRIANGLES, 0, 9);

	// Disable the vertex array as we don't need it anymore.
	glDisableClientState(GL_VERTEX_ARRAY);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,m_ui32Texture);

	// We've now done rendering to the PBuffer Surface
	/*
		Firstly we bind the current set texture to the current PBuffer surface.
	*/

	if(!eglBindTexImage(m_CurrentDisplay, m_PBufferSurface[m_ui32CurrentBuffer], EGL_BACK_BUFFER))
	{
		PVRShellSet(prefExitMessage, "ERROR: Failed to bind m_PBufferSurface.\n");
		return false;
	}

	// We now switch back to the backbuffer for rendering.
	if(!eglMakeCurrent(m_CurrentDisplay, m_CurrentSurface, m_CurrentSurface, m_CurrentContext))
	{
		PVRShellOutputDebug("ERROR: Unable to make the main context current.\n");
		return false;
	}

	return true;
}


/*******************************************************************************
 * Function Name  : SelectEGLConfig
 * Inputs		  : Mode Selection
 * Description    : Finds an EGL config with required options based on Mode Requested - for PBuffer
 *******************************************************************************/
EGLConfig OGLESRenderToTexture::SelectEGLConfig()
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
	return new OGLESRenderToTexture();
}

/*****************************************************************************
 End of file (OGLESRenderToTexture.cpp)
*****************************************************************************/
