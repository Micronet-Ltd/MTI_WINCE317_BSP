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
 *  module: stmenu.cpp
\*---------------------------------------------------------------------------*/
//#include "explorer.h"
//#include "fsmenu.h"
//#include "cpl.h"

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shtypes.h>
#include <cpl.h>

#include "stmenu.h"
#include "resource.h"
#include "taskbar.hxx"
#include "fsmenu.h"
#include "shlobj.h"
#include "utils.h"
#include "idlist.h"
#include "debugzones.h"

#ifndef _PREFAST_
#pragma warning(disable:4068)
#endif // _PREFAST_

#define MAX_TEXT_WIDTH        200
#define CX_CASCADE_WIDGET    8

// from cstrings.c
WCHAR const c_szCplAppletFunc[] = L"CPlApplet";    // applet entry proc
WCHAR const c_szWindows[] = L"\\Windows";
WCHAR const c_chBS = L'\\';
WCHAR const c_szConnMC[] = L"ConnMC.exe";
WCHAR const c_szSlash[] = L"\\";
WCHAR const c_szPegHelp[] = L"\\windows\\peghelp.exe";
WCHAR const c_szControlPanel[] = L"control.exe";
WCHAR const c_szControlPanelLauncher[] = L"ctlpnl.exe";   // used to launch sub-items

// from cstrings.h
#define CCHELLIPSES 3

extern HINSTANCE g_hInstance;
extern CTaskBar *g_TaskBar;

// command id, string, icon id, sp folder

static STMENUDATA const stmenuData[] = {
    {IDM_START_PROGRAMS, IDS_MENUPROGRAMS, IDI_STPROGS, CSIDL_PROGRAMS, 0, 0, 0},
    {IDM_START_FAVORITES, IDS_MENUFAVORITES, IDI_STFAVORITES, CSIDL_FAVORITES, 0, 0, 0},
    {IDM_START_DOCUMENTS, IDS_MENUDOCUMENTS,  IDI_STDOCS, CSIDL_RECENT, 0, 0, 0},
    {IDM_START_SETTINGS, IDS_MENUSETTING, IDI_STSETNGS, STATIC_ITEM, 0, 0, 0},
    {IDM_START_HELP, IDS_MENUHELP, IDI_STHELP, STATIC_ITEM, 0, 0, 0},
    {IDM_START_RUN, IDS_MENURUN, IDI_STRUN, STATIC_ITEM, 0, 0, 0},
    {0, IDS_NULL, 0, STATIC_ITEM, 0, 0, 0},
    {IDM_START_SUSPEND, IDS_MENUSUSPEND, IDI_STSUSPD, STATIC_ITEM, 0, 0, 0},
};

#define START_NUM_STATIC_ITEMS   (sizeof(stmenuData)/sizeof(stmenuData[0]))

// string, item id, icon id, sp folder
static STMENUDATA const settingsData[] = {
    {IDM_START_CTLPNL, IDS_MENUCTLPNL, IDI_STCTLPNL, STATIC_ITEM, 0, 0, 0},
    {IDM_START_CONNMC, IDS_MENUCONNMC, IDI_STCONNMC, STATIC_ITEM, 0, 0, 0},
    {IDM_START_TASKBAR, IDS_MENUTASKBAR, IDI_STTASKBAR, STATIC_ITEM, 0, 0, 0}
};
#define SETTINGS_NUM_STATIC_ITEMS  \
        (sizeof(settingsData)/sizeof(settingsData[0]))

static const WCHAR * const cplSkipList[] = { L"connpnl.cpl" };
#define CPL_SKIP_NUM_STATIC_ITEMS (sizeof(cplSkipList)/sizeof(*cplSkipList))

static HBITMAP hbmSTPegasus;
static SIZE sizeSTPegasus;
static RECT rcMenu; // Set this through ODA_DRAWENTIRE
static int idFirstMenu;
static int idLastMenu;

extern int g_idFileMenu;
HMENU hStartMenu;

static const DWORD dwControlPanelStartID = IDM_START_LAST_NO_BANNER + 1;
static DWORD dwControlPanelCount = 0; // Count of Control Panels to show up in Start Menu

// An array to contain our Control-Panel applet information (icon, .cpl, name, etc.)
LPCONTROLPANELINFO ControlPanels = NULL;

