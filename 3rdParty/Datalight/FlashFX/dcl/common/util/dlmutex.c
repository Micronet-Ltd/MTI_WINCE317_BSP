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

    This module contains the DCL mutex abstraction.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlmutex.c $
    Revision 1.30  2010/09/19 01:09:52Z  garyp
    Commented out an assert to allow some test code to run.
    Revision 1.29  2010/09/06 21:55:56Z  garyp
    Removed some debug code to allow parameter checking tests to pass.
    Revision 1.28  2010/07/31 19:47:14Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.27  2010/02/23 20:30:08Z  garyp
    Corrected some profiler instrumentation -- no functional changes.
    Revision 1.26  2009/12/11 20:28:29Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.25  2009/11/07 23:09:51Z  garyp
    Added profiler instrumentation.  Updated debugging messages.  No
    functional changes.
    Revision 1.24  2009/11/04 18:50:48Z  garyp
    Updated to track statistics in microseconds rather than milliseconds.
    Revision 1.23  2009/10/27 14:13:28Z  garyp
    Debug messages updated -- no functional changes.
    Revision 1.22  2009/06/28 00:57:14Z  garyp
    Updated so the mutex owner thread ID is tracked all the time, regardless
    whether the mutex tracking feature is enabled or not.  Updated the usage
    count field to make examination of the owner field more accurate.  Updated
    to allow recursively acquired mutexes.  Updated to handle mutexes which
    can time-out.
    Revision 1.21  2009/06/12 02:03:16Z  garyp
    Updated debug messages -- no functional changes.
    Revision 1.20  2009/04/10 02:00:53Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.19  2008/06/03 21:43:08Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.18  2007/12/18 03:57:20Z  brandont
    Updated function headers.
    Revision 1.17  2007/11/03 05:02:28Z  pauli
    Added assertion that the thread releasing the mutex owns it.
    Revision 1.16  2007/10/31 17:49:02Z  pauli
    Separated multiple conditions of an assertion into multiple assertions
    to make it clear which one failed.  Removed tabs.
    Revision 1.15  2007/04/24 20:19:00Z  pauli
    Updated comments related to Bug 1017.
    Revision 1.14  2007/04/01 23:39:46Z  Garyp
    Removed the DCLCONF_MUTEXTRACKING symbol initialization since it
    is now handled in dclconf.h.
    Revision 1.13  2007/03/30 18:25:40Z  Garyp
    Added support for tracking mutex statistics.  Eliminated the no longer used
    function DclMutexRecreate().
    Revision 1.12  2007/03/20 21:37:59Z  Garyp
    Tweaked a TRACE debug level.
    Revision 1.11  2006/10/22 00:43:12Z  Garyp
    Avoid the mutex recursion check when the profiler is enabled.
    Revision 1.10  2006/10/19 01:05:36Z  joshuab
    Move decrement of open count in DclMutexRelease to be protected by the
    mutex.
    Revision 1.9  2006/10/18 04:27:32Z  Garyp
    Simplified preprocessor logic now that DclOsThreadID() is a required
    function.
    Revision 1.8  2006/10/17 23:33:49Z  joshuab
    Sandbox confusion:  I checked in the wrong version of this file.
    idOwner must be before ulOsMutex.
    Revision 1.7  2006/10/17 22:26:40Z  joshuab
    Added protection to assert on attempted recursive acquires of mutexes.
    Recursive acquires are invalid on Nucleus (at least), and so the generic
    DCL mutex abstraction now enforces that across all OSes.
    Revision 1.6  2006/10/04 02:40:05Z  Garyp
    Updated to use DclMemAllocZero().
    Revision 1.5  2006/08/21 23:12:35Z  Garyp
    Documentation update.
    Revision 1.4  2006/05/14 05:22:00Z  Keithg
    Added explicit type cast to placate MSVC7
    Revision 1.3  2006/02/23 21:10:57Z  Garyp
    Updated some debug levels.
    Revision 1.2  2006/02/23 02:59:00Z  Garyp
    Tweaked some debug settings.
    Revision 1.1  2005/10/14 01:47:20Z  Pauli
    Initial revision
    Revision 1.3  2005/09/28 01:45:51Z  Garyp
    Tweaked the structure name.
    Revision 1.2  2005/08/03 19:17:44Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/06 02:59:08Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiprv.h>
#include <dlstats.h>

