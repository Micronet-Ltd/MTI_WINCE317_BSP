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

#include "browse.h"
#include "api.h"
#include "aygutils.h"
#include "guid.h"
#include "idlist.h"
#include "resource.h"
#include <shellsdk.h>

/*++
Routine Description:
    The dialog procedure for processing the browse for starting folder dialog.

    
Arguments:
    Regular DlgProc stuff
    
Return Value:
    BOOL value based on processed message
    
--*/
BOOL CALLBACK _BrowseForFolderBFSFDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PBFSF pbfsf = (PBFSF)GetWindowLong(hwndDlg, DWL_USER);

    switch (msg) {
    HANDLE_MSG(pbfsf, WM_COMMAND, _BrowseForFolderOnBFSFCommand);

    case WM_INITDIALOG: 
        LoadAygshellLibrary();
        return (BOOL)HANDLE_WM_INITDIALOG(hwndDlg, wParam, lParam, _BrowseForFolderOnBFSFInitDlg);
        break;
        
    case WM_DESTROY:
    
        if (pbfsf->psfParent)
        {
            pbfsf->psfParent->Release();
            pbfsf->psfParent = NULL;
        }
        FreeAygshellLibrary();
        break;

    case BFFM_SETSELECTION:
        return _BFSFSetSelection(pbfsf, (BOOL)wParam, lParam);

    case BFFM_ENABLEOK:
        DlgEnableOk(hwndDlg, lParam);
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code)
        {
        case TVN_GETDISPINFO:
            _BFSFGetDisplayInfo(pbfsf, (TV_DISPINFO *)lParam);
            break;

        case TVN_ITEMEXPANDING:
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            if (!_BFSFHandleItemExpanding(pbfsf, (LPNM_TREEVIEW)lParam))
                SetCursor(LoadCursor(NULL, IDC_ARROW));
            break;

        case TVN_ITEMEXPANDED:
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            break;

        case TVN_DELETEITEM:
            _BFSFHandleDeleteItem(pbfsf, (LPNM_TREEVIEW)lParam);
            break;

        case TVN_SELCHANGED:
            _BFSFHandleSelChanged(pbfsf, (LPNM_TREEVIEW)lParam);
            break;
        }
        break;

    case WM_HELP:
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