//=================================================================================================
// Function:   DWORD ControlPanelHelper
// Parameters: cmd - CPH_CMD_ENUMERATE (0) to return the number of control-panel applets
//                   CPH_CMD_FILLARRAY (1) to fill the array with control-panel applet info
// Returns:    DWORD value which depends on the cmd passed in.
// Notes:      Some of this code is borrowed from the control-panel code.  .cpl files are really
//             just DLLs which have a common set of entry-points.
//
// Version:   Date:    Author:                    Reason:
//   1.0   2000.02.22  Brian King                 Creation.
//=================================================================================================
static DWORD ControlPanelHelper(int cmd)
{
    WIN32_FIND_DATA fd;
    WCHAR sz[ MAX_PATH ];
    HINSTANCE hInstance = NULL;
    APPLET_PROC lpfnControlPanel = NULL;
    DWORD dwTotalCount = 0; // Total number of control-panel applets, regardless of number of .cpl files
    DWORD i = 0, index = 0;
    WORD cxSmallIcon = GetSystemMetrics(SM_CXSMICON);
    WORD cySmallIcon = GetSystemMetrics(SM_CYSMICON);

    // Let's get our first .cpl file
    HANDLE hFile = FindFirstFile(L"\\Windows\\*.cpl", &fd);

    // Find all .cpl files, and call each one's CPlApplet func with CPL_INIT, CPL_GETCOUNT, and CPL_EXIT
    if (hFile != INVALID_HANDLE_VALUE)
    {
        BOOL fSkipCPL;
        do
        {
            fSkipCPL = FALSE;
            for (int cplSkipCounter=0; cplSkipCounter < CPL_SKIP_NUM_STATIC_ITEMS; cplSkipCounter++)
            {
                if (!_tcsicmp(cplSkipList[cplSkipCounter], fd.cFileName))
                {
                    DEBUGMSG(ZONE_WARNING, (L"Did not add %s to the control panel expanded menu", fd.cFileName));
                    fSkipCPL = TRUE;
                    break;
                }
            }

            wcscpy(sz, L"\\Windows\\");
            StringCchCat(sz, ARRAYSIZE(sz), fd.cFileName);

            if ((!fSkipCPL) && (hInstance = LoadLibrary(sz)) &&
                (lpfnControlPanel = (APPLET_PROC)GetProcAddress((HMODULE)hInstance, c_szCplAppletFunc)))
            {
                // Let's gain access to the control-panel's functionality
                lpfnControlPanel(NULL, CPL_INIT, 0L, 0L);

                if (cmd == CPH_CMD_ENUMERATE)
                {
                    // We just want to return the total number of control-panel applets for all .cpl files, so let's sum them.
                    dwTotalCount += lpfnControlPanel(NULL, CPL_GETCOUNT, 0L, 0L);
                }
                else if (cmd == CPH_CMD_FILLARRAY)
                {
                    // We want to fill in our ControlPanels array...
                    if (dwControlPanelCount && (index < dwControlPanelCount) && ControlPanels)
                    {
                        // Get the count for this particular .cpl file
                        dwTotalCount = lpfnControlPanel(NULL, CPL_GETCOUNT, 0L, 0L);
                        for (i = 0; i < dwTotalCount; i++)
                        {
                            NEWCPLINFO    Newcpl;

                            Newcpl.dwSize = 0L;
                            Newcpl.dwFlags = 0L;
                            if (lpfnControlPanel(NULL, CPL_NEWINQUIRE, i, (LONG)(LPCPLINFO)&Newcpl) == -1)
                            {
                                // This is expected & harmless--results from componentization
                                DEBUGMSG(ZONE_WARNING, (L"CPL: Failed CPL_NEWINQUIRE for DLL=%s applet %d\r\n", sz, i));
                                dwControlPanelCount--; // Our total count needs to be decremented
                                continue;
                            }

                            if (Newcpl.dwSize != sizeof(NEWCPLINFO))
                            {
                                DEBUGMSG(ZONE_WARNING, (L"CPL: Incompatible struct size from '%s'\r\n", sz));
                                dwControlPanelCount--; // Our total count needs to be decremented
                                continue;
                            }

                            if (Newcpl.lData) // so a small icon can be loaded (we do not have CopyImage())
                            {
                                ControlPanels[index].hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(Newcpl.lData),
                                                                               IMAGE_ICON, cxSmallIcon, cySmallIcon, LR_DEFAULTCOLOR);
                            }
                            else
                            {
                                ControlPanels[index].hIcon = NULL;
                            }

                            ControlPanels[index].hInst = hInstance; // Module instance for later FreeLibrary() call
                            ControlPanels[index].nApplet = i; // Applet index inside of this particular .cpl file
                            lstrcpy(ControlPanels[index].szAppletName, Newcpl.szName); // Name of applet
                            lstrcpy(ControlPanels[index].szDesc, Newcpl.szInfo); // Description (useful for future Tooltip)
                            lstrcpy(ControlPanels[index].szDllName, sz); // DLL name where we got this applet from
                            ControlPanels[index].lpMenuData = NULL; // No associated menu item (yet)
                            index++;
                            lpfnControlPanel(NULL, CPL_STOP, i, Newcpl.lData);

                            DEBUGMSG(ZONE_VERBOSE, (L"Found applet='%s' desc='%s', index=%d dllpath=%s \r\n", Newcpl.szName, Newcpl.szInfo, i, sz));
                        }
                    }
                }

                // Finished with the control-panel
                lpfnControlPanel(NULL, CPL_EXIT, 0L, 0L);
                if (cmd == CPH_CMD_ENUMERATE)
                {
                    FreeLibrary(hInstance); // Free it in the CPH_CMD_ENUMERATE case only, because the CPH_CMD_FILLARRAY case needs
                                          // the library around still for the icons (until we get CopyIcon() functionality).
                }
            }
        } while (FindNextFile(hFile, &fd));
        FindClose(hFile);
    }

    return dwTotalCount;
}

