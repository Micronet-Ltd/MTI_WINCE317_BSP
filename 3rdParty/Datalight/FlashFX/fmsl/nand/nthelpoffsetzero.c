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

    This module contains helper functions which are used by multiple NTMs to
    support "OffsetZero" style flash.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nthelpoffsetzero.c $
    Revision 1.4  2009/12/11 21:04:10Z  garyp
    Updated to use some functions which were renamed to avoid naming
    conflicts.  Use the more flexible NTMHOOKHANDLE rather than the old
    PNANDCTL.  Refactored FfxNtmOffsetZeroBuildSpareArea() into two 
    functions to allow easier use by hooks code.
    Revision 1.3  2009/10/06 21:21:43Z  garyp
    Modified to use the revamped single-bit ECC calculation and
    correction functions.  Updated the "BuildSpareArea" functions to
    take a NANDCTL parameter rather than a Device handle.  Fixed
    documentation errors.
    Revision 1.2  2009/08/04 22:41:42Z  garyp
    Merged from the v4.0 branch.  Re-checked in to work around the MKS
    branch first conundrum.
    Revision 1.1  2008/07/24 22:48:20Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NANDSUPPORT

#include <fimdev.h>
#include <ecc.h>
#include <nandconf.h>
#include <nandctl.h>
#include <errmanager.h>
#include "nandid.h"
#include "nand.h"

#if FFXCONF_NANDSUPPORT_OFFSETZERO


/*-------------------------------------------------------------------
    Public: FfxNtmOffsetZeroBuildSpareArea()

    Initialize an OffsetZero style spare area using the supplied
    values, calculating the ECC on the fly.  This function will
    initialize the tag, the ECC bytes, and the flags value to 
    indicate "Written With ECC".  All other fields will be 
    initialized to 0xFF.  If pTag is NULL and/or fUseEcc is
    FALSE, those respective fields will be set to 0xFF.

    Note that this function may be used to build arrays of spare
    areas when emulating larger page sizes with small-block NAND.

    Likewise when emulating larger page sizes, we only record the
    tag value in the first one, and therefore the pTag pointer may
    be NULL if we do not want to record the tag for this given 
    spare area.

    *Note* -- This function is typically employed where software
              ECC is being used.

    Parameters:
        hNtmHook - The NTM Hook handle to use.  This value may be
                   NULL for some NTMs.
        pNtmInfo - A pointer to the NTMINFO structure to use.
        pData    - A pointer to the data for which ECCs are to be
                   calculated.
        pSpare   - A pointer to the buffer in which to build the
                   spare data.
        pTag     - A pointer to the tag data to store in the spare
                   area.  This pointer may be NULL if the tag value
                   is to remain unset.
        fUseEcc  - A flag indicating whether ECCs should be used.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxNtmOffsetZeroBuildSpareArea(
    NTMHOOKHANDLE       hNtmHook,
    NTMINFO            *pNtmInfo,
    const D_BUFFER     *pData,
    D_BUFFER           *pSpare,
    const D_BUFFER     *pTag,
    D_BOOL              fUseEcc)
{
    DclAssert(pData);
    DclAssert(pSpare);
    DclAssert(pNtmInfo);
    DclAssert(pNtmInfo->uPageSize);
    DclAssert(pNtmInfo->uPageSize % DATA_BYTES_PER_ECC == 0);
    DclAssert(pNtmInfo->uSpareSize);

    DclMemSet(pSpare, ERASED8, pNtmInfo->uSpareSize);

    if(pTag)
        FfxNtmHelpTagEncode(&pSpare[NSOFFSETZERO_TAG_OFFSET], pTag);

    if(fUseEcc)
    {
        D_BUFFER    abECC[MAX_ECC_BYTES_PER_PAGE];
        size_t      nECCLen = BYTES_PER_ECC * pNtmInfo->uPageSize / DATA_BYTES_PER_ECC;

        DclAssert(pNtmInfo->uSpareSize >= ((nECCLen / (BYTES_PER_ECC * 2)) * NAND512_SPARE));

        FfxHookEccCalcStart(hNtmHook, pData, pNtmInfo->uPageSize, abECC, ECC_MODE_WRITE);
        FfxHookEccCalcRead(hNtmHook, pData, pNtmInfo->uPageSize, abECC, ECC_MODE_WRITE);

        FfxNtmOffsetZeroSpareAreaECCSet(pSpare, abECC, nECCLen);
    }

    return;
}


/*-------------------------------------------------------------------
    Public: FfxNtmOffsetZeroSpareAreaECCSet()

    Initialize an OffsetZero style spare area ECC data using the
    supplied values.  In addition to the ECC data, this function will
    set the flags value to indicate "Written With ECC".  All other
    fields will not be touched.

    This function will handle any abitrary number of ECC bytes, up
    to 6 per each 16 bytes of spare area.  Regardless of the count,
    they will be written sequentially to the standard OffsetZero
    locations in Datalight standard format.

    Parameters:
        pSpare    - A pointer to the buffer in which to build the 
                    spare data.  The length of this buffer must be
                    at least 16 bytes for each 6 total bytes of ECC
                    data, as specified by nECCLen.
        pabECC    - A pointer to the array of ECC bytes to use, 
                    which is nECCLen long. 
        nECCLen   - The number of bytes in pabECC. 

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxNtmOffsetZeroSpareAreaECCSet(
    D_BUFFER           *pSpare,
    const D_BUFFER     *pabECC,
    size_t              nECCLen)
{
    DclAssert(pSpare);
    DclAssert(pabECC);
    DclAssert(nECCLen);

    while(nECCLen)
    {
        size_t nIndex = 0;
        
            /*  Build the ECC values
            */
            pSpare[NSOFFSETZERO_FLAGS_OFFSET] = (D_UINT8)LEGACY_WRITTEN_WITH_ECC;

        /*  Process up to BYTES_PER_ECC (3) at a time in ECC area 1
        */
        while(nECCLen && nIndex < BYTES_PER_ECC)
        {
            pSpare[NSOFFSETZERO_ECC1_OFFSET + nIndex++] = *pabECC;
            pabECC++;
            nECCLen--;
        }

        nIndex = 0;

        /*  Process up to BYTES_PER_ECC (3) at a time in ECC area 2
        */
        while(nECCLen && nIndex < BYTES_PER_ECC)
        {
            pSpare[NSOFFSETZERO_ECC2_OFFSET + nIndex++] = *pabECC;
            pabECC++;
            nECCLen--;
        }

        /*  Bump to the next 16-byte segment
        */
        pSpare += NAND512_SPARE;
    }

    return;
}


