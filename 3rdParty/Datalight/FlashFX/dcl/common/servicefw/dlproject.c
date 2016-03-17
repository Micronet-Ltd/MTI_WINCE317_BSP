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

    This module contains code to create and destroy a set of services for a
    project.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlproject.c $
    Revision 1.3  2009/12/31 17:24:18Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.2  2009/11/10 20:35:21Z  garyp
    Updated to accommodate NULLs in the service pointer list.
    Revision 1.1  2008/12/08 21:29:12Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlinstance.h>
#include <dlapiprv.h>

#include "dlservicefw.h"


/*-------------------------------------------------------------------
    Public: DclProjectHandle()

    This function obtains a Project handle given a DCL Instance
    handle.

    Parameters:
        hDclInst  - The DCL Instance handle.  If this value is NULL,
                    the default DCL Instance will be used.

    Return Value:
        Returns the DCLPROJECTHANDLE if successful, or NULL
        otherwise.
-------------------------------------------------------------------*/
DCLPROJECTHANDLE DclProjectHandle(
    DCLINSTANCEHANDLE       hDclInst)
{
    if(!hDclInst)
    {
        hDclInst = DclInstanceHandle(0);
        if(!hDclInst)
            return NULL;
    }

    return hDclInst->hProject;
}


/*-------------------------------------------------------------------
    Public: DclProjectServicesCreate()

    This function creates all the services specfied in the array
    apfnServiceInit[], in the order specified.  The failure of any
    service to initialize will cause the operation to abort, and
    any created services will be destroyed in reverse order of
    creation.

    NOTE: A service which fails with DCLSTAT_FEATUREDISABLED is
          allowed.

    Parameters:
        hDclInst        - The DCL Instance handle
        hProject        - The Project handle
        apfnServiceInit - An array of service init pointers
        nServiceCount   - The number of elements in apfnServiceInit[]

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclProjectServicesCreate(
    DCLINSTANCEHANDLE       hDclInst,
    DCLPROJECTHANDLE        hProject,
    const DCLPFNSERVICEINIT apfnServiceInit[],
    unsigned                nServiceCount)
{
    DCLSTATUS               dclStat;
    unsigned                nn;

    /*  Basic error handling, since output and asserts are
        generally NOT going to be available at this point.
    */

    if(!hDclInst)
        return DCLSTAT_PROJ_BADINSTANCEHANDLE;

    if(!hProject)
        return DCLSTAT_PROJ_BADHANDLE;

    /*  Initialize the project handle now, prior to creating the services,
        so that any custom services an OEM might implement, can have access
        to the project data.
    */
    hDclInst->hProject = hProject;

    for(nn=0; nn<nServiceCount; nn++)
    {
        if(!apfnServiceInit[nn])
        {
            /*  If any pointer except the final one is NULL, print a warning
            */                
            if(nn < nServiceCount-1)
                DCLPRINTF(1, ("DclProjectServicesCreate() ignoring NULL service in slot #%u\n", nn));

            continue;
        }
        
        /*  Initialize the service
        */
        dclStat = (*(apfnServiceInit[nn]))(hDclInst);

        /*  Every service must initialize successully, or use the formal
            status code to indicate that it is a disabled feature.
            Anything else causes the initialization process to abort.
        */
        if(dclStat != DCLSTAT_SUCCESS && dclStat != DCLSTAT_FEATUREDISABLED)
        {
            /*  Something failed, so destroy whatever services were created.

                We're going to ignore any error codes which could be generated
                by the service destruction process, and rather preserve the
                original status code.
            */
            DclServiceDestroyAll(hDclInst, FALSE);

            /*  Mark the project as uninitialized (even though technically
                a service could have failed to go away).
            */
            hDclInst->hProject = NULL;

            return dclStat;
        }

        /*  On to the next service...
        */
    }

    hDclInst->fServicesInited = TRUE;

    /*  By the time we get to this point, everything must be initialized
        to do basic things like displaying output and asserting.
    */
    DCLPRINTF(1, ("Created Project %P with %u default services\n",
        hProject, hDclInst->nServices));

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclProjectServicesDestroy()

    This function destroys any and all services which are currently
    installed, in reverse order of creation.  Note that this includes
    not only the "project" services which were automatically created
    at project create time, but also "transient" services which may
    have been created at a later time.

    Parameters:
        hDclInst        - The DCL instance handle
        hProject        - The Project handle

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclProjectServicesDestroy(
    DCLINSTANCEHANDLE   hDclInst,
    DCLPROJECTHANDLE    hProject)
{
    DCLSTATUS           dclStat;

    if(!hDclInst)
        return DCLSTAT_PROJ_BADINSTANCEHANDLE;

    if(hDclInst->hProject == NULL)
        return DCLSTAT_PROJ_NOTINITIALIZED;

    if(hDclInst->hProject != hProject)
        return DCLSTAT_PROJ_INVALID;

    DCLPRINTF(1, ("Destroying services for Project %P\n", hProject));

    /*  Destroy any services which were created -- in reverse
        order of creation.
    */

    dclStat = DclServiceDestroyAll(hDclInst, FALSE);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    hDclInst->fServicesInited = FALSE;

    DCLPRINTF(1, ("Destroying Project %P\n", hProject));

    hDclInst->hProject = NULL;

    return dclStat;;
}



