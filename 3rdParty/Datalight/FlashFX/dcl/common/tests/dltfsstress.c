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

    This module implements a multithreaded file system stress test.                                
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltfsstress.c $
    Revision 1.21.1.2  2011/12/15 22:27:06Z  jimmb
    Removed set but not used variables
    Revision 1.21  2011/04/05 21:46:19Z  qa
    Updated to build cleanly with some picky compilers.
    Revision 1.20  2011/03/30 01:12:46Z  garyp
    Updated the database test to no longer attempt to read/write the same
    areas of a file at the same time.
    Revision 1.19  2011/03/29 03:46:35Z  garyp
    Updated to reduce the volume of output for the STREAM tests.
    Revision 1.18  2011/03/26 20:51:48Z  garyp
    Minor message reformatting -- no functional changes.
    Revision 1.17  2011/03/24 02:42:14Z  garyp
    Major update to support streaming threads.
    Revision 1.16  2010/12/08 21:28:36Z  jeremys
    Initialized a local variable to satisfy picky compilers.
    Revision 1.15  2010/11/01 03:11:31Z  garyp
    Modified to use the new DclLogOpen() function which now takes a
    flags parameter rather than a whole bunch of bools.
    Revision 1.14  2010/10/21 19:38:12Z  jimmb
    Added initialization for GNU compiler 
    Revision 1.13  2010/10/21 15:09:07Z  jimmb
    Removed the second ";" from the MACRO expansion to satisfy the
    TI ARM compiler warning.  No functional changes.
    Revision 1.12  2010/09/23 18:47:13Z  garyp
    Fixed a memory leak and cleaned up some error handling logic.
    Revision 1.11  2010/06/02 14:54:12Z  garyp
    Fixed to build cleanly with DCLCONF_OUTPUT_ENABLED turned on.
    Revision 1.10  2010/06/01 23:52:17Z  garyp
    Major update.  Added the ability to run multiple FSIOTEST threads.  Enhanced
    the Database test to keep retrying a read when a record is locked, rather 
    than giving up.  Now use the shared test FS primitives.  Greatly enhanced 
    the statistics.  Now use the LOG service to ensure printf atomicity, rather
    than a separate mutex.  Increased the default sample rate from 30 to 60 
    seconds.  Fixed to preserve the original error code from a failed thread.
    Revision 1.9  2010/04/12 05:41:08Z  garyp
    Removed an unused prototype.
    Revision 1.8  2010/04/12 03:23:35Z  garyp
    Added a "DelTree" test case.  Removed some dead testing code.
    Revision 1.7  2009/12/12 18:56:52Z  garyp
    Added some missing static keywords.
    Revision 1.6  2009/11/02 20:00:05Z  garyp
    Updated to use the shared test code.  The /MINS parameter is now /TIME.
    Updated to use the 64-bit rand() function, where appropriate.  Added 
    support for controlling tracing, stats, and the profiler.
    Revision 1.5  2009/10/05 17:11:31Z  keithg
    Removed a now unecessary typecast to DclRand() return value.
    Revision 1.4  2009/07/01 21:37:07Z  garyp
    Fixed to properly initialize a local variable.  Updated debug code.
    Revision 1.3  2009/06/25 23:49:06Z  garyp
    Updated to accommodate functions which now take DCL Instance handles.
    Revision 1.2  2009/06/15 16:33:15Z  garyp
    Corrected to eliminate a compiler warning.
    Revision 1.1  2009/06/12 02:23:20Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

#if DCL_OSFEATURE_THREADS

#include <dlprintf.h>
#include <dlapireq.h>
#include <dlstats.h>
#include <dltools.h>
#include <dlperflog.h>
#include <test/dltfsstress.h>
#include "dltshared.h"
#include "dltfsshared.h"

#define MAX_FILESPEC_LEN              (256)
#define MAX_STREAM_THREADS             (64) 
#define MAX_STREAM_FILES               (16) 
#define IO_BUFFER_SIZE                (512UL)
#define DEFAULT_LOG_BUFFER_LEN          (4)
#define DEFAULT_FSIO_THREADS            (0)
#define DEFAULT_CASE_THREADS            (4)
#define DEFAULT_DB_THREADS              (2)
#define DEFAULT_DB_READWRITE_RATIO      (5)
#define DEFAULT_DB_OPSPERITERATION   (2000)
#define DEFAULT_DB_RECORDS             (10) /* Intentionally small to ensure contention */
#define DEFAULT_STREAM_THREADS          (0) 
#define DEFAULT_ITERATIONS           (1000)
#define DEFAULT_SAMPLE_SECONDS         (60) /* Sample every 60 seconds by default */
#define DEFAULT_SAMPLE_MINUTES          (5) /* Sample every 5 minutes for "long" tests */
#define DEFAULT_TEST_SECONDS       (5 * 60)
#define DEFAULT_LONG_HOURS              (2) /* A "long" test is two hours or longer */        
#define DEFAULT_PACING_MS               (0)
#define STREAM_IO_FACTOR               (64)
#define STREAM_RESULT_COUNT            (11)

#define COPYNATIVETOLE(__CDFdst, __src)   \
   DclCopyNativeToLittleEndian(__CDFdst, __src, sizeof(*(__src)));  \
   __CDFdst = DclPtrAddByte(__CDFdst, sizeof(*(__src)))

typedef enum
{
    EXEC_LOWLIMIT = -1,
    EXEC_INITIALIZING,          /* Unstarted or initializing, MUST BE ZERO */
    EXEC_WAITING,               /* Initialized, waiting to run */
    EXEC_RUNNING,               /* Running */
    EXEC_QUITTING,              /* Cleaning up */
    EXEC_EXITING,               /* Done */
    EXEC_HIGHLIMIT
} EXECSTATE;

typedef enum
{
    PACEPOINT_LOWLIMIT = -1,
    PACEPOINT_DB_READCOMPLETE,      /* An individual DB read is complete */
    PACEPOINT_DB_WRITECOMPLETE,     /* An individual DB write is complete */
    PACEPOINT_DB_ITERCOMPLETE,      /* One DB thread iteration is complete */
    PACEPOINT_CASE_BEGIN,           /* An individual case is about to init */
    PACEPOINT_CASE_RUN,             /* An individual case is about to run */
    PACEPOINT_CASE_END,             /* An individual case is about to cleanup */
    PACEPOINT_CASE_ITERCOMPLETE,    /* One CASE thread iteration is complete */
    PACEPOINT_STREAM_IOCOMPLETE,    /* An individual Stream I/O operation has completed */
    PACEPOINT_STREAM_ITERCOMPLETE,  /* A Stream thread iteration is complete */
    PACEPOINT_HIGHLIMIT
} PACEPOINT;

typedef enum
{
    THREADTYPE_LOWLIMIT = -1,
    THREADTYPE_CASE,
    THREADTYPE_DB,
    THREADTYPE_FSIO,
    THREADTYPE_STREAM,
    THREADTYPE_HIGHLIMIT
} THREADTYPE;


/*  Percentage of the occurrances for each PACEPOINT which should
    initiate a check of relative thread progress, and invoke
    pacing if necessary (and enabled).
*/
static const unsigned anRatio[PACEPOINT_HIGHLIMIT] = {5, 10, 30, 25, 30, 35, 40, 2, 95};


typedef struct
{
    D_UINT64    ullFileSize;
    D_ATOMIC32  ulAtomicUsageCount;
    unsigned    nWriters;               /* Number of threads writing */
    unsigned    nReaders;               /* Number of threads reading */
    char        szName[MAX_FILESPEC_LEN];
} STREAMFILEINFO;

typedef struct
{
    unsigned    nPriority;
    unsigned    nFileNum;
    D_BUFFER   *pIOBuff;
    D_UINT32    ulFrameSize;
    D_UINT32    ulFrames;
    D_UINT32    ulSampleCount;              /* Protected by ulAtomicReadGate    */
    D_UINT64    ullSampleUS;                /*                    "             */
    D_UINT32    aulSampleResults[STREAM_RESULT_COUNT];  /*        "             */
    D_UINT32    ulTotalCount;               /* Thread totals for the whole test */
    D_UINT64    ullTotalUS;                 /*                    "             */
    D_UINT32    aulTotalResults[STREAM_RESULT_COUNT];   /*        "             */
    D_BOOL      fWriting;
} STREAMTHREADINFO;    


/*  holds the context given to each thread
*/
typedef struct sFSSTRESSTHREADINFO
{
    struct sFSSTRESSTESTINFO *pTI;
    char                szName[DCL_THREADNAMELEN];
    const char         *pszTypeName;
    unsigned            nThreadNum;
    unsigned            nThreadNumByType;
    unsigned            nPriority;
    THREADTYPE          nType;
    DCLTHREADHANDLE     hThread;
    DCLTHREADID         tid;
    D_UINT64            ullSeed;
    unsigned            anPacingSleeps[PACEPOINT_HIGHLIMIT];   /* Total times slept for each PACEPOINT in this thread */
    unsigned            anPacingSleepMS[PACEPOINT_HIGHLIMIT];  /* Total MS slept for each PACEPOINT in this thread */
    volatile DCLSTATUS  dclStat;            /* final status code */
    volatile EXECSTATE  ThreadExecState;    /* execution state for this thread */
    D_ATOMIC32          ulAtomicIterationsCompleted;
    D_ATOMIC32        ulAtomicWriteGate;    /* Critical Section gate for write stats */
    D_UINT32            ulWrites;           /* Totals                       */
    D_UINT32            ulWriteBytes;       /*      accumulated             */
    D_UINT32            ulWriteUS;          /*           for each           */
    D_UINT32            ulWriteMaxUS;       /*                sample        */
    D_ATOMIC32        ulAtomicReadGate;     /* Critical Section gate for read stats */
    D_UINT32            ulReads;            /* Totals                       */
    D_UINT32            ulReadBytes;        /*      accumulated             */
    D_UINT32            ulReadUS;           /*           for each           */
    D_UINT32            ulReadMaxUS;        /*                sample        */
    FSPRIMCONTROL       FSPrim;
    D_UINT32            ulTotalWrites;      /* Totals                       */
    D_UINT64            ullTotalWriteBytes; /*    for                       */
    D_UINT64            ullTotalWriteUS;    /*       the                    */
    D_UINT32            ulTotalWriteMaxUS;  /*          entire              */
    D_UINT32            ulTotalReads;       /*             test             */
    D_UINT64            ullTotalReadBytes;  /*                for           */
    D_UINT64            ullTotalReadUS;     /*                   this       */
    D_UINT32            ulTotalReadMaxUS;   /*                      thread  */
} FSSTRESSTHREADINFO;


typedef struct sFSSTRESSTESTINFO
{
    DCLSHAREDTESTINFO   sti;
    unsigned            nTotalThreads;
    unsigned            nCaseThreads;
    unsigned            nFSIOThreads;
    unsigned            nDBThreads;
    unsigned            nStreamThreads; 
    unsigned            nStreamFiles;
    unsigned            nPacingMS;
    unsigned            nNumTestCases;
    unsigned            nSample;
    unsigned            nSampleSecs;        /* Seconds between samples */
    int                 iDBRWRatio;         /* ratio of reads to writes, with special cases (see docs) */
    unsigned            nDBOpsPerIteration;
    unsigned            nDBRecords;
    unsigned           *pnIndices;
    volatile EXECSTATE  TestExecState;      /* execution state for the test as a whole */
    FSSTRESSTHREADINFO *paTC;
    STREAMTHREADINFO    aStrT[MAX_STREAM_THREADS];
    STREAMFILEINFO      aStrF[MAX_STREAM_FILES];
    D_BOOL             *pfDBRecordLocked;   /* prevent multiple updates to the same record */
    D_UINT32            ulFSBlockSize;
    D_UINT32            ulIterations;
    D_UINT32            ulDBRecordSize;
    char                szPath[MAX_FILESPEC_LEN];
    char                szFileSpec[MAX_FILESPEC_LEN];
    DCLFSSTATFS         statfs;
    DCLTHREADID         tidMaster;
  #if DCLCONF_OUTPUT_ENABLED
    DCLLOGHANDLE        hLocalLog;
  #endif
    PDCLMUTEX           pDBMutex;
    unsigned            fSupportsCWD    : 1;
    unsigned            fDoStreamHeader : 1;
} FSSTRESSTESTINFO;


/*  FSSTRESSDBHEADER

    This structure contains the main index information that is
    used by the database threads to traverse the database file.
*/
typedef struct
{
    D_UINT32    ulNumRecords;
    D_UINT32    ulFullRecordSize;
    D_UINT32    ulOffsetFirstRecord;
} FSSTRESSDBHEADER;


/*  FSSTRESSRECHEADER

    This structure holds information about a single record in
    the database.
*/
typedef struct
{
    unsigned    nRecordNum;
    D_UINT32    ulRecordSize;
    D_UINT32    ulCRC;
    D_BOOL      fRecordLocked;
} FSSTRESSRECHEADER;


typedef struct
{
    DCLSHAREDTESTINFO  *pSTI;
    FSPRIMCONTROL      *pFSPrim;
    D_BUFFER           *pDataBuff;
    size_t              nDataBuffLen;
    char                szPathBuff[MAX_FILESPEC_LEN];
    D_UINT32            ulFilesCreated;
    D_UINT64            ullTimeUS;
    unsigned            nMaxLevel;
    unsigned            nFiles;
} TREEDELINFO;


static void      ThreadPacing(  FSSTRESSTHREADINFO *pTC, D_UINT32 ulCurrentIteration, PACEPOINT nPacingPoint);
static void      DisplaySummary(FSSTRESSTESTINFO *pTI, D_UINT32 ulMS, D_BOOL fSummary);
static DCLSTATUS ProcessParams( FSSTRESSTESTINFO *pTI, const char *pszCmdName, const char *pszCmdLine);
static DCLSTATUS FSStressRun(   FSSTRESSTESTINFO *pTI);
static D_BOOL    InternalStatFS(FSSTRESSTESTINFO *pTI, DCLFSSTATFS *pStatFS);
static DCLSTATUS DBInit(        FSSTRESSTESTINFO *pTI);
static DCLSTATUS DBDestroy(     FSSTRESSTESTINFO *pTI);
static DCLSTATUS DBFindFreeRecord(FSSTRESSTHREADINFO *pTC, FSSTRESSDBHEADER *pDBHdr, unsigned *pnRecordNum);
static void *    DBThread(    void *pParam);
static void *    CaseThread(  void *pParam);
static void *    FSIOThread(  void *pParam);
static void *    StreamThread(void *pParam);
static void      ShowHelp(DCLINSTANCEHANDLE hDclInst, const char *pszCmdName);
static void      ShuffleCases(unsigned *pnIndices, unsigned nNumIndices, D_UINT64 *pullSeed);
static D_UINT32  GetKBPerSecond(D_UINT64 ullKB, D_UINT32 ulMS);
static DCLSTATUS FileCreate(   const DCLSHAREDTESTINFO *pSTI, FSPRIMCONTROL *pFSPrim, const char *pszPath, const char *pszPrefix, unsigned nFileNum, D_BUFFER *pDataBuff, size_t nDataBuffLen, D_UINT32 ulBlocks, D_BOOL fFlush);
static DCLSTATUS CreateFileSet(      DCLSHAREDTESTINFO *pSTI, FSPRIMCONTROL *pFSPrim, const char *pszPath, const char *pszPrefix, unsigned nCount, D_UINT32 *pulTotalUS, D_UINT32 *pulMaxUS, D_BUFFER *pDataBuff, size_t nDataBuffLen, D_INT32 lBlocks);
static DCLSTATUS TreeCreateRecurse(TREEDELINFO *pTDI, unsigned nLevel);
static void      RandomizeBuffer(void *pBuffer, D_UINT32 ulBuffSize, D_UINT64 *pullSeed);
static D_BOOL    ProcessStreamArgs(FSSTRESSTESTINFO *pTI, const char *pszArgs);


#define TEST_PROTOTYPES(nam)                                                    \
    static DCLSTATUS Case_##nam##_Init( const DLTFSSTRESSCASEINFO *CaseInfo);   \
    static DCLSTATUS Case_##nam##_Run(  const DLTFSSTRESSCASEINFO *CaseInfo);   \
    static DCLSTATUS Case_##nam##_Clean(const DLTFSSTRESSCASEINFO *CaseInfo);

#define TEST_DECLARE(nam) Case_##nam##_Init, Case_##nam##_Run, Case_##nam##_Clean, DCLSTRINGIZE(nam)

TEST_PROTOTYPES(MkRmDir)
TEST_PROTOTYPES(GetSetCWD)
TEST_PROTOTYPES(CreateRemove)
TEST_PROTOTYPES(ReadWrite)
TEST_PROTOTYPES(Flush)
TEST_PROTOTYPES(Stat)
TEST_PROTOTYPES(StatFs)
TEST_PROTOTYPES(Rename)
TEST_PROTOTYPES(ReadDir)
TEST_PROTOTYPES(TreeDel)

/*
    The main table of multi-threaded test sections
*/
static DLTFSSTRESSTESTCASE aFSStressTestCases[] =
{
    { TEST_DECLARE(MkRmDir) },
    { TEST_DECLARE(GetSetCWD) },
    { TEST_DECLARE(CreateRemove) },
    { TEST_DECLARE(ReadWrite) },
    { TEST_DECLARE(Flush) },
    { TEST_DECLARE(Stat) },
    { TEST_DECLARE(StatFs) },
    { TEST_DECLARE(Rename) },
    { TEST_DECLARE(ReadDir) },
    { TEST_DECLARE(TreeDel) }
};


