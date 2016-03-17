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

    This module contains code to dynamically adjust a dialog box position.

    This code must be fully compilable and usable under both normal Win32
    user mode, as well as Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: windlgposition.c $
    Revision 1.1  2009/07/11 02:29:42Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>
#include <dlwinutil.h>


/*-------------------------------------------------------------------
    Public: DclWinDialogSetPosition()

    Set the specified dialog position on the screen, adjusted
    according to the specified level value.  This allows for a
    certain level of aesthetic acceptability with a mimimal
    amount of hassle.  If the window will not fit on the screen
    it will be positioned in the upper left corner such that
    the dialog border is just off the screen.

    Parameters:
        hDlg    - The dialog box handle.
        nLevel  - The dialog level.  Top level is 0, next is 1, etc.

    Return Value:
        Returns TRUE if the dialog fits on the screen, and FALSE
        if not.
-------------------------------------------------------------------*/
D_BOOL DclWinDialogSetPosition(
    HWND        hDlg,
    unsigned    nLevel)
{
    #define     BIASX   (100)
    #define     BIASY   (60)
    RECT        rc;
    int         cX, cY;
    int         devX, devY;
    int         newX, newY;
    D_BOOL      fFits = TRUE;

    GetWindowRect(hDlg, &rc);

    cX = rc.right - rc.left;
    cY = rc.bottom - rc.top;

    devX = GetSystemMetrics(SM_CXSCREEN);
    devY = GetSystemMetrics(SM_CYSCREEN);

    if(cX > devX)
    {
        newX = -GetSystemMetrics(SM_CXDLGFRAME);
        fFits = FALSE;
    }
    else
    {
        newX = (devX - cX) / 2;
    }

    if(cY > devY)
    {
        newY = -GetSystemMetrics(SM_CYDLGFRAME);
        fFits = FALSE;
    }
    else
    {
        newY = (devY - cY) / 2;
    }

    if(newX + cX + ((int)nLevel * BIASX) <= devX)
        newX += (nLevel * BIASX);

    if(newY + cY + ((int)nLevel * BIASY) <= devY)
        newY += (nLevel * BIASY);

    MoveWindow(hDlg, newX, newY, cX, cY, TRUE);

    return fFits;
}



