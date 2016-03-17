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

    This module contains the DCL read/write semaphore abstraction.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlrdwrsem.c $
    Revision 1.18  2010/12/21 22:53:56Z  garyp
    Documentation corrections -- no functional changes.
    Revision 1.17  2010/08/03 22:52:46Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.16  2010/06/12 23:59:39Z  garyp
    Fixed some error handling logic in DclSemaphoreRdWrAcquireWrite().
    Added debug code.
    Revision 1.15  2009/12/12 01:30:34Z  garyp
    Corrected Bug 2812 -- misplaced asserts.
    Revision 1.14  2009/11/07 23:12:15Z  garyp
    Added profiler instrumentation.  Updated debugging messages.  No
    functional changes.
    Revision 1.13  2009/11/04 18:50:48Z  garyp
    Updated to track statistics in microseonds rather than milliseconds.
    Revision 1.12  2009/10/18 00:21:56Z  garyp
    Fixed DclSemaphoreRdWrCreate() to use the proper product prefix for the
    mutex and semaphore names.  Enhanced the debugging messages.
    Revision 1.11  2009/04/09 22:09:42Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.10  2008/06/03 21:43:09Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.9  2007/12/18 03:54:31Z  brandont
    Updated function headers.
    Revision 1.8  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.7  2007/10/15 21:29:18Z  billr
    Remove invalid assertion (see bug 1537).
    Revision 1.6  2007/04/27 02:43:03Z  pauli
    Added a missing parameter to a TRACEPRINTF in DclSemaphoreRdWrRelease.
    Revision 1.5  2007/03/30 18:26:53Z  Garyp
    Updated to fully support tracking read/write semaphore statistics.
    Revision 1.4  2007/02/13 00:50:27Z  brandont
    Changed the read/write semphores to use only a single mutex if the thread
    count is one.
    Revision 1.3  2006/10/04 02:40:53Z  Garyp
    Updated to use DclMemAllocZero().
    Revision 1.2  2006/08/03 23:03:42Z  joshuab
    Bug fixes for read/write semaphore, using DCL abstractions
    instead of the direct OS implementations.
    Revision 1.1  2006/07/26 21:18:26Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiprv.h>
#include <dlstats.h>

/*
    ToDo: Update this to use the system mutex when semaphore tracking is 
          enabled, rather than a private mutex.
*/

struct DCLSEMAPHORERDWR
{
  #if DCLCONF_SEMAPHORETRACKING
    struct DCLSEMAPHORERDWR *pNext;     /* next in the linked list                  */
    D_UINT32        ulReadAcquireCount; /* total # of times acquired for read       */
    D_UINT32        ulReadHighWater;    /* highest acquire count for read           */
    D_UINT64        ullReadTotalWaitUS; /* Total US spent waiting to acquire read   */
    D_UINT32        ulReadMaxWaitUS;    /* Max US spent waiting to acquire read     */
    D_UINT32        ulWriteAcquireCount;/* total # of times acquired for write      */
    D_UINT64        ullWriteTotalWaitUS;/* Total US spent waiting to acquire write  */
    D_UINT32        ulWriteMaxWaitUS;   /* Max US spent waiting to acquire write    */
  #endif
    D_UINT32        ulMaxCount;         /* max count at init time                   */
    char            szName[DCL_PAD(DCL_SEMAPHORENAMELEN, char)];
    D_UINT32        ulCurrentCount;     /* current acquire count                    */
    PDCLMUTEX       pWriteMutex;        /* mutex for contending write threads       */
    PDCLSEMAPHORE   pSem;               /* semaphore for all threads                */
    unsigned        fInWriteMode : 1;   /* acquired in write mode                   */
};

#if DCLCONF_SEMAPHORETRACKING
  static PDCLMUTEX          pMutexSemChain = NULL;
  static PDCLSEMAPHORERDWR  pHeadSemChain  = NULL;
#endif


