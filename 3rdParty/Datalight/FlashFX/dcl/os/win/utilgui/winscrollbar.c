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

    This module contains the Win32 scrollbar abstraction.

    This code must be fully compilable and usable under both normal Win32
    user mode, as well as Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: winscrollbar.c $
    Revision 1.3  2010/11/01 04:18:59Z  garyp
    Modified DclWinScrollbarSetPosition() to allow nPos to remain unchanged.
    Revision 1.2  2009/06/28 02:46:24Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.3  2008/11/05 01:20:47Z  garyp
    Documentation updated.
    Revision 1.1.1.2  2008/09/04 17:05:55Z  garyp
    Changed a number of memset() calls to use DclMemSet().
    Revision 1.1  2008/07/20 05:51:18Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <string.h>
#include <limits.h>
#include <commdlg.h>

#include <dcl.h>
#include <dlwinutil.h>
#include <winscrollapi.h>

#define WHEELMOUSE_LINES    (3)


typedef struct
{
    HWND        hWnd;
    UINT        uBarType;
    ULONG       ulFlags;
    unsigned    nItemCount;
    unsigned    nPosition;
    unsigned    nMaxPosition;
    unsigned    nItemsPerPage;
} SCROLLBARDATA;


            /*-------------------------------------------------*\
             *                                                 *
             *                 Public Interface                *
             *                                                 *
            \*-------------------------------------------------*/


/*-------------------------------------------------------------------
    Public: DclWinScrollbarObjectCreate()

    Create a scrollbar object for the specified window.

    Parameters:
        hWndParent - The window handle
        uBarType   - The standard Windows bar type, which is usually
                     SB_VERT or SB_HORZ.
        ulFlags    - Reserved, should be 0.

    Return Value:
        Returns an HSCROLLBAR handle if successful, or NULL
        otherwise.
-------------------------------------------------------------------*/
HSCROLLBAR WINAPI DclWinScrollbarObjectCreate(
    HWND            hWndParent,
    UINT            uBarType,
    ULONG           ulFlags)
{
    SCROLLBARDATA  *pSBD;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "DclWinScrollbarObjectCreate() hWndParent=%P Type=%u Flags=%lX\n", hWndParent, uBarType, ulFlags));

    pSBD = DclMemAllocZero(sizeof(SCROLLBARDATA));

    if(pSBD)
    {
        SCROLLINFO  si = {0};

        pSBD->hWnd       = hWndParent;
        pSBD->uBarType   = uBarType;
        pSBD->ulFlags    = ulFlags;

        si.cbSize = sizeof(si);
        si.fMask = SIF_POS | SIF_RANGE;

        SetScrollInfo(hWndParent, uBarType, &si, TRUE);
    }

    return (HSCROLLBAR)pSBD;
}


/*-------------------------------------------------------------------
    Public: DclWinScrollbarObjectDestroy()

    Destroy a scrollbar object.

    Parameters:
        hScrollbar - The scrollbar handle

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
BOOL WINAPI DclWinScrollbarObjectDestroy(
    HSCROLLBAR      hScrollbar)
{
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "DclWinScrollbarObjectDestroy() hScrollbar=%P\n", hScrollbar));

    if(!hScrollbar)
        return FALSE;

    DclMemFree(hScrollbar);

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: DclWinScrollbarSetMetrics()

    Set the geometry for a scrollbar object.

    Parameters:
        hScrollbar    - The scrollbar handle
        nItemCount    - The number of items managed.  If this value
                        is UINT_MAX, then the original value should
                        remain unchanged.
        nItemsPerPage - The number of items that fit in the current
                        view.  If this value is UINT_MAX, then the
                        original value should remain unchanged.
        fRedraw       - TRUE or FALSE to indicate whether the
                        scrollbar should be redrawn or not.

    Return Value:
        None.
-------------------------------------------------------------------*/
void WINAPI DclWinScrollbarSetMetrics(
    HSCROLLBAR      hScrollbar,
    unsigned        nItemCount,
    unsigned        nItemsPerPage,
    BOOL            fRedraw)
{
    SCROLLBARDATA  *pSBD = (SCROLLBARDATA*)hScrollbar;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "DclWinScrollbarSetMetrics() hScrollbar=%P ItemCount=%lu ItemsPerPage=%u fRedraw=%u\n",
        hScrollbar, nItemCount, nItemsPerPage, fRedraw));

    if(!pSBD)
        return;

    /*  flag to use original value?
    */
    if(nItemCount == UINT_MAX)
        nItemCount = pSBD->nItemCount;

    /*  flag to use original value?
    */
    if(nItemsPerPage == UINT_MAX)
        nItemsPerPage = pSBD->nItemsPerPage;

    DclAssert(nItemsPerPage >= 0);

    if((nItemCount != pSBD->nItemCount) ||
        (nItemsPerPage != pSBD->nItemsPerPage))
    {
        SCROLLINFO  si = {0};

        if(nItemCount)
            pSBD->nMaxPosition = nItemCount-1;
        else
            pSBD->nMaxPosition = 0;

        pSBD->nItemsPerPage   = nItemsPerPage;
        pSBD->nItemCount      = nItemCount;

        si.cbSize = sizeof(si);
        si.fMask = SIF_RANGE | SIF_PAGE;
        si.nMax = pSBD->nMaxPosition;
        si.nPage = pSBD->nItemsPerPage;

        SetScrollInfo(pSBD->hWnd, pSBD->uBarType, &si, FALSE);
    }

    if(pSBD->nPosition > pSBD->nMaxPosition)
    {
        pSBD->nPosition = pSBD->nMaxPosition;
        fRedraw = TRUE;
    }

    if(fRedraw)
        DclWinScrollbarSetPosition(hScrollbar, pSBD->nPosition, fRedraw);

    return;
}


