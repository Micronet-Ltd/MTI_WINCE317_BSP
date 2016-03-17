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
 *  module: api.cpp
\*---------------------------------------------------------------------------*/
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include "taskbar.hxx"
#include "explorerlist.hpp"

// in mainwnd.cpp
HRESULT HandleNewWindow2(LPWSTR lpszUrl, LPCITEMIDLIST pidl, DISPPARAMS FAR* pdparams);
extern CTaskBar *g_TaskBar;
// in taskbar/utils.cpp
HICON CopyIcon(HICON hicon);

/*
    @doc BOTH EXTERNAL PEGSHELL

    @func BOOL | SHCreateExplorerInstance | Creates an instance of an
    Explorer

    @parm LPCWSTR | pszPath | Pointer to a buffer that contains the starting
    path for the explorer instance.
    @parm UINT | uFlags | Reserved parameter, you must pass 0.

    @comm This API will open up a new window and thread each time that it is
    called.
*/
BOOL WINAPI SHCreateExplorerInstance(LPCWSTR pszPath, UINT uFlags)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    BOOL bRet = FALSE;

    __try
    {
        if (!pszPath)
        {
            goto leave;
        }

        // REVIEW: ignoring flags

        HWND hwndExplorer = ExplorerList_t::FindExplorerWnd(pszPath);
        if (hwndExplorer)
        {
            SetForegroundWindow(hwndExplorer);
            bRet = TRUE;
        }
        else
        {
            HRESULT hr = HandleNewWindow2( (LPWSTR) pszPath, NULL, NULL );
            bRet = (hr == S_OK);
        }

    leave:
        return bRet;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        ASSERT(FALSE);
        return FALSE;
    }
} /* SHCreateExplorerInstance()
   */

BOOL
WINAPI
Shell_NotifyIconI(DWORD dwMsg, PNOTIFYICONDATA pNID, DWORD cbNID)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    COPYDATASTRUCT cds = {0};
    BOOL bRet = FALSE;

    __try
    {
		if (!pNID ||
		   sizeof(NOTIFYICONDATA) != pNID->cbSize ||
		   !IsWindow(pNID->hWnd))
		{
		   SetLastError(ERROR_INVALID_PARAMETER);
		   goto leave;
		}

		// Set up the copy data struct
		cds.cbData = sizeof(NOTIFYICONDATA);
		cds.lpData = pNID;
		cds.dwData = dwMsg;
	
		HWND hWnd;
		if ( !g_TaskBar )
		{
			goto leave;
		}

		hWnd = g_TaskBar->GetWindow();
		bRet = SendMessage(hWnd, WM_COPYDATA, NULL, (LPARAM) &cds);

	leave:
		return bRet;
	}
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        ASSERT(FALSE);
        return FALSE;
    }

} /* Shell_NotifyIcon()
   */


/*
 * SHCloseApps, should close some apps to recover needed memory
 */
BOOL WINAPI SHCloseAppsI( DWORD dwMemSought )
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
} /* SHCloseApps()
   */

#if 0
#define MAX_RECENT_FILES 10
BOOL DeleteSomeRecentDocs()
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    // routine to keep the number of documents in the recent folder down.

    WIN32_FIND_DATA fd;
    HANDLE hfind;
    WCHAR szRoadKill[MAX_PATH];
    FILETIME ftOldest;
    CStringRes sz(_Module.m_hInst);

    if (NULL == sz.LoadString(IDS_WINDOWSRECENT))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE; //out of memory
    }
    int len = _tcslen(sz)+1; // +1 for the "\"

    // Note: it's not perfect but tries to get the number as close to
    //    MAX_RECENT_FILES as possible in one simple pass

    // use roadkill as a tmp
    _tcscpy(szRoadKill, sz);
    _tcscat(szRoadKill, TEXT("\\"));
    _tcscat(szRoadKill, c_szStarDotStar);

    hfind = FindFirstFile(szRoadKill, &fd);
    if (hfind == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }
    int i = 0;
    ftOldest = fd.ftLastWriteTime;
    _tcscpy(&szRoadKill[len], fd.cFileName);

    do
    {
        i++;
        if (CompareFileTime(&ftOldest, &fd.ftLastWriteTime) > 0)
        {
            // get the older file
            ftOldest = fd.ftLastWriteTime;
            _tcscpy(&szRoadKill[len], fd.cFileName);
        }
        if ((i > MAX_RECENT_FILES) && szRoadKill[len])
        {
            DeleteFile(szRoadKill);
            szRoadKill[len] = 0;
        }
    } while (FindNextFile(hfind, &fd));

    FindClose(hfind);
    return TRUE;

}
#endif


