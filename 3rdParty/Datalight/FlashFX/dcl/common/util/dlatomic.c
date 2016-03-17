/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
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

    This module implements a generic Atomic API, which may be used as a
    default implementation for those environments where such services do
    not exist.

    This code is NOT intended to be called directly by client code, but
    rather is called by the OS Services "Atomic" implementation (typically
    in osatomic.c).

    The Atomic API is considered to be one of the lowest level primitive
    APIs, and as such does not rely on fancy things like dynamically
    allocated memory and mutexes (rather, those services rely on <this> API
    so that <they> can be thread-safe).

    As such, this module uses the low level, statically allocated OS mutex,
    which must be provided by the OS Services mutex implementation.  In
    single-threaded environments, this is no big deal, as in most cases, the
    mutex implementation is going to be a simple pass-thru stub.  However,
    in more sophisticated multi-threaded environments, using a mutex is
    probably NOT the most efficient way to accomplish what is needed.  It is
    likely that the OS Services Atomic API implementation could be more
    optimally coded to use more efficient operating system services, or even
    assembly language.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlatomic.c $
    Revision 1.3  2010/01/23 19:29:10Z  garyp
    Added DclAtomic32ExchangeAdd().  Documentation updated.
    Revision 1.2  2009/06/28 02:46:15Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.2  2008/12/05 21:03:46Z  garyp
    Added functions for managing pointers atomically.  Renamed the functions
    for clarity.
    Revision 1.1  2008/11/29 03:18:02Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiprv.h>


/*-------------------------------------------------------------------
    Public: DclAtomic32CompareExchange()

    Perform an atomic comparison of the value at pulDestination with
    ulCompare.  If the values are equal, ulExchange is stored in
    pulDestination.  If the values are not equal, no operation is
    performed.

    The *pulAtomic value must be aligned on a 32-bit boundary.

    Parameters:
        pulAtomic  - A pointer to the destination value
        ulCompare  - The value to compare to *pulDestination
        ulExchange - The value to conditionally store

    Return Value:
        Returns the original value at *pulDestination.
-------------------------------------------------------------------*/
D_UINT32 DclAtomic32CompareExchange(
    D_ATOMIC32     *pulAtomic,
    D_UINT32        ulCompare,
    D_UINT32        ulExchange)
{
    DclAssert(DCLISALIGNED((D_UINTPTR)pulAtomic, sizeof(*pulAtomic)));

    if(DclOsMutexAcquire(NULL) == DCLSTAT_SUCCESS)
    {
        D_UINT32 ulReturn = *pulAtomic;

        if(*pulAtomic == ulCompare)
            *pulAtomic = ulExchange;

        DclOsMutexRelease(NULL);

        return ulReturn;
    }

    /*  If the interfaces are being used properly, this
        should just not ever happen...
    */
    DclError();
    return 0;
}


/*-------------------------------------------------------------------
    Public: DclAtomic32Exchange()

    Atomically exchange two values.

    The *pulAtomic value must be aligned on a 32-bit boundary.

    Parameters:
        pulAtomic   - A pointer to a D_ATOMIC32 location to modify.
        ulExchange  - The value to exchange with *pulAtomic.

    Return Value:
        Returns the original value at *pulAtomic.
-------------------------------------------------------------------*/
D_UINT32 DclAtomic32Exchange(
    D_ATOMIC32     *pulAtomic,
    D_UINT32        ulExchange)
{
    DclAssert(DCLISALIGNED((D_UINTPTR)pulAtomic, sizeof(*pulAtomic)));

    if(DclOsMutexAcquire(NULL) == DCLSTAT_SUCCESS)
    {
        D_UINT32 ulReturn = *pulAtomic;

        *pulAtomic = ulExchange;

        DclOsMutexRelease(NULL);

        return ulReturn;
    }

    /*  If the interfaces are being used properly, this
        should just not ever happen...
    */
    DclError();
    return 0;
}


/*-------------------------------------------------------------------
    Public: DclAtomic32ExchangeAdd()

    Atomically add a one 32-bit value to another, and return the 
    original value.

    The *pulAtomic value must be aligned on a 32-bit boundary.

    Parameters:
        pulAtomic   - A pointer to a D_ATOMIC32 location to modify.
        ulAdd       - The value to add to *pulAtomic.

    Return Value:
        Returns the original value at *pulAtomic.
-------------------------------------------------------------------*/
D_UINT32 DclAtomic32ExchangeAdd(
    D_ATOMIC32     *pulAtomic,
    D_UINT32        ulAdd)
{
    DclAssert(DCLISALIGNED((D_UINTPTR)pulAtomic, sizeof(*pulAtomic)));

    if(DclOsMutexAcquire(NULL) == DCLSTAT_SUCCESS)
    {
        D_UINT32 ulReturn = *pulAtomic;

        *pulAtomic += ulAdd;

        DclOsMutexRelease(NULL);

        return ulReturn;
    }

    /*  If the interfaces are being used properly, this
        should just not ever happen...
    */
    DclError();
    return 0;
}


