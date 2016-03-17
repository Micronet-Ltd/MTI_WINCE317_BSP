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

       DclOsMutexAcquire()
       DclOsMutexCreate()
       DclOsMutexDestroy()
       DclOsMutexInfo()
       DclOsMutexRelease()

    Should these routines need customization for your project, copy this
    module into the Project Directory, make your changes, and modify
    the master product's make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                              Revision History
    $Log: osmutex.c $
    Revision 1.3  2009/04/10 20:55:02Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.2  2007/11/03 23:50:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2006/02/09 22:26:36Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>
#include <dlapiprv.h>


struct tagDCLOSMUTEX
{
    CRITICAL_SECTION critsec;
};


/*-------------------------------------------------------------------
    Public: DclOsMutexAcquire()

    Acquire an OS mutex object created with DclOsMutexCreate().

    This function is *NOT* intended to be called directly by client
    code.  The general mutex abstraction DclMutexAcquire() should
    be used instead.

    Parameters:
        pOsMutex   - A pointer to the OS mutex object.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsMutexAcquire(
    PDCLOSMUTEX     pOsMutex)
{
    DclAssert(pOsMutex);

    EnterCriticalSection(&pOsMutex->critsec);

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsMutexCreate()

    Create an OS mutex object.

    This function is *NOT* intended to be called directly by client
    code.  The general mutex abstraction DclMutexCreate() should
    be used instead.

    Parameters:
        pOsMutex - A pointer to a buffer which will hold the OS mutex
                   object.  this buffer must be at least as long as
                   the object length returned by DclOsMutexInfo().
        pszName  - A pointer to the null terminated mutex name.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsMutexCreate(
    PDCLOSMUTEX     pOsMutex,
    const char *         pszName)
{
    DclAssert(pOsMutex);
    DclAssert(pszName);
    DclAssert(DclStrLen(pszName) < DCL_MUTEXNAMELEN);

    (void)pszName;
    
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
        pOsMutex   - A pointer to the OS mutex object.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsMutexDestroy(
    PDCLOSMUTEX     pOsMutex)
{
    DclAssert(pOsMutex);

    DeleteCriticalSection(&pOsMutex->critsec);

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
    DclAssert(pOsMutex);

    LeaveCriticalSection(&pOsMutex->critsec);

    return DCLSTAT_SUCCESS;
}
