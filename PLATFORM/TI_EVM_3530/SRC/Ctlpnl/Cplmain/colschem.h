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
/*-----------------------------------------------------------------------
--
--  Color Scheme Page
--
--
--  scheme.c
--
-------------------------------------------------------------------------*/

#ifndef __SCHEME_H
#define __SCHEME_H

#define COLOR_MAX C_SYS_COLOR_TYPES

typedef struct
{
    TCHAR    szCurrentScheme[MAX_PATH];
    COLORREF argb[COLOR_MAX];
    HWND	 hwndPreview;
    CHOOSECOLOR cc;
	COLORREF g_rgCustColors[16];
	BOOL g_fFullControl;

} SCHEMEDIALOG, *PSCHEMEDIALOG;

void SchemeDialog_Init(HWND hwndDlg);
void SchemeDialog_Terminate(HWND hwndDlg);
void SchemeDialog_ChangeScheme(HWND hwndDlg, HWND hwndSchemes);
void SchemeDialog_ApplyScheme(HWND hwndDlg);
void SchemeDialog_PickColor(HWND hwndDlg);
void SchemeDialog_SaveScheme(HWND hwndDlg);
void SchemeDialog_DeleteScheme(HWND hwndDlg);
void SchemeDialog_DrawColorBox(HWND hwndDlg, DRAWITEMSTRUCT *pdis);

COLORREF CurrentItemColor(HWND hwndDlg, int *pnSysColor);
void Update3DColors(COLORREF crObject);
DWORD FAR PASCAL AdjustLuma(DWORD rgb, int n, BOOL fScale);

// The scheme data format used in WinCE is different from that used on
// NT/Win95.
#define SCHEME_VERSION_WINCE -1
typedef struct 
{
    SHORT nVersion;
    // for alignment
    WORD  wDummy;
    COLORREF argb[COLOR_MAX];
} SCHEMEDATA, *PSCHEMEDATA;

BOOL Scheme_Init(HWND hwndSchemes, LPTSTR pszCurrentScheme);
BOOL Scheme_ReadScheme(LPCTSTR pszScheme, COLORREF argb[]);
void Scheme_WriteCurrentScheme(LPCTSTR pszSchemeName);

#define PCM_DRAWBITMAP    WM_USER
#define PCM_SETCOLORS     WM_USER+1

BOOL Preview_RegisterClass(HINSTANCE hinst);

typedef enum 
{
    FONT_FIRST = 0,
    FONT_MENU = 0,
    FONT_SYSTEM_BOLD,
    FONT_MSGBOX,
    FONT_LAST = FONT_MSGBOX 
} FONT_TYPE;

typedef enum // Elements must be ordered by painting z-order (back to front)
{
    ELEMENT_FIRST = 0,
    ELEMENT_DESKTOP = 0,
    ELEMENT_APPWINDOW,
    ELEMENT_APPBANDS,
    ELEMENT_MENU,
    ELEMENT_MENUNORMAL,
    ELEMENT_MENUDISABLED,
    ELEMENT_MENUSELECTED,
    ELEMENT_APPBUTTONS,
    ELEMENT_APPADORNMENTS,
    ELEMENT_SCROLLBAR,
    ELEMENT_SCROLLUP,
    ELEMENT_SCROLLDOWN,
    ELEMENT_INACTIVEWINDOW,
    ELEMENT_INACTIVECAPTION,
	ELEMENT_INACTIVETITLE,
    ELEMENT_INACTIVEADORNMENTS,
    ELEMENT_ACTIVEWINDOW,
    ELEMENT_ACTIVECAPTION,
	ELEMENT_ACTIVETITLE,
    ELEMENT_ACTIVEADORNMENTS,
    ELEMENT_WINDOWTEXT,
    ELEMENT_MESSAGETEXT,
    ELEMENT_BUTTON,
    ELEMENT_LAST = ELEMENT_BUTTON 
} ELEMENT_TYPE;

