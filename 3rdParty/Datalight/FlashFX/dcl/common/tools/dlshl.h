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

    This module defines the internal shared interface between various shell
    components.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlshl.h $
    Revision 1.9  2010/11/09 01:55:18Z  garyp
    Updated to include dlshell.h.
    Revision 1.8  2010/04/18 21:01:25Z  garyp
    Moved some tool commands to the module dlshelltools.c.  Updated the
    ProfSummary command to take the /NOOA parameter (no overhead adjustment).
    Revision 1.7  2009/02/21 00:48:21Z  brandont
    Added DCLFUNC for DclShellConsoleInput.
    Revision 1.6  2009/02/08 00:52:01Z  garyp
    Structure names updated.
    Revision 1.5  2008/03/17 17:05:46Z  Garyp
    Fixed so the "echo" parameter is passed to sub-shell invocations.
    Revision 1.4  2008/01/06 17:49:30Z  garyp
    Updated to release resources in reverse order of allocation.
    Revision 1.3  2007/11/29 21:29:06Z  Garyp
    Added a generalized function for adding an array of commands to the
    shell.  Added the ability to rename a command.  Added the ability to
    specify partial command names, if they are unambiguous.  Modified so
    the shell command structures must be declared const.
    Revision 1.2  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/09/12 18:50:14Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLSHL_H_INCLUDED
#define DLSHL_H_INCLUDED

#include <dlshell.h>

#define DLS_PATHLEN      (256)
#define DLS_MAX_ARGS      (30)
#define CMDQUEUESIZE     (128)
#define WORKBUFFSIZE    (1024)
#define MAX_COMMAND_LEN   (32)
#define DEFAULT_CONSOLE_WIDTH   (80)

#define GENERALCLASS        "General"
#define SCRIPTINGCLASS      "Scripting"
#define DEBUGGINGCLASS      "Debugging"
#define PROFILERCLASS       "Profiler"
#define REQUESTORCLASS      "Requestor"
#define HELPCMD             "?"
#define SHELLHELPCMD        "??"
#define DEBUGGINGHELPCMD    "?D"
#define EXITCMD             "Exit"

typedef struct sCOMMANDINFO
{
    DCLSHELLCOMMAND         sCommand;
    char                    szCmdName[MAX_COMMAND_LEN];
    struct sCOMMANDINFO    *pNext;
    unsigned                nSeqNum;
} COMMANDINFO;


typedef struct sDCLSHELL
{
    COMMANDINFO    *pCommandList;
    D_BUFFER       *pWorkBuff;
    const char     *pszTitle;           /* Shell instance title, and prompt */
    const char     *pszScript;
    const char     *pszExitCmd;
    unsigned        nScriptIndex;
    unsigned        nSequenceNum;
    unsigned        fQuiet         : 1;
    unsigned        fExit          : 1;
    unsigned        fTimeStamp     : 1;
    unsigned        fCommandQueued : 1;
    unsigned        fBreakOn       : 1; /* Break before each command...     */
    unsigned        fBreakAsk      : 1; /* ...but ask first                 */
    unsigned        fCurDirSupport : 1; /* Get/Set directory works          */
    unsigned        fWeAllocedScript:1; /* Script memory was allocated internally */
    unsigned        fWeCreatedEnv  : 1; /* Environment was created internally */
    DCLSTATUS       ulStatus;           /* DCLSTATUS for the last command   */
    char            szCmdQueue[CMDQUEUESIZE];
    char            szArgBuffer[1024];
    DCLSHELLPARAMS  params;
} DCLSHELL;


/*  Helper macros and functions
*/
#define ISHELPREQUEST() (argc > 1 && (!DclStrCmp(argv[1], "?") || !DclStrCmp(argv[1], "/?")))

#define DclShellConsoleInput            DCLFUNC(DclShellConsoleInput)

unsigned    DclShellConsoleInput(DCLSHELLHANDLE hShell, char *pszBuffer, unsigned nMaxLen);

#if DCLCONF_SHELL_TOOLS
#define     DclShellAddBlockDevCommands DCLFUNC(DclShellAddBlockDevCommands)
#define     DclShellAddToolCommands     DCLFUNC(DclShellAddToolCommands)

DCLSTATUS   DclShellAddBlockDevCommands(DCLSHELLHANDLE hShell);
DCLSTATUS   DclShellAddToolCommands(DCLSHELLHANDLE hShell);
#endif

#endif  /* DLSHL_H_INCLUDED */

