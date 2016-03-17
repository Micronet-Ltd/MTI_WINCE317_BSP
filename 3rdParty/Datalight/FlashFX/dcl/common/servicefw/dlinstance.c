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

    This module contains functions for managing DCL Instances.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlinstance.c $
    Revision 1.7.1.2  2011/11/30 20:00:39Z  billr
    Make messages about DCL instance usage counts debug level 2, not 1.
    Revision 1.7  2010/04/28 23:31:28Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.6  2009/12/31 17:24:18Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.5  2009/11/07 23:10:31Z  garyp
    Updated to allow multiple initializations of the same DCL instance
    even if the APP versus DRIVER flags are different -- so long as the
    other flags are the same.
    Revision 1.4  2009/11/03 13:27:56Z  garyp
    Fixed a broken debug message -- no functional changes.
    Revision 1.3  2009/09/02 17:25:57Z  billr
    Fix one-byte buffer overrun.
    Revision 1.2  2009/06/23 03:06:39Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.2  2009/01/18 23:06:10Z  garyp
    Added a usage count to support multiple opens.
    Revision 1.1  2008/12/10 16:22:46Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiprv.h>
#include <dlinstance.h>

#include "dlservicefw.h"

static unsigned         fSysMutexInited = FALSE;
static unsigned         nInstanceCount = 0;
static DCLINSTANCEDATA  DclInst[DCL_MAX_INSTANCES];


