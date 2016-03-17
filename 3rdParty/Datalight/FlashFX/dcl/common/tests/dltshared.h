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

    This header contains settings and interface descriptions for things
    which are shared by multiple tests implemented in this directory.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltshared.h $
    Revision 1.6  2011/03/29 00:56:31Z  garyp
    Updated to allow the /TIME option to be specified in hours.
    Revision 1.5  2010/04/14 22:20:59Z  garyp
    Changed nVerbosity to not be a bit-field.
    Revision 1.4  2010/04/12 03:19:08Z  garyp
    Include dlperflog.h if needed.  Protected against multiple inclusions.
    Revision 1.3  2009/11/02 16:43:17Z  garyp
    Added common help definitions for the shared test parameters.
    Revision 1.2  2009/10/19 19:58:02Z  garyp
    Updated to support a 64-bit random seed.
    Revision 1.1  2009/09/10 16:59:22Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLTSHARED_H_INCLUDED
#define DLTSHARED_H_INCLUDED

#if DCLCONF_OUTPUT_ENABLED
#include <dlperflog.h>
#endif


#define MAX_FILESPEC_LEN              (256)
#define DEFAULT_LOG_BUFFER_LEN          (4)


typedef struct
{
    DCLINSTANCEHANDLE   hDclInst;
    D_UINT32            ulTestSeconds;
    D_UINT64            ullRandomSeed;
    D_BUFFER           *pStatsBuff;
    const char         *pszDeviceName;
    unsigned            nLogBuffLenKB;
  #if DCLCONF_OUTPUT_ENABLED
    char                szPerfLogSuffix[PERFLOG_MAX_SUFFIX_LEN];
    DCLLOGHANDLE        hLog;
    char                szLogFile[MAX_FILESPEC_LEN];
  #endif
    DCLOSREQHANDLE      hReqBD;             /* BlockDev requestor handle */
    DCLOSREQHANDLE      hReqFS;             /* Filesys requestor handle */
    unsigned            nVerbosity;
    unsigned            fPerfLog        : 1;
    unsigned            fLogShadow      : 1;
    unsigned            fTrace          : 1;
    unsigned            fProfile        : 1;
    unsigned            fProfileFS      : 1;
    unsigned            fProfileBD      : 1;
    unsigned            fStats          : 1;
    unsigned            fReqDevDisabled : 1;
    unsigned            fReqFSDisabled  : 1;
} DCLSHAREDTESTINFO;


D_BOOL      DclTestParseParam(          DCLSHAREDTESTINFO *pTI, const char *pszParam);
DCLSTATUS   DclTestLogOpen(             DCLSHAREDTESTINFO *pTI);
DCLSTATUS   DclTestLogClose(      const DCLSHAREDTESTINFO *pTI);
DCLSTATUS   DclTestRequestorOpen(       DCLSHAREDTESTINFO *pTI, const char *pszPath, const char *pszDevName);
DCLSTATUS   DclTestRequestorClose(      DCLSHAREDTESTINFO *pTI);
void        DclTestProfilerStart(       DCLSHAREDTESTINFO *pTI);
void        DclTestProfilerStop(        DCLSHAREDTESTINFO *pTI);
void        DclTestTraceStart(          DCLSHAREDTESTINFO *pTI);
void        DclTestTraceStop(           DCLSHAREDTESTINFO *pTI);
void        DclTestInstrumentationStart(DCLSHAREDTESTINFO *pTI);
void        DclTestInstrumentationStop( DCLSHAREDTESTINFO *pTI, const char *pszCategorySuffix);
DCLSTATUS   DclTestStatsReset(          DCLSHAREDTESTINFO *pTI);
DCLSTATUS   DclTestStatsDump(           DCLSHAREDTESTINFO *pTI, const char *pszCategorySuffix);
D_BOOL      DclTestPressEnter(DCLINSTANCEHANDLE hDclInst);


/*-------------------------------------------------------------------
    The following section contains macros for implementing the
    standard help for the shared test parameters.  Note that 
    tests may forgo using any of these macros as needed, for 
    tests where the parameter behavior may be different than 
    the default behavior defined here.
-------------------------------------------------------------------*/

#define DLTSHAREDTESTHELP_TIME()                                                                    \
    DclPrintf("  /Time:n[s|m|h] Specifies the amount of Time (in Seconds, Minutes, or Hours)\n");   \
    DclPrintf("                 to run the tests which are timed (default is %u seconds).\n", DEFAULT_TEST_SECONDS);

#define DLTSHAREDTESTHELP_LOG()                                                                     \
    DclPrintf("  /Log:'file'    The log file specification (default to the standard display).\n");

#define DLTSHAREDTESTHELP_LB()                                                                      \
    DclPrintf("  /LB:size       The log output buffer length (default is %uKB).\n", DEFAULT_LOG_BUFFER_LEN);

#define DLTSHAREDTESTHELP_LS()                                                                      \
    DclPrintf("  /LS            Shadow the output to the standard device as well the log file.\n");

#define DLTSHAREDTESTHELP_PERFLOG()                                                                 \
    DclPrintf("  /PerfLog[:sfx] Write test results in CSV form, in addition to the standard\n");    \
    DclPrintf("                 test output.  Normally this is used in combination with /LOG,\n");  \
    DclPrintf("                 and it modifies the logging behavior such that the standard\n");    \
    DclPrintf("                 test output goes to the screen, and the CSV output goes to the\n"); \
    DclPrintf("                 log file.  If /LOG is not used, both types of output will go\n");   \
    DclPrintf("                 to the screen.  The optional 'sfx' value is a text suffix to\n");   \
    DclPrintf("                 append to the build number in the PerfLog CSV output.\n");
    
#define DLTSHAREDTESTHELP_STATS()                                                                   \
    DclPrintf("  /Stats         Reset and display block device and file system statistics (if\n");  \
    DclPrintf("                 available) before and after each test.\n");
    
#define DLTSHAREDTESTHELP_TRACE()                                                                   \
    DclPrintf("  /Trace         Enable and disable tracing around each test (if available).\n");

#define DLTSHAREDTESTHELP_PROF()                                                                    \
    DclPrintf("  /Prof          Start and stop the profiler before and after each test.\n");

#define DLTSHAREDTESTHELP_REQ()                                                                     \
    DclPrintf("  /Req:opt       Control the external requestor interface, where 'opt' is:\n");      \
    DclPrintf("                    Off - Disable the requestor interface entirely.\n");             \
    DclPrintf("                   NoFS - Disable the requestor interface to the File System.\n");   \
    DclPrintf("                   NoBD - Disable the requestor interface to the Block Device.\n");

#define DLTSHAREDTESTHELP_SEED()                                                                    \
    DclPrintf("  /Seed:n        Specifies the random seed to use (default [0] is time stamp).\n");

#define DLTSHAREDTESTHELP_VERBOSITY()                                                               \
    DclPrintf("  /V:n           The Verbosity level 0-3 (Default=1, 3 affects perf results).\n");


#endif  /* DLTSHARED_H_INCLUDED */

