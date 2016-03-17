/******************************************************************************

 @File         PVRTPrintVG.cpp

 @Title        

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     ANSI Independant

 @Description  Display a text string passed by the user. This class uses OpenVG
               image functionality to display glyphs. A later version might use a
               vector based font.

******************************************************************************/
#include "PVRTPrintVG.h"
#include "PVRTPrint3Ddat.h"
#include "vg/openvg.h"

const VGImageQuality vgImageQualityDefault = VG_IMAGE_QUALITY_FASTER;
const int nMapWidth = 256;
const int nMapHeight = 256;

/*!***************************************************************************
@Function			CPVRTPrintVG
@Description		Constructor.
*****************************************************************************/
CPVRTPrintVG::CPVRTPrintVG()
{
	m_vgiCharMap = 0;
	m_vgIMGLogo  = 0;
	m_vgPVRImage = 0;
	m_vgLogoPaint= 0;
	m_vgFontPaint= 0;

	m_vgImageQuality = vgImageQualityDefault;
	m_bInitialized = false;
}

/*!***************************************************************************
@Function			~CPVRTPrintVG
@Description		Destructor
*****************************************************************************/
CPVRTPrintVG::~CPVRTPrintVG()
{
	Terminate();
}

/*!***************************************************************************
@Function			Initialize
@Input				ui32Width Screen width
@Input				ui32Height Screen height
@Input				ScaleX Scale in the x-dimension
@Input				ScaleY Scale in the y-dimension
@Returns			true on success
@Description		Initialises PrintVG
*****************************************************************************/
bool CPVRTPrintVG::Initialize(unsigned int ui32Width, unsigned int ui32Height, float ScaleX, float ScaleY)
{
	m_ui32Width  = ui32Width;
	m_ui32Height = ui32Height;

	m_fScaleX = ScaleX;
	m_fScaleY = ScaleY;

	// clear OpenVG error so we don't accidentally report a previously ignored error
	vgGetError();

	// create an OpenVG image for the character map
	m_vgiCharMap = vgCreateImage(
		VG_A_8,
		nMapWidth, nMapHeight,
		VG_IMAGE_QUALITY_FASTER | VG_IMAGE_QUALITY_BETTER);

	vgImageSubData(m_vgiCharMap, PVRTPrint3DABC_Pixels, nMapWidth, VG_A_8, 0, 0, nMapWidth, nMapHeight);

	if(vgGetError() != VG_NO_ERROR)
	{
		// "CPVRTPrintVG::Initialize - failed to create image (character map)";
		return false;
	}

	for(int i = 0; i < 256; ++i)
	{
		m_charRect[i].top = (int)(PVRTPrint3DV_Bold[i] * 256) - 28;
		m_charRect[i].left = (int)(PVRTPrint3DU_Bold[i] * 256) + 1;
		m_charRect[i].width = (int)(PVRTPrint3DSize_Bold[i] * 256) + 1;
		m_charRect[i].height = 28;

		if(m_charRect[i].width > 2)
		{
			m_vgiChars[i] = vgChildImage(m_vgiCharMap,
								m_charRect[i].left,
								m_charRect[i].top,
								m_charRect[i].width,
								m_charRect[i].height);
			if((vgGetError() != VG_NO_ERROR) || (m_vgiChars[i] == 0))
			{
				// "CPVRTPrintVG::Initialize - failed to create image (character child)";
				return false;
			}
		}
		else
		{
			m_vgiChars[i] = 0;
		}

	}

	m_vgFontPaint = vgCreatePaint();
	vgSetParameteri(m_vgFontPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);

	// Initialization done, ready to draw text
	m_bInitialized = true;
	return true;
}

