/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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

    This module contains tests for the ECC enabled FMSL API.  These tests
    are currently only run on NAND flash and require that the optional
    RAW API are supported by the NTM.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmslecc.c $
    Revision 1.24  2011/07/26 15:24:39Z  garyp
    Updated a message -- no functional changes.
    Revision 1.23  2010/10/11 22:16:29Z  glenns
    Adjust ECC test to more randomly choose a starting bit position
    in test bytes to begin injecting bit errors. Repair flawed boundary
    checking in the same tests.
    Revision 1.22  2010/09/23 05:58:14Z  garyp
    Corrected non-functional error handling logic in TestSetup().
    Revision 1.21  2010/08/05 21:12:56Z  glenns
    Add conditional to not run ECC tests on NAND flash with transparent EDC.
    Revision 1.20  2010/07/07 01:11:58Z  garyp
    Fixed some incorrect Hungarian notation.
    Revision 1.19  2010/07/06 01:19:55Z  garyp
    Minor debug code / datatype updates -- no functional changes.
    Revision 1.18  2010/06/19 03:33:49Z  garyp
    Used the EDC capabilities reported for the Device to decide how
    many bit errors to correct.
    Revision 1.17  2010/03/02 18:44:27Z  glenns
    Fix Bug 2630: Apply PXA320_HACK more appropriately.
    Revision 1.16  2010/01/10 20:50:59Z  garyp
    General cleanup to follow naming and code formatting standards.
    Updated debug message levels.  No functional changes.
    Revision 1.15  2009/12/31 17:24:42Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.14  2009/07/28 18:59:45Z  garyp
    Merged from the v4.0  branch.  Changed all uses of FMLREAD_CORRECTEDPAGES()
    to use FMLREAD_PAGES().
    Revision 1.13  2009/05/27 17:36:50Z  garyp
    Updated debug code to be more useful.
    Revision 1.12  2009/04/17 18:45:09Z  keithg
    Fixed bug 2632, build failures when BBMSUPPORT is disabled.
    Revision 1.11  2009/03/31 19:54:04Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.10  2009/03/18 07:15:43Z  keithg
    Bug 2462 fixed, ECC errors were reported when attempting to verify
    ECC operation in pages within a second chip.
    Revision 1.9  2009/03/09 19:52:50Z  glenns
    Modified Test_FMLREAD_TAGS to accept correctable errors to account for 
    device/controller combinations (such as FlexOneNAND) that don't distinguish
    between main page and spare area bit errors when performing corrections.
    Revision 1.8  2009/02/19 18:18:22Z  glenns
    Changed some DclPrintfs to FFXPRINTFs so that the FMSL ECC tests are not so
    verbose at debug level 0 when testing configurations requiring multi-bit 
    EDC and/or with larger spare areas.
    Revision 1.7  2009/01/21 20:55:07Z  glenns
    Removed checks on FFXSTAT_FIMCORRECTABLESPARE in Test_FMLREAD_TAGGEDPAGES
    and Test_FMLREAD_TAGS.  This indication is to be removed.
    Revision 1.6  2009/01/21 00:09:49Z  glenns
    Modified Test_FMLREAD_CORRECTEDPAGES to accomodate the fact that bit errors
    injected into the spare area can touch ECC bytes and cause correctable error
    status to be noted.
    Revision 1.5  2009/01/19 17:26:10Z  keithg
    Updated to account for new status information passed from the
    FMSL on detection of corrected read errors.
    Revision 1.4  2008/01/31 01:05:30Z  Garyp
    Modified the ECC test to allow, without failing, more errors to be
    corrrected than were expected to be corrected, since depending on the
    error pattern, this is possible with some hardware.  Updated debugging
    code and message verbosity to be more useful.
    Revision 1.3  2008/01/13 07:26:48Z  keithg
    Function header updates to support autodoc.
    Revision 1.2  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2007/10/22 22:48:40Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxfmlapi.h>
#include <fxtools.h>
#include <fxtrace.h>
#include "fmsltst.h"

#if FFXCONF_NANDSUPPORT
#include "fmslnand.h"


/*  The PXA320 DFC has an apparent hardware defect which will cause this
    test to fail, unless PXA320_HACK is set to TRUE.  This will cause an
    extra read to be performed, which will clear a previous error single-
    bit error state.  This flaw affects FMSLTEST due to the sequence of
    operations, but not the general operation of FlashFX (generally...).
*/
#ifndef PXA320_HACK
  #define PXA320_HACK     FALSE
#endif

/*  ECC test data and definitions
*/
typedef struct
{
    D_UINT16            uEccCorrect;
    D_UINT16            uEccDetect;
    D_UINT32            ulStartPage;
    D_UINT32            ulStartRawPage;
    D_UINT32            ulEndPage;
    D_UINT32            ulEndRawPage;
    D_UINT32            ulCurrentPage;
    D_UINT32            ulCurrentRawPage;
    D_UINT32            ulGoodPage;
    D_BUFFER           *pMainData;
    D_BUFFER           *pSpareData;
    DCLALIGNEDBUFFER    (tag,data,FFX_NAND_TAGSIZE);
} ECCTESTINFO;

#define NAND_ECC_TEST_NUMBER_BLOCKS         2   /*  Number of erase blocks to use */
#define NAND_ECC_TEST_TAGDATA               0xAC
#define NAND_ECC_TEST_ATTEMPTS              5   /* Number of attempts on a given page. */
#define NAND_ECC_TEST_NAME                  "ECC_TEST"
#define NAND_ECC_TEST_MAIN                  1
#define NAND_ECC_TEST_SPARE                 2

#define NAND_ECC_TEST_STATUS_PASS           0   /* test passed */
#define NAND_ECC_TEST_STATUS_ERROR          1   /* general/unknown error */
#define NAND_ECC_TEST_STATUS_NOTREPORTED    2   /* error not reported */
#define NAND_ECC_TEST_STATUS_NOTCORRECTED   3   /* correctable error not corrected */
#define NAND_ECC_TEST_STATUS_CORRECTED      4   /* corrected even though not expected */

