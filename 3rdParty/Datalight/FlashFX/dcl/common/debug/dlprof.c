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

    This module provides an instrumented profiler implementation.  See
    dlprof.h for details about instrumenting code to use this profiler.

    Note that this code specifically avoids using mutexes to protect against
    concurrent access, except in the case of maintaining the thread list.
    Rather it uses static flags, structured such that the worst case scenario
    is that some profiler information may be lost.  This is a trade-off to
    avoid adding code to the profiler which would not only skew the results
    by adding significant overhead, but most certainly will cause unnatural
    context switches which will change system behavior.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlprof.c $
    Revision 1.35  2010/07/31 19:47:14Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.34  2010/07/15 01:38:04Z  garyp
    Fixed the Profiler "toggle" function not be calling back out and around
    through the requestor interface -- already inside the profiler.
    Revision 1.33  2010/07/15 01:02:53Z  garyp
    Added functionality to reset the profiler data.
    Revision 1.32  2010/04/21 16:57:52Z  garyp
    Tweaked the calibration code so the minimum delay is 250 ns.
    Revision 1.31  2010/04/18 18:53:26Z  garyp
    Removed some unused prototypes.
    Revision 1.30  2010/04/17 21:39:46Z  garyp
    Revamped the self-calibration (yet again).  Changed the calibration
    feature so that it can now be enabled/disabled at run-time.  Fixed so
    the calibration can take place even if the profiler has already been
    stopped.  Removed the use of the 64-bit macros except in the case
    of multiply/divide.
    Revision 1.29  2010/01/07 02:45:14Z  garyp
    Updated to use DclInstanceNumber().
    Revision 1.28  2009/11/16 03:09:13Z  garyp
    Updated to be able to programmatically control whether the profiler
    does memory pool validation.  Eliminated some static variables.
    Revision 1.27  2009/11/09 15:31:23Z  garyp
    Updated to initialize the profiler as a service.  Now use the Atomic
    API to manipulate the enable/disable/busy flag.
    Revision 1.26  2009/11/02 20:53:48Z  garyp
    Updated some messages/debug code -- no functional changes.
    Revision 1.25  2009/10/14 01:54:28Z  garyp
    Updated the self-calibration function to be skipped if the high-res
    timer is non-functional or insufficient.
    Revision 1.24  2009/06/24 22:07:35Z  garyp
    Modified to use the updated memory tracking functions.
    Revision 1.23  2009/04/09 22:05:37Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.22  2009/03/25 16:52:58Z  garyp
    Removed an unused variable.
    Revision 1.21  2009/02/08 02:30:15Z  garyp
    Merged from the v4.0 branch.  Major update to take advantage of high-
    res timers.  Made the overhead self-calibration more accurate.  Added
    an option to ignore meaningless records.
    Revision 1.20  2008/06/03 21:43:07Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.19  2007/12/18 21:00:13Z  brandont
    Updated function headers.
    Revision 1.18  2007/12/01 02:06:10Z  brandont
    Corrected warnings for unused arguments.
    Revision 1.17  2007/11/03 23:31:10Z  Garyp
    Added the standard module header.
    Revision 1.16  2007/10/16 19:16:28Z  Garyp
    Modified to use the updated implementation of DclMemPoolVerify().
    Revision 1.15  2006/10/26 03:10:21Z  Garyp
    Updated to work around a potential overflow problem.
    Revision 1.14  2006/10/05 20:01:24Z  Garyp
    Modified to use a corrected macro name.
    Revision 1.13  2006/08/28 03:28:17Z  Garyp
    Fixed to compile cleanly with emulated 64-bit types.
    Revision 1.12  2006/08/21 23:16:23Z  Garyp
    Various fixes to work properly in a multi-threaded environment and in
    combination with memory tracking.  Further cleanup of the report to make
    it more readable.
    Revision 1.11  2006/08/18 19:31:51Z  Garyp
    Minor enhancements to the format of the report.  Fixed to use the new
    64-bit comparison macros.  Minor changes to the stack usage reporting.
    Revision 1.10  2006/08/07 23:38:48Z  Garyp
    Updated to use DCLALIGNEDSTRUCT() to ensure structure alignment.
    Revision 1.9  2006/07/07 18:21:26Z  Garyp
    Fixed a reentrancy problem with memory pool validation.  Added the ability
    to disable the output of timing information.
    Revision 1.8  2006/03/09 00:53:21Z  Garyp
    Updated to work with the enhanced sprintf code which prints "null" for
    null string pointers.
    Revision 1.7  2006/03/04 17:20:34Z  Garyp
    Added DclProfContextName().
    Revision 1.6  2006/02/22 23:29:23Z  Garyp
    Modified DclProfEnter/Leave() to validate the memory pool (if memory
    tracking is enabled).
    Revision 1.5  2006/01/10 05:21:10Z  Garyp
    Debug code updated.
    Revision 1.4  2006/01/02 08:31:22Z  Garyp
    Updated to work properly in a multithreaded environment.  Added the ability
    to record maximum stack depths.
    Revision 1.3  2005/12/09 21:54:04Z  Garyp
    Cosmetic change to make the top-level functions stand out.
    Revision 1.2  2005/12/08 23:39:35Z  pauli
    Updated to use DclPtr macro.
    Revision 1.1  2005/12/07 03:52:02Z  Pauli
    Initial revision
    Revision 1.5  2005/12/07 03:52:02Z  Garyp
    Eliminated // comments.
    Revision 1.4  2005/11/13 04:41:05Z  Garyp
    Minor fix to a message.
    Revision 1.3  2005/11/07 18:25:01Z  Garyp
    Minor type changes to mollify MSVC6.
    Revision 1.2  2005/10/09 18:45:38Z  Garyp
    Minor cleanup.  Started adding support for recording stack depth (not
    completed).
    Revision 1.1  2005/10/02 04:57:08Z  Garyp
    Initial revision
    Revision 1.8  2005/09/16 05:14:17Z  garyp
    Added DclProfToggle().
    Revision 1.7  2005/08/16 22:34:31Z  garyp
    Fixed to cleanly handle running out of nesting levels.
    Revision 1.6  2005/08/06 01:48:36Z  pauli
    Revision 1.5  2005/08/04 03:47:12Z  Garyp
    Minor tweak to the banner.
    Revision 1.4  2005/08/03 19:17:18Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.2  2005/07/31 04:42:48Z  Garyp
    Modified DclProfilerLeave() to take ulUserData parameter which is summed
    and displayed in the summary statistics.
    Revision 1.1  2005/07/25 21:43:28Z  pauli
    Initial revision
    Revision 1.4  2005/06/20 20:25:12Z  PaulI
    Updated to use DCL 64-bit operations.
    Revision 1.3  2005/06/16 20:38:46Z  PaulI
    Replaced TFS asserts with DCL asserts.
    Expanded asserts with multiple conditions (where appropriate).
    Revision 1.2  2005/06/15 19:27:37Z  Pauli
    Removed obsolete TfsAssertModuleName().
    Revision 1.1  2005/05/27 23:01:14Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlprof.h>
#include <dlservice.h>
#include <dlinstance.h>
#include <dlapiprv.h>

#if DCLCONF_PROFILERENABLED

#define DCLPROF_CALIBRATE           0x80000000
#define DCLPROF_SKIPPED             0x40000000

#define DUMP_RECORDS                FALSE           /* Must be FALSE for checkin */
#define IGNORE_TIMINGS              FALSE           /* Must be FALSE for checkin */

#define DCLPROF_CALIBRATENS         (200*1000*1000)  /* ns to spend calibrating (200ms default) */
#define DCLPROF_MASTERID            ((DCLRECORDID)0xFFFFFFFF)
#define MAXFUNCNAMELEN               50             /* (includes indents) */

/*  Values used for ulAtomicProfilerState
*/
#define PROF_DISABLED               (FALSE)
#define PROF_ENABLED                (TRUE)
#define PROF_BUSY                   (D_UINT32_MAX-1)

/*  The unique record ID is a <pointer> to the static record name value
    created in the stack frame of the calling code.
*/
typedef const char *                DCLRECORDID;

/*  Internal flags
*/
#define INTFLAG_PROCESSED           0x0001  /* used during report generation */
#define INTFLAG_FORCETOPLEVEL       0x0002  /* record forced to top level */
#define INTFLAG_ORPHANEDCHILDREN    0x0004  /* record has children forced to top level */
#define INTFLAG_CALIBRATE           0x0008  /* denote the calibration records */

/*  DCLPROFRECORD
*/
typedef struct sDCLPROFRECORD
{
    D_UINT64                ullSubCallCount;    /* Total count of nested calls */
    D_UINT64                ullTotalLocalTimeUS;/* Total us in this function alone */
    D_UINT64                ullTotalTreeTimeUS; /* Total us under this tree   */
    D_UINT64                ullMaxTreeTimeUS;   /* Max tree time in usecs */
    D_UINT64                ullTotalUserData;   /* Total user data */
    D_UINT64                ullLocalCallCount;
    DCLRECORDID             idCurrent;
    DCLTHREADID             idThread;
    const char             *pszLabel;
    D_BUFFER               *pStackPtr;
    struct sDCLPROFRECORD  *pTop;               /* The top record (0 if master, self if top) */
    struct sDCLPROFRECORD  *pParent;            /* The parent record (0 if master) */
    struct sDCLPROFRECORD  *pSibling;           /* The next sibling (0 if none) */
    struct sDCLPROFRECORD  *pChild;             /* The first child (0 if none) */
    ptrdiff_t               nMaxStackDepth;     /* Max stack depth relative to top level record */
    D_UINT32                ulCallCount;
    D_UINT32                ulMaxUserData;      /* Max single user data value */
    D_UINT16                uLevel;
    D_UINT16                uIntFlags;          /* Internal flags */
} DCLPROFRECORD;

DCLALIGNEDSTRUCT(DCLPROFRECORD_ALIGNED, DCLPROFRECORD, pr);

/*  DCLPROFSTACK
*/
typedef struct sDCLPROFSTACK
{
    D_UINT64                ullSubTreeTimeUS;
    DCLTIMESTAMP            tsStart;
    DCLPROFRECORD          *pRecord;
    struct sDCLPROFSTACK   *pNext;
    struct sDCLPROFSTACK   *pPrev;
    D_UINT32                ulSubCallCount;
    D_UINT16                uLevel;
} DCLPROFSTACK;

DCLALIGNEDSTRUCT(DCLPROFSTACK_ALIGNED, DCLPROFSTACK, ps);

