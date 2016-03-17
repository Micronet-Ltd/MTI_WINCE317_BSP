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

Abstract: Main entry point to the CPL applets
**/

#include "cplpch.h"
#include <lass.h>
#include <lass_ae.h>

//
// Global data
//
HINSTANCE	g_hInst;
HWND		g_hwndWelcome;
// Customizations set via registry
BOOL		g_fNoDrag;
BOOL		g_fFullScreen;
BOOL		g_fRecenterForSIP;
BOOL		g_fRaiseLowerSIP;

// SIP fn ptrs
HINSTANCE	g_hinstCoreDll;
LPFNSIP 	g_pSipGetInfo;
LPFNSIP 	g_pSipSetInfo;

// Ayg data
HINSTANCE        g_hInstAygshell = NULL;
BOOL             g_bCalledSHInitExtraControls = FALSE;
BOOL             g_bLoadedAygshellLibrary = FALSE;
int              g_iAygshellRefCount = 0;
CRITICAL_SECTION g_csAygshell;

// The big const table of CPL data. #included here so we can get the table size in this file
#include "cpltable.cpp"


// Debug Zones.
#ifdef DEBUG
  DBGPARAM dpCurSettings = {
    TEXT("CPLMAIN.DLL"), {
    TEXT("Error"),TEXT("Warning"),TEXT("Main CPL"),TEXT("Utils"),
    TEXT("Registry"),TEXT("DateTime"),TEXT(""),TEXT(""),
    TEXT("Network"),TEXT("Comm"),TEXT(""),TEXT("Power"),
    TEXT(""),TEXT(""),TEXT(""),TEXT("Msgs") },
    0x7FFF
  }; 
#endif

extern "C" BOOL WINAPI DllEntry( HANDLE hInstDll, DWORD fdwReason, LPVOID lpvReserved )
{
	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			DEBUGREGISTER((HINSTANCE)hInstDll);
			g_hInst = (HINSTANCE)hInstDll;
			// check if we have a SIP & get ptrs to it if so
			g_hinstCoreDll = LoadLibrary(TEXT("coredll.dll"));
			if(g_hinstCoreDll) {
				g_pSipGetInfo = (LPFNSIP)GetProcAddress(g_hinstCoreDll, TEXT("SipGetInfo"));
				g_pSipSetInfo = (LPFNSIP)GetProcAddress(g_hinstCoreDll, TEXT("SipSetInfo"));
			} else {
				g_pSipGetInfo = g_pSipSetInfo = NULL;
			}

			// Load global registry-based settings
			LoadRegGlobals();
            InitializeCriticalSection( &g_csAygshell );

			break;

		case DLL_PROCESS_DETACH:
            DeleteCriticalSection( &g_csAygshell );
			break;
	}
	return TRUE;
}

extern "C" LONG CALLBACK CPlApplet(HWND hwndCPL, UINT uMsg, LONG lParam1, LONG lParam2)
{
	switch( uMsg ) {
		case CPL_INIT:
			return TRUE;

		case CPL_GETCOUNT:
			return ARRAYSIZEOF(rgApplets);
			break;

		case CPL_NEWINQUIRE:
			return GetCplInfo(lParam1, (NEWCPLINFO*)lParam2);

		case CPL_STARTWPARMS:
		case CPL_DBLCLK:
			int iApplet;
			iApplet = LOWORD(lParam1);
			if(iApplet<0 || iApplet>=ARRAYSIZEOF(rgApplets)) {
				ASSERT(FALSE);
				return FALSE;
			}
			HANDLE hMutex;
			// returns non-NULL if no prev instance was found
			hMutex = CheckPrevInstance(rgApplets[iApplet].pszMutex, CELOADSZ(rgApplets[iApplet].idsTitle), rgApplets[iApplet].fPwdProtect);
			if(hMutex)
			{
				LaunchCpl(iApplet, HIWORD(lParam1));
				if(hMutex != (HANDLE)-1)
					CloseHandle(hMutex);
			}
			return TRUE;

		case CPL_SELECT:
		case CPL_STOP:
		case CPL_EXIT:
		default:
			break;
	}
	return 0;
}

