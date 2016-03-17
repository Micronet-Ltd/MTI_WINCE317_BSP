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
//  Header:  eboot.h
//
//  This header file is comprised of component header files that defines
//  the standard include hierarchy for the bootloader. It also defines few
//  trivial constant.
//
#ifndef __EBOOT_H
#define __EBOOT_H

//------------------------------------------------------------------------------

#include <windows.h>
#include <ceddk.h>
#include <blcommon.h>
#include <nkintr.h>
#include <halether.h>
#include <fmd.h>
#include <bootpart.h>

#include <oal.h>
#include <oal_blmenu.h>

#include <omap35xx.h>

#include <bsp.h>
#include <bsp_base_regs.h>
#include <boot_args.h>
#include <bsp_version.h>
#include <bsp_keypad.h>

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// The following constants are used to distinguish between ES2.0 and ES2.1 CPU 
// revisions.  This mechanism is necessary because the ID fuses are incorrectly
// set to ES2.0 in some ES2.1 devices.
#define PUBLIC_ROM_CRC_PA		0x14020
#define PUBLIC_ROM_CRC_ES2_0	0x5a540331
#define PUBLIC_ROM_CRC_ES2_1	0x6880d8d6
//------------------------------------------------------------------------------

typedef struct {
    UINT32 start;
    UINT32 length;
    UINT32 base;
} REGION_INFO_EX;

//------------------------------------------------------------------------------

#define DOWNLOAD_TYPE_UNKNOWN       0
#define DOWNLOAD_TYPE_RAM           1
#define DOWNLOAD_TYPE_BINDIO        2
#define DOWNLOAD_TYPE_XLDR          3
#define DOWNLOAD_TYPE_EBOOT         4
#define DOWNLOAD_TYPE_IPL           5
#define DOWNLOAD_TYPE_FLASHRAM      6
#define DOWNLOAD_TYPE_BMP			7

//------------------------------------------------------------------------------
#define BL_DOWNLOAD_BMP				(BL_DOWNLOAD | 0x00010000)
//--------------------------------------------
typedef struct {
    OAL_KITL_TYPE bootDeviceType;
    UINT32 type;
    UINT32 numRegions;
    UINT32 launchAddress;
    REGION_INFO_EX region[BL_MAX_BIN_REGIONS];

    UINT32 recordOffset;
    UINT8  *pReadBuffer;
    UINT32 readSize;
} EBOOT_CONTEXT;

//------------------------------------------------------------------------------

extern BOOT_CFG g_bootCfg;
extern EBOOT_CONTEXT g_eboot;
extern OAL_KITL_DEVICE g_bootDevices[];
extern OAL_KITL_DEVICE g_kitlDevices[];

extern UINT32   g_ulFlashBase;

//------------------------------------------------------------------------------

VOID OEMMultiBinNotify(MultiBINInfo *pInfo);

//------------------------------------------------------------------------------

VOID   BLMenu(BOOL bForced);    
BOOL   BLReadBootCfg(BOOT_CFG *pBootCfg);
BOOL   BLWriteBootCfg(BOOT_CFG *pBootCfg);
BOOL   BLReserveBootBlocks();
BOOL   BLConfigureFlashPartitions(BOOL bForceEnable);
BOOL   BLShowLogo();
BOOL   BLShowProgress();
BOOL   BLSetProgressData(DWORD noOfRectangles,DWORD stepsNo);
UINT32 BLEthDownload(BOOT_CFG *pBootCfg, OAL_KITL_DEVICE *pBootDevices);
UINT32 BLEthStartLDR( BOOT_CFG *pConfig, OAL_KITL_DEVICE *pBootDevices);
BOOL   BLEthReadData(ULONG size, UCHAR *pData);
VOID   BLEthConfig(BSP_ARGS *pArgs);
UINT32 BLSDCardDownload(BOOT_CFG *pBootCfg, OAL_KITL_DEVICE *pBootDevices);
BOOL   BLSDCardReadData(ULONG size, UCHAR *pData);
VOID   BLSDCardConfig(BSP_ARGS *pArgs);
UINT32 BLFlashDownload(BOOT_CFG *pConfig, OAL_KITL_DEVICE *pBootDevices);
UINT32 BLVAtoPA(UINT32 address);

UINT32 OALStringToUINT32(LPCWSTR psz);


BOOL BLReadManufactureCfg( BYTE *pManufCfg);
BOOL BLWriteManufactureCfg(BYTE *pManufCfg);
BOOL BLReadBootCfgSectors(BYTE *pOutBuffer, UINT16 nOutBuffer);

BOOL FlReadData(ULONG size, UCHAR *pData);
//------------------------------------------------------------------------------

#endif