/*  DCLPROFTHREAD
*/
typedef struct
{
    DCLTHREADID         idThread;
    DCLPROFSTACK       *pStackTop;          /* The first stack record */
    DCLPROFSTACK       *pStackPointer;      /* The current stack pointer */
    ptrdiff_t           nMaxStackDepth;     /* Max stack depth relative to top level record */
    unsigned            nMaxNestLevels;
    unsigned            nMaxNestLevelHit;
    unsigned            nThreadBusyCount : 8;
    unsigned            fStackOverflow   : 1;
    unsigned            fStackUnderflow  : 1;
} DCLPROFTHREAD;

DCLALIGNEDSTRUCT(DCLPROFTHREAD_ALIGNED, DCLPROFTHREAD, pt);

/*  DCLPROFINFO
*/
typedef struct
{
    DCLPROFTHREAD      *pThread;
    DCLPROFRECORD      *pMasterRecord;
    DCLPROFRECORD      *pNextFreeRecord;
    unsigned            nMaxRecords;
    D_UINT32            ulFreeRecs;
    D_UINT32            ulOverheadTopLevNS;
    D_UINT32            ulOverheadSubLevNS;
    D_UINT32            ulDisplayLastCallCount;
    D_UINT16            uDisplayLastLevel;
    unsigned            nMaxThreads;
    unsigned            nThreadCount;
    unsigned            nRecordsSkipped;
    unsigned            fStackGrowsDown          : 1;
    unsigned            fRecordsLost             : 1;
    unsigned            fRecordContention        : 1;
    unsigned            fThreadSlotOverflow      : 1;
    unsigned            fEnableMemPoolValidation : 1;
} DCLPROFINFO;

static D_ATOMIC32       ulAtomicProfilerState = PROF_BUSY; /* AKA: uninitialized */
static DCLPROFINFO     *pProf = NULL;
static D_BOOL           fIgnoreTimings = IGNORE_TIMINGS;

/*  Prototypes for functions related to the IOCTL interface
*/
static DCLSTATUS        ProfServiceIoctl(DCLSERVICEHANDLE hService, DCLREQ_SERVICE *pReq, void *pPriv);
static DCLSTATUS        ProfCreate(unsigned nMaxThreads, unsigned nMaxRecords, unsigned nMaxNestLevels);
static DCLSTATUS        ProfDestroy(void);
static DCLSTATUS        ProfEnable(void);
static DCLSTATUS        ProfDisable(void);
static DCLSTATUS        ProfReset(void);
static DCLSTATUS        ProfSummary(D_BOOL fReset, D_BOOL fShort, D_BOOL fAdjustOverhead);
static DCLSTATUS        ProfToggle(D_BOOL fSummaryReset);

/*  Prototypes for internal functions
*/
static void             Reset(                  DCLPROFINFO *pPI);
static void             ResetThread(            DCLPROFINFO *pPI, DCLPROFTHREAD *pThread);
static void             DisplayFunctionReport(  DCLPROFINFO *pPI, D_BOOL fShort, D_BOOL fAdjustOverhead);
static void             DisplayCallTreeReport(  DCLPROFINFO *pPI, D_BOOL fShort, D_BOOL fAdjustOverhead);
static void             ProcessNode(            DCLPROFINFO *pPI, DCLPROFRECORD *pNode, D_UINT16 uLevel, char *pBuffer, D_BOOL fShort, D_BOOL fAdjustOverhead);
static void             DisplayMatchingRecords( DCLPROFINFO *pPI, DCLPROFRECORD *pRec, D_BOOL fShort, D_BOOL fAdjustOverhead);
static void             DisplayRecord(          DCLPROFINFO *pPI, DCLPROFRECORD *pRec, D_UINT16 uLevel, char *pBuffer, unsigned nContexts, D_BOOL fShort, D_BOOL fAdjustOverhead);
static void             UpdateRecord(           DCLPROFSTACK *pStack, D_UINT32 ulElapsed, D_UINT32 ulUserData);
static DCLPROFSTACK    *SetNextStack(           DCLPROFTHREAD *pThread);
static DCLPROFSTACK    *SetPreviousStack(       DCLPROFTHREAD *pThread);
static D_BOOL           PushStack(              DCLPROFTHREAD *pThread, const char *pszLabel, DCLRECORDID idCurrent, D_UINT32 *pulFlags, D_UINT32 ulUserData);
static void             PopStack(               DCLPROFTHREAD *pThread, const char *pszLabel, DCLRECORDID idCurrent, D_UINT32 ulFlags, D_UINT32 ulUserData);
static unsigned         AddContexts(            DCLPROFRECORD *pStartRec, DCLPROFRECORD *pSummaryRec);
static DCLPROFRECORD   *AllocateRecord(         DCLPROFTHREAD *pThread, DCLPROFSTACK *pStack, DCLRECORDID idCurrent, const char *pszLabel, D_UINT32 *pulFlags);
static DCLPROFRECORD   *GetNewRecord(           DCLPROFRECORD *pTopRec, DCLPROFRECORD *pParentRec, DCLTHREADID idThread, DCLRECORDID idNew, const char *pszLabel, D_UINT32 *pulFlags);
static unsigned         GetThreadNum(           DCLTHREADID idThread);
static D_BOOL           CheckStackDirection(    DCLRECORDID *pID);
static DCLPROFTHREAD   *GetThreadContext(       D_BOOL fCreate);
static void             CalculateOverhead(void);
#if DUMP_RECORDS
static void             DumpRecord(DCLPROFRECORD *pRecord);
#endif


                    /*-----------------------------*\
                     *                             *
                     *      Public Interface       *
                     *                             *
                    \*-----------------------------*/


/*-------------------------------------------------------------------
    Protected: DclProfServiceInit()

    Parameters:
        hDclInst - The DCL instance handle.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclProfServiceInit(
    DCLINSTANCEHANDLE       hDclInst)
{
    static DCLSERVICEHEADER SrvProf[DCL_MAX_INSTANCES];
    DCLSERVICEHEADER       *pSrv;
    DCLSTATUS               dclStat;

    /*  REMINDER! Because some environments are capable of restarting,
                  without reinitializing static data, this function must
                  explicitly ensure that any static data is manually
                  zero'd, rather than just expecting it to already be in
                  that state.
    */

    if(!hDclInst)
        return DCLSTAT_SERVICE_BADHANDLE;

    pSrv = &SrvProf[DclInstanceNumber(hDclInst)];
    dclStat = DclServiceHeaderInit(&pSrv, "DLPROF", DCLSERVICE_PROFILER, ProfServiceIoctl, NULL, DCLSERVICEFLAG_THREADSAFE);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        dclStat = DclServiceCreate(hDclInst, pSrv);
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Protected: DclProfEnter()

    This function is called (typically via the DclProfilerEnter()
    macro) upon entry into a function that is to be profiled.

    Parameters:
        pszLabel     - A pointer to the null-terminated label to use
                       for the function -- typically the function
                       name.
        pulProfFlags - A pointer to the profiler flags.  This must
                       be allocated on the stack -- see the macro
                       DclProfilerEnter() for more information.
        ulUserData   - Any generic user data that you want to have
                       accumulated for this function.  The sum total
                       value will be displayed as a part of the
                       report.
    Return Value:
        None.
-------------------------------------------------------------------*/
void DclProfEnter(
    const char     *pszLabel,
    D_UINT32       *pulProfFlags,
    D_UINT32        ulUserData)
{
    DCLRECORDID     idCurrent = pszLabel;
    DCLPROFTHREAD  *pThread;

    DclAssert(pszLabel);
    DclAssert(pulProfFlags);

    if(*pulProfFlags & DCLPROF_IGNORE)
        return;

    pThread = GetThreadContext(TRUE);
    if(!pThread)
    {
        *pulProfFlags |= DCLPROF_SKIPPED;
        return;
    }

    /*  For this thread, indicate that we are in the profiler code
    */
    pThread->nThreadBusyCount++;
    if(pThread->nThreadBusyCount == 1)
    {
        if(pProf->fEnableMemPoolValidation)
        {
            DCLSTATUS   dclStat;

            dclStat = DclMemTrackPoolVerify(0, FALSE);
            if( dclStat != DCLSTAT_SUCCESS &&
                dclStat != DCLSTAT_SERVICE_NOTREGISTERED &&
                dclStat != DCLSTAT_FEATUREDISABLED &&
                dclStat != DCLSTAT_BUSY)
            {
                DclPrintf("Memory pool validation failure on entry to '%s', Status=%lX\n",
                    pszLabel, dclStat);

                DclProductionError();
            }
        }
        
        /*  Nest a level deeper and create a new entry for this function
        */
        if(!PushStack(pThread, pszLabel, idCurrent, pulProfFlags, ulUserData))
            *pulProfFlags |= DCLPROF_SKIPPED;
    }
    else
    {
        /*
        We must quietly handle this situation since the profiler itself
        may use functions that are being profiled, such as DclMemCpy(),
        DclMemSet(), etc., otherwise we would do something like...

        DclPrintf("DCL: Profiler Busy, enter(%s)\n", pszLabel);
        DclProfDumpCallTree();
        DclProductionError();
        */

/* Shouldn't we still do this though???
        *pulProfFlags |= DCLPROF_SKIPPED;
*/        
    }

    /*  Indicate that we are out of the profiler code
    */
    pThread->nThreadBusyCount--;
}


/*-------------------------------------------------------------------
    Protected: DclProfLeave()

    This function is called (typically via the DclProfilerLeave()
    macro) upon exit from a function that is being profiled.

    Parameters:
        pszLabel     - A pointer to the null-terminated label to use
                       for the function -- typically the function
                       name.  This must be the same pointer that was
                       passed to DclProfEnter().
        pulProfFlags - A pointer to the profiler flags.  This must
                       be allocated on the stack -- see the macro
                       DclProfilerEnter() for more information.  This
                       must be the same pointer that was passed to
                       DclProfEnter().
        ulUserData   - Any generic user data that you want to have
                       accumulated for this function.  The sum total
                       value will be displayed as a part of the
                       report.
    Return Value:
        None.
-------------------------------------------------------------------*/
void DclProfLeave(
    const char     *pszLabel,
    D_UINT32       *pulProfFlags,
    D_UINT32        ulUserData)
{
    DCLRECORDID     idCurrent = pszLabel;
    DCLPROFTHREAD  *pThread;

    DclAssert(pszLabel);
    DclAssert(pulProfFlags);

    /*  If this is a record that we were explicitly directed to ignore,
        or for whatever reason had to be skipped, just return.
    */
    if(*pulProfFlags & (DCLPROF_IGNORE | DCLPROF_SKIPPED))
        return;

    pThread = GetThreadContext(TRUE);
    if(!pThread)
        return;

    /*  For this thread, indicate that we are in the profiler code
    */
    pThread->nThreadBusyCount++;
    if(pThread->nThreadBusyCount == 1)
    {
        if(pProf->fEnableMemPoolValidation)
        {
            DCLSTATUS   dclStat;

            dclStat = DclMemTrackPoolVerify(0, FALSE);
            if( dclStat != DCLSTAT_SUCCESS &&
                dclStat != DCLSTAT_SERVICE_NOTREGISTERED &&
                dclStat != DCLSTAT_FEATUREDISABLED &&
                dclStat != DCLSTAT_BUSY )
            {
                DclPrintf("Memory pool validation failure on exit from '%s', Status=%lX\n",
                pszLabel, dclStat);

                DclProductionError();
            }
        }
        
        /*  Record everything for the current level and pop a level off
        */
        PopStack(pThread, pszLabel, idCurrent, *pulProfFlags, ulUserData);
    }
    else
    {
        /*
        We must quietly handle this situation since the profiler itself
        may use functions that are being profiled, such as DclMemCpy(),
        DclMemSet(), etc., otherwise we would do something like...

        DclPrintf("DCL: Profiler Busy, leave(%s)\n", pszLabel);
        DclProfDumpCallTree();
        DclProductionError();
        */
    }

    /*  Indicate that we are out of the profiler code
    */
    pThread->nThreadBusyCount--;
}