//=================================================================================================
// Function:   DWORD StartMenu_FreeControlPanelInfo
// Parameters: none
// Returns:    none
// Notes:      Iterates through our control-panel information and frees the associate menu-item
//             and icon information.
//
// Version:   Date:    Author:                    Reason:
//   1.0   2000.02.22  Brian King                 Creation.
//=================================================================================================
static void StartMenu_FreeControlPanelInfo(void)
{
    DWORD i;
    HINSTANCE hInstCurrent = NULL, hInstLast = NULL;

    if (ControlPanels)
    {
        ASSERT(dwControlPanelCount > 0); // This better be true...
        hInstLast = ControlPanels[0].hInst;
        for (i = 0; i < dwControlPanelCount; i++)
        {
            DestroyIcon((HICON)ControlPanels[i].hIcon); // We did a LoadImage to get this guy
#if 0
// Don't free these here.  It looks like FileMenu_FreeMenuItems will do this by calling FileMenu_FreeData
            if (ControlPanels[i].lpMenuData)
            {
                LocalFree(ControlPanels[i].lpMenuData); // Free our menu-data
                ControlPanels[i].lpMenuData = NULL;
            }
#endif
            hInstCurrent = ControlPanels[i].hInst;
            if (hInstCurrent != hInstLast)
            {
                FreeLibrary(hInstLast);
                hInstLast = hInstCurrent;
            }
        }

        // Clean up our control-panel information
        LocalFree(ControlPanels);
        ControlPanels = NULL;
        dwControlPanelCount = 0;
    }
}

//=================================================================================================
// Function:   UINT FindNewMenuItemPosition
// Parameters: HMENU    menu handle
//               LPCWSTR    new menu item string
// Returns:    UINT for alphabetical position in menu
// Notes:
//
// Version:   Date:    Author:                    Reason:
//   1.0   2002.12.31                             Creation.
//=================================================================================================
static UINT FindNewMenuItemPosition(HMENU hMenu, LPCWSTR strNewItem, int iMenuSize = -1)
{
    // passing in the size of the menu means we don't have to compute the menu's size
    int cMenuItems = (iMenuSize > -1) ? iMenuSize : GetMenuItemCount(hMenu);
    MENUITEMINFO mii;
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_DATA | MIIM_TYPE;
    mii.fType = MFT_OWNERDRAW;
    mii.hSubMenu = NULL;


    for (int i = 0; i < cMenuItems; i++)
    {
        if (GetMenuItemInfo(hMenu, (UINT) i, TRUE, &mii))
        {
            LPSTMENUDATA pmd = (LPSTMENUDATA) mii.dwItemData;
            if (_tcsicmp(strNewItem, (LPCWSTR) pmd->idszMenuItem) < 0)
            {
                return (UINT) i;
            }
        }
    }

    return -1;
}

