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

    This module contains code for testing the OS Threads abstraction.

    ToDo: Finalized or remove TLS stuff as the case may be.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltthread.c $
    Revision 1.6  2010/05/22 21:13:20Z  garyp
    Partial merge from the v2.6 branch.  Cleaned up the logic used to
    detect the test quitting.
    Revision 1.5  2010/01/23 16:45:49Z  garyp
    Fixed to avoid "possible loss of data" warnings (no actual loss of data).
    Revision 1.4  2009/07/01 21:16:25Z  garyp
    Initialize a local to satify a picky compiler.  Updated debug code.
    Revision 1.3  2009/06/28 03:59:00Z  garyp
    Commented out the use of some disabled functionality.
    Revision 1.2  2009/06/28 02:46:14Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.2  2008/12/14 23:02:23Z  garyp
    Updated to use atomic variables to track the test and thread states.
    Revision 1.1  2008/11/27 22:19:26Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dltools.h>
#include "dltests.h"

/*
#include <dltls.h>
#include <dltlstemp.h>
*/

#if DCL_OSFEATURE_THREADS

/*  Set the default number of threads to test to be the same as the
    number of unique priority levels.  This could still be limited
    at run-time by DCL_MAX_THREADS.
*/
#define TEST_PRI_LEVELS  (DCL_THREADPRIORITY_ENUMHIGHLIMIT - (DCL_THREADPRIORITY_ENUMLOWLIMIT+1))
#define TEST_MAX_THREADS (TEST_PRI_LEVELS)
#define TEST_SECS           (10)

#define TEST_INITTING   (0)
#define TEST_RUNNING    (1)
#define TEST_QUITTING   (2) /* Waiting for all threads to be at THRD_DONE */
#define TEST_EXITING    (3)

#define THRD_INITTING   (0)
#define THRD_WAITING    (1) /* Waiting for test state != TEST_INITTING  */
#define THRD_RUNNING    (2)
#define THRD_DONE       (3) /* Thread is about to exit */


typedef struct
{
    unsigned                    nThreadNum;
    struct sDLTTHREADTESTINFO  *pTTI;
    unsigned                    nPriority;
    D_ATOMIC32                  ulThreadState;
    D_ATOMIC32                  ulPaused;
    volatile D_UINT32           ulCounter;
    volatile DCLTHREADID        ThreadID;
    volatile DCLSTATUS          ThreadStatusCode;
    volatile D_UINT32           ulTLSAllocs;
    volatile D_UINT32           ulTLSReallocs;
    volatile D_UINT32           ulTLSFrees;
    volatile D_UINT32           ulTLSVerifies;
} DLTTHREADINFO;

typedef struct sDLTTHREADTESTINFO
{
    unsigned            fSleep : 1;
/*  unsigned            fTestTLS : 1; */
    D_ATOMIC32          ulTestState;
    DCLTHREADHANDLE     ahThread[DCL_MAX_THREADS];
    DLTTHREADINFO       aTI[TEST_MAX_THREADS];
} DLTTHREADTESTINFO;


static DCLSTATUS    TestBasicFunctionality(void);
static DCLSTATUS    TestPriorityWithSleep(void);
static DCLSTATUS    TestPriorityWithoutSleep(void);
static void *       ThreadFunc(void *pArgs);
static DCLSTATUS    GenericThreadRun(DLTTHREADTESTINFO *pTTI, unsigned nThreads);
static DCLSTATUS    GenericThreadCleanup(DLTTHREADTESTINFO *pTTI, unsigned nThreads);
static DCLSTATUS    DisplayThreadInfo(D_UINT32 ulElapsedMS, unsigned nMaxThreads, DLTTHREADTESTINFO *pTTI);
static const char * FormatThreadState(D_UINT32 ulState);

/*
static void *       ThreadFuncTLS(void *pArgs);
static void *       ThreadFuncTLSTemp(void *pArgs);
static DCLSTATUS    TestTLS(void);
static DCLSTATUS    TestTLSTemp(void);
static DCLSTATUS    DisplayThreadTLSInfo(D_UINT32 ulElapsedMS, unsigned nMaxThreads, DLTTHREADTESTINFO *pTTI);
*/


#endif  /* DCL_OSFEATURE_THREADS */


/*-------------------------------------------------------------------
    Public: DclTestThreads()

    This function invokes the unit tests for excercising the thread
    abstraction.

    Parameters:

    Returns:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclTestThreads(void)
{
    DCLSTATUS       dclStat;
    DCLTHREADID     tid;

    DclPrintf("Testing Threads...\n");

    tid = DclOsThreadID();

    if(tid == DCL_INVALID_THREADID)
    {
        DclPrintf("DclOsThreadID() returned an invalid thread ID (%lX)", tid);
        dclStat = DCLSTAT_THREADINVALIDID;
        goto TestCleanup;
    }

  #if !DCL_OSFEATURE_THREADS
    DclPrintf("  The OS abstraction is configured with DCL_OSFEATURE_THREADS set to FALSE\n");

    dclStat = DCLSTAT_SUCCESS;

  #else

    /*  To test:
            ThreadTerminate()
            ThreadWait()
            ThreadSuspend/Resume()
            Terminate while suspended
            Priority inversion protection
    */

    dclStat = TestBasicFunctionality();
    if(dclStat != DCLSTAT_SUCCESS)
        goto TestCleanup;

    dclStat = TestPriorityWithSleep();
    if(dclStat != DCLSTAT_SUCCESS)
        goto TestCleanup;

    dclStat = TestPriorityWithoutSleep();
    if(dclStat != DCLSTAT_SUCCESS)
        goto TestCleanup;

