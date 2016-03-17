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

    This module contains Device Driver Framework functions that are used
    to manage background compaction.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvcomp.c $
    Revision 1.28  2010/11/09 13:45:03Z  garyp
    Added FfxDriverDiskCompactAggressive().  Renamed some functions
    for interface consistency.
    Revision 1.27  2010/07/21 14:24:25Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.26  2010/07/13 00:08:00Z  garyp
    Updated so release level compaction diagnostics messages are 
    displayed only if FFXCONF_STATS_COMPACTION is greater than TRUE.
    Revision 1.25  2009/08/10 16:45:17Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.24  2009/07/21 20:52:28Z  garyp
    Merged from the v4.0 branch.  Updated the compaction functions to take
    a compaction level.  Renamed some functions to match current standards.
    Modified to use a longer sleep period after having completed a sequence
    of compactions.
    Revision 1.23  2009/04/09 21:26:38Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.22  2009/04/07 19:53:41Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.21  2009/03/31 18:08:22Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.20  2008/06/02 16:44:44Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.19  2008/05/16 00:17:00Z  garyp
    Fixed a bug in the stats interface which could cause a fault.  Enhanced the
    stats to display the amount of time required for each batch of compactions.
    Revision 1.18  2008/05/08 01:42:20Z  garyp
    Improved error handling in FfxDriverDiskCompStatsQuery().
    Revision 1.17  2008/03/13 02:55:31Z  Garyp
    Made the code conditional on FFXCONF_VBFSUPPORT.  Minor data type changes.
    Revision 1.16  2008/01/13 07:26:19Z  keithg
    Function header updates to support autodoc.
    Revision 1.15  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.14  2007/06/20 01:34:22Z  Garyp
    Factored out the stats reset logic so that it can be independently invoked.
    Revision 1.13  2007/02/10 22:26:19Z  Garyp
    Added in commented out code for compaction suspend/resume when using the
    "idletime" compaction model.  Modified the compaction thread to only
    display "compaction successful" messages after a sequence of compactions.
    Revision 1.12  2006/11/10 03:25:46Z  Garyp
    Standardized on a naming convention for the various "stats" interfaces.
    Modified the stats query to use the generic ParameterGet() ability now
    implemented through most all layers of the code.
    Revision 1.11  2006/10/04 02:29:18Z  Garyp
    Updated to use DclMemAllocZero().  Updated to use the new printf macros.
    Revision 1.10  2006/05/08 01:35:56Z  Garyp
    Finalized the statistics interfaces.
    Revision 1.9  2006/03/06 17:19:14Z  Garyp
    Updated to build cleanly in BACKGROUNDIDLE mode.
    Revision 1.8  2006/02/14 02:55:25Z  Garyp
    Corrected to use DiskNum instead of DeviceNum.
    Revision 1.7  2006/02/13 00:24:58Z  Garyp
    Commented out an assert.
    Revision 1.6  2006/02/11 23:39:50Z  Garyp
    Minor structure changes.
    Revision 1.5  2006/02/06 21:33:56Z  Garyp
    Updated to use new device/disk handle model.
    Revision 1.4  2005/12/30 17:52:21Z  Garyp
    Updated to use renamed thread related settings and functions, which are
    now a part of DCL.
    Revision 1.3  2005/12/12 22:56:55Z  garyp
    Added compaction tracking logic.
    Revision 1.2  2005/12/09 20:03:32Z  Garyp
    Conditioned the include of FfxDriverCompatifIdle().
    Revision 1.1  2005/12/04 21:07:10Z  Pauli
    Initial revision
    Revision 1.2  2005/12/04 21:07:10Z  Garyp
    Modified the compaction model to be specified as a tri-state value, which is
    one of the following FFX_COMPACT_SYNCHRONOUS, FFX_COMPACT_BACKGROUNDIDLE, or
    FFX_COMPACT_BACKGROUNDTHREAD.
    Revision 1.1  2005/10/22 07:51:24Z  garyp
    Initial revision
    Revision 1.3  2005/09/20 19:01:10Z  pauli
    Replaced _syssleep with DclOsSleep.
    Revision 1.2  2005/09/18 08:04:09Z  garyp
    Added FfxDriverCompactifIdle().
    Revision 1.1  2005/08/03 19:30:46Z  pauli
    Initial revision
    Revision 1.22  2005/04/13 00:34:40Z  garyp
    Minor code cleanup.  No functional changes.
    Revision 1.21  2005/02/18 01:26:07Z  GaryP
    Minor debug code change.
    Revision 1.20  2004/12/30 17:32:42Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.19  2004/12/10 18:36:44Z  GaryP
    Removed an invalid assert.
    Revision 1.18  2004/12/01 18:59:19Z  PaulI
    Set pDsk->pFCI before calling DclOsThreadCreate to prevent problems
    where the thread begins operating immediately and uses this pointer.
    Revision 1.17  2004/11/20 04:42:52Z  GaryP
    Modified to accomodate nested compaction suspends.
    Revision 1.16  2004/11/12 01:33:48Z  GaryP
    Fixed several issue per code review.
    Revision 1.15  2004/11/11 00:51:52Z  GaryP
    Modified the background compaction API to include suspend and resume
    capabilities, primarily to better accomodate OS's that don't have robust
    protection against priority inversion problems.
    Revision 1.14  2004/10/28 21:54:18Z  GaryP
    Debugging messages updated.  Eliminated garbage collection terminology.
    Revision 1.13  2004/10/03 04:46:42Z  GaryP
    Minor debug code change.
    Revision 1.12  2004/09/28 20:26:21  billr
    Replace FfxGetMillisecondCount() with FfxTimestamp() and FfxTimePassed().
    Revision 1.11  2004/09/27 19:33:17Z  jaredw
    Fixed use of szThreadName variable to only be used when COMPACTSTATS
    is true.
    Revision 1.10  2004/09/27 19:24:24Z  jaredw
    Changed return of thread wait to be stored in ffxstat and check it
    accordingly.
    Revision 1.9  2004/09/27 18:45:05Z  GaryP
    Added parameter checking, and cleaned up the debugging code.
    Revision 1.8  2004/09/24 22:58:08Z  garys
    initialize variable to placate compiler
    Revision 1.7  2004/09/23 08:24:07Z  GaryP
    Semantic change from "garbage collection" to "compaction".
    Revision 1.6  2004/09/23 03:36:54Z  GaryP
    Changed from the term "garbage collection" to "compaction".  Modified so
    the compaction characteristics are customizable.
    Revision 1.5  2004/09/17 04:16:48Z  GaryP
    Modified to only attempt background compaction after a cerain period of
    inactivity by FlashFX.  Modified the compaction thread to always sleep at
    least once during each iteraction to ensure that we behave nicely in a
    non-time sliced multithreaded system.
    Revision 1.4  2004/09/15 23:39:17Z  garys
    fAggressive parameter to FfxVbfCompact() to support background G.C.
    Revision 1.3  2004/09/15 20:04:43  jaredw
    Return from _sysThreadWait is now saved.
    Revision 1.2  2004/09/09 22:14:20Z  jaredw
    Added a new COMPACTSTATS print to show when the background thread
    had completed a successful garbage collection.
    Revision 1.1  2004/08/25 17:08:14Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_ALLOCATORSUPPORT