/*++
Routine Description:
    Initialization routine for SHBrowseForFolder dialog
    
Arguments:
    hwnd            handle to this dialog
    hwndFocus       handle to window that previously had focus
    lParam          a pointer to our BFSF structure
    
Return Value:
    TRUE if initialization is successful
    
--*/
BOOL _BrowseForFolderOnBFSFInitDlg(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    HTREEITEM hti = NULL;
    PBFSF pbfsf = (PBFSF)lParam;
    HWND hwndTree = NULL;
    HIMAGELIST himl = NULL;
    HRESULT hr = E_FAIL;

    // Initalize the SipPref control if necessary
    AygAddSipprefControl( hwnd );
    
    SetWindowLong(hwnd, DWL_USER, lParam);
    pbfsf->hwndDlg = hwnd;
    hwndTree = pbfsf->hwndTree = GetDlgItem(hwnd, IDC_FOLDERLIST);

    // Update the user specified text
    if (pbfsf->lpszTitle)
    {
        SetDlgItemText(hwnd, IDC_BROWSETITLE, pbfsf->lpszTitle);
    }
    if (hwndTree)
    {
        UINT swpFlags = SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER
                | SWP_NOACTIVATE;
        RECT rc;
        POINT pt = {0,0};

        GetClientRect(hwndTree, &rc);
        MapWindowPoints(hwndTree, hwnd, (POINT*)&rc, 2);
        pbfsf->hwndEdit = GetDlgItem(hwnd, IDC_BROWSEEDIT);

        if (!(pbfsf->ulFlags & BIF_STATUSTEXT)) 
        {
            HWND hwndStatus = GetDlgItem(hwnd, IDC_BROWSESTATUS);
            ShowWindow(hwndStatus, SW_HIDE);
            MapWindowPoints(hwndStatus, hwnd, &pt, 1);
            rc.top = pt.y;
            swpFlags =  SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOACTIVATE;
        }

        if (pbfsf->ulFlags & BIF_EDITBOX) 
        {
            RECT rcT;
            GetClientRect(pbfsf->hwndEdit, &rcT);
            SetWindowPos(pbfsf->hwndEdit, NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            rc.top += (rcT.bottom - rcT.top) + GetSystemMetrics(SM_CYEDGE) * 4;
            swpFlags =  SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOACTIVATE;
        } 
        else 
        {
            DestroyWindow(pbfsf->hwndEdit);
            pbfsf->hwndEdit = NULL;
        }

        SHFILEINFO sfi = {0};
        himl = (HIMAGELIST) SHGetFileInfo(__TEXT(""), 0, &sfi, sizeof(SHFILEINFO),
                                          SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
        TreeView_SetImageList(hwndTree, himl, TVSIL_NORMAL);

        SetWindowLong(hwndTree, GWL_EXSTYLE, GetWindowLong(hwndTree, GWL_EXSTYLE) | WS_EX_CLIENTEDGE);

        // Now try to get this window to know to recalc
        SetWindowPos(hwndTree, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, swpFlags);
    }

    // Handle the case where the user specifies a root pidl
    if (pbfsf->pidlRoot)
    {
        LPITEMIDLIST pidl;
       
        if (IS_INTRESOURCE(pbfsf->pidlRoot)) 
        {
            pidl = NULL;
            hr = SHGetSpecialFolderLocation( hwnd, PtrToUlong((void *)pbfsf->pidlRoot), &pidl);
            ASSERT(SUCCEEDED(hr) && pidl);
            //pidl = SHCloneSpecialIDList(NULL, PtrToUlong((void *)pbfsf->pidlRoot), TRUE);
        }
        else 
        {
            pidl = ILCopy(pbfsf->pidlRoot, IL_ALL);
        }
        // Now lets insert the Root object
        hti = _BFSFAddItemToTree(hwndTree, TVI_ROOT, pidl, 1);
        // Still need to expand below this point. to the starting location
        // That was passed in. But for now expand the first level.
        TreeView_Expand(hwndTree, hti, TVE_EXPAND);
    }
    else
    {
        LPITEMIDLIST pidlRoot = NULL;
        hr = SHGetSpecialFolderLocation(hwnd, CSIDL_DRIVES, &pidlRoot);
        ASSERT(SUCCEEDED(hr) && pidlRoot);

        HTREEITEM htiRoot = _BFSFAddItemToTree(hwndTree, TVI_ROOT, pidlRoot, 1);

        // Expand the first level under the desktop
        TreeView_Expand(hwndTree, htiRoot, TVE_EXPAND);

/* - We would only do this if the root was the desktop, but we set the root as
   - Drives (My Computer) so this is not necessary

        // Pre-expand the first level
        BOOL bFoundDrives = FALSE;
        hti = TreeView_GetChild(hwndTree, htiRoot);
        while (hti && !bFoundDrives)
        {
            LPITEMIDLIST pidl = _BFSFGetIDListFromTreeItem(hwndTree, hti);
            if (pidl)
            {

                LPITEMIDLIST pidlDrives = NULL;
                hr = SHGetSpecialFolderLocation(hwnd, CSIDL_DRIVES, &pidlDrives);
                ASSERT(SUCCEEDED(hr) && pidlDrives);
                if (pidlDrives)
                {
                    bFoundDrives = ILIsEqual(pidl, pidlDrives);
                    if (bFoundDrives)
                    {
                        TreeView_Expand(hwndTree, hti, TVE_EXPAND);
                        TreeView_SelectItem(hwndTree, hti);
                    }
                    ILFree(pidlDrives);
                }
                ILFree(pidl);
            }
            hti = TreeView_GetNextSibling(hwndTree, hti);
        }
*/
    }

    NM_TREEVIEW nmtv;
    hti = TreeView_GetSelection(hwndTree);
    if (hti) 
    {
        TV_ITEM ti;
        ti.mask = TVIF_PARAM;
        ti.hItem = hti;
        TreeView_GetItem(hwndTree, &ti);
        nmtv.itemNew.hItem = hti;
        nmtv.itemNew.lParam = ti.lParam;

        _BFSFHandleSelChanged(pbfsf, &nmtv);
    }

#ifdef DEBUG
    //Check for invalid flags - no need to fail here - we can act on the other flags
    if ((pbfsf->ulFlags & BIF_BROWSEFORCOMPUTER) != 0)
    {
        DEBUGMSG(ZONE_WARNING, (_T("BIF_BROWSEFORCOMPUTER not supported")));
    }
    if ((pbfsf->ulFlags & BIF_BROWSEFORPRINTER) != 0)
    {
        DEBUGMSG(ZONE_WARNING, (_T("BIF_BROWSEFORPRINTER not supported")));
    }
    if ((pbfsf->ulFlags & BIF_DONTGOBELOWDOMAIN) != 0)
    {
        DEBUGMSG(ZONE_WARNING, (_T("BIF_DONTGOBELOWDOMAIN not supported")));
    }
    if ((pbfsf->ulFlags & BIF_RETURNFSANCESTORS) != 0)
    {
        DEBUGMSG(ZONE_WARNING, (_T("BIF_RETURNFSANCESTORS not supported")));
    }
    if ((pbfsf->ulFlags & BIF_RETURNONLYFSDIRS) != 0)
    {
        DEBUGMSG(ZONE_WARNING, (_T("BIF_RETURNONLYFSDIRS not supported")));
    }

#endif //DEBUG

    AygInitDialog( hwnd, SHIDIF_SIZEDLG );
    _BFSFCallback(pbfsf, BFFM_INITIALIZED, 0);

    return TRUE;
}


/*++
Routine Description:
    Gets an ITEMIDLIST pointer based on a TreeItem 
    
Arguments:
    hwndTree        handle to the TreeView window
    hti             Specific tree item (or NULL for currently selected item)
    
Return Value:
    A pointer to a valid ITEMIDLIST if successful
    NULL on failure
    
--*/
LPITEMIDLIST _BFSFGetIDListFromTreeItem(HWND hwndTree, HTREEITEM hti)
{
    LPITEMIDLIST pidl;
    LPITEMIDLIST pidlT;
    TV_ITEM tvi;

    // If no hti passed in, get the selected one.
    if (hti == NULL)
    {
        hti = TreeView_GetSelection(hwndTree);
        if (hti == NULL)
            return NULL;
    }

    // now lets get the information about the item
    tvi.mask = TVIF_PARAM | TVIF_HANDLE;
    tvi.hItem = hti;
    if (!TreeView_GetItem(hwndTree, &tvi))
        return(NULL);   // Failed again

    pidl = ILCopy((LPITEMIDLIST)tvi.lParam, IL_ALL);

    // Now walk up parents.
    while ((NULL != (tvi.hItem = TreeView_GetParent(hwndTree, tvi.hItem))) && pidl)
    {
        if (!TreeView_GetItem(hwndTree, &tvi))
            return(pidl);
        pidlT = ILConcatenate((LPITEMIDLIST)tvi.lParam, pidl);

        ILFree(pidl);

        pidl = pidlT;

    }

    // Since we only deal with items that are relative to the
    // desktop strip off that pidl here
    LPITEMIDLIST pidlRoot = ILCopy(pidl, 1);
    if (pidlRoot)
    {
        if (ILIsNameSpace(pidlRoot, CLSID_CEShellDesktop))
        {
            pidlT = ILCopy(ILNext(pidl), IL_ALL);
            ILFree(pidl);
            pidl = pidlT;
        }

        ILFree(pidlRoot);
    }

    return pidl;
}



/*++
Routine Description:
    Handles selection changes in the treeview control
    
Arguments:
    pbfsf       pointer to our BFSF structure
    lpnmtv      pointer to a NMTREEVIEW structure
    
Return Value:
    none
    
--*/
void _BFSFHandleSelChanged(PBFSF pbfsf, LPNM_TREEVIEW lpnmtv)
{
    LPITEMIDLIST pidl = NULL;

    if (pbfsf->ulFlags & BIF_EDITBOX) 
    {
        TCHAR szText[MAX_PATH];        // update the edit box
        TVITEM tvi;
        
        szText[0] = 0;
        tvi.mask = TVIF_TEXT;
        tvi.hItem = lpnmtv->itemNew.hItem;
        tvi.pszText = szText;
        tvi.cchTextMax = lengthof(szText);
        TreeView_GetItem(pbfsf->hwndTree, &tvi);
        SetWindowText(pbfsf->hwndEdit, szText);
    }
    
    if (pbfsf->lpfn) 
    {
        pidl = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree, lpnmtv->itemNew.hItem);
        if (pidl)
        {
            _BFSFCallback(pbfsf, BFFM_SELCHANGED, (LPARAM)pidl);
            ILFree(pidl);
        }
    }
}


/*++
Routine Description:
    Get the right icon info to display for a tree item
    
Arguments:
    pbfsf       a pointer to our BFSF structure
    lpnm        pointer to a TV_DISPINFO structure
    
Return Value:
    none
    
--*/
void _BFSFGetDisplayInfo(PBFSF pbfsf, TV_DISPINFO *lpnm)
{
    TV_ITEM ti = {0};
    LPITEMIDLIST pidlItem = (LPITEMIDLIST)lpnm->item.lParam;

    if ((lpnm->item.mask & (TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_CHILDREN)) == 0)
        return; // nothing for us to do here.

    pidlItem = _BFSFUpdateISHCache(pbfsf, lpnm->item.hItem, pidlItem);

    if (NULL == pidlItem)
    {
        ASSERT(pidlItem);
        return;
    } 

    ti.mask = 0;
    ti.hItem = (HTREEITEM)lpnm->item.hItem;

    // They are asking for IconIndex.  See if we can find it now.
    // Once found update their list, such that they wont call us back for
    // it again.
    if (lpnm->item.mask & (TVIF_IMAGE | TVIF_SELECTEDIMAGE))
    {
        // We now need to map the item into the right image index.
        SHFILEINFO sfi = {0};
        SHGetFileInfo(L".", FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi),
                      SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX);
        ti.iImage = lpnm->item.iImage = ti.iSelectedImage = sfi.iIcon;

        // we should save it back away to
        lpnm->item.iSelectedImage = ti.iSelectedImage;
        ti.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    }
    // Also see if this guy has any child folders
    if (lpnm->item.mask & TVIF_CHILDREN)
    {
        LPITEMIDLIST pidlFQ = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree,
                                                         lpnm->item.hItem);
        if (pidlFQ)
        {
            ULONG ulAttrs = SFGAO_HASSUBFOLDER;
            IShellFolder * pFolder = NULL;
    
            if (SUCCEEDED(SHBindToParent(pidlFQ, IID_IShellFolder,
                                         (VOID**) &pFolder, NULL)))
            {
                if (SUCCEEDED(pFolder->GetAttributesOf(1, (LPCITEMIDLIST*) &pidlItem,
                                                       &ulAttrs)))
                {
                    if (ulAttrs & SFGAO_HASSUBFOLDER)
                        ti.cChildren = lpnm->item.cChildren = 1;
                    else
                        ti.cChildren = lpnm->item.cChildren = 0;

                    ti.mask |= TVIF_CHILDREN;
                }
                pFolder->Release();
            }
            ILFree(pidlFQ);
        }
    }

    if (lpnm->item.mask & TVIF_TEXT)
    {
        STRRET str;
        if (SUCCEEDED(pbfsf->psfParent->GetDisplayNameOf(pidlItem, SHGDN_INFOLDER, &str)) &&
            SUCCEEDED(StrRetToBuf(&str, NULL, lpnm->item.pszText, lpnm->item.cchTextMax)))
        {
            ti.mask |= TVIF_TEXT;
            ti.pszText = lpnm->item.pszText;
        }
        else
        {
            ASSERTMSG(_T("No name found for tree item"),0);
            // Oh well - display a blank name and hope for the best.
        }
    }

    // Update the item now
    TreeView_SetItem(pbfsf->hwndTree, &ti);
}




