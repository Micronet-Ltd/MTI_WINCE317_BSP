/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
  IF YOU ARE A LICENSEE, YOUR RIGHT, IF ANY, TO COPY, PUBLISH, MODIFY, OR
  OTHERWISE USE THE SOFTWARE, IS SUBJECT TO THE TERMS AND CONDITIONS OF THE
  BINDING AGREEMENTS.  BY USING THE SOFTWARE IN ANY MANNER, IN WHOLE OR IN
  PART, YOU AGREE TO BE BOUND BY THE TERMS OF THE BINDING AGREEMENTS.

  IF YOU ARE NOT A DATALIGHT LICENSEE, ANY USE MAY RESULT IN CIVIL AND
  CRIMINAL ACTION AGAINST YOU.  CONTACT DATALIGHT, INC. AT THE ADDRESS
  SET FORTH ABOVE IF YOU OBTAINED THIS SOFTWARE IN ERROR.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This module implements an interface for storing and retrieving common
    application settings from the system registry.

    This code must be fully compilable and usable under both normal Win32
    user mode, as well as Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: winsettings.c $
    Revision 1.3  2010/11/01 04:20:22Z  garyp
    Added some missing typecasts which caused some bad configuration
    data to be used.  Added DclWinSettingsDelete().
    Revision 1.2  2010/01/23 16:44:26Z  garyp
    Fixed to avoid using an uninitialized variable in an error case.
    Revision 1.1  2009/07/13 18:24:22Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>
#include <dlwinutil.h>
#include "winutilgui.rh"

#define MAX_KEY_LEN     (255)

typedef enum
{
    TYP_BYTE,
    TYP_BOOL,
    TYP_LONG,
    TYP_WSTR,
} SETTINGTYPE;

typedef struct
{
    const D_WCHAR  *pwzSettingName; /* Setting name in the registry */
    SETTINGTYPE     nType;          /* Data type of the field in the DCLWINSETTINGS structure */
    const size_t    nOffset;        /* Offset of the field in the DCLWINSETTINGS structure */
    const size_t    nSize;          /* Size of the field in the DCLWINSETTINGS structure */
    const D_INTPTR  iDefault;       /* Container large enough for a pointer or DWORD */
} SETTINGINFO;

#define MEMBER(typ, member) offsetof(typ, member), DCLSIZEOFMEMBER(typ, member)

static const SETTINGINFO aSettings[] =
{
    {TEXT("EscKeyToExit"),     TYP_BOOL, MEMBER(DCLWINSETTINGS, fEscKeyToExit),       FALSE},
    {TEXT("FontHeight"),       TYP_LONG, MEMBER(DCLWINSETTINGS, lFontHeight),         -17},
    {TEXT("FontWeight"),       TYP_LONG, MEMBER(DCLWINSETTINGS, lFontWeight),         FW_NORMAL},
    {TEXT("FontItalic"),       TYP_BOOL, MEMBER(DCLWINSETTINGS, fFontItalic),         FALSE},
    {TEXT("FontPitch&Family"), TYP_BYTE, MEMBER(DCLWINSETTINGS, bFontPitchAndFamily), DEFAULT_PITCH | FF_DONTCARE},
    {TEXT("FontFaceName"),     TYP_WSTR, MEMBER(DCLWINSETTINGS, wzFontFaceName),      (D_INTPTR)TEXT("Courier New")}
};

static const D_WCHAR wzPath[] = {TEXT("Software\\Datalight, Inc.\\")};

static BOOL CALLBACK DialogProc(HWND hWndDlg, UINT nMsg, WPARAM wParam, LPARAM lParam);
static DCLSTATUS CopySetting(const SETTINGINFO *pSetting, void *pDest, const void *pSource);