/* Disabled until TLS stuff is finalized

    dclStat = TestTLS();
    if(dclStat != DCLSTAT_SUCCESS)
        goto TestCleanup;

    dclStat = TestTLSTemp();
    if(dclStat != DCLSTAT_SUCCESS)
        goto TestCleanup;
*/

  #endif

  TestCleanup:
    if(dclStat == DCLSTAT_SUCCESS)
        DclPrintf("    OK\n");
    else
        DclPrintf("    FAILED\n");

    return dclStat;
}


#if DCL_OSFEATURE_THREADS

/*-------------------------------------------------------------------
    Local: TestBasicFunctionality()

    This function invokes the unit tests for exercising the thread
    abstraction.

    Parameters:

    Returns:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestBasicFunctionality(void)
{
    DCLSTATUS           dclStat;
    DCLSTATUS           cleanupStat;
    unsigned            nCurThread;
    DLTTHREADTESTINFO   tti;
    unsigned            nMaxThreads = DCLMIN(TEST_MAX_THREADS, DCL_MAX_THREADS);

    DclPrintf("  Testing Basic Threading Functionality...\n");
    DclPrintf("    Creating %u threads of equal priority, each incrementing a counter\n", nMaxThreads);
    DclPrintf("    and sleeping for 1 ms after every 100000 counts.\n");

    DclMemSet(&tti, 0, sizeof(tti));

/*    tti.fTestTLS = FALSE; */
    tti.fSleep = TRUE;

    for(nCurThread=0; nCurThread<nMaxThreads; nCurThread++)
    {
        DCLTHREADATTR  *pAttr;
        char            achName[DCL_THREADNAMELEN];

        tti.aTI[nCurThread].pTTI = &tti;
        tti.aTI[nCurThread].nThreadNum = nCurThread;

        pAttr = DclOsThreadAttrCreate();
        if(!pAttr)
        {
            dclStat = DCLSTAT_THREADATTRCREATEFAILED;
            goto ThreadCreateCleanup;
        }

        tti.aTI[nCurThread].nPriority = DCL_THREADPRIORITY_NORMAL;

        DclSNPrintf(achName, sizeof(achName), "DLTTH%u", nCurThread);

        DclPrintf("    Creating thread %u\n", nCurThread);

        dclStat = DclOsThreadCreate(&tti.ahThread[nCurThread], achName, pAttr, ThreadFunc, &tti.aTI[nCurThread]);
        if(dclStat != DCLSTAT_SUCCESS)
            goto ThreadCreateCleanup;

        /*  The "attr" object was only needed temporarily while the
            thread was being created.  We can destroy it now.
        */
        DclOsThreadAttrDestroy(pAttr);
        pAttr = NULL;

        continue;

      ThreadCreateCleanup:
        if(pAttr)
            DclOsThreadAttrDestroy(pAttr);

        /*  ToDo: Add cleanup logic to try to terminate/destroy the threads
        */
        return dclStat;
    }

    dclStat = GenericThreadRun(&tti, nMaxThreads);

    DclOsAtomic32Exchange(&tti.ulTestState, TEST_QUITTING);

    cleanupStat = GenericThreadCleanup(&tti, nMaxThreads);

    /*  Preserve any original error code we might have
    */
    if(dclStat == DCLSTAT_SUCCESS)
        dclStat = cleanupStat;

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: TestPriorityWithSleep()

    This function invokes the unit tests for exercising the thread
    abstraction.

    Parameters:

    Returns:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestPriorityWithSleep(void)
{
    DCLSTATUS           dclStat;
    DCLSTATUS           cleanupStat;
    unsigned            nCurThread;
    DLTTHREADTESTINFO   tti;
    unsigned            nMaxThreads = DCLMIN(TEST_MAX_THREADS, DCL_MAX_THREADS);
    unsigned            nPriority = DCL_THREADPRIORITY_ENUMLOWLIMIT + 1;

    DclPrintf("  Testing Prioritized Threading Functionality with sleeps...\n");
    DclPrintf("    Creating %u threads of varying priorities, each incrementing a counter\n", nMaxThreads);
    DclPrintf("    and sleeping for 1 ms after every 100000 counts.\n");

    DclMemSet(&tti, 0, sizeof(tti));

/*    tti.fTestTLS = FALSE; */
    tti.fSleep = TRUE;

    /*  If the max threads value is not large enough (due to DCL_MAX_THREADS)
        to allow one thread per priority level, adjust the start priority
        level such that 'n' number of the higher priority level (numerically
        lower) are skipped.
    */
    if(nMaxThreads != TEST_PRI_LEVELS)
    {
        DclAssert(TEST_PRI_LEVELS > nMaxThreads);

        nPriority += TEST_PRI_LEVELS - nMaxThreads;

        DclPrintf("DCL_MAX_THREADS limits the thread test to %u threads (would prefer at least %u)\n",
            DCL_MAX_THREADS, TEST_MAX_THREADS);
    }

    nPriority += (nMaxThreads - 1);

    for(nCurThread=0; nCurThread<nMaxThreads; nCurThread++)
    {
        DCLTHREADATTR  *pAttr;
        char            achName[DCL_THREADNAMELEN];

        tti.aTI[nCurThread].pTTI = &tti;
        tti.aTI[nCurThread].nThreadNum = nCurThread;

        pAttr = DclOsThreadAttrCreate();
        if(!pAttr)
        {
            dclStat = DCLSTAT_THREADATTRCREATEFAILED;
            goto ThreadCreateCleanup;
        }

        /*  Cast is safe due to the range of thread priority values.
            ToDo: DclOsThreadAttrSetPriority() should be changed to 
            take an "unsigned" thread priority, similar to other 
            functions.
        */            
        dclStat = DclOsThreadAttrSetPriority(pAttr, (D_UINT16)nPriority);
        if(dclStat != DCLSTAT_SUCCESS)
            goto ThreadCreateCleanup;

        tti.aTI[nCurThread].nPriority = nPriority;

        DclSNPrintf(achName, sizeof(achName), "DLTTH%u", nCurThread);

        DclPrintf("    Creating thread %u with priority %u\n", nCurThread, nPriority);

        dclStat = DclOsThreadCreate(&tti.ahThread[nCurThread], achName, pAttr, ThreadFunc, &tti.aTI[nCurThread]);
        if(dclStat != DCLSTAT_SUCCESS)
            goto ThreadCreateCleanup;

        /*  The "attr" object was only needed temporarily while the
            thread was being created.  We can destroy it now.
        */
        DclOsThreadAttrDestroy(pAttr);
        pAttr = NULL;

        nPriority--;
        continue;

      ThreadCreateCleanup:
        if(pAttr)
            DclOsThreadAttrDestroy(pAttr);

        /*  ToDo: Add cleanup logic to try to terminate/destroy the threads
        */
        return dclStat;
    }

    dclStat = GenericThreadRun(&tti, nMaxThreads);

    DclOsAtomic32Exchange(&tti.ulTestState, TEST_QUITTING);

    cleanupStat = GenericThreadCleanup(&tti, nMaxThreads);

    /*  Preserve any original error code we might have
    */
    if(dclStat == DCLSTAT_SUCCESS)
        dclStat = cleanupStat;

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: TestPriorityWithoutSleep()

    This function invokes the unit tests for exercising the thread
    abstraction.

    Parameters:

    Returns:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestPriorityWithoutSleep(void)
{
    DCLSTATUS           dclStat;
    DCLSTATUS           cleanupStat;
    unsigned            nCurThread;
    DLTTHREADTESTINFO   tti;
    unsigned            nMaxThreads = DCLMIN(TEST_MAX_THREADS, DCL_MAX_THREADS);
    unsigned            nPriority = DCL_THREADPRIORITY_ENUMLOWLIMIT + 1;

    DclPrintf("  Testing Prioritized Threading Functionality without sleeps...\n");
    DclPrintf("    Creating %u threads of varying priorities, each incrementing a counter.\n", nMaxThreads);

    DclMemSet(&tti, 0, sizeof(tti));

/*    tti.fTestTLS = FALSE; */
    tti.fSleep = FALSE;

    /*  If the max threads value is not large enough (due to DCL_MAX_THREADS)
        to allow one thread per priority level, adjust the start priority
        level such that 'n' number of the higher priority level (numerically
        lower) are skipped.
    */
    if(nMaxThreads != TEST_PRI_LEVELS)
    {
        DclAssert(TEST_PRI_LEVELS > nMaxThreads);

        nPriority += TEST_PRI_LEVELS - nMaxThreads;

        DclPrintf("DCL_MAX_THREADS limits the thread test to %u threads (would prefer at least %u)\n",
            DCL_MAX_THREADS, TEST_MAX_THREADS);
    }

    nPriority += (nMaxThreads - 1);

    for(nCurThread=0; nCurThread<nMaxThreads; nCurThread++)
    {
        DCLTHREADATTR  *pAttr;
        char            achName[DCL_THREADNAMELEN];

        tti.aTI[nCurThread].pTTI = &tti;
        tti.aTI[nCurThread].nThreadNum = nCurThread;

        pAttr = DclOsThreadAttrCreate();
        if(!pAttr)
        {
            dclStat = DCLSTAT_THREADATTRCREATEFAILED;
            goto ThreadCreateCleanup;
        }

        /*  Cast is safe due to the range of thread priority values.
            ToDo: DclOsThreadAttrSetPriority() should be changed to 
            take an "unsigned" thread priority, similar to other 
            functions.
        */            
        dclStat = DclOsThreadAttrSetPriority(pAttr, (D_UINT16)nPriority);
        if(dclStat != DCLSTAT_SUCCESS)
            goto ThreadCreateCleanup;

        tti.aTI[nCurThread].nPriority = nPriority;

        DclSNPrintf(achName, sizeof(achName), "DLTTH%u", nCurThread);

        DclPrintf("    Creating thread %u with priority %u\n", nCurThread, nPriority);

        dclStat = DclOsThreadCreate(&tti.ahThread[nCurThread], achName, pAttr, ThreadFunc, &tti.aTI[nCurThread]);
        if(dclStat != DCLSTAT_SUCCESS)
            goto ThreadCreateCleanup;

        /*  The "attr" object was only needed temporarily while the
            thread was being created.  We can destroy it now.
        */
        DclOsThreadAttrDestroy(pAttr);
        pAttr = NULL;

        nPriority--;
        continue;

      ThreadCreateCleanup:
        if(pAttr)
            DclOsThreadAttrDestroy(pAttr);

        /*  ToDo: Add cleanup logic to try to terminate/destroy the threads
        */
        return dclStat;
    }

    dclStat = GenericThreadRun(&tti, nMaxThreads);

    DclOsAtomic32Exchange(&tti.ulTestState, TEST_QUITTING);

    cleanupStat = GenericThreadCleanup(&tti, nMaxThreads);

    /*  Preserve any original error code we might have
    */
    if(dclStat == DCLSTAT_SUCCESS)
        dclStat = cleanupStat;

    return dclStat;
}


#if 0

/*-------------------------------------------------------------------
    Local: TestTLS()

    Parameters:

    Returns:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestTLS(void)
{
    DCLSTATUS           dclStat;
    DCLSTATUS           cleanupStat;
    unsigned            nCurThread;
    DLTTHREADTESTINFO   tti;
    unsigned            nMaxThreads = DCLMIN(TEST_MAX_THREADS, DCL_MAX_THREADS);

    DclPrintf("  Testing Thread Local Storage...\n");
    DclPrintf("    Creating %u threads of equal priority\n", nMaxThreads);

    DclMemSet(&tti, 0, sizeof(tti));

    tti.fTestTLS = TRUE;
    tti.fSleep = FALSE;

    for(nCurThread=0; nCurThread<nMaxThreads; nCurThread++)
    {
        DCLTHREADATTR  *pAttr;
        char            achName[DCL_THREADNAMELEN];

        tti.aTI[nCurThread].pTTI = &tti;
        tti.aTI[nCurThread].nThreadNum = nCurThread;

        pAttr = DclOsThreadAttrCreate();
        if(!pAttr)
        {
            dclStat = DCLSTAT_THREADATTRCREATEFAILED;
            goto ThreadCreateCleanup;
        }

        tti.aTI[nCurThread].nPriority = DCL_THREADPRIORITY_NORMAL;

        DclSNPrintf(achName, sizeof(achName), "DLTTH%u", nCurThread);

        DclPrintf("    Creating thread %u\n", nCurThread);

        dclStat = DclOsThreadCreate(&tti.ahThread[nCurThread], achName, pAttr, ThreadFuncTLS, &tti.aTI[nCurThread]);
        if(dclStat != DCLSTAT_SUCCESS)
            goto ThreadCreateCleanup;

        /*  The "attr" object was only needed temporarily while the
            thread was being created.  We can destroy it now.
        */
        DclOsThreadAttrDestroy(pAttr);
        pAttr = NULL;

        continue;

      ThreadCreateCleanup:
        if(pAttr)
            DclOsThreadAttrDestroy(pAttr);

        /*  ToDo: Add cleanup logic to try to terminate/destroy the threads
        */
        return dclStat;
    }

    dclStat = GenericThreadRun(&tti, nMaxThreads);

    DclOsAtomic32Exchange(&tti.ulTestState, TEST_QUITTING);

    cleanupStat = GenericThreadCleanup(&tti, nMaxThreads);

    /*  Preserve any original error code we might have
    */
    if(dclStat == DCLSTAT_SUCCESS)
        dclStat = cleanupStat;

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: TestTLSTemp()

    Parameters:

    Returns:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TestTLSTemp(void)
{
    DCLSTATUS           dclStat;
    DCLSTATUS           cleanupStat;
    unsigned            nCurThread;
    DLTTHREADTESTINFO   tti;
    unsigned            nMaxThreads = DCLMIN(TEST_MAX_THREADS, DCL_MAX_THREADS);

    DclPrintf("  Testing Thread Local Temporary Storage...\n");
    DclPrintf("    Creating %u threads of equal priority\n", nMaxThreads);

    DclMemSet(&tti, 0, sizeof(tti));

    tti.fTestTLS = TRUE;
    tti.fSleep = FALSE;

    for(nCurThread=0; nCurThread<nMaxThreads; nCurThread++)
    {
        DCLTHREADATTR  *pAttr;
        char            achName[DCL_THREADNAMELEN];

        tti.aTI[nCurThread].pTTI = &tti;
        tti.aTI[nCurThread].nThreadNum = nCurThread;

        pAttr = DclOsThreadAttrCreate();
        if(!pAttr)
        {
            dclStat = DCLSTAT_THREADATTRCREATEFAILED;
            goto ThreadCreateCleanup;
        }

        tti.aTI[nCurThread].nPriority = DCL_THREADPRIORITY_NORMAL;

        DclSNPrintf(achName, sizeof(achName), "DLTTH%u", nCurThread);

        DclPrintf("    Creating thread %u\n", nCurThread);

        dclStat = DclOsThreadCreate(&tti.ahThread[nCurThread], achName, pAttr, ThreadFuncTLSTemp, &tti.aTI[nCurThread]);
        if(dclStat != DCLSTAT_SUCCESS)
            goto ThreadCreateCleanup;

        /*  The "attr" object was only needed temporarily while the
            thread was being created.  We can destroy it now.
        */
        DclOsThreadAttrDestroy(pAttr);
        pAttr = NULL;

        continue;

      ThreadCreateCleanup:
        if(pAttr)
            DclOsThreadAttrDestroy(pAttr);

        /*  ToDo: Add cleanup logic to try to terminate/destroy the threads
        */
        return dclStat;
    }

    dclStat = GenericThreadRun(&tti, nMaxThreads);

    DclOsAtomic32Exchange(&tti.ulTestState, TEST_QUITTING);

    cleanupStat = GenericThreadCleanup(&tti, nMaxThreads);

    /*  Preserve any original error code we might have
    */
    if(dclStat == DCLSTAT_SUCCESS)
        dclStat = cleanupStat;

    return dclStat;
}

