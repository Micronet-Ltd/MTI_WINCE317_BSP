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

    FMSL tests to stress flash.  The purpose for the tests in this module
    is not so much to pass/fail, per se, as much as it is to test flash to
    its extremes for the purpose of understanding faiure modes.  Typically
    these tests must be run for a LONG time.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmslstress.c $
    Revision 1.12  2010/08/01 18:16:18Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.11  2010/07/31 21:37:16Z  garyp
    Removed the use of some 64-bit macros.
    Revision 1.10  2009/12/31 17:24:42Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.9  2009/08/04 18:58:21Z  garyp
    Minor datatype update.
    Revision 1.8  2009/07/20 20:46:53Z  garyp
    Merged from the v4.0 branch.  Minor datatype changes.  Docs updated.
    Revision 1.7  2009/04/09 02:58:25Z  garyp
    Renamed a structure to avoid AutoDoc naming collisions.
    Revision 1.6  2009/03/31 21:20:42Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.5  2008/01/21 00:27:08Z  garyp
    Updated to accommodate variable length tags.
    Revision 1.4  2008/01/13 07:26:52Z  keithg
    Function header updates to support autodoc.
    Revision 1.3  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2007/11/01 20:15:01Z  billr
    Fix compiler warnings.
    Revision 1.1  2007/10/29 06:32:40Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxfmlapi.h>
#include <fxtools.h>
#include <fxtrace.h>
#include "fmsltst.h"

#define SAMPLE_MINUTES      (1)
#define MAX_ERRORS          (8) /* errors per page which will terminate the test */
#define WIPE_ERASE_COUNT    (3)

typedef struct
{
    FFXFMLHANDLE        hFML;
    FFXFMLINFO          FmlInfo;
    FFXFMLDEVINFO       DevInfo;
    D_UINT32            ulMinutes;
    D_UINT32           *pulRandomSeed;
    D_UINT32            ulPagesPerBlock;
    unsigned            nVerbosity;
    unsigned            fExtensive : 1;
} FMSLSTRESSINFO;

#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    static D_BOOL   StressNOR(FMSLSTRESSINFO *pTI);
#endif
#if FFXCONF_NANDSUPPORT
    static D_BOOL   StressNAND(FMSLSTRESSINFO *pTI);
    static unsigned ReadPageAndVerify(FMSLSTRESSINFO *pTI, D_UINT32 ulPage, D_UINT8 *pbErrCount, D_BUFFER *pPageBuff, D_BUFFER *pTagBuff, const D_BUFFER *pOriginalData);
    static unsigned VerifyData(FMSLSTRESSINFO *pTI, D_UINT32 ulPageNum, const D_BUFFER *pPageBuff, const D_BUFFER *pOriginalData);
#endif


/*-------------------------------------------------------------------
    Protected: FfxFmslStress()

    Parameters:
       hFML
       fExtensive

    Return Value:
       TRUE if the tests passed, FALSE if any failed.
-------------------------------------------------------------------*/
D_BOOL FfxFmslStress(
    FFXFMLHANDLE    hFML,
    D_UINT32        ulTestMinutes,
    D_UINT32       *pulRandomSeed,
    D_BOOL          fExtensive,
    unsigned        nVerbosity)
{
    FMSLSTRESSINFO  ti;

    DclMemSet(&ti, 0, sizeof(ti));

    if(FfxFmlDiskInfo(hFML, &ti.FmlInfo) != FFXSTAT_SUCCESS)
        return FALSE;

    if(FfxFmlDeviceInfo(ti.FmlInfo.nDeviceNum, &ti.DevInfo) != FFXSTAT_SUCCESS)
        return FALSE;

    ti.hFML = hFML;
    ti.ulMinutes = ulTestMinutes;
    ti.fExtensive = fExtensive;
    ti.pulRandomSeed = pulRandomSeed;
    ti.nVerbosity = nVerbosity;

    DclPrintf("Flash Stress Tests\n");

    /*  Compute the number of pages in one erase block
    */
    DclAssert((ti.FmlInfo.ulBlockSize % ti.FmlInfo.uPageSize) == 0);
    ti.ulPagesPerBlock = ti.FmlInfo.ulBlockSize / ti.FmlInfo.uPageSize;

    /*  Validate assumptions about the media and test parameters that
        are vital to the rest of the test code.  Some or all of these
        checks should really be done at the top level of the FMSL tests.
    */
    if (ti.FmlInfo.ulBlockSize == 0)
    {
        DclPrintf("    ERROR: erase zone size is zero\n");
        return FALSE;
    }

    if (!DCLISPOWEROF2(ti.FmlInfo.ulBlockSize))
    {
        DclPrintf("    ERROR: erase zone size is not a power of 2\n");
        return FALSE;
    }

  #if FFXCONF_NANDSUPPORT
    if (ti.FmlInfo.uDeviceType == DEVTYPE_NAND)
    {
        return StressNAND(&ti);
    }
  #endif
  #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    if (ti.FmlInfo.uDeviceType != DEVTYPE_NAND)
    {
        return StressNOR(&ti);
    }
  #endif

  return FALSE;
}


