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

 ---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmslbbm.c $
    Revision 1.5  2010/07/12 18:24:42Z  garyp
    Commented out some unused prototypes.
    Revision 1.4  2010/07/09 22:32:47Z  garyp
    Updated to be more tolerant of some expected errors.  Remove the use
    of __FUNCTION__.
    Revision 1.3  2010/07/08 03:25:30Z  garyp
    The main test now validates that error injection is enabled.
    Revision 1.2  2010/07/07 20:01:29Z  garyp
    Fixed to ignore the tests if error injection is not enabled.
    Revision 1.1  2010/07/07 18:53:38Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NANDSUPPORT && FFXCONF_BBMSUPPORT

#include <fxfmlapi.h>
#include <fxtools.h>
#include <fxtrace.h>
#include "fmsltst.h"
#include "fmslnand.h"

#define BBMTESTNAME "FmslTestBBM"

/*  BBM test data and definitions
*/
typedef struct
{
    unsigned            nTests;
    D_UINT32            ulStartBlock;
    unsigned            nBlocks;
    unsigned            nMinSetBitsOnErasedPage;
    DCLALIGNEDBUFFER    (tag, data, FFX_NAND_TAGSIZE);
} BBMTESTINFO;
   
static  FFXSTATUS TestSetup(                FMSLNANDTESTINFO *pTI, BBMTESTINFO *pBTI);
static  FFXSTATUS TestMain(                 FMSLNANDTESTINFO *pTI, BBMTESTINFO *pBTI);
static  FFXSTATUS TestCleanup(              FMSLNANDTESTINFO *pTI, BBMTESTINFO *pBTI);
static  FFXSTATUS ValidateAfterBBMRemapping(FMSLNANDTESTINFO *pTI, BBMTESTINFO *pBTI, D_UINT32 ulStartPage, unsigned nTest);
static  FFXSTATUS FMLxxx_PAGES(             FMSLNANDTESTINFO *pTI, BBMTESTINFO *pBTI, D_UINT32 ulPage, D_BOOL fWrite);
static  FFXSTATUS FMLxxx_TAGS(              FMSLNANDTESTINFO *pTI, BBMTESTINFO *pBTI, D_UINT32 ulPage, D_BOOL fWrite);
static  FFXSTATUS FMLxxx_TAGGEDPAGES(       FMSLNANDTESTINFO *pTI, BBMTESTINFO *pBTI, D_UINT32 ulPage, D_BOOL fWrite);
static  FFXSTATUS FMLxxx_EXPEND_PAGE(       FMSLNANDTESTINFO *pTI, BBMTESTINFO *pBTI, D_UINT32 ulPage, D_BOOL fWrite);
/*
static  FFXSTATUS FMLxxx_SPARES(            FMSLNANDTESTINFO *pTI, BBMTESTINFO *pBTI, D_UINT32 ulPage, D_BOOL fWrite);
static  FFXSTATUS FMLxxx_NATIVEPAGES(       FMSLNANDTESTINFO *pTI, BBMTESTINFO *pBTI, D_UINT32 ulPage, D_BOOL fWrite);
static  FFXSTATUS FMLxxx_UNCORRECTEDPAGES(  FMSLNANDTESTINFO *pTI, BBMTESTINFO *pBTI, D_UINT32 ulPage, D_BOOL fWrite);
*/
typedef FFXSTATUS (FNBBMTEST)              (FMSLNANDTESTINFO *pTI, BBMTESTINFO *pBTI, D_UINT32 ulPage, D_BOOL fWrite);

typedef struct
{
    char           *pszName;
    FNBBMTEST      *pfnTest;
} FMSLBBMTESTINFO;