int GetCplInfo(int iApplet, NEWCPLINFO *pInfo)
{

	if(iApplet >= ARRAYSIZEOF(rgApplets)) {
		DEBUGMSG(ZONE_ERROR, (L"CPL: Wanted Applet %d, but we only have %d\r\n", iApplet, ARRAYSIZEOF(rgApplets)));
		return -1;
	}

	// Check if applet exists
	if(!GETTABDLGPROC(iApplet, 0)) {
		DEBUGMSG(ZONE_MAIN, (L"CPL: Wanted Applet %d, but component (%s) not present\r\n", iApplet, rgApplets[iApplet].pszMutex));
		return -1;
	}
	
	ZEROMEM(pInfo);
	pInfo->dwSize = sizeof(NEWCPLINFO);
	pInfo->lData = rgApplets[iApplet].rcidIcon;
	pInfo->hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(rgApplets[iApplet].rcidIcon));
	LOADSZ(rgApplets[iApplet].idsName, pInfo->szName);
	LOADSZ(rgApplets[iApplet].idsDesc, pInfo->szInfo);
	return 0;
}

LONG LaunchCpl(int iApplet, int iSelectTab)
{
    PROPSHEETPAGE* rgPropPages;
    PROPSHEETHEADER PropHdr;
    int nPages, i, j;
    BOOL fRet;
    LPTSTR pszPasswd = NULL;
    
    // Check if applet exists
    if(!GETTABDLGPROC(iApplet, 0)) {
        DEBUGMSG(ZONE_ERROR, (L"CPL: Launched Applet %d, but component (%s) not present\r\n", iApplet, rgApplets[iApplet].pszMutex));
        return FALSE;
    }
    
    // Find out if the Welcome app is running. If so, we do special things later
    if((g_hwndWelcome = FindWindow(WELCOME_CLASS, NULL))) {
        DEBUGMSG(ZONE_MAIN, (L"CPL: WELCOME APP is running: hwndWelcome=%x\r\n", g_hwndWelcome));
    }
    
    // Allow the CPLTABINFO to be modified, perhaps to add or remove tabs.
    if (rgApplets[iApplet].pszLaunchCplCallback) {
        // get a pointer to the callback routine
        LPFNLAUNCHCPLCALLBACK pfnLaunchCpl = 
            (LPFNLAUNCHCPLCALLBACK) GetProcAddress(g_hInst, rgApplets[iApplet].pszLaunchCplCallback);
        if(pfnLaunchCpl == NULL) {
            DEBUGMSG(ZONE_ERROR|ZONE_MAIN, (L"CPL: Can't find launch callback '%s'--exiting propsheet\r\n",
                rgApplets[iApplet].pszLaunchCplCallback));
            return FALSE;
        }
        
        // If the function fails, don't open the propsheet
        if (!(*pfnLaunchCpl)(&rgApplets[iApplet])) {
            DEBUGMSG(ZONE_ERROR|ZONE_MAIN, (L"CPL: Problem modifying the CPLTABINFO--exiting propsheet\r\n"));
            return FALSE;
        }
    }
    
    // check if we need to prompt for password
    if(rgApplets[iApplet].fPwdProtect)
    {
        // if we fail to Authenticate
        const HWND hParentWindow=NULL;
#ifndef USE_LASS_AUTH
        if(!(pszPasswd = PromptForPasswd(hParentWindow)))
#else
        if(!(VerifyUser(&AE_CPLMAIN,NULL,hParentWindow,VU_UNTIL_SUCCESS,0)))
#endif 
        {
            DEBUGMSG(ZONE_ERROR|ZONE_MAIN, (L"Failed to get good password--exiting propsheet\r\n"));
            
            return FALSE;
        }
    }  		
    
    // check to see if we need to load any common controls
    if(rgApplets[iApplet].cctrlFlags != 0)
    {
        INITCOMMONCONTROLSEX    icex;
        
        // Initialize the common controls...
        icex.dwSize = sizeof(icex);
        icex.dwICC  = rgApplets[iApplet].cctrlFlags;
        InitCommonControlsEx(&icex);
    }
    
    // special handling for "externl" CPLs, i.e. the Dialing CPL
    // if the title of the first tab is NULL then we assume it is external
    if(rgApplets[iApplet].rgptab[0]->idsTitle==NULL)
    {
        DEBUGMSG(ZONE_MAIN, (L"External CPL\r\n"));
        // cast the "dlgproc" ptr to a FARPROC (no args) and call it
        //Prefix
        MyFree(pszPasswd);
        return ((FARPROC)GETTABDLGPROC(iApplet, 0))();
    }
    
    // Count the pages
    for(nPages=0;  nPages<MAX_TABS && rgApplets[iApplet].rgptab[nPages]; nPages++)
        ;
    // alloc the page array
    rgPropPages = MyRgAlloc(PROPSHEETPAGE, nPages); // freed at end of this fn
    
    if(!rgPropPages) {
        MyFree(pszPasswd);
        return FALSE;
    }
    
    // populate the page array. i iterates through rgPropPages, 
    // j iterates through rgApplets[iApplet].rgptab. Usually they are
    // identical, but if a tab is missing due to componentization
    // then j may skip forward while i remains the same
    for(i=0, j=0; j<nPages; j++)
    {
        DEBUGCHK(i<=j);
        const CPLTABINFO* pCurrTab = rgApplets[iApplet].rgptab[j];
        // create a CRunningTab object as our working space for this page
        // allow indiv tabs to be omited (used by Backlight tab) if their wndproc is missing
        // Only works for tabs at the end of the sequence, though
        CRunningTab* pRunningTab = new CRunningTab(iApplet, j, &(rgPropPages[i]), pszPasswd); // freed at end of this fn
        //Prefix
        if(pRunningTab && !pRunningTab->m_pfnDlg) {
            DEBUGMSG(ZONE_MAIN | ZONE_ERROR, (L"Applet %d, Tab %d missing\r\n", iApplet, j));
            // free the object & continue at head of loop, trying next tab
            delete pRunningTab;
            continue;
        }
        
        rgPropPages[i].dwSize = sizeof(PROPSHEETPAGE);
        rgPropPages[i].dwFlags = PSP_USETITLE;
        rgPropPages[i].hInstance = g_hInst;
        rgPropPages[i].pszTemplate = MAKEINTRESOURCE(pCurrTab->iddDlg);
        rgPropPages[i].pszIcon = NULL;
        rgPropPages[i].pfnDlgProc = CplPageProc;
        rgPropPages[i].pszTitle = CELOADSZ(pCurrTab->idsTitle);
        rgPropPages[i].pfnCallback = NULL;
        rgPropPages[i].lParam = (LPARAM)pRunningTab; // freed at end of this fn
        i++;
    }
    // correct the number of pages & start tab if neccesary
    nPages = i;
    if(iSelectTab >= nPages) 
        iSelectTab=0;			
    
    PropHdr.dwSize     = sizeof(PROPSHEETHEADER);
    PropHdr.dwFlags    = PSH_PROPSHEETPAGE | PSH_USECALLBACK | PSH_USEICONID;
    if(GetFileAttributes(PEGHELP_EXE) != -1)
        PropHdr.dwFlags |= PSH_HASHELP;
    if(g_fFullScreen)
        PropHdr.dwFlags    = PropHdr.dwFlags | PSH_MAXIMIZE;
    PropHdr.hwndParent = NULL;
    PropHdr.hInstance  = g_hInst;
    PropHdr.pszIcon    = MAKEINTRESOURCE(rgApplets[iApplet].rcidIcon);
    PropHdr.pszCaption = CELOADSZ(rgApplets[iApplet].idsTitle);
    PropHdr.nPages     = nPages;
    PropHdr.nStartPage = iSelectTab;
    PropHdr.ppsp       = rgPropPages;
    PropHdr.pfnCallback = CplSheetCallback;
    
    LoadAygshellLibrary();
    fRet = PropertySheet(&PropHdr);
    
    // free our CRunningTab objects
    for(i=0;  i<nPages; i++) 
    {
        DEBUGCHK(rgPropPages[i].lParam);
        delete ((CRunningTab*)(rgPropPages[i].lParam));
    }
    MyFree(rgPropPages);
    MyFree(pszPasswd);

    FreeAygshellLibrary();
    return fRet;
}

