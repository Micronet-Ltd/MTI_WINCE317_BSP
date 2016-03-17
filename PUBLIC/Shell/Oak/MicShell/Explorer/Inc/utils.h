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

#ifndef _H_UTILS_
#define _H_UTILS_

#include <shlobj.h>
#include <aygshell.h>

#define MAX(a,b)  (a > b) ? a : b
#define MIN(a,b)  (a > b) ? b : a
#define ARRAYSIZE(a)    (sizeof(a)/sizeof((a)[0]))
#define InRange(id, idFirst, idLast)      ((UINT)((id)-(idFirst)) <= (UINT)((idLast)-(idFirst)))
#define IsInRange                   InRange
#define SAFE_FREE_LIBRARY( x ) if ( x ) { FreeLibrary( x ); }

extern "C" void RegisterShellAPIs();
extern "C" void ShellNotifyCallback(DWORD cause, DWORD proc, DWORD thread);
extern "C" void UnRegisterShellAPIs();

STDAPI_(BOOL) GUIDFromString(LPCTSTR psz, GUID *pguid);
BOOL HexStringToDword(LPCTSTR * ppsz, DWORD * lpValue, int cDigits, TCHAR chDelim);
STDAPI InitVariantFromIDList(VARIANT* pvar, LPCITEMIDLIST pidl);
STDAPI InitVariantFromBuffer(VARIANT *pvar, const void *pv, UINT cb);
UINT ILGetSize(LPCITEMIDLIST pidl);

int GetMenuItemCount(HMENU hMenu);
UINT GetMenuItemID(HMENU hMenu, int nPos);

LPWSTR ToWide(LPCWSTR);
LPWSTR ToWide(LPCSTR);
LPSTR ToAnsi(LPCWSTR);
LPSTR ToAnsi(LPCSTR);

BOOL NeedsEllipses(HDC hdc, LPCTSTR pszText, LPRECT prc, int FAR* pcchDraw, int cxEllipses);

void SetSchemeColors();

BOOL GetParsingName(LPCWSTR pszDisplayName, LPWSTR pszParsingName, PUINT pcchOutBuf);
BOOL SaveDisplayName(LPCWSTR pszParsingName, LPCWSTR pszDisplayName, LPWSTR* ppszPrevDisplayName);

HICON CopyIcon(HICON hicon); // defined in utils.cpp
BOOL BrowseInPlace();
BOOL PathCanBrowseInPlace (LPCTSTR pszPath);
#endif
