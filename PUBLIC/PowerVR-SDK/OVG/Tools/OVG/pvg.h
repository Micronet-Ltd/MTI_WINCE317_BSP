/******************************************************************************

 @File         pvg.h

 @Title        

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     ANSI Independant

 @Description  PVG format is an OpenVG friendly format optimised for PowerVR
               platforms. It is exported from Illustrator using the PVRVecEx
               plug-in distributed with the PowerVR OpenVG SDks.

******************************************************************************/
typedef struct _PVG_FILE_HEADER
{
	unsigned int dwMagicToken;  // Has to be 'POVG'
	unsigned int dwSizeOfHeader;
	unsigned int dwVersion;		// Mayor(short), Minor(short)
	unsigned int dwBuild;		// Branch(short), Build(short)
	unsigned int dwFileFlags;
	unsigned int dwNumPaths;
	unsigned int dwNumPaints;
	unsigned int dwNumRamps;
	unsigned int dwNumDashes;
	unsigned int dwNumPatterns;
	float		 fLeft, fTop, fRight, fBottom; // drawing extensions
} PVG_FILE_HEADER;

/*!****************************************************************************
 @Struct      _PVG_PATH_HEADER
 @Brief       The PVG path header
******************************************************************************/
typedef struct _PVG_PATH_HEADER
{
	unsigned int	dwPathType;		// VG_FILL_PATH and/or VG_STROKE_PATH
	unsigned int	dwClipping;		// Clipping path = 1 otherwise = 0
	unsigned int	dwAlphaBlend;	// 0 if no alpha, otherwise 1-255 (opacity: fully translucent (1) to fully opaque (255))
	unsigned int	StrokePaint;	// StrokePaint definition in file
	unsigned int	FillPaint;		// Fill definition in file
	unsigned int	StrokeDash;		// 0 if none, otherwise Dash definition in file
	unsigned int	dwFillRule;		// VG_EVEN_ODD  or VG_NON_ZERO
	unsigned int	dwStrokeJoin;	// VG_JOIN_MITER, VG_JOIN_ROUND or VG_JOIN_BEVEL
	unsigned int	dwStrokeCap;		// VG_CAP_BUTT, VG_CAP_ROUND or VG_CAP_SQUARE
	float			fStrokeWidth;
	float			fStrokeMiterLimit;
	unsigned int	dwNumSegments;
	unsigned int	dwPathDataSize;
} PVG_PATH_HEADER;

/*!****************************************************************************
 @Struct      _PVG_PAINT_HEADER
 @Brief       The PVG paint header
******************************************************************************/
typedef struct _PVG_PAINT_HEADER
{
	unsigned int dwPaint; // (Paint Identifier)
	unsigned int dwPaintStyle;
	float fPaintParameters[5];
	unsigned int dwRamp; // (ignored if not gradient, position in file of Ramp data)
	unsigned int dwPattern; // (ignored if not pattern, position in file of pattern data)
} PVG_PAINT_HEADER;

/*!****************************************************************************
 @Struct      _PVG_COLOR_RAMP_HEADER
 @Brief       The PVG color ramp header
******************************************************************************/
typedef struct _PVG_COLOR_RAMP_HEADER
{
	unsigned int dwNumRampValues;
	// Ramp values will be written after this header as 4*dwNumRampValues floats (STOP, R, G, B)
} PVG_COLOR_RAMP_HEADER;

/*!****************************************************************************
 @Struct      _PVG_DASH_HEADER
 @Brief       The PVG dash header
******************************************************************************/
typedef struct _PVG_DASH_HEADER
{
	unsigned int	dwNumDashes;
	float			fPhase;
	float			fDashValues[6];
} PVG_DASH_HEADER;

/*!****************************************************************************
 @Struct      _PVG_PATTERN_HEADER
 @Brief       The PVG pattern header
******************************************************************************/
typedef struct _PVG_PATTERN_HEADER
{
	unsigned int	dwWidth;
	unsigned int	dwHeight;
	// Pattern texture values to be written after this header in 16-bit 565RGB format
} PVG_PATTERN_HEADER;
