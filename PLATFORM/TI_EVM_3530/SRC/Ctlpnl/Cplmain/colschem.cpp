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
/**
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Abstract: Code for the Color-Schemes tab of the Display CPL
**/

#include "cplpch.h"
#include <commdlg.h>
#include "colschem.h"

// The current state of the preview is stored in g_ppvw.
PPREVIEW g_ppvw;
PSCHEMEDIALOG g_pschemedlg;

const TCHAR c_szPreview[] = TEXT("MSPreview"); // Preview controls class name

// at least these are const
LPCTSTR g_szNormal;
LPCTSTR g_szDisabled;
LPCTSTR g_szSelected;
LPCTSTR g_szButton;
LPCTSTR g_szInactiveWindow;
LPCTSTR g_szActiveWindow;
LPCTSTR g_szWindowText;
LPCTSTR g_szMessageText;
LPCTSTR g_rgpszElementName[COLOR_MAX];

const int g_i3DShadowAdj      = -333;
const int g_i3DHilightAdj     = 500;
const BOOL g_fScale3DShadowAdj  = TRUE;
const BOOL g_fScale3DHilightAdj = TRUE;

BOOL CALLBACK SaveSchemeDlgProc(HWND, UINT, WPARAM, LPARAM);

// Initialize Scheme Page
BOOL SchemePage_Init(HINSTANCE hinst)
{
    int i;

    g_szNormal = CELOADSZ(IDS_NORMAL);
    g_szDisabled = CELOADSZ(IDS_DISABLED);
    g_szSelected = CELOADSZ(IDS_SELECTED);
    g_szButton = CELOADSZ(IDS_BUTTON);
    g_szActiveWindow = CELOADSZ(IDS_ACTIVEWINDOW);
    g_szInactiveWindow = CELOADSZ(IDS_INACTIVEWINDOW);
    g_szWindowText = CELOADSZ(IDS_WINDOWTEXT);
    g_szMessageText = CELOADSZ(IDS_MESSAGETEXT);

    // Load up the element description strings.
    for (i = 0; i < IDS_CLR_LAST - IDS_CLR_FIRST + 1; i++)
        g_rgpszElementName[i] = CELOADSZ(IDS_CLR_FIRST + i);

    return Preview_RegisterClass(hinst);
}

// Uninitialize Scheme Page
void SchemePage_Terminate(HINSTANCE hinst)
{
    UnregisterClass(c_szPreview, hinst);
}

// Scheme Page dialogs DialogProc.
extern "C" BOOL CALLBACK ColSchemeDlgProc(HWND hwndDlg,  // handle to dialog box
                               UINT uMsg,     // message  
                               WPARAM wParam, // first message parameter
                               LPARAM lParam  // second message parameter
                               )
{
    if (uMsg != WM_INITDIALOG)
    {
        //ASSERT(g_pschemedlg);
        // In case the dialog fails and sends a WM_DESTROY before WM_INITDIALOG
        if (!g_pschemedlg)
            return FALSE;
    }

    switch (uMsg)
    {
    case WM_INITDIALOG:
        SchemeDialog_Init(hwndDlg);
        AygInitDialog( hwndDlg, SHIDIF_SIPDOWN );
        return 1; // let GWE set focus

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) 
        {
        case IDOK:
            SchemeDialog_ApplyScheme(hwndDlg);
            // FALL THROUGH
        case IDCANCEL:
             SchemeDialog_Terminate(hwndDlg);
             return TRUE; // return TRUE or the dialog won't close

        case IDC_APPLY:
            SchemeDialog_ApplyScheme(hwndDlg);
            return TRUE;

        case IDC_COLOR:
            SchemeDialog_PickColor(hwndDlg);
            return TRUE;

        case IDC_SAVEAS:
            SchemeDialog_SaveScheme(hwndDlg);
            return TRUE;

        case IDC_DELETE:
            SchemeDialog_DeleteScheme(hwndDlg);
            return TRUE;

        case IDC_SCHEME:
            switch (GET_WM_COMMAND_CMD(wParam, lParam))
            {
                case CBN_CLOSEUP:
                    UpdateWindow(hwndDlg);
                    SchemeDialog_ChangeScheme(hwndDlg, GET_WM_COMMAND_HWND(wParam, lParam));
                return TRUE;
            }
            break;

        case IDC_ITEM:
            switch (GET_WM_COMMAND_CMD(wParam, lParam))
            {
            case CBN_SELCHANGE:
                // Force a repaint of the color button.
                InvalidateRect(GetDlgItem(hwndDlg, IDC_COLOR), NULL, FALSE);
                return TRUE;
            }
            break;
        } // WM_COMMAND
        break;

    case WM_DRAWITEM:
        SchemeDialog_DrawColorBox(hwndDlg, (DRAWITEMSTRUCT *) lParam);
        break;
        
    case WM_CLOSE:
        SchemeDialog_Terminate(hwndDlg);
        return FALSE;
    }
    return FALSE;
}

// Initialize scheme dialog
void SchemeDialog_Init(HWND hwndDlg)
{
    PSCHEMEDIALOG pschemedlg;

    // Allocate structure to hold scheme name and corresponding colors
    // Must alloc zero-inited. 
    g_pschemedlg = pschemedlg = (PSCHEMEDIALOG) LocalAlloc(LPTR, sizeof(SCHEMEDIALOG));
    SetWindowLong(hwndDlg, DWL_USER, (LONG) pschemedlg);
    if (!pschemedlg)
       return;

    // load our globals & register the Preview class
    if(!SchemePage_Init(g_hInst)) {
        DEBUGCHK(FALSE);
        return;
    }

    // size the preview window to same size/location as IDC_PREVIEWDUMMY
    RECT rc;
    GetWindowRect(GetDlgItem(hwndDlg, IDC_PREVIEWDUMMY), &rc);
    MapWindowRect(NULL, hwndDlg, &rc);
        
    // create the preview window
    if(!(pschemedlg->hwndPreview = CreateWindowEx(0, c_szPreview, NULL, WS_VISIBLE|WS_CHILD|WS_BORDER, 
        rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top, hwndDlg, (HMENU)(IDC_PREVIEW), g_hInst, NULL))) {
        DEBUGCHK(FALSE);
        return;
    }
    DEBUGCHK(pschemedlg->hwndPreview == GetDlgItem(hwndDlg, IDC_PREVIEW));

    // ComboBox_SetExtendedUI(GetDlgItem(hwndDlg, IDC_SCHEME), TRUE);
    // ComboBox_SetExtendedUI(GetDlgItem(hwndDlg, IDC_ITEM), TRUE);

    // Fill combobox with schemes in the registry. Get current scheme name from
    // registry. Scheme_Init allocates space for current scheme name.
    if (!Scheme_Init(hwndDlg, pschemedlg->szCurrentScheme))
    {
        SchemeDialog_Terminate(hwndDlg);
        return;
    }

    // If there is no current scheme, use current sys colors.
    if (ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_SCHEME)) == CB_ERR)
    {
        int n;
        for (n = 0; n < COLOR_MAX; n++)
            pschemedlg->argb[n] = GetSysColor(n | SYS_COLOR_INDEX_FLAG);  
        SendDlgItemMessage(hwndDlg, IDC_PREVIEW, PCM_SETCOLORS,
                           (WPARAM) COLOR_MAX, (LPARAM) pschemedlg->argb);
        SendDlgItemMessage(hwndDlg, IDC_PREVIEW, PCM_DRAWBITMAP, 0, 0);
    }
    else 
    {
        // Change to current scheme
        SchemeDialog_ChangeScheme(hwndDlg, GetDlgItem(hwndDlg, IDC_SCHEME));
    }
}

// Terminate scheme dialog
void SchemeDialog_Terminate(HWND hwndDlg)
{
    //ASSERT(g_pschemedlg);
    if(g_pschemedlg)
    {
        SetWindowLong(hwndDlg, DWL_USER, 0);
        LocalFree(g_pschemedlg);
        g_pschemedlg = 0; // dont leave dangling global ptr
    }
    // zero our globals & un-register the Preview class
    SchemePage_Terminate(g_hInst);
}

// Process users change of scheme.
void SchemeDialog_ChangeScheme(HWND hwndDlg, HWND hwndSchemes)
{
    int nIndex;

    // Get current scheme name
    nIndex = ComboBox_GetCurSel(hwndSchemes);
    if (nIndex == CB_ERR || nIndex == -1)
        return;
    ComboBox_GetLBText(hwndSchemes, nIndex, g_pschemedlg->szCurrentScheme); 

    // Get colors corresponding to current scheme
    if (!Scheme_ReadScheme(g_pschemedlg->szCurrentScheme, g_pschemedlg->argb))
    {
        int n;
        // In case of failure, default to current colors
        g_pschemedlg->szCurrentScheme[0] = TEXT('\0');
        for (n = 0; n < COLOR_MAX; n++)
            g_pschemedlg->argb[n] = GetSysColor(n | SYS_COLOR_INDEX_FLAG);
        ComboBox_SetCurSel(hwndSchemes, -1);
        // REVIEW: Should we notify the user that the selected scheme
        // is invalid and that we defaulted to the current system colors?
    }

    // Set preview control colors using this scheme and ask preview control
    // to re-draw its bitmap.
    SendDlgItemMessage(hwndDlg, IDC_PREVIEW, PCM_SETCOLORS,
                       (WPARAM) COLOR_MAX, (LPARAM) g_pschemedlg->argb);
    SendDlgItemMessage(hwndDlg, IDC_PREVIEW, PCM_DRAWBITMAP, 0, 0);
}

// Apply current color scheme to the system
void SchemeDialog_ApplyScheme(HWND hwndDlg)
{
    int n, nColors[COLOR_MAX];

    // Set system colors using current scheme
    for (n=0; n<COLOR_MAX; n++)
        nColors[n] = n | SYS_COLOR_INDEX_FLAG;
    SetSysColors(COLOR_MAX, nColors, g_pschemedlg->argb);

    // Save current scheme name in the registry
    if (ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_SCHEME)) != CB_ERR)
        Scheme_WriteCurrentScheme(g_pschemedlg->szCurrentScheme);
    else
        Scheme_WriteCurrentScheme(L"");
}

void SchemeDialog_PickColor(HWND hwndDlg)
{
    int nSysColor;

    if (g_pschemedlg->cc.lStructSize == 0)
    {
        g_pschemedlg->cc.lStructSize = sizeof(g_pschemedlg->cc);
        g_pschemedlg->cc.hwndOwner = hwndDlg;
        g_pschemedlg->cc.lpCustColors = g_pschemedlg->g_rgCustColors;
        g_pschemedlg->cc.Flags = CC_RGBINIT;
    }
    g_pschemedlg->cc.rgbResult = CurrentItemColor(hwndDlg, &nSysColor);
            
    if (ChooseColor(&g_pschemedlg->cc))
    {
        // The user picked a new color
        // Clear the current scheme setting
        ComboBox_SetCurSel(GetDlgItem(hwndDlg, IDC_SCHEME), -1);
        // g_pschemedlg->szCurrentScheme[0] = L'\0';

        // Stuff the new color into the current settings
        // Change for 3D Object change, because it triggers others.
        if (nSysColor == COLOR_3DFACE)
            Update3DColors(g_pschemedlg->cc.rgbResult);
        else 
            COLOR(nSysColor) = g_pschemedlg->cc.rgbResult;

        // Force a redraw of the color box
        InvalidateRect(GetDlgItem(hwndDlg, IDC_COLOR), NULL, FALSE);
        // Force an update of the preview
        SendDlgItemMessage(hwndDlg, IDC_PREVIEW, PCM_DRAWBITMAP, 0, 0);
    }
}