#endif


/*-------------------------------------------------------------------
    Local: ThreadFunc()

    Parameters:

    Returns:
-------------------------------------------------------------------*/
static void * ThreadFunc(
    void               *pArgs)
{
    DLTTHREADINFO      *pTI = pArgs;
    DLTTHREADTESTINFO  *pTTI = pTI->pTTI;
    DCLTIMER            timer;

    DclAssertWritePtr(pTI, sizeof(*pTI));

    pTI->ThreadID = DclOsThreadID();

    DclOsAtomic32Exchange(&pTI->ulThreadState, THRD_WAITING);

    /*  Sleep while we are still in the "initializing" state, so that all
        the threads can start counting at the same time (more or less).
    */
    while(DclOsAtomic32Retrieve(&pTTI->ulTestState) == TEST_INITTING)
        DclOsSleep(1);

    DclTimerSet(&timer, 1000);

    DclOsAtomic32Exchange(&pTI->ulThreadState, THRD_RUNNING);

    while(DclOsAtomic32Retrieve(&pTTI->ulTestState) < TEST_QUITTING)
    {
        pTI->ulCounter++;

        if(DclTimerExpired(&timer))
        {
            DclOsAtomic32Exchange(&pTI->ulPaused, TRUE);

            while((DclOsAtomic32Retrieve(&pTI->ulPaused) == TRUE) &&
                DclOsAtomic32Retrieve(&pTTI->ulTestState) < TEST_QUITTING)
            {
                DclOsSleep(1);
            }

            DclTimerSet(&timer, 1000);
        }
        else if((pTTI->fSleep) && (!pTI->ulCounter % 100000))
        {
            DclOsSleep(1);
        }
    }

    DclOsAtomic32Exchange(&pTI->ulThreadState, THRD_DONE);

    return NULL;
}


