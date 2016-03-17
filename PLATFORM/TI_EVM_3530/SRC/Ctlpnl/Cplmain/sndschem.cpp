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

Abstract: Code specific to the Sound schemes tab of the Sounds CPL
**/

#include "cplpch.h"
#include <commdlg.h>

BOOL CALLBACK SaveSndSchemeDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LONG lParam);
BOOL CreateUniqueSchemeName(LPCTSTR pszDisplay, LPTSTR pszKey, int iMaxLen);
extern "C" BOOL APIENTRY SndSchemeDlgProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

#define MAXSCHEME  37      //max length of scheme name
#define KEYLEN 		8

// this table maps events to friendly names for display. This table order & size 
// MUST match that in audiotbl.h in WAVEAPI.DLL. Don't change it.
#define NUM_SOUND_EVENTS   21

const UINT idsSoundEvents[NUM_SOUND_EVENTS] = 
{
IDS_SND_ASTERISK,
IDS_SND_CLOSEAPPS,
IDS_SND_CRITICAL,
IDS_SND_DEFAULTSND,
IDS_SND_EMPTYRECYC,
IDS_SND_EXCLAMATION,
IDS_SND_IRDABEGIN,
IDS_SND_IRDAEND,
IDS_SND_IRDAINTERRUPT,
IDS_SND_MENUPOPUP,
IDS_SND_MENUSELECT,
IDS_SND_OPENAPPS,
IDS_SND_QUESTION,
IDS_SND_REMNETSTART,
IDS_SND_REMNETEND,
IDS_SND_REMNETINT,
IDS_SND_STARTUP,
IDS_SND_MAXWIN,
IDS_SND_MINWIN,
IDS_SND_RECSTART,
IDS_SND_RECEND,
};

const int rgSndIcons[] = {IDI_PROGRAM, IDI_AUDIO, IDI_BLANK };
const int rgDisableControls[] = { IDC_SAVE, IDC_DELETE, IDC_TEST, IDC_STOP, IDC_BROWSE, IDC_COMBO_SOUND };

class CSoundScheme
{
friend BOOL CALLBACK SaveSndSchemeDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LONG lParam);
friend BOOL APIENTRY SndSchemeDlgProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

private:
	HWND		m_hDlg;
	HWND		m_hwndScheme, m_hwndSound, m_hwndTree;
	HIMAGELIST	m_hImgList;
	TCHAR		m_szCurDir[MAX_PATH];
	HTREEITEM	m_htiCur;

	// mask out some events if so configured
	DWORD		m_dwEventMask;

	CFileCombo*		m_pSoundCombo;
	CFileCombo*		m_pSchemeCombo;
	CFileTreeView* 	m_pSoundTree;

	BOOL		m_fChangedEventMapping;

	// temporary buffer used to get SaveAs scheme name
	TCHAR		m_szCurScheme[MAX_PATH];

	// Do we need to notify the Wave driver
	BOOL		m_fUpdate;

	// we load & cache all the sound event names from the RC file
	PCTSTR 		m_cszSoundEvents[NUM_SOUND_EVENTS];

public:
	// dialog init & shutdown
	CSoundScheme(HWND hDlg);
	~CSoundScheme();

	// init the treeview, handle tree-view selection change
	BOOL SelectScheme(int iSel, LPCTSTR pszKey=NULL, BOOL fRevertToDefault=TRUE); // select new scheme into TV & set button states approp
	BOOL LoadSchemeIntoTree(LPCTSTR pszSchemeReg); // init treeview from registry defn of a scheme
	BOOL FindWAVPath(LPCTSTR pszFileName, LPTSTR pszPath, int iLen); // convert WAV name to full path
	void OnTVSelChange(TV_ITEM tvi);	 // match combo selection & button states to treeview selection
	void SelectSndFile(LPCTSTR pszFile); // set button state to match combo & tree selection
	void OnSndCBSelCancel();			 // restore CB setting


	// match sounds to events
	void ChooseSound();			// sound selected in combo is assigned to event selected in tree
	void BrowseForWAV();		// browse button pressed
	void PlayCurrentEventSnd();	// play button pressed

	// choose, create, delete, save schemes
	void ChooseScheme();			// select scheme in scheme combo
	void OnDelete();			    // delete scheme selected in combo
	void OnSave();				    // On "Save As" button
	void AddNewScheme(LPCTSTR pszDisplay); // Create new scheme in registry & combo box
	void SetSchemeAsDefault(LPCTSTR pszKey);	
	void SaveSettings();			// save selected scheme as current (on OK)
	void DeleteSchemeFromRegAndCombo(int iIndex); 
	void SaveNewScheme(LPCTSTR lpszDisplay, LPCTSTR lpszRegName); // save event mappings to registry
};

