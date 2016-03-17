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

    This module contains FMSL tests which apply only to NOR or ISWF (Sibley)
    flash.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmslnor.c $
    Revision 1.14  2009/08/04 18:58:20Z  garyp
    Minor datatype update.
    Revision 1.13  2009/07/20 20:46:04Z  garyp
    Merged from the v4.0 branch.  Minor datatype changes.  Docs updated.
    Revision 1.12  2009/04/09 02:58:25Z  garyp
    Renamed a structure to avoid AutoDoc naming collisions.
    Revision 1.11  2009/03/31 21:20:41Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.10  2008/01/13 07:26:51Z  keithg
    Function header updates to support autodoc.
    Revision 1.9  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.8  2007/10/16 13:57:26Z  Garyp
    Minor enhancements to the _sysdelay() tests for better accuracy.
    Revision 1.7  2007/10/11 02:06:29Z  Garyp
    Per code review of fmsltst.c rev 1.29, reverted the sysdelay() tests so they
    use the standard ticker to do the measurements, to avoid the possibility
    that we would be measuring a sysdelay() implementation which uses the
    HighRes tick, using the HighRes ticker.
    Revision 1.6  2007/08/24 20:25:30Z  pauli
    Changed the control read write tests to be run as part of the NOR
    tests.  FfxFmslNORControlReadWrite is now ControlReadWriteTests.
    Revision 1.5  2007/05/26 21:36:54Z  Garyp
    Removed some commented out obsolete code.
    Revision 1.4  2007/02/27 21:03:36Z  Garyp
    Migrated NOR specific tests from fmsltst.c into this module.
    Revision 1.3  2007/01/03 02:07:56Z  Garyp
    Minor type changes -- nothing functional.
    Revision 1.2  2006/05/22 20:47:19Z  billr
    Fix type in include file name so this will compile.
    Revision 1.1  2006/05/20 19:06:08Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxfmlapi.h>
#include "fmsltst.h"

#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT

#define MILLISECS_TO_DELAY      (50UL)
#define MICROSECS_PER_MILLISEC  (1000UL)

typedef struct NORTESTINFO
{
    FFXFMLHANDLE        hFML;
    D_UINT32           *pulRandomSeed;
    D_UINT32            ulSerial;
    FFXFMLINFO          FmlInfo;
    FFXFMLDEVINFO       FmlDevInfo;
    D_BUFFER           *pBuffer;
    D_BUFFER           *pPatBuff;
    unsigned            fExtensive;
} NORTESTINFO;

static D_BOOL BitWriteAndVerify(FFXFMLHANDLE hFML, D_UINT32 ulStart, D_UINT32 ulData, FFXFMLINFO *pFmlInfo);
static D_BOOL BootBlockTests(NORTESTINFO *pTI);
static D_BOOL ControlReadWriteTests(NORTESTINFO *pTI);
static D_BOOL TestBootBlockArea(NORTESTINFO *pTI, D_UINT32 ulStartBB, D_UINT32 ulCount, D_UINT32 ulBBsPerBlock);
static D_BOOL WriteBlockPattern(NORTESTINFO *pTI, D_UINT32 ulStart, D_UINT32 ulSize);
static D_BOOL VerifyPattern(NORTESTINFO *pTI, D_UINT32 ulStart, D_UINT32 ulSize);
static void   FillPatternBuff(NORTESTINFO *pTI, D_UINT32 ulStart, D_UINT32 ulSize);


