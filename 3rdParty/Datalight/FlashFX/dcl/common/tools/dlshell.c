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

    This module implements the Datalight command shell and adds the basic,
    built in commands.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlshell.c $
    Revision 1.73  2010/12/13 18:22:55Z  garyp
    Initialize a local to placate a picky compiler.
    Revision 1.72  2010/12/02 13:17:04Z  garyp
    Removed some unnecessary braces which some compilers don't like.
    Revision 1.71  2010/11/23 18:26:17Z  garyp
    Added the AssertMode command.
    Revision 1.70  2010/07/15 01:04:33Z  garyp
    Added functionality to reset the profiler data.
    Revision 1.69  2010/04/28 23:27:44Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.68  2010/04/18 21:01:25Z  garyp
    Moved some tool commands to the module dlshelltools.c.  Updated
    the ProfSummary command to take the /NOOA parameter (no overhead
    adjustment).
    Revision 1.67  2010/01/04 22:47:25Z  jimmb
    Needed to cast the variable chr because some compilers consider 
    char to be unsigned and others signed.
    Revision 1.66  2009/11/20 19:03:11Z  garyp
    Conditioned various commands on DCLCONF_SHELL_TOOLS/TESTS.
    Revision 1.65  2009/11/11 17:53:42Z  garyp
    Updated the WAIT command to use a sleep() in a loop rather than
    a sleep() by itself.  Updated the TIMESTAMP feature to work even
    if the DclOsGetDateTime() API is not implemented.  Better handle
    invalid characters in the input function.
    Revision 1.64  2009/11/08 16:21:06Z  garyp
    Updated to use some renamed symbols -- no functional changes.
    Revision 1.63  2009/11/03 13:21:04Z  garyp
    Debug code and minor datatype updates -- no functional changes.
    Revision 1.62  2009/10/18 01:09:38Z  garyp
    Renamed TraceOn to TraceMax, TraceOff to TraceNormal, and added the 
    TraceToggle command.
    Revision 1.61  2009/09/09 16:49:40Z  garyp
    Added support for DEVIOTEST.
    Revision 1.60  2009/06/24 23:19:16Z  garyp
    Added the "memval" command.  Updated to accommodate functions which take
    a DCL Instance parameter.
    Revision 1.59  2009/03/03 02:56:50Z  brandont
    Moved DclShellExecute to dlshellex.c.
    Revision 1.58  2009/02/21 04:18:52Z  brandont
    Added DclShellExecute.
    Revision 1.57  2009/02/18 07:47:15Z  garyp
    Voided some unused parameters.
    Revision 1.56  2009/02/17 06:06:23Z  keithg
    Added explicit void of unused formal parameters.
    Revision 1.55  2009/02/09 22:37:41Z  billr
    Fix compiler warning: pointer targets in passing argument 2 of
    'FfxDclInputChar' differ in signedness
    Revision 1.54  2009/02/08 03:27:39Z  garyp
    Fixed a broken status code.
    Revision 1.53  2009/02/08 00:33:23Z  garyp
    Merged from the v4.0 branch.  Added the ReqOpen and ReqClose commands.
    Updated the MemChain command to allow the command to be passed to a device
    driver via the requestor interface. Split debugging, requestor, and profiler
    commands out to the own separate help screen.  Updated the TraceOn/Off
    commands to take a requestor handle.  Updated the profiler commands so they
    are always included, and so they can direct the operation to the requestor
    interface.  Updated to use the new DclInputChar() function.  Clarified some
    help text.
    Revision 1.52  2009/01/26 23:18:53Z  keithg
    Added explicit void of unused formal parameter(s).
    Revision 1.51  2008/04/07 22:03:39Z  brandont
    Updated to use the new DCL file system services.
    Revision 1.50  2008/04/03 23:31:31Z  brandont
    Updated all defines and structures used by the DCL file system services to
    use the DCLFS prefix.
    Revision 1.49  2008/03/19 18:46:54Z  Garyp
    Fixed so the "echo" parameter is passed to sub-shell invocations.
    Revision 1.48  2008/03/12 00:41:22Z  brandont
    Corrected uninitialized variable warnings.
    Revision 1.47  2008/01/06 17:48:21Z  garyp
    Updated to release resources in reverse order of allocation.
    Revision 1.46  2007/12/18 20:31:14Z  brandont
    Updated function headers.
    Revision 1.45  2007/12/17 02:36:54Z  Garyp
    Updated so console-input support can be disabled at compile time, but
    still include the shell functionality.
    Revision 1.44  2007/12/05 02:48:44Z  Garyp
    Added a generalized function for adding an array of commands to the
    shell.  Added the ability to rename a command.  Added the ability to
    specify partial command names, if they are unambiguous.  Modified so
    the shell command structures must be declared const.
    Revision 1.43  2007/11/03 23:31:12Z  Garyp
    Added the standard module header.
    Revision 1.42  2007/10/04 22:46:41Z  pauli
    Updated how input is handled to consistently skip CR characters.
    Revision 1.41  2007/09/13 16:57:50Z  pauli
    Moved the file system related commands to dlshellfs.c, with their own
    initialization function.  Broke the help into two parts.  Added a
    "pUserData" parameter to the shell create function to allow private data
    to be accessed by custom shell commands in a thread-safe fashion.  Added
    an interface for querying certain shell configuration parameters.  Added
    a "Width" command.  Removed the profiler create/destroy commands from the
    shell.  It must be separately initialized if it is to be used in the shell.
    Modified so the PROMPT environment variable can be used to specify a custom
    console prompt.  Removed the copy_dir command.
    Revision 1.40  2007/09/06 17:40:04Z  johnb
    Added new command CopyDir to copy the contents of a directory and optionally
    subdirectories.
    Revision 1.39  2007/08/03 01:42:02Z  garyp
    Added an ASSERT option (undocumented) to the DebugBreak command.
    Revision 1.38  2007/06/24 02:23:22Z  Garyp
    Updated so an environment variable value may be enclosed in single
    or double ticks, to allow values with spaces to be specified.
    Revision 1.37  2007/05/17 18:33:15Z  garyp
    Minor syntax and error handling fixes.
    Revision 1.36  2007/05/15 01:30:09Z  garyp
    Updated to receive an environment handle on entry, or to create one on
    the fly otherwise.  Added the 'echo' and 'set' commands.  Updated the 'if'
    command to handle environment variable comparisons.
    Revision 1.35  2007/05/11 02:52:49Z  Garyp
    Modified to use a macro renamed for clarity.
    Revision 1.34  2007/04/16 18:21:31Z  Garyp
    Added the "Run" and "Goto" commands.  Modified the "Append" and "Write"
    commands so that the count value is optional.  If it not specified, exactly
    one line of console input will be appended or written.
    Revision 1.33  2007/04/08 02:01:30Z  Garyp
    Updated to work cleanly on systems which do not support a CWD.
    Revision 1.32  2007/04/06 19:01:04Z  Garyp
    Updated to handle quoted arguments.  Added error handling for Create/
    Remove/Rename/Change Dir.
    Revision 1.31  2007/04/05 20:07:49Z  Garyp
    Updated to build cleanly with the RVDS tools.
    Revision 1.30  2007/04/04 21:56:00Z  Garyp
    Fixed the MemChain command initialization.
    Revision 1.29  2007/04/02 20:55:09Z  Garyp
    Added the DebugBreak command.  Fixed the Compare command to properly handle
    files of different sizes.  Eliminated the batch file style "not" option in
    the If command, and changed it to use "!=".  Cleaned up all the syntax
    messages to use similar formatting, and use the abstracted rather than
    hard-coded command names.
    Revision 1.28  2007/03/19 18:24:51Z  Garyp
    Added shell commands to get/set the DCL trace mask, as well as commands
    to enable and disable global tracing.  Fixed so that the case is preserved
    when passing the command name to the various "exec" functions.
    Revision 1.27  2007/03/16 02:53:44Z  Garyp
    Added a separate "RenDir" command for renaming directories.  Enhanced
    the output from the "Dir:" command to include the file attributes and the
    millisecond portion of the timestamp.
    Revision 1.26  2007/03/06 03:46:51Z  Garyp
    Documentation update.
    Revision 1.25  2007/02/08 18:51:29Z  Garyp
    Updated so a local copy of the comand name is kept with the command
    information.
    Revision 1.24  2007/01/17 01:49:16Z  Garyp
    Added the MemChain command.
    Revision 1.23  2007/01/16 01:29:12Z  Garyp
    Added the "Rem" command.
    Revision 1.22  2007/01/12 22:54:47Z  Garyp
    Fixed a bug in the IF command and updated so that a leading colon causes
    the remainder of the line to be ignored.
    Revision 1.21  2007/01/12 04:31:46Z  Garyp
    Removed a problematic const qualifier.
    Revision 1.20  2007/01/12 03:55:29Z  Garyp
    Added the "if", "pause", "wait", and "status" commands.  Updated so that it
    is the resposibility of each command to display its success/failure results.
    Modified so that the status code from the final command is returned when the
    shell exits.  Modified the "exit" command so you can exit with a specific
    result code, if desired, rather than that of the last command.  Updated
    so that one command can queue up another one, which will be executed
    immediately after the first one finishes (so that "if" can do something
    useful).
    Revision 1.19  2007/01/10 01:01:13Z  Garyp
    Updated to be able to run automated commands from a script.
    Revision 1.18  2007/01/02 22:16:10Z  Garyp
    Fixed the "type" command to not be subject to the maximum string length
    limit imposed by DclPrintf().
    Revision 1.17  2006/12/11 22:04:18Z  Garyp
    Updated so compilation is conditional on whether the symbol
    DCL_OSFEATURE_CONSOLEINPUT is set.
    Revision 1.16  2006/12/08 04:16:52Z  Garyp
    Removed a number of static local buffers.
    Revision 1.15  2006/12/08 00:05:19Z  Garyp
    Updated to build cleanly when the "tools" are turned off.
    Revision 1.14  2006/11/30 02:01:02Z  Garyp
    Added the memdump and timestamp commands.  Updated so that the commands are
    subdivided into classes.
    Revision 1.13  2006/10/07 16:23:52Z  Garyp
    General cleanup of the read/write logic to treat size_t types properly as
    unsigned, and to fix warnings generated by the RealView tools.
    Revision 1.12  2006/10/07 01:56:42Z  Garyp
    Fixed various errors and warnings noted by the RealView tools.
    Revision 1.11  2006/09/28 01:36:11Z  garyp
    Updated to allow the shell tests or tools to be compiled out.
    Revision 1.10  2006/08/22 17:45:33Z  Pauli
    Added conditions to all profiler commands.
    Revision 1.9  2006/08/21 00:41:29Z  Garyp
    Added support for automatically creating a profiler instance.
    Revision 1.8  2006/08/18 19:36:01Z  Garyp
    Added profiler commands.  Cleaned up some declarations to build cleanly
    with the DIAB tools.
    Revision 1.7  2006/07/05 23:01:40Z  Pauli
    Removed an unused variable.
    Revision 1.6  2006/07/03 20:04:45Z  Pauli
    Added usage information to all commands with parameters.  Corrected a
    possible overflow condidtion in append and write.  Added a command to
    display DCL statistics.
    Revision 1.5  2006/06/15 23:33:45Z  Pauli
    Added function to allow removing commands.  Added option to echo input
    characters.  Added usage information to commands.
    Revision 1.4  2006/05/31 20:07:43Z  Garyp
    Eliminated the use of globals by using a shell handle to track context
    information.  Automatically alphabetize the command list, as well as made
    it display all the commands, not just the internal ones.  Added a command
    description.
    Revision 1.3  2006/05/13 00:56:07Z  Pauli
    Corrected how errors are displayed... again.
    Revision 1.2  2006/05/08 19:32:56Z  Pauli
    Updated how errors are displayed.
    Revision 1.1  2006/05/04 23:31:50Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlshell.h>

#if DCLCONF_COMMAND_SHELL

#include <dltools.h>
#include <dlapireq.h>
#include <dlservice.h>
#include "dlshl.h"

#define DEBUG_CHARS         FALSE      /* Must be FALSE for checkin */

