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
#include <exdisp.h>
#include <commctrl.h>
#include <commdlg.h>
#include "resource.h"
#include "filemenu.h"
#include "curlfile.h"
#include "utils.h"

#ifndef _PREFAST_
#pragma warning(disable:4068)
#endif // _PREFAST_

extern HINSTANCE g_hInstance;
extern BOOL g_bBrowseInPlace;

static const WCHAR szRegRoot[] = L"SOFTWARE\\Microsoft\\Internet Explorer\\Favorites";


WCHAR MakeFileChar(WCHAR ch)
{
    switch(ch)
    {
    case L'\\':
    case L'/':
    case L'.':
        return L'_';
    case L'*':
    case L':':
    case L'|':
    case L'"':
    case L'<':
    case L'>':
    case L'?':
        return L' ';
    default:
        return ch;
    }

}


BOOL CFilesysMenu::PopulateFromReg(HMENU hmenu, LPCWSTR szRegKey)
{
    HKEY hKey;
    HKEY hKey2;
    LPWSTR pszDisplayName = NULL;
    LPWSTR pszURL = NULL;
    BOOL bRet = FALSE;
    BOOL fDirectory; 
    MenuData *pData = NULL;
    int cchDisplay;
    int cchBase;

    // regKey is the path to the current 'root
    DWORD dwSize = MAX_PATH;
    DWORD dwIndex = 0;

    cchBase = wcslen(szRegKey);

    pszDisplayName = new WCHAR[MAX_PATH];
    pszURL = new WCHAR[MAX_URL];
    if (!pszDisplayName || !pszURL)
    {
        goto Exit;
    }

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, szRegKey, 0,0, &hKey))
    {
        while(ERROR_SUCCESS == RegEnumKeyEx(hKey, dwIndex, pszDisplayName, &dwSize, NULL, NULL, NULL, NULL))
        {
            if (ERROR_SUCCESS == RegOpenKeyEx(hKey, pszDisplayName, 0, 0, &hKey2))
            {
                // if this key has a default value, or that default value is empty, then it is a 'directory'
                dwSize = MAX_URL*sizeof(WCHAR);
                if (ERROR_SUCCESS == RegQueryValueEx(hKey2, NULL, NULL, NULL, (LPBYTE)pszURL, &dwSize))
                {
                    if (pszURL[0] == 0)
                    {
                        fDirectory = TRUE;
                    }
                    else
                    {
                        fDirectory = FALSE;
                    }
                }
                else
                {
                    fDirectory = TRUE;
                }

                pData = new MenuData;
                if (!pData)
                {
                    goto Error;
                }

                cchDisplay = wcslen(pszDisplayName);
                pData->szDisplay = new WCHAR[cchDisplay+1];
                pData->szPath = new WCHAR[cchDisplay + cchBase + 2];  // add a '\' and a NULL
                if (!pData->szPath || !pData->szDisplay)
                {
                    goto Error;
                }

                if ( StringCchCopy(pData->szDisplay, cchDisplay+1, pszDisplayName) != S_OK )
                {
                    goto Error;
                }

                if ( StringCchCopy(pData->szPath, cchDisplay + cchBase + 2, szRegKey) != S_OK )
                {
                    goto Error;
                }

                pData->szPath[cchBase] = L'\\';
                pData->szPath[cchBase+1] = NULL;
                if ( StringCchCat(pData->szPath, cchDisplay + cchBase + 2, pszDisplayName) != S_OK )
                {
                    goto Error;
                }

                // to make the rest of the code symmetric with the filsys parts
                pData->dwAttribs = fDirectory ? FILE_ATTRIBUTE_DIRECTORY : 0;

                WORD wNextID= wIDFavReg_AddRef();
                if (wNextID == -1)
                {
                    goto Error;
                }
                
                if (!InsertItem(hmenu, wNextID, pData))
                {
                    wIDFavReg_Release();
                    delete pData;
                }
                // after insert pData is owned by the menu item
                pData = NULL;
                              
                RegCloseKey(hKey2);
            }
            dwSize = MAX_PATH;
            dwIndex++;
        }

        RegCloseKey(hKey);
        // if the enumkey didn't find anything make this an empty item

        bRet = TRUE;        
    }

Exit:
    if (pszDisplayName)
    {
        delete [] pszDisplayName;
    }

    if (pszURL)
    {
        delete [] pszURL;
    }
    return bRet;

Error:
    if (pData)
    {
         delete pData;    
    }
    RegCloseKey(hKey2);
    RegCloseKey(hKey);
    bRet = FALSE;
    goto Exit;
    
}

const WCHAR *szSearchFilter= L"*";
BOOL CFilesysMenu::PopulateFromPath(HMENU hmenu, LPCWSTR szPath)
{
    WIN32_FIND_DATA fd;
    HANDLE hFind;
    unsigned int cchBaseLen = 0;
    WCHAR szSearchPath[MAX_PATH];
    MenuData *pData = NULL;
    LPWSTR pszTemp;
    
    if (!szPath)
    {
        return FALSE;
    }

    // set up the search filter
    if (FAILED(StringCchLength(szPath, MAX_PATH, &cchBaseLen)))
    {
        return FALSE;
    }

    if (cchBaseLen == 0 || cchBaseLen >= MAX_PATH)
    {
        ASSERT(cchBaseLen > 0);
        return FALSE;
    }

    if ( StringCchCopy(szSearchPath, MAX_PATH, szPath) != S_OK )
    {
        return FALSE;
    }
        
    if ( (szSearchPath[cchBaseLen-1] != L'\\') &&
        ( StringCchCat(szSearchPath, MAX_PATH, L"\\") != S_OK ) )
    {
        return FALSE;
    }

    if ( StringCchCat(szSearchPath, MAX_PATH, szSearchFilter) != S_OK )
    {
        return FALSE;
    }

    hFind = FindFirstFile(szSearchPath, &fd);
    if ( StringCchCopy(szSearchPath, MAX_PATH, szPath) != S_OK )
    {
        return FALSE;
    }
    
    if ( (szSearchPath[cchBaseLen-1] != L'\\') && ( StringCchCat(szSearchPath, MAX_PATH, L"\\") != S_OK ) )
    {
        return FALSE;
    }

    if (INVALID_HANDLE_VALUE != hFind)
    {
        
        int cchNameLen;
        do {
            pData = new MenuData;
            if (!pData)
            {
                break;
            }
            cchNameLen = wcslen(fd.cFileName);

            pData->szDisplay = new WCHAR[cchNameLen+1];
            if (!pData->szDisplay)
            {
                break;
            }

            if ( StringCchCopy(pData->szDisplay, cchNameLen+1, fd.cFileName) != S_OK )
            {
                delete [] pData->szDisplay;
                break;
            }

            pData->szPath = new WCHAR[cchBaseLen+cchNameLen+2]; // possible backslash
            if (!pData->szPath)
            {
                break;
            }

            if ( StringCchCopy(pData->szPath, cchBaseLen+cchNameLen+2, szPath) != S_OK )
            {
                delete [] pData->szDisplay;
                delete [] pData->szPath;
                break;
            }

            if ( ( pData->szPath[cchBaseLen-1] != L'\\') && ( StringCchCat(pData->szPath, cchBaseLen+cchNameLen+2, L"\\") != S_OK ) )
            {
                delete [] pData->szDisplay;
                delete [] pData->szPath;
                break;
            }

            if ( StringCchCat(pData->szPath, cchBaseLen+cchNameLen+2, fd.cFileName) != S_OK )
            {
                delete [] pData->szDisplay;
                delete [] pData->szPath;
                break;
            }

            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                // remove extenstion from the display name
                pszTemp = pData->szDisplay;
                while(cchNameLen && pszTemp[cchNameLen] != L'.')
                    cchNameLen--;
                // only care about URL and LNK files
                if ((wcsicmp(pszTemp+cchNameLen+1, L"url") != 0) 
                    && (wcsicmp(pszTemp+cchNameLen+1, L"lnk") != 0))
                {
                    delete pData;
                    pData = NULL;
                    continue;
                }
                if (pszTemp[cchNameLen] == L'.')
                {
                    pszTemp[cchNameLen] = L'\0';
                }
            }
            
            pData->dwAttribs = fd.dwFileAttributes;

            WORD wNextID= wIDCounter_AddRef();
            if (wNextID == -1)
            {
                break;
            }

            if (!InsertItem(hmenu, wNextID, pData))
            {
                wIDCounter_Release();
                delete pData;
            }
            // after insert pData is owned by the menu item
            pData = NULL;
        }
        while(FindNextFile(hFind, &fd));

        // clean up any failed item
        if (pData)
        {
            delete pData;
        }
     
    }

    return TRUE;
}

HMENU CFilesysMenu::CreateSubMenu(MenuData *pItem)
{
    HMENU hmenu = NULL;

    MenuData *pItem2 = new MenuData;
    if (!pItem2)
    {
        goto Leave;
    }

    if (pItem)
    {
        int cchLen = wcslen(pItem->szPath);
        pItem2->szDisplay = NULL;
        pItem2->szPath = new WCHAR[cchLen+1];
        if (!pItem2->szPath)
        {
            goto Error;
        }
        else
        {
            if ( StringCchCopy(pItem2->szPath, cchLen+1, pItem->szPath) != S_OK )
            {
                goto Error;
            }
        }
    }

    hmenu = CreatePopupMenu();
    if (hmenu)
    {
        if (InsertMenu(hmenu, 0, MF_BYPOSITION | MF_OWNERDRAW,
                      FILEMENU_ID_UNINITIALIZED, (LPCTSTR)pItem2))
        {
            goto Leave;
        }
    }

Error:
    // Clean up
    delete pItem2;
    
    if (hmenu)
    {
        DestroyMenu(hmenu);
        hmenu = NULL;
    }

Leave:
    return hmenu;
}

BOOL CFilesysMenu::InsertItem(HMENU hmenu, int id, MenuData *pData)
{
    int pos = FindSortedIndex(hmenu, pData);
    HMENU hmenuSub = NULL;
    BOOL bRet = FALSE;
        
    if (pData->dwAttribs & FILE_ATTRIBUTE_DIRECTORY)
    {
        hmenuSub = CreateSubMenu(pData);
        if (hmenuSub)
        {
            bRet = InsertMenu(hmenu, pos, MF_BYPOSITION | MF_OWNERDRAW | MF_POPUP, (UINT)hmenuSub, (LPCTSTR)pData);

            if (pos == -1) 
            { 
                pos = GetMenuItemCount(hmenu) -1;
            }        
            bRet = SetItemData(hmenu, pos, pData);
        }
    }
    else
    {
        int prev = (pos==-1) ? GetMenuItemCount(hmenu)-1:pos-1;
        MenuData *pDataPrev = GetItemData(hmenu, prev, TRUE);
        if (!pDataPrev || (pDataPrev && CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, 
                pDataPrev->szDisplay, -1, pData->szDisplay, -1) != CSTR_EQUAL))
        {
            bRet = InsertMenu(hmenu, pos, MF_BYPOSITION | MF_OWNERDRAW, id, (LPCTSTR)pData);
        }
    }

    return bRet;
}


BOOL CFilesysMenu::InsertEmptyItem(HMENU hmenu)
{
    WCHAR szTemp[100];

    if (!LoadString(g_hInstance, IDS_EMPTY, szTemp, 100))
        return FALSE;

    return InsertMenu(hmenu, 0, MF_BYPOSITION | MF_GRAYED, FILEMENU_ID_EMPTY, szTemp);
}

int CFilesysMenu::FindSortedIndex(HMENU hmenu, MenuData *pItem)
{
    int min, max, index, maxSave; 
    MenuData *pItem2;

    min = 0;
    index = maxSave = max = GetMenuItemCount(hmenu) - 1; // -1 if empty menu (insert at end)
    
    while(min <= max) 
    {
        index = min + (max - min +1)/2;
        if (!(pItem2 = GetItemData(hmenu, index)))
        {
            return -1; 
        }
        if (CompareItems(pItem, pItem2) < 0) 
        {
            max = index-1; 
        }
        else
        {
            min = index+1;
            index++; // if its greater and we exit, we want the next index
        }
    }

    if (index > maxSave) 
    {
        index = -1; // inserting into a menu -1 is at the end
    }
    return index; 
}

BOOL   CFilesysMenu::SetItemData(HMENU hmenu, int index, MenuData *pData)
{
    MENUITEMINFO mii;
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_DATA;
    mii.dwItemData = (DWORD)pData;
    return SetMenuItemInfo(hmenu, index, TRUE, &mii);
}

MenuData *CFilesysMenu::GetItemData(HMENU hmenu, int index, BOOL fByPosition)
{
    MENUITEMINFO mii;
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_DATA;

    if (!GetMenuItemInfo(hmenu, index, fByPosition, &mii)) 
    {
        return 0;
    }
    return (MenuData *)mii.dwItemData;
}


HMENU CFilesysMenu::GetSubMenu(HMENU hmenu, int index)
{
    MENUITEMINFO mii;
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_SUBMENU;

    if (!GetMenuItemInfo(hmenu, index, TRUE, &mii)) 
    {
        return NULL;
    }
    return mii.hSubMenu;
}

int CFilesysMenu::CompareItems(MenuData *pItem1, MenuData *pItem2)
{
    if ((pItem1->dwAttribs & FILE_ATTRIBUTE_DIRECTORY) &&
        !(pItem2->dwAttribs & FILE_ATTRIBUTE_DIRECTORY)) 
    {
        return -1; 
    }
    if (!(pItem1->dwAttribs & FILE_ATTRIBUTE_DIRECTORY) &&
        (pItem2->dwAttribs & FILE_ATTRIBUTE_DIRECTORY)) 
    {
        return 1;
    }
    return CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, pItem1->szDisplay, -1, pItem2->szDisplay, -1) - 2;
}


void CFilesysMenu::RemoveAllItems(HMENU hmenu)
{
    MenuData *pData; 

    // smaller recursion footprint
    if (!hmenu)
    {
        return;
    }
    while (1) 
    {
        pData = GetItemData(hmenu, 0);
        RemoveAllItems(GetSubMenu(hmenu,0));

        if (!RemoveMenu(hmenu, 0, MF_BYPOSITION)) 
        {
            return; 
        }
        if (pData) 
        {
            delete pData;
        }
    }
}


BOOL CFilesysMenu::HandleInitMenuPopup(HMENU hmenu)
{
    UINT id = GetMenuItemID(hmenu, 0);
    if (id == FILEMENU_ID_UNINITIALIZED )
    {
        MenuData *pData = GetItemData(hmenu, 0);
        if (pData)
        {
            RemoveMenu(hmenu, 0, MF_BYPOSITION);
            if (wcsncmp(pData->szPath, szRegRoot, sizeof(szRegRoot)/sizeof(WCHAR)-1) == 0)
            {
                PopulateFromReg(hmenu, pData->szPath);
            }
            else
            {
                PopulateFromPath(hmenu, pData->szPath);
            }

            // if we didn't find anything make this an empty item
            if (GetMenuItemCount(hmenu) == 0)
            {
                InsertEmptyItem(hmenu);
            }

            delete pData;
        }
    }
    return TRUE;
}

BOOL CFilesysMenu::HandleMeasureItem(MEASUREITEMSTRUCT *lpmis)
{
    MenuData *pData;
    SIZE size;
    WCHAR szTemp[100];
    LPWSTR pszDisplay = L"";
       
    pData = (MenuData *)lpmis->itemData;


    if (pData->width && pData->height)
    {
        // already measured
        lpmis->itemHeight = pData->height;
        lpmis->itemWidth = pData->width;
        return TRUE;
    }

    size.cx = 94;
    size.cy = 0;

    if (lpmis->itemID)
    {
        // Height of 16 for icon + 3 pixels top & bottom
        lpmis->itemHeight = GetSystemMetrics(SM_CYSMICON) + 6;
    }
    else
    {
        // This means we have a separator
        lpmis->itemHeight = 7;
    }

    if (lpmis->itemID == FILEMENU_ID_EMPTY) 
    {
        if (!LoadString(g_hInstance, IDS_EMPTY, szTemp, 100))
        {
            szTemp[0] = L'\0';
        }
        pszDisplay = szTemp;
    } 
    else 
    {
        if (!pData->szDisplay) 
        {
            ASSERT(pData->szDisplay);
        } 
        else 
        {    
            pszDisplay = pData->szDisplay;
        }    
    }
    HDC hdc = GetDC(NULL);
    if (hdc)
    {
        GetTextExtentExPoint(hdc, pszDisplay, lstrlen(pszDisplay), MAX_TEXT_WIDTH, NULL, NULL, &size);
        if (size.cx > MAX_TEXT_WIDTH) 
        { 
            size.cx = MAX_TEXT_WIDTH;
        }
        ReleaseDC(NULL, hdc);
    }

    // reset height if need be 
    if (size.cy > GetSystemMetrics(SM_CYSMICON))
    {
        lpmis->itemHeight = size.cy + 10;        
    }

    // 4 pixel border + icon width + 4 pixel + text length + 8 pixels
    lpmis->itemWidth  = size.cx + GetSystemMetrics(SM_CXSMICON) + 8 + CX_CASCADE_WIDGET; 

    if (pData->szDisplay) 
    {
        pData->height = lpmis->itemHeight;
        pData->width = lpmis->itemWidth;
    }
    return TRUE;
}

