/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
  IF YOU ARE A LICENSEE, YOUR RIGHT, IF ANY, TO COPY, PUBLISH, MODIFY, OR
  OTHERWISE USE THE SOFTWARE, IS SUBJECT TO THE TERMS AND CONDITIONS OF THE
  BINDING AGREEMENTS.  BY USING THE SOFTWARE IN ANY MANNER, IN WHOLE OR IN
  PART, YOU AGREE TO BE BOUND BY THE TERMS OF THE BINDING AGREEMENTS.

  IF YOU ARE NOT A DATALIGHT LICENSEE, ANY USE MAY RESULT IN CIVIL AND
  CRIMINAL ACTION AGAINST YOU.  CONTACT DATALIGHT, INC. AT THE ADDRESS
  SET FORTH ABOVE IF YOU OBTAINED THIS SOFTWARE IN ERROR.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This module contains a sample implementation of an FMD-to-FlashFX
    interface.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmd_flashfx.c $
    Revision 1.5  2010/07/18 00:49:08Z  garyp
    Updated so the "AltUnlock" functionality is optional, and must be
    explicitly turned on.
    Revision 1.4  2010/01/11 02:52:00Z  garyp
    Minor structure initialization updates.
    Revision 1.3  2010/01/10 22:07:40Z  garyp
    Changed to default to Device spanning where possible.  Initialize
    the AutoUnlock flag.  Documentation updated.
    Revision 1.2  2009/08/04 22:42:22Z  garyp
    Merged from the v4.0 branch.  Re-checked in to work around the MKS
    branch first conundrum.
    Revision 1.1  2008/12/18 04:38:42Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <flashfx_wincebl.h>
#include <fxloaderapi.h>

static FFXLOADERHANDLE    hLdr;


/*-------------------------------------------------------------------
    Public: FMD_Init()

    Initialize the FMD-to-FlashFX translation layer.

    *Note* -- For WinMobile, typically pActive is NULL if IPL is being
              initialized, and points to a valid wide-character name
              for everything else.  This layer depends on that behavior
              to know how to initialize the AutoUnlock feature, as well
              as how to process output.

    Parameters:
        pActive - A pointer to the null-terminated wide-char name of
                  the object being initialized.
        pRegIn  - Input parameters -- not used in this implementation.                  
        pRegOut - Output parameters -- not used in this implementation.                  

    Return Value:
        Returns an FMD context pointer if successful, or NULL if not.
-------------------------------------------------------------------*/
PVOID FMD_Init(
    LPCTSTR         pActive,
    PPCI_REG_INFO   pRegIn,
    PPCI_REG_INFO   pRegOut)
{
    RETAILMSG(DBG_MSG, (TEXT("FlashFX:FMD_Init()\r\n")));

    if(pActive != NULL)
        RETAILMSG(DBG_MSG, (TEXT("  pActive=%s\r\n"), pActive));
    else
        RETAILMSG(DBG_MSG, (TEXT("  pActive=IPL\r\n")));

    hLdr = FfxWM_FMD_Init(pActive, pRegIn, pRegOut);
    if(hLdr)
    {
        FFXWMFMDRANGE   Range1 = {0};

        /*  This initialization above clears all the fields to zeros, which
            happens to be the required standard default values needed.
        */

        /*  ... That is, except for turning the BBM flag on
        */
        Range1.fUseBBM = TRUE;

        /*  The fSpanDevices flag will cause the FlashFX Disk initialization
            logic to attempt to span multiple Devices, if possible.
        */
        Range1.fSpanDevices = TRUE;

        /*  If we are operating in the context of anything except IPL, unlock
            all the flash at startup.  If we are using IPL (pActive == NULL),
            leave the lock state unchanged.
        */            
        Range1.fAutoUnlock = pActive ? TRUE : FALSE;

        /*  If we are using the default IPL implementation which
            iteratively tries to unlock the flash one block at a 
            time, but the flash only supports single range unlock
            commands, set this value to TRUE.  This will cause 
            the FlashFX Loader code to ignore unlock requests for
            higher block numbers than what have already been 
            unlocked (knowing that they've already been unlocked
            because BBM must be unlocked).
        */            
        Range1.fAltUnlock = FALSE;

        /*  Initialize RANGE1 to be all of flash, with the exception
            that any reserved space specified in ffxconf.h is honored.
        */
        if(!FfxWM_FMD_InitRanges(hLdr, &Range1, NULL))
        {
            FfxWM_FMD_Deinit(hLdr);

            hLdr = NULL;
        }
    }

    RETAILMSG(DBG_MSG, (TEXT("FlashFX:FMD_Init() returning 0x%X\r\n"), hLdr));

    return hLdr;
}


