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

Abstract: externs for global data, CPL-specfic macros, prototypes for functions
**/

#ifndef _CPLGLOBL_H_
#define _CPLGLOBL_H_

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)             ((sizeof(x)) / (sizeof(x[0])))
#endif
//------------- Externs for global data (more below) ------------------------------
extern HINSTANCE	g_hInst;

//------------- Data structure defns for the CPL applets -----------------------

#define MAX_TABS	5

// This structure holds the static initialization data for each tab of each CPL
typedef struct 
{
    int		idsTitle;	// RCID of title of prop-page
    int		iddDlg;		// RCID of dlg
    //DLGPROC pfnDlg;	// dlg function
    LPCTSTR pszDlg;		// for componentization we use GetProcAddress to get our own Dlg procs!
    const int*	rgBoldStatics;	// array of ids of controls that need to be bolded
    int		iNumStatics;	// length of the above array
    BOOL	fSip;		// Does this tab want the SIP up or down
    LPCTSTR	pszHelp;	// help cmd string
} 
CPLTABINFO, *PCPLTABINFO;

// For componentization, we need to translate DlgProc names to fn ptrs 
// using GetProcAddress. If it's missing it means the component is not present
#define GETTABDLGPROC(iApplet, iTab) GetProcAddress(g_hInst, rgApplets[iApplet].rgptab[iTab]->pszDlg)

// This structure holds the static initialization data for each CPL.
// pfnLaunch was added to allow a specific applet to dynamically modify its
// tabs. This would be useful in situations where certain tabs may or may
// not be desired under certain run-time conditions. If it returns FALSE,
// the applet will not be displayed.
typedef struct CPLAPPLETINFO
{
    LPCTSTR pszMutex; // Mutex name (must not be localized)
    LPCTSTR pszLaunchCplCallback;   // optional initialization callback routine name
    BOOL	fPwdProtect;// whether applet is to be passwd protected
    int	rcidIcon;	// RCID of icon
    int 	idsName;	// RCID of name in CPL listview
    int 	idsDesc;	// RCID of desc in CPL listview
    int 	idsTitle;	// RCID of title of propsheet
    int     cctrlFlags; // classes to pass to InitCommctrlsEx or 0 if not used
    const CPLTABINFO* rgptab[MAX_TABS];
} 
CPLAPPLETINFO, *PCPLAPPLETINFO;

extern CPLAPPLETINFO rgApplets[];

// Applets in cplmain can optionally request to receive notifications when they
// are being launched.  This notification would give them an opportunity to
// do initialization before receiving windows messages -- for example, they might
// choose to add or remove tabs to their property sheet.  The notification callback
// routine must be exposed as a DLL entry point and have the following prototype.
typedef BOOL  (APIENTRY *LPFNLAUNCHCPLCALLBACK)(CPLAPPLETINFO*);

class CRunningTab
{
//private:
public:
	const CPLTABINFO*	  m_pTabData;
	const PROPSHEETPAGE*  m_psp;
    DLGPROC m_pfnDlg;	// dlg function ptr. For componentization we use GetProcAddress to get our own Dlg procs!
	int		m_iApplet;
	int		m_iTab;
	HWND	m_hwndSavedFocus;
	HWND	m_hwndSheet;	
	HFONT	m_hfontBold;
	CSipUpDown m_SipUpDown;
	LPCTSTR m_pszOldPasswd;  // *not* owned by this object, DO NOT free
	LONG	m_lParam;  // Can be used by the tab's DlgProc

public:
	CRunningTab(int iApplet, int iTab, PROPSHEETPAGE* psp, LPCTSTR pszOldPasswd) {
	    ZEROMEM(this);
		m_iApplet = iApplet;
		m_iTab = iTab;
		m_psp = psp;
		m_pTabData = rgApplets[iApplet].rgptab[iTab];
		m_pfnDlg = (DLGPROC)GETTABDLGPROC(iApplet, iTab);
		m_pszOldPasswd = pszOldPasswd; // *not* owned by this object, DO NOT free
		m_lParam = 0; // Just initialize to 0
	}		
	~CRunningTab() {
	    if(m_hfontBold) DeleteObject(m_hfontBold);
	}
};