/*++
Routine Description:
    
    
Arguments:
    
    
Return Value:
    
    
--*/
LPITEMIDLIST _BFSFUpdateISHCache(PBFSF pbfsf, HTREEITEM hti, LPITEMIDLIST pidlItem)
{
    HTREEITEM htiParent;
    IShellFolder *psfDesktop = NULL;
    HRESULT hr = E_FAIL;
    LPITEMIDLIST pidlRet = NULL;

    if (pidlItem == NULL)
        goto leave;

    hr = SHGetDesktopFolder(&psfDesktop);
    if (FAILED(hr) || (NULL == psfDesktop))
    {
        ASSERT(SUCCEEDED(hr)  &&  psfDesktop);
        goto leave;
    }
    
    // Need to handle the root case here!
    htiParent = TreeView_GetParent(pbfsf->hwndTree, hti);
    if ((htiParent != pbfsf->htiCurParent) || (pbfsf->psfParent == NULL))
    {
        LPITEMIDLIST pidl;

        if (pbfsf->psfParent)
        {
            pbfsf->psfParent->Release();
            pbfsf->psfParent = NULL;
        }

        if (htiParent)
        {
            pidl = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree, htiParent);
        }
        else
        {
            //
            // If No Parent then the item here is one of our roots which
            // should be fully qualified.  So try to get the parent by
            // decomposing the ID.
            //
            LPITEMIDLIST pidlT = (LPITEMIDLIST)ILFindLast(pidlItem);
            if (pidlT != pidlItem)
            {
                pidl = ILCopy(pidlItem, IL_ALL);
                ILRemoveLast(pidl);
                pidlItem = pidlT;
            }
            else
            {
                pidl = NULL;
            } 
        }

        pbfsf->htiCurParent = htiParent;

        SHBindToObject(psfDesktop, IID_IShellFolder, pidl, reinterpret_cast<PVOID *>(&pbfsf->psfParent));

        ILFree(pidl);
        if (pbfsf->psfParent == NULL)
            goto leave;
    }
    pidlRet = ILFindLast(pidlItem);

