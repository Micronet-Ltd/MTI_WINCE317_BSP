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
    support SSFDC style flash.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nthelpssfdc.c $
    Revision 1.4  2009/12/11 21:06:10Z  garyp
    Updated to use some functions which were renamed to avoid naming
    conflicts.  Use the more flexible NTMHOOKHANDLE rather than the old
    PNANDCTL.  
    Revision 1.3  2009/10/06 21:21:54Z  garyp
    Modified to use the revamped single-bit ECC calculation and
    correction functions.  Updated the "BuildSpareArea" functions to
    take a NANDCTL parameter rather than a Device handle.  Fixed
    documentation errors.
    Revision 1.2  2009/08/04 22:41:54Z  garyp
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

#if FFXCONF_NANDSUPPORT_SSFDC


/*-------------------------------------------------------------------
    The SmartMedia physical format spec, to which some NAND parts
    conform when it comes to marking blocks as bad from the factory,
    declares that byte 517 (the 6th byte of the spare area of a NAND
    page) of some page in that block will be some value other than
    0xFF if the block came bad from the factory.  In order to
    maximize our chances of discovering such factory bad blocks
    should the original BBM block list be destroyed, we attempt to
    establish this convention for all factory fresh bad blocks by
    calling the SetBlockStatus() function as the very first thing
    we do on virgin NAND flash.
-------------------------------------------------------------------*/
#define SSFDC_BAD_BLOCK_INDEX    5


/*-------------------------------------------------------------------
    Public: FfxNtmSSFDCBuildSpareArea()

    Initializes the spare area with the fixed values and the
    ECC generated from the data given using the SSFDC format.

    Note that this function is used to build arrays of spare
    areas when emulating larger page sizes with small-block
    NAND.

    Likewise when emulating larger page sizes, we only record
    the tag value in the first one, and therefore the pTag
    pointer may be NULL if we do not want to record the tag for
    this given spare area.

    *Note* -- This function is only appropriate for use where
              software ECC is being used.

    Parameters:
        hNtmHook    - The NTM Hook handle to use.  This value may be
                      NULL for some NTMs.
        pNtmInfo    - A pointer to the NTMINFO structure to use.
        pData       - A pointer to the data for which ECCs are to be
                      calculated.
        pSpare      - A pointer to the buffer in which to build the
                      spare data.
        pTag        - A pointer to the tag data to store in the
                      spare area.  This pointer may be NULL if the
                      tag value is to remain unset.
        fUseEcc     - A flag indicating whether ECCs should be used.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxNtmSSFDCBuildSpareArea(
    NTMHOOKHANDLE       hNtmHook,
    NTMINFO            *pNtmInfo,
    const D_BUFFER     *pData,
    D_BUFFER           *pSpare,
    const D_BUFFER     *pTag,
    D_BOOL              fUseEcc)
{
    D_UINT16            uSpareRemaining;

    DclAssert(pData);
    DclAssert(pSpare);
    DclAssert(pNtmInfo);
    DclAssert(pNtmInfo->uPageSize);
    DclAssert(pNtmInfo->uPageSize % DATA_BYTES_PER_ECC == 0);
    DclAssert(pNtmInfo->uSpareSize);

    uSpareRemaining = pNtmInfo->uSpareSize;

    DclMemSet(pSpare, ERASED8, pNtmInfo->uSpareSize);

    if(pTag)
        FfxNtmHelpTagEncode(&pSpare[NSSSFDC_TAG_OFFSET], pTag);

    if(fUseEcc)
    {
        D_BUFFER    abECC[MAX_ECC_BYTES_PER_PAGE];
        unsigned    nn = 0;

        FfxHookEccCalcStart(hNtmHook, pData, pNtmInfo->uPageSize, abECC, ECC_MODE_WRITE);
        FfxHookEccCalcRead(hNtmHook, pData, pNtmInfo->uPageSize, abECC, ECC_MODE_WRITE);

        while(uSpareRemaining)
        {
            /*  Build the ECC values
            */
            pSpare[NSSSFDC_FLAGS_OFFSET] = (D_UINT8)LEGACY_WRITTEN_WITH_ECC;

            pSpare[NSSSFDC_ECC1_OFFSET + 0] = abECC[nn++];
            pSpare[NSSSFDC_ECC1_OFFSET + 1] = abECC[nn++];
            pSpare[NSSSFDC_ECC1_OFFSET + 2] = abECC[nn++];

            pSpare[NSSSFDC_ECC2_OFFSET + 0] = abECC[nn++];
            pSpare[NSSSFDC_ECC2_OFFSET + 1] = abECC[nn++];
            pSpare[NSSSFDC_ECC2_OFFSET + 2] = abECC[nn++];

            uSpareRemaining -= NAND512_SPARE;
            pSpare          += NAND512_SPARE;
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Public: FfxNtmSSFDCCorrectPage()

    This function examines an SSFDC style page, checking the page
    flags and potentially the ECC to determine if the page is OK.
    If a failure is found, the ECC will attempt to correct the
    problem and return a status which will indicate the state of
    the data.

    Parameters:
        nSpareSize - The spare area size to use
        pData      - A pointer to page of data read from the flash.
        pSpare     - A pointer to spare area associated with the
                     given data sector.
        pabECC     - A pointer to an array of ECC bytes, typically
                     3 bytes for for every DATA_BYTES_PER_ECC worth
                     of data.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.

        ffxStat will be FFXSTAT_FIMCORRECTABLEDATA if one or more
        bit errors were corrected with ECC.

        If ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA, the ulCount
        field will indicate the number of correct segments that
        were found.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxNtmSSFDCCorrectPage(
    size_t              nSpareSize,
    D_BUFFER           *pData,
    D_BUFFER           *pSpare,
    const D_BUFFER     *pabECC)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMUNCORRECTABLEDATA);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "FfxNtmSSFDCCorrectPage() SpareSize=%u\n", nSpareSize));

    DclProfilerEnter("FfxNtmSSFDCCorrectPage", 0, 0);

    DclAssert(pData);
    DclAssert(pSpare);
    DclAssert(pabECC);
    DclAssert(nSpareSize);

    /*  Check the data block status, if it's clean, then the block is good
    */
    if(ISUNWRITTEN(pSpare[NSSSFDC_FLAGS_OFFSET]))
    {
        /*  TODO: For consistency, if we are using 2K pages, we really
            should be validating the bFlags fields for the full array of
            LEGACYNANDSPARE structures, as we do in the next clause.
        */
        ioStat.ulCount = 1;
        ioStat.ffxStat = FFXSTAT_SUCCESS;
    }
    else if(ISWRITTENWITHECC(pSpare[NSSSFDC_FLAGS_OFFSET]))
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
            if(!ISWRITTENWITHECC(pSpare[NSSSFDC_FLAGS_OFFSET]))
            {
                fUncorrectableError = TRUE;
            }

            if(FfxNtmHelpCorrectDataSegment(nCount, pData, &pSpare[NSSSFDC_ECC1_OFFSET], pabECC, &ioStat))
                fUncorrectableError = TRUE;

            nCount++;
            pData   += DATA_BYTES_PER_ECC;
            pabECC  += BYTES_PER_ECC;

            if(FfxNtmHelpCorrectDataSegment(nCount, pData, &pSpare[NSSSFDC_ECC2_OFFSET], pabECC, &ioStat))
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
      #if D_DEBUG > 1
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