/*-------------------------------------------------------------------
    Protected: DclProfContextName()

    This function returns a pointer to the static name of the
    currently executing context.

    Parameters:
        None.

    Return Value:
        Returns a pointer to a constant string which is the most
        recently entered profiler context, or NULL if it is not
        known or not available.
-------------------------------------------------------------------*/
const char *DclProfContextName(void)
{
    DCLPROFTHREAD  *pThread;

    pThread = GetThreadContext(FALSE);
    if(!pThread)
        return NULL;

    DclAssert(pThread->pStackPointer);
    DclAssert(pThread->pStackPointer->pRecord);

    return pThread->pStackPointer->pRecord->pszLabel;
}


/*-------------------------------------------------------------------
    Protected: DclProfDumpCallTree()

    This function dumps the function call tree, as instrumented
    with profiler code.

    Parameters:
        None.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclProfDumpCallTree(void)
{
    DCLPROFTHREAD      *pThread;
    static D_UINT32     ulAtomicSectionGate = 0;

    /*  Exit now if the profiler has not been initialized, otherwise
        GetThreadContext() will try to initialize it!
    */
    if(!pProf)
        return;

    /*  Don't allow recursive entries into this code
    */
    if(!DclOsAtomic32SectionEnter(&ulAtomicSectionGate))
        return;

    pThread = GetThreadContext(FALSE);
    if(pThread)
    {
        D_UINT32    ulOldState;

        /*  Disable the profiler while dumping this stuff out.  At the 
            same time, setting the BUSY state prevents other reports
            from happening while we are in this code.
        */
        ulOldState = DclOsAtomic32Exchange(&ulAtomicProfilerState, PROF_BUSY);
        if(ulOldState != PROF_BUSY)
        {
            if(pThread->pStackPointer && pThread->pStackPointer->pRecord)
            {
                DCLPROFRECORD  *pRecord;

                DclPrintf("DCL: Profiler CallTree: ");

                pRecord = pThread->pStackPointer->pRecord;

                while(pRecord && pRecord->idCurrent && pRecord->idCurrent != DCLPROF_MASTERID)
                {
                    DclPrintf("%s<-", pRecord->pszLabel);
                    pRecord = pRecord->pParent;
                }
                DclPrintf("\n");
            }

            /*  Atomically restore the original state
            */
            DclOsAtomic32Exchange(&ulAtomicProfilerState, ulOldState);
        }
    }

    DclOsAtomic32SectionLeave(&ulAtomicSectionGate);

    return;
}



                    /*-----------------------------*\
                     *                             *
                     *      Helper Functions       *
                     *                             *
                    \*-----------------------------*/



/*-------------------------------------------------------------------
    Local: ProfServiceIoctl()

    Parameters:
        hService - The service handle
        pReq     - A pointer to the DCLREQ_SERVICE structure
        pPriv    - A pointer to any private data to pass

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS ProfServiceIoctl(
    DCLSERVICEHANDLE    hService,
    DCLREQ_SERVICE     *pReq,
    void               *pPriv)
{
    DclAssert(hService);
    DclAssertWritePtr(pReq, sizeof(*pReq));

    if(pReq->ior.ulReqLen != sizeof(*pReq))
        return DCLSTAT_BADSTRUCLEN;

    switch(pReq->ior.ioFunc)
    {
        case DCLIOFUNC_SERVICE_CREATE:
            return ProfCreate(DCLPROF_MAX_THREADS, DCLPROF_MAX_RECORDS, DCLPROF_MAX_NESTING_LEVEL);

        case DCLIOFUNC_SERVICE_DESTROY:
            return ProfDestroy();

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
                case DCLIOFUNC_PROFILER_SUMMARY:
                {
                    DCLREQ_PROFILER_SUMMARY *pSubReq = (DCLREQ_PROFILER_SUMMARY*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    pSubReq->dclStat = ProfSummary(pSubReq->fReset, pSubReq->fShort, pSubReq->fAdjustOverhead);

                    return DCLSTAT_SUCCESS;
                }

                case DCLIOFUNC_PROFILER_ENABLE:
                {
                    DCLREQ_PROFILER_ENABLE   *pSubReq = (DCLREQ_PROFILER_ENABLE*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    pSubReq->dclStat = ProfEnable();

                    return DCLSTAT_SUCCESS;
                }

                case DCLIOFUNC_PROFILER_DISABLE:
                {
                    DCLREQ_PROFILER_DISABLE  *pSubReq = (DCLREQ_PROFILER_DISABLE*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    pSubReq->dclStat = ProfDisable();

                    return DCLSTAT_SUCCESS;
                }

                case DCLIOFUNC_PROFILER_RESET:
                {
                    DCLREQ_PROFILER_RESET  *pSubReq = (DCLREQ_PROFILER_RESET*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    pSubReq->dclStat = ProfReset();

                    return DCLSTAT_SUCCESS;
                }

                case DCLIOFUNC_PROFILER_TOGGLE:
                {
                    DCLREQ_PROFILER_TOGGLE  *pSubReq = (DCLREQ_PROFILER_TOGGLE*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    pSubReq->dclStat = ProfToggle(pSubReq->fSummaryReset);

                    return DCLSTAT_SUCCESS;
                }

                case DCLIOFUNC_PROFILER_MEMPOOLSTATE:
                {
                    DCLREQ_PROFILER_MEMPOOLSTATE  *pSubReq = (DCLREQ_PROFILER_MEMPOOLSTATE*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    pSubReq->fOldValidationEnabled = (pProf->fEnableMemPoolValidation == TRUE);
                    pProf->fEnableMemPoolValidation = (pSubReq->fNewValidationEnabled == TRUE);

                    DclPrintf("Profiler memory pool validation state set to %u, was %u\n", 
                        pSubReq->fNewValidationEnabled, pSubReq->fOldValidationEnabled);
                        
                    return DCLSTAT_SUCCESS;
                }

                default:
                    DCLPRINTF(1, ("ProfServiceIoctl() Unsupported subfunction %x\n", pReq->pSubRequest->ioFunc));
                    return DCLSTAT_SERVICE_UNSUPPORTEDREQUEST;
            }
        }

        default:
            DCLPRINTF(1, ("ProfServiceIoctl() Unhandled request %x\n", pReq->ior.ioFunc));
            return DCLSTAT_SERVICE_UNHANDLEDREQUEST;
    }
}


/*-------------------------------------------------------------------
    Local: ProfCreate()

    Parameters:
        nMaxThreads     - The maximum number of threads for which
                          profiler information will be tracked.
        nMaxRecords     - The maxumum number of records which will
                          stored by the profiler (shared across all
                          threads).
        nMaxNestLevels  - The maximum function nesting level within
                          each context, relative to the top-most
                          function which invokes the profiler.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS ProfCreate(
    unsigned                nMaxThreads,
    unsigned                nMaxRecords,
    unsigned                nMaxNestLevels)
{
    DCLSTATUS               dclStat;
    DCLPROFINFO            *pPI;
    DCLRECORDID             idTemp;
    D_UINT32                ulLen;
    D_UINT32                ulTotalMem;
    unsigned                tt;

    DclAssert(!pProf);
    DclAssert(nMaxThreads);
    DclAssert(nMaxRecords);
    DclAssert(nMaxNestLevels);
    DclAssert(!(sizeof(DCLPROFRECORD_ALIGNED) % DCL_ALIGNSIZE));
    DclAssert(!(sizeof(DCLPROFTHREAD_ALIGNED) % DCL_ALIGNSIZE));
    DclAssert(!(sizeof(DCLPROFSTACK_ALIGNED) % DCL_ALIGNSIZE));
    DclAssert(DclOsAtomic32Retrieve(&ulAtomicProfilerState) == PROF_BUSY);

  #if D_DEBUG
    DclPrintf("NOTE: Profiling in DEBUG mode will provide skewed results\n");
  #endif

    dclStat = DclMemTrackPoolVerify(0, FALSE);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        DclPrintf("NOTE: Profiling with memory tracking enabled will provide skewed results\n");
    }
    else if(dclStat != DCLSTAT_SERVICE_NOTREGISTERED && dclStat != DCLSTAT_FEATUREDISABLED)
    {
        DclPrintf("Memory pool verification failed, Status=%lX\n", dclStat);
        return dclStat;
    }

    ulLen = sizeof(DCLPROFINFO);
    pPI = DclMemAllocZero(ulLen);
    if(!pPI)
        goto CreateFailed;

    ulTotalMem = ulLen;

    pPI->nMaxThreads = nMaxThreads;
    pPI->nMaxRecords = nMaxRecords;
    pPI->fStackGrowsDown = CheckStackDirection(&idTemp);
    pPI->fEnableMemPoolValidation = TRUE;
        
    /*  Allocate space for the records.  This space is shared by all
        thread and stack contexts.
    */
    ulLen = sizeof(DCLPROFRECORD_ALIGNED) * nMaxRecords;
    pPI->pMasterRecord = DclMemAlloc(ulLen);
    if(!pPI->pMasterRecord)
        goto CreateFailed;

    ulTotalMem += ulLen;

    /*  Allocate space for the thread info.
    */
    ulLen = sizeof(DCLPROFTHREAD_ALIGNED) * nMaxThreads;
    pPI->pThread = DclMemAllocZero(ulLen);
    if(!pPI->pThread)
        goto CreateFailed;

    ulTotalMem += ulLen;

    /*  If a max nest level is specified, pre-allocate the stack space for
        each thread context (required behavior for now).  Add one for the
        master record.
    */
    if(nMaxNestLevels)
    {
        ulLen = sizeof(DCLPROFSTACK_ALIGNED) * (nMaxNestLevels+1);
        for(tt=0; tt<nMaxThreads; tt++)
        {
            pPI->pThread[tt].pStackTop = DclMemAlloc(ulLen);
            if(!pPI->pThread[tt].pStackTop)
                goto CreateFailed;

            ulTotalMem += ulLen;

            pPI->pThread[tt].nMaxNestLevels = nMaxNestLevels+1;
        }
    }

    Reset(pPI);

    pProf = pPI;

    DclPrintf("Profiler initialized using %lU KB of memory -- Initial state is: %s\n",
        (ulTotalMem + 512) /1024, DCLPROF_AUTOENABLE ? "Enabled" : "Disabled");

    /*  Change the profiler state from BUSY to either ENABLED or DISABLED,
        based on the DCLPROF_AUTOENABLE setting.  Either way, the profiler
        is ready for use.
    */
    DclAssert(DCLPROF_AUTOENABLE == PROF_ENABLED || DCLPROF_AUTOENABLE == PROF_DISABLED);
    DclOsAtomic32Exchange(&ulAtomicProfilerState, DCLPROF_AUTOENABLE);

    /*  All good
    */
    return DCLSTAT_SUCCESS;

  CreateFailed:

    DclPrintf("The profiler was unable to allocate %lU bytes of memory\n", ulLen);

    /*  Release anything and everything we may have allocated, in
        reverse order.
    */
    if(pPI)
    {
        if(nMaxNestLevels)
        {
            /*  Deallocate in reverse order
            */
            for(tt=nMaxThreads-1; tt>=0; tt--)
            {
                if(pPI->pThread[tt].pStackTop)
                    DclMemFree(pPI->pThread[tt].pStackTop);
            }
        }

        if(pPI->pThread)
            DclMemFree(pPI->pThread);

        if(pPI->pMasterRecord)
            DclMemFree(pPI->pMasterRecord);

        DclMemFree(pPI);
    }

    return DCLSTAT_MEMALLOCFAILED;
}


