//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//

#include "fallite.h"
// define this constant larger if there will be more than 16 flash regions
#define MAX_FLASH_REGIONS   16


PSECTOR_ADDR Log2Phys = 0;//(IMAGE_DISPLAY_BUF_PA + IMAGE_DISPLAY_BUF_SIZE);

// structure to hold the FAL context (in case multiple instances are used,
// though currently not supported)
typedef struct _FALContext 
{    
    PVOID hFMD;
    PVOID hFMDHook;
    FMDInterface FMDInterface;
    FlashInfoEx flashInfoEx;
    FlashRegion regions[MAX_FLASH_REGIONS-1];
    
} FALContext, *PFALContext;

// global context information, multiple instances not supported
FALContext g_FAL = {0};

pfnFMD_ReadSectorNotify  g_fnFal_ReadSectorNotify = 0;
// --------------------------------------------------------------------
void SetFalNotify(pfnFMD_ReadSectorNotify fnFal_ReadSectorNotify)
{
 RETAILMSG (0, (TEXT("SetFalNotify\r\n")));
	g_fnFal_ReadSectorNotify = fnFal_ReadSectorNotify;
}

// --------------------------------------------------------------------
static inline BOOL IsBlockUsable(PFALContext pFALContext, DWORD dwBlock)
{
    // blocks marked as bad or reserved are not usable, all others 
    // (including readonly) are usable
    return (0 == ((pFALContext->FMDInterface.pGetBlockStatus(dwBlock) & (BLOCK_STATUS_BAD | BLOCK_STATUS_RESERVED))));
}

// --------------------------------------------------------------------
// --------------------------------------------------------------------
static inline BOOL IsSectorValid(WORD wSectorFlags)
{
    wSectorFlags = ~wSectorFlags;

    // bit 0x0004 indicates that the sector has been written, bit 0x0001 
    // indicates that the sector is dirty. only written, non-dirty
    // sectors are considered valid.
    return ((wSectorFlags & 0x0004) && !(wSectorFlags & 0x0001));
}

// --------------------------------------------------------------------
// --------------------------------------------------------------------
static BOOL GetFMDInterface(PFALContext pFALContext)
{
    pFALContext->FMDInterface.cbSize = sizeof(FMDInterface);
    pFALContext->FMDInterface.pOEMIoControl = FMD_OEMIoControl;
    
    if (!FMD_OEMIoControl (IOCTL_FMD_GET_INTERFACE, NULL, 0, (PBYTE)&pFALContext->FMDInterface, sizeof(FMDInterface), NULL)) {
        // FMD does not support IOCTL_FMD_GET_INTERFACE, so build the FMDInterface 
        // structure using the legacy FMD functions
        pFALContext->FMDInterface.pInit = FMD_Init;
        pFALContext->FMDInterface.pDeInit = FMD_Deinit;
        pFALContext->FMDInterface.pGetInfo = FMD_GetInfo;        
        pFALContext->FMDInterface.pGetBlockStatus = FMD_GetBlockStatus;     
        pFALContext->FMDInterface.pSetBlockStatus = FMD_SetBlockStatus;
        pFALContext->FMDInterface.pReadSector = FMD_ReadSector;
        pFALContext->FMDInterface.pWriteSector = FMD_WriteSector;
        pFALContext->FMDInterface.pEraseBlock = FMD_EraseBlock;
        pFALContext->FMDInterface.pPowerUp = FMD_PowerUp;
        pFALContext->FMDInterface.pPowerDown = FMD_PowerDown;

    } else if (!pFALContext->FMDInterface.pInit ||
        !pFALContext->FMDInterface.pDeInit ||
        (!pFALContext->FMDInterface.pGetInfo && !pFALContext->FMDInterface.pGetInfoEx) ||
        !pFALContext->FMDInterface.pGetBlockStatus ||
        !pFALContext->FMDInterface.pSetBlockStatus ||
        !pFALContext->FMDInterface.pReadSector ||
        !pFALContext->FMDInterface.pWriteSector ||
        !pFALContext->FMDInterface.pEraseBlock ||
        !pFALContext->FMDInterface.pOEMIoControl) {
        // one of the required functions was not implemented
        return FALSE;
    }

    // query hook library in case any FMD functions need to be shimmed
    pFALContext->hFMDHook = FMDHOOK_HookInterface(&pFALContext->FMDInterface);
    return (NULL != pFALContext->hFMDHook);
}

