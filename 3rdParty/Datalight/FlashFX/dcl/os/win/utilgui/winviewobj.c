/*---------------------------------------------------------------------------
           Copyright (c) 1988-2002 Modular Software Systems, Inc.
                            All Rights Reserved.
---------------------------------------------------------------------------*/

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

    This module contains the high level viewer functions.

    This code must be fully compilable and usable under both normal Win32
    user mode, as well as Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: winviewobj.c $
    Revision 1.4  2010/11/01 04:25:20Z  garyp
    Fixed to finally work properly under both Win32 and WinCE at the same
    time.  Fixed font scaling problems.  Modified so logging is handled at
    a higher level.
    Revision 1.3  2009/07/01 21:12:55Z  garyp
    Initialize a local to satisfy a picky compiler.  Updated debug code.
    Revision 1.2  2009/06/28 02:46:25Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.4  2008/11/05 01:20:46Z  garyp
    Documentation updated.
    Revision 1.1.1.3  2008/09/01 17:49:46Z  garyp
    Modified the "page" window to allow for reserved space which may be
    needed for the CE CommandBar.
    Revision 1.1.1.2  2008/08/25 18:00:13Z  garyp
    Updated to include private implementations of DclDebugBreak() and
    DclOsHalt().  Added caret support.
    Revision 1.1  2008/07/27 00:43:32Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <string.h>
#include <limits.h>

#include <dcl.h>
#include <dlwinutil.h>
#include <winscrollapi.h>
#include <winviewapi.h>
#include "winview.h"

static const TCHAR  szPageClassName[]    = TEXT("DclWinView:Page");
static const TCHAR  szStatusClassName[]  = TEXT("DclWinView:Status");

#define KEYBUFF_SIZE        (256)
#define IDCW_STATUS         (1001)
#define IDCW_VIEW           (1002)
#define VIEWCORE_FLAGSMASK  VIEWFLAGS_DYNAMIC

typedef struct sVIEWINSTEX
{
    struct sVIEWINSTEX *pNext;
    struct sVIEWINSTEX *pPrev;
    HINSTANCE           hInstance;
    HWND                hWndParent;
    HWND                hWndStatus;
    HWND                hWndPage;
    HVIEW               hView;
    ULONG               ulFlags;
    UINT                uCharWidth;
    UINT                uCharHeight;
    HSCROLLBAR          hVScrollbar;
    HSCROLLBAR          hHScrollbar;
    UINT                uDisplayCharsPerLine;
    ULONG              *pulKeyBuff;
    unsigned            nKeyBuffHead;
    unsigned            nKeyBuffTail;
    unsigned            nState;
    VIEWINST           *pVI;
    unsigned            fHaveFocus : 1;
    unsigned            fHasCaret : 1;
    unsigned            fCaretVisible : 1;
} VIEWINSTEX;

static VIEWINSTEX      *pHeadVIX;

static VIEWINSTEX *     LookupViewObjectFromHView(HVIEW hView);
static VIEWINSTEX *     LookupViewObjectFromHWnd(HWND hWnd);
static LRESULT WINAPI   ViewObjPageWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
static LRESULT WINAPI   ViewObjStatusWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
static VIEWINSTEX *     AddViewObjectInstance(void);
static LONG             GetFontHeight2(            HWND hWnd, HFONT hFont);
static void             DeleteViewObjectInstance(  VIEWINSTEX *pVIX);
static UINT             GetLinesPerPage(HWND hWnd, VIEWINSTEX *pVIX);
static LONG             GetFontHeight(             VIEWINSTEX *pVIX, HWND hWnd, HFONT hFont);
static void             ViewSizeWindow(            VIEWINSTEX *pVIX, HWND hWnd, int cx, int cy);
static ULONG            GetKey(                    VIEWINSTEX *pVIX);
static BOOL             PutKey(                    VIEWINSTEX *pVIX, ULONG ulKey);
static void             UpdateCaret(               VIEWINSTEX *pVIX, HWND hWnd);
static void             RecalcScrollbars(          VIEWINSTEX *pVIX);




            /*-------------------------------------------------*\
             *                                                 *
             *                Public Interface                 *
             *                                                 *
            \*-------------------------------------------------*/


