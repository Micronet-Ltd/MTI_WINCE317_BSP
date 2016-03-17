
#include "PVRScope.h"
#include "PVRScopeGraph.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

/****************************************************************************
** Defines
****************************************************************************/
#define GL_FRAGMENT_SHADER_ARB				0x8B30
#define GL_VERTEX_SHADER_ARB				0x8B31
#define GL_OBJECT_COMPILE_STATUS_ARB		0x8B81
#define GL_OBJECT_LINK_STATUS_ARB			0x8B82
#define GL_OBJECT_INFO_LOG_LENGTH_ARB		0x8B84

typedef char GLcharARB; /* native character */
typedef unsigned int GLhandleARB; /* shader object handle */
typedef void (APIENTRY* PFNGLDELETEOBJECTARBPROC) (GLhandleARB obj);
typedef GLhandleARB (APIENTRY* PFNGLCREATESHADEROBJECTARBPROC) (GLenum shaderType);
typedef void (APIENTRY* PFNGLSHADERSOURCEARBPROC) (GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length);
typedef void (APIENTRY* PFNGLCOMPILESHADERARBPROC) (GLhandleARB shaderObj);
typedef GLhandleARB (APIENTRY* PFNGLCREATEPROGRAMOBJECTARBPROC) (void);
typedef void (APIENTRY* PFNGLATTACHOBJECTARBPROC) (GLhandleARB containerObj, GLhandleARB obj);
typedef void (APIENTRY* PFNGLLINKPROGRAMARBPROC) (GLhandleARB programObj);
typedef void (APIENTRY* PFNGLUSEPROGRAMOBJECTARBPROC) (GLhandleARB programObj);
typedef void (APIENTRY* PFNGLGETOBJECTPARAMETERIVARBPROC) (GLhandleARB obj, GLenum pname, GLint *params);
typedef void (APIENTRY* PFNGLGETINFOLOGARBPROC) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);

typedef void (APIENTRY * PFNGLDELETEPROGRAMSARBPROC) (GLsizei n, const GLuint *programs);
typedef void (APIENTRY * PFNGLBINDATTRIBLOCATIONARBPROC) (GLuint program, GLuint index, const char *name);

