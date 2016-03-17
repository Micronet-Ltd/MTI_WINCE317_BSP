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

    This module contains code for testing block locking/unlocking.  

    ToDo:
    - Add more test cases for exercising various lock modes.
    - Add more test cases for locking only portions of a Disk.
    - Add more test cases to ensure that things like Tag Writes and Spare
      Writes are properly blocked for locked blocks (and appropriate error
      code returned).
    - At some point if and when generalized block locking is supported for
      NOR, this test should be updated to work for NOR as well.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmsllock.c $
    Revision 1.6  2010/12/19 23:27:47Z  billr
    Fix warning that a structure member may be used uninitialized.
    Revision 1.5  2010/04/29 00:04:20Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.4  2009/12/12 00:10:38Z  garyp
    Fixed to properly initialize a local.
    Revision 1.3  2009/11/25 22:54:10Z  garyp
    Enhanced the locking test to exercise more conditions.
    Revision 1.2  2009/11/18 17:24:36Z  garyp
    Conditioned the code to build only if NAND support is enabled.
    Revision 1.1  2009/11/17 22:52:16Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxfmlapi.h>
#include <fxdevapi.h>
#include <fxtools.h>
#include "fmsltst.h"

#if FFXCONF_NANDSUPPORT

#define WHOLE_DISK_PAGE_START       (0)
#define WHOLE_DISK_PAGE_COUNT       (2)
#define SINGLE_RANGE_PAGE_START     (WHOLE_DISK_PAGE_START + WHOLE_DISK_PAGE_COUNT)
#define SINGLE_RANGE_PAGE_COUNT     (6)
#define INVERTED_RANGE_PAGE_START   (SINGLE_RANGE_PAGE_START + SINGLE_RANGE_PAGE_COUNT)
#define INVERTED_RANGE_PAGE_COUNT   (6)

typedef struct
{
    FFXFMLHANDLE    hFML;
    FFXFMLINFO      FmlInfo;
    FFXFMLDEVINFO   FmlDevInfo;
    D_UINT32       *pulRandomSeed;
    D_UINT32        ulPagesPerBlock;
    unsigned        nVerbosity;
    D_BUFFER       *pPageBuff;
    D_BUFFER       *pPatternBuff;
} FMSLLOCKTESTINFO;    


static FFXSTATUS DisplayBlockStatus(     FMSLLOCKTESTINFO *pTI);
static FFXSTATUS TestWholeDiskLockUnlock(FMSLLOCKTESTINFO *pTI);
static FFXSTATUS TestUnlockSingleRange(  FMSLLOCKTESTINFO *pTI);
static FFXSTATUS TestUnlockInvertedRange(FMSLLOCKTESTINFO *pTI);
static FFXSTATUS WritePatternAndVerify(  FMSLLOCKTESTINFO *pTI, D_UINT32 ulStartBlock, D_UINT32 ulCount, D_UINT32 ulPageInBlock, D_BOOL fErase);
static FFXSTATUS TryWrites(              FMSLLOCKTESTINFO *pTI, D_UINT32 ulStartBlock, D_UINT32 ulCount, D_UINT32 ulPageInBlock);
static FFXSTATUS VerifyUnlocked(         FMSLLOCKTESTINFO *pTI, D_UINT32 ulStartBlock, D_UINT32 ulCount);
static FFXSTATUS VerifyLocked(           FMSLLOCKTESTINFO *pTI, D_UINT32 ulStartBlock, D_UINT32 ulCount);

 

