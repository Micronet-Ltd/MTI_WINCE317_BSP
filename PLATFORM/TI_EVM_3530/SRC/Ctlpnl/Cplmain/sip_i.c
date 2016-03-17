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
/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 3.00.44 */
/* at Sat Apr 19 23:35:02 1997
 */
/* Compiler settings for .\sip.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

const IID LIBID_QWERTYIM = {0x42429668,0xae04,0x11d0,{0xa4,0xf8,0x00,0xaa,0x00,0xa7,0x49,0xb9}};


const IID IID_IIMCallback = {0x42429669,0xae04,0x11d0,{0xa4,0xf8,0x00,0xaa,0x00,0xa7,0x49,0xb9}};


const IID IID_IInputMethod = {0x42429666,0xae04,0x11d0,{0xa4,0xf8,0x00,0xaa,0x00,0xa7,0x49,0xb9}};


const CLSID CLSID_CMSQwertyIm = {0x42429667,0xae04,0x11d0,{0xa4,0xf8,0x00,0xaa,0x00,0xa7,0x49,0xb9}};


#ifdef __cplusplus
}
#endif

