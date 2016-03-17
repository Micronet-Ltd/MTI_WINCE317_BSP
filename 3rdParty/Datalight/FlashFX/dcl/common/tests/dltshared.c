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

    This module contains code which is shared by multiple tests which are
    implemented in this directory.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltshared.c $
    Revision 1.11  2011/03/29 00:56:14Z  garyp
    Updated to allow the /TIME option to be specified in hours.
    Revision 1.10  2010/11/01 03:11:31Z  garyp
    Modified DclLogOpen() to take a flags parameter rather than a whole
    bunch of bools.  Updated to support binary logs.  Renamed DclLogWrite()
    to DclLogWriteString() to avoid confusion with the new logging capability
    of handling binary logs.
    Revision 1.9  2010/07/16 01:32:33Z  garyp
    Added some diagnostics messages.
    Revision 1.8  2010/04/27 23:42:53Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.7  2010/04/17 22:34:42Z  garyp
    Updated to allow an extra, undocumented level of verbosity (4).  Some
    variables renamed for clarity -- no other functional changes.
    Revision 1.6  2009/11/08 16:21:05Z  garyp
    Updated to use some renamed symbols -- no functional changes.
    Revision 1.5  2009/11/02 16:43:18Z  garyp
    Updated documentation and updated debugging code.  Use the
    preferred structure initialization mechanism.
    Revision 1.4  2009/10/19 19:58:02Z  garyp
    Updated to support a 64-bit random seed.
    Revision 1.3  2009/09/17 21:52:06Z  garyp
    Added a missing definition of DCLOUTPUT_BUFFERLEN.
    Revision 1.2  2009/09/14 17:38:41Z  garyp
    Tweaked a message.
    Revision 1.1  2009/09/10 16:59:20Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapireq.h>
#include <dlstats.h>
#include <dlperflog.h>
#include "dltshared.h"

#ifndef DCLOUTPUT_BUFFERLEN
#define DCLOUTPUT_BUFFERLEN           (256)
#endif

static DCLSTATUS DumpStatsForDevice(DCLSHAREDTESTINFO *pSTI, const char *pszCategorySuffix, DCLOSREQHANDLE hReq);


