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

Abstract: Utility functions for the CPL applets.
		  All functions in this file must be leaf-level
**/

#include "cplpch.h"

extern HINSTANCE        g_hInstAygshell;
extern BOOL             g_bCalledSHInitExtraControls;
extern BOOL             g_bLoadedAygshellLibrary;
extern int              g_iAygshellRefCount;
extern CRITICAL_SECTION g_csAygshell;

////////////////////////////////////////////////////////////////////////////
//
//  AygShell utility functions
//
////////////////////////////////////////////////////////////////////////////
BOOL LoadAygshellLibrary()
{
    BOOL bReturn = FALSE;
    EnterCriticalSection( &g_csAygshell );

    if ( g_hInstAygshell )
    {
        ASSERT( g_iAygshellRefCount > 0 );
        g_iAygshellRefCount++;
        bReturn = TRUE;
    }
    else
    {
        ASSERT( g_iAygshellRefCount == 0 );
        g_hInstAygshell = LoadLibrary( TEXT( "aygshell.dll" ) );
        if ( g_hInstAygshell )
        {
            g_iAygshellRefCount++;
            g_bLoadedAygshellLibrary = TRUE;
            bReturn = TRUE;
        }
    }

    LeaveCriticalSection( &g_csAygshell );

    return bReturn;
}

BOOL FreeAygshellLibrary()
{
    if ( !g_bLoadedAygshellLibrary )
    {
        return FALSE;
    }

    ASSERT( g_hInstAygshell );
    ASSERT( g_iAygshellRefCount > 0 );

    EnterCriticalSection( &g_csAygshell );

    g_iAygshellRefCount--;

    if ( g_iAygshellRefCount < 1 )
    {
        FreeLibrary( g_hInstAygshell );
        g_hInstAygshell = NULL;
        g_bCalledSHInitExtraControls = FALSE;
    }

    LeaveCriticalSection( &g_csAygshell );

    return TRUE;
}

BOOL AygInitDialog( HWND hwnd, DWORD dwFlags )
{
    BOOL bReturn = FALSE;
    SHINITDLGINFO shidi = { 0 };
    BOOL (*PFN_SHInitDialog)(PSHINITDLGINFO) = NULL;

    if ( !LoadAygshellLibrary() )
    {
        return FALSE;
    }
    
    shidi.dwMask = SHIDIM_FLAGS;
    shidi.hDlg = hwnd;
    shidi.dwFlags = dwFlags;
    PFN_SHInitDialog = (BOOL (*)(PSHINITDLGINFO)) GetProcAddress( g_hInstAygshell, TEXT("SHInitDialog") );
    if (PFN_SHInitDialog)
    {
        bReturn = PFN_SHInitDialog( &shidi );
    }

    if ( !FreeAygshellLibrary() )
    {
        return FALSE;
    }

    return bReturn;
}

BOOL AygInitExtraControls()
{
    BOOL bReturn = FALSE;
    BOOL (*PFN_SHInitExtraControls)() = NULL;

    if ( !g_hInstAygshell )
    {
        return FALSE;
    }

    if ( g_bCalledSHInitExtraControls )
    {
        return TRUE;
    }

    PFN_SHInitExtraControls = (BOOL (*)()) GetProcAddress( g_hInstAygshell, TEXT( "SHInitExtraControls" ) );
    if ( PFN_SHInitExtraControls )
    {
        bReturn = PFN_SHInitExtraControls();
    }

    g_bCalledSHInitExtraControls = bReturn;

    return bReturn;
}

