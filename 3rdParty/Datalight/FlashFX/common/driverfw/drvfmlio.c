/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2009 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more of these US
  patents: US#5860082, US#6260156.  Patents may be pending.

  In addition to civil penalties for infringement of copyright under appli-
  cable U.S. law, 17 U.S.C. 1204 provides criminal penalties for violation
  of (a) the restrictions on circumvention of copyright protection systems
  found in 17 U.S.C. 1201 and (b) the protections for the integrity of
  copyright management information found in 17 U.S.C. 1202.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  A SOURCE CODE AGREEMENT, NON-DISCLOSURE AGREEMENT, AND/OR SIMILAR BINDING
  CONTRACT BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    This module contains the Device Driver Framework functions to provide
    low level, FML access to a FlashFX Disk.

    This interface is <NOT> the standard interface for reading and writing
    data to a FlashFX Disk, but rather is used to provided direct access
    to flash data, without using the Allocator abstraction.

    Software using this interface <MUST> be aware of the restrictions which
    are in place for the type of flash memory being used.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvfmlio.c $
    Revision 1.2  2009/03/18 22:49:54Z  garyp
    Merged from the v4.0 branch.
    Revision 1.1.1.5  2009/03/18 22:49:54Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.1.1.4  2009/03/09 02:12:13Z  garyp
    Eliminated a deprecated header.
    Revision 1.1.1.3  2009/02/04 23:14:52Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.1.1.2  2008/10/09 17:23:42Z  garyp
    Tweaked a debug level.
    Revision 1.1  2008/09/07 21:49:04Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <fxfmlapi.h>