#if FFXCONF_VBFSUPPORT

#include <fxdriver.h>
#include <vbf.h>

#if FFXCONF_STATS_COMPACTION
#include <fxstats.h>
#endif

#if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD

/*  Milliseconds to sleep when we are actively compacting during idle time.

    Note that on some systems it may be legal (and more optimal) to sleep
    for 0 ticks, releasing the remainder of the timeslice.  Sleep(0) is not
    valid in all operating systems.
*/
#define ACTIVE_COMPACTION_SLEEP_INTERVAL  1

struct tagFFXCOMPACTINFO
{
    DCLTHREADHANDLE     hThread;
    volatile VBFHANDLE  hVBF;
    unsigned            fSuspended : 1;
    volatile D_BOOL     fWaitingForCompletion;
    volatile D_BOOL     fCompleted;
    PDCLMUTEX           hMutex;
    D_UINT32            ulSleepMS;
    D_UINT32            ulLongSleepMS;          /* ms to sleep if nothing to do */
    D_UINT32            ulCompactionsCompleted;
  #if FFXCONF_STATS_COMPACTION
    D_UINT16            uThreadPriority;
    char                szThreadName[DCL_THREADNAMELEN];
    D_UINT32            ulCompactionsInitiated;
    D_UINT32            ulTotalMS;              /* Total time spent compacting            */
    D_UINT32            ulSlowestCompactMS;     /* Slowest individual compaction          */
  #endif
};


static void *  CompactionThread(void * pThreadArg);

#endif


/*-------------------------------------------------------------------
    ToDo:
     - Finish implementing the "Suspend" and "Resume" functions when
       using the "BackgroundIdle" compaction model.
     - Consider whether the standard "SleepMS" configuration
       parameter should be eliminated.  See the comments regarding
       that at the "LongSleepMS" variable.
-------------------------------------------------------------------*/


#if FFXCONF_STATS_COMPACTION

