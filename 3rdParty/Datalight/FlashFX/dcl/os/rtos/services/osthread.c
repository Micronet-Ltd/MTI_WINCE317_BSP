/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2009 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

  In addition to civil penalties for infringement of copyright under appli-
  cable U.S. law, 17 U.S.C. 1204 provides criminal penalties for violation
  of (a) the restrictions on circumvention of copyright protection systems
  found in 17 U.S.C. 1201 and (b) the protections for the integrity of
  copyright management information found in 17 U.S.C. 1202.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  A SOURCE CODE AGREEMENT, NON-DISCLOSURE AGREEMENT, AND/OR SIMILAR BINDING
  CONTRACT BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    This module contains the default OS Services functions for managing
    threads.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify the
    make file to build the new module.

    NOTE:  This module contains a sample threading implementation for VxWorks
           that may be used for reference purposes in the development of
           threading functions for this RTOS.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osthread.c $
    Revision 1.8  2009/04/10 20:52:11Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.7  2007/11/03 23:31:30Z  Garyp
    Added the standard module header.
    Revision 1.6  2006/10/18 04:27:24Z  Garyp
    Modified so DclOsThreadID() is always included even if the threading
    features are turned off.
    Revision 1.5  2006/10/04 02:36:58Z  Garyp
    Updated to use DclMemAllocZero().
    Revision 1.4  2006/03/01 05:08:26Z  brandont
    Added stubbed version of DclOsThreadID.
    Revision 1.3  2006/02/16 22:38:38Z  Pauli
    Increased default stack size to 8Kb to avoid stack overflow.
    Revision 1.2  2006/02/09 23:21:54Z  Pauli
    Disabled VxWorks sample code that will not compile without VxWorks.
    Revision 1.1  2005/12/31 03:29:50Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Public: DclOsThreadID()

    Return the thread ID for the current thread.  A valid thread ID
    may never be zero.

    Note that this function must be implemented even if the
    DCL_OSFEATURE_THREADS setting is FALSE.  In an environment
    where threading is disabled or not supported, this function
    should be stubbed to return any legal (non-zero) value.

    Parameters:
        None.

    Return Value:
        Returns a DCLTHREADID value if successful, otherwise zero.
-------------------------------------------------------------------*/
DCLTHREADID DclOsThreadID(void)
{
    /*  Pretend we are returning a real thread ID value.
    */
    return 1;
}


#if DCL_OSFEATURE_THREADS


/*  Remove this condition to enable and implement the OS service routines in
    this module.
*/
#if 0


/*-------------------------------------------------------------------
                              Local types
-------------------------------------------------------------------*/

/*  thread attributes.
*/
struct DCLOSTHREADATTR
{
    D_UINT32       ulStackSize;
    int             nPriority;
};


struct DCLOSTHREAD
{
    int             iTID;
};


/*-------------------------------------------------------------------
                         Run-time configuration
-------------------------------------------------------------------*/

/*  Default stack size for threads.
*/
static D_UINT32 ulDefaultStackSize = 8 * 1024;


/*-------------------------------------------------------------------
    Public: DclOsThreadAttrCreate()

    Allocate a thread attributes object and initialize it with
    default values.

    Parameters:
        None.

    Return Value:
        Returns a pointer to a thread attributes object (opaque type).
-------------------------------------------------------------------*/
DCLTHREADATTR *DclOsThreadAttrCreate(void)
{
    DCLTHREADATTR  *pAttr = DclMemAllocZero(sizeof *pAttr);

    if(pAttr)
    {
        DclOsThreadAttrSetStackSize(pAttr, ulDefaultStackSize);
        pAttr->nPriority = THREAD_PRIORITY_NORMAL;
    }

    return pAttr;
}


