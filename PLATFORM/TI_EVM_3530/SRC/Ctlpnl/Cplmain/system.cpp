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

Abstract: Code specific to the SYSTEM CPL
**/

#include "cplpch.h"
#include <dbt.h>

extern "C" DWORD EnumPnpIds(LPTSTR PnpList, LPDWORD lpBuflen);

#define TICK_RESOLUTION		16 			// in 4KB * 16 = 64KB
#define ONE_KB				1024
#define PAGES_RESERVED_STG	0			//64*4=256KB resreved pages needed by store. must be at lease 3*TICK_RESOLUTION
#define PAGES_RESERVED_SYS	48			//48*4=192KB resreved pages needed by system. must be at lease 3*TICK_RESOLUTION
#define PAGE_INCR			16
#define PAGES_RESERVED_SHL  40          // Pages reserved by shell to enable ctlpnl to be launched again

// device name tab
#define MAX_NAME    15
#define MAX_DESC    50
#define MAX_STRING    max(MAX_DESC+1, MAX_NAME+1)

typedef int (FAR PASCAL *HOSTPROC)(char FAR *, int);
BOOL CheckDeviceName(HOSTPROC SetHostProc, HWND hDlg, int idcEdit, int iMax);

typedef int (FAR PASCAL *PFNWSASTARTUP)(WORD, LPWSADATA);
typedef int (FAR PASCAL *PFNWSACLEANUP)();

WORD wVersionRequested = MAKEWORD(1,1);
PFNWSASTARTUP pfnWSAStartup = NULL;
PFNWSACLEANUP pfnWSACleanup = NULL;  

// Persistant data for the memory tab
typedef struct {
    	DWORD orgStorePages;
	    int  idTimer;
	    BOOL fSliderMoved;
	    int oldMin;
	    int oldMax;
	    DWORD oldStoreUsed;
	    DWORD  oldRamUsed;
} MemoryState;

// Tooltips for long strings over combo
extern HINSTANCE g_hInst; // From cplmain
static HWND     g_hToolTip = NULL;
static WNDPROC  g_wndProcTT = NULL;

// Local prototypes
void MemoryDlgOnTimer(HWND hDlg,MemoryState *pmst);
void MemoryDlgOnSlider(HWND hDlg);
void MemoryDlgOnOK(HWND hDlg, DWORD orgStorePages);

void	GetObjStoreFolder( TCHAR* FolderName );
TCHAR	ObjStoreFolder[MAX_PATH] = L"\\";//ObjectStore as root

class CEnumPNP
{
private:
	LPTSTR m_pszIDs;
	LPCTSTR m_pszNextID; // this is a ptr into m_pszIDs, so doesn't get freed by itself
	// we keep this ptr around so we can free it ourselves & callers don't have to worry
	LPTSTR m_pszLastRet;

public:
	// allocate space for IDs of the installed PNP devices
	CEnumPNP() {
		m_pszIDs = m_pszLastRet = NULL;
		m_pszNextID = NULL;
		DWORD dwSize = _MAX_PATH;

		if(EnumPnpIds(NULL, &dwSize) == ERROR_MORE_DATA)
		    dwSize = 0;
		    
		if(dwSize && (m_pszIDs = (LPTSTR)LocalAlloc(LPTR, dwSize)))
		{
			if(ERROR_SUCCESS==EnumPnpIds(m_pszIDs, &dwSize))
				m_pszNextID = m_pszIDs;
		}
	}

	~CEnumPNP() {
		MyFree(m_pszIDs);
		MyFree(m_pszLastRet);	
	}

	// iterate thru IDs of the installed PNP devices. Return friendly name if available
	// Get friendly name corresponding to PNP device from registry
	// if no freidnly name, then return PNP ID, with CRC, if any, truncated
	LPTSTR NextFriendly() {
	    
		MyFree(m_pszLastRet);  // free prev returned str	
		m_pszLastRet = NULL;   // perhaps MyFree should be changed to do this for us?
		if(!m_pszNextID || !m_pszNextID[0])
			return NULL;

		// try for friendly name in registry
		CReg reg1(HKEY_LOCAL_MACHINE, RK_DRIVERS_PCMCIA);
		CReg reg2(reg1, m_pszNextID);
		LPCTSTR pszTemp;
		LPTSTR pszRet;
		int iLen = lstrlen(m_pszNextID);

		if(pszTemp = reg2.ValueSZ(RV_FRIENDLYNAME))
		{
			pszRet = MySzDup(pszTemp); // dup the str because lifetime of reg returned str is <= life f object, which is just this stack frame
		}
		else
		{
			// pretty up the ID by removing the CRC
			pszRet = MySzDup(m_pszNextID);

			// if CRC present, nuke it
			if(iLen >= 5)
				pszRet[iLen-5]=0;
		}
		m_pszNextID += iLen+1; // increment ID ptr for next time
		m_pszLastRet = pszRet; // save returned string for freeing next time
		return pszRet;
	}
};