/*-------------------------------------------------------------------
    Protected: FfxDriverDiskCompactStatsQuery()

    Return compaction statistics if FFXCONF_STATS_COMPACTION
    is enabled.

    Parameters:
        pDsk   - A pointer to the FFXDSKINFO structure to use
        pStat  - A pointer to the FFXCOMPSTATS structure to fill
        fReset - Reset the reset-able counts to zero

    Return Value:
        Returns TRUE if successful or FALSE otherwise.
-------------------------------------------------------------------*/
D_BOOL FfxDriverDiskCompactStatsQuery(
    FFXDISKINFO    *pDisk,
    FFXCOMPSTATS   *pStat,
    D_BOOL          fReset)
{
    DclAssert(pDisk);
    DclAssert(pStat);

  #if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD
    if(pDisk->pFCI)
    {
        DclStrNCpy(pStat->szThreadName, pDisk->pFCI->szThreadName, sizeof pStat->szThreadName);
        pStat->uThreadPriority        = pDisk->pFCI->uThreadPriority;
        pStat->ulCompactionsInitiated = pDisk->pFCI->ulCompactionsInitiated;
        pStat->ulTotalMS              = pDisk->pFCI->ulTotalMS;
        pStat->ulSlowestCompactMS     = pDisk->pFCI->ulSlowestCompactMS;
        pStat->ulSleepMS              = pDisk->pFCI->ulLongSleepMS;

        if(fReset)
            FfxDriverDiskCompactStatsReset(pDisk);

        return TRUE;
    }
    else
    {
        return FALSE;
    }

  #elif FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDIDLE

    return FALSE;

  #elif FFX_COMPACTIONMODEL == FFX_COMPACT_SYNCHRONOUS

    return FALSE;

  #else

    #error "FFX: Invalid compaction model"

  #endif
}


/*-------------------------------------------------------------------
    Protected: FfxDriverDiskCompactStatsReset()

    Return compaction statistics if FFXCONF_STATS_COMPACTION
    is enabled.

    Parameters:
        pDsk    - A pointer to the FFXDSKINFO structure to use

    Return Value:
        Returns TRUE if successful or FALSE otherwise.
-------------------------------------------------------------------*/
D_BOOL FfxDriverDiskCompactStatsReset(
    FFXDISKINFO    *pDisk)
{
    DclAssert(pDisk);

  #if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD
    if(pDisk->pFCI)
    {
    pDisk->pFCI->ulCompactionsInitiated = 0;
    pDisk->pFCI->ulTotalMS = 0;
    pDisk->pFCI->ulSlowestCompactMS = 0;

    return TRUE;
    }
  #endif

    return FALSE;
}

#endif  /* FFXCONF_STATS_COMPACTION */


/*-------------------------------------------------------------------
    Public: FfxDriverDiskCompactAggressive()

    Aggressively compact the Disk until nothing more can be recovered. 

    Use this function with caution, as it will aggressively compact
    the flash, and may result in shortened flash life.

    Parameters:
        hVBF    - The handle denoting the VBF instance to use.

    Return Value:
        Returns the number of pages which were compacted.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxDriverDiskCompactAggressive(
    FFXDISKHANDLE   hDisk)
{
    unsigned        nNumCompactions = 0;
    FFXIOSTATUS     ioTotal = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);

    DclAssert(hDisk);
    DclAssert(*hDisk);

    DclPrintf("Aggressively compacting -- one moment please...\n");

    while(TRUE)
    {
        FFXIOSTATUS ioStat;

        ioStat = FfxVbfCompact((*hDisk)->hVBF, FFX_COMPACTFLAGS_EVERYTHING);

        ioTotal.ffxStat = ioStat.ffxStat;
        ioTotal.ulCount += ioStat.ulCount;
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        FFXPRINTF(1, ("Compacting a unit freed %lU pages\n", ioStat.ulCount));

        /*  Count the number of compactions
        */
        nNumCompactions++;
    }

    DclPrintf("\nCompacted %u units recovering %lU pages.\n", nNumCompactions, ioTotal.ulCount);

    return ioTotal;
}


#if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE

/*-------------------------------------------------------------------
    Public: FfxDriverDiskCompactIfIdle()

    Compact the flash according to the specified criteria if the
    flash manager has been idle.

    During normal write operations, synchronous compactions will
    occur frequently and are sometimes long processes.  This function
    forces a compaction to occur immediately.  It can provide an
    apparent performance boost by creating more space that is
    writeable, and therefore reduce the frequency of synchronous
    compactions.  Higher level modules may call this interface to
    perform cleanup during idle time.

    Note that this function exists independently from the "Background
    Compaction" feature, which requires thread support.  it may be
    called from OS specific code where idle time execution paths may
    be modified to allow FlashFX to do compaction.

    The level specified by ulFlags must be one of the following
    values:
      FFX_COMPACTFLAGS_MILD       - Only Erase Units which are highly
                                    qualified (almost completely
                                    dirty) will be compacted.
      FFX_COMPACTFLAGS_GOOD       - Only Erase Units which are very
                                    well qualified (or better) will
                                    be compacted.
      FFX_COMPACTFLAGS_NORMAL     - Any Erase Unit which meets the
                                    background compaction threshold
                                    may be compacted.
      FFX_COMPACTFLAGS_EVERYTHING - Any Erase Unit which is dirty at
                                    all may be compacted.

    Regardless of the level specified, the dirtiest Erase Unit
    available will be compacted.  Regardless how dirty the flash
    may be, this function never compacts more than one Erase Unit
    in a given call.

    Use FFX_COMPACTFLAGS_EVERYTHING with caution, as it will
    aggressively compact the flash, and may result in shortened
    flash life.

    If the return value is anything other than 0 or 1, there is
    potentially information to compact, however not enough idle time
    has elapsed since the last I/O operation.  The value indicates
    the minimum number of milliseconds that the system <should> wait,
    before attempting another compaction.  This is based on the
    longest read/write idle time.  Trying to compact again at an
    earlier time will have no ill-effect, but will not do anything.

    Parameters:
        hVBF    - The handle denoting the VBF instance to use
        ulFlags - This is the level of compaction which is
                  desired, as described above.

    Return Value:
        0 - A compaction was attempted, but there is nothing to do
        1 - A compaction was successfully performed
        n - No attempt made, should wait at least n milliseconds
-------------------------------------------------------------------*/
D_UINT32 FfxDriverDiskCompactIfIdle(
    FFXDISKHANDLE   hDisk,
    D_UINT32        ulFlags)
{
    DclAssert(hDisk);
    DclAssert(*hDisk);
    DclAssert(!(ulFlags & FFX_COMPACTFLAGS_RESERVEDMASK));

    return FfxVbfCompactIfIdle((*hDisk)->hVBF, ulFlags);
}




#if 0
/*-------------------------------------------------------------------
    FfxDriverDiskCompactSuspend()

    This function suspends idle-time compaction.

    Note: It is acceptable to call this function even when
          idle-time compaction is already suspended.

    Parameters:
        pDsk - A pointer to the FFXDSKINFO structure to use

    Return Value:
        Returns one of the following FFXSTATUS codes:
          FFXSTAT_SUCCESS
          FFXSTAT_DRVCOMPACTNOTENABLED
          FFXSTAT_DRVCOMPACTALREADYSUSPENDED
          FFXSTAT_DRVCOMPACTSUSPENDFAILED
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskCompactSuspend(
    FFXDISKHANDLE   hDisk)
{
    D_UINT32        ulCount;

    DclAssert(hDisk);

    /*  Tell VBF to quit servicing idle-time compaction requests
    */
    ulCount = FfxVbfCompactIdleSuspend((*hDisk)->hVBF);

  #if FFXCONF_STATS_COMPACTION || D_DEBUG > 0
    DclPrintf("FFX: Idle-time compaction suspended, Count=%lU\n", ulCount);
  #else
    (void)ulCount;
  #endif

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    FfxDriverDiskCompactResume()

    This function resumes the background compaction thread.

    Note: It is acceptable to call this function even when
          background compaction is already running.

    Parameters:
        pDsk - A pointer to the FFXDSKINFO structure to use

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskCompactResume(
    FFXDISKHANDLE   hDisk)
{
    DclAssert(hDisk);

    /*  Tell VBF to resume servicing idle-time compaction requests.
        Since this outer layer expects to only have to call resume
        one time, call VBF as many times as is necessary to reduce
        the suspend count to zero.
    */
    while(TRUE)
    {
        D_UINT32 ulCount;

        ulCount = FfxVbfCompactIdleResume(pDsk->hVBF);
        if(ulCount && ulCount != D_UINT32_MAX)
            continue;

        break;
    }

  #if FFXCONF_STATS_COMPACTION || D_DEBUG > 0
    DclPrintf("FFX: Idle-time compaction resumed\n" );
  #endif

    return FFXSTAT_SUCCESS;
}
#endif

#endif  /*  FFX_COMPACTIONMODEL != FFX_COMPACT_NONE */


#if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD

/*-------------------------------------------------------------------
    Protected: FfxDriverDiskCompactCreate()

    Initiate a background compaction thread.

    Parameters:
        pDsk - A pointer to the FFXDSKINFO structure to use

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskCompactCreate(
    FFXDISKHANDLE   hDisk)
{
    FFXDISKINFO    *pDsk = *hDisk;
    FFXCOMPACTINFO *pFCI;
    DCLTHREADATTR  *pAttr = 0;
    char            szMutexName[DCL_MUTEXNAMELEN];
    char            szThreadName[DCL_THREADNAMELEN];
    D_UINT16        uPriority;
    D_UINT32        ulWriteIdleMS;
    FFXSTATUS       ffxStat = FFXSTAT_PROJUNSUPPORTEDOPTION;

    /*  Handle the case where we are reinitializing after a suspend.
        This could happen if a prior call to FfxDriverDiskCompactDestroy()
        failed for some reason, but was able to suspend compaction.
    */
    if(pDsk->pFCI)
    {
        /*  Reality check...
        */
        DclAssert(pDsk->pFCI->fSuspended == TRUE);
        DclAssert(pDsk->pFCI->fWaitingForCompletion == FALSE);
        DclAssert(pDsk->pFCI->fCompleted == FALSE);

        DclPrintf("FFX: Background compaction resume %X\n", pDsk->pFCI);
        return FfxDriverDiskCompactResume(pDsk);
    }

    pFCI = DclMemAllocZero(sizeof *pFCI);
    if(!pFCI)
        return DCLSTAT_MEMALLOCFAILED;

    pFCI->fSuspended = TRUE;

    if(!FfxHookOptionGet(FFXOPT_COMPACTIONPRIORITY, hDisk, &uPriority, sizeof uPriority))
    {
        DclError();
        goto ErrorCleanup;
    }

    if(!FfxHookOptionGet(FFXOPT_COMPACTIONSLEEPMS, hDisk, &pFCI->ulSleepMS, sizeof pFCI->ulSleepMS))
    {
        DclError();
        goto ErrorCleanup;
    }

    if(!FfxHookOptionGet(FFXOPT_COMPACTIONWRITEIDLEMS, hDisk, &ulWriteIdleMS, sizeof ulWriteIdleMS))
    {
        DclError();
        goto ErrorCleanup;
    }

    /*  Calculate the longer of the two sleep periods.  See the comment
        where ulLongSleepMS is used for more details.  We may want to
        consider simply removing the ulSleepMS configuration parameter
        entirely... maybe.
    */
    pFCI->ulLongSleepMS = DCLMAX(pFCI->ulSleepMS, ulWriteIdleMS);

    DclAssert(sizeof szMutexName >= 8);
    DclSNPrintf(szMutexName, sizeof(szMutexName), "FMCMP%02x", pDsk->Conf.nDiskNum);

    pFCI->hMutex = DclMutexCreate(szMutexName);
    if(!pFCI->hMutex)
    {
        ffxStat = DCLSTAT_MUTEXCREATEFAILED;
        goto ErrorCleanup;
    }

    pAttr = DclOsThreadAttrCreate();
    if(!pAttr)
    {
        ffxStat = DCLSTAT_THREADATTRCREATEFAILED;
        goto ErrorCleanup;
    }

    DclOsThreadAttrSetPriority(pAttr, uPriority);

    DclAssert(sizeof szThreadName >= 8);
    DclSNPrintf(szThreadName, sizeof(szThreadName), "FTCMP%02x", pDsk->Conf.nDiskNum);

  #if FFXCONF_STATS_COMPACTION
    DclAssert(sizeof pFCI->szThreadName >= sizeof szThreadName);
    DclStrCpy(pFCI->szThreadName, szThreadName);
    pFCI->uThreadPriority = uPriority;
  #endif

    pDsk->pFCI = pFCI;
    ffxStat = DclOsThreadCreate(&pFCI->hThread, szThreadName, pAttr, CompactionThread, pDsk);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        /*  Don't need the thread attributes any more
        */
        DclOsThreadAttrDestroy(pAttr);

     #if 1 //FFXCONF_STATS_COMPACTION > TRUE || D_DEBUG > 0
        DclPrintf("FFX: Background compaction thread %X, '%s' started\n", pDsk->pFCI, szThreadName);
     #endif

        /*  The thread is initially suspended, enable it now...
        */
        FfxDriverDiskCompactResume(pDsk);

        return ffxStat;
    }
    pDsk->pFCI = NULL;

  ErrorCleanup:
    if(pAttr)
        DclOsThreadAttrDestroy(pAttr);

    if(pFCI->hMutex)
        DclMutexDestroy(pFCI->hMutex);

    DclMemFree(pFCI);

    FFXPRINTF(1, ("Background compaction thread failed to start, Status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Protected: FfxDriverDiskCompactDestroy()

    Close down a background compaction thread.  This is a two stage
    operation, the first and most critical of which is to get the 
    compaction thread into suspended mode, so that no further 
    FfxVbfCompact() calls are made.

    The second stage is to get the thread to actually terminate,
    either by request, or by force if necessary, after which the
    allocated resources will be freed.

    If the thread cannot be terminated, it will be left in suspended
    mode.  Typically the system is about to be powered off, however,
    subsequent calls to FFxDriverCompactionCreate() will simply cause
    the suspended state to be removed so the thread is active once
    again.

    Parameters:
        pDsk      - A pointer to the FFXDSKINFO structure to use
        ulTimeout - The number of milliseconds to wait

    Return Value:
        Returns FFXSTAT_SUCCESS if we were able to successfully
        deinitialize the background compaction process.  Returns
        FFXSTAT_DRVCOMPACTSUSPENDED if the thread was successfully
        suspended but the thread and resources could not be completely
        freed.  Any other FFXSTAT_ code indicates a serious error.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskCompactDestroy(
    FFXDISKINFO    *pDsk,
    D_UINT32        ulTimeout)
{
    FFXSTATUS       ffxStat;
    FFXCOMPACTINFO *pFCI;

    DclAssert(pDsk);
    DclAssert(pDsk->pFCI);
    DclAssert(pDsk->pFCI->fWaitingForCompletion == FALSE);

    pFCI = pDsk->pFCI;

    pFCI->fWaitingForCompletion = TRUE;

    ffxStat = FfxDriverDiskCompactSuspend(pDsk);
    if(ffxStat != FFXSTAT_SUCCESS && ffxStat != FFXSTAT_DRVCOMPACTALREADYSUSPENDED)
    {
        DclError();
        return ffxStat;
    }

    if(!pFCI->fCompleted)
    {
      #if FFXCONF_STATS_COMPACTION > TRUE || D_DEBUG > 0
        DclPrintf("FFX: Instructing compaction thread to terminate\n");
      #endif
 
        ffxStat = DclOsThreadWait(pFCI->hThread, ulTimeout);
        if(ffxStat != FFXSTAT_SUCCESS)
        {
          #if FFXCONF_STATS_COMPACTION > TRUE || D_DEBUG > 0
            DclPrintf("FFX: Compaction thread wait failed, forcibly terminating\n");
          #endif

/*            DclError(); */

            ffxStat = DclOsThreadTerminate(pFCI->hThread);
            if(ffxStat != FFXSTAT_SUCCESS)
            {
                /*  We're in a situation where for whatever reason we cannot
                    terminate the compaction thread.  Most likely it is due to
                    the scheduler being too busy to schedule the compaction
                    thread so it terminates normally, and the OS not supporting
                    forcible thread termination.

                    In this case, turn <OFF> the "fWaitingForCompletion" flag
                    and just leave the thread in suspended mode.  Chances are
                    good that the system is about to be powered down anyway...
                */
                pFCI->fWaitingForCompletion = FALSE;

                /*  Check fCompleted one last time just in case the thread
                    really did complete before we had a chance to change
                    back to the fSuspended state.

                    If we are able to change back to suspended mode, we return
                    a status code indicating as such.
                */
                if(!pFCI->fCompleted)
                {
                  #if FFXCONF_STATS_COMPACTION > TRUE || D_DEBUG > 0
                    DclPrintf("FFX: Compaction thread termination failed, reverting to suspended mode\n");
                  #endif

                    return FFXSTAT_DRVCOMPACTSUSPENDED;
                }

                /*  If we get to here, somehow the thread finally did complete
                    at the last possible moment, so fall out and release our
                    resources as we would prefer.
                */
            }
        }
    }

    ffxStat = DclOsThreadDestroy(pFCI->hThread);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

  #if FFXCONF_STATS_COMPACTION > TRUE
    DclPrintf("FFX: Compaction thread '%s' stats: Called=%lU Completed=%lU Time=%lUms Longest=%lUms\n",
         pFCI->szThreadName, pFCI->ulCompactionsInitiated,
         pFCI->ulCompactionsCompleted, pFCI->ulTotalMS,
         pFCI->ulSlowestCompactMS);
  #endif

    DclMutexDestroy(pFCI->hMutex);

    DclMemFree(pFCI);

    pDsk->pFCI = NULL;

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDiskCompactSuspend()

    Wait until the current compaction operation (if any) is completed,
    and suspend any new background compactions.

    *Note* -- It is acceptable to call this function even when
              background compaction is already suspended.

    Parameters:
        pDsk - A pointer to the FFXDSKINFO structure to use

    Return Value:
        Returns one of the following FFXSTATUS codes:
          - FFXSTAT_SUCCESS
          - FFXSTAT_DRVCOMPACTNOTENABLED
          - FFXSTAT_DRVCOMPACTALREADYSUSPENDED
          - FFXSTAT_DRVCOMPACTSUSPENDFAILED
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskCompactSuspend(
    FFXDISKINFO    *pDsk)
{
	DclPrintf("FFX: FfxDriverDiskCompactSuspend start %X\n", pDsk);
    DclAssert(pDsk);

	DclPrintf("FFX: FfxDriverDiskCompactSuspend %X\n", pDsk->pFCI);
    if(!pDsk->pFCI)
	{
        DclPrintf("FFX: FfxDriverDiskCompactSuspend compactor is desabled\n");
        return FFXSTAT_DRVCOMPACTNOTENABLED;
	}

	DclPrintf("FFX: FfxDriverDiskCompactSuspend Asquire mutex\n");
    //if(1)
    if(DclMutexAcquire(pDsk->pFCI->hMutex))
    {
		DclPrintf("FFX: FfxDriverDiskCompactSuspend: is disk suspended?\n");
        if(pDsk->pFCI->fSuspended)
        {
			DclPrintf("FFX: FfxDriverDiskCompactSuspend disk is suspended\n");
            DclMutexRelease(pDsk->pFCI->hMutex);
            return FFXSTAT_DRVCOMPACTALREADYSUSPENDED;
        }

		DclPrintf("FFX: FfxDriverDiskCompactSuspend mark disk as suspended\n");
        pDsk->pFCI->fSuspended = TRUE;

      #if 1 //FFXCONF_STATS_COMPACTION > TRUE || D_DEBUG > 0
        DclPrintf("FFX: Background compaction thread suspended\n");
      #endif

      #if FFXCONF_STATS_COMPACTION > TRUE
        DclPrintf("FFX: Compaction thread '%s' stats: Called=%lU Completed=%lU Time=%lUms Longest=%lUms\n",
             pDsk->pFCI->szThreadName,
             pDsk->pFCI->ulCompactionsInitiated,
             pDsk->pFCI->ulCompactionsCompleted, pDsk->pFCI->ulTotalMS,
             pDsk->pFCI->ulSlowestCompactMS));
      #endif

        DclMutexRelease(pDsk->pFCI->hMutex);

        return FFXSTAT_SUCCESS;
    }
    else
    {
        /*  Really shouldn't ever happen...
        */
        DclPrintf("FFX: FfxDriverDiskCompactSuspend suspend compactor is failed !!!!!!!!!!!!!!!\n");
        DclError();

        return FFXSTAT_DRVCOMPACTSUSPENDFAILED;
    }
}


/*-------------------------------------------------------------------
    Public: FfxDriverDiskCompactResume()

    Resume background compaction operations.

    *Note* -- It is acceptable to call this function even when
          background compaction is already running.

    Parameters:
        pDsk - A pointer to the FFXDSKINFO structure to use

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskCompactResume(
    FFXDISKINFO    *pDsk)
{
    DclAssert(pDsk);

    if(!pDsk->pFCI)
        return FFXSTAT_DRVCOMPACTNOTENABLED;

//    if(1)
    if(DclMutexAcquire(pDsk->pFCI->hMutex))
    {
        /*  Reload the VBF handle since it might have changed
        */
        pDsk->pFCI->hVBF = pDsk->hVBF;

        pDsk->pFCI->fSuspended = FALSE;

      #if FFXCONF_STATS_COMPACTION > TRUE || D_DEBUG > 0
        DclPrintf("FFX: Background compaction thread resumed %X, %X\n", pDsk, pDsk->pFCI);
      #endif

        DclMutexRelease(pDsk->pFCI->hMutex);

        return FFXSTAT_SUCCESS;
    }
    else
    {
        /*  Really shouldn't ever happen...
        */
        DclError();

        return FFXSTAT_DRVCOMPACTRESUMEFAILED;
    }
}