CSoundScheme::~CSoundScheme() 
{
	delete m_pSoundCombo;
	delete m_pSchemeCombo;
	delete m_pSoundTree;
	//DebugBreak();

	// can't delete the image list until the treeview is destroyed, but for some reason we
	// get the WM_DESTROY before our child windows. So explicitly destroy the treeview first
	DestroyWindow(GetDlgItem(m_hDlg, IDC_EVENT_TREE));
	
	if (m_hImgList)
    	ImageList_Destroy(m_hImgList);
}


// Init dialog by loading sound schemes
CSoundScheme::CSoundScheme(HWND hDlg) 
{
	ZEROMEM(this);
	m_hDlg = hDlg;

	// load sound-event names from RC file
	for(int i=0; i<NUM_SOUND_EVENTS; i++)
		m_cszSoundEvents[i] = CELOADSZ(idsSoundEvents[i]);
	
	m_hwndScheme = DI(IDC_COMBO_SCHEME);
    m_hwndSound = DI(IDC_COMBO_SOUND);        
    m_hwndTree = DI(IDC_EVENT_TREE);        

	m_pSoundCombo = new CFileCombo(DI(IDC_COMBO_SOUND));
	m_pSchemeCombo = new CFileCombo(DI(IDC_COMBO_SCHEME));
	m_pSoundTree = new CFileTreeView(DI(IDC_EVENT_TREE));

    // ComboBox_SetExtendedUI(m_hwndSound, TRUE);
    // ComboBox_SetExtendedUI(m_hwndScheme, TRUE);

	InitImageList(m_hImgList, TRUE, (int*)rgSndIcons, ARRAYSIZEOF(rgSndIcons));
    TreeView_SetImageList(m_hwndTree, m_hImgList, TVSIL_NORMAL);

	// disable stuff
	EnableDisable(m_hDlg, rgDisableControls, ARRAYSIZEOF(rgDisableControls), FALSE);

	// populate sound combo with all .wav files in \windows
	lstrcpy(m_szCurDir, PATH_WINDOWS);
	m_pSoundCombo->AddFileSpec(m_szCurDir, TEXT("*.wav"));
	// add the (None) entry
    m_pSoundCombo->InsertItem(CELOADSZ(IDS_NONE), NULL, 0);

	// now fill up with all Schemes from registry
	TCHAR szName[MAX_PATH+1];
	TCHAR szValue[MAX_PATH+1];
	//DebugBreak();
	
	CReg reg1(HKEY_LOCAL_MACHINE, RK_SND_SCHEME);
	while(reg1.EnumValue(szName, MAX_PATH, szValue, MAX_PATH))
	{
		// szValue is the display name of the scheme
		// szName is the registry VaueName, which is saved as the datum behind the combo entry
		m_pSchemeCombo->AddItem(szValue, szName);
	}

	// get the currently selected scheme
	CReg reg2(HKEY_LOCAL_MACHINE, RK_SND_EVENT);
	LPCTSTR pszCurrScheme = reg2.ValueSZ(RV_SCHEME);

	// load a mask from the registry that allows us to hide some events
	if(!(m_dwEventMask = reg2.ValueDW(RV_EVENTMASK)))
		m_dwEventMask = (DWORD)(-1);
	
	// select the current scheme & load into treeview
	SelectScheme(CB_ERR, pszCurrScheme);
}