/*-------------------------------------------------------------------
    Private: FfxFmslLockTest()

    Parameters:
        hFML
        fExtensive

    Return Value:
        Returns an FFXSTATUS value indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxFmslLockTest(
    FFXFMLHANDLE        hFML,
    D_UINT32           *pulRandomSeed,
    unsigned            nVerbosity)
{
    FFXIOSTATUS         ioStat; 
    FFXSTATUS           ffxStat;
    FMSLLOCKTESTINFO    ti = {0};
    unsigned            nn;

    ffxStat = FfxFmlDiskInfo(hFML, &ti.FmlInfo);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    ffxStat = FfxFmlDeviceInfo(ti.FmlInfo.nDeviceNum, &ti.FmlDevInfo);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclPrintf("Block Locking Tests\n");

    if(!(ti.FmlDevInfo.uDeviceFlags & DEV_LOCKABLE))
    {
        DclPrintf("   The device does not support locking.\n");
        return DCLSTAT_UNSUPPORTEDFEATURE;
    }

    ti.hFML             = hFML;
    ti.pulRandomSeed    = pulRandomSeed;
    ti.nVerbosity       = nVerbosity;

    /*  Compute the number of pages in one erase block
    */
    DclAssert((ti.FmlInfo.ulBlockSize % ti.FmlInfo.uPageSize) == 0);
    ti.ulPagesPerBlock = ti.FmlInfo.ulBlockSize / ti.FmlInfo.uPageSize;

    ti.pPageBuff = DclMemAlloc(ti.FmlInfo.uPageSize);
    if(!ti.pPageBuff)
    {
        ffxStat = DCLSTAT_OUTOFMEMORY;
        goto Cleanup;
    }        

    ti.pPatternBuff = DclMemAlloc(ti.FmlInfo.uPageSize);
    if(!ti.pPatternBuff)
    {
        ffxStat = DCLSTAT_OUTOFMEMORY;
        goto Cleanup;
    }        

    for(nn=0; nn<ti.FmlInfo.uPageSize/sizeof(D_UINT32); nn++)
        ((D_UINT32*)ti.pPatternBuff)[nn] = DclRand(ti.pulRandomSeed);

    ffxStat = DisplayBlockStatus(&ti);
    if(ffxStat != FFXSTAT_SUCCESS)
        goto Cleanup;

    ffxStat = TestWholeDiskLockUnlock(&ti);
    if(ffxStat != FFXSTAT_SUCCESS)
        goto CleanupLocks;

    if(ti.FmlDevInfo.nLockFlags & (FFXLOCKFLAGS_UNLOCKSINGLERANGE | FFXLOCKFLAGS_UNLOCKINVERTEDRANGE))
    {
        ffxStat = TestUnlockSingleRange(&ti);
        if(ffxStat != FFXSTAT_SUCCESS)
            goto CleanupLocks;

        if(ti.FmlDevInfo.nLockFlags & FFXLOCKFLAGS_UNLOCKINVERTEDRANGE)
        {
            ffxStat = TestUnlockInvertedRange(&ti);
            if(ffxStat != FFXSTAT_SUCCESS)
                goto CleanupLocks;
        }
    }

  CleanupLocks:

    /*  Unlock all blocks in the Disk -- regardless whether the test passed
        or failed, we don't want to exit the test with any blocks locked.
    */
    FMLUNLOCK_BLOCKS(ti.hFML, 0, ti.FmlInfo.ulTotalBlocks, FALSE, ioStat);

    (void)ioStat;

  Cleanup:

    /*  Release resources in reverse order of allocation
    */
    if(ti.pPatternBuff)
        DclMemFree(ti.pPatternBuff);

    if(ti.pPageBuff)
        DclMemFree(ti.pPageBuff);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: DisplayBlockStatus()

    Parameters:
 
    Return Value:
        Returns an FFXSTATUS value indicating the results.
