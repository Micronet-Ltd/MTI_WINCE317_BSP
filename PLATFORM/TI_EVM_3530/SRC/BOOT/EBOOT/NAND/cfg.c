// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.
// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
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
//  File:  cfg.c
//
//  This file implements functions used to load/save EBOOT configuration info.
//  The EBOOT configuration is located on last block reserved for EBOOT image
//  on NAND flash memory.
//
#include <eboot.h>
#include <BootCmd.h>
#include <bsp_logo.h>

//------------------------------------------------------------------------------

BOOL
BLReadBootCfg(
    BOOT_CFG *pBootCfg
    )
{
    BOOL rc = FALSE;
    HANDLE hFMD;
    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;
    SectorInfo sectorInfo;
    SECTOR_ADDR sector;
    BLOCK_ID block;
    UINT32 count, offset;
    UINT8 buffer[2048];
    

    // EBOOT configuration is placed in last sector of EBOOT image
    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    hFMD = FMD_Init(NULL, &regInfo, NULL);
    if (hFMD == NULL)
        {
        OALMSG(OAL_ERROR, (L"ERROR: FMD_Init call failed!\r\n"));
        goto cleanUp;
        }

    // Get flash info
    if (!FMD_GetInfo(&flashInfo))
        {
        OALMSG(OAL_ERROR, (L"ERROR: FMD_GetInfo call failed!\r\n"));
        goto cleanUp;
        }

    // We can support only flash with sector size < 2048 bytes
    if (flashInfo.wDataBytesPerSector > sizeof(buffer))
        {
        OALMSG(OAL_ERROR, (L"ERROR: "
            L"Flash sector size %d bytes bigger than supported %d bytes\r\n",
            flashInfo.wDataBytesPerSector, sizeof(buffer)    
            ));
        goto cleanUp;
        }

    // Configuration is located in last sector of EBOOT image
    offset = IMAGE_XLDR_BOOTSEC_NAND_SIZE + IMAGE_EBOOT_BOOTSEC_NAND_SIZE;
    
    // Start from beginning
    block  = 0;
    sector = 0;

    // Skip X-Loader & EBOOT code & bad blocks
	// Note that we also check the last eboot block in order to ensure it is good
    count = 0;
    while ((count < offset) && (block < flashInfo.dwNumBlocks))
        {
        if ((FMD_GetBlockStatus(block) & BLOCK_STATUS_BAD) == 0)
            {
            count += flashInfo.dwBytesPerBlock;
            }
        block++;
        }

	// We've incremented past the last eboot block in order to check it too
	// Back up now, the previous block is the last one containing eboot and is good
	block--;

    //  Compute sector within the block where config lies
	sector = block * flashInfo.wSectorsPerBlock;
    sector += flashInfo.wSectorsPerBlock - 1;

    // Read sector to buffer
    if (!FMD_ReadSector(sector, buffer, &sectorInfo, 1)) {
        OALMSG(OAL_ERROR, (L"ERROR: EBOOT!BLReadBootCfg: "
            L"Flash sector %d read failed\r\n", sector
        ));
            goto cleanUp;
    }

    // Copy data to BOOT_CFG structure
    memcpy(pBootCfg, buffer, sizeof(BOOT_CFG));        

    // Done    
    rc = TRUE;

cleanUp:
    if (hFMD != NULL) FMD_Deinit(hFMD);
    return rc;
}

//------------------------------------------------------------------------------

