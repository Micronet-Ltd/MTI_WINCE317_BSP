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
#include "utils.h"

HICON CopyIcon(HICON hicon)
{
    static HIMAGELIST s_himl = NULL;
    HICON             hiconNew = NULL;
    int               iIcon = -1;

    if ( !hicon )
    {
        return NULL;
    }

    if (NULL == s_himl)
    {
        s_himl = ImageList_Create(16, 16, ILC_MASK, 1, 0);
        if (NULL != s_himl)
        {
            iIcon = ImageList_AddIcon(s_himl, hicon);
        }
    }
    else
    {
        iIcon = ImageList_ReplaceIcon(s_himl, 0, hicon);
    }

    if (iIcon >= 0)
    {
        hiconNew = ImageList_GetIcon(s_himl, iIcon, ILD_TRANSPARENT);
    }

    return hiconNew;

}

#define COLOR_MAX C_SYS_COLOR_TYPES
#define SCHEME_VERSION_WINCE -1

void SetSchemeColors()
{
    HKEY hKey;
    WCHAR pszScheme[MAX_PATH];
    DWORD  dwLen = (MAX_PATH-1)*sizeof(WCHAR);
    typedef struct 
    {
        SHORT nVersion;
        // for alignment
        WORD  wDummy;
        COLORREF argb[COLOR_MAX];
    } SCHEMEDATA, *PSCHEMEDATA;

    SCHEMEDATA sd;
    int n, cColors;
    DWORD dwSize;
    COLORREF argb[COLOR_MAX];
    int nColors[COLOR_MAX];


    // get the color scheme
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("ControlPanel\\Appearance"), 0, 0, &hKey))
    {
        goto exit;
    }

    // get the current scheme name
    if (ERROR_SUCCESS != RegQueryValueEx(hKey, TEXT("Current"), NULL, NULL, (LPBYTE) pszScheme, &dwLen ))
    {
        goto exit;
    }

    RegCloseKey(hKey);
    
    // load the current scheme colors
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("ControlPanel\\Appearance\\Schemes"), 0, 0, &hKey))
    {
        goto exit;
    }

    dwSize = sizeof(sd);
    if (ERROR_SUCCESS != RegQueryValueEx(hKey, (LPWSTR) pszScheme, NULL, NULL, (LPBYTE)&sd, &dwSize))
    {
        goto exit;
    }

    if (!dwSize || (sd.nVersion != SCHEME_VERSION_WINCE))
    {
        goto exit;
    }

    // If scheme in the registry has less colors
    if (dwSize < sizeof(sd))
    {
        cColors = COLOR_MAX - (sizeof(sd)-dwSize)/sizeof(COLORREF);
    }
    else cColors = COLOR_MAX;

    // Set colors from the registry
    for (n = 0; n < cColors; n++)
    {
        argb[n] = sd.argb[n];
    }

    // If scheme in the registry has less colors, use
    // current system colors for the missing colors.
    if (cColors < COLOR_MAX)
    {
        for (n = cColors; n < COLOR_MAX; n++)
        {
            argb[n] = GetSysColor(n | SYS_COLOR_INDEX_FLAG);
        }
    }

    // Set system colors using current scheme
    for (n=0; n<COLOR_MAX; n++)
        nColors[n] = n | SYS_COLOR_INDEX_FLAG;
    SetSysColors(COLOR_MAX, nColors, argb);

exit:
    RegCloseKey(hKey);
}


#define VALIDATE_PIDL(pidl) // add debugging code as needed
#define _ILSkip(pidl, cb)       ((LPITEMIDLIST)(((BYTE*)(pidl))+cb))
#define _ILNext(pidl)           _ILSkip(pidl, (pidl)->mkid.cb)

UINT ILGetSize(LPCITEMIDLIST pidl)
{
    UINT cbTotal = 0;
    if (pidl)
    {
        VALIDATE_PIDL(pidl);
        cbTotal += sizeof(pidl->mkid.cb);       // Null terminator
        while (pidl->mkid.cb)
        {
            cbTotal += pidl->mkid.cb;
            pidl = _ILNext(pidl);
        }
    }
    return cbTotal;
}