/*-------------------------------------------------------------------
    Public: DclWinSettingsLoad()

    Load the program's settings from the system registry.  If the
    key corresponding to pwzAppName does not exist, or for any values
    which are not found, default values will be used.

    Settings are loaded from HKLM\Software\Datalight, Inc.\pszAppName.

    Parameters:
        pwzAppName       - A pointer to the null-terminated application
                           name.
        pDefaultSettings - A pointer to the default settings value to
                           use for those settings not found in the
                           registry.  May be NULL to use the internal
                           system default settings values.
        pSettingsData    - A pointer to the DCLWINSETTINGS structure
                           to use.

    Return Value:
        Returns a DCLSTATUS value indicating the result.  The status
        code DCLSTAT_WIN_REGKEYNOTFOUND indicates that the main key
        did not exist, and that default values were used.
-------------------------------------------------------------------*/
DCLSTATUS DclWinSettingsLoad(
    const D_WCHAR          *pwzAppName,
    const DCLWINSETTINGS   *pDefaultSettings,
    DCLWINSETTINGS         *pSettingsData)
{
    LONG                    lError;
    D_WCHAR                 wzKey[MAX_KEY_LEN];
    HKEY                    hKey = 0;
    DCLSTATUS               dclStat;
    unsigned                nn;

    DclAssertReadPtr(pwzAppName, 0);
    DclAssertWritePtr(pSettingsData, sizeof(*pSettingsData));

    /*  Start with a clean output structure
    */
    DclMemSet(pSettingsData, 0, sizeof(*pSettingsData));

    wcscpy(wzKey, wzPath);
    wcscat(wzKey, pwzAppName);

    lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wzKey, 0, KEY_READ, &hKey);
    if(lError != ERROR_SUCCESS)
    {
        DCLPRINTF(1, ("DclWinSettingsLoad() failed to open the key, \"%W\", Error=%ld\n", wzKey, lError));

        if(pDefaultSettings)
        {
            DCLPRINTF(1, ("DclWinSettingsLoad() using the supplied default settings\n"));

            *pSettingsData = *pDefaultSettings;
        }
        else
        {
            /*  No default settings were supplied, so use internal system
                default settings.
            */
            DCLPRINTF(1, ("DclWinSettingsLoad() using internal system default settings\n"));

            for(nn=0; nn<DCLDIMENSIONOF(aSettings); nn++)
            {
                void *pDest = ((char*)pSettingsData) + aSettings[nn].nOffset;

                dclStat = CopySetting(&aSettings[nn], pDest, &aSettings[nn].iDefault);
                if(dclStat != DCLSTAT_SUCCESS)
                    goto Cleanup;
            }
        }

        return DCLSTAT_WIN_REGKEYNOTFOUND;
    }

    for(nn=0; nn<DCLDIMENSIONOF(aSettings); nn++)
    {
        DWORD           dwValType;
        DWORD           dwExpectedType;
        DWORD           dwValLen = aSettings[nn].nSize;
        const D_WCHAR  *pwzName = aSettings[nn].pwzSettingName;
        void           *pDest = ((char*)pSettingsData) + aSettings[nn].nOffset;
        DWORD           dwValue = 0; /* (Unnecessary init for a dump compiler) */
        const void     *pDefault;

        /*  If the destination field is a string, then the registry data
            must be of the type REG_SZ.  For any other type of destination
            field, the type expected in the registry must be REG_DWORD.
        */
        if(aSettings[nn].nType == TYP_WSTR)
        {
            dwExpectedType = REG_SZ;
        }
        else
        {
            dwExpectedType = REG_DWORD;

            if(aSettings[nn].nType == TYP_BYTE)
            {
                pDest = &dwValue;
                dwValLen = sizeof(DWORD);
            }

            /*  Regardless what the ultimate destination data type is, any
                REG_DWORD types must a have a length which matches the size
                of a DWORD.
            */
            DclAssert(dwValLen == sizeof(DWORD));
        }

        lError = RegQueryValueEx(hKey, pwzName, NULL, &dwValType, pDest, &dwValLen);
        if(lError != ERROR_SUCCESS || dwValType != dwExpectedType)
        {
            if(lError != ERROR_SUCCESS)
                DCLPRINTF(1, ("DclWinSettingsLoad() failed to find the %W setting, Error=%ld, using default value.\n", pwzName, lError));

            if(dwValType != dwExpectedType)
                DCLPRINTF(1, ("DclWinSettingsLoad() found an %W setting of the wrong type %lU, using default value.\n", pwzName, dwValType));

            if(pDefaultSettings)
                pDefault = ((char*)pDefaultSettings) + aSettings[nn].nOffset;
            else
                pDefault = &aSettings[nn].iDefault;

            dclStat = CopySetting(&aSettings[nn], pDest, pDefault);
            if(dclStat != DCLSTAT_SUCCESS)
                goto Cleanup;
        }
        else
        {
            if(aSettings[nn].nType == TYP_BYTE)
            {
                pDest = ((char*)pSettingsData) + aSettings[nn].nOffset;

                if(dwValue > D_UINT8_MAX)
                {
                    DCLPRINTF(1, ("DclWinSettingsLoad() found an out-of-range %W setting of %lU, using default value.\n", pwzName, dwValue));

                    if(pDefaultSettings)
                        pDefault = ((char*)pDefaultSettings) + aSettings[nn].nOffset;
                    else
                        pDefault = &aSettings[nn].iDefault;

                    dclStat = CopySetting(&aSettings[nn], pDest, pDefault);
                    if(dclStat != DCLSTAT_SUCCESS)
                        goto Cleanup;
                }
                else
                {
                    *(BYTE*)pDest = (BYTE)dwValue;
                }
            }
        }
    }

    dclStat = DCLSTAT_SUCCESS;

  Cleanup:

    RegCloseKey(hKey);

    return dclStat;
}


