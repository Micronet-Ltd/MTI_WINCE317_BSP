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

    Multi-threaded stress test for FlashFX.  Starts multiple threads, each
    of which makes random calls to FfxVbfReadPages(), FfxVbfWritePages(),
    FfxVbfDiscardPages(), and FfxVbfCompact().  The master thread
    periodically requests the other threads to stop, and remounts the
    disk.

    Operations of the threads are coordinated and tracked.  Reads are checked
    to make sure they return what was last written (or return the fill value
    if the client address was discarded).

    As part of the coordination process, threads claim ownership of the area
    of client address space in which they wish to read, write, or discard.
    A thread may read anywhere, but only verify what was read, written, or
    discarded in an area it owns.

    Relative proportions of the operations may be configured from the command
    line as a comma separated list of values representing the proportions of
    calls to FfxVbfReadPages(), FfxVbfWritePages(), FfxVbfDiscardPages() and
    FfxVbfCompact() in that order.  A value omitted is taken to be zero.  For
    example, "50,25,23,2" means 50% reads, 25% writes, 23% discards, and 2%
    compactions. "5,3" means 5/8 reads, 3/8 writes, no discards or compactions.
    "1,1,,1" means 1/3 reads, 1/3 writes, and 1/3 compactions.

    ToDo:
    - Modify the drive designator parser to use the standard device names as
      specified by the OS Layer.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: mtstrss.c $
    Revision 1.40  2010/12/19 02:40:43Z  garyp
    Changed the max threads from 20 to 50.
    Revision 1.39  2010/12/12 06:54:05Z  garyp
    Updated to reserve portions of the flash for little and no write/discard
    activity, so that wear-leveling will eventually kick in.  Added the /DEBUG
    command-line option.
    Revision 1.38  2010/11/01 13:42:19Z  garyp
    Updated to the new DclLogOpen() calling convention.
    Revision 1.37  2010/09/21 02:26:37Z  garyp
    Synchronized the Disk summary information at cleanup time so it is not
    interleaved when testing with multiple Disks.
    Revision 1.36  2010/01/09 18:03:33Z  garyp
    Minor documentation updates -- no functional changes.
    Revision 1.35  2009/07/22 18:22:38Z  garyp
    Added a variable lost in the merge.
    Revision 1.34  2009/07/21 22:07:11Z  garyp
    Merged from the v4.0 branch.  Updated so that VBF instances are created in
    compaction suspend mode and have to be explicitly resumed.  Fixed a minor 
    parameter parsing issue.  Added the /LOG command-line option to allow all 
    output to be captured, rather than sent to the console.  Modified the 
    shutdown processes to take a mode parameter.  Updated the compaction 
    functions to take a compaction level.  Minor datatype changes from 
    D_UINT16 to unsigned.  Updated to use the revised FfxSignOn() function,
    which now takes an fQuiet parameter.  Updated for new compaction functions
    which now return an FFXIOSTATUS value rather than a D_BOOL.
    Revision 1.33  2009/04/01 14:26:51Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.32  2009/02/09 01:35:39Z  garyp
    Merged from the v4.0 branch.  Modified so the test's "main" function still
    compiles, even if allocator support is disabled.  Updated to use the new
    FFXTOOLPARAMS structure.
    Revision 1.31  2008/05/06 06:10:40Z  garyp
    Documentation correction.
    Revision 1.30  2008/04/14 23:20:56Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.29  2008/02/06 20:21:19Z  Garyp
    Made allocator/VBF code conditional on FFXCONF_ALLOCATORSUPPORT and
    FFXCONF_VBFSUPPORT, respectively.
    Revision 1.28  2008/01/29 03:36:42Z  Garyp
    Increased the number of buffer blocks used by default, to better exercise
    large flash arrays and large I/O requests.
    Revision 1.27  2008/01/13 07:26:54Z  keithg
    Function header updates to support autodoc.
    Revision 1.26  2007/11/02 23:21:46Z  Garyp
    Re-added the inclusion of ctype.h.
    Revision 1.25  2007/11/02 02:52:30Z  Garyp
    Eliminated some unnecessary includes.
    Revision 1.24  2007/06/22 17:26:01Z  Garyp
    Updated to use the new PerfLog "write" macro, as well as the new "Category
    Suffix" field.
    Revision 1.23  2007/04/22 20:06:52Z  Garyp
    Updated to use the DclRatio() function to display percentages.
    Revision 1.22  2007/04/08 22:54:53Z  Garyp
    Aded page size to the PerfLog output.
    Revision 1.21  2007/04/03 20:39:48Z  Garyp
    Major update to support performance and latency measurements.
    Changed the default settings to run for 6 minutes with 4 threads.  Changed
    the default mount (sample) interval to 20 seconds.  Increased the default
    number of buffer blocks to 17.  Updated the startup logic so that all the
    threads start (more or less) simultaneously.  Updated to provide PerfLog
    style output, as well as to provide general performance and latency
    numbers.  Updated so that all test data is instanced.  Updated a number
    of test options to use the standard syntax used by other tools.  Added
    the /NoRemount option to prevent VBF from remounting (which wipes
    out any statistics which were being gathered).  Modified so the maximum
    number of blocks to be processed with any given call is 128.
    Revision 1.20  2007/03/20 20:44:27Z  Garyp
    Updated to use the new "page" oriented VBF interface, rather than the old
    byte oriented interface.  Eliminated logic to break things into the smaller
    chunks required by the old interface, and generally widened the counts to
    be 32-bit values.  General cleanup of the function headers.  That all being
    said, the code is functionally the same as earlier versions.
    Revision 1.19  2006/10/11 19:07:05Z  chrisl
    Remove compiler warning and improve readability.
    Revision 1.18  2006/10/09 22:30:54Z  chrisl
    Entered the correct value for  tot_prob.
    Revision 1.17  2006/10/09 22:26:29Z  chrisl
    Changed the total_probability value to a variable so it doesn't overflow
    the MIPS compiler.
    Revision 1.16  2006/10/02 17:55:39Z  Garyp
    Updated to use DclMemAllocZero().  Updated to use the new printf macros.
    Revision 1.15  2006/08/20 00:16:13Z  Garyp
    Modified to use the DCL time-of-day functionality.
    Revision 1.14  2006/06/12 11:59:14Z  Garyp
    Eliminated the use of the deprecated vbfclientsize().
    Revision 1.13  2006/02/24 23:59:02Z  joshuab
    Added check in mtstress.c so we only destroy the VBF if we created it.
    Revision 1.12  2006/02/21 02:13:01Z  Garyp
    Updated to work with the new VBF API.
    Revision 1.11  2006/02/18 05:34:37Z  Garyp
    Added a /TIME parameter and changed to have a default test execution
    of 5 minutes.
    Revision 1.10  2006/02/16 02:09:20Z  Garyp
    Modified to allow the number of threads to be 1.
    Revision 1.9  2006/02/16 01:55:14Z  Pauli
    Fixed compiler errors, type casting an lvalue, using the Code Sourcery
    ARM gnu tools 3.4.2.
    Revision 1.8  2006/02/15 19:23:26Z  Garyp
    Modified to create a VBF instance if necessary.
    Revision 1.7  2006/02/13 18:58:43Z  Garyp
    Fixed to check for a valid drive before accessing the structure.
    Revision 1.6  2006/01/31 04:44:43Z  Garyp
    Modified to no longer attempt to remount VBF while it is already mounted.
    Revision 1.5  2006/01/13 22:11:34Z  Garyp
    Updated to use the new printf().  General cleanup -- no functional changes.
    Revision 1.4  2006/01/11 02:01:37Z  Garyp
    Modified to work with run-time VBF block size determination.
    Revision 1.3  2006/01/05 04:02:55Z  Garyp
    Changes per Bill and GP.  Updated to eliminate the fixed compile-time
    VBF allocation block size setting.
    Revision 1.2  2005/12/31 03:07:00Z  Garyp
    Updated to use renamed thread related settings and functions, which are
    now a part of DCL.
    Revision 1.1  2005/11/25 23:02:10Z  Pauli
    Initial revision
    Revision 1.3  2005/11/25 23:02:09Z  Garyp
    Modified to use DclRand().
    Revision 1.2  2005/11/07 15:17:27Z  Garyp
    Minor type changes to mollify MSVC6.
    Revision 1.1  2005/10/02 03:24:32Z  Garyp
    Initial revision
    Revision 1.3  2005/09/20 19:01:10Z  pauli
    Replaced _syssleep with DclOsSleep.
    Revision 1.2  2005/09/18 05:55:22Z  garyp
    Renamed vbfcompact() to FfxVbfCompact() and added FfxVbfCompactIfIdle().
    Revision 1.1  2005/08/03 19:31:50Z  pauli
    Initial revision
    Revision 1.55  2005/05/17 18:55:19Z  garyp
    Modified the usage screen to accurately reflect the drive designator, though
    it may be different from the rest of the tools.
    Revision 1.54  2005/03/25 00:14:47Z  PaulI
    Corrected a build warning of mismatched data types in comparison.
    Revision 1.53  2004/12/30 23:17:53Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.52  2004/12/16 08:10:19Z  garys
    Added TOUPPER and TOLOWER macros to allow typecasting for gnu.
    Revision 1.51  2004/11/20 05:18:34  GaryP
    Updated the syntax information.
    Revision 1.50  2004/09/29 06:49:53Z  GaryP
    Minor updates to docs and comments.
    Revision 1.49  2004/09/27 07:49:42Z  GaryP
    Updated to disaplay the usage if no parameters are specified.  Added
    asserts.
    Revision 1.48  2004/09/23 08:13:13Z  GaryP
    Semantic change from "garbage collection" to "compaction".
    Revision 1.47  2004/09/15 23:14:38Z  garys
    fAggressive parameter to FfxVbfCompact() to support background G.C.
    Revision 1.46  2004/09/10 22:22:32  GaryP
    Changed some data types to mollify the MSVC6 compiler.
    Revision 1.45  2004/07/26 16:42:54Z  jaredw
    Added use of DCL_OSFEATURE_THREADS for conditional compilation.
    Revision 1.44  2004/07/22 21:01:48Z  jaredw
    Removed use of 64bit data type in randval function.
    Revision 1.43  2004/07/22 02:02:05Z  GaryP
    Updated to include fxtools.h.
    Revision 1.42  2003/07/02 21:56:54Z  jaredw
    minor Change to allow test to exit if VBF mount fails
    Revision 1.41  2004/07/02 21:23:38Z  jaredw
    no lunger calls oemunmount if VBF mount fails, this patches
    the exception that was occuring on subsequent calls to oemmount
    with NULL as a parameter after it had unmounted
    Revision 1.4  2004/05/21 02:44:32Z  garyp
    Added TRACEMTSTRESS logging.  Added typecasts to the isspace() calls
    so we compile cleanly.
    Revision 1.3  2004/04/28 01:42:54Z  garyp
    Updated to supply unique thread names at thread creation time.  Added
    documentation for the /M, /T, and /B parameters.
    Revision 1.2  2004/03/15 22:50:31Z  garys
    Updated to TEMPORARILY disable the build for DOS targets.
    Revision 1.1.1.2  2004/02/06 20:46:26Z  garyp
    Renamed from mtstress to mtstrss to avoid naming conflicts.  Fixed to
    initialize the drive number field.  Minor type fixes so it builds under CE.
    Revision 1.1.1.4  2004/02/05 22:44:12Z  billr
    Command line switches for all options (number of threads per
    drive, buffer size, mount interval).
    Revision 1.1.1.3  2004/02/05 21:04:34Z  billr
    Fix processing of /n switch.
    Revision 1.1.1.2  2004/02/05 20:44:23Z  billr
    Add support for multiple drives. Compiles, runs on a single drive under
    OSE, minimal testing.
    Revision 1.1.1.1  2004/01/16 21:24:48Z  billr
    Duplicate revision
    Revision 1.1  2004/01/16 21:24:48Z  billr
    Initial revision
    ---------------------
    Bill Roman 2003-12-09
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxtools.h>
#include <dlerrlev.h>

#if FFXCONF_VBFSUPPORT && DCL_OSFEATURE_THREADS

#include <fxfmlapi.h>
#include <vbf.h>
#include <dlprintf.h>
#include <dlperflog.h>
#include <dllog.h>


/*  This is a test -- all asserts are "production" asserts...
*/
#undef DclAssert
#undef DclError
#define DclAssert DclProductionAssert
#define DclError DclProductionError


/*-------------------------------------------------------------------
                Compile-Time Configuration parameters
-------------------------------------------------------------------*/

#define TRACEMTSTRESS              FALSE /* FALSE for checkin */
#define DEFAULT_TEST_MINUTES         (6) /* Default number of minutes to run the test */
#define DEFAULT_THREAD_COUNT         (4) /* Default number of test threads to start */
#define DEFAULT_MAX_THREADS         (50) /* Maximum number of test threads per drive */
#define DEFAULT_SAMPLE_INTERVAL     (20) /* Default sample interval, in seconds */
#define DEFAULT_BUFFER_BLOCKS       (37) /* Default size of transfer buffer, in allocation blocks */
#define DEFAULT_LOG_BUFFER_LEN_KB   (32) /* Default buffer size if /LOG option is used */
#define MAX_BLOCKS_PER_OP          (128) /* Max blocks to use for any given operation */
#define MAX_FILESPEC_LEN           (256)
#define SPARSE_REGION_SHIFT          (2) /* Reserve 1/4 of the flash for sparse writes/discards (0 to disable) */
#define SPARSE_WRITEDISCARD_COUNT (2000) /* 1/Nth iterations allowed, set to 0 to disable the sparse feature */