/*!***************************************************************************
@Function			Terminate
@Returns			true on success
@Description		Tidies up PrintVG
*****************************************************************************/
bool CPVRTPrintVG::Terminate()
{
	if (!m_bInitialized) return true;

	m_bInitialized = false;

	// destroy the character map image and all child images if necessary
	for(int i = 0; i < 256; ++i)
	{
		if(m_vgiChars[i] != 0)
		{
			vgDestroyImage(m_vgiChars[i]);
			m_vgiChars[i] = 0;
		}
	}
	if(m_vgiCharMap)
	{
		vgDestroyImage(m_vgiCharMap);
		m_vgiCharMap = 0;
	}

	if(m_vgIMGLogo)
	{
		vgDestroyImage(m_vgIMGLogo);
		m_vgIMGLogo = 0;
	}

	if(m_vgPVRImage)
	{
		vgDestroyImage(m_vgPVRImage);
		m_vgPVRImage = 0;
	}

	if(m_vgLogoPaint)
	{
		vgDestroyPaint(m_vgLogoPaint);
		m_vgLogoPaint = 0;
	}

	if(m_vgFontPaint)
	{
		vgDestroyPaint(m_vgFontPaint);
		m_vgFontPaint = 0;
	}

	return true;
}

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
bool CPVRTPrintVG::DrawString(float fX, float fY, float fSize, const char* String, int Colour, bool bSaveState)
{
	if(!m_bInitialized)
	{
		// "CPVRTPrintVG::DrawString - CPVRTPrintVG instance not initialized";
		return false;
	}

	if(bSaveState)
		SaveState();

	vgSetColor(m_vgFontPaint, Colour);
	vgSetPaint(m_vgFontPaint, VG_FILL_PATH);

	vgSeti(VG_IMAGE_MODE, VG_DRAW_IMAGE_MULTIPLY);
	vgSeti(VG_BLEND_MODE, VG_BLEND_SRC_OVER);

	vgSeti(VG_IMAGE_QUALITY, m_vgImageQuality);

	vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
	vgLoadIdentity();
	vgScale(m_fScaleX, m_fScaleY);
	vgTranslate(fX, fY);
	vgScale(fSize, fSize);

	int row = 0;
	for(const unsigned char* pc = (const unsigned char*)String; *pc != '\0'; ++pc) // Iterate through string chars
	{
		if(*pc == '\n')
		{
			++row;
			vgLoadIdentity();
			vgScale(m_fScaleX, m_fScaleY);
			vgTranslate(fX, fY);
			vgScale(fSize, fSize);
			vgTranslate(0, -row * 30.0f);
		}
		else if(m_vgiChars[*pc])
		{
			vgDrawImage(m_vgiChars[*pc]);
			vgTranslate(m_charRect[*pc].width + 2.0f, 0);
		}
		else
		{
			vgTranslate(7, 0); // non-printable chars get width 5
		}
	}

	// Put the image mode back to the way it was
	if(bSaveState)
		RestoreState();

	if(vgGetError() != VG_NO_ERROR)
	{
		// "CPVRTPrintVG::DrawString - an OpenVG error ocurred during text drawing";
		return false;
	}

	return true;
}

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
bool CPVRTPrintVG::DrawShadowString(float X, float Y, float Size, const char* String, int Colour)
{
	SaveState();

	if(!DrawString(X + 1, Y - 1, Size, String, PVRTRGBA(0,0,0,255), false))
		return false;
	if(!DrawString(X, Y, Size, String, Colour, false))
		return false;

	RestoreState();

	return true;
}