/*-------------------------------------------------------------------
    Public: DclWinSettingsSave()

    Save standard settings to the registry.
    
    Settings are stored in HKLM\Software\Datalight, Inc.\pszAppName.

    Parameters:
        pwzAppName    - A pointer to the null-terminated application
                        name.
        pSettingsData - A pointer to the DCLWINSETTINGS structure
                        to use.

    Return Value:
        Returns a DCLSTATUS value indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclWinSettingsSave(
    const D_WCHAR          *pwzAppName,
    const DCLWINSETTINGS   *pSettingsData)
{
    LONG                    lError;
    D_WCHAR                 wzKey[MAX_KEY_LEN];
    HKEY                    hKey = 0;
    DCLSTATUS               dclStat;
    unsigned                nn;

    DclAssertReadPtr(pwzAppName, 0);
    DclAssertReadPtr(pSettingsData, sizeof(*pSettingsData));

    /*  Validate all the settings first
    */
    if(pSettingsData->fEscKeyToExit != 0 && pSettingsData->fEscKeyToExit != 1)
    {
        DclPrintf("DCL: Setting EscKeyToExit value of %u is not valid\n", pSettingsData->fEscKeyToExit);
        return DCLSTAT_WIN_REGBADSETTINGSVALUE;
    }

    wcscpy(wzKey, wzPath);
    wcscat(wzKey, pwzAppName);

    lError = RegCreateKeyEx(HKEY_LOCAL_MACHINE, wzKey, 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL);
    if(lError != ERROR_SUCCESS)
    {
        DCLPRINTF(1, ("\nDclWinSettingsSave() failed to open/create the key, \"%W\", Error=%ld\n", wzKey, lError));
        return DCLSTAT_WIN_REGKEYNOTFOUND;
    }

    for(nn=0; nn<DCLDIMENSIONOF(aSettings); nn++)
    {
        DWORD           dwValType;
        DWORD           dwValLen = aSettings[nn].nSize;
        const D_WCHAR  *pwzName = aSettings[nn].pwzSettingName;
        const void     *pValue = ((char*)pSettingsData) + aSettings[nn].nOffset;
        DWORD           dwValue;

        if(aSettings[nn].nType == TYP_WSTR)
        {
            dwValType = REG_SZ;
        }
        else
        {
            dwValType = REG_DWORD;

            if(aSettings[nn].nType == TYP_BYTE)
            {
                dwValue = (DWORD)*(BYTE*)pValue;
                pValue = &dwValue;
                dwValLen = sizeof(DWORD);
            }

            /*  Regardless what the ultimate destination data type is, any
                REG_DWORD types must a have a length which matches the size
                of a DWORD.
            */
            DclAssert(dwValLen == sizeof(DWORD));
        }

        lError = RegSetValueEx(hKey, pwzName, 0, dwValType, pValue, dwValLen);
        if(lError != ERROR_SUCCESS)
        {
            DCLPRINTF(1, ("\nDclWinSettingsSave() failed to set the %W setting, Error=%ld.\n", pwzName, lError));
            dclStat = DCLSTAT_WIN_REGSETVALUEFAILED;            
            goto Cleanup;
        }
    }

    dclStat = DCLSTAT_SUCCESS;

  Cleanup:

    RegCloseKey(hKey);

    return dclStat;
}