// --------------------------------------------------------------------
// --------------------------------------------------------------------
static VOID CalculateLogicalRange(PFALContext pFALContext, PFlashRegion pRegion)
{
    DWORD dwBlockID;
    DWORD dwNumLogicalBlocks = 0;
    
    for (dwBlockID = pRegion->dwStartPhysBlock; dwBlockID < pRegion->dwStartPhysBlock + pRegion->dwNumPhysBlocks; dwBlockID++)
    {
        DWORD dwStatus = pFALContext->FMDInterface.pGetBlockStatus (dwBlockID);

        if (!(dwStatus & (BLOCK_STATUS_RESERVED | BLOCK_STATUS_BAD)))
        {
            dwNumLogicalBlocks++;
        }
    }

    // Account for compaction blocks
    dwNumLogicalBlocks -= pRegion->dwCompactBlocks;
    pRegion->dwNumLogicalBlocks = dwNumLogicalBlocks;
}

// --------------------------------------------------------------------
// --------------------------------------------------------------------
static VOID CalculatePhysRange(PFALContext pFALContext, PFlashRegion pRegion)
{
    DWORD dwBlockID = pRegion->dwStartPhysBlock;
    DWORD dwNumLogicalBlocks = 0;
    
    while (dwNumLogicalBlocks < pRegion->dwNumLogicalBlocks)
    {
        DWORD dwStatus = pFALContext->FMDInterface.pGetBlockStatus (dwBlockID);

        if (!(dwStatus & (BLOCK_STATUS_RESERVED | BLOCK_STATUS_BAD)))
        {
            dwNumLogicalBlocks++;
        }
        
        dwBlockID++;
    }    

    pRegion->dwNumPhysBlocks = dwBlockID - pRegion->dwStartPhysBlock + pRegion->dwCompactBlocks;
}

// --------------------------------------------------------------------
// --------------------------------------------------------------------
static BOOL GetFlashInfo(PFALContext pFALContext)
{
    if (pFALContext->FMDInterface.pGetInfoEx) {
        DWORD dwRegionCount = MAX_FLASH_REGIONS;
        pFALContext->flashInfoEx.cbSize = sizeof(FlashInfoEx);
        return pFALContext->FMDInterface.pGetInfoEx (&pFALContext->flashInfoEx, &dwRegionCount);
    } else {   
        BOOL fXIPMode;
        FlashInfo flashInfo;
        if (pFALContext->FMDInterface.pGetInfo (&flashInfo)) {
            pFALContext->flashInfoEx.flashType = flashInfo.flashType;
            pFALContext->flashInfoEx.dwNumBlocks = flashInfo.dwNumBlocks;
            pFALContext->flashInfoEx.dwDataBytesPerSector = flashInfo.wDataBytesPerSector;
            pFALContext->flashInfoEx.dwNumRegions = 1;
            if (FMD_OEMIoControl (IOCTL_FMD_GET_XIPMODE, (PBYTE)&fXIPMode, sizeof(BOOL), NULL, 0, NULL) && fXIPMode) {
                // the single region is in XIP mode, so treat it as an XIP region
                pFALContext->flashInfoEx.region[0].regionType = XIP;
            } else {
                // the single region is NOT in XIP mode, so it is just a FILESYS region
                pFALContext->flashInfoEx.region[0].regionType = FILESYS;
            }
            pFALContext->flashInfoEx.region[0].dwStartPhysBlock = 0;
            pFALContext->flashInfoEx.region[0].dwNumPhysBlocks = flashInfo.dwNumBlocks;
            pFALContext->flashInfoEx.region[0].dwNumLogicalBlocks = FIELD_NOT_IN_USE;
            pFALContext->flashInfoEx.region[0].dwSectorsPerBlock = flashInfo.wSectorsPerBlock;            
            pFALContext->flashInfoEx.region[0].dwBytesPerBlock = flashInfo.dwBytesPerBlock;
            pFALContext->flashInfoEx.region[0].dwCompactBlocks = 2;
            return TRUE;
        }
    }
    return FALSE;
}