/*!***************************************************************************
@Function			DisplayDefaultTitle
@Input				pszTitle The title string to display
@Input				pszDescription The description to display below the title
@Input				ui32DisplayLogo The logo to display
@Returns			true on success
@Description		Displays the title, a description and a logo.
*****************************************************************************/
bool CPVRTPrintVG::DisplayDefaultTitle(char *pszTitle, char *pszDescription, unsigned int ui32DisplayLogo)
{
	if(!m_bInitialized)
		return false;

	SaveState();

	bool bResult = true;
	float fScaleX, fScaleY;

	/*
		We don't want to apply the scale to the title so save the current scale and then set
		it to 1.0f.
	*/
	fScaleX = m_fScaleX;
	fScaleY = m_fScaleY;

	m_fScaleX = m_fScaleY = 1.0f;

	/*
		Display Title
	*/
	if(pszTitle)
	{
		bResult &= DrawString(2.0f, (float) m_ui32Height - 20, 0.7f,  pszTitle, (int) PVRTRGBA(255, 255, 0, 255), false);
	}

	/*
		Display Description
	*/
	if(pszDescription)
	{
		bResult &= DrawString(2.0f, (float) m_ui32Height - 35, 0.5f,  pszDescription, (int) PVRTRGBA(255, 255, 255, 255), false);
	}


	m_fScaleX = fScaleX;
	m_fScaleY = fScaleY;

	/* Draw a logo if requested */
#if defined(FORCE_NO_LOGO)
	/* Do nothing */

#elif defined(FORCE_PVR_LOGO)
    DrawLogo(ePVRTPrint3DLogoPVR, ePVRTPrintVGLogoRight);	/* PVR to the right */

#elif defined(FORCE_IMG_LOGO)
	DrawLogo(ePVRTPrint3DLogoIMG, ePVRTPrintVGLogoRight);	/* IMG to the right */

#elif defined(FORCE_ALL_LOGOS)
	DrawLogo(ePVRTPrint3DLogoIMG, ePVRTPrintVGLogoLeft); /* IMG to the left */
	DrawLogo(ePVRTPrint3DLogoPVR, ePVRTPrintVGLogoRight);	/* PVR to the right */

#else
	/* User selected logos */
	switch (ui32DisplayLogo)
	{
		case ePVRTPrint3DLogoNone:
			break;
		default:
		case ePVRTPrint3DLogoPVR:
			DrawLogo(ePVRTPrint3DLogoPVR, ePVRTPrintVGLogoRight);	/* PVR to the right */
			break;
		case ePVRTPrint3DLogoIMG:
			DrawLogo(ePVRTPrint3DLogoIMG, ePVRTPrintVGLogoRight);	/* IMG to the right */
			break;
		case (ePVRTPrint3DLogoPVR | ePVRTPrint3DLogoIMG):
			DrawLogo(ePVRTPrint3DLogoIMG, ePVRTPrintVGLogoLeft);  /* IMG to the left */
			DrawLogo(ePVRTPrint3DLogoPVR, ePVRTPrintVGLogoRight);	/* PVR to the right */
			break;
	}
#endif

	RestoreState();

	return bResult;
}

/*!***************************************************************************
@Function			SetQuality
@Input				vgiq The image quality to use for the font
@Description		Allows the user to set the image quality
*****************************************************************************/
void CPVRTPrintVG::SetQuality(VGImageQuality vgiq)
{
	m_vgImageQuality = vgiq;
}

/*!***************************************************************************
@Function			DrawLogo
@Input				ui32DisplayLogo The logo to draw
@Input				ui32Pos A predefined position on where to display the logo
@Returns			true on success
@Description		Draws the requested logo
*****************************************************************************/
bool CPVRTPrintVG::DrawLogo(unsigned int ui32DisplayLogo, unsigned int ui32Pos)
{
	VGImage *pImage = 0;

	switch(ui32DisplayLogo)
	{
		case ePVRTPrint3DLogoIMG:
			if(m_vgIMGLogo)
				pImage = &m_vgIMGLogo;
		break;
		case ePVRTPrint3DLogoPVR:
			if(m_vgPVRImage)
				pImage = &m_vgPVRImage;
		break;
	}

	if(!pImage)
	{
		if(!LoadLogoImage(ui32DisplayLogo))
			return false;
		else
			return DrawLogo(ui32DisplayLogo, ui32Pos);
	}

	vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
	vgLoadIdentity();
	float fScaleX = 0.6f;
	float fScaleY = 0.4f;
	int i32ImageWidth;
	int i32ImageHeight = vgGetParameteri(*pImage, VG_IMAGE_HEIGHT);

	/*
		For the logos we are using data that is used in OGLES etc.. which has
		the y-coordinate zero'd at the top of the screen where as OpenVG has
		the zero at the bottom.

		Therefore I've inverted the translation and the scale.
	*/

	switch(ui32Pos)
	{
		case ePVRTPrintVGLogoRight:
			i32ImageWidth = vgGetParameteri(*pImage, VG_IMAGE_WIDTH);
			vgTranslate((VGfloat) m_ui32Width - (i32ImageWidth * fScaleX) - 5.0f, 5.0f + (i32ImageHeight * fScaleY));
		break;
		case ePVRTPrintVGLogoLeft:
			vgTranslate(5.0f, 5.0f + (i32ImageHeight * fScaleY));
		break;
	}

	vgScale(fScaleX, -fScaleY);

	vgSetPaint(m_vgLogoPaint, VG_FILL_PATH);

	vgSeti(VG_IMAGE_MODE, VG_DRAW_IMAGE_MULTIPLY);
	vgSeti(VG_BLEND_MODE, VG_BLEND_SRC_OVER);

	vgSeti(VG_IMAGE_QUALITY, m_vgImageQuality);

	vgDrawImage(*pImage);

	return true;
}