// selects Scheme in combo, loads corresponding events from registry 
// into TreeView & sets change & current-scheme flags 
// can select by CB index or by registry value. 
// If a failure occurs tries to reverts to "Windows CE Default" or 
// if all else fails a null scheme
BOOL CSoundScheme::SelectScheme(int iSel, LPCTSTR pszKey, BOOL fRevertToDefault)
{
	// if one of teh args is missing, fill it in using the other
	if(!pszKey && iSel!=CB_ERR) {
		pszKey = (LPCTSTR)ComboBox_GetItemData(m_hwndScheme, iSel);
	}
	else if(pszKey && iSel==CB_ERR) {
		iSel = m_pSchemeCombo->FindByData(pszKey);
	}

	// every time we reload the tree, any previous event-mapping changes are 
	// discarded, so reset this flag
	m_fChangedEventMapping = FALSE;
	
    if(pszKey && (CB_ERR!=iSel) && LoadSchemeIntoTree(pszKey))
    {
        EnableWindow(GetDlgItem(m_hDlg, IDC_SAVE), TRUE);

			// enable delete button if current sel is NOT "Windows CE Default" or "No Sounds"
			BOOL fDelete = ((CSTR_EQUAL != CompareString(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			                                             NORM_IGNORECASE, pszKey, -1, REGVALUE_DEFAULTSOUNDS, -1)) &&
			                (CSTR_EQUAL != CompareString(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), 
			                                             NORM_IGNORECASE, pszKey, -1, REGVALUE_NOSOUNDS, -1)) &&
			                (CSTR_EQUAL != CompareString(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), 
			                                             NORM_IGNORECASE, pszKey, -1, REGVALUE_ALLSOUNDS, -1)));

		EnableWindow(GetDlgItem(m_hDlg, IDC_DELETE), fDelete);

		// select it in CB in case not already selected
		ComboBox_SetCurSel(m_hwndScheme, iSel);  
		
		DEBUGMSG(ZONE_SOUNDS, (L"Loaded scheme %s %d OK\r\n", pszKey, iSel));
		return TRUE;
    }
    else
    {
		DEBUGMSG(ZONE_SOUNDS, (L"Failed to load scheme %s %d\r\n", pszKey, iSel));
    	// we failed to load the selected scheme. If we're not already recursed (trying to get
    	// the default) then try to find & set to the default scheme
	    if(fRevertToDefault)
		{
			// recurse on ourselves to load "Default", but pass FALSE
			// for 2nd param so we don't recurse any further. Return immediately
			// because we have already handled the error by now
			return SelectScheme(CB_ERR, REGVALUE_DEFAULTSOUNDS, FALSE);
		}
		// in all error cases we get here, the all-else-fails code
    	ASSERT(FALSE);
	    m_pSoundTree->Clear();
        ComboBox_SetCurSel(m_hwndScheme, CB_ERR);
		EnableWindow(GetDlgItem(m_hDlg, IDC_DELETE), FALSE);
        EnableWindow(GetDlgItem(m_hDlg, IDC_SAVE), FALSE);
		return FALSE;
	}
}

// load the sound events for a scheme into the tree view
BOOL CSoundScheme::LoadSchemeIntoTree(LPCTSTR pszSchemeReg)
{
	// clean out the tree first
    m_pSoundTree->Clear();

    SendMessage(m_hwndTree, WM_SETREDRAW, FALSE, 0L);

	// Add the base item
    HTREEITEM htiBase = m_pSoundTree->AddItem(TVI_ROOT, CELOADSZ(IDS_WINDOWSCE), NULL, 0, TVI_ROOT);

	// read the MULTI_SZ string for the current scheme
	CReg reg(HKEY_LOCAL_MACHINE, RK_SND_EVENT);
	LPCTSTR pszSchemeData;

	if(!(pszSchemeData = (LPCTSTR)reg.ValueBinary(pszSchemeReg)))
		return FALSE;
		
	// fill up with all event names from RC file & corresponding WAVs from 
	// scheme in registry. The scheme is a MULTI_SZ with each string being 
	// a WAV file for the corresponding event. If the string is ' ' (space)
	// this means NO WAV for that event.
	// Each string in the MULTI_SZ is seperated by NULL, with 2 nulls terminating the whole 
	LPCTSTR pszWAV;
	int j;
	DWORD dwMaskCheck;
	for (j=0, dwMaskCheck=1, pszWAV=pszSchemeData; j<NUM_SOUND_EVENTS; j++, dwMaskCheck<<=1, pszWAV+=(lstrlen(pszWAV)+1))
	{
		TCHAR szPath[MAX_PATH];
		ASSERT(pszWAV);

		if(dwMaskCheck & m_dwEventMask)
		{
			// convert WAV name to full path if neccesary
			if(pszWAV[0]==0 || pszWAV[0]==' ' || !FindWAVPath(pszWAV, szPath, CCHSIZEOF(szPath)))
			{
				m_pSoundTree->AddItem(htiBase, m_cszSoundEvents[j], NULL, 2, TVI_SORT);
			}
			else
			{
				m_pSoundTree->AddItem(htiBase, m_cszSoundEvents[j], szPath, 1, TVI_SORT);
			}
		}
		else DEBUGMSG(ZONE_SOUNDS, (L"SKIPPING SOUND EVENT %s (Mask=0x%08x)\r\n", m_cszSoundEvents[j], m_dwEventMask));
	}
	TreeView_Expand(m_hwndTree, htiBase, TVE_EXPAND);
    SendMessage(m_hwndTree, WM_VSCROLL, (WPARAM)SB_TOP, 0L);    
    SendMessage(m_hwndTree, WM_SETREDRAW, TRUE, 0L);
    return TRUE;
}

