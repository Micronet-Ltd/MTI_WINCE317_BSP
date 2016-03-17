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
    $Log: dltharness.h $
    Revision 1.16  2010/04/28 23:31:30Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.15  2009/08/06 23:16:11Z  garyp
    Updated to make the DCL instance handle available to the test code.
    Revision 1.14  2009/06/28 01:16:58Z  garyp
    Updated to replace the assert functionality using a service.
    Revision 1.13  2009/04/08 19:17:56Z  garyp
    Updates for AutoDoc -- no functional changes.
    Revision 1.12  2009/03/12 21:40:47Z  keithg
    Removed trailing comma from enum lists to placate a number of
    compilers that complain.
    Revision 1.11  2009/02/19 02:35:10Z  keithg
    Added type for printing formatted C strings within the test context.
    Revision 1.10  2009/02/16 07:22:39Z  keithg
    Changed timimg statistics to correctly use the timestamp and time
    passed functionality.
    Revision 1.9  2009/02/05 18:34:29Z  keithg
    Removed obsolete prototypes and added test report types.
    Revision 1.8  2009/02/01 22:44:33  keithg
    Test case variable setup now handles a NULL pointer; increased test case
    string length to allow for larger test name descriptions.
    Revision 1.7  2009/01/26 12:49:13  keithg
    - Removed 'group' concept to simplify test structures and error handling.
    - Wholesale changes to variable namespace to fullfill hungarian
      notations and coding standards.
    - Consolidated output to DclTestLogOutput() and reformated text
      to be a bit more user friendly and consistent.
    - Updated comments to match the current implementation.
    Revision 1.6  2009/01/21 10:37:57  keithg
    Removed grouping feature, now just libraries and suites.
    Revision 1.5  2009/01/20 21:03:54  keithg
    More corrections, variable clarifications, fixed prototype, and removed
    unused macro declarations.
    Revision 1.4  2009/01/15 14:36:35  keithg
    Corrected labels and macro function calls introduced in the last checkin;
    and added test case data pointer and 32-bit parameter for each test case
    description.
    Revision 1.3  2009/01/14 18:10:21Z  keithg
    Updated to use the new DclTestHandle_t; Removed unused prototypes.
    Revision 1.2  2009/01/09 02:52:06Z  keithg
    Corrected macro names, removed unused macros.
    Revision 1.1  2009/01/09 00:50:24Z  keithg
    Initial revision
---------------------------------------------------------------------------*/
#ifndef DLTHARNESS_H_INCLUDED
#define DLTHARNESS_H_INCLUDED

#ifndef DCL_H_INCLUDED
#error "dcl.h must be included before dltharness.h"
#endif

/*  DclTestHandle_t is an opaque handle to test-defined object.  The setup
    routine may allocate a block of memory or a single pointer-length value
    to be passed by this value.  The teardown routine is responsible for
    deallocating the block and anything it refers to.  The setup routine will
    recieve this as a pointer to a NULL pointer.  If the setup doesn't
    change it, then the teardown has little to do.
*/
typedef struct tagDclTestCaseData_s DclTestCaseData_t;
typedef struct tagDclTestData_s DclTestData_t;
typedef DclTestData_t *DclTestHandle_t;
typedef struct tagDclTestFailure_s DclTestFailure_t;
typedef struct tagDclTestStats_s DclTestStats_t;
typedef struct tagDclTestDescriptor_s DclTestDescriptor_t;
typedef struct tagDclTestLibrary_s DclTestLibrary_t;
typedef struct tagDclTestSuite_s DclTestSuite_t;
typedef struct tagDclTestCase_s DclTestCase_t;

