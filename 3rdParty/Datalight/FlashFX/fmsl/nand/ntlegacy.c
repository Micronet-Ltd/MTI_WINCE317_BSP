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

    This module contains helper functions for dealing with legacy, Toshiba-
    style "AnyBitZero" flash.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ntlegacy.c $
    Revision 1.23  2009/12/11 21:06:39Z  garyp
    Eliminated nthelp.h.
    Revision 1.22  2009/07/24 22:41:59Z  garyp
    Merged from the v4.0 branch.  Conditioned all the code on 
    FFXCONF_NANDSUPPORT_LEGACY.
    Revision 1.21  2009/04/13 17:38:22Z  glenns
    - Fix Bugzilla #2596: Remove erroneous assertion which could be
      triggered by a valid AnyBitZero bad block marker.
    Revision 1.20  2009/04/02 14:34:07Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.19  2009/03/18 05:27:18Z  glenns
    - Fix Bugzilla #2370: Removed references to obsolete block status
      value. Okay here because on-media compatibility with pre-3.0
      versions of FlashFX is not a requirement for FlashFX Tera.
    Revision 1.18  2009/01/19 04:34:41Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.17  2008/12/12 07:54:57Z  keithg
    Updated to conditionally use the deprecated xxx_BBMBLOCK type.
    Revision 1.16  2008/06/16 13:23:02Z  thomd
    Renamed ChipClass field to match higher levels;
    propagate chip capability fields in Create routine
    Revision 1.15  2008/03/28 17:53:00Z  Garyp
    Modified to use DEFAULT_BLOCKIO_STATUS (not a functional change).
    Revision 1.14  2008/02/03 05:31:00Z  keithg
    comment updates to support autodoc
    Revision 1.13  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.12  2007/09/12 22:24:01Z  Garyp
    Fixed FfxNtmLegacyIsBadBlock() so that the block status is properly set
    on exit.
    Revision 1.11  2007/08/01 15:17:07Z  timothyj
    Added minor assertions that flash block size is evenly divisible by the
    page size.
    Revision 1.10  2007/04/02 18:01:38Z  rickc
    Fix for bug 928.  Removed requirement that structure had to match on-media
    spare area format.
    Revision 1.9  2007/02/06 20:28:41Z  timothyj
    Updated interfaces to use blocks and pages instead of linear byte offsets.
    Revision 1.8  2006/03/10 02:29:25Z  Garyp
    Added the ability to mark blocks as used by BBM, and renamed the NTM
    helper functions from "MarkBlockBad" to "SetBlockType".
    Revision 1.7  2006/03/03 19:32:10Z  Garyp
    Updated the spare area handlers to use the FFXNANDCHIP flags field to
    determine the proper behavior in reading and writing the spare area.
    Revision 1.6  2006/02/26 02:58:05Z  Garyp
    Header update.
    Revision 1.5  2006/02/24 04:31:11Z  Garyp
    Updated to use refactored headers.
    Revision 1.4  2006/02/23 20:48:15Z  Garyp
    Modified to use helper functions in the NTMs for SpareRead/Write() rather
    than the regular NTM entry points, to properly account for the reserved
    space adjustment.
    Revision 1.3  2006/02/09 22:28:14Z  Garyp
    Updated debugging code.
    Revision 1.2  2006/01/25 04:00:43Z  Garyp
    Updated to conditionally build only if NAND support is enabled.
    Revision 1.1  2005/12/03 03:00:48Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NANDSUPPORT

#include <fimdev.h>
#include <nandconf.h>
#include <nandctl.h>
#include "nandid.h"
#include "nand.h"

#if FFXCONF_NANDSUPPORT_LEGACY


