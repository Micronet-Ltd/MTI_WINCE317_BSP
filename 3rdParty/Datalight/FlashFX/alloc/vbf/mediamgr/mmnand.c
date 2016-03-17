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

    This module contains the low level Media Manager code to support NAND
    flash.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: mmnand.c $
    Revision 1.45  2012/04/12 21:32:54Z  billr
    Resolve bug 3302: FfxMMNandReadHeader() handling of an ECC error in EUH
    page.
    Revision 1.44  2010/12/22 09:21:31Z  garyp
    Corrected the examination of a return value to avoid a signed/unsigned
    compiler warning with some tool chains.
    Revision 1.43  2009/12/31 17:24:42Z  billr
    Declare local functions static.
    Revision 1.42  2009/08/04 01:04:24Z  garyp
    Merged from the v4.0 branch.  Changed all uses of FMLREAD_CORRECTEDPAGES()
    to FMLREAD_PAGES().  Changed all uses of FMLREAD/WRITE_PAGES()
    to FMLREAD/WRITE_UNCORRECTEDPAGES().  Minor datatype changes.
    Documentation updated.
    Revision 1.41  2009/04/19 17:28:59Z  billr
    Resolve bug 2643: VBFTEST write interruption tests fail.
    Revision 1.40  2009/04/09 02:58:22Z  garyp
    Renamed a structure to avoid AutoDoc naming collisions.
    Revision 1.39  2009/03/31 16:35:30Z  davidh
    Function headers updated for AutoDoc
    Revision 1.38  2009/03/12 22:08:43Z  billr
    Resolve bug 2124: Page buffers are allocated on the stack.
    Revision 1.37  2009/03/09 21:21:15Z  billr
    Partial resolution of bug 2142: Page buffers are allocated on the stack.
    Revision 1.36  2009/01/29 19:47:45Z  billr
    Resolve bug 1601: VBF write protects the disk after an uncorrectable ECC
    error.  Implement new Media Manager function FfxMediaMgrCopyBadBlock(). 
    Allow per-instance data for media-specific functions.
    Revision 1.35  2009/01/23 22:44:50Z  billr
    Resolve bug 1838: EUH Invalidation may fail for MLC parts.  Overwrite
    the EUH tag only if DEV_PGM_ONCE is not set for the device.
    Revision 1.34  2009/01/22 00:34:17Z  billr
    Resolve bug 1562: implement scrubbing of correctable errors.
    Revision 1.33  2009/01/16 04:21:25Z  glenns
    Fixed up literal FFXIOSTATUS initialization in two places.
    Revision 1.32  2009/01/09 17:21:47Z  billr
    Simplify reporting of correctable errors.
    Revision 1.31  2009/01/08 00:16:30Z  billr
    FfxMMNandReadHeader() now returns a full FFXIOSTATUS, and
    propagates information about correctable errors.
    Revision 1.30  2008/06/03 19:43:57Z  thomd
    Added fFormatNoErase
    Revision 1.29  2008/03/17 18:55:08Z  Garyp
    Made the code conditional on FFXCONF_VBFSUPPORT.  Updated to accommodate
    interfaces which have changed to support variable length tags.
    Revision 1.28  2008/01/13 07:20:40Z  keithg
    Function header updates to support autodoc.
    Revision 1.27  2007/11/05 19:14:23Z  Garyp
    Corrected the debug trace messages -- no functional changes.
    Revision 1.26  2007/11/02 02:26:04Z  Garyp
    Eliminated the inclusion of limits.h.
    Revision 1.25  2007/11/01 18:58:32Z  billr
    Fix compiler warnings.
    Revision 1.24  2007/11/01 01:00:05Z  Garyp
    Major update to eliminate the inconsistent use of D_UINT16 types.  All
    block indexes and counts which were D_UINT16 are now consistently unsigned.
    Replaced the use of Return_t and IOReturn_t with FFXSTATUS and FFXIOSTATUS.
    Improved error handling.
    Revision 1.23  2007/10/16 23:41:49Z  Garyp
    Removed an extraneous call to FfxMMDataBlockNum().  Updated to use
    the DCLALIGNEDBUFFER() macro.
    Revision 1.22  2007/10/10 21:23:24Z  billr
    Resolve bug 412.
    Revision 1.21  2007/10/08 19:44:56Z  billr
    Resolve bug 970.
    Revision 1.20  2007/07/19 21:08:01Z  timothyj
    Minor comments and assertions.
    Revision 1.19  2007/06/16 23:13:36Z  Garyp
    Added debug code to display when I/O errors or corrected data is
    encountered.
    Revision 1.18  2007/04/19 17:27:53Z  timothyj
    Added fLargeAddressing parameter, to indicate whether the on-media format
    should use large flash array compatible addressing.
    Revision 1.17  2007/04/11 23:40:39Z  timothyj
    Modified AllocEntryPage() and FfxMMNandFormatEraseUnit() to use raw block
    offsets in lieu of linear byte offsets.
    Revision 1.16  2007/02/28 00:43:59Z  Garyp
    Updated to no longer use the deprecated "old" FML: interface.
    Revision 1.15  2006/11/08 20:40:11Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.14  2006/08/20 00:09:38Z  Garyp
    Added debug code.
    Revision 1.13  2006/05/08 08:52:11Z  Garyp
    Updated to use the new write interruption test interfaces.
    Revision 1.12  2006/03/18 01:09:28Z  Garyp
    Removed dead code and cleaned up comments -- no functional changes.
    Revision 1.11  2006/03/15 00:23:16Z  billr
    Improve debug output.
    Revision 1.10  2006/03/05 01:57:20Z  Garyp
    Modified the usage of FfxFmlDiskInfo() and FfxFmlDeviceInfo().
    Revision 1.9  2006/02/26 22:10:53Z  Garyp
    Modified so that all logic for handling the tag's check byte and ECC is done
    within the NTM layer.
    Revision 1.8  2006/02/21 02:53:10Z  Garyp
    Changed EUH fields back to using block terminology, rather than pages,
    to be consistent with other internal VBF fields (from a client perspective,
    VBF operates on pages).
    Revision 1.7  2006/02/16 01:25:14Z  Garyp
    Fixed an off-by-one error in AllocEntryPage().
    Revision 1.6  2006/02/15 18:32:23Z  thomd
    Comment out unused static function for gnu compile
    Revision 1.5  2006/02/15 09:36:24Z  Garyp
    Modified to no longer use FfxFmlOldRead/Write() for MEM_HIDDEN stuff.
    Revision 1.4  2006/02/08 20:21:26Z  Garyp
    Modified to use the updated FML interface.
    Revision 1.3  2006/01/07 01:34:12Z  billr
    Merge Sibley support from v2.01.
    Revision 1.2  2005/12/25 13:26:22Z  Garyp
    Added/updated debugging code -- no functional changes.
    Revision 1.1  2005/12/08 03:08:52Z  Garyp
    Initial revision
    Revision 1.4  2005/11/15 18:50:09Z  Garyp
    Debug code updated.
    Revision 1.3  2005/11/09 16:43:39Z  Garyp
    Eliminated the use of STD_NAND_DATA_SIZE.  Changed some terminology from
    "blocks" to "pages" for clarity.  Began changing from a compile-time block
    size to a run-time page size paradigm.
    Revision 1.2  2005/10/21 03:09:10Z  garyp
    Added debug code and improved error handling.
    Revision 1.1  2005/10/12 04:43:36Z  Garyp
    Initial revision
    Revision 1.3  2005/10/12 04:43:36Z  Garyp
    Fixed a stupid math bug causing writes longer than 32KB to fail.
    Revision 1.2  2005/10/11 17:30:24Z  Garyp
    Modified to use FfxFmlWritePages() rather than oemwrite(MEM_NAND_PAGE);
    Revision 1.1  2005/10/02 02:33:40Z  Garyp
    Initial revision
    Revision 1.2  2005/09/13 04:24:17Z  garyp
    Commented out an apparently bogus assert.
    Revision 1.1  2005/08/03 19:31:48Z  pauli
    Initial revision
    Revision 1.4  2005/08/03 19:31:48Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.2  2005/07/31 05:41:16Z  Garyp
    Updated to use new profiler leave function which now takes a ulUserData
    parameter.
    Revision 1.1  2005/07/25 21:11:10Z  pauli
    Initial revision
    Revision 1.2  2005/10/19 22:26:46Z  billr
    It's perfectly reasonable to read allocation entries until there are none
    left, and find out about that by a return of RESULT_ERANGE.
    Revision 1.1  2005/08/27 20:08:30Z  Cheryl
    Initial revision
    ---------------------
    Bill Roman 2003-03-14
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_VBFSUPPORT
#if FFXCONF_NANDSUPPORT