//=================================================================================================
// Function:   DWORD StartMenu_CreateControlPanelMenu
// Parameters: none
// Returns:    HMENU for our "Control Panel" Popup Menu
// Notes:
//
// Version:   Date:    Author:                    Reason:
//   1.0   2000.02.22  Brian King                 Creation.
//=================================================================================================
static HMENU StartMenu_CreateControlPanelMenu(void)
{
    HMENU hControlPanelMenu = NULL;
    DWORD i = 0;

    // Get the total number of control-panels in the system
    if (dwControlPanelCount = ControlPanelHelper(CPH_CMD_ENUMERATE))
    {
        DEBUGMSG(ZONE_VERBOSE, (L"CreateControlPanelMenu has %d items\n", dwControlPanelCount));

        // Allocate enough memory to hold our data for each control-panel item
        if (ControlPanels = (LPCONTROLPANELINFO)LocalAlloc(LPTR, dwControlPanelCount * sizeof(CONTROLPANELINFO)))
        {
            // Fill in our array with control-panel information
            if (ControlPanelHelper(CPH_CMD_FILLARRAY))
            {
                hControlPanelMenu = CreatePopupMenu();
                if (hControlPanelMenu)
                {
                    DWORD dwControlPanelID = dwControlPanelStartID;
                    for (i = 0; i < dwControlPanelCount; i++)
                    {
                        // Get some Start-Menu Data (STMENUDATA) storage
                        LPSTMENUDATA md = (LPSTMENUDATA)LocalAlloc(LPTR, sizeof(STMENUDATA));
                        if (md)
                        {
                            md->idMenuCmd      = dwControlPanelID++;
                            md->idszMenuItem   = (UINT)ControlPanels[i].szAppletName; // Our name
                            md->idMenuIcon     = (UINT)ControlPanels[i].hIcon; // Our icon
                            md->nSpecialFolder = CONTROL_PANEL_ITEM; // This is our magic flag indicating a "special" menu item...
                            md->szDisplay = NULL;
                            md->width     = 0;
                            md->height    = 0;

                            // add the new menu item in alphabetical order (BUG 40080)
                            UINT uPosition = FindNewMenuItemPosition(hControlPanelMenu, ControlPanels[ i ].szAppletName, i);

                            if (InsertMenu(hControlPanelMenu, uPosition, MF_BYPOSITION | MF_OWNERDRAW, md->idMenuCmd, (LPCWSTR)md) == FALSE)
                            {
                                DEBUGMSG(ZONE_WARNING, (L"CreateControlPanelMenu couldn't append menu item %d\n", i));
                                LocalFree(md);
                            }
                            else
                            {
                                ControlPanels[i].lpMenuData = md;
                                DEBUGMSG(ZONE_VERBOSE, (L"ControlPanelMenu: id = %d, name = %s, icon = 0x%x, md = 0x%x\n", md->idMenuCmd, md->idszMenuItem, md->idMenuIcon, md));
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        DEBUGMSG(ZONE_WARNING, (L"ControlPanelHelper didn't enumerate any items.\n"));
    }

    return hControlPanelMenu;
}

HMENU StartMenu_CreateSettingsMenu(VOID)
{
    HMENU hmenu = CreatePopupMenu();
    BOOL fOK;

    if (hmenu)
    {
        // settings menu
        for (int i=0; i<SETTINGS_NUM_STATIC_ITEMS; i++)
        {
#ifdef OS_CONTROLPANEL
            // If control.exe is missing, drop the "Control Panel" item from Start Menu
            if ((settingsData[i].idMenuCmd == IDM_START_CTLPNL) && (GetFileAttributes(L"\\Windows\\control.exe") == -1))
            {
                continue;
            }
#endif

            // Check for ConnMC before we add it to the Settings menu
            if (IDM_START_CONNMC == settingsData[i].idMenuCmd)
            {
                WCHAR szComponentPath[MAX_PATH];
                _stprintf(szComponentPath, L"%s%c%s", c_szWindows, c_chBS, c_szConnMC);
                if (-1 == GetFileAttributes(szComponentPath))
                {
                    continue;
                }
            }

            if (settingsData[i].idMenuCmd == IDM_START_CTLPNL)
            {
                HMENU hmenuControlPanel = NULL;
                BOOL bExpandControlPanel = FALSE;
                if (g_TaskBar->GetExpandControlPanel(&bExpandControlPanel) && bExpandControlPanel)
                {
                    if (hmenuControlPanel = StartMenu_CreateControlPanelMenu())
                    {
                        fOK = AppendMenu(hmenu, MF_OWNERDRAW | MF_POPUP, (UINT)hmenuControlPanel, (LPCWSTR)&settingsData[i]);
                        ASSERT(fOK);
                    }
                    else
                    {
                        DEBUGMSG(ZONE_FATAL, (L"Call to CreateControlPanelMenu() failed.\n"));
                    }
                }
                else
                {
                    fOK = AppendMenu(hmenu, MF_OWNERDRAW,
                                     settingsData[i].idMenuCmd,
                                     (LPCWSTR) &settingsData[i]);
                    ASSERT(fOK);
                }
            }
            else
            {
                fOK = AppendMenu(hmenu, MF_OWNERDRAW,
                                 settingsData[i].idMenuCmd,
                                 (LPCWSTR) &settingsData[i]);
                ASSERT(fOK);
            }
        }
    }

    return hmenu;
} /* StartMenu_CreateSettingsMenu
   */

HMENU StartMenu_Create(VOID)
{
    BOOL fOK= FALSE;
    HMENU hmenu;
    int i;

    HMENU hmenuSettings = StartMenu_CreateSettingsMenu();
    if (!hmenuSettings)
    {
        return NULL;
    }

    hmenu = CreatePopupMenu();
    if (!hmenu)
    {
        FileMenu_Destroy(hmenuSettings);
        return 0;
    }

    // start menu
    for (i=0; i<START_NUM_STATIC_ITEMS; i++)
    {
        // If peghelp.exe is missing, drop the "Help" item from Start Menu
        if ((IDM_START_HELP == stmenuData[i].idMenuCmd) && (GetFileAttributes(L"\\Windows\\peghelp.exe") == -1))
        {
            continue;
        }

        // Don't include Suspend if registry says so (either seperator or Suspend).
        if ((IDM_START_SUSPEND == stmenuData[i].idMenuCmd) || (0 == stmenuData[i].idMenuCmd))
        {
            DWORD dwLen;
            DWORD dwValue;
            HKEY  hkey;

            dwValue = TRUE;    // Set default
            dwLen = sizeof(dwValue);
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Explorer", 0, KEY_READ, &hkey) == ERROR_SUCCESS)
            {
                RegQueryValueEx(hkey, L"Suspend", NULL, NULL, (LPBYTE)&dwValue, &dwLen);
                RegCloseKey(hkey);
            }

            if (FALSE == dwValue)
            {
                continue;
            }
        }

        if (0 == idFirstMenu)
        {
            idFirstMenu = stmenuData[i].idMenuCmd;
        }
        idLastMenu = stmenuData[i].idMenuCmd;

        if (stmenuData[i].idMenuIcon)
        {
            if (stmenuData[i].idMenuCmd == IDM_START_SETTINGS)
            {
                // insert settings menu
                fOK = AppendMenu(hmenu,     MF_OWNERDRAW | MF_POPUP,
                                 (UINT)hmenuSettings, (LPCWSTR)&stmenuData[i]);
            }
            else if (!stmenuData[i].nSpecialFolder ||
                       (stmenuData[i].nSpecialFolder == STATIC_ITEM))
            {
                // insert standard menu item
                fOK = AppendMenu(hmenu, MF_OWNERDRAW, stmenuData[i].idMenuCmd,
                                 (LPCWSTR) &stmenuData[i]);
            }
            else
            {
                // insert cascade item
                HMENU hmenuSubMenu;
                LPITEMIDLIST pidl;

                if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, stmenuData[i].nSpecialFolder, &pidl)))
                {
                    hmenuSubMenu = FileMenu_CreatePopupWithPIDL(pidl);
                    fOK = AppendMenu(hmenu,     MF_OWNERDRAW | MF_POPUP,
                                     (UINT)hmenuSubMenu,
                                     (LPCWSTR)&stmenuData[i]);
                }
            }
        }
        else
        {
            fOK = AppendMenu(hmenu,MF_SEPARATOR, 0,0);
        }
        ASSERT(fOK);

    }

#ifdef USE_STARTMENU_BITMAP
    HDC hdc = GetDC(NULL);
    BITMAP bm;
    if (GetDeviceCaps(hdc, BITSPIXEL) == 2)
    {
        hbmSTPegasus = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_STPEGASUSG));
    }
    else
    {
        hbmSTPegasus = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_STPEGASUS));
    }
    ReleaseDC(NULL, hdc);

    GetObject(hbmSTPegasus, sizeof(BITMAP), &bm);

    sizeSTPegasus.cx = bm.bmWidth;
    sizeSTPegasus.cy = bm.bmHeight;
