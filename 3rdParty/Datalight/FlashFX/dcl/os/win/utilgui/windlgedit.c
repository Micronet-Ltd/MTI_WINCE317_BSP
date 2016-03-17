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

    This module implements a simple dialog box with an edit control.

    This code must be fully compilable and usable under both normal Win32
    user mode, as well as Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: windlgedit.c $
    Revision 1.3  2009/07/11 05:23:13Z  garyp
    Moved the SetDialogPosition() functionality to a common implementation.
    Revision 1.2  2009/06/28 02:46:24Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.2  2008/11/05 02:00:53Z  garyp
    Added error handling code.  Updated documentation.
    Revision 1.1  2008/07/27 00:37:10Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>
#include <dlwinutil.h>

#include "winutilgui.rh"

static BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


/*-------------------------------------------------------------------
    Public: DclWinDialogEdit()

    This function implements a simple edit dialog box, using the
    initialization data in the specified DCLWINDLGEDITDATA structure.

    Parameters:
        hInstance - The application instance to use.
        pEditData - A pointer to the DCLWINDLGEDITDATA structure
                    to use.

    Returns:
        Returns the standard result from DialogBoxParam().
-------------------------------------------------------------------*/
int DclWinDialogEdit(
    HINSTANCE           hInstance,
    DCLWINDLGEDITDATA  *pEditData)
{
    int                 iResult;

    DclAssert(pEditData);
    DclAssert(pEditData->pwzBuffer);
    DclAssert(pEditData->nBufferLen);

    iResult = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_EDITDLG), NULL, DialogProc, (LPARAM)pEditData);
    if(iResult == -1)
    {
        D_WCHAR wcsErrorMsg[100] = {0};

        wsprintf(wcsErrorMsg, TEXT("Could not create the edit dialog box.  Error=0x%08x"), GetLastError());

        MessageBox(NULL, wcsErrorMsg, TEXT("Error"), MB_OK);

        return FALSE;
    }

    return iResult;
}


/*-------------------------------------------------------------------
    DialogProc()

    Parameters:

    Returns:
-------------------------------------------------------------------*/
static BOOL CALLBACK DialogProc(
    HWND            hWndDlg,
    UINT            nMsg,
    WPARAM          wParam,
    LPARAM          lParam)
{
    BOOL            fResult = FALSE;

    DCLWINMESSAGETRACEPRINT(MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, TRACEINDENT),
        "WinDlgEdit:DialogProc(%s)\n", hWndDlg, nMsg, wParam, lParam);

    switch(nMsg)
    {
        case WM_INITDIALOG:
        {
            DCLWINDLGEDITDATA  *pEditData = (DCLWINDLGEDITDATA*)lParam;
/*            UINT    nX = LOWORD(GetDialogBaseUnits());
            UINT    nY = HIWORD(GetDialogBaseUnits());
*/
            SetWindowText(hWndDlg, pEditData->pwzCaption);
            SetWindowText(GetDlgItem(hWndDlg, IDC_EDIT_PROMPT), pEditData->pwzPrompt);

            DclWinDialogSetPosition(hWndDlg, 0);

            SetLastError(0);
            if(!SetWindowLong(hWndDlg, GWL_USERDATA, lParam) && GetLastError())
            {
                DclError();
            }

            SendMessage(GetDlgItem(hWndDlg, IDC_EDIT_TEXT), WM_SETTEXT, 0, (LPARAM)pEditData->pwzBuffer);
/*
            MoveWindow(GetDlgItem(hWndDlg, IDCANCEL), 50*nX/4,74*nY/8,40*nX/4,15*nY/8, FALSE);
*/
            /*  Return success (TRUE).
            */
            fResult = TRUE;
        }
        break;

        case WM_CLOSE:
            /*  Nothing to do but end the dialog box.
            */
            EndDialog(hWndDlg, 0);

            /*  Return success (TRUE).
            */
            fResult = TRUE;
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                {
                    DCLWINDLGEDITDATA  *pEditData;

                    pEditData = (DCLWINDLGEDITDATA*)GetWindowLong(hWndDlg, GWL_USERDATA);
                    DclAssert(pEditData);

                    SendMessage(GetDlgItem(hWndDlg, IDC_EDIT_TEXT), WM_GETTEXT,
                        pEditData->nBufferLen, (LPARAM)pEditData->pwzBuffer);

                    EndDialog(hWndDlg, 1);
                    break;
                }

                case IDCANCEL:
                    EndDialog(hWndDlg, 0);
                    break;
            }

            fResult = TRUE;
            break;
    }

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, TRACEUNDENT),
        "WinDlgEdit:DialogProc() %s returning %u\n", DclWinMessageName(nMsg), fResult));

    return fResult;
}