// --------------------------------------------------------------------
// --------------------------------------------------------------------
static PFlashRegion RegionFromLogicalSector(PFALContext pFALContext, SECTOR_ADDR sectorAddr, PSECTOR_ADDR pSectorPhysStart)
{
    SECTOR_ADDR sectorLogicalStart = 0, sectorPhysicalStart = 0, sectorLogicalEnd = 0;
    
    for (DWORD iRegion = 0; iRegion < pFALContext->flashInfoEx.dwNumRegions; iRegion ++) {

        sectorLogicalEnd += pFALContext->flashInfoEx.region[iRegion].dwNumLogicalBlocks * 
            pFALContext->flashInfoEx.region[iRegion].dwSectorsPerBlock;

        if ((sectorAddr >= sectorLogicalStart) && (sectorAddr < sectorLogicalEnd)) {
            // the logical sector is in this flash region
            if (pSectorPhysStart) {
                *pSectorPhysStart = sectorPhysicalStart;
            }
            return &pFALContext->flashInfoEx.region[iRegion];
        }

        // calculate the starting physical sector of the next region;
        sectorPhysicalStart += pFALContext->flashInfoEx.region[iRegion].dwNumPhysBlocks * 
            pFALContext->flashInfoEx.region[iRegion].dwSectorsPerBlock;

        sectorLogicalStart = sectorLogicalEnd;
    }

    return NULL;
}

static BOOL LockOrUnlockRegion (DWORD dwIoctl, REGION_TYPE regionType)
{
    BOOL fRet = FALSE;
    PFALContext pFALContext = &g_FAL;

    // Check to see if the FAL context has been initialized
    if (pFALContext->flashInfoEx.dwNumRegions == 0) {
        RETAILMSG (1, (TEXT("LockOrUnlockRegion: calling FAL_Init\r\n")));
        FAL_Init(NULL,0);
    }

    // determine which flash region this block lands in
    PFlashRegion pRegion = NULL;

    for (DWORD iRegion = 0; iRegion < pFALContext->flashInfoEx.dwNumRegions; iRegion ++) {

        pRegion = &pFALContext->flashInfoEx.region[iRegion];
        
        if (pRegion->regionType == regionType) {

            if (dwIoctl == IOCTL_FMD_LOCK_BLOCKS) {
                RETAILMSG (1, (TEXT("LockRegion: Start block = 0x%x, Num blocks = 0x%x\r\n"), pRegion->dwStartPhysBlock, pRegion->dwNumPhysBlocks));
            } else {
                RETAILMSG (1, (TEXT("UnlockRegion: Start block = 0x%x, Num blocks = 0x%x\r\n"), pRegion->dwStartPhysBlock, pRegion->dwNumPhysBlocks));
            }

            for (DWORD iBlock = pRegion->dwStartPhysBlock; iBlock < pRegion->dwStartPhysBlock + pRegion->dwNumPhysBlocks; iBlock++) {

                DWORD dwStatus = pFALContext->FMDInterface.pGetBlockStatus (iBlock);

                if (!(dwStatus & (BLOCK_STATUS_RESERVED | BLOCK_STATUS_BAD))) {
                   
                    // Lock the physical block, skipping bad or reserved blocks.
                    BlockLockInfo lockInfo;
                    lockInfo.StartBlock = iBlock;
                    lockInfo.NumBlocks = 1;        
                    
                    fRet = pFALContext->FMDInterface.pOEMIoControl(dwIoctl, (BYTE*)&lockInfo, sizeof(BlockLockInfo), NULL, 0, NULL); 

                    if (!fRet) {
                        RETAILMSG (1, (TEXT("LockOrUnlockRegion: failed to lock block %u\r\n"), iBlock));
                        return FALSE;
                    }
                }
            }
        }
    }
    
    return fRet;
}



