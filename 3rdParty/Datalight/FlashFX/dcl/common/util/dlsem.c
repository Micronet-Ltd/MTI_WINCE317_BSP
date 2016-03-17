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

    This module contains the DCL semaphore abstraction.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlsem.c $
    Revision 1.15  2010/12/21 22:53:56Z  garyp
    Documentation corrections -- no functional changes.
    Revision 1.14  2010/08/04 00:10:32Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.13  2009/11/07 23:09:51Z  garyp
    Added profiler instrumentation.  Updated debugging messages.  No
    functional changes.
    Revision 1.12  2009/11/04 18:50:48Z  garyp
    Updated to track statistics in microseonds rather than milliseconds.
    Revision 1.11  2009/10/27 23:26:15Z  garyp
    Debug messages updated -- no functional changes.
    Revision 1.10  2009/06/28 00:51:03Z  garyp
    Updated to track some stats in microseconds rather than milliseconds.
    Revision 1.9  2009/04/10 02:00:54Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.8  2007/12/18 03:53:18Z  brandont
    Updated function headers.
    Revision 1.7  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.6  2007/03/29 22:52:18Z  Garyp
    Modified the semaphore statistics to track the total and maximum amount
    of time waiting to acquire the semaphores.
    Revision 1.5  2006/10/04 02:42:02Z  Garyp
    Updated to use DclMemAllocZero().
    Revision 1.4  2006/05/14 05:22:00Z  Keithg
    Added explicit type cast to placate MSVC7
    Revision 1.3  2006/02/23 02:14:56Z  Garyp
    Renamed the mutex to be product specific.
    Revision 1.2  2006/01/02 06:39:01Z  Garyp
    Updated the stats interface to query and return the stats in a structure,
    rather than display them.  Fixed the bug referenced in the log for the
    revision dated 11-7-2005.
    Revision 1.1  2005/11/13 02:33:30Z  Pauli
    Initial revision
    Revision 1.3  2005/11/13 02:33:29Z  Garyp
    Updated the statistics interface functions to quietly return FALSE if the
    particular stat interface is disabled.
    Revision 1.2  2005/11/07 08:08:20Z  Garyp
    Added DclSemaphoreStatistics().  NOTE -- There is a bug somewhere in
    the logic for handling the semaphore chain (for semaphore tracking only),
    so beware when displaying the stats.
    Revision 1.1  2005/10/02 04:57:08Z  Garyp
    Initial revision
    Revision 1.4  2005/09/28 01:45:48Z  Garyp
    Tweaked the structure name.
    Revision 1.3  2005/09/18 01:50:28Z  garyp
    Debug code updated.
    Revision 1.2  2005/08/03 19:24:28Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/06 02:59:28Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiprv.h>
#include <dlstats.h>

/*
    ToDo: Update this to use the system mutex when semaphore tracking is 
          enabled, rather than a private mutex.
*/

struct DCLSEMAPHORE
{
  #if DCLCONF_SEMAPHORETRACKING
    struct DCLSEMAPHORE    *pNext;          /* next in the linked list          */
    D_UINT64                ullTotalWaitUS; /* Total US spent waiting to acquire*/
    D_UINT32                ulMaxWaitUS;    /* Max US spent waiting to acquire  */
    D_UINT32                ulMaxCount;     /* max count at init time           */
    D_UINT32                ulAcquireCount; /* total # of times acquired        */
    D_UINT32                ulHighWater;    /* highest acquire count            */
  #endif
    char                    szName[DCL_PAD(DCL_SEMAPHORENAMELEN, char)];
    D_UINT32                ulCurrentCount; /* current acquire count            */
    D_UINT32                ulOsSem;        /* The start of the OS semaphore object */
};

#if DCLCONF_SEMAPHORETRACKING
  static PDCLMUTEX          pMutexSemChain = NULL;
  static PDCLSEMAPHORE      pHeadSemChain = NULL;
#endif


