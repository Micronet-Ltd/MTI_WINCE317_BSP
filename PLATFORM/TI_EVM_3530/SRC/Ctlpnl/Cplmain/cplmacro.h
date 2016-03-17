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

Abstract: Macros and helper classes & functions of general applicability
**/


#define ARRAYSIZEOF(x)	(sizeof(x) / sizeof((x)[0]))
#define CCHSIZEOF(x)	(sizeof(x) / sizeof(TCHAR))

#define ZEROMEM(p)		memset(p, 0, sizeof(*(p)))

#define LOADSZ(ids, psz)	(LoadString(g_hInst, ids, psz, CCHSIZEOF(psz)))
#define CELOADSZ(ids)		((LPCTSTR)LoadString(g_hInst, ids, NULL, 0))

#define MyAlloc(typ)		((typ*)LocalAlloc(LPTR, sizeof(typ)))
#define MyRgAlloc(typ, n)	((typ*)LocalAlloc(LPTR, (n)*sizeof(typ)))
#define MyRgReAlloc(typ, p, n)	((typ*)LocalReAlloc(p, (n)*sizeof(typ), LMEM_MOVEABLE | LMEM_ZEROINIT))
#define MySzAlloc(n)		((LPTSTR)LocalAlloc(LPTR, (1+(n))*sizeof(TCHAR)))
#define MySzAlloca(n)		((LPTSTR)_alloca((1+(n))*sizeof(TCHAR)))
#define MyFree(p)			{ if(p) LocalFree(p); }

#define DI(x)		GetDlgItem(hDlg, x)

#define MyFreeLib(h)		{ if(h) FreeLibrary(h); }
#define MyCloseHandle(h)	{ if(h) CloseHandle(h); }
#define MyDeleteObject(h)	{ if(h) DeleteObject(h); }

#define abs(x)		( (x) < 0 ? -(x) : (x) )

#ifdef DEBUG
	#define DECLAREWAITCURSOR  HCURSOR hcursor_wait_cursor_save = (HCURSOR)0xDEADBEEF
	#define SetWaitCursor()   { ASSERT(hcursor_wait_cursor_save == (HCURSOR)0xDEADBEEF); hcursor_wait_cursor_save = SetCursor(LoadCursor(NULL, IDC_WAIT)); }
	#define ResetWaitCursor() { ASSERT(hcursor_wait_cursor_save != (HCURSOR)0xDEADBEEF); SetCursor(hcursor_wait_cursor_save); hcursor_wait_cursor_save = (HCURSOR)0xDEADBEEF; }
#else
	#define DECLAREWAITCURSOR  HCURSOR hcursor_wait_cursor_save = NULL
	#define SetWaitCursor()   { hcursor_wait_cursor_save = SetCursor(LoadCursor(NULL, IDC_WAIT)); }
	#define ResetWaitCursor() { SetCursor(hcursor_wait_cursor_save);  hcursor_wait_cursor_save = NULL; }
#endif

#define EMPTY_STRING			TEXT("")

/////////////////////////////////////////////////////////////////////////////
// CReg: Registry helper class
/////////////////////////////////////////////////////////////////////////////
class CReg
{
private:
	HKEY	m_hKey;
	int		m_Index;
	LPBYTE	m_lpbValue; // last value read, if any

public:
	BOOL Create(HKEY hkRoot, LPCTSTR pszKey) {
		DWORD dwDisp;
		return ERROR_SUCCESS==RegCreateKeyEx(hkRoot, pszKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dwDisp);
	}

	BOOL Open(HKEY hkRoot, LPCTSTR pszKey, REGSAM sam=KEY_READ) {
		return ERROR_SUCCESS==RegOpenKeyEx(hkRoot, pszKey, 0, sam, &m_hKey);
	}

	CReg(HKEY hkRoot, LPCTSTR pszKey) {
		m_hKey = NULL;
		m_Index = 0;
		m_lpbValue = NULL;
		Open(hkRoot, pszKey);
	}

	CReg() {
		m_hKey = NULL;
		m_Index = 0;
		m_lpbValue = NULL;
	}

	~CReg() { 
		if(m_hKey) RegCloseKey(m_hKey); 
		MyFree(m_lpbValue);
	}

	void Reset() { 
		if(m_hKey) RegCloseKey(m_hKey); 
		MyFree(m_lpbValue);
		m_hKey = NULL;
		m_Index = 0;
		m_lpbValue = NULL;
	}

