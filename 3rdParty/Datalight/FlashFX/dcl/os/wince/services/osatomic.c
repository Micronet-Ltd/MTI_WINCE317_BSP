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

    This module contains the OS Services default implementations for the
    Atomic API.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    the master product's make file to build the new module.

    The Atomic API is considered to be one of the lowest level primitive
    APIs, and as such does not rely on fancy things like dynamically
    allocated memory and mutexes (rather, those services rely on <this> API
    so that <they> can be thread-safe).
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osatomic.c $
    Revision 1.3  2010/01/23 21:26:58Z  garyp
    Added DclOsAtomic32ExchangeAdd().  Cleaned up the documentation
    and some variable names.
    Revision 1.2  2009/06/24 18:20:31Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.3  2009/03/23 01:07:51Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.1.1.2  2008/12/06 20:57:47Z  garyp
    Added functions for managing pointers atomically.  Renamed the functions
    for clarity.
    Revision 1.1  2008/11/29 21:05:58Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>
#include <dlinstance.h>


/*-------------------------------------------------------------------
    Public: DclOsAtomic32CompareExchange()

    Perform an atomic comparison of the value at pulAtomic with
    ulCompare.  If the values are equal, ulExchange is stored in
    pulAtomic.  If the values are not equal, no operation is
    performed.

    Parameters:
        pulAtomic  - A pointer to the location which contains
                     the atomic value.  This must be aligned
                     on a native 32-bit boundary.
        ulCompare  - The value to compare to *pulAtomic
        ulExchange - The value to conditionally store

    Return Value:
        Returns the original value at *pulAtomic.
-------------------------------------------------------------------*/
D_UINT32 DclOsAtomic32CompareExchange(
    D_ATOMIC32     *pulAtomic,
    D_UINT32        ulCompare,
    D_UINT32        ulExchange)
{
    DclAssert(DCLISALIGNED((D_UINTPTR)pulAtomic, sizeof(*pulAtomic)));

    return (D_UINT32)InterlockedCompareExchange((D_UINT32*)pulAtomic, ulExchange, ulCompare);
}


/*-------------------------------------------------------------------
    Public: DclOsAtomic32Exchange()

    Atomically exchange two values.

    Parameters:
        pulAtomic  - A pointer to the location which contains
                     the atomic value.  This must be aligned
                     on a native 32-bit boundary.
        ulExchange - The value to exchange with *pulAtomic.

    Return Value:
        Returns the original value at *pulAtomic.
-------------------------------------------------------------------*/
D_UINT32 DclOsAtomic32Exchange(
    D_ATOMIC32     *pulAtomic,
    D_UINT32        ulExchange)
{
    DclAssert(DCLISALIGNED((D_UINTPTR)pulAtomic, sizeof(*pulAtomic)));

    return (D_UINT32)InterlockedExchange((D_UINT32*)pulAtomic, ulExchange);
}


/*-------------------------------------------------------------------
    Public: DclOsAtomic32ExchangeAdd()

    Atomically add a one 32-bit value to another, and return the 
    original value.

    Parameters:
        pulAtomic - A pointer to the location which contains the
                    atomic value to which to add.  This must
                    be aligned on a native 32-bit boundary.
        ulAdd     - The value to add to *pulAtomic.

    Return Value:
        Returns the original value at *pulAtomic.
-------------------------------------------------------------------*/
D_UINT32 DclOsAtomic32ExchangeAdd(
    D_ATOMIC32     *pulAtomic,
    D_UINT32        ulAdd)
{
    DclAssert(DCLISALIGNED((D_UINTPTR)pulAtomic, sizeof(*pulAtomic)));

    return (D_UINT32)InterlockedExchangeAdd((D_UINT32*)pulAtomic, ulAdd);
}


/*-------------------------------------------------------------------
    Public: DclOsAtomic32Decrement()

    Atomically decrement the specified variable and return the new
    value.

    Parameters:
        pulAtomic   - A pointer to a D_ATOMIC32 value to increment.
                      This must be aligned on a 32-bit boundary.

    Return Value:
        Returns the decremented value.
-------------------------------------------------------------------*/
D_UINT32 DclOsAtomic32Decrement(
    D_ATOMIC32     *pulAtomic)
{
    DclAssert(DCLISALIGNED((D_UINTPTR)pulAtomic, sizeof(*pulAtomic)));

    return (D_UINT32)InterlockedDecrement((D_UINT32*)pulAtomic);
}


/*-------------------------------------------------------------------
    Public: DclOsAtomic32Increment()

    Atomically increment the specified variable and return the new
    value.

    Parameters:
        pulAtomic   - A pointer to a D_ATOMIC32 value to increment.
                      This must be aligned on a 32-bit boundary.

    Return Value:
        Returns the incremented value.
-------------------------------------------------------------------*/
D_UINT32 DclOsAtomic32Increment(
    D_ATOMIC32     *pulAtomic)
{
    DclAssert(DCLISALIGNED((D_UINTPTR)pulAtomic, sizeof(*pulAtomic)));

    return (D_UINT32)InterlockedIncrement((D_UINT32*)pulAtomic);
}


/*-------------------------------------------------------------------
    Public: DclOsAtomicPtrCompareExchange()

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
void * DclOsAtomicPtrCompareExchange(
    D_ATOMICPTR    *pDestination,
    void           *pCompare,
    void           *pExchange)
{
    DclAssert(DCLISALIGNED((D_UINTPTR)pDestination, sizeof(*pDestination)));

    return InterlockedCompareExchangePointer(pDestination, pExchange, pCompare);
}


/*-------------------------------------------------------------------
    Public: DclOsAtomicPtrExchange()

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
void * DclOsAtomicPtrExchange(
    D_ATOMICPTR    *pDestination,
    void           *pExchange)
{
    DclAssert(DCLISALIGNED((D_UINTPTR)pDestination, sizeof(*pDestination)));

    return InterlockedExchangePointer(pDestination, pExchange);
}






