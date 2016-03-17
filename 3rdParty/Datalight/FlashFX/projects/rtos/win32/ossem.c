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

       DclOsSemaphoreAcquire()
       DclOsSemaphoreCreate()
       DclOsSemaphoreDestroy()
       DclOsSemaphoreInfo()
       DclOsSemaphoreRelease()

    Should these routines need customization for your project, copy this
    module into the Project Directory, make your changes, and modify
    the master product's make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                              Revision History
    $Log: ossem.c $
    Revision 1.3  2009/04/10 20:55:02Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.2  2007/11/03 23:50:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2006/02/09 22:26:26Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/
 
#include <windows.h>

#include <dcl.h>
#include <dlapiprv.h>

struct tagDCLOSSEMAPHORE
{
    HANDLE          hSem;
};


/*-------------------------------------------------------------------
    Public: DclOsSemaphoreAcquire()

    Acquire a semaphore object created with DclOsSemaphoreCreate().

    This function is *NOT* intended to be called directly by client
    code.  The general semaphore abstraction DclSemaphoreAcquire()
    should be used instead.

    Parameters:
        pOsSem   - a pointer to the semaphore object.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsSemaphoreAcquire(
    PDCLOSSEMAPHORE pOsSem)
{
    DWORD           dwState;

    DclAssert(pOsSem);

    dwState = WaitForSingleObject(pOsSem->hSem, INFINITE);
    if(dwState != WAIT_OBJECT_0)
    {
        DCLPRINTF(1, ("Unable to obtain semaphore, error %lu\n", GetLastError()));

        DclError();

        return DCLSTAT_SEMAPHOREACQUIREFAILED;
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsSemaphoreCreate()

    Create a semaphore object.

    This function is *NOT* intended to be called directly by client
    code.  The general semaphore abstraction DclSemaphoreCreate()
    should be used instead.

    Parameters:
        pOsSem   - A pointer to a zero-inited buffer of the size
                   returned by DclOsSemaphoreInfo(NULL).
        pszName  - A pointer to the null terminated semaphore name.
        ulCount  - The initial semaphore count.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsSemaphoreCreate(
    PDCLOSSEMAPHORE pOsSem,
    const char *         pszName,
    D_UINT32        ulCount)
{
    TCHAR           awcBuffer[DCL_SEMAPHORENAMELEN];
    
    DclAssert(pOsSem);
    DclAssert(pszName);
    DclAssert(DclStrLen(pszName) < DCL_SEMAPHORENAMELEN);

    MultiByteToWideChar(CP_ACP, 0, pszName, -1, awcBuffer, DCL_SEMAPHORENAMELEN);

    pOsSem->hSem = CreateSemaphore(NULL, ulCount, ulCount, awcBuffer);
    if(!pOsSem->hSem)
    {
        DCLPRINTF(1, ("Semaphore creation failed, error %ld\n", GetLastError()));

        DclError();

        return DCLSTAT_SEMAPHORECREATEFAILED;
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsSemaphoreDestroy()

    Destroy a semaphore object which was created with
    DclOsSemaphoreCreate().

    This function is *NOT* intended to be called directly by client
    code.  The general semaphore abstraction DclSemaphoreDestroy()
    should be used instead.

    Parameters:
        pOsSem   - A pointer to the semaphore object.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsSemaphoreDestroy(
    PDCLOSSEMAPHORE pOsSem)
{
    D_BOOL          fSuccess;
    
    DclAssert(pOsSem);

    fSuccess = CloseHandle(pOsSem->hSem);
    if(!fSuccess)
    {
        DCLPRINTF(1, ("Unable to delete semaphore, error %lu\n", GetLastError()));

        DclError();

        return DCLSTAT_SEMAPHOREDESTROYFAILED;
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsSemaphoreInfo()

    Return the current usage count for a semaphore.  If the pOsSem
    parameter is NULL, the size of the semaphore object is returned.

    Parameters:
        pOsSem  - A pointer to the semaphore object.  Specify NULL
                  to return the semaphore object size.

    Return Value:
        Returns the usage count for the semaphore if pOsSem is
        a valid pointer.  This function may return a value of 
        D_UINT32_MAX if the functionality is not supported.  If
        pOsSem is NULL, the size of the semaphore object will
        be returned.
-------------------------------------------------------------------*/
D_UINT32 DclOsSemaphoreInfo(
    PDCLOSSEMAPHORE pOsSem)
{
    if(pOsSem)
    {   
        /*  Unsupported for this port right now.
        */
        return D_UINT32_MAX;
    }
    else
    {
        return sizeof(*pOsSem);
    }
}


/*-------------------------------------------------------------------
    Public: DclOsSemaphoreRelease()

    Release a semaphore object, reducing its usage count by one.

    This function is *NOT* intended to be called directly by client
    code.  The general semaphore abstraction DclSemaphoreRelease()
    should be used instead.

    Parameters:
        pOsSem   - A pointer to the semaphore object.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsSemaphoreRelease(
    PDCLOSSEMAPHORE pOsSem)
{
    D_BOOL          fSuccess;

    DclAssert(pOsSem);

    fSuccess = ReleaseSemaphore(pOsSem->hSem, 1, NULL);
    if(!fSuccess)
    {
        DCLPRINTF(1, ("Unable to release semaphore, error %lu\n", GetLastError()));

        DclError();

        return DCLSTAT_SEMAPHORERELEASEFAILED;
    }

    return DCLSTAT_SUCCESS;
}
