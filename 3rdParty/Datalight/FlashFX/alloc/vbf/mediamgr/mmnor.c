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

    This module contains the low level Media Manager code to support NOR
    flash.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: mmnor.c $
    Revision 1.26  2009/08/04 01:05:27Z  garyp
    Merged from the v4.0 branch.  Minor datatype changes.  Documentation
    updated -- no functional changes.
    Revision 1.25  2009/03/31 16:37:29Z  davidh
    Function headers updated for AutoDoc
    Revision 1.24  2009/01/27 03:14:35Z  billr
    Resolve bug 1601: VBF write protects the disk after an uncorrectable ECC
    error.  Implement new Media Manager function FfxMediaMgrCopyBadBlock(). 
    Allow per-instance data for media-specific functions.
    Revision 1.23  2009/01/12 17:59:40Z  billr
    Fix compiler warning.
    Revision 1.22  2009/01/08 18:49:58Z  jimmb
    Supplied patch from BillR 2009-01-08
    Revision 1.21  2009/01/07 01:18:23Z  billr
    FfxMMNorReadHeader() now returns a full FFXIOSTATUS.
    Revision 1.20  2008/06/03 19:43:57Z  thomd
    Added fFormatNoErase
    Revision 1.19  2008/03/17 18:14:50Z  Garyp
    Made allocator/VBF code conditional on FFXCONF_ALLOCATORSUPPORT and
    FFXCONF_VBFSUPPORT, respectively.
    Revision 1.18  2008/01/30 22:33:51Z  Garyp
    Updated to use VBF_EUH_BUFFER_SIZE rather than the misused BLOCK_SIZE_MIN.
    Revision 1.17  2008/01/13 07:20:41Z  keithg
    Function header updates to support autodoc.
    Revision 1.16  2007/11/05 19:14:23Z  Garyp
    Corrected the debug trace messages -- no functional changes.
    Revision 1.15  2007/11/02 20:13:18Z  billr
    Fix compiler warnings.
    Revision 1.14  2007/11/02 02:26:05Z  Garyp
    Eliminated the inclusion of limits.h.
    Revision 1.13  2007/11/01 01:00:05Z  Garyp
    Major update to eliminate the inconsistent use of D_UINT16 types.  All block
    indexes and counts which were D_UINT16 are now consistently unsigned.
    Replaced the use of Return_t and IOReturn_t with FFXSTATUS and FFXIOSTATUS.
    Improved error handling.  Moved NOR/ISWF shared settings into mmint.h.
    Revision 1.12  2007/04/19 17:36:37Z  timothyj
    Added fLargeAddressing parameter, to indicate whether the on-media format
    should use large flash array compatible addressing (Always false for NOR).
    Revision 1.11  2007/04/11 23:34:54Z  timothyj
    Moved functions that became NOR-specific (obsolete for NAND) after large
    flash array changes into this module (UnitLinearAddress and RawUnitSize).
    Revision 1.10  2007/02/28 00:46:27Z  Garyp
    Documentation updated.
    Revision 1.9  2006/10/06 01:02:05Z  Garyp
    Modified to no longer use the "old" FML interface.  Page read/write logic
    is simplified since there is no more max byte count stored in a D_UINT16.
    Revision 1.8  2006/05/08 08:52:15Z  Garyp
    Updated to use the new write interruption test interfaces.
    Revision 1.7  2006/03/05 01:57:19Z  Garyp
    Modified the usage of FfxFmlDiskInfo() and FfxFmlDeviceInfo().
    Revision 1.6  2006/02/26 02:16:38Z  Garyp
    Split out the NOR and NAND tag handling logic.
    Revision 1.5  2006/02/21 02:53:09Z  Garyp
    Changed EUH fields back to using block terminology, rather than pages, to
    be consistent with other internal VBF fields (from a client perspective,
    VBF operates on pages).
    Revision 1.4  2006/02/07 02:02:38Z  Garyp
    Modified to use the updated FML interface.
    Revision 1.3  2006/01/10 15:57:35Z  billr
    More Sibley merge details.
    Revision 1.2  2006/01/04 20:54:51Z  billr
    Merge Sibley support from v2.01.
    Revision 1.1  2005/12/08 03:08:52Z  Garyp
    Initial revision
    Revision 1.2  2005/10/26 03:16:29Z  Garyp
    Replaced STD_NAND_PAGE_SIZE with BLOCK_SIZE_MIN.
    Revision 1.1  2005/10/02 03:03:40Z  Garyp
    Initial revision
    Revision 1.4  2005/09/14 10:23:22Z  Rickc
    changed dbgprintf levels
    Revision 1.3  2005/09/01 17:18:34  billr
    Replace TABs with spaces.
    Revision 1.2  2005/08/31 22:40:30Z  Cheryl
    use achSig instead of VBF_SIGNATURE
    Revision 1.1  2005/08/27 20:08:44Z  Cheryl
    Initial revision
    ---------------------
    Bill Roman 2003-03-14
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_VBFSUPPORT
#if FFXCONF_NORSUPPORT