void ShowPNPInfo(HWND hDlg)
{
    CEnumPNP enumpnp;

	LPTSTR psz1;
    HWND hwndCB = DI(IDC_EXPANSION_CMB);
    
    while(ComboBox_GetCount(hwndCB) > 0)
        ComboBox_DeleteString(hwndCB,0);
        
    while(psz1 = enumpnp.NextFriendly())
        ComboBox_AddString(hwndCB,psz1);

    if(ComboBox_GetCount(hwndCB) == 0)
        EnableWindow(hwndCB,FALSE);
    else
    {
        EnableWindow(hwndCB,TRUE);
        ComboBox_SetCurSel(hwndCB,0);
    }
}

void GetProgramInformation(DWORD PageSize, DWORD *pRamUsed)
{					  
	MEMORYSTATUS mst;

	mst.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&mst);

	mst.dwTotalPhys /= (PageSize * ONE_KB); 	
	mst.dwAvailPhys /= (PageSize * ONE_KB); 	

	*pRamUsed = mst.dwTotalPhys - mst.dwAvailPhys + 1;	 // '-1' because page #0 for system page ?

	DEBUGMSG(ZONE_SYSTEM, (L"GlobalMemoryStatus: dwTotalPhys=%d dwAvailPhys=%d dwMemoryLoad=%d \r\n", mst.dwTotalPhys, mst.dwAvailPhys, mst.dwMemoryLoad));
}
void GetObjStoreFolder( TCHAR* FolderName )
{
	CReg reg;

	if(reg.Open(HKEY_LOCAL_MACHINE, _T("System\\StorageManager\\AutoLoad\\ObjectStore") ))
	{
		if( reg.ValueSZ( _T("Folder"), FolderName, MAX_PATH ) )
			return;
	}
	_tcscpy( FolderName, _T("\\") );
}
void GetMemoryInfo(DWORD& PageSize, DWORD& TotPages, DWORD& StoreUsed, DWORD& RamUsed, DWORD& StorePages)
{
	DWORD RamPages;
	STORE_INFORMATION  StoreInfo;
	
	GetSystemMemoryDivision(&StorePages, &RamPages, &PageSize);
	
	if( 0 == _tcscmp( ObjStoreFolder, _T("\\") ) )//ObjectStore is the root
	{
		GetStoreInformation(&StoreInfo);
		StoreInfo.dwStoreSize /= PageSize;
		StoreInfo.dwFreeSize /= PageSize;
	}
	else
	{
		ULARGE_INTEGER FreeBytesAvailableToCaller;
		ULARGE_INTEGER TotalNumberOfBytes;
		ULARGE_INTEGER TotalNumberOfFreeBytes;

		GetDiskFreeSpaceEx( ObjStoreFolder, &FreeBytesAvailableToCaller, &TotalNumberOfBytes, &TotalNumberOfFreeBytes );

		StoreInfo.dwStoreSize	= TotalNumberOfBytes.LowPart / PageSize;
		StoreInfo.dwFreeSize	= TotalNumberOfFreeBytes.LowPart / PageSize;
	}

	PageSize /= ONE_KB;
	TotPages = StorePages+RamPages;			
	StoreUsed =  StorePages - StoreInfo.dwFreeSize; // use the 'free' number. Becuase dwStoreSize has some compression and it is in-accurate 
	GetProgramInformation(PageSize, &RamUsed);
	
	DEBUGMSG(ZONE_SYSTEM, (L"CPL:GetMemInfo: PageSize=%d TotPages=%d StorePages=%d StoreUsed=%d ProgUsed=%d\r\n", 
			PageSize, TotPages, StorePages, StoreUsed, RamUsed));
}

LRESULT CALLBACK SYS_ToolTipProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (TTM_WINDOWFROMPOINT == uMsg)
	{
		ASSERT(lParam);
		POINT pt = (*((POINT FAR *)lParam));
		HWND hDlg = (HWND) GetWindowLong(hwnd, GWL_USERDATA);
		MapWindowPoints(NULL, hDlg, &pt, 1);
		return ((LRESULT) ChildWindowFromPoint(hDlg, pt));
	}

	ASSERT(g_wndProcTT);
	return CallWindowProc(g_wndProcTT, hwnd, uMsg, wParam, lParam);
}