/*-------------------------------------------------------------------
    Public: FfxNtmLegacyIsBadBlock()

    This function determines if the given block is marked bad.

    Parameters:
        hNTM             - The NTM handle
        pfnReadSpareArea - A pointer to the ReadSpareArea function
                           to use
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
FFXIOSTATUS FfxNtmLegacyIsBadBlock(
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
        "FfxNtmLegacyIsBadBlock() Block=%lX\n", ulBlock));

    DclAssert(hNTM);
    DclAssert(pfnReadSpareArea);
    DclAssert(pChipInfo->pChipClass);
    DclAssert(pChipInfo->pChipClass->uSpareSize == NAND512_SPARE);

    DclAssert((pChipInfo->pChipClass->ulBlockSize % pChipInfo->pChipClass->uPageSize) == 0);
    ulPagesPerBlock = pChipInfo->pChipClass->ulBlockSize / pChipInfo->pChipClass->uPageSize;

    /*  We would have written the mark in one of the first two pages, so
        that's all we need to scan.
    */
    ulPagesRemaining = 2;

    ulPage = ulBlock * ulPagesPerBlock;

    while(ulPagesRemaining)
    {
        DCLALIGNEDBUFFER    (buffer, ns, NAND512_SPARE);
        D_BUFFER           *pNS = &buffer.ns[0];

        ioStat = (*pfnReadSpareArea)(hNTM, ulPage, (void *)buffer.ns);
        if(!IOSUCCESS(ioStat, 1))
        {
            /*  If we failed to read the spare area, zero out the
                ulBlockStatus field, as it could have PageStatus
                information in there (since we just did a page
                oriented operation).
            */
            ioStat.op.ulBlockStatus = 0;
            goto IsLegacyBadBlockCleanup;
        }

        /*  If our contrived factory mark is set, it is bad
        */
        if(pNS[NSSSFDC_FACTORYBAD_OFFSET] != ERASED8)
        {
            ioStat.op.ulBlockStatus = BLOCKSTATUS_FACTORYBAD;
            goto IsLegacyBadBlockCleanup;
        }

        /*  If our unambiguous FlashFX Pro v3.x + mark is there,
            the block is good.
        */
        if(ISWRITTENWITHECC(pNS[NSSSFDC_FLAGS_OFFSET]))
        {
            ioStat.op.ulBlockStatus = BLOCKSTATUS_NOTBAD;
            goto IsLegacyBadBlockCleanup;
        }

        /*  FFX-Pro 3.x+:  BBM blocks are not written with ECC, but do
            use a different mark so we know they were written by FlashFX.
        */
        if(ISWRITTENBYBBM(pNS[NSSSFDC_FLAGS_OFFSET]))
        {
            ioStat.op.ulBlockStatus = BLOCKSTATUS_NOTBAD;
            goto IsLegacyBadBlockCleanup;
        }

        /*  Prior to FlashFX Pro v3, there was only a single bit
            distinguishing between ECC marked blocks, and clean
            blocks, leaving no 100% solid way of discerning them
            if there is a single bit error.
        */
        if((pNS[NSSSFDC_FLAGS_OFFSET] != LEGACY_UNWRITTEN) && (ISOLDWRITTENWITHECC(pNS[NSSSFDC_FLAGS_OFFSET])))
        {
            ioStat.op.ulBlockStatus = BLOCKSTATUS_NOTBAD;
            goto IsLegacyBadBlockCleanup;
        }

        ulPage++;
        ulPagesRemaining--;
    }

    /*  If we get here, we can't definitively tell if we are good or
        bad, should this be the type of flash where "any zero bit"
        is used to denote bad blocks.

        If this is an initial BBM format, we really DO care, therefore
        return a special flag, which will cause BBM format to do a more
        extensive examination.
    */
    if((pChipInfo->bFlags & CHIPFBB_MASK) == CHIPFBB_ANYBITZERO)
        ioStat.op.ulBlockStatus = BLOCKSTATUS_LEGACYNOTBAD;
    else
        ioStat.op.ulBlockStatus = BLOCKSTATUS_NOTBAD;

  IsLegacyBadBlockCleanup:

    /*  We were doing page operations, so change the type back to block...
    */
    ioStat.ulFlags = IOFLAGS_BLOCK;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxNtmLegacyIsBadBlock() Block=%lX returning %s\n",
        ulBlock, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmLegacySetBlockType()

    Attempts to mark the given according to the specified
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

    In both cases, this functionality is only required for old
    style Toshiba flash, which does not have a specific factory
    mark (which is why this function is called "legacy", and in
    fact does not even exist for other flash types).

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
FFXIOSTATUS FfxNtmLegacySetBlockType(
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
        "FfxNtmLegacySetBlockType() Block=%lX Status=%lX\n", ulBlock, ulBlockStatus));

    DclAssert(hNTM);
    DclAssert(pfnReadSpareArea);
    DclAssert(pfnWriteSpareArea);
    DclAssert(pChipInfo);
    DclAssert(pChipInfo->pChipClass->uSpareSize == NAND512_SPARE);
    DclAssert(ulBlockStatus == BLOCKSTATUS_FACTORYBAD);

    /*  We only need to write the mark in one of the first two pages (as of
        FlashFX 3.0, BBM does not require every page to be marked).
    */
    ulPagesRemaining = 2;

    /* Compute the number of pages per block once, before the loop
    */
    DclAssert((pChipInfo->pChipClass->ulBlockSize % pChipInfo->pChipClass->uPageSize) == 0);
    ulPagesPerBlock = pChipInfo->pChipClass->ulBlockSize / pChipInfo->pChipClass->uPageSize;

    ulPage = ulBlock * ulPagesPerBlock;


    while(ulPagesRemaining)
    {
        DCLALIGNEDBUFFER    (buffer, ns, NAND512_SPARE);
        D_BUFFER           *pNS = &buffer.ns[0];

        /*  Write out the data and ignore error
        */
        ioStat = (*pfnReadSpareArea)(hNTM, ulPage, (void *)buffer.ns);
        if(IOSUCCESS(ioStat, 1))
        {
            if(ulBlockStatus == BLOCKSTATUS_FACTORYBAD)
            {
                if(pNS[NSSSFDC_FACTORYBAD_OFFSET] == ERASED8)
                {
                    pNS[NSSSFDC_FACTORYBAD_OFFSET] = 0;

                    /*  Write out the data and ignore error
                    */
                    ioStat = (*pfnWriteSpareArea)(hNTM, ulPage, (void *)buffer.ns);
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
                    /*  At least one bit was already zero, so consider it marked
                    */
                    fMarked = TRUE;
                }
            }
            else
            {
                DclAssert(pNS[NSSSFDC_FACTORYBAD_OFFSET] == ERASED8);
                DclAssert(pNS[NSSSFDC_FLAGS_OFFSET] == ERASED8);

                pNS[NSSSFDC_FLAGS_OFFSET] = LEGACY_WRITTEN_BY_BBM;

                /*  Write out the data and ignore error
                */
                ioStat = (*pfnWriteSpareArea)(hNTM, ulPage, (void *)buffer.ns);
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

    /*  If at least one page was marked, consider it good.
    */
    if(fMarked)
        ioStat.ffxStat = FFXSTAT_SUCCESS;
    else
        ioStat.ffxStat = FFXSTAT_FIMIOERROR;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxNtmLegacySetBlockType() Block=%lX returning %s\n",
        ulBlock, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


#endif  /* FFXCONF_NANDSUPPORT_LEGACY */
#endif  /* FFXCONF_NANDSUPPORT */