#if 0

/*-------------------------------------------------------------------
    Local: ThreadFuncTLS()

    Parameters:

    Returns:
-------------------------------------------------------------------*/
static void * ThreadFuncTLS(
    void               *pArgs)
{
    DLTTHREADINFO      *pTI = pArgs;
    DLTTHREADTESTINFO  *pTTI = pTI->pTTI;
    DCLTLSDATA          (tls, DCL_MAX_THREADS, sizeof(D_UINT32));
    DCLTIMER            timer;
    D_UINT32           *pulTLSCounter = NULL;
    D_UINT32            ulTLSNextStateChange = 1;

    pTI->ThreadID = DclOsThreadID();

    DclOsAtomic32Exchange(&pTI->ulThreadState, THRD_WAITING);

    /*  Sleep while we are still in the "initializing" state, so that all
        the threads can start counting at the same time (more or less).
    */
    while(DclOsAtomic32Retrieve(&pTTI->ulTestState) == TEST_INITTING)
        DclOsSleep(1);

    DclTimerSet(&timer, 1000);

    DclOsAtomic32Exchange(&pTI->ulThreadState, THRD_RUNNING);

    while(DclOsAtomic32Retrieve(&pTTI->ulTestState) < TEST_QUITTING)
    {
        if(pulTLSCounter)
        {
            if(*pulTLSCounter != pTI->ulCounter)
            {
                DclPrintf("FAILED: TLS data corrupted in thread %u, Expected=%lX Got=%lX Verifies=%lU\n",
                    pTI->nThreadNum,
                    pTI->ulCounter,
                    *pulTLSCounter,
                    pTI->ulTLSVerifies);

              #if D_DEBUG
                DclTlsDump(&tls);
              #endif

                pTI->ThreadStatusCode = DCLSTAT_CURRENTLINE;
                break;
            }
            else
            {
                pTI->ulTLSVerifies++;
            }
        }

        pTI->ulCounter++;

        if(pulTLSCounter)
            *pulTLSCounter = pTI->ulCounter;

        if(DclTimerExpired(&timer))
        {
            DclOsAtomic32Exchange(&pTI->ulPaused, TRUE);

            while((DclOsAtomic32Retrieve(&pTI->ulPaused) == TRUE) &&
                DclOsAtomic32Retrieve(&pTTI->ulTestState) < TEST_QUITTING)
            {
                DclOsSleep(1);
            }

            DclTimerSet(&timer, 1000);
        }

        if(pTI->ulCounter == ulTLSNextStateChange)
        {
            if(pulTLSCounter)
            {
                /*  For every 5th time we would try to free the data, do
                    a realloc instead and make sure the same value is
                    returned.
                */
                if(!(pTI->ulCounter % 5))
                {
                    void   *pTmp;

                    pTmp = DclTlsAllocate(&tls);
                    if(pTmp != pulTLSCounter)
                    {
                        DclPrintf("FAILED: Realloc of the same TLS data for thread %u returned %P, expected %P\n",
                            pTI->nThreadNum, pTmp, pulTLSCounter);

                        pTI->ThreadStatusCode = DCLSTAT_CURRENTLINE;

                        break;
                    }

                    pTI->ulTLSReallocs++;
                }
                else
                {
                    DCLSTATUS dclStat = DclTlsFree(&tls, pulTLSCounter);
                    if(dclStat != DCLSTAT_SUCCESS)
                    {
                        DclPrintf("FAILED: Freeing TLS data in thread %u returned status %lX\n",
                            pTI->nThreadNum, dclStat);

                        pTI->ThreadStatusCode = dclStat;

                        break;
                    }

                    pulTLSCounter = NULL;

                    pTI->ulTLSFrees++;
                }
            }
            else
            {
                pulTLSCounter = DclTlsAllocate(&tls);
                if(!pulTLSCounter)
                {
                    DclPrintf("FAILED: Thread %u failed to allocate TLS\n", pTI->nThreadNum);
                    pTI->ThreadStatusCode = DCLSTAT_CURRENTLINE;
                    break;
                }

                *pulTLSCounter = pTI->ulCounter;

                pTI->ulTLSAllocs++;
            }

            ulTLSNextStateChange += (DclRand(NULL) % 100) + 1;
        }
    }

    DclOsAtomic32Exchange(&pTI->ulThreadState, THRD_DONE);

    return NULL;
}


