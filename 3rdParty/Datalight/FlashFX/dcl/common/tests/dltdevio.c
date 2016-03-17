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

    This module contains block device I/O performance test.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltdevio.c $
    Revision 1.19  2011/04/16 03:54:53Z  garyp
    Corrected to use the proper header.
    Revision 1.18  2010/06/16 23:53:58Z  jeremys
    Added forward allocator pattern.
    Revision 1.17  2010/06/09 18:21:10Z  jeremys
    Updated the StepperIteration stub implementation to initialize
    pullSectors and pullTimeUS.
    Revision 1.16  2010/06/09 03:30:43Z  jeremys
    Disabled stepper code by default until portability issues are resolved.
    Revision 1.15  2010/06/08 23:01:59Z  jeremys
    Fixed some input handling bugs.
    Revision 1.14  2010/06/08 08:02:10Z  jeremys
    Added stepper test.
    Revision 1.13  2010/05/22 21:32:15Z  garyp
    Added that ability to specify the offset in KB, MB, or GB, rather than 
    sectors only.  Updated the random test to always write relative sector
    0 first to provide more deterministic results when used with media which
    has a "sliding window" style of region.  Adjusted the output to scale
    better for very slow media.
    Revision 1.12  2010/04/28 23:31:29Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.11  2009/11/05 02:16:07Z  garyp
    Updated to terminate immediately if the DCL instance handle is not
    initialized.  Fixed to free the stats buffer, if any.  Updated to use the
    shared test documentation, where appropriate.
    Revision 1.10  2009/10/19 22:01:27Z  garyp
    Updated to use DclRand64(), and to support true 64-bit sector addressing.
    Modified the /PASSES and /TIME options to accept a value of '0', meaning
    to run forever.  Clarified the documentation.
    Revision 1.9  2009/10/08 22:08:53Z  garyp
    Corrected GetKBPerSecond() to avoid a divide-by-zero error on periods
    smaller than half a second.  Improved result accuracy as well on very
    small time periods.
    Revision 1.8  2009/10/01 01:32:14Z  garyp
    Updated to build cleanly.
    Revision 1.7  2009/09/30 21:54:30Z  garyp
    Changed the default TIME value to 30 seconds in discrete mode and 10 
    seconds when in /DLPERF mode.  Changed the range count to double each
    time by default.  Added the /INC:n option to allow the range count to
    increment by an arbitrary amount.  Changed the /DLPERF max sector count
    to 128.  Reduced the verbosity at level 2.  Added PerfLog support for
    gather CSV statistics.  Updated to track the longest time for any given
    I/O operation within each sample.
    Revision 1.6  2009/09/24 21:53:55Z  garyp
    Updated to accommodate the case where the lack of a discard interface is
    not detected until an attempt is made to use it.
    Revision 1.5  2009/09/20 07:24:13Z  keithg
    Added NoConfirm option for automation purposes.
    Revision 1.4  2009/09/15 23:28:01Z  garyp
    Added a discard test.  Replaced the /LIMIT parameter with the /PASSES
    parameter, for simpler operation.
    Revision 1.3  2009/09/14 16:23:23Z  glenns
    Fix compilation issues with gcc.
    Revision 1.2  2009/09/14 02:51:27Z  keithg
    Removed multi-byte initializer since some compilers do not support it.
    Revision 1.1  2009/09/10 16:59:20Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dltools.h>
#include <dlperflog.h>
#include <dlservice.h>
#include <dllog.h>
#include <dlosblockdev.h>
#include "dltshared.h"

/*  Default test characteristics
*/
#define DEFAULT_TEST_SECONDS            (30) /* Default seconds to run some tests */
#define MAX_DEVNAME_LEN                (128)
#define DLPERF_RANDOM_SEED        (12345678UL)
#define DLPERF_VERBOSITY  (DCL_VERBOSE_LOUD)
#define DLPERF_TEST_SECONDS             (10)
#define DLPERF_MINIOSECTORS              (1)
#define DLPERF_MAXIOSECTORS            (128)
#define SECTOR_SIGNATURE        (0x4456494F) /* 'DVIO' in hex */
#define DEFAULT_SAMPLE_RATE              (2)

typedef struct
{
    DCLSHAREDTESTINFO   sti;
    DCLTOOLPARAMS      *pTP;
    const char         *pszCmdName;
    D_BUFFER           *pBuffer;
    D_UINT32            ulSectorOffset;     /* Sectors to skip at the beginning of the device */
    D_UINT32            ulKBOffset;         /* KB to skip at the beginning of the device - to convert to sectors */
    D_UINT32            ulSectorSkip;       /* Skip/increment n sectors in SEQ test */
    D_UINT32            ulMinIOSectors;     /* Start of the IO range */
    D_UINT32            ulMaxIOSectors;     /* End of the IO range */
    D_UINT32            ulMaxSizeKB;        /* Max space to touch in KB */
    D_UINT32            ulPasses;           /* Number of passes per iteration */
    D_UINT64            ullMaxSectors;      /* Max space to touch in sectors */
    D_UINT32            ulSampleSecs;       /* Time in seconds for each sample */
    char                szDevName[MAX_DEVNAME_LEN];
    DCLOSBLOCKDEVHANDLE hBlockDev;
    DCLOSBLOCKDEVINFO   DevInfo;
    unsigned            nIncrement;
    unsigned            fRandomIO       : 1;
    unsigned            fRandRead       : 1;
    unsigned            fRandWrite      : 1;
    unsigned            fRandDiscard    : 1;
    unsigned            fSequentialIO   : 1;
    unsigned            fSeqRead        : 1;
    unsigned            fSeqWrite       : 1;
    unsigned            fSeqDiscard     : 1;
    unsigned            fStepper        : 1;
    unsigned            fVerify         : 1;
    unsigned            fNoConfirm      : 1;
} DEVIOTESTINFO;

typedef struct
{
    D_UINT32            ulCRC;          /* Must be first */
    D_UINT32            ulSignature;    /* Must be second */
    D_UINT64            ullSectorNum;
    D_UINT32            ulPassNum;
} SECTORINF;


static void      ShowHelp(const DCLTOOLPARAMS *pTP);
static D_BOOL    ProcessParams(         DEVIOTESTINFO *pTI, const char *pszCmdName, const char *pszCmdLine);
static DCLSTATUS RunTests(              DEVIOTESTINFO *pTI);
static DCLSTATUS SeqReadIteration(      DEVIOTESTINFO *pTI, D_UINT32 ulCount, D_UINT64 *pullTimeUS, D_UINT64 *pullSectors);
static DCLSTATUS SeqWriteIteration(     DEVIOTESTINFO *pTI, D_UINT32 ulCount, D_UINT64 *pullTimeUS, D_UINT64 *pullSectors);
static DCLSTATUS SeqDiscardIteration(   DEVIOTESTINFO *pTI, D_UINT32 ulCount, D_UINT64 *pullTimeUS, D_UINT64 *pullSectors);
static DCLSTATUS RandomReadIteration(   DEVIOTESTINFO *pTI, D_UINT32 ulCount, D_UINT64 *pullTimeUS, D_UINT64 *pullSectors);
static DCLSTATUS RandomWriteIteration(  DEVIOTESTINFO *pTI, D_UINT32 ulCount, D_UINT64 *pullTimeUS, D_UINT64 *pullSectors);
static DCLSTATUS RandomDiscardIteration(DEVIOTESTINFO *pTI, D_UINT32 ulCount, D_UINT64 *pullTimeUS, D_UINT64 *pullSectors);
static DCLSTATUS StepperIteration(      DEVIOTESTINFO *pTI, D_UINT32 ulCount, D_UINT64 *pullTimeUS, D_UINT64 *pullSectors);
static D_BOOL    VerifySectors(         DEVIOTESTINFO *pTI, D_UINT64 ullSector, D_UINT32 ulCount);
static D_UINT32  GetKBPerSecond(        D_UINT32 ulBytesPerSector, D_UINT64 ullSectors, D_UINT64 ullUS);


/*-------------------------------------------------------------------
    Protected: DclTestDevIOMain()

    Parameters:
        pTP - A pointer to the DCLTOOLPARAMS structure to use

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclTestDevIOMain(
    DCLTOOLPARAMS          *pTP)
{
    DCLSTATUS               dclStat;
    DEVIOTESTINFO           ti = {{0}};
    DCLOSBLOCKDEVACCESS     nMode = DCLOSBLOCKDEVACCESS_LOWLIMIT;

    /*  Ensure the DCL instance is already initialized before bothering
        to do anything else.
    */
    if(!pTP->hDclInst)
        return DCLSTAT_INST_NOTINITIALIZED;

    ti.sti.hDclInst = pTP->hDclInst;
    ti.pszCmdName = pTP->pszCmdName;
    ti.pTP = pTP;

    /*  If the command line parameters are not valid (or if
        it was a help request), return a failure code.
    */
    if(!ProcessParams(&ti, pTP->pszCmdName, pTP->pszCmdLine))
        return DCLSTAT_FAILURE;

    /*  Never used for this test
    */
    ti.sti.fReqFSDisabled = TRUE;

    /*  Calculate the proper access modes and WARN if writing or discarding.
        We <must> do this before we open the log file, as the "press enter"
        message could be hidden if using a log file, and the output is not
        shadowed to the screen.
    */
    if(ti.fSeqRead || ti.fRandRead || ti.fStepper)
        nMode = DCLOSBLOCKDEVACCESS_READ;

    if(ti.fSeqWrite || ti.fRandWrite || ti.fSeqDiscard || ti.fRandDiscard || ti.fStepper)
    {
        if(nMode == DCLOSBLOCKDEVACCESS_READ)
            nMode = DCLOSBLOCKDEVACCESS_READWRITE;
        else
            nMode = DCLOSBLOCKDEVACCESS_WRITE;

        DclPrintf("\n----> WARNING!  This test will irreversibly destroy any existing <----\n");
        DclPrintf("                data on the media \"%s\".\n", ti.szDevName);

        if(!ti.fNoConfirm)
        {
            if(!DclTestPressEnter(ti.sti.hDclInst))
                return DCLSTAT_USERABORT;
        }
    }

    /*  Open any requested log file.
    */
    dclStat = DclTestLogOpen(&ti.sti);
    if(dclStat != DCLSTAT_SUCCESS && dclStat != DCLSTAT_FEATUREDISABLED)
        goto MainCleanup;

    DclPrintf("DCL Block Device I/O Performance Test\n");
    DclSignOn(FALSE);

    /*  Create a block device instance
    */
    dclStat = DclOsBlockDevCreate(ti.szDevName, nMode, 0, &ti.hBlockDev, &ti.DevInfo);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Unable to open block device \"%s\", status %lX\n", ti.szDevName, dclStat);
        goto MainCleanup;
    }

    ti.sti.pszDeviceName = &ti.szDevName[0];

    if(ti.ulKBOffset)
    {
        D_UINT64 ullByteOffset = ((D_UINT64)ti.ulKBOffset) * 1024;
        
        /*  If the offset was specified in KB/MB/GB, then the sector 
            offset count MUST be zero, or we messed up somewhere in
            the parameter parsing.
        */  
        DclAssert(!ti.ulSectorOffset);

        if(ullByteOffset % ti.DevInfo.ulBytesPerSector)
        {
            DclPrintf("The specified offset %lU KB does not fall on a sector boundary\n", ti.ulKBOffset);
            dclStat = DCLSTAT_OUTOFRANGE;
            goto MainCleanup;
        }

        ti.ulSectorOffset = (D_UINT32)(ullByteOffset / ti.DevInfo.ulBytesPerSector);
    }

    if(ti.ulSectorOffset >= ti.DevInfo.ullTotalSectors)
    {
        DclPrintf("The sector offset %lU is too large for the block device\n", ti.ulSectorOffset);
        dclStat = DCLSTAT_OUTOFRANGE;
        goto MainCleanup;
    }

    if(ti.ulSectorOffset + ti.ulMaxIOSectors > ti.DevInfo.ullTotalSectors)
    {
        DclPrintf("The sector offset plus max IO sectors (%lU) is too large for the device\n", ti.ulSectorOffset + ti.ulMaxIOSectors);
        dclStat = DCLSTAT_OUTOFRANGE;
        goto MainCleanup;
    }

    /*  Convert the MAX value to a sector count, and make sure the range is OK.
    */
    ti.ullMaxSectors = DclMulDiv(ti.ulMaxSizeKB, 1024, ti.DevInfo.ulBytesPerSector);
    if(ti.ullMaxSectors == 0 || ti.ullMaxSectors > ti.DevInfo.ullTotalSectors - ti.ulSectorOffset)
    {
        /*  MAX can be anything from 1 to the number of sectors on the disk,
            after adjusting for the starting offset.
        */
        ti.ullMaxSectors = ti.DevInfo.ullTotalSectors - ti.ulSectorOffset;
    }

    ti.pBuffer = DclMemAlloc(ti.ulMaxIOSectors * ti.DevInfo.ulBytesPerSector);
    if(!ti.pBuffer)
    {
        dclStat = DCLSTAT_MEMALLOCFAILED;
        goto MainCleanup;
    }

    if(!ti.DevInfo.pfnDiscard && (ti.fSeqDiscard || ti.fRandDiscard))
    {
        DclPrintf("The block device interface does not support discards.  Skipping discard tests.\n");
        ti.fSeqDiscard = FALSE;
        ti.fRandDiscard = FALSE;
    }

    /*  Seed the pseudo-random number generator
    */
    if(!ti.sti.ullRandomSeed)
        ti.sti.ullRandomSeed = DclOsTickCount();

    DclTestRequestorOpen(&ti.sti, NULL, ti.szDevName);

    /*  Print out the test header
    */
    DclPrintf("Test Parameters:\n");
    DclPrintf("  Test Cases:        %s%s\n",
        ti.fRandomIO            ? " /RAND"   : "",
        ti.fSequentialIO        ? " /SEQ"    : "");

    DclPrintf("  Device Info for:   %24s\n",                        ti.szDevName);
    DclPrintf("    Total Sectors:                 %10llU\n",        ti.DevInfo.ullTotalSectors);
    DclPrintf("    Sector Size:                   %10lU B\n",       ti.DevInfo.ulBytesPerSector);
    DclPrintf("  Sector Offset (/Offset):         %10lU\n",         ti.ulSectorOffset);
    DclPrintf("  Max Sectors to Touch (/Max):     %10llU\n",        ti.ullMaxSectors);
    if(ti.ulPasses)
        DclPrintf("  Passes (/Passes):                %10lU\n",     ti.ulPasses);
    else
        DclPrintf("  Passes (/Passes):                   Forever\n");
    if(ti.sti.ulTestSeconds)
        DclPrintf("  Max Time (/Time):                %10lU seconds\n", ti.sti.ulTestSeconds);
    else
        DclPrintf("  Max Time (/Time):                   Forever\n");
    DclPrintf("  Sample Rate (/Sample):           %10lU seconds\n", ti.ulSampleSecs);
    DclPrintf("  Min Sectors per Operation (/C):  %10lU\n",         ti.ulMinIOSectors);
    DclPrintf("  Max Sectors per Operation (/C):  %10lU\n",         ti.ulMaxIOSectors);
    DclPrintf("  SEQ Sectors to Skip (/Skip):     %10lU\n",         ti.ulSectorSkip);
    DclPrintf("  Random Seed:                     %10llU\n",        ti.sti.ullRandomSeed);

    /*-------------------------------------------------------------------
        Write data to the performance log, if enabled.  Note that any
        changes to the test name or category must be accompanied by
        changes to perfdevio.bat.  Any changes to the actual data fields
        recorded here requires changes to the various spreadsheets which
        track this data.
    -------------------------------------------------------------------*/
    {
        DCLPERFLOGHANDLE    hPerfLog;
        hPerfLog = DCLPERFLOG_OPEN(ti.sti.fPerfLog, ti.sti.hLog, "DEVIO", "Configuration", NULL, ti.sti.szPerfLogSuffix);
        DCLPERFLOG_STRING(hPerfLog, "DevName",      ti.szDevName);
        DCLPERFLOG_NUM64( hPerfLog, "DevSectors",   ti.DevInfo.ullTotalSectors);
        DCLPERFLOG_NUM(   hPerfLog, "SectorSize",   ti.DevInfo.ulBytesPerSector);
        DCLPERFLOG_NUM(   hPerfLog, "Offset",       ti.ulSectorOffset);
        DCLPERFLOG_NUM64( hPerfLog, "MaxSecs",      ti.ullMaxSectors);
        DCLPERFLOG_NUM(   hPerfLog, "MaxTime",      ti.sti.ulTestSeconds);
        DCLPERFLOG_NUM(   hPerfLog, "Passes",       ti.ulPasses);
        DCLPERFLOG_NUM(   hPerfLog, "MinSecOp",     ti.ulMinIOSectors);
        DCLPERFLOG_NUM(   hPerfLog, "MaxSecOp",     ti.ulMaxIOSectors);
        DCLPERFLOG_NUM(   hPerfLog, "SkipSecs",     ti.ulSectorSkip);
        DCLPERFLOG_NUM64( hPerfLog, "RandomSeed",   ti.sti.ullRandomSeed);
        DCLPERFLOG_WRITE( hPerfLog);
        DCLPERFLOG_CLOSE( hPerfLog);
    }

    /*  Call the function which will call the individual test routines
    */
    dclStat = RunTests(&ti);

  MainCleanup:

    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("\n%s: Error %lX\n\n", pTP->pszCmdName, dclStat);
    else
        DclPrintf("%s completed successfully\n\n", pTP->pszCmdName);

    DclTestRequestorClose(&ti.sti);

    if(ti.pBuffer)
        DclMemFree(ti.pBuffer);

    if(ti.hBlockDev)
        DclOsBlockDevDestroy(ti.hBlockDev);

    DclTestLogClose(&ti.sti);

    if(ti.sti.pStatsBuff)
        DclMemFree(ti.sti.pStatsBuff);

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: ShowHelp()

    Show the usage of the program.

    Parameters:
        pTP - A pointer to the DCLTOOLPARAMS structure to use.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void ShowHelp(
    const DCLTOOLPARAMS    *pTP)
{
    DclPrintf("Usage:  %s DevName tests [options]\n\n", pTP->pszCmdName);
    DclPrintf("Where:\n");
    if(pTP->pszDriveForms)
        DclPrintf("  DevName        The OS specific device name to use in the form \"%s\".\n\n", pTP->pszDriveForms);
    else
        DclPrintf("  DevName        The device name to use.\n\n");
    DclPrintf("And 'tests' are one or more of the following:\n");
    DclPrintf("  /DLPERF        Run the Datalight standard performance tests (see description)\n");
    DclPrintf("  /RAND[:r|w|d]  Run the random read/write/discard device I/O tests\n");
    DclPrintf("  /SEQ[:r|w|d]   Run the sequential read/write/discard device I/O tests\n\n");
    DclPrintf("Basic Options:\n");
    DclPrintf("  /?             Display this help information\n");
    DclPrintf("  /??            Display an extended test description for all tests\n");
    DclPrintf("  /TestName /?   Display an extended test description for the specified test\n");
    DclPrintf("  /Offset:sects  The sector Offset at which to start.  This may also be specified \n");
    DclPrintf("                 as a KB, MB, or GB value if followed by the appropriate suffix.\n");
    DclPrintf("                 Any offsets specified in this fashion must fall on sector\n");
    DclPrintf("                 boundaries.  The default offset is 0.\n");
    DclPrintf("  /C:sects[:max] The Count of sectors per I/O operation (default is 1).  If a \n");
    DclPrintf("                 Max sectors value is provided, then the test will iterate\n");
    DclPrintf("                 and double the count each time (use /Inc to test every size).\n");
    DclPrintf("  /Max:size      The Maximum amount of data to touch, starting at the sector\n");
    DclPrintf("                 specified with /Offset.  If not specified, the test use all\n");
    DclPrintf("                 the space remaining space on the device.\n");
    DclPrintf("  /Passes:n      Specifies the number of passes to perform over the data range\n");
    DclPrintf("                 specified by /Max (per iteration).  The default is 1.  Specify\n");
    DclPrintf("                 0 to cause the passes to be treated as infinite -- the iteration\n");
    DclPrintf("                 will quit only when the time limit has been reached.\n");
    DclPrintf("  /Time:n[s|m]   Specifies the amount of Time (in Seconds or Minutes) to run\n");
    DclPrintf("                 each iteration (default is %u seconds).  Specify 0 to cause\n", DEFAULT_TEST_SECONDS);
    DclPrintf("                 time to be ignored -- the iteration will quit when the passes\n");
    DclPrintf("                 limit has been reached.\n\n");
    DclPrintf("Each test (or each iteration of each test, if /C specifies a range) will run\n");
    DclPrintf("until either the time has expired, or until the amount of data specified by the\n");
    DclPrintf("combination of the /Max and /Passes parameters has been touched.  Setting both\n");
    DclPrintf("/Time and /Passes to 0 will cause the test to run forever\n");
    if(!DclTestPressEnter(pTP->hDclInst))
        return;
    DclPrintf("\n");
    DclPrintf("Advanced Options:\n");
    DclPrintf("  /Skip:sects    The number of sectors to Skip when running the /SEQ test.  The\n");
    DclPrintf("                 default is 0 which results in a contiguous linear operation.\n");
    DclPrintf("  /Verify        Caused the sequential read test to verify that the sector data\n");
    DclPrintf("                 read matches that which was written.\n");
    DclPrintf("  /Sample:rate   Specifies the sample rate in seconds at which intermediate\n");
    DclPrintf("                 results will be displayed.  The default is %u, 0 to disable.\n", DEFAULT_SAMPLE_RATE);
    DclPrintf("  /Inc:n         When /C specifies a range of sectors, this specifies that the\n");
    DclPrintf("                 count will increase by 'n', rather than doubling each time.\n");
    DclPrintf("  /NoConfirm     Skips user confirmation before initiating destructive writes.\n");
  #if DCLCONF_OUTPUT_ENABLED
    DLTSHAREDTESTHELP_LOG();
    DLTSHAREDTESTHELP_LB();
    DLTSHAREDTESTHELP_LS();
    DLTSHAREDTESTHELP_PERFLOG();
    DLTSHAREDTESTHELP_STATS();
    DLTSHAREDTESTHELP_TRACE();
  #endif
    DLTSHAREDTESTHELP_PROF();
    DclPrintf("  /Req:off       Disable the external requestor interface.  \"off\" is the only\n");
    DclPrintf("                 valid option.\n");
    DLTSHAREDTESTHELP_SEED();
    DLTSHAREDTESTHELP_VERBOSITY();
     if(!DclTestPressEnter(pTP->hDclInst))
        return;
    DclPrintf("\n");
    DclPrintf("Notes: 1) Options which have a 'size' designation may be specified in hex (0x)\n");
    DclPrintf("          or decimal, and may be suffixed by 'KB', 'MB', or 'GB'.  If there is\n");
    DclPrintf("          no scale designated, then KB is assumed.\n");
    DclPrintf("       2) For consistent test results (especially on flash media), each test\n");
    DclPrintf("          iteration MUST be individually run on a freshly formatted disk.\n");
    DclPrintf("       3) Test names and options are not case sensitive or order sensitive.\n");

    return;
}