BOOL AygAddSipprefControl( HWND hwnd )
{
    if ( !g_hInstAygshell )
    {
        return FALSE;
    }

    if ( !AygInitExtraControls() )
    {
        return FALSE;
    }

    WNDCLASS wc;
    if (GetClassInfo(g_hInst, WC_SIPPREF, &wc))
    {
       CreateWindow(WC_SIPPREF, NULL, WS_CHILD, -10, -10,
                    5, 5, hwnd, NULL, g_hInst, NULL);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

void LoadRegGlobals(void)
{
	CReg reg(HKEY_LOCAL_MACHINE, RK_CONTROLPANEL);
	g_fNoDrag = reg.ValueDW(RV_NODRAG);
	g_fFullScreen = reg.ValueDW(RV_FULLSCREEN);
	g_fRecenterForSIP = reg.ValueDW(RV_RECENTERFORSIP);
	g_fRaiseLowerSIP = reg.ValueDW(RV_RAISELOWERSIP);
}

// returns NULL if we found another instance, non-NULL otherwise
// if the return is not -1, then it is a valid handle that teh caller must release
HANDLE CheckPrevInstance(LPCTSTR pszMutexName, LPCTSTR pszTitle, BOOL fPwd)
{
	HANDLE hMutex;

	// try to create a mutex object
    hMutex=CreateMutex(NULL, FALSE, pszMutexName);
    if(!hMutex) {
    	// Failed for unknown reasons. We now won't have single-instanceing 
    	// of this CPL. C'est la vie. Carry on anyway & create new CPL
    	ASSERT(FALSE);
    	return (HANDLE)-1;
    }

	// We got the mutex. Now check whether it previously existed
    if(ERROR_ALREADY_EXISTS==GetLastError()) {
		HWND hwnd;
        // release the handle so mutex will be freed when the other guy exits
        CloseHandle(hMutex);
        if(hwnd = FindWindow(DIALOG_CLASS, pszTitle ) ) {
        	DEBUGMSG(ZONE_MSGS, (L"CPL: found prev instance of (%s %s) %08x\r\n", pszMutexName, pszTitle, hwnd));
            SetForegroundWindow( (HWND)((DWORD)hwnd | 1) );
            return NULL;
        } else if(fPwd && (hwnd = FindWindow(DIALOG_CLASS, CELOADSZ(IDS_PWDCHK_TITLE)))) {
        	DEBUGMSG(ZONE_MSGS, (L"CPL: found prev instance of (%s) in Passwd prompt %08x\r\n", pszMutexName, hwnd));
            SetForegroundWindow( (HWND)((DWORD)hwnd | 1) );
            return NULL;
        } else {
        	// Bad news. Mutex existed but window did not. Carry on anyway & create new CPL
        	DEBUGMSG(ZONE_MSGS, (L"CPL: Found prev instance of (%s) but no window! Creating anyway\r\n",pszMutexName));
	        return (HANDLE)-1;
        }
    } else {
    	// if we got here we have a first-time created mutex. Return the 
    	// handle so caller can free it before exiting
	    return hMutex;
	}
}

void SetDeviceDependentText(HWND hDlg, int idc, int idsFmt)
{
	LPCTSTR pszInsert;
	LPTSTR pszBuf;
	LPCTSTR pszFmt = CELOADSZ(idsFmt);
	
	CReg reg(HKEY_LOCAL_MACHINE, RK_CONTROLPANEL);
	
	if(!(pszInsert = reg.ValueSZ(RV_DEVICENAME)))
		pszInsert = CELOADSZ(IDS_DEFAULT_LONGNAME);

	if(!pszInsert) return;
	//Prefix
	if(pszFmt) {
		pszBuf = MySzAlloca(lstrlen(pszFmt) + lstrlen(pszInsert) + 1); // _alloca freed on fn exit
		swprintf(pszBuf, pszFmt, pszInsert);
		SetDlgItemText(hDlg, idc, pszBuf);
	}
}



BOOL SetWordCompletion(BOOL fOn)
{
#if 1	// word completion feature not supported yet
	return FALSE;
#else
	SIPINFO	si;
	BOOL	fRet = FALSE;
    ZEROMEM(&si);
    si.cbSize = sizeof(SIPINFO);
    if (g_pSipGetInfo && g_pSipSetInfo && g_pSipGetInfo(&si))
    {
    	// Toggle password completion, but keep track of 
        // whether or not this flag was set a priori when setting focus
		fRet = !(si.fdwFlags & SIPF_DISABLECOMPLETION);
        if(fOn) 
        	si.fdwFlags |= SIPF_DISABLECOMPLETION;
		else
			si.fdwFlags &= ~SIPF_DISABLECOMPLETION;
		g_pSipSetInfo(&si);
	}
	return fRet;
#endif         
}

BOOL CenterWindowSIPAware(HWND hwnd, BOOL fInitial)
{
	RECT    rcWnd;
	//Prefix
    RECT    rcWorkArea = {0};
    SIPINFO si;

    // Get the size of window
    GetWindowRect(hwnd, &rcWnd);

   	ZEROMEM(&si);
	si.cbSize = sizeof(SIPINFO);

	// Get the size of SIP-less area OR if no SIP, then screen workarea
    if(g_fRecenterForSIP && g_pSipGetInfo && g_pSipGetInfo(&si)) {
		rcWorkArea = si.rcVisibleDesktop;
		// if we're positioning after a INICHANGE, then only do something on SIP-up, not SIP-down
		if(!fInitial && !(si.fdwFlags & SIPF_ON))
			return FALSE;
		
   	} else {
    	SystemParametersInfo(SPI_GETWORKAREA, FALSE, &rcWorkArea, 0);
	}

	// Calc total space leftover on both sides (may be negative if window is too big)
    int cx = ((rcWorkArea.right - rcWorkArea.left) - (rcWnd.right - rcWnd.left))/2;
    int cy = ((rcWorkArea.bottom - rcWorkArea.top) - (rcWnd.bottom - rcWnd.top))/2;

	// center the dialog in the work area
	DEBUGMSG(ZONE_UTILS, (L"CPLMAIN recenter: WorkArea=(top=%d, bottom=%d, left=%d, right=%d) Wnd=(top=%d, bottom=%d, left=%d, right=%d) cx=%d cy=%d\r\n",
		rcWorkArea.top, rcWorkArea.bottom, rcWorkArea.left, rcWorkArea.right,
		rcWnd.top, rcWnd.bottom, rcWnd.left, rcWnd.right, cx, cy));

	// if X is negative reset to 0 (so we fall off right edge only, not both)
	// if Y is negative reset to 0 (so we fall off bottom only, not top & bottom)
	if(cx < 0) cx = 0;
	if(cy < 0) cy = 0;

    return SetWindowPos(hwnd, NULL, rcWorkArea.left+cx, rcWorkArea.top+cy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
} 

DWORD GetSipState(void)
{
	SIPINFO si;
    ZEROMEM(&si);
	si.cbSize = sizeof(si);
	
	if(g_pSipGetInfo && g_pSipGetInfo(&si))
	{
		DEBUGMSG(ZONE_UTILS, (L"GetSipState=%d\r\n", si.fdwFlags & SIPF_ON));
		return si.fdwFlags;
	}
	else
		return 0;
}			

void RaiseLowerSip(BOOL fUp)
{
	SIPINFO si;
    ZEROMEM(&si);
	si.cbSize = sizeof(si);
	
	if(!g_pSipGetInfo || !g_pSipSetInfo)
		return;

	if(g_pSipGetInfo(&si))
	{
        if(fUp)
            si.fdwFlags |= SIPF_ON;
        else
            si.fdwFlags &= ~SIPF_ON;
		DEBUGMSG(ZONE_UTILS, (L"SetSipState=%d %d\r\n", fUp, si.fdwFlags & SIPF_ON));
		g_pSipSetInfo(&si);
	}
}

#if 0 // moved to comm.cpp to make use of sethostname from winsock
// We don't let the user type spaces or any other weird chars, just like Minerva...
BOOL CheckDeviceName(BOOL fFinal, HWND hDlg, int idcEdit, int iMax)
{
	LPTSTR pszBuf = MySzAlloca(iMax); // alloca freed automatically on fn exit
	pszBuf[0] = 0;
	GetDlgItemText(hDlg, idcEdit, pszBuf, iMax);

	// check for illegal char always. Check for empty string only is fFinal==TRUE
    if( (wcscspn(pszBuf, TEXT(" \\/:*?\"<>|.")) != wcslen(pszBuf)) ||
    	(fFinal && !pszBuf[0]) )
    {
		MessageBox(hDlg, CELOADSZ(IDS_CPLERR_DEVICENAME), CELOADSZ(IDS_CPLERR_DEVICETITLE), MB_ICONEXCLAMATION);
		SetFocus(GetDlgItem(hDlg, idcEdit));
	    Edit_SetSel(DI(idcEdit), 0L, -1);	
		return FALSE;
	}			
	return TRUE;
}
#endif // 0

void DrawBitmapOnDc(DRAWITEMSTRUCT *lpdis, int nBitmapId, BOOL fDisabled)
{
	HDC     hdc 	= lpdis->hDC;
	LPRECT	lpRect  = &lpdis->rcItem;
	BOOL 	bPushed = lpdis->itemState & ODS_SELECTED;
	BOOL    bFocus  = lpdis->itemState & ODS_FOCUS;
	BITMAP bm;
	HBITMAP hbmp, hbmp2;
	HDC hdcMem;
	int x,y;
	COLORREF crBk, crText;

     hbmp = LoadBitmap(g_hInst, MAKEINTRESOURCE(nBitmapId));
     if (hbmp) {
         hdcMem = CreateCompatibleDC(hdc);
         if (hdcMem) {
            hbmp2 = (HBITMAP) SelectObject(hdcMem, hbmp);
            GetObject(hbmp, sizeof(BITMAP), (LPSTR) &bm);

			x = (lpRect->right  + lpRect->left -  bm.bmWidth)  / 2;
			y = (lpRect->bottom + lpRect->top  -  bm.bmHeight) / 2;

			if (bPushed) {
				x++;
				y++;
				}

			#define CLR_WHITE   0x00FFFFFFL
			#define CLR_BLACK   0x00000000L
			#define CLR_LTGRAY	RGB(192, 192, 192)
			#define CLR_GRAY	RGB(128, 128, 128)
			crBk=SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
			if(fDisabled)
				crText=SetTextColor(hdc, (GetSysColor(COLOR_BTNFACE)==CLR_GRAY) ? CLR_LTGRAY : CLR_GRAY);
			else
				crText=SetTextColor(hdc, (GetSysColor(COLOR_BTNFACE)==CLR_BLACK) ? CLR_WHITE : CLR_BLACK);
    	    BitBlt(hdc, x, y, bm.bmWidth,  bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

			// restore everything
			SetBkColor(hdc, crBk);
			SetTextColor(hdc, crText);
            hbmp = (HBITMAP) SelectObject(hdcMem, hbmp2);
            DeleteDC(hdcMem);
        	}
        DeleteObject(hbmp);
	    }
     if (bFocus) {
		InflateRect(lpRect, -3, -3);
        DrawFocusRect(hdc, lpRect);
		}
}

void DrawCPLButton(DRAWITEMSTRUCT* lpdis, int idbBitmap)
{
	// Fill area with gray and frame it black
    FillRect(lpdis->hDC, &lpdis->rcItem, (HBRUSH)(COLOR_BTNFACE+1));
    DrawEdge(lpdis->hDC, &lpdis->rcItem, (lpdis->itemState & ODS_SELECTED) ? EDGE_SUNKEN : EDGE_RAISED, BF_RECT | BF_SOFT);

	DrawBitmapOnDc(lpdis, idbBitmap, (lpdis->itemState & ODS_DISABLED));
}

// Popup dlg to get user password before allowing entry into certain CPLs
// It prompts for passwd & returns NULL or the entered password (localalloc'd)
BOOL APIENTRY GetPasswordDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	TCHAR	*pszHelpPath[] = {TEXT("file:ctpnl.htm#prompt_for_password\0")};

    switch (message)
    {
    case WM_HELP:
		MyCreateProcess(TEXT("peghelp"), pszHelpPath[0]);
		break;

    case WM_INITDIALOG:
      {
      	SetWindowText(hDlg, CELOADSZ(IDS_PWDCHK_TITLE));
		CenterWindowSIPAware(hDlg);
		Edit_LimitText(DI(IDC_EDIT_OLD_PASS), PASSWORD_LENGTH);
		// The old password if any is passed in in lParam
        LPTSTR pszText = (LPTSTR)lParam;
        if (pszText && *pszText) {
        	SetWindowText(DI(IDC_EDIT_OLD_PASS), pszText);
            Edit_SetSel(DI(IDC_EDIT_OLD_PASS), 0L, -1);    
		}
        return 1; // let GWE set focus
	  }
	  
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
        	case IDOK:
	        	TCHAR szBuf[PASSWORD_LENGTH+2];
            	GetDlgItemText(hDlg, IDC_EDIT_OLD_PASS, szBuf, PASSWORD_LENGTH+1);
            	// Check the entered password
				// NOTE: The Control Panels & the StartUI & Welcome dialogs all force passwds to
				// lowercase so as to be case-insensitive, despite the fact that the underlying
				// OS password APIs are case sensitive.
				//
				_wcslwr(szBuf);
				if(CheckPassword(szBuf)) {
					EndDialog(hDlg, (int)MySzDup(szBuf)); // caller must free
				} else {
					// bad password. set an error message & stay in dlg box
					SetWindowText(DI(IDC_PWDMSG), CELOADSZ(IDS_PWDCHKERR_INCORRECT));
					MessageBeep(MB_ICONASTERISK);
					SetFocus(DI(IDC_EDIT_OLD_PASS));
                    Edit_SetSel(DI(IDC_EDIT_OLD_PASS), 0L, -1);
				}
                return TRUE;
                                                
			case IDCANCEL:
        		EndDialog(hDlg, 0);
				return TRUE;
		}
		break;
	}
    return (FALSE);
}

LPTSTR PromptForPasswd(HWND hParent)
{
	// Check if we have a password at all
	if(CheckPassword(EMPTY_STRING) || CheckPassword(NULL)) {
		DEBUGMSG(ZONE_UTILS, (L"CPL:PromptForPasswd -- NO password\r\n"));
		return MySzDup(EMPTY_STRING); // caller frees
	}

	// caller frees the return value of this dialog box
	return (LPTSTR)DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_CHECK_PASSWD), 
									hParent, GetPasswordDlgProc, NULL);
}									

