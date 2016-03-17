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

    This module contains code which manages services.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlservice.c $
    Revision 1.7  2010/01/08 01:03:47Z  garyp
    Undid the "const" change from the previous revision.
    Revision 1.6  2010/01/07 02:44:58Z  garyp
    Fixed a bug in DclServiceDeregister() where the service chain could
    become corrupted.  Updated so that the instance private data pointer
    is considered "const" so far as the service framework is concerned.
    Added DclServiceIsInChain() and DclServiceIsInitialized().
    Revision 1.5  2009/12/31 17:24:18Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.4  2009/11/23 17:44:54Z  johnbr
    Correct a compiler warning reported by the GCC 4.4.1 compiler 
    under Linux.
    Revision 1.3  2009/11/08 03:51:12Z  garyp
    Documentation and debug code updated -- no functional changes.
    Revision 1.2  2009/06/25 21:28:00Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.2  2009/01/10 02:48:04Z  garyp
    Added DclServiceIoctl().
    Revision 1.1  2008/12/10 16:21:36Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiprv.h>
#include <dlinstance.h>
#include <dlservice.h>

#include "dlservicefw.h"

static DCLSTATUS DispatchIoctl(DCLSERVICEHEADER *pService, DCLREQ_SERVICE *pReq);


/*-------------------------------------------------------------------
    Public: DclServiceHeaderInit()

    This function initializes a service header structure with the
    specified data.  If a pointer to the header is not supplied,
    the header will be dynamically allocated.  Headers created in
    this fashion will be automatically freed when the service is
    destroyed.

    *Note:* -- If the service header is dynamically allocated, but
               the call to DclServiceCreate() fails, the memory will
               not be automatically freed.  The caller must do this
               with DclMemFree() to avoid a memory leak.

    *Note:* -- Even if a pre-allocated structure is supplied, it
               will be completely cleared to zeros by this function.

    Parameters:
        ppService    - A pointer to a location which stores the
                       pointer to the DCLSERVICEHEADER structure.
                       If the indirect pointer is NULL, the structure
                       will be allocated on the fly, and that pointer
                       stored in this location.
        pszName      - A pointer to the null-terminated service name.
        nServiceType - The service type number (DCLSERVICE_*)
        pfnIoctl     - A pointer to a DCLPFNSERVICEIOCTL function
                       used to manage the service.  May be NULL.
        pPrivateData - A pointer to any private data to associate
                       with the service.  May be NULL.
        nFlags       - Service flags, which may be one or more of 
                       the following values:
          DCLSERVICEFLAG_THREADSAFE - Defines a service as inherently
                       thread-safe.  If a service has an IOCTL function,
                       and this flag is not defined, a mutex will be
                       created and used to serialize access through the 
                       IOCTL interface.
          DCLSERVICEFLAG_SLAVE - Denotes that the service is a slave
                       of another, and that the master service will 
                       be responsible for destroying the slave service.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclServiceHeaderInit(
    DCLSERVICEHEADER  **ppService,
    const char         *pszName,
    DCLSERVICE          nServiceType,
    DCLPFNSERVICEIOCTL  pfnIoctl,
    void               *pPrivateData,
    unsigned            nFlags)
{
    if(!ppService)
        return DCLSTAT_BADPOINTER;

    if(nFlags & ~DCLSERVICEFLAG_MASKVALID)
        return DCLSTAT_SERVICE_FLAGSBAD;

    if(!(*ppService))
    {
        *ppService = DclMemAllocZero(sizeof(**ppService));
        if(!*ppService)
            return DCLSTAT_MEMALLOCFAILED;

        /*  Set a flag that we need to auto-free the memory when
            the service is destroyed.
        */
        (*ppService)->nFlags = nFlags | DCLSERVICEFLAG_AUTOALLOCED;
    }
    else
    {
        DclMemSet(*ppService, 0, sizeof(**ppService));
        (*ppService)->nFlags = nFlags;
    }

    (*ppService)->nStrucLen     = sizeof(**ppService);
    (*ppService)->pszName       = pszName;
    (*ppService)->nType         = nServiceType;
    (*ppService)->pfnIoctl      = pfnIoctl;
    (*ppService)->pPrivateData  = pPrivateData;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclServiceCreate()

    This function creates a service for the specified DCL Instance.

    Parameters:
        hDclInst     - The DCL instance handle.  May be NULL to use
                       the default DCL instance.
        pService     - The service pointer

    Returns:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclServiceCreate(
    DCLINSTANCEHANDLE   hDclInst,
    DCLSERVICEHEADER   *pService)
{
    DCLSTATUS           dclStat;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SERVICE, 1, 0),
        "Creating service, hInst=%P pService=%P\n", hDclInst, pService));

    if(!hDclInst)
    {
        hDclInst = DclInstanceHandle(0);
        if(!hDclInst)
            return DCLSTAT_INST_NOTINITIALIZED;
    }

    if(!pService)
        return DCLSTAT_BADPOINTER;

    if(pService->nStrucLen != sizeof(*pService))
        return DCLSTAT_BADSTRUCLEN;

    if(pService->nType >= DCLSERVICE_HIGHLIMIT)
        return DCLSTAT_SERVICE_NUMBERINVALID;

    if(pService->nFlags & DCLSERVICEFLAG_MASKRESERVED)
        return DCLSTAT_SERVICE_FLAGSBAD;

    pService->hDclInst = hDclInst;

    /*  If the service has an IOCTL function, call the CREATE method
    */
    if(pService->pfnIoctl)
    {
        DCLREQ_SERVICE  req = {{DCLIOFUNC_SERVICE_CREATE, sizeof(DCLREQ_SERVICE)}};

        /*  If the service is NOT defined as thread-safe, create a mutex
            for the service.  Note that this only applies to services
            which have an IOCTL interface.
        */
        if(!(pService->nFlags & DCLSERVICEFLAG_THREADSAFE))
        {
            char    szMutexName[DCL_MUTEXNAMELEN];

            DclSNPrintf(szMutexName, sizeof(szMutexName), D_PRODUCTPREFIX"SV%1x%02x",
                hDclInst->nInstNum, pService->nType);

            pService->pServiceMutex = DclMutexCreate(szMutexName);
            if(!pService->pServiceMutex)
                return DCLSTAT_MUTEXCREATEFAILED;
        }
        else
        {
            /*  If the service IS inherently threadsafe, then the recursion
                count stuff is not used.
            */
            DclAssert(pService->nMaxRecurse == 0);
        }

        dclStat = (*pService->pfnIoctl)(pService, &req, pService->pPrivateData);
        if(dclStat != DCLSTAT_SUCCESS)
            goto CreateCleanup;
    }

    /*  Use the system mutex to protect the service chain while we
        hook it in.
    */
    dclStat = DclOsMutexAcquire(NULL);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        /*  Add the service to the master list of services, at the head.
            This list is used to ensure that all services are destroyed
            in the reverse order of creation.
        */
        pService->pNext = hDclInst->hServiceHead;
        hDclInst->hServiceHead = pService;

        hDclInst->nServices++;

        dclStat = DclOsMutexRelease(NULL);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            /*  Now register the service relative to the service type.
            */
            dclStat = DclServiceRegister(pService);
        }
        else
        {
            /*  This really should just NOT happen...

                Remove it from the master service list.
            */
            DclOsMutexAcquire(NULL);
            {
                hDclInst->hServiceHead = pService->pNext;
                hDclInst->nServices--;
            }
            DclOsMutexRelease(NULL);
        }
    }

  CreateCleanup:

    /*  If things did not work, and there is an IOCTL function,
        call the DESTROY method.
    */
    if((dclStat != DCLSTAT_SUCCESS) && pService->pfnIoctl)
    {
        DCLREQ_SERVICE  req = {{DCLIOFUNC_SERVICE_DESTROY, sizeof(DCLREQ_SERVICE)}};

        /*  Ignore the error code from this and preserve the original
        */
        (*pService->pfnIoctl)(pService, &req, pService->pPrivateData);

        if(pService->pServiceMutex)
        {
            DclMutexDestroy(pService->pServiceMutex);
            pService->pServiceMutex = NULL;
        }
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Public: DclServiceDestroy()

    This function destroys a service for the specified DCL Instance.

    Parameters:
        pService     - The service pointer

    Returns:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclServiceDestroy(
    DCLSERVICEHEADER   *pService)
{
    DCLSERVICEHEADER   *pPrev;
    DCLSERVICEHEADER   *pThis;
    DCLSTATUS           dclStat;

    dclStat = DclServiceValidate(pService);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    if(pService->nFlags & DCLSERVICEFLAG_REGISTERED)
    {
        /*  Remove the service from the list of handlers for a
            given service type.
        */
        dclStat = DclServiceDeregister(pService);
        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;
    }

    /*  If there is an IOCTL function, call the DESTROY method.
    */
    if(pService->pfnIoctl)
    {
        DCLREQ_SERVICE  req = {{DCLIOFUNC_SERVICE_DESTROY, sizeof(DCLREQ_SERVICE)}};

        dclStat = (*pService->pfnIoctl)(pService, &req, pService->pPrivateData);
        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;

        if(pService->pServiceMutex)
        {
            DclMutexDestroy(pService->pServiceMutex);
            pService->pServiceMutex = NULL;
        }
    }

    /*  Use the system mutex to protect things while we remove the
        service from the master service list.
    */
    dclStat = DclOsMutexAcquire(NULL);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    pPrev = NULL;
    pThis = pService->hDclInst->hServiceHead;

    while(pThis)
    {
        if(pThis == pService)
        {
            /*  Remove the service from the linked list
            */
            if(pPrev)
                pPrev->pNext = pService->pNext;
            else
                pService->hDclInst->hServiceHead = pService->pNext;

            pService->hDclInst->nServices--;

            break;
        }

        pPrev = pThis;
        pThis = pThis->pNext;
    }

    dclStat = DclOsMutexRelease(NULL);

    if(pThis)
    {
        DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SERVICE, 1, 0),
            "Destroyed service %P, type %2u \"%s\"\n", pService, pService->nType, pService->pszName));

        /*  Free the memory if it was automatically allocated
        */
        if(pService->nFlags & DCLSERVICEFLAG_AUTOALLOCED)
            DclMemFree(pService);
    }
    else
    {
        DCLPRINTF(1, ("Error destroying service %P, type %2u \"%s\" : Service not found\n",
            pService, pService->nType, pService->pszName));

        dclStat = DCLSTAT_SERVICE_DESTROYFAILED;
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Public: DclServiceDestroyAll()

    This function destroys all services, or all transient services,
    in reverse order of creation.

    Project services are those which are automatically created
    during the course of project creation, and have the
    DCLSERVICEFLAG_PROJECT flag set.

    Transient services are those which are created ad hoc, after
    the project was created.

    Parameters:
        hDclInst   - The DCL instance handle.  May be NULL to use
                     the default DCL instance.
        fTransient - If TRUE, then destroy only those services which
                     are transient (created after the automatic
                     project services were created).  If FALSE, then
                     all the services are destroyed.

    Returns:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclServiceDestroyAll(
    DCLINSTANCEHANDLE   hDclInst,
    D_BOOL              fTransient)
{
    DCLSTATUS           dclStat;
    DCLSERVICEHANDLE    hCurrent;

    if(!hDclInst)
    {
        hDclInst = DclInstanceHandle(0);
        if(!hDclInst)
            return DCLSTAT_INST_NOTINITIALIZED;
    }

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SERVICE, 1, 0),
        "Destroying %s services for DCL Instance %lX\n", fTransient ? "transient" : "all", hDclInst));

    /*  hServiceHead is always the most recently created service.  We always
        destroy in reverse order of creation, except that any slave services
        are ideally destroyed by their masters.  Do all the non-slave services
        first -- if there are any leftover, do them last.
    */
    hCurrent = hDclInst->hServiceHead;
    while(hCurrent)
    {
        DCLSERVICEHANDLE    hNext = hCurrent->pNext;

        /*  If we are only supposed to destroy transient services, and we
            found a service which has the DCLSERVICEFLAG_PROJECT flag set,
            then we are done (all project services are always created before
            any transient services are created).
        */
        if(fTransient && (hCurrent->nFlags & DCLSERVICEFLAG_PROJECT))
            break;

        if(!(hCurrent->nFlags & DCLSERVICEFLAG_SLAVE))
        {
            dclStat = DclServiceDestroy(hCurrent);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;
        }

        hCurrent = hNext;
    }

    /*  If we were doing transient services only -- we're done
    */
    if(fTransient)
        return DCLSTAT_SUCCESS;

    /*  If there are any orphaned slave service left (should NOT be),
        destroy them.
    */
    while(hDclInst->hServiceHead)
    {
        DCLPRINTF(1, ("Destroying orphaned service %P\n", hDclInst->hServiceHead));

        /*  Should only be slave services left (if any)
        */
        DclAssert(hDclInst->hServiceHead->nFlags & DCLSERVICEFLAG_SLAVE);

        dclStat = DclServiceDestroy(hDclInst->hServiceHead);
        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclServiceRegister()

    This function registers a service with the specified DCL
    instance.

    Parameters:
        hService     - The service handle

    Returns:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclServiceRegister(
    DCLSERVICEHANDLE    hService)
{
    DCLSTATUS           dclStat;

    dclStat = DclServiceValidate(hService);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    if(hService->nFlags & DCLSERVICEFLAG_REGISTERED)
         return DCLSTAT_SERVICE_ALREADYREGISTERED;

    if(hService->pPrevByType)
         return DCLSTAT_SERVICE_CORRUPTED;

    /*  Use the system mutex to protect the linked list
    */
    dclStat = DclOsMutexAcquire(NULL);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    /*  Link the service in at the head of the list for the specific type
    */
    hService->pPrevByType = hService->hDclInst->ahService[hService->nType];
    hService->hDclInst->ahService[hService->nType] = hService;
    hService->nFlags |= DCLSERVICEFLAG_REGISTERED;

    dclStat = DclOsMutexRelease(NULL);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        /*  If the service has an IOCTL function, call the REGISTER method
        */
        if(hService->pfnIoctl)
        {
            DCLREQ_SERVICE  req = {{DCLIOFUNC_SERVICE_REGISTER, sizeof(DCLREQ_SERVICE)}};

            dclStat = (*hService->pfnIoctl)(hService, &req, hService->pPrivateData);
        }
    }

    if(dclStat != DCLSTAT_SUCCESS)
    {
        hService->hDclInst->ahService[hService->nType] = hService->pPrevByType;
        hService->pPrevByType = NULL;
        hService->nFlags &= ~DCLSERVICEFLAG_REGISTERED;

        DCLPRINTF(1, ("Service %P, type %2u \"%s\" registration failed with status %lX\n",
            hService, hService->nType, hService->pszName, dclStat));
   }
   else
   {
      #if D_DEBUG
        /*  If we are registering, display the message <after> doing so...
        */
        if(hService->pPrevByType)
        {
            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SERVICE, 1, 0),
                "Registered service %P, type %2u \"%s\" replacing service %P\n",
                hService, hService->nType, hService->pszName, hService->pPrevByType));
        }
        else
        {
            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SERVICE, 1, 0),
                "Registered new service %P, type %2u \"%s\"\n",
                hService, hService->nType, hService->pszName));
        }
      #endif
   }

    return dclStat;
}