int CALLBACK CplSheetCallback(
   HWND hwndDlg,  // handle to the property sheet dialog box
   UINT uMsg,     // message identifier
   LPARAM lParam  // message parameter
   )
{
	CSipUpDown* pSipHelper = NULL;
	
	DEBUGMSG(ZONE_MSGS, (L"CplSheetCallback(0x%X, 0x%X, 0x%X)\r\n", hwndDlg, uMsg, lParam));
	switch(uMsg)
	{
	  case PSCB_PRECREATE:
	  	DEBUGMSG(ZONE_MAIN, (L"PSCB_PRECREATE: turn off ws_popup\r\n"));
	  	((LPDLGTEMPLATE)lParam)->style &= ~WS_POPUP;
		break;

	  case PSCB_INITIALIZED:
		ASSERT(hwndDlg);
		// can't center yet --sheet hasn't even been sized correctly
		// if(!g_fFullScreen)
	    //	CenterWindowSIPAware(hwndDlg);
	    if(g_fNoDrag) {
        	DWORD dwExStyle = GetWindowLong(hwndDlg, GWL_EXSTYLE);
	        dwExStyle |= WS_EX_NODRAG;
    	    SetWindowLong(hwndDlg, GWL_EXSTYLE, dwExStyle);
		}
		// create a SIP-helper class & save current Sip state in it
		if(g_fRaiseLowerSIP) {
			if(pSipHelper = new CSipUpDown) {
					pSipHelper->SaveSipState();
			}
		}
		SetWindowLong(hwndDlg, GWL_USERDATA, (DWORD)pSipHelper); // GWL_USERDATA of *sheet* is a CSipUpDown object
		//Prefix
		delete pSipHelper;

		break;
	}
	return 0;
}   

