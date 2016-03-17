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

    This module implements the trace facility base functionality.  See
    dltrace.h for more information on using this feature.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltrace.c $
    Revision 1.20  2011/09/06 00:58:27Z  garyp
    Fixed to not try to indent a zero length.
    Revision 1.19  2009/09/30 21:47:20Z  garyp
    Updated GetThreadSlot() to handle the case where DclTimePassedUS()
    returns a microsecond count of zero.
    Revision 1.18  2009/06/28 00:17:08Z  garyp
    Updated so the indent level is tracked on a per-thread basis.  Add the
    BackTrace capability to aid in debugging multithreaded issues.
    Revision 1.17  2009/04/10 02:00:51Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.16  2008/05/03 02:28:47Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.15  2008/04/08 19:49:04Z  billr
    Include limits.h for INT_MIN.
    Revision 1.14  2008/03/19 20:34:41Z  Garyp
    Updated to allow multiple trace outputs to be appended to each other on
    the same line (assuming the low level output device supports that).
    Revision 1.13  2008/01/30 02:32:31Z  Garyp
    Updated so that when trace output is forced on, due to mismatched indent/
    undent directives, the forced output stays on for a set number of lines,
    rather than indefinitely.
    Revision 1.12  2007/12/18 21:03:48Z  brandont
    Updated function headers.
    Revision 1.11  2007/11/03 23:31:10Z  Garyp
    Added the standard module header.
    Revision 1.10  2007/10/20 22:46:04Z  Garyp
    Added DclTraceIndentGet/Set().
    Revision 1.9  2007/03/18 17:55:42Z  Garyp
    Added DclTraceMaskGet() and DclTraceMaskSwap().  Now store the trace mask
    in a static global so that it can be modified on the fly in a debugger
    environment.
    Revision 1.8  2007/03/06 03:46:50Z  Garyp
    Updated to use a renamed setting.
    Revision 1.7  2007/01/02 22:15:25Z  Garyp
    Documentation fixes.
    Revision 1.6  2006/10/03 21:13:57Z  Garyp
    Updated to allow output to be entirely disabled (and the code not pulled
    into the image).
    Revision 1.5  2006/08/18 19:33:38Z  Garyp
    Notational change for clarity -- nothing functional.
    Revision 1.4  2006/01/10 21:30:35Z  Garyp
    Documentation changes only.
    Revision 1.3  2006/01/04 02:07:15Z  Garyp
    Updated to use some renamed settings.
    Revision 1.2  2005/12/19 01:22:48Z  garyp
    Modified DclTraceToggle() to operate using the trace initial state, and
    to return the trace enable level.
    Revision 1.1  2005/10/28 04:32:52Z  Pauli
    Initial revision
    Revision 1.4  2005/10/28 05:32:52Z  Garyp
    Tweaked the error handling code.
    Revision 1.3  2005/10/20 07:49:47Z  garyp
    Modified so the initial trace state is specified at compile time in
    dclconf.h.  Added the "global" trace capability.
    Revision 1.2  2005/10/12 04:01:34Z  pauli
    Added TraceToggle.
    Revision 1.1  2005/10/05 22:08:14Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <stdarg.h>

#include <dcl.h>

#if D_DEBUG && DCLCONF_OUTPUT_ENABLED

#include <dlprintf.h>

#define INDENTCHARS     (2)
#define MAXINDENT       (50)
#define PREFIXLEN       (5)         /* Length of "REL: ", etc.  */
#define FORCEDCOUNT     (500)

typedef struct
{
    DCLTHREADID         tid;
    unsigned            nIndentLevel;
    DCLTIMESTAMP        ts;         /* last time used for LRU */
} THREADCONTEXT;

static THREADCONTEXT    aTC[DCL_MAX_THREADS+1];     /* one extra for good measure */
static int              iTraceEnable = DCLTRACE_AUTOENABLE;
static D_UINT32         ulDclTraceMask = DCLTRACEMASK;

