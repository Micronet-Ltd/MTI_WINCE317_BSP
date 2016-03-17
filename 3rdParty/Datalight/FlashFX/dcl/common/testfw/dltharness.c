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
  jurisdictions.  Patents may be pending.

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

    Datalight Test Harness

    Describes the DCL test harness, its data structures, and macros
    necessary to build tests.  The test framework is designed to make
    writing test cases easy.

    There is a three-level heirarchy consisting of:

    1)  A Test Library.  Containing Test Suites.
        The library object carries the command line parameters for the run,
        exporting them via accessor methods.  It also exports the log format.

    2)  One or more Test Suites.  The Test Suite handles test setup and
        test teardown, a level of timestamp control, and contains a group of
        related tests.  A Test Suite may include all of the basic functional
        tests error tests, and performance tests for a single function,
        totaling three suites.  All of these might be in a single group.

    3)  One or more Test Cases.  A Test Case verifies some aspect of a
        function's behavior under a fixed set of conditions.  It may require
        several assertions to complete. Assertions may be called 'Checks'
        If a sinble check fails, then the entire test case is considered to
        fail.

    A Test Suite consists of a group of test cases with the same setup and
    teardown requirements.  If a group of tests requires a particular set of
    files, then the group is logically a suite.  The fact that test cases are
    related by subject (they all test the FfxFmlIoRequest() call) does not
    make them a logical suite.  The collection of tests for that function
    would make a test suite, designed to test just that call.

    The Test Suite calls its setup function, the test case function(s) and
    the teardown function for each case, in the order added to the suite.  If
    the setup or teardown function fails or aborts, then the suite is aborted
    as a failure, and all remaining tests are marked as skipped.  If the
    setup function reports a "Skipped" result, then the testcase function is
    not run, because "something happened" during setup.  A memory allocation
    failure may cause such a condition.  The Test Suite is not aborted.  If
    a test case fails or is aborted, then the result is logged and the next
    case setup/test/teardown is executed.

    The setup, teardown and the test case function communicate via an opaque
    handle, which will hold a pointer to a block that the setup routine has
    created and the teardown routine will deallocate any resources used.

    Test libraries, suites, groups, and cases are managed with the:
    DclTestCreateXXX and DclTestDestroyXXX functions.

    The DclTestAddXXX routines associate one object to another object.
    Typically this is done by linking the object to the appropriate list.
    Each test case is described with a descriptor that includes an identity
    description, setup, and pointer to the actual test function.  The
    structure that defines each test case is DclTestCase_t.

    The accessor functions get the approprite item from the objects they
    reference.  The various object pointer types may be passed as opaque
    to prevent the user from accessing the fields directly, forcing the
    use of these functions.  The accessor functions are prefixed with
    DclTestGetXXX.

    The actual execution of tests are completed by DclTestRunTestLibrary().
    Order of test execution of a library is a) Suites; c) test cases, each
    in the order they are placed into the lists.

    There are three phases to the test output.

    A running log of the test results as they are executed.  This provides
    the user with a status of the tests.

    A test case output consists of a header line which announces the test
    case, one line per check in the case which gives the status of the
    internal checks, and a footer line indicating the results of the test.
    One of these blocks will be output for each test case.

    A test summary for the run will be generated at the end of the run. It
    contains totals for Pass, Fail, Skipped and Aborted for each test suite
    and a total for the run.

    Optionally, a list of non-passing checks in each failing test case with
    the file and line number for each failed check.

    Future improvements may include:

    - The DCL Asssert function needs to be hooked and handled
      in a way that can carry the test context into the assert.
    - Flush and block to ensure execution and logging are in sync.
    - Limit number of failed checks or tests before aborting a suite.
    - Validation code for the various object pointers.
    - Consider addition of support for a watch dog.
    - Guard againsted excessive length strings.
---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltharness.c $
    Revision 1.25  2009/08/06 23:16:10Z  garyp
    Updated to make the DCL instance handle available to the test code.
    Revision 1.24  2009/06/28 00:31:49Z  garyp
    Updated to replace the assert functionality using a service.
    Revision 1.23  2009/03/31 18:09:09Z  keithg
    Corrected test for suite failure condition.
    Revision 1.22  2009/03/24 21:56:53Z  keithg
    Added volatile for autos which may be colbbered by a longjmp.
    Revision 1.21  2009/02/26 17:26:30Z  keithg
    Now uses memset to initialize the DCLTIMESTAMPS; added public
    for sending test formatted output.
    Revision 1.20  2009/02/17 06:01:22Z  keithg
    Added explicit void of unused formal parameters.
    Revision 1.19  2009/02/16 07:53:23Z  keithg
    Changed timimg statistics to correctly use the timestamp and time
    passed functionality.
    Revision 1.18  2009/02/13 02:29:20Z  keithg
    Test case execution now starts with ABORT results.
    Revision 1.17  2009/02/09 22:52:16Z  thomd
    Better way to determine inclusion of vxworks.h
    Revision 1.16  2009/02/09 21:00:59Z  thomd
    CBIO requires vxworks.h before setjmp.h
    Revision 1.15  2009/02/08 08:03:42Z  keithg
    Completed support to catch asserts and gracefully fail a test case in
    the event of a product assert.  Moved assert hooks into the test case
    control code rather than the actual test case itself.
    Revision 1.14  2009/02/06 00:45:38  keithg
    - Added new assert handling to catch product asserts in a thread-safe
      fashion and log ocurrences and fail the tests.  (not fully implemented)
    - Wholesale changes to static naming conventions.
    - Added one time initialization into the create functions.
    - Properly labled protected functions.
    - Added memory pool validation at end of test case run.
    - Reorganized test summaries to be more readable.
    Revision 1.13  2009/02/02 21:30:45  keithg
    Added not implemented error checking into assert function;  Now test
    cases can assert it is not implemented.
    Revision 1.12  2009/02/02 09:27:01  keithg
    Increased string buffer length; now logs not implemented test cases in
    a more elegent manner;  cleaned up assert logging and comments.
    Revision 1.11  2009/01/28 08:18:47  keithg
    Now builds cleanly with DCLCONF_OUTPUT_ENABLED disabled.
    Revision 1.10  2009/01/27 08:40:41  keithg
    Removed unused label and static prototype.
    Revision 1.9  2009/01/26 15:01:01  keithg
    - Removed 'group' concept to simplify test structures and error handling.
    - Wholesale changes to variable namespace to fullfill hungarian
      notations and coding standards.
    - Consolidated output to DclTestLogOutput() and reformated text
      to be a bit more user friendly and consistent.
    - Updated comments to match the current implementation.
    Revision 1.8  2009/01/21 22:51:55  keithg
    Conditioned printf output on DCLCONF_OUTPUTENABLED.
    Revision 1.7  2009/01/21 14:52:13  keithg
    Corrected prototype.
    Revision 1.6  2009/01/21 10:41:41  keithg
    Removed grouping feature, now just libraries and suites.
    Revision 1.5  2009/01/20 21:04:33  keithg
    Corrected typo in DclTestFRunTestLibrary() function declaration.
    Revision 1.4  2009/01/15 14:37:51  keithg
    Corrected labels and macro function calls introduced in the last checkin;
    and added test case data pointer and 32-bit parameter for each test case
    description.
    Revision 1.3  2009/01/14 18:10:23Z  keithg
    Updated to use the new DclTestHandle_t.
    Revision 1.2  2009/01/12 01:18:04Z  michaelm
    revised formatting
    Revision 1.1  2009/01/09 00:50:20Z  keithg
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