typedef D_UINT32 (FNECCTEST)(FMSLNANDTESTINFO *pTI, ECCTESTINFO *pEccTestInfo, D_BOOL fMain, D_BOOL fCorrectable);

static D_UINT32 Test_FMLREAD_PAGES(      FMSLNANDTESTINFO *pTI, ECCTESTINFO *pEccTestInfo, D_BOOL fMain, D_BOOL fCorrectable);
static D_UINT32 Test_FMLREAD_TAGGEDPAGES(FMSLNANDTESTINFO *pTI, ECCTESTINFO *pEccTestInfo, D_BOOL fMain, D_BOOL fCorrectable);
static D_UINT32 Test_FMLREAD_TAGS(       FMSLNANDTESTINFO *pTI, ECCTESTINFO *pEccTestInfo, D_BOOL fMain, D_BOOL fCorrectable);


typedef struct
{
    FNECCTEST      *pfnTest;
    char           *pszName;
} FMSLECCTESTINFO;

static FMSLECCTESTINFO aECCReadTests[] =
{
    {Test_FMLREAD_PAGES,        "FMLREAD_PAGES"},
    {Test_FMLREAD_TAGGEDPAGES,  "FMLREAD_TAGGEDPAGES"},
    {Test_FMLREAD_TAGS,         "FMLREAD_TAGS"}
};


static D_BOOL   TestCleanup(    FMSLNANDTESTINFO *pTI, ECCTESTINFO *pEccTestInfo);
static D_BOOL   TestMain(       FMSLNANDTESTINFO *pTI, ECCTESTINFO *pEccTestInfo);
static D_BOOL   TestSetup(      FMSLNANDTESTINFO *pTI, ECCTESTINFO *pEccTestInfo);
static D_BOOL   TestSpare(      FMSLNANDTESTINFO *pTI, ECCTESTINFO *pEccTestInfo);
static D_BOOL   WriteBitErrors( FMSLNANDTESTINFO *pTI, ECCTESTINFO *pEccTestInfo, D_UINT32 ulNumErrors, D_UINT32 ulByteOffset, D_UINT16 uFlag);
static void     MakeBitErrors(D_BUFFER *pDestBuffer, D_UINT32 ulBufferSize, D_UINT32 ulNumberErrors, D_UINT32 ulByteOffset);
static unsigned MatchBuffers(const D_BUFFER *pBuffer1, const D_BUFFER *pBuffer2, unsigned nLen);


/*-------------------------------------------------------------------
    Private: FfxFmslNANDTestEcc()

    Tests the NTM's ECC correction and detection.

    Parameters:
        pTI - The test parameters structure

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
--------------------------------------------------------------------*/
D_BOOL FfxFmslNANDTestEcc(
    FMSLNANDTESTINFO   *pTI)
{
    D_BOOL              fReturn = FALSE;
    FFXIOSTATUS         ioStat;
    ECCTESTINFO         eccTestInfo;

    DclAssert(pTI);

    DclPrintf("    Error Detection/Correction (EDC) Tests...\n");

    /*  See if this device requires ECC. There is a variety of NAND flash
        that includes internal EDC engines that are completely transparent
        to FlashFX, and thier workings are not accessible. Don't run
        ECC tests on devices on which ECC function is invisible.
    */
    if(!(pTI->FmlDevInfo.uDeviceFlags & DEV_REQUIRES_ECC))
    {
        DclPrintf("      Device EDC function is transparent.  Test SKIPPED.\n");
        return TRUE;
    }

    /*  Attempt a raw read to check if funtionality is supported.  Don't
        fail test if unsupported, but log as not implemented and cleanup
    */
    FMLREAD_RAWPAGES(pTI->hFML, 0, 1, pTI->pBuffer, NULL, ioStat);
    if(ioStat.ffxStat == FFXSTAT_FIM_UNSUPPORTEDFUNCTION)
    {
        DclPrintf("      NTM does not support \"raw\" access.  Test SKIPPED.\n");
        return TRUE;
    }

    if(TestSetup(pTI, &eccTestInfo))
    {
        if(TestMain(pTI, &eccTestInfo))
        {
            if(TestSpare(pTI, &eccTestInfo))
                fReturn = TRUE;
        }
    }

    /*  Make sure to report clean up failure, but not to turn a test failure
        into a pass.
    */
    if(!TestCleanup(pTI, &eccTestInfo) && fReturn)
        fReturn = FALSE;

    return fReturn;
}


/*-------------------------------------------------------------------
    Local: TestSetup()

    Sets up the initial test conditions.  This includes finding
    suitable (good, raw) blocks to use for the test.  It also
    allocates test memory and creates the control data for the
    main and spare areas of a page.

    ToDo: This function leaks memory if it fails!

    Parameters:
        pTI          - The test parameters structure
        pEccTestInfo - ECC tests structure

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
 -------------------------------------------------------------------*/
