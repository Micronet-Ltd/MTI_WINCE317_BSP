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

    This module contains the Windows CE BootLoader OS Layer default
    implementations for:

       _sysdelay()

    Should this routine need customization for your project, copy this
    module into your project directory, make your changes, and modify
    ffxproj.mak to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: oedelay.c $
    Revision 1.6  2010/01/12 18:26:53Z  billr
    Resolve bug 2964: header refactoring requires including fxdelay.h
    for prototypes of FfxDelay(), FfxDelayCalibrate().
    Revision 1.5  2008/01/25 07:43:11Z  keithg
    Comment style updates to support autodoc.
    Revision 1.4  2007/11/03 23:50:18Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2006/12/16 23:30:58Z  Garyp
    Updated so that all the _sysdelay() implementations consistently do
    nothing (return immediately) if the specified delay is zero.
    Revision 1.2  2006/01/25 19:53:41Z  Garyp
    Updated to build this code if Sibley flash is in use.
    Revision 1.1  2005/10/02 01:33:54Z  Pauli
    Initial revision
    Revision 1.3  2005/09/17 23:51:24Z  garyp
    Modified so that _sysdelay() and related code is only built if
    FFXCONF_NORSUPPORT is TRUE.
    Revision 1.2  2005/08/21 11:31:29Z  garyp
    Documentation update.
    Revision 1.1  2004/12/31 01:22:06Z  pauli
    Initial revision
    Revision 1.3  2004/12/30 23:22:06Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.2  2004/08/28 23:23:46Z  GaryP
    Changed to use the FlashFX internal self-calibrating delay function.
    Revision 1.1  2004/08/16 18:37:42Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdelay.h>

#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT


/*-------------------------------------------------------------------
    Public: _sysdelay

    This function delays until the specified number of
    microseconds have elapsed.

    This function is heavily used by the NOR FIMs and MUST be
    accurate.  Insufficient delay can cause malfunction by
    violating flash chip specifications, while excessive delay
    is likely to degrade overall system performance.

    Parameters:
        ulMicroSeconds - Number of microseconds to delay.  It is
                         legal for this value to be zero, in which
                         case the function should simply return.

    Return:
        None
-------------------------------------------------------------------*/
void _sysdelay(
    D_UINT32 ulMicroSeconds)
{
    if(ulMicroSeconds)
        FfxDelay(ulMicroSeconds);
}

#endif