BOOL
BLWriteBootCfg(
    BOOT_CFG *pBootCfg
    )
{
    BOOL rc = FALSE;
    HANDLE hFMD;
    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;
    SectorInfo sectorInfo;
    SECTOR_ADDR sector;
    BLOCK_ID block;
    UINT32 count, offset, length;
    UINT8 buffer[2048];
    UINT8 *pEBOOT;
    

    // EBOOT configuration is placed in last sector of image
    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    hFMD = FMD_Init(NULL, &regInfo, NULL);
    if (hFMD == NULL)
        {
        OALMSG(OAL_ERROR, (L"ERROR: FMD_Init call failed!\r\n"));
        goto cleanUp;
        }

    // Get flash info
    if (!FMD_GetInfo(&flashInfo))
        {
        OALMSG(OAL_ERROR, (L"ERROR: FMD_GetInfo call failed!\r\n"));
        goto cleanUp;
        }

    // We can support only flash with sector size which fit to our buffer
    if (flashInfo.wDataBytesPerSector > sizeof(buffer))
        {
        OALMSG(OAL_ERROR, (L"ERROR: "
            L"Flash sector size %d bytes bigger that supported %d bytes\r\n",
            flashInfo.wDataBytesPerSector, sizeof(buffer)    
            ));
        goto cleanUp;
        }

    // Configuration is located in last sector of last EBOOT block
    offset = IMAGE_XLDR_BOOTSEC_NAND_SIZE + IMAGE_EBOOT_BOOTSEC_NAND_SIZE;
    
    // Skip X-Loader & EBOOT code & bad blocks
	// Note that we also check the last eboot block in order to ensure it is good
    block  = 0;
    count = 0;
    while ((count < offset) && (block < flashInfo.dwNumBlocks))
        {
        if ((FMD_GetBlockStatus(block) & BLOCK_STATUS_BAD) == 0)
            {
            count += flashInfo.dwBytesPerBlock;
            }
        block++;
        }

	// We've incremented past the last eboot block in order to check it too
	// Back up now, the previous block is the last one containing eboot and is good
	block--;

    // Need to copy off the block contents to RAM (minus the config sector)
    pEBOOT = (UINT8*)IMAGE_WINCE_CODE_PA;
    length = flashInfo.dwBytesPerBlock - flashInfo.wDataBytesPerSector;

    memset((VOID*)pEBOOT, 0xFF, flashInfo.dwBytesPerBlock);
	sector = block * flashInfo.wSectorsPerBlock;
    offset = 0;

	OALMSG(0, (L"EBOOT!BLWriteBootCfg: wDataBytesPerSector %x, wSectorsPerBlock =%x sizeof (BOOT_CFG) = %x \r\n",
		flashInfo.wDataBytesPerSector,flashInfo.wSectorsPerBlock,sizeof (BOOT_CFG)));
   

    while (offset < length) 
    {
        // When block read fail, there isn't what we can do more
        if (!FMD_ReadSector(sector, pEBOOT + offset, &sectorInfo, 1)) {
                OALMSG(OAL_ERROR, (L"\r\nERROR: EBOOT!BLWriteBootCfg: "
                    L"Failed read sector %d from flash\r\n", sector
                ));
            goto cleanUp;
            }

        // Move to next sector
        sector++;
        offset += flashInfo.wDataBytesPerSector;
    }


    //  Copy the config info into last sector of saved block in RAM
    memcpy(pEBOOT + offset, pBootCfg, sizeof(BOOT_CFG)); 


    // Erase block
    if (!FMD_EraseBlock(block))
        {
        OALMSG(OAL_ERROR, (L"ERROR: EBOOT!BLWriteBootCfg: "
            L"Flash block %d erase failed\r\n", block
            ));
        goto cleanUp;
        }


    // Write contents of the save block + config sector back to flash
    pEBOOT = (UINT8*)IMAGE_WINCE_CODE_PA;
    length = flashInfo.dwBytesPerBlock;

	sector = block * flashInfo.wSectorsPerBlock;
    offset = 0;
    while (offset < length)
    {
        // Prepare sector info
        memset(&sectorInfo, 0xFF, sizeof(sectorInfo));
        sectorInfo.bOEMReserved &= ~(OEM_BLOCK_READONLY|OEM_BLOCK_RESERVED);
        sectorInfo.dwReserved1 = 0;
        sectorInfo.wReserved2 = 0;

        // Write sector        
        if (!FMD_WriteSector(sector, pEBOOT + offset, &sectorInfo, 1))
            {
            OALMSG(OAL_ERROR, (L"ERROR: EBOOT!BLWriteBootCfg: "
                L"Flash sector %d write failed\r\n", sector
                ));
            goto cleanUp;
            }

        // Move to next sector
        sector++;
        offset += flashInfo.wDataBytesPerSector;
    }

    // Done    
    rc = TRUE;

cleanUp:
    if (hFMD != NULL) FMD_Deinit(hFMD);
    return rc;
}

