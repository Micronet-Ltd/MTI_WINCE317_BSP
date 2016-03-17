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
//RS #include <ceddkex.h>
//RS #include <omap35xx.h>
#include "usbaudio.hpp"

#ifndef SHIP_BUILD

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
DBGPARAM dpCurSettings =
{
    L"AUDIOUSBCLASS",
        {
        L"ZONE_ERR", L"ZONE_WARN",   L"ZONE_INIT",     L"ZONE_TRACE",
        L"ZONE_AUD_INIT",     L"ZONE_AUD_READ",    L"ZONE_AUD_WRITE",         L"ZONE_AUD_IOCTL",
        L"ZONE_USB_PARSE",    L"ZONE_USB_INIT",     L"ZONE_USB_CONTROL",      L"ZONE_USB_ISO",
        L"ZONE_USBCLIENT"
        },
    0xC000//0xFFFF//0xC000
};

#endif //~SHIP_BUILD