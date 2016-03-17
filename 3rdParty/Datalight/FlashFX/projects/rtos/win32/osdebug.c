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

    This module contains the Win32 OS Services default implementations
    for:

        DclOsDebugBreak()

    Should these routines need customization for your project, copy this
    module into the Project Directory, make your changes, and modify the
    master product's make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osdebug.c $
    Revision 1.5  2009/04/10 00:08:28Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.4  2009/02/09 01:45:50Z  garyp
    Merged from the v4.0 branch.  Use the new DclOutputString() calling
    convention.
    Revision 1.3  2008/07/23 23:31:03Z  keithg
    Replaced inline assembly with OS calls to initiate a debug break.
    Revision 1.2  2007/11/03 23:50:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2007/04/01 22:08:26Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <conio.h>

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
        case DCLDBGCMD_CAPABILITIES | DCLDBGCMD_BREAK:
        case DCLDBGCMD_CAPABILITIES | DCLDBGCMD_QUERY:
        case DCLDBGCMD_CAPABILITIES | DCLDBGCMD_AUTOQUERY:
            return DCLSTAT_SUCCESS;

        case DCLDBGCMD_BREAK:
            __asm int 3;
            return DCLSTAT_SUCCESS;

        case DCLDBGCMD_QUERY:
        case DCLDBGCMD_AUTOQUERY:
        {
            int             chr;
            DCLTIMESTAMP    t;
            D_UINT32        ulMaxTime;

            /*  loop until the keyboard buffer is empty
            */
            while(_kbhit())
            {
                chr = _getch();
                if(chr == 0x0 || chr == 0xE0)
                    _getch();
            }

            if(ulDebugCmd == DCLDBGCMD_QUERY)
            {
                DclOutputString(NULL, "Press 'D' to invoke the debugger, or any other key to skip it...\n");
                ulMaxTime = D_UINT32_MAX;
            }
            else
            {
                DclOutputString(NULL, "Press 'D' within 10 seconds to invoke the debugger...\n");
                ulMaxTime = 10000;
            }

            chr = 0;
            t = DclTimeStamp();
            do
            {
                if(_kbhit())
                {
                    chr = _getch();
                    if(chr == 0x0 || chr == 0xE0)
                        chr = _getch();

                    break;
                }
            } while(DclTimePassed(t) < ulMaxTime);

            if(chr == 'd' || chr == 'D')
            {
                /*  Got a debugger handy?
                */
                DebugBreak();
                return DCLSTAT_SUCCESS;
            }

            return DCLSTAT_FAILURE;
        }

        default:
            return DCLSTAT_UNSUPPORTEDCOMMAND;
    }
}