/*-------------------------------------------------------------------
    Public: FfxNtmOffsetZeroCorrectPage()

    This function examines an "Offset Zero" style page, checking
    the page flags and potentially the ECC to determine if the
    page is OK.  If a failure is found, the ECC will attempt to
    correct the problem and return a status which will indicate
    the state of the data.

    Parameters:
        nSpareSize - The spare area size to use
        pData      - A pointer to page of data read from the flash.
        pSpare     - A pointer to spare area associated with the
                     given data sector.
        pabECC     - A pointer to an array of ECC bytes, typically
                     3 bytes for every DATA_BYTES_PER_ECC worth of
                     data.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.

        ffxStat will be FFXSTAT_FIMCORRECTABLEDATA if one or more
        bit errors were corrected with ECC.

        If ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA, the ulCount
        field will indicate the number of correct segments that
        were found.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxNtmOffsetZeroCorrectPage(
    size_t              nSpareSize,
    D_BUFFER           *pData,
    D_BUFFER           *pSpare,
    const D_BUFFER     *pabECC)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMUNCORRECTABLEDATA);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "FfxNtmOffsetZeroCorrectPage() SpareSize=%u\n", nSpareSize));

    DclProfilerEnter("FfxNtmOffsetZeroCorrectPage", 0, 0);

    DclAssert(pData);
    DclAssert(pSpare);
    DclAssert(pabECC);
    DclAssert(nSpareSize);
    DclAssert(nSpareSize % NAND512_SPARE == 0);

    /*  Check the data block status, if it's clean, then the block is good
    */
    if(ISUNWRITTEN(pSpare[NSOFFSETZERO_FLAGS_OFFSET]))
    {
        /*  TODO: For consistency, if we are using 2K pages, we really
            should be validating the bFlags fields for the full array of
            LEGACYNANDSPARE structures, as we do in the next clause.
        */
        ioStat.ulCount = 1;
        ioStat.ffxStat = FFXSTAT_SUCCESS;
    }
    else if(ISWRITTENWITHECC(pSpare[NSOFFSETZERO_FLAGS_OFFSET]))
    {
        unsigned    nCount = 0;
        D_BOOL      fUncorrectableError = FALSE;

        /*  Note that it is possible for a single-bit error to corrupt
            the status value.  Double-check it by testing whether it
            differs by a single bit.
        */
        while(nSpareSize)
        {
            /*  If dealing with multiple ECCs in a 2K page, all the
                block status values must be identical (except for the
                possible single-bit error).
            */
            if(!ISWRITTENWITHECC(pSpare[NSOFFSETZERO_FLAGS_OFFSET]))
            {
                fUncorrectableError = TRUE;
            }

            if(FfxNtmHelpCorrectDataSegment(nCount, pData, &pSpare[NSOFFSETZERO_ECC1_OFFSET], pabECC, &ioStat))
                fUncorrectableError = TRUE;

            nCount++;
            pData   += DATA_BYTES_PER_ECC;
            pabECC  += BYTES_PER_ECC;

            if(FfxNtmHelpCorrectDataSegment(nCount, pData, &pSpare[NSOFFSETZERO_ECC2_OFFSET], pabECC, &ioStat))
                fUncorrectableError = TRUE;

            nCount++;
            pData   += DATA_BYTES_PER_ECC;
            pabECC  += BYTES_PER_ECC;

            /*  Even if we found errors, attempt to correct errors in
                any subsequent areas.
            */
            nSpareSize  -= NAND512_SPARE;
            pSpare      += NAND512_SPARE;
        }

        /*  Return FFXSTAT_SUCCESS if none of the segments had uncorrectable
            errors.
        */
        if(!fUncorrectableError)
        {
            if (ioStat.op.ulPageStatus & PAGESTATUS_DATACORRECTED)
                ioStat.ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
            else
                ioStat.ffxStat = FFXSTAT_SUCCESS;
        }
      #if D_DEBUG > 2
        else
        {
            DclHexDump("Failing page:\n", HEXDUMP_UINT8, 16, nCount * DATA_BYTES_PER_ECC,
                       pData - (nCount * DATA_BYTES_PER_ECC));
            DclHexDump("Spare:\n", HEXDUMP_UINT8, 16, nCount * 8,
                       pSpare - ((nCount / 2) * NAND512_SPARE));
        }
      #endif
    }
    else
    {
        /*  We should never be checking for ECC on a page that is written
            without ECC.  BBM writes pages without ECC.  This condition can
            happen for other pages if more than a single bit error occurs in
            the flags field.
        */
        FFXPRINTF(2, ("Attempting to correct a page not written with ECC\n"));
    }

