/******************************************************************************
* Name			: PVRTextureUtilities.h
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
* Description	: Texture processing utility class. Employs a singleton structure.
*
* Platform		: ANSI
******************************************************************************/


#ifndef PVRTEXLIB_H
#define PVRTEXLIB_H

/*****************************************************************************
* Includes
*****************************************************************************/
#include "PVRTexLibGlobals.h"
#include "singleton.h"
#include "CPVRTextureData.h"
#include "CPVRTextureHeader.h"
#include "CPVRTexture.h"

namespace pvrtexlib
{
	class PVR_DLL PVRTextureUtilities : public singleton<PVRTextureUtilities>
	{

	public:
		/*******************************************************************************
		* Constructor - Don't use explicitly. To access this class use  
		*	PVRTextureUtilities::ptr() functions instead.
		*******************************************************************************/
		PVRTextureUtilities();

		/*******************************************************************************
		* Destructor
		*******************************************************************************/
		~PVRTextureUtilities();
		/*******************************************************************************
		* Function Name			: CompressPVR
		* In/Outputs		
		: sCompressedTexture	: Output CPVRTexture 
		: sDecompressedTexture	: Input CPVRTexture needs to be in a standard format
		: nMode					: Parameter value for specific image compressor - eg ETC
		* Description			: Takes a CPVRTexture in one of the standard formats
		*						: and compresses to the pixel type specified in the destination
		*						: PVRTexture. nMode specifies the quality mode.
		*******************************************************************************/
		void CompressPVR(	CPVRTexture& sDecompressedTexture,
			CPVRTexture& sCompressedTexture, const int nMode=0);
		/*******************************************************************************
		* Function Name			: CompressPVR
		* In/Outputs		
		: sDecompressedHeader	: Input CPVRTexture needs to be in a standard format
		: sDecompressedData		: Input CPVRTexture needs to be in a standard format
		: sCompressedHeader		: Output CPVRTextureHeader with output format set
		: sCompressedData		: Output CPVRTextureData
		: nMode					: Parameter value for specific image compressor - i.e. ETC
		* Description			: Takes a CPVRTextureHeader/CPVRTextureData pair in one of the
		*						: standard formats
		*						: and compresses to the pixel type specified in the destination
		*						: CPVRTextureHeader, the data goes in the destination CPVRTextureData.
		*						: nMode specifies the quality mode.
		*******************************************************************************/
		void CompressPVR(	CPVRTextureHeader			&sDecompressedHeader,
			CPVRTextureData				&sDecompressedData,
			CPVRTextureHeader			&sCompHeader,
			CPVRTextureData				&sCompData,
			const int					nMode);

		/*******************************************************************************
		* Function Name			: DecompressPVR
		* In/Outputs		
		: sCompressedTexture	: Input CPVRTexture 
		: sDecompressedTexture	: Output CPVRTexture will be in a standard format
		* Description			: Takes a CPVRTexture and decompresses it into a
		*						: standard format.
		*******************************************************************************/
		void DecompressPVR(CPVRTexture& sCompressedTexture,
			CPVRTexture& sDecompressedTexture);
		/*******************************************************************************
		* Function Name			: DecompressPVR
		* In/Outputs		
		: sCompressedHeader		: Input CPVRTextureHeader 
		: sCompressedData		: Input CPVRTextureData 
		: sDecompressedHeader	: Output CPVRTextureHeader will be in a standard format
		: sDecompressedData		: Output CPVRTextureData will be in a standard format
		* Description			: Takes a CPVRTextureHeader/Data pair and decompresses it into a
		*						: standard format.
		*******************************************************************************/
		void DecompressPVR(	const CPVRTextureHeader		& sCompressedHeader,
			const CPVRTextureData		& sCompressedData,
			CPVRTextureHeader			& sDecompressedHeader,
			CPVRTextureData				& sDecompressedData);

		/*******************************************************************************
		* Function Name			: ProcessRawPVR
		* In/Outputs		
		: sInputTexture			: Input CPVRTexture needs to be in a standard format
		: sOutputTexture		: Output CPVRTexture will be in a standard format (not necessarily the same)
		* Description			: Takes a CPVRTexture and processes it according to the differences in the passed
		*						:	output CPVRTexture and the passed parameters. Requires the input texture
		*						:	to be in a standard format.
		*******************************************************************************/
		bool ProcessRawPVR(	CPVRTexture&		sInputTexture,
			CPVRTexture&		sOutputTexture,
			const bool				bDoBleeding=false,
			const float				fBleedRed=0.0f,
			const float				fBleedGreen=0.0f,
			const float				fBleedBlue=0.0f,
			PVR_RESIZE				eResizeMode=eRESIZE_BICUBIC );
		/*******************************************************************************
		* Function Name			: ProcessRawPVR
		* In/Outputs		
		: sInputTexture			: Input CPVRTexture needs to be in a standard format.
		: sOutputTexture		: Output CPVRTexture 
		* Description			: Takes a CPVRTexture and decompresses it into one of the standard
		*						: data formats.
		*******************************************************************************/
		bool ProcessRawPVR(	CPVRTextureHeader&		sInputHeader,
			CPVRTextureData&		sInputData,
			CPVRTextureHeader&		sOutputHeader,
			const bool				bDoBleeding=false,
			const float				fBleedRed=0.0f,
			const float				fBleedGreen=0.0f,
			const float				fBleedBlue=0.0f,
			PVR_RESIZE				eResizeMode=eRESIZE_BICUBIC );

	};


}


#endif
/*****************************************************************************
End of file (pvr_utils.h)
*****************************************************************************/
