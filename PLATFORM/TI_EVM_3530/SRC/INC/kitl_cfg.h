// Copyright (c) 2007, 2013 Micronet ltd. All rights reserved.
// by Vladimir Zatulovsky

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
//------------------------------------------------------------------------------
//
//  File:  kitl_cfg.h
//
#ifndef __KITL_CFG_H
#define __KITL_CFG_H

//------------------------------------------------------------------------------
OAL_KITL_ETH_DRIVER g_kitlUsbRndis = OAL_ETHDRV_RNDIS;

OAL_KITL_DEVICE g_kitlDevices[] = {
    { 
        L"USBFn RNDIS ", Internal, OMAP_USBHS_REGS_PA, 
        0, OAL_KITL_TYPE_ETH, &g_kitlUsbRndis
    }, {
        NULL, 0, 0, 0, 0, NULL
    }
};
#endif
