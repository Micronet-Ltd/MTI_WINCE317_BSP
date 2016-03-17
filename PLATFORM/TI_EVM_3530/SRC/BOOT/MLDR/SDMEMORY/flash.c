/*
================================================================================

 Copyright (c) 2008, 2013 Micronet LTD. All rights reserved.

 History of Changes:
 Vladimir Zatulovsky CE500
 Igor Lantsman UI
 Vladimir Zatulovsky CE300/CE317
================================================================================
*/
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


#define MLDR_PATTERN_T  L"\r\nD:\\>"
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
//  Local Functions

static UINT32 ReadFlashNK();
static BOOL WriteFlashXLDR(UINT32 address, UINT32 size);
static BOOL WriteFlashEBOOT(UINT32 address, UINT32 size);
static BOOL WriteFlashNK(UINT32 address, UINT32 size);
static VOID DumpTOC(ROMHDR *pTOC);
static BOOL VerifyImage(UCHAR *pData, ROMHDR **ppTOC);
static BOOL BLWriteBmp( UINT32 address, UINT32 DataLength );
BOOL	DownloadBmp();

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
UINT32 BLFlashDownload(BOOT_CFG *pConfig, OAL_KITL_DEVICE *pBootDevices)
{
    UINT32 rc = BL_ERROR;

    // We have do device initialization for some devices
    switch (pConfig->bootDevLoc.IfcType)
        {
        case Internal:
            switch (pConfig->bootDevLoc.LogicalLoc)
                {
                case BSP_NAND_REGS_PA + 0x20:
                rc = ReadFlashNK();
                break;
                }
            break;
        }

    return rc;
}

//------------------------------------------------------------------------------

static UINT32 ReadFlashNK()
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

    // Read remainder of image
    offset += size;
    size = pTOC->physlast - pTOC->physfirst - offset;
	if (!BP_ReadData(hPartition, pData + offset, size))
    {
        OALMSG(OAL_ERROR, (L"ERROR: "
            L"BP_ReadData call failed!\r\n"
            ));
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
//
//  Function:  OEMStartEraseFlash
//
//  This function is called by the bootloader to initiate the flash memory
//  erasing process.
//
BOOL OEMStartEraseFlash(ULONG address, ULONG size)
{
    BOOL rc = TRUE;

    OALMSG(OAL_FUNC, (L"+OEMStartEraseFlash(0x%08x, 0x%08x)\r\n", address, size));
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
VOID OEMContinueEraseFlash()
{
}


//------------------------------------------------------------------------------
//
//  Function:  OEMFinishEraseFlash
//
//  This function is called by the bootloader to finish flash erase before
//  it will call OEMWriteFlash.
//
BOOL OEMFinishEraseFlash()
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
BOOL OEMWriteFlash(ULONG address, ULONG size)
{
    BOOL rc;

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
		case DOWNLOAD_TYPE_BMP:
			rc = BLWriteBmp(address, size);
			break;
        default:
            rc = FALSE;
            break;
        }

    return rc;
}

//------------------------------------------------------------------------------

BOOL WriteFlashXLDR(UINT32 address, UINT32 size)
{
    BOOL rc = FALSE;
    HANDLE hFlash = NULL;
    ROMHDR *pTOC;
    UINT32 offset, xldrSize, blocknum, blocksize;
    UINT8 *pData;


    OALMSG(1, (MLDR_PATTERN_T L"Burning XLDR image to flash memory.\r\n"));
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

	OALMSG(1, (MLDR_PATTERN_T L"XLDR image burning succeeded. \r\n"));

    // Done
    rc = TRUE;

cleanUp:
    if (hFlash != NULL) OALFlashStoreClose(hFlash);
	if (!rc) OALMSG(1, (MLDR_PATTERN_T L"XLDR image burning failed.\r\n"));
    return rc;
}

//------------------------------------------------------------------------------

BOOL WriteFlashEBOOT(UINT32 address, UINT32 size)
{
    BOOL rc = FALSE;
    HANDLE hFlash = NULL;
    UINT8 *pData;
    UINT32 offset;

	UINT8* buffer = (UINT8*)IMAGE_WINCE_CODE_PA;
	OALMSG(1, (MLDR_PATTERN_T L"Burning EBOOT image to flash memory.\r\n"));
   // OALMSG(OAL_INFO, (L"\r\nWriting EBOOT image to flash memory\r\n"));

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

    //OALMSG(OAL_INFO, (L"EBOOT image written\r\n"));
	OALMSG(1, (MLDR_PATTERN_T L"EBOOT image burning succeeded. \r\n"));

    // Done
    rc = TRUE;

cleanUp:
    if (hFlash != NULL) OALFlashStoreClose(hFlash);
	if (!rc) OALMSG(1, (MLDR_PATTERN_T L"EBOOT image burning failed. \r\n"));
    return rc;
}

//------------------------------------------------------------------------------

BOOL WriteFlashNK(UINT32 address, UINT32 size)
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


	OALLog(MLDR_PATTERN_T L"Burning NK image to flash memory.\r\n");

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
	

    // Change boot device to NAND
    g_bootCfg.bootDevLoc.IfcType = Internal;
    g_bootCfg.bootDevLoc.LogicalLoc = BSP_NAND_REGS_PA + 0x20;

	OALLog(MLDR_PATTERN_T L"NK image burning succeded. \r\n");
	OALLog(MLDR_PATTERN_T L"UPDATE_COMPLETED: succeded. \r\n");

    // Done
    rc = TRUE;

cleanUp:
	if (!rc) 
	{
		OALLog(MLDR_PATTERN_T L"NK image burning failed. \r\n");
		OALLog(MLDR_PATTERN_T L"UPDATE_COMPLETED: failed. \r\n");
	}

	
    return rc;
}