/*!***************************************************************************
@Function			LoadLogoImage
@Input				ui32DisplayLogo The logo to load
@Returns			true on success
@Description		Loads the required printvg logos
*****************************************************************************/
bool CPVRTPrintVG::LoadLogoImage(unsigned int ui32DisplayLogo)
{
	VGImage *pImage = 0;
	unsigned long  *pLogoData = 0;

	switch(ui32DisplayLogo)
	{
		case ePVRTPrint3DLogoIMG:
			if(m_vgIMGLogo)
				vgDestroyImage(m_vgIMGLogo);

			pLogoData = PVRTPrint3DIMGLogo;

			if(!pLogoData)
				return false;

			m_vgIMGLogo = vgCreateImage(VG_sRGBA_8888, PVRTPrint3DIMGLogo[2], PVRTPrint3DIMGLogo[1], m_vgImageQuality);

			pImage = &m_vgIMGLogo;
		break;
		case ePVRTPrint3DLogoPVR:
			if(m_vgPVRImage)
				vgDestroyImage(m_vgPVRImage);

			pLogoData = PVRTPrint3DPVRLogo;

			if(!pLogoData)
				return false;

			m_vgPVRImage = vgCreateImage(VG_sRGBA_8888, PVRTPrint3DIMGLogo[2], PVRTPrint3DIMGLogo[1], m_vgImageQuality);

			pImage = &m_vgPVRImage;
		break;
		default:
			return false;
	}

	if(!pImage)
		return false;

	vgImageSubData(*pImage,(VGubyte*) (pLogoData + (pLogoData[0] / sizeof(unsigned long))), sizeof(VGubyte) * pLogoData[2] * 2,VG_sRGB_565, 0, 0, pLogoData[2], pLogoData[1]);

	if(vgGetError() != VG_NO_ERROR)
		return false;

	if(!m_vgLogoPaint)
	{
		m_vgLogoPaint = vgCreatePaint();
		vgSetParameteri(m_vgLogoPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
		vgSetColor(m_vgLogoPaint, PVRTRGBA(255,255,255,192));
	}

	return true;
}

/*!***************************************************************************
@Function			SaveState
@Returns			true on success
@Description		Save OpenVG state values that PrintVG will change
*****************************************************************************/
bool CPVRTPrintVG::SaveState()
{
	m_OVGState.i32ImageMode = vgGeti(VG_IMAGE_MODE);
	m_OVGState.i32BlendMode = vgGeti(VG_BLEND_MODE);
	m_OVGState.i32ImageQuality = vgGeti(VG_IMAGE_QUALITY);

	m_OVGState.i32MatrixMode = vgGeti(VG_MATRIX_MODE);

	vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
	vgGetMatrix(m_OVGState.fImageUserToSurface.f);
	return true;
}

/*!***************************************************************************
@Function			RestoreState
@Returns			true on success
@Description		Restores OpenVG state values that were saved by SaveState
*****************************************************************************/
bool CPVRTPrintVG::RestoreState()
{
	vgSeti(VG_IMAGE_MODE, m_OVGState.i32ImageMode);
	vgSeti(VG_BLEND_MODE, m_OVGState.i32BlendMode);
	vgSeti(VG_IMAGE_QUALITY, m_OVGState.i32ImageQuality);

	vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
	vgLoadMatrix(m_OVGState.fImageUserToSurface.f);

	vgSeti(VG_MATRIX_MODE, m_OVGState.i32MatrixMode);
	return true;
}

/*****************************************************************************
 End of file (PVRTPrintVG.cpp)
*****************************************************************************/