/*-------------------------------------------------------------------
    Public: DclWinViewObjectCreate()

    This function creates a master viewer object, and all the various
    related objects such as scrollbars and the status line.

    Parameters:
        hInstance     - The application instance handle.
        hWndParent    - The parent window handle.
        hFont         - The font handle to use.
        ulFlags       - The flags to use, which is one or more of the
                        following values:
                        - VIEWFLAGS_NOVSCROLL - Indicates that a
                          vertical scrollbar should not be used.
                        - VIEWFLAGS_NOHSCROLL - Indicates that a
                          horizontal scrollbar should not be used.
                        - VIEWFLAGS_NOSTATUS - Indicates that a
                          status line should not be used.
                        - VIEWFLAGS_DYNAMIC - Indicates that the data
                          is dynamically growing, and that the cursor
                          focus should be maintained at the end of
                          the data stream.
                        - VIEWFLAGS_INPUT - Indicates that the object
                          will be dynamically getting input.
        uTabLen       - The number of characters which a hard tab
                        should represent.
        ulCacheSize   - The size of the cache buffer to use when
                        reading data.

    Return Value:
        Returns an HVIEW handle if successful, or NULL otherwise.
-------------------------------------------------------------------*/
HVIEW WINAPI DclWinViewObjectCreate(
    HINSTANCE       hInst,
    HWND            hWndParent,
    HFONT           hFont,
    ULONG           ulFlags,
    UINT            uTabLen,
    ULONG           ulCacheSize)
{
    WNDCLASS        wc;
    VIEWINST       *pVI;
    VIEWINSTEX     *pVIX;
    HVIEW           hView;
    DWORD           dwFlags;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, TRACEINDENT),
        "DclWinViewObjectCreate() hInst=%P hWndParent=%P Flags=%lX TabLen=%u CacheSize=%lX\n",
        hInst, hWndParent, ulFlags, uTabLen, ulCacheSize));

    DclMemSet(&wc, 0, sizeof(wc));

    /*  Register the record display window class
    */
    wc.style          = CS_HREDRAW | CS_VREDRAW;
    wc.hInstance      = hInst;
    wc.hbrBackground  = GetStockObject(WHITE_BRUSH);
    wc.lpfnWndProc    = ViewObjPageWndProc;
    wc.lpszClassName  = szPageClassName;

    RegisterClass(&wc);

    if(!(ulFlags & VIEWFLAGS_NOSTATUS))
    {
        /*  Register the status bar class
        */
        wc.hbrBackground  = GetStockObject(LTGRAY_BRUSH);
        wc.lpfnWndProc    = ViewObjStatusWndProc;
        wc.lpszClassName  = szStatusClassName;

        RegisterClass(&wc);
    }

    pVIX = AddViewObjectInstance();
    DclAssert(pVIX);

    pVIX->hInstance  = hInst;
    pVIX->hWndParent = hWndParent;
    pVIX->ulFlags    = ulFlags;

    if(ulFlags & VIEWFLAGS_INPUT)
    {
        pVIX->pulKeyBuff = DclMemAllocZero(sizeof(*pVIX->pulKeyBuff) * KEYBUFF_SIZE);
        DclAssert(pVIX->pulKeyBuff);
    }

    if(!(ulFlags & VIEWFLAGS_NOSTATUS))
    {
        pVIX->hWndStatus = CreateWindow(szStatusClassName,
            NULL,
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
            0, 0, 1, 1,
            hWndParent,
            (HMENU)IDCW_STATUS,
            hInst,
            NULL);
    }

    dwFlags = WS_CHILD | WS_VISIBLE | WS_BORDER;

    if(!(ulFlags & VIEWFLAGS_NOVSCROLL))
        dwFlags |= WS_VSCROLL;

    if(!(ulFlags & VIEWFLAGS_NOHSCROLL))
        dwFlags |= WS_HSCROLL;

    pVIX->hWndPage = CreateWindow(szPageClassName,
        NULL,
        dwFlags,
        0, 0, 1, 1,
        hWndParent,
        (HMENU)IDCW_VIEW,
        hInst,
        NULL);

    hView = DclWinViewCreate(hWndParent, ulFlags & VIEWCORE_FLAGSMASK, uTabLen, ulCacheSize);
    DclAssert(hView);

    pVI = (VIEWINST*)hView;

    pVIX->pVI = pVI;

    pVI->hFont = hFont;

    pVI->uTextHeight = GetFontHeight(pVIX, pVIX->hWndPage, hFont);

    /*  initialize scroll bars if any
    */
    if(!(ulFlags & VIEWFLAGS_NOVSCROLL))
        pVIX->hVScrollbar = DclWinScrollbarObjectCreate(pVIX->hWndPage, SB_VERT, 0);

    if(!(ulFlags & VIEWFLAGS_NOHSCROLL))
        pVIX->hHScrollbar = DclWinScrollbarObjectCreate(pVIX->hWndPage, SB_HORZ, 0);

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, TRACEUNDENT),
        "DclWinViewObjectCreate() returning hView=%P\n", hView));

    return hView;
}


/*-------------------------------------------------------------------
    Public: DclWinViewObjectDestroy()

    This function destroys a master viewer object, and all the
    various related objects such as scrollbars and the status line.

    Parameters:
        hView         - The view handle.

    Return Value:
        None.
-------------------------------------------------------------------*/
void WINAPI DclWinViewObjectDestroy(
    HVIEW           hView)
{
    VIEWINSTEX     *pVIX;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
        "DclWinViewObjectDestroy() hView=%P\n", hView));

    pVIX = LookupViewObjectFromHView(hView);
    DclAssert(pVIX);

    if(!pVIX)
        return;

    if(pVIX->fHasCaret)
    {
        DestroyCaret();
        pVIX->fHasCaret = FALSE;
    }

    DclWinViewDestroy(hView);

    if(pVIX->pulKeyBuff)
        DclMemFree(pVIX->pulKeyBuff);

    DeleteViewObjectInstance(pVIX);

    return;
}


/*-------------------------------------------------------------------
    Public: DclWinViewObjectDispatchMessage()

    This function is the standard message handler for the "View"
    system.

    Parameters:
        hWnd    - The window handle.
        nMsg    - The message number.
        wParam  - The WPARAM parameter.
        lParam  - The LPARAM parameter.

    Return Value:
        Returns a standard message LRESULT value.
-------------------------------------------------------------------*/
LRESULT WINAPI DclWinViewObjectDispatchMessage(
    HWND            hWnd,
    UINT            nMsg,
    WPARAM          wParam,
    LPARAM          lParam)
{
    VIEWINSTEX     *pVIX;
    LRESULT         lResult;

    DCLWINMESSAGETRACEPRINT(MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, TRACEINDENT),
        "DclWinViewObjectDispatchMessage(%s)\n", hWnd, nMsg, wParam, lParam);

    pVIX = LookupViewObjectFromHWnd(hWnd);

    if(nMsg == WM_SIZE)
    {
        ViewSizeWindow(pVIX, hWnd, (int)LOWORD(lParam), (int)HIWORD(lParam));

        lResult = 0;
    }
    else
    {
        if(pVIX)
            lResult = SendMessage(pVIX->hWndPage, nMsg, wParam, lParam);
        else
            lResult = DefWindowProc(hWnd, nMsg, wParam, lParam);
    }

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, TRACEUNDENT),
        "DclWinViewObjectDispatchMessage() %s returning %lX\n",
        DclWinMessageName(nMsg), lResult));

    return lResult;
}


            /*-------------------------------------------------*\
             *                                                 *
             *                    Wnd Procs                    *
             *                                                 *
            \*-------------------------------------------------*/