//------------------------------------------------------------------------------

BOOL
BLReserveBootBlocks(
    BOOT_CFG *pBootCfg
    )
{
    BOOL rc = FALSE;
    HANDLE hFMD;
    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;
    UINT32 size;
    BLOCK_ID firstblock, lastblock;
    UINT32 status;
    

    // Automatically mark the bootloader blocks as read-only/reserved
    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    hFMD = FMD_Init(NULL, &regInfo, NULL);
    if (hFMD == NULL)
        {
        OALMSG(OAL_ERROR, (L"ERROR: FMD_Init call failed!\r\n"));
        goto cleanUp;
        }

    // Get flash info
    if (!FMD_GetInfo(&flashInfo))
        {
        OALMSG(OAL_ERROR, (L"ERROR: FMD_GetInfo call failed!\r\n"));
        goto cleanUp;
        }


    //  Loop thru the bootloader blocks to ensure they are marked reserved
    firstblock = 0;
    size = IMAGE_BOOTLOADER_NAND_SIZE;
    lastblock = ((size -1) / flashInfo.dwBytesPerBlock) + 1;
#ifndef SHIP_BUILD
    OALLog(L"Checking bootloader blocks are marked as reserved (Num = %d)\r\n", lastblock-firstblock);
#endif

    while (firstblock < lastblock)
        {

        // If block is bad, we have to offset it
        status = FMD_GetBlockStatus(firstblock);

        // Skip bad blocks
        if(status & BLOCK_STATUS_BAD) 
		{
#ifndef SHIP_BUILD
            OALLog(L" Skip bad block %d\r\n", firstblock);
#endif
			// blocks marked bad would not have been written either, so don't include this 
			// in the count of blocks that are reserved.
            firstblock++;
			lastblock++;
			if(lastblock > flashInfo.dwNumBlocks)
				break;
            continue;
		}

        // Skip already reserved blocks
        if(status & BLOCK_STATUS_RESERVED) 
        {
#ifndef SHIP_BUILD
            OALLog(L" Skip reserved block %d\r\n", firstblock);
#endif
            firstblock++;
            continue;
		}

        // Mark block as read-only & reserved
        if (!FMD_SetBlockStatus(firstblock, BLOCK_STATUS_READONLY|BLOCK_STATUS_RESERVED)) 
            {
            OALLog(L" Oops, can't mark block %d - as reserved\r\n", firstblock);
		}

#ifndef SHIP_BUILD
        OALLog(L"reserved %d", firstblock);
#endif
        firstblock++;
	}

    // Done    
    rc = TRUE;

#ifndef SHIP_BUILD
    OALLog(L"\r\n");
#endif

cleanUp:
    if (hFMD != NULL) FMD_Deinit(hFMD);
    return rc;
}

//------------------------------------------------------------------------------

BOOL
BLShowLogo(
    )
{
    //  Show the bootloader splashscreen
    ShowLogo(g_ulFlashBase, IMAGE_XLDR_BOOTSEC_NAND_SIZE + IMAGE_EBOOT_BOOTSEC_NAND_SIZE);

    return TRUE;
}

//------------------------------------------------------------------------------


BOOL   BLShowProgress()
{
	// Display the bootloader progress
	return ShowProgress();

}

//------------------------------------------------------------------------------