/*-------------------------------------------------------------------
                              Local types
-------------------------------------------------------------------*/
enum test_type
{
    TEST_READ = 0,
    TEST_WRITE,
    TEST_DISCARD,
    TEST_COMPACT,
    TEST_LIMIT
};

/*  The following states are used at three levels:  The test level,
    the drive thread level, and the test thread level.  The levels are
    hierarchical, with each level aware of its parent and child ojects.
*/
typedef enum
{
    EXEC_INITIALIZING = 0,
    EXEC_WAITING,   /* Initialized and waiting for the parent object to change to "running" */
    EXEC_RUNNING,   /* Running and will continue until parent object state changes */
    EXEC_QUITTING,  /* Quitting and waiting until all child objects have changed to "Exiting" */
    EXEC_QUITTING2, /* Quitting extra state for Disk summary information */
    EXEC_EXITING    /* Thread is exiting (or has exited) */
} EXECSTATE;


/*  Test parameters (other than drive) that may be set from the command
    line.
*/
typedef struct
{
    unsigned    nBufBlks;           /* in allocator blocks */
    unsigned    nThreads;           /* Number of test threads. */
    D_UINT32    ulSampleInterval;   /* Seconds between samples */
    unsigned    nProbabilities[TEST_LIMIT];
    unsigned    nTotalProbability;
    D_UINT16    uDrive;
} TESTPARAMS;


typedef struct
{
    unsigned    seq:31;
    unsigned    owned:1;
} BLOCKMAP;


typedef struct
{
    D_UINT32    ulSequenceNumber;
    D_UINT32    ulStartBlock;
    D_UINT32    ulBlockCount;
} VERIFY;


/*  All of the information maintained per drive.  This is the structure
    passed to the controller thread for each drive.
*/
typedef struct
{
    volatile EXECSTATE  DriveState;         /* Execution state for this drive */
    FFXFMLHANDLE        hFML;
    VBFHANDLE           hVBF;               /* can run on multiple partitions at once. */
    struct TESTARGS    *pTestArgs;          /* pointer to array, allocated at run time. */
    struct TESTINFO    *pTestInfo;          /* pointer to test-wide info */
    D_UINT32            ulClientBlocks;     /* Partition size in client blocks. */
    D_UINT32            ulStartSparse;      /* Client block start of the sparese area */
    PDCLMUTEX           pMutexOwnership;    /* Mutex for serializing access to the block ownership map */
    PDCLMUTEX           pMutexMount;
    TESTPARAMS          TestParams;
    BLOCKMAP           *pBlockMap;
    D_UINT16            uDriveNum;
    D_UINT16            uVBFBlockSize;
} DRIVEARGS;

/*  Information maintained per test thread.
*/
typedef struct TESTARGS
{
    volatile EXECSTATE  ThreadState;                /* Execution state for this thread */
    unsigned            nThreadNum;
    DCLTHREADHANDLE     hThread;
    D_UINT32            ulRandSeed;
    D_UINT32            ulOps[TEST_LIMIT];          /* Number of operations */
    D_UINT32            ulItems[TEST_LIMIT];        /* Number of items (pages) processed */
    D_UINT32            ulTotalMS[TEST_LIMIT];      /* Total MS to complete */
    D_UINT32            ulMaxMS[TEST_LIMIT];        /* Max MS for one operation to complete */
    D_UINT32            ulCumOps[TEST_LIMIT];       /* Cumulative number of operations */
    D_UINT32            ulCumItems[TEST_LIMIT];     /* Cumulative number of items (pages) processed */
    D_UINT32            ulCumTotalMS[TEST_LIMIT];   /* Cumulative total MS to complete */
    D_UINT32            ulCumMaxMS[TEST_LIMIT];     /* Cumulative max MS for one operation to complete */
    unsigned            nWriteCounter;              /* Write counter */
    unsigned            nDiscardCounter;            /* Discard counter */
    DRIVEARGS          *pDrive;
    D_BUFFER           *pPattern;
} TESTARGS;

/*  Test-wide state information
*/
typedef struct TESTINFO
{
    D_UINT32            ulTestMinutes;  /* How many minutes to run the test for */
    TESTPARAMS          TestParams;     /* default test parameters */
    DRIVEARGS          *apDrive[FFX_MAX_DISKS];
    volatile EXECSTATE  TestState;      /* execution state for the test as a whole */
    PDCLMUTEX           pMutexSync;     /* Mutex for synchronizing threads before remounting */
    PDCLMUTEX           pMutexSequence; /* Mutex for protecting the test wide sequence number */
    PDCLMUTEX           pMutexOutput;   /* Mutex for keeping output from getting scrambled */
    DCLTIMESTAMP        ts;
  #if DCLCONF_OUTPUT_ENABLED
    char                szLogFile[MAX_FILESPEC_LEN];
    DCLLOGHANDLE        hLog;
    unsigned            nLogBufferKB;
  #endif
    char                szPerfLogSuffix[PERFLOG_MAX_SUFFIX_LEN];
    unsigned            fNoRemount : 1; /* Disable remounting the disk */
    unsigned            fPerfLog   : 1; /* Enable PerfLog output */
    unsigned            fDebug     : 1; /* Invoke debugger on asserts */
} TESTINFO;


/*-------------------------------------------------------------------
                    Local function prototypes
-------------------------------------------------------------------*/
static int      AtomicPrintf(    TESTINFO *pTestInfo, const char *pszFmt, ...);
static D_INT16  SetSwitches(     TESTINFO *pTestInfo, FFXTOOLPARAMS *pTP);
static void     ShowUsage(       FFXTOOLPARAMS *pTP);
static void    *DriveThread(     void *pa);
static void    *TestThread(      void *pa);
static D_BOOL   drivesetup(      DRIVEARGS *pDrive);
static D_BOOL   drivestart(      DRIVEARGS *pDrive);
static void     driverun(        DRIVEARGS *pDrive);
static void     drivesummary(    DRIVEARGS *pDrive);
static void     driveshutdown(   DRIVEARGS *pDrive);
static void     DiscardMedia(    DRIVEARGS *pDrive);
static void     TestMount(       DRIVEARGS *pDrive);
static D_INT32  ClaimOwnership(  DRIVEARGS *pDrive, D_UINT32 ulStart, D_INT32 lCount);
static void     ReleaseOwnership(DRIVEARGS *pDrive, D_UINT32 ulStart, D_INT32 lCount, D_INT32 lWhich);
static void     MountSync(       TESTARGS *pArgs);
static void     TestRead(        TESTARGS *pArgs);
static void     TestWrite(       TESTARGS *pArgs);
static void     TestDiscard(     TESTARGS *pArgs);
static void     TestCompact(     TESTARGS *pArgs);
static void     VerifyBlocks(    TESTARGS *pArgs, D_UINT32 ulStartBlock, D_INT32 lBlocks);
static void     ReadBlocks(      TESTARGS *pArgs, D_UINT32 ulStartBlock, D_INT32 lBlocks);
static void     WriteBlocks(     TESTARGS *pArgs, D_UINT32 ulStartBlock, D_INT32 lBlocks);
static void     DiscardBlocks(   TESTARGS *pArgs, D_UINT32 ulStartBlock, D_INT32 lBlocks);
static void     RandomRange(     TESTARGS *pArgs, D_UINT32 *pulStartBlock, D_INT32 *plCount);
static void     FillBuffer(      TESTARGS *pArgs, D_BUFFER *pBuffer);
static D_BOOL   CheckBuffer(     TESTARGS *pArgs, D_BUFFER *pBuffer);
static unsigned TestType(        TESTARGS *pArgs);
static BLOCKMAP*InitBlockMap(    D_UINT32 ulBlocks);
static D_UINT32 GetBlockSequence(BLOCKMAP *pBlockMap, D_UINT32 ulBlock);
static void     SetBlockSequence(BLOCKMAP *pBlockMap, D_UINT32 ulBlock, D_UINT32 ulSeq);
static D_UINT32 GetSequenceNumber(TESTINFO *pTestInfo);
static D_BOOL   ParseTestProbabilities(TESTPARAMS *pParams, const char *list);

#endif  /* FFXCONF_VBFSUPPORT && DCL_OSFEATURE_THREADS */


/*-------------------------------------------------------------------
    Protected: FfxStressMT()

    Begins a read/verify, write, discard, and compaction stress
    test.  The test involves multiple threads (configured at
    compile time) operating concurrently on pseudo-randomly
    chosen areas of client address space.

    Parameters:
        pTP - parameters for this invocation of the test

    Return Value:
        Returns zero if successful, else non-zero on error.
-------------------------------------------------------------------*/
D_INT16 FfxStressMT(
    FFXTOOLPARAMS  *pTP)
{
  #if FFXCONF_VBFSUPPORT && DCL_OSFEATURE_THREADS
    DCLTHREADATTR  *pAttr;
    unsigned        ii;
    TESTINFO       *pTestInfo = NULL;
    DCLASSERTMODE   nOldAssertMode = DCLASSERTMODE_INVALID;
    D_INT16         iRet;

    DclAssert(pTP);

    DclPrintf("\nFlashFX Multithreaded VBF Stress Test\n");
    FfxSignOn(FALSE);

    DclPrintf("\nThis test is designed to exercise VBF in a time-sliced multithreaded\n");
    DclPrintf("environment.  While the test may complete normally when used in a non-\n");
    DclPrintf("time-sliced execution environment, it will not stress the desired areas\n");
    DclPrintf("of VBF which are critical to proper operation.\n\n");

    pTestInfo = DclMemAllocZero(sizeof *pTestInfo);
    if(!pTestInfo)
    {
        DclPrintf("Unable to allocate memory!\n");
        iRet = __LINE__;
        goto StressCleanup;
    }

    /*  Parse command line arguments.  This allocates testarg structures
        for each drive specified on the command line.
    */
    if(SetSwitches(pTestInfo, pTP))
    {
        iRet = 1;
        goto StressCleanup;
    }

    if(pTestInfo->fDebug)
        nOldAssertMode = DclAssertMode(pTP->dtp.hDclInst, DCLASSERTMODE_DEBUGGERFAIL);

  #if DCLCONF_OUTPUT_ENABLED
    if(pTestInfo->szLogFile[0])
    {
        D_UINT32 ulFlags = pTestInfo->fPerfLog ? DCLLOGFLAGS_PRIVATE : 0;
        
        pTestInfo->hLog = DclLogOpen(pTP->dtp.hDclInst, pTestInfo->szLogFile,
                                     pTestInfo->nLogBufferKB, ulFlags);
        if(!pTestInfo->hLog)
        {
            DclPrintf("Unable to open logfile!\n");
            iRet = __LINE__;
            goto StressCleanup;
        }
    }
  #endif

    pAttr = DclOsThreadAttrCreate();

    pTestInfo->pMutexOutput = DclMutexCreate("FMSOut");
    pTestInfo->pMutexSync = DclMutexCreate("FMSSync");
    pTestInfo->pMutexSequence = DclMutexCreate("FMSSeq");

    DclAssert(pTestInfo->pMutexOutput);
    DclAssert(pTestInfo->pMutexSync);
    DclAssert(pTestInfo->pMutexSequence);

    for(ii = 0; ii < FFX_MAX_DISKS; ii++)
    {
        if(pTestInfo->apDrive[ii])
        {
            char    achName[DCL_THREADNAMELEN];

            DclSNPrintf(achName, sizeof(achName), "FTSD%u", ii);

            pTestInfo->apDrive[ii]->uDriveNum = (D_UINT16)ii;

            /*  Command-line processing succeeded for this drive.
            */
            if(DclOsThreadCreate(NULL, achName, pAttr, DriveThread, pTestInfo->apDrive[ii]) == FFXSTAT_SUCCESS)
            {
                while(pTestInfo->apDrive[ii]->DriveState == EXEC_INITIALIZING)
                    DclOsSleep(1);
            }
            else
            {
                DclPrintf("Thread %u creation failed!\n", ii);
            }

            if(pTestInfo->apDrive[ii]->DriveState != EXEC_WAITING)
            {
                /*  If a thread did not start as expected, mark the test
                    state as "quitting" so any threads which did start will
                    know to quit.
                */
                pTestInfo->TestState = EXEC_QUITTING;
                break;
            }

            /*---------------------------------------------------------
                Write data to the performance log, if enabled.  Note
                that any changes to the test name or category must
                be accompanied by changes to perfmtstress.bat.  Any
                changes to the actual data fields recorded here
                requires changes to the various spreadsheets which
                track this data.
            ---------------------------------------------------------*/
            {
                DCLPERFLOGHANDLE    hPerfLog;
                hPerfLog = DCLPERFLOG_OPEN(pTestInfo->fPerfLog, pTestInfo->hLog, "MTSTRESS", "Configuration", NULL, pTestInfo->szPerfLogSuffix);
                DCLPERFLOG_NUM  (hPerfLog, "Minutes",        pTestInfo->ulTestMinutes);
                DCLPERFLOG_NUM  (hPerfLog, "Threads",        pTestInfo->apDrive[ii]->TestParams.nThreads);
                DCLPERFLOG_NUM  (hPerfLog, "PageSize",       pTestInfo->apDrive[ii]->uVBFBlockSize);
                DCLPERFLOG_NUM  (hPerfLog, "BufferPages",    pTestInfo->apDrive[ii]->TestParams.nBufBlks);
                DCLPERFLOG_NUM  (hPerfLog, "SampleInterval", pTestInfo->apDrive[ii]->TestParams.ulSampleInterval);
                DCLPERFLOG_WRITE(hPerfLog);
                DCLPERFLOG_CLOSE(hPerfLog);
            }
        }
    }

    if(pTestInfo->TestState == EXEC_INITIALIZING)
    {
        AtomicPrintf(pTestInfo, "      [---------Reads--------][---------Writes----------][--------Discards--------][---Compactions--]\n");
        AtomicPrintf(pTestInfo, "Thread Ops   Pages   Avg/MaxMS    Ops   Pages   Avg/MaxMS    Ops  Pages   Avg/MaxMS   Ops   Avg/MaxMS\n");

        pTestInfo->ts = DclTimeStamp();

        /*  Changing the test state to "running" turns everything loose...
        */
        pTestInfo->TestState = EXEC_RUNNING;

        /*  Periodically scan the drive threads to see if anything is still
            running.
        */
        while(pTestInfo->TestState == EXEC_RUNNING &&
            (DclTimePassed(pTestInfo->ts) < (pTestInfo->ulTestMinutes * 60) * 1000))
        {
            DclOsSleep(5000);
            DclMutexAcquire(pTestInfo->pMutexSync);
            for(ii = 0; ii < FFX_MAX_DISKS; ii++)
            {
                if(pTestInfo->apDrive[ii] && pTestInfo->apDrive[ii]->DriveState > EXEC_RUNNING)
                {
                    pTestInfo->TestState = EXEC_QUITTING;
                    break;
                }
            }
            DclOsSleep(5000);
            DclMutexRelease(pTestInfo->pMutexSync);
        }

        pTestInfo->TestState = EXEC_QUITTING;
    }

    /*  Loop until all the drive threads have exited.
    */
    for(ii = 0; ii < FFX_MAX_DISKS; ii++)
    {
        if(pTestInfo->apDrive[ii])
        {
            while(pTestInfo->apDrive[ii]->DriveState != EXEC_EXITING)
                DclOsSleep(1);
        }
    }

    pTestInfo->TestState = EXEC_EXITING;
        
    DclPrintf("Test exiting...\n");
    DclOsSleep(1000);

    DclMutexDestroy(pTestInfo->pMutexSequence);
    DclMutexDestroy(pTestInfo->pMutexSync);
    DclMutexDestroy(pTestInfo->pMutexOutput);

    iRet = 0;
    
  StressCleanup:

  #if DCLCONF_OUTPUT_ENABLED
    if(pTestInfo && pTestInfo->hLog)
        DclLogClose(pTP->dtp.hDclInst, pTestInfo->hLog);
  #endif

    if(nOldAssertMode != DCLASSERTMODE_INVALID)
        DclAssertMode(pTP->dtp.hDclInst, nOldAssertMode);

    if(pTestInfo)
        DclMemFree(pTestInfo);

    return iRet;

  #else

    (void)pTP;

    DclPrintf("FlashFX is configured with Allocator support disabled\n");

    return DCLERRORLEVEL_FEATUREDISABLED;

  #endif
}