/*-------------------------------------------------------------------
    ViewObjPageWndProc
-------------------------------------------------------------------*/
static LRESULT WINAPI ViewObjPageWndProc(
    HWND            hWnd,
    UINT            nMsg,
    WPARAM          wParam,
    LPARAM          lParam)
{
    LRESULT         lResult = 0;
    VIEWINST       *pVI;
    VIEWINSTEX     *pVIX;

    DCLWINMESSAGETRACEPRINT(MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, TRACEINDENT),
        "ViewObj:PageWndProc(%s)\n", hWnd, nMsg, wParam, lParam);

    pVIX = LookupViewObjectFromHWnd(hWnd);

    if(!pVIX)
    {
        lResult = DefWindowProc(hWnd, nMsg, wParam, lParam);
        goto ViewObjPageWndProcCleanup;
    }

    DclAssert(pVIX);

    pVI = pVIX->pVI;

    switch(nMsg)
    {
        case WM_KEYDOWN:
        {
            BOOL  fRedraw = FALSE;              /* assume no redraw required */
            BOOL  fRedrawVScrollbar = FALSE;    /* assume no redraw required */
            BOOL  fRedrawHScrollbar = FALSE;    /* assume no redraw required */

            switch(wParam)
            {
                case VK_DOWN:
                {
                    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
                        "ViewObj:PWP() VK_DOWN\n"));

                    pVI->ulTopLine++;
                    fRedraw = TRUE;
                    fRedrawVScrollbar = TRUE;
                }
                break;

                case VK_UP:
                {
                    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
                        "ViewObj:PWP() VK_UP\n"));

                    if(pVI->ulTopLine > 1)
                    {
                        pVI->ulTopLine--;
                        fRedraw = TRUE;
                        fRedrawVScrollbar = TRUE;
                        pVI->fNewLineFocus = FALSE;
                    }
                    else
                    {
                        MessageBeep(MB_ICONASTERISK);
                    }
                }
                break;

                case VK_LEFT:
                {
                    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
                        "ViewObj:PWP() VK_LEFT\n"));

                    if(pVI->uHorizOffset)
                    {
                        if(pVI->uHorizOffset >= WINVIEW_TABAMOUNT)
                            pVI->uHorizOffset -= WINVIEW_TABAMOUNT;
                        else
                            pVI->uHorizOffset = 0;

                        fRedraw = TRUE;
                        fRedrawHScrollbar = TRUE;
                    }
                    else
                    {
                        MessageBeep(MB_ICONASTERISK);
                    }
                }
                break;

                case VK_RIGHT:
                {
                    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
                        "ViewObj:PWP() VK_RIGHT\n"));

                    if(pVI->uHorizOffset + pVIX->uDisplayCharsPerLine < pVI->uMaxDisplayLineLength)
                    {
                        pVI->uHorizOffset += WINVIEW_TABAMOUNT;

                        fRedraw = TRUE;
                        fRedrawHScrollbar = TRUE;
                    }
                }
                break;

                case VK_PRIOR:
                {
                    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
                        "ViewObj:PWP() VK_PRIOR\n"));

                    if(pVI->ulTopLine > 1)
                    {
                        if(pVI->ulTopLine > pVI->uLinesPerPage)
                            pVI->ulTopLine -= pVI->uLinesPerPage;
                        else
                            pVI->ulTopLine = 1;

                        fRedraw = TRUE;
                        fRedrawVScrollbar = TRUE;

                        pVI->fNewLineFocus = FALSE;
                    }
                    else
                    {
                        MessageBeep(MB_ICONASTERISK);
                    }
                }
                break;

                case VK_NEXT:
                {
                    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
                        "ViewObj:PWP() VK_NEXT\n"));

                    pVI->ulTopLine += pVI->uLinesPerPage;

                    fRedraw = TRUE;
                    fRedrawVScrollbar = TRUE;
                }
                break;

                case VK_HOME:
                {
                    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
                        "ViewObj:PWP() VK_HOME\n"));

                    if(pVI->ulTopLine > 1)
                    {
                        pVI->ulTopLine = 1;

                        fRedraw = TRUE;
                        fRedrawVScrollbar = TRUE;

                        pVI->fNewLineFocus = FALSE;
                    }
                    else
                    {
                        MessageBeep(MB_ICONASTERISK);
                    }
                }
                break;

                case VK_END:
                {
                    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
                        "ViewObj:PWP() VK_END\n"));

                    pVI->ulTopLine = WINVIEW_MAXLINENUM;

                    fRedraw = TRUE;
                    fRedrawVScrollbar = TRUE;

                    if(pVIX->ulFlags & VIEWFLAGS_DYNAMIC)
                        pVI->fNewLineFocus = TRUE;
                }
                break;

                /*  ignore without MessageBeep()
                */
                case VK_SHIFT:
                {
                }
                break;

                default:
                {
                    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
                        "ViewObj:PWP() WM_KEYDOWN unhandled WP=%lX LP=%lX\n", wParam, lParam));