extern "C" BOOL APIENTRY SystemDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    switch (message)
    {
		case WM_INITDIALOG:
		{
			// set System Version
			OSVERSIONINFO osvi;
		    TCHAR szTemp[190];

			osvi.dwOSVersionInfoSize = sizeof(osvi);
			GetVersionEx(&osvi);

			if (osvi.dwBuildNumber > 0) {
	 			StringCbPrintf(szTemp, sizeof(szTemp), CELOADSZ(IDS_SYSTEM_VER_WITH_BUILD), 
							osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
			} else {
	 			StringCbPrintf(szTemp, sizeof(szTemp), CELOADSZ(IDS_SYSTEM_VER_WITHOUT_BUILD), 
								osvi.dwMajorVersion, osvi.dwMinorVersion);
			}
			SetDlgItemText(hDlg, IDC_STATIC_VER_SYS, szTemp);
			
			// set Processor type
			SYSTEM_INFO sys_info;
			ZEROMEM(&sys_info);
			GetSystemInfo(&sys_info);

			PROCESSOR_INFO pro; 
			DWORD count;
			KernelIoControl(IOCTL_PROCESSOR_INFORMATION, NULL, 0, &pro, sizeof(PROCESSOR_INFO), &count);
			lstrcpy(szTemp, pro.szVendor);
			if(pro.szVendor[0] != NULL && (pro.szProcessCore[0] != NULL || pro.szProcessorName[0] != NULL)) {
				lstrcat(szTemp, TEXT(", "));
			}
			lstrcat(szTemp, pro.szProcessCore);
			if(pro.szProcessCore[0] != NULL && pro.szProcessorName[0] != NULL){
				lstrcat(szTemp, TEXT("-"));
			}
			lstrcat(szTemp, pro.szProcessorName);

			SetDlgItemText(hDlg, IDC_STATIC_PROCESSOR, szTemp);
			PostMessage(GetDlgItem(hDlg, IDC_STATIC_PROCESSOR), EM_SETSEL, 0, 0);

			// set RAM size
			DWORD PageSize, TotPages, StoreUsed, RamUsed, StorePages;
			GetMemoryInfo(PageSize, TotPages, StoreUsed, RamUsed, StorePages);
			StringCbPrintf(szTemp, sizeof(szTemp), CELOADSZ(IDS_SYSTEM_RAMSIZE), TotPages*PageSize);
			SetDlgItemText(hDlg, IDC_STATIC_RAM, szTemp);

			// set PNP card info
			ShowPNPInfo(hDlg);

			// set User & Company do this on WM_PAINT as it can change
			OWNER_PROFILE OwnerProfile;
			GetOwnerInfo(&OwnerProfile, NULL);

			// Truncate the strings and append ellipses is they are too long
			SetTextWithEllipsis(DI(IDC_STATIC_USER), OwnerProfile.szName);
			HWND hUserCompany = DI(IDC_STATIC_USER_COMPANY);
			if (hUserCompany)
				SetTextWithEllipsis(hUserCompany, OwnerProfile.szCompany);

			// Create the tooltip window for long strings
			g_hToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
			                            WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
			                            CW_USEDEFAULT, CW_USEDEFAULT,
			                            CW_USEDEFAULT, CW_USEDEFAULT,
			                            hDlg, NULL, g_hInst, NULL);

			if (g_hToolTip)
			{
				TOOLINFO ti = {0};
				ti.cbSize = sizeof(ti);
				ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_TRANSPARENT;
				ti.hwnd = hDlg;
				ti.lpszText = LPSTR_TEXTCALLBACK;
				ti.uId = (UINT) GetDlgItem(hDlg, IDC_STATIC_PROCESSOR);
				SendMessage(g_hToolTip, TTM_ADDTOOL, NULL, (LPARAM) &ti);

				g_wndProcTT = (WNDPROC) SetWindowLong(g_hToolTip, GWL_WNDPROC,
				                                      (LONG) SYS_ToolTipProc);
				ASSERT(g_wndProcTT);
				SetWindowLong(g_hToolTip, GWL_USERDATA, (LONG) hDlg);
			}

            AygInitDialog( hDlg, SHIDIF_SIPDOWN );

			SetFocus(GetDlgItem(hDlg, IDC_EXPANSION_CMB));
			return 0;
		}
		
		case WM_DEVICECHANGE:		// PnP device changed
			// update PNP card info
			ShowPNPInfo(hDlg);
			return TRUE;

		case WM_COMMAND:      
			switch (LOWORD(wParam))
			{
				case IDOK:
				return TRUE;
			}
			break;

		case WM_NOTIFY:
			ASSERT(lParam);
			switch (((LPNMHDR) lParam)->code)
			{
				case TTN_GETDISPINFO:
				{
					ASSERT(TTF_IDISHWND & ((LPNMTTDISPINFO) lParam)->uFlags);

					HWND hwndItem = NULL;
					RECT rcItem = {0};
					HDC hdc = NULL;
					int textLen = 0;
					TCHAR * pszText = NULL;
					SIZE size = {0};

					hwndItem = GetDlgItem(hDlg, IDC_STATIC_PROCESSOR);
					ASSERT(hwndItem);
					GetClientRect(hwndItem, &rcItem);

					if (hdc = GetDC(hwndItem))
					{
						textLen = SendMessage(hwndItem, WM_GETTEXTLENGTH, NULL, NULL);
						if (textLen && (pszText = new TCHAR[textLen+1]))
						{

							SendMessage(hwndItem, WM_GETTEXT, textLen+1, (LPARAM) pszText);

							if (GetTextExtentPoint(hdc, pszText, textLen, &size) &&
							    (size.cx >= rcItem.right))
							{
								LPNMTTDISPINFO lpnmttdi = (LPNMTTDISPINFO) lParam;
								ZeroMemory(lpnmttdi->szText, sizeof(lpnmttdi->szText));
								_tcsncpy(lpnmttdi->szText, pszText, 79); // See NMTTDISPINFO in MSDN
							}
							delete [] pszText;
						}
						ReleaseDC(hwndItem, hdc);
					}
					return TRUE;
				}

				case TTN_SHOW:
				{
					HWND hwndItem = (HWND) ((LPNMHDR) lParam)->idFrom;
					ASSERT(hwndItem);

					RECT rcItem = {0};
					GetWindowRect(hwndItem, &rcItem);

					RECT rcToolTip = {0};
					GetWindowRect(g_hToolTip, &rcToolTip);
					LONG lToolTipWidth = rcToolTip.right - rcToolTip.left;

					HMONITOR hMonitor = MonitorFromWindow(hwndItem, MONITOR_DEFAULTTONEAREST);
					ASSERT(hMonitor);
					if (hMonitor)
					{
						MONITORINFO mi = {0};
						mi.cbSize = sizeof(mi);
						GetMonitorInfo(hMonitor, &mi);

						if (mi.rcWork.right < (rcItem.left+lToolTipWidth))
						{
							rcItem.left -= (rcItem.left+lToolTipWidth)-mi.rcWork.right;
						}
					}

					// TTM_ADJUSTRECT is not supported on CE so just assume +=2
					// SendMessage(g_hToolTip, TTM_ADJUSTRECT, TRUE, (LPARAM) &rc);
					rcItem.top += 2;
					rcItem.left += 2;

					SetWindowPos(g_hToolTip, NULL, rcItem.left, rcItem.top,
					             0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
					SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);
					return TRUE;
				}
			}
			break;

	}
    return (FALSE);
}

extern "C" BOOL APIENTRY MemoryDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{

    MemoryState *pMemoryState;

    pMemoryState = (MemoryState *)GetWindowLong(hDlg,DWL_USER);

    // if we don't have our state, don't process
    if(message != WM_INITDIALOG && !pMemoryState)
        return FALSE;

    switch (message)
    {
	    case WM_INITDIALOG:
	    {
			TCHAR  szTemp[100];
	    	DWORD PageSize, TotPages, StoreUsed, RamUsed, StorePages;
			GetObjStoreFolder( ObjStoreFolder );
			GetMemoryInfo(PageSize, TotPages, StoreUsed, RamUsed, StorePages);

            pMemoryState = new MemoryState;
            if(!pMemoryState)
            {
                ASSERT(0);  // Couldn't create persistant storage
                return FALSE;
            }
            else
            {
                ZEROMEM(pMemoryState);
                SetWindowLong(hDlg,DWL_USER,(LONG)pMemoryState);
            }
            
			pMemoryState->fSliderMoved = FALSE;

			// save this for restore if we fail
			pMemoryState->orgStorePages = StorePages;
			
			// Init the slider
			HWND hwndSlider = GetDlgItem(hDlg, IDC_TRACKBAR);
			WORD wMax = (WORD) (TotPages / TICK_RESOLUTION);
			SendMessage(hwndSlider, TBM_SETRANGE, TRUE, MAKELONG(1, wMax)); 
			SendMessage(hwndSlider, TBM_SETTICFREQ, (wMax / 75), 0L);			  // tick freq.
			SendMessage(hwndSlider, TBM_SETPAGESIZE, 0L, 3);			  // page up/dn size
			SendMessage(hwndSlider, TBM_SETPOS,TRUE, StorePages / TICK_RESOLUTION);	  

			// init the current allocations
			_snwprintf(szTemp, ARRAYSIZE(szTemp) -1, CELOADSZ(IDS_SYSTEM_MEMKBYTE), StorePages * PageSize);
			szTemp[ARRAYSIZE(szTemp) -1] = 0;
			SetDlgItemText(hDlg, IDC_STATIC_LEFT, szTemp);

			_snwprintf(szTemp, ARRAYSIZE(szTemp) -1, CELOADSZ(IDS_SYSTEM_MEMKBYTE), (TotPages-StorePages) * PageSize);
			szTemp[ARRAYSIZE(szTemp) -1] = 0;
			SetDlgItemText(hDlg, IDC_STATIC_RIGHT, szTemp);

			// init timer so we can update
			pMemoryState->idTimer = SetTimer(hDlg, 2, 1500, NULL);
			MemoryDlgOnTimer(hDlg,pMemoryState);

            AygInitDialog( hDlg, SHIDIF_SIPDOWN );
			return 1;
		}

		case WM_TIMER:
			MemoryDlgOnTimer(hDlg,pMemoryState);
			return TRUE;
		
		case WM_HSCROLL:			// track bar message
			switch LOWORD(wParam) {
				case TB_BOTTOM:
				case TB_THUMBPOSITION:
				case TB_LINEUP:
				case TB_LINEDOWN:
				case TB_PAGEUP:
				case TB_PAGEDOWN:
				case TB_TOP:
					pMemoryState->fSliderMoved = TRUE;
					MemoryDlgOnSlider(hDlg);
					return (TRUE);
				}
			break;

	    case WM_COMMAND:
            switch (LOWORD(wParam))
            {
				case IDOK:
				{
					if(pMemoryState->fSliderMoved) {
						MemoryDlgOnOK(hDlg, pMemoryState->orgStorePages);
					}
					if (pMemoryState->idTimer)  {
						KillTimer(hDlg, pMemoryState->idTimer);
						pMemoryState->idTimer = 0;
					}
					return TRUE;
		    	}
		    }
			break;
		case WM_SYSCOLORCHANGE:
			// fwd the syscolorchange to our trackbar
			SendMessage(DI(IDC_TRACKBAR), message, wParam, lParam);
			break; // return FALSE anyway so DefDlgProc handles it for ourselves

	    case WM_DESTROY:
            // we won't get this far if the 
		    delete pMemoryState;
		    SetWindowLong(hDlg,DWL_USER,0L);
		    break;
	}
    return (FALSE);
}


