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

    This module contains the default OS Services functions for managing
    semaphores.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify the
    make file to build the new module.

    *Note*
    This is a stub implementation of semaphores for a single-threaded
    environment.  It includes code that can help detect some common
    mistakes like failing to balance acquire/release, or continuing to
    use a semaphore that has been destroyed.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ossem.c $
    Revision 1.5  2010/01/11 02:23:26Z  garyp
    Enhanced error checking and made the code more intuitive.
    Revision 1.4  2009/04/10 20:52:23Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.3  2007/11/03 23:31:38Z  Garyp
    Added the standard module header.
    Revision 1.2  2006/03/30 23:23:22Z  joshuab
    Make semaphores for wincebl actually maintain a count, since 
    it's used by the boot loader code due to static region mapping.
    Revision 1.1  2005/10/02 03:58:08Z  Pauli
    Initial revision
    Revision 1.3  2005/08/21 11:39:23Z  garyp
    Documentation update.
    Revision 1.2  2005/08/03 19:17:30Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/01 03:22:00Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiprv.h>

#if DCL_OSFEATURE_THREADS
    /*  If the OS supports multiple threads, this module must be modified to
        properly support real semaphores.  In a single-threaded environment,
        this stubbed implementation will simply operate in a fashion which
        checked for mismatched acquire/release calls.
    */
    #error "DCL: Default (single-threaded) DCL OS semaphore built in OS with threading capabilities."
#endif 

typedef struct tagDCLOSSEMAPHORE
{
    D_UINT32        ulMaxCount;
    D_UINT32        ulCurrentCount;
} DCLOSSEMAPHORE;


/*-------------------------------------------------------------------
    Public: DclOsSemaphoreAcquire()

    Acquire a semaphore object created with DclOsSemaphoreCreate().

    This function is *NOT* intended to be called directly by client
    code.  The general semaphore abstraction DclSemaphoreAcquire()
    should be used instead.

    Parameters:
        pOsSem   - A pointer to the semaphore object.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsSemaphoreAcquire(
    PDCLOSSEMAPHORE pOsSem)
{
    DclAssert(pOsSem);

    /*  This is a stubbed semaphore implementation for a single-threaded
        environment, and therefore, this better ever happen...
    */  
    if(!pOsSem->ulCurrentCount)
    {
        DCLPRINTF(1, ("DclOsSemaphoreAcquire() should never block in a single-threaded environment\n"));
        DclError();
        return DCLSTAT_SEMAPHOREACQUIREFAILED;
    }        

    pOsSem->ulCurrentCount--;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsSemaphoreCreate()

    Create a semaphore object.

    This function is *NOT* intended to be called directly by client
    code.  The general semaphore abstraction DclSemaphoreCreate()
    should be used instead.

    Parameters:
        pOsSem   - A pointer to a zero-initialized buffer of the
                   size returned by DclOsSemaphoreInfo(NULL).
        pszName  - A pointer to the null terminated semaphore name.
        ulCount  - The initial semaphore count.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsSemaphoreCreate(
    PDCLOSSEMAPHORE pOsSem,
    const char     *pszName,
    D_UINT32        ulCount)
{
    DclAssert(pOsSem);
    DclAssert(pszName);
    DclAssert(DclStrLen(pszName) < DCL_SEMAPHORENAMELEN);
    DclAssert(ulCount);

    (void)pszName;

    pOsSem->ulMaxCount = ulCount;
    pOsSem->ulCurrentCount = ulCount;

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
    DclAssert(pOsSem);

    (void)pOsSem;

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
        Returns the usage count for the semaphore if pOsSem is a valid
        pointer.  This function may return a value of D_UINT32_MAX if
        the functionality is not supported.  If pOsSem is NULL, the
        size of the semaphore object will be returned.
-------------------------------------------------------------------*/
D_UINT32 DclOsSemaphoreInfo(
    PDCLOSSEMAPHORE pOsSem)
{
    if(pOsSem)
    {
        return pOsSem->ulCurrentCount;
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
    DclAssert(pOsSem);

    if(pOsSem->ulCurrentCount >= pOsSem->ulMaxCount)
    {
        DCLPRINTF(1, ("DclOsSemaphoreRelease() attempting to release a semaphore which is not acquired\n"));
        DclError();
        return DCLSTAT_SEMAPHORERELEASEFAILED;
    }

    pOsSem->ulCurrentCount++;

    return DCLSTAT_SUCCESS;
}