leave:
    if (NULL != psfDesktop)
    {
        psfDesktop->Release();
    }

    return pidlRet;
}




/*++
Routine Description:
    Process WM_COMMAND messages
    
Arguments:
    pbsf        a pointer to our BFSF structure
    id          ID of control to handle
    hwndCtl     hwnd of the control
    codeNotify  notification code
    
Return Value:
    none
    
--*/
void _BrowseForFolderOnBFSFCommand(PBFSF pbfsf, int id, HWND hwndCtl, UINT codeNotify)
{
    HTREEITEM hti;

    switch (id)
    {
    case IDC_BROWSEEDIT:
        if (codeNotify == EN_CHANGE)
        {
            TCHAR szBuf[4];     // (arb. size, anything > 2)

            szBuf[0] = 1;       // if Get fails ('impossible'), enable OK
            GetDlgItemText(pbfsf->hwndDlg, IDC_BROWSEEDIT, szBuf, lengthof(szBuf));
            DlgEnableOk(pbfsf->hwndDlg, (WPARAM)(BOOL)szBuf[0]);
        }
        break;

    case IDOK:
    {
        TV_ITEM tvi;
        TCHAR szText[MAX_PATH];
        BOOL fDone = TRUE;

        // We can now update the structure with the idlist of the item selected
        hti = TreeView_GetSelection(pbfsf->hwndTree);
        pbfsf->pidlCurrent = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree, hti);

        tvi.mask = TVIF_TEXT | TVIF_IMAGE;
        tvi.hItem = hti;
        tvi.pszText = pbfsf->pszDisplayName;
        if (!tvi.pszText)
            tvi.pszText = szText;
        tvi.cchTextMax = MAX_PATH;
        TreeView_GetItem(pbfsf->hwndTree, &tvi);
        
        if (pbfsf->ulFlags & BIF_EDITBOX) 
        {
            TCHAR szEditText[MAX_PATH];

            GetWindowText(pbfsf->hwndEdit, szEditText, lengthof(szEditText));

            if (CSTR_EQUAL != CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                            szEditText, -1, tvi.pszText, -1))
            {
                // the two are different, we need to get the user typed one
                IShellFolder * psfDesktopFolder = NULL;
                HRESULT hr = SHGetDesktopFolder(&psfDesktopFolder);
                if (SUCCEEDED(hr))
                {
                    LPITEMIDLIST pidl = NULL;
                    hr = psfDesktopFolder->ParseDisplayName(pbfsf->hwndOwner,
                                                            NULL, szEditText,
                                                            NULL, &pidl, NULL);
                    if (SUCCEEDED(hr))
                    {
                        ILFree(pbfsf->pidlCurrent);
                        pbfsf->pidlCurrent = pidl;
                        lstrcpy(tvi.pszText, szEditText);
                        tvi.iImage = -1;
                    }

                   psfDesktopFolder->Release();
                }

                if (FAILED(hr) && (pbfsf->ulFlags & BIF_VALIDATE))
                {
                    LPARAM lParam;

                    ASSERT(pbfsf->lpfn != NULL);

                    ILFree(pbfsf->pidlCurrent);
                    pbfsf->pidlCurrent = NULL;
                    tvi.pszText[0] = 0;
                    tvi.iImage = -1;
                    lParam = (LPARAM)szEditText;

                    // 0:EndDialog, 1:continue
                    fDone = _BFSFCallback(pbfsf, BFFM_VALIDATEFAILED, lParam) == 0;
                }

                // legacy behavior: hand back last-clicked pidl (even
                // though it doesn't match editbox text!)
            }
        }
        
        if (pbfsf->piImage)
            *pbfsf->piImage = tvi.iImage;
        if (fDone)
            EndDialog(pbfsf->hwndDlg, TRUE);        // To return TRUE.
        break;
    }
    case IDCANCEL:
        EndDialog(pbfsf->hwndDlg, 0);     // to return FALSE from this.
        break;
    }
}