/*-------------------------------------------------------------------
    Private: DclTestParseParam()

    Parse command-line parameters which are shared by multiple tests.
    This function handles the following paramters:
        /LB:n             - The log buffer size
        /LOG:name         - Log to a file
        /LS               - Enable log shadowing
        /PERFLOG[:suffix] - PerfLog settings
        /PROF             - Enable profileing
        /REQ:mode         - Requestor settings
        /SEED:n           - Random seed
        /STATS            - Enable stats gathering
        /TIME:n[s|m]      - Time in seconds or minutes
        /TRACE            - Enable tracing
        /V:n              - Verbosity level

    The settings are placed into the supplied DCLSHAREDTESTINFO
    structure.  Note that (at this time) syntax help for these 
    commands must be implemented discretely within each test.

    NOTE!  If the /STATS option is used, a stats buffer will be 
           allocated and the pointer stored in pStatsBuff.  It is
           the responsibility of the caller to free this buffer
           at shutdown time, or a memory leak will result.

    Parameters:
        pSTI      - A pointer to the DCLSHAREDTESTINFO structure to use.
        pszParam - A pointer to the parameter to process.

    Return Value:
        Returns TRUE if the parameter was valid and properly parsed,
        or FALSE if it was not recognized.
-------------------------------------------------------------------*/
D_BOOL DclTestParseParam(
    DCLSHAREDTESTINFO  *pSTI,
    const char         *pszParam)
{
    DclAssertWritePtr(pSTI, sizeof(*pSTI));
    DclAssertReadPtr(pszParam, 0);

    if(DclStrNICmp(pszParam, "/SEED:", 6) == 0)
    {
        pSTI->ullRandomSeed = DclAtoI(pszParam+6);
        if(pSTI->ullRandomSeed == 0)
            DclPrintf("A random seed value of 0 will use the current time.\n\n");

        return TRUE;
    }
    if(DclStrNICmp(pszParam, "/V:", 3) == 0)
    {
        unsigned nLevel;

        /*  Note that this logic is intentionally allowing an undocumented
            extra level of verbosity -- DCL_VERBOSE_OBNOXIOUS+1.
        */
        nLevel = DclAtoL(pszParam+3);
        if(nLevel > DCL_VERBOSE_OBNOXIOUS+1)
            goto BadOption;

        pSTI->nVerbosity = nLevel;

        return TRUE;
    }
    if(DclStrNICmp(pszParam, "/TIME:", 6) == 0)
    {
        const char *pStr = pszParam+6;

        pSTI->ulTestSeconds = DclAtoL(pStr);

        while(DclIsDigit(*pStr))
            pStr++;

        /*  Default is for seconds.  Only convert from minutes or
            hours if the right suffix is included.
        */
        if(DclToUpper(*pStr) == 'H' && *(pStr+1) == 0)
        {
            pSTI->ulTestSeconds *= (60*60);
        }
        else if(DclToUpper(*pStr) == 'M' && *(pStr+1) == 0)
        {
            pSTI->ulTestSeconds *= 60;
        }
        else if(!((DclToUpper(*pStr) == 'S' && *(pStr+1) == 0) || *pStr == 0))
        {
            /*  Error if there is an unrecognized suffix
            */
            goto BadOption;
        }

        return TRUE;
    }
    else if(DclStrNICmp(pszParam, "/REQ:", 5) == 0)
    {
        if(DclStrICmp(&pszParam[5], "OFF") == 0)
        {
            pSTI->fReqDevDisabled = TRUE;
            pSTI->fReqFSDisabled = TRUE;
        }
        else if(DclStrICmp(&pszParam[5], "NOFS") == 0)
        {
            pSTI->fReqFSDisabled = TRUE;
        }
        else if(DclStrICmp(&pszParam[5], "NOBD") == 0)
        {
            pSTI->fReqDevDisabled = TRUE;
        }
        else
        {
            goto BadOption;
        }

        return TRUE;
    }
  #if DCLCONF_OUTPUT_ENABLED
    else if(DclStrNICmp(pszParam, "/PERFLOG", 8) == 0)
    {
        pSTI->fPerfLog = TRUE;

        if(pszParam[8] == ':')
        {
            DclStrNCpy(pSTI->szPerfLogSuffix, &pszParam[9], sizeof(pSTI->szPerfLogSuffix));
            pSTI->szPerfLogSuffix[sizeof(pSTI->szPerfLogSuffix)-1] = 0;
        }

        return TRUE;
    }
    else if(DclStrICmp(pszParam, "/STATS") == 0)
    {
        pSTI->pStatsBuff = DclMemAlloc(DCLOUTPUT_BUFFERLEN * 2);
        if(pSTI->pStatsBuff)
            pSTI->fStats = TRUE;
        else
            DclPrintf("Error allocating stats buffer... continuing\n");

        return TRUE;
    }
    else if(DclStrNICmp(pszParam, "/LOG:", 5) == 0)
    {
        if((pszParam[5] == '"') || (pszParam[5] == '\''))
            DclStrNCpy(pSTI->szLogFile, &pszParam[6], DclStrLen(&pszParam[6]) - 1);
        else
            DclStrNCpy(pSTI->szLogFile, &pszParam[5], DclStrLen(&pszParam[5]));

        return TRUE;
    }
    else if(DclStrNICmp(pszParam, "/LB:", 4) == 0)
    {
        const char *pTmp;
        D_UINT32    ulResult;

        pTmp = DclSizeToUL(&pszParam[4], &ulResult);
        if(!pTmp || *pTmp)
            goto BadOption;

        if(ulResult / 1024UL > UINT_MAX)
        {
            DclPrintf("The log buffer size (/LB:n) is too large.\n\n");
            return FALSE;
        }

        /*  Cast is safe due to the check above
        */
        pSTI->nLogBuffLenKB = (unsigned)(ulResult / 1024UL);
        return TRUE;
    }
    else if(DclStrICmp(pszParam, "/LS") == 0)
    {
        pSTI->fLogShadow = TRUE;
        return TRUE;
    }
    else if(DclStrICmp(pszParam, "/TRACE") == 0)
    {
        pSTI->fTrace = TRUE;
        return TRUE;
    }
  #endif
    else if(DclStrICmp(pszParam, "/PROF") == 0)
    {
        pSTI->fProfile = TRUE;
        pSTI->fProfileFS = TRUE;
        pSTI->fProfileBD = TRUE;
        return TRUE;
    }
    else
    {
        return FALSE;
    }

  BadOption:
    DclPrintf("Syntax, overflow, or range error in: \"%s\"\n\n", pszParam);
    return FALSE;
}


