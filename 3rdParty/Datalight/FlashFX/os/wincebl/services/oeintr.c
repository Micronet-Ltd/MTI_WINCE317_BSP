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

       _sysinterruptdisable()
       _sysinterruptrestore()

    Should these routines need customization for your project, copy this
    module into your project directory, make your changes, and modify
    ffxproj.mak to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                 Revision History
    $Log: oeintr.c $
    Revision 1.3  2008/01/25 07:23:36Z  keithg
    Comment style updates to support autodoc.
    Revision 1.2  2007/11/03 23:50:18Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2005/10/02 01:34:02Z  Pauli
    Initial revision
    Revision 1.1  2005/10/02 02:34:02Z  Garyp
    Initial revision
    Revision 1.2  2005/08/21 11:31:37Z  garyp
    Documentation update.
    Revision 1.1  2005/01/28 23:47:36Z  pauli
    Initial revision
    Revision 1.1  2005/01/28 22:47:36Z  pauli
    Initial revision
    Revision 1.3  2005/01/28 21:47:35Z  GaryP
    Updated to use the new DCL headers and functions.
    Revision 1.2  2004/12/30 23:22:02Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.1  2004/08/16 18:37:42Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <dlcpuapi.h>


/*-------------------------------------------------------------------
   Public: _sysinterruptdisable

   This function disables interrupts.  It may be implemented such
   that it calls the operating system to perform the function, or
   it may manipulate the hardware directly using assembly language
   code in the CPU tree.

    Parameters:
       None

    Return:
       The old interrupt state.
-------------------------------------------------------------------*/
D_UINT32 _sysinterruptdisable(
    void)
{
    return DclCpuInterruptDisable();
}


/*-------------------------------------------------------------------
   Public: _sysinterruptrestore

   This function restores the original interrupt state.  It may
   be implemented such that it calls the operating system to
   perform the function, or it may manipulate the hardware
   directly using assembly language code in the CPU tree.

    Parameters:
       ulOldState specifies the interrupt state to restore.

    Return:
       Nothing
-------------------------------------------------------------------*/
void _sysinterruptrestore(
    D_UINT32 ulOldState)
{
    DclCpuInterruptRestore(ulOldState);
}
