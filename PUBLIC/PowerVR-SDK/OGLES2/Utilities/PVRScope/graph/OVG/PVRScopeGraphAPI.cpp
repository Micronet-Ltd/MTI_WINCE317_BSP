
#include "PVRScope.h"
#include "PVRScopeGraph.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <vg/openvg.h>

/****************************************************************************
** Defines
****************************************************************************/


/****************************************************************************
** Structures
****************************************************************************/
struct SVertex
{
	VGfloat x, y;
};

struct SPVRScopeAPIData
{
	VGPath	vgGraph;
	VGPath	vgLines;
	float	fTransform[9];
	float	fStrokeWidth;
	VGPaint vgStrokePaint;

	SVertex *pPoints;
	VGubyte *pSegments;
};

/****************************************************************************
** Constants
****************************************************************************/

/****************************************************************************
** Code
****************************************************************************/
SPVRScopeAPIData *CPVRScopeGraph::APIInit()
{
	SPVRScopeAPIData *pData;

	pData = new SPVRScopeAPIData;

	if(pData)
	{
		pData->vgGraph = vgCreatePath(VG_PATH_FORMAT_STANDARD,VG_PATH_DATATYPE_F, 1.0f, 0.0f, 0, 0, VG_PATH_CAPABILITY_APPEND_TO);
		pData->vgLines = vgCreatePath(VG_PATH_FORMAT_STANDARD,VG_PATH_DATATYPE_F, 1.0f, 0.0f, 0, 0, VG_PATH_CAPABILITY_APPEND_TO);

		memset(pData->fTransform, 0, sizeof(*pData->fTransform) * 9);
		pData->fTransform[0] = 1.0f;
		pData->fTransform[4] = 1.0f;
		pData->fTransform[8] = 1.0f;

		pData->fStrokeWidth = 1.0f;

		pData->vgStrokePaint = vgCreatePaint();

		pData->vgStrokePaint = vgCreatePaint();
		vgSetParameteri(pData->vgStrokePaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
		vgSetColor(pData->vgStrokePaint, 0xFF0000FF);

		pData->pPoints = 0;
		pData->pSegments = 0;
	}

	return pData;
}

void CPVRScopeGraph::APISize(SPVRScopeAPIData * const pData, const unsigned int nViewportW, const unsigned int nViewportH)
{
	vgClearPath(pData->vgLines, VG_PATH_CAPABILITY_APPEND_TO);
	
	VGubyte g_aui8PathSegments[] = {
		VG_MOVE_TO_ABS, 
		VG_LINE_TO_ABS,
		VG_LINE_TO_ABS,
		VG_LINE_TO_ABS,
		VG_LINE_TO_ABS,
		
		VG_MOVE_TO_ABS, 
		VG_LINE_TO_ABS
	};

	VGfloat g_afPathCoords[] = {
			m_fX, m_fY,
			m_fX + m_nSizeCB * m_fPixelW, m_fY,	

			m_fX + m_nSizeCB * m_fPixelW, m_fY + m_fGraphH,

			m_fX, m_fY + m_fGraphH,

			m_fX, m_fY,

			m_fX, m_fY + m_fGraphH * 0.5f,	
			m_fX + m_nSizeCB * m_fPixelW, m_fY + m_fGraphH * 0.5f,
	};

	vgAppendPathData(pData->vgLines, 7, g_aui8PathSegments, g_afPathCoords);

	pData->fTransform[0] = nViewportW * 0.5f;
	pData->fTransform[4] = nViewportH * 0.5f;
	pData->fTransform[6] = nViewportW * 0.5f;
	pData->fTransform[7] = nViewportH * 0.5f;

	pData->fStrokeWidth = 3.0f / nViewportH;

	delete[] pData->pPoints;
	delete[] pData->pSegments;

	pData->pPoints   = new SVertex[m_nSizeCB];
	pData->pSegments = new VGubyte[m_nSizeCB];

	pData->pSegments[0] = VG_MOVE_TO_ABS;

	for(unsigned int i = 1; i < m_nSizeCB; ++i)
		pData->pSegments[i] = VG_LINE_TO_ABS;
}

void CPVRScopeGraph::APIShutdown(SPVRScopeAPIData ** const ppData)
{
	SPVRScopeAPIData *pData;

	pData	= *ppData;
	*ppData	= 0;

	vgDestroyPath(pData->vgGraph);
	vgDestroyPath(pData->vgLines);
	vgDestroyPaint(pData->vgStrokePaint);

	delete[] pData->pPoints;
	delete[] pData->pSegments;

	delete pData;
}

void CPVRScopeGraph::APIRender(SPVRScopeAPIData * const pData)
{
	float fRatio;
	
	VGint i32MatrixMode = vgGeti(VG_MATRIX_MODE);
	VGint i32BlendMode = vgGeti(VG_BLEND_MODE);
	VGfloat fStrokeWidth = vgGetf(VG_STROKE_LINE_WIDTH);

	float fCurrentMatrix[9];

	vgSeti(VG_BLEND_MODE, VG_BLEND_SRC);

	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
	vgGetMatrix(fCurrentMatrix);
	vgLoadMatrix(pData->fTransform);

	vgSetf(VG_STROKE_LINE_WIDTH, pData->fStrokeWidth);
	vgSetPaint(pData->vgStrokePaint, VG_STROKE_PATH);

	vgDrawPath(pData->vgLines, VG_STROKE_PATH);
	
	vgClearPath(pData->vgGraph, VG_PATH_CAPABILITY_APPEND_TO);

	// Generate geometry
	for(int iDst = 0, iSrc = m_nWritePosCB; iDst < (int)m_nSizeCB; ++iDst, ++iSrc)
	{
		// Wrap the source index when necessary
		if(iSrc >= (int) m_nSizeCB)
		{
			iSrc = 0;
		}

		// X
		pData->pPoints[iDst].x = m_fX + iDst * m_fPixelW;

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
		pData->pPoints[iDst].y = m_fY + fRatio * m_fGraphH;
	}

	vgAppendPathData(pData->vgGraph, m_nSizeCB, pData->pSegments, pData->pPoints);

	// Render geometry
	vgDrawPath(pData->vgGraph, VG_STROKE_PATH);

	// Reset our state changes
	vgSetPaint(0, VG_STROKE_PATH);

	vgLoadMatrix(fCurrentMatrix);

	vgSeti(VG_MATRIX_MODE, i32MatrixMode);
	vgSeti(VG_BLEND_MODE, i32BlendMode);
	vgSetf(VG_STROKE_LINE_WIDTH, fStrokeWidth);
}

/*****************************************************************************
 End of file (PVRScopeGraphAPI.cpp)
*****************************************************************************/
