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

    This module contains the Windows CE OESL default implementations for:

       FfxHookDriverCreate()
       FfxHookDriverDestroy()

    Should these routines need customization for your project, copy this
    module into your project directory, make your changes, and modify
    ffxproj.mak to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhinit.c $
    Revision 1.4  2009/04/08 20:32:48Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.3  2007/11/03 23:50:11Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2006/02/12 01:15:10Z  Garyp
    Continuing updates to the new Device and Disk model.
    Revision 1.1  2005/10/06 05:44:04Z  Pauli
    Initial revision
    Revision 1.3  2004/12/30 23:34:27Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.2  2004/07/19 22:17:32Z  GaryP
    Eliminated the "formatbbm" command line option.
    Revision 1.1  2004/02/23 05:50:58Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>


/*-------------------------------------------------------------------
    Public: FfxHookDriverCreate()

    This function is called once at driver initialization time
    from the Driver Framework, prior to any access to the media.

    If this function returns FALSE, the driver initialization
    process will fail.

    Parameters:
        pFDI - A pointer to the FFXDRIVERINFO structure.

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
-------------------------------------------------------------------*/
D_BOOL FfxHookDriverCreate(
    FFXDRIVERINFO  *pFDI)
{
    (void)pFDI;

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: FfxHookDriverDestroy()

    This function is called once at driver instance destruction
    time, after all devices and disks have been destroyed.

    Parameters:
        pFDI - A pointer to the FFXDRIVERINFO structure.

    Return Value:
        None
-------------------------------------------------------------------*/
void FfxHookDriverDestroy(
    FFXDRIVERINFO  *pFDI)
{
    (void)pFDI;

    return;
}