#include <vbf.h>
#include <fxfmlapi.h>
#include <vbfint.h>
#include <mediamgr.h>
#include <tstwint.h>

#include "mmint.h"
#include "mmnor.h"

#define MMPREFIX "FfxMMNor"


/*-------------------------------------------------------------------
    Internal function prototypes
-------------------------------------------------------------------*/
static D_UINT32  AllocEntryAddress(const VBFDATA *pVBF, LinearUnit_t lnu, unsigned nBlock);
static D_BOOL    IsValidNorAlloc(const FFXNORTAG alloc);


                /*---------------------------------*\
                 *                                 *
                 * Indirect Interface to MediaMgr  *
                 *                                 *
                \*---------------------------------*/



/*-------------------------------------------------------------------
    Protected: ReadBlocks()

    Read data from an allocation unit in NOR flash.  (Allocation
    units must be read in control mode)

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

    FMLREAD_PAGES(pVBF->hFML,
                    FfxMMDataBlockNum(pVBF, lnu, nBlockIndex),
                    nBlockCount, pClientData, ioStat);

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEUNDENT),
        MMPREFIX":ReadBlocks() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Protected: WriteBlocks()

    Writes data to a unit in NOR flash. (Allocation units must
    be written in contol mode, data units to be written in object
    mode)

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
    const VBFDATA      *pVBF,
    LinearUnit_t        lnu,
    unsigned            nBlockIndex,
    unsigned            nBlockCount,
    unsigned            nBlockKey,
    const D_BUFFER     *pClientData)
{
    FFXIOSTATUS         ioStat = DEFAULT_PAGEIO_STATUS;
    FFXIOSTATUS         ioTotal = DEFAULT_PAGEIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEINDENT),
        MMPREFIX":WriteBlocks() LNU=%U BlkIndex=%u BlkCount=%u BlkKey=%u\n",
        lnu, nBlockIndex, nBlockCount, nBlockKey));

    DclProfilerEnter(MMPREFIX":WriteBlocks", 0, nBlockCount);

    DclAssert(pVBF);
    DclAssert(nBlockCount);
    DclAssert(pClientData);

    while(nBlockCount)
    {
        union
        {
            FFXNORTAG       alloc[NOR_TAG_BUFFER_DIM];
            DCL_ALIGNTYPE   DummyAlign;
        } buffer;
        unsigned            ii;
        unsigned            nCount = DCLMIN(nBlockCount, DCLDIMENSIONOF(buffer.alloc));
        D_UINT32            ulLen = nCount * sizeof buffer.alloc[0];

        /*  Assertion ensures that the length is appropriate for the current
            low level code which expects a max value of D_UINT16_MAX.
        */
        DclAssert(ulLen <= D_UINT16_MAX);

        /*  Build the initial state of allocation entries.
        */
        DclMemSet(buffer.alloc, ERASED_8, sizeof buffer.alloc);
        for(ii = 0; ii < nCount; ++ii)
        {
            D_UINT16 alloc = MAKE_ALLOC(MEDIA_ALLOC_TYPE_VALID, nBlockKey + ii);

            buffer.alloc[ii][0] = (D_UCHAR) alloc;
            buffer.alloc[ii][1] = (D_UCHAR) (alloc >> CHAR_BIT);
        }

        /*  Write the allocation entries.
        */
        FMLWRITE_CONTROLDATA(pVBF->hFML,
                            AllocEntryAddress(pVBF, lnu, nBlockIndex),
                            ulLen, (D_BUFFER*)buffer.alloc, ioStat);

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        DclAssert(ioStat.ulCount == ulLen);

        FFX_ISSUE_WRITE_INTERRUPTION(pVBF->nDiskNum, WRITE_INT_MEDIAMGR1);

        /*  Write the client data
        */
        FMLWRITE_PAGES(pVBF->hFML,
                        FfxMMDataBlockNum(pVBF, lnu, nBlockIndex),
                        nCount, pClientData, ioStat);

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        DclAssert(ioStat.ulCount == nCount);

        FFX_ISSUE_WRITE_INTERRUPTION(pVBF->nDiskNum, WRITE_INT_MEDIAMGR2);

        /*  Build the completed state of allocation entries.
        */
        if(!pVBF->hMM->mergeWrites)
            DclMemSet(buffer.alloc, ERASED_8, sizeof buffer.alloc);

        for(ii = 0; ii < nCount; ++ii)
        {
            D_UINT16 alloc = MAKE_ALLOC(MEDIA_ALLOC_TYPE_VALID, nBlockKey + ii);

            buffer.alloc[ii][2] = (D_UCHAR) (alloc ^ NOR_HASH);
            buffer.alloc[ii][3] = (D_UCHAR) ((alloc ^ NOR_HASH) >> CHAR_BIT);
        }

        /*  Write the allocation entries.
        */
        FMLWRITE_CONTROLDATA(pVBF->hFML,
                            AllocEntryAddress(pVBF, lnu, nBlockIndex),
                            ulLen, (D_BUFFER*)buffer.alloc, ioStat);

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        DclAssert(ioStat.ulCount == ulLen);

        FFX_ISSUE_WRITE_INTERRUPTION(pVBF->nDiskNum, WRITE_INT_MEDIAMGR3);

        ioTotal.ulCount += nCount;
        pClientData     += nCount * pVBF->uAllocationBlockSize;
        nBlockIndex     += nCount;
        nBlockKey       += nCount;
        nBlockCount     -= nCount;
    }

    ioTotal.ffxStat = ioStat.ffxStat;
    ioTotal.op.ulPageStatus |= ioTotal.op.ulPageStatus;

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEUNDENT),
        MMPREFIX":WriteBlocks() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Protected: ReadAllocations()

    Read the some or all of the allocation list from a unit on
    NOR flash.  Allocation entries are read, validated, and
    converted to Allocation structures in rgaUnitAllocList.
    Processing stops when the end of the allocation list on the
    media is reached or when nAllocCount allocations have been
    processed, whichever comes first.  (Allocation units must be
    read in control mode.)

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
    const VBFDATA      *pVBF,
    LinearUnit_t        lnu,
    unsigned            nStartAlloc,
    unsigned            nAllocCount,
    Allocation          rgaUnitAllocList[])
{
    FFXIOSTATUS         ioStat = DEFAULT_PAGEIO_STATUS;
    FFXIOSTATUS         ioTotal = DEFAULT_PAGEIO_STATUS;

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
            FFXNORTAG       alloc[NOR_TAG_BUFFER_DIM];
            DCL_ALIGNTYPE   DummyAlign;
        } buffer;
        unsigned            ii;
        unsigned            nCount = DCLMIN(nAllocCount, DCLDIMENSIONOF(buffer.alloc));
        D_UINT32            ulLen = nCount * sizeof buffer.alloc[0];

        /*  Assertion ensures that the length is appropriate for the current
            low level code which expects a max value of D_UINT16_MAX.
        */
        DclAssert(ulLen <= D_UINT16_MAX);

        FFXPRINTF(3, ("ReadAllocations - %lU \n", ulLen));

        FMLREAD_CONTROLDATA(pVBF->hFML, AllocEntryAddress(pVBF, lnu, nStartAlloc),
                            ulLen, (D_BUFFER*)buffer.alloc, ioStat);

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        DclAssert(ioStat.ulCount == ulLen);

        for(ii = 0; ii < nCount; ++ii)
        {
            rgaUnitAllocList[ii].uBlkInUnit = nStartAlloc + ii;

            /*  If the allocation entry is in its unprogrammed state, this
                allocation is free.
            */
            if(FfxMMIsErasedAlloc(buffer.alloc[ii], FFX_NOR_TAGSIZE))
            {
                rgaUnitAllocList[ii].nState = ALLOC_FREE;
            }
            else if(IsValidNorAlloc(buffer.alloc[ii]))
            {
                D_UINT16 key = (D_UINT16)((buffer.alloc[ii][1] << CHAR_BIT) | buffer.alloc[ii][0]);

                switch (MEDIA_ALLOC_TYPE(key))
                {
                    case MEDIA_ALLOC_TYPE_DISCARDED:
                        rgaUnitAllocList[ii].nState = ALLOC_DISCARDED;
                        rgaUnitAllocList[ii].uBlkInRegion = key & MEDIA_ALLOC_MASK;
                        break;

                    case MEDIA_ALLOC_TYPE_VALID:
                        rgaUnitAllocList[ii].nState = ALLOC_VALID;
                        rgaUnitAllocList[ii].uBlkInRegion = key & MEDIA_ALLOC_MASK;
                        break;

                    default:
                        /*  write interruption?
                        */
                        rgaUnitAllocList[ii].nState = ALLOC_BAD;
                        break;
                }
            }
            else
            {
                /*  write interruption?
                */
                rgaUnitAllocList[ii].nState = ALLOC_BAD;
            }
        }

        ioTotal.ulCount     += nCount;
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
    Protected: DiscardAllocations()

    Mark allocations discarded on NOR flash.  This is done by
    clearing a bit in each half of the allocation entry to change
    the state from "valid" to "discarded".  It is done in two
    writes (to modify the halves separately) to accommodate MLC
    flash.

    It is explicitly permitted to call this function for a free
    allocation (one that has no bits programmed).  Doing so
    produces allocations that are reported with a state of
    ALLOC_BAD.

    Note: it is an error to call this function with arguments
    designating an allocation entry that is currently anything
    other than either a valid allocation or a free allocation.
    Doing so may violate the restriction on some NOR flash parts
    that a given bit may only be programmed once.

    Allocation units must be read in control mode.

    Parameters:
        pVBF        - A pointer to the VBFDATA structure to use
        lnu         - linear unit index
        u1stAlloc   - first allocation to mark discarded.
        nAllocCount - number of allocations to discard.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the results of
        the request.