STDAPI InitVariantFromBuffer(VARIANT *pvar, const void *pv, UINT cb)
{
    HRESULT hr;
    SAFEARRAY *psa = SafeArrayCreateVector(VT_UI1, 0, cb);   // create a one-dimensional safe array
    if (psa) 
    {
        memcpy(psa->pvData, pv, cb);

        memset(pvar, 0, sizeof(*pvar));  // VariantInit()
        pvar->vt = VT_ARRAY | VT_UI1;
        pvar->parray = psa;
        hr = S_OK;
    }
    else
        hr = E_OUTOFMEMORY;
    return hr;
}


STDAPI InitVariantFromIDList(VARIANT* pvar, LPCITEMIDLIST pidl)
{
    return InitVariantFromBuffer(pvar, pidl, ILGetSize(pidl));
}


// scan psz for a number of hex digits (at most 8); update psz, return
// value in Value; check for chDelim; return TRUE for success.
BOOL HexStringToDword(LPCTSTR * ppsz, DWORD * lpValue, int cDigits, TCHAR chDelim)
{
    int ich;
    LPCTSTR psz = *ppsz;
    DWORD Value = 0;
    BOOL bRet = TRUE;

    for (ich = 0; ich < cDigits; ich++)
    {
        TCHAR ch = psz[ich];
        if (InRange(ch, TEXT('0'), TEXT('9')))
        {
            Value = (Value << 4) + ch - TEXT('0');
        }
        else if ( InRange( (ch |= (TEXT('a')-TEXT('A'))), TEXT('a'), TEXT('f')) )
        {
            Value = (Value << 4) + ch - TEXT('a') + 10;
        }
        else
        {
            bRet = FALSE;
            goto Leave;
        }
    }

    if (chDelim)
    {
        bRet = (psz[ich++] == chDelim);
    }

    *lpValue = Value;
    *ppsz = psz+ich;

Leave:
    return bRet;
}

