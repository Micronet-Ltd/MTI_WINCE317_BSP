/**************************************************************************
 * Name         : eglplatform.h
 *
 * Copyright    : 2005-8 by Imagination Technologies Limited. All rights reserved.
 *              : No part of this software, either material or conceptual
 *              : may be copied or distributed, transmitted, transcribed,
 *              : stored in a retrieval system or translated into any
 *              : human or computer language in any form by any means,
 *              : electronic, mechanical, manual or other-wise, or
 *              : disclosed to third parties without the express written
 *              : permission of Imagination Technologies Limited, Unit 8, HomePark
 *              : Industrial Estate, King's Langley, Hertfordshire,
 *              : WD4 8LZ, U.K.
 *
 * Platform     : ANSI
 *
 * $Date: 2008/05/13 15:52:52 $ $Revision: 1.1 $
 **************************************************************************/
#ifndef _egltypes_h_
#define _egltypes_h_

#if defined(_WIN32) || defined(__VC32__)             /* Win32 */
#   if defined (_DLL_EXPORTS)
#       define EGLAPI __declspec(dllexport)
#   else
#       define EGLAPI __declspec(dllimport)
#   endif
#elif defined (__SYMBIAN32__) /* Symbian GCC */
#   if defined (__GCC32__)
#       define EGLAPI __declspec(dllexport)
#   else
#       define EGLAPI IMPORT_C
#   endif
#elif defined (__ARMCC_VERSION)                      /* ADS */
#   define EGLAPI
#elif defined (__GNUC__)                             /* GCC dependencies (kludge) */
#   define EGLAPI
#elif defined (_UITRON_)
#	define EGLAPI
#endif

#if !defined (EGLAPI)
#   error Unsupported platform!
#endif

#ifndef EGLAPIENTRY
#define EGLAPIENTRY
#endif

#ifndef EGL_APIENTRY
#define EGL_APIENTRY EGLAPIENTRY
#endif

#if defined __linux__
	#include <sys/types.h>
	#if defined(SUPPORT_X11)
		#include <X11/Xlib.h>
		typedef Display*	EGLNativeDisplayType;
		typedef Window		EGLNativeWindowType;
		typedef Pixmap		EGLNativePixmapType;
	#else
		typedef int		EGLNativeDisplayType;
		typedef void*	EGLNativeWindowType;
		typedef void*	EGLNativePixmapType;
	#endif
#elif defined(UNDER_CE) || defined(_WIN32)
	typedef int int32_t;
	#undef UNREFERENCED_PARAMETER
	#include <windows.h>
	typedef HDC		EGLNativeDisplayType;
	typedef HWND	EGLNativeWindowType;
	typedef void*	EGLNativePixmapType;
#elif defined(__SYMBIAN32__)
	#include <e32def.h>

#   ifndef int32_t
        typedef int int32_t;
#   endif

	typedef TInt EGLNativeDisplayType;
	/*
		Declare these as void although they points to classes - we can't
		include	a C++ header file as the EGL files are all written in C.
	*/
	#define EGLNativeWindowType void* /* Is really an RWindow* */
	#define EGLNativePixmapType void* /* Is really a CFbsBitmap* */
#elif defined(_UITRON_)
	typedef int int32_t;
	typedef int		EGLNativeDisplayType;
	typedef void*	EGLNativeWindowType;
	typedef void*	EGLNativePixmapType;
#else
	typedef int		EGLNativeDisplayType;
	typedef void*	EGLNativeWindowType;
	typedef void*	EGLNativePixmapType;
#endif

/* EGL 1.2 types, renamed for consistency in EGL 1.3 */
typedef EGLNativeDisplayType NativeDisplayType;
typedef EGLNativePixmapType NativePixmapType;
typedef EGLNativeWindowType NativeWindowType;

#endif /* _egltypes_h_ */
