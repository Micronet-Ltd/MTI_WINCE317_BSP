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
#include "inputlang.hpp"
#include "resource.h"
#include "utils.h"
#include <winnls.h>
#include "debugzones.h"
#include "taskbar.hxx"
#include "taskbarhelpers.h"

extern HINSTANCE g_hInstance;

CList* InputLang_t::s_pList = NULL;
HKL InputLang_t::s_hklActive = (HKL)0x00000000;
WCHAR InputLang_t::s_wszAbbrevLang[4] = L"";
bool InputLang_t::s_bIgnoreJpnKorHKL = FALSE;
const WCHAR InputLang_t::s_RegHklInstalled[] = L"SYSTEM\\CurrentControlSet\\Control\\Layouts";
const WCHAR InputLang_t::s_RegHklPreloaded[] = L"Keyboard Layout\\Preload";
const WCHAR InputLang_t::s_RegLayoutDisplayName[] = L"Layout Display Name";
const WCHAR InputLang_t::s_RegLayoutName[] = L"Layout Text";
const WCHAR InputLang_t::s_RegImeFile[] = L"Ime File";

// Special case for IMEs
// In general WinCE allows only one IME to be Preloaded at a time
// due to non-compatible registry settings (IMELEVEL for the edit control
// and IME settings for IMM)
// CHS and CHT can Preload a second IME, but still Simplified and Traditional
// group of IMEs are mutually exclusive between each other

// Simplified Chinese Pocket IMEs
const HKL InputLang_t::s_hklShuangPin = (HKL)0xe0010804;
const WCHAR InputLang_t::s_RegImeFileShuangPin[] = L"msimesp.dll";
const HKL InputLang_t::s_hklPinYin = (HKL)0xe0020804;
const WCHAR InputLang_t::s_RegImeFilePinYin[] = L"msimepy.dll";

// Traditional Chinese Pocket IMEs
const HKL InputLang_t::s_hklPhonetic = (HKL)0xe0010404;
const WCHAR InputLang_t::s_RegImeFilePhonetic[] = L"msimeph.dll";
const HKL InputLang_t::s_hklChajei = (HKL)0xe0020404;
const WCHAR InputLang_t::s_RegImeFileChajei[] = L"msimecj.dll";

// Input Locale (Keyboard layouts)

// Table used to get the full description name for supported layouts
// New Input Locales should be appended to this table.
// NOTE: It is OEM responsibility to keep this in sync with the resources.
const HKL InputLang_t::s_InputLocaleTbl[] = {
    (HKL)0x00000401, // Arabic (Saudi Arabia)
    (HKL)0x00000409, // English (United States)-US
    (HKL)0x00010409, // English (United States)-Dvorak
    (HKL)0x0000040D, // Hebrew
    (HKL)0x00000411, // Japanese
    (HKL)0x00000412, // Korean
    (HKL)0x0000041E, // Thai
    (HKL)0x00010439, // Hindi
    (HKL)0x00000446, // Punjabi
    (HKL)0x00000447, // Gujarati
    (HKL)0x00000449, // Tamil
    (HKL)0x0000044A, // Telugu
    (HKL)0x0000044B, // Kannada
    (HKL)0x0000044E // Marathi
};


