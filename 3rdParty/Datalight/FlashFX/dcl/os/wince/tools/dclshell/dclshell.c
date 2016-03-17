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

    This module is an application framework for creating standalone
    executables for this given OS.

    See the comments included in the respective tests/tools code for
    the syntax for this utility.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dclshell.c $
    Revision 1.9  2009/06/27 22:41:49Z  garyp
    Updated to create a DCL Instance for use by the program.
    Revision 1.8  2008/06/03 21:43:10Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.7  2007/12/14 18:24:47Z  Garyp
    Modified to use the new generalized CE output redirection functions.
    Renamed the executable file name to "dlshell.exe".
    Revision 1.6  2007/11/03 23:31:37Z  Garyp
    Added the standard module header.
    Revision 1.5  2007/10/16 20:04:41Z  Garyp
    Fixed to use a proper printf() format string.  Minor type changes to avoid
    compiler warnings.
    Revision 1.4  2007/08/28 19:49:40Z  Garyp
    Updated to initialize the profiler and add the file system commands
    explicitly, since those are no longer done automatically when the
    generic shell is created.
    Revision 1.3  2007/08/19 05:19:03Z  garyp
    Corrected an invalid errorlevel value.
    Revision 1.2  2007/08/06 00:43:48Z  garyp
    Modified to always return a valid errorlevel value as defined in dlerrlev.h.
    Revision 1.1  2007/06/26 01:39:08Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>
#include <dltools.h>
#include <dlerrlev.h>
#include <dlceutil.h>
#include <dlcetools.h>

/*  The file is named "dclshell" but the EXE is "dlshell.exe"
*/
#define PROGRAM_NAME    "DLSHELL"


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
int WINAPI WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPTSTR      ptzCmdLine,
    int         nCmdShow)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)nCmdShow;

  #if DCLCONF_COMMAND_SHELL
    {
        DCLSTATUS               dclStat;
        DCLINSTANCEHANDLE       hDclInst;

        dclStat = DclInstanceCreate(0, DCLFLAG_APP, &hDclInst);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            #define                 BUFFLEN     (256)
            char                    szCmdLine[BUFFLEN];
            DCLREDIRECTIONHANDLE    hRedirect;
            DCLTOOLPARAMS           tp = {0};

            tp.hDclInst     = hDclInst;
            tp.pszCmdName   = PROGRAM_NAME;
            tp.pszCmdLine   = szCmdLine;

            /*  Convert the string from Unicode to ANSI
            */
            WideCharToMultiByte(CP_ACP, 0, ptzCmdLine, -1, szCmdLine, BUFFLEN, NULL, NULL);

            hRedirect = DclCeOutputRedirectionBegin(hDclInst, szCmdLine);

            dclStat = DclCeShell(&tp);

            if(hRedirect)
                DclCeOutputRedirectionEnd(hRedirect);

            if(DclInstanceDestroy(hDclInst) != DCLSTAT_SUCCESS)
                printf(PROGRAM_NAME": Instance destruction failed\n");
        }
        else
        {
            printf(PROGRAM_NAME": Instance creation failed with status code 0x%08lX\n", dclStat);
        }

        return DclStatusToErrorlevel(dclStat);
    }

  #else

    (void)ptzCmdLine;

    printf(PROGRAM_NAME": The Datalight Command Shell functionality is not enabled\n");

    return DCLERRORLEVEL_FEATUREDISABLED;

  #endif
}