BOOL   BLSetProgressData(DWORD noOfRectangles,DWORD stepsNo)
{
	return SetProgressData(noOfRectangles,stepsNo);
}



//------------------------------------------------------------------------------



BOOL
BLReadManufactureCfg(
    BYTE *pManufCfg
    )
{
    BOOL rc = FALSE;
    HANDLE hFMD;
    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;
    SectorInfo sectorInfo;
    SECTOR_ADDR sector;
    BLOCK_ID block;
    UINT32 count, offset;
    UINT8 buffer[2048];
    
	// eboot configuration is placed in the last sector of EBOOT image
	// manufacture block is placed in last sectors before eboot configuration sector

    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    hFMD = FMD_Init(NULL, &regInfo, NULL);
    if (hFMD == NULL)
        {
        OALMSG(OAL_ERROR, (L"ERROR: FMD_Init call failed!\r\n"));
        goto cleanUp;
        }

    // Get flash info
    if (!FMD_GetInfo(&flashInfo))
        {
        OALMSG(OAL_ERROR, (L"ERROR: FMD_GetInfo call failed!\r\n"));
        goto cleanUp;
        }

    // We can support only flash with sector size < 2048 bytes
    if (flashInfo.wDataBytesPerSector > sizeof(buffer))
        {
        OALMSG(OAL_ERROR, (L"ERROR: "
            L"Flash sector size %d bytes bigger than supported %d bytes\r\n",
            flashInfo.wDataBytesPerSector, sizeof(buffer)    
            ));
        goto cleanUp;
        }

    // Configuration is located in last sector of EBOOT image
    offset = IMAGE_XLDR_BOOTSEC_NAND_SIZE + IMAGE_EBOOT_BOOTSEC_NAND_SIZE;
    
    // Start from beginning
    block  = 0;
    sector = 0;

    // Skip X-Loader & EBOOT code & bad blocks
	// Note that we also check the last eboot block in order to ensure it is good
    count = 0;
    while ((count < offset) && (block < flashInfo.dwNumBlocks))
        {
        if ((FMD_GetBlockStatus(block) & BLOCK_STATUS_BAD) == 0)
            {
            count += flashInfo.dwBytesPerBlock;
            }
        block++;
        }

	// We've incremented past the last eboot block in order to check it too
	// Back up now, the previous block is the last one containing eboot and is good
	block--;

    //  Compute sector within the block where manufacture lies
	sector = block * flashInfo.wSectorsPerBlock;
    sector += flashInfo.wSectorsPerBlock - 2;

    // Read 2 sectors to buffer
    if (!FMD_ReadSector(sector, buffer, &sectorInfo, 1)) {
        OALMSG(OAL_ERROR, (L"ERROR: EBOOT!BLReadBootCfg: "
            L"Flash sector %d read failed\r\n", sector
        ));
            goto cleanUp;
    }

	

    // Copy data to BOOT_CFG structure
    memcpy(pManufCfg, buffer, MANUFACTURE_BLOCK_SIZE);        

    // Done    
    rc = TRUE;

cleanUp:
    if (hFMD != NULL) FMD_Deinit(hFMD);
    return rc;
}

//------------------------------------------------------------------------------