void RemoveBlanks(LPTSTR lpszString)
{
    LPTSTR lpszPosn;

    /* strip leading blanks */
    lpszPosn = lpszString;
    while(*lpszPosn == TEXT(' ')) {
        lpszPosn++;
    }
    if (lpszPosn != lpszString)
        lstrcpy(lpszString, lpszPosn);

    /* strip trailing blanks */
    if ((lpszPosn=lpszString+lstrlen(lpszString)) != lpszString) {
        lpszPosn = CharPrev(lpszString, lpszPosn);
        while(*lpszPosn == TEXT(' '))
           lpszPosn = CharPrev(lpszString, lpszPosn);
        lpszPosn = CharNext(lpszPosn);
        *lpszPosn = TEXT('\0');
    }

}

BOOL CALLBACK SaveSchemeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR szBuf[100];

    switch (message)
    {

#if 0  //  removed the help button from this dialog 
        case WM_HELP:
            MyCreateProcess(TEXT("peghelp"), pszSaveColSchemeHelp /*ColSchemeTab.pszHelp*/);
            break;
#endif 
        case WM_INITDIALOG:
            LoadAygshellLibrary();
            SetDlgItemText(hDlg, IDC_SAVESCHEME,  g_pschemedlg->szCurrentScheme);
            SendDlgItemMessage(hDlg, IDC_SAVESCHEME, EM_SETSEL, 0, -1);
            SendDlgItemMessage(hDlg, IDC_SAVESCHEME, EM_LIMITTEXT, 32, 0L);
            EnableWindow(GetDlgItem(hDlg, IDOK), (g_pschemedlg->szCurrentScheme[0] != L'\0'));

            AygAddSipprefControl( hDlg );
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_SAVESCHEME:
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                        GetDlgItemText(hDlg, IDC_SAVESCHEME, szBuf, ARRAYSIZEOF(szBuf));
                        RemoveBlanks(szBuf);
                        EnableWindow(GetDlgItem(hDlg, IDOK), (*szBuf != L'\0'));
                    }
                    break;

                case IDOK:
                    GetDlgItemText(hDlg, IDC_SAVESCHEME, szBuf, ARRAYSIZEOF(szBuf));
                    RemoveBlanks(szBuf);

                    if (!(*szBuf))
                    {
                        // This case should never be hit because OK is not enabled
                        // unless the user has entered a non-empty string.
                        MessageBox(hDlg, CELOADSZ(IDS_BLANKNAME), NULL, MB_OK | MB_ICONEXCLAMATION);
                        return TRUE;
                    }
                    else if(!_wcsicmp(szBuf,CELOADSZ(IDS_BLESSEDSCHEME)))
                    {
                        MessageBox(hDlg, CELOADSZ(IDS_READONLYSCHEME), NULL, MB_OK | MB_ICONEXCLAMATION);
                        return TRUE;
                    }
                    lstrcpy(g_pschemedlg->szCurrentScheme, szBuf);
                    // FALL THRU

                case IDCANCEL:
                    EndDialog(hDlg, LOWORD(wParam));
                    return TRUE;
            }
            break;
        case WM_DESTROY:
            FreeAygshellLibrary();
            break;
    }
    return FALSE;
}

BOOL FindScheme(LPCWSTR szScheme, LPWSTR pszSubKeyName, UINT cchOutBuf,  CReg* pregScheme)
{
    WCHAR szSubKey[MAX_PATH];
    WCHAR szName[MAX_PATH];
    CReg reg, regTemp; 

    if(!pregScheme)
    {
        pregScheme = &regTemp;
    }

    reg.Create(HKEY_CURRENT_USER, RK_COLORSCHEMES);
    while(reg.EnumKey(szSubKey, MAX_PATH))
    {
        pregScheme->Open(HKEY(reg), szSubKey);
        if(pregScheme->ValueSZ(L"DisplayName", szName, MAX_PATH))
        {
            if(_wcsicmp(szScheme, szName) == 0)
            {
                if(pszSubKeyName)
                {
                    StringCchCopy(pszSubKeyName, cchOutBuf, szSubKey);
                }
                break;
            }
        }
        pregScheme->Reset();
    }

    return (BOOL)HKEY(*pregScheme);
}


void SchemeDialog_SaveScheme(HWND hwndDlg)
{
    HWND hwndCombo;
    SCHEMEDATA sd;
    int i;

    if (DialogBox(g_hInst, MAKEINTRESOURCE(IDD_COLSCHEMESAVE), hwndDlg, SaveSchemeDlgProc) == IDOK)
    {
        CReg regTemp; 

        // Check if it is already saved in the registry
        if(!FindScheme(g_pschemedlg->szCurrentScheme, NULL, 0, &regTemp))
        {
            // Create key if not found
            CReg reg(HKEY_CURRENT_USER, RK_COLORSCHEMES);
            regTemp.Create(HKEY(reg), g_pschemedlg->szCurrentScheme);
            regTemp.SetSZ(L"DisplayName", g_pschemedlg->szCurrentScheme);
        }
        
        sd.nVersion = SCHEME_VERSION_WINCE;
        sd.wDummy = 0;
        for (i = 0; i < COLOR_MAX; i++)
        {
            sd.argb[i] = g_pschemedlg->argb[i];
        }
        if(regTemp.SetBinary(L"Settings", (LPBYTE) &sd, sizeof(sd)))
        {
            // The save was successful, so add if necessary and select the new scheme.
            hwndCombo = GetDlgItem(hwndDlg, IDC_SCHEME);
            i = ComboBox_FindStringExact(hwndCombo, 0, g_pschemedlg->szCurrentScheme);
            if (i == CB_ERR) // we need to add the new scheme
                i = ComboBox_AddString(hwndCombo, g_pschemedlg->szCurrentScheme);
            ComboBox_SetCurSel(hwndCombo, i);
        }
    }
}

void SchemeDialog_DeleteScheme(HWND hwndDlg)
{
    HWND hwndCombo = GetDlgItem(hwndDlg, IDC_SCHEME);
    int i = ComboBox_GetCurSel(hwndCombo);
    LPWSTR  szDefault;
    TCHAR szBuf[100];

    if (i != CB_ERR)
    {
        // Get the name of the currently selected scheme
        ComboBox_GetLBText(hwndCombo, i, szBuf);
        szDefault = (LPWSTR)CELOADSZ(IDS_BLESSEDSCHEME);
        //Prefix
        if(szDefault && !wcscmp(szBuf,szDefault))
        {
            MessageBox(hwndDlg, CELOADSZ(IDS_WINDOWSDEFAULT), NULL, MB_OK | MB_ICONEXCLAMATION);
            return;
        }
        // Delete the current selected item from the list
        ComboBox_DeleteString(hwndCombo, i);

        // find the default scheme
        i = ComboBox_FindString(hwndCombo, -1, szDefault);
        ComboBox_SetCurSel(hwndCombo, (i==CB_ERR) ? 0 : i);

        // Zap the scheme from the registry
        WCHAR szSubKeyName[MAX_PATH];
        if(FindScheme(szBuf, szSubKeyName, MAX_PATH, NULL))
        {
            CReg reg(HKEY_CURRENT_USER, RK_COLORSCHEMES);
            reg.DeleteKey(szSubKeyName);
        }
    }
    else // Nothing is selected right now
    {
        MessageBox(hwndDlg, CELOADSZ(IDS_NOSCHEME2DEL), NULL, MB_OK | MB_ICONEXCLAMATION);
    }
}

void SchemeDialog_DrawColorBox(HWND hwndDlg, DRAWITEMSTRUCT *pdis)
{
    RECT *prc = &pdis->rcItem;
    HBRUSH hbr;

    // Draw the 3D Button border
    DrawEdge(pdis->hDC, prc,
             (pdis->itemState & ODS_SELECTED) ? EDGE_SUNKEN : EDGE_RAISED,
             BF_RECT | BF_SOFT);
        
    // Fill the rect with the color of the current item.
    InflateRect(prc, -2, -2);
    hbr = CreateSolidBrush(CurrentItemColor(hwndDlg, NULL));
    //Prefix
    if( hbr ) {
        FillRect(pdis->hDC, prc, hbr);
        DeleteObject(hbr);
    }

    // Draw a focus rectangle if we have the focus
    if (pdis->itemState & ODS_FOCUS) 
    {
        InflateRect(prc, -1, -1);
        DrawFocusRect(pdis->hDC, prc);
    }
}

// Return the color of the currently selected item.
COLORREF CurrentItemColor(HWND hwndDlg, int *pnSysColor)
{
    HWND hwndItem = GetDlgItem(hwndDlg, IDC_ITEM);
    int nIndex = ComboBox_GetCurSel(hwndItem);
    int nSysColor = ComboBox_GetItemData(hwndItem, nIndex);
    COLORREF cr = COLOR(nSysColor);
    if (pnSysColor != NULL)
        *pnSysColor = nSysColor;
    return cr;
}

// Compute the colors for other 3d components based on object
// Basically taken from lookdlg.c in IE4 Desktop control panel.
void Update3DColors(COLORREF rgb)
{
    COLOR(COLOR_3DFACE) = rgb;

    // Compute the new shadow colors.
    COLOR(COLOR_3DLIGHT) = AdjustLuma(rgb, g_i3DHilightAdj, g_fScale3DHilightAdj);
    COLOR(COLOR_3DHIGHLIGHT) = AdjustLuma(COLOR(COLOR_3DLIGHT),
                                          g_i3DHilightAdj, g_fScale3DHilightAdj);
    COLOR(COLOR_3DSHADOW) = AdjustLuma(rgb, g_i3DShadowAdj, g_fScale3DShadowAdj);
    COLOR(COLOR_3DDKSHADOW) = AdjustLuma(COLOR(COLOR_3DSHADOW),
                                         g_i3DShadowAdj, g_fScale3DShadowAdj);
    RETAILMSG(1, (L"New Shadows = 0x%06X, 0x%06X, 0x%06X, 0x%06X\n",
                  COLOR(COLOR_3DHIGHLIGHT), COLOR(COLOR_3DLIGHT),
                  COLOR(COLOR_3DSHADOW), COLOR(COLOR_3DDKSHADOW)));

    // Update other colors that are tied to this.
    COLOR(COLOR_ACTIVEBORDER) = rgb;
    COLOR(COLOR_INACTIVEBORDER) = rgb;
    // COLOR(COLOR_MENU) = rgb;  

    COLOR(COLOR_SCROLLBAR) = COLOR(COLOR_3DHIGHLIGHT);
    // If the scrollbar is the same color as the window, adjust it.
    if ((COLOR(COLOR_SCROLLBAR) & 0x00FFFFFF) ==
        (COLOR(COLOR_WINDOW) & 0x00FFFFFF))
        COLOR(COLOR_SCROLLBAR) = RGB(192, 192, 192);

}


// Enumerate schemes in the registry and put the scheme names into
// the hwndSchemes combobox. Return the CurrentScheme name in 
// ppszCurrentScheme and select the CurrentScheme in the combobox.
// If there is no CurrentScheme, an empty string is returned and
// no scheme is selected in the combobox.
// Caller must free *ppszCurrentScheme if function succeeds.
void Scheme_ComboInit(HWND hwndCombo)
{
    TCHAR szScheme[MAX_PATH];

    SendMessage(hwndCombo, EM_LIMITTEXT, MAX_PATH-1, 0L);

     // Enumerate scheme names in the registry and add them to the combobox.
    CReg reg1(HKEY_CURRENT_USER, RK_COLORSCHEMES);
    while(reg1.EnumKey(szScheme, MAX_PATH))
    {
        CReg regTemp(HKEY(reg1), szScheme);
        if(regTemp.ValueSZ(L"DisplayName", szScheme, MAX_PATH))
        {
            ComboBox_AddString(hwndCombo, szScheme); 
        }
        regTemp.Reset();
    }
}