#include <vbf.h>
#include <fxfmlapi.h>
#include <vbfint.h>
#include <mediamgr.h>
#include <tstwint.h>
#include <errmanager.h>

#include "mmint.h"
#include "mmnand.h"

#define MMPREFIX    "FfxMMNand"

/*  Signature used in place of an allocation entry on NAND flash.  The
    first byte is initially neither all ones nor all zeros; it is set
    to all zeros to invalidate a unit.
*/
static const unsigned char NANDEuhSig[FFX_NAND_TAGSIZE+1] = {"\xE2" "H"};

struct MMDEVINFO
{
    PDCLMUTEX pMutex;    /*  Hold this mutex to use buffers */
    D_BUFFER *pMain;     /*  Buffers for CopyBadBlock, etc. */
    D_BUFFER *pSpare;
};


/*-------------------------------------------------------------------
    Internal function prototypes
-------------------------------------------------------------------*/
static D_UINT32  AllocEntryPage(const VBFDATA *pVBF, LinearUnit_t lnu, unsigned nBlock);



                /*---------------------------------*\
                 *                                 *
                 * Indirect Interface to MediaMgr  *
                 *                                 *
                \*---------------------------------*/



/*-------------------------------------------------------------------
    Protected: ReadBlocks()

    Read data from a unit in NAND flash.

    Parameters:
        pVBF        - A pointer to the VBFDATA structure to use
        lnu         - linear unit index
        nBlockIndex - starting block (allocation index) in unit
        nBlockCount - block count to read
        pClientData - buffer to fill

    Return Value:
        Returns an FFXIOSTATUS structure indicating the results of
        the request.
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadBlocks(
    const VBFDATA      *pVBF,
    LinearUnit_t        lnu,
    unsigned            nBlockIndex,
    unsigned            nBlockCount,
    D_BUFFER           *pClientData)
{
    FFXIOSTATUS         ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEINDENT),
        MMPREFIX":ReadBlocks() LNU=%U BlkIndex=%u BlkCount=%u\n", lnu, nBlockIndex, nBlockCount));

    DclProfilerEnter(MMPREFIX":ReadBlocks", 0, nBlockCount);

    DclAssert(pVBF);
    DclAssert(nBlockCount);
    DclAssert(pClientData);

    FMLREAD_PAGES(pVBF->hFML, FfxMMDataBlockNum(pVBF, lnu, nBlockIndex),
        nBlockCount, pClientData, ioStat);

    if (ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA
        || ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEMAIN)
    {
        /*  Casting ulCount to unsigned int is safe because
            nBlockCount was unsigned int.
        */
        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 1, TRACENODENT),
                        MMPREFIX ":ReadBlocks() correctable error unit %lu, block %u\n",
                        (unsigned long) lnu, nBlockIndex + (unsigned) ioStat.ulCount));

        if (FfxErrMgrDecodeRecommendation(&ioStat) == FFXERRPOLICY_NO_ACTION)
            FfxErrMgrKillError(&ioStat);
        else
            ioStat.ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEUNDENT),
        MMPREFIX":ReadBlocks() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Protected: WriteBlocks()

    Writes data to a unit in NAND flash.  The allocation block
    size for the partition MUST be equal to the standard NAND
    main page size.

    Parameters:
        pVBF        - A pointer to the VBFDATA structure to use
        lnu         - linear unit index
        nBlockIndex - starting block (allocation index) in unit
        nBlockCount - block count to write
        nBlockKey   - retrieval key (i.e., client address) for the
                      first block.  Keys for successive blocks are
                      formed by incrementing this value.
        pClientData - buffer containing data to write

    Return Value:
        Returns an FFXIOSTATUS structure indicating the results of
        the request.