#if FFXCONF_VBFSUPPORT && DCL_OSFEATURE_THREADS

/*---------------------------------------------------------------------------
                            Per-drive functions
---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------
    Local: DriveThread()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void    *DriveThread(
    void       *pa)
{
    DRIVEARGS  *pDrive = pa;
    D_BOOL      fWeCreatedVBF = FALSE;

    DclAssert(pDrive != NULL);
    DclAssert(pDrive->hFML);

  #if TRACEMTSTRESS
    AtomicPrintf(pDrive->pTestInfo, "DISK%u Master Thread initializing...\n", pDrive->uDriveNum);
  #endif

    pDrive->hVBF = FfxVbfHandle(pDrive->TestParams.uDrive);
    if(!pDrive->hVBF)
    {
        pDrive->hVBF = FfxVbfCreate(pDrive->hFML);
        if(pDrive->hVBF)
        {
            fWeCreatedVBF = TRUE;

          #if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
            /*  The VBF instance is created with idle-time compaction
                suspended.  It must be explicitly enabled.
            */
            FfxVbfCompactIdleResume(pDrive->hVBF);
          #endif
        }
    }

    if(pDrive->hVBF)
    {
        VBFDISKINFO di;

        if(FfxVbfDiskInfo(pDrive->hVBF, &di) == FFXSTAT_SUCCESS)
        {
            pDrive->hFML = di.hFML;
            pDrive->uVBFBlockSize = di.uPageSize;
            pDrive->ulClientBlocks = di.ulTotalPages;
            pDrive->ulStartSparse = di.ulTotalPages - (di.ulTotalPages >> SPARSE_REGION_SHIFT);

            DiscardMedia(pDrive);
            if(drivesetup(pDrive))
            {
                if(drivestart(pDrive))
                {
                    driverun(pDrive);
                    drivesummary(pDrive);
                    driveshutdown(pDrive);
                }
            }
        }
        else
        {
            AtomicPrintf(pDrive->pTestInfo, "MTSTRESS: Failed getting VBF disk information\n");
            DclError();
        }

        if(fWeCreatedVBF)
        {
            FfxVbfDestroy(pDrive->hVBF, FFX_SHUTDOWNFLAGS_NORMAL);
            pDrive->hVBF = NULL;
        }
    }
    else
    {
        AtomicPrintf(pDrive->pTestInfo, "MTSTRESS: VBF instance creation failed\n");
    }

  #if TRACEMTSTRESS
    AtomicPrintf(pDrive->pTestInfo, "DISK%u Master Thread exiting...\n", pDrive->uDriveNum);
  #endif

    pDrive->DriveState = EXEC_EXITING;

    return NULL;
}


/*-------------------------------------------------------------------
    Local: drivesetup()

    Allocate per-drive resources: block map, ownership semaphore,
    list of test threads (but not the threads themselves)

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL drivesetup(
    DRIVEARGS  *pDrive)
{
    DclAssert(pDrive != NULL);

    pDrive->pBlockMap = InitBlockMap(pDrive->ulClientBlocks);
    if(pDrive->pBlockMap)
    {
        char    mutexname[DCL_MUTEXNAMELEN + 1];

        DclSNPrintf(mutexname, sizeof(mutexname), "FMSOwn%U", pDrive->TestParams.uDrive);
        pDrive->pMutexOwnership = DclMutexCreate(mutexname);
        if(pDrive->pMutexOwnership)
        {
            DclSNPrintf(mutexname, sizeof(mutexname), "FMSnt%U", pDrive->TestParams.uDrive);
            pDrive->pMutexMount = DclMutexCreate(mutexname);
            if(pDrive->pMutexMount)
            {
                pDrive->pTestArgs = DclMemAllocZero(pDrive->TestParams.nThreads * sizeof *pDrive->pTestArgs);
                if(pDrive->pTestArgs)
                    return TRUE;

                DclMutexDestroy(pDrive->pMutexMount);
            }
            DclMutexDestroy(pDrive->pMutexOwnership);
        }
        DclMemFree(pDrive->pBlockMap);
    }
    return FALSE;
}


/*-------------------------------------------------------------------
    Local: drivestart()

    Returns number of threads started successfully.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL drivestart(
    DRIVEARGS      *pDrive)
{
    unsigned        ii;
    DCLTHREADATTR  *pAttr = DclOsThreadAttrCreate();

    for(ii = 0; ii < pDrive->TestParams.nThreads; ++ii)
    {
        char    achName[DCL_THREADNAMELEN];

        DclSNPrintf(achName, sizeof(achName), "FTSD%UT%u", pDrive->uDriveNum, ii);

        pDrive->pTestArgs[ii].nThreadNum = ii;
        pDrive->pTestArgs[ii].pDrive = pDrive;
        pDrive->pTestArgs[ii].ulRandSeed = pDrive->TestParams.uDrive * pDrive->TestParams.nThreads + ii + 1;

        if(DclOsThreadCreate(&pDrive->pTestArgs[ii].hThread, achName, pAttr, TestThread, &pDrive->pTestArgs[ii]) == FFXSTAT_SUCCESS)
        {
            while(pDrive->pTestArgs[ii].ThreadState == EXEC_INITIALIZING)
                DclOsSleep(1);
        }

        /*  If the thread did not initialize properly, loop through the
            threads which did initialize and wait for them to exit.
        */
        if(pDrive->pTestArgs[ii].ThreadState != EXEC_WAITING)
        {
            unsigned    jj;

            /*  Setting the drive state to "quitting" will cause any of the
                test threads to exit.
            */
            pDrive->DriveState = EXEC_QUITTING;

            for(jj = 0; jj < ii; jj++)
            {
                while(pDrive->pTestArgs[jj].ThreadState != EXEC_EXITING)
                    DclOsSleep(1);
            }

            return FALSE;
        }
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: driverun()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void driverun(
    DRIVEARGS  *pDrive)
{
    unsigned    ii;
    unsigned    nSample = 0;

  #if TRACEMTSTRESS
    AtomicPrintf(pDrive->pTestInfo, "DISK%u Master Thread waiting...\n", pDrive->uDriveNum);
  #endif

    pDrive->DriveState = EXEC_WAITING;

    while(pDrive->pTestInfo->TestState == EXEC_INITIALIZING)
        DclOsSleep(1);

  #if TRACEMTSTRESS
    AtomicPrintf(pDrive->pTestInfo, "DISK%u Master Thread running...\n", pDrive->uDriveNum);
  #endif

    pDrive->DriveState = EXEC_RUNNING;

    do
    {
        unsigned    ii, jj;
        D_UINT32    ulElapsedSecs;

        DclOsSleep(pDrive->TestParams.ulSampleInterval * 1000);
        DclMutexAcquire(pDrive->pTestInfo->pMutexSync);

        /*  Display progress statistics.
        */
        ulElapsedSecs = (DclTimePassed(pDrive->pTestInfo->ts) + 500) / 1000;

        DclPrintf("DISK%U:  Sample: %u  Elapsed Time: %u:%02u:%02u\n",
            pDrive->TestParams.uDrive,
            ++nSample,
            ulElapsedSecs / (60*60),
            (ulElapsedSecs % (60*60)) / 60,
            (ulElapsedSecs % (60*60)) % 60);

        for(ii = 0; ii < pDrive->TestParams.nThreads; ++ii)
        {
            char    szReadAvg[16];
            char    szWriteAvg[16];
            char    szDiscardAvg[16];
            char    szCompactAvg[16];

            DclPrintf("%2d: %6lU %7lU %6s/%4lU %6lU %7lU %6s/%4lU %5lU %6lU %7s/%4lU %5lU %6s/%4lU\n", ii,
                pDrive->pTestArgs[ii].ulOps[TEST_READ],
                pDrive->pTestArgs[ii].ulItems[TEST_READ],
                DclRatio(szReadAvg, sizeof(szReadAvg), pDrive->pTestArgs[ii].ulTotalMS[TEST_READ], pDrive->pTestArgs[ii].ulOps[TEST_READ], 2),
                pDrive->pTestArgs[ii].ulMaxMS[TEST_READ],
                pDrive->pTestArgs[ii].ulOps[TEST_WRITE],
                pDrive->pTestArgs[ii].ulItems[TEST_WRITE],
                DclRatio(szWriteAvg, sizeof(szWriteAvg), pDrive->pTestArgs[ii].ulTotalMS[TEST_WRITE], pDrive->pTestArgs[ii].ulOps[TEST_WRITE], 2),
                pDrive->pTestArgs[ii].ulMaxMS[TEST_WRITE],
                pDrive->pTestArgs[ii].ulOps[TEST_DISCARD],
                pDrive->pTestArgs[ii].ulItems[TEST_DISCARD],
                DclRatio(szDiscardAvg, sizeof(szDiscardAvg), pDrive->pTestArgs[ii].ulTotalMS[TEST_DISCARD], pDrive->pTestArgs[ii].ulOps[TEST_DISCARD], 2),
                pDrive->pTestArgs[ii].ulMaxMS[TEST_DISCARD],
                pDrive->pTestArgs[ii].ulOps[TEST_COMPACT],
                DclRatio(szCompactAvg, sizeof(szCompactAvg), pDrive->pTestArgs[ii].ulTotalMS[TEST_COMPACT], pDrive->pTestArgs[ii].ulOps[TEST_COMPACT], 1),
                pDrive->pTestArgs[ii].ulMaxMS[TEST_COMPACT]);

            for(jj=TEST_READ; jj < TEST_LIMIT; jj++)
            {
                pDrive->pTestArgs[ii].ulCumOps[jj]      += pDrive->pTestArgs[ii].ulOps[jj];
                pDrive->pTestArgs[ii].ulCumItems[jj]    += pDrive->pTestArgs[ii].ulItems[jj];
                pDrive->pTestArgs[ii].ulCumTotalMS[jj]  += pDrive->pTestArgs[ii].ulTotalMS[jj];
                if(pDrive->pTestArgs[ii].ulCumMaxMS[jj] < pDrive->pTestArgs[ii].ulMaxMS[jj])
                    pDrive->pTestArgs[ii].ulCumMaxMS[jj] = pDrive->pTestArgs[ii].ulMaxMS[jj];

                pDrive->pTestArgs[ii].ulOps[jj]         = 0;
                pDrive->pTestArgs[ii].ulItems[jj]       = 0;
                pDrive->pTestArgs[ii].ulTotalMS[jj]     = 0;
                pDrive->pTestArgs[ii].ulMaxMS[jj]       = 0;
            }
        }

        DclMutexRelease(pDrive->pTestInfo->pMutexSync);

        TestMount(pDrive);

        if(pDrive->DriveState != EXEC_RUNNING)
            break;

        for(ii = 0; ii < pDrive->TestParams.nThreads; ii++)
        {
            if(pDrive->pTestArgs[ii].ThreadState > EXEC_RUNNING)
            {
                pDrive->DriveState = EXEC_QUITTING;
                break;
            }
        }
    }
    while(pDrive->pTestInfo->TestState == EXEC_RUNNING && pDrive->DriveState == EXEC_RUNNING);

  #if TRACEMTSTRESS
    AtomicPrintf(pDrive->pTestInfo, "DISK%u Master Thread quitting...\n", pDrive->uDriveNum);
  #endif

    pDrive->DriveState = EXEC_QUITTING;

    /*  Loop until all the threads for this Disk are done
    */
    for(ii = 0; ii < pDrive->TestParams.nThreads; ii++)
    {
        while(pDrive->pTestArgs[ii].ThreadState != EXEC_EXITING)
            DclOsSleep(1);
    }

    return;
}


/*-------------------------------------------------------------------
    Local: drivesummary()

    Display summary information for a drive .

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void drivesummary(
    DRIVEARGS  *pDrive)
{
    unsigned    ii;
    D_UINT32    ulSecs;
    D_UINT32    ulReadKB;
    D_UINT32    ulReadPages = 0;
    D_UINT32    ulReadTimeMS = 0;
    D_UINT32    ulWriteKB;
    D_UINT32    ulWritePages = 0;
    D_UINT32    ulWriteTimeMS = 0;
    D_UINT32    ulDiscardKB;
    D_UINT32    ulDiscardPages = 0;
    D_UINT32    ulDiscardTimeMS = 0;
    D_UINT32    ulCompactions = 0;
    D_UINT32    ulCompactionTimeMS = 0;
    D_UINT32    ulOperationSecs = 0;
    D_UINT32    ulElapsedSecs;

    ulElapsedSecs = (DclTimePassed(pDrive->pTestInfo->ts) + 500) / 1000;

    /*  Loop until all the Disks are at least to the "Quitting" state
    */
    for(ii = 0; ii < FFX_MAX_DISKS; ii++)
    {
        if(pDrive->pTestInfo->apDrive[ii])
        {
            while(pDrive->pTestInfo->apDrive[ii]->DriveState < EXEC_QUITTING)
                DclOsSleep(1);
        }    
    }

    DclMutexAcquire(pDrive->pTestInfo->pMutexOutput);
    {
        DclPrintf("DISK%U Cumulative Thread Summary:\n", pDrive->uDriveNum);
        DclPrintf("      [-------------Reads-------------][--------------Writes--------------][------------Discards--------------][------Compactions------]\n");
        DclPrintf("Thread Ops   Pages    Total/  Avg/MaxMS    Ops   Pages    Total/  Avg/MaxMS    Ops   Pages    Total/  Avg/MaxMS  Ops   Total/  Avg/MaxMS\n");

        for(ii = 0; ii < pDrive->TestParams.nThreads; ii++)
        {
            char    szReadAvg[16];
            char    szWriteAvg[16];
            char    szDiscardAvg[16];
            char    szCompactAvg[16];

            DclPrintf("%2u: %6lU %7lU %8lU/%6s/%4lU %6lU %7lU %8lU/%6s/%4lU %6lU %7lU %8lU/%6s/%4lU %4lU %7lU/%6s/%4lU\n",
                ii,
                pDrive->pTestArgs[ii].ulCumOps[TEST_READ],
                pDrive->pTestArgs[ii].ulCumItems[TEST_READ],
                pDrive->pTestArgs[ii].ulCumTotalMS[TEST_READ],
                DclRatio(szReadAvg, sizeof(szReadAvg), pDrive->pTestArgs[ii].ulCumTotalMS[TEST_READ], pDrive->pTestArgs[ii].ulCumOps[TEST_READ], 2),
                pDrive->pTestArgs[ii].ulCumMaxMS[TEST_READ],
                pDrive->pTestArgs[ii].ulCumOps[TEST_WRITE],
                pDrive->pTestArgs[ii].ulCumItems[TEST_WRITE],
                pDrive->pTestArgs[ii].ulCumTotalMS[TEST_WRITE],
                DclRatio(szWriteAvg, sizeof(szWriteAvg), pDrive->pTestArgs[ii].ulCumTotalMS[TEST_WRITE], pDrive->pTestArgs[ii].ulCumOps[TEST_WRITE], 2),
                pDrive->pTestArgs[ii].ulCumMaxMS[TEST_WRITE],
                pDrive->pTestArgs[ii].ulCumOps[TEST_DISCARD],
                pDrive->pTestArgs[ii].ulCumItems[TEST_DISCARD],
                pDrive->pTestArgs[ii].ulCumTotalMS[TEST_DISCARD],
                DclRatio(szDiscardAvg, sizeof(szDiscardAvg), pDrive->pTestArgs[ii].ulCumTotalMS[TEST_DISCARD], pDrive->pTestArgs[ii].ulCumOps[TEST_DISCARD], 2),
                pDrive->pTestArgs[ii].ulCumMaxMS[TEST_DISCARD],
                pDrive->pTestArgs[ii].ulCumOps[TEST_COMPACT],
                pDrive->pTestArgs[ii].ulCumTotalMS[TEST_COMPACT],
                DclRatio(szCompactAvg, sizeof(szCompactAvg), pDrive->pTestArgs[ii].ulCumTotalMS[TEST_COMPACT], pDrive->pTestArgs[ii].ulCumOps[TEST_COMPACT], 1),
                pDrive->pTestArgs[ii].ulCumMaxMS[TEST_COMPACT]);

            ulReadPages         += pDrive->pTestArgs[ii].ulCumItems[TEST_READ];
            ulReadTimeMS        += pDrive->pTestArgs[ii].ulCumTotalMS[TEST_READ];
            ulWritePages        += pDrive->pTestArgs[ii].ulCumItems[TEST_WRITE];
            ulWriteTimeMS       += pDrive->pTestArgs[ii].ulCumTotalMS[TEST_WRITE];
            ulDiscardPages      += pDrive->pTestArgs[ii].ulCumItems[TEST_DISCARD];
            ulDiscardTimeMS     += pDrive->pTestArgs[ii].ulCumTotalMS[TEST_DISCARD];
            ulCompactions       += pDrive->pTestArgs[ii].ulCumItems[TEST_COMPACT];
            ulCompactionTimeMS  += pDrive->pTestArgs[ii].ulCumTotalMS[TEST_COMPACT];
        }
    }
    DclMutexRelease(pDrive->pTestInfo->pMutexOutput);

    pDrive->DriveState = EXEC_QUITTING2;

    /*  Loop until all the Disks are at least to the "Quitting2" state
    */
    for(ii = 0; ii < FFX_MAX_DISKS; ii++)
    {
        if(pDrive->pTestInfo->apDrive[ii])
        {
            while(pDrive->pTestInfo->apDrive[ii]->DriveState < EXEC_QUITTING2)
                DclOsSleep(1);
        }    
    }

    DclMutexAcquire(pDrive->pTestInfo->pMutexOutput);
    {
        DclPrintf("DISK%U Operations Throughput:\n", pDrive->uDriveNum);

        ulReadKB = (ulReadPages * pDrive->uVBFBlockSize) / 1024;
        ulSecs = (ulReadTimeMS+500)/1000;
        if(!ulSecs)
            ulSecs++;
        ulOperationSecs += ulSecs;
        DclPrintf("  Read         %9lU KB in %9lU seconds -- %6lU KB per second\n",
            ulReadKB, ulSecs, ulReadKB/ulSecs);

        ulWriteKB = (ulWritePages * pDrive->uVBFBlockSize) / 1024;
        ulSecs = (ulWriteTimeMS+500)/1000;
        if(!ulSecs)
            ulSecs++;
        ulOperationSecs += ulSecs;
        DclPrintf("  Wrote        %9lU KB in %9lU seconds -- %6lU KB per second\n",
            ulWriteKB, ulSecs, ulWriteKB/ulSecs);

        ulDiscardKB = (ulDiscardPages * pDrive->uVBFBlockSize) / 1024;
        ulSecs = (ulDiscardTimeMS+500)/1000;
        if(!ulSecs)
            ulSecs++;
        ulOperationSecs += ulSecs;
        DclPrintf("  Discarded    %9lU KB in %9lU seconds -- %6lU KB per second\n",
            ulDiscardKB, ulSecs, ulDiscardKB/ulSecs);

        ulSecs = (ulCompactionTimeMS+500)/1000;
        if(!ulSecs)
            ulSecs++;
        ulOperationSecs += ulSecs;
        DclPrintf("  Compacted    %9lU units in %6lU seconds\n", ulCompactions, ulSecs);

        DclPrintf("  Total operation run time      %8lU seconds (all threads for DISK%U)\n", ulOperationSecs, pDrive->uDriveNum);
        DclPrintf("  Per thread operation run time %8lU seconds\n", ulOperationSecs / pDrive->TestParams.nThreads);
        DclPrintf("  Actual test run time          %8lU seconds\n", ulElapsedSecs);
    }
    DclMutexRelease(pDrive->pTestInfo->pMutexOutput);
    
    for(ii = 0; ii < pDrive->TestParams.nThreads; ii++)
    {
        /*---------------------------------------------------------
            Write data to the performance log, if enabled.  Note
            that any changes to the test name or category must
            be accompanied by changes to perfmtstress.bat.  Any
            changes to the actual data fields recorded here
            requires changes to the various spreadsheets which
            track this data.
        ---------------------------------------------------------*/
        {
            DCLPERFLOGHANDLE    hPerfLog;
            char                szThread[] = "ThreadN";

            szThread[6] = ii + '0';

            hPerfLog = DCLPERFLOG_OPEN(pDrive->pTestInfo->fPerfLog, pDrive->pTestInfo->hLog, "MTSTRESS", szThread, NULL, pDrive->pTestInfo->szPerfLogSuffix);
            DCLPERFLOG_NUM  (hPerfLog, "ReadOps",   pDrive->pTestArgs[ii].ulCumOps[    TEST_READ]);
            DCLPERFLOG_NUM  (hPerfLog, "ReadItems", pDrive->pTestArgs[ii].ulCumItems[  TEST_READ]);
            DCLPERFLOG_NUM  (hPerfLog, "ReadTotMS", pDrive->pTestArgs[ii].ulCumTotalMS[TEST_READ]);
            DCLPERFLOG_NUM  (hPerfLog, "ReadMaxMS", pDrive->pTestArgs[ii].ulCumMaxMS[  TEST_READ]);
            DCLPERFLOG_NUM  (hPerfLog, "WrtOps",    pDrive->pTestArgs[ii].ulCumOps[    TEST_WRITE]);
            DCLPERFLOG_NUM  (hPerfLog, "WrtItems",  pDrive->pTestArgs[ii].ulCumItems[  TEST_WRITE]);
            DCLPERFLOG_NUM  (hPerfLog, "WrtTotMS",  pDrive->pTestArgs[ii].ulCumTotalMS[TEST_WRITE]);
            DCLPERFLOG_NUM  (hPerfLog, "WrtMaxMS",  pDrive->pTestArgs[ii].ulCumMaxMS[  TEST_WRITE]);
            DCLPERFLOG_NUM  (hPerfLog, "DiscOps",   pDrive->pTestArgs[ii].ulCumOps[    TEST_DISCARD]);
            DCLPERFLOG_NUM  (hPerfLog, "DiscItems", pDrive->pTestArgs[ii].ulCumItems[  TEST_DISCARD]);
            DCLPERFLOG_NUM  (hPerfLog, "DiscTotMS", pDrive->pTestArgs[ii].ulCumTotalMS[TEST_DISCARD]);
            DCLPERFLOG_NUM  (hPerfLog, "DiscMaxMS", pDrive->pTestArgs[ii].ulCumMaxMS[  TEST_DISCARD]);
            DCLPERFLOG_NUM  (hPerfLog, "CompOps",   pDrive->pTestArgs[ii].ulCumOps[    TEST_COMPACT]);
            DCLPERFLOG_NUM  (hPerfLog, "CompTotMS", pDrive->pTestArgs[ii].ulCumTotalMS[TEST_COMPACT]);
            DCLPERFLOG_NUM  (hPerfLog, "CompMaxMS", pDrive->pTestArgs[ii].ulCumMaxMS[  TEST_COMPACT]);
            DCLPERFLOG_WRITE(hPerfLog);
            DCLPERFLOG_CLOSE(hPerfLog);
        }
    }

    /*---------------------------------------------------------
        Write data to the performance log, if enabled.  Note
        that any changes to the test name or category must
        be accompanied by changes to perfmtstress.bat.  Any
        changes to the actual data fields recorded here
        requires changes to the various spreadsheets which
        track this data.
    ---------------------------------------------------------*/
    {
        DCLPERFLOGHANDLE    hPerfLog;
        hPerfLog = DCLPERFLOG_OPEN(pDrive->pTestInfo->fPerfLog, pDrive->pTestInfo->hLog, "MTSTRESS", "Throughput", NULL, pDrive->pTestInfo->szPerfLogSuffix);
        DCLPERFLOG_NUM  (hPerfLog, "ReadKB",       ulReadKB);
        DCLPERFLOG_NUM  (hPerfLog, "ReadSecs",    (ulReadTimeMS+500)/1000);
        DCLPERFLOG_NUM  (hPerfLog, "WriteKB",      ulWriteKB);
        DCLPERFLOG_NUM  (hPerfLog, "WriteSecs",   (ulWriteTimeMS+500)/1000);
        DCLPERFLOG_NUM  (hPerfLog, "DiscardKB",    ulDiscardKB);
        DCLPERFLOG_NUM  (hPerfLog, "DiscardSecs", (ulDiscardTimeMS+500)/1000);
        DCLPERFLOG_NUM  (hPerfLog, "CompactUnits", ulCompactions);
        DCLPERFLOG_NUM  (hPerfLog, "CompactSecs", (ulCompactionTimeMS+500)/1000);
        DCLPERFLOG_NUM  (hPerfLog, "TestSecs",     ulElapsedSecs);
        DCLPERFLOG_WRITE(hPerfLog);
        DCLPERFLOG_CLOSE(hPerfLog);
    }

    return;
}