// parse above format; return TRUE if succesful; always writes over *pguid.
STDAPI_(BOOL) GUIDFromString(LPCTSTR psz, GUID *pguid)
{
    DWORD dw;
    if (*psz++ != TEXT('{') /*}*/ )
        return FALSE;

    if (!HexStringToDword(&psz, &pguid->Data1, sizeof(DWORD)*2, TEXT('-')))
        return FALSE;

    if (!HexStringToDword(&psz, &dw, sizeof(WORD)*2, TEXT('-')))
        return FALSE;

    pguid->Data2 = (WORD)dw;

    if (!HexStringToDword(&psz, &dw, sizeof(WORD)*2, TEXT('-')))
        return FALSE;

    pguid->Data3 = (WORD)dw;

    if (!HexStringToDword(&psz, &dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[0] = (BYTE)dw;

    if (!HexStringToDword(&psz, &dw, sizeof(BYTE)*2, TEXT('-')))
        return FALSE;

    pguid->Data4[1] = (BYTE)dw;

    if (!HexStringToDword(&psz, &dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[2] = (BYTE)dw;

    if (!HexStringToDword(&psz, &dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[3] = (BYTE)dw;

    if (!HexStringToDword(&psz, &dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[4] = (BYTE)dw;

    if (!HexStringToDword(&psz, &dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[5] = (BYTE)dw;

    if (!HexStringToDword(&psz, &dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[6] = (BYTE)dw;
    if (!HexStringToDword(&psz, &dw, sizeof(BYTE)*2, /*(*/ TEXT('}')))
        return FALSE;

    pguid->Data4[7] = (BYTE)dw;

    return TRUE;
}


// Helper functions to obtain NameSpaces from the registry given a DisplayName
static const WCHAR szDesktopCLSID[] = L"{00021400-0000-0000-C000-000000000046}";
static const WCHAR szGUIDs[] = L"\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\CurrentGUIDs";

BOOL FindNameInClassesRoot(LPCWSTR szNameSpace, LPCWSTR pszDisplayName, size_t cchToCompare)
{
    BOOL bRet = FALSE;
    WCHAR szValName[MAX_PATH];
    WCHAR szCLSID[256] = L"CLSID\\";
    DWORD dwType = 0;
    DWORD cbValName;
    HKEY hKey = NULL;

    if ( StringCchCat(szCLSID, 256, szNameSpace) != S_OK )
    {
        goto Leave;
    }

    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szCLSID, 0, 0, &hKey) == ERROR_SUCCESS)
    {
        cbValName = MAX_PATH * sizeof(WCHAR);
        if (RegQueryValueEx(hKey, TEXT("DisplayName"), NULL, &dwType, (LPBYTE)szValName, &cbValName) == ERROR_SUCCESS)
        {
            if (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT,
                                            NORM_IGNORECASE | NORM_IGNOREWIDTH,
                                            pszDisplayName, cchToCompare,
                                            szValName, -1))
            {
                bRet = TRUE;
            }
        }
        RegCloseKey(hKey);
    }

Leave:
    return bRet;
}


/*
    GetParsingName

    @parm LPCWSTR | pszDisplayName | Name for which to retrieve its ParsingName
    @parm LPWSTR | pszParsingName | (Out) buffer to store the ParsingName
    @parm PUINT | pcchOutBuf | (In/Out) pointer to buffer size in characters
    
    @rdesc If the function finds a match, the return value is TRUE.
        pcchOutBuf's value is either equal to the number of copied chars or the
        required buffer size (if pszParsingName parameter is not large enough).
        If the function fails, the return value is FALSE. 
*/
BOOL GetParsingName(LPCWSTR pszDisplayName, LPWSTR pszParsingName, PUINT pcchOutBuf)
{
    static const WCHAR pNameSpaceHolder[2][25]= {
        {L"Explorer\\Desktop"},
        {L"Explorer\\MyDevice"}
    };

    WCHAR szValName[256];
    LPWSTR pszValData= NULL;
    LPCWSTR pszEndName= NULL;
    size_t cchInput, cchToCompare , cchToCpy = *pcchOutBuf;
    DWORD dwType = 0;
    DWORD cbValName, cbData;
    DWORD dwIndex, i;
    HKEY hKey;
    LONG lRes = !ERROR_SUCCESS;

    if (!pszDisplayName || !pszParsingName || !pcchOutBuf)
    {
        goto Leave;
    }

    if (pszDisplayName[0] == L'\\' || wcschr(pszDisplayName, L':'))
    {
        goto Leave;
    }

    pszValData= new WCHAR [MAX_PATH];
    if (!pszValData)
    {
        goto Leave;
    }

    // Init to negative number
    cbValName = -1;

    // Strip the possible namespace. Tail will be concatenated to Parsing name in case of a match
    cchInput = cchToCompare = wcslen(pszDisplayName);
    pszEndName= wcschr(pszDisplayName, L'\\');
    if (pszEndName)
    {
        cchToCompare = (pszEndName - pszDisplayName);
    }

    // Look up in our cache's set first
    ASSERT(pszParsingName);

    hKey = NULL;
    lRes = RegOpenKeyEx(HKEY_CURRENT_USER, szGUIDs, 0, 0, &hKey);
    if (hKey && lRes==ERROR_SUCCESS)
    {
        dwIndex = 0;
        cbValName = 256;
        cbData = sizeof(WCHAR)*MAX_PATH;    
        
        while((lRes =RegEnumValue(hKey, dwIndex, szValName, &cbValName, NULL, &dwType, (BYTE *)pszValData, &cbData)) == ERROR_SUCCESS)
        {
            dwIndex++;
            if (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT,
                                            NORM_IGNORECASE | NORM_IGNOREWIDTH,
                                            pszDisplayName, cchToCompare,
                                            pszValData, -1))
            {
                cchToCpy = MIN(cbValName+1, *pcchOutBuf);
                lRes = StringCchCopy(pszParsingName, cchToCpy, szValName);
                break;
            }
            cbValName = 256;
            cbData = sizeof(WCHAR)*MAX_PATH;
        }
        RegCloseKey(hKey);
    }

    if (lRes == ERROR_SUCCESS)
    {
        // We have a match
        goto ExitParsingName;
    }

    // Compare with the Root
    if (FindNameInClassesRoot(szDesktopCLSID, pszDisplayName, cchToCompare))
    {
        cchToCpy = MIN(sizeof(szDesktopCLSID)/sizeof(WCHAR), *pcchOutBuf);
        lRes = StringCchCopy(pszParsingName, cchToCpy, szDesktopCLSID);
        goto ExitParsingName;
    }
    
    // As a last chance try a match on Explorer Namespace Holders
    for(i=0; i<2; i++)
    {
        hKey= NULL;
        lRes= RegOpenKeyEx(HKEY_LOCAL_MACHINE, pNameSpaceHolder[i], 0, 0, &hKey);
        if (hKey && lRes==ERROR_SUCCESS)
        {
            dwIndex= 0;
            cbValName = 256;
            while((lRes= RegEnumValue(hKey, dwIndex, szValName, &cbValName, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS)
            {
                DWORD cbCLSID = cbValName;

                if (FindNameInClassesRoot(szValName, pszDisplayName, cchToCompare))
                {
                    cchToCpy = MIN(cbCLSID+1, *pcchOutBuf);
                    lRes = StringCchCopy(pszParsingName, cchToCpy, szValName);
                    cbValName = cbCLSID; // characters copied or needed
                    break;
                }
                dwIndex++;
                cbValName = 256;
            }
            RegCloseKey(hKey);
        }

        if (lRes == ERROR_SUCCESS)
        {
            // We have a match
            goto ExitParsingName;
        }
    }
    
ExitParsingName:
    delete [] pszValData;

    if (lRes == ERROR_SUCCESS)
    {
        if (pszEndName && (*pcchOutBuf > (cchToCpy + cchInput - cchToCompare)))
        {
            wcscat(pszParsingName, pszEndName);
        }
        
        // characters copied or needed;
        *pcchOutBuf = cbValName+1;
    }

 Leave:
    return !(BOOL)lRes;
}

/*
    SaveDisplayName

    @parm LPCWSTR | pszParsingName | ParsingName used as registry key
    @parm LPCWSTR | pszDisplayName | corresponding DisplayName to be saved
    @parm LPWSTR | ppszPrevDisplayName | (Out) pointer to a pointer of a string 
        containing previuos display name if different. Can be NULL
    
    @rdesc If the function succeeds, the return value is TRUE. 
        If the function fails, the return value is FALSE.
*/
BOOL SaveDisplayName(LPCWSTR pszParsingName, LPCWSTR pszDisplayName, LPWSTR *ppszPrevDisplayName)
{
    HKEY hKey = NULL;
    DWORD dwDisp;
    
    LONG lRes = RegCreateKeyEx(HKEY_CURRENT_USER, szGUIDs, 0, NULL, 0, 0, NULL, &hKey, &dwDisp);
    if (hKey && lRes==ERROR_SUCCESS)
    {
        BOOL fNeedToSave= TRUE;

        if (ppszPrevDisplayName)
        {
            ASSERT(!*ppszPrevDisplayName);
            *ppszPrevDisplayName = (LPWSTR) LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * MAX_PATH);
            if (*ppszPrevDisplayName)
            {
                DWORD dwType= 0, cbValue= sizeof(WCHAR)*MAX_PATH;
                lRes = RegQueryValueEx(hKey, pszParsingName, NULL, &dwType, (LPBYTE)*ppszPrevDisplayName, &cbValue);

                if (lRes == ERROR_SUCCESS)
                {
                    (*ppszPrevDisplayName)[MAX_PATH-1] = 0;
                    if (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT,
                                                    NORM_IGNORECASE | NORM_IGNOREWIDTH,
                                                    pszDisplayName, -1,
                                                    *ppszPrevDisplayName,
                                                    cbValue/sizeof(WCHAR)))
                    {
                        fNeedToSave= FALSE;
                    }
                }
                
                if (lRes != ERROR_SUCCESS || !fNeedToSave)
                {
                    LocalFree(*ppszPrevDisplayName);
                    *ppszPrevDisplayName = NULL;                    
                }
            }
        }

        if (fNeedToSave)
        {
            lRes = RegSetValueEx(hKey, pszParsingName, 0, REG_SZ, (BYTE *)pszDisplayName, (wcslen(pszDisplayName)+1)*sizeof(WCHAR));
        }
        RegCloseKey(hKey);
    }

    return !(BOOL)lRes;
}


// Following API's are not available on WinCE
int GetMenuItemCount(HMENU hmenu)
{
    MENUITEMINFO mii = {0};
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_ID;
    int n = 0;

    while (GetMenuItemInfo(hmenu, n, TRUE, &mii)) n++;
    return n;
}

UINT GetMenuItemID(HMENU hmenu, int nPos)
{
    MENUITEMINFO mii;
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_ID;
    GetMenuItemInfo(hmenu, nPos, TRUE, &mii);
    return mii.wID;
}


// From taskbar's utils
LPSTR ToAnsi(LPCWSTR pszWide)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
    if (!pszWide)
    {
        return NULL;
    }
    LPSTR psz;
    UINT len = wcslen(pszWide) + 1;
    psz = (LPSTR)LocalAlloc(LMEM_FIXED, len);
    if (psz)
    {
        wcstombs(psz, pszWide, len);
    }
    return psz;
    
} /* ToAnsi()
   */

LPSTR ToAnsi(LPCSTR szSource)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
    if (!szSource)
    {
        return NULL;
    }
    LPSTR psz;
    UINT len = strlen(szSource) + 1;
    psz = (LPSTR)LocalAlloc(LMEM_FIXED, len);
    if (psz)
    {
        strcpy(psz, szSource);
    }
    return psz;
} /* ToAnsi()
   */

LPWSTR ToWide(LPCSTR psz)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
    if (!psz)
    {
        return NULL;
    }
    UINT len = strlen(psz) + 1;
    WCHAR *lpszWide;

    lpszWide = (LPWSTR)LocalAlloc(LMEM_FIXED, len * sizeof(WCHAR));
    if (lpszWide)
    {
        mbstowcs(lpszWide, psz, len);
    }
    return lpszWide;
    
} /* ToWide()
   */

LPWSTR ToWide(LPCWSTR szSource)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
    if (!szSource)
    {
        return NULL;
    }
    LPWSTR psz;
    UINT len = wcslen(szSource) + 1;
    psz = (LPWSTR)LocalAlloc(LMEM_FIXED, len * sizeof(WCHAR));
    if (psz)
    {
        wcscpy(psz, szSource);
    }
    return psz;
} /* ToWide()
   */


BOOL
NeedsEllipses(HDC hdc, LPCTSTR pszText, LPRECT prc, int FAR* pcchDraw, 
              int cxEllipses)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
    int cchText;
    int cxRect;
    SIZE siz;

    cxRect = prc->right - prc->left;

    cchText = lstrlen(pszText);

    if (cchText == 0)
    {
        *pcchDraw = cchText;
        return FALSE;
    }

    GetTextExtentExPoint(hdc, pszText, cchText, cxRect, pcchDraw, NULL, &siz);

    if (*pcchDraw == cchText)
    {
        return FALSE;
    }
    else
    {
        cxRect -= cxEllipses;
        GetTextExtentExPoint(hdc, pszText, cchText, cxRect, pcchDraw, NULL, &siz);
        return TRUE;
    }

} /* NeedsEllipses()
   */

BOOL BrowseInPlace()
{
    HKEY hKey = NULL;
    DWORD dwBrowseInPlace = 1;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Explorer"), 0, 0, &hKey))
    {
        DWORD dwSize = sizeof(dwBrowseInPlace);
        RegQueryValueEx(hKey, TEXT("BrowseInPlace"), NULL,
                        NULL, (LPBYTE)&dwBrowseInPlace, &dwSize);
        RegCloseKey(hKey);
    }

    return (0 != dwBrowseInPlace);
}

/*
    PathCanBrowseInPlace
*/
BOOL PathCanBrowseInPlace (LPCTSTR pszPath)
{
    BOOL bResult = FALSE;

    if (!pszPath || !pszPath[0])
    {
        goto Leave;
    }

    //check if path is a directory or a guid
    LPCTSTR pch = NULL;
    if (*pszPath == TEXT('\\'))
    {
        pch = pszPath;
    }
    else if (pszPath[0] == TEXT(':') && pszPath[1] == TEXT(':'))
    {
        GUID guid;
        bResult = GUIDFromString(pszPath + 2, &guid);
    }
    else if (!_tcsnicmp(pszPath, TEXT("file://"), 7))
    {
        pch = pszPath + 7;
    } 
    else if (!_tcsnicmp(pszPath, TEXT("file:"), 5))
    {
        pch = pszPath + 5;
    }

    if (pch)
    {
        bResult = (BOOL)(GetFileAttributes(pch) & FILE_ATTRIBUTE_DIRECTORY);
    }

Leave:
    return bResult;
} /* PathCanBrowseInPlace
   */