/*                    MessageBeep(MB_ICONASTERISK); */
                }
                break;
            }

            if(fRedraw)
                InvalidateRect(pVIX->hWndPage, 0, TRUE);

/*            UpdateCaret(pVIX, hWnd);*/

            if(fRedrawVScrollbar && pVIX->hVScrollbar)
                DclWinScrollbarSetPosition(pVIX->hVScrollbar, pVI->ulTopLine-1, TRUE);

            if(fRedrawHScrollbar && pVIX->hHScrollbar)
                DclWinScrollbarSetPosition(pVIX->hHScrollbar, pVI->uHorizOffset, TRUE);
        }
        break;

        case WM_CHAR:
        {
            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
                "ViewObj:PWP() WM_CHAR WP=%lX LP=%lX\n", wParam, lParam));

            if(PutKey(pVIX, wParam))
                lResult = 1;
            else
                MessageBeep(MB_ICONASTERISK);
        }
        break;

        case WM_SIZE:
        {
            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
                "ViewObj:PWP() WM_SIZE\n"));

            pVIX->pVI->nPageWidth = LOWORD(lParam);
            pVIX->pVI->nPageHeight = HIWORD(lParam);

            RecalcScrollbars(pVIX);
        }
        break;

        case WM_MOUSEWHEEL:
        case WM_VSCROLL:
        {
            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
                "ViewObj:PWP() WM_VSCROLL or WM_MOUSEWHEEL WP=%lX LP=%lX\n", wParam, lParam));

            if(pVIX->hVScrollbar)
            {
                unsigned    nNewPos;

                nNewPos = DclWinScrollbarProcessEvent(pVIX->hVScrollbar, nMsg, wParam, lParam);

                if(nNewPos != UINT_MAX && (nNewPos+1 != pVI->ulTopLine))
                {
                    pVI->ulTopLine = nNewPos+1;

                    InvalidateRect(pVIX->hWndPage, 0, TRUE);

                    pVI->fNewLineFocus = FALSE;
                }
            }
        }
        break;

        case WM_HSCROLL:
        {
            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
                "ViewObj:PWP() WM_HSCROLL\n"));

            if(pVIX->hHScrollbar)
            {
                unsigned    nNewPos;

                nNewPos = DclWinScrollbarProcessEvent(pVIX->hHScrollbar, nMsg, wParam, lParam);

                if(nNewPos != UINT_MAX && (nNewPos != pVI->uHorizOffset))
                {
                    pVI->uHorizOffset = nNewPos;

                    InvalidateRect(pVIX->hWndPage, 0, TRUE);
                }
            }
        }
        break;

        case WM_SETFOCUS:
        {
            pVIX->fHaveFocus = TRUE;

            if(pVIX->ulFlags & VIEWFLAGS_INPUT)
            {
                if(CreateCaret(hWnd, NULL, pVIX->uCharWidth, pVI->uTextHeight))
                {
                    pVIX->fHasCaret = TRUE;

                    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
                        "ViewObj:PWP() WM_SETFOCUS created caret\n"));

                    UpdateCaret(pVIX, hWnd);
                }
            }
        }
        break;

        case WM_KILLFOCUS:
        {
            pVIX->fHaveFocus = FALSE;

            if(pVIX->fHasCaret)
            {
                pVIX->fHasCaret = FALSE;
                pVIX->fCaretVisible = FALSE;

                HideCaret(hWnd);
                DestroyCaret();

                DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
                    "ViewObj:PWP() WM_KILLFOCUS destroyed caret\n"));
            }
        }
        break;

        case WM_WINVIEW_SETFONT:
        {
            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
                "ViewObj:PWP() WM_WINVIEW_SETFONT\n"));

            pVI->hFont = (HFONT)wParam;
 
            pVI->uTextHeight = GetFontHeight(pVIX, hWnd, (HFONT)wParam);

            if(lParam)
            {
                InvalidateRect(hWnd, 0, TRUE);

                RecalcScrollbars(pVIX);
            }
        }
        break;

        case WM_WINVIEW_GETLINESPERPAGE:
        {
            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
                "ViewObj:PWP() WM_WINVIEW_GETLINESPERPAGE\n"));

            lResult = GetLinesPerPage(hWnd, pVIX);
        }
        break;

        case WM_WINVIEW_GETKEY:
        {
            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
                "ViewObj:PWP() WM_WINVIEW_GETKEY\n"));

            if(pVIX->ulFlags & VIEWFLAGS_INPUT)
            {
                if(pVIX->nState != VIEWSTATE_WAITING)
                {
/*                    SendMessage(pVIX->hWndPage, WM_WINVIEW_SETSTATE, VIEWSTATE_WAITING, 0); */

                    if(pVIX->nState != VIEWSTATE_WAITING)
                    {
                        pVIX->nState = VIEWSTATE_WAITING;

                        if(pVIX->hWndStatus)
                        {
                            InvalidateRect(pVIX->hWndStatus, 0, TRUE);
                            UpdateWindow(pVIX->hWndStatus);
                        }
                    }

                    UpdateCaret(pVIX, hWnd);
                }

                lResult = GetKey(pVIX);

                if(lResult)
                {
                    pVIX->nState = VIEWSTATE_RUNNING;

                    if(pVIX->hWndStatus)
                    {
                        InvalidateRect(pVIX->hWndStatus, 0, TRUE);
                        UpdateWindow(pVIX->hWndStatus);
                    }

                    UpdateCaret(pVIX, hWnd);
                }

                DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
                    "ViewObj:PWP() WM_WINVIEW_GETKEY got %lX\n", lResult));
            }
        }
        break;

        case WM_WINVIEW_SETSTATE:
        {
            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
                "ViewObj:PWP() WM_WINVIEW_SETSTATE State=%lU\n", wParam));

            if(pVIX->nState != (unsigned)wParam)
            {
                pVIX->nState = (unsigned)wParam;

                if(pVIX->hWndStatus)
                {
                    InvalidateRect(pVIX->hWndStatus, 0, TRUE);
                    UpdateWindow(pVIX->hWndStatus);
                }
            }
        }
        break;

        case WM_WINVIEW_PAUSEGO:
        {
            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
                "ViewObj:PWP() WM_WINVIEW_PAUSEGO\n" ));

            if(pVIX->nState == VIEWSTATE_RUNNING)
            {
                lResult = pVIX->nState;

                pVIX->nState = VIEWSTATE_PAUSED;

                if(pVIX->hWndStatus)
                    InvalidateRect(pVIX->hWndStatus, 0, TRUE);
            }
            else if(pVIX->nState == VIEWSTATE_PAUSED)
            {
                lResult = pVIX->nState;

                pVIX->nState = VIEWSTATE_RUNNING;

                if(pVIX->hWndStatus)
                    InvalidateRect(pVIX->hWndStatus, 0, TRUE);
            }
            else
            {
                lResult = -1;
            }
        }
        break;

        case WM_WINVIEW_UPDATEWINDOW:
        {
            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
                "ViewObj:PWP() WM_WINVIEW_UPDATEWINDOW\n" ));