BOOL
BLWriteManufactureCfg(
    BYTE *pManufCfg
    )
{
    BOOL rc = FALSE;
    HANDLE hFMD;
    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;
    SectorInfo sectorInfo;
    SECTOR_ADDR sector;
    BLOCK_ID block;
    UINT32 count, offset, length;
    UINT8 buffer[2048];
    UINT8 *pEBOOT;
    BSP_ARGS *pArgs = OALPAtoUA(IMAGE_SHARE_ARGS_PA);


    // EBOOT configuration is placed in last sector of image
    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    hFMD = FMD_Init(NULL, &regInfo, NULL);
    if (hFMD == NULL)
        {
        OALMSG(OAL_ERROR, (L"ERROR: FMD_Init call failed!\r\n"));
        goto cleanUp;
        }

    // Get flash info
    if (!FMD_GetInfo(&flashInfo))
        {
        OALMSG(OAL_ERROR, (L"ERROR: FMD_GetInfo call failed!\r\n"));
        goto cleanUp;
        }

    // We can support only flash with sector size which fit to our buffer
    if (flashInfo.wDataBytesPerSector > sizeof(buffer))
        {
        OALMSG(OAL_ERROR, (L"ERROR: "
            L"Flash sector size %d bytes bigger that supported %d bytes\r\n",
            flashInfo.wDataBytesPerSector, sizeof(buffer)    
            ));
        goto cleanUp;
        }

    // Configuration is located in last sector of last EBOOT block
    offset = IMAGE_XLDR_BOOTSEC_NAND_SIZE + IMAGE_EBOOT_BOOTSEC_NAND_SIZE;
    
    // Skip X-Loader & EBOOT code & bad blocks
	// Note that we also check the last eboot block in order to ensure it is good
    block  = 0;
    count = 0;
    while ((count < offset) && (block < flashInfo.dwNumBlocks))
        {
        if ((FMD_GetBlockStatus(block) & BLOCK_STATUS_BAD) == 0)
            {
            count += flashInfo.dwBytesPerBlock;
            }
        block++;
        }

	// We've incremented past the last eboot block in order to check it too
	// Back up now, the previous block is the last one containing eboot and is good
	block--;

    // Need to copy off the block contents to RAM 
    pEBOOT = (UINT8*)IMAGE_WINCE_CODE_PA;
    length = flashInfo.dwBytesPerBlock;

    memset((VOID*)pEBOOT, 0xFF, flashInfo.dwBytesPerBlock);
	sector = block * flashInfo.wSectorsPerBlock;
    offset = 0;

	OALMSG(0, (L"EBOOT!BLWriteBootCfg: wDataBytesPerSector %x, wSectorsPerBlock =%x sizeof (BOOT_CFG) = %x \r\n",
		flashInfo.wDataBytesPerSector,flashInfo.wSectorsPerBlock,sizeof (BOOT_CFG)));
   

    while (offset < length) 
    {
        // When block read fail, there isn't what we can do more
        if (!FMD_ReadSector(sector, pEBOOT + offset, &sectorInfo, 1)) {
                OALMSG(OAL_ERROR, (L"\r\nERROR: EBOOT!BLWriteBootCfg: "
                    L"Failed read sector %d from flash\r\n", sector
                ));
            goto cleanUp;
            }

        // Move to next sector
        sector++;
        offset += flashInfo.wDataBytesPerSector;
    }


	offset = flashInfo.wDataBytesPerSector*(flashInfo.wSectorsPerBlock -2);

    //  Copy the manufacture info into before last sector of saved block in RAM
    memcpy(pEBOOT + offset, pManufCfg, MANUFACTURE_BLOCK_SIZE); 


    // Erase block
    if (!FMD_EraseBlock(block))
        {
        OALMSG(OAL_ERROR, (L"ERROR: EBOOT!BLWriteBootCfg: "
            L"Flash block %d erase failed\r\n", block
            ));
        goto cleanUp;
        }


    // Write contents of the save block + config sector back to flash
    pEBOOT = (UINT8*)IMAGE_WINCE_CODE_PA;
    length = flashInfo.dwBytesPerBlock;

	sector = block * flashInfo.wSectorsPerBlock;
    offset = 0;
    while (offset < length)
    {
        // Prepare sector info
        memset(&sectorInfo, 0xFF, sizeof(sectorInfo));
        sectorInfo.bOEMReserved &= ~(OEM_BLOCK_READONLY|OEM_BLOCK_RESERVED);
        sectorInfo.dwReserved1 = 0;
        sectorInfo.wReserved2 = 0;

        // Write sector        
        if (!FMD_WriteSector(sector, pEBOOT + offset, &sectorInfo, 1))
            {
            OALMSG(OAL_ERROR, (L"ERROR: EBOOT!BLWriteBootCfg: "
                L"Flash sector %d write failed\r\n", sector
                ));
            goto cleanUp;
            }

        // Move to next sector
        sector++;
        offset += flashInfo.wDataBytesPerSector;
    }

    // Done    
    rc = TRUE;
	memcpy(pArgs->ManufBlock, pManufCfg, MANUFACTURE_BLOCK_SIZE);//copy to oal args
cleanUp:
    if (hFMD != NULL) FMD_Deinit(hFMD);
    return rc;
}