BOOL Scheme_Init(HWND hwndDlg, LPTSTR szCurrentScheme)
{
    int i, nIndex;
    HWND hwndCombo;
    LPCTSTR pszTemp;
    
    hwndCombo = GetDlgItem(hwndDlg, IDC_SCHEME);
    Scheme_ComboInit(hwndCombo);

    // Read the current scheme name from the registry and select it in hwndSchemes combobox.
    CReg reg2(HKEY_CURRENT_USER, RK_APPEARANCE);
    if(pszTemp = reg2.ValueSZ(RV_CURRENT))
    {
        if(CB_ERR != (nIndex = ComboBox_FindStringExact(hwndCombo, 0, pszTemp)))
            ComboBox_SetCurSel(hwndCombo, nIndex); 

        // return the current scheme name
        lstrcpy(szCurrentScheme, pszTemp);
    }
    else
    {
        szCurrentScheme[0] = 0;
    }
        
    // Turn on FullControl if this key exists.
    // Full control disables automatic selection of some colors.
    g_pschemedlg->g_fFullControl = reg2.ValueDW(RV_FULLCONTROL);

    // Add the list of elements to the item combo box
    hwndCombo = GetDlgItem(hwndDlg, IDC_ITEM);

    for (i = 0; i < IDS_CLR_LAST - IDS_CLR_FIRST + 1; i++)
    {
        // Don't add Items that start with a paren.
        // The user shouldn't have to worry about them.
        if (g_rgpszElementName[i][0] == L'(' && !g_pschemedlg->g_fFullControl)
            continue;
        nIndex = ComboBox_AddString(hwndCombo, g_rgpszElementName[i]);
        // Stuff the COLOR_ constant into the item data for this item.
        // This depends on COLOR_SCROLLBAR being the first color!
        if (nIndex != CB_ERR && nIndex != CB_ERRSPACE)
            ComboBox_SetItemData(hwndCombo, nIndex, i | SYS_COLOR_INDEX_FLAG);
        else
            goto error;
        // Set the initial selection to the entry for "Desktop"
        nIndex = ComboBox_FindStringExact(hwndCombo, 0,
                                  g_rgpszElementName[COLOR_BACKGROUND & ~SYS_COLOR_INDEX_FLAG]);
        if (nIndex == CB_ERR)
            nIndex = 0;
        ComboBox_SetCurSel(hwndCombo, nIndex); 
    }
    
    return TRUE;

error:
    return FALSE;
}

// Read colors for pszScheme scheme from the registry into argb.
BOOL Scheme_ReadScheme(LPCTSTR pszScheme, COLORREF argb[])
{
    SCHEMEDATA sd;
    int n, cColors;

    // Read scheme data
    CReg reg;
    if(!FindScheme(pszScheme, NULL, 0, &reg))
        return FALSE;
    
    DWORD dwSize = reg.ValueBinary(L"Settings", (LPBYTE)&sd, sizeof(sd));
    if(!dwSize || (sd.nVersion != SCHEME_VERSION_WINCE))
        return FALSE;
  
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
    return TRUE;
}

// Write pszScheme as the current scheme in the registry. If
// pszScheme is an empty string, the current scheme is deleted 
// in the registry.
void Scheme_WriteCurrentScheme(LPCTSTR pszScheme)
{
    CReg reg; reg.Create(HKEY_CURRENT_USER, RK_APPEARANCE);

    if (pszScheme[0] != TEXT('\0'))
        reg.SetSZ(RV_CURRENT, pszScheme);
    else
        reg.DeleteValue(RV_CURRENT);
}

// Register Preview Controls class
BOOL Preview_RegisterClass(HINSTANCE hinst)
{
    WNDCLASS wc;

    // Register Preview Controls class
    if (!GetClassInfo(hinst, c_szPreview, &wc)) 
    {
        wc.style = 0;
        wc.lpfnWndProc = PreviewWindowProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = sizeof(PPREVIEW);
        wc.hInstance = hinst;
        wc.hIcon = NULL;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = c_szPreview;

        if (!RegisterClass(&wc))
            return FALSE;
    }
    return TRUE;
}

// Preview controls WindowProc
LRESULT CALLBACK PreviewWindowProc(HWND hwnd,      // handle to window
                                   UINT uMsg,      // message identifier
                                   WPARAM wParam,  // first message parameter
                                   LPARAM lParam   // second message parameter
                                   )
{
    switch (uMsg)
    {
    case WM_CREATE:
        if (!Preview_Create(hwnd))
            return -1;
        break;

    case WM_DESTROY:
        Preview_Destroy(hwnd);
        break;

    case WM_PAINT:
        Preview_Paint(hwnd);
        break;

    case WM_LBUTTONDOWN:
        g_ppvw->pt.x = LOWORD(lParam);
        g_ppvw->pt.y = HIWORD(lParam);
        Preview_LButtonDown(hwnd, g_ppvw->pt);
        break;

    case PCM_SETCOLORS:
        g_ppvw->cColors = (short) wParam;
        g_ppvw->prgb = (COLORREF *) lParam;
        // Force a repaint of the color button.
        InvalidateRect(GetDlgItem(GetParent(hwnd), IDC_COLOR), NULL, FALSE);
        break;

    case PCM_DRAWBITMAP:
        Preview_DrawBitmap(hwnd);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

BOOL Preview_Create(HWND hwnd)
{
   RECT rc;
   HDC hdc;

   if (g_ppvw)
   {
       ASSERT(FALSE);
       RETAILMSG(1, (L"WARNING: PPREVIEW already exists!\n"));
       return TRUE;
   }
   // alloc inited to 0. Very important.
   g_ppvw = (PPREVIEW) LocalAlloc(LPTR, sizeof(PREVIEW));
   if (!g_ppvw)
       return FALSE;

    // init
    g_ppvw->nLastItem = -1;

   // For speedy painting create a bitmap and memory DC to create a pre-drawn preview.
   GetClientRect(hwnd, &rc);
   hdc = GetDC(hwnd);
   g_ppvw->hdcMem = CreateCompatibleDC(hdc);
   if (g_ppvw->hdcMem)
   {
       g_ppvw->hbm = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
   }
   // From this point g_ppvw->hbm is non-NULL only if g_ppvw->hdcMem is also non-NULL.
   // So g_ppvw->hbm can be used to check if both g_ppvw->hdcMem and g_ppvw->hbm are valid.
   ReleaseDC(hwnd, hdc);
 
   // Create necsessary fonts
   if (!Preview_CreateFonts(hwnd))
       goto error;

   // Create dither brush for drawing scroll track
   g_ppvw->hbrLightGrayDither = Preview_CreateLightGrayDitherBrush();

   // Calculate dimensions and positions of each of the preview elements
   Preview_CalcElementRects(hwnd);
   RETAILMSG(1, (L"Btn Rect1 = (%d, %d, %d, %d)\n",
                 RC(ELEMENT_BUTTON).top,
                 RC(ELEMENT_BUTTON).left,
                 RC(ELEMENT_BUTTON).bottom,
                 RC(ELEMENT_BUTTON).right));

   // Create image list and add images to be drawn on buttons and adornments
   g_ppvw->himl = ImageList_Create(IMAGEX, IMAGEY, ILC_MASK | ILC_COLORDDB, 5, 1);
   if (!g_ppvw->himl)
       goto error;
   ImageList_AddMasked(g_ppvw->himl, 
                       LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_SCHEME)),
                       CLR_DEFAULT);
   ImageList_AddMasked(g_ppvw->himl, 
                       LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CLOSE)),
                       CLR_DEFAULT);
   ImageList_AddMasked(g_ppvw->himl, 
                       LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_OK)),
                       CLR_DEFAULT);

   // Pre-draw the preview elements into the preview bitmap.
   if (g_ppvw->hbm)
       Preview_DrawBitmap(hwnd);
   return TRUE;

error:
   Preview_Destroy(hwnd);
   return FALSE;
}

void Preview_Destroy(HWND hwnd)
{
    int fonttype;

    if (!g_ppvw)
        return;

    RETAILMSG(1, (L"DeleteObject #1\n"));
    if (g_ppvw->hbm)
        DeleteObject(g_ppvw->hbm);
    RETAILMSG(1, (L"DeleteObject #2\n"));
    if (g_ppvw->hdcMem)
        DeleteDC(g_ppvw->hdcMem);

    // Delete fonts
    for (fonttype = FONT_FIRST; fonttype <= FONT_LAST; fonttype++)
    {
    RETAILMSG(1, (L"DeleteObject #F\n"));
        if (FONT(fonttype))
            DeleteObject(FONT(fonttype));
    }

    RETAILMSG(1, (L"DeleteObject #3\n"));
    if (g_ppvw->hbrLightGrayDither)
        DeleteObject(g_ppvw->hbrLightGrayDither);

    RETAILMSG(1, (L"ImageList_Destry #4\n"));
    if (g_ppvw->himl)
        ImageList_Destroy(g_ppvw->himl);

    LocalFree(g_ppvw);
    g_ppvw = 0;               // dont leave dangling global ptr!!
}

void Preview_Paint(HWND hwnd)
{
    PAINTSTRUCT ps;

    BeginPaint(hwnd, &ps);

    if (g_ppvw->hbm) // If the control has a pre-drawn Preview bitmap, Blt it. 
    {
        Preview_ShowBitmap(hwnd, ps.hdc);
    }
    else           // Otherwise draw the Preview contents. This is slower.
    {
        Preview_Draw(hwnd, ps.hdc);
    }
    EndPaint(hwnd, &ps);
}

void Preview_LButtonDown(HWND hwnd, POINT pt)
{
    int i, nIndex, nItem;
    LPCTSTR pszItem = L"";
    HWND hwndItem;

    RETAILMSG(1, (L"Click @ %d, %d\n", pt.x, pt.y));

    // Run through the element rects to determine the click location.
    for (i = 0; i <= ELEMENT_LAST; i++)
    {
        if (PtInRect(&RC(i), pt))
        {
            nItem = i;
            pszItem = g_rgpszElementName[SC(i) & ~SYS_COLOR_INDEX_FLAG];
            RETAILMSG(1, (L"  Inside %s (%d)\n", pszItem, nItem));
        }
    }

    // If the user clicks the same item twice, see if we have another choice
    // for him to pick from.  This effectively toggles values in the item box.
    if (nItem == g_ppvw->nLastItem)
        {
            switch (nItem) {
                
              case ELEMENT_MENUSELECTED:
                  nItem = -1;  // reset
                  pszItem = g_rgpszElementName[COLOR_HIGHLIGHTTEXT & ~SYS_COLOR_INDEX_FLAG];
                  break;

              case ELEMENT_BUTTON:
                  nItem = -1;  // reset
                  pszItem = g_rgpszElementName[COLOR_BTNTEXT & ~SYS_COLOR_INDEX_FLAG];
                  break;
              case ELEMENT_MENUNORMAL:
                  nItem = -1;  // reset
                  pszItem = g_rgpszElementName[COLOR_MENU & ~SYS_COLOR_INDEX_FLAG];
                  break;
            }
        }

    // Now that we've found the item, select it in the combo box.
    hwndItem = GetDlgItem(GetParent(hwnd), IDC_ITEM);
    nIndex = ComboBox_FindStringExact(hwndItem, 0, pszItem);
    RETAILMSG(1, (L"Found it at %d\n", nIndex));
    if (nIndex != CB_ERR)
    {
        ComboBox_SetCurSel(hwndItem, nIndex);
        InvalidateRect(GetDlgItem(GetParent(hwnd), IDC_COLOR), NULL, FALSE);
    }

    g_ppvw->nLastItem = nItem;
}