/*-------------------------------------------------------------------
    Local: driveshutdown()

    Free all the resources allocated by a successful drivesetup().

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void driveshutdown(
    DRIVEARGS  *pDrive)
{
    DclAssert(pDrive->pTestArgs);
    DclMemFree(pDrive->pTestArgs);
    pDrive->pTestArgs = NULL;

    DclAssert(pDrive->pMutexMount);
    DclMutexDestroy(pDrive->pMutexMount);
    pDrive->pMutexMount = NULL;

    DclAssert(pDrive->pMutexOwnership);
    DclMutexDestroy(pDrive->pMutexOwnership);
    pDrive->pMutexOwnership = NULL;

    DclAssert(pDrive->pBlockMap);
    DclMemFree(pDrive->pBlockMap);
    pDrive->pBlockMap = NULL;
}


/*-------------------------------------------------------------------
    Local: DiscardMedia()

    Discard the entire partition to create a known initial state
    of all blocks as the test starts.

    Parameters:
        pDrive - the drive information structure.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void DiscardMedia(
    DRIVEARGS      *pDrive)
{
    VBFDISKINFO     di;
    FFXIOSTATUS     ioStat;

    FfxVbfDiskInfo(pDrive->hVBF, &di);

    ioStat = FfxVbfDiscardPages(pDrive->hVBF, 0, di.ulTotalPages);
    if(!IOSUCCESS(ioStat, di.ulTotalPages))
    {
        AtomicPrintf(pDrive->pTestInfo, "FAILED: discard media\n");
        DclProductionError();
    }

    return;
}


/*-------------------------------------------------------------------
    Local: TestMount()

    Wait a period of time, then synchronize with the test threads
    to ensure that they have all exited from VBF functions, then
    remount the partition.  If the mount is not successful, clear
    the "test fTestRunning" flag to signal that the test threads
    should exit.

    Parameters:
        hVBF - specifies the VBF partition

    Return Value:
        None.
-------------------------------------------------------------------*/
static void TestMount(
    DRIVEARGS  *pDrive)
{
    unsigned    ii;
    FFXSTATUS   ffxStat;

    DclMutexAcquire(pDrive->pMutexMount);

    /*  Wait for all threads to be idle.
    */
    for(ii = 0; ii < pDrive->TestParams.nThreads; ++ii)
    {
        while(pDrive->pTestArgs[ii].ThreadState == EXEC_RUNNING)
            DclOsSleep(1);
    }

    /*  Remount the VBF instance unless this funtionality is disabled.
    */
    if(!pDrive->pTestInfo->fNoRemount)
    {
        /*  NOTE: The nMode parameter must be set to FFX_SHUTDOWNFLAGS_NORMAL
                  to ensure that any cached discard data is written to the
                  media (if the VBFCONF_DEFERREDDISCARD feature is enabled).

                  Otherwise verification of discarded data will fail.
        */
        ffxStat = FfxVbfDestroy(pDrive->hVBF, FFX_SHUTDOWNFLAGS_NORMAL);
        if(ffxStat != FFXSTAT_SUCCESS)
        {
            AtomicPrintf(pDrive->pTestInfo, "Error destroying VBF instance, Status=%lX\n", ffxStat);
            pDrive->DriveState = EXEC_QUITTING;
            DclError();
        }
        else
        {
            pDrive->hVBF = FfxVbfCreate(pDrive->hFML);

            if(pDrive->hVBF)
            {
              #if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
                /*  The VBF instance is created with idle-time compaction
                    suspended.  It must be explicitly enabled.
                */
                FfxVbfCompactIdleResume(pDrive->hVBF);
              #endif
            }
            else
            {
                AtomicPrintf(pDrive->pTestInfo, "FAILED: Remount\n");
                pDrive->DriveState = EXEC_QUITTING;
                DclError();
            }
        }
    }

    DclMutexRelease(pDrive->pMutexMount);

    return;
}



/*---------------------------------------------------------------------------
                          Per-test thread functions
---------------------------------------------------------------------------*/



/*-------------------------------------------------------------------
    Local: MountSync()

    Synchronize with the main thread so that it can safely
    remount the disk.  Change the thread state to "waiting",
    then acquires the mount mutex before setting the thread
    state back to "running" again.

    Parameters:
        pArgs - argument structure for this thread.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void MountSync(
    TESTARGS *pArgs)
{
    pArgs->ThreadState = EXEC_WAITING;

    DclMutexAcquire(pArgs->pDrive->pMutexMount);

    pArgs->ThreadState = EXEC_RUNNING;

    DclMutexRelease(pArgs->pDrive->pMutexMount);

    return;
}


/*-------------------------------------------------------------------
    Local: TestThread()

    Entry point for the test threads.  Randomly select and
    perform a test (read/verify, write, discard, or compaction),
    then synchronize with the main thread in case it is time to
    remount the disk.  Exit if the "test fTestRunning" flag is
    cleared.

    Parameters:
        pa - argument structure for this thread.

    Return Value:
        Always returns NULL.
-------------------------------------------------------------------*/
static void *TestThread(
    void           *pa)
{
    TESTARGS       *pArgs = pa;

  #if TRACEMTSTRESS
    AtomicPrintf(pArgs->pDrive->pTestInfo, "DISK%U Thread %u initializing...\n", pArgs->pDrive->uDriveNum, pArgs->nThreadNum);
  #endif

    /*  Allocate buffer.
    */
    pArgs->pPattern = DclMemAlloc(pArgs->pDrive->TestParams.nBufBlks * pArgs->pDrive->uVBFBlockSize);
    if(pArgs->pPattern)
    {
      #if TRACEMTSTRESS
        AtomicPrintf(pArgs->pDrive->pTestInfo, "DISK%U Thread %u waiting...\n", pArgs->pDrive->uDriveNum, pArgs->nThreadNum);
      #endif

        /*  Mark this thread as "ready-to-run"...
        */
        pArgs->ThreadState = EXEC_WAITING;

        /*  ...and then wait until the drive thread is done initializing
            everything.
        */
        while(pArgs->pDrive->DriveState < EXEC_RUNNING)
            DclOsSleep(1);

      #if TRACEMTSTRESS
        AtomicPrintf(pArgs->pDrive->pTestInfo, "DISK%U Thread %u running...\n", pArgs->pDrive->uDriveNum, pArgs->nThreadNum);
      #endif

        /*  Off to the races...
        */
        pArgs->ThreadState = EXEC_RUNNING;

        while(pArgs->pDrive->DriveState == EXEC_RUNNING)
        {
            /*  Select and run a test.
            */
            switch (TestType(pArgs))
            {
                case TEST_READ:
                    TestRead(pArgs);
                    break;
                case TEST_WRITE:
                    TestWrite(pArgs);
                    break;
                case TEST_DISCARD:
                    TestDiscard(pArgs);
                    break;
                case TEST_COMPACT:
                    TestCompact(pArgs);
                    break;
                default:
                    DclError();
                    break;
            }

            /*  If necessary, pause for a VBF remount
            */
            MountSync(pArgs);
        }

        DclMemFree(pArgs->pPattern);
    }
    else
    {
        AtomicPrintf(pArgs->pDrive->pTestInfo, "FAILED: could not allocate buffer\n");
        DclError();
    }

  #if TRACEMTSTRESS
    AtomicPrintf(pArgs->pDrive->pTestInfo, "DISK%U Thread %u exiting...\n", pArgs->pDrive->uDriveNum, pArgs->nThreadNum);
  #endif

    pArgs->ThreadState = EXEC_EXITING;

    return NULL;
}


/*-------------------------------------------------------------------
    Local: TestRead()

    Read and possibly verify a range of client addresses.  Select
    a random range, read all of it, and verify as much as can
    be claimed.

    Parameters:
        pArgs - argument structure for this thread.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void TestRead(
    TESTARGS   *pArgs)
{
    D_UINT32    ulStartBlock;
    D_INT32     lBlocks;
    D_INT32     lVerifyBlocks;

  #if TRACEMTSTRESS
    AtomicPrintf(pArgs->pDrive->pTestInfo, "MTSTRESS: DISK%U Read begin\n", pArgs->pDrive->uDriveNum);
  #endif

    /*  Select a range of flash to read.
    */
    RandomRange(pArgs, &ulStartBlock, &lBlocks);

    /*  Claim ownership of as much of it as possible, which may be none.
    */
    lVerifyBlocks = ClaimOwnership(pArgs->pDrive, ulStartBlock, lBlocks);
    {
        /*  The following code verifies those blocks for which we were able
            to claim ownership, and does a plain read on the remaining blocks.
        */

        if(lVerifyBlocks >= 0)
        {
            /*  Verify at beginning.
            */
            DclProductionAssert(lVerifyBlocks <= lBlocks);
            VerifyBlocks(pArgs, ulStartBlock, lVerifyBlocks);
            ReadBlocks(pArgs, ulStartBlock + lVerifyBlocks, lBlocks - lVerifyBlocks);
        }
        else
        {
            /*  Verify at end.
            */
            DclProductionAssert(-lVerifyBlocks < lBlocks);
            ReadBlocks(pArgs, ulStartBlock, lBlocks + lVerifyBlocks);
            VerifyBlocks(pArgs, ulStartBlock + lBlocks + lVerifyBlocks, -lVerifyBlocks);
        }
    }
    ReleaseOwnership(pArgs->pDrive, ulStartBlock, lBlocks, lVerifyBlocks);

  #if TRACEMTSTRESS
    AtomicPrintf(pArgs->pDrive->pTestInfo, "MTSTRESS: DISK%U Read complete\n", pArgs->pDrive->uDriveNum);
  #endif

}


/*-------------------------------------------------------------------
    Local: TestWrite()

    Write a range of client addresses.  If none of the randomly
    chosen range can be claimed, no write occurs.

    Parameters:
        pArgs - argument structure for this thread.

    Return Value:
        None
-------------------------------------------------------------------*/
static void TestWrite(
    TESTARGS   *pArgs)
{
    D_UINT32    ulStartBlock;
    D_INT32     lBlocks;
    D_INT32     lWriteBlocks;

  #if TRACEMTSTRESS
    AtomicPrintf(pArgs->pDrive->pTestInfo, "MTSTRESS: DISK%U Write begin\n", pArgs->pDrive->uDriveNum);
  #endif

    pArgs->nWriteCounter++;

    /*  Select a range of flash to try to write.
    */
    while(TRUE)
    {
        RandomRange(pArgs, &ulStartBlock, &lBlocks);
        if(pArgs->nWriteCounter > SPARSE_WRITEDISCARD_COUNT)
        {
            /*  Reset the counter only when we've actually done
                a write into the sparse range.
            */    
            if(ulStartBlock > pArgs->pDrive->ulStartSparse)
            {
                pArgs->nWriteCounter = 0;
                /* DclPrintf("w\n"); */
            }

            break;
        }
        else if(ulStartBlock < pArgs->pDrive->ulStartSparse)
        {
            /* DclPrintf("W"); */
            break;
        }
    };
        
    /*  Claim ownership of as much of it as possible, which may be none.
    */
    lWriteBlocks = ClaimOwnership(pArgs->pDrive, ulStartBlock, lBlocks);
    {
        /*  Write only what is owned, which could be none.
        */
        if(lWriteBlocks >= 0)
        {
            DclProductionAssert(lWriteBlocks <= lBlocks);
            WriteBlocks(pArgs, ulStartBlock, lWriteBlocks);
        }
        else
        {
            DclProductionAssert(-lWriteBlocks < lBlocks);
            WriteBlocks(pArgs, ulStartBlock + lBlocks + lWriteBlocks, -lWriteBlocks);
        }
    }
    ReleaseOwnership(pArgs->pDrive, ulStartBlock, lBlocks, lWriteBlocks);

  #if TRACEMTSTRESS
    AtomicPrintf(pArgs->pDrive->pTestInfo, "MTSTRESS: DISK%U Write complete\n", pArgs->pDrive->uDriveNum);
  #endif

    return;
}