//------------------------------------------------------------------------------



BOOL
BLReadSWRights(
    BYTE *pSWRights
    )
{
    BOOL rc = FALSE;
    HANDLE hFMD;
    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;
    SectorInfo sectorInfo;
    SECTOR_ADDR sector;
    BLOCK_ID block;
    UINT32 count, offset;
    UINT8 buffer[2048];
    
	// sw rights sector is placed in the 3-th last sector of EBOOT image
	// before manufacture sector and eboot configuration sector

    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    hFMD = FMD_Init(NULL, &regInfo, NULL);
    if (hFMD == NULL)
        {
        OALMSG(OAL_ERROR, (L"ERROR: FMD_Init call failed!\r\n"));
        goto cleanUp;
        }

    // Get flash info
    if (!FMD_GetInfo(&flashInfo))
        {
        OALMSG(OAL_ERROR, (L"ERROR: FMD_GetInfo call failed!\r\n"));
        goto cleanUp;
        }

    // We can support only flash with sector size < 2048 bytes
    if (flashInfo.wDataBytesPerSector > sizeof(buffer))
        {
        OALMSG(OAL_ERROR, (L"ERROR: "
            L"Flash sector size %d bytes bigger than supported %d bytes\r\n",
            flashInfo.wDataBytesPerSector, sizeof(buffer)    
            ));
        goto cleanUp;
        }

    // sw rights sector  is located in the 3-th  last sector of EBOOT image
    offset = IMAGE_XLDR_BOOTSEC_NAND_SIZE + IMAGE_EBOOT_BOOTSEC_NAND_SIZE;
    
    // Start from beginning
    block  = 0;
    sector = 0;

    // Skip X-Loader & EBOOT code & bad blocks
	// Note that we also check the last eboot block in order to ensure it is good
    count = 0;
    while ((count < offset) && (block < flashInfo.dwNumBlocks))
        {
        if ((FMD_GetBlockStatus(block) & BLOCK_STATUS_BAD) == 0)
            {
            count += flashInfo.dwBytesPerBlock;
            }
        block++;
        }

	// We've incremented past the last eboot block in order to check it too
	// Back up now, the previous block is the last one containing eboot and is good
	block--;

    //  Compute the sector within the block where sw rights lies
	sector = block * flashInfo.wSectorsPerBlock;
    sector += flashInfo.wSectorsPerBlock - 3;

    // Read 1 sector to the buffer
    if (!FMD_ReadSector(sector, buffer, &sectorInfo, 1)) {
        OALMSG(OAL_ERROR, (L"ERROR: EBOOT!BLReadSWRights: "
            L"Flash sector %d read failed\r\n", sector
        ));
            goto cleanUp;
    }

	

    // Copy data to BOOT_CFG structure
    memcpy(pSWRights, buffer, SW_RIGHTS_BLOCK_SIZE);        

    // Done    
    rc = TRUE;

cleanUp:
    if (hFMD != NULL) FMD_Deinit(hFMD);
    return rc;
}

//------------------------------------------------------------------------------

