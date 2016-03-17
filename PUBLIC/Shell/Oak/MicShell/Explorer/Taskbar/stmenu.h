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
/*---------------------------------------------------------------------------*\
 *  module: stmenu.h
 *
 *  purpose:
 *
 * Notes:
 *
 *		menu id:
 *				0					- separator
 *				1-IDM_START_LAST_BANNER	- items on the first level (w/ the bitmap)
 *			    IDM_STARTLAST-60k	- standard menu items
 *				>60k				- pidl based item (FileMenu item)
 *
\*---------------------------------------------------------------------------*/
#ifndef __STMENU_H__
#define __STMENU_H__

/////////////////////////////////////////////////////////////////////////////

typedef struct tagSTMENUDATA {
	union {
		int idMenuCmd;			// start menu command
		LPITEMIDLIST pidl;		// file menu pidl
	};
	union {
		UINT idszMenuItem;		// start menu string id
		DWORD dwAttributes;		// file menu pidl attributes
	};
	int idMenuIcon;				// start menu icon resource id
	int nSpecialFolder;			// start menu special folder id

	//
	TCHAR *szDisplay;
	int width;
	int height;
} STMENUDATA, FAR *LPSTMENUDATA;

// +++Start: Stuff for control-panel expansion in Start Menu
#define IDC_MAXSTRINGLEN 32

typedef struct tagCONTROLPANELINFO
{
	TCHAR szAppletName[IDC_MAXSTRINGLEN];
	TCHAR szDesc[ MAX_PATH ];           // description
	TCHAR szDllName[ MAX_PATH ];		// full path name
	DWORD nApplet;						// applet index within the cpl
    HICON hIcon;                        // hold the small icon handle
    HMODULE hInst;                      // hold the handle so we can FreeLibrary later
    LPSTMENUDATA lpMenuData;            // pointer to the menu-item which references this data
} CONTROLPANELINFO, *LPCONTROLPANELINFO;

#define CPH_CMD_ENUMERATE 0 // Tell our ControlPanelHelper to enumerate
#define CPH_CMD_FILLARRAY 1 // Tell our ControlPanelHelper to fill our array
// ---End: Stuff for control-panel expansion in Start Menu

BOOL  StartMenu_Track(UINT uFlags, int x, int y, HWND hwnd);
int   StartMenu_MeasureItem(MEASUREITEMSTRUCT *lpmis);
int   StartMenu_DrawMenuItem(DRAWITEMSTRUCT *lpdis);
int   StartMenu_InitPopup(HMENU hmenu);
DWORD StartMenu_ProcessCommand(HWND hwnd, WPARAM wp, LPARAM lp);
DWORD StartMenu_MenuChar(HMENU hmenu, TCHAR cChar);
void  StartMenu_Destroy();

/////////////////////////////////////////////////////////////////////////////


#endif /* __STMENU_H__ */