typedef struct 
{
    HFONT hfont;
    LOGFONT lf;
} PREVIEW_FONT, *PPREVIEW_FONT;

typedef struct _PREVIEW
{
   HDC          hdcMem;     // Memory DC for PREVIEW.hbm.
   HBITMAP      hbm;        // Pre-drawn bitmap. Allows faster painting.
   short        cColors;    // Number of elements of prgb array.
   COLORREF     *prgb;      // Array of colors used in drawing preview.
							// In order of COLOR_ constants.
   PREVIEW_FONT ppvwfont[FONT_LAST+1]; // Fonts used in drawing preview.
	 								   // In order of FONT_TYPE enum.
   RECT          prc[ELEMENT_LAST+1];  // Rects of elements. In order of ELEMENT_TYPE enum.
   int			 rgSysColor[ELEMENT_LAST + 1]; // SysColor corresponding to element
   HBRUSH        hbrLightGrayDither;   // Brush to draw scroll track
   HIMAGELIST    himl;      // Image list with images for toolbar buttons and adornments

	IMAGELISTDRAWPARAMS imldp;
	POINT pt;
	int nLastItem;

} PREVIEW, *PPREVIEW;

// Macros to access PREVIEW structure members
#define RC(n) (g_ppvw->prc[n])
#define SC(n) (g_ppvw->rgSysColor[n])
#define FONT(n) (g_ppvw->ppvwfont[n].hfont)
#define LF(n) (g_ppvw->ppvwfont[n].lf)
#define COLOR(n) (g_ppvw->prgb[n & ~SYS_COLOR_INDEX_FLAG])

// Preview_DrawBorder flags
#define PDB_LEFT   0x1
#define PDB_RIGHT  0x2
#define PDB_TOP    0x4
#define PDB_BOTTOM 0x8
#define PDB_ALL (PDB_LEFT | PDB_RIGHT | PDB_TOP | PDB_BOTTOM)

// From commctrl's toolbar.c
#define TOOLBARBUTTONX      23
#define TOOLBARBUTTONY      22
#define TOOLBARSEPARATORX   12

// From GWE
#define BUTTONX             21
#define BUTTONY             21
#define SEPARATORX           4

// Size of images in resource file
#define IMAGEX 16
#define IMAGEY 16

// Order of images in image list
#define NEW    0
#define OPEN   1
#define SAVE   2
#define HELP   3
#define CLOSE  4
#define OK     5

// Image offsets on button faces
#define TOOLBARIMAGE_OFFSETX ((TOOLBARBUTTONX-IMAGEX)/2)
#define TOOLBARIMAGE_OFFSETY ((TOOLBARBUTTONY-IMAGEY)/2)
#define IMAGE_OFFSETX        ((BUTTONX-IMAGEX)/2)
#define IMAGE_OFFSETY        ((BUTTONY-IMAGEY)/2)

LRESULT CALLBACK PreviewWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL Preview_Create(HWND hwnd);
BOOL Preview_CreateFonts(HWND hwnd);
HBRUSH Preview_CreateLightGrayDitherBrush();
void Preview_Destroy(HWND hwnd);
void Preview_Paint(HWND hwnd);
void Preview_ShowBitmap(HWND hwnd, HDC hdc);
void Preview_Draw(HWND hwnd, HDC hdc);
void Preview_DrawBitmap(HWND hwnd);
void Preview_DrawBlankButton(HDC hdc, 
                             int x, int y, int cx, int cy, 
                             short cxBorder, short cyBorder);
void Preview_DrawImageButton(HDC hdc, int iImage, int x, int y);
void Preview_DrawBorder(HDC hdc, LPRECT prc, 
                             short cxWidth, short cyWidth, 
                             HBRUSH hbrTL, HBRUSH hbrBR, UINT uFlags);
void Preview_DrawArrow(HDC hdc, int x, int y, int cWidth, int sign);
void Preview_CalcElementRects(HWND hwnd);
void Preview_LButtonDown(HWND hwnd, POINT pt);
     
#endif /* __SCHEME_H */