-------------------------------------------------------------------*/
static FFXIOSTATUS WriteBlocks(
    const VBFDATA          *pVBF,
    LinearUnit_t            lnu,
    unsigned                nBlockIndex,
    unsigned                nBlockCount,
    unsigned                nBlockKey,
    const D_BUFFER         *pClientData)
{
    FFXIOSTATUS             ioStat = DEFAULT_PAGEIO_STATUS;
    FFXIOSTATUS             ioTotal = DEFAULT_PAGEIO_STATUS;
    D_UINT32                ulPageNum;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEINDENT),
        MMPREFIX":WriteBlocks() LNU=%U BlkIndex=%u BlkCount=%u BlkKey=%u\n",
        lnu, nBlockIndex, nBlockCount, nBlockKey));

    DclProfilerEnter(MMPREFIX":WriteBlocks", 0, nBlockCount);

    DclAssert(pVBF);
    DclAssert(nBlockCount);
    DclAssert(pClientData);

    ulPageNum = FfxMMDataBlockNum(pVBF, lnu, nBlockIndex);

    while(nBlockCount)
    {
        #define MAX_PAGES   (64)
        union
        {
            FFXNANDTAG      tags[MAX_PAGES];
            DCL_ALIGNTYPE   DummyAlign;
        } buffer;
        unsigned            ii;
        unsigned            nCount = DCLMIN(nBlockCount, MAX_PAGES);

        for(ii=0; ii<nCount; ii++)
        {
            D_UINT16 alloc = MAKE_ALLOC(MEDIA_ALLOC_TYPE_VALID, nBlockKey);

            /*  Stuff the allocation entry value into a tag field in a
                byte-order independent fashion.
            */
            buffer.tags[ii][0] = (D_UINT8)alloc;
            buffer.tags[ii][1] = (D_UINT8)(alloc >> CHAR_BIT);

            /*  This code needs attention if any tag size other
                than 2 is used.
            */
            #if FFX_NAND_TAGSIZE != 2
              #error "FFX: Tag size is not 2"
            #endif

            nBlockKey++;
        }

        /*  Write multiple pages.  Each page is written together with
            its tag in a single atomic operation.
        */
        FMLWRITE_TAGGEDPAGES(pVBF->hFML, ulPageNum, nCount, pClientData,
            (const D_BUFFER*)buffer.tags, FFX_NAND_TAGSIZE, ioStat);

        ioTotal.ulCount         += ioStat.ulCount;
        ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        DclAssert(ioStat.ulCount == nCount);

        FFX_ISSUE_WRITE_INTERRUPTION(pVBF->nDiskNum, WRITE_INT_MEDIAMGR1);
        FFX_ISSUE_WRITE_INTERRUPTION(pVBF->nDiskNum, WRITE_INT_MEDIAMGR2);
        FFX_ISSUE_WRITE_INTERRUPTION(pVBF->nDiskNum, WRITE_INT_MEDIAMGR3);
        FFX_ISSUE_WRITE_INTERRUPTION(pVBF->nDiskNum, WRITE_INT_MEDIAMGR4);
        FFX_ISSUE_WRITE_INTERRUPTION(pVBF->nDiskNum, WRITE_INT_MEDIAMGR5);

        pClientData += nCount * pVBF->uAllocationBlockSize;
        ulPageNum   += nCount;
        nBlockCount -= nCount;
    }

    ioTotal.ffxStat = ioStat.ffxStat;

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEUNDENT),
         MMPREFIX":WriteBlocks() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Protected: ReadAllocations()

    Read the some or all of the allocation list from a unit on
    NAND flash.  Allocation entries are read, validated, and
    converted to Allocation structures in rgaUnitAllocList.
    Processing stops when the end of the allocation list on the
    media is reached or when nAllocCount allocations have been
    processed, whichever comes first.

    Parameters:
        pVBF             - A pointer to the VBFDATA structure to use
        lnu              - linear unit index
        nStartAlloc      - index of first allocation entry to read
        nAllocCount      - maximum number of allocations to read
        rgaUnitAllocList - An array allocation structures to fill

    Return Value:
        Returns an FFXIOSTATUS structure indicating the results of
        the request.
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadAllocations(
    const VBFDATA          *pVBF,
    LinearUnit_t            lnu,
    unsigned                nStartAlloc,
    unsigned                nAllocCount,
    Allocation              rgaUnitAllocList[])
{
    FFXIOSTATUS             ioStat = DEFAULT_PAGEIO_STATUS;
    FFXIOSTATUS             ioTotal = DEFAULT_PAGEIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEINDENT),
        MMPREFIX":ReadAllocations() LNU=%U Start=%u Count=%u\n",
        lnu, nStartAlloc, nAllocCount));

    DclProfilerEnter(MMPREFIX":ReadAllocations", 0, nAllocCount);

    DclAssert(pVBF);
    DclAssert(nAllocCount);

    /*  Don't run off the end of the unit.
    */
    if(nStartAlloc >= pVBF->uBlocksPerUnit)
    {
        ioTotal.ffxStat = FFXSTAT_OUTOFRANGE;
        goto ReadAllocsCleanup;
    }

    /*  Don't try to process more allocation entries than are available
        in this unit.
    */
    nAllocCount = DCLMIN(nAllocCount, pVBF->uBlocksPerUnit - nStartAlloc);

    /*  Process allocations in chunks that fit in the local buffer.
    */
    while(nAllocCount)
    {
        union
        {
            FFXNANDTAG      tags[NAND_TAG_BUFFER_DIM];
            DCL_ALIGNTYPE   DummyAlign;
        } buffer;
        unsigned            ii;
        unsigned            nCount = DCLMIN(nAllocCount, DCLDIMENSIONOF(buffer.tags));

        FMLREAD_TAGS(pVBF->hFML,
                    AllocEntryPage(pVBF, lnu, nStartAlloc),
                    nCount, (D_BUFFER*)buffer.tags,
                    FFX_NAND_TAGSIZE, ioStat);

        /*  Whether or not ioStat.ffxStat indicates an error,
            ioStat.ulCount is always the number of tags read
            successfully.  The read could return fewer than the
            requested number of tags (like if there's a correctable
            error).  It will never return more.
        */
        ioTotal.ulCount         += ioStat.ulCount;
        ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;
        if (ioStat.ulCount < nCount)
            nCount = (unsigned int) ioStat.ulCount;
        DclAssert(ioStat.ulCount == nCount);

        for(ii = 0; ii < nCount; ++ii)
        {
            rgaUnitAllocList[ii].uBlkInUnit = nStartAlloc + ii;

            if(FfxMMIsErasedAlloc(buffer.tags[ii], FFX_NAND_TAGSIZE))
            {
                rgaUnitAllocList[ii].nState = ALLOC_FREE;
            }
            else
            {
                D_UINT16 key = (D_UINT16)(buffer.tags[ii][0] | (buffer.tags[ii][1] << CHAR_BIT));

                switch (MEDIA_ALLOC_TYPE(key))
                {
                    case MEDIA_ALLOC_TYPE_VALID:
                        rgaUnitAllocList[ii].nState = ALLOC_VALID;
                        rgaUnitAllocList[ii].uBlkInRegion = key & MEDIA_ALLOC_MASK;
                        break;

                    default:
                        /*  write interruption?
                        */
                        FFXPRINTF(2, (MMPREFIX":ReadAllocations() %X in allocation %u in unit %lu\n",
                                      key, nStartAlloc+ii, (unsigned long) lnu));
                        rgaUnitAllocList[ii].nState = ALLOC_BAD;
                        break;
                }
            }
        }

        if (ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA
            || ioStat.ffxStat == FFXSTAT_FIMCORRECTABLESPARE)
        {
            /*  Casting ulCount to unsigned int is safe because
                nBlockCount was unsigned int.
            */
            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 1, TRACENODENT),
                            MMPREFIX ":ReadAllocations() correctable error unit %lu, block %u\n",
                            (unsigned long) lnu, nStartAlloc + (unsigned) ioTotal.ulCount));

            if (FfxErrMgrDecodeRecommendation(&ioStat) == FFXERRPOLICY_NO_ACTION)
                FfxErrMgrKillError(&ioStat);
            else
                ioStat.ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
        }

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        nStartAlloc         += nCount;
        rgaUnitAllocList    += nCount;
        nAllocCount         -= nCount;
    }

    ioTotal.ffxStat = ioStat.ffxStat;

  ReadAllocsCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEUNDENT),
        MMPREFIX":ReadAllocations() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Protected: ExpendAllocations()

    Mark free allocations as no longer valid for use.  They will
    be reported by ReadLnuAllocations with state ALLOC_BAD.

    The specified allocations must all have state ALLOC_FREE when
    this function is called.

    Parameters:
        pVBF        - A pointer to the VBFDATA structure to use
        lnu         - linear unit index
        nFirstAlloc - first allocation to mark discarded.
        nAllocCount - number of allocations to discard.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the results of
        the request.