/*-------------------------------------------------------------------
    Private: DclTestLogOpen()

    Parameters:
        pSTI - A pointer to the DCLSHAREDTESTINFO structure to use.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclTestLogOpen(
    DCLSHAREDTESTINFO  *pSTI)
{
    DclAssertWritePtr(pSTI, sizeof(*pSTI));

  #if DCLCONF_OUTPUT_ENABLED
    if(*pSTI->szLogFile)
    {
        D_UINT32    ulFlags = 0;

        if(pSTI->fLogShadow)
            ulFlags |= DCLLOGFLAGS_SHADOW;
        
        if(pSTI->fPerfLog)
            ulFlags |= DCLLOGFLAGS_PRIVATE;
        
        if(!pSTI->nLogBuffLenKB)
            pSTI->nLogBuffLenKB = DEFAULT_LOG_BUFFER_LEN;

        pSTI->hLog = DclLogOpen(pSTI->hDclInst, pSTI->szLogFile, pSTI->nLogBuffLenKB, ulFlags);
        if(!pSTI->hLog)
            return DCLSTAT_LOG_OPENFAILED;
        else
            return DCLSTAT_SUCCESS;
    }
    
    return DCLSTAT_FEATUREDISABLED;

  #else

    (void)pSTI;
    return DCLSTAT_FEATUREDISABLED;

  #endif
}


/*-------------------------------------------------------------------
    Private: DclTestLogClose()

    Parameters:
        pSTI - A pointer to the DCLSHAREDTESTINFO structure to use.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclTestLogClose(
    const DCLSHAREDTESTINFO  *pSTI)
{
    DclAssertReadPtr(pSTI, sizeof(*pSTI));

  #if DCLCONF_OUTPUT_ENABLED
    if(pSTI->hLog)
        return DclLogClose(pSTI->hDclInst, pSTI->hLog);
    else
        return DCLSTAT_SUCCESS;
  #else

    (void)pSTI;
    return DCLSTAT_FEATUREDISABLED;

  #endif
}


/*-------------------------------------------------------------------
    Private: DclTestRequestorOpen()

    Parameters:
        pSTI - A pointer to the DCLSHAREDTESTINFO structure to use.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclTestRequestorOpen(
    DCLSHAREDTESTINFO  *pSTI,
    const char         *pszPath,
    const char         *pszDeviceName)
{
    DCLSTATUS           dclStat = DCLSTAT_FEATUREDISABLED;

    DclAssertWritePtr(pSTI, sizeof(*pSTI));

    /*  If any of these features are being used, try to open the external
        requester interface to the FileSys and BlockDev, so long as they
        are not explicitly disabled.
    */
    if(pSTI->fPerfLog || pSTI->fProfile || pSTI->fTrace || pSTI->fStats)
    {
        if(!pSTI->fReqFSDisabled)
        {
            DclAssert(pszPath);

            dclStat = DclOsRequestorOpen(pszPath, DCLIODEVICE_FILESYSTEM, &pSTI->hReqFS);
            if(dclStat != DCLSTAT_SUCCESS)
                DclPrintf("Unable to open requestor interface for FileSys \"%s\", Status=%lX\n", pszPath, dclStat);
        }

        if(!pSTI->fReqDevDisabled)
        {
            DclAssert(pszDeviceName);

            /*  If a device name is available, try to open the block device
                using that name.  If not, then try to open the block device
                using the file system path (if available).  If the latter is
                successful, the block dev handle must be different than the
                file sys handle.
            */
            dclStat = DCLSTAT_FEATUREDISABLED;

            if(*pszDeviceName)
                dclStat = DclOsRequestorOpen(pszDeviceName, DCLIODEVICE_BLOCK, &pSTI->hReqBD);
            else if(pszPath)
                dclStat = DclOsRequestorOpen(pszPath, DCLIODEVICE_BLOCK, &pSTI->hReqBD);

            if(dclStat == DCLSTAT_SUCCESS)
            {
                /*  Reality check -- these handles should never be the same
                */
                DclAssert(pSTI->hReqBD != pSTI->hReqFS);
            }
            else
            {
                DclPrintf("Unable to open requestor interface for BlockDev \"%s\", Status=%lX\n", pszDeviceName, dclStat);
            }
        }
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestRequestorClose()

    Parameters:
        pSTI - A pointer to the DCLSHAREDTESTINFO structure to use.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclTestRequestorClose(
    DCLSHAREDTESTINFO  *pSTI)
{
    DCLSTATUS           dclStat = DCLSTAT_SUCCESS;

    DclAssertWritePtr(pSTI, sizeof(*pSTI));

    /*  ToDo: Should ideally do some better error checking here...
    */

    if(pSTI->hReqBD)
        dclStat = DclOsRequestorClose(pSTI->hReqBD);

    if(pSTI->hReqFS)
        dclStat = DclOsRequestorClose(pSTI->hReqFS);

    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestPressEnter()

    This function displays the message:

        "Press [Q] to Quit, or [Enter] to continue..."

    and waits until the user presses an appropriate key.

    If console input is not supported, the message will not be
    displayed and this function will return TRUE immediately.

    Parameters:
        hDclInst - The DCL instance handle

    Return Value:
        Returns TRUE if [Enter] was pressed, or if console input is
        not supported.  Returns FALSE if [Q] was pressed, or if the
        process is terminating.
-------------------------------------------------------------------*/
D_BOOL DclTestPressEnter(
    DCLINSTANCEHANDLE   hDclInst)
{
    /*  Note that the function intentionally includes logic to
        handle both compile-time and run-time detection of the
        ability to handle console-input.  This is due to some
        systems which generally support console input, but at
        certain times (or in certain execution contexts) it may
        be disabled.
    */

  #if DCL_OSFEATURE_CONSOLEINPUT
    DclPrintf("\nPress [Q] to Quit, or [Enter] to continue...");

    while(TRUE)
    {
        D_UCHAR     chr;
        DCLSTATUS   dclStat;

        dclStat = DclInputChar(hDclInst, &chr, 0);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            if(chr == '\n')
            {
                return TRUE;
            }
            else if(chr == 'q' || chr == 'Q')
            {
                /*  If it is 'q' followed by [enter] then process
                    that and return FALSE.  If it is any other
                    combination, resume processing at the outer
                    loop.
                */
                dclStat = DclInputChar(hDclInst, &chr, 0);
                if((dclStat == DCLSTAT_SUCCESS) && (chr == '\n'))
                    return FALSE;
            }
            else
            {
                continue;
            }
        }
        else if(dclStat == DCLSTAT_INPUT_TERMINATE)
        {
            /*  If there is a pending request to terminate the process,
                pretend a 'Q' was pressed.
            */
            return FALSE;
        }
        else
        {
            /*  If for whatever reason input is disabled, or some other
                error occurred, just pretend an [Enter] key was pressed,
                and continue on...
            */
            return TRUE;
        }
    }

  #else

    /*  If for whatever reason input is disabled, just pretend an
        [Enter] key was pressed, and continue on...
    */
    return TRUE;
  #endif
}


/*-------------------------------------------------------------------
    Private: DclTestProfilerStart()

    This function starts the profiler.  This includes any profiler
    which may be linked in with this test in a monolithic fashion,
    as well as any devices in the device hierarchy which accept the
    command.

    Parameters:
        pSTI - A pointer to the DCLSHAREDTESTINFO structure to use.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclTestProfilerStart(
    DCLSHAREDTESTINFO      *pSTI)
{
    DCLREQ_PROFILER_ENABLE  req;

    DclAssertWritePtr(pSTI, sizeof(*pSTI));
    DclAssert(pSTI->fProfile);

  #if DCLCONF_PROFILERENABLED
    /*  Send the command locally first.  If profiler stuff is
        linked directly into the executable, this will cause
        the profiler to be locally started.
    */
    DclProfEnable();
  #endif

    /*  If we have a device handle, send the profiler command
        down the chain, to both the file system and the block
        device if possible.
    */
    if(pSTI->hReqFS && pSTI->fProfileFS)
    {
        DclMemSet(&req, 0, sizeof(req));

        req.ior.ioFunc = DCLIOFUNC_PROFILER_ENABLE;
        req.ior.ulReqLen = sizeof(req);

        DclOsRequestorDispatch(pSTI->hReqFS, &req.ior);

        /*  Turn FileSystem profiling off if the feature is disabled.
        */
        if(req.dclStat == DCLSTAT_FEATUREDISABLED)
            pSTI->fProfileFS = FALSE;
    }

    if(pSTI->hReqBD && pSTI->fProfileBD)
    {
        DclMemSet(&req, 0, sizeof(req));

        req.ior.ioFunc = DCLIOFUNC_PROFILER_ENABLE;
        req.ior.ulReqLen = sizeof(req);

        DclOsRequestorDispatch(pSTI->hReqBD, &req.ior);

        /*  Turn BlockDev profiling off if the feature is disabled.
        */
        if(req.dclStat == DCLSTAT_FEATUREDISABLED)
            pSTI->fProfileBD = FALSE;
    }

    return;
}