/*-------------------------------------------------------------------
    Public: FMD_Deinit()

    Deinitialize the FMD-to-FlashFX translation layer.

    Parameters:
        pContext - The FMD context pointer returned by FMD_Init().

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
BOOL FMD_Deinit(
    VOID   *pContext)
{
    RETAILMSG(DBG_MSG, (TEXT("FlashFX:FMD_Deinit() pContext=0x%x\r\n"), pContext));

    if(hLdr)
    {
        FfxWM_FMD_Deinit(hLdr);
        hLdr = NULL;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: FMD_GetInfo()

    Get information about the flash.

    Parameters:
        pInfo - A pointer to the FlashInfo structure to fill.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
BOOL FMD_GetInfo(
    FlashInfo  *pInfo)
{
    RETAILMSG(DBG_MSG, (TEXT("FlashFX:FMD_GetInfo() pInfo=0x%x\r\n"), pInfo));

    return FfxWM_FMD_GetInfo(pInfo);
}


/*-------------------------------------------------------------------
    Public: FMD_GetPhysSectorAddr()

    Get the phsysical sector address.  This function is stubbed and
    does nothing.

    Parameters:
        dwSector         - The sector number
        pStartSectorAddr - A pointer to the SECTOR_ADDR structure to use.

    Return Value:
        None.
-------------------------------------------------------------------*/
static VOID FMD_GetPhysSectorAddr(
    DWORD           dwSector,
    PSECTOR_ADDR    pStartSectorAddr)
{
    RETAILMSG(DBG_MSG, (TEXT("FlashFX:FMD_GetPhysSectorAddr() is stubbed!\r\n")));

/*
    *pStartSectorAddr = dwSector * pageSize;
*/
    return;
}


/*-------------------------------------------------------------------
    Public: FMD_ReadSector()

    Read sectors from flash.

    Parameters:
        sector      - The sector number
        pBuffer     - A pointer to the buffer to fill.
        pSectorInfo - A pointer to the SectorInfo structure to use.
        dwCount     - The number of sectors to read.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
BOOL FMD_ReadSector(
    SECTOR_ADDR     sector,
    UCHAR          *pBuffer,
    SectorInfo     *pSectorInfo,
    DWORD           dwCount)
{
    BOOL            rc;

    DEBUGMSG(DBG_READ, (TEXT("FlashFX:FMD_ReadSector() sector = %d count = %d pBuffer = 0x%X\r\n"), sector, dwCount, pBuffer));

    rc = FfxWM_FMD_ReadSector(sector, pBuffer, pSectorInfo, dwCount);

    DEBUGMSG(DBG_READ, (TEXT("FlashFX:FMD_ReadSector() returning %d\r\n"), rc));

    return rc;
}


/*-------------------------------------------------------------------
    Public: FMD_WriteSector()

    Write sectors to flash.

    Parameters:
        sector      - The sector number
        pBuffer     - A pointer to the buffer to fill.
        pSectorInfo - A pointer to the SectorInfo structure to use.
        dwCount     - The number of sectors to read.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
BOOL FMD_WriteSector(
    SECTOR_ADDR     sector,
    UCHAR          *pBuffer,
    SectorInfo     *pSectorInfo,
    DWORD           dwCount)
{
    BOOL            rc;

    DEBUGMSG(DBG_WRITE, (TEXT("FlashFX:FMD_WriteSector() sector = 0x%X Count = %d pBuffer = 0x%X\r\n"), sector, dwCount, pBuffer));

    rc = FfxWM_FMD_WriteSector(sector, pBuffer, pSectorInfo, dwCount);

    DEBUGMSG(DBG_WRITE, (TEXT("FlashFX:FMD_WriteSector() returning %d\r\n"), rc));

    return rc;
}


/*-------------------------------------------------------------------
    Public: FMD_GetBlockStatus()

    Get the status of an erase block.

    Parameters:
        blockId - The block number.

    Return Value:
        Returns the block status flags.
-------------------------------------------------------------------*/
DWORD FMD_GetBlockStatus(
    BLOCK_ID    blockId)
{
    DWORD       rc;

    DEBUGMSG(DBG_FMD, (TEXT("FlashFX:FMD_GetBlockStatus() Block=%X\r\n"), blockId));

    rc = FfxWM_FMD_GetBlockStatus(blockId);

    DEBUGMSG(DBG_FMD, (TEXT("FlashFX:FMD_GetBlockStatus() returning %d\r\n"), rc));

    return rc;
}


