/******************************************************************************

 @File         OVGFont.cpp

 @Title        OVGFont

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     Independent

 @Description  Shows how to use vgFonts.

******************************************************************************/
#include "PVRShell.h"
#include "OVGTools.h"

#include "vg/openvg.h"

/****************************************************************************
** Constants
****************************************************************************/

// The pvg file that contains the path based font data
const char c_szPVGFile[] = "Font.pvg";
// The pvr file that contains the image based font data
const char c_szPVRFile[] = "ImageFont.pvr";

// Number of characters contained within the image and pvg file
const int g_i32ImageCharNo = 94;

/*
	A struct for storing the required information about each glyph / character

	i32Origin - The bottom left co-ordinates of a glyph in the image
	i32Width  - The width of the glyph in the image
	i32Height - The height of the glyph in the image
	fOriginOffset - The value to offset the glyph by so it is displayed correctly e.g. 'y' needs to be offset so it is lower
	fEscapement - The offset from the origin in which to draw the next character
*/
struct SGlyph
{
	int i32Origin[2];
	int i32Width;
	int i32Height;
	PVRTVECTOR2 fOriginOffset;
	PVRTVECTOR2 fEscapement;
};

// An array storing the description for each glyph
const SGlyph g_CharDesc[g_i32ImageCharNo] = {
{ {246, 202}, 7	, 26, {0, 2}	, {8, 0} 	},
{ {75, 83} 	, 12, 12, {0, -12}	, {11, 0} 	},
{ {80, 119} , 18, 25, {0, 2}	, {16, 0} 	},
{ {19, 225} , 16, 31, {0, 5}	, {17, 0} 	},
{ {194, 149}, 28, 25, {0, 2}	, {26, 0} 	},
{ {151, 202}, 23, 26, {0, 2}	, {20, 0} 	},
{ {88, 83} 	, 7	, 12, {0, -12}	, {6, 0} 	},
{ {62, 226} , 11, 30, {0, 6}	, {9, 0} 	},
{ {86, 226} , 11, 30, {0, 6}	, {9, 0} 	},
{ {28, 75} 	, 16, 15, {0, -9}	, {14, 0} 	},
{ {56, 96} 	, 20, 20, {0, 2}	, {19, 0} 	},
{ {45, 77} 	, 9	, 13, {0, 6}	, {7, 0} 	},
{ {158, 92} , 12, 6	, {0, -6}	, {10, 0} 	},
{ {129, 89} , 7	, 8	, {0, 2}	, {7, 0} 	},
{ {212, 229}, 15, 27, {0, 3}	, {11, 0} 	},
{ {0, 112} 	, 19, 25, {0, 2}	, {17, 0} 	},
{ {244, 149}, 11, 25, {0, 2}	, {17, 0} 	},
{ {99, 119}	, 18, 25, {0, 2}	, {17, 0} 	},
{ {118, 119}, 18, 25, {0, 2}	, {17, 0} 	},
{ {223, 149}, 20, 25, {0, 2}	, {17, 0} 	},
{ {137, 119}, 18, 25, {0, 2}	, {17, 0} 	},
{ {40, 117}	, 19, 25, {0, 2}	, {17, 0} 	},
{ {156, 120}, 17, 25, {0, 2}	, {17, 0} 	},
{ {60, 117}	, 19, 25, {0, 2}	, {17, 0} 	},
{ {20, 112}	, 19, 25, {0, 2}	, {17, 0} 	},
{ {0, 70}	, 7	, 20, {0, 3}	, {7, 0} 	},
{ {188, 124}, 9	, 23, {0, 6}	, {7, 0} 	},
{ {218, 127}, 19, 21, {0, 2}	, {19, 0} 	},
{ {55, 78}	, 19, 12, {0, -2}	, {19, 0} 	},
{ {198, 127}, 19, 21, {0, 2}	, {19, 0} 	},
{ {163, 148}, 14, 26, {0, 2}	, {13, 0} 	},
{ {75, 199}	, 26, 26, {0, 2}	, {24, 0} 	},
{ {175, 202}, 23, 26, {0, 2}	, {20, 0} 	},
{ {40, 143}	, 18, 26, {0, 2}	, {18, 0} 	},
{ {66, 172}	, 21, 26, {0, 2}	, {19, 0} 	},
{ {223, 202}, 22, 26, {0, 2}	, {22, 0} 	},
{ {78, 145}	, 17, 26, {0, 2}	, {16, 0} 	},
{ {131, 145}, 16, 26, {0, 2}	, {16, 0} 	},
{ {199, 202}, 23, 26, {0, 2}	, {22, 0} 	},
{ {44, 170}	, 21, 26, {0, 2}	, {22, 0} 	},
{ {178, 148}, 7	, 26, {0, 2}	, {8, 0} 	},
{ {148, 146}, 14, 26, {0, 2}	, {12, 0} 	},
{ {175, 175}, 20, 26, {0, 2}	, {18, 0} 	},
{ {114, 145}, 16, 26, {0, 2}	, {16, 0} 	},
{ {47, 199}	, 27, 26, {0, 2}	, {27, 0} 	},
{ {132, 173}, 21, 26, {0, 2}	, {22, 0} 	},
{ {102, 199}, 24, 26, {0, 2}	, {23, 0} 	},
{ {59, 143}	, 18, 26, {0, 2}	, {18, 0} 	},
{ {110, 227}, 24, 29, {0, 5}	, {23, 0} 	},
{ {196, 175}, 19, 26, {0, 2}	, {18, 0} 	},
{ {96, 145}	, 17, 26, {0, 2}	, {16, 0} 	},
{ {22, 170}	, 21, 26, {0, 2}	, {17, 0} 	},
{ {0, 165}	, 21, 26, {0, 2}	, {22, 0} 	},
{ {127, 200}, 23, 26, {0, 2}	, {19, 0} 	},
{ {15, 197}	, 31, 26, {0, 2}	, {28, 0} 	},
{ {110, 172}, 21, 26, {0, 2}	, {19, 0} 	},
{ {88, 172}	, 21, 26, {0, 2}	, {18, 0} 	},
{ {154, 175}, 20, 26, {0, 2}	, {19, 0} 	},
{ {98, 226}	, 11, 30, {0, 6}	, {9, 0} 	},
{ {228, 229}, 15, 27, {0, 3}	, {11, 0} 	},
{ {74, 226}	, 11, 30, {0, 6}	, {9, 0} 	},
{ {8, 71}	, 19, 19, {0, -4}	, {19, 0} 	},
{ {137, 91}	, 20, 6	, {0, 6}	, {16, 0} 	},
{ {118, 88}	, 10, 9	, {0, -15}	, {10, 0} 	},
{ {210, 106}, 16, 20, {0, 2}	, {16, 0} 	},
{ {135, 229}, 19, 27, {0, 2}	, {19, 0} 	},
{ {238, 128}, 17, 20, {0, 2}	, {15, 0} 	},
{ {155, 229}, 19, 27, {0, 2}	, {19, 0} 	},
{ {137, 98}	, 18, 20, {0, 2}	, {17, 0} 	},
{ {0, 192}	, 14, 27, {0, 2}	, {10, 0} 	},
{ {216, 175}, 19, 26, {0, 8}	, {19, 0} 	},
{ {194, 229}, 17, 27, {0, 2}	, {18, 0} 	},
{ {186, 148}, 7	, 26, {0, 2}	, {8, 0} 	},
{ {7, 224}	, 11, 32, {0, 8}	, {8, 0} 	},
{ {175, 229}, 18, 27, {0, 2}	, {16, 0} 	},
{ {244, 229}, 7	, 27, {0, 2}	, {8, 0} 	},
{ {0, 91}	, 27, 20, {0, 2}	, {28, 0} 	},
{ {156, 99}	, 17, 20, {0, 2}	, {18, 0} 	},
{ {77, 96}	, 19, 20, {0, 2}	, {18, 0} 	},
{ {20, 138}	, 19, 26, {0, 8}	, {19, 0} 	},
{ {236, 175}, 19, 26, {0, 8}	, {19, 0} 	},
{ {242, 107}, 12, 20, {0, 2}	, {11, 0} 	},
{ {227, 106}, 14, 20, {0, 2}	, {13, 0} 	},
{ {174, 123}, 13, 24, {0, 2}	, {11, 0} 	},
{ {174, 102}, 17, 20, {0, 2}	, {18, 0} 	},
{ {97, 98}	, 19, 20, {0, 2}	, {16, 0} 	},
{ {28, 91}	, 27, 20, {0, 2}	, {24, 0} 	},
{ {117, 98}	, 19, 20, {0, 2}	, {15, 0} 	},
{ {0, 138}	, 19, 26, {0, 8}	, {16, 0} 	},
{ {192, 103}, 17, 20, {0, 2}	, {14, 0} 	},
{ {36, 226}	, 12, 30, {0, 6}	, {9, 0} 	},
{ {0, 220}	, 6	, 36, {0, 10}	, {8, 0} 	},
{ {49, 226}	, 12, 30, {0, 6}	, {9, 0} 	},
{ {96, 86}	, 21, 9	, {0, -5}	, {19, 0}	}
};