HFONT CreateBoldFont(HWND hDlg)
{	
	LOGFONT	lFont;
	HFONT	hDlgFont;
	DWORD   dwWeight;
	
	CReg reg(HKEY_LOCAL_MACHINE, RK_SYSTEM_GWE_BUTTON);
	if(!(dwWeight = reg.ValueDW(RV_FONTWEIGHT)))
		dwWeight = FW_BOLD;
		
	if(dwWeight <= FW_NORMAL)
	{
		DEBUGMSG(ZONE_UTILS, (L"CPL: Static bolding turned OFF\r\n"));
		return NULL;
	}

	// Set bold text for separator static
	ZEROMEM(&lFont);
	hDlgFont = (HFONT)SendMessage(hDlg,  WM_GETFONT, 0, 0L);
	if (!hDlgFont)				// use system font otherwise
		hDlgFont = (HFONT)GetStockObject(SYSTEM_FONT);
	GetObject(hDlgFont, sizeof(LOGFONT), (LPSTR)&lFont);

	if (lFont.lfHeight) {
		lFont.lfWeight =  dwWeight;
		if ((hDlgFont = CreateFontIndirect(&lFont)) != NULL)
			return hDlgFont;
	}
	return NULL;
}

void SetTextWithEllipsis(HWND hwnd, LPCTSTR pszText)
{
    int   	nMaxChars;
    SIZE    size;
    HDC     hdc = GetDC(hwnd);
    RECT    rc;
    LPTSTR	pszNew;

    GetClientRect(hwnd, &rc);
    nMaxChars = lstrlen(pszText);
    GetTextExtentExPoint(hdc, pszText, nMaxChars, rc.right-rc.left, &nMaxChars, NULL, &size);
    ReleaseDC(hwnd, hdc);

	// arbitrarily decide not to use ellipsis unless we have space for 6 chars total
    if( (nMaxChars <= (int)lstrlen(pszText)) && 
    	(nMaxChars > 6) &&
    	(pszNew = MySzDup(pszText)) ) // freed below
    {
        pszNew[nMaxChars-3] = (TCHAR)'.';
        pszNew[nMaxChars-2] = (TCHAR)'.';
        pszNew[nMaxChars-1] = (TCHAR)'.';
        pszNew[nMaxChars]   = 0;  // Null-terminate the string
	    SetWindowText(hwnd, pszNew);
	    MyFree(pszNew);
    }
    else
    {
	    SetWindowText(hwnd, pszText);
	}
}