	operator HKEY() { return m_hKey; }

	BOOL IsOK(void) { return m_hKey!=NULL; }


	BOOL EnumKey(LPTSTR psz, DWORD dwLen) {
		if(!m_hKey) return FALSE;
		return ERROR_SUCCESS==RegEnumKeyEx(m_hKey, m_Index++, psz, &dwLen, NULL, NULL, NULL, NULL);
	}

	BOOL EnumValue(LPTSTR pszName, DWORD dwLenName, LPTSTR pszValue, DWORD dwLenValue) {
		DWORD dwType;
		if(!m_hKey) return FALSE;
		dwLenValue *= sizeof(TCHAR); // convert length in chars to bytes
		return ERROR_SUCCESS==RegEnumValue(m_hKey, m_Index++, pszName, &dwLenName, NULL, &dwType, (LPBYTE)pszValue, &dwLenValue);
	}

	BOOL ValueSZ(LPCTSTR szName, LPTSTR szValue, DWORD dwLen) {
		if(!m_hKey) return FALSE;
		dwLen *= sizeof(TCHAR); // convert length in chars to bytes
		return ERROR_SUCCESS==RegQueryValueEx(m_hKey, szName, NULL, NULL, (LPBYTE)szValue, &dwLen);
	}

	DWORD ValueBinary(LPCTSTR szName, LPBYTE lpbValue, DWORD dwLen) {
		if(!m_hKey) return FALSE;
		DWORD dwLenWant = dwLen;
		if(ERROR_SUCCESS==RegQueryValueEx(m_hKey, szName, NULL, NULL, lpbValue, &dwLen))
			return dwLen;
		else
			return 0;
	}

	LPCTSTR ValueSZ(LPCTSTR szName);

	LPBYTE ValueBinary(LPCTSTR szName) {
		return (LPBYTE)ValueSZ(szName);
	}

	DWORD ValueDW(LPCTSTR szName, DWORD dwDefault=0) {
		if(!m_hKey) return FALSE;
		DWORD dwValue = dwDefault;
		DWORD dwLen = sizeof(DWORD);
		RegQueryValueEx(m_hKey, szName, NULL, NULL, (LPBYTE)&dwValue, &dwLen);
		return dwValue;
	}

	BOOL SetSZ(LPCTSTR szName, LPCTSTR szValue, DWORD dwLen) {
		//Prefix
		if(!m_hKey) return FALSE;
		//
		return ERROR_SUCCESS==RegSetValueEx(m_hKey, szName, 0, REG_SZ, (LPBYTE)szValue, sizeof(TCHAR)*dwLen);
	}
	
	BOOL SetSZ(LPCTSTR szName, LPCTSTR szValue) {
		return SetSZ(szName, szValue, 1+lstrlen(szValue));
	}

	BOOL SetDW(LPCTSTR szName, DWORD dwValue) {
		//Prefix
		if(!m_hKey) return FALSE;
		//
		return ERROR_SUCCESS==RegSetValueEx(m_hKey, szName, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
	}
	
	BOOL SetBinary(LPCTSTR szName, LPBYTE lpbValue, DWORD dwLen) {
		//Prefix
		if(!m_hKey) return FALSE;
		//
		return ERROR_SUCCESS==RegSetValueEx(m_hKey, szName, 0, REG_BINARY, lpbValue, dwLen);
	}

	BOOL SetMultiSZ(LPCTSTR szName, LPCTSTR lpszValue, DWORD dwLen) {
		return ERROR_SUCCESS==RegSetValueEx(m_hKey, szName, 0, REG_MULTI_SZ, (LPBYTE)lpszValue, sizeof(TCHAR)*dwLen);
	}

	BOOL DeleteValue(LPCTSTR szName) {
		//Prefix
		if(!m_hKey) return FALSE;
		//
		return ERROR_SUCCESS==RegDeleteValue(m_hKey, szName);
	}

	BOOL DeleteKey(LPCTSTR szName) {
		if(!m_hKey) return FALSE;
		return ERROR_SUCCESS==RegDeleteKey(m_hKey, szName);
	}

};

/////////////////////////////////////////////////////////////////////////////
// a class that populates a combo box with filenames. Friendly names are displayed
// Full paths are stored as item-data. Main point of the class is to manage the memory
// for the path strings cleanly in one place
/////////////////////////////////////////////////////////////////////////////
class CFileCombo
{
	HWND m_hwndCB;
	int m_iSavedSel;

public:
	CFileCombo(HWND hwnd) : m_hwndCB(hwnd) {}
	~CFileCombo();
	
	void SaveSelection(void) {
		m_iSavedSel = ComboBox_GetCurSel(m_hwndCB);
	}

	void RestoreSelection(void) {
		ComboBox_SetCurSel(m_hwndCB, m_iSavedSel);
	}

	int AddFile(LPCTSTR szDir, LPCTSTR szFile);
	void AddFileSpec(LPCTSTR pszDir, LPCTSTR szSpec);
	
// these should really be in a seperate CCombo base class, since they have nothing to do with files as such
	int InsertItem(LPCTSTR pszDisplay, LPCTSTR pszData=EMPTY_STRING, int iPos=0);
	int AddItem(LPCTSTR pszDisplay, LPCTSTR pszData=EMPTY_STRING);
	void DeleteItem(int iSel);
	int FindByData(LPCTSTR pszFind);
	
	// find item by display name & select it. Add it if it's missing
	int SelectByDisplay(LPCTSTR pszDisplay, LPCTSTR pszData);
	// Tries to find the item in CB by the Data value & select it. If missing adds it.
	int SelectByPath(LPCTSTR pszPath);
};

/////////////////////////////////////////////////////////////////////////////
// TreeView helper class
/////////////////////////////////////////////////////////////////////////////
class CFileTreeView
{
private:
	HWND m_hwndTree;

	void ClearItem(HTREEITEM hti);
	void ClearSubtree(HTREEITEM htiRoot);

public:
	void Clear();
	CFileTreeView(HWND hwnd) : m_hwndTree(hwnd) {}
	CFileTreeView() { Clear(); }

 	// Adds items to tree-view
 	HTREEITEM AddItem(HTREEITEM htiParent, LPCTSTR pszDisplay, LPCTSTR pszData, int iImage, HTREEITEM htiAfter);
	void SetItemData(HTREEITEM hti, LPCTSTR pszData, int iImage);
	LPCTSTR GetItemDataText(HTREEITEM hti, PTSTR pszText=0, int cch=0);
};

/////////////////////////////////////////////////////////////////////////////
// SIP helper class & helper functions
/////////////////////////////////////////////////////////////////////////////

DWORD GetSipState(void);
void  RaiseLowerSip(BOOL fUp);

class CSipUpDown
{
private:
	BOOL	m_fOriginalSipState;

public:
	CSipUpDown() { m_fOriginalSipState= 0; }
	~CSipUpDown() {}
	