#ifdef DCL_OSTARGET_VXWORKS
#include <types/vxTypesOld.h>
#endif

#include <setjmp.h>

#include <dlprintf.h>
#include <dlservice.h>
#include "dltharness.h"


typedef struct tagAssertContext_s AssertContext_t;
typedef struct tagAssertList_s  AssertList_t;

struct tagAssertContext_s {
    DCLTHREADID      threadId;
    unsigned         nAssertCount;
    jmp_buf          jumpBuffer;
    AssertContext_t   *pNext;
    AssertContext_t   *pPrev;
};
struct tagAssertList_s {
    PDCLMUTEX         pmutexLock;
    unsigned          nMaxAsserts;
    unsigned          nContextCount;
    AssertContext_t   *pHead;
};


#define MAX_ALLOWED_ASSERTS     0
static AssertList_t gAssertList = {
    NULL, MAX_ALLOWED_ASSERTS, 0, NULL };

static void TestUpdateTestStats(
    DclTestStats_t * pStatistics,
    DclTestCase_t * testcase);
static void TestWriteReport(
    DclTestReport_t type,
    char *szName,
    DclTestStats_t *pstats);
static DclTestCase_t *TestCreateCase(
    char                *szTestID,
    char                *szName,
    D_UINT32             ulTestCaseParam,
    DclTestCaseData_t   *pSetupData,
    PFNDCL_TESTCASE      fFunctionPtr );
static void TestDestroyCase(
    DclTestCase_t    * testcase);
static void TestMarkTestsBeginning(
    DclTestStats_t *statistics );
static void TestMarkTestsEnding(
    DclTestStats_t *statistics );
static void TestMarkTestCaseSkipped(
    DclTestCase_t *testcase );
static void TestRunTestCase(
    DclTestCase_t *testcase );
static void TestRunTestSuite(
    DclTestSuite_t *suite );
static void TestClearStatistics(
    DclTestStats_t *statistics );
static DCLSTATUS TestAssertIoctl(
    DCLSERVICEHANDLE        hService,
    DCLREQ_SERVICE         *pReq,
    void                   *pPriv);

static void TestPrintf(
    const char  *pszFmt, ...);
static void TestLogOutput(
    DclTestType_t       type,
    DclTestResult_t     result,
    char               *szPostFormat,
    ...);
static void TestFrameworkSetupCheck(void);
static void TestContextAddAssert(
    AssertContext_t *pAssertContext);
static void TestContextRemoveAssert(
    AssertContext_t *pAssertContext);
static void TestContextReportAssert(void);



/*-------------------------------------------------------------------
    Protected: DclTestBeginTest

    A routine to be called at the start of each test case.

    Parameters:
        testcase - The current test case.

    Return:
        Nothing
-------------------------------------------------------------------*/
D_BOOL DclTestBeginTest(
    DclTestCase_t  * ptestcase)
{
    /* Default to success, any failure will change this... */
    ptestcase->testResult = DCL_TESTRESULT_PASS;

    return TRUE;
}


/*-------------------------------------------------------------------
    Protected: DclTestEndTest

    A routine to be called at the end of each test case.

    Parameters:
        testcase - The current test case context.

    Return:
        Nothing.
-------------------------------------------------------------------*/
void DclTestEndTest(
    DclTestCase_t  * ptestcase)
{
    (void)ptestcase;
    return;
}


/*-------------------------------------------------------------------
    Protected: DclTestDoAssertTest

    Log a test assertion.
    This is called to log the results for an assertion as part of a test
    case.  By the time this routine gets control, the assertion test will
    be complete, and the result parameter will contain a proper result code.

    Parameters:
        testResult   - The assertion result code.
                           Typically, this will be pass, fail or abort.
        fileName       - The name of the file containing the assertion.
        lineNumber - The line number within the file containing the assertion.
        comment    - The test case comment field.
        expression  - The expression which was tested in the assertion
        bugNumber - The bugzilla number for the abort or skip done.
                           This will always be logged if sent in.
        testcase      - A pointer to the testcase being recorded.

    Return:
        Nothing.

-------------------------------------------------------------------*/
void DclTestDoAssertTest(
    DclTestResult_t testResult,
    char           *szFileName,
    int             nLineNumber,
    char           *szComment,
    char           *szExpression,
    char           *szBugInfo,
    DclTestCase_t  *ptestcase)
{
    if(szBugInfo)
    {
        TestLogOutput(DCL_TESTLOG_CHECK, testResult, "  (%s) \"Bug: %s\"\n",
               szExpression, szBugInfo);
    }
    else
    {
        TestLogOutput(DCL_TESTLOG_CHECK, testResult, "  (%s:%d) \"%s\" - %s\n",
           szFileName, nLineNumber, szExpression, szComment);
    }

    ptestcase->testChecks++;

    switch(testResult)
    {
        case DCL_TESTRESULT_PASS:
        {
            ptestcase->testChecksPassed++;
            break;
        }
        case DCL_TESTRESULT_FAIL:
        {
            ptestcase->testChecksFailed++;
            break;
        }
        case DCL_TESTRESULT_ABORT:
        {
            ptestcase->testChecksAborted++;
            break;
        }
        case DCL_TESTRESULT_SKIP:
        {
            ptestcase->testChecksSkipped++;
            break;
        }
        default:
        {
            break;
        }
    }

    if(testResult != DCL_TESTRESULT_PASS)
    {
        ptestcase->testResult = testResult;
    }

    return;
}