/*-------------------------------------------------------------------
    Public: DclSemaphoreRdWrAcquireRead()

    Acquire a read/write semaphore object for reading.

    Multiple readers can acquire concurrently while no writer has
    acquired.

    Parameters:
        pSemaphore   - a pointer to the semaphore object.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
D_BOOL DclSemaphoreRdWrAcquireRead(
    PDCLSEMAPHORERDWR   pSemaphore)
{
    D_BOOL              fSuccess = TRUE;
  #if DCLCONF_SEMAPHORETRACKING
    DCLTIMESTAMP        ts = DclTimeStamp();
    D_UINT32            ulElapsedUS;
  #endif

    DclAssert(pSemaphore);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 2, 0),
        "Acquiring read/write semaphore '%s' for reading, CurrentCount=%lU\n",
        pSemaphore->szName, pSemaphore->ulCurrentCount));

    DclProfilerEnter("DclSemaphoreRdWrAcquireRead", 0, 0);

    if(pSemaphore->ulMaxCount > 1)
    {
        if(!DclSemaphoreAcquire(pSemaphore->pSem))
        {
            DCLPRINTF(1, ("Unable to obtain RDWRSEM read semaphore '%s'\n", pSemaphore->szName));
            DclError();
            fSuccess = FALSE;
        }
    }
    else
    {
        /*  If count is 1, acquire a single mutex
        */
        if(!DclMutexAcquire(pSemaphore->pWriteMutex))
        {
            DCLPRINTF(1, ("Unable to obtain RDWRSEM write mutex '%s'\n", pSemaphore->szName));
            DclError();
            fSuccess = FALSE;
        }
    }

    if(fSuccess)
    {
        /*  ulCurrentCount is maintained for debug messages and statistical
            counts only.  It possible that the value may get "off" due to 
            multithreaded access.  This could be remedied by using the 
            Atomic API, however that could introduce behavioral changes
            into the system which would not be desirable.  Take the value
            with a grain of salt...
        */        
        pSemaphore->ulCurrentCount++;

      #if DCLCONF_SEMAPHORETRACKING
        ulElapsedUS = DclTimePassedUS(ts);
        pSemaphore->ullReadTotalWaitUS += ulElapsedUS;
        if(pSemaphore->ulReadMaxWaitUS < ulElapsedUS)
            pSemaphore->ulReadMaxWaitUS = ulElapsedUS;

        pSemaphore->ulReadAcquireCount++;

        if(pSemaphore->ulReadHighWater < pSemaphore->ulCurrentCount)
        {
            pSemaphore->ulReadHighWater = pSemaphore->ulCurrentCount;

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 1, 0),
                "RDWRSEM '%s' new HighWaterCount is %lU\n", 
                pSemaphore->szName, pSemaphore->ulReadHighWater));
        }
        
        DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 3, 0),
            "Acquired RDWRSEM read semaphore '%s' CurrentCount=%lU HighWaterCount=%lU Time=%lU us\n",
             pSemaphore->szName, pSemaphore->ulCurrentCount,
             pSemaphore->ulReadHighWater, ulElapsedUS));
      #else
        DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 3, 0),
            "Acquired RDWRSEM read semaphore '%s' CurrentCount=%lU\n",
            pSemaphore->szName, pSemaphore->ulCurrentCount));
      #endif
    }

    DclProfilerLeave(0);
    
    return fSuccess;
}