// Init Preloaded Layouts
BOOL InputLang_t::InitializeInputLangList()
{
    HKEY hKeyPreloaded = NULL;
    DWORD dwType;
    WCHAR wszLayout[KL_NAMELENGTH];
    HKL hklKeyboardList[MAX_HKL_PRELOAD_COUNT];
    HKL hklDefault = NULL;
    HKL hklPreloadedIME = NULL;
    UINT cKeyboards;
    int i;

    // Reset active Input Locale
    s_hklActive = (HKL)0x00000000;
    s_wszAbbrevLang[0] = L'\0';
    s_bIgnoreJpnKorHKL = FALSE;
    
    UnInitializeInputLangList();

    s_pList = new CList();
    if (!s_pList)
    {
        goto leave;
    }

    s_pList->Lock(TRUE);

    // [HKEY_CURRENT_USER\Keyboard Layout\Preload]
    // Read unnamed value directly for default input (previously saved)
    //SystemParametersInfo(SPI_GETDEFAULTINPUTLANG, 0, (void*)&hklDefault, 0);
    if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_CURRENT_USER,
                    s_RegHklPreloaded,
                    0,
                    0,
                    &hKeyPreloaded))
    {
        DWORD cbData = sizeof(wszLayout);

        if ((ERROR_SUCCESS == ::RegQueryValueEx(hKeyPreloaded,
                    NULL,
                    NULL,
                    &dwType,
                    (BYTE*)wszLayout,
                    &cbData)) &&
            (dwType == REG_SZ))
        {
            hklDefault = SzToHkl(wszLayout);
        }
    }

    // [HKEY_CURRENT_USER\Keyboard Layout\Preload\<NUM>]
    // is used to hold the keyboard layouts available for the user to toggle between. 
    cKeyboards = ::GetKeyboardLayoutList(ARRAYSIZE(hklKeyboardList), hklKeyboardList);

    // First pass: check if Japanese or Korean IMEs are present
    for (i = 0; i < (int)cKeyboards; i++)
    {
        if (IsHklIme(hklKeyboardList[i]) &&
            ((0x0411 == LOWORD(hklKeyboardList[i])) ||
            (0x0412 == LOWORD(hklKeyboardList[i]))))
        {
            s_bIgnoreJpnKorHKL = TRUE;
        }
    }

    for (i = 0; i < (int)cKeyboards; i++)
    {
        if (IsHklIme(hklKeyboardList[i]))
        {
            if ((hklPreloadedIME ||
            (IsHklIme(hklDefault) &&
            (hklDefault != hklKeyboardList[i]) &&
            ((!IsSimplifiedChineseIME(hklDefault) ||
            !IsSimplifiedChineseIME(hklKeyboardList[i])) &&
            (!IsTraditionalChineseIME(hklDefault) ||
            !IsTraditionalChineseIME(hklKeyboardList[i]))))) &&
            ((!IsSimplifiedChineseIME(hklPreloadedIME) ||
            !IsSimplifiedChineseIME(hklKeyboardList[i])) &&
            (!IsTraditionalChineseIME(hklPreloadedIME) ||
            !IsTraditionalChineseIME(hklKeyboardList[i]))))
            {
                continue;
            }
            else
            {
                hklPreloadedIME = hklKeyboardList[i];
            }
        }

        AddInputLocale(hklKeyboardList[i]);
    }

    s_pList->Lock(FALSE);

leave:
    if (hKeyPreloaded)
    {
        RegCloseKey(hKeyPreloaded);
    }

    return (BOOL)s_pList;
}

BOOL InputLang_t::UnInitializeInputLangList()
{
    BOOL bRet = FALSE;
    LPINPUTLANGLISTOBJ lpiobj;
    LPINPUTLANGLISTOBJ lpiobjNext;

    if (!s_pList)
    {
        goto leave;
    }

    s_pList->Lock(TRUE);

    lpiobj = (LPINPUTLANGLISTOBJ)s_pList->Head();
    while (lpiobj)
    {
        lpiobjNext = (LPINPUTLANGLISTOBJ)s_pList->Next(&lpiobj->listObject);

        // Free resource
        s_pList->Disconnect(&lpiobj->listObject);
        if (lpiobj->inputLocale.bAllocated)
        {
            ::LocalFree((LPWSTR)lpiobj->inputLocale.pwszName);
        }
        ::LocalFree(lpiobj);

        lpiobj = lpiobjNext;
    }

    s_pList->Lock(FALSE);

    ASSERT(0 == s_pList->GetCount());
    delete s_pList;
    s_pList = NULL;

leave:
    return bRet;
}

