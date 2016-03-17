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

    This module contains the RTOS OESL default implementations for:

       _sysinterruptdisable()
       _sysinterruptrestore()

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    ffxproj.mak to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: oeintr.c $
    Revision 1.5  2009/04/08 20:56:05Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.4  2008/01/13 07:28:40Z  keithg
    Function header updates to support autodoc.
    Revision 1.3  2007/11/03 23:50:02Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2006/10/04 00:05:45Z  Garyp
    Updated to use the new style printf macros and functions.
    Revision 1.1  2005/10/02 01:34:00Z  Pauli
    Initial revision
    Revision 1.2  2005/08/21 11:31:36Z  garyp
    Documentation update.
    Revision 1.1  2005/01/28 23:45:20Z  pauli
    Initial revision
    Revision 1.3  2005/01/28 21:45:19Z  GaryP
    Updated to use the new DCL headers and functions.
    Revision 1.2  2004/12/31 00:04:19Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.1  2003/07/03 22:29:56Z  garyp
    Initial revision
---------------------------------------------------------------------------*/


#include <flashfx.h>
#include <dlcpuapi.h>


/*-------------------------------------------------------------------
    Public: _sysinterruptdisable()

    This function disables interrupts.  It may be implemented such
    that it calls the operating system to perform the function, or
    it may manipulate the hardware directly using assembly language
    code in the CPU tree.

    Parameters:

    Return Value:
        The old interrupt state.
-------------------------------------------------------------------*/
D_UINT32 _sysinterruptdisable(
    void)
{
    DclPrintf("FFX not implemented: _sysinterruptdisable\n");

    return 0xFFFFFFFFUL;
}


/*-------------------------------------------------------------------
    Public: _sysinterruptrestore()

    This function restores the original interrupt state.  It may
    be implemented such that it calls the operating system to
    perform the function, or it may manipulate the hardware
    directly using assembly language code in the CPU tree.

    Parameters:
        ulOldState specifies the interrupt state to restore.

    Return Value:
        None
-------------------------------------------------------------------*/
void _sysinterruptrestore(
    D_UINT32 ulOldState)
{
    DclPrintf("FFX not implemented: _sysinterruptrestore ulOldState=%lX\n",
              ulOldState);

    return;
}