-------------------------------------------------------------------*/
static FFXIOSTATUS ExpendAllocations(
    const VBFDATA          *pVBF,
    LinearUnit_t            lnu,
    unsigned                nFirstAlloc,
    unsigned                nAllocCount)
{
    union
    {
        FFXNANDTAG          tags[NAND_TAG_BUFFER_DIM];
        DCL_ALIGNTYPE       DummyAlign;
    } buffer;
    FFXIOSTATUS             ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    FFXIOSTATUS             ioTotal = DEFAULT_PAGEIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEINDENT),
        MMPREFIX":ExpendAllocations() LNU=%U BlkIndex=%u BlkCount=%u\n",
        lnu, nFirstAlloc, nAllocCount));

    DclProfilerEnter(MMPREFIX":ExpendAllocations", 0, nAllocCount);

    /*  Don't run off the end of the unit.
    */
    if(nFirstAlloc >= pVBF->uBlocksPerUnit
       || nAllocCount > pVBF->uBlocksPerUnit - nFirstAlloc)
    {
        ioTotal.ffxStat = FFXSTAT_OUTOFRANGE;
        goto ExpendAllocationsCleanup;
    }

    /*  Allocations are expended by zeroing them.  No matter what this
        does to the ECC for an allocation entry, it won't produce a
        valid check byte, and so will always be reported as ALLOC_BAD.
    */
    DclMemSet(buffer.tags, 0, DCLMIN(sizeof buffer.tags,
                                nAllocCount * sizeof buffer.tags[0]));

    /*  Process allocations in chunks that fit in the local buffer.
    */
    while(nAllocCount)
    {
        unsigned    nCount = DCLMIN(nAllocCount, DCLDIMENSIONOF(buffer.tags));

        /*  Assertion checks that cast is safe (given proper initialization
            of nCount above).
        */
        DclAssert(sizeof buffer.tags <= D_UINT16_MAX);

        FMLWRITE_TAGS(pVBF->hFML,
                        AllocEntryPage(pVBF, lnu, nFirstAlloc),
                        nCount, (const D_BUFFER *)buffer.tags,
                        FFX_NAND_TAGSIZE, ioStat);

        ioTotal.ulCount         += ioStat.ulCount;
        ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        DclAssert(ioStat.ulCount == nCount);

        nFirstAlloc += nCount;
        nAllocCount -= nCount;
    }

    ioTotal.ffxStat = ioStat.ffxStat;

  ExpendAllocationsCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEUNDENT),
         MMPREFIX":ExpendAllocations() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Protected: DiscardAllocations()

    The operation is not supported on the media

    Parameters:
        pVBF        - A pointer to the VBFDATA structure to use
        lnu         - linear unit index
        nStartAlloc - first allocation to mark discarded.
        nAllocCount - number of allocations to discard.

    Return Value:
        Always returns an FFXIOSTATUS type with a status value
        of FFXSTAT_UNSUPPORTEDFUNCTION.
-------------------------------------------------------------------*/
static FFXIOSTATUS DiscardAllocations(
    const VBFDATA          *pVBF,
    LinearUnit_t            lnu,
    unsigned                nStartAlloc,
    unsigned                nAllocCount)
{
    static const FFXIOSTATUS ioStat = DEFAULT_UNSUPPORTEDFUNCTION_PAGEIO_STATUS;

    DclAssert(pVBF);
    DclAssert(nAllocCount);

    (void) pVBF;
    (void) lnu;
    (void) nStartAlloc;
    (void) nAllocCount;

    return ioStat;
}


