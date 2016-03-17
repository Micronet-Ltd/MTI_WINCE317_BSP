/******************************************************************************

 @File         OGLESBasicTnL.cpp

 @Title        Shows basic transformations and lighting

 @Copyright    Copyright (C) 2000 - 2008 by Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows basic transformations and lighting

******************************************************************************/
#include "PVRShell.h"
#include <math.h>

#include <GLES/egl.h>

/******************************************************************************
 Defines
******************************************************************************/

// Defines to abstract float/fixed data for Common/CommonLite profiles
#ifdef PVRT_FIXED_POINT_ENABLE
#define VERTTYPE			GLfixed
#define VERTTYPEENUM		GL_FIXED
#define f2vt(x)				((int)((x)*65536))
#define myglLoadMatrix		glLoadMatrixx
#define myglClearColor		glClearColorx
#define myglLight			glLightx
#define myglLightv			glLightxv
#define myglLightModel		glLightModelx
#define myglLightModelv		glLightModelxv
#define myglTexParameter	glTexParameterx
#else
#define VERTTYPE			GLfloat
#define VERTTYPEENUM		GL_FLOAT
#define f2vt(x)				(x)
#define myglLoadMatrix		glLoadMatrixf
#define myglClearColor		glClearColor
#define myglLight			glLightf
#define myglLightv			glLightfv
#define myglLightModel		glLightModelf
#define myglLightModelv		glLightModelfv
#define myglTexParameter	glTexParameterf
#endif

// Size of the texture we create
const int g_i32TexSize = 128;

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESBasicTnL : public PVRShell
{
	// Texture handle
	GLuint	m_ui32Texture;

	// Angle to rotate the triangle
	float	m_fAngle;

	// Vertex Buffer Object (VBO) handle
	GLuint	m_ui32Vbo;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();
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
bool OGLESBasicTnL::InitApplication()
{
	m_fAngle = 0;
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
bool OGLESBasicTnL::QuitApplication()
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
bool OGLESBasicTnL::InitView()
{
	// Sets the clear color
	myglClearColor(f2vt(0.6f), f2vt(0.8f), f2vt(1.0f), f2vt(1.0f));

	// Enables texturing
	glEnable(GL_TEXTURE_2D);

	/*
		Creates the texture.
		Please refer to the training course "Texturing" for a detailed explanation.
	*/
	glGenTextures(1, &m_ui32Texture);
	glBindTexture(GL_TEXTURE_2D, m_ui32Texture);
	GLuint* pTexData = new GLuint[g_i32TexSize*g_i32TexSize];

	for(int i = 0; i < g_i32TexSize; ++i)
	{
		for(int j = 0; j < g_i32TexSize; ++j)
		{
			GLuint col = (255L<<24) + ((255L-j*2)<<16) + ((255L-i)<<8) + (255L-i*2);

			if ( ((i*j)/8) % 2 ) 
				col = 0xffff00ff;

			pTexData[j*g_i32TexSize+i] = col;
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_i32TexSize, g_i32TexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, pTexData);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	delete[] pTexData;

	// Enables lighting and light 0
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	/*
		Specifies the light direction.
		If the 4th component is 0, it's a parallel light (the case here).
		If the 4th component is not 0, it's a point light.
	*/
	VERTTYPE aLightPosition[] = {f2vt(0.0f),f2vt(0.0f),f2vt(1.0f),f2vt(0.0f)};

	/*
		Assigns the light direction to the light number 0.
		This function allows you to set also the ambiant, diffuse,
		specular, emission colors of the light as well as attenuation parameters.
		We keep the other parameters to their default value in this demo.
	*/
	myglLightv(GL_LIGHT0, GL_SPOT_DIRECTION, aLightPosition);

	// Create VBO for the triangle from our data

	// Interleaved vertex data
	VERTTYPE afVertices[] = { f2vt(-0.4f),f2vt(-0.4f),f2vt(0.0f), // Position
							  f2vt(0.0f),f2vt(0.0f),			  // UV
							  f2vt(0),f2vt(0),f2vt(1),			  // Normal
							  f2vt(0.4f),f2vt(-0.4f),f2vt(0.0f),
							  f2vt(1.0f),f2vt(0.0f), 
							  f2vt(0),f2vt(0),f2vt(1), 
							  f2vt(0.0f),f2vt(0.4f),f2vt(0.0f),
							  f2vt(0.5f),f2vt(1.0f),
							  f2vt(0),f2vt(0),f2vt(1)};

	glGenBuffers(1, &m_ui32Vbo);

	unsigned int uiSize = 3 * (sizeof(VERTTYPE) * 8); // 3 vertices * stride (8 verttypes per vertex)

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

	// Set the buffer's data
	glBufferData(GL_ARRAY_BUFFER, uiSize, afVertices, GL_STATIC_DRAW);

	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESBasicTnL::ReleaseView()
{
	// Frees the texture
	glDeleteTextures(1, &m_ui32Texture);

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
bool OGLESBasicTnL::RenderScene()
{
	// Clears the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Model view matrix
	VERTTYPE aModelView[] =
	{
		f2vt((float)cos(m_fAngle)),		f2vt(0),	f2vt((float)sin(m_fAngle)),	f2vt(0),
		f2vt(0),						f2vt(1),	f2vt(0),					f2vt(0),
		f2vt(-(float)sin(m_fAngle)),	f2vt(0),	f2vt((float)cos(m_fAngle)),	f2vt(0),
		f2vt(0),						f2vt(0),	f2vt(0),					f2vt(1)
	};

	// Sets the matrix mode to modify the Model View matrix
	glMatrixMode(GL_MODELVIEW);

	// Loads our matrix into OpenGL Model View matrix
	myglLoadMatrix(aModelView);

	// Increments the angle of the view
	m_fAngle += .02f;

	/*
		Draw a triangle.
		Please refer to the training course IntroducingPVRShell for a detailed explanation.
	*/

	// bind the VBO for the triangle
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,VERTTYPEENUM,sizeof(VERTTYPE) * 8, 0);

	// Pass the texture coordinates data
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2,VERTTYPEENUM,sizeof(VERTTYPE) * 8, (unsigned char*) (sizeof(VERTTYPE) * 3));

	// Pass the normals data
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(VERTTYPEENUM,sizeof(VERTTYPE) * 8, (unsigned char*) (sizeof(VERTTYPE) * 5));

	// Draws a non-indexed triangle array
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// unbind the vertex buffer as we don't need it bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
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
	return new OGLESBasicTnL();
}

/******************************************************************************
 End of file (OGLESBasicTnL.cpp)
******************************************************************************/