/*
    ------------------------------------------
    Some helper functions for processing the dialog
    ------------------------------------------
*/





/*++
Routine Description:
    Adds Item to treeview control
    
Arguments:
    hwndTree        handle to treeview control
    htiParent       handle to treeitem parent
    pidl            pidl of item
    cChildren       number of children
Return Value:
    whatever SendMessage(...TVM_INSERTITEM...) returns
    
--*/
HTREEITEM _BFSFAddItemToTree(HWND hwndTree, HTREEITEM htiParent, LPITEMIDLIST pidl, int cChildren)
{
    TV_INSERTSTRUCT tii;

    // Initialize item to add with callback for everything
    tii.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE |
            TVIF_PARAM | TVIF_CHILDREN;
    tii.hParent = htiParent;
    tii.hInsertAfter = TVI_FIRST;
    tii.item.iImage = I_IMAGECALLBACK;
    tii.item.iSelectedImage = I_IMAGECALLBACK;
    tii.item.pszText = LPSTR_TEXTCALLBACK;   //
    tii.item.cChildren = cChildren; //  Assume it has children
    tii.item.lParam = (LPARAM)pidl;
    return TreeView_InsertItem(hwndTree, &tii);
}


/*++
Routine Description:
    Sorts the treeview
    
Arguments:
    pbfsf   our bfsf structure
    hti     handle to tree item
    psf     pointer to ISHellFolderInterface
    
Return Value:
    none
    
--*/
void _BFSFSort(PBFSF pbfsf, HTREEITEM hti, IShellFolder *psf)
{
    TV_SORTCB sSortCB;
    sSortCB.hParent = hti;
    sSortCB.lpfnCompare = _BFSFTreeCompare;

    psf->AddRef();
    sSortCB.lParam = (LPARAM)psf;
    TreeView_SortChildrenCB(pbfsf->hwndTree, &sSortCB, FALSE);
    psf->Release();
}



