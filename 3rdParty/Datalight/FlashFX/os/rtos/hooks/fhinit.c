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

    This module contains the default implementations for:

       FfxHookDriverCreate()
       FfxHookDriverDestroy()

    Should these routines need customization for your project, copy this
    module into your project directory, make your changes, and modify
    ffxproj.mak to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhinit.c $
    Revision 1.7  2010/01/09 18:12:17Z  garyp
    Addessed Bug 2964 -- fxdelay.h is now required due to header
    refactoring.
    Revision 1.6  2009/04/08 15:09:45Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.5  2008/01/13 07:29:18Z  keithg
    Function header updates to support autodoc.
    Revision 1.4  2007/11/03 23:50:01Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2006/02/12 01:15:11Z  Garyp
    Continuing updates to the new Device and Disk model.
    Revision 1.2  2006/02/09 19:07:34Z  Pauli
    Added Sibley support.
    Revision 1.1  2005/10/06 05:44:02Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdelay.h>


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

  #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    /*  Calibrate the _sysdelay() timing loop.
    */
    return FfxDelayCalibrate(0, 0);
  #else
    return TRUE;
  #endif
}

/*-------------------------------------------------------------------
    Public: FfxHookDriverDestroy()

    This function is called once at driver instance destruction
    time, after all devices and disks have been destroyed.

    Parameters:
        pFDI - A pointer to the FFXDRIVERINFO structure.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxHookDriverDestroy(
    FFXDRIVERINFO  *pFDI)
{
    (void)pFDI;

    return;
}