/*-------------------------------------------------------------------
    Protected: ProfDestroy()

    Destroy the profiler instance.  

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS ProfDestroy(void)
{
    int     tt; /* (used in signed fashion) */

    /*  Irrespective of anything else that is going on, setting the
        busy flag prevents anyone else from entering the profiler.
    */        
    DclOsAtomic32Exchange(&ulAtomicProfilerState, PROF_BUSY);

    /*  NOTE:  We have a deficiency here in that while setting the BUSY
               flag above will guarantee that nobody new will enter the
               profiler code, it does not guarantee that all threads
               which were already in the code have exited.
    */

    DclAssert(pProf);

    if(pProf->pThread)
    {
        /*  Deallocate in reverse order
        */
        for(tt=pProf->nMaxThreads-1; tt>=0; tt--)
        {
            if(pProf->pThread[tt].pStackTop)
                DclMemFree(pProf->pThread[tt].pStackTop);
        }

        DclMemFree(pProf->pThread);
    }

    if(pProf->pMasterRecord)
    {
        DclMemFree(pProf->pMasterRecord);
    }

    DclMemFree(pProf);

    pProf = NULL;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: ProfSummary()

    Parameters:
        fReset - TRUE to reset the profiler data
        fShort - TRUE if a short summary is desired

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS ProfSummary(
    D_BOOL      fReset,
    D_BOOL      fShort,
    D_BOOL      fAdjustOverhead)
{
    unsigned    kk;
    D_UINT32    ulOldState;

    if(!pProf)
    {
        DclPrintf("Profiler not initialized\n");
        return DCLSTAT_NOTENABLED;
    }

    ulOldState = DclOsAtomic32Retrieve(&ulAtomicProfilerState);
    if(ulOldState == PROF_BUSY)
    {
        /*  The profiler could be busy doing some other report.  In that
            event, ignore the request.
        */            
        DclPrintf("Profiler is busy or uninitialized\n");
        return DCLSTAT_BUSY;
    }

    /*  Reminder!  CalculateOverhead() may put the profiler into
                   the BUSY state, and leave it there.
    */                  
    if(fAdjustOverhead)
        CalculateOverhead();

    /*  Indicate that we are in the profiler code.  Whether the profiler
        was enabled or disabled upon entering this code, disabling this
        will prevent anyone else from accessing the profiler while we are
        dumping the summary.
    */
    DclOsAtomic32Exchange(&ulAtomicProfilerState, PROF_BUSY);

    /*  Get the total elapsed time, in or under this function
    */
/*    ulElapsed = DclTimePassed(pStackTop->tsStart);
    UpdateRecord(pStackTop, ulElapsed, 0);
*/
    pProf->nRecordsSkipped = 0;

    DisplayFunctionReport(pProf, fShort, fAdjustOverhead);
    DisplayCallTreeReport(pProf, fShort, fAdjustOverhead);

    DclPrintf("Used %u of %u record slots.  %u %s.  CallOverhead=%lUns SubLevOverhead=%lUns fRecsLost=%d fRecContention=%d\n",
        pProf->nMaxRecords - pProf->ulFreeRecs, pProf->nMaxRecords,
        pProf->nRecordsSkipped, fShort ? "Records Skipped" : "~Meaningless Records",
        pProf->ulOverheadTopLevNS, pProf->ulOverheadSubLevNS,
        pProf->fRecordsLost, pProf->fRecordContention);
    DclPrintf("'~' Records that could be skipped.  '*' Records forced to be TopLevel.  '!' Records with children who were forced to be TopLevel.\n");

    DclPrintf("---------------------------------------------------[ Profiler Thread Summary ]---------------------------------------------------\n");
    for(kk=0; kk<pProf->nMaxThreads; kk++)
    {
        if(pProf->pThread[kk].idThread)
        {
            DclPrintf("Thread %c - TID=%lX.  Used %2u of %u nest levels.  MaxStackUsed=0x%04x  fOverflowed=%d  fUnderflowed=%d\n",
                kk+'A',
                pProf->pThread[kk].idThread,
                pProf->pThread[kk].nMaxNestLevelHit,
                pProf->pThread[kk].nMaxNestLevels-1,
                pProf->pThread[kk].nMaxStackDepth,
                pProf->pThread[kk].fStackOverflow,
                pProf->pThread[kk].fStackUnderflow);
        }
    }

    DclPrintf("Used %u of %u thread slots.  fThreadOverflow=%d.\n",
        pProf->nThreadCount, pProf->nMaxThreads, pProf->fThreadSlotOverflow);

    DclPrintf("Definitions:\n");
    DclPrintf("  Call Count         The total number of times the function was called.\n");
    DclPrintf("  Call Tree AvgUS    The average number of microseconds each call spent in or under the function.\n");
    DclPrintf("  Call Tree MaxUS    The maximum number of microseconds any single call spent in or under the function.\n");
    DclPrintf("  Call Tree TotalMS  The total number of milliseconds spent in or under the function.\n");
    DclPrintf("  LocalMS            The total number of milliseconds spent in the function, and unprofiled subfunctions.\n");
    DclPrintf("  User Data Total    The total accumulated user data for all calls.\n");
    DclPrintf("  User Data Max      The maximum user data value for any single call.\n");
    DclPrintf("  User Data Avg      The average user data value for each call.\n");
    DclPrintf("  Max Stack          The maximum stack depth relative to the stack position when the context was entered.\n");

    DclPrintf("Notes:\n");
    DclPrintf("- The profiler uses 4 bytes of stack space for each nested level.\n");

    if(fIgnoreTimings)
        DclPrintf("- The profiler is configured to ignore timing data -- summary timing information is set to zero\n");

    /*  Reset the profiler
    */
    if(fReset)
        Reset(pProf);

    /*  Restore the original state.
    */
    DclOsAtomic32Exchange(&ulAtomicProfilerState, ulOldState);

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: ProfEnable()

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS ProfEnable(void)
{
    D_UINT32    ulOldState;
    
    ulOldState = DclOsAtomic32CompareExchange(&ulAtomicProfilerState, PROF_DISABLED, PROF_ENABLED);

    switch(ulOldState)
    {
        case PROF_DISABLED:
            DclPrintf("Profiler enabled\n");
            return DCLSTAT_SUCCESS;
            
        case PROF_ENABLED:
            DclPrintf("Profiler already enabled\n");
            return DCLSTAT_SUCCESS;
            
        case PROF_BUSY:
            DclPrintf("Profiler is busy or not initialized\n");
            return DCLSTAT_BUSY;

        default:
            DclProductionError();
            return DCLSTAT_FAILURE;
    }
}


/*-------------------------------------------------------------------
    Local: ProfDisable()

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS ProfDisable(void)
{
    D_UINT32    ulOldState;
    
    ulOldState = DclOsAtomic32CompareExchange(&ulAtomicProfilerState, PROF_ENABLED, PROF_DISABLED);

    switch(ulOldState)
    {
        case PROF_ENABLED:
            DclPrintf("Profiler disabled\n");
            return DCLSTAT_SUCCESS;
            
        case PROF_DISABLED:
            DclPrintf("Profiler already disabled\n");
            return DCLSTAT_SUCCESS;
            
        case PROF_BUSY:
            DclPrintf("Profiler is busy or not initialized\n");
            return DCLSTAT_BUSY;

        default:
            DclProductionError();
            return DCLSTAT_FAILURE;
    }
}


/*-------------------------------------------------------------------
    Local: ProfToggle()

    Toggle the enable/disabled state of the profiler.

    Parameters:
        fSummaryReset - TRUE to display the profiler summary and
                        reset the counters.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS ProfToggle(
    D_BOOL      fSummaryReset)
{
    D_UINT32    ulState;
    
    ulState = DclOsAtomic32Retrieve(&ulAtomicProfilerState);

    switch(ulState)
    {
        case PROF_DISABLED:
            /*  The profiler was disabled, so enable it
            */
            return ProfEnable();
            break;
            
        case PROF_ENABLED:
            /*  The profiler is enabled
            */

            if(fSummaryReset)
            {
                /*  Display the output and reset to original state if
                    so directed.
                */
                ProfSummary(TRUE, FALSE, TRUE);
            }

            /*  Finish the toggle operation and disable the profiler
            */
            return ProfDisable();
            break;

        case PROF_BUSY:
            DclPrintf("Profiler is busy or not initialized\n");
            return DCLSTAT_BUSY;
            break;

        default:
            DclProductionError();
            return DCLSTAT_INTERNAL_ERROR;
    }
}