/*-------------------------------------------------------------------
    Public: DclTestCreateLibrary

    Creates a test harness library object

    Parameters:
        szName           - the name by which the library will be referenced
        log_type          - one of text, html or xml (not all may be implemented)
        szLogFileName - the name of the log file, or NULL to use the default

    Return:
        Returns a pointer to the Library object or NULL if no memory
        exists or an error is shown.

    Note:  This object is the root of a test object tree.  Typically
    there will be only one of these.  Note:  We're creating the name
    for the global mutex using the address of the mutex structure to
    insure uniqueness.  If the uniqueness assumption fails for any
    reason, testing may hang in mysterious ways.
-------------------------------------------------------------------*/
DclTestLibrary_t * DclTestCreateLibrary(
    DCLINSTANCEHANDLE   hDclInst,
    char               *szName,
    char               *szLogFileName)
{
    DclTestLibrary_t   *plibSelf;
    char                szNameBuffer[32];

    DclAssert(hDclInst);

    /* validate the parameters
     */
    if(szName == NULL || DclStrLen(szName) == 0 )
    {
        return NULL;
    }

    TestFrameworkSetupCheck();

    /* Allocate a zeroed object, kick for home if we don't get it.
     */

    plibSelf = (DclTestLibrary_t * )DclMemAllocZero( sizeof(*plibSelf) );
    if(plibSelf == NULL )
    {
        return NULL;
    }

    plibSelf->hDclInst = hDclInst;

    /*  Create the modify mutex and acquire it.  Create a unique
        name for this, as a NULL name will assert, and we only
        ignore asserts during the test run.
    */
    DclSNPrintf( szNameBuffer, 31, "M%06X", plibSelf );
    plibSelf->modificationMutex = DclMutexCreate( szNameBuffer );
    if(plibSelf->modificationMutex == NULL)
    {
        return NULL;
    }

    DclMutexAcquire( plibSelf->modificationMutex );

    /* Create the update Mutex.
     */
    DclSNPrintf( szNameBuffer, 31, "U%06X", plibSelf );
    plibSelf->updateMutex = DclMutexCreate( szNameBuffer );
    if(plibSelf->updateMutex == NULL)
    {
        DclMutexRelease( plibSelf->modificationMutex );
        DclMutexDestroy( plibSelf->modificationMutex );
        return NULL;
    }

    /* Fill in the fields.  Most of these are zeroed already, but
       I'm just a touch paranoid about field initialization.
    */
    plibSelf->next                = NULL;
    plibSelf->ulSuiteCount        = 0;
    plibSelf->pSuiteListHead      = NULL;
    plibSelf->pSuiteListTail      = NULL;

    plibSelf->libraryGlobals      = NULL;
    plibSelf->ulThreadsComplete   = 0;

    DclStrNCpy( plibSelf->szName, szName, DCLT_STR_LEN);
    TestClearStatistics( &plibSelf->testStatistics );

    if(szLogFileName == NULL)
    {
        DclStrNCpy( plibSelf->szLogFileName, "Default.log", DCLT_STR_LEN);
    }
    else
    {
        DclStrNCpy( plibSelf->szLogFileName, szLogFileName, DCLT_STR_LEN);
    }
    DclMutexRelease( plibSelf->modificationMutex );

    return plibSelf;
}


/*-------------------------------------------------------------------
    Public: DclTestCreateSuite

    Creates a test harness suite object

    Parameters:
        szName         - the name by which the suite will be referenced
        fSetupPtr       - A pointer to a function that will initialize the test environment for
                      this Suite
        fTeardownPtr - A pointer to a function that will deallocate the items created by
                      the setup function

    Return:
        Returns a pointer to the Suite object or NULL if no memory exists.

    Note: This is the generic container for test case objects.  Its
    job is to organize the test cases for use by the executor
    functions.
-------------------------------------------------------------------*/
DclTestSuite_t * DclTestCreateSuite(
    char                *szName,
    PFNDCL_TESTSETUP     pfnSetup,
    PFNDCL_TESTTEARDOWN  pfnTeardown  )
{
    DclTestSuite_t * ptestsuiteSelf;

    /* Validate parameters
     */
    if(szName == NULL || DclStrLen(szName) == 0)
    {
        return NULL;
    }

    TestFrameworkSetupCheck();

    /* Allocate a zeroed object, kick for home if we don't get it.
     */
    ptestsuiteSelf = (DclTestSuite_t *)DclMemAllocZero(sizeof(DclTestSuite_t));
    if(ptestsuiteSelf != NULL )
    {
        DclStrNCpy( ptestsuiteSelf->szName, szName, DCLT_STR_LEN);
        ptestsuiteSelf->pLibraryParent  = NULL;
        ptestsuiteSelf->pNext           = NULL;
        ptestsuiteSelf->fSetupPtr       = pfnSetup;
        ptestsuiteSelf->fTeardownPtr    = pfnTeardown;

        ptestsuiteSelf->testcaseHead    = NULL;
        ptestsuiteSelf->testcaseTail    = NULL;
        ptestsuiteSelf->ulTestcaseCount = 0;

        TestClearStatistics( &ptestsuiteSelf->testStatistics );
    }
    return ptestsuiteSelf;
}

/*-------------------------------------------------------------------
    Public: DclTestAddSuiteToLibrary

    Associates a suite object to a libaray object

    Parameters:
        library - the library which will contain the new group
        suite   - the new suite to be added to the library

    Return:
        Returns nothing

    Note: Add a suite to the specified library.  This will fail with a
    log entry if the suite already has a parent object or if any of
    the objects is NULL.
-------------------------------------------------------------------*/
void DclTestAddSuiteToLibrary(
    DclTestLibrary_t * plibrary,
    DclTestSuite_t   * psuite)
{

    if(plibrary == NULL || psuite == NULL)
    {
        return;
    }

    DclMutexAcquire( plibrary->modificationMutex );
    psuite->pLibraryParent = plibrary;

    if(plibrary->ulSuiteCount == 0 )
    {
        plibrary->ulSuiteCount  = 1;
        plibrary->pSuiteListHead = psuite;
        plibrary->pSuiteListTail = psuite;
        DclMutexRelease( plibrary->modificationMutex );
        return;
    }

    plibrary->ulSuiteCount ++;
    plibrary->pSuiteListTail->pNext = psuite;
    plibrary->pSuiteListTail        = psuite;
    psuite->pNext                   = NULL;

    DclMutexRelease( plibrary->modificationMutex );
    return;
}



/*-------------------------------------------------------------------
    Public: DclTestAddCasesToSuite

    Creates a series of test case objects and adds the objects to a suite object

    Parameters:
        suite        - The parent suite for the new test case
        testcases - The array of test case descriptors used to generate the test case objects

    Return:
        Returns nothing

    Note:  This procedure is one of a few that appends an element to a
    list on one of the objects.
-------------------------------------------------------------------*/
void DclTestAddCasesToSuite(
    DclTestSuite_t       *psuite,
    DclTestDescriptor_t  *rgtestcases )
{
    int             nCurrent;
    DclTestCase_t  *pcurrentTestcase;

    if(psuite == NULL || rgtestcases == NULL)
    {
        return;
    }

    for (nCurrent=0; rgtestcases[nCurrent].szName != NULL; nCurrent ++)
    {
        pcurrentTestcase = TestCreateCase(
            rgtestcases[nCurrent].szTestID,
            rgtestcases[nCurrent].szName,
            rgtestcases[nCurrent].ulParam,
            rgtestcases[nCurrent].pSetupData,
            rgtestcases[nCurrent].fFunctionPtr);

        if(pcurrentTestcase == NULL )
        {
            break;
        }

        DclTestAddCaseToSuite( psuite, pcurrentTestcase );
    }

    return;
}