extern "C" BOOL APIENTRY SystemIdentDlgProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    CReg    reg;
    static TCHAR    szBuf[MAX_STRING+1];
    static HINSTANCE   hWinSock;
    HOSTPROC SetHostProc = NULL;
    HOSTPROC GetHostProc = NULL;
	WSADATA	wsaData;

    if(message != WM_INITDIALOG)
        SetHostProc = (HOSTPROC)GetWindowLong(hDlg,DWL_USER);
    
    switch (message)     
    {

	    case WM_INITDIALOG:
	    {
	        char      szAnsiName[MAX_STRING+1];

	        DEBUGMSG(ZONE_SYSTEM, (L"SystemIdentDlgProc: Got WM_INITDLG....\r\n"));

	        // load 
	        hWinSock = LoadLibrary(TEXT("winsock.dll"));
	        if(hWinSock)  
	        {
	            GetHostProc = (HOSTPROC)GetProcAddress(hWinSock,TEXT("gethostname"));
	            SetHostProc = (HOSTPROC)GetProcAddress(hWinSock,TEXT("sethostname"));

				pfnWSAStartup = (PFNWSASTARTUP)GetProcAddress(hWinSock,TEXT("WSAStartup"));
				pfnWSACleanup = (PFNWSACLEANUP)GetProcAddress(hWinSock,TEXT("WSACleanup"));
	        }
			else
			{
				// net components aren't loaded
				SetDeviceDependentText(hDlg, IDC_IDENTIFY, IDS_SYSTEM_ID_NONET);
				EnableWindow(DI(IDC_EDIT_NAME), FALSE);
				EnableWindow(DI(IDC_EDIT_DESCR), FALSE);
				return 1;  //no other processing needed
			}

	        SetWindowLong(hDlg,DWL_USER,(LONG)SetHostProc);

	        //Make the text device-neutral. 
	        SetDeviceDependentText(hDlg, IDC_IDENTIFY, IDS_SYSTEM_IDENTIFY);
	        Edit_LimitText(DI(IDC_EDIT_NAME), MAX_NAME);
	        Edit_LimitText(DI(IDC_EDIT_DESCR), MAX_DESC);

	        ImmAssociateContext(GetDlgItem(hDlg, IDC_EDIT_NAME), NULL);
	         
	        // open the registry key for reading the device description
	        reg.Open(HKEY_LOCAL_MACHINE, RK_IDENT);

			// Start up winsock									
	        if(GetHostProc && pfnWSAStartup && pfnWSACleanup &&
				!pfnWSAStartup(wVersionRequested, &wsaData) &&
				!GetHostProc(szAnsiName,MAX_STRING))
	        {
	            // convert the name to unicode
	            MultiByteToWideChar(CP_ACP, 0 ,szAnsiName, -1, szBuf, MAX_STRING);
	            SetDlgItemText(hDlg, IDC_EDIT_NAME, szBuf); 
				
				pfnWSACleanup();

	        }
	        SetDlgItemText(hDlg, IDC_EDIT_DESCR, reg.ValueSZ(RV_DESC));

            LoadAygshellLibrary();
            AygAddSipprefControl( hDlg );

	        return 1; // let GWE set focus
	    }

    case WM_NOTIFY:
        {
			ASSERT(lParam);
			switch (((LPNMHDR) lParam)->code)
			{
				case PSN_SETACTIVE:
                    // set focus to the name to raise the sip
                    PostMessage( DI(IDC_EDIT_NAME), WM_SETFOCUS, WPARAM(0), LPARAM(0) );
                    break;
                case PSN_KILLACTIVE:
                    PostMessage( DI(IDC_EDIT_NAME), WM_KILLFOCUS, WPARAM(0), LPARAM(0) );
                    break;
                default:
                    // do nothing
                    break;
            }
        }
        break;
            
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
	        case IDOK:
	        {
	            DEBUGMSG(ZONE_SYSTEM, (L"SystemIdentDlgProc: Got IDOK....\r\n"));
                   TCHAR szCompName[MAX_NAME + 1];
				   TCHAR szBufTmp[MAX_DESC + 1] = {0};

                   GetDlgItemText(hDlg, IDC_EDIT_NAME, szCompName, MAX_NAME + 1);
                   
					reg.Open(HKEY_LOCAL_MACHINE, RK_IDENT);
					GetDlgItemText(hDlg, IDC_EDIT_DESCR, szBufTmp, MAX_DESC);
					reg.SetSZ(RV_DESC, szBufTmp);
                   
				   if (lstrcmpi(szBuf, szCompName))  //only need to do this stuff if the name has changed
                   {
        	            if(!CheckDeviceName(SetHostProc, hDlg, IDC_EDIT_NAME, MAX_STRING))
        	            {
                             // sethostname writes the value to the registry even on failure, so revert the text
                             // in both the registry and the edit box
                             
                             reg.SetSZ(RV_NAME, szBuf);
                             SetDlgItemText(hDlg, IDC_EDIT_NAME, szBuf);  
                             return FALSE;    // force propsheet to stay open
        	            }    
        	            else
        	            {
        	                return TRUE;
    	                }
					}
					else
					{
						return TRUE;
					}
	        }
        }
        break;
     case WM_DESTROY:
        {
			//if (hWinSock)
			//   FreeLibrary(hWinSock);
            FreeAygshellLibrary();
        }
        break;
    }
    return (FALSE);
}



