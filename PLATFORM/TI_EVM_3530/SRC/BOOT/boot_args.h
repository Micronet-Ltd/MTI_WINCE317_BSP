// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File:  args.h
//
//  This header file defines device structures and constant related to boot
//  configuration. BOOT_CFG structure defines layout of persistent device
//  information. It is used to control boot process.
//
#ifndef __BOOT_ARGS_H
#define __BOOT_ARGS_H

//------------------------------------------------------------------------------

#include <oal_args.h>
#include <oal_kitl.h>

//------------------------------------------------------------------------------

#define BOOT_CFG_SIGNATURE      'BCFG'
#define BOOT_CFG_VERSION        317

#define BOOT_CFG_OAL_FLAGS_RETAILMSG_ENABLE     (1 << 0)

// BOOT_SDCARD_TYPE must not overlap with OAL_KITL_TYPE enum values
#define BOOT_SDCARD_TYPE		(OAL_KITL_TYPE_FLASH+1)

typedef struct {
    UINT32 signature;                   // Structure signature
    UINT32 version;                     // Structure version

    DEVICE_LOCATION bootDevLoc;         // Boot device
    DEVICE_LOCATION kitlDevLoc;
    
    UINT32 kitlFlags;                   // Debug/KITL mode
    UINT32 ipAddress;
    UINT32 ipMask;
    UINT32 ipRoute;    

    UINT32 deviceID;                    // Unique ID for development platform
	
	UINT32 osPartitionSize;				// Space to reserve for OS partition in NAND
	
	WCHAR filename[13];					// Space to reserve for SDCard filename (8.3 format)
	UINT32 launch_manutool;
	UINT32	SwUpdateFlags;				//inform that sw update was started
	UINT8	RndisMacAddress[6];
	UINT16	EthMacAddress[3];
    UINT32 oalFlags;
	UINT16	WiFiMacAddress[3];
	UINT16  CdsParams[8];   //CPU Dynamic Speed Feature state and temperature thresholds/ Maximal frequency allowed by CDS in MHz
} BOOT_CFG;

//------------------------------------------------------------------------------

#endif