//------------------------------------------------------------------------------

VOID DumpTOC(ROMHDR *pTOC)
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

BOOL VerifyImage(UCHAR *pData, ROMHDR **ppTOC)
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
    	OALLog(L"OS partition does not exist!!\r\n");
	else
	{
		pPartitionEntry = BP_GetPartitionInfo(hPartition);
		if (dwBootPartitionSectorCount != pPartitionEntry->Part_TotalSectors)
		{
    		OALLog(L"OS partition does not match configured size!!  Sector count expected: 0x%x, actual 0x%x\r\n", dwBootPartitionSectorCount, pPartitionEntry->Part_TotalSectors);
			// Mark handle invalid to kick us into formatting code
			hPartition = INVALID_HANDLE_VALUE;
		}
	}
		
	if ((hPartition == INVALID_HANDLE_VALUE) || (bForceEnable == TRUE))
	{
		// OS binary partition either does not exist or does not match configured size
    	OALLog(L"Formatting flash...\r\n");
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
#if 1
		// Create FAT partition on remaining flash (can be automatically mounted)
    	hPartition = BP_OpenPartition(NEXT_FREE_LOC, USE_REMAINING_SPACE, PART_DOS32, FALSE, PART_CREATE_NEW);
		if (hPartition == INVALID_HANDLE_VALUE)
		{
    		OALLog(L"Error creating file partition!!\r\n");
			goto cleanUp;
		}
#endif
    	OALLog(L"Flash format complete!\r\n");
	}
	
	// Done
	rc = TRUE;

cleanUp:
	return rc;
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
//------------------------------------------------------------------------------

BOOL DownloadBmp()
{
	UINT32 size = 0;

	if( size = BLSDCardReadFileData(IMAGE_BOOTLOADER_BITMAP_SIZE, OALPAtoCA(IMAGE_WINCE_CODE_PA)))
	{
		g_eboot.type = DOWNLOAD_TYPE_BMP;
		if( OEMWriteFlash((ULONG)OALPAtoCA(IMAGE_WINCE_CODE_PA), size ) )
			return 1;
	}

	return 0;
}
UINT32 EraseFlash()
{
    HANDLE hFMD = NULL;
    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;
    BLOCK_ID block;
    UINT32 status;
    UINT32 listmode=0;

    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    hFMD = FMD_Init(NULL, &regInfo, NULL);
    if (hFMD == NULL) 
	{
		OALLog(L"Can't open FMD driver\r\n");
		return 0;
	}

    if(!FMD_GetInfo(&flashInfo)) 
    {
		FMD_Deinit(hFMD);
		OALLog(L"Can't get flash geometry info\r\n");
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
            OALLog(L"Reserved block %d\r\n", block);
            //block++;
            //continue;
		}

        // Erase block
        if(!FMD_EraseBlock(block)) 
		{
            OALLog(L"Oops, can't erase block %d - mark as bad\r\n", block);
            FMD_SetBlockStatus(block, BLOCK_STATUS_BAD);
		}

        block++;
    }

	FMD_Deinit(hFMD);

	OALLog(L" Done\r\n");

	return 1;
}
void EraseEntireFlash(UINT32 fSaveParams)
{

//    WCHAR key;
    HANDLE hFMD = NULL;
    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;
    BLOCK_ID block;
    UINT32 status;
    UINT32 listmode=0;
	UINT8* buffer = (UINT8*)IMAGE_WINCE_CODE_PA;
	UCHAR	from_ecc_type	= 1;
	UCHAR	to_ecc_type		= 1; 	
	// Open FMD
///////////////////////////////
	if(0 != fSaveParams)
	{
		regInfo.MemBase.Reg[0] = g_ulFlashBase;
		hFMD = FMD_Init(NULL, &regInfo, NULL);
		if (hFMD == NULL) 
		{
			OALLog(L"Can't open FMD driver\r\n");
			return;
		}
		if(!FMD_GetInfo(&flashInfo)) 
		{
			FMD_Deinit(hFMD);
			OALLog(L"Can't get flash geometry info\r\n");
			return;
		}
		if (!BLReadBootCfgSectors(buffer, 3 * flashInfo.wDataBytesPerSector))
		{
			FMD_Deinit(hFMD);
			OALMSG(OAL_ERROR, (L"ERROR: OALEraseEntireFlash: couldn't read boot configuration/manufacture sectors\r\n"));
			return;
		}
		FMD_Deinit(hFMD);
	}
/////////////////////////////////////////
	EraseFlash();
	
///////////////////////////////
	if(0 != fSaveParams)
	{
		regInfo.MemBase.Reg[0] = g_ulFlashBase;
		hFMD = FMD_Init(NULL, &regInfo, NULL);
		if (hFMD == NULL) 
		{
			OALLog(L"Can't open FMD driver\r\n");
			return;
		}
		if(!FMD_GetInfo(&flashInfo)) 
		{
			FMD_Deinit(hFMD);
			OALLog(L"Can't get flash geometry info\r\n");
			return;
		}

		if (!BLWriteBootCfgSectors(buffer, 3 * flashInfo.wDataBytesPerSector))
		{
			FMD_Deinit(hFMD);
			OALMSG(OAL_ERROR, (L"ERROR: OALEraseEntireFlash: couldn't write boot configuration/manufacture sectors\r\n"));
			return;
		}
		BLReadBootCfg(&g_bootCfg);
		g_bootCfg.osPartitionSize = 0x02FFF000;//for old versions 
		BLWriteBootCfg(&g_bootCfg);

		FMD_Deinit(hFMD);
	}
/////////////////////////////////////////
	OALMSG(1, (L"-OALEraseEntireFlash\r\n"));
    return;
}