/*  IsValidPageCleanup: */

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxNtmOffsetZeroCorrectPage() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmOffsetZeroIsBadBlock()

    Parameters:
        hNTM             - The NTM handle
        pfnReadSpareArea - A pointer to the ReadSpareArea function
                           to use.
        pChipInfo        - A pointer to the FFXNANDCHIP structure
                           describing the chip being used.
        ulBlock          - The block to mark.  Must have been
                           adjusted for reserved space (if any).

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.  If ffxStat is FFXSTAT_SUCCESS, the
        op.ulBlockStatus field will contain the block status
        bits pertaining to bad blocks.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxNtmOffsetZeroIsBadBlock(
    NTMHANDLE                   hNTM,
    PFNREADSPAREAREA            pfnReadSpareArea,
    const FFXNANDCHIP          *pChipInfo,
    D_UINT32                    ulBlock)
{
    D_UINT32                    ulPagesRemaining;
    FFXIOSTATUS                 ioStat;
    D_UINT32                    ulPage;
    D_UINT32                    ulPagesPerBlock;
    D_BOOL                      fCheckLastPage;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "FfxNtmOffsetZeroIsBadBlock() Block=%lX\n", ulBlock));

    DclAssert(hNTM);
    DclAssert(pfnReadSpareArea);
    DclAssert(pChipInfo);
    DclAssert(pChipInfo->pChipClass->uSpareSize <= FFX_NAND_MAXSPARESIZE);

    /* Compute the number of pages per block once, before the loop
    */
    DclAssert((pChipInfo->pChipClass->ulBlockSize % pChipInfo->pChipClass->uPageSize) == 0);
    ulPagesPerBlock = pChipInfo->pChipClass->ulBlockSize / pChipInfo->pChipClass->uPageSize;

    /*  If there is a factory bad block mark, it will be in one of the
        first two pages, or possibly the last page if the device is so
        specified.
    */
    ulPagesRemaining = 2;

    ulPage = ulBlock * ulPagesPerBlock;
    fCheckLastPage = pChipInfo->pChipClass->fLastPageFBB;
    while(ulPagesRemaining)
    {
        /*  Note that this buffer <must> be large enough to hold a
            2KB-page spare area (64-bytes) regardless what our max
            NAND page size might be.  This operates on physical spare
            areas, not emulated ones.
        */
        DCLALIGNEDBUFFER    (spare, data, FFX_NAND_MAXSPARESIZE);

        /*  Read and scan the next common region of the flash.
        */
        ioStat = (*pfnReadSpareArea)(hNTM, ulPage, (void *)spare.data);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        {
            /*  If we failed to read the spare area, zero out the
                ulBlockStatus field, as it could have PageStatus
                information in there (since we just did a page
                oriented operation).
            */
            ioStat.op.ulBlockStatus = 0;
            goto IsOffsetZeroBadBlockCleanup;
        }

      #if FFXCONF_MIGRATE_LEGACY_FLASH
        if(spare.data[NSLEGACY_FACTORYBAD_OFFSET] != ERASED8)
        {
            ioStat.op.ulBlockStatus = BLOCKSTATUS_FACTORYBAD;
            goto IsOffsetZeroBadBlockCleanup;
        }
        else
        {
            /*  NOTE: This may not catch expended allocations
            */

            /*  If there was something written in the old "hidden" area...
            */
            if(*(D_UINT32*)&spare.data[NSLEGACY_HIDDEN_OFFSET] != ERASED32)
            {
                D_UINT32    ulStatus;

                /*  See if the old block status value indicated that it
                    was normal block written with ECC.
                */
                ulStatus = *(D_UINT32*)&spare.data[NSLEGACY_BLOCKSTATUS_OFFSET];
                DCLNATIVE(&ulStatus, sizeof ulStatus);

                if(ulStatus == NSLEGACY_GOOD_ECC)
                {
                    /*  If so, we are going to assume that the stuff in
                        the hidden area really is hidden data, and not
                        a genuine factory bad block.  Report the block
                        as good.  Since we are likely in a BBM format
                        at the moment, the block will be erased, and
                        from here on out we will properly mark the block.
                    */
                    ioStat.op.ulBlockStatus = BLOCKSTATUS_NOTBAD;
                    goto IsOffsetZeroBadBlockCleanup;
                }
            }
        }
      #endif

        if(spare.data[0] != ERASED8)
        {
            ioStat.op.ulBlockStatus = BLOCKSTATUS_FACTORYBAD;
            goto IsOffsetZeroBadBlockCleanup;
        }
        else if((pChipInfo->bFlags & CHIPINT_MASK) == CHIPINT_16BIT)
        {
            /*  For 16-bit interfaces, one of the first two bytes
                will be marked.
            */
            if(spare.data[1] != ERASED8)
            {
                ioStat.op.ulBlockStatus = BLOCKSTATUS_FACTORYBAD;
                goto IsOffsetZeroBadBlockCleanup;
            }
        }

        ulPage++;
        ulPagesRemaining--;
        if (!ulPagesRemaining && fCheckLastPage)
        {
            ulPagesRemaining = 1;
            fCheckLastPage = FALSE;
            /*  Last page in a block = start page + (number of pages -1):
            */
            ulPage = ulBlock * ulPagesPerBlock + ulPagesPerBlock - 1;
        }
    }

    ioStat.op.ulBlockStatus = BLOCKSTATUS_NOTBAD;

  IsOffsetZeroBadBlockCleanup:

    /*  We were doing page operations, so change the type back to block...
    */
    ioStat.ulFlags = IOFLAGS_BLOCK;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxNtmOffsetZeroIsBadBlock() Block=%lX returning %s\n",
        ulBlock, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmOffsetZeroSetBlockType()

    Attempts to mark the given block according to the specified
    ulBlockStatus value.

    Only two block types are supported:

    BLOCKSTATUS_FACTORYBAD - Mark the block bad by writing a
                             non-0xFF value to offset 0 in the
                             spare area for the first two pages
                             in a block (non 0xFFFF if 16-bit
                             flash).
    BLOCKSTATUS_BBMBLOCK   - Mark the block as one that is being
                             used by BBM.  Since BBM does not
                             write with ECC, we cannot rely on
                             that mark to identify all blocks
                             which have been used by FlashFX.
                             When resizing disks, this BBM mark
                             will prevent blocks which were
                             previously used by BBM from being
                             considered "factory-bad".

    Parameters:
        hNTM              - The NTM handle
        pfnReadSpareArea  - A pointer to the ReadSpareArea function
                            to use
        pfnWriteSpareArea - A pointer to the WriteSpareArea function
                            to use
        pChipInfo         - A pointer to the FFXNANDCHIP structure
                            describing the chip being used.
        ulBlock           - The block to mark.  Must have been
                            adjusted for reserved space (if any).
        ulBlockStatus     - The block status value.

    Return Value:
        Returns an FFXIOSTATUS structure describing the state of
        the operation.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxNtmOffsetZeroSetBlockType(
    NTMHANDLE           hNTM,
    PFNREADSPAREAREA    pfnReadSpareArea,
    PFNWRITESPAREAREA   pfnWriteSpareArea,
    const FFXNANDCHIP  *pChipInfo,
    D_UINT32            ulBlock,
    D_UINT32            ulBlockStatus)
{
    D_UINT32            ulPagesRemaining;
    FFXIOSTATUS         ioStat = DEFAULT_BLOCKIO_STATUS;
    D_BOOL              fMarked = FALSE;
    D_UINT32            ulPage;
    D_UINT32            ulPagesPerBlock;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "FfxNtmOffsetZeroSetBlockType() Block=%lX Status=%lX\n", ulBlock, ulBlockStatus));

    DclAssert(hNTM);
    DclAssert(pfnReadSpareArea);
    DclAssert(pfnWriteSpareArea);
    DclAssert(pChipInfo);
    DclAssert(ulBlockStatus == BLOCKSTATUS_FACTORYBAD);

    /*  We only need to write the mark in one of the first two pages (as of
        FlashFX 3.0, BBM does not require every page to be marked).
    */
    ulPagesRemaining = 2;

    /*  Compute the number of pages per block once, before the loop
    */
    DclAssert((pChipInfo->pChipClass->ulBlockSize % pChipInfo->pChipClass->uPageSize) == 0);
    ulPagesPerBlock = pChipInfo->pChipClass->ulBlockSize / pChipInfo->pChipClass->uPageSize;

    ulPage = ulBlock * ulPagesPerBlock;

    while(ulPagesRemaining)
    {
        /*  Note that this buffer <must> be large enough to hold a spare
            area for the maximum size NAND page we support.
        */
        DCLALIGNEDBUFFER    (spare, data, FFX_NAND_MAXSPARESIZE);

        /*  Write out the data and ignore error
        */
        ioStat = (*pfnReadSpareArea)(hNTM, ulPage, (void *)spare.data);
        if(IOSUCCESS(ioStat, 1))
        {
            if(ulBlockStatus == BLOCKSTATUS_FACTORYBAD)
            {
                if(spare.data[NSOFFSETZERO_FACTORYBAD_OFFSET+0] != ERASED8)
                {
                    fMarked = TRUE;
                    goto OffsetZeroSetBlockTypeCleanup;
                }
                else if((pChipInfo->bFlags & CHIPINT_MASK) == CHIPINT_16BIT)
                {
                    /*  For 16-bit interfaces, one of the first two bytes
                        will be marked.
                    */
                    if(spare.data[NSOFFSETZERO_FACTORYBAD_OFFSET+1] != ERASED8)
                    {
                        fMarked = TRUE;
                        goto OffsetZeroSetBlockTypeCleanup;
                    }
                }

                spare.data[NSOFFSETZERO_FACTORYBAD_OFFSET+0] = 0;
                spare.data[NSOFFSETZERO_FACTORYBAD_OFFSET+1] = 0;

                ioStat = (*pfnWriteSpareArea)(hNTM, ulPage, (void *)spare.data);
                if(IOSUCCESS(ioStat, 1))
                {
                    fMarked = TRUE;
                }
                else
                {
                    /*  Since the block is failing, keep trying.  So long
                        as at least one page is updated, we consider this
                        operation to be good.
                    ioStat.ffxStat = FFXSTAT_FIMIOERROR;
                    break;
                    */
                }
            }
        }

        ulPage++;
        ulPagesRemaining--;
    }

  OffsetZeroSetBlockTypeCleanup:

    /*  If at least one page was marked, consider it good.
    */
    if(fMarked)
        ioStat.ffxStat = FFXSTAT_SUCCESS;
    else
        ioStat.ffxStat = FFXSTAT_FIMIOERROR;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxNtmOffsetZeroSetBlockType() Block=%lX returning %s\n",
        ulBlock, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmOffsetZeroGetPageStatus()

    This function returns the page status for OffsetZero style flash.

    Parameters:
        hNTM             - The NTM handle
        pfnReadSpareArea - A pointer to the ReadSpareArea function
                           to use.
        pChipClass       - A pointer to the FFXNANDCHIPCLASS
                           describing the chip being used.
        ulPage           - The page from which to return status.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.  If ffxStat is FFXSTAT_SUCCESS, the
        op.ulBlockStatus field will contain the block status
        bits pertaining to bad blocks.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxNtmOffsetZeroGetPageStatus(
    NTMHANDLE           hNTM,
    PFNREADSPAREAREA    pfnReadSpareArea,
    D_UINT32            ulPage)
{
    FFXIOSTATUS         ioStat;
    DCLALIGNEDBUFFER    (buffer, ns, FFX_NAND_MAXSPARESIZE);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "FfxNtmOffsetZeroGetPageStatus() Page=%lX\n", ulPage));

    DclAssert(hNTM);
    DclAssert(pfnReadSpareArea);

    /*  Read the spare area
    */
    ioStat = (*pfnReadSpareArea)(hNTM, ulPage, (void *)buffer.ns);
    if(IOSUCCESS(ioStat, 1))
    {
        /*  If the tag area, including the ECC and check bytes, is within
            1 bit of being erased, then we know that a tag was not written.
            If it is anything else, we know a tag was written.  This function
            always uses standard size tags.
        */
        if(!FfxNtmHelpIsRangeErased1Bit(&buffer.ns[NSOFFSETZERO_TAG_OFFSET], NSOFFSETZERO_TAG_LENGTH))
            ioStat.op.ulPageStatus |= PAGESTATUS_SET_TAG_WIDTH(LEGACY_TAG_SIZE);

        if(ISWRITTENWITHECC(buffer.ns[NSOFFSETZERO_FLAGS_OFFSET]))
        {
            ioStat.op.ulPageStatus |= PAGESTATUS_WRITTENWITHECC;
        }
        else if(ISUNWRITTEN(buffer.ns[NSOFFSETZERO_FLAGS_OFFSET]))
        {
            ioStat.op.ulPageStatus |= PAGESTATUS_UNWRITTEN;
        }
        else
        {
            ioStat.op.ulPageStatus |= PAGESTATUS_UNKNOWN;
            FFXPRINTF(2, ("Warning: Unknown page status, page = %lX\n", ulPage));
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxNtmOffsetZeroGetPageStatus() Page=%lX returning %s\n",
        ulPage, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


#endif  /* FFXCONF_NANDSUPPORT_OFFSETZERO */
#endif  /* FFXCONF_NANDSUPPORT */