/*-------------------------------------------------------------------
    Public: DclTestDestroyLibrary

    Destroy the supplied test library and all of the contained objects

    Parameters:
        library - The library object in question.

    Return:
        Returns nothing

    Note:  Delete the library and all of the enclosed objects.  In order
    for this to work properly, all testing MUST BE COMPETE.  The results
    of any destroy operation while testing is going on is UNDEFINED and
    will probably crash.
-------------------------------------------------------------------*/
void DclTestDestroyLibrary(
    DclTestLibrary_t  *plibrary )
{
    DclTestSuite_t    *psuite;

    if(plibrary == NULL)
    {
        return;
    }

    /* If there are any suites, then delete them.
     */
    if(plibrary->ulSuiteCount != 0)
    {
        for(psuite = plibrary->pSuiteListHead;
              psuite != NULL; )
        {
            DclTestSuite_t    * current = psuite->pNext;
            DclTestDestroySuite( psuite );
            psuite = current;
        }
    }

    DclMutexDestroy( plibrary->modificationMutex );
    DclMutexDestroy( plibrary->updateMutex );
    DclMemFree( plibrary );
    return;
}


/*-------------------------------------------------------------------
    Public: DclTestDestroySuite

    Destroy the supplied test suite.

    Parameters:
        suite - The suite to be destroyed.

    Return:
        Returns nothing

    Note:  Delete the  supplied test Suite and any contained testcases.
    In order for this to work properly, all testing MUST BE COMPETE.
    The results of any destroy operation while testing is going on is
    UNDEFINED and will probably crash.
-------------------------------------------------------------------*/
void DclTestDestroySuite(
    DclTestSuite_t   *psuite)
{
    DclTestCase_t    *ptestcase;

    if(psuite == NULL)
    {
        return;
    }

    for(ptestcase = psuite->testcaseHead; ptestcase != NULL; )
    {
        DclTestCase_t * current = ptestcase->pNext;
        TestDestroyCase( ptestcase );
        ptestcase = current;
    }

    DclMemFree(psuite);
    return;
}


/*-------------------------------------------------------------------
    Public: DclTestFRunTestLibrary

    Run the supplied test library.
    Execute the testcases from the entire test library. Usually, the
    caller will be some automated test application.  This could be
    sophisticated enough to run small groups of tests to handle bug
    regression and the like.

    Parameters:
        library - The library object in question.  This contains at least
            one group or suite. In theory, the group or suite contains at
            least one testcase.

    Return:
        Nothing
-------------------------------------------------------------------*/
void DclTestRunTestLibrary(
    DclTestLibrary_t  *plibrary)
{
    DclTestSuite_t    *psuite;

    if(plibrary == NULL || plibrary->ulSuiteCount == 0)
    {
        return;
    }

    TestLogOutput(DCL_TESTLOG_INFO, DCL_TESTRESULT_NA, "Library start: %s\n", plibrary->szName);

    TestMarkTestsBeginning( &plibrary->testStatistics );

    /*  If there are any suites, then run them.
    */
    if(plibrary->ulSuiteCount != 0)
    {
        for (psuite  = plibrary->pSuiteListHead;
             psuite != NULL;
             psuite  = psuite->pNext )
        {
            psuite->hDclInst = plibrary->hDclInst;

            TestRunTestSuite(psuite);
        }
    }

    TestMarkTestsEnding( &plibrary->testStatistics );

    TestLogOutput(DCL_TESTLOG_INFO, DCL_TESTRESULT_NA, "Library stop: %s\n", plibrary->szName);

    TestWriteReport(DCL_TESTREPORT_LIB, plibrary->szName, &plibrary->testStatistics );

    return;
}


/*-------------------------------------------------------------------
    Public: DclTestAddCaseToSuite

    Associates a test case object to a suite object

    Parameters:
        suite      - The parent suite for the new test case
        testcase - The new test case going into the suite

    Return:
        Returns nothing

    Note:  This procedure is one of a few that appends an element to
    a list on one of the objects.
-------------------------------------------------------------------*/
void DclTestAddCaseToSuite(
    DclTestSuite_t  *psuite,
    DclTestCase_t   *ptestcase )
{
    if(psuite->pLibraryParent != NULL )
    {
        DclMutexAcquire(psuite->pLibraryParent->modificationMutex);
    }

    if(psuite->ulTestcaseCount == 0 )
    {
        psuite->ulTestcaseCount  = 1;
        psuite->testcaseHead     = ptestcase;
        psuite->testcaseTail     = ptestcase;
        ptestcase->pNext         = NULL;
        ptestcase->pSuiteParent  = psuite;

        if(psuite->pLibraryParent != NULL)
            DclMutexRelease(psuite->pLibraryParent->modificationMutex);

        return;
    }

    ++psuite->ulTestcaseCount;
    psuite->testcaseTail->pNext = ptestcase;
    psuite->testcaseTail        = ptestcase;
    ptestcase->pNext            = NULL;
    ptestcase->pSuiteParent     = psuite;

    if(psuite->pLibraryParent != NULL )
    {
        DclMutexRelease(psuite->pLibraryParent->modificationMutex);
    }
    return;
}


/*-------------------------------------------------------------------
    Local: TestLogOutput

    Captures a line of output and displays it formatted for
	test loged output.

    Paramters:

    Return:
-------------------------------------------------------------------*/
void DclTestLogOutput(
    char               *szFormat,
    ...)
{
#if DCLCONF_OUTPUT_ENABLED
    va_list arglist;

    va_start(arglist, szFormat);

    TestLogOutput(DCL_TESTLOG_OUT, DCL_TESTRESULT_INVALID, szFormat, arglist);

    va_end(arglist);
#else
    (void)szFormat;
#endif
}


/*-------------------------------------------------------------------
    Local: TestFrameworkSetupCheck

    Complete any one time initialization necessary to use the
    framework.  Note this is called during the create of a suite
    or library.

    Parameters:
        None

    Return:
        Nothing
-------------------------------------------------------------------*/
static void TestFrameworkSetupCheck(void)
{
    if(gAssertList.pmutexLock == NULL)
    {
        gAssertList.pmutexLock = DclMutexCreate("DclTest");
    }

    return;
}


/*-------------------------------------------------------------------
    Local: TestDestroyCase

    Destroy the supplied testcase.

    Parameters:
        testcase - The suite to be destroyed.

    Return:
        Returns nothing

    Note:  Delete the supplied test case.  In order for this to work
    properly, all testing MUST BE COMPETE.  The results of any destroy
    operation while testing is going on is UNDEFINED and will probably crash.
-------------------------------------------------------------------*/
static void TestDestroyCase(
    DclTestCase_t    *ptestcase)
{
    if(ptestcase == NULL )
    {
        return;
    }

    DclMemFree( ptestcase );
    return;
}