/*-------------------------------------------------------------------
    Local: HelpDLPerfTest()

    This function displays an extended test description.

    Parameters:
        None.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void HelpDLPerfTest(void)
{
    DclPrintf("  /DLPERF\n");
    DclPrintf("    This option causes a subset set of tests to be run using a predefined\n");
    DclPrintf("    set of options.  Using this option causes the following options to be\n");
    DclPrintf("    used:\n\n");
    DclPrintf("        /SEQ\n");
    DclPrintf("        /RAND\n");
    DclPrintf("        /C:1:128\n");
    DclPrintf("        /TIME:10s\n");
    DclPrintf("        /VERIFY\n");
    DclPrintf("        /SEED:12345678\n");
    DclPrintf("        /V:2\n\n");
    DclPrintf("    Additional options may be specified which can override or supplement\n");
    DclPrintf("    these settings.  Individual test types can be disabled by specifying\n");
    DclPrintf("    the switch with a trailing '-'.\n");

    return;
}


/*-------------------------------------------------------------------
    Local: HelpSequentialTest()

    This function displays an extended test description.

    Parameters:
        None.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void HelpSequentialTest(void)
{
    DclPrintf("  /SEQ[:r|w|d]\n");
    DclPrintf("    This test exercises sequential I/O performance.  Specify 'r', 'w', or\n");
    DclPrintf("    'd' to cause only the Read, Write, or Discard portion of the test to run.\n");
    DclPrintf("    Specify 'r-', 'w-', or 'd-' to prevent the Read, Write, or Discard portion\n");
    DclPrintf("    of the test from running.\n\n");

    return;
}


/*-------------------------------------------------------------------
    Local: HelpRandomTest()

    This function displays an extended test description.

    Parameters:
        None.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void HelpRandomTest(void)
{
    DclPrintf("  /RAND[:r|w|d]\n");
    DclPrintf("    This test exercises random I/O performance.  Specify 'r', 'w', or 'd' to\n");
    DclPrintf("    cause only the Read, Write, or Discard portion of the test to run.  Specify\n");
    DclPrintf("    'r-', 'w-', or 'd-' to prevent the Read, Write, or Discard portion of the\n");
    DclPrintf("    test from running.\n\n");

    return;
}


/*-------------------------------------------------------------------
    Local: DisplayTestDescription()

    This function displays an extended description for each
    test.

    Parameters:
        hDclInst - The DCL instance handle.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void DisplayTestDescription(
    DCLINSTANCEHANDLE   hDclInst)
{
    DclPrintf("\n                          Extended Test Description\n\n");
    DclPrintf("The Block Device I/O test is designed to test block device performance.\n\n");
    DclPrintf("The following sections describe the various tests:\n");
    DclPrintf("\n");

    HelpDLPerfTest();
    if(!DclTestPressEnter(hDclInst))
        return;
    DclPrintf("\n");

    HelpRandomTest();
    if(!DclTestPressEnter(hDclInst))
        return;
    DclPrintf("\n");

    HelpSequentialTest();
    if(!DclTestPressEnter(hDclInst))
        return;
    DclPrintf("\n");

    return;
}


/*-------------------------------------------------------------------
    Local: ProcessParams()

    This function parses the command line items and configures
    the test as specified.

    Parameters:
        pTI        - A pointer to the DEVIOTESTINFO structure to use
        pszCmdName - The name of the test program
        pszCmdLine - The test command line

    Return Value:
        TRUE if successful, else FALSE.  In the event that FALSE is
        returned any necessary error messages or help display will
        already have been displayed.
-------------------------------------------------------------------*/
static D_BOOL ProcessParams(
    DEVIOTESTINFO  *pTI,
    const char     *pszCmdName,
    const char     *pszCmdLine)
{
    #define         ARGBUFFLEN  (128)
    D_UINT16        uIndex;
    char            achArgBuff[ARGBUFFLEN];
    D_UINT16        uArgCount;

    /*  Init default settings
    */
    pTI->sti.nVerbosity     = DCL_VERBOSE_NORMAL;
    pTI->sti.ulTestSeconds  = DEFAULT_TEST_SECONDS;
    pTI->ulSampleSecs       = DEFAULT_SAMPLE_RATE;
    pTI->ulPasses           = 1;

    uArgCount = DclArgCount(pszCmdLine);
    if(!uArgCount)
    {
        ShowHelp(pTI->pTP);
        return FALSE;
    }

    /*  Start with argument number 1
    */
    for(uIndex = 1; uIndex <= uArgCount; uIndex++)
    {
        char    achArgHelp[8];

        if(!DclArgRetrieve(pszCmdLine, uIndex, sizeof(achArgBuff), achArgBuff))
        {
            DclPrintf("Bad argument!\n\n");
            ShowHelp(pTI->pTP);
            return FALSE;
        }

                /*-----------------------------------------*\
                 *                                         *
                 *       Process required args first       *
                 *                                         *
                \*-----------------------------------------*/

        if(DclStrICmp(achArgBuff, "/DLPERF") == 0)
        {
            if((DclArgRetrieve(pszCmdLine, (D_UINT16)(uIndex+1), sizeof(achArgHelp), achArgHelp)
                && (DclStrCmp(achArgHelp, "/?") == 0)))
            {
                HelpDLPerfTest();
                return FALSE;
            }

            pTI->fSequentialIO = TRUE;
            pTI->fSeqRead = TRUE;
            pTI->fSeqWrite = TRUE;
            pTI->fSeqDiscard = TRUE;
            pTI->fRandomIO = TRUE;
            pTI->fRandRead = TRUE;
            pTI->fRandWrite = TRUE;
            pTI->fRandDiscard = TRUE;
            pTI->fVerify = TRUE;
            pTI->sti.ullRandomSeed  = DLPERF_RANDOM_SEED;
            pTI->sti.nVerbosity     = DLPERF_VERBOSITY;
            pTI->sti.ulTestSeconds  = DLPERF_TEST_SECONDS;
            pTI->ulMinIOSectors     = DLPERF_MINIOSECTORS;
            pTI->ulMaxIOSectors     = DLPERF_MAXIOSECTORS;
            continue;
        }
        else if((DclStrNICmp(achArgBuff, "/RAND", 5) == 0))
        {
            if((DclArgRetrieve(pszCmdLine, (D_UINT16)(uIndex+1), sizeof(achArgHelp), achArgHelp)
                && (DclStrCmp(achArgHelp, "/?") == 0)))
            {
                HelpRandomTest();
                return FALSE;
            }

            pTI->fRandomIO = TRUE;

            if(achArgBuff[5] == ':')
            {
                if(DclToUpper(achArgBuff[6]) == 'R')
                {
                    if(achArgBuff[7] == '-' && achArgBuff[8] == 0 && pTI->fRandRead)
                        pTI->fRandRead = FALSE;
                    else if(achArgBuff[7] == 0)
                        pTI->fRandRead = TRUE;
                    else
                        goto BadOption;
                }
                else if(DclToUpper(achArgBuff[6]) == 'W')
                {
                    if(achArgBuff[7] == '-' && achArgBuff[8] == 0 && pTI->fRandWrite)
                        pTI->fRandWrite = FALSE;
                    else if(achArgBuff[7] == 0)
                        pTI->fRandWrite = TRUE;
                    else
                        goto BadOption;
                }
                else if(DclToUpper(achArgBuff[6]) == 'D')
                {
                    if(achArgBuff[7] == '-' && achArgBuff[8] == 0 && pTI->fRandDiscard)
                        pTI->fRandDiscard = FALSE;
                    else if(achArgBuff[7] == 0)
                        pTI->fRandDiscard = TRUE;
                    else
                        goto BadOption;
                }
                else
                {
                    goto BadOption;
                }

                /*  If both random read and write are, or have become, FALSE,
                    turn off the random I/O test entirely.
                */
                if(!pTI->fRandRead && !pTI->fRandWrite && !pTI->fRandDiscard)
                    pTI->fRandomIO = FALSE;
            }
            else if(achArgBuff[5] == '-' && achArgBuff[6] == 0 && pTI->fRandomIO)
            {
                pTI->fRandomIO = FALSE;
                pTI->fRandRead = FALSE;
                pTI->fRandWrite = FALSE;
                pTI->fRandDiscard = FALSE;
            }
            else if(achArgBuff[5] == 0)
            {
                pTI->fRandRead = TRUE;
                pTI->fRandWrite = TRUE;
                pTI->fRandDiscard = TRUE;
            }
            else
            {
                goto BadOption;
            }

            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/SEQ", 4) == 0)
        {
            if((DclArgRetrieve(pszCmdLine, (D_UINT16)(uIndex+1), sizeof(achArgHelp), achArgHelp)
                && (DclStrCmp(achArgHelp, "/?") == 0)))
            {
                HelpSequentialTest();
                return FALSE;
            }

            pTI->fSequentialIO = TRUE;

            if(achArgBuff[4] == ':')
            {
                if(DclToUpper(achArgBuff[5]) == 'R')
                {
                    if(achArgBuff[6] == '-' && achArgBuff[7] == 0 && pTI->fSeqRead)
                        pTI->fSeqRead = FALSE;
                    else if(achArgBuff[6] == 0)
                        pTI->fSeqRead = TRUE;
                    else
                        goto BadOption;
                }
                else if(DclToUpper(achArgBuff[5]) == 'W')
                {
                    if(achArgBuff[6] == '-' && achArgBuff[7] == 0 && pTI->fSeqWrite)
                        pTI->fSeqWrite = FALSE;
                    else if(achArgBuff[6] == 0)
                        pTI->fSeqWrite = TRUE;
                    else
                        goto BadOption;
                }
                else if(DclToUpper(achArgBuff[5]) == 'D')
                {
                    if(achArgBuff[6] == '-' && achArgBuff[7] == 0 && pTI->fSeqDiscard)
                        pTI->fSeqDiscard = FALSE;
                    else if(achArgBuff[6] == 0)
                        pTI->fSeqDiscard = TRUE;
                    else
                        goto BadOption;
                }
                else
                {
                    goto BadOption;
                }

                /*  If all the sequential I/O categories are, or have become,
                    FALSE, turn off the sequential I/O test entirely.
                */
                if(!pTI->fSeqRead && !pTI->fSeqWrite && !pTI->fSeqDiscard)
                    pTI->fSequentialIO = FALSE;
            }
            else if(achArgBuff[4] == '-' && achArgBuff[5] == 0 && pTI->fSequentialIO)
            {
                pTI->fSequentialIO = FALSE;
                pTI->fSeqRead = FALSE;
                pTI->fSeqWrite = FALSE;
                pTI->fSeqDiscard = FALSE;
            }
            else if(achArgBuff[4] == 0)
            {
                pTI->fSeqRead = TRUE;
                pTI->fSeqWrite = TRUE;
                pTI->fSeqDiscard = TRUE;
            }
            else
            {
                goto BadOption;
            }

            continue;
        }
        else if((DclStrNICmp(achArgBuff, "/STEP", 5) == 0))
        {
            pTI->fStepper = TRUE;
        }

                /*-----------------------------------------*\
                 *                                         *
                 *    Test type args complete.             *
                 *    Process optional arguments next.     *
                 *                                         *
                \*-----------------------------------------*/

        else if(DclStrNICmp(achArgBuff, "/C:", 3) == 0)
        {
            char   *pStr = &achArgBuff[3];

            pTI->ulMinIOSectors = DclAtoL(pStr);
            if(!pTI->ulMinIOSectors)
            {
                DclPrintf("The count of sectors must be at least 1\n");
                return FALSE;
            }

            while(DclIsDigit(*pStr))
                pStr++;

            if(*pStr ==':')
            {
                pTI->ulMaxIOSectors = DclAtoL(++pStr);

                if(pTI->ulMaxIOSectors <= pTI->ulMinIOSectors)
                {
                    DclPrintf("The max count of sectors must be greater than the minimum count\n");
                    return FALSE;
                }
            }
            else
            {
                pTI->ulMaxIOSectors = pTI->ulMinIOSectors;
            }

            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/OFFSET:", 8) == 0)
        {
            const char *pszStr = &achArgBuff[8];
            D_BOOL      fIsSectorCount = TRUE;

            if(!*pszStr || !DclIsDigit(*pszStr))
                goto BadOption;

            if(DclStrNICmp(pszStr, "0x", 2) == 0)
                fIsSectorCount = FALSE;

            while(DclIsDigit(*pszStr))
                pszStr++;

            if(*pszStr)
                fIsSectorCount = FALSE;

            if(fIsSectorCount)
            {
                pTI->ulSectorOffset = DclAtoL(&achArgBuff[8]);
            }
            else
            {
                /*  The offset is in KB for the moment.  After we
                    determine the geometry, we'll convert it to
                    a sector offset.
                */
                pszStr = DclSizeToULKB(&achArgBuff[8], &pTI->ulKBOffset);
                if(!pszStr || *pszStr)
                    goto BadSyntaxOverflow;
            }

            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/SKIP:", 6) == 0)
        {
            if(!pTI->fSequentialIO)
            {
                DclPrintf("The /SKIP option is only valid with the /SEQ test\n");
                return FALSE;
            }

            pTI->ulSectorSkip = DclAtoL(&achArgBuff[6]);

            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/MAX:", 5) == 0)
        {
            const char *pTmp;

            pTmp = DclSizeToULKB(&achArgBuff[5], &pTI->ulMaxSizeKB);
            if(!pTmp || *pTmp)
                goto BadSyntaxOverflow;

            if(!pTI->ulMaxSizeKB)
            {
                DclPrintf("A /MAX:size value of 0 is not legal\n");
                return FALSE;
            }

            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/PASSES:", 8) == 0)
        {
            pTI->ulPasses = DclAtoL(&achArgBuff[8]);

            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/SAMPLE:", 8) == 0)
        {
            pTI->ulSampleSecs = DclAtoL(&achArgBuff[8]);

            continue;
        }
        else if(DclStrICmp(achArgBuff, "/NOCONFIRM") == 0)
        {
            /*  Option to skip user interraction
            */
            pTI->fNoConfirm = TRUE;
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/INC:", 5) == 0)
        {
            pTI->nIncrement = DclAtoI(&achArgBuff[5]);
            continue;
        }
        else if(DclStrICmp(achArgBuff, "/VERIFY") == 0)
        {
            pTI->fVerify = TRUE;
            continue;
        }
        else if(DclStrICmp(achArgBuff, "/?") == 0)
        {
            ShowHelp(pTI->pTP);
            return FALSE;
        }
        else if(DclStrICmp(achArgBuff, "/??") == 0)
        {
            DisplayTestDescription(pTI->sti.hDclInst);
            return FALSE;
        }
        else
        {
            /*  Process common test arguments which are shared by
                multiple tests.
            */
            if(DclTestParseParam(&pTI->sti, achArgBuff))
                continue;

            if(uIndex == 1)
            {
                DclStrNCpy(pTI->szDevName, achArgBuff, sizeof(pTI->szDevName));
            }
            else
            {
              BadOption:
                DclPrintf("Bad option: \"%s\"\n\n", achArgBuff);
                return FALSE;
              BadSyntaxOverflow:
                DclPrintf("Syntax or overflow error in '%s'\n\n", achArgBuff);
                return FALSE;
            }
        }
    }

    if(!pTI->fRandomIO && !pTI->fSequentialIO && !pTI->fStepper)
    {
        DclPrintf("One or more test types must be specified.\n\n");
        return FALSE;
    }

    if(!pTI->ulMinIOSectors)
        pTI->ulMinIOSectors = 1;

    if(!pTI->ulMaxIOSectors)
        pTI->ulMaxIOSectors = pTI->ulMinIOSectors;

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: RunTests()

    Invoke the various tests, based on the specified command-line
    parameters.

    Parameters:
        pTI        - A pointer to the DEVIOTESTINFO structure to use

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS RunTests(
    DEVIOTESTINFO      *pTI)
{
    unsigned            nn;
    unsigned            nRandomElements;
    D_UINT32            ulSeed;
    SECTORINF          *pSectorInf;
    DCLSTATUS           dclStat = DCLSTAT_BADPARAMETER;
    DCLPERFLOGHANDLE    hPerfLog;
    D_UINT64            ullTimeUS;
    D_UINT64            ullSectors;
    D_UINT32            ulKBperSec;
    D_UINT32            ulIOperSec;
    char                szSize[16];
    D_UINT32            ulCount;

    DclAssertWritePtr(pTI, sizeof(*pTI));

    /*  Initialize the sector data to a unique yet reproducible pattern
    */

    /*  Make a private copy of the current seed so that the original is
        unmodified by this process.  Use only the low 32-bits.
    */
    ulSeed = (D_UINT32)pTI->sti.ullRandomSeed;

    /*  Calculate the number of D_UINT32 random elements in each sector,
        Not including the area use for the SECTORINF structure.  Ensure
        that things are evenly divisible.
    */
    DclAssert(sizeof(SECTORINF) % sizeof(D_UINT32) == 0);
    nRandomElements = (pTI->DevInfo.ulBytesPerSector - sizeof(SECTORINF)) / sizeof(D_UINT32);

    pSectorInf = (SECTORINF*)pTI->pBuffer;

    for(nn=0; nn < pTI->ulMaxIOSectors; nn++)
    {
        unsigned    dd;
        D_UINT32   *pul;

        pSectorInf->ulSignature = SECTOR_SIGNATURE;

        pul = (D_UINT32*)(((char*)pSectorInf) + sizeof(SECTORINF));

        for(dd=0; dd<nRandomElements; dd++)
            *pul++ = DclRand(&ulSeed);

        pSectorInf = (SECTORINF*)(((char*)pSectorInf) + pTI->DevInfo.ulBytesPerSector);
    }

    /*-------------------------------------------------------------------
        Write data to the performance log, if enabled.  Note that any
        changes to the test category names must be accompanied by
        changes to perfdevio.bat.  Any changes to the actual data fields
        recorded here requires changes to the various spreadsheets which
        track this data.
    -------------------------------------------------------------------*/

    if(pTI->fSeqWrite)
    {
        DclPrintf("Starting the sequential write test\n");

        hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "DEVIO", "SeqWrite", NULL, pTI->sti.szPerfLogSuffix);

        for(ulCount = pTI->ulMinIOSectors;
            ulCount <= pTI->ulMaxIOSectors;
            (pTI->nIncrement ? ulCount += pTI->nIncrement : (ulCount<<=1)))
        {
            DclPrintf("  Writing %lU sectors at a time\n", ulCount);

            DclTestInstrumentationStart(&pTI->sti);

            dclStat = SeqWriteIteration(pTI, ulCount, &ullTimeUS, &ullSectors);
            if(dclStat != DCLSTAT_SUCCESS)
                break;

            ulKBperSec = GetKBPerSecond(pTI->DevInfo.ulBytesPerSector, ullSectors, ullTimeUS);
            ulIOperSec = (ulKBperSec * 1024) / (ulCount * pTI->DevInfo.ulBytesPerSector);

            DclPrintf("  Iteration wrote %8llU sectors in %5llU ms -------------> %4lU KB/sec (%lU IOPS)\n",
                ullSectors, (ullTimeUS+500)/1000, ulKBperSec, ulIOperSec);

            DclTestInstrumentationStop(&pTI->sti, "SeqWrite");

            DclSNPrintf(szSize, DCLDIMENSIONOF(szSize), "IOSize-%lU", ulCount);
            DCLPERFLOG_NUM(hPerfLog, szSize, ulKBperSec);
        }

        DCLPERFLOG_WRITE(hPerfLog);
        DCLPERFLOG_CLOSE(hPerfLog);
    }

    if(pTI->fSeqRead)
    {
        DclPrintf("Starting the sequential read test\n");

        hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "DEVIO", "SeqRead", NULL, pTI->sti.szPerfLogSuffix);

        for(ulCount = pTI->ulMinIOSectors;
            ulCount <= pTI->ulMaxIOSectors;
            (pTI->nIncrement ? ulCount += pTI->nIncrement : (ulCount<<=1)))
        {
            DclTestInstrumentationStart(&pTI->sti);

            DclPrintf("  Reading %lU sectors at a time\n", ulCount);

            dclStat = SeqReadIteration(pTI, ulCount, &ullTimeUS, &ullSectors);
            if(dclStat != DCLSTAT_SUCCESS)
                break;

            ulKBperSec = GetKBPerSecond(pTI->DevInfo.ulBytesPerSector, ullSectors, ullTimeUS);
            ulIOperSec = (ulKBperSec * 1024) / (ulCount * pTI->DevInfo.ulBytesPerSector);

            DclPrintf("  Iteration read %9llU sectors in %5llU ms -------------> %4lU KB/sec (%lU IOPS)\n",
                ullSectors, (ullTimeUS+500)/1000, ulKBperSec, ulIOperSec);

            DclTestInstrumentationStop(&pTI->sti, "SeqRead");

            DclSNPrintf(szSize, DCLDIMENSIONOF(szSize), "IOSize-%lU", ulCount);
            DCLPERFLOG_NUM(hPerfLog, szSize, ulKBperSec);
        }

        DCLPERFLOG_WRITE(hPerfLog);
        DCLPERFLOG_CLOSE(hPerfLog);
    }

    if(pTI->fSeqDiscard)
    {
        DclPrintf("Starting the sequential discard test\n");

        hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "DEVIO", "SeqDiscard", NULL, pTI->sti.szPerfLogSuffix);

        for(ulCount = pTI->ulMinIOSectors;
            ulCount <= pTI->ulMaxIOSectors;
            (pTI->nIncrement ? ulCount += pTI->nIncrement : (ulCount<<=1)))
        {
            DclTestInstrumentationStart(&pTI->sti);

            DclPrintf("  Discarding %lU sectors at a time\n", ulCount);

            dclStat = SeqDiscardIteration(pTI, ulCount, &ullTimeUS, &ullSectors);
            if(dclStat != DCLSTAT_SUCCESS)
                break;

            ulKBperSec = GetKBPerSecond(pTI->DevInfo.ulBytesPerSector, ullSectors, ullTimeUS);
            ulIOperSec = (ulKBperSec * 1024) / (ulCount * pTI->DevInfo.ulBytesPerSector);

            DclPrintf("  Iteration discarded %9llU sectors in %5llU ms --------> %4lU KB/sec (%lU IOPS)\n",
                ullSectors, (ullTimeUS+500)/1000, ulKBperSec, ulIOperSec);

            DclTestInstrumentationStop(&pTI->sti, "SeqDisc");

            DclSNPrintf(szSize, DCLDIMENSIONOF(szSize), "IOSize-%lU", ulCount);
            DCLPERFLOG_NUM(hPerfLog, szSize, ulKBperSec);
        }

        DCLPERFLOG_WRITE(hPerfLog);
        DCLPERFLOG_CLOSE(hPerfLog);
    }

    if(pTI->fRandWrite)
    {
        DclPrintf("Starting the random write test\n");

        hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "DEVIO", "RandWrite", NULL, pTI->sti.szPerfLogSuffix);

        for(ulCount = pTI->ulMinIOSectors;
            ulCount <= pTI->ulMaxIOSectors;
            (pTI->nIncrement ? ulCount += pTI->nIncrement : (ulCount<<=1)))
        {
            DclPrintf("  Writing %lU sectors at a time\n", ulCount);

            DclTestInstrumentationStart(&pTI->sti);

            dclStat = RandomWriteIteration(pTI, ulCount, &ullTimeUS, &ullSectors);
            if(dclStat != DCLSTAT_SUCCESS)
                break;

            ulKBperSec = GetKBPerSecond(pTI->DevInfo.ulBytesPerSector, ullSectors, ullTimeUS);
            ulIOperSec = (ulKBperSec * 1024) / (ulCount * pTI->DevInfo.ulBytesPerSector);

            DclPrintf("  Iteration wrote %8llU sectors in %5llU ms -------------> %4lU KB/sec (%lU IOPS)\n",
                ullSectors, (ullTimeUS+500)/1000, ulKBperSec, ulIOperSec);

            DclTestInstrumentationStop(&pTI->sti, "RandWrite");

            DclSNPrintf(szSize, DCLDIMENSIONOF(szSize), "IOSize-%lU", ulCount);
            DCLPERFLOG_NUM(hPerfLog, szSize, ulKBperSec);
        }

        DCLPERFLOG_WRITE(hPerfLog);
        DCLPERFLOG_CLOSE(hPerfLog);
    }

    if(pTI->fRandRead)
    {
        DclPrintf("Starting the random read test\n");

        hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "DEVIO", "RandRead", NULL, pTI->sti.szPerfLogSuffix);

        for(ulCount = pTI->ulMinIOSectors;
            ulCount <= pTI->ulMaxIOSectors;
            (pTI->nIncrement ? ulCount += pTI->nIncrement : (ulCount<<=1)))
        {
            DclPrintf("  Reading %lU sectors at a time\n", ulCount);

            DclTestInstrumentationStart(&pTI->sti);

            dclStat = RandomReadIteration(pTI, ulCount, &ullTimeUS, &ullSectors);
            if(dclStat != DCLSTAT_SUCCESS)
                break;

            ulKBperSec = GetKBPerSecond(pTI->DevInfo.ulBytesPerSector, ullSectors, ullTimeUS);
            ulIOperSec = (ulKBperSec * 1024) / (ulCount * pTI->DevInfo.ulBytesPerSector);

            DclPrintf("  Iteration read %9llU sectors in %5llU ms -------------> %4lU KB/sec (%lU IOPS)\n",
                ullSectors, (ullTimeUS+500)/1000, ulKBperSec, ulIOperSec);

            DclTestInstrumentationStop(&pTI->sti, "RandRead");

            DclSNPrintf(szSize, DCLDIMENSIONOF(szSize), "IOSize-%lU", ulCount);
            DCLPERFLOG_NUM(hPerfLog, szSize, ulKBperSec);
        }

        DCLPERFLOG_WRITE(hPerfLog);
        DCLPERFLOG_CLOSE(hPerfLog);
    }

    if(pTI->fRandDiscard)
    {
        DclPrintf("Starting the random discard test\n");

        hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "DEVIO", "RandDiscard", NULL, pTI->sti.szPerfLogSuffix);

        for(ulCount = pTI->ulMinIOSectors;
            ulCount <= pTI->ulMaxIOSectors;
            (pTI->nIncrement ? ulCount += pTI->nIncrement : (ulCount<<=1)))
        {
            DclPrintf("  Discarding %lU sectors at a time\n", ulCount);

            DclTestInstrumentationStart(&pTI->sti);

            dclStat = RandomDiscardIteration(pTI, ulCount, &ullTimeUS, &ullSectors);
            if(dclStat != DCLSTAT_SUCCESS)
                break;

            ulKBperSec = GetKBPerSecond(pTI->DevInfo.ulBytesPerSector, ullSectors, ullTimeUS);
            ulIOperSec = (ulKBperSec * 1024) / (ulCount * pTI->DevInfo.ulBytesPerSector);

            DclPrintf("  Iteration discarded %9llU sectors in %5llU ms --------> %4lU KB/sec (%lU IOPS)\n",
                ullSectors, (ullTimeUS+500)/1000, ulKBperSec, ulIOperSec);

            DclTestInstrumentationStop(&pTI->sti, "RandDisc");

            DclSNPrintf(szSize, DCLDIMENSIONOF(szSize), "IOSize-%lU", ulCount);
            DCLPERFLOG_NUM(hPerfLog, szSize, ulKBperSec);
        }

        DCLPERFLOG_WRITE(hPerfLog);
        DCLPERFLOG_CLOSE(hPerfLog);
    }

    if(pTI->fStepper)
    {
        DclPrintf("Starting the stepper test\n");

        hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "DEVIO", "Stepper", NULL, pTI->sti.szPerfLogSuffix);

        for(ulCount = pTI->ulMinIOSectors;
            ulCount <= pTI->ulMaxIOSectors;
            (pTI->nIncrement ? ulCount += pTI->nIncrement : (ulCount<<=1)))
        {
            DclPrintf("  MAX BUFFER SIZE: %lu\n", pTI->ulMaxIOSectors * pTI->DevInfo.ulBytesPerSector);

            DclTestInstrumentationStart(&pTI->sti);

            dclStat = StepperIteration(pTI, ulCount, &ullTimeUS, &ullSectors);
            if(dclStat != DCLSTAT_SUCCESS)
                break;

            ulKBperSec = GetKBPerSecond(pTI->DevInfo.ulBytesPerSector, ullSectors, ullTimeUS);
            ulIOperSec = (ulKBperSec * 1024) / (ulCount * pTI->DevInfo.ulBytesPerSector);

            DclPrintf("  Stepper test returned DCLSTAT_SUCCESS.\n",
                ullSectors, (ullTimeUS+500)/1000, ulKBperSec, ulIOperSec);

            DclTestInstrumentationStop(&pTI->sti, "Stepper");

            DclSNPrintf(szSize, DCLDIMENSIONOF(szSize), "IOSize-%lU", ulCount);
            DCLPERFLOG_NUM(hPerfLog, szSize, ulKBperSec);
        }

        DCLPERFLOG_WRITE(hPerfLog);
        DCLPERFLOG_CLOSE(hPerfLog);
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: SeqWriteIteration()

    Parameters:
        pTI         - A pointer to the DEVIOTESTINFO structure to use
        ulCount     - The number of sectors per I/O operation
        pullTimeUS  - A pointer to a variable to receive the total
                      time in microseconds.
        pullSectors - A pointer to a variable to receive the total
                      sectors written.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS SeqWriteIteration(
    DEVIOTESTINFO  *pTI,
    D_UINT32        ulCount,
    D_UINT64       *pullTimeUS,
    D_UINT64       *pullSectors)
{
    D_UINT64        ullTotalUS = 0;
    D_UINT64        ullTotalSects = 0;
    D_UINT32        ulPass = 1;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertWritePtr(pullTimeUS, sizeof(*pullTimeUS));
    DclAssertWritePtr(pullSectors, sizeof(*pullSectors));

    /*  Keep looping while there is both more I/O to perform, based
        on the combination of /MAX and /PASSES, and if there is more
        time remaining.  If either/both of /PASSES and /TIME are zero,
        then that respective factor is considered to be "forever".
    */
    while(((pTI->ulPasses == 0) || (ulPass <= pTI->ulPasses)) &&
        ((pTI->sti.ulTestSeconds == 0) || (ullTotalUS < pTI->sti.ulTestSeconds * 1000000)))
    {
        DCLIOSTATUS ioStat = {0};
        D_UINT64    ullSector;
        D_UINT64    ullPassUS = 0;
        D_UINT64    ullPassSects = 0;
        unsigned    nSample = 1;
        D_UINT64    ullStartSampleUS = 0;
        D_UINT64    ullStartSampleSects = 0;
        D_UINT32    ulLongUS = 0;

        if(pTI->ulPasses != 1 && pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
            DclPrintf("    Starting pass %lU\n", ulPass);

        for(ullSector = 0;
            ullSector + ulCount <= pTI->ullMaxSectors;
            ullSector += ulCount + pTI->ulSectorSkip)
        {
            unsigned        nn;
            DCLTIMESTAMP    ts;
            D_UINT32        ulUS;
            SECTORINF      *pSectorInf = (SECTORINF*)pTI->pBuffer;

            /*  Initialize the sector number, pass number, and CRC for
                each sector.
            */
            for(nn=0; nn < ulCount; nn++)
            {
                pSectorInf->ullSectorNum = pTI->ulSectorOffset + ullSector + nn;
                pSectorInf->ulPassNum = ulPass;
                pSectorInf->ulCRC = DclCrc32Update(0, (const D_BUFFER*)&pSectorInf->ulSignature,
                        pTI->DevInfo.ulBytesPerSector - sizeof(pSectorInf->ulCRC));

                pSectorInf = (SECTORINF*)(((char*)pSectorInf) + pTI->DevInfo.ulBytesPerSector);
            }

            ts = DclTimeStamp();

            ioStat = pTI->DevInfo.pfnWrite(pTI->hBlockDev, pTI->ulSectorOffset + ullSector, ulCount, pTI->pBuffer);

            ulUS = DclTimePassedUS(ts);

            if(ulUS > ulLongUS)
                ulLongUS = ulUS;

            ullPassUS += ulUS;
            ullPassSects += ioStat.ulCount;

            if(ioStat.dclStat != DCLSTAT_SUCCESS || ioStat.ulCount != ulCount)
            {
                DclPrintf("Write at relative sector %llU failed! status=%lX count=%lU\n",
                    ullSector, ioStat.dclStat, ioStat.ulCount);

                if(ioStat.dclStat == DCLSTAT_SUCCESS)
                    dclStat = DCLSTAT_WRITEFAILED;
                else
                    dclStat = ioStat.dclStat;

                goto Cleanup;
            }

            if(pTI->sti.nVerbosity > DCL_VERBOSE_LOUD)
                DclPrintf("        Wrote %lU sectors at relative offset %8llU in %6lU us\n", ulCount, ullSector, ulUS);

            if(pTI->ulSampleSecs && ullPassUS - ullStartSampleUS > pTI->ulSampleSecs * 1000000)
            {
                DclPrintf("      Sample %3u wrote %4llU sectors in %6llU ms (long %lU us)\n",
                    nSample, ullPassSects-ullStartSampleSects, (ullPassUS-ullStartSampleUS+500)/1000, ulLongUS);

                nSample++;
                ullStartSampleSects = ullPassSects;
                ullStartSampleUS = ullPassUS;
                ulLongUS = 0;
            }

            if(pTI->sti.ulTestSeconds && (ullTotalUS + ullPassUS >= pTI->sti.ulTestSeconds * 1000000))
                break;
        }

        if(ullStartSampleSects != ullPassSects)
        {
            DclPrintf("      Sample %3u wrote %4llU sectors in %6llU ms (long %lU us)\n",
                nSample, ullPassSects-ullStartSampleSects, (ullPassUS-ullStartSampleUS+500)/1000, ulLongUS);
        }

        if(pTI->ulPasses != 1)
            DclPrintf("    Pass %2lU wrote %8llU sectors in %6llU ms\n", ulPass, ullPassSects, (ullPassUS+500)/1000);

        ulPass++;
        ullTotalUS += ullPassUS;
        ullTotalSects += ullPassSects;
    }

  Cleanup:

    *pullSectors = ullTotalSects;
    *pullTimeUS = ullTotalUS;

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: SeqReadIteration()

    Parameters:
        pTI         - A pointer to the DEVIOTESTINFO structure to use
        ulCount     - The number of sectors per I/O operation
        pullTimeUS  - A pointer to a variable to receive the total
                      time in microseconds.
        pullSectors - A pointer to a variable to receive the total
                      sectors read.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS SeqReadIteration(
    DEVIOTESTINFO  *pTI,
    D_UINT32        ulCount,
    D_UINT64       *pullTimeUS,
    D_UINT64       *pullSectors)
{
    D_UINT64        ullTotalUS = 0;
    D_UINT64        ullTotalSects = 0;
    D_UINT32        ulPass = 1;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertWritePtr(pullTimeUS, sizeof(*pullTimeUS));
    DclAssertWritePtr(pullSectors, sizeof(*pullSectors));

    /*  Keep looping while there is both more I/O to perform, based
        on the combination of /MAX and /PASSES, and if there is more
        time remaining.  If either/both of /PASSES and /TIME are zero,
        then that respective factor is considered to be "forever".
    */
    while(((pTI->ulPasses == 0) || (ulPass <= pTI->ulPasses)) &&
        ((pTI->sti.ulTestSeconds == 0) || (ullTotalUS < pTI->sti.ulTestSeconds * 1000000)))
    {
        DCLIOSTATUS ioStat = {0};
        D_UINT64    ullSector;
        D_UINT64    ullPassUS = 0;
        D_UINT64    ullPassSects = 0;
        unsigned    nSample = 1;
        D_UINT64    ullStartSampleUS = 0;
        D_UINT64    ullStartSampleSects = 0;
        D_UINT32    ulLongUS = 0;

        if(pTI->ulPasses != 1 && pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
            DclPrintf("    Starting pass %lU\n", ulPass);

        for(ullSector = 0;
            ullSector + ulCount <= pTI->ullMaxSectors;
            ullSector += ulCount + pTI->ulSectorSkip)
        {
            DCLTIMESTAMP    ts;
            D_UINT32        ulUS;

            ts = DclTimeStamp();

            ioStat = pTI->DevInfo.pfnRead(pTI->hBlockDev, pTI->ulSectorOffset + ullSector, ulCount, pTI->pBuffer);

            ulUS = DclTimePassedUS(ts);

            if(ulUS > ulLongUS)
                ulLongUS = ulUS;

            if(pTI->fVerify)
            {
                if(!VerifySectors(pTI, ullSector, ioStat.ulCount))
                {
                    dclStat = DCLSTAT_DATAVERIFYERROR;
                    goto Cleanup;
                }
            }

            ullPassUS += ulUS;
            ullPassSects += ioStat.ulCount;

            if(ioStat.dclStat != DCLSTAT_SUCCESS || ioStat.ulCount != ulCount)
            {
                DclPrintf("Read at relative sector %llU failed! status=%lX count=%lU\n",
                    ullSector, ioStat.dclStat, ioStat.ulCount);

                if(ioStat.dclStat == DCLSTAT_SUCCESS)
                    dclStat = DCLSTAT_READFAILED;
                else
                    dclStat = ioStat.dclStat;

                goto Cleanup;
            }

            if(pTI->sti.nVerbosity > DCL_VERBOSE_LOUD)
                DclPrintf("        Read %lU sectors at relative offset %8llU in %6lU us\n", ulCount, ullSector, ulUS);

            if(pTI->ulSampleSecs && ullPassUS - ullStartSampleUS > pTI->ulSampleSecs * 1000000)
            {
                DclPrintf("      Sample %3u read %5llU sectors in %6llU ms (long %lU us)\n",
                    nSample, ullPassSects-ullStartSampleSects, (ullPassUS-ullStartSampleUS+500)/1000, ulLongUS);

                nSample++;
                ullStartSampleSects = ullPassSects;
                ullStartSampleUS = ullPassUS;
                ulLongUS = 0;
            }

            if(pTI->sti.ulTestSeconds && (ullTotalUS + ullPassUS >= pTI->sti.ulTestSeconds * 1000000))
                break;
        }

        if(ullStartSampleSects != ullPassSects)
        {
            DclPrintf("      Sample %3u read %5llU sectors in %6llU ms (long %lU us)\n",
                nSample, ullPassSects-ullStartSampleSects, (ullPassUS-ullStartSampleUS+500)/1000, ulLongUS);
        }

        if(pTI->ulPasses != 1)
            DclPrintf("    Pass %2lU read %8llU sectors in %6llU ms\n", ulPass, ullPassSects, (ullPassUS+500)/1000);

        ulPass++;
        ullTotalUS += ullPassUS;
        ullTotalSects += ullPassSects;
    }

  Cleanup:

    *pullSectors = ullTotalSects;
    *pullTimeUS = ullTotalUS;

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: SeqDiscardIteration()

    Parameters:
        pTI         - A pointer to the DEVIOTESTINFO structure to use
        ulCount     - The number of sectors per I/O operation
        pullTimeUS  - A pointer to a variable to receive the total
                      time in microseconds.
        pullSectors - A pointer to a variable to receive the total
                      sectors discarded.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS SeqDiscardIteration(
    DEVIOTESTINFO  *pTI,
    D_UINT32        ulCount,
    D_UINT64       *pullTimeUS,
    D_UINT64       *pullSectors)
{
    D_UINT64        ullTotalUS = 0;
    D_UINT64        ullTotalSects = 0;
    D_UINT32        ulPass = 1;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertWritePtr(pullTimeUS, sizeof(*pullTimeUS));
    DclAssertWritePtr(pullSectors, sizeof(*pullSectors));

    /*  Keep looping while there is both more I/O to perform, based
        on the combination of /MAX and /PASSES, and if there is more
        time remaining.  If either/both of /PASSES and /TIME are zero,
        then that respective factor is considered to be "forever".
    */
    while(((pTI->ulPasses == 0) || (ulPass <= pTI->ulPasses)) &&
        ((pTI->sti.ulTestSeconds == 0) || (ullTotalUS < pTI->sti.ulTestSeconds * 1000000)))
    {
        DCLIOSTATUS ioStat = {0};
        D_UINT64    ullSector;
        D_UINT64    ullPassUS = 0;
        D_UINT64    ullPassSects = 0;
        unsigned    nSample = 1;
        D_UINT64    ullStartSampleUS = 0;
        D_UINT64    ullStartSampleSects = 0;
        D_UINT32    ulLongUS = 0;

        if(pTI->ulPasses != 1 && pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
            DclPrintf("    Starting pass %lU\n", ulPass);

        for(ullSector = 0;
            ullSector + ulCount <= pTI->ullMaxSectors;
            ullSector += ulCount + pTI->ulSectorSkip)
        {
            DCLTIMESTAMP    ts;
            D_UINT32        ulUS;

            ts = DclTimeStamp();

            ioStat = pTI->DevInfo.pfnDiscard(pTI->hBlockDev, pTI->ulSectorOffset + ullSector, ulCount);

            ulUS = DclTimePassedUS(ts);

            if(ulUS > ulLongUS)
                ulLongUS = ulUS;

            ullPassUS += ulUS;
            ullPassSects += ioStat.ulCount;

            if(ioStat.dclStat != DCLSTAT_SUCCESS || ioStat.ulCount != ulCount)
            {
                if(ioStat.dclStat == DCLSTAT_UNSUPPORTED)
                {
                    DclPrintf("The media does not support a discard interface, skipping...\n");
                }
                else
                {
                    DclPrintf("Discard at relative sector %llU failed! status=%lX count=%lU\n",
                        ullSector, ioStat.dclStat, ioStat.ulCount);

                    if(ioStat.dclStat == DCLSTAT_SUCCESS)
                        dclStat = DCLSTAT_DISCARDFAILED;
                    else
                        dclStat = ioStat.dclStat;
                }

                goto Cleanup;
            }

            if(pTI->sti.nVerbosity > DCL_VERBOSE_LOUD)
                DclPrintf("        Discarded %lU sectors at relative offset %8llU in %6lU us\n", ulCount, ullSector, ulUS);

            if(pTI->ulSampleSecs && ullPassUS - ullStartSampleUS > pTI->ulSampleSecs * 1000000)
            {
                DclPrintf("      Sample %3u discarded %4llU sectors in %6llU ms (long %lU us)\n",
                    nSample, ullPassSects-ullStartSampleSects, (ullPassUS-ullStartSampleUS+500)/1000, ulLongUS);

                nSample++;
                ullStartSampleSects = ullPassSects;
                ullStartSampleUS = ullPassUS;
                ulLongUS = 0;
            }

            if(pTI->sti.ulTestSeconds && (ullTotalUS + ullPassUS >= pTI->sti.ulTestSeconds * 1000000))
                break;
        }

        if(ullStartSampleSects != ullPassSects)
        {
            DclPrintf("      Sample %3u discarded %4llU sectors in %6llU ms (long %lU us)\n",
                nSample, ullPassSects-ullStartSampleSects, (ullPassUS-ullStartSampleUS+500)/1000, ulLongUS);
        }

        if(pTI->ulPasses != 1)
            DclPrintf("    Pass %2lU discarded %8llU sectors in %6llU ms\n", ulPass, ullPassSects, (ullPassUS+500)/1000);

        ulPass++;
        ullTotalUS += ullPassUS;
        ullTotalSects += ullPassSects;
    }

  Cleanup:

    *pullSectors = ullTotalSects;
    *pullTimeUS = ullTotalUS;

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: RandomWriteIteration()

    For the RANDOM test, the concept of "PASSES" is a bit different
    than for the SEQUENTIAL test.  A single pass is when the equivalent
    amount of data specified by /MAX has been processed, regardless of
    where it was performed on the disk.

    Parameters:
        pTI         - A pointer to the DEVIOTESTINFO structure to use
        ulCount     - The number of sectors per I/O operation
        pullTimeUS  - A pointer to a variable to receive the total
                      time in microseconds.
        pullSectors - A pointer to a variable to receive the total
                      sectors written.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS RandomWriteIteration(
    DEVIOTESTINFO  *pTI,
    D_UINT32        ulCount,
    D_UINT64       *pullTimeUS,
    D_UINT64       *pullSectors)
{
    D_UINT64        ullTotalUS = 0;
    D_UINT64        ullTotalSects = 0;
    D_UINT32        ulPass = 1;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    unsigned        nSample = 1;
    D_UINT64        ullStartSampleUS = 0;
    D_UINT64        ullStartSampleSects = 0;
    D_UINT64        ullLimitSectors;
    D_UINT32        ulLongUS = 0;

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertWritePtr(pullTimeUS, sizeof(*pullTimeUS));
    DclAssertWritePtr(pullSectors, sizeof(*pullSectors));
    DclAssert(pTI->ullMaxSectors >= ulCount);

    ullLimitSectors = pTI->ullMaxSectors * pTI->ulPasses;

    /*  Keep looping while there is both more I/O to perform, based
        on the combination of /MAX and /PASSES, and if there is more
        time remaining.  If either/both of /PASSES and /TIME are zero,
        then that respective factor is considered to be "forever".
    */
    while(((ullLimitSectors == 0) || (ullTotalSects + ulCount <= ullLimitSectors)) &&
        ((pTI->sti.ulTestSeconds == 0) || (ullTotalUS < pTI->sti.ulTestSeconds * 1000000)))
    {
        DCLIOSTATUS     ioStat = {0};
        D_UINT64        ullSector;
        unsigned        nn;
        DCLTIMESTAMP    ts;
        D_UINT32        ulUS;
        SECTORINF      *pSectorInf = (SECTORINF*)pTI->pBuffer;

        /*  The very first random sector is not random at all, but is
            always relative sector 0.  This ensures that any target
            media which implements a "sliding window" style of region,
            is deterministically handled.
        */            
        if(ulPass != 1 && pTI->ullMaxSectors > ulCount)
            ullSector = DclRand64(&pTI->sti.ullRandomSeed) % (pTI->ullMaxSectors - ulCount);
        else
            ullSector = 0;

        /*  Always do the I/O on boundaries which are aligned with ulCount.
        */
        ullSector = (ullSector / ulCount) * ulCount;

        /*  Initialize the sector number, pass number, and CRC for
            each sector.
        */
        for(nn=0; nn < ulCount; nn++)
        {
            pSectorInf->ullSectorNum = pTI->ulSectorOffset + ullSector + nn;
            pSectorInf->ulPassNum = ulPass;
            pSectorInf->ulCRC = DclCrc32Update(0, (const D_BUFFER*)&pSectorInf->ulSignature,
                    pTI->DevInfo.ulBytesPerSector - sizeof(pSectorInf->ulCRC));

            pSectorInf = (SECTORINF*)(((char*)pSectorInf) + pTI->DevInfo.ulBytesPerSector);
        }

        ts = DclTimeStamp();

        ioStat = pTI->DevInfo.pfnWrite(pTI->hBlockDev, pTI->ulSectorOffset + ullSector, ulCount, pTI->pBuffer);

        ulUS = DclTimePassedUS(ts);

        if(ulUS > ulLongUS)
            ulLongUS = ulUS;

        ullTotalUS += ulUS;
        ullTotalSects += ioStat.ulCount;

        if(ioStat.dclStat != DCLSTAT_SUCCESS || ioStat.ulCount != ulCount)
        {
            DclPrintf("Write at relative sector %llU failed! status=%lX count=%lU\n",
                ullSector, ioStat.dclStat, ioStat.ulCount);

            if(ioStat.dclStat == DCLSTAT_SUCCESS)
                dclStat = DCLSTAT_WRITEFAILED;
            else
                dclStat = ioStat.dclStat;

            goto Cleanup;
        }

        if(pTI->sti.nVerbosity > DCL_VERBOSE_LOUD)
            DclPrintf("        Wrote %lU sectors at relative offset %8llU in %6lU us\n", ulCount, ullSector, ulUS);

        if(pTI->ulSampleSecs && ullTotalUS - ullStartSampleUS > pTI->ulSampleSecs * 1000000)
        {
            DclPrintf("      Sample %3u wrote %4llU sectors in %6llU ms (long %lU us)\n",
                nSample, ullTotalSects-ullStartSampleSects, (ullTotalUS-ullStartSampleUS+500)/1000, ulLongUS);

            nSample++;
            ullStartSampleSects = ullTotalSects;
            ullStartSampleUS = ullTotalUS;
            ulLongUS = 0;
        }

        ulPass++;
    }

    if(ullStartSampleSects != ullTotalSects)
    {
        /*  There may be a partial sample at the end of the test
        */
        DclPrintf("      Sample %3u wrote %4llU sectors in %6llU ms (long %lU us)\n",
            nSample, ullTotalSects-ullStartSampleSects, (ullTotalUS-ullStartSampleUS+500)/1000, ulLongUS);
    }

  Cleanup:

    *pullSectors = ullTotalSects;
    *pullTimeUS = ullTotalUS;

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: RandomReadIteration()

    For the RANDOM test, the concept of "PASSES" is a bit different
    than for the SEQUENTIAL test.  A single pass is when the equivalent
    amount of data specified by /MAX has been processed, regardless of
    where it was performed on the disk.

    Parameters:
        pTI         - A pointer to the DEVIOTESTINFO structure to use
        ulCount     - The number of sectors per I/O operation
        pullTimeUS  - A pointer to a variable to receive the total
                      time in microseconds.
        pullSectors - A pointer to a variable to receive the total
                      sectors read.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS RandomReadIteration(
    DEVIOTESTINFO  *pTI,
    D_UINT32        ulCount,
    D_UINT64       *pullTimeUS,
    D_UINT64       *pullSectors)
{
    D_UINT64        ullTotalUS = 0;
    D_UINT64        ullTotalSects = 0;
    D_UINT32        ulPass = 1;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    unsigned        nSample = 1;
    D_UINT64        ullStartSampleUS = 0;
    D_UINT64        ullStartSampleSects = 0;
    D_UINT64        ullLimitSectors;
    D_UINT32        ulLongUS = 0;

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertWritePtr(pullTimeUS, sizeof(*pullTimeUS));
    DclAssertWritePtr(pullSectors, sizeof(*pullSectors));
    DclAssert(pTI->ullMaxSectors >= ulCount);

    ullLimitSectors = pTI->ullMaxSectors * pTI->ulPasses;

    /*  Keep looping while there is both more I/O to perform, based
        on the combination of /MAX and /PASSES, and if there is more
        time remaining.  If either/both of /PASSES and /TIME are zero,
        then that respective factor is considered to be "forever".
    */
    while(((ullLimitSectors == 0) || (ullTotalSects + ulCount <= ullLimitSectors)) &&
        ((pTI->sti.ulTestSeconds == 0) || (ullTotalUS < pTI->sti.ulTestSeconds * 1000000)))
    {
        DCLIOSTATUS     ioStat = {0};
        D_UINT64        ullSector;
        DCLTIMESTAMP    ts;
        D_UINT32        ulUS;

        /*  The very first random sector is not random at all, but is
            always relative sector 0.  This ensures that any target
            media which implements a "sliding window" style of region,
            is deterministically handled.
        */            
        if(ulPass != 1 && pTI->ullMaxSectors > ulCount)
            ullSector = DclRand64(&pTI->sti.ullRandomSeed) % (pTI->ullMaxSectors - ulCount);
        else
            ullSector = 0;

        /*  Always do the I/O on boundaries which are aligned with ulCount.
        */
        ullSector = (ullSector / ulCount) * ulCount;

        ts = DclTimeStamp();

        ioStat = pTI->DevInfo.pfnRead(pTI->hBlockDev, pTI->ulSectorOffset + ullSector, ulCount, pTI->pBuffer);

        ulUS = DclTimePassedUS(ts);
/*
            if(pTI->fVerify)
            {
                if(!VerifySectors(pTI, ullSector, ioStat.ulCount))
                {
                    dclStat = DCLSTAT_DATAVERIFYERROR;
                    goto Cleanup;
                }
            }
*/
        if(ulUS > ulLongUS)
            ulLongUS = ulUS;

        ullTotalUS += ulUS;
        ullTotalSects += ioStat.ulCount;

        if(ioStat.dclStat != DCLSTAT_SUCCESS || ioStat.ulCount != ulCount)
        {
            DclPrintf("Read at relative sector %llU failed! status=%lX count=%lU\n",
                ullSector, ioStat.dclStat, ioStat.ulCount);

            if(ioStat.dclStat == DCLSTAT_SUCCESS)
                dclStat = DCLSTAT_READFAILED;
            else
                dclStat = ioStat.dclStat;

            goto Cleanup;
        }

        if(pTI->sti.nVerbosity > DCL_VERBOSE_LOUD)
            DclPrintf("        Read %lU sectors at relative offset %8llU in %6lU us\n", ulCount, ullSector, ulUS);

        if(pTI->ulSampleSecs && ullTotalUS - ullStartSampleUS > pTI->ulSampleSecs * 1000000)
        {
            DclPrintf("      Sample %3u read %5llU sectors in %6llU ms (long %lU us)\n",
                nSample, ullTotalSects-ullStartSampleSects, (ullTotalUS-ullStartSampleUS+500)/1000, ulLongUS);

            nSample++;
            ullStartSampleSects = ullTotalSects;
            ullStartSampleUS = ullTotalUS;
            ulLongUS = 0;
        }

        ulPass++;
    }

    if(ullStartSampleSects != ullTotalSects)
    {
        /*  There may be a partial sample at the end of the test
        */
        DclPrintf("      Sample %3u read %5llU sectors in %6llU ms (long %lU us)\n",
            nSample, ullTotalSects-ullStartSampleSects, (ullTotalUS-ullStartSampleUS+500)/1000, ulLongUS);
    }

  Cleanup:

    *pullSectors = ullTotalSects;
    *pullTimeUS = ullTotalUS;

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: RandomDiscardIteration()

    For the RANDOM test, the concept of "PASSES" is a bit different
    than for the SEQUENTIAL test.  A single pass is when the equivalent
    amount of data specified by /MAX has been processed, regardless of
    where it was performed on the disk.

    Parameters:
        pTI         - A pointer to the DEVIOTESTINFO structure to use
        ulCount     - The number of sectors per I/O operation
        pullTimeUS  - A pointer to a variable to receive the total
                      time in microseconds.
        pullSectors - A pointer to a variable to receive the total
                      sectors discarded.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS RandomDiscardIteration(
    DEVIOTESTINFO  *pTI,
    D_UINT32        ulCount,
    D_UINT64       *pullTimeUS,
    D_UINT64       *pullSectors)
{
    D_UINT64        ullTotalUS = 0;
    D_UINT64        ullTotalSects = 0;
    D_UINT32        ulPass = 1;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    unsigned        nSample = 1;
    D_UINT64        ullStartSampleUS = 0;
    D_UINT64        ullStartSampleSects = 0;
    D_UINT64        ullLimitSectors;
    D_UINT32        ulLongUS = 0;

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertWritePtr(pullTimeUS, sizeof(*pullTimeUS));
    DclAssertWritePtr(pullSectors, sizeof(*pullSectors));
    DclAssert(pTI->ullMaxSectors >= ulCount);

    ullLimitSectors = pTI->ullMaxSectors * pTI->ulPasses;

    /*  Keep looping while there is both more I/O to perform, based
        on the combination of /MAX and /PASSES, and if there is more
        time remaining.  If either/both of /PASSES and /TIME are zero,
        then that respective factor is considered to be "forever".
    */
    while(((ullLimitSectors == 0) || (ullTotalSects + ulCount <= ullLimitSectors)) &&
        ((pTI->sti.ulTestSeconds == 0) || (ullTotalUS < pTI->sti.ulTestSeconds * 1000000)))
    {
        DCLIOSTATUS     ioStat = {0};
        D_UINT64        ullSector;
        DCLTIMESTAMP    ts;
        D_UINT32        ulUS;

        /*  The very first random sector is not random at all, but is
            always relative sector 0.  This ensures that any target
            media which implements a "sliding window" style of region,
            is deterministically handled.
        */            
        if(ulPass != 1 && pTI->ullMaxSectors > ulCount)
            ullSector = DclRand64(&pTI->sti.ullRandomSeed) % (pTI->ullMaxSectors - ulCount);
        else
            ullSector = 0;

        /*  Always do the I/O on boundaries which are aligned with ulCount.
        */
        ullSector = (ullSector / ulCount) * ulCount;

        ts = DclTimeStamp();

        ioStat = pTI->DevInfo.pfnDiscard(pTI->hBlockDev, pTI->ulSectorOffset + ullSector, ulCount);

        ulUS = DclTimePassedUS(ts);

        if(ulUS > ulLongUS)
            ulLongUS = ulUS;

        ullTotalUS += ulUS;
        ullTotalSects += ioStat.ulCount;

        if(ioStat.dclStat != DCLSTAT_SUCCESS || ioStat.ulCount != ulCount)
        {
            if(ioStat.dclStat == DCLSTAT_UNSUPPORTED)
            {
                DclPrintf("The media does not support a discard interface, skipping...\n");
            }
            else
            {
                DclPrintf("Discard at relative sector %llU failed! status=%lX count=%lU\n",
                    ullSector, ioStat.dclStat, ioStat.ulCount);

                if(ioStat.dclStat == DCLSTAT_SUCCESS)
                    dclStat = DCLSTAT_WRITEFAILED;
                else
                    dclStat = ioStat.dclStat;
            }

            goto Cleanup;
        }

        if(pTI->sti.nVerbosity > DCL_VERBOSE_LOUD)
            DclPrintf("        Discard %lU sectors at relative offset %8llU in %6lU us\n", ulCount, ullSector, ulUS);

        if(pTI->ulSampleSecs && ullTotalUS - ullStartSampleUS > pTI->ulSampleSecs * 1000000)
        {
            DclPrintf("      Sample %3u discarded %4llU sectors in %6llU ms (long %lU us)\n",
                nSample, ullTotalSects-ullStartSampleSects, (ullTotalUS-ullStartSampleUS+500)/1000, ulLongUS);

            nSample++;
            ullStartSampleSects = ullTotalSects;
            ullStartSampleUS = ullTotalUS;
            ulLongUS = 0;
        }

        ulPass++;
    }

    if(ullStartSampleSects != ullTotalSects)
    {
        /*  There may be a partial sample at the end of the test
        */
        DclPrintf("      Sample %3u discarded %4llU sectors in %6llU ms (long %lU us)\n",
            nSample, ullTotalSects-ullStartSampleSects, (ullTotalUS-ullStartSampleUS+500)/1000, ulLongUS);
    }

  Cleanup:

    *pullSectors = ullTotalSects;
    *pullTimeUS = ullTotalUS;

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: VerifySectors()

    Verify that the specified range of sectors is valid.  The data
    has already been read into pTI->pBuffer.

    Parameters:

    Return Value:
        Returns TRUE if all sectors in the range are either unwritten
        by DEVIOTEST or are fully valid.  If any sector appears to
        have been written by DEVIOTEST, but does not match, FALSE
        will be returned.
-------------------------------------------------------------------*/
static D_BOOL VerifySectors(
    DEVIOTESTINFO  *pTI,
    D_UINT64        ullSector,
    D_UINT32        ulCount)
{
    D_UINT32        nn;
    SECTORINF      *pSectorInf = (SECTORINF*)pTI->pBuffer;
    D_BOOL          fValid = TRUE;

    DclAssertWritePtr(pTI, sizeof(*pTI));

    /*  Verify the sector number, pass number, and CRC for each sector.
    */
    for(nn=0; nn < ulCount; nn++)
    {
        if(pTI->sti.nVerbosity > DCL_VERBOSE_LOUD)
        {
            DclPrintf("%llU: Signature=0x%lX Sector=%llU Pass=%lU CRC=%lX\n",
                ullSector + nn, pSectorInf->ulSignature, pSectorInf->ullSectorNum,
                pSectorInf->ulPassNum, pSectorInf->ulCRC);
        }

        if(pSectorInf->ulSignature != SECTOR_SIGNATURE)
        {
            if(pTI->sti.nVerbosity > DCL_VERBOSE_LOUD)
            {
                DclPrintf("Sector %llU does not appear to have been written by %s\n",
                    ullSector + nn, pTI->pszCmdName);
            }
        }
        else
        {
            if(pSectorInf->ullSectorNum != pTI->ulSectorOffset + ullSector + nn)
            {
                DclPrintf("SectorNum %llU does not match %llU\n",
                    pSectorInf->ullSectorNum, pTI->ulSectorOffset + ullSector);

                fValid = FALSE;
            }

            if(pSectorInf->ulCRC != DclCrc32Update(0, (const D_BUFFER*)&pSectorInf->ulSignature,
                pTI->DevInfo.ulBytesPerSector - sizeof(pSectorInf->ulCRC)))
            {
                DclPrintf("SectorNum %llU has a bad CRC\n", ullSector);

                fValid = FALSE;
            }
        }

        pSectorInf = (SECTORINF*)(((char*)pSectorInf) + pTI->DevInfo.ulBytesPerSector);
    }

    return fValid;
}


/*-------------------------------------------------------------------
    Local: GetKBPerSecond()

    Calculate KB/second based on the supplied information.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_UINT32 GetKBPerSecond(
    D_UINT32        ulBytesPerSector,
    D_UINT64        ullSectors,
    D_UINT64        ullUS)
{
    #define         SCALESECONDS    (16)
    #define         ONEMILLION      (1000000UL)
    D_UINT64        ullKB;

    /*  Calculate the total KB processed.  If it can't fit in a D_UINT64
        then fail, but that seems rather unlikely...
    */
    ullKB = DclMulDiv64(ullSectors, ulBytesPerSector, UINT64SUFFIX(1024));
    if(ullKB == D_UINT64_MAX)
        return D_UINT32_MAX;

    /*  If the test ran in 0 microseconds, set this to 1 to avoid
        nullifying the math below.
    */
    if(!ullUS)
        ullUS++;

    /*  Scale the numbers up by a factor of 16.  Still allows the test
        to run for about 31,000 years before overflowing the microsecond
        counter, which should probably be sufficient.  The number 16 was
        purposely chosen -- as a power-of-two, it will let DclMulDiv64()
        do any necessary internal scaling using simple shifts (at least
        for the first four iterations).  It also serves to strike a
        balance between limiting the time or size, without much chance
        of introducing inaccuracies due to scaling.
    */
    DclUint64MulUint32(&ullUS, SCALESECONDS);

    ullKB = DclMulDiv64(ullKB, SCALESECONDS * ONEMILLION, ullUS);

    if(ullKB > D_UINT32_MAX)
        return D_UINT32_MAX;
    else
        return (D_UINT32)ullKB;
}


#ifndef DEVIO_STEPPER_ENABLED
#define DEVIO_STEPPER_ENABLED FALSE
#endif

#if DEVIO_STEPPER_ENABLED
/******************************************************************************

Read-Only Stepper Test (broken)

******************************************************************************/
#if 0
static DCLSTATUS Read(
    DEVIOTESTINFO * pTI,
    D_UINT32 ulChunkSize,
    D_UINT32 ulReadCount,
    D_UINT64 * pullCumulativeUS,
    D_UINT64 * pullCumulativeBytes)
{
    D_UINT32 ulChunksTotal;
    D_UINT64 ullBytesTotal;


    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertWritePtr(pullCumulativeUS, sizeof(*pullCumulativeUS));


    ullBytesTotal = pTI->DevInfo.ullTotalSectors * pTI->DevInfo.ulBytesPerSector;
    ulChunksTotal = (D_UINT32)(ullBytesTotal / ulChunkSize);
    ullBytesTotal = ulChunksTotal * ulChunkSize;


    while(ulReadCount--)
    {

        D_UINT32 ulChunkStart;
        D_UINT32 ulChunks;
        D_UINT32 ulSectorStart;
        D_UINT32 ulSectors;
        D_UINT64 ullBytes;


        ulChunkStart = (D_UINT32)(DclRand64(&pTI->sti.ullRandomSeed) % ulChunksTotal);
        ulSectorStart = (ulChunkStart * ulChunkSize) / pTI->DevInfo.ulBytesPerSector;

        ulChunks = 1;
        ullBytes = ulChunks * ulChunkSize;
        ulSectors = (D_UINT32)(ullBytes / pTI->DevInfo.ulBytesPerSector);


        {
            DCLIOSTATUS IoStat;
            D_UINT32 ulLocalUS;
            DCLTIMESTAMP ts;

            ts = DclTimeStamp();
        
            IoStat = pTI->DevInfo.pfnRead(pTI->hBlockDev, ulSectorStart, ulSectors, pTI->pBuffer);

            DclProductionAssert((IoStat.dclStat == DCLSTAT_SUCCESS) && (IoStat.ulCount == ulSectors));
        
            ulLocalUS = DclTimePassedUS(ts);
            *pullCumulativeUS += ulLocalUS;
            *pullCumulativeBytes += ullBytes;
        }
    }

    return DCLSTAT_SUCCESS;
}

void readtest(void)
{
            {
                D_UINT64 ullOffset;
                D_UINT32 ulBytes;
    
                DclPrintf("Sequential Read:\n");
                DclPrintf("   Read Size,     Total Bytes,          Offset,        KBPS\n");
    
                for(ulBytes = pConf->RegionSizeMin; ulBytes <= pConf->RegionSizeMax; ulBytes *= 2)
                {
                    D_UINT64 ullUSTotal= 0;
                    D_UINT64 ullBytesTotal = 0;
    
    
                    for(ullOffset = 0; ullOffset < ullDiskBytes; ullOffset += (ulBytes))
                    {
                        DiskRead(pTI, ulBytes, ullOffset, &ullUSTotal, &ullBytesTotal);
                        DclPrintf("\r");
                        DclPrintf("%12lu,%16llx,%16llx,", ulBytes, ullDiskBytes, ullOffset);
                        DclPrintf("%12lu,", GetKBPerSecond2(ullBytesTotal, ullUSTotal));
    
                        if(kbhit())
                        {
                            int c = getch();
                            (void)c;
                            break;
                        }
                    }
    
                    DclPrintf("\n");
                }
    
    
                DclPrintf("Random Read, 2k Aligned:\n");
                DclPrintf("   Read Size,     Total Reads,      Read Index,        KBPS\n");
    
                for(ulBytes = pConf->RegionSizeMin; ulBytes <= pConf->RegionSizeMax; ulBytes *= 2)
                {
                    D_UINT64 ullUSTotal= 0;
                    D_UINT64 ullBytesTotal = 0;
                    D_UINT32 ulReadIndex;
    
    
                    for(ulReadIndex = 0; ulReadIndex < (D_UINT32)(ullDiskBytes / ulBytes); ulReadIndex ++)
                    {
                        ullOffset = (DclRand64(&pTI->sti.ullRandomSeed) % (ullDiskBytes / 2048)) * 2048;
    
                        DiskRead(pTI, ulBytes, ullOffset, &ullUSTotal, &ullBytesTotal);
                        DclPrintf("\r");
                        DclPrintf("%12lu,%16llx,    %12lu,", ulBytes, (ullDiskBytes / ulBytes), ulReadIndex);
                        DclPrintf("%12lu,", GetKBPerSecond2(ullBytesTotal, ullUSTotal));
    
                        if(kbhit())
                        {
                            int c = getch();
                            (void)c;
                            break;
                        }
                    }
                    
                    DclPrintf("\n");
                }
    
    
                DclPrintf("Random Read, Read Size Aligned:\n");
                DclPrintf("   Read Size,     Total Reads,      Read Index,        KBPS\n");
                
                for(ulBytes = pConf->RegionSizeMin; ulBytes <= pConf->RegionSizeMax; ulBytes *= 2)
                {
                    D_UINT64 ullUSTotal= 0;
                    D_UINT64 ullBytesTotal = 0;
                    D_UINT32 ulReadIndex;
                
                
                    for(ulReadIndex = 0; ulReadIndex < (D_UINT32)(ullDiskBytes / ulBytes); ulReadIndex ++)
                    {
                        ullOffset = (DclRand64(&pTI->sti.ullRandomSeed) % (ullDiskBytes / ulBytes)) * ulBytes;
                
                        DiskRead(pTI, ulBytes, ullOffset, &ullUSTotal, &ullBytesTotal);
                        DclPrintf("\r");
                        DclPrintf("%12lu,%16llx,    %12lu,", ulBytes, (ullDiskBytes / ulBytes), ulReadIndex);
                        DclPrintf("%12lu,", GetKBPerSecond2(ullBytesTotal, ullUSTotal));
    
                        if(kbhit())
                        {
                            int c = getch();
                            (void)c;
                            break;
                        }
                    }
                    
                    DclPrintf("\n");
                }
            }
}
#endif

/******************************************************************************

Stepper Test

******************************************************************************/

#define TEST_VERBOSE        FALSE
#define STEPPER_STATUS_VALUE_MAX    20
#define STEPPER_STATUS_STRLEN       256
    
#define d_KB(kbnum) ((D_UINT64)((kbnum) / ((D_UINT64)1*1024)))
#define m_KB(kbnum) ((D_UINT64)((kbnum) % ((D_UINT64)1*1024)))
#define d_MB(mbnum) ((D_UINT64)((mbnum) / ((D_UINT64)1*1024*1024)))
#define m_MB(mbnum) ((D_UINT64)((mbnum) % ((D_UINT64)1*1024*1024)))
#define d_GB(gbnum) ((D_UINT64)((gbnum) / ((D_UINT64)1*1024*1024*1024)))
#define m_GB(gbnum) ((D_UINT64)((gbnum) % ((D_UINT64)1*1024*1024*1024)))

#define BKBMBGB(num) (m_GB(num) ? (m_MB(num) ? (m_KB(num) ? (num) : d_KB(num)) : d_MB(num)) : d_GB(num))
#define BKBMBGB_STR(num_s) (m_GB(num_s) ? (m_MB(num_s) ? (m_KB(num_s) ? ("B") : ("KB")) : ("MB")) : ("GB"))



typedef struct _sStepperConfiguration sStepperConfiguration;
typedef struct _sFWALConfiguration sFWALConfiguration;

typedef void (*fnWritePattern)(
    sStepperConfiguration * pConf,
    D_UINT64    ullRegionOffset);


typedef struct
{
    D_UINT64    ullTotalUS;
    D_UINT64    ullTotalBytes;
} sTestData;

/*** Forward Allocator ***/
struct _sFWALConfiguration
{
    D_UINT32 ulMinRead;
    D_UINT32 ulCopyPercent;
    D_UINT32 ulReadFragments;
    D_BOOL fVerbose;
};

struct _sStepperConfiguration
{
    DEVIOTESTINFO * pTI;

    char        szStatusHeadingStringPrevious[STEPPER_STATUS_STRLEN];
    D_UINT32    aulStatusColumnWidthPrevious[STEPPER_STATUS_VALUE_MAX];
    D_UINT32    ulStatusHeadingCountPrevious;

    D_UINT32    aulStatusColumnWidth[STEPPER_STATUS_VALUE_MAX];
    D_BOOL      afStatusValueHexDisplay[STEPPER_STATUS_VALUE_MAX];
    char        szStatusHeadingString[STEPPER_STATUS_STRLEN];
    char        szStatusValueString[STEPPER_STATUS_STRLEN];
    D_UINT32    ulStatusHeadingCount;
    D_UINT32    ulStatusValueIndex;

    char     ** ppszWritePatternShorts;

    D_UINT32    ulBreakLevels;

    D_BOOL      fEnablePatternWrite;
    D_BOOL      fEnablePatternRead;
    D_BOOL      fEnableRandomRead;
    D_BOOL      fEnableBigWrite;
    D_BOOL      fEnableBigRead;
    D_BOOL      fEnableCleanseWrite;

    D_UINT64    ullMaxTimePerTest;

    D_UINT64    ullDiskBytes;

    D_UINT32    ulPatternRegionSizeMin;
    D_UINT32    ulPatternRegionSizeMax;
    D_UINT32    ulPatternIoSizeMin;
    D_UINT32    ulPatternIoSizeMax;
    double      dRepeatDiskFraction;

    double      dSanitiseDiskFraction;
    D_UINT32    ulSanitiseDiskWriteSize;
    D_UINT32    ulSanitiseDiskSleepSeconds;

    D_UINT32    ulBigWriteSize;
    D_UINT32    ulBigWriteFrequency;

    D_UINT32    ulRandomIOSize;

    D_UINT32    ulRegionSize;
    D_UINT32    ulIOPattern;
    D_UINT32    ulPatternIOSize;
    D_UINT32    ulRegionIndex;
    D_UINT64    ullTotalTestUS;


    D_UINT64    ullTestInfo;


    sTestData   sDataWritePattern[4];
    sTestData   sDataWriteBig[4];
    sTestData   sDataReadPattern[4];
    sTestData   sDataReadRandom[4];
    sTestData   sDataReadBig[4];

    D_BOOL    * pfWritePatterns;


    sFWALConfiguration sConfFWAL;
};

#define STEPPER_INPUT_MAX (256)

D_BOOL StepperGets(
    sStepperConfiguration * pConf,
    char * pszStr,
    D_UINT32 ulMaxChar)
{
    D_UINT32    ulCharIndex;

    for(ulCharIndex = 1; ulCharIndex <= STEPPER_INPUT_MAX; ulCharIndex++)
    {
        char cChar;
        DCLSTATUS DclStatus;

        DclStatus = DclInputChar(pConf->pTI->sti.hDclInst, &cChar, 0);
        if(DclStatus != DCLSTAT_SUCCESS)
        {
            DclPrintf("\nError in DclInputChar: %lx\n", DclStatus);
            return FALSE;
        }

        pszStr[ulCharIndex - 1] = cChar;

        if((cChar == '\r') || (cChar == '\n'))
        {
            break;
        }
        else if(cChar == '\b')
        {
            DclPrintf(" ");

            if(ulCharIndex > 1)
            {
                DclPrintf("\b");
            }

            ulCharIndex -= DCLMIN(ulCharIndex, 2);
        }
    }

    if(ulCharIndex >= STEPPER_INPUT_MAX)
    {
        DclPrintf("\nERROR: Input too long.\n");
        pszStr[0] = 0;
        return FALSE;
    }
    else
    {
        pszStr[ulCharIndex - 1] = 0;
        return TRUE;
    }
}

D_BOOL StepperGetsUINT64(
    sStepperConfiguration * pConf,
    D_UINT64 * pullNum)
{
    char szInput[STEPPER_INPUT_MAX + 1];

    if(StepperGets(pConf, szInput, STEPPER_INPUT_MAX))
    {
        D_UINT64 ullNum;
        D_UINT64 ullMul;
        D_UINT32 ulIndex;
        char * pszGBMBKB;


        /*  Make sure there's a number in the input.
        */
        for(ulIndex = 0; ulIndex < DclStrLen(szInput); ulIndex++)
        {
            if((szInput[ulIndex] < '0') || (szInput[ulIndex] > '9'))
            {
                break;
            }
        }
        if(ulIndex == 0)
        {
            return FALSE;
        }


        ullMul = 1;

        pszGBMBKB = DclStrChr(szInput, 'G');
        if(!pszGBMBKB)
        {
            pszGBMBKB = DclStrChr(szInput, 'g');
        }

        if(pszGBMBKB)
        {
            ullMul = 1024 * 1024 * 1024;
        }
        else
        {
            pszGBMBKB = DclStrChr(szInput, 'M');
            if(!pszGBMBKB)
            {
                pszGBMBKB = DclStrChr(szInput, 'm');
            }

            if(pszGBMBKB)
            {
                ullMul = 1024 * 1024;
            }
            else
            {
                pszGBMBKB = DclStrChr(szInput, 'K');
                if(!pszGBMBKB)
                {
                    pszGBMBKB = DclStrChr(szInput, 'k');
                }

                if(pszGBMBKB)
                {
                    ullMul = 1024;
                }
                else
                {
                    pszGBMBKB = DclStrChr(szInput, 'B');
                    if(!pszGBMBKB)
                    {
                        pszGBMBKB = DclStrChr(szInput, 'b');
                    }
                }
            }
        }

        if(pszGBMBKB)
        {
            *pszGBMBKB = 0;
        }

        ullNum = DclAtoL(szInput);
        ullNum *= ullMul;
        *pullNum = ullNum;

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static void StatusInit(
    sStepperConfiguration * pConf)
{
    if(pConf->ulStatusHeadingCount)
    {
        pConf->ulStatusHeadingCountPrevious = pConf->ulStatusHeadingCount;
        DclMemCpy(pConf->aulStatusColumnWidthPrevious, pConf->aulStatusColumnWidth, sizeof(pConf->ulStatusHeadingCount));
        DclMemCpy(pConf->szStatusHeadingStringPrevious, pConf->szStatusHeadingString, sizeof(pConf->szStatusHeadingStringPrevious));
    }

    DclMemSet(pConf->szStatusHeadingString, 0, sizeof(pConf->szStatusHeadingString));
    DclMemSet(pConf->aulStatusColumnWidth, 0, sizeof(pConf->aulStatusColumnWidth));
    pConf->ulStatusHeadingCount = 0;
    pConf->ulStatusValueIndex = 0;
}

static void StatusHeadingAdd(
    sStepperConfiguration * pConf,
    char * pszHeading,
    D_UINT32 ulValueSizeBytes,
    D_BOOL fHexDisplay)
{
    D_UINT32    ulColumnWidth;

    ulColumnWidth = DCLMAX(DclStrLen(pszHeading), ulValueSizeBytes);

    if(pConf->ulStatusHeadingCount)
    {
        /*  For the comma and space
        */
        ulColumnWidth += 2;
    }

    if(pConf->ulStatusHeadingCount >= STEPPER_STATUS_VALUE_MAX)
    {
        DclPrintf("ERROR: Too many headings trying to add %s!\n", pszHeading);
        DclProductionError();
    }
    else if(DclStrLen(pConf->szStatusHeadingString) + ulColumnWidth + 1 > STEPPER_STATUS_STRLEN)
    {
        DclPrintf("ERROR: Insufficient output string space to add heading %s!\n", pszHeading);
        DclProductionError();
    }
    else
    {
        D_UINT32 ulFill;

        ulFill = ulColumnWidth - DclStrLen(pszHeading);

        if(pConf->ulStatusHeadingCount)
        {
            DclStrCat(pConf->szStatusHeadingString, ", ");
            ulFill -= 2;
        }

        if(ulColumnWidth > ulValueSizeBytes)
        {
            D_UINT32 ulSpaceIndex;

            for(ulSpaceIndex = 0; ulSpaceIndex < ulFill; ulSpaceIndex++)
            {
                DclStrCat(pConf->szStatusHeadingString, " ");
            }
        }

        DclStrCat(pConf->szStatusHeadingString, pszHeading);

        pConf->aulStatusColumnWidth[pConf->ulStatusHeadingCount] = ulColumnWidth;
        pConf->afStatusValueHexDisplay[pConf->ulStatusHeadingCount] = fHexDisplay;
        pConf->ulStatusHeadingCount++;
    }
}

static void StatusHeadingDisplay(
    sStepperConfiguration * pConf)
{
    /*  If the headings for this test are the same as the headings for the
        previous test, don't re-display the headings.
    */
    if(pConf->ulStatusHeadingCountPrevious == pConf->ulStatusHeadingCount)
    {
        if(DclMemCmp(pConf->aulStatusColumnWidthPrevious, pConf->aulStatusColumnWidth, sizeof(pConf->ulStatusHeadingCount)) == 0)
        {
            if(DclMemCmp(pConf->szStatusHeadingStringPrevious, pConf->szStatusHeadingString, sizeof(pConf->szStatusHeadingStringPrevious)) == 0)
            {
                return;
            }
        }
    }

    DclPrintf("%s\n", pConf->szStatusHeadingString);
}


static void StatusValueAddUINT64(
    sStepperConfiguration * pConf,
    D_UINT64 ullValue)
{
    if(pConf->ulStatusValueIndex >= pConf->ulStatusHeadingCount)
    {
        DclPrintf("ERROR: Caller attempted to add value %lu, but max is %lu.\n",
                pConf->ulStatusValueIndex + 1,
                pConf->ulStatusHeadingCount);
    }
    else if(pConf->aulStatusColumnWidth[pConf->ulStatusValueIndex] < sizeof(D_UINT64) + 1)
    {
        DclPrintf("ERROR: Caller attempted to add an 8 character value, but column with is %lu.\n",
                pConf->aulStatusColumnWidth[pConf->ulStatusValueIndex] - 1);
    }
    else
    {
        char szConvertBuffer[STEPPER_STATUS_STRLEN] = {0};
        D_UINT32 ulRadix;
        size_t nConverted;
        size_t nFill;

        if(pConf->afStatusValueHexDisplay[pConf->ulStatusValueIndex])
        {
            ulRadix = 16;
        }
        else
        {
            ulRadix = 10;
        }

        nConverted = sizeof(szConvertBuffer);
        nFill = pConf->aulStatusColumnWidth[pConf->ulStatusValueIndex];

        if(pConf->ulStatusValueIndex)
        {
            DclStrCat(pConf->szStatusValueString, ",");
            nFill--;
        }

        DclULLtoA(szConvertBuffer, &nConverted, &ullValue, ulRadix, nFill, ' ');

        DclStrCat(pConf->szStatusValueString, szConvertBuffer);
    }

    pConf->ulStatusValueIndex++;
}


static void StatusValueAddUINT32(
    sStepperConfiguration * pConf,
    D_UINT32 ulValue)
{
    if(pConf->ulStatusValueIndex >= pConf->ulStatusHeadingCount)
    {
        DclPrintf("ERROR: Caller attempted to add value %lu, but max is %lu.\n",
                pConf->ulStatusValueIndex + 1,
                pConf->ulStatusHeadingCount);
    }
    else if(pConf->aulStatusColumnWidth[pConf->ulStatusValueIndex] < sizeof(D_UINT32) + 1)
    {
        DclPrintf("ERROR: Caller attempted to add a 4 character value, but column with is %lu.\n",
                pConf->aulStatusColumnWidth[pConf->ulStatusValueIndex] - 1);
    }
    else
    {
        char szConvertBuffer[STEPPER_STATUS_STRLEN] = {0};
        D_UINT32 ulRadix;
        size_t nConverted;
        size_t nFill;

        if(pConf->afStatusValueHexDisplay[pConf->ulStatusValueIndex])
        {
            ulRadix = 16;
        }
        else
        {
            ulRadix = 10;
        }

        nConverted = sizeof(szConvertBuffer);
        nFill = pConf->aulStatusColumnWidth[pConf->ulStatusValueIndex];

        if(pConf->ulStatusValueIndex)
        {
            DclStrCat(pConf->szStatusValueString, ",");
            nFill--;
        }

        DclULtoA(szConvertBuffer, &nConverted, ulValue, ulRadix, nFill, ' ');

        DclStrCat(pConf->szStatusValueString, szConvertBuffer);
    }

    pConf->ulStatusValueIndex++;
}


static void StatusValueAddString(
    sStepperConfiguration * pConf,
    char * pszValue)
{
    D_UINT32 ulStrLen;

    ulStrLen = DclStrLen(pszValue);

    if(pConf->ulStatusValueIndex >= pConf->ulStatusHeadingCount)
    {
        DclPrintf("ERROR: Caller attempted to add value %lu, but max is %lu.\n",
                pConf->ulStatusValueIndex + 1,
                pConf->ulStatusHeadingCount);
    }
    else if(pConf->aulStatusColumnWidth[pConf->ulStatusValueIndex] < ulStrLen + 1)
    {
        DclPrintf("ERROR: Caller attempted to add \"%s\", but column with is %lu.\n", pszValue, ulStrLen - 1);
    }
    else
    {
        D_UINT32 ulFill;

        ulFill = pConf->aulStatusColumnWidth[pConf->ulStatusValueIndex] - ulStrLen;

        if(pConf->ulStatusValueIndex)
        {
            DclStrCat(pConf->szStatusValueString, ",");
            ulFill--;
        }

        while(ulFill--)
        {
            DclStrCat(pConf->szStatusValueString, " ");
        }

        DclStrCat(pConf->szStatusValueString, pszValue);
    }

    pConf->ulStatusValueIndex++;
}


static void StatusValueDisplay(
    sStepperConfiguration * pConf)
{
    DclPrintf("\r%s", pConf->szStatusValueString);
    pConf->szStatusValueString[0] = 0;
    pConf->ulStatusValueIndex = 0;
}


static DCLSTATUS DiskRead(
    sStepperConfiguration * pConf,
    D_UINT32 ulBytes,
    D_UINT64 ullOffset,
    D_UINT64 * pullCumulativeUS,
    D_UINT64 * pullCumulativeBytes)
{
    D_UINT32 ulSectorStart;
    D_UINT32 ulSectors;


    ulSectorStart = (D_UINT32)(ullOffset / pConf->pTI->DevInfo.ulBytesPerSector);
    ulSectors = (D_UINT32)(ulBytes / pConf->pTI->DevInfo.ulBytesPerSector);


    {
        DCLIOSTATUS IoStat;
        D_UINT32 ulLocalUS;
        DCLTIMESTAMP ts;

        ts = DclTimeStamp();
    
        IoStat = pConf->pTI->DevInfo.pfnRead(pConf->pTI->hBlockDev, ulSectorStart, ulSectors, pConf->pTI->pBuffer);

        DclProductionAssert((IoStat.dclStat == DCLSTAT_SUCCESS) && (IoStat.ulCount == ulSectors));
    
        ulLocalUS = DclTimePassedUS(ts);
        *pullCumulativeUS += ulLocalUS;
        *pullCumulativeBytes += ulBytes;
    }

    return DCLSTAT_SUCCESS;
}


void DiskWrite(
    sStepperConfiguration * pConf,
    D_UINT32    ulBytes,
    D_UINT64    ullOffset,
    D_UINT64 *  pullElapsedUS,
    D_UINT64 *  pullTotalBytes)
{
    DCLIOSTATUS     IoStat = {0};
    DCLTIMESTAMP    ts;
    SECTORINF     * pSectorInf = (SECTORINF*)pConf->pTI->pBuffer;
    D_UINT32        ulSectorIndex;
    D_UINT32    ulLocalUS;
    D_UINT32    ulWriteSector;
    D_UINT32    ulSectors;


    ulWriteSector = (D_UINT32)(ullOffset / pConf->pTI->DevInfo.ulBytesPerSector);
    ulSectors = ulBytes / pConf->pTI->DevInfo.ulBytesPerSector;


    /*  Make the data unique.
    */
    for(ulSectorIndex=0; ulSectorIndex < ulSectors; ulSectorIndex++)
    {
        pSectorInf->ullSectorNum = ulWriteSector + ulSectorIndex;
        pSectorInf->ulPassNum = ulSectorIndex;
        pSectorInf->ulCRC = DclCrc32Update(0, (const D_BUFFER*)&pSectorInf->ulSignature,
                pConf->pTI->DevInfo.ulBytesPerSector - sizeof(pSectorInf->ulCRC));
        pSectorInf = (SECTORINF*)(((char*)pSectorInf) + pConf->pTI->DevInfo.ulBytesPerSector);
    }

    ts = DclTimeStamp();

    IoStat = pConf->pTI->DevInfo.pfnWrite(pConf->pTI->hBlockDev, ulWriteSector, ulSectors, pConf->pTI->pBuffer);

    DclProductionAssert((IoStat.dclStat == DCLSTAT_SUCCESS) && (IoStat.ulCount == ulSectors));

    ulLocalUS = DclTimePassedUS(ts);
    *pullElapsedUS += ulLocalUS;
    *pullTotalBytes += ulBytes;


    DclProductionAssert((IoStat.dclStat == DCLSTAT_SUCCESS) && (IoStat.ulCount == ulSectors));
}


void PatternIO(
    sStepperConfiguration * pConf,
    D_UINT32    ulBytes,
    D_UINT64    ullOffset)
{
    if(pConf->fEnablePatternRead)
    {
        DiskRead(pConf, ulBytes, ullOffset, &pConf->sDataReadPattern[0].ullTotalUS, &pConf->sDataReadPattern[0].ullTotalBytes);
    }
    if(pConf->fEnablePatternWrite)
    {
        DiskWrite(pConf, ulBytes, ullOffset, &pConf->sDataWritePattern[0].ullTotalUS, &pConf->sDataWritePattern[0].ullTotalBytes);
    }
    if(pConf->fEnableRandomRead)
    {
        D_UINT64 ullRandOffset;
    
        ullRandOffset = DclRand64(&pConf->pTI->sti.ullRandomSeed) % (pConf->ullDiskBytes / pConf->ulRandomIOSize);
        ullRandOffset *= pConf->ulRandomIOSize;
        if(ullRandOffset + pConf->ulRandomIOSize > pConf->ullDiskBytes)
        {
            ullRandOffset -= pConf->ulRandomIOSize;
        }
    
        DiskRead(pConf, pConf->ulRandomIOSize, ullRandOffset, &pConf->sDataReadRandom[0].ullTotalUS, &pConf->sDataReadRandom[0].ullTotalBytes);
    }
}


static D_UINT32 GetKBPerSecond2(
    D_UINT64        ullBytes,
    D_UINT64        ullUS)
{
    #define         SCALESECONDS    (16)
    #define         ONEMILLION      (1000000UL)
    D_UINT64        ullKB;

    /*  Calculate the total KB processed.  If it can't fit in a D_UINT64
        then fail, but that seems rather unlikely...
    */
    ullKB = ullBytes /= 1024;


    /*  If the test ran in 0 microseconds, set this to 1 to avoid
        nullifying the math below.
    */
    if(!ullUS)
        ullUS++;

    /*  Scale the numbers up by a factor of 16.  Still allows the test
        to run for about 31,000 years before overflowing the microsecond
        counter, which should probably be sufficient.  The number 16 was
        purposely chosen -- as a power-of-two, it will let DclMulDiv64()
        do any necessary internal scaling using simple shifts (at least
        for the first four iterations).  It also serves to strike a
        balance between limiting the time or size, without much chance
        of introducing inaccuracies due to scaling.
    */
    DclUint64MulUint32(&ullUS, SCALESECONDS);

    ullKB = DclMulDiv64(ullKB, SCALESECONDS * ONEMILLION, ullUS);

    if(ullKB > D_UINT32_MAX)
        return D_UINT32_MAX;
    else
        return (D_UINT32)ullKB;
}


static void DiskSanitise(
    sStepperConfiguration * pConf,
    double dSanitisePercent,
    D_UINT32 ulSanitiseWriteSize)
{
    D_UINT64    ullDiskBytes;
    D_UINT64    ullCleanseBytes;
    D_UINT64    ullBytesCleansed;
    D_UINT64    ullOffset;
    D_UINT32    ulCleanseNum;
    char *      pszCleanseStr;


    ullDiskBytes = pConf->pTI->DevInfo.ullTotalSectors * pConf->pTI->DevInfo.ulBytesPerSector;
    ullCleanseBytes = ullDiskBytes * dSanitisePercent;
    ullOffset = 0;
    ullBytesCleansed = 0;

    ulCleanseNum = BKBMBGB(ulSanitiseWriteSize);
    pszCleanseStr = BKBMBGB_STR(ulSanitiseWriteSize);

    DclPrintf("Sanitising the disk: Write %lu MB, %lu %s at a time:  00%%",
            (D_UINT32)(ullCleanseBytes/(1024*1024)),
            ulCleanseNum,
            pszCleanseStr);

    while(ullBytesCleansed < ullCleanseBytes)
    {
        D_UINT32    ulThisWriteSize;
        D_UINT64    ullJunk;

        ulThisWriteSize = (D_UINT32)(DCLMIN(ullDiskBytes - ullOffset, ulSanitiseWriteSize));

        DiskWrite(pConf, ulThisWriteSize, ullOffset, &ullJunk, &ullJunk);

        DclPrintf("\b\b\b");
        DclPrintf("%02lu%%", (ullBytesCleansed * 100) / ullCleanseBytes);

        ullOffset += ulThisWriteSize;
        ullBytesCleansed += ulThisWriteSize;

#if 0
        if(kbhit())
        {
            int c = getch();
            (void)c;
            break;
        }
#endif

        if(ullOffset >= ullDiskBytes)
        {
            ullOffset -= ullDiskBytes;
        }
    }

    DclOsSleep(pConf->ulSanitiseDiskSleepSeconds * 1000);

    DclPrintf("\r");
    DclPrintf("                                                                       ");
    DclPrintf("\r");
}


//#define pTConf->ulMinRead       (1024)
//#define pTConf->ulCopyPercent   (2)
//#define pTConf->ulReadFragments (32)
//#define pTConf->fVerbose        (FALSE)



/*** Forward Allocator ***/
static void FWAL_ConfigDisplay(
    sStepperConfiguration * pConf)
{
    D_UINT32 ulMinReadNum;
    char * pszMinReadStr;
    sFWALConfiguration * pTConf = &pConf->sConfFWAL;


    /*  Something's up with DclPrintf and these macros, so for now they need
        to be done this way.
    */
    ulMinReadNum = BKBMBGB(pTConf->ulMinRead);

    pszMinReadStr = BKBMBGB_STR(pTConf->ulMinRead);

    DclPrintf("Forward Allocator Test Configuration:\n");
    DclPrintf("(a) Page Size:                   %10lu %s\n", ulMinReadNum, pszMinReadStr);
    DclPrintf("(b) Region Copy Percent:         %10lu%%\n", pTConf->ulCopyPercent);
    DclPrintf("(c) Read Fragment Count:         %10lu\n", pTConf->ulReadFragments);

    DclPrintf("\n");
}

static void FWAL_ConfigMenu(
    sStepperConfiguration * pConf)
{
    sFWALConfiguration * pTConf = &pConf->sConfFWAL;

    pTConf->ulCopyPercent = 50;
    pTConf->ulMinRead = 1024;
    pTConf->ulReadFragments = 16;

    FWAL_ConfigDisplay(pConf);


    DclPrintf("(x) Accept Configuration\n");
    DclPrintf("(z) Display Configuration\n");
    

    while(TRUE)
    {
        char cChar = 0;
        D_BOOL fInputResult = TRUE;


        DclPrintf("Enter a command:");

        if(DclInputChar(pConf->pTI->sti.hDclInst, &cChar, 0) != DCLSTAT_SUCCESS)
        {
            break;
        }

        DclPrintf("\r                                   \r");

        switch(cChar)
        {
            case 'a':
            case 'A':
                {
                    D_UINT64    ullNew;

                    DclPrintf("Enter new min read size: ");
                    fInputResult = StepperGetsUINT64(pConf, &ullNew);
                    if(fInputResult)
                    {
                        pTConf->ulMinRead = (D_UINT32)ullNew;
                        DclPrintf("\rNew min read size: %lu          \n", (D_UINT32)ullNew);
                    }
                    else
                    {
                        DclPrintf("\nInvalid input!\n");
                    }
                }
                break;

            case 'b':
            case 'B':
                {
                    D_UINT64    ullNew;

                    DclPrintf("Enter new region copy percent: ");
                    fInputResult = StepperGetsUINT64(pConf, &ullNew);
                    if(fInputResult)
                    {
                        DclAssert(ullNew <= D_UINT32_MAX);
                        pTConf->ulCopyPercent = (D_UINT32)ullNew;
                        DclPrintf("\rNew region copy percent: %lu%%          ", (D_UINT32)ullNew);
                    }
                    else
                    {
                        DclPrintf("\nInvalid input!\n");
                    }
                }
                break;

            case 'c':
            case 'C':
                {
                    D_UINT64    ullNew;

                    DclPrintf("Enter new read fragment per region count: ");
                    fInputResult = StepperGetsUINT64(pConf, &ullNew);
                    if(fInputResult)
                    {
                        DclAssert(ullNew <= D_UINT32_MAX);
                        pTConf->ulReadFragments = (D_UINT32)ullNew;
                        DclPrintf("\rNew read fragment per region count: %lu          ", (D_UINT32)ullNew);
                    }
                    else
                    {
                        DclPrintf("\nInvalid input!\n");
                    }
                }
                break;

            case 'z':
            case 'Z':
                DclPrintf("\n\n");
                FWAL_ConfigDisplay(pConf);
                DclPrintf("(x) Accept Configuration\n");
                DclPrintf("(z) Display Configuration\n");
                break;

            case 'x':
            case 'X':
                DclPrintf("Configuration finalized.\n");
                return;

            default:
                if(cChar != '\n')
                {
                    DclPrintf("Unknown command '%c'.             ", cChar);
                }
                break;
        }

        DclPrintf("\n");
    }

    return;
}

void PatternIO_ForwardAllocator(
    sStepperConfiguration * pConf,
    D_UINT64 ullRegionOffset)
{
    static D_UINT64 ullBytesReadTotal = 0;
    static D_UINT64 ullBytesSkippedTotal = 0;
    static D_UINT64 ullReadRegion;
    D_UINT64 ullWriteOffset;
    D_UINT64 ullReadOffset;
    D_UINT32 ulBytesWritten = 0;
    D_UINT32 ulReadBytesTotal;
    sFWALConfiguration * pTConf = &pConf->sConfFWAL;
    D_UINT32 ulFragmentsLeft = pTConf->ulReadFragments;


    StatusInit(pConf);
    StatusHeadingAdd(pConf, "Pattern", 4, FALSE);
    StatusHeadingAdd(pConf, "Region Size", 10, FALSE);
    StatusHeadingAdd(pConf, "I/O Size", 10, FALSE);
    StatusHeadingAdd(pConf, "Region Total", 10, FALSE);
    StatusHeadingAdd(pConf, "Region Index", 10, FALSE);
    StatusHeadingAdd(pConf, "Write Pattern KBPS", 7, FALSE);
    StatusHeadingAdd(pConf, "Read Pattern KBPS", 7, FALSE);
    StatusHeadingAdd(pConf, "Write Bytes", 16, FALSE);
    StatusHeadingAdd(pConf, "Read Bytes", 16, FALSE);
    StatusHeadingAdd(pConf, "Write US", 16, FALSE);
    StatusHeadingAdd(pConf, "Read US", 16, FALSE);
    StatusHeadingDisplay(pConf);


    if(pTConf->fVerbose)
    {
        DclPrintf("\n");
    }


    /*  Init
    */
    if(pConf->ulRegionIndex == 0)
    {
        ullReadRegion = ullRegionOffset;
    }

    /*  Write the region we were reading from last.
    */
    ullWriteOffset = ullReadRegion;
    ullReadRegion = (D_UINT32)(DclRand64(&pConf->pTI->sti.ullRandomSeed) % (pConf->ullDiskBytes / pConf->ulRegionSize));
    ullReadRegion *= pConf->ulRegionSize;
    ullReadOffset = ullReadRegion;

    /*  Precompute to clean up code.
    */
    ulReadBytesTotal = pConf->ulRegionSize / ((float)100 / pTConf->ulCopyPercent);


    DclProductionAssert(ulReadBytesTotal % pConf->ulPatternIOSize == 0);
    DclProductionAssert(ulReadBytesTotal / pTConf->ulMinRead >= pTConf->ulReadFragments);
    DclProductionAssert((pTConf->ulReadFragments * pTConf->ulMinRead) + ulReadBytesTotal <= pConf->ulRegionSize);


    while(ulBytesWritten < pConf->ulRegionSize)
    {
        D_UINT64 ullWriteUS = 0;

        if(ulBytesWritten < ulReadBytesTotal)
        {
            D_UINT32 ulBytesRead = 0;

            while(ulBytesRead < pConf->ulPatternIOSize)
            {
                D_UINT64 ullReadUS = 0;
                D_UINT32 ulReadSize;
                D_UINT32 ulReadBytesLeft;
                D_UINT32 ulSeek = 0;


                ulReadBytesLeft = ulReadBytesTotal - (ulBytesWritten + ulBytesRead);
                ulReadSize = pConf->ulPatternIOSize - ulBytesRead;


                /*  Decide whether to fragment the read 
                */
                if(ulFragmentsLeft)
                do {
                    float dChance;
                    float dFragmentChance;
                    D_UINT32 ulTotalSizeMax;
                    D_UINT32 ulReadSizeMax;
                    D_UINT32 ulSeekMax;


                    /*  *********************************************
                        **** Compute the chance of fragmentation ****
                        *********************************************  */
                    {
                        float dFragmentChanceNormal;
                        float dReadProgress;
                        float dFragProgress;
                        float fMultiplier;


                        dFragmentChanceNormal = (float)pTConf->ulReadFragments / (ulReadBytesTotal / pConf->ulPatternIOSize);
                        dReadProgress = (float)(ulReadBytesTotal - ulReadBytesLeft) / ulReadBytesTotal;
                        dFragProgress = (float)(pTConf->ulReadFragments - ulFragmentsLeft) / pTConf->ulReadFragments;


                        dFragmentChance = dFragmentChanceNormal;


                        fMultiplier = 1.0;
                        if(dReadProgress)
                        {
                            fMultiplier += dReadProgress;

                            if(dFragProgress)
                            {
                                fMultiplier -= dFragProgress;
                            }
                        }


                        dFragmentChance *= fMultiplier;
                    }


                    dChance = (float)(DclRand64(&pConf->pTI->sti.ullRandomSeed) % 1000) / 1000;
                    if(dChance > dFragmentChance)
                    {
                        /*  Don't fragment.
                        */
                        break;
                    }



                    /*  ******************************************************
                        **** Compute maximum size of combined read + seek ****
                        ******************************************************  */

                    /*  Start with the remaining space in the region
                    */
                    ulTotalSizeMax = pConf->ulRegionSize;
                    ulTotalSizeMax -= (D_UINT32)(ullReadOffset - ullReadRegion);

                    /*  Subtract bytes left to read, not including the
                        remainder of this write buffer.
                    */
                    ulTotalSizeMax -= ulReadBytesLeft - (pConf->ulPatternIOSize - ulBytesRead);

                    /*  Minimum space to make more fragments.
                    */
                    ulTotalSizeMax -= (ulFragmentsLeft - 1) * pTConf->ulMinRead;
                    if(ulTotalSizeMax == pTConf->ulMinRead)
                    {
                        /*  Can't fragment, must read.
                        */
                        ulReadSize = pTConf->ulMinRead;
                        break;
                    }



                    /*  **************************************
                        **** Compute maximum size of read ****
                        **************************************  */

                    /*  Bytes left in write buffer or maximum total
                        size - min seek length, whichever is less.
                    */
                    ulReadSizeMax = DCLMIN(ulReadSize, ulTotalSizeMax - pTConf->ulMinRead);



                    /*  ***************************
                        **** Compute read size ****
                        ***************************  */

                    /*  
                    */
                    ulReadSize = (D_UINT32)(DclRand64(&pConf->pTI->sti.ullRandomSeed) % (ulReadSizeMax / pTConf->ulMinRead));
                    ulReadSize ++;
                    ulReadSize *= pTConf->ulMinRead;



                    /*  ***************************
                        **** Compute seek size ****
                        ***************************  */

                    /*  Start with maximum possible
                    */
                    ulSeekMax = ulTotalSizeMax - ulReadSizeMax;



                    /*  Compute the average size of the remaining seeks
                    */
                    {
                        D_UINT32 ulSeekAvg;

                        ulSeekAvg = ulSeekMax / pTConf->ulMinRead;
                        ulSeekAvg /= ulFragmentsLeft;
                        ulSeekAvg = DCLMAX(ulSeekAvg, 2);

                        ulSeek = 1;
                        while(ulSeek < (ulSeekMax / pTConf->ulMinRead))
                        {
                            D_UINT32 ulSeekChunk;

                            ulSeekChunk = (D_UINT32)(DclRand64(&pConf->pTI->sti.ullRandomSeed) % ulSeekAvg);
                            if(ulSeekChunk == 0)
                            {
                                /*  Allow a seek to be greater than the average.
                                */
                                ulSeek += ulSeekAvg;
                                continue;
                            }

                            ulSeek += ulSeekChunk;
                            break;
                        }
                        ulSeek *= pTConf->ulMinRead;

                        /*  Above loop uses average as max, so double to make
                            average the mid value.
                        */
                        ulSeek *= 2;

                        /*
                        */
                        ulSeek = DCLMIN(ulSeek, ulSeekMax);

                        {
                            D_UINT32 ulOffsetsRemaining = (D_UINT32)((ullReadRegion + pConf->ulRegionSize) - ullReadOffset);
                            D_UINT32 ulFudgeRoom = ulOffsetsRemaining - ulReadBytesLeft;
                            D_UINT32 ulBufferReadBytesLeft = pConf->ulPatternIOSize - ulBytesRead;

                            if(pTConf->fVerbose)
                            {
                                DclPrintf("%lX, %lX, %lX, %lX --- %lX, %lX, %lX --- %lX, %lX\n",

                                        ulOffsetsRemaining,
                                        ulReadBytesLeft,
                                        ulFudgeRoom,
                                        ulBufferReadBytesLeft,

                                        ulTotalSizeMax,
                                        ulReadSizeMax,
                                        ulSeekMax,

                                        ulSeek,
                                        ulReadSize);
                            }
                        }
                    }
                } while(FALSE);


                DclAssert(ullReadOffset < ullReadRegion + pConf->ulRegionSize);
                DiskRead(pConf, ulReadSize, ullReadOffset, &ullReadUS, &pConf->sDataReadPattern[0].ullTotalBytes);

                ulBytesRead += ulReadSize;
                ullBytesReadTotal += ulReadSize;

                if(pTConf->fVerbose)
                {
                    DclPrintf("Read %4lukb, Seek %4lukb, Remaining %4lukb, Offset %llX\n", ulReadSize / 1024, ulSeek / 1024, (pConf->ulPatternIOSize - ulBytesRead) / 1024, ullReadOffset);
                }

                if(ulSeek)
                {
                    ulFragmentsLeft--;
                }

                ullReadOffset += ulSeek + ulReadSize;


                /*  Count the time against both reading and writing.
                */
                pConf->sDataReadPattern[0].ullTotalUS += ullReadUS;
            }
        }


        DiskWrite(pConf, pConf->ulPatternIOSize, ullWriteOffset, &ullWriteUS, &pConf->sDataWritePattern[0].ullTotalBytes);

        pConf->sDataWritePattern[0].ullTotalUS += ullWriteUS;

        ulBytesWritten += pConf->ulPatternIOSize;
        
        if(pTConf->fVerbose)
        {
            DclPrintf("Wrote %lukb, Remaining %lukb, Offset %llX\n", pConf->ulPatternIOSize / 1024, (pConf->ulRegionSize - ulBytesWritten) / 1024, ullWriteOffset);
        }

        ullWriteOffset += pConf->ulPatternIOSize;
    }

    StatusValueAddString(pConf, pConf->ppszWritePatternShorts[pConf->ulIOPattern]);
    StatusValueAddUINT32(pConf, pConf->ulRegionSize);
    StatusValueAddUINT32(pConf, pConf->ulPatternIOSize);
    StatusValueAddUINT32(pConf, (D_UINT32)(pConf->ullDiskBytes / pConf->ulRegionSize));
    StatusValueAddUINT32(pConf, pConf->ulRegionIndex);
    StatusValueAddUINT32(pConf, GetKBPerSecond2(pConf->sDataWritePattern[1].ullTotalBytes, pConf->sDataWritePattern[1].ullTotalUS));
    StatusValueAddUINT32(pConf, GetKBPerSecond2(pConf->sDataReadPattern[1].ullTotalBytes, pConf->sDataReadPattern[1].ullTotalUS));
    StatusValueAddUINT64(pConf, pConf->sDataWritePattern[1].ullTotalBytes);
    StatusValueAddUINT64(pConf, pConf->sDataReadPattern[1].ullTotalBytes);
    StatusValueAddUINT64(pConf, pConf->sDataWritePattern[1].ullTotalUS);
    StatusValueAddUINT64(pConf, pConf->sDataReadPattern[1].ullTotalUS);
    StatusValueDisplay(pConf);

    if(pTConf->fVerbose)
    {
        DclPrintf("Fragments left: %lu\n", ulFragmentsLeft);
        DclPrintf("Offsets left: %lu\n", (D_UINT32)(pConf->ulRegionSize - (ullReadOffset - ullReadRegion)));
    }
}

/*** Sequential Forward ***/
void PatternIO_SequentialForward(
    sStepperConfiguration * pConf,
    D_UINT64    ullRegionOffset)
{
    D_UINT32 ulBytesWritten = 0;
    D_UINT64 ullStartOffset = ullRegionOffset;

    while(ulBytesWritten < pConf->ulRegionSize)
    {
        PatternIO(pConf, pConf->ulPatternIOSize, ullStartOffset);

        ulBytesWritten += pConf->ulPatternIOSize;
        ullStartOffset += pConf->ulPatternIOSize;
    }
}


/*** Region Zero Sequential ***/
void PatternIO_RegionZeroSequential(
    sStepperConfiguration * pConf,
    D_UINT64    ullRegionOffset)
{
    D_UINT32 ulBytesWritten = 0;
    D_UINT64 ullStartOffset = 0;

    while(ulBytesWritten < pConf->ulRegionSize)
    {
        PatternIO(pConf, pConf->ulPatternIOSize, ullStartOffset);

        ulBytesWritten += pConf->ulPatternIOSize;
        ullStartOffset += pConf->ulPatternIOSize;
    }
}


void PatternIO_RegionZeroRandom(
    sStepperConfiguration * pConf,
    D_UINT64    ullRegionOffset)
{
    D_UINT32 ulBytesWritten = 0;
    D_UINT64 ullStartOffset;

    while(ulBytesWritten < pConf->ulRegionSize)
    {
        ullStartOffset = (D_UINT32)(DclRand64(&pConf->pTI->sti.ullRandomSeed) % (pConf->ulRegionSize / pConf->ulPatternIOSize));
        ullStartOffset *= pConf->ulPatternIOSize;

        PatternIO(pConf, pConf->ulPatternIOSize, ullStartOffset);

        ulBytesWritten += pConf->ulPatternIOSize;
    }
}


/*** Region Random Sequential ***/
void PatternIO_RegionRandomSequential(
    sStepperConfiguration * pConf,
    D_UINT64    ullRegionOffset)
{
    D_UINT32 ulBytesWritten = 0;
    D_UINT64 ullStartOffset;

    if(pConf->ulRegionIndex == 0)
    {
        D_UINT64 ullDiskBytes;
        
        ullDiskBytes = pConf->pTI->DevInfo.ullTotalSectors * pConf->pTI->DevInfo.ulBytesPerSector;

        do
        {
            pConf->ullTestInfo = (D_UINT32)(DclRand64(&pConf->pTI->sti.ullRandomSeed) % (ullDiskBytes / pConf->ulRegionSize));;
        } while(pConf->ullTestInfo == 0);
    }

    ullStartOffset = pConf->ullTestInfo * pConf->ulRegionSize;

    while(ulBytesWritten < pConf->ulRegionSize)
    {
#if TEST_VERBOSE
        DclPrintf("RandSeq: %12llu,%10lu\n", ullStartOffset, pConf->ulPatternIOSize);
#endif
        PatternIO(pConf, pConf->ulPatternIOSize, ullStartOffset);

        ulBytesWritten += pConf->ulPatternIOSize;
        ullStartOffset += pConf->ulPatternIOSize;
    }
}


/*** Region Random Random ***/
void PatternIO_RegionRandomRandom(
    sStepperConfiguration * pConf,
    D_UINT64    ullRegionOffset)
{
    D_UINT32 ulBytesWritten = 0;
    D_UINT64 ullStartOffset;
    D_UINT64 ullDiskBytes;
    
    ullDiskBytes = pConf->pTI->DevInfo.ullTotalSectors * pConf->pTI->DevInfo.ulBytesPerSector;


    if(pConf->ulRegionIndex == 0)
    {

        do
        {
            pConf->ullTestInfo = (D_UINT32)(DclRand64(&pConf->pTI->sti.ullRandomSeed) % (ullDiskBytes / pConf->ulRegionSize));
        } while(pConf->ullTestInfo == 0);
    }

    while(ulBytesWritten < pConf->ulRegionSize)
    {
        ullStartOffset = (D_UINT32)(DclRand64(&pConf->pTI->sti.ullRandomSeed) % (pConf->ulRegionSize / pConf->ulPatternIOSize));
        ullStartOffset *= pConf->ulPatternIOSize;
        ullStartOffset += (pConf->ullTestInfo * pConf->ulRegionSize);

#if TEST_VERBOSE
        DclPrintf("RandRand: %16llx - %16llx,%16llx,%10lu\n",
                (D_UINT64)(pConf->ullTestInfo * pConf->ulRegionSize),
                (D_UINT64)((pConf->ullTestInfo * pConf->ulRegionSize) + pConf->ulRegionSize),
                ullStartOffset,
                pConf->ulPatternIOSize);
#endif

        PatternIO(pConf, pConf->ulPatternIOSize, ullStartOffset);

        ulBytesWritten += pConf->ulPatternIOSize;
    }
}


/*** Sequential Forward Log Zero ***/
void PatternIO_SequentialForwardLogZero(
    sStepperConfiguration * pConf,
    D_UINT64    ullRegionOffset)
{
    D_UINT32 ulBytesWritten = 0;
    D_UINT64 ullStartOffset = ullRegionOffset;

    if(pConf->ulRegionIndex == 0)
    {
        pConf->ullTestInfo = 0;
    }
    if(pConf->ullTestInfo == pConf->ulRegionSize / pConf->ulPatternIOSize)
    {
        pConf->ullTestInfo = 0;
    }

    while(ulBytesWritten < pConf->ulRegionSize)
    {
        PatternIO(pConf, pConf->ulPatternIOSize, ullStartOffset);

        ulBytesWritten += pConf->ulPatternIOSize;
        ullStartOffset += pConf->ulPatternIOSize;
    }

    PatternIO(pConf, pConf->ulPatternIOSize, pConf->ullTestInfo * pConf->ulPatternIOSize);
}


/*** Random Read One, Sequential Write One ***/
void PatternIO_ReadOneWriteOne(
    sStepperConfiguration * pConf,
    D_UINT64    ullRegionOffset)
{
    D_UINT32 ulBytesWritten = 0;
    D_UINT64 ullStartOffset = ullRegionOffset;

    D_BOOL fOrigPatternRead = pConf->fEnablePatternRead;
    D_BOOL fOrigPatternWrite = pConf->fEnablePatternWrite;

    pConf->fEnablePatternRead = TRUE;
    pConf->fEnablePatternWrite = TRUE;

    while(ulBytesWritten < pConf->ulRegionSize)
    {
        PatternIO(pConf, pConf->ulPatternIOSize, ullStartOffset);

        ulBytesWritten += pConf->ulPatternIOSize;
        ullStartOffset += pConf->ulPatternIOSize;
    }

    pConf->fEnablePatternRead = fOrigPatternRead;
    pConf->fEnablePatternWrite = fOrigPatternWrite;
}


/*** Pound First ***/
void PatternIO_PoundFirst(
    sStepperConfiguration * pConf,
    D_UINT64    ullRegionOffset)
{
    D_UINT32 ulBytesWritten = 0;
    D_UINT64 ullStartOffset = ullRegionOffset;

    while(ulBytesWritten < pConf->ulRegionSize)
    {
        PatternIO(pConf, pConf->ulPatternIOSize, ullStartOffset);

        ulBytesWritten += pConf->ulPatternIOSize;
    }
}

/*** Alternate First Two ***/
void PatternIO_AlternateFirstTwo(
    sStepperConfiguration * pConf,
    D_UINT64    ullRegionOffset)
{
    D_UINT32 ulBytesWritten = 0;
    D_UINT64 ullStartOffset = ullRegionOffset;

    while(ulBytesWritten < pConf->ulRegionSize)
    {
        PatternIO(pConf, pConf->ulPatternIOSize, ullStartOffset);

        if(ullStartOffset == ullRegionOffset)
        {
            ullStartOffset += pConf->ulPatternIOSize;
        }
        else
        {
            ullStartOffset = ullRegionOffset;
        }

        ulBytesWritten += pConf->ulPatternIOSize;
    }
}

/*** Skip Forward x1 ***/
void PatternIO_SkipForward(
    sStepperConfiguration * pConf,
    D_UINT64    ullRegionOffset)
{
    D_UINT32 ulBytesWritten = 0;
    D_UINT64 ullStartOffset = ullRegionOffset;

    while(ulBytesWritten < pConf->ulRegionSize / 2)
    {
        PatternIO(pConf, pConf->ulPatternIOSize, ullStartOffset);

        ulBytesWritten += pConf->ulPatternIOSize;
        ullStartOffset += pConf->ulPatternIOSize * 2;
    }
}

/*** Skip Forward x3 ***/
void PatternIO_SkipForward3(
    sStepperConfiguration * pConf,
    D_UINT64    ullRegionOffset)
{
    D_UINT32 ulBytesWritten = 0;
    D_UINT64 ullStartOffset = ullRegionOffset;

    while(ulBytesWritten < pConf->ulRegionSize / 4)
    {
        PatternIO(pConf, pConf->ulPatternIOSize, ullStartOffset);

        ulBytesWritten += pConf->ulPatternIOSize;
        ullStartOffset += pConf->ulPatternIOSize * 4;
    }
}

/*** Stripe Forward x1 ***/
void PatternIO_StripeForward(
    sStepperConfiguration * pConf,
    D_UINT64    ullRegionOffset)
{
    D_UINT32 ulBytesWritten = 0;
    D_UINT64 ullStartOffset = ullRegionOffset;

    while(ulBytesWritten < pConf->ulRegionSize)
    {
        PatternIO(pConf, pConf->ulPatternIOSize, ullStartOffset);

        ulBytesWritten += pConf->ulPatternIOSize;
        ullStartOffset += pConf->ulPatternIOSize * 2;
        if(ullStartOffset >= ullRegionOffset + pConf->ulRegionSize)
        {
            ullStartOffset -= pConf->ulRegionSize;
            ullStartOffset += pConf->ulPatternIOSize;
        }
    }
}

/*** Sequential Backward ***/
void PatternIO_SequentialBackward(
    sStepperConfiguration * pConf,
    D_UINT64    ullRegionOffset)
{
    D_UINT32 ulBytesWritten = 0;
    D_UINT64 ullStartOffset = ullRegionOffset + pConf->ulRegionSize - pConf->ulPatternIOSize;

    while(ulBytesWritten < pConf->ulRegionSize)
    {
        PatternIO(pConf, pConf->ulPatternIOSize, ullStartOffset);

        ullStartOffset -= pConf->ulPatternIOSize;

        ulBytesWritten += pConf->ulPatternIOSize;
    }
}


/*** Outside In ***/
void PatternIO_OutsideIn(
    sStepperConfiguration * pConf,
    D_UINT64    ullRegionOffset)
{
    D_UINT32 ulBytesWritten = 0;
    D_UINT64 ullStartOffset = ullRegionOffset;
    D_UINT64 ullEndOffset = ullRegionOffset + pConf->ulRegionSize - pConf->ulPatternIOSize;

    while(ulBytesWritten < pConf->ulRegionSize)
    {
#if TEST_VERBOSE
        DclPrintf("OutsideIn: %16llx - %16llx,%16llx,%16llx,%10lu\n",
                ullRegionOffset,
                ullRegionOffset + pConf->ulRegionSize,
                ullStartOffset,
                ullEndOffset,
                pConf->ulPatternIOSize);
#endif

        PatternIO(pConf, pConf->ulPatternIOSize, ullStartOffset);
        PatternIO(pConf, pConf->ulPatternIOSize, ullEndOffset);

        ullStartOffset += pConf->ulPatternIOSize;
        ullEndOffset -= pConf->ulPatternIOSize;

        ulBytesWritten += pConf->ulPatternIOSize * 2;
    }
}


/*** Outside In Last Last ***/
void PatternIO_OutsideInLastLast(
    sStepperConfiguration * pConf,
    D_UINT64    ullRegionOffset)
{
    D_UINT32 ulBytesWritten = 0;
    D_UINT64 ullStartOffset = ullRegionOffset;
    D_UINT64 ullEndOffset = ullRegionOffset + pConf->ulRegionSize - (pConf->ulPatternIOSize * 2);

    while(ulBytesWritten < (pConf->ulRegionSize - pConf->ulPatternIOSize))
    {
        PatternIO(pConf, pConf->ulPatternIOSize, ullStartOffset);
        PatternIO(pConf, pConf->ulPatternIOSize, ullEndOffset);

        ullStartOffset += pConf->ulPatternIOSize;
        ullEndOffset -= pConf->ulPatternIOSize;

        ulBytesWritten += pConf->ulPatternIOSize * 2;
    }

    PatternIO(pConf, pConf->ulPatternIOSize, ullRegionOffset - pConf->ulPatternIOSize);
}

/*** Random ***/
void PatternIO_Random(
    sStepperConfiguration * pConf,
    D_UINT64    ullRegionOffset)
{
    D_UINT32 ulBytesWritten = 0;
    D_UINT64 ullStartOffset;

    while(ulBytesWritten < pConf->ulRegionSize)
    {
        ullStartOffset = (D_UINT32)(DclRand64(&pConf->pTI->sti.ullRandomSeed) % (pConf->ulRegionSize / pConf->ulPatternIOSize));
        ullStartOffset *= pConf->ulPatternIOSize;
        ullStartOffset += ullRegionOffset;

#if TEST_VERBOSE
        DclPrintf("    Rand: %16llx - %16llx,%16llx,%10lu\n",
                ullRegionOffset,
                ullRegionOffset + pConf->ulRegionSize,
                ullStartOffset,
                pConf->ulPatternIOSize);
#endif

        PatternIO(pConf, pConf->ulPatternIOSize, ullStartOffset);

        ulBytesWritten += pConf->ulPatternIOSize;
    }
}


static fnWritePattern gafnWritePaterns[] = {
    PatternIO_ForwardAllocator,
    PatternIO_SequentialForward,
    PatternIO_SkipForward,
    PatternIO_SkipForward3,
    PatternIO_StripeForward,
    PatternIO_RegionZeroSequential,
    PatternIO_RegionZeroRandom,
    PatternIO_SequentialForwardLogZero,
    PatternIO_RegionRandomSequential,
    PatternIO_OutsideIn,
    PatternIO_Random,
    PatternIO_RegionRandomRandom
};

static char gaszWritePatternNames[][31] =
{
        {"             Forward Allocator"},
        {"            Forward Sequential"},
        {"               Forward Skip x1"},
        {"               Forward Skip x3"},
        {"             Forward Stripe x1"},
        {"        Region Zero Sequential"},
        {"            Region Zero Random"},
        {"   Forward Sequential Log Zero"},
        {"      Region Random Sequential"},
        {"                    Outside In"},
        {"                        Random"},
        {"          Region Random Random"}
};

static char * gaszWritePatternShorts[] =
{
        {"FWAL"},
        {"FWSQ"},
        {"FSX1"},
        {"FSX3"},
        {"FST1"},
        {"RZSQ"},
        {"RZRD"},
        {"FSLZ"},
        {"RRSQ"},
        {"OSIN"},
        {"RNDM"},
        {"RGRR"}
};

#define STEPPER_IO_PATTERN_COUNT (sizeof(gafnWritePaterns) / sizeof(gafnWritePaterns[0]))

static void StepperLegendDisplay(
    sStepperConfiguration * pConf)
{
    D_UINT32    ulIndex;
    DclPrintf("\nWrite Pattern Legend:\n");
    DclPrintf("SHORT                             LONG\n");
    DclPrintf("--------------------------------------\n");

    for(ulIndex = 0; ulIndex < STEPPER_IO_PATTERN_COUNT; ulIndex++)
    {
        if(pConf->pfWritePatterns[ulIndex])
        {
            DclPrintf("%s    %30s\n", gaszWritePatternShorts[ulIndex], gaszWritePatternNames[ulIndex]);
        }
    }
}

static void StepperConfigDisplay(
    sStepperConfiguration * pConf)
{
    D_UINT32 ulPatternRegionSizeMinNum;
    D_UINT32 ulPatternRegionSizeMaxNum;
    D_UINT32 ulPatternIoSizeMinNum;
    D_UINT32 ulPatternIoSizeMaxNum;
    D_UINT32 ulBigIoSizeNum;
    D_UINT32 ulRandomIoSizeNum;
    D_UINT32 ulSanitiseWriteSize;
    char * pszPatternRegionSizeMinStr;
    char * pszPatternRegionSizeMaxStr;
    char * pszPatternIoSizeMinStr;
    char * pszPatternIoSizeMaxStr;
    char * pszBigIoSizeStr;
    char * pszRandomIoSizeStr;
    char * pszSanitiseWriteSizeStr;

    D_UINT32 ulIndex;

    /*  Something's up with DclPrintf and these macros, so for now they need
        to be done this way.
    */
    ulPatternRegionSizeMinNum = BKBMBGB(pConf->ulPatternRegionSizeMin);
    ulPatternRegionSizeMaxNum = BKBMBGB(pConf->ulPatternRegionSizeMax);
    ulPatternIoSizeMinNum = BKBMBGB(pConf->ulPatternIoSizeMin);
    ulPatternIoSizeMaxNum = BKBMBGB(pConf->ulPatternIoSizeMax);
    ulBigIoSizeNum = BKBMBGB(pConf->ulBigWriteSize);
    ulRandomIoSizeNum = BKBMBGB(pConf->ulRandomIOSize);
    ulSanitiseWriteSize = BKBMBGB(pConf->ulSanitiseDiskWriteSize);

    pszPatternRegionSizeMinStr = BKBMBGB_STR(pConf->ulPatternRegionSizeMin);
    pszPatternRegionSizeMaxStr = BKBMBGB_STR(pConf->ulPatternRegionSizeMax);
    pszPatternIoSizeMinStr = BKBMBGB_STR(pConf->ulPatternIoSizeMin);
    pszPatternIoSizeMaxStr = BKBMBGB_STR(pConf->ulPatternIoSizeMax);
    pszBigIoSizeStr = BKBMBGB_STR(pConf->ulBigWriteSize);
    pszRandomIoSizeStr = BKBMBGB_STR(pConf->ulRandomIOSize);
    pszSanitiseWriteSizeStr = BKBMBGB_STR(pConf->ulSanitiseDiskWriteSize);

    DclPrintf("Stepper Test Configuration:\n");
    DclPrintf("(a) Disk Size:                   %10lu Megabytes\n", pConf->ullDiskBytes / (1024*1024));
    DclPrintf("(b) Min Pattern Region size:     %10lu %s\n", ulPatternRegionSizeMinNum, pszPatternRegionSizeMinStr);
    DclPrintf("(c) Max Pattern Region size:     %10lu %s\n", ulPatternRegionSizeMaxNum, pszPatternRegionSizeMaxStr);
    DclPrintf("(d) Min Pattern I/O size:        %10lu %s\n", ulPatternIoSizeMinNum, pszPatternIoSizeMinStr);
    DclPrintf("(e) Max Pattern I/O size:        %10lu %s\n", ulPatternIoSizeMaxNum, pszPatternIoSizeMaxStr);
    DclPrintf("(f) Random I/O Size:             %10lu %s\n", ulRandomIoSizeNum, pszRandomIoSizeStr);
    DclPrintf("(g) Big I/O Size:                %10lu %s\n", ulBigIoSizeNum, pszBigIoSizeStr);
    DclPrintf("(h) Big I/O Frequency:           %10lu\n", pConf->ulBigWriteFrequency);
    DclPrintf("(i) Per-Test Pattern Disk %:     %10lu%%\n", (D_UINT32)((float)pConf->dRepeatDiskFraction * 100));
    DclPrintf("(j) Maximum Time Per Test:       %10lu seconds\n", pConf->ullMaxTimePerTest / 1000000);
    DclPrintf("(k) Sanitise Write Size:         %10lu %s\n", ulSanitiseWriteSize, pszSanitiseWriteSizeStr);
    DclPrintf("(l) Sanitise Write %:            %10lu%%\n", (D_UINT32)((float)pConf->dSanitiseDiskFraction * 100));
    DclPrintf("(m) Sanitise Sleep Seconds:      %10lu\n", pConf->ulSanitiseDiskSleepSeconds);
    DclPrintf("(n) Pattern Write:               %s\n", pConf->fEnablePatternWrite ? "      TRUE" : "     FALSE");
    DclPrintf("(o) Pattern Read:                %s\n", pConf->fEnablePatternRead ? "      TRUE" : "     FALSE");
    DclPrintf("(p) Big Write:                   %s\n", pConf->fEnableBigWrite ? "      TRUE" : "     FALSE");
    DclPrintf("(q) Big Read:                    %s\n", pConf->fEnableBigRead ? "      TRUE" : "     FALSE");
    DclPrintf("(r) Random Read:                 %s\n", pConf->fEnableRandomRead ? "      TRUE" : "     FALSE");
    DclPrintf("(s) Sanitize:                    %s\n", pConf->fEnableCleanseWrite ? "      TRUE" : "     FALSE");

    DclPrintf("(t) I/O Patterns:\n");

    for(ulIndex = 0; ulIndex < STEPPER_IO_PATTERN_COUNT; ulIndex++)
    {
        DclPrintf("(%2lu) %s   %s\n", ulIndex, gaszWritePatternNames[ulIndex], pConf->pfWritePatterns[ulIndex] ? " TRUE" : "FALSE");
    }

    DclPrintf("\n");
}


static void StepperConfigMenu(
    sStepperConfiguration * pConf)
{
    StepperConfigDisplay(pConf);
    DclPrintf("(x) Accept Configuration\n");
    DclPrintf("(z) Display Configuration\n");
    

    while(TRUE)
    {
        char cChar = 0;
        D_BOOL fInputResult = TRUE;


        DclPrintf("Enter a command:");

        if(DclInputChar(pConf->pTI->sti.hDclInst, &cChar, 0) != DCLSTAT_SUCCESS)
        {
            break;
        }

        DclPrintf("\r                                   \r");

        switch(cChar)
        {
            case 'a':
            case 'A':
                {
                    D_UINT64    ullNew;

                    DclPrintf("Enter new disk size: ");
                    fInputResult = StepperGetsUINT64(pConf, &ullNew);
                    if(fInputResult)
                    {
                        pConf->ullDiskBytes = ullNew;
                        DclPrintf("\rNew disk size: %llu          \n", ullNew);
                    }
                    else
                    {
                        DclPrintf("\nInvalid input!\n");
                    }
                }
                break;

            case 'b':
            case 'B':
                {
                    D_UINT64    ullNew;

                    DclPrintf("Enter new min pattern region size: ");
                    fInputResult = StepperGetsUINT64(pConf, &ullNew);
                    if(fInputResult)
                    {
                        DclAssert(ullNew <= D_UINT32_MAX);
                        pConf->ulPatternRegionSizeMin = (D_UINT32)ullNew;
                        DclPrintf("\rNew min pattern region size: %lu          ", (D_UINT32)ullNew);
                    }
                    else
                    {
                        DclPrintf("\nInvalid input!\n");
                    }
                }
                break;

            case 'c':
            case 'C':
                {
                    D_UINT64    ullNew;

                    DclPrintf("Enter new max pattern region size: ");
                    fInputResult = StepperGetsUINT64(pConf, &ullNew);
                    if(fInputResult)
                    {
                        DclAssert(ullNew <= D_UINT32_MAX);
                        pConf->ulPatternRegionSizeMax = (D_UINT32)ullNew;
                        DclPrintf("\rNew max pattern region size: %lu          ", (D_UINT32)ullNew);
                    }
                    else
                    {
                        DclPrintf("\nInvalid input!\n");
                    }
                }
                break;

            case 'd':
            case 'D':
                {
                    D_UINT64    ullNew;

                    DclPrintf("Enter new min pattern I/O size: ");
                    fInputResult = StepperGetsUINT64(pConf, &ullNew);
                    if(fInputResult)
                    {
                        DclAssert(ullNew <= D_UINT32_MAX);
                        pConf->ulPatternIoSizeMin = (D_UINT32)ullNew;
                        DclPrintf("\rNew min pattern I/O size: %lu          ", (D_UINT32)ullNew);
                    }
                    else
                    {
                        DclPrintf("\nInvalid input!\n");
                    }
                }
                break;

            case 'e':
            case 'E':
                {
                    D_UINT64    ullNew;

                    DclPrintf("Enter new max pattern I/O size: ");
                    fInputResult = StepperGetsUINT64(pConf, &ullNew);
                    if(fInputResult)
                    {
                        DclAssert(ullNew <= D_UINT32_MAX);
                        pConf->ulPatternIoSizeMax = (D_UINT32)ullNew;
                        DclPrintf("\rNew max pattern I/O size: %lu          ", (D_UINT32)ullNew);
                    }
                    else
                    {
                        DclPrintf("\nInvalid input!\n");
                    }
                }
                break;
                
            case 'f':
            case 'F':
                {
                    D_UINT64    ullNew;
            
                    DclPrintf("Enter new random I/O size: ");
                    fInputResult = StepperGetsUINT64(pConf, &ullNew);
                    if(fInputResult)
                    {
                        pConf->ulRandomIOSize = (D_UINT32)ullNew;
                        DclPrintf("\rNew random I/O size: %lu          ", (D_UINT32)ullNew);
                    }
                    else
                    {
                        DclPrintf("\nInvalid input!\n");
                    }
                }
                break;
            
            case 'g':
            case 'G':
                {
                    D_UINT64    ullNew;
            
                    DclPrintf("Enter new big I/O size: ");
                    fInputResult = StepperGetsUINT64(pConf, &ullNew);
                    if(fInputResult)
                    {
                        DclAssert(ullNew <= D_UINT32_MAX);
                        pConf->ulBigWriteSize = (D_UINT32)ullNew;
                        DclPrintf("\rNew big I/O size: %lu          ", (D_UINT32)ullNew);
                    }
                    else
                    {
                        DclPrintf("\nInvalid input!\n");
                    }
                }
                break;
            
            case 'h':
            case 'H':
                {
                    D_UINT64    ullNew;
            
                    DclPrintf("Enter new big write frequency: ");
                    fInputResult = StepperGetsUINT64(pConf, &ullNew);
                    if(fInputResult)
                    {
                        DclAssert(ullNew <= D_UINT32_MAX);
                        pConf->ulBigWriteFrequency = (D_UINT32)ullNew;
                        DclPrintf("\rNew big write frequency: %lu          ", (D_UINT32)ullNew);
                    }
                    else
                    {
                        DclPrintf("\nInvalid input!\n");
                    }
                }
                break;
            
            case 'i':
            case 'I':
                {
                    D_UINT64    ullNew;
            
                    DclPrintf("Enter new pattern I/O disk %: ");
                    fInputResult = StepperGetsUINT64(pConf, &ullNew);
                    if(fInputResult)
                    {
                        pConf->dRepeatDiskFraction = (double)ullNew / 100;
                        DclPrintf("\rNew min pattern I/O disk %: %lu          ", (D_UINT32)ullNew);
                    }
                    else
                    {
                        DclPrintf("\nInvalid input!\n");
                    }
                }
                break;
            
            case 'j':
            case 'J':
                {
                    D_UINT64    ullNew;
            
                    DclPrintf("Enter new maximum seconds per test: ");
                    fInputResult = StepperGetsUINT64(pConf, &ullNew);
                    if(fInputResult)
                    {
                        pConf->ullMaxTimePerTest = ullNew * (1000*1000);
                        DclPrintf("\rNew maximum seconds per test: %lu          ", (D_UINT32)ullNew);
                    }
                    else
                    {
                        DclPrintf("\nInvalid input!\n");
                    }
                }
                break;

            case 'k':
            case 'K':
                {
                    D_UINT64    ullNew;

                    DclPrintf("Enter new sanitise write size: ");
                    fInputResult = StepperGetsUINT64(pConf, &ullNew);
                    if(fInputResult)
                    {
                        DclAssert(ullNew <= D_UINT32_MAX);
                        pConf->ulSanitiseDiskWriteSize = (D_UINT32)ullNew;
                        DclPrintf("\rNew sanitise write size: %lu          ", (D_UINT32)ullNew);
                    }
                    else
                    {
                        DclPrintf("\nInvalid input!\n");
                    }
                }
                break;

            case 'l':
            case 'L':
                {
                    D_UINT64    ullNew;

                    DclPrintf("Enter new sanitise write disk %%: ");
                    fInputResult = StepperGetsUINT64(pConf, &ullNew);
                    if(fInputResult)
                    {
                        DclAssert(ullNew <= D_UINT32_MAX);
                        pConf->dSanitiseDiskFraction = (double)ullNew / 100;
                        DclPrintf("\rNew sanitise write disk %%: %lu%%          ", (D_UINT32)ullNew);
                    }
                    else
                    {
                        DclPrintf("\nInvalid input!\n");
                    }
                }
                break;

            case 'm':
            case 'M':
                {
                    D_UINT64    ullNew;

                    DclPrintf("Enter new sanitize sleep seconds: ");
                    fInputResult = StepperGetsUINT64(pConf, &ullNew);
                    if(fInputResult)
                    {
                        DclAssert(ullNew <= D_UINT32_MAX);
                        pConf->ulSanitiseDiskSleepSeconds = (D_UINT32)ullNew;
                        DclPrintf("\rNew sanitise sleep seconds:: %lu          ", (D_UINT32)ullNew);
                    }
                    else
                    {
                        DclPrintf("\nInvalid input!\n");
                    }
                }
                break;

            case 'n':
            case 'N':
                if(pConf->fEnablePatternWrite)
                {
                    pConf->fEnablePatternWrite = FALSE;
                    DclPrintf("Pattern Write: disabled.");
                }
                else
                {
                    pConf->fEnablePatternWrite = TRUE;
                    DclPrintf("Pattern Write: enabled.");
                }
                break;

            case 'o':
            case 'O':
                if(pConf->fEnablePatternRead)
                {
                    pConf->fEnablePatternRead = FALSE;
                    DclPrintf("Pattern Read: disabled.");
                }
                else
                {
                    pConf->fEnablePatternRead = TRUE;
                    DclPrintf("Pattern Read: enabled.");
                }
                break;

            case 'p':
            case 'P':
                if(pConf->fEnableBigWrite)
                {
                    pConf->fEnableBigWrite = FALSE;
                    DclPrintf("Big Write: disabled.");
                }
                else
                {
                    pConf->fEnableBigWrite = TRUE;
                    DclPrintf("Big Write: enabled.");
                }
                break;

            case 'q':
            case 'Q':
                if(pConf->fEnableBigRead)
                {
                    pConf->fEnableBigRead = FALSE;
                    DclPrintf("Big Read: disabled.");
                }
                else
                {
                    pConf->fEnableBigRead = TRUE;
                    DclPrintf("Big Read: enabled.");
                }
                break;

            case 'r':
            case 'R':
                if(pConf->fEnableRandomRead)
                {
                    pConf->fEnableRandomRead = FALSE;
                    DclPrintf("Random Read: disabled.");
                }
                else
                {
                    pConf->fEnableRandomRead = TRUE;
                    DclPrintf("Random Read: enabled.");
                }
                break;

            case 's':
            case 'S':
                if(pConf->fEnableCleanseWrite)
                {
                    pConf->fEnableCleanseWrite = FALSE;
                    DclPrintf("Sanitise disk: disabled.");
                }
                else
                {
                    pConf->fEnableCleanseWrite = TRUE;
                    DclPrintf("Sanitise disk: enabled.");
                }
                break;

            case 't':
            case 'T':
                {
                    D_UINT64    ullPattern;

                    DclPrintf("Enter I/O Pattern to toggle: ");
                    fInputResult = StepperGetsUINT64(pConf, &ullPattern);
                    if(fInputResult)
                    {
                        DclAssert(ullPattern <= D_UINT32_MAX);
                        if((D_UINT32)ullPattern >= STEPPER_IO_PATTERN_COUNT)
                        {
                            DclPrintf("\rInvalid I/O Pattern # %lu           ", (D_UINT32)ullPattern);
                        }
                        else
                        {
                            if(pConf->pfWritePatterns[(D_UINT32)ullPattern])
                            {
                                pConf->pfWritePatterns[(D_UINT32)ullPattern] = FALSE;
                            }
                            else
                            {
                                pConf->pfWritePatterns[(D_UINT32)ullPattern] = TRUE;
                            }

                            DclPrintf("%s: %s.          ",
                                    gaszWritePatternNames[(D_UINT32)ullPattern],
                                    pConf->pfWritePatterns[(D_UINT32)ullPattern] ? "enabled" : "disabled");
                        }
                    }
                    else
                    {
                        DclPrintf("\nInvalid input!\n");
                    }
                }
                break;

            case 'z':
            case 'Z':
                DclPrintf("\n\n");
                StepperConfigDisplay(pConf);
                DclPrintf("(x) Accept Configuration\n");
                DclPrintf("(z) Display Configuration\n");
                break;

            case 'x':
            case 'X':
                DclPrintf("Configuration finalized.\n");
                return;

            default:
                if(cChar != '\n')
                {
                    DclPrintf("Unknown command '%c'.             ", cChar);
                }
                break;
        }

        DclPrintf("\n");
    }

    return;
}

static DCLSTATUS StepperIteration(
    DEVIOTESTINFO  *pTI,
    D_UINT32        ulCount,
    D_UINT64       *pullTimeUS,
    D_UINT64       *pullSectors)
{
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    sStepperConfiguration sConf;
    sStepperConfiguration * pConf = &sConf;


    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertWritePtr(pullTimeUS, sizeof(*pullTimeUS));
    DclAssertWritePtr(pullSectors, sizeof(*pullSectors));
    DclAssert(pTI->ullMaxSectors >= ulCount);

    DclMemSet(&sConf, 0, sizeof(sConf));
    pConf->pTI = pTI;
    pConf->ppszWritePatternShorts = gaszWritePatternShorts;
    pConf->ullDiskBytes = pTI->DevInfo.ullTotalSectors * pTI->DevInfo.ulBytesPerSector;
    pConf->fEnablePatternRead = FALSE;
    pConf->fEnablePatternWrite = TRUE;
    pConf->fEnableBigWrite = FALSE;
    pConf->fEnableBigRead = FALSE;
    pConf->fEnableRandomRead = FALSE;
    pConf->fEnableCleanseWrite = TRUE;
    pConf->dRepeatDiskFraction = 0.25; /* ((double)1 / (double)4); */
    pConf->dSanitiseDiskFraction = 0.05; /* ((double)1 / (double)16); */
    pConf->ulSanitiseDiskWriteSize = 4*1024*1024;
    pConf->ulSanitiseDiskSleepSeconds = 2;
    pConf->ulPatternRegionSizeMin = 4*1024*1024;
    pConf->ulPatternRegionSizeMax = 8*1024*1024;
    pConf->ulPatternIoSizeMin = 64*1024;
    pConf->ulPatternIoSizeMax = 1*1024*1024;
    pConf->ulRandomIOSize = 64*1024;
    pConf->ulBigWriteSize = 4*1024*1024;
    pConf->ulBigWriteFrequency = 16;
    pConf->ullMaxTimePerTest = 30 * (1000*1000);
    pConf->pfWritePatterns = DclMemAllocZero(STEPPER_IO_PATTERN_COUNT * sizeof(*pConf->pfWritePatterns));
    if(!pConf->pfWritePatterns)
    {
        DclPrintf("Malloc failed!\n");
        return DCLSTAT_MEMALLOCFAILED;
    }
    pConf->pfWritePatterns[0] = TRUE;


    DclPrintf("\n\n\n");
    DclPrintf("************************************************************\n");
    DclPrintf("************************************************************\n");
    DclPrintf("**********  WARNING: Epic Hijacking In Progress!  **********\n");
    DclPrintf("**********  WARNING: Epic Hijacking In Progress!  **********\n");
    DclPrintf("**********  WARNING: Epic Hijacking In Progress!  **********\n");
    DclPrintf("**********                   ...                  **********\n");
    DclPrintf("**********   This is not the random write test    **********\n");
    DclPrintf("**********          you are looking for.          **********\n");
    DclPrintf("**********                                        **********\n");
    DclPrintf("**********    Set your screen buffer width to     **********\n");
    DclPrintf("**********       300 and press the anykey.        **********\n");
    DclPrintf("************************************************************\n");
    DclPrintf("************************************************************\n");
    DclPrintf("\n\n\n");

    StepperConfigMenu(pConf);
    StepperLegendDisplay(pConf);

    if(pConf->pfWritePatterns[0] == TRUE)
    {
        FWAL_ConfigMenu(pConf);
    }

    DclPrintf("\nStepper test starting...\n");


    {
        DclPrintf("\n");
        StatusInit(pConf);
        StatusHeadingAdd(pConf, "Pattern", 4, FALSE);
        StatusHeadingAdd(pConf, "Region Size", 10, FALSE);
        StatusHeadingAdd(pConf, "I/O Size", 10, FALSE);
        StatusHeadingAdd(pConf, "Region Total", 10, FALSE);
        StatusHeadingAdd(pConf, "Region Index", 10, FALSE);
        StatusHeadingAdd(pConf, "Write Pattern KBPS", 7, FALSE);
        StatusHeadingAdd(pConf, "Read Pattern KBPS", 7, FALSE);
        if(pConf->fEnableBigRead)
        {
            StatusHeadingAdd(pConf, "Read Big KBPS", 7, FALSE);
        }
        if(pConf->fEnableBigWrite)
        {
            StatusHeadingAdd(pConf, "Write Big KBPS", 7, FALSE);
        }
        if(pConf->fEnableRandomRead)
        {
            StatusHeadingAdd(pConf, "Read Random KBPS", 7, FALSE);
        }
        StatusHeadingDisplay(pConf);
        

        for(pConf->ulPatternIOSize = pConf->ulPatternIoSizeMin;
            pConf->ulPatternIOSize <= pConf->ulPatternIoSizeMax;
            pConf->ulPatternIOSize *= 2)
        {
            DclMemSet(&pConf->sDataReadBig[3], 0, sizeof(pConf->sDataReadBig[3]));
            DclMemSet(&pConf->sDataReadPattern[3], 0, sizeof(pConf->sDataReadPattern[3]));
            DclMemSet(&pConf->sDataReadRandom[3], 0, sizeof(pConf->sDataReadRandom[3]));
            DclMemSet(&pConf->sDataWriteBig[3], 0, sizeof(pConf->sDataWriteBig[3]));
            DclMemSet(&pConf->sDataWritePattern[3], 0, sizeof(pConf->sDataWritePattern[3]));

            for(pConf->ulRegionSize = pConf->ulPatternRegionSizeMin;
                pConf->ulRegionSize <= pConf->ulPatternRegionSizeMax;
                pConf->ulRegionSize *= 2)
            {
                if(pConf->ulPatternIOSize > pConf->ulRegionSize)
                {
                    continue;
                }

                DclMemSet(&pConf->sDataReadBig[2], 0, sizeof(pConf->sDataReadBig[2]));
                DclMemSet(&pConf->sDataReadPattern[2], 0, sizeof(pConf->sDataReadPattern[2]));
                DclMemSet(&pConf->sDataReadRandom[2], 0, sizeof(pConf->sDataReadRandom[2]));
                DclMemSet(&pConf->sDataWriteBig[2], 0, sizeof(pConf->sDataWriteBig[2]));
                DclMemSet(&pConf->sDataWritePattern[2], 0, sizeof(pConf->sDataWritePattern[2]));

                for(pConf->ulIOPattern = 0;
                    pConf->ulIOPattern < STEPPER_IO_PATTERN_COUNT;
                    pConf->ulIOPattern++)
                {
                    D_UINT32 ulRegionCount;
                    DCLTIMESTAMP ts;

                    if(pConf->pfWritePatterns[pConf->ulIOPattern] == FALSE)
                    {
                        continue;
                    }

                    DclMemSet(&pConf->sDataReadBig[1], 0, sizeof(pConf->sDataReadBig[1]));
                    DclMemSet(&pConf->sDataReadPattern[1], 0, sizeof(pConf->sDataReadPattern[1]));
                    DclMemSet(&pConf->sDataReadRandom[1], 0, sizeof(pConf->sDataReadRandom[1]));
                    DclMemSet(&pConf->sDataWriteBig[1], 0, sizeof(pConf->sDataWriteBig[1]));
                    DclMemSet(&pConf->sDataWritePattern[1], 0, sizeof(pConf->sDataWritePattern[1]));
                    

                    if(pConf->fEnableCleanseWrite)
                    {
                        DiskSanitise(pConf, pConf->dSanitiseDiskFraction, pConf->ulSanitiseDiskWriteSize);
                    }


                    ulRegionCount = (D_UINT32)((pConf->ullDiskBytes * pConf->dRepeatDiskFraction) / pConf->ulRegionSize);


                    /*  For now, ignore cleanse time.
                    */
                    ts = DclTimeStamp();

                    for(pConf->ulRegionIndex = 0; pConf->ulRegionIndex < ulRegionCount; pConf->ulRegionIndex++)
                    {
                        D_UINT64 ullRegionOffset;

                        DclMemSet(&pConf->sDataReadBig[0], 0, sizeof(pConf->sDataReadBig[0]));
                        DclMemSet(&pConf->sDataReadPattern[0], 0, sizeof(pConf->sDataReadPattern[0]));
                        DclMemSet(&pConf->sDataReadRandom[0], 0, sizeof(pConf->sDataReadRandom[0]));
                        DclMemSet(&pConf->sDataWriteBig[0], 0, sizeof(pConf->sDataWriteBig[0]));
                        DclMemSet(&pConf->sDataWritePattern[0], 0, sizeof(pConf->sDataWritePattern[0]));

       
                        ullRegionOffset = (D_UINT32)(DclRand64(&pTI->sti.ullRandomSeed) % (pConf->ullDiskBytes / pConf->ulRegionSize));


                        if( pConf->fEnableBigWrite &&
                            pConf->ulBigWriteFrequency &&
                            pConf->ulRegionIndex &&
                            (pConf->ulRegionIndex % pConf->ulBigWriteFrequency) == 0)
                        {
                            D_UINT64 ullBigOffset;

                            ullBigOffset = DclRand64(&pTI->sti.ullRandomSeed) % (pConf->ullDiskBytes / pConf->ulBigWriteSize);
                            ullBigOffset *= pConf->ulBigWriteSize;

                            DiskWrite(pConf, pConf->ulBigWriteSize, ullBigOffset, &pConf->sDataWriteBig[0].ullTotalUS, &pConf->sDataWriteBig[0].ullTotalBytes);
                        }

                        if( pConf->fEnableBigRead &&
                            pConf->ulBigWriteFrequency &&
                            pConf->ulRegionIndex &&
                            (pConf->ulRegionIndex % pConf->ulBigWriteFrequency) == 0)
                        {
                            D_UINT64 ullBigOffset;

                            ullBigOffset = DclRand64(&pTI->sti.ullRandomSeed) % (pConf->ullDiskBytes / pConf->ulBigWriteSize);
                            ullBigOffset *= pConf->ulBigWriteSize;

                            DiskRead(pConf, pConf->ulBigWriteSize, ullBigOffset, &pConf->sDataReadBig[0].ullTotalUS, &pConf->sDataReadBig[0].ullTotalBytes);
                        }

                        if(pConf->fEnablePatternWrite || pConf->fEnablePatternRead)
                        {
                            (gafnWritePaterns[pConf->ulIOPattern])(pConf, ullRegionOffset * pConf->ulRegionSize);
                        }


                        pConf->sDataReadBig[1].ullTotalBytes        += pConf->sDataReadBig[0].ullTotalBytes;
                        pConf->sDataReadBig[1].ullTotalUS           += pConf->sDataReadBig[0].ullTotalUS;
                        pConf->sDataReadPattern[1].ullTotalBytes    += pConf->sDataReadPattern[0].ullTotalBytes;
                        pConf->sDataReadPattern[1].ullTotalUS       += pConf->sDataReadPattern[0].ullTotalUS;
                        pConf->sDataReadRandom[1].ullTotalBytes     += pConf->sDataReadRandom[0].ullTotalBytes;
                        pConf->sDataReadRandom[1].ullTotalUS        += pConf->sDataReadRandom[0].ullTotalUS;
                        pConf->sDataWriteBig[1].ullTotalBytes       += pConf->sDataWriteBig[0].ullTotalBytes;
                        pConf->sDataWriteBig[1].ullTotalUS          += pConf->sDataWriteBig[0].ullTotalUS;
                        pConf->sDataWritePattern[1].ullTotalBytes   += pConf->sDataWritePattern[0].ullTotalBytes;
                        pConf->sDataWritePattern[1].ullTotalUS      += pConf->sDataWritePattern[0].ullTotalUS;


#if 0
                        StatusValueAddString(pConf, gaszWritePatternShorts[pConf->ulIOPattern]);
                        StatusValueAddUINT32(pConf, pConf->ulRegionSize);
                        StatusValueAddUINT32(pConf, pConf->ulPatternIOSize);
                        StatusValueAddUINT32(pConf, ulRegionCount);
                        StatusValueAddUINT32(pConf, pConf->ulRegionIndex);
                        StatusValueAddUINT32(pConf, GetKBPerSecond2(pConf->sDataWritePattern[1].ullTotalBytes,  pConf->sDataWritePattern[1].ullTotalUS));
                        StatusValueAddUINT32(pConf, GetKBPerSecond2(pConf->sDataReadPattern[1].ullTotalBytes,   pConf->sDataReadPattern[1].ullTotalUS));
                        if(pConf->fEnableBigRead)
                        {
                            StatusValueAddUINT32(pConf, GetKBPerSecond2(pConf->sDataReadBig[1].ullTotalBytes,       pConf->sDataReadBig[1].ullTotalUS));
                        }
                        if(pConf->fEnableBigWrite)
                        {
                            StatusValueAddUINT32(pConf, GetKBPerSecond2(pConf->sDataWriteBig[1].ullTotalBytes,      pConf->sDataWriteBig[1].ullTotalUS));
                        }
                        if(pConf->fEnableRandomRead)
                        {
                            StatusValueAddUINT32(pConf, GetKBPerSecond2(pConf->sDataReadRandom[1].ullTotalBytes,    pConf->sDataReadRandom[1].ullTotalUS));
                        }
                        StatusValueDisplay(pConf);
#endif
#if 0
                        DclPrintf("\r");
                        DclPrintf("%s, %10lu,%10lu,%10lu,%10lu,", gaszWritePatternShorts[pConf->ulIOPattern], pConf->ulRegionSize, pConf->ulPatternIOSize, ulRegionCount, pConf->ulRegionIndex);
                        DclPrintf("%10lu,", GetKBPerSecond2(pConf->sDataWritePattern[1].ullTotalBytes,  pConf->sDataWritePattern[1].ullTotalUS));
                        DclPrintf("%10lu,", GetKBPerSecond2(pConf->sDataWriteBig[1].ullTotalBytes,      pConf->sDataWriteBig[1].ullTotalUS));
                        DclPrintf("%10lu,", GetKBPerSecond2(pConf->sDataReadPattern[1].ullTotalBytes,   pConf->sDataReadPattern[1].ullTotalUS));
                        DclPrintf("%10lu,", GetKBPerSecond2(pConf->sDataReadBig[1].ullTotalBytes,       pConf->sDataReadBig[1].ullTotalUS));
                        DclPrintf("%10lu",  GetKBPerSecond2(pConf->sDataReadRandom[1].ullTotalBytes,    pConf->sDataReadRandom[1].ullTotalUS));
#endif

#if 0
                        if(kbhit())
                        {
                            int c = getch();

                            switch(c)
                            {
                                default:
                                    pConf->ulBreakLevels = 1;
                            }
                        }
                        else
#endif
                        if(DclTimePassedUS(ts) > pConf->ullMaxTimePerTest)
                        {
                            pConf->ulBreakLevels = 1;
                        }

                        if(pConf->ulBreakLevels)
                        {
                            pConf->ulBreakLevels--;
                            break;
                        }
                    }

                    pConf->sDataReadBig[2].ullTotalBytes += pConf->sDataReadBig[1].ullTotalBytes;
                    pConf->sDataReadBig[2].ullTotalUS += pConf->sDataReadBig[1].ullTotalUS;
                    pConf->sDataReadPattern[2].ullTotalBytes += pConf->sDataReadPattern[1].ullTotalBytes;
                    pConf->sDataReadPattern[2].ullTotalUS += pConf->sDataReadPattern[1].ullTotalUS;
                    pConf->sDataReadRandom[2].ullTotalBytes += pConf->sDataReadRandom[1].ullTotalBytes;
                    pConf->sDataReadRandom[2].ullTotalUS += pConf->sDataReadRandom[1].ullTotalUS;
                    pConf->sDataWriteBig[2].ullTotalBytes += pConf->sDataWriteBig[1].ullTotalBytes;
                    pConf->sDataWriteBig[2].ullTotalUS += pConf->sDataWriteBig[1].ullTotalUS;
                    pConf->sDataWritePattern[2].ullTotalBytes += pConf->sDataWritePattern[1].ullTotalBytes;
                    pConf->sDataWritePattern[2].ullTotalUS += pConf->sDataWritePattern[1].ullTotalUS;

                    /*
                    */
                    DclPrintf("\n");

                    if(pConf->ulBreakLevels)
                    {
                        pConf->ulBreakLevels--;
                        break;
                    }
                }

                pConf->sDataReadBig[3].ullTotalBytes += pConf->sDataReadBig[2].ullTotalBytes;
                pConf->sDataReadBig[3].ullTotalUS += pConf->sDataReadBig[2].ullTotalUS;
                pConf->sDataReadPattern[3].ullTotalBytes += pConf->sDataReadPattern[2].ullTotalBytes;
                pConf->sDataReadPattern[3].ullTotalUS += pConf->sDataReadPattern[2].ullTotalUS;
                pConf->sDataReadRandom[3].ullTotalBytes += pConf->sDataReadRandom[2].ullTotalBytes;
                pConf->sDataReadRandom[3].ullTotalUS += pConf->sDataReadRandom[2].ullTotalUS;
                pConf->sDataWriteBig[3].ullTotalBytes += pConf->sDataWriteBig[2].ullTotalBytes;
                pConf->sDataWriteBig[3].ullTotalUS += pConf->sDataWriteBig[2].ullTotalUS;
                pConf->sDataWritePattern[3].ullTotalBytes += pConf->sDataWritePattern[2].ullTotalBytes;
                pConf->sDataWritePattern[3].ullTotalUS += pConf->sDataWritePattern[2].ullTotalUS;

                if(pConf->ulBreakLevels)
                {
                    pConf->ulBreakLevels--;
                    break;
                }
            }

            pConf->sDataReadBig[4].ullTotalBytes += pConf->sDataReadBig[3].ullTotalBytes;
            pConf->sDataReadBig[4].ullTotalUS += pConf->sDataReadBig[3].ullTotalUS;
            pConf->sDataReadPattern[4].ullTotalBytes += pConf->sDataReadPattern[3].ullTotalBytes;
            pConf->sDataReadPattern[4].ullTotalUS += pConf->sDataReadPattern[3].ullTotalUS;
            pConf->sDataReadRandom[4].ullTotalBytes += pConf->sDataReadRandom[3].ullTotalBytes;
            pConf->sDataReadRandom[4].ullTotalUS += pConf->sDataReadRandom[3].ullTotalUS;
            pConf->sDataWriteBig[4].ullTotalBytes += pConf->sDataWriteBig[3].ullTotalBytes;
            pConf->sDataWriteBig[4].ullTotalUS += pConf->sDataWriteBig[3].ullTotalUS;
            pConf->sDataWritePattern[4].ullTotalBytes += pConf->sDataWritePattern[3].ullTotalBytes;
            pConf->sDataWritePattern[4].ullTotalUS += pConf->sDataWritePattern[3].ullTotalUS;
            
            if(pConf->ulBreakLevels)
            {
                pConf->ulBreakLevels--;
                break;
            }
        }
    }


    *pullSectors = 100;
    *pullTimeUS = 1000000;

    return dclStat;
}
#else
static DCLSTATUS StepperIteration(
    DEVIOTESTINFO  *pTI,
    D_UINT32        ulCount,
    D_UINT64       *pullTimeUS,
    D_UINT64       *pullSectors)
{
    (void)pTI;
    (void)ulCount;
    (void)pullTimeUS;
    (void)pullSectors;

    *pullSectors = 100;
    *pullTimeUS = 1000000;

    return DCLSTAT_SUCCESS;
}
#endif