/*-------------------------------------------------------------------
    Private: DclTestProfilerStop()

    This function stops the profiler and displays the results.
    This includes any profiler which may be linked in with this
    test in a monolithic fashion, as well as any devices in the
    device hierarchy which accept the command.

    Parameters:
        pSTI - A pointer to the DCLSHAREDTESTINFO structure to use.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclTestProfilerStop(
    DCLSHAREDTESTINFO      *pSTI)
{
    DCLREQ_PROFILER_SUMMARY reqSum;
    DCLREQ_PROFILER_DISABLE reqDisable;

    DclAssertWritePtr(pSTI, sizeof(*pSTI));
    DclAssert(pSTI->fProfile);

  #if DCLCONF_PROFILERENABLED
    /*  Send the command locally first.  If profiler stuff is
        linked directly into the executable, this will cause
        the profiler to be locally started.
    */
    DclProfSummary(TRUE, TRUE, TRUE);
    DclProfDisable();
  #endif

    /*  If we have a device handle, send the profiler command
        down the chain, to both the file system and the block
        device if possible.
    */
    if(pSTI->hReqFS && pSTI->fProfileFS)
    {
        DclMemSet(&reqSum, 0, sizeof(reqSum));

        reqSum.ior.ioFunc = DCLIOFUNC_PROFILER_SUMMARY;
        reqSum.ior.ulReqLen = sizeof(reqSum);
        reqSum.fReset = TRUE;
        reqSum.fShort = TRUE;

        DclOsRequestorDispatch(pSTI->hReqFS, &reqSum.ior);

        DclMemSet(&reqDisable, 0, sizeof(reqDisable));

        reqDisable.ior.ioFunc = DCLIOFUNC_PROFILER_DISABLE;
        reqDisable.ior.ulReqLen = sizeof(reqDisable);

        DclOsRequestorDispatch(pSTI->hReqFS, &reqDisable.ior);
    }

    if(pSTI->hReqBD && pSTI->fProfileBD)
    {
        DclMemSet(&reqSum, 0, sizeof(reqSum));

        reqSum.ior.ioFunc = DCLIOFUNC_PROFILER_SUMMARY;
        reqSum.ior.ulReqLen = sizeof(reqSum);
        reqSum.fReset = TRUE;
        reqSum.fShort = TRUE;

        DclOsRequestorDispatch(pSTI->hReqBD, &reqSum.ior);

        DclMemSet(&reqDisable, 0, sizeof(reqDisable));

        reqDisable.ior.ioFunc = DCLIOFUNC_PROFILER_DISABLE;
        reqDisable.ior.ulReqLen = sizeof(reqDisable);

        DclOsRequestorDispatch(pSTI->hReqBD, &reqDisable.ior);
    }

    return;
}


