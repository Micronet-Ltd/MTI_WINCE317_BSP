// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
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

/*
================================================================================

 Copyright (c) 2009, Micronet Corporation. All rights reserved.

 History of Changes:
 Anna R.                Adaptation to M5000 platform.
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
#include <fmd.h>
#include <bsp_fmd.h>
#include "BootCmd.h"

#include <fallite.h>
#include <exfileio.h>
#include <bsp_logo.h>
#include <swupdate.h>
//------------------------------------------------------------------------------
//  DEFINES

#define NUMBER_PROGRESS_RECTANGLES 10
#define	BL_HDRSIG_SIZE				7
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
//  Global Functions



UINT32
ReadFlashNK(
    );


UINT32
ReadFlashImage( 
		UINT32* pdwImageSize 
		);


BOOL
ReadFlashXLDR(
    UINT32* p_size
    );

DWORD ReadProgressNotify();

HANDLE	GetPartEntry( PPARTENTRY pEntry, BYTE FileSystem, BYTE fActive );
BOOL	BLGetFlashInfo( FlashInformation* pFlashInfo );
BOOL	BLReadSector( BYTE* pBuf, DumpParams* params );
BOOL	BLWriteBmp( UINT32 address, UINT32 DataLength );
VOID	FlReadDataSetStart(UCHAR *pDataStart);


static BYTE* g_pData = 0;
UINT32  BLFlashStoreLoad();

extern BOOL DownloadImage       (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr);
extern BOOL ResetGlobalVariables();
//------------------------------------------------------------------------------
//  Local Functions

static
BOOL
WriteFlashXLDR(
    UINT32 address,
    UINT32 size
    );


static
BOOL
WriteFlashEBOOT(
    UINT32 address,
    UINT32 size
    );

static
BOOL
WriteFlashNK(
    UINT32 address,
    UINT32 size
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

    // We have do device initialization for some devices
    switch (pConfig->bootDevLoc.IfcType)
        {
        case Internal:
            switch (pConfig->bootDevLoc.LogicalLoc)
            {
				case BSP_NAND_REGS_PA:
				{
					if( BL_ERROR == ( rc = BLFlashStoreLoad() ) )
						pConfig->bootDevLoc.LogicalLoc = BSP_NAND_REGS_PA + 0x20;
				}
				//without break!!!: error or not - read from flash
                case BSP_NAND_REGS_PA + 0x20:
                rc = ReadFlashNK();
                break;
 			}
			break;
        }

    return rc;
}
//
HANDLE	GetPartEntry( PPARTENTRY pEntry, BYTE FileSystem, BYTE fActive )
{
    HANDLE			hPartition = INVALID_HANDLE_VALUE;
	PCI_REG_INFO	regInfo;

	PPARTENTRY		pPEntry;

    memset(&regInfo, 0, sizeof(PCI_REG_INFO));
    regInfo.MemBase.Num    = 1;
    regInfo.MemLen.Num     = 1;
    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    regInfo.MemLen.Reg[0]  = g_ulFlashLengthBytes;


    // Check if there is a valid image
    OALMSG(OAL_INFO, (L"\r\nGet Partitions Entryes\r\n"));

	if( !pEntry )
	{
    	OALLog(L"GetPartEntryes: Error initializing bootpart library!!\r\n");
		return hPartition;
	}

	// Initialize boot partition library
	if (!BP_Init((LPBYTE)g_ulBPartBase, g_ulBPartLengthBytes, NULL, &regInfo, NULL))
	{
    	OALLog(L"GetPartEntryes: Error initializing bootpart library!!\r\n");
		return hPartition;
	}
	
	// Open OS boot partition
    hPartition = BP_OpenPartition(NEXT_FREE_LOC, USE_REMAINING_SPACE, FileSystem, fActive, PART_OPEN_EXISTING);
	if (hPartition == INVALID_HANDLE_VALUE)
	{
        OALMSG(OAL_ERROR, (L"ERROR: OS partition not found!\r\n"));
        return hPartition;
	}

	if( pPEntry = BP_GetPartitionInfo(hPartition) )
		memcpy( pEntry, pPEntry, sizeof(PARTENTRY) );

	return hPartition;
}	
//------------------------------------------------------------------------------


UINT32
ReadFlashNK(
    )
{
    UINT32 rc = BL_ERROR;
    HANDLE hPartition;
    ROMHDR *pTOC;
    ULONG offset, size;
    UCHAR *pData;
    DWORD *pInfo;
    PCI_REG_INFO regInfo;
	FlashInfo FlashInfo;
	UINT32 i, j, steps, addSteps, fakeSteps, ticks;

    memset(&regInfo, 0, sizeof(PCI_REG_INFO));
    regInfo.MemBase.Num    = 1;
    regInfo.MemLen.Num     = 1;
    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    regInfo.MemLen.Reg[0]  = g_ulFlashLengthBytes;


    // Check if there is a valid image
    OALMSG(OAL_INFO, (L"\r\nLoad NK image from flash memory\r\n"));

	// Initialize boot partition library
	if (!BP_Init((LPBYTE)g_ulBPartBase, g_ulBPartLengthBytes, NULL, &regInfo, NULL))
	{
    	OALLog(L"ReadFlashNK: Error initializing bootpart library!!\r\n");
		FillProgress(-1);
		TextWrite( 10, 10, "Flash initialization error", TX_BOTTOM | TX_CENTER, CL_TEXT );
		goto cleanUp;
	}
	
	// Open OS boot partition
    hPartition = BP_OpenPartition(NEXT_FREE_LOC, USE_REMAINING_SPACE, PART_BOOTSECTION, FALSE, PART_OPEN_EXISTING);
	if (hPartition == INVALID_HANDLE_VALUE)
	{
        OALMSG(OAL_ERROR, (L"ERROR: OS partition not found!\r\n"));
		FillProgress(-1);
		TextWrite( 10, 10, "BOOT partition not found", TX_BOTTOM | TX_CENTER, CL_TEXT );
        goto cleanUp;
	}
	
	BP_SetDataPointer(hPartition, 0);
	
    // Set address where to place image
    pData = OALPAtoCA(IMAGE_WINCE_CODES_VA);


    // First read 4kB with pointer to TOC
    offset = 0;
    size = 4096;
	if (!BP_ReadData(hPartition, pData + offset, size))
    {
        OALMSG(OAL_ERROR, (L"ERROR: Error reading OS partition!\r\n"));
		FillProgress(-1);
		TextWrite( 10, 10, "BOOT partition reading error", TX_BOTTOM | TX_CENTER, CL_TEXT );
        goto cleanUp;
    }

    // Verify that we get CE image
    pInfo = (DWORD*)(pData + ROM_SIGNATURE_OFFSET);
    if (*pInfo != ROM_SIGNATURE)
    {
        OALMSG(OAL_ERROR, (L"ERROR: Image signature not found\r\n"));
		FillProgress(-1);
		TextWrite( 10, 10, "         ", TX_BOTTOM | TX_LEFT, CL_TEXT );
		TextWrite( 10, 10, "OS image not found", TX_BOTTOM | TX_CENTER, CL_TEXT );

        goto cleanUp;
    }

    // Read image up through actual TOC
    offset = size;
    size = pInfo[2] - size + sizeof(ROMHDR);
	OALMSG(0, (L"BP_ReadData: pInfo[2]=%d  ,size = %d",pInfo[2],size));
    
	// start display progress
	FMD_GetInfo(&FlashInfo);
	SetFMDNotify(ReadProgressNotify);

//#define FAKE_DELAY_MSEC		7000
//#define FAKE_DELAY_RATIO	200
//#define PROGRESS_BAR_MAX	25

#if FAKE_DELAY_RATIO
	steps = size / PROGRESS_BAR_MAX / FlashInfo.wDataBytesPerSector;
	addSteps = steps * FAKE_DELAY_RATIO / 100;
	fakeSteps = ((size / FlashInfo.wDataBytesPerSector) * FAKE_DELAY_RATIO / 100) / addSteps;
    BLSetProgressData(PROGRESS_BAR_MAX - 1, steps + addSteps);
#else
    BLSetProgressData(NUMBER_PROGRESS_RECTANGLES, size/(NUMBER_PROGRESS_RECTANGLES+1)/FlashInfo.wDataBytesPerSector);
#endif
	if (!BP_ReadData(hPartition, pData + offset, size))
    {
        OALMSG(OAL_ERROR, (L"ERROR: BP_ReadData call failed!\r\n"));
		FillProgress(-1);
		TextWrite( 10, 10, "Flash reading error", TX_BOTTOM | TX_CENTER, CL_TEXT );
        goto cleanUp;
    }

    // Verify image
    if (!VerifyImage(pData, &pTOC))
    {
        OALMSG(OAL_ERROR, (L"ERROR: NK image doesn't have ROM signature\r\n"));
		FillProgress(-1);
		TextWrite( 10, 10, "OS verification error", TX_BOTTOM | TX_CENTER, CL_TEXT );
        goto cleanUp;
    }

#if FAKE_DELAY_RATIO
	for (i = addSteps; i > 0; i--)
	{
		for (j = fakeSteps; j > 0; j--) 
			BLShowProgress();

		ticks = OALGetTickCount() + FAKE_DELAY_MSEC / addSteps;
		while (ticks > OALGetTickCount())
			;
	}
#endif

	// stop  progress display
	SetFMDNotify(NULL);
	BLSetProgressData(0, 0);

    // Read remainder of image
    offset += size;
    size = pTOC->physlast - pTOC->physfirst - offset;

	OALMSG(0, (L"BP_ReadData: remainder size = %d",size));
	if (!BP_ReadData(hPartition, pData + offset, size))
    {
        OALMSG(OAL_ERROR, (L"ERROR: BP_ReadData call failed!\r\n"));
		FillProgress(-1);
		TextWrite( 10, 10, "Flash reading error", TX_BOTTOM | TX_CENTER, CL_TEXT );
        goto cleanUp;
    }

    OALMSG(OAL_INFO, (L"NK Image Loaded\r\n"));

    // Done
    g_eboot.launchAddress = IMAGE_WINCE_CODES_VA;
    rc = BL_JUMP;

cleanUp:
    return rc;
}



//------------------------------------------------------------------------------


UINT32
ReadFlashImage( UINT32* pdwImageSize )
{
    UINT32 rc = BL_ERROR;
    HANDLE hPartition;
    ROMHDR *pTOC;
    ULONG offset, size;
    UCHAR *pData;
    DWORD *pInfo;
    PCI_REG_INFO regInfo;
	FlashInfo FlashInfo;

    memset(&regInfo, 0, sizeof(PCI_REG_INFO));
    regInfo.MemBase.Num    = 1;
    regInfo.MemLen.Num     = 1;
    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    regInfo.MemLen.Reg[0]  = g_ulFlashLengthBytes;


    // Check if there is a valid image
    OALMSG(OAL_INFO, (L"\r\nLoad NK image from flash memory\r\n"));

	// Initialize boot partition library
	if (!BP_Init((LPBYTE)g_ulBPartBase, g_ulBPartLengthBytes, NULL, &regInfo, NULL))
	{
    	OALLog(L"ReadFlashNK: Error initializing bootpart library!!\r\n");
		goto cleanUp;
	}
	
	// Open OS boot partition
    hPartition = BP_OpenPartition(NEXT_FREE_LOC, USE_REMAINING_SPACE, PART_BOOTSECTION, FALSE, PART_OPEN_EXISTING);
	if (hPartition == INVALID_HANDLE_VALUE)
	{
        OALMSG(OAL_ERROR, (L"ERROR: OS partition not found!\r\n"));
        goto cleanUp;
	}
	
	BP_SetDataPointer(hPartition, 0);
	
    // Set address where to place image
    pData = OALPAtoCA(IMAGE_WINCE_CODES_VA);
	  OALMSG(OAL_INFO, (L"\r\nLoad NK image from flash memory pData = %x \r\n",(DWORD)pData));


    // First read 4kB with pointer to TOC
    offset = 0;
    size = 4096;
	if (!BP_ReadData(hPartition, pData + offset, size))
    {
        OALMSG(OAL_ERROR, (L"ERROR: Error reading OS partition!\r\n"));
        goto cleanUp;
    }

    // Verify that we get CE image
    pInfo = (DWORD*)(pData + ROM_SIGNATURE_OFFSET);
    if (*pInfo != ROM_SIGNATURE)
        {
        OALMSG(OAL_ERROR, (L"ERROR: "
            L"Image signature not found\r\n"
            ));
        goto cleanUp;
        }

    // Read image up through actual TOC
    offset = size;
    size = pInfo[2] - size + sizeof(ROMHDR);
	OALMSG(0, (L"BP_ReadData: pInfo[2]=%d  ,size = %d",pInfo[2],size));
    
	// start display progress
	FMD_GetInfo(&FlashInfo);
	//SetFMDNotify(ReadProgressNotify);
   // BLSetProgressData(NUMBER_PROGRESS_RECTANGLES, size/NUMBER_PROGRESS_RECTANGLES/FlashInfo.wDataBytesPerSector);

	if (!BP_ReadData(hPartition, pData + offset, size))
    {
        OALMSG(OAL_ERROR, (L"ERROR: "
            L"BP_ReadData call failed!\r\n"
            ));
        goto cleanUp;
     }

    // Verify image
    if (!VerifyImage(pData, &pTOC))
        {
        OALMSG(OAL_ERROR, (L"ERROR: "
            L"NK image doesn't have ROM signature\r\n"
            ));
        goto cleanUp;
        }

	// stop  progress display
	//SetFMDNotify(NULL);
	//BLSetProgressData(0, 0);

    // Read remainder of image
    offset += size;
    size = pTOC->physlast - pTOC->physfirst - offset;

	*pdwImageSize = pTOC->physlast - pTOC->physfirst;

	OALMSG(0, (L"BP_ReadData: remainder size = %d",size));
	if (!BP_ReadData(hPartition, pData + offset, size))
    {
        OALMSG(OAL_ERROR, (L"ERROR: "
            L"BP_ReadData call failed!\r\n"
            ));
        goto cleanUp;
    }

    OALMSG(OAL_INFO, (L"NK Image Loaded *pdwImageSize = %x \r\n",*pdwImageSize));

    // Done
    g_eboot.launchAddress = IMAGE_WINCE_CODES_PA;
    rc = BL_JUMP;

cleanUp:
    return rc;
}

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
    BOOL rc = TRUE;

    OALMSG(OAL_FUNC, (
        L"+OEMStartEraseFlash(0x%08x, 0x%08x)\r\n", address, size
        ));
    
    OALMSG(OAL_FUNC, (L"-OEMStartEraseFlash(rc = %d)\r\n", rc));
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
    BOOL rc = TRUE;

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
    BOOL rc;
	OALMSG(0, (L"OEMWriteFlash: g_eboot.type = %x \r\n",g_eboot.type));

    switch (g_eboot.type)
        {
        case DOWNLOAD_TYPE_XLDR:
            rc = WriteFlashXLDR(address, size);
            break;
        case DOWNLOAD_TYPE_EBOOT:
            rc = WriteFlashEBOOT(address, size);
            break;
        case DOWNLOAD_TYPE_FLASHRAM:
            rc = WriteFlashNK(address, size);
            break;
        default:
            rc = FALSE;
            break;
        }

    return rc;
}

//------------------------------------------------------------------------------

BOOL
WriteFlashXLDR(
    UINT32 address,
    UINT32 size
    )
{
    BOOL rc = FALSE;
    HANDLE hFlash = NULL;
    ROMHDR *pTOC;
    UINT32 offset, xldrSize, blocknum, blocksize;
    UINT8 *pData;


	OALMSG(OAL_INFO, (L"\r\nWriting XLDR image to flash memory\r\n"));

    // Open flash storage
    hFlash = OALFlashStoreOpen(g_ulFlashBase);
    if (hFlash == NULL)
        {
        OALMSG(OAL_ERROR, (L"ERROR: OEMWriteFlash: "
            L"OALFlashStoreOpen call failed!\r\n"
            ));
        goto cleanUp;
        }

    // Get data location
    pData = OEMMapMemAddr(address, address);

    // Verify image
    if (!VerifyImage(pData, &pTOC))
        {
        OALMSG(OAL_ERROR, (L"ERROR: OEMWriteFlash: "
            L"XLDR image signature not found\r\n"
            ));
        goto cleanUp;
        }

    // Verify that this is XLDR image
    if (pTOC->numfiles > 0 || pTOC->nummods > 1)
        {
        OALMSG(OAL_ERROR, (L"ERROR: OEMWriteFlash: "
            L"XLDR image must have only one module and no file\r\n"
            ));
        goto cleanUp;
        }

    // Check for maximal XLRD size
    xldrSize = pTOC->physlast - pTOC->physfirst;
    if (xldrSize > (IMAGE_XLDR_CODE_SIZE - 2*sizeof(DWORD)) )
        {
        OALMSG(OAL_ERROR, (L"ERROR: OEMWriteFlash: "
            L"XLDR image size 0x%04x doesn't fit to limit 0x%04x\r\n", size, IMAGE_XLDR_CODE_SIZE - 2*sizeof(DWORD)
            ));
        goto cleanUp;
        }
		
	blocksize = OALFlashStoreBlockSize(hFlash);
	
	if (blocksize < IMAGE_XLDR_CODE_SIZE)
        {
        OALMSG(OAL_ERROR, (L"ERROR: OEMWriteFlash: "
            L"XLDR image size 0x%04x doesn't fit to flash block size 0x%04x\r\n", IMAGE_XLDR_CODE_SIZE, blocksize
            ));
        goto cleanUp;
        }

    // First we have to offset image by 2 DWORDS to insert BootROM header 
    memmove(pData + 2*sizeof(DWORD), pData, xldrSize);
    
    // Insert BootROM header
    ((DWORD*)pData)[0] = IMAGE_XLDR_CODE_SIZE - 2*sizeof(DWORD);    // Max size of image
    ((DWORD*)pData)[1] = IMAGE_XLDR_CODE_PA;                        // Load address

    // Now copy into first four blocks per boot ROM requirement.
	// Internal boot ROM expects the loader to be duplicated in the first 4 blocks for
	// data redundancy. Note that block size of memory used on this platform is larger 
	// than xldr size so there will be a gap in each block.
    offset = 0;
	for (blocknum = 0; blocknum < 4; blocknum++)
		{
        if (!OALFlashStoreWrite(
                hFlash, offset, pData, IMAGE_XLDR_CODE_SIZE, FALSE, FALSE
                ))
            {
            OALMSG(OAL_ERROR, (L"ERROR: OEMWriteFlash: "
                L"OALFlashStoreWrite at relative address 0x%08x failed\r\n",
                offset
                ));
            }
        offset += blocksize;
		}

      OALMSG(OAL_INFO, (L"XLDR image written\r\n"));

    // Done
    rc = TRUE;

cleanUp:
    if (hFlash != NULL) OALFlashStoreClose(hFlash);
    return rc;
}

//------------------------------------------------------------------------------

BOOL
ReadFlashXLDR(
    UINT32* p_size
    )
{
    BOOL rc = FALSE;
    HANDLE hFlash = NULL;
    ROMHDR *pTOC;
    UINT32 offset, xldrSize;
	UINT32 blocknum, blocksize;
    UINT8 *pData;


	OALMSG(OAL_INFO, (L"\r\nReading XLDR image from flash memory to RAM\r\n"));

    // Open flash storage
    hFlash = OALFlashStoreOpen(g_ulFlashBase);
    if (hFlash == NULL)
        {
        OALMSG(OAL_ERROR, (L"ERROR: ReadFlashXLDR: "
            L"OALFlashStoreOpen call failed!\r\n"
            ));
        goto cleanUp;
        }

	blocksize = OALFlashStoreBlockSize(hFlash);
	
	if (blocksize < IMAGE_XLDR_CODE_SIZE)
        {
        OALMSG(OAL_ERROR, (L"ERROR: ReadFlashXLDR: "
            L"XLDR image size 0x%04x doesn't fit to flash block size 0x%04x\r\n", IMAGE_XLDR_CODE_SIZE, blocksize
            ));
        goto cleanUp;
        }

    // Get data location

	pData = (UINT8*)IMAGE_WINCE_CODE_PA;

	offset = 0;
	for (blocknum = 0; blocknum < 4; blocknum++)
	{
		// skip bad blocks
		if ((FMD_GetBlockStatus(blocknum) & BLOCK_STATUS_BAD) == 0)
        {
			if (!OALFlashStoreRead( hFlash, offset, pData, IMAGE_XLDR_CODE_SIZE, FALSE, FALSE))
				{
					OALMSG(OAL_ERROR, (L"ERROR: ReadFlashXLDR: "
						 L"OALFlashStoreRead at relative address 0x%08x failed\r\n",offset));
				}
			else
				break;
		}
		else
			offset += blocksize;
	}

	
    // Verify image
	//offset image by 2 DWORDS for BootROM header 
    if (!VerifyImage(pData + 2*sizeof(DWORD), &pTOC))
        {
        OALMSG(OAL_ERROR, (L"ERROR: OEMWriteFlash: "
            L"XLDR image signature not found\r\n"
            ));
        }

    // Verify that this is XLDR image
    if (pTOC->numfiles > 0 || pTOC->nummods > 1)
        {
        OALMSG(OAL_ERROR, (L"ERROR: OEMWriteFlash: "
            L"XLDR image must have only one module and no file\r\n"
            ));
        goto cleanUp;
        }

    // Check for maximal XLRD size
    xldrSize = pTOC->physlast - pTOC->physfirst;
    if (xldrSize > (IMAGE_XLDR_CODE_SIZE - 2*sizeof(DWORD)) )
        {
        OALMSG(OAL_ERROR, (L"ERROR: OEMWriteFlash: "
            L"XLDR image size 0x%04x doesn't fit to limit 0x%04x\r\n", xldrSize, IMAGE_XLDR_CODE_SIZE - 2*sizeof(DWORD)
            ));
        goto cleanUp;
        }
		
	*p_size = xldrSize;
	
     OALMSG(OAL_INFO, (L"XLDR image read\r\n"));

    // Done
    rc = TRUE;

cleanUp:
    if (hFlash != NULL) OALFlashStoreClose(hFlash);
    return rc;
}

//------------------------------------------------------------------------------

BOOL
WriteFlashEBOOT(
    UINT32 address,
    UINT32 size
    )
{
    BOOL rc = FALSE;
    HANDLE hFlash = NULL;
    UINT8 *pData;
    UINT32 offset;

	UINT8* buffer = (UINT8*)IMAGE_WINCE_CODE_PA;


    OALMSG(OAL_INFO, (L"\r\nWriting EBOOT image to flash memory. \r\n"));

    // Open flash storage
    hFlash = OALFlashStoreOpen(g_ulFlashBase);
    if (hFlash == NULL)
        {
        OALMSG(OAL_ERROR, (L"ERROR: OEMWriteFlash: "
            L"OALFlashStoreOpen call failed!\r\n"
            ));
        goto cleanUp;
        }

    // Check if image fit (last 3 sectors used for configuration)
    if (size > (IMAGE_EBOOT_CODE_SIZE - 3*OALFlashStoreSectorSize(hFlash)))
        {
        OALMSG(OAL_ERROR, (L"ERROR: OEMWriteFlash: "
            L"EBOOT image too big (size 0x%08x, maximum size 0x%08x)\r\n",
            size, IMAGE_EBOOT_CODE_SIZE - OALFlashStoreBlockSize(hFlash)
            ));
        goto cleanUp;
        }

    // Get data location
    pData = OEMMapMemAddr(address, address);

    // Verify that we get CE image
    if (!VerifyImage(pData, NULL))
        {
        OALMSG(OAL_ERROR, (L"ERROR: OEMWriteFlash: "
            L"EBOOT image signature not found\r\n"
            ));
        goto cleanUp;
        }

    // Fill unused space with 0xFF
    if (size < IMAGE_EBOOT_CODE_SIZE)
        {
        memset(pData + size, 0xFF, IMAGE_EBOOT_CODE_SIZE - size);
        }


	// read all the configuration data and save it in the new boot block'



	if (!BLReadBootCfgSectors(buffer ,3*(UINT16)OALFlashStoreSectorSize(hFlash)))
	{
		OALMSG(OAL_ERROR, (L"ERROR: OEMWriteFlash: "
            L"couldn't read boot configuration/manufacture sectors\r\n"
            ));
        goto cleanUp;
	}
	
	// save 3 last configuration sectors
	memcpy (pData + IMAGE_EBOOT_CODE_SIZE- 3*OALFlashStoreSectorSize(hFlash), buffer, 3*OALFlashStoreSectorSize(hFlash));

  

	// burn  boot blocks

	offset = IMAGE_XLDR_BOOTSEC_NAND_SIZE;

    if (!OALFlashStoreWrite(
            hFlash, offset, pData, IMAGE_EBOOT_CODE_SIZE, FALSE, FALSE
            ))
        {
        OALMSG(OAL_ERROR, (L"ERROR: OEMWriteFlash: "
            L"OALFlashStoreWrite at relative address 0x%08x failed\r\n", offset
            ));
        goto cleanUp;
        }

	OALMSG(OAL_INFO, (L"EBOOT image written\r\n"));

    // Done
    rc = TRUE;

cleanUp:
    if (hFlash != NULL) OALFlashStoreClose(hFlash);
    return rc;
}

//------------------------------------------------------------------------------

BOOL
WriteFlashNK(
    UINT32 address,
    UINT32 size
    )
{
    BOOL rc = FALSE;
    HANDLE hPartition;
    UCHAR *pData;
    PCI_REG_INFO regInfo;

    memset(&regInfo, 0, sizeof(PCI_REG_INFO));
    regInfo.MemBase.Num    = 1;
    regInfo.MemLen.Num     = 1;
    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    regInfo.MemLen.Reg[0]  = g_ulFlashLengthBytes;


#ifndef SHIP_BUILD
	OALLog(L"Writing NK image to OS partition\r\n");
#endif

    // Get data location
    pData = OEMMapMemAddr(address, address);

    // Verify that we get CE image.
    if (!VerifyImage(pData, NULL))
        {
        OALMSG(OAL_ERROR, (L"ERROR: OEMWriteFlash: "
            L"NK image signature not found\r\n"
            ));
        goto cleanUp;
        }

	// Initialize boot partition library
	if (!BP_Init((LPBYTE)g_ulBPartBase, g_ulBPartLengthBytes, NULL, &regInfo, NULL))
	{
    	OALLog(L"WriteFlashNK: Error initializing bootpart library!!\r\n");
		goto cleanUp;
	}
	
	// Open OS boot partition
    hPartition = BP_OpenPartition(NEXT_FREE_LOC, USE_REMAINING_SPACE, PART_BOOTSECTION, FALSE, PART_OPEN_EXISTING);
	if (hPartition == INVALID_HANDLE_VALUE)
	{
        OALMSG(OAL_ERROR, (L"ERROR: OS partition not found!\r\n"));
        goto cleanUp;
	}
	
	// Check length against size of partition
	if (!BP_SetDataPointer(hPartition, size))
	{
        OALMSG(OAL_ERROR, (L"ERROR: OS partition too small!  Aborting...\r\n"));
		goto cleanUp;
	}
	
	// Write image to partition
	BP_SetDataPointer(hPartition, 0);
	if (!BP_WriteData(hPartition, pData, size))
	{
        OALMSG(OAL_ERROR, (L"ERROR: Failed writing to OS partition!\r\n"));
		goto cleanUp;
	}
	
#ifndef SHIP_BUILD
	 OALLog(L"NK image written\r\n");
#endif

    // Change boot device to NAND
    g_bootCfg.bootDevLoc.IfcType = Internal;
    g_bootCfg.bootDevLoc.LogicalLoc = BSP_NAND_REGS_PA + 0x20;

    // Done
    rc = TRUE;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------

BOOL BLConfigureFlashPartitions(BOOL bForceEnable)
{
    BOOL rc = FALSE;
    HANDLE hFMD;
    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;
    HANDLE hPartition;
	PPARTENTRY pPartitionEntry;
	DWORD dwBootPartitionSectorCount;

    memset(&regInfo, 0, sizeof(PCI_REG_INFO));
    regInfo.MemBase.Num    = 1;
    regInfo.MemLen.Num     = 1;
    regInfo.MemBase.Reg[0] = g_ulFlashBase;
	
    // Get flash info
	hFMD = FMD_Init(NULL, &regInfo, NULL);
    if (hFMD == NULL)
        goto cleanUp;

    if (!FMD_GetInfo(&flashInfo))
        goto cleanUp;
	
	FMD_Deinit(hFMD);
	
	// Initialize boot partition library
	if (!BP_Init((LPBYTE)g_ulBPartBase, g_ulBPartLengthBytes, NULL, &regInfo, NULL))
	{
    	OALLog(L"BLConfigureFlashPartitions: Error initializing bootpart library!!\r\n");
		goto cleanUp;
	}
	
	// Get boot partition size
	// Ensure boot partition uses entire blocks with no space left over
	// Round up to an even block size
	dwBootPartitionSectorCount = ((g_bootCfg.osPartitionSize + (flashInfo.dwBytesPerBlock - 1))/ flashInfo.dwBytesPerBlock) * flashInfo.wSectorsPerBlock;
	// Reduce by one to account for MBR, which will be the first sector in non reserved area.
	// This causes boot partition to end on a block boundary
	dwBootPartitionSectorCount -= 1;

	OALLog(L"OS part info: P(%d, %d) F(%d, %d)\r\n", dwBootPartitionSectorCount, g_bootCfg.osPartitionSize, flashInfo.dwBytesPerBlock, flashInfo.wSectorsPerBlock);

	// Check for existence and size of OS boot partition
    hPartition = BP_OpenPartition(NEXT_FREE_LOC, USE_REMAINING_SPACE, PART_BOOTSECTION, FALSE, PART_OPEN_EXISTING);
	if (hPartition == INVALID_HANDLE_VALUE)
	{
#ifndef SHIP_BUILD
    	OALLog(L"OS partition does not exist!!\r\n");
#endif
	}
	else
	{
		pPartitionEntry = BP_GetPartitionInfo(hPartition);
		if (dwBootPartitionSectorCount != pPartitionEntry->Part_TotalSectors)
		{
#ifndef SHIP_BUILD
    		OALLog(L"OS partition does not match configured size!!  Sector count expected: 0x%x, actual 0x%x\r\n", dwBootPartitionSectorCount, pPartitionEntry->Part_TotalSectors);
#endif
			// Mark handle invalid to kick us into formatting code
			hPartition = INVALID_HANDLE_VALUE;
		}
	}
		
	if ((hPartition == INVALID_HANDLE_VALUE) || (bForceEnable == TRUE))
	{
		// OS binary partition either does not exist or does not match configured size
#ifndef SHIP_BUILD
    	OALLog(L"Formatting flash...\r\n");
#endif
		// Create a new partion
		// Can't just call BP_OpenPartition with PART_OPEN_ALWAYS because it will erase reserved 
		// blocks (bootloader) if MBR doesn't exist.  Also, we want to ensure the boot partition
		// is actually the first partition on the flash.  So do low level format here (note that 
		// this destroys all other partitions on the device)
		// Note, we're skipping the block check for speed reasons. Might not want this in a production device...
        BP_LowLevelFormat (0, flashInfo.dwNumBlocks, FORMAT_SKIP_RESERVED|FORMAT_SKIP_BLOCK_CHECK);
		// Create the OS partition
    	hPartition = BP_OpenPartition(NEXT_FREE_LOC, dwBootPartitionSectorCount, PART_BOOTSECTION, FALSE, PART_CREATE_NEW);
		if (hPartition == INVALID_HANDLE_VALUE)
		{
    		OALLog(L"Error creating OS partition!!\r\n");
			goto cleanUp;
		}
//#if 0
		// Create FAT partition on remaining flash (can be automatically mounted)
    	hPartition = BP_OpenPartition(NEXT_FREE_LOC, USE_REMAINING_SPACE, PART_DOS32, FALSE, PART_CREATE_NEW);
		if (hPartition == INVALID_HANDLE_VALUE)
		{
    		OALLog(L"Error creating file partition!!\r\n");
			goto cleanUp;
		}
//#endif
#ifndef SHIP_BUILD
    	OALLog(L"Flash format complete!\r\n");
#endif
	}
	
	// Done
	rc = TRUE;

cleanUp:
	return rc;
}


//------------------------------------------------------------------------------
DWORD ReadProgressNotify()
{
	//static DWORD i = 0;

	BLShowProgress();

	
	//OALLog(L"RPN: %d" ,i);
	//i++;

	return 1;
}
BOOL	BLGetFlashInfo( FlashInformation* pFlashInfo )
{
	HANDLE			hFMD	= 0;
	PCI_REG_INFO	regInfo;
	BOOL			res		= 0;
	UINT32			status, block;
	FlashInfo		finfo;

	regInfo.MemBase.Reg[0] = g_ulFlashBase;
	hFMD = FMD_Init(NULL, &regInfo, NULL);
	
	if(hFMD)
	{
		if( FMD_GetInfo(&finfo) )
		{
			pFlashInfo->flashType			=	(UINT32)finfo.flashType;
			pFlashInfo->dwNumBlocks			=	finfo.dwNumBlocks;
			pFlashInfo->dwBytesPerBlock		=	finfo.dwBytesPerBlock;
			pFlashInfo->wSectorsPerBlock	=	finfo.wSectorsPerBlock;
			pFlashInfo->wDataBytesPerSector	=	finfo.wDataBytesPerSector;

			pFlashInfo->dwNumOfBadBlocks	= 0;
			
			block = 0;					
			
			while(block < pFlashInfo->dwNumBlocks) 
			{
				status = FMD_GetBlockStatus(block);

				if(status & BLOCK_STATUS_BAD) 
				{
					OALMSG(OAL_INFO, (L"Bad block No %d\r\n", block ));
					pFlashInfo->dwNumOfBadBlocks++;
				}
				block++;
			}
			res		= 1;
		}
	}
    if( hFMD ) 
		FMD_Deinit(hFMD);
	
	return res;
}
BOOL BLReadSector( BYTE* pBuf, DumpParams* params )
{
	static HANDLE	hFlash	= 0;
	BOOL	res		= 0;

	if( !params )
		return res;

	if( DUMP_START & params->DumpFlag )
		hFlash = OALFlashStoreOpen(g_ulFlashBase);
	
	if(0 == hFlash)
		return res;
	
	if( OALFlashStoreRead( hFlash, params->DumpOffset, pBuf, params->DumpSize, FALSE, FALSE))
	{
		res = 1;
	}

	if( DUMP_STOP & params->DumpFlag )
		OALFlashStoreClose( hFlash );
	
	return res;
}
VOID FlReadDataSetStart(UCHAR *pDataStart)
{
	g_pData = pDataStart;
}
BOOL FlReadData(ULONG size, UCHAR *pData)
{
	memcpy(pData,g_pData,size);
    g_pData += size;

	return TRUE;
}
BOOL BLWriteBmp( UINT32 address, UINT32 DataLength )
{
    BOOL rc			= 0;
    HANDLE hFlash	= 0;
    UINT8 *pData;
    UINT32 offset;

    OALMSG(OAL_INFO, (L"\r\nWriting bmp to flash memory. \r\n"));

    pData = (UINT8*)address;

	if( DataLength			> IMAGE_BOOTLOADER_BITMAP_SIZE ||
	    *((UINT16*)pData)	!=   0x4D42						)  //  Check for 'BM' signature
	{
		OALMSG(OAL_ERROR, (L"FLWriteBmp: data error\r\n") );
		return rc;
	}
    // Open flash storage
    hFlash = OALFlashStoreOpen(g_ulFlashBase);
    if( !hFlash)
    {
        OALMSG(OAL_ERROR, (L"ERROR: OEMWriteFlash: OALFlashStoreOpen call failed!\r\n" ));
        return rc;
    }
	// burn blocks
	offset = IMAGE_XLDR_BOOTSEC_NAND_SIZE + IMAGE_EBOOT_BOOTSEC_NAND_SIZE;

	if( DataLength % OALFlashStoreSectorSize(hFlash) )
		DataLength += OALFlashStoreSectorSize(hFlash) - (DataLength % OALFlashStoreSectorSize(hFlash));

    if(!OALFlashStoreWrite( hFlash, offset, pData, DataLength, FALSE, FALSE  ))
    {
		OALMSG(OAL_ERROR, (L"ERROR: OEMWriteFlash: OALFlashStoreWrite at relative address 0x%08x failed\r\n", offset ));
    }
	else
	{
		OALMSG(OAL_INFO, (L"bmp written\r\n"));
	    rc = TRUE;
	}

	OALFlashStoreClose(hFlash);
    return rc;
}
DWORD ProgressNotify()
{
	//static DWORD i = 0;
	ShowProgressLoop();
	//OALLog(L"RPN: %d" ,i);+
	//i++;
	return 1;
}

//typedef union
//{
//	struct
//	{
//		unsigned long conversation	: 4;
//		unsigned long data_type		: 6;
//		unsigned long params		: 6;
//		unsigned long err_type		: 4;
//		unsigned long err_data_type : 6;
//		unsigned long err_params	: 6;
//	};
//	UINT32 bits;
//} sw_update_state;

#if BSP_FLASHFX == 1	//OTA from SD

UINT32 BLFlashStoreLoad()
{
    UINT32			rc		= BL_ERROR;
    DWORD			Data	= 0;
    DWORD			length	= 0, dwLaunchAddr = 0;
	UINT8*			ptr;
	BSP_ARGS*		pArgs	= OALPAtoUA(IMAGE_SHARE_ARGS_PA);

	UINT32			type;
	UINT32			MaxSize = 0, size = 0;

	OMAP_PRCM_GLOBAL_PRM_REGS * pPrmGlobal = OALPAtoUA(OMAP_PRCM_GLOBAL_PRM_REGS_PA);

	sw_update_state* pFlags = (sw_update_state*)&pArgs->SwUpdateFlags;
	OALMSG(OAL_INFO, (L"+BLFlashStoreLoad:\r\n"));

	g_bootCfg.bootDevLoc.LogicalLoc = BSP_NAND_REGS_PA + 0x20;//for the next boot
	BLWriteBootCfg(&g_bootCfg);
	
	/////
	BLSetProgressData(0, 0);
	BLSetProgressData(NUMBER_PROGRESS_RECTANGLES, NUMBER_PROGRESS_RECTANGLES);
	SetFatNotify(ProgressNotify);

	for(type = sw_type_xldr; type <= sw_type_nk; type <<= 1)
	{
		//if current type update is not needed
		if(!(type & ((sw_update_state*)&g_bootCfg.SwUpdateFlags)->data_type))
			continue;

		g_bootCfg.filename[0] = 0;
		ptr = (UINT8*)IMAGE_WINCE_CODES_PA;	
	
		switch(type)
		{
			case sw_type_xldr:
				wcscpy(g_bootCfg.filename, L"XLDRNAND.BIN");
				MaxSize = IMAGE_XLDR_CODE_SIZE;
				ptr = (UINT8*)IMAGE_WINCE_CODE_PA;
			break;
			case sw_type_eboot:
				wcscpy(g_bootCfg.filename, L"EBNAND.BIN");
				MaxSize = IMAGE_EBOOT_CODE_SIZE;
				ptr = (UINT8*)IMAGE_WINCE_CODE_PA;
			break;
			case sw_type_mnfr:
				wcscpy(g_bootCfg.filename, L"MNFR.BIN");
				MaxSize = sizeof(manufacture_block);
			break;
			case sw_type_logo:
				wcscpy(g_bootCfg.filename, L"LOGO.BMP");
				MaxSize = IMAGE_BOOTLOADER_BITMAP_SIZE;
			break;
			case sw_type_nk:
				wcscpy(g_bootCfg.filename, L"NK.BIN");
				MaxSize = IMAGE_WINCE_CODE_SIZE;
			break;
		}
		if(0 == g_bootCfg.filename[0])
			continue;

		//BLSetProgressData(0, 0);
		//BLSetProgressData(NUMBER_PROGRESS_RECTANGLES, NUMBER_PROGRESS_RECTANGLES);
		//SetFatNotify(ProgressNotify);

		if(BL_ERROR == BLSDCardDownload(&g_bootCfg, 0))
		{
			pFlags->bits			|= SW_CANNOT_OPEN;
			pFlags->err_data_type	= type;
			OALMSG(OAL_ERROR, (L"BLFlashStoreLoad:  cannot open file %s\r\n", g_bootCfg.filename));
			break;
		}
		if(0 == (size = BLSDCardReadFileData(MaxSize, ptr) ) )
		{
	 		
			pFlags->bits			|= SW_CANNOT_READ;
			pFlags->err_data_type	= type;
			OALMSG(OAL_ERROR, (L"BLFlashStoreLoad:  cannot read file %s\r\n", g_bootCfg.filename));
			break;
		}
		OALMSG(OAL_INFO, (L"BLFlashStoreLoad:  read OK\r\n"));
		//////////////////////////////////////

		g_bootCfg.bootDevLoc.LogicalLoc = BSP_NAND_REGS_PA;//for now
		g_pData	= ptr;
		
		ProgressNotify();

		if(type == sw_type_nk)
		{
			BLSetProgressData(0, 0);
			BLSetProgressData(NUMBER_PROGRESS_RECTANGLES, size/NUMBER_PROGRESS_RECTANGLES/2048);
			SetFMDNotify(ProgressNotify);//for write
		}

		if(sw_type_mnfr == type)
		{
			if(sizeof(manufacture_block) < size)//pFile->file_size)
			{
				pFlags->bits			|= SW_DATA_ERROR;
				pFlags->err_data_type	= type;
				OALMSG(OAL_ERROR, (L"BLFlashStoreLoad:  manufacture block error size\r\n"));
				break;
			}
			if( !BLWriteManufactureCfg(g_pData) )
			{
				pFlags->bits			|= SW_DATA_ERROR;
				pFlags->err_data_type	= type;
				OALMSG(OAL_ERROR, (L"BLFlashStoreLoad:  BLWriteManufactureCfg error\r\n"));
				break;
			}

			OALMSG(OAL_INFO, (L"manufacture block written\r\n"));
		}
		else if(sw_type_logo == type)
		{
			if(!BLWriteBmp((UINT32)g_pData, size))
			{
				pFlags->bits			|= SW_DATA_ERROR;
				pFlags->err_data_type	= type;
				OALMSG(OAL_ERROR, (L"BLFlashStoreLoad:  BLWriteBmp error\r\n"));
				break;
			}
		}
		else 
		{
			if (0 != memcmp (g_pData, "B000FF\x0A", BL_HDRSIG_SIZE))
			{
				pFlags->bits			|= SW_DATA_ERROR;
				pFlags->err_data_type	= type;
				OALMSG(OAL_ERROR, (L"BLFlashStoreLoad:  bin signature error\r\n"));
 				break;
			}
			ResetGlobalVariables();
			if(!DownloadImage( (LPDWORD)&Data, &length, &dwLaunchAddr ) )
			{
				pFlags->bits			|= SW_DATA_ERROR;
				pFlags->err_data_type	= type;
				OALMSG(OAL_ERROR, (L"BLFlashStoreLoad:  DownloadImage error\r\n"));
 				break;
			}
		}		
	}//////	

	pFlags->bits &= ~SW_START_UPDATE;
	pFlags->bits |= SW_END_UPDATE;

	rc = BL_JUMP;

	g_bootCfg.bootDevLoc.LogicalLoc = BSP_NAND_REGS_PA + 0x20;////for the next boot
	g_bootCfg.SwUpdateFlags = 0;
	memset(g_bootCfg.filename, 0, sizeof(g_bootCfg.filename));
	BLWriteBootCfg(&g_bootCfg);

//	OALMSG(OAL_INFO, (L"BLFlashStoreLoad: Download Image binary: completed. Status = %x.\r\n",  rval));

	FillProgress(-1);	

	// Do warm reset
	OUTREG32(&pPrmGlobal->PRM_RSTCTRL, /*RSTCTRL_RST_DPLL3|*/ RSTCTRL_RST_GS);

	return rc;
}

