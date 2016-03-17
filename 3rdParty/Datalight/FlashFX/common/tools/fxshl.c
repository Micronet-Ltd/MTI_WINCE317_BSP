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
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

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

    This module creates a FlashFX command shell using the DCL shell
    functionality.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxshl.c $
    Revision 1.28  2012/03/01 20:58:02Z  johnb
    Added /Run command line option to run a single command and 
    exit.
    Revision 1.27  2010/12/09 23:41:42Z  garyp
    Added the /DEBUG command-line option.
    Revision 1.26  2010/11/01 13:48:33Z  garyp
    Updated to the new DclLogOpen() calling convention.
    Revision 1.25  2009/10/09 01:19:12Z  garyp
    Updated so the wrapper code always builds regardless whether the SHELL is
    enabled or not, to allow OS specific code to link properly, since they may
    not have access to the DCLCONF_SHELL_ENABLED setting.
    Revision 1.24  2009/07/21 21:33:17Z  garyp
    Merged from the v4.0 branch.  Updated to use the revised FfxSignOn()
    function, which now takes an fQuiet parameter.  Documentation updated.
    Revision 1.23  2009/04/01 14:58:08Z  davidh
    Function Headers Updated for AutoDoc.
    Revision 1.22  2009/02/09 01:38:07Z  garyp
    Merged from the v4.0 branch.  Updated to use the new FFXTOOLPARAMS structure.
    Revision 1.21  2008/04/07 22:55:29Z  brandont
    Updated to use the new DCL file system services.
    Revision 1.20  2008/04/03 23:30:42Z  brandont
    Updated all defines and structures used by the DCL file system
    services to use the DCLFS prefix.
    Revision 1.19  2008/02/26 17:09:38Z  garyp
    Added the /E (echo) command-line parameter.
    Revision 1.18  2008/01/13 07:27:03Z  keithg
    Function header updates to support autodoc.
    Revision 1.17  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.16  2007/08/28 19:17:10Z  Garyp
    Modified to use the new dlshell.h header and shell creation function.
    Revision 1.15  2007/06/25 19:11:14Z  Garyp
    Updated to allow environment variables to be specified with quotes or
    ticks, so that embedded spaces may be used.
    Revision 1.14  2007/05/15 01:34:34Z  garyp
    Updated to create an environment and pass any variables specified on
    the command-line, into the shell.
    Revision 1.13  2007/05/07 17:33:02Z  keithg
    Changed the logging buffer size from 4MB to 4KB.
    Revision 1.12  2007/04/27 20:00:45Z  Garyp
    Updated to handle a null command-line pointer.
    Revision 1.11  2007/04/06 18:53:37Z  Garyp
    Added support for the /SCRIPT and /LOG options.
    Revision 1.10  2007/02/08 20:52:01Z  Garyp
    Factored the commands out into the module fxshlcmd.c.
    Revision 1.9  2007/01/12 02:02:04Z  Garyp
    Minor type changes -- nothing functional.
    Revision 1.8  2007/01/10 01:59:18Z  Garyp
    Updated for new parameters to DclShellCreate().
    Revision 1.7  2006/11/30 02:01:01Z  Garyp
    Modified to use the new shell functionality which include a command class.
    Revision 1.6  2006/10/07 03:35:22Z  Garyp
    Corrected to eliminate warnings generated by the RealView tools.
    Revision 1.5  2006/08/21 21:50:41Z  Garyp
    Removed FlashFXReclaim, as this is handled by the FlashFXCompact
    command.  Minor help text changes.
    Revision 1.4  2006/08/01 16:30:22Z  Pauli
    Added FfxReclaim.  Changed strcpy to DclStrCpy.
    Revision 1.3  2006/06/19 17:44:48Z  johnb
    Removed unused variable.
    Revision 1.2  2006/06/16 00:02:03Z  Pauli
    Updated to work with changes made to the DCL Command Shell.
    Revision 1.1  2006/05/29 01:46:42Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxtools.h>
#include <dlshell.h>
#include <dllog.h>

#define MAX_RUN_LENGTH 256