/*-------------------------------------------------------------------
    Protected: FfxFmslNORTest()

    This function invokes NOR/Sibley specific aspects of
    FMSLTEST.

    Parameters:
        hFML          - The FML handle
        pulRandomSeed - A pointer to the random seed to use.
        fExtensive    - A TRUE/FALSE flag to indicate whether the
                        extensive version of the test should be run.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
D_BOOL FfxFmslNORTest(
    FFXFMLHANDLE    hFML,
    D_UINT32       *pulRandomSeed,
    D_BOOL          fExtensive)
{
    NORTESTINFO     ti;
    D_BOOL          fSuccess = FALSE;

    DclMemSet(&ti, 0, sizeof(ti));

    FfxFmlDiskInfo(hFML, &ti.FmlInfo);
    FfxFmlDeviceInfo(ti.FmlInfo.nDeviceNum, &ti.FmlDevInfo);

    ti.hFML = hFML;
    ti.fExtensive = fExtensive;
    ti.pulRandomSeed = pulRandomSeed;
    DclPrintf("NOR Specific Tests\n");

    /*  Validate assumptions about the media and test parameters that
        are vital to the rest of the test code.  Some or all of these
        checks should really be done at the top level of the FMSL tests.
    */
    if(ti.FmlInfo.uDeviceType != DEVTYPE_NOR && ti.FmlInfo.uDeviceType != DEVTYPE_ISWF)
    {
        DclPrintf("  ERROR: Flash is not NOR or Sibley\n");
        return FALSE;
    }
    if (ti.FmlInfo.ulBlockSize == 0)
    {
        DclPrintf("  ERROR: erase zone size is zero\n");
        return FALSE;
    }
    if (!DCLISPOWEROF2(ti.FmlInfo.ulBlockSize))
    {
        DclPrintf("  ERROR: erase zone size is not a power of 2\n");
        return FALSE;
    }

    ti.pBuffer = DclMemAlloc(ti.FmlInfo.uPageSize);
    if(!ti.pBuffer)
        goto Cleanup;

    ti.pPatBuff = DclMemAlloc(ti.FmlInfo.uPageSize);
    if(!ti.pPatBuff)
        goto Cleanup;

    fSuccess = ControlReadWriteTests(&ti);
    if(!fSuccess)
        goto Cleanup;

    fSuccess = BootBlockTests(&ti);
    if(!fSuccess)
        goto Cleanup;

  Cleanup:

    /*  Release resources in reverse order of allocation
    */
    if(ti.pPatBuff)
        DclMemFree(ti.pPatBuff);

    if(ti.pBuffer)
        DclMemFree(ti.pBuffer);

    return fSuccess;
}


