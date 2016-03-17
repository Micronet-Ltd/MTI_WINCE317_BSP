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
    mutexes.

    Should these routines need customization for your project, copy this
    module into the Project Directory, make your changes, and modify the
    project's make file to build the new module.

    *Note* -- This API is *NOT* intended to be called directly by client code.
    The general mutex abstraction implemented in dlmutex.c should be used.
    That API uses this API, which is considered to be internal.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osmutex.c $
    Revision 1.7  2009/10/27 23:56:35Z  garyp
    Added the optional capability (for debugging) to support a mutex
    timeout.
    Revision 1.6  2009/06/27 21:34:35Z  garyp
    Added support for a static system mutex.
    Revision 1.5  2009/04/10 18:34:56Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.4  2009/02/08 01:43:48Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.3  2007/11/03 23:31:36Z  Garyp
    Added the standard module header.
    Revision 1.2  2007/01/21 16:15:18Z  Garyp
    Documentation update.
    Revision 1.1  2005/10/03 05:57:08Z  Pauli
    Initial revision
 ---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>
#include <dlapiprv.h>

/*  The TIMEOUT_MS value can be used to change the mutex behavior to cause
    the mutex to time-out after a specified number of milliseconds.  This
    is useful for debugging multithreaded issues, however it is for internal
    use only, and must be disabled in production software.
*/
#define TIMEOUT_MS      (0)             /* MUST be 0 for checkin */

struct tagDCLOSMUTEX
{
    CRITICAL_SECTION critsec;
};

static struct tagDCLOSMUTEX StaticMutex;
static PDCLOSMUTEX          pStaticMutex;

/*-------------------------------------------------------------------
    Datalight Development Notes:

    Note that this code uses critical section objects rather than
    the mutexes available in WinCE on the understanding that they
    are internally more efficient.  The primary advantage of mutex
    objects is external visibility which is not important to DCL.

    Note that CE requires that critical sections are released before a
    thread terminates.  Not doing so will generate an exception when
    running in debug mode.
-------------------------------------------------------------------*/


/*-------------------------------------------------------------------
    Public: DclOsMutexAcquire()

    Acquire an OS mutex object created with DclOsMutexCreate().

    This function is *NOT* intended to be called directly by client
    code.  The general mutex abstraction DclMutexAcquire() should
    be used instead.

    Parameters:
        pOsMutex   - A pointer to the OS mutex object, or NULL to
                     acquire the single static OS mutex.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsMutexAcquire(
    PDCLOSMUTEX     pOsMutex)
{
    if(!pOsMutex)
    {
        if(!pStaticMutex)
            return DCLSTAT_MUTEX_STATICOSMUTEXNOTINUSE;

        pOsMutex = pStaticMutex;
    }

  #if TIMEOUT_MS
    {
        DCLTIMESTAMP    ts;

        if(TryEnterCriticalSection(&pOsMutex->critsec))
            return DCLSTAT_SUCCESS;

        ts = DclTimeStamp();

        while(DclTimePassed(ts) < TIMEOUT_MS)
        {
            if(TryEnterCriticalSection(&pOsMutex->critsec))
                return DCLSTAT_SUCCESS;

            DclOsSleep(1);
        }

        return DCLSTAT_MUTEX_TIMEOUT;
    }

  #else

    EnterCriticalSection(&pOsMutex->critsec);

  #endif

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsMutexCreate()

    Create an OS mutex object.

    The OS mutex API supports the use of a single statically created
    mutex.  This is not thread-safe and therefore is typically used
    early in the initialization process, usually by code which is
    designed to make <everything else> thread-safe.  This special
    statically allocated mutex is specified by using a NULL pOsMutex
    pointer.

    This function is *NOT* intended to be called directly by client
    code.  The general mutex abstraction DclMutexCreate() should
    be used instead.

    Parameters:
        pOsMutex - A pointer to a buffer which will hold the OS mutex
                   object, or NULL to use the single, statically
                   allocated mutex.  If used, this buffer must be at
                   least as long as the object length returned by
                   DclOsMutexInfo().
        pszName  - A pointer to the null terminated mutex name.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsMutexCreate(
    PDCLOSMUTEX     pOsMutex,
    const char     *pszName)
{
    if(!pOsMutex)
    {
        if(pStaticMutex)
        {
            DclError();
            return DCLSTAT_MUTEX_STATICOSMUTEXINUSE;
        }

        pStaticMutex = &StaticMutex;

        pOsMutex = pStaticMutex;
    }

    DclAssert(pszName);
    DclAssert(DclStrLen(pszName) < DCL_MUTEXNAMELEN);

    (void)pszName;

    /*  Can't fail
    */
    InitializeCriticalSection(&pOsMutex->critsec);

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsMutexDestroy()

    Destroy an OS mutex object that was created with DclOsMutexCreate().

    This function is *NOT* intended to be called directly by client
    code.  The general mutex abstraction DclMutexDestroy() should
    be used instead.

    Parameters:
        pOsMutex   - A pointer to the OS mutex object, or NULL to
                     destroy the single static OS mutex.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsMutexDestroy(
    PDCLOSMUTEX     pOsMutex)
{
    if(!pOsMutex)
    {
        if(!pStaticMutex)
        {
            DclError();
            return DCLSTAT_MUTEX_STATICOSMUTEXNOTINUSE;
        }

        pOsMutex = pStaticMutex;
    }

    DeleteCriticalSection(&pOsMutex->critsec);

    if(pStaticMutex == pOsMutex)
        pStaticMutex = NULL;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsMutexInfo()

    Return information about a mutex object, or mutex objects in
    general.  If the pOsMutex parameter is NULL, the size of the
    mutex object is returned.  All other values for pOsMutex are
    reserved.

    Parameters:
        pOsMutex - A pointer to the mutex object (must be NULL
                   at this time).

    Return Value:
        If pOsMutex is NULL, the size of the mutex object will
        be returned.
-------------------------------------------------------------------*/
D_UINT32 DclOsMutexInfo(
    PDCLOSMUTEX     pOsMutex)
{
    if(pOsMutex)
    {
        /*  Unsupported functionality at this time.
        */
        DclError();

        return D_UINT32_MAX;
    }
    else
    {
        return sizeof(*pOsMutex);
    }
}


/*-------------------------------------------------------------------
    Public: DclOsMutexRelease()

    Release an OS mutex object.

    This function is *NOT* intended to be called directly by client
    code.  The general mutex abstraction DclMutexRelease() should
    be used instead.

    Parameters:
        pOsMutex   - A pointer to the OS mutex object, or NULL to
                     release the single static OS mutex.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsMutexRelease(
    PDCLOSMUTEX     pOsMutex)
{
    if(!pOsMutex)
    {
        if(!pStaticMutex)
        {
            DclError();
            return DCLSTAT_MUTEX_STATICOSMUTEXNOTINUSE;
        }

        pOsMutex = pStaticMutex;
    }

    LeaveCriticalSection(&pOsMutex->critsec);

    return DCLSTAT_SUCCESS;
}