static D_BOOL TestSetup(
    FMSLNANDTESTINFO           *pTI,
    ECCTESTINFO                *pEccTestInfo)
{
    D_UINT32                    ulAttempts;
    D_UINT32                    ulMaxAttempts;
    D_UINT32                    ulStartBlock = D_UINT32_MAX;
    D_UINT32                    ulStartRawBlock = D_UINT32_MAX;
    FFXIOSTATUS                 ioStat;
  #if FFXCONF_BBMSUPPORT
    FFXIOR_FML_GET_BLOCK_INFO   blockinfo;
  #endif

    DclAssert(pTI);
    DclAssert(pEccTestInfo);

    if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("      Creating test control data...\n");

    /*  Find test blocks
    */
    if(!FfxFmslNANDFindTestBlocks(pTI, NAND_ECC_TEST_NUMBER_BLOCKS, &ulStartBlock, TRUE))
    {
        DclPrintf("      Failed to find %u consecutive erase block(s)\n", NAND_ECC_TEST_NUMBER_BLOCKS);
        return FALSE;
    }
    
  #if FFXCONF_BBMSUPPORT
    FML_GET_BLOCK_INFO(pTI->hFML, ulStartBlock, ioStat, blockinfo);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("      Failed to request block info for %lU (%s)\n", ulStartBlock, FfxDecodeIOStatus(&ioStat));
        return FALSE;
    }
    ulStartRawBlock = blockinfo.ulRawMapping;
  #else
    ulStartRawBlock = ulStartBlock;
  #endif
  
    DclMemSet(pEccTestInfo, 0, sizeof(*pEccTestInfo));

    /*  Allocate buffers for this specific test
    */
    pEccTestInfo->pMainData = DclMemAlloc(pTI->FmlInfo.uPageSize);
    if(!pEccTestInfo->pMainData)
        return FALSE;

    pEccTestInfo->pSpareData = DclMemAlloc(pTI->FmlInfo.uSpareSize);
    if(!pEccTestInfo->pSpareData)
        return FALSE;

    /*  Initialize tags for this test
    */
    DclMemSet(pEccTestInfo->tag.data, NAND_ECC_TEST_TAGDATA, FFX_NAND_TAGSIZE);

    /*  Calculate starting and ending page
    */
    pEccTestInfo->ulStartPage      = ulStartBlock * pTI->ulPagesPerBlock;
    pEccTestInfo->ulStartRawPage   = ulStartRawBlock * pTI->ulPagesPerBlock;

    pEccTestInfo->ulEndPage        = pEccTestInfo->ulStartPage +
                                       (pTI->ulPagesPerBlock * NAND_ECC_TEST_NUMBER_BLOCKS);
    pEccTestInfo->ulEndRawPage     = pEccTestInfo->ulStartRawPage +
                                       (pTI->ulPagesPerBlock * NAND_ECC_TEST_NUMBER_BLOCKS);

    pEccTestInfo->ulCurrentPage    = pEccTestInfo->ulStartPage;
    pEccTestInfo->ulCurrentRawPage = pEccTestInfo->ulStartRawPage;

    /*  Determine level of correctable and detectable ECC errors.
    */
    pEccTestInfo->uEccCorrect    = pTI->FmlDevInfo.uEdcCapability;
    pEccTestInfo->uEccDetect     = pTI->FmlDevInfo.uEdcCapability + 1;

    /*  Debug assert that correctable <= detectable.  As the chip ID
        process becomes more developed to deal with MLC flash (and 
        with that different levels of correct/detect, this check 
        should possibly fail the test.
    */
    DclAssert(pEccTestInfo->uEccCorrect <= pEccTestInfo->uEccDetect);

    /*  Erase the first block
    */
    FMLERASE_BLOCKS(pTI->hFML, ulStartBlock, 1, ioStat);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("        Failed to erase block %lU\n", ulStartBlock);
        return FALSE;
    }

    /*  Get original page buffer data
    */
    FfxFmslNANDPageFill(pTI, pTI->pBuffer, 1, pTI->ulSerial,
                        pEccTestInfo->ulStartPage, NAND_ECC_TEST_NAME);

    /*  Write out page buffer with ECC to get the spare area data this
        test will use for all testing

        The attempts threshold allows for normal bit errors above and
        beyond what this test introduces, and avoids false test failures.
    */
    ulMaxAttempts = DCLMIN(NAND_ECC_TEST_ATTEMPTS, pTI->ulPagesPerBlock);
    for(ulAttempts = 1; ulAttempts <= ulMaxAttempts; ulAttempts++)
    {
        /*  Write page using ECC
        */
        FMLWRITE_TAGGEDPAGES(pTI->hFML, pEccTestInfo->ulCurrentPage, 1,
                             pTI->pBuffer, pEccTestInfo->tag.data, FFX_NAND_TAGSIZE, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("        Failed to write page %lU\n", pEccTestInfo->ulCurrentPage);
            return FALSE;
        }

        /*  Read back with rawRead to verify write completed error free
        */
        FMLREAD_RAWPAGES(pTI->hFML, pEccTestInfo->ulCurrentRawPage, 1,
                         pEccTestInfo->pMainData, pEccTestInfo->pSpareData, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("        Failed to raw read page %lU\n", pEccTestInfo->ulCurrentPage);
            return FALSE;
        }

        pEccTestInfo->ulCurrentPage++;
        pEccTestInfo->ulCurrentRawPage++;

        /*  Buffers should match exactly.  If not, there must have been a
            "natural" bit error.  Try again.
        */
        if(DclMemCmp(pEccTestInfo->pMainData, pTI->pBuffer, pTI->FmlInfo.uPageSize) != 0)
        {
            if(pTI->nVerbosity >= DCL_VERBOSE_NORMAL)
                DclPrintf("        Raw page read verify failed, repeating attempt to initialize test buffers\n");

            continue;
        }

        /*  Record the page number for a page which is considered to be "good"
        */
        pEccTestInfo->ulGoodPage = pEccTestInfo->ulCurrentPage - 1;

        return TRUE;
    }

    DclPrintf("        Unexpected number of errors encountered, failing test.\n");
    return FALSE;
}


/*-------------------------------------------------------------------
    Local: TestCleanup()

    Cleans up after the ECC tests.  This includes erasing the
    test blocks and free any allocated resources.

    Parameters:
        pTI          - The test parameters structure
        pEccTestInfo - ECC tests structure

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
 -------------------------------------------------------------------*/