	void CSipUpDown::SaveSipState(void) { 
		m_fOriginalSipState=GetSipState(); 
//		DEBUGMSG(DEBUG_NOISY, (L"SaveSipState=%d\r\n", m_fOriginalSipState & SIPF_ON));
	}
	void CSipUpDown::RestoreSipState() { 
		RaiseLowerSip(m_fOriginalSipState & SIPF_ON);
//		DEBUGMSG(DEBUG_NOISY, (L"RestoreSipState=%d\r\n", m_fOriginalSipState & SIPF_ON));
	}
};

/////////////////////////////////////////////////////////////////////////////
// Misc dialog-handling helpers
/////////////////////////////////////////////////////////////////////////////

// Useful for initializing a combo box from a table
typedef struct {
	int		idcString;	// string to go in combo box
	DWORD	dwData;		// data to associate with it
}
COMBODATA;

// Useful for setting up table of text-limits within a dlg.
typedef struct 
{ 
	int idc; 
	int iMaxLen; 
}
TEXTLIMITS;

inline void SetTextLimits(HWND hDlg, const TEXTLIMITS* rgLimits, int N)
{
	for(int i=0; i<N; i++)
	{
		Edit_LimitText(DI(rgLimits[i].idc), rgLimits[i].iMaxLen);		
	}
}

inline void ShowHide(HWND hDlg, const int rgControls[], int N, int show)
{
	for(int i=0; i<N; i++)
	{
		ShowWindow(DI(rgControls[i]), show);		
	}
}

inline void EnableDisable(HWND hDlg, const int rgControls[], int N, int fEnable)
{
	for(int i=0; i<N; i++)
	{
		EnableWindow(DI(rgControls[i]), fEnable);		
	}
}

inline void Controldxdy(HWND hDlg, int idc1, int idc2, PPOINT ppt)
{
	RECT rc1, rc2;
	// finds the X & Y difference between the top-left corners of idc1 & idc2
	GetWindowRect(DI(idc1), &rc1);
	GetWindowRect(DI(idc2), &rc2);
	ppt->x = (rc2.left-rc1.left);
	ppt->y = (rc2.top-rc1.top);
}

inline void MoveControls(HWND hDlg, const int rgControls[], int N, int dx, int dy)
{
	for(int i=0; i<N; i++)
	{
		RECT rc;
		GetWindowRect(DI(rgControls[i]), &rc);
		MapWindowRect(NULL, hDlg, &rc);
		OffsetRect(&rc, dx, dy);
		MoveWindow(DI(rgControls[i]), rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE);
	}
}

// find a CB item by the data value
inline int ComboBox_FindData(HWND hItem, DWORD dwFindData)
{
	for(int i=0; ;i++)
	{
		DWORD dwData = ComboBox_GetItemData(hItem, i);
		if(dwData == CB_ERR)
			return CB_ERR;
		if(dwFindData == dwData)
			return i;
	}
}

inline void MakePoint(POINT *ppt, LPARAM lp) {
	ppt->x = LOWORD(lp);
	ppt->y = HIWORD(lp);
}



/////////////////////////////////////////////////////////////////////////////
// Misc file-path handling helpers
/////////////////////////////////////////////////////////////////////////////

inline void NukePath(LPTSTR psz)
{
	int iLen = lstrlen(psz); 
	for(int i=iLen-1; i>=0; i--)
	{
		// if we find a \\ move string past  \\ to start of buf
		if(psz[i]==TEXT('\\') && i>0 && i<iLen)
		{
			memmove(psz, psz+i+1, sizeof(TCHAR)*(iLen-i+1));
			psz[iLen-i+1]=0;
			break;
		}
	}
}

inline void NukeExt(LPTSTR psz)
{
    for(int i=lstrlen(psz)-1; i>=0; i--)
    {
    	if(psz[i]=='.')
    	{
    		psz[i] = 0;
    		break;
    	}
    }
}

inline void MakeNameNice(LPTSTR psz)
{
	NukePath(psz);
    NukeExt(psz);
    // if we have uniform lower or uniform upper casing
    // change to lower case with leading char upper
	//Prefix
    if (psz[0] != TEXT('\0') && IsCharUpper(psz[0]) == IsCharUpper(psz[1]))
    {
        CharLower(psz);
        CharUpperBuff(psz, 1);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Misc string handling helpers
/////////////////////////////////////////////////////////////////////////////

inline LPTSTR MySzDup(LPCTSTR pszIn) 
{ 
	if(!pszIn) return NULL;
	LPTSTR pszOut=MySzAlloc(lstrlen(pszIn)); 
	if(pszOut) lstrcpy(pszOut, pszIn); 
	return pszOut; 
}

/////////////////////////////////////////////////////////////////////////////
// Misc file handling helpers
/////////////////////////////////////////////////////////////////////////////

#define MyCreateProcess(path, args)	CreateProcess(path, args, 0,0,0,0,0,0,0,0)
#define MyCreateThread(fn, arg)		CreateThread(0, 0, fn, (PVOID)arg, 0, 0)
#define MyCreateFile(name, access, share, flags) CreateFile(name,access,share,NULL,flags,FILE_ATTRIBUTE_NORMAL,NULL)
#define FILE_FAILED(x)	(INVALID_HANDLE_VALUE == (x))
#define FileExists(lpFileName) (-1 != GetFileAttributes(lpFileName))

inline BOOL IsROMFile(LPCTSTR lpFileName)
{
	DWORD dwAttrib = GetFileAttributes(lpFileName);
	if((dwAttrib != (UINT)-1) && (dwAttrib & FILE_ATTRIBUTE_INROM)) {
		return TRUE;
	}
	return FALSE;
}

inline void AppendSlash(LPTSTR pszFile)
{
	int iLen = lstrlen(pszFile);
	if(iLen && pszFile[iLen-1] != L'\\') {
		pszFile[iLen] = L'\\';
		pszFile[iLen+1] = 0;
	}
}

inline BOOL IsFileInUse(LPCTSTR lpFileName)
{
	HANDLE hFile;
	// see if we can write exclusive to it (this is to figure out if it's in use)
	if(FILE_FAILED(hFile = MyCreateFile(lpFileName,GENERIC_WRITE,0,OPEN_EXISTING))) 
		return TRUE;
	else {
		CloseHandle(hFile);
		return FALSE;
	}
}