/*-------------------------------------------------------------------
    Protected: CopyBadBlock()

    Copies an allocation block that is presumed to have an
    uncorrectable error in the user data.  The block is read
    and written with FML "native pages" functions, which read with
    the normal error correction for the data disabled, and write
    bit-for-bit images of the data and ECC.

    The intention is to perform the copy in a way that preserves the
    allocation entry and the uncorrectable error indication, along
    with whatever data (probably corrupted) is available.

    Parameters:
        pVBF       - A pointer to the VBFDATA structure to use
        lnuFrom    - linear unit index of the unit containing the
                     allocation block with an uncorrectable error
        nBlockFrom - block (allocation index) in unit to copy from
        lnuTo      - linear unit index of the unit to copy to
        nBlockTo   - block (allocation index) in unit to copy to

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static FFXSTATUS CopyBadBlock(
    const VBFDATA      *pVBF,
    LinearUnit_t        lnuFrom,
    unsigned            nBlockFrom,
    LinearUnit_t        lnuTo,
    unsigned            nBlockTo)
{
    FFXIOSTATUS ioStat;
    MMDEVINFO *pMmInfo = pVBF->hMM->pMmInfo;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEINDENT),
                    MMPREFIX":CopyBadBlock() lnu %lU blk %u -> lnu %lU blk %u\n",
                    (D_UINT32) lnuFrom, nBlockFrom, (D_UINT32) lnuTo, nBlockTo));

    DclProfilerEnter(MMPREFIX":CopyBadBlock", 0, 1);

    if (!DclMutexAcquire(pMmInfo->pMutex))
    {
        DclError();
        return FFXSTAT_FAILURE;
    }

    FMLREAD_NATIVEPAGES(pVBF->hFML, FfxMMDataBlockNum(pVBF, lnuFrom, nBlockFrom),
                        1, pMmInfo->pMain, pMmInfo->pSpare, ioStat);

    /*  FMLREAD_NATIVEPAGES is supposed to correct any correctable
        errors.  FFXSTAT_FIMUNCORRECTED means this wasn't actually
        done, perhaps because the hardware isn't capable, perhaps
        because it's not implemented in software.

        Here, it's known to be a bad page, and whatever bits could be
        scraped will have to do.
    */
    if (ioStat.ffxStat == FFXSTAT_FIMUNCORRECTED)
    {
        FFXPRINTF(1, (MMPREFIX ":CopyBadBlock() warning: no correction on read\n"));
/*         FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACENODENT), */
/*                         MMPREFIX ":CopyBadBlock() warning: no correction on read\n")); */
        ioStat.ffxStat = FFXSTAT_SUCCESS;
    }
    if (ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        FMLWRITE_NATIVEPAGES(pVBF->hFML, FfxMMDataBlockNum(pVBF, lnuTo, nBlockTo),
                             1, pMmInfo->pMain, pMmInfo->pSpare, ioStat);
    }

    if (!DclMutexRelease(pMmInfo->pMutex))
    {
        DclError();
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEUNDENT),
                    MMPREFIX":CopyBadBlock() returning %lX\n", ioStat.ffxStat));

    return ioStat.ffxStat;
}


/*-------------------------------------------------------------------
    Protected: Destroy()

    Free any resources allocated for this media-specific media
    manager.

    Parameters:
        pVBF - A pointer to the VBFDATA structure using this instance.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void Destroy(
    MEDIAMGRINSTANCE *pMM)
{
    DclAssert(pMM && pMM->pMmInfo);

    DclMutexDestroy(pMM->pMmInfo->pMutex);
    DclMemFree(pMM->pMmInfo->pSpare);
    DclMemFree(pMM->pMmInfo->pMain);
    DclMemFree(pMM->pMmInfo);
}


/*-------------------------------------------------------------------
    Protected: BlankCheck()

    Checks the page for an allocation block and determines whether
    it is entirely blank (unprogrammed).

    Parameters:
        pVBF        - A pointer to the VBFDATA structure to use
        lnu         - linear unit index
        nBlockIndex - block (allocation index) in unit

    Return Value:
        Returns FFXSTAT_SUCCESS if the block is blank (unprogrammed),
        or FFXSTAT_MEDIAMGR_NOTBLANK if the block is not blank,
        otherwise a standard status code is returned for other types
        of errors.
 -------------------------------------------------------------------*/
static FFXSTATUS BlankCheck(
    const VBFDATA      *pVBF,
    LinearUnit_t        lnu,
    unsigned            nBlockIndex)
{
    FFXIOSTATUS         ioStat;
    MMDEVINFO          *pMmInfo = pVBF->hMM->pMmInfo;
    FFXFMLINFO          FmlInfo;

    ioStat.ffxStat = FfxFmlDiskInfo(pVBF->hFML, &FmlInfo);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        return ioStat.ffxStat;

    if (!DclMutexAcquire(pMmInfo->pMutex))
    {
        DclError();
        return FFXSTAT_FAILURE;
    }

    FMLREAD_UNCORRECTEDPAGES(pVBF->hFML, FfxMMDataBlockNum(pVBF, lnu, nBlockIndex), 1,
                  pMmInfo->pMain, NULL, ioStat);

    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
    {
        /*  Correctable errors aren't reported as such, they simply
            mean the block is not blank.  This is true even if the
            Error Manager recommendation is "no action".
        */
        if (ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA
            || ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEMAIN
            || ioStat.ffxStat == FFXSTAT_FIMCORRECTABLESPARE)
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 1, TRACENODENT),
                            MMPREFIX ":BlankCheck() correctable error unit %lu, block %u\n",
                            (unsigned long) lnu, nBlockIndex));

            ioStat.ffxStat = FFXSTAT_MEDIAMGR_NOTBLANK;
        }
    }
    else
    {
        if (pMmInfo->pMain[0] != ERASED_8
            || DclMemCmp(pMmInfo->pMain, pMmInfo->pMain + 1, FmlInfo.uPageSize - 1) != 0)
        {
            ioStat.ffxStat = FFXSTAT_MEDIAMGR_NOTBLANK;
        }
    }

    if (ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        FMLREAD_SPARES(pVBF->hFML, FfxMMDataBlockNum(pVBF, lnu, nBlockIndex), 1,
                       pMmInfo->pSpare, ioStat);
    }

    if (ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        if (pMmInfo->pSpare[0] != ERASED_8
            || DclMemCmp(pMmInfo->pSpare, pMmInfo->pSpare + 1, FmlInfo.uSpareSize - 1) != 0)
        {
            ioStat.ffxStat = FFXSTAT_MEDIAMGR_NOTBLANK;
        }
    }

    if (!DclMutexRelease(pMmInfo->pMutex))
    {
        DclError();
    }

    return ioStat.ffxStat;
}


