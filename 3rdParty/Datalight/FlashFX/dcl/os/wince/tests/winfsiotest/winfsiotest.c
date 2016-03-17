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
    $Log: winfsiotest.c $
    Revision 1.2  2009/06/28 02:46:27Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.2  2008/12/10 20:22:14Z  garyp
    Updated to create a DCL Instance for use by program.
    Revision 1.1  2008/09/01 05:14:16Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>
#include <dltools.h>
#include <dlwinutil.h>
#include <winviewappfw.h>

#define MAX_COMMAND_LINE_LEN    (256)
#define PROGRAM_NAME            "WINFSIOTEST"

static const WCHAR tzDescription[] = TEXT("Datalight File System I/O Test for ")TEXT(DCL_OSNAME);

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

    dclStat = DclWinViewAppRun(hDclInst, WINVIEWAPPFLAGS_EXITWAIT,
                    hInstance, hPrevInstance, ptzCmdLine, nCmdShow,
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
        "WinFsioTest:AppWndProc(%s)\n", hWnd, nMsg, wParam, lParam);

    lResult = DclWinViewAppWndProc(hWnd, nMsg, wParam, lParam);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINAPP, 1, TRACEUNDENT),
        "WinFsioTest:AppWndProc() %s returning %lX\n", DclWinMessageName(nMsg), lResult));

    return lResult;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS ExecCommand(
    DCLINSTANCEHANDLE   hDclInst,
    const WCHAR        *ptzCmdLine)
{
    DCLTOOLPARAMS       tp;
    char                szCmdLine[MAX_COMMAND_LINE_LEN];

    DclOsWcsToAnsi(szCmdLine, sizeof(szCmdLine), ptzCmdLine, -1);

    DclMemSet(&tp, 0, sizeof(tp));

    tp.hDclInst     = hDclInst;
    tp.pszCmdName   = PROGRAM_NAME;
    tp.pszCmdLine   = szCmdLine;

    return DclTestFSIOMain(&tp);
}