#else //BSP_FLASHFX != 1 //OTA from flash
UINT32 BLFlashStoreLoad()
{
	WCHAR FileName[64] = {0};
	
    UINT32					rc			= BL_ERROR;
	int						fio_status;
	EXFILEHANDLE			File;
	PEXFILEHANDLE			pFile		= &File;
	S_EXFILEIO_OPERATIONS	fileio_ops;

	HANDLE			hFMD = 0;
    PCI_REG_INFO	regInfo;
    DWORD			Data = 0;
    DWORD			length = 0, dwLaunchAddr = 0;
	UINT8*			buf, *ptr;
    FlashInfo		finfo;
	BSP_ARGS*		pArgs = OALPAtoUA(IMAGE_SHARE_ARGS_PA);

	UINT32			type;

	OMAP_PRCM_GLOBAL_PRM_REGS * pPrmGlobal = OALPAtoUA(OMAP_PRCM_GLOBAL_PRM_REGS_PA);

	sw_update_state* pFlags = (sw_update_state*)&pArgs->SwUpdateFlags;
	OALMSG(OAL_INFO, (L"+BLFlashStoreLoad:\r\n"));

	g_bootCfg.bootDevLoc.LogicalLoc = BSP_NAND_REGS_PA + 0x20;//for the next boot
	BLWriteBootCfg(&g_bootCfg);

	// set up data structure used by file system driver
	fileio_ops.init			= 0;
	fileio_ops.identify		= 0;
	fileio_ops.read_sector	= &FAL_ReadSectors;
		
	regInfo.MemBase.Reg[0] = g_ulFlashBase;
	hFMD = FMD_Init(NULL, &regInfo, NULL);
	if (hFMD == NULL) 
    {
		pFlags->bits |= (SW_FLASH_ERROR | SW_END_UPDATE);
		pFlags->bits &= ~SW_START_UPDATE;
		g_bootCfg.SwUpdateFlags = 0;
		BLWriteBootCfg(&g_bootCfg);
		OALMSG(OAL_ERROR, (L"BLFlashStoreLoad: Oops, can't open FMD driver\r\n"));
		return rc;
    }
	FMD_GetInfo(&finfo);

	SetFMDNotify(0);
	g_eboot.type = DOWNLOAD_TYPE_RAM;

	buf = (UINT8*)IMAGE_DISPLAY_BUF_PA + (IMAGE_DISPLAY_BUF_SIZE>>1);

	OALMSG(OAL_INFO, (L"BLFlashStoreLoad:  OEMMapMemAddr 0x%x\r\n", buf));
	
	BLSetProgressData( NUMBER_PROGRESS_RECTANGLES, NUMBER_PROGRESS_RECTANGLES);  
	SetFalNotify(ProgressNotify);
	
	fileio_ops.drive_info = FAL_Init(hFMD, buf );

	// initialize file system driver
	if(ExFileIoInit(&fileio_ops) != FILEIO_STATUS_OK)
	{
		pFlags->bits |= (SW_FLASH_ERROR | SW_END_UPDATE);
		pFlags->bits &= ~SW_START_UPDATE;
		g_bootCfg.SwUpdateFlags = 0;
		BLWriteBootCfg(&g_bootCfg);
		OALMSG(OAL_ERROR, (L"BLFlashStoreLoad:  fileio init failed\r\n"));
		return rc;
	}
//	BLSetProgressData( NUMBER_PROGRESS_RECTANGLES, NUMBER_PROGRESS_RECTANGLES * 100);  
	
	/////
	for(type = sw_type_xldr; type <= sw_type_nk; type <<= 1)
	{
		//if current type update is not needed
		if(!(type & ((sw_update_state*)&g_bootCfg.SwUpdateFlags)->data_type))
			continue;
		FileName[0] = 0;
		ptr = IMAGE_WINCE_CODE_PA;
		switch(type)
		{
			case sw_type_xldr:
				wcscpy(FileName, L"XLDRNAND.bin");
			break;
			case sw_type_eboot:
				wcscpy(FileName, L"ebnand.bin");
			break;
			case sw_type_mnfr:
				wcscpy(FileName, L"Mnfr.bin");
			break;
			case sw_type_logo:
				wcscpy(FileName, L"logo.bmp");
			break;
			case sw_type_nk:
			{
				wcscpy(FileName, L"nk.bin");
				ptr = IMAGE_WINCE_CODES_PA;
			}
			break;
		}
		if(0 == FileName[0])
			continue;

		BLSetProgressData(0, 0);
//		ProgressNotify();//for the first
		BLSetProgressData(NUMBER_PROGRESS_RECTANGLES, NUMBER_PROGRESS_RECTANGLES * 100);//for FAL

		ExFileNameToDirEntry(FileName, pFile->name);

		// try to open file, return BL_ERROR on failure
		if((fio_status = ExFileIoOpen(&fileio_ops, pFile)) != FILEIO_STATUS_OK)
		{
			pFlags->bits			|= SW_CANNOT_OPEN;
			pFlags->err_data_type	= type;
			OALMSG(OAL_ERROR, (L"BLFlashStoreLoad:  cannot open file %s\r\n", pFile->name));
			break;
		}

		// Set address where to place image
	    g_pData	= ptr;

		if(ExFileIoRead(&fileio_ops, pFile, (PVOID)g_pData, pFile->file_size) != FILEIO_STATUS_OK)
		{
	 		
			pFlags->bits			|= SW_CANNOT_READ;
			pFlags->err_data_type	= type;
			OALMSG(OAL_ERROR, (L"BLFlashStoreLoad:  cannot read file %s\r\n", pFile->name));
			break;
		}

		OALMSG(OAL_INFO, (L"BLFlashStoreLoad:  read OK\r\n"));
		g_bootCfg.bootDevLoc.LogicalLoc = BSP_NAND_REGS_PA;//for now
		g_pData	= ptr;
		
		//FillProgress(0);
		
		BLSetProgressData(0, 0);
		//ProgressNotify();
		if(type == sw_type_nk)
			BLSetProgressData(NUMBER_PROGRESS_RECTANGLES, pFile->file_size/NUMBER_PROGRESS_RECTANGLES/finfo.wDataBytesPerSector);
		else
			BLSetProgressData(NUMBER_PROGRESS_RECTANGLES, 200);
		SetFMDNotify(ProgressNotify);//for write
		//ProgressNotify();//for the first
		
		if(sw_type_mnfr == type)
		{
			if(sizeof(manufacture_block) < pFile->file_size)
			{
				pFlags->bits			|= SW_DATA_ERROR;
				pFlags->err_data_type	= type;
				OALMSG(OAL_ERROR, (L"BLFlashStoreLoad:  manufacture block error size\r\n"));
				break;
			}
			//Copy the manufacture info after last sector of saved block in RAM
			memcpy(g_pData + finfo.dwBytesPerBlock, g_pData, MANUFACTURE_BLOCK_SIZE); 
			if( !BLWriteManufactureCfg(g_pData + finfo.dwBytesPerBlock) )
			{
				pFlags->bits			|= SW_DATA_ERROR;
				pFlags->err_data_type	= type;
				OALMSG(OAL_ERROR, (L"BLFlashStoreLoad:  BLWriteManufactureCfg error\r\n"));
				break;
			}

			OALMSG(OAL_INFO, (L"manufacture block written\r\n"));
		}
		else if(sw_type_logo == type)
		{
			if(!BLWriteBmp((UINT32)g_pData, pFile->file_size))
			{
				pFlags->bits			|= SW_DATA_ERROR;
				pFlags->err_data_type	= type;
				OALMSG(OAL_ERROR, (L"BLFlashStoreLoad:  BLWriteBmp error\r\n"));
				break;
			}
		}
		else 
		{
			if (0 != memcmp (g_pData, "B000FF\x0A", BL_HDRSIG_SIZE))
			{
				pFlags->bits			|= SW_DATA_ERROR;
				pFlags->err_data_type	= type;
				OALMSG(OAL_ERROR, (L"BLFlashStoreLoad:  bin signature error\r\n"));
 				break;
			}
			ResetGlobalVariables();
			if(!DownloadImage( (LPDWORD)&Data, &length, &dwLaunchAddr ) )
			{
				pFlags->bits			|= SW_DATA_ERROR;
				pFlags->err_data_type	= type;
				OALMSG(OAL_ERROR, (L"BLFlashStoreLoad:  DownloadImage error\r\n"));
 				break;
			}
		}
		
		//((sw_update_state*)&g_bootCfg.SwUpdateFlags)->data_type &= ~type;
		//BLWriteBootCfg(&g_bootCfg);
	///////////////////////////////////////////////////////////////////////////////
		if(type < sw_type_nk)
		{
			//reinit FMD and FAL(without pa2la table)
			regInfo.MemBase.Reg[0] = g_ulFlashBase;
			hFMD = FMD_Init(NULL, &regInfo, NULL);
			if (hFMD == NULL) 
			{
				pFlags->bits |= SW_FLASH_ERROR;
				OALMSG(OAL_ERROR, (L"BLFlashStoreLoad: Oops, can't open FMD driver\r\n"));
				break;
			}
			FMD_GetInfo(&finfo);
			SetFMDNotify(0);
			FAL_Init(hFMD, buf );
		}
	}//////	
	
	pFlags->bits &= ~SW_START_UPDATE;
	pFlags->bits |= SW_END_UPDATE;

	rc = BL_JUMP;

	g_bootCfg.bootDevLoc.LogicalLoc = BSP_NAND_REGS_PA + 0x20;////for the next boot
	g_bootCfg.SwUpdateFlags = 0;
	BLWriteBootCfg(&g_bootCfg);

//	OALMSG(OAL_INFO, (L"BLFlashStoreLoad: Download Image binary: completed. Status = %x.\r\n",  rval));

	FillProgress(-1);	

	// Do warm reset
	OUTREG32(&pPrmGlobal->PRM_RSTCTRL, /*RSTCTRL_RST_DPLL3|*/ RSTCTRL_RST_GS);

	return rc;
}

