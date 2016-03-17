/******************************************************************************

 @File         PVRTPrintVG.h

 @Title        

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     ANSI compatible

 @Description  Header file for PVRTPrintVG.

******************************************************************************/
#ifndef _PVRTPRINTVG_H_
#define _PVRTPRINTVG_H_


#include "vg/openvg.h"
#include "PVRTContext.h"
#include "../PVRTVector.h"

/****************************************************************************
** Enumerations
****************************************************************************/
typedef enum {
	ePVRTPrint3DLogoNone  = 0x00,
	ePVRTPrint3DLogoPVR = 0x02,
	ePVRTPrint3DLogoIMG = 0x04,
	ePVRTPrint3DSDKLogo = ePVRTPrint3DLogoIMG
} EPVRTPrintVGLogo;

typedef enum {
	ePVRTPrintVGLogoNone,
	ePVRTPrintVGLogoRight,
	ePVRTPrintVGLogoLeft
} EPVRTPrintVGLogoPos;

/****************************************************************************
** Structures
****************************************************************************/
/*!****************************************************************************
 @Struct      SState
 @Brief       A struct for storing OpenVG states that are modified by PrintVG
******************************************************************************/
struct SState{
	VGint i32ImageMode;
	VGint i32BlendMode;
	VGint i32ImageQuality;
	VGint i32MatrixMode;
	PVRTMat3 fImageUserToSurface;
};

/****************************************************************************
** Functions
****************************************************************************/
/*!****************************************************************************
 @Class       CPVRTPrintVG
 @Brief       Display text/logos on the screen
******************************************************************************/
class CPVRTPrintVG
{
public:
	/*!***************************************************************************
	 @Function			CPVRTPrintVG
	 @Description		Constructor.
	*****************************************************************************/
	CPVRTPrintVG();

	/*!***************************************************************************
	 @Function			~CPVRTPrintVG
	 @Description		Destructor
	*****************************************************************************/
	~CPVRTPrintVG();

	/*!***************************************************************************
	 @Function			Initialize
	 @Input				ui32Width Screen width
	 @Input				ui32Height Screen height
	 @Input				ScaleX Scale in the x-dimension
	 @Input				ScaleY Scale in the y-dimension
	 @Returns			true on success
	 @Description		Initialises PrintVG
	*****************************************************************************/
	bool Initialize(unsigned int ui32Width, unsigned int ui32Height, float ScaleX = 1.0f, float ScaleY = 1.0f);

	/*!***************************************************************************
	 @Function			Terminate
	 @Returns			true on success
	 @Description		Tidies up PrintVG
	*****************************************************************************/
	bool Terminate();

	/*!***************************************************************************
	 @Function			DrawString
	 @Input				X x-coordinate of where to draw the string
	 @Input				Y y-coordinate of where to draw the string
	 @Input				Size The size of the text
	 @Input				String The string to display
	 @Input				Colour The colour of the string when displayed
	 @Input				bSaveState Set whether to save the OpenVG state or now
	 @Returns			true on success
	 @Description		Draws a string
	*****************************************************************************/
	bool DrawString(float X, float Y, float Size, const char* String, int Colour = -1, bool bSaveState=true);

	/*!***************************************************************************
	 @Function			DrawString
	 @Input				X x-coordinate of where to draw the string
	 @Input				Y y-coordinate of where to draw the string
	 @Input				Size The size of the text
	 @Input				String The string to display
	 @Input				Colour The colour of the string when displayed
	 @Returns			true on success
	 @Description		Draws a string with a shadow
	*****************************************************************************/
	bool DrawShadowString(float X, float Y, float Size, const char* String, int Colour = -1);

	/*!***************************************************************************
	 @Function			DisplayDefaultTitle
	 @Input				pszTitle The title string to display
	 @Input				pszDescription The description to display below the title
	 @Input				ui32DisplayLogo The logo to display
	 @Returns			true on success
	 @Description		Displays the title, a description and a logo.
	*****************************************************************************/
	bool DisplayDefaultTitle(char *pszTitle, char *pszDescription, unsigned int ui32DisplayLogo);

	/*!***************************************************************************
	 @Function			SetQuality
	 @Input				vgiq The image quality to use for the font
	 @Description		Allows the user to set the image quality
	*****************************************************************************/
	void SetQuality(VGImageQuality vgiq);

private:
	/*!***************************************************************************
	 @Function			DrawLogo
	 @Input				ui32DisplayLogo The logo to draw
	 @Input				ui32Pos A predefined position on where to display the logo
	 @Returns			true on success
	 @Description		Draws the requested logo
	*****************************************************************************/
	bool DrawLogo(unsigned int ui32DisplayLogo, unsigned int ui32Pos);

	/*!***************************************************************************
	 @Function			LoadLogoImage
	 @Input				ui32DisplayLogo The logo to load
	 @Returns			true on success
	 @Description		Loads the required printvg logos
	*****************************************************************************/
	bool LoadLogoImage(unsigned int ui32DisplayLogo);

	/*!***************************************************************************
	 @Function			SaveState
	 @Returns			true on success
	 @Description		Save OpenVG state values that PrintVG will change
	*****************************************************************************/
	bool SaveState();

	/*!***************************************************************************
	 @Function			RestoreState
	 @Returns			true on success
	 @Description		Restores OpenVG state values that were saved by SaveState
	*****************************************************************************/
	bool RestoreState();

private:
	SState  m_OVGState;
	VGPaint m_vgLogoPaint;
	VGPaint m_vgFontPaint;
	VGImage m_vgIMGLogo;
	VGImage m_vgPVRImage;

	VGImage m_vgiCharMap;
	VGImage m_vgiChars[256];
	VGImageQuality m_vgImageQuality;

	struct
	{
		int top, left, width, height;
	} m_charRect[256];

	bool m_bInitialized;
	float m_fScaleX, m_fScaleY;
	unsigned int m_ui32Height;
	unsigned int m_ui32Width;
};


#endif /* _PRINTVG_H_ */

/*****************************************************************************
 End of file (PVRTPrintVG.h)
*****************************************************************************/
