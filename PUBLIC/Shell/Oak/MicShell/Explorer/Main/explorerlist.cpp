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
#include <winbase.h>
#include <shobjidl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include "explorerlist.hpp"


CList* ExplorerList_t::m_pList = NULL;

BOOL ExplorerList_t::InitializeExplorerList()
{
    m_pList = new CList();
    return (BOOL)m_pList;
}

BOOL ExplorerList_t::UnInitializeExplorerList()
{   
    if (m_pList)
    {
        delete m_pList;
        m_pList = NULL;
    }

    return TRUE;
}

BOOL ExplorerList_t::AddExplorerWnd(HWND hwnd)
{
    BOOL bRet = FALSE;
    LPEXPLORERLISTOBJ lpeobj;

    if (!m_pList || !hwnd || !IsWindow(hwnd))
    {
        goto leave;
    }

    m_pList->Lock(TRUE);

    lpeobj = FindExplorerListObject(hwnd);
    if (!lpeobj)
    {
        lpeobj = (LPEXPLORERLISTOBJ)LocalAlloc(LPTR, sizeof(EXPLORERLISTOBJ));
        if (lpeobj)
        {
            bRet = TRUE;
            lpeobj->explorerInfo.hwndExplorer = hwnd;
            m_pList->AddHead(&lpeobj->listObject);
        }
    }

    m_pList->Lock(FALSE);

leave:
    return bRet;
}

LPEXPLORERLISTOBJ ExplorerList_t::FindExplorerListObject(HWND hwnd)
{
    LPEXPLORERLISTOBJ lpeobj;

    lpeobj = (LPEXPLORERLISTOBJ)m_pList->Head();
    while (lpeobj)
    {
        if (lpeobj->explorerInfo.hwndExplorer == hwnd)
        {
            break;
        }
        lpeobj = (LPEXPLORERLISTOBJ)m_pList->Next(&lpeobj->listObject);
    }

    return lpeobj;
}

HWND ExplorerList_t::FindExplorerWnd(LPCWSTR pszPath)
{
    HWND hwndRet = NULL;
    IMalloc *pIMalloc = NULL;
    IShellFolder *psfDesktop = NULL;
    IShellFolder *psfParent = NULL;
    LPITEMIDLIST pidl = NULL;
    LPITEMIDLIST pidlRelative = NULL;
    LPEXPLORERLISTOBJ lpeobj;
    WCHAR szPath[MAX_PATH];
    STRRET str = {0};

    if (!m_pList || !pszPath)
    {
        goto leave;
    }

    if (FAILED(SHGetMalloc(&pIMalloc)))
    {
        goto leave;
    }

    if (FAILED(SHGetDesktopFolder(&psfDesktop)) ||
        FAILED(psfDesktop->ParseDisplayName(NULL,
                        NULL,
                        (LPOLESTR)pszPath,
                        NULL,
                        &pidl,
                        NULL)) ||
        FAILED(SHBindToParent(pidl,
                        IID_IShellFolder,
                        (void**)&psfParent,
                        (LPCITEMIDLIST*)&pidlRelative)) ||
        FAILED(psfParent->GetDisplayNameOf(pidlRelative,
                        SHGDN_NORMAL | SHGDN_FORADDRESSBAR,
                        &str)) ||
        FAILED(StrRetToBuf(&str,
                        pidl,
                        szPath,
                        sizeof(szPath)/sizeof(*szPath))))
    {
        goto cleanup;
    }

    m_pList->Lock(TRUE);

    lpeobj = (LPEXPLORERLISTOBJ)m_pList->Head();
    while (lpeobj)
    {
        if (lpeobj->explorerInfo.pszPath &&
            !_wcsicmp(szPath, lpeobj->explorerInfo.pszPath))
        {
            hwndRet = lpeobj->explorerInfo.hwndExplorer;
            break;
        }

        lpeobj = (LPEXPLORERLISTOBJ)m_pList->Next(&lpeobj->listObject);
    }

    m_pList->Lock(FALSE);

cleanup:
    if (psfDesktop)
    {
        psfDesktop->Release();
    }

    if (psfParent)
    {
        psfParent->Release();
    }

    if (pidl)
    {
        pIMalloc->Free(pidl);
    }

    if (pidlRelative)
    {
        pIMalloc->Free(pidlRelative);
    }

    pIMalloc->Release();

leave:
    return hwndRet;
}

BOOL ExplorerList_t::RemoveExplorerWnd(HWND hwnd)
{
    BOOL bRet = FALSE;
    LPEXPLORERLISTOBJ lpeobj;

    if (!m_pList || !hwnd)
    {
        goto leave;
    }

    m_pList->Lock(TRUE);

    lpeobj = FindExplorerListObject(hwnd);
    if (lpeobj)
    {
        bRet = TRUE;
        m_pList->Disconnect(&lpeobj->listObject);

        if (lpeobj->explorerInfo.pszPath)
        {
            LocalFree(lpeobj->explorerInfo.pszPath);
        }
        LocalFree(lpeobj);
    }

    m_pList->Lock(FALSE);

leave:
    return bRet;
}

BOOL ExplorerList_t::UpdateExplorerWndInfo(HWND hwnd, LPCWSTR pszPath)
{
    BOOL bRet = FALSE;
    LPEXPLORERLISTOBJ lpeobj;

    if (!m_pList || !hwnd)
    {
        goto leave;
    }

    m_pList->Lock(TRUE);

    lpeobj = FindExplorerListObject(hwnd);
    if (lpeobj)
    {
        if (!pszPath)
        {
            if (lpeobj->explorerInfo.pszPath)
            {
                LocalFree(lpeobj->explorerInfo.pszPath);
                lpeobj->explorerInfo.pszPath = NULL;
            }
            bRet = TRUE;
        }
        else
        {
            LPWSTR pszNewPath;
            size_t cbLength = (wcslen(pszPath) + 1) * sizeof(WCHAR);

            if (lpeobj->explorerInfo.pszPath)
            {
                pszNewPath = (LPWSTR)LocalReAlloc(lpeobj->explorerInfo.pszPath,
                                cbLength,
                                LMEM_MOVEABLE);
            }
            else
            {
                pszNewPath = (LPWSTR)LocalAlloc(LMEM_FIXED, cbLength);
            }

            if (pszNewPath)
            {
                bRet = TRUE;
                wcscpy(pszNewPath, pszPath);
                lpeobj->explorerInfo.pszPath = pszNewPath;
            }
        }
    }

    m_pList->Lock(FALSE);

leave:
    return bRet;
}

