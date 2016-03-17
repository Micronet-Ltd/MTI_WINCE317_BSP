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
//  File:  flash.c
//
//  This file implements boot loader functions related to image flash.
//
#include <windows.h>
#include <bootpart.h>

#include <eboot.h>
#include <nand.h>
#include "BootCmd.h"

//------------------------------------------------------------------------------
//  Global variables

UINT32  g_ulFlashBase = BSP_NAND_REGS_PA;
//------------------------------------------------------------------------------
//  Static variables

static UINT8 g_bpartBuffer[NAND_BPART_BUFFER_SIZE];

//------------------------------------------------------------------------------
// Global variables
//
UINT32 g_ulFlashLengthBytes = sizeof(UINT16);
UINT32 g_ulBPartBase        = (UINT32)g_bpartBuffer;    
UINT32 g_ulBPartLengthBytes = sizeof(g_bpartBuffer);

//------------------------------------------------------------------------------
// Global (Unsupported) Functions

UINT32
ReadFlashNK(
    )
{
	return 0;
};


UINT32
ReadFlashImage( 
		UINT32* pdwImageSize 
		)
{
	return 0;
};


BOOL
ReadFlashXLDR(
    UINT32* p_size
    )

{
	return 0;
};
HANDLE	GetPartEntry( PPARTENTRY pEntry, BYTE FileSystem, BYTE fActive )
{
	return (HANDLE)-1;
};
BOOL	BLGetFlashInfo( FlashInformation* pFlashInfo )
{
	return 0;
};
BOOL	BLReadSector( BYTE* pBuf, DumpParams* params )
{
	return 0;
};

//------------------------------------------------------------------------------
//  Local Functions

static
UINT32
ReadSDNK(
    );

static
VOID
DumpTOC(
    ROMHDR *pTOC
    );

static
BOOL
VerifyImage(
    UCHAR *pData,
    ROMHDR **ppTOC
    );

//------------------------------------------------------------------------------
//  Local Variables

struct {
    HANDLE hFlash;
    BOOL failure;
    ULONG address;
    ULONG size;
} s_binDio;

//------------------------------------------------------------------------------
//
//  Function:  BLFlashDownload
//
//  This function download image from flash memory to RAM.
//
UINT32
BLFlashDownload(
    BOOT_CFG *pConfig,
    OAL_KITL_DEVICE *pBootDevices
    )
{
    UINT32 rc = BL_ERROR;

    return rc;
}

#if 0

//------------------------------------------------------------------------------

static
UINT32
ReadSDNK(
    )
{
    UINT32 rc = BL_ERROR;
    HANDLE hPartition;
    ROMHDR *pTOC;
    ULONG offset, size;
    UCHAR *pData;
    DWORD *pInfo;
    PCI_REG_INFO regInfo;

    memset(&regInfo, 0, sizeof(PCI_REG_INFO));
    regInfo.MemBase.Num    = 1;
    regInfo.MemLen.Num     = 1;
    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    regInfo.MemLen.Reg[0]  = g_ulFlashLengthBytes;

    // Check if there is a valid image
    OALMSG(OAL_INFO, (L"\r\nLoad NK image from SD card file\r\n"));

    // Set address where to place image
    pData = OALPAtoCA(IMAGE_WINCE_CODE_PA);

    // TODO: check for NK.BIN on SD card, load NK.BIN image

    OALMSG(OAL_INFO, (L"NK Image Loaded\r\n"));

    // Done
    g_eboot.launchAddress = IMAGE_WINCE_CODE_PA;
    rc = BL_JUMP;

cleanUp:
    return rc;
}

#endif