-------------------------------------------------------------------*/
static FFXIOSTATUS DiscardAllocations(
    const VBFDATA      *pVBF,
    LinearUnit_t        lnu,
    unsigned            nStartAlloc,
    unsigned            nAllocCount)
{
    FFXIOSTATUS         ioStat = DEFAULT_PAGEIO_STATUS;
    FFXIOSTATUS         ioTotal = DEFAULT_PAGEIO_STATUS;
    D_UINT32            ulAddress;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEINDENT),
        MMPREFIX":DiscardAllocations() LNU=%U Start=%u Count=%u\n", lnu, nStartAlloc, nAllocCount));

    DclProfilerEnter(MMPREFIX":DiscardAllocations", 0, nAllocCount);

    DclAssert(pVBF);
    DclAssert(nAllocCount);

    ulAddress = AllocEntryAddress(pVBF, lnu, nStartAlloc);

    /*  Overwrite the allocation entries a buffer's worth at a time.
    */
    while(nAllocCount)
    {
        union
        {
            FFXNORTAG       alloc[NOR_TAG_BUFFER_DIM];
            DCL_ALIGNTYPE   DummyAlign;
        } buffer;
        unsigned            ii;
        unsigned            nCount = DCLMIN(nAllocCount, DCLDIMENSIONOF(buffer.alloc));
        D_UINT32            ulLen = nCount * sizeof buffer.alloc[0];

        /*  Assertion ensures that the length is appropriate for the current
            low level code which expects a max value of D_UINT16_MAX.
        */
        DclAssert(ulLen <= D_UINT16_MAX);

        if(pVBF->hMM->mergeWrites)
        {
            /*  Initialize the buffer by reading the current data from the
                flash.
            */
            FFXPRINTF(3, ("DiscardAllocations - %lU\n", ulLen ));

            FMLREAD_CONTROLDATA(pVBF->hFML, ulAddress, ulLen, (D_BUFFER*)buffer.alloc, ioStat);

            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            DclAssert(ioStat.ulCount == ulLen);
        }
        else
        {
            /*  Initialize the buffer to all ones
            */
            DclMemSet(buffer.alloc, ERASED_8, ulLen);
        }

        if(pVBF->hMM->multiLevelCell)
        {
            /*  To avoid the possibility of undetectably corrupting an
                allocation entry, update it in two separate operations,
                one on each half.

                First modify the buffer contents to clear the "valid" bit
                in the first half of each allocation entry on the media
                and leave the rest of the entry unchanged.
            */
            for(ii = 0; ii < nCount; ++ii)
                buffer.alloc[ii][1] &= NOR_ALLOC_DISCARD_MASK;

            /*  Write it out.
            */
            FMLWRITE_CONTROLDATA(pVBF->hFML, ulAddress, ulLen, (D_BUFFER*)buffer.alloc, ioStat);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            DclAssert(ioStat.ulCount == ulLen);

            /*  NOTE: This code must be updated to support flash parts
                that prohibit programming a bit to zero more than once.
            */
            DclAssert(!pVBF->hMM->programOnce);

            /*  Modify the buffer contents to clear the "valid" bit in the
                second half of each allocation entry.  The write happens
                below.
            */
            for(ii = 0; ii < nCount; ++ii)
                buffer.alloc[ii][3] &= NOR_ALLOC_DISCARD_MASK;
        }
        else
        {
            /*  Not MLC flash, so the discard can be done all in one write.
                Modify the buffer contents to clear the "valid" bits in
                both halves of the allocation entry.
            */
            for(ii = 0; ii < nCount; ++ii)
            {
                buffer.alloc[ii][1] &= NOR_ALLOC_DISCARD_MASK;
                buffer.alloc[ii][3] &= NOR_ALLOC_DISCARD_MASK;
            }
        }

        /*  There may or may not actually have been a write above, but
            the interruption count must be satisfied or the test will
            never terminate.
        */
        FFX_ISSUE_WRITE_INTERRUPTION(pVBF->nDiskNum, WRITE_INT_MEDIAMGR4);

        /*  This is either the second write (MLC flash) or only write (non-MLC).
        */
        FMLWRITE_CONTROLDATA(pVBF->hFML, ulAddress, ulLen, (D_BUFFER*)buffer.alloc, ioStat);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        DclAssert(ioStat.ulCount == ulLen);

        FFX_ISSUE_WRITE_INTERRUPTION(pVBF->nDiskNum, WRITE_INT_MEDIAMGR5);

        ioTotal.ulCount += nCount;
        ulAddress       += ulLen;
        nAllocCount     -= nCount;
    }

    ioTotal.ffxStat = ioStat.ffxStat;

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_MEDIAMGR, 2, TRACEUNDENT),
        MMPREFIX":DiscardAllocations() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}



                /*---------------------------------*\
                 *                                 *
                 *   Internal Helper Functions     *
                 *                                 *
                \*---------------------------------*/



