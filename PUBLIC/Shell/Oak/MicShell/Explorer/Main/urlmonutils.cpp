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
#include "urlmonutils.hpp"

#ifndef _PREFAST_
#pragma warning(disable:4068)
#endif // _PREFAST_


DWORD UrlmonUtils_t::m_UrlmonRefCount = 0;
HINSTANCE UrlmonUtils_t::m_hInstUrlmon = NULL;
CRITICAL_SECTION UrlmonUtils_t::m_csUrlmon = {0};
BOOL UrlmonUtils_t::m_fInitialized = FALSE;
BOOL UrlmonUtils_t::m_fLoadedUrlmonLibrary = FALSE;

HICON UrlmonUtils_t::m_hStatusIcons[MAX_STATUSICONS] = {0};
LPWSTR UrlmonUtils_t::m_pszStatusZones[MAX_STATUSZONES] = {0};

// pointers to Urlmon functions
HRESULT (WINAPI* UrlmonUtils_t::pfnCoInternetCreateSecurityManager)(IServiceProvider*, IInternetSecurityManager**, DWORD) = NULL;


BOOL UrlmonUtils_t::InitializeUrlmonUtils(HINSTANCE HInstExplorer)
{
    InitializeCriticalSection(&m_csUrlmon);
    m_fInitialized = TRUE;

    int i;
    for (i=0; i<MAX_STATUSICONS; i++)
    {   
        m_hStatusIcons[i] = (HICON)LoadImage(HInstExplorer,
                        MAKEINTRESOURCE(IDI_ZONE + i),
                        IMAGE_ICON,
                        16,
                        16,
                        0);
    }

    for (i=0; i<MAX_STATUSZONES; i++)
    {   
        m_pszStatusZones[i] = (LPWSTR)LoadString(HInstExplorer,
                        (IDS_ZONEBASE + i),
                        NULL,
                        0);
    }

    return TRUE;
}

BOOL UrlmonUtils_t::UnInitializeUrlmonUtils()
{
    DeleteCriticalSection(&m_csUrlmon);
    m_fInitialized = FALSE;

    int i;
    for (i=0; i<MAX_STATUSICONS; i++)
    {
        if (m_hStatusIcons[i])
        {
            DestroyIcon(m_hStatusIcons[i]);
        }
    }

    for (i=0; i<MAX_STATUSZONES; i++)
    {   
        m_pszStatusZones[i] = NULL;
    }

    return TRUE;
}

BOOL UrlmonUtils_t::LoadUrlmonLibrary()
{
    BOOL bRet = FALSE;
    if (!m_fInitialized)
    {
        return FALSE;
    }

    EnterCriticalSection(&m_csUrlmon);

    if (m_hInstUrlmon)
    {
        ASSERT(m_UrlmonRefCount > 0);
        m_UrlmonRefCount++;
        bRet = TRUE;
    }
    else
    {
        ASSERT(m_UrlmonRefCount == 0);
#pragma prefast(suppress:321, "Not loading dll via relative path")
        m_hInstUrlmon = LoadLibrary(L"Urlmon.dll");
        if (m_hInstUrlmon)
        {
            m_UrlmonRefCount++;
            m_fLoadedUrlmonLibrary = TRUE;
            bRet = TRUE;
        }
    }

    LeaveCriticalSection(&m_csUrlmon);

    return bRet;
}

BOOL UrlmonUtils_t::FreeUrlmonLibrary()
{
    if (!m_fInitialized || !m_fLoadedUrlmonLibrary)
    {
        return FALSE;
    }

    ASSERT(m_hInstUrlmon);
    ASSERT(m_UrlmonRefCount > 0);

    EnterCriticalSection(&m_csUrlmon);

    m_UrlmonRefCount--;

    if (m_UrlmonRefCount < 1)
    {
        FreeLibrary(m_hInstUrlmon);
        m_hInstUrlmon = NULL;
        m_fLoadedUrlmonLibrary = FALSE;

        // Clean Global Pointers
        pfnCoInternetCreateSecurityManager = NULL;
    }

    LeaveCriticalSection( &m_csUrlmon );

    return TRUE;
}

HRESULT
UrlmonUtils_t::
CoInternetCreateSecurityManager(
    IServiceProvider *pSP,
    IInternetSecurityManager **ppSM,
    DWORD dwReserved
    )
{
    HRESULT hr = E_NOTIMPL;

    if (!m_fInitialized || !m_fLoadedUrlmonLibrary)
    {
        goto leave;
    }

    if (!pfnCoInternetCreateSecurityManager)
    {
        pfnCoInternetCreateSecurityManager = (HRESULT (*)(IServiceProvider*, IInternetSecurityManager**, DWORD))
                        GetProcAddress(m_hInstUrlmon, L"CoInternetCreateSecurityManager");
    }

    if (pfnCoInternetCreateSecurityManager)
    {
        hr = pfnCoInternetCreateSecurityManager(pSP, ppSM, dwReserved);
    }

leave:
    return hr;
}

HICON
UrlmonUtils_t::
GetStatusIcon(
    DWORD dwIndex
    )
{
    HICON hIconRet = NULL;
    
    if (dwIndex < 0 ||dwIndex >= MAX_STATUSICONS)
    {
        goto leave;
    }

    hIconRet = m_hStatusIcons[dwIndex];

leave:
    return hIconRet;
}

LPCWSTR
UrlmonUtils_t::
GetStatusZone(
    DWORD dwIndex
    )
{
    LPCWSTR pszRet = NULL;
    
    if (dwIndex < 0 ||dwIndex >= MAX_STATUSZONES)
    {
        goto leave;
    }

    pszRet = m_pszStatusZones[dwIndex];

leave:
    return pszRet;
}