/*-------------------------------------------------------------------
    Protected: Verify()

    Checks an allocation block and determines whether it is
    valid or has an uncorrectable ECC error.

    Parameters:
        pVBF        - A pointer to the VBFDATA structure to use
        lnu         - linear unit index
        nBlockIndex - block (allocation index) in unit

    Return Value:
        Returns FFXSTAT_SUCCESS if the page can be successfully
        read and contains no uncorrectable errors.  If the page
        has uncorrectable errors, FFXSTAT_FIMUNCORRECTABLEDATA
        will be returned.  Otherwise a standard status code will
        be returned in the event of some other error.
-------------------------------------------------------------------*/
static FFXSTATUS Verify(
    const VBFDATA      *pVBF,
    LinearUnit_t        lnu,
    unsigned            nBlockIndex)
{
    FFXIOSTATUS         ioStat;
    MMDEVINFO *pMmInfo = pVBF->hMM->pMmInfo;

    if (!DclMutexAcquire(pMmInfo->pMutex))
    {
        DclError();
        return FFXSTAT_FAILURE;
    }

    FMLREAD_TAGGEDPAGES(pVBF->hFML, FfxMMDataBlockNum(pVBF, lnu, nBlockIndex),
                        1, pMmInfo->pMain, pMmInfo->pSpare, FFX_NAND_TAGSIZE, ioStat);

    if (ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA
        || ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEMAIN
        || ioStat.ffxStat == FFXSTAT_FIMCORRECTABLESPARE)
    {
        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 1, TRACENODENT),
                        MMPREFIX ":Verify() correctable error unit %lu block %u\n",
                        (unsigned long) lnu, nBlockIndex));

        if (FfxErrMgrDecodeRecommendation(&ioStat) == FFXERRPOLICY_NO_ACTION)
            FfxErrMgrKillError(&ioStat);
        else
            ioStat.ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
    }

    if (!DclMutexRelease(pMmInfo->pMutex))
    {
        DclError();
    }

    return ioStat.ffxStat;
}


/*-------------------------------------------------------------------
    Protected: ReadHeader()

    Read an EUH from a linear page address, unpacking the fields into
    an EUH structure.  Check it for validity by checking the signature
    and validation code.  Note that there is no consistency or sanity
    checking of the fields, as only the allocator understands what
    they mean.

    Parameters:
        hMM     - The Media Manager instance
        ulStart - Starting page number of the unit
        pEUH    - Where to put the unpacked unit header

    Return Value:
        Returns FFXSTAT_SUCCESS if the header was read successfully.
        The destination EUH structure contents represent the header
        from the media.

        Returns FFXSTAT_MEDIAMGR_NOEUH or FFXSTAT_MEDIAMGR_BADEUH if
        no EUH was found on the media.  This is usually due to an
        interrupted format operation.

        Otherwise a standard status code is returned.

        If the return value is not FFXSTAT_SUCCESS, the destination
        structure contents are undefined (may have been modified).
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadHeader(
    MEDIAMGRINSTANCE *hMM,
    D_UINT32 ulStart,
    EUH *pEUH)
{
    FFXIOSTATUS ioStat = INITIAL_UNKNOWNFLAGS_STATUS(FFXSTAT_FAILURE);

    if (DclMutexAcquire(hMM->pMmInfo->pMutex))
    {
        ioStat = FfxMMNandReadHeader(hMM->hFML, ulStart, pEUH, hMM->pMmInfo->pMain);

        (void) DclMutexRelease(hMM->pMmInfo->pMutex);
    }
    else
    {
        DclError();
    }

    return ioStat;
}


/*-------------------------------------------------------------------
    Protected: FormatUnit()

    Given the address of a unit, erase it and write out an erase
    unit header. This function is shared between the client
    interface, garbage collection, and unit recovery code.

    Parameters:
        hMM              - the Media Manager instance
        ulUnitStart      - page number of the first erase block of the
                           unit.
        pEUH             - information to write into the new erase
                           unit header.
        fLargeAddressing - indicates whether the on-media format uses
                           values that are capable of supporting
                           large flash arrays (> 2GB).
        fFormatNoErase   - skip the erase step as part of format;
                           requires parts erased by the factory

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static FFXSTATUS FormatUnit(
    MEDIAMGRINSTANCE   *hMM,
    D_UINT32            ulUnitStart,
    const EUH          *pEUH,
    D_BOOL              fLargeAddressing,
    D_BOOL              fFormatNoErase)
{
    FFXSTATUS ffxStat = FFXSTAT_FAILURE;

    if (DclMutexAcquire(hMM->pMmInfo->pMutex))
    {
        ffxStat = FfxMMNandFormatEraseUnit(hMM->hFML, ulUnitStart, pEUH,
                                           fLargeAddressing, fFormatNoErase, hMM->pMmInfo->pMain);

        (void) DclMutexRelease(hMM->pMmInfo->pMutex);
    }
    else
    {
        DclError();
    }

    return ffxStat;
}



                /*---------------------------------*\
                 *                                 *
                 *   Internal Helper Functions     *
                 *                                 *
                \*---------------------------------*/



/*-------------------------------------------------------------------
    Local: AllocEntryPage()

    Obtain the page address of the allocation entry for a data
    block specified in terms of partition, linear unit index, and
    block (allocation) index.

    Parameters:
        pVBF   - A pointer to the VBFDATA structure to use
        lnu    - linear unit index of the unit containing the
                 block being addressed.
        nBlock - the block index within the unit.

    Return Value:
        A page suitable for use as an argument to FMLREAD_TAGS()
        or FMLWRITE_TAGS().
-------------------------------------------------------------------*/
static D_UINT32 AllocEntryPage(
    const VBFDATA          *pVBF,
    LinearUnit_t            lnu,
    unsigned                nBlock)
{
    D_UINT32                ulAllocationBlockIndex;

    /*  First compute the index of the first allocation block in the
        linear unit.
    */
    ulAllocationBlockIndex = lnu * pVBF->uTotalBlocksPerUnit;

    /*  The first block is the EUH and must not be considered!
        Account for the first block, and add the block index
        within the unit to the base.
    */
    ulAllocationBlockIndex += (nBlock + 1);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 3, 0),
        "AllocEntryPage() Lnu=%U Block=%u is %lX\n", lnu, nBlock, ulAllocationBlockIndex));

    return ulAllocationBlockIndex;
}



                /*---------------------------------*\
                 *                                 *
                 *  Direct Interface to MediaMgr   *
                 *                                 *
                \*---------------------------------*/