/****************************************************************************
** Class OVGFont
****************************************************************************/
class OVGFont : public PVRShell
{
public:
	OVGFont() {}

	// The two fonts our app contains
	VGFont  m_vgPathFont;
	VGFont  m_vgImageFont;

	// The paint for the fonts
	VGPaint m_vgFontPaint;

	// PrintVG
	CPVRTPrintVG m_PrintVG;

	// The width and height of the screen
	unsigned int m_ui32ScreenWidth;
	unsigned int m_ui32ScreenHeight;

	// PVRShell functions
	bool InitApplication();
	bool QuitApplication();
	bool InitView();
	bool ReleaseView();
	bool RenderScene();

	/****************************************************************************
	** Function Definitions
	****************************************************************************/
	void DrawLine(VGFont &vgFont, VGint i32Size, VGuint *pStr, float fHeight, float fScale);

};

/****************************************************************************
** Demo class Implementation
****************************************************************************/

/****************************************************************************
** InitApplication() is called by PVRShell to enable user to initialise    **
** the application														   **
****************************************************************************/
bool OVGFont::InitApplication()
{
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*) PVRShellGet(prefReadPath));

	// This sets up PVRShell to use an OpenVG context
	PVRShellSet(prefOpenVGContext, true);

	// Set the anti-alias mode to 'better'
	PVRShellSet(prefFSAAMode, 2);

	return true;
}

