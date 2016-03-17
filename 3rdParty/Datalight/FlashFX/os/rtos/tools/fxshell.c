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

    This module contains the FlashFX RTOS commands that can be added to the
    DCL Command Shell.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxshell.c $
    Revision 1.17  2009/11/06 18:00:20Z  garyp
    Updated to include/exclude TESTS and TOOLS based on FlashFX
    settings rather than DCL settings.
    Revision 1.16  2009/02/18 21:41:08Z  glenns
    - Added code to prevent compiler warnings about unused formal
      parameters.
    Revision 1.15  2008/01/13 07:28:51Z  keithg
    Function header updates to support autodoc.
    Revision 1.14  2007/11/03 23:50:03Z  Garyp
    Updated to use the standard module header.
    Revision 1.13  2007/08/28 17:56:13Z  Garyp
    Modified to use the new dlshell.h header and shell creation function.
    Revision 1.12  2007/06/11 21:14:27Z  timothyj
    Account for RTOS's without threading capability (BZ 1206, 1207)
    Revision 1.11  2007/02/03 23:38:13Z  Garyp
    Minor type and variable name changes so similar modules from different
    ports are easier to diff.
    Revision 1.10  2007/01/11 01:46:30Z  Garyp
    Minor function and type name changes -- nothing functional.
    Revision 1.9  2006/11/30 03:37:40Z  Garyp
    Modified to use the new shell functionality which include a command class.
    Revision 1.8  2006/09/28 01:45:26Z  garyp
    Modified so the tests and tools can be trimmed out.
    Revision 1.7  2006/08/22 16:48:55Z  Pauli
    Corrected return type of command prototype.
    Revision 1.6  2006/08/21 22:02:38Z  Garyp
    Renamed FlashFXDiskInfo to just FlashFXInfo.  Renamed FlashFXReclaim
    to FlashFXCompact.
    Revision 1.5  2006/08/01 16:21:41Z  Pauli
    Added FfxReclaim.
    Revision 1.4  2006/06/23 17:33:07Z  Pauli
    Updated to work with changes to the DCL command shell.
    Revision 1.3  2006/05/10 17:57:23Z  Pauli
    Updated shell function return types to match the dclshell cmd prototype.
    Revision 1.2  2006/05/06 01:21:52Z  Pauli
    Added FlashFXCheck as a shell command.
    Revision 1.1  2006/05/06 01:13:56Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <dlshell.h>


#if DCLCONF_COMMAND_SHELL

#include <flashfx_rtos.h>

#if FFXCONF_SHELL_TOOLS || FFXCONF_SHELL_TESTS


/*--------------------------------------------------------
    Public: ffx_toolfw_exec

    Parameters:

    Return:
--------------------------------------------------------*/
static FFXSTATUS    ffx_toolfw_exec(DCLSHELLHANDLE hShell, int argc, char **argv);


typedef struct _FFXSHELL_CMD
{
    DCLSHELLCOMMAND cmd;
    int             (*function) (const char *);
} FFXSHELL_CMD;


static FFXSHELL_CMD aCmds[] =
{
  #if FFXCONF_SHELL_TOOLS
    {{"FlashFX", "FXCheck",    "Check the spare unit on disk",           ffx_toolfw_exec}, FlashFXCheck},
    {{"FlashFX", "FXCompact",  "Recover discarded disk space",           ffx_toolfw_exec}, FlashFXReclaim},
    {{"FlashFX", "FXDump",     "Dump a FlashFX disk image",              ffx_toolfw_exec}, FlashFXDump},
    {{"FlashFX", "FXImage",    "Read or write a FlashFX disk",           ffx_toolfw_exec}, FlashFXImage},
    {{"FlashFX", "FXInfo",     "Display FlashFX information",            ffx_toolfw_exec}, FlashFXInfo},
    {{"FlashFX", "FXRemount",  "Remount a disk",                         ffx_toolfw_exec}, FlashFXRemount},
  #endif
  #if FFXCONF_SHELL_TESTS
  #if DCL_OSFEATURE_THREADS
    {{"FlashFX", "FXStressMT", "Run the Multi-threaded VBF stress test", ffx_toolfw_exec}, FlashFXStressMT},
  #endif /* DCL_OSFEATURE_THREADS */
    {{"FlashFX", "FXTestFMSL", "Run the FMSL unit test",                 ffx_toolfw_exec}, FlashFXTestFMSL},
    {{"FlashFX", "FXTestVBF",  "Run the VBF unit test",                  ffx_toolfw_exec}, FlashFXTestVBF},
  #endif
};


/*  Add FlashFX commands to the shell.
*/

/*--------------------------------------------------------
    Public: FlashFXShellAddCommands

    Parameters:

    Return:
--------------------------------------------------------*/
FFXSTATUS FlashFXShellAddCommands(
    DCLSHELLHANDLE  hShell)
{

    D_UINT16        uIndex;
    D_UINT16        uEntries = DCLDIMENSIONOF(aCmds);
    DCLSTATUS       dclStat;

    DclAssert(hShell);
    if(!hShell)
    {
        return FFXSTAT_BADPARAMETER;
    }

    /*  Add the commands to the shell.
    */
    for(uIndex = 0; uIndex < uEntries; uIndex++)
    {
        dclStat = DclShellAddCommand(hShell, &aCmds[uIndex].cmd);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            return dclStat;
        }
    }

    return FFXSTAT_SUCCESS;
}


/*--------------------------------------------------------
    FlashFX Commands
--------------------------------------------------------*/


/*--------------------------------------------------------
    Public: ffx_toolfw_exec

    Parameters:

    Return:
--------------------------------------------------------*/
static FFXSTATUS ffx_toolfw_exec(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    D_UINT16        uIndex;
    D_UINT16        uEntries = DCLDIMENSIONOF(aCmds);
    char            szCmdLine[256];

    DclAssert(argv);

    (void)hShell;
    
    /*  Combine the args to a single string.
    */
    szCmdLine[0] = 0;
    for(uIndex = 1; uIndex < argc; uIndex++)
    {
        DclStrCat(szCmdLine, argv[uIndex]);
        DclStrCat(szCmdLine, " ");
    }

    /*  Find the function and call it.
    */
    for(uIndex = 0; uIndex < uEntries; uIndex++)
    {
        if(aCmds[uIndex].function == NULL)
        {
            continue;
        }
        if(DclStrICmp(aCmds[uIndex].cmd.pszName, argv[0]) == 0)
        {
            return (FFXSTATUS)aCmds[uIndex].function(szCmdLine);
        }
    }
    return FFXSTAT_FAILURE;
}

#else

/*  No commands to add to the shell, so just pretend it worked.
*/


/*--------------------------------------------------------
    Public: FlashFXShellAddCommands

    Parameters:

    Return:
--------------------------------------------------------*/
FFXSTATUS FlashFXShellAddCommands(
    DCLSHELLHANDLE  hShell)
{
    (void)hShell;
    return FFXSTAT_SUCCESS;
}

#endif

#endif


