/******************************************************************************

 @File         PVRTgles2Ext.h

 @Title        Miscellaneous functions used in 3D rendering.

 @Copyright    Copyright (C) 2007 - 2008 by Imagination Technologies Limited.

 @Platform     Independent

 @Description  OpenGL ES 2.0 extensions

******************************************************************************/
#ifndef _PVRTGLES2EXT_H_
#define _PVRTGLES2EXT_H_

#include <GLES2/gl2extimg.h>

/****************************************************************************
** Build options
****************************************************************************/

#define GL_PVRTGLESEXT_VERSION 2

/**************************************************************************
****************************** GL EXTENSIONS ******************************
**************************************************************************/

class CPVRTgles2Ext
{
public:
	/*!***********************************************************************
	@Function			Init
	@Description		Initialises IMG extensions
	*************************************************************************/
	void Init();

	/*!***********************************************************************
	@Function			IsGLExtensionSupported
	@Input				extension extension to query for
	@Returns			True if the extension is supported
	@Description		Queries for support of an extension
	*************************************************************************/
	static bool IsGLExtensionSupported(const char *extension);
};

#endif /* _PVRTGLES2EXT_H_ */

/*****************************************************************************
 End of file (PVRTgles2Ext.h)
*****************************************************************************/