/*-------------------------------------------------------------------
    Local: TestDiscard()

    Discard a range of client addresses.  If none of the randomly
    chosen range can be claimed, no discard occurs.

    Parameters:
        pArgs - argument structure for this thread.

    Return Value:
        None
-------------------------------------------------------------------*/
static void TestDiscard(
    TESTARGS   *pArgs)
{
    D_UINT32    ulStartBlock;
    D_INT32     lBlocks;
    D_INT32     lDiscardBlocks;

  #if TRACEMTSTRESS
    AtomicPrintf(pArgs->pDrive->pTestInfo, "MTSTRESS: DISK%U Discard begin\n", pArgs->pDrive->uDriveNum);
  #endif

    pArgs->nDiscardCounter++;

    /*  Select a range of flash to try to discard.
    */
    while(TRUE)
    {
        RandomRange(pArgs, &ulStartBlock, &lBlocks);
        if(pArgs->nDiscardCounter > SPARSE_WRITEDISCARD_COUNT)
        {
            /*  Reset the counter only when we've actually done
                a discard in the sparse range.
            */    
            if(ulStartBlock > pArgs->pDrive->ulStartSparse)
            {
                pArgs->nDiscardCounter = 0;
                /* DclPrintf("d\n"); */
            }

            break;
        }
        else if(ulStartBlock < pArgs->pDrive->ulStartSparse)
        {
            /* DclPrintf("D"); */
            break;
        }
    };
        
    /*  Claim ownership of as much of it as possible, which may be none.
    */
    lDiscardBlocks = ClaimOwnership(pArgs->pDrive, ulStartBlock, lBlocks);
    {
        /*  Discard only what is owned, which could be none.
        */
        if(lDiscardBlocks >= 0)
        {
            DclProductionAssert(lDiscardBlocks <= lBlocks);
            DiscardBlocks(pArgs, ulStartBlock, lDiscardBlocks);
        }
        else
        {
            DclProductionAssert(-lDiscardBlocks < lBlocks);
            DiscardBlocks(pArgs, ulStartBlock + lBlocks + lDiscardBlocks, -lDiscardBlocks);
        }
    }
    ReleaseOwnership(pArgs->pDrive, ulStartBlock, lBlocks, lDiscardBlocks);

  #if TRACEMTSTRESS
    AtomicPrintf(pArgs->pDrive->pTestInfo, "MTSTRESS: DISK%U Discard complete\n", pArgs->pDrive->uDriveNum);
  #endif

    return;
}


/*-------------------------------------------------------------------
    Local: TestCompact()

    Perform a single FfxVbfCompact() call.

    Parameters:
        pArgs - argument structure for this thread.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void TestCompact(
    TESTARGS       *pArgs)
{
    DCLTIMESTAMP    ts;
    D_UINT32        ulElapsedMS;
    FFXIOSTATUS     ioStat;

  #if TRACEMTSTRESS
    AtomicPrintf(pArgs->pDrive->pTestInfo, "MTSTRESS: DISK%U Compaction begin\n", pArgs->pDrive->uDriveNum);
  #endif

    ts = DclTimeStamp();

    ioStat = FfxVbfCompact(pArgs->pDrive->hVBF, FFX_COMPACTFLAGS_EVERYTHING);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        ulElapsedMS = DclTimePassed(ts);

        pArgs->ulOps[TEST_COMPACT]++;
        pArgs->ulItems[TEST_COMPACT]++;
        pArgs->ulTotalMS[TEST_COMPACT] += ulElapsedMS;
        if(pArgs->ulMaxMS[TEST_COMPACT] < ulElapsedMS)
            pArgs->ulMaxMS[TEST_COMPACT] = ulElapsedMS;
    }

  #if TRACEMTSTRESS
    AtomicPrintf(pArgs->pDrive->pTestInfo, "MTSTRESS: DISK%U Compaction complete, Status=%s\n",
        pArgs->pDrive->uDriveNum, FfxDecodeIOStatus(&ioStat));
  #endif
}


/*-------------------------------------------------------------------
    Local: VerifyBlocks()

    Read and verify a range of client addresses.  The range must
    be owned by the calling thread.

    Parameters:
        pArgs        - argument structure for this thread.
        ulStartBlock - starting VBF block number
        lBlocks      - number of VBF blocks to verify.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void VerifyBlocks(
    TESTARGS   *pArgs,
    D_UINT32    ulStartBlock,
    D_INT32     lBlocks)
{
    VBFHANDLE   hVBF = pArgs->pDrive->hVBF;
    D_BUFFER   *pBuffer = pArgs->pPattern;

  #if TRACEMTSTRESS
    AtomicPrintf(pArgs->pDrive->pTestInfo,
        "MTSTRESS: DISK%U Start=%7lU, Verifying %lD Blocks\n",
        pArgs->pDrive->uDriveNum, ulStartBlock, lBlocks);
  #endif

    while(lBlocks)
    {
        FFXIOSTATUS     ioStat;
        D_UINT32        ulReadBlocks = DCLMIN((D_UINT32)lBlocks, pArgs->pDrive->TestParams.nBufBlks);
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedMS;

        ts = DclTimeStamp();

        ioStat = FfxVbfReadPages(hVBF, ulStartBlock, ulReadBlocks, pBuffer);
        if(IOSUCCESS(ioStat, ulReadBlocks))
        {
            unsigned    ii;
            VERIFY     *pVerify = (VERIFY*)pBuffer;

            ulElapsedMS = DclTimePassed(ts);

            pArgs->ulOps[TEST_READ]++;
            pArgs->ulItems[TEST_READ] += ulReadBlocks;
            pArgs->ulTotalMS[TEST_READ] += ulElapsedMS;
            if(pArgs->ulMaxMS[TEST_READ] < ulElapsedMS)
                pArgs->ulMaxMS[TEST_READ] = ulElapsedMS;

            for(ii = 0; ii < ulReadBlocks; ++ii)
            {
                D_UINT32   sequence;

                sequence = GetBlockSequence(pArgs->pDrive->pBlockMap, ulStartBlock + ii);
                if(sequence != 0)
                {
                    if(pVerify->ulSequenceNumber != sequence)
                    {
                        AtomicPrintf(pArgs->pDrive->pTestInfo, "FAILED: Sequence check at %lX %X %d\n", ulStartBlock, ulReadBlocks, ii);
                        AtomicPrintf(pArgs->pDrive->pTestInfo, "expected %lx, found %lX %lX %lX\n",
                            sequence,
                            pVerify->ulSequenceNumber,
                            pVerify->ulStartBlock,
                            pVerify->ulBlockCount);
                        DclError();
                    }

                    if(!CheckBuffer(pArgs, (D_BUFFER*)pVerify))
                    {
                        AtomicPrintf(pArgs->pDrive->pTestInfo, "FAILED: Fill check %lX %x %d\n", ulStartBlock, ulReadBlocks, ii);
                        DclError();
                    }
                }
                else
                {
                    /*  Block is discarded.
                    */
                    D_BUFFER   *p = (D_BUFFER*)pVerify;

                    if((*p != VBF_FILL_VALUE) ||
                        (DclMemCmp(p, p + 1, pArgs->pDrive->uVBFBlockSize - 1) != 0))
                    {
                        AtomicPrintf(pArgs->pDrive->pTestInfo, "FAILED: Discard check %lX %lU %u\n", ulStartBlock, ulReadBlocks, ii);
                        DclError();
                    }
                }

                pVerify = (VERIFY*)DclPtrAddByte(pVerify, pArgs->pDrive->uVBFBlockSize);
            }
        }
        else
        {
            AtomicPrintf(pArgs->pDrive->pTestInfo, "FAILED: read at %lX %X\n", ulStartBlock, ulReadBlocks);
            DclError();
        }

        ulStartBlock += ulReadBlocks;
        lBlocks -= ulReadBlocks;
    }

    return;
}


/*-------------------------------------------------------------------
    Local: ReadBlocks()

    Read from a range of client addresses.  The range need not
    be owned by the calling thread.

    Parameters:
        pArgs        - argument structure for this thread.
        ulStartBlock - starting VBF block number
        lBlocks      - number of VBF blocks to read.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void ReadBlocks(
    TESTARGS   *pArgs,
    D_UINT32    ulStartBlock,
    D_INT32     lBlocks)
{
    VBFHANDLE   hVBF = pArgs->pDrive->hVBF;
    D_BUFFER   *pBuffer = pArgs->pPattern;

  #if TRACEMTSTRESS
    AtomicPrintf(pArgs->pDrive->pTestInfo,
        "MTSTRESS: DISK%U Start=%7lU, Reading %lD Blocks\n",
        pArgs->pDrive->uDriveNum, ulStartBlock, lBlocks);
  #endif

    DclAssert(pArgs);
    DclAssert(lBlocks >= 0);

    while(lBlocks)
    {
        FFXIOSTATUS     ioStat;
        D_UINT32        ulReadBlocks = DCLMIN((D_UINT32)lBlocks, pArgs->pDrive->TestParams.nBufBlks);
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedMS;

        ts = DclTimeStamp();

        ioStat = FfxVbfReadPages(hVBF, ulStartBlock, ulReadBlocks, pBuffer);
        if(!IOSUCCESS(ioStat, ulReadBlocks))
        {
            AtomicPrintf(pArgs->pDrive->pTestInfo, "FAILED: read at %lX %lX\n", ulStartBlock, ulReadBlocks);
            DclProductionError();
        }

        ulElapsedMS = DclTimePassed(ts);

        pArgs->ulOps[TEST_READ]++;
        pArgs->ulItems[TEST_READ] += ulReadBlocks;
        pArgs->ulTotalMS[TEST_READ] += ulElapsedMS;
        if(pArgs->ulMaxMS[TEST_READ] < ulElapsedMS)
            pArgs->ulMaxMS[TEST_READ] = ulElapsedMS;

        ulStartBlock += ulReadBlocks;
        lBlocks -= ulReadBlocks;
    }

    return;
}


/*-------------------------------------------------------------------
    Local: WriteBlocks()

    Write known data to a range of client addresses and record
    information about the write in the sequence number map.  The
    range must be owned by the calling thread.

    A unique sequence number is assigned to the whole range.
    This sequence number is written in each block in the range.
    The write is performed by one or more FfxVbfWritePages()
    calls for an amount that fits in the transfer buffer.  The
    starting client address and number of blocks in each of
    these individual FfxVbfWritePages() calls is also written
    in each block.

    Parameters:
        pArgs        - argument structure for this thread.
        ulStartBlock - starting VBF block number
        lBlocks      - number of VBF blocks to write.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void WriteBlocks(
    TESTARGS   *pArgs,
    D_UINT32    ulStartBlock,
    D_INT32     lBlocks)
{
    VBFHANDLE   hVBF = pArgs->pDrive->hVBF;
    D_BUFFER   *pBuffer = pArgs->pPattern;
    D_UINT32    ulSequenceNumber = GetSequenceNumber(pArgs->pDrive->pTestInfo);

  #if TRACEMTSTRESS
    AtomicPrintf(pArgs->pDrive->pTestInfo,
        "MTSTRESS: DISK%U Start=%7lU, Writing %lD Blocks\n",
        pArgs->pDrive->uDriveNum, ulStartBlock, lBlocks);
  #endif

    DclAssert(lBlocks >= 0);

    while(lBlocks)
    {
        FFXIOSTATUS     ioStat;
        D_UINT32        ulWriteBlocks = DCLMIN((D_UINT32)lBlocks, pArgs->pDrive->TestParams.nBufBlks);
        unsigned        ii;
        VERIFY         *pVerify = (VERIFY*)pBuffer;
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedMS;

        for(ii = 0; ii < ulWriteBlocks; ++ii)
        {
            DclMemSet(pVerify, 0, sizeof *pVerify);
            pVerify->ulSequenceNumber = ulSequenceNumber;
            pVerify->ulStartBlock = ulStartBlock;
            pVerify->ulBlockCount = ulWriteBlocks;
            FillBuffer(pArgs, (D_BUFFER*)pVerify);
            SetBlockSequence(pArgs->pDrive->pBlockMap, ulStartBlock + ii, ulSequenceNumber);
            pVerify = (VERIFY*)DclPtrAddByte(pVerify, pArgs->pDrive->uVBFBlockSize);
        }

        ts = DclTimeStamp();

        ioStat = FfxVbfWritePages(hVBF, ulStartBlock, ulWriteBlocks, pBuffer);
        if(!IOSUCCESS(ioStat, ulWriteBlocks))
        {
            AtomicPrintf(pArgs->pDrive->pTestInfo, "FAILED: write at %lX, len=%X, %s\n",
                ulStartBlock, ulWriteBlocks, FfxDecodeIOStatus(&ioStat));

            DclProductionError();
        }

        ulElapsedMS = DclTimePassed(ts);

        pArgs->ulOps[TEST_WRITE]++;
        pArgs->ulItems[TEST_WRITE] += ulWriteBlocks;
        pArgs->ulTotalMS[TEST_WRITE] += ulElapsedMS;
        if(pArgs->ulMaxMS[TEST_WRITE] < ulElapsedMS)
            pArgs->ulMaxMS[TEST_WRITE] = ulElapsedMS;

        ulStartBlock += ulWriteBlocks;
        lBlocks -= ulWriteBlocks;
    }

    return;
}


/*-------------------------------------------------------------------
    Local: DiscardBlocks()

    Discard a range of client addresses and record this in the
    block sequence number map.  The range must be owned by the
    calling thread.

    Parameters:
        pArgs        - argument structure for this thread.
        ulStartBlock - starting VBF block number.
        lBlocks      - number of VBF blocks to discard.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void DiscardBlocks(
    TESTARGS       *pArgs,
    D_UINT32        ulStartBlock,
    D_INT32         lBlocks)
{
    DclAssert(lBlocks >= 0);

  #if TRACEMTSTRESS
    AtomicPrintf(pArgs->pDrive->pTestInfo,
        "MTSTRESS: DISK%U Start=%7lU, Discarding %lD Blocks\n",
        pArgs->pDrive->uDriveNum, ulStartBlock, lBlocks);
  #endif

    if(lBlocks)
    {
        FFXIOSTATUS     ioStat;
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedMS;

        ts = DclTimeStamp();

        ioStat = FfxVbfDiscardPages(pArgs->pDrive->hVBF, ulStartBlock, lBlocks);
        if(!IOSUCCESS(ioStat, (D_UINT32)lBlocks))
        {
            AtomicPrintf(pArgs->pDrive->pTestInfo, "FAILED: discard at %lX, len=%lX\n", ulStartBlock, lBlocks);
            DclProductionError();
        }

        ulElapsedMS = DclTimePassed(ts);

        pArgs->ulOps[TEST_DISCARD]++;
        pArgs->ulItems[TEST_DISCARD] += lBlocks;
        pArgs->ulTotalMS[TEST_DISCARD] += ulElapsedMS;
        if(pArgs->ulMaxMS[TEST_DISCARD] < ulElapsedMS)
            pArgs->ulMaxMS[TEST_DISCARD] = ulElapsedMS;

        while(lBlocks--)
        {
            SetBlockSequence(pArgs->pDrive->pBlockMap, ulStartBlock++, 0);
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Local: FillBuffer()

    Fills the padding area of a block buffer with copies of the
    verification data.

    Parameters:
        pPatBuff - the buffer to fill

    Return Value:
        None.
-------------------------------------------------------------------*/
static void FillBuffer(
    TESTARGS   *pArgs,
    D_BUFFER   *pBuffer)
{
    D_BUFFER   *pPadding = pBuffer + sizeof(VERIFY);
    size_t      nRemaining = pArgs->pDrive->uVBFBlockSize - sizeof(VERIFY);

    while(nRemaining)
    {
        size_t          length = DCLMIN(sizeof(VERIFY), nRemaining);

        DclMemCpy(pPadding, pBuffer, length);

        pPadding += length;
        nRemaining -= length;
    }
}