struct DCLMUTEX
{
  #if DCLCONF_MUTEXTRACKING
    struct DCLMUTEX *pNext;     /* Next in the linked list                      */
    D_UINT64    ullTotalWaitUS; /* Total US spent waiting to acquire this mutex */
    D_UINT32    ulMaxWaitUS;    /* Max US spent waiting to acquire this mutex   */
    D_UINT32    ulAcquireCount; /* Total times acquired                         */
  #endif
    char        szName[DCL_PAD(DCL_MUTEXNAMELEN, char)];
    unsigned    nCurrentCount;  /* Current times acquired                       */
    DCLTHREADID idOwner;        /* The current owner of the object              */
    D_UINT32    ulOsMutex;      /* The start of the OS mutex object             */
                                /* This must be the last item in the struct     */
};

/*-------------------------------------------------------------------
    NOTES:
    - The idOwner field is DCL_INVALID_THREADID for newly created
      mutexes which have never been acquired.  Once a mutex has been
      acquired the first time, the idOwner field will be the current
      or previous owner of the mutex.  It can only be assumed to be
      the current owner if the nCurrentCount value is >0.  If that
      value is zero, the idOwner value cannot safely be determined.
    - The nCurrentCount value will always be 0 or 1 so long as
      recursive mutex acquisitions are not allowed.  This is always
      incremented after the mutex is acquired, and decremented before
      the mutex is released.  It is always incremented after the
      idOwner value is set, so IFF this value is 1, then the idOwner
      field accurately has the current mutex owner.
-------------------------------------------------------------------*/


#if DCLCONF_MUTEXTRACKING
  static PDCLOSMUTEX    pMutexChainProtect = NULL;
  static PDCLMUTEX      pMutexChainHead = NULL;
#endif


/*-------------------------------------------------------------------
    Public: DclMutexAcquire()

    Acquire a mutex object created with DclMutexCreate().

    *Note* -- In some environments, mutex objects can be acquired
    multiple times by the same thread.  However this is not the
    case in all environments.  Therefore you must not rely on this
    characteristic.

    Parameters:
        pMutex   - a pointer to the mutex object.

    Return Value:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
D_BOOL DclMutexAcquire(
    PDCLMUTEX       pMutex)
{
    DCLSTATUS       dclStat;
    DCLTHREADID     tid = DclOsThreadID();
    D_BOOL          fSuccess = TRUE;
  #if DCLCONF_MUTEXTRACKING
    D_UINT32        ulElapsedUS;
    DCLTIMESTAMP    ts = DclTimeStamp();
  #endif

    DclAssert(pMutex);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_MUTEX, 2, 0),
        "Acquiring mutex '%s' Count=%u Current/PreviousOwner=%lX\n",
        pMutex->szName, pMutex->nCurrentCount, pMutex->idOwner));

    DclProfilerEnter("DclMutexAcquire", 0, 0);

    DclAssert(tid != DCL_INVALID_THREADID);

  #if D_DEBUG >= 2
    if(pMutex->nCurrentCount && (pMutex->idOwner == tid))
    {
        DclPrintf("Thread %lX is recursively acquiring mutex '%s'\n", tid, pMutex->szName);
    }
  #endif

    dclStat = DclOsMutexAcquire((PDCLOSMUTEX)&pMutex->ulOsMutex);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(dclStat == DCLSTAT_MUTEX_TIMEOUT)
        {
            /*  NOTE: Mutexes are *NOT* allowed to time-out.  This code
                      only exists for special case debugging purposes 
                      used when diagnosing multithreading issues.  This
                      is a DclPrintf() rather than a DCLPRINTF() because
                      sometimes this diagnosis needs to happen when using
                      a RELEASE build of the product.
            */                  
            DclPrintf("Thread %lX timed out waiting on mutex '%s', Owner=%lX\n",
                tid, pMutex->szName, pMutex->idOwner);
        }

        DCLPRINTF(1, ("Unable to acquire mutex '%s', Status=%lX\n", pMutex->szName, dclStat));

        DclError();

        fSuccess = FALSE;
    }
    else
    {
        /*  Record the thread ID of the new owner.  Do this at the earliest
            possible convenience <after> acquiring the mutex, and before
            incrementing nCurrentCount.
        */
        pMutex->idOwner = tid;
        pMutex->nCurrentCount++;

      #if DCLCONF_MUTEXTRACKING
        ulElapsedUS = DclTimePassedUS(ts);
        pMutex->ullTotalWaitUS += ulElapsedUS;
        if(pMutex->ulMaxWaitUS < ulElapsedUS)
            pMutex->ulMaxWaitUS = ulElapsedUS;
        pMutex->ulAcquireCount++;
      #endif
    }
    
    DclProfilerLeave(0);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_MUTEX, 3, 0),
        "Acquiring mutex '%s' Count=%u returning %u\n", pMutex->szName, pMutex->nCurrentCount, fSuccess));

    return fSuccess;
}