BOOL CSoundScheme::FindWAVPath(LPCTSTR pszFileName, LPTSTR pszPath, int iLen)
{
	BOOL fRet = FALSE;
	pszPath[0] = 0;
	if(!pszFileName || pszFileName[0] == ' ')
		goto done;

	if(iLen < (int)(1+lstrlen(PATH_WINDOWS_SLASH)+lstrlen(pszFileName)+lstrlen(TEXT(".wav"))))
		goto done;

	// the input is either a fully qualified path or just a basename w/o path
	// in which case window dir is assumed. .wav extn may be missing in eiterh case
	if(pszFileName[0] == '\\') {
		lstrcpy(pszPath, pszFileName);
	} else {
		lstrcpy(pszPath, PATH_WINDOWS_SLASH);
    	lstrcat(pszPath, pszFileName);
	}
	// add '.wav' extension if missing
    if (pszPath[wcslen(pszPath) - 4] != '.')
        lstrcat(pszPath, TEXT(".wav")); 
    
	fRet = GetFileAttributes(pszPath)!=(-1);
done:
	DEBUGMSG(ZONE_SOUNDS, (L"FindWAVPath(%s)==>(%s) %d\r\n", pszFileName, pszPath, fRet));
	return fRet;
}

void CSoundScheme::OnTVSelChange(TV_ITEM tvi)
{
	m_htiCur = tvi.hItem;
	SelectSndFile((LPCTSTR)tvi.lParam);
	
	// enable these two whever an actual event is selected
	BOOL fEnable = tvi.hItem != TreeView_GetRoot(m_hwndTree);
	EnableWindow(GetDlgItem(m_hDlg, IDC_COMBO_SOUND), fEnable);
	EnableWindow(GetDlgItem(m_hDlg, IDC_BROWSE), fEnable);
}

void CSoundScheme::SelectSndFile(LPCTSTR pszFile)
{
	if(!pszFile)
	{
		// no sound mapping. Select (None) in combo box
       	EnableWindow(GetDlgItem(m_hDlg, IDC_TEST), FALSE);
        EnableWindow(GetDlgItem(m_hDlg, IDC_STOP), FALSE);
        
		int iSel = ComboBox_FindStringExact(m_hwndSound, -1, CELOADSZ(IDS_NONE));
		ASSERT(iSel >= 0);
		ComboBox_SetCurSel(m_hwndSound, iSel);
	}
	else 
	{
		// valid sound mapping
        EnableWindow(GetDlgItem(m_hDlg, IDC_TEST), TRUE);
        EnableWindow(GetDlgItem(m_hDlg, IDC_STOP), TRUE);

        // find & select in combo. Add if missing
		m_pSoundCombo->SelectByPath(pszFile);
    }
}

void CSoundScheme::ChooseSound()
{
	int iSel = ComboBox_GetCurSel(m_hwndSound); 
	if (iSel == CB_ERR) {
		return;
	}

	LPCTSTR pszSnd = (LPCTSTR)ComboBox_GetItemData(m_hwndSound, iSel);
	m_pSoundTree->SetItemData(m_htiCur, pszSnd, (pszSnd ? 1 : 2));
	m_fChangedEventMapping = TRUE;
	
	SelectSndFile(pszSnd);
	// Don't set focus to the play button, this will break the ability to scroll
	// through the list of sounds.
	//SetFocus(GetDlgItem(m_hDlg, (pszSnd ? IDC_TEST : IDC_COMBO_SOUND)));
}

void CSoundScheme::BrowseForWAV()
{
	OPENFILENAME ofn;
	TCHAR szPath[MAX_PATH+2];
	TCHAR szFilter[64];

	// Load filter from RES file & convert \1 to \0
    szFilter[0] = '\0';
    int iFilterLen = LOADSZ(IDS_WAVFILES, szFilter);
	for(int i=0; i<iFilterLen; i++) {
		if(TEXT('\1') == szFilter[i] ) {
			szFilter[i] = TEXT('\0');
		}
	} 

	szPath[0] = 0;
	ZEROMEM(&ofn);
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = m_hDlg;
    ofn.hInstance = g_hInst;
    ofn.lpstrFilter = szFilter;
    ofn.nFilterIndex = 1;
	ofn.lpstrFile = szPath; 			 
	ofn.nMaxFile = CCHSIZEOF(szPath);		
	if(L'\\' == m_szCurDir[0])
		ofn.lpstrInitialDir = m_szCurDir;

    ofn.lpstrTitle = CELOADSZ(IDS_BROWSE); 
    ofn.Flags =  OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = TEXT("wav");

	if (GetOpenFileName(&ofn))
	{
		m_pSoundTree->SetItemData(m_htiCur, ofn.lpstrFile, 1);
		m_fChangedEventMapping = TRUE;

		// change current dir
        if(ofn.nFileOffset && (ofn.nFileOffset < MAX_PATH))
        {
	    	lstrcpy(m_szCurDir, ofn.lpstrFile);
    	    m_szCurDir[ofn.nFileOffset] = 0;
    	    if(m_szCurDir[ofn.nFileOffset-1]=='\\')
	    		m_szCurDir[ofn.nFileOffset-1]=0;
		}
        // and load all WAV files in that dir into combo box
		// m_pSoundCombo->AddFileSpec(m_szCurDir, STAR_DOT_WAV);
		SelectSndFile(ofn.lpstrFile);
	}		
}