BOOL
BLWriteSWRights(
    BYTE *pSWRights
    )
{
    BOOL rc = FALSE;
    HANDLE hFMD;
    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;
    SectorInfo sectorInfo;
    SECTOR_ADDR sector;
    BLOCK_ID block;
    UINT32 count, offset, length;
    UINT8 buffer[2048];
    UINT8 *pEBOOT;
    

    // SW rights is placed in the 3-th last sector of image
    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    hFMD = FMD_Init(NULL, &regInfo, NULL);
    if (hFMD == NULL)
        {
        OALMSG(OAL_ERROR, (L"ERROR: FMD_Init call failed!\r\n"));
        goto cleanUp;
        }

    // Get flash info
    if (!FMD_GetInfo(&flashInfo))
        {
        OALMSG(OAL_ERROR, (L"ERROR: FMD_GetInfo call failed!\r\n"));
        goto cleanUp;
        }

    // We can support only flash with sector size which fit to our buffer
    if (flashInfo.wDataBytesPerSector > sizeof(buffer))
        {
        OALMSG(OAL_ERROR, (L"ERROR: "
            L"Flash sector size %d bytes bigger that supported %d bytes\r\n",
            flashInfo.wDataBytesPerSector, sizeof(buffer)    
            ));
        goto cleanUp;
        }

    // sw rights sector  is located in the 3-th  last sector of EBOOT image
    offset = IMAGE_XLDR_BOOTSEC_NAND_SIZE + IMAGE_EBOOT_BOOTSEC_NAND_SIZE;
    
    // Skip X-Loader & EBOOT code & bad blocks
	// Note that we also check the last eboot block in order to ensure it is good
    block  = 0;
    count = 0;
    while ((count < offset) && (block < flashInfo.dwNumBlocks))
        {
        if ((FMD_GetBlockStatus(block) & BLOCK_STATUS_BAD) == 0)
            {
            count += flashInfo.dwBytesPerBlock;
            }
        block++;
        }

	// We've incremented past the last eboot block in order to check it too
	// Back up now, the previous block is the last one containing eboot and is good
	block--;

    // Need to copy off the block contents to RAM 
    pEBOOT = (UINT8*)IMAGE_WINCE_CODE_PA;
    length = flashInfo.dwBytesPerBlock;

    memset((VOID*)pEBOOT, 0xFF, flashInfo.dwBytesPerBlock);
	sector = block * flashInfo.wSectorsPerBlock;
    offset = 0;

	OALMSG(0, (L"EBOOT!BLWriteBootCfg: wDataBytesPerSector %x, wSectorsPerBlock =%x sizeof (BOOT_CFG) = %x \r\n",
		flashInfo.wDataBytesPerSector,flashInfo.wSectorsPerBlock,sizeof (BOOT_CFG)));
   

    while (offset < length) 
    {
        // When block read fail, there isn't what we can do more
        if (!FMD_ReadSector(sector, pEBOOT + offset, &sectorInfo, 1)) {
                OALMSG(OAL_ERROR, (L"\r\nERROR: EBOOT!BLWriteBootCfg: "
                    L"Failed read sector %d from flash\r\n", sector
                ));
            goto cleanUp;
            }

        // Move to next sector
        sector++;
        offset += flashInfo.wDataBytesPerSector;
    }


	offset = flashInfo.wDataBytesPerSector*(flashInfo.wSectorsPerBlock - 3);

    //  Copy the sw rights info into 3-th last sector of saved block in RAM
    memcpy(pEBOOT + offset, pSWRights, SW_RIGHTS_BLOCK_SIZE); 


    // Erase block
    if (!FMD_EraseBlock(block))
        {
        OALMSG(OAL_ERROR, (L"ERROR: EBOOT!BLWriteBootCfg: "
            L"Flash block %d erase failed\r\n", block
            ));
        goto cleanUp;
        }


    // Write contents of the save block + config sector back to flash
    pEBOOT = (UINT8*)IMAGE_WINCE_CODE_PA;
    length = flashInfo.dwBytesPerBlock;

	sector = block * flashInfo.wSectorsPerBlock;
    offset = 0;
    while (offset < length)
    {
        // Prepare sector info
        memset(&sectorInfo, 0xFF, sizeof(sectorInfo));
        sectorInfo.bOEMReserved &= ~(OEM_BLOCK_READONLY|OEM_BLOCK_RESERVED);
        sectorInfo.dwReserved1 = 0;
        sectorInfo.wReserved2 = 0;

        // Write sector        
        if (!FMD_WriteSector(sector, pEBOOT + offset, &sectorInfo, 1))
            {
            OALMSG(OAL_ERROR, (L"ERROR: EBOOT!BLWriteBootCfg: "
                L"Flash sector %d write failed\r\n", sector
                ));
            goto cleanUp;
            }

        // Move to next sector
        sector++;
        offset += flashInfo.wDataBytesPerSector;
    }

    // Done    
    rc = TRUE;

cleanUp:
    if (hFMD != NULL) FMD_Deinit(hFMD);
    return rc;
}