/*-------------------------------------------------------------------
    Public: DclTestFSStressMain()

    Common tool interface for the multithreaded file system stress
    test.

    Parameters:
        pTP - A pointer to the DCLTOOLPARAMS structure to use.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSStressMain(
    DCLTOOLPARAMS      *pTP)
{
    DCLSTATUS           dclStat;
    FSSTRESSTESTINFO    ti = {{0}};

    DclAssertWritePtr(pTP, sizeof(*pTP));

    /*  Ensure the DCL instance is already initialized before bothering
        to do anything else.
    */
    if(!pTP->hDclInst)
        return DCLSTAT_INST_NOTINITIALIZED;

    ti.sti.hDclInst = pTP->hDclInst;

    /*  If the command line switches are not valid (or if
        it was a help request), return the status code.
    */
    dclStat = ProcessParams(&ti, pTP->pszCmdName, pTP->pszCmdLine);
    if(dclStat != DCLSTAT_SUCCESS)
         return dclStat;

    dclStat = DclTestLogOpen(&ti.sti);
    if(dclStat != DCLSTAT_SUCCESS && dclStat != DCLSTAT_FEATUREDISABLED)
        goto MainCleanup;

  #if DCLCONF_OUTPUT_ENABLED
    if(!ti.sti.hLog)
        ti.hLocalLog = DclLogOpen(ti.sti.hDclInst, NULL, 0, DCLLOGFLAGS_SHADOW);
  #endif

    DclPrintf("DCL File System Multithreaded Stress Test\n");
    DclSignOn(FALSE);

    ti.tidMaster = DclOsThreadID();

    /*  Seed the pseudo-random number generator
    */
    if(!ti.sti.ullRandomSeed)
        ti.sti.ullRandomSeed = DclOsTickCount();

    DclTestRequestorOpen(&ti.sti, ti.szPath, ti.statfs.szDeviceName);

    DclPrintf("  Device Name: %32s\n",                               ti.statfs.szDeviceName);
    DclPrintf("  Base Path:   %32s\n",                               ti.szPath);
    DclPrintf("  File System Total Size:            %10lU KB\n",    (ti.statfs.ulTotalBlocks * ti.statfs.ulBlockSize) / 1024);
    DclPrintf("  File System Free Space:            %10lU KB\n",    (ti.statfs.ulFreeBlocks * ti.statfs.ulBlockSize) / 1024);
    DclPrintf("  File System Block Size:            %10lU B\n",      ti.statfs.ulBlockSize);
    DclPrintf("  Random Seed:                       %10llU\n",       ti.sti.ullRandomSeed);
    DclPrintf("  Max Iterations:                    %10lU\n",        ti.ulIterations);
    DclPrintf("  Max Seconds:                       %10lU\n",        ti.sti.ulTestSeconds);
    DclPrintf("  Sample Rate:                       %10u seconds\n", ti.nSampleSecs);
    DclPrintf("  Pacing:                            %10u ms\n",      ti.nPacingMS);
    DclPrintf("  Case Threads:                      %10u\n",         ti.nCaseThreads);
    DclPrintf("  FSIO Threads:                      %10u\n",         ti.nFSIOThreads);
    DclPrintf("  Stream Threads:                    %10u\n",         ti.nStreamThreads);
    DclPrintf("  Database Threads:                  %10u\n",         ti.nDBThreads);

    if(ti.nDBThreads)
    {
        DclPrintf("    DB Header Size                   %10u bytes\n", sizeof(FSSTRESSDBHEADER));
        DclPrintf("    DB Record Header Size            %10u bytes\n", sizeof(FSSTRESSRECHEADER));
        DclPrintf("    DB Record Max Data Size          %10u bytes\n", ti.ulDBRecordSize);
        DclPrintf("    DB Total Records                 %10u\n",       ti.nDBRecords);
        DclPrintf("    DB Operations per Test Iteration %10u\n",       ti.nDBOpsPerIteration);

        if(ti.iDBRWRatio > 0)
            DclPrintf("    DB Read:Write Ratio                     %u:1\n", ti.iDBRWRatio);
        else if(ti.iDBRWRatio == 0)
            DclPrintf("    DB Read:Write Ratio             Writes Only\n");
        else if(ti.iDBRWRatio == -1)
            DclPrintf("    DB Read:Write Ratio              Reads Only\n");
        else
            DclPrintf("    DB Read:Write Ratio                     1:%u\n", -ti.iDBRWRatio);
    }

    DclPrintf("\n");

    /*---------------------------------------------------------
        Write data to the performance log, if enabled.  Note
        that any changes to the test name or category must be
        accompanied by changes to perffsio.bat.  Any changes to
        the actual data fields recorded here requires changes
        to the various spreadsheets which track this data.
    ---------------------------------------------------------*/
    {
        DCLPERFLOGHANDLE    hPerfLog;
        hPerfLog = DCLPERFLOG_OPEN(ti.sti.fPerfLog, ti.sti.hLog, "FSSTRESS", "Configuration", NULL, ti.sti.szPerfLogSuffix);
        DCLPERFLOG_NUM(   hPerfLog, "DiskSizeKB",   (ti.statfs.ulTotalBlocks * ti.statfs.ulBlockSize) / 1024);
        DCLPERFLOG_NUM(   hPerfLog, "FreeSpaceKB",  (ti.statfs.ulFreeBlocks * ti.statfs.ulBlockSize) / 1024);
        DCLPERFLOG_NUM(   hPerfLog, "BlockSize",     ti.statfs.ulBlockSize);
        DCLPERFLOG_NUM64( hPerfLog, "RandomSeed",    ti.sti.ullRandomSeed);
        DCLPERFLOG_NUM(   hPerfLog, "MaxSeconds",    ti.sti.ulTestSeconds);
        DCLPERFLOG_NUM(   hPerfLog, "MaxIterations", ti.ulIterations);
        DCLPERFLOG_NUM(   hPerfLog, "CaseThreads",   ti.nCaseThreads);
        DCLPERFLOG_NUM(   hPerfLog, "DBThreads",     ti.nDBThreads);
        DCLPERFLOG_NUM(   hPerfLog, "FSIOThreads",   ti.nFSIOThreads);
        DCLPERFLOG_NUM(   hPerfLog, "StreamThreads", ti.nStreamThreads);
        DCLPERFLOG_NUM(   hPerfLog, "SampleSecs",    ti.nSampleSecs);
        DCLPERFLOG_NUM(   hPerfLog, "PacingMS",      ti.nPacingMS);
        DCLPERFLOG_NUM(   hPerfLog, "DBHdrSize",     sizeof(FSSTRESSDBHEADER));
        DCLPERFLOG_NUM(   hPerfLog, "DBRecHdrSize",  sizeof(FSSTRESSRECHEADER));
        DCLPERFLOG_NUM(   hPerfLog, "DBRecSize",     ti.ulDBRecordSize);
        DCLPERFLOG_NUM(   hPerfLog, "DBRatio",       ti.iDBRWRatio);
        DCLPERFLOG_WRITE( hPerfLog);
        DCLPERFLOG_CLOSE( hPerfLog);
    }

    /*  Perform the test
    */
    dclStat = FSStressRun(&ti);

  MainCleanup:

    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("\n%s: Error %lX\n\n", pTP->pszCmdName, dclStat);
    else
        DclPrintf("%s completed successfully\n\n", pTP->pszCmdName);

    DclTestRequestorClose(&ti.sti);

  #if DCLCONF_OUTPUT_ENABLED
    if(ti.hLocalLog)
        DclLogClose(ti.sti.hDclInst, ti.hLocalLog);
    else
  #endif        
        DclTestLogClose(&ti.sti);

    if(ti.sti.pStatsBuff)
        DclMemFree(ti.sti.pStatsBuff);

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: FSStressRun()

    Runs a full suite of random tests.  Creates the specified number
    of threads, which each run the specified number of iterations
    over all the test cases.  Each thread gets a different random
    number seed, with thread 0 getting ullSeed, and thread n getting
    ullSeed + n.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS FSStressRun(
    FSSTRESSTESTINFO       *pTest)
{
    FSSTRESSTHREADINFO     *pTC = NULL; /* Not necessary, but some compilers seem to think so... */
    DCLTHREADATTR          *pTA;
    DCLSTATUS               dclStat;
    DCLSTATUS               dclStat2;
    char                    mutexName[DCL_MUTEXNAMELEN];
    unsigned                ii;
    D_BOOL                  fDatabaseInitialized = FALSE;
    char                    szFullCurrentName[MAX_FILESPEC_LEN];

    DclAssertWritePtr(pTest, sizeof(*pTest));
    DclAssert(pTest->nTotalThreads);

    pTest->nNumTestCases = DCLDIMENSIONOF(aFSStressTestCases);

    /*  First do a GetCWD to see if the functionality is supported.
    */
    dclStat = DclFsDirGetWorking(szFullCurrentName, DCLDIMENSIONOF(szFullCurrentName));
    if(dclStat == DCLSTAT_SUCCESS)
    {
        pTest->fSupportsCWD = TRUE;
    }
    else
    {
        DclPrintf("Get/Set CWD functionality is not supported in this environment -- test case will be skipped.\n");
    }

    /*  create the mutex used for database synchronization
    */
    DclSNPrintf(mutexName, sizeof(mutexName), D_PRODUCTPREFIX"FSMdb");
    pTest->pDBMutex = DclMutexCreate(mutexName);
    if(!pTest->pDBMutex)
    {
        dclStat = DCLSTAT_MUTEXCREATEFAILED;
        goto EXIT;
    }

    /*  allocate storage for our thread context structures
    */
    pTest->paTC = DclMemAllocZero(pTest->nTotalThreads * sizeof(FSSTRESSTHREADINFO));
    if(!pTest->paTC)
    {
        dclStat = DCLSTAT_MEMALLOCFAILED;
        goto EXIT;
    }

    /*  initialize the database segment to its initial state if we have any
        database threads
    */
    if(pTest->nDBThreads)
    {
        dclStat = DBInit(pTest);
        if(dclStat != DCLSTAT_SUCCESS)
            goto EXIT;

        fDatabaseInitialized = TRUE;
    }

    /*  Create the thread attributes - the default attributes will be fine
        for most threads...
    */
    pTA = DclOsThreadAttrCreate();

    /*  Loop around, creating the correct number of threads
    */
    for(ii = 0; ii < pTest->nTotalThreads; ii++)
    {
        /*  Setup pointers for this thread.
        */
        pTC = &pTest->paTC[ii];

        /*  fill out the context for this thread
        */
        pTC->pTI = pTest;
        pTC->ullSeed = pTest->sti.ullRandomSeed + ii;
        pTC->nThreadNum = ii;
        pTC->nPriority = DCL_THREADPRIORITY_NORMAL;

        /*  Create all threads for any of the various category
        */
        if(ii < pTest->nCaseThreads)
        {
            /*  Create a "regular" thread.
            */
            DclSNPrintf(pTC->szName, sizeof(pTC->szName), D_PRODUCTPREFIX"FSC%02x", ii);
            pTC->pszTypeName = "-Case-";
            pTC->nType = THREADTYPE_CASE;
            pTC->nThreadNumByType = ii;
            dclStat = DclOsThreadCreate(&pTC->hThread, pTC->szName, pTA, CaseThread, pTC);
        }
        else if(ii < pTest->nCaseThreads + pTest->nDBThreads)
        {
            /*  Create a "database" thread.
            */
            DclSNPrintf(pTC->szName, sizeof(pTC->szName), D_PRODUCTPREFIX"FSD%02x", ii);
            pTC->pszTypeName = "--DB--";
            pTC->nType = THREADTYPE_DB;
            pTC->nThreadNumByType = ii - pTest->nCaseThreads;
            dclStat = DclOsThreadCreate(&pTC->hThread, pTC->szName, pTA, DBThread, pTC);
        }
        else if(ii < pTest->nCaseThreads + pTest->nDBThreads + pTest->nFSIOThreads)
        {
            /*  Create an "FSIO" thread.
            */
            DclSNPrintf(pTC->szName, sizeof(pTC->szName), D_PRODUCTPREFIX"FSF%02x", ii);
            pTC->pszTypeName = "-FSIO-";
            pTC->nType = THREADTYPE_FSIO;
            pTC->nThreadNumByType = ii - (pTest->nCaseThreads + pTest->nDBThreads);
            dclStat = DclOsThreadCreate(&pTC->hThread, pTC->szName, pTA, FSIOThread, pTC);
        }
        else
        {
            STREAMFILEINFO     *pStrF;
            STREAMTHREADINFO   *pStrT;
            char                szScaleBuff[32];
            char                szScaleBuff2[32];
            
            /*  Create a "Stream" thread.
            */
            DclSNPrintf(pTC->szName, sizeof(pTC->szName), D_PRODUCTPREFIX"FSS%02x", ii);
            pTC->pszTypeName = "Stream";
            pTC->nType = THREADTYPE_STREAM;
            pTC->nThreadNumByType = ii - (pTest->nCaseThreads + pTest->nDBThreads + pTest->nFSIOThreads);

            pStrT = &pTest->aStrT[pTC->nThreadNumByType];
            pStrF = &pTest->aStrF[pStrT->nFileNum];

            pTC->nPriority = pStrT->nPriority;
            
            /*  Use default I/O size if not specified.
            */
            if(!pStrT->ulFrameSize)
                pStrT->ulFrameSize = pTest->ulFSBlockSize * STREAM_IO_FACTOR;

            /*  Use default file size if not specified.
            */
            if(!pStrF->ullFileSize)
                pStrF->ullFileSize = ((D_UINT64)pTest->statfs.ulFreeBlocks) * pTest->ulFSBlockSize / (2 * pTest->nStreamFiles);

            if(pStrF->ullFileSize < pStrT->ulFrameSize)
                pStrF->ullFileSize = pStrT->ulFrameSize;

            pStrT->ulFrames = (D_UINT32)(pStrF->ullFileSize / pStrT->ulFrameSize);
            
            pStrT->pIOBuff = DclMemAlloc(pStrT->ulFrameSize);
            if(!pStrT->pIOBuff)
            {
                dclStat = DCLSTAT_OUTOFMEMORY;
                goto RunCleanup;
            }

            /*  If this is the first thread for this file, create the file.
            */
            if(pStrF->nReaders + pStrF->nWriters == 0)
            {
                D_UINT64            ullTotalSize = 0;
                DCLFSFILEHANDLE     hFile;
                FSSTRESSTHREADINFO *pTI = pTC;  /* So the PRIM* macros work... */
                
                DclSNPrintf(pStrF->szName, sizeof(pStrF->szName), "%sFSStreamFile%u.dat",
                    pTest->szPath, pStrT->nFileNum);

                dclStat = PRIMCREATE(pStrF->szName, &hFile);
                if(dclStat != DCLSTAT_SUCCESS)
                {
                    DclMemFree(pStrT->pIOBuff);
                    goto RunCleanup;
                }

                while(ullTotalSize + pStrT->ulFrameSize <= pStrF->ullFileSize)
                {
                    D_UINT32    ulTransferred;
                    
                    RandomizeBuffer(pStrT->pIOBuff, pStrT->ulFrameSize, &pTC->ullSeed);
                    
                    dclStat = PRIMWRITE(hFile, pStrT->pIOBuff, pStrT->ulFrameSize, &ulTransferred);
                    if((dclStat != DCLSTAT_SUCCESS) || (ulTransferred != pStrT->ulFrameSize))
                    {
                        /*  Ensure any original error code is preserved
                        */
                        if(dclStat == DCLSTAT_SUCCESS)
                            dclStat = DCLSTAT_FS_WRITEFAILED;

                        PRIMCLOSE(hFile);
                        PRIMDELETE(pStrF->szName);
                        DclMemFree(pStrT->pIOBuff);
                        goto RunCleanup;
                    }

                    ullTotalSize += ulTransferred;
                }

                /*  Record the actual file size, might have rounded down...
                */
                pStrF->ullFileSize = ullTotalSize;

                dclStat = PRIMCLOSE(hFile);
                DclAssert(dclStat == DCLSTAT_SUCCESS);

                /*  For each file, keep track of the number of read and
                    write threads.
                */
                if(pStrT->fWriting)
                    pStrF->nWriters++;
                else
                    pStrF->nReaders++;
            }            

            DclPrintf("Stream thread %s (P%u): %s %s frames %s %s file %s (%lU total frames)\n",
                pTC->szName, 
                pStrT->nPriority,
                pStrT->fWriting ? "Writing" : "Reading", 
                DclScaleBytes(pStrT->ulFrameSize, szScaleBuff, sizeof(szScaleBuff)),
                pStrT->fWriting ? "to" : "from", 
                DclScaleItems(pStrF->ullFileSize, 1, szScaleBuff2, sizeof(szScaleBuff2)),
                pStrF->szName, 
                pStrT->ulFrames);

            dclStat = DclOsThreadAttrSetPriority(pTA, (D_UINT16)pTC->nPriority);
            DclAssert(dclStat == DCLSTAT_SUCCESS);
            
            dclStat = DclOsThreadCreate(&pTC->hThread, pTC->szName, pTA, StreamThread, pTC);
        }

        if(dclStat != DCLSTAT_SUCCESS)
            break;

        /*  Sleep until the new thread is done initializing
        */
        while(pTC->ThreadExecState == EXEC_INITIALIZING)
            DclOsSleep(1);

        if(pTest->sti.nVerbosity > DCL_VERBOSE_NORMAL)
        {
            DclPrintf("Started %s Thread %u: Name=%s Handle=%lX TID=%lX\n",
                pTC->pszTypeName, ii, pTC->szName, pTC->hThread, pTC->tid);
        }
        
        if(pTC->ThreadExecState != EXEC_WAITING)
        {
            /*  If a thread did not start as expected, mark the test
                state as "quitting" so any threads which did start will
                know to quit.
            */
            pTest->TestExecState = EXEC_QUITTING;
            break;
        }

        /*  Restore the default priority for subsequent threads
        */
        dclStat = DclOsThreadAttrSetPriority(pTA, DCL_THREADPRIORITY_NORMAL);
        DclAssert(dclStat == DCLSTAT_SUCCESS);
    }

  RunCleanup:

    /*  get rid of the thread attributes
    */
    DclOsThreadAttrDestroy(pTA);

    pTC->pTI->fDoStreamHeader = TRUE;

    if(pTest->TestExecState == EXEC_INITIALIZING)
    {
        D_UINT32        ulLastMS = D_UINT32_MAX;
        DCLTIMESTAMP    ts = DclTimeStamp();

        DclTestInstrumentationStart(&pTest->sti);

        /*  Changing the test state to "running" turns everything loose...
        */
        pTest->TestExecState = EXEC_RUNNING;

        /*  Periodically scan the drive threads to see if anything is still
            running.
        */
        while(pTest->TestExecState == EXEC_RUNNING || pTest->TestExecState == EXEC_QUITTING)
        {
            D_UINT32    ulMS = DclTimePassed(ts);
            D_BOOL      fStillRunning = FALSE;

            /*  If the time has expired, change the test state to "quitting",
                however don't break out of the loop.  If we are running one
                or more FSIO threads, they may not complete an iteration for
                quite a while after the test time runs out.  Staying in the
                loop allows the summary display to continue to update, so it
                is obvious that the test is still running.  Otherwise it will
                appear as if the system is hung.
            */                
            if(ulMS >= pTest->sti.ulTestSeconds * 1000)
                pTest->TestExecState = EXEC_QUITTING;

            if(ulLastMS == D_UINT32_MAX || ulMS >= ulLastMS + (pTest->nSampleSecs*1000))
            {
                if(ulLastMS != D_UINT32_MAX)
                {
                    DisplaySummary(pTest, ulMS, FALSE);
                    pTest->fDoStreamHeader = TRUE;
                }
                
                ulLastMS = ulMS;
            }

            /*  Regardless how frequently we are displaying an intermediate
                summary, we want to actively watch for all threads having
                completed, or more importantly, a thread which reports an
                error, so that we can quit the test in a timely fashion,
                before too much water has gone under the bridge.
            */
            DclOsSleep(100);

            for(ii = 0; ii < pTest->nTotalThreads; ii++)
            {
                /*  Keep a record of whether any thread is still running.
                */
                if(pTest->paTC[ii].ThreadExecState < EXEC_EXITING)
                    fStillRunning = TRUE;

                /*  If any of the threads had an abnormal termination, set
                    the flag to indicate we should exit, and quit early.
                */
                if(pTest->paTC[ii].dclStat != DCLSTAT_SUCCESS)
                {
                    fStillRunning = FALSE;
                    dclStat = pTest->paTC[ii].dclStat;
                    break;
                }
            }

            /*  If all the threads are now in the "exiting" state, then
                break out, regardless what the time remaining is.
            */
            if(!fStillRunning)
                break;
        }

        pTest->TestExecState = EXEC_QUITTING;

        DclTestInstrumentationStop(&pTest->sti, "Main");

        /*  Display a final summary.
        */
        DisplaySummary(pTest, DclTimePassed(ts), TRUE);
    }

    /*  Loop until all the drive threads have exited.
    */
    for(ii = 0; ii < pTest->nTotalThreads; ii++)
    {
        pTC = &pTest->paTC[ii];

        while(pTC->ThreadExecState != EXEC_EXITING)
            DclOsSleep(10);

        if(pTest->nPacingMS)
        {
            if(ii == 0)
                DclPrintf("  Pacing Statistics:\n");
            
            if(pTC->nType == THREADTYPE_DB)
            {
                DclPrintf("    %6s Thread %2u: Read=%u/%ums Write=%u/%ums Iter=%u/%ums\n",
                    pTC->pszTypeName,
                    ii,
                    pTC->anPacingSleeps[PACEPOINT_DB_READCOMPLETE],   pTC->anPacingSleepMS[PACEPOINT_DB_READCOMPLETE],
                    pTC->anPacingSleeps[PACEPOINT_DB_WRITECOMPLETE],  pTC->anPacingSleepMS[PACEPOINT_DB_WRITECOMPLETE],
                    pTC->anPacingSleeps[PACEPOINT_DB_ITERCOMPLETE],   pTC->anPacingSleepMS[PACEPOINT_DB_ITERCOMPLETE]);
            }
            else if(pTC->nType == THREADTYPE_STREAM)
            {
                DclPrintf("    %6s Thread %2u: IO=%u/%ums Iter=%u/%ums\n",
                    pTC->pszTypeName,
                    ii,
                    pTC->anPacingSleeps[PACEPOINT_STREAM_IOCOMPLETE],   pTC->anPacingSleepMS[PACEPOINT_STREAM_IOCOMPLETE],
                    pTC->anPacingSleeps[PACEPOINT_STREAM_ITERCOMPLETE], pTC->anPacingSleepMS[PACEPOINT_STREAM_ITERCOMPLETE]);
            }
            else
            {
                DclAssert(pTC->nType == THREADTYPE_CASE);
                
                DclPrintf("    %6s Thread %2u: Begin=%u/%ums Run=%u/%ums End=%u/%ums Iter=%u/%ums\n",
                    pTC->pszTypeName,
                    ii,
                    pTC->anPacingSleeps[PACEPOINT_CASE_BEGIN],        pTC->anPacingSleepMS[PACEPOINT_CASE_BEGIN],
                    pTC->anPacingSleeps[PACEPOINT_CASE_RUN],          pTC->anPacingSleepMS[PACEPOINT_CASE_RUN],
                    pTC->anPacingSleeps[PACEPOINT_CASE_END],          pTC->anPacingSleepMS[PACEPOINT_CASE_END],
                    pTC->anPacingSleeps[PACEPOINT_CASE_ITERCOMPLETE], pTC->anPacingSleepMS[PACEPOINT_CASE_ITERCOMPLETE]);
            }
        }
    }

    DclPrintf("Test exiting...\n");
    DclOsSleep(1000);

    /*  Even though we've been assured that all the threads are exiting by
        waiting for the EXEC_EXITING state, and we know that is the very
        last thing each thread does, we don't technically know that the
        thread has actually exited.  Therefore, do this at the very end,
        after the 1000 ms sleep above, and hope it's safe...
    */
    for(ii = 0; ii < pTest->nTotalThreads; ii++)
    {
        pTC = &pTest->paTC[ii];

        /*  Make sure any original error code is preserved.
        */
        dclStat2 = DclOsThreadDestroy(pTC->hThread);
        if(dclStat2 != DCLSTAT_SUCCESS)
        {
            DclPrintf("Error %lX destroying thread %u\n", dclStat2, ii);
            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = dclStat2;
        }
    }

  EXIT:

    if(fDatabaseInitialized)
    {
        /*  Make sure any original error code is preserved.
        */
        dclStat2 = DBDestroy(pTest);
        if(dclStat == DCLSTAT_SUCCESS)
            dclStat = dclStat2;
    }
    
    if(pTest->paTC)
        DclMemFree(pTest->paTC);

    if(pTest->pDBMutex)
        DclMutexDestroy(pTest->pDBMutex);

    DclPrintf("FSStressRun:  Complete %lX\n", dclStat);

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: DBReadRecord()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DBReadRecord(
    FSSTRESSTHREADINFO     *pTC,
    DCLFSFILEHANDLE         hFile,
    unsigned                nRecordNum,
    D_UINT32                ulRecordOffset,
    D_UINT32                ulFullRecordSize,
    D_UINT32               *pulRealRecordSize)
{
    D_UINT32                ulTransferred;
    D_UINT32                ulCRC;
    DCLSTATUS               dclStat;
    FSSTRESSTHREADINFO     *pTI = pTC;
    FSSTRESSRECHEADER      *pRecord = NULL;
    D_UINT32                ulRealSize = ulFullRecordSize;

    DclAssertWritePtr(pTC, sizeof(*pTC));
    DclAssertWritePtr(pulRealRecordSize, sizeof(*pulRealRecordSize));
    DclAssert(hFile);

    if(pTC->pTI->sti.nVerbosity > DCL_VERBOSE_LOUD)
    {
        DclPrintf("DB Thread %u, record %u, about to read\n",
            pTC->nThreadNum, nRecordNum);
    }

    /*  Allocate space for a single max-size record
    */
    pRecord = DclMemAlloc(ulFullRecordSize);
    if(!pRecord)
    {
        dclStat = DCLSTAT_MEMALLOCFAILED;

        DclPrintf("DBReadRecord() record %u failed at line %u, status=%lX\n",
            nRecordNum, __LINE__, dclStat);

        goto EXIT;
    }

    /*  Seek to that record number
    */
    dclStat = PRIMSEEK(hFile, (D_INT32)ulRecordOffset, DCLFSFILESEEK_SET);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("DBReadRecord() record %u failed at line %u, status=%lX\n",
            nRecordNum, __LINE__, dclStat);
        
        goto EXIT;
    }

    dclStat = PRIMREAD(hFile, pRecord, ulRealSize, &ulTransferred);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("DBReadRecord() record %u failed at line %u, status=%lX\n",
            nRecordNum, __LINE__, dclStat);
        
        goto EXIT;
    }

    if(ulTransferred != ulRealSize)
    {
        DclPrintf("Read failed: Requested %lU bytes, but only read %lU bytes\n",
            ulRealSize, ulTransferred);

        dclStat = DCLSTAT_CURRENTLINE;
        goto EXIT;
    }

    /*  Verify the information in it is correct.  
    */
    if(pRecord->nRecordNum != nRecordNum)
    {
        dclStat = DCLSTAT_CURRENTLINE;
        DclPrintf("DBReadRecord() record %u failed at line %u, status=%lX\n",
            nRecordNum, __LINE__, dclStat);
        
        goto EXIT;
    }

    if(!pRecord->fRecordLocked)
    {
        /*  Provide the caller with the actual record size
        */
        *pulRealRecordSize = pRecord->ulRecordSize;
    }
    else
    {
        DclProductionError();
    }
 
    ulCRC = DclCrc32Update(0, (const D_BUFFER*)(pRecord + 1), pRecord->ulRecordSize);
    if(ulCRC != pRecord->ulCRC)
    {
        DclPrintf("DB Thread %u Rec#%3u expected CRC %lX, got CRC %lX\n", 
            pTC->nThreadNum, nRecordNum, pRecord->ulCRC, ulCRC);

        dclStat = DCLSTAT_CURRENTLINE;
        goto EXIT;
    }

    if(pTC->pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
    {
        DclPrintf("DB Thread %u Rec#%3u CRC=%lX %4lU bytes read successfully\n",
            pTC->nThreadNum, nRecordNum, pRecord->ulCRC, pRecord->ulRecordSize);
    }
 
  EXIT:

    if(pRecord)
        DclMemFree(pRecord);

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: DBReadRandomRecord()

    Reads in one random record and verifies the contained information.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DBReadRandomRecord(
    FSSTRESSTHREADINFO     *pTC,
    DCLFSFILEHANDLE         hFile)
{
    DCLSTATUS               dclStat;
    FSSTRESSDBHEADER        dbHdr;
    unsigned                nRecordNum;
    D_UINT32                ulTransferred;
    D_UINT32                ulRealRecordSize = 0;
    DCLTIMESTAMP            ts;
    FSSTRESSTHREADINFO     *pTI = pTC;

    DclAssertWritePtr(pTC, sizeof(*pTC));
    DclAssert(hFile);

    ts = DclTimeStamp();

    /*  Set the current offset into the file to the database index
    */
    dclStat = PRIMSEEK(hFile, 0, DCLFSFILESEEK_SET);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("DBReadRecordRandom() failed at line %u, status=%lX\n", __LINE__, dclStat);
        goto EXIT;
    }

    /*  Read in the database index
    */
    dclStat = PRIMREAD(hFile, &dbHdr, sizeof(dbHdr), &ulTransferred);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("DBReadRecordRandom() failed at line %u, status=%lX\n", __LINE__, dclStat);
        goto EXIT;
    }

    if(ulTransferred != sizeof(dbHdr))
    {
        dclStat = DCLSTAT_CURRENTLINE;
        DclPrintf("DBReadRecordRandom() failed at line %u, status=%lX\n", __LINE__, dclStat);
        goto EXIT;
    }

    dclStat = DBFindFreeRecord(pTC, &dbHdr, &nRecordNum);
    if(dclStat != DCLSTAT_SUCCESS)
        goto EXIT;

    /*  Read the record
    */
    dclStat = DBReadRecord(pTC, hFile, nRecordNum,
        dbHdr.ulOffsetFirstRecord + (nRecordNum * dbHdr.ulFullRecordSize),
        dbHdr.ulFullRecordSize, &ulRealRecordSize);

    if(dclStat == DCLSTAT_SUCCESS)
    {
        D_UINT32 ulElapsedUS = DclTimePassedUS(ts);
        
        if(DclCriticalSectionEnter(&pTC->ulAtomicReadGate, 2000, TRUE))
        {
            /*  Should never wrap since D_UINT32_MAX microseconds is more 
                than an hour, and we should be sampling much more often
                than that.
            */                    
            DclAssert(D_UINT32_MAX - pTC->ulReadUS >= ulElapsedUS);

            pTC->ulReads++;            
            pTC->ulReadBytes += ulRealRecordSize;
            pTC->ulReadUS += ulElapsedUS;
            if(pTC->ulReadMaxUS < ulElapsedUS)
                pTC->ulReadMaxUS = ulElapsedUS;

            DclCriticalSectionLeave(&pTC->ulAtomicReadGate);
        }
        else
        {
            DclPrintf("Error entering critical section while gathering read DB stats -- ignoring...\n");
        }
    }

    if(pTC->pTI->sti.nVerbosity > DCL_VERBOSE_LOUD)
    {
        DclPrintf("DB Thread %u, record %u, read returned status %lX\n",
            pTC->nThreadNum, nRecordNum, dclStat);
    }

    /*  Release this record
    */
    pTC->pTI->pfDBRecordLocked[nRecordNum] = FALSE;

  EXIT:

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: DBWriteRecord()

    Writes out one record with random data out to the database file

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DBWriteRecord(
    FSSTRESSTHREADINFO     *pTC,
    DCLFSFILEHANDLE         hFile,
    unsigned                nRecordNum,
    D_UINT32                ulRecordOffset,
    D_UINT32                ulDataSize,
    D_UINT64               *pullSeed)
{
    DCLSTATUS               dclStat;
    FSSTRESSRECHEADER      *pRecord;
    D_BUFFER               *pData;
    D_UINT32                ulDataByte;
    D_UINT32                ulTransferred;
    FSSTRESSTHREADINFO     *pTI = pTC;

/*  DclAssert(pTC);  allowed to be NULL */
    DclAssert(hFile);
    DclAssertWritePtr(pullSeed, sizeof(*pullSeed));
/*  DclAssert(ulDataSize);  zero-length records are allowed */

    if(pTC && pTC->pTI->sti.nVerbosity > DCL_VERBOSE_LOUD)
    {
        DclPrintf("DB Thread %u, record %u, about to write\n",
            pTC->nThreadNum, nRecordNum);
    }

    /*  Allocate space for a single max-size record
    */
    pRecord = DclMemAllocZero(sizeof(FSSTRESSRECHEADER) + ulDataSize);
    if(!pRecord)
    {
        dclStat = DCLSTAT_MEMALLOCFAILED;

        DclPrintf("DBWriteRecord() record %u failed at line %u, status=%lX\n",
            nRecordNum, __LINE__, dclStat);

        goto EXIT;
    }

    /*  Set up the record header.  It is written seperately first to lock
        the record so readers won't read a partial (incorrect) record.
    */
    pRecord->nRecordNum = nRecordNum;
    pRecord->ulRecordSize = ulDataSize;
    pRecord->fRecordLocked = TRUE;

    /*  Get the start of the data buffer
    */
    pData = (D_BUFFER*)(pRecord + 1);

    /*  Create the random garbage in the data
    */
    for(ulDataByte = 0; ulDataByte < ulDataSize; ulDataByte++)
    {
        pData[ulDataByte] = (D_UINT8)DclRand64(pullSeed);
    }

    /*  Compute the current checksum
    */
    pRecord->ulCRC = DclCrc32Update(0, pData, ulDataSize);

    /*  Set the current offset into the file
    */
    dclStat = PRIMSEEK(hFile, (D_INT32)ulRecordOffset, DCLFSFILESEEK_SET);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("DBWriteRecord() record %u failed at line %u, status=%lX\n",
            nRecordNum, __LINE__, dclStat);
        
        goto EXIT;
    }

    /*  Write out the record header to lock the record
    */
    dclStat = PRIMWRITE(hFile, pRecord, sizeof(*pRecord), &ulTransferred);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("DBWriteRecord() record %u failed at line %u, status=%lX\n",
            nRecordNum, __LINE__, dclStat);
        
        goto EXIT;
    }

    if(ulTransferred != sizeof(*pRecord))
    {
        DclPrintf("Write failed: Requested %lU bytes, but only wrote %lU bytes\n",
            sizeof(*pRecord), ulTransferred);

        dclStat = DCLSTAT_CURRENTLINE;
        goto EXIT;
    }

    dclStat = PRIMFLUSH(hFile);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("DBWriteRecord() record %u failed at line %u, status=%lX\n",
            nRecordNum, __LINE__, dclStat);
        
        goto EXIT;
    }

    /*  Write out the data for the record
    */
    dclStat = PRIMWRITE(hFile, pData, ulDataSize, &ulTransferred);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("DBWriteRecord() record %u failed at line %u, status=%lX\n",
            nRecordNum, __LINE__, dclStat);
        
        goto EXIT;
    }

    if(ulTransferred != ulDataSize)
    {
        DclPrintf("Write failed: Requested %lU bytes, but only wrote %lU bytes\n",
            ulDataSize, ulTransferred);

        dclStat = DCLSTAT_CURRENTLINE;
        goto EXIT;
    }

    if(pTC && pTC->pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
    {
        DclPrintf("DB Thread %u Rec#%3u CRC=%lX %4lU bytes written successfully\n",
            pTC->nThreadNum, nRecordNum, pRecord->ulCRC, ulDataSize);
    }

    dclStat = PRIMFLUSH(hFile);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("DBWriteRecord() record %u failed at line %u, status=%lX\n",
            nRecordNum, __LINE__, dclStat);
        
        goto EXIT;
    }

    /*  Reset the record lock field
    */
    pRecord->fRecordLocked = FALSE;

    /*  Seek back to the beginning of the record
    */
    dclStat = PRIMSEEK(hFile, (D_INT32)ulRecordOffset, DCLFSFILESEEK_SET);
    if(DCLSTAT_SUCCESS != dclStat)
    {
        DclPrintf("DBWriteRecord() record %u failed at line %u, status=%lX\n",
            nRecordNum, __LINE__, dclStat);
        
        goto EXIT;
    }

    /*  Finally, write out the record header to unlock the record
    */
    dclStat = PRIMWRITE(hFile, pRecord, sizeof(*pRecord), &ulTransferred);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("DBWriteRecord() record %u failed at line %u, status=%lX\n",
            nRecordNum, __LINE__, dclStat);
        
        goto EXIT;
    }

    if(ulTransferred != sizeof(*pRecord))
    {
        DclPrintf("Write failed: Requested %lU bytes, but only wrote %lU bytes\n",
            sizeof(*pRecord), ulTransferred);

        dclStat = DCLSTAT_CURRENTLINE;
        goto EXIT;
    }

  EXIT:

    if(pRecord)
        DclMemFree(pRecord);

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: DBWriteRandomRecord()

    Writes one random record.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DBWriteRandomRecord(
    FSSTRESSTHREADINFO *pTC,
    DCLFSFILEHANDLE     hFile)
{
    DCLSTATUS           dclStat;
    FSSTRESSDBHEADER    dbHdr;
    unsigned            nRecordNum;
    D_UINT32            ulDataSize;
    D_UINT32            ulRecordOffset;
    D_UINT32            ulTransferred;
    DCLTIMESTAMP        ts;
    FSSTRESSTHREADINFO *pTI = pTC;

    DclAssertWritePtr(pTC, sizeof(*pTC));
    DclAssert(hFile);

    /*  Set the current offset into the file to the database index
    */
    dclStat = PRIMSEEK(hFile, 0, DCLFSFILESEEK_SET);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("DBWriteRecordRandom() failed at line %u, status=%lX\n", __LINE__, dclStat);
        goto EXIT;
    }

    /*  Read in the database index
    */
    dclStat = PRIMREAD(hFile, &dbHdr, sizeof(FSSTRESSDBHEADER), &ulTransferred);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("DBWriteRecordRandom() failed at line %u, status=%lX\n", __LINE__, dclStat);
        goto EXIT;
    }

    if(ulTransferred != sizeof(FSSTRESSDBHEADER))
    {
        dclStat = DCLSTAT_CURRENTLINE;
        DclPrintf("DBWriteRecordRandom() failed at line %u, status=%lX\n", __LINE__, dclStat);
        goto EXIT;
    }

    dclStat = DBFindFreeRecord(pTC, &dbHdr, &nRecordNum);
    if(dclStat != DCLSTAT_SUCCESS)
        goto EXIT;

    /*  Compute a data size that's within the max allowed size
    */
    ulDataSize = (D_UINT32)(DclRand64(&pTC->ullSeed) % pTC->pTI->ulDBRecordSize);

    /*  Compute the offset for the record
    */
    ulRecordOffset = dbHdr.ulOffsetFirstRecord + (nRecordNum * dbHdr.ulFullRecordSize);

    ts = DclTimeStamp();

    /*  Write out the new record
    */
    dclStat = DBWriteRecord(pTC, hFile, nRecordNum, ulRecordOffset, ulDataSize, &pTC->ullSeed);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        D_UINT32 ulElapsedUS = DclTimePassedUS(ts);
        
        if(DclCriticalSectionEnter(&pTC->ulAtomicWriteGate, 2000, TRUE))
        {
            /*  Should never wrap since D_UINT32_MAX microseconds is more 
                than an hour, and we should be sampling much more often
                than that.
            */                    
            DclAssert(D_UINT32_MAX - pTC->ulWriteUS >= ulElapsedUS);

            pTC->ulWrites++;            
            pTC->ulWriteBytes += ulDataSize;
            pTC->ulWriteUS += ulElapsedUS;
            if(pTC->ulWriteMaxUS < ulElapsedUS)
                pTC->ulWriteMaxUS = ulElapsedUS;

            DclCriticalSectionLeave(&pTC->ulAtomicWriteGate);
        }
        else
        {
            DclPrintf("Error entering critical section while gathering write DB stats -- ignoring...\n");
        }
    }

    if(pTC->pTI->sti.nVerbosity > DCL_VERBOSE_LOUD)
    {
        DclPrintf("DB Thread %u, record %u, write of %lU bytes, returned status %lX\n",
            pTC->nThreadNum, nRecordNum, ulDataSize, dclStat);
    }

    /*  Release this record
    */
    pTC->pTI->pfDBRecordLocked[nRecordNum] = FALSE;

  EXIT:

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: DBFindFreeRecord()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DBFindFreeRecord(
    FSSTRESSTHREADINFO *pTC,
    FSSTRESSDBHEADER   *pDBHdr,
    unsigned           *pnRecordNum)
{
    DCLSTATUS           dclStat;
    unsigned            nRecordNum;

    DclAssertWritePtr(pTC, sizeof(*pTC));
    DclAssertWritePtr(pnRecordNum, sizeof(*pnRecordNum));
    
    /*  Get the mutex protecting the array of busy records
    */
    if(!DclMutexAcquire(pTC->pTI->pDBMutex))
    {
        dclStat = DCLSTAT_MUTEXACQUIREFAILED;
        DclPrintf("DBWriteRecordRandom() failed at line %u, status=%lX\n", __LINE__, dclStat);
        return dclStat;
    }

    /*  Compute a record number that's within bounds of the number
        specified in the index.
    */
    nRecordNum = (unsigned)(DclRand64(&pTC->ullSeed) % pDBHdr->ulNumRecords);

    /*  Loop until we have a non-busy record.
    */
    while(TRUE)
    {
        /*  Check if this record is available
        */
        if(!pTC->pTI->pfDBRecordLocked[nRecordNum])
        {
            pTC->pTI->pfDBRecordLocked[nRecordNum] = TRUE;
            break;
        }

        nRecordNum++;
        if(nRecordNum >= pDBHdr->ulNumRecords)
            nRecordNum = 0;
    }

    /*  We're done with the array
    */
    if(!DclMutexRelease(pTC->pTI->pDBMutex))
    {
        dclStat = DCLSTAT_MUTEXRELEASEFAILED;
        DclPrintf("DBWriteRecordRandom() failed at line %u, status=%lX\n", __LINE__, dclStat);
        return dclStat;
    }

    *pnRecordNum = nRecordNum;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: DBInit()

    Creates the initial database file used by the database test threads.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DBInit(
    FSSTRESSTESTINFO   *pTI)
{
    char                szFullPathName[MAX_FILESPEC_LEN];
    DCLFSFILEHANDLE     hFile = NULL;
    DCLSTATUS           dclStat;
    FSSTRESSDBHEADER    dbHdr;
    D_UINT32            ulCurrentOffset;
    D_UINT32            ulTransferred;
    unsigned            nRecordNum;

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssert(pTI->nDBThreads);

    /*  Allocate storage for the record locks used by writes to avoid
        stepping on each other while updating a record.
    */
    pTI->pfDBRecordLocked = DclMemAllocZero(sizeof(*pTI->pfDBRecordLocked) * pTI->nDBRecords);
    if(!pTI->pfDBRecordLocked)
        return DCLSTAT_MEMALLOCFAILED;

    /*  Create the initial file
    */
    DclSNPrintf(szFullPathName, MAX_FILESPEC_LEN, "%sFSSTRESS_THREAD_%08lX_DB.dat",
        pTI->szPath, DclOsThreadID());

    dclStat = DclFsFileOpen(szFullPathName, "w+b", &hFile);
    if(dclStat != DCLSTAT_SUCCESS)
        goto Cleanup;

    /*  create the index record in the database
    */
    dbHdr.ulNumRecords = pTI->nDBRecords;
    dbHdr.ulFullRecordSize = sizeof(FSSTRESSRECHEADER) + pTI->ulDBRecordSize;
    dbHdr.ulOffsetFirstRecord = sizeof(dbHdr);
    dclStat = DclFsFileWrite(hFile, &dbHdr, sizeof(dbHdr), &ulTransferred);
    if(dclStat != DCLSTAT_SUCCESS)
        goto Cleanup;

    if(ulTransferred != sizeof(dbHdr))
    {
        dclStat = DCLSTAT_CURRENTLINE;
        goto Cleanup;
    }

    /*  set up our current offset
    */
    ulCurrentOffset = sizeof(dbHdr);

    /*  create the initial records for the database
    */
    for(nRecordNum = 0; nRecordNum < pTI->nDBRecords; nRecordNum++)
    {
        /*  write out a starting record
        */
        dclStat = DBWriteRecord(NULL, hFile, nRecordNum, ulCurrentOffset,
                                pTI->ulDBRecordSize, &pTI->sti.ullRandomSeed);
        if(dclStat != DCLSTAT_SUCCESS)
            goto Cleanup;

        /*  move the current offset
        */
        ulCurrentOffset += dbHdr.ulFullRecordSize;
    }

  Cleanup:

    if(hFile)
    {
        /*  Return any original status code first...
        */
        if(dclStat == DCLSTAT_SUCCESS)
            dclStat = DclFsFileClose(hFile);
        else
            DclFsFileClose(hFile);
    }

    if(dclStat != DCLSTAT_SUCCESS)
    {
        /*  Attempt to delete the file if it was created, but preserve
            the original error code.
        */
        if(hFile)
            DclFsFileDelete(szFullPathName);

        if(pTI->pfDBRecordLocked)
            DclMemFree(pTI->pfDBRecordLocked);
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: DBDestroy()

    Destroys the database that we created.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DBDestroy(
    FSSTRESSTESTINFO   *pTI)
{
    char                szFullPathName[MAX_FILESPEC_LEN];
    DCLSTATUS           dclStat;

    DclAssertWritePtr(pTI, sizeof(*pTI));

    /*  create the initial file
    */
    DclSNPrintf(szFullPathName, MAX_FILESPEC_LEN, "%sFSSTRESS_THREAD_%08lX_DB.dat",
        pTI->szPath, DclOsThreadID());

    dclStat = DclFsFileDelete(szFullPathName);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    DclAssert(pTI->pfDBRecordLocked);
    DclMemFree(pTI->pfDBRecordLocked);

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: DBThread()

    Takes a FSSTRESSTHREADINFO and runs database-style operations for
    100*ulIterations.  On any failure, the thread will assert and exit.
    These threads are designed to test contention and data integrity on
    a single file and larger reads/writes.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void *DBThread(
    void               *pParam)
{
    FSSTRESSTHREADINFO *pTC = (FSSTRESSTHREADINFO*)pParam;
    DCLFSFILEHANDLE     hFile = NULL;
    D_UINT32            ulIteration;
    D_UINT32            ulDatabaseOp;
    DCLSTATUS           dclStat;
    char                szFullPathName[MAX_FILESPEC_LEN];
    FSSTRESSTHREADINFO *pTI = pTC;      /*  So the PRIM macros work properly */

    DclAssertWritePtr(pTC, sizeof(*pTC));
    DclAssert(pTC->ThreadExecState == EXEC_INITIALIZING);

    pTC->tid = DclOsThreadID();

    /*  Each thread must register as a file user, or it cannot use
        the file system.
    */
    pTC->dclStat = DclFsBecomeFileUser();
    if(pTC->dclStat != DCLSTAT_SUCCESS)
    {
        DclError();
        goto EXIT;
    }

    /*  Open the database file
    */
    DclSNPrintf(szFullPathName, MAX_FILESPEC_LEN, "%sFSSTRESS_THREAD_%08lX_DB.dat",
        pTC->pTI->szPath, pTC->pTI->tidMaster);

    pTC->dclStat = PRIMOPEN(szFullPathName, "r+b", &hFile);
    if(pTC->dclStat != DCLSTAT_SUCCESS)
    {
        DclError();
        goto EXIT;
    }

    /*  Done initializing, go into the waiting state until the
        master test state is done initializing.
    */
    pTC->ThreadExecState = EXEC_WAITING;

    while(pTC->pTI->TestExecState == EXEC_INITIALIZING)
        DclOsSleep(1);

    pTC->ThreadExecState = EXEC_RUNNING;

    /*  loop over the specified number of iterations
    */
    for(ulIteration = 0; ulIteration <  pTC->pTI->ulIterations; ulIteration++)
    {
        /*  Break out early if the master test state is no longer "running"
        */
        if(pTC->pTI->TestExecState != EXEC_RUNNING)
            break;

        for(ulDatabaseOp = 0;
            ulDatabaseOp < pTC->pTI->nDBOpsPerIteration;
            ulDatabaseOp++)
        {
            D_BOOL  fWriting = FALSE;

            /*  Break out early if the master test state is no longer "running"
            */
            if(pTC->pTI->TestExecState != EXEC_RUNNING)
                break;

            if(pTC->pTI->iDBRWRatio == 0)
            {
                /*  If the ratio is zero then we are writing only,
                    and never reading.
                */
                fWriting = TRUE;
            }
            else if(pTC->pTI->iDBRWRatio == -1)
            {
                /*  If the ratio is -1 then we are reading only,
                    and never writing.
                */
                fWriting = FALSE;
            }
            else if(pTC->pTI->iDBRWRatio > 0)
            {
                if((DclRand64(&pTC->ullSeed) % (pTC->pTI->iDBRWRatio+1)) == 0)
                    fWriting = TRUE;
                else
                    fWriting = FALSE;
            }
            else
            {
                /*  If the ratio is negative then we are doing n writes for
                    every 1 read.
                */
                if((DclRand64(&pTC->ullSeed) % ((-pTC->pTI->iDBRWRatio)+1)) == 0)
                    fWriting = FALSE;
                else
                    fWriting = TRUE;
            }

            if(fWriting)
            {
                /*  Perform a write of random length to a random record
                */
                pTC->dclStat = DBWriteRandomRecord(pTC, hFile);
                if(pTC->dclStat != DCLSTAT_SUCCESS)
                {
                    goto EXIT;
                }

                ThreadPacing(pTC, ulIteration, PACEPOINT_DB_WRITECOMPLETE);
            }
            else
            {
                /*  Perform a read and verify of a random record
                */
                pTC->dclStat = DBReadRandomRecord(pTC, hFile);
                if(pTC->dclStat != DCLSTAT_SUCCESS)
                {
                    goto EXIT;
                }

                ThreadPacing(pTC, ulIteration, PACEPOINT_DB_READCOMPLETE);
            }
        }

        DclOsAtomic32Increment(&pTC->ulAtomicIterationsCompleted);

        ThreadPacing(pTC, ulIteration, PACEPOINT_DB_ITERCOMPLETE);
    }

  EXIT:

    pTC->ThreadExecState = EXEC_QUITTING;

    /*  close our handle to the file (preserve any original error code)
    */
    if(hFile)
    {
        dclStat = PRIMCLOSE(hFile);
        if(pTC->dclStat == DCLSTAT_SUCCESS)
           pTC->dclStat = dclStat;
    }

    /*  deregister as a file user (preserve any original error code)
    */
    dclStat = DclFsReleaseFileUser();
    if(pTC->dclStat == DCLSTAT_SUCCESS)
       pTC->dclStat = dclStat;

    /*  The "exiting" state must only be set as THE VERY LAST THING
        before the thread exits.
    */
    pTC->ThreadExecState = EXEC_EXITING;

    return NULL;
}


/*-------------------------------------------------------------------
    Local: CaseThread()

    Takes a FSSTRESSTHREADINFO and runs test cases for the specified
    number of iterations.  On each iteration, it runs a permutation
    of all the available test cases, using the passed in seed to
    randomize things.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void *CaseThread(
    void               *pParam)
{
    FSSTRESSTHREADINFO *pTC = (FSSTRESSTHREADINFO*)pParam;
    unsigned           *pnTestIndices;
    D_BOOL              fFailed = FALSE;
    D_UINT32            ulCurrentIteration;
    unsigned            nTest;
    unsigned            nTestToRun;
    DCLSTATUS           dclStat;

    DclAssertWritePtr(pTC, sizeof(*pTC));
    DclAssert(pTC->ThreadExecState == EXEC_INITIALIZING);

    /*  get the ID for the current thread, which is passed to the test cases to permit
        creating unique names
    */
    pTC->tid = DclOsThreadID();

    /*  allocate storage for our thread context structures
    */
    pnTestIndices = DclMemAlloc(pTC->pTI->nNumTestCases * sizeof(*pnTestIndices));
    if(!pnTestIndices)
        goto EXIT;

    /*  each thread must register as a file user, or it cannot use the file system
    */
    pTC->dclStat = DclFsBecomeFileUser();
    if(pTC->dclStat != DCLSTAT_SUCCESS)
        goto EXIT;

    /*  Done initializing, go into the waiting state until the
        master test state is done initializing.
    */
    pTC->ThreadExecState = EXEC_WAITING;

    while(pTC->pTI->TestExecState == EXEC_INITIALIZING)
        DclOsSleep(1);

    pTC->ThreadExecState = EXEC_RUNNING;

    /*  loop over the requested number of iterations
    */
    for(ulCurrentIteration = 0; ulCurrentIteration < pTC->pTI->ulIterations; ulCurrentIteration++)
    {
        /*  Break out early if the master test state is no longer "running"
        */
        if(pTC->pTI->TestExecState != EXEC_RUNNING)
            break;

        /*  Create a shuffled array of indices to determine the order
            we'll run the tests.
        */
        ShuffleCases(pnTestIndices, pTC->pTI->nNumTestCases, &pTC->ullSeed);

        /*  Run through our array, running each test suite as we go
        */
        for(nTest = 0; nTest < pTC->pTI->nNumTestCases; nTest++)
        {
            char                szBaseName[MAX_FILESPEC_LEN];
            DLTFSSTRESSCASEINFO CaseInfo;
            DCLTIMESTAMP        ts;
            D_UINT32            ulElapsedUS;

            /*  Break out early if the master test state is no longer "running"
            */
            if(pTC->pTI->TestExecState != EXEC_RUNNING)
                break;

            nTestToRun = pnTestIndices[nTest];

            ThreadPacing(pTC, ulCurrentIteration, PACEPOINT_CASE_BEGIN);

            CaseInfo.pTC          = pTC;
            CaseInfo.pszTestName  = aFSStressTestCases[nTestToRun].pszTestName;
            CaseInfo.pszBasePath  = pTC->pTI->szPath;
            CaseInfo.pszBaseName  = szBaseName;
            CaseInfo.nThreadNum   = pTC->nThreadNum;
            CaseInfo.nCaseNum     = nTest;
            CaseInfo.fSupportsCWD = pTC->pTI->fSupportsCWD;

            DclSNPrintf(szBaseName, sizeof(szBaseName), "%sFSSTRESS_THREAD_%03U_%s",
                pTC->pTI->szPath, pTC->nThreadNum, CaseInfo.pszTestName);

            /*  run the init for the test
            */
            pTC->dclStat = aFSStressTestCases[nTestToRun].pfnInit(&CaseInfo);
            if(pTC->dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("CaseThread:  Init failed for section \"%s\", dclStat=%lX\n",
                        aFSStressTestCases[nTestToRun].pszTestName, pTC->dclStat);

                fFailed = TRUE;
                break;
            }

            ThreadPacing(pTC, ulCurrentIteration, PACEPOINT_CASE_RUN);

            /*  run the body of the test
            */
            ts = DclTimeStamp();
            
            pTC->dclStat = aFSStressTestCases[nTestToRun].pfnTest(&CaseInfo);
            if(pTC->dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("CaseThread:  Body failed for section \"%s\", dclStat=%lX\n",
                        aFSStressTestCases[nTestToRun].pszTestName, pTC->dclStat);

                fFailed = TRUE;
                break;
            }

            ulElapsedUS = DclTimePassedUS(ts);

            if(DclCriticalSectionEnter(&aFSStressTestCases[nTestToRun].ulAtomicGate, 2000, TRUE))
            {
                /*  Should never wrap since D_UINT32_MAX microseconds is more 
                    than an hour, and we should be sampling much more often
                    than that.
                */                    
                DclAssert(D_UINT32_MAX - aFSStressTestCases[nTestToRun].ullSampleTotalUS >= ulElapsedUS);
                
                aFSStressTestCases[nTestToRun].ullSampleTotalUS += ulElapsedUS;
                aFSStressTestCases[nTestToRun].nSampleOperations++;
                if(aFSStressTestCases[nTestToRun].ulSampleMaxUS < ulElapsedUS)
                    aFSStressTestCases[nTestToRun].ulSampleMaxUS = ulElapsedUS;

                DclCriticalSectionLeave(&aFSStressTestCases[nTestToRun].ulAtomicGate);
            }
            else
            {
                DclPrintf("Error entering critical section while gathering stats -- ignoring...\n");
            }

            ThreadPacing(pTC, ulCurrentIteration, PACEPOINT_CASE_END);

            /*  run the cleanup
            */
            pTC->dclStat = aFSStressTestCases[nTestToRun].pfnCleanup(&CaseInfo);
            if(pTC->dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("CaseThread:  Cleanup failed for section \"%s\", dclStat=%lX\n",
                        aFSStressTestCases[nTestToRun].pszTestName, pTC->dclStat);

                fFailed = TRUE;
                break;
            }
        }

        if(fFailed)
            break;

        DclOsAtomic32Increment(&pTC->ulAtomicIterationsCompleted);

        ThreadPacing(pTC, ulCurrentIteration, PACEPOINT_CASE_ITERCOMPLETE);
    }

  EXIT:

    pTC->ThreadExecState = EXEC_QUITTING;

    /*  Deregister as a file user, but don't trash the status code for
        the thread.
    */
    dclStat = DclFsReleaseFileUser();
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclError();
    }

    if(pnTestIndices)
        DclMemFree(pnTestIndices);

    /*  The "exiting" state must only be set as THE VERY LAST THING
        before the thread exits.
    */
    pTC->ThreadExecState = EXEC_EXITING;

    return NULL;
}


/*-------------------------------------------------------------------
    Local: FSIOThread()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void *FSIOThread(
    void               *pParam)
{
    FSSTRESSTHREADINFO *pTC = (FSSTRESSTHREADINFO*)pParam;
    D_UINT32            ulIteration;
    DCLSTATUS           dclStat;
 
    DclAssertWritePtr(pTC, sizeof(*pTC));
    DclAssert(pTC->ThreadExecState == EXEC_INITIALIZING);

    pTC->tid = DclOsThreadID();

    /*  Each thread must register as a file user, or it cannot use
        the file system.
    */
    pTC->dclStat = DclFsBecomeFileUser();
    if(pTC->dclStat != DCLSTAT_SUCCESS)
    {
        DclError();
        goto EXIT;
    }

    /*  Done initializing, go into the waiting state until the
        master test state is done initializing.
    */
    pTC->ThreadExecState = EXEC_WAITING;

    while(pTC->pTI->TestExecState == EXEC_INITIALIZING)
        DclOsSleep(1);

    pTC->ThreadExecState = EXEC_RUNNING;

    /*  loop over the specified number of iterations
    */
    for(ulIteration = 0; ulIteration <  pTC->pTI->ulIterations; ulIteration++)
    {
        DCLTOOLPARAMS   dtp = {0};
        char            szCmdLine[256];  
        unsigned        nVerbosity = pTC->pTI->sti.nVerbosity;

        /*  Run FSIOTEST and one verbosity level lower than that which 
            FSSTRESSTEST is being run.
        */            
        if(nVerbosity >= 1)
            nVerbosity--;
        
        DclSNPrintf(szCmdLine, sizeof(szCmdLine), "/dlperf /frag- /SEED:%lU /V:%u /p:'%s' /FSBlockSize:%lU /FSDevName:%s", 
            (pTC->nThreadNum * 100000) + (ulIteration * 100), 
            nVerbosity,
            pTC->pTI->szPath,
            pTC->pTI->ulFSBlockSize,
            pTC->pTI->statfs.szDeviceName);

        /*  Break out early if the master test state is no longer "running"
        */
        if(pTC->pTI->TestExecState != EXEC_RUNNING)
            break;

        dtp.hDclInst = pTC->pTI->sti.hDclInst;
        dtp.pszCmdName = "FSSTRESS:FSIO";
        dtp.pszCmdLine = szCmdLine;

        dclStat = DclTestFSIOMain(&dtp);

        DclOsAtomic32Increment(&pTC->ulAtomicIterationsCompleted);
    }

  EXIT:

    pTC->ThreadExecState = EXEC_QUITTING;

    /*  deregister as a file user (preserve any original error code)
    */
    dclStat = DclFsReleaseFileUser();
    if(pTC->dclStat == DCLSTAT_SUCCESS)
       pTC->dclStat = dclStat;

    /*  The "exiting" state must only be set as THE VERY LAST THING
        before the thread exits.
    */
    pTC->ThreadExecState = EXEC_EXITING;

    return NULL;
}


/*-------------------------------------------------------------------
    Local: StreamThread()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void *StreamThread(
    void               *pParam)
{
    FSSTRESSTHREADINFO *pTC = (FSSTRESSTHREADINFO*)pParam;
    D_UINT32            ulIteration;
    DCLSTATUS           dclStat;
    STREAMTHREADINFO   *pStreamThread;
    STREAMFILEINFO     *pStreamFile;
    DCLFSFILEHANDLE     hFile = NULL;
    FSSTRESSTHREADINFO *pTI = pTC;      /*  So the PRIM macros work properly */
    D_UINT32           *pulSamples = NULL;
 
    DclAssertWritePtr(pTC, sizeof(*pTC));
    DclAssert(pTC->ThreadExecState == EXEC_INITIALIZING);

    pTC->tid = DclOsThreadID();

    pStreamThread = &pTC->pTI->aStrT[pTC->nThreadNumByType];
    pStreamFile = &pTC->pTI->aStrF[pStreamThread->nFileNum];

    pulSamples = DclMemAlloc(pStreamThread->ulFrames * sizeof(*pulSamples));
    if(!pulSamples)
    {
        pTC->dclStat = DCLSTAT_OUTOFMEMORY;
        goto Cleanup;
    }

    /*  Each thread must register as a file user, or it cannot use
        the file system.
    */
    pTC->dclStat = DclFsBecomeFileUser();
    if(pTC->dclStat != DCLSTAT_SUCCESS)
    {
        DclError();
        goto Cleanup;
    }

    /*  Done initializing, go into the waiting state until the
        master test state is done initializing.
    */
    pTC->ThreadExecState = EXEC_WAITING;

    while(pTC->pTI->TestExecState == EXEC_INITIALIZING)
        DclOsSleep(1);

    /*  Do this before going into the "RUNNING" state, so ensure that
        there are not attempts to delete the file before we have even
        entered this code.
    */        
    DclOsAtomic32Increment(&pStreamFile->ulAtomicUsageCount);

    pTC->ThreadExecState = EXEC_RUNNING;

    pTC->dclStat = PRIMOPEN(pStreamFile->szName, pStreamThread->fWriting ? "r+" : "r", &hFile);
    if(pTC->dclStat != DCLSTAT_SUCCESS)
        goto EXIT;

    for(ulIteration = 0; ulIteration <  pTC->pTI->ulIterations; ulIteration++)
    {
        D_UINT32        nn;
        D_UINT64        ullPos = 0;
        D_UINT32        ulSample;
        D_UINT32        ulTotalUS = 0;
        D_UINT32        ulMaxUS = 0;
        DCLTIMESTAMP    ts;
        D_UINT32        ulMeanUS;
        unsigned        n10thPercent;

        DclMemSet(pulSamples, 0, pStreamThread->ulFrames * sizeof(*pulSamples));

        for(ulSample = 0; ulSample < pStreamThread->ulFrames; ulSample++)
        {
            D_UINT32        ulTransferred;
            D_UINT32        ulElapsedUS;

            if(pTC->pTI->TestExecState != EXEC_RUNNING)
                goto EXIT;

            if(pStreamThread->fWriting)
            {
                RandomizeBuffer(pStreamThread->pIOBuff, pStreamThread->ulFrameSize, &pTC->ullSeed);

                ts = DclTimeStamp();
                
                pTC->dclStat = PRIMWRITE(hFile, pStreamThread->pIOBuff, pStreamThread->ulFrameSize, &ulTransferred);
                if((pTC->dclStat != DCLSTAT_SUCCESS) || (ulTransferred != pStreamThread->ulFrameSize))
                {
                    /*  Ensure any original error code is preserved
                    */
                    if(pTC->dclStat == DCLSTAT_SUCCESS)
                        pTC->dclStat = DCLSTAT_FS_WRITEFAILED;

                    goto EXIT;
                } 
            }
            else
            {
                ts = DclTimeStamp();
                
                pTC->dclStat = PRIMREAD(hFile, pStreamThread->pIOBuff, pStreamThread->ulFrameSize, &ulTransferred);
                if((pTC->dclStat != DCLSTAT_SUCCESS) || (ulTransferred != pStreamThread->ulFrameSize))
                {
                    /*  Ensure any original error code is preserved
                    */
                    if(pTC->dclStat == DCLSTAT_SUCCESS)
                        pTC->dclStat = DCLSTAT_FS_READFAILED;

                    goto EXIT;
                }            
            }

            ulElapsedUS = DclTimePassedUS(ts);

            *(pulSamples + ulSample) = ulElapsedUS;
            ulTotalUS += ulElapsedUS;
            if(ulMaxUS < ulElapsedUS)
                ulMaxUS = ulElapsedUS;

            ullPos += ulTransferred;

            ThreadPacing(pTC, ulIteration, PACEPOINT_STREAM_IOCOMPLETE);
        }

        if(pStreamThread->fWriting)
        {
            ts = DclTimeStamp();
            
            pTC->dclStat = PRIMFLUSH(hFile);
            if(pTC->dclStat != DCLSTAT_SUCCESS)
                break;

            ulTotalUS += DclTimePassedUS(ts);
        }

        DclSortInsertion32(pulSamples, pStreamThread->ulFrames);

        ulMeanUS = pulSamples[pStreamThread->ulFrames / 2];

        if(!(pStreamThread->ulFrames & 1))
            ulMeanUS = (ulMeanUS + pulSamples[(pStreamThread->ulFrames / 2) + 1]) / 2;

        n10thPercent = (pStreamThread->ulFrames * 100) / 1000;

        if(pTC->pTI->sti.nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
        {
            for(nn = 0; nn < pStreamThread->ulFrames; nn++)
                DclPrintf("%4lU ", pulSamples[nn]);
    
            DclPrintf("\n");
        }
        
        /*  Any given stream thread is ONLY reading or writing.  Therefore,
            we always use the READER critical section gate, and accumulate
            stats in the reader fields.  The high level code displaying the
            stats know whether they are <really> writer stats instead.
        */            
        if(DclCriticalSectionEnter(&pTC->ulAtomicReadGate, 2000, TRUE))
        {
            D_UINT32    aulTemp[STREAM_RESULT_COUNT];

            DclAssert(DCLDIMENSIONOF(aulTemp) == DCLDIMENSIONOF(pStreamThread->aulSampleResults));
            DclAssert(ullPos <= D_UINT32_MAX);

            pTC->ulReads     += pStreamThread->ulFrames;            
            pTC->ulReadBytes += (D_UINT32)ullPos;
            pTC->ulReadUS    += ulTotalUS;
            if(pTC->ulReadMaxUS < ulMaxUS)
                pTC->ulReadMaxUS = ulMaxUS;
           
            for(nn = 0; nn < DCLDIMENSIONOF(pStreamThread->aulSampleResults); nn++)
            {
                aulTemp[nn] = pulSamples[pStreamThread->ulFrames - 1 - (n10thPercent * nn / 10)];
                pStreamThread->aulSampleResults[nn] += aulTemp[nn];
            }
            
            pStreamThread->ulSampleCount++;
            pStreamThread->ullSampleUS += ulTotalUS;

            DclCriticalSectionLeave(&pTC->ulAtomicReadGate);

            if(pTC->pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
            {
                DclAssert(DCLDIMENSIONOF(aulTemp) == 11);

                DclPrintf("%s %2u Iter %2lU: %5lU %5lU %5lU %5lU %5lU %5lU %5lU %5lU %5lU %5lU %5lU A=%4lU M=%4lU\n", 
                    pStreamThread->fWriting ? "Writer" : "Reader",
                    pTC->nThreadNumByType,
                    pStreamThread->ulSampleCount,
                    1000000 / aulTemp[0], 1000000 / aulTemp[1], 1000000 / aulTemp[2], 
                    1000000 / aulTemp[3], 1000000 / aulTemp[4], 1000000 / aulTemp[5], 
                    1000000 / aulTemp[6], 1000000 / aulTemp[7], 1000000 / aulTemp[8], 
                    1000000 / aulTemp[9], 1000000 / aulTemp[10], 
                    ulTotalUS / pStreamThread->ulFrames,
                    ulMeanUS);
            }
        }
        else
        {
            DclPrintf("Error entering critical section while gathering stream stats -- ignoring...\n");
        }

        DclOsAtomic32Increment(&pTC->ulAtomicIterationsCompleted);
        
        /*  Reset back to the start of the file and do it all again
        */
        pTC->dclStat = PRIMSEEK(hFile, 0, DCLFSFILESEEK_SET);
        if(pTC->dclStat != DCLSTAT_SUCCESS)
            break;

        ThreadPacing(pTC, ulIteration, PACEPOINT_STREAM_ITERCOMPLETE);
    }

  EXIT:

    pTC->ThreadExecState = EXEC_QUITTING;
    
    if(hFile)
        PRIMCLOSE(hFile);

    if(DclOsAtomic32Decrement(&pStreamFile->ulAtomicUsageCount) == 0)
        PRIMDELETE(pStreamFile->szName);

    /*  Deregister as a file user (preserve any original error code)
    */
    dclStat = DclFsReleaseFileUser();
    if(pTC->dclStat == DCLSTAT_SUCCESS)
       pTC->dclStat = dclStat;

  Cleanup:

    if(pulSamples)
    {
        dclStat = DclMemFree(pulSamples);
        if(pTC->dclStat == DCLSTAT_SUCCESS)
           pTC->dclStat = dclStat;
    }
    
    /*  The "exiting" state must only be set as THE VERY LAST THING
        before the thread exits.
    */
    pTC->ThreadExecState = EXEC_EXITING;

    return NULL;
}


/*-------------------------------------------------------------------
    Local: ThreadPacing()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void ThreadPacing(
    FSSTRESSTHREADINFO *pTC,
    D_UINT32            ulCurrentIteration,
    PACEPOINT           nPacingPoint)
{
    unsigned            tt;
    unsigned            nSleep = pTC->pTI->nPacingMS;
    D_UINT32            ulThreshold = ulCurrentIteration - (ulCurrentIteration >> 3);

    DclAssertWritePtr(pTC, sizeof(*pTC));
    DclAssert(nPacingPoint > PACEPOINT_LOWLIMIT && nPacingPoint < PACEPOINT_HIGHLIMIT);

    /*  Skip if pacing is disabled
    */
    if(!nSleep)
        return;

    /*  If we have not completed enough iterations, then there is
        nothing to do.
    */
    if(!ulThreshold)
        return;

    /*  We only check for a certain percentage of the time, which may
        be different for each interruption point.
    */
    if(DclRand64(&pTC->ullSeed) % 100 >= anRatio[nPacingPoint])
        return;

    /*  If any thread is below the calculated threshold, then delay
        the current thread for the current value of nSleep.

        Note that we only loop through the threads once per each
        pacing point, so we depend on the cumulative effect of all
        the pacing points to get us what we want.
    */
    for(tt=0; tt < pTC->pTI->nTotalThreads; tt++)
    {
        if(DclOsAtomic32Retrieve(&pTC->pTI->paTC[tt].ulAtomicIterationsCompleted) < ulThreshold)
        {
            unsigned    nThisSleep = (unsigned)(DclRand64(&pTC->ullSeed) % (nSleep * anRatio[nPacingPoint]));

            nThisSleep = DCLMIN(nThisSleep, 500);
            nThisSleep = DCLMAX(nThisSleep, 1);
            
            DCLPRINTF(2, ("Pacing: Thread %u Iter %lU -- Point=%u, Thresh=%lU, Ratio=%u Sleep=%u ThisSleep=%u\n",
                tt, 
                DclOsAtomic32Retrieve(&pTC->pTI->paTC[tt].ulAtomicIterationsCompleted),
                nPacingPoint, ulThreshold, anRatio[nPacingPoint], nSleep, nThisSleep));
            
            DclOsSleep(nThisSleep);

            /*  Keep stats on how much pacing is happening
            */
            pTC->anPacingSleeps[nPacingPoint]++;
            pTC->anPacingSleepMS[nPacingPoint] += nThisSleep;

            /*  Double the sleep amount if there are subsequent threads
            */
            nSleep <<= 1;
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Local: ShuffleCases()

    Populates the provided array with numbers from 0 to n, randomly
    arranged.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void ShuffleCases(
    unsigned   *pnIndices,
    unsigned    nNumIndices,
    D_UINT64   *pullSeed)
{
    unsigned    ii;
    unsigned    nTemp;
    D_UINT32    ulRand;

    DclAssert(pnIndices);
    DclAssert(nNumIndices);
    DclAssertWritePtr(pullSeed, sizeof(*pullSeed));

    /*  fill out the array with 0 through n
    */
    for (ii = 0; ii < nNumIndices; ii++)
    {
        pnIndices[ii] = ii;
    }

    /*  loop back over the array, shuffling as we go
    */
    for (ii = 0; ii < nNumIndices; ii++)
    {
        ulRand = (D_UINT32)(DclRand64(pullSeed) % (nNumIndices - ii));
        nTemp = pnIndices[ii];
        pnIndices[ii] = pnIndices[ii + ulRand];
        pnIndices[ii + ulRand] = nTemp;
    }
}


/*-------------------------------------------------------------------
    Local: ProcessParams()

    This function parses the command line items and configures
    the test as specified.

    Parameters:
        pTI        - A pointer to the FSIOTESTINFO structure to use
        pszCmdLine - The test command line

    Return Value:
        Returns a status code indicating the results.  In the event
        that an error status is returned any necessary error messages
        or help display will already have been displayed.
-------------------------------------------------------------------*/
static DCLSTATUS ProcessParams(
    FSSTRESSTESTINFO   *pTI,
    const char         *pszCmdName,
    const char         *pszCmdLine)
{
    #define             ARGBUFFLEN  (128)
    D_UINT16            uIndex;
    D_UINT16            uSwitch;
    D_UINT16            uArgCount;
    char                achArgBuff[ARGBUFFLEN];
    char                szFSDevName[DCL_MAX_DEVICENAMELEN] = {0};
    unsigned            nLen;

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertReadPtr(pszCmdName, 0);
    DclAssertReadPtr(pszCmdLine, 0);

    /*  Init default settings
    */
    pTI->sti.nVerbosity     = DCL_VERBOSE_NORMAL;
    pTI->sti.ulTestSeconds  = DEFAULT_TEST_SECONDS;
    pTI->nStreamThreads     = DEFAULT_STREAM_THREADS;
    pTI->nFSIOThreads       = DEFAULT_FSIO_THREADS;
    pTI->nCaseThreads       = DEFAULT_CASE_THREADS;
    pTI->nDBThreads         = DEFAULT_DB_THREADS;
    pTI->ulIterations       = DEFAULT_ITERATIONS;
    pTI->nPacingMS          = DEFAULT_PACING_MS;
    pTI->nSampleSecs        = DEFAULT_SAMPLE_SECONDS;
    pTI->iDBRWRatio         = DEFAULT_DB_READWRITE_RATIO;
    pTI->nDBOpsPerIteration = DEFAULT_DB_OPSPERITERATION;
    pTI->nDBRecords         = DEFAULT_DB_RECORDS;

    uArgCount = DclArgCount(pszCmdLine);

    /*  Start with argument number 1
    */
    for(uIndex = 1; uIndex <= uArgCount; uIndex++)
    {
        /*  Initialize this to 1 so that the error handling code at the
            label "BadOption" will work properly for this set of options.
        */
        uSwitch = 1;

        if(!DclArgRetrieve(pszCmdLine, uIndex, sizeof(achArgBuff), achArgBuff))
        {
            DclPrintf("Bad argument!\n\n");
            ShowHelp(pTI->sti.hDclInst, pszCmdName);
            return DCLSTAT_BADPARAMETER;
        }

                /*-----------------------------------------*\
                 *                                         *
                 *    Process main test type args first    *
                 *                                         *
                \*-----------------------------------------*/

        if(DclStrNICmp(achArgBuff, "/CASE:", 6) == 0)
        {
            pTI->nCaseThreads = (unsigned)DclAtoL(&achArgBuff[6]);
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/DB:", 4) == 0)
        {
            pTI->nDBThreads = (unsigned)DclAtoL(&achArgBuff[4]);
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/FSIO:", 6) == 0)
        {
            pTI->nFSIOThreads = (unsigned)DclAtoL(&achArgBuff[6]);
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/STREAM:", 8) == 0)
        {
            if(!ProcessStreamArgs(pTI, &achArgBuff[8]))
                goto BadOption;
            
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/DBRATIO:", 9) == 0)
        {
            pTI->iDBRWRatio = (int)DclAtoL(&achArgBuff[9]);
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/DBOPS:", 7) == 0)
        {
            pTI->nDBOpsPerIteration = (unsigned)DclAtoL(&achArgBuff[7]);
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/DBRECS:", 8) == 0)
        {
            pTI->nDBRecords = (unsigned)DclAtoL(&achArgBuff[8]);
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/PACING:", 8) == 0)
        {
            pTI->nPacingMS = (unsigned)DclAtoL(&achArgBuff[8]);
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/SAMPLE:", 8) == 0)
        {
            const char *pStr = &achArgBuff[8];

            pTI->nSampleSecs = DclAtoL(pStr);

            while(DclIsDigit(*pStr))
                pStr++;

            /*  Default is for seconds.  Only convert from minutes
                if the right suffix is included.
            */
            if(DclToUpper(*pStr) == 'M' && *(pStr+1) == 0)
            {
                pTI->nSampleSecs *= 60;
            }
            else if(!((DclToUpper(*pStr) == 'S' && *(pStr+1) == 0) || *pStr == 0))
            {
                /*  Error if there is an unrecognized suffix
                */
                goto BadOption;
            }

            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/FSBLOCKSIZE:", 13) == 0)
        {
            pTI->ulFSBlockSize = DclAtoI(&achArgBuff[13]);
            if(pTI->ulFSBlockSize == 0 || !DCLISPOWEROF2(pTI->ulFSBlockSize))
            {
                DclPrintf("The /FSBlockSize value must be a power-of-two value greater than 0.\n\n");
                return DCLSTAT_BADPARAMETER;
            }

            DclPrintf("Overriding the default FS Block Size with that specified on the command-line (%lU)\n", pTI->ulFSBlockSize);
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/FSDEVNAME:", 11) == 0)
        {
            DclStrNCpy(szFSDevName, &achArgBuff[11], sizeof(szFSDevName));

            continue;
        }
        else
        {
            /*  Process common test arguments which are shared by
                multiple tests.
            */
            if(DclTestParseParam(&pTI->sti, achArgBuff))
                continue;
        }

        uSwitch = 1;

        if(achArgBuff[uSwitch-1] == '/')
        {
            switch(achArgBuff[uSwitch])
            {
                case '?':
                    ShowHelp(pTI->sti.hDclInst, pszCmdName);
                    return DCLSTAT_HELPREQUEST;

                case 'i':
                case 'I':
                {
                    if(achArgBuff[uSwitch + 1] != ':')
                        goto BadOption;

                    pTI->ulIterations = (D_UINT32)DclAtoL(&achArgBuff[uSwitch + 2]);
                    break;
                }

                case 'p':
                case 'P':
                    if(achArgBuff[uSwitch + 1] != ':')
                        goto BadOption;

                    if((achArgBuff[uSwitch + 2] == '"') || (achArgBuff[uSwitch + 2] == '\''))
                    {
                         DclStrNCpy(pTI->szPath, &achArgBuff[uSwitch + 3],
                             DclStrLen(&achArgBuff[uSwitch + 3]) - 1);
                    }
                    else
                    {
                         DclStrNCpy(pTI->szPath, &achArgBuff[uSwitch + 2],
                             DclStrLen(&achArgBuff[uSwitch + 2]));
                    }
                    break;

                default:
                    goto BadOption;
            }
        }
        else
        {
      BadOption:
            DclPrintf("Bad option: \"%s\"\n\n", &achArgBuff[uSwitch-1]);
            return DCLSTAT_BADPARAMETER;
        }
    }

    pTI->nTotalThreads = pTI->nCaseThreads + pTI->nDBThreads + pTI->nFSIOThreads + pTI->nStreamThreads;
    if(!pTI->nTotalThreads)
    {
        DclPrintf("At least one thread must be specified.\n\n");
        return DCLSTAT_BADPARAMETER;
    }

    if(!InternalStatFS(pTI, &pTI->statfs))
    {
        DclPrintf("Unable to get file system information (try using the \"/P:path\" option).\n\n");
        return DCLSTAT_BADPARAMETER;
    }

    if(pTI->sti.ulTestSeconds >= DEFAULT_LONG_HOURS*60*60 && pTI->nSampleSecs == DEFAULT_SAMPLE_SECONDS)
        pTI->nSampleSecs = DEFAULT_SAMPLE_MINUTES * 60;

    /*  If the sample rate is larger than the test run length, reduce
        it to 1/4 of the test run length.
    */  
    if(pTI->sti.ulTestSeconds < pTI->nSampleSecs)
        pTI->nSampleSecs = pTI->sti.ulTestSeconds / 4;

    pTI->nSampleSecs = DCLMAX(pTI->nSampleSecs, 1);

    /*  If not specified on the command-line, set our local block size value
        to match that returned by statfs.
    */
    if(!pTI->ulFSBlockSize)
        pTI->ulFSBlockSize = pTI->statfs.ulBlockSize;

    /*  Ensure that the DB record size is evenly divisible by 4, is bigger
        than the FS blocks size, but is not evenly divisible by the FS block
        size.  We use 1.5 times the FS block size, minus 12.
    */        
    pTI->ulDBRecordSize = (pTI->ulFSBlockSize + (pTI->ulFSBlockSize >> 1)) - 12;

    if(szFSDevName[0])
    {
        DclPrintf("Overriding the default FS Device Name \"%s\" with that specified on the command-line \"%s\"\n",
            pTI->statfs.szDeviceName, szFSDevName);

        DclStrNCpy(pTI->statfs.szDeviceName, szFSDevName, sizeof(pTI->statfs.szDeviceName));
    }

    /*  If a path was not specified, default to the current directory
    */
    if(!pTI->szPath[0])
    {
        DCLSTATUS   dclStat;

        dclStat = DclFsDirGetWorking(pTI->szPath, sizeof(pTI->szPath));

        /*  If that did not work, or is not supported, try to use a
            relative path.
        */
        if(dclStat != DCLSTAT_SUCCESS)
            DclStrCpy(pTI->szPath, ".");
    }

    /*  A path was specified, ensure it ends with a path separator
    */
    nLen = DclStrLen(pTI->szPath);
    DclAssert(nLen < sizeof(pTI->szPath));

    if(pTI->szPath[nLen - 1] != DCL_PATHSEPCHAR)
    {
        pTI->szPath[nLen + 0] = DCL_PATHSEPCHAR;
        pTI->szPath[nLen + 1] = 0;
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: ShowHelp()

    Shows the usage of the program.

    Parameters:
        pReserved  - Not currently used
        pszCmdName - The name of the command

    Return Value:
        None.
-------------------------------------------------------------------*/
static void ShowHelp(
    DCLINSTANCEHANDLE   hDclInst,
    const char         *pszCmdName)
{
    DclAssert(hDclInst);
    DclAssertReadPtr(pszCmdName, 0);

    DclPrintf("Usage:  %s [options]\n\n", pszCmdName);
    DclPrintf("Where 'options' are zero or more of the following:\n");
    DclPrintf("  /?             Display this help information\n");
    DclPrintf("  /Case:n        Specifies the number of Case threads to use (default is %u)\n", DEFAULT_CASE_THREADS);
    DclPrintf("  /DB:n          Specifies the number of DataBase threads to use (default is %u)\n", DEFAULT_DB_THREADS);
    DclPrintf("  /FSIO:n        Specifies the number of FSIOTEST threads to use (default is %u)\n", DEFAULT_FSIO_THREADS);
    DclPrintf("  /Stream:opts   Specifies a streaming test configuration (see description)\n");
    DclPrintf("  /I:n           Specifies the number of iterations to run each thread (default\n");
    DclPrintf("                 is %u)\n", DEFAULT_ITERATIONS);
    DLTSHAREDTESTHELP_TIME();
    DclPrintf("  /P:'path'      Specifies the Path to use (default is the current directory)\n");
    DclPrintf("  /Sample:n[s|m] Specifies the time between samples.  Defaults to %u seconds for\n", DEFAULT_SAMPLE_SECONDS);
    DclPrintf("                 tests under %u hours, and %u minutes for longer tests.\n", DEFAULT_LONG_HOURS, DEFAULT_SAMPLE_MINUTES);
    DLTSHAREDTESTHELP_SEED();
    if(!DclTestPressEnter(hDclInst))
        return;
    DclPrintf("\n");
    DclPrintf("Logging Options:\n");
  #if DCLCONF_OUTPUT_ENABLED
    DLTSHAREDTESTHELP_LOG();
    DLTSHAREDTESTHELP_LB();
    DLTSHAREDTESTHELP_LS();
    DLTSHAREDTESTHELP_PERFLOG();
    DLTSHAREDTESTHELP_STATS();
    DLTSHAREDTESTHELP_TRACE();
  #endif
    DLTSHAREDTESTHELP_VERBOSITY();
    if(!DclTestPressEnter(hDclInst))
        return;
    DclPrintf("\n");
    DclPrintf("Advanced Options:\n");
    DclPrintf("  /DBRatio:n     Specifies the DataBase operations ratio of reads to writes (if\n");
    DclPrintf("                 positive), or writes to reads (if negative).  Special cases of\n");
    DclPrintf("                 0 means to do writes only, and -1 means to do reads only.  The\n");
    DclPrintf("                 default value is %u reads for each write.\n", DEFAULT_DB_READWRITE_RATIO);
    DclPrintf("  /DBOps:n       Specifies the number of DataBase Operations per iteration.\n");
    DclPrintf("                 Defaults to %u.\n", DEFAULT_DB_OPSPERITERATION);
    DclPrintf("  /DBRecs:n      Specifies the number of DataBase Records.  Defaults to %u.\n", DEFAULT_DB_RECORDS);
    DclPrintf("  /FSBlockSize:n The file system block size to use, in bytes (See Notes).\n");
    DclPrintf("  /FSDevName:nam The device name on which the file system resides (see Notes).\n");
    DLTSHAREDTESTHELP_REQ();
    DclPrintf("  /Pacing:n      Pacing may be used to help ensure that all the threads stay\n");
    DclPrintf("                 relatively close with their iteration counts, by causing the\n");
    DclPrintf("                 more advanced threads to sleep occasionally.  The larger the\n");
    DclPrintf("                 specified value is, the more aggressive the pacing will be.\n");
    DclPrintf("                 Specify 0 to disable the feature (the default).\n");
    DLTSHAREDTESTHELP_PROF();
    if(!DclTestPressEnter(hDclInst))
        return;
    DclPrintf("\n");
    DclPrintf("Streaming Test Configuration:\n");
    DclPrintf("Multiple streaming threads (up to %u) may be configured to access one or more\n", MAX_STREAM_THREADS);
    DclPrintf("files.  The /Stream parameter may be specified multiple times to configure\n");
    DclPrintf("groups of threads with similar characteristics, and has the following format:\n\n");
    DclPrintf("            /Stream:count[:Pn]:R|W[:FrameSize[:FileNum[:FileSize]]]\n");
    DclPrintf("Where:\n");
    DclPrintf("     Count - The number of threads to run in this configuration.  If this value\n");
    DclPrintf("             is zero, then streaming test cases will be disabled, and no other\n");
    DclPrintf("             arguments may be specified.\n");
    DclPrintf("        Pn - The optional thread priority, where 'n' is a value from %u to %u,\n", DCL_THREADPRIORITY_ENUMLOWLIMIT+1, DCL_THREADPRIORITY_ENUMHIGHLIMIT-1);
    DclPrintf("             with %u being highest priority.  If not specified, the threads will\n", DCL_THREADPRIORITY_ENUMLOWLIMIT+1);
    DclPrintf("             have \"normal\" (%u) priority.\n", DCL_THREADPRIORITY_NORMAL);
    DclPrintf("       R|W - Specifies whether these threads are reading or writing.  This\n");
    DclPrintf("             argument is required (if Count is non-zero).\n");
    DclPrintf(" FrameSize - The size of each I/O operation.  If not specified, will default to\n");
    DclPrintf("             %u times the file system block size.\n", STREAM_IO_FACTOR);
    DclPrintf("   FileNum - The file number upon which these threads will operate.  File\n");
    DclPrintf("             numbers range from 1 to %u.\n", MAX_STREAM_FILES);
    DclPrintf("  FileSize - The file size.  May have a KB/MB/GB suffix.  Defaults to one half\n");
    DclPrintf("             of the free disk space divided by the number of stream threads.\n");
    DclPrintf("             If multiple /Stream configurations specify the same FileNum, and\n");
    DclPrintf("             the FileSize is specified more than once, the size specified in\n");
    DclPrintf("             the final configuration will be used.\n");
    DclPrintf("Stream Notes:\n");
    DclPrintf(" - The stream test results are quantified in terms of frames per second, with a\n");
    DclPrintf("   range of drop rates from 0 to 10%%.\n");
    DclPrintf(" - It is the user's responsibility to ensure that there is enough disk space to\n");
    DclPrintf("   accommodate the needs of the stream threads in combination with any other\n");
    DclPrintf("   threads invoked by this test.\n");
    if(!DclTestPressEnter(hDclInst))
        return;
    DclPrintf("\n");
    DclPrintf("Notes: 1) The test completes when all the threads have completed the specified\n");
    DclPrintf("          number of iterations, or the specified test time has elapsed, which-\n");
    DclPrintf("          ever comes first.\n");
    DclPrintf("       2) Options which have a 'size' designation may be specified in hex (0x)\n");
    DclPrintf("          or decimal, and may be suffixed by 'B', 'KB', or 'MB'.  If there is\n");
    DclPrintf("          no scale designated, then KB is assumed.  Don't use a 'B' suffix for\n");
    DclPrintf("          hex numbers as it will be interpreted as part of the hex number.\n");
    DclPrintf("       3) For consistent throughput results (especially on flash media), the\n");
    DclPrintf("          test MUST be run on a freshly formatted disk.\n");
    DclPrintf("       4) The /FSBlockSize and /FSDevName options are often used together and\n");
    DclPrintf("          are useful for environments where the automatic determination doesn't\n");
    DclPrintf("          work.  Using them will override any automatically determined values.\n");
    DclPrintf("       5) When the /LOG option is used, the file should be on a different disk\n");
    DclPrintf("          than that being tested, or the /LB option should specify a buffer\n");
    DclPrintf("          large enough to hold the entire test output.  Otherwise throughput\n");
    DclPrintf("          results may be adversely affected.\n");
    DclPrintf("       6) Test options are neither case sensitive nor order sensitive.\n\n");

    return;
}


/*-------------------------------------------------------------------
    Local: ProcessStreamArgs()

    Parameters:
        pTI     - A pointer to the FSIOTESTINFO structure to use.
        pStatFS - A pointer to the DCLFSSTATFS structure to fill.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL ProcessStreamArgs(
    FSSTRESSTESTINFO   *pTI,
    const char         *pszArgs)
{
    const char         *pStr = pszArgs;
    D_UINT32            ulThreads;
    D_UINT32            tt;
    D_BOOL              fWriting = FALSE;                           /* default */
    D_UINT32            ulFileNum = pTI->nStreamFiles+1;            /* default */
    D_UINT32            ulFrameSize = 0;
    D_UINT32            ulFileSizeKB = 0;
    D_UINT32            ulPriority = DCL_THREADPRIORITY_NORMAL;
    
    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertReadPtr(pszArgs, 0);

    /*  Expected format:  /stream:threads[:r|w[:IOSize[:FileNum[:FileSize]]]]

        If "threads" is zero, then no other arguments may be specified.
        If "threads" is non-zero, the access mode R or W must be specified.
        If FileSize for a given FileNum is specfied more than once, the 
        latter value will be used.  
    */    

    pStr = DclNtoUL(pStr, &ulThreads);
    if(!pStr || (*pStr != 0 && *pStr != ':'))
        return FALSE;

    if(!ulThreads && pTI->nStreamThreads)
    {
        DclPrintf("Specifying 0 threads is not legal when other streams have been initialized\n");
        return FALSE;
    }
    
    if(!ulThreads && *pStr)
    {
        DclPrintf("Supplemental arguments are not valid when specifying 0 threads\n");
        return FALSE;
    }

    if(ulThreads + pTI->nStreamThreads > MAX_STREAM_THREADS)
    {
        DclPrintf("The maximum stream threads count of %u would be exceeded\n", MAX_STREAM_THREADS);
        return FALSE;
    }

    /*  Check to see if we're disabling the streams feature
    */
    if(!ulThreads)
    {
        pTI->nStreamThreads = 0;
        return TRUE;
    }
    
    if(*pStr != ':')
    {
        DclPrintf("An access mode must be specified\n");
        return FALSE;
    }

    /*  Skip the ':'
    */
    pStr++;
    if(DclToUpper(*pStr) == 'P')
    {
        pStr++;
        pStr = DclNtoUL(pStr, &ulPriority);
        if(!pStr || *pStr != ':')
            goto SyntaxError;

        if( (ulPriority < (D_UINT32)(DCL_THREADPRIORITY_ENUMLOWLIMIT+1)) ||
            (ulPriority > (D_UINT32)(DCL_THREADPRIORITY_ENUMHIGHLIMIT-1)) )
        {
            DclPrintf("The thread priority must range from %u to %u.\n", 
                DCL_THREADPRIORITY_ENUMLOWLIMIT+1, DCL_THREADPRIORITY_ENUMHIGHLIMIT-1);
            
            return FALSE;
        }

        /*  Skip the ':'
        */
        pStr++;
    }
    
    if(DclToUpper(*pStr) == 'W')
    {
        fWriting = TRUE;
    }
    else if(DclToUpper(*pStr) != 'R')
    {
        DclPrintf("The access mode must be 'R' or 'W'\n");
        return FALSE;
    }
        
    /*  Skip the mode and process any remaining parameters, all optional
    */
    pStr++;
    if(*pStr)
    {
        if(*pStr != ':' || *++pStr == 0)
            goto SyntaxError;

        /*  Get the I/O size in bytes.
        */
        pStr = DclSizeToUL(pStr, &ulFrameSize);
        if(!pStr)
            goto SyntaxError;

        if(!ulFrameSize)
        {
            DclPrintf("An frame size of zero is not valid\n");
            return FALSE;
        }

        /*  Process any remaining parameters, all optional
        */
        if(*pStr)
        {
            if(*pStr != ':' || *++pStr == 0)
                goto SyntaxError;

            /*  Get the file number
            */
            pStr = DclNtoUL(pStr, &ulFileNum);
            if(!pStr)
                goto SyntaxError;

            if(!ulFileNum || ulFileNum > MAX_STREAM_FILES)
            {
                DclPrintf("Stream file numbers must range from 1 to %u\n", MAX_STREAM_FILES);
                return FALSE;
            }

            /*  Process any remaining parameters, all optional
            */
            if(*pStr)
            {
                D_UINT64    ullFileSize;
                
                if(*pStr != ':' || *++pStr == 0)
                    goto SyntaxError;

                pStr = DclSizeToULKB(pStr, &ulFileSizeKB);
                if(!pStr || *pStr)
                    goto SyntaxError;

                ullFileSize = ((D_UINT64)ulFileSizeKB) * 1024;

                if(ullFileSize < ulFrameSize)
                {
                    DclPrintf("The file size cannot be less than the frame size\n");
                    return FALSE;
                }
            }
        }
    }

    /*  For each of however many threads we found in this definition,
        record the thread information.
    */  
    for(tt = pTI->nStreamThreads; tt < pTI->nStreamThreads + ulThreads; tt++)
    {
        pTI->aStrT[tt].nPriority   = (unsigned)ulPriority;
        pTI->aStrT[tt].nFileNum    = (unsigned)ulFileNum-1;
        pTI->aStrT[tt].ulFrameSize = ulFrameSize;
        pTI->aStrT[tt].fWriting    = fWriting;
    }

    /*  For the file used, record the size.
    */
    pTI->aStrF[ulFileNum-1].ullFileSize = ((D_UINT64)ulFileSizeKB) * 1024;

    /*  Adjust the threads counter
    */
    pTI->nStreamThreads += ulThreads;

    /*  Adjust the files counter in case we need to auto-determine
        the next file number.
    */
    pTI->nStreamFiles++;
 
    return TRUE;


  SyntaxError:
    if(!pStr)
        pStr = pszArgs;
    
    DclPrintf("Syntax error in \"%s\"\n", pStr);
    
    return FALSE;
}


/*-------------------------------------------------------------------
    Local: InternalStatFS()

    This function implements "statfs" functionality but scales
    the results based on the FS block size specified on the
    command-line (if any).

    Parameters:
        pTI     - A pointer to the FSIOTESTINFO structure to use.
        pStatFS - A pointer to the DCLFSSTATFS structure to fill.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL InternalStatFS(
    FSSTRESSTESTINFO   *pTI,
    DCLFSSTATFS        *pStatFS)
{
    DCLFSSTATFS         statfs;

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertWritePtr(pStatFS, sizeof(*pStatFS));

    if(DclFsStatFs((pTI->szPath[0] ? pTI->szPath : "."), &statfs))
    {
        DclPrintf("Error: DclFsStatFs(%s) failed!\n", pTI->szPath[0] ? pTI->szPath : ".");
        return FALSE;
    }

    /*  Since we may have overridden the block size to use on the command-
        line (because some environments don't let this be accurately queried),
        scale the results reported so they are accurate based on the desired
        block size.
    */
    if(pTI->ulFSBlockSize && statfs.ulBlockSize != pTI->ulFSBlockSize)
    {
        D_UINT64    ullTemp;

        ullTemp = statfs.ulTotalBlocks;
        DclUint64MulUint32(&ullTemp, statfs.ulBlockSize);
        DclUint64DivUint32(&ullTemp, pTI->ulFSBlockSize);
        DclAssert(ullTemp < D_UINT32_MAX);
        statfs.ulTotalBlocks = (D_UINT32)ullTemp;

        ullTemp = statfs.ulFreeBlocks;
        DclUint64MulUint32(&ullTemp, statfs.ulBlockSize);
        DclUint64DivUint32(&ullTemp, pTI->ulFSBlockSize);
        DclAssert(ullTemp < D_UINT32_MAX);
        statfs.ulFreeBlocks = (D_UINT32)ullTemp;

        statfs.ulBlockSize = pTI->ulFSBlockSize;
    }

    *pStatFS = statfs;

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: DisplaySummary()

    Used to output a progress note, synchronized with the other threads

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void DisplaySummary(
    FSSTRESSTESTINFO   *pTI,
    D_UINT32            ulMS,
    D_BOOL              fSummary)
{
    unsigned            tt;
    D_UINT32            ulSecs = (ulMS + 500) / 1000;
    char                szState[16];
    char                szElapsed[32];
    FSPRIMCONTROL       PrimSummary = {0};

    DclAssertWritePtr(pTI, sizeof(*pTI));

    /*  Avoid any possible divide by zero
    */
    if(!ulMS)
        ulMS++;

    /*  Display progress statistics.
    */
    DclSNPrintf(szElapsed, sizeof(szElapsed), "%u:%02u:%02u",
        ulSecs / (60*60),
        (ulSecs % (60*60)) / 60,
        (ulSecs % (60*60)) % 60);

    if(fSummary)
    {
        DclPrintf("-------------------------------------------------------------------------------\n");
        DclPrintf("------------------------------> Final Sum Totals <-----------------------------\n" );
        DclPrintf("Total Elapsed Time: %8s                             ThreadState StatusCode\n", szElapsed);
    }
    else
    {
        pTI->nSample++;

        /*  Display progress statistics.
        */
        DclPrintf("Sample: %3u  Elapsed Time: %8s                      ThreadState StatusCode\n", pTI->nSample, szElapsed);
    }
    
    for(tt = 0; tt < pTI->nTotalThreads; tt++)
    {
        if(fSummary)
            DclTestFSPrimStatsAccumulate(&PrimSummary, &pTI->paTC[tt].FSPrim);
        
        switch(pTI->paTC[tt].ThreadExecState)
        {
            case EXEC_INITIALIZING:
                DclStrCpy(szState, " INITING  ");
                break;
            case EXEC_WAITING:
                DclStrCpy(szState, " WAITING  ");
                break;
            case EXEC_RUNNING:
                DclStrCpy(szState, " RUNNING  ");
                break;
            case EXEC_QUITTING:
                DclStrCpy(szState, " QUITTING ");
                break;
            case EXEC_EXITING:
                DclStrCpy(szState, " EXITING  ");
                break;
            default:
                DclProductionError();
        }

        DclPrintf("  %6s Thread %s (P%u) completed iterations: %5lU  %10s %lX\n",
            pTI->paTC[tt].pszTypeName,
            pTI->paTC[tt].szName, 
            pTI->paTC[tt].nPriority,
            DclOsAtomic32Retrieve(&pTI->paTC[tt].ulAtomicIterationsCompleted),
            szState,
            pTI->paTC[tt].dclStat);
    }        

    if(pTI->nDBThreads || pTI->nCaseThreads || pTI->nStreamThreads)
        DclPrintf("  Operation Details         Count   TotalMS AverageUS MaximumUS  TotalMB KB/sec\n");

    if(pTI->nCaseThreads)
    {
        for(tt = 0; tt < pTI->nNumTestCases; tt++)
        {
            D_UINT64    ullSampleUS;
            D_UINT32    ulSampleMaxUS;
            D_UINT32    ulSampleOps;

            if(DclCriticalSectionEnter(&aFSStressTestCases[tt].ulAtomicGate, 2000, TRUE))
            {
                ullSampleUS   = aFSStressTestCases[tt].ullSampleTotalUS;
                ulSampleOps   = aFSStressTestCases[tt].nSampleOperations;
                ulSampleMaxUS = aFSStressTestCases[tt].ulSampleMaxUS; 

                /*  Reset for the next sample
                */
                aFSStressTestCases[tt].ullSampleTotalUS  = 0;
                aFSStressTestCases[tt].nSampleOperations = 0;
                aFSStressTestCases[tt].ulSampleMaxUS     = 0; 

                DclCriticalSectionLeave(&aFSStressTestCases[tt].ulAtomicGate);
            }
            else
            {
                DclPrintf("Error entering critical section while displaying stats for test %u -- ignoring...\n", tt);
                continue;
            }

            if(!fSummary)
            {
                /*  We're not doing a final summary, so just accumulate
                    the counters.
                */
                aFSStressTestCases[tt].ullTotalUS        += ullSampleUS;
                aFSStressTestCases[tt].ulTotalOperations += ulSampleOps;
                aFSStressTestCases[tt].ulMaxUS            = DCLMAX(ulSampleMaxUS, aFSStressTestCases[tt].ulMaxUS);
            }
            else
            {
                /*  This is the final summary so adjust what we are 
                    displaying to be the grand sum totals (for this
                    test case).
                */
                ullSampleUS   += aFSStressTestCases[tt].ullTotalUS;
                ulSampleOps   += aFSStressTestCases[tt].ulTotalOperations;
                ulSampleMaxUS  = DCLMAX(aFSStressTestCases[tt].ulMaxUS, ulSampleMaxUS);
            }
        }
    }

    for(tt = 0; tt < pTI->nTotalThreads; tt++)
    {
        char        szTempBuff[24];
        D_UINT32    ulItems;
        D_UINT64    ullBytes;
        D_UINT64    ullUS;
        D_UINT32    ulMaxUS;
            
        if(pTI->paTC[tt].nType != THREADTYPE_DB && pTI->paTC[tt].nType != THREADTYPE_STREAM)
            continue;

        if(DclCriticalSectionEnter(&pTI->paTC[tt].ulAtomicReadGate, 2000, TRUE))
        {
            /*  Get our values for this sample...
            */
            ulItems  = pTI->paTC[tt].ulReads;
            ullBytes = pTI->paTC[tt].ulReadBytes;
            ullUS    = pTI->paTC[tt].ulReadUS;
            ulMaxUS  = pTI->paTC[tt].ulReadMaxUS;

            /*  ...reset the counters...
            */
            pTI->paTC[tt].ulReads = 0;
            pTI->paTC[tt].ulReadBytes = 0;
            pTI->paTC[tt].ulReadUS = 0;
            pTI->paTC[tt].ulReadMaxUS = 0;

            /*  ...and leave the critical section ASAP
            */
            DclCriticalSectionLeave(&pTI->paTC[tt].ulAtomicReadGate);

            if(!fSummary)
            {
                /*  We're not doing a final summary, so just accumulate
                    the counters.
                */
                pTI->paTC[tt].ulTotalReads      += ulItems;
                pTI->paTC[tt].ullTotalReadBytes += ullBytes;
                pTI->paTC[tt].ullTotalReadUS    += ullUS;
                pTI->paTC[tt].ulTotalReadMaxUS   = DCLMAX(ulMaxUS, pTI->paTC[tt].ulTotalReadMaxUS);
            }
            else
            {
                /*  This is the final summary so adjust what we are 
                    displaying to be the grand sum totals (for this
                    thread).
                */
                ulItems  += pTI->paTC[tt].ulTotalReads;
                ullBytes += pTI->paTC[tt].ullTotalReadBytes;
                ullUS    += pTI->paTC[tt].ullTotalReadUS;
                ulMaxUS   = DCLMAX(pTI->paTC[tt].ulTotalReadMaxUS, ulMaxUS);
            }

            if(ulItems)
            {
                D_BOOL  fReading = TRUE;

                /*  Special case for STREAM threads which accumuldate stats
                    only in the READER fields, even when writing.
                */    
                if((pTI->paTC[tt].nType == THREADTYPE_STREAM) && pTI->aStrT[pTI->paTC[tt].nThreadNumByType].fWriting)
                    fReading = FALSE;
                
                DclPrintf("  %6s Thread%02u %6s:%8lU %9llU %9llU %9lU %8s %6lU\n",
                    pTI->paTC[tt].nType == THREADTYPE_DB ? "DB" : "Stream",
                    tt,
                    fReading ? "Reads" : "Writes",
                    ulItems, 
                    (ullUS + 500) / 1000,
                    ulItems ? ullUS / ulItems : 0,
                    ulMaxUS,
                    DclRatio64(szTempBuff, sizeof(szTempBuff), ullBytes + ((512*1024)/10), 1024 * 1024, 1),
                    GetKBPerSecond((ullBytes + 512) / 1024, (D_UINT32)((ullUS + 500) / 1000)));
            }
        }
        else
        {
            DclPrintf("Error entering critical section while displaying read stats -- ignoring...\n");
        }

        if(DclCriticalSectionEnter(&pTI->paTC[tt].ulAtomicWriteGate, 2000, TRUE))
        {
            /*  Get our values for this sample...
            */
            ulItems  = pTI->paTC[tt].ulWrites;
            ullBytes = pTI->paTC[tt].ulWriteBytes;
            ullUS    = pTI->paTC[tt].ulWriteUS;
            ulMaxUS  = pTI->paTC[tt].ulWriteMaxUS;

            /*  ...reset the counters...
            */
            pTI->paTC[tt].ulWrites = 0;
            pTI->paTC[tt].ulWriteBytes = 0;
            pTI->paTC[tt].ulWriteUS = 0;
            pTI->paTC[tt].ulWriteMaxUS = 0;

            /*  ...and leave the critical section ASAP
            */
            DclCriticalSectionLeave(&pTI->paTC[tt].ulAtomicWriteGate);

            if(!fSummary)
            {
                /*  We're not doing a final summary, so just accumulate
                    the counters.
                */
                pTI->paTC[tt].ulTotalWrites      += ulItems;
                pTI->paTC[tt].ullTotalWriteBytes += ullBytes;
                pTI->paTC[tt].ullTotalWriteUS    += ullUS;
                pTI->paTC[tt].ulTotalWriteMaxUS   = DCLMAX(ulMaxUS, pTI->paTC[tt].ulTotalWriteMaxUS);
            }
            else
            {
                /*  This is the final summary so adjust what we are 
                    displaying to be the grand sum totals (for this
                    thread).
                */
                ulItems  += pTI->paTC[tt].ulTotalWrites;
                ullBytes += pTI->paTC[tt].ullTotalWriteBytes;
                ullUS    += pTI->paTC[tt].ullTotalWriteUS;
                ulMaxUS   = DCLMAX(pTI->paTC[tt].ulTotalWriteMaxUS, ulMaxUS);
            }

            if(ulItems)
            {
                DclPrintf("      DB Thread%02u Writes:%8lU %9llU %9llU %9lU %8s %6lU\n",
                    tt,
                    ulItems, 
                    (ullUS + 500) / 1000,
                    ulItems ? ullUS / ulItems : 0,
                    ulMaxUS,
                    DclRatio64(szTempBuff, sizeof(szTempBuff), ullBytes + ((512*1024)/10), 1024 * 1024, 1),
                    GetKBPerSecond((ullBytes + 512) / 1024, (D_UINT32)((ullUS + 500) / 1000)));
            }
        }
        else
        {
            DclPrintf("Error entering critical section while displaying write stats -- ignoring...\n");
        }
    }

    if(pTI->nStreamThreads)
    {
        DclPrintf("  Frames/Sec@DropRate 0%%   1%%   2%%   3%%   4%%   5%%   6%%   7%%   8%%   9%%  10%% AvgUS\n");
                
        for(tt = 0; tt < pTI->nTotalThreads; tt++)
        {
            STREAMTHREADINFO   *pStreamThread;
            D_UINT64            ullUS;
            D_UINT32            ulCount;
            D_UINT32            aulResults[STREAM_RESULT_COUNT];
            
            if(pTI->paTC[tt].nType != THREADTYPE_STREAM)
                continue;

            pStreamThread = &pTI->aStrT[pTI->paTC[tt].nThreadNumByType];

            if(DclCriticalSectionEnter(&pTI->paTC[tt].ulAtomicReadGate, 2000, TRUE))
            {
                unsigned    nn;
                
                /*  Get our values for this sample...
                */
                ulCount  = pStreamThread->ulSampleCount;
                ullUS  = pStreamThread->ullSampleUS;
                DclMemCpy(aulResults, pStreamThread->aulSampleResults, DCLDIMENSIONOF(aulResults) * sizeof(aulResults[0]));

                /*  ...reset the counters...
                */
                pStreamThread->ulSampleCount = 0;
                pStreamThread->ullSampleUS = 0;
                DclMemSet(pStreamThread->aulSampleResults, 0, 
                    DCLDIMENSIONOF(pStreamThread->aulSampleResults) * sizeof(pStreamThread->aulSampleResults[0]));
 
                /*  ...and leave the critical section ASAP
                */
                DclCriticalSectionLeave(&pTI->paTC[tt].ulAtomicReadGate);

                if(!fSummary)
                {
                    /*  We're not doing a final summary, so just accumulate
                        the counters.
                    */
                    pStreamThread->ulTotalCount += ulCount;
                    pStreamThread->ullTotalUS   += ullUS;
                    for(nn = 0; nn < DCLDIMENSIONOF(aulResults); nn++)
                        pStreamThread->aulTotalResults[nn] += aulResults[nn];
                }
                else
                {
                    /*  This is the final summary so adjust what we are 
                        displaying to be the grand sum totals (for this
                        thread).
                    */
                    ulCount += pStreamThread->ulTotalCount;
                    ullUS   += pStreamThread->ullTotalUS;
                    for(nn = 0; nn < DCLDIMENSIONOF(aulResults); nn++)
                        aulResults[nn] += pStreamThread->aulTotalResults[nn];
                }

                if(ulCount)
                {
                    DclPrintf("    %5s stream %u:%5lU%5lU%5lU%5lU%5lU%5lU%5lU%5lU%5lU%5lU%5lU%6lU\n", 
                        pStreamThread->fWriting ? "Write" : " Read",
                        pTI->paTC[tt].nThreadNumByType,
                        1000000 / (aulResults[0] / ulCount), 1000000 / (aulResults[1] / ulCount), 
                        1000000 / (aulResults[2] / ulCount), 1000000 / (aulResults[3] / ulCount), 
                        1000000 / (aulResults[4] / ulCount), 1000000 / (aulResults[5] / ulCount), 
                        1000000 / (aulResults[6] / ulCount), 1000000 / (aulResults[7] / ulCount), 
                        1000000 / (aulResults[8] / ulCount), 1000000 / (aulResults[9] / ulCount), 
                        1000000 / (aulResults[10] / ulCount), 
                        (D_UINT32)(ullUS / (pStreamThread->ulFrames * ulCount)));
                }
                else
                {
                    DclPrintf("    %5s stream %u:%5lU%5lU%5lU%5lU%5lU%5lU%5lU%5lU%5lU%5lU%5lU%6lU\n", 
                        pStreamThread->fWriting ? "Write" : " Read",
                        pTI->paTC[tt].nThreadNumByType,
                        0, 0, 
                        0, 0, 
                        0, 0, 
                        0, 0, 
                        0, 0, 
                        0, 
                        0);
                }
            }
        }
    }        

    if(fSummary) 
        DclTestFSPrimStatsDisplay(&PrimSummary);

    return;
}


/*-------------------------------------------------------------------
    GetKBPerSecond() - Calculate KB/sec, scaling as necessary

    Description

    Parameters

    Return Value
-------------------------------------------------------------------*/
static D_UINT32 GetKBPerSecond(
    D_UINT64        ullKB,
    D_UINT32        ulMS)
{
    D_UINT32        ulScale = 100000UL;

    /*  Avoid divide-by-zero...
    */
    if(!ulMS)
    {
        /*  If nothing was written in no time, just return zero
        */
        if(!ullKB)
            return 0;

        /*  Something was written, so assume it took at least a millisecond...
        */
        ulMS++;
    }

    while((ullKB > D_UINT32_MAX / ulScale) || (ulMS > D_UINT32_MAX / ulScale))
        ulScale /= 2;

    ullKB *= ulScale;
    ulMS *= ulScale;

    if(ulMS < 1000)
        return 0;
    else
        return (D_UINT32)(ullKB / (ulMS / 1000));
}


/*-------------------------------------------------------------------
    Local: Case_MkRmDir_Init()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_MkRmDir_Init(
    const DLTFSSTRESSCASEINFO *pCaseInfo)
{
    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: Case_MkRmDir_Run()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_MkRmDir_Run(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    DCLSTATUS                   dclStat;
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;

    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    dclStat = PRIMDIRCREATE(pCaseInfo->pszBaseName);
    if(dclStat == DCLSTAT_SUCCESS)
        dclStat = PRIMDIRREMOVE(pCaseInfo->pszBaseName);

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: Case_MkRmDir_Clean()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_MkRmDir_Clean(
    const DLTFSSTRESSCASEINFO *pCaseInfo)
{
    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: Case_GetSetCWD_Init()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_GetSetCWD_Init(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;
    
    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    if(!pCaseInfo->fSupportsCWD)
        return DCLSTAT_SUCCESS;

    return PRIMDIRCREATE(pCaseInfo->pszBaseName);
}


/*-------------------------------------------------------------------
    Local: Case_GetSetCWD_Run()

    *Note* -- Typically the CWD is a per-process item, rather than
              per thread.  Therefore we can get and set all we want
              to make sure we don't have any internal FS instability,
              but we can't necessarily expect that we're going to be
              where we think we are.

    *Note* -- Some operating systems do not support the concept of a
              current diretory.  If this is the case, it will be
              detected and this test will return "SUCCESS".

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_GetSetCWD_Run(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;
    char                        szFullCurrentName[MAX_FILESPEC_LEN];
    DCLSTATUS                   dclStat;

    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    if(!pCaseInfo->fSupportsCWD)
        return DCLSTAT_SUCCESS;

    dclStat = PRIMDIRSETCWD(pCaseInfo->pszBaseName);
    if(dclStat != DCLSTAT_SUCCESS)
         return dclStat;

    szFullCurrentName[0] = 0;
    dclStat = PRIMDIRGETCWD(szFullCurrentName, DCLDIMENSIONOF(szFullCurrentName));
    if(dclStat != DCLSTAT_SUCCESS)
         return dclStat;

    /*  Restore the original base CWD so we don't try removing the temp dir
        while we're in it.  Since the directory name has our thread number
        in it, we know that no other thread but us could be in it (so
        therefore even if another thread happens to change the directory
        right after this operation, it will cause no harm since it will
        be some "other" directory).
    */
    return PRIMDIRSETCWD(pCaseInfo->pszBasePath);
}


/*-------------------------------------------------------------------
    Local: Case_GetSetCWD_Clean()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_GetSetCWD_Clean(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;
    
    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    if(!pCaseInfo->fSupportsCWD)
        return DCLSTAT_SUCCESS;

    return PRIMDIRREMOVE(pCaseInfo->pszBaseName);
}


/*-------------------------------------------------------------------
    Local: Case_CreateRemove_Init()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_CreateRemove_Init(
    const DLTFSSTRESSCASEINFO *pCaseInfo)
{
    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: Case_CreateRemove_Run()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_CreateRemove_Run(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    DCLFSFILEHANDLE             hFile;
    DCLSTATUS                   dclStat;
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;

    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    dclStat = PRIMCREATE(pCaseInfo->pszBaseName, &hFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = PRIMCLOSE(hFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    return PRIMDELETE(pCaseInfo->pszBaseName);
}


/*-------------------------------------------------------------------
    Local: Case_CreateRemove_Clean()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_CreateRemove_Clean(
    const DLTFSSTRESSCASEINFO *pCaseInfo)
{
    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: Case_ReadWrite_Init()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_ReadWrite_Init(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    DCLFSFILEHANDLE             hFile;
    DCLSTATUS                   dclStat;
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;

    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    dclStat = PRIMCREATE(pCaseInfo->pszBaseName, &hFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    return PRIMCLOSE(hFile);
}


/*-------------------------------------------------------------------
    Local: Case_ReadWrite_Run()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS Case_ReadWrite_Run(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    DCLFSFILEHANDLE             hFile = NULL;
    D_BUFFER                   *pReadBuffer = NULL;
    D_BUFFER                   *pWriteBuffer = NULL;
    void                       *pTemp;
    D_UINT32                    ulValue;
    D_UINT32                    ulIndex;
    D_UINT32                    ulBufferSize = pCaseInfo->pTC->pTI->ulFSBlockSize * 16;
    D_UINT32                    ulTransferred;
    D_UINT32                    ulTransferSize;
    DCLSTATUS                   dclStat = DCLSTAT_MEMALLOCFAILED;
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;

    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    ulTransferSize = (D_UINT32)((DclRand64(&pCaseInfo->pTC->pTI->sti.ullRandomSeed) % ulBufferSize) + 1);

    /*  Allocate read/write buffers
    */
    pWriteBuffer = DclMemAlloc(ulBufferSize);
    if(!pWriteBuffer)
        goto Cleanup;
    
    pReadBuffer = DclMemAlloc(ulBufferSize);
    if(!pReadBuffer)
        goto Cleanup;

    /*  Serialize the write data
    */
    pTemp = pWriteBuffer;
    ulValue = pCaseInfo->nThreadNum;
    for(ulIndex = 0; ulIndex < ulBufferSize; ulIndex += sizeof(ulValue))
    {
        COPYNATIVETOLE(pTemp, &ulValue);
        ulValue += 7;
    }

    dclStat = PRIMCREATE(pCaseInfo->pszBaseName, &hFile);
    if(dclStat != DCLSTAT_SUCCESS)
        goto Cleanup;

    dclStat = PRIMWRITE(hFile, pWriteBuffer, ulTransferSize, &ulTransferred);
    if((dclStat != DCLSTAT_SUCCESS) || (ulTransferred != ulTransferSize))
    {
        /*  Ensure any original error code is preserved
        */
        if(dclStat == DCLSTAT_SUCCESS)
            return DCLSTAT_FS_WRITEFAILED;

        goto Cleanup;
    }

    dclStat = PRIMSEEK(hFile, 0, DCLFSFILESEEK_SET);
    if(dclStat != DCLSTAT_SUCCESS)
        goto Cleanup;

    dclStat = PRIMREAD(hFile, pReadBuffer, ulTransferSize, &ulTransferred);
    if((dclStat != DCLSTAT_SUCCESS) || (ulTransferred != ulTransferSize))
    {
        /*  Ensure any original error code is preserved
        */
        if(dclStat == DCLSTAT_SUCCESS)
            return DCLSTAT_FS_READFAILED;

        goto Cleanup;
    }

    if(DclMemCmp(pReadBuffer, pWriteBuffer, ulTransferSize) != 0)
    {
        PRIMCLOSE(hFile);
        return DCLSTAT_CURRENTLINE;
    }

#if 0
    if(DclCriticalSectionEnter(&aFSStressTestCases[pCaseInfo->nTestCase].ulAtomicGate, 2000, TRUE))
    {
        /*  Should never wrap since D_UINT32_MAX microseconds is more 
            than an hour, and we should be sampling much more often
            than that.
        */                    
        DclAssert(D_UINT32_MAX - pTI->ulReadUS >= ulElapsedReadUS);
        DclAssert(D_UINT32_MAX - pTI->ulReadUS >= ulElapsedReadUS);

        aFSStressTestCases[pCaseInfo->nTestCase].ulReads++;            
        
        pTI->ulReadBytes += ulTransferred;
        pTI->ulReadUS += ulElapsedUS;
        if(pTI->ulReadMaxUS < ulElapsedUS)
            pTI->ulReadMaxUS = ulElapsedUS;

        DclCriticalSectionLeave(&pTI->ulAtomicReadGate);
    }
    else
    {
        DclPrintf("Error entering critical section while gathering case read stats -- ignoring...\n");
    }
#endif

 Cleanup:

    if(hFile)
    {
        DCLSTATUS   dclStat2;
        
        dclStat2 = PRIMCLOSE(hFile);

        /*  Ensure any original error code is preserved
        */
        if(dclStat == DCLSTAT_SUCCESS)
            dclStat = dclStat2;
    }

    /*  Release the read and write buffers
    */
    if(pReadBuffer)
        DclMemFree(pReadBuffer);

    if(pWriteBuffer)
        DclMemFree(pWriteBuffer);

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: Case_ReadWrite_Clean()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_ReadWrite_Clean(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;
    
    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    return PRIMDELETE(pCaseInfo->pszBaseName);
}


/*-------------------------------------------------------------------
    Local: Case_Flush_Init()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_Flush_Init(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    DCLFSFILEHANDLE             hFile;
    DCLSTATUS                   dclStat;
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;

    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    dclStat = PRIMCREATE(pCaseInfo->pszBaseName, &hFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    return PRIMCLOSE(hFile);
}


/*-------------------------------------------------------------------
    Local: Case_Flush_Run()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_Flush_Run(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    DCLFSFILEHANDLE             hFile;
    D_BUFFER                   *pWriteBuffer = NULL;
    void                       *pTemp;
    D_UINT32                    ulValue;
    D_UINT32                    ulIndex;
    D_UINT32                    ulBufferSize = IO_BUFFER_SIZE;
    D_UINT32                    ulTransferred;
    DCLSTATUS                   dclStat;
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;

    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    /*  Allocate a write buffer
    */
    pWriteBuffer = DclMemAlloc(ulBufferSize);
    if(!pWriteBuffer)
        return DCLSTAT_MEMALLOCFAILED;

    /*  Serialize the write data
    */
    pTemp = pWriteBuffer;
    ulValue = pCaseInfo->nThreadNum;
    for(ulIndex = 0; ulIndex < IO_BUFFER_SIZE; ulIndex += sizeof(ulValue))
    {
        COPYNATIVETOLE(pTemp, &ulValue);
        ulValue += 7;
    }

    dclStat = PRIMCREATE(pCaseInfo->pszBaseName, &hFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = PRIMWRITE(hFile, pWriteBuffer, ulBufferSize, &ulTransferred);
    if((dclStat != DCLSTAT_SUCCESS) || (ulTransferred != ulBufferSize))
    {
        /*  Ignore any error from this so the original error code is preserved
        */
        PRIMCLOSE(hFile);

        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;
        else
            return DCLSTAT_FS_WRITEFAILED;
    }

    dclStat = PRIMFLUSH(hFile);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        PRIMCLOSE(hFile);
        return dclStat;
    }

    dclStat = PRIMCLOSE(hFile);

    /*  Release the write buffer
    */
    DclMemFree(pWriteBuffer);

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: Case_Flush_Clean()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_Flush_Clean(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;
    
    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    return PRIMDELETE(pCaseInfo->pszBaseName);
}


/*-------------------------------------------------------------------
    Local: Case_Stat_Init()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_Stat_Init(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    DCLFSFILEHANDLE             hFile;
    DCLSTATUS                   dclStat;
    D_UINT32                    ulTransferred;
    char                        cBuffer = 'a';
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;

    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    dclStat = PRIMCREATE(pCaseInfo->pszBaseName, &hFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = PRIMWRITE(hFile, &cBuffer, 1, &ulTransferred);
    if((dclStat != DCLSTAT_SUCCESS) || (ulTransferred != 1))
    {
        /*  Ignore any error from this so the original error code is preserved
        */
        PRIMCLOSE(hFile);

        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;
        else
            return DCLSTAT_FS_WRITEFAILED;
    }

    return PRIMCLOSE(hFile);
}


/*-------------------------------------------------------------------
    Local: Case_Stat_Run()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_Stat_Run(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    DCLFSSTAT                   sStat;
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;

    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    return PRIMSTAT(pCaseInfo->pszBaseName, &sStat);
}


/*-------------------------------------------------------------------
    Local: Case_Stat_Clean()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_Stat_Clean(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;
    
    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    return PRIMDELETE(pCaseInfo->pszBaseName);
}


/*-------------------------------------------------------------------
    Local: Case_StatFs_Init()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_StatFs_Init(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;
    
    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    return PRIMDIRCREATE(pCaseInfo->pszBaseName);
}


/*-------------------------------------------------------------------
    Local: Case_StatFs_Run()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_StatFs_Run(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    DCLFSSTATFS                 sStatFs;
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;

    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    return PRIMSTATFS(pCaseInfo->pszBaseName, &sStatFs);
}


/*-------------------------------------------------------------------
    Local: Case_StatFs_Clean()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_StatFs_Clean(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;
    
    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    return PRIMDIRREMOVE(pCaseInfo->pszBaseName);
}


/*-------------------------------------------------------------------
    Local: Case_Rename_Init()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_Rename_Init(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    DCLFSFILEHANDLE             hFile;
    DCLSTATUS                   dclStat;
    char                        cBuffer = 'a';
    D_UINT32                    ulTransferred;
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;

    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    dclStat = PRIMCREATE(pCaseInfo->pszBaseName, &hFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = PRIMWRITE(hFile, &cBuffer, 1, &ulTransferred);
    if((dclStat != DCLSTAT_SUCCESS) || (ulTransferred != 1))
    {
        /*  Ignore any error from this so the original error code is preserved
        */
        PRIMCLOSE(hFile);

        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;
        else
            return DCLSTAT_FS_WRITEFAILED;
    }

    return PRIMCLOSE(hFile);
}


/*-------------------------------------------------------------------
    Local: Case_Rename_Run()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_Rename_Run(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    char                        szFullNewPathName[MAX_FILESPEC_LEN];
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;

    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    DclSNPrintf(szFullNewPathName,
            DCLDIMENSIONOF(szFullNewPathName),
            "%s-new",
            pCaseInfo->pszBaseName);

    return PRIMRENAME(pCaseInfo->pszBaseName, szFullNewPathName);
}


/*-------------------------------------------------------------------
    Local: Case_Rename_Clean()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_Rename_Clean(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    char                        szFullPathName[MAX_FILESPEC_LEN];
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;

    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    DclSNPrintf(szFullPathName,
            DCLDIMENSIONOF(szFullPathName),
            "%s-new",
            pCaseInfo->pszBaseName);

    return PRIMDELETE(szFullPathName);
}


/*-------------------------------------------------------------------
    Local: Case_ReadDir_Init()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_ReadDir_Init(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    char                        szFullPathName[MAX_FILESPEC_LEN];
    DCLFSFILEHANDLE             hFile;
    DCLSTATUS                   dclStat;
    char                        cBuffer = 'a';
    D_UINT32                    ulTransferred;
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;

    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    dclStat = PRIMDIRCREATE(pCaseInfo->pszBaseName);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    DclSNPrintf(szFullPathName,
            DCLDIMENSIONOF(szFullPathName),
            "%s%ctopendir",
            pCaseInfo->pszBaseName, DCL_PATHSEPCHAR);

    dclStat = PRIMCREATE(szFullPathName, &hFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = PRIMWRITE(hFile, &cBuffer, 1, &ulTransferred);
    if((dclStat != DCLSTAT_SUCCESS) || (ulTransferred != 1))
    {
        /*  Ignore any error from this so the original error code is preserved
        */
        PRIMCLOSE(hFile);

        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;
        else
            return DCLSTAT_FS_WRITEFAILED;
    }

    return PRIMCLOSE(hFile);
}


/*-------------------------------------------------------------------
    Local: Case_ReadDir_Run()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_ReadDir_Run(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    char                        szName[MAX_FILESPEC_LEN];
    DCLFSDIRHANDLE              hDir;
    DCLSTATUS                   dclStat;
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;

    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    dclStat = PRIMDIROPEN(pCaseInfo->pszBaseName, &hDir);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = PRIMDIRREAD(hDir, szName, DCLDIMENSIONOF(szName), NULL);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        PRIMDIRCLOSE(hDir);
        return dclStat;
    }

    return PRIMDIRCLOSE(hDir);
}


/*-------------------------------------------------------------------
    Local: Case_ReadDir_Clean()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_ReadDir_Clean(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    char                        szFullPathName[MAX_FILESPEC_LEN];
    DCLSTATUS                   dclStat;
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;

    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    DclSNPrintf(szFullPathName,
            DCLDIMENSIONOF(szFullPathName),
            "%s%ctopendir",
            pCaseInfo->pszBaseName, DCL_PATHSEPCHAR);

    dclStat = PRIMDELETE(szFullPathName);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    return PRIMDIRREMOVE(pCaseInfo->pszBaseName);
}


/*-------------------------------------------------------------------
    Local: Case_TreeDel_Init()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_TreeDel_Init(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;
    
    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    return PRIMDIRCREATE(pCaseInfo->pszBaseName);
}


/*-------------------------------------------------------------------
    Local: Case_TreeDel_Run()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_TreeDel_Run(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    #define                     TREE_LEVELS         (4) 
    #define                     FILES_PER_DIR       (3)
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;
    DCLSTATUS                   dclStat;
    DCLTIMESTAMP                ts;
    TREEDELINFO                *pTDI;
    unsigned                    nVerbosity = DCL_VERBOSE_QUIET;

    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    pTDI = DclMemAllocZero(sizeof(*pTDI));
    if(!pTDI)
        return DCLSTAT_MEMALLOCFAILED;

    pTDI->pDataBuff = DclMemAlloc(pTI->pTI->ulFSBlockSize * 4);
    if(!pTDI->pDataBuff)
    {
        DclMemFree(pTDI);
        return DCLSTAT_MEMALLOCFAILED;
    }

    DclStrCpy(pTDI->szPathBuff, pCaseInfo->pszBaseName);
    pTDI->pSTI = &pTI->pTI->sti;
    pTDI->pFSPrim = &pTI->FSPrim;
    pTDI->nDataBuffLen = pTI->pTI->ulFSBlockSize * 4;
    pTDI->nMaxLevel = TREE_LEVELS;
    pTDI->nFiles = FILES_PER_DIR;

    DclStrCat(pTDI->szPathBuff, DCL_PATHSEPSTR);
        
    /*  Create a directory to test in
    */
    dclStat = DclTestDirCreateAndAppend(&pTI->pTI->sti, &pTI->FSPrim, pTDI->szPathBuff, sizeof(pTDI->szPathBuff), "TREE", UINT_MAX);
    if(dclStat != DCLSTAT_SUCCESS)
        goto Cleanup;

    dclStat = TreeCreateRecurse(pTDI, 2);
    if(dclStat != DCLSTAT_SUCCESS)
        goto Cleanup;

    if(pTI->pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
    {
        char szScaleBuff[16];
        
        DclPrintf("  Created %lU files in a %u level tree in %s seconds\n",
            pTDI->ulFilesCreated, TREE_LEVELS,
            DclRatio64(szScaleBuff, sizeof(szScaleBuff), pTDI->ullTimeUS, 1000000, 1));
    }
    
    if(pTI->pTI->sti.nVerbosity > DCL_VERBOSE_QUIET)
        nVerbosity = pTI->pTI->sti.nVerbosity - 1;

    ts = DclTimeStamp();    
    
    dclStat = PRIMDIRTREEDEL(pTDI->szPathBuff, nVerbosity);
    if(dclStat != DCLSTAT_SUCCESS)
        goto Cleanup;

    if(pTI->pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
    {
        char szScaleBuff[16];
        
        DclPrintf("  Deleted the directory tree in %s seconds\n",
            DclRatio64(szScaleBuff, sizeof szScaleBuff, DclTimePassedUS(ts), 1000000, 1));
    }
    
    dclStat = DclTestDirRemoveFromPath(pTDI->szPathBuff);    

  Cleanup:

    DclMemFree(pTDI->pDataBuff);

    DclMemFree(pTDI);

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: Case_TreeDel_Clean()

    Parameters:
        pCaseInfo - A pointer to the DLTFSSTRESSCASEINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Case_TreeDel_Clean(
    const DLTFSSTRESSCASEINFO  *pCaseInfo)
{
    FSSTRESSTHREADINFO         *pTI = pCaseInfo->pTC;
    
    DclAssertReadPtr(pCaseInfo, sizeof(*pCaseInfo));
    DclAssert(pCaseInfo->pszBaseName);

    return PRIMDIRREMOVE(pCaseInfo->pszBaseName);
}


/*-------------------------------------------------------------------
    Local: TreeCreateRecurse()

    Recursively create a directory tree of the specified dimensions.

    ToDo: Fix the relationship between nLevel and pTDI->nMaxLevels,
          as it is non-intuitive.

    Parameters:
        pTDI   - A pointer to the TREEDELINFO structure to use.
        nLevel - The current directory level

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS TreeCreateRecurse(
    TREEDELINFO        *pTDI,
    unsigned            nLevel)
{
    unsigned            nn;
    DCLSTATUS           dclStat = DCLSTAT_SUCCESS;
    D_UINT32            ulCreateUS = 0; /* Initialized only to satisfy picky compiler */
    D_UINT32            ulMaxUS;

    DclAssertWritePtr(pTDI, sizeof(*pTDI));
    DclAssert(nLevel <= pTDI->nMaxLevel + 1);
    DclAssert(pTDI->nFiles);
    
    dclStat = CreateFileSet(pTDI->pSTI, pTDI->pFSPrim, pTDI->szPathBuff, "TreeA", pTDI->nFiles,
                            &ulCreateUS, &ulMaxUS, pTDI->pDataBuff, pTDI->nDataBuffLen, -1);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    pTDI->ulFilesCreated += pTDI->nFiles;
    pTDI->ullTimeUS += ulCreateUS;
    
    if(pTDI->pSTI->nVerbosity > DCL_VERBOSE_LOUD)
    {
        DclPrintf("    Required %4lU us to create %u files in directory \"%s\"\n", 
            ulCreateUS, pTDI->nFiles, pTDI->szPathBuff);
    }
 
    if(nLevel <= pTDI->nMaxLevel)
    {
        for(nn = 1; nn <= nLevel; nn++)
        {
            dclStat = DclTestDirCreateAndAppend(pTDI->pSTI, pTDI->pFSPrim, pTDI->szPathBuff, sizeof(pTDI->szPathBuff), "LEV", UINT_MAX);
            if(dclStat != DCLSTAT_SUCCESS)
                break;

            dclStat = TreeCreateRecurse(pTDI, nLevel+1);
            if(dclStat != DCLSTAT_SUCCESS)
                break;

            dclStat = DclTestDirRemoveFromPath(pTDI->szPathBuff);    
            if(dclStat != DCLSTAT_SUCCESS)
                break;
        }
    }
   
    return dclStat;
}


/*-------------------------------------------------------------------
    Local: CreateFileSet()

    Create a set of files in the specified directory.

    Note the special case if lBlocks is negative.  In this event, the
    value of nDataBuffLen is treated as a maximum, and a random size
    from 1 to that number is chosen.  For example, if nDataBuffLen is
    4KB and lBlocks is -1, then a single block ranging in size from
    1 to 4096 is written to the file.  If nDataBuffLen is 8KB and 
    lBlocks is -3, then a random block size from 1 to 8192 is chosen,
    and 3 of them are written to the file.  When multiple files are 
    being written to a set, each file will (likely) be a different
    size.

    Parameters:
        pSTI         - The DCLSHAREDTESTINFO structure to use.
        pFSPrim      - The FSPRIMCONTROL structure to use.
        pszPath      - The path to use -- must have a trailing separator.
        pszPrefix    - The filename prefix to use.
        nCount       - The number of files to create.
        pulTotalUS   - The location in which to store the total 
                       microseconds required to create the file set.
        pulMaxUS     - The location in which to sore the maximum number
                       of microseconds required to create any one file.
        pDataBuff    - The data buffer to use for writing.
        nDataBuffLen - The size of nDataBuffLen.
        lBlocks      - If positive, this is the number of nDataBuffLen
                       sized blocks to write.  If negative, this causes
                       nDataBuffLen to be treated as a maximum number 
                       of bytes, and that size times the absolute value
                       of lBlocks is written.  See the note above.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS CreateFileSet(
    DCLSHAREDTESTINFO  *pSTI,
    FSPRIMCONTROL      *pFSPrim,
    const char         *pszPath,
    const char         *pszPrefix,
    unsigned            nCount,
    D_UINT32           *pulTotalUS,
    D_UINT32           *pulMaxUS,
    D_BUFFER           *pDataBuff,
    size_t              nDataBuffLen,
    D_INT32             lBlocks)
{
    unsigned            kk;
    D_UINT32            ulTotalUS = 0;
    D_UINT32            ulMaxUS = 0;

    DclAssertWritePtr(pSTI, sizeof(*pSTI));
    DclAssertReadPtr(pszPrefix, 0);
    DclAssertWritePtr(pulTotalUS, sizeof(*pulTotalUS));
    DclAssertWritePtr(pulMaxUS, sizeof(*pulMaxUS));
    DclAssert(nCount);
    DclAssert(lBlocks);
    DclAssert(nDataBuffLen);

    for(kk = 0; kk < nCount; kk++)
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulUS;
        DCLSTATUS       dclStat;
        D_UINT32        ulBlocks = (D_UINT32)lBlocks;
        size_t          nThisBlockSize = nDataBuffLen;

        /*  Special case if lBlocks is negative.  Set the block size for
            to a random value from 1 to ulBlockSize.
        */            
        if(lBlocks < 0)
        {
            nThisBlockSize = (D_UINT32)((DclRand64(&pSTI->ullRandomSeed) % nDataBuffLen) + 1);
            ulBlocks = -lBlocks;
        }

        ts = DclTimeStamp();

        dclStat = FileCreate(pSTI, pFSPrim, pszPath, pszPrefix, kk, pDataBuff, nThisBlockSize, ulBlocks, FALSE);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("    FSIO: Error creating file set. SetSize=%u BlockCount=%lU BlockSize=%u FileNum=%u Error=%lX\n",
                 nCount, ulBlocks, nThisBlockSize, kk, dclStat);

            return dclStat;
        }

        ulUS = DclTimePassedUS(ts);
        ulTotalUS += ulUS;
        if(ulMaxUS < ulUS)
            ulMaxUS = ulUS;

        if(pSTI->nVerbosity > DCL_VERBOSE_LOUD)
            DclPrintf("    File %u create required %4lU us\n", kk, ulUS);
    }

    *pulTotalUS = ulTotalUS;
    *pulMaxUS = ulMaxUS;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: FileCreate()

    Create a file file with a numbered name in the specified directory.

    Parameters:
        pSTI         - The DCLSHAREDTESTINFO structure to use.
        pFSPrim      - The FSPRIMCONTROL structure to use.
        pszPath      - The path to use -- must have a trailing separator.
        pszPrefix    - The filename prefix to use.
        nFileNum     - The file number to use
        pDataBuff    - The data buffer to use for writing.
        nDataBuffLen - The size of nDataBuffLen.
        ulBlocks     - The number of nDataBuffLen sized blocks to write.
        fFlush       - TRUE to flush before closing the file.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS FileCreate(
    const DCLSHAREDTESTINFO    *pSTI,
    FSPRIMCONTROL              *pFSPrim,
    const char                 *pszPath,
    const char                 *pszPrefix,
    unsigned                    nFileNum,
    D_BUFFER                   *pDataBuff,
    size_t                      nDataBuffLen,
    D_UINT32                    ulBlocks,
    D_BOOL                      fFlush)
{
    char                        szFileSpec[MAX_FILESPEC_LEN];
    size_t                      kk;
    DCLFSFILEHANDLE             hFile;
    DCLSTATUS                   dclStat;

    DclAssertReadPtr(pSTI, sizeof(*pSTI));
    DclAssertReadPtr(pszPath, 0);
    DclAssertReadPtr(pszPrefix, 0);
    DclAssertWritePtr(pDataBuff, nDataBuffLen);
    DclAssert(nDataBuffLen);
    DclAssert(ulBlocks);

    DclSNPrintf(szFileSpec, sizeof(szFileSpec), "%s%s%05u.DAT", pszPath, pszPrefix, nFileNum);

    dclStat = DclTestFSPrimCreate(pFSPrim, szFileSpec, &hFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    /*  Generate a data pattern.  
    */
    for(kk = 0; kk < nDataBuffLen; kk++)
        pDataBuff[kk] = kk % 256;

    while(ulBlocks--)
    {
        D_UINT32 ulWritten;
        
        dclStat = DclTestFSPrimWrite(pFSPrim, hFile, pDataBuff, nDataBuffLen, &ulWritten);
        if(dclStat != DCLSTAT_SUCCESS || ulWritten != nDataBuffLen)
        {
            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = DCLSTAT_FS_WRITEFAILED;
                
            if(DclTestFSPrimClose(pFSPrim, hFile) != DCLSTAT_SUCCESS)
                DclPrintf("    Unable to close file after write error\n");

            return dclStat;
        }
    }

    if(fFlush)
    {
        if(pSTI->nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("Created file %s with flush\n", szFileSpec);
        
        dclStat = DclTestFSPrimFlush(pFSPrim, hFile);
        if(dclStat != DCLSTAT_SUCCESS)
            DclPrintf("    Unable to flush file, Status=%lX\n", dclStat);

        /*  Ignore the error code and try to close the file anyway...
        */
    }
    
    dclStat = DclTestFSPrimClose(pFSPrim, hFile);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("    Unable to close file, Status=%lX\n", dclStat);
    }
    
    return dclStat;
}


/*-------------------------------------------------------------------
    Local: RandomizeBuffer()

    Parameters:

    Return Value:
        None.
-------------------------------------------------------------------*/
static void RandomizeBuffer(
    void       *pBuffer,
    D_UINT32    ulBuffSize,
    D_UINT64   *pullSeed)
{
    D_UINT32   *pulData = pBuffer;
    D_UINT32    nn;

    DclAssert(DCLISALIGNED((D_UINTPTR)pBuffer, sizeof(D_UINT32)));

    for(nn = 0; nn < ulBuffSize / sizeof(D_UINT32); nn++)
    {
        *pulData = (D_UINT32)DclRand64(pullSeed);
        pulData++;    
    }

    return;
}



#endif /* DCL_OSFEATURE_THREADS */