/*-------------------------------------------------------------------
    Local: CompactionThread()

    This function implements background compaction.  It loops
    calling FfxVbfCompact() so long as the fSuspended flag is
    FALSE.

    If the fWaitingForCompletion flag becomes TRUE, indicating
    that the device/allocator is about to be dismounted, the
    thread will exit.  In this case, the fCompleted flag will
    be set to TRUE.

    This thread is intended to run at the lowest (idle) priority.
    However, in order to work on a variety of operating systems
    that have various scheduler quirks, several measures are
    taken to ensure that this thread does not detrimentally
    affect overall system performance.  These are:

    1) A compaction operation will not be requested until a
       minimum amount of time has passed since the last FlashFX
       read, write, or discard operation.  The read and write
       idle times are individually customizable, with discards
       currently being treated the same as writes.

    2) If a compaction operation was requested, but nothing was
       found to do, the thread will sleep for a customizable
       period of time (typically several seconds at least).

    3) Even when a compaction operation is requested and it
       succeeds, a short sleep operation is performed.  This is
       designed to allow other threads of equal priority to run,
       in the event that time-sliced preemption is not being
       used.

    Parameters:
        pThreadArg - A pointer to the FFXDSKINFO structure to use

    Return Value:
        Always returns NULL.
-------------------------------------------------------------------*/
static void * CompactionThread(
    void           *pThreadArg)
{
    FFXDISKINFO    *pDsk = pThreadArg;
    FFXCOMPACTINFO *pFCI;
  #if FFXCONF_STATS_COMPACTION
    unsigned        nBatchCount = 0;    /* Batch count of compactions completed */
    DCLTIMESTAMP    tsBatch;            /* Time batch was started */
  #endif

    DclAssert(pDsk);
    DclAssert(pDsk->pFCI);
    DclAssert(pDsk->hVBF);

    pFCI = pDsk->pFCI;

    while(!pFCI->fWaitingForCompletion)
    {
        D_UINT32       ulSleepMS;

        /*  Default to our standard sleep period that we use when there
            is nothing to do, or we are suspended.  If we actually do a
            successful compaction, we will shorten the sleep interval to
            ACTIVE_COMPACTION_SLEEP_INTERVAL for the iteration.
        */
        ulSleepMS = pFCI->ulSleepMS;

        if(DclMutexAcquire(pFCI->hMutex))
        {
            /*  Note that we are specifically checking for the fSuspended flag
                after we have acquired the mutex!
            */
            if(!pFCI->fSuspended)
            {
              #if FFXCONF_STATS_COMPACTION
                D_UINT32        ulMSecs;
                DCLTIMESTAMP    ts;

                pFCI->ulCompactionsInitiated++;

                ts = DclTimeStamp();

                /*  If the count is zero, we're starting a new batch, so
                    record the time.
                */
                if(!nBatchCount)
                    tsBatch = ts;
              #endif

                ulSleepMS = FfxVbfCompactIfIdle(pFCI->hVBF, FFX_COMPACTFLAGS_NORMAL);

              #if FFXCONF_STATS_COMPACTION
                ulMSecs = DclTimePassed(ts);

                /*  Record the slowest compaction
                */
                if(pFCI->ulSlowestCompactMS < ulMSecs)
                    pFCI->ulSlowestCompactMS = ulMSecs;

                /*  Note that the total time represents all the time spent
                    within FfxVbfCompactIdle(), even if an actual compaction
                    is not done.
                */
                pFCI->ulTotalMS += ulMSecs;
              #endif

                if(ulSleepMS == 0)
                {
                    /*  We've determined that there is nothing to compact.
                        Since we know this, we can also know that nothing
                        will become ready to compact until after a write
                        has been done.  If a write is done, we know that
                        we (this compaction thread) will be disallowed from
                        doing anything until the "WriteIdleMS" time period
                        has elapsed.

                        Therefore, we should wait at least as long as the
                        "WriteIdleMS" time period before bothering to wake
                        up and check again.  For good measure we wait the
                        longer of the standard sleep period and the write
                        idle threshold -- which is the ulLongSleepMS value
                        used here.
                    */
                    ulSleepMS = pFCI->ulLongSleepMS;
                }
                else if(ulSleepMS == 1)
                {
                    pFCI->ulCompactionsCompleted++;

                  #if FFXCONF_STATS_COMPACTION
                    nBatchCount++;
                  #endif

                    /*  If we cleaned something, sleep for a shorter period of
                        time, since we already know we are in an idle period,
                        and since we just found something to do, we can assume
                        there is yet more to do.

                        Note that we always have sleep intervals in this loop
                        to allow other threads of the same priority to execute,
                        just in case the system does not support time-sliced
                        preemption.
                    */
                    ulSleepMS = ACTIVE_COMPACTION_SLEEP_INTERVAL;
                }

              #if FFXCONF_STATS_COMPACTION
                if(nBatchCount && (ulSleepMS != ACTIVE_COMPACTION_SLEEP_INTERVAL))
                {
                    D_UINT32    ulBatchMS;

                    /*  Stop the batch timer.  Note that this time includes
                        the 1+ millisecond sleeps between compaction attempts.
                        Using the batch time is a good way to guage whether
                        sleep intervals in things like FSIOTEST are long
                        enough.
                    */
                    ulBatchMS = DclTimePassed(tsBatch);

                  #if FFXCONF_STATS_COMPACTION > TRUE
                    /*  Display a single message when we have compacted
                        everything we can (or are allowed to, right now).
                    */
                    DclPrintf("FFX: Completed a batch of %u background compactions in %lU ms.  Sleeping for %lU ms.\n",
                        nBatchCount, ulBatchMS, ulSleepMS);
                  #endif

                    /*  Reset the batch counter to zero and start anew...
                    */
                    nBatchCount = 0;
                }
              #endif
            }

            DclMutexRelease(pFCI->hMutex);
        }
        else
        {
            /*  really shouldn't ever happen...
            */
            DclError();
        }

        /*  If thread termination is requested, get out now...
        */
        if(pFCI->fWaitingForCompletion)
            break;

        DclOsSleep(ulSleepMS);
    }

  #if FFXCONF_STATS_COMPACTION > TRUE || D_DEBUG > 0
    DclPrintf("FFX: Exiting background compaction thread normally\n");
  #endif

    pFCI->fCompleted = TRUE;

    return NULL;
}



#endif /* FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD */

#endif  /* FFXCONF_VBFSUPPORT */
#endif  /* FFXCONF_ALLOCATORSUPPORT */

