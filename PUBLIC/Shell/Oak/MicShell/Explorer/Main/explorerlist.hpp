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

#ifndef __LIST_H__
#include "list.hpp"
#endif


typedef struct tagEXPWNDINFO {
    HWND hwndExplorer;
    LPWSTR pszPath;
} EXPWNDINFO, *LPEXPWNDINFO;


typedef struct tagEXPLORERLISTOBJ {
    LISTOBJ listObject;
    EXPWNDINFO explorerInfo;
} EXPLORERLISTOBJ, *LPEXPLORERLISTOBJ;


class ExplorerList_t
{
protected:
    static CList* m_pList;

    static LPEXPLORERLISTOBJ FindExplorerListObject(HWND hwnd);

public:
    static BOOL InitializeExplorerList();
    static BOOL UnInitializeExplorerList();

    static BOOL AddExplorerWnd(HWND hwnd);
    static HWND FindExplorerWnd(LPCWSTR pszPath);
    static BOOL RemoveExplorerWnd(HWND hwnd);
    static BOOL UpdateExplorerWndInfo(HWND hwnd, LPCWSTR pszPath);
};


