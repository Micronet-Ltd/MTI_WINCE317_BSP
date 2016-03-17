// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

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
//  File:  image_cfg.h
//
//  This file contains image layout definition. They should be consistent
//  with *.bib files and addrtab_cfg.inc mapping table. Generally saying
//  *.bib files constants should be determined from constants in this file
//  and mapping file (*.bib file values are virtual cached addresses for
//  Windows CE OS, but physical address for IPL/EBOOT if they don't use
//  MMU).
//
#ifndef __IMAGE_CFG_H
#define __IMAGE_CFG_H

//------------------------------------------------------------------------------
//
//  Define: DEVICE_RAM/FLASH_xxx
//
//  EVM3530 has 128MB SDRAM located at physical address 0x80000000.
//
#define DEVICE_RAM_PA                   0x80000000
#define DEVICE_RAM_CA                   0x80000000
#define DEVICE_RAM_SIZE                 0x08000000

//------------------------------------------------------------------------------
//
//  Define: IMAGE_SHARE_ARGS_xxx
//
//  Following constants define location and maximal size of arguments shared
//  between loader and kernel. For actual structure see args.h file.
//
#define IMAGE_SHARE_ARGS_PA             0x80000000
#if (BSP_OPM_SELECT == A) || (BSP_CDS == 1)
	#define IMAGE_SHARE_ARGS_SIZE		(0x00001000 - 8)
	#define	IMAGE_SHARE_OPM				(IMAGE_SHARE_ARGS_PA + IMAGE_SHARE_ARGS_SIZE)
	#define	IMAGE_SHARE_RAM_TOPOLOGY	(IMAGE_SHARE_OPM + 4)
#else
	#define IMAGE_SHARE_ARGS_SIZE		(0x00001000 - 4)
	#define	IMAGE_SHARE_RAM_TOPOLOGY	(IMAGE_SHARE_ARGS_PA + IMAGE_SHARE_ARGS_SIZE)
#endif

//------------------------------------------------------------------------------
//
//  Define: IMAGE_WINCE_xxx
//
//  Following constants define Windows CE OS image layout.
//
#define IMAGE_WINCE_CODE_PA             0x8A001000
#define IMAGE_WINCE_CODES_VA            0x80001000
#define IMAGE_WINCE_CODES_PA            0x86001000
#define IMAGE_WINCE_CODE_SIZE           0x03FFF000

//------------------------------------------------------------------------------
//
//  Define: IMAGE_DISPLAY_BUF_xxx
//
//  Following constants define location and size of the display buffer
//
#define IMAGE_DISPLAY_BUF_PA			0x84000000

#define IMAGE_DISPLAY_BUF_SIZE          0x01000000

//------------------------------------------------------------------------------
//
//  Define: IMAGE_XLDR_xxx
//
//  Following constants define image layout for X-Loader. 
//  XLDR executes from SRAM
//
#define IMAGE_XLDR_CODE_PA              0x40202000
#define IMAGE_XLDR_CODE_SIZE            0x00009000

#define IMAGE_XLDR_DATA_PA              0x4020B000
#define IMAGE_XLDR_DATA_SIZE            0x00004000

#define IMAGE_XLDR_STACK_PA             0x4020F000
#define IMAGE_XLDR_STACK_SIZE           0x00001000

//------------------------------------------------------------------------------
//
//  Define: IMAGE_EBOOT_xxx
//
//  Following constants define EBOOT image layout. 
//
#define IMAGE_EBOOT_CODE_PA             0x85E00000
#define IMAGE_EBOOT_CODE_SIZE           0x00040000

#define IMAGE_EBOOT_DATA_PA             0x85E80000
#define IMAGE_EBOOT_DATA_SIZE           0x00050000

#define IMAGE_EBOOT_STACK_PA            0x85EE0000
#define IMAGE_EBOOT_STACK_SIZE          0x00020000


//------------------------------------------------------------------------------
//
//  Define: IMAGE_STARTUP_xxx
//
//  Jump address XLDR uses to bring-up the device.
//
#define IMAGE_STARTUP_IMAGE_PA         (IMAGE_EBOOT_CODE_PA)
#define IMAGE_STARTUP_IMAGE_SIZE       (IMAGE_EBOOT_CODE_SIZE)


//------------------------------------------------------------------------------
//
//  Define: IMAGE_BOOTLOADER_xxx
//
//  Following constants define bootloader information
//

#define IMAGE_XLDR_BOOTSEC_NAND_SIZE        (4 * 128 * 1024)        // Needs to be equal to four NAND flash blocks due to boot ROM requirements
#define IMAGE_EBOOT_BOOTSEC_NAND_SIZE       IMAGE_EBOOT_CODE_SIZE   // Needs to be a multiple of flash block size

#define IMAGE_BOOTLOADER_BITMAP_SIZE        0x00100000                  // Needs to be a multiple of 128k, and minimum 480x640x3 (VGA)  

#define IMAGE_BOOTLOADER_NAND_SIZE      (IMAGE_XLDR_BOOTSEC_NAND_SIZE + \
                                         IMAGE_EBOOT_BOOTSEC_NAND_SIZE + \
                                         IMAGE_BOOTLOADER_BITMAP_SIZE)

//------------------------------------------------------------------------------

#endif