// --------------------------------------------------------------------
// --------------------------------------------------------------------
VOID FAL_Deinit(PVOID hFAL)
{
    PFALContext pFALContext = (PFALContext) hFAL;
    if (&g_FAL != pFALContext) {
        RETAILMSG(1, (TEXT("FAL_Deinit: invalid context\r\n")));
        return;
    }
    
    if (pFALContext->hFMDHook) {
        // unhook fmd interface shims
        FMDHOOK_UnhookInterface(pFALContext->hFMDHook, &pFALContext->FMDInterface);
    }

	Log2Phys = 0;
}
BOOL InitMap(PVOID hFAL)
{
	static UINT32 NotifCount = 440;

    RETAILMSG(0, (TEXT("FAL_InitMap+\r\n")));
    PFALContext pFALContext = (PFALContext) hFAL;
    if (&g_FAL != pFALContext) {
        RETAILMSG(1, (TEXT("InitMap: invalid context\r\n")));
        return FALSE;
    }
    SECTOR_ADDR sectorPhysicalStart;
	//now we have only 1 region

	PFlashRegion pRegion = RegionFromLogicalSector(pFALContext, 0, &sectorPhysicalStart);
    if (NULL == pRegion) {
        RETAILMSG(1, (TEXT("InitMap: Region is not found\r\n")));
        return FALSE;
    }

    if (XIP != pRegion->regionType)
	{
        // To read sector data from a non-XIP region, we cannot guarantee that there is a 
        // one-to-one logical-to-physical sector mapping. So, we must read the spare area
        // of every sector to determine which physical sector is mapped to the logical
        // sector requested by the caller. This read operation can be very slow.
		if(Log2Phys)//iii
		{
			memset(Log2Phys, 0, pRegion->dwNumPhysBlocks * pRegion->dwSectorsPerBlock * sizeof(SECTOR_ADDR));

			// walk every block in this region looking for the requested logical sector
			for (DWORD dwBlock = pRegion->dwStartPhysBlock;	dwBlock < pRegion->dwStartPhysBlock + pRegion->dwNumPhysBlocks;	dwBlock ++) 
			{
				if (IsBlockUsable(pFALContext, dwBlock)) 
				{
					SectorInfo sectorInfo;
					for (SECTOR_ADDR sectorCur = 0; sectorCur < pRegion->dwSectorsPerBlock; sectorCur ++) 
					{
						if (!pFALContext->FMDInterface.pReadSector(sectorPhysicalStart + sectorCur, NULL, &sectorInfo, 1)) 
						{
							RETAILMSG(1, (TEXT("FAL_InitMap: failed to read sector 0x%08x\r\n"), sectorPhysicalStart + sectorCur));
							continue;
						}
			
						// check for valid, matching sector info
						if( (sectorInfo.dwReserved1 < pRegion->dwNumPhysBlocks * pRegion->dwSectorsPerBlock )	&&
							IsSectorValid(sectorInfo.wReserved2)											)
						{
							Log2Phys[sectorInfo.dwReserved1] = (SECTOR_ADDR)(sectorPhysicalStart + sectorCur);
						}

						RETAILMSG(0,(TEXT("FAL_InitMap: log 0x%08x phys 0x%08x)\r\n"), sectorInfo.dwReserved1, sectorPhysicalStart + sectorCur));
						if( g_fnFal_ReadSectorNotify )
						{
							if( ++NotifCount > 439 )
							{
								NotifCount = 0;
								g_fnFal_ReadSectorNotify();
							}
						}
					}
				}
				sectorPhysicalStart += pRegion->dwSectorsPerBlock;
			}
		}
	}

    RETAILMSG(0, (TEXT("FAL_InitMap:-\r\n")));
    return FALSE;
}


