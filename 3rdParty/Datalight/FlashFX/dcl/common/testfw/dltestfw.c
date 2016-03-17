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
  jurisdictions.  Patents may be pending.

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
    $Log: dltestfw.c $
    Revision 1.19  2011/03/07 17:00:44Z  garyp
    Cleaned up some strange logging code.
    Revision 1.18  2010/11/18 19:06:07Z  garyp
    Modified the assert logger to break into the debugger should the
    DCL_TESTFWFLAG_DEBUGERRORS flag be set.
    Revision 1.17  2010/11/01 03:11:31Z  garyp
    Modified to use the new DclLogOpen() function which now takes a
    flags parameter rather than a whole bunch of bools.
    Revision 1.16  2010/09/03 18:16:37Z  garyp
    Updated to build cleanly with the RealView tools.
    Revision 1.15  2010/09/01 01:04:56Z  garyp
    Minor message update -- no functional changes.
    Revision 1.14  2010/08/30 00:28:56Z  garyp
    Major update to support trapping asserts, quitting when the first failure
    occurs, allow popping into the debugger, and allow multiple verbosity
    levels.  Along with a general code cleanup.
    Revision 1.13  2010/08/27 17:50:35Z  garyp
    Added some const keywords where appropriate.
    Revision 1.12  2009/07/24 20:36:36Z  johnbr
    Changes for the power cycling tests.
    Revision 1.11  2009/06/24 22:39:04Z  garyp
    Updated to accommodate functions which now take DCL Instance handles.
    Revision 1.10  2009/02/21 02:00:11Z  brandont
    Changed DclTestFwVPrintf to be private to the module.
    Revision 1.9  2009/01/26 23:14:04Z  keithg
    Added explicit void of unused formal parameter(s).
    Revision 1.8  2009/01/16 23:56:40  keithg
    Extended DCLCONF_OUTPUT_ENABLED condition to include now
    unused variable szDescription.
    Revision 1.7  2009/01/15 18:46:57Z  brandont
    Conditioned DclVSNPrintf calls on DCLCONF_OUTPUT_ENABLED.
    Revision 1.6  2009/01/06 01:07:43Z  brandont
    Updated to display lists of failed, skipped, and not implemented tests
    in the test summary.
    Revision 1.5  2008/06/01 21:46:37Z  garyp
    Merged from the WinMobile branch.  Updated to correctly use
    DclTimeStamp() and DclTimePassed().
    Revision 1.4  2008/05/03 19:54:00Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.3  2007/11/22 03:59:28Z  brandont
    Corrected an issue evaulating the log file name which determines the
    parameters for calling DclLogOpen.
    Revision 1.2  2007/11/03 23:31:12Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/10/17 02:08:24Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlprintf.h>
#include <dlver.h>
#include <dllog.h>
#include <dltestfw.h>
#include "dltestfwp.h"


static DCLSTATUS TestFwAssertIoctl(DCLSERVICEHANDLE hService, DCLREQ_SERVICE *pReq, void *pPriv);


/*-------------------------------------------------------------------
    Local: TestFwVPrintf()
-------------------------------------------------------------------*/
static void TestFwVPrintf(
    tDclTestFwHandle    hTestFw,
    unsigned            nVerbosity,
    const char         *pszFmt,
    va_list             sArgList)
{
    DclProductionAssert(hTestFw);

    if(nVerbosity > hTestFw->nVerbosity)
        return;
    
  #if DCLCONF_OUTPUT_ENABLED
    DclVPrintf(pszFmt, sArgList);
  #endif

    return;
}