// Converts an input method handle to an input method name
LPWSTR InputLang_t::HklToSz(HKL hklIdentifier, WCHAR wszLayout[KL_NAMELENGTH])
{
    LPWSTR pwszHkl = NULL;
    int iErr;

    if (!hklIdentifier || !wszLayout)
    {
        goto leave;
    }

    wszLayout[KL_NAMELENGTH - 1] = L'\0';
    iErr = ::_snwprintf(wszLayout, (KL_NAMELENGTH - 1), L"%08X", (UINT)hklIdentifier);
    ASSERT(iErr == (KL_NAMELENGTH - 1));

    pwszHkl = wszLayout;

leave:
    return pwszHkl;
}

// Converts an input method name to an input method handle
HKL InputLang_t::SzToHkl(LPCWSTR pwszLayout)
{
    HKL hklIdentifier = 0;
    LPWSTR pwszStop;
    size_t cch;

    if (!pwszLayout ||
        FAILED(::StringCchLength(pwszLayout, KL_NAMELENGTH, &cch)) ||
        cch != (KL_NAMELENGTH - 1))
    {
        goto leave;
    }

    hklIdentifier = (HKL)::wcstoul(pwszLayout, &pwszStop, 16);
    ASSERT(pwszStop && pwszStop[0] == L'\0');

leave:
    return hklIdentifier;
}

BOOL InputLang_t::AddInputLocale(HKL hklIdentifier)
{
    BOOL bRet = FALSE;
    LPINPUTLANGLISTOBJ lpiobj;
    LPINPUTLANGLISTOBJ lpiobjNew;
    int i;

    // Do not show Japanese or Korean keyboard layouts
    // Each will get loaded by respective IME
    if (!hklIdentifier ||
        (s_bIgnoreJpnKorHKL &&
        (((HKL)0x00000411 == hklIdentifier) ||
        ((HKL)0x00000412 == hklIdentifier))) ||
        !s_pList)
    {
        goto exit;
    }

    s_pList->Lock(TRUE);

    // It is important to keep the list sorted by Language ID
    // TrackPopupMenu code relies on this

    lpiobj = (LPINPUTLANGLISTOBJ)s_pList->Tail();
    while (lpiobj)
    {
        if (LOWORD(hklIdentifier) > LOWORD(lpiobj->inputLocale.hklIdentifier))
        {
            break;
        }
        else if (LOWORD(hklIdentifier) == LOWORD(lpiobj->inputLocale.hklIdentifier))
        {
            if (hklIdentifier == lpiobj->inputLocale.hklIdentifier)
            {
                goto leave;
            }
            else if (hklIdentifier > lpiobj->inputLocale.hklIdentifier)
            {
                break;
            }
        }

        lpiobj = (LPINPUTLANGLISTOBJ)s_pList->Prev(&lpiobj->listObject);
    }

    lpiobjNew = (LPINPUTLANGLISTOBJ)::LocalAlloc(LMEM_FIXED, sizeof(INPUTLANGLISTOBJ));
    if (!lpiobjNew)
    {
        goto leave;
    }

    lpiobjNew->inputLocale.hklIdentifier = hklIdentifier;

    lpiobjNew->inputLocale.pwszName = NULL;
    lpiobjNew->inputLocale.bAllocated = FALSE;
    for (i = 0; i < ARRAYSIZE(s_InputLocaleTbl); i++)
    {
        if (hklIdentifier == s_InputLocaleTbl[i])
        {
            break;
        }
    }

    if (i < ARRAYSIZE(s_InputLocaleTbl))
    {
        lpiobjNew->inputLocale.pwszName = (LPCWSTR)::LoadString(g_hInstance,
                        (IDS_BASE_HKL__ + i),
                        NULL,
                        0);
    }

    if (!lpiobjNew->inputLocale.pwszName)
    {
        // Get name from registry
        HKEY hKeyLayouts = NULL;
        HKEY hKeyKeyboard = NULL;
        DWORD dwType;
        DWORD cbData;
        LPWSTR pwszName;
        WCHAR wszLayout[KL_NAMELENGTH];

        wszLayout[ARRAYSIZE(wszLayout) - 1] = L'\0';
        int iErr = ::_snwprintf(wszLayout, (ARRAYSIZE(wszLayout) - 1), L"%08X", (UINT)hklIdentifier);
        ASSERT(iErr == (KL_NAMELENGTH - 1));

        if ((ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        s_RegHklInstalled,
                        0,
                        0,
                        &hKeyLayouts)) &&
            (ERROR_SUCCESS == ::RegOpenKeyEx(hKeyLayouts, wszLayout, 0, 0, &hKeyKeyboard)))
        {
            cbData = sizeof(WCHAR) * MAX_PATH;
            pwszName = (LPWSTR)::LocalAlloc(LMEM_FIXED, cbData);
            if (!pwszName)
            {
                goto CloseRegistryKeys;
            }

            // 1. "Layout Display Name" - Indirect String
            if ((ERROR_SUCCESS != ::RegQueryValueEx(hKeyKeyboard,
                            s_RegLayoutDisplayName,
                            NULL,
                            &dwType,
                            (LPBYTE)pwszName,
                            &cbData)) ||
                (dwType != REG_SZ) ||
                FAILED(SHLoadIndirectString(pwszName, pwszName, MAX_PATH, NULL)))
            {
                // 2. "Layout Text" - display description text (fall back)
                cbData = sizeof(WCHAR) * MAX_PATH;
                if ((ERROR_SUCCESS != ::RegQueryValueEx(hKeyKeyboard,
                                s_RegLayoutName,
                                NULL,
                                &dwType,
                                (LPBYTE)pwszName,
                                &cbData)) ||
                    (dwType != REG_SZ))
                {
                    ::LocalFree(pwszName);
                    goto CloseRegistryKeys;
                }
            }

            lpiobjNew->inputLocale.pwszName = pwszName;
            lpiobjNew->inputLocale.bAllocated = TRUE;
        }

CloseRegistryKeys:
        if (hKeyKeyboard)
        {
            ::RegCloseKey(hKeyKeyboard);
        }

        if (hKeyLayouts)
        {
            ::RegCloseKey(hKeyLayouts);
        }
    }

    if (!lpiobjNew->inputLocale.pwszName)
    {
        ASSERT(0);
        ::LocalFree(lpiobjNew);
        goto leave;
    }

    if (!lpiobj)
    {
        s_pList->AddHead(&lpiobjNew->listObject);
    }
    else
    {
        s_pList->InsertAfterObject(&lpiobjNew->listObject, &lpiobj->listObject);
    }

    bRet = TRUE;

