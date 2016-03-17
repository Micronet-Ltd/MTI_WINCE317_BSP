/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    This module defines the public interface to the Datalight Shell.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlshell.h $
    Revision 1.8  2010/11/13 02:10:47Z  garyp
    Minor formatting cleanup -- no functional changes.
    Revision 1.7  2009/06/25 00:43:23Z  garyp
    Prototype update.
    Revision 1.6  2009/02/21 03:04:43Z  brandont
    Added DclShellExecute and supporting prototypes.
    Revision 1.5  2009/02/08 02:34:37Z  garyp
    Updated shell creation parameters.
    Revision 1.4  2008/03/19 20:37:05Z  Garyp
    Fixed so the "echo" parameter is passed to sub-shell invocations.
    Revision 1.3  2007/12/05 02:48:39Z  Garyp
    Added a generalized function for adding an array of commands to the
    shell.  Added the ability to rename a command.  Added the ability to
    specify partial command names, if they are unambiguous.  Modified so
    the shell command structures must be declared const.
    Revision 1.2  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/09/12 18:50:14Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLSHELL_H_INCLUDED
#define DLSHELL_H_INCLUDED


#ifndef DCLCONF_COMMAND_SHELL
#define DCLCONF_COMMAND_SHELL   FALSE
#endif

#if DCLCONF_COMMAND_SHELL

#ifndef DCLCONF_SHELL_TOOLS
#define DCLCONF_SHELL_TOOLS     TRUE
#endif
#ifndef DCLCONF_SHELL_TESTS
#define DCLCONF_SHELL_TESTS     TRUE
#endif

#define         DclShellCreate                  DCLFUNC(DclShellCreate)
#define         DclShellDestroy                 DCLFUNC(DclShellDestroy)
#define         DclShellAddCommand              DCLFUNC(DclShellAddCommand)
#define         DclShellAddMultipleCommands     DCLFUNC(DclShellAddMultipleCommands)
#define         DclShellRenameCommand           DCLFUNC(DclShellRenameCommand)
#define         DclShellRemoveCommand           DCLFUNC(DclShellRemoveCommand)
#define         DclShellRun                     DCLFUNC(DclShellRun)
#define         DclShellAddFileSystemCommands   DCLFUNC(DclShellAddFileSystemCommands)
#define         DclShellAddOsCommands           DCLFUNC(DclShellAddOsCommands)
#define         DclShellParams                  DCLFUNC(DclShellParams)
#define         DclShellExecute                 DCLFUNC(DclShellExecute)

typedef struct
{
    const char *pszClass;
    const char *pszName;
    const char *pszDescription;
    DCLSTATUS   (*function) (DCLSHELLHANDLE hShell, int argc, char **argv);
} DCLSHELLCOMMAND;

typedef struct
{
    DCLINSTANCEHANDLE   hDclInst;
    void               *pUserData;
    DCLENVHANDLE        hEnv;
    unsigned            nWidth;
    unsigned            fEcho : 1;
} DCLSHELLPARAMS;

DCLSHELLHANDLE  DclShellCreate(DCLINSTANCEHANDLE hDclInst, const char *pszTitle, const char *pszScript, DCLENVHANDLE hEnviron, unsigned fQuiet, unsigned fEcho, void *pUserData);
void            DclShellDestroy(              DCLSHELLHANDLE hShell);
DCLSTATUS       DclShellAddCommand(           DCLSHELLHANDLE hShell, const DCLSHELLCOMMAND *pCommand);
DCLSTATUS       DclShellAddMultipleCommands(  DCLSHELLHANDLE hShell, const DCLSHELLCOMMAND *paCommands, unsigned nCommandCount, const char *pszClassName, const char *pszPrefix);
DCLSTATUS       DclShellRenameCommand(        DCLSHELLHANDLE hShell, const char *pszName, const char *pszNewName);
DCLSTATUS       DclShellRemoveCommand(        DCLSHELLHANDLE hShell, const char *pszName);
DCLSTATUS       DclShellRun(                  DCLSHELLHANDLE hShell);
DCLSTATUS       DclShellAddFileSystemCommands(DCLSHELLHANDLE hShell);
DCLSTATUS       DclShellAddOsCommands(        DCLSHELLHANDLE hShell, const char *pszClassName, const char *pszPrefix);
void            DclShellParams(               DCLSHELLHANDLE hShell, DCLSHELLPARAMS *pParams);

/*  ToDo: These need to be reconsidered...
*/
DCLSTATUS       DclShellExecute(const char *pszTitle, const char *pszScript, DCLENVHANDLE hEnv, unsigned fQuiet, unsigned fEcho, void *pUserData);
DCLSTATUS       FfxShellAddCommands(          DCLSHELLHANDLE hShell, const char *pszClassName, const char *pszPrefix);
DCLSTATUS       RelianceShellAddCommands(     DCLSHELLHANDLE hShell, const char *pszClassName, const char *pszPrefix);
DCLSTATUS       DL4GRShellAddCommands(        DCLSHELLHANDLE hShell, const char *pszClassName, const char *pszPrefix);


#endif /* #if DCLCONF_COMMAND_SHELL */

#endif  /* DLSHELL_H_INCLUDED */

