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

    This module contains the Windows CE BootLoader OS Services default
    implementations for:

        DclOsDebugBreak()

    Should these routines need customization for your project, copy this
    module into the Project Directory, make your changes, and modify the
    master product's make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osdebug.c $
    Revision 1.3  2009/04/10 02:01:32Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.2  2007/11/03 23:31:38Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/04/01 21:58:40Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Public: DclOsDebugBreak()

    Break into the debugger, or query debugger capabilities.

    This function provides a generalized ability to support
    primitive operations for entering a debugger.

    The available debug commands values are...

    DCLDBGCMD_BREAK - Immediately enter the debugger.  If this
        capability is supported, and execution returns from this
        function, the value DCLSTAT_SUCCESS will be returned.
        If this function is not supported, DCLSTAT_FAILURE will
        be returned.
    DCLDBGCMD_QUERY - Ask the user if he wants to enter the
        debugger.  Returns DCLSTAT_SUCCESS if the user chooses to
        enter the debugger and then continues execution.  If the
        user chooses not to enter the debugger, or if this
        capability is not supported, the value DCLSTAT_FAILURE is
        returned.
    DCLDBGCMD_AUTOQUERY - Same as DCLDBGCMD_QUERY, however the
        user must respond within 10 seconds, or the default action
        of NOT entering the debugger will be taken.
    DCLDBGCMD_CAPABILITIES - Report the capabilities of this
        function.  This is a bitmapped flag which may be combined
        with one of the other flags to query the capabilities of
        this function.  If this function supports the functionality
        represented by the specified flag, DCLSTAT_SUCCESS is
        returned, otherwise DCLSTAT_FAILURE is returned.

    Parameters:
        ulDebugCmd - The bit-mapped debug command flags, as described
                     above.

    Return Value:
        Returns a DCLSTATUS value as described above.
-------------------------------------------------------------------*/
DCLSTATUS DclOsDebugBreak(
    D_UINT32        ulDebugCmd)
{
    switch(ulDebugCmd)
    {
        /*  None of this functionality is currently supported.
        */
        case DCLDBGCMD_BREAK:
        case DCLDBGCMD_QUERY:
        case DCLDBGCMD_AUTOQUERY:
        case DCLDBGCMD_CAPABILITIES | DCLDBGCMD_BREAK:
        case DCLDBGCMD_CAPABILITIES | DCLDBGCMD_QUERY:
        case DCLDBGCMD_CAPABILITIES | DCLDBGCMD_AUTOQUERY:
            return DCLSTAT_FAILURE;

        default:
            return DCLSTAT_UNSUPPORTEDCOMMAND;
    }
}