/*-------------------------------------------------------------------
    Local: TestClearStatistics

    Clear the supplied statistics object to a starting state

    Parameters:
        statistics - The statistics object in question

    Return:
        Returns nothing

    Note: Clears the run time and test case statisitcs to a beginning
    state. Currently that state is zero.
-------------------------------------------------------------------*/
static void TestClearStatistics(
    DclTestStats_t  *pstats)
{
    DclMemSet(&pstats->timestampStart, 0, sizeof pstats->timestampStart);
    pstats->ulMsTimeTotal   = 0;

    pstats->ulTestsTried    = 0;
    pstats->ulTestsPassed   = 0;
    pstats->ulTestsFailed   = 0;
    pstats->ulTestsAborted  = 0;
    pstats->ulTestsSkipped  = 0;
    pstats->ulChecksDone    = 0;
    pstats->ulChecksPassed  = 0;

    return;
}


/*-------------------------------------------------------------------
    Local: TestCreateCase

    Create a testcase object.

    Parameters:
        szName         - The name of the testcase.  This should be unique for all
                      testcases in the current library.  The maximum length for this
                      item is DCLT_STR_LEN
        szDescription - The description of the current testcase.  The maximum length
                      for this item is TEST_DESC_LENGTH.
        fFunctionPtr   - The function for this testcase.  If this is passed in as NULL, the
                      testcase will be marked as not_implemented on execution.

    Return:
        Returns a pointer of type DclTesttest case * which refers to the testcase object.

    Note: This is the primary testcase creator.  It is called by
    DclTestAddCasesToSuite to create the test cases it builds and adds
    to the suite.
-------------------------------------------------------------------*/
static DclTestCase_t *TestCreateCase(
    char                *szTestID,
    char                *szName,
    D_UINT32             ulTestCaseParam,
    DclTestCaseData_t   *pTestcaseData,
    PFNDCL_TESTCASE      pfnTestcase )
{
    DclTestCase_t * ptestcaseSelf;

        /* Verify our parameters
         */
    if(szName == NULL || szTestID == NULL )
    {
        return NULL;
    }

        /* Allocate a zeroed object, kick for home if we don't get it.
         */
    ptestcaseSelf = (DclTestCase_t * )DclMemAllocZero(sizeof(DclTestCase_t));
    if(ptestcaseSelf == NULL )
    {
        return NULL;
    }

    /*  Fill in the fields, most of these will start as zero, fill them
        in explicitly anyways.
    */
    DclStrNCpy( ptestcaseSelf->szName, szName, DCLT_STR_LEN);

    ptestcaseSelf->pNext             = NULL;
    ptestcaseSelf->pSuiteParent      = NULL;
    ptestcaseSelf->ulTestCaseParam   = ulTestCaseParam;
    ptestcaseSelf->pSetupData        = pTestcaseData;
    ptestcaseSelf->pTestcase         = pfnTestcase;
    ptestcaseSelf->szTestID          = szTestID;

    ptestcaseSelf->ulFailureCount    = 0;
    ptestcaseSelf->pFailuresHead     = NULL;
    ptestcaseSelf->pFailuresTail     = NULL;

    ptestcaseSelf->ulMajorErrorFlag  = 0;

    ptestcaseSelf->testResult        = DCL_TESTRESULT_NOT_RUN;
    ptestcaseSelf->testBegun         = FALSE;
    ptestcaseSelf->testChecks        = 0;
    ptestcaseSelf->testChecksFailed  = 0;
    ptestcaseSelf->testChecksAborted = 0;
    ptestcaseSelf->testChecksPassed  = 0;
    ptestcaseSelf->testChecksSkipped = 0;

    DclMemSet(&ptestcaseSelf->timestampStart, 0,
            sizeof ptestcaseSelf->timestampStart);
    ptestcaseSelf->ulMsTimeTestRun   = 0;

    return ptestcaseSelf;
}


/*-------------------------------------------------------------------
    Local: TestMarkTestsBeginning

    Mark the begin time in the statistics block specified.

    Parameters:
        statistics - The library/group/suite statistics block

    Return:
        Returns nothing

    Note: A convienence procedure to mark the item start time.  This
    procedure requires that the update mutex be acquired before entry.
    It has no way to verify that this has been done.
-------------------------------------------------------------------*/
static void TestMarkTestsBeginning(
    DclTestStats_t  * pstats)
{
    pstats->timestampStart = DclTimeStamp();
    return;
}


/*-------------------------------------------------------------------
    Local: TestMarkTestsEnding

    Mark the end time and update the duration in the statistics block specified.

    Parameters:
        statistics - The library/group/suite statistics block

    Return:
        Returns nothing

    Note: A convienence procedure to mark the item end time and
    duration.  This procedure requires that the update mutex be
    acquired before entry.  It has no way to verify that this has been
    done.
-------------------------------------------------------------------*/
static void TestMarkTestsEnding(
    DclTestStats_t  * pstats)
{
    pstats->ulMsTimeTotal = DclTimePassed(pstats->timestampStart);

    return;
}

/*-------------------------------------------------------------------
    Local: TestMarkTestCaseSkipped

    Run the supplied testcase, taking information from the supplied thes suite.

    Parameters:
        library   - The current library pointer.
        testcase - The testcase we're marking skipped.

    Return:
        Returns nothing

    Note: A shortcut procedure to mark the a given testcase as
    "Skipped".  This acquires & releases the library updateMutex.
-------------------------------------------------------------------*/
static void TestMarkTestCaseSkipped(
    DclTestCase_t * ptestcase)
{
    DclMutexAcquire(ptestcase->pSuiteParent->pLibraryParent->updateMutex);

    ptestcase->ulMsTimeTestRun = 0;
    ptestcase->testResult        = DCL_TESTRESULT_SKIP;

    TestUpdateTestStats(&ptestcase->pSuiteParent->testStatistics, ptestcase );
    TestUpdateTestStats(&ptestcase->pSuiteParent->pLibraryParent->testStatistics,
                                                                 ptestcase );
    DclMutexRelease( ptestcase->pSuiteParent->pLibraryParent->updateMutex);
    return;
}