/*          if((pVI->fNewLineFocus) || (pVI->ulTopLine + pVI->uLinesPerPage > pVI->ulLSACountUsed)) */
            {
                InvalidateRect(pVIX->hWndPage, NULL, TRUE);
                UpdateWindow(pVIX->hWndPage);
            }

            InvalidateRect(pVIX->hWndStatus, NULL, TRUE);
            UpdateWindow(pVIX->hWndStatus);
        }
        break;

        case WM_PAINT:
        {
            ULONG   ulOldTop    = pVI->ulTopLine;
            ULONG   ulOldCount  = pVI->ulLSACountUsed;
            UINT    uOldMaxLen  = pVI->uMaxDisplayLineLength;

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
                "ViewObj:PWP() WM_PAINT\n"));

            DclWinViewPaint((HVIEW)pVI, hWnd);

            UpdateCaret(pVIX, hWnd);

            if((pVIX->hVScrollbar) &&
                ((pVI->ulLSACountUsed != ulOldCount) || (pVI->ulTopLine != ulOldTop)))
            {
                DclWinScrollbarSetMetrics(pVIX->hVScrollbar, pVI->ulLSACountUsed, pVI->uLinesPerPage, FALSE);
                DclWinScrollbarSetPosition(pVIX->hVScrollbar, pVI->ulTopLine-1, TRUE);
            }

            if((pVIX->hHScrollbar) && (pVI->uMaxDisplayLineLength != uOldMaxLen))
            {
                DclWinScrollbarSetMetrics(pVIX->hHScrollbar, pVI->uMaxDisplayLineLength, pVIX->uDisplayCharsPerLine, TRUE);
            }

            if(pVIX->hWndStatus)
                InvalidateRect(pVIX->hWndStatus, 0, TRUE);
        }
        break;

        default:
        {
            lResult = DefWindowProc(hWnd, nMsg, wParam, lParam);
        }
        break;
    }

  ViewObjPageWndProcCleanup:

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, TRACEUNDENT),
        "ViewObj:PageWndProc() %s returning %lX\n", DclWinMessageName(nMsg), lResult));

    return lResult;
}