/*-------------------------------------------------------------------
    Local: ProfReset()

    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS ProfReset(void)
{
    D_UINT32    ulOldState;

    if(!pProf)
    {
        DclPrintf("Profiler not initialized\n");
        return DCLSTAT_NOTENABLED;
    }

    ulOldState = DclOsAtomic32Exchange(&ulAtomicProfilerState, PROF_BUSY);
    if(ulOldState == PROF_BUSY)
    {
        /*  The profiler could be busy doing some other report.  In that
            event, ignore the request.
        */            
        DclPrintf("Profiler is busy or uninitialized\n");
        return DCLSTAT_BUSY;
    }

    /*  Reset the profiler
    */
    Reset(pProf);

    /*  Restore the original state.
    */
    DclOsAtomic32Exchange(&ulAtomicProfilerState, ulOldState);

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: GetThreadContext()
-------------------------------------------------------------------*/
static DCLPROFTHREAD *GetThreadContext(
    D_BOOL          fCreate)
{
    DCLTHREADID     idThread = DclOsThreadID();
    DCLPROFTHREAD  *pThread = NULL;
    unsigned        tt;

    /*  A NULL thread ID is invalid
    */
    if(!idThread)
        return NULL;

    /*  If the profiler is not enabled, we cannot enter the profiler code.
        Returning a NULL DCLPROFTHREAD pointer will accomplish this.

        ToDo: This really should set the PROF_BUSY flag briefly while
              getting the thread context.  That would eliminate the
              possibility of other threads putting the profiler into
              "Busy" mode while we're in here.  The use of the system
              mutex below could then be eliminated.
    */
    if(DclOsAtomic32Retrieve(&ulAtomicProfilerState) != PROF_ENABLED)
        return NULL;
  
    DclAssert(pProf);

    /*  Find the thread in the list.  Since the list never shrinks, 
        aside from a full-on profiler reset, this should be safe.
    */
    for(tt=0; tt<pProf->nMaxThreads; tt++)
    {
        if(pProf->pThread[tt].idThread == idThread)
            return &pProf->pThread[tt];
    }

    /*  If this thread is not yet initialized and fCreate is FALSE,
        just exit.
    */
    if(!fCreate)
        return NULL;

    /*  Not in the list, so we must lock the list and add a new entry.

        Generally we don't like using <any> synchronization mechanisms
        inside the profiler code, since they may change the program
        behavior.  However, since this is only needed when a <new>
        thread is encountered, any negative effect should be minimal.
    */
    if(DclOsMutexAcquire(NULL) == DCLSTAT_SUCCESS)
    {
        /*  WARNING!  We are using the "system" mutex rather than a
            standard mutex, since standard mutexes are instrumented
            with profiler stuff.  Therefore, this code <must not>
            invoke anything else which could also try to acquire the
            system mutex.
        */
        for(tt=0; tt<pProf->nMaxThreads; tt++)
        {
            if(pProf->pThread[tt].idThread == 0)
            {
                pThread = &pProf->pThread[tt];
                pThread->idThread = idThread;
                pProf->nThreadCount++;
                break;
            }
        }
        
        DclOsMutexRelease(NULL);    
    }
    else
    {
        DclError();
        return NULL;
    }

    /*  Failed to add the entry?
    */
    if(!pThread)
    {
        pProf->fThreadSlotOverflow = TRUE;
        return NULL;
    }

    return pThread;
}


/*-------------------------------------------------------------------
    Local: PushStack()
-------------------------------------------------------------------*/
static D_BOOL PushStack(
    DCLPROFTHREAD  *pThread,
    const char     *pszLabel,
    DCLRECORDID     idCurrent,
    D_UINT32       *pulFlags,
    D_UINT32        ulUserData)
{
    DCLPROFSTACK   *pStack;

    DclAssert(pThread);
    DclAssert(pszLabel);
    DclAssert(idCurrent);

    /*  On to the next stack entry
    */
    pStack = SetNextStack(pThread);
    if(!pStack)
    {
        /* DclError(); */
        return FALSE;
    }

    /*  Find an existing record that matches this context, or
        allocate a new one (we don't care which).
    */
    pStack->pRecord = AllocateRecord(pThread, pStack, idCurrent, pszLabel, pulFlags);
    if(!pStack->pRecord)
    {
        SetPreviousStack(pThread);
        /* DclError(); */
        return FALSE;
    }

    /*  Record the (approximate) maximum stack space used, relative to where
        the stack was when the top-level record was created.
    */
    if(pStack->pRecord->pTop == pStack->pRecord)
    {
        /*  The top-level records will always have a pTop that points
            to itself, and must always have a stack depth of 0.
        */
        DclAssert(pStack->pRecord->nMaxStackDepth == 0);
    }
    else
    {
        ptrdiff_t   nStackDepth;

        if(pProf->fStackGrowsDown)
            nStackDepth = pStack->pRecord->pTop->pStackPtr - (D_BUFFER*)pulFlags;
        else
            nStackDepth = (D_BUFFER*)pulFlags - pStack->pRecord->pTop->pStackPtr;

/*        DclAssert(nStackDepth); */

        if(nStackDepth < 0)
            nStackDepth *= -1;

        if(pStack->pRecord->nMaxStackDepth < nStackDepth)
            pStack->pRecord->nMaxStackDepth = nStackDepth;

        if(pThread->nMaxStackDepth < nStackDepth)
            pThread->nMaxStackDepth = nStackDepth;
    }

    /*  Update the "user data" fields
    */
    if(pStack->pRecord->ulMaxUserData < ulUserData)
        pStack->pRecord->ulMaxUserData = ulUserData;

    pStack->pRecord->ullTotalUserData += ulUserData;

    /*  If this record was forcibly placed at the top, mark it as
        such so we can display it properly when we do reports.
    */
    if(*pulFlags & DCLPROF_TOPLEVEL)
        pStack->pRecord->uIntFlags |= INTFLAG_FORCETOPLEVEL;

    /*  Set the start time in this stack entry
    */
    pStack->tsStart = DclTimeStamp();

    return TRUE;
 }


/*-------------------------------------------------------------------
    Local: PopStack()
-------------------------------------------------------------------*/
static void PopStack(
    DCLPROFTHREAD  *pThread,
    const char     *pszLabel,
    DCLRECORDID     idCurrent,
    D_UINT32        ulFlags,
    D_UINT32        ulUserData)
{
    DCLPROFSTACK   *pStack;
    D_UINT32        ulElapsedUS;
    D_UINT32        ulSubCallCount;

    DclAssert(pThread);
    DclAssert(pszLabel);
    DclAssert(idCurrent);

    /*  Get the current stack entry
    */
    pStack = pThread->pStackPointer;
    if(!pStack)
    {
        DclError();
        return;
    }

    /*  Get the total elapsed time, in or under this function
    */
    ulElapsedUS = DclTimePassedUS(pStack->tsStart);

    /*  Check for enter/leave mismatch
    */
    if(idCurrent != pStack->pRecord->idCurrent)
    {
        DclPrintf("DCL: Profiler Enter/Leave mismatch! Expected '%s', got '%s'\n",
            pszLabel, pStack->pRecord->pszLabel);
        DclProfDumpCallTree();
        /* DclError(); */
        return;
    }

    /*  Update the record associated with this stack entry
    */
    UpdateRecord(pStack, ulElapsedUS, ulUserData);

    ulSubCallCount = pStack->ulSubCallCount;

    /*  Back to the previous stack entry
    */
    pStack = SetPreviousStack(pThread);
    if(!pStack)
    {
        DclError();
        return;
    }

    pStack->ulSubCallCount += ulSubCallCount;

    /*  For the parent function, record the total time spent in this
        sub-function tree.
    */
    pStack->ullSubTreeTimeUS += ulElapsedUS;
}


/*-------------------------------------------------------------------
    Local: AllocateRecord()

    This function locates an existing record that matches the
    supplied context and returns it, or allocates a new record
    if one does not already exist.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLPROFRECORD *AllocateRecord(
    DCLPROFTHREAD  *pThread,
    DCLPROFSTACK   *pStack,
    DCLRECORDID     idCurrent,
    const char     *pszLabel,
    D_UINT32       *pulFlags)
{
    DCLPROFRECORD  *pTopRec;
    DCLPROFRECORD  *pParentRec;
    DCLPROFRECORD  *pSiblingRec;

    DclAssert(pThread);
    DclAssert(pStack);
    DclAssert(pStack->uLevel > 0);
    DclAssert(pStack->pPrev);

    if(*pulFlags & DCLPROF_TOPLEVEL)
    {
        /*  We want to force this record to be a top-level record,
            probably to reduce the number of contexts, so make it
            a child of the master record.
        */
        pParentRec = pThread->pStackTop->pRecord;

        /*  Mark the record that <should> have been the real parent
            of this record, so that we know we have artificially
            removed some children.
        */
        pStack->pPrev->pRecord->uIntFlags |= INTFLAG_ORPHANEDCHILDREN;
    }
    else
    {
        pParentRec = pStack->pPrev->pRecord;
    }

    DclAssert(pParentRec);

    /*  Initialize pTop to the top level record which is a child of
        the master.  pTop will point to itself for top level records.
    */
    if(pParentRec == pThread->pStackTop->pRecord)
        pTopRec = 0;
    else
        pTopRec = pParentRec->pTop;

    /*  If the parent record we are using has no children yet, no further
        searching is necessary, add a child and return.
    */
    if(!pParentRec->pChild)
    {
        pParentRec->pChild = GetNewRecord(
            pTopRec, pParentRec, pThread->idThread, idCurrent, pszLabel, pulFlags);

        return pParentRec->pChild;
    }

    /*  This parent has children, so search the siblings for
        a matching record.
    */
    pSiblingRec = pParentRec->pChild;
    while(TRUE)
    {
        /*  Found one -- return it and be done.
        */
        if((pSiblingRec->idCurrent == idCurrent) && (pSiblingRec->idThread == pThread->idThread))
        {
            DclAssert(pSiblingRec->pszLabel == pszLabel);
            return pSiblingRec;
        }

        /*  If there are no further siblings, add a new one.
        */
        if(!pSiblingRec->pSibling)
        {
            pSiblingRec->pSibling = GetNewRecord(
                pTopRec, pParentRec, pThread->idThread, idCurrent, pszLabel, pulFlags);

            return pSiblingRec->pSibling;
        }

        /*  On to the next sibling...
        */
        pSiblingRec = pSiblingRec->pSibling;
    }
}


