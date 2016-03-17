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

    This module contains version manipulation functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxver.c $
    Revision 1.6  2009/04/01 15:45:05Z  davidh
    Function Headers updated for AutoDoc.
    Revision 1.5  2008/01/13 07:27:16Z  keithg
    Function header updates to support autodoc.
    Revision 1.4  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2007/08/16 05:55:14Z  garyp
    Updated to use tje new 32-bit version number.
    Revision 1.2  2006/09/19 23:18:08Z  Garyp
    Modified to handle build numbers which are now strings rather than numbers.
    Revision 1.1  2005/08/15 05:55:04Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxver.h>


/*-------------------------------------------------------------------
    Public: FfxVersion()

    This function returns the version and build number for the
    product.

    Parameters:
        pulVersion - A pointer to a D_UINT32 in which the version
                     number will be stored, major version in the
                     high byte, minor version in the low byte.
        pszBuffer  - A pointer to a buffer in which the build
                     number string will be stored.  The buffer
                     must be at least DCL_MAX_BUILDNUM_LENGTH+1
                     bytes long.  This value may be NULL if the
                     build number value is not required.

    Return Value:
        None
-------------------------------------------------------------------*/
void FfxVersion(
    D_UINT32   *pulVersion,
    char       *pszBuffer)
{
    DclAssert(pulVersion);

    *pulVersion = VERSIONVAL;

    if(pszBuffer)
        DclStrNCpy(pszBuffer, FFXBUILDNUM, DCL_MAX_BUILDNUM_LENGTH+1);

    return;
}