/*-------------------------------------------------------------------
    Local: AllocEntryAddress()

    Obtain the linear address of the allocation entry for a data
    block specified in terms of partition, linear unit index, and
    block (allocation) index.

    Parameters:
        pVBF   - A pointer to the VBFDATA structure to use
        lnu    - linear unit index of the unit containing the
                 block being addressed.
        nBlock - the block index within the unit.

    Return Value:
        An address suitable for use as an argument to
        FMLREAD/WRITE_CONTROLDATA()
-------------------------------------------------------------------*/
static D_UINT32 AllocEntryAddress(
    const VBFDATA      *pVBF,
    LinearUnit_t        lnu,
    unsigned            nBlock)
{
    D_UINT32            ulRawUnitSize;

    ulRawUnitSize = (D_UINT32)pVBF->uTotalBlocksPerUnit * pVBF->uAllocationBlockSize;

    return (lnu * ulRawUnitSize) + pVBF->uAllocationBlockSize + (nBlock * FFX_NOR_TAGSIZE);
}


/*-------------------------------------------------------------------
    Local: IsValidNorAlloc()

    Tests an image of an allocation entry read from the media to
    see whether it is a valid NOR allocation entry.

    Parameters:
        alloc - an allocation entry in the on-media format

    Return Value:
        Returns TRUE if the allocation entry is valid, FALSE
        otherwise.
-------------------------------------------------------------------*/
static D_BOOL IsValidNorAlloc(
    const FFXNORTAG alloc)
{
    return !((alloc[0] | (alloc[1] << CHAR_BIT))
             ^ (alloc[2] | (alloc[3] << CHAR_BIT)) ^ NOR_HASH);
}



                /*---------------------------------*\
                 *                                 *
                 *  Direct Interface to MediaMgr   *
                 *                                 *
                \*---------------------------------*/