// Useful for initializing a combo box from a table
void LoadCB(HWND hwndCB, const COMBODATA rgCBData[], BOOL fData) {
	for(int i=0; rgCBData[i].idcString; i++) {
		int iIndex = ComboBox_AddString(hwndCB, CELOADSZ(rgCBData[i].idcString));
		if(fData && iIndex!=CB_ERR) {
			ComboBox_SetItemData(hwndCB, iIndex, rgCBData[i].dwData);
		}
	}
}

HBRUSH WINAPI LoadDIBitmapBrush(LPCTSTR szFileName)
{
	HBRUSH hbrush = NULL;
	BITMAPFILEHEADER bmfh;
	LPBYTE pdib;
	HANDLE hFile;
	DWORD dwFileSize, dwRead;
	
	hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ,
					   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return NULL;
	}

	dwFileSize = GetFileSize(hFile, NULL);
	//Prefix
	if (dwFileSize != 0xFFFFFFFF) { 
	
		if (!ReadFile(hFile, (LPVOID)&bmfh, sizeof(BITMAPFILEHEADER), &dwRead, NULL))
			goto errReturn;
	
		pdib = (LPBYTE)VirtualAlloc(NULL, (dwFileSize - dwRead),
								MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
		if (pdib == NULL) {
			goto errReturn;
		}

		if (!ReadFile(hFile, (LPVOID)pdib, (dwFileSize - dwRead), &dwRead, NULL))
			goto errReturn;

		hbrush = CreateDIBPatternBrushPt(pdib, DIB_RGB_COLORS);
		VirtualFree(pdib, 0, MEM_RELEASE);
	}

errReturn:	
	CloseHandle(hFile);
	return hbrush;
	
}