/****************************************************************************
** QuitApplication() is called by PVRShell to enable user to release      **
** any memory before quitting the program.								   **
****************************************************************************/
bool OVGFont::QuitApplication()
{
	return true;
}

/****************************************************************************
** InitView() is called by PVRShell each time a rendering variable is changed
** in the Shell menu (Z-Buffer On/Off, resolution change, buffering mode...)
** In this function one should initialise all variables that are dependant on
** general rendering variables (screen mode, 3D device, etc...)
****************************************************************************/
bool OVGFont::InitView()
{
	CPVRTPVGObject* pPVGObj;
	VGImage vgImage;
	VGImage vgChild;
	float fW,fH;
	PVRTVECTOR2 fGlyphOrigin, fEscapement;

	// Store the screen width and height
	m_ui32ScreenWidth  = PVRShellGet(prefWidth);
	m_ui32ScreenHeight = PVRShellGet(prefHeight);

	// Set the clear colour
	VGfloat afClearColour[] = { 0.6f, 0.8f, 1.0f, 1.0f };
	vgSetfv(VG_CLEAR_COLOR, 4, afClearColour);

	// Initialise PrintVG for the logo and the title
	m_PrintVG.Initialize(m_ui32ScreenWidth, m_ui32ScreenHeight);

	// Load the font path data from the pvg file
	pPVGObj = CPVRTPVGObject::FromFile(c_szPVGFile);

	if(pPVGObj == NULL)
	{
		PVRShellSet(prefExitMessage, "Error: Failed to load Font.pvg.");
		return false;
	}

	if(pPVGObj->m_i32NumPaths != g_i32ImageCharNo)
	{
		PVRShellSet(prefExitMessage, "Error: Font.pvg doesn't contain the expected amount of characters.");

		delete pPVGObj;
		return false;
	}

	// Load the image based font data
	if(PVRTImageLoadFromPVR(c_szPVRFile, &vgImage) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "Error: Failed to open mask.pvr.");
		return false;
	}

	/*
		Create two fonts. m_vgPathFont will purely contain glyphs represented by paths
		and m_vgImageFont by images. It is also reasonable to have a font that contains
		a mixture but we are keeping them seperate so they can be compared.
	*/

	m_vgImageFont = vgCreateFont(g_i32ImageCharNo);
	m_vgPathFont  = vgCreateFont(pPVGObj->m_i32NumPaths);

	// Add the glyphs to the fonts.
	for(int i = 0; i < g_i32ImageCharNo; ++i)
	{
		// Load glyph from path data

		/*
			Each path in the PVG file represents a glyph. First we need to acquire the origin
			of the glyph so we use vgPathBounds to achieve this as the origin described in
			g_CharDesc is for the image.
		*/
		vgPathBounds(pPVGObj->m_pPaths[i].m_path, &fGlyphOrigin.x, &fGlyphOrigin.y, &fW, &fH);

		// We offset the origin so glyphs like a 'y' are lower
		fGlyphOrigin.x += g_CharDesc[i].fOriginOffset.x;
		fGlyphOrigin.y += g_CharDesc[i].fOriginOffset.y;

		/*
			Add the glyph and assign it a unique ID. The characters we're loading have ASCII codes ranging from
			33 to 128 hence we're giving the glyphs IDs starting at 33. The glyph origin defines the coordinates
			in path space at which to start drawing the glyph and the escapement character is the offset to start
			drawing the next following character.
		*/
		vgSetGlyphToPath(m_vgPathFont, 33 + i, pPVGObj->m_pPaths[i].m_path, VG_TRUE, (VGfloat*) &fGlyphOrigin.x, (VGfloat*) &g_CharDesc[i].fEscapement.x);

		// Load glyph from image data

		/*
			Using a child image we 'cut' the glyph out of the main image (see the child image training course for
			an explanation).
		*/
		vgChild = vgChildImage(vgImage,  g_CharDesc[i].i32Origin[0], g_CharDesc[i].i32Origin[1], g_CharDesc[i].i32Width, g_CharDesc[i].i32Height);

		/*
			We then add the child image to the font. We use the origin offset value directly for the value as the glyph's
			origin is 0,0 within the child image.
		*/
		vgSetGlyphToImage(m_vgImageFont, 33 + i, vgChild, (VGfloat*) &g_CharDesc[i].fOriginOffset.x, (VGfloat*) &g_CharDesc[i].fEscapement.x);

		// Destroy the child image as we no longer need it.
		vgDestroyImage(vgChild);
	}

	// Destroy the image as it is no longer required
	vgDestroyImage(vgImage);

	// Destroy the PVG data as it too is no longer required
	delete pPVGObj;
	pPVGObj = 0;

	// Space

	/*
		Glyphs such as spaces that do not have a visual representation can be added to
		the fonts by passing VG_INVALID_HANDLE instead of a path or image handle.
	*/

	// Set the glyph origin and escapement for the space...
	fGlyphOrigin.x = 0.0f;
	fGlyphOrigin.y = 0.0f;
	// ... We're giving the space a width of 10.0f
	fEscapement.x = 10.0f;
	fEscapement.y = 0.0f;

	vgSetGlyphToImage(m_vgImageFont, 32, VG_INVALID_HANDLE, (VGfloat*) &fGlyphOrigin.x, (VGfloat*) &fEscapement.x);
	vgSetGlyphToPath(m_vgPathFont, 32, VG_INVALID_HANDLE, VG_TRUE, (VGfloat*) &fGlyphOrigin.x, (VGfloat*) &fEscapement.x);

	// Create font paint
	m_vgFontPaint = vgCreatePaint();

	vgSetParameteri(m_vgFontPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetColor(m_vgFontPaint, PVRTRGBA(255,0,0,255));

	return true;
}

