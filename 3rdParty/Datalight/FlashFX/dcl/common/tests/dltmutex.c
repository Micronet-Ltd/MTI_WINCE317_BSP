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

    This module contains code for testing the OS Mutex abstraction.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltmutex.c $
    Revision 1.4  2010/09/22 22:24:14Z  garyp
    Fixed to build cleanly when DCL_OSFEATURE_THREADS is FALSE (again).
    Revision 1.3  2010/09/18 02:13:10Z  garyp
    Added a mutex scope test.
    Revision 1.2  2010/09/09 00:24:41Z  garyp
    Fixed to build cleanly when DCL_OSFEATURE_THREADS is FALSE.
    Revision 1.1  2010/09/06 18:51:16Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dltools.h>
#include "dltests.h"


#if DCL_OSFEATURE_THREADS

#define TEST_INITTING   (0)
#define TEST_RUNNING    (1)
#define TEST_QUITTING   (2) /* Waiting for all threads to be at THRD_DONE */
#define TEST_EXITING    (3)

#define THRD_INITTING   (0)
#define THRD_WAITING    (1) /* Waiting for test state != TEST_INITTING  */
#define THRD_RUNNING    (2)
#define THRD_DONE       (3) /* Thread is about to exit */

#define BLOCKING_THREADS        (3)
#define BLOCKING_ITERATIONS   (100)
#define BLOCKING_MS            (10)
#define BLOCKING_TOTAL_TEST_MS (BLOCKING_ITERATIONS * BLOCKING_MS * BLOCKING_THREADS)
#define BLOCKING_TOTAL_MAX_MS  (BLOCKING_TOTAL_TEST_MS * 10) /* max before we abort */
typedef struct
{
    D_ATOMIC32          ulAtomicTestState;      /* TEST_* state values */
    char                szSharedResource[16];
    PDCLMUTEX           pMutex;
} BLOCKINGTESTINFO;    

typedef struct
{
    BLOCKINGTESTINFO   *pTestInf;
    DCLTHREADHANDLE     hThread;
    DCLTHREADID         idThread;
    D_ATOMIC32          ulAtomicThreadState;    /* THRD_* state values */
    DCLSTATUS           ThreadStatusCode;
    D_UINT32            ulCounter;
    D_UINT32            ulMaxAcquireUS;
    D_UINT32            ulMaxReacquireUS;
    char                szString[16];
} BLOCKINGTHREADINFO;


typedef struct
{
    D_ATOMIC32          ulAtomicThreadState;    /* THRD_* state values */
    PDCLMUTEX           pMutex;
    DCLSTATUS           dclStat;
} SCOPETESTINFO;    


static DCLSTATUS    TestBasicFunctionality(void);
static DCLSTATUS    TestMutexBlocking(void);
static DCLSTATUS    TestMutexScope(void);
static void *       ThreadFunc(void *pArgs);
static void *       ScopeThread(void *pArgs);
static DCLSTATUS    CompeteForResource(BLOCKINGTHREADINFO *pBTI);


#endif  /* DCL_OSFEATURE_THREADS */


/*-------------------------------------------------------------------
    Public: DclTestMutexes()

    Invoke the unit tests for excercising the mutex abstraction.

    Parameters:

    Return Value:
        Returns a DCLSTATUS value indicating the test result.
-------------------------------------------------------------------*/
DCLSTATUS DclTestMutexes(
    unsigned        nVerbosity)
{
    DCLSTATUS       dclStat;

    DclPrintf("Testing Mutexes...\n");

  #if !DCL_OSFEATURE_THREADS
    DclPrintf("  The OS abstraction is configured with DCL_OSFEATURE_THREADS set to FALSE\n");

    dclStat = DCLSTAT_SUCCESS;

  #else

    /*  To test:
            Priority inversion protection
    */

    dclStat = TestBasicFunctionality();
    if(dclStat != DCLSTAT_SUCCESS)
        goto TestCleanup;

    dclStat = TestMutexBlocking();
    if(dclStat != DCLSTAT_SUCCESS)
        goto TestCleanup;

    dclStat = TestMutexScope();
    if(dclStat != DCLSTAT_SUCCESS)
        goto TestCleanup;
    
  TestCleanup:
  #endif
  
    if(dclStat == DCLSTAT_SUCCESS)
        DclPrintf("    OK\n");
    else
        DclPrintf("    FAILED\n");

    return dclStat;
}