/*-------------------------------------------------------------------
    Public: FMD_SetBlockStatus()

    Set the status of an erase block.

    Parameters:
        blockId - The block number.
        status  - The block status flags.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
BOOL FMD_SetBlockStatus(
    BLOCK_ID    blockId,
    DWORD       status)
{
    BOOL        rc;

    DEBUGMSG(DBG_FMD, (TEXT("FlashFX:FMD_SetBlockStatus() Block=%X\r\n"), blockId));

    rc = FfxWM_FMD_SetBlockStatus(blockId, status);

    DEBUGMSG(DBG_FMD, (TEXT("FlashFX:FMD_SetBlockStatus %d\r\n"), rc));

    return rc;
}


/*-------------------------------------------------------------------
    Public: FMD_EraseBlock()

    Erase a flash memory block.

    Parameters:
        blockId - The block number.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
BOOL FMD_EraseBlock(
    BLOCK_ID    blockId)
{
    BOOL        rc;

    DEBUGMSG(DBG_ERASE, (TEXT("FlashFX:FMD_EraseBlock() Block=%d\r\n"), blockId));

    rc = FfxWM_FMD_EraseBlock(blockId);

    DEBUGMSG(DBG_ERASE, (TEXT("FlashFX:FMD_EraseBlock() returning %d\r\n"), rc));

    return rc;
}


/*-------------------------------------------------------------------
    Public: FMD_PowerUp()

    Process power-up requests.

    Parameters:
        None.

    Return Value:
        None.
-------------------------------------------------------------------*/
VOID FMD_PowerUp(VOID)
{
    RETAILMSG(DBG_MSG, (TEXT("FlashFX:FMD_PowerUp()\r\n")));

    FfxWM_FMD_PowerUp();

    RETAILMSG(DBG_MSG, (TEXT("FlashFX:FMD_PowerUp() returning\r\n")));

    return;
}


/*-------------------------------------------------------------------
    Public: FMD_PowerDown()

    Process power-down requests.
    
    Parameters:
        None.

    Return Value:
        None.
-------------------------------------------------------------------*/
VOID FMD_PowerDown(VOID)
{
    RETAILMSG(DBG_MSG,  (TEXT("FlashFX:FMD_PowerDown()\r\n")));

    FfxWM_FMD_PowerDown();

    return;
}