/*  The possible results of a test/setup/teardown function.  At no point
    should these tags be considered to have any particular value either
    implicitly or in relation to one of the other values.

    Zero, in particular, is used as an invalid value, to detect incorrect
    use of the system.  If you're seeing "UNKNOWN" as a test result, you
    didn't start the test case function with a call to DCLTFBEGIN
*/
enum tagDclTestResult_e {
    DCL_TESTRESULT_INVALID = 0,
    DCL_TESTRESULT_NA,
    DCL_TESTRESULT_FAIL,
    DCL_TESTRESULT_ABORT,
    DCL_TESTRESULT_SKIP,
    DCL_TESTRESULT_NOT_IMPLEMENTED,
    DCL_TESTRESULT_PASS,
    DCL_TESTRESULT_NOT_RUN
};

enum tagDclTestType_e {
    DCL_TESTLOG_ASSERT = 1,
    DCL_TESTLOG_CHECK,
    DCL_TESTLOG_RESULT,
    DCL_TESTLOG_INFO,
    DCL_TESTLOG_OUT
};

typedef enum tagDclTestReport_e {
    DCL_TESTREPORT_LIB,
    DCL_TESTREPORT_STE
} DclTestReport_t;

typedef enum tagDclTestResult_e DclTestResult_t;
typedef enum tagDclTestType_e DclTestType_t;
/* typedef enum tagDclTestLogType_e DclTestLogType_t; */


/*  These are the types for the various test functions.  These will be used
    to strongly type the various test functions.
*/
typedef DclTestResult_t (*PFNDCL_TESTSETUP)(
    DclTestHandle_t*, D_UINT32, DclTestCaseData_t*, DclTestCase_t*);
typedef DclTestResult_t (*PFNDCL_TESTTEARDOWN)(
    DclTestHandle_t*, D_UINT32, DclTestCaseData_t*, DclTestCase_t*);
typedef DclTestResult_t (*PFNDCL_TESTCASE)(
    DclTestHandle_t*, D_UINT32, DclTestCaseData_t*, DclTestCase_t*);

typedef DclTestResult_t DECL_DCLTESTCASE(
        DclTestHandle_t*, D_UINT32, DclTestCaseData_t*, DclTestCase_t*);

/*  The test case name type. Allocate one extra character for the
    terminating zero string terminator.
*/
#define DCLT_STR_LEN  60
typedef char DCLT_STRING[DCLT_STR_LEN+1];

/*  The test case Descriptor, used to add a batch of tests to a suite.

    The szName field gives the name of a test case.  A future implementation
    may allow a single test to be executed, these should be unique within
    a test suite.

    The pSetupData field a pointer-sized item, used to pass data into a
    testcase.  This allows a single set of code to test a number of related
    use cases by supplying one or more parameters via this value.

    The function pointer refers to the entry point of the test function. A
    test function uses a standard set of parameters to gain access to test
    state as created by the setup function, and to enable access to the
    reporting and test state variables.
*/
struct tagDclTestDescriptor_s {
    char                *szTestID;
    char                *szName;
    D_UINT32            ulParam;
    DclTestCaseData_t   *pSetupData;
    PFNDCL_TESTCASE     fFunctionPtr;
};

#define DCLTFTEST_END { NULL, NULL, 0, NULL, NULL }



/*  Test Failure Block
    This holds the data for a test failure.  Instances of this block
    will be held as a linked list from the DclTestCase_t structure.
    It will be filed in regardless of the logging vebosity, but will
    only be displayed or logged if the user requests it.
*/
struct tagDclTestFailure_s {
    DclTestFailure_t *next;
    char *szFileName;
    int iLineNumber;
    DclTestResult_t testResult;
    char *szCheckString;
};

/* The test statistics for a run.  This object is kept for each suite,
   and library.  Most fields are maintained by the function
   DclTestUpdateTestStats() which is called for each of the structures
   at the end of a test case execution.
*/
struct tagDclTestStats_s {
    DCLTIMESTAMP timestampStart;
    D_UINT32     ulMsTimeTotal;

    D_UINT32     ulTestsTried;
    D_UINT32     ulTestsPassed;
    D_UINT32     ulTestsFailed;
    D_UINT32     ulTestsAborted;
    D_UINT32     ulTestsSkipped;
    D_UINT32     ulTestsNotImplemented;