// SIP fn ptr types
typedef BOOL (WINAPI* LPFNSIP)(SIPINFO*);
typedef DWORD (WINAPI* LPFNSIPSTATUS)();

//------------- Externs for global data ------------------------------

extern CPLAPPLETINFO rgApplets[];
extern HINSTANCE	g_hInst;
extern HWND			g_hwndWelcome;
extern BOOL			g_fNoDrag;
extern BOOL			g_fFullScreen;
extern BOOL			g_fRecenterForSIP;
extern BOOL			g_fRaiseLowerSIP;
extern HINSTANCE	g_hinstCoreDll;
extern LPFNSIP 		g_pSipGetInfo;
extern LPFNSIP 		g_pSipSetInfo;

//------------- Defines useful only in these CPLs --------------------

// Debug zones
#define ZONE_ERROR		DEBUGZONE(0)
#define ZONE_WARNING	DEBUGZONE(1)
#define ZONE_MAIN		DEBUGZONE(2)
#define ZONE_UTILS		DEBUGZONE(3)
#define ZONE_REG		DEBUGZONE(4)
#define ZONE_DATETIME	DEBUGZONE(5)
#define ZONE_NETWORK	DEBUGZONE(8)
#define ZONE_COMM		DEBUGZONE(9)
#define ZONE_KEYBD		DEBUGZONE(10)
#define ZONE_POWER		DEBUGZONE(11)
#define ZONE_SYSTEM		DEBUGZONE(12)
#define ZONE_STYLUS		DEBUGZONE(13)
#define ZONE_SOUNDS		DEBUGZONE(14)
#define ZONE_MSGS		DEBUGZONE(15)
#define ZONE_SCREEN     DEBUGZONE(16)
#define ZONE_REMOVE     DEBUGZONE(17)


//------------- Function prototypes ------------------------------
//
// from CPLMAIN.CPP
//
int GetCplInfo(int iApplet, NEWCPLINFO *pInfo);
LONG LaunchCpl(int iApplet, int iTab);
int CALLBACK CplSheetCallback(HWND hwndDlg,UINT uMsg, LPARAM lParam);
BOOL CALLBACK CplPageProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
//
// from CPLUTILS.CPP
//
void LoadRegGlobals(void);
HANDLE CheckPrevInstance(LPCTSTR pszMutexName, LPCTSTR pszTitle, BOOL fPwd);
void SetDeviceDependentText(HWND hDlg, int idc, int idsFmt);
BOOL SetWordCompletion(BOOL fOn);
BOOL CenterWindowSIPAware(HWND hwnd, BOOL fInitial=TRUE);
void DrawBitmapOnDc(DRAWITEMSTRUCT *lpdis, int nBitmapId, BOOL fDisabled);
void DrawCPLButton(DRAWITEMSTRUCT* lpdis, int idbBitmap);
LPTSTR PromptForPasswd(HWND hParent);
HFONT CreateBoldFont(HWND hDlg);
void SetTextWithEllipsis(HWND hwnd, LPCTSTR lpszText);
void LoadCB(HWND hwndCB, const COMBODATA rgCBData[], BOOL fData=TRUE);
HBRUSH WINAPI LoadDIBitmapBrush(LPCTSTR szFileName);
void InitImageList(HIMAGELIST& hImgList, BOOL fMini, int rgIcons[], int iNumIcons);
BOOL AygInitDialog( HWND hwnd, DWORD dwFlags );
BOOL AygAddSipprefControl( HWND hwnd );
BOOL LoadAygshellLibrary();
BOOL FreeAygshellLibrary();

// from OWNER.CPP
void GetOwnerInfo(OWNER_PROFILE* pProfile, OWNER_NOTES* pNotes);

// from UNLOADN.CPP
extern "C" int APIENTRY UninstallApplication(HINSTANCE hinst, HWND hwndParent, LPCTSTR pszApp);

//------------- Const data prototypes ------------------------------
//
// from CPLTABL.CPP
//
extern const CPLTABINFO CommRasTab;
extern const CPLTABINFO SndSchemeTab;
//extern const CPLTABINFO ColSchemeTab;
extern const LPCTSTR pszSaveColSchemeHelp;

#endif //_CPLGLOBL_H_


