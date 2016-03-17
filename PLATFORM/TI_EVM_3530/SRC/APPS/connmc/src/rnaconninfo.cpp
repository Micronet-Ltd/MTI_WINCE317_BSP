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
#include "rnaconninfo.h"
#include "connmc.h" // For RNM_SHOWERROR

////////////////////////
// RnaConnInfo
////////////////////////
#define BLUETOOTH_CHECK_FILE				TEXT("\\Windows\\btd.dll")

CFormattedEdit RnaConnInfo::FormattedEdit;
UINT RnaConnInfo::iconID = (UINT) -1;
const UINT RnaConnInfo::CNTRYRGN_CODE_SIZE = 16;

RnaConnInfo::RnaConnInfo(TCHAR * pszName) :
RasConnInfo(pszName)
{
    // This means that we are not a Modem, make ourselves one here
    if (::_tcscmp(RasEntry.szDeviceType, RASDT_Modem))
    {
        LPRASDEVINFO pRasDevInfo = NULL;
        DWORD dwRasDevices = GetDevices(&pRasDevInfo);
        for (DWORD i=0; (i < dwRasDevices) && pRasDevInfo; i++)
        {
            if (!_tcscmp(pRasDevInfo[i].szDeviceType, RASDT_Modem))
            {
                ::_tcscpy(RasEntry.szDeviceName, pRasDevInfo[i].szDeviceName);
                ::_tcscpy(RasEntry.szDeviceType, RASDT_Modem);
                break;
            }
        }
        if (pRasDevInfo)
            delete [] pRasDevInfo;
    }
}

void RnaConnInfo::getTypeAsDisplayString(HINSTANCE hInstance, TCHAR * pszType,
                                         DWORD cchType) const
{
    ::LoadString(hInstance, IDS_RAS_MODEM, pszType, cchType);
}

HPROPSHEETPAGE RnaConnInfo::getPropPage(UINT uPage, HINSTANCE hInstance)
{
    if (!pfnCreatePropertySheetPage)
        return NULL;
    
    PROPSHEETPAGE psp = {0};
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.hInstance = hInstance;
    switch (uPage)
    {
    case 0:
        psp.pszTemplate = MAKEINTRESOURCE(g_fPortrait?IDD_RNA_0_P:IDD_RNA_0);
        psp.pfnDlgProc = RnaConnInfo::RnaConnPropSheetProc0;
        break;
    case 1:
        psp.pszTemplate = MAKEINTRESOURCE(g_fPortrait?IDD_RNA_1_P:IDD_RNA_1);
        psp.pfnDlgProc = RnaConnInfo::RnaConnPropSheetProc1;
        break;
    default: return NULL;
    }
    psp.lParam = reinterpret_cast<LPARAM>(this);
    
    return (*pfnCreatePropertySheetPage)(&psp);
}