//------------------------------------------------------------------------------
//
//  Function:  OEMStartEraseFlash
//
//  This function is called by the bootloader to initiate the flash memory
//  erasing process.
//
BOOL
OEMStartEraseFlash(
    ULONG address,
    ULONG size
    )
{
    BOOL rc = FALSE;

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OEMContinueEraseFlash
//
//  This function is called by the bootloader to during download to provide
//  ability to continue the flash block erasing operation.
//
VOID
OEMContinueEraseFlash(
    )
{
}


//------------------------------------------------------------------------------
//
//  Function:  OEMFinishEraseFlash
//
//  This function is called by the bootloader to finish flash erase before
//  it will call OEMWriteFlash.
//
BOOL
OEMFinishEraseFlash(
    )
{
    BOOL rc = FALSE;

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMWriteFlash
//
//  This function is called by the bootloader to write the image that may
//  be stored in a RAM file cache area to flash memory. This function is
//  called once per each downloaded region.
//
BOOL
OEMWriteFlash(
    ULONG address,
    ULONG size
    )
{
    BOOL rc = FALSE;

    return rc;
}

#if 0

//------------------------------------------------------------------------------

static
VOID
DumpTOC(
    ROMHDR *pTOC
    )
{
    // Print out ROMHDR information
    OALMSG(OAL_INFO, (L"\r\n"));
    OALMSG(OAL_INFO, (L"ROMHDR (pTOC = 0x%08x) ---------------------\r\n", pTOC));
    OALMSG(OAL_INFO, (L"  DLL First           : 0x%08x\r\n", pTOC->dllfirst));
    OALMSG(OAL_INFO, (L"  DLL Last            : 0x%08x\r\n", pTOC->dlllast));
    OALMSG(OAL_INFO, (L"  Physical First      : 0x%08x\r\n", pTOC->physfirst));
    OALMSG(OAL_INFO, (L"  Physical Last       : 0x%08x\r\n", pTOC->physlast));
    OALMSG(OAL_INFO, (L"  Num Modules         : %10d\r\n",   pTOC->nummods));
    OALMSG(OAL_INFO, (L"  RAM Start           : 0x%08x\r\n", pTOC->ulRAMStart));
    OALMSG(OAL_INFO, (L"  RAM Free            : 0x%08x\r\n", pTOC->ulRAMFree));
    OALMSG(OAL_INFO, (L"  RAM End             : 0x%08x\r\n", pTOC->ulRAMEnd));
    OALMSG(OAL_INFO, (L"  Num Copy Entries    : %10d\r\n",   pTOC->ulCopyEntries));
    OALMSG(OAL_INFO, (L"  Copy Entries Offset : 0x%08x\r\n", pTOC->ulCopyOffset));
    OALMSG(OAL_INFO, (L"  Prof Symbol Length  : 0x%08x\r\n", pTOC->ulProfileLen));
    OALMSG(OAL_INFO, (L"  Prof Symbol Offset  : 0x%08x\r\n", pTOC->ulProfileOffset));
    OALMSG(OAL_INFO, (L"  Num Files           : %10d\r\n",   pTOC->numfiles));
    OALMSG(OAL_INFO, (L"  Kernel Flags        : 0x%08x\r\n", pTOC->ulKernelFlags));
    OALMSG(OAL_INFO, (L"  FileSys RAM Percent : 0x%08x\r\n", pTOC->ulFSRamPercent));
    OALMSG(OAL_INFO, (L"  Driver Glob Start   : 0x%08x\r\n", pTOC->ulDrivglobStart));
    OALMSG(OAL_INFO, (L"  Driver Glob Length  : 0x%08x\r\n", pTOC->ulDrivglobLen));
    OALMSG(OAL_INFO, (L"  CPU                 :     0x%04x\r\n", pTOC->usCPUType));
    OALMSG(OAL_INFO, (L"  MiscFlags           :     0x%04x\r\n", pTOC->usMiscFlags));
    OALMSG(OAL_INFO, (L"  Extensions          : 0x%08x\r\n", pTOC->pExtensions));
    OALMSG(OAL_INFO, (L"  Tracking Mem Start  : 0x%08x\r\n", pTOC->ulTrackingStart));
    OALMSG(OAL_INFO, (L"  Tracking Mem Length : 0x%08x\r\n", pTOC->ulTrackingLen));
    OALMSG(OAL_INFO, (L"------------------------------------------------\r\n"));
    OALMSG(OAL_INFO, (L"\r\n"));
}

//------------------------------------------------------------------------------

static
BOOL
VerifyImage(
    UCHAR *pData,
    ROMHDR **ppTOC
    )
{
    BOOL rc = FALSE;
    UINT32 *pInfo;
    ROMHDR *pTOC;

    // Verify that we get CE image.
    pInfo = (UINT32*)(pData + ROM_SIGNATURE_OFFSET);
    if (*pInfo != ROM_SIGNATURE) goto cleanUp;

    // We are on correct location....
    pTOC = (ROMHDR*)(pData + pInfo[2]);

    // Let see
    DumpTOC(pTOC);

    // Return pTOC if pointer isn't NULL
    if (ppTOC != NULL) *ppTOC = pTOC;

    // Done
    rc = TRUE;

cleanUp:
    return rc;
}

#endif

//------------------------------------------------------------------------------

BOOL BLConfigureFlashPartitions(BOOL bForceEnable)
{
    BOOL rc = FALSE;

	return rc;
}
BOOL FlReadData(ULONG size, UCHAR *pData)
{
	return 0;
}
BOOL BLWriteBmp( UINT32 address, UINT32 DataLength )
{
	return 0;
}