/*-------------------------------------------------------------------
    Public: DclTestFwCheckAsserts()
-------------------------------------------------------------------*/
unsigned DclTestFwCheckAsserts(
    tDclTestFwHandle    hTestFw,
    D_BOOL              fPassed)
{
    unsigned            nAssertCount = hTestFw->nAssertCount;

    /*  Regardless what other actions are taken, always reset the
        assert count to zero.
    */        
    hTestFw->nAssertCount = 0;

    if(nAssertCount)
    {
        if(fPassed)
        {
            DclTestFwInternalPrintf(hTestFw, DCL_VERBOSE_NORMAL, 
                "Test claims to have passed and had %u assert(s)\n", 
                nAssertCount);
        }
        else
        {
            DclTestFwInternalPrintf(hTestFw, DCL_VERBOSE_OBNOXIOUS, 
                "Test claims to have failed and had %u assert(s)\n", 
                nAssertCount);
        }
    }

    return nAssertCount;
}


/*-------------------------------------------------------------------
    Public: DclTestFwFlags()
-------------------------------------------------------------------*/
D_UINT32 DclTestFwFlags(
    tDclTestFwHandle hTestFw)
{
    return hTestFw->ulFlags;
}


/*-------------------------------------------------------------------
    Public: DclTestFwPrintf()
-------------------------------------------------------------------*/
void DclTestFwPrintf(
    tDclTestFwHandle    hTestFw,
    const char         *pszMsg,
    ...)
{
    va_list             sArgList;
    unsigned            nIndentCount;

    DclProductionAssert(hTestFw);

    if(hTestFw->nVerbosity == DCL_VERBOSE_QUIET)
        return;
    
    if(hTestFw->fDoIndent)
    {
        for(nIndentCount = hTestFw->nIndentCount; nIndentCount; nIndentCount--)
        {
            DclPrintf(" ");
        }
    }

    va_start(sArgList, pszMsg);
    TestFwVPrintf(hTestFw, DCL_VERBOSE_NORMAL, pszMsg, sArgList);
    va_end(sArgList);

    return;
}


/*-------------------------------------------------------------------
    Public: DclTestFwInternalPrintf()
-------------------------------------------------------------------*/
void DclTestFwInternalPrintf(
    tDclTestFwHandle    hTestFw,
    unsigned            nVerbosity,
    const char         *pszMsg,
    ...)
{
    va_list             sArgList;
    unsigned            nIndentCount;

    DclProductionAssert(hTestFw);
 
    if(nVerbosity > hTestFw->nVerbosity)
        return;
    
    if(hTestFw->fDoIndent)
    {
        for(nIndentCount = hTestFw->nIndentCount; nIndentCount; nIndentCount--)
        {
            DclPrintf(" ");
        }
    }

    va_start(sArgList, pszMsg);
    TestFwVPrintf(hTestFw, nVerbosity, pszMsg, sArgList);
    va_end(sArgList);

    return;
}


/*-------------------------------------------------------------------
    Local: TestFwSummaryListAdd()
-------------------------------------------------------------------*/
static void TestFwSummaryListAdd(
    D_UINT32                ulCategoryNumber,
    DCLTESTFWSUMMARYLIST  **ppListHead,
    D_BOOL                 *pfListIncomplete,
    const char             *pszDescription)
{
    DCLTESTFWSUMMARYLIST   *pNewListEntry;

    /*  Add this item to a list that will displayed in
        test suite summary.
    */
    pNewListEntry = DclMemAllocZero(sizeof(*pNewListEntry));
    if(pNewListEntry)
    {
        pNewListEntry->ulCategoryNumber = ulCategoryNumber;
        pNewListEntry->pNext = *ppListHead;
        *ppListHead = pNewListEntry;
        DclStrNCpy(
                pNewListEntry->szString,
                pszDescription,
                DCLDIMENSIONOF(pNewListEntry->szString));
    }
    else
    {
        *pfListIncomplete = TRUE;
    }

    return;
}


