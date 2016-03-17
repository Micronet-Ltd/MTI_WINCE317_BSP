/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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

    This module implements "Win32" OS specific shell commands.

    This module implements generic Win32 shell commands.  This code must be
    fully compilable and runnable under generic Win32 user mode, as well as
    Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: winshellcmd.c $
    Revision 1.6  2011/05/28 02:41:53Z  garyp
    Corrected a diagnostics message.
    Revision 1.5  2010/11/13 02:39:28Z  garyp
    Fixed some issues with the previous rev.
    Revision 1.4  2010/11/09 13:25:22Z  garyp
    Enhanced the "OSRUN" command to include an option to kill a process.
    Revision 1.3  2010/11/01 04:11:19Z  garyp
    Added the "OSRUN" command.
    Revision 1.2  2009/02/17 06:35:13Z  keithg
    Added explicit void of unused formal parameters.
    Revision 1.1  2008/11/05 02:16:20Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>

#if DCLCONF_COMMAND_SHELL

#include <dlshell.h>
#include <dltools.h>
#include <dlerrlev.h>
#include <dlwinutil.h>

#define ISHELPREQUEST() (argc > 1 && (!DclStrCmp(argv[1], "?") || !DclStrCmp(argv[1], "/?")))

static DCLSTATUS Comm(DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS Run( DCLSHELLHANDLE hShell, int argc, char **argv);

static const DCLSHELLCOMMAND aWinCmds[] =
{
    {"OS", "%sComm", "Get serial port parameters", Comm},
    {"OS", "%sRun",  "Run a program", Run}
};



/*-------------------------------------------------------------------
    Public: DclWinShellAddCommands()

    Add "win32" specific commands to the shell.

    Parameters:
        hShell       - The shell handle.
        pszClassName - A pointer to the null-terminated command
                       class name to use.
        pszPrefix    - A pointer to the null-terminated command
                       prefix to use.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclWinShellAddCommands(
    DCLSHELLHANDLE  hShell,
    const char     *pszClassName,
    const char     *pszPrefix)
{
    DCLSTATUS       dclStat;

    if(!hShell)
    {
        DclError();
        return DCLSTAT_BADPARAMETER;
    }

    DclAssert(pszClassName);
    DclAssert(pszPrefix);

    dclStat = DclShellAddMultipleCommands(hShell, aWinCmds, DCLDIMENSIONOF(aWinCmds), pszClassName, pszPrefix);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        /*  For "win32", the file system commands should always be usable.
        */
        dclStat = DclShellAddFileSystemCommands(hShell);
    }

    return dclStat;
}



                    /*------------------------------*\
                     *                              *
                     *  Win Specific Shell Commands *
                     *                              *
                    \*------------------------------*/



