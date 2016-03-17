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
//  File:  spi.h
//
#ifndef __SPI_BOOT_H
#define __SPI_BOOT_H

//------------------------------------------------------------------------------
//
//  Define:  SPI_DEVICE_NAME
//
#define SPI1_DEVICE_NAME         L"SPI1:"
#define SPI2_DEVICE_NAME         L"SPI2:"
#define SPI3_DEVICE_NAME         L"SPI3:"
#define SPI4_DEVICE_NAME         L"SPI4:"  // ##### Michael, 17-Feb-2010
	
typedef enum 
{
    UNKNOWN = 0,
    MASTER,
    SLAVE,
} SPI_MODE;

typedef struct
{
	BOOL				fInit;
    SPI_MODE			eSpiMode;
    OMAP_MCSPI_REGS*	pSPIRegs;
}SPI_BOOT_DEVICE;

typedef struct 
{
	DWORD				DevNum;
	OMAP_MCSPI_CHANNEL_REGS*	pSPIChannelRegs;
	DWORD				config;
    BOOL						exclusiveAccess;
} SPI_BOOT_INSTANCE;
//------------------------------------------------------------------------------
//
//  Functions: SPIxxx
//
HANDLE SPIOpen(LPCTSTR pSpiName);

VOID SPIClose(HANDLE hContext);

BOOL SPIDisableChannel(HANDLE hContext);

BOOL SPIConfigure(HANDLE hContext, DWORD address, DWORD config);

BOOL SPISetSlaveMode(HANDLE hContext);

DWORD SPIRead(HANDLE hContext, DWORD size, VOID *pBuffer);
   
DWORD SPIWrite(HANDLE hContext, DWORD size, VOID *pBuffer);
   
DWORD SPIWriteRead(HANDLE hContext, DWORD size, VOID *pOutBuffer, VOID *pInBuffer);

// ##### Michael
DWORD SPIWriteReadCS(HANDLE hContext, DWORD inSize, UINT8 *pInBuffer, DWORD outSize, const UINT8 *pOutBuffer );


#endif