/*-------------------------------------------------------------------
    Public: DclServiceDeregister()

    This function deregisters a service.

    Parameters:
        hService     - The service handle

    Returns:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclServiceDeregister(
    DCLSERVICEHANDLE    hService)
{
    DCLSERVICEHEADER   *pService;
    DCLSERVICEHEADER   *pPrev;
    DCLSTATUS           dclStat;

    dclStat = DclServiceValidate(hService);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    if( (hService->nFlags & DCLSERVICEFLAG_REGISTERED) == 0 )
         return DCLSTAT_SERVICE_NOTREGISTERED;

    if(hService->pfnIoctl)
    {
        DCLREQ_SERVICE  req = {{DCLIOFUNC_SERVICE_DEREGISTER, sizeof(DCLREQ_SERVICE)}};

        dclStat = (*hService->pfnIoctl)(hService, &req, hService->pPrivateData);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DCLPRINTF(1, ("De-registering service %P, type %2u \"%s\" failed, Status=%lX\n",
                hService, hService->nType, hService->pszName, dclStat));

            return dclStat;
        }
    }

    /*  If we are de-registering, display the message prior to doing so...
    */
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SERVICE, 1, 0),
        "De-registering service %P, type %2u \"%s\"\n", hService, hService->nType, hService->pszName));

    /*  Use the system mutex to protect the linked list
    */
    dclStat = DclOsMutexAcquire(NULL);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    pPrev = NULL;
    pService = hService->hDclInst->ahService[hService->nType];

    while(pService)
    {
        if(pService == hService)
        {
            /*  Remove the service from the linked list
            */
            if(pPrev)
                pPrev->pPrevByType = pService->pPrevByType;
            else
                hService->hDclInst->ahService[hService->nType] = pService->pPrevByType;

            hService->nFlags &= ~DCLSERVICEFLAG_REGISTERED;

            break;
        }

        pPrev = pService;
        pService = pService->pPrevByType;
    }

    dclStat = DclOsMutexRelease(NULL);

    if(pService)
    {
        DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SERVICE, 1, 0),
            "De-registered service %P, type %2u \"%s\" replaced with service %P\n",
            hService, hService->nType, hService->pszName, pService->pPrevByType));
    }
    else
    {
        DCLPRINTF(1, ("Error de-registering service %P, type %2u \"%s\" : Service not registered\n",
            hService, hService->nType, hService->pszName));

        dclStat = DCLSTAT_SERVICE_DEREGISTERFAILED;
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Public: DclServiceIsInitialized()

    Report whether a given service is already initialized or not.

    Parameters:
        hDclInst    - The DCL instance handle to use.
        pszDevName  - A pointer to the block device name to use.
        pDevInfo    - A pointer to the DCLBLOCKDEVINFO structure
                      to fill.

    Return Value:
        Returns a DCLSTAT_SUCCESS if the service is initialized, or
        a standard status code if not.
-------------------------------------------------------------------*/
DCLSTATUS DclServiceIsInitialized(
    DCLINSTANCEHANDLE       hDclInst,
    DCLSERVICE              nServiceType)
 {
    DCLSERVICEHANDLE        hService = NULL;

    if(!hDclInst)
    {
        hDclInst = DclInstanceHandle(0);
        if(!hDclInst)
            return DCLSTAT_INST_NOTINITIALIZED;
    }

    return DclServicePointer(hDclInst, nServiceType, &hService);
}


/*-------------------------------------------------------------------
    Public: DclServiceIsInChain()

    Report whether a given service is in the chain of previous 
    services of the same type.

    Parameters:
        hService       - The starting service where the search should
                         start.
        hSearchService - The service for which to search.

    Return Value:
        Returns TRUE if the hSearchService was found in the chain,
        or FALSE if not.
-------------------------------------------------------------------*/
D_BOOL DclServiceIsInChain(
    DCLSERVICEHANDLE        hService,
    DCLSERVICEHANDLE        hSearchService)
{
    DclAssertReadPtr(hService, sizeof(*hService));
    DclAssertReadPtr(hSearchService, sizeof(*hSearchService));
    
    do
    {
        if(hService->pPrevByType == hSearchService)
            return TRUE;

        hService = hService->pPrevByType;
    }
    while(hService);

    return FALSE;
}


/*-------------------------------------------------------------------
    Public: DclServicePointer()

    Retrieve a service pointer for a given service type.  If the
    supplied value at *phService is NULL, then the service returned
    is that which was most recently registered for the service type.
    If the supplied value at *phService is not NULL, then it must be
    a service handle, and the returned value will the the last 
    service which was registered prior to that service.

    Parameters:
        hDclInst     - The DCL instance handle.  May be NULL to use
                       the default DCL instance.
        nServiceType - The DCLSERVICE_* number.
        phService    - A pointer to a location in which to store the
                       service handle.  The passed in value must be
                       NULL or a valid service handle, as described
                       above.

    Returns:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclServicePointer(
    DCLINSTANCEHANDLE   hDclInst,
    DCLSERVICE          nServiceType,
    DCLSERVICEHANDLE   *phService)
{
    DCLSTATUS           dclStat;
    DCLSERVICEHANDLE    hService;

    if(!hDclInst)
    {
        hDclInst = DclInstanceHandle(0);
        if(!hDclInst)
            return DCLSTAT_INST_NOTINITIALIZED;
    }

    if(nServiceType >= DCLSERVICE_HIGHLIMIT)
        return DCLSTAT_SERVICE_NUMBERINVALID;

    if(!phService)
        return DCLSTAT_BADPOINTER;

    /*  Use the system mutex to protect the linked list
    */
    dclStat = DclOsMutexAcquire(NULL);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    if(*phService == NULL)
        hService = hDclInst->ahService[nServiceType];
    else
        hService = (*phService)->pPrevByType;

    dclStat = DclOsMutexRelease(NULL);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    if(!hService)
    {
        if(*phService == NULL)
            return DCLSTAT_SERVICE_NOTREGISTERED;
        else
            return DCLSTAT_SERVICE_NOPREVIOUSSERVICE;
    }

    *phService = hService;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclServiceData()

    This function retrieves the private data pointer for the current
    service of a given type.

    Parameters:
        hDclInst      - The DCL instance handle.  May be NULL to use
                        the default DCL instance.
        nServiceType  - The DCLSERVICE_* number.
        ppServiceData - A pointer to a location in which to store the
                        service data pointer.

    Returns:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclServiceData(
    DCLINSTANCEHANDLE   hDclInst,
    DCLSERVICE          nServiceType,
    void              **ppServiceData)
{
    DCLSTATUS           dclStat;
    DCLSERVICEHANDLE    hService = NULL;

    dclStat = DclServicePointer(hDclInst, nServiceType, &hService);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    DclAssert(hService);

    *ppServiceData = hService->pPrivateData;

    return dclStat;
}


/*-------------------------------------------------------------------
    Public: DclServiceDataPrevious()

    Retrieve the private data pointer for the previous service of
    the same type, relative to the supplied service handle.

    Parameters:
        hService      - The service handle.
        ppServiceData - A pointer to a location in which to store the
                        service data pointer for the previous service.

    Returns:
        Returns a DCLSTATUS code indicating the results.  Note that
        even if this function returns DCLSTAT_SUCCESS, the previous
        service's data pointer may still be NULL.
-------------------------------------------------------------------*/
DCLSTATUS DclServiceDataPrevious(
    DCLSERVICEHANDLE    hService,
    void              **ppServiceData)
{
    DCLSTATUS           dclStat;

    dclStat = DclServiceValidate(hService);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    if(hService->pPrevByType)
    {
        *ppServiceData = hService->pPrevByType->pPrivateData;

        /*  Reminder that even though we're returning "success", the
            previous service's private data pointer might still be NULL.
        */
        return dclStat;
    }
    else
    {
        return DCLSTAT_SERVICE_NOPREVIOUSSERVICE;
    }
}


/*-------------------------------------------------------------------
    Public: DclServiceIoctl()

    This function dispatches an IOCTL request to the most recently
    registered service of the specified type.

    Parameters:
        hDclInst      - The DCL instance handle.  May be NULL to use
                        the default DCL instance.
        nServiceType  - The DCLSERVICE_* number.
        pRequest      - A pointer to the DCLIOREQUEST structure
                        containing the IOCTL request.

    Returns:
        Returns a DCLSTATUS code indicating the results.  Note that
        this status code indicates the status of the requestor
        interface.  The actual results of the request itself are
        typically returned in the request packet.
-------------------------------------------------------------------*/
DCLSTATUS DclServiceIoctl(
    DCLINSTANCEHANDLE   hDclInst,
    DCLSERVICE          nServiceType,
    DCLIOREQUEST       *pRequest)
{
    DCLSTATUS           dclStat;
    DCLSERVICEHEADER   *pService = NULL;
    DCLREQ_SERVICE      req = {{DCLIOFUNC_SERVICE_DISPATCH, sizeof(DCLREQ_SERVICE)}};

    if(!pRequest)
        return DCLSTAT_SERVICE_BADREQUEST;

    /*  The packet length will typically be larger than the header
        length, but it certainly cannot be less...
    */
    if(pRequest->ulReqLen < sizeof(*pRequest))
        return DCLSTAT_BADSTRUCLEN;

    /*  Get the most recently registered service for this type
    */
    dclStat = DclServicePointer(hDclInst, nServiceType, &pService);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    DclAssert(pService);

    if(!pService->pfnIoctl)
        return DCLSTAT_SERVICE_NOIOCTLINTERFACE;

    req.pSubRequest = pRequest;

    return DispatchIoctl(pService, &req);
}


/*-------------------------------------------------------------------
    Public: DclServiceIoctlPrevious()

    Call the IOCTL function for the previous service of the same
    type as hService, passing the request packet in pReq.

    Parameters:
        hService - The current service.
        pReq     - An IOCTL request packet for the current service.

    Returns:
        Returns a DCLSTATUS code indicating the results.  Note that
        this status code indicates the status of the requestor
        interface.  The actual results of the request itself are
        typically returned in the request packet.
-------------------------------------------------------------------*/
DCLSTATUS DclServiceIoctlPrevious(
    DCLSERVICEHANDLE    hService,
    DCLREQ_SERVICE     *pReq)
{
    DCLSTATUS           dclStat;
    DCLSERVICEHEADER   *pService = hService;

    if(!hService)
        return DCLSTAT_SERVICE_BADHANDLE;

    if(!pReq)
        return DCLSTAT_SERVICE_BADREQUEST;

    /*  Get the previously registered service for this type
    */
    dclStat = DclServicePointer(hService->hDclInst, hService->nType, &pService);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    DclAssert(pService);

    if(!pService->pfnIoctl)
        return DCLSTAT_SERVICE_NOIOCTLINTERFACE;

    return DispatchIoctl(pService, pReq);
}


/*-------------------------------------------------------------------
    Public: DclServiceValidate()

    Ensure that the specified service is valid.

    Parameters:
        hService - The handle of the service to examine.

    Returns:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclServiceValidate(
    DCLSERVICEHANDLE    hService)
{
    if(!hService)
        return DCLSTAT_SERVICE_BADHANDLE;

    if(hService->nStrucLen != sizeof(*hService))
        return DCLSTAT_BADSTRUCLEN;

    if(hService->nType >= DCLSERVICE_HIGHLIMIT)
        return DCLSTAT_SERVICE_NUMBERINVALID;

    if(!hService->hDclInst)
        return DCLSTAT_INST_BADHANDLE;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: DispatchIoctl()

    Parameters:
        hService - The handle of the service to examine.

    Returns:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS DispatchIoctl(
    DCLSERVICEHEADER   *pService,
    DCLREQ_SERVICE     *pReq)
{
    DCLSTATUS           dclStat;

    /*  If pServiceMutex is initialized, then the service is protected
        by a mutex.
    */
    if(pService->pServiceMutex)
    {
        if(!DclMutexAcquire(pService->pServiceMutex))
            return DCLSTAT_MUTEXACQUIREFAILED;

        /*  Don't allow recursion to any level deeper than that specified.
        */
        if(pService->nRecurseCount > pService->nMaxRecurse)
        {
            DclMutexRelease(pService->pServiceMutex);
            return DCLSTAT_SERVICE_RECURSEFAILED;
        }

        pService->nRecurseCount++;

        /*  Better not have wrapped
        */
        DclAssert(pService->nRecurseCount);
    }

    /*  Perform the IOCTL function
    */
    dclStat = (*pService->pfnIoctl)(pService, pReq, pService->pPrivateData);

    if(pService->pServiceMutex)
    {
        DclAssert(pService->nRecurseCount);
        pService->nRecurseCount--;

        /*  Ensure that any original error code is returned
        */
        if(!DclMutexRelease(pService->pServiceMutex) && dclStat == DCLSTAT_SUCCESS)
            dclStat = DCLSTAT_MUTEXRELEASEFAILED;
    }

    return dclStat;
}


