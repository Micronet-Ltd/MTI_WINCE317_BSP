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
#ifndef _H_FILEMENU_
#define _H_FILEMENU_

#define FILEMENU_ID_EMPTY    0xFFFF    
#define FILEMENU_ID_UNINITIALIZED  0xFFFE 
#define MAX_TEXT_WIDTH		200
#define CX_CASCADE_WIDGET	8

#define DT_END_ELLIPSIS 0x00008000

#ifndef MAX_URL
#define MAX_URL  2060
#endif

struct MenuData
{
        WCHAR *szDisplay;
        WCHAR *szPath;
        DWORD dwAttribs;
        WORD   width;
        WORD   height;

        MenuData() {
                szDisplay = NULL;
                szPath = NULL;
                dwAttribs = 0;
                width = 0;
                height = 0;
        }

        ~MenuData()
        {
            if(szDisplay)
                delete szDisplay;
            if(szPath)
                delete szPath;
        }
};



class CFilesysMenu
{
public:
        BOOL PopulateFromPath(HMENU hmenu, LPCWSTR szPath);
        BOOL PopulateFromReg(HMENU hmenu, LPCWSTR szRegKey);

        
        BOOL HandleInitMenuPopup(HMENU hmenu);
        BOOL HandleMeasureItem(MEASUREITEMSTRUCT *lpmis);
        BOOL HandleDrawItem(DRAWITEMSTRUCT *lpdis);
        DWORD HandleMenuChar(HMENU hmenu, WCHAR keyChar);

        void Reset() {
            wIDCounter = IDM_FAVORITES_FIRST__-1;
            wIDFavReg = IDM_FAVORITES_LAST__+1;
        }

        WORD wIDCounter_AddRef()
        {
            if((wIDFavReg -wIDCounter) > 1)
            {
                return ++wIDCounter;
            }
            return -1;
        }

        void wIDCounter_Release()
        {
            if (wIDCounter >= IDM_FAVORITES_FIRST__)
            {
                --wIDCounter;
            }
        }

        WORD wIDFavReg_AddRef()
        {
            if((wIDFavReg -wIDCounter) > 1)
            {
                return --wIDFavReg;
            }
            return -1;
        }

        void wIDFavReg_Release()
        {
            if (wIDFavReg <= IDM_FAVORITES_LAST__)
            {
                ++wIDFavReg;
            }
        }

    protected:
        BOOL InsertItem(HMENU hmenu, int id, MenuData *pData);
        BOOL InsertEmptyItem(HMENU hmenu);
        void RemoveAllItems(HMENU hmenu);
        
        int CompareItems(MenuData *pItem1, MenuData *pItem2);
        int FindSortedIndex(HMENU hmenu, MenuData *pItem);
        HMENU CreateSubMenu(MenuData *pItem);
        MenuData *GetItemData(HMENU hmenu, int index, BOOL fByPosition=TRUE);
        HMENU GetSubMenu(HMENU hmenu, int index);
        BOOL   SetItemData(HMENU hmenu, int index, MenuData *pData);

        WORD wIDCounter;
        WORD wIDFavReg;
};

class CFavoritesMenu : public CFilesysMenu
{

    public:
        CFavoritesMenu() : m_hMenu(NULL)
            {
                Reset();
            }

        void   ExecuteCommand(HMENU hmenu, int nID, IWebBrowser2 *pBrowser, HWND hwndParent);
        void CreateFavoritesMenu(HMENU hmenu);

        void DoOrganizeFavorites(HWND hwndParent);
        void DoAddToFavorites(HWND hwndParent, LPCWSTR szURL, LPCWSTR szName);

	void Cleanup() { RemoveAllItems(m_hMenu); }

    private:
        void   ExecuteCommandFromFile(LPCWSTR pszCmdPath, IWebBrowser2 *pBrowser, HWND hwndParent);
        void   ExecuteCommandFromReg(LPCWSTR pszCmdPath, IWebBrowser2 *pBrowser, HWND hwndParent);
        BOOL AddFavoritesFile(HWND hwndParent, LPCWSTR szURL, LPCWSTR szName);
        //BOOL AddFavoritesReg(HWND hwndParent , LPCWSTR szURL, LPCWSTR szName);

	HMENU m_hMenu;    
};

#endif
