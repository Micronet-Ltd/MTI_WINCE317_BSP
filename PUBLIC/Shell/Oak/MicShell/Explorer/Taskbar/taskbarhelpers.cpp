//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//

#include <windows.h>
#include "debugzones.h"
#include "taskbarhelpers.h"

extern HBRUSH g_hbrMonoDither;

BOOL ExecSilent(HWND hwnd, LPTSTR pszExe, LPTSTR pszArgs)
{
   SHELLEXECUTEINFO sei;
   sei.cbSize = sizeof(sei);
   sei.fMask = SEE_MASK_FLAG_NO_UI;
   sei.hwnd = hwnd;
   sei.lpVerb = NULL;
   sei.lpFile = pszExe;
   sei.lpParameters = pszArgs;
   sei.lpDirectory = NULL;
   sei.nShow = SW_SHOWNORMAL;

   return ShellExecuteEx(&sei);
}

// If the user manages to kill one of these system processes, the system will
// crash, so disallow it. This situation can happen if, say, a device driver
// in device.exe puts up a dialog box & the user tries to kill it
//
// Forbidden processes list. Must be all lower-case
LPTSTR rgSystemProcesses[] = { L"device.exe", L"gwes.exe", L"nk.exe", L"filesys.exe", L"services.exe", 0 };

// Check if a window belongs to a system process
BOOL IsOKToKill(HWND hwndApp)
{
    HANDLE hProc;
    WCHAR wszBuf[MAX_PATH];
    int i;
    DWORD dwProcessId;

    // Get the window's owning process
    GetWindowThreadProcessId(hwndApp, &dwProcessId);
    // Open the process
    hProc = OpenProcess(PROCESS_ALL_ACCESS,0,dwProcessId);
    wszBuf[0] = 0;
    // Get it's filename
    GetModuleFileName((HINSTANCE)hProc, wszBuf, MAX_PATH);
    wszBuf[MAX_PATH-1]=0;

    CloseHandle(hProc);
    // lowercase the filename so substring match will suceed
    wcslwr(wszBuf);

    DEBUGMSG(ZONE_VERBOSE, (L"Trying to kill process %s\r\n", wszBuf));

    // Check if the process is in the list of system processes
    for(i=0; rgSystemProcesses[i]; i++)
    {
        // Note: we use a substring match so path effects are eliminated
        if (wcsstr(wszBuf, rgSystemProcesses[i]))
        {
            DEBUGMSG(ZONE_FATAL, (L"Cannot kill system process %s\r\n", wszBuf));
            return FALSE;
        }
    }
    return TRUE;
}

typedef struct tagDIB {
    BITMAPINFOHEADER    bi;
    DWORD               colors[2];
    DWORD               biBits[8];
} DIB;


void FAR PASCAL InitDitherBrush()
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    DIB dib;

    dib.bi.biSize = sizeof(BITMAPINFOHEADER);
    dib.bi.biWidth = 8;
    dib.bi.biHeight = 8;
    dib.bi.biPlanes = 1;
    dib.bi.biBitCount = 1;
    dib.bi.biCompression = BI_RGB;
    dib.bi.biSizeImage = 0;
    dib.bi.biXPelsPerMeter = 0;
    dib.bi.biYPelsPerMeter = 0;
    dib.bi.biClrUsed = 2;
    dib.bi.biClrImportant = 2;
    dib.colors[0] = RGB(0x0, 0x0, 0x0);
    dib.colors[1] = RGB(0xff, 0xff, 0xff);
    dib.biBits[0] = 0x55555555;
    dib.biBits[1] = 0xAAAAAAAA;
    dib.biBits[2] = 0x55555555;
    dib.biBits[3] = 0xAAAAAAAA;
    dib.biBits[4] = 0x55555555;
    dib.biBits[5] = 0xAAAAAAAA;
    dib.biBits[6] = 0x55555555;
    dib.biBits[7] = 0xAAAAAAAA;

    g_hbrMonoDither = CreateDIBPatternBrushPt( &dib, DIB_RGB_COLORS );

} /* InitDitherBrush()
   */


BOOL CenterWindow(HWND hwndChild, HWND hwndParent)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    RECT    rcChild, rcParent;
    int     cxChild, cyChild, cxParent, cyParent;
    int     cxScreen, cyScreen, xNew, yNew;
    HDC     hdc;

     // if parent is NULL, center it at the primary display!!
    if (!hwndParent) {
        rcParent.left = rcParent.top = 0; // primary display is always @ (0,0)
        rcParent.right = GetSystemMetrics(SM_CXSCREEN);
        rcParent.bottom = GetSystemMetrics(SM_CYSCREEN);
    }
    else
    {
        GetWindowRect(hwndParent, &rcParent);
    }

    // Get the Height and Width of the parent window
    cxParent = rcParent.right - rcParent.left;
    cyParent = rcParent.bottom - rcParent.top;

    // Get the Height and Width of the child window
    GetWindowRect(hwndChild, &rcChild);
    cxChild = rcChild.right - rcChild.left;
    cyChild = rcChild.bottom - rcChild.top;

    // Get the display limits
    hdc = GetDC(hwndChild);
    cxScreen = GetDeviceCaps(hdc, HORZRES);
    cyScreen = GetDeviceCaps(hdc, VERTRES);
    ReleaseDC(hwndChild, hdc);

    // Calculate new X position, then adjust for screen
    xNew = rcParent.left + ((cxParent - cxChild) / 2);
    if (xNew < 0)
    {
        xNew = 0;
    }
    else if ((xNew + cxChild) > cxScreen)
    {
        xNew = cxScreen - cxChild;
    }

    // Calculate new Y position, then adjust for screen
    yNew = rcParent.top  + ((cyParent - cyChild) / 2);
    if (yNew < 0)
    {
        yNew = 0;
    }
    else if ((yNew + cyChild) > cyScreen)
    {
        yNew = cyScreen - cyChild;
    }

    // Set it, and return
    return SetWindowPos(hwndChild,
                        NULL,
                        xNew, yNew,
                        0, 0,
                        SWP_NOSIZE | SWP_NOZORDER);
} /* CenterWindow()
   */