/*  (The ulDclTraceMask is declared as such so that in a debugger
    environment, it may be interactively patched if need be.)
*/

#if DCLTRACE_BACKTRACEBUFFERSIZE
    typedef struct
    {
        const char         *pszFormat;
        DCLTHREADID         tid;
        DCLTIMESTAMP        ts;
    } BACKTRACEENTRY;

    static unsigned         nBackTraceBusy = 0;
    static unsigned         nBackTraceIndex = 0;
    static unsigned         nBackTraceRecordsLost = 0;
    static unsigned         nBackTraceDumpInProgress = 0;
    static BACKTRACEENTRY   aBT[DCLTRACE_BACKTRACEBUFFERSIZE];
    static D_UINT32         aulDiffUS[DCLTRACE_BACKTRACEBUFFERSIZE];
#endif

static unsigned GetThreadSlot(void);


/*-------------------------------------------------------------------
    Protected: DclDebugPrintf()

    Display debug output with a number of formatting options.  See
    dltrace.h for more information.

    This function provides a subset of the ANSI C printf()
    functionality with several extensions to support fixed
    size data types.

    See DclVSprintf() for the list of supported types.

    Parameters:
        pszFmt  - A pointer to the null-terminated format string
        ...     - The variable length argument list

    Return Value:
        Returns the length processed.
-------------------------------------------------------------------*/
int DclDebugPrintf(
    const char     *pszFmt,
    ...)
{
    va_list         arglist;
    int             nReturn;

    va_start(arglist, pszFmt);

    /*  We use a debug level of '0' here because the macro that
        invoked us has already qualified the debug level.  Using
        '0' ensures that the output will always be displayed.
    */
    nReturn = DclTraceVPrintf(MAKETRACEFLAGS(TRACEALWAYS, 0, 0),
        DCLTRACEBIT_ALL, pszFmt, arglist);

    va_end(arglist);

    return nReturn;
}


/*-------------------------------------------------------------------
    Protected: DclTracePrintf()

    Display debug output with a number of formatting options.  See
    dltrace.h for more information.

    This function provides a subset of the ANSI C printf()
    functionality with several extensions to support fixed
    size data types.

    See DclVSprintf() for the list of supported types.

    Parameters:
        ulFlags - The trace flags
        pszFmt  - A pointer to the null-terminated format string
        ...     - The variable length argument list

    Return Value:
        Returns the length processed.
-------------------------------------------------------------------*/
int DclTracePrintf(
    D_UINT32        ulFlags,
    const char     *pszFmt,
    ...)
{
    va_list         arglist;
    int             nReturn;

    va_start(arglist, pszFmt);

    nReturn = DclTraceVPrintf(ulFlags, ulDclTraceMask, pszFmt, arglist);

    va_end(arglist);

    return nReturn;
}


/*-------------------------------------------------------------------
    Protected: DclTraceIndentGet()

    Return the current DCL trace indent level.

    Typically the indent get/set functions are only used where
    code abnormally exits while inside a multi-level nested
    indent.

    Parameters:
        None.

    Return Value:
        Returns the current DCL trace indent level.
-------------------------------------------------------------------*/
unsigned DclTraceIndentGet(void)
{
    return aTC[GetThreadSlot()].nIndentLevel;
}


/*-------------------------------------------------------------------
    Protected: DclTraceIndentSet()

    Sets the current DCL trace indent level.

    Parameters:
        uIndent - The indent level to use, from 0 to n.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclTraceIndentSet(
    unsigned    nIndent)
{
    aTC[GetThreadSlot()].nIndentLevel = nIndent;

    return;
}


/*-------------------------------------------------------------------
    Protected: DclTraceMaskGet()

    Return the current DCL trace mask value.

    Parameters:
        None.

    Return Value:
        Returns the current DCL trace mask value.
-------------------------------------------------------------------*/
D_UINT32 DclTraceMaskGet(void)
{
    return ulDclTraceMask;
}


