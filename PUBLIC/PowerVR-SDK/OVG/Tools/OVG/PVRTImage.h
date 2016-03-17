/******************************************************************************

 @File         PVRTImage.h

 @Title        

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     ANSI compatible

 @Description  Image loading.

******************************************************************************/
#ifndef _PVRTIMAGE_H_
#define _PVRTIMAGE_H_

#include "PVRTContext.h"
#include "../PVRTGlobal.h"
#include "../PVRTTexture.h"
#include "../PVRTError.h"

/*!***************************************************************************
 @Function		PVRTImageLoadFromPointer
 @Input			pPointer			Pointer to header-texture's structure
 @Modified		vgImageHandle		An OpenVG image handle
 @Input			eImageQuality		The quality of the image to be created
 @Input			eWantedImageFormat	The format of the image to be created.
 @Return		PVR_SUCCESS on success
 @Description	Allows images to be stored in C header files and loaded in.
*****************************************************************************/
EPVRTError PVRTImageLoadFromPointer(const void* pPointer,
									VGImage * const vgImageHandle, 
									VGImageQuality eImageQuality = VG_IMAGE_QUALITY_NONANTIALIASED,
									VGImageFormat eWantedImageFormat = (VGImageFormat) -1);

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
								VGImageQuality eImageQuality = VG_IMAGE_QUALITY_NONANTIALIASED,
								VGImageFormat eWantedImageFormat = (VGImageFormat) -1);

#endif /* _PVRTIMAGE_H_ */


/*****************************************************************************
 End of file (PVRTImage.h)
*****************************************************************************/