/*-------------------------------------------------------------------
    Public: DclTestFwLogNotImplemented()
-------------------------------------------------------------------*/
void DclTestFwLogNotImplemented(
    tDclTestFwHandle    hTestFw,
    const char         *pszMsg,
    ...)
{
    D_BOOL              fDoIndent;

    DclProductionAssert(hTestFw);
    
    hTestFw->ulTotalNotImplemented++;
    
    DclTestFwInternalPrintf(hTestFw, DCL_VERBOSE_NORMAL, "%s: ", DCL_TESTFW_STRING_NOTIMPLEMENTED);

  #if DCLCONF_OUTPUT_ENABLED
    {
        char    szDescription[DCL_TESTFW_SUMMARY_LIST_STRING];
        va_list sArgList;

        va_start(sArgList, pszMsg);
        TestFwVPrintf(hTestFw, DCL_VERBOSE_NORMAL, pszMsg, sArgList);
        DclVSNPrintf(
                szDescription,
                DCLDIMENSIONOF(szDescription),
                pszMsg,
                sArgList);
        va_end(sArgList);

        /*  Add this skip to a list so that it can be
            displayed in test suite summary.
        */
        TestFwSummaryListAdd(
                hTestFw->ulTestNumber,
                &hTestFw->pNIList,
                &hTestFw->fNIListIncomplete,
                szDescription);
    }
  #else
    {
        TestFwSummaryListAdd(
                hTestFw->ulTestNumber,
                &hTestFw->pNIList,
                &hTestFw->fNIListIncomplete,
                "No description");
    }
  #endif

    fDoIndent = hTestFw->fDoIndent;
    hTestFw->fDoIndent = FALSE;
    DclTestFwInternalPrintf(hTestFw, DCL_VERBOSE_NORMAL, "\n");
    hTestFw->fDoIndent = fDoIndent;

    return;
}


/*-------------------------------------------------------------------
    Local: TestFwSectionSetRange()
-------------------------------------------------------------------*/
static void TestFwSectionSetRange(
    tDclTestFwHandle hTestFw)
{
#if 1
    hTestFw->ulFirstTestNumber = 0;
    hTestFw->ulLastTestNumber = D_UINT32_MAX;
#else
    hTestFw->ulFirstTestNumber = 69;
    hTestFw->ulLastTestNumber = 69;
#endif

    return;
}


/*-------------------------------------------------------------------
    Local: TestFwDisplayElapsedTime()

    Display a millisecond count as an elapsed time string.
-------------------------------------------------------------------*/
static void TestFwDisplayElapsedTime(
    D_UINT32    ulElapsedMS)
{
    D_UINT64    ullMicrosecs = (D_UINT64)ulElapsedMS * 1000;
    char        szBuff[32];

    if(DclTimeFormatUS(ullMicrosecs, szBuff, sizeof(szBuff)))
    {
        char   *pStr = szBuff;

        /*  Skip past the "days" value if it is zero.
        */
        if(*pStr == '0')
            pStr += 2;

        DclPrintf("Elapsed Time: %s\n", pStr);
    }
    else
    {
        DclPrintf("Elapsed Time: ERROR!\n");
    }
}


/*-------------------------------------------------------------------
    Local: TestFwDisplayLine()
-------------------------------------------------------------------*/
static void TestFwDisplayLine(
    const char     *pszFmt,
    D_UINT32        ulTestNumber,
    const char     *pszText)
{
    #define         DCL_TESTFW_LEADING_SYMBOL_COUNT     (4)
    #define         DCL_TESTFW_DISPLAY_SYMBOL_COUNT     (75)
    char            szBuffer[128];
    size_t          nCount;

    nCount = DCL_TESTFW_LEADING_SYMBOL_COUNT;
    for(; nCount; nCount--)
    {
        DclPrintf("-");
    }
    
    if(ulTestNumber != D_UINT32_MAX)
        DclSNPrintf(szBuffer, sizeof(szBuffer), pszFmt, ulTestNumber, pszText);
    else
        DclSNPrintf(szBuffer, sizeof(szBuffer), pszFmt, pszText);

    DclPrintf(szBuffer);
    
    nCount = DclStrLen(szBuffer) + DCL_TESTFW_LEADING_SYMBOL_COUNT;
    for(; nCount < DCL_TESTFW_DISPLAY_SYMBOL_COUNT; nCount++)
    {
        DclPrintf("-");
    }
    
    DclPrintf("\n");

    return;
}