void InitImageList(HIMAGELIST& hImgList, BOOL fMini, int rgIcons[], int iNumIcons)
{
    WORD  cx;
    WORD  cy;

    if (hImgList)
    {                          
        ImageList_Destroy(hImgList);
        hImgList = NULL;
    }
    cx = GetSystemMetrics(fMini ? SM_CXSMICON : SM_CXICON);
    cy = GetSystemMetrics(fMini ? SM_CYSMICON : SM_CYICON);

    if(!(hImgList = ImageList_Create(cx, cy, TRUE, iNumIcons, 2))) {
    	ASSERT(FALSE);
    	return;
    }

    for(int i=0; i<iNumIcons; i++)
    {
	    HICON hIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(rgIcons[i]),IMAGE_ICON,cx,cy,LR_DEFAULTCOLOR);
		ASSERT(hIcon);
		//Prefix
		if( hIcon ) {
   			ImageList_AddIcon(hImgList, hIcon);
    		DestroyIcon(hIcon);
		}
    }
}

/////////////////////////////////////////////////////////////////////////////
// CReg: Registry read/write helper class
/////////////////////////////////////////////////////////////////////////////


LPCTSTR CReg::ValueSZ(LPCTSTR szName)
{
	if(!m_hKey) return FALSE;
	DWORD dwLen = 0;
	if( (ERROR_SUCCESS != RegQueryValueEx(m_hKey, szName, NULL, NULL, NULL, &dwLen)) || 
	    (dwLen == 0) )
	    	return NULL;
	MyFree(m_lpbValue);
	if( !(m_lpbValue = MyRgAlloc(BYTE, dwLen)) ||
		(ERROR_SUCCESS != RegQueryValueEx(m_hKey, szName, NULL, NULL, m_lpbValue, &dwLen)) )
			return NULL;
	return (LPTSTR)m_lpbValue;
}