/*-------------------------------------------------------------------
    Protected: FfxMMNorReadHeader()

    Read an EUH, unpacking the fields into an EUH structure.
    Check it for validity.

    EUH must be read in control mode.

    Parameters:
        hFML    - The FML handle
        ulStart - starting linear address of the unit
        pEUH    - where to put the unpacked unit header

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
FFXIOSTATUS FfxMMNorReadHeader(
    FFXFMLHANDLE        hFML,
    D_UINT32            ulPage,
    EUH                *pEUH)
{
    DCLALIGNEDBUFFER    (buffer, data, VBF_EUH_BUFFER_SIZE);
    FFXFMLINFO          FmlInfo;
    FFXIOSTATUS         ioStat = { 0 };

    DclAssert(hFML);
    DclAssert(pEUH);

    ioStat.ffxStat = FfxFmlDiskInfo(hFML, &FmlInfo);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        return ioStat;

    /*  Read the first allocation block of the unit into a buffer.  Note
        that the minimum block size is (supposed to be) selected to be
        large enough to hold the on-media representation of an EUH.
    */
    DclAssert(sizeof buffer.data >= VBF_EUH_BUFFER_SIZE);

    FMLREAD_CONTROLDATA(hFML, ulPage*FmlInfo.uPageSize, VBF_EUH_BUFFER_SIZE, buffer.data, ioStat);
    if(IOSUCCESS(ioStat, VBF_EUH_BUFFER_SIZE))
    {
        /*  Unpack the EUH and validate it.
        */
        if (!FfxMMParseEUH(buffer.data, pEUH))
            ioStat.ffxStat = FFXSTAT_MEDIAMGR_NOEUH;
    }

    return ioStat;
}