// BitBlt the pre-drawn Preview bitmap into hdc.
void Preview_ShowBitmap(HWND hwnd, HDC hdc)
{
    RECT rc;
    HBITMAP hbmOld;

    GetClientRect(hwnd, &rc);
    hbmOld = (HBITMAP)SelectObject(g_ppvw->hdcMem, g_ppvw->hbm);
    BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, g_ppvw->hdcMem, 0, 0, SRCCOPY);
    //Prefix
    if( hbmOld )
        SelectObject(g_ppvw->hdcMem, hbmOld);
}

// Draw the preview bitmap and draw the Preview control.
void Preview_DrawBitmap(HWND hwnd)
{
    HBITMAP hbmOld;

    if (g_ppvw->hbm)
    {
        // Draw preview bitmap
        hbmOld = (HBITMAP)SelectObject(g_ppvw->hdcMem, g_ppvw->hbm);
        Preview_Draw(hwnd, g_ppvw->hdcMem);
        //Prefix
        if( hbmOld )
            SelectObject(g_ppvw->hdcMem, hbmOld);
    }

    // Draw Preview control. This will Blt the preview bitmap.
    InvalidateRect(hwnd, NULL, FALSE);
}

// Draw Preview contents into hdc
void Preview_Draw(HWND hwnd, HDC hdc)
{
    HBRUSH hbr, hbr2, hbrOld;
    short cxBorder, cyBorder, cxEdge, cyEdge;
    int x, y, cxArrow, cyArrow;
    RECT rc;

    // If PCM_SETCOLORS has not been called
    if (g_ppvw->cColors < COLOR_MAX || !g_ppvw->prgb)
        return;

    // System Metrics
    cxBorder = GetSystemMetrics(SM_CXBORDER);
    cyBorder = GetSystemMetrics(SM_CYBORDER);
    cxEdge = GetSystemMetrics(SM_CXEDGE);
    cyEdge = GetSystemMetrics(SM_CYEDGE);
    RETAILMSG(1, (L"SystemMetrics cxB = %d, cyB = %d, cxE = %d, cyE = %d\n",
                  cxBorder, cyBorder, cxEdge, cyEdge));

    SaveDC(hdc);

    // Desktop
    hbr = CreateSolidBrush(COLOR(SC(ELEMENT_DESKTOP)));
    //Prefix
    if( hbr ) {
        FillRect(hdc, &RC(ELEMENT_DESKTOP), hbr);
        DeleteObject(hbr);
    }

    // App Window
    hbr = CreateSolidBrush(COLOR(SC(ELEMENT_APPWINDOW)));
    FillRect(hdc, &RC(ELEMENT_APPWINDOW), hbr);
    DeleteObject(hbr);
    // App Window outline
    if (COLOR(COLOR_WINDOW) == RGB(0,0,0))
        hbr = (HBRUSH) GetStockObject(WHITE_BRUSH);
    else
        hbr = (HBRUSH) GetStockObject(BLACK_BRUSH);
    rc = RC(ELEMENT_APPWINDOW);
    // Draw the rect outside the app window, because app windows do not have borders
    InflateRect(&rc, 1, 1);
    Preview_DrawBorder(hdc, &rc, cxBorder, cyBorder, hbr, hbr, PDB_LEFT | PDB_TOP);

    // App Bands
    hbr = CreateSolidBrush(COLOR(SC(ELEMENT_APPBANDS)));
    FillRect(hdc, &RC(ELEMENT_APPBANDS), hbr);
    DeleteObject(hbr);

    // Menu Band
    hbr = CreateSolidBrush(COLOR(SC(ELEMENT_MENU)));
    FillRect(hdc, &RC(ELEMENT_MENU), hbr);
    DeleteObject(hbr);

    // Menu Normal
    SelectObject(hdc, FONT(FONT_MENU));
    SetTextColor(hdc, COLOR(SC(ELEMENT_MENUNORMAL)));
    SetBkColor(hdc, COLOR(SC(ELEMENT_MENU)));
    DrawText(hdc, g_szNormal, lstrlen(g_szNormal),
             &RC(ELEMENT_MENUNORMAL),  DT_SINGLELINE | DT_CENTER | DT_VCENTER);

    // Menu Disabled
    SetTextColor(hdc, COLOR(SC(ELEMENT_MENUDISABLED)));
    DrawText(hdc, g_szDisabled, lstrlen(g_szDisabled), 
             &RC(ELEMENT_MENUDISABLED), DT_SINGLELINE | DT_CENTER | DT_VCENTER);

    // Menu Selected
    hbr = CreateSolidBrush(COLOR(SC(ELEMENT_MENUSELECTED)));
    FillRect(hdc, &RC(ELEMENT_MENUSELECTED), hbr);
    DeleteObject(hbr);

    SetTextColor(hdc, COLOR(COLOR_HIGHLIGHTTEXT));
    SetBkColor(hdc, COLOR(SC(ELEMENT_MENUSELECTED)));
    DrawText(hdc, g_szSelected, lstrlen(g_szSelected), 
             &RC(ELEMENT_MENUSELECTED), DT_SINGLELINE | DT_CENTER | DT_VCENTER);

    // Menu Outline
    if (COLOR(COLOR_WINDOW) == RGB(0,0,0))
        hbr = (HBRUSH) GetStockObject(WHITE_BRUSH);
    else
        hbr = (HBRUSH) GetStockObject(BLACK_BRUSH);
    Preview_DrawBorder(hdc, &RC(ELEMENT_MENU), cxBorder, cyBorder,
                       hbr, hbr, PDB_ALL);

    // CmdBar Left Side
    rc = RC(ELEMENT_APPBANDS);
    rc.left = RC(ELEMENT_APPBUTTONS).left + 1;
    hbr = (HBRUSH)GetStockObject(GRAY_BRUSH);
    Preview_DrawBorder(hdc, &rc, cxBorder, cyBorder,
                       hbr, hbr, PDB_LEFT);
    hbr = CreateSolidBrush(COLOR(COLOR_3DHIGHLIGHT));
    rc.left += 1;
    Preview_DrawBorder(hdc, &rc, cxBorder, cyBorder,
                       hbr, hbr, PDB_LEFT);
    DeleteObject(hbr);

    // Gripper
    hbr = CreateSolidBrush(COLOR(COLOR_3DDKSHADOW));
    hbr2 = CreateSolidBrush(COLOR(COLOR_3DSHADOW));
    rc.top += cyBorder;
    rc.bottom -= cyBorder;
    rc.left += cxBorder + 4;
    rc.right = rc.left + 7*cxBorder;
    Preview_DrawBorder(hdc, &rc, cxBorder, cyBorder, hbr, hbr, PDB_LEFT | PDB_TOP);
    DeleteObject(hbr);
    hbr = CreateSolidBrush(COLOR(COLOR_3DHILIGHT));
    rc.top += cyBorder;
    rc.left += cxBorder;
    rc.right = rc.left + 3*cxBorder;
    Preview_DrawBorder(hdc, &rc, cxBorder, cyBorder, hbr, hbr2, PDB_ALL);
    rc.left += 3*cxBorder;
    rc.right = rc.left + 3*cxBorder;
    Preview_DrawBorder(hdc, &rc, cxBorder, cyBorder, hbr, hbr2, PDB_ALL);
    DeleteObject(hbr);
    DeleteObject(hbr2);

    // App Command Buttons
    x = rc.right + 3;
    Preview_DrawImageButton(hdc, NEW, x, RC(ELEMENT_APPBUTTONS).top);

    x += TOOLBARBUTTONX;
    Preview_DrawImageButton(hdc, OPEN, x, RC(ELEMENT_APPBUTTONS).top);

    x += TOOLBARBUTTONX;
    Preview_DrawImageButton(hdc, SAVE, x, RC(ELEMENT_APPBUTTONS).top);


    // App Adornments
    // Erase background in case the app buttons intrude into the adornments space
    hbr = CreateSolidBrush(COLOR(SC(ELEMENT_APPADORNMENTS)));
    FillRect(hdc, &RC(ELEMENT_APPADORNMENTS), hbr);
    DeleteObject(hbr);

    x = RC(ELEMENT_APPADORNMENTS).left;
    Preview_DrawImageButton(hdc, HELP, x, RC(ELEMENT_APPADORNMENTS).top);

    x += (TOOLBARBUTTONX + SEPARATORX);
    Preview_DrawImageButton(hdc, CLOSE, x, RC(ELEMENT_APPADORNMENTS).top);

    // App Scroll Thumb and Scroll track
    // If dither brush is not available, use the light gray brush to draw scroll track
    // Just like the OS, only use the COLOR_SCROLLBAR if it is different from the default.
    if (COLOR(COLOR_SCROLLBAR) == RGB(224,224,224))
        hbr = (g_ppvw->hbrLightGrayDither) ?
            g_ppvw->hbrLightGrayDither : (HBRUSH) GetStockObject(LTGRAY_BRUSH);
    else
        hbr = CreateSolidBrush(COLOR(COLOR_SCROLLBAR));
    FillRect(hdc, &RC(ELEMENT_SCROLLBAR), hbr);
    if (hbr != g_ppvw->hbrLightGrayDither)
        DeleteObject(hbr);
    // Thumb
    Preview_DrawBlankButton(hdc, RC(ELEMENT_SCROLLUP).left, 
                            RC(ELEMENT_SCROLLUP).bottom + 2*cxEdge, 
                            RC(ELEMENT_SCROLLBAR).right - RC(ELEMENT_SCROLLBAR).left, 
                            (int) (1.5* (RC(ELEMENT_SCROLLUP).bottom - RC(ELEMENT_SCROLLUP).top)),
                            cxBorder, cyBorder);

    // Scroll button arrows
    hbr = CreateSolidBrush(COLOR(COLOR_BTNTEXT));
    hbrOld = (HBRUSH)SelectObject(hdc, hbr);

    // App Scroll Up button
    Preview_DrawBlankButton(hdc, RC(ELEMENT_SCROLLUP).left, 
                            RC(ELEMENT_SCROLLUP).top, 
                            RC(ELEMENT_SCROLLUP).right - RC(ELEMENT_SCROLLUP).left, 
                            RC(ELEMENT_SCROLLUP).bottom - RC(ELEMENT_SCROLLUP).top,
                            cxBorder, cyBorder);    
    // Obscure positioning code of the scroll arrow, borrowed from GWE
    rc = RC(ELEMENT_SCROLLUP);
    InflateRect(&rc, -1, -1);
    cyArrow = ((rc.bottom-rc.top) <= 6) 
                ? 2 
                : (((rc.bottom-rc.top) - 7) / 4) + 3;
    cxArrow = (cyArrow * 2) - 1;

    // Draw Up Arrow
    x = (((rc.right-rc.left)- cxArrow + 1) / 2) + rc.left;
    y = (((rc.bottom-rc.top) - cyArrow + 1) / 2) + rc.top;
    Preview_DrawArrow(hdc, x, y, cxArrow, -1);

    // App Scroll Down button
    Preview_DrawBlankButton(hdc, RC(ELEMENT_SCROLLDOWN).left, 
                            RC(ELEMENT_SCROLLDOWN).top, 
                            RC(ELEMENT_SCROLLDOWN).right - RC(ELEMENT_SCROLLDOWN).left, 
                            RC(ELEMENT_SCROLLDOWN).bottom - RC(ELEMENT_SCROLLDOWN).top,
                            cxBorder, cyBorder);
    // Draw down arrow
    rc = RC(ELEMENT_SCROLLDOWN);
    InflateRect(&rc, -1, -1);
    x = (((rc.right-rc.left)- cxArrow + 1) / 2) + rc.left;
    y = (((rc.bottom-rc.top) - cyArrow + 1) / 2) + rc.top;
    Preview_DrawArrow(hdc, x, y, cxArrow, +1);

    SelectObject(hdc, hbrOld);
    DeleteObject(hbr);

    // Inactive Window
    hbr = CreateSolidBrush(COLOR(SC(ELEMENT_INACTIVEWINDOW)));
    FillRect(hdc, &RC(ELEMENT_INACTIVEWINDOW), hbr);
    DeleteObject(hbr);

    // Inactive Window Border
    rc = RC(ELEMENT_INACTIVEWINDOW);
    if (COLOR(COLOR_WINDOW) == RGB(0,0,0))
        hbr = (HBRUSH) GetStockObject(WHITE_BRUSH);
    else
        hbr = (HBRUSH) GetStockObject(BLACK_BRUSH);
    Preview_DrawBorder(hdc, &rc, cxBorder, cyBorder,
                       hbr, hbr, PDB_LEFT | PDB_RIGHT | PDB_TOP);

    InflateRect(&rc, -cxBorder, -cyBorder);
    hbr = CreateSolidBrush(COLOR(COLOR_3DLIGHT));
    hbr2 = CreateSolidBrush(COLOR(COLOR_3DDKSHADOW));
    Preview_DrawBorder(hdc, &rc, cxBorder, cyBorder,
                       hbr, hbr2, PDB_LEFT | PDB_RIGHT | PDB_TOP);
    DeleteObject(hbr);
    DeleteObject(hbr2);
    InflateRect(&rc, -cxBorder, -cyBorder);
    hbr = CreateSolidBrush(COLOR(COLOR_3DHIGHLIGHT));
    hbr2 = CreateSolidBrush(COLOR(COLOR_3DSHADOW));
    Preview_DrawBorder(hdc, &rc, cxBorder, cyBorder,
                       hbr, hbr2, PDB_LEFT | PDB_RIGHT | PDB_TOP);
    DeleteObject(hbr);
    DeleteObject(hbr2);

    // Inactive Window Caption
    hbr = CreateSolidBrush(COLOR(SC(ELEMENT_INACTIVECAPTION)));
    FillRect(hdc, &RC(ELEMENT_INACTIVECAPTION), hbr);
    DeleteObject(hbr);

    // Inactive Window Caption Text
    SelectObject(hdc, FONT(FONT_SYSTEM_BOLD));
    SetTextColor(hdc, COLOR(SC(ELEMENT_INACTIVETITLE)));
    SetBkColor(hdc, COLOR(SC(ELEMENT_INACTIVECAPTION)));
    rc = RC(ELEMENT_INACTIVETITLE);
    rc.left += cxBorder; // Leave a bit of space before the text
    DrawText(hdc, g_szInactiveWindow, lstrlen(g_szInactiveWindow), 
             &rc, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

    // Inactive Window Adornments
    x = RC(ELEMENT_INACTIVEADORNMENTS).left;
    Preview_DrawImageButton(hdc, HELP, x, RC(ELEMENT_INACTIVEADORNMENTS).top);
    
    x += (BUTTONX + SEPARATORX);
    Preview_DrawImageButton(hdc, OK, x, RC(ELEMENT_INACTIVEADORNMENTS).top);

    x += (BUTTONX + SEPARATORX);
    Preview_DrawImageButton(hdc, CLOSE, x, RC(ELEMENT_INACTIVEADORNMENTS).top);

    // Window Text
    SelectObject(hdc, FONT(FONT_SYSTEM_BOLD));
    SetTextColor(hdc, COLOR(SC(ELEMENT_WINDOWTEXT)));
    SetBkColor(hdc, COLOR(SC(ELEMENT_INACTIVEWINDOW)));
    DrawText(hdc, g_szWindowText, lstrlen(g_szWindowText), 
             &RC(ELEMENT_WINDOWTEXT), 
             DT_SINGLELINE | DT_LEFT | DT_VCENTER);

    // Active Window*cxEdge
    hbr = CreateSolidBrush(COLOR(SC(ELEMENT_ACTIVEWINDOW)));
    FillRect(hdc, &RC(ELEMENT_ACTIVEWINDOW), hbr);
    DeleteObject(hbr);

    // Active Window Border
    rc = RC(ELEMENT_ACTIVEWINDOW);
    if (COLOR(COLOR_WINDOW) == RGB(0,0,0))
        hbr = (HBRUSH) GetStockObject(WHITE_BRUSH);
    else
        hbr = (HBRUSH) GetStockObject(BLACK_BRUSH);
    Preview_DrawBorder(hdc, &rc, cxBorder, cyBorder,
                       hbr, hbr, PDB_LEFT | PDB_RIGHT | PDB_TOP);

    InflateRect(&rc, -cxBorder, -cyBorder);
    hbr = CreateSolidBrush(COLOR(COLOR_3DLIGHT));
    hbr2 = CreateSolidBrush(COLOR(COLOR_3DDKSHADOW));
    Preview_DrawBorder(hdc, &rc, cxBorder, cyBorder,
                       hbr, hbr2, PDB_LEFT | PDB_RIGHT | PDB_TOP);
    DeleteObject(hbr);
    DeleteObject(hbr2);
    InflateRect(&rc, -cxBorder, -cyBorder);
    hbr = CreateSolidBrush(COLOR(COLOR_3DHIGHLIGHT));
    hbr2 = CreateSolidBrush(COLOR(COLOR_3DSHADOW));
    Preview_DrawBorder(hdc, &rc, cxBorder, cyBorder,
                       hbr, hbr2, PDB_LEFT | PDB_RIGHT | PDB_TOP);
    DeleteObject(hbr);
    DeleteObject(hbr2);

    // Active Window Caption
    hbr = CreateSolidBrush(COLOR(SC(ELEMENT_ACTIVECAPTION)));
    FillRect(hdc, &RC(ELEMENT_ACTIVECAPTION), hbr);
    DeleteObject(hbr);

    // Active Window Caption Text
    SelectObject(hdc, FONT(FONT_SYSTEM_BOLD));
    SetTextColor(hdc, COLOR(SC(ELEMENT_ACTIVETITLE)));
    SetBkColor(hdc, COLOR(SC(ELEMENT_ACTIVECAPTION)));
    rc = RC(ELEMENT_ACTIVETITLE);
    rc.left += cxBorder; // Leave a bit of space before the text
    DrawText(hdc, g_szActiveWindow, lstrlen(g_szActiveWindow), 
             &rc, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

    // Active Window Adornments
    x = RC(ELEMENT_ACTIVEADORNMENTS).left;
    Preview_DrawImageButton(hdc, HELP, x, RC(ELEMENT_ACTIVEADORNMENTS).top);
    
    x += (BUTTONX + SEPARATORX);
    Preview_DrawImageButton(hdc, OK, x, RC(ELEMENT_ACTIVEADORNMENTS).top);

    x += (BUTTONX + SEPARATORX);
    Preview_DrawImageButton(hdc, CLOSE, x, RC(ELEMENT_ACTIVEADORNMENTS).top);

    // Message Text
    SelectObject(hdc, FONT(FONT_MSGBOX));
    SetTextColor(hdc, COLOR(SC(ELEMENT_MESSAGETEXT)));
    SetBkColor(hdc, COLOR(SC(ELEMENT_ACTIVEWINDOW)));
    DrawText(hdc, g_szMessageText, lstrlen(g_szMessageText), 
             &RC(ELEMENT_MESSAGETEXT), DT_SINGLELINE | DT_LEFT | DT_VCENTER);

    // Button
    Preview_DrawBlankButton(hdc, RC(ELEMENT_BUTTON).left, 
        RC(ELEMENT_BUTTON).top, 
        RC(ELEMENT_BUTTON).right - RC(ELEMENT_BUTTON).left, 
        RC(ELEMENT_BUTTON).bottom - RC(ELEMENT_BUTTON).top,
        cxBorder, cyBorder);
    SelectObject(hdc, FONT(FONT_SYSTEM_BOLD));
    SetTextColor(hdc, COLOR(COLOR_BTNTEXT));
    SetBkColor(hdc, COLOR(SC(ELEMENT_BUTTON)));
    DrawText(hdc, g_szButton, lstrlen(g_szButton), 
             &RC(ELEMENT_BUTTON), 
             DT_SINGLELINE | DT_CENTER | DT_VCENTER);

    RestoreDC(hdc, -1);
}

void Preview_DrawImageButton(HDC hdc, int iImage, int x, int y)
{
    BOOL fInvert = (COLOR(COLOR_3DFACE) == RGB(0,0,0));

    if (g_ppvw->imldp.cbSize == 0)
    {
        g_ppvw->imldp.cbSize = sizeof(g_ppvw->imldp);
        g_ppvw->imldp.himl = g_ppvw->himl;
        g_ppvw->imldp.hdcDst = hdc;
        g_ppvw->imldp.cx = 0;
        g_ppvw->imldp.cy = 0;
        g_ppvw->imldp.xBitmap = 0;
        g_ppvw->imldp.yBitmap = 0;
        g_ppvw->imldp.rgbBk = CLR_NONE;
        g_ppvw->imldp.rgbFg = CLR_DEFAULT;
        g_ppvw->imldp.dwRop = SRCINVERT;
    }

    Preview_DrawBlankButton(hdc, x, y, TOOLBARBUTTONX, TOOLBARBUTTONY, 1, 1);
    g_ppvw->imldp.i = iImage;
    g_ppvw->imldp.x = x + TOOLBARIMAGE_OFFSETX;
    g_ppvw->imldp.y = y + TOOLBARIMAGE_OFFSETY;

    g_ppvw->imldp.fStyle = ILD_TRANSPARENT;
    ImageList_DrawIndirect(&g_ppvw->imldp);
    if (fInvert)
    {
        COLORREF bkColor = SetBkColor(hdc, RGB(0,0,0));
        COLORREF fgColor = SetTextColor(hdc, RGB(255,255,255));
        g_ppvw->imldp.fStyle = ILD_MASK | ILD_ROP;
        ImageList_DrawIndirect(&g_ppvw->imldp);
        SetBkColor(hdc, bkColor);
        SetTextColor(hdc, fgColor);
    }
}

// Draw a blank button of dimensions (x, y, cx, cy).
// (cxBorder, cyBorder) is the width of the raised outer border
// and the width of the raised inner border.
void Preview_DrawBlankButton(HDC hdc, 
                             int x, int y, int cx, int cy, 
                             short cxBorder, short cyBorder)
{
    RECT rc;
    HBRUSH hbr, hbrTL, hbrBR;

    // Draw raised outer border
    SetRect(&rc, x, y, x+cx, y+cy);
    hbrTL = (HBRUSH)GetStockObject(GRAY_BRUSH); 
    // hbrTL = CreateSolidBrush(COLOR(COLOR_3DHIGHLIGHT));
    hbrBR = CreateSolidBrush(COLOR(COLOR_3DDKSHADOW));
    //Prefix
    if( hbrBR ) {
        Preview_DrawBorder(hdc, &rc, cxBorder, cyBorder,
                       hbrTL, hbrBR, PDB_ALL);
        DeleteObject(hbrBR);
    }
    //
    // DeleteObject(hbrTL);

    // Draw raised inner border
    InflateRect(&rc, -cxBorder, -cyBorder);
    hbrTL = CreateSolidBrush(COLOR(COLOR_3DLIGHT));
    //Prefix
    if( hbrTL ) {
        hbrBR = CreateSolidBrush(COLOR(COLOR_3DSHADOW));     
        if( hbrBR ) {
            Preview_DrawBorder(hdc, &rc, cxBorder, cyBorder,
                       hbrTL, hbrBR, PDB_ALL);
        
        DeleteObject(hbrBR);
        }

        DeleteObject(hbrTL);
    }
    //
    // Fill button face
    InflateRect(&rc, -cxBorder, -cyBorder);
    hbr = CreateSolidBrush(COLOR(COLOR_3DFACE)); 
    FillRect(hdc, &rc, hbr);
    DeleteObject(hbr);
}

void Preview_DrawBorder(HDC hdc, LPRECT prc, 
                             short cxWidth, short cyWidth, 
                             HBRUSH hbrTL, HBRUSH hbrBR, UINT uFlags)
{
    HBRUSH hbrOld;
    RECT rc;

    rc = *prc;
    hbrOld = SelectBrush(hdc, hbrBR);
    if (uFlags & PDB_RIGHT)
    {
        rc.right -= cxWidth;
        PatBlt(hdc, rc.right, rc.top, cxWidth, rc.bottom - rc.top, PATCOPY);
    }
    if (uFlags & PDB_BOTTOM)
    {
        rc.bottom -= cyWidth;
        PatBlt(hdc, rc.left, rc.bottom, rc.right - rc.left, cyWidth, PATCOPY);
    }
    SelectBrush(hdc, hbrTL);
    if (uFlags & PDB_LEFT)
    {
        PatBlt(hdc, rc.left, rc.top, cxWidth, rc.bottom - rc.top, PATCOPY);
        rc.left += cxWidth;
    }
    if (uFlags & PDB_TOP)
    {
        PatBlt(hdc, rc.left, rc.top, rc.right - rc.left, cyWidth, PATCOPY);
        rc.top += cyWidth;
    }
    //Prefix
    if( hbrOld )
        SelectBrush(hdc, hbrOld);
}

// Draw scroll bar arrow at (x, y) of width cxArrow. nSign should be -1 for an
// up-arrow and +1 for a down-arrow
void Preview_DrawArrow(HDC hdc, int x, int y, int cxArrow, int nSign)
{
    int i;
    int nCount = (cxArrow + 1) / 2;
    int nInc = 2 * nSign;

    if (nSign == -1)
        cxArrow = (cxArrow % 2) ? 1 : 2;

    if (nSign == -1)
        x += nCount - 1;

    for (i = 0; i < nCount; i++, cxArrow -= nInc, x += nSign)
        PatBlt(hdc, x, y++, cxArrow, 1, PATCOPY);
}

// Create light-gray dither brush to draw scroll track. 
// Code borrowed from GWE
HBRUSH Preview_CreateLightGrayDitherBrush()
{
typedef struct 
{
    BITMAPINFOHEADER    bi;
    DWORD                colors[4];
    DWORD                biBits[8];
} DIB;
    
    DIB        dib;

    dib.bi.biSize = sizeof(BITMAPINFOHEADER);
    dib.bi.biHeight = 8;
    dib.bi.biWidth = 8;
    dib.bi.biPlanes = 1;
    dib.bi.biBitCount = 2;
    dib.bi.biCompression = BI_RGB;
    dib.bi.biSizeImage = 0;
    dib.bi.biXPelsPerMeter = 0;
    dib.bi.biYPelsPerMeter = 0;
    dib.bi.biClrUsed = 0;
    dib.bi.biClrUsed = 0;
    dib.bi.biClrImportant = 0;
    dib.colors[0] = 0x00000000;            // black
    dib.colors[1] = 0x00808080;         // dk gray
    dib.colors[2] = 0x00C0C0C0;         // light gray
    dib.colors[3] = 0x00FFFFFF;            // white
    
    dib.biBits[0] = 0xEEEEEEEE;
    dib.biBits[1] = 0xBBBBBBBB;
    dib.biBits[2] = 0xEEEEEEEE;
    dib.biBits[3] = 0xBBBBBBBB;
    dib.biBits[4] = 0xEEEEEEEE;
    dib.biBits[5] = 0xBBBBBBBB;
    dib.biBits[6] = 0xEEEEEEEE;
    dib.biBits[7] = 0xBBBBBBBB;

    return CreateDIBPatternBrushPt(&dib, DIB_RGB_COLORS);    
}

// Fill in a LOGFONT structure with information from the registry, or with
//   default info if there is nothing in the registry.
// Code borrowed from GWE
void
GdiGetLogFontFromRegistry (
    LOGFONT *plf,
    LPTSTR szRegKey,
    LONG lDefaultWeight
    )
{
    HKEY hkey;
    DWORD cbSize, dwItalic, dwCharSet;

    // Set up LOGFONT from values in registry
    // If values are not in registry, use these defaults
    plf->lfHeight = 13;
    plf->lfWidth = 0;
    plf->lfEscapement = 0;
    plf->lfOrientation = 0;
    plf->lfWeight = lDefaultWeight;
    plf->lfItalic = FALSE;
    plf->lfUnderline = FALSE;
    plf->lfStrikeOut = FALSE;
    plf->lfCharSet = ANSI_CHARSET;
    plf->lfOutPrecision = OUT_DEFAULT_PRECIS;
    plf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
    plf->lfQuality = DEFAULT_QUALITY;
    plf->lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
    _tcscpy (plf->lfFaceName, TEXT("MS Sans Serif"));

    // Get system font info from registry, if there.
    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, szRegKey,
                      /*reserved*/0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
        cbSize = sizeof(DWORD);

        RegQueryValueEx (hkey, TEXT("Ht"), 0, NULL, (LPBYTE)&plf->lfHeight, &cbSize);
        RegQueryValueEx (hkey, TEXT("Wt"), 0, NULL, (LPBYTE)&plf->lfWeight, &cbSize);

        if (RegQueryValueEx (hkey, TEXT("CS"), 0, NULL, (LPBYTE)&dwCharSet, &cbSize) == ERROR_SUCCESS)
        {
            plf->lfCharSet = (BYTE) dwCharSet;
        }

        if (RegQueryValueEx (hkey, TEXT("It"), 0, NULL, (LPBYTE)&dwItalic, &cbSize) == ERROR_SUCCESS)
        {
            plf->lfItalic = (BYTE) dwItalic;
        }

        cbSize = LF_FACESIZE * sizeof (TCHAR);
        RegQueryValueEx (hkey, TEXT("Nm"), 0, NULL, (LPBYTE)plf->lfFaceName, &cbSize);

        RegCloseKey (hkey);
    }
}

// Create fonts necessary for preview window. If this function succeeds, all fonts
// have been created and the caller must free those fonts. If this function fails,
// one or more fonts could not be created and the caller is responsible for freeing 
// the fonts that were created.
BOOL Preview_CreateFonts(HWND hwnd)
{
   // Menu Font
   GdiGetLogFontFromRegistry(&LF(FONT_MENU), TEXT("SYSTEM\\GWE\\Menu\\BarFnt"), FW_BOLD);
   FONT(FONT_MENU) = CreateFontIndirect(&LF(FONT_MENU));
   if (!FONT(FONT_MENU))
       return FALSE;

   // Bold System Font
   GetObject((HFONT)GetStockObject(SYSTEM_FONT), sizeof(LF(FONT_SYSTEM_BOLD)), &LF(FONT_SYSTEM_BOLD));
   LF(FONT_SYSTEM_BOLD).lfWeight = FW_BOLD;
   FONT(FONT_SYSTEM_BOLD) = CreateFontIndirect(&LF(FONT_SYSTEM_BOLD));
   if (!FONT(FONT_SYSTEM_BOLD))
       return FALSE;

   // MessageBox Font
   GdiGetLogFontFromRegistry(&LF(FONT_MSGBOX), TEXT("SYSTEM\\GWE\\OOMFnt"), FW_NORMAL);
   FONT(FONT_MSGBOX) = CreateFontIndirect(&LF(FONT_MSGBOX));
   if (!FONT(FONT_MSGBOX))
       return FALSE;

   return TRUE;
}

// Calculate the rects for each element in the preview
void Preview_CalcElementRects(HWND hwnd)
{
    RECT rc;
    short cxBorder, cyBorder, cyCaption, cxAvg2Char;
    short cxEdge, cyEdge, cx3Adornments;
    HDC hdc;
    SIZE sz;
    HFONT hfontOld;
    TEXTMETRIC tm;

    RETAILMSG(1, (L"Preview_CalcElementRects CALLED\r\n"));

    GetClientRect(hwnd, &rc);
    hdc = GetDC(NULL);

    cyCaption = GetSystemMetrics(SM_CYCAPTION);
    // Get window border widths
    cxBorder = GetSystemMetrics(SM_CXBORDER);
    cyBorder = GetSystemMetrics(SM_CYBORDER);
    // Get 3d border widths
    cxEdge = GetSystemMetrics(SM_CXEDGE);
    cyEdge = GetSystemMetrics(SM_CYEDGE);
    cx3Adornments = 3*BUTTONX + 2*SEPARATORX; // Width of 3 adornments with 2 separators in between

    // Desktop
    SC(ELEMENT_DESKTOP) = COLOR_BACKGROUND;
    RC(ELEMENT_DESKTOP) = rc;

    // App Window
    SC(ELEMENT_APPWINDOW) = COLOR_WINDOW;
    RC(ELEMENT_APPWINDOW).left = RC(ELEMENT_DESKTOP).left + 12;
    RC(ELEMENT_APPWINDOW).top = RC(ELEMENT_DESKTOP).top + 12;
    RC(ELEMENT_APPWINDOW).bottom = RC(ELEMENT_DESKTOP).bottom;
    RC(ELEMENT_APPWINDOW).right = RC(ELEMENT_DESKTOP).right;

    // App Bands
    SC(ELEMENT_APPBANDS) = COLOR_3DFACE;
    RC(ELEMENT_APPBANDS).left = RC(ELEMENT_APPWINDOW).left;
    RC(ELEMENT_APPBANDS).top = RC(ELEMENT_APPWINDOW).top;
    RC(ELEMENT_APPBANDS).bottom = RC(ELEMENT_APPBANDS).top + TOOLBARBUTTONY + 2*cxBorder;
    RC(ELEMENT_APPBANDS).right = RC(ELEMENT_APPWINDOW).right;

    // Menus
    hfontOld = (HFONT)SelectObject(hdc, FONT(FONT_MENU));
    // Get the average width of upper and lowercase letters. 2*AvgWidth is padded
    // on to width of the menu strings.
    GetTextMetrics(hdc,&tm);
    cxAvg2Char = (short)(2 * tm.tmAveCharWidth);

    // Menu Normal
    GetTextExtentPoint32(hdc, g_szNormal, lstrlen(g_szNormal), &sz);
    SC(ELEMENT_MENUNORMAL) = COLOR_MENUTEXT;
    RC(ELEMENT_MENUNORMAL).left = RC(ELEMENT_APPWINDOW).left + 2*cxBorder;
    RC(ELEMENT_MENUNORMAL).top = RC(ELEMENT_APPWINDOW).top + 2*cyBorder;
    RC(ELEMENT_MENUNORMAL).right = RC(ELEMENT_MENUNORMAL).left + sz.cx + cxAvg2Char;
    RC(ELEMENT_MENUNORMAL).bottom = RC(ELEMENT_MENUNORMAL).top + TOOLBARBUTTONY - cyBorder;

    // Menu Disabled
    GetTextExtentPoint32(hdc, g_szDisabled, lstrlen(g_szDisabled), &sz);
    SC(ELEMENT_MENUDISABLED) = COLOR_GRAYTEXT;
    RC(ELEMENT_MENUDISABLED).left = RC(ELEMENT_MENUNORMAL).right;
    RC(ELEMENT_MENUDISABLED).top = RC(ELEMENT_MENUNORMAL).top;
    RC(ELEMENT_MENUDISABLED).right = RC(ELEMENT_MENUDISABLED).left + sz.cx + cxAvg2Char;
    RC(ELEMENT_MENUDISABLED).bottom = RC(ELEMENT_MENUNORMAL).bottom;

    // Menu Selected
    GetTextExtentPoint32(hdc, g_szSelected, lstrlen(g_szSelected), &sz);
    SC(ELEMENT_MENUSELECTED) = COLOR_HIGHLIGHT;
    RC(ELEMENT_MENUSELECTED).left = RC(ELEMENT_MENUDISABLED).right;
    RC(ELEMENT_MENUSELECTED).top = RC(ELEMENT_MENUNORMAL).top;
    RC(ELEMENT_MENUSELECTED).right = RC(ELEMENT_MENUSELECTED).left + sz.cx + cxAvg2Char;
    RC(ELEMENT_MENUSELECTED).bottom = RC(ELEMENT_MENUNORMAL).bottom;

    // Menu
    SC(ELEMENT_MENU) = COLOR_MENU;
    RC(ELEMENT_MENU).left = RC(ELEMENT_APPWINDOW).left + cxBorder;
    RC(ELEMENT_MENU).top = RC(ELEMENT_APPWINDOW).top + cyBorder;
    RC(ELEMENT_MENU).right = RC(ELEMENT_MENUSELECTED).right;
    RC(ELEMENT_MENU).bottom = RC(ELEMENT_MENU).top + TOOLBARBUTTONY;

    hfontOld = (HFONT)SelectObject(hdc, hfontOld);

    // App Buttons
    SC(ELEMENT_APPBUTTONS) = COLOR_3DFACE;
    RC(ELEMENT_APPBUTTONS).left = RC(ELEMENT_MENU).right;
    RC(ELEMENT_APPBUTTONS).top = RC(ELEMENT_MENU).top;
    RC(ELEMENT_APPBUTTONS).right = RC(ELEMENT_APPADORNMENTS).left;
    RC(ELEMENT_APPBUTTONS).bottom = RC(ELEMENT_MENU).bottom;

    // App Adornments
    SC(ELEMENT_APPADORNMENTS) = COLOR_3DFACE;
    RC(ELEMENT_APPADORNMENTS).left = RC(ELEMENT_APPWINDOW).right -
        2*TOOLBARBUTTONX - SEPARATORX;
    RC(ELEMENT_APPADORNMENTS).top = RC(ELEMENT_MENU).top;
    RC(ELEMENT_APPADORNMENTS).right = RC(ELEMENT_APPWINDOW).right;
    RC(ELEMENT_APPADORNMENTS).bottom = RC(ELEMENT_MENU).bottom;

    // Scroll Up
    SC(ELEMENT_SCROLLUP) = COLOR_3DFACE;
    RC(ELEMENT_SCROLLUP).left = RC(ELEMENT_APPWINDOW).right - GetSystemMetrics(SM_CXVSCROLL);
    RC(ELEMENT_SCROLLUP).top = RC(ELEMENT_APPBANDS).bottom;
    RC(ELEMENT_SCROLLUP).right = RC(ELEMENT_APPWINDOW).right;
    RC(ELEMENT_SCROLLUP).bottom = RC(ELEMENT_SCROLLUP).top + GetSystemMetrics(SM_CYVSCROLL);

    // Scroll Down
    SC(ELEMENT_SCROLLDOWN) = COLOR_3DFACE;
    RC(ELEMENT_SCROLLDOWN).left = RC(ELEMENT_SCROLLUP).left;
    RC(ELEMENT_SCROLLDOWN).top = RC(ELEMENT_APPWINDOW).bottom - GetSystemMetrics(SM_CYVSCROLL);
    RC(ELEMENT_SCROLLDOWN).right = RC(ELEMENT_APPWINDOW).right;
    RC(ELEMENT_SCROLLDOWN).bottom = RC(ELEMENT_APPWINDOW).bottom;

    // Scroll
    SC(ELEMENT_SCROLLBAR) = COLOR_3DFACE;
    RC(ELEMENT_SCROLLBAR).left = RC(ELEMENT_SCROLLUP).left;
    RC(ELEMENT_SCROLLBAR).top = RC(ELEMENT_SCROLLUP).bottom;
    RC(ELEMENT_SCROLLBAR).right = RC(ELEMENT_APPWINDOW).right;
    RC(ELEMENT_SCROLLBAR).bottom = RC(ELEMENT_SCROLLDOWN).top;
  
    // Inactive Window
    SC(ELEMENT_INACTIVEWINDOW) = COLOR_WINDOW;
// For WinCE OS Raid #15112
//    RC(ELEMENT_INACTIVEWINDOW).left = RC(ELEMENT_APPWINDOW).left + cyCaption - cxEdge;
    RC(ELEMENT_INACTIVEWINDOW).left = RC(ELEMENT_APPWINDOW).left + cyCaption/2 - cxEdge;
    RC(ELEMENT_INACTIVEWINDOW).top = RC(ELEMENT_APPWINDOW).top + cyCaption - cyEdge;
/// OS ///    
    /*RC(ELEMENT_INACTIVEWINDOW).right = RC(ELEMENT_SCROLLBAR).left -
        (RC(ELEMENT_APPWINDOW).right - RC(ELEMENT_APPWINDOW).left)/5; */
// For WinCE OS Raid #15112
//    RC(ELEMENT_INACTIVEWINDOW).right = RC(ELEMENT_SCROLLBAR).left - (cyCaption - cxEdge);
    RC(ELEMENT_INACTIVEWINDOW).right = RC(ELEMENT_SCROLLBAR).left - (cyCaption/2 - cxEdge);
/// OS ///    
    RC(ELEMENT_INACTIVEWINDOW).bottom = RC(ELEMENT_APPWINDOW).bottom;

    // Inactive Caption
    SC(ELEMENT_INACTIVECAPTION) = COLOR_INACTIVECAPTION;
    RC(ELEMENT_INACTIVECAPTION).left = RC(ELEMENT_INACTIVEWINDOW).left + cxEdge + cxBorder;
    RC(ELEMENT_INACTIVECAPTION).top = RC(ELEMENT_INACTIVEWINDOW).top + cyEdge + cyBorder;
    RC(ELEMENT_INACTIVECAPTION).right = RC(ELEMENT_INACTIVEWINDOW).right - cxEdge - cxBorder;
    RC(ELEMENT_INACTIVECAPTION).bottom = RC(ELEMENT_INACTIVECAPTION).top + cyCaption;

    // Inactive Title
    hfontOld = (HFONT)SelectObject(hdc, FONT(FONT_SYSTEM_BOLD));
    GetTextExtentPoint32(hdc, g_szInactiveWindow, lstrlen(g_szInactiveWindow), &sz);
    SelectObject(hdc, hfontOld);

    SC(ELEMENT_INACTIVETITLE) = COLOR_INACTIVECAPTIONTEXT;
    RC(ELEMENT_INACTIVETITLE).top = RC(ELEMENT_INACTIVECAPTION).top;
    RC(ELEMENT_INACTIVETITLE).left = RC(ELEMENT_INACTIVECAPTION).left + 1;
    RC(ELEMENT_INACTIVETITLE).bottom = RC(ELEMENT_INACTIVECAPTION).bottom;
    RC(ELEMENT_INACTIVETITLE).right = RC(ELEMENT_INACTIVETITLE).left + cxEdge + sz.cx;

    // Inactive Adornments
    SC(ELEMENT_INACTIVEADORNMENTS) = COLOR_3DFACE;
    RC(ELEMENT_INACTIVEADORNMENTS).left = RC(ELEMENT_INACTIVECAPTION).right -
        cx3Adornments - cxEdge;
    RC(ELEMENT_INACTIVEADORNMENTS).top = RC(ELEMENT_INACTIVECAPTION).top + cyBorder;
    RC(ELEMENT_INACTIVEADORNMENTS).right = RC(ELEMENT_INACTIVECAPTION).right;
    RC(ELEMENT_INACTIVEADORNMENTS).bottom = RC(ELEMENT_INACTIVECAPTION).bottom;

    // Window Text
    hfontOld = (HFONT)SelectObject(hdc, FONT(FONT_SYSTEM_BOLD));
    GetTextExtentPoint32(hdc, g_szWindowText, lstrlen(g_szWindowText), &sz);
    SelectObject(hdc, hfontOld);

    SC(ELEMENT_WINDOWTEXT) = COLOR_WINDOWTEXT;
    RC(ELEMENT_WINDOWTEXT).top = RC(ELEMENT_INACTIVECAPTION).bottom + cxEdge;
    RC(ELEMENT_WINDOWTEXT).bottom = RC(ELEMENT_WINDOWTEXT).top + sz.cy + cyBorder;
    RC(ELEMENT_WINDOWTEXT).left = RC(ELEMENT_INACTIVEWINDOW).left + 2*cxEdge + 2*cxBorder;
    RC(ELEMENT_WINDOWTEXT).right = RC(ELEMENT_WINDOWTEXT).left + sz.cx + cxBorder + cxEdge;

    // Active Window
    SC(ELEMENT_ACTIVEWINDOW) = COLOR_STATIC;
// For WinCE OS Raid #15112
//    RC(ELEMENT_ACTIVEWINDOW).left = RC(ELEMENT_INACTIVEWINDOW).left + cyCaption - cxEdge;
    RC(ELEMENT_ACTIVEWINDOW).left = RC(ELEMENT_INACTIVEWINDOW).left + cyCaption/2 - cxEdge;
    RC(ELEMENT_ACTIVEWINDOW).top = RC(ELEMENT_INACTIVECAPTION).bottom + 2*cyEdge + sz.cy;
/// OS ///    
// For WinCE OS Raid #15112
//    RC(ELEMENT_ACTIVEWINDOW).right = RC(ELEMENT_SCROLLBAR).left - cyCaption - (cyCaption/2);
    RC(ELEMENT_ACTIVEWINDOW).right = RC(ELEMENT_SCROLLBAR).left - (cyCaption - cxEdge);
    //RC(ELEMENT_ACTIVEWINDOW).right = RC(ELEMENT_SCROLLBAR).left - 2*cyCaption;
/// OS ///    
    RC(ELEMENT_ACTIVEWINDOW).bottom = RC(ELEMENT_APPWINDOW).bottom;

    // Active Caption
    SC(ELEMENT_ACTIVECAPTION) = COLOR_ACTIVECAPTION;
    RC(ELEMENT_ACTIVECAPTION).left = RC(ELEMENT_ACTIVEWINDOW).left + cxEdge + cxBorder;
    RC(ELEMENT_ACTIVECAPTION).top = RC(ELEMENT_ACTIVEWINDOW).top + cyEdge + cyBorder;
    RC(ELEMENT_ACTIVECAPTION).right = RC(ELEMENT_ACTIVEWINDOW).right - cxEdge - cxBorder;
    RC(ELEMENT_ACTIVECAPTION).bottom = RC(ELEMENT_ACTIVECAPTION).top + cyCaption;

    // Active Title
    hfontOld = (HFONT)SelectObject(hdc, FONT(FONT_SYSTEM_BOLD));
    GetTextExtentPoint32(hdc, g_szActiveWindow, lstrlen(g_szActiveWindow), &sz);
    SelectObject(hdc, hfontOld);

    SC(ELEMENT_ACTIVETITLE) = COLOR_CAPTIONTEXT;
    RC(ELEMENT_ACTIVETITLE).top = RC(ELEMENT_ACTIVECAPTION).top;
    RC(ELEMENT_ACTIVETITLE).left = RC(ELEMENT_ACTIVECAPTION).left + 1;
    RC(ELEMENT_ACTIVETITLE).bottom = RC(ELEMENT_ACTIVECAPTION).bottom;
    RC(ELEMENT_ACTIVETITLE).right = RC(ELEMENT_ACTIVETITLE).left + cxEdge + sz.cx;

    // Active Adornments
    SC(ELEMENT_ACTIVEADORNMENTS) = COLOR_3DFACE;
    RC(ELEMENT_ACTIVEADORNMENTS).left = RC(ELEMENT_ACTIVECAPTION).right - cx3Adornments - cxEdge;
    RC(ELEMENT_ACTIVEADORNMENTS).top = RC(ELEMENT_ACTIVECAPTION).top + cyBorder;
    RC(ELEMENT_ACTIVEADORNMENTS).right = RC(ELEMENT_ACTIVECAPTION).right;
    RC(ELEMENT_ACTIVEADORNMENTS).bottom = RC(ELEMENT_ACTIVECAPTION).bottom;

    // Dialog/Message Text
    hfontOld = (HFONT)SelectObject(hdc, FONT(FONT_MSGBOX));
    GetTextExtentPoint32(hdc, g_szMessageText, lstrlen(g_szMessageText), &sz);
    SelectObject(hdc, hfontOld);

    SC(ELEMENT_MESSAGETEXT) = COLOR_STATICTEXT;
    RC(ELEMENT_MESSAGETEXT).top = RC(ELEMENT_ACTIVECAPTION).bottom + cxEdge;
    RC(ELEMENT_MESSAGETEXT).bottom = RC(ELEMENT_MESSAGETEXT).top + sz.cy + cyBorder;
    RC(ELEMENT_MESSAGETEXT).left = RC(ELEMENT_ACTIVEWINDOW).left + 2*cxEdge + 2*cxBorder;
    RC(ELEMENT_MESSAGETEXT).right = RC(ELEMENT_MESSAGETEXT).left + sz.cx + cxBorder + cxEdge;

    // Button
    hfontOld = (HFONT)SelectObject(hdc, FONT(FONT_SYSTEM_BOLD));
    GetTextExtentPoint32(hdc, g_szButton, lstrlen(g_szButton), &sz);
    SelectObject(hdc, hfontOld);

    SC(ELEMENT_BUTTON) = COLOR_3DFACE;
//// OS ////
    RC(ELEMENT_BUTTON).bottom = RC(ELEMENT_APPWINDOW).bottom - 2*cyEdge;
    RC(ELEMENT_BUTTON).top = RC(ELEMENT_BUTTON).bottom - sz.cy - 4*cyEdge;
    // RC(ELEMENT_BUTTON).top = RC(ELEMENT_ACTIVECAPTION).bottom + 2*cxEdge;
    // RC(ELEMENT_BUTTON).bottom = RC(ELEMENT_BUTTON).top + sz.cy + 4*cyEdge;
///// OS ////    
    RC(ELEMENT_BUTTON).right = RC(ELEMENT_ACTIVEWINDOW).right - 10*cxEdge;
    RC(ELEMENT_BUTTON).left = RC(ELEMENT_BUTTON).right - sz.cx - 12*cxEdge;
}
// TODO:
// 1. Check if HELP works.
// Use DrawFrameControl for rendering preview?



/*  
**    COLOR2.C
*/


int H,L,S;                         /* Hue, Lightness, Saturation */
#define  RANGE   240                 /* range of values for HLS scrollbars */
                                     /* HLS-RGB conversions work best when
                                        RANGE is divisible by 6 */
#define  HLSMAX   RANGE
#define  RGBMAX   255
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* Color conversion routines --

   RGBtoHLS() takes a DWORD RGB value, translates it to HLS, and stores the
   results in the global vars H, L, and S.  HLStoRGB takes the current values
   of H, L, and S and returns the equivalent value in an RGB DWORD.  The vars
   H, L and S are written to only by 1) RGBtoHLS (initialization) or 2) the
   scrollbar handlers.

   A point of reference for the algorithms is Foley and Van Dam, pp. 618-19.
   Their algorithm is in floating point.  CHART implements a less general
   (hardwired ranges) integral algorithm.