static FMSLBBMTESTINFO aBBMTests[] =
{
    {"FMLWRITE_PAGES",            FMLxxx_PAGES},
    {"FMLWRITE_TAGS",             FMLxxx_TAGS},
    {"FMLWRITE_TAGGEDPAGES",      FMLxxx_TAGGEDPAGES},
    {"FMLEXPEND_PAGE",            FMLxxx_EXPEND_PAGE}
/*
    {"FMLWRITE_SPARES",           FMLxxx_SPARES},
    {"FMLWRITE_NATIVEPAGES",      FMLxxx_NATIVEPAGES},
    {"FMLWRITE_UNCORRECTEDPAGES", FMLxxx_UNCORRECTEDPAGES},
    {"FMLBLOCK_ERASE",            FMLBLOCK_ERASE} */
};

 
/*-------------------------------------------------------------------
    Private: FfxFmslNANDTestBBM()

    Parameters:
        pTI - The test parameters structure

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
--------------------------------------------------------------------*/
D_BOOL FfxFmslNANDTestBBM(
    FMSLNANDTESTINFO   *pTI)
{
    FFXSTATUS           ffxStat;
    BBMTESTINFO         BbmTestInfo = {0};

    DclAssert(pTI);

    DclPrintf("    Bad Block Manager Tests...\n");

    ffxStat = TestSetup(pTI, &BbmTestInfo);
    if(ffxStat == FFXSTAT_UNSUPPORTEDFEATURE)
    {
        return TRUE;
    }
    else if(ffxStat == FFXSTAT_SUCCESS)
    {
        FFXSTATUS   ffxStat2;
        
        ffxStat = TestMain(pTI, &BbmTestInfo);

        /*  Make sure to report clean up failure, but not to turn
            a test failure into a pass.
        */
        ffxStat2 = TestCleanup(pTI, &BbmTestInfo);
        if(ffxStat == FFXSTAT_SUCCESS)
            ffxStat = ffxStat2;
    }
    
    if(ffxStat == FFXSTAT_SUCCESS)
        return TRUE;
    else
        return FALSE;
}


/*-------------------------------------------------------------------
    Local: TestSetup()

    Sets up the initial test conditions.  This includes finding
    suitable (good, raw) blocks to use for the test.  It also
    allocates test memory and creates the control data for the
    main and spare areas of a page.

    Parameters:
        pTI     - The test parameters structure
        pBTI    - ECC tests structure

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static FFXSTATUS TestSetup(
    FMSLNANDTESTINFO   *pTI,
    BBMTESTINFO        *pBTI)
{
    FFXIOSTATUS         ioStat;
  
    DclAssert(pTI);
    DclAssert(pBTI);

    if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("      Creating test control data...\n");

    pBTI->nTests = DCLDIMENSIONOF(aBBMTests);

    if(pTI->ulPagesPerBlock < pBTI->nTests + 1)
    {
        DclPrintf("      Not enough pages per block to run this test\n");
        return DCLSTAT_CURRENTLINE;
    }

    /*  Calculate the minimum number of bits which must be set on an erased
        page, taking into account the ECC capabilities of the flash and the
        segment size.  (Ultimately this really should be examined on a per-
        segment basis.)  This is for the main page area only.
    */        
    pBTI->nMinSetBitsOnErasedPage = pTI->FmlInfo.uPageSize * CHAR_BIT;
    pBTI->nMinSetBitsOnErasedPage -= (pTI->FmlInfo.uPageSize / pTI->FmlDevInfo.uEdcSegmentSize) * pTI->FmlDevInfo.uEdcCapability;

    pBTI->nBlocks = 2;
    
    /*  Find test blocks
    */
    if(!FfxFmslNANDFindTestBlocks(pTI, pBTI->nBlocks, &pBTI->ulStartBlock, TRUE))
    {
        DclPrintf("      Failed to find %u consecutive erase block(s)\n", pBTI->nTests);
        return DCLSTAT_CURRENTLINE;
    }
    
    /*  Erase the blocks
    */
    FMLERASE_BLOCKS(pTI->hFML, pBTI->ulStartBlock, pBTI->nBlocks, ioStat);
    if(!IOSUCCESS(ioStat, pBTI->nBlocks))
    {
        DclPrintf("        Failed to erase %u blocks starting at block %lU, status %s\n", 
            pBTI->nBlocks, pBTI->ulStartBlock, FfxDecodeIOStatus(&ioStat));

        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
            ioStat.ffxStat = DCLSTAT_CURRENTLINE;
    }
        
    return ioStat.ffxStat;
}