/*-------------------------------------------------------------------
    Public: DclWinSettingsEdit()

    Pop up a dialog box that allows settings to be edited.  Note that
    these settings are a subset of the settings included in the
    DCLWINSETTINGS structure.  A number of those settings are edited
    elsewhere.

    Parameters:
        hInstance  - The application instance to use.
        hWndParent - The parent window handle.
        pSettings  - A pointer to the DCLWINSETTINGS structure to use.

    Return Value:
        Returns the standard result from DialogBoxParam().
-------------------------------------------------------------------*/
int DclWinSettingsEdit(
    HINSTANCE           hInstance,
    HWND                hWndParent,
    DCLWINSETTINGS     *pSettings)
{
    int                 iResult;

    DclAssert(hInstance);
    DclAssert(hWndParent);
    DclAssertWritePtr(pSettings, sizeof(*pSettings));

    iResult = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SETTINGSDLG), hWndParent, DialogProc, (LPARAM)pSettings);
    if(iResult == -1)
    {
        D_WCHAR wcsErrorMsg[100] = {0};

        wsprintf(wcsErrorMsg, TEXT("Could not create the settings dialog box.  Error=0x%08x"), GetLastError());

        MessageBox(NULL, wcsErrorMsg, TEXT("Error"), MB_OK);
    }

    return iResult;
}