leave:
    s_pList->Lock(FALSE);

exit:
    return bRet;
}

LPINPUTLANGLISTOBJ InputLang_t::FindInputLangListObject(HKL hklIdentifier)
{
    LPINPUTLANGLISTOBJ lpiobj;

    lpiobj = (LPINPUTLANGLISTOBJ)s_pList->Head();
    while (lpiobj)
    {
        if (hklIdentifier == lpiobj->inputLocale.hklIdentifier)
        {
            break;
        }
        lpiobj = (LPINPUTLANGLISTOBJ)s_pList->Next(&lpiobj->listObject);
    }

    return lpiobj;
}

HKL InputLang_t::GetInputLocale()
{
    return s_hklActive;
}

LPCWSTR InputLang_t::GetInputLocaleAbbreviation()
{
    return (LPCWSTR)s_wszAbbrevLang;
}

LPCWSTR InputLang_t::GetInputLocaleDescription()
{
    LPCWSTR pwszDescription = NULL;
    LPINPUTLANGLISTOBJ lpiobj;

    if (!s_pList)
    {
        goto exit;
    }

    s_pList->Lock(TRUE);

    lpiobj = FindInputLangListObject(s_hklActive);
    if (!lpiobj)
    {
        goto leave;
    }

    pwszDescription = lpiobj->inputLocale.pwszName;

leave:
    s_pList->Lock(FALSE);

exit:
    return pwszDescription;
}