/*-------------------------------------------------------------------
    Public: DclSemaphoreRdWrAcquireWrite()

    Acquire a read/write semaphore object for writing.

    Multiple readers can acquire concurrently while no writer has
    acquired.

    Parameters:
        pSemaphore   - a pointer to the semaphore object.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
D_BOOL DclSemaphoreRdWrAcquireWrite(
    PDCLSEMAPHORERDWR   pSemaphore)
{
    D_BOOL              fSuccess = TRUE;
  #if DCLCONF_SEMAPHORETRACKING
    DCLTIMESTAMP        ts = DclTimeStamp();
    D_UINT32            ulElapsedUS;
  #endif

    DclAssert(pSemaphore);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 2, 0),
        "Acquiring read/write semaphore '%s' for writing, CurrentCount=%lU\n",
        pSemaphore->szName, pSemaphore->ulCurrentCount));

    DclProfilerEnter("DclSemaphoreRdWrAcquireWrite", 0, 0);

    if(pSemaphore->ulMaxCount > 1)
    {
        D_UINT32    ulAttained;

        /*  Block for competing write threads
        */
        if(!DclMutexAcquire(pSemaphore->pWriteMutex))
        {
            DCLPRINTF(1, ("Unable to obtain RDWRSEM write mutex '%s'\n", pSemaphore->szName));
            DclError();
            fSuccess = FALSE;
        }

        /*  Get exclusive access by acquiring all the counting semaphores
        */
        for(ulAttained = 0; ulAttained < pSemaphore->ulMaxCount; ulAttained++)
        {
            if(!DclSemaphoreAcquire(pSemaphore->pSem))
            {
                while(ulAttained)
                {
                    DclSemaphoreRelease(pSemaphore->pSem);
                    ulAttained--;
                }
                
                DclMutexRelease(pSemaphore->pWriteMutex);
                
                DCLPRINTF(1, ("Unable to obtain RDWRSEM write semaphore '%s'\n", pSemaphore->szName));
                DclError();
                fSuccess = FALSE;
                break;
            }
        }

        /*  Finished block for competing write threads
        */
        if(!DclMutexRelease(pSemaphore->pWriteMutex))
        {
            DCLPRINTF(1, ("Unable to release RDWRSEM write mutex '%s'\n", pSemaphore->szName));
            DclError();
            fSuccess = FALSE;
        }

        /*  Indicate that this semaphore is now in write mode
        */
        if(fSuccess)
            pSemaphore->fInWriteMode = TRUE;
    }
    else
    {
        /*  If count is 1, acquire a single mutex
        */
        if(!DclMutexAcquire(pSemaphore->pWriteMutex))
        {
            DCLPRINTF(1, ("Unable to obtain RDWRSEM write mutex '%s'\n", pSemaphore->szName));
            DclError();
            fSuccess = FALSE;
        }
    }

    if(fSuccess)
    {
        pSemaphore->ulCurrentCount++;

      #if DCLCONF_SEMAPHORETRACKING
        ulElapsedUS = DclTimePassedUS(ts);
        pSemaphore->ullWriteTotalWaitUS += ulElapsedUS;
        if(pSemaphore->ulWriteMaxWaitUS < ulElapsedUS)
            pSemaphore->ulWriteMaxWaitUS = ulElapsedUS;

        pSemaphore->ulWriteAcquireCount++;

        DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 3, 0),
            "Acquired RDWRSEM write semaphore '%s' CurrentCount=%lU Time=%lU us\n",
             pSemaphore->szName, pSemaphore->ulCurrentCount, ulElapsedUS));
      #else
        DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 3, 0),
            "Acquired RDWRSEM write semaphore '%s' CurrentCount=%lU\n",
            pSemaphore->szName, pSemaphore->ulCurrentCount));
      #endif
    }

    DclProfilerLeave(0);
    
    return fSuccess;
}


/*-------------------------------------------------------------------
    Public: DclSemaphoreRdWrCreate()

    Create a read/write semaphore object.

    Parameters:
        pszName  - A pointer to the null terminated semaphore name.
        ulCount  - The initial semaphore count.

    Return Value:
        Returns a pointer to the semaphore object or NULL if failure.
-------------------------------------------------------------------*/
PDCLSEMAPHORERDWR DclSemaphoreRdWrCreate(
    const char         *pszName,
    D_UINT32            ulCount)
{
    PDCLSEMAPHORERDWR   pSemaphore;
    char                szMutexName[DCL_MUTEXNAMELEN];

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 1, 0),
        "Creating read/write semaphore '%s' MaxCount=%lU\n", pszName, ulCount));

    DclAssert(pszName);
    DclAssert(DclStrLen(pszName) + 5 < DCL_MUTEXNAMELEN);
    DclAssert(ulCount);

    /*  Allocate memory for our semaphore object - storage for the
        mutex and sempahore is handled by those routines
    */
    pSemaphore = DclMemAllocZero(sizeof(*pSemaphore));
    if(!pSemaphore)
    {
        DclError();
        return NULL;
    }

    DclStrCpy(pSemaphore->szName, pszName);

    /*   Create a mutex to manage competing write threads
    */
    DclSNPrintf(szMutexName, DCLDIMENSIONOF(szMutexName), D_PRODUCTPREFIX"RWM%s", pszName);
    pSemaphore->pWriteMutex = DclMutexCreate(szMutexName);
    if(NULL == pSemaphore->pWriteMutex)
    {
        DCLPRINTF(1, ("Unable to create RDWRSEM mutex '%s'\n", szMutexName));
        DclError();
        DclMemFree(pSemaphore);
        return NULL;
    }

    /*  This abstraction will treat read/write semaphores as a single mutex
        if the count is 1
    */
    if(ulCount > 1)
    {
        char    szSemName[DCL_SEMAPHORENAMELEN];

        DclAssert(DclStrLen(pszName) + 5 < DCL_SEMAPHORENAMELEN);

        /*   Create a counting semaphore to manage all threads
        */
        DclSNPrintf(szSemName, DCLDIMENSIONOF(szSemName), D_PRODUCTPREFIX"RWS%s", pszName);
        pSemaphore->pSem = DclSemaphoreCreate(szSemName, ulCount);
        if(NULL == pSemaphore->pSem)
        {
            DclMutexDestroy(pSemaphore->pWriteMutex);
            DCLPRINTF(1, ("Unable to create RDWRSEM semaphore '%s'\n", szSemName));
            DclError();
            DclMemFree(pSemaphore);
            return NULL;
        }
    }

    pSemaphore->ulMaxCount = ulCount;
    pSemaphore->fInWriteMode = FALSE;

  #if DCLCONF_SEMAPHORETRACKING
    /*  Use a mutex to protect the semaphore chain
    */
    if(!pMutexSemChain)
    {
        DclAssert( DclStrLen(D_PRODUCTPREFIX"RWSTR") < DCL_MUTEXNAMELEN );
        pMutexSemChain = DclMutexCreate(D_PRODUCTPREFIX"RWSTR");
    }

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
    Public: DclSemaphoreRdWrDestroy()

    Destroy a read/write semaphore object that was created with
    DclSemaphoreRdWrCreate().

    Parameters:
        pSemaphore   - A pointer to the semaphore object.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
