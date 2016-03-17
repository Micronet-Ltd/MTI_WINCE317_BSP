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
  jurisdictions.  The software may be subject to one or more of these US
  patents: US#5860082, US#6260156.  Patents may be pending.

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

    This module contains the RTOS OS Services default implementations for:

        DclOsThreadAttrCreate()
        DclOsThreadAttrDestroy()
        DclOsThreadAttrSetPriority()
        DclOsThreadAttrSetStackSize()
        DclOsThreadCreate()
        DclOsThreadDestroy()
        DclOsThreadID()
        DclOsThreadResume()
        DclOsThreadSuspend()
        DclOsThreadTerminate()
        DclOsThreadWait()

    Should these routines need customization for your project, copy this
    module into the Project Directory, make your changes, and modify
    the master product's make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osthread.c $
    Revision 1.7  2009/04/10 20:05:29Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.6  2009/02/18 04:24:27Z  keithg
    Added explicit void of unused formal parameters.
    Revision 1.5  2007/11/03 23:50:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.4  2007/07/31 18:53:01Z  Garyp
    Implemented DclOsThreadSuspend/Resume().
    Revision 1.3  2006/10/18 04:27:23Z  Garyp
    Updated so DclOsThreadID() is always included.
    Revision 1.2  2006/10/04 02:20:47Z  Garyp
    Updated to use DclMemAllocZero().
    Revision 1.1  2006/02/09 22:25:40Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <stdlib.h>
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
    DCLTHREADID id;

    id = GetCurrentThreadId();
    DclAssert(id);

    return id;
}


#if DCL_OSFEATURE_THREADS

/*-------------------------------------------------------------------
                              Local types
-------------------------------------------------------------------*/

struct DCLOSTHREADATTR
{
    D_UINT32        ulStackSize;
    D_UINT16        uPriority;
};

struct DCLOSTHREAD
{
    HANDLE          thread;
};


/*-------------------------------------------------------------------
                         Run-time configuration
-------------------------------------------------------------------*/

/*  Default stack size for threads.  Zero here implies using the default
    set by /STACK.
*/
static D_UINT32 ulDefaultStackSize = 0;


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
    DCLTHREADATTR  *p = DclMemAllocZero(sizeof(struct DCLOSTHREADATTR));

    if(p)
    {
        p->ulStackSize = ulDefaultStackSize;
    }

    return p;
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
            pAttr->uPriority = 0;
            break;

        case DCL_THREADPRIORITY_HIGH:
            pAttr->uPriority = 0;
            break;

        case DCL_THREADPRIORITY_ABOVENORMAL:
            pAttr->uPriority = 0;
            break;

        case DCL_THREADPRIORITY_NORMAL:
            pAttr->uPriority = 0;
            break;

        case DCL_THREADPRIORITY_BELOWNORMAL:
            pAttr->uPriority = 0;
            break;

        case DCL_THREADPRIORITY_LOW:
            pAttr->uPriority = 0;
            break;

        case DCL_THREADPRIORITY_ABOVEIDLE:
            pAttr->uPriority = 0;
            break;

        case DCL_THREADPRIORITY_IDLE:
            pAttr->uPriority = 0;
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
    D_UINT32        ulStacksize)
{
    DclAssert(pAttr);

    pAttr->ulStackSize = ulStacksize;

    return DCLSTAT_SUCCESS;
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

    (void) pszName;
    DCLPRINTF(2, ("Creating thread %s\n", pszName));

    pThread = DclMemAlloc(sizeof *pThread);

    if(pThread)
    {
        pThread->thread = CreateThread(
            NULL,                       /* ignored, must be NULL */
            pAttr->ulStackSize,
            (LPTHREAD_START_ROUTINE)pFunc,
            pArgs,
            /*pAttr->ulStackSize ? STACK_SIZE_PARAM_IS_A_RESERVATION :*/ 0,
            NULL);

        if(pThread->thread)
        {
            if(phThread)
            {
                *phThread = pThread;
            }
            else
            {
                DclMemFree(pThread);
            }

            return DCLSTAT_SUCCESS;
        }

        DclMemFree(pThread);
    }

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

    if(hThread)
    {
        if(hThread->thread)
        {
            if(!CloseHandle(hThread->thread))
            {
                return DCLSTAT_THREADDESTROYFAILED;
            }
        }
        DclMemFree(hThread);

        return DCLSTAT_SUCCESS;
    }

    return DCLSTAT_THREADDESTROYFAILED;
}


/*-------------------------------------------------------------------
    Public: DclOsThreadResume

    This function suspends execution of the specified thread.

    Parameters:
        hThread    - The target thread to resume

    Return:
        Returns a DCLSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclOsThreadResume(
    DCLTHREADHANDLE hThread)
{
    DWORD           dwResult;

    DclAssert(hThread);
    DclAssert(hThread->thread);

    dwResult = ResumeThread(hThread->thread);
    if (dwResult == -1)
    {
        return DCLSTAT_THREADRESUMEFAILED;
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsThreadSuspend

    This function resumes execution of a suspended thread.

    Parameters:
        hThread    - The target thread to suspend

    Return:
        Returns a DCLSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclOsThreadSuspend(
    DCLTHREADHANDLE hThread)
{
    DWORD           dwResult;

    DclAssert(hThread);
    DclAssert(hThread->thread);

    dwResult = SuspendThread(hThread->thread);
    if (dwResult == -1)
    {
        return DCLSTAT_THREADSUSPENDFAILED;
    }

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

    if(hThread)
    {
        if(hThread->thread)
        {
            if(!TerminateThread(hThread->thread, 0))
            {
                return DCLSTAT_THREADTERMINATEFAILED;
            }
        }

        return DCLSTAT_SUCCESS;
    }

    return DCLSTAT_THREADTERMINATEFAILED;
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
    D_UINT32        ulStatus;

    DclAssert(hThread);

    if(hThread)
    {
        if(hThread->thread)
        {
            ulStatus = WaitForSingleObject(hThread->thread, ulTimeout);
            if(ulStatus == WAIT_OBJECT_0)
            {
                return DCLSTAT_SUCCESS;
            }
            else if(ulStatus == WAIT_TIMEOUT)
            {
                return DCLSTAT_THREADWAITTIMEOUT;
            }
            else
            {
                return DCLSTAT_THREADWAITFAILED;
            }
        }
    }

    return DCLSTAT_THREADWAITFAILED;
}

#endif /* DCL_OSFEATURE_THREADS */