/*-------------------------------------------------------------------
    Local: TestFwSectionHeader()
-------------------------------------------------------------------*/
static void TestFwSectionHeader(
    tDclTestFwHandle    hTestFw,
    D_UINT32            ulTestNumber,
    const char         *pszTestName)
{
    char                szBuffer[128];

    TestFwDisplayLine(" [Test %lU - %s - Begin] ", ulTestNumber, pszTestName);
    
    hTestFw->tsSectionStart = DclTimeStamp();
    DclDateTimeString(NULL, szBuffer, sizeof(szBuffer));
    DclPrintf("Begin time: %s\n\n", szBuffer);

    return;
}


/*-------------------------------------------------------------------
    Local: TestFwSectionFooter()
-------------------------------------------------------------------*/
static void TestFwSectionFooter(
    tDclTestFwHandle    hTestFw,
    D_UINT32            ulTestNumber,
    const char         *pszTestName,
    const char         *pszTestResult)
{
    char                szBuffer[128];
    D_UINT32            ulElapsedMS;

    DclDateTimeString(NULL, szBuffer, sizeof(szBuffer));
    DclPrintf("\nEnd time: %s\n", szBuffer);
    
    ulElapsedMS = DclTimePassed(hTestFw->tsSectionStart);
    TestFwDisplayElapsedTime(ulElapsedMS);
    
    DclPrintf("Result: %s\n", pszTestResult);
    TestFwDisplayLine(" [Test %lU - %s - End] ", ulTestNumber, pszTestName);
    DclPrintf("\n");

    return;
}


/*-------------------------------------------------------------------
    Public: DclTestGetResultString()

    Return a char * to the string equivalent to eDCLTESTFWSTATUS. 
    This is used to return a set of short strings for test results.
    For a set of longer strings, use DclTestGetLongResultString()
-------------------------------------------------------------------*/
const char * DclTestGetResultString( 
    eDCLTESTFWSTATUS    eResult)
{
    /*  This must match the eDCLTESTFWSTATUS, in .../include/dltestfw.h
    */
    static const char  *apszDclResultStrings[] =
    {
        "INT ERROR",
        "NULL",
        "NULL",
        "pass",
        "FAIL",
        "skip",
        "ERROR",
        "Missing"
    };

    if (eResult <= DCL_TESTFW_INTERNAL_ERR || eResult > DCL_TESTFW_NOTIMPLEMENTED)
        eResult = DCL_TESTFW_INTERNAL_ERR;

    return apszDclResultStrings[eResult];
}


/*-------------------------------------------------------------------
    Public: DclTestGetLongResultString()

    Return a char * to the string equivalent to eDCLTESTFWSTATUS.
    This is used to return a set of short strings for test results.
    For a set of longer strings, use DclTestGetResultLongString()
-------------------------------------------------------------------*/
const char * DclTestGetLongResultString(
    eDCLTESTFWSTATUS    eResult)
{
    /*  This must match the eDCLTESTFWSTATUS, in .../include/dltestfw.h
    */
    static const char  *apszDclLongResultStrings[] =
    {
        DCL_TESTFW_STRING_INTERNAL,
        "NULL",
        "NULL",
        DCL_TESTFW_STRING_PASSED,
        DCL_TESTFW_STRING_FAILED,
        DCL_TESTFW_STRING_SKIPPED,
        DCL_TESTFW_STRING_ERROR,
        DCL_TESTFW_STRING_NOTIMPLEMENTED
    };

    if (eResult <= DCL_TESTFW_INTERNAL_ERR || eResult > DCL_TESTFW_NOTIMPLEMENTED)
        eResult = DCL_TESTFW_INTERNAL_ERR;

    return apszDclLongResultStrings[eResult];
}