// --------------------------------------------------------------------
//BOOL InitMap(PVOID hFAL)
//{
//    RETAILMSG(0, (TEXT("FAL_InitMap+\r\n")));
//    PFALContext pFALContext = (PFALContext) hFAL;
//    if (&g_FAL != pFALContext) {
//        RETAILMSG(1, (TEXT("InitMap: invalid context\r\n")));
//        return FALSE;
//    }
//    SECTOR_ADDR sectorPhysicalStart;
//	//now we have only 1 region
//
//	PFlashRegion pRegion = RegionFromLogicalSector(pFALContext, 0, &sectorPhysicalStart);
//    if (NULL == pRegion) {
//        RETAILMSG(1, (TEXT("InitMap: Region is not found\r\n")));
//        return FALSE;
//    }
//
//    if (XIP != pRegion->regionType)
//	{
//        // To read sector data from a non-XIP region, we cannot guarantee that there is a 
//        // one-to-one logical-to-physical sector mapping. So, we must read the spare area
//        // of every sector to determine which physical sector is mapped to the logical
//        // sector requested by the caller. This read operation can be very slow.
//		if(Log2Phys)//iii
//		{
//			memset(Log2Phys, 0, pRegion->dwNumPhysBlocks * pRegion->dwSectorsPerBlock * sizeof(SECTOR_ADDR));
//
//			// walk every block in this region looking for the requested logical sector
//			for (DWORD dwBlock = pRegion->dwStartPhysBlock;	dwBlock < pRegion->dwStartPhysBlock + pRegion->dwNumPhysBlocks;	dwBlock ++) 
//			{
//				if (IsBlockUsable(pFALContext, dwBlock)) 
//				{
//					SectorInfo* sectorInfo = (SectorInfo*)((BYTE*)Log2Phys + pRegion->dwNumPhysBlocks * pRegion->dwSectorsPerBlock * sizeof(DWORD));
//					
//					if(!pFALContext->FMDInterface.pReadSector(sectorPhysicalStart, NULL, sectorInfo, pRegion->dwSectorsPerBlock)) 
//					{
//						RETAILMSG(1, (TEXT("FAL_InitMap: failed to read sector 0x%08x\r\n"), sectorPhysicalStart));
//						continue;
//					}
//					for (SECTOR_ADDR sectorCur = 0; sectorCur < pRegion->dwSectorsPerBlock; sectorCur ++) 
//					{
//						// check for valid, matching sector info
//						if( (sectorInfo[(DWORD)sectorCur].dwReserved1 < pRegion->dwNumPhysBlocks * pRegion->dwSectorsPerBlock )	&&
//							IsSectorValid(sectorInfo[(DWORD)sectorCur].wReserved2)											)
//						{
//							Log2Phys[sectorInfo[(DWORD)sectorCur].dwReserved1] = (SECTOR_ADDR)(sectorPhysicalStart + sectorCur);
//						}
//
//						RETAILMSG(0,(TEXT("FAL_InitMap: log 0x%08x phys 0x%08x)\r\n"), sectorInfo[(DWORD)sectorCur].dwReserved1, sectorPhysicalStart + sectorCur));
//					}
//				}
//				sectorPhysicalStart += pRegion->dwSectorsPerBlock;
//			}
//		}
//	}
//
//    RETAILMSG(0, (TEXT("FAL_InitMap:-\r\n")));
//    return FALSE;
//}
//
//
//
// --------------------------------------------------------------------
PVOID FAL_Init (PVOID hFMD, LPBYTE buffer)
{   
    PFALContext pFALContext = (PFALContext) &g_FAL;
	
	// currently this is just saved
    pFALContext->hFMD = hFMD;
    
    // query flash interface
    if (!GetFMDInterface (pFALContext)) {
        RETAILMSG(1, (TEXT("FAL_Init: failed to query FMD interface\r\n")));
        FAL_Deinit(pFALContext);
        return FALSE;
    }
        
    // get flash info
    if (!GetFlashInfo (pFALContext)) {
        RETAILMSG(1, (TEXT("FAL_Init: failed to query flash info\r\n")));        
        FAL_Deinit(pFALContext);
        return FALSE;
    }

    // calculate flash region sizes
    for (DWORD iRegion = 0; iRegion < pFALContext->flashInfoEx.dwNumRegions; iRegion ++) {
        PFlashRegion pRegion = &pFALContext->flashInfoEx.region[iRegion];
                
        // calculate the number of logical and physical blocks
        if (pRegion->dwNumLogicalBlocks == FIELD_NOT_IN_USE) {
            // determine logical range from the given physical range
            CalculateLogicalRange(pFALContext, pRegion);
        } else {
            // determine the physical range from the given logical range
            if (iRegion == 0) {
                // the first region starts on block zero
                pRegion->dwStartPhysBlock = 0;
            } else {
                // subsequent regions start on the next block past the previous region
                pRegion->dwStartPhysBlock = pFALContext->flashInfoEx.region[iRegion-1].dwStartPhysBlock + 
                    pFALContext->flashInfoEx.region[iRegion-1].dwNumPhysBlocks;
            }
            
            if (pRegion->dwNumLogicalBlocks == END_OF_FLASH) {
                // This region goes to the end of flash.  Determine how many logical
                // sectors are available
                pRegion->dwNumPhysBlocks = pFALContext->flashInfoEx.dwNumBlocks - pRegion->dwStartPhysBlock;
                CalculateLogicalRange(pFALContext, pRegion);
            } else {
                // Determine phsyical range from the given logical number of blocks
                CalculatePhysRange(pFALContext, pRegion);
            }
        }
    }

	if(!Log2Phys)//don't remap twice (with FAL_Deinit only)
	{
		Log2Phys = (PSECTOR_ADDR)buffer;
		InitMap(pFALContext);
	}

    return (PVOID)pFALContext;
}
// --------------------------------------------------------------------
// --------------------------------------------------------------------
BOOL FAL_ReadSectors(PVOID hFAL, SECTOR_ADDR sectorLogical, LPBYTE pSectorBuff, DWORD dwSectorCount)
{
	static DWORD NotifCount = 440;

    RETAILMSG(0, (TEXT("FAL_ReadSectors+\r\n")));
    PFALContext pFALContext = (PFALContext) hFAL;
    if (&g_FAL != pFALContext) {
        RETAILMSG(1, (TEXT("FAL_ReadSectors: invalid context\r\n")));
        RETAILMSG(1, (TEXT("FAL_ReadSectors: invalid context\r\n")));
        return FALSE;
    }

    if (1 != dwSectorCount) {
        // currently we only support single-sector reads because that is all bootpart requries
        return FALSE;
    }

    SECTOR_ADDR sectorPhysicalStart;
    PFlashRegion pRegion = RegionFromLogicalSector(pFALContext, sectorLogical, &sectorPhysicalStart);
    if (NULL == pRegion) {
        RETAILMSG(1, (TEXT("FAL_ReadSectors: invalid logical sector address (0x%08x)\r\n"), sectorLogical));
        return FALSE;
    }

    if (XIP == pRegion->regionType) {
        // To read sector data from an XIP region, we know there is a one-to-one logical-
        // to-physical sector mapping (accounting for bad/reserved blocks)

        SECTOR_ADDR sectorPhysical = sectorLogical;
        SECTOR_ADDR sectorOffset = sectorLogical - sectorPhysicalStart;
        for (DWORD dwBlock = pRegion->dwStartPhysBlock; 
            dwBlock <= pRegion->dwStartPhysBlock + (sectorOffset / pRegion->dwSectorsPerBlock);
            dwBlock ++) 
        {
            while (!IsBlockUsable(pFALContext, dwBlock)) {
                // for every unusable block, the physical sector location advances
                // by the number of sectors per block.
                sectorPhysical += pRegion->dwSectorsPerBlock;
                dwBlock ++;

                // this should never walk off the end of the region or the regions
                // reported by the FMD are incorrect.
                ASSERT(dwBlock < pRegion->dwStartPhysBlock + pRegion->dwNumPhysBlocks);
            }
        }

        DEBUGMSG(0, (TEXT("FAL_ReadSectors: reading logical XIP sector 0x%08x (mapped to physical sector 0x%08x)\r\n"), sectorLogical, sectorPhysical));
        return pFALContext->FMDInterface.pReadSector(sectorPhysical, pSectorBuff, NULL, 1);
        
    } else {
        // To read sector data from a non-XIP region, we cannot guarantee that there is a 
        // one-to-one logical-to-physical sector mapping. So, we must read the spare area
        // of every sector to determine which physical sector is mapped to the logical
        // sector requested by the caller. This read operation can be very slow.
	if(!Log2Phys)//iii
	{
		// walk every block in this region looking for the requested logical sector
        for (DWORD dwBlock = pRegion->dwStartPhysBlock; 
            dwBlock < pRegion->dwStartPhysBlock + pRegion->dwNumPhysBlocks;
            dwBlock ++) 
        {
            if (IsBlockUsable(pFALContext, dwBlock)) {
                SectorInfo sectorInfo;
                for (SECTOR_ADDR sectorCur = 0; sectorCur < pRegion->dwSectorsPerBlock; sectorCur ++) {
                    if (!pFALContext->FMDInterface.pReadSector(sectorPhysicalStart + sectorCur, NULL, &sectorInfo, 1)) {
                        DEBUGMSG(1, (TEXT("FAL_ReadSectors: failed to read sector 0x%08x\r\n"), sectorPhysicalStart + sectorCur));
                        continue;
                    }
					RETAILMSG(0,(TEXT("FAL_ReadSectors: buf %x log 0x%08x phys 0x%08x)\r\n"), (DWORD)Log2Phys, sectorInfo.dwReserved1, sectorPhysicalStart + sectorCur));
					
					// check for valid, matching sector info
					RETAILMSG(0,(TEXT("FAL_ReadSectors: Log2Phys %x log 0x%08x phys 0x%08x)\r\n"), (DWORD)Log2Phys, sectorInfo.dwReserved1, sectorPhysicalStart + sectorCur));


                    if ((sectorInfo.dwReserved1 == sectorLogical) &&
                        IsSectorValid(sectorInfo.wReserved2)) {
                        // found the desired sector
                        RETAILMSG(0, (TEXT("FAL_ReadSectors: reading logical sector 0x%08x (mapped to physical sector 0x%08x)\r\n"), sectorLogical, sectorPhysicalStart + sectorCur));

						if( g_fnFal_ReadSectorNotify )
						{
							if(++NotifCount > 439)
							{
								NotifCount = 0;
								g_fnFal_ReadSectorNotify();
							}
						}
						return pFALContext->FMDInterface.pReadSector(sectorPhysicalStart + sectorCur, pSectorBuff, NULL, 1);
                    }
                }
            }
            sectorPhysicalStart += pRegion->dwSectorsPerBlock;
        }
	}//InitFlag
	else if(Log2Phys)
	{
		if( (sectorLogical < pRegion->dwBytesPerBlock * pRegion->dwNumPhysBlocks) &&
			( Log2Phys[(UINT)sectorLogical] )										)
		{
            RETAILMSG(0, (TEXT("FAL_ReadSectors: reading logical sector 0x%08x (mapped to physical sector 0x%08x)\r\n"), sectorLogical, sectorPhysicalStart + Log2Phys[(UINT)sectorLogical]));
			if( g_fnFal_ReadSectorNotify )
				g_fnFal_ReadSectorNotify();
	
			return pFALContext->FMDInterface.pReadSector(Log2Phys[(UINT)sectorLogical], pSectorBuff, NULL, 1);
		}
	}
    }

    RETAILMSG(1, (TEXT("FAL_ReadSectors: logical sector 0x%08x is unmapped!\r\n"), sectorLogical));
    return FALSE;
}