/*++
Routine Description:
    Expand the treeview items - it only *sounds* easy...

    NOTE - there is code here to handle items that we may never want to deal
          on CE - only time will tell
    
Arguments:
    pbfsf       our BFSF struct
    lpnmtv      pointer to a NM_TREEEVIEW struct

Return Value:
    TRUE if successful    
    
--*/
BOOL _BFSFHandleItemExpanding(PBFSF pbfsf, LPNM_TREEVIEW lpnmtv)
{
    LPITEMIDLIST pidlToExpand;
    LPITEMIDLIST pidl;
    IShellFolder *psf;
    IShellFolder *psfDesktop;
    DWORD grfFlags;
    // BOOL fPrinterTest = FALSE;
    int cAdded = 0;
    TV_ITEM tvi;
    ULONG celt;

    if (lpnmtv->action != TVE_EXPAND)
    {
        return FALSE;
    }
    if ((lpnmtv->itemNew.state & TVIS_EXPANDEDONCE))
    {
        return FALSE;
    }

    // set this bit now because we might be reentered
    tvi.mask = TVIF_STATE;
    tvi.hItem = lpnmtv->itemNew.hItem;
    tvi.state = TVIS_EXPANDEDONCE;
    tvi.stateMask = TVIS_EXPANDEDONCE;
    TreeView_SetItem(pbfsf->hwndTree, &tvi);


    if (lpnmtv->itemNew.hItem == NULL)
    {
        lpnmtv->itemNew.hItem = TreeView_GetSelection(pbfsf->hwndTree);
        if (lpnmtv->itemNew.hItem == NULL)
            return FALSE;
    }

    pidlToExpand = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree, lpnmtv->itemNew.hItem);

    HRESULT hr = SHGetDesktopFolder(&psfDesktop);
    ASSERT(SUCCEEDED(hr) && psfDesktop);
    if (FAILED(hr) || (NULL == psfDesktop))
    {
        return FALSE;
    }

    if (FAILED(SHBindToObject(psfDesktop, IID_IShellFolder, pidlToExpand, (void**)&psf)))
    {
        if (pidlToExpand)
            ILFree(pidlToExpand);
        psfDesktop->Release();
        return FALSE; // Could not get IShellFolder.
    }

    grfFlags = SHCONTF_FOLDERS;

    if (pbfsf->fShowAllObjects)
    {
        grfFlags |= SHCONTF_INCLUDEHIDDEN;
    } 

    IEnumIDList *penum;
    if (FAILED(psf->EnumObjects(pbfsf->hwndDlg, grfFlags, &penum)))
    {
        if (pidlToExpand)
            ILFree(pidlToExpand);
        psfDesktop->Release();
        psf->Release();

        tvi.mask = TVIF_CHILDREN | TVIF_HANDLE;   // only change the number of children
        tvi.hItem = lpnmtv->itemNew.hItem;
        tvi.cChildren = 0;

        TreeView_SetItem(pbfsf->hwndTree, &tvi);
        return FALSE;
    }

    while ((NOERROR == penum->Next(1, &pidl, &celt)) && (1 == celt))
    {
        int cChildren = I_CHILDRENCALLBACK;  // Do call back for children
        _BFSFAddItemToTree(pbfsf->hwndTree, lpnmtv->itemNew.hItem,
                pidl, cChildren);
        cAdded++;
    }

    // Now Cleanup after ourself
    penum->Release();

    _BFSFSort(pbfsf, lpnmtv->itemNew.hItem, psf);
    if (pidlToExpand)
        ILFree(pidlToExpand);
    psfDesktop->Release();
    psf->Release();

    // If we did not add anything we should update this item to let
    // the user know something happened.
    //
    if (cAdded == 0)
    {
        tvi.mask = TVIF_CHILDREN | TVIF_HANDLE;   // only change the number of children
        tvi.hItem = lpnmtv->itemNew.hItem;
        tvi.cChildren = 0;

        TreeView_SetItem(pbfsf->hwndTree, &tvi);
        return FALSE;
    }

    return TRUE;
}