BOOL ValidName(LPTSTR pszBuf)
{

    if(!pszBuf)
        return FALSE;

    // check the beginning
    if(pszBuf[0] == 0)
        return FALSE;
    if( !((pszBuf[0] >= TEXT('a') && pszBuf[0] <= TEXT('z')) 
                            || (pszBuf[0] >= TEXT('A') && pszBuf[0] <= TEXT('Z'))))
    {
        return FALSE;
    }

    // check the middle
    int i=1;
    while(pszBuf[i])
    {
        if(i >= MAX_NAME)
            return FALSE;
        if(pszBuf[i] != TEXT('-') && 
           pszBuf[i] != TEXT('_') && 
           !((pszBuf[i] >= TEXT('a') && pszBuf[i] <= TEXT('z'))
              || (pszBuf[i] >= TEXT('A') && pszBuf[i] <= TEXT('Z'))
              || (pszBuf[i] >= TEXT('0') && pszBuf[i] <= TEXT('9'))))
        {
            return FALSE;
        }    
        i++;
    }
    // check the end
    if(pszBuf[i-1] == TEXT('-') || pszBuf[i-1] == TEXT('_'))
        return FALSE;
    return TRUE;
}



BOOL CheckDeviceName(HOSTPROC SetHostProc, HWND hDlg, int idcEdit, int iMax)
{
    LPTSTR  pszBuf = MySzAlloca(iMax); // alloca freed automatically on fn exit
    char    *szAnsiName = (char *)alloca(1+iMax);
    CReg    reg;
    int     status;
	WSADATA	wsaData;

    pszBuf[0] = 0;
    GetDlgItemText(hDlg, idcEdit, pszBuf, iMax);

    if( !ValidName(pszBuf))
    {
        MessageBox(hDlg, CELOADSZ(IDS_CPLERR_DEVICENAME), CELOADSZ(IDS_CPLERR_DEVICETITLE), MB_ICONEXCLAMATION);
        SetFocus(GetDlgItem(hDlg, idcEdit));
        Edit_SetSel(DI(idcEdit), 0L, -1);    
        return FALSE;
    }            
    
    // if there is no winsock, just set the registry
    if(!SetHostProc)
    {
        reg.Create(HKEY_LOCAL_MACHINE, RK_IDENT);
        if(!reg.SetSZ(RV_NAME, pszBuf))
        {
            MessageBox(hDlg, CELOADSZ(IDS_CPLERR_MEMORY_ERROR), CELOADSZ(IDS_CPLERR_LOWMEM_TITLE), MB_ICONEXCLAMATION);
        }
        return TRUE; 
    }

    // make it ansi compatible
    WideCharToMultiByte (CP_ACP, 0, pszBuf, -1, szAnsiName, iMax, NULL, NULL);

    DECLAREWAITCURSOR;
    SetWaitCursor();

	if(pfnWSAStartup && pfnWSACleanup &&
		!pfnWSAStartup(wVersionRequested, &wsaData)) {
		// check the string by calling sethostname
		if(status = SetHostProc (szAnsiName, strlen(szAnsiName) + 1))
		{
			ResetWaitCursor();
			SetFocus(GetDlgItem(hDlg, idcEdit));
			Edit_SetSel(DI(idcEdit), 0L, -1);

			pfnWSACleanup();
			return FALSE;
		}

		pfnWSACleanup();
	}
	
    ResetWaitCursor();
    return TRUE;
}