/*-------------------------------------------------------------------
    ViewObjStatusWndProc
-------------------------------------------------------------------*/
static LRESULT WINAPI ViewObjStatusWndProc(
    HWND            hWnd,
    UINT            nMsg,
    WPARAM          wParam,
    LPARAM          lParam)
{
    LRESULT         lResult = 0;
    VIEWINST       *pVI;
    VIEWINSTEX     *pVIX;

    DCLWINMESSAGETRACEPRINT(MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, TRACEINDENT),
        "ViewObj:StatusWndProc(%s)\n", hWnd, nMsg, wParam, lParam);

    pVIX = LookupViewObjectFromHWnd(hWnd);

    if(!pVIX)
    {
        lResult = DefWindowProc(hWnd, nMsg, wParam, lParam);

        goto ViewObjStatusWndProcCleanup;
    }

    DclAssert(pVIX);

    pVI = pVIX->pVI;

    switch(nMsg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC         hDC;

            hDC = BeginPaint(hWnd, &ps);
            {
                static TCHAR    awcBuffer[100];
                static TCHAR    awcTempBuff[200];
                RECT            rc;
                int             TextMode;
                ULONG           ulTempLine;

                GetClientRect(hWnd, &rc);

                TextMode = GetBkMode(hDC);
                SetBkMode(hDC, TRANSPARENT);

                ulTempLine = DCLMIN(pVI->ulTopLine, pVI->ulLSACountUsed);

                wsprintf(awcBuffer, TEXT(" Line %lu of %lu"), ulTempLine, pVI->ulLSACountUsed);

                if(pVIX->nState != VIEWSTATE_COMPLETE)
                    wcscat(awcBuffer, TEXT("+"));

                rc.top++;

                wsprintf(awcTempBuff, TEXT("%s  Offset %u"), (LPTSTR)awcBuffer, pVI->uHorizOffset);

                DrawText(hDC, awcTempBuff, -1, &rc, DT_LEFT | DT_VCENTER);

                switch(pVIX->nState)
                {
                    case VIEWSTATE_RUNNING:
                        DrawText(hDC, TEXT("Running "), -1, &rc, DT_RIGHT | DT_VCENTER);
                        break;

                    case VIEWSTATE_WAITING:
                        DrawText(hDC, TEXT("Waiting... "), -1, &rc, DT_RIGHT | DT_VCENTER);
                        break;

                    case VIEWSTATE_PAUSED:
                        DrawText(hDC, TEXT("Paused "), -1, &rc, DT_RIGHT | DT_VCENTER);
                        break;

                    case VIEWSTATE_TERMINATING:
                        DrawText(hDC, TEXT("Terminating... "), -1, &rc, DT_RIGHT | DT_VCENTER);
                        break;

                    case VIEWSTATE_COMPLETE:
                        DrawText(hDC, TEXT("Completed "), -1, &rc, DT_RIGHT | DT_VCENTER);
                        break;

                    default:
                        DclError();
                        break;
                }

                SetBkMode(hDC, TextMode);
            }

            EndPaint(hWnd, &ps);
        }
        break;

        default:
        {
            lResult = DefWindowProc(hWnd, nMsg, wParam, lParam);
        }
        break;
    }

  ViewObjStatusWndProcCleanup:

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, TRACEUNDENT),
        "ViewObj:StatusWndProc() %s returning %lX\n", DclWinMessageName(nMsg), lResult));

    return lResult;
}


            /*-------------------------------------------------*\
             *                                                 *
             *                  Helper Functions               *
             *                                                 *
            \*-------------------------------------------------*/


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static UINT GetLinesPerPage(
    HWND            hWnd,
    VIEWINSTEX     *pVIX)
{
    RECT            rect;

    DclAssertWritePtr(pVIX, sizeof(*pVIX));
    DclAssert(pVIX->hWndPage == hWnd);

    if(!pVIX->pVI->uTextHeight)
        pVIX->pVI->uTextHeight = GetFontHeight(pVIX, hWnd, 0);

    DclAssert(pVIX->pVI->uTextHeight);

    GetClientRect(hWnd, &rect);

    return (rect.bottom - rect.top) / pVIX->pVI->uTextHeight;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static LONG GetFontHeight(
    VIEWINSTEX     *pVIX,
    HWND            hWnd,
    HFONT           hFont)
{
    HDC             hDC;
    HFONT           hOldFont = 0; /* Init'ed for a picky compiler only */
    TEXTMETRIC      tm;
/*    short           cxCaps; */
    SIZE            sz;

    DclAssertWritePtr(pVIX, sizeof(*pVIX));

    /*  get a temporary DC and select our desired font
    */
    hDC = GetDC(hWnd);

    if(hFont)
        hOldFont = SelectObject(hDC, hFont);

    GetTextMetrics(hDC, &tm);

    pVIX->uCharWidth = tm.tmAveCharWidth;
/*    cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * pVI->pVIX->uCharWidth / 2; */
    pVIX->uCharHeight = tm.tmHeight + tm.tmExternalLeading;

    GetTextExtentPoint32(hDC, TEXT("A"), 1, &sz);

    DclAssert(sz.cy == (int)pVIX->uCharHeight);

    /*  restore old font
    */
    if(hFont)
        SelectObject(hDC, hOldFont);

    ReleaseDC(hWnd, hDC);

    return sz.cy;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static LONG GetFontHeight2(
    HWND            hWnd,
    HFONT           hFont)
{
    HDC             hDC;
    HFONT           hOldFont = 0; /* Init'ed for a picky compiler only */
    SIZE            sz;

    /*  get a temporary DC and select our desired font
    */
    hDC = GetDC(hWnd);

    if(hFont)
        hOldFont = SelectObject(hDC, hFont);

    GetTextExtentPoint32(hDC, TEXT("A"), 1, &sz);

    /*  restore old font
    */
    if(hFont)
        SelectObject(hDC, hOldFont);

    ReleaseDC(hWnd, hDC);

    return sz.cy;
}


/*-------------------------------------------------------------------
 -------------------------------------------------------------------*/
static void RecalcScrollbars(
    VIEWINSTEX     *pVIX)
{
    VIEWINST       *pVI = pVIX->pVI;
        
    pVI->uLinesPerPage = GetLinesPerPage(pVIX->hWndPage, pVIX);

    DclAssert(pVIX->uCharHeight);
    DclAssert(pVIX->uCharWidth);

    pVIX->uDisplayCharsPerLine = pVI->nPageWidth / pVIX->uCharWidth;

    if(pVIX->hVScrollbar)
    {
        DclWinScrollbarSetMetrics(pVIX->hVScrollbar, pVI->ulLSACountUsed, pVI->uLinesPerPage, TRUE);

        if(pVI->ulLSACountUsed <= pVI->uLinesPerPage)
            pVI->ulTopLine = 1;
    }

    if(pVIX->hHScrollbar)
    {
        DclWinScrollbarSetMetrics(pVIX->hHScrollbar, pVI->uMaxDisplayLineLength, pVIX->uDisplayCharsPerLine, TRUE);

        if(pVI->uMaxDisplayLineLength <= pVIX->uDisplayCharsPerLine)
            pVI->uHorizOffset = 0;
    }

    return;
}


/*-------------------------------------------------------------------
    Size the main frame window
-------------------------------------------------------------------*/
static void ViewSizeWindow(
    VIEWINSTEX     *pVIX,
    HWND            hWnd,
    int             cx,
    int             cy)
{
    int             cyStatus = 0;
    unsigned        nReservedY;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "ViewObj:Size hWnd=%P cX=%d cY=%d\n", hWnd, cx, cy));

    DclAssertWritePtr(pVIX, sizeof(*pVIX));
    DclAssert(pVIX->hWndPage);
/*    DclAssert(pVIX->hWndPage == hWnd); */

    if(pVIX->hWndStatus)
    {
        DclAssert(pVIX->hWndStatus);

        cyStatus = GetFontHeight2(pVIX->hWndStatus, 0) + 3;

        MoveWindow(pVIX->hWndStatus, 0, cy - cyStatus, cx, cyStatus, FALSE);
    }

    nReservedY = SendMessage(pVIX->hWndParent, WM_WINVIEW_RESERVEDY, 0, 0);

    MoveWindow(pVIX->hWndPage, -1, nReservedY - 1,
        cx + 2, (cy - cyStatus - nReservedY) + 1, FALSE);

    return;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static VIEWINSTEX * AddViewObjectInstance(void)
{
    VIEWINSTEX *pVIX;

    pVIX = DclMemAllocZero(sizeof(VIEWINSTEX));
    DclAssert(pVIX);

    if(pHeadVIX)
    {
        pVIX->pNext = pHeadVIX;

        pHeadVIX->pPrev = pVIX;
    }

    pHeadVIX = pVIX;

    return pVIX;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void DeleteViewObjectInstance(
    VIEWINSTEX     *pVIX)
{
    VIEWINSTEX     *pThisVIX;

    DclAssertWritePtr(pVIX, sizeof(*pVIX));

    pThisVIX = pHeadVIX;

    do
    {
        DclAssert(pThisVIX);

        if(pThisVIX == pVIX)
        {
            if(pVIX->pPrev)
                (pVIX->pPrev)->pNext = pVIX->pNext;
            else
                pHeadVIX = pVIX->pNext;

            if(pVIX->pNext)
                (pVIX->pNext)->pPrev = pVIX->pPrev;

            DclMemFree(pVIX);

            return;
        }

        pThisVIX = pThisVIX->pNext;

    } while(pThisVIX);

    return;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static VIEWINSTEX * LookupViewObjectFromHView(
    HVIEW           hView)
{
    VIEWINSTEX     *pVIX;

    DclAssert(hView);

    pVIX = pHeadVIX;

    while(pVIX)
    {
        DclAssert(pVIX);

        if(pVIX->pVI == (VIEWINST*)hView)
        {
            return pVIX;
        }

        pVIX = pVIX->pNext;
    }

    return 0;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static VIEWINSTEX * LookupViewObjectFromHWnd(
    HWND            hWnd)
{
    VIEWINSTEX     *pVIX;

    DclAssert(hWnd);

    pVIX = pHeadVIX;

    while(pVIX)
    {
        DclAssert(pVIX);

        if(pVIX->hWndParent == hWnd)
            return pVIX;

        if(pVIX->hWndPage == hWnd)
            return pVIX;

        if(pVIX->hWndStatus == hWnd)
            return pVIX;

        pVIX = pVIX->pNext;
    }

    return 0;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static ULONG GetKey(
    VIEWINSTEX     *pVIX)
{
    ULONG           ulKey;

    DclAssertWritePtr(pVIX, sizeof(*pVIX));
    DclAssert(pVIX->ulFlags & VIEWFLAGS_INPUT);
    DclAssert(pVIX->pulKeyBuff);

    /*  If the head and tail buffer indices are equal, the buffer is empty
    */
    if(pVIX->nKeyBuffHead == pVIX->nKeyBuffTail)
        return 0;

    ulKey = pVIX->pulKeyBuff[pVIX->nKeyBuffHead];

    if(pVIX->nKeyBuffHead == KEYBUFF_SIZE-1)
        pVIX->nKeyBuffHead = 0;
    else
        pVIX->nKeyBuffHead++;

    return ulKey;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static BOOL PutKey(
    VIEWINSTEX     *pVIX,
    ULONG           ulKey)
{
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "ViewObj:PutKey Key=%lX\n", ulKey));

    DclAssertWritePtr(pVIX, sizeof(*pVIX));
    DclAssert(pVIX->ulFlags & VIEWFLAGS_INPUT);
    DclAssert(pVIX->pulKeyBuff);

    pVIX->pVI->fNewLineFocus = TRUE;

    /*  Fail if the buffer is full
    */
    if((pVIX->nKeyBuffHead == pVIX->nKeyBuffTail-1) ||
       ((pVIX->nKeyBuffTail == KEYBUFF_SIZE-1) && (pVIX->nKeyBuffHead == 0)))
    {
        return FALSE;
    }

    pVIX->pulKeyBuff[pVIX->nKeyBuffTail] = ulKey;

    if(pVIX->nKeyBuffTail == KEYBUFF_SIZE-1)
        pVIX->nKeyBuffTail = 0;
    else
        pVIX->nKeyBuffTail++;

    return TRUE;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void UpdateCaret(
    VIEWINSTEX     *pVIX,
    HWND            hWnd)
{
    VIEWINST       *pVI;

    DclAssertWritePtr(pVIX, sizeof(*pVIX));
    DclAssert(hWnd);

    pVI = pVIX->pVI;

    if(!pVIX->fHasCaret)
        return;

    if(!pVIX->fCaretVisible && pVI->fNewLineFocus && (pVIX->nState == VIEWSTATE_WAITING))
    {
        SetCaretPos(pVI->nX, pVI->nY * pVI->uTextHeight);
        ShowCaret(hWnd);

        pVIX->fCaretVisible = TRUE;

        DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
             "ViewObj:UpdateCaret() drew caret at x=%u y=%u\n",
             pVI->nX, pVI->nY * pVI->uTextHeight));
    }
    else if(pVIX->fCaretVisible)
    {
        HideCaret(hWnd);

        pVIX->fCaretVisible = FALSE;

        DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, 0),
             "ViewObj:UpdateCaret() hid caret\n"));
    }

    return;
}