void CSoundScheme::PlayCurrentEventSnd()
{
	LPCTSTR pszSnd = m_pSoundTree->GetItemDataText(m_htiCur);
	ASSERT(pszSnd);

	Sleep(150);	//delay to let the keybd/touch click to go away. 
	if(!sndPlaySound(pszSnd, SND_ASYNC))
	{
		RETAILMSG(1, (L"sndPlaySound failed. file='%s'\r\n", pszSnd));
	    MessageBox(m_hDlg, CELOADSZ(IDS_ERRORPLAY2), CELOADSZ(IDS_SOUND_ERROR), MB_APPLMODAL | MB_OK |MB_ICONSTOP);
	}
}

void CSoundScheme::ChooseScheme()
{
	int iSel = ComboBox_GetCurSel(m_hwndScheme); 
	// select the new scheme into the treeview & set buttons approp
	SelectScheme(iSel);
}

void CSoundScheme::OnDelete()
{
    int iSel = ComboBox_GetCurSel(m_hwndScheme);
    if (iSel == CB_ERR)
     	return;

	// get confirmation from user
    if (MessageBox(m_hDlg, CELOADSZ(IDS_CONFIRMREMOVE), CELOADSZ(IDS_REMOVESCHEME), 
    		MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
	{
		// delete scheme from registry & combo
		DeleteSchemeFromRegAndCombo(iSel);
		
		// Explicitly set the combo box to the ".NoSounds" (or first entry in CB if "No Sounds" is missing)
		iSel = m_pSchemeCombo->FindByData(REGVALUE_NOSOUNDS);
		if(iSel == CB_ERR)
			iSel = 0;

		// select the No Sounds into the treeview & set buttons approp
		SelectScheme(iSel);
	}
	SetFocus(GetDlgItem(m_hDlg, IDC_COMBO_SCHEME));
}

void CSoundScheme::DeleteSchemeFromRegAndCombo(int iIndex)
{    
    LPCTSTR pszKey = (LPCTSTR)ComboBox_GetItemData(m_hwndScheme, iIndex);
    
	CReg reg1(HKEY_LOCAL_MACHINE, RK_SND_EVENT);
	reg1.DeleteValue(pszKey);

	CReg reg2(HKEY_LOCAL_MACHINE, RK_SND_SCHEME);
	reg2.DeleteValue(pszKey);

	// delete item from combo box
    m_pSchemeCombo->DeleteItem(iIndex);

	// need to inform WAVE driver
    m_fUpdate = TRUE;
}

void CSoundScheme::SetSchemeAsDefault(LPCTSTR pszKey)
{
   	DEBUGMSG(ZONE_SOUNDS, (L"Saving '%s' as default. \r\n", pszKey));
   	
	CReg reg;
	reg.Create(HKEY_LOCAL_MACHINE, RK_SND_EVENT);
	reg.SetSZ(RV_SCHEME, pszKey);

	// need to inform WAVE driver
    m_fUpdate = TRUE;
}

// Create a new scheme & save it & add it to the combo
void CSoundScheme::AddNewScheme(LPCTSTR pszDisplay)
{
	TCHAR szKey[KEYLEN+8];
	
	// create unique regkey name
	CreateUniqueSchemeName(pszDisplay, szKey, CCHSIZEOF(szKey));
	
	// grab the evnt mapping & save it & save all reg keys
	SaveNewScheme(pszDisplay, szKey);
			
	// add the newly created scheme to the combo
	int iSel = m_pSchemeCombo->AddItem(pszDisplay, szKey);
		
	// and select it into the tree & set button state & state vars approp
	SelectScheme(iSel);
}

// On "Save As" button
void CSoundScheme::OnSave()
{
    int iSel = ComboBox_GetCurSel(m_hwndScheme);
    if (iSel == CB_ERR)
     	return;
     	
	// Retrieve current scheme and pass it to the savescheme dialog.
	ComboBox_GetLBText(m_hwndScheme, iSel, m_szCurScheme);
	
	DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_SNDSCHEMESAVE), m_hDlg, 
					SaveSndSchemeDlgProc, (LPARAM)this);
}