*/

/* There are potential roundoff errors throughout here.
   (0.5 + x/y) without floating point,
      (x/y) phrased ((x + (y/2))/y)
   yields very small roundoff error.
   This makes many of the following divisions look unusual.
*/

                        /* H,L, and S vary over 0-HLSMAX */
                        /* R,G, and B vary over 0-RGBMAX */
                        /* HLSMAX BEST IF DIVISIBLE BY 6 */
                        /* RGBMAX, HLSMAX must each fit in a byte. */

#define UNDEFINED (HLSMAX*2/3)/* Hue is undefined if Saturation is 0 (grey-scale) */
                           /* This value determines where the Hue scrollbar is */
                           /* initially set for achromatic colors */

void RGBtoHLS(DWORD lRGBColor)
{
   int R,G,B;                /* input RGB values */
   WORD cMax,cMin;        /* max and min RGB values */
   WORD cSum,cDif;
   int  Rdelta,Gdelta,Bdelta;  /* intermediate value: % of spread from max */

   /* get R, G, and B out of DWORD */
   R = GetRValue(lRGBColor);
   G = GetGValue(lRGBColor);
   B = GetBValue(lRGBColor);

   /* calculate lightness */
   cMax = max( max(R,G), B);
   cMin = min( min(R,G), B);
   cSum = cMax + cMin;
   L = (WORD)(((cSum * (DWORD)HLSMAX) + RGBMAX )/(2*RGBMAX));

   cDif = cMax - cMin;
   if (!cDif)       /* r=g=b --> achromatic case */
     {
       S = 0;                         /* saturation */
       H = UNDEFINED;                 /* hue */
     }
   else                           /* chromatic case */
     {
       /* saturation */
       if (L <= (HLSMAX/2))
           S = (WORD) (((cDif * (DWORD) HLSMAX) + (cSum / 2) ) / cSum);
       else
           S = (WORD) ((DWORD) ((cDif * (DWORD) HLSMAX) + (DWORD)((2*RGBMAX-cSum)/2) )
                                                 / (2*RGBMAX-cSum));
      /* hue */
      Rdelta = (int) (( ((cMax-R)*(DWORD)(HLSMAX/6)) + (cDif / 2) ) / cDif);
      Gdelta = (int) (( ((cMax-G)*(DWORD)(HLSMAX/6)) + (cDif / 2) ) / cDif);
      Bdelta = (int) (( ((cMax-B)*(DWORD)(HLSMAX/6)) + (cDif / 2) ) / cDif);

      if ((WORD) R == cMax)
         H = Bdelta - Gdelta;
      else if ((WORD) G == cMax)
         H = (HLSMAX/3) + Rdelta - Bdelta;
      else /* B == cMax */
         H = ((2*HLSMAX)/3) + Gdelta - Rdelta;

      if (H < 0)
         H += HLSMAX;
      if (H > HLSMAX)
         H -= HLSMAX;
   }
}