#endif

UINT32 OALEraseEntireFlash(UINT16 Param)
{
	//    WCHAR key;
	HANDLE hFMD = NULL;
	PCI_REG_INFO regInfo;
	FlashInfo flashInfo;
	BLOCK_ID block;
	UINT32 status;
	UINT32 listmode=0;

	// Open FMD
	//g_ecctype = Hamming1bit;
	regInfo.MemBase.Reg[0] = g_ulFlashBase;
	hFMD = FMD_Init(NULL, &regInfo, NULL);
	if (hFMD == NULL) 
	{
		OALMSG(OAL_ERROR, (L"Can't open FMD driver\r\n"));
		return 0;
	}

	if(!FMD_GetInfo(&flashInfo)) 
	{
		FMD_Deinit(hFMD);
		OALMSG(OAL_ERROR, (L"Can't get flash geometry info\r\n"));
		return 0;
	}

	// First offset given
	block = 0;
	while (block < flashInfo.dwNumBlocks) 
	{
		// If block is bad, we have to offset it
		status = FMD_GetBlockStatus(block);

		// Skip bad blocks
		if((status & BLOCK_STATUS_BAD) != 0) 
		{
			OALLog(L"Bad block %d\r\n", block);
			//block++;
			//continue;
		}

		// Skip reserved blocks
		if((status & BLOCK_STATUS_RESERVED) != 0) 
		{
#ifndef SHIP_BUILD
			OALLog(L"Reserved block %d\r\n", block);
#endif
			if(1 != Param)
			{
				block++;
				continue;
			}
		}

		// Erase block
		if(!FMD_EraseBlock(block)) 
		{
#ifndef SHIP_BUILD
			OALLog(L"Oops, can't erase block %d - mark as bad\r\n", block);
#endif
			FMD_SetBlockStatus(block, BLOCK_STATUS_BAD);
		}

		block++;
	}

#ifndef SHIP_BUILD
	OALLog(L" Done\r\n");
#endif

	if(hFMD != NULL)
		FMD_Deinit(hFMD);

	return 1;
}
