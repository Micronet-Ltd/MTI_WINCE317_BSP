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
//  Header: boot_cfg.h
//
#ifndef __BOOT_CFG_H
#define __BOOT_CFG_H

//------------------------------------------------------------------------------

OAL_KITL_DEVICE g_bootDevices[] = {
    {L"Burn XLDR  from SD Card",Internal, OMAP_MMCHS1_REGS_PA,	  IMAGE_XLDR_CODE_PA,  BOOT_SDCARD_TYPE,	0},
	{L"Burn EBOOT from SD Card",Internal, OMAP_MMCHS1_REGS_PA,	  IMAGE_EBOOT_CODE_PA, BOOT_SDCARD_TYPE,	0},
	{L"Burn Logo  from SD Card",Internal, OMAP_MMCHS1_REGS_PA,	  IMAGE_WINCE_CODE_PA,BOOT_SDCARD_TYPE,	0},
	{L"Burn NK    from SD Card",Internal, OMAP_MMCHS1_REGS_PA,	  IMAGE_WINCE_CODES_PA,BOOT_SDCARD_TYPE,	0},
	{L"Load NK    from NAND",	Internal, BSP_NAND_REGS_PA + 0x20,IMAGE_WINCE_CODE_PA, OAL_KITL_TYPE_FLASH, 0},
	{0, 0, 0, 0, 0, 0}
};


//------------------------------------------------------------------------------

#endif