//
// This is the WndProc for all tabs of all CPLs. All the msgs first come
// here & are then dispatched
//
BOOL CALLBACK CplPageProc(HWND hwndPage, UINT message, WPARAM wParam, LPARAM lParam)
{
  CRunningTab* pCurrTab = (CRunningTab*)GetWindowLong(hwndPage, GWL_USERDATA);  // GWL_USERDATA of *tab* is a CRunningTab object
  BOOL fRet = FALSE;

#ifdef DEBUG
  if(pCurrTab) {
	  DEBUGMSG(ZONE_MSGS, (L"+CplPageProc(%d %d 0x%X, 0x%X, 0x%X, 0x%X)\r\n", 
  					pCurrTab->m_iApplet, pCurrTab->m_iTab, hwndPage, message, wParam, lParam));
  }
#endif



  switch (message)
  {
  	case WM_INITDIALOG:
  		// lParam is the PROPSHEETPAGE struct. Get its lParam field
  		// which is a ptr to our CRunningTab object
  		pCurrTab = (CRunningTab*)(((PROPSHEETPAGE*)lParam)->lParam);
  		SetWindowLong(hwndPage, GWL_USERDATA, (DWORD)pCurrTab); // GWL_USERDATA of *tab* is a CRunningTab object

		pCurrTab->m_hwndSheet = GetParent(hwndPage);
		if (NULL != pCurrTab)
		{
			// Create Bold font if we need it & bold the listed statics 
  			if(pCurrTab->m_pTabData->rgBoldStatics)
  			{
				if(pCurrTab->m_hfontBold = CreateBoldFont(hwndPage))
				{
					// set up bold fonts statics	
					for (int i = 0; i < pCurrTab->m_pTabData->iNumStatics; i++) {
						SendDlgItemMessage(hwndPage, pCurrTab->m_pTabData->rgBoldStatics[i], WM_SETFONT, (WPARAM)pCurrTab->m_hfontBold, 0L);
					}
				}
			}
			// special case for dialogs that use the password (password & owner)--pass it into the dlgproc
			if(pCurrTab->m_pszOldPasswd)
				lParam = (LPARAM)pCurrTab->m_pszOldPasswd;
		}else{
			DEBUGMSG(ZONE_ERROR, (L"CplPageProc GetParent failed"));
		}
		break;

	case WM_ACTIVATE:
		if (NULL == pCurrTab)
		{
			DEBUGMSG(ZONE_ERROR, (L"CplPageProc Current tab is null during WM_ACTIVATE."));
			return false;
		}
		// save & restore the focus within our dlg
		if(LOWORD(wParam) == WA_INACTIVE)
		{
			pCurrTab->m_hwndSavedFocus = GetFocus();
		}
		else
		{
			// set SIP state to what we want
			if(g_fRaiseLowerSIP)
				RaiseLowerSip(pCurrTab->m_pTabData->fSip);

			if(pCurrTab->m_hwndSavedFocus)
				SetFocus(pCurrTab->m_hwndSavedFocus);
			else
				pCurrTab->m_hwndSavedFocus = GetFocus();
			
			// If Welcome launched this CPL, then Welcome also needs to get an activate
			if(g_hwndWelcome)
				PostMessage(g_hwndWelcome, WM_USER+2, wParam, lParam);
		}
		fRet = TRUE;
		break;
	
	case WM_NOTIFY:
	{
		if (NULL == pCurrTab)
		{
			DEBUGMSG(ZONE_ERROR, (L"CplPageProc Current tab is null during WM_NOTIFY."));
			return false;
		}
	  DEBUGMSG(ZONE_MSGS, (L"CplPageProc: WM_NOTIFY(Applet=%d Tab=%d Code=%d)\r\n", 
  					pCurrTab->m_iApplet, pCurrTab->m_iTab, ((NMHDR*)lParam)->code));
	  switch(((NMHDR*)lParam)->code)
	  {
		case PSN_HELP:
			MyCreateProcess(TEXT("peghelp"), pCurrTab->m_pTabData->pszHelp);
			fRet = TRUE;
			break;

		case PSN_APPLY:
			// send an IDOK to the actual page dlgproc
			//DebugBreak();
			ASSERT(pCurrTab->m_pfnDlg);
			if(!pCurrTab->m_pfnDlg(hwndPage, WM_COMMAND, IDOK, 0)) {
				// something went wrong. refuse to close the propsheet
				SetWindowLong(hwndPage, DWL_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
			} else {
				SetWindowLong(hwndPage, DWL_MSGRESULT, PSNRET_NOERROR);
			}
			return TRUE; // don't fall thru to regular fwding

		case PSN_RESET:
			// send an IDCANCEL to the actual page dlgproc
			ASSERT(pCurrTab->m_pfnDlg);
			pCurrTab->m_pfnDlg(hwndPage, WM_COMMAND, IDCANCEL, 0);
			return TRUE; // don't fall thru to regular fwding

		  case PSN_SETACTIVE:
		  	if(g_fRaiseLowerSIP) {
				// set SIP state to what we want
				RaiseLowerSip(pCurrTab->m_pTabData->fSip);
				fRet = TRUE;
			}
			break;
	  }
	}
	
	case WM_SETTINGCHANGE:
	  if(!g_fFullScreen) {
	  	// SPI_SETWORKAREA==change in taskbar area, SPI_SETSIPINFO==change in SIP state
		if(wParam==SPI_SETWORKAREA) {
			CenterWindowSIPAware(GetParent(hwndPage));
		} else if(wParam==SPI_SETSIPINFO && g_fRecenterForSIP) {
			// 2nd param==FALSE makes it do nothing if SIP just went *down* to prevent unneccsaery bouncing
			CenterWindowSIPAware(GetParent(hwndPage), FALSE);
			fRet = TRUE;
		}
	  }
	  break;
		
	case WM_DESTROY:
		if (NULL == pCurrTab)
		{
			DEBUGMSG(ZONE_ERROR, (L"CplPageProc Current tab is null during WM_DESTROY."));
			return false;
		}
	  if(g_fRaiseLowerSIP) {
		// Restore SIP state to what it was when this CPL *sheet* was created (not just this tab)
		CSipUpDown *pSipHelper = (CSipUpDown*)GetWindowLong(pCurrTab->m_hwndSheet, GWL_USERDATA);  // GWL_USERDATA of *sheet* is a CSipUpDown object
	  	pSipHelper->RestoreSipState();
		fRet = TRUE;
	  }
	  break;
	  
	case WM_CLOSE:
		if (NULL == pCurrTab)
		{
			DEBUGMSG(ZONE_ERROR, (L"CplPageProc Current tab is null during WM_CLOSE."));
			return false;
		}
	    SendMessage(pCurrTab->m_hwndSheet, PSM_PRESSBUTTON, IDCANCEL, 0);
	    return TRUE;

	case WM_NEXTDLGCTL:
		if (NULL == pCurrTab)
		{
			DEBUGMSG(ZONE_ERROR, (L"CplPageProc Current tab is null during WM_NEXTDLGCTL."));
			return false;
		}
	    return SendMessage(pCurrTab->m_hwndSheet,message,wParam,lParam);
  }
  if(pCurrTab)
  {
	ASSERT(pCurrTab->m_pfnDlg);
  	fRet |= pCurrTab->m_pfnDlg(hwndPage, message, wParam, lParam);
  }
  return fRet;
}