BOOL CALLBACK RnaConnInfo::RnaConnPropSheetProc0(HWND hDlg, UINT uMsg,
                                                 WPARAM wParam, LPARAM lParam)
{
    PROPSHEETPAGE * ppsp = reinterpret_cast<PROPSHEETPAGE *>(GetWindowLong(hDlg, GWL_USERDATA));
    RnaConnInfo * pRnaConn = NULL;
    if (ppsp) pRnaConn = reinterpret_cast<RnaConnInfo *>(ppsp->lParam);
    
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            ::SetWindowLong(hDlg, GWL_USERDATA, lParam);
            ppsp = reinterpret_cast<PROPSHEETPAGE *>(lParam);
            ASSERT(ppsp);
            pRnaConn = reinterpret_cast<RnaConnInfo *>(ppsp->lParam);
            ASSERT(pRnaConn);
            
            if ((GetFileAttributes(BLUETOOTH_CHECK_FILE) != -1))
            {
                // if bluetooth is enabled in the image then we want to show this button
                ShowWindow(::GetDlgItem(hDlg, IDC_BLUETOOTH), SW_SHOW);
            }
            
            
            // Fill in the modem list box
            LPRASDEVINFO pRasDevInfo = NULL;
            DWORD dwRasDevices = GetDevices(&pRasDevInfo);
            for (DWORD i=0; i < dwRasDevices; i++)
            {
                if (!_tcscmp(pRasDevInfo[i].szDeviceType, RASDT_Modem))
                {
                    ::SendDlgItemMessage(hDlg, IDC_DEVICE, CB_ADDSTRING, 0,
                        (LPARAM) pRasDevInfo[i].szDeviceName);
                }
            }
            if (pRasDevInfo)
                delete [] pRasDevInfo;
            
            // Try to find the modem in the list.
            LRESULT lResult = ::SendDlgItemMessage(hDlg, IDC_DEVICE, CB_FINDSTRINGEXACT,
                0, (LPARAM) pRnaConn->RasEntry.szDeviceName);
            ASSERT(CB_ERR != lResult);
            ::SendMessage(::GetDlgItem(hDlg, IDC_DEVICE), CB_SETCURSEL,
                (CB_ERR != lResult) ? lResult : 0, 0);
            
            // Modem combo box gets focus by default
            ::SetFocus(::GetDlgItem(hDlg, IDC_DEVICE));
            
            return FALSE; // We will set our own focused control
        }
        break;
        
    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case IDC_BLUETOOTH:
                {
                    typedef int (*PFN_CreateScanDevice) (HINSTANCE hInst, HWND hWnd);
                    
                    HMODULE hNetUI = LoadLibrary(_T("netui.dll"));
                    if(hNetUI)
                    {
                        PFN_CreateScanDevice pCreateScanDevice = (PFN_CreateScanDevice) GetProcAddress(hNetUI, _T("CreateScanDevice"));
                        if(pCreateScanDevice)
                        {
                            pCreateScanDevice(hNetUI, (HWND)hDlg);
                            PostMessage(hDlg, WM_DEVICECHANGE, 0, 0);
                        }
                        
                        FreeLibrary(hNetUI);
                    }
                }break;
                
            case IDC_CONFIG:
                if (pRnaConn)
                {
                    pRnaConn->handleDevConfig(hDlg);
                }else{
                    DEBUGMSG (ZONE_ERROR,  (TEXT("ConnMC: RnaConnInfo::RnaConnPropSheetProc0 tried to use null pRnaConn\r\n")));
                }
                break;
                
            case IDC_TCPSETTINGS:
                if (pRnaConn)
                {
                    pRnaConn->ConfigureTcpIp(hDlg, ppsp->hInstance);
                }else{
                    DEBUGMSG (ZONE_ERROR,  (TEXT("ConnMC: RnaConnInfo::RnaConnPropSheetProc0 tried to use null pRnaConn\r\n")));
                }
                break;
                
            case IDC_SECSETTINGS:
                if (pRnaConn)
                {
                    pRnaConn->ConfigureSecurity(hDlg, ppsp->hInstance);
                }else{
                    DEBUGMSG (ZONE_ERROR,  (TEXT("ConnMC: RnaConnInfo::RnaConnPropSheetProc0 tried to use null pRnaConn\r\n")));
                }
                break;
                
            case IDC_DEVICE:
                if (pRnaConn)
                {
                    WCHAR wszNewDeviceName[RAS_MaxDeviceName+1];
                    
                    wszNewDeviceName[0] = L'\0';
                    ::GetWindowText(::GetDlgItem(hDlg, IDC_DEVICE),
                        wszNewDeviceName,
                        RAS_MaxDeviceName+1);
                    if (0 != (wcscmp(pRnaConn->RasEntry.szDeviceName, wszNewDeviceName)))
                    {
                        // If the device name changed then any prior devconfig
                        // info is no longer valid, so free it.
                        //
                        // Note that if connmc wanted to be really smart, it could maintain
                        // a cache of the devconfig info on a per-DeviceName basis. That way,
                        // if the user changes the DeviceName back to one he previously
                        // configured, the devconfig info for that device could be restored.
                        //
                        wcscpy(pRnaConn->RasEntry.szDeviceName, wszNewDeviceName);
                        delete [] pRnaConn->pDevConfig;
                        pRnaConn->pDevConfig = NULL;
                        pRnaConn->cbDevConfig = 0;
                    }
                }else{
                    DEBUGMSG (ZONE_ERROR,  (TEXT("ConnMC: RnaConnInfo::RnaConnPropSheetProc0 tried to use null pRnaConn\r\n")));
                }
                
                break;
            }
        }
        break;
        
    case WM_NOTIFY:
        {
            if (!pRnaConn)
            {
                DEBUGMSG (ZONE_ERROR,  (TEXT("ConnMC: RnaConnInfo::RnaConnPropSheetProc0 tried to use null pRnaConn\r\n")));
                return FALSE;
            }
            
            PSHNOTIFY * ppshn = (PSHNOTIFY *) lParam;
            switch (ppshn->hdr.code)
            {
            case PSN_SETACTIVE:
                {
                    PropSheet_SetWizButtons(::GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                    
                    // Set the connection name.
                    ::SetWindowText(::GetDlgItem(hDlg, IDC_CONNLABEL), pRnaConn->pszName);
                }
                break;
                
            case PSN_KILLACTIVE:
                {
                    ::GetWindowText(::GetDlgItem(hDlg, IDC_DEVICE),
                        pRnaConn->RasEntry.szDeviceName,
                        RAS_MaxDeviceName+1);
                }
                break;
                
            case PSN_APPLY:
                {
                    if (!pRnaConn) {
                        // pRnaConn shouldn't be NULL
                        return FALSE;
                    }
                    
                    pRnaConn->writeChanges();
                }
                break;
            }
            ::SetWindowLong(hDlg, DWL_MSGRESULT, 0);
        }
        break;
        
    case WM_DEVICECHANGE:
        {
            if (pRnaConn)
            {
                // Refresh the modem list box
                HCURSOR hCursorSave = SetCursor(LoadCursor(NULL, IDC_WAIT));
                ::SendDlgItemMessage(hDlg, IDC_DEVICE, CB_RESETCONTENT, 0, 0);
                LPRASDEVINFO pRasDevInfo = NULL;
                DWORD dwRasDevices = GetDevices(&pRasDevInfo);
                
                for (DWORD i=0; (i < dwRasDevices) && pRasDevInfo; i++)
                {
                    if (!_tcscmp(pRasDevInfo[i].szDeviceType, RASDT_Modem))
                    {
                        ::SendDlgItemMessage(hDlg, IDC_DEVICE, CB_ADDSTRING, 0,
                            (LPARAM) pRasDevInfo[i].szDeviceName);
                    }
                }
                if (pRasDevInfo)
                    delete [] pRasDevInfo;
                
                // Try to find the modem in the list.
                LRESULT lResult = ::SendDlgItemMessage(hDlg, IDC_DEVICE, CB_FINDSTRINGEXACT,
                    0, (LPARAM) pRnaConn->RasEntry.szDeviceName);
                ASSERT(CB_ERR != lResult);
                ::SendMessage(::GetDlgItem(hDlg, IDC_DEVICE), CB_SETCURSEL,
                    (CB_ERR != lResult) ? lResult : 0, 0);
                SetCursor(hCursorSave);
            }else{
                DEBUGMSG (ZONE_ERROR,  (TEXT("ConnMC: RnaConnInfo::RnaConnPropSheetProc0 tried to use null pRnaConn\r\n")));
            }
            
        }
        break;
        
    default:
        return FALSE;
   }
   
   return TRUE;
}