/*-------------------------------------------------------------------
    Public: FfxDriverFmlBlockErase()

    Erase blocks on a FlashFX Disk, using the FML API.

    Parameters:
        hDisk         - The disk handle
        ulStartBlock  - The starting block number
        ulCount       - The number of blocks to erase

    Return Value:
        Returns an FFXIOSTATUS structure indicating the results of
        the operation.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxDriverFmlBlockErase(
    FFXDISKHANDLE   hDisk,
    D_UINT32        ulStartBlock,
    D_UINT32        ulCount)
{
    FFXDISKINFO    *pDsk = *hDisk;
    FFXIOSTATUS     ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEINDENT),
        "FfxDriverFmlBlockErase() hDisk=%lX DISK%u Start=%lU Count=%lU\n",
        hDisk, pDsk->Conf.nDiskNum, ulStartBlock, ulCount));

    DclProfilerEnter("FfxDriverFmlBlockErase", 0, ulCount);

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
    if(pDsk->hVBF)
    {
        FFXPRINTF(1, ("NOTE: FfxDriverFmlBlockErase() is being used to erase a block on a DISK%u which has an allocator loaded.\n",
            pDsk->Conf.nDiskNum));
    }
  #endif

    DclAssert(pDsk);
    DclAssert(ulCount);

    FMLERASE_BLOCKS(pDsk->hFML, ulStartBlock, ulCount, ioStat);

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 2, TRACEUNDENT),
        "FfxDriverFmlBlockErase() returning status %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxDriverFmlSectorRead()

    Read sectors from a FlashFX Disk, using the FML API.

    Parameters:
        hDisk         - The disk handle
        ulStartSector - The starting sector number
        ulCount       - The number of sectors to read
        pBuffer       - A pointer to the buffer to fill

    Return Value:
        Returns an FFXIOSTATUS structure indicating the results of
        the operation.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxDriverFmlSectorRead(
    FFXDISKHANDLE   hDisk,
    D_UINT32        ulStartSector,
    D_UINT32        ulCount,
    D_BUFFER       *pBuffer)
{
    FFXDISKINFO    *pDsk = *hDisk;
    FFXIOSTATUS     ioStat;
    D_UINT32        ulStartPage;
    D_UINT32        ulPageCount;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEINDENT),
        "FfxDriverFmlSectorRead() hDisk=%lX DISK%u Start=%lU Count=%lU pBuff=%P\n",
        hDisk, pDsk->Conf.nDiskNum, ulStartSector, ulCount, pBuffer));

    DclProfilerEnter("FfxDriverFmlSectorRead", 0, ulCount);

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
    if(pDsk->hVBF)
    {
        FFXPRINTF(1, ("NOTE: FfxDriverFmlSectorRead() is being used to read from a DISK%u which has an allocator loaded.\n",
            pDsk->Conf.nDiskNum));
    }
  #endif

    DclAssert(pDsk);
    DclAssert(pDsk->ulSectorLength);
    DclAssert(ulCount);
    DclAssert(pBuffer);
    DclAssert(!pDsk->uSectorToPageShift);

    ulStartPage = ulStartSector << pDsk->uSectorToPageShift;
    ulPageCount = ulCount << pDsk->uSectorToPageShift;

    DclAssert(ulStartPage >> pDsk->uSectorToPageShift == ulStartSector);
    DclAssert(ulPageCount >> pDsk->uSectorToPageShift == ulCount);

    FMLREAD_PAGES(pDsk->hFML, ulStartPage, ulPageCount, pBuffer, ioStat);

    ioStat.ulCount >>= pDsk->uSectorToPageShift;

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 2, TRACEUNDENT),
        "FfxDriverFmlSectorRead() returning status %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxDriverFmlSectorWrite()

    Write sectors to a FlashFX Disk using the FML API.

    This function provides an extremely inefficient method for
    updating the data in a flash page -- read all the old data
    into a temporary memory block, erase the flash block, write
    the old and new data back out to the flash.  This is slow,
    and may cause premature failure of the flash device, due to
    excessive erases.

    If pBlockBuff is NULL, then sectors will be written exactly
    as specified -- meaning that it is up to the caller to ensure
    the block is erased first, as well as ensure that sectors are
    always written in order to the block.

    Parameters:
        hDisk         - The disk handle
        ulStartSector - The starting sector number
        ulCount       - The number of sectors to write
        pBuffer       - A pointer to the data to write
        pBlockBuff    - An erase block sized buffer to use for
                        temporarily holding data.  If this value is
                        NULL, then sectors will be written exactly
                        as specified.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the results of
        the operation.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxDriverFmlSectorWrite(
    FFXDISKHANDLE       hDisk,
    D_UINT32            ulStartSector,
    D_UINT32            ulCount,
    const D_BUFFER     *pBuffer,
    D_BUFFER           *pBlockBuff)
{
    FFXDISKINFO        *pDsk = *hDisk;
    D_UINT32            ulSectorsWritten = 0;
    D_UINT32            ulStartPage;
    D_UINT32            ulPageCount;
    unsigned            nPagesPerBlock;
    FFXIOSTATUS         ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEINDENT),
        "FfxDriverFmlSectorWrite() hDisk=%lX DISK%u Start=%lU Count=%lU pBuff=%P pBlockBuff=%P\n",
        hDisk, pDsk->Conf.nDiskNum, ulStartSector, ulCount, pBuffer, pBlockBuff));

    DclProfilerEnter("FfxDriverFmlSectorWrite", 0, ulCount);

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
    if(pDsk->hVBF)
    {
        FFXPRINTF(1, ("NOTE: FfxDriverFmlSectorWrite() is being used to write to DISK%u which has an allocator loaded.\n",
            pDsk->Conf.nDiskNum));
    }
  #endif

    DclAssert(pDsk);
    DclAssert(pDsk->ulSectorLength);
    DclAssert(ulCount);
    DclAssert(pBuffer);
    DclAssert(!pDsk->uSectorToPageShift);

    nPagesPerBlock = pDsk->ulBlockSize / pDsk->ulSectorLength;
    ulStartPage = ulStartSector << pDsk->uSectorToPageShift;
    ulPageCount = ulCount << pDsk->uSectorToPageShift;

    DclAssert(ulStartPage >> pDsk->uSectorToPageShift == ulStartSector);
    DclAssert(ulPageCount >> pDsk->uSectorToPageShift == ulCount);

    if(!pBlockBuff)
    {
        /*  No block buffer was provided, so assume the pages to be written
            are already erased.
        */
        FMLWRITE_PAGES(pDsk->hFML, ulStartPage, ulPageCount, pBuffer, ioStat);

        ulSectorsWritten += ioStat.ulCount >> pDsk->uSectorToPageShift;
    }
    else
    {
        while(ulPageCount)
        {
            D_UINT32    ulBlockNum = ulStartPage / nPagesPerBlock;
            D_UINT32    ulBlockPage = ulStartPage % nPagesPerBlock;
            D_UINT32    ulCount = DCLMIN(ulPageCount, nPagesPerBlock - ulBlockPage);

            /*  If we are not writing an entire erase block, then we need
                to read the original contents into a temporary buffer, so
                we can erase the block and then write everything out.
            */
            if(ulCount != nPagesPerBlock)
            {
                D_UINT32 ulTailPage = ulBlockPage + ulCount;

                /*  Read in the data from any sectors preceding the ones
                    we want to write out.
                */
                if(ulBlockPage)
                {
                    FMLREAD_PAGES(pDsk->hFML, ulBlockNum * nPagesPerBlock, ulBlockPage, pBlockBuff, ioStat);
                    if(!IOSUCCESS(ioStat, ulBlockPage))
                        break;
                }

                /*  Read in the data from any sectors following the ones
                    we want to write out.
                */
                if(ulTailPage != nPagesPerBlock)
                {
                    FMLREAD_PAGES(pDsk->hFML,
                        (ulBlockNum * nPagesPerBlock) + ulTailPage,
                        nPagesPerBlock - ulTailPage,
                        pBlockBuff + (ulTailPage * pDsk->ulSectorLength), ioStat);

                    if(!IOSUCCESS(ioStat, nPagesPerBlock - ulTailPage))
                        break;
                }

                /*  Place the new data into the proper spot in the buffer.
                */
                DclMemCpy(pBlockBuff + (ulBlockPage * pDsk->ulSectorLength),
                    pBuffer, ulCount * pDsk->ulSectorLength);

                FMLERASE_BLOCKS(pDsk->hFML, ulBlockNum, 1, ioStat);
                if(!IOSUCCESS(ioStat, 1))
                    break;

                /*  Write the pages out all at once.
                */
                FMLWRITE_PAGES(pDsk->hFML, ulBlockNum * nPagesPerBlock, nPagesPerBlock, pBlockBuff, ioStat);

                if(ioStat.ulCount > ulBlockPage)
                {
                    D_UINT32 ulWritten = DCLMIN(ioStat.ulCount - ulBlockPage, ulCount);

                    ulSectorsWritten += ulWritten >> pDsk->uSectorToPageShift;
                }

                if(!IOSUCCESS(ioStat, nPagesPerBlock))
                    break;
            }
            else
            {
                /*  We are writing the entire block, so just erase it
                    and write out the data.
                */
                FMLERASE_BLOCKS(pDsk->hFML, ulBlockNum, 1, ioStat);
                if(!IOSUCCESS(ioStat, 1))
                    break;

                FMLWRITE_PAGES(pDsk->hFML, ulBlockNum * nPagesPerBlock, ulCount, pBuffer, ioStat);

                ulSectorsWritten += ulCount >> pDsk->uSectorToPageShift;

                if(!IOSUCCESS(ioStat, ulCount))
                    break;
            }

            ulStartPage += ulCount;
            ulPageCount -= ulCount;
            pBuffer     += ulCount * pDsk->ulSectorLength;
        }
    }

    ioStat.ulCount = ulSectorsWritten;

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 2, TRACEUNDENT),
        "FfxDriverFmlSectorWrite() returning status %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


