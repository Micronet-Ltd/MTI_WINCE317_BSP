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
#ifndef __BASE64_H__
#define __BASE64_H__

#ifdef __cplusplus
extern "C" {
#endif


#ifdef UNICODE
#define Base64Decode  Base64DecodeW
#else
#define Base64Decode  Base64DecodeA
#endif // !UNICODE

DWORD			// ERROR_*
Base64DecodeA(
    IN CHAR const *pchIn,
    IN DWORD cchIn,
    OUT BYTE *pbOut,
    OUT DWORD *pcbOut);

DWORD			// ERROR_*
Base64DecodeW(
    IN WCHAR const *pchIn,
    IN DWORD cchIn,
    OUT BYTE *pbOut,
    OUT DWORD *pcbOut);

    
#ifdef UNICODE
#define Base64Encode  Base64EncodeW
#else
#define Base64Encode  Base64EncodeA
#endif // !UNICODE

DWORD			// ERROR_*
Base64EncodeA(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    OUT CHAR *pchOut,
    OUT DWORD *pcchOut);

DWORD			// ERROR_*
Base64EncodeW(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    OUT WCHAR *pchOut,
    OUT DWORD *pcchOut);

    
#ifdef __cplusplus
}       // Balance extern "C" above
#endif

#endif // BASE64