/*-------------------------------------------------------------------
    Local: CheckBuffer()

    Compares the padding area of a block buffer to the
    verification data.

    Parameters:
        pPatBuff - the buffer to verify

    Return Value:
        Returns TRUE if the padding area is correct, or FALSE
        otherwise.
-------------------------------------------------------------------*/
static D_BOOL CheckBuffer(
    TESTARGS   *pArgs,
    D_BUFFER   *pBuffer)
{
    D_BUFFER   *pPadding = pBuffer + sizeof(VERIFY);
    size_t      nRemaining = pArgs->pDrive->uVBFBlockSize - sizeof(VERIFY);

    while(nRemaining)
    {
        size_t          length = DCLMIN(sizeof(VERIFY), nRemaining);

        if(DclMemCmp(pPadding, pBuffer, length) != 0)
            return FALSE;

        pPadding += length;
        nRemaining -= length;
    }


    return TRUE;
}


/*-------------------------------------------------------------------
    Local: RandomRange()

    Pseudo-randomly select a range of allocator blocks within
    the VBF client address space.

    The length is greater than zero and not more than the lessor
    of MAX_BLOCKS_PER_OP and the total number of blocks remaining
    in the disk, relative to the returned start block.

    Parameters:
        pArgs         - The test arguments.
        pulStartBlock - Receives the selected starting block.
        plCount       - Receives the number of blocks.

    Return Value:
        None.  The starting block number is stored in the location
        pointed to by pulStartBlock, and the number of blocks is
        stored in the location pointed to by plCount.
-------------------------------------------------------------------*/
static void RandomRange(
    TESTARGS   *pArgs,
    D_UINT32   *pulStartBlock,
    D_INT32    *plCount)
{
    D_UINT32    ulCount;
    D_UINT32    ulReservedCount;

    DclAssert(pArgs);
    DclAssert(pulStartBlock);
    DclAssert(plCount);

    ulCount = (DclRand(&pArgs->ulRandSeed) % MAX_BLOCKS_PER_OP) + 1;

    if(ulCount > D_INT32_MAX)
        ulCount /= 2;

    DclAssert(ulCount > 0 && ulCount <= pArgs->pDrive->ulClientBlocks);

    /*  1/2 of the sparse area is never accessed at all
    */
  #if SPARSE_REGION_SHIFT   
    ulReservedCount = pArgs->pDrive->ulClientBlocks >> (SPARSE_REGION_SHIFT + 1);
  #else
    ulReservedCount = 0;
  #endif

    DclAssert(pArgs->pDrive->ulClientBlocks > ulCount + ulReservedCount);

    *pulStartBlock = DclRand(&pArgs->ulRandSeed) % 
        (pArgs->pDrive->ulClientBlocks - ulCount - ulReservedCount + 1);

    DclAssert(*pulStartBlock + ulCount <= pArgs->pDrive->ulClientBlocks);

    *plCount = (D_INT32)ulCount;

    return;
}


/*-------------------------------------------------------------------
    Local: ParseTestProbabilities()

    Parse the command line parameters representing the relative
    proportions of read/verify, write, discard, and compaction
    operations to perform.

    The parameters are up to four numbers separated by whitespace
    and/or commas.  A missing number (two commas with only
    whitespace between, or less than four numbers present)
    represents a value of zero.

    Parameters:
        list - string representing the proportions of different test
               operations.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL ParseTestProbabilities(
    TESTPARAMS *pParams,
    const char *list)
{
    unsigned    t;

    pParams->nTotalProbability = 0;
    for(t = 0; t < TEST_LIMIT; ++t)
    {
        const char *pEnd;
        D_UINT32    ulProbability;

        /*  Eat any leading white space
        */
        while(*list && isspace((unsigned char)*list))
            list++;

        pEnd = DclNtoUL(list, &ulProbability);

        if(!pEnd || ulProbability >= UINT_MAX
           || (unsigned)UINT_MAX - (unsigned)ulProbability < pParams->nTotalProbability)
        {
            return FALSE;
        }

        pParams->nProbabilities[t] = (unsigned)ulProbability;
        pParams->nTotalProbability += pParams->nProbabilities[t];
        list = pEnd;
        if(*list == ',')
            ++list;
        else if(*list && !isspace((unsigned char)*list))
            return FALSE;
    }

    return (pParams->nTotalProbability != 0);
}


/*-------------------------------------------------------------------
    Local: TestType()

    Selects a test type using a pseudo-random number generator
    and the proportions of different tests specified on the
    command line (or default).

    Parameters:
        seed - seed value for pseudo-random number generator

    Return Value:
        Returns TEST_READ, TEST_WRITE, TEST_DISCARD, or TEST_COMPACT.
-------------------------------------------------------------------*/
static unsigned TestType(
    TESTARGS   *pArgs)
{
    unsigned    nTestval;
    unsigned    nProbability = 0;
    unsigned    t;
    unsigned    nTotProb = pArgs->pDrive->TestParams.nTotalProbability;

    DclAssert(nTotProb > 0);
    DclAssert(nTotProb <= UINT_MAX);

    /*  Cast is safe because due to the assert on nTotProb above.
    */
    nTestval = (unsigned)(DclRand(&pArgs->ulRandSeed) % nTotProb);

    for(t = 0; t < TEST_LIMIT; t++)
    {
        nProbability += pArgs->pDrive->TestParams.nProbabilities[t];
        if(nTestval < nProbability)
            return t;
    }

    DclError();
    return TEST_READ;
}


/*-------------------------------------------------------------------
    Local: InitBlockMap()

    Allocates storage to hold sequence numbers for each block.
    Each sequence number is either a non-zero value written to
    the block, or zero to represent a discarded block.  The
    map is initialized to all zero.

    Parameters:
        ulBlocks - the size of the block map to initialize

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static BLOCKMAP *InitBlockMap(
    D_UINT32    ulBlocks)
{
    BLOCKMAP *BlockMap = DclMemAllocZero(ulBlocks * sizeof *BlockMap);

    return BlockMap;
}


/*-------------------------------------------------------------------
    Local: GetBlockSequence()

    Retrieves the record of the sequence number most recently
    written to the specified block.

    Parameters:
        ulBlock - block number

    Return Value:
        Returns a non-zero sequence number if the block is valid, or
        zero if the block is discarded.
-------------------------------------------------------------------*/
static D_UINT32 GetBlockSequence(
    BLOCKMAP   *pBlockMap,
    D_UINT32    ulBlock)
{
    return pBlockMap[ulBlock].seq;
}


/*-------------------------------------------------------------------
    Local: SetBlockSequence()

    Saves the sequence number in the block map.

    Parameters:
        ulBlock - block number
        ulSeq   - the sequence number

    Return Value:
        None.
-------------------------------------------------------------------*/
static void SetBlockSequence(
    BLOCKMAP   *pBlockMap,
    D_UINT32    ulBlock,
    D_UINT32    ulSeq)
{
    pBlockMap[ulBlock].seq = ulSeq;
}


/*-------------------------------------------------------------------
    Local: GetSequenceNumber()

    Increments the sequence number and returns the new value,
    skipping zero.

    Parameters:
        None.

    Return Value:
        Returns a non-zero sequence number.
-------------------------------------------------------------------*/
static D_UINT32 GetSequenceNumber(
    TESTINFO       *pTestInfo)
{
    D_UINT32        ulThisSequenceNumber;
    static BLOCKMAP BlockSequence;

    DclMutexAcquire(pTestInfo->pMutexSequence);

    /*  Never return zero.
    */
    do
    {
        ulThisSequenceNumber = ++BlockSequence.seq;
    }
    while(ulThisSequenceNumber == 0);

    DclMutexRelease(pTestInfo->pMutexSequence);

    return ulThisSequenceNumber;
}


/*-------------------------------------------------------------------
    Local: ClaimOwnership()

    Attempts to claim ownership of a range of client addresses.
    If the first block of the requested range is not already
    owned, it assigns ownership of blocks at the beginning of
    the range up to either the requested number or the first
    which is already owned.  If the first block is already owned
    but the last block of the range is not, it assigns ownership
    of blocks at the end of the range back to the last owned
    block in the range.  Otherwise no blocks are claimed.

    Parameters:
        ulStart - The starting block number
        lCount  - The number of blocks

    Return Value:
        A value greater than zero reporting the number of blocks
        now owned by the caller starting at the beginning of the
        requested range, or a negative value reporting the number
        of blocks owned at the end of the range, or zero if no
        blocks could be claimed.
-------------------------------------------------------------------*/
static D_INT32 ClaimOwnership(
    DRIVEARGS  *pDrive,
    D_UINT32    ulStart,
    D_INT32     lCount)
{
    D_UINT32    ulBlock = ulStart;

    DclAssert(pDrive);
    DclAssert(lCount > 0);

    DclMutexAcquire(pDrive->pMutexOwnership);
    {
        /*  Find contiguous unowned blocks at the beginning of the range.
        */
        while(!pDrive->pBlockMap[ulBlock].owned && --lCount >= 0)
        {
            pDrive->pBlockMap[ulBlock++].owned = TRUE;
        }

        if(ulBlock == ulStart)
        {
            /*  There were no unowned blocks at the beginning of the range.
                Find contiguous unowned blocks at the end.
            */
            ulStart += lCount - 1;
            ulBlock = ulStart;
            while(!pDrive->pBlockMap[ulBlock].owned && --lCount >= 0)
            {
                pDrive->pBlockMap[ulBlock--].owned = TRUE;
            }
        }
    }
    DclMutexRelease(pDrive->pMutexOwnership);

    /*  Cast is safe because magnitude of difference never exceeds lCount
    */
    return (D_INT32)(ulBlock - ulStart);
}


/*-------------------------------------------------------------------
    Local: ReleaseOwnership()

    Relinquishes ownership of an address range claimed with
    ClaimOwnership().

    Parameters:
        ulStart - The starting address originally passed to
                  ClaimOwnership().
        lCount  - The length originally passed to ClaimOwnership().
        lWhich  - The value originally returned by ClaimOwnership().

    Return Value:
        None.
-------------------------------------------------------------------*/
static void ReleaseOwnership(
    DRIVEARGS  *pDrive,
    D_UINT32    ulStart,
    D_INT32     lCount,
    D_INT32     lWhich)
{
    int         direction = 1;

    DclAssert(pDrive);
    DclAssert(lCount > 0);

    DclMutexAcquire(pDrive->pMutexOwnership);
    {
        if(lWhich < 0)
        {
            direction = -1;
            lWhich = -lWhich;
            ulStart += lCount - 1;
        }

        while(--lWhich >= 0)
        {
            pDrive->pBlockMap[ulStart].owned = FALSE;
            ulStart += direction;
        }
    }
    DclMutexRelease(pDrive->pMutexOwnership);

    return;
}