/*-------------------------------------------------------------------
    Local: ThreadFuncTLSTemp()

    Parameters:

    Returns:
-------------------------------------------------------------------*/
static void * ThreadFuncTLSTemp(
    void               *pArgs)
{
    DLTTHREADINFO      *pTI = pArgs;
    DLTTHREADTESTINFO  *pTTI = pTI->pTTI;
    DCLTLSTEMPDATA      (tls, DCL_MAX_THREADS, sizeof(D_UINT32));
    DCLTIMER            timer;
    D_UINT32           *pulTLSCounter = NULL;
    D_UINT32            ulTLSNextStateChange = 1;

    pTI->ThreadID = DclOsThreadID();

    DclOsAtomic32Exchange(&pTI->ulThreadState, THRD_WAITING);

    /*  Sleep while we are still in the "initializing" state, so that all
        the threads can start counting at the same time (more or less).
    */
    while(DclOsAtomic32Retrieve(&pTTI->ulTestState) == TEST_INITTING)
        DclOsSleep(1);

    DclTimerSet(&timer, 1000);

    DclOsAtomic32Exchange(&pTI->ulThreadState, THRD_RUNNING);

    while(DclOsAtomic32Retrieve(&pTTI->ulTestState) < TEST_QUITTING)
    {
        if(pulTLSCounter)
        {
            if(*pulTLSCounter != pTI->ulCounter)
            {
                DclPrintf("FAILED: TLS temp data corrupted in thread %u, Expected=%lX Got=%lX Verifies=%lU\n",
                    pTI->nThreadNum,
                    pTI->ulCounter,
                    *pulTLSCounter,
                    pTI->ulTLSVerifies);

              #if D_DEBUG
                DclTlsTempDump(&tls);
              #endif

                pTI->ThreadStatusCode = DCLSTAT_CURRENTLINE;
                break;
            }
            else
            {
                pTI->ulTLSVerifies++;
            }
        }

        pTI->ulCounter++;

        if(pulTLSCounter)
            *pulTLSCounter = pTI->ulCounter;

        if(DclTimerExpired(&timer))
        {
            DclOsAtomic32Exchange(&pTI->ulPaused, TRUE);

            while((DclOsAtomic32Retrieve(&pTI->ulPaused) == TRUE) &&
                DclOsAtomic32Retrieve(&pTTI->ulTestState) < TEST_QUITTING)
            {
                DclOsSleep(1);
            }

            DclTimerSet(&timer, 1000);
        }

        if(pTI->ulCounter == ulTLSNextStateChange)
        {
            if(pulTLSCounter)
            {
                /*  For every 5th time we would try to free the data, do
                    a realloc instead and make sure the same value is
                    returned.
                */
                if(pTI->ulCounter & 1)
                {
                    void   *pTmp;

                    pTmp = DclTlsTempAllocate(&tls);
                    if(pTmp != pulTLSCounter)
                    {
                        DclPrintf("FAILED: Realloc of the same TLS temp data for thread %u returned %P, expected %P\n",
                            pTI->nThreadNum, pTmp, pulTLSCounter);

                        pTI->ThreadStatusCode = DCLSTAT_CURRENTLINE;

                        break;
                    }

                    pTI->ulTLSReallocs++;
                }
                else
                {
                    DCLSTATUS dclStat = DclTlsTempFree(&tls, pulTLSCounter);
                    if(dclStat != DCLSTAT_SUCCESS)
                    {
                        DclPrintf("FAILED: Freeing TLS temp data in thread %u returned status %lX\n",
                            pTI->nThreadNum, dclStat);

                        pTI->ThreadStatusCode = dclStat;

                        break;
                    }

                    pulTLSCounter = NULL;

                    pTI->ulTLSFrees++;
                }
            }
            else
            {
                pulTLSCounter = DclTlsTempAllocate(&tls);
                if(!pulTLSCounter)
                {
                    DclPrintf("FAILED: Thread %u failed to allocate TLS temp data\n", pTI->nThreadNum);
                    pTI->ThreadStatusCode = DCLSTAT_CURRENTLINE;
                    break;
                }

                *pulTLSCounter = pTI->ulCounter;

                pTI->ulTLSAllocs++;
            }

            ulTLSNextStateChange += (DclRand(NULL) % 10) + 1;
        }
    }

    DclOsAtomic32Exchange(&pTI->ulThreadState, THRD_DONE);

    return NULL;
}