/*-------------------------------------------------------------------
    Protected: FfxMMNorFormatEraseUnit()

    Format a NOR erase unit by erasing it and writing a new EUH.

    Note that there is a slight but troubling possibility that
    the erase could be interrupted after it had changed some data
    bits or allocations, but before it had made the EUH invalid.
    This could lead to erroneous data, or replacement of valid
    allocations in other units.  To make this much less likely,
    the EUH signature is overwritten before erasing the unit.

    EUH must be read/written to in control mode.

    Parameters:
        hFML           - The FML handle
        ulUnitStart    - The starting unit number
        pEUH           - information to write into the new erase unit
                         header.
        fMergeWrites   - TRUE if "mergewrites" functionality is
                         required.
        fFormatNoErase - skip the erase step as part of format;
                         requires parts erased by the factory

    Return Value:
        Returns an FFXSTATUS value indicating the result of the
        operation.
-------------------------------------------------------------------*/
FFXSTATUS FfxMMNorFormatEraseUnit(
    FFXFMLHANDLE        hFML,
    D_UINT32            ulUnitStart,
    const EUH          *pEUH,
    D_BOOL              fMergeWrites,
    D_BOOL              fFormatNoErase)
{
    FFXFMLINFO          FmlInfo;
    DCLALIGNEDBUFFER    (buffer, data, VBF_EUH_BUFFER_SIZE);
    FFXIOSTATUS         ioStat;
    D_UINT32            ulBlocksPerUnit;

    ioStat.ffxStat = FfxFmlDiskInfo(hFML, &FmlInfo);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        return ioStat.ffxStat;

    ulBlocksPerUnit = ((D_UINT32)pEUH->uUnitTotalBlocks * pEUH->uAllocBlockSize) / FmlInfo.ulBlockSize;

    FFXMM_ISSUE_WRITE_INTERRUPTION(hFML, COMPACT_INT_MEDIAMGR1);

    /*  Invalidate the EUH on the media by destroying a byte of the EUH
        signature.  Start by reading part of the signature containing
        that byte and checking it.  If the unit is already invalid,
        there's no need to destroy it.
    */
    FMLREAD_CONTROLDATA(hFML, ulUnitStart, sizeof(D_UINT32), buffer.data, ioStat);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        return ioStat.ffxStat;

    DclAssert(ioStat.ulCount == sizeof(D_UINT32));

    if(DclMemCmp(buffer.data, achSig, sizeof(D_UINT32)) == 0)
    {
        /*  Write an entire D_UINT32 (four bytes) because FMLWRITE_CONTROLDATA()
            requires it.  Only change bits in one byte because that saves
            time on flash with only a single-byte write operation.
        */
        if(fMergeWrites)
        {
            /*  Must write zero to all bits already programmed to zero.
            */
            buffer.data[0] = (D_UCHAR) 0;
        }
        else
        {
            /*  Avoid reprogramming bits that are already programmed.
            */
            DclMemSet(buffer.data, ERASED_8, sizeof(D_UINT32));
            DclAssert((D_UCHAR) ~ achSig[0] != (D_UCHAR) ERASED_8);
            buffer.data[0] = (D_UCHAR) ~ achSig[0];
        }

        FMLWRITE_CONTROLDATA(hFML, ulUnitStart, sizeof(D_UINT32),  buffer.data, ioStat);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
          return ioStat.ffxStat;

        DclAssert(ioStat.ulCount == sizeof(D_UINT32));
    }

    FFXMM_ISSUE_WRITE_INTERRUPTION(hFML, COMPACT_INT_MEDIAMGR2);

    if(!fFormatNoErase)
    {
        /*  Erase the unit.
        */
        FMLERASE_BLOCKS(hFML, ulUnitStart / FmlInfo.ulBlockSize, ulBlocksPerUnit, ioStat);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
          return ioStat.ffxStat;

        DclAssert(ioStat.ulCount == ulBlocksPerUnit);

        FFXMM_ISSUE_WRITE_INTERRUPTION(hFML, COMPACT_INT_MEDIAMGR3);
    }

    /*  Initialize the buffer to the erased state and build the new EUH
        in the buffer.  Note that the minimum block size is (supposed to
        be) selected to be large enough to hold the on-media
        representation of an EUH.
    */
    DclAssert(sizeof buffer.data >= VBF_EUH_BUFFER_SIZE);
    DclMemSet(buffer.data, ERASED_8, VBF_EUH_BUFFER_SIZE);
    FfxMMBuildEUH(buffer.data, pEUH, FALSE);

    /*  Write the new EUH to the media.
    */
    FMLWRITE_CONTROLDATA(hFML, ulUnitStart, VBF_EUH_BUFFER_SIZE,  buffer.data, ioStat);

    return ioStat.ffxStat;
}