/*-------------------------------------------------------------------
    Public: DclTestFwSectionExecute()

    Run each of the tests listed the the provided test matrix
-------------------------------------------------------------------*/
eDCLTESTFWSTATUS DclTestFwSectionExecute(
    tDclTestFwHandle                hTestFw,
    tDclTestSpecificData           *pTestData,
    const tDclTestFwTestSection    *TestSection,
    D_UINT32                        ulTestEntries)
{
    #define                         DCL_TESTFW_INDENT_SIZE  (4)
    D_UINT32                        ulIndex;
    eDCLTESTFWSTATUS                eTestResult;
    D_BOOL                          fAllTestsPassed = TRUE;

    /*  For each test case, ...
    */
    for(ulIndex = 0; ulIndex < ulTestEntries; ulIndex++)
    {
        if(TestSection[ulIndex].fIsCategory)
        {
            /*  Perform the test category
            */
            eTestResult = (TestSection[ulIndex].TestPtr)(hTestFw, pTestData);
            if(eTestResult == DCL_TESTFW_FAILED)
            {
                fAllTestsPassed = FALSE;

                if(hTestFw->ulFlags & DCL_TESTFWFLAG_DEBUGERRORS)
                    DclOsDebugBreak(DCLDBGCMD_BREAK);
                
                if(hTestFw->ulFlags & DCL_TESTFWFLAG_QUITONFAILURE)
                    break;
            }
            
            continue;
        }

        /*  Skip test numbers that are not within the range specified
            on the command line.
        */
        hTestFw->ulTestNumber++;
        if( (hTestFw->ulTestNumber < hTestFw->ulFirstTestNumber) ||
            (hTestFw->ulTestNumber > hTestFw->ulLastTestNumber) )
        {
            continue;
        }

        /*  Display the test section header
        */
        TestFwSectionHeader(hTestFw, hTestFw->ulTestNumber, TestSection[ulIndex].pszTestName);

        /*  Perform the test
        */
        hTestFw->fDoIndent = TRUE;
        hTestFw->nIndentCount += DCL_TESTFW_INDENT_SIZE;
        eTestResult = (TestSection[ulIndex].TestPtr)(hTestFw, pTestData);
        hTestFw->nIndentCount -= DCL_TESTFW_INDENT_SIZE;
        hTestFw->fDoIndent = FALSE;

        if(DclTestFwCheckAsserts(hTestFw, eTestResult == DCL_TESTFW_PASSED ? TRUE : FALSE))
        {
            if(eTestResult == DCL_TESTFW_PASSED)
                eTestResult = DCL_TESTFW_FAILED;
        }

        /*  Display the appropriate output for the test result
        */
        hTestFw->ulTotalTests++;
        switch(eTestResult)
        {
            case DCL_TESTFW_PASSED:
                hTestFw->ulTotalPassed++;
                TestFwSectionFooter(
                        hTestFw,
                        hTestFw->ulTestNumber,
                        TestSection[ulIndex].pszTestName,
                        DCL_TESTFW_STRING_PASSED);
                break;

            case DCL_TESTFW_FAILED:
                hTestFw->ulTotalFailed++;
                fAllTestsPassed = FALSE;
                TestFwSectionFooter(
                        hTestFw,
                        hTestFw->ulTestNumber,
                        TestSection[ulIndex].pszTestName,
                        DCL_TESTFW_STRING_FAILED);

                /*  Add this failure to a list so that it can be
                    displayed in test suite summary.
                */
                TestFwSummaryListAdd(
                        hTestFw->ulTestNumber,
                        &hTestFw->pFailedList,
                        &hTestFw->fFailedListIncomplete,
                        TestSection[ulIndex].pszTestName);

                /*  Note that because the debug check is done by both
                    the various "CHECK_*" macros and at this location,
                    you may end up in the debugger twice for any given
                    error.

                    Necessary because most tests don't yet use the 
                    CHECK_* macros.
                */                    
                if(hTestFw->ulFlags & DCL_TESTFWFLAG_DEBUGERRORS)
                    DclOsDebugBreak(DCLDBGCMD_BREAK);
                
                if(hTestFw->ulFlags & DCL_TESTFWFLAG_QUITONFAILURE)
                    goto Cleanup;
                
                break;

            case DCL_TESTFW_SKIPPED:
                hTestFw->ulTotalSkipped++;
                TestFwSectionFooter(
                        hTestFw,
                        hTestFw->ulTestNumber,
                        TestSection[ulIndex].pszTestName,
                        DCL_TESTFW_STRING_SKIPPED);

                /*  Add this skip to a list so that it can be
                    displayed in test suite summary.
                */
                TestFwSummaryListAdd(
                        hTestFw->ulTestNumber,
                        &hTestFw->pSkippedList,
                        &hTestFw->fSkippedListIncomplete,
                        TestSection[ulIndex].pszTestName);
                break;

            /*  Invalid test result
            */
            default:
                DclProductionError();
                break;
        }
    }

  Cleanup:

    /*  Return whether all the tests in this section passed
    */
    if(fAllTestsPassed)
        return DCL_TESTFW_PASSED;
    else
        return DCL_TESTFW_FAILED;
}