BOOL InputLang_t::RemoveInputLocale(HKL hklIdentifier)
{
    BOOL bRet = FALSE;
    LPINPUTLANGLISTOBJ lpiobj;

    if (!s_pList)
    {
        goto leave;
    }

    s_pList->Lock(TRUE);

    lpiobj = FindInputLangListObject(hklIdentifier);
    if (lpiobj)
    {
        bRet = TRUE;
        s_pList->Disconnect(&lpiobj->listObject);
        if (lpiobj->inputLocale.bAllocated)
        {
            ::LocalFree((LPWSTR)lpiobj->inputLocale.pwszName);
        }
        ::LocalFree(lpiobj);
    }

    s_pList->Lock(FALSE);

leave:
    return bRet;

}

BOOL InputLang_t::ShowIcon()
{
    BOOL bRet = FALSE;

    if (!s_pList)
    {
        goto leave;
    }

    s_pList->Lock(TRUE);

    // Add support for registry key. A value other than zero shows the icon
    // [HKEY_LOCAL_MACHINE\Software\Microsoft\InputLanguage]
    //     ShowIcon = dword:0

    bRet = (BOOL)(s_pList->GetCount() > 1);

    s_pList->Lock(FALSE);

leave:
    return bRet;
}

UINT InputLang_t::TrackPopUpMenu(HWND hwndTaskBar, LPCRECT prcIcon)
{
    HMENU hmenuTrack = NULL;
    LPINPUTLANGLISTOBJ lpiobj;
    LPINPUTLANGLISTOBJ lpiobjNext;
    WORD wPreviousLangID = 0;
    RECT rcTemp;
    UINT uRet = 0;

    if (!hwndTaskBar || !prcIcon || !s_pList)
    {
        goto exit;
    }

    s_pList->Lock(TRUE);

    hmenuTrack = ::CreatePopupMenu();
    if (!hmenuTrack)
    {
        goto leave;
    }

    // Fill menu
    lpiobj = (LPINPUTLANGLISTOBJ)s_pList->Head();
    while (lpiobj)
    {
        WCHAR wszTemp[MAX_PATH];
        LPWSTR pwszDisplayName;
        LPCWSTR pwszSeparator;

        lpiobjNext = (LPINPUTLANGLISTOBJ)s_pList->Next(&lpiobj->listObject);

        if (!lpiobj->inputLocale.pwszName)
        {
            ASSERT(0);
            goto nextListObject;
        }

        pwszSeparator = ::wcschr(lpiobj->inputLocale.pwszName, L'$');
        if (pwszSeparator)
        {
            // Use full description name when there is more than one input
            // locale for the same input language.
            // Use shorter name otherwise.

            size_t cch = pwszSeparator - lpiobj->inputLocale.pwszName;

            if (cch > ARRAYSIZE(wszTemp) ||
                FAILED(::StringCchCopy(wszTemp,
                            ARRAYSIZE(wszTemp),
                            lpiobj->inputLocale.pwszName)))
            {
                goto useDefault;
            }
                
            pwszDisplayName = wszTemp;

            if ((wPreviousLangID == LOWORD(lpiobj->inputLocale.hklIdentifier)) ||
                (lpiobjNext &&
                (LOWORD(lpiobjNext->inputLocale.hklIdentifier) == LOWORD(lpiobj->inputLocale.hklIdentifier))))
            {
                // replace string separator
                pwszDisplayName[cch] = L'-';
            }
            else
            {              
                // truncate at string separator
                pwszDisplayName[cch] = L'\0';
                wPreviousLangID = LOWORD(lpiobj->inputLocale.hklIdentifier);
            }
        }
        else
        {
useDefault:
            pwszDisplayName = (LPWSTR)lpiobj->inputLocale.pwszName;
        }

        if (!::AppendMenu(hmenuTrack,
                        MF_STRING,
                        (UINT)lpiobj->inputLocale.hklIdentifier,
                        pwszDisplayName))
        {
            goto leave;
        }

nextListObject:
        lpiobj = lpiobjNext;
    }

    // Mark the input locale which is currently active
    ::CheckMenuItem(hmenuTrack, (UINT)s_hklActive, MF_CHECKED);

    rcTemp = *prcIcon;
    MapWindowPoints(hwndTaskBar, NULL, (LPPOINT)&rcTemp, 2);

    uRet = ::TrackPopupMenu(hmenuTrack,
                    TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RETURNCMD,
                    (::GetWindowLong(hwndTaskBar, GWL_EXSTYLE) & WS_EX_LAYOUTRTL) ?
                            rcTemp.left :
                            rcTemp.right,
                    rcTemp.top,
                    0,
                    hwndTaskBar,
                    NULL);

    // Selection of current input locale is same as cancel.
    if (s_hklActive == (HKL)uRet)
    {
        uRet = 0;
    }

leave:
    s_pList->Lock(FALSE);

    if (hmenuTrack)
    {
        ::DestroyMenu(hmenuTrack);
    }

exit:
    return uRet;
}

