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

    This module contains the DCL unit test code.

    ToDo:
      - Flesh out the string functions tests.
      - Flesh out the sprintf functionality test to do more than just test
        the string output.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltmain.c $
    Revision 1.69  2011/03/24 06:03:02Z  garyp
    Fixed to build with various and sundry arrogant compilers which whine
    about anything they cannot understand.
    Revision 1.68  2011/03/23 23:48:33Z  garyp
    Added a test for DclSortInsertion32().
    Revision 1.67  2010/12/02 13:47:10Z  garyp
    Added a test for double percent signs with sprintf().
    Revision 1.66  2010/10/11 20:22:51Z  glenns
    Fix bitmask in ECCHelper function that was causing the ECC test
    to never test bits in the upper nibble of the test bytes.
    Revision 1.65  2010/09/19 02:30:52Z  garyp
    Display the maximum timer sample period.
    Revision 1.64  2010/09/19 01:04:18Z  garyp
    Increased the argument buffer length.  Fixed documentation and 
    spelling errors.  
    Revision 1.63  2010/09/06 18:50:17Z  garyp
    Added a mutex test.
    Revision 1.62  2010/08/29 19:49:01Z  garyp
    Updated to use DclTimeFormatUS().
    Revision 1.61  2010/08/04 00:21:31Z  garyp
    Updated to work around the "VA64BUG" issue.  Added the /TIMERS
    option to run time related tests only.
    Revision 1.60  2010/05/10 18:56:30Z  garyp
    Updated to run the thread test all the time, rather than only when the
    /threads option was used (now obsolete).
    Revision 1.59  2010/04/21 20:24:15Z  garyp
    Updated the DclNanosecondDelay() test to exercise long (1 second) delays.
    Revision 1.58  2010/04/18 21:18:01Z  garyp
    Fixed the bit manipulation macro testing to work properly on big-endian
    targets.
    Revision 1.57  2010/01/07 02:35:25Z  garyp
    Enhanced the tests for the bit manipulation macros.
    Revision 1.56  2009/12/18 18:43:55Z  garyp
    Fixed a cut-and-paste error with the previous revision.
    Revision 1.55  2009/12/18 18:34:27Z  garyp
    Removed a test which A) could not be conditionally compiled out with
    old compilers, B) could not be compiled in with new compilers, and C)
    could not be executed on current embedded architectures.
    Revision 1.54  2009/12/18 17:11:48Z  jimmb
    Modified a pre-processed evaulation to a run time evaluation.  This is
    required for and older GNU version of the compiler which does not do the
    pre-process correctly.
    Revision 1.53  2009/12/12 04:40:16Z  garyp
    Added tests for the DCLBITGET/SET/CLEAR() macros.  Moved the "Alignment"
    tests in the "macros" test category.
    Revision 1.52  2009/11/18 01:43:43Z  garyp
    Added the /PROFILER switch (undocumented).
    Revision 1.51  2009/11/11 02:41:04Z  keithg
    Added basic test for DclRand() and DclRand64().
    Revision 1.50  2009/11/09 17:15:41Z  garyp
    Updated the TestBitOperations() test to not use a NULL random seed pointer.
    Updated to not call the High-Res timestamp functions directly, but rather 
    let the remapping feature do the work.
    Revision 1.49  2009/11/02 18:40:38Z  garyp
    Updated the test init code to fail immediately if the DCL instance is
    not already initialized.
    Revision 1.48  2009/10/09 17:56:38Z  garyp
    Added a DclMulDiv64() test case.  Corrected comments.
    Revision 1.47  2009/10/03 00:48:13Z  garyp
    Added ECC tests.  Fixed some broken format specifiers.
    Revision 1.46  2009/09/30 02:43:02Z  garyp
    Added tests for DclMulDiv64().   Changed the DclMulDiv() test to be table
    driven.  Added the /V - verbosity command-line option.
    Revision 1.45  2009/09/16 03:12:37Z  garyp
    Tentative fix for Bug #2842 -- the "Delay" test was not properly detecting
    that high-res tick functionality was not available.  Added a test for
    DclMulDiv().
    Revision 1.44  2009/07/07 17:31:54Z  keithg
    Corrected spelling errors.
    Revision 1.43  2009/06/30 21:54:09Z  garyp
    Corrected another occurrance where we were incorrectly using sizeof()
    instead of DCLDIMENSIONOF().
    Revision 1.42  2009/06/24 23:01:37Z  garyp
    Fixed a number of wide-character buffer calculations to use "dimensionof"
    rather than sizeof().  Added tests for threads, atomic operations, asserts,
    macros, bit manipulations, as well as a number of different memory
    management tests.  Enhanced the sleep test.  And finally, put the fun back
    in the code.
    Revision 1.41  2009/05/08 02:13:06Z  garyp
    Added UTF-8 tests.
    Revision 1.40  2009/02/08 02:42:17Z  garyp
    Merged from the v4.0 branch.  Added a math operations performance test.
    Revision 1.39  2009/01/19 20:59:06Z  johnb
    Added call to test multibyte/wide char test functions.
    Revision 1.38  2009/01/03 00:39:00Z  keithg
    Removed test for D_BOOL size.  Booleans are no longer in size.
    Revision 1.37  2008/08/20 00:11:19Z  keithg
    Added test for CRC16 function.
    Revision 1.36  2008/05/27 19:43:31Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.35  2008/05/05 20:00:22Z  garyp
    Added more tests for the DCLISALIGNED() macro.
    Revision 1.34  2008/04/21 15:04:09Z  garyp
    Enhanced the byte order test to verbosely display the memory byte, word,
    and dword order.  Updated to build cleanly when output is disabled.
    Revision 1.33  2008/03/05 05:18:12Z  Garyp
    Enhanced the "ticker" test.
    Revision 1.32  2008/01/30 03:01:25Z  Garyp
    Updated the Hamming test to test and display the Hamming codes on
    all 256 1-byte values.
    Revision 1.31  2007/11/03 23:31:12Z  Garyp
    Added the standard module header.
    Revision 1.30  2007/10/27 18:49:45Z  Garyp
    Added tests for bit operations.
    Revision 1.29  2007/10/16 20:09:26Z  Garyp
    Updated the HighRes ticker test to actually test the ticker and not the
    timestamp APIs.  Added tests for the "delay" services.
    Revision 1.28  2007/10/10 02:19:15Z  Garyp
    Removed an invalid high-res timestamp test.  Added a new high-res ticker
    test. Fixed the high-res timestamp test to ensure that regular low res
    timers are used to validate the results.
    Revision 1.27  2007/10/05 20:42:54Z  pauli
    Added memory tracking tests.
    Revision 1.26  2007/10/05 18:23:02Z  pauli
    Removed invalid tests that expected DCLALIGNEDBUFFER and DCLALIGNEDSTRUCT
    to result in an aligned buffer or structure size.
    Revision 1.25  2007/10/02 17:27:31Z  pauli
    Enhanced the misaligned access test to verify the data.
    Revision 1.24  2007/10/01 19:05:10Z  pauli
    Changed the type of index variables from signed to unsigned to be
    consistent with their usage.
    Revision 1.23  2007/08/01 22:52:59Z  Garyp
    Added tests for ANSI/Unicode conversions and some aspects of sprintf()
    functionality.
    Revision 1.22  2007/06/22 17:26:02Z  Garyp
    Modified the PerfLog stuff to use the new DCLPERFLOG_WRITE() functionality
    as well as handle the new "CategorySuffix" parameter.
    Revision 1.21  2007/06/10 22:05:30Z  keithg
    API change for CRC32 calculations, added test vector references added test
    cases for multi-buffer calls.
    Revision 1.20  2007/05/25 01:25:20Z  keithg
    Added test for the CRC module
    Revision 1.19  2007/05/13 16:51:56Z  garyp
    Added an option to run the "Compiler Bugs" test.
    Revision 1.18  2007/05/07 17:50:50Z  Garyp
    Modified to spit out more PerfLog information about system information.
    Revision 1.17  2007/04/03 19:13:44Z  Garyp
    Updated to support a PerfLog build number suffix.
    Revision 1.16  2007/02/27 23:00:13Z  Garyp
    Enhanced the ticker test and cleaned up the output.
    Revision 1.15  2007/02/11 01:46:04Z  Garyp
    Updated an error message.
    Revision 1.14  2007/01/28 01:05:12Z  Garyp
    Added PerfLog support.
    Revision 1.13  2007/01/18 22:58:02Z  Garyp
    Documented the /PERF switch.
    Revision 1.12  2006/11/02 20:45:09Z  Garyp
    Updated to eliminate a compiler warning when using the Green Hills tools.
    Revision 1.11  2006/10/05 23:31:51Z  Garyp
    Added a test for byte order.  Updated to use a properly renamed macro.
    Revision 1.10  2006/08/18 19:34:32Z  Garyp
    Added tests for DCLALIGNEDBUFFER() and DCLALIGNEDSTRUCT().
    Revision 1.9  2006/08/03 16:55:00Z  Garyp
    Added tests for misaligned data accesses.
    Revision 1.8  2006/07/06 00:43:20Z  Garyp
    Added tests for the high resolution ticker and timestamp.
    Revision 1.7  2006/03/27 12:04:43Z  Garyp
    Added new tests for memory functions.
    Revision 1.6  2006/03/14 23:59:08Z  Pauli
    Added path test.
    Revision 1.5  2006/02/28 23:21:53Z  Garyp
    Modified so the hamming tests only run when explicitly turned on.
    Revision 1.4  2006/02/26 20:57:15Z  Garyp
    Added hamming code tests.
    Revision 1.3  2006/02/03 00:41:59Z  Pauli
    Added tests for Date/Time functions.
    Revision 1.2  2005/12/28 00:32:57Z  Pauli
    Added 64-bit math and byte order tests.
    Revision 1.1  2005/10/21 00:58:26Z  Pauli
    Initial revision
    Revision 1.3  2005/10/21 01:58:25Z  garyp
    Enhanced the data type tests.
    Revision 1.2  2005/10/07 19:00:52Z  Garyp
    Minor output tweaks.
    Revision 1.1  2005/10/04 21:44:44Z  Garyp
    Initial revision
    Revision 1.4  2005/08/18 02:09:56Z  garyp
    Changed the sign-on message.
    Revision 1.3  2005/08/15 06:16:39Z  garyp
    Updated to call DclSignOn().
    Revision 1.2  2005/08/03 17:57:02Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/16 15:41:00Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlver.h>
#include <dltools.h>
#include <dlperflog.h>
#include "dltests.h"

#define TEST_MS_SNPRINTF        FALSE       /* FALSE for checkin */
#if TEST_MS_SNPRINTF
    #include <windows.h>
    #include <stdio.h>
    #undef DclSNPrintf
    #define DclSNPrintf _snprintf
#endif

#define NANOS_PER_SECOND    (1000000000UL)  /* must fit into a D_UINT32 */
#define PICOS_PER_NANO            (1000UL)  /* must fit into a D_UINT32 */
#define SEQHIGHRESCOUNT           (4000UL)  /* (Requires 8 bytes of memory per iteration) */
#define HIGHRESTESTSECONDS          (10UL)  /* number of seconds to run the high-res wrap test */
#define MATH_TEST_SECONDS            (2)    /* Seconds for each math test section */
#define MATH_OPS_PER_LOOP         (1000)

#define SPRINTFBUFFLEN  (32)
#define MAGICCHAR       'M'
#define MAGICCHAR2      'X'
#define MAGICWIDECHAR   'W'
#define MAGICWIDECHAR2  '2'

typedef struct
{
    unsigned    nVerbosity;
    unsigned    fHamming          : 1;
    unsigned    fTestClib         : 1;
    unsigned    fPerf             : 1;
    unsigned    fPerfLog          : 1;
    unsigned    fTestTimers       : 1;
    unsigned    fMemProtect       : 1;
    unsigned    fMemStress        : 1;
    unsigned    fTestMisaligned   : 1;
    unsigned    fTestCompilerBugs : 1;
    unsigned    fTestProfiler     : 1;
    char        szPerfLogSuffix[PERFLOG_MAX_SUFFIX_LEN];
} DCLTESTINFO;

struct TESTSTRUCT
{
    D_UINT64    ullTest;
    D_UINT8     ucTest;
};

static DCLSTATUS    ProcessParameters(DCLTOOLPARAMS *pTP, DCLTESTINFO *pTI);
static DCLSTATUS    TestDataTypes(DCLPERFLOGHANDLE hPerfLog);
static DCLSTATUS    TestMisaligned(void);
static DCLSTATUS    TestByteOrder(void);
static DCLSTATUS    TestMacros(void);
static DCLSTATUS    TestBitOperations(const DCLTESTINFO *pTI);
static DCLSTATUS    TestStringFuncs(void);
static DCLSTATUS    TestSprintf(void);
static DCLSTATUS    TestTickers(DCLTESTINFO *pTI);
static DCLSTATUS    TestTimers(void);
static DCLSTATUS    TestDelay(void);
static DCLSTATUS    TestSleep(void);
static DCLSTATUS    TestCrc32(void);
static DCLSTATUS    TestCrc16(void);
static DCLSTATUS    TestECC(void);
static DCLSTATUS    TestRandom(void);
static DCLSTATUS    TestHamming(void);
static DCLSTATUS    TestMulDiv(DCLTESTINFO *pTI);
static DCLSTATUS    TestMulDiv64(DCLTESTINFO *pTI);
static DCLSTATUS    TestInsertionSort(void);
static D_BOOL       HammingHelper(const void *pData, void *pWorkBuff, size_t length);
static D_UINT32     CalculatePeriod(D_UINT64 *pullMilliSecs, char **pBuffer);
static DCLSTATUS    SprintfHelper(const char *pszTitle, unsigned nBuffLen, const char *pszFmt, const char *pData, const char *pszResult, int iExpectedLen, unsigned nOverflowLen);
static DCLSTATUS    DelayHelper(D_UINT32 ulTotalMicrosecs, unsigned nIterations, D_UINT32 ulResolution, D_UINT32 ulSystemTicks);
static DCLSTATUS    ECCHelper(D_BUFFER *pBuff, size_t nLen);
static D_BOOL       BitArrayInitialize(void *pBuff, unsigned nBits, unsigned nSentinelBytes, unsigned nArrayBytes);
static D_BOOL       BitArrayValidate(  void *pBuff, unsigned nBits, unsigned nSentinelBytes, unsigned nArrayBytes, unsigned nElementBytes, unsigned nExpectedCount);

#if DCL_NATIVE_64BIT_SUPPORT
  static DCLSTATUS  TestMathPerformance(void);
  static DCLSTATUS  TestDivisionPerformance(D_UINT32 ulOverheadNS);
  static DCLSTATUS  TestMultiplicationPerformance(D_UINT32 ulOverheadNS);
  static DCLSTATUS  TestRightShiftPerformance(D_UINT32 ulOverheadNS);
  static DCLSTATUS  TestLeftShiftPerformance(D_UINT32 ulOverheadNS);
#endif

/*  CRC check string macros... These are used to validate the
    correctness of the CRC implementations
*/
#define CRC_CHECK_STRING     "123456789123456789"
#define CRC_CHECK_LENGTH_1    9
#define CRC_CHECK_LENGTH_2    18

/*  CRC-16 test  with public test vector from:
    http://www.zorc.breitbandkatze.de/crc.html
*/
#define CRC_CHECK_CRC16_1   0xBB3D
#define CRC_CHECK_CRC16_2   0xED7B

/*  CRC-32 test  with public test vector from:
    http://www.zorc.breitbandkatze.de/crc.html
*/
#define CRC_CHECK_CCITT32_1   0xCBF43926UL
#define CRC_CHECK_CCITT32_2   0x4B837AE4UL



/*-------------------------------------------------------------------
    Protected: DclTestMain()

    This function invokes the Datalight Common Libraries Unit Tests.

    Parameters:
        pTP - A pointer to the DCLTOOLPARAMS structure to use

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
DCLSTATUS DclTestMain(
    DCLTOOLPARAMS      *pTP)
{
    DCLSTATUS           dclStat;
    DCLTESTINFO         ti = {0};
    DCLPERFLOGHANDLE    hPerfLog = NULL;
    DCLTIMESTAMP        t;
    D_UINT64            ullTestUS;
    char                szTimeBuff[32];

    DclAssertWritePtr(pTP, sizeof(*pTP));

    /*  Ensure the DCL instance is already initialized before bothering
        to do anything else.
    */
    if(!pTP->hDclInst)
        return DCLSTAT_INST_NOTINITIALIZED;

    DclPrintf("DCL Unit Test\n");
    DclSignOn(FALSE);

    dclStat = ProcessParameters(pTP, &ti);
    if(dclStat == DCLSTAT_HELPREQUEST)
        return DCLSTAT_SUCCESS;
    if(dclStat != DCLSTAT_SUCCESS)
        goto Failed;

    /*---------------------------------------------------------
        Write data to the performance log, if enabled.  Note
        that any changes to the test name or category must be
        accompanied by changes to perfdcl.bat.  Any changes to
        the actual data fields recorded here requires changes
        to the various spreadsheets which track this data.
    ---------------------------------------------------------*/
    hPerfLog = DCLPERFLOG_OPEN(ti.fPerfLog, NULL, "DCL", "Configuration", NULL, ti.szPerfLogSuffix);
    DCLPERFLOG_STRING(hPerfLog, "CPUType",   DCLSTRINGIZE(D_CPUTYPE));
    DCLPERFLOG_STRING(hPerfLog, "OSName",    DCL_OSNAME);
    DCLPERFLOG_STRING(hPerfLog, "OSVersion", DCLSTRINGIZE(D_OSVER));
    DCLPERFLOG_STRING(hPerfLog, "ToolSet",   DCL_TOOLSETNAME);
    DCLPERFLOG_STRING(hPerfLog, "ByteOrder", DCL_BIG_ENDIAN ? "Big" : "Little");
    DCLPERFLOG_STRING(hPerfLog, "BuildNum",  DCLBUILDNUM);
    DCLPERFLOG_NUM(   hPerfLog, "DebugLev",  D_DEBUG);

    t = DclTimeStamp();

    if(ti.fTestCompilerBugs)
    {
        if((dclStat = DclTestCompilerBugs()) != DCLSTAT_SUCCESS)
            goto Failed;
    }

    if((dclStat = TestDataTypes(hPerfLog)) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = TestByteOrder()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = TestMacros()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = TestTickers(&ti)) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = TestTimers()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = TestDelay()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = TestSleep()) != DCLSTAT_SUCCESS)
        goto Failed;

    /*  If the /TIMERS switch was specified, test everything up to and
        including the time related stuff, then quit early.
    */        
    if(ti.fTestTimers)
        goto Success;

    if(ti.fTestMisaligned)
    {
        if((dclStat = TestMisaligned()) != DCLSTAT_SUCCESS)
            goto Failed;
    }

    if((dclStat = TestBitOperations(&ti)) != DCLSTAT_SUCCESS)
        goto Failed;

    if(ti.fTestProfiler)
    {
        /*  If testing the profiler is enabled, start the profiler and do
            its initial tests now (before the assert and memory tracking
            tests).  It will be disabled at the end of DCLTEST.
        */            
        if((dclStat = DclTestProfiler()) != DCLSTAT_SUCCESS)
            goto Failed;
    }

    if((dclStat = DclTestAssert()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = DclTestMemoryManagement(ti.fMemStress)) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = DclTestMemoryProtection(ti.fMemProtect)) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = DclTestMemoryValidation()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = DclTestMemFuncs(ti.fTestClib, ti.fPerf, ti.fPerfLog, ti.szPerfLogSuffix)) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = DclTest64BitMath()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = TestMulDiv(&ti)) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = TestMulDiv64(&ti)) != DCLSTAT_SUCCESS)
        goto Failed;

  #if DCL_NATIVE_64BIT_SUPPORT
    if(ti.fPerf && ((dclStat = TestMathPerformance()) != DCLSTAT_SUCCESS))
        goto Failed;
  #endif

    if((dclStat = DclTestByteOrderConversion()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = TestStringFuncs()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = TestSprintf()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = TestCrc32()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = TestCrc16()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = DclTestDateTime()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = DclTestPath()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = DclTestAtomicOperations()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = DclTestMutexes(ti.nVerbosity)) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = DclTestThreads()) != DCLSTAT_SUCCESS)
        goto Failed;

    if ((dclStat = DclTestUtf8()) != DCLSTAT_SUCCESS)
        goto Failed;

    if ((dclStat = DclTestMBWCConv()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = TestECC()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = TestRandom()) != DCLSTAT_SUCCESS)
        goto Failed;

    if((dclStat = TestInsertionSort()) != DCLSTAT_SUCCESS)
        goto Failed;

    if(ti.fHamming)
    {
        if((dclStat = TestHamming()) != DCLSTAT_SUCCESS)
            goto Failed;
    }
    else
    {
        DclPrintf("Hamming code tests skipped\n");
    }

    if(ti.fTestProfiler)
    {
        /*  Disable the profiler which would have been started earlier...
        */
        dclStat = DclProfDisable();
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("    Attempting to disable the profiler returned status %lX\n", dclStat);
            goto Failed;
        }
    }

  Success:

    DCLPERFLOG_WRITE(hPerfLog);
    DCLPERFLOG_CLOSE(hPerfLog);

    ullTestUS = DclTimePassedNS(t) / 1000;

    DclPrintf("Time elapsed:  %s\n", DclTimeFormatUS(ullTestUS, szTimeBuff, sizeof(szTimeBuff)));

  #if DCLCONF_MEMORYVALIDATION
    DclMemValSummary(pTP->hDclInst, NULL, FALSE, FALSE);
  #endif

    DclPrintf("DCL Unit Test PASSED\n\n");
    return DCLSTAT_SUCCESS;

  Failed:
    DCLPERFLOG_WRITE(hPerfLog);
    DCLPERFLOG_CLOSE(hPerfLog);
    DclPrintf("DCL Unit Test FAILED: Status %lX\n\n", dclStat);
    return dclStat;
}