/*-------------------------------------------------------------------
    Local: TestFwTestHeader()
-------------------------------------------------------------------*/
static eDCLTESTFWSTATUS TestFwTestHeader(
    tDclTestFwHandle        hTestFw,
    tDclTestSpecificData   *pTestData,
    const char             *pszTitle,
    D_UINT32                ulVERSIONVAL,
    const char             *pszBUILDNUM,
    const char             *pszCOPYRIGHT)
{
    D_BOOL                  fDebug = D_DEBUG;
    char                    achBuff[DCL_MAX_VERSION_LENGTH+1];
    char                    szBuffer[128];
    eDCLTESTFWSTATUS        eResult;

    DclVersionFormat(achBuff, sizeof(achBuff), ulVERSIONVAL, pszBUILDNUM);

    /*  Display the program name and version
    */
    TestFwDisplayLine(" [%s Header - Begin] ", D_UINT32_MAX, pszTitle);
    DclPrintf("%s %s%s\n", pszTitle, achBuff, fDebug ? " (DEBUG Version)" : "");
    DclPrintf("%s\n", PRODUCTEDITION);
    DclPrintf("%s\n\n", pszCOPYRIGHT);

    eResult = hTestFw->TestHeaderCallback(hTestFw, pTestData);

    DclDateTimeString(NULL, szBuffer, sizeof(szBuffer));
    DclPrintf("\nBegin time: %s\n", szBuffer);
    
    hTestFw->tsTestStart = DclTimeStamp();
    TestFwDisplayLine(" [%s Header - End] ", D_UINT32_MAX, pszTitle);
    DclPrintf("\n");

    return eResult;
}


/*-------------------------------------------------------------------
    Local: TestFwDisplaySingleList()
-------------------------------------------------------------------*/
static void TestFwDisplaySingleList(
    tDclTestFwHandle        hTestFw,
    const char             *pszFmtMsg,
    D_UINT32                ulCount,
    D_BOOL                  fListIncomplete,
    DCLTESTFWSUMMARYLIST  **ppListHead)
{
    void                   *pTemp;
    DCLTESTFWSUMMARYLIST   *pEntry;

    (void)hTestFw; /* Currently unused */

    pEntry = *ppListHead;
    if(pEntry)
    {
        DclPrintf(pszFmtMsg, ulCount);
        
        while(pEntry)
        {
            if(pEntry->ulCategoryNumber)
                DclPrintf("    Test %3lU - %s\n", pEntry->ulCategoryNumber, pEntry->szString);
            else
                DclPrintf("    %s\n", pEntry->szString);
            pTemp = pEntry;
            pEntry = pEntry->pNext;
            DclMemFree(pTemp);
        }
        
        *ppListHead = NULL;
        if(fListIncomplete)
            DclPrintf("    *** One or more entries are missing from this list ***\n");

        DclPrintf("\n");
    }

    return;
}


