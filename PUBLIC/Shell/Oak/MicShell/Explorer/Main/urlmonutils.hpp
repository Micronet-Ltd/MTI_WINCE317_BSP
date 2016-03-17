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
#include "urlmon.h"
#include "resource.h"

#define MAX_STATUSICONS 6
#define MAX_STATUSZONES 5

class UrlmonUtils_t
{
protected:
    static DWORD m_UrlmonRefCount;
    static HINSTANCE m_hInstUrlmon;
    static CRITICAL_SECTION m_csUrlmon;
    static BOOL m_fInitialized;
    static BOOL m_fLoadedUrlmonLibrary;

    // Icon-Zones cache
    static HICON m_hStatusIcons[MAX_STATUSICONS];
    static LPWSTR m_pszStatusZones[MAX_STATUSZONES];

    static HRESULT (WINAPI* pfnCoInternetCreateSecurityManager)(IServiceProvider*, IInternetSecurityManager**, DWORD);

public:  
    static BOOL InitializeUrlmonUtils(HINSTANCE hInstExplorer);
    static BOOL UnInitializeUrlmonUtils();

    static BOOL LoadUrlmonLibrary();
    static BOOL FreeUrlmonLibrary();

    // Urlmon thunks
    static HRESULT CoInternetCreateSecurityManager(IServiceProvider *pSP, IInternetSecurityManager **ppSM, DWORD dwReserved);

    // Helper functions to support Zones
    static HICON GetStatusIcon(DWORD dwIndex);
    static LPCWSTR GetStatusZone(DWORD dwIndex);

};



