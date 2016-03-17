/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2008 Datalight, Inc.
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

    This module contains the default project startup and shutdown logic.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dclproj.c $
    Revision 1.1  2009/10/29 23:03:08Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiprv.h>


/*-------------------------------------------------------------------
    DCLPROJECTDATA

    This structure contains any proprietary project data which may
    be required.  If necessary, an OEM can modify this structure
    definition as needed.

    If access to this structure is required by multiple modules, this
    typedef should be moved into a project specific header, and that
    header can then be included by each module which needs it.

    OEM project specific code in other modules can get access to this
    data through the DCL Project handle, which is a pointer to this
    structure.  If the DCL Project handle is not available, it can be
    obtained by calling DclProjectHandle(hDclInst), where hDclInst
    may be NULL to specify default instance.
-------------------------------------------------------------------*/
typedef struct sDCLPROJECTDATA
{
    unsigned    fInitialized : 1;
} DCLPROJECTDATA;

static DCLPROJECTDATA   DclOemProjectData;


/*-------------------------------------------------------------------
    Project Services

    This lists the service initialization functions as defined in
    the project's dclconf.h file.
-------------------------------------------------------------------*/
static const DCLPFNSERVICEINIT  pfnSrvInit[] = DCL_SERVICES;


/*-------------------------------------------------------------------
    Public: DclProjectCreate()

    This function creates a project object for the specified DCL
    Instance.

    This function will cause the creation of all the services
    defined for the project.

    OEM's may modify this logic if necessary to perform additional
    startup operations (typically after the services are created).

    Parameters:
        hDclInst - The DCL instance handle.

    Returns:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclProjectCreate(
    DCLINSTANCEHANDLE   hDclInst)
{
    DCLSTATUS           dclStat;

    /*  Basic error handling, since output and asserts are
        generally NOT going to be available at this point.
    */
    if(!hDclInst)
        return DCLSTAT_PROJ_BADINSTANCEHANDLE;

    if(DclOemProjectData.fInitialized)
        return DCLSTAT_PROJ_ALREADYINITIALIZED;

    /*  Use the default project and service initialization process
    */
    dclStat = DclProjectServicesCreate(hDclInst, &DclOemProjectData, pfnSrvInit, DCLDIMENSIONOF(pfnSrvInit));

    /*  Additional OEM startup logic can be added here
    */

    if(dclStat == DCLSTAT_SUCCESS)
        DclOemProjectData.fInitialized = TRUE;

    return dclStat;
}


/*-------------------------------------------------------------------
    Public: DclProjectDestroy()

    This function destroys the project object in the specified DCL
    Instance.

    OEM's may modify this logic if necessary to perform additional
    shutdown operations.

    This function will cause the destruction of any and all services
    which may still be in use.

    Parameters:
        hDclInst - The DCL instance handle.

    Returns:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclProjectDestroy(
    DCLINSTANCEHANDLE   hDclInst)
{
    DCLSTATUS           dclStat;

    if(!hDclInst)
        return DCLSTAT_PROJ_BADINSTANCEHANDLE;

    if(!DclOemProjectData.fInitialized)
        return DCLSTAT_PROJ_CORRUPTED;

    /*  Additional OEM shutdown logic can be added here
    */

    /*  Use the default project and service destruction process
    */
    dclStat = DclProjectServicesDestroy(hDclInst, &DclOemProjectData);

    if(dclStat == DCLSTAT_SUCCESS)
        DclOemProjectData.fInitialized = FALSE;

    return dclStat;
}


