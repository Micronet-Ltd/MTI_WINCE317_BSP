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

    This header defines the general structure used to provide the external
    API functionality.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlapireq.h $
    Revision 1.6  2009/02/11 02:38:17Z  garyp
    No longer use DCLIODEVICE_BASE.
    Revision 1.5  2009/02/08 01:04:15Z  garyp
    Merged from the v4.0 branch.  Moved the definition of DLOSREQHANDLE to
    dliosys.h so it is available to code being built outside the standard
    build process.  Updated prototypes for the revamped requestor interface.
    Revision 1.4  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.3  2007/08/18 20:32:00Z  garyp
    Modified version numbers to be 32-bit values.
    Revision 1.2  2007/07/31 01:22:20Z  Garyp
    Prototype fix.
    Revision 1.1  2007/07/31 00:43:08Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLAPIREQ_H_INCLUDED
#define DLAPIREQ_H_INCLUDED

typedef enum
{
    DCLIODEVICE_RESERVED          = 0,
    DCLIODEVICE_BLOCK,
    DCLIODEVICE_FILESYSTEM,
    DCLIODEVICE_HIGHLIMIT
} DCLIODEVICE;

/*
typedef struct
{
    DCLIOREQUEST    req;
    D_UINT32        ulSignature;
    D_UINT32        ulVersion;
    char            szBuildNum[DCL_MAX_BUILDNUM_LENGTH+1];
} DCLOSREQ_VERSIONCHECK;
*/

/*---------------------------------------------------------
    Function prototypes
---------------------------------------------------------*/
#define     DclOsRequestorOpen      DCLFUNC(DclOsRequestorOpen)
#define     DclOsRequestorClose     DCLFUNC(DclOsRequestorClose)
#define     DclOsRequestorDispatch  DCLFUNC(DclOsRequestorDispatch)

DCLSTATUS   DclOsRequestorOpen(const char *pszName, DCLIODEVICE nIODevType, DCLOSREQHANDLE *phRequestor);
DCLSTATUS   DclOsRequestorClose(DCLOSREQHANDLE hReq);
DCLSTATUS   DclOsRequestorDispatch(DCLOSREQHANDLE hReq, DCLIOREQUEST *pReq);


#endif /* DLAPIREQ_H_INCLUDED */