BOOL InputLang_t::UpdateInputLocale(HKL hklIdentifier)
{
    BOOL bRet = FALSE;

    if ((hklIdentifier == s_hklActive) || !s_pList)
    {
        goto exit;
    }

    s_pList->Lock(TRUE);

    if (!FindInputLangListObject(hklIdentifier) &&
        !AddInputLocale(hklIdentifier))
    {
        goto leave;
    }

    bRet = TRUE;
    s_hklActive = hklIdentifier;

    if (!::GetLocaleInfo(LOWORD(hklIdentifier),
                    LOCALE_SABBREVLANGNAME,
                    s_wszAbbrevLang,
                    ARRAYSIZE(s_wszAbbrevLang)))
    {
        // NLS data not included for this Locale
        DEBUGMSG(ZONE_WARNING,
                (L"Input Language: NLS data not included for Locale 0x%x", LOWORD(hklIdentifier)));
        s_wszAbbrevLang[0] = L'\0';
        goto leave;
    }

    // SABBREVLANGNAME has three letters, remove the last one
    s_wszAbbrevLang[2] = L'\0';

leave:
    s_pList->Lock(FALSE);

exit:
    return bRet;
}

BOOL InputLang_t::VerifyImeFile(HKL hklIdentifier, LPCWSTR pwszImeFile)
{
    HKEY hKeyLayouts = NULL;
    HKEY hKeyKeyboard = NULL;
    BOOL bRet = FALSE;

    if (!hklIdentifier || !pwszImeFile)
    {
        goto leave;
    }

    if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    s_RegHklInstalled,
                    0,
                    0,
                    &hKeyLayouts))
    {
        WCHAR wszLayout[KL_NAMELENGTH];

        if (HklToSz(hklIdentifier, wszLayout) &&
            (ERROR_SUCCESS == ::RegOpenKeyEx(hKeyLayouts, wszLayout, 0, 0, &hKeyKeyboard)))
        {
            WCHAR wszTemp[MAX_PATH];
            DWORD dwType;
            DWORD cbData = sizeof(wszTemp);
            if ((ERROR_SUCCESS == ::RegQueryValueEx(hKeyKeyboard,
                            s_RegImeFile,
                            NULL,
                            &dwType,
                            (LPBYTE)wszTemp,
                            &cbData)) &&
                (dwType == REG_SZ) &&
                (0 == ::wcscmp(wszTemp, pwszImeFile)))
            {
                bRet = TRUE;
            }
        }
    }

    if (hKeyLayouts)
    {
        RegCloseKey(hKeyLayouts);
    }

    if (hKeyKeyboard)
    {
        RegCloseKey(hKeyKeyboard);
    }

leave:
    return bRet;
}


InputLangNotification::InputLangNotification() :
    TaskbarNotification()
{
    m_hfontInputLang = NULL;
}