/*-------------------------------------------------------------------
    Public: DclInstanceCreate()

    This function creates a DCL Instance.  This function must be
    called by any application or driver which uses DCL, and must
    be called <prior> to making any other calls into DCL.

    This function will cause calls to DclProjectCreate() and in turn,
    creation of all the services defined for the project.

    A DCL Instance can be recreated, so long as the ulFlags value
    is identical.  The original services and project creation is
    untouched.  A usage counter is incremented and the same DCL
    handle will be returned.

    Parameters:
        nInstNum - The DCL instance number, which must be a value
                   from 0 to DCL_MAX_INSTANCES-1.
        ulFlags  - The DCLFLAGS_* values to use.
        pHandle  - A pointer to a buffer in which the DCL instance
                   handle will be stored.  May be NULL.

    Returns:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclInstanceCreate(
    unsigned            nInstNum,
    D_UINT32            ulFlags,
    DCLINSTANCEHANDLE  *pHandle)
{
    DCLSTATUS           dclStat;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_SERVICE, 1, 0),
        "Creating DCL instance #%u Flags=%lX pHandle=%P\n", nInstNum, ulFlags, pHandle));

    /*  Basic error handling, since output and asserts are
        generally NOT going to be available at this point.
    */

    if(nInstNum >= DCL_MAX_INSTANCES)
        return DCLSTAT_INST_OUTOFRANGE;

    /*  Check if we are recreating an instance which already exists
    */
    if(DclInst[nInstNum].fInited)
    {
        /*  Recreating an instance is OK if the "APP" versus "DRIVER" flags
            are different, however all other flags bits must match.
        */            
        if((DclInst[nInstNum].ulFlags & ~(DCLFLAG_APP | DCLFLAG_DRIVER)) == 
            (ulFlags & ~(DCLFLAG_APP | DCLFLAG_DRIVER)))
        {
            D_UINT32 ulUsage;

            ulUsage = DclOsAtomic32Increment(&DclInst[nInstNum].ulUsageCount);

            DclAssert(ulUsage > 1);

            DclInst[nInstNum].ulFlags |= ulFlags;

            if(pHandle)
                *pHandle = &DclInst[nInstNum];

            DCLPRINTF(2, ("DclInstanceCreate() Incremented DCL Instance %P usage count to %lU, Flags=%lX\n",
                &DclInst[nInstNum], ulUsage, DclInst[nInstNum].ulFlags));

            (void)ulUsage;

            return DCLSTAT_SUCCESS;
        }
        else
        {
            DCLPRINTF(1, ("DclInstanceCreate() Cannot recreate a DCL Instance with different flags, Old=%lX New=%lX\n",
                DclInst[nInstNum].ulFlags, ulFlags));

            return DCLSTAT_INST_RECREATEFAILED;
        }
    }

    /*  The system mutex may be shared by more than one DCL instance
    */
    if(!fSysMutexInited)
    {
        /*  Reality check -- the instance count must be 0 if the system
            mutex is not initialized.
        */
        if(nInstanceCount != 0)
            return DCLSTAT_INST_CORRUPTED;

        dclStat = DclOsMutexCreate(NULL, D_PRODUCTPREFIX"MSYST");
        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;

        fSysMutexInited = TRUE;
    }

    dclStat = DclOsMutexAcquire(NULL);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        DclInst[nInstNum].ulUsageCount  = 1;
        DclInst[nInstNum].ulFlags       = ulFlags;
        DclInst[nInstNum].fInited       = TRUE;
        DclInst[nInstNum].nInstNum      = nInstNum;
        nInstanceCount++;

        dclStat = DclOsMutexRelease(NULL);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            dclStat = DclProjectCreate(&DclInst[nInstNum]);
            if(dclStat == DCLSTAT_SUCCESS)
            {
                DCLSERVICEHEADER   *pThis;
                unsigned            nCount = 0;
              #if D_DEBUG
                char                szServiceNames[128] = "Services:";
              #endif

                /*  Walk through all the newly initialized services and mark
                    them as "project" services.  This allows us to distinguish
                    between "project" and "transient" services to allow an
                    orderly shutdown.
                */
                pThis = DclInst[nInstNum].hServiceHead;

                while(pThis)
                {
                    nCount++;
                    pThis->nFlags |= DCLSERVICEFLAG_PROJECT;

                  #if D_DEBUG
                    if(DclSNPrintf(&szServiceNames[DclStrLen(szServiceNames)],
                        sizeof(szServiceNames) - DclStrLen(szServiceNames),
                        " %s", pThis->pszName) == -1)
                    {
                        /*  Ensure null termination in the event of overflow
                        */
                        szServiceNames[sizeof szServiceNames - 1] = 0;
                    }
                  #endif

                    pThis = pThis->pNext;
                }

                /*  Only after successfully returning out of DclProjectCreate()
                    can we use things like DCLPRINTF().
                */
                DCLPRINTF(1, ("Created DCL Instance %P InstNum=%u Flags=%lX ServiceCount=%u\n",
                    &DclInst[nInstNum], nInstNum, ulFlags, nCount));

                DCLPRINTF(1, ("%s\n", szServiceNames));

                if(pHandle)
                    *pHandle = &DclInst[nInstNum];
            }
        }
        else
        {
            DclInst[nInstNum].fInited = FALSE;
            nInstanceCount--;
        }
    }

    if(!nInstanceCount)
    {
        if(DclOsMutexDestroy(NULL) == DCLSTAT_SUCCESS)
            fSysMutexInited = FALSE;
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Public: DclInstanceDestroy()

    This function destroys a DCL Instance which was created with
    DclInstanceCreate().  Once this function returns successfully,
    no further calls to DCL functions should be made.

    This function will cause calls to DclProjectDestroy() and in turn,
    destruction of any and all services which may still be in use.

    If the DCL Instance was created more than once, the usage counter
    will decrement, and the Instance will not be destroyed.  Any
    projects and services will be destroyed only when the usage
    counter goes to zero.

    Parameters:
        hDclInst - The DCL instance handle

    Returns:
        Returns a DCLSTATUS code indicating the results.  If the
        usage count was reduced only, and the instance was not
        destroyed, DCLSTAT_INST_USAGECOUNTREDUCED will be returned.
-------------------------------------------------------------------*/
DCLSTATUS DclInstanceDestroy(
    DCLINSTANCEHANDLE   hDclInst)
{
    DCLSTATUS           dclStat;
    D_UINT32            ulUsage;

    if(hDclInst == NULL)
        return DCLSTAT_INST_BADHANDLE;

    if(!hDclInst->fInited)
        return DCLSTAT_INST_NOTINITIALIZED;

    if(!hDclInst->hProject)
        return DCLSTAT_INST_PROJECTNOTINITIALIZED;

    if(!fSysMutexInited)
        return DCLSTAT_INST_SYSMUTEXNOTINITIALIZED;

    ulUsage = DclOsAtomic32Decrement(&hDclInst->ulUsageCount);
    if(ulUsage)
    {
        DCLPRINTF(2, ("Reduced DCL Instance %P usage count to %lU\n", hDclInst, ulUsage));
        return DCLSTAT_INST_USAGECOUNTREDUCED;
    }

    DCLPRINTF(1, ("Destroying DCL Instance %P, InstNum=%u\n", hDclInst, hDclInst->nInstNum));

    dclStat = DclProjectDestroy(hDclInst);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    hDclInst->hProject = NULL;

    dclStat = DclOsMutexAcquire(NULL);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        hDclInst->fInited = FALSE;
        nInstanceCount--;

        dclStat = DclOsMutexRelease(NULL);
        if(dclStat == DCLSTAT_SUCCESS && !nInstanceCount)
        {
            /*  If we just destroyed the last DCL instance, destroy the
                OS system mutex as well.
            */
            dclStat = DclOsMutexDestroy(NULL);
            if(dclStat == DCLSTAT_SUCCESS)
                fSysMutexInited = FALSE;
        }
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Public: DclInstanceHandle()

    This function returns the DCLINSTANCEHANDLE for a given DCL
    instance number.

    Parameters:
        nInstNum - The DCL instance number, which must be a value
                   from 0 to DCL_MAX_INSTANCES-1.

    Returns:
        Returns the instance handle if successful, or NULL otherwise.
-------------------------------------------------------------------*/
DCLINSTANCEHANDLE DclInstanceHandle(
    unsigned        nInstNum)
{
    if(nInstNum >= DCL_MAX_INSTANCES)
        return NULL;

    if(!DclInst[nInstNum].fInited)
        return NULL;

    return &DclInst[nInstNum];
}


/*-------------------------------------------------------------------
    Public: DclInstanceNumber()

    This function returns the DCL instance number for a given
    DCLINSTANCEHANDLE.

    Parameters:
        hDclInst - The DCL instance handle.

    Returns:
        Returns the instance number for the handle.
-------------------------------------------------------------------*/
unsigned DclInstanceNumber(
    DCLINSTANCEHANDLE hDclInst)
{
    DclAssert(hDclInst);

    return hDclInst->nInstNum;
}