    D_UINT32     ulChecksDone;
    D_UINT32     ulChecksPassed;
    D_UINT32     ulChecksSkipped;
    D_UINT32     ulChecksFailed;
    D_UINT32     ulChecksAborted;
};


/* The internal structure for a test case.

   The szTestID is a short (currently 7 character) string used
   to identify the test case in the log and from the command
   line (a feature coming soon).  The szName field is a short
   (about 40 characters) description of the test case.  This
   will not be searchable, but is intended to make the log file
   and reports easier to read.

   The current goal is to maintain the test cases as a linked
   list linked to a container object (a test suite).  The test
   case knows its parent group, which gives it access to suite
   properties for updating the test result statistics.  Since
   the test case knows about its parent, it is responsible for
   updating statistics in the higher level structures.

   The failure list holds the results for any check which did not
   show a result of "Pass".  It is possible for a test case to show
   either "Aborted" or "Skipped" without this list being populated.
   Typically, this occurs when the suite setup routine fails.  At
   that point, the test being run is Aborted, and the other tests
   in the suite are skipped.

   The item "testResult" holds the result of the test.  If the
   result value is "not_run", then the case has not been executed
   in the current run.  Any of the other results means that the
   test case has been run or attempted and the value reflects
   the success or failure of the test case.

   The testBegun field must be set to TRUE under the control of the
   library global mutex.  Once this is set, no thread may reset it.
   If a thread attempts to execute this testcase, and testBegun is
   already set, then the runner should exit immediately, without
   making further changes to this structure.

   The majorErrorFlag is a mode flag stating that the setup or teardown
   functions failed in some way.  This is considered catastrophic for the
   suite, and will cause all of the remaining tests in the suite to be
   skipped.
*/
struct tagDclTestCase_s
{
    char               *szTestID;
    DCLT_STRING         szName;

    DclTestSuite_t     *pSuiteParent;

    DclTestFailure_t   *pFailuresHead;
    DclTestFailure_t   *pFailuresTail;

    D_UINT32            ulFailureCount;
    D_UINT32            ulMajorErrorFlag;

    D_UINT32            ulTestCaseParam;
    DclTestCaseData_t  *pSetupData;

    PFNDCL_TESTCASE     pTestcase;
    DclTestResult_t     testResult;
    D_BOOL              testBegun;

    D_UINT32            testChecks;
    D_UINT32            testChecksPassed;
    D_UINT32            testChecksFailed;
    D_UINT32            testChecksAborted;
    D_UINT32            testChecksSkipped;

    DCLINSTANCEHANDLE   hDclInst;
    DCLTIMESTAMP        timestampStart;
    D_UINT32            ulMsTimeTestRun;

    DclTestCase_t      *pNext;
};


/* The internal structure for a test suite.

   The main points of this structure are:
   1) The list of test cases in the testcase<Head|Tail|Count> block.
      These are scanned in order to execute the suite and again to
      report the suite's results (in verbose mode). At the end of a
      run, the testsTried and testcaseCount fields should be equal.
   2) The <group|library> parent fields are used to hold pointers
      to the container object(s) for this suite.  If the pGroupParent
      field is non-NULL, then we belong to a group, and should report
      results to the group.  The pLibraryParent field will always be
      set, and we alway report results to the library.
   3) The block of test results is gathered during a run, and output
      as part of the final test report.
*/
struct tagDclTestSuite_s
{
    DclTestSuite_t     *pNext;
    DclTestLibrary_t   *pLibraryParent;

    DCLINSTANCEHANDLE   hDclInst;

    DCLT_STRING         szName;
    PFNDCL_TESTSETUP    fSetupPtr;
    PFNDCL_TESTTEARDOWN fTeardownPtr;

    DclTestCase_t      *testcaseHead;
    DclTestCase_t      *testcaseTail;
    D_UINT32            ulTestcaseCount;

    DclTestStats_t      testStatistics;
};