InputLangNotification::InputLangNotification(RECT rcIcon, HWND hwndTray) :
    TaskbarNotification(NULL, HHTBS_NOTIFY | HHTBS_INPUTLANG_NOTIFY, rcIcon, hwndTray, L"", HHTBI_INPUTLANG, INPUTLANG_NOTIFYICON)
{
    m_hfontInputLang = NULL;
}

InputLangNotification::~InputLangNotification()
{
    if (m_hfontInputLang != NULL)
    {
        DeleteObject(m_hfontInputLang);
        m_hfontInputLang = NULL;        
    }
}

BOOL InputLangNotification::CalcSizeInputLang(HWND hwndTaskBar)
{
    BOOL bRet = FALSE;
    LPCWSTR pwszAbbrevLang;
    HDC hdc = NULL;
    HFONT hfontOld = NULL;
    SIZE size = {0};

    hdc = GetDC(hwndTaskBar);
    if (!hdc)
    {
        goto leave;
    }

    if (!m_hfontInputLang)
    {
        LOGFONT lf;
        HFONT hSystemFont;

        hSystemFont = (HFONT)GetStockObject(SYSTEM_FONT);
        if (hSystemFont)
        {
            memset(&lf, 0, sizeof(LOGFONT));
            GetObject(hSystemFont, sizeof(LOGFONT), (void*)&lf);

            lf.lfHeight = 12;
            lf.lfWeight =  FW_BOLD;

            m_hfontInputLang = CreateFontIndirect(&lf);
        }
    }

    if (m_hfontInputLang)
    {
        hfontOld = (HFONT)SelectObject(hdc, m_hfontInputLang);
    }

    pwszAbbrevLang = InputLang_t::GetInputLocaleAbbreviation();
    GetTextExtentExPoint(hdc,
                    pwszAbbrevLang,
                    ::wcslen(pwszAbbrevLang),
                    100,
                    NULL,
                    NULL,
                    &size);

    if (size.cx)
    {
        // Add some spacing
        size.cx += (_SEPERATOR*2 + 1);
    }

    m_rc.left = m_rc.right - size.cx;

    if (hfontOld)
    {
        SelectObject(hdc, hfontOld);
    }
    bRet = TRUE;

leave:
    if (hdc)
    {
        ReleaseDC(hwndTaskBar, hdc);
    }
    return bRet;
}

BOOL InputLangNotification::UpdateInputLangIcon(HKL hklIdentifier)
{
    return FALSE;
}

BOOL InputLangNotification::DrawItem(HWND hwndTaskBar, HWND hwndTray, HDC hdc)
{
    HFONT hfontOld = NULL;
    HBRUSH hbrOld = NULL;
    COLORREF clrTextOld = CLR_NONE;
    RECT rcInputLangIcon;
    CopyRect(&rcInputLangIcon, &m_rc);
    MapWindowPoints(hwndTaskBar, hwndTray, (LPPOINT)&rcInputLangIcon, 2);

    LPCWSTR pwszAbbrevLang = InputLang_t::GetInputLocaleAbbreviation();

    if (m_hfontInputLang)
    {
        hfontOld = (HFONT)SelectObject(hdc, m_hfontInputLang);
    }

    hbrOld = (HBRUSH)SelectObject(hdc, GetSysColorBrush(COLOR_BTNTEXT));
    int r = 2;
    RoundRect(hdc,
              rcInputLangIcon.left,
              rcInputLangIcon.top,
              rcInputLangIcon.right,
              rcInputLangIcon.bottom,
              r,
              r);
    if (hbrOld)
    {
        SelectObject(hdc, hbrOld);
    }

    clrTextOld = SetTextColor(hdc, GetSysColor(COLOR_WINDOW));
    DrawText(hdc,
             pwszAbbrevLang,
             -1,
             &rcInputLangIcon,
             DT_CENTER | DT_VCENTER);

    if (clrTextOld != CLR_NONE)
    {
        SetTextColor(hdc, clrTextOld);
    }

    if (hfontOld)
    {
        SelectObject(hdc, hfontOld);
    }
    
    return TRUE;
}

