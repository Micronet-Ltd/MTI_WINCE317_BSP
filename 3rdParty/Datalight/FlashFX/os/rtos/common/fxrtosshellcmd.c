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

    This module contains code to add any OS-specific FlashFX commands to the
    specified shell.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxrtosshellcmd.c $
    Revision 1.2  2008/01/13 07:29:07Z  keithg
    Function header updates to support autodoc.
    Revision 1.1  2007/12/15 02:11:26Z  Garyp
    Initial revision
 ---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxtools.h>
#include <dlshell.h>

#if DCLCONF_COMMAND_SHELL


/*-------------------------------------------------------------------
    Public: FfxShellAddOsCommands

    This function adds OS-specific commands to the FlashFX
    Shell.

    Parameters:
        hShell       - The shell handle.
        pszClassName - A pointer to the null-terminated command
                       class name to use.
        pszPrefix    - A pointer to the null-terminated command
                       prefix to use.

    Return:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxShellAddOsCommands(
    DCLSHELLHANDLE  hShell,
    const char     *pszClassName,
    const char     *pszPrefix)
{
    if(!hShell)
    {
        DclError();
        return FFXSTAT_BADPARAMETER;
    }

    DclAssert(pszClassName);
    DclAssert(pszPrefix);

    (void)pszPrefix;

    /*  No local FlashFX level commands for this OS, so just add any
        DCL level OS commands we might have.
    */

/*  NONE FOR RTOS RIGHT NOW....

    return DclShellAddOsCommands(hShell, pszClassName, "OS");
*/
    return FFXSTAT_SUCCESS;
}



#endif  /* DCLCONF_COMMAND_SHELL */