/*******************************************************************************
 * Function Name  : ReleaseView
 * Returns        : true if no error occured
 * Description    : Code in ReleaseView() will be called by the Shell before
 *					changing to a new rendering context.
 *******************************************************************************/
bool OVGFont::ReleaseView()
{
	// Cleanup: destroy OpenVG path and paint
	vgDestroyFont(m_vgPathFont);
	m_vgPathFont = 0;

	vgDestroyFont(m_vgImageFont);
	m_vgPathFont = 0;

	vgDestroyPaint(m_vgFontPaint);
	m_vgFontPaint = 0;
	return true;
}

/*******************************************************************************
 * Function Name  : RenderScene
 * Returns		  : true if no error occured
 * Description    : Main rendering loop function of the program. The shell will
 *					call this function every frame.
 *******************************************************************************/
bool OVGFont::RenderScene()
{
	// Clear the screen
	vgClear(0, 0, m_ui32ScreenWidth, m_ui32ScreenHeight);

	// Set the current fill paint...
	vgSetPaint(m_vgFontPaint, VG_FILL_PATH);

	/*
		Set the matrix mode to GLYPH_USER_TO_SURFACE as we want to
		translate and scale the glyphs.
	*/
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_GLYPH_USER_TO_SURFACE);

	/*
		Set the string. The vgDrawGlyphs function used and
		explained in DrawLine excepts an array of unsiged
		ints for an array of glyph IDs which is why we are
		not defining a char array.

		An alternative to vgDrawGlyphs is vgDrawGlyph that
		can be used to display 1 glyph at a time.
	*/

	static VGuint str[] = {'T','e','x','t','y', '!',' '};

	/*
		The DrawLine function repeats the string until the
		line has covered the width of the screen.
	*/

	// Draw the string with the path based font at 4 different scales
	DrawLine(m_vgPathFont, 7, &str[0], m_ui32ScreenHeight * 0.75f, 2.0f);
	DrawLine(m_vgPathFont, 7, &str[0], m_ui32ScreenHeight * 0.65f, 1.0f);
	DrawLine(m_vgPathFont, 7, &str[0], m_ui32ScreenHeight * 0.60f, 0.5f);
	DrawLine(m_vgPathFont, 7, &str[0], m_ui32ScreenHeight * 0.55f, 0.25f);

	/*
		Set the blend mode for the image based glyphs
	*/
	vgSeti(VG_BLEND_MODE, VG_BLEND_SRC_OVER);

	/*
		Set the image mode to DRAW_IMAGE_MULTIPLY so the image based glyphs
		are multiplied by the paint colour and alpha values.
	*/
	vgSeti(VG_IMAGE_MODE, VG_DRAW_IMAGE_MULTIPLY);

	// Draw the string with the image based font at 4 different scales.
	DrawLine(m_vgImageFont, 7, &str[0], m_ui32ScreenHeight * 0.35f, 2.0f);
	DrawLine(m_vgImageFont, 7, &str[0], m_ui32ScreenHeight * 0.25f, 1.0f);
	DrawLine(m_vgImageFont, 7, &str[0], m_ui32ScreenHeight * 0.20f, 0.5f);
	DrawLine(m_vgImageFont, 7, &str[0], m_ui32ScreenHeight * 0.15f, 0.25f);

	// Disable blending
	vgSeti(VG_BLEND_MODE, VG_BLEND_SRC);

	// Display the title and logo.
	m_PrintVG.DisplayDefaultTitle("Font", "", ePVRTPrint3DSDKLogo);

	return true;
}