/*-------------------------------------------------------------------
    Local: TestRunTestCase

    Run the given test case. Executes a single test case specified
    by the caller. Usually, the caller will be RuntestSuite, but a
    command-line utility could locate and specify a single test
    case to execute. This would be very useful for bug regression
    and system debugging. This acquires and releases the update mutex.

    Parameters:
        testcase - The test case to be run.  The runner needs
        information from the suite object, and will handle the updates
        as necessary.

    Return:
        Returns nothing
-------------------------------------------------------------------*/
static void TestRunTestCase(
    DclTestCase_t      *ptestcase)
{
    DclTestHandle_t     hTest = NULL;
    DclTestSuite_t     *psuite = ptestcase->pSuiteParent;
    DclTestLibrary_t   *plibrary = ptestcase->pSuiteParent->pLibraryParent;
    DCLDECLARESERVICE   (ServiceHdr, "TESTASSERT", DCLSERVICE_ASSERT, TestAssertIoctl, NULL, DCLSERVICEFLAG_THREADSAFE);
    DCLSTATUS           dclStat;

    /*  The following need to be declared volatile since they may be
        modified between a setjmp and longjmp.
    */
    volatile DclTestResult_t    resultSetup      = DCL_TESTRESULT_ABORT;
    volatile DclTestResult_t    resultTestcase   = DCL_TESTRESULT_ABORT;
    volatile DclTestResult_t    resultTeardown   = DCL_TESTRESULT_ABORT;

    AssertContext_t     assertContext;

    DclAssert(ptestcase->hDclInst);

    TestLogOutput(DCL_TESTLOG_INFO, DCL_TESTRESULT_NA, "  Test start: %s %s\n",
            ptestcase->szTestID, ptestcase->szName);

    /*  Otherwise, proceed with setup, test and teardown.  All of this is
        done under the control of the update Mutex.

        If the testBegun field is set, then some thread must be running
        this or have already finished it.  If so, release the updateMutex
        and return immediately.  If not, then set the flag ourselves and
        proceed to run the test.  Under no circumstances should the
        testBegin flag be written to except under the control of the
        updateMutex once testing has started.
    */
    DclMutexAcquire( plibrary->updateMutex );

    if(ptestcase->testBegun == TRUE)
    {
        DclMutexRelease( plibrary->updateMutex );
        return;
    }

    ptestcase->timestampStart    = DclTimeStamp();
    ptestcase->ulMsTimeTestRun   = 0;
    ptestcase->testResult        = DCL_TESTRESULT_INVALID;
    ptestcase->testBegun         = TRUE;

    /*  Replace the assert service with our own
    */
    dclStat = DclServiceCreate(ptestcase->hDclInst, &ServiceHdr);

    DclMutexRelease(plibrary->updateMutex);

    if(dclStat != DCLSTAT_SUCCESS)
        return;

    if(setjmp(assertContext.jumpBuffer))
    {
        resultTestcase = DCL_TESTRESULT_ABORT;
        goto Teardown;
    }

    TestContextAddAssert(&assertContext);

    /*  If the test case has a setup, then invoke it.  If the setup
        function returns anything other than a pass condition then
        we have a major error and cannot proceed with the test.
    */
    if(psuite->fSetupPtr != NULL)
    {
        resultSetup = psuite->fSetupPtr(&hTest, ptestcase->ulTestCaseParam,
                                ptestcase->pSetupData, ptestcase);
        if(resultSetup != DCL_TESTRESULT_PASS)
        {
            goto Teardown;
        }
    }
    else
    {
        /* Successful setup (nothing to do) */
        resultSetup = DCL_TESTRESULT_PASS;
    }

    /*  Second, run the test function.  Abort or skip here are just results.
        That is, we'll continue to the teardown regardless of the result of
        this phase of the test case
    */
    /*  flag this as not implemented if the procedure pointer is NULL
    */
    if(ptestcase->pTestcase != NULL)
    {

        resultTestcase = ptestcase->pTestcase(&hTest,
            ptestcase->ulTestCaseParam, ptestcase->pSetupData, ptestcase);

        /* Check the memory pool for consistency */
        if(DclMemTrackPoolVerify(NULL, TRUE) != DCLSTAT_SUCCESS)
        {
            resultTestcase = DCL_TESTRESULT_FAIL;
        }
    }
    else
    {
        resultTestcase  = DCL_TESTRESULT_NOT_IMPLEMENTED;
    }

    /*  Third, run the teardown function.  If this aborts, then we need to
        tell the psuite that it must abort the rest of the tests, as we
        can't reset the test parameters.
    */
Teardown:
    if(psuite->fTeardownPtr != NULL)
    {
        if(resultSetup == DCL_TESTRESULT_PASS)
        {
            resultTeardown = psuite->fTeardownPtr(
                &hTest, ptestcase->ulTestCaseParam,
                ptestcase->pSetupData, ptestcase);
        }
    }
    else
    {
        resultTeardown = DCL_TESTRESULT_PASS;
    }

    /*  Finally, we notify the upstream callers of the test results.
    */
    DclMutexAcquire(plibrary->updateMutex);

    /* Remove the saved context and assert function */
    TestContextRemoveAssert(&assertContext);

    dclStat = DclServiceDestroy(&ServiceHdr);

/*  Should really do something here...
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;
*/

    ptestcase->ulMsTimeTestRun = DclTimePassed(ptestcase->timestampStart);

    /*  Log the test results and record it in the test case.
    */
    if(resultSetup != DCL_TESTRESULT_PASS)
    {
        /* Failed setup means nothing else was run
        */
        TestLogOutput(DCL_TESTLOG_RESULT, resultSetup,
                "%s %s \"Test setup failed!\"\n",
                ptestcase->szTestID, ptestcase->szName);

        ptestcase->testResult = resultSetup;
    }
    else
    {
        /*  If the teardown was not successful make sure it's known
        */
        if(resultTeardown != DCL_TESTRESULT_PASS)
        {
            TestLogOutput(DCL_TESTLOG_INFO, resultTeardown,
                    "\"Warning: teardown unsuccessful, failing test case...\"\n");

            ptestcase->testResult = resultTeardown;
        }
        else
        {
            ptestcase->testResult = resultTestcase;
        }

        /*  Successful setup, log result of the test case
        */
        TestLogOutput(DCL_TESTLOG_RESULT, ptestcase->testResult, "%s %s\n",
            ptestcase->szTestID, ptestcase->szName);
    }

    TestUpdateTestStats(&psuite->testStatistics,   ptestcase);
    TestUpdateTestStats(&plibrary->testStatistics, ptestcase);
    DclMutexRelease(plibrary->updateMutex);

    TestLogOutput(DCL_TESTLOG_INFO, DCL_TESTRESULT_NA, "  Test stop: %s %s\n",
            ptestcase->szTestID, ptestcase->szName);

    return;
}


/*-------------------------------------------------------------------
    Local: TestRunTestSuite

    Run the supplied test suite.
    Execute the testcases from a single test suite.  Usually,
    the caller will be RunTestLibrary or RunTestGroup, but a
    command-line utility could locate and specify a single testcase to
    execute.  This would be very useful for bug regression and system
    debugging.

    Parameters:
        library - The library object containing the suite in question.
            This contains the information needed for logging and other
            topics.
        suite   - The suite containing the testcases to be run.  If
            any of them fail to initialize, or fail to teardown
            properly, then the remaining testcases will be skipped.

    Return:
        Returns nothing

-------------------------------------------------------------------*/
static void TestRunTestSuite(
    DclTestSuite_t     *psuite)
{
    DclTestCase_t      *ptestcase;
    D_UINT32            ulMajorErrorFlag = 0;

    if(psuite == NULL || psuite->ulTestcaseCount == 0)
    {
        return;
    }

    DclAssert(psuite->hDclInst);

    /*  We're either running testcases or marking them skipped.  Marking
        skipped only happens when either the setup or teardown functions
        exist and fail to execute without error.
    */
    TestLogOutput(DCL_TESTLOG_INFO, DCL_TESTRESULT_NA, "  Suite start: %s\n", psuite->szName);
    TestMarkTestsBeginning( &psuite->testStatistics );

    for(ptestcase = psuite->testcaseHead;
        ptestcase != NULL;
        ptestcase=ptestcase->pNext )
    {
        if(ulMajorErrorFlag == 0 )
        {
            ptestcase->hDclInst = psuite->hDclInst;

            TestRunTestCase( ptestcase );
            ulMajorErrorFlag = ptestcase->ulMajorErrorFlag;
        }
        else
        {
            TestMarkTestCaseSkipped( ptestcase );
        }
    }

    TestMarkTestsEnding(&psuite->testStatistics);

    TestLogOutput(DCL_TESTLOG_INFO, DCL_TESTRESULT_NA, "  Suite stop: %s\n", psuite->szName);

    TestWriteReport(DCL_TESTREPORT_STE, psuite->szName, &psuite->testStatistics);

    return;
}


