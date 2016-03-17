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
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

Abstract: Main entry point to the CPL applets
**/
//
// sipcpl.cpp
//


#include "cplpch.h"

// Need this struct defn for sip_i.h
typedef struct tagIMINFO
{
    DWORD	cbSize;
    HANDLE	hImageNarrow;
    HANDLE	hImageWide;
    int		iNarrow;
    int		iWide;
    DWORD	fdwFlags;
    RECT	rcSipRect;
} IMINFO;

#include "sip_i.h"
// to get IID_IInputMethod
#include "sip_i.c"

//
// Magic number to send to SIP to get past no-app-change restriction.
//
#define MAGIC_BYPASS        0x10921B39

// Prototypes
void SIP_SelectCurrentIm(HWND hwndCB, HWND hwndIcon);
void SIP_DoOptions(HWND hwndParent, BOOL fCOMInited);

// Global data
HICON g_hiconDefault;

// This struct is attached as CB data to each IM combo box item
#pragma warning(disable:4200)
typedef struct {
    CLSID	m_clsid;
    HICON 	m_hiconWide;
    HICON	m_hiconNarrow;
    TCHAR	m_szName[];
}
IMENTRY, *PIMENTRY;

struct SipDlgState_t {
    CFileCombo* pCombo;
    BOOL        fCOMInited;

    SipDlgState_t() { 
        pCombo = NULL;
        fCOMInited = FALSE;
    }

    ~SipDlgState_t() {
        if(pCombo)
            delete pCombo;
    }
};


// Class to travel the registry & enumerate the IMs
class CIMEnum
{
private:
	CReg m_regCLSID;

public:
	CIMEnum() {
	    // Open the CLSID key.
		m_regCLSID.Open(HKEY_CLASSES_ROOT, RK_CLSID);
	} 
	~CIMEnum() {}

	// Enumerate all CLSIDs that are SIPInputMethods 
	PIMENTRY GetNextIM(void)
	{
		#define BUFSIZE 260
    	TCHAR szTemp[BUFSIZE];

	    // Enumerate next CLSID, looking for input methods.
    	while(m_regCLSID.EnumKey(szTemp, BUFSIZE))
    	{
    	    // Check if this CLSID has an IsSIPInputMethod key with value "1".
        	CReg regIM(m_regCLSID, szTemp);
        	CReg regIsSIP(regIM, RK_ISSIPINPUTMETHOD);
	        LPCTSTR pszIsIM = regIsSIP.ValueSZ(NULL);
    	    if(pszIsIM && pszIsIM[0]=='1')
        	{
            	// Get the CLSID (from the CLSID string in szTemp), 
	            // friendly name (default value of the szTemp key)
    	        // and the default icon for the IM (DefaultIcon subkey)
    	        LPCTSTR  pszIMName;
    	        CLSID    clsid;
    	        
    	        if( (pszIMName = regIM.ValueSZ(NULL)) && 
    	        	SUCCEEDED(CLSIDFromString(szTemp, &clsid)) )
        	    {
    	        	PIMENTRY pIMEntry = (PIMENTRY)LocalAlloc(LPTR, 
    	        		sizeof(IMENTRY)+sizeof(TCHAR)*(1+lstrlen(pszIMName)));

					if(pIMEntry)
					{
						lstrcpy(pIMEntry->m_szName, pszIMName);
						pIMEntry->m_clsid = clsid;
						
	            		// Get Icon if we have one
	            		CReg regIcon(regIM, RK_DEFAULTICON);
						if(regIcon.ValueSZ(NULL, szTemp, BUFSIZE))
						{
		    	        	int iIndex = 0;
    		    	    	LPTSTR pszIndex = _tcschr(szTemp, ',');
        		    		if(pszIndex)
	        	    		{
    	        				*pszIndex = 0;  // Null terminate the name portion of the string
        	    				iIndex=_ttol((pszIndex+1)); // get the icon index just past the ','
            				}
							ExtractIconEx(szTemp, iIndex, &pIMEntry->m_hiconWide, &pIMEntry->m_hiconNarrow, 1);
						}
						return pIMEntry;
					}
					else { ASSERT(FALSE); } // OOM
    	        }
    	        else { ASSERT(FALSE); } // shouldn't happen unless registry data incorrect
        	}
        	// not an IM key, try next one
	    }
    	return NULL;
	}
};

//
// Get the IMENTRY data associated with the currently selected combo-box entry
// 
PIMENTRY GetCurSelIM(HWND hDlg)
{
	HWND hwndCB = DI(IDC_COMBOIM);
	
	int i = ComboBox_GetCurSel(hwndCB);
	if(i == CB_ERR)
		return NULL;
		
	PIMENTRY pIMEntry = (PIMENTRY)ComboBox_GetItemData(hwndCB, i);
	if((int)pIMEntry == CB_ERR)
		return NULL;
	return pIMEntry;
}