D_BOOL DclSemaphoreRdWrDestroy(
    PDCLSEMAPHORERDWR pSemaphore)
{
    DclAssert(pSemaphore);

  #if DCLCONF_SEMAPHORETRACKING
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 1, 0),
        "Destroying read/write semaphore '%s' MaxCount=%lU CurrentCount=%lU fWriting=%u\n",
        pSemaphore->szName, pSemaphore->ulMaxCount, pSemaphore->ulCurrentCount, pSemaphore->fInWriteMode));
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 1, 0),
        "    Read Stats:  Acquired=%lU TotUS=%llU MaxUS=%lU HighWater=%lU\n",
        pSemaphore->ulReadAcquireCount,
        VA64BUG(pSemaphore->ullReadTotalWaitUS), pSemaphore->ulReadMaxWaitUS, pSemaphore->ulReadHighWater));
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 1, 0),
        "    Write Stats: Acquired=%lU TotUS=%llU MaxUS=%lU\n",
        pSemaphore->ulWriteAcquireCount,
        VA64BUG(pSemaphore->ullWriteTotalWaitUS), pSemaphore->ulWriteMaxWaitUS));

    /*  Must use a mutex to protect modification to the semaphore chain.
    */
    if(pMutexSemChain && DclMutexAcquire(pMutexSemChain))
    {
        PDCLSEMAPHORERDWR pPrev;

        pPrev = pHeadSemChain;

        /*  Find the previous semaphore that is pointing to us (could
            be the head), and modify it to point to the one after us.
        */
        while(pPrev && pPrev->pNext != pSemaphore)
        {
            pPrev = pPrev->pNext;
        }
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
        "Destroying read/write semaphore '%s' CurrentCount=%lU fWriting=%u\n",
        pSemaphore->szName, pSemaphore->ulCurrentCount, pSemaphore->fInWriteMode));
  #endif

    if(pSemaphore->ulMaxCount > 1)
    {
        /*  Destroy the counting semaphore
        */
        if(!DclSemaphoreDestroy(pSemaphore->pSem))
        {
            DCLPRINTF(1, ("Unable to delete RDWRSEM semaphore '%s'\n", pSemaphore->szName));
            DclError();
            return FALSE;
        }
    }

    /*  Destroy the mutex used for competing write threads
    */
    if(!DclMutexDestroy(pSemaphore->pWriteMutex))
    {
        DCLPRINTF(1, ("Unable to delete RDWRSEM mutex '%s'\n", pSemaphore->szName));
        DclError();
        return FALSE;
    }

    DclMemFree(pSemaphore);

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: DclSemaphoreRdWrRelease()

    Release a read/write semaphore object.

    Parameters:
        pSemaphore   - A pointer to the semaphore object.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