#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT

/*-------------------------------------------------------------------
    Local: StressNOR()

    Parameters:

    Return Value:
 -------------------------------------------------------------------*/
static D_BOOL StressNOR(
    FMSLSTRESSINFO *pTI)
{
    (void)pTI;

    DclPrintf("    Stress tests for NOR are not implemented\n");

    return FALSE;
}

#endif


#if FFXCONF_NANDSUPPORT

/*-------------------------------------------------------------------
    Local: StressNAND()

    Parameters:

    Return Value:
 -------------------------------------------------------------------*/
static D_BOOL StressNAND(
    FMSLSTRESSINFO *pTI)
{
    D_UINT32        ulBlock;
    D_UINT32        ulPageInBlock;
    D_UINT32        ulPageNum;
    D_BUFFER       *pBlockData = NULL;  /* A block sized chunk of memory */
    D_BUFFER       *pTags = NULL;       /* Enough memory for the tags for all pages in a block */
    D_BUFFER       *pPageBuff = NULL;   /* A page sized buffer into which to read */
    D_BUFFER       *pabErrCount = NULL; /* byte array of errors per page */
    D_BUFFER       *pTmp;
    unsigned        jj, kk;
    FFXIOSTATUS     ioStat;
    D_UINT32        ulSample = 0;       /* Number of times the entire block has been verified */
    D_UINT64        ullTotalSeconds;    /* Total seconds the test has run */
    D_UINT64        ullReadCount;       /* Total read operations for the primary page */
    D_UINT64        ullAltReadCount;    /* Total read operations for the other pages */
    DCLTIMESTAMP    ts;
    D_BOOL          fSuccess = FALSE;

    DclPrintf("    NAND read-disturb test\n");

    /*  Pick a random block and page in the block to test
    */
    ulBlock = DclRand(pTI->pulRandomSeed) % pTI->FmlInfo.ulTotalBlocks;
    ulPageInBlock = DclRand(pTI->pulRandomSeed) % pTI->ulPagesPerBlock;
    ulPageNum = (ulBlock * pTI->ulPagesPerBlock) + ulPageInBlock;

    DclPrintf("      Testing DISK block: %lU, Primary page in block: %lU\n", ulBlock, ulPageInBlock);
    if(pTI->ulMinutes)
        DclPrintf("      Test Duration:  %lU minutes\n", pTI->ulMinutes);
    else
        DclPrintf("      Test Duration:  Indefinite (until terminal error count is reached)\n");
    DclPrintf("      Sampling every %u minute(s)\n", SAMPLE_MINUTES);
    DclPrintf("      Terminal Error Counts:\n");
    DclPrintf("        Any error in the primary page (corrected or not)\n");
    DclPrintf("        %u errors in any other page in the block (secondary pages)\n", MAX_ERRORS);

    pBlockData = DclMemAlloc(pTI->FmlInfo.ulBlockSize);
    if(!pBlockData)
        goto Cleanup;

    pPageBuff = DclMemAllocZero(pTI->FmlInfo.uPageSize);
    if(!pPageBuff)
        goto Cleanup;

    /*  Allocate a buffer to store tag data to write.  Currently we don't
        care about the tag values, so just initialize them to zero for now.
    */
    pTags = DclMemAllocZero(pTI->ulPagesPerBlock * pTI->DevInfo.uMetaSize);
    if(!pTags)
        goto Cleanup;

    pabErrCount = DclMemAllocZero(pTI->ulPagesPerBlock);
    if(!pabErrCount)
        goto Cleanup;

    pTmp = pBlockData;
    for(kk=0; kk<pTI->ulPagesPerBlock; kk++)
    {
        for(jj=0; jj<pTI->FmlInfo.uPageSize; jj++)
            *pTmp++ = (D_UINT8)DclRand(pTI->pulRandomSeed);
    }

    /*  Start with an erased block.  Erase it several times to make
        extra certain there are no latent charges in the cells.
    */
    for(kk=0; kk<WIPE_ERASE_COUNT; kk++)
    {
        FMLERASE_BLOCKS(pTI->hFML, ulBlock, 1, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("FMLERASE_BLOCKS() Failed, %s\n", FfxDecodeIOStatus(&ioStat));
            goto Cleanup;
        }
    }

    /*  Write a bunch of random data to the entire block.  The tags are all
        NULL because this test does not care about them.
    */
    FMLWRITE_TAGGEDPAGES(pTI->hFML, ulBlock * pTI->ulPagesPerBlock, pTI->ulPagesPerBlock,
                            pBlockData, pTags, pTI->DevInfo.uMetaSize, ioStat);
    if(!IOSUCCESS(ioStat, pTI->ulPagesPerBlock))
    {
        DclPrintf("FMLWRITE_TAGGEDAGES() Failed, %s\n", FfxDecodeIOStatus(&ioStat));
        goto Cleanup;
    }

    ullTotalSeconds = 0;
    ullReadCount = 0;
    ullAltReadCount = 0;
    
    ts = DclTimeStamp();
    while(TRUE)
    {
        D_UINT32    ulElapsedMS;
        unsigned    nPrimaryErrors;
        unsigned    nSecondaryErrors = 0;

        nPrimaryErrors = ReadPageAndVerify(pTI, ulPageNum, &pabErrCount[ulPageInBlock],
            pPageBuff, pTags, pBlockData+(ulPageInBlock*pTI->FmlInfo.uPageSize));

        if(nPrimaryErrors)
        {
            DclPrintf("WARNING!  Abnormal flash behavior.  The primary page reported errors.\n");
/*            DclPrintf("WARNING!  Abnormal flash behavior.  The primary page should never\n");
            DclPrintf("          develop errors due to the read-disturb effect.\n");
            fSuccess = FALSE;
            goto Cleanup; */
        }

        /*  Increment the primary read counter
        */
        ullReadCount++;

        ulElapsedMS = DclTimePassed(ts);
        if(ulElapsedMS >= SAMPLE_MINUTES * 60 * 1000)
        {
            D_UINT32    ulPage = ulBlock * pTI->ulPagesPerBlock;
            char        szTime[16];

            ulSample++;

            ullTotalSeconds += (ulElapsedMS+500) / 1000;

            DclPrintf("      Sample %lU, Elapsed time %s, %llU/%llU primary/secondary reads performed\n",
                ulSample, DclTimeFormat(ullTotalSeconds, szTime, sizeof(szTime)), 
                VA64BUG(ullReadCount), VA64BUG(ullAltReadCount));

            pTmp = pBlockData;
            for(kk=0; kk<pTI->ulPagesPerBlock; kk++)
            {
                /*  Process each page except the primary page.
                */
                if(kk != ulPageInBlock)
                {
                    /*  Increment the secondary read counter
                    */
                    ullAltReadCount++;

                    nSecondaryErrors = ReadPageAndVerify(pTI, ulPage+kk, &pabErrCount[kk],
                        pPageBuff, pTags, pBlockData+(kk*pTI->FmlInfo.uPageSize));

                    if(nSecondaryErrors > MAX_ERRORS)
                    {
                        DclPrintf("      Terminal error threshold reached -- quitting\n");
                        fSuccess = FALSE;
                        goto Cleanup;
                    }
                }

                pTmp += (kk*pTI->FmlInfo.uPageSize);
            }

            if(pTI->ulMinutes)
            {
                D_UINT64    ullMinutes = ullTotalSeconds / 60;

                if(ullMinutes > pTI->ulMinutes-1)
                {
                    DclPrintf("      Test time period expired (%lU minutes) -- quitting\n", pTI->ulMinutes);
                    fSuccess = TRUE;
                    break;
                }
            }

            /*  nPrimary errors will <normally> always be zero here (due to
                error handling code above).
            */
            if(nPrimaryErrors + nSecondaryErrors > 0)
                DclPrintf("        Block contains %u/%u primary/secondary errors (corrected or not)\n",
                    nPrimaryErrors, nSecondaryErrors);

            /*  Reset the timestamp base to start counting up until the
                next sample.
            */
            ts = DclTimeStamp();
        }
    }


  Cleanup:
    if(pabErrCount)
        DclMemFree(pabErrCount);
    if(pTags)
        DclMemFree(pTags);
    if(pPageBuff)
        DclMemFree(pPageBuff);
    if(pBlockData)
        DclMemFree(pBlockData);

    return fSuccess;
}