#else // USE_STARTMENU_BITMAP
    hbmSTPegasus = NULL;
    sizeSTPegasus.cx = 0;
    sizeSTPegasus.cy = 0;
#endif // USE_STARTMENU_BITMAP

    return hmenu;
} /* StartMenu_Create() */

void StartMenu_Destroy()
{
    if (hbmSTPegasus)
    {
        DeleteObject(hbmSTPegasus);
        hbmSTPegasus = NULL;
    }
    FileMenu_FreeMenuItems(hStartMenu);
    DestroyMenu(hStartMenu);
    hStartMenu = 0;

    // Delete our control-panel info
    StartMenu_FreeControlPanelInfo();
} /* StartMenu_Destroy() */

int StartMenu_MeasureItem(MEASUREITEMSTRUCT *lpmis)
{
    LPSTMENUDATA  lpst;
    SIZE size;
    HDC hdc;
    WCHAR szDisplayName[MAX_PATH];
    WCHAR *szName;

    if (!lpmis)
    {
        ASSERT(lpmis);
        return FALSE;
    }

    lpst = (LPSTMENUDATA)lpmis->itemData;
    if (!lpst)
    {
        ASSERT(lpst);
        return FALSE;
    }

    // NOTE: In the case of being out of memory, either GetDC()
    //       or GetTextExtentExPoint() can fail, so I am going to
    //       cheat and set the width to what I know it should be in
    //       that case. However, this will need to be updated if the
    //       system font changes.

    if (lpst->width && lpst->height)
    {
        // already measured
        lpmis->itemHeight = lpst->height;
        lpmis->itemWidth = lpst->width;
        return TRUE;
    }

    size.cx = 94;
    size.cy = 0;

    if (lpst->idMenuCmd)
    {
        // Height of 16 for icon + 3 pixels top & bottom
        lpmis->itemHeight = GetSystemMetrics(SM_CYSMICON) + 6;
    }
    else
    {
        // This means we have a separator
        lpmis->itemHeight = 7;
    }

    // load the string int szDisplayName
    szName = szDisplayName;
    if (!Menu_IsPidl(lpst))
    {
        if (lpst->nSpecialFolder == CONTROL_PANEL_ITEM)
        {
            szName = (WCHAR *)lpst->idszMenuItem;
        }
        else
        {
            LoadString(g_hInstance, lpst->idszMenuItem, szDisplayName, MAX_PATH);
        }
    }
    else if (Menu_IsEmptyPidl(lpst->pidl))
    {
        LoadString(g_hInstance, IDS_EMPTY, szDisplayName, MAX_PATH);
    }
    else
    {
        if (!lpst->szDisplay)
        {
            ASSERT(lpst->szDisplay);
        }
        else
        {
            szName = lpst->szDisplay;
        }
    }

    hdc = GetDC(NULL);
    GetTextExtentExPoint(hdc, szName, lstrlen(szName),
                         MAX_TEXT_WIDTH, NULL, NULL, &size);
    if (size.cx > MAX_TEXT_WIDTH)
    {
        // limit if needed.
        size.cx = MAX_TEXT_WIDTH;
    }
    ReleaseDC(NULL, hdc);

    // reset height if need be
    if (size.cy > GetSystemMetrics(SM_CYSMICON))
    {
        lpmis->itemHeight = size.cy + 10;
    }

    // 4 pixel border + icon width + 4 pixel + text length + 8 pixels
    lpmis->itemWidth  = size.cx + GetSystemMetrics(SM_CXSMICON) + 8 +
                        CX_CASCADE_WIDGET;
    if (lpst->idMenuCmd <= IDM_START_LAST_BANNER)
    {
        lpmis->itemWidth += sizeSTPegasus.cx;
    }

    if (lpst->szDisplay)
    {
        // display will be 0 for static items
        lpst->height = lpmis->itemHeight;
        lpst->width = lpmis->itemWidth;
    }

    return TRUE;
} /* StartMenu_MeasureItem()
   */