/*-------------------------------------------------------------------
    Private: DclTestTraceStart()

    This function starts the trace facility.  This includes any local
    tracing which may be linked in with this test in a monolithic
    fashion, as well as any devices in the device hierarchy which
    accept the command.

    Parameters:
        pSTI - A pointer to the DCLSHAREDTESTINFO structure to use.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclTestTraceStart(
    DCLSHAREDTESTINFO  *pSTI)
{
    DCLREQ_TRACEENABLE  req;

    DclAssertWritePtr(pSTI, sizeof(*pSTI));
    DclAssert(pSTI->fTrace);

    /*  Send the command locally first.
    */
    DCLTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, TRACEFORCEON), "Global Trace Started\n"));

    /*  If we have a device handle, send the trace command
        down the chain, to both the file system and the block
        device if possible.
    */
    if(pSTI->hReqFS)
    {
        DclMemSet(&req, 0, sizeof(req));

        req.ior.ioFunc = DCLIOFUNC_TRACE_ENABLE;
        req.ior.ulReqLen = sizeof(req);
        req.nLevel = 1;

        DclOsRequestorDispatch(pSTI->hReqFS, &req.ior);
    }

    if(pSTI->hReqBD)
    {
        DclMemSet(&req, 0, sizeof(req));

        req.ior.ioFunc = DCLIOFUNC_TRACE_ENABLE;
        req.ior.ulReqLen = sizeof(req);

        DclOsRequestorDispatch(pSTI->hReqBD, &req.ior);
    }

    return;
}