/*-------------------------------------------------------------------
    ToDo: Update this command to be able to initialize the serial
          port parameters.  The problem is that the params only
          persist if the handle is held open, and we don't have a
          clean way to accomplish that at this time.
-------------------------------------------------------------------*/
 static DCLSTATUS Comm(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    int             ii;
    HANDLE          hComm = INVALID_HANDLE_VALUE;
    DCB             dcb;
    DCLSTATUS       dclStat;
/*    D_UINT32        ulBaud;
    unsigned        fSetState = FALSE;
*/
    (void)hShell;

    if(argc <= 1)
        goto DisplayHelp;

    for(ii=1; ii<argc; ii++)
    {
        if((DclStrICmp(argv[ii], "?") == 0) || (DclStrICmp(argv[ii], "/?") == 0))
            goto DisplayHelp;

        if(ii == 1)
        {
            #define MAX_COMM_NAME   (16)
            D_WCHAR tzCommName[MAX_COMM_NAME];

            DclOsAnsiToWcs(tzCommName, DCLDIMENSIONOF(tzCommName), argv[ii], -1);

            hComm = DclWinCommOpen(tzCommName);
            if(hComm == INVALID_HANDLE_VALUE)
            {
                DclPrintf("Unable to open port \"%W\"\n", tzCommName);
                dclStat = DCLSTAT_FAILURE;
                goto Cleanup;
            }

            if(!GetCommState(hComm, &dcb))
            {
                DclPrintf("Error %lU getting COMM state\n", GetLastError());
                dclStat = DCLSTAT_FAILURE;
                goto Cleanup;
            }

            DclWinCommParamDump(&dcb, D_DEBUG);

            continue;
        }
/*
        if(DclStrNICmp(argv[ii], "/Baud:", 6) == 0)
        {
            const char *pStr;

            pStr = DclNtoUL(&argv[ii][6], &ulBaud);
            if(!pStr || *pStr)
            {
                DclPrintf("Bad syntax in \"%s\"\n", argv[ii]);
                dclStat = DCLSTAT_FAILURE;
                goto Cleanup;
            }

            fSetState = TRUE;
        }
        else
*/
        {
            DclPrintf("Unrecognized command \"%s\"\n", argv[ii]);
            dclStat = DCLSTAT_FAILURE;
            goto Cleanup;
        }
    }
/*
    if(fSetState)
    {
        if(!SetCommState(hComm, &dcb))
        {
            DclPrintf("Error %lU setting COMM state\n", GetLastError());
            dclStat = DCLSTAT_FAILURE;
            goto Cleanup;
        }

        return DCLSTAT_SUCCESS;
    }
*/
    dclStat = DCLSTAT_SUCCESS;
    goto Cleanup;

  DisplayHelp:
    DclPrintf("This command gets serial port parameters.\n\n");
    DclPrintf("Syntax:  %s COMn:\n\n", argv[0]);
/*  DclPrintf("This command gets and sets serial port parameters.\n\n");
    DclPrintf("Syntax:  %s COMn: [Options]\n\n", argv[0]);
    DclPrintf("Where 'Options' are zero or more of the following:\n");
    DclPrintf("       /Baud:n  Set the baud rate, where 'n' is the exact baud rate, from 110 to\n");
    DclPrintf("                to 256000.  No abbreviations are allowed\n");
    DclPrintf("\nIf no options are specified, the configuration information for the specified\n");
    DclPrintf("port will be displayed, and not modified.\n\n");
*/
    dclStat = DCLSTAT_HELPREQUEST;

  Cleanup:
    if(hComm != INVALID_HANDLE_VALUE)
        DclWinCommClose(hComm);

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
 static DCLSTATUS Run(
    DCLSHELLHANDLE      hShell,
    int                 argc,
    char              **argv)
{
    DCLSTATUS           dclStat;

    if((argc <= 1) || ISHELPREQUEST())
        goto DisplayHelp;

    if(DclStrNICmp(argv[1], "/KILL:", 6) != 0)
    {
        const char          szVar[] = {"OSRUN_HANDLE"};
        int                 ii;
        char                szCommandLine[256];
        D_WCHAR             wzCommandLine[256];
        D_WCHAR             wzProgram[256];
        SHELLEXECUTEINFO    ExecInfo = {sizeof(SHELLEXECUTEINFO)};
        DCLSHELLPARAMS      Params;

        DclShellParams(hShell, &Params);

        szCommandLine[0] = 0;
        for(ii=2; ii<argc; ii++)
        {
            DclStrCat(szCommandLine, " ");
            DclStrCat(szCommandLine, argv[ii]);
        }

        if(!DclOsAnsiToWcs(wzProgram, DCLDIMENSIONOF(wzProgram), argv[1], -1))
            return DCLSTAT_BUFFERTOOSMALL;
        
        if(!DclOsAnsiToWcs(wzCommandLine, DCLDIMENSIONOF(wzCommandLine), szCommandLine, -1))
            return DCLSTAT_BUFFERTOOSMALL;

    	/*  Open the specified file using the default associations.
    	*/
    	ExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        ExecInfo.lpVerb = TEXT("open");
        ExecInfo.lpFile = wzProgram;
        ExecInfo.lpParameters = wzCommandLine;
        ExecInfo.nShow = SW_SHOWNORMAL;

        /*  It is possible for ShellExecuteEx() to succeed but still return
            a NULL hProcess value (see the docs).  Remove any pre-existing
            environment variable at this point, to satisfy that condition,
            and the condition where the process creation really does fail.
        */                    
        DclEnvVarDelete(Params.hEnv, szVar);
        
    	if(!ShellExecuteEx(&ExecInfo))
        {
            DclPrintf("ShellExecuteEx(%W) failed with error %lU\n", wzProgram, GetLastError());
            return DCLSTAT_FAILURE;
        }

        if(ExecInfo.hProcess)
        {
            char    szModule[32];

            DclSNPrintf(szModule, sizeof(szModule), "%P", ExecInfo.hProcess);

            DclEnvVarAdd(Params.hEnv, szVar, szModule);
        }

        DclPrintf("Successfully created process with handle %P\n", ExecInfo.hProcess);

        return DCLSTAT_SUCCESS;
    }
    else
    {
        const char *pStr;
        HANDLE      hProcess;

        DclAssert(sizeof(hProcess) == sizeof(D_UINT32*));
        
        pStr = DclNtoUL(&argv[1][6], (D_UINT32*)&hProcess);
        if(!pStr || *pStr)
        {
            DclPrintf("Syntax error in \"%s\"\n", argv[1]);
            return DCLSTAT_BADSYNTAX;
        }

        if(!TerminateProcess(hProcess, 1))
        {
            DclPrintf("TerminateProcess(%P) failed with error %lU\n", hProcess, GetLastError());
            return DCLSTAT_FAILURE;
        }

        return DCLSTAT_SUCCESS;
    }

  DisplayHelp:
    DclPrintf("This command starts a new process, or kills an existing process.\n\n");
    DclPrintf("Syntax:  %s  {ProgramName Arguments...  OR  /KILL:hProcess}\n\n", argv[0]);
    DclPrintf("When starting a new process, the process handle (hProcess) will be\n");
    DclPrintf("returned in the environment variable \"OSRUN_HANDLE\".  This value may\n");
    DclPrintf("may be used to kill the process.\n\n");
    DclPrintf("The '/KILL:hProcess' option may be used to forcibly terminate a running\n");
    DclPrintf("process.  This should be used with caution, as some environments may not\n");
    DclPrintf("properly clean things up.\n\n");
        
    dclStat = DCLSTAT_HELPREQUEST;

    return dclStat;
}


#endif

