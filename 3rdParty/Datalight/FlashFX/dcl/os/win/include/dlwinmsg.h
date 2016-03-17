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

    This module contains macros and structures for decoding Windows messages.

    This code must be fully compilable and usable under both normal Win32
    user mode, as well as Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlwinmsg.h $
    Revision 1.2  2009/06/28 02:46:22Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.2  2008/11/05 01:50:37Z  garyp
    Documentation updated.
    Revision 1.1  2008/07/17 17:48:06Z  garyp
    Initial revision
    07/16/08 gp  Migrated from the WinLight project
    05/18/00 gp  Added MMSYSTEM messages
    03/29/99 vjp Remove SWP_FRAMECHANGED entry from list (it's not a message).
    03/03/99 gp  Changed msgTextArray[] declaration to FAR
    06/10/97 gp  Added internal AFX messages from MSVC 1.5
    03/06/97 gp  Created
--------------------------------------------------------------------------*/
/*
#ifndef __MMSYSTEM_H
#include <mmsystem.h>
#endif
*/
typedef struct
{
    UINT        nMsgNum;
    const char *pszMsgName;
} DCLWINMSGINFO;

static const DCLWINMSGINFO aMsgInfo[] =
{
    { WM_COMPACTING,                "WM_COMPACTING" },
    { WM_WININICHANGE,              "WM_WININICHANGE" },
    { WM_SYSCOLORCHANGE,            "WM_SYSCOLORCHANGE" },
    { WM_QUERYNEWPALETTE,           "WM_QUERYNEWPALETTE" },
    { WM_PALETTEISCHANGING,         "WM_PALETTEISCHANGING" },
    { WM_PALETTECHANGED,            "WM_PALETTECHANGED" },
    { WM_FONTCHANGE,                "WM_FONTCHANGE" },
    { WM_SPOOLERSTATUS,             "WM_SPOOLERSTATUS" },
    { WM_DEVMODECHANGE,             "WM_DEVMODECHANGE" },
    { WM_TIMECHANGE,                "WM_TIMECHANGE" },
    { WM_NULL,                      "WM_NULL" },
    { WM_USER,                      "WM_USER" },
    { WM_PENWINFIRST,               "WM_PENWINFIRST" },
    { WM_PENWINLAST,                "WM_PENWINLAST" },
    { WM_COALESCE_FIRST,            "WM_COALESCE_FIRST" },
    { WM_COALESCE_LAST,             "WM_COALESCE_LAST" },
    { WM_POWER,                     "WM_POWER" },
    { WM_QUERYENDSESSION,           "WM_QUERYENDSESSION" },
    { WM_ENDSESSION,                "WM_ENDSESSION" },
    { WM_QUIT,                      "WM_QUIT" },
    { WM_SYSTEMERROR,               "WM_SYSTEMERROR" },
    { WM_CREATE,                    "WM_CREATE" },
    { WM_NCCREATE,                  "WM_NCCREATE" },
    { WM_DESTROY,                   "WM_DESTROY" },
    { WM_NCDESTROY,                 "WM_NCDESTROY" },
    { WM_SHOWWINDOW,                "WM_SHOWWINDOW" },
    { WM_SETREDRAW,                 "WM_SETREDRAW" },
    { WM_ENABLE,                    "WM_ENABLE" },
    { WM_SETTEXT,                   "WM_SETTEXT" },
    { WM_GETTEXT,                   "WM_GETTEXT" },
    { WM_GETTEXTLENGTH,             "WM_GETTEXTLENGTH" },
    { WM_WINDOWPOSCHANGING,         "WM_WINDOWPOSCHANGING" },
    { WM_WINDOWPOSCHANGED,          "WM_WINDOWPOSCHANGED" },
    { WM_MOVE,                      "WM_MOVE" },
    { WM_SIZE,                      "WM_SIZE" },
    { WM_QUERYOPEN,                 "WM_QUERYOPEN" },
    { WM_CLOSE,                     "WM_CLOSE" },
    { WM_GETMINMAXINFO,             "WM_GETMINMAXINFO" },
    { WM_PAINT,                     "WM_PAINT" },
    { WM_ERASEBKGND,                "WM_ERASEBKGND" },
    { WM_ICONERASEBKGND,            "WM_ICONERASEBKGND" },
    { WM_NCPAINT,                   "WM_NCPAINT" },
    { WM_NCCALCSIZE,                "WM_NCCALCSIZE" },
    { WM_NCHITTEST,                 "WM_NCHITTEST" },
    { WM_QUERYDRAGICON,             "WM_QUERYDRAGICON" },
    { WM_DROPFILES,                 "WM_DROPFILES" },
    { WM_ACTIVATE,                  "WM_ACTIVATE" },
    { WM_ACTIVATEAPP,               "WM_ACTIVATEAPP" },
    { WM_NCACTIVATE,                "WM_NCACTIVATE" },
    { WM_SETFOCUS,                  "WM_SETFOCUS" },
    { WM_KILLFOCUS,                 "WM_KILLFOCUS" },
    { WM_KEYDOWN,                   "WM_KEYDOWN" },
    { WM_KEYUP,                     "WM_KEYUP" },
    { WM_CHAR,                      "WM_CHAR" },
    { WM_DEADCHAR,                  "WM_DEADCHAR" },
    { WM_SYSKEYDOWN,                "WM_SYSKEYDOWN" },
    { WM_SYSKEYUP,                  "WM_SYSKEYUP" },
    { WM_SYSCHAR,                   "WM_SYSCHAR" },
    { WM_SYSDEADCHAR,               "WM_SYSDEADCHAR" },
    { WM_KEYFIRST,                  "WM_KEYFIRST" },
    { WM_KEYLAST,                   "WM_KEYLAST" },
    { WM_MOUSEMOVE,                 "WM_MOUSEMOVE" },
    { WM_LBUTTONDOWN,               "WM_LBUTTONDOWN" },
    { WM_LBUTTONUP,                 "WM_LBUTTONUP" },
    { WM_LBUTTONDBLCLK,             "WM_LBUTTONDBLCLK" },
    { WM_RBUTTONDOWN,               "WM_RBUTTONDOWN" },
    { WM_RBUTTONUP,                 "WM_RBUTTONUP" },
    { WM_RBUTTONDBLCLK,             "WM_RBUTTONDBLCLK" },
    { WM_MBUTTONDOWN,               "WM_MBUTTONDOWN" },
    { WM_MBUTTONUP,                 "WM_MBUTTONUP" },
    { WM_MBUTTONDBLCLK,             "WM_MBUTTONDBLCLK" },
    { WM_MOUSEFIRST,                "WM_MOUSEFIRST" },
    { WM_MOUSELAST,                 "WM_MOUSELAST" },
    { WM_NCMOUSEMOVE,               "WM_NCMOUSEMOVE" },
    { WM_NCLBUTTONDOWN,             "WM_NCLBUTTONDOWN" },
    { WM_NCLBUTTONUP,               "WM_NCLBUTTONUP" },
    { WM_NCLBUTTONDBLCLK,           "WM_NCLBUTTONDBLCLK" },
    { WM_NCRBUTTONDOWN,             "WM_NCRBUTTONDOWN" },
    { WM_NCRBUTTONUP,               "WM_NCRBUTTONUP" },
    { WM_NCRBUTTONDBLCLK,           "WM_NCRBUTTONDBLCLK" },
    { WM_NCMBUTTONDOWN,             "WM_NCMBUTTONDOWN" },
    { WM_NCMBUTTONUP,               "WM_NCMBUTTONUP" },
    { WM_NCMBUTTONDBLCLK,           "WM_NCMBUTTONDBLCLK" },
    { WM_MOUSEACTIVATE,             "WM_MOUSEACTIVATE" },
    { WM_CANCELMODE,                "WM_CANCELMODE" },
    { WM_TIMER,                     "WM_TIMER" },
    { WM_INITMENU,                  "WM_INITMENU" },
    { WM_INITMENUPOPUP,             "WM_INITMENUPOPUP" },
    { WM_MENUSELECT,                "WM_MENUSELECT" },
    { WM_MENUCHAR,                  "WM_MENUCHAR" },
    { WM_COMMAND,                   "WM_COMMAND" },
    { WM_HSCROLL,                   "WM_HSCROLL" },
    { WM_VSCROLL,                   "WM_VSCROLL" },
    { WM_CUT,                       "WM_CUT" },
    { WM_COPY,                      "WM_COPY" },
    { WM_PASTE,                     "WM_PASTE" },
    { WM_CLEAR,                     "WM_CLEAR" },
    { WM_UNDO,                      "WM_UNDO" },
    { WM_RENDERFORMAT,              "WM_RENDERFORMAT" },
    { WM_RENDERALLFORMATS,          "WM_RENDERALLFORMATS" },
    { WM_DESTROYCLIPBOARD,          "WM_DESTROYCLIPBOARD" },
    { WM_DRAWCLIPBOARD,             "WM_DRAWCLIPBOARD" },
    { WM_PAINTCLIPBOARD,            "WM_PAINTCLIPBOARD" },
    { WM_SIZECLIPBOARD,             "WM_SIZECLIPBOARD" },
    { WM_VSCROLLCLIPBOARD,          "WM_VSCROLLCLIPBOARD" },
    { WM_HSCROLLCLIPBOARD,          "WM_HSCROLLCLIPBOARD" },
    { WM_ASKCBFORMATNAME,           "WM_ASKCBFORMATNAME" },
    { WM_CHANGECBCHAIN,             "WM_CHANGECBCHAIN" },
    { WM_SETCURSOR,                 "WM_SETCURSOR" },
    { WM_SYSCOMMAND,                "WM_SYSCOMMAND" },
    { WM_MDICREATE,                 "WM_MDICREATE" },
    { WM_MDIDESTROY,                "WM_MDIDESTROY" },
    { WM_MDIACTIVATE,               "WM_MDIACTIVATE" },
    { WM_MDIRESTORE,                "WM_MDIRESTORE" },
    { WM_MDINEXT,                   "WM_MDINEXT" },
    { WM_MDIMAXIMIZE,               "WM_MDIMAXIMIZE" },
    { WM_MDITILE,                   "WM_MDITILE" },
    { WM_MDICASCADE,                "WM_MDICASCADE" },
    { WM_MDIICONARRANGE,            "WM_MDIICONARRANGE" },
    { WM_MDIGETACTIVE,              "WM_MDIGETACTIVE" },
    { WM_MDISETMENU,                "WM_MDISETMENU" },
    { WM_CHILDACTIVATE,             "WM_CHILDACTIVATE" },
    { DM_GETDEFID,                  "DM_GETDEFID" },
    { DM_SETDEFID,                  "DM_SETDEFID" },
    { WM_INITDIALOG,                "WM_INITDIALOG" },
    { WM_NEXTDLGCTL,                "WM_NEXTDLGCTL" },
    { WM_PARENTNOTIFY,              "WM_PARENTNOTIFY" },
    { WM_ENTERIDLE,                 "WM_ENTERIDLE" },
    { WM_GETDLGCODE,                "WM_GETDLGCODE" },
    { WM_CTLCOLOR,                  "WM_CTLCOLOR" },
    { WM_SETFONT,                   "WM_SETFONT" },
    { WM_GETFONT,                   "WM_GETFONT" },
    { WM_DRAWITEM,                  "WM_DRAWITEM" },
    { WM_MEASUREITEM,               "WM_MEASUREITEM" },
    { WM_DELETEITEM,                "WM_DELETEITEM" },
    { WM_COMPAREITEM,               "WM_COMPAREITEM" },
    { STM_SETICON,                  "STM_SETICON" },
    { STM_GETICON,                  "STM_GETICON" },
    { BM_GETCHECK,                  "BM_GETCHECK" },
    { BM_SETCHECK,                  "BM_SETCHECK" },
    { BM_GETSTATE,                  "BM_GETSTATE" },
    { BM_SETSTATE,                  "BM_SETSTATE" },
    { BM_SETSTYLE,                  "BM_SETSTYLE" },
    { EM_GETSEL,                    "EM_GETSEL" },
    { EM_SETSEL,                    "EM_SETSEL" },
    { EM_GETRECT,                   "EM_GETRECT" },
    { EM_SETRECT,                   "EM_SETRECT" },
    { EM_SETRECTNP,                 "EM_SETRECTNP" },
    { EM_LINESCROLL,                "EM_LINESCROLL" },
    { EM_GETMODIFY,                 "EM_GETMODIFY" },
    { EM_SETMODIFY,                 "EM_SETMODIFY" },
    { EM_GETLINECOUNT,              "EM_GETLINECOUNT" },
    { EM_LINEINDEX,                 "EM_LINEINDEX" },
    { EM_SETHANDLE,                 "EM_SETHANDLE" },
    { EM_GETHANDLE,                 "EM_GETHANDLE" },
    { EM_LINELENGTH,                "EM_LINELENGTH" },
    { EM_REPLACESEL,                "EM_REPLACESEL" },
    { EM_SETFONT,                   "EM_SETFONT" },
    { EM_GETLINE,                   "EM_GETLINE" },
    { EM_LIMITTEXT,                 "EM_LIMITTEXT" },
    { EM_CANUNDO,                   "EM_CANUNDO" },
    { EM_UNDO,                      "EM_UNDO" },
    { EM_FMTLINES,                  "EM_FMTLINES" },
    { EM_LINEFROMCHAR,              "EM_LINEFROMCHAR" },
    { EM_SETWORDBREAK,              "EM_SETWORDBREAK" },
    { EM_SETTABSTOPS,               "EM_SETTABSTOPS" },
    { EM_SETPASSWORDCHAR,           "EM_SETPASSWORDCHAR" },
    { EM_EMPTYUNDOBUFFER,           "EM_EMPTYUNDOBUFFER" },
    { EM_GETFIRSTVISIBLELINE,       "EM_GETFIRSTVISIBLELINE" },
    { EM_SETREADONLY,               "EM_SETREADONLY" },
    { EM_SETWORDBREAKPROC,          "EM_SETWORDBREAKPROC" },
    { EM_GETWORDBREAKPROC,          "EM_GETWORDBREAKPROC" },
    { EM_GETPASSWORDCHAR,           "EM_GETPASSWORDCHAR" },
    { LB_ADDSTRING,                 "LB_ADDSTRING" },
    { LB_INSERTSTRING,              "LB_INSERTSTRING" },
    { LB_DELETESTRING,              "LB_DELETESTRING" },
    { LB_RESETCONTENT,              "LB_RESETCONTENT" },
    { LB_SETSEL,                    "LB_SETSEL" },
    { LB_SETCURSEL,                 "LB_SETCURSEL" },
    { LB_GETSEL,                    "LB_GETSEL" },
    { LB_GETCURSEL,                 "LB_GETCURSEL" },
    { LB_GETTEXT,                   "LB_GETTEXT" },
    { LB_GETTEXTLEN,                "LB_GETTEXTLEN" },
    { LB_GETCOUNT,                  "LB_GETCOUNT" },
    { LB_SELECTSTRING,              "LB_SELECTSTRING" },
    { LB_DIR,                       "LB_DIR" },
    { LB_GETTOPINDEX,               "LB_GETTOPINDEX" },
    { LB_FINDSTRING,                "LB_FINDSTRING" },
    { LB_GETSELCOUNT,               "LB_GETSELCOUNT" },
    { LB_GETSELITEMS,               "LB_GETSELITEMS" },
    { LB_SETTABSTOPS,               "LB_SETTABSTOPS" },
    { LB_GETHORIZONTALEXTENT,       "LB_GETHORIZONTALEXTENT" },
    { LB_SETHORIZONTALEXTENT,       "LB_SETHORIZONTALEXTENT" },
    { LB_SETCOLUMNWIDTH,            "LB_SETCOLUMNWIDTH" },
    { LB_SETTOPINDEX,               "LB_SETTOPINDEX" },
    { LB_GETITEMRECT,               "LB_GETITEMRECT" },
    { LB_GETITEMDATA,               "LB_GETITEMDATA" },
    { LB_SETITEMDATA,               "LB_SETITEMDATA" },
    { LB_SELITEMRANGE,              "LB_SELITEMRANGE" },
    { LB_SETCARETINDEX,             "LB_SETCARETINDEX" },
    { LB_GETCARETINDEX,             "LB_GETCARETINDEX" },
    { LB_SETITEMHEIGHT,             "LB_SETITEMHEIGHT" },
    { LB_GETITEMHEIGHT,             "LB_GETITEMHEIGHT" },
    { LB_FINDSTRINGEXACT,           "LB_FINDSTRINGEXACT" },
    { WM_VKEYTOITEM,                "WM_VKEYTOITEM" },
    { WM_CHARTOITEM,                "WM_CHARTOITEM" },
    { CB_GETEDITSEL,                "CB_GETEDITSEL" },
    { CB_LIMITTEXT,                 "CB_LIMITTEXT" },
    { CB_SETEDITSEL,                "CB_SETEDITSEL" },
    { CB_ADDSTRING,                 "CB_ADDSTRING" },
    { CB_DELETESTRING,              "CB_DELETESTRING" },
    { CB_DIR,                       "CB_DIR" },
    { CB_GETCOUNT,                  "CB_GETCOUNT" },
    { CB_GETCURSEL,                 "CB_GETCURSEL" },
    { CB_GETLBTEXT,                 "CB_GETLBTEXT" },
    { CB_GETLBTEXTLEN,              "CB_GETLBTEXTLEN" },
    { CB_INSERTSTRING,              "CB_INSERTSTRING" },
    { CB_RESETCONTENT,              "CB_RESETCONTENT" },
    { CB_FINDSTRING,                "CB_FINDSTRING" },
    { CB_SELECTSTRING,              "CB_SELECTSTRING" },
    { CB_SETCURSEL,                 "CB_SETCURSEL" },
    { CB_SHOWDROPDOWN,              "CB_SHOWDROPDOWN" },
    { CB_GETITEMDATA,               "CB_GETITEMDATA" },
    { CB_SETITEMDATA,               "CB_SETITEMDATA" },
    { CB_GETDROPPEDCONTROLRECT,     "CB_GETDROPPEDCONTROLRECT" },
    { CB_SETITEMHEIGHT,             "CB_SETITEMHEIGHT" },
    { CB_GETITEMHEIGHT,             "CB_GETITEMHEIGHT" },
    { CB_SETEXTENDEDUI,             "CB_SETEXTENDEDUI" },
    { CB_GETEXTENDEDUI,             "CB_GETEXTENDEDUI" },
    { CB_GETDROPPEDSTATE,           "CB_GETDROPPEDSTATE" },
    { CB_FINDSTRINGEXACT,           "CB_FINDSTRINGEXACT" },
    { WM_QUEUESYNC,                 "WM_QUEUESYNC" },
    { WM_COMMNOTIFY,                "WM_COMMNOTIFY" },

/*  Undocumented Windows messages
*/
    { WM_ENTERSIZEMOVE,             "WM_ENTERSIZEMOVE" },
    { WM_EXITSIZEMOVE,              "WM_EXITSIZEMOVE" },
    { WM_GETHOTKEY,                 "WM_GETHOTKEY" },
    { WM_ISACTIVEICON,              "WM_ISACTIVEICON" },
    { WM_PAINTICON,                 "WM_PAINTICON" },
    { WM_SETHOTKEY,                 "WM_SETHOTKEY" },
    { WM_SYNCPAINT,                 "WM_SYNCPAINT" },
    { WM_SYSTIMER,                  "WM_SYSTIMER" },

/*  Microsoft AFX messages
*/
   { WM_VBXEVENT,                   "WM_VBXEVENT" },
   { WM_SIZEPARENT,                 "WM_SIZEPARENT" },
   { WM_SETMESSAGESTRING,           "WM_SETMESSAGESTRING" },
   { WM_IDLEUPDATECMDUI,            "WM_IDLEUPDATECMDUI" },
   { WM_INITIALUPDATE,              "WM_INITIALUPDATE" },
   { WM_COMMANDHELP,                "WM_COMMANDHELP" },
   { WM_HELPHITTEST,                "WM_HELPHITTEST" },
   { WM_EXITHELPMODE,               "WM_EXITHELPMODE" },
   { WM_RECALCPARENT,               "WM_RECALCPARENT" },
   { WM_SIZECHILD,                  "WM_SIZECHILD" },
   { WM_KICKIDLE,                   "WM_KICKIDLE" },
   { WM_SOCKET_NOTIFY,              "WM_SOCKET_NOTIFY" },
   { WM_SOCKET_DEAD,                "WM_SOCKET_DEAD" },
/*
    MMSYSTEM messages
   { MM_JOY1MOVE,                   "MM_JOY1MOVE" },
   { MM_JOY2MOVE,                   "MM_JOY2MOVE" },
   { MM_JOY1ZMOVE,                  "MM_JOY1ZMOVE" },
   { MM_JOY2ZMOVE,                  "MM_JOY2ZMOVE" },
   { MM_JOY1BUTTONDOWN,             "MM_JOY1BUTTONDOWN" },
   { MM_JOY2BUTTONDOWN,             "MM_JOY2BUTTONDOWN" },
   { MM_JOY1BUTTONUP,               "MM_JOY1BUTTONUP" },
   { MM_JOY2BUTTONUP,               "MM_JOY2BUTTONUP" },
   { MM_MCINOTIFY,                  "MM_MCINOTIFY" },
*/
   { MM_WOM_OPEN,                   "MM_WOM_OPEN" },
   { MM_WOM_CLOSE,                  "MM_WOM_CLOSE" },
   { MM_WOM_DONE,                   "MM_WOM_DONE" },
/*
   { MM_WIM_OPEN,                   "MM_WIM_OPEN" },
   { MM_WIM_CLOSE,                  "MM_WIM_CLOSE" },
   { MM_WIM_DATA,                   "MM_WIM_DATA" },
   { MM_MIM_OPEN,                   "MM_MIM_OPEN" },
   { MM_MIM_CLOSE,                  "MM_MIM_CLOSE" },
   { MM_MIM_DATA,                   "MM_MIM_DATA" },
   { MM_MIM_LONGDATA,               "MM_MIM_LONGDATA" },
   { MM_MIM_ERROR,                  "MM_MIM_ERROR" },
   { MM_MIM_LONGERROR,              "MM_MIM_LONGERROR" },
   { MM_MOM_OPEN,                   "MM_MOM_OPEN" },
   { MM_MOM_CLOSE,                  "MM_MOM_CLOSE" },
   { MM_MOM_DONE,                   "MM_MOM_DONE" },
*/
   { 0, 0 }
};
