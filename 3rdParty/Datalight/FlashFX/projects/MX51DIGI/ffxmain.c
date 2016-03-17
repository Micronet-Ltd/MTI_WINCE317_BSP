/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2007 Datalight, Inc.
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

    This module contains a stub project main to ensure that FXPROJ.LIB is
    created.  No changes to this module are necessary, however project
    specific code can be added here if desired.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ffxmain.c $
    Revision 1.1  2007/11/04 00:50:12Z  jimmb
    Initial revision
    Revision 1.3  2007/11/03 23:50:12Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2006/10/03 23:33:06Z  Garyp
    Added a default signon message.
    Revision 1.1  2005/10/01 11:03:54Z  Pauli
    Initial revision
    Revision 1.5  2004/12/30 23:43:25Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.4  2002/11/11 20:50:30Z  garyp
    Changed FfxProjMain() to return a D_BOOL to indicate whether the
    driver load should continue or fail.
    Revision 1.2  2002/10/28 23:31:02Z  garyp
    Standardized the header.
    Revision 1.2  2002/10/18 20:31:38Z  garyp
    Cleaned up the docs and header.
---------------------------------------------------------------------------*/

#include <windows.h>
#include <flashfx.h>


/*-------------------------------------------------------------------
    FfxProjMain()

    Description
        This function is called by the FlashFX device driver at
        initialization time, immediately prior to the display of
        the FlashFX copyright messages.

    Parameters
        None

    Return Value
        Returns TRUE if the driver is to continue loading, or FALSE
        to cause the driver load process to be cancelled.
-------------------------------------------------------------------*/
D_BOOL FfxProjMain(void)
{
    DclPrintf("New FlashFX project\n");

    return TRUE;
}