BOOL TaskbarNeedsEllipses(HDC hdc, LPCTSTR pszText, LPRECT prc, int FAR* pcchDraw, int cxEllipses)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    int cchText;
    int cxRect;
    int ichMin, ichMax, ichMid;
    SIZE siz;

    cxRect = prc->right - prc->left;

    cchText = lstrlen(pszText);

    if (cchText == 0) {
    *pcchDraw = cchText;
    return FALSE;
    }

    GetTextExtentPoint(hdc, pszText, cchText, &siz);

    if (siz.cx <= cxRect) {
    *pcchDraw = cchText;
    return FALSE;
    }

    cxRect -= cxEllipses;

    ichMax = 1;
    if (cxRect > 0) {

        ichMin = 0;
    ichMax = cchText;

        while (ichMin < ichMax) {
        ichMid = (ichMin + ichMax + 1) / 2;

        GetTextExtentPoint(hdc, &pszText[ichMin], ichMid - ichMin, &siz);

        if (siz.cx < cxRect) {
        ichMin = ichMid;
        cxRect -= siz.cx;

        }else if (siz.cx > cxRect) {
        ichMax = ichMid - 1;

            }else{
                ichMax = ichMid;
                break;
        }
    }

        if (ichMax < 1)
        ichMax = 1;
    }

    *pcchDraw = ichMax;
    return TRUE;

} /* TaskbarNeedsEllipses()
   */

void
DrawRect (
    HDC hdc, HBRUSH hbr, int x1, int y1, int x2, int y2 )
{
    if (x2 > x1 && y2 > y1)
    {
        HBRUSH hbrOld = (HBRUSH)SelectObject(hdc, hbr);
        PatBlt( hdc, x1, y1, x2 - x1, y2 - y1, PATCOPY );
        SelectObject(hdc, hbrOld);
    }
}


void
DrawArrow (
    HDC hdc, HBRUSH hbr, int dir,
    const RECT &rcBounds )
{

    ASSERT( rcBounds.right  > rcBounds.left );
    ASSERT( rcBounds.bottom > rcBounds.top );

#if 0 // not used

    Assert(
        dir == BG_UP || dir == BG_DOWN || dir == BG_LEFT || dir == BG_RIGHT );

    if (dir == BG_UP || dir == BG_DOWN)
    {
            //
            // Determine the height of the arrow by computing the largest
            // arrows we allow for the given width and height, and then taking
            // the smaller of the two.  Also make sure it is non zero.
            //

        long arrow_height =
            max(
                (long)1,
                (long)min(
                    (rcBounds.bottom - rcBounds.top + 2) / 3,
                    ((rcBounds.right - rcBounds.left) * 5 / 8 + 1) / 2 ) );

            //
            // Locate where the top of the arrow starts and where it is
            // centered horizontally
            //

        long sy =
            rcBounds.top +
                (rcBounds.bottom - rcBounds.top + 1 - arrow_height) / 2;

        long cx =
            rcBounds.left + (rcBounds.right - rcBounds.left - 1) / 2;

            //
            // Draw the arrow from top to bottom in successive strips
            //

        for ( i = 0 ; i < arrow_height ; i++ )
        {
            long y = dir == BG_UP ? sy + i : sy + arrow_height - i - 1;

            DrawRect( hdc, hbr, cx - i, y, cx - i + 1 + i * 2, y + 1 );
        }
    }
    else
#else
    if (dir == BG_LEFT || dir == BG_RIGHT)
#endif
    {
            //
            // Determine the width of the arrow by computing the largest
            // arrows we allow for the given width and height, and then taking
            // the smaller of the two.  Also make sure it iz non zero.
            //

        long arrow_width =
                max(
                    (long)1,
                    (long)min(
                        (rcBounds.right - rcBounds.left),
                        ((rcBounds.bottom - rcBounds.top) * 5 / 8 + 1) / 2 ) );

            //
            // Locate where the left of the arrow starts and where it is
            // centered vertically
            //

        long sx =
            rcBounds.left +
                (rcBounds.right - rcBounds.left + 1 - arrow_width) / 2;

        long cy =
            rcBounds.top + (rcBounds.bottom - rcBounds.top) / 2;

            //
            // Draw the arrow from top to bottom in successive strips
            //

        for ( long i = 0 ; i < arrow_width ; i++ )
        {
            long x = dir == BG_LEFT ? sx + i : sx + arrow_width - i - 1;

            DrawRect( hdc, hbr, x, cy - i, x + 1, cy + i + 1 );
        }
    }
}

BOOL IsSmallScreen()
{
    HKEY hkey;
    LONG l;
    DWORD dw = 0;
    DWORD dwSize = sizeof(DWORD);
    BOOL returnVal = FALSE;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Explorer"), 0,0,&hkey))
    {
        l = RegQueryValueEx(hkey, TEXT("QVGA"), NULL, NULL, (LPBYTE) &dw, &dwSize);
        if ( l == ERROR_SUCCESS )
        {
            returnVal = ( dw != 0 );
        }
        RegCloseKey( hkey );
    }

    return returnVal;
}

HFONT GetTBFont(int size)
{
    LOGFONTW lf;

    memset(&lf, 0, sizeof(LOGFONTW));
    lf.lfHeight = size;
    lf.lfWeight = FW_BOLD;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = FF_SWISS;

    return CreateFontIndirect(&lf);
}