// On "Save As" button
BOOL CALLBACK SaveSndSchemeDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LONG lParam)
{
	// parent passes us the this ptr to CSoundScheme
	static CSoundScheme* pSoundScheme;
	
	switch (uMsg)
	{
    case WM_HELP:
		MyCreateProcess(TEXT("peghelp"), SndSchemeTab.pszHelp);
		break;

    case WM_INITDIALOG:
	  {
		CenterWindowSIPAware(hDlg);
		pSoundScheme = (CSoundScheme*)lParam; // lParam is this ptr
		HWND hwndEdit = GetDlgItem(hDlg, IDC_SCHEMENAME);
        Edit_LimitText(hwndEdit, MAXSCHEME);
        // dump the text from lparam into the edit control. 
        Edit_SetText(hwndEdit, pSoundScheme->m_szCurScheme);
		SetFocus(hwndEdit);
		SendMessage(hwndEdit, EM_SETSEL, 0L, -1);
        return 0; // we already set focus
	  }
    
	case WM_COMMAND:	        
		switch (wParam)
		{
		case IDOK:
		  {

			TCHAR szMsg[MAX_PATH];
			Edit_GetText(DI(IDC_SCHEMENAME), pSoundScheme->m_szCurScheme, MAX_PATH);

			if (!(*(pSoundScheme->m_szCurScheme)))
			{				
				// the user has entered an empty string.
				StringCbPrintf(szMsg, sizeof(szMsg), CELOADSZ(IDS_NOOVERWRITEDEFAULT), pSoundScheme->m_szCurScheme);
				MessageBox(hDlg, szMsg, CELOADSZ(IDS_CHANGESCHEME), MB_ICONEXCLAMATION | MB_OKCANCEL);
				return TRUE;
			}

			// see if this name is a duplicate
			int iIndex = ComboBox_FindStringExact(pSoundScheme->m_hwndScheme, -1, pSoundScheme->m_szCurScheme);
			DEBUGMSG(ZONE_SOUNDS, (L"SaveScheme IDOK %s %x %d\r\n", pSoundScheme->m_szCurScheme, pSoundScheme->m_hwndScheme, iIndex));
			if (iIndex == CB_ERR)
			{
				// not duplicate. Save it to registry
				pSoundScheme->AddNewScheme(pSoundScheme->m_szCurScheme);
			    EndDialog(hDlg, TRUE); 
			}
			else
		    {
		    	// duplicate
//			  	TCHAR szMsg[MAX_PATH];
		    	// now check to see if it is .Default or .None
		    	LPCTSTR pszKey = (LPCTSTR)ComboBox_GetItemData(pSoundScheme->m_hwndScheme, iIndex);

				if ((NULL != pszKey) &&
				    ((CSTR_EQUAL == CompareString(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
				                                  NORM_IGNORECASE, pszKey, -1, REGVALUE_DEFAULTSOUNDS, -1)) ||
				     (CSTR_EQUAL == CompareString(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), 
				                                  NORM_IGNORECASE, pszKey, -1, REGVALUE_NOSOUNDS, -1)) ||
				     (CSTR_EQUAL == CompareString(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), 
				                                  NORM_IGNORECASE, pszKey, -1, REGVALUE_ALLSOUNDS, -1))))
				{
					wsprintf(szMsg, CELOADSZ(IDS_NOOVERWRITEDEFAULT), pSoundScheme->m_szCurScheme);
					MessageBox(hDlg, szMsg, CELOADSZ(IDS_CHANGESCHEME), MB_ICONEXCLAMATION | MB_OKCANCEL);
				}
		        else
		        {
		        	wsprintf(szMsg, CELOADSZ(IDS_OVERWRITESCHEME), pSoundScheme->m_szCurScheme);
					if(IDYES==MessageBox(hDlg, szMsg, CELOADSZ(IDS_CHANGESCHEME), 
								MB_ICONEXCLAMATION |  MB_YESNOCANCEL))
					{
						// existing scheme to be over-written. Delete it first
						pSoundScheme->DeleteSchemeFromRegAndCombo(iIndex);
						// Save new one to registry
						pSoundScheme->AddNewScheme(pSoundScheme->m_szCurScheme);
			            EndDialog(hDlg, TRUE); 
						break;
					}
				}
				// we get here on user cancelling out of the warning prompts. Stay in dialog box
				SetFocus(DI(IDC_SCHEMENAME));
				SendMessage(GetDlgItem(hDlg, IDC_SCHEMENAME), EM_SETSEL, 0L, -1);
			}
			return TRUE;
		  }
		  
		case IDCANCEL:
		  {
			EndDialog(hDlg, FALSE);
			return TRUE;
		  }
		}
	}
    return FALSE;
}