/*-------------------------------------------------------------------
    Protected: FfxShell()

    This function invokes the FlashFX shell.

    Parameters:
        pTP - A pointer to the FFXTOOLPARAMS structure to use.

    Return Value:
        Returns zero if successful, otherwise non-zero
-------------------------------------------------------------------*/
D_INT16 FfxShell(
    FFXTOOLPARAMS  *pTP)
{
  #if DCLCONF_COMMAND_SHELL
    #define         ARGBUFFLEN      (256)
    #define         LOGBUFFERSIZE   (4)
    DCLSHELLHANDLE  hShell;
    DCLENVHANDLE    hEnv = NULL;
    DCLLOGHANDLE    hLog = NULL;
    DCLFSFILEHANDLE hScript = NULL;
    DCLSTATUS       dclStat = DCLSTAT_FAILURE;
    char            achArgBuff[ARGBUFFLEN];
    D_UINT16        uArgNum = 1;
    char           *pszScript = NULL;
    D_UINT16        uArgCount;
    D_BOOL          fEcho = FALSE;
    DCLASSERTMODE   nOldAssertMode = DCLASSERTMODE_INVALID;

    DclPrintf("\nFlashFX Command Shell\n");
    FfxSignOn(FALSE);
    DclPrintf("\n");

    uArgCount = DclArgCount(pTP->dtp.pszCmdLine);

    while(uArgCount && DclArgRetrieve(pTP->dtp.pszCmdLine, uArgNum, ARGBUFFLEN, achArgBuff))
     {
        if((DclStrICmp(achArgBuff, "?") == 0) || (DclStrICmp(achArgBuff, "/?") == 0))
        {
            DclPrintf("This command invokes a FlashFX command shell\n\n");
            DclPrintf("Syntax:  %s [options]\n\n", pTP->dtp.pszCmdName);
            DclPrintf("Where [options] are:\n");
            DclPrintf("            /E  Causes input to be echoed to the console\n");
            DclPrintf("  /Run:command  Run the specified command immediately upon startup, and exit\n");
            DclPrintf("                once the command has completed.  Enclose \"command\" in quotes\n");
            DclPrintf("                if it contains spaces.  Note that if the command being run is\n");
            DclPrintf("                the shell's RUN command to execute a script, that script must\n");
            DclPrintf("                have its own EXIT command if you wish to exit upon completion.\n");
            DclPrintf("  /Script:file  Specifies the name of a script file containing shell commands,\n");
            DclPrintf("                one per line.\n");
            DclPrintf("        /Debug  Invoke the debugger when a failure occurs (where supported).\n");
            DclPrintf("     /Log:file  Specifies the name of a file to which messages are to be\n");
            DclPrintf("                logged.\n");
            DclPrintf("  /Env:Var=Val  Specifies the name of an environment variable and value to add\n");
            DclPrintf("                to the environment.  This option may be specified multiple\n");
            DclPrintf("                times, as necessary.\n");

            if(pTP->dtp.pszExtraHelp)
                DclPrintf(pTP->dtp.pszExtraHelp);

            goto Cleanup;
        }

        if(DclStrICmp(achArgBuff, "/E") == 0)
        {
            fEcho = TRUE;

            uArgNum++;

            continue;
        }

        if(DclStrNICmp(achArgBuff, "/Script:", 8) == 0)
        {
            long            lLen;
            size_t          nRead;

            if (pszScript)
            {
                DclPrintf("Warning: Script specified by Run option -- ignoring Script option.\n");
                continue;
            }

            dclStat = DclFsFileOpen(&achArgBuff[8], "rb", &hScript);
            if(dclStat)
            {
                DclPrintf("Error opening script file \"%s\"\n", &achArgBuff[8]);
                goto Cleanup;
            }

            DclOsFileSeek(hScript, 0, DCLFSFILESEEK_END);
            lLen = DclOsFileTell(hScript);
            pszScript = DclMemAlloc(lLen+2);
            if(!pszScript)
            {
                DclPrintf("Unable to allocate memory\n");
                goto Cleanup;
            }

            DclOsFileSeek(hScript, 0, DCLFSFILESEEK_SET);
            nRead = DclOsFileRead(pszScript, 1, lLen, hScript);
            if(nRead != (size_t)lLen)
            {
                DclPrintf("Error reading script file\n");
                goto Cleanup;
            }

            /*  Ensure that the script is properly terminated
            */
            pszScript[lLen+0] = '\n';
            pszScript[lLen+1] = 0;

            DclFsFileClose(hScript);
            hScript = NULL;

            uArgNum++;

            continue;
        }

        if(DclStrNICmp(achArgBuff, "/Run:", 5) == 0)
        {
            char *pTmp;
            
            if (pszScript)
            {
                DclPrintf("Warning: Script specified by file -- ignoring Run option.\n");
                continue;
            }

            /*  Build a pseudo script, with the command to run, followed
                by an "exit" command.
            */
            pTmp = &achArgBuff[5];

            if (DclStrLen(pTmp) >= MAX_RUN_LENGTH)
            {
                DclPrintf("Run command too large.  Maximum length is %d\n", MAX_RUN_LENGTH);
                goto Cleanup;
            }
                
            pszScript = DclMemAlloc(MAX_RUN_LENGTH);
            if(!pszScript)
            {
                DclPrintf("Unable to allocate memory\n");
                goto Cleanup;
            }

            /*  remove quotes around the script command
            */
            if (*pTmp == '"')
                pTmp++;
            
            DclStrCpy(pszScript, pTmp);
            
            if (pszScript[DclStrLen(pTmp) - 1] == '"')
                pszScript[DclStrLen(pTmp) - 1] = 0;
            
            DclStrCat(pszScript, "\r\nExit\r\n");
            
            DclAssert(DclStrLen(pszScript) < MAX_RUN_LENGTH);

            uArgNum++;

            continue;
        }

        if(DclStrICmp(achArgBuff, "/Debug") == 0)
        {
            nOldAssertMode = DclAssertMode(pTP->dtp.hDclInst, DCLASSERTMODE_DEBUGGERFAIL);

            uArgNum++;

            continue;
        }

        if(DclStrNICmp(achArgBuff, "/Log:", 5) == 0)
        {
            hLog = DclLogOpen(pTP->dtp.hDclInst, &achArgBuff[5], LOGBUFFERSIZE, DCLLOGFLAGS_SHADOW);
            if(!hLog)
            {
                DclPrintf("Error opening log file \"%s\"\n", &achArgBuff[5]);
                goto Cleanup;
            }

            uArgNum++;

            continue;
        }

        if(DclStrNICmp(achArgBuff, "/Env:", 5) == 0)
        {
            char   *pEqual;

            if(!hEnv)
            {
                hEnv = DclEnvCreate(0, NULL);
                if(!hEnv)
                {
                    DclPrintf("Error creating environment\n");
                    goto Cleanup;
                }
            }

            pEqual = DclStrChr(&achArgBuff[5], '=');
            if(!pEqual || pEqual == &achArgBuff[5] || *(pEqual+1) == 0)
            {
                DclPrintf("Syntax error in \"%s\"\n", achArgBuff);
                goto Cleanup;
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

            if(DclEnvVarAdd(hEnv, &achArgBuff[5], pEqual) != DCLSTAT_SUCCESS)
            {
                DclPrintf("Error adding the environment string \"%s=%s\"\n", achArgBuff[5], pEqual);
                goto Cleanup;
            }

            uArgNum++;

            continue;
        }
        DclPrintf("%s: Unknown option '%s'\n", pTP->dtp.pszCmdName, achArgBuff);

        goto Cleanup;
    }

    hShell = DclShellCreate(pTP->dtp.hDclInst, "FFX", pszScript, hEnv, TRUE, fEcho, NULL);
    if(hShell)
    {
        dclStat = FfxShellAddCommands(hShell, "FlashFX", "FX");
        if(dclStat == FFXSTAT_SUCCESS)
        {
            dclStat = DclShellRun(hShell);
        }

        DclShellDestroy(hShell);
    }

  Cleanup:
    if(pszScript)
        DclMemFree(pszScript);

    if(hScript)
        DclFsFileClose(hScript);

    if(hLog)
        DclLogClose(pTP->dtp.hDclInst, hLog);

    if(hEnv)
        DclEnvDestroy(hEnv);

    if(nOldAssertMode != DCLASSERTMODE_INVALID)
            DclAssertMode(pTP->dtp.hDclInst, nOldAssertMode);
                
    if(dclStat == DCLSTAT_SUCCESS)
        return 0;
    else
        return __LINE__;

  #else

    DclPrintf("The %s functionality is not enabled\n\n", pTP->dtp.pszCmdName);
    return -1;

  #endif
}