/*-------------------------------------------------------------------
    Local: SetSwitches()

    Parses the command line arguments and sets any globals needed
    for the tests.  Also shows help via ShowUsage() if needed.

    Parameters:
        pTP - A pointer to the FFXTOOLPARAMS structure to use

    Return Value:
        Returns zero if the command line arguments are valid and
        recorded.  Returns a non-zero line number if any problems
        are encountered (syntax errors, etc.).
-------------------------------------------------------------------*/
#define ARGBUFFLEN 128
static D_INT16 SetSwitches(
    TESTINFO       *pTestInfo,
    FFXTOOLPARAMS  *pTP)
{
    D_UINT16        i, j;
    D_CHAR          achArgBuff[ARGBUFFLEN];
    D_UINT16        argc;
    TESTPARAMS     *pCmdArgs = &pTestInfo->TestParams;

    /*  Initialize defaults at run time rather than compile time
        so they are the same every time this test is invoked.
    */
    pTestInfo->ulTestMinutes = DEFAULT_TEST_MINUTES;
    pTestInfo->TestParams.nBufBlks = DEFAULT_BUFFER_BLOCKS;
    pTestInfo->TestParams.nThreads = DEFAULT_THREAD_COUNT;
    pTestInfo->TestParams.ulSampleInterval = DEFAULT_SAMPLE_INTERVAL;
    ParseTestProbabilities(&pTestInfo->TestParams, "50, 30, 15, 5");

    argc = DclArgCount(pTP->dtp.pszCmdLine);

    if(!argc)
    {
        ShowUsage(pTP);
        return __LINE__;
    }

    /*  Parse each string
    */
    for(i = 1; i <= argc; i++)
    {
        if(!DclArgRetrieve(pTP->dtp.pszCmdLine, i, ARGBUFFLEN, achArgBuff))
        {
            DclPrintf("\nBad argument!\n");
            ShowUsage(pTP);
            return __LINE__;
        }

        if(DclStrNICmp(achArgBuff, "/RATIO:", 7) == 0)
        {
            /*  Parse the test probabilities, if any were supplied on the
                command line.
            */
            if(!ParseTestProbabilities(pCmdArgs, &achArgBuff[7]))
            {
                DclPrintf("Invalid test ratio\n");

                ShowUsage(pTP);
                return __LINE__;
            }

            continue;
        }

        if(DclStrNICmp(achArgBuff, "/Time:", 6) == 0)
        {
            const char *pEnd;

            pEnd = DclNtoUL(achArgBuff + 6, &pTestInfo->ulTestMinutes);
            if(!pEnd || (!(isspace((unsigned char)*pEnd) || *pEnd == '/' || *pEnd == '\0')))
            {
                DclPrintf("\n  Invalid /TIME parameter\n");
                ShowUsage(pTP);
                return __LINE__;
            }

            continue;
        }

        if(DclStrICmp(achArgBuff, "/Debug") == 0)
        {
            pTestInfo->fDebug = TRUE;

            continue;
        }

        if(DclStrICmp(achArgBuff, "/NoRemount") == 0)
        {
            pTestInfo->fNoRemount = TRUE;

            continue;
        }

        if(DclStrNICmp(achArgBuff, "/PerfLog", 8) == 0)
        {
            pTestInfo->fPerfLog = TRUE;

            if(achArgBuff[8] == ':')
            {
                DclStrNCpy(pTestInfo->szPerfLogSuffix, &achArgBuff[9], sizeof(pTestInfo->szPerfLogSuffix));
                pTestInfo->szPerfLogSuffix[sizeof(pTestInfo->szPerfLogSuffix)-1] = 0;
            }

            continue;
        }

      #if DCLCONF_OUTPUT_ENABLED
        if(DclStrNICmp(achArgBuff, "/LOG:", 5) == 0)
        {
            char       *pColon;
            const char *pTmp;

            pColon = DclStrChr(&achArgBuff[5], ':');
            if(pColon)
            {
                D_UINT32    ulLogBuffSize;

                /*  Zap the colon separating the name from the size with
                    a nul.
                */
                *pColon = 0;
                pColon++;

                pTmp = DclSizeToUL(pColon, &ulLogBuffSize);
                if(!pTmp || *pTmp)
                {
                    DclPrintf("Improperly formatted /LOG option \"%s\"\n", achArgBuff);
                    return __LINE__;
                }

                if(ulLogBuffSize / 1024UL > UINT_MAX)
                {
                    DclPrintf("The log buffer size is too large.\n\n");
                    return FALSE;
                }

                pTestInfo->nLogBufferKB = (unsigned)ulLogBuffSize / 1024;
            }
            else
            {
                pTestInfo->nLogBufferKB = DEFAULT_LOG_BUFFER_LEN_KB;
            }

            if((achArgBuff[5] == '"') || (achArgBuff[5] == '\''))
                DclStrNCpy(pTestInfo->szLogFile, &achArgBuff[6], DclStrLen(&achArgBuff[6]) - 1);
            else
                DclStrNCpy(pTestInfo->szLogFile, &achArgBuff[5], DclStrLen(&achArgBuff[5]));

            continue;
        }
      #endif

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
                switch(DclToLower(*(achArgBuff + j)))
                {
                    case '?':
                        ShowUsage(pTP);
                        return __LINE__;

                    case 'd':   /* disk number */
                    {
                        D_UINT32        ulDrive;
                        const char     *pEnd;
                        FFXFMLHANDLE    hFML;

                        if(!((DclToLower(*(achArgBuff + j + 1)) == 'i') &&
                            (DclToLower(*(achArgBuff + j + 2)) == 's') &&
                            (DclToLower(*(achArgBuff + j + 3)) == 'k')))
                        {
                            DclPrintf("Syntax error in \"%s\"\n", achArgBuff);
                            return __LINE__;
                        }

                        j += 4;

                        pEnd = DclNtoUL(achArgBuff + j, &ulDrive);
                        if(ulDrive >= FFX_MAX_DISKS
                           || !(isspace((unsigned char)*pEnd) || *pEnd == '/'
                                || *pEnd == '\0'))
                        {
                            DclPrintf("Invalid DISK number\n");
                            return __LINE__;
                        }
                        j = pEnd - achArgBuff;

                        hFML = FfxFmlHandle((unsigned)ulDrive);
                        if(!hFML)
                        {
                            DclPrintf("DISK%lU is not initialized\n", ulDrive);
                            return __LINE__;
                        }

                        if(pTestInfo->apDrive[ulDrive] == NULL)
                        {
                            pTestInfo->apDrive[ulDrive] = DclMemAllocZero(sizeof *pTestInfo->apDrive[ulDrive]);
                            if(pTestInfo->apDrive[ulDrive] == NULL)
                            {
                                DclPrintf("Can't allocate memory\n");
                                return __LINE__;
                            }
                            pTestInfo->apDrive[ulDrive]->hFML = hFML;
                            pTestInfo->apDrive[ulDrive]->pTestInfo = pTestInfo;
                        }

                        pCmdArgs = &pTestInfo->apDrive[ulDrive]->TestParams;

                        DclMemCpy(pCmdArgs, &pTestInfo->TestParams, sizeof *pCmdArgs);

                        pCmdArgs->uDrive = (D_UINT16)ulDrive;

                        break;
                    }

                    case 't':   /* number of threads */
                    {
                        D_UINT32    ulNThreads;
                        const char *pEnd;

                        j++;
                        if(achArgBuff[j] != ':')
                        {
                            DclPrintf("Syntax error in \"%s\"\n", achArgBuff);
                            return __LINE__;
                        }
                        j++;

                        pEnd = DclNtoUL(achArgBuff + j, &ulNThreads);
                        if(ulNThreads < 1
                           || !(isspace((unsigned char)*pEnd) || *pEnd == '/'
                                || *pEnd == '\0'))
                        {
                            DclPrintf("Invalid number of threads.\n");
                            return __LINE__;
                        }
                        j = pEnd - achArgBuff;

                        if(ulNThreads > DEFAULT_MAX_THREADS)
                        {
                            DclPrintf("Limiting number of threads to %d\n", DEFAULT_MAX_THREADS);
                            ulNThreads = DEFAULT_MAX_THREADS;
                        }
                        pCmdArgs->nThreads = (unsigned)ulNThreads;
                        break;
                    }

                    case 's':   /* sample interval in seconds  */
                    {
                        D_UINT32    ulSeconds;
                        const char *pEnd;

                        j++;
                        if(achArgBuff[j] != ':')
                        {
                            DclPrintf("Syntax error in \"%s\"\n", achArgBuff);
                            return __LINE__;
                        }
                        j++;

                        pEnd = DclNtoUL(achArgBuff + j, &ulSeconds);
                        if(ulSeconds < 2 || ulSeconds > 3600
                           || !(isspace((unsigned char)*pEnd) || *pEnd == '/'
                                || *pEnd == '\0'))
                        {
                            DclPrintf("Invalid mount interval.\n");
                            return __LINE__;
                        }
                        j = pEnd - achArgBuff;

                        pCmdArgs->ulSampleInterval = ulSeconds;
                        break;
                    }

                    case 'b':   /* buffer size in allocator blocks */
                    {
                        D_UINT32    ulSize;
                        const char *pEnd;

                        j++;
                        if(achArgBuff[j] != ':')
                        {
                            DclPrintf("Syntax error in \"%s\"\n", achArgBuff);
                            return __LINE__;
                        }
                        j++;

                        pEnd = DclNtoUL(achArgBuff + j, &ulSize);
                        if(ulSize == 0
                           || ulSize > UINT_MAX
                           || !(isspace((unsigned char)*pEnd) || *pEnd == '/'
                                || *pEnd == '\0'))
                        {
                            DclPrintf("Invalid buffer size.\n");
                            return __LINE__;
                        }
                        j = pEnd - achArgBuff;

                        pCmdArgs->nBufBlks = (unsigned)ulSize;
                        break;
                    }

                    default:
                        /*  Unrecognized switch
                        */
                        DclPrintf("Bad option: \"%s\"\n", (achArgBuff + j));
                        ShowUsage(pTP);
                        return __LINE__;
                }
            }
            else
            {
                DclPrintf("Bad option: \"%s\"\n", (achArgBuff + j));
                ShowUsage(pTP);
                return __LINE__;
            }
        }
    }

    /*  All worked fine
    */
    return 0;
}


/*-------------------------------------------------------------------
    Local: ShowUsage()

    Shows help to the user and returns.

    Parameters:
        pTP - A pointer to the FFXTOOLPARAMS structure to use

    Return Value:
        None.
-------------------------------------------------------------------*/
static void ShowUsage(
    FFXTOOLPARAMS  *pTP)
{
    DclPrintf("\nSyntax: %s /DISKn [options]\n\n", pTP->dtp.pszCmdName);
    DclPrintf("Where [options] are:\n");
    DclPrintf("  /?               This help screen\n");
    DclPrintf("  /DISKn           The FlashFX Disk designation, in the form: /DISK0\n");
    DclPrintf("  /Ratio:r,w,d,c   Specifies the ratio of Read, Write, Discard, and Compaction\n");
    DclPrintf("                   operations, in that order.  Defaults to 50,30,15,5 which\n");
    DclPrintf("                   denotes 50 percent reads, 30 percent writes, 15 percent\n");
    DclPrintf("                   discards, and 5 percent compactions.  The total is scaled to\n");
    DclPrintf("                   100 percent which means that a ratio of 1,1,1,1 denotes 25\n");
    DclPrintf("                   percent for each of the operations.  Empty values are assumed\n");
    DclPrintf("                   to be zero.\n");
    DclPrintf("  /Time:n          Specifies the number of minutes to run the test for.\n");
    DclPrintf("                   Default is 6.\n");
    DclPrintf("  /T:n             Specifies the number of threads per Disk.  Default is 4.\n");
    DclPrintf("  /S:n             Specifies the sample interval in seconds.  Default is 20.\n");
    DclPrintf("  /B:n             Specifies the buffer size in allocator blocks.  Default is 37.\n");
    DclPrintf("  /NoRemount       Disables remounting VBF when the sampling is done.\n");
    DclPrintf("  /Debug           Invoke the debugger when a failure occurs (where supported).\n");
    DclPrintf("  /Log:'file'[:KB] The log file specification (default to the standard display).\n");
    DclPrintf("                   The optional 'KB' designation is the buffer size in KB to use\n");
    DclPrintf("                   for the log which will limit file writes if it is sufficiently\n");
    DclPrintf("                   large.  The size may be suffixed with 'B', or 'MB' to override\n");
    DclPrintf("                   the KB default.  If the size is not supplied at all, the default\n");
    DclPrintf("                   buffer size of %uKB will be used.\n", DEFAULT_LOG_BUFFER_LEN_KB);
    DclPrintf("  /PerfLog[:sfx]   Write test results in CSV form, in addition to the standard\n");
    DclPrintf("                   test output.  Normally this is used in combination with /Log,\n");
    DclPrintf("                   and it modifies the logging behavior such that the standard\n");
    DclPrintf("                   test output goes to the screen, and the CSV output goes to the\n");
    DclPrintf("                   log file.  If /Log is not used, both types of output will go\n");
    DclPrintf("                   to the screen.  The optional 'sfx' value is a text suffix to\n");
    DclPrintf("                   append to the build number in the PerfLog CSV output.  Multiple\n");
    DclPrintf("                   /DISKn designations should NOT be specified when using /PerfLog.\n");
    DclPrintf("Multiple Disk designations may be specified.  When this is done, the /T, /S,\n");
    DclPrintf("/B, and /Ratio parameters are positional and apply only to the immediately\n");
    DclPrintf("preceding Disk.\n");

    if(pTP->dtp.pszExtraHelp)
        DclPrintf(pTP->dtp.pszExtraHelp);

    return;
}


/*-------------------------------------------------------------------
    Local: AtomicPrintf()

    Print formatted data with a variable argument list.

    This function provides a subset of the ANSI C printf()
    functionality with several extensions to support fixed
    size data types.

    See DclVSNPrintf() for the list of supported types.

    Parameters:
        pTestInfo - The TESTINFO structure to use
        pszFmt    - A pointer to the null-terminated format string
        ...       - The variable length argument list

    Return Value:
        Returns the length processed
-------------------------------------------------------------------*/
static int AtomicPrintf(
    TESTINFO       *pTestInfo,
    const char     *pszFmt,
    ...)
{
  #if DCLCONF_OUTPUT_ENABLED
    int             nLen;
    va_list         arglist;

    DclAssert(pszFmt);

    va_start(arglist, pszFmt);

    DclMutexAcquire(pTestInfo->pMutexOutput);
    {
        nLen = DclVPrintf(pszFmt, arglist);
    }
    DclMutexRelease(pTestInfo->pMutexOutput);

    va_end(arglist);

    return nLen;

  #else

    (void)pTestInfo;
    (void)pszFmt;

    return 0;
  #endif
}


#endif  /* FFXCONF_VBFSUPPORT && DCL_OSFEATURE_THREADS */