/*-------------------------------------------------------------------
    Local: TestMain()

    Parameters:
        pTI          - The test parameters structure
        pBTI - ECC tests structure

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static FFXSTATUS TestMain(
    FMSLNANDTESTINFO   *pTI,
    BBMTESTINFO        *pBTI)
{
    FFXSTATUS           ffxStat;
    D_UINT32            ulCurrentBlock;
    D_UINT32            ulStartPage;
    unsigned            nPass;
    const unsigned      nErrorCount = 1;

    DclAssert(pTI);
    DclAssert(pBTI);
    DclAssert(pTI->ulBufferSize >= pTI->FmlInfo.uPageSize);

    ulCurrentBlock = pBTI->ulStartBlock;
    ulStartPage = pBTI->ulStartBlock * pTI->ulPagesPerBlock;

    for(nPass = 0; nPass < pBTI->nTests; nPass++)
    {
        FFXIOSTATUS ioStat;
        D_UINT32    ulPage = ulStartPage;
        unsigned    tt = nPass;
        D_BOOL      fInjected = FALSE;

        DclPrintf("      Starting Pass %u of %u\n", nPass+1, pBTI->nTests);

        /*  For each relative erase block index (nPass), we will start
            the various tests (tt) based on that index, wrapping back
            to the beginning.  This ensures that each iteration has all
            the various I/O types on the media, and that each I/O type
            is uniquely tested for interruptions.

            ToDo: Support I/O operations with counts greater than one,
                  and iterate through the various interruption points
                  up to that value by varying "nErrorCount".
        */        
        while(TRUE)
        {
            if((tt == nPass-1) || ((tt == pBTI->nTests-1) && (nPass == 0)))
            {
                /*  Once we get to this point, we're about to do the I/O for
                    the final type in this round-robin scheme.  Instruct the
                    FIM/NTM to fail the next write I/O with an I/O error.
                */                    
                ffxStat = FfxFmlParameterSet(pTI->hFML, 
                                             FFXPARAM_FIM_ERRINJECT_WRITEIO, 
                                             &nErrorCount, 
                                             sizeof(nErrorCount));
                if(ffxStat != FFXSTAT_SUCCESS)
                {
                    DclPrintf("FfxFmlParameterSet() failed with status %lX\n", ffxStat);
                    return ffxStat;
                }

                if(pTI->nVerbosity > DCL_VERBOSE_LOUD)
                    DclPrintf("          Injecting error in next write...\n");

                fInjected = TRUE;
            }

            /*  Do the I/O
            */
            ffxStat = aBBMTests[tt].pfnTest(pTI, pBTI, ulPage, TRUE);
            if(ffxStat != FFXSTAT_SUCCESS)
            {
                DclPrintf("Test failed with status %lX\n", ffxStat);
                return ffxStat;
            }

            if(fInjected)
            {
                FFXIOR_FML_GET_BLOCK_INFO   inf;
                
                FML_GET_BLOCK_INFO(pTI->hFML, ulCurrentBlock, ioStat, inf);
                if(!IOSUCCESS(ioStat, 1))  
                {
                    DclPrintf("FML_GET_BLOCK_INFO() failed with status %s\n", FfxDecodeIOStatus(&ioStat));

                    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                        ioStat.ffxStat = DCLSTAT_CURRENTLINE;
                    
                    return ioStat.ffxStat; 
                }

                if(inf.ulBlockInfo != BLOCKINFO_RETIRED)
                {
                    DclPrintf("FML_GET_BLOCK_INFO() returned %lX %lX %lX %lX\n", inf.ulBlock, inf.ulBlockInfo, inf.ulBlockStatus, inf.ulRawMapping);
                    return DCLSTAT_CURRENTLINE;
                }            
                
                /*  We just got done injecting an error during I/O.  All the
                    previous data, and the data for the interrupted operation
                    should now be on the media in the new block.
                */                    
                ffxStat = ValidateAfterBBMRemapping(pTI, pBTI, ulStartPage, nPass);
                if(ffxStat != FFXSTAT_SUCCESS)
                {
                    DclPrintf("ValidateAfterBBMRemapping() failed with status %lX\n", ffxStat);
                    return ffxStat;
                }
/*
                if(pTI->nVerbosity > DCL_VERBOSE_LOUD)
                    DclPrintf("        Validated\n");
*/
                /*  Once we've gotten to this point, this iteration of the
                    test is complete, so break out of the while{} loop.
                */                    
                break;
            }

            /*  Next page
            */
            ulPage++;

            /*  On to the next test, wrapping back to the beginning
                when necessary.
            */                
            tt++;
            if(tt == pBTI->nTests)
                tt = 0;
        }

        /*  On to the next erase block.  For every two blocks, erase the
            range and reset to the first block.  This forces remapping to
            happen on already remapped blocks.
        */
        if(nPass & 1)
        {
            FMLERASE_BLOCKS(pTI->hFML, pBTI->ulStartBlock, pBTI->nBlocks, ioStat);
            if(!IOSUCCESS(ioStat, pBTI->nBlocks))
            {
                DclPrintf("        Failed to erase %u blocks starting at block %lU, status %s\n", 
                    pBTI->nBlocks, pBTI->ulStartBlock, FfxDecodeIOStatus(&ioStat));
    
                if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                    ioStat.ffxStat = DCLSTAT_CURRENTLINE;
            }

            ulCurrentBlock = pBTI->ulStartBlock;
            ulStartPage = pBTI->ulStartBlock * pTI->ulPagesPerBlock;
        }
        else
        {
            ulStartPage += pTI->ulPagesPerBlock;
            ulCurrentBlock++;
        }
    }

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: TestCleanup()

    Cleans up after the ECC tests.  This includes erasing the
    test blocks and free any allocated resources.

    Parameters:
        pTI     - The test parameters structure
        pBTI    - ECC tests structure

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static FFXSTATUS TestCleanup(
    FMSLNANDTESTINFO   *pTI,
    BBMTESTINFO        *pBTI)
{
    FFXIOSTATUS         ioStat;

    DclAssert(pTI);
    DclAssert(pBTI);

    if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("      Cleaning up...\n");

    /*  Erase the blocks we used for this test.
    */
    FMLERASE_BLOCKS(pTI->hFML, pBTI->ulStartBlock, pBTI->nTests, ioStat);
    if(!IOSUCCESS(ioStat, pBTI->nTests))
    {
        DclPrintf("      Erase failed with status %s\n", FfxDecodeIOStatus(&ioStat));

        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
            ioStat.ffxStat = DCLSTAT_CURRENTLINE;
    }

    return ioStat.ffxStat;
}