/*-------------------------------------------------------------------
    Public: DclWinScrollbarSetPosition()

    Set the scrollbar position.

    Parameters:
        hScrollbar    - The scrollbar handle.
        nPos          - The scrollbar position, or UINT_MAX to leave
                        the position unchanged.
        fRedraw       - TRUE or FALSE to indicate whether the
                        scrollbar should be redrawn or not.

    Return Value:
        None.
-------------------------------------------------------------------*/
void WINAPI DclWinScrollbarSetPosition(
    HSCROLLBAR      hScrollbar,
    unsigned        nPos,
    BOOL            fRedraw)
{
    SCROLLBARDATA  *pSBD = (SCROLLBARDATA*)hScrollbar;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "DclWinScrollbarSetPosition() hScrollbar=%P Position=%u fRedraw=%u\n",
        hScrollbar, nPos, fRedraw));

    if(!pSBD)
        return;

    if(nPos == UINT_MAX)
        nPos = pSBD->nPosition;

    if(nPos > pSBD->nMaxPosition)
        nPos = pSBD->nMaxPosition;

    if(fRedraw || (nPos != pSBD->nPosition))
    {
        SCROLLINFO  si = {0};

        pSBD->nPosition = nPos;

        si.cbSize = sizeof(si);
        si.fMask = SIF_POS;
        si.nPos = nPos;

        SetScrollInfo(pSBD->hWnd, pSBD->uBarType, &si, TRUE);
    }

    return;
}


/*-------------------------------------------------------------------
    Public: DclWinScrollbarProcessEvent()

    Process scrollbar specific messages.

    Parameters:
        hScrollbar    - The scrollbar handle.
        nMsg          - The message number.
        wParam        - The WPARAM parameter.
        lParam        - The LPARAM parameter.

    Return Value:
        Returns the scrollbar position.
-------------------------------------------------------------------*/
unsigned WINAPI DclWinScrollbarProcessEvent(
    HSCROLLBAR      hScrollbar,
    UINT            nMsg,
    WPARAM          wParam,
    LPARAM          lParam)
{
    SCROLLBARDATA  *pSBD = (SCROLLBARDATA*)hScrollbar;
    unsigned        nNewPos;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 2, 0),
        "DclWinScrollbarProcessEvent() hScrollbar=%P %s WP=%lX LP=%lX\n",
        hScrollbar, DclWinMessageName(nMsg), wParam, lParam));

    if(!pSBD)
        return UINT_MAX;

    nNewPos = pSBD->nPosition;

    switch(nMsg)
    {
        case WM_MOUSEWHEEL:
        {
            short sDelta = GET_WHEEL_DELTA_WPARAM(wParam);

            if(sDelta > 0)
            {
                if(nNewPos > WHEELMOUSE_LINES)
                    nNewPos -= WHEELMOUSE_LINES;
                else
                    nNewPos = 0;
            }
            else if(sDelta < 0)
            {
                if(pSBD->nMaxPosition - nNewPos >= WHEELMOUSE_LINES)
                    nNewPos += WHEELMOUSE_LINES;
                else
                    nNewPos = pSBD->nMaxPosition;
            }
        }
        break;

        case WM_VSCROLL:
        case WM_HSCROLL:
        {
            switch(LOWORD(wParam))
            {
                case SB_LINEUP:             /* AKA SB_LINELEFT */
                {
                    if(nNewPos)
                        nNewPos--;
                }
                break;

                case SB_LINEDOWN:           /* AKA SB_LINERIGHT */
                {
                    if(nNewPos < pSBD->nMaxPosition)
                        nNewPos++;
                }
                break;

                case SB_PAGEUP:             /* AKA SB_PAGELEFT */
                {
                    if(nNewPos >= pSBD->nItemsPerPage)
                        nNewPos -= pSBD->nItemsPerPage;
                    else
                        nNewPos = 0;
                }
                break;

                case SB_PAGEDOWN:           /* AKA SB_PAGERIGHT */
                {
                    if(pSBD->nMaxPosition - nNewPos >= pSBD->nItemsPerPage)
                        nNewPos += pSBD->nItemsPerPage;
                    else
                        nNewPos = pSBD->nMaxPosition;
                }
                break;

                case SB_THUMBTRACK:
                {
                    SCROLLINFO  si;

                    DclMemSet(&si, 0, sizeof(si));

                    si.cbSize = sizeof(si);
                    si.fMask = SIF_TRACKPOS;

                    if(GetScrollInfo(pSBD->hWnd, pSBD->uBarType, &si))
                        nNewPos = si.nTrackPos;
                }
                break;

                case SB_THUMBPOSITION:
                case SB_ENDSCROLL:
                    break;

                default:
                    DCLPRINTF(1, ("DclWinScrollbarProcessEvent() hScrollbar=%P unsupported SB_* option %X\n",
                        hScrollbar, LOWORD(wParam)));
                    break;
            }
        }
        break;

        default:
        {
            DCLPRINTF(1, ("DclWinScrollbarProcessEvent() hScrollbar=%P unsupported message %u\n",
                hScrollbar, nMsg));
        }
        break;
    }

    if(nNewPos != pSBD->nPosition)
        DclWinScrollbarSetPosition(hScrollbar, nNewPos, TRUE);

    return pSBD->nPosition;
}