/*-------------------------------------------------------------------
    Public: DclOsThreadAttrDestroy()

    Destroy a thread attributes object.

    It is an error for the caller to dereference pAttr after
    this function returns.

    Parameters:
        pAttr - A pointer to a thread attributes object previously
                allocated with DclOsThreadAttrCreate().

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclOsThreadAttrDestroy(
    DCLTHREADATTR  *pAttr)
{
    DclAssert(pAttr);

  #if D_DEBUG
    DclMemSet(pAttr, 0xFF, sizeof *pAttr);
  #endif

    DclMemFree(pAttr);
}


/*-------------------------------------------------------------------
    Public: DclOsThreadAttrSetPriority()

    Set the thread priority attribute.  This function must be
    called prior to calling DclOsThreadCreate().  Dynamic
    changing of thread priorities is not supported.

    Parameters:
        pAttr       - A pointer to the thread attributes object
        uPriority   - One of the thread priority values defined in
                      dlosconf.h.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsThreadAttrSetPriority(
    DCLTHREADATTR  *pAttr,
    D_UINT16        uPriority)
{
    DclAssert(pAttr);

    /*  Map the generic thread priorities onto the OS thread priorities.
    */
    switch (uPriority)
    {
        case DCL_THREADPRIORITY_TIMECRITICAL:
            pAttr->nPriority = THREAD_PRIORITY_TIME_CRITICAL;
            break;

        case DCL_THREADPRIORITY_HIGH:
            pAttr->nPriority = THREAD_PRIORITY_HIGHEST;
            break;

        case DCL_THREADPRIORITY_ABOVENORMAL:
            pAttr->nPriority = THREAD_PRIORITY_ABOVE_NORMAL;
            break;

        case DCL_THREADPRIORITY_NORMAL:
            pAttr->nPriority = THREAD_PRIORITY_NORMAL;
            break;

        case DCL_THREADPRIORITY_BELOWNORMAL:
            pAttr->nPriority = THREAD_PRIORITY_BELOW_NORMAL;
            break;

        case DCL_THREADPRIORITY_LOW:
            pAttr->nPriority = THREAD_PRIORITY_LOWEST;
            break;

        case DCL_THREADPRIORITY_ABOVEIDLE:
            pAttr->nPriority = THREAD_PRIORITY_ABOVE_IDLE;
            break;

        case DCL_THREADPRIORITY_IDLE:
            pAttr->nPriority = THREAD_PRIORITY_IDLE;
            break;

        default:
            DclError();
            return DCLSTAT_THREADBADPRIORITY;
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsThreadAttrSetStackSize()

    Set the stack size attribute.  Threads created using this
    thread attributes object will have at least this much stack
    space allocated.

    Parameters:
        pAttr       - thread attributes object
        ulStackSize - the stack size

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsThreadAttrSetStackSize(
    DCLTHREADATTR  *pAttr,
    D_UINT32        ulStackSize)
{
    DclAssert(pAttr);

    pAttr->ulStackSize = ulStackSize;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    rtos_thread_startup() - RTOS specific thread startup

    Description
       RTOS specific thread startup code to support passing an
       argument pointer to a thread.

    Parameters

    Return Value
       Always returns 0.
-------------------------------------------------------------------*/
static int rtos_thread_startup(
    DCLTHREADFUNC       pFunc,
    void               *pArgs,
    struct DCLOSTHREAD *pThread,
    int                 arg4,
    int                 arg5,
    int                 arg6,
    int                 arg7,
    int                 arg8,
    int                 arg9,
    int                 arg10)
{
    DclAssert(pFunc);

    (void)pFunc(pArgs);

    return 0;
}


/*-------------------------------------------------------------------
    Public: DclOsThreadCreate()

    Create a new thread of execution that calls the function pointed
    to by pFunc with the argument pointer pArgs.

    If phThread is not NULL, a handle representing the thread is
    returned.

    Characteristics of the thread may be controlled using a
    thread attributes object pointed to by pAttr.

    Parameters:
        phThread - A pointer to a location to receive a handle for
                   the new thread.  May be NULL if the thread handle
                   is not desired.
        pszName  - A pointer to the null-terminated thread name.
        pAttr    - A pointer to the attributes object describing the
                   thread to be created.
        pFunc    - A pointer to the entry point of the thread.
        pArgs    - A pointer to an argument passed to the thread.
                   May be NULL if no argument is required.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsThreadCreate(
    DCLTHREADHANDLE    *phThread,
    char               *pszName,
    DCLTHREADATTR      *pAttr,
    DCLTHREADFUNC       pFunc,
    void               *pArgs)
{
    struct DCLOSTHREAD *pThread;

    DclAssert(pAttr);
    DclAssert(pFunc);

    DCLPRINTF(1, ("Creating thread %s\n", pszName));

    pThread = DclMemAllocZero(sizeof *pThread);
    if(pThread)
    {
        pThread->iTID = taskSpawn(
            pszName,
            pAttr->nPriority,       /* priority       */
            0,                      /* flags          */
            pAttr->ulStackSize,
            &rtos_thread_startup,
            (int)pFunc,             /* arg1           */
            (int)pArgs,             /* arg2           */
            (int)pThread,           /* arg3           */
            0, 0, 0, 0, 0, 0, 0);   /* arg4 - 10      */

        if(pThread->iTID != ERROR)
        {
            if(phThread)
            {
                *phThread = pThread;
            }
            return DCLSTAT_SUCCESS;
        }

    }

    DCLPRINTF(1, ("DclOsThreadCreate() failed, Status=%lX\n", pThread->iTID));

    if(pThread)
        DclMemFree(pThread);

    return DCLSTAT_THREADCREATEFAILED;
}