-------------------------------------------------------------------*/
static FFXSTATUS DisplayBlockStatus(
    FMSLLOCKTESTINFO   *pTI)
{
    D_UINT32            ulBlock;
    D_UINT32            ulStartRange = D_UINT32_MAX;
    D_UINT32            ulLastStatus = 0;
    
    DclAssertReadPtr(pTI, sizeof(*pTI));

    DclPrintf("    Block Status Current State...\n");

    /*  Loop through all the blocks on the Disk, displaying the ranges of
        locked and unlocked blocks.
    */        
    for(ulBlock=0; ulBlock<=pTI->FmlInfo.ulTotalBlocks; ulBlock++)
    {
        FFXIOSTATUS         ioStat = DEFAULT_BLOCKIO_STATUS;
        char                szBuffer[120];
        char               *pszBuffer;
        int                 iRemaining;
        int                 iLen;

        /*  Note above how we are doing one extra pass through the loop
            with ulBlock == ulTotalBlocks.  This is to ensure that the
            data for the last range is displayed properly.  We <don't>
            want to try to read the status for the non-existant block.
        */            
        if(ulBlock < pTI->FmlInfo.ulTotalBlocks)
        {
            FML_GET_BLOCK_STATUS(pTI->hFML, ulBlock, ioStat);
            if(!IOSUCCESS(ioStat, 1))
                return DCLSTAT_CURRENTLINE;

            if(!(ioStat.ulFlags & IOFLAGS_BLOCK))
                return DCLSTAT_CURRENTLINE;

            if(ulStartRange == D_UINT32_MAX)
            {
                DclAssert(ulBlock == 0);
                ulStartRange = 0;
                ulLastStatus = ioStat.op.ulBlockStatus;
                continue;
            }

            if((ioStat.op.ulBlockStatus == ulLastStatus) && (ulBlock < pTI->FmlInfo.ulTotalBlocks))
                continue;
        }
        
        iRemaining = sizeof(szBuffer);
        pszBuffer = &szBuffer[0];
        if(ulBlock == ulStartRange + 1)
            iLen = DclSNPrintf(pszBuffer, iRemaining, "      Block           %5lU - %lX", ulStartRange, ulLastStatus);
        else    
            iLen = DclSNPrintf(pszBuffer, iRemaining, "      Blocks %5lU to %5lU - %lX", ulStartRange, ulBlock-1, ulLastStatus);

        if(iLen == -1 || iLen >= iRemaining)
        {
            DclProductionError();
            return DCLSTAT_BUFFERTOOSMALL;
        }

        iRemaining -= iLen;
        pszBuffer += iLen;

        /*  Don't display anything for the "NotBad" status, at this time...
        */
        if(ulLastStatus & BLOCKSTATUS_NOTBAD)
            ulLastStatus &= ~BLOCKSTATUS_NOTBAD;
        
        if(ulLastStatus & BLOCKSTATUS_DEV_LOCKED)
        {
            iLen = DclSNPrintf(pszBuffer, iRemaining, " DEVLOCKED");

            if(iLen == -1 || iLen >= iRemaining)
            {
                DclProductionError();
                return DCLSTAT_BUFFERTOOSMALL;
            }

            iRemaining -= iLen;
            pszBuffer += iLen;

            ulLastStatus &= ~BLOCKSTATUS_DEV_LOCKED;
        }

        if(ulLastStatus & BLOCKSTATUS_SOFT_LOCKED)
        {
            iLen = DclSNPrintf(pszBuffer, iRemaining, " SOFTLOCKED");

            if(iLen == -1 || iLen >= iRemaining)
            {
                DclProductionError();
                return DCLSTAT_BUFFERTOOSMALL;
            }

            iRemaining -= iLen;
            pszBuffer += iLen;

            ulLastStatus &= ~BLOCKSTATUS_SOFT_LOCKED;
        }

        if(ulLastStatus & BLOCKSTATUS_DEV_LOCKFROZEN)
        {
            iLen = DclSNPrintf(pszBuffer, iRemaining, " LOCKFROZEN");

            if(iLen == -1 || iLen >= iRemaining)
            {
                DclProductionError();
                return DCLSTAT_BUFFERTOOSMALL;
            }

            iRemaining -= iLen;
            pszBuffer += iLen;

            ulLastStatus &= ~BLOCKSTATUS_DEV_LOCKFROZEN;
        }
/*
        if(ulLastStatus & BLOCKSTATUS_BBMREMAPPED)
        {
            iLen = DclSNPrintf(pszBuffer, iRemaining, " BBMREMAPED");

            if(iLen == -1 || iLen >= iRemaining)
            {
                DclProductionError();
                return DCLSTAT_BUFFERTOOSMALL;
            }

            iRemaining -= iLen;
            pszBuffer += iLen;

            ulLastStatus &= ~BLOCKSTATUS_BBMREMAPPED;
        }
*/
        /*  For any bits we did not explicitly handle, deal with them here
        */
        if(ulLastStatus)
        {
            iLen = DclSNPrintf(pszBuffer, iRemaining, " UNHANDLED (%lX)", ulLastStatus);

            if(iLen == -1 || iLen >= iRemaining)
            {
                DclProductionError();
                return DCLSTAT_BUFFERTOOSMALL;
            }
        }

        /*  Display the status for that range of blocks
        */
        DclPrintf("%s\n", szBuffer);

        /*  Reset things to record info for a new range
        */
        ulStartRange = ulBlock;
        ulLastStatus = ioStat.op.ulBlockStatus;
    }    

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: TestWholeDiskLockUnlock()

    Parameters:
 
    Return Value:
        Returns an FFXSTATUS value indicating the results.
-------------------------------------------------------------------*/
static FFXSTATUS TestWholeDiskLockUnlock(
    FMSLLOCKTESTINFO   *pTI)
{
    FFXIOSTATUS         ioStat;
    FFXSTATUS           ffxStat;
    
    DclAssertReadPtr(pTI, sizeof(*pTI));

    DclPrintf("    Test locking/unlocking the whole Disk...\n");
    
    /*  Unlock all blocks in the Disk
    */
    FMLUNLOCK_BLOCKS(pTI->hFML, 0, pTI->FmlInfo.ulTotalBlocks, FALSE, ioStat);
    if(!IOSUCCESS(ioStat, pTI->FmlInfo.ulTotalBlocks))
        return DCLSTAT_CURRENTLINE;

    /*  Erase all the blocks, since we'll be writing to them.  This also
        preps the disk for all the other tests...
    */
    FMLERASE_BLOCKS(pTI->hFML, 0, pTI->FmlInfo.ulTotalBlocks, ioStat);
    if(!IOSUCCESS(ioStat, pTI->FmlInfo.ulTotalBlocks))
        return DCLSTAT_CURRENTLINE;

    /*  Verify that the block status reports they are all unlocked
    */
    ffxStat = VerifyUnlocked(pTI, 0, pTI->FmlInfo.ulTotalBlocks);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    /*  Write a known pattern to the first page of every block
    */
    ffxStat = WritePatternAndVerify(pTI, 0, pTI->FmlInfo.ulTotalBlocks, WHOLE_DISK_PAGE_START, TRUE);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    /*  Lock all the blocks
    */
    FMLLOCK_BLOCKS(pTI->hFML, 0, pTI->FmlInfo.ulTotalBlocks, ioStat);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
    {
        ffxStat = ioStat.ffxStat;
        return ffxStat;
    }        

    /*  REMINDER!  Depending on the nature of the flash locking
                   characteristics, you may not be able to lock
                   ONLY the blocks requested.  This request could
                   result in locking everything.  Therefore we
                   can't check ulCount for an exact match, just
                   one that is at least as large as what was
                   requested.
    */                   
    if(ioStat.ulCount < pTI->FmlInfo.ulTotalBlocks)
        return DCLSTAT_CURRENTLINE;

    /*  Verify that the block status reports they are all locked
    */
    ffxStat = VerifyLocked(pTI, 0, pTI->FmlInfo.ulTotalBlocks);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    /*  Verify that erasing/writing fails -- use the next page in the block
    */
    ffxStat = TryWrites(pTI, 0, pTI->FmlInfo.ulTotalBlocks, WHOLE_DISK_PAGE_START + 1);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    /*  Unlock all blocks in the Disk
    */
    FMLUNLOCK_BLOCKS(pTI->hFML, 0, pTI->FmlInfo.ulTotalBlocks, FALSE, ioStat);
    if(!IOSUCCESS(ioStat, pTI->FmlInfo.ulTotalBlocks))
        return DCLSTAT_CURRENTLINE;

    /*  Verify that the block status reports they are all unlocked
    */
    ffxStat = VerifyUnlocked(pTI, 0, pTI->FmlInfo.ulTotalBlocks);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    /*  Write a known pattern to the 2nd page of every block
    */
    return WritePatternAndVerify(pTI, 0, pTI->FmlInfo.ulTotalBlocks, WHOLE_DISK_PAGE_START + 1, FALSE);
}


/*-------------------------------------------------------------------
    Local: TestUnlockSingleRange()

    Parameters:
 
    Return Value:
        Returns an FFXSTATUS value indicating the results.
-------------------------------------------------------------------*/
static FFXSTATUS TestUnlockSingleRange(
    FMSLLOCKTESTINFO   *pTI)
{
    FFXIOSTATUS         ioStat;
    FFXSTATUS           ffxStat;
    D_UINT32            ulRangeSize;
    D_UINT32            ulStepSize;
    D_UINT32            ulStart;
    D_UINT32            ulPage = SINGLE_RANGE_PAGE_START;
    
    DclAssertReadPtr(pTI, sizeof(*pTI));

    DclPrintf("    Test unlocking a single range...\n");
  
    /*  Pick a range size equal to 25% of the disk size, but not
        less than 3.
    */                
    ulRangeSize = DCLMAX(3, pTI->FmlInfo.ulTotalBlocks >> 2);

    ulStepSize = DCLMAX(1, ulRangeSize - 3);

    for(ulStart = 0; 
        ulStart < pTI->FmlInfo.ulTotalBlocks; 
        ulStart += ulStepSize)
    {
        /*  For the very last iteration of the loop, ensure that the range
            size will not exceed the size of the Disk.
        */            
        if(ulStart + ulRangeSize > pTI->FmlInfo.ulTotalBlocks)
            ulRangeSize = pTI->FmlInfo.ulTotalBlocks - ulStart;

        if(pTI->nVerbosity >= DCL_VERBOSE_NORMAL)
            DclPrintf("      Unlocking the %lU blocks starting with block %lU\n", ulRangeSize, ulStart);
        
        /*  Lock all blocks in the Disk
        */
        FMLLOCK_BLOCKS(pTI->hFML, 0, pTI->FmlInfo.ulTotalBlocks, ioStat);
        if(!IOSUCCESS(ioStat, pTI->FmlInfo.ulTotalBlocks))
            return DCLSTAT_CURRENTLINE;

        /*  Verify that the block status reports they are all locked
        */
        ffxStat = VerifyLocked(pTI, 0, pTI->FmlInfo.ulTotalBlocks);
        if(ffxStat != FFXSTAT_SUCCESS)
            return ffxStat;

        /*  Since we've already run the "WholeDiskLock" test and verified
            that writes/erases will fail, don't bother trying that again
            at this point.
        */            

        /*  Unlock the specified range
        */
        FMLUNLOCK_BLOCKS(pTI->hFML, ulStart, ulRangeSize, FALSE, ioStat);
        if(!IOSUCCESS(ioStat, ulRangeSize))
            return DCLSTAT_CURRENTLINE;

        if(ulStart)
        {
            /*  Verify that the block status reports that all the blocks
                prior to the range are still locked.
            */
            ffxStat = VerifyLocked(pTI, 0, ulStart);
            if(ffxStat != FFXSTAT_SUCCESS)
                return ffxStat;

            /*  Verify that erasing/writing fails
            */
            ffxStat = TryWrites(pTI, 0, ulStart, ulPage);
            if(ffxStat != FFXSTAT_SUCCESS)
                return ffxStat;
        }

        /*  Verify that the block status reports that everything from the
            specified range, to the end of the Disk is unlocked and writeable.
        */
        ffxStat = VerifyUnlocked(pTI, ulStart, pTI->FmlInfo.ulTotalBlocks - ulStart);
        if(ffxStat != FFXSTAT_SUCCESS)
            return ffxStat;

        /*  Write a known pattern to the next page of every block
        */
        ffxStat = WritePatternAndVerify(pTI, ulStart, pTI->FmlInfo.ulTotalBlocks - ulStart, ulPage, FALSE);
        if(ffxStat != FFXSTAT_SUCCESS)
            return ffxStat;

        /*  Bump to the next page.  Due to the way the ranges are set up,
            we'll never write more than two pages to any given block, so
            when that count is reached, just set the page number back to
            its original value.
        */            
        ulPage++;
        if(ulPage == SINGLE_RANGE_PAGE_START + SINGLE_RANGE_PAGE_COUNT)
            ulPage = SINGLE_RANGE_PAGE_START;

        if(ulStart + ulRangeSize == pTI->FmlInfo.ulTotalBlocks)
            break;
    }

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: TestUnlockInvertedRange()

    Parameters:
 
    Return Value:
        Returns an FFXSTATUS value indicating the results.
-------------------------------------------------------------------*/
static FFXSTATUS TestUnlockInvertedRange(
    FMSLLOCKTESTINFO   *pTI)
{
    FFXIOSTATUS         ioStat;
    FFXSTATUS           ffxStat;
    D_UINT32            ulRangeSize;
    D_UINT32            ulStepSize;
    D_UINT32            ulStart;
    D_UINT32            ulPage = INVERTED_RANGE_PAGE_START;
    
    DclAssertReadPtr(pTI, sizeof(*pTI));

    DclPrintf("    Test locking using an inverted unlock range...\n");

    /*  Pick a range size equal to 25% of the disk size, but not
        less than 3.
    */                
    ulRangeSize = DCLMAX(3, pTI->FmlInfo.ulTotalBlocks >> 2);

    ulStepSize = DCLMAX(1, ulRangeSize - 3);

    for(ulStart = 0; 
        ulStart < pTI->FmlInfo.ulTotalBlocks; 
        ulStart += ulStepSize)
    {
        /*  For the very last iteration of the loop, ensure that the range
            size will not exceed the size of the Disk.
        */            
        if(ulStart + ulRangeSize > pTI->FmlInfo.ulTotalBlocks)
            ulRangeSize = pTI->FmlInfo.ulTotalBlocks - ulStart;

        if(pTI->nVerbosity >= DCL_VERBOSE_NORMAL)
            DclPrintf("      Locking the %lU blocks starting with block %lU\n", ulRangeSize, ulStart);
        
        /*  Lock all blocks in the Disk
        */
        FMLUNLOCK_BLOCKS(pTI->hFML, 0, pTI->FmlInfo.ulTotalBlocks, FALSE, ioStat);
        if(!IOSUCCESS(ioStat, pTI->FmlInfo.ulTotalBlocks))
            return DCLSTAT_CURRENTLINE;

        /*  Verify that the block status reports they are all unlocked
        */
        ffxStat = VerifyUnlocked(pTI, 0, pTI->FmlInfo.ulTotalBlocks);
        if(ffxStat != FFXSTAT_SUCCESS)
            return ffxStat;

        /*  Since we've already run the "WholeDiskLock" test and verified
            that writes/erases will fail, don't bother trying that again
            at this point.
        */            

        /*  Lock the specified range using unlock with "invert"
        */
        FMLUNLOCK_BLOCKS(pTI->hFML, ulStart, ulRangeSize, TRUE, ioStat);
        if(!IOSUCCESS(ioStat, ulRangeSize))
            return DCLSTAT_CURRENTLINE;

        if(ulStart)
        {
            /*  Verify that the block status reports that all the blocks
                prior to the range are still unlocked.
            */
            ffxStat = VerifyUnlocked(pTI, 0, ulStart);
            if(ffxStat != FFXSTAT_SUCCESS)
                return ffxStat;

            /*  Verify that erasing/writing works
            */
            ffxStat = WritePatternAndVerify(pTI, 0, ulStart, ulPage, FALSE);
            if(ffxStat != FFXSTAT_SUCCESS)
                return ffxStat;
        }

        /*  Verify that the block status reports that everything from the
            specified range, to the end of the Disk is unlocked and writeable.
        */
        ffxStat = VerifyLocked(pTI, ulStart, ulRangeSize);
        if(ffxStat != FFXSTAT_SUCCESS)
            return ffxStat;

        /*  Write a known pattern to the next page of every block
        */
        ffxStat = TryWrites(pTI, ulStart, ulRangeSize, ulPage);
        if(ffxStat != FFXSTAT_SUCCESS)
            return ffxStat;

        if(ulStart + ulRangeSize < pTI->FmlInfo.ulTotalBlocks)
        {
            /*  Verify that the block status reports that all the blocks
                prior to the range are still unlocked.
            */
            ffxStat = VerifyUnlocked(pTI, ulStart + ulRangeSize, 
                                     pTI->FmlInfo.ulTotalBlocks - (ulStart + ulRangeSize));
            if(ffxStat != FFXSTAT_SUCCESS)
                return ffxStat;

            /*  Verify that erasing/writing works
            */
            ffxStat = WritePatternAndVerify(pTI, ulStart + ulRangeSize, 
                                            pTI->FmlInfo.ulTotalBlocks - (ulStart + ulRangeSize), 
                                            ulPage, FALSE);
            if(ffxStat != FFXSTAT_SUCCESS)
                return ffxStat;
        }

        /*  Bump to the next page.  Due to the way the ranges are set up,
            we'll never write more than two pages to any given block, so
            when that count is reached, just set the page number back to
            its original value.
        */            
        ulPage++;
        if(ulPage == INVERTED_RANGE_PAGE_START + INVERTED_RANGE_PAGE_COUNT)
            ulPage = INVERTED_RANGE_PAGE_START;

        if(ulStart + ulRangeSize == pTI->FmlInfo.ulTotalBlocks)
            break;
    }

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: WritePatternAndVerify()

    Parameters:
 
    Return Value:
        Returns an FFXSTATUS value indicating the results.
-------------------------------------------------------------------*/
static FFXSTATUS WritePatternAndVerify(
    FMSLLOCKTESTINFO   *pTI,
    D_UINT32            ulStartBlock,
    D_UINT32            ulCount,
    D_UINT32            ulPageInBlock,
    D_BOOL              fErase)
{
    D_UINT32            ulBlock;
    FFXIOSTATUS         ioStat;

    DclAssert(ulCount);
    DclAssertReadPtr(pTI, sizeof(*pTI));

    /*  Erase each block, write a known pattern to the first page,
        read it back and verify it.
    */        
    for(ulBlock=ulStartBlock; ulBlock<ulStartBlock + ulCount; ulBlock++)
    {
        D_UINT32    ulAbsolutePage = (ulBlock * pTI->ulPagesPerBlock) + ulPageInBlock;

        if(fErase)
        {
            FMLERASE_BLOCKS(pTI->hFML, ulBlock, 1, ioStat);
            if(!IOSUCCESS(ioStat, 1))
                return DCLSTAT_CURRENTLINE;
        }
        
        if(pTI->nVerbosity > DCL_VERBOSE_LOUD)
            DclPrintf("      Verifying writeability for block %lU, using page %lU\n", ulBlock, ulPageInBlock);
        
        DclSNPrintf((char*)pTI->pPatternBuff, pTI->FmlInfo.uPageSize, "FMSLLOCKTEST-Unlocked-%lX-%lX-", ulBlock, ulPageInBlock);

        FMLWRITE_PAGES(pTI->hFML, ulAbsolutePage, 1, pTI->pPatternBuff, ioStat);
        if(!IOSUCCESS(ioStat, 1))
            return DCLSTAT_CURRENTLINE;

        FMLREAD_PAGES(pTI->hFML, ulAbsolutePage, 1, pTI->pPageBuff, ioStat);
        if(!IOSUCCESS(ioStat, 1))
            return DCLSTAT_CURRENTLINE;

        if(DclMemCmp(pTI->pPageBuff, pTI->pPatternBuff, pTI->FmlInfo.uPageSize) != 0)
            return DCLSTAT_CURRENTLINE;
    }

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: TryWrites()

    Parameters:
 
    Return Value:
        Returns an FFXSTATUS value indicating the results.
-------------------------------------------------------------------*/
static FFXSTATUS TryWrites(
    FMSLLOCKTESTINFO   *pTI,
    D_UINT32            ulStartBlock,
    D_UINT32            ulCount,
    D_UINT32            ulPageInBlock)
{
    D_UINT32            ulBlock;
    FFXIOSTATUS         ioStat;

    DclAssert(ulCount);
    DclAssertReadPtr(pTI, sizeof(*pTI));

    /*  Ensure that erases and writes fail
    */        
    for(ulBlock=ulStartBlock; ulBlock<ulStartBlock + ulCount; ulBlock++)
    {
        D_UINT32    ulAbsolutePage = (ulBlock * pTI->ulPagesPerBlock) + ulPageInBlock;
        
        FMLERASE_BLOCKS(pTI->hFML, ulBlock, 1, ioStat);
        if(ioStat.ulCount != 0)
            return DCLSTAT_CURRENTLINE;
        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
            return DCLSTAT_CURRENTLINE;

        DclSNPrintf((char*)pTI->pPatternBuff, pTI->FmlInfo.uPageSize, "FMSLLOCKTEST-Locked---%lX-%lX-", ulBlock, ulPageInBlock);

        FMLWRITE_PAGES(pTI->hFML, ulAbsolutePage, 1, pTI->pPatternBuff, ioStat);
        if(ioStat.ulCount != 0)
            return DCLSTAT_CURRENTLINE;
        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
            return DCLSTAT_CURRENTLINE;
    }

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: VerifyUnlocked()

    Parameters:
 
    Return Value:
        Returns an FFXSTATUS value indicating the results.
-------------------------------------------------------------------*/
static FFXSTATUS VerifyUnlocked(
    FMSLLOCKTESTINFO   *pTI,
    D_UINT32            ulStartBlock,
    D_UINT32            ulCount)
{
    D_UINT32            ulBlock;
    FFXIOSTATUS         ioStat;

    DclAssert(ulCount);
    DclAssertReadPtr(pTI, sizeof(*pTI));

    for(ulBlock = ulStartBlock; 
        ulBlock < ulStartBlock + ulCount; 
        ulBlock ++)
    {
        FML_GET_BLOCK_STATUS(pTI->hFML, ulBlock, ioStat);
        if(!IOSUCCESS(ioStat, 1))
            return DCLSTAT_CURRENTLINE;

        if(ioStat.op.ulBlockStatus & (BLOCKSTATUS_DEV_LOCKED | BLOCKSTATUS_SOFT_LOCKED))
            return DCLSTAT_CURRENTLINE;
    }

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: VerifyLocked()

    Parameters:
 
    Return Value:
        Returns an FFXSTATUS value indicating the results.
-------------------------------------------------------------------*/
static FFXSTATUS VerifyLocked(
    FMSLLOCKTESTINFO   *pTI,
    D_UINT32            ulStartBlock,
    D_UINT32            ulCount)
{
    D_UINT32            ulBlock;
    FFXIOSTATUS         ioStat;

    DclAssert(ulCount);
    DclAssertReadPtr(pTI, sizeof(*pTI));

    for(ulBlock = ulStartBlock; 
        ulBlock < ulStartBlock + ulCount; 
        ulBlock ++)
    {
        FML_GET_BLOCK_STATUS(pTI->hFML, ulBlock, ioStat);
        if(!IOSUCCESS(ioStat, 1))
            return DCLSTAT_CURRENTLINE;

        if(!((ioStat.op.ulBlockStatus & BLOCKSTATUS_DEV_LOCKED) ||
            ((ioStat.op.ulBlockStatus & BLOCKSTATUS_SOFT_LOCKED))))
        {
            return DCLSTAT_CURRENTLINE;
        }
    }

    return FFXSTAT_SUCCESS;
}


#endif  /* FFXCONF_NANDSUPPORT */