int CALLBACK _BFSFTreeCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    IShellFolder *psfParent = (IShellFolder *)lParamSort;
    HRESULT hres = psfParent->CompareIDs(0, (LPITEMIDLIST)lParam1, (LPITEMIDLIST)lParam2);
    if (FAILED(hres))
        return 0;
    
    return (short)SCODE_CODE(GetScode(hres));
}

void _BFSFHandleDeleteItem(PBFSF pbfsf, LPNM_TREEVIEW lpnmtv)
{
    // We need to free the IDLists that we allocated previously
    if (lpnmtv->itemOld.lParam != 0)
        ILFree((LPITEMIDLIST)lpnmtv->itemOld.lParam);
}


/*++
Routine Description:
    Sets initial selection in dialog
    
Arguments:
    pbfsf           - pointer to our main struct
    blParamIsPath   - if this is TRUE, lParam is a path - otherwise, it's a PIDL
    lParam          - path or PIDL, depending on blParamIsPath
    
Return Value:
    TRUE if the selection was successful.
    
--*/
BOOL _BFSFSetSelection(PBFSF pbfsf, BOOL blParamIsPath, LPARAM lParam)
{
    BOOL fRet = FALSE;

    if (blParamIsPath) 
    {
        IShellFolder * psfDesktopFolder = NULL;

        if (SUCCEEDED(SHGetDesktopFolder(&psfDesktopFolder)))
        {
            LPITEMIDLIST pidl = NULL;

            if (SUCCEEDED(psfDesktopFolder->ParseDisplayName(pbfsf->hwndOwner,
                                                             NULL, (LPWSTR) lParam,
                                                             NULL, &pidl, NULL)))
            {
                HTREEITEM hti = _BFSFFindItem(pbfsf->hwndTree, pidl);

                if (hti)
                {
                    fRet = TreeView_SelectItem(pbfsf->hwndTree, hti);
                }

                ILFree(pidl);
             }

           psfDesktopFolder->Release();
        }
    }
    else
    {
        fRet = _BFSFSelectPidl(pbfsf, (LPITEMIDLIST)lParam);
    }

    return fRet;
}

/*++

    
Routine Description:
    "Walks treeview control to find item.  This is a recursive function
    
Arguments:
    hWnd        - window handle to treeview control
    pidl        - ITEMIDLIST describing the item                         
    
Return Value:
    HTREEITEM of the contol with the matching PIDL - NULL if on match is found.
    
--*/
HTREEITEM _BFSFFindItem(HWND hWnd, LPITEMIDLIST pidl)
{
    HTREEITEM hItem = TreeView_GetRoot(hWnd);
    LPITEMIDLIST pidlItem = NULL;

    while (hItem != NULL)
    {
        pidlItem = _BFSFGetIDListFromTreeItem(hWnd, hItem);
        if (pidlItem)
        {
            if (ILIsEqualParsingNames(pidl, ILFindLast(pidlItem)))
            {
                pidl = ILNext(pidl);

                if (ILIsEmpty(pidl))
                {
                    // We got it
                    break;
                }
                else
                {
                    // Correct path, keep going
                    TreeView_Expand(hWnd, hItem, TVE_EXPAND);
                    hItem = TreeView_GetChild(hWnd, hItem);
                }
            }
            else
            {
                // Go to next sibling item.
                hItem = TreeView_GetNextSibling(hWnd, hItem);
            }

            ILFree(pidlItem);
        }
        else
        {
            hItem = NULL;
        }
    }

    return hItem;
}