/*-------------------------------------------------------------------
    Local: ValidateAfterBBMRemapping()

    Parameters:
        pTI     - The test parameters structure
        pBTI    - ECC tests structure

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static FFXSTATUS ValidateAfterBBMRemapping(
    FMSLNANDTESTINFO   *pTI,
    BBMTESTINFO        *pBTI,
    D_UINT32            ulStartPage,
    unsigned            nTest)
{
    FFXSTATUS           ffxStat;
    D_UINT32            ulPage = ulStartPage;
    unsigned            tt = nTest;

    DclAssert(pTI);
    DclAssert(pBTI);

    do
    {
        /*  Do the validation
        */
        ffxStat = aBBMTests[tt].pfnTest(pTI, pBTI, ulPage, FALSE);
        if(ffxStat != FFXSTAT_SUCCESS)
        {
            DclPrintf("Test failed with status %lX\n", ffxStat);
            return ffxStat;
        }

        /*  Next page
        */
        ulPage++;

        /*  On to the next test, wrapping back to the beginning
            when necessary.
        */                
        tt++;
        if(tt == pBTI->nTests)
            tt = 0;
    }
    while(tt != nTest);

    return FFXSTAT_SUCCESS  /* ValidateErasedPage(pTI, pBTI, ulPage) */;
}


