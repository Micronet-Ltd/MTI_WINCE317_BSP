/******************************************************************************
 * Name			: CPVRTextureHeader.h
 * Author		: PowerVR
 *
 * Created		: April 2006
 *
 *
 * Copyright	: 2005 by Imagination Technologies Limited. All rights reserved.
 *				: No part of this software, either material or conceptual
 *				: may be copied or distributed, transmitted, transcribed,
 *				: stored in a retrieval system or translated into any
 *				: human or computer language in any form by any means,
 *				: electronic, mechanical, manual or other-wise, or
 *				: disclosed to third parties without the express written
 *				: permission of VideoLogic Limited, Unit 8, HomePark
 *				: Industrial Estate, King's Langley, Hertfordshire,
 *				: WD4 8LZ, nU.K.
 *
 * Description	: Class to represent header information of a pvr texture file. Typically associated
 *				  with a CPVRTextureData instance or part of a CPVRTexture instance
 *
 * Platform		: ANSI
 ******************************************************************************/

#ifndef CPVRTEXTUREHEADER_H
#define CPVRTEXTUREHEADER_H

#include <stdio.h>
#include "PVRTexLibGlobals.h"

namespace pvrtexlib
{

class PVR_DLL PVRTextureUtilities;

class PVR_DLL CPVRTextureHeader
{
public:

	/*******************************************************************************
	* Constructor
	* Description		: Blank constructor that allows the creation of a 
	*						valid but blank CPVRHeader
	*******************************************************************************/
	CPVRTextureHeader();


	/*******************************************************************************
	* Constructor
	* In
	:	u32Width		:	width of topmost image in pixels
	:	u32Height		:	height of topmost image
	:	u32MipMapCount	:	count of MIP-map levels
	:	u32NumSurfaces	:	number of surfaces present in texture
	:	bBorder			:	does the texture have an added border (se PVRTC compression documentation)
	:	bTwiddled		:	is the texture twiddled (use morton order)
	:	bCubeMap		:	is the texture a cube map
	:	bVolume			:	is the texture a volume texture (little support right now)
	:	bFalseMips		:	are false-coloured MIP-map levels encoded
	:	ePixelType		:	which pixel format is used for the data
	:	fNormalMap		:	multiplier when used in normal map creation for this texture.
	:						0.0f implies that this is not a normal map texture
	* Description		: Manual constructor that allows the creation of a CPVRHeader
	*******************************************************************************/
	CPVRTextureHeader(const unsigned int u32Width,
		const unsigned int	u32Height,
		const unsigned int	u32MipMapCount,
		const unsigned int	u32NumSurfaces,
		const bool			bBorder,
		const bool			bTwiddled,
		const bool			bCubeMap,
		const bool			bVolume,
		const bool			bFalseMips,
		const bool			bDoAlpha,
		const PixelType		ePixelType,
		const float			fNormalMap);

	/*******************************************************************************
	* Constructor
	* In
	:	fFile			:	open FILE pointer to PVR header data (beginning of a .pvr file)
	* Description		:	Reads a header from the passed FILE pointer leaving this
	*						at the beginning of any data following the header.
	*******************************************************************************/
	CPVRTextureHeader(FILE *const fFile);
	/*******************************************************************************
	* Constructor
	* In
	:	fFile			:	pointer to PVR header data (beginning of a .pvr file)
	* Description		:	Reads a header from the passed pointer.
	*******************************************************************************/
	CPVRTextureHeader(const uint8* const pPVRData);


	/*******************************************************************************
	* Accessor Methods for basic properties
	*******************************************************************************/
	unsigned int	getWidth() const;
	void			setWidth(unsigned int u32Width);
	unsigned int	getHeight() const;
	void			setHeight(unsigned int u32Height);
	void			getDimensions(unsigned int& width, unsigned int& height) const;
	void			setDimensions(const unsigned int u32Width, const unsigned int u32Height);
	unsigned int	getMipMapCount() const;
	void			setMipMapCount(unsigned int u32MipMapCount);
	PixelType		getPixelType() const;
	void			setPixelType(const PixelType ePixelType);
	unsigned int	getNumSurfaces() const;
	void			setNumSurfaces(unsigned int u32NumSurfaces);

	/*******************************************************************************
	* sets to the standard pixel type for the passed prcision mode
	*******************************************************************************/
	void			setPixelType(const EPRECMODE ePrecMode);

	/*******************************************************************************
	* Accessor Methods for preprocessing properties etc.
	*******************************************************************************/
	bool			isBordered() const;
	void			setBorder(bool bBorder);
	bool			isTwiddled() const;
	void			setTwiddled(bool bTwiddled);
	bool			isCubeMap() const;
	void			setCubeMap(bool bCubeMap);
	bool			isVolume() const;
	void			setVolume(const bool bVolume);
	float			getNormalMap() const;
	void			setNormalMap(const float fNormalMap);
	bool			isNormalMap() const;
	bool			hasMips() const;
	bool			hasFalseMips() const;
	void			setFalseMips(const bool bFalseMips);
	bool			hasAlpha() const;
	void			setAlpha(const bool bAlpha);