/*  Command execution prototypes
*/
static DCLSTATUS exec_help(          DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_ShellHelp(     DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_DebuggingHelp( DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_exit(          DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_status(        DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_time(          DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_TimeStamp(     DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_Width(         DCLSHELLHANDLE hShell, int argc, char **argv);
#if DCLCONF_SHELL_TOOLS
static DCLSTATUS exec_echo(          DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_goto(          DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_if(            DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_pause(         DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_run(           DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_set(           DCLSHELLHANDLE hShell, int argc, char **argv);
#endif
static DCLSTATUS exec_wait(          DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_stats(         DCLSHELLHANDLE hShell, int argc, char **argv);
#if DCLCONF_SHELL_TESTS
static DCLSTATUS exec_dcltest(       DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_deviotest(     DCLSHELLHANDLE hShell, int argc, char **argv);
#endif
static DCLSTATUS exec_AssertMode(    DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_DebugBreak(    DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_TraceMax(      DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_TraceNormal(   DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_TraceToggle(   DCLSHELLHANDLE hShell, int argc, char **argv);
#if D_DEBUG
static DCLSTATUS exec_TraceMaskGet(  DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_TraceMaskSet(  DCLSHELLHANDLE hShell, int argc, char **argv);
#endif
static DCLSTATUS exec_ProfReset(     DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_ProfStart(     DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_ProfStop(      DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_ProfSummary(   DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_ReqOpen(       DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS exec_ReqClose(      DCLSHELLHANDLE hShell, int argc, char **argv);

/*  Helper functions
*/
static void      GetCommand(DCLSHELLHANDLE hShell, char *pszCmdBuff, unsigned nMaxLen, int * argc, char **argv);
static void      DisplayPrompt(DCLSHELLHANDLE hShell);
static D_BOOL    ParseNumericValue(const char *pszNum, D_UINT32 *pulNum);

static const DCLSHELLCOMMAND aBasicCmds[] =
{
    {GENERALCLASS,   HELPCMD,          "Display help",                                      exec_help},
    {GENERALCLASS,   SHELLHELPCMD,     "Display help for user interface and scripting",     exec_ShellHelp},
    {GENERALCLASS,   DEBUGGINGHELPCMD, "Display help for debugging and advanced commands",  exec_DebuggingHelp},
    {GENERALCLASS,   EXITCMD,          "Exit the shell",                                    exec_exit},
    {GENERALCLASS,   "Status",         "Display the status result of the last command",     exec_status},
    {GENERALCLASS,   "Time",           "Display the current date and time",                 exec_time},
    {GENERALCLASS,   "TimeStamp",      "Enable or disable the use of timestamps",           exec_TimeStamp},
    {GENERALCLASS,   "Width",          "Specify the console width",                         exec_Width},
  #if DCLCONF_SHELL_TOOLS
    {SCRIPTINGCLASS, "Echo",           "Display text on the console",                       exec_echo},
    {SCRIPTINGCLASS, "Goto",           "Go to a label in a script",                         exec_goto},
    {SCRIPTINGCLASS, "If",             "Conditionally perform a command",                   exec_if},
    {SCRIPTINGCLASS, "Pause",          "Pause until [Enter] is pressed",                    exec_pause},
    {SCRIPTINGCLASS, "Rem",            "Place a remark in a script",                        NULL},
    {SCRIPTINGCLASS, "Run",            "Run a shell command script",                        exec_run},
    {SCRIPTINGCLASS, "Set",            "Add, remove, or display environment variables",     exec_set},
  #endif        
    {SCRIPTINGCLASS, "Wait",           "Wait for the specified number of seconds",          exec_wait},
    {"DCL",          "Stats",          "Display system statistics",                         exec_stats},
  #if DCLCONF_SHELL_TESTS
    {"DCL",          "BDevTest",       "Run the Block Device tests",                        exec_deviotest},
    {"DCL",          "DclTest",        "Run the DCL unit tests",                            exec_dcltest},
  #endif
    {DEBUGGINGCLASS, "AssertMode",     "Manipulate the assert mode",                        exec_AssertMode},
    {DEBUGGINGCLASS, "DebugBreak",     "Manipulate the debug break state",                  exec_DebugBreak},
    {DEBUGGINGCLASS, "TraceNormal",    "Restores tracing to its default settings",          exec_TraceNormal},
    {DEBUGGINGCLASS, "TraceMax",       "Turn on all debug trace classes",                   exec_TraceMax},
    {DEBUGGINGCLASS, "TraceToggle",    "Toggle the trace enabled setting",                  exec_TraceToggle},
  #if  D_DEBUG
    {DEBUGGINGCLASS, "TraceDCLGet",    "Get the DCL trace mask",                            exec_TraceMaskGet},
    {DEBUGGINGCLASS, "TraceDCLSet",    "Set the DCL trace mask",                            exec_TraceMaskSet},
  #endif
};

static const DCLSHELLCOMMAND aRequestorCmds[] =
{
    {REQUESTORCLASS, "ReqOpen",        "Open a requestor interface handle",                 exec_ReqOpen},
    {REQUESTORCLASS, "ReqClose",       "Close an open requestor handle",                    exec_ReqClose}
};

static const DCLSHELLCOMMAND aProfilerCmds[] =
{
    {PROFILERCLASS,  "ProfReset",      "Reset the profiler counters",                       exec_ProfReset},
    {PROFILERCLASS,  "ProfStart",      "Start the profiler",                                exec_ProfStart},
    {PROFILERCLASS,  "ProfStop",       "Stop the profiler",                                 exec_ProfStop},
    {PROFILERCLASS,  "ProfSummary",    "Display profiler results",                          exec_ProfSummary}
};



                    /*------------------------------*\
                     *                              *
                     *    Public Shell Interface    *
                     *                              *
                    \*------------------------------*/


/*-------------------------------------------------------------------
    Public: DclShellCreate()

    Creates an instance of the DCL Command Shell.

    NOTE:  If an environment is supplied to this function,
           on exit it will contain any final information
           which was in use when the shell was exited.
           This is a useful way to return information out
           of a shell instantiation.

    Parameters:
        hDclInst    - The DCL instance handle.
        pszTitle    - The shell instance title, or NULL if none.
                      This value will be used as the shell prompt.
        pszScript   - A pointer to a script to play or NULL if none.
        hEnviron    - The environment handle to use, or NULL to use
                      a default internal environment.  An environment
                      can be created with DclEnvCreate().
        fQuiet      - Initialize without signing on.
        fEcho       - Indicates whether input should be echoed.
        pUserData   - An optional pointer to any arbitrary data
                      which the caller needs to have available to
                      the shell commands.

    Return Value:
        Returns a pointer to the shell if successful, or NULL
        otherwise.
-------------------------------------------------------------------*/
DCLSHELLHANDLE DclShellCreate(
    DCLINSTANCEHANDLE   hDclInst,
    const char         *pszTitle,
    const char         *pszScript,
    DCLENVHANDLE        hEnviron,
    unsigned            fQuiet,
    unsigned            fEcho,
    void               *pUserData)
{
    DCLSHELL           *pShell = NULL;
    D_UINT16            uIndex;
    DCLSTATUS           dclStat;
    D_BOOL              fRequestorEnabled = FALSE;
    D_BOOL              fProfilerIncluded = DCLCONF_PROFILERENABLED;

    /*  Allocate memory to manage the shell.
    */
    pShell = DclMemAllocZero(sizeof(*pShell));
    if(!pShell)
        return NULL;

    pShell->pWorkBuff = DclMemAlloc(WORKBUFFSIZE);
    if(!pShell->pWorkBuff)
    {
        DclMemFree(pShell);
        return NULL;
    }

    /*  If an environment was not supplied to us, create one which we
        will use internally.
    */
    if(!hEnviron)
    {
        hEnviron = DclEnvCreate(0, NULL);
        if(!hEnviron)
        {
            DclMemFree(pShell->pWorkBuff);
            DclMemFree(pShell);
            return NULL;
        }

        pShell->fWeCreatedEnv = TRUE;
    }

    pShell->pszTitle            = pszTitle;
    pShell->pszScript           = pszScript;
    pShell->fQuiet              = fQuiet;
    pShell->params.hDclInst     = hDclInst;
    pShell->params.fEcho        = fEcho;
    pShell->params.hEnv         = hEnviron;
    pShell->params.pUserData    = pUserData;
    pShell->params.nWidth       = DEFAULT_CONSOLE_WIDTH;

    /*  Add the basic commands to the shell.
    */
    for(uIndex = 0; uIndex < DCLDIMENSIONOF(aBasicCmds); uIndex++)
    {
        dclStat = DclShellAddCommand(pShell, &aBasicCmds[uIndex]);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclShellDestroy(pShell);
            return NULL;
        }
    }

  #if DCLCONF_SHELL_TOOLS
    dclStat = DclShellAddToolCommands(pShell);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclShellDestroy(pShell);
        return NULL;
    }
  #endif

    /*  Add requestor commands to the shell only if the requestor
        interface is implemented.
    */
    if(DclOsRequestorOpen(NULL, 0, NULL) == DCLSTAT_REQUESTOR_BADDEVICE)
    {
        fRequestorEnabled = TRUE;

        for(uIndex = 0; uIndex < DCLDIMENSIONOF(aRequestorCmds); uIndex++)
        {
            dclStat = DclShellAddCommand(pShell, &aRequestorCmds[uIndex]);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclShellDestroy(pShell);
                return NULL;
            }
        }
    }

    /*  Add Profiler commands to the shell only if the profiler is
        locally enabled, OR if the requestor interface is enabled.
        (Even if the profiler is not locally included in the build,
        we still retain the ability to send profiler commands via
        the requestor interface.)
    */
    if(fProfilerIncluded || fRequestorEnabled)
    {
        for(uIndex = 0; uIndex < DCLDIMENSIONOF(aProfilerCmds); uIndex++)
        {
            dclStat = DclShellAddCommand(pShell, &aProfilerCmds[uIndex]);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclShellDestroy(pShell);
                return NULL;
            }
        }
    }

    return pShell;
}


/*-------------------------------------------------------------------
    Public: DclShellDestroy()

    Destroys an instance of the DCL Command Shell.

    Parameters:
        hShell      - The handle of the shell instance to destroy.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclShellDestroy(
    DCLSHELLHANDLE  hShell)
{
    DclAssertReadPtr(hShell, sizeof(*hShell));

    /*  If we internally allocated the memory for any previous script, we
        are responsible for internally releasing it as well.  If it was
        handed to us on entry, don't release it as we have no idea where
        it came from.
    */
    if(hShell->fWeAllocedScript)
    {
        DclAssert(hShell->pszScript);
        DclMemFree((void*)hShell->pszScript);
    }

    /*  Free all the commands added to the shell.  Always free the
        command with the highest sequence number first in an attempt
        to release resources in reverse order of allocation.
    */
    while(hShell->pCommandList)
    {
        COMMANDINFO    *pCurr;
        COMMANDINFO    *pNext;
        COMMANDINFO    *pPrev = NULL;
        COMMANDINFO    *pMax = NULL;
        COMMANDINFO    *pMaxPrev = NULL;

        pCurr = hShell->pCommandList;
        while(pCurr)
        {
            if(!pMax || pCurr->nSeqNum > pMax->nSeqNum)
            {
                pMaxPrev = pPrev;
                pMax = pCurr;
            }

            pPrev = pCurr;
            pCurr = pCurr->pNext;
        }

        pNext = pMax->pNext;

        DclMemFree(pMax);

        if(pMaxPrev)
        {
            /*  If there was a previous entry, link the chain back together
            */
            pMaxPrev->pNext = pNext;
        }
        else
        {
            /*  There was no previous entry, so the chain head must be adjusted
            */
            DclAssert(pMax == hShell->pCommandList);
            hShell->pCommandList = pNext;
        }
    }

    /*  If we internally created the environment, we are responsible for
        internally releasing it as well.  If it was handed to us on entry,
        don't release it as we have no idea where it came from.
    */
    if(hShell->fWeCreatedEnv)
    {
        DclAssert(hShell->params.hEnv);
        DclEnvDestroy(hShell->params.hEnv);
    }

    DclMemFree(hShell->pWorkBuff);

    /*  Free the base memory used to manage the shell.
    */
    DclMemFree(hShell);

    return;
}


/*-------------------------------------------------------------------
    Public: DclShellRun()

    Run the DCL Command Shell.

    Parameters:
        hShell      - The handle of the shell instance to run.

    Return Value:
        Returns the DCLSTATUS value from the last command run.
 -------------------------------------------------------------------*/
DCLSTATUS DclShellRun(
    DCLSHELLHANDLE  hShell)
{
    DclAssertReadPtr(hShell, sizeof(*hShell));

    if(!hShell->fQuiet)
    {
        DclPrintf("\nDatalight Command Shell\n");
        DclSignOn(FALSE);
        DclPrintf("\n");
    }

    /*  Detect in advance whether Get/SetCurrentDirectory works
    */
    if(DclFsDirGetWorking(hShell->szArgBuffer, DCLDIMENSIONOF(hShell->szArgBuffer)) == DCLSTAT_SUCCESS)
        hShell->fCurDirSupport = TRUE;

    /*  Process commands (forever).
    */
    while(TRUE)
    {
        int     argc;
        char   *argv[DLS_MAX_ARGS];

        /*  Show the prompt
        */
        DisplayPrompt(hShell);

        /*  Get a command and process it if we can.
        */
        GetCommand(hShell, hShell->szArgBuffer, sizeof(hShell->szArgBuffer), &argc, &argv[0]);
        if(argc > 0)
        {
            unsigned        fFound = FALSE;
            unsigned        fAmbiguous = FALSE;
            COMMANDINFO    *pEntry = hShell->pCommandList;

            /*  Any command starting with a colon is considered a
                comment/label, so ignore it and continue on.
            */
            if(*argv[0] == ':')
            {
                DclPrintf("\n");
                continue;
            }

            /*  REM could be treated like a regular command and simply do
                nothing, but we just short-circuit it here and do nothing...
            */
            if(DclStrICmp(argv[0], "REM") == 0)
            {
                DclPrintf("\n");
                continue;
            }

            /*  First search the whole list for an exact match.  If one is
                found, we don't care that it might partially match some
                other command.
            */
            while(pEntry)
            {
                if(DclStrICmp(argv[0], pEntry->sCommand.pszName) == 0)
                    break;

                /*  On to the next command.
                */
                pEntry = pEntry->pNext;
            }

            /*  If no exact match was found, THEN we search for a partial
                match, if any
            */
            if(!pEntry)
            {
                COMMANDINFO    *pFirstMatch = NULL;
                size_t          nLen = DclStrLen(argv[0]);

                pEntry = hShell->pCommandList;
                while(pEntry)
                {
                    if(DclStrNICmp(argv[0], pEntry->sCommand.pszName, nLen) == 0)
                    {
                        if(pFirstMatch == NULL)
                        {
                            pFirstMatch = pEntry;
                            pEntry = pEntry->pNext;
                            continue;
                        }
                        else
                        {
                            DclPrintf("Ambiguous command entry:  \"%s\" matches:\n", argv[0]);
                            DclPrintf("  %-20s - %s\n", pFirstMatch->sCommand.pszName, pFirstMatch->sCommand.pszDescription);
                            DclPrintf("  %-20s - %s\n", pEntry->sCommand.pszName, pEntry->sCommand.pszDescription);
                            pEntry = pEntry->pNext;
                            while(pEntry)
                            {
                                /*  Display all the commands that partially
                                    match.
                                */
                                if(DclStrNICmp(argv[0], pEntry->sCommand.pszName, nLen) == 0)
                                    DclPrintf("  %-20s - %s\n", pEntry->sCommand.pszName, pEntry->sCommand.pszDescription);

                                pEntry = pEntry->pNext;
                            }

                            pFirstMatch = NULL;
                            fAmbiguous = TRUE;
                            break;
                        }
                    }

                    pEntry = pEntry->pNext;
                }

                pEntry = pFirstMatch;
            }

            if(pEntry)
            {
                /*  Use the case-correct command name rather than the
                    command which the user typed in.  This ensures that
                    when the command syntax is displayed, that the case
                    is in the correct formal form.
                */
                argv[0] = (char*)pEntry->sCommand.pszName;

                /*  Yes, command was found, execute the routine.
                */
                fFound = TRUE;
                DclPrintf("\n");

                if(hShell->fBreakAsk)
                    DclOsDebugBreak(DCLDBGCMD_QUERY);
                else if(hShell->fBreakOn)
                    DclOsDebugBreak(DCLDBGCMD_BREAK);

                /*******************************************************\
                 *                                                     *
                 *  If you are in a debugger and you get here due to   *
                 *  a "DebugBreak" command, step into the following    *
                 *  statement to debug the next command.               *
                 *                                                     *
                \*******************************************************/

                hShell->ulStatus = pEntry->sCommand.function(hShell, argc, &argv[0]);
            }

            if(hShell->fExit)
            {
                if(hShell->pszTitle)
                    DclPrintf("\n%s Shell exiting with status: %lX\n", hShell->pszTitle, hShell->ulStatus);
                else
                    DclPrintf("\nShell exiting with status: %lX\n", hShell->ulStatus);

                return hShell->ulStatus;
            }

            /*  Check if Command not found.
            */
            if(!fFound)
            {
                if(!fAmbiguous)
                    DclPrintf("\nCommand \"%s\" not found!\n", argv[0]);

                hShell->ulStatus = DCLSTAT_SHELLCOMMANDNOTFOUND;
            }
        }
        else if(argc == -1)
        {
            DclPrintf("Console input not supported!\n");
            return DCLSTAT_SHELLINPUTFAILURE;
        }
        else if(argc == -2)
        {
            DclPrintf("Terminating...\n");
            return DCLSTAT_INPUT_TERMINATE;
        }
        else
        {
            DclPrintf("\n");
        }
    }
}


/*-------------------------------------------------------------------
    Public: DclShellAddCommand()

    Add a command to an instance of the DCL Command Shell.

    NOTE:  The pszClass and pszDescription values must point to
           static data that persists throughout the life of this
           particular shell command.  A private copy of the
           pszName value is stored, so that does not need to be
           static.

    Parameters:
        hShell      - The handle of the shell instance to use.
        pCommand    - The command to add.

    Return Value:
        Status indicating if the operation was successful.
-------------------------------------------------------------------*/
DCLSTATUS DclShellAddCommand(
    DCLSHELLHANDLE          hShell,
    const DCLSHELLCOMMAND  *pCommand)
{
    COMMANDINFO            *pEntry;
    COMMANDINFO            *pThis;
    COMMANDINFO            *pPrev;

    DclAssertReadPtr(hShell, sizeof(*hShell));
    DclAssert(pCommand);
    DclAssert(pCommand->pszClass);
    DclAssert(pCommand->pszName);
    DclAssert(pCommand->pszDescription);

    /*  Allocate a new command entry.
    */
    pEntry = DclMemAlloc(sizeof(*pEntry));
    if(!pEntry)
        return DCLSTAT_MEMALLOCFAILED;

    pEntry->sCommand = *pCommand;
    pEntry->nSeqNum = hShell->nSequenceNum;
    hShell->nSequenceNum++;

    /*  Make a private copy of the actual command name
    */
    DclAssert(DclStrLen(pEntry->sCommand.pszName) < sizeof(pEntry->szCmdName));
    DclStrNCpy(pEntry->szCmdName, pEntry->sCommand.pszName, sizeof(pEntry->szCmdName));

    pEntry->sCommand.pszName = pEntry->szCmdName;

    /*  Store a special pointer to the 'exit' command because this command
        is subject to being renamed, and we want the help display to work
        properly.
    */
    if(DclStrICmp(pEntry->sCommand.pszName, EXITCMD) == 0)
        hShell->pszExitCmd = pEntry->sCommand.pszName;

    DCLPRINTF(2, ("Adding shell command: \"%s - %s - %s\"\n",
        pEntry->sCommand.pszClass, pEntry->sCommand.pszName, pEntry->sCommand.pszDescription));

    /*  Add the new command to the list in alphabetical order within the
        matching class.  If no matching class is found, it will be added
        to the end as the first of a new class.
    */
    pPrev = NULL;
    pThis = hShell->pCommandList;

    while(pThis)
    {
        /*  Found a matching class?
        */
        if(DclStrICmp(pThis->sCommand.pszClass, pEntry->sCommand.pszClass) == 0)
        {
            /*  Find the alphabetical spot within this class...
            */
            while(pThis && (DclStrICmp(pThis->sCommand.pszClass, pEntry->sCommand.pszClass) == 0))
            {
                if(DclStrICmp(pThis->sCommand.pszName, pEntry->sCommand.pszName) > 0)
                    break;

                pPrev = pThis;
                pThis = pThis->pNext;
            }

            break;
        }

        pPrev = pThis;
        pThis = pThis->pNext;
    }

    /*  Insert the entry at the current position in the list
    */
    pEntry->pNext = pThis;

    if(pPrev)
        pPrev->pNext = pEntry;
    else
        hShell->pCommandList = pEntry;

    return DCLSTAT_SUCCESS;
}



                    /*------------------------------*\
                     *                              *
                     *     Protected Functions      *
                     *                              *
                    \*------------------------------*/


/*-------------------------------------------------------------------
    Protected: DclShellAddMultipleCommands()

    This function adds multiple commands to an instance of the
    DCL Command Shell.

    NOTE:  The pszClass and pszDescription values must point to
           static data that persists throughout the life of the
           shell commands.  A private copy of the pszName value
           is stored, so that does not need to be static.

    Parameters:
        hShell        - The handle of the shell instance to use.
        paCommands    - A pointer to an array of DCLSHELLCOMMAND
                        structures.
        nCommandCount - The number of commands to add.
        pszClassName  - The command class name.  Note that this
                        class name is ONLY used if the command
                        definition does not include one already.
        pszPrefix     - The command prefix.

    Return Value:
        Status indicating if the operation was successful.
-------------------------------------------------------------------*/
DCLSTATUS DclShellAddMultipleCommands(
    DCLSHELLHANDLE          hShell,
    const DCLSHELLCOMMAND  *paCommands,
    unsigned                nCommandCount,
    const char             *pszClassName,
    const char             *pszPrefix)
{
    unsigned                nn;

    DclAssertReadPtr(hShell, sizeof(*hShell));
    DclAssert(pszClassName);
    DclAssert(pszPrefix);

    for(nn = 0; nn < nCommandCount; nn++)
    {
        char            szTempBuff[MAX_COMMAND_LEN];
        DCLSHELLCOMMAND cmd;
        DCLSTATUS       dclStat;

        /*  Make a local copy of the command information, and modify
            it to contain the properly prefixed command name and the
            command class.
        */
        cmd = *paCommands;

        /*  Only if the command class is not specified, do we use the
            passed in value.
        */
        if(!cmd.pszClass)
            cmd.pszClass = pszClassName;

        DclSNPrintf(szTempBuff, sizeof(szTempBuff), cmd.pszName, pszPrefix);
        cmd.pszName = szTempBuff;

        dclStat = DclShellAddCommand(hShell, &cmd);
        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;

        paCommands++;
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Protected: DclShellRenameCommand()

    Rename a command in an instance of the DCL Command Shell.

    NOTE: Commands which are renamed will NOT be re-sorted to be
          alphabetical relative to the other commands.

    Parameters:
        hShell      - The handle of the shell instance to use.
        pszName     - Name of the command to rename.
        pszNewName  - The new command name.

    Return Value:
        Status indicating if the operation was successful.
-------------------------------------------------------------------*/
DCLSTATUS DclShellRenameCommand(
    DCLSHELLHANDLE      hShell,
    const char         *pszName,
    const char         *pszNewName)
{
    COMMANDINFO        *pEntry;

    DclAssertReadPtr(hShell, sizeof(*hShell));
    
    /*  Find the command in the list.
    */
    pEntry = hShell->pCommandList;
    while(pEntry)
    {
        if(DclStrICmp(pEntry->sCommand.pszName, pszName) == 0)
            break;

        pEntry = pEntry->pNext;
    }

    /*  Remove the command if we found it.
    */
    if(pEntry)
    {
        /*  Make a private copy of the actual command name
        */
        DclAssert(DclStrLen(pszNewName) < sizeof(pEntry->szCmdName));
        DclStrNCpy(pEntry->szCmdName, pszNewName, sizeof(pEntry->szCmdName));

        pEntry->sCommand.pszName = pEntry->szCmdName;

        return DCLSTAT_SUCCESS;
    }
    else
    {
        return DCLSTAT_BADPARAMETER;
    }
}


/*-------------------------------------------------------------------
    Protected: DclShellRemoveCommand()

    Remove a command from an instance of the DCL Command Shell.

    Parameters:
        hShell      - The handle of the shell instance to use.
        pszName     - Name of the command to remove.

    Return Value:
        Status indicating if the operation was successful.
-------------------------------------------------------------------*/
DCLSTATUS DclShellRemoveCommand(
    DCLSHELLHANDLE      hShell,
    const char         *pszName)
{
    COMMANDINFO        *pEntry;
    COMMANDINFO        *pPrev = NULL;

    DclAssertReadPtr(hShell, sizeof(*hShell));
    
    /*  Find the command in the list.
    */
    pEntry = hShell->pCommandList;
    while(pEntry)
    {
        if(DclStrICmp(pEntry->sCommand.pszName, pszName) == 0)
        {
            if(pPrev)
                pPrev->pNext = pEntry->pNext;

            if(pEntry == hShell->pCommandList)
                hShell->pCommandList = pEntry->pNext;

            DclMemFree(pEntry);

            return DCLSTAT_SUCCESS;
        }

        pPrev = pEntry;
        pEntry = pEntry->pNext;
    }

    return DCLSTAT_BADPARAMETER;
}


/*-------------------------------------------------------------------
    Protected: DclShellParams()

    Returns the shell parameter information in the supplied
    structure.

    Parameters:
        hShell     - The shell instance being operated on.
        pParams    - A pointer to the DCLSHELLPARAMS structure to
                     fill.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclShellParams(
    DCLSHELLHANDLE  hShell,
    DCLSHELLPARAMS *pParams)
{
    DclAssertReadPtr(hShell, sizeof(*hShell));
    DclAssert(pParams);

    *pParams = hShell->params;

    return;
}


                    /*------------------------------*\
                     *                              *
                     *      Private Functions       *
                     *                              *
                    \*------------------------------*/


/*-------------------------------------------------------------------
    Private: DclShellConsoleInput()

    Gets a single line from the console.

    Parameters:
        hShell     - The shell instance being operated on.
        pszBuffer  - A buffer in which the input is stored.
        nMaxLen    - The buffer length

    Return Value:
        Returns the length of the line.  Returns UINT_MAX if console
        input function is not supported, or UINT_MAX-1 if a request
        to terminate the process was made.
-------------------------------------------------------------------*/
unsigned DclShellConsoleInput(
    DCLSHELLHANDLE  hShell,
    char           *pszBuffer,
    unsigned        nMaxLen)
{
    unsigned        uIndex;
    char            chr;
    unsigned        nQueueIndex = 0;
    DCLSTATUS       dclStat;

    DclAssertReadPtr(hShell, sizeof(*hShell));
    DclAssert(pszBuffer);
    DclAssert(nMaxLen);

    /*  Get a command string from the input device
    */
    uIndex = 0;
    while(uIndex < nMaxLen)
    {
        D_BOOL  fScriptChar = FALSE;

        /*  Get a single character
        */
        if(hShell->fCommandQueued)
        {
            /*  Read the queued command a character at a time
            */
            chr = hShell->szCmdQueue[nQueueIndex];
            nQueueIndex++;

            /*  If we have read the last char, change it from a
                NULL to a linefeed, and clear the flag.
            */
            if(!chr)
            {
                chr = '\n';
                hShell->fCommandQueued = FALSE;
            }

            fScriptChar = TRUE;
        }
        else if(hShell->pszScript && hShell->pszScript[hShell->nScriptIndex])
        {
            /*  Use the script if there is one and we have not exhausted it.
            */
            chr = hShell->pszScript[hShell->nScriptIndex];
            hShell->nScriptIndex++;

            fScriptChar = TRUE;
        }
        else
        {
          #if DCL_OSFEATURE_CONSOLEINPUT

            /*  Normally we will not be here if there is a script, however if
                the script is incorrectly terminated, it will finish and we
                will begin reading the normal input stream.
            */
            dclStat = DclInputChar(hShell->params.hDclInst, (D_UCHAR*)&chr, 0);
            if(dclStat == DCLSTAT_INPUT_DISABLED)
            {
                return UINT_MAX;
            }
            else if(dclStat == DCLSTAT_INPUT_TERMINATE)
            {
                return UINT_MAX-1;
            }
            else if(dclStat != DCLSTAT_SUCCESS)
            {
                chr = 0;
            }

          #else

            chr = 0;

          #endif
        }

      #if DEBUG_CHARS
        DclPrintf("%02x ", (unsigned char)chr);
      #endif

        if((signed char)chr < 0)
            continue;

        switch(chr)
        {
            /*  Zero is returned if DclInputChar() is not supported.
            */
            case 0:
            {
                DclAssert(uIndex == 0);

                pszBuffer[uIndex] = 0;

                return UINT_MAX;
            }

            /*  Ignore CRs since lines (console or scripts) will generally be
                CR/LF or LF terminated.  This does mean that the shell will
                only operate if all lines are LF terminated.
            */
            case '\r':
            {
                continue;
            }

            /*  Linefeed, this is the end of the line.
            */
            case '\n':
            {
                if(hShell->params.fEcho)
                    DclPrintf("\n");
                break;
            }

            /*  Backspace.
            */
            case '\b':
            {
                if(uIndex)
                {
                    if(hShell->params.fEcho)
                        DclPrintf("\b \b");

                    uIndex--;
                }
                continue;
            }

            /*  Add this char to the message string.
            */
            default:
            {
                if(hShell->params.fEcho || fScriptChar)
                    DclPrintf("%c", chr);

                pszBuffer[uIndex] = chr;
                uIndex++;
                continue;
            }
        }

        /*  We will only get here if we are at the end of the line.
        */
        DclAssert(chr == '\n');
        break;
    }

    /*  Terminate the message string.
    */
    pszBuffer[uIndex] = 0;

    dclStat = DclEnvSubstitute(hShell->params.hEnv, pszBuffer, nMaxLen);
    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("Error %lX performing environment variable substitution, continuing...\n", dclStat);

    uIndex = DclStrLen(pszBuffer);
    DclAssert(uIndex < nMaxLen);

    return uIndex;
}



                    /*------------------------------*\
                     *                              *
                     *   Standard Shell Commands    *
                     *                              *
                    \*------------------------------*/


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_help(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    COMMANDINFO    *pThis;
    const char     *pLastClass = NULL;

    DclAssertReadPtr(hShell, sizeof(*hShell));

    /*  Explicitly unused parameter 
    */
    ((void)argc);
    ((void)argv);

    pThis = hShell->pCommandList;
    while(pThis)
    {
        if( (DclStrICmp(pThis->sCommand.pszClass, SCRIPTINGCLASS) != 0) &&
            (DclStrICmp(pThis->sCommand.pszClass, DEBUGGINGCLASS) != 0) &&
            (DclStrICmp(pThis->sCommand.pszClass, PROFILERCLASS) != 0) &&
            (DclStrICmp(pThis->sCommand.pszClass, REQUESTORCLASS) != 0) )
        {
            if( (DclStrICmp(pThis->sCommand.pszClass, GENERALCLASS) != 0) ||
               ((DclStrICmp(pThis->sCommand.pszClass, GENERALCLASS) == 0) &&
               ((DclStrICmp(pThis->sCommand.pszName, HELPCMD) == 0) ||
                (DclStrICmp(pThis->sCommand.pszName, SHELLHELPCMD) == 0) ||
                (DclStrICmp(pThis->sCommand.pszName, DEBUGGINGHELPCMD) == 0) ||
                (DclStrICmp(pThis->sCommand.pszName, hShell->pszExitCmd) == 0))) )
            {
                if(!pLastClass || DclStrICmp(pThis->sCommand.pszClass, pLastClass) != 0)
                {
                    DclPrintf("%s Commands:\n", pThis->sCommand.pszClass);
                    pLastClass = pThis->sCommand.pszClass;
                }

                DclPrintf("  %-20s - %s\n", pThis->sCommand.pszName, pThis->sCommand.pszDescription);
            }
        }

        pThis = pThis->pNext;
    }

    DclPrintf("\nHelp for individual commands can be obtained by entering \"CmdName /?\".\n");
    DclPrintf("Partial command names may be specified, so long as they are unique.\n\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_ShellHelp(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    COMMANDINFO    *pThis;
    const char     *pLastClass = NULL;

    DclAssertReadPtr(hShell, sizeof(*hShell));

    /*  Explicitly unused parameters 
    */
    ((void)argc);
    ((void)argv);

    pThis = hShell->pCommandList;
    while(pThis)
    {
        if((DclStrICmp(pThis->sCommand.pszClass, SCRIPTINGCLASS) == 0) ||
            (DclStrICmp(pThis->sCommand.pszClass, GENERALCLASS) == 0))
        {
            if(pThis->sCommand.pszClass != pLastClass)
            {
                DclPrintf("%s Commands:\n", pThis->sCommand.pszClass);
                pLastClass = pThis->sCommand.pszClass;
            }

            DclPrintf("  %-20s - %s\n", pThis->sCommand.pszName, pThis->sCommand.pszDescription);
        }

        pThis = pThis->pNext;
    }

    DclPrintf("\nHelp for individual commands can be obtained by entering \"CmdName /?\".\n\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_DebuggingHelp(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    COMMANDINFO    *pThis;
    const char     *pLastClass = NULL;

    DclAssertReadPtr(hShell, sizeof(*hShell));
    
    (void) argc;
    (void) argv;

    pThis = hShell->pCommandList;
    while(pThis)
    {
        if( (DclStrICmp(pThis->sCommand.pszClass, DEBUGGINGCLASS) == 0) ||
            (DclStrICmp(pThis->sCommand.pszClass, REQUESTORCLASS) == 0) ||
            (DclStrICmp(pThis->sCommand.pszClass, PROFILERCLASS) == 0) )
        {
            if(pThis->sCommand.pszClass != pLastClass)
            {
                DclPrintf("%s Commands:\n", pThis->sCommand.pszClass);
                pLastClass = pThis->sCommand.pszClass;
            }

            DclPrintf("  %-20s - %s\n", pThis->sCommand.pszName, pThis->sCommand.pszDescription);
        }

        pThis = pThis->pNext;
    }

    DclPrintf("\nHelp for individual commands can be obtained by entering \"CmdName /?\".\n\n");

    return DCLSTAT_SUCCESS;
}


#if DCLCONF_SHELL_TOOLS

/*-------------------------------------------------------------------
    NOTE:  This function returns the same status as that currently
           recorded so the state does not change by running this
           command.
-------------------------------------------------------------------*/
static DCLSTATUS exec_echo(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    int             ii;

    DclAssertReadPtr(hShell, sizeof(*hShell));
    
    /*  Display help screen
    */
    if(argc == 1 || ISHELPREQUEST())
    {
        DclPrintf("The \"%s\" command displays the specified text on the console.\n", argv[0]);
        DclPrintf("This command does not change the current shell \"status\" value.\n");

        return hShell->ulStatus;
    }

    for(ii=1; ii<argc; ii++)
        DclPrintf("%s ", argv[ii]);

    DclPrintf("\n");

    return hShell->ulStatus;
}


/*-------------------------------------------------------------------
    NOTE:  If this command is successful, this function returns the
           same status as that currently recorded so the state does
           not change by running this command.

           If this command fails, it will return an updated status
           code.
-------------------------------------------------------------------*/
static DCLSTATUS exec_goto(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    unsigned        nLen;
    unsigned        ii;

    DclAssertReadPtr(hShell, sizeof(*hShell));
    
    /*  Display help screen
    */
    if(argc != 2 || ISHELPREQUEST())
    {
        DclPrintf("This command transfers control to the statement following the specified\n");
        DclPrintf("label in a script.\n\n");
        DclPrintf("Syntax:  %s Label\n\n", argv[0]);
        DclPrintf("Where:\n");
        DclPrintf("  Label - Is the script label to jump to.  Labels must begin with a colon\n");
        DclPrintf("          in the first column, and are not case-sensitive.\n\n");
        DclPrintf("Note that this command may be used to interactively start execution at the\n");
        DclPrintf("specified label in the last executed script, even if it has completed.\n\n");
        DclPrintf("If the operation is successful, this command will return the original\n");
        DclPrintf("status code from the previous command.\n");

        return hShell->ulStatus;
    }

    if(!hShell->pszScript)
    {
        DclPrintf("There is no current script in which to search for the label\n");
        return DCLSTAT_FAILURE;
    }

    /*  Start at the beginning of the current/last executed script, and
        search for a label that matches.  The colon must be in the first
        column, and the non case-sensitive label is terminated by the
        end-of-line, or white space.
    */
    nLen = DclStrLen(argv[1]);
    ii = 0;

    while(TRUE)
    {
        if((hShell->pszScript[ii] == ':') &&
            (DclStrNICmp(&hShell->pszScript[ii+1], argv[1], nLen) == 0) &&
            ((hShell->pszScript[ii+1+nLen] == 0) ||
            (hShell->pszScript[ii+1+nLen] == ' ') ||
            (hShell->pszScript[ii+1+nLen] == '\t') ||
            (hShell->pszScript[ii+1+nLen] == '\r') ||
            (hShell->pszScript[ii+1+nLen] == '\n')))
        {
            /*  Found it, set the script index, and return the original
                status code.
            */
            hShell->nScriptIndex = ii;
            return hShell->ulStatus;
        }

        /*  Advance to the start of the next line
        */
        while(hShell->pszScript[ii])
        {
            ii++;

            if(hShell->pszScript[ii-1] == '\n')
                break;
        }

        if(!hShell->pszScript[ii])
        {
            DclPrintf("Label \"%s\" not found!\n", argv[1]);
            return DCLSTAT_FAILURE;
        }
    }
}


/*-------------------------------------------------------------------
    NOTE:  This function returns the same status as that currently
           recorded so the state does not change by running this
           command.
-------------------------------------------------------------------*/
static DCLSTATUS exec_if(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    unsigned        fNot;
    unsigned        fIsStatus = FALSE;
    unsigned        fDifferent;

    DclAssertReadPtr(hShell, sizeof(*hShell));
    
    /*  Display help screen
    */
    if(argc < 5 || ISHELPREQUEST())
    {
        DclPrintf("This command performs a comparison and conditionally executes the specified\n");
        DclPrintf("command.\n\n");
        DclPrintf("Syntax:  %s Value1 Operator Value2 Command [Command-Arguments]\n\n", argv[0]);
        DclPrintf("Where:\n");
        DclPrintf("    Value1 - A value to compare with Value2\n");
        DclPrintf("  Operator - The type of comparison operation to perform, which must be either\n");
        DclPrintf("             \"==\" or \"!=\".\n");
        DclPrintf("    Value2 - A value to compare with Value1\n");
        DclPrintf("   Command - The shell command to execute.   Any data following the command\n");
        DclPrintf("             will be treated as arguments to the command.  The maximum length\n");
        DclPrintf("             of the command and any arguments is 128 bytes.\n\n");
        DclPrintf("Standard environment variable substitution is supported for all arguments.  It\n");
        DclPrintf("is recommended that the Value1 and Value2 arguments be surrounded in double\n");
        DclPrintf("quotes.  String comparisons are NOT case-sensitive.\n\n");
        DclPrintf("If Value1 is the word \"status\" (no quotes), then Value2 must be a decimal or\n");
        DclPrintf("hex numeric value which will be compared to the status from the last command\n");
        DclPrintf("executed.\n");

        return hShell->ulStatus;
    }

    if(DclStrICmp(argv[1], "status") == 0)
    {
        fIsStatus = TRUE;
    }

    if(DclStrICmp(argv[2], "==") == 0)
    {
        fNot = FALSE;
    }
    else if(DclStrICmp(argv[2], "!=") == 0)
    {
        fNot = TRUE;
    }
    else
    {
        DclPrintf("IF: Syntax error, unrecognized operator \"%s\"\n", argv[2]);
        return hShell->ulStatus;
    }

    if(fIsStatus)
    {
        D_UINT32    ulValue;

        if(!ParseNumericValue(argv[3], &ulValue))
            return hShell->ulStatus;

        /*  Determine if the condition is TRUE or FALSE
        */
        if(hShell->ulStatus != ulValue)
            fDifferent = TRUE;
        else
            fDifferent = FALSE;
    }
    else
    {
        /*  Determine if the condition is TRUE or FALSE
        */
        fDifferent = DclStrICmp(argv[1], argv[3]);
    }

    /*  If the condition is not met, with respect to the operator, ignore
        the command and return the orginal status code.
    */
    if((fDifferent && !fNot) || (!fDifferent && fNot))
        return hShell->ulStatus;

    /*  The condition evaluated to TRUE.  Concatenate the command and
        command arguments into one string and queue it up.
    */
    {
        int iArgNum = 4;
        int iPos = 0;

        while(iArgNum < argc)
        {
            int iLen;

            iLen = DclSNPrintf(&hShell->szCmdQueue[iPos], sizeof(hShell->szCmdQueue) - iPos, "%s ", argv[iArgNum]);
            if(iLen == -1)
            {
                DCLPRINTF(1, ("Command too long\n"));

                /*  Ensure it is null-terminated
                */
                hShell->szCmdQueue[sizeof(hShell->szCmdQueue)-1] = 0;
                break;
            }

            iPos += iLen;

            iArgNum++;
        }

        hShell->fCommandQueued = TRUE;
    }

    return hShell->ulStatus;
}


/*-------------------------------------------------------------------
    NOTE:  This function returns the same status as that currently
           recorded so the state does not change by running this
           command.
-------------------------------------------------------------------*/
static DCLSTATUS exec_pause(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DclAssertReadPtr(hShell, sizeof(*hShell));
    
    /*  Display help screen
    */
    if(argc != 1 || ISHELPREQUEST())
    {
      #if DCL_OSFEATURE_CONSOLEINPUT

        DclPrintf("The \"%s\" command displays a \"Press [Enter] to continue...\" message and\n", argv[0]);
        DclPrintf("waits for the user to press [Enter].  This command uses the standard console\n");
        DclPrintf("input functions even if a script or queued command is being executed.\n\n");
        DclPrintf("This command does not change the current shell \"status\" value.\n");

      #else

        DclPrintf("The \"%s\" command is disabled because DCL_OSFEATURE_CONSOLEINPUT is FALSE\n", argv[0]);

      #endif

        return hShell->ulStatus;
    }

  #if DCL_OSFEATURE_CONSOLEINPUT

    DclPrintf("Press [Enter] to continue...");

    while(TRUE)
    {
        D_UCHAR     uChar;
        DCLSTATUS   dclStat;

        dclStat = DclInputChar(hShell->params.hDclInst, &uChar, 0);
        if(dclStat == DCLSTAT_INPUT_DISABLED)
        {
            break;
        }
        else if(dclStat == DCLSTAT_INPUT_TERMINATE)
        {
            hShell->fExit = TRUE;
            hShell->ulStatus = dclStat;
            break;
        }
        else if(dclStat != DCLSTAT_SUCCESS)
        {
            hShell->ulStatus = dclStat;
            break;
        }

        if(uChar == '\n')
            break;
    }

  #endif

    return hShell->ulStatus;
}


/*-------------------------------------------------------------------
    NOTE:  If this command is successful, this function returns the
           same status as that currently recorded so the state does
           not change by running this command.

           If this command fails, it will return an updated status
           code.
-------------------------------------------------------------------*/
static DCLSTATUS exec_run(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLFSSTAT       stat;
    char           *pMem;
    DCLSTATUS       DclStatus;

    DclAssertReadPtr(hShell, sizeof(*hShell));

    /*  Display help screen
    */
    if(argc < 2 || ISHELPREQUEST())
    {
        DclPrintf("This command loads a shell command script into memory and runs it.  Control\n");
        DclPrintf("is returned to the console once the script is completed.  However, if one\n");
        DclPrintf("script is used to run another, control never returns to the original script.\n\n");
        DclPrintf("Syntax:  %s ScriptFile\n\n", argv[0]);
        DclPrintf("Where:\n");
        DclPrintf("  ScriptFile - The filename of the script file to run.  Specify '*' to run\n");
        DclPrintf("               the last script executed, or to restart a currently running\n");
        DclPrintf("               script.\n");

        return hShell->ulStatus;
    }

    if(DclStrCmp(argv[1], "*") == 0)
    {
        if(!hShell->pszScript)
        {
            DclPrintf("There is no script loaded\n");
            return DCLSTAT_FAILURE;
        }

        hShell->nScriptIndex = 0;

        /*  All good -- return the original status code.
        */
        return hShell->ulStatus;
    }

    if(DclFsStat(argv[1], &stat))
    {
        DclPrintf("Unable to access the file \"%s\"\n", argv[1]);
        return DCLSTAT_FAILURE;
    }

    pMem = DclMemAlloc(stat.ulSize+2);
    if(pMem)
    {
        DCLFSFILEHANDLE   hFile;

        DclStatus = DclFsFileOpen(argv[1], "rb", &hFile);
        if(!DclStatus)
        {
            size_t  nBytesRead;

            nBytesRead = DclOsFileRead(pMem, 1, stat.ulSize, hFile);

            DclFsFileClose(hFile);

            if(nBytesRead != stat.ulSize)
            {
                DclPrintf("Error reading file!\n");
                DclMemFree(pMem);
                return DCLSTAT_FAILURE;
            }

            /*  Ensure that the script is properly terminated
            */
            pMem[nBytesRead+0] = '\n';
            pMem[nBytesRead+1] = 0;
        }
        else
        {
            DclPrintf("Error opening file!\n");
            DclMemFree(pMem);
            return DCLSTAT_FAILURE;
        }
    }
    else
    {
        DclPrintf("Unable to allocate memory\n");
        return DCLSTAT_MEMALLOCFAILED;
    }

    /*  If we internally allocated the memory for any previous script, we
        are responsible for internally releasing it as well.  If it was
        handed to us on entry, don't release it as we have no idea where
        it came from.
    */
    if(hShell->fWeAllocedScript)
    {
        DclAssert(hShell->pszScript);
        DclMemFree((void*)hShell->pszScript);
    }

    /*  Prepare the new script for execution...
    */
    hShell->pszScript = pMem;
    hShell->fWeAllocedScript = TRUE;
    hShell->nScriptIndex = 0;

    /*  All good -- return the original status code.
    */
    return hShell->ulStatus;
}


/*-------------------------------------------------------------------
    NOTE:  This function returns the same status as that currently
           recorded so the state does not change by running this
           command.
-------------------------------------------------------------------*/
static DCLSTATUS exec_set(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSTATUS       dclStat;
    char           *pEqual;

    DclAssertReadPtr(hShell, sizeof(*hShell));
    
    /*  Display help screen
    */
    if(argc > 2 || ISHELPREQUEST())
    {
        DclPrintf("This command adds, deletes, or displays environment variables.\n\n");
        DclPrintf("Syntax:  %s [VarName=[Value] | VarPart]\n\n", argv[0]);
        DclPrintf("Where:\n");
        DclPrintf(" VarName - The environment variable name to set or delete.\n");
        DclPrintf("   Value - The value to assign to VarName.\n");
        DclPrintf(" VarPart - A partial variable name to match in the environment.  Any variables\n");
        DclPrintf("           starting with VarPart will be displayed.\n\n");
        DclPrintf("If no parameters are specified, then the current contents of the environment\n");
        DclPrintf("will be displayed.  Environment variable names are case preserved, but not\n");
        DclPrintf("case-sensitive, and may not contain an '=' sign.  The environment size is\n");
        DclPrintf("dynamically increased in size as needed.\n\n");
        DclPrintf("To set an environment variable value which contains embedded spaces, enclose\n");
        DclPrintf("the VarName=Value sequence in double quotes.\n");

        return hShell->ulStatus;
    }

    if(argc == 1)
    {
        DclEnvDisplay(hShell->params.hEnv, NULL);
        return hShell->ulStatus;
    }

    pEqual = DclStrChr(argv[1], '=');
    if(!pEqual)
    {
        DclEnvDisplay(hShell->params.hEnv, argv[1]);
        return hShell->ulStatus;
    }

    if(pEqual == argv[1])
    {
        DclPrintf("Syntax error in \"%s\"\n", argv[1]);
        return hShell->ulStatus;
    }

    if(*(pEqual+1) == 0)
    {
        *pEqual = 0;

        if(!DclEnvVarDelete(hShell->params.hEnv, argv[1]))
            DclPrintf("Variable \"%s\" not found\n", argv[1]);

        return hShell->ulStatus;
    }

    *pEqual++ = 0;

    /*  If the value string has matching leading and trailing
        single or double quotes, strip them off.
    */
    if(((*pEqual == '"') || (*pEqual == '\''))
        && (DclStrLen(pEqual) > 1)
        && (*(pEqual + DclStrLen(pEqual) - 1) == *pEqual))
    {
        pEqual++;
        *(pEqual + DclStrLen(pEqual) - 1) = 0;
    }

    dclStat = DclEnvVarAdd(hShell->params.hEnv, argv[1], pEqual);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Error %lX setting variable \"%s\" to \"%s\"\n", dclStat, argv[1], pEqual);
        return hShell->ulStatus;
    }

    return hShell->ulStatus;
}

#endif


/*-------------------------------------------------------------------
    NOTE:  This function returns the same status as that currently
           recorded so the state does not change by running this
           command.
-------------------------------------------------------------------*/
static DCLSTATUS exec_wait(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    D_UINT32        ulSeconds;
    D_UINT32        ulMilliseconds;
    DCLTIMESTAMP    ts;

    DclAssertReadPtr(hShell, sizeof(*hShell));
    
    /*  Display help screen
    */
    if(argc != 2 || ISHELPREQUEST())
    {
        DclPrintf("The \"%s\" command waits for the specified number of seconds\n", argv[0]);
        DclPrintf("to elapse before continuing execution.\n\n");
        DclPrintf("This command does not change the current shell \"status\" value.\n");

        return hShell->ulStatus;
    }

    if(!ParseNumericValue(argv[1], &ulSeconds))
        return hShell->ulStatus;

    ts = DclTimeStamp();

    if(ulSeconds > D_UINT32_MAX / 1000)
        ulMilliseconds = D_UINT32_MAX;
    else
        ulMilliseconds = ulSeconds * 1000;

    /*  We've now calculated the total number of milliseconds to sleep.  
        However, there is no guarantee that sleep will wait the full
        period, or even a guarantee that sleep is implemented at all,
        so use a combination of 10 MS sleeps in a loop to ensure that
        we really do wait the prescribed period.
    */

    while(DclTimePassed(ts) < ulMilliseconds)
        DclOsSleep(10);

    return hShell->ulStatus;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_status(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DclAssertReadPtr(hShell, sizeof(*hShell));
    
    /*  Explicitly unused parameters 
    */
    ((void)argc);
    ((void)argv);

    DclPrintf("Last command status: %lX\n", hShell->ulStatus);

    /*  Return the same status to so that the act of examining the
        status does NOT change the current status.
    */
    return hShell->ulStatus;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_time(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    D_TIME          ullTime;
    char            szDateTime[64];

    /*  Explicitly unused parameter 
    */
    ((void)hShell);
    ((void)argc);
    ((void)argv);

    DclOsGetDateTime(&ullTime);
    DclDateTimeString(&ullTime, szDateTime, 64);
    DclPrintf("%s\n", szDateTime);
    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_TimeStamp(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DclAssertReadPtr(hShell, sizeof(*hShell));
    
    if(argc == 2)
    {
        if(DclStrICmp(argv[1], "on") == 0)
        {
            hShell->fTimeStamp = TRUE;
            return DCLSTAT_SUCCESS;
        }

        if(DclStrICmp(argv[1], "off") == 0)
        {
            hShell->fTimeStamp = FALSE;
            return DCLSTAT_SUCCESS;
        }
    }

    DclPrintf("%s enables or disables the use of timestamps in the console prompt.\n\n", argv[0]);
    DclPrintf("Syntax:  %s on | off\n\n", argv[0]);

    return DCLSTAT_FAILURE;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_Width(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DclAssertReadPtr(hShell, sizeof(*hShell));
    
    if(argc == 2)
    {
        D_UINT32    ulWidth;

        if(!ParseNumericValue(argv[1], &ulWidth))
            return DCLSTAT_FAILURE;

        if(ulWidth < 1 || ulWidth > UINT_MAX)
        {
            DclPrintf("The specified width value is not valid\n");
            return DCLSTAT_FAILURE;
        }

        hShell->params.nWidth = (unsigned)ulWidth;
    }

    DclPrintf("The console width is set to %u characters\n\n", hShell->params.nWidth);

    return DCLSTAT_SUCCESS;
}


#if DCLCONF_SHELL_TESTS

/*-------------------------------------------------------------------
    Run the DCL Unit Tests
-------------------------------------------------------------------*/
static DCLSTATUS exec_dcltest(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLTOOLPARAMS   tp = {0};
    char            szCmdLine[256];
    int             ii;

    DclAssertReadPtr(hShell, sizeof(*hShell));

    /*  Combine the args to a single string.
    */
    szCmdLine[0] = 0;
    for(ii = 1; ii < argc; ii++)
    {
        DclStrCat(szCmdLine, argv[ii]);
        DclStrCat(szCmdLine, " ");
    }

    tp.hDclInst = hShell->params.hDclInst;
    tp.pszCmdName = argv[0];
    tp.pszCmdLine = szCmdLine;

    return DclTestMain(&tp);
}


/*-------------------------------------------------------------------
    Run the DEV IO Tests
-------------------------------------------------------------------*/
static DCLSTATUS exec_deviotest(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLTOOLPARAMS   tp = {0};
    char            szCmdLine[256];
    int             ii;

    DclAssertReadPtr(hShell, sizeof(*hShell));
    
    /*  Combine the args to a single string.
    */
    szCmdLine[0] = 0;
    for(ii = 1; ii < argc; ii++)
    {
        DclStrCat(szCmdLine, argv[ii]);
        DclStrCat(szCmdLine, " ");
    }

    tp.hDclInst = hShell->params.hDclInst;
    tp.pszCmdName = argv[0];
    tp.pszCmdLine = szCmdLine;

    return DclTestDevIOMain(&tp);
}

#endif


/*-------------------------------------------------------------------
    Exit the command shell

    NOTE:  This function returns the same status as that currently
           recorded so the state does not change by running this
           command.
-------------------------------------------------------------------*/
static DCLSTATUS exec_exit(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DclAssertReadPtr(hShell, sizeof(*hShell));
    
    /*  Display help screen
    */
    if(argc > 2 || ISHELPREQUEST())
    {
        DclPrintf("The \"%s\" command causes the shell to terminate.  An optional\n", argv[0]);
        DclPrintf("decimal or hex (0x prefix) status code may be returned.  If a\n");
        DclPrintf("status code is not specified, the status of the last command will\n");
        DclPrintf("be returned.\n");

        return hShell->ulStatus;
    }

    if(argc > 1)
    {
        if(!ParseNumericValue(argv[1], &hShell->ulStatus))
            return hShell->ulStatus;
    }

    hShell->fExit = TRUE;

    return hShell->ulStatus;
}


/*-------------------------------------------------------------------
    Display statistics
-------------------------------------------------------------------*/
static DCLSTATUS exec_stats(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLTOOLPARAMS   tp = {0};
    char            szCmdLine[256];
    int             ii;

    DclAssertReadPtr(hShell, sizeof(*hShell));
    
    /*  Combine the args to a single string.
    */
    szCmdLine[0] = 0;
    for(ii = 1; ii < argc; ii++)
    {
        DclStrCat(szCmdLine, argv[ii]);
        DclStrCat(szCmdLine, " ");
    }

    /*  Run the test
    */
    tp.hDclInst = hShell->params.hDclInst;
    tp.pszCmdName = argv[0];
    tp.pszCmdLine = szCmdLine;

    return DclStats(&tp);
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_AssertMode(
    DCLSHELLHANDLE      hShell,
    int                 argc,
    char              **argv)
{
    DCLSTATUS           dclStat;
    int                 iArgNum;
    D_UINT32            ulHandle = 0;
    DCLASSERTMODE       nMode = DCLASSERTMODE_QUERY;
    DCLASSERTMODE       nOldMode = DCLASSERTMODE_INVALID; /* Init'ed to placate a picky compiler only */
    static const char  *apszModes[] =
    {
        /*  Must be properly ordered!
        */
        "AUTODEBUG",        /* DCLASSERTMODE_AUTODEBUGGERFAIL   */
        "QUERYDEBUG",       /* DCLASSERTMODE_QUERYDEBUGGERFAIL  */
        "DEBUG",            /* DCLASSERTMODE_DEBUGGERFAIL       */
        "FAIL",             /* DCLASSERTMODE_FAIL               */
        "WARN",             /* DCLASSERTMODE_WARN               */
        "IGNORE"            /* DCLASSERTMODE_IGNORE             */
    };

    /*  Any changes to the enumeration requires updates to this code!
    */
    DclProductionAssert(DCLASSERTMODE_LOWLIMIT == 0);
    DclProductionAssert(DCLASSERTMODE_INVALID == 0);
    DclProductionAssert(DCLASSERTMODE_AUTODEBUGGERFAIL == 1);
    DclProductionAssert(DCLASSERTMODE_QUERYDEBUGGERFAIL == 2);
    DclProductionAssert(DCLASSERTMODE_DEBUGGERFAIL == 3);
    DclProductionAssert(DCLASSERTMODE_FAIL == 4);
    DclProductionAssert(DCLASSERTMODE_WARN == 5);
    DclProductionAssert(DCLASSERTMODE_IGNORE == 6);
    DclProductionAssert(DCLASSERTMODE_HIGHLIMIT == 7);
    DclProductionAssert(DCLASSERTMODE_QUERY == -1);
    
    if(argc > 3 || ISHELPREQUEST())
        goto AssertModeSyntax;

    for(iArgNum = 1; iArgNum < argc; iArgNum++)
    {
        if(DclStrNICmp(argv[iArgNum], "/REQ:", 5) == 0)
        {
            if(!ParseNumericValue(&argv[iArgNum][5], &ulHandle))
                goto AssertModeSyntax;

            continue;
        }

        if(DclStrNICmp(argv[iArgNum], "/MODE:", 6) == 0)
        {
            for(nMode = DCLASSERTMODE_LOWLIMIT + 1; 
                nMode < DCLASSERTMODE_HIGHLIMIT;
                nMode ++)
            {
                if(DclStrICmp(&argv[iArgNum][6], apszModes[nMode-1]) == 0)
                    break;
            }

            if(nMode == DCLASSERTMODE_HIGHLIMIT)
                goto AssertModeSyntax;

            continue;
        }
        
        DclPrintf("Syntax error in \"%s\"\n", argv[iArgNum]);
        return DCLSTAT_FAILURE;
    }

    if(ulHandle)
    {
        DCLDECLAREREQUESTPACKET (ASSERT, MODE, mode);  /* DCLREQ_ASSERT_MODE */

        mode.ior.ioFunc = DCLIOFUNC_ASSERT_MODE;
        mode.ior.ulReqLen = sizeof(mode);
        mode.nNewMode = nMode;

        dclStat = DclOsRequestorDispatch(ulHandle, &mode.ior);
        if(dclStat == DCLSTAT_SUCCESS)
            nOldMode = mode.nOldMode;
        else
            DclPrintf("Requestor command failed with status %lX\n", dclStat);
    }
    else
    {
        nOldMode = DclAssertMode(hShell->params.hDclInst, nMode);
        dclStat = DCLSTAT_SUCCESS;
    }

    if(dclStat == DCLSTAT_SUCCESS)
    {
        DclAssert(nOldMode > DCLASSERTMODE_LOWLIMIT && nOldMode < DCLASSERTMODE_HIGHLIMIT);
        
        if(nMode == DCLASSERTMODE_QUERY)
            DclPrintf("The current assert mode is '%s'.\n", apszModes[nOldMode-1]);
        else
            DclPrintf("New assert mode set.  The old assert mode was '%s'.\n", apszModes[nOldMode-1]);
    }
    
    return dclStat;

  AssertModeSyntax:

    DclPrintf("This command displays or changes the behavior when asserts occur, optionally\n");
    DclPrintf("specifying a device to which the command should be directed.\n\n");
    DclPrintf("Syntax:  %s [/Mode:type] [/Req:handle]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("   /Mode:type - The assert type requested, where 'type' is one of the values\n");
    DclPrintf("                listed below.  If /Mode is not specified, the current mode will\n");
    DclPrintf("                displayed.  Valid mode types are:\n");
    DclPrintf("                  IGNORE     - Ignore asserts entirely.\n");
    DclPrintf("                  WARN       - Do nothing but display a warning.\n");
    DclPrintf("                  FAIL       - Halt the thread (the typical default behavior).\n");
    DclPrintf("                  DEBUG      - If possible pop into the debugger, FAIL if not.\n");
    DclPrintf("                  QUERYDEBUG - Query the user as to whether to pop into the\n");
    DclPrintf("                               debugger, or FAIL otherwise.\n");
    DclPrintf("                  AUTODEBUG  - Query the user for 10 seconds as to whether to\n");
    DclPrintf("                               pop into the debugger, or FAIL otherwise.\n"); 
    DclPrintf("  /Req:handle - The optional Requestor Handle to which the command will be sent.\n");
    DclPrintf("                The ReqHandle can be obtained using the 'ReqOpen' command.  Not\n");
    DclPrintf("                all environments support the requestor interface.\n\n");
    DclPrintf("If a handle is not specified, the command will apply locally (useful primarily\n");
    DclPrintf("in a monolithic environment).\n\n");

    return DCLSTAT_SHELLSYNTAXERROR;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_DebugBreak(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DclAssertReadPtr(hShell, sizeof(*hShell));
    
    if(argc != 2 || ISHELPREQUEST())
        goto DebugBreakSyntax;

    if(DclStrICmp(argv[1], "ON") == 0)
    {
        if(DclOsDebugBreak(DCLDBGCMD_CAPABILITIES | DCLDBGCMD_BREAK) == DCLSTAT_SUCCESS)
        {
            hShell->fBreakAsk = FALSE;
            hShell->fBreakOn = TRUE;
            return DCLSTAT_SUCCESS;
        }
        else
        {
            DclPrintf("The OS abstraction does not support this capability\n");
            return DCLSTAT_FAILURE;
        }
    }
    else if(DclStrICmp(argv[1], "OFF") == 0)
    {
        hShell->fBreakAsk = FALSE;
        hShell->fBreakOn = FALSE;
        return DCLSTAT_SUCCESS;
    }
    else if(DclStrICmp(argv[1], "ASK") == 0)
    {
        if(DclOsDebugBreak(DCLDBGCMD_CAPABILITIES | DCLDBGCMD_QUERY) == DCLSTAT_SUCCESS)
        {
            hShell->fBreakAsk = TRUE;
            hShell->fBreakOn = TRUE;
            return DCLSTAT_SUCCESS;
        }
        else
        {
            DclPrintf("The OS abstraction does not support this capability\n");
            return DCLSTAT_FAILURE;
        }
    }
    else if(DclStrICmp(argv[1], "NOW") == 0)
    {
        if(DclOsDebugBreak(DCLDBGCMD_BREAK) == DCLSTAT_SUCCESS)
        {
            return DCLSTAT_SUCCESS;
        }
        else
        {
            DclPrintf("The OS abstraction does not support this capability\n");
            return DCLSTAT_FAILURE;
        }
    }
    else if(DclStrICmp(argv[1], "ASSERT") == 0)
    {
        /*  (This is an undocumented option)
        */
        DclProductionError();
        return DCLSTAT_SUCCESS;
    }
    else
    {
        DclPrintf("Invalid option \"%s\"\n\n", argv[1]);
    }

  DebugBreakSyntax:

    DclPrintf("This command manipulates the Debug Break state, as described below.\n\n");
    DclPrintf("Syntax:  %s {On | Ask | Now | Off}\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("   On - Causes the debugger to be invoked just prior to executing each command.\n");
    DclPrintf("  Ask - Queries the user as to whether the debugger should be be invoked just\n");
    DclPrintf("        prior to executing each command.\n");
    DclPrintf("  Now - Invokes the debugger right now.\n");
    DclPrintf("  Off - Disables debugger invocation set with the \"on\" or \"ask\" commands.\n\n");
    DclPrintf("NOTE:  Some OS abstractions may NOT support all aspects of this functionality.\n\n");

    return DCLSTAT_SHELLSYNTAXERROR;
}


#if D_DEBUG

/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_TraceMaskGet(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    /*  Explicitly unused parameter 
    */
    ((void)hShell);

    if(argc > 1)
        goto TraceGetSyntax;

    DclPrintf("The current DCL trace mask value is:  %lX\n\n", DclTraceMaskGet());

    return DCLSTAT_SUCCESS;

  TraceGetSyntax:

    DclPrintf("This command displays the current DCL trace mask value.\n\n");
    DclPrintf("Syntax:  %s\n\n", argv[0]);

    return DCLSTAT_SHELLSYNTAXERROR;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_TraceMaskSet(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    D_UINT32        ulMask;

    /*  Explicitly unused parameter 
    */
    ((void)hShell);

    if(argc != 2 || ISHELPREQUEST())
        goto TraceSetSyntax;

    if(!ParseNumericValue(argv[1], &ulMask))
        return DCLSTAT_SHELLSYNTAXERROR;

    DclPrintf("The old DCL trace mask value was:  %lX\n",   DclTraceMaskSwap(ulMask));
    DclPrintf("The new DCL trace mask value is:   %lX\n\n", DclTraceMaskGet());

    return DCLSTAT_SUCCESS;

  TraceSetSyntax:

    DclPrintf("This command sets a new current DCL trace mask value.\n\n");
    DclPrintf("Syntax:  %s Mask\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  Mask - Is the new trace mask value, expressed as a hex (0x) or\n");
    DclPrintf("         decimal number\n\n");

    return DCLSTAT_SHELLSYNTAXERROR;
}

#endif  /* D_DEBUG */


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_TraceMax(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSTATUS       dclStat;
    D_UINT32        ulLev;

    /*  Explicitly unused parameter 
    */
    ((void)hShell);

    if(argc < 2 || argc > 3 || ISHELPREQUEST())
        goto TraceOnSyntax;

    if(!ParseNumericValue(argv[1], &ulLev))
        return DCLSTAT_SHELLSYNTAXERROR;

    if(!(ulLev >= 1 && ulLev <=3))
    {
        DclPrintf("The debug level must be a value from 1 to 3\n");
        return DCLSTAT_SUCCESS;
    }

    if(argc == 3)
    {
        if(DclStrNICmp(argv[2], "/REQ:", 5) == 0)
        {
            D_UINT32            ulHandle;
            DCLREQ_TRACEENABLE  req;

            if(!ParseNumericValue(&argv[2][5], &ulHandle))
                goto TraceOnSyntax;

            DclMemSet(&req, 0, sizeof(req));

            req.ior.ioFunc = DCLIOFUNC_TRACE_ENABLE;
            req.ior.ulReqLen = sizeof(req);
            req.nLevel = (unsigned)ulLev;

            dclStat = DclOsRequestorDispatch(ulHandle, &req.ior);
        }
        else
        {
            DclPrintf("Syntax error in \"%s\"\n", argv[2]);
            return DCLSTAT_FAILURE;
        }
    }
    else
    {
      #if D_DEBUG
        DclTracePrintf(MAKETRACEFLAGS(TRACEALWAYS, ulLev, TRACEFORCEON), "Global Trace Started\n");
        dclStat = DCLSTAT_SUCCESS;
      #else
        DclPrintf("This command can only be issued locally when the code is built in DEBUG mode\n");
        dclStat = DCLSTAT_FEATUREDISABLED;
      #endif
    }

    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("The %s command failed, Status=%lX\n", argv[0], dclStat);

    return dclStat;

  TraceOnSyntax:

    DclPrintf("This command forces on all tracing for the specified debug level on, regardless\n");
    DclPrintf("of trace class.\n\n");
    DclPrintf("Syntax:  %s Level [/Req:handle]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("        Level - Is the debug level from 1 to 3.\n");
    DclPrintf("  /Req:handle - The optional Requestor Handle to which the command will be\n");
    DclPrintf("                sent.  The ReqHandle can be obtained using the 'ReqOpen'\n");
    DclPrintf("                command.  Not all environments support the requestor interface.\n\n");
    DclPrintf("If a handle is not specified, the command will apply locally (useful primarily\n");
    DclPrintf("in a monolithic environment).  Regardless whether the command is handled locally\n");
    DclPrintf("or remotely, it is only meaningful if the target is built in DEBUG mode.\n\n");

    return DCLSTAT_SHELLSYNTAXERROR;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_TraceNormal(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSTATUS       dclStat;

    /*  Explicitly unused parameter
    */
    ((void)hShell);

    if(argc > 2 || ISHELPREQUEST())
        goto TraceOffSyntax;

    if(argc == 2)
    {
        if(DclStrNICmp(argv[1], "/REQ:", 5) == 0)
        {
            D_UINT32            ulHandle;
            DCLREQ_TRACEDISABLE req;

            if(!ParseNumericValue(&argv[1][5], &ulHandle))
                goto TraceOffSyntax;

            DclMemSet(&req, 0, sizeof(req));

            req.ior.ioFunc = DCLIOFUNC_TRACE_DISABLE;
            req.ior.ulReqLen = sizeof(req);

            dclStat = DclOsRequestorDispatch(ulHandle, &req.ior);
        }
        else
        {
            DclPrintf("Syntax error in \"%s\"\n", argv[1]);
            return DCLSTAT_FAILURE;
        }
    }
    else
    {
      #if D_DEBUG
        DclTracePrintf(MAKETRACEFLAGS(TRACEALWAYS, 1, TRACEOFF), "Global Trace Stopped\n");
        dclStat = DCLSTAT_SUCCESS;
      #else
        DclPrintf("This command can only be issued locally when the code is built in DEBUG mode\n");
        dclStat = DCLSTAT_FEATUREDISABLED;
      #endif
    }

    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("The %s command failed, Status=%lX\n", argv[0], dclStat);

    return dclStat;

  TraceOffSyntax:

    DclPrintf("This command turns the trace settings to their default values.\n\n");
    DclPrintf("Syntax:  %s [/Req:handle]\n", argv[0]);
    DclPrintf("  /Req:handle - The optional Requestor Handle to which the command will be\n");
    DclPrintf("                sent.  The ReqHandle can be obtained using the 'ReqOpen'\n");
    DclPrintf("                command.  Not all environments support the requestor interface.\n\n");
    DclPrintf("If a handle is not specified, the command will apply locally (useful primarily\n");
    DclPrintf("in a monolithic environment).  Regardless whether the command is handled locally\n");
    DclPrintf("or remotely, it is only meaningful if the target is built in DEBUG mode.\n\n");

    return DCLSTAT_SHELLSYNTAXERROR;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_TraceToggle(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSTATUS       dclStat;

    /*  Explicitly unused parameter
    */
    ((void)hShell);

    if(argc > 2 || ISHELPREQUEST())
        goto TraceToggleSyntax;

    if(argc == 2)
    {
        if(DclStrNICmp(argv[1], "/REQ:", 5) == 0)
        {
            D_UINT32            ulHandle;
            DCLREQ_TRACEDISABLE req;

            if(!ParseNumericValue(&argv[1][5], &ulHandle))
                goto TraceToggleSyntax;

            DclMemSet(&req, 0, sizeof(req));

            req.ior.ioFunc = DCLIOFUNC_TRACE_TOGGLE;
            req.ior.ulReqLen = sizeof(req);

            dclStat = DclOsRequestorDispatch(ulHandle, &req.ior);
        }
        else
        {
            DclPrintf("Syntax error in \"%s\"\n", argv[1]);
            return DCLSTAT_FAILURE;
        }
    }
    else
    {
      #if D_DEBUG && DCLCONF_OUTPUT_ENABLED
        int iEnable = DclTraceToggle();
        DclPrintf("Trace state toggled to %d\n", iEnable);
        dclStat = DCLSTAT_SUCCESS;
      #else
        DclPrintf("This command can only be issued locally when the code is built in DEBUG mode\n");
        dclStat = DCLSTAT_FEATUREDISABLED;
      #endif
    }

    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("The %s command failed, Status=%lX\n", argv[0], dclStat);

    return dclStat;

  TraceToggleSyntax:

    DclPrintf("This command toggles the trace enabled flag.\n\n");
    DclPrintf("Syntax:  %s [/Req:handle]\n", argv[0]);
    DclPrintf("  /Req:handle - The optional Requestor Handle to which the command will be\n");
    DclPrintf("                sent.  The ReqHandle can be obtained using the 'ReqOpen'\n");
    DclPrintf("                command.  Not all environments support the requestor interface.\n\n");
    DclPrintf("If a handle is not specified, the command will apply locally (useful primarily\n");
    DclPrintf("in a monolithic environment).  Regardless whether the command is handled locally\n");
    DclPrintf("or remotely, it is only meaningful if the target is built in DEBUG mode.\n\n");

    return DCLSTAT_SHELLSYNTAXERROR;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_ProfReset(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSTATUS       dclStat;

    (void)hShell;
    
    if(argc > 2 || ISHELPREQUEST())
        goto ProfStopSyntax;

    if(argc == 2)
    {
        if(DclStrNICmp(argv[1], "/REQ:", 5) == 0)
        {
            D_UINT32                ulHandle;
            DCLDECLAREREQUESTPACKET (PROFILER, RESET, reset);   /* DCLREQ_PROFILER_RESET */

            if(!ParseNumericValue(&argv[1][5], &ulHandle))
                goto ProfStopSyntax;

            dclStat = DclOsRequestorDispatch(ulHandle, &reset.ior);
            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = reset.dclStat;
        }
        else
        {
            DclPrintf("Syntax error in \"%s\"\n", argv[1]);
            return DCLSTAT_FAILURE;
        }
    }
    else
    {
        dclStat = DclProfReset();
    }

    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("The %s command failed, Status=%lX\n", argv[0], dclStat);

    return dclStat;

  ProfStopSyntax:

    DclPrintf("This command resets the profiler counters.\n\n");
    DclPrintf("Syntax:  %s [/Req:handle]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  /Req:handle - The optional Requestor Handle to which the command will be\n");
    DclPrintf("                sent.  The ReqHandle can be obtained using the 'ReqOpen'\n");
    DclPrintf("                command.  Not all environments support the requestor interface.\n\n");
    DclPrintf("If a handle is not specified, the command will apply locally (useful primarily\n");
    DclPrintf("in a monolithic environment).  Regardless whether the command is handled locally\n");
    DclPrintf("or remotely, it is only meaningful if the target code has the profiler built in.\n\n");

    return DCLSTAT_SHELLSYNTAXERROR;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_ProfStart(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSTATUS       dclStat;

    (void)hShell;
    
    if(argc > 2 || ISHELPREQUEST())
        goto ProfStartSyntax;

    if(argc == 2)
    {
        if(DclStrNICmp(argv[1], "/REQ:", 5) == 0)
        {
            D_UINT32                ulHandle;
            DCLREQ_PROFILER_ENABLE  req;

            if(!ParseNumericValue(&argv[1][5], &ulHandle))
                goto ProfStartSyntax;

            DclMemSet(&req, 0, sizeof(req));

            req.ior.ioFunc = DCLIOFUNC_PROFILER_ENABLE;
            req.ior.ulReqLen = sizeof(req);

            dclStat = DclOsRequestorDispatch(ulHandle, &req.ior);
            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = req.dclStat;
        }
        else
        {
            DclPrintf("Syntax error in \"%s\"\n", argv[1]);
            return DCLSTAT_FAILURE;
        }
    }
    else
    {
        dclStat = DclProfEnable();
    }

    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("The %s command failed, Status=%lX\n", argv[0], dclStat);

    return dclStat;

  ProfStartSyntax:

    DclPrintf("This command starts the profiler, optionally specifying a device to which the\n");
    DclPrintf("command should be directed.\n\n");
    DclPrintf("Syntax:  %s [/Req:handle]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  /Req:handle - The optional Requestor Handle to which the command will be\n");
    DclPrintf("                sent.  The ReqHandle can be obtained using the 'ReqOpen'\n");
    DclPrintf("                command.  Not all environments support the requestor interface.\n\n");
    DclPrintf("If a handle is not specified, the command will apply locally (useful primarily\n");
    DclPrintf("in a monolithic environment).  Regardless whether the command is handled locally\n");
    DclPrintf("or remotely, it is only meaningful if the target code has the profiler built in.\n\n");

    return DCLSTAT_SHELLSYNTAXERROR;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_ProfStop(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSTATUS       dclStat;

    (void)hShell;
    
    if(argc > 2 || ISHELPREQUEST())
        goto ProfStopSyntax;

    if(argc == 2)
    {
        if(DclStrNICmp(argv[1], "/REQ:", 5) == 0)
        {
            D_UINT32                ulHandle;
            DCLREQ_PROFILER_DISABLE req;

            if(!ParseNumericValue(&argv[1][5], &ulHandle))
                goto ProfStopSyntax;

            DclMemSet(&req, 0, sizeof(req));

            req.ior.ioFunc = DCLIOFUNC_PROFILER_DISABLE;
            req.ior.ulReqLen = sizeof(req);

            dclStat = DclOsRequestorDispatch(ulHandle, &req.ior);
            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = req.dclStat;
        }
        else
        {
            DclPrintf("Syntax error in \"%s\"\n", argv[1]);
            return DCLSTAT_FAILURE;
        }
    }
    else
    {
        dclStat = DclProfDisable();
    }

    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("The %s command failed, Status=%lX\n", argv[0], dclStat);

    return dclStat;

  ProfStopSyntax:

    DclPrintf("This command stops the profiler, optionally specifying a device to which the\n");
    DclPrintf("command should be directed.\n\n");
    DclPrintf("Syntax:  %s [/Req:handle]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  /Req:handle - The optional Requestor Handle to which the command will be\n");
    DclPrintf("                sent.  The ReqHandle can be obtained using the 'ReqOpen'\n");
    DclPrintf("                command.  Not all environments support the requestor interface.\n\n");
    DclPrintf("If a handle is not specified, the command will apply locally (useful primarily\n");
    DclPrintf("in a monolithic environment).  Regardless whether the command is handled locally\n");
    DclPrintf("or remotely, it is only meaningful if the target code has the profiler built in.\n\n");

    return DCLSTAT_SHELLSYNTAXERROR;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_ProfSummary(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSTATUS       dclStat;
    D_BOOL          fReset = FALSE;
    D_BOOL          fShort = FALSE;
    D_BOOL          fAdjust = TRUE;

    (void)hShell;
    
    if(argc > 5 || ISHELPREQUEST())
        goto ProfSummarySyntax;

    while(argc > 1)
    {
        if(DclStrICmp(argv[argc-1], "/RESET") == 0)
        {
            fReset = TRUE;
            argc--;
            continue;
        }

        if(DclStrICmp(argv[argc-1], "/SHORT") == 0)
        {
            fShort = TRUE;
            argc--;
            continue;
        }

        if(DclStrICmp(argv[argc-1], "/NOOA") == 0)
        {
            fAdjust = FALSE;
            argc--;
            continue;
        }

        if(argc > 2)
        {
            DclPrintf("Syntax error in \"%s\"\n", argv[argc-1]);
            return DCLSTAT_SHELLSYNTAXERROR;
        }
        else
        {
            break;
        }
    }

    if(argc == 2)
    {
        if(DclStrNICmp(argv[1], "/REQ:", 5) == 0)
        {
            D_UINT32                ulHandle;
            DCLDECLAREREQUESTPACKET (PROFILER, SUMMARY, req); /* DCLREQ_PROFILER_SUMMARY */

            if(!ParseNumericValue(&argv[1][5], &ulHandle))
                goto ProfSummarySyntax;

            req.fReset = fReset;
            req.fShort = fShort;
            req.fAdjustOverhead = fAdjust;

            dclStat = DclOsRequestorDispatch(ulHandle, &req.ior);
            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = req.dclStat;
        }
        else
        {
            DclPrintf("Syntax error in \"%s\"\n", argv[1]);
            return DCLSTAT_FAILURE;
        }
    }
    else
    {
        dclStat = DclProfSummary(fReset, fShort, fAdjust);
    }

    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("The %s command failed, Status=%lX\n", argv[0], dclStat);

    return dclStat;

  ProfSummarySyntax:

    DclPrintf("This command displays the profiler summary data, optionally resetting the data\n");
    DclPrintf("command should be directed.\n\n");
    DclPrintf("Syntax:  %s [/Req:handle] [/Reset] [/Short] [/NOOA\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  /Req:handle - The optional Requestor Handle to which the command will be\n");
    DclPrintf("                sent.  The ReqHandle can be obtained using the 'ReqOpen'\n");
    DclPrintf("                command.  Not all environments support the requestor interface.\n");
    DclPrintf("       /Reset - Reset the profiler statistics after they are displayed.  If this\n");
    DclPrintf("                is not specified, the profiler stats will continue to accumulate.\n");
    DclPrintf("       /Short - Display shortened statistics by ignoring meaningless records.\n");
    DclPrintf("        /NOOA - No Overhead Adjustment -- disable internal calibration.\n\n");
    DclPrintf("If a handle is not specified, the command will apply locally (useful primarily\n");
    DclPrintf("in a monolithic environment).  Regardless whether the command is handled locally\n");
    DclPrintf("or remotely, it is only meaningful if the target code has the profiler built in.\n\n");

    return DCLSTAT_SHELLSYNTAXERROR;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_ReqOpen(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    char           *pStr;
    DCLIODEVICE     nDevType;
    DCLOSREQHANDLE  hReq;
    DCLSTATUS       dclStat;

    (void)hShell;
    
    if(argc != 3 || ISHELPREQUEST())
        goto ReqOpenSyntax;

    pStr = argv[1];
    if(*pStr == '\'')
    {
        do
        {
            pStr++;
        } while(*pStr && *pStr != argv[1][0]);

        if(!(*pStr))
            goto ReqOpenSyntax;

        *pStr = 0;

        pStr = &argv[1][1];
    }

    if(DclStrICmp(argv[2], "BLOCK") == 0)
        nDevType = DCLIODEVICE_BLOCK;
    else if(DclStrICmp(argv[2], "FILESYS") == 0)
        nDevType = DCLIODEVICE_FILESYSTEM;
    else
        goto ReqOpenSyntax;

    dclStat = DclOsRequestorOpen(pStr, nDevType, &hReq);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Unable to open the requestor interface to \"%s\" on DeviceType %u, Status=%lX\n",
            pStr, nDevType, dclStat);
    }
    else
    {
        DCLDECLAREREQUESTPACKET (SYSTEM, INFO, Info);   /* DCLREQ_SYSTEM_INFO */
            
        DclPrintf("Opened the requestor interface to \"%s\" on DeviceType %u, Handle=%lX\n",
            pStr, nDevType, hReq);

        Info.SysInfo.nStrucLen = sizeof(Info.SysInfo);

        dclStat = DclOsRequestorDispatch(hReq, &Info.ior);
        if( (dclStat == DCLSTAT_SUCCESS) && 
            (Info.dclStat == DCLSTAT_SUCCESS) &&
            (Info.SysInfo.nStrucLen == sizeof(Info.SysInfo)) )
        {
            dclStat = DclSystemVersionDisplay(&Info.SysInfo);
            DclAssert(dclStat == DCLSTAT_SUCCESS);
        }
        else
        {
            DclPrintf("NOTE: The device which was opened does not appear to be managed\n");
            DclPrintf("      by Datalight Software, status=%lX\n", dclStat);
        }

        /*  Regardless what happened with the info request, return success
            if the original open request succeeded...
        */            
        dclStat = DCLSTAT_SUCCESS;
    }

    return dclStat;

  ReqOpenSyntax:

    DclPrintf("This command opens a requestor handle using the specified name and device type.\n\n");
    DclPrintf("Syntax:  %s 'Name' DevType\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("   Name   - Is the OS specific name of the device.  The name must be enclosed\n");
    DclPrintf("            in single or double quotes if it has spaces.\n");
    DclPrintf("  DevType - This is the device type string, which must be 'BLOCK' or 'FILESYS'\n");
    DclPrintf("            with no quotes.\n\n");
    DclPrintf("If successful, the requestor handle will be displayed as a hex string which may\n");
    DclPrintf("be passed to those commands which accept a requestor handle.\n\n");

    return DCLSTAT_SHELLSYNTAXERROR;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS exec_ReqClose(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    D_UINT32        ulHandle;
    DCLSTATUS       dclStat;

    (void)hShell;
    
    if(argc != 2 || ISHELPREQUEST())
        goto ReqCloseSyntax;

    if(!ParseNumericValue(argv[1], &ulHandle))
        goto ReqCloseSyntax;

    dclStat = DclOsRequestorClose(ulHandle);
    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("Unable to close the requestor interface, Status=%lX\n", dclStat);

    return dclStat;

  ReqCloseSyntax:

    DclPrintf("This command closes an open requestor handle.\n\n");
    DclPrintf("Syntax:  %s HandleNum\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  HandleNum - Is the requestor handle which was returned by the 'ReqOpen'\n");
    DclPrintf("              command.\n");

    return DCLSTAT_SHELLSYNTAXERROR;
}



                    /*------------------------------*\
                     *                              *
                     *      Helper Functions        *
                     *                              *
                    \*------------------------------*/


/*-------------------------------------------------------------------
    Local: GetCommand

    Gets a command in argv argc format from the console.

    Parameters:
        hShell     - The shell instance being operated on.
        pszCmdBuff - A buffer in which the input is stored.
        nMaxLen    - The buffer length
        argc       - The argument count
        argv       - The argument array

    Return:
        None
        argc will be -1 if console input is not supported.
        argc will be -2 if a request to terminate the process was
        made.
-------------------------------------------------------------------*/
static void GetCommand(
    DCLSHELLHANDLE  hShell,
    char           *pszCmdBuff,
    unsigned        nMaxLen,
    int            *argc,
    char          **argv)
{
    unsigned        uIndex;
    unsigned        uMsgLen;
    char           *pszCommand;

    DclAssertReadPtr(hShell, sizeof(*hShell));
    
    /*  Zero out the message buffer.
    */
    DclMemSet(pszCmdBuff, 0, nMaxLen);

    /*  Clear the argument list.
    */
    for(uIndex = 0; uIndex < DLS_MAX_ARGS; uIndex++)
    {
        *argv = NULL;
    }

    /*  Get a message from the console.
    */
    uMsgLen = DclShellConsoleInput(hShell, pszCmdBuff, nMaxLen);
    if(uMsgLen == UINT_MAX)
    {
        *argc = -1;
        return;
    }
    else if(uMsgLen == UINT_MAX-1)
    {
        *argc = -2;
        return;
    }

    /*  Build argv and argc from the message.
    */
    *argc = 0;
    pszCommand = pszCmdBuff;
    for(uIndex = 0; uIndex < DLS_MAX_ARGS; uIndex++)
    {
        /*  Skip any preceeding whitespace
        */
        while((*pszCommand == ' ' || *pszCommand == '\t') &&
              (pszCommand < &pszCmdBuff[uMsgLen]))
        {
            pszCommand++;
        }

        /*  Check if we are at the end of the command.
        */
        if((*pszCommand == '\n') || (*pszCommand == '\r') ||
           (pszCommand == &pszCmdBuff[uMsgLen]))
        {
            break;
        }

        /*  This is the start of a new argument.
        */
        *argc = *argc+1;
        argv[uIndex] = pszCommand;

        if(*pszCommand == '"')
        {
            /*  Skip past the leading quote
            */
            argv[uIndex]++;
            pszCommand++;

            /*  Skip to the ending quote, or the end of the input
            */
            while((*pszCommand != '"') && (*pszCommand != '\n') && (*pszCommand != '\r') &&
                (pszCommand < &pszCmdBuff[uMsgLen]))
            {
                pszCommand++;
            }
        }
        else
        {
            /*  Skip to the end of this argument.

                ToDo: Need to consider whether an arument with embedded
                      quotes should be specially handled.
            */
            while((*pszCommand != ' ') && (*pszCommand != '\t') &&
                  (*pszCommand != '\n') && (*pszCommand != '\r') &&
                  (pszCommand < &pszCmdBuff[uMsgLen]))
            {
                pszCommand++;
            }
        }

        /*  Terminate this argument.
        */
        *pszCommand = 0;

        /*   Check if more arguments.
        */
        if(pszCommand < &pszCmdBuff[uMsgLen])
        {
            /*  On to the next argument.
            */
            pszCommand++;
        }
        else
        {
            /*  This is the end of the command.
            */
            break;
        }
    }
}


/*-------------------------------------------------------------------
    Local: DisplayPrompt

    Display the command shell prompt.

    Parameters:

        hShell      - The handle of the shell instance to run.

    Return:
        None.
 -------------------------------------------------------------------*/
static void DisplayPrompt(
    DCLSHELLHANDLE  hShell)
{
    char            szPrompt[DLS_PATHLEN + 1];
    DCLSTATUS       DclStatus;
    char           *pTmp = &szPrompt[0];
    const char     *pszPrompt;
    unsigned        nMaxLength;

    DclAssertReadPtr(hShell, sizeof(*hShell));

    if(hShell->fTimeStamp)
    {
        D_TIME      ullTime;

        DclOsGetDateTime(&ullTime);

        /*  For environments where the Date/Time OS Service is not
            implemented, zero will be returned.  In this case, simply
            use the elapsed time since system startup (or whenever the
            timer last rolled over).
        */                
        if(!ullTime)
            ullTime = DclTimePassedUS(0);
        
        DclDateTimeString(&ullTime, pTmp, DLS_PATHLEN);

        DclMemCpy(pTmp, &szPrompt[7], 9);

        *(pTmp+9) = 0;

        pTmp += 9;
    }

    /*  Ensure null termination
    */
    *pTmp = 0;

    /*  If there is a prompt variable in the environment, use that
        value as the prompt.
    */
    pszPrompt = DclEnvVarLookup(hShell->params.hEnv, "PROMPT");
    if(pszPrompt)
    {
        DclPrintf("%s%s", szPrompt, pszPrompt);
    }
    else
    {
        /*  Get the current dir.
        */
        if(hShell->fCurDirSupport)
        {
            nMaxLength = DCLDIMENSIONOF(szPrompt) - DclStrLen(szPrompt);
            DclStatus = DclFsDirGetWorking(pTmp, nMaxLength);
            if(DclStatus)
                *pTmp = 0;
        }
        else
        {
            *pTmp = 0;
        }

        if(hShell->pszTitle)
            DclPrintf("%s-%s>", hShell->pszTitle, szPrompt);
        else
            DclPrintf("%s>", szPrompt);
    }

    return;
}


/*-------------------------------------------------------------------
    Local: ParseNumericValue

    This helper function parses the specified ASCII string for
    a numeric value which may be in hex or decimal format.  Hex
    numbers must be prefixed with 0x.

    This function will display a syntax error message if the
    parse fails.

    Parameters:
        pszNum  - A pointer to the ASCII number to parse
        pulNum  - The buffer in which to store the result

    Return:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL ParseNumericValue(
    const char *pszNum,
    D_UINT32   *pulNum)
{
    const char *pStr;

    pStr = DclNtoUL(pszNum, pulNum);

    /*  If the function failed, or the number is NOT followed immediately
        by a NULL, then report the error.
    */
    if(!pStr || *pStr != 0)
    {
        DclPrintf("Syntax error, \"%s\" not recognized as a numeric value\n", pszNum);
        return FALSE;
    }

    return TRUE;
}




#endif