#endif


/*-------------------------------------------------------------------
    Local: GenericThreadRun()

    Parameters:

    Returns:
-------------------------------------------------------------------*/
static DCLSTATUS GenericThreadRun(
    DLTTHREADTESTINFO  *pTTI,
    unsigned            nThreads)
{
    DCLSTATUS           dclStat = DCLSTAT_SUCCESS;
    DCLTIMER            timer;

    DclAssertWritePtr(pTTI, sizeof(*pTTI));
    DclAssert(nThreads);

    DclPrintf("    Syncing to display thread information every second for %u seconds\n", TEST_SECS);

    DclTimerSet(&timer, TEST_SECS*1000);

    while(TRUE)
    {
        unsigned    nCurThread;
        unsigned    fAllPaused = TRUE;

        for(nCurThread=0; nCurThread<nThreads; nCurThread++)
        {
            if(DclOsAtomic32Retrieve(&pTTI->aTI[nCurThread].ulPaused) != TRUE)
            {
                fAllPaused = FALSE;
                break;
            }
        }

        if(fAllPaused || (DclOsAtomic32Retrieve(&pTTI->ulTestState) == TEST_INITTING))
        {
            D_UINT32 ulElapsedMS = DclTimerElapsed(&timer);

/*            if(pTTI->fTestTLS)
                dclStat = DisplayThreadTLSInfo(ulElapsedMS, nThreads, pTTI);
            else  */
                dclStat = DisplayThreadInfo(ulElapsedMS, nThreads, pTTI);

            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Test failed -- attempting to clean up gracefully...\n");
                break;
            }

            if(DclTimerExpired(&timer))
                break;

            if(DclOsAtomic32Retrieve(&pTTI->ulTestState) == TEST_INITTING)
            {
                /*  If this is the first time through, set the threads loose...
                */
                DclOsAtomic32Exchange(&pTTI->ulTestState, TEST_RUNNING);
            }
            else
            {
                /*  On subsequent passes, reset the "paused" state for
                    all the threads and do the next interval.
                */
                for(nCurThread=0; nCurThread<nThreads; nCurThread++)
                    DclOsAtomic32Exchange(&pTTI->aTI[nCurThread].ulPaused, FALSE);
            }
        }

        DclOsSleep(1);
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: GenericThreadCleanup()

    Parameters:

    Returns:
-------------------------------------------------------------------*/
static DCLSTATUS GenericThreadCleanup(
    DLTTHREADTESTINFO  *pTTI,
    unsigned            nThreads)
{
    DCLSTATUS           dclStat = DCLSTAT_FAILURE; /* Init'ed for a picky compiler only */
    DCLTIMER            timer;
    D_UINT32            ulSeconds;
    unsigned            nCurThread;

    DclAssertWritePtr(pTTI, sizeof(*pTTI));
    DclAssert(nThreads);

    DclPrintf("    Test quitting: Polling for thread completion every second for %u seconds\n", TEST_SECS);

    /*  We have changed the test state to "quitting" by now, however some
        threads could still be paused.  Set them free one last time...
    */
    for(nCurThread=0; nCurThread<nThreads; nCurThread++)
        DclOsAtomic32Exchange(&pTTI->aTI[nCurThread].ulPaused, FALSE);

    DclTimerSet(&timer, TEST_SECS*1000);

    ulSeconds = D_UINT32_MAX;
    while(TRUE)
    {
        D_UINT32 ulElapsedMS = DclTimerElapsed(&timer);

        if(ulSeconds != ulElapsedMS / 1000)
        {
            unsigned nDoneCount = 0;

            ulSeconds = ulElapsedMS / 1000;

            DisplayThreadInfo(ulElapsedMS, nThreads, pTTI);

            for(nCurThread = nThreads - 1;
                nCurThread != UINT_MAX;
                nCurThread --)
            {
                if(DclOsAtomic32Retrieve(&pTTI->aTI[nCurThread].ulThreadState) == THRD_DONE)
                    nDoneCount++;
            }

            if(nDoneCount == nThreads)
            {
                DclPrintf("    All threads completed normally\n");
                break;
            }

            if(DclTimerExpired(&timer))
            {
                DclPrintf("NOTE: One or more threads failed to finish before %u seconds elapsed\n", TEST_SECS);
                break;
            }
        }

        DclOsSleep(100);
    }

    for(nCurThread = nThreads - 1;
        nCurThread != UINT_MAX;
        nCurThread --)
    {
        DclPrintf("    Destroying object for thread %u\n", nCurThread);

        dclStat = DclOsThreadDestroy(pTTI->ahThread[nCurThread]);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("Error %lX attempting to destroy thread %u (hThread=%lX)\n",
                dclStat, nCurThread, pTTI->ahThread[nCurThread]);

            break;
        }
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: DisplayThreadInfo()

    Parameters:

    Returns:
-------------------------------------------------------------------*/
static DCLSTATUS DisplayThreadInfo(
    D_UINT32            ulElapsedMS,
    unsigned            nMaxThreads,
    DLTTHREADTESTINFO  *pTTI)
{
    unsigned            nn;
    DCLSTATUS           dclStat = DCLSTAT_SUCCESS;

    DclAssertWritePtr(pTTI, sizeof(*pTTI));
    DclAssert(nMaxThreads);

    DclPrintf("    Secs:%2lU.%lU Num    ID    Pri  State     Counter   Status\n",
        ulElapsedMS / 1000, (ulElapsedMS % 1000) / 100);

    for(nn=0; nn<nMaxThreads; nn++)
    {
        DclPrintf("              %2u %lX %u %8s %10lU %lX\n",
            nn,
            pTTI->aTI[nn].ThreadID,
            pTTI->aTI[nn].nPriority,
            FormatThreadState(DclOsAtomic32Retrieve(&pTTI->aTI[nn].ulThreadState)),
            pTTI->aTI[nn].ulCounter,
            pTTI->aTI[nn].ThreadStatusCode);

        if(pTTI->aTI[nn].ThreadStatusCode != DCLSTAT_SUCCESS)
            dclStat = pTTI->aTI[nn].ThreadStatusCode;
    }

    return dclStat;
}


#if 0

/*-------------------------------------------------------------------
    Local: DisplayThreadTLSInfo()

    Parameters:

    Returns:
-------------------------------------------------------------------*/
static DCLSTATUS DisplayThreadTLSInfo(
    D_UINT32            ulElapsedMS,
    unsigned            nMaxThreads,
    DLTTHREADTESTINFO  *pTTI)
{
    unsigned            nn;
    DCLSTATUS           dclStat = DCLSTAT_SUCCESS;

    DclPrintf("    Secs:%2lU.%lU Num    ID    Pri  State     Counter TLS:Alloc   Realloc      Free    Verify   Status\n",
        ulElapsedMS / 1000, (ulElapsedMS % 1000) / 100);

    for(nn=0; nn<nMaxThreads; nn++)
    {
        DclPrintf("              %2u %lX %u %8s %10lU %9lU %9lU %9lU %9lU %lX\n",
            nn,
            pTTI->aTI[nn].ThreadID,
            pTTI->aTI[nn].nPriority,
            FormatThreadState(DclOsAtomic32Retrieve(&pTTI->aTI[nn].ulThreadState)),
            pTTI->aTI[nn].ulCounter,
            pTTI->aTI[nn].ulTLSAllocs,
            pTTI->aTI[nn].ulTLSReallocs,
            pTTI->aTI[nn].ulTLSFrees,
            pTTI->aTI[nn].ulTLSVerifies,
            pTTI->aTI[nn].ThreadStatusCode);

        if(pTTI->aTI[nn].ThreadStatusCode != DCLSTAT_SUCCESS)
            dclStat = pTTI->aTI[nn].ThreadStatusCode;
    }

    return dclStat;
}

#endif


/*-------------------------------------------------------------------
    Local: FormatThreadState()

    Parameters:

    Returns:
-------------------------------------------------------------------*/
static const char * FormatThreadState(
    D_UINT32    ulState)
{
    switch(ulState)
    {
        case THRD_INITTING:
            return "INITTING";
        case THRD_WAITING:
            return "WAITING";
        case THRD_RUNNING:
            return "RUNNING";
        case THRD_DONE:
            return "DONE";
        default:
            DclProductionError();
            return "??";
    }
}


#endif  /* DCL_OSFEATURE_THREADS */


