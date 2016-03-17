/*---------------------------------------------------------------------------
           Copyright (c) 1988-2002 Modular Software Systems, Inc.
                            All Rights Reserved.
---------------------------------------------------------------------------*/

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

    This header defines the interface to the Win32 viewer abstraction.

    This code must be fully compilable and usable under both normal Win32
    user mode, as well as Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: winviewapi.h $
    Revision 1.2  2009/06/28 02:46:22Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.3  2008/11/05 01:20:48Z  garyp
    Documentation updated.
    Revision 1.1.1.2  2008/09/04 16:56:02Z  garyp
    Added a new user MSG definition.
    Revision 1.1  2008/07/22 19:51:20Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef WINVIEWAPI_H_INCLUDED
#define WINVIEWAPI_H_INCLUDED


#define WINVIEW_MAXLINENUM              0x7FFFFFFF
#define WINVIEW_TABAMOUNT               8

#define VIEWFLAGS_NOVSCROLL             0x00000001  /* Do NOT use a vertical scroll bar */
#define VIEWFLAGS_NOHSCROLL             0x00000002  /* Do NOT use a horizontal scroll bar */
#define VIEWFLAGS_NOSTATUS              0x00000004  /* Do NOT include a status line */
#define VIEWFLAGS_DYNAMIC               0x00000008
#define VIEWFLAGS_INPUT                 0x00000010

#define VIEWSTATE_RUNNING               (0)
#define VIEWSTATE_WAITING               (1)
#define VIEWSTATE_PAUSED                (2)
#define VIEWSTATE_COMPLETE              (3)
#define VIEWSTATE_TERMINATING           (4)

#define WM_WINVIEW_GETDATALEN           WM_USER+1
#define WM_WINVIEW_SETPOSITION          WM_USER+2
#define WM_WINVIEW_RELATIVEPOSITION     WM_USER+3
#define WM_WINVIEW_READDATA             WM_USER+4
#define WM_WINVIEW_GETLINESPERPAGE      WM_USER+5
#define WM_WINVIEW_SETFONT              WM_USER+6
#define WM_WINVIEW_GETKEY               WM_USER+7
#define WM_WINVIEW_SETSTATE             WM_USER+8
#define WM_WINVIEW_PAUSEGO              WM_USER+9
#define WM_WINVIEW_UPDATEWINDOW         WM_USER+11
#define WM_WINVIEW_RESERVEDY            WM_USER+12


DECLARE_HANDLE(HVIEW);

HVIEW      WINAPI DclWinViewCreate(HWND hWnd, D_UINT32 ulFlags, UINT uTabLen, ULONG ulCacheSize);
BOOL       WINAPI DclWinViewDestroy(HVIEW hView);
BOOL       WINAPI DclWinViewPaint(HVIEW hView, HWND hWnd);
HVIEW      WINAPI DclWinViewObjectCreate(HINSTANCE hInst, HWND hWndParent, HFONT hFont, ULONG ulFlags, UINT uTabLen, ULONG ulCacheSize);
void       WINAPI DclWinViewObjectDestroy(HVIEW hView);
LONG       WINAPI DclWinViewObjectDispatchMessage(HWND hWnd, UINT message, UINT wParam, LONG lParam);


#endif  /* WINVIEWAPI_H_INCLUDED */