/*-------------------------------------------------------------------
    Local: ReadPageAndVerify()

    Parameters:

    Return Value:
        Returns a count of the number of data errors in the page.
-------------------------------------------------------------------*/
static unsigned ReadPageAndVerify(
    FMSLSTRESSINFO *pTI,
    D_UINT32        ulPage,
    D_UINT8        *pbErrCount,
    D_BUFFER       *pPageBuff,
    D_BUFFER       *pTagBuff,
    const D_BUFFER *pOriginalData)
{
    unsigned        nErrors;
    FFXIOSTATUS     ioStat;

    FMLREAD_TAGGEDPAGES(pTI->hFML, ulPage, 1, pPageBuff, pTagBuff, pTI->DevInfo.uMetaSize, ioStat);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS &&
        ioStat.ulCount == 1 &&
        ioStat.op.ulPageStatus == 0 &&
        *pbErrCount == 0)
    {
        /*  If everything possible for this page was reported as
            completely normal, <AND>, there were not previous errors
            reported for this page, return 0.
        */
        return 0;
    }

    nErrors = VerifyData(pTI, ulPage, pPageBuff, pOriginalData);

    /*  If there was a corrected data error, increase the total error count
    */
    if(ioStat.op.ulPageStatus & PAGESTATUS_DATACORRECTED)
        nErrors++;

    if(*pbErrCount < nErrors)
    {
        DclPrintf("        Page %lU absolute error count increased from %u to %u, %s\n",
            ulPage, *pbErrCount, nErrors, FfxDecodeIOStatus(&ioStat));
    }
    else if(*pbErrCount > nErrors)
    {
        DclPrintf("        Page %lU absolute error count DECREASED from %u to %u, %s\n",
            ulPage, *pbErrCount, nErrors, FfxDecodeIOStatus(&ioStat));
    }

    *pbErrCount = (D_UINT8)DCLMIN(nErrors, D_UINT8_MAX);

    return nErrors;
}


/*-------------------------------------------------------------------
    Local: VerifyData()

    Parameters:

    Return Value:
        Returns a count of the number of data errors in the page.
-------------------------------------------------------------------*/
static unsigned VerifyData(
    FMSLSTRESSINFO *pTI,
    D_UINT32        ulPage,
    const D_BUFFER *pPageBuff,
    const D_BUFFER *pOriginalData)
{
    unsigned        jj;
    unsigned        nErrors = 0;

    for(jj=0; jj<pTI->FmlInfo.uPageSize; jj++)
    {
        if(pPageBuff[jj] != pOriginalData[jj])
        {
            unsigned    nBitsDifferent;

            nBitsDifferent = DclBitCount(pPageBuff[jj] ^ pOriginalData[jj]);

            DclPrintf("        Page %lU offset %u, read %02x expected %02x, %u bit(s) changed\n",
                ulPage, jj, pPageBuff[jj], pOriginalData[jj], nBitsDifferent);

            nErrors += nBitsDifferent;
        }
    }

    return nErrors;
}


#endif  /* FFXCONF_NANDSUPPORT */