/* The internal structure for a test library.  The library consists of
   a set of test suites (carried in a linked list through the next ptrs
   in the suite structure), a similar set of test groups, the test status
   values for the run, the test timing data for the run and the run start
   and end times.

   The thread model for this system is fairly simple and quite rigid.  The
   test cases, suites and groups may be added to a library in any order, by
   any number of threads.  However, once testing has begun, no modifications
   to the test structure are allowed (and no internal checks can be made to
   prevent this in the general case).
*/
struct tagDclTestLibrary_s {
    DclTestLibrary_t      *next;
    DCLT_STRING            szName;

    DCLINSTANCEHANDLE       hDclInst;

    DclTestSuite_t        *pSuiteListHead;
    DclTestSuite_t        *pSuiteListTail;
    D_UINT32               ulSuiteCount;

    DclTestStats_t         testStatistics;

    /* Logging Information */
    DCLT_STRING       szLogFileName;

    /* The library-global mutex.  All changes to the task structure are run through
       the modificationMutex.  Once execution starts, the modification mutex is
       acquired and not released until the ulThreadsComplete has reached its
       goal value. This is to prevent further changes to the structure during
       test execution.  Note, attempting to change the test structure will hang
       the thread without notice, and is probably not what you want.

       The thread model for this looks like:
       Master Thread (starts test threads and runs no tests iteself, waits for
                      the ulThreadsComplete counter to reach the threadCount)
          Test Thread 1 - runs to completion and increments ulThreadsComplete
                          under the comtrol of the updateMutex when it's
                          done.
          Test Thread 2 - same as Test Thread 1.

       The updateMutex latches the data structure during test execution so that
       test results may be recorded and so that the testBegun flag can mark a test
       as started.  The goal for this is to block multiple threads from executing
       a single test and trying to be the last one to record a result.

       There are no mutex controls for the updates for the AssertHook logic. Adding
       a mutex to this would be done in the assert logic.  It would require a single
       thread initialization phase (to create the mutex) and would cause tests to
       be single threaded (to acquire the mutex, set the hook and run the test,
       then back everything out).
    */
    PDCLMUTEX             modificationMutex;
    PDCLMUTEX             updateMutex;
    D_UINT32              ulThreadsComplete;

    /* Current Information for the executing suite. */
    void                      * libraryGlobals;
};


DclTestLibrary_t * DclTestCreateLibrary(
    DCLINSTANCEHANDLE hDclInst,
    char *szName,
    char *szLogFileName);
void DclTestDestroyLibrary(
    DclTestLibrary_t *library);
DclTestSuite_t * DclTestCreateSuite(
    char *szName,
    PFNDCL_TESTSETUP pfnSetupPtr,
    PFNDCL_TESTTEARDOWN pfnTeardownPtr);
void DclTestDestroySuite(
    DclTestSuite_t *psuite);
void DclTestAddCasesToSuite(
    DclTestSuite_t *psuite,
    DclTestDescriptor_t *ptestcases);
void DclTestAddCaseToSuite(
    DclTestSuite_t *psuite,
    DclTestCase_t *ptestcase );
void DclTestAddSuiteToLibrary(
    DclTestLibrary_t *plibrary,
    DclTestSuite_t *psuite);
void DclTestRunTestLibrary(
    DclTestLibrary_t *plibrary);
void DclTestDoAssertTest(
    DclTestResult_t  testResult,
    char               *fileName,
    int                 lineNumber,
    char               *comment,
    char               *expression,
    char               *bugNumber,
    DclTestCase_t      *testcase);
D_BOOL DclTestBeginTest(
    DclTestCase_t     * testcase);
void DclTestEndTest(
    DclTestCase_t     * testcase);