/*-------------------------------------------------------------------
    Local: TestFwDisplayAllSummaryLists()
-------------------------------------------------------------------*/
static void TestFwDisplayAllSummaryLists(
    tDclTestFwHandle hTestFw)
{
    TestFwDisplaySingleList(
            hTestFw,
            "  Individual categories failed : %lU\n",
            hTestFw->ulTotalFailed,
            hTestFw->fFailedListIncomplete,
            &hTestFw->pFailedList);

    TestFwDisplaySingleList(
            hTestFw,
            "  Individual categories skipped : %lU\n",
            hTestFw->ulTotalSkipped,
            hTestFw->fSkippedListIncomplete,
            &hTestFw->pSkippedList);

    TestFwDisplaySingleList(
            hTestFw,
            "  Individual cases not implemented : %lU\n",
            hTestFw->ulTotalNotImplemented,
            hTestFw->fNIListIncomplete,
            &hTestFw->pNIList);

    return;
}


/*-------------------------------------------------------------------
    Local: TestFwTestFooter()
-------------------------------------------------------------------*/
static void TestFwTestFooter(
    tDclTestFwHandle    hTestFw,
    const char         *pszTitle)
{
    char                szBuffer[128];
    D_UINT32            ulElapsedMS;

    /*  Display the results
    */
    TestFwDisplayLine(" [%s Summary - Begin] ", D_UINT32_MAX, pszTitle);
    
    DclDateTimeString(NULL, szBuffer, sizeof(szBuffer));
    DclPrintf("End Time: %s\n", szBuffer);
    
    ulElapsedMS = DclTimePassed(hTestFw->tsTestStart);
    TestFwDisplayElapsedTime(ulElapsedMS);
    
    DclPrintf("\n");
    DclPrintf("  Total Tests   : %3lU\n", hTestFw->ulTotalTests);
    DclPrintf("  Tests Passed  : %3lU\n", hTestFw->ulTotalPassed);
    DclPrintf("  Tests Skipped : %3lU\n", hTestFw->ulTotalSkipped);
    DclPrintf("  Tests Failed  : %3lU\n\n", hTestFw->ulTotalFailed);

    TestFwDisplayAllSummaryLists(hTestFw);

    TestFwDisplayLine(" [%s Summary - End] ", D_UINT32_MAX, pszTitle);
    DclPrintf("\n");

    return;
}


/*-------------------------------------------------------------------
    Public: DclTestFwTestExecute()
-------------------------------------------------------------------*/
eDCLTESTFWSTATUS DclTestFwTestExecute(
    D_UINT32                        ulFlags,
    unsigned                        nVerbosity,
    const char                     *pszTitle,
    D_UINT32                        ulVERSIONVAL,
    const char                     *pszBUILDNUM,
    const char                     *pszCOPYRIGHT,
    const char                     *pszLogFile,
    tDclTestSpecificData           *pTestData,
    tDclTestFwCase                  TestHeaderCallback,
    const tDclTestFwTestSection    *TestSection,
    D_UINT32                        ulTestEntries)
{
    DCLLOGHANDLE                    hLog = NULL;
    tDclTestFwHandle                hTestFw;
    eDCLTESTFWSTATUS                eResult;
    DCLDECLARESERVICE               (srv, "TESTFWASSERT", DCLSERVICE_ASSERT, TestFwAssertIoctl, NULL, DCLSERVICEFLAG_THREADSAFE);

    /*  Allocate a test framework handle
    */
    hTestFw = DclMemAllocZero(sizeof(*hTestFw));
    if(!hTestFw)
        return DCL_TESTFW_ERROR;

    hTestFw->ulTestNumber = 0;
    hTestFw->TestHeaderCallback = TestHeaderCallback;
    hTestFw->ulFlags = ulFlags;
    hTestFw->nVerbosity = nVerbosity;

    /*  Create a log file
    */
    if(pszLogFile && *pszLogFile)
        hLog = DclLogOpen(NULL, pszLogFile, 2048, DCLLOGFLAGS_SHADOW);
    
    srv.pPrivateData = hTestFw;
    if(DclServiceCreate(0, &srv) != DCLSTAT_SUCCESS)
    {
        if(hLog)
            DclLogClose(NULL, hLog);
            
        DclMemFree(hTestFw);
        
        return DCL_TESTFW_ERROR;
    }

    /*  Display test signon
    */
    eResult = TestFwTestHeader(
            hTestFw,
            pTestData,
            pszTitle,
            ulVERSIONVAL,
            pszBUILDNUM,
            pszCOPYRIGHT);

    /*  Execute the test cases
    */
    if(eResult == DCL_TESTFW_PASSED)
    {
        TestFwSectionSetRange(hTestFw);
        eResult = DclTestFwSectionExecute(hTestFw, pTestData, TestSection, ulTestEntries);
    }

    /*  Display test summary
    */
    TestFwTestFooter(hTestFw, pszTitle);

    DclServiceDestroy(&srv);

    if(hLog)
        DclLogClose(NULL, hLog);

    /*  Free the test framework handle
    */
    DclMemFree(hTestFw);

    return eResult;
}


