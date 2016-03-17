
#include "PVRScope.h"
#include "PVRScopeGraph.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <GLES2/gl2.h>

/****************************************************************************
** Defines
****************************************************************************/
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
};

/****************************************************************************
** Constants
****************************************************************************/
static const char *c_pszVertexShader =
	"attribute vec2\tmyVertex;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\tgl_Position = vec4(myVertex, 1, 1);\r\n"
	"\tgl_PointSize = 1.;\r\n"
	"}\r\n";

static const char *c_pszFragmentShader =
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
		pData->pVtx = 0;
		pData->pVtxLines = 0;

		// Create the fragment shader object
		pData->uiFragShader = glCreateShader(GL_FRAGMENT_SHADER);

		// Load the source code into it
		glShaderSource(pData->uiFragShader, 1, &c_pszFragmentShader, 0);

		// Compile the source code
		glCompileShader(pData->uiFragShader);

		// Check if compilation succeeded
		GLint bShaderCompiled;
		glGetShaderiv(pData->uiFragShader, GL_COMPILE_STATUS, &bShaderCompiled);
		if (!bShaderCompiled)
		{
			// An error happened, first retrieve the length of the log message
			int i32InfoLogLength, i32CharsWritten;
			glGetShaderiv(pData->uiFragShader, GL_INFO_LOG_LENGTH, &i32InfoLogLength);

			// Allocate enough space for the message and retrieve it
			char* pszInfoLog = new char[i32InfoLogLength];
			glGetShaderInfoLog(pData->uiFragShader, i32InfoLogLength, &i32CharsWritten, pszInfoLog);

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
		pData->uiVertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(pData->uiVertexShader, 1, &c_pszVertexShader, 0);
		glCompileShader(pData->uiVertexShader);
		glGetShaderiv(pData->uiVertexShader, GL_COMPILE_STATUS, &bShaderCompiled);
		if (!bShaderCompiled)
		{
			int i32InfoLogLength, i32CharsWritten;
			glGetShaderiv(pData->uiVertexShader, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
			char* pszInfoLog = new char[i32InfoLogLength];
			glGetShaderInfoLog(pData->uiVertexShader, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
			char* pszMsg = new char[i32InfoLogLength+256];
//			sprintf(pszMsg, "Failed to compile vertex shader: %s", pszInfoLog);
//			PVRShellSet(prefExitMessage, pszMsg);
			delete [] pszMsg;
			delete [] pszInfoLog;
			return false;
		}

		// Create the shader program
		pData->uiProgramObject = glCreateProgram();

		// Attach the fragment and vertex shaders to it
		glAttachShader(pData->uiProgramObject, pData->uiFragShader);
		glAttachShader(pData->uiProgramObject, pData->uiVertexShader);

		// Bind the custom vertex attribute "myVertex" to location VERTEX_ARRAY
		glBindAttribLocation(pData->uiProgramObject, VERTEX_ARRAY, "myVertex");

		// Link the program
		glLinkProgram(pData->uiProgramObject);

		// Check if linking succeeded in the same way we checked for compilation success
		GLint bLinked;
		glGetShaderiv(pData->uiProgramObject, GL_LINK_STATUS, &bLinked);
		if (!bLinked)
		{
			int i32InfoLogLength, i32CharsWritten;
			glGetShaderiv(pData->uiProgramObject, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
			char* pszInfoLog = new char[i32InfoLogLength];
			glGetShaderInfoLog(pData->uiProgramObject, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
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
	glDeleteProgram(pData->uiProgramObject);

	glDeleteShader(pData->uiVertexShader);
	glDeleteShader(pData->uiFragShader);

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
	glUseProgram(pData->uiProgramObject);

	glEnableVertexAttribArray(VERTEX_ARRAY);
	glVertexAttribPointer(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, pData->pVtxLines);
	glDrawElements(GL_LINES, 10, GL_UNSIGNED_SHORT, c_pwLineIdx);

	glVertexAttribPointer(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, pData->pVtx);

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

	glDisableVertexAttribArray(VERTEX_ARRAY);

	glUseProgram(0);

	if(bDepthTest)
		glEnable(GL_DEPTH_TEST);
}

/*****************************************************************************
 End of file (PVRScopeGraphAPI.cpp)
*****************************************************************************/