/*-------------------------------------------------------------------
    Public: DclWinSettingsDelete()

    Delete a program's settings from the system registry.  This API
    only deletes the "standard" settings managed in this interface.
    Other custom app settings, and the app key itself are not deleted.

    Settings are deleted from HKLM\Software\Datalight, Inc.\pszAppName.

    Parameters:
        pwzAppName  - A pointer to the null-terminated application
                      name.
    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclWinSettingsDelete(
    const D_WCHAR          *pwzAppName)
{
    LONG                    lError;
    HKEY                    hKey = 0;
    D_WCHAR                 wzKey[MAX_KEY_LEN];
    unsigned                nn;

    DclAssertReadPtr(pwzAppName, 0);

    wcscpy(wzKey, wzPath);
    wcscat(wzKey, pwzAppName);

    lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wzKey, 0, KEY_ALL_ACCESS, &hKey);
    if(lError != ERROR_SUCCESS)
    {
        DCLPRINTF(1, ("DclWinSettingsDelete() failed to open the key, \"%W\", Error=%ld\n", wzKey, lError));

        return DCLSTAT_WIN_REGKEYNOTFOUND;
    }

    for(nn=0; nn<DCLDIMENSIONOF(aSettings); nn++)
    {
        lError = RegDeleteValue(hKey, aSettings[nn].pwzSettingName);
        if(lError != ERROR_SUCCESS)
        {
            DCLPRINTF(1, ("DclWinSettingsDelete() failed to delete the key, \"%W\", Error=%ld\n", aSettings[nn].pwzSettingName, lError));
        }
    }

    RegCloseKey(hKey);

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: DialogProcSettings()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static BOOL CALLBACK DialogProc(
    HWND            hWndDlg,
    UINT            nMsg,
    WPARAM          wParam,
    LPARAM          lParam)
{
    BOOL            fResult = FALSE;

    DCLWINMESSAGETRACEPRINT(MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, TRACEINDENT),
        "WinSettings:DialogProc(%s)\n", hWndDlg, nMsg, wParam, lParam);

    switch(nMsg)
    {
        case WM_INITDIALOG:
        {
            DCLWINSETTINGS  *pSettings = (DCLWINSETTINGS*)lParam;

            DclWinDialogSetPosition(hWndDlg, 0);

            SetLastError(0);
            if(!SetWindowLong(hWndDlg, GWL_USERDATA, lParam) && GetLastError())
            {
                DclError();
            }

            /*  Initialize the checkbox to match the current setting
            */
            if(pSettings->fEscKeyToExit)
                CheckDlgButton(hWndDlg, IDC_SETTINGS_ESCKEYTOEXIT, BST_CHECKED);

            /*  Return success (TRUE).
            */
            fResult = TRUE;
        }
        break;

        case WM_CLOSE:
            /*  Nothing to do but end the dialog box.
            */
            EndDialog(hWndDlg, 0);

            /*  Return success (TRUE).
            */
            fResult = TRUE;
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                {
                    DCLWINSETTINGS  *pSettings;

                    pSettings = (DCLWINSETTINGS*)GetWindowLong(hWndDlg, GWL_USERDATA);
                    DclAssert(pSettings);

                    if(IsDlgButtonChecked(hWndDlg, IDC_SETTINGS_ESCKEYTOEXIT) == BST_CHECKED)
                        pSettings->fEscKeyToExit = TRUE;
                    else
                        pSettings->fEscKeyToExit = FALSE;

                    EndDialog(hWndDlg, 1);
                    break;
                }

                case IDCANCEL:
                    EndDialog(hWndDlg, 0);
                    break;
            }

            fResult = TRUE;
            break;
    }

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINGUI, 1, TRACEUNDENT),
        "WinSettings:DialogProc() %s returning %u\n", DclWinMessageName(nMsg), fResult));

    return fResult;
}


/*-------------------------------------------------------------------
    Local: CopySetting()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS CopySetting(
    const SETTINGINFO  *pSetting,
    void               *pDest,
    const void         *pSource)
{
    size_t              nLen;

    DclAssertReadPtr(pSetting, sizeof(*pSetting));
    DclAssertWritePtr(pDest, 0);
    DclAssertReadPtr(pSource, 0);

    switch(pSetting->nType)
    {
        case TYP_BYTE:
            DclAssert(pSetting->nSize == sizeof(BYTE));
            nLen = sizeof(BYTE);
            break;

        case TYP_BOOL:
            DclAssert(pSetting->nSize == sizeof(BOOL));
            nLen = sizeof(BOOL);
            break;

        case TYP_LONG:
            DclAssert(pSetting->nSize == sizeof(LONG));
            nLen = sizeof(LONG);
            break;

        case TYP_WSTR:
            nLen = pSetting->nSize * sizeof(D_WCHAR);
            break;

        default:
            DCLPRINTF(1, ("WinSettings:CopySetting() invalid type (%u) for field %W\n",
                pSetting->nType, pSetting->pwzSettingName));

            DclProductionError();

            return DCLSTAT_WIN_REGINVALIDTYPE;
    }

    DclMemCpy(pDest, pSource, nLen);

    return DCLSTAT_SUCCESS;
}