#define GetRange(min, max) \
	min = (StoreUsed + PAGES_RESERVED_STG ) / TICK_RESOLUTION; \
	max = (TotPages - (RamUsed + PAGES_RESERVED_SYS)) / TICK_RESOLUTION;


void MemoryDlgOnTimer(HWND hDlg,MemoryState *pmst)
{

	TCHAR  szTemp[100];

	DWORD PageSize, TotPages, StoreUsed, RamUsed, StorePages;
	GetMemoryInfo(PageSize, TotPages, StoreUsed, RamUsed, StorePages);

	int min, max;
	GetRange(min, max);

	HWND hwndSlider = GetDlgItem(hDlg, IDC_TRACKBAR);

	DEBUGMSG(ZONE_SYSTEM, (L"min=%d max=%d \r\n",min, max));
		
	if (pmst->oldMin != min || pmst->oldMax != max)	{		// prevent flashing
		SendMessage(hwndSlider, TBM_SETSEL,TRUE, MAKELONG((WORD)min, (WORD)max)); //in 4KB*TICK_RESOLUTION
		pmst->oldMin = min;
		pmst->oldMax = max;
	}

	if (pmst->oldStoreUsed != StoreUsed) {	// prevent flashing
		_snwprintf(szTemp,ARRAYSIZE(szTemp) -1, CELOADSZ(IDS_SYSTEM_MEMKBYTE), StoreUsed*PageSize);			
		szTemp[ARRAYSIZE(szTemp) -1] = 0;
		SetDlgItemText(hDlg, IDC_IN_USE_LEFT, szTemp);
		pmst->oldStoreUsed = StoreUsed;
	}

	if (pmst->oldRamUsed != RamUsed) {			// prevent flashing
		_snwprintf(szTemp, ARRAYSIZE(szTemp) -1, CELOADSZ(IDS_SYSTEM_MEMKBYTE), RamUsed*PageSize);			
		szTemp[ARRAYSIZE(szTemp) -1] = 0;
		SetDlgItemText(hDlg, IDC_IN_USE_RIGHT, szTemp);
		pmst->oldRamUsed = RamUsed;
	}
}

void MemoryDlgOnSlider(HWND hDlg)
{
	TCHAR  szTemp[100];
	DWORD PageSize, TotPages, StoreUsed, RamUsed, StorePages;
	GetMemoryInfo(PageSize, TotPages, StoreUsed, RamUsed, StorePages);

	int min, max;
	GetRange(min, max);

	HWND hwndSlider = GetDlgItem(hDlg, IDC_TRACKBAR);
	int pos = SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
	if (pos < (min + 32/(int)(PageSize*TICK_RESOLUTION))) 	 // allow at least 32KB of room for storage
		pos = (min + 32/(PageSize*TICK_RESOLUTION));
	else if (pos > max - (PAGES_RESERVED_SHL/TICK_RESOLUTION)) 
		pos = max - (PAGES_RESERVED_SHL/TICK_RESOLUTION);
										
	SendMessage(hwndSlider, TBM_SETPOS,TRUE, pos);	  // keep in limits 
					
	pos *= TICK_RESOLUTION;

	_snwprintf(szTemp, ARRAYSIZE(szTemp) -1, CELOADSZ(IDS_SYSTEM_MEMKBYTE), pos * PageSize);
	szTemp[ARRAYSIZE(szTemp) -1] = 0;
	SetDlgItemText(hDlg, IDC_STATIC_LEFT, szTemp);
        
	_snwprintf(szTemp, ARRAYSIZE(szTemp) -1, CELOADSZ(IDS_SYSTEM_MEMKBYTE), (TotPages-pos) * PageSize);
	szTemp[ARRAYSIZE(szTemp) -1] = 0;
	SetDlgItemText(hDlg, IDC_STATIC_RIGHT, szTemp);
}

