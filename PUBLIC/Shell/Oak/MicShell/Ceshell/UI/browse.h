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

#include "ceshpriv.h"

/* 
	SHBindToObject is used often in desktop code, but did not exist in CE for Cedar.  
	This has been added for support for the SHBrowseForFolder function and can be 
	removed if it is removed from that function.
*/	
STDAPI SHBindToObject (IShellFolder *psf, REFIID riid, LPCITEMIDLIST pidl, void **ppvOut);

#define IS_INTRESOURCE(_r) (((ULONG_PTR)(_r) >> 16) == 0)

//network stuff for shitemid stuff - remove if network support is not needed
#define RESOURCEDISPLAYTYPE_DOMAIN         0x00000001
#define RESOURCEDISPLAYTYPE_SERVER         0x00000002
#define RESOURCEDISPLAYTYPE_SHARE          0x00000003

//SHITEMID stuff
#define SHID_INGROUPMASK        0x0f

#define SHID_NET                0x40
#define SHID_NET_DOMAIN         (SHID_NET  | RESOURCEDISPLAYTYPE_DOMAIN)         // 0x41
#define SHID_NET_SERVER         (SHID_NET  | RESOURCEDISPLAYTYPE_SERVER)         // 0x42
#define SHID_NET_SHARE          (SHID_NET  | RESOURCEDISPLAYTYPE_SHARE)          // 0x43

// Structure to pass information to browse for folder dialog
typedef struct _bfsf
{
    HWND        hwndOwner;
    LPCITEMIDLIST pidlRoot;      // Root of search.  Typically desktop or my net
    LPTSTR        pszDisplayName;// Return display name of item selected.
    int         *piImage;      // where to return the Image index.
    LPCTSTR      lpszTitle;      // resource (or text to go in the banner over the tree.
    UINT         ulFlags;       // Flags that control the return stuff
    BFFCALLBACK  lpfn;
    LPARAM      lParam;
    HWND         hwndDlg;       // The window handle to the dialog
    HWND         hwndTree;      // The tree control.
    HWND        hwndEdit;
    HTREEITEM    htiCurParent;  // tree item associated with Current shell folder
    IShellFolder *psfParent;    // Cache of the last IShell folder I needed...
    LPITEMIDLIST pidlCurrent;   // IDlist of current folder to select
    BOOL         fShowAllObjects:1; // Should we Show all ?
} BFSF, *PBFSF;

inline void DlgEnableOk(HWND hwndDlg, LPARAM lParam)
{
    EnableWindow(GetDlgItem(hwndDlg, IDOK), (BOOL)lParam);
    return;
}


/* 
	prototypes for browse.cpp function
*/
typedef int (CALLBACK* BFFCALLBACK)(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
BOOL CALLBACK _BrowseForFolderBFSFDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL _BrowseForFolderOnBFSFInitDlg(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void _BFSFHandleSelChanged(PBFSF pbfsf, LPNM_TREEVIEW lpnmtv);
void _BFSFGetDisplayInfo(PBFSF pbfsf, TV_DISPINFO *lpnm);
LPITEMIDLIST _BFSFUpdateISHCache(PBFSF pbfsf, HTREEITEM hti, LPITEMIDLIST pidlItem);
void _BrowseForFolderOnBFSFCommand(PBFSF pbfsf, int id, HWND hwndCtl, UINT codeNotify);
HTREEITEM _BFSFAddItemToTree(HWND hwndTree, HTREEITEM htiParent, LPITEMIDLIST pidl, int cChildren);
int _BFSFCallback(PBFSF pbfsf, UINT uMsg, LPARAM lParam);
LPITEMIDLIST _BFSFGetIDListFromTreeItem(HWND hwndTree, HTREEITEM hti);
void _BFSFSort(PBFSF pbfsf, HTREEITEM hti, IShellFolder *psf);
BOOL _BFSFHandleItemExpanding(PBFSF pbfsf, LPNM_TREEVIEW lpnmtv);
int CALLBACK _BFSFTreeCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
void _BFSFHandleDeleteItem(PBFSF pbfsf, LPNM_TREEVIEW lpnmtv);
BOOL _BFSFSetSelection(PBFSF pbfsf, BOOL blParamIsPath, LPARAM lParam);
HTREEITEM _BFSFFindItem(HWND hWnd, LPITEMIDLIST pidl);
BOOL _BFSFSelectPidl(PBFSF pbfsf, LPCITEMIDLIST pidl);

