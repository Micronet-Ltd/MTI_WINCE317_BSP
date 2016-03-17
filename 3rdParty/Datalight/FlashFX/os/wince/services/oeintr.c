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

       _sysinterruptdisable()
       _sysinterruptrestore()

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    ffxproj.mak to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                 Revision History
    $Log: oeintr.c $
    Revision 1.3  2009/04/08 21:02:51Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.2  2007/11/03 23:50:12Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2005/10/02 01:34:02Z  Pauli
    Initial revision
    Revision 1.1  2005/10/02 02:34:02Z  Garyp
    Initial revision
    Revision 1.2  2005/08/21 11:31:38Z  garyp
    Documentation update.
    Revision 1.1  2005/01/28 23:46:48Z  pauli
    Initial revision
    Revision 1.1  2005/01/28 22:46:48Z  pauli
    Initial revision
    Revision 1.7  2005/01/28 21:46:47Z  GaryP
    Updated to use the new DCL headers and functions.
    Revision 1.6  2004/12/31 00:04:18Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.5  2004/01/27 02:31:10Z  garys
    Merge from FlashFXMT
    Revision 1.4.1.2  2004/01/27 02:31:10  garyp
    Modified to use the newly renamed CPU layer functions.
    Revision 1.4  2002/11/13 09:25:16Z  garyp
    Implemented according to the finalized the API.
    Revision 1.3  2002/11/12 04:23:50Z  dennis
    Added copyright and usage restrictions header to several source files.
    Revision 1.2  2002/10/28 23:38:44  garyp
    Standardized the module and function headers.
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <dlcpuapi.h>


/*-------------------------------------------------------------------
    Public: _sysinterruptdisable()

    Notes:
       This function disables interrupts.  It may be implemented such
       that it calls the operating system to perform the function, or
       it may manipulate the hardware directly using assembly language
       code in the CPU tree.

    Parameters:
        None

    Return Value:
        The old interrupt state.
-------------------------------------------------------------------*/
D_UINT32 _sysinterruptdisable(
    void)
{
    return DclCpuInterruptDisable();
}


/*-------------------------------------------------------------------
    Public: _sysinterruptrestore()

    Notes:
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
    DclCpuInterruptRestore(ulOldState);
}