int StartMenu_DrawMenuItem(DRAWITEMSTRUCT *lpdis)
{
    LPSTMENUDATA  lpst;
    RECT rc;
    HICON hicon;
    CStringRes szMenu(g_hInstance);

    if (!lpdis)
    {
        ASSERT(lpdis);
        return FALSE;
    }

    lpst = (LPSTMENUDATA)lpdis->itemData;
    if (!lpst)
    {
        ASSERT(lpst);
        return FALSE;
    }
    // ASSERT(lpst->szDisplay);

    CopyRect(&rc, &lpdis->rcItem);
    if (lpst->idMenuCmd <= IDM_START_LAST_BANNER)
    {
        rc.left += sizeSTPegasus.cx; // This is for the start menu banner.
    }

    switch (lpdis->itemAction)
    {
        case ODA_DRAWENTIRE:
            if (idFirstMenu == lpst->idMenuCmd)
            {
                CopyRect(&rcMenu, &lpdis->rcItem);
                rcMenu.right = rcMenu.left + sizeSTPegasus.cx;
            }
            else if  (idLastMenu == lpst->idMenuCmd)
            {
                rcMenu.bottom = lpdis->rcItem.bottom;

                if (hbmSTPegasus)
                {
                    HDC hdcmem = CreateCompatibleDC(lpdis->hDC);
                    if (hdcmem)
                    {
                        SelectObject(hdcmem, hbmSTPegasus);

                        FillRect(lpdis->hDC, &rcMenu,
                                 GetSysColorBrush(COLOR_ACTIVECAPTION));

                        BitBlt(lpdis->hDC, 0, rcMenu.bottom - sizeSTPegasus.cy,
                               sizeSTPegasus.cx, sizeSTPegasus.cy, hdcmem, 0, 0,
                               SRCCOPY);
                        DeleteDC(hdcmem);
                    }
                }
            }
            // drop into color selects...

        case ODA_SELECT:
            if ((lpdis->itemState & ODS_SELECTED) && lpst->idMenuCmd)
            {
                HBRUSH hbrush = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
                SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
                FillRect(lpdis->hDC, &rc, hbrush);
                DeleteObject((HGDIOBJ)hbrush);
            }
            else
            {
                HBRUSH hbrush = CreateSolidBrush(GetSysColor(COLOR_MENU));
                SetTextColor(lpdis->hDC, GetSysColor(COLOR_MENUTEXT));
                FillRect(lpdis->hDC, &rc, hbrush);
                DeleteObject((HGDIOBJ)hbrush);
            }
            break;

        default:
            break;
    }

    RECT rcT = rc;
    if (Menu_IsPidl(lpdis->itemData))
    {
        if (Menu_IsEmptyPidl(Menu_GetPidl(lpdis->itemData)))
        {
            szMenu.LoadString(IDS_EMPTY);
            if (NULL == (LPCWSTR)szMenu)
            {
                ASSERT(szMenu);
                return FALSE;
            }
            DrawText(lpdis->hDC, szMenu, lstrlen(szMenu),
                     &rcT, DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
        }
        else
        {
            SIZE siz;
            SHFILEINFO sfi;
            int cch;
            WCHAR sz[MAX_PATH];
            if (SHGetPathFromIDList(Menu_GetPidl(lpdis->itemData), sz))
            {
                HIMAGELIST himl = (HIMAGELIST)SHGetFileInfo((LPCWSTR)
                    sz, 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
                SetBkMode(lpdis->hDC, TRANSPARENT);
                ImageList_Draw(himl, sfi.iIcon, lpdis->hDC,
                           rc.left + 4, rc.top + 3, ILD_NORMAL);
                rcT.left += 24; // skip the space, icon, space
                rcT.right -= CX_CASCADE_WIDGET; // room for the cascade widget
            }
            GetTextExtentPoint(lpdis->hDC, c_szEllipses, CCHELLIPSES, &siz);
            if (NeedsEllipses(lpdis->hDC, ((LPSTMENUDATA)lpdis->itemData)->szDisplay,
                              &rcT, &cch, siz.cx))
            {
                lstrcpy(((LPSTMENUDATA)lpdis->itemData)->szDisplay + cch, c_szEllipses);
                cch += CCHELLIPSES;
            }
            else
            {
                cch =  _tcslen(((LPSTMENUDATA)lpdis->itemData)->szDisplay);
            }
            DrawText(lpdis->hDC, ((LPSTMENUDATA)lpdis->itemData)->szDisplay,
                     cch,  &rcT,
                     DT_VCENTER | DT_SINGLELINE | DT_NOCLIP | DT_NOPREFIX);
        }
    }
    else if (lpst->idMenuCmd)
    {
        SetBkMode(lpdis->hDC, TRANSPARENT);
        if (lpst->idMenuIcon)
        {
            if (lpst->nSpecialFolder == CONTROL_PANEL_ITEM)
            {
                hicon = (HICON)lpst->idMenuIcon;
            }
            else
            {
                hicon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(lpst->idMenuIcon),
                                         IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
            }
            DrawIconEx(lpdis->hDC, rc.left + 4, rc.top + 3,
                       hicon, 16, 16, 0, NULL, DI_NORMAL);
        }
        else
        {
            hicon = 0;
        }

        OffsetRect (&rcT, 24, 0);
        if (lpst->nSpecialFolder == CONTROL_PANEL_ITEM)
        {
            // Specify DT_NOPREFIX here because we:
            // a) don't handle keyboard shortcuts for the control-panel items
            // b) don't want to see "Volume _Sounds" instead of "Volume & Sounds"
            DrawText(lpdis->hDC, (LPCWSTR)lpst->idszMenuItem, _tcslen((LPCWSTR)(lpst->idszMenuItem)),
                     &rcT, DT_VCENTER | DT_SINGLELINE | DT_NOCLIP | DT_NOPREFIX);
        }
        else
        {
            szMenu.LoadString(lpst->idszMenuItem);
            if (NULL == (LPCWSTR)szMenu)
            {
                ASSERT(szMenu);
                return FALSE;
            }
            DrawText(lpdis->hDC, szMenu, _tcslen(szMenu),
                     &rcT, DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
        }

        if (hicon)
        {
            if (lpst->nSpecialFolder != CONTROL_PANEL_ITEM)
            {
                DestroyIcon(hicon);
            }
        }
    }
    else
    {
        // Draw the separator

        POINT pt[2];

        pt[0].x = rc.left;
        pt[0].y = rc.top + 3;
        pt[1].x = rc.right - 3;
        pt[1].y = rc.top + 3;

        Polyline(lpdis->hDC, pt, 2);
    }

    return TRUE;
} /* StartMenu_DrawMenuItem()
   */

BOOL StartMenu_Track(UINT uFlags, int x, int y, HWND hwnd)
{
    // set the id's back to the starting point
    g_idFileMenu = FILEMENU_ID_START;

    ASSERT(!hStartMenu);

    hStartMenu = StartMenu_Create();

    if (!hStartMenu)
    {
        return FALSE;
    }
    TrackPopupMenu(hStartMenu, uFlags, x, y, 0, hwnd, 0);
    // we can't destroy here since we need the menu around to lookup the pidl
    PostMessage(hwnd, WM_COMMAND, IDM_TASKBAR_DESTROY_STARTMENU, 0);

    return TRUE;
} /* StartMenu_Track
   */

int StartMenu_InitPopup(HMENU hmenu)
{
    UINT id = GetMenuItemID(hmenu, 0);
    if (Menu_IsUninitializedPidl(id))
    {
        LPSTMENUDATA pstd = FileMenu_GetMenuItemData(hmenu, 0);
        ASSERT(GetMenuItemCount(hmenu) == 1);
        RemoveMenu(hmenu, 0, MF_BYPOSITION);
        FileMenu_PopulateUsingPIDL(hmenu, 0,Menu_GetPidl(pstd));
        FileMenu_FreeData(pstd);
    }

    return TRUE;
} /* StartMenu_InitPopup
   */

BOOL StartMenu_ExecutePidl(HWND hwnd, LPITEMIDLIST pidl)
{
    WCHAR sz[MAX_PATH];

    if (SHGetPathFromIDList(pidl, sz))
    {
        SHELLEXECUTEINFO sei;

        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = 0;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = sz;
        sei.lpParameters = 0;
        sei.lpDirectory = NULL;
        sei.nShow = SW_SHOWNORMAL;

        ShellExecuteEx(&sei);
        return TRUE;
    }

    RETAILMSG(1,(L"Failed to get path from pidl\r\n"));
    return FALSE;
} /* StartMenu_ExecutePidl
   */

HWND GetHelpWindow(HWND hwnd)
{
    HWND hwndHelp;

    hwndHelp = GetWindow(hwnd, GW_HWNDNEXT);

    //OK here's the deal:  If it's NOT VISIBLE or
    //                     it's a HELPER_EX_STYLES or
    //                     it's an OLD APP and the window has no title,
    //                     then keep looking
    //
    //                     For new apps, we will send the WM_HELP to a window
    //                     that has no title.  This is because many full screen
    //                     "foo" windows do not specify a caption, since the
    //                     caption is inherited from the owning app.
    while (!(WS_VISIBLE & GetWindowLong(hwndHelp, GWL_STYLE)))
//        ||
//           (HELPER_EX_STYLES & GetWindowLong(hwndHelp, GWL_EXSTYLE)) ||
//           (SHIsPreRapierApp(hwndHelp)
//           && !GetWindowTextLength(hwndHelp)) //)
    {
        hwndHelp = GetWindow(hwndHelp, GW_HWNDNEXT);
    }

    return hwndHelp;
}

//
// Send a WM_HELP to the last active window.  If that window was the desktop,
// start up help ourselves.
//
BOOL StartMenu_HelpBtn(HWND hwndTB)
{
    HWND hwndHelp = GetHelpWindow(hwndTB);

    CStringRes sz(g_hInstance);
    // No error message
#pragma prefast(suppress:309, "Argument 9 is supposed to be set to NULL in CE")
    CreateProcess(c_szPegHelp, sz.LoadString(IDS_HELPFILE), NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    Sleep(500);

    // Send help to the window.
    if (GetDesktopWindow() != hwndHelp)
    {
        PostMessage(hwndHelp, WM_HELP, 0, 0);
    }

    return TRUE;
}

DWORD StartMenu_ProcessCommand(HWND hwnd, WPARAM wp, LPARAM lp)
{
    DWORD cmdId = GET_WM_COMMAND_ID(wp,lp);
    SHELLEXECUTEINFO info;

    switch (cmdId)
    {
        case IDM_START_EXPLORE:
            info.cbSize = sizeof(info);
            info.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
            info.lpVerb = NULL;
            info.lpFile = c_szSlash;
            info.lpParameters = NULL;
            info.lpDirectory = NULL;
            info.nShow = SW_SHOW;
            info.hInstApp = NULL;

            ShellExecuteEx(&info);
            break;

        case IDM_START_RUN:
            if (g_TaskBar)
            {
                g_TaskBar->ShowRunDlg();
            }
            break;

        case IDM_START_HELP:
        {
            StartMenu_HelpBtn(g_TaskBar->GetWindow());
            break;
        }

        case IDM_START_SUSPEND:
        {
            DWORD dwType;
            DWORD dwLen;
            DWORD dwValue;
            DWORD lRet;

            dwValue = TRUE;    // Set default
            dwLen = sizeof(dwValue);
            lRet = RegQueryValueEx(HKEY_LOCAL_MACHINE, L"Suspend",
                            (LPDWORD) L"Explorer", &dwType,
                            (LPBYTE) &dwValue, &dwLen);
            if (dwValue)
            {
                GwesPowerOffSystem();
            }
            break;
        }

        case IDM_START_CTLPNL:
            info.cbSize = sizeof(info);
            info.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
            info.lpVerb = NULL;
            info.lpFile = c_szControlPanel;
            info.lpParameters = NULL;
            info.lpDirectory = NULL;
            info.nShow = SW_SHOW;
            info.hInstApp = NULL;

            ShellExecuteEx(&info);
            break;

        case IDM_START_CONNMC:
            info.cbSize = sizeof(info);
            info.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
            info.lpVerb = NULL;
            info.lpFile = c_szConnMC;
            info.lpParameters = NULL;
            info.lpDirectory = NULL;
            info.nShow = SW_SHOW;
            info.hInstApp = NULL;

            ShellExecuteEx(&info);

            break;

        case IDM_START_TASKBAR:
            if (g_TaskBar)
            {
                g_TaskBar->Properties(hwnd);
            }
            //HHTaskBar_Properties(hwnd);
            break;

        default:
            if ((cmdId >= FILEMENU_ID_START))
            {
                LPSTMENUDATA pstd = FileMenu_GetItemData(hStartMenu, cmdId, FALSE);
                if (!pstd)
                {
                    return FALSE;
                }
                StartMenu_ExecutePidl(hwnd, pstd->pidl);
            }
            else
            {
                LPSTMENUDATA pstd = FileMenu_GetItemData(hStartMenu, cmdId, FALSE);
                if (pstd && pstd->nSpecialFolder == CONTROL_PANEL_ITEM)
                {
                    WCHAR szControlPanelCmdLine[ MAX_PATH ];
                    swprintf(szControlPanelCmdLine, L"%s,%d", ControlPanels[cmdId - dwControlPanelStartID].szDllName, ControlPanels[cmdId - dwControlPanelStartID].nApplet);
                    DEBUGMSG(ZONE_VERBOSE, (L"Launching Control-Panel with cmd-line of %s\n", szControlPanelCmdLine));
                    //Host_Exec(hwnd, c_szControlPanelLauncher, szControlPanelCmdLine);
                    info.cbSize = sizeof(info);
                    info.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
                    info.lpVerb = NULL;
                    info.lpFile = c_szControlPanelLauncher;
                    info.lpParameters = szControlPanelCmdLine;
                    info.lpDirectory = NULL;
                    info.nShow = SW_SHOW;
                    info.hInstApp = NULL;


                    ShellExecuteEx(&info);

                    return TRUE;
                }
                RETAILMSG(1, (L"Command not processed, because id is 0x%x\r\n", cmdId));
                return FALSE;
            }
            break;
    }

    return TRUE;
} /* StartMenu_ProcessCommand
   */


DWORD StartMenu_MenuChar(HMENU hmenu, WCHAR cChar)
{
    // We should be iterating through the menu items and finding the next
    // item here.

    int n = 0, index = -1, indexSelected = -1, bIsFolder = 0;
    BOOL bMoreThanOne = FALSE;
    MENUITEMINFO mii;
    LPSTMENUDATA pstd;
    SHFILEINFO sfi;
    WCHAR *pc = NULL, *ptmp = NULL;

    ZeroMemory(&mii, sizeof(MENUITEMINFO));
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_DATA | MIIM_STATE | MIIM_SUBMENU | MIIM_ID;

    // if a menu is getting characters, it shouldn't be empty so we
    // shouldn't need a degenerate case
    while (GetMenuItemInfo(hmenu, n, TRUE, &mii))
    {
        pstd = (LPSTMENUDATA)mii.dwItemData;
        pc = (WCHAR*)sfi.szDisplayName;
        *pc = 0;
        // get the string
        if (!pstd)
        {
            n++;
            continue;
        }
        else if (Menu_IsPidl(mii.dwItemData) && (mii.wID != FILEMENU_ID_EMPTY))
        {
            pc = ((LPSTMENUDATA)mii.dwItemData)->szDisplay;
        }
        else if (pstd->idMenuCmd)
        {
            if (pstd->nSpecialFolder == CONTROL_PANEL_ITEM)
            {
                lstrcpy(sfi.szDisplayName, (LPCWSTR)pstd->idszMenuItem);
            }
            else
            {
                LoadString(g_hInstance, pstd->idszMenuItem, sfi.szDisplayName, MAX_PATH);
            }
            // skip to accelerator if need be
            ptmp = pc;
            while (*ptmp != L'&')
            {
                ptmp++;
            }

            if (*ptmp == L'&')
            {
                pc = ++ptmp;
            }
        }

        if (_istlower(*pc))
        {
            cChar = _totlower(cChar);
        }
        else
        {
            cChar = _totupper(cChar);
        }

        // this item is selected?
        if (mii.fState & MFS_HILITE)
        {
            indexSelected = n;
        }

        if (*pc == cChar)
        {
            if (index >= 0)
            {
                bMoreThanOne = TRUE;
            }
            // match
            if (((indexSelected >= 0) && (n > indexSelected) &&
               (index <= indexSelected)) || (index < 0))
            {
                // this selection is past the hilite or the first found
                index = n;
                bIsFolder = (mii.hSubMenu != 0);
            }
        }
        n++;
    }

    if (index >= 0)
    {
        if (bMoreThanOne)
        {
            return MAKELONG(index, MNC_SELECT);
        }
        else
        {
            return MAKELONG(index, MNC_EXECUTE);
        }
    }
    MessageBeep(0xFFFFFFFF);
    return MAKELONG(0, MNC_IGNORE);
} /* StartMenu_ProcessMenuChar
   */