/*-------------------------------------------------------------------
    Public: DclSemaphoreAcquire()

    Acquire a semaphore object created with DclSemaphoreCreate().

    Parameters:
        pSemaphore   - a pointer to the semaphore object.

    Return Value:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
D_BOOL DclSemaphoreAcquire(
    PDCLSEMAPHORE   pSemaphore)
{
    DCLSTATUS       dclStat;
    D_BOOL          fSuccess = TRUE;
  #if DCLCONF_SEMAPHORETRACKING
    DCLTIMESTAMP    ts = DclTimeStamp();
    D_UINT32        ulElapsedUS;
  #endif

    DclAssert(pSemaphore);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 2, 0),
        "Acquiring semaphore '%s' CurrentCount=%lU\n", pSemaphore->szName, pSemaphore->ulCurrentCount));

    DclProfilerEnter("DclSemaphoreAcquire", 0, 0);

    dclStat = DclOsSemaphoreAcquire((PDCLOSSEMAPHORE)&pSemaphore->ulOsSem);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DCLPRINTF(1, ("Unable to obtain semaphore, status=%lX\n", dclStat));

        DclError();

        fSuccess = FALSE;
    }
    else
    {
        pSemaphore->ulCurrentCount++;

      #if DCLCONF_SEMAPHORETRACKING
        ulElapsedUS = DclTimePassedUS(ts);
        pSemaphore->ullTotalWaitUS += ulElapsedUS;
        if(pSemaphore->ulMaxWaitUS < ulElapsedUS)
            pSemaphore->ulMaxWaitUS = ulElapsedUS;

        pSemaphore->ulAcquireCount++;

        if(pSemaphore->ulHighWater < pSemaphore->ulCurrentCount)
            pSemaphore->ulHighWater = pSemaphore->ulCurrentCount;

        DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 3, 0),
            "Acquired semaphore '%s' CurrentCount=%lU HighCount=%lU Time=%lU us\n",
             pSemaphore->szName, pSemaphore->ulCurrentCount, pSemaphore->ulHighWater, ulElapsedUS));
      #else
        DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 3, 0),
            "Acquired semaphore '%s' CurrentCount=%lU\n",
            pSemaphore->szName, pSemaphore->ulCurrentCount));
      #endif
    }

    DclProfilerLeave(0);

    return fSuccess;
}


/*-------------------------------------------------------------------
    Public: DclSemaphoreCreate()

    Create a semaphore object.

    Parameters:
        pszName  - A pointer to the null terminated semaphore name.
        ulCount  - The initial semaphore count.

    Return Value:
        Returns a pointer to the semaphore object or NULL if failure.
-------------------------------------------------------------------*/
PDCLSEMAPHORE DclSemaphoreCreate(
    const char     *pszName,
    D_UINT32        ulCount)
{
    D_UINT16        uSemObjLen;
    PDCLSEMAPHORE   pSemaphore;
    DCLSTATUS       dclStat;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 1, 0),
        "Creating semaphore '%s' MaxCount=%lU\n", pszName, ulCount));

    DclAssert(pszName);
    DclAssert(DclStrLen(pszName) < DCL_SEMAPHORENAMELEN);

    /*  Allocate memory for our semaphore object, plus enough for the
        actual OS level semaphore information.
    */
    uSemObjLen = sizeof *pSemaphore - sizeof(pSemaphore->ulOsSem) + (D_UINT16) DclOsSemaphoreInfo(NULL);

    pSemaphore = DclMemAllocZero(uSemObjLen);
    if(!pSemaphore)
    {
        DclError();

        return NULL;
    }

    DclStrCpy(pSemaphore->szName, pszName);

    dclStat = DclOsSemaphoreCreate((PDCLOSSEMAPHORE)&pSemaphore->ulOsSem, pszName, ulCount);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DCLPRINTF(1, ("Unable to create semaphore, status=%lX\n", dclStat));

        DclError();

        DclMemFree(pSemaphore);

        return NULL;
     }

  #if DCLCONF_SEMAPHORETRACKING
    pSemaphore->ulMaxCount = ulCount;

    /*  Use a mutex to protect the semaphore chain
    */
    if(!pMutexSemChain)
        pMutexSemChain = DclMutexCreate(D_PRODUCTPREFIX"SEMTR");

    if(pMutexSemChain && DclMutexAcquire(pMutexSemChain))
    {
        pSemaphore->pNext = pHeadSemChain;

        pHeadSemChain = pSemaphore;

        DclMutexRelease(pMutexSemChain);
    }
    else
    {
        DclError();
    }
  #endif

    return pSemaphore;
}


/*-------------------------------------------------------------------
    Public: DclSemaphoreDestroy()

    Destroy a semaphore object that was created with
    DclSemaphoreCreate().

    Parameters:
        pSemaphore   - A pointer to the semaphore object.

    Return Value:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
D_BOOL DclSemaphoreDestroy(
    PDCLSEMAPHORE   pSemaphore)
{
    DCLSTATUS       dclStat;

    DclAssert(pSemaphore);

  #if DCLCONF_SEMAPHORETRACKING
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 1, 0),
        "Destroying semaphore '%s' MaxCount=%lU HighWater=%lU Current=%lU Acquired=%lU TotUS=%llU MaxUS=%lU\n",
        pSemaphore->szName, pSemaphore->ulMaxCount, pSemaphore->ulHighWater,
        pSemaphore->ulCurrentCount, pSemaphore->ulAcquireCount,
        VA64BUG(pSemaphore->ullTotalWaitUS), pSemaphore->ulMaxWaitUS));

    /*  Must use a mutex to protect modification to the semaphore chain.
    */
    if(pMutexSemChain && DclMutexAcquire(pMutexSemChain))
    {
        PDCLSEMAPHORE   pPrev;

        pPrev = pHeadSemChain;

        /*  Find the previous semaphore that is pointing to us (could
            be the head), and modify it to point to the one after us.
        */
        while(pPrev && pPrev->pNext != pSemaphore)
            pPrev = pPrev->pNext;

        if(pPrev == NULL)
        {
            DclAssert(pSemaphore == pHeadSemChain);
            pHeadSemChain = pSemaphore->pNext;
        }
        else
        {
            pPrev->pNext = pSemaphore->pNext;
        }

        DclMutexRelease(pMutexSemChain);
    }

  #else
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 1, 0),
        "Destroying semaphore '%s' CurrentCount=%lU\n",
        pSemaphore->szName, pSemaphore->ulCurrentCount));
  #endif

    dclStat = DclOsSemaphoreDestroy((PDCLOSSEMAPHORE)&pSemaphore->ulOsSem);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DCLPRINTF(1, ("Unable to delete semaphore, status=%lX\n", dclStat));

        DclError();

        return FALSE;
    }

    DclMemFree(pSemaphore);

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: DclSemaphoreRelease()

    Release a semaphore object, reducing its usage count by one.

    Parameters:
        pSemaphore   - A pointer to the semaphore object.

    Return Value:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