/////////////////////////////////////////////////////////////////////////////
// CFileCombo
// a class that populates a combo box with filenames. Friendly names are displayed
// Full paths are stored as item-data. Main point of the class is to manage the memory
// for the path strings cleanly in one place
/////////////////////////////////////////////////////////////////////////////

CFileCombo::~CFileCombo()
{
	for(int i=0; ;i++)
	{
		DWORD dwData = ComboBox_GetItemData(m_hwndCB, i);
		if(CB_ERR == dwData)
			break;
		ComboBox_SetItemData(m_hwndCB, i, NULL);
		MyFree((LPVOID)dwData);
	}
}

int CFileCombo::AddFile(LPCTSTR szDir, LPCTSTR szFile)
{
    TCHAR szTemp[MAX_PATH+5] = _T("");
	int iIndex = 0;
	LPTSTR pszData = NULL;
    size_t TotalSize= 0;

    if (szDir)
    {
        TotalSize = _tcslen(szDir);
    }

    if (szFile)
    {
        TotalSize += _tcslen(szFile);
    }

    TotalSize += _tcslen(BACKSLASH);


	// verify paramters will fit into buffer
	if (TotalSize > sizeof(szTemp))
	{
		return CB_ERR;
	}

	// insert prettied up name into combobox
	if(!szFile) {		
		return CB_ERR;
	}

	if (FAILED(StringCbCopy(szTemp, sizeof(szTemp), szFile)))
		return CB_ERR;

	MakeNameNice(szTemp);
	if(CB_ERR == (iIndex = ComboBox_AddString(m_hwndCB, szTemp)))
		return CB_ERR;

	szTemp[0] = 0;

	if(szDir) {
		StringCchCopy(szTemp, MAX_PATH+5, szDir);
		StringCchCat(szTemp, MAX_PATH+5, BACKSLASH);
	}

	StringCchCat(szTemp, MAX_PATH+5, szFile);
	pszData = MySzDup(szTemp); // freed in destructor

	// associate fullpath as data
	if(CB_ERR == ComboBox_SetItemData(m_hwndCB, iIndex, pszData)) {
		ASSERT(FALSE);
		MyFree(pszData);
		return CB_ERR;
	}
    return iIndex;
}	   

