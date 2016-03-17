//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//
//  Debugging support.
//

#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap35xx.h>

#ifndef SHIP_BUILD

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
DBGPARAM dpCurSettings =
{
    L"WAVEDEV",
        {
        L"HWBridge", L"IOCTL",   L"Verbose",     L"Irq",
        L"WODM",     L"WIDM",    L"PDD",         L"MDD",
        L"Mixer",    L"Ril",     L"Unused",      L"Modem",
        L"Power",    L"Function",L"Warning",     L"Error"
        },
    0xC000
};

#endif