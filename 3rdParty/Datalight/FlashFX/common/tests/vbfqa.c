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

    This module contains extra VBF QA tests that involve special test
    conditions and thus are not suitable for inclusion in the general
    VBF test.  Each of these tests is identified by the tracking number
    for the issue it tests.

    These tests should be run only if the specific conditions they require
    exist.  The tests attempt to verify these conditions (where possible)
    and will report failure if the needed conditions do not exist.

    504: Possible infinite compaction loop on NAND with 16 KB erase unit.

        For this test to work, the configuration must be such that the erase
        unit size is 16 KB, and there is one unit in the last region of the
        partition.  This generally involves editing ffxconf.h to adjust the
        disk size.  It needs to be under the size defined by
        VBF_PARTITION_MAX_UNITS times 16 KB (so that each unit is a single
        erase block).

    546: VBF unit tests "Discard Set" test doesn't account for multiple
         region caches

        This is probably a good candidate to move here from the main VBF
        test, as it also requires very specific test conditions and is
        meaningless to run without them.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: vbfqa.c $
    Revision 1.9  2009/07/21 22:11:34Z  garyp
    Merged from the v4.0 branch.  Updated the compaction functions to take
    a compaction level.  Updated for new compaction functions which now
    return an FFXIOSTATUS value rather than a D_BOOL.
    Revision 1.8  2009/04/01 14:46:54Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.7  2008/05/05 22:12:32Z  keithg
    Function header comment updates and additions.
    Revision 1.6  2008/02/06 20:22:42Z  Garyp
    Made allocator/VBF code conditional on FFXCONF_ALLOCATORSUPPORT and
    FFXCONF_VBFSUPPORT, respectively.
    Revision 1.5  2008/01/13 07:26:55Z  keithg
    Function header updates to support autodoc.
    Revision 1.4  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2007/10/30 18:19:26Z  billr
    Update bug 504 test: accommodate changes in behavior caused by the fix for
    bug 1556 in alloc/vbf/core/vbfreg.c revision 1.26, and add more compaction
    cases.
    Revision 1.2  2007/10/15 18:43:02Z  billr
    Two passes, not three.
    Revision 1.1  2007/10/12 22:29:26Z  billr
    Initial revision
    ---------------------
    Bill Roman 2007-10-11
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_VBFSUPPORT

#include <fxtools.h>
#include <dlperflog.h>
#include <vbf.h>
#include <fxiosys.h>
#include "vbftst.h"


static D_INT16 Test504(VBFTESTINFO *pTI);


/*---------------------------------------------------------------------------
    Protected: FfxTestVBFQA()

    Interface to the VBF QA extended tests.

    Parameters:
        pTI      - A pointer to the VBFTESTINFO structure
        uTestNum - Number specifying which test to run

    Return Value:
        SUCCESS_RETURN_CODE - the test completed successfully.
        (a line number)     - the line at which an error was detected; it is
                              an error to attempt this test with inappropriate
                              test conditions.
---------------------------------------------------------------------------*/
D_INT16 FfxTestVBFQA(VBFTESTINFO *pTI)
{
    D_INT16 result;

    switch (pTI->lQATestNum)
    {
        case 504:
            result = Test504(pTI);
            break;

        default:
            DclPrintf("VBF Extended QA test: unknown test number %ld\n", pTI->lQATestNum);
            result = __LINE__;
            break;
    }

    return result;
}