void MemoryDlgOnOK(HWND hDlg, DWORD orgStorePages)
{
	HWND hwndSlider = GetDlgItem(hDlg, IDC_TRACKBAR);
	int pos = SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
	pos *= TICK_RESOLUTION;

	DWORD PageSize, TotPages, StoreUsed, RamUsed, StorePages;
	GetMemoryInfo(PageSize, TotPages, StoreUsed, RamUsed, StorePages);

	int min, max;
	GetRange(min, max);

	min = min*TICK_RESOLUTION;
	max = max*TICK_RESOLUTION;

	DEBUGMSG(ZONE_SYSTEM, (L"Before SetSystemMemoryDivision min=%d max=%d required setting=%d\r\n", min,max,pos));
	if(pos > max - PAGES_RESERVED_SHL) {
		pos = max - PAGES_RESERVED_SHL;
	}

	if(!SetSystemMemoryDivision(pos))
		return; // success!

	DEBUGMSG(ZONE_SYSTEM, (L"SetSystemMemoryDivision FAILED due to fragmentation\r\n"));
		
	// if failed try to find a close setting that will work
	min += PAGE_INCR;		// narrow limits by 16 pages
	max -= PAGE_INCR;

	// Now we iterate in steps of 16 pages. At each step we first try a setting that 
	// gives us i*16 more pages of program memory than requested, then i*16 pages more
	// of storage memory. If we fall of either end, give up
	for(int i=1; ;i++)
	{
		int pos1 = pos - (i*PAGE_INCR); // try on left of desired divison
		int pos2 = pos + (i*PAGE_INCR); // try on right of desired divison
		BOOL fKeepTrying = FALSE;

		DEBUGMSG(ZONE_SYSTEM, (L"Fragmentation retry: i=%d min=%d max=%d pos1=%d pos2=%d\r\n", i, min, max, pos1, pos2));

		// first try to increase program memory													 
		if(pos1>=min && pos1<=max)
		{
			if(!SetSystemMemoryDivision(pos1))
			{
				// succeeded--if we are close to max system memory add a additional 40 pages that is needed by GWE/Shell to start ControlPanel/System again
				DEBUGMSG(ZONE_SYSTEM, (L"Fragmentation retry SUCCESS left: i=%d min=%d max=%d set=%d\r\n", i, min, max, pos1));
				pos1 -= PAGES_RESERVED_SHL;
				DWORD dwRet = SetSystemMemoryDivision(pos1);
				ASSERT(!dwRet); // this must succeed!!
				DEBUGMSG(ZONE_SYSTEM, (L"Fragmentation retry SUCCESS left: i=%d min=%d max=%d really set=%d\r\n", i, min, max, pos1));
				pos = pos1;
				break;
			}
			DEBUGMSG(ZONE_SYSTEM, (L"Fragmentation retry FAILED left: i=%d min=%d max=%d pos1=%d pos2=%d\r\n", i, min, max, pos1, pos2));
			fKeepTrying = TRUE;
		}
			
		// now try to increase storage memory													 
		if(pos2>=min && pos2<=max) 
		{  
			if(!SetSystemMemoryDivision(pos2))
			{
				DEBUGMSG(ZONE_SYSTEM, (L"Fragmentation retry SUCCESS right: i=%d min=%d max=%d set=%d\r\n", i, min, max, pos2));
				pos = pos2;
				break;
			}
			DEBUGMSG(ZONE_SYSTEM, (L"Fragmentation retry FAILED right: i=%d min=%d max=%d pos1=%d pos2=%d\r\n", i, min, max, pos1, pos2));
			fKeepTrying = TRUE;
		}
		
		if(!fKeepTrying)
		{
			DEBUGMSG(ZONE_SYSTEM, (L"Fragmentation retry TOTALLY FAILED: i=%d min=%d max=%d pos1=%d pos2=%d\r\n", i, min, max, pos1, pos2));
			// time to give up & exit from loop. Restore pos to original 
			// value so user will see that we didnt change anything
			pos = orgStorePages;
			break;
		}
	}
		
#ifdef DEBUG
	DWORD RamPages;
	GetSystemMemoryDivision(&StorePages, &RamPages, &PageSize);
	DEBUGMSG(ZONE_SYSTEM, (L"After GetSystemMemoryDivision store=%d krnel=%d\r\n", StorePages,RamPages));
#endif
	// Update display & pause a second for user to see what we actually did
	SendMessage(hwndSlider, TBM_SETPOS,TRUE, pos/TICK_RESOLUTION);	   //reset slider					
	UpdateWindow(hDlg);
	Sleep(1000);			// let user see the new settings for 1 sec

	// put up warning
	MessageBox(hDlg, CELOADSZ(IDS_SYSTEM_MEMFAIL), CELOADSZ(IDS_SYSTEM_MEMERRTITLE), MB_OK | MB_ICONWARNING);
}