/*-------------------------------------------------------------------
    Public: DclMutexCreate()

    Create a mutex object.

    Parameters:
        pszName  - A pointer to the null terminated mutex name.

    Return Value:
        Returns a pointer to the mutex object or NULL if failure.
-------------------------------------------------------------------*/
PDCLMUTEX DclMutexCreate(
    const char     *pszName)
{
    D_UINT16        uMutexObjLen;
    PDCLMUTEX       pMutex;
    DCLSTATUS       dclStat;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_MUTEX, 1, TRACEINDENT),
        "Creating mutex '%s'\n", pszName));

    DclAssert(pszName);
    DclAssert(DclStrLen(pszName) < DCL_MUTEXNAMELEN);

  #if DCLCONF_MUTEXTRACKING
    /*  Use an OS level mutex to protect the mutex chain.  This prevents a
        recursion problem since we can't use a high-level mutex to protect
        the high-level mutex chain.  Consequently, this special OS-level
        mutex will not be included in the mutex statistics.
    */
    if(!pMutexChainProtect)
    {
        pMutexChainProtect = DclMemAllocZero(DclOsMutexInfo(NULL));
        if(pMutexChainProtect)
        {
            dclStat = DclOsMutexCreate(pMutexChainProtect, D_PRODUCTPREFIX"MUTTR");
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclMemFree(pMutexChainProtect);
                pMutexChainProtect = NULL;
            }
        }
    }
    if(!pMutexChainProtect)
        DclPrintf("Error creating mutex chain protector!\n");
  #endif

    /*  Allocate memory for our mutex object, plus enough for the
        actual OS level mutex information.
    */
    uMutexObjLen = sizeof *pMutex - sizeof(pMutex->ulOsMutex) + (D_UINT16) DclOsMutexInfo(NULL);

    pMutex = DclMemAllocZero(uMutexObjLen);
    if(!pMutex)
    {
        DclError();

        return NULL;
    }

    /*  We do this stuff prior to actually creating the
        mutex since theoretically another thread could use
        it before we get a chance to do this.
    */
    DclStrCpy(pMutex->szName, pszName);

    pMutex->idOwner = DCL_INVALID_THREADID;

    dclStat = DclOsMutexCreate((PDCLOSMUTEX)&pMutex->ulOsMutex, pszName);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DCLTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, TRACEUNDENT),
            "Unable to create mutex, status=%lX\n", dclStat));

        DclError();

        DclMemFree(pMutex);

        return NULL;
     }

  #if DCLCONF_MUTEXTRACKING
    if(pMutexChainProtect && (DclOsMutexAcquire(pMutexChainProtect) == DCLSTAT_SUCCESS))
    {
        pMutex->pNext = pMutexChainHead;

        pMutexChainHead = pMutex;

        DclOsMutexRelease(pMutexChainProtect);
    }
  #endif

    DCLTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 2, TRACEUNDENT),
        "Created mutex '%s' Handle=%P\n", pszName, pMutex));

    return pMutex;
}


/*-------------------------------------------------------------------
    Public: DclMutexDestroy()

    Destroy a mutex object which was created with DclMutexCreate().

    Parameters:
        pMutex   - A pointer to the mutex object.

    Return Value:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
D_BOOL DclMutexDestroy(
    PDCLMUTEX       pMutex)
{
    DCLSTATUS       dclStat;

    DclAssert(pMutex);

  #if DCLCONF_MUTEXTRACKING
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_MUTEX, 1, 0),
        "Destroying mutex '%s' Current=%u TotAcquired=%lU TotalUS=%llU MaxUS=%lU\n",
        pMutex->szName, pMutex->nCurrentCount, pMutex->ulAcquireCount,
        VA64BUG(pMutex->ullTotalWaitUS), pMutex->ulMaxWaitUS));

    /*  Must use a mutex to protect modification to the mutex chain.
    */
    if(pMutexChainProtect && (DclOsMutexAcquire(pMutexChainProtect) == DCLSTAT_SUCCESS))
    {
        PDCLMUTEX   pPrev;

        pPrev = pMutexChainHead;

        /*  Find the previous mutex that is pointing to us (could
            be the head), and modify it to point to the one after us.
        */
        while(pPrev && pPrev->pNext != pMutex)
            pPrev = pPrev->pNext;

        if(pPrev == NULL)
        {
            DclAssert(pMutex == pMutexChainHead);
            pMutexChainHead = pMutex->pNext;
        }
        else
        {
            pPrev->pNext = pMutex->pNext;
        }

        DclOsMutexRelease(pMutexChainProtect);
    }

  #else
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_MUTEX, 1, 0),
        "Destroying mutex '%s' Current=%u\n",
        pMutex->szName, pMutex->nCurrentCount));
  #endif

    dclStat = DclOsMutexDestroy((PDCLOSMUTEX)&pMutex->ulOsMutex);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DCLTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, TRACEUNDENT),
            "Unable to destroy mutex, status=%lX\n", dclStat));

        DclError();

        return FALSE;
    }

    DclMemFree(pMutex);

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: DclMutexRelease()

    Release a mutex object.

    Parameters:
        pMutex   - A pointer to the mutex object.

    Return Value:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