void CFileCombo::AddFileSpec(LPCTSTR pszDir, LPCTSTR szSpec)
{
	TCHAR szBuf[MAX_PATH];

	if (pszDir && szSpec)
	{
		if (SUCCEEDED(StringCbPrintf(szBuf, sizeof(szBuf),
		                             TEXT("%s\\%s"), pszDir, szSpec)))
		{
			WIN32_FIND_DATA fd;
			HANDLE h = FindFirstFile(szBuf, &fd);

			if (h && h!=INVALID_HANDLE_VALUE)
			{
				do
				{
					if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
						AddFile(pszDir, fd.cFileName);
				} while (FindNextFile(h, &fd));
			}

			FindClose(h);
		}
	}
}

// these should really be in a seperate CCombo base class, since they have nothing to do with files as such
int CFileCombo::InsertItem(LPCTSTR pszDisplay, LPCTSTR pszData, int iPos)
{
	int iIndex = ComboBox_InsertString(m_hwndCB, iPos, pszDisplay);
    if(CB_ERR != iIndex) {
    	ComboBox_SetItemData(m_hwndCB, iIndex, (DWORD)MySzDup(pszData)); // freed in destructor
    	ComboBox_SetCurSel(m_hwndCB, iIndex);
   	}
   	return iIndex;
}

int CFileCombo::AddItem(LPCTSTR pszDisplay, LPCTSTR pszData)
{
	int iIndex = ComboBox_AddString(m_hwndCB, pszDisplay);
    if(CB_ERR != iIndex) {
    	ComboBox_SetItemData(m_hwndCB, iIndex, (DWORD)MySzDup(pszData)); // freed in destructor
    	ComboBox_SetCurSel(m_hwndCB, iIndex);
   	}
   	return iIndex;
}

void CFileCombo::DeleteItem(int iSel)
{
	DWORD dwData = ComboBox_GetItemData(m_hwndCB, iSel);
	if(CB_ERR != dwData)
		MyFree((LPVOID)dwData);
    ComboBox_DeleteString(m_hwndCB, iSel);
}

int CFileCombo::FindByData(LPCTSTR pszFind)
{
	// find item by the data field. Note we have the string-compare the
	// data field, not just compare teh values of the pointers!!
	for(int i=0; ;i++)
	{
		LPCTSTR pszData = (LPCTSTR)ComboBox_GetItemData(m_hwndCB, i);
		if((int)pszData == CB_ERR)
			return CB_ERR;
		if(0==lstrcmpi(pszFind, pszData))
			return i;
	}
}