/*-------------------------------------------------------------------
    Local: TestUpdateTestStats

    Reports the results of a test case to the statistics block

    Parameters:
        pStatistics - The statistics block being updated by this function.
        testcase    - The testcase results to be logged

    Return:
        Returns nothing

    Note: Update the status counters in the statistics object.
-------------------------------------------------------------------*/
static void TestUpdateTestStats(
    DclTestStats_t * pstats,
    DclTestCase_t  * ptestcase )
{
    /*  First, make sure we have a stat structure ...
       */
    if(pstats == NULL || ptestcase == NULL)
    {
        return;
    }

    /*  Second, update the block
       */
    pstats->ulChecksDone     += ptestcase->testChecks;
    pstats->ulChecksPassed   += ptestcase->testChecksPassed;
    pstats->ulChecksFailed   += ptestcase->testChecksFailed;
    pstats->ulChecksSkipped  += ptestcase->testChecksSkipped;
    pstats->ulChecksAborted  += ptestcase->testChecksAborted;
    pstats->ulMsTimeTotal    += ptestcase->ulMsTimeTestRun;
    pstats->ulTestsTried     += 1;

    switch(ptestcase->testResult)
    {
        case DCL_TESTRESULT_PASS:
            pstats->ulTestsPassed ++;
            break;
        case DCL_TESTRESULT_FAIL:
            pstats->ulTestsFailed ++;
            break;
        case DCL_TESTRESULT_SKIP:
            pstats->ulTestsSkipped ++;
            break;
        case DCL_TESTRESULT_ABORT:
            pstats->ulTestsAborted ++;
            break;
        case DCL_TESTRESULT_NOT_IMPLEMENTED:
            pstats->ulTestsNotImplemented ++;
            break;

        case DCL_TESTRESULT_NOT_RUN:
        default:
            break;
    }
    return;
}


/*-------------------------------------------------------------------
    Local: TestWriteReport

    Write a short report for the end of Suite, Group or Library

    Parameters:
        szLevel     - One of the strings "Library", "Group" or "Suite".  These are not
                    checked, they are simply output as part of the report title
        szName    - The name of the thing being reported
        pStatistics - The statistics block being reported for.

    Return:
        Returns nothing

    Note: Dumps a summary for the test level being finished.
-------------------------------------------------------------------*/
static void TestWriteReport(
    DclTestReport_t type,
    char *szName,
    DclTestStats_t *pstats)
{
    char *szPrefix;
    char *szResult;
    D_UINT32  ulHours;
    D_UINT32  ulMinutes;
    D_UINT32  ulSeconds;
    D_UINT32  ulMilliseconds;
    D_UINT32  ulTotalMs;
#define TIME_STRING_LEN  14
    char      szDurationString[TIME_STRING_LEN];

    ulTotalMs = pstats->ulMsTimeTotal;


    ulMilliseconds  = ulTotalMs % 1000;
    ulTotalMs      /= 1000;
    ulSeconds       = ulTotalMs % 60;
    ulTotalMs      /= 60;
    ulMinutes       = ulTotalMs % 60;
    ulTotalMs      /= 60;
    ulHours         = ulTotalMs / 60;
    DclSNPrintf(szDurationString, TIME_STRING_LEN, "%02lU:%02lU:%02lU.%03lU",
            ulHours, ulMinutes, ulSeconds, ulMilliseconds);

    switch(type)
    {
        case DCL_TESTREPORT_LIB:
        {
            szPrefix = "[TST] lib  ";
            break;
        }
        case DCL_TESTREPORT_STE:
        {
            szPrefix = "[TST] ste  ";
            break;
        }
        default:
        {
            szPrefix = "[TST]      ";
            break;
        }
    }

    if(pstats->ulTestsFailed || pstats->ulTestsAborted)
    {
        szResult = " *** FAILED ***";
    }
    else
    {
        szResult = " PASSED";
    }

    TestPrintf("%s *******************************************************\n", szPrefix);
    TestPrintf("%s \"%s\": %s\n", szPrefix, szName, szResult);
    TestPrintf("%s \"Duration\": %s\n", szPrefix, szDurationString);
    TestPrintf("%s *******************************************************\n", szPrefix);
    TestPrintf("%s                    Tests  Checks\n", szPrefix);
    TestPrintf("%s           Total:   %-5lU  %-5lU\n",  szPrefix, pstats->ulTestsTried, pstats->ulChecksDone);
    TestPrintf("%s          Passed:   %-5lU  %-5lU\n",  szPrefix, pstats->ulTestsPassed, pstats->ulChecksPassed);
    TestPrintf("%s         Skipped:   %-5lU  %-5lU\n",  szPrefix, pstats->ulTestsSkipped, pstats->ulChecksSkipped);
    TestPrintf("%s         Aborted:   %-5lU  %-5lU\n",  szPrefix, pstats->ulTestsAborted, pstats->ulChecksAborted);
    TestPrintf("%s        FAILURES:   %-5lU  %-5lU\n",  szPrefix, pstats->ulTestsFailed, pstats->ulChecksFailed);
    TestPrintf("%s Not Implemented:   %-5lU       \n",  szPrefix, pstats->ulTestsNotImplemented);
    TestPrintf("%s *******************************************************\n", szPrefix);
    TestPrintf("\n");

    return;
}


/*-------------------------------------------------------------------
    Local: TestPrintf

    The test framework's printf hook - all test output uses this API.

    Paramters:
        pszModuleName - The name of the source file throwing the assert.
        uLineNumber     - Line number within the file that failed the assert.

    Return:
        TRUE if the assert should be performed, otherwise FALSE.  In this
        simple routine, all asserts are written to the log file and ignored.

    Note:  The default for use by any test case.
-------------------------------------------------------------------*/
static void TestPrintf(
    const char  *pszFmt, ...)
{
#if DCLCONF_OUTPUT_ENABLED
    va_list         arglist;

    if(!pszFmt)
        return;

    va_start(arglist, pszFmt);

    DclVPrintf(pszFmt, arglist);

    va_end(arglist);
#else
    (void)pszFmt;
#endif
    return;
}