D_BOOL DclSemaphoreRelease(
    PDCLSEMAPHORE   pSemaphore)
{
    DCLSTATUS       dclStat;
    D_BOOL          fSuccess = TRUE;

    DclAssert(pSemaphore);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 2, 0),
        "Releasing semaphore '%s' CurrentCount=%lU\n",
        pSemaphore->szName, pSemaphore->ulCurrentCount));

    DclProfilerEnter("DclSemaphoreRelease", 0, 0);

    pSemaphore->ulCurrentCount--;

    dclStat = DclOsSemaphoreRelease((PDCLOSSEMAPHORE)&pSemaphore->ulOsSem);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DCLPRINTF(1, ("Unable to release semaphore '%s', status=%lX\n", pSemaphore->szName, dclStat));

        DclError();

        fSuccess = FALSE;
    }

    DclProfilerLeave(0);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 3, 0),
        "Released semaphore '%s' CurrentCount=%lU\n",
         pSemaphore->szName, pSemaphore->ulCurrentCount));

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected: DclSemaphoreStatsQuery()

    Gather semaphore statistics.

    Parameters:
        pDSS     - A pointer to the DCLSEMAPHORESTATS structure to
                   fill.  On entry, the nStrucLen field must be set
                   to the size of the structure.  Additionally, the
                   uSupplied field must contain the number of
                   DCLSEMAPHOREINFO structures supplied (may be
                   zero).
        fVerbose - Display verbose statistics.
        fReset   - Reset the reset-able counts to zero.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclSemaphoreStatsQuery(
    DCLSEMAPHORESTATS  *pDSS,
    D_BOOL              fVerbose,
    D_BOOL              fReset)
{
    (void)fVerbose;

  #if DCLCONF_SEMAPHORETRACKING
    DclAssert(pDSS);

    if(pDSS->nStrucLen != sizeof(*pDSS))
        return DCLSTAT_BADSTRUCLEN;

    pDSS->nCount = 0;
    pDSS->nReturned = 0;

    if(pMutexSemChain && DclMutexAcquire(pMutexSemChain))
    {
        PDCLSEMAPHORE       pSem = pHeadSemChain;
        unsigned            nAvailable = pDSS->nSupplied;
        DCLSEMAPHOREINFO   *pDSI = pDSS->pDSI;

        while(pSem)
        {
            pDSS->nCount++;

            if(nAvailable && pDSI)
            {
                DclMemSet(pDSI, 0, sizeof(*pDSI));

                DclStrNCpy(pDSI->szName, pSem->szName, sizeof(pDSI->szName)-1);
                pDSI->ulMaxCount        = pSem->ulMaxCount;
                pDSI->ulAcquireCount    = pSem->ulAcquireCount;
                pDSI->ulCurrentCount    = pSem->ulCurrentCount;
                pDSI->ulHighWater       = pSem->ulHighWater;
                pDSI->ullTotalWaitUS    = pSem->ullTotalWaitUS;
                pDSI->ulMaxWaitUS       = pSem->ulMaxWaitUS;

                pDSI++;
                pDSS->nReturned++;
                nAvailable--;
            }

            if(fReset)
            {
                pSem->ulAcquireCount = 0;
                pSem->ulHighWater = 0;
                pSem->ullTotalWaitUS = 0;
                pSem->ulMaxWaitUS = 0;
            }

            pSem = pSem->pNext;
        }

        DclMutexRelease(pMutexSemChain);

        if(fReset)
        {
            DCLPRINTF(1, ("Semaphore statistics reset\n"));
        }

        return DCLSTAT_SUCCESS;
    }

    return DCLSTAT_SEMAPHORESTATSERROR;

  #else

    (void)pDSS;
    (void)fReset;

    return DCLSTAT_SEMAPHORETRACKINGDISABLED;

  #endif
}


