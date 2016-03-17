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
 *  module: fsmenu.cpp
 *  purpose: provide menus based on the filesystem
 *
\*---------------------------------------------------------------------------*/

//#include "explorer.h"
#include <windows.h>
#include <shlwapi.h>
#include <shcore.h>
#include "idlist.h"
#include "stmenu.h"
#include "fsmenu.h"
#include "taskbar.hxx"
#include "resource.h"
#include "utils.h"

extern HINSTANCE g_hInstance;
int g_idFileMenu; 

#define ADDREF(x)    if ((x) != NULL) { (x)->AddRef(); }
#define RELEASE(x)   if ((x) != NULL) { (x)->Release(); (x) = NULL; }

inline LPWSTR CopyString(LPCWSTR sz)
{
	return ToWide(sz);
}

inline LPSTR CopyString(LPCSTR sz)
{
	return ToAnsi(sz);
}


void FileMenu_FreeMenuItems(HMENU hmenu)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
	int n = 0;
	MENUITEMINFO mii;
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_DATA | MIIM_SUBMENU | MIIM_ID;
	mii.hSubMenu = NULL;
	
	while (GetMenuItemInfo(hmenu, n, TRUE, &mii)) {
		if (mii.dwItemData && !Menu_IsSpecialFolder(mii.dwItemData)){
			FileMenu_FreeData((LPSTMENUDATA)mii.dwItemData);
		}
		
		if (mii.hSubMenu) {
			FileMenu_FreeMenuItems(mii.hSubMenu);
		}
		n++;
	}
	
} /* FileMenu_FreePidls
   */


void FileMenu_RemoveAllItems(HMENU hmenu)
/*---------------------------------------------------------------------------*\
 *
 * Assumes all pidl menu items are dynamic
 *
\*---------------------------------------------------------------------------*/
{
	LPSTMENUDATA pstd; 
	while (1) {
		if (pstd = FileMenu_GetItemData(hmenu, 0)) {
		if (pstd && !Menu_IsSpecialFolder(pstd)){
				FileMenu_FreeData(pstd);
			}
		}
		if (!RemoveMenu(hmenu, 0, MF_BYPOSITION)) {
			return; // assume empy 
		}
		
	}

} /* FileMenu_RemoveAllItems
   */


BOOL FileMenu_InsertEmptyItem(HMENU hmenu)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
	CStringRes sz(g_hInstance);

	if (!sz.LoadString(IDS_EMPTY)) {
		return FALSE;
	}

	return InsertMenu(hmenu, 0, MF_BYPOSITION | MF_GRAYED,
					  FILEMENU_ID_EMPTY, sz);

} /* FileMenu_Empty(HMENU hmenu)
   */

HMENU FileMenu_CreatePopupWithPIDL(LPITEMIDLIST pidl)
/*---------------------------------------------------------------------------*\
 *
 * takes ownership of the pidl
 *
\*---------------------------------------------------------------------------*/
{
	HMENU hmenu = CreatePopupMenu();
	LPSTMENUDATA lpstd = FileMenu_CreateItemDataFromPidl(pidl);
	
	RETAILMSG(!hmenu, (TEXT("CreatePopupMenu failed\r\n")));

	if (hmenu && lpstd) {
		if (InsertMenu(hmenu, 0, MF_BYPOSITION | MF_OWNERDRAW,
					  FILEMENU_ID_PIDL, (LPCTSTR)lpstd)) {
			return hmenu;
		}
	}
	if (lpstd) {
		FileMenu_FreeData(lpstd);
	}
	if (hmenu) {
		DestroyMenu(hmenu);
	}
	return 0;
	
} /* FileMenu_CreatePopupWithPIDL(HMENU hmenu)
   */

INT FileMenu_CompareItems(TCHAR *szDisplay1, DWORD dwAttributes1,
						  TCHAR *szDisplay2, DWORD dwAttributes2)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
	if ((dwAttributes1 & FILE_ATTRIBUTE_DIRECTORY) &&
		!(dwAttributes2 & FILE_ATTRIBUTE_DIRECTORY)) {
		return -1; 
	}
	if (!(dwAttributes1 & FILE_ATTRIBUTE_DIRECTORY) &&
		(dwAttributes2 & FILE_ATTRIBUTE_DIRECTORY)) {
		return 1;
	}
	return CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
						 szDisplay1, -1, szDisplay2, -1) - 2;
	// return _tcsicmp(szDisplay1, szDisplay2);
	
} /* FileMenu_CompareItems
   */