/*  The test case asserts and the header and footer logic for a case.

    The DCLTFVARAIBLES(testcase) define defines and initializes some
    variables needed by the assertion #defines.  The parameter is
    the testcase parameter in the calling function.  DCLTFVARAIBLES needs
    to be included before the first executable statement in each testcase,
    setup and teardown function.  Note:  if you add a variable to this
    define, it is critical that you leave the final ';' off of the last
    statement.  Failing to do so will result in hundreds of syntax errors
    and warnings.

    DCLTFBEGIN() is the starting point of the testcase execution.  Private
    initialization code (parsing the pSetupData parameter for instance),
    is done before this statement.

    DCLTFEND defines the end of the testcase logic.  A testcase ABORT is sent
    to this point.  Any code after DCLTFEND should be minor cleanup (freeing
    memory, for instance).  Major cleanup operations should be done in the
    teardown function.

    DCLTFCOMPLETE defines the end of the function, and returns the test result
    to the caller.  Code after this statement will cause an error.
*/


/*-------------------------------------------------------------------
    Protected: DCLTFVARIABLES()

    Parameters:

    Return Value:

    See Also:
-------------------------------------------------------------------*/
#define DCLTFVARIABLES(ptestcase)                       \
        DclTestCase_t   * DCLTFTestcase = ptestcase;    \
        DclTestSuite_t  * DCLTFSuite                    \
            = ptestcase ? DCLTFTestcase->pSuiteParent : NULL;


/*-------------------------------------------------------------------
    Protected: DCLTFBEGIN()

    Parameters:

    Return Value:

    See Also:
-------------------------------------------------------------------*/
#define DCLTFBEGIN                                      \
    if(!DclTestBeginTest(DCLTFTestcase))                \
        goto _DCLTFEndOfTest;


/*-------------------------------------------------------------------
    Protected: DCLTFEND()

    Parameters:

    Return Value:

    See Also:
-------------------------------------------------------------------*/
#define DCLTFEND                                        \
    _DCLTFEndOfTest:                                    \
    DclTestEndTest(DCLTFTestcase);


/*-------------------------------------------------------------------
    Protected: DCLTFCOMPLETE()

    Parameters:

    Return Value:

    See Also:
-------------------------------------------------------------------*/
#define DCLTFCOMPLETE                                   \
    ((void)DCLTFSuite);                                 \
    return DCLTFTestcase->testResult;


/*-------------------------------------------------------------------
    Protected: DCLTFASSERT()

    Parameters:

    Return Value:

    See Also:
-------------------------------------------------------------------*/
#define DCLTFASSERT(test,comment)                       \
    DclTestDoAssertTest( (DclTestResult_t)              \
        ((test) ? DCL_TESTRESULT_PASS : DCL_TESTRESULT_FAIL),       \
                       __FILE__,                        \
                       __LINE__,                        \
                       (comment),                       \
                       #test,                           \
                       NULL,                            \
                       DCLTFTestcase )


/*-------------------------------------------------------------------
    Protected: DCLTFASSERT_SKIP()

    Parameters:

    Return Value:

    See Also:
-------------------------------------------------------------------*/
#define DCLTFASSERT_SKIP(test,comment)                  \
    DclTestDoAssertTest(  (DclTestResult_t)             \
        ((test) ? DCL_TESTRESULT_PASS : DCL_TESTRESULT_SKIP),       \
                       __FILE__,                        \
                       __LINE__,                        \
                       (comment),                       \
                       #test,                           \
                       NULL,                            \
                       DCLTFTestcase )


/*-------------------------------------------------------------------
    Protected: DCLTFASSERT_ABORT()

    Parameters:

    Return Value:

    See Also:
-------------------------------------------------------------------*/
#define DCLTFASSERT_ABORT(test,comment)                 \
    {                                                   \
        D_BOOL DCLTFiTemp = (D_BOOL)(test);             \
        DclTestDoAssertTest((DclTestResult_t)           \
        (DCLTFiTemp ? DCL_TESTRESULT_PASS : DCL_TESTRESULT_ABORT),  \
                       __FILE__,                        \
                       __LINE__,                        \
                       (comment),                       \
                       #test,                           \
                       NULL,                            \
                       DCLTFTestcase );                 \
        if (!DCLTFiTemp)                                \
            goto _DCLTFEndOfTest;                       \
    }