/*-------------------------------------------------------------------
    Protected: DclTraceMaskSwap()

    This function swaps the DCL trace mask value -- setting a
    new mask and returning the original.

    Parameters:
        The new DCL trace mask value.

    Return Value:
        Returns the old DCL trace mask value.
-------------------------------------------------------------------*/
D_UINT32 DclTraceMaskSwap(
    D_UINT32    ulNewMask)
{
    D_UINT32    ulOldMask = ulDclTraceMask;

    ulDclTraceMask = ulNewMask;

    return ulOldMask;
}


/*-------------------------------------------------------------------
    Protected: DclTraceVPrintf()

    This is the workhorse routine that is used by all the other
    debug output functions and macros.

    See DclVSprintf() for the list of supported types.

    Parameters:
        ulFlags     - The trace flags
        ulTraceMask - The trace mask for the product
        pszFmt      - A pointer to the null-terminated format string
        arglist     - The variable length argument list

    Return Value:
        Returns the length processed.
-------------------------------------------------------------------*/
int DclTraceVPrintf(
    D_UINT32        ulFlags,
    D_UINT32        ulTraceMask,
    const char     *pszFmt,
    va_list         arglist)
{
    static unsigned fAppendMode = FALSE;
    static unsigned nForcedOnCount = 0;
    static unsigned nForceLevel = 0;    /* Debug level to force */
    static int      iForceNest = 0;     /* "Enable" level at which "force" was turned on */
    int             iLen = 0;
    unsigned        nThreadSlot;
    D_UINT32        ulLocale = DCLTRACEGETLOCALE(ulFlags);
    D_UINT32        ulDebug  = DCLTRACEGETDEBUG(ulFlags);
    D_UINT32        ulFlg    = DCLTRACEGETFLAGS(ulFlags);
    D_UINT32        ulClass  = DCLTRACEGETCLASS(ulFlags);
    D_UINT32        ulLevel  = DCLTRACEGETLEVEL(ulFlags);

    nThreadSlot = GetThreadSlot();

    DclAssert(pszFmt);

  #if DCLTRACE_BACKTRACEBUFFERSIZE
    if(iTraceEnable > 0)
    {
        /*  The BackTrace feature logs every format string, thread ID,
            and timestamp in a circular buffer, which can be dumped with
            DclTraceDumpBackTrace().

            Use a simple control variable here -- no fancy mutex as it
            may affect scheduling.
        */
        nBackTraceBusy++;
        if(nBackTraceBusy == 1)
        {
            aBT[nBackTraceIndex].ts = aTC[nThreadSlot].ts;
            aBT[nBackTraceIndex].tid = aTC[nThreadSlot].tid;
            aBT[nBackTraceIndex].pszFormat = pszFmt;

            nBackTraceIndex++;
            if(nBackTraceIndex == DCLTRACE_BACKTRACEBUFFERSIZE)
                nBackTraceIndex = 0;
        }
        else
        {
            nBackTraceRecordsLost++;
        }
        nBackTraceBusy--;
    }
  #endif

    /*  If undenting, do so now, before displaying.  We do this now
        even if the debug level might not be a match.  This allows
        function prologue and epilogue trace macros to use mismatched
        debug levels and still function properly without corrupting
        the indentation.

        Also we must handle mismatched TRACE classes, for example where
        the function prologue might use a locale specific class and
        the function epilogue might use TRACEALWAYS.

        Special case in the event that the "TraceMask" is zero (meaning
        that no trace classes are enabled).  In this event we don't
        want to do any indent/undent so that regular non-trace (and
        TRACEALWAYS) output doesn't get indented for essentially
        invisible classes.  If any single (or more) trace classes are
        enabled, everything gets indented as specified.  (However even
        if TraceMask is zero, if "Force" is on, do the indentation.)
    */
    if((ulFlg & TRACEUNDENT) && (ulTraceMask || nForceLevel))
    {
        if(aTC[nThreadSlot].nIndentLevel)
        {
            aTC[nThreadSlot].nIndentLevel--;
        }
        else
        {
            DclPrintf("DCL: Trace Error: Mismatched TRACEINDENT/UNDENT\n");

            /* DclError(); */

            if(!nForcedOnCount)
            {
                DclPrintf("DCL: Trace Error: Forcibly turning on all trace output for %u lines\n", FORCEDCOUNT);

                nForcedOnCount = FORCEDCOUNT;
            }
        }
    }

    /*  If turning ON the trace, do it PRIOR to doing our output
    */
    if(ulFlg & TRACEON)
    {
        if(iTraceEnable != INT_MAX)
            iTraceEnable++;
    }
    else if(ulFlg & TRACEFORCEON)
    {
        if(iTraceEnable != INT_MAX)
            iTraceEnable++;

        if(!nForcedOnCount)
        {
            /*  Are we wanting to enforce a given debug level?

                If so, record the debug level, as well as the Enable nesting
                level so we can turn it off when the appropriate TRACEOFF
                comes around.
            */
            nForceLevel = (unsigned)ulLevel;
            if(iForceNest == 0)
                iForceNest = iTraceEnable;
        }
    }

    if(nForcedOnCount)
    {
        if(--nForcedOnCount == 0)
        {
            DclPrintf("DCL: Turning off forced trace output\n");
        }
        else
        {
            /*  Setting these two variables to 0 will force all trace
                information to be displayed.  This is done to help debug
                mismatched indent levels.
            */
            ulClass = 0;
            ulLevel = 0;
        }
    }
    else
    {
        if(nForceLevel)
        {
            /*  If a force level was specified, ignore class and pretend
                the the compiled debug level is whatever was specified.
            */
            ulClass = 0;
            ulDebug = nForceLevel;
        }
    }

    /*  Determine if the specified trace class bit (ulClass) is set
        in the mask bits for the product code base (D_PRODUCTLOCALE)
        being built.  (However if ulClass is zero (TRACEALWAYS), we
        always display the output, assuming the debug level is OK).
    */
    if(ulClass && (!(ulTraceMask & (1 << (ulClass-1)))))
        goto Cleanup;

    /*  If a class is specified and tracing is disabled, don't
        display anything.  (If a class is not specified then
        we are doing TRACEALWAYS, and this ignores the TRACEOFF
        setting.)
    */
    if(ulClass && iTraceEnable <= 0)
        goto Cleanup;

    /*  Display the output only if it's in the proper debug level range.

        The Debug Level at which the "calling" code was compiled, must
        be greater than or equal to the trace level information specified
        in the DCLTRACEPRINTF macro.
    */
    if(ulLevel <= ulDebug)
    {
        char    achBuffer[DCLOUTPUT_BUFFERLEN + (MAXINDENT*INDENTCHARS)+PREFIXLEN+1];

        if(!fAppendMode)
        {
            unsigned    nThisIndent = aTC[nThreadSlot].nIndentLevel;

            switch(ulLocale)
            {
                case PRODUCTNUM_RELIANCE:
                    DclStrCpy(achBuffer, RELPREFIX": ");
                    break;
                case PRODUCTNUM_FLASHFX:
                    DclStrCpy(achBuffer, FFXPREFIX": ");
                    break;
                case PRODUCTNUM_4GR:
                    DclStrCpy(achBuffer, DL4GRPREFIX": ");
                    break;
                case PRODUCTNUM_DCL:
                    DclStrCpy(achBuffer, DCLPREFIX": ");
                    break;
                default:
                    DclError();
                    DclStrCpy(achBuffer, "???: ");
                    break;
            }

            DclAssert(DclStrLen(achBuffer) == PREFIXLEN);

            if(nThisIndent > MAXINDENT)
                nThisIndent = MAXINDENT;

            if(nThisIndent)
                DclMemSet(&achBuffer[PREFIXLEN], ' ', nThisIndent * INDENTCHARS);

            iLen = PREFIXLEN + (nThisIndent * INDENTCHARS);
        }

        /*  Append the format string after the indentation.  We must
            concatenate all the data so that there is only one call
            to the resulting output function -- otherwise the stuff
            may end up on different lines, depending on the device.
        */
        DclStrCpy(&achBuffer[iLen], pszFmt);

        iLen = DclVPrintf(&achBuffer[0], arglist);

        /*  Recalculate the "AppendMode" state for the next call.  If the
            format string we just processed did not end with a '\n', then
            we switch into AppendMode, so the next string we process is
            put on the same line.

            Note that this feature should not be used in the generic
            portions of the product since it is OS port specific (some
            devices will put every string output on a different line
            regardless how they are terminated).  Therefore this should
            only be used in port specific code where it is known to be
            supported.
        */
        fAppendMode = pszFmt[DclStrLen(pszFmt)-1] == '\n' ? FALSE : TRUE;
    }

    /*  Display a "TRACE OFF" message if this is the command where
        it was turned off, and it is the outermost of any nested
        sequence of TRACEOFF commands.
    */
    if((ulFlg & TRACEOFF) && (iTraceEnable == 1))
        iLen += DclPrintf("TRACE OFF\n");

  Cleanup:
    /*  If indenting, we turn up the level AFTER doing our output
    */
    if(ulFlg & TRACEINDENT && (ulTraceMask || nForceLevel))
        aTC[nThreadSlot].nIndentLevel++;

    /*  If turning off the trace, do it AFTER doing our output
    */
    if(ulFlg & TRACEOFF)
    {
        /*  If Force is enabled and we received this TRACEOFF at the
            same nesting level at which forcing was turned on, turn
            Force off.
        */
        if(nForceLevel && iTraceEnable == iForceNest)
        {
            nForceLevel = 0;
            iForceNest = 0;
        }

        if(iTraceEnable != INT_MIN)
            iTraceEnable--;
    }

    return iLen;
}


