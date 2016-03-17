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

//------------------------------------------------------------------------------

//#define EBOOT_VERSION_MAJOR         1
//#define EBOOT_VERSION_MINOR         0

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
//UINT32 BLEthDownload(BOOT_CFG *pBootCfg, OAL_KITL_DEVICE *pBootDevices);
BOOL   BLEthReadData(ULONG size, UCHAR *pData);
VOID   BLEthConfig(BSP_ARGS *pArgs);
UINT32 BLSDCardDownload(BOOT_CFG *pBootCfg, OAL_KITL_DEVICE *pBootDevices);
BOOL   BLSDCardReadData(ULONG size, UCHAR *pData);
VOID   BLSDCardConfig(BSP_ARGS *pArgs);
UINT32 BLFlashDownload(BOOT_CFG *pConfig, OAL_KITL_DEVICE *pBootDevices);
UINT32 BLVAtoPA(UINT32 address);

UINT32 OALStringToUINT32(LPCWSTR psz);

BOOL BLSDCardReadFileData(ULONG MaxSize, UCHAR *pData);
BOOL BLReadBootCfgSectors(BYTE *pOutBuffer, UINT16 nOutBuffer);
UINT32 EraseFlash();
void EraseEntireFlash(UINT32 fSaveParams);
//------------------------------------------------------------------------------
typedef enum 
{
	_1bit_to_1bit = 1,
	_1bit_to_4bit,
	_4bit_to_4bit,
	_4bit_to_1bit
};
#endif