#if DCL_OSFEATURE_THREADS

/*-------------------------------------------------------------------
    Local: TestBasicFunctionality()

    Parameters:

    Return Value:
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS TestBasicFunctionality(void)
{
    PDCLMUTEX           pMutexA;

    DclPrintf("    Testing Basic Mutex Functionality...\n");
    DclPrintf("      Create/acquire/release/destroy an unnamed mutex\n");

    pMutexA = DclMutexCreate("");
    if(!pMutexA)
        return DCLSTAT_CURRENTLINE;

    if(!DclMutexAcquire(pMutexA))
        return DCLSTAT_CURRENTLINE;
        
    if(!DclMutexRelease(pMutexA))
        return DCLSTAT_CURRENTLINE;
        
    if(!DclMutexDestroy(pMutexA))
        return DCLSTAT_CURRENTLINE;

    DclPrintf("      Release a mutex which is not acquired\n");
    
    pMutexA = DclMutexCreate("DLTMUTA");
    if(!pMutexA)
        return DCLSTAT_CURRENTLINE;

    if(!DclMutexAcquire(pMutexA))
        return DCLSTAT_CURRENTLINE;
        
    if(!DclMutexRelease(pMutexA))
        return DCLSTAT_CURRENTLINE;

    /*  This should fail...
    */
    if(DclMutexRelease(pMutexA))
    {
        DclPrintf("        Warning: test did not fail as expected -- continuing...\n");
        /* return DCLSTAT_CURRENTLINE; */
    }
        
    if(!DclMutexDestroy(pMutexA))
        return DCLSTAT_CURRENTLINE;

    DclPrintf("      Destroy a mutex which is still acquired\n");
    
    pMutexA = DclMutexCreate("DLTMUTA");
    if(!pMutexA)
        return DCLSTAT_CURRENTLINE;

    if(!DclMutexAcquire(pMutexA))
        return DCLSTAT_CURRENTLINE;
        
    if(!DclMutexDestroy(pMutexA))
        return DCLSTAT_CURRENTLINE;


    DclPrintf("      Recursively acquire a mutex multiple times\n");
    
    pMutexA = DclMutexCreate("DLTMUTA");
    if(!pMutexA)
        return DCLSTAT_CURRENTLINE;

    /*  Acquire 3 times
    */
    if(!DclMutexAcquire(pMutexA))
        return DCLSTAT_CURRENTLINE;
        
    if(!DclMutexAcquire(pMutexA))
        return DCLSTAT_CURRENTLINE;
        
    if(!DclMutexAcquire(pMutexA))
        return DCLSTAT_CURRENTLINE;

    /*  Release 3 times
    */
    if(!DclMutexRelease(pMutexA))
        return DCLSTAT_CURRENTLINE;

    if(!DclMutexRelease(pMutexA))
        return DCLSTAT_CURRENTLINE;

    if(!DclMutexRelease(pMutexA))
        return DCLSTAT_CURRENTLINE;
       
    if(!DclMutexDestroy(pMutexA))
        return DCLSTAT_CURRENTLINE;

    return DCLSTAT_SUCCESS;
}   
    