/*-------------------------------------------------------------------
    Local: GetNewRecord()
-------------------------------------------------------------------*/
static DCLPROFRECORD *GetNewRecord(
    DCLPROFRECORD  *pTopRec,
    DCLPROFRECORD  *pParentRec,
    DCLTHREADID     idThread,
    DCLRECORDID     idNew,
    const char     *pszLabel,
    D_UINT32       *pulFlags)
{
    DCLPROFRECORD  *pRec;
    static volatile unsigned nGate = 0;

    nGate++;
    if(nGate == 1)
    {
        if(!pProf->ulFreeRecs)
        {
            nGate--;

            if(!pProf->fRecordsLost)
            {
                DclPrintf("DCL: Profiler out of records\n");
                pProf->fRecordsLost = TRUE;
                /* DclError(); */
            }
            return NULL;
        }

        /*  Adjust pointers and counts...
        */
        pProf->ulFreeRecs--;
        
        pRec = pProf->pNextFreeRecord;

        DclAssert(pRec);
        
        pProf->pNextFreeRecord++;

        nGate--;
    }
    else
    {
        nGate--;
        pProf->fRecordContention = TRUE;
        return NULL;
    }

    if(pTopRec)
    {
        pRec->pTop = pTopRec;
        pRec->pStackPtr = 0;
    }
    else
    {
        pRec->pTop = pRec;
        pRec->pStackPtr = (D_BUFFER*)pulFlags;
    }

    pRec->idThread = idThread;
    pRec->pParent = pParentRec;
    pRec->idCurrent = idNew;
    pRec->pszLabel = pszLabel;

    if(*pulFlags & DCLPROF_CALIBRATE)
        pRec->uIntFlags |= INTFLAG_CALIBRATE;

    return pRec;
}


/*-------------------------------------------------------------------
    Local: UpdateRecord()
-------------------------------------------------------------------*/
static void UpdateRecord(
    DCLPROFSTACK   *pStack,
    D_UINT32        ulElapsedUS,
    D_UINT32        ulUserData)
{
    DCLPROFRECORD  *pRecord;

    DclAssert(pStack);
    DclAssert(pStack->pRecord);

    /*  Find the entry
    */
    pRecord = pStack->pRecord;

    /*  Update the call count
    */
    pRecord->ulCallCount++;

    /*  Update the minimum time
    */
/*    if(pRecord->ullMinTime > ulElapsedUS)
        pRecord->ullMinTime = ulElapsedUS;
*/
    /*  Update the maximum time
    */
    if(pRecord->ullMaxTreeTimeUS < ulElapsedUS)
        pRecord->ullMaxTreeTimeUS = ulElapsedUS;

    /*  Update the total tree time spent in or under this function
    */
    pRecord->ullTotalTreeTimeUS += ulElapsedUS;

    /*  Update the total local time spent only in this function
    */

    DclAssert(ulElapsedUS >= pStack->ullSubTreeTimeUS);
    
    pRecord->ullTotalLocalTimeUS += (ulElapsedUS - pStack->ullSubTreeTimeUS);
    pRecord->ullSubCallCount += pStack->ulSubCallCount;

    /*  Update the "user data" fields
    */
    if(pRecord->ulMaxUserData < ulUserData)
        pRecord->ulMaxUserData = ulUserData;

    pRecord->ullTotalUserData += ulUserData;

    return;
}


/*-------------------------------------------------------------------
    Local: SetNextStack()
-------------------------------------------------------------------*/
static DCLPROFSTACK *SetNextStack(
    DCLPROFTHREAD  *pThread)
{
    /*  Set the next stack entry as the current entry
    */
    if(!pThread->pStackPointer->pNext)
    {
        pThread->fStackOverflow = TRUE;
        return 0;
    }

    pThread->pStackPointer->pRecord->ullLocalCallCount++;
    
    pThread->pStackPointer = pThread->pStackPointer->pNext;

    /*  Initialize this stack entry
    */
    pThread->pStackPointer->ulSubCallCount = 1;
    pThread->pStackPointer->ullSubTreeTimeUS = 0;
    pThread->pStackPointer->pRecord = NULL;
    pThread->pStackPointer->tsStart = 0;

    /*  Bump up the max level we've nested if we're hitting
        new depths...
    */
    if(pThread->nMaxNestLevelHit < pThread->pStackPointer->uLevel)
        pThread->nMaxNestLevelHit = pThread->pStackPointer->uLevel;

    return pThread->pStackPointer;
}


/*-------------------------------------------------------------------
    Local: SetPreviousStack()
-------------------------------------------------------------------*/
static DCLPROFSTACK *SetPreviousStack(
    DCLPROFTHREAD  *pThread)
{
    if(!pThread->pStackPointer->pPrev)
    {
        pThread->fStackUnderflow = TRUE;
        return 0;
    }

    /*  Set the previous stack entry as the current entry
    */
    pThread->pStackPointer = pThread->pStackPointer->pPrev;

    return pThread->pStackPointer;
}


                    /*-----------------------------*\
                     *                             *
                     *      Calibration Stuff      *
                     *                             *
                    \*-----------------------------*/


#define CALIBRATE_LEVEL1_DELAY_NS   (1000000000) /* 1 second delay */
#define CALIBRATE_LEVEL2_DELAY_NS   (10000000)   /* 10000 us delay */
#define CALIBRATE_LEVEL3_DELAY_NS   (250000)     /* 250 us delay */

#define CALIBRATE_LEVEL1_ITERATIONS (10) 
#define CALIBRATE_LEVEL2_ITERATIONS (10) 
#define CALIBRATE_LEVEL3_ITERATIONS (1000) 

#define CALIBRATE_LEVEL3A_NAME      "#CalibrateLevel_3a___25___25"

static void CalibrateLevel_4a(void)
{
    DclProfilerEnter("#CalibrateLevel_4a___0____0", DCLPROF_CALIBRATE, 0);
    DclProfilerLeave(0);
    return;
}


static void CalibrateLevel_4b(void)
{
    DclProfilerEnter("#CalibrateLevel_4b___0____0", DCLPROF_CALIBRATE, 0);
    DclProfilerLeave(0);
    return;
}


static void CalibrateLevel_3a(void)
{
    unsigned    nn; 
    
    DclProfilerEnter(CALIBRATE_LEVEL3A_NAME, DCLPROF_CALIBRATE, 0);

    DclNanosecondDelay(CALIBRATE_LEVEL3_DELAY_NS);  /* 250 us delay */

    for(nn = 0; nn < CALIBRATE_LEVEL3_ITERATIONS; nn++)
    {
        CalibrateLevel_4a();                        /* 1000 * 0 us */
        CalibrateLevel_4b();                        /* 1000 * 0 us */
    }

    DclProfilerLeave(0);
    return;
}

static void CalibrateLevel_3b(void)
{
    unsigned    nn; 
    
    DclProfilerEnter("#CalibrateLevel_3b___25___25", DCLPROF_CALIBRATE, 0);

    DclNanosecondDelay(CALIBRATE_LEVEL3_DELAY_NS);  /* 250 us delay */

    for(nn = 0; nn < CALIBRATE_LEVEL3_ITERATIONS; nn++)
    {
        CalibrateLevel_4a();                        /* 1000 * 0 us */
        CalibrateLevel_4b();                        /* 1000 * 0 us */
    }

    DclProfilerLeave(0);
    return;
}


static void CalibrateLevel_2(void)
{
    unsigned    nn;
    
    DclProfilerEnter("#CalibrateLevel_2____150__100", DCLPROF_CALIBRATE, 0);
    
    DclNanosecondDelay(CALIBRATE_LEVEL2_DELAY_NS);  /* 10000 us delay */

    for(nn = 0; nn < CALIBRATE_LEVEL2_ITERATIONS; nn++)
    {
        CalibrateLevel_3a();                        /* 10 * 250 us */
        CalibrateLevel_3b();                        /* 10 * 250 us */
    }
    
    DclProfilerLeave(0);
    return;
}


static void CalibrateLevel_1(void)
{
    unsigned        nn;
    
    DclProfilerEnter("#CalibrateLevel_1____1150_1000", DCLPROF_CALIBRATE, 0);

    DclNanosecondDelay(CALIBRATE_LEVEL1_DELAY_NS);  /* 1 second delay */

    for(nn = 0; nn < CALIBRATE_LEVEL1_ITERATIONS; nn++)
        CalibrateLevel_2();                         /* 10 * (10000 us + (100 * 250 us)) */

    DclProfilerLeave(0);
    return;
}


/*-------------------------------------------------------------------
    Local: CalculateOverhead()

    Parameters:
        None.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void CalculateOverhead(void)
{
    DCLPROFRECORD  *pRecord;
    D_UINT32        ulOldState;

    /*  No need to calibrate again if it's already been done...
    */
    if(pProf->ulOverheadTopLevNS)
        return;

    if(DclNanosecondDelayMinimum() == CALIBRATE_LEVEL3_DELAY_NS)
    {
        DclPrintf("The high-res timer does not exist, or does not provide sufficient\n");
        DclPrintf("resolution to support the overhead calibration -- ignoring.\n");
        return;
    }

    /*  If the profiler is currently disabled, temporarily enable it so we  
        can do our overhead calculation.
    */        
    ulOldState = DclOsAtomic32CompareExchange(&ulAtomicProfilerState, PROF_DISABLED, PROF_ENABLED);
    if(ulOldState != PROF_DISABLED && ulOldState != PROF_ENABLED)
    {
        DclPrintf("Unable to calibrate the profiler overhead\n");
        return;
    }

    DclPrintf("Profiler calibrating...\n");

    CalibrateLevel_1();

    /*  Set the profiler to a BUSY state, and leave it there.  The parent
        function will ultimately set things to the desired state when it
        is finished with the report.
    */        
    DclOsAtomic32Exchange(&ulAtomicProfilerState, PROF_BUSY);

    pRecord = pProf->pMasterRecord;
    while(pRecord && pRecord->idCurrent)
    {
        if((pRecord->uIntFlags & INTFLAG_CALIBRATE) && (DclStrCmp(pRecord->pszLabel, CALIBRATE_LEVEL3A_NAME) == 0))
        {
            D_UINT32 ulLevel3DelayUS = pRecord->ulCallCount * (CALIBRATE_LEVEL3_DELAY_NS / 1000);
                
             pProf->ulOverheadTopLevNS = (D_UINT32)DclMulDiv64(
                pRecord->ullTotalLocalTimeUS - ulLevel3DelayUS,
                1000, pRecord->ullSubCallCount); 

            pProf->ulOverheadSubLevNS = (D_UINT32)DclMulDiv64(
                pRecord->ullTotalTreeTimeUS - ulLevel3DelayUS, 
                1000, pRecord->ullSubCallCount - pRecord->ulCallCount); 
            
            return;
        }
        pRecord++;
    }

    DclPrintf("Profiler self-calibration failed -- ignoring...\n");

    return;
}