/*---------------------------------------------------------------------------
    Local: Test504()

    Completes the reression test against bug number 504.
    This test requires very specific conditions: there must be only one
    unit in the last region, and it must have only one allocation block
    (page) of cushion.  This can happen on real small-block NAND flash if
    the disk size is small enough that erase units are single blocks: the
    (raw) erase unit size is 16 KB; there are 31 available pages after
    out one for the EUH, and one page of cushion is all that is needed
    for the default of 2.5%.  Formatting with other values for cushion
    could potentiall

    On entry, there are no assumptions about the state of the last
    region.  There may or may not be valid data, and may or may not be a
    discard metadata record.  Depending on what's there, the bug may
    manifest itself earlier or later in this test.  The expected failure
    is a write error due to exhausting compaction retries.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        SUCCESS_RETURN_CODE - the test completed successfully.
        (a line number)     - the line at which an error was detected; it is
                              an error to attempt this test with inappropriate
                              test conditions.
---------------------------------------------------------------------------*/
static D_INT16 Test504(VBFTESTINFO *pTI)
{
    D_UINT32 ulStartPage, ulPage, ulDiscardPage;
    int iCompactMode;

    DclPrintf("\nQA Test 504\n");

    /*  Verify that the correct conditions for this test exist.
    */
    if (pTI->DiskMets.uUnitDataPages - pTI->DiskMets.uUnitClientPages != 1)
    {
        DclPrintf("    FAILED: cannot test: more than one allocation block of cushion per unit\n"
                  "    %u data pages, %u client pages per unit\n",
                  pTI->DiskMets.uUnitDataPages, pTI->DiskMets.uUnitClientPages);
        return __LINE__;
    }

    if (pTI->DiskMets.ulClientPages % pTI->DiskMets.ulRegionClientPages
        != pTI->DiskMets.uUnitClientPages)
    {
        DclPrintf("   FAILED: cannot test: last region is not a single unit\n"
                  "   Total size %lU pages, region size %lU pages, unit size %u pages\n",
                  pTI->DiskMets.ulClientPages, pTI->DiskMets.ulRegionClientPages,
                  pTI->DiskMets.uUnitClientPages);
        return __LINE__;
    }

    DclAssert(pTI->ulTestBufferSize >= pTI->DiskMets.uPageSize);
    DclMemSet(pTI->pBuffer, 'X', pTI->DiskMets.uPageSize);

    /*  First page of last region.
    */
    ulStartPage = (pTI->DiskMets.ulClientPages
                   - pTI->DiskMets.ulClientPages % pTI->DiskMets.ulRegionClientPages);

    /*  Run four passes of the test: without any compaction, compacting
        after the write but before the discard, compacting after the
        discard, and compacting both before and after the discard.
    */
    for (iCompactMode = 0; iCompactMode <= 3; ++iCompactMode)
    {
        switch (iCompactMode)
        {
            case 0:
                DclPrintf("    Without forced compaction\n");
                break;
            case 1:
                DclPrintf("    With forced compaction before discard\n");
                break;
            case 2:
                DclPrintf("    With forced compaction after discard\n");
                break;
            case 3:
                DclPrintf("    With forced compaction before and after discard\n");
                break;
            default:
                DclProductionError();
                break;
        }

        /*  Write the entire region (its single unit), then discard a single
            allocation block.  Repeat discarding a different block each time
            until every block in the region has been tried.

            Note that the last pass of this loop is a special case to
            overwrite the region when the very last allocation block has been
            discarded, and does not do a discard at the end.
        */
        for (ulDiscardPage = ulStartPage;
             ulDiscardPage <= ulStartPage + pTI->DiskMets.uUnitClientPages;
             ++ulDiscardPage)
        {
            FFXIOSTATUS ioStat;

            /*  Write to the entire region (one unit).  If there's already a
                discard metadata record, bug 504 will cause repeated failure
                of compaction, and ultimately a write failure.
            */
            for (ulPage = ulStartPage;
                 ulPage < ulStartPage + pTI->DiskMets.uUnitClientPages;
                 ++ulPage)
            {
                ioStat = FfxVbfWritePages(pTI->hVBF, ulPage, 1, pTI->pBuffer);

                if (ioStat.ffxStat != FFXSTAT_SUCCESS || ioStat.ulCount != 1)
                {
                    DclPrintf("    FAILED: FfxVbfWritePages() failed: status = %lX, count = %u\n",
                              ioStat.ffxStat, ioStat.ulCount);
                    return __LINE__;
                }
            }

            /*  At this point there are 30 valid blocks; there may be a
                discard record, a discarded block, or a free block.
            */
            if ((iCompactMode & 1) != 0)
            {
                do
                {
                    ioStat = FfxVbfCompact(pTI->hVBF, FFX_COMPACTFLAGS_EVERYTHING);
                }
                while(ioStat.ffxStat == FFXSTAT_SUCCESS);
            }

            /*  Discard the nth allocation block of the region.  This will
                create a discard metadata record to record the discard.  Now
                there are 29 valid blocks, a discard record, and either a
                discarded block or a free block.
            */
            if (ulDiscardPage < ulStartPage + pTI->DiskMets.uUnitClientPages)
            {
                ioStat = FfxVbfDiscardPages(pTI->hVBF, ulDiscardPage, 1);
                if (ioStat.ffxStat != FFXSTAT_SUCCESS || ioStat.ulCount != 1)
                {
                    DclPrintf("    FAILED: FfxVbfDiscardPages() failed: status = %lX, count = %u\n",
                              ioStat.ffxStat, ioStat.ulCount);
                    return __LINE__;
                }
                DclPrintf("    %4lU of %lU\n", ulDiscardPage - ulStartPage + 1,
                          pTI->DiskMets.uUnitClientPages);

                /*  On second pass, always compact fully after the discard.
                */
                if ((iCompactMode & 2) != 0)
                {
                    do
                    {
                        ioStat = FfxVbfCompact(pTI->hVBF, FFX_COMPACTFLAGS_EVERYTHING);
                    }
                    while(ioStat.ffxStat == FFXSTAT_SUCCESS);
                }
            }
        }
    }

    DclPrintf("    PASSED\n");

    return SUCCESS_RETURN_CODE;
}


#endif  /* FFXCONF_VBFSUPPORT */