/*-------------------------------------------------------------------
    Public: FMD_OEMIoControl()

    Dispatch IOCTL requests.

    Parameters:
        dwIoControlCode - The IOCTL code
        pInBuf          - Points to the buffer containing data to be
                          transferred to the device.
        nInBufSize      - Specifies the number of bytes of data in the
                          buffer specified for pInBuf.
        pOutBuf         - Points to the buffer used to transfer the
                          output data from the device.
        nOutBufSize     - Specifies the maximum number of bytes in the
                          buffer specified by pOutBuf
        pBytesReturned  - Points to DWORD buffer the function uses to
                          return the actual number of bytes returned.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
BOOL FMD_OEMIoControl(
    DWORD   dwIoControlCode,
    PBYTE   pInBuf,
    DWORD   nInBufSize,
    PBYTE   pOutBuf,
    DWORD   nOutBufSize,
    PDWORD  pBytesReturned)
{
    RETAILMSG(DBG_IOCTL, (TEXT("FlashFX:FMD_OEMIoControl() for code 0x%x\r\n"), dwIoControlCode));

    switch(dwIoControlCode)
    {
        case IOCTL_FMD_GET_INTERFACE:
        {
            PFMDInterface pInterface = (PFMDInterface)pOutBuf;

            RETAILMSG(DBG_IOCTL,  (TEXT("FlashFX:FMD_OEMIoControl() IOCTL_FMD_GET_INTERFACE\r\n")));

            if (!pOutBuf || nOutBufSize < sizeof(FMDInterface))
            {
                DEBUGMSG(DBG_ERROR,  (TEXT("FlashFX:FMD_OEMIoControl() IOCTL_FMD_GET_INTERFACE bad parameter(s).\r\n")));
                return FALSE;
            }

            pInterface->cbSize              = sizeof(FMDInterface);
            pInterface->pInit               = FMD_Init;
            pInterface->pDeInit             = FMD_Deinit;
            pInterface->pGetInfo            = FMD_GetInfo;
            pInterface->pGetInfoEx          = NULL;
            pInterface->pGetBlockStatus     = FMD_GetBlockStatus;
            pInterface->pSetBlockStatus     = FMD_SetBlockStatus;
            pInterface->pReadSector         = FMD_ReadSector;
            pInterface->pWriteSector        = FMD_WriteSector;
            pInterface->pEraseBlock         = FMD_EraseBlock;
            pInterface->pPowerUp            = FMD_PowerUp;
            pInterface->pPowerDown          = FMD_PowerDown;
            pInterface->pGetPhysSectorAddr  = FMD_GetPhysSectorAddr;

            return TRUE;
        }
        break;

        case IOCTL_FMD_SET_XIPMODE:
        {
            /*  Select between XIP mode or non-XIP mode.  The difference from
                the FMD's standpoint is whether or not sector information is
                stored along with each sector.
            */
            RETAILMSG(DBG_IOCTL,  (TEXT("FlashFX:FMD_OEMIoControl() IOCTL_FMD_SET_XIPMODE\r\n")));

            if (!pInBuf || nInBufSize < sizeof(BOOLEAN))
            {
                DEBUGMSG(DBG_IOCTL, (TEXT("FlashFX:FMD_OEMIoControl() IOCTL_FMD_SET_XIPMODE bad parameter(s).\r\n")));
                return FALSE;
            }

            RETAILMSG(1, (TEXT("FlashFX:IOCTL_FMD_SET_XIP Mode = %d\r\n"), *(PBOOLEAN)pInBuf));
        }
        break;

        case IOCTL_FMD_GET_XIPMODE:
        {
            RETAILMSG(DBG_IOCTL,  (TEXT("FlashFX:FMD_OEMIoControl() IOCTL_FMD_GET_XIPMODE\r\n")));

            if (!pOutBuf || nOutBufSize < sizeof(BOOLEAN))
            {
                DEBUGMSG(DBG_IOCTL, (TEXT("FlashFX:FMD_OEMIoControl() IOCTL_FMD_GET_XIPMODE bad parameter(s).\r\n")));
                return FALSE;
            }

            *(PBOOLEAN)pOutBuf = FALSE;
        }
        break;

        case IOCTL_FMD_READ_RESERVED:
        {
            /*  Read from a specified reserved region.
            */
            if (!pInBuf || nInBufSize < sizeof(ReservedReq))
            {
                DEBUGMSG(1, (TEXT("FMD_OEMIoControl: IOCTL_FMD_READ_RESERVED bad parameter(s).\r\n")));
                return FALSE;
            }

            RETAILMSG(DBG_IOCTL,  (TEXT("FlashFX:FMD_OEMIoControl() IOCTL_FMD_READ_RESERVED unsupported\r\n")));

            return FALSE;
        }
        break;

        case IOCTL_FMD_WRITE_RESERVED:
        {
            /*  Write to a specified reserved region.
            */
            if (!pInBuf || nInBufSize < sizeof(ReservedReq))
            {
                DEBUGMSG(1, (TEXT("FMD_OEMIoControl() IOCTL_FMD_READ_RESERVED bad parameter(s).\r\n")));
                return FALSE;
            }

            RETAILMSG(DBG_IOCTL, (TEXT("FlashFX:FMD_OEMIoControl() IOCTL_FMD_WRITE_RESERVED unsupported\r\n")));
            return FALSE;
        }
        break;

        case IOCTL_FMD_GET_RESERVED_TABLE:
        {
            // Get the reserved table.
            //        if (!pOutBuf)
            //        {
            //            // If no buffer is provided, return the required size.
            //            if (pBytesReturned)
            //                *pBytesReturned = g_dwNumReserved * sizeof(ReservedEntry);
            //            return TRUE;
            //        }
            //        else
            //        {
            //            DWORD dwTableSize = g_dwNumReserved * sizeof(ReservedEntry);
            //
            //            if (nOutBufSize < dwTableSize)
            //            {
            //                if (pBytesReturned)
            //                    *pBytesReturned = 0;
            //                return FALSE;
            //            }
            //            else
            //            {
            //                if (pBytesReturned)
            //                    *pBytesReturned = dwTableSize;
            //                if (g_pReservedTable)
            //                    memcpy (pOutBuf, g_pReservedTable, dwTableSize);
            //
            //                return TRUE;
            //            }
            //        }
            RETAILMSG(DBG_IOCTL, (TEXT("FlashFX:FMD_OEMIoControl() GetReserved unsupported\r\n")));
            return FALSE;
        }
        break;