int _BFSFCallback(PBFSF pbfsf, UINT uMsg, LPARAM lParam)
{
    return pbfsf->lpfn ? pbfsf->lpfn(pbfsf->hwndDlg, uMsg, lParam, pbfsf->lParam) : 0;
}

BOOL _BFSFSelectPidl(PBFSF pbfsf, LPCITEMIDLIST pidl)
{
    HTREEITEM htiParent;
    LPITEMIDLIST pidlTemp;
    LPITEMIDLIST pidlNext = NULL;
    LPITEMIDLIST pidlParent = NULL;
    BOOL fRet = FALSE;

    htiParent = TreeView_GetChild(pbfsf->hwndTree, NULL);
    if (htiParent) 
    {
        // step through each item of the pidl
        for (;;) 
        {
            TreeView_Expand(pbfsf->hwndTree, htiParent, TVE_EXPAND);
            pidlParent = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree, htiParent);
            if (!pidlParent)
                break;

            pidlNext = ILCopy(pidl, IL_ALL);
            if (!pidlNext)
                break;

            pidlTemp = ILFindChild(pidlParent, pidlNext);
            if (!pidlTemp)
                break;

            if (ILIsEmpty(pidlTemp)) 
            {
                // found it!
                TreeView_SelectItem(pbfsf->hwndTree, htiParent);
                fRet = TRUE;
                break;
            } 
            else 
            {
                // loop to find the next item
                HTREEITEM htiChild;

                pidlTemp = ILNext(pidlTemp); //GetNext?
                if (!pidlTemp)
                    break;
                else
                    pidlTemp->mkid.cb = 0;


                htiChild = TreeView_GetChild(pbfsf->hwndTree, htiParent);
                while (htiChild) 
                {
                    BOOL fEqual;
                    pidlTemp = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree, htiChild);
                    if (!pidlTemp) 
                    {
                        htiChild = NULL;
                        break;
                    }
                    fEqual = ILIsEqual(pidlTemp, pidlNext);

                    ILFree(pidlTemp);
                    if (fEqual) 
                    {
                        break;
                    }
                    else 
                    {
                        htiChild = TreeView_GetNextSibling(pbfsf->hwndTree, htiChild);
                    }
                }

                if (!htiChild) 
                {
                    // we didn't find the next one... bail
                    break;
                }
                else 
                {
                    // the found child becomes the next parent
                    htiParent = htiChild;
                    ILFree(pidlParent);
                    ILFree(pidlNext);
                }
            }
        }
    }

    if (pidlParent) ILFree(pidlParent);
    if (pidlNext) ILFree(pidlNext);
    return fRet;
}

/*++
Routine Description:
    Displays a dialog box that enables the user to select a shell folder. 
    This function is exported through ceshell.dll
    
Arguments:
    lpbi        pointer to BROWSEINFO structure

                we shove everything from lpbi into our bigger BFSF
                structure and ust that pointer liberally throughout
                this file
    
Return Value:
    A valid LPITEMIDLIST if the user presses OK, and everything works well    
    NULL if the user presses cancel, or something bad happens
    
--*/
extern "C" LPITEMIDLIST WINAPI SHBrowseForFolder(LPBROWSEINFO lpbi)
{   
    LPITEMIDLIST lpRet = NULL;

    if (NULL == lpbi)
    {
        ASSERTMSG(_T("Invalid (NULL) pointer passed to SHBrowseForFolder"),lpbi);
        return NULL;
    }

    BFSF bfsf = {
      lpbi->hwndOwner,
      lpbi->pidlRoot,
      lpbi->pszDisplayName,
      &lpbi->iImage,
      lpbi->lpszTitle,
      lpbi->ulFlags,
      lpbi->lpfn,
      lpbi->lParam,
    };
    HCURSOR hcOld = SetCursor(LoadCursor(NULL,IDC_WAIT));

    // Initialize the extra common controls
    if ( LoadAygshellLibrary() )
    {
        AygInitExtraControls();
    }

    // Now Create the dialog that will be doing the browsing.
    if (DialogBoxParam(HINST_CESHELL, MAKEINTRESOURCE(IDD_BROWSEFORFOLDER),
                       lpbi->hwndOwner, _BrowseForFolderBFSFDlgProc, (LPARAM)&bfsf))
    {                       
        lpRet = bfsf.pidlCurrent;
    }

    if (hcOld)
    {
        SetCursor(hcOld);
    }

    FreeAygshellLibrary();

    return lpRet;
}