/*-------------------------------------------------------------------
    Local: TestLogOutput

    Captures a line of output and prepares a string for printing
    or logging.

    Paramters:

    Return:
-------------------------------------------------------------------*/
static void TestLogOutput(
    DclTestType_t       type,
    DclTestResult_t     result,
    char               *szPostFormat,
    ...)
{
#define LOG_STR_LEN  120
    char    szPreFormat[LOG_STR_LEN];
    char    *szType;
    char    *szResult;
#if DCLCONF_OUTPUT_ENABLED
    va_list arglist;
#endif

    switch(type)
    {
        case DCL_TESTLOG_ASSERT:
            szType = "[TST] assert  ";
            break;
        case DCL_TESTLOG_CHECK:
            szType = "[TST] check   ";
            break;
        case DCL_TESTLOG_RESULT:
            szType = "[TST] test    ";
            break;
        case DCL_TESTLOG_OUT:
            szType = "[TST] log     ";
            break;
        case DCL_TESTLOG_INFO:
            szType = "[TST] info    ";
            break;
        default:
            szType = "[TST] error   ";
            break;
    }

    switch(result)
    {
        case DCL_TESTRESULT_PASS:
            szResult = "     Passed      ";
            break;
        case DCL_TESTRESULT_FAIL:
            szResult = "  ***FAILED***   ";
            break;
        case DCL_TESTRESULT_ABORT:
            szResult = "     Aborted     ";
            break;
        case DCL_TESTRESULT_SKIP:
            szResult = "     Skipped     ";
            break;
        case DCL_TESTRESULT_NOT_RUN:
            szResult = "     Not Run     ";
            break;
        case DCL_TESTRESULT_NOT_IMPLEMENTED:
            szResult = " Not Implemented ";
            break;
        case DCL_TESTRESULT_INVALID:
            szResult = "     UNKNOWN     ";
            break;
        default:
            szResult = "    ";
            break;
    }

    DclSNPrintf(szPreFormat, LOG_STR_LEN, "%s%s%s", szType, szResult, szPostFormat);

#if DCLCONF_OUTPUT_ENABLED

    va_start(arglist, szPostFormat);

    DclVPrintf(szPreFormat, arglist);

    va_end(arglist);
#else
    (void)szPostFormat;
#endif
    return;
}


/*-------------------------------------------------------------------
    Local: TestContextAddAssert

    Parameters:
        testcase - The current test case.

    Return:
        Nothing
-------------------------------------------------------------------*/
static void TestContextAddAssert(AssertContext_t *pAssertContext)
{
    DclAssert(pAssertContext);

    DclMutexAcquire(gAssertList.pmutexLock);

    /* Initialize the given assert context */
    pAssertContext->pPrev = NULL;
    pAssertContext->nAssertCount = 0;
    pAssertContext->threadId = DclOsThreadID();

    /* Place the assert context at the head of the list */
    if(gAssertList.pHead)
    {
        pAssertContext->pNext = gAssertList.pHead;
        gAssertList.pHead->pPrev = pAssertContext;
    }
    else
    {
        pAssertContext->pNext = NULL;
    }
    gAssertList.pHead = pAssertContext;
    ++gAssertList.nContextCount;



    DclMutexRelease(gAssertList.pmutexLock);
}


/*-------------------------------------------------------------------
    Local: TestContextRemoveAssert

    Parameters:
        testcase - The current test case.

    Return:
        Nothing
-------------------------------------------------------------------*/
static void TestContextRemoveAssert(AssertContext_t *pAssertContext)
{
    DclAssert(pAssertContext);

    DclMutexAcquire(gAssertList.pmutexLock);

    if(pAssertContext == gAssertList.pHead)
    {
        gAssertList.pHead = pAssertContext->pNext;
    }

    if(pAssertContext->pNext)
    {
        pAssertContext->pNext->pPrev = pAssertContext->pPrev;
    }
    if(pAssertContext->pPrev)
    {
        pAssertContext->pPrev->pNext = pAssertContext->pNext;
    }

    --gAssertList.nContextCount;

    DclMutexRelease(gAssertList.pmutexLock);
}


/*-------------------------------------------------------------------
    Local: TestContextReportAssert

    Parameters:
        testcase - The current test case.

    Return:
        Nothing
-------------------------------------------------------------------*/
static void TestContextReportAssert(void)
{
    DCLTHREADID tid;
    unsigned nCount;
    AssertContext_t *pTemp;
    D_BOOL fJump;

    DclMutexAcquire(gAssertList.pmutexLock);

    /* Find this assert context */
    tid = DclOsThreadID();
    pTemp = gAssertList.pHead;
    for(nCount = 0; nCount < gAssertList.nContextCount; ++nCount)
    {
        DclAssert(pTemp);
        if(pTemp->threadId == tid)
        {
            break;
        }

        pTemp = pTemp->pNext;
    }

    fJump = FALSE;
    if(pTemp)
    {
        /* Update the assert count and set our jump flag if neccessary */
        nCount = ++pTemp->nAssertCount;
        if(nCount >= gAssertList.nMaxAsserts)
        {
            fJump = TRUE;
        }
    }

    DclMutexRelease(gAssertList.pmutexLock);

    if(fJump)
    {
        /* Jump back into the test case error handler */
        longjmp(pTemp->jumpBuffer, 1);
    }

    return;
}


/*-------------------------------------------------------------------
    Local: TestAssertIoctl()

    Parameters:
        hService - The service handle
        pReq     - A pointer to the DCLREQ_SERVICE structure
        pPriv    - A pointer to any private data defined when the
                   service was created.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestAssertIoctl(
    DCLSERVICEHANDLE        hService,
    DCLREQ_SERVICE         *pReq,
    void                   *pPriv)
{
    DclAssert(hService);
    DclAssert(pReq);
    DclAssert(!pPriv);

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

                    TestLogOutput(DCL_TESTLOG_ASSERT, DCL_TESTRESULT_FAIL,
                        "  (%s:%u) \"FAILED PRODUCT ASSERT!\"\n", pSubReq->pszModuleName, pSubReq->nLineNumber);

                    /*  Report the assert - this may not return
                    */
                    TestContextReportAssert();

                    return DCLSTAT_SUCCESS;
                }

                default:
                    DCLPRINTF(1, ("TestAssert:ServiceIoctl() Unsupported subfunction %x\n", pReq->pSubRequest->ioFunc));
                    return DCLSTAT_SERVICE_UNSUPPORTEDREQUEST;
            }
        }

        default:
            DCLPRINTF(1, ("TestAssert:ServiceIoctl() Unhandled request %x\n", pReq->ior.ioFunc));
            return DCLSTAT_SERVICE_UNHANDLEDREQUEST;
    }
}