void OVGFont::DrawLine(VGFont &vgFont, VGint i32Size, VGuint *pStr, float fHeight, float fScale)
{
	static float fOrigin[] = {0.0f, 0.0f};
	static float f = 0.0f;

	/*
		Everytime vgDrawGlyph(s) is called the glyph(s) are drawn at the position
		defined by VG_GLYPH_ORIGIN. This value is updated after each call by the
		escapement vector defined by the glyph.

		As we don't want our text to follow any of the text previously drawn we're
		setting the glyph orign to 0,0.
	*/
	vgSetfv(VG_GLYPH_ORIGIN, 2, fOrigin);

	/*
		Using the matrix mode MATRIX_GLYPH_USER_TO_SURFACE translate and scale
		the font.
	*/
	vgLoadIdentity();
	vgTranslate(0.0f, fHeight);
	vgScale(fScale, fScale);

	/*
		Our string is only a few glyphs long so we're going to repeatedly
		draw it until the x value of the GLYPH_ORIGIN is greater than the
		scaled width.
	*/

	float fGlyphOrigin[2];
	fGlyphOrigin[0] = 0.0f;
	fGlyphOrigin[1] = 0.0f;

	float fScaledWidth = m_ui32ScreenWidth / fScale;

	while(fGlyphOrigin[0] < fScaledWidth)
	{
		/*
			Draw i32Size no of glyphs from pStr. The VG_FILL_PATH parameter
			defines how you would like the glyph (if a path) to be displayed.
			You can also have it stroked by using VG_STROKE_PATH. This parameter
			doesn't affect image based glyphs unless it's value is set to 0
			in which case no glyph (path or image based) will be drawn.

			The 2 parameters being set by variable f are the x and y adjustments
			for each glyph. These can be set to adjust the position of the glyphs
			drawn.

			The final parameter (set to VG_TRUE) disables or enables autohinting.
			If equal to true autohinting may be applied to alter the glyphs slightly
			to improve the render quality.
		*/
		vgDrawGlyphs(vgFont, i32Size, &pStr[0], &f,&f , VG_FILL_PATH, VG_TRUE);

		// Get the updated GLYPH_ORIGIN
		vgGetfv(VG_GLYPH_ORIGIN, 2, &fGlyphOrigin[0]);
	}
}

/*!***************************************************************************
 @Function		NewDemo
 @Return		The demo supplied by the user
 @Description	This function is required by the shell framework, and must
				return a class derived from PVRShell.
*****************************************************************************/
PVRShell* NewDemo()
{
	return new OVGFont;
}

/*****************************************************************************
 End of file (OVGFont.cpp)
*****************************************************************************/
