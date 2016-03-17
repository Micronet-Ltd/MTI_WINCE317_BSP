// Copyright (c) 2008 BSQUARE Corporation. All rights reserved.

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
//  File:  nand_MT29F2G16.h
//
//  This file contains information about NAND flash supported by FMD driver.
//
#ifndef __NAND__MT29F2G16_H
#define __NAND__MT29F2G16_H

//-----------------------------------------------------------------------------
// nand geometry
#define ECC_BYTES					(12)    // number of bytes used for ecc calculation
#define WORD_ACCESS_SIZE			(2)     // Accesses 2 bytes of data
#define SECTOR_DATA_SIZE			(2048)  // 2048 bytes per sector/page
#define SECTORS_PER_BLOCK			(64)    // 64 sectors/page per block
#define NUMBER_OF_BLOCKS_MT29F2G16			(2048)  // 2048 blocks
#define NUMBER_OF_BLOCKS_MT29C4G48			(4096)  // 4096 blocks
#define NUMBER_OF_BLOCKS_MT29F8G16			(8192)  // 8192 blocks
#define NUMBER_OF_BLOCKS_02GR3B2D			(4096)  // 4096 blocks
#define NUMBER_OF_BLOCKS_H8KDS0UN0MER_46M	(4096)  // 4096 blocks

//-----------------------------------------------------------------------------
// nand id
#define MANUFACTURER_ID_MT29F2G16			(0x2C)
#define MANUFACTURER_ID_MT29F8G16			(0x2C)
#define MANUFACTURER_ID_MT29C4G48			(0x2C)
#define MANUFACTURER_ID_02GR3B2D			(0x20)
#define MANUFACTURER_ID_H8KDS0UN0MER_46M	(0xAD)
#define NAND_DEVICE_ID_MT29F2G16			(0xBA)
#define NAND_DEVICE_ID_MT29F8G16			(0xB3)
#define NAND_DEVICE_ID_MT29C4G48			(0xBC)
#define NAND_DEVICE_ID_02GR3B2D				(0xBC)
#define NAND_DEVICE_ID_H8KDS0UN0MER_46M		(0xBC)

//-----------------------------------------------------------------------------
// nand access definitions
#define WRITE_NAND(x,y)         OUTREG16(x,y)   // 16-bit access
#define READ_NAND(x)            INREG16(x)      // 16-bit access
typedef UINT16                  NANDREG;        // 16-bit access

//-----------------------------------------------------------------------------
#define FIFO_THRESHOLD          (64)            // pre-fetch fifo config

//-----------------------------------------------------------------------------
// nand pin connection information
#define GPMC_NAND_CS            (0)             // NAND is on CHIP SELECT 0
#define GPMC_IRQ_WAIT_EDGE      (GPMC_IRQENABLE_WAIT0_EDGEDETECT)

//-----------------------------------------------------------------------------
// nand ecc parameters
#define GPMC_ECC_CONFIG         (GPMC_ECC_CONFIG_16BIT|GPMC_NAND_CS)
#define ECC_PARITY_ARRAY_SIZE   ((SECTOR_DATA_SIZE >> 9) * 3)


//------------------------------------------------------------------------------
//  NAND Spare Area Format for x16 devices

/* 16 bit access, large page nand */
typedef struct
{
    UCHAR  hwBadBlock[2];		// 0, Hardware bad block flag
    UCHAR  ecc[ECC_BYTES];		// 2
    UCHAR reserved1[4];         // 14, E, Reserved - used by FAL
    UCHAR reserved2[2];			// 18, 12, Reserved - used by FAL
    UCHAR swBadBlock;         	// 20, 14, Software bad block flag
    UCHAR oemReserved;          // 21, For use by OEM
    UCHAR unused[42];           // 22, Unused 32-22=10 38-22=16
}NAND_SPARE_AREA;
// for on-die ecc enabled 4-7, 20 - 23, 36 - 39, 52 - 55
// ecc 2-5 (4-7) -> dwReserved1,
// swBadBlock (20) -> bBadBlock,
// oemReserved (21) -> bOEMReserved,
// unused 0-1 (22-23) -> wReserved2
// hwBadBlock will ignored
//-----------------------------------------------------------------------------
__inline OMAP_GPMC_REGS *NAND_GetGpmcRegs(HANDLE hNand);

//-----------------------------------------------------------------------------
__inline NAND_INFO const *NAND_GetGeometry(HANDLE hNand);

//-----------------------------------------------------------------------------
BOOL NAND_LockBlocks(HANDLE hNand, UINT blockStart, UINT blockEnd, BOOL bLock);

//-----------------------------------------------------------------------------
void NAND_Uninitialize(HANDLE hNand);

//-----------------------------------------------------------------------------
HANDLE NAND_Initialize(LPCTSTR szContext, PCI_REG_INFO *pRegIn, PCI_REG_INFO *pRegOut);

//-----------------------------------------------------------------------------
BOOL NAND_Seek(HANDLE hNand, SECTOR_ADDR sector, UINT offset);

//-----------------------------------------------------------------------------
BOOL NAND_Read(HANDLE hNand, BYTE *pData, int size, BYTE *pEcc);

//-----------------------------------------------------------------------------
BOOL NAND_Write(HANDLE hNand, BYTE *pData, int size, BYTE *pEcc);

//-----------------------------------------------------------------------------
UINT16 NAND_GetStatus(HANDLE hNand);

//-----------------------------------------------------------------------------
BOOL NAND_EraseBlock(HANDLE hNand, BLOCK_ID blockId);

//-----------------------------------------------------------------------------
BOOL NAND_Enable(HANDLE hNand, BOOL bEnable);

//-----------------------------------------------------------------------------
UINT NAND_MutexEnter(HANDLE hNand);

//-----------------------------------------------------------------------------
UINT NAND_MutexExit(HANDLE hNand);

//-----------------------------------------------------------------------------
BOOL NAND_SendCommand(HANDLE hNand, UINT cmd);

//-----------------------------------------------------------------------------
BOOL NAND_ConfigurePrefetch(HANDLE hNand, UINT accessType);

//-----------------------------------------------------------------------------

#endif