/*-------------------------------------------------------------------
    Parses the command line arguments and sets any globals needed
    for the tests.  Also shows help via ShowUsage() if they need
    it.

    Parameters:
        pTP     - A pointer to the DCLTOOLPARAMS structure
        pTI     - A pointer to the DCLTESTINFO structure

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS ProcessParameters(
    DCLTOOLPARAMS  *pTP,
    DCLTESTINFO    *pTI)
{
    D_INT16         i, j;
    char            achArgBuff[256];
    D_UINT16        argc;

    pTI->nVerbosity = DCL_VERBOSE_NORMAL;

    argc = DclArgCount(pTP->pszCmdLine);

    for(i = 1; i <= argc; i++)
    {
        if(!DclArgRetrieve(pTP->pszCmdLine, i, DCLDIMENSIONOF(achArgBuff), achArgBuff))
        {
            DclPrintf("Bad argument!\n");
            return DCLSTAT_BADPARAMETER;
        }

        if(DclStrICmp(achArgBuff, "/hamm") == 0)
        {
            pTI->fHamming = TRUE;
            continue;
        }

        if(DclStrICmp(achArgBuff, "/clib") == 0)
        {
            pTI->fTestClib = TRUE;
            continue;
        }

        if(DclStrICmp(achArgBuff, "/timers") == 0)
        {
            pTI->fTestTimers = TRUE;
            continue;
        }

        if(DclStrICmp(achArgBuff, "/memstress") == 0)
        {
            pTI->fMemStress = TRUE;
            continue;
        }

        if(DclStrICmp(achArgBuff, "/memprotect") == 0)
        {
            pTI->fMemProtect = TRUE;
            continue;
        }

        if(DclStrICmp(achArgBuff, "/misaligned") == 0)
        {
            pTI->fTestMisaligned = TRUE;
            continue;
        }

        if(DclStrICmp(achArgBuff, "/perf") == 0)
        {
            pTI->fPerf = TRUE;
            continue;
        }

        if(DclStrICmp(achArgBuff, "/profiler") == 0)
        {
            pTI->fTestProfiler = TRUE;
            continue;
        }

        if(DclStrNICmp(achArgBuff, "/v:", 3) == 0)
        {
            pTI->nVerbosity = DclAtoI(&achArgBuff[3]);
            if(pTI->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            {
                DclPrintf("The verbosity level must range from 0 to 2\n");
                return DCLSTAT_BADPARAMETER;
            }

            continue;
        }

      #if DCLCONF_OUTPUT_ENABLED
        if(DclStrNICmp(achArgBuff, "/perflog", 8) == 0)
        {
            pTI->fPerf = TRUE;
            pTI->fPerfLog = TRUE;
            pTI->fTestClib = TRUE;

            if(achArgBuff[8] == ':')
            {
                DclStrNCpy(pTI->szPerfLogSuffix, &achArgBuff[9], sizeof(pTI->szPerfLogSuffix));
                pTI->szPerfLogSuffix[sizeof(pTI->szPerfLogSuffix)-1] = 0;
            }

          #if !DCLCONF_MEMORYTRACKING
            DclPrintf("WARNING: PerfLog output is being generated but DCLCONF_MEMORYTRACKING is FALSE!\n");
          #endif

            continue;
        }
      #endif

        if(DclStrICmp(achArgBuff, "/CompilerBugs") == 0)
        {
            pTI->fTestCompilerBugs = TRUE;
            continue;
        }

        j = 0;

        /*  Allow for multiple switches in a string
        */
        while(*(achArgBuff + j))
        {
            if(*(achArgBuff + j) == '/')
            {
                /*  Go to the next character to test
                */
                ++j;

                /*  Information specfic for each switch
                */
                switch (*(achArgBuff + j))
                {
                    case '?':
                        DclPrintf("\nSyntax:  %s [options]\n\n", pTP->pszCmdName);
                        DclPrintf("Where [options] are:\n");
                        DclPrintf("  /?             This help information\n");
                        DclPrintf("  /CLib          Test the C libary mem functions\n");
                        DclPrintf("  /CompilerBugs  Test known compiler bugs\n");
                        DclPrintf("  /Hamm          Run hamming code tests (may take a while)\n");
                        DclPrintf("  /MemProtect    Test the memory bounds even when DCLCONF_MEMTRACKING is off.\n");
                        DclPrintf("                 This is always tested when the feature is turned on, however\n");
                        DclPrintf("                 the test could cause a fault when the feature is off.\n");
                        DclPrintf("  /MemStress     Run memory manager stress tests (may take a long time)\n");
                        DclPrintf("  /Misaligned    Test misaligned data accesses (may fault!)\n");
                        DclPrintf("  /Perf          Run the performance tests\n");
                        DclPrintf("  /Timers        Test the time related stuff only, then quit early.\n");
                        DclPrintf("  /V:n           The verbosity level, from 0 to 2.  Defaults to 1.\n");
                      #if DCLCONF_OUTPUT_ENABLED
                        DclPrintf("  /PerfLog[:sfx] Output performance test results in CSV form, in addition to\n");
                        DclPrintf("                 the standard output.  Implies the /Perf and /CLib switches.\n");
                        DclPrintf("                 The optional 'sfx' value is a text suffix which will be\n");
                        DclPrintf("                 appended to the build number in the PerfLog CSV output.\n");
                      #endif

                        if(pTP->pszExtraHelp)
                            DclPrintf(pTP->pszExtraHelp);

                        return DCLSTAT_HELPREQUEST;

                    /*  Unrecognized switch
                    */
                    default:
                        DclPrintf("\nBad option: %s\n", (achArgBuff + j - 1));
                        return DCLSTAT_BADPARAMETER;
                }
            }
            else
            {
                DclPrintf("\nBad option: %s\n", (achArgBuff + j));
                return DCLSTAT_BADPARAMETER;
            }
        }
    }

    /*  All worked fine!
    */
    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    This test verifies that the basic data type sizes are correct.

    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestDataTypes(
    DCLPERFLOGHANDLE    hPerfLog)
{
    unsigned long       k;

    DclPrintf("Testing Data Type Sizes...\n");
    DclPrintf("    sizeof(char)          = %u\n", sizeof(char));
    DclPrintf("    sizeof(short)         = %u\n", sizeof(short));
    DclPrintf("    sizeof(int)           = %u\n", sizeof(int));
    DclPrintf("    sizeof(long)          = %u\n", sizeof(long));
    DclPrintf("    sizeof(pointer)       = %u\n", sizeof(void *));
    DclPrintf("    sizeof(DCL_ALIGNTYPE) = %u\n", sizeof(DCL_ALIGNTYPE));
    DclPrintf("    DCL_ALIGNSIZE         = %u\n", DCL_ALIGNSIZE);
    DclPrintf("    DCL_CPU_ALIGNSIZE     = %u\n", DCL_CPU_ALIGNSIZE);
  #if DCL_NATIVE_64BIT_SUPPORT
    DclPrintf("    64-bit types          Supported\n");
  #else
    DclPrintf("    64-bit types          Emulated\n");
  #endif

    DCLPERFLOG_NUM(hPerfLog,    "CharW",        sizeof(char));
    DCLPERFLOG_NUM(hPerfLog,    "ShortW",       sizeof(short));
    DCLPERFLOG_NUM(hPerfLog,    "IntW",         sizeof(int));
    DCLPERFLOG_NUM(hPerfLog,    "LongW",        sizeof(long));
    DCLPERFLOG_NUM(hPerfLog,    "PtrW",         sizeof(void *));
    DCLPERFLOG_NUM(hPerfLog,    "CompAlign",    DCL_ALIGNSIZE);
    DCLPERFLOG_NUM(hPerfLog,    "CPUAlign",     DCL_CPU_ALIGNSIZE);
  #if DCL_NATIVE_64BIT_SUPPORT
    DCLPERFLOG_STRING(hPerfLog, "64BitTypes",   "Supported");
  #else
    DCLPERFLOG_STRING(hPerfLog, "64BitTypes",   "Emulated");
  #endif

    /*  Test D_INT8 characteristics
    */
    {
        D_INT8          Int8;

        if(sizeof(D_INT8) != 1)
            return DCLSTAT_CURRENTLINE;

        Int8 = D_INT8_MIN;
        Int8--;
        if(Int8 != D_INT8_MAX)
            return DCLSTAT_CURRENTLINE;

        for(k=0; k<D_UINT8_MAX; k++)
            Int8--;

        if(Int8 != D_INT8_MIN)
            return DCLSTAT_CURRENTLINE;
    }

    /*  Test D_UINT8 characteristics
    */
    {
        D_UINT8         Uint8;

        if(sizeof(D_UINT8) != 1)
            return DCLSTAT_CURRENTLINE;

        Uint8 = 0;
        Uint8--;
        if(Uint8 != D_UINT8_MAX)
            return DCLSTAT_CURRENTLINE;

        for(k=0; k<D_UINT8_MAX; k++)
            Uint8--;

        if(Uint8 != 0)
            return DCLSTAT_CURRENTLINE;
    }

    /*  Test D_INT16 characteristics
    */
    {
        D_INT16         Int16;

        if(sizeof(D_INT16) != 2)
            return DCLSTAT_CURRENTLINE;

        Int16 = D_INT16_MIN;
        Int16--;
        if(Int16 != D_INT16_MAX)
            return DCLSTAT_CURRENTLINE;

        for(k=0; k<D_UINT16_MAX; k++)
            Int16--;

        if(Int16 != D_INT16_MIN)
            return DCLSTAT_CURRENTLINE;
    }

    /*  Test D_UINT16 characteristics
    */
    {
        D_UINT16        Uint16;

        if(sizeof(D_UINT16) != 2)
            return DCLSTAT_CURRENTLINE;

        Uint16 = 0;
        Uint16--;
        if(Uint16 != D_UINT16_MAX)
            return DCLSTAT_CURRENTLINE;

        for(k=0; k<D_UINT16_MAX; k++)
            Uint16--;

        if(Uint16 != 0)
            return DCLSTAT_CURRENTLINE;
    }

    /*  Test D_INT32 characteristics
    */
    {
        D_INT32         Int32;

        if(sizeof(D_INT32) != 4)
            return DCLSTAT_CURRENTLINE;

        Int32 = D_INT32_MIN;
        Int32--;
        if(Int32 != D_INT32_MAX)
            return DCLSTAT_CURRENTLINE;

        /*  Note that we don't do the counting thing here like we did for
            the 16-bit types because it takes TOO LONG...
        */
        Int32++;
        if(Int32 != D_INT32_MIN)
            return DCLSTAT_CURRENTLINE;
    }

    /*  Test D_UINT32 characteristics
    */
    {
        D_UINT32        Uint32;

        if(sizeof(D_UINT32) != 4)
            return DCLSTAT_CURRENTLINE;

        Uint32 = 0;
        Uint32--;
        if(Uint32 != D_UINT32_MAX)
            return DCLSTAT_CURRENTLINE;

        Uint32++;
        if(Uint32 != 0)
            return DCLSTAT_CURRENTLINE;
    }

  #if DCL_NATIVE_64BIT_SUPPORT
    /*  Test D_INT64 characteristics
    */
    {
        D_INT64         Int64;

        if(sizeof(D_INT64) != 8)
            return DCLSTAT_CURRENTLINE;

        Int64 = D_INT64_MIN;
        Int64--;
        if(Int64 != D_INT64_MAX)
            return DCLSTAT_CURRENTLINE;

        Int64++;
        if(Int64 != D_INT64_MIN)
            return DCLSTAT_CURRENTLINE;
    }

    /*  Test D_UINT64 characteristics
    */
    {
        D_UINT64        Uint64;

        if(sizeof(D_UINT64) != 8)
            return DCLSTAT_CURRENTLINE;

        Uint64 = 0;
        Uint64--;
        if(Uint64 != D_UINT64_MAX)
            return DCLSTAT_CURRENTLINE;

        Uint64++;
        if(Uint64 != 0)
            return DCLSTAT_CURRENTLINE;
    }
  #endif

    DclPrintf("    OK\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestMisaligned()

    This function tests misaligned data accesses.  It is disabled
    by default, as it may (WILL) exception on some systems.

    Even though this test may very well cause the system to halt/
    crash, it will help determine the effect of misaligned accesses
    on the target system.  The can be helpful in diagnosing problems
    which may be the result of misaligned accesses.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
#define DCLTEST_MISALIGNED_BUF_SIZE  (16)
static DCLSTATUS TestMisaligned(void)
{
    DCLALIGNEDBUFFER(test, data, DCLTEST_MISALIGNED_BUF_SIZE);
    DCLALIGNEDBUFFER(control, data, DCLTEST_MISALIGNED_BUF_SIZE);
    D_UINT8         cFill = 0xAB;
    D_UINT8         cTest = 0x00;
    D_UINT16       *puData;
    D_UINT32       *pulData;
  #if DCL_NATIVE_64BIT_SUPPORT
    D_UINT64       *pullData;
  #endif
    unsigned        i;
    D_BOOL          fFailure = FALSE;

    DclPrintf("Testing Misaligned Data Accesses...\n");

    /*  test misaligned 16-bit accesses.
    */
    DclPrintf("    D_UINT16 misaligned by 1 byte...\n");
    DclMemSet(&test.data, cFill, DCLTEST_MISALIGNED_BUF_SIZE);
    DclMemSet(&control.data, cFill, DCLTEST_MISALIGNED_BUF_SIZE);
    DclMemSet(&control.data[1], cTest, sizeof(D_UINT16));
    puData = (D_UINT16 *)&test.data[1];
    *puData = (D_UINT16)cTest;
    if(DclMemCmp(&test.data, &control.data, DCLTEST_MISALIGNED_BUF_SIZE) != 0)
    {
        DclPrintf("      Data Mismatch!\n");
        DclHexDump("         Test: ", HEXDUMP_UINT8, 16, DCLTEST_MISALIGNED_BUF_SIZE, &test.data);
        DclHexDump("      Control: ", HEXDUMP_UINT8, 16, DCLTEST_MISALIGNED_BUF_SIZE, &control.data);
        fFailure = TRUE;
    }

    /*  Test misaligned 32-bit accesses.
    */
    for(i = 1; i < sizeof(D_UINT32); i++)
    {
        DclPrintf("    D_UINT32 misaligned by %u byte...\n", i);
        DclMemSet(&test.data, cFill, DCLTEST_MISALIGNED_BUF_SIZE);
        DclMemSet(&control.data, cFill, DCLTEST_MISALIGNED_BUF_SIZE);
        DclMemSet(&control.data[i], cTest, sizeof(D_UINT32));
        pulData = (D_UINT32 *)&test.data[i];
        *pulData = (D_UINT32)cTest;
        if(DclMemCmp(&test.data, &control.data, DCLTEST_MISALIGNED_BUF_SIZE) != 0)
        {
            DclPrintf("      Data Mismatch!\n");
            DclHexDump("         Test: ", HEXDUMP_UINT8, 16, DCLTEST_MISALIGNED_BUF_SIZE, &test.data);
            DclHexDump("      Control: ", HEXDUMP_UINT8, 16, DCLTEST_MISALIGNED_BUF_SIZE, &control.data);
            fFailure = TRUE;
        }
    }

  /*  This really only makes sense if we are using native 64-bit
      types.  If 64-bit type emulation is enabled we use a structure
      containing two 32-bit types.  Therefore the 32-bit type test covers
      64-bit alignment in that case.  Additionally, this test would need
      to account for how the compiler stores that structure in memory.
  */
  #if DCL_NATIVE_64BIT_SUPPORT
    /*  Test misaligned 64-bit accesses.
    */
    for(i = 1; i < sizeof(D_UINT64); i++)
    {
        DclPrintf("    D_UINT64 misaligned by %u byte...\n", i);
        DclMemSet(&test.data, cFill, DCLTEST_MISALIGNED_BUF_SIZE);
        DclMemSet(&control.data, cFill, DCLTEST_MISALIGNED_BUF_SIZE);
        DclMemSet(&control.data[i], cTest, sizeof(D_UINT64));
        pullData = (D_UINT64 *)&test.data[i];
        DclUint64AssignUint32(pullData, (D_UINT32)cTest);
        if(DclMemCmp(&test.data, &control.data, DCLTEST_MISALIGNED_BUF_SIZE) != 0)
        {
            DclPrintf("      Data Mismatch!\n");
            DclHexDump("         Test: ", HEXDUMP_UINT8, 16, DCLTEST_MISALIGNED_BUF_SIZE, &test.data);
            DclHexDump("      Control: ", HEXDUMP_UINT8, 16, DCLTEST_MISALIGNED_BUF_SIZE, &control.data);
            fFailure = TRUE;
        }
    }
  #endif

    if(fFailure)
    {
        return DCLSTAT_FAILURE;
    }
    else
    {
        DclPrintf("    OK\n");

        return DCLSTAT_SUCCESS;
    }
}


/*-------------------------------------------------------------------
    This test verifies that the native byte ordering of the
    system is either true big or little-endian, and that the
    compile time byte order flag matches that of the real
    hardware.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestByteOrder(void)
{
    unsigned        jj, kk;
    D_BUFFER       *pBuff;
    D_UINT16        uTemp = 0x1122;
    D_UINT32        ulTemp = 0x11223344UL;
  #if DCL_NATIVE_64BIT_SUPPORT
    D_UINT64        ullTemp = UINT64SUFFIX(0x1122334455667788);
  #endif

    DclPrintf("Testing Byte Order...\n");

    {
        D_UINT8    *puc;
        D_UINT16   *pu;

      #if DCL_NATIVE_64BIT_SUPPORT
        {
            D_UINT64    ullTmp = UINT64SUFFIX(0x0807060504030201);
            D_UINT32   *pul;

            puc = (D_UINT8*)&ullTmp;
            DclPrintf("    Memory byte order:  %02x %02x %02x %02x %02x %02x %02x %02x\n",
                puc[0], puc[1], puc[2], puc[3], puc[4], puc[5], puc[6], puc[7]);

            pu = (D_UINT16*)&ullTmp;
            DclPrintf("    Memory word order:  %04x %04x %04x %04x\n",
                pu[0], pu[1], pu[2], pu[3]);

            pul = (D_UINT32*)&ullTmp;
            DclPrintf("    Memory dword order: %08lX %08lX\n", pul[0], pul[1]);
        }
      #else
        {
            D_UINT32    ulTmp = 0x04030201UL;

            puc = (D_UINT8*)&ulTmp;
            DclPrintf("    Memory byte order:  %02x %02x %02x %02x\n",
                puc[0], puc[1], puc[2], puc[3]);

            pu = (D_UINT16*)&ulTmp;
            DclPrintf("    Memory word order:  %04x %04x\n", pu[0], pu[1]);
        }
      #endif
    }

    pBuff = (D_BUFFER*)&uTemp;

    if(*pBuff == 0x11)
    {
        DclPrintf("    Native hardware is Big-Endian\n");

      #if !DCL_BIG_ENDIAN
        DclPrintf("Error: Environment is configured for Little-Endian (DCL_BIG_ENDIAN=FALSE)\n");
        return DCLSTAT_CURRENTLINE;
      #endif
    }
    else
    {
        DclPrintf("    Native hardware is Little-Endian\n");

        /*  If the native hardare is little-endian, reverse all the values
            now so that we can use the same comparison logic later, regardless
            whether we are big or little-endian.
        */
        DclReverseInPlace(&uTemp, sizeof(uTemp));
        DclReverseInPlace(&ulTemp, sizeof(ulTemp));
      #if DCL_NATIVE_64BIT_SUPPORT
        DclReverseInPlace(&ullTemp, sizeof(ullTemp));
      #endif

      #if DCL_BIG_ENDIAN
        DclPrintf("Error: Environment is configured for Big-Endian (DCL_BIG_ENDIAN=TRUE)\n");
        return DCLSTAT_CURRENTLINE;
      #endif
    }

    DclPrintf("    Testing D_UINT16\n");
    for(jj=0,kk=0x11; jj<sizeof(D_UINT16); jj++,kk+=0x11)
    {
        if(pBuff[jj] != kk)
        {
            DclPrintf("Failed! D_UINT16 value %X at offset %u\n", uTemp, jj);
            return DCLSTAT_CURRENTLINE;
        }
    }

    DclPrintf("    Testing D_UINT32\n");
    pBuff = (D_BUFFER*)&ulTemp;
    for(jj=0,kk=0x11; jj<sizeof(D_UINT32); jj++,kk+=0x11)
    {
        if(pBuff[jj] != kk)
        {
            DclPrintf("Failed! D_UINT32 value %lX at offset %u\n", ulTemp, jj);
            return DCLSTAT_CURRENTLINE;
        }
    }

  #if DCL_NATIVE_64BIT_SUPPORT
    DclPrintf("    Testing D_UINT64\n");
    pBuff = (D_BUFFER*)&ullTemp;
    for(jj=0,kk=0x11; jj<sizeof(D_UINT64); jj++,kk+=0x11)
    {
        if(pBuff[jj] != kk)
        {
            DclPrintf("Failed! D_UINT64 value %llX at offset %u\n", ullTemp, jj);
            return DCLSTAT_CURRENTLINE;
        }
    }
  #endif

    DclPrintf("    OK\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestMacros()

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestMacros(void)
{
    DclPrintf("Testing Macros...\n");

    DclPrintf("    Testing DCLBITMAP()\n");
    {
        DCLBITMAP   (temp1, 1);
        DCLBITMAP   (temp2, 8);
        DCLBITMAP   (temp3, 9);

        if(sizeof(temp1) != 1)
            return DCLSTAT_CURRENTLINE;
            
        if(sizeof(temp2) != 1)
            return DCLSTAT_CURRENTLINE;
            
        if(sizeof(temp3) != 2)
            return DCLSTAT_CURRENTLINE;
    }

    DclPrintf("    Testing DCLISALIGNED() error handling...\n");
    {
        /*  Ensure that a null pointer is reported as aligned, regardless
            whether an alignsize value is provided or not.

            Also, ensure that an align size of 0 does not cause a fault,
            regardless how the pointer value is set.
        */
        if(!DCLISALIGNED((D_UINTPTR)0, DCL_ALIGNSIZE))
        {
            DclPrintf("NULL pointer reported as unaligned! (a)\n");
            return DCLSTAT_CURRENTLINE;
        }

        if(!DCLISALIGNED((D_UINTPTR)0, 0))
        {
            DclPrintf("NULL pointer reported as unaligned! (b)\n");
            return DCLSTAT_CURRENTLINE;
        }

        if(!DCLISALIGNED((D_UINTPTR)1, 0))
        {
            DclPrintf("Pointer reported as unaligned! (c)\n");
            return DCLSTAT_CURRENTLINE;
        }
    }

    DclPrintf("    Testing DCLALIGNEDBUFFER()...\n");
    {
        DCLALIGNEDBUFFER    (buffer1, data, 64+1);
        DCLALIGNEDBUFFER    (buffer2, data, 128);
        D_BUFFER            ucUnused;                /* here for a reason... */
        DCLALIGNEDBUFFER    (buffer3, data, 256+1);

        /*  This variable exists solely to try to throw the alignment off.
            However, due to various compiler differences, we can't simply
            cast the value to (void) as we would usually do to prevent
            compiler warnings.  Instead we do some real, but innocuous
            things to make the compiler really think we are using the
            variable.
        */
        ucUnused = 1;
        buffer1.data[0] = ucUnused;

        /*  DCLALIGNEDBUFFER() ensures that the start of each data
            item is evenly divisible by DCL_ALIGNSIZE.
        */
        if(!DCLISALIGNED((D_UINTPTR)&buffer1.data[0], DCL_ALIGNSIZE))
        {
            DclPrintf("Buffer1 failed! pBuff=%P\n", &buffer1.data[0]);
            return DCLSTAT_CURRENTLINE;
        }

        if(!DCLISALIGNED((D_UINTPTR)&buffer2.data[0], DCL_ALIGNSIZE))
        {
            DclPrintf("Buffer2 failed! pBuff=%P\n", &buffer2.data[0]);
            return DCLSTAT_CURRENTLINE;
        }

        if(!DCLISALIGNED((D_UINTPTR)&buffer3.data[0], DCL_ALIGNSIZE))
        {
            DclPrintf("Buffer3 failed! pBuff=%P\n", &buffer3.data[0]);
            return DCLSTAT_CURRENTLINE;
        }
    }

    DclPrintf("    Testing DCLALIGNEDBITMAP()...\n");
    {
        DCLALIGNEDBITMAP    (buffer1, data, 64+1);
        DCLALIGNEDBITMAP    (buffer2, data, 128);
        D_BUFFER            ucUnused;                /* here for a reason... */
        DCLALIGNEDBITMAP    (buffer3, data, 256+1);

        /*  This variable exists solely to try to throw the alignment off.
            However, due to various compiler differences, we can't simply
            cast the value to (void) as we would usually do to prevent
            compiler warnings.  Instead we do some real, but innocuous
            things to make the compiler really think we are using the
            variable.
        */
        ucUnused = 1;
        buffer1.data[0] = ucUnused;

        /*  DCLALIGNEDBITMAP() ensures that the start of each data
            item is evenly divisible by DCL_ALIGNSIZE.
        */
        if(!DCLISALIGNED((D_UINTPTR)&buffer1.data[0], DCL_ALIGNSIZE))
        {
            DclPrintf("Buffer1 failed! pBuff=%P\n", &buffer1.data[0]);
            return DCLSTAT_CURRENTLINE;
        }

        if(!DCLISALIGNED((D_UINTPTR)&buffer2.data[0], DCL_ALIGNSIZE))
        {
            DclPrintf("Buffer2 failed! pBuff=%P\n", &buffer2.data[0]);
            return DCLSTAT_CURRENTLINE;
        }

        if(!DCLISALIGNED((D_UINTPTR)&buffer3.data[0], DCL_ALIGNSIZE))
        {
            DclPrintf("Buffer3 failed! pBuff=%P\n", &buffer3.data[0]);
            return DCLSTAT_CURRENTLINE;
        }
    }

    DclPrintf("    Testing DCLALIGNEDSTRUCT()...\n");
    {
        DCLALIGNEDSTRUCT    (Item1, struct TESTSTRUCT, TestStruct);
        D_BUFFER            ucUnused;                /* here for a reason... */
        DCLALIGNEDSTRUCT    (Item2, struct TESTSTRUCT, TestStruct);

        /*  This variable exists solely to try to throw the alignment off.
            However, due to various compiler differences, we can't simply
            cast the value to (void) as we would usually do to prevent
            compiler warnings.  Instead we do some real, but innocuous
            things to make the compiler really think we are using the
            variable.
        */
        ucUnused = 1;
        Item1.TestStruct.ucTest = ucUnused;

        /*  DCLALIGNEDSTRUCT() ensures that the start of each data
            item is evenly divisible by DCL_ALIGNSIZE.
        */
        if(!DCLISALIGNED((D_UINTPTR)&Item1.TestStruct, DCL_ALIGNSIZE))
        {
            DclPrintf("Failed! pStruct=%P\n", &Item1.TestStruct);
            return DCLSTAT_CURRENTLINE;
        }

        if(!DCLISALIGNED((D_UINTPTR)&Item2.TestStruct, DCL_ALIGNSIZE))
        {
            DclPrintf("Failed! pStruct=%P\n", &Item2.TestStruct);
            return DCLSTAT_CURRENTLINE;
        }
    }
     
    DclPrintf("    Testing DCLBITGET/SET/CLEAR()\n");
    {
        #define     SENTINEL_BYTES      (sizeof(DCL_ALIGNTYPE))
        #define     MAX_BYTES_PER_TEST  (256)
        #define     TEST_SECONDS        (2)
        unsigned    nBit;
        unsigned    nCountSet;
        unsigned    nOperations;
        DCLTIMER    timer;
        D_UINT32    ulSeed;

        /*  ELEMENT_SIZE  - The fundamental integral type size for this
                            portion of the test.
            ELEMENT_COUNT - The number of elements in the array.  This is
                            intentionally odd to help trap problems.
            BIT_COUNT     - The total number of bits within ELEMENT_COUNT
                            to use.  This is intentionally constructed so
                            there are unused bits at the end.
            ARRAY_BYTES   - The total number of bytes required to contain
                            an array of ELEMENT[ELEMENT_COUNT];
            BUFFER_BYTES  - The total number of bytes required to contain
                            ARRAY_BYTES with leading and trailing sentinels.
        */

        {
            #define     ELEMENT         (D_UINT8)
            #define     ELEMENT_SIZE    (sizeof ELEMENT)
            #define     ELEMENT_COUNT   ((MAX_BYTES_PER_TEST / ELEMENT_SIZE) - 1)
            #define     BIT_COUNT       ((((ELEMENT_COUNT-1) * ELEMENT_SIZE) * CHAR_BIT) + 1)
            #define     ARRAY_BYTES     (ELEMENT_COUNT * ELEMENT_SIZE)
            #define     BUFFER_BYTES    (SENTINEL_BYTES + ARRAY_BYTES + SENTINEL_BYTES)
            DCLBITMAP   (abTest, BUFFER_BYTES * CHAR_BIT);
            D_BUFFER   *paBits = &abTest[SENTINEL_BYTES];

            DclPrintf("      Testing using a %u bit array using %u byte elements...\n", BIT_COUNT, ELEMENT_SIZE);

            nCountSet = 0;
            nOperations = 0;
            ulSeed = 1;
            
            DclAssert(sizeof(abTest) == BUFFER_BYTES);

            if(!BitArrayInitialize(abTest, BIT_COUNT, SENTINEL_BYTES, ARRAY_BYTES))
                return DCLSTAT_CURRENTLINE;
            
            DclTimerSet(&timer, TEST_SECONDS * 1000);

            while(!DclTimerExpired(&timer))
            {
                nBit = DclRand(&ulSeed) % BIT_COUNT;
                if(DCLBITGET(paBits, nBit))
                {
                    DCLBITCLEAR(paBits, nBit);
                    if(!nCountSet)
                        return DCLSTAT_CURRENTLINE;

                    nCountSet--;

                  #if D_DEBUG
                    /*  Do this in debug mode only since we are timing this
                        whole thing.
                    */
                    if(DCLBITGET(paBits, nBit))
                    {
                        DclPrintf("Bit %u clear failed at operation %u\n", nBit, nOperations);
                        return DCLSTAT_CURRENTLINE;
                    }
                  #endif
                }
                else
                {
                    DCLBITSET(paBits, nBit);
                    if(nCountSet >= BIT_COUNT)
                        return DCLSTAT_CURRENTLINE;

                    nCountSet++;

                  #if D_DEBUG
                    /*  Do this in debug mode only since we are timing this
                        whole thing.
                    */
                    if(!DCLBITGET(paBits, nBit))
                    {
                        DclPrintf("Bit %u set failed at operation %u\n", nBit, nOperations);
                        return DCLSTAT_CURRENTLINE;
                    }
                  #endif
                }

                nOperations++;
            }

            DclPrintf("      Performed %u GET/SET/CLEAR iterations in %u seconds\n",
                nOperations, TEST_SECONDS);

            if(!BitArrayValidate(abTest, BIT_COUNT, SENTINEL_BYTES, ARRAY_BYTES, ELEMENT_SIZE, nCountSet)) 
                return DCLSTAT_CURRENTLINE;
            
            #undef  ELEMENT
            #undef  ELEMENT_SIZE
            #undef  ELEMENT_COUNT
            #undef  BIT_COUNT
            #undef  ARRAY_BYTES
            #undef  BUFFER_BYTES

        }    

        {
            #define     ELEMENT         (D_UINT16)
            #define     ELEMENT_SIZE    (sizeof ELEMENT)
            #define     ELEMENT_COUNT   ((MAX_BYTES_PER_TEST / ELEMENT_SIZE) - 1)
            #define     BIT_COUNT       ((((ELEMENT_COUNT-1) * ELEMENT_SIZE) * CHAR_BIT) + 1)
            #define     ARRAY_BYTES     (ELEMENT_COUNT * ELEMENT_SIZE)
            #define     BUFFER_BYTES    (SENTINEL_BYTES + ARRAY_BYTES + SENTINEL_BYTES)
            D_UINT16    auTest[BUFFER_BYTES / ELEMENT_SIZE];
            D_UINT16   *paBits = &auTest[SENTINEL_BYTES / ELEMENT_SIZE];

            DclPrintf("      Testing using a %u bit array using %u byte elements...\n", BIT_COUNT, ELEMENT_SIZE);

            nCountSet = 0;
            nOperations = 0;
            ulSeed = 1;

            if(!BitArrayInitialize(auTest, BIT_COUNT, SENTINEL_BYTES, ARRAY_BYTES))
                return DCLSTAT_CURRENTLINE;
            
            DclTimerSet(&timer, TEST_SECONDS * 1000);

            while(!DclTimerExpired(&timer))
            {
                nBit = DclRand(&ulSeed) % BIT_COUNT;
                if(DCLBITGET(paBits, nBit))
                {
                    DCLBITCLEAR(paBits, nBit);
                    if(!nCountSet)
                        return DCLSTAT_CURRENTLINE;
                    
                    nCountSet--;

                  #if D_DEBUG
                    /*  Do this in debug mode only since we are timing this
                        whole thing.
                    */
                    if(DCLBITGET(paBits, nBit))
                    {
                        DclPrintf("Bit %u clear failed at operation %u\n", nBit, nOperations);
                        return DCLSTAT_CURRENTLINE;
                    }
                  #endif
                }
                else
                {
                    DCLBITSET(paBits, nBit);
                    if(nCountSet >= BIT_COUNT)
                        return DCLSTAT_CURRENTLINE;
                    
                    nCountSet++;

                  #if D_DEBUG
                    /*  Do this in debug mode only since we are timing this
                        whole thing.
                    */
                    if(!DCLBITGET(paBits, nBit))
                    {
                        DclPrintf("Bit %u set failed at operation %u\n", nBit, nOperations);
                        return DCLSTAT_CURRENTLINE;
                    }
                  #endif
                }

                nOperations++;
            }

            DclPrintf("      Performed %u GET/SET/CLEAR iterations in %u seconds\n",
                nOperations, TEST_SECONDS);

            if(!BitArrayValidate(auTest, BIT_COUNT, SENTINEL_BYTES, ARRAY_BYTES, ELEMENT_SIZE, nCountSet)) 
                return DCLSTAT_CURRENTLINE;
            
            #undef  ELEMENT
            #undef  ELEMENT_SIZE
            #undef  ELEMENT_COUNT
            #undef  BIT_COUNT
            #undef  ARRAY_BYTES
            #undef  BUFFER_BYTES

        }    

      #if D_UINT32_MAX <= UINT_MAX
        {
            #define     ELEMENT         (D_UINT32)
            #define     ELEMENT_SIZE    (sizeof ELEMENT)
            #define     ELEMENT_COUNT   ((MAX_BYTES_PER_TEST / ELEMENT_SIZE) - 1)
            #define     BIT_COUNT       ((((ELEMENT_COUNT-1) * ELEMENT_SIZE) * CHAR_BIT) + 1)
            #define     ARRAY_BYTES     (ELEMENT_COUNT * ELEMENT_SIZE)
            #define     BUFFER_BYTES    (SENTINEL_BYTES + ARRAY_BYTES + SENTINEL_BYTES)
            D_UINT32    aulTest[BUFFER_BYTES / ELEMENT_SIZE];
            D_UINT32   *paBits = &aulTest[SENTINEL_BYTES / ELEMENT_SIZE];

            DclPrintf("      Testing using a %u bit array using %u byte elements...\n", BIT_COUNT, ELEMENT_SIZE);

            nCountSet = 0;
            nOperations = 0;
            ulSeed = 1;

            if(!BitArrayInitialize(aulTest, BIT_COUNT, SENTINEL_BYTES, ARRAY_BYTES))
                return DCLSTAT_CURRENTLINE;
            
            DclTimerSet(&timer, TEST_SECONDS * 1000);

            while(!DclTimerExpired(&timer))
            {
                nBit = DclRand(&ulSeed) % BIT_COUNT;
                if(DCLBITGET(paBits, nBit))
                {
                    DCLBITCLEAR(paBits, nBit);
                    if(!nCountSet)
                        return DCLSTAT_CURRENTLINE;

                    nCountSet--;

                  #if D_DEBUG
                    /*  Do this in debug mode only since we are timing this
                        whole thing.
                    */
                    if(DCLBITGET(paBits, nBit))
                    {
                        DclPrintf("Bit %u clear failed at operation %u\n", nBit, nOperations);
                        return DCLSTAT_CURRENTLINE;
                    }
                  #endif
                }
                else
                {
                    DCLBITSET(paBits, nBit);
                    if(nCountSet >= BIT_COUNT)
                        return DCLSTAT_CURRENTLINE;

                    nCountSet++;

                  #if D_DEBUG
                    /*  Do this in debug mode only since we are timing this
                        whole thing.
                    */
                    if(!DCLBITGET(paBits, nBit))
                    {
                        DclPrintf("Bit %u set failed at operation %u\n", nBit, nOperations);
                        return DCLSTAT_CURRENTLINE;
                    }
                  #endif
                }

                nOperations++;
            }

            DclPrintf("      Performed %u GET/SET/CLEAR iterations in %u seconds\n",
                nOperations, TEST_SECONDS);

            if(!BitArrayValidate(aulTest, BIT_COUNT, SENTINEL_BYTES, ARRAY_BYTES, ELEMENT_SIZE, nCountSet)) 
                return DCLSTAT_CURRENTLINE;
            
            #undef  ELEMENT
            #undef  ELEMENT_SIZE
            #undef  ELEMENT_COUNT
            #undef  BIT_COUNT
            #undef  ARRAY_BYTES
            #undef  BUFFER_BYTES

        }    
      #endif
    }
    
    DclPrintf("    Testing DCLNEXTPOW2BOUNDARY()\n");
    {
        unsigned        nn;
        unsigned        nActualResult;

        typedef struct
        {
            unsigned    nBase;
            unsigned    nRound;
            unsigned    nExpectedResult;
        } ROUNDINGTESTCASES;

        ROUNDINGTESTCASES aNext[] =
        {
            {0,0,0}, {0,1, 0}, {1,0, 1}, {1, 1, 1},
            {1,2,2}, {1,4, 4}, {1,8, 8}, {1,16,16},
            {2,0,2}, {2,1, 2}, {2,2, 2}, {2, 4, 4},
            {3,0,3}, {3,2, 4}, {3,4, 4}, {3, 8, 8},
            {4,0,4}, {4,2, 4}, {4,4, 4}, {4, 8, 8},
            {5,0,5}, {5,2, 6}, {5,4, 8}, {5, 8, 8},
            {6,0,6}, {6,2, 6}, {6,4, 8}, {6, 8, 8},
            {7,0,7}, {7,2, 8}, {7,4, 8}, {7, 8, 8},
            {8,0,8}, {8,2, 8}, {8,4, 8}, {8, 8, 8},
            {9,0,9}, {9,2,10}, {9,4,12}, {9, 8,16}, {9,16,16}
        };

        for(nn=0; nn<DCLDIMENSIONOF(aNext); nn++)
        {
            nActualResult =  DCLNEXTPOW2BOUNDARY(aNext[nn].nBase, aNext[nn].nRound);
            if(nActualResult != aNext[nn].nExpectedResult)
            {
                DclPrintf("DCLNEXTPOW2BOUNDARY(%u, %u) failed.  Expected %u, got %u\n",
                    aNext[nn].nBase, aNext[nn].nRound,
                    aNext[nn].nExpectedResult, nActualResult);

                return DCLSTAT_CURRENTLINE;
            }
        }
    }

    DclPrintf("    OK\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestBitOperations()

    This test verifies that the various primitive bit
    manipulation functions work as documented.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestBitOperations(
    const DCLTESTINFO  *pTI)
{
    unsigned        jj;
    D_UINT32        ulTemp;

    DclAssertReadPtr(pTI, sizeof(*pTI));

    DclPrintf("Testing Bit Manipulation Functions...\n");

    DclPrintf("    Testing DclBitCount()\n");
    for(ulTemp=0, jj=0; jj<=sizeof(D_UINT32)*CHAR_BIT; jj++)
    {
        if(DclBitCount(ulTemp) != jj)
        {
            DclPrintf("Failed! A value of %lU did not report %u bits set\n", ulTemp, jj);
            return DCLSTAT_CURRENTLINE;
        }

        ulTemp = (ulTemp << 1) | 1;
    }

    DclPrintf("    Testing DclBitCountArray()\n");
    {
        #define     ARRAY_BYTES     (8)
        D_BUFFER    abBits[ARRAY_BYTES];
        unsigned    nBits;
        unsigned    nStartBits = ((ARRAY_BYTES-1) * 8) - 1;

        /*  This will test value below, at, and above the number of bits
            in a byte threshold.  So for an 8 byte array, it will test
            55, 56, and 57 bit arrays.
        */
        for(nBits = nStartBits; nBits < nStartBits + 3; nBits++)
        {
            unsigned    nSetBits = 0;
            unsigned    nTries = 0;
            D_UINT32    ulSeed = 1;

            DclPrintf("      Testing a %u bit array...\n", nBits);

            DclMemSet(abBits, 0, sizeof(abBits));

            while(nSetBits < nBits)
            {
                unsigned    bb;

                /*  Pick a random bit in the range
                */
                bb = (unsigned)(DclRand(&ulSeed) % nBits);

                DclAssert(bb < nBits);

                /*  If it is already set, pick another bit
                */
                if(DCLBITGET(abBits, bb))
                {
                    if(nTries++ < 10000)
                        continue;

                    /*  Can't really guarantee that the random number
                        generator is going to hit every position, so if
                        we have tried "enough" times, just sequentially
                        find the next unset bit and use it.
                    */
                    for(bb=0; bb < nBits; bb++)
                    {
                        if(!DCLBITGET(abBits, bb))
                        {
                            DCLPRINTF(1, ("Using %u\n", bb));
                            break;
                        }
                    }
                }

                if(pTI->nVerbosity > DCL_VERBOSE_NORMAL)
                    DclPrintf("Testing bit %u\n", bb);

                /*  Set the bit and increment the "SetBitCounter"
                */
                DCLBITSET(abBits, bb);
                nSetBits++;

                /*  See if our "CountTheBits" function matches what
                    we expect.
                */
                if(DclBitCountArray(abBits, nBits) != nSetBits)
                {
                    DclPrintf("Failed! Number of set bits did not match %u\n", nSetBits);
                    return DCLSTAT_CURRENTLINE;
                }

                /*  Keep looping until every bit has been set
                */
            }
        }
    }

    DclPrintf("    Testing DclBitHighest()\n");
    for(ulTemp=0, jj=0; jj<=sizeof(D_UINT32)*CHAR_BIT; jj++)
    {
        if(DclBitHighest(ulTemp) != jj)
        {
            DclPrintf("Failed! A value of %lU did not report %u as the highest bit set\n", ulTemp, jj);
            return DCLSTAT_CURRENTLINE;
        }

        if(ulTemp == 0)
            ulTemp++;
        else
            ulTemp <<= 1;
    }

    for(ulTemp=0, jj=0; jj<=sizeof(D_UINT32)*CHAR_BIT; jj++)
    {
        if(DclBitHighest(ulTemp) != jj)
        {
            DclPrintf("Failed! A value of %lU did not report %u as the highest bit set\n", ulTemp, jj);
            return DCLSTAT_CURRENTLINE;
        }

        ulTemp = (ulTemp << 1) | 1;
    }

    DclPrintf("    OK\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestStringFuncs()

    This test verifies the operation of generic string functions.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestStringFuncs(void)
{
    DclPrintf("Testing String Functions...\n");

  #if DCL_OSFEATURE_UNICODE
    {
        #define STRBUFFLEN      (16)
        #define SHORTSTRING     "abcd"
        #define SHORTSTRINGLEN  (4)
        D_WCHAR                 tzBuff[STRBUFFLEN];
        char                    szBuff[STRBUFFLEN];
        unsigned                nLen;

                    /*----------------------------------*\
                     *                                  *
                     *   Test ANSI to WCS conversion    *
                     *                                  *
                    \*----------------------------------*/

        DclPrintf("    Testing DclOsAnsiToWcs()...\n");

        /*------------------------------------------------------
            Test getting the buffer requirements
        ------------------------------------------------------*/
        DclPrintf("      Testing querying the buffer requirements...\n");

        nLen = DclOsAnsiToWcs(NULL, 0, SHORTSTRING, SHORTSTRINGLEN);
        if(nLen != SHORTSTRINGLEN)
        {
            DclPrintf("      FAILED! Returned an invalid length: %u (A)\n", nLen);
            return DCLSTAT_CURRENTLINE;
        }

        nLen = DclOsAnsiToWcs(NULL, 0, SHORTSTRING, -1);
        if(nLen != sizeof(SHORTSTRING))
        {
            DclPrintf("      FAILED! Returned an invalid length: %u (B)\n", nLen);
            return DCLSTAT_CURRENTLINE;
        }

        nLen = DclOsAnsiToWcs(NULL, 0, "", -1);
        if(nLen != 1)
        {
            DclPrintf("      FAILED! Returned an invalid length: %u (C)\n", nLen);
            return DCLSTAT_CURRENTLINE;
        }

        /*------------------------------------------------------
            Test converting only part of a string
        ------------------------------------------------------*/
        DclPrintf("      Converting a string fragment...\n");

        tzBuff[SHORTSTRINGLEN - 1] = MAGICWIDECHAR;
        nLen = DclOsAnsiToWcs(tzBuff, DCLDIMENSIONOF(tzBuff), SHORTSTRING, SHORTSTRINGLEN - 1);

        if(nLen != SHORTSTRINGLEN - 1)
        {
            DclPrintf("      FAILED! Returned an invalid length: %u\n", nLen);
            return DCLSTAT_CURRENTLINE;
        }

        if(tzBuff[SHORTSTRINGLEN - 1] != MAGICWIDECHAR)
        {
            DclPrintf("      FAILED! Trashed the buffer beyond the designated length\n");
            return DCLSTAT_CURRENTLINE;
        }

        /*------------------------------------------------------
            Test converting a null-terminated string
        ------------------------------------------------------*/
        DclPrintf("      Converting a null-terminated string...\n");

        tzBuff[sizeof(SHORTSTRING)-1] = MAGICWIDECHAR;
        tzBuff[sizeof(SHORTSTRING)] = MAGICWIDECHAR2;
        nLen = DclOsAnsiToWcs(tzBuff, DCLDIMENSIONOF(tzBuff), SHORTSTRING, -1);

        if(nLen != sizeof(SHORTSTRING))
        {
            DclPrintf("      FAILED! Returned an invalid length: %u\n", nLen);
            return DCLSTAT_CURRENTLINE;
        }

        if(tzBuff[sizeof(SHORTSTRING) - 1] != 0)
        {
            DclPrintf("      FAILED! Result not properly null-terminated\n");
            return DCLSTAT_CURRENTLINE;
        }

        if(tzBuff[sizeof(SHORTSTRING)] != MAGICWIDECHAR2)
        {
            DclPrintf("      FAILED! Trashed the buffer beyond the designated length\n");
            return DCLSTAT_CURRENTLINE;
        }

        /*------------------------------------------------------
            Test a lack of buffer space
        ------------------------------------------------------*/
        DclPrintf("      Testing without enough buffer space...\n");

        tzBuff[0] = MAGICWIDECHAR;
        nLen = DclOsAnsiToWcs(tzBuff, SHORTSTRINGLEN - 1, SHORTSTRING, SHORTSTRINGLEN);

        if(nLen != 0)
        {
            DclPrintf("      FAILED! Returned %u when it should have returned 0\n", nLen);
            return DCLSTAT_CURRENTLINE;
        }
#if 0
        /*  The Windows string conversion functions WILL trash the buffer
            even if the function fails due to lack of space.
        */
        if(tzBuff[0] != MAGICWIDECHAR)
        {
            DclPrintf("      FAILED! Trashed the buffer even though the function failed\n");
            return DCLSTAT_CURRENTLINE;
        }
#endif
                    /*----------------------------------*\
                     *                                  *
                     *   Test WCS to ANSI conversion    *
                     *                                  *
                    \*----------------------------------*/

        DclPrintf("    Testing DclOsWcsToAnsi()...\n");

        /*------------------------------------------------------
            Test getting the buffer requirements
        ------------------------------------------------------*/
        DclPrintf("      Testing querying the buffer requirements...\n");

        /*  Create a known wide char string, without actually having to use
            any special notation.
        */
        DclOsAnsiToWcs(tzBuff, DCLDIMENSIONOF(tzBuff), SHORTSTRING, sizeof(SHORTSTRING));

        nLen = DclOsWcsToAnsi(NULL, 0, tzBuff, SHORTSTRINGLEN);
        if(nLen != SHORTSTRINGLEN)
        {
            DclPrintf("      FAILED! Returned an invalid length: %u (A)\n", nLen);
            return DCLSTAT_CURRENTLINE;
        }

        nLen = DclOsWcsToAnsi(NULL, 0, tzBuff, -1);
        if(nLen != sizeof(SHORTSTRING))
        {
            DclPrintf("      FAILED! Returned a invalid length: %u (B)\n", nLen);
            return DCLSTAT_CURRENTLINE;
        }

        DclOsAnsiToWcs(tzBuff, DCLDIMENSIONOF(tzBuff), "", -1);
        nLen = DclOsWcsToAnsi(NULL, 0, tzBuff, -1);
        if(nLen != 1)
        {
            DclPrintf("      FAILED! Returned an invalid length: %u (C)\n", nLen);
            return DCLSTAT_CURRENTLINE;
        }

        /*------------------------------------------------------
            Test converting only part of a string
        ------------------------------------------------------*/
        DclPrintf("      Converting a string fragment...\n");

        /*  Create a known wide char string, without actually having to use
            any special notation.
        */
        DclOsAnsiToWcs(tzBuff, DCLDIMENSIONOF(tzBuff), SHORTSTRING, sizeof(SHORTSTRING));

        szBuff[SHORTSTRINGLEN - 1] = MAGICCHAR;
        nLen = DclOsWcsToAnsi(szBuff, sizeof(szBuff), tzBuff, SHORTSTRINGLEN - 1);

        if(nLen != SHORTSTRINGLEN - 1)
        {
            DclPrintf("      FAILED! Returned an invalid length: %u\n", nLen);
            return DCLSTAT_CURRENTLINE;
        }

        if(szBuff[SHORTSTRINGLEN - 1] != MAGICCHAR)
        {
            DclPrintf("      FAILED! Trashed the buffer beyond the designated length\n");
            return DCLSTAT_CURRENTLINE;
        }

        /*------------------------------------------------------
            Test converting a null-terminated string
        ------------------------------------------------------*/
        DclPrintf("      Converting a null-terminated string...\n");

        /*  Create a known wide char string, without actually having to use
            any special notation.
        */
        DclOsAnsiToWcs(tzBuff, DCLDIMENSIONOF(tzBuff), SHORTSTRING, sizeof(SHORTSTRING));

        szBuff[sizeof(SHORTSTRING)-1] = MAGICCHAR;
        szBuff[sizeof(SHORTSTRING)] = MAGICCHAR2;
        nLen = DclOsWcsToAnsi(szBuff, sizeof(szBuff), tzBuff, -1);

        if(nLen != sizeof(SHORTSTRING))
        {
            DclPrintf("      FAILED! Returned an invalid length: %u\n", nLen);
            return DCLSTAT_CURRENTLINE;
        }

        if(szBuff[sizeof(SHORTSTRING) - 1] != 0)
        {
            DclPrintf("      FAILED! Result not properly null-terminated\n");
            return DCLSTAT_CURRENTLINE;
        }

        if(szBuff[sizeof(SHORTSTRING)] != MAGICCHAR2)
        {
            DclPrintf("      FAILED! Trashed the buffer beyond the designated length\n");
            return DCLSTAT_CURRENTLINE;
        }

        /*------------------------------------------------------
            Test a lack of buffer space
        ------------------------------------------------------*/
        DclPrintf("      Testing without enough buffer space...\n");

        /*  Create a known wide char string, without actually having to use
            any special notation.
        */
        DclOsAnsiToWcs(tzBuff, DCLDIMENSIONOF(tzBuff), SHORTSTRING, sizeof(SHORTSTRING));

        szBuff[0] = MAGICCHAR;
        nLen = DclOsWcsToAnsi(szBuff, SHORTSTRINGLEN - 1, tzBuff, SHORTSTRINGLEN);

        if(nLen != 0)
        {
            DclPrintf("      FAILED! Returned %u when it should have returned 0\n", nLen);
            return DCLSTAT_CURRENTLINE;
        }

#if 0
        /*  The Windows string conversion functions WILL trash the buffer
            even if the function fails due to lack of space.
        */
        if(tzBuff[0] != MAGICWIDECHAR)
        {
            DclPrintf("      FAILED! Trashed the buffer even though the function failed\n");
            return DCLSTAT_CURRENTLINE;
        }
#endif
    }
  #else

    DclPrintf("    No Unicode support\n");

  #endif

    DclPrintf("    OK\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TestSprintf()

    This function tests sprintf() functionality.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestSprintf(void)
{
    #define SPRSTR  "abcd"
    DCLSTATUS   dclStat;

    DclPrintf("Testing Sprintf Functionality...\n");

    DclPrintf("    Testing ANSI string operations...\n");

    /*  Notes on "Left justified, padded with zeros"

        The MS implementation of snprintf() appears to not allow right-
        side padding with anything other than spaces.  Even though we
        specify '0' padding here, it is spaces which are used.
    */

    /*  Notes on "Exact buffer fill, no terminator"

        Note that this will fail when testing the Microsoft version of
        _snprintf() due to slight behavioral differences.  See the errata
        in the DclVSNPrintf() function header for more information.
    */

    /*                                      Test Title                    OutputBuffSize    FmtStr   DataStr  ResultStr       ExpectedLen            OverflowLen                                       */
    if((dclStat = SprintfHelper("Normal string output",                   SPRINTFBUFFLEN  , "%s"   , SPRSTR,     SPRSTR, sizeof(    SPRSTR) - 1, sizeof(    SPRSTR))) != DCLSTAT_SUCCESS) return dclStat;
    if((dclStat = SprintfHelper("Exact buffer fill, with terminator",     sizeof(SPRSTR)  , "%s"   , SPRSTR,     SPRSTR, sizeof(    SPRSTR) - 1, sizeof(SPRSTR)    )) != DCLSTAT_SUCCESS) return dclStat;
    if((dclStat = SprintfHelper("Exact buffer fill, no terminator",       sizeof(SPRSTR)-1, "%s"   , SPRSTR,     SPRSTR, -1                    , sizeof(SPRSTR) - 1)) != DCLSTAT_SUCCESS) return dclStat;
    if((dclStat = SprintfHelper("Buffer too short",                       sizeof(SPRSTR)-2, "%s"   , SPRSTR,     SPRSTR, -1                    , sizeof(SPRSTR) - 2)) != DCLSTAT_SUCCESS) return dclStat;
    if((dclStat = SprintfHelper("Right justified, padded with spaces",    SPRINTFBUFFLEN  , "%6s"  , SPRSTR, "  "SPRSTR, sizeof("  "SPRSTR) - 1, sizeof("  "SPRSTR))) != DCLSTAT_SUCCESS) return dclStat;
    if((dclStat = SprintfHelper("Right justified, padded with zeros",     SPRINTFBUFFLEN  , "%06s" , SPRSTR, "00"SPRSTR, sizeof("00"SPRSTR) - 1, sizeof("00"SPRSTR))) != DCLSTAT_SUCCESS) return dclStat;
    if((dclStat = SprintfHelper("Right justified, padded, exact length",  7               , "%6s"  , SPRSTR, "  "SPRSTR, sizeof("  "SPRSTR) - 1, 7                 )) != DCLSTAT_SUCCESS) return dclStat;
    if((dclStat = SprintfHelper("Right justified, padded, no terminator", 6               , "%6s"  , SPRSTR, "  "SPRSTR, -1                    , 6                 )) != DCLSTAT_SUCCESS) return dclStat;
    if((dclStat = SprintfHelper("Right justified, padded, too short",     5               , "%6s"  , SPRSTR, "  "SPRSTR, -1                    , 5                 )) != DCLSTAT_SUCCESS) return dclStat;
    if((dclStat = SprintfHelper("Left justified, padded with spaces",     SPRINTFBUFFLEN  , "%-6s" , SPRSTR, SPRSTR"  ", sizeof(SPRSTR"  ") - 1, sizeof(SPRSTR"  "))) != DCLSTAT_SUCCESS) return dclStat;
    if((dclStat = SprintfHelper("Left justified, padded with zeros",      SPRINTFBUFFLEN  , "%-06s", SPRSTR, SPRSTR"  ", sizeof(SPRSTR"  ") - 1, sizeof(SPRSTR"  "))) != DCLSTAT_SUCCESS) return dclStat;
    if((dclStat = SprintfHelper("Left justified, padded, exact length",   7               , "%-6s" , SPRSTR, SPRSTR"  ", sizeof(SPRSTR"  ") - 1, 7                 )) != DCLSTAT_SUCCESS) return dclStat;
    if((dclStat = SprintfHelper("Left justified, padded, no terminator",  6               , "%-6s" , SPRSTR, SPRSTR"  ", -1                    , 6                 )) != DCLSTAT_SUCCESS) return dclStat;
    if((dclStat = SprintfHelper("Left justified, padded, too short",      5               , "%-6s" , SPRSTR, SPRSTR"  ", -1                    , 5                 )) != DCLSTAT_SUCCESS) return dclStat;
    if((dclStat = SprintfHelper("Double percent",                         SPRINTFBUFFLEN  , "%%%s" , SPRSTR, "%"SPRSTR,  sizeof(SPRSTR)        , sizeof(SPRSTR) + 1)) != DCLSTAT_SUCCESS) return dclStat;

  #if DCL_OSFEATURE_UNICODE && !TEST_MS_SNPRINTF
    {
        DclPrintf("    Testing Unicode string operations...\n");

        /*                                      Test Title                    OutputBuffSize    FmtStr   DataStr  ResultStr       ExpectedLen            OverflowLen                                       */
        if((dclStat = SprintfHelper("Normal string output",                   SPRINTFBUFFLEN  , "%W"   , SPRSTR,     SPRSTR, sizeof(    SPRSTR) - 1, sizeof(    SPRSTR))) != DCLSTAT_SUCCESS) return dclStat;
        if((dclStat = SprintfHelper("Exact buffer fill, with terminator",     sizeof(SPRSTR)  , "%W"   , SPRSTR,     SPRSTR, sizeof(    SPRSTR) - 1, sizeof(SPRSTR)    )) != DCLSTAT_SUCCESS) return dclStat;
        if((dclStat = SprintfHelper("Exact buffer fill, no terminator",       sizeof(SPRSTR)-1, "%W"   , SPRSTR,     SPRSTR, -1                    , sizeof(SPRSTR) - 1)) != DCLSTAT_SUCCESS) return dclStat;
        if((dclStat = SprintfHelper("Buffer too short",                       sizeof(SPRSTR)-2, "%W"   , SPRSTR,     SPRSTR, -1                    , sizeof(SPRSTR) - 2)) != DCLSTAT_SUCCESS) return dclStat;
        if((dclStat = SprintfHelper("Right justified, padded with spaces",    SPRINTFBUFFLEN  , "%6W"  , SPRSTR, "  "SPRSTR, sizeof("  "SPRSTR) - 1, sizeof("  "SPRSTR))) != DCLSTAT_SUCCESS) return dclStat;
        if((dclStat = SprintfHelper("Right justified, padded with zeros",     SPRINTFBUFFLEN  , "%06W" , SPRSTR, "00"SPRSTR, sizeof("00"SPRSTR) - 1, sizeof("00"SPRSTR))) != DCLSTAT_SUCCESS) return dclStat;
        if((dclStat = SprintfHelper("Right justified, padded, exact length",  7               , "%6W"  , SPRSTR, "  "SPRSTR, sizeof("  "SPRSTR) - 1, 7                 )) != DCLSTAT_SUCCESS) return dclStat;
        if((dclStat = SprintfHelper("Right justified, padded, no terminator", 6               , "%6W"  , SPRSTR, "  "SPRSTR, -1                    , 6                 )) != DCLSTAT_SUCCESS) return dclStat;
        if((dclStat = SprintfHelper("Right justified, padded, too short",     5               , "%6W"  , SPRSTR, "  "SPRSTR, -1                    , 5                 )) != DCLSTAT_SUCCESS) return dclStat;
        if((dclStat = SprintfHelper("Left justified, padded with spaces",     SPRINTFBUFFLEN  , "%-6W" , SPRSTR, SPRSTR"  ", sizeof(SPRSTR"  ") - 1, sizeof(SPRSTR"  "))) != DCLSTAT_SUCCESS) return dclStat;
        if((dclStat = SprintfHelper("Left justified, padded with zeros",      SPRINTFBUFFLEN  , "%-06W", SPRSTR, SPRSTR"  ", sizeof(SPRSTR"  ") - 1, sizeof(SPRSTR"  "))) != DCLSTAT_SUCCESS) return dclStat;
        if((dclStat = SprintfHelper("Left justified, padded, exact length",   7               , "%-6W" , SPRSTR, SPRSTR"  ", sizeof(SPRSTR"  ") - 1, 7                 )) != DCLSTAT_SUCCESS) return dclStat;
        if((dclStat = SprintfHelper("Left justified, padded, no terminator",  6               , "%-6W" , SPRSTR, SPRSTR"  ", -1                    , 6                 )) != DCLSTAT_SUCCESS) return dclStat;
        if((dclStat = SprintfHelper("Left justified, padded, too short",      5               , "%-6W" , SPRSTR, SPRSTR"  ", -1                    , 5                 )) != DCLSTAT_SUCCESS) return dclStat;
    }
  #endif

    DclPrintf("    OK\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    This test verifies that the various timing facilities are
    functioning properly.

    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestTickers(
    DCLTESTINFO        *pTI)
{
    D_UINT64            ullMilliSecs;
    D_UINT32            ulPeriod;
    D_UINT32            ulResolution;
    char               *pszName;
    DCLPERFLOGHANDLE    hPerfLog = NULL;
    DCLTIMER            t;

    /*---------------------------------------------------------
        Write data to the performance log, if enabled.  Note
        that any changes to the test name or category must be
        accompanied by changes to perfdcl.bat.  Any changes to
        the actual data fields recorded here requires changes
        to the various spreadsheets which track this data.
    ---------------------------------------------------------*/
    hPerfLog = DCLPERFLOG_OPEN(pTI->fPerfLog, NULL, "DCL", "Tickers", NULL, pTI->szPerfLogSuffix);

    DclPrintf("Testing Ticker Services...\n");

                /*-------------------------------
                    System Tick Information
                -------------------------------*/
    {
        D_UINT32        ulModulus;
        D_UINT32        ulFrequency;
        D_UINT32        ulLastTick = 0;
        D_UINT32        ulMinDiff = D_UINT32_MAX;
        D_UINT32        ulMaxDiff = 0;
        D_UINT32        ulQueries = 0;
        D_UINT32        ulTotalDiff = 0;

        ulResolution = DclOsTickResolution();
        ulModulus = DclOsTickModulus();
        ulFrequency = 1000000UL / ulResolution;
        if(ulModulus)
            ulPeriod = ulModulus / ulFrequency;
        else
            ulPeriod = D_UINT32_MAX / ulFrequency;

        DclAssert(ulPeriod);

        DclUint64AssignUint32(&ullMilliSecs, ulPeriod);
        DclUint64MulUint32(&ullMilliSecs, 1000UL);          /* secs to millisecs */
        ulPeriod = CalculatePeriod(&ullMilliSecs, &pszName);

        DclPrintf("    System Tick Information:\n");
        DclPrintf("      Resolution, microseconds per tick     %12lU\n", ulResolution);
        DclPrintf("      Frequency, ticks per second           %12lU\n", ulFrequency);
        DclPrintf("      Modulus                               %12lU\n", ulModulus);
        DclPrintf("      Current tick count                    %12lU\n", DclOsTickCount());
        DclPrintf("      Maximum timeable period (approximate) %12lU %s\n", ulPeriod, pszName);

        DCLPERFLOG_NUM(hPerfLog, "TickResUS",   ulResolution);
        DCLPERFLOG_NUM(hPerfLog, "TicksPerSec", ulFrequency);
        DCLPERFLOG_NUM(hPerfLog, "TickModulus", ulModulus);

        DclTimerSet(&t, 5000);
        while(!DclTimerExpired(&t))
        {
            D_UINT32    ulDiff;
            D_UINT32    ulCurrentTick;

            /*  One-time initialization of ulLastTick
            */
            while(ulLastTick == 0)
                ulLastTick = DclOsTickCount();

            ulCurrentTick = DclOsTickCount();

            ulQueries++;

            if(ulCurrentTick < ulLastTick)
                ulDiff = (ulCurrentTick + ulModulus) - ulLastTick;
            else
                ulDiff = ulCurrentTick - ulLastTick;

            ulTotalDiff += ulDiff;

            /*  Don't allow "0" as a minimum difference...
            */
            if(ulDiff)
            {
                ulMinDiff = DCLMIN(ulMinDiff, ulDiff);
                ulMaxDiff = DCLMAX(ulMaxDiff, ulDiff);
            }

            ulLastTick = ulCurrentTick;
        }

        DclPrintf("      Sequential Queries for 5 seconds:\n");
        DclPrintf("        Total queries                       %12lU\n", ulQueries);
        DclPrintf("        Minimum tick differences            %12lU\n", ulMinDiff);
        DclPrintf("        Maximum tick differences            %12lU\n", ulMaxDiff);
        DclPrintf("        Total tick differences              %12lU\n", ulTotalDiff);
    }

                /*-------------------------------
                    HighRes Tick Information
                -------------------------------*/
    {
        D_UINT64        ullResolution;
        D_UINT64        ullFrequency;
        D_UINT64        ullTemp;
        D_UINT64        ullModulus;
        D_UINT64        ullRes;
        D_UINT64        ullCount;

        ullResolution = DclOsHighResTickResolution();
        if(DclUint64GreaterUint32(&ullResolution, D_UINT32_MAX))
        {
            /*  No macros for dividing a 64-bit number by another, so scale
                scale the values first so we can divide by a 32-bit value.
            */
            ullTemp = ullResolution;

            DclUint64DivUint32(&ullTemp, 1000000UL);
            DclAssert(DclUint64GreaterUint32(&ullTemp, 0));
            DclAssert(!DclUint64GreaterUint32(&ullTemp, D_UINT32_MAX));

            DclUint64AssignUint32(&ullFrequency, 1000000UL);
            DclUint64DivUint32(&ullFrequency, DclUint32CastUint64(&ullTemp));
        }
        else
        {
            /*  We know that ullResolution fits into a D_UINT32, so no
                scaling necessary in this case.
            */
            DclAssert(DclUint64GreaterUint32(&ullResolution, 0));

            DclUint64AssignUint32(&ullFrequency, NANOS_PER_SECOND);
            DclUint64MulUint32(&ullFrequency, PICOS_PER_NANO);
            DclUint64DivUint32(&ullFrequency, DclUint32CastUint64(&ullResolution));
         }

        DclAssert(!DclUint64GreaterUint32(&ullFrequency, D_UINT32_MAX));

        ullModulus = DclOsHighResTickModulus();
        ullMilliSecs = ullModulus;

        /*  If the modulus is zero, we need to change it to the max 64-bit
            value, which we can easily do by subtracting 1.
        */
        if(!DclUint64GreaterUint32(&ullMilliSecs, 0))
            DclUint64SubUint32(&ullMilliSecs, 1);

        DclAssert(DclUint64GreaterUint32(&ullFrequency, 0));

        /*  Scale this to milliseconds being careful not to lose accuracy
        */
        if(DclUint64GreaterUint32(&ullFrequency, 1000000UL))
        {
            ullTemp = ullFrequency;
            DclUint64DivUint32(&ullTemp, 1000UL);
            DclUint64DivUint32(&ullMilliSecs, DclUint32CastUint64(&ullTemp));
        }
        else
        {
            DclUint64DivUint32(&ullMilliSecs, DclUint32CastUint64(&ullFrequency));
            DclUint64MulUint32(&ullMilliSecs, 1000UL);
        }

        ulPeriod = CalculatePeriod(&ullMilliSecs, &pszName);
        ullRes = DclOsHighResTickResolution();
        ullCount = DclOsHighResTickCount();

        DclPrintf("    HighRes Tick Information:\n");
        DclPrintf("      Resolution, picoseconds per tick  %16llU\n", VA64BUG(ullRes));
        DclPrintf("      Frequency, ticks per second       %16llU\n", VA64BUG(ullFrequency));
        DclPrintf("      Modulus                           %16llU\n", VA64BUG(ullModulus));
        DclPrintf("      Current tick count                %16llU\n", VA64BUG(ullCount));
        DclPrintf("      Maximum timeable period (approximate) %12lU %s\n", ulPeriod, pszName);

        DCLPERFLOG_NUM64(hPerfLog, "HRTickResPS",   DclOsHighResTickResolution());
        DCLPERFLOG_NUM64(hPerfLog, "HRTicksPerSec", ullFrequency);
        DCLPERFLOG_NUM64(hPerfLog, "HRTickModulus", ullModulus);

        DclUint64AssignUint32(&ullTemp, ulResolution);
        DclUint64MulUint32(&ullTemp, 1000000UL);

        if(DclUint64IsEqualUint64(&ullResolution, &ullTemp))
            DclPrintf("    NOTE: The HighRes ticker appears to be using the system tick\n");

        do
        {
            unsigned        k;
            D_UINT64       *pullTick;
            D_UINT64        ullMin;
            D_UINT64        ullMax;
            D_UINT64        ullAvg;
            D_UINT32        ulElapsedMS;
            unsigned        nWrapped = 0;
            unsigned        nMemSize = (SEQHIGHRESCOUNT+1) * sizeof(D_UINT64);

            DclUint64AssignUint32(&ullMax, 0);
            DclUint64AssignUint32(&ullMin, 0);
            DclUint64SubUint32(&ullMin, 1);

            pullTick = DclMemAlloc(nMemSize);
            if(!pullTick)
            {
                DclPrintf("Failed to allocate %u bytes of memory, skipping this test...\n", nMemSize);
                break;
            }

            /*  Independently measure how long this takes using the DCL
                timer facilities.  We expressly do NOT use the standard
                timestamp functions because they could be mapped to use
                the high-res tick/timestamp functions which is what we
                are testing here.  The DCL timer functions always use
                the standard tick.
            */
            DclTimerSet(&t, 60*60*1000);

            for(k=0; k<SEQHIGHRESCOUNT+1; k++)
                pullTick[k] = DclOsHighResTickCount();

            ulElapsedMS = DclTimerElapsed(&t);

            for(k=1; k<SEQHIGHRESCOUNT+1; k++)
            {
                D_UINT64    ullDiff;

                if(DclUint64GreaterUint32(&ullModulus, 0))
                {
                    if(DclUint64GreaterUint64(&pullTick[k], &ullModulus))
                    {
                        DclPrintf("    Failed! Tick value %llU greater than tick modulus %llU (element %u)\n",
                            VA64BUG(pullTick[k]), VA64BUG(ullModulus), k);

                        DclMemFree(pullTick);
                        return DCLSTAT_CURRENTLINE;
                    }
                }

                ullDiff = pullTick[k];
                DclUint64SubUint64(&ullDiff, &pullTick[k-1]);

                if(DclUint64LessUint64(&pullTick[k], &pullTick[k-1]))
                {
                    DclUint64AddUint64(&ullDiff, &ullModulus);

                    nWrapped++;
                }

                if(DclUint64GreaterUint64(&ullDiff, &ullMax))
                    ullMax = ullDiff;

                if(DclUint64LessUint64(&ullDiff, &ullMin))
                    ullMin = ullDiff;
            }

            ullAvg = pullTick[SEQHIGHRESCOUNT];
            DclUint64SubUint64(&ullAvg, &pullTick[0]);
            DclUint64DivUint32(&ullAvg, SEQHIGHRESCOUNT);

            DclPrintf("      %u sequential HighRes tick queries:\n", SEQHIGHRESCOUNT);
            DclPrintf("        Minimum tick differences        %16llU\n", VA64BUG(ullMin));
            DclPrintf("        Maximum tick differences        %16llU\n", VA64BUG(ullMax));
            DclPrintf("        Average tick differences        %16llU\n", VA64BUG(ullAvg));
            DclPrintf("        Ticker wrapped                        %10u\n", nWrapped);
            DclPrintf("        Time required (std tick services)     %10lU ms\n", ulElapsedMS);

            DclMemFree(pullTick);
        }
        while(FALSE);

        /*  Query the HighRes timer for a given period of time, watching for
            it to wrap.  (This test may not wrap at all in many cases, just
            depends on the timer characteristics.)
        */
        {
            D_UINT64        ullLast;
            D_UINT64        ullCurrent;
            D_UINT32        ulWrapped = 0;
            D_UINT32        ulCount = 0;

            DclPrintf("      Starting a %u second sequential HighRes ticker wrap test\n", HIGHRESTESTSECONDS);

            DclUint64AssignUint32(&ullLast, 0);

            DclTimerSet(&t, HIGHRESTESTSECONDS*1000*2);

            while(DclTimerElapsed(&t) < HIGHRESTESTSECONDS*1000)
            {
                ulCount++;

                ullCurrent = DclOsHighResTickCount();

                if(DclUint64GreaterUint32(&ullModulus, 0))
                {
                    if(DclUint64GreaterUint64(&ullCurrent, &ullModulus))
                    {
                        DclPrintf("Failed! Tick value %llU greater than tick modulus %llU (count=%lU)\n",
                            VA64BUG(ullCurrent), VA64BUG(ullModulus), ulCount);

                        return DCLSTAT_CURRENTLINE;
                    }
                }

                if(DclUint64LessUint64(&ullCurrent, &ullLast))
                    ulWrapped++;

                ullLast = ullCurrent;
            }

            DclPrintf("        The HighRes ticker wrapped (at least) %lU times (%lU queries)\n",
                ulWrapped, ulCount);
        }
    }

    DCLPERFLOG_WRITE(hPerfLog);
    DCLPERFLOG_CLOSE(hPerfLog);

    DclPrintf("    OK\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    This test verifies that the various timing facilities are
    functioning properly.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestTimers(void)
{
    D_UINT32        ulLastTickCount;
    D_UINT32        ulCurrentTickCount;
    D_UINT32        ulElapsedMicroSecs = 0;
    D_UINT32        ulResolution;
    DCLTIMER        t;
    DCLTIMESTAMP    TimeStamp;
    D_UINT64        ullElapsed;

    /*  Do timer checks  - this only gives a rough (qualitative) indication
        that the timer routines are working
    */
    DclPrintf("Testing Timer Services...\n");

    ulResolution = DclOsTickResolution();

                /*-------------------------------
                    Testing Time Stamps
                -------------------------------*/

    DclPrintf("    Testing TimeStamps...\n");
    {
        D_UINT32        ulTicks = 0;
        D_UINT64        ullLastElapsed;

        DclUint64AssignUint32(&ullLastElapsed, 0);

        /*  Loop until the system tick changes
        */
        ulLastTickCount = DclOsTickCount();
        while(DclOsTickCount() == ulLastTickCount)
        {}

        /*  Count how many times the high res ticker ticks while waiting
            for one system tick to elapse.
        */
        ulLastTickCount = DclOsTickCount();
        TimeStamp = DclTimeStamp();
        do
        {
            D_UINT64 ullTimeElapsed = DclTimePassedNS(TimeStamp);

            if(!DclUint64IsEqualUint64(&ullTimeElapsed, &ullLastElapsed))
            {
                ulTicks++;

                DclUint64AssignUint64(&ullLastElapsed, &ullTimeElapsed);
            }
        }
        while(DclOsTickCount() == ulLastTickCount);

        if(ulTicks <= 1)
        {
            DclPrintf("      The time stamp changed only %lU times during 1 system tick\n", ulTicks);
        }
        else
        {
            unsigned    kk;

            DclPrintf("      The time stamp changed at least %lU times during 1 system tick\n", ulTicks);

            /*  Remember that we are explicitly using the Timer services
                rather than the more convenient TimeStamp/TimePassed
                services because the former always use the standard system
                tick, while the latter may be using the HighRes ticker,
                and you can't measure something against itself.
            */
            DclTimerSet(&t, 60*60*1000);

            for(kk=0; kk<1000; kk++)
            {
                DCLTIMESTAMP  ts;

                ts = DclTimeStamp();
                while(TRUE)
                {
                    D_UINT64 ullElapsedNS = DclTimePassedNS(ts);

                    if(DclUint64GreaterUint32(&ullElapsedNS, 999999UL))
                        break;
                }
            }

            DclPrintf("      1000 1 ms timestamp delays took %lU ms according to the system ticker\n", DclTimerElapsed(&t));
        }
    }

                /*-------------------------------
                    DclTimerSet/Expired Tests
                -------------------------------*/

    DclPrintf("    Testing DclTimerSet/Expired()...\n");
    {
        
        DclPrintf("      Maximum sample period is %lU ms\n", DclTimerSamplePeriod());
        
        DclPrintf("      Starting 500 ms timeout...\n");

        TimeStamp = DclTimeStamp();
        ulLastTickCount = DclOsTickCount();
        DclTimerSet(&t, 500UL);
        while(!DclTimerExpired(&t))
            continue;

        ullElapsed = DclTimePassedNS(TimeStamp);
        ulCurrentTickCount = DclOsTickCount();
        ulElapsedMicroSecs = (ulCurrentTickCount - ulLastTickCount) * ulResolution;

        DclPrintf("      Elapsed time %lU us or %llU ns*\n", ulElapsedMicroSecs, VA64BUG(ullElapsed));

        if(ulElapsedMicroSecs < 500000UL)
        {
            DclPrintf("      Requested timeout of 500 ms, but only %lU ms elapsed.\n",
                      ulElapsedMicroSecs / 1000);
            return DCLSTAT_CURRENTLINE;
        }
    }

    {
        DclPrintf("      Starting 200 ms timeout...\n");

        TimeStamp = DclTimeStamp();
        ulLastTickCount = DclOsTickCount();
        DclTimerSet(&t, 200UL);
        while(!DclTimerExpired(&t))
            continue;

        ullElapsed = DclTimePassedNS(TimeStamp);
        ulCurrentTickCount = DclOsTickCount();
        ulElapsedMicroSecs = (ulCurrentTickCount - ulLastTickCount) * ulResolution;

        DclPrintf("      Elapsed time %lU us or %llU ns*\n", ulElapsedMicroSecs, VA64BUG(ullElapsed));

        if(ulElapsedMicroSecs < 200000UL)
        {
            DclPrintf("      Requested timeout of 200 ms, but only %lU ms elapsed.\n",
                      ulElapsedMicroSecs / 1000);
            return DCLSTAT_CURRENTLINE;
        }
    }

    {
        DclPrintf("      Starting 100 ms timeout...\n");

        TimeStamp = DclTimeStamp();
        ulLastTickCount = DclOsTickCount();
        DclTimerSet(&t, 100UL);
        while(!DclTimerExpired(&t))
            continue;

        ullElapsed = DclTimePassedNS(TimeStamp);
        ulCurrentTickCount = DclOsTickCount();
        ulElapsedMicroSecs = (ulCurrentTickCount - ulLastTickCount) * ulResolution;

        DclPrintf("      Elapsed time %lU us or %llU ns*\n", ulElapsedMicroSecs, VA64BUG(ullElapsed));

        if(ulElapsedMicroSecs < 100000UL)
        {
            DclPrintf("      Requested timeout of 100 ms, but only %lU ms elapsed.\n",
                      ulElapsedMicroSecs / 1000);
            return DCLSTAT_CURRENTLINE;
        }
    }

    {
        DclPrintf("      Starting 50 ms timeout...\n");

        TimeStamp = DclTimeStamp();
        ulLastTickCount = DclOsTickCount();
        DclTimerSet(&t, 50UL);
        while(!DclTimerExpired(&t))
            continue;

        ullElapsed = DclTimePassedNS(TimeStamp);
        ulCurrentTickCount = DclOsTickCount();
        ulElapsedMicroSecs = (ulCurrentTickCount - ulLastTickCount) * ulResolution;

        DclPrintf("      Elapsed time %lU us or %llU ns*\n", ulElapsedMicroSecs, VA64BUG(ullElapsed));

        if(ulElapsedMicroSecs < 50000UL)
        {
            DclPrintf("      Requested timeout of 50 ms, but only %lU ms elapsed.\n",
                      ulElapsedMicroSecs / 1000);
            return DCLSTAT_CURRENTLINE;
        }
    }

    {
        DclPrintf("      Starting 20 ms timeout...\n");

        TimeStamp = DclTimeStamp();
        ulLastTickCount = DclOsTickCount();
        DclTimerSet(&t, 20UL);
        while(!DclTimerExpired(&t))
            continue;

        ullElapsed = DclTimePassedNS(TimeStamp);
        ulCurrentTickCount = DclOsTickCount();
        ulElapsedMicroSecs = (ulCurrentTickCount - ulLastTickCount) * ulResolution;

        DclPrintf("      Elapsed time %lU us or %llU ns*\n", ulElapsedMicroSecs, VA64BUG(ullElapsed));

        if(ulElapsedMicroSecs < 20000UL)
        {
            DclPrintf("      Requested timeout of 20 ms, but only %lU ms elapsed.\n",
                      ulElapsedMicroSecs / 1000);
            return DCLSTAT_CURRENTLINE;
        }
    }

    {
        DclPrintf("      Starting 5 ms timeout...\n");

        TimeStamp = DclTimeStamp();
        ulLastTickCount = DclOsTickCount();
        DclTimerSet(&t, 5UL);
        while(!DclTimerExpired(&t))
            continue;

        ullElapsed = DclTimePassedNS(TimeStamp);
        ulCurrentTickCount = DclOsTickCount();
        ulElapsedMicroSecs = (ulCurrentTickCount - ulLastTickCount) * ulResolution;

        DclPrintf("      Elapsed time %lU us or %llU ns*\n", ulElapsedMicroSecs, VA64BUG(ullElapsed));

        if(ulElapsedMicroSecs < 5000UL)
        {
            DclPrintf("      Requested timeout of 5 ms, but only %lU ms elapsed.\n",
                      ulElapsedMicroSecs / 1000);
            return DCLSTAT_CURRENTLINE;
        }
    }

    DclPrintf("     *Nanosecond measurements are accurate ONLY if the HighRes ticker can\n");
    DclPrintf("      measure long periods, and the HighRes timer feature is enabled.\n");

    DclPrintf("    OK\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    This test verifies that the "delay" functionality is
    functioning properly.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestDelay(void)
{
    #define     MILLISECS_TO_DELAY            (50UL)
    #define     MICROSECS_PER_MILLISEC      (1000UL)
    #define     EXHAUSTIVE_DELAY_TIME_SECS    (30UL)
    D_UINT32    ulResolutionUSperTick;
    D_UINT32    ulMicroSecsToDelay;
    D_UINT32    ulTicksToDelay;
    D_UINT32    ulMinDelayNS;
    D_UINT32    kk;
    D_UINT32    ulIncrement;
    D_UINT32    ulStartDelayNS;
    DCLSTATUS   dclStat;
    DCLTIMER    tmr;

    DclPrintf("Testing Delay Services...\n");

    /*  In additional to ensuring that the delay functionality is available,
        the following function call also serves to ensure that the one-time
        initialization of the delay services has already been completed when
        the actual test starts.
    */
    ulMinDelayNS = DclNanosecondDelayMinimum();
    if(!ulMinDelayNS)
    {
        DclPrintf("    Skipping -- HighRes ticker is inadequate or non-existent\n");

        /*  Return success in this instance.  The HighRes ticker and the
            nanosecond delay interfaces are optional, and the lack of
            this functionality does not constitute a test failure.
        */
        return DCLSTAT_SUCCESS;
    }

    ulResolutionUSperTick = DclOsTickResolution();

    DclPrintf("    Minimum allowable delay count is %lU nanoseconds\n", ulMinDelayNS);

    DclPrintf("    Testing an out-of-range request...\n");
    dclStat = DclNanosecondDelay(ulMinDelayNS-1);
    if(dclStat != DCLSTAT_OUTOFRANGE)
    {
        DclPrintf("FAILED! DclNanosecondDelay(%lU) should have reported DCLSTAT_OUTOFRANGE, got %lX instead\n", ulMinDelayNS-1, dclStat);
        return DCLSTAT_CURRENTLINE;
    }

    DclPrintf("    Testing %lU to n nanosecond delays for a maximum of %lU seconds\n",
        ulMinDelayNS, EXHAUSTIVE_DELAY_TIME_SECS);

    DclTimerSet(&tmr, EXHAUSTIVE_DELAY_TIME_SECS*1000UL);
    kk = ulMinDelayNS;
    ulStartDelayNS = ulMinDelayNS;
    ulIncrement = 1;
    while(TRUE)
    {
        dclStat = DclNanosecondDelay(kk);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("    Failed at %lU ns, code=%lX\n", kk, dclStat);
            break;
        }

        if(DclTimerExpired(&tmr))
        {
            DclPrintf("    Tested delays from %lU to %lU ns before %lU second timer expired\n",
                ulMinDelayNS, kk, EXHAUSTIVE_DELAY_TIME_SECS);
            break;
        }

        if(kk == 10000)
        {
            DclPrintf("      Finished %lU to 10,000 ns counting by 1 ns, counting by 10 ns\n",
                ulStartDelayNS);

            ulStartDelayNS = 10000;
            ulIncrement = 10;
        }
        else if(kk == 100000)
        {
            DclPrintf("      Finished %lU to 100,000 ns counting by 10 ns, counting by 100 ns\n",
                ulStartDelayNS);

            ulStartDelayNS = 100000;
            ulIncrement = 100;
        }
        else if(kk == 1000000)
        {
            DclPrintf("      Finished %lU to 1,000,000 ns counting by 100 ns, counting by 1000 ns\n",
                ulStartDelayNS);

            ulIncrement = 1000;
        }

        if(kk > (D_UINT32_MAX - ulIncrement))
        {
            /*  If we ever get here, either the system ticker or the
                nanosecond delay functionality is horribly broken (or
                this test was configured to run for a really long time).
            */
            DclPrintf("FAILED! System tick or nanosecond delay is broken!\n");
            return DCLSTAT_CURRENTLINE;
        }

        kk += ulIncrement;
    }

    /*  Calculate the number of ticks to delay to at least get a 50ms delay
                      MILLISECS_TO_DELAY * MICROSECS_PER_MILLISEC
        ticks =      --------------------------------------------
                               MICROSECS_PER_TICK

        Round up to ensure at least a 50ms timeout
    */
    ulTicksToDelay = ((MILLISECS_TO_DELAY * MICROSECS_PER_MILLISEC) +
        ulResolutionUSperTick - 1) / ulResolutionUSperTick;

    /*  Now get the actual value of microsecs to delay (should be
        above MILLISECS_TO_DELAY * MICROSECS_PER_MILLISEC).
    */
    ulMicroSecsToDelay = ulTicksToDelay * ulResolutionUSperTick;

    dclStat = DelayHelper(1000000, 1, ulMinDelayNS, 1000000UL / ulResolutionUSperTick);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = DelayHelper(ulMicroSecsToDelay, 5, ulMinDelayNS, ulTicksToDelay);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = DelayHelper(ulMicroSecsToDelay, 50, ulMinDelayNS, ulTicksToDelay);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = DelayHelper(ulMicroSecsToDelay, 500, ulMinDelayNS, ulTicksToDelay);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = DelayHelper(ulMicroSecsToDelay, 5000, ulMinDelayNS, ulTicksToDelay);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = DelayHelper(ulMicroSecsToDelay, 50000, ulMinDelayNS, ulTicksToDelay);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    DclPrintf("   *Nanosecond measurements are accurate ONLY if the HighRes ticker can\n");
    DclPrintf("    measure long periods, and the HighRes timer feature is enabled.\n");
    DclPrintf("    OK\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestSleep(void)
{
    DCLTIMER        timer;
    DCLTIMESTAMP    TimeStamp;
    unsigned        nn;
    D_UINT32        ulElapsedMS;
    D_UINT64        ullTimeNS;

    DclPrintf("Testing Sleep...\n");

    /*---------------------------------------------------------------
    ---------------------------------------------------------------*/
    {
        
        DclPrintf("  Starting 1 1000 ms sleep...\n");

        /*  The timer period does not really matter since all we are measuring
            is elapsed time.
        */
        DclTimerSet(&timer, 1000);
        TimeStamp = DclTimeStamp();

        DclOsSleep(1000);

        ulElapsedMS = DclTimerElapsed(&timer);
        ullTimeNS = DclTimePassedNS(TimeStamp);

        DclPrintf("    Elapsed time %lU ms or %llU ns\n", ulElapsedMS, VA64BUG(ullTimeNS));

        if(ulElapsedMS < 1000L)
    {
            DclPrintf("    NOTE: Requested sleep of 1000 ms, but only %lU ms elapsed.\n", ulElapsedMS);

        /*  For now this test can't fail -- it's OK to sleep less than
            that requested.
        */
    }
    }

    /*---------------------------------------------------------------
    ---------------------------------------------------------------*/
    {
        DclPrintf("  Starting 125 8 ms sleeps...\n");

        DclTimerSet(&timer, 1000);
        TimeStamp = DclTimeStamp();

        for(nn=0; nn<125; nn++)
            DclOsSleep(8);

        ulElapsedMS = DclTimerElapsed(&timer);
        ullTimeNS = DclTimePassedNS(TimeStamp);
        
        DclPrintf("    Elapsed time %lU ms or %llU ns\n", ulElapsedMS, VA64BUG(ullTimeNS));
    }

    /*---------------------------------------------------------------
    ---------------------------------------------------------------*/
    {
        DclPrintf("  Starting 250 4 ms sleeps...\n");

        DclTimerSet(&timer, 1000);
        TimeStamp = DclTimeStamp();

        for(nn=0; nn<250; nn++)
            DclOsSleep(4);

        ulElapsedMS = DclTimerElapsed(&timer);
        ullTimeNS = DclTimePassedNS(TimeStamp);

        DclPrintf("    Elapsed time %lU ms or %llU ns\n", ulElapsedMS, VA64BUG(ullTimeNS));
    }

    /*---------------------------------------------------------------
    ---------------------------------------------------------------*/
    {
        DclPrintf("  Starting 500 2 ms sleeps...\n");

        DclTimerSet(&timer, 1000);
        TimeStamp = DclTimeStamp();

        for(nn=0; nn<500; nn++)
            DclOsSleep(2);

        ulElapsedMS = DclTimerElapsed(&timer);
        ullTimeNS = DclTimePassedNS(TimeStamp);

        DclPrintf("    Elapsed time %lU ms or %llU ns\n", ulElapsedMS, VA64BUG(ullTimeNS));
    }

    /*---------------------------------------------------------------
    ---------------------------------------------------------------*/
    {
        DclPrintf("  Starting 1000 1 ms sleeps...\n");

        DclTimerSet(&timer, 1000);
        TimeStamp = DclTimeStamp();

        for(nn=0; nn<1000; nn++)
            DclOsSleep(1);

        ulElapsedMS = DclTimerElapsed(&timer);
        ullTimeNS = DclTimePassedNS(TimeStamp);

        DclPrintf("    Elapsed time %lU ms or %llU ns\n", ulElapsedMS, VA64BUG(ullTimeNS));

        if(ulElapsedMS > 3000)
            DclPrintf("    NOTE: Sleep periods of less than %lU ms appear to be very inefficient\n", ulElapsedMS / 1000);
    }

    /*---------------------------------------------------------------
    ---------------------------------------------------------------*/
    {
        D_UINT32    ulSleepTotalMS = ulElapsedMS;

        DclPrintf("  Starting 1000 0 ms sleeps...\n");

        DclTimerSet(&timer, 1000);
        TimeStamp = DclTimeStamp();

        for(nn=0; nn<1000; nn++)
            DclOsSleep(0);

        ulElapsedMS = DclTimerElapsed(&timer);
        ullTimeNS = DclTimePassedNS(TimeStamp);

        DclPrintf("    Elapsed time %lU ms or %llU ns\n", ulElapsedMS, VA64BUG(ullTimeNS));

        if( (ulElapsedMS > ulSleepTotalMS - (ulSleepTotalMS >> 3)) &&
            (ulElapsedMS < ulSleepTotalMS + (ulSleepTotalMS >> 3)) )
        {
            /*  If the 0 and 1 ms sleep totals are within about 12.5% of
                each other, then we can assume that they are <probably>
                implemented in similar fashion, meaning Sleep(0) is not
                very useful.
            */
            DclPrintf("    NOTE: 0 ms sleeps appear to be pretty much the same as 1 ms sleeps\n");
        }
/*
        Not really enough information to come to this conclusion.  If we
        knew for certain that a different thread could get scheduled when
        a running thread does a Sleep(0), this we could say it is effective.

        if(ulElapsedMS < 1000)
            DclPrintf("    NOTE: Sleep periods of 0 ms appear to be effective\n");
*/
    }

    DclPrintf("   *Nanosecond measurements are accurate ONLY if the HighRes ticker can\n");
    DclPrintf("    measure long periods, and the HighRes timer feature is enabled.\n");

    DclPrintf("    OK\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    This test verifies that the ECC code functions properly.

    ToDo: There are some other ECC tests in the FlashFX code base,
          and they should probably be examined to see whether any
          functionality from them should be moved into this test,
          or whether they should just be removed altogether.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestECC(void)
{
    #define         MAXECCSIZE  (8192)
    D_BUFFER       *pBuff;
    size_t          nBuffSize;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;

    DclPrintf("Testing ECC Calculation and Correction...\n");

    pBuff = DclMemAlloc(MAXECCSIZE);
    if(!pBuff)
    {
        DclPrintf("Out of memory!\n");
        return DCLSTAT_OUTOFMEMORY;
    }

    for(nBuffSize = 4; nBuffSize <= MAXECCSIZE; nBuffSize <<= 1)
    {
        unsigned    nn;

        DclPrintf("    Testing %u byte buffers...\n", nBuffSize);

            /*---------------------------------------------------*\
             *       Test special case of all data being 0's     *
            \*---------------------------------------------------*/

        DclMemSet(pBuff, 0, nBuffSize);

        if(nBuffSize == MAXECCSIZE)
        {
            D_UINT32 ulECC = DclEccCalculate(pBuff, nBuffSize);

            /*  8KB buffers with all 0x00 data should generate
                an ECC of all 0xFFs.
            */
            if(ulECC != D_UINT32_MAX)
            {
                DclPrintf("Invalid ECC generated for buffer size %u, ECC=%lX\n", nBuffSize, ulECC);
                dclStat = DCLSTAT_CURRENTLINE;
                break;
            }
        }

        dclStat = ECCHelper(pBuff, nBuffSize);
        if(dclStat != DCLSTAT_SUCCESS)
            break;

            /*---------------------------------------------------*\
             *       Test special case of all data being 1's     *
            \*---------------------------------------------------*/

        DclMemSet(pBuff, 0xFF, nBuffSize);

        if(nBuffSize == MAXECCSIZE)
        {
            D_UINT32 ulECC = DclEccCalculate(pBuff, nBuffSize);

            /*  8KB buffers with all 0xFF data should generate
                an ECC of all 0xFFs.
            */
            if(ulECC != D_UINT32_MAX)
            {
                DclPrintf("Invalid ECC generated for buffer size %u, ECC=%lX\n", nBuffSize, ulECC);
                dclStat = DCLSTAT_CURRENTLINE;
                break;
            }
        }

        dclStat = ECCHelper(pBuff, nBuffSize);
        if(dclStat != DCLSTAT_SUCCESS)
            break;

            /*---------------------------------------------------*\
             *                 Test with random data             *
            \*---------------------------------------------------*/

        for(nn=0; nn<nBuffSize/sizeof(D_UINT32); nn++)
        {
            *(((D_UINT32*)pBuff)+nn) = DclRand(NULL);
        }

        dclStat = ECCHelper(pBuff, nBuffSize);
        if(dclStat != DCLSTAT_SUCCESS)
            break;
    }

    DclMemFree(pBuff);

    if(dclStat == DCLSTAT_SUCCESS)
        DclPrintf("    OK\n");

    return dclStat;
}


/*-------------------------------------------------------------------
    This test verifies that the random number generator appear
    to be working as expected.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure
-------------------------------------------------------------------*/
static DCLSTATUS TestRandom(void)
{
    #define         RANDOM_SAMPLES 10
    #define         START_SEED64   UINT64SUFFIX(0x123456789ABCEDF0);
    #define         START_SEED32   0x12345678;

    D_UINT32        aulSamples[RANDOM_SAMPLES];
    D_UINT64        aullSamples[RANDOM_SAMPLES];
    D_UINT32        aulNullSamples[RANDOM_SAMPLES];
    D_UINT64        aullNullSamples[RANDOM_SAMPLES];

    D_UINT32        ulSeed;
    D_UINT64        ullSeed;

    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    unsigned        index;

    DclPrintf("Testing Random number generator...\n");

    ulSeed = START_SEED32;
    ullSeed = START_SEED64;
    for(index=0; index < RANDOM_SAMPLES; ++index)
    {
        aulSamples[index] = DclRand(&ulSeed);
        aulNullSamples[index] = DclRand(NULL);
        aullSamples[index] = DclRand64(&ullSeed);
        aullNullSamples[index] = DclRand64(NULL);
    }

    /* Make sure the same seed produces the same sequence */
    ulSeed = START_SEED32;
    ullSeed = START_SEED64;
    for(index=0; index < RANDOM_SAMPLES; ++index)
    {
        /* The sequence should be repeatable starting with the same seed */
        if(aulSamples[index] != DclRand(&ulSeed))
        {
            DclPrintf("Warning!  DclRand() given a fixed seed produced non-deterministic results.\n");
            dclStat = DCLSTAT_CURRENTLINE;
            break;
        }
        if(aullSamples[index] != DclRand64(&ullSeed))
        {
            DclPrintf("Warning!  DclRand64() given a fixed seed produced non-deterministic results.\n");
            dclStat = DCLSTAT_CURRENTLINE;
            break;
        }

        /* Make sure we do not get two of the same 'random' numbers back to back */
        if(index && (aulSamples[index] == aulSamples[index-1]))
        {
            DclPrintf("Warning!  DclRand() returned the same number in consecutive calls.\n");
            dclStat = DCLSTAT_CURRENTLINE;
            break;
        }
        if(index && (aullSamples[index] == aullSamples[index-1]))
        {
            DclPrintf("Warning!  DclRand64() returned the same number in consecutive calls.\n");
            dclStat = DCLSTAT_CURRENTLINE;
            break;
        }
        if(index && (aulNullSamples[index] == aulNullSamples[index-1]))
        {
            DclPrintf("Warning!  DclRand() with NULL seed returned the same number in consecutive calls.\n");
            dclStat = DCLSTAT_CURRENTLINE;
            break;
        }
        if(index && (aullNullSamples[index] == aullNullSamples[index-1]))
        {
            DclPrintf("Warning!  DclRand64() with NULL seed returned the same number in consecutive calls.\n");
            dclStat = DCLSTAT_CURRENTLINE;
            break;
        }
    }

    if(dclStat == DCLSTAT_SUCCESS)
        DclPrintf("    OK\n");

    return dclStat;
}


/*-------------------------------------------------------------------
    This test verifies that the hamming code functions properly.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestHamming(void)
{
    long            i;
    DCLTIMESTAMP    t;
    D_UINT32        ulSeed = 1;

    DclPrintf("Testing Hamming Codes...\n");

    DclPrintf("    Exhaustive 1-byte test, 256 iterations\n");

    t = DclTimeStamp();
    for(i = 0; i < 256; ++i)
    {
        unsigned char   trial;
        unsigned char   tempBuff;

        trial = (unsigned char)i;

        if(!HammingHelper(&trial, &tempBuff, sizeof trial))
        {
            DclPrintf("Failure: %02X\n", trial);
            return DCLSTAT_FAILURE;
        }

        DclPrintf("        Hamming code for %lu is %lu\n", i, DclHammingCalculate(&trial, 1, 0));
    }
    DclPrintf("        OK -- Elapsed time %lD seconds\n", DclTimePassed(t)/1000);

    DclPrintf("    Exhaustive 3-byte test, 16-million iterations\n");
    DclPrintf("        ");

    t = DclTimeStamp();
    for(i = 0; i < (16L * 1024 * 1024); ++i)
    {
        unsigned char   trial[3];
        unsigned char   tempBuff[3];

        trial[0] = (unsigned char)i;
        trial[1] = (unsigned char)(i >> 8);
        trial[2] = (unsigned char)(i >> 16);

        if(!HammingHelper(trial, tempBuff, sizeof trial))
        {
            DclPrintf("Failure: %02X %02X %02X\n", trial[0], trial[1], trial[2]);
            return DCLSTAT_FAILURE;
        }

        if(i % (1L << 19) == 0)
        {
            DclPrintf(".");
        }
    }
    DclPrintf("\n");
    DclPrintf("        OK -- Elapsed time %lD seconds\n", DclTimePassed(t)/1000);

    DclPrintf("    Random 8-byte test, 1-million iterations\n");
    DclPrintf("        ");

    t = DclTimeStamp();
    for(i = 0; i < 1000000L; ++i)
    {
        unsigned char   trial[8];
        unsigned char   tempBuff[8];
        unsigned int    j;

        for(j = 0; j < sizeof trial; ++j)
        {
            trial[j] = (unsigned char)DclRand(&ulSeed);
        }

        if(!HammingHelper(trial, tempBuff, sizeof trial))
        {
            DclPrintf("Failure at %ld\n", i);
            return DCLSTAT_FAILURE;
        }

        if(i % 50000L == 0)
        {
            DclPrintf(".");
        }
    }
    DclPrintf("\n");
    DclPrintf("        OK -- Elapsed time %lD seconds\n", DclTimePassed(t)/1000);

    DclPrintf("    Random 524-byte test, 800 iterations\n");
    DclPrintf("        ");

    t = DclTimeStamp();
    for(i = 0; i < 800L; ++i)
    {
        unsigned char   trial[524];
        unsigned char   tempBuff[524];
        unsigned int    j;

        for(j = 0; j < sizeof trial; ++j)
        {
            trial[j] = (unsigned char)DclRand(&ulSeed);
        }

        if(!HammingHelper(trial, tempBuff, sizeof trial))
        {
            DclPrintf("Failure at %ld\n", i);
            return DCLSTAT_FAILURE;
        }

        if(i % 40 == 0)
        {
            DclPrintf(".");
        }
    }
    DclPrintf("\n");
    DclPrintf("        OK -- Elapsed time %lD seconds\n", DclTimePassed(t)/1000);

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestMulDiv(
    DCLTESTINFO    *pTI)
{
    unsigned        nn;
    typedef struct
    {
        D_UINT32    ulBase;
        D_UINT32    ulMultiplier;
        D_UINT32    ulDivisor;
        D_UINT64    ullResult;
    } MULDIVTEST;
    static const MULDIVTEST aCases[] =
    {   /*   ulBase        ulMultiplier     ulDivisor      ullResult    */
        {1,              1,              0,              D_UINT64_MAX   }, /* Causes a "divide-by-zero" warning message */
        {0,              0,              1,              0              },
        {0,              1,              1,              0              },
        {1,              0,              1,              0              },
        {D_UINT32_MAX,   1,              1,              D_UINT32_MAX   },
        {1,              D_UINT32_MAX,   1,              D_UINT32_MAX   },
        {D_UINT32_MAX,   D_UINT32_MAX,   1,              UINT64SUFFIX(0xFFFFFFFF) * UINT64SUFFIX(0xFFFFFFFF) },
        {D_UINT32_MAX,   1,              D_UINT32_MAX,   1              },
        {1,              D_UINT32_MAX,   D_UINT32_MAX,   1              },
        {D_UINT32_MAX,   D_UINT32_MAX,   D_UINT32_MAX,   D_UINT32_MAX   },
        {1,              D_UINT32_MAX-1, D_UINT32_MAX,   0              },
        {D_UINT32_MAX-1, 1,              D_UINT32_MAX,   0              }
    };

    DclPrintf("    Testing DclMulDiv()...\n");

    for(nn=0; nn<DCLDIMENSIONOF(aCases); nn++)
    {
        D_UINT64    ullResult;

        if(pTI->nVerbosity > DCL_VERBOSE_NORMAL)
        {
            DclPrintf("      DclMulDiv(%lX, %lX, %lX) expecting %llX\n",
                aCases[nn].ulBase, aCases[nn].ulMultiplier, aCases[nn].ulDivisor, aCases[nn].ullResult);
        }

        ullResult = DclMulDiv(aCases[nn].ulBase, aCases[nn].ulMultiplier, aCases[nn].ulDivisor);
        if(ullResult != aCases[nn].ullResult)
        {
            DclPrintf("Test failed in case %u DclMulDiv(%lX, %lX, %lX) %llX != %llX\n",
                nn, aCases[nn].ulBase, aCases[nn].ulMultiplier,
                aCases[nn].ulDivisor, ullResult, aCases[nn].ullResult);
            return DCLSTAT_CURRENTLINE;
        }
    }

    DclPrintf("    OK\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestMulDiv64(
    DCLTESTINFO    *pTI)
{
    unsigned        nn;
    typedef struct
    {
        D_UINT64    ullBase;
        D_UINT32    ulMultiplier;
        D_UINT64    ullDivisor;
        D_UINT64    ullResult;
    } MULDIV64TEST;
    static const MULDIV64TEST aCases[] =
    {   /*   ullBase       ulMultiplier    ullDivisor      ullResult      */
        {1,               1,              0,              D_UINT64_MAX    }, /* Causes a "divide-by-zero" warning message */
        {0,               0,              1,              0               },
        {0,               1,              1,              0               },
        {1,               0,              1,              0               },
        {D_UINT64_MAX,    1,              1,              D_UINT64_MAX    },
        {1,               D_UINT32_MAX,   1,              D_UINT32_MAX    },
        {D_UINT64_MAX-32, 32,             32,             D_UINT64_MAX-32 }, /* CODE-PATH #1a */
        {D_UINT64_MAX-32, 33,             32,             D_UINT64_MAX    }, /* CODE-PATH #1a then #4: Causes overflow message #4 */
        {D_UINT64_MAX,    D_UINT32_MAX,   1,              D_UINT64_MAX    }, /* CODE-PATH #2: Causes overflow message #2 */
        {D_UINT64_MAX,    D_UINT32_MAX-1, 2,              D_UINT64_MAX    }, /* CODE-PATH #1b then #2: Causes overflow message #2 */
        {D_UINT64_MAX,    D_UINT32_MAX-3, 11,             D_UINT64_MAX    }, /* CODE-PATH #3: Causes overflow message #3 */
        {D_UINT64_MAX,    D_UINT32_MAX,   13,             D_UINT64_MAX    }, /* CODE-PATH #4: Causes overflow message #4 */
        {D_UINT64_MAX,    1,              D_UINT64_MAX,   1               },
        {1,               D_UINT32_MAX,   D_UINT64_MAX,   0               },
        {1,               D_UINT32_MAX,   D_UINT32_MAX,   1               },
        {1,               D_UINT32_MAX,   UINT64SUFFIX(0x100000000), 0    },
        {1,               D_UINT32_MAX,   D_UINT32_MAX-1, 1               },
        {D_UINT64_MAX,    D_UINT32_MAX,   D_UINT64_MAX,   D_UINT32_MAX    },
        {1,               D_UINT32_MAX-1, D_UINT64_MAX,   0               },
        {D_UINT64_MAX-1,  1,              D_UINT64_MAX,   0               },
        {10,              16000000UL,     24000000UL,     6               }
    };

    /*  Note that the test scenarios list above, particularly those
        identified with a particular code path, are explicitly set up
        to test code paths inside the implementation of DclMulDiv64().
        See that code for more details.
    */
    DclPrintf("    Testing DclMulDiv64()...\n");

    for(nn=0; nn<DCLDIMENSIONOF(aCases); nn++)
    {
        D_UINT64    ullResult;

        if(pTI->nVerbosity > DCL_VERBOSE_NORMAL)
        {
            DclPrintf("      DclMulDiv64(%llX, %lX, %llX) expecting %llX\n",
                aCases[nn].ullBase, aCases[nn].ulMultiplier, aCases[nn].ullDivisor, aCases[nn].ullResult);
        }

        ullResult = DclMulDiv64(aCases[nn].ullBase, aCases[nn].ulMultiplier, aCases[nn].ullDivisor);
        if(ullResult != aCases[nn].ullResult)
        {
            DclPrintf("Test failed in case %u DclMulDiv64(%llX, %lX, %llX) %llX != %llX\n",
                nn, aCases[nn].ullBase, aCases[nn].ulMultiplier,
                aCases[nn].ullDivisor, ullResult, aCases[nn].ullResult);
            return DCLSTAT_CURRENTLINE;
        }
    }

    DclPrintf("    OK\n");

    return DCLSTAT_SUCCESS;
}


#if DCL_NATIVE_64BIT_SUPPORT

/*-------------------------------------------------------------------
    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestMathPerformance(void)
{
    DCLSTATUS   dclStat;
    D_UINT32    ulOverheadMS = 0;
    D_UINT32    ulOverheadNS = 0;
    D_UINT32    ulLoops = 1000;

    /*  The test period, measured in nanoseconds, must be able to fit
        in a D_UINT32 variable -- assert it so...
    */
    DclAssert(MATH_TEST_SECONDS <= 4);

    DclPrintf("    Testing math performance...\n");

    /*  All the sub-functions use the same loop logic -- calibrate the
        overhead one time, and pass it into those functions.
    */
    DclPrintf("      Calibrating loop overhead...\n");

    while(TRUE)
    {
        DCLTIMESTAMP    t;
        DCLTIMESTAMP    tPrev;
        D_UINT32        ulTmpCount = ulLoops;
        D_UINT32        ulTotal = 0;

        /*  For this to work accurately on fast and slow systems, we may
            need to try several times to get a sample size which is
            large enough.
        */

        /*  NOTE: The logic inside this loop must exactly the same as that
                  used in the various sub-functions, except for the actual
                  operation being timed.  Don't change this function without
                  also dealing with the others!
        */

        /*  Wait until the tick changes...
        */
        tPrev = DclTimeStamp();
        while((t = DclTimeStamp()) == tPrev);

        /*  Note that the check against D_UINT32_MAX in the following
            statement will never fail.  This loop only exits when
            ulTmpCount becomes zero.
        */
        while((ulOverheadMS = DclTimePassed(t)) < D_UINT32_MAX)
        {
            D_UINT32    ulTmpNum = 100;
            unsigned    nn;

            for(nn=0; nn<MATH_OPS_PER_LOOP; nn++)
            {
                /*  Do similar stuff inside this loop as the real
                    tests -- just not the operation being timed.
                */
                ulTotal += ulTmpNum;
                ulTmpNum++;
            }

            ulTmpCount--;

            if(!ulTmpCount)
                break;
        }

        /*  If it took at least a second to execute the loop above, then
            the sample is large enough to be valid.
        */
        if(ulOverheadMS >= 1000)
            break;

        /*  Otherwise, increase the loop count and try again...
        */
        ulLoops *= 2;
    }

    DCLPRINTF(2, ("Overhead MS=%lU for %lU loops\n", ulOverheadMS, ulLoops));
    DclAssert(ulLoops);

    if(ulOverheadMS > D_UINT32_MAX - 1000000)
        ulOverheadNS = ulOverheadMS * (1000000 / ulLoops);
    else
        ulOverheadNS = (ulOverheadMS * 1000000) / ulLoops;

    /*  Arbitrarily reduce the overhead by 12.5%
    */
/*    ulOverheadNS -= (ulOverheadNS >> 3);   */

    DclPrintf("        Loop overhead is %lU ns per iteration (%u operations per loop)\n",
        ulOverheadNS, MATH_OPS_PER_LOOP);

    dclStat = TestDivisionPerformance(ulOverheadNS);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = TestMultiplicationPerformance(ulOverheadNS);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = TestRightShiftPerformance(ulOverheadNS);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = TestLeftShiftPerformance(ulOverheadNS);

    return dclStat;
}


/*-------------------------------------------------------------------
    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestDivisionPerformance(
    D_UINT32        ulOverheadNS)
{
    unsigned        nDivisor;
    D_UINT32        ulTmpNS;

    DclPrintf("      Testing division performance...\n");

    for(nDivisor=2; nDivisor<=10; nDivisor++)
    {
        DCLTIMESTAMP    t;
        DCLTIMESTAMP    tPrev;
        D_UINT32        ulLoops = 0;
        D_UINT32        ulResult;
        D_UINT32        ulTotal = 0;

        /*  Wait until the tick changes...
        */
        tPrev = DclTimeStamp();
        while((t = DclTimeStamp()) == tPrev);

        while(DclTimePassed(t) < MATH_TEST_SECONDS * 1000)
        {
            D_UINT32    ulNum = 100;    /* arbitrary starting dividend */
            unsigned    nn;

            /*  NOTE: The logic inside this loop must exactly the same as
                      that in the calibration loop, except for the actual
                      operation being timed.  Don't change this loop without
                      also dealing with the others!
            */

            for(nn=0; nn<MATH_OPS_PER_LOOP; nn++)
            {
                /*  Do the actual operation being timed.
                */
                ulResult = ulNum / nDivisor;

                /*  Do something with the result so it does not appear like
                    we are throwing it away.
                */
                ulTotal += ulResult;

                /*  Increment the number we're operating on so it won't be
                    the same every time through the loop.
                */
                ulNum++;
            }

            /*  Count how many times we have looped.
            */
            ulLoops++;

            /*  This is a bogus check which should never fail.  It is here
                to ensure that the logic in this loop is the same as that
                in the calibration code.
            */
            if(!ulNum)
                break;
        }

/*        DclPrintf("Loops=%lU\n", ulLoops); */

        DclAssert(ulLoops);
        ulTmpNS = (MATH_TEST_SECONDS * 1000000000) / ulLoops;
        if(ulTmpNS < ulOverheadNS)
            ulTmpNS = ulOverheadNS;

        DclPrintf("        Performed %9lU divide by %2u in %u seconds -- %5lu ps per operation\n",
            ulLoops*MATH_OPS_PER_LOOP, nDivisor, MATH_TEST_SECONDS,
            ((ulTmpNS - ulOverheadNS)*1000) / MATH_OPS_PER_LOOP);
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestMultiplicationPerformance(
    D_UINT32        ulOverheadNS)
{
    unsigned        nFactor;
    D_UINT32        ulTmpNS;

    DclPrintf("      Testing multiplication performance...\n");

    for(nFactor=2; nFactor<=10; nFactor++)
    {
        DCLTIMESTAMP    t;
        DCLTIMESTAMP    tPrev;
        D_UINT32        ulLoops = 0;
        D_UINT32        ulResult;
        D_UINT32        ulTotal = 0;

        /*  Wait until the tick changes...
        */
        tPrev = DclTimeStamp();
        while((t = DclTimeStamp()) == tPrev);

        while(DclTimePassed(t) < MATH_TEST_SECONDS * 1000)
        {
            D_UINT32    ulNum = 100;    /* arbitrary starting value */
            unsigned    nn;

            /*  NOTE: The logic inside this loop must exactly the same as
                      that in the calibration loop, except for the actual
                      operation being timed.  Don't change this loop without
                      also dealing with the others!
            */

            for(nn=0; nn<MATH_OPS_PER_LOOP; nn++)
            {
                /*  Do the actual operation being timed.
                */
                ulResult = ulNum * nFactor;

                /*  Do something with the result so it does not appear like
                    we are throwing it away.
                */
                ulTotal += ulResult;

                /*  Increment the number we're operating on so it won't be
                    the same every time through the loop.
                */
                ulNum++;
            }

            /*  Count how many times we have looped.
            */
            ulLoops++;

            /*  This is a bogus check which should never fail.  It is here
                to ensure that the logic in this loop is the same as that
                in the calibration code.
            */
            if(!ulNum)
                break;
        }

/*        DclPrintf("Loops=%lU\n", ulLoops); */

        DclAssert(ulLoops);
        ulTmpNS = (MATH_TEST_SECONDS * 1000000000) / ulLoops;
        if(ulTmpNS < ulOverheadNS)
            ulTmpNS = ulOverheadNS;

        DclPrintf("        Performed %9lU multiply by %2u in %u seconds -- %5lu ps per operation\n",
            ulLoops*MATH_OPS_PER_LOOP, nFactor, MATH_TEST_SECONDS,
            ((ulTmpNS - ulOverheadNS)*1000) / MATH_OPS_PER_LOOP);
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestRightShiftPerformance(
    D_UINT32        ulOverheadNS)
{
    unsigned        nShift;
    D_UINT32        ulTmpNS;

    DclPrintf("      Testing right-shift performance...\n");

    for(nShift=1; nShift<=31; nShift++)
    {
        DCLTIMESTAMP    t;
        DCLTIMESTAMP    tPrev;
        D_UINT32        ulLoops = 0;
        D_UINT32        ulResult;
        D_UINT32        ulTotal = 0;

        /*  Wait until the tick changes...
        */
        tPrev = DclTimeStamp();
        while((t = DclTimeStamp()) == tPrev);

        while(DclTimePassed(t) < MATH_TEST_SECONDS * 1000)
        {
            D_UINT32    ulNum = 100;    /* arbitrary starting value */
            unsigned    nn;

            /*  NOTE: The logic inside this loop must exactly the same as
                      that in the calibration loop, except for the actual
                      operation being timed.  Don't change this loop without
                      also dealing with the others!
            */

            for(nn=0; nn<MATH_OPS_PER_LOOP; nn++)
            {
                /*  Do the actual operation being timed.
                */
                ulResult = ulNum >> nShift;

                /*  Do something with the result so it does not appear like
                    we are throwing it away.
                */
                ulTotal += ulResult;

                /*  Increment the number we're operating on so it won't be
                    the same every time through the loop.
                */
                ulNum++;
            }

            /*  Count how many times we have looped.
            */
            ulLoops++;

            /*  This is a bogus check which should never fail.  It is here
                to ensure that the logic in this loop is the same as that
                in the calibration code.
            */
            if(!ulNum)
                break;
        }

/*        DclPrintf("Loops=%lU\n", ulLoops); */

        DclAssert(ulLoops);
        ulTmpNS = (MATH_TEST_SECONDS * 1000000000) / ulLoops;
        if(ulTmpNS < ulOverheadNS)
            ulTmpNS = ulOverheadNS;

        DclPrintf("        Performed %9lU right-shift by %2u in %u seconds -- %5lu ps per operation\n",
            ulLoops*MATH_OPS_PER_LOOP, nShift, MATH_TEST_SECONDS,
            ((ulTmpNS - ulOverheadNS)*1000) / MATH_OPS_PER_LOOP);
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestLeftShiftPerformance(
    D_UINT32        ulOverheadNS)
{
    unsigned        nShift;
    D_UINT32        ulTmpNS;

    DclPrintf("      Testing left-shift performance...\n");

    for(nShift=1; nShift<=31; nShift++)
    {
        DCLTIMESTAMP    t;
        DCLTIMESTAMP    tPrev;
        D_UINT32        ulLoops = 0;
        D_UINT32        ulResult;
        D_UINT32        ulTotal = 0;

        /*  Wait until the tick changes...
        */
        tPrev = DclTimeStamp();
        while((t = DclTimeStamp()) == tPrev);

        while(DclTimePassed(t) < MATH_TEST_SECONDS * 1000)
        {
            D_UINT32    ulNum = 100;    /* arbitrary starting value */
            unsigned    nn;

            /*  NOTE: The logic inside this loop must exactly the same as
                      that in the calibration loop, except for the actual
                      operation being timed.  Don't change this loop without
                      also dealing with the others!
            */

            for(nn=0; nn<MATH_OPS_PER_LOOP; nn++)
            {
                /*  Do the actual operation being timed.
                */
                ulResult = ulNum << nShift;

                /*  Do something with the result so it does not appear like
                    we are throwing it away.
                */
                ulTotal += ulResult;

                /*  Increment the number we're operating on so it won't be
                    the same every time through the loop.
                */
                ulNum++;
            }

            /*  Count how many times we have looped.
            */
            ulLoops++;

            /*  This is a bogus check which should never fail.  It is here
                to ensure that the logic in this loop is the same as that
                in the calibration code.
            */
            if(!ulNum)
                break;
        }

/*        DclPrintf("Loops=%lU\n", ulLoops); */

        DclAssert(ulLoops);
        ulTmpNS = (MATH_TEST_SECONDS * 1000000000) / ulLoops;
        if(ulTmpNS < ulOverheadNS)
            ulTmpNS = ulOverheadNS;

        DclPrintf("        Performed %9lU left-shift by %2u in %u seconds -- %5lu ps per operation\n",
            ulLoops*MATH_OPS_PER_LOOP, nShift, MATH_TEST_SECONDS,
            ((ulTmpNS - ulOverheadNS)*1000) / MATH_OPS_PER_LOOP);
    }

    return DCLSTAT_SUCCESS;
}

#endif  /* DCL_NATIVE_64BIT_SUPPORT */


/*-------------------------------------------------------------------
    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestInsertionSort(void)
{
    #define         MAX_SORT_VAL    1024
    D_UINT32        aul[256];
    D_UINT64        ullSeed = 0;
    unsigned        nn;

    DclPrintf("Testing Insertion Sort32...\n");

    /*  Iterate through a bunch of list lengths
    */
    for(nn = 0; nn < DCLDIMENSIONOF(aul); nn++)
    {
        D_UINT32    vv;

        /*  For each list length, try a varying range of random values, to
            ensure that there are plenty of duplicates.
        */            
        for(vv = 10; vv < MAX_SORT_VAL; vv++)
        {
            size_t      nMoves;
            unsigned    ii;

            /*  Build the list
            */
            for(ii = 0; ii < nn; ii++)
            {
                aul[ii] = (D_UINT32)(DclRand64(&ullSeed) % vv);
            }

            /*  Sort it
            */
            nMoves = DclSortInsertion32(aul, nn);
            (void)nMoves;    /* Unused for the moment */

            /*  Confirm that it is sorted
            */
            for(ii = 1; ii < nn; ii++)
            {
                if(aul[ii] < aul[ii-1])
                    DclProductionError();
            }
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    This is a helper function used by TestHamming().

    Parameters:

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
-------------------------------------------------------------------*/
static D_BOOL HammingHelper(
    const void     *pData,
    void           *pTempBuff,
    size_t          length)
{
    unsigned char  *pCopy = (unsigned char *)pTempBuff;
    unsigned long   parity = DclHammingCalculate(pData, length, 0);
    unsigned long   check;
    unsigned int    i, bit;

    DclAssert(pData);
    DclAssert(pTempBuff);
    DclAssert(length);

    check = DclHammingCalculate(pData, length, parity);
    if(check != 0)
    {
        DclPrintf("Basic check failure: got %lx\n", check);
        return FALSE;
    }

    DclMemCpy(pCopy, pData, length);
    bit = 1;

    for(i = 0; i < length; ++i)
    {
        unsigned char   mask;

        for(mask = 1; mask; mask <<= 1)
        {
            while(DCLISPOWEROF2(bit))
            {
                check = DclHammingCalculate(pCopy, length, parity ^ bit);
                if(check != bit)
                {
                    DclPrintf("Bit error failure: bit %d gave %lx\n", bit, check);
                    return FALSE;
                }
                ++bit;
            }

            pCopy[i] ^= mask;
            check = DclHammingCalculate(pCopy, length, parity);
            if(check != bit)
            {
                DclPrintf("Bit error failure: bit %d gave %lx\n", bit, check);
                return FALSE;
            }

            DclHammingCorrect(pCopy, length, check);
            if(DclMemCmp(pData, pCopy, length) != 0)
            {
                DclPrintf("Correction failure: byte %d mask %02x\n", i, (int)mask);
                return FALSE;
            }
            ++bit;
        }
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    This test verifies that the CRC code functions properly.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestCrc32(void)
{
    D_UINT32  ul32Crc;
    D_BUFFER  rbDataBlock[CRC_CHECK_LENGTH_2];

    DclPrintf("Testing CRC32 calculations... \n");
    DclPrintf("      (check string: \'%s\')\n", CRC_CHECK_STRING);

    DclMemCpy(rbDataBlock, CRC_CHECK_STRING, CRC_CHECK_LENGTH_2);

    /*  Test the CCITT-32 CRC with a zero buffer
    */
    ul32Crc = DclCrc32Update(0, rbDataBlock, 0);
    if(ul32Crc != 0)
    {
        DclPrintf("    Failed! calculated CRC: %lX  expected: %lX\n",
            ul32Crc, (D_UINT32) 0);
        return DCLSTAT_CURRENTLINE;
    }

    /*  Check the first string / length
    */
    ul32Crc = DclCrc32Update(0, rbDataBlock, CRC_CHECK_LENGTH_1);
    if(ul32Crc != CRC_CHECK_CCITT32_1)
    {
        DclPrintf("    Failed! calculated CRC: %lX  expected: %lX\n",
            ul32Crc, CRC_CHECK_CCITT32_1);
        return DCLSTAT_CURRENTLINE;
    }

    /*  Check the second passing the first as a parameter
    */
    ul32Crc = DclCrc32Update(ul32Crc, rbDataBlock, CRC_CHECK_LENGTH_1);
    if(ul32Crc != CRC_CHECK_CCITT32_2)
    {
        DclPrintf("    Failed! calculated CRC: %lX  expected: %lX\n",
            ul32Crc, CRC_CHECK_CCITT32_2);
        return DCLSTAT_CURRENTLINE;
    }

    /*  Check the second passing the entire buffer for calculation
    */
    ul32Crc = DclCrc32Update(0, rbDataBlock, CRC_CHECK_LENGTH_2);
    if(ul32Crc != CRC_CHECK_CCITT32_2)
    {
        DclPrintf("    Failed! calculated CRC: %lX  expected: %lX\n",
            ul32Crc, CRC_CHECK_CCITT32_2);
        return DCLSTAT_CURRENTLINE;
    }

    DclPrintf("    OK\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    This test verifies that the CRC code functions properly.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the test result.
-------------------------------------------------------------------*/
static DCLSTATUS TestCrc16(void)
{
    D_UINT16  u16Crc;
    D_BUFFER  rbDataBlock[CRC_CHECK_LENGTH_2];

    DclPrintf("Testing CRC16 calculations... \n");
    DclPrintf("      (check string: \'%s\')\n", CRC_CHECK_STRING);

    DclMemCpy(rbDataBlock, CRC_CHECK_STRING, CRC_CHECK_LENGTH_2);

    /*  Test the CCITT-32 CRC with a zero buffer
    */
    u16Crc = DclCrc16Update(0, rbDataBlock, 0);
    if(u16Crc != 0)
    {
        DclPrintf("    Failed! calculated CRC: %X  expected: %X\n",
            u16Crc, (D_UINT16) 0);
        return DCLSTAT_CURRENTLINE;
    }

    /*  Check the first string / length
    */
    u16Crc = DclCrc16Update(0, rbDataBlock, CRC_CHECK_LENGTH_1);
    if(u16Crc != CRC_CHECK_CRC16_1)
    {
        DclPrintf("    Failed! calculated CRC: %X  expected: %X\n",
            u16Crc, CRC_CHECK_CRC16_1);
        return DCLSTAT_CURRENTLINE;
    }

    /*  Check the second passing the first as a parameter
    */
    u16Crc = DclCrc16Update(u16Crc, rbDataBlock, CRC_CHECK_LENGTH_1);
    if(u16Crc != CRC_CHECK_CRC16_2)
    {
        DclPrintf("    Failed! calculated CRC: %X  expected: %X\n",
            u16Crc, CRC_CHECK_CRC16_2);
        return DCLSTAT_CURRENTLINE;
    }

    /*  Check the second passing the entire buffer for calculation
    */
    u16Crc = DclCrc16Update(0, rbDataBlock, CRC_CHECK_LENGTH_2);
    if(u16Crc != CRC_CHECK_CRC16_2)
    {
        DclPrintf("    Failed! calculated CRC: %X  expected: %X\n",
            u16Crc, CRC_CHECK_CRC16_2);
        return DCLSTAT_CURRENTLINE;
    }

    DclPrintf("    OK\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_UINT32 CalculatePeriod(
    D_UINT64   *pullMilliSecs,
    char      **pBuffer)
{
    D_UINT64    ullSecs;

    DclAssert(pullMilliSecs);
    DclAssert(pBuffer);

    if(DclUint64LessUint32(pullMilliSecs, 1000UL * 30))         /* < 30 seconds */
    {
        *pBuffer = "ms";
        return DclUint32CastUint64(pullMilliSecs);
    }

    /*  Convert milliseconds to seconds.  Need to do that at this
        point because all the cascading multipliers below must
        fit into a D_UINT32 value.
    */
    ullSecs = *pullMilliSecs;
    DclUint64DivUint32(&ullSecs, 1000UL);

    if(DclUint64LessUint32(&ullSecs, 60UL * 100))               /* < 100 minutes */
    {
        *pBuffer = "seconds";
    }
    else if(DclUint64LessUint32(&ullSecs, 60UL * 60 * 20))      /* < 20 hours */
    {
        DclUint64DivUint32(&ullSecs, 60UL);
        *pBuffer = "minutes";
    }
    else if(DclUint64LessUint32(&ullSecs, 60UL * 60 * 24 * 20)) /* < 20 days */
    {
        DclUint64DivUint32(&ullSecs, 60UL * 60);
        *pBuffer = "hours";
    }
    else if(DclUint64LessUint32(&ullSecs, 60UL * 60 * 24 * 365 * 20)) /* < 20 years */
    {
        DclUint64DivUint32(&ullSecs, 60UL * 60 * 24);
        *pBuffer = "days";
    }
    else                                                        /* >= 20 years */
    {
        DclUint64DivUint32(&ullSecs, 60UL * 60 * 24 * 365);
        *pBuffer = "years";
    }

    return DclUint32CastUint64(&ullSecs);
}


/*-------------------------------------------------------------------
    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS SprintfHelper(
    const char *pszTitle,
    unsigned    nBuffLen,
    const char *pszFmt,
    const char *pData,
    const char *pszResult,
    int         iExpectedLen,
    unsigned    nOverflowLen)
{
    char        buffer[SPRINTFBUFFLEN];
    int         iLen;

  #if DCL_OSFEATURE_UNICODE && !TEST_MS_SNPRINTF
    D_WCHAR     tzBuff[SPRINTFBUFFLEN];

    /*  If Unicode stuff is enabled, and there are any 'W' characters,
        convert the supplied ANSI data string into Unicode, and redirect
        the pData pointer to use the converted string.
    */
    if(DclStrChr(pszFmt, 'W'))
    {
        unsigned    nLen;

        nLen = DclOsAnsiToWcs(tzBuff, DCLDIMENSIONOF(tzBuff), pData, -1);
        DclProductionAssert(nLen);

        pData = (const char*)tzBuff;
    }
  #endif

    DclAssert(nOverflowLen < sizeof(buffer));

    DclPrintf("      %s...\n", pszTitle);

    /*  Initialize the buffer to a known value
    */
    DclMemSet(buffer, '1', sizeof(buffer));

    buffer[nOverflowLen] = MAGICCHAR;
    iLen = DclSNPrintf(buffer, nBuffLen, pszFmt, pData);

    if(iLen != iExpectedLen)
    {
        DclPrintf("      FAILED! Length is bad, expected %d, got %d\n", iExpectedLen, iLen);
        return DCLSTAT_CURRENTLINE;
    }

    if(iLen == -1)
    {
        if(DclStrNCmp(buffer, pszResult, nOverflowLen) != 0)
        {
            DclPrintf("      FAILED! String mismatch (A)\n");
            return DCLSTAT_CURRENTLINE;
        }
    }
    else
    {
        if(DclStrCmp(buffer, pszResult) != 0)
        {
            DclPrintf("      FAILED! String mismatch (B)\n");
            return DCLSTAT_CURRENTLINE;
        }
    }

    if(buffer[nOverflowLen] != MAGICCHAR)
    {
        DclPrintf("      FAILED! Buffer overflow\n");
        return DCLSTAT_CURRENTLINE;
    }

    return DCLSTAT_SUCCESS;
}



/*-------------------------------------------------------------------
    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS DelayHelper(
    D_UINT32        ulTotalMicrosecs,
    unsigned        nIterations,
    D_UINT32        ulMinDelayNS,
    D_UINT32        ulDesiredSystemTicks)
{
    D_UINT32        ulStartTicks;
    D_UINT32        ulElapsedTicks;
    D_UINT32        ulTotalNanosecs;
    D_UINT64        ullElapsedNS;
    DCLSTATUS       dclStat;
    DCLTIMESTAMP    ts;

    DclAssert(ulTotalMicrosecs);
    DclAssert(ulTotalMicrosecs <= (D_UINT32_MAX/1000));
    DclAssert(nIterations);
    DclAssert(ulMinDelayNS);
    DclAssert(ulDesiredSystemTicks);

    ulTotalNanosecs = (ulTotalMicrosecs * 1000) / nIterations;

    /*  If the delay is too short for the system's HighRes ticker, use the
        smallest allowable delay, and adjust the iterations accordingly
        so the total time in system ticks remains constant.
    */
    if(ulTotalNanosecs < ulMinDelayNS)
    {
        ulTotalNanosecs = ulMinDelayNS;

        nIterations = (ulTotalMicrosecs * 1000) / ulTotalNanosecs;
    }

    DclPrintf("    Starting %u sequential %lU ns delays (~%lU system ticks)...\n",
        nIterations, ulTotalNanosecs, ulDesiredSystemTicks);

    ulStartTicks = DclOsTickCount();
    ts = DclTimeStamp();
    {
        unsigned    kk;

        for(kk = 0; kk < nIterations; kk++)
        {
            dclStat = DclNanosecondDelay(ulTotalNanosecs);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("    DclNanosecondDelay(%lU) failed with code %lX\n",
                    ulTotalNanosecs, dclStat);

                return dclStat;
            }
        }
    }
    ullElapsedNS = DclTimePassedNS(ts);
    ulElapsedTicks = DclOsTickCount() - ulStartTicks;

    DclPrintf("      Elapsed: System ticks=%lU, HighRes nanoseconds=%llU\n",
        ulElapsedTicks, VA64BUG(ullElapsedNS));

    if(ulElapsedTicks > ulDesiredSystemTicks + 1)
    {
        DclPrintf("      NOTE: Requested a total delay of %lU ticks, but %lU ticks elapsed.\n",
             ulDesiredSystemTicks, ulElapsedTicks);
    }
    else if(ulElapsedTicks < ulDesiredSystemTicks)
    {
        /*  The total delay, according to the system ticker, was less than
            what we expected.  Unfortunately, on some hardware, querying
            a high resolution counter in a tight loop can have a detrimental
            affect on the regular system counter (probably due to having to
            disable interrupts to atomically update the HighRes counter).

            It's not ideal to use the HighRes ticker to measure the accuracy
            of the nanosecond delay functions, because it (in most cases) is
            using the same timer to measure itself.  However, because we have
            already tested the HighRes ticker, earlier in this test suite,
            (and presumably it passed), we're going to use it to decide if
            the delay functions really are working.
        */
        if(DclUint64GreaterUint32(&ullElapsedNS, ulTotalNanosecs))
        {
            DclPrintf("      Elapsed system ticks is low, but the timestamp is OK.\n");
        }
        else
        {
            /*  If we get here, neither the system ticker nor the HighRes
                ticker thought enough time elapsed.
            */
            DclPrintf("      Failed! Requested a total delay of %lU ticks, but only %lU ticks elapsed.\n",
                ulDesiredSystemTicks, ulElapsedTicks);
            DclPrintf("              At least %lU HighRes nanoseconds should have elapsed.\n",
                ulTotalNanosecs);

            return DCLSTAT_CURRENTLINE;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS ECCHelper(
    D_BUFFER       *pBuff,
    size_t          nLen)
{
    D_BUFFER       *pDuplicate;
    D_UINT32        ulOriginalECC;
    D_UINT32        ulOldECC;
    D_UINT32        ulNewECC;
    D_UINT32        ulValidMask;
    DCLECCRESULT    iResult;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    unsigned        nBitAddress;
    unsigned        nn;

    DclAssertWritePtr(pBuff, nLen);

    pDuplicate = DclMemAlloc(nLen);
    if(!pDuplicate)
    {
        DclPrintf("Out of memory!\n");
        return DCLSTAT_OUTOFMEMORY;
    }

    /*  Make a duplicate copy of the data for comparisons
    */
    DclMemCpy(pDuplicate, pBuff, nLen);

    ulValidMask = (D_UINT32_MAX >> (32-((DclBitHighest(nLen) + 2) << 1)));

            /*---------------------------------------------------*\
             *  Test Case: No correction needed                  *
            \*---------------------------------------------------*/

    ulOldECC = DclEccCalculate(pBuff, nLen);
    ulOriginalECC = ulOldECC;

    if((ulOldECC & ~ulValidMask) != 0)
    {
        DclPrintf("Invalid ECC generated, ECC=%lX\n", ulOldECC);
        dclStat = DCLSTAT_CURRENTLINE;
        goto Cleanup;
    }

    ulNewECC = DclEccCalculate(pBuff, nLen);
    DclAssert(ulNewECC == ulOldECC);

    iResult = DclEccCorrect(pBuff, nLen, &ulOldECC, ulNewECC);
    if(iResult != DCLECC_VALID)
    {
        DclPrintf("Expected result %d, got result %d\n", DCLECC_VALID, iResult);
        dclStat = DCLSTAT_CURRENTLINE;
        goto Cleanup;
    }

            /*---------------------------------------------------*\
             *  Test Case: Correct a bad ECC                     *
            \*---------------------------------------------------*/

    ulOldECC ^= 1 << DclRand(NULL) % 6;

    ulNewECC = DclEccCalculate(pBuff, nLen);

    iResult = DclEccCorrect(pBuff, nLen, &ulOldECC, ulNewECC);
    if(iResult != DCLECC_ECCCORRECTED)
    {
        DclPrintf("Expected result %d, got result %d\n", DCLECC_ECCCORRECTED, iResult);
        dclStat = DCLSTAT_CURRENTLINE;
        goto Cleanup;
    }

    if(ulOldECC != ulOriginalECC)
    {
        DclPrintf("Expected the ECC value to be corrected\n");
        dclStat = DCLSTAT_CURRENTLINE;
        goto Cleanup;
    }

    if(DclMemCmp(pBuff, pDuplicate, nLen) != 0)
    {
        DclPrintf("Buffer comparison failed (A)\n");
        dclStat = DCLSTAT_CURRENTLINE;
        goto Cleanup;
    }

            /*---------------------------------------------------*\
             *  Test Case: Data and the ECC are bad              *
            \*---------------------------------------------------*/

    ulOldECC ^= 1 << DclRand(NULL) % 6;

    nBitAddress = DclRand(NULL) % (nLen << 3);

    /*  Flip one bit
    */
    pBuff[nBitAddress >> 3] ^= (1 << (nBitAddress & 7));

    ulNewECC = DclEccCalculate(pBuff, nLen);
    DclAssert(ulNewECC != ulOldECC);

    iResult = DclEccCorrect(pBuff, nLen, &ulOldECC, ulNewECC);
    if(iResult != DCLECC_UNCORRECTABLE)
    {
        DclPrintf("Expected result %d, got result %d\n", DCLECC_UNCORRECTABLE, iResult);
        dclStat = DCLSTAT_CURRENTLINE;
        goto Cleanup;
    }

    /*  Now really correct it for the following test
    */
    ulOldECC = ulOriginalECC;
    iResult = DclEccCorrect(pBuff, nLen, &ulOldECC, ulNewECC);
    if(iResult != DCLECC_DATACORRECTED)
    {
        DclPrintf("Expected result %d, got result %d\n", DCLECC_DATACORRECTED, iResult);
        dclStat = DCLSTAT_CURRENTLINE;
        goto Cleanup;
    }

            /*---------------------------------------------------*\
             *  Test Case: Correct random errors in the data     *
            \*---------------------------------------------------*/

    for(nn=0; nn<nLen; nn++)
    {
        /*  Pick a random bit to flip.  Special case the first two
            iterations -- make sure the lowest and highest possible
            bits are handled.
        */
        if(nn == 0)
            nBitAddress = (nLen << 3) - 1;
        else if(nn == 1)
            nBitAddress = 0;
        else
            nBitAddress = DclRand(NULL) % (nLen << 3);

        /*  Flip one bit
        */
        pBuff[nBitAddress >> 3] ^= (1 << (nBitAddress & 7));

        ulNewECC = DclEccCalculate(pBuff, nLen);
        DclAssert(ulNewECC != ulOldECC);

        iResult = DclEccCorrect(pBuff, nLen, &ulOldECC, ulNewECC);
        if(iResult != DCLECC_DATACORRECTED)
        {
            DclPrintf("Expected result %d, got result %d\n", DCLECC_DATACORRECTED, iResult);
            dclStat = DCLSTAT_CURRENTLINE;
            goto Cleanup;
        }

        /*  Make sure the data really was corrected, and that no other
            bits were messed with.
        */
        if(DclMemCmp(pBuff, pDuplicate, nLen) != 0)
        {
            DclPrintf("Buffer comparison failed (B)\n");
            dclStat = DCLSTAT_CURRENTLINE;
            goto Cleanup;
        }
    }

  Cleanup:
    DclMemFree(pDuplicate);

    return dclStat;
}


/*-------------------------------------------------------------------
    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL BitArrayInitialize(
    void       *pBuff,
    unsigned    nBits,
    unsigned    nSentinelBytes,
    unsigned    nArrayBytes)
{
    D_BUFFER   *paBytes = pBuff;
    unsigned    nn;

    DclAssert(nBits);
    DclAssert(nSentinelBytes);
    DclAssert(nArrayBytes);
    DclAssertWritePtr(paBytes, nSentinelBytes + nArrayBytes + nSentinelBytes);
     
    DclMemSet(paBytes, 0, nSentinelBytes + nArrayBytes + nSentinelBytes);

    /*  Fill the leading and trailing sentinel areas with known
        values so we can check for under/overflow.
    */                
    for(nn=0; nn<nSentinelBytes; nn++)
        paBytes[nn] = (D_UINT8)nn;
    
    for(nn=0; nn<nSentinelBytes; nn++)
        paBytes[nSentinelBytes + nArrayBytes + nn] = (D_UINT8)(nSentinelBytes + nn);

    /*  All bits should be cleared...
    */
    if(DclBitCountArray(&paBytes[nSentinelBytes], nBits))
        return FALSE;

    return TRUE;
}


/*-------------------------------------------------------------------
    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL BitArrayValidate(
    void       *pBuff,
    unsigned    nBits,
    unsigned    nSentinelBytes,
    unsigned    nArrayBytes,
    unsigned    nElementBytes,
    unsigned    nExpectedCount)
{
    D_BUFFER   *paBytes = pBuff;
    unsigned    nCount;
    unsigned    nn;
    D_BOOL      fSuccess = FALSE;
    unsigned    nBaseBytes = (nBits / (nElementBytes * CHAR_BIT)) * nElementBytes;
    D_BUFFER    buff[sizeof(int)];

    DclAssert(nBits);
    DclAssert(nElementBytes);
    DclAssert(nElementBytes <= sizeof(int));
    DclAssert(nSentinelBytes);
    DclAssert(nArrayBytes);
    DclAssertWritePtr(paBytes, nSentinelBytes + nArrayBytes + nSentinelBytes);

  #if DCL_BIG_ENDIAN 
    DclCopyAndReverse(buff, &paBytes[nSentinelBytes + nBaseBytes], nArrayBytes - nBaseBytes);
  #else
    DclMemCpy(buff, &paBytes[nSentinelBytes + nBaseBytes], nArrayBytes - nBaseBytes);
  #endif
  
  #if DCL_BIG_ENDIAN 
    if(nElementBytes > 1)
    {
        unsigned nRemainingBits = nBits - (nBaseBytes * CHAR_BIT);
        
        nCount = DclBitCountArray(&paBytes[nSentinelBytes], nBits - nRemainingBits);
        nCount += DclBitCountArray(buff, nRemainingBits);
    }
    else
  #endif
    {
        nCount = DclBitCountArray(&paBytes[nSentinelBytes], nBits);
    }
  
    if(nCount != nExpectedCount)
    {
        DclPrintf("Expected %u bits set, but found %u bits set\n", nExpectedCount, nCount);
        goto BitValidateCleanup;
    }        

    for(nn=0; nn<nSentinelBytes; nn++)
    {
        if(paBytes[nn] != (D_UINT8)nn)
        {
            DclPrintf("Head sentinel corruption at offset %u\n", nn);
            goto BitValidateCleanup;
        }
    }
    
    for(nn=0; nn<nSentinelBytes; nn++)
    {
        if(paBytes[nSentinelBytes+nArrayBytes+nn] != (D_UINT8)(nSentinelBytes + nn))
        {
            DclPrintf("Tail sentinel corruption at offset %u\n", nn);
            goto BitValidateCleanup;
        }
    }

    /*  The test setup always configures things so that the lowest bit
        of the last ELEMENT is part of the set, and could therefore
        be on or off.  All the other bits must be off -- verify that...
    */                
    if((buff[0] & 0xFE) != 0)
    {
        DclPrintf("Partial element byte 0 corrupted\n");
        goto BitValidateCleanup;
    }
    
    for(nn = 1; nn < nElementBytes; nn++)
    {
        if(buff[nn] != 0)
        {
            DclPrintf("Partial element byte %u corrupted\n", nn);
            goto BitValidateCleanup;
        }
    }

    fSuccess = TRUE;

  BitValidateCleanup:

    if(!fSuccess)
    {
        DclHexDump("Head: ", HEXDUMP_UINT8 | HEXDUMP_NOASCII | HEXDUMP_NOOFFSET, 32, nSentinelBytes, paBytes);
        DclHexDump(NULL,     HEXDUMP_UINT8 | HEXDUMP_NOASCII,                    32, nArrayBytes,    &paBytes[nSentinelBytes]);
        DclHexDump("Tail: ", HEXDUMP_UINT8 | HEXDUMP_NOASCII | HEXDUMP_NOOFFSET, 32, nSentinelBytes, &paBytes[nSentinelBytes + nArrayBytes]);
    }
    
    return fSuccess;
}