/* utility routine for HLStoRGB */
WORD NEAR PASCAL HueToRGB(WORD n1, WORD n2, WORD hue)
{

   /* range check: note values passed add/subtract thirds of range */

   /* The following is redundant for WORD (unsigned int) */

   if (hue > HLSMAX)
      hue -= HLSMAX;

   /* return r,g, or b value from this tridrant */
   if (hue < (HLSMAX/6))
      return ( n1 + (((n2-n1)*hue+(HLSMAX/12))/(HLSMAX/6)) );
   if (hue < (HLSMAX/2))
      return ( n2 );
   if (hue < ((HLSMAX*2)/3))
      return ( n1 + (((n2-n1)*(((HLSMAX*2)/3)-hue)+(HLSMAX/12)) / (HLSMAX/6)) );
   else
      return ( n1 );
}


DWORD NEAR PASCAL HLStoRGB(WORD hue, WORD lum, WORD sat)
{
  WORD R,G,B;                      /* RGB component values */
  WORD  Magic1,Magic2;       /* calculated magic numbers (really!) */

  if (sat == 0)                /* achromatic case */
    {
      R = G = B = (lum * RGBMAX) / HLSMAX;
      if (hue != UNDEFINED)
        {
         /* ERROR */
        }
    }
  else                         /* chromatic case */
    {
      /* set up magic numbers */
      if (lum <= (HLSMAX/2))
          Magic2 = (WORD)((lum * ((DWORD)HLSMAX + sat) + (HLSMAX/2))/HLSMAX);
      else
          Magic2 = lum + sat - (WORD)(((lum*sat) + (DWORD)(HLSMAX/2))/HLSMAX);
      Magic1 = 2*lum-Magic2;

      /* get RGB, change units from HLSMAX to RGBMAX */
      R = (WORD)((HueToRGB(Magic1,Magic2,(WORD)(hue+(WORD)(HLSMAX/3)))*(DWORD)RGBMAX + (HLSMAX/2))) / (WORD)HLSMAX;
      G = (WORD)((HueToRGB(Magic1,Magic2,hue)*(DWORD)RGBMAX + (HLSMAX/2))) / HLSMAX;
      B = (WORD)((HueToRGB(Magic1,Magic2,(WORD)(hue-(WORD)(HLSMAX/3)))*(DWORD)RGBMAX + (HLSMAX/2))) / (WORD)HLSMAX;
    }
  return(RGB(R,G,B));
}


DWORD  DarkenColor(DWORD rgb, int n)
{
    RGBtoHLS(rgb);
    return HLStoRGB((WORD)H, (WORD)((long)L * n / 1000), (WORD)S);
}

DWORD  BrightenColor(DWORD rgb, int n)
{
    RGBtoHLS(rgb);
    return HLStoRGB((WORD)H, (WORD)(((long)L * (1000-n) + (RANGE+1l)*n) / 1000), (WORD)S);
}

DWORD FAR PASCAL AdjustLuma(DWORD rgb, int n, BOOL fScale)
{
    if (n == 0)
        return rgb;

    RGBtoHLS(rgb);

    if (fScale)
    {
        if (n > 0)
        {
            return HLStoRGB((WORD)H, (WORD)(((long)L * (1000 - n) + (RANGE + 1l) * n) / 1000), (WORD)S);
        }
        else
        {
            return HLStoRGB((WORD)H, (WORD)(((long)L * (n + 1000)) / 1000), (WORD)S);
        }
    }

    L += (int)((long)n * RANGE / 1000);

    if (L < 0)
        L = 0;
    if (L > HLSMAX)
        L = HLSMAX;

    return HLStoRGB((WORD)H, (WORD)L, (WORD)S);
}