/*-------------------------------------------------------------------
    Protected: FfxMMNandReadHeader()

    Read an EUH, unpacking the fields into an EUH structure.
    Check it for validity.

    Parameters:
        hFML    - The FML handle
        ulStart - starting linear address of the unit
        pEUH    - where to put the unpacked unit header
        pPage   - a buffer with room for the main page (not spare)

    Return Value:
        Returns FFXSTAT_SUCCESS if the header was read successfully.
        The destination EUH structure contents represent the header
        from the media.

        Returns FFXSTAT_MEDIAMGR_NOEUH or FFXSTAT_MEDIAMGR_BADEUH if
        no EUH was found on the media.  This is usually due to an
        interrupted format operation.

        Otherwise a standard status code is returned.

        If the return value is not FFXSTAT_SUCCESS, the destination
        structure contents are undefined (may have been modified).
-------------------------------------------------------------------*/
FFXIOSTATUS FfxMMNandReadHeader(
    FFXFMLHANDLE        hFML,
    D_UINT32            ulPage,
    EUH                *pEUH,
    void               *pPage)
{
    FFXIOSTATUS         ioStat = INITIAL_UNKNOWNFLAGS_STATUS(FFXSTAT_OUTOFMEMORY);
    union
    {
        FFXNANDTAG      tag;
        DCL_ALIGNTYPE   DummyAlign;
    } buffer;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 3, TRACEINDENT),
         "FfxMMNandReadHeader() Page=%lX\n", ulPage));

    DclProfilerEnter("FfxMMNandReadHeader", 0, 0);

    /*  Read just the allocation entry first.
    */
    FMLREAD_TAGS(hFML, ulPage, 1, buffer.tag, FFX_NAND_TAGSIZE, ioStat);
    if (ioStat.ffxStat == FFXSTAT_SUCCESS
        || ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA
        || ioStat.ffxStat == FFXSTAT_FIMCORRECTABLESPARE)
    {
        /*  The EUH doesn't have a normal allocation entry, just a signature
        */
        if(DclMemCmp(buffer.tag, NANDEuhSig, sizeof buffer.tag) != 0)
        {
            ioStat.ffxStat = FFXSTAT_MEDIAMGR_NOEUH;
        }
        else
        {
            /*  If the signature is present, this is (probably) a formatted
                unit.  Read the EUH.
            */
            FMLREAD_PAGES(hFML, ulPage, 1, pPage, ioStat);
            if (ioStat.ffxStat == FFXSTAT_SUCCESS
                || ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA
                || ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEMAIN)
            {
                /*  Unpack the EUH and validate it.
                */
                if (!FfxMMParseEUH(pPage, pEUH))
                    ioStat.ffxStat = FFXSTAT_MEDIAMGR_BADEUH;
            }
            else if (ioStat.ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA)
            {
                /*  A page was read from the flash, but is corrupted.
                */
                ioStat.ffxStat = FFXSTAT_MEDIAMGR_BADEUH;
            }
        }
    }
    else
    {
        ioStat.ffxStat = FFXSTAT_FIMIOERROR;
    }

    /*  If a correctable error occurred, check the error policy
        recommendation.  Return FFXSTAT_FIMCORRECTABLEDATA if
        the error should be scrubbed, or FFXSTAT_SUCCESS if no
        action is required.
    */
    if (ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA
        || ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEMAIN
        || ioStat.ffxStat == FFXSTAT_FIMCORRECTABLESPARE)
    {
        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 1, TRACENODENT),
                        "FfxMMNandReadHeader() correctable error page %lu\n",
                        (unsigned long) ulPage));

        if (FfxErrMgrDecodeRecommendation(&ioStat) == FFXERRPOLICY_NO_ACTION)
            FfxErrMgrKillError(&ioStat);
        else
            ioStat.ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEUNDENT),
         "FfxMMNandReadHeader() for page %lX returning %lX\n", ulPage, ioStat.ffxStat));

    return ioStat;
}