/*-------------------------------------------------------------------
    Local: DisplayFunctionReport()

    Display the records alphabetically by function name, summing
    the statistics for all records that have the same ID.

    Brute force sorting since the records are relatively limited,
    time is not critical, and we really don't want to have to mess
    with our cross-linked pointers necessary for the context
    display.

    Parameters:
        None.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void DisplayFunctionReport(
    DCLPROFINFO    *pPI,
    D_BOOL          fShort,
    D_BOOL          fAdjustOverhead)
{
    DCLPROFRECORD  *pRecord;
    DCLPROFRECORD  *pBest;

    if(!fAdjustOverhead || !pPI->ulOverheadSubLevNS )
    {
        /*  Display this message if overhead adjustment is disabled at the
            preprocessor level, or if the overhead could not be calculated
            for some reason (probably lack of timer resolution).
        */            
        DclPrintf("WARNING: Profiler overhead adjustment is disabled or non-functional\n");
    }

    /*  Clear our sorted/processed flag from all entries
    */
    pRecord = pProf->pMasterRecord;
    while(pRecord && pRecord->idCurrent)
    {
        pRecord->uIntFlags &= ~INTFLAG_PROCESSED;
        pRecord++;
    }

    DclPrintf("---------------------------------------------------[ Profiler Function Report ]--------------------------------------------------\n");
    DclPrintf("  Call   [-------Call Tree------]                                                                   [--User Data Counts---]  Max \n");
    DclPrintf("  Count   AvgUS    MaxUS  TotalMS LocalMS             Name[thread]-contexts                         Total  Average  Maximum Stack\n");
    DclPrintf("---------------------------------------------------------------------------------------------------------------------------------\n");

    /*  Start at the top -- assume no record can be found
    */
    pRecord = pProf->pMasterRecord;
    pBest = 0;
    while(pRecord->idCurrent)
    {
        /*  If this is not a record we've already processed...
        */
        if(!(pRecord->uIntFlags & INTFLAG_PROCESSED))
        {
            if(pBest == 0)
            {
                pBest = pRecord;
            }
            else
            {
                /*  See if this record is alphabetically less than the
                    current "best" record.  Note that we are not resetting
                    pBest in the event of equality -- pBest must be the first
                    of any equal records.
                */
                if(DclStrICmp(pRecord->pszLabel, pBest->pszLabel) < 0)
                    pBest = pRecord;
            }
        }

        /*  On to the next record...
        */
        pRecord++;

        /*  Gone through all the records?
        */
        if(pRecord->idCurrent == 0)
        {
            /*  If so and we didn't find a best, we're done, otherwise
                display the record (and any equivalents there might be).
            */
            if(!pBest)
                break;

            DisplayMatchingRecords(pPI, pBest, fShort, fAdjustOverhead);

            /*  Force everything to start over from the top.
            */
            pRecord = pProf->pMasterRecord;
            pBest = 0;
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Local: DisplayMatchingRecords()

    Display a record's statistics, summed with the stats for all
    records with the same ID.  All processed records are marked
    as such.  Note that the pRecord parameter must be the first
    of any equivalent records (same ID) -- the list is only
    searched from the current position to the end.

    Parameters:
        None.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void DisplayMatchingRecords(
    DCLPROFINFO    *pPI,
    DCLPROFRECORD  *pRecord,
    D_BOOL          fShort,
    D_BOOL          fAdjustOverhead)
{
    char            achBuffer[MAXFUNCNAMELEN];
    DCLPROFRECORD   rec;
    unsigned        nContexts = 1;

    /*  Make a local copy of the record data so we can modify
        it by adding data from equivalent records (same ID).
    */
    rec = *pRecord;

  #if DUMP_RECORDS
    DumpRecord(pRecord);
  #endif

    /*  Add the statistics for all the remaining records that
        have the same ID.
    */
    nContexts += AddContexts(pRecord+1, &rec);

    DisplayRecord(pPI, &rec, 0, achBuffer, nContexts, fShort, fAdjustOverhead);

    /*  Mark this record as processed
    */
    pRecord->uIntFlags |= INTFLAG_PROCESSED;

    return;
}


/*-------------------------------------------------------------------
    Local: DisplayCallTreeReport()

    Parameters:
        None.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void DisplayCallTreeReport(
    DCLPROFINFO    *pPI,
    D_BOOL          fShort,
    D_BOOL          fAdjustOverhead)
{
    char            achBuffer[MAXFUNCNAMELEN+DCLPROF_MAX_NESTING_LEVEL+1];

    /*  These fields must be reinitialized for each CallTree Report.
        They are not used in the Function Report.
    */        
    pPI->ulDisplayLastCallCount = 0;
    pPI->uDisplayLastLevel = 0;

    DclPrintf("---------------------------------------------------[ Profiler CallTree Report ]--------------------------------------------------\n");
    DclPrintf("  Call   [-------Call Tree------]                                                                   [--User Data Counts---]  Max \n");
    DclPrintf("  Count   AvgUS    MaxUS  TotalMS LocalMS                  Name[thread]                             Total  Average  Maximum Stack\n");
    DclPrintf("---------------------------------------------------------------------------------------------------------------------------------\n");

    /*  Start with the child of the master record since we have no need
        to display the master record in this report.
    */
    if(pPI->pMasterRecord->pChild)
        ProcessNode(pPI, pPI->pMasterRecord->pChild, 1, &achBuffer[0], fShort, fAdjustOverhead);
}


/*-------------------------------------------------------------------
    Local: ProcessNode()

    Process all records at the given nesting level with the matching
    Head and Parent ID, recursing until complete.

    Parameters:

    Return Value:
        None.
-------------------------------------------------------------------*/
static void ProcessNode(
    DCLPROFINFO    *pPI,
    DCLPROFRECORD  *pNode,
    D_UINT16        uLevel,
    char           *pBuffer,
    D_BOOL          fShort,
    D_BOOL          fAdjustOverhead)
{
    DisplayRecord(pPI, pNode, uLevel, pBuffer, 0, fShort, fAdjustOverhead);

    /*  Process any child nodes...
    */
    if(pNode->pChild)
        ProcessNode(pPI, pNode->pChild, uLevel + 1, pBuffer, fShort, fAdjustOverhead);

    /*  Lastly process any siblings...
    */
    if(pNode->pSibling)
        ProcessNode(pPI, pNode->pSibling, uLevel, pBuffer, fShort, fAdjustOverhead);

    return;
}


/*-------------------------------------------------------------------
    Local: DisplayRecord()

    Parameters:

    Return Value:
        None.
-------------------------------------------------------------------*/
static void DisplayRecord(
    DCLPROFINFO    *pPI,
    DCLPROFRECORD  *pRec,
    D_UINT16        uLevel,
    char           *pBuffer,
    unsigned        nContexts,
    D_BOOL          fShort,
    D_BOOL          fAdjustOverhead)
{
    D_UINT64        ullAverageUS;
    D_UINT64        ullAvgUserData;
    D_UINT64        ullMaxTreeTimeUS = pRec->ullMaxTreeTimeUS;
    D_UINT64        ullTotalTime = pRec->ullTotalTreeTimeUS;
    D_UINT64        ullLocalTime = pRec->ullTotalLocalTimeUS;
    unsigned        nThreadNum;
    char           *pStar = "*";
    char           *pBang = "!";
    char           *pNULL = "";

    nThreadNum = GetThreadNum(pRec->idThread);

    if(fAdjustOverhead && pPI->ulOverheadSubLevNS)
    {
        D_UINT64    ullTmp;

        DclAssert(pRec->ulCallCount || pRec->ullLocalCallCount);

        if(pRec->ulCallCount + pRec->ullLocalCallCount)
        {
            ullTmp = DclMulDiv64(pRec->ulCallCount + pRec->ullLocalCallCount, 
                                 pPI->ulOverheadTopLevNS, 1000);

            if(ullLocalTime > ullTmp)
                ullLocalTime -= ullTmp;
            else
                ullLocalTime = 0;

            if(ullTotalTime > ullTmp)
                ullTotalTime -= ullTmp;
            else
                ullTotalTime = 0;

            if(ullTotalTime < ullLocalTime)
                ullTotalTime = ullLocalTime;
                
        }

        DclAssert(pRec->ullSubCallCount >= pRec->ulCallCount);
        DclAssert(pPI->ulOverheadSubLevNS >= pPI->ulOverheadTopLevNS);

        ullTmp = DclMulDiv64(pRec->ullSubCallCount - pRec->ulCallCount, pPI->ulOverheadSubLevNS, 1000);
       
        DclAssert(ullTmp != D_UINT64_MAX);

        if(ullTotalTime > ullTmp)
            ullTotalTime -= ullTmp;
        else
            ullTotalTime = 0;

        if(ullTotalTime < ullLocalTime)
            ullTotalTime = ullLocalTime;

        if(pRec->ulCallCount)
        {
            DclAssert(pRec->ullTotalTreeTimeUS >= ullTotalTime);

            ullTmp = (pRec->ullTotalTreeTimeUS - ullTotalTime) / pRec->ulCallCount;

            if(ullMaxTreeTimeUS > ullTmp)
                ullMaxTreeTimeUS -= ullTmp;
            else
                ullMaxTreeTimeUS = 0;
        }
        
      #if 0 
        /*  Enable this code to assist in debugging calibration problems.
        */
        if(pRec->uIntFlags & INTFLAG_CALIBRATE)
        {
            DclPrintf("Calibration: Calls=%lU SubCalls=%llU LocCalls=%llU Tot=%llU Loc=%llU Max=%llU NewTot=%llU NewLoc=%llU NewMax=%llU\n",
                pRec->ulCallCount, VA64BUG(pRec->ullSubCallCount), VA64BUG(pRec->ullLocalCallCount),
                VA64BUG(pRec->ullTotalTreeTimeUS), VA64BUG(pRec->ullTotalLocalTimeUS), VA64BUG(pRec->ullMaxTreeTimeUS),
                VA64BUG(ullTotalTime), VA64BUG(ullLocalTime), VA64BUG(ullMaxTreeTimeUS));
        }
      #endif       
    }
  
    /*  Calculate average call time, rounding if needed.
    */
    ullAverageUS = ullTotalTime;
    if(pRec->ulCallCount)
    {
        ullAverageUS += (pRec->ulCallCount >> 1);
        DclUint64DivUint32(&ullAverageUS, pRec->ulCallCount);
    }

    /*  Calculate average user data volume, rounding if needed.
    */
    ullAvgUserData = pRec->ullTotalUserData;
    if(pRec->ulCallCount)
    {
        ullAvgUserData += (pRec->ulCallCount >> 1);
        DclUint64DivUint32(&ullAvgUserData, pRec->ulCallCount);
    }

    /*  Check flags as necessary
    */
    if(!(pRec->uIntFlags & INTFLAG_ORPHANEDCHILDREN))
        pBang = pNULL;

    if(!(pRec->uIntFlags & INTFLAG_FORCETOPLEVEL))
        pStar = pNULL;

    /*  Any record with a '#' as the first character is an internal profiler
        record, so if we are in the context window (uLevel != 0), skip it.
    */
    if((*pRec->pszLabel == '#') && uLevel)
    {
        pPI->nRecordsSkipped++;

        /*  Return early if we are doing shortened output
        */
        if(fShort)
            return;
    }

    if(nContexts)
    {
        /*  If a context count is specified, level will always be
            zero -- so assert it.
        */
        DclAssert(uLevel == 0);

        DclSNPrintf(pBuffer, MAXFUNCNAMELEN, "%s%s%s[%c]-%u",
            pStar, pRec->pszLabel, pBang, nThreadNum+'A'-1, nContexts);
    }
    else
    {
        unsigned    ii;
        int         iLen;

        /*  If a context count is NOT specified, level will always be
            non-zero -- so assert it.
        */
        DclAssert(uLevel != 0);

        /*  Note that level '1' means no indent, level '2' means
            indent one place, and so on.
        */
        for(ii=0; ii<(D_UINT16)(uLevel+1); ii+=2)
        {
            pBuffer[ii] = '.';
            pBuffer[ii+1] = ' ';
        }

        iLen = DclSNPrintf(pBuffer+(uLevel-1), MAXFUNCNAMELEN-uLevel, "%s%s%s",
            pStar, pRec->pszLabel, pBang);

        /*  If there is still room in the buffer, and this is a top-level
            record, add a dividing line to make this easier to find in
            the output.
        */
        if((iLen != -1) && (uLevel == 1))
        {
            pBuffer[iLen+0] = '[';
            pBuffer[iLen+1] = (char)(nThreadNum+'A'-1);
            pBuffer[iLen+2] = ']';
            pBuffer[iLen+3] = '<';
            DclMemSet(pBuffer+iLen+4, '=', (MAXFUNCNAMELEN-iLen)-4);
            pBuffer[MAXFUNCNAMELEN-4] = 'T';
            pBuffer[MAXFUNCNAMELEN-3] = 'O';
            pBuffer[MAXFUNCNAMELEN-2] = 'P';
            pBuffer[MAXFUNCNAMELEN-1] = 0;
        }
    }

    if(fIgnoreTimings)
    {
        ullAverageUS = 0;
        ullMaxTreeTimeUS = 0;
        ullTotalTime = 0;
        ullLocalTime = 0;
    }
    else
    {
        /*  Round and convert times from US to MS
        */
        ullTotalTime += 500;
        DclUint64DivUint32(&ullTotalTime, 1000);
        ullLocalTime += 500;
        DclUint64DivUint32(&ullLocalTime, 1000);
    }

    /*  If allowed to abbreviate the output (context display only), skip
        any records which:
        1) Are at a level greater than or equal the the level of the
           previous record.
        2) Have a call count which is equal to or less than that of the
           previous record.
        3) Have timings which are all zeros.
    */
    if( (uLevel > 1) &&
        (uLevel >= pPI->uDisplayLastLevel) &&
        (pRec->ulCallCount <= pPI->ulDisplayLastCallCount) &&
        !ullAverageUS &&
        !ullMaxTreeTimeUS &&
        !ullTotalTime &&
        !ullLocalTime )
    {
        pPI->nRecordsSkipped++;

        if(fShort)
        {
            return;
        }
        else
        {
            /*  Place a special mark in records which <would> have been
                skipped, had the Abbreviate feature been turned on.
            */
            *pBuffer = '~';
        }
    }

    pPI->uDisplayLastLevel = uLevel;
    pPI->ulDisplayLastCallCount = pRec->ulCallCount;

    DclPrintf("%7lU %7llU %8llU %8llU %7llU %-*s %10llX %8llX %8lX %5x\n",
            pRec->ulCallCount,
            VA64BUG(ullAverageUS),
            VA64BUG(ullMaxTreeTimeUS),
            VA64BUG(ullTotalTime),
            VA64BUG(ullLocalTime),
            MAXFUNCNAMELEN+2,
            pBuffer,
            pRec->ullTotalUserData,
            ullAvgUserData,
            pRec->ulMaxUserData,
            pRec->nMaxStackDepth);

    return;
}


/*-------------------------------------------------------------------
    Local: AddContexts()

    Add the statistics for all the records that have the same
    idCurrent and idThread values, irrespective of other context
    information.  Mark them as sorted.

    Start with the specified record, rather than process the
    entire list.

    Parameters:

    Return Value:
        Return the number of records that matched.
-------------------------------------------------------------------*/
static unsigned AddContexts(
    DCLPROFRECORD      *pStartRec,
    DCLPROFRECORD      *pSummaryRec)
{
    unsigned            nContexts = 0;

    DclAssert(pStartRec);
    DclAssert(pSummaryRec);

    while(pStartRec->idCurrent)
    {
        /*  If the ID matches, and we have not already processed this
            record, add the statistics together.
        */
        if((pStartRec->idCurrent == pSummaryRec->idCurrent) &&
           (pStartRec->idThread == pSummaryRec->idThread) &&
            !(pStartRec->uIntFlags & INTFLAG_PROCESSED))
        {
          #if DUMP_RECORDS
            DumpRecord(pStartRec);
          #endif

            pSummaryRec->ullSubCallCount += pStartRec->ullSubCallCount;

            pSummaryRec->ulCallCount += pStartRec->ulCallCount;
            pSummaryRec->ullTotalTreeTimeUS += pStartRec->ullTotalTreeTimeUS;
            pSummaryRec->ullTotalLocalTimeUS += pStartRec->ullTotalLocalTimeUS;

            if(pSummaryRec->ullMaxTreeTimeUS < pStartRec->ullMaxTreeTimeUS)
                pSummaryRec->ullMaxTreeTimeUS = pStartRec->ullMaxTreeTimeUS;

            pSummaryRec->ullTotalUserData += pStartRec->ullTotalUserData;

            if(pSummaryRec->ulMaxUserData < pStartRec->ulMaxUserData)
                pSummaryRec->ulMaxUserData = pStartRec->ulMaxUserData;

            if(pSummaryRec->nMaxStackDepth < pStartRec->nMaxStackDepth)
                pSummaryRec->nMaxStackDepth = pStartRec->nMaxStackDepth;

            nContexts++;

            /*  Mark this record as processed
            */
            pStartRec->uIntFlags |= INTFLAG_PROCESSED;
        }

        pStartRec++;
    }

    return nContexts;
}


/*-------------------------------------------------------------------
    Local: Reset()
-------------------------------------------------------------------*/
static void Reset(
    DCLPROFINFO    *pPI)
{
    unsigned        kk;

    DclAssert(pPI);

    /*  Reset the error conditions and calibration
    */
    pPI->fRecordsLost = FALSE;
    pPI->fRecordContention = FALSE;
    pPI->fThreadSlotOverflow = FALSE;
    pPI->ulOverheadTopLevNS = 0;
    
    /*  Build the record list
    */
    DclMemSet(pPI->pMasterRecord, 0, sizeof(DCLPROFRECORD_ALIGNED) * pPI->nMaxRecords);

    /*  The very first record is always a master record, used simply to
        make the search algorithms cleaner.
    */
    pPI->pMasterRecord->pszLabel = "#ProfilerMasterRecord";
    pPI->pMasterRecord->idCurrent = DCLPROF_MASTERID;
    pPI->pNextFreeRecord = pPI->pMasterRecord + 1;
    pPI->ulFreeRecs = pPI->nMaxRecords - 1;
    pPI->nThreadCount = 0;

    for(kk=0; kk<pPI->nMaxThreads; kk++)
    {
        ResetThread(pPI, &pPI->pThread[kk]);
    }
}


/*-------------------------------------------------------------------
    Local: ResetThread()
-------------------------------------------------------------------*/
static void ResetThread(
    DCLPROFINFO    *pPI,
    DCLPROFTHREAD  *pThread)
{
    DCLPROFSTACK   *pStackCur;
    DCLPROFSTACK   *pStackPrev;
    unsigned        nIndex;

    DclAssert(pThread);
    DclAssert(pThread->pStackTop);
    DclAssert(pThread->nMaxNestLevels);

    DclMemSet(pThread->pStackTop, 0, sizeof(DCLPROFSTACK_ALIGNED) * pThread->nMaxNestLevels);

    pThread->idThread = 0;
    pThread->nThreadBusyCount = 0;
    pThread->nMaxNestLevelHit = 0;
    pThread->fStackOverflow = FALSE;
    pThread->fStackUnderflow = FALSE;

    pStackPrev = (void *)0;
    pStackCur = pThread->pStackTop;
    for(nIndex = 0; nIndex < pThread->nMaxNestLevels; nIndex++)
    {
        pStackCur->uLevel = (D_UINT16)nIndex;
        pStackCur->pPrev = pStackPrev;
        pStackCur->pNext = DclPtrAddByte(pStackCur, sizeof(DCLPROFSTACK_ALIGNED));
        pStackPrev = pStackCur;
        pStackCur = pStackCur->pNext;
    }
    pStackPrev->pNext = (void *)0;

    pThread->pStackTop->pRecord = pPI->pMasterRecord;
    pThread->pStackPointer = pThread->pStackTop;

    pThread->pStackTop->tsStart = DclTimeStamp();

    return;
}


/*-------------------------------------------------------------------
    Local: GetThreadNum()

    Return Value:
    Returns the thread number (relative to 1) for the matching
    thread ID, or zero if not found.
-------------------------------------------------------------------*/
static unsigned GetThreadNum(
    DCLTHREADID         idThread)
{
    unsigned            kk;

/*    DclAssert(idThread); */

    for(kk=0; kk<pProf->nMaxThreads; kk++)
    {
        if(pProf->pThread[kk].idThread == idThread)
            return kk+1;
    }

    return 0;
}


/*-------------------------------------------------------------------
    Local: CheckStackDirection()

    Return Value:
        Returns TRUE if the stack grows down, and FALSE if it grows
        up.
-------------------------------------------------------------------*/
static D_BOOL CheckStackDirection(
    DCLRECORDID    *pID)
{
    DCLRECORDID     idTemp;

    if(&idTemp < pID)
        return TRUE;
    else
        return FALSE;
}


#if DUMP_RECORDS
/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void DumpRecord(
    DCLPROFRECORD  *pRecord)
{
    DclPrintf("Rec=%P TID=%lX Top=%P Par=%P ID=%lX Lev=%U Calls=%5lU Sub=%6llU Loc=%6llU Stack=%4u mUS=%7llU tUS=%7llU lUS=%7llU\n",
        pRecord, pRecord->idThread, pRecord->pTop, pRecord->pParent,
        pRecord->idCurrent, pRecord->uLevel, 
        pRecord->ulCallCount, 
        VA64BUG(pRecord->ullSubCallCount), 
        VA64BUG(pRecord->ullLocalCallCount),
        pRecord->nMaxStackDepth,
        VA64BUG(pRecord->ullMaxTreeTimeUS),
        VA64BUG(pRecord->ullTotalTreeTimeUS),
        VA64BUG(pRecord->ullTotalLocalTimeUS));

    return;
}
#endif

 
#endif  /* DCLCONF_PROFILERENABLED */