/*-------------------------------------------------------------------
    Public: DclAtomic32Decrement()

    Atomically decrement the specified variable and return the new
    value.

    The *pulAtomic value must be aligned on a 32-bit boundary.

    Parameters:
        pulAtomic   - A pointer to a D_ATOMIC32 value to decrement.

    Return Value:
        Returns the decremented value.
-------------------------------------------------------------------*/
D_UINT32 DclAtomic32Decrement(
    D_ATOMIC32     *pulAtomic)
{
    DclAssert(DCLISALIGNED((D_UINTPTR)pulAtomic, sizeof(*pulAtomic)));

    if(DclOsMutexAcquire(NULL) == DCLSTAT_SUCCESS)
    {
        D_UINT32 ulReturn;

        (*pulAtomic)--;
        ulReturn = *pulAtomic;

        DclOsMutexRelease(NULL);

        return ulReturn;
    }

    /*  If the interfaces are being used properly, this
        should just not ever happen...
    */
    DclError();
    return 0;
}


/*-------------------------------------------------------------------
    Public: DclAtomic32Increment()

    Atomically increment the specified variable and return the new
    value.

    The *pulAtomic value must be aligned on a 32-bit boundary.

    Parameters:
        pulAtomic   - A pointer to a D_ATOMIC32 value to increment.

    Return Value:
        Returns the incremented value.
-------------------------------------------------------------------*/
D_UINT32 DclAtomic32Increment(
    D_ATOMIC32     *pulAtomic)
{
    DclAssert(DCLISALIGNED((D_UINTPTR)pulAtomic, sizeof(*pulAtomic)));

    if(DclOsMutexAcquire(NULL) == DCLSTAT_SUCCESS)
    {
        D_UINT32 ulReturn;

        (*pulAtomic)++;
        ulReturn = *pulAtomic;

        DclOsMutexRelease(NULL);

        return ulReturn;
    }

    /*  If the interfaces are being used properly, this
        should just not ever happen...
    */
    DclError();
    return 0;
}


/*-------------------------------------------------------------------
    Public: DclAtomicPtrCompareExchange()

    Perform an atomic comparison of the pointer at pDestination with
    pCompare.  If the pointers are equal, pExchange is stored in
    pDestination.  If the pointers are not equal, no operation is
    performed.

    Parameters:
        pDestination   - A pointer to the location which contains
                         the pointer value.  This must be aligned
                         on a native pointer-size boundary.
        pCompare       - The pointer value to compare to with
                         *pDestination.
        pExchange      - The pointer value to conditionally store.

    Return Value:
        Returns the original pointer value at *pDestination.
-------------------------------------------------------------------*/
void * DclAtomicPtrCompareExchange(
    D_ATOMICPTR    *pDestination,
    void           *pCompare,
    void           *pExchange)
{
    DclAssert(DCLISALIGNED((D_UINTPTR)pDestination, sizeof(*pDestination)));

    if(DclOsMutexAcquire(NULL) == DCLSTAT_SUCCESS)
    {
        void *pReturn = (void*)*pDestination;

        if(*pDestination == pCompare)
            *pDestination = pExchange;

        DclOsMutexRelease(NULL);

        return pReturn;
    }

    /*  If the interfaces are being used properly, this
        should just not ever happen...
    */
    DclError();
    return NULL;
}


/*-------------------------------------------------------------------
    Public: DclAtomicPtrExchange()

    Atomically exchange two pointer values.

    Parameters:
        pDestination   - A pointer to the location which contains
                         the pointer value.  This must be aligned
                         on a native pointer-size boundary.
        pExchange      - The pointer value to exchange with
                         *pDestination.

    Return Value:
        Returns the original pointer value at *pDestination.
-------------------------------------------------------------------*/
void * DclAtomicPtrExchange(
    D_ATOMICPTR    *pDestination,
    void           *pExchange)
{
    DclAssert(DCLISALIGNED((D_UINTPTR)pDestination, sizeof(*pDestination)));

    if(DclOsMutexAcquire(NULL) == DCLSTAT_SUCCESS)
    {
        void *pReturn = (void*)*pDestination;

        *pDestination = pExchange;

        DclOsMutexRelease(NULL);

        return pReturn;
    }

    /*  If the interfaces are being used properly, this
        should just not ever happen...
    */
    DclError();
    return 0;
}