/*-------------------------------------------------------------------
    Local: FMLxxx_PAGES()

    Parameters:
        pTI     - The test parameters structure
        pBTI    - ECC tests structure

    Return Value:
        Returns an FFXIOSTATUS structure containing the result.
-------------------------------------------------------------------*/
static FFXSTATUS FMLxxx_PAGES(
    FMSLNANDTESTINFO   *pTI, 
    BBMTESTINFO        *pBTI, 
    D_UINT32            ulPage, 
    D_BOOL              fWrite)
{
    FFXIOSTATUS         ioStat;
    
    if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("        FMLxxx_PAGES() -- %s...\n", fWrite ? "Writing" : "Verifying");

    if(fWrite)
    {
        FfxFmslNANDPageFill(pTI, pTI->pBuffer, 1, 1, ulPage, BBMTESTNAME);

        FMLWRITE_PAGES(pTI->hFML, ulPage, 1, pTI->pBuffer, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("FMLxxx_PAGES() FMLWRITE_PAGES() page %lU failed with status %s\n",
                ulPage, FfxDecodeIOStatus(&ioStat));

            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                ioStat.ffxStat = DCLSTAT_CURRENTLINE;
        }
    }
    else
    {
        D_BUFFER    abTag[FFX_NAND_TAGSIZE] = {0xFF, 0xFF};

        if(pTI->nVerbosity > DCL_VERBOSE_LOUD)
            DclPrintf("          Verifying FMLREAD_TAGS()\n");
            
        FMLREAD_TAGS(pTI->hFML, ulPage, 1, pTI->pSpareArea, FFX_NAND_TAGSIZE, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("FMLxxx_PAGES() FMLREAD_TAGS() page %lU failed with status %s\n",
                ulPage, FfxDecodeIOStatus(&ioStat));

            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                ioStat.ffxStat = DCLSTAT_CURRENTLINE;

            return ioStat.ffxStat;
        }

        if(DclMemCmp(abTag, pTI->pSpareArea, FFX_NAND_TAGSIZE))
        {
            DclPrintf("FMLxxx_PAGES() FMLREAD_TAGS() returned a non-0xFF tag when all 0xFFs was expected\n",
                ulPage);

            return DCLSTAT_CURRENTLINE;
        }

        if(pTI->nVerbosity > DCL_VERBOSE_LOUD)
            DclPrintf("          Verifying FMLREAD_TAGGEDPAGES()\n");
            
        FMLREAD_TAGGEDPAGES(pTI->hFML, ulPage, 1, pTI->pBuffer2, pTI->pSpareArea, FFX_NAND_TAGSIZE, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("FMLxxx_PAGES() FMLREAD_TAGGEDPAGES() page %lU failed with status %s\n",
                ulPage, FfxDecodeIOStatus(&ioStat));

            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                ioStat.ffxStat = DCLSTAT_CURRENTLINE;

            return ioStat.ffxStat;
        }

        if(DclMemCmp(abTag, pTI->pSpareArea, FFX_NAND_TAGSIZE))
        {
            DclPrintf("FMLxxx_PAGES() FMLREAD_TAGGEDPAGES() returned a non-zero tag when zero was expected\n",
                ulPage);

            return DCLSTAT_CURRENTLINE;
        }
        
        if(!FfxFmslNANDPageVerify(pTI, pTI->pBuffer2, 1, 1, ulPage, BBMTESTNAME, 0, 1, WITH_ECC))
        {
            DclPrintf("FMLxxx_PAGES() FMLREAD_TAGGEDPAGES() returned a mismatched data buffer\n",
                ulPage);

            return DCLSTAT_CURRENTLINE;
        }

        if(pTI->nVerbosity > DCL_VERBOSE_LOUD)
            DclPrintf("          Verifying FMLREAD_PAGES()\n");
            
        FMLREAD_PAGES(pTI->hFML, ulPage, 1, pTI->pBuffer2, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("FMLxxx_PAGES() FMLREAD_PAGES() page %lU failed with status %s\n",
                ulPage, FfxDecodeIOStatus(&ioStat));

            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                ioStat.ffxStat = DCLSTAT_CURRENTLINE;

            return ioStat.ffxStat;
        }

        if(!FfxFmslNANDPageVerify(pTI, pTI->pBuffer2, 1, 1, ulPage, BBMTESTNAME, 0, 1, WITH_ECC))
        {
            DclPrintf("FMLxxx_PAGES() FMLREAD_TAGGEDPAGES() returned a mismatched data buffer\n",
                ulPage);

            return DCLSTAT_CURRENTLINE;
        }
    }

    return ioStat.ffxStat;
}