//------------------------------------------------------------------------------
// This function should be called after FMD_Init call.
// 
//------------------------------------------------------------------------------
BOOL
BLReadBootCfgSectors( BYTE *pOutBuffer, UINT16 nOutBuffer )
{
    BOOL rc = FALSE;
//    HANDLE hFMD;
//    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;
    SectorInfo sectorInfo;
    SECTOR_ADDR sector;
    BLOCK_ID block;
    UINT32 count, offset;
   // UINT8 buffer[4096];
    
	// eboot configuration is placed in the last sector of EBOOT image
	// manufacture block is placed in last sectors before eboot configuration sector

	/*
    regInfo.MemBase.Reg[0] = g_ulFlashBase;
    hFMD = FMD_Init(NULL, &regInfo, NULL);
    if (hFMD == NULL)
        {
        OALMSG(OAL_ERROR, (L"ERROR: FMD_Init call failed!\r\n"));
        goto cleanUp;
        }
		*/

    // Get flash info
    if (!FMD_GetInfo(&flashInfo))
        {
        OALMSG(OAL_ERROR, (L"ERROR: FMD_GetInfo call failed!\r\n"));
        goto cleanUp;
        }
		

    if (!pOutBuffer || 3* flashInfo.wDataBytesPerSector > nOutBuffer)
        {
        OALMSG(OAL_ERROR, (L"ERROR: output buffer is NULL \r\n "));
        goto cleanUp;
        }

    // Configuration sectors (eboot config/manufacture/sw rights) are located in the 3 last sectors of EBOOT image
    offset = IMAGE_XLDR_BOOTSEC_NAND_SIZE + IMAGE_EBOOT_BOOTSEC_NAND_SIZE;
    
    // Start from beginning
    block  = 0;
    sector = 0;

    // Skip X-Loader & EBOOT code & bad blocks
	// Note that we also check the last eboot block in order to ensure it is good
    count = 0;
    while ((count < offset) && (block < flashInfo.dwNumBlocks))
        {
        if ((FMD_GetBlockStatus(block) & BLOCK_STATUS_BAD) == 0)
            {
            count += flashInfo.dwBytesPerBlock;
            }
        block++;
        }

	// We've incremented past the last eboot block in order to check it too
	// Back up now, the previous block is the last one containing eboot and is good
	block--;

    //  Compute sectors within the block where configurations lies
	sector = block * flashInfo.wSectorsPerBlock;
    sector += flashInfo.wSectorsPerBlock - 3;

    // Read 3 sectors to buffer
    if (!FMD_ReadSector(sector, pOutBuffer, &sectorInfo, 3)) {
        OALMSG(OAL_ERROR, (L"ERROR: EBOOT!BLReadBootCfg: "
            L"Flash sector %d read failed\r\n", sector
        ));
            goto cleanUp;
    }
      

    // Done    
    rc = TRUE;

cleanUp:
//    if (hFMD != NULL) FMD_Deinit(hFMD);
    return rc;
}


