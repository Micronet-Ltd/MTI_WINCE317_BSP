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

#ifndef _TASKBAR_HELPERS_H_
#define _TASKBAR_HELPERS_H_

// these are non-class functions used by the taskbar and other classes

BOOL  ExecSilent(HWND hwnd, LPTSTR pszExe, LPTSTR pszArgs);
BOOL  IsOKToKill(HWND hwndApp);
void  FAR PASCAL InitDitherBrush();
BOOL  CenterWindow(HWND hwndChild, HWND hwndParent);
BOOL  TaskbarNeedsEllipses(HDC hdc, LPCTSTR pszText, LPRECT prc, int FAR* pcchDraw, int cxEllipses);
BOOL  IsSmallScreen();
HFONT GetTBFont(int size);

#define BG_UP 1
#define BG_DOWN 2
#define BG_LEFT 3
#define BG_RIGHT 4

void DrawRect(HDC hdc, HBRUSH hbr, int x1, int y1, int x2, int y2);
void DrawArrow(HDC hdc, HBRUSH hbr, int dir, const RECT &rcBounds);


#define CCHELLIPSES 3
extern TCHAR const c_szEllipses[];

#endif //_TASKBAR_HELPERS_H_