/*-------------------------------------------------------------------
    Local: FMLxxx_TAGS()

    Parameters:
        pTI     - The test parameters structure
        pBTI    - ECC tests structure

    Return Value:
        Returns an FFXIOSTATUS structure containing the result.
-------------------------------------------------------------------*/
static FFXSTATUS FMLxxx_TAGS(
    FMSLNANDTESTINFO   *pTI, 
    BBMTESTINFO        *pBTI, 
    D_UINT32            ulPage, 
    D_BOOL              fWrite)
{
    FFXIOSTATUS         ioStat;
    D_BUFFER            abTag[FFX_NAND_TAGSIZE] = {0x56, 0x78};
    
    if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("        FMLxxx_TAGS() -- %s...\n", fWrite ? "Writing" : "Verifying");

    if(fWrite)
    {
        FMLWRITE_TAGS(pTI->hFML, ulPage, 1, abTag, sizeof(abTag), ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("FMLxxx_TAGS() FMLWRITE_TAGS() page %lU failed with status %s\n",
                ulPage, FfxDecodeIOStatus(&ioStat));

            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                ioStat.ffxStat = DCLSTAT_CURRENTLINE;
        }
    }
    else
    {
        unsigned    nBits;

        if(pTI->nVerbosity > DCL_VERBOSE_LOUD)
            DclPrintf("          Verifying FMLREAD_TAGS()\n");
            
        FMLREAD_TAGS(pTI->hFML, ulPage, 1, pTI->pSpareArea, FFX_NAND_TAGSIZE, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("FMLxxx_TAGS() FMLREAD_TAGS() page %lU failed with status %s\n",
                ulPage, FfxDecodeIOStatus(&ioStat));

            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                ioStat.ffxStat = DCLSTAT_CURRENTLINE;

            return ioStat.ffxStat;
        }

        if(DclMemCmp(abTag, pTI->pSpareArea, FFX_NAND_TAGSIZE))
        {
            DclPrintf("FMLxxx_TAGS() FMLREAD_TAGS() returned invalid tag data\n");

            return DCLSTAT_CURRENTLINE;
        }

        if(pTI->nVerbosity > DCL_VERBOSE_LOUD)
            DclPrintf("          Verifying FMLREAD_TAGGEDPAGES()\n");
            
        FMLREAD_TAGGEDPAGES(pTI->hFML, ulPage, 1, pTI->pBuffer2, pTI->pSpareArea, FFX_NAND_TAGSIZE, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("FMLxxx_TAGS() FMLREAD_TAGGEDPAGES() page %lU failed with status %s\n",
                ulPage, FfxDecodeIOStatus(&ioStat));

            if(ioStat.ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA)
            {
                /*  Since we tried to read page data from an area which 
                    was never programmed, it may legitimately report an
                    uncorrectable error, but that is not a fatal error
                    in this case.
                */  
                ioStat.ffxStat = FFXSTAT_SUCCESS;
            }
            else
            {
                if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                    ioStat.ffxStat = DCLSTAT_CURRENTLINE;

                return ioStat.ffxStat;
            }
        }

        if(DclMemCmp(abTag, pTI->pSpareArea, FFX_NAND_TAGSIZE))
        {
            DclPrintf("FMLxxx_TAGS() FMLREAD_TAGGEDPAGES() returned invalid tag data\n");

            return DCLSTAT_CURRENTLINE;
        }

        nBits = DclBitCountArray(pTI->pBuffer2, pTI->FmlInfo.uPageSize * CHAR_BIT);
        if(nBits < pBTI->nMinSetBitsOnErasedPage)
        {
            DclPrintf("FMLxxx_TAGS() FMLREAD_TAGGEDPAGES() returned successfully, but the page was not erased\n");

            return DCLSTAT_CURRENTLINE;
        }
    }

    return ioStat.ffxStat;
}