// find item by display name & select it. Add it if it's missing
int CFileCombo::SelectByDisplay(LPCTSTR pszDisplay, LPCTSTR pszData)
{
	int iIndex;
	if(CB_ERR == (iIndex = ComboBox_FindStringExact(m_hwndCB, -1, pszDisplay)))
	{
		iIndex = AddItem(pszDisplay, pszData);
	}
	// select it
	ComboBox_SetCurSel(m_hwndCB, iIndex);
	return iIndex;
}

// Tries to find the item in CB by the Data value & select it.
// If missing adds it
int CFileCombo::SelectByPath(LPCTSTR pszPath)
{
    if(!pszPath || !pszPath[0])
    	return CB_ERR;
    // try to find item by data
	int iIndex;
	if(CB_ERR == (iIndex = FindByData(pszPath)))
	{
		iIndex = AddFile(NULL, pszPath);
	}
	// select it
	ComboBox_SetCurSel(m_hwndCB, iIndex);
	return iIndex;
}

/////////////////////////////////////////////////////////////////////////////
// CFileTreeView
// TreeView helper class
/////////////////////////////////////////////////////////////////////////////

void CFileTreeView::ClearItem(HTREEITEM hti)
{
	// clear the root item
    TV_ITEM    tvi;
    tvi.mask = TVIF_PARAM;
   	tvi.hItem = hti;
   	tvi.lParam = NULL; // in case getitem call fails
    TreeView_GetItem(m_hwndTree, &tvi);
    MyFree((PVOID)tvi.lParam);
}

void CFileTreeView::ClearSubtree(HTREEITEM htiRoot)
{
	// clear the root item
	ClearItem(htiRoot);

   	// and recurse to all the children
    for(HTREEITEM hti=TreeView_GetChild(m_hwndTree, htiRoot); 
   			hti; hti = TreeView_GetNextSibling(m_hwndTree, hti))
   	{
		ClearSubtree(hti);    	
	}
}

void CFileTreeView::Clear()
{
	ClearSubtree(TreeView_GetRoot(m_hwndTree));
	SendMessage(m_hwndTree, WM_SETREDRAW, FALSE, 0L);
   	TreeView_DeleteAllItems(m_hwndTree);
   	SendMessage(m_hwndTree, WM_SETREDRAW, TRUE, 0L);
}

// Adds items to tree-view
HTREEITEM CFileTreeView::AddItem(HTREEITEM htiParent, LPCTSTR pszDisplay, LPCTSTR pszData, int iImage, HTREEITEM htiAfter)
{
    TV_INSERTSTRUCT ti;
   	ti.hParent = htiParent;
    ti.hInsertAfter = htiAfter;
   	ti.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    ti.item.iImage = ti.item.iSelectedImage = iImage;
   	ti.item.pszText = (LPTSTR)pszDisplay;
    ti.item.cchTextMax = lstrlen(pszDisplay);
   	ti.item.lParam = (DWORD)MySzDup(pszData);
    HTREEITEM hti = TreeView_InsertItem(m_hwndTree, &ti);
	ASSERT(hti);
	return hti;
}

void CFileTreeView::SetItemData(HTREEITEM hti, LPCTSTR pszData, int iImage)
{
	ClearItem(hti);
 		
  	TV_ITEM    tvi;
	tvi.mask = TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvi.hItem = hti;
	tvi.lParam = (DWORD)MySzDup(pszData);
	tvi.iImage = tvi.iSelectedImage = iImage;
	TreeView_SetItem(m_hwndTree, &tvi);
}

LPCTSTR CFileTreeView::GetItemDataText(HTREEITEM hti, PTSTR pszText, int cch)
{
   	TV_ITEM    tvi;
	tvi.mask = TVIF_PARAM;
	tvi.hItem = hti;
	tvi.lParam = NULL; // in case getitem call fails
	if(pszText)
	{
		pszText[0] = 0;
   	    tvi.mask |= TVIF_TEXT;
		tvi.pszText = pszText;
       	tvi.cchTextMax = cch;
    }
	TreeView_GetItem(m_hwndTree, &tvi);
	return (LPCTSTR)tvi.lParam;
}