/*-------------------------------------------------------------------
    Local: TestMutexBlocking()

    Parameters:

    Return Value:
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS TestMutexBlocking(void)
{
    DCLSTATUS           dclStat;
    DCLTHREADATTR      *pAttr = NULL;
    BLOCKINGTESTINFO    TestInf = {0};
    BLOCKINGTHREADINFO  bti[BLOCKING_THREADS];
    unsigned            nThreads = 0;
    unsigned            nCurThread = 0;
    char                szBaseString[] = "ABCDEFG";
    DCLTIMER            timer;
    D_UINT32            ulMS;

    DclPrintf("    Testing Mutex Blocking Functionality...\n");

    TestInf.pMutex = DclMutexCreate("DLTMUTA");
    if(!TestInf.pMutex)
        return DCLSTAT_CURRENTLINE;

    pAttr = DclOsThreadAttrCreate();
    if(!pAttr)
    {
        dclStat = DCLSTAT_THREADATTRCREATEFAILED;
        goto BlockingCleanup;
    }

    for(nThreads = 0; nThreads < DCLDIMENSIONOF(bti); nThreads++)
    {
        char   *pStr;
        char    szName[DCL_THREADNAMELEN];

        DclMemSet(&bti[nThreads], 0, sizeof(bti[nThreads]));

        bti[nThreads].pTestInf = &TestInf;

        DclStrNCpy(bti[nThreads].szString, szBaseString, sizeof(bti[nThreads].szString));

        DclSNPrintf(szName, sizeof(szName), "DLTTHD%u", nThreads);
        
        dclStat = DclOsThreadCreate(&bti[nThreads].hThread, szName, pAttr, ThreadFunc, &bti[nThreads]);
        if(dclStat != DCLSTAT_SUCCESS)
            goto BlockingCleanup;

        /*  Make each byte of the string unique for the next thread
        */
        pStr = szBaseString;
        while(*pStr)
        {
            (*pStr)++;
            pStr++;
        };
    }
    
    /*  The "attr" object was only needed temporarily while the
        thread was being created.  We can destroy it now.
    */
    DclOsThreadAttrDestroy(pAttr);
    pAttr = NULL;

    DclPrintf("      %u threads competing for %u ms access to a resource for %u iterations\n",
        BLOCKING_THREADS, BLOCKING_MS, BLOCKING_ITERATIONS);

    /*  Set the threads loose...
    */
    DclOsAtomic32Exchange(&TestInf.ulAtomicTestState, TEST_RUNNING);

    DclTimerSet(&timer, BLOCKING_TOTAL_MAX_MS);

    while(TRUE)
    {
        unsigned nDoneCount = 0;
        
        for(nCurThread = 0; nCurThread < nThreads; nCurThread++)
        {
            if(DclOsAtomic32Retrieve(&bti[nCurThread].ulAtomicThreadState) == THRD_DONE)
                nDoneCount++;
        }

        if(nDoneCount == nThreads)
        {
            DclPrintf("      All threads completed normally\n");
            dclStat = DCLSTAT_SUCCESS;
            break;
        }

        if(DclTimerExpired(&timer))
        {
            DclPrintf("NOTE: One or more threads failed to finish before %u ms elapsed\n", BLOCKING_TOTAL_MAX_MS);
            dclStat = DCLSTAT_CURRENTLINE;
            break;
        }

        DclOsSleep(200);
    }

    ulMS = DclTimerElapsed(&timer);

    DclPrintf("      Test required %lU ms to complete (\"perfect\" results would be %u ms)\n",
        ulMS, BLOCKING_TOTAL_TEST_MS);

    DclPrintf("                                                               Max US    Max US  \n");
    DclPrintf("                 Handle        ID      StatusCode   Counter    Acquire  ReAcquire\n");
    
    for(nCurThread = 0; nCurThread < nThreads; nCurThread++)
    {  
        DclPrintf("      THREAD%u  %lX  %lX  %lX  %8lU %10lU %10lU\n", 
            nCurThread,
            bti[nCurThread].hThread,
            bti[nCurThread].idThread,
            bti[nCurThread].ThreadStatusCode,
            bti[nCurThread].ulCounter,
            bti[nCurThread].ulMaxAcquireUS,
            bti[nCurThread].ulMaxReacquireUS);
    }

  BlockingCleanup:
    
    for(nCurThread = 0; nCurThread < nThreads; nCurThread++)
    {
        DCLSTATUS   dclStat2;
        
        DCLPRINTF(1, ("      Destroying thread %u\n", nCurThread));

        dclStat2 = DclOsThreadDestroy(bti[nCurThread].hThread);
        if(dclStat2 != DCLSTAT_SUCCESS)
        {
            DclPrintf("Error %lX attempting to destroy thread %u (hThread=%lX)\n",
                dclStat, nCurThread, bti[nCurThread].hThread);

            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = dclStat2;

            break;
        }
    }

    if(pAttr)
        DclOsThreadAttrDestroy(pAttr);

    if(TestInf.pMutex)
        DclMutexDestroy(TestInf.pMutex);

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: TestMutexScope()

    Parameters:

    Return Value:
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS TestMutexScope(void)
{
    DCLSTATUS           dclStat;
    DCLSTATUS           dclStat2;
    DCLTHREADATTR      *pAttr = NULL;
    SCOPETESTINFO       sti = {0};
    DCLTHREADHANDLE     hThread = NULL;

    DclPrintf("    Testing Mutex Scope...\n");

    sti.pMutex = DclMutexCreate("DLTMUTA");
    if(!sti.pMutex)
        return DCLSTAT_CURRENTLINE;

    pAttr = DclOsThreadAttrCreate();
    if(!pAttr)
    {
        dclStat = DCLSTAT_THREADATTRCREATEFAILED;
        goto ScopeCleanup;
    }

    dclStat = DclOsThreadCreate(&hThread, "DLTTHDS", pAttr, ScopeThread, &sti);
    if(dclStat != DCLSTAT_SUCCESS)
        goto ScopeCleanup;

    /*  The "attr" object was only needed temporarily while the
        thread was being created.  We can destroy it now.
    */
    DclOsThreadAttrDestroy(pAttr);
    pAttr = NULL;

    DclPrintf("      Release a mutex acquired by a different thread\n",
        BLOCKING_THREADS, BLOCKING_MS, BLOCKING_ITERATIONS);

    /*  Sleep until ScopeThread() has acquired the mutex and gone into
        the "waiting" state.
    */        
    dclStat = DclSleepLock(&sti.ulAtomicThreadState, THRD_WAITING, THRD_WAITING, 2000, 10);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("SleepLockA failed with status %lX!\n", dclStat);
        goto ScopeCleanup;
    }
    
    if(!DclMutexRelease(sti.pMutex))
    {
        DclPrintf("      Failed -- releasing a mutex owned by another thread is not allowed\n");
    }
    else
    {
        DclPrintf("      Passed - releasing a mutex owned by another thread is allowed\n");
    }

    DclOsAtomic32Exchange(&sti.ulAtomicThreadState, THRD_RUNNING);

    /*  Moving to the DONE state tells ScopeThread() to cleanup and exit.
    */ 
    dclStat = DclSleepLock(&sti.ulAtomicThreadState, THRD_DONE, THRD_DONE, 2000, 10);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("SleepLockB failed with status %lX!\n", dclStat);
        goto ScopeCleanup;
    }

    dclStat = sti.dclStat;

  ScopeCleanup:

    if(pAttr)
        DclOsThreadAttrDestroy(pAttr);

    if(sti.pMutex)
        DclMutexDestroy(sti.pMutex);

    dclStat2 = DclOsThreadDestroy(hThread);
    if(dclStat2 != DCLSTAT_SUCCESS)
    {
        DclPrintf("Error %lX attempting to destroy thread (hThread=%lX)\n",
            dclStat, hThread);

        if(dclStat == DCLSTAT_SUCCESS)
            dclStat = dclStat2;
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: CompeteForResource()

    Parameters:

    Return Value:
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS CompeteForResource(
    BLOCKINGTHREADINFO *pBTI)
{
    unsigned            nn;
    
    for(nn = 1; nn <= BLOCKING_ITERATIONS; nn++)
    {
        D_UINT32        ulUS;
        DCLTIMER        timer;
        unsigned        nAcquired = 0;
        unsigned        nCmpCount = 0;
        DCLTIMESTAMP    ts;
        
        ts = DclTimeStamp();

        /*  Claim initial ownership of the resource at the beginning of
            each of the iterations, and copy our unique string into the
            shared buffer.
        */            
        if(!DclMutexAcquire(pBTI->pTestInf->pMutex))
            return DCLSTAT_CURRENTLINE;

        nAcquired++;

        ulUS = DclTimePassedUS(ts);

        if(ulUS > pBTI->ulMaxAcquireUS)
            pBTI->ulMaxAcquireUS = ulUS;
                
        DclStrNCpy(pBTI->pTestInf->szSharedResource, pBTI->szString, sizeof(pBTI->pTestInf->szSharedResource));

        /*  For the next BLOCKING_MS ms, do the following:
            1) Continually check the string for corruption
            2) Recursively re-acquire the mutex
            3) Periodically release a recursively acquired mutex, but don't
               ever fully release it.
            4) Keep stats on the performance.
        */
        DclTimerSet(&timer, BLOCKING_MS);

        while(!DclTimerExpired(&timer))
        {
            /*  Every 30th compare, release one of the recursively acquired
                instances we own.
            */
            if((nAcquired > 1) && (!(nCmpCount % 30)))
            {
                if(!DclMutexRelease(pBTI->pTestInf->pMutex))
                    return DCLSTAT_CURRENTLINE;

                nAcquired--;                
            }

            /*  Check for corruption
            */
            if(DclStrCmp(pBTI->pTestInf->szSharedResource, pBTI->szString) != 0)
                return DCLSTAT_CURRENTLINE;

            nCmpCount++;

            /*  Every 10th compare, recursively re-acquire the mutex
                we already own.
            */
            if(!(nCmpCount % 10))
            {
                ts = DclTimeStamp();
                
                if(!DclMutexAcquire(pBTI->pTestInf->pMutex))
                    return DCLSTAT_CURRENTLINE;

                ulUS = DclTimePassedUS(ts);

                if(ulUS > pBTI->ulMaxReacquireUS)
                    pBTI->ulMaxReacquireUS = ulUS;
                
                nAcquired++;                
            }
        }

        pBTI->ulCounter++;

        /*  Release all instances of the mutex to let another thread run
        */
        while(nAcquired)
        {
            if(!DclMutexRelease(pBTI->pTestInf->pMutex))
                return DCLSTAT_CURRENTLINE;

            nAcquired--;
        }
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: ThreadFunc()

    Parameters:

    Returns:
-------------------------------------------------------------------*/
static void * ThreadFunc(
    void               *pArgs)
{
    BLOCKINGTHREADINFO *pBTI = pArgs;

    DclAssertWritePtr(pBTI, sizeof(*pBTI));

    pBTI->idThread = DclOsThreadID();

    DclOsAtomic32Exchange(&pBTI->ulAtomicThreadState, THRD_WAITING);

    /*  Sleep while we are still in the "initializing" state, so that all
        the threads can start counting at the same time (more or less).
    */
    while(DclOsAtomic32Retrieve(&pBTI->pTestInf->ulAtomicTestState) == TEST_INITTING)
        DclOsSleep(1);

    if(DclOsAtomic32Retrieve(&pBTI->pTestInf->ulAtomicTestState) == TEST_RUNNING)
    {
        DclOsAtomic32Exchange(&pBTI->ulAtomicThreadState, THRD_RUNNING);

        pBTI->ThreadStatusCode = CompeteForResource(pBTI);
    }
    
    DclOsAtomic32Exchange(&pBTI->ulAtomicThreadState, THRD_DONE);

    return NULL;
}


/*-------------------------------------------------------------------
    Local: ScopeThread()

    Parameters:

    Returns:
-------------------------------------------------------------------*/
static void * ScopeThread(
    void               *pArgs)
{
    SCOPETESTINFO      *pSTI = pArgs;

    DclAssertWritePtr(pSTI, sizeof(*pSTI));

    if(!DclMutexAcquire(pSTI->pMutex))
    {
        pSTI->dclStat = DCLSTAT_CURRENTLINE;
        return NULL;
    }

    DclOsAtomic32Exchange(&pSTI->ulAtomicThreadState, THRD_WAITING);

    pSTI->dclStat = DclSleepLock(&pSTI->ulAtomicThreadState, THRD_RUNNING, THRD_RUNNING, 2000, 10);
    if(pSTI->dclStat == DCLSTAT_SUCCESS)
    {
        if(!DclMutexRelease(pSTI->pMutex))
        {
            DclPrintf("ScopeThread() mutex release failed\n");
        }
    }
    
    DclOsAtomic32Exchange(&pSTI->ulAtomicThreadState, THRD_DONE);
    
    return NULL;   
}

#endif  /* DCL_OSFEATURE_THREADS */