BOOL CALLBACK RnaConnInfo::RnaConnPropSheetProc1(HWND hDlg, UINT uMsg,
                                                 WPARAM wParam, LPARAM lParam)
{
    PROPSHEETPAGE * ppsp = reinterpret_cast<PROPSHEETPAGE *>(GetWindowLong(hDlg, GWL_USERDATA));
    RnaConnInfo * pRnaConn = NULL;
    if (ppsp) pRnaConn = reinterpret_cast<RnaConnInfo *>(ppsp->lParam);
    
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            ::SetWindowLong(hDlg, GWL_USERDATA, lParam);
            ppsp = reinterpret_cast<PROPSHEETPAGE *>(lParam);
            ASSERT(ppsp);
            pRnaConn = reinterpret_cast<RnaConnInfo *>(ppsp->lParam);
            ASSERT(pRnaConn);
            
            LPLINETRANSLATECAPS pCaps = RasConnInfo::GetTranslateCaps(ppsp->hInstance);
            if (pCaps)
            {
                if (pRnaConn->RasEntry.dwCountryCode == 0)
                    pRnaConn->RasEntry.dwCountryCode = RasConnInfo::GetDefaultCntryRgnCode(pCaps);
                
                if (pRnaConn->RasEntry.szAreaCode[0] == _T('\0'))
                    RasConnInfo::GetDefaultAreaCode(pCaps, pRnaConn->RasEntry.szAreaCode,
                    RAS_MaxAreaCode);
                
                ::LocalFree(pCaps);
            }
            
            // Limit the text fields to resonable limits
            ::SendMessage(::GetDlgItem(hDlg, IDC_AREA_CODE), EM_LIMITTEXT, RAS_MaxAreaCode, 0);
            ::SendMessage(::GetDlgItem(hDlg, IDC_PHONE_NUM), EM_LIMITTEXT, RAS_MaxPhoneNumber, 0);
            ::SendMessage( ::GetDlgItem(hDlg, IDC_CNTRYRGN), EM_LIMITTEXT, CNTRYRGN_CODE_SIZE,	0);
            
            // Set Phone Number
            ::SetWindowText(::GetDlgItem(hDlg, IDC_AREA_CODE), pRnaConn->RasEntry.szAreaCode);
            ::SetWindowText(::GetDlgItem(hDlg, IDC_PHONE_NUM), pRnaConn->RasEntry.szLocalPhoneNumber);		
            
            // Initialize country/region code list box.
            TCHAR szCntryRgnCode[CNTRYRGN_CODE_SIZE];
            szCntryRgnCode[0] = TEXT('\0');
            if (pRnaConn->RasEntry.dwCountryCode)
                wsprintf(szCntryRgnCode, TEXT("%u"), pRnaConn->RasEntry.dwCountryCode);
            ::SetWindowText(::GetDlgItem(hDlg, IDC_CNTRYRGN), szCntryRgnCode);
            
            // Initialize the use country/region check box
            if (pRnaConn->RasEntry.dwfOptions & RASEO_UseCountryAndAreaCodes)
                ::SendDlgItemMessage(hDlg, IDC_FORCELD, BM_SETCHECK, 1, 0);
            
            if (pRnaConn->RasEntry.dwfOptions & RASEO_DialAsLocalCall)
                ::SendDlgItemMessage(hDlg, IDC_FORCELOCAL, BM_SETCHECK, 1, 0);
            
            // Associate IMM with these controls
            ImmAssociateContext(::GetDlgItem(hDlg, IDC_CNTRYRGN), (HIMC)NULL);
            ImmAssociateContext(::GetDlgItem(hDlg, IDC_AREA_CODE), (HIMC)NULL);
            ImmAssociateContext(::GetDlgItem(hDlg, IDC_PHONE_NUM), (HIMC)NULL);
            

			// Try to attach a formater to the phone number.  If this failes for any reason we 
			// don't care and will go with the default edit behaviour
			FormattedEdit.Attach(L"1234567890*#\b", hDlg, IDC_PHONE_NUM);

            // Phone number box gets focus by default
            ::SetFocus(::GetDlgItem(hDlg, IDC_PHONE_NUM));
            
            return FALSE; // We will set our own focused control
        }
        break;

	case WM_DESTROY:
		{
			FormattedEdit.Detach();

			return FALSE;
		}break;
        
    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case IDC_FORCELD:
                {
                    if (BST_CHECKED == ::SendDlgItemMessage(hDlg, IDC_FORCELD,
                        BM_GETCHECK, 0, 0))
                    {
                        ::SendDlgItemMessage(hDlg, IDC_FORCELOCAL, BM_SETCHECK,
                            BST_UNCHECKED, 0);
                    }
                }
                break;
                
            case IDC_FORCELOCAL:
                {
                    if (BST_CHECKED == ::SendDlgItemMessage(hDlg, IDC_FORCELOCAL,
                        BM_GETCHECK, 0, 0))
                    {
                        ::SendDlgItemMessage(hDlg, IDC_FORCELD, BM_SETCHECK,
                            BST_UNCHECKED, 0);
                    }
                }
                break;
            }
            break;
        }
        break;
        
    case WM_NOTIFY:
        {
            if (!pRnaConn)
            {
                DEBUGMSG (ZONE_ERROR,  (TEXT("ConnMC: RnaConnInfo::RnaConnPropSheetProc1 tried to use null pRnaConn\r\n")));
                break;
            }
            
            LONG lReturn = 0;
            PSHNOTIFY * ppshn = (PSHNOTIFY *) lParam;
            switch (ppshn->hdr.code)
            {
            case PSN_SETACTIVE:
                {
                    PropSheet_SetWizButtons(::GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
                    
                    // Set the connection name.
                    ::SetWindowText(::GetDlgItem(hDlg, IDC_CONNLABEL), pRnaConn->pszName);
                }
                break;
                
            case PSN_WIZFINISH: // WIZFINISH doesn't send a KILLACTIVE so simulate here
            case PSN_KILLACTIVE:
                {
                    ::GetDlgItemText(hDlg, IDC_AREA_CODE,
                        pRnaConn->RasEntry.szAreaCode,
                        RAS_MaxAreaCode+1);
                    
                    ::GetDlgItemText(hDlg, IDC_PHONE_NUM,
                        pRnaConn->RasEntry.szLocalPhoneNumber,
                        RAS_MaxPhoneNumber+1);
                    
                    // trim white space in phone number
                    TCHAR * pGet = pRnaConn->RasEntry.szLocalPhoneNumber;
                    TCHAR * pPut = pGet;
                    while (*pPut && *pGet)
                    {
                        if (*pGet != _T(' '))
                        {
                            *pPut = *pGet;
                            pPut++;
                        }
                        pGet++;
                    }
                    
                    pRnaConn->RasEntry.dwCountryCode =
                        ::GetDlgItemInt(hDlg, IDC_CNTRYRGN, NULL, FALSE);
                    
                    // country/region code check box
                    if (BST_CHECKED == ::SendDlgItemMessage(hDlg, IDC_FORCELD,
                        BM_GETCHECK, 0, 0))
                    {
                        pRnaConn->RasEntry.dwfOptions |= (RASEO_UseCountryAndAreaCodes);
                    }
                    else
                    {
                        pRnaConn->RasEntry.dwfOptions &= ~(RASEO_UseCountryAndAreaCodes);
                    }
                    
                    if (BST_CHECKED == ::SendDlgItemMessage(hDlg, IDC_FORCELOCAL,
                        BM_GETCHECK, 0, 0))
                    {
                        pRnaConn->RasEntry.dwfOptions |= (RASEO_DialAsLocalCall);
                    }
                    else
                    {
                        pRnaConn->RasEntry.dwfOptions &= ~(RASEO_DialAsLocalCall);
                    }
                }
                if (PSN_WIZFINISH != ppshn->hdr.code)
                    break;
                
            case PSN_APPLY:
                {
                    // Validate Phone...
                    if (_T('\0') == pRnaConn->RasEntry.szLocalPhoneNumber[0])
                    {
                        ::EnableWindow(::GetParent(hDlg), FALSE);
                        ::SendMessage(::GetParent(::GetParent(hDlg)), RNM_SHOWERROR,
                            NULL, IDS_ERR_PHONEREQ);
                        ::EnableWindow(::GetParent(hDlg), TRUE);
                        ::SetFocus(::GetDlgItem(hDlg, IDC_PHONE_NUM));
                        lReturn = 1;
                        break;
                    }
                    
                    if (!isValidRasInfoString(pRnaConn->RasEntry.szLocalPhoneNumber))
                    {
                        ::EnableWindow(::GetParent(hDlg), FALSE);
                        // Need a better error message
                        ::SendMessage(::GetParent(::GetParent(hDlg)), RNM_SHOWERROR,
                            NULL, IDS_ERR_PHONEREQ);
                        ::EnableWindow(::GetParent(hDlg), TRUE);
                        ::SetFocus(::GetDlgItem(hDlg, IDS_ERR_BADNAME));
                        lReturn = 1;
                        break;
                    }
                    
                    // if they have checked the force local checkbox, make sure they have entered an area code
                    if ((RASEO_DialAsLocalCall & pRnaConn->RasEntry.dwfOptions) && 
                        (_T('\0') == pRnaConn->RasEntry.szAreaCode[0]))
                    {
                        ::EnableWindow(::GetParent(hDlg), FALSE);
                        ::SendMessage(::GetParent(::GetParent(hDlg)), RNM_SHOWERROR,
                            NULL, IDS_ERR_AREAREQ);
                        ::EnableWindow(::GetParent(hDlg), TRUE);
                        ::SetFocus(::GetDlgItem(hDlg, IDC_AREA_CODE));
                        lReturn = 1;
                        break;
                    }
                    
                    pRnaConn->writeChanges();
                }
                break;
         }
         ::SetWindowLong(hDlg, DWL_MSGRESULT, lReturn);
      }
      break;
      
      default:
          return FALSE;
   }
   
   return TRUE;
}