/*--------------------------------------------------------------------
    Protected: FfxMMNorCalcEUHDataUnitBlocks()

    Determine how many blocks of the unit are available for data
    storage after taking out the EUH and any space for the
    allocation list.

    Parameters:
        EUH information

    Return Value:
        Returns the number of unit blocks available.

        On NOR, space is also needed for an allocation entry
        corresponding to each data block.  After accounting for the
        EUH, divide the available space by the sum of the block size
        and the allocation entry size to find out how many can be
        accommodated.
-------------------------------------------------------------------*/
unsigned FfxMMNorCalcEUHDataUnitBlocks(
    const EUH * pEUH)
{
    DclAssert(pEUH);

    return((unsigned)(((D_UINT32)(pEUH->uUnitTotalBlocks - 1) * pEUH->uAllocBlockSize)
                    / (pEUH->uAllocBlockSize + FFX_NOR_TAGSIZE)));
}


/*-------------------------------------------------------------------
    Protected: FfxMMNorDeviceInit()

    Initialize the supplied MMDEVICE structure with the
    necessary function pointer to enable this interface.

    Note that this function is located at the end of the
    module to avoid having to create prototypes that would
    otherwise be unnecessary.

    Parameters:
        pMM - A pointer to the MMDEVICE structure to initialize

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxMMNorDeviceInit(
    MEDIAMGRINSTANCE *pMM)
{
    static const MMDEVICE ops =
    {
        ReadBlocks,
        WriteBlocks,
        ReadAllocations,
        DiscardAllocations,
        DiscardAllocations
    };

    DclAssert(pMM);

    pMM->pMmOps = &ops;
}



#endif  /* FFXCONF_NORSUPPORT */
#endif  /* FFXCONF_VBFSUPPORT */