LPSTMENUDATA FileMenu_GetMenuItemData(HMENU hmenu, int pos)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_DATA;
	
	if (!GetMenuItemInfo(hmenu, pos, TRUE, &mii)) {
		return 0;
	}
	return (LPSTMENUDATA)mii.dwItemData;
} /* FileMenu_GetMenuItemData
   */
				
INT FileMenu_FindSortedPosition(HMENU hmenu, LPSTMENUDATA pstd)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
	int min, max, index, maxSave; 
	LPSTMENUDATA pstd2;


	min = 0;
	index = maxSave = max = GetMenuItemCount(hmenu) - 1; // -1 if empty menu (insert at end)
	
	while(min <= max) {
			index = min + (max - min +1)/2;
			if (!(pstd2 = FileMenu_GetMenuItemData(hmenu, index))) {
				return -1; 
			}
			if (FileMenu_CompareItems(pstd->szDisplay,  pstd->dwAttributes,
									  pstd2->szDisplay, pstd2->dwAttributes) < 0) {
				max = index-1; 
			}else{
				min = index+1;
				index++; // if its greater and we exit, we want the next index
			}
	}

	if (index > maxSave) {
		index = -1; // inserting into a menu -1 is at the end
	}
	return index; 
	

} /* FileMenu_FindSortedPosition(hmenu, pidl);
   */

BOOL FileMenu_SetItemData(HMENU hmenu, int pos, LPSTMENUDATA lpstd)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_DATA;
	mii.dwItemData = (DWORD)lpstd;
	return SetMenuItemInfo(hmenu, pos, TRUE, &mii);
} /* FileMenu_SetItemData
   */

BOOL FileMenu_InsertItem(HMENU hmenu, UINT id, LPITEMIDLIST pidl)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
	int pos;
	BOOL rv;
	LPSTMENUDATA pstd;
	TCHAR szPath[MAX_PATH];
	TCHAR *szName; 

	pstd = FileMenu_CreateItemDataFromPidl(pidl);

	if (NULL == pstd ) {
	    ASSERT (pstd);
	    return FALSE;
	}    

	// get the display string
	IShellFolder * psfParent = NULL;
	LPCITEMIDLIST pidlLast = NULL;
	HRESULT hr = SHBindToParent(pstd->pidl, IID_IShellFolder, (VOID **) &psfParent, &pidlLast);
	if (SUCCEEDED(hr))
	{
		STRRET str = {0};
		hr = psfParent->GetDisplayNameOf(pidlLast, SHGDN_INFOLDER | SHGDN_FORADDRESSBAR, &str);

		if (SUCCEEDED(hr))
			hr = StrRetToBuf(&str, pidlLast, szPath, sizeof(szPath)/sizeof(*szPath));

		psfParent->Release();
		ILFree(pidlLast);
	}  

	if (FAILED(hr))
	{
		FileMenu_FreeData(pstd);
		return FALSE;
	}

	szName = PathFindFileName(szPath);
	PathRemoveExtension(szPath);
	pstd->szDisplay = CopyString(szName);
	
	pos = FileMenu_FindSortedPosition(hmenu, pstd);
	
	if (rv = (pstd != NULL)) {
		if (pstd->dwAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			pidl = ILCopy(pidl, IL_ALL);
			HMENU hSubMenu = FileMenu_CreatePopupWithPIDL(pidl);
			if (rv = (hSubMenu != NULL)) {
				rv = InsertMenu(hmenu, pos,
							MF_BYPOSITION | MF_OWNERDRAW | MF_POPUP,
								(UINT)hSubMenu, (LPCTSTR)pstd);
				RETAILMSG(!rv, (TEXT("!!!Insertmenu failed pos(%d)\r\n"), pos));
				
				if (pos == -1) { 
					pos = GetMenuItemCount(hmenu) -1;
				}
				
				if (!FileMenu_SetItemData(hmenu, pos, pstd)) {
					RETAILMSG(1, (TEXT("FileMenuSetItemData failed! pos = %d\r\n"), pos));
				}
					
			} else {
				ILFree(pidl);
			}
		}else{
			rv = InsertMenu(hmenu, pos, MF_BYPOSITION | MF_OWNERDRAW,
							id, (LPCTSTR)pstd);
		}
	}
	
	if (!rv && pstd) {
		FileMenu_FreeData(pstd);
	}
	return rv;


} /* FileMenu_InsertItem
   */