CFormattedEdit::CFormattedEdit()
{
	m_Format[0]			= NULL;
	m_Window			= NULL;
	m_OriginalWndProc	= NULL;
}

CFormattedEdit::~CFormattedEdit()
{
	Detach();
}

HRESULT CFormattedEdit::Attach(TCHAR *pFormat, HWND ParentWindow, DWORD ControlID)
{
	HRESULT RetVal = E_FAIL;

	// Ensure the parameters are valid
	if (!pFormat[0] || !ParentWindow || !ControlID)
	{
		ASSERT(!L"Invalid paramters to CFormattedEdit::Attach");
	}

	HWND Window = NULL;

	Window = GetDlgItem(ParentWindow, ControlID);

	if (Window)
	{
		RetVal = Attach(pFormat, Window);
	}else{
		ASSERT(!L"Unable to find child control");
	}

	return RetVal;
}

HRESULT CFormattedEdit::Attach(TCHAR *pFormat, HWND Window)
{
	HRESULT RetVal = E_FAIL;

	// Ensure the parameters are valid
	if (!pFormat[0] || !Window)
	{
		ASSERT(!L"Invalid parameters to CFormattedEdit::Attach");
	}

	// Now check that we are in a reasonable state
	if (m_Window || m_OriginalWndProc)
	{
		ASSERT(!L"CFormattedEdit::Attach object already Attached");
	}

	// Store the original window proc
	m_OriginalWndProc = (WNDPROC) GetWindowLong(Window, GWL_WNDPROC);

	if (m_OriginalWndProc)
	{
		m_Window = Window;
		SetWindowLong(m_Window, GWL_USERDATA, PtrToLong(this));
		SetWindowLong(m_Window, GWL_WNDPROC, (LONG)CFormattedEdit::SubclassWndProc);
		StringCchCopy(m_Format, MAX_FORMAT_LEN, pFormat);

		RetVal = S_OK;
	}else{
		ASSERT(!"Unable to get the original window proc");
	}

	return RetVal;
}

void CFormattedEdit::Detach()
{
	if (m_Window && m_OriginalWndProc)
	{
		SetWindowLong(m_Window, GWL_WNDPROC, (LONG)m_OriginalWndProc);
		SetWindowLong(m_Window, GWL_USERDATA, NULL);
	}

	m_Window			= NULL;
	m_OriginalWndProc	= NULL;
}

LRESULT CALLBACK CFormattedEdit::SubclassWndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	CFormattedEdit
		*pScreenWnd = NULL;

	pScreenWnd = reinterpret_cast<CFormattedEdit *>(GetWindowLong(hwnd, GWL_USERDATA));

	if (pScreenWnd && pScreenWnd->m_OriginalWndProc)
	{
		switch (Message)
		{
		case WM_CHAR:
			{
				if (!_tcschr(pScreenWnd->m_Format, (TCHAR)wParam))
				{
					return 0;
				}
			}break;
		}

		return CallWindowProc(pScreenWnd->m_OriginalWndProc, hwnd, Message, wParam, lParam);
	}else{
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
}
