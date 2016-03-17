// All rights reserved ADENEO EMBEDDED 2010
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
#ifndef OMAP3530_MUSBOTG_H
#define OMAP3530_MUSBOTG_H
#include <omap_musbcore.h>
#include <omap3530_musbcore.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OTG_DRIVER (TEXT("omap_musbotg.dll"))
    
#define IDLE_MODE       0
#define DEVICE_MODE     1
#define HOST_MODE       2

#define A_DEVICE        0
#define B_DEVICE        1

#define CONN_CCS        (1<<0)
#define CONN_CSC        (1<<1)
#define CONN_DC         (1<<2)

#define DEFAULT_FIFO_ENDPOINT_0_SIZE    64 // this is according to MUSBMHDRC product spec, pg 13
#define MAX_DMA_CHANNEL 8

#ifdef __cplusplus
}
#endif

#endif