/*-------------------------------------------------------------------
    Protected: FfxMMNandFormatEraseUnit()

    Format a NAND erase unit by erasing it and writing a new EUH.

    Note that there is a slight but troubling possibility that
    the erase could be interrupted after it had changed some
    data bits or allocations, but before it had made the EUH
    invalid.  This could lead to spurious BBM remapping,
    erroneous data, or replacement of valid allocations in other
    units.  To make this much less likely, the Media Manager
    signature for the EUH (stored in the spare area where the
    allocation entry would be for a client data block) is
    invalidated before erasing the unit.

    Parameters:
        hFML             - The FML handle
        ulUnitStart      - The starting block number of the unit
        pEUH             - information to write into the new erase
                           unit header.
        fLargeAddressing - indicates whether the on-media format
                           uses values that are capable of supporting
                           large flash arrays (> 2GB).
        fFormatNoErase   - skip the erase step as part of format;
                           requires parts erased by the factory
        pPage            - a buffer with room for the main page
                           (not spare)

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxMMNandFormatEraseUnit(
    FFXFMLHANDLE        hFML,
    D_UINT32            ulUnitStart,
    const EUH          *pEUH,
    D_BOOL              fLargeAddressing,
    D_BOOL              fFormatNoErase,
    void               *pPage)
{
    DCLALIGNEDSTRUCT    (tag, FFXNANDTAG, data);
    FFXIOSTATUS         ioStat;
    FFXFMLINFO          FmlInfo;
    FFXFMLDEVINFO       FmlDevInfo;
    D_UINT32            ulBlocksPerUnit;
    D_UINT32            ulPagesPerBlock;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEINDENT),
         "FfxMMNandFormatEraseUnit() Unit=%lX\n", ulUnitStart));

    DclProfilerEnter("FfxMMNandFormatEraseUnit", 0, 0UL);

    DclAssert(hFML);
    DclAssert(pEUH);

    ioStat.ffxStat = FfxFmlDiskInfo(hFML, &FmlInfo);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        goto FormatCleanup;

    ioStat.ffxStat = FfxFmlDeviceInfo(FmlInfo.nDeviceNum, &FmlDevInfo);
    if (ioStat.ffxStat != FFXSTAT_SUCCESS)
        goto FormatCleanup;

    DclAssert((FmlInfo.ulBlockSize % FmlInfo.uPageSize) == 0);
    DclAssert(pEUH->uAllocBlockSize == FmlInfo.uPageSize);

    ulBlocksPerUnit = ((D_UINT32)pEUH->uUnitTotalBlocks * pEUH->uAllocBlockSize) / FmlInfo.ulBlockSize;
    ulPagesPerBlock = FmlInfo.ulBlockSize / FmlInfo.uPageSize;

    DclAssert(pEUH->uAllocBlockSize == FmlInfo.uPageSize);

    FFXMM_ISSUE_WRITE_INTERRUPTION(hFML, COMPACT_INT_MEDIAMGR1);

    /*  MLC NAND flash does not allow more than one program operation
        per page before erasing; on some parts it may actually be
        physically impossible.  On such parts, stronger error detection
        is used to protect the allocation entries, so an interrupted
        erase can't change them undetected.
    */
    if (!(FmlDevInfo.uDeviceFlags & DEV_PGM_ONCE))
    {
        /*  Invalidate the EUH on the media by zeroing the first byte of the
            signature in what would be the allocation entry area in the
            spare area.  This violates the "sequential write, zero rewrite"
            policy for NAND flash, but it's allowable here because these
            restrictions exist to prevent data corruption due to the
            "write disturb" effect.  Since the unit will never be read
            again, who cares?

            Note that care is taken to write zero only to bits that are
            still one in the first byte of the signature.
        */
        FMLREAD_TAGS(hFML, ulUnitStart, 1, tag.data, FFX_NAND_TAGSIZE, ioStat);

        switch (ioStat.ffxStat)
        {
            case FFXSTAT_FIMCORRECTABLEDATA:
            case FFXSTAT_FIMCORRECTABLESPARE:
                /*  A correctable error in the tag means it still
                    needs to be invalidated.
                */
                ioStat.ffxStat = FFXSTAT_SUCCESS;
                /* fall through */

            case FFXSTAT_SUCCESS:
                DclAssert(ioStat.ulCount == 1);

                if((DclMemCmp(tag.data, NANDEuhSig, sizeof tag.data)) == 0)
                {
                    DclMemSet(tag.data, ERASED_8, sizeof tag.data);
                    DclAssert((D_UCHAR) ~ NANDEuhSig[0] != (D_UCHAR) ERASED_8);
                    tag.data[0] = (D_UCHAR) ~ NANDEuhSig[0];

                    FMLWRITE_TAGS(hFML, ulUnitStart, 1, tag.data, FFX_NAND_TAGSIZE, ioStat);
                    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                        goto FormatCleanup;

                    DclAssert(ioStat.ulCount == 1);
                }
                break;

            case FFXSTAT_FIMUNCORRECTABLEDATA:
            case FFXSTAT_FIMUNCORRECTABLESPARE:
                /*  The tag is already invalid.
                */
                ioStat.ffxStat = FFXSTAT_SUCCESS;
                break;

            default:
                goto FormatCleanup;
        }
    }

    FFXMM_ISSUE_WRITE_INTERRUPTION(hFML, COMPACT_INT_MEDIAMGR2);

    if(!fFormatNoErase)
    {
        /*  Erase the unit.
        */
        FMLERASE_BLOCKS(hFML, ulUnitStart / ulPagesPerBlock, ulBlocksPerUnit, ioStat);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            goto FormatCleanup;

        DclAssert(ioStat.ulCount == ulBlocksPerUnit);

        FFXMM_ISSUE_WRITE_INTERRUPTION(hFML, COMPACT_INT_MEDIAMGR3);
    }
    /*  Initialize the buffer to the erased state and build the new EUH
        in the buffer.
    */
    DclMemSet(pPage, ERASED_8, FmlInfo.uPageSize);
    FfxMMBuildEUH(pPage, pEUH, fLargeAddressing);

    /*  The "allocation entry" for the EUH isn't really an allocation
        entry.  Store a signature value instead of client address.
    */
    DclMemCpy(tag.data, NANDEuhSig, sizeof tag.data);

    /*  Write the new EUH to the media, including the signature in the
        spare area.
    */
    FMLWRITE_TAGGEDPAGES(hFML, ulUnitStart, 1, pPage, tag.data, FFX_NAND_TAGSIZE, ioStat);

  FormatCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEUNDENT),
         "FfxMMNandFormatEraseUnit() returning %lX\n", ioStat.ffxStat));

    return ioStat.ffxStat;
}


/*-------------------------------------------------------------------
    Protected: FfxMMNandCalcEUHDataUnitBlocks()

    Determine how many blocks of the unit are available for data
    storage after taking out the EUH and any space for the
    allocation list.

    Parameters:
        EUH information

    Return Value:
        Returns the number of unit blocks available.

        On NAND flash, # of unit blocks available is simply the
        number of blocks in the unit minus one for the EUH.
-------------------------------------------------------------------*/
unsigned FfxMMNandCalcEUHDataUnitBlocks(
    const EUH   *pEUH)
{
    DclAssert(pEUH);

    return (pEUH->uUnitTotalBlocks - 1);
}


/*-------------------------------------------------------------------
    Protected: FfxMMNandDeviceInit()

    Initialize the supplied MMDEVICE structure with the necessary 
    function pointer to enable this interface.

    Note that this function is located at the end of the module 
    to avoid having to create prototypes that would otherwise be 
    unnecessary.

    Parameters:
        pMM - A pointer to the MMDEVICE structure to initialize

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxMMNandDeviceInit(
    MEDIAMGRINSTANCE       *pMM)
{
    static const MMDEVICE   ops =
    {
        ReadBlocks,
        WriteBlocks,
        ReadAllocations,
        DiscardAllocations,
        ExpendAllocations,
        Destroy,
        ReadHeader,
        FormatUnit,
        CopyBadBlock,
        BlankCheck,
        Verify
    };
    FFXFMLINFO              diskInfo;
    char                    szMutexName[sizeof "MMnnnnn"];  /*  Disk num is 16 bits, 5 decimal digits */
    MMDEVINFO              *pMmInfo;
    FFXSTATUS               ffxStat;

    DclAssert(pMM);

    ffxStat = FfxFmlDiskInfo(pMM->hFML, &diskInfo);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    if(DclSNPrintf(szMutexName, sizeof(szMutexName), "MM%u", diskInfo.nDiskNum) == -1)
    {
        DclError();
        return FFXSTAT_FAILURE;
    }

    pMmInfo = DclMemAlloc(sizeof(*pMmInfo));
    if (pMmInfo)
    {
        pMmInfo->pMain = DclMemAlloc(pMM->uPageSize);
        if (pMmInfo->pMain)
        {
            pMmInfo->pSpare = DclMemAlloc(pMM->uSpareSize);
            if (pMmInfo->pSpare)
            {
                pMmInfo->pMutex = DclMutexCreate(szMutexName);
                if (pMmInfo->pMutex)
                {
                    pMM->pMmOps = &ops;
                    pMM->pMmInfo = pMmInfo;
                    return FFXSTAT_SUCCESS;
                }
                DclMemFree(pMmInfo->pSpare);
            }
            DclMemFree(pMmInfo->pMain);
        }
        DclMemFree(pMmInfo);
    }
    
    return FFXSTAT_FAILURE;
}


#endif  /* FFXCONF_NANDSUPPORT */
#endif  /* FFXCONF_VBFSUPPORT */