	/*******************************************************************************
	* Other Accessor Methods
	*******************************************************************************/
	unsigned int	getOriginalVersionNumber();	// only relevant for files loaded from disk
	unsigned int	getCurrentVersionNumber();
	/*******************************************************************************
	* Function Name  : getSurfaceSize
	* Returns        : size of an individual surface (including MIP-map levels)
	*					described by the header in bytes
	*******************************************************************************/
	size_t			getSurfaceSize() const;
	/*******************************************************************************
	* Function Name  : getSurfaceSizeInPixels
	* Returns        : the number of pixels described by a surface in this texture
	*******************************************************************************/
	unsigned int	getSurfaceSizeInPixels() const;
	/*******************************************************************************
	* Function Name  : getTotalTextureSize
	* Returns        : sum of the size of all individual surfaces (including MIP-map
	*					levels) described by the header
	*******************************************************************************/
	size_t			getTotalTextureSize() const;
	/*******************************************************************************
	* Function Name  : getFileHeaderSize
	* Returns        : returns file size from original header version
	*******************************************************************************/
	size_t			getFileHeaderSize();
	/*******************************************************************************
	* Function Name  : getFileHeaderSize
	* Returns        : returns file size from passed header version (if possible)
	*					otherwise PVRTHROWs
	*******************************************************************************/
	size_t			getFileHeaderSize(int u32HeaderVersion);
	/*******************************************************************************
	* Function Name  : getPrecMode
	* Returns        : returns the precision mode of the pixel type of this header
	*******************************************************************************/
	EPRECMODE	getPrecMode() const;
	/*******************************************************************************
	* Function Name  : getBitsPerPixel
	* Returns        : returns the number of bits of data per pixel required by the
	*					pixel type of this header
	*******************************************************************************/
	unsigned int	getBitsPerPixel() const;
	/*******************************************************************************
	* Function Name  : hasSurfaceCompatibleWith
	* Returns        : returns whether this header describes a texture with surfaces
						that are compatible for appending etc with the ones described
						in the passed header.
	*******************************************************************************/
	bool			hasSurfaceCompatibleWith(const CPVRTextureHeader & sHeader);
	/*******************************************************************************
	* Function Name  : writeToFile
	* Returns        : writes this pvr header to the FILE* passed leaving the FILE
						pointer at the end of the header data. Returns the size 
						of data written
	*******************************************************************************/
	size_t			writeToFile(FILE* const fFile)const;
	/*******************************************************************************
	* Function Name  : writeToPointer
	* Returns        : writes this pvr header to the pointer passed. Returns the size
						of the data written
	*******************************************************************************/
	size_t			writeToPointer(uint8 * const pPointer)const;
	/*******************************************************************************
	* Function Name  : writeToIncludeFile
	* Returns        :  writes this pvr header to the FILE* passed leaving the FILE
						pointer at the end of the header data. Writes this as the
						beginning of a c++ compatible header file. Returns the size 
						of data written
	*******************************************************************************/
	size_t			writeToIncludeFile(FILE* const fFile)const;

	const static unsigned int	u32CURRENT_VERSION	= 2;	// currentlt v2 of .pvr files is used

private:
	PVRTextureUtilities			*PVRTU;		// note this is a singleton
	unsigned int				m_u32Height, m_u32Width, m_u32MipMapCount,m_u32NumSurfaces;
	unsigned int				m_u32OriginalVersionNumber;	// the file header version on the disk from which this class was constructed
	bool						m_bTwiddled, m_bCubeMap, m_bVolume, m_bFalseMips, m_bBorder, m_bAlpha;
	PixelType					m_ePixelType;
	float						m_fNormalMap;

	void InitBlank();
	void InitFromPointer(const uint8* const pData);



	// *** CURRENT PVR HEADER V2 ***

	struct PVRTextureHeaderV2{
		unsigned int  dwHeaderSize;		/* size of the structure */
		unsigned int  dwHeight;			/* height of surface to be created */
		unsigned int  dwWidth;			/* width of input surface */
		unsigned int  dwMipMapCount;	/* number of mip-map levels requested */
		unsigned int  dwpfFlags;		/* pixel format flags */
		unsigned int  dwDataSize;		/* Size of the compress data */
		unsigned int  dwBitCount;		/* number of bits per pixel  */
		unsigned int  dwRBitMask;		/* mask for red bit */
		unsigned int  dwGBitMask;		/* mask for green bits */
		unsigned int  dwBBitMask;		/* mask for blue bits */
		unsigned int  dwAlphaBitMask;	/* mask for alpha channel */
		unsigned int  dwPVR;			/* should be 'P' 'V' 'R' '!' */
		unsigned int  dwNumSurfs;		/* number of slices for volume textures or skyboxes */
	};

	const static unsigned int PVRTEX_MIPMAP			= (1<<8);
	const static unsigned int PVRTEX_TWIDDLE		= (1<<9);
	const static unsigned int PVRTEX_BUMPMAP		= (1<<10);
	const static unsigned int PVRTEX_TILING			= (1<<11);
	const static unsigned int PVRTEX_CUBEMAP		= (1<<12);
	const static unsigned int PVRTEX_FALSEMIPCOL	= (1<<13);
	const static unsigned int PVRTEX_VOLUME			= (1<<14);
	const static unsigned int PVRTEX_ALPHA			= (1<<15);

	// legacy
	const static unsigned int PVRTEX_IDENTIFIER		= 0x21525650;	// 'P''V''R''!'
	const static unsigned int PVRTEX_PIXELTYPE		= 0xff;			// pixel type is always in the last 16bits of the flags

	void DoBitMasks(PVRTextureHeaderV2 *phPVR, bool bAlpha) const;

};

}

#endif // CPVRTEXTUREHEADER_H

/*****************************************************************************
 End of file (PVRTexture.h)
*****************************************************************************/