/*-------------------------------------------------------------------
    Public: DclTraceToggle()

    Toggle DCL debug trace output on/off.

    Parameters:
        None.

    Return Value:
        Returns the new trace enable state.
-------------------------------------------------------------------*/
int DclTraceToggle(void)
{
    if(iTraceEnable > 0)
    {
        /*  Turn output off.
        */
        DCLTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, TRACEOFF),
                "Trace enable level decremented\n"));
    }
    else
    {
        /*  Turn output on.
        */
        DCLTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, TRACEON),
                "Trace enable level incremented\n"));
    }

    /*  Return the current setting
    */
    return iTraceEnable;
}


/*-------------------------------------------------------------------
    Protected: DclTraceDumpBackTrace()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
unsigned DclTraceDumpBackTrace(
    unsigned    nMaxRecords)
{
  #if DCLTRACE_BACKTRACEBUFFERSIZE
    unsigned    nIndex;
    unsigned    nLost;
    unsigned    nStart;
    unsigned    nStartIndex;
    unsigned    nRecs;
    unsigned    nRecsDumped = 0;

    /*  Use a simple control variables here -- no fancy mutex as it may
        affect scheduling.
    */

    /*  Increment the "busy" flag to prevent any new records from
        overwriting what we are trying to dump out.
    */
    nBackTraceBusy++;

    /*  Increment the "DumpBusy" flag to prevent another thread from
        trying to dump at the same time as us.
    */
    nBackTraceDumpInProgress++;
    if(nBackTraceDumpInProgress != 1)
    {
        nBackTraceBusy--;
        nBackTraceDumpInProgress--;
        return UINT_MAX;
    }

    nLost = nBackTraceRecordsLost;
    nStartIndex = nBackTraceIndex;

    /*  <Prior> to displaying anything, calculate the time difference for
        each record.  Otherwise the data will be skewed since the amount
        of time required to process DclPrintf() is unknown.  Will still
        be skewed a little bit, but not as bad...
    */
    nRecs = DCLMIN(nMaxRecords, DCLTRACE_BACKTRACEBUFFERSIZE);
    nIndex = nStartIndex;
    nStart = UINT_MAX;
    while(nRecs)
    {
        if(nIndex)
            nIndex--;
        else
            nIndex = DCLTRACE_BACKTRACEBUFFERSIZE-1;

        if(nStart == UINT_MAX)
            nStart = nIndex;
        else if(nStart == nIndex)
            break;

        aulDiffUS[nIndex] = DclTimePassedUS(aBT[nIndex].ts);

        nRecs--;
    }

    nRecs = DCLMIN(nMaxRecords, DCLTRACE_BACKTRACEBUFFERSIZE);
    nIndex = nStartIndex;
    nStart = UINT_MAX;

    DclPrintf("BACKTRACE dumping up to %u records in reverse order of creation\n", nRecs);
    DclPrintf("Record  DeltaUS   ThreadID         TraceFormatString\n");
    while(nRecs)
    {
        if(nIndex)
            nIndex--;
        else
            nIndex = DCLTRACE_BACKTRACEBUFFERSIZE-1;

        if(nStart == UINT_MAX)
            nStart = nIndex;
        else if(nStart == nIndex)
            break;

        if(aBT[nIndex].pszFormat)
        {
            nRecsDumped++;
            nRecs--;

            DclPrintf("%6d %9lD %lX: %s",
                -(int)nRecsDumped, -(D_INT32)aulDiffUS[nIndex], aBT[nIndex].tid, aBT[nIndex].pszFormat);
        }
    }

    nBackTraceBusy--;
    nBackTraceDumpInProgress--;

    DclPrintf("BACKTRACE dumped %u records, %u records were lost\n", nRecsDumped, nLost);

    return nRecsDumped;

  #else

    (void)nMaxRecords;

    DclPrintf("The BACKTRACE feature is disabled\n");

    return UINT_MAX;

  #endif
}