D_BOOL DclSemaphoreRdWrRelease(
    PDCLSEMAPHORERDWR   pSemaphore)
{
    D_BOOL              fSuccess = TRUE;
    
    DclAssert(pSemaphore);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 2, 0),
        "Releasing read/write semaphore '%s' CurrentCount=%lU fWriting=%u\n",
        pSemaphore->szName, pSemaphore->ulCurrentCount, pSemaphore->fInWriteMode));

    DclProfilerEnter("DclSemaphoreRdWrRelease", 0, 0);

    if(pSemaphore->ulMaxCount > 1)
    {
        D_UINT32    ulToRelease;

        /*  Determine if this semaphore is in write mode
        */
        if(pSemaphore->fInWriteMode)
        {
            /*  Clear the write mode flag
            */
            pSemaphore->fInWriteMode = FALSE;

            /*  Set the number of semaphores to release
            */
            ulToRelease = pSemaphore->ulMaxCount;
        }
        else
        {
            /*  Set the number of semaphores to release
            */
            ulToRelease = 1;
        }

        /*  Release the semaphore once or for max count if in write mode
        */
        while(ulToRelease)
        {
            if(!DclSemaphoreRelease(pSemaphore->pSem))
            {
                DCLPRINTF(1, ("Unable to release RDWRSEM semaphore '%s'\n", pSemaphore->szName));
                DclError();
                fSuccess = FALSE;
            }
            ulToRelease--;
        }
    }
    else
    {
        if(!DclMutexRelease(pSemaphore->pWriteMutex))
        {
            DCLPRINTF(1, ("Unable to release RDWRSEM mutex '%s'\n", pSemaphore->szName));
            DclError(); 
            fSuccess = FALSE;
        }
    }

    if(fSuccess)
        pSemaphore->ulCurrentCount--;

    DclProfilerLeave(0);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SEMAPHORE, 2, 0),
        "Releasing read/write semaphore '%s' CurrentCount=%lU returning %u\n",
         pSemaphore->szName, pSemaphore->ulCurrentCount, fSuccess));

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected: DclSemaphoreRdWrStatsQuery()

    Gather semaphore statistics.

    Parameters:
        pRWS     - A pointer to the DCLRDWRSEMAPHORESTATS structure
                   to fill.  On entry, the nStrucLen field must be
                   set to the size of the structure.  Additionally,
                   the uSupplied field must contain the number of
                   DCLRDWRSEMAPHOREINFO structures supplied (may be
                   zero).
        fVerbose - Display verbose statistics.
        fReset   - Reset the reset-able counts to zero.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclSemaphoreRdWrStatsQuery(
    DCLRDWRSEMAPHORESTATS  *pRWS,
    D_BOOL                  fVerbose,
    D_BOOL                  fReset)
{
    (void)fVerbose;

  #if DCLCONF_SEMAPHORETRACKING
    DclAssert(pRWS);

    if(pRWS->nStrucLen != sizeof(*pRWS))
    {
        return DCLSTAT_BADSTRUCLEN;
    }

    pRWS->nCount = 0;
    pRWS->nReturned = 0;

    if(pMutexSemChain && DclMutexAcquire(pMutexSemChain))
    {
        PDCLSEMAPHORERDWR       pSem = pHeadSemChain;
        unsigned                nAvailable = pRWS->nSupplied;
        DCLRDWRSEMAPHOREINFO   *pRWI = pRWS->pRWI;

        while(pSem)
        {
            pRWS->nCount++;
            if(nAvailable && pRWI)
            {
                DclMemSet(pRWI, 0, sizeof(*pRWI));

                DclStrNCpy(pRWI->szName, pSem->szName, sizeof(pRWI->szName)-1);
                pRWI->ulMaxCount            = pSem->ulMaxCount;
                pRWI->ulCurrentCount        = pSem->ulCurrentCount;
                pRWI->fWriting              = pSem->fInWriteMode;
                pRWI->ulReadAcquireCount    = pSem->ulReadAcquireCount;
                pRWI->ulReadHighWater       = pSem->ulReadHighWater;
                pRWI->ullReadTotalWaitUS    = pSem->ullReadTotalWaitUS;
                pRWI->ulReadMaxWaitUS       = pSem->ulReadMaxWaitUS;
                pRWI->ulWriteAcquireCount   = pSem->ulWriteAcquireCount;
                pRWI->ullWriteTotalWaitUS   = pSem->ullWriteTotalWaitUS;
                pRWI->ulWriteMaxWaitUS      = pSem->ulWriteMaxWaitUS;

                pRWI++;
                pRWS->nReturned++;
                nAvailable--;
            }

            if(fReset)
            {
                pSem->ulReadAcquireCount = 0;
                pSem->ulReadHighWater = 0;
                pSem->ullReadTotalWaitUS = 0;
                pSem->ulReadMaxWaitUS = 0;
                pSem->ulWriteAcquireCount = 0;
                pSem->ullWriteTotalWaitUS = 0;
                pSem->ulWriteMaxWaitUS = 0;
            }

            pSem = pSem->pNext;
        }

        DclMutexRelease(pMutexSemChain);

        if(fReset)
        {
            DCLPRINTF(1, ("Read/write semaphore statistics reset\n"));
        }

        return DCLSTAT_SUCCESS;
    }

    return DCLSTAT_SEMAPHORESTATSERROR;

  #else

    (void)pRWS;
    (void)fReset;

    return DCLSTAT_SEMAPHORETRACKINGDISABLED;
  #endif
}


