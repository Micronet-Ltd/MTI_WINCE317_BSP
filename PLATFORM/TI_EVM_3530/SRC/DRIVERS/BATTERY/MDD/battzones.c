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
#include <battimpl.h>

#ifdef DEBUG

#define DEBUGMASK(bit)      (1 << (bit))

#define MASK_ERROR          DEBUGMASK(0)
#define MASK_WARN           DEBUGMASK(1)
#define MASK_INIT           DEBUGMASK(2)
#define MASK_BATTERY        DEBUGMASK(3)
#define MASK_IOCTL          DEBUGMASK(4)
#define MASK_HARDWARE       DEBUGMASK(5)
#define MASK_RESUME         DEBUGMASK(6)
#define MASK_FUNCTION       DEBUGMASK(7)
#define MASK_PDD            DEBUGMASK(8)

DBGPARAM dpCurSettings = {
    _T("BATTDRVR"), 
    {
        _T("Errors"), _T("Warnings"), _T("Init"), _T("Battery"), 
        _T("Ioctl"), _T("Hardware"), _T("Resume"), _T("Function"),
        _T("PDD"),_T(""),_T(""),_T(""),
        _T(""),_T(""),_T(""),_T("") 
    },
    MASK_ERROR | MASK_WARN
}; 

#endif