/*-------------------------------------------------------------------
    Local: GetThreadSlot()

    Return a thread slot number.  If the thread ID matches no
    existing thread ID, and there are no available slots, the
    least recently used slot will be reset to default values
    and reused.

    Parameters:
        None.

    Return Value:
        The thread slot number, relative to zero.
-------------------------------------------------------------------*/
static unsigned GetThreadSlot(void)
{
    unsigned        nn;
    unsigned        nSlot = UINT_MAX;
    DCLTHREADID     tid = DclOsThreadID();
    DCLTIMESTAMP    tsCurrent = DclTimeStamp();

    /*  There is no explicit initialization of the static globals, so
        we pretty much expect that an invalid thread ID is zero.
    */
    DclAssert(DCL_INVALID_THREADID == 0);

    for(nn=0; nn<DCLDIMENSIONOF(aTC); nn++)
    {
        /*  Found the thread, update the timestamp and return the slot number
        */
        if(aTC[nn].tid == tid)
        {
            aTC[nn].ts = tsCurrent;
            return nn;
        }

        /*  Remember the first available slot
        */
        if(aTC[nn].tid == DCL_INVALID_THREADID)
            nSlot = nn;
    }

    /*  If no free slot was found, find the LRU slot
    */
    if(nSlot == UINT_MAX)
    {
        D_UINT32 ulOldestUS = 0;

        for(nn=0; nn<DCLDIMENSIONOF(aTC); nn++)
        {
            D_UINT32 ulElapsedUS = DclTimePassedUS(aTC[nn].ts);

            if(ulElapsedUS >= ulOldestUS)
            {
                nSlot = nn;
                ulOldestUS = ulElapsedUS;
            }
        }

        /*  <Had> to have found something...
        */
        DclAssert(nSlot != UINT_MAX);

        DclMemSet(&aTC[nSlot], 0, sizeof(aTC[0]));
    }

    aTC[nSlot].tid = tid;
    aTC[nSlot].ts = tsCurrent;

    return nSlot;
}




#endif  /* D_DEBUG && DCLCONF_OUTPUT_ENABLED */