void CSoundScheme::SaveSettings()
{
	int iCurSel;

	//DebugBreak();

	// if we've changed the event mapping then we have in effect a new scheme
	// We always save this sa "Current Settings" overwriting any previous
	// Scheme of that name
	if(m_fChangedEventMapping)
	{
		// selects the existing entry by name, and adds it if missing
		iCurSel = m_pSchemeCombo->SelectByDisplay(CELOADSZ(IDS_CURRENTSETTINGS), CURR0);
		SaveNewScheme(CELOADSZ(IDS_CURRENTSETTINGS), CURR0);
	}
	else
	{
		iCurSel = ComboBox_GetCurSel(m_hwndScheme);
	}
	
	DEBUGMSG(ZONE_SOUNDS, (L"Sound: SaveSettings: iCurSel=%d\r\n", iCurSel));

	// get selected scheme & save it as the default
	if(iCurSel != CB_ERR)
	{
    	SetSchemeAsDefault((LPCTSTR)ComboBox_GetItemData(m_hwndScheme, iCurSel));
	
		// delete any entry named "Current Settings" IFF it is not the selected default!
		int iTemp = ComboBox_FindStringExact(m_hwndScheme, -1, CELOADSZ(IDS_CURRENTSETTINGS));
	    if(iTemp != CB_ERR  && iTemp != iCurSel)
			DeleteSchemeFromRegAndCombo(iTemp);
	}
}

void CSoundScheme::SaveNewScheme(LPCTSTR lpszDisplay, LPCTSTR lpszRegName)
{
	int   iBufLen = 2 + 2*NUM_SOUND_EVENTS;	// min MULTI_SZ len is space+null per event plus dbl-null
	PCTSTR rgszPaths[NUM_SOUND_EVENTS];
	memset(rgszPaths, 0, sizeof(rgszPaths));
	
	DEBUGMSG(ZONE_SOUNDS, (L"SaveNewScheme %s %s\r\n", lpszDisplay, lpszRegName));

	// Saving is complicated by the fact that the treeview is sorted. Hence
	// the order of items does not match the order in the event table anymore. We could
	// have been smart and saved an event-number with each item in the tree, but we're
	// being simple instead. We walk the whole tree, for each item, search  
	// for the text of the item in m_cszSoundEvents to find the event number, and use
	// this to populate rgszPaths.
	
    for(HTREEITEM hti=TreeView_GetChild(m_hwndTree, TreeView_GetRoot(m_hwndTree)); hti; hti=TreeView_GetNextSibling(m_hwndTree, hti))
	{
		TCHAR szText[MAX_PATH];
        PCTSTR pszPath = m_pSoundTree->GetItemDataText(hti, szText, CCHSIZEOF(szText));
        DEBUGCHK(szText[0]);
        // if no sound, then skip to next (since that's the default anyway)
        if(!pszPath)
        	continue;
        // find name in m_cszSoundEvents
        for(int i=0; i<NUM_SOUND_EVENTS; i++)
        {
        	if(0==lstrcmpi(szText, m_cszSoundEvents[i]))
        	{
        		// save the path in the correct event-number slot in rgszPaths[]
        		rgszPaths[i] = pszPath;
				iBufLen += lstrlen(pszPath);	// this will be an overestimate.
        		break;
        	}
        }
        DEBUGCHK(i!=NUM_SOUND_EVENTS);
	}       
        
    // now rgszPaths[] contains the event-->WAV mappings for all events present and in use.
    // create a MULTI_SZ out of this The order & number of SZs must match the event table!
    PTSTR pszMultiSz = MySzAlloc(iBufLen);
    DEBUGCHK(pszMultiSz);
    PTSTR pszNext = pszMultiSz;
   	int iWinLen = lstrlen(PATH_WINDOWS_SLASH);
   	for(int j=0; j<NUM_SOUND_EVENTS; j++)
    {
		if(rgszPaths[j])
		{
       		// if we have a \Windows\ prefix & no further path, eliminate it
   	    	if( (0==_wcsnicmp(rgszPaths[j], PATH_WINDOWS_SLASH, iWinLen)) && (NULL==wcschr(rgszPaths[j]+iWinLen, L'\\')) )
   	    		rgszPaths[j] += iWinLen;
   	    	// if we have an extension, remove it [NOTE: dont actually modify the string!!]
   	    	PCTSTR pszEnd;
   	    	int iLen;
   	    	if(pszEnd = wcsrchr(rgszPaths[j], L'.'))
   	    		iLen = (pszEnd - rgszPaths[j]);
   	    	else
   	    		iLen = lstrlen(rgszPaths[j]);
   	    	// copy what's left to the multisz
   	    	memcpy(pszNext, rgszPaths[j], sizeof(TCHAR)*iLen);
   	    	pszNext += iLen;
   	    }
   	    else
   	    {
			//Prefix
			if(pszNext)
   	    		*pszNext++ = L' ';	// a single space for missing/silent events
   	    }

		//Prefix
		if(pszNext)
    		*pszNext++ = 0;	// null terminate
    	DEBUGCHK((pszNext - pszMultiSz)+1 < iBufLen);
	}   	   		
	// terminating double-nul
	//Prefix
	if(pszNext)
		*pszNext++ = 0;
	
	// save multisz event mapping
	CReg reg1;
	reg1.Create(HKEY_LOCAL_MACHINE, RK_SND_EVENT);
	reg1.SetMultiSZ(lpszRegName, pszMultiSz, pszNext-pszMultiSz);
	MyFree(pszMultiSz);

	// save an entry in the scheme key to map from key to display name
	CReg reg2;
	reg2.Create(HKEY_LOCAL_MACHINE, RK_SND_SCHEME);
	reg2.SetSZ(lpszRegName, lpszDisplay);

	// need to inform WAVE driver
    m_fUpdate = TRUE;
}