static D_BOOL TestCleanup(
    FMSLNANDTESTINFO   *pTI,
    ECCTESTINFO        *pEccTestInfo)
{
    D_UINT32            ulBlock;
    FFXIOSTATUS         ioStat;

    DclAssert(pTI);
    DclAssert(pEccTestInfo);

    if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("      Cleaning up...\n");

    ulBlock = pEccTestInfo->ulStartPage / pTI->ulPagesPerBlock;

    /*  Erase the blocks we used for this test.
    */
    FMLERASE_BLOCKS(pTI->hFML, ulBlock, NAND_ECC_TEST_NUMBER_BLOCKS, ioStat);
    if(!IOSUCCESS(ioStat, NAND_ECC_TEST_NUMBER_BLOCKS))
    {
        DclPrintf("      Failed to erase block %lU\n", ulBlock);
        return FALSE;
    }

    /*  Release resources in reverse order of allocation
    */
    if(pEccTestInfo->pSpareData)
        DclMemFree(pEccTestInfo->pSpareData);

    if(pEccTestInfo->pMainData)
        DclMemFree(pEccTestInfo->pMainData);

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: TestMain()

    Tests bit errors in the main page.

    Parameters:
        pTI          - The test parameters structure
        pEccTestInfo - ECC tests structure

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
 -------------------------------------------------------------------*/
static D_BOOL   TestMain(
    FMSLNANDTESTINFO   *pTI,
    ECCTESTINFO        *pEccTestInfo)
{
    D_UINT32            ulByteOffset;
    D_UINT16            uEccCurrent;
    D_UINT32            ulStatus;
    D_BOOL              fReturn = FALSE;
    D_BOOL              fCorrectable;
    D_UINT16            i;
    FMSLECCTESTINFO    *pTest;

    DclAssert(pTI);
    DclAssert(pEccTestInfo);

    DclPrintf("      Main Page Tests...\n");

    /*  Test all correctable and detectable bit errors.
    */
    for(uEccCurrent = 1;
        uEccCurrent <= pEccTestInfo->uEccDetect;
        uEccCurrent++)
    {
        fCorrectable = uEccCurrent <= pEccTestInfo->uEccCorrect;
        if(pTI->nVerbosity >= DCL_VERBOSE_NORMAL)
        {
            DclPrintf("        Testing %U-bit error (%s)...\n", uEccCurrent,
                      fCorrectable ? "correctable" : "uncorrectable");
        }

        /*  Choose a random offset within the main page and write the error
            to the flash. Avoid choosing an offset where the number of bit
            errors to inject is greater than the number of bits left before
            the end of the buffer.
        */
        ulByteOffset = DclRand(pTI->pulRandomSeed) % (pTI->FmlInfo.uPageSize - (uEccCurrent / 8U));
        if(!WriteBitErrors(pTI, pEccTestInfo, uEccCurrent, ulByteOffset, NAND_ECC_TEST_MAIN))
        {
            DclPrintf("          Failed to create test condition\n");
            goto Cleanup;
        }

        /*  Test each of the ECC enabled FMSL Read API with the current error
            state on the flash.
        */
        for(i = 0; i < DCLDIMENSIONOF(aECCReadTests); i++)
        {
            pTest = &aECCReadTests[i];

            DclPrintf("          Testing %s...\n", pTest->pszName);

            ulStatus = pTest->pfnTest(pTI, pEccTestInfo, TRUE, fCorrectable);
            switch(ulStatus)
            {
                case NAND_ECC_TEST_STATUS_PASS:
                    break;

                case NAND_ECC_TEST_STATUS_CORRECTED:
                    DclPrintf("            Error was corrected when not expected.\n");
                    break;

                case NAND_ECC_TEST_STATUS_ERROR:
                    DclPrintf("        %s failed.\n", pTest->pszName);
                    goto Cleanup;

                case NAND_ECC_TEST_STATUS_NOTREPORTED:
                    DclPrintf("        %s did not report error.\n", pTest->pszName);
                    goto Cleanup;

                case NAND_ECC_TEST_STATUS_NOTCORRECTED:
                    DclPrintf("        %s did not correct error.\n", pTest->pszName);
                    goto Cleanup;

                default:
                    DclPrintf("        %s returned unknown status %lU.\n", pTest->pszName, ulStatus);
                    goto Cleanup;
            }
        }

      #if PXA320_HACK
        {
            FFXIOSTATUS     ioStat;

            DclPrintf("Performing an extra read of good tags to accomplish PXA320_HACK\n");

            FMLREAD_TAGS(pTI->hFML, pEccTestInfo->ulGoodPage, 1, pEccTestInfo->tag.data, FFX_NAND_TAGSIZE, ioStat);
            DclAssert(IOSUCCESS(ioStat, 1));
        }
      #endif

        /*  Increment the current page, wrap to beginning of erase block if
            needed.
        */
        pEccTestInfo->ulCurrentPage++;
        pEccTestInfo->ulCurrentRawPage++;

        if(pEccTestInfo->ulCurrentPage == pEccTestInfo->ulEndPage)
        {
            pEccTestInfo->ulCurrentPage = pEccTestInfo->ulStartPage;
            pEccTestInfo->ulCurrentRawPage = pEccTestInfo->ulStartRawPage;
        }
    }

    fReturn = TRUE;

  Cleanup:

    DclPrintf("        %s\n", fReturn ?  "PASSED" : "FAILED");

    return fReturn;
}


/*-------------------------------------------------------------------
    Local: TestSpare()

    Tests bit errors in the spare area.

    Parameters:
        pTI          - The test parameters structure
        pEccTestInfo - ECC tests structure

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
 -------------------------------------------------------------------*/
static D_BOOL   TestSpare(
    FMSLNANDTESTINFO   *pTI,
    ECCTESTINFO        *pEccTestInfo)
{
    D_UINT32            ulByteOffset;
    D_UINT16            uEccCurrent;
    D_UINT32            ulStatus;
    D_BOOL              fReturn = FALSE;
    D_BOOL              fCorrectable;
    D_UINT16            i;
    FMSLECCTESTINFO    *pTest;
    D_UINT16            uOldEccCorrect = pEccTestInfo->uEccCorrect;
    D_UINT16            uOldEccDetect = pEccTestInfo->uEccDetect;

    DclAssert(pTI);
    DclAssert(pEccTestInfo);

    DclPrintf("      Spare Area Tests...\n");

    if(pEccTestInfo->uEccCorrect > 1)
    {
        DclPrintf("        Only testing single bit errors in the spare area!\n");
        pEccTestInfo->uEccCorrect = 1;
        pEccTestInfo->uEccDetect = 2;
    }
    
    /*  Test all correctable and detectable bit errors
    */
    for(uEccCurrent = 1;
        uEccCurrent <= pEccTestInfo->uEccDetect;
        uEccCurrent++)
    {
        fCorrectable = uEccCurrent <= pEccTestInfo->uEccCorrect;
        if(pTI->nVerbosity >= DCL_VERBOSE_NORMAL)
        {
            DclPrintf("        Testing %U-bit error (%s)...\n", uEccCurrent,
                      fCorrectable ? "correctable" : "uncorrectable");
        }

        /*  For the spare area test, an error is tested in each byte of the
            spare area, up to the limit where the number of bit errors is
            greater than the number of bits left in the buffer.
        */
        for(ulByteOffset = 0;
            ulByteOffset < (pTI->FmlInfo.uSpareSize - (uEccCurrent / 8U));
            ulByteOffset++)
        {
            if(!WriteBitErrors(pTI, pEccTestInfo, uEccCurrent, ulByteOffset, NAND_ECC_TEST_SPARE))
            {
                DclPrintf("        Failed to create test condition.\n");
                goto Cleanup;
            }

            /*  Test each of the ECC enabled FMSL Read API with the current
                error state on the flash.
            */
            for(i = 0; i < DCLDIMENSIONOF(aECCReadTests); i++)
            {
                pTest = &aECCReadTests[i];

                DclPrintf("          Testing offset %lU with %s...\n", ulByteOffset, pTest->pszName);

                ulStatus = pTest->pfnTest(pTI, pEccTestInfo, FALSE, fCorrectable);
                switch(ulStatus)
                {
                    case NAND_ECC_TEST_STATUS_PASS:
                        break;

                    case NAND_ECC_TEST_STATUS_ERROR:
                        DclPrintf("        %s failed.\n", pTest->pszName);
                        goto Cleanup;

                    case NAND_ECC_TEST_STATUS_NOTREPORTED:
                        DclPrintf("        %s did not report error.\n", pTest->pszName);
                        goto Cleanup;

                    case NAND_ECC_TEST_STATUS_NOTCORRECTED:
                        DclPrintf("        %s did not correct error.\n", pTest->pszName);
                        goto Cleanup;

                    default:
                        DclPrintf("        %s returned unknown status %lU.\n", pTest->pszName, ulStatus);
                        goto Cleanup;
                }
            }

            /*  Increment the current page, wrap to beginning of erase block if
                needed.
            */
            pEccTestInfo->ulCurrentPage++;
            pEccTestInfo->ulCurrentRawPage++;

            if(pEccTestInfo->ulCurrentPage == pEccTestInfo->ulEndPage)
            {
                pEccTestInfo->ulCurrentPage = pEccTestInfo->ulStartPage;
                pEccTestInfo->ulCurrentRawPage = pEccTestInfo->ulStartRawPage;
            }
        }
    }

    fReturn = TRUE;

  Cleanup:

    /*  Restore any original values we trashed.
    */
    pEccTestInfo->uEccCorrect = uOldEccCorrect;
    pEccTestInfo->uEccDetect = uOldEccDetect;

    DclPrintf("        %s\n", fReturn ?  "PASSED" : "FAILED");

    return fReturn;
}


/*-------------------------------------------------------------------
    Local: MakeBitErrors()

    Toggles ulNumberErrors consecutive number of bits inside
    pBuffer, starting at ulByteOffset.

    Conducts simple DclAssert parameter checks to ensure the
    buffer is not overrun.

    Parameters:
        pBuffer        - Buffer to inject errors.
        ulBufferSize   - Size of pBuffer.
        ulNumberErrors - Number of bit errors to inject.
        ulByteOffset   - Byte offset into the buffer to
                         begin errors.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void MakeBitErrors(
    D_BUFFER       *pBuffer,
    D_UINT32        ulBufferSize,
    D_UINT32        ulNumberErrors,
    D_UINT32        ulByteOffset)
{
    D_UINT32        i, j;
    D_BUFFER        mask = 0;
    D_UINT32        ulCurrentErrorTotal = 0;

    DclAssert(pBuffer);
    DclAssert(ulBufferSize);
    DclAssert(ulByteOffset < ulBufferSize);
    DclAssert(ulBufferSize >= ((ulNumberErrors + 7) / 8) + ulByteOffset);

    /*  Quick check: */
    if (!ulNumberErrors)
        return;
    
    /*  Be careful not to select a first bit location too close to the
        end of the buffer such that we end up spilling over the end.
    */
    if(ulBufferSize == ((ulNumberErrors + 7) / 8) + ulByteOffset)
        j = DclRand(NULL) % (8UL - ((ulNumberErrors-1UL) % 8));
    else
        j = DclRand(NULL) % 8;

    /*  Start injecting bit errors.
    */    
    for(i = ulByteOffset;
        (i < ulBufferSize) && (ulCurrentErrorTotal < ulNumberErrors); i++)
    {
        /*  Create bit mask
        */
        mask = 0;
        for(; (j < 8) && (ulCurrentErrorTotal < ulNumberErrors); j++)
        {
            mask |= (1<<j);
            ulCurrentErrorTotal++;
        }
        pBuffer[i] ^= mask;
        j = 0;
    }

    return;
}