/*-------------------------------------------------------------------
    Local: ControlReadWriteTests()

    This function tests control read/write functionality.

    Parameters:
        pTI - A pointer a NORTESTINFO structure to use.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL ControlReadWriteTests(
    NORTESTINFO    *pTI)
{
    D_UINT32        ulOffset;
    FFXIOSTATUS     ioStat;
    D_UINT16        uIndex;
    #define  NUM_SMALL_WRITES  (4)
    D_UINT32        ulData[NUM_SMALL_WRITES];

    DclPrintf("  NOR Control Read/Write tests...\n");

    FMLERASE_BLOCKS(pTI->hFML, 0, 1, ioStat);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("      Erase failed, %s\n", FfxDecodeIOStatus(&ioStat));
    }

    /*  Test writing small (32-bit) data chunks
    */
    ulData[0] = 0xF2345678L;
    ulData[1] = 0x87654E21L;
    ulData[2] = 0x10D93847L;
    ulData[3] = 0xF8A7830CL;
    ulOffset = 0;
    for(uIndex = 0;
        uIndex < NUM_SMALL_WRITES;
        uIndex++, ulOffset += sizeof(D_UINT32))
    {
        if(!BitWriteAndVerify(pTI->hFML, ulOffset, ulData[uIndex], &pTI->FmlInfo))
        {
            DclPrintf("    Control write failed, Offset: %lX, Index: %U\n",
                      ulOffset, uIndex);
            return FALSE;
        }
    }

    /*  Test overwriting small (32-bit) data chunks
    */
    ulData[0] = 0xFFFFFFF8L;
    ulData[1] = 0xFCFFFF00L;
    ulData[2] = 0xC8FF5500L;
    ulData[3] = 0xC87F0000L;
    ulOffset = NUM_SMALL_WRITES * sizeof(D_INT32);
    for(uIndex = 0; uIndex < NUM_SMALL_WRITES; uIndex++)
    {
        if(!BitWriteAndVerify(pTI->hFML, ulOffset, ulData[uIndex], &pTI->FmlInfo))
        {
            DclPrintf("    Control overwrite failed, Offset: %lX, Index: %U\n",
                      ulOffset, uIndex);
            return FALSE;
        }
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: FfxFmslNORSysDelay()

    Parameters:
        None

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
D_BOOL FfxFmslNORSysDelay(void)
{
    D_UINT32        ulResolution;
    D_UINT32        ulStartTicks;
    D_UINT32        ulElapsedTicks;
    D_UINT32        ulMicroSecsToDelay;
    D_UINT32        ulTicksToDelay;
    D_UINT16        k;

    DclPrintf("Testing delay routines...\n");

    ulResolution = DclOsTickResolution();
    DclPrintf("    Tick Resolution=%lU us per tick, Tick Modulus=%lU\n",
              ulResolution, DclOsTickModulus());

    DclPrintf("    Current Tick Count %lU\n", DclOsTickCount());

    /*  Do an initial _sysdelay() now to ensure that any one-time
        initialization stuff does not skew these test results.
    */
    _sysdelay(1000);

    /*  Calculate the number of ticks to delay to at least get a 50ms delay
                      MILLISECS_TO_DELAY * MICROSECS_PER_MILLISEC
        ticks =      --------------------------------------------
                               MICROSECS_PER_TICK

        Round up to ensure at least a 50ms timeout
    */
    ulTicksToDelay = ((MILLISECS_TO_DELAY * MICROSECS_PER_MILLISEC) +
        ulResolution - 1) / ulResolution;

    /*  Now get the actual value of microsecs to delay (should
        be above MILLISECS_TO_DELAY * MICROSECS_PER_MILLISEC).
    */
    ulMicroSecsToDelay = ulTicksToDelay * ulResolution;

    DclPrintf("    Starting 1 %lU us delay (~%lU system ticks)...\n",
                  ulMicroSecsToDelay, ulTicksToDelay);

    ulStartTicks = DclOsTickCount();
    {
        _sysdelay(ulMicroSecsToDelay);
    }
    ulElapsedTicks = DclOsTickCount() - ulStartTicks;

    if(ulElapsedTicks > ulTicksToDelay + 1)
    {
        /*  If we see one more tick than we should it is okay, if more warn
        */
        DclPrintf("    NOTE: Requested a total delay of %lU ticks, but %lU ticks elapsed.\n",
             ulTicksToDelay, ulElapsedTicks);
    }
    else if(ulElapsedTicks < ulTicksToDelay)
    {
        /*  Delay is less than ulTicksToDelay this is a problem
        */
        DclPrintf("    NOTE: Requested a total delay of %lU ticks, but only %lU ticks elapsed.\n",
             ulTicksToDelay, ulElapsedTicks);

        /*  return FALSE;
        */
    }

    DclPrintf("    Starting 50 sequential %lU us delays (~%lU system ticks)...\n",
               ulMicroSecsToDelay / 50, ulTicksToDelay);

    ulMicroSecsToDelay = ulMicroSecsToDelay / 50;

    ulStartTicks = DclOsTickCount();
    {
        for(k = 0; k < 50; k++)
            _sysdelay(ulMicroSecsToDelay);

        /*  Add one extra delay to account for truncation
        */
        _sysdelay((ulTicksToDelay * ulResolution) % 50);
    }
    ulElapsedTicks = DclOsTickCount() - ulStartTicks;

    if(ulElapsedTicks > ulTicksToDelay + 1)
    {
        DclPrintf("    NOTE: Requested a total delay of %lU ticks, but %lU ticks elapsed.\n",
             ulTicksToDelay, ulElapsedTicks);
    }
    else if(ulElapsedTicks < ulTicksToDelay)
    {
        /*  Delay is less than ulTicksToDelay this is a problem
        */
        DclPrintf("    NOTE: Requested a total delay of %lU ticks, but only %lU ticks elapsed.\n",
             ulTicksToDelay, ulElapsedTicks);

        /*  return FALSE;
        */
    }

    ulMicroSecsToDelay = ulTicksToDelay * ulResolution;

    DclPrintf("    Starting 500 sequential %lU us delays (~%lU system ticks)...\n",
               ulMicroSecsToDelay / 500, ulTicksToDelay);

    ulMicroSecsToDelay = ulMicroSecsToDelay / 500;

    ulStartTicks = DclOsTickCount();
    {
        for(k = 0; k < 500; k++)
            _sysdelay(ulMicroSecsToDelay);

        /*  Add one extra delay to account for truncation
        */
        _sysdelay((ulTicksToDelay * ulResolution) % 500);
    }
    ulElapsedTicks = DclOsTickCount() - ulStartTicks;

    if(ulElapsedTicks > ulTicksToDelay + 1)
    {
        DclPrintf("    NOTE: Requested a total delay of %lU ticks, but %lU ticks elapsed.\n",
             ulTicksToDelay, ulElapsedTicks);
    }
    else if(ulElapsedTicks < ulTicksToDelay)
    {
        /*  Delay is less than ulTicksToDelay this is a problem
        */
        DclPrintf("    NOTE: Requested a total delay of %lU ticks, but only %lU ticks elapsed.\n",
             ulTicksToDelay, ulElapsedTicks);

        /*  return FALSE;
        */
    }

    DclPrintf("    PASSED\n");

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: BitWriteAndVerify()

    Writes the data in pcBlockBuffer1 and verifies it in 2.

    Parameters:
        hFML    - The handle denoting the FML to use
        ulStart - Starting offset to read the media

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL BitWriteAndVerify(
    FFXFMLHANDLE    hFML,
    D_UINT32        ulStart,
    D_UINT32        ulData,
    FFXFMLINFO     *pFmlInfo)
{
    D_UINT32        ulReadBack;
    D_UINT32        ulAddress = ~(D_UINT32) 0;
    FFXIOSTATUS     ioStat;

    DclAssert(pFmlInfo);

    /*  This initial read is for debugging purposes to check
        the value of the flash before it is written to!
    */
	switch(pFmlInfo->uDeviceType)
	{
      #if FFXCONF_ISWFSUPPORT
    	case DEVTYPE_ISWF:
    		ulAddress = FfxFmslISWFCalcControlAddress(ulStart, pFmlInfo->uPageSize);
    		break;
      #endif

    	default:
            ulAddress = ulStart;
	}

    FMLREAD_CONTROLDATA(hFML, ulAddress, sizeof(D_UINT32), (D_BUFFER*)&ulReadBack, ioStat);
    if(!IOSUCCESS(ioStat, sizeof(D_UINT32)))
    {
        DclPrintf("  BitWriteAndVerify 1 FMLREAD_CONTROLDATA() failed!\n");
        return FALSE;
    }

    /*  Test writing small data chunks
    */
    FMLWRITE_CONTROLDATA(hFML, ulAddress, sizeof(D_UINT32), (D_BUFFER*)&ulData, ioStat);
    if(!IOSUCCESS(ioStat, sizeof(D_UINT32)))
    {
        DclPrintf("  BitWriteAndVerify FMLWRITE_CONTROLDATA() failed!\n");
        return FALSE;
    }

    /*  Read back the four bytes that were written to verify they
        were written correctly
    */
    FMLREAD_CONTROLDATA(hFML, ulAddress, sizeof(D_UINT32), (D_BUFFER*)&ulReadBack, ioStat);
    if(!IOSUCCESS(ioStat, sizeof(D_UINT32)))
    {
        DclPrintf("  BitWriteAndVerify 2 FMLREAD_CONTROLDATA() failed!\n");
        return FALSE;
    }

    if(ulData != ulReadBack)
    {
        DclPrintf("    32 bit write verify failed!\n");
        return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: BootBlockTests()

    Parameters:
        hFML - The handle denoting the FML to use

    Return Value:
        Returns TRUE if successful, FALSE otherwise.
 -------------------------------------------------------------------*/
static D_BOOL BootBlockTests(
    NORTESTINFO        *pTI)
{
    D_UINT32            ulBBsPerBlock;  /* boot blocks per block */
    D_UINT32            ulBlocks;
    D_UINT32            ulAbsStartBlk;
    unsigned            ChpBlk;

    DclPrintf("  NOR Boot Block Tests...\n");

    if(!pTI->FmlDevInfo.ulBootBlockSize)
    {
        DclPrintf("    Device has no boot blocks\n");
        return TRUE;
    }

    DclAssert(pTI->FmlDevInfo.ulChipBlocks);

    /*  If a boot block size is specified, at least one of these must
        be set.
    */
    DclAssert(pTI->FmlDevInfo.uLowBootBlockCount || pTI->FmlDevInfo.uHighBootBlockCount);

    /*  calculate the number of boot blocks in each regular block
    */
    ulBBsPerBlock = pTI->FmlDevInfo.ulBlockSize / pTI->FmlDevInfo.ulBootBlockSize;

    /*  calc absolute block where the FML window starts
    */
    ulAbsStartBlk = pTI->FmlDevInfo.ulReservedBlocks + pTI->FmlInfo.ulStartBlock;

    if(!pTI->FmlDevInfo.uLowBootBlockCount)
        DclPrintf("    Device has no low boot blocks\n");

    if(!pTI->FmlDevInfo.uHighBootBlockCount)
        DclPrintf("    Device has no high boot blocks\n");

    for(ChpBlk  = 0;
        ChpBlk  < pTI->FmlDevInfo.ulReservedBlocks + pTI->FmlDevInfo.ulTotalBlocks;
        ChpBlk += pTI->FmlDevInfo.ulChipBlocks)
    {
        D_UINT32    ulChipNum;
        D_UINT32    ulChipStart;
        D_UINT32    ulChipLen;

        ulChipNum = ChpBlk / pTI->FmlDevInfo.ulChipBlocks;

        if(ChpBlk + pTI->FmlDevInfo.ulChipBlocks <= pTI->FmlDevInfo.ulReservedBlocks)
        {
            DclPrintf("    Chip %lU falls within reserved space\n", ulChipNum);
            continue;
        }

        if(ChpBlk + pTI->FmlDevInfo.ulChipBlocks <= ulAbsStartBlk)
        {
            DclPrintf("    Chip %lU resides before the mapped FML window\n", ulChipNum);
            continue;
        }

        if(ulAbsStartBlk + pTI->FmlInfo.ulTotalBlocks <= ChpBlk)
        {
            DclPrintf("    Chip %lU resides after the mapped FML window\n", ulChipNum);
            continue;
        }

        /*  Calculate starting block within the chip where the window starts
        */
        if(ChpBlk >= ulAbsStartBlk)
            ulChipStart = 0;
        else
            ulChipStart = pTI->FmlDevInfo.ulChipBlocks - (ulAbsStartBlk % pTI->FmlDevInfo.ulChipBlocks);

        /*  Calculate blocks remaining within this chip
        */
        if(ulAbsStartBlk + pTI->FmlInfo.ulTotalBlocks >= ChpBlk + pTI->FmlDevInfo.ulChipBlocks)
        {
            /*  The end of the window extends past the chip boundary
            */
            ulChipLen = pTI->FmlDevInfo.ulChipBlocks - ulChipStart;
        }
        else
        {
            /*  The window ends within this chip, so reduce the count
            */
            ulChipLen = ((ulAbsStartBlk + pTI->FmlInfo.ulTotalBlocks) %
                pTI->FmlDevInfo.ulChipBlocks) - ulChipStart;
        }

        if(pTI->FmlDevInfo.uLowBootBlockCount)
        {
            ulBlocks = pTI->FmlDevInfo.uLowBootBlockCount / ulBBsPerBlock;

            if(ulBlocks > ulChipStart)
            {
                ulBlocks -= ulChipStart;

                DclPrintf("    Testing %lU low boot block areas in chip %lU...\n",
                    ulBlocks * ulBBsPerBlock, ulChipNum);

                if(!TestBootBlockArea(pTI,
                    (ChpBlk + ulChipStart - ulAbsStartBlk) * ulBBsPerBlock,
                    ulBlocks * ulBBsPerBlock, ulBBsPerBlock))
                {
                    return FALSE;
                }
            }
            else
            {
                DclPrintf("    Chip %lU low boot block area is not within the mapped FML window\n", ulChipNum);
            }
        }

        if(pTI->FmlDevInfo.uHighBootBlockCount)
        {
            ulBlocks = pTI->FmlDevInfo.uHighBootBlockCount / ulBBsPerBlock;

            if(ulBlocks >= pTI->FmlDevInfo.ulChipBlocks - (ulChipStart + ulChipLen))
                ulBlocks -= pTI->FmlDevInfo.ulChipBlocks - (ulChipStart + ulChipLen);
            else
                ulBlocks = 0;

            if(ulBlocks)
            {
                ulChipStart += ulChipLen - ulBlocks;

                DclPrintf("    Testing %lU high boot block areas in chip %lU...\n",
                    ulBlocks * ulBBsPerBlock, ulChipNum);

                if(!TestBootBlockArea(pTI,
                    (ChpBlk + ulChipStart - ulAbsStartBlk) * ulBBsPerBlock,
                    ulBlocks * ulBBsPerBlock, ulBBsPerBlock))
                {
                    return FALSE;
                }
            }
            else
            {
                DclPrintf("    Chip %lU high boot block area is not within the mapped FML window\n", ulChipNum);
            }
        }

    }

    DclPrintf("    PASSED\n");

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: TestBootBlockArea()

    Parameters:
        hFML - The handle denoting the FML to use

    Return Value:
        Returns TRUE if successful, FALSE otherwise.
 -------------------------------------------------------------------*/
static D_BOOL TestBootBlockArea(
    NORTESTINFO        *pTI,
    D_UINT32            ulStartBB,
    D_UINT32            ulCount,
    D_UINT32            ulBBsPerBlock)
{
    unsigned            ii;
    FFXIOSTATUS         ioStat;
    D_UINT32            ulPrior = D_UINT32_MAX;
    D_UINT32            ulAfter = D_UINT32_MAX;

    DclAssert(pTI);
    DclAssert(ulCount);
    DclAssert(ulBBsPerBlock);

    /*  If the window mapping allows it, initialize the block prior
        to the boot blocks.
    */
    if(ulStartBB != 0)
    {
        ulPrior = (ulStartBB - ulBBsPerBlock) / ulBBsPerBlock;

        DclPrintf("      Writing pattern to the block prior to the boot blocks\n");

        FMLERASE_BLOCKS(pTI->hFML, ulPrior, 1, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      Erase failed, %s\n", FfxDecodeIOStatus(&ioStat));
        }

        WriteBlockPattern(pTI, ulPrior, pTI->FmlInfo.ulBlockSize);
    }

    /*  Initialize the boot blocks.
    */
    DclPrintf("      Writing pattern to %lU boot blocks at block %lU\n", ulCount, ulStartBB);

    FMLERASE_BOOT_BLOCKS(pTI->hFML, ulStartBB, ulCount, ioStat);
    if(!IOSUCCESS(ioStat, ulCount))
    {
        DclPrintf("      Erase failed, %s\n", FfxDecodeIOStatus(&ioStat));
    }

    for(ii=0; ii<ulCount; ii++)
    {
        WriteBlockPattern(pTI, ulStartBB+ii, pTI->FmlDevInfo.ulBootBlockSize);
    }

    /*  If the window mapping allows it, initialize the block after
        the boot blocks.
    */
    if(pTI->FmlInfo.ulTotalBlocks * ulBBsPerBlock > ulStartBB + ulCount)
    {
        ulAfter = (ulStartBB + ulCount) / ulBBsPerBlock;

        DclPrintf("      Writing pattern to the block after the boot blocks\n");

        FMLERASE_BLOCKS(pTI->hFML, ulAfter, 1, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      Erase failed, %s\n", FfxDecodeIOStatus(&ioStat));
        }

        WriteBlockPattern(pTI, ulAfter, pTI->FmlInfo.ulBlockSize);
    }

    /*  At this point, all the boot blocks, and potentially the blocks
        before and after the boot blocks have been initialized to a known
        pattern.
    */

    for(ii=0; ii<ulCount; ii++)
    {
        DclPrintf("      Erasing boot block %lU and verifying neighboring blocks\n", ulStartBB+ii);

        FMLERASE_BOOT_BLOCKS(pTI->hFML, ulStartBB+ii, 1, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      Erase failed, %s\n", FfxDecodeIOStatus(&ioStat));
        }

        if(ii)
        {
            if(!VerifyPattern(pTI, ulStartBB+ii-1, pTI->FmlDevInfo.ulBootBlockSize))
                return FALSE;
        }

        if(ii < ulCount-1)
        {
            if(!VerifyPattern(pTI, ulStartBB+ii+1, pTI->FmlDevInfo.ulBootBlockSize))
                return FALSE;
        }

        WriteBlockPattern(pTI, ulStartBB+ii, pTI->FmlDevInfo.ulBootBlockSize);
    }

    if(ulPrior != D_UINT32_MAX)
    {
        DclPrintf("      Verifying the preceding full block\n");
        if(!VerifyPattern(pTI, ulPrior, pTI->FmlInfo.ulBlockSize))
            return FALSE;
    }

    if(ulAfter != D_UINT32_MAX)
    {
        DclPrintf("      Verifying the subsequent full block\n");
        if(!VerifyPattern(pTI, ulAfter, pTI->FmlInfo.ulBlockSize))
            return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: WriteBlockPattern()

    Parameters:
        hFML - The handle denoting the FML to use

    Return Value:
        Returns TRUE if successful, FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL WriteBlockPattern(
    NORTESTINFO        *pTI,
    D_UINT32            ulStart,
    D_UINT32            ulSize)
{
    D_UINT32            ulStartPage;
    unsigned            ii;
    FFXIOSTATUS         ioStat;

    DclAssert(pTI);
    DclAssert(ulSize);

    ulStartPage = (ulStart * ulSize) / pTI->FmlInfo.uPageSize;

    for(ii=0; ii<ulSize / pTI->FmlInfo.uPageSize; ii++)
    {
        FillPatternBuff(pTI, ulStartPage+ii, ulSize);

        FMLWRITE_PAGES(pTI->hFML, ulStartPage+ii, 1, pTI->pPatBuff, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      Page write failed at page %u, %s\n", ii, FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: VerifyPattern()

    Parameters:
        hFML - The handle denoting the FML to use

    Return Value:
        Returns TRUE if successful, FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL VerifyPattern(
    NORTESTINFO        *pTI,
    D_UINT32            ulStart,
    D_UINT32            ulSize)
{
    D_UINT32            ulStartPage;
    unsigned            ii;
    FFXIOSTATUS         ioStat;

    DclAssert(pTI);
    DclAssert(ulSize);

    ulStartPage = (ulStart * ulSize) / pTI->FmlInfo.uPageSize;

    for(ii=0; ii<ulSize / pTI->FmlInfo.uPageSize; ii++)
    {
        FillPatternBuff(pTI, ulStartPage+ii, ulSize);

        FMLREAD_PAGES(pTI->hFML, ulStartPage+ii, 1, pTI->pBuffer, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      Page read failed at page %u, %s\n", ii, FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }

        if(DclMemCmp(pTI->pBuffer, pTI->pPatBuff, pTI->FmlInfo.uPageSize) != 0)
        {
            DclPrintf("      Page mismatch at page %u\n");
            return FALSE;
        }
     }

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: FillPatternBuff()

    Parameters:
        hFML - The handle denoting the FML to use

    Return Value:
        None.
-------------------------------------------------------------------*/
static void FillPatternBuff(
    NORTESTINFO    *pTI,
    D_UINT32        ulStart,
    D_UINT32        ulSize)
{
    int             nLen = 0;
    char            szFormatString[] = "%lX %lX  "; /* an odd length */

    DclAssert(pTI);
    DclAssert(ulSize);

    /*  establish verifiable pattern
    */
    while(nLen != pTI->FmlInfo.uPageSize)
    {
        int     nThis;

        nThis = DclSNPrintf((char*)&pTI->pPatBuff[nLen], pTI->FmlInfo.uPageSize-nLen,
            &szFormatString[0], ulStart, ulSize);

        if(nThis == -1)
            break;

        nLen += nThis;
    }

    return;
}





#endif