/* IsValidPageCleanup: */

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxNtmSSFDCCorrectPage() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmSSFDCIsBadBlock()

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
FFXIOSTATUS FfxNtmSSFDCIsBadBlock(
    NTMHANDLE                   hNTM,
    PFNREADSPAREAREA            pfnReadSpareArea,
    const FFXNANDCHIP          *pChipInfo,
    D_UINT32                    ulBlock)
{
    D_UINT32                    ulPagesRemaining;
    FFXIOSTATUS                 ioStat;
    D_UINT32                    ulPage;
    D_UINT32                    ulPagesPerBlock;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "FfxNtmSSFDCIsBadBlock() ulBlock=%lX\n", ulBlock));

    DclAssert(hNTM);
    DclAssert(pfnReadSpareArea);
    DclAssert(pChipInfo);
    DclAssert(pChipInfo->pChipClass->uSpareSize == NAND512_SPARE);

    /* Compute the number of pages per block once, before the loop
    */
    DclAssert((pChipInfo->pChipClass->ulBlockSize % pChipInfo->pChipClass->uPageSize) == 0);
    ulPagesPerBlock = pChipInfo->pChipClass->ulBlockSize / pChipInfo->pChipClass->uPageSize;

    /*  SSFDC spec indicates that one of the first two pages in a block
        will contain the factory bad block marker, if any.
    */
    ulPagesRemaining = 2;

    ulPage = ulBlock * ulPagesPerBlock;

    while(ulPagesRemaining)
    {
        DCLALIGNEDBUFFER    (buffer, ns, NAND512_SPARE);

        /*  Read and scan the next common region of the flash.
        */
        ioStat = (*pfnReadSpareArea)(hNTM, ulPage, (void *)buffer.ns);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        {
            /*  If we failed to read the spare area, zero out the
                ulBlockStatus field, as it could have PageStatus
                information in there (since we just did a page
                oriented operation).
            */
            ioStat.op.ulBlockStatus = 0;
            goto IsSSFDCBadBlockCleanup;
        }

        if(buffer.ns[SSFDC_BAD_BLOCK_INDEX] != ERASED8)
        {
            ioStat.op.ulBlockStatus = BLOCKSTATUS_FACTORYBAD;
            goto IsSSFDCBadBlockCleanup;
        }

        ulPage++;
        ulPagesRemaining--;
    }

    ioStat.op.ulBlockStatus = BLOCKSTATUS_NOTBAD;

  IsSSFDCBadBlockCleanup:

    /*  We were doing page operations, so change the type back to block...
    */
    ioStat.ulFlags = IOFLAGS_BLOCK;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxNtmSSFDCIsBadBlock() Block=%lX returning %s\n",
        ulBlock, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmSSFDCSetBlockType()

    Attempts to mark the given block according to the specified
    ulBlockStatus value.

    Only two block types are supported:

    BLOCKSTATUS_FACTORYBAD - Mark the block bad by writing a
                             non-0xFF value to offset 5 in the
                             spare area for the first two pages
                             in a block.
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
FFXIOSTATUS FfxNtmSSFDCSetBlockType(
    NTMHANDLE           hNTM,
    PFNREADSPAREAREA    pfnReadSpareArea,
    PFNWRITESPAREAREA   pfnWriteSpareArea,
    const FFXNANDCHIP  *pChipInfo,
    D_UINT32            ulBlock,
    D_UINT32            ulBlockStatus)
{
    D_UINT32            ulPagesRemaining;
    FFXIOSTATUS         ioStat = {0, FFXSTAT_BADPARAMETER, IOFLAGS_BLOCK};
    D_BOOL              fMarked = FALSE;
    D_UINT32            ulPage;
    D_UINT32            ulPagesPerBlock;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "FfxNtmSSFDCSetBlockType() Block=%lX Status=%lX\n", ulBlock, ulBlockStatus));

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
                if(spare.data[NSSSFDC_FACTORYBAD_OFFSET] != ERASED8)
                {
                    fMarked = TRUE;
                    goto SetBlockTypeCleanup;
                }

                spare.data[NSSSFDC_FACTORYBAD_OFFSET] = 0;

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
            else
            {
/*              DclAssert(ulBlockStatus == BLOCKSTATUS_BBMBLOCK); */

                /*  We don't do anything here because this mark is not
                    necessary for this kind of flash.
                */
            }
        }

        ulPage++;
        ulPagesRemaining--;
    }

  SetBlockTypeCleanup:

    /*  If at least one page was marked, consider it good.
    */
    if(fMarked)
        ioStat.ffxStat = FFXSTAT_SUCCESS;
    else
        ioStat.ffxStat = FFXSTAT_FIMIOERROR;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxNtmSSFDCSetBlockType() Block=%lX returning %s\n",
        ulBlock, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmSSFDCGetPageStatus()

    This function returns the page status for SSFDC style flash.

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
FFXIOSTATUS FfxNtmSSFDCGetPageStatus(
    NTMHANDLE           hNTM,
    PFNREADSPAREAREA    pfnReadSpareArea,
    D_UINT32            ulPage)
{
    FFXIOSTATUS         ioStat;
    DCLALIGNEDBUFFER    (buffer, ns, FFX_NAND_MAXSPARESIZE);
    D_BUFFER           *pNS = &buffer.ns[0];

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "FfxNtmSSFDCGetPageStatus() Page=%lX\n", ulPage));

    DclAssert(hNTM);
    DclAssert(pfnReadSpareArea);

    /*  Read and scan the next common region of the flash.
    */
    ioStat = (*pfnReadSpareArea)(hNTM, ulPage, (void *)buffer.ns);
    if(IOSUCCESS(ioStat, 1))
    {
        /*  If the tag area, including the ECC and check bytes, is within
            1 bit of being erased, then we know that a tag was not written.
            If it is anything else, we know a tag was written.  This function
            always uses standard size tags.
        */
        if(!FfxNtmHelpIsRangeErased1Bit(&buffer.ns[NSSSFDC_TAG_OFFSET], NSSSFDC_TAG_LENGTH))
            ioStat.op.ulPageStatus |= PAGESTATUS_SET_TAG_WIDTH(LEGACY_TAG_SIZE);

        if(ISWRITTENWITHECC(pNS[NSSSFDC_FLAGS_OFFSET]))
        {
            ioStat.op.ulPageStatus |= PAGESTATUS_WRITTENWITHECC;
        }
        else if(ISUNWRITTEN(pNS[NSSSFDC_FLAGS_OFFSET]))
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
        "FfxNtmSSFDCGetPageStatus() Page=%lX returning %s\n",
        ulPage, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


#endif  /* FFXCONF_NANDSUPPORT_SSFDC */
#endif  /* FFXCONF_NANDSUPPORT */


