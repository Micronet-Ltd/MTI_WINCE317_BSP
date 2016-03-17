//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*==========================================================================
 *
 *
 *  File:	lbprintf.h
 *  Content:	list box printf header file
 *
 ***************************************************************************/
#ifndef __LBPRINTF_INCLUDED__

#define __LBPRINTF_INCLUDED__
#ifdef __cplusplus
extern "C" {
#endif
extern void LBCreate( HWND hWnd, DWORD pos );
extern void LBSize( DWORD dwWidth, DWORD dwHeight );
extern void LBClear( void );
extern void __cdecl LBPrintf( LPSTR fmt, ... );
extern void __cdecl LBPrintfDDRC( HRESULT rc, LPSTR fmt, ... );

#ifdef __cplusplus
}
#endif

#endif