/*-------------------------------------------------------------------
    Protected: DCL_TESTRESULT_PASS()

    Parameters:

    Return Value:

    See Also:
-------------------------------------------------------------------*/
#define DCLTFPASS(comment)                              \
    {                                                   \
        DclTestDoAssertTest(DCL_TESTRESULT_PASS,              \
                          __FILE__,                     \
                          __LINE__,                     \
                          comment,                      \
                          "Declared pass",              \
                           NULL,                        \
                          DCLTFTestcase );              \
        goto _DCLTFEndOfTest;                           \
    }


/*-------------------------------------------------------------------
    Protected: DCL_TESTRESULT_FAIL()

    Parameters:

    Return Value:

    See Also:
-------------------------------------------------------------------*/
#define DCLTFFAIL(comment)                              \
    {                                                   \
        DclTestDoAssertTest(DCL_TESTRESULT_FAIL,              \
                      __FILE__,                         \
                      __LINE__,                         \
                      comment,                          \
                      "Failed",                         \
                       NULL,                            \
                      DCLTFTestcase );                  \
        goto _DCLTFEndOfTest;                           \
    }


/*-------------------------------------------------------------------
    Protected: DCLTF_NOT_IMPLEMENTED()

    Parameters:

    Return Value:

    See Also:
-------------------------------------------------------------------*/
#define DCLTF_NOT_IMPLEMENTED()                              \
    {                                                   \
        DclTestDoAssertTest(DCL_TESTRESULT_NOT_IMPLEMENTED, \
                      __FILE__,                         \
                      __LINE__,                         \
                      "",                          \
                      "",                         \
                       NULL,                            \
                      DCLTFTestcase );                  \
        goto _DCLTFEndOfTest;                           \
    }


/*-------------------------------------------------------------------
    Protected: DCL_TESTRESULT_FAIL_ON_BUG()

    Parameters:

    Return Value:

    See Also:
-------------------------------------------------------------------*/
#define DCLTFFAIL_ON_BUG(bugNumber, comment)            \
    {                                                   \
        DclTestDoAssertTest(DCL_TESTRESULT_FAIL,              \
                      __FILE__,                         \
                      __LINE__,                         \
                      comment,                          \
                      "failed",                         \
                      #bugNumber,                       \
                      DCLTFTestcase );                  \
         goto _DCLTFEndOfTest;                          \
     }


/*-------------------------------------------------------------------
    Protected: DCLTFSKIP()

    Parameters:

    Return Value:

    See Also:
-------------------------------------------------------------------*/
#define DCLTFSKIP( comment )                            \
    {                                                   \
        DclTestDoAssertTest(DCL_TESTRESULT_SKIP,              \
                      __FILE__,                         \
                      __LINE__,                         \
                      comment,                          \
                      "Skipped",                        \
                       NULL,                            \
                      DCLTFTestcase );                  \
         goto _DCLTFEndOfTest;                          \
     }


/*-------------------------------------------------------------------
    Protected: DCLTFSKIP_ON_BUG()

    Parameters:

    Return Value:

    See Also:
-------------------------------------------------------------------*/
#define DCLTFSKIP_ON_BUG( bugNumber, comment )          \
    {                                                   \
        DclTestDoAssertTest( DCL_TESTRESULT_SKIP,             \
                      __FILE__,                         \
                      __LINE__,                         \
                      comment,                          \
                      "Skipped",                        \
                      #bugNumber,                       \
                      DCLTFTestcase );                  \
        goto _DCLTFEndOfTest;                           \
    }


/*-------------------------------------------------------------------
    Protected: DCLTFABORT()

    Parameters:

    Return Value:

    See Also:
-------------------------------------------------------------------*/
#define DCLTFABORT(comment)                             \
    {                                                   \
        DclTestDoAssertTest(DCL_TESTRESULT_ABORT,             \
                      __FILE__,                         \
                      __LINE__,                         \
                      comment,                          \
                      "Aborted",                        \
                       NULL,                            \
                      DCLTFTestcase );                  \
        goto _DCLTFEndOfTest;                           \
     }


#endif /* DLTHARNESS_H_INCLUDED */