typedef void (APIENTRY * PFNGLVERTEXATTRIBPOINTERARBPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLENABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
typedef void (APIENTRY * PFNGLDISABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);

#define VERTEX_ARRAY	(0)

/****************************************************************************
** Structures
****************************************************************************/
struct SVertex
{
	GLfloat x, y;
};

struct SPVRScopeAPIData
{
	SVertex	*pVtx;
	SVertex	*pVtxLines;

	GLuint	uiVertexShader, uiFragShader, uiProgramObject;

	PFNGLDELETEOBJECTARBPROC				glDeleteObjectARB;
	PFNGLCREATESHADEROBJECTARBPROC			glCreateShaderObjectARB;
	PFNGLSHADERSOURCEARBPROC				glShaderSourceARB;
	PFNGLCOMPILESHADERARBPROC				glCompileShaderARB;
	PFNGLCREATEPROGRAMOBJECTARBPROC			glCreateProgramObjectARB;
	PFNGLATTACHOBJECTARBPROC				glAttachObjectARB;
	PFNGLLINKPROGRAMARBPROC					glLinkProgramARB;
	PFNGLUSEPROGRAMOBJECTARBPROC			glUseProgramObjectARB;
	PFNGLGETOBJECTPARAMETERIVARBPROC		glGetObjectParameterivARB;
	PFNGLGETINFOLOGARBPROC					glGetInfoLogARB;
	
	PFNGLDELETEPROGRAMSARBPROC				glDeleteProgramsARB;
	PFNGLBINDATTRIBLOCATIONARBPROC			glBindAttribLocationARB;

	PFNGLVERTEXATTRIBPOINTERARBPROC			glVertexAttribPointerARB;
	PFNGLENABLEVERTEXATTRIBARRAYARBPROC		glEnableVertexAttribArrayARB;
	PFNGLDISABLEVERTEXATTRIBARRAYARBPROC	glDisableVertexAttribArrayARB;
};

/****************************************************************************
** Constants
****************************************************************************/
static const GLcharARB *c_pszVertexShader = 
	"attribute vec2\tmyVertex;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\tgl_Position = vec4(myVertex, 1, 1);\r\n"
	"}\r\n";

static const GLcharARB *c_pszFragmentShader = 
	"void main()\r\n"
	"{\r\n"
	"\tgl_FragColor = vec4(1,0,0,0);\r\n"
	"}\r\n";

static const GLushort c_pwLineIdx[10] = {
	0, 1,  2, 3,  4, 5,  0, 4,  1, 5
};

/****************************************************************************
** Code
****************************************************************************/
SPVRScopeAPIData *CPVRScopeGraph::APIInit()
{
	SPVRScopeAPIData *pData;

	pData = new SPVRScopeAPIData;

	{
		pData->glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)wglGetProcAddress("glDeleteObjectARB");
		pData->glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress("glCreateShaderObjectARB");
		pData->glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)wglGetProcAddress("glShaderSourceARB");
		pData->glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress("glCompileShaderARB");
		pData->glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress("glCreateProgramObjectARB");
		pData->glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)wglGetProcAddress("glAttachObjectARB");
		pData->glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)wglGetProcAddress("glLinkProgramARB");
		pData->glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)wglGetProcAddress("glUseProgramObjectARB");
		pData->glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
		pData->glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");

		pData->glVertexAttribPointerARB			= (PFNGLVERTEXATTRIBPOINTERARBPROC)wglGetProcAddress("glVertexAttribPointerARB");
		pData->glEnableVertexAttribArrayARB		= (PFNGLENABLEVERTEXATTRIBARRAYARBPROC)wglGetProcAddress("glEnableVertexAttribArrayARB");
		pData->glDisableVertexAttribArrayARB	= (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)wglGetProcAddress("glDisableVertexAttribArrayARB");
		pData->glDeleteProgramsARB				= (PFNGLDELETEPROGRAMSARBPROC)wglGetProcAddress("glDeleteProgramsARB");
		pData->glBindAttribLocationARB			= (PFNGLBINDATTRIBLOCATIONARBPROC)wglGetProcAddress("glBindAttribLocationARB");
	}

	{
		pData->pVtx = 0;
		pData->pVtxLines = 0;

		// Create the fragment shader object
		pData->uiFragShader = pData->glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

		// Load the source code into it
		pData->glShaderSourceARB(pData->uiFragShader, 1, &c_pszFragmentShader, NULL);

		// Compile the source code
		pData->glCompileShaderARB(pData->uiFragShader);

		// Check if compilation succeeded
		GLint bShaderCompiled;
		pData->glGetObjectParameterivARB(pData->uiFragShader, GL_OBJECT_COMPILE_STATUS_ARB, &bShaderCompiled);
		if (!bShaderCompiled)
		{
			// An error happened, first retrieve the length of the log message
			int i32InfoLogLength, i32CharsWritten;
			pData->glGetObjectParameterivARB(pData->uiFragShader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &i32InfoLogLength);

			// Allocate enough space for the message and retrieve it
			char* pszInfoLog = new char[i32InfoLogLength];
			pData->glGetInfoLogARB(pData->uiFragShader, i32InfoLogLength, &i32CharsWritten, pszInfoLog);

			/*
				Displays the message in a dialog box when the application quits
				using the shell PVRShellSet function with first parameter prefExitMessage.
			*/
			char* pszMsg = new char[i32InfoLogLength+256];
//			sprintf(pszMsg, "Failed to compile fragment shader: %s", pszInfoLog);
//			PVRShellSet(prefExitMessage, pszMsg);
			delete [] pszMsg;
			delete [] pszInfoLog;
			return false;
		}

		// Loads the vertex shader in the same way
		pData->uiVertexShader = pData->glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
		pData->glShaderSourceARB(pData->uiVertexShader, 1, &c_pszVertexShader, NULL);
		pData->glCompileShaderARB(pData->uiVertexShader);
		pData->glGetObjectParameterivARB(pData->uiVertexShader, GL_OBJECT_COMPILE_STATUS_ARB, &bShaderCompiled);
		if (!bShaderCompiled)
		{
			int i32InfoLogLength, i32CharsWritten;
			pData->glGetObjectParameterivARB(pData->uiVertexShader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &i32InfoLogLength);
			char* pszInfoLog = new char[i32InfoLogLength];
			pData->glGetInfoLogARB(pData->uiVertexShader, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
			char* pszMsg = new char[i32InfoLogLength+256];
//			sprintf(pszMsg, "Failed to compile vertex shader: %s", pszInfoLog);
//			PVRShellSet(prefExitMessage, pszMsg);
			delete [] pszMsg;
			delete [] pszInfoLog;
			return false;
		}

		// Create the shader program
		pData->uiProgramObject = pData->glCreateProgramObjectARB();

		// Attach the fragment and vertex shaders to it
		pData->glAttachObjectARB(pData->uiProgramObject, pData->uiFragShader);
		pData->glAttachObjectARB(pData->uiProgramObject, pData->uiVertexShader);

		// Bind the custom vertex attribute "myVertex" to location VERTEX_ARRAY
		pData->glBindAttribLocationARB(pData->uiProgramObject, VERTEX_ARRAY, "myVertex");

		// Link the program
		pData->glLinkProgramARB(pData->uiProgramObject);

		// Check if linking succeeded in the same way we checked for compilation success
		GLint bLinked;
		pData->glGetObjectParameterivARB(pData->uiProgramObject, GL_OBJECT_LINK_STATUS_ARB, &bLinked);
		if (!bLinked)
		{
			int i32InfoLogLength, i32CharsWritten;
			pData->glGetObjectParameterivARB(pData->uiProgramObject, GL_OBJECT_INFO_LOG_LENGTH_ARB, &i32InfoLogLength);
			char* pszInfoLog = new char[i32InfoLogLength];
			pData->glGetInfoLogARB(pData->uiProgramObject, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
			char* pszMsg = new char[i32InfoLogLength+256];
//			sprintf(pszMsg, "Failed to link program: %s", pszInfoLog);
//			PVRShellSet(prefExitMessage, pszMsg);
			delete [] pszMsg;
			delete [] pszInfoLog;
			return false;
		}
	}

	return pData;
}

void CPVRScopeGraph::APISize(SPVRScopeAPIData * const pData, const unsigned int, const unsigned int)
{
	delete [] pData->pVtx;
	delete [] pData->pVtxLines;

	pData->pVtx = new SVertex[m_nSizeCB];
	pData->pVtxLines = new SVertex[6];

	pData->pVtxLines[0].x = m_fX;
	pData->pVtxLines[0].y = m_fY;

	pData->pVtxLines[1].x = m_fX + m_nSizeCB * m_fPixelW;
	pData->pVtxLines[1].y = m_fY;

	pData->pVtxLines[2].x = m_fX;
	pData->pVtxLines[2].y = m_fY + m_fGraphH * 0.5f;

	pData->pVtxLines[3].x = m_fX + m_nSizeCB * m_fPixelW;
	pData->pVtxLines[3].y = m_fY + m_fGraphH * 0.5f;

	pData->pVtxLines[4].x = m_fX;
	pData->pVtxLines[4].y = m_fY + m_fGraphH;

	pData->pVtxLines[5].x = m_fX + m_nSizeCB * m_fPixelW;
	pData->pVtxLines[5].y = m_fY + m_fGraphH;
}

void CPVRScopeGraph::APIShutdown(SPVRScopeAPIData ** const ppData)
{
	SPVRScopeAPIData *pData;

	pData	= *ppData;
	*ppData	= 0;

	// Delete program and shader objects
	pData->glDeleteProgramsARB(1, &pData->uiProgramObject);

	pData->glDeleteObjectARB(pData->uiVertexShader);
	pData->glDeleteObjectARB(pData->uiFragShader);

	delete [] pData->pVtx;
	delete [] pData->pVtxLines;
	delete pData;
}

void CPVRScopeGraph::APIRender(SPVRScopeAPIData * const pData)
{
	float fRatio;
	GLboolean bDepthTest = glIsEnabled(GL_DEPTH_TEST);

	glDisable(GL_DEPTH_TEST);
//	glDisable(GL_CULL_FACE);

	// Use the loaded shader program
	pData->glUseProgramObjectARB(pData->uiProgramObject);

	pData->glEnableVertexAttribArrayARB(VERTEX_ARRAY);
	pData->glVertexAttribPointerARB(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, pData->pVtxLines);
	glDrawElements(GL_LINES, 10, GL_UNSIGNED_SHORT, c_pwLineIdx);

	pData->glVertexAttribPointerARB(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, pData->pVtx);

	// Generate geometry
	for(int iDst = 0, iSrc = m_nWritePosCB; iDst < (int)m_nSizeCB; ++iDst, ++iSrc)
	{
		// Wrap the source index when necessary
		if(iSrc >= (int) m_nSizeCB)
		{
			iSrc = 0;
		}

		// X
		pData->pVtx[iDst].x = m_fX + iDst * m_fPixelW;

		// Y
		fRatio = ((float)m_pnValueCB[iSrc]) / ((float)m_pCounters[m_nActiveCounter].nMaximum);
		if(fRatio < 0)
		{
			fRatio = 0;
		}
		else if(fRatio > 1)
		{
			fRatio = 1;
		}
		pData->pVtx[iDst].y = m_fY + fRatio * m_fGraphH;
	}

	// Render geometry
	glDrawArrays(GL_POINTS, 0, m_nSizeCB);

	pData->glDisableVertexAttribArrayARB(VERTEX_ARRAY);

	pData->glUseProgramObjectARB(0);

	if(bDepthTest)
		glEnable(GL_DEPTH_TEST);
}

/*****************************************************************************
 End of file (PVRScopeGraphAPI.cpp)
*****************************************************************************/