/*-------------------------------------------------------------------
    Local: WriteBitErrors()

    Introduces ulNumErrors number of bit toggles into the current
    page specified in the ECCTestInfo structure starting at byte
    offset ulByteOffset, either into the main area or the spare
    area depending on the value passed in by uFlag.

    Parameters:
        pTI          - FMSL NAND Test Information
        pEccTestInfo - ECC Test Information
        ulNumErrors  - Number of bit error(s) to inject
        ulByteOffset - Byte offset to inject bit error(s)
        uFlag        - Flag to specify main data or spare area

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
-------------------------------------------------------------------*/
static D_BOOL WriteBitErrors(
    FMSLNANDTESTINFO   *pTI,
    ECCTESTINFO        *pEccTestInfo,
    D_UINT32            ulNumErrors,
    D_UINT32            ulByteOffset,
    D_UINT16            uFlag)
{
    D_UINT32            ulTestAttempts  = 0;

    DclAssert(pTI);
    DclAssert(pEccTestInfo);

    /*  Allow for multiple attempts to create the exact bit error state on
        the flash.
    */
    while(ulTestAttempts < NAND_ECC_TEST_ATTEMPTS)
    {
        FFXIOSTATUS ioStat;
        unsigned    nPageMatch;
        unsigned    nSpareMatch;

        /*  Copy the original buffers into the buffers for this test.
        */
        DclMemCpy(pTI->pBuffer, pEccTestInfo->pMainData, pTI->FmlInfo.uPageSize);
        DclMemCpy(pTI->pSpareArea, pEccTestInfo->pSpareData, pTI->FmlInfo.uSpareSize);

        /*  Create ulNumErrors number of bit errors.
        */
        switch(uFlag)
        {
            case NAND_ECC_TEST_MAIN:
            {
                if(pTI->nVerbosity > DCL_VERBOSE_NORMAL)
                {
                    DclPrintf("          Creating %lU bit error in main page, page %lU, byte offset %lU\n",
                               ulNumErrors, pEccTestInfo->ulCurrentPage, ulByteOffset);
                }

                MakeBitErrors(pTI->pBuffer, pTI->FmlInfo.uPageSize, ulNumErrors, ulByteOffset);

                if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
                {
                    DclPrintf("          Changed byte 0x%02X to 0x%02X\n",
                        pEccTestInfo->pMainData[ulByteOffset], pTI->pBuffer[ulByteOffset]);
                }
                break;
            }

            case NAND_ECC_TEST_SPARE:
            {
                if(pTI->nVerbosity > DCL_VERBOSE_NORMAL)
                {
                    DclPrintf("          Creating %lU bit error in spare area, page %lU, byte offset %lU\n",
                               ulNumErrors, pEccTestInfo->ulCurrentPage, ulByteOffset);
                }

                MakeBitErrors(pTI->pSpareArea, pTI->FmlInfo.uSpareSize, ulNumErrors, ulByteOffset);

                if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
                {
                    DclPrintf("          Changed byte 0x%02X to 0x%02X\n",
                        pEccTestInfo->pSpareData[ulByteOffset], pTI->pSpareArea[ulByteOffset]);
                }
                break;
            }

            default:
            {
                DclPrintf("          Unknown flag %U in CreateBitErrorsInPage.\n", uFlag);
                DclError();
                return FALSE;
            }
        }

        /*  Erase the block whenever this hits the first page of the block.
        */
        if((pEccTestInfo->ulCurrentPage % pTI->ulPagesPerBlock) == 0)
        {
            FMLERASE_BLOCKS(pTI->hFML, pEccTestInfo->ulCurrentPage / pTI->ulPagesPerBlock, 1, ioStat);
            if(!IOSUCCESS(ioStat, 1))
            {
                DclPrintf("          Failed to erase block %lU\n",
                          pEccTestInfo->ulCurrentPage / pTI->ulPagesPerBlock);
                return FALSE;
            }
        }

        /*  Write bit errors out with RawPageWrite.
        */
        FMLWRITE_RAWPAGES(pTI->hFML, pEccTestInfo->ulCurrentRawPage, 1, pTI->pBuffer, pTI->pSpareArea, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("          Failed to raw write page %lU\n", pEccTestInfo->ulCurrentPage);
            return FALSE;
        }

        /*  Read back verify with RawPageRead to check for extra
            bit errors other than we injected.
        */
        FMLREAD_RAWPAGES(pTI->hFML, pEccTestInfo->ulCurrentRawPage, 1, pTI->pBuffer2, pTI->pSpareArea2, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("          Raw page read verify failed for page %lU\n", pEccTestInfo->ulCurrentPage);
            return FALSE;
        }

        /*  Buffers should match exactly.  If not, assume there was an
            additional bit error other than the errors this test was to inject
            and reattempt test with the same number of injected bit errors.
        */
        nPageMatch = MatchBuffers(pTI->pBuffer, pTI->pBuffer2, pTI->FmlInfo.uPageSize);
        nSpareMatch = MatchBuffers(pTI->pSpareArea, pTI->pSpareArea2, pTI->FmlInfo.uSpareSize);

        if(!nPageMatch && !nSpareMatch)
        {
            /*  Test error state successfully created on the flash at the
                current page.
            */
            break;
        }
        else
        {
            if(nPageMatch)
            {
                DclPrintf("          Raw page read verify failed at offset %u, retrying test for %lU bit errors on page %lU\n",
                          nPageMatch-1, ulNumErrors, pEccTestInfo->ulCurrentRawPage);

                if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
                {
                    DclHexDump("Expected:\n", HEXDUMP_UINT8, 32, pTI->FmlInfo.uPageSize, pTI->pBuffer);
                    DclHexDump("Got:     \n", HEXDUMP_UINT8, 32, pTI->FmlInfo.uPageSize, pTI->pBuffer2);
                }
            }

            if(nSpareMatch)
            {
                DclPrintf("          Raw spare read verify failed at offset %u, retrying test for %lU bit errors on page %lU\n",
                          nSpareMatch-1, ulNumErrors, pEccTestInfo->ulCurrentRawPage);

                if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
                {
                    DclHexDump("Expected:\n", HEXDUMP_UINT8, 16, pTI->FmlInfo.uSpareSize, pTI->pSpareArea);
                    DclHexDump("Got:     \n", HEXDUMP_UINT8, 16, pTI->FmlInfo.uSpareSize, pTI->pSpareArea2);
                }
            }

            /*  Increment ulPage, wrap to beginning of erase block if needed.
            */
            pEccTestInfo->ulCurrentPage++;
            pEccTestInfo->ulCurrentRawPage++;

            if(pEccTestInfo->ulCurrentPage == pEccTestInfo->ulEndPage)
            {
                pEccTestInfo->ulCurrentPage = pEccTestInfo->ulStartPage;
                pEccTestInfo->ulCurrentRawPage = pEccTestInfo->ulStartRawPage;
            }

            /*  Check the number of times this loop has run.  This allows a
                threshold of attempts to account for "natural" bit errors
                above and beyond what this test introduces, and avoids false
                test failures.
            */
            if(++ulTestAttempts < NAND_ECC_TEST_ATTEMPTS)
            {
                continue;
            }
            else
            {
                DclPrintf("          Unexpected number of errors encountered, failing test.\n");
                return FALSE;
            }
        }
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: Test_FMLREAD_PAGES()

    This tests FMLREAD_PAGES given the specified bit error state on
    the flash.  It must correct and report correctable errors and
    report uncorrectable errors.

    FMLREAD_PAGES only returns the main page data.  It is unclear
    what the expected result should be for errors that occur in the
    spare area.

    Parameters:
        pTI          - FMSL NAND Test Information
        pEccTestInfo - ECC Test Information
        fMain        - Flag indicating if the error is in the main
                       page
        fCorrectable - Flag indicating if the error is correctable

    Return Value:
        NAND_ECC_TEST_STATUS_PASS  - Test passed
        NAND_ECC_TEST_STATUS_ERROR - Unexpected IO error
        NAND_ECC_TEST_STATUS_NOTCORRECTED - Correctable error not corrected
        NAND_ECC_TEST_STATUS_NOTREPORTED - Error not reported.
-------------------------------------------------------------------*/
static D_UINT32 Test_FMLREAD_PAGES(
    FMSLNANDTESTINFO   *pTI,
    ECCTESTINFO        *pEccTestInfo,
    D_BOOL              fMain,
    D_BOOL              fCorrectable)
{
    FFXIOSTATUS         ioStat;
    FFXSTATUS           statusExpected;
    D_BOOL              fMatch;

    DclAssert(pTI);
    DclAssert(pEccTestInfo);

    FMLREAD_PAGES(pTI->hFML, pEccTestInfo->ulCurrentPage, 1, pTI->pBuffer, ioStat);

    /*  Compare data with original data
    */
    fMatch = FfxFmslNANDPageVerify(pTI, pTI->pBuffer, 1, pTI->ulSerial,
                                   pEccTestInfo->ulStartPage,
                                   NAND_ECC_TEST_NAME, 0, 1, WITH_ECC);

    statusExpected = fMain ? FFXSTAT_FIMCORRECTABLEDATA : FFXSTAT_SUCCESS;

    /*  Determine what the test result is based on the test case and the
        result of the operation.
    */
    if(fCorrectable)
    {
        /*  The operation should have been successful.
        */
        if(ioStat.ffxStat != statusExpected || ioStat.ulCount != 1)
        {
            /*  Sooner or later, bit errors injected into the spare area
                will touch an ECC byte, causing correctable error status
                to be returned.
            */
            if (!fMain && (ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA))
                return NAND_ECC_TEST_STATUS_PASS;
            else
                return NAND_ECC_TEST_STATUS_ERROR;
        }

        /*  The data should match and the status should have been reported.
        */
        if(!fMatch)
        {
            return NAND_ECC_TEST_STATUS_NOTCORRECTED;
        }
        else
        {
            /*  Correctable errors in the main page must be reported.
                We cannot require correctable errors in the spare area to
                be reported because they may be at an unused location.
            */
            if(!fMain || ioStat.op.ulPageStatus & PAGESTATUS_DATACORRECTED)
                return NAND_ECC_TEST_STATUS_PASS;
            else
                return NAND_ECC_TEST_STATUS_NOTREPORTED;
        }
    }
    else
    {
        /*  Uncorrectable errors in the main page must always be reported.
            Uncorrectable errors in the spare area may go unnoticed
            if they occur in an unused location.
        */
        if(fMain && (ioStat.ffxStat != FFXSTAT_FIMUNCORRECTABLEDATA))
        {
            if(fMatch)
                return NAND_ECC_TEST_STATUS_CORRECTED;
            else
                return NAND_ECC_TEST_STATUS_NOTREPORTED;
        }
        else
        {
            return NAND_ECC_TEST_STATUS_PASS;
        }
    }
}


/*-------------------------------------------------------------------
    Local: Test_FMLREAD_TAGGEDPAGES()

    This tests FMLREAD_TAGGEDPAGES given the specified bit
    error state on the flash.  It must correct and report
    correctable errors and report uncorrectable errors.

    Parameters:
        pTI          - FMSL NAND Test Information
        pEccTestInfo - ECC Test Information
        fMain        - Flag indicating if the error is in the main page
        fCorrectable - Flag indicating if the error is correctable

    Return Value:
        NAND_ECC_TEST_STATUS_PASS  - Test passed
        NAND_ECC_TEST_STATUS_ERROR - Unexpected IO error
        NAND_ECC_TEST_STATUS_NOTCORRECTED - Correctable error not corrected
        NAND_ECC_TEST_STATUS_NOTREPORTED - Error not reported.
-------------------------------------------------------------------*/
static D_UINT32 Test_FMLREAD_TAGGEDPAGES(
    FMSLNANDTESTINFO   *pTI,
    ECCTESTINFO        *pEccTestInfo,
    D_BOOL              fMain,
    D_BOOL              fCorrectable)
{
    FFXIOSTATUS         ioStat;
    FFXSTATUS           statusExpected;
    D_BOOL              fMatch;
    DCLALIGNEDBUFFER    (tmptag,data,FFX_NAND_TAGSIZE);

    DclAssert(pTI);
    DclAssert(pEccTestInfo);

    /*  Read data using FMLREAD_TAGGEDPAGES.
    */
    FMLREAD_TAGGEDPAGES(pTI->hFML, pEccTestInfo->ulCurrentPage, 1,
                        pTI->pBuffer, tmptag.data, FFX_NAND_TAGSIZE, ioStat);

    /*  Compare data with original data
    */
    fMatch = FfxFmslNANDPageVerify(pTI, pTI->pBuffer, 1, pTI->ulSerial,
                                   pEccTestInfo->ulStartPage,
                                   NAND_ECC_TEST_NAME, 0, 1, WITH_ECC);
    fMatch = fMatch && (DclMemCmp(pEccTestInfo->tag.data, tmptag.data, FFX_NAND_TAGSIZE) == 0);

    statusExpected = FFXSTAT_FIMCORRECTABLEDATA;

    /*  Determine what the test result is based on the test case and the
        result of the operation.
    */
    if(fCorrectable)
    {
        /*  The operation should have been successful.
        */
        if(ioStat.ffxStat != statusExpected || ioStat.ulCount != 1)
        {
            if(!IOSUCCESS(ioStat, 1))
            {
                FFXPRINTF(1, ("FMLREAD_TAGGEDPAGES() for page %lU returned %s\n", 
                    pEccTestInfo->ulCurrentPage, FfxDecodeIOStatus(&ioStat)));
            
                return NAND_ECC_TEST_STATUS_ERROR;
            }
        }

        /*  The data should match and the status should have been reported.
        */
        if(!fMatch)
        {
            return NAND_ECC_TEST_STATUS_NOTCORRECTED;
        }
        else
        {
            /*  Correctable errors in the main page must be reported.
                We cannot require correctable errors in the spare area to
                be reported because they may be at an unused location.
            */
            if(!fMain || ioStat.op.ulPageStatus & PAGESTATUS_DATACORRECTED)
                return NAND_ECC_TEST_STATUS_PASS;
            else
                return NAND_ECC_TEST_STATUS_NOTREPORTED;
        }
    }
    else
    {
        /*  Uncorrectable errors in the main page must always be reported.
            Uncorrectable errors in the spare area may go unnoticed
            if they occur in an unused location.
        */
        if(fMain && (ioStat.ffxStat != FFXSTAT_FIMUNCORRECTABLEDATA))
        {
            if(fMatch)
                return NAND_ECC_TEST_STATUS_CORRECTED;
            else
                return NAND_ECC_TEST_STATUS_NOTREPORTED;
        }
        else
        {
            return NAND_ECC_TEST_STATUS_PASS;
        }
    }
}


/*-------------------------------------------------------------------
    Local: Test_FMLREAD_TAGS()

    This tests FMLREAD_TAGS given the specified bit
    error state on the flash.

    Because FMLREAD_TAGS only returns the tags, we cannot require
    that any errors be reported which occur in the main page or
    spare area.  The ECC for the main page may not be checked
    and an error in the spare area may be at an unused location.

    Parameters:
        pTI          - FMSL NAND Test Information
        pEccTestInfo - ECC Test Information
        fMain        - Flag indicating if the error is in the main page
        fCorrectable - Flag indicating if the error is correctable

    Return Value:
        NAND_ECC_TEST_STATUS_PASS  - Test passed
        NAND_ECC_TEST_STATUS_ERROR - Unexpected IO error
        NAND_ECC_TEST_STATUS_NOTCORRECTED - Correctable error not corrected
        NAND_ECC_TEST_STATUS_NOTREPORTED - Error not reported.
-------------------------------------------------------------------*/
static D_UINT32 Test_FMLREAD_TAGS(
    FMSLNANDTESTINFO   *pTI,
    ECCTESTINFO        *pEccTestInfo,
    D_BOOL              fMain,
    D_BOOL              fCorrectable)
{
    FFXIOSTATUS         ioStat;
    FFXSTATUS           statusExpected;
    D_BOOL              fMatch;
    DCLALIGNEDBUFFER    (tmptag,data,FFX_NAND_TAGSIZE);

    DclAssert(pTI);
    DclAssert(pEccTestInfo);

    /*  Read data using FMLREAD_TAGS.
    */
    FMLREAD_TAGS(pTI->hFML, pEccTestInfo->ulCurrentPage, 1, tmptag.data, FFX_NAND_TAGSIZE, ioStat);

    /*  Compare data with original data
    */
    fMatch = DclMemCmp(pEccTestInfo->tag.data, tmptag.data, FFX_NAND_TAGSIZE) == 0;

    statusExpected = fMain ? FFXSTAT_SUCCESS : FFXSTAT_FIMCORRECTABLEDATA;

    /*  Determine what the test result is based on the test case and the
        result of the operation.
    */
    if(fCorrectable)
    {
        /*  The operation should have been successful.
        */
        if(ioStat.ffxStat != statusExpected || ioStat.ulCount != 1)
        {
            if(!IOSUCCESS(ioStat, 1))
            {
                /*  Some controllers/ECC mechanisms may not sufficiently
                    distinguish between main page and spare area bit
                    errors:
                */
                if (ioStat.ffxStat != FFXSTAT_FIMCORRECTABLEDATA)
                    return NAND_ECC_TEST_STATUS_ERROR;
            }
        }

        /*  The data should match.
        */
        if(!fMatch)
        {
            return NAND_ECC_TEST_STATUS_NOTCORRECTED;
        }
        else
        {
            return NAND_ECC_TEST_STATUS_PASS;
        }
    }
    else
    {
        return NAND_ECC_TEST_STATUS_PASS;
    }
}


/*-------------------------------------------------------------------
    Local: MatchBuffers()

    Compare two buffers to ensure that they match.

    Parameters:
        pBuffer1    - The first buffer to use
        pBuffer2    - The second buffer to use
        nLen        - The length to compare

    Return Value:
        Returns 0 if the buffer match exactly, or the 1-based index
        of the first non-matching byte.
-------------------------------------------------------------------*/
static unsigned MatchBuffers(
    const D_BUFFER *pBuffer1,
    const D_BUFFER *pBuffer2,
    unsigned        nLen)
{
    unsigned        kk;

    DclAssert(pBuffer1);
    DclAssert(pBuffer2);
    DclAssert(nLen);

    for(kk=0; kk<nLen; kk++)
    {
        if(pBuffer1[kk] != pBuffer2[kk])
            return kk+1;
    }

    return 0;
}



#endif /* #if FFXCONF_NANDSUPPORT */