/*  The DclOsHalt() function is coded in this module so it will always
    be linked in by anyone using the viewer abstraction, and forcibly
    override the default Win32 or WinCE version of this service.
*/


/*-------------------------------------------------------------------
    DclOsHalt()

    Description
        This function provides the back-end part of the DclAssert()
        functionality that halts execution.

        Depending on the OS implementation, this function may only
        halt execution of the currently executing thread.

        This function must not return.

    Parameters
        pszMessage  - A pointer to a null-terminated message string
        ulError     - The error code

    Return Value
        This function does not return.
-------------------------------------------------------------------*/
void DclOsHalt(
    const char     *pszMessage,
    D_UINT32        ulError)
{
    void(*pNull)(void) = NULL;
    char            achBuff[64];
    WCHAR           tzBuffer[256];
    unsigned        nLen;
    extern HWND ghTempWnd;

    wsprintf(tzBuffer, TEXT(D_PRODUCTNAME)TEXT(" v%S\nThread ID 0x%08lX\n\n"),
        DclVersionFormat(achBuff, sizeof(achBuff), D_PRODUCTVER, DCLSTRINGIZE(D_PRODUCTBUILD)),
        DclOsThreadID());

    nLen = wcslen(tzBuffer);

    wsprintf(tzBuffer+nLen, TEXT("Failure at \"%S-%lu\"\n\nProgram Halting..."),
        pszMessage, ulError);

    MessageBox(ghTempWnd, tzBuffer, TEXT("Error!"), MB_OK|MB_ICONEXCLAMATION/*|MB_APPLMODAL|MB_TOPMOST|MB_SETFOREGROUND*/);

  #ifdef _WIN32_WCE
    /*  We need to decide if we are better off sitting in a tight loop, or
        causing a fault as done below.  Uncertain if the fault method will
        produce desireable results in all platforms...
    */
    while(TRUE)
    {
        DclOsSleep(1);
    }
  #else
    /*  Terminate the application
    */
    exit(ulError);
  #endif

    /*  We should not ever get to get to this point, but if we do,
        cause a fault.
    */
    pNull();
}