/*-------------------------------------------------------------------
    Local: FMLxxx_TAGGEDPAGES()

    Parameters:
        pTI     - The test parameters structure
        pBTI    - ECC tests structure

    Return Value:
        Returns an FFXIOSTATUS structure containing the result.
-------------------------------------------------------------------*/
static FFXSTATUS FMLxxx_TAGGEDPAGES(
    FMSLNANDTESTINFO   *pTI, 
    BBMTESTINFO        *pBTI, 
    D_UINT32            ulPage, 
    D_BOOL              fWrite)
{
    FFXIOSTATUS         ioStat;
    D_BUFFER            abTag[FFX_NAND_TAGSIZE] = {0x12, 0x34};
    
    if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("        FMLxxx_TAGGEDPAGES() -- %s...\n", fWrite ? "Writing" : "Verifying");

    if(fWrite)
    {
        FfxFmslNANDPageFill(pTI, pTI->pBuffer, 1, 1, ulPage, BBMTESTNAME);

        FMLWRITE_TAGGEDPAGES(pTI->hFML, ulPage, 1, pTI->pBuffer, abTag, sizeof(abTag), ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("FMLxxx_TAGGEDPAGES() FMLWRITE_TAGGEDPAGES() page %lU failed with status %s\n",
                ulPage, FfxDecodeIOStatus(&ioStat));

            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                ioStat.ffxStat = DCLSTAT_CURRENTLINE;
        }
    }
    else
    {
        if(pTI->nVerbosity > DCL_VERBOSE_LOUD)
            DclPrintf("          Verifying FMLREAD_TAGS()\n");
            
        FMLREAD_TAGS(pTI->hFML, ulPage, 1, pTI->pSpareArea, FFX_NAND_TAGSIZE, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("FMLxxx_TAGGEDPAGES() FMLREAD_TAGS() page %lU failed with status %s\n",
                ulPage, FfxDecodeIOStatus(&ioStat));

            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                ioStat.ffxStat = DCLSTAT_CURRENTLINE;

            return ioStat.ffxStat;
        }

        if(DclMemCmp(abTag, pTI->pSpareArea, FFX_NAND_TAGSIZE))
        {
            DclPrintf("FMLxxx_TAGGEDPAGES() FMLREAD_TAGS() returned invaid tag data\n");

            return DCLSTAT_CURRENTLINE;
        }

        if(pTI->nVerbosity > DCL_VERBOSE_LOUD)
            DclPrintf("          Verifying FMLREAD_TAGGEDPAGES()\n");
            
        FMLREAD_TAGGEDPAGES(pTI->hFML, ulPage, 1, pTI->pBuffer2, pTI->pSpareArea, FFX_NAND_TAGSIZE, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("FMLxxx_TAGGEDPAGES() FMLREAD_TAGGEDPAGES() page %lU failed with status %s\n",
                ulPage, FfxDecodeIOStatus(&ioStat));

            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                ioStat.ffxStat = DCLSTAT_CURRENTLINE;

            return ioStat.ffxStat;
        }

        if(DclMemCmp(abTag, pTI->pSpareArea, FFX_NAND_TAGSIZE))
        {
            DclPrintf("FMLxxx_TAGGEDPAGES() FMLREAD_TAGS() returned invaid tag data\n");

            return DCLSTAT_CURRENTLINE;
        }
        
        if(!FfxFmslNANDPageVerify(pTI, pTI->pBuffer2, 1, 1, ulPage, BBMTESTNAME, 0, 1, WITH_ECC))
        {
            DclPrintf("FMLxxx_TAGGEDPAGES() FMLREAD_TAGGEDPAGES() returned a mismatched data buffer\n",
                ulPage);

            return DCLSTAT_CURRENTLINE;
        }

        if(pTI->nVerbosity > DCL_VERBOSE_LOUD)
            DclPrintf("          Verifying FMLREAD_PAGES()\n");
            
        FMLREAD_PAGES(pTI->hFML, ulPage, 1, pTI->pBuffer2, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("FMLxxx_TAGGEDPAGES() FMLREAD_PAGES() page %lU failed with status %s\n",
                ulPage, FfxDecodeIOStatus(&ioStat));

            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                ioStat.ffxStat = DCLSTAT_CURRENTLINE;

            return ioStat.ffxStat;
        }

        if(!FfxFmslNANDPageVerify(pTI, pTI->pBuffer2, 1, 1, ulPage, BBMTESTNAME, 0, 1, WITH_ECC))
        {
            DclPrintf("FMLxxx_TAGGEDPAGES() FMLREAD_TAGGEDPAGES() returned a mismatched data buffer\n",
                ulPage);

            return DCLSTAT_CURRENTLINE;
        }
    }

    return ioStat.ffxStat;
}


