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

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: windlshell.c $
    Revision 1.2  2009/03/14 22:59:14Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.4  2009/03/14 22:59:14Z  garyp
    Updated to call DclShellAddOsCommands() which will in turn add the FS
    commands, if appropriate.
    Revision 1.1.1.3  2008/12/10 21:23:06Z  garyp
    Pass the DCL instance handle to DclShellCreate().
    Revision 1.1.1.2  2008/12/10 20:22:17Z  garyp
    Updated to create a DCL Instance for use by program.
    Revision 1.1  2008/09/04 16:46:02Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <commctrl.h>

#include <dcl.h>
#include <dlshell.h>
#include <dlwinutil.h>
#include <winviewappfw.h>

#define PROGRAM_NAME            "WINDLSHELL"

static const WCHAR tzDescription[] = TEXT("Datalight Command Shell for ")TEXT(DCL_OSNAME);

static DCLSTATUS        ExecCommand(DCLINSTANCEHANDLE hDclInst, const WCHAR *ptzCmdLine);
static LRESULT WINAPI   AppWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
int WINAPI WinMain(
    HINSTANCE           hInstance,
    HINSTANCE           hPrevInstance,
    LPTSTR              ptzCmdLine,
    int                 nCmdShow)
{
    DCLSTATUS           dclStat;
    DCLINSTANCEHANDLE   hDclInst;

    dclStat = DclInstanceCreate(0, DCLFLAG_APP, &hDclInst);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = DclWinViewAppRun(hDclInst, 0, hInstance, hPrevInstance, NULL, nCmdShow,
                    TEXT(PROGRAM_NAME), tzDescription, ExecCommand, AppWndProc);

    DclInstanceDestroy(hDclInst);

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static LRESULT WINAPI AppWndProc(
    HWND            hWnd,
    UINT            nMsg,
    WPARAM          wParam,
    LPARAM          lParam)
{
    LRESULT         lResult;

    DCLWINMESSAGETRACEPRINT(MAKETRACEFLAGS(DCLTRACE_WINAPP, 1, TRACEINDENT),
        "WinDlShell:AppWndProc(%s)\n", hWnd, nMsg, wParam, lParam);

    lResult = DclWinViewAppWndProc(hWnd, nMsg, wParam, lParam);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINAPP, 1, TRACEUNDENT),
        "WinDlShell:AppWndProc() %s returning %lX\n", DclWinMessageName(nMsg), lResult));

    return lResult;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS ExecCommand(
    DCLINSTANCEHANDLE   hDclInst,
    const WCHAR        *ptzCmdLine)
{
    DCLSHELLHANDLE      hShell;
    DCLSTATUS           dclStat;

    (void)ptzCmdLine;

    hShell = DclShellCreate(hDclInst, "DCL", NULL, NULL, FALSE, TRUE, NULL);
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

    return dclStat;
}