// --------------------------------------------------------------------
// --------------------------------------------------------------------
BOOL FAL_WriteSectors(PVOID hFAL, SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, DWORD dwSectorCount)
{
    PFALContext pFALContext = (PFALContext) hFAL;
    if (&g_FAL != pFALContext) {
        RETAILMSG(1, (TEXT("FAL_WriteSector: invalid context\r\n")));
        return FALSE;
    }

    // write is not currently supported
    return FALSE;
}

// --------------------------------------------------------------------
// --------------------------------------------------------------------
BOOL FAL_GetInfo (PVOID hFAL, DISK_INFO *pdi)
{
    PFALContext pFALContext = (PFALContext) hFAL;
    if (&g_FAL != pFALContext) {
        RETAILMSG(1, (TEXT("FAL_GetInfo: invalid context\r\n")));
        return FALSE;
    }

    if (NULL == pdi) {
        return FALSE;
    }   

    pdi->di_total_sectors = 0;    
    for (DWORD iRegion = 0; iRegion < pFALContext->flashInfoEx.dwNumRegions; iRegion ++) {
        pdi->di_total_sectors += pFALContext->flashInfoEx.region[iRegion].dwNumLogicalBlocks * 
            pFALContext->flashInfoEx.region[iRegion].dwSectorsPerBlock;
    }
    pdi->di_bytes_per_sect = pFALContext->flashInfoEx.dwDataBytesPerSector;
    pdi->di_cylinders = 1;
    pdi->di_heads = 1;
    pdi->di_sectors = pdi->di_total_sectors;

    return TRUE;
}
// --------------------------------------------------------------------
// --------------------------------------------------------------------
BOOL FAL_LockFlashRegion (REGION_TYPE regionType)
{
    return LockOrUnlockRegion(IOCTL_FMD_LOCK_BLOCKS, regionType);
}
// --------------------------------------------------------------------
// --------------------------------------------------------------------

BOOL FAL_UnlockFlashRegion (REGION_TYPE regionType)
{
    return LockOrUnlockRegion(IOCTL_FMD_UNLOCK_BLOCKS, regionType);
}

