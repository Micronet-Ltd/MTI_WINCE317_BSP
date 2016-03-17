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

#include "taskbarnotification.h"

#define INPUTLANG_NOTIFYICON    (WM_USER + 5)
#define HHTBI_INPUTLANG         4

typedef struct tagINPUTLOCALE {
    HKL hklIdentifier;
    LPCWSTR pwszName;
    bool bAllocated;
} INPUTLOCALE, *LPINPUTLOCALE;

typedef struct tagINPUTLANGLISTOBJ {
    LISTOBJ listObject;
    INPUTLOCALE inputLocale;
} INPUTLANGLISTOBJ, *LPINPUTLANGLISTOBJ;



class InputLang_t
{
protected:
    static CList* s_pList;
    static HKL s_hklActive;
    static WCHAR s_wszAbbrevLang[4];
    static bool s_bIgnoreJpnKorHKL;

    static const HKL s_InputLocaleTbl[];
    static const WCHAR s_RegHklInstalled[];
    static const WCHAR s_RegHklPreloaded[];
    static const WCHAR s_RegImeFile[];
    static const WCHAR s_RegLayoutName[];
    static const WCHAR s_RegLayoutDisplayName[];

    // Simplified Chinese Pocket IMEs
    static const HKL s_hklShuangPin;
    static const WCHAR s_RegImeFileShuangPin[];
    static const HKL s_hklPinYin;
    static const WCHAR s_RegImeFilePinYin[];

    // Traditional Chinese Pocket IMEs
    static const HKL s_hklPhonetic;
    static const WCHAR s_RegImeFilePhonetic[];
    static const HKL s_hklChajei;
    static const WCHAR s_RegImeFileChajei[];

    inline static BOOL IsHklIme(HKL hklIdentifier)
    {
        return ((((UINT) hklIdentifier) & 0xff000000) == 0xe0000000);
    }

    inline static BOOL IsSimplifiedChineseIME(HKL hklIdentifier)
    {
        return (((hklIdentifier == s_hklShuangPin) &&
            VerifyImeFile(hklIdentifier, s_RegImeFileShuangPin)) ||
            ((hklIdentifier == s_hklPinYin) &&
            VerifyImeFile(hklIdentifier, s_RegImeFilePinYin)));
    }

    inline static BOOL IsTraditionalChineseIME(HKL hklIdentifier)
    {
        return (((hklIdentifier == s_hklPhonetic) &&
            VerifyImeFile(hklIdentifier, s_RegImeFilePhonetic)) ||
            ((hklIdentifier == s_hklChajei) &&
            VerifyImeFile(hklIdentifier, s_RegImeFileChajei)));
    }

    static LPINPUTLANGLISTOBJ FindInputLangListObject(HKL hklIdentifier);
    static LPWSTR HklToSz(HKL hklIdentifier, WCHAR wszLayout[KL_NAMELENGTH]);
    static HKL SzToHkl(LPCWSTR pwszLayout);
    static BOOL VerifyImeFile(HKL hklIdentifier, LPCWSTR pwszImeFile);

public:
    static BOOL InitializeInputLangList();
    static BOOL UnInitializeInputLangList();

    static BOOL AddInputLocale(HKL hklIdentifier);
    static HKL GetInputLocale();
    static LPCWSTR GetInputLocaleAbbreviation();
    static LPCWSTR GetInputLocaleDescription();
    static BOOL RemoveInputLocale(HKL hklIdentifier);
    static BOOL ShowIcon();
    static UINT TrackPopUpMenu(HWND hwndTaskBar, LPCRECT prcIcon);
    static BOOL UpdateInputLocale(HKL hklIdentifier);
};

class InputLangNotification :
    public TaskbarNotification
{
private:
    HFONT  m_hfontInputLang;

public:
    InputLangNotification();
    InputLangNotification(RECT rcIcon, HWND hwndTray);
    virtual ~InputLangNotification();
    
    BOOL CalcSizeInputLang(HWND hwndTaskBar);
    BOOL UpdateInputLangIcon(HKL hklIdentifier);
    BOOL DrawItem(HWND hwndTaskBar, HWND hwndTray, HDC hdc);
};