/*-------------------------------------------------------------------
    Local: FMLxxx_EXPEND_PAGE()

    Parameters:
        pTI     - The test parameters structure
        pBTI    - ECC tests structure

    Return Value:
        Returns an FFXIOSTATUS structure containing the result.
-------------------------------------------------------------------*/
static FFXSTATUS FMLxxx_EXPEND_PAGE(
    FMSLNANDTESTINFO   *pTI, 
    BBMTESTINFO        *pBTI, 
    D_UINT32            ulPage, 
    D_BOOL              fWrite)
{
    FFXIOSTATUS         ioStat;
    
    if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("        FMLxxx_EXPEND_PAGE() -- %s...\n", fWrite ? "Writing" : "Verifying");

    if(fWrite)
    {
        FMLEXPEND_PAGE(pTI->hFML, ulPage, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("FMLxxx_EXPEND_PAGE() FMLEXPEND_PAGE() page %lU failed with status %s\n",
                ulPage, FfxDecodeIOStatus(&ioStat));

            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                ioStat.ffxStat = DCLSTAT_CURRENTLINE;
        }
    }
    else
    {
        D_BUFFER    abTag[FFX_NAND_TAGSIZE] = {0x00, 0x00};
        unsigned    nBits;

        if(pTI->nVerbosity > DCL_VERBOSE_LOUD)
            DclPrintf("          Verifying FMLREAD_TAGS()\n");
            
        FMLREAD_TAGS(pTI->hFML, ulPage, 1, pTI->pSpareArea, FFX_NAND_TAGSIZE, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("FMLxxx_EXPEND_PAGE() FMLREAD_TAGS() page %lU failed with status %s\n",
                ulPage, FfxDecodeIOStatus(&ioStat));

            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                ioStat.ffxStat = DCLSTAT_CURRENTLINE;

            return ioStat.ffxStat;
        }

        if(DclMemCmp(abTag, pTI->pSpareArea, FFX_NAND_TAGSIZE))
        {
            DclPrintf("FMLxxx_EXPEND_PAGE() FMLREAD_TAGS() returned invalid tag data\n");

            return DCLSTAT_CURRENTLINE;
        }

        if(pTI->nVerbosity > DCL_VERBOSE_LOUD)
            DclPrintf("          Verifying FMLREAD_TAGGEDPAGES()\n");
            
        FMLREAD_TAGGEDPAGES(pTI->hFML, ulPage, 1, pTI->pBuffer2, pTI->pSpareArea, FFX_NAND_TAGSIZE, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("FMLxxx_EXPEND_PAGE() FMLREAD_TAGGEDPAGES() page %lU failed with status %s\n",
                ulPage, FfxDecodeIOStatus(&ioStat));

            if(ioStat.ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA)
            {
                /*  Since we tried to read page data from an area which 
                    was never programmed, it may legitimately report an
                    uncorrectable error, but that is not a fatal error
                    in this case.
                */  
                ioStat.ffxStat = FFXSTAT_SUCCESS;
            }
            else
            {
                if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                    ioStat.ffxStat = DCLSTAT_CURRENTLINE;

                return ioStat.ffxStat;
            }
        }

        if(DclMemCmp(abTag, pTI->pSpareArea, FFX_NAND_TAGSIZE))
        {
            DclPrintf("FMLxxx_EXPEND_PAGE() FMLREAD_TAGGEDPAGES() returned invalid tag data\n");

            return DCLSTAT_CURRENTLINE;
        }

        nBits = DclBitCountArray(pTI->pBuffer2, pTI->FmlInfo.uPageSize * CHAR_BIT);
        if(nBits < pBTI->nMinSetBitsOnErasedPage)
        {
            DclPrintf("FMLxxx_EXPEND_PAGE() FMLREAD_TAGGEDPAGES() returned successfully, but the page was not erased\n");

            return DCLSTAT_CURRENTLINE;
        }
    }

    return ioStat.ffxStat;
}



#endif /* #if FFXCONF_NANDSUPPORT */

