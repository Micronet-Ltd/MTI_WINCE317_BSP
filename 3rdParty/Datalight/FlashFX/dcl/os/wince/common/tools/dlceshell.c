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

    This module implements the general DCL Command Shell for Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlceshell.c $
    Revision 1.7  2010/11/01 03:55:13Z  garyp
    Modified to use the new DclLogOpen() function which now takes a
    flags parameter rather than a whole bunch of bools.
    Revision 1.6  2009/11/08 03:51:12Z  garyp
    Eliiminated explicit profiler creation and destruction, since it now
    initializes as a service.
    Revision 1.5  2009/06/27 23:00:01Z  garyp
    Updated for function calls which now take a DCL instance handle.  Updated
    to use the DCLTOOLPARAMS structure.  Improved error handling.
    Revision 1.4  2009/02/08 02:34:37Z  garyp
    Updated shell creation parameters.
    Revision 1.3  2008/04/07 22:54:15Z  brandont
    Updated to use the new file system services.
    Revision 1.2  2008/04/03 23:55:56Z  brandont
    Updated all defines and structures used by the DCL file system
    services to use the DCLFS prefix.
    Revision 1.1  2007/12/14 02:12:20Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>

#if DCLCONF_COMMAND_SHELL

#include <dltools.h>
#include <dlerrlev.h>
#include <dlshell.h>
#include <dlcetools.h>
#include <dllog.h>

#define BUFFLEN     (256)

/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
DCLSTATUS DclCeShell(
    DCLTOOLPARAMS  *pTP)
{
    #define         LOGBUFFERSIZEKB (4)
    DCLSHELLHANDLE  hShell;
    DCLENVHANDLE    hEnv = NULL;
    DCLLOGHANDLE    hLog = NULL;
    DCLFSFILEHANDLE hScript = NULL;
    DCLSTATUS       dclStat;
    char           *pszScript = NULL;
    char            achBuffer[BUFFLEN];
    D_UINT16        uArgNum;
    D_UINT16        uArgCount;

    uArgCount = DclArgCount(pTP->pszCmdLine);

    for(uArgNum=1; uArgNum<=uArgCount; uArgNum++)
    {
        if(!DclArgRetrieve(pTP->pszCmdLine, uArgNum, sizeof(achBuffer), achBuffer))
        {
            DclProductionError();
            dclStat = DCLSTAT_FAILURE;
            goto Cleanup;
        }

        if((DclStrICmp(achBuffer, "?") == 0) || (DclStrICmp(achBuffer, "/?") == 0))
        {
            DclPrintf("This command invokes a DCL command shell\n\n");
            DclPrintf("Syntax:  %s [/Script:file] [/Log:file] [/Env:Var=Value]\n\n", pTP->pszCmdName);
            DclPrintf("Where:\n");
            DclPrintf("  /Script:file  Specifies the name of a script file containing shell commands,\n");
            DclPrintf("                one per line.\n");
            DclPrintf("     /Log:file  Specifies the name of a file to which messages are to be\n");
            DclPrintf("                logged.\n");
            DclPrintf("  /Env:Var=Val  Specifies the name of an environment variable and value to add\n");
            DclPrintf("                to the environment.  This option may be specified multiple\n");
            DclPrintf("                times, as necessary.\n");

            dclStat = DCLSTAT_HELPREQUEST;
            goto Cleanup;
        }

        if(DclStrNICmp(achBuffer, "/Script:", 8) == 0)
        {
            long            lLen;
            size_t          nRead;

        	dclStat = DclFsFileOpen(&achBuffer[8], "rb", &hScript);
            if(dclStat)
            {
                DclPrintf("Error opening script file \"%s\"\n", &achBuffer[8]);
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

            continue;
        }

        if(DclStrNICmp(achBuffer, "/Log:", 5) == 0)
        {
            hLog = DclLogOpen(NULL, &achBuffer[5], LOGBUFFERSIZEKB, DCLLOGFLAGS_SHADOW);
            if(!hLog)
            {
                DclPrintf("Error opening log file \"%s\"\n", &achBuffer[5]);
                goto Cleanup;
            }

            continue;
        }

        if(DclStrNICmp(achBuffer, "/Env:", 5) == 0)
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

            pEqual = DclStrChr(&achBuffer[5], '=');
            if(!pEqual || pEqual == &achBuffer[5] || *(pEqual+1) == 0)
            {
                DclPrintf("Syntax error in \"%s\"\n", achBuffer);
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

            if(DclEnvVarAdd(hEnv, &achBuffer[5], pEqual) != DCLSTAT_SUCCESS)
            {
                DclPrintf("Error adding the environment string \"%s=%s\"\n", achBuffer[5], pEqual);
                goto Cleanup;
            }

            continue;
        }

        DclPrintf("Unknown option '%s'\n", achBuffer);

        goto Cleanup;
    }

    hShell = DclShellCreate(pTP->hDclInst, "DCL", pszScript, hEnv, FALSE, FALSE, NULL);
    if(hShell)
    {
        dclStat = DclShellAddOsCommands(hShell, "WinCE", "OS");
        if(dclStat == DCLSTAT_SUCCESS)
            dclStat = DclShellRun(hShell);

        DclShellDestroy(hShell);
    }
    else
    {
        dclStat = DCLSTAT_SHELLCREATEFAILED;
    }

  Cleanup:
    if(pszScript)
        DclMemFree(pszScript);

    if(hScript)
        DclFsFileClose(hScript);

    if(hLog)
        DclLogClose(pTP->hDclInst, hLog);

    if(hEnv)
        DclEnvDestroy(hEnv);

    return dclStat;
}


#endif