//
// SIP dialog box proc
// 
extern "C" BOOL CALLBACK SipDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{

    SipDlgState_t  *pSavedState = (SipDlgState_t *)GetWindowLong(hDlg,DWL_USER);
	HRESULT hr;

    if(msg != WM_INITDIALOG && pSavedState == NULL)
    {
        return (FALSE);
	} 
	
    switch( msg )
    {
      case WM_INITDIALOG:
      {

        pSavedState = new SipDlgState_t;
        SetWindowLong(hDlg, DWL_USER, (LONG)pSavedState);

        if(pSavedState)  {
            // Initialize COM
			hr = CoInitializeEx(NULL,COINIT_MULTITHREADED);
            if(SUCCEEDED(hr)){		
                pSavedState->fCOMInited = TRUE;
			}
                
          	// load a default icon in case an IM doesn't have one
          	g_hiconDefault = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_SIP));
    		// Create an object that manages the combo-box
    		pSavedState->pCombo = new CFileCombo(DI(IDC_COMBOIM));
          	// Create an object to enumerate the IMs
    		CIMEnum* pIMEnum = new CIMEnum();
    		// Populate the combo
    		PIMENTRY pEntry;
			//Prefix
    		while(pIMEnum && (pEntry = pIMEnum->GetNextIM()))
    		{
//    			pSavedState->pCombo->AddItem(pEntry->m_szName, (LPCTSTR)pEntry);
				int iIndex = ComboBox_AddString(DI(IDC_COMBOIM), pEntry->m_szName);
				ComboBox_SetItemData(DI(IDC_COMBOIM), iIndex, (VOID*)pEntry);
    		}
    		// done with enumerator
    		delete pIMEnum;
          	// Initialize checkbox
          	CReg reg(HKEY_CURRENT_USER, RK_CONTROLPANEL_SIP);
          	Button_SetCheck(DI(IDC_CHECKALLOWCHANGE), reg.ValueDW(RV_ALLOWCHANGE));
    		// Select the current IM		   
            SIP_SelectCurrentIm(DI(IDC_COMBOIM), DI(IDC_ICONIM));
        }
		return 1; // ask GWE to set focus
      }

      case WM_DESTROY:
        // release COM
        if(pSavedState->fCOMInited)
            CoUninitialize();
        SetWindowLong(hDlg, DWL_USER, 0L);
      	delete pSavedState;
      	break;

	  case WM_COMMAND:
	  	switch (LOWORD (wParam))
		{
          case IDOK:
          {
        	// Save the AllowChange setting
			CReg reg(HKEY_CURRENT_USER, RK_CONTROLPANEL_SIP);
			reg.SetDW(RV_ALLOWCHANGE, Button_GetCheck(DI(IDC_CHECKALLOWCHANGE)));
			// Set the current IM to the one selected in the combo box.
			PIMENTRY pIMEntry = GetCurSelIM(hDlg);
			if( !pIMEntry || !SipSetCurrentIM(&pIMEntry->m_clsid) ) {
				ASSERT(FALSE);
			}
            return TRUE;
          }

          case IDC_BUTTONOPTIONS:
    		SIP_DoOptions(hDlg, pSavedState->fCOMInited); 
            return TRUE;

          case IDC_COMBOIM:
            if(HIWORD(wParam)==CBN_SELCHANGE)
            {
				PIMENTRY pIMEntry = GetCurSelIM(hDlg);

				if(pIMEntry) {
					SendMessage(DI(IDC_ICONIM), STM_SETIMAGE, IMAGE_ICON, 
						(LPARAM)(pIMEntry->m_hiconWide ? pIMEntry->m_hiconWide : g_hiconDefault));
				}
            }
            return TRUE;
        }
        break;

    case WM_SETTINGCHANGE:
        if(SPI_SETCURRENTIM== wParam) {
            SIP_SelectCurrentIm(DI(IDC_COMBOIM), DI(IDC_ICONIM));
        }
        break;
    } // switch( message )
	return FALSE;
}

//
// Select the current im in the combo box.
//
void SIP_SelectCurrentIm(HWND hwndCB, HWND hwndIcon)
{
    CLSID clsidCur;
    if (SipGetCurrentIM(&clsidCur))
    {
    	for(int i=0; ;i++)
		{
			PIMENTRY pIMEntry = (PIMENTRY)ComboBox_GetItemData(hwndCB, i);
			if((int)pIMEntry == CB_ERR)
				break;
			if(pIMEntry->m_clsid == clsidCur)
			{
				ComboBox_SetCurSel(hwndCB, i);
				SendMessage(hwndIcon, STM_SETIMAGE, IMAGE_ICON, 
					(LPARAM)(pIMEntry->m_hiconWide ? pIMEntry->m_hiconWide : g_hiconDefault));
			}
		}
	}
}


//
// Bring up the custom options for an IM.
//
void SIP_DoOptions(HWND hDlg, BOOL fCOMInited)
{
    HRESULT hr;
    IInputMethod *pIm = NULL;
	PIMENTRY pIMEntry = GetCurSelIM(hDlg);

	if(!pIMEntry || !fCOMInited )
		goto done;

    // Load an instance of the IM.
    if(FAILED(hr = CoCreateInstance(pIMEntry->m_clsid, NULL, CLSCTX_ALL,
                			IID_IInputMethod, (void **)&pIm))) {
        MessageBox(hDlg, CELOADSZ(IDS_CANT_LOAD_IM), CELOADSZ(IDS_SIP_TABTITLE), MB_OK|MB_ICONEXCLAMATION);
        goto done;           
    }

    // Ask the IM to bring up its options.
    if(FAILED(hr = pIm->UserOptionsDlg(hDlg))) {
        MessageBox(hDlg, CELOADSZ(IDS_NO_IM_OPTIONS), CELOADSZ(IDS_SIP_TABTITLE), MB_OK);
    }

    // Cleanup and return.
    pIm->Release();

done:
    return;
}

