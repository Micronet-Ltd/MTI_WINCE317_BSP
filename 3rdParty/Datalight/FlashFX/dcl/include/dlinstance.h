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

    This module contains the interface to DCL Instances.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlinstance.h $
    Revision 1.3  2009/12/31 17:24:18Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.2  2009/01/18 23:06:21Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.2  2009/01/18 23:06:21Z  garyp
    Added a usage count to support multiple opens.
    Revision 1.1  2008/12/08 18:25:22Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLINSTANCE_H_INCLUDED
#define DLINSTANCE_H_INCLUDED


/*-------------------------------------------------------------------
    DCL Instances/Projects

    Type: DCLINSTANCEHANDLE

    A DCLINSTANCEHANDLE is an opaque pointer used to refer to a DCL
    instance.

    Type: DCLPROJECTHANDLE

    A DCLPROJECTHANDLE is an opaque pointer used to refer to a
    project instance.

    Type: DCLSERVICEHANDLE

    A DCLSERVICEHANDLE is an opaque pointer used to refer to a
    service instance.
-------------------------------------------------------------------*/
/* typedef struct sDCLINSTANCEDATA DCLINSTANCEDATA; */

typedef struct sDCLPROJECTDATA          *DCLPROJECTHANDLE;
typedef DCLSTATUS(*DCLPFNSERVICEINIT)   (DCLINSTANCEHANDLE hDclInst);

#define DCLFLAG_DRIVER      (0x00000001)
#define DCLFLAG_APP         (0x00000002)

#define             DclInstanceCreate           DCLFUNC(DclInstanceCreate)
#define             DclInstanceDestroy          DCLFUNC(DclInstanceDestroy)
#define             DclInstanceHandle           DCLFUNC(DclInstanceHandle)
#define             DclInstanceNumber           DCLFUNC(DclInstanceNumber)
#define             DclProjectHandle            DCLFUNC(DclProjectHandle)
#define             DclProjectServicesCreate    DCLFUNC(DclProjectServicesCreate)
#define             DclProjectServicesDestroy   DCLFUNC(DclProjectServicesDestroy)

DCLSTATUS           DclInstanceCreate(unsigned nInstNum, D_UINT32 ulFlags, DCLINSTANCEHANDLE *pHandle);
DCLSTATUS           DclInstanceDestroy(DCLINSTANCEHANDLE hInst);
DCLINSTANCEHANDLE   DclInstanceHandle(unsigned nInstNum);
unsigned            DclInstanceNumber(DCLINSTANCEHANDLE hInst);

DCLPROJECTHANDLE    DclProjectHandle(           DCLINSTANCEHANDLE hDclInst);
DCLSTATUS           DclProjectServicesCreate(   DCLINSTANCEHANDLE hDclInst, DCLPROJECTHANDLE hProject, const DCLPFNSERVICEINIT apfnServiceInit[], unsigned nServiceCount);
DCLSTATUS           DclProjectServicesDestroy(  DCLINSTANCEHANDLE hDclInst, DCLPROJECTHANDLE hProject);



#endif  /* DLINSTANCE_H_INCLUDED */