/*-------------------------------------------------------------------
    Private: DclTestTraceStop()

    This function stops the trace facility.  This includes any trace
    ability which may be linked in with this test in a monolithic
    fashion, as well as any devices in the device hierarchy which
    accept the command.

    Parameters:
        pSTI - A pointer to the DCLSHAREDTESTINFO structure to use.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclTestTraceStop(
    DCLSHAREDTESTINFO  *pSTI)
{
    DCLREQ_TRACEDISABLE req;

    DclAssertWritePtr(pSTI, sizeof(*pSTI));
    DclAssert(pSTI->fTrace);

    /*  Send the command locally first.
    */
    DCLTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, TRACEOFF), "Global Trace Stopped\n"));

    /*  If we have a device handle, send the trace command
        down the chain, to both the file system and the block
        device if possible.
    */
    if(pSTI->hReqFS)
    {
        DclMemSet(&req, 0, sizeof(req));

        req.ior.ioFunc = DCLIOFUNC_TRACE_DISABLE;
        req.ior.ulReqLen = sizeof(req);

        DclOsRequestorDispatch(pSTI->hReqFS, &req.ior);
    }

    if(pSTI->hReqBD)
    {
        DclMemSet(&req, 0, sizeof(req));

        req.ior.ioFunc = DCLIOFUNC_TRACE_DISABLE;
        req.ior.ulReqLen = sizeof(req);

        DclOsRequestorDispatch(pSTI->hReqBD, &req.ior);
    }

    return;
}


