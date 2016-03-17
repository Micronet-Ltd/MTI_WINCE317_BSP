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
 *  File:       ddutil.cpp
 *  Content:    Routines for loading bitmap and palettes from resources
 *
 ***************************************************************************/

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif	/* __cplusplus */

extern IDirectDrawPalette * DDLoadPalette2(IDirectDraw2 *pdd, LPCSTR szBitmap);
extern IDirectDrawSurface3 * DDLoadBitmap2(IDirectDraw2 *pdd, LPCSTR szBitmap, int dx, int dy);
extern HRESULT              DDReLoadBitmap2(IDirectDrawSurface3 *pdds, LPCSTR szBitmap);
extern HRESULT              DDCopyBitmap2(IDirectDrawSurface3 *pdds, HBITMAP hbm, int x, int y, int dx, int dy);
extern DWORD                DDColorMatch2(IDirectDrawSurface3 *pdds, COLORREF rgb);
extern HRESULT              DDSetColorKey2(IDirectDrawSurface3 *pdds, COLORREF rgb);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