#ifndef _WIN32_WCE

/*  The DclOsDebugBreak() function is coded in this module so it will
    always be linked in by anyone using the viewer abstraction, and
    forcibly override the default Win32 versions of this services.
    We don't bother with CE, because we don't want any different
    functionality in CE than what is in the default implementation.
*/

/*-------------------------------------------------------------------
    DclOsDebugBreak()

    Description
        This function provides a generalized ability to support
        primitive operations for entering a debugger.

        The available debug commands values are:

        DCLDBGCMD_BREAK
          Immediately enter the debugger.  If this capability is
          supported, and execution returns from this function, the
          value DCLSTAT_SUCCESS will be returned.  If this function
          is not supported, DCLSTAT_FAILURE will be returned.

        DCLDBGCMD_QUERY
          Ask the user if he wants to enter the debugger.  Returns
          DCLSTAT_SUCCESS if the user chooses to enter the debugger
          and then continues execution.  If the user chooses not to
          enter the debugger, or if this capability is not supported,
          the value DCLSTAT_FAILURE is returned.

        DCLDBGCMD_AUTOQUERY
          Same as DCLDBGCMD_QUERY, however the user must respond
          within 10 seconds, or the default action of NOT entering
          the debugger will be taken.

        DCLDBGCMD_CAPABILITIES
          Report the capabilities of this function.  This is a bit-
          mapped flag which may be combined with one of the other
          flags to query the capabilities of this function.  If this
          function supports the functionality represented by the
          specified flag, DCLSTAT_SUCCESS is returned, otherwise
          DCLSTAT_FAILURE is returned.

    Parameters
        ulDebugCmd - The bit-mapped debug command flags, as described
                     above.

    Return Value
        Returns a DCLSTATUS value as described above.
-------------------------------------------------------------------*/
DCLSTATUS DclOsDebugBreak(
    D_UINT32        ulDebugCmd)
{
    switch(ulDebugCmd)
    {
        case DCLDBGCMD_CAPABILITIES | DCLDBGCMD_BREAK:
        case DCLDBGCMD_CAPABILITIES | DCLDBGCMD_QUERY:
            return DCLSTAT_SUCCESS;

        case DCLDBGCMD_BREAK:
            __asm int 3;
            return DCLSTAT_SUCCESS;

        case DCLDBGCMD_QUERY:
        {
            int     iRet;

            iRet = MessageBox(NULL, TEXT("A serious program error has occurred.\nDo you want to try to invoke a debugger?"),
                TEXT("Assertion Failure!"), MB_YESNO|MB_ICONEXCLAMATION|MB_APPLMODAL|MB_TOPMOST);

            if(iRet == IDYES)
            {
                __asm int 3;
                return DCLSTAT_SUCCESS;
            }

            return DCLSTAT_FAILURE;
        }

        case DCLDBGCMD_CAPABILITIES | DCLDBGCMD_AUTOQUERY:
            return DCLSTAT_FAILURE;

        default:
            return DCLSTAT_UNSUPPORTEDCOMMAND;
    }
}







#endif


