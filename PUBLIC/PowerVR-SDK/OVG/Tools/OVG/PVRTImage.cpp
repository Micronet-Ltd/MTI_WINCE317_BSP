/******************************************************************************

 @File         PVRTImage.cpp

 @Title        

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     ANSI compatible

 @Description  Image loading.

******************************************************************************/
#include <string.h>
#include <stdlib.h>

#include "PVRTTexture.h"
#include "PVRTImage.h"
#include "PVRTResourceFile.h"

/*!***************************************************************************
 @Function		PVRTImageLoadFromPointer
 @Input			pPointer			Pointer to header-texture's structure
 @Modified		vgImageHandle		An OpenVG image handle
 @Input			eImageQuality		The quality of the image to be created
 @Input			eWantedImageFormat	The format of the image to be created.
 @Return		PVR_SUCCESS on success
 @Description	Allows images to be stored in C header files and loaded in.
*****************************************************************************/
EPVRTError PVRTImageLoadFromPointer(const void * pPointer,
									VGImage * const vgImageHandle,
									VGImageQuality eImageQuality,
									VGImageFormat eWantedImageFormat)
{
	PVR_Texture_Header* psPVRHeader = (PVR_Texture_Header*) pPointer;
	*vgImageHandle = 0;

	// Only accept untwiddled data
	if((psPVRHeader->dwpfFlags & PVRTEX_TWIDDLE) == PVRTEX_TWIDDLE)
	{
		PVRTErrorOutputDebug("PVRTImageLoadPartialFromPointer failed: Texture should be untwiddled.\n");
		return PVR_FAIL;
	}

	VGImageFormat eImageFormat = (VGImageFormat) 0;

	switch(psPVRHeader->dwpfFlags & PVRTEX_PIXELTYPE)
	{
		case ePT_VG_sRGBX_8888:		eImageFormat = VG_sRGBX_8888;		break;
		case ePT_VG_sRGBA_8888:		eImageFormat = VG_sRGBA_8888;		break;
		case ePT_VG_sRGBA_8888_PRE: eImageFormat = VG_sRGBA_8888_PRE;	break;
		case ePT_VG_sRGB_565:		eImageFormat = VG_sRGB_565;			break;
		case ePT_VG_sRGBA_5551:		eImageFormat = VG_sRGBA_5551;		break;
		case ePT_VG_sRGBA_4444:		eImageFormat = VG_sRGBA_4444;		break;
		case ePT_VG_sL_8:			eImageFormat = VG_sL_8;				break;
		case ePT_VG_lRGBX_8888:		eImageFormat = VG_lRGBX_8888;		break;
		case ePT_VG_lRGBA_8888:		eImageFormat = VG_lRGBA_8888;		break;
		case ePT_VG_lRGBA_8888_PRE: eImageFormat = VG_lRGBA_8888_PRE;	break;
		case ePT_VG_lL_8:			eImageFormat = VG_lL_8;				break;
		case ePT_VG_A_8:			eImageFormat = VG_A_8;				break;
		case ePT_VG_BW_1:			eImageFormat = VG_BW_1;				break;

		case ePT_VG_sXRGB_8888:		eImageFormat = VG_sXRGB_8888;		break;
		case ePT_VG_sARGB_8888:		eImageFormat = VG_sARGB_8888;		break;
		case ePT_VG_sARGB_8888_PRE: eImageFormat = VG_sARGB_8888_PRE;	break;
		case ePT_VG_sARGB_1555:		eImageFormat = VG_sARGB_1555;		break;
		case ePT_VG_sARGB_4444:		eImageFormat = VG_sARGB_4444;		break;
		case ePT_VG_lXRGB_8888:		eImageFormat = VG_lXRGB_8888;		break;
		case ePT_VG_lARGB_8888:		eImageFormat = VG_lARGB_8888;		break;
		case ePT_VG_lARGB_8888_PRE: eImageFormat = VG_lARGB_8888_PRE;	break;

		case ePT_VG_sBGRX_8888:		eImageFormat = VG_sBGRX_8888;		break;
		case ePT_VG_sBGRA_8888:		eImageFormat = VG_sBGRA_8888;		break;
		case ePT_VG_sBGRA_8888_PRE: eImageFormat = VG_sBGRA_8888_PRE;	break;
		case ePT_VG_sBGR_565:		eImageFormat = VG_sBGR_565;			break;
		case ePT_VG_sBGRA_5551:		eImageFormat = VG_sBGRA_5551;		break;
		case ePT_VG_sBGRA_4444:		eImageFormat = VG_sBGRA_4444;		break;
		case ePT_VG_lBGRX_8888:		eImageFormat = VG_lBGRX_8888;		break;
		case ePT_VG_lBGRA_8888:		eImageFormat = VG_lBGRA_8888;		break;
		case ePT_VG_lBGRA_8888_PRE: eImageFormat = VG_lBGRA_8888_PRE;	break;

		case ePT_VG_sXBGR_8888:		eImageFormat = VG_sXBGR_8888;		break;
		case ePT_VG_sABGR_8888:		eImageFormat = VG_sABGR_8888;		break;
		case ePT_VG_sABGR_8888_PRE: eImageFormat = VG_sABGR_8888_PRE;	break;
		case ePT_VG_sABGR_1555:		eImageFormat = VG_sABGR_1555;		break;
		case ePT_VG_sABGR_4444:		eImageFormat = VG_sABGR_4444;		break;
		case ePT_VG_lXBGR_8888:		eImageFormat = VG_lXBGR_8888;		break;
		case ePT_VG_lABGR_8888:		eImageFormat = VG_lABGR_8888;		break;
		case ePT_VG_lABGR_8888_PRE: eImageFormat = VG_lABGR_8888_PRE;	break;

		default: // NOT SUPPORTED
			PVRTErrorOutputDebug("PVRTImageLoadPartialFromPointer failed: pixel type not supported.\n");
			return PVR_FAIL;
	}

	unsigned int ui32Width = psPVRHeader->dwWidth;
	unsigned int ui32Height= psPVRHeader->dwHeight;
	unsigned int ui32Stride= (unsigned int) ((psPVRHeader->dwBitCount * 0.125f) * ui32Width);
	VGubyte* pData = (VGubyte*) pPointer + psPVRHeader->dwHeaderSize;

	//Create the VGImage.
	*vgImageHandle = vgCreateImage(eWantedImageFormat != -1 ? eWantedImageFormat : eImageFormat, ui32Width, ui32Height, eImageQuality);

	if(!(*vgImageHandle))
	{
		PVRTErrorOutputDebug("PVRTImageLoadPartialFromPointer failed: Failed to create image.\n");
		return PVR_FAIL;
	}

	vgImageSubData(*vgImageHandle, pData, ui32Stride, eImageFormat, 0, 0, ui32Width, ui32Height);

	return PVR_SUCCESS;
}

/*!***************************************************************************
 @Function		PVRTImageLoadFromPVR
 @Input			pFilename			Filename of the .PVR file to load the image from
 @Modified		vgImageHandle		An OpenVG image handle
 @Input			eImageQuality		The quality of the image to be created
 @Input			eWantedImageFormat	The format of the image to be created.
 @Return		PVR_SUCCESS on success
 @Description	Allows images to be stored in binary PVR files and loaded in.
*****************************************************************************/
EPVRTError PVRTImageLoadFromPVR(const char * const pFilename,
								VGImage * const vgImageHandle,
								VGImageQuality eImageQuality,
								VGImageFormat eWantedImageFormat)
{
	CPVRTResourceFile TexFile(pFilename);

	if (!TexFile.IsOpen()) 
		return PVR_FAIL;

	return PVRTImageLoadFromPointer(TexFile.DataPtr(), vgImageHandle, eImageQuality, eWantedImageFormat);
}


/*****************************************************************************
End of file (PVRTImage.cpp)
*****************************************************************************/
