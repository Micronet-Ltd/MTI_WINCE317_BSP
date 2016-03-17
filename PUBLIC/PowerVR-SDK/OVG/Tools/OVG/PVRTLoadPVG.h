/******************************************************************************

 @File         PVRTLoadPVG.h

 @Title        

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     ANSI compatible

 @Description  Include file for loading and display a PVG file.

******************************************************************************/
#include "vg/openvg.h"

#ifndef _PVRTLOADOVG_H_
#define _PVRTLOADOVG_H_

/*!****************************************************************************
 @Struct      PathData
 @Brief       Holds PVG path data
******************************************************************************/
typedef struct
{
	VGFillRule      m_fillRule;
	VGPaintMode     m_paintMode;
	VGCapStyle      m_capStyle;
	VGJoinStyle     m_joinStyle;
	float           m_fMiterLimit;
	float           m_fStrokeWidth;
	VGPaint         m_fillPaint;
	VGPaint         m_strokePaint;
	VGPath          m_path;
	bool			m_bIsNewStroke;
	bool			m_bIsNewFill;
	bool			m_bNeedsBlending;
	unsigned int	m_ui32DashID;
	unsigned int	m_ui32NumDashes;
	float			m_fDashPhase;
	float			m_fDashValues[6];
} PathData;

/*!****************************************************************************
 @Class       CPVRTPVGObject
 @Brief       Stores and displays loaded PVG data
******************************************************************************/
class CPVRTPVGObject
{
protected:
	bool m_bInitialized;

	/*!***************************************************************************
	 @Function		CPVRTPVGObject
	 @Description	CPVRTPVGObject class constructor where global variables are set up
	*****************************************************************************/
	CPVRTPVGObject();

	/*!***************************************************************************
	 @Function		LoadMemoryBuffer
	 @Input			pui8DataFile Data Buffer
	 @Input			i32FileSize	 Size of buffer
	 @Returns 		True on success
	 @Description	Process a PVG data block.
	*****************************************************************************/
	bool LoadMemoryBuffer(unsigned char* pui8DataFile, int i32FileSize);

public:
	PathData* m_pPaths;
	VGPaint* m_pPaints;
	float m_fLeft, m_fTop, m_fRight, m_fBottom, m_fWidth, m_fHeight;
	int m_i32NumPaths;
	int m_i32NumPaints;

	/*!***************************************************************************
	 @Function		~CPVRTPVGObject
	 @Description	CPVRTPVGObject class destructor where Paths and Paints are deallocated
	*****************************************************************************/
	~CPVRTPVGObject();

	/*!***************************************************************************
	 @Function		FromFile
	 @Input			pszFilepath
	 @Returns		A PVG Object
	 @Description	Load a PVG file and sent it for processing.
	*****************************************************************************/
	static CPVRTPVGObject* FromFile(const char* pszFilepath);

	/*!***************************************************************************
	 @Function		FromMemoryBuffer
	 @Input			pui8Buffer Data buffer
	 @Input			i32Size	   Size of buffer
	 @Returns		A PVG Object
	 @Description	Load a PVG data block from memory and send it for processing.
	*****************************************************************************/
	static CPVRTPVGObject* FromMemoryBuffer(unsigned char* pui8Buffer, int i32Size);

	/*!***************************************************************************
	 @Function		Draw
	 @Input			i32StartPath Path to start drawing from
	 @Input			i32EndPath	Path to end drawing on
	 @Returns		True on success
	 @Description	Draw a set of the paths loaded from the file. If no argument
					is specified this function will draw all paths.
	*****************************************************************************/
	bool Draw(int i32StartPath = 0, int i32EndPath = -1);

	/*!***************************************************************************
	 @Function		SetAlpha
	 @Input			ui8Alpha (0x00 = Fuly translucent, 0xFF = fully opaque)
	 @Description	Overwrites path translucency parameter
	*****************************************************************************/
	void SetAlpha(unsigned char ui8Alpha);

	/*!***************************************************************************
	 @Function		SetupCenterOnOrigin
	 @Description	Move artwork center to origin so full figure is displayed.
	*****************************************************************************/
	void SetupCenterOnOrigin();

	/*!***************************************************************************
	 @Function		SetupTranslateToOrigin
	 @Description	Function used to move the artwork center to the origin of the
					coordinate system so the artwork can be cetered on the screen.
	*****************************************************************************/
	void SetupTranslateToOrigin();

	/*!***************************************************************************
	 @Function		SetupScaleToSize
	 @Input			fTargetWidth	Target width to scale to
	 @Input			fTargetHeight	Target height to scale to
	 @Input			bKeepAspect		Maintain the aspect ratio
	 @Description	Function used to fit the artwork on the screen.
	*****************************************************************************/
	void SetupScaleToSize(float fTargetWidth, float fTargetHeight, bool bKeepAspect = true);
};

#endif // _PVRTLOADOVG_H_

/*****************************************************************************
 End of file (PVRTLoadPVG.h)
*****************************************************************************/