/*-------------------------------------------------------------------
    Private: DclTestInstrumentationStart()

    This function starts any tracing, profiling, and/or status which
    may be enabled.

    Parameters:
        pSTI - A pointer to the DCLSHAREDTESTINFO structure to use.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclTestInstrumentationStart(
    DCLSHAREDTESTINFO  *pSTI)
{
    DclAssertWritePtr(pSTI, sizeof(*pSTI));

    DclTestStatsReset(pSTI);

    if(pSTI->fTrace)
        DclTestTraceStart(pSTI);

    if(pSTI->fProfile)
        DclTestProfilerStart(pSTI);

    return;
}


/*-------------------------------------------------------------------
    Private: DclTestInstrumentationStop()

    This function stops any tracing, profiling, and/or status which
    may be enabled.

    Parameters:
        pSTI - A pointer to the DCLSHAREDTESTINFO structure to use.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclTestInstrumentationStop(
    DCLSHAREDTESTINFO  *pSTI,
    const char         *pszCategorySuffix)
{
    DclAssertWritePtr(pSTI, sizeof(*pSTI));

    if(pSTI->fProfile)
        DclTestProfilerStop(pSTI);

    if(pSTI->fTrace)
        DclTestTraceStop(pSTI);

    if(pszCategorySuffix)
        DclTestStatsDump(pSTI, pszCategorySuffix);

    return;
}


/*-------------------------------------------------------------------
    Private: DclTestStatsReset()

    This function uses the IOCTL interface (if available) to
    reset the values in the various statistics gathering
    interfaces.

    Parameters:
        pSTI - A pointer to the DCLSHAREDTESTINFO structure to use.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclTestStatsReset(
    DCLSHAREDTESTINFO  *pSTI)
{
    DclAssertWritePtr(pSTI, sizeof(*pSTI));

    if(pSTI->fStats)
    {
        DCLREQ_STATSRESET   req;

        if(pSTI->hReqFS)
        {
            DCLSTATUS   dclStat;

            if(pSTI->nVerbosity >= DCL_VERBOSE_LOUD)
                DclPrintf("Resetting file system statistics counters\n");

            DclMemSet(&req, 0, sizeof(req));

            req.ior.ioFunc = DCLIOFUNC_STATS_RESET;
            req.ior.ulReqLen = sizeof(req);

            dclStat = DclOsRequestorDispatch(pSTI->hReqFS, &req.ior);
            if( (dclStat != DCLSTAT_SUCCESS) &&
                (dclStat != DCLSTAT_REQUESTOR_BADDEVICE) &&
                (dclStat != DCLSTAT_REQUESTOR_INTERFACEFAILED) )
            {
                return dclStat;
            }
        }

        if(pSTI->hReqBD)
        {
            DclAssertReadPtr(pSTI->pszDeviceName, 0);

            if(pSTI->nVerbosity >= DCL_VERBOSE_LOUD)
                DclPrintf("Resetting block device statistics counters\n");

            DclMemSet(&req, 0, sizeof(req));

            req.ior.ioFunc = DCLIOFUNC_STATS_RESET;
            req.ior.ulReqLen = sizeof(req);
            DclStrNCpy(req.szDeviceName, pSTI->pszDeviceName, sizeof(req.szDeviceName));

            return DclOsRequestorDispatch(pSTI->hReqBD, &req.ior);
        }
    }

    return DCLSTAT_FAILURE;
}


/*-------------------------------------------------------------------
    Private: DclTestStatsDump()

    This function uses the IOCTL interface (if available) to
    dump the values from the various statistics gathering
    interfaces.

    Parameters:
        pSTI               - A pointer to the DCLSHAREDTESTINFO structure.
        pszCategorySuffix - A pointer to a suffix to use for the
                            PerfLog category field (may be NULL).

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclTestStatsDump(
    DCLSHAREDTESTINFO  *pSTI,
    const char         *pszCategorySuffix)
{
    DclAssertWritePtr(pSTI, sizeof(*pSTI));

    if(pSTI->fStats)
    {
        if(pSTI->hReqFS)
        {
            DCLSTATUS   dclStat;

            dclStat = DumpStatsForDevice(pSTI, pszCategorySuffix, pSTI->hReqFS);
            if( (dclStat != DCLSTAT_SUCCESS) &&
                (dclStat != DCLSTAT_REQUESTOR_BADDEVICE) &&
                (dclStat != DCLSTAT_REQUESTOR_INTERFACEFAILED) )
            {
                return dclStat;
            }
        }

        if(pSTI->hReqBD)
        {
            return DumpStatsForDevice(pSTI, pszCategorySuffix, pSTI->hReqBD);
        }
    }

    return DCLSTAT_FAILURE;
}


/*-------------------------------------------------------------------
    Local: DumpStatsForDevice()

    This function uses the IOCTL interface (if available) to dump
    the values from the various statistics gathering interfaces.

    Parameters:
        pSTI               - A pointer to the DCLSHAREDTESTINFO structure.
        pszCategorySuffix - A pointer to a suffix to use for the
                            PerfLog category field (may be NULL).
        hReq              - The requestor handle.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS DumpStatsForDevice(
    DCLSHAREDTESTINFO  *pSTI,
    const char         *pszCategorySuffix,
    DCLOSREQHANDLE      hReq)
{
    DCLREQ_STATS        req = {{0}};

    DclAssert(hReq);
    DclAssertWritePtr(pSTI, sizeof(*pSTI));
    DclAssertReadPtr(pSTI->pszDeviceName, 0);
    DclAssertWritePtr(pSTI->pStatsBuff, DCLOUTPUT_BUFFERLEN*2);

    req.ior.ioFunc      = DCLIOFUNC_STATS_REQUEST;
    req.ior.ulReqLen    = sizeof(req);
    req.nBufferLen      = DCLOUTPUT_BUFFERLEN;
    req.pDescBuffer     = (char*)pSTI->pStatsBuff;
    req.pDataBuffer     = (char*)pSTI->pStatsBuff + DCLOUTPUT_BUFFERLEN;
    req.pszCatSuffix    = pszCategorySuffix;
  #if DCLCONF_OUTPUT_ENABLED
    req.pszBldSuffix    = pSTI->szPerfLogSuffix;
  #endif
    req.nType           = 0;
    req.nSubType        = 0;
    DclStrNCpy(req.szDeviceName, pSTI->pszDeviceName, sizeof(req.szDeviceName));

    while(TRUE)
    {
        DCLSTATUS   dclStat;

        dclStat = DclOsRequestorDispatch(hReq, &req.ior);
        if( (dclStat == DCLSTAT_REQUESTOR_BADDEVICE) ||
            (dclStat == DCLSTAT_REQUESTOR_INTERFACEFAILED) )
        {
            /*  It is quite reasonable that the interface will not exist
                in some environments.  Even where the interface exists,
                there may not be any devices below us which recognize
                the request, so just politely do nothing.
            */
            if(pSTI->nVerbosity >= DCL_VERBOSE_LOUD)
            {
                DclPrintf("OS Requestor found nobody to service the stats request, Status=%lX\n", dclStat);
            }

            return dclStat;
        }
        else if(dclStat != DCLSTAT_SUCCESS)
        {
            /*  If the interface does exist, processing the request should
                not normally return an error.
            */
            DclPrintf("OS Requestor interface error %lX\n", dclStat);

            return dclStat;
        }

        /*  The request was processed in some fashion...
        */
        switch(req.ResultStatus)
        {
            case DCLSTAT_SUCCESS:
                /*  If it was successfully processed, write the results
                    to the log file.
                */
              #if DCLCONF_OUTPUT_ENABLED
                DclLogWriteString(pSTI->hDclInst, pSTI->hLog, req.pDescBuffer);
                DclLogWriteString(pSTI->hDclInst, pSTI->hLog, req.pDataBuffer);
              #endif

                /*  See if there is any (more) subtype data to return
                */
                req.nSubType++;
                continue;

            case DCLSTAT_SUBTYPECOMPLETE:
            case DCLSTAT_SUBTYPEUNUSED:
            case DCLSTAT_CATEGORYDISABLED:
            case DCLSTAT_UNSUPPORTEDREQUEST:
                /*  Try the next category
                */
                req.nType++;
                req.nSubType = 0;
                continue;

            case DCLSTAT_LIMITREACHED:
                /*  If we've reached the limit for the category type,
                    there is no point in continuing further, we are done.
                */
                return DCLSTAT_SUCCESS;

            default:
                /*  Unexpected status code
                */
                DclPrintf("Unhandled stats result! Status=%lX\n", req.ResultStatus);
                return dclStat;
        }
    }
}