/*-------------------------------------------------------------------
    Local: TestFwAssertIoctl()

    Parameters:
        hService - The service handle
        pReq     - A pointer to the DCLREQ_SERVICE structure
        pPriv    - A pointer to any private data to pass

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestFwAssertIoctl(
    DCLSERVICEHANDLE        hService,
    DCLREQ_SERVICE         *pReq,
    void                   *pPriv)
{
    tDclTestFwHandle        hTestFw = pPriv;

    DclAssertWritePtr(hService, sizeof(*hService));
    DclAssertWritePtr(pReq, sizeof(*pReq));
    DclAssertWritePtr(hTestFw, sizeof(*hTestFw));

    if(pReq->ior.ulReqLen != sizeof(*pReq))
        return DCLSTAT_BADSTRUCLEN;

    switch(pReq->ior.ioFunc)
    {
        case DCLIOFUNC_SERVICE_CREATE:
        case DCLIOFUNC_SERVICE_DESTROY:
        case DCLIOFUNC_SERVICE_REGISTER:
        case DCLIOFUNC_SERVICE_DEREGISTER:
            DclAssert(pReq->pSubRequest == NULL);

            /*  Nothing to do
            */
            return DCLSTAT_SUCCESS;

        case DCLIOFUNC_SERVICE_DISPATCH:
        {
            if(pReq->pSubRequest == NULL)
                return DCLSTAT_SERVICE_BADSUBREQUEST;

            if(pReq->pSubRequest->ulReqLen < sizeof(pReq->pSubRequest))
                return DCLSTAT_BADSTRUCLEN;

            switch(pReq->pSubRequest->ioFunc)
            {
                case DCLIOFUNC_ASSERT_FIRED:
                {
                    DCLREQ_ASSERT_FIRED *pSubReq = (DCLREQ_ASSERT_FIRED*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    DclPrintf("Dcl:TestFwAssertIoctl() captured assert which fired in \"%s\" at line %u\n", pSubReq->pszModuleName, pSubReq->nLineNumber);

                    hTestFw->nAssertCount++;

                    if(hTestFw->ulFlags & DCL_TESTFWFLAG_DEBUGERRORS)
                        DclOsDebugBreak(DCLDBGCMD_BREAK);

                    return DCLSTAT_SUCCESS;
                }

                default:
                    DCLPRINTF(1, ("Dcl:TestFwAssertIoctl() Unsupported subfunction %x\n", pReq->pSubRequest->ioFunc));
                    return DCLSTAT_SERVICE_UNSUPPORTEDREQUEST;
            }
        }

        default:
            DCLPRINTF(1, ("Dcl:TestFwAssertIoctl() Unhandled request %x\n", pReq->ior.ioFunc));
            return DCLSTAT_SERVICE_UNHANDLEDREQUEST;
    }
}

