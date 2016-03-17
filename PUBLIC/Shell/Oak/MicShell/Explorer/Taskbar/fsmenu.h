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
 *  module: fsmenu.h
 *  purpose:
 *
 * Notes:
 *
 *        menu id:
 *                0                    - separator
 *                1-IDM_START_LAST_BANNER    - items on the first level (w/ the bitmap)
 *                IDM_STARTLAST-60k    - standard menu items
 *                >60k                - pidl based item (FileMenu item)
 *
\*---------------------------------------------------------------------------*/
#ifndef __FSMENU_H__
#define __FSMENU_H__

#include "idlist.h"

// stuff for pidl based menus

BOOL    FileMenu_Destroy(HMENU hmenu);
HRESULT FileMenu_PopulateUsingPIDL(HMENU hmenu, UINT uPosition,
                                   LPITEMIDLIST pidl,
                                   BOOL bColumnBreak = TRUE);

HMENU   FileMenu_CreatePopupWithPIDL(LPITEMIDLIST pidl);
int     FileMenu_MeasureItem(MEASUREITEMSTRUCT *lpmis);
int     FileMenu_DrawItem(DRAWITEMSTRUCT *lpdis);
void    FileMenu_RemoveAllItems(HMENU hmenu);
void    FileMenu_FreeMenuItems(HMENU hmenu);
BOOL    FileMenu_Execute(HWND hwnd, HMENU hmenu, UINT id);

LPSTMENUDATA FileMenu_GetMenuItemData(HMENU hmenu, int pos);
LPSTMENUDATA FileMenu_CreateItemDataFromPidl(LPITEMIDLIST pidl);

// PIDLS are used in idMenuCmd, ID's are used for menu item id's
#define FILEMENU_ID_EMPTY    0xFFFF    // empty
#define FILEMENU_ID_PIDL    0xFFFE    // uninitilized
#define FILEMENU_ID_START    0xEFFF    // 60k
#define FILEMENU_PIDL_EMPTY 0xFFFF000

// if nSpecialFolder != 0, the item is not free'd this constant is to keep
// from freeing static item data that is not a special folder
#define STATIC_ITEM  0xFFFF    // static data item flag 
#define CONTROL_PANEL_ITEM 0xCDEF // control-panel item flag

#define Menu_IsPidl(x) ((x) && ((((LPSTMENUDATA)(x))->idMenuCmd) & 0xFFFF0000))
#define Menu_GetPidl(x) ((LPITEMIDLIST)((LPSTMENUDATA)(x))->idMenuCmd)
#define Menu_IsFolder(x) (((LPSTMENUDATA)(x))->dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
#define Menu_IsEmptyPidl(x) ((INT)(x) == FILEMENU_ID_EMPTY)
#define Menu_IsUninitializedPidl(x) ((INT)(x) == FILEMENU_ID_PIDL)
#define Menu_IsSpecialFolder(x) (((LPSTMENUDATA)(x))->nSpecialFolder && (((LPSTMENUDATA)(x))->nSpecialFolder != CONTROL_PANEL_ITEM))


//
// inline routines
//


inline LPSTMENUDATA FileMenu_GetItemData(HMENU hmenu, int pos, BOOL fByPosition = TRUE)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
    MENUITEMINFO mii;
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_DATA;
    
    if (hmenu && GetMenuItemInfo(hmenu, pos, fByPosition, &mii))
    {
        return (LPSTMENUDATA)mii.dwItemData;
    }
    return 0;
    
} /* GetMenuItemCount
   */


inline void FileMenu_FreeData(LPSTMENUDATA pstd)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
    // Inline function, caller must perform check
    PREFAST_ASSERT(pstd);

    if(Menu_IsPidl(pstd))
    {
        ILFree(pstd->pidl);
    }

    if (pstd->szDisplay)
    {
        LocalFree(pstd->szDisplay);
    }

    LocalFree(pstd);
    
} /* FileMenu_FreeData
   */


inline BOOL FileMenu_Destroy(HMENU hmenu)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
    FileMenu_FreeMenuItems(hmenu);
    return DestroyMenu(hmenu);
    
} /* FileMenu_Destroy()
   */


#endif /* __FSMENU_H__ */