D_BOOL DclMutexRelease(
    PDCLMUTEX       pMutex)
{
    DCLSTATUS       dclStat;
    D_BOOL          fSuccess = TRUE;

    DclAssert(pMutex);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_MUTEX, 2, 0),
        "Releasing mutex '%s' Count=%u Owner=%lX\n",
        pMutex->szName, pMutex->nCurrentCount, pMutex->idOwner));

    DclProfilerEnter("DclMutexRelease", 0, 0);

  #if D_DEBUG
    if(pMutex->idOwner != DclOsThreadID())
    {
        DCLPRINTF(1, ("Thread %lX is releasing mutex '%s', but it is owned by %lX\n",
            DclOsThreadID(), pMutex->szName, pMutex->idOwner));

        /*  Don't assert here as we have a test which exercises this...
        DclError(); 
        */
    }
  #endif

    /*  Must decrement this <before> releasing the mutex.  This is
        necessary to allow interpretation of the owner field to be
        accurate.
    */
    pMutex->nCurrentCount--;

    dclStat = DclOsMutexRelease((PDCLOSMUTEX)&pMutex->ulOsMutex);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DCLPRINTF(1, ("Unable to release mutex %s, status=%lX\n", pMutex->szName, dclStat));

        /*  Don't assert here as we have a test which exercises this...
        DclError();
        */
        
        fSuccess = FALSE;
    }

    DclProfilerLeave(0);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_MUTEX, 3, 0),
        "Releasing mutex '%s' Count=%u returning %u\n", pMutex->szName, pMutex->nCurrentCount, fSuccess));

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected: DclMutexStatsQuery()

    Gather mutex statistics.

    Parameters:
        pDMS     - A pointer to the DCLMUTEXSTATS structure to fill.
                   On entry, the nStrucLen field must be set to
                   the size of the structure.  Additionally, the
                   uSupplied field must contain the number of
                   DCLMUTEXINFO structures supplied (may be zero).
        fVerbose - Display verbose statistics.
        fReset   - Reset the reset-able counts to zero.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclMutexStatsQuery(
    DCLMUTEXSTATS  *pDMS,
    D_BOOL          fVerbose,
    D_BOOL          fReset)
{
    (void)fVerbose;

  #if DCLCONF_MUTEXTRACKING
    DclAssert(pDMS);

    if(pDMS->nStrucLen != sizeof(*pDMS))
        return DCLSTAT_BADSTRUCLEN;

    pDMS->nCount = 0;
    pDMS->nReturned = 0;

    if(pMutexChainProtect && (DclOsMutexAcquire(pMutexChainProtect) == DCLSTAT_SUCCESS))
    {
        PDCLMUTEX       pMut = pMutexChainHead;
        unsigned        nAvailable = pDMS->nSupplied;
        DCLMUTEXINFO   *pDMI = pDMS->pDMI;

        while(pMut)
        {
            pDMS->nCount++;

            if(nAvailable && pDMI)
            {
                DclMemSet(pDMI, 0, sizeof(*pDMI));

                DclStrNCpy(pDMI->szName, pMut->szName, sizeof(pDMI->szName)-1);
                pDMI->nCurrentCount     = pMut->nCurrentCount;
                pDMI->ulAcquireCount    = pMut->ulAcquireCount;
                pDMI->ullTotalWaitUS    = pMut->ullTotalWaitUS;
                pDMI->ulMaxWaitUS       = pMut->ulMaxWaitUS;

                pDMI++;
                pDMS->nReturned++;
                nAvailable--;
            }

            if(fReset)
            {
                pMut->ulAcquireCount = 0;
                pMut->ullTotalWaitUS = 0;
                pMut->ulMaxWaitUS = 0;
            }

            pMut = pMut->pNext;
        }

        DclOsMutexRelease(pMutexChainProtect);

        if(fReset)
        {
            DCLPRINTF(1, ("Mutex statistics reset\n"));
        }

        return DCLSTAT_SUCCESS;
    }

    return DCLSTAT_MUTEXSTATSERROR;

  #else

    (void)pDMS;
    (void)fReset;

    return DCLSTAT_MUTEXTRACKINGDISABLED;

  #endif
}