BOOL CFilesysMenu::HandleDrawItem(DRAWITEMSTRUCT *lpdis)
{
    WCHAR szTemp[100];
    MenuData *pData;
    RECT rc;

    if (!lpdis)
    {
        return FALSE;
    }

    pData = (MenuData *)lpdis->itemData;
    if (!pData)
    {
        return FALSE;
    }

    rc = lpdis->rcItem;
 
    switch (lpdis->itemAction)
    {
        case ODA_DRAWENTIRE:
        case ODA_SELECT:
            if ((lpdis->itemState & ODS_SELECTED) && lpdis->itemID)
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
        
    if (lpdis->itemID)
    {
        if (lpdis->itemID == FILEMENU_ID_EMPTY || lpdis->itemID == FILEMENU_ID_UNINITIALIZED) 
        {
            if (LoadString(g_hInstance, IDS_EMPTY, szTemp, 100))
            {
                DrawText(lpdis->hDC, szTemp, lstrlen(szTemp), 
                    &rcT, DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
            }
        } 
        else if ((lpdis->itemID >= IDM_FAVORITES_FIRST__ &&  lpdis->itemID <= IDM_FAVORITES_LAST__ ) || pData->dwAttribs & FILE_ATTRIBUTE_DIRECTORY)
        {    
            SHFILEINFO sfi;
            const WCHAR *pszFooPath;

            if (lpdis->itemID <= wIDCounter)
            {
                pszFooPath = (LPCWSTR)pData->szPath;
            }
            else
            {
                pszFooPath= (pData->dwAttribs & FILE_ATTRIBUTE_DIRECTORY) ? L"\foo":L"foo.htm";
            }
            
            HIMAGELIST himl = (HIMAGELIST)SHGetFileInfo(pszFooPath, pData->dwAttribs, &sfi,
                    sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
            SetBkMode(lpdis->hDC, TRANSPARENT);    
            if (himl)
            {
                ImageList_Draw(himl, sfi.iIcon, lpdis->hDC, rc.left + 4, rc.top + 3, ILD_NORMAL);
            }

            rcT.left += 24; // skip the space, icon, space
            rcT.right -= CX_CASCADE_WIDGET; // room for the cascade widget

            if (pData->szDisplay)
            {
                DrawText(lpdis->hDC, pData->szDisplay, wcslen(pData->szDisplay),  &rcT,
                    DT_VCENTER | DT_SINGLELINE | DT_NOCLIP | DT_NOPREFIX | DT_END_ELLIPSIS);
            }
        }
        else
        {
            rcT.left += 24; // skip the space, icon, space
            if (pData->szDisplay)
            {
                DrawText(lpdis->hDC, pData->szDisplay, wcslen(pData->szDisplay),  &rcT,
                    DT_VCENTER | DT_SINGLELINE | DT_NOCLIP | DT_END_ELLIPSIS);
            }
   
        }
    }
    else
    {
        ASSERT(0);
        // Draw the separator

        POINT pt[2];

        pt[0].x = rc.left;
        pt[0].y = rc.top + 3;
        pt[1].x = rc.right - 3;
        pt[1].y = rc.top + 3;

        Polyline(lpdis->hDC, pt, 2);        
    }

    return TRUE;
}

DWORD CFilesysMenu::HandleMenuChar(HMENU hmenu, WCHAR keyChar)
{
    DWORD dwRet = MAKELONG(0, MNC_IGNORE);
    MenuData *pData;
    WCHAR *pChar;

    WCHAR lowCaseKeyChar = towlower(keyChar);

    MENUITEMINFO mii ={0};
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_DATA | MIIM_ID;
    
    if (GetMenuItemInfo(hmenu, 0, TRUE, &mii) && (mii.wID == ID_FAV_ADD))
    {
        pData = (MenuData *)mii.dwItemData;
        pChar = wcschr(pData->szDisplay, L'&');
        if (pChar && (tolower(pChar[1]) == lowCaseKeyChar))
        {
            dwRet = MAKELONG(0, MNC_EXECUTE);
            goto Handled;
        }
    }

    if (GetMenuItemInfo(hmenu, 1, TRUE, &mii) && (mii.wID == ID_FAV_ORG))
    {
        pData = (MenuData *)mii.dwItemData;
        pChar = wcschr(pData->szDisplay, L'&');
        if (pChar && (tolower(pChar[1]) == lowCaseKeyChar))
        {
            dwRet = MAKELONG(1, MNC_EXECUTE);
        }
    }

Handled:
    return dwRet;
}

void CFavoritesMenu::ExecuteCommandFromReg(LPCWSTR pszCmdPath, IWebBrowser2 *pBrowser, HWND hwndParent)
{
    HKEY hKey = NULL;
    LPWSTR pszPath = NULL;
    DWORD dwLen;
            
    // set up to look in the registry
    pszPath = new WCHAR[MAX_URL];
    dwLen = MAX_URL*sizeof(WCHAR);
    if (!pszPath)
    {
        goto Cleanup;
    }

    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, pszCmdPath, 0,0, &hKey))
    {
        goto Cleanup;
    }

    if (ERROR_SUCCESS != RegQueryValueEx(hKey, L"", NULL, NULL, (LPBYTE)pszPath, &dwLen))
    {
        goto Cleanup;
    }

    // Make sure pszPath is zero terminated
    pszPath[MAX_URL-1] = L'\0';

    // just navigate to this URL
    BOOL bNavigate = TRUE;
    if (!g_bBrowseInPlace && !PathCanBrowseInPlace(pszPath))
    {
        SHELLEXECUTEINFO sei = {0};
        sei.cbSize = sizeof(sei);
        sei.fMask = 0;
        sei.lpVerb = NULL;
        sei.lpFile = TEXT("explorer.exe");
        sei.lpParameters = pszPath;
        sei.lpDirectory = NULL;
        sei.nShow = SW_SHOWNORMAL;

        if (ShellExecuteEx(&sei))
        {
            bNavigate = FALSE;
        }
    }

    if (bNavigate)
    {
#pragma prefast(suppress:307, "This is an ok cast") 
        pBrowser->Navigate(pszPath, NULL, NULL, NULL, NULL);
    }
            
Cleanup:
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    if (pszPath)
    {
        delete [] pszPath;
    }
}

void CFavoritesMenu::ExecuteCommandFromFile(LPCWSTR pszCmdPath, IWebBrowser2 *pBrowser, HWND hwndParent)
{
    MenuData *pData= NULL;
    LPWSTR pszPath = new WCHAR[MAX_URL];

    if (!pszPath)
    {
        goto Cleanup;
    }
       
    if ( StringCchCopy(pszPath, MAX_URL, pszCmdPath) == S_OK )
    {
        // handle stuff we know about
        WCHAR *pszExt;

Restart:               
        int i = wcslen(pszPath);
               
        while(i && pszPath[i] != L'.')
        {
            i--;
        }

        if (pszPath[i] == L'.')
        {
            pszExt = &pszPath[i];
            if (!wcsicmp(pszExt, L".lnk"))
            {
                LPWSTR pszTemp = new WCHAR[MAX_URL];
                if (!pszTemp)
                {
                    goto Cleanup;
                }

                if (SHGetShortcutTarget(pszPath, pszTemp, MAX_URL))
                {
                    delete [] pszPath;
                    pszPath = pszTemp;
                    goto Restart;
                }
                else
                {
                    delete [] pszTemp;
                }
            }
            else if (!wcsicmp(pszExt, L".url"))
            {
                // a URL file
                CUrlFile theURL;

                if (S_OK == theURL.OpenFile(pszPath, FALSE))
                {
                    theURL.ReadURL(pszPath, MAX_URL);
                    if (pBrowser)
                    {
                        BOOL bNavigate = TRUE;
                        if (!g_bBrowseInPlace && !PathCanBrowseInPlace(pszPath))
                        {
                            SHELLEXECUTEINFO sei = {0};
                            sei.cbSize = sizeof(sei);
                            sei.fMask = 0;
                            sei.lpVerb = NULL;
                            sei.lpFile = TEXT("explorer.exe");
                            sei.lpParameters = pszPath;
                            sei.lpDirectory = NULL;
                            sei.nShow = SW_SHOWNORMAL;

                            if (ShellExecuteEx(&sei))
                            {
                                bNavigate = FALSE;
                            }
                        }

                        if (bNavigate)
                        {
#pragma prefast(suppress:307, "This is an ok cast") 
                            pBrowser->Navigate(pszPath, NULL, NULL, NULL, NULL);
                        }
                    }
                    goto Cleanup;
                }
            }
            else if (!wcsicmp(pszExt, L".htm") || !wcsicmp(pszExt, L".html"))
            {
                // handle html files in favorites
                if (pBrowser)
                {
                        BOOL bNavigate = TRUE;
                        if (!g_bBrowseInPlace && !PathCanBrowseInPlace(pszPath))
                        {
                            SHELLEXECUTEINFO sei = {0};
                            sei.cbSize = sizeof(sei);
                            sei.fMask = 0;
                            sei.lpVerb = NULL;
                            sei.lpFile = TEXT("explorer.exe");
                            sei.lpParameters = pszPath;
                            sei.lpDirectory = NULL;
                            sei.nShow = SW_SHOWNORMAL;

                            if (ShellExecuteEx(&sei))
                            {
                                bNavigate = FALSE;
                            }
                        }

                        if (bNavigate)
                        {
#pragma prefast(suppress:307, "This is an ok cast") 
                            pBrowser->Navigate(pszPath, NULL, NULL, NULL, NULL);
                        }
                }
                goto Cleanup;
            }
            else
            {
                goto DoShellExecute;
            }
        }
        else
        {
DoShellExecute:
            SHELLEXECUTEINFO sei;

            sei.cbSize = sizeof(sei);
            sei.fMask = SEE_MASK_FLAG_NO_UI;
            sei.hwnd = hwndParent;
            sei.lpVerb = NULL;
            sei.lpFile = pszPath;
            sei.lpParameters = NULL;
            sei.lpDirectory = NULL;
            sei.nShow = SW_SHOWNORMAL;

            ShellExecuteEx(&sei);
        }
    }
Cleanup:
    if (pszPath)
    {
        delete [] pszPath;
    }
}

void CFavoritesMenu::ExecuteCommand(HMENU hmenu, int nID, IWebBrowser2 *pBrowser, HWND hwndParent)
{
    if (nID < IDM_FAVORITES_FIRST__ || nID > IDM_FAVORITES_LAST__)
    {
        return;  // we were passed an incorrect ID
    }

    MenuData *pData = GetItemData(hmenu, nID, FALSE);
    if (!pData)
    {
        return;
    }

    if (nID <= wIDCounter)
    {
        ExecuteCommandFromFile(pData->szPath, pBrowser, hwndParent);
    }
    else if (nID >= wIDFavReg)
    {
        ExecuteCommandFromReg(pData->szPath, pBrowser, hwndParent);
    }
}

void CFavoritesMenu::CreateFavoritesMenu(HMENU hmenu)
{
    MenuData *pExtra1, *pExtra2;
    LPWSTR pszFavorites;
    
	// cache off this instance of the menu
	if(NULL != m_hMenu && m_hMenu != hmenu)
	{
		// clean out any items in the previous one
		RemoveAllItems(m_hMenu);	
	}
	m_hMenu = hmenu;

    Reset();

    RemoveAllItems(hmenu);

    pszFavorites = (WCHAR *)LocalAlloc(0, MAX_PATH * sizeof(WCHAR));
    if (!pszFavorites)
    {
        goto Leave;
    }

    SHGetSpecialFolderPath(NULL, pszFavorites, CSIDL_FAVORITES, FALSE);
    int i = wcslen(pszFavorites);
    if (i == 0)
    {
        goto Leave;
    }

    if ( (pszFavorites[i-1] != L'\\') && ( StringCchCat(pszFavorites, MAX_PATH, L"\\") != S_OK ) )
    {
        goto Leave;
    }

    PopulateFromPath(hmenu, pszFavorites);
    PopulateFromReg(hmenu, szRegRoot);

    // if we didn't find anything make this an empty item
    if (GetMenuItemCount(hmenu) == 0)
    {
        InsertEmptyItem(hmenu);
    }

    pExtra1 = new MenuData;
    pExtra2 = new MenuData;
    if (pExtra1 && pExtra2)
    {
        pExtra1->szDisplay = new WCHAR[100];
        if (pExtra1->szDisplay)
        {
            LoadString(g_hInstance, IDS_ORGANIZEFAVORITES, pExtra1->szDisplay, 100);
        }

        pExtra2->szDisplay = new WCHAR[100];
        if (pExtra2->szDisplay)
        {
            LoadString(g_hInstance,IDS_ADDTOFAVORITES , pExtra2->szDisplay, 100);
        }

        InsertMenu(hmenu, 0, MF_SEPARATOR | MF_BYPOSITION, 0, 0);

        if (!InsertMenu(hmenu, 0, MF_BYPOSITION | MF_OWNERDRAW, ID_FAV_ORG, (LPCTSTR)pExtra1))
        {
            delete pExtra1;
        }

        if (!InsertMenu(hmenu, 0, MF_BYPOSITION | MF_OWNERDRAW, ID_FAV_ADD, (LPCTSTR)pExtra2))
        {
            delete pExtra2;
        }
    }
    else
    {
        if (pExtra1)
        {
            delete pExtra1;
        }

        if (pExtra2)
        {
            delete pExtra2;
        }
    }

Leave:
    if (pszFavorites)
    {
        LocalFree(pszFavorites);
    }

}

void CFavoritesMenu::DoOrganizeFavorites(HWND hwndParent)
{                  
    SHELLEXECUTEINFO sei;
    WCHAR *szRootPath = (WCHAR *)LocalAlloc(0, MAX_PATH*sizeof(WCHAR));
               
    if (szRootPath)
    {
        SHGetSpecialFolderPath(hwndParent, szRootPath, CSIDL_FAVORITES, TRUE);
        sei.cbSize = sizeof(sei);
        sei.fMask = SEE_MASK_FLAG_NO_UI;
        sei.hwnd = hwndParent;
        sei.lpVerb = NULL;
        sei.lpFile = szRootPath;
        sei.lpParameters = NULL;
        sei.lpDirectory = NULL;
        sei.nShow = SW_SHOWNORMAL;
        
        ShellExecuteEx(&sei);
        LocalFree(szRootPath);
    }
}
/*
BOOL CFavoritesMenu::AddFavoritesReg(HWND hwndParent, LPCWSTR szURL, LPCWSTR szName)
{
    BOOL bRet = FALSE;
    HKEY hKey = NULL;
    HKEY hKey2 = NULL;
    HKEY hKey3 = NULL;
    DWORD cchLen;
    DWORD dwDisp = NULL;

    EDITFAVORITESTRUCT EFS;

    EFS.szPath = new WCHAR[MAX_URL];
    if (!EFS.szPath)
    {
        goto Cleanup;
    }
    EFS.szPath[0] = 0;
    EFS.cchPath = MAX_URL;
    EFS.szFavURL = new WCHAR[MAX_URL];
    if (!EFS.szFavURL)
    {
        goto Cleanup;
    }
    EFS.cchFavURL = MAX_URL;
    StringCchCopy(EFS.szFavURL, MAX_URL, szURL);
    EFS.szFavName = new WCHAR[1024];
    if (!EFS.szFavName)
    {
        goto Cleanup;
    }
    EFS.cchFavName = 1024;
    StringCchCopy(EFS.szFavName, 1024, szName);

    if (IDOK != DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_FAV_ADD),  hwndParent, AddFavDialogProc, (LONG)&EFS))
    {
        goto Cleanup;
    }

    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER, szRegRoot, NULL, NULL, NULL, NULL, NULL, &hKey, &dwDisp))
    {
        goto Cleanup;
    }

    if (EFS.szPath[0] != 0)
    {
        if (ERROR_SUCCESS != RegOpenKeyEx(hKey, EFS.szPath, NULL, NULL, &hKey2))
        {
            hKey2 = hKey;
            hKey = NULL;
        }
    }
    else
    {
        hKey2 = hKey;
        hKey = NULL;
    }

    if (ERROR_SUCCESS != RegCreateKeyEx(hKey2, EFS.szFavName, NULL, NULL, NULL, NULL, NULL, &hKey3, &dwDisp))
    {
        goto Cleanup;
    }

    if (dwDisp == REG_OPENED_EXISTING_KEY)
    {
        goto Cleanup;
    }

    cchLen = (wcslen(EFS.szFavURL) + 1)*sizeof(WCHAR);
    if (ERROR_SUCCESS == RegSetValueEx(hKey3, NULL, 0, REG_SZ, (LPBYTE)EFS.szFavURL, cchLen))
    {
        bRet = TRUE;
    }

Cleanup:
    if (EFS.szPath)
    {
        delete EFS.szPath;
    }

    if (EFS.szFavURL)
    {
        delete EFS.szFavURL;
    }

    if (EFS.szFavName)
    {
        delete EFS.szFavName;
    }

    if (hKey)
    {
        RegCloseKey(hKey);
    }

    if (hKey2)
    {
        RegCloseKey(hKey2);
    }

    if (hKey3)
    {
        RegCloseKey(hKey3);
    }

    return bRet;    
}
*/
BOOL CFavoritesMenu::AddFavoritesFile(HWND hwndParent, LPCWSTR szURL, LPCWSTR szName)
{
    BOOL bRet = FALSE;
    CUrlFile  theURL;

    WCHAR *pszDlgTitle = new WCHAR[MAX_PATH];
    WCHAR *pszFavoritesDir = new WCHAR[MAX_PATH];
    WCHAR *pszTitle = new WCHAR[MAX_PATH];
    WCHAR *pszTemp;
                                
    if (pszDlgTitle && pszFavoritesDir && pszTitle)
    {
        BOOL fCharValidate = TRUE;
        size_t cchIgnore = 0;
        if (wcsncmp(szURL, L"file", 4) == 0)
        {
            // szName is system path, get file/folder name only
            for (int i=0; szName[i] != L'\0'; i++)
            {
                cchIgnore = (szName[i] == L'\\') ? i+1:cchIgnore;
            }
            fCharValidate = FALSE;
        }
        if ( StringCchCopy(pszTitle, MAX_PATH, szName+cchIgnore) != S_OK )
        {
            goto exit;
        }
        pszTemp = pszTitle;
        if (fCharValidate)
        {
            while(*pszTemp)
            {
                *pszTemp = MakeFileChar(*pszTemp);
                pszTemp++;
            }
            // Remove preceding underscore(s).
            for (pszTemp= pszTitle; *pszTemp == L'_'; pszTemp++);
        }                                                
        // StringCchCopy(szUrl, MAX_URL, szRootPath);
        SHGetSpecialFolderPath(hwndParent, pszFavoritesDir, CSIDL_FAVORITES, FALSE);

        WCHAR szFilterBuffer[100];
        int iLen;
        OPENFILENAME ofn = {0};
                        
        // OFN_EXPLORER | OFN_LONGNAMES are ignored since Explorer user interface is always used.
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT; // |OFN_NODEREFERENCELINKS;
        LoadString(g_hInstance, IDS_ADDFAVTITLE, pszDlgTitle, MAX_PATH);

        //Should be using a real filter - IDS_FAVORITEFILTER
        LoadString(g_hInstance, IDS_IE_TTSTR6, szFilterBuffer, 100);
        wcscat(szFilterBuffer, TEXT(" (*.url)"));
        iLen = wcslen(szFilterBuffer);
        wcscpy(szFilterBuffer + iLen + 1, TEXT("*.url"));
        *(szFilterBuffer + iLen + 7) = 0;  // Double NULL terminate

        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hwndParent;
        ofn.lpstrFilter = szFilterBuffer;
        ofn.lpstrDefExt = TEXT("url"); //ofn.lpstrDefExt = TEXT("lnk");
        ofn.nMaxFile = MAX_PATH - 1;
        ofn.lpstrFile = pszTemp;
        ofn.lpstrTitle = pszDlgTitle;
        ofn.lpstrInitialDir = pszFavoritesDir;

        if (GetSaveFileName(&ofn))
        {
            if (ofn.Flags & OFN_EXTENSIONDIFFERENT)
            {
                if (lstrlen(pszTemp) < MAX_PATH-5)
                {
                    lstrcat(pszTemp, TEXT(".url"));
                }
            }
                        
            if (S_OK == theURL.OpenFile(pszTemp, TRUE))
            {
                theURL.WriteURL((WCHAR *)szURL);
                theURL.CloseFile();
                bRet = TRUE;
            }
        }                 
    }

exit:    
    if (pszDlgTitle)
    {
        delete [] pszDlgTitle;
    }

    if (pszFavoritesDir)
    {
        delete [] pszFavoritesDir;
    }

    if (pszTitle)
    {
        delete [] pszTitle;
    }

    return bRet;
}

void CFavoritesMenu::DoAddToFavorites(HWND hwndParent, LPCWSTR szURL, LPCWSTR szTitle)
{
    AddFavoritesFile(hwndParent, szURL, szTitle);
}