HRESULT FileMenu_PopulateUsingPIDL(HMENU hmenu, UINT uPosition,
								   LPITEMIDLIST pidl, BOOL bColumnBreak)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{

	HRESULT hr;
	LPENUMIDLIST pEnumIdList = 0;
	LPITEMIDLIST pidlItem;
	IShellFolder *lpdtsf, *pShellFolder = 0;
	int n = 0;
	int cy;
	int height = 0;
	int cyScreen;

	ASSERT(!uPosition); // not implemented

	cyScreen = GetSystemMetrics(SM_CYSCREEN);
	cy = max(GetSystemMetrics(SM_CYSMICON) + 6, GetSystemMetrics(SM_CYMENU));

	ASSERT(GetMenuItemCount(hmenu) == 0);
	
	if (NOERROR != (hr = SHGetDesktopFolder(&lpdtsf))) {
	    return hr;
	}    
	hr = lpdtsf->BindToObject(pidl, 0, IID_IShellFolder, (void **)&pShellFolder);
	RELEASE(lpdtsf);
	if (FAILED(hr))
		goto release;

	hr = pShellFolder->EnumObjects(0, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, 
								   &pEnumIdList);
	if (hr != NOERROR)
		goto release;

	while ( (hr = pEnumIdList->Next(1, &pidlItem, 0)) == NOERROR) {
		LPITEMIDLIST pidlFQ = ILConcatenate(pidl, pidlItem);
		ILFree(pidlItem);
		if (!FileMenu_InsertItem(hmenu, g_idFileMenu++, pidlFQ)) {
			hr = E_UNEXPECTED;
			goto release;
		}
		n++;
	}
	if (bColumnBreak) {
	
		// now add the breaks
		if (n*cy >= cyScreen) {
			MENUITEMINFO mii; 
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_TYPE;
			int i;
			for (i = 0, height = cy; i < n; i++, height += cy) {
				if (height >= cyScreen) { 
					if (GetMenuItemInfo(hmenu, i, TRUE, &mii)) {
						mii.fType |=  MF_MENUBARBREAK;
					}
					if (!SetMenuItemInfo(hmenu, i, TRUE, &mii)) {
						RETAILMSG(1,(TEXT("start menu:set menu item info failed\r\n")));
					}
					height = 0;
				}
			}
		}
	}
	hr = NOERROR;

release:
	if (!n) {
		FileMenu_InsertEmptyItem(hmenu);
	}
	RELEASE(pEnumIdList);
	RELEASE(pShellFolder);
	return hr;
	
} /* FileMenu_PopulateUsingPIDL
   */


DWORD   FileMenu_AttributesFromPidl(LPITEMIDLIST pidl)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
	TCHAR szPath[MAX_PATH];
	DWORD dwAttributes = 0; 

	if (SHGetPathFromIDList(pidl, szPath)) {
		dwAttributes = GetFileAttributes(szPath);
	}
	return dwAttributes;
	
} /* FileMenu_AttributesFromPidl
   */

LPSTMENUDATA FileMenu_CreateItemDataFromPidl(LPITEMIDLIST pidl)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
	LPSTMENUDATA lpsmd = (LPSTMENUDATA)LocalAlloc(LMEM_FIXED, sizeof(STMENUDATA));
	if (lpsmd) {
		lpsmd->pidl = pidl;
		lpsmd->dwAttributes = FileMenu_AttributesFromPidl(pidl);
		lpsmd->idMenuIcon = 0;
		lpsmd->nSpecialFolder = 0;
		lpsmd->szDisplay = 0;
		lpsmd->height = 0;
		lpsmd->width = 0;
	}
	return lpsmd;
			
} /* FileMenu_CreateItemDataFromPidl
   */