BOOL CreateUniqueSchemeName(LPCTSTR pszDisplay, LPTSTR pszKey, int iMaxLen)
{
	ASSERT(iMaxLen >= KEYLEN+4);
	
	int iLen = min(lstrlen(pszDisplay), KEYLEN);
	memcpy(pszKey, pszDisplay, sizeof(TCHAR)*iLen);
	pszKey[iLen] = '0';
    pszKey[iLen+1] = '\0';

	CReg reg(HKEY_LOCAL_MACHINE, RK_SND_SCHEME);
	
  	for(int i=0; reg.ValueBinary(pszKey) && i<50; i++)
  	{        
		DEBUGMSG(ZONE_SOUNDS, (L"RegAddScheme: scheme='%s' exists\r\n"));      
        pszKey[iLen]++;
    }
    return i<50;
}

extern "C" BOOL APIENTRY SndSchemeDlgProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	static CSoundScheme* pSoundScheme;
	static BOOL fSchemeCBDroppedDown,fFilesCBDroppedDown;
	
    switch (message)
    {
    case WM_INITDIALOG:
	    pSoundScheme = new CSoundScheme(hDlg);
    	fSchemeCBDroppedDown = fFilesCBDroppedDown = FALSE;
	    return 1;

   	case WM_DRAWITEM:
   		DrawCPLButton((LPDRAWITEMSTRUCT)lParam, (LOWORD(wParam)==IDC_TEST) ? IDB_RIGHT : IDB_STOP);
   		break;

   	case WM_DESTROY:
   		delete pSoundScheme;
   		break;

    case WM_COMMAND:      
		switch (LOWORD(wParam))
		{
		case IDOK:
			pSoundScheme->SaveSettings();
			// fall through
		case IDCANCEL:
			if(pSoundScheme->m_fUpdate)
				AudioUpdateFromRegistry(); // Tell Wave driver to reload settings
			return TRUE;
			
        case IDC_BROWSE:
			pSoundScheme->BrowseForWAV();
			return TRUE;
			
        case IDC_TEST:
			pSoundScheme->PlayCurrentEventSnd();
			return TRUE;

        case IDC_STOP:
			sndPlaySound(NULL, 0);
			return TRUE;

        case IDC_DELETE:
			pSoundScheme->OnDelete();
            return TRUE;

        case IDC_SAVE:
			pSoundScheme->OnSave();
            return TRUE;
			
        case IDC_COMBO_SCHEME:
            switch (HIWORD(wParam))
            {   
            case CBN_DROPDOWN:  fSchemeCBDroppedDown = TRUE;  break; 
            case CBN_CLOSEUP:   fSchemeCBDroppedDown = FALSE; break;
            case CBN_SELCHANGE: if(fSchemeCBDroppedDown) break;
            	// else fall through to OK
            case CBN_SELENDOK:
	    		pSoundScheme->ChooseScheme(); 
	    		break;
			}                   
            return TRUE;
        
        case IDC_COMBO_SOUND:
			DEBUGMSG(ZONE_SOUNDS, (L"SoundCombo msg(%d)\r\n", HIWORD(wParam)));
            switch (HIWORD(wParam))
            {   
            case CBN_DROPDOWN:
				fFilesCBDroppedDown = TRUE; 
				break; 

            case CBN_CLOSEUP:
				{
					// per the docs you can't predict the order of CBN_CLOSEUP and CBN_SELCHANGE so we need to
					// handle the change when we close also.

					fFilesCBDroppedDown = FALSE;

					pSoundScheme->ChooseSound(); 
				}break;

            case CBN_SELCHANGE:
				{
					if(fFilesCBDroppedDown)
						break;

            		// else fall through to OK
				}

            case CBN_SELENDOK:
	    		pSoundScheme->ChooseSound(); 
	    		break;
	    	}
	    	return TRUE;
	    }
	    break;

	case WM_NOTIFY:
		switch(((NMHDR*)lParam)->code)
        {
        case TVN_ITEMEXPANDING:			// prevent Tree from collapsing
			if(!(((LPNM_TREEVIEW)lParam)->itemNew.state & TVIS_EXPANDEDONCE))
				return TRUE;
			break;
			
		case TVN_SELCHANGED:
			pSoundScheme->OnTVSelChange(((LPNM_TREEVIEW)lParam)->itemNew);
			break;
		}
    }
    return (FALSE);
}