//    case IOCTL_SET_REGION_TABLE:
//        RETAILMSG(1, (TEXT("FMD_OEMIoControl IOCTL_SET_REGION_TABLE unsupported\r\n")));
//        return FALSE;
//        break;

        case IOCTL_DISK_DELETE_SECTORS:
        {
            DELETE_SECTOR_INFO* pDeleteInfo = (DELETE_SECTOR_INFO*) pInBuf;
            RETAILMSG(DBG_IOCTL,  (TEXT("FlashFX:FMD_OEMIoControl() IOCTL_DISK_DELETE_SECTORS unsupported\r\n")));
            RETAILMSG(DBG_IOCTL,  (TEXT("DELETE_SECTOR_INFO:\r\n")));
            RETAILMSG(DBG_IOCTL,  (TEXT(" - cbSize     : 0x%08X (%d)\r\n"), pDeleteInfo->cbSize, pDeleteInfo->cbSize));
            RETAILMSG(DBG_IOCTL,  (TEXT(" - startSector: 0x%08X (%d)\r\n"), pDeleteInfo->startsector, pDeleteInfo->startsector));
            RETAILMSG(DBG_IOCTL,  (TEXT(" - numsectors : 0x%08X (%d)\r\n"), pDeleteInfo->numsectors, pDeleteInfo->numsectors));
            return TRUE;
        }
        break;

        case IOCTL_DISK_GET_STORAGEID:
        {
            STORAGE_IDENTIFICATION* pStorageInfo = (STORAGE_IDENTIFICATION*) pOutBuf;

            RETAILMSG(DBG_IOCTL,  (TEXT("FlashFX:FMD_OEMIoControl() IOCTL_DISK_GET_STORAGEID\r\n")));

            if (!pOutBuf || nOutBufSize < sizeof(STORAGE_IDENTIFICATION))
            {
                DEBUGMSG(DBG_IOCTL,  (TEXT("FlashFX:FMD_OEMIoControl() IOCTL_DISK_GET_STORAGEID bad parameter(s).\r\n")));
                return FALSE;
            }

            pStorageInfo->dwSize = sizeof(STORAGE_IDENTIFICATION);
            pStorageInfo->dwFlags = MANUFACTUREID_INVALID | SERIALNUM_INVALID;
            pStorageInfo->dwManufactureIDOffset = 0;
            pStorageInfo->dwSerialNumOffset = 0;
            return TRUE;
        }
        break;

        case IOCTL_FMD_SET_SECTORSIZE:
        {
            RETAILMSG(DBG_IOCTL,  (TEXT("FlashFX:FMD_OEMIoControl() - IOCTL_FMD_SET_SECTORSIZE\r\n")));

            if (!pInBuf || nInBufSize < sizeof(DWORD))
            {
                DEBUGMSG(DBG_IOCTL,  (TEXT("FlashFX:FMD_OEMIoControl() IOCTL_FMD_SET_SECTORSIZE bad parameter(s).\r\n")));
                return FALSE;
            }

            //FlashFX_FMD.pageSize = *(PDWORD)pInBuf;
        }
        break;

        case IOCTL_FMD_RAW_WRITE_BLOCKS:
        {
            //        if (!pInBuf || nInBufSize < sizeof(RawWriteBlocksReq))
            //        {
            //            DEBUGMSG(1, (TEXT("FMD_OEMIoControl: IOCTL_FMD_RAW_WRITE_BLOCKS bad parameter(s).\r\n")));
            //            return FALSE;
            //        }
            //        return RawWriteBlocks ((PRawWriteBlocksReq)pInBuf);

            RETAILMSG(DBG_IOCTL,  (TEXT("FlashFX:FMD_OEMIoControl() RAW_WRITE_BLOCK unsupported\r\n")));

            return FALSE;
        }
        break;


        case IOCTL_FMD_GET_RAW_BLOCK_SIZE:
        {
            RETAILMSG(DBG_IOCTL, (TEXT("FlashFX:FMD_OEMIoControl() - IOCTL_FMD_GET_RAW_BLOCK_SIZE\r\n")));

            if (!pOutBuf || nOutBufSize < sizeof(DWORD))
            {
                DEBUGMSG(DBG_IOCTL, (TEXT("FlashFX:FMD_OEMIoControl() IOCTL_FMD_GET_RAW_BLOCK_SIZE bad parameter(s).\r\n")));
                return FALSE;
            }

            //*((PDWORD)pOutBuf) = FlashFX_FMD.pFlashInfo->pagePerBlock * FlashFX_FMD.pFlashInfo->pageSize;

            return TRUE;
        }
        break;

        case IOCTL_FMD_GET_INFO:
        {
            RETAILMSG(DBG_IOCTL, (TEXT("FlashFX:FMD_OEMIoControl() IOCTL_FMD_GET_INFO\r\n")));
            if (!pOutBuf || nOutBufSize < sizeof(FMDInfo))
            {
                DEBUGMSG(DBG_IOCTL, (TEXT("FlashFX:FMD_OEMIoControl() IOCTL_FMD_GET_INFO bad parameter(s).\r\n")));
                return FALSE;
            }
            else
            {
                PFMDInfo pInfo = (PFMDInfo)pOutBuf;
                pInfo->flashType = NAND;
                pInfo->dwBaseAddress = 0;
            //    pInfo->dwNumRegions = FlashFX_FMD.pFlashInfo->numBlocks;
                pInfo->dwNumReserved = 0;
            }
            return TRUE;
        }
        break;

        default:
            return FfxWM_FMD_OEMIoControl(dwIoControlCode, pInBuf, nInBufSize, pOutBuf, nOutBufSize, pBytesReturned);
    }

    RETAILMSG(DBG_IOCTL, (TEXT("FlashFX:FMD_OEMIoControl() returning TRUE\r\n")));

    return TRUE;
}