/*-------------------------------------------------------------------
    Public: DclOsThreadDestroy()

    Destroy the specified thread and releases its resources.  This
    function is corollary to DclOsThreadCreate.  It must not be used
    to terminate a running thread.  The thread must have either
    exited normally, or have been terminated prior to calling this
    function.

    Parameters:
        hThread    - The target thread to destroy

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsThreadDestroy(
    DCLTHREADHANDLE hThread)
{
    DclAssert(hThread);
    DclAssert(hThread->iTID);

    if(taskDelete(hThread->iTID) == ERROR && errno != S_objLib_OBJ_ID_ERROR)
        return DCLSTAT_THREADDESTROYFAILED;

    DclMemFree(hThread);

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsThreadTerminate()

    Terminate an active thread.  It should only be used in extreme
    circumstances if the thread cannot complete its execution run
    normally.

    Parameters:
        hThread    - The target thread to terminate

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsThreadTerminate(
    DCLTHREADHANDLE hThread)
{
    DclAssert(hThread);
    DclAssert(hThread->iTID);

    taskDeleteForce(hThread->iTID);

    if(errno == S_intLib_NOT_ISR_CALLABLE ||
       errno == S_objLib_OBJ_DELETED ||
       errno == S_objLib_OBJ_UNAVAILABLE || errno == S_objLib_OBJ_ID_ERROR)
    {
        return DCLSTAT_THREADTERMINATEFAILED;
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsThreadWait()

    Cause execution of the current thread to wait for completion of
    the specified thread.

    Parameters:
        hThread    - The target thread to wait on.
        ulTimeout  - The timeout value in milliseconds.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsThreadWait(
    DCLTHREADHANDLE hThread,
    D_UINT32        ulTimeout)
{
    STATUS          sResult;
    char            caStatus[10];

    DclAssert(hThread);
    DclAssert(hThread->iTID);

    /*  If the thread hasn't finished yet then just fail.  Currently this
        function will only be called when we are shutting down.  Therefore,
        if the thread hasn't yet finished execution we don't need to wait
        on its completion.
    */
    sResult = taskStatusString(hThread->iTID, caStatus);
    if(sResult == OK)
    {
        if(DclStrICmp(caStatus, "DEAD") == 0)
            return DCLSTAT_SUCCESS;
    }

    DclOsSleep(ulTimeout);

    sResult = taskStatusString(hThread->iTID, caStatus);
    if(sResult == OK)
    {
        if(DclStrICmp(caStatus, "DEAD") == 0)
            return DCLSTAT_SUCCESS;
    }

    return DCLSTAT_THREADWAITFAILED;
}

#endif



#endif /* DCL_OSFEATURE_THREADS */
