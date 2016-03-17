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
// eaptlscfg.cpp : Implemention of EAP extension configuration UI
//

#include "windows.h"
#include "windowsx.h"
#include <commctrl.h>
#include "wcommctl.h"
#include <wincrypt.h>
#include <certui.h>
#include <certuip.h>
#include <eapcfg.h>
#include <winscard.h>
#include <netuip.h>
#include <raseapif.h>
#include "resource.h"
#include <eaputil.h>

class EapTlsConfig {
    public:
        EapTlsConfig() 
            : m_pConnProp(NULL),
            m_hMyStore(0),
            m_pUserCert(NULL),
            m_hDlg(NULL)
            { m_fCapi2 = LoadCAPI2();};
        DWORD InitConnectionData(BYTE *pConnectionDataIn, DWORD dwSize, DWORD dwConnFlags);
        void GetConnectionData(BYTE **ppConnectionDataOut, DWORD *pdwSize)
            {
                *ppConnectionDataOut = (BYTE *)m_pConnProp;
                *pdwSize = m_pConnProp ? m_pConnProp->dwSize : 0;
             };
        BOOL DoDialog(HINSTANCE hInst, HWND hwndParent);
        ~EapTlsConfig();
        static BOOL CALLBACK ConfigDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    private:
        EAPTLS_CONN_PROPERTIES_V1 *m_pConnProp;
        BOOL m_fCapi2;
        HCERTSTORE m_hMyStore;
        PCCERT_CONTEXT m_pUserCert;
        HWND m_hDlg;

        BOOL InitDialog(HWND hDlg);
        void FinishDialog();
        DWORD PickUserCertificate();
        void UpdateUserCert();
        
};


EapTlsConfig::~EapTlsConfig()
{
    if (m_pUserCert)
        pCertFreeCertificateContext(m_pUserCert);
    if (m_hMyStore)
        pCertCloseStore(m_hMyStore,0);
    FreeCAPI2();
    if (m_pConnProp)
        LocalFree(m_pConnProp);
        
}

DWORD
EapTlsConfig::InitConnectionData(BYTE *pConnectionDataIn, DWORD dwSizeOfConnectionDataIn, DWORD dwConnFlags)
{
    DWORD                       dwErr       = NO_ERROR;
    EAPTLS_CONN_PROPERTIES_V1*     pConnProp   = NULL;

    if (dwSizeOfConnectionDataIn < sizeof(EAPTLS_CONN_PROPERTIES_V1))
    {
        pConnProp = (EAPTLS_CONN_PROPERTIES_V1 *)LocalAlloc(LPTR, sizeof(EAPTLS_CONN_PROPERTIES_V1));

        if (NULL == pConnProp)
        {
            dwErr = GetLastError();
            DEBUGMSG(ZONE_ERROR, (TEXT("LocalAlloc failed and returned %d"), dwErr));
            goto LDone;
        }

        pConnProp->dwVersion = EAPTLS_CONN_PROPS_V1;
        pConnProp->dwSize = sizeof(EAPTLS_CONN_PROPERTIES_V1);
        pConnProp->fFlags = (dwConnFlags | EAPTLS_CONN_FLAG_NO_VALIDATE_NAME);
    }
    else
    {
        DEBUGCHK(NULL != pConnectionDataIn);

        pConnProp = (EAPTLS_CONN_PROPERTIES_V1 *)LocalAlloc(LPTR, dwSizeOfConnectionDataIn);

        if (NULL == pConnProp)
        {
            dwErr = GetLastError();
            DEBUGMSG(ZONE_ERROR, (TEXT("LocalAlloc failed and returned %d"), dwErr));
            goto LDone;
        }

        CopyMemory(pConnProp, pConnectionDataIn, dwSizeOfConnectionDataIn);

        // If the user has mucked with the phonebook, we mustn't be affected.
        // The size must be correct.
        pConnProp->dwSize = dwSizeOfConnectionDataIn;
        // The Unicode string must be NULL terminated.
        ((BYTE*)pConnProp)[dwSizeOfConnectionDataIn - 2] = 0;
        ((BYTE*)pConnProp)[dwSizeOfConnectionDataIn - 1] = 0;
    }

    m_pConnProp = pConnProp;
    pConnProp = NULL;

LDone:

    LocalFree(pConnProp);
    return(dwErr);
}

BOOL
EapTlsConfig::DoDialog(HINSTANCE hInst, HWND hwndParent)
{
    HCURSOR hCur;
    BOOL ret;
    BOOL fPortrait = (GetSystemMetrics(SM_CXSCREEN) < 480)? TRUE:FALSE;
    hCur = SetCursor(NULL);
    ret = DialogBoxParam (hInst, MAKEINTRESOURCE(fPortrait?IDD_EAPTLSCONFIG_N:IDD_EAPTLSCONFIG_W),
    				   hwndParent, EapTlsConfig::ConfigDialogProc, (LONG)this);
    SetCursor(hCur);
    return ret;
}

BOOL
EapTlsConfig::InitDialog(HWND hDlg)
{
    int ValidateServerChecked;
    m_hDlg = hDlg;
    // init the validate server checkbox
    ValidateServerChecked =  (m_pConnProp->fFlags & EAPTLS_CONN_FLAG_NO_VALIDATE_CERT) ?
        BST_UNCHECKED :
        BST_CHECKED;
    Button_SetCheck(GetDlgItem(hDlg, IDCB_VALIDATESERVER),ValidateServerChecked); 
    // Open the My Certificate store

    if (!m_fCapi2)
        return FALSE;

    m_hMyStore = pCertOpenStore(CERT_STORE_PROV_SYSTEM_W, CRYPT_ASN_ENCODING, 
                            0, CERT_SYSTEM_STORE_CURRENT_USER, L"MY");
    if (!m_hMyStore)
        return FALSE;

    if (m_pConnProp->fFlags & EAPTLS_CONN_FLAG_NO_CLIENT_CERT)
    {
        // disable all the client certificate fields
        EnableWindow(GetDlgItem(m_hDlg, IDB_PICKCERT), FALSE);
    }
    else   
    // is a client cert set ?
    if (m_pConnProp->UserHash.cbHash)
    {
        CRYPT_HASH_BLOB HashBlob;
        HashBlob.cbData = m_pConnProp->UserHash.cbHash;
        HashBlob.pbData = m_pConnProp->UserHash.pbHash;

        // find the cert in the store
        m_pUserCert = pCertFindCertificateInStore(m_hMyStore, X509_ASN_ENCODING,
                            0, CERT_FIND_HASH, &HashBlob, NULL);
        if (m_pUserCert)
            UpdateUserCert();
    }
    return TRUE;
}

void
EapTlsConfig::FinishDialog()
{
    // right now we can modify the EAPTLS_CONN_PROPERIES struct in place,
    // because we don't change its size. 
    // 
    // not checking server name
    BOOL fValidateCert;
    fValidateCert = Button_GetCheck(GetDlgItem(m_hDlg, IDCB_VALIDATESERVER)) == BST_CHECKED;
    if (fValidateCert)
        m_pConnProp->fFlags &= ~EAPTLS_CONN_FLAG_NO_VALIDATE_CERT;
    else
        m_pConnProp->fFlags |= EAPTLS_CONN_FLAG_NO_VALIDATE_CERT;
    // get the user cert hash id
    if (m_pUserCert)
    {
        DWORD dwHashSize = MAX_HASH_SIZE;
        if (pCertGetCertificateContextProperty(m_pUserCert,
            CERT_HASH_PROP_ID, &m_pConnProp->UserHash.pbHash, &dwHashSize))
        {
            m_pConnProp->UserHash.cbHash = dwHashSize;
        }
    }
}

DWORD EapTlsConfig::PickUserCertificate()
{
    DWORD fNewCert = FALSE;
    NETUI_PICKCERT pickCert = {0};
    PCCERT_CONTEXT  pCerts[20];
    PCCERT_CONTEXT pCertContext;
    DWORD dwHashSize;
    pickCert.nCerts = 0;
    pickCert.iSelected = 0;
    pickCert.ppCerts = pCerts;
    if (!m_hMyStore)
        return FALSE;

    // Find client certificate. 
    pCertContext = NULL;

    while (pickCert.nCerts < sizeof(pCerts)/sizeof(pCerts[0])
        && (pCertContext = pCertEnumCertificatesInStore(m_hMyStore,  pCertContext)))
    {
        BYTE hash[MAX_HASH_SIZE];
        dwHashSize = MAX_HASH_SIZE;
    	// check for client certificate usage 
    		//if (!FCheckUsage( pCertContext, NULL, FALSE))
    		//	continue;

    		// this is a possible candidate
    		pCerts[pickCert.nCerts++] = pCertDuplicateCertificateContext(pCertContext);

    	//compare the hash
    	pCertGetCertificateContextProperty(pCertContext, CERT_HASH_PROP_ID, hash, &dwHashSize);
    	if (dwHashSize && dwHashSize == m_pConnProp->UserHash.cbHash 
            && dwHashSize <= MAX_HASH_SIZE
    	    && memcmp(hash, m_pConnProp->UserHash.pbHash, dwHashSize) == 0)
    		    pickCert.iSelected = pickCert.nCerts - 1;
    }
    if (pCertContext)
    {
    	// more certs than we bargained for
    	pCertFreeCertificateContext(pCertContext);
    	pCertContext = NULL;
    }
    
    	int i;
    	if (PickCertificate(m_hDlg, &pickCert) && pickCert.iSelected < pickCert.nCerts)
    	{
    		EAPTLS_HASH newhash;
    		newhash.cbHash = MAX_HASH_SIZE;
    		pCertContext = pickCert.ppCerts[pickCert.iSelected];
    		// save the hash away
    		if (pCertGetCertificateContextProperty(pCertContext, CERT_HASH_PROP_ID, newhash.pbHash, &newhash.cbHash))
    		{
                    if (memcmp(newhash.pbHash, m_pConnProp->UserHash.pbHash, newhash.cbHash))
                    {
                        m_pConnProp->UserHash = newhash;
                        if (m_pUserCert)
                            pCertFreeCertificateContext(m_pUserCert);
                        m_pUserCert = pCertDuplicateCertificateContext(pCertContext);
                        UpdateUserCert();
                        fNewCert = TRUE;
                    }
    		}
    		
    	}
    	// free up the cert references
    	for (i = 0; i < pickCert.nCerts ; i++)
    		pCertFreeCertificateContext(pickCert.ppCerts[i]);

    	return fNewCert;
}

void
EapTlsConfig::UpdateUserCert(void)
{
        WCHAR szName[MAX_PATH];
        DWORD cchName;
        cchName =pCertGetNameStringW(m_pUserCert, CERT_NAME_UPN_TYPE, 0, NULL, szName, MAX_PATH   );
        if (cchName < 2)
            cchName = pCertGetNameStringW(m_pUserCert, CERT_NAME_FRIENDLY_DISPLAY_TYPE, 0, NULL, szName, MAX_PATH   );
        SetDlgItemTextW(m_hDlg,  IDC_ISSUEDTO_VALUE, szName);
        cchName=pCertGetNameStringW(m_pUserCert, CERT_NAME_FRIENDLY_DISPLAY_TYPE, CERT_NAME_ISSUER_FLAG, NULL, szName, MAX_PATH   );
        if (cchName >= 2)
        {
            SetDlgItemTextW(m_hDlg, IDC_ISSUEDBY_VALUE, szName);
        }
}        

BOOL
CALLBACK
EapTlsConfig::ConfigDialogProc(
	HWND hDlg, 
	UINT uMsg, 
	WPARAM wParam, 
	LPARAM lParam)
{
	BOOL fRet = TRUE;
	EapTlsConfig *pConfig;
	PCCERT_CONTEXT pClientCert = NULL;
	//HCERTSTORE hStore= (HCERTSTORE) lParam;
	LONG wCmdId;
	LONG wCmdCbn;

	HWND hCtlList = GetDlgItem(hDlg, IDC_CERTLISTVIEW);
	switch (uMsg) {
    case WM_NOTIFY :
        break;
    case WM_INITDIALOG :
        {
            pConfig = (EapTlsConfig *)  lParam;
            if (!pConfig)
                return FALSE;
            SetWindowLong (hDlg, DWL_USER, (LONG)pConfig);
            fRet = pConfig->InitDialog(hDlg);
        }
        break;

    case WM_COMMAND :
        wCmdId  = LOWORD(wParam);
        wCmdCbn = HIWORD(wParam);

        switch ( wCmdId ) {
        
        case IDCANCEL:
            EndDialog(hDlg,FALSE);
            break;

        case IDOK:
        { 
              pConfig = (EapTlsConfig *)GetWindowLong(hDlg, DWL_USER);
              pConfig->FinishDialog();
        	EndDialog(hDlg, TRUE);

        break;
        }
        case IDB_PICKCERT:
        {
        	// pick a certificate button
              pConfig = (EapTlsConfig *)GetWindowLong(hDlg, DWL_USER);
              pConfig->PickUserCertificate();
              break;
        }
		default:
			fRet = FALSE;
			break;
		}
	default:
		fRet = FALSE;
		break;
	}
	return fRet;
}

DWORD
RegGetDwordValue(
	IN  HKEY		hKey,
	IN  PWSTR		wszSubKeyName,
	IN  PWSTR		wszValueName,
	OUT PDWORD      pdwValue)
{
	HKEY                hk;
	DWORD               dwSize;
	DWORD               dwType;
	DWORD               dwResult;
	DWORD               dwValue;

	dwResult = RegOpenKeyEx(hKey, wszSubKeyName, 0, KEY_ALL_ACCESS, &hk);
	if (ERROR_SUCCESS == dwResult)
	{
		dwSize = sizeof(DWORD);
		dwResult = RegQueryValueEx(hk, wszValueName, NULL, &dwType, (BYTE *)&dwValue, &dwSize);
		if (ERROR_SUCCESS == dwResult)
		{
			if (REG_DWORD == dwType)
				*pdwValue = dwValue;
			else
				dwResult = ERROR_BADDB;
		}
		RegCloseKey(hk);
	}

	return dwResult;
}

/*
  RasEapInvokeConfigUI (PUBLIC)

Returns:

Notes:
    Called to get the EAP-TLS properties for a connection. Currently called by NetUI

*/

DWORD 
EapTlsInvokeConfigUI(
    IN  DWORD       dwEapTypeId,
    IN  HWND        hwndParent,
    IN  DWORD       dwFlags,
    IN  BYTE*       pConnectionDataIn,
    IN  DWORD       dwSizeOfConnectionDataIn,
    OUT BYTE**      ppConnectionDataOut,
    OUT DWORD*      pdwSizeOfConnectionDataOut
)
{
    DWORD               dwErr               = NO_ERROR;
    EapTlsConfig       *pConfig;
    DWORD               dwConnFlags;
    WCHAR               wszKeyName[MAX_PATH];
    DWORD               dwValidateServerCert;

    DEBUGCHK(NULL != ppConnectionDataOut);
    DEBUGCHK(NULL != pdwSizeOfConnectionDataOut);

    *ppConnectionDataOut = NULL;
    *pdwSizeOfConnectionDataOut = 0;

    pConfig = new EapTlsConfig();
    if (!pConfig)
    {
        dwErr = ERROR_OUTOFMEMORY;
        goto LDone;
        
    }
    dwConnFlags = (dwEapTypeId == PPP_EAP_PEAP) ? EAPTLS_CONN_FLAG_NO_CLIENT_CERT : 0;

    //
    // If HKEY_LOCAL_MACHINE\Comm\EAP\Extension\<dwEapTypeId>\ValidateServerCert == dword:0, then
    // by default server cert validation is disabled.
    //
    swprintf(wszKeyName, L"Comm\\EAP\\Extension\\%u", dwEapTypeId);
    dwValidateServerCert = 1;
    (void)RegGetDwordValue(HKEY_LOCAL_MACHINE, wszKeyName, L"ValidateServerCert", &dwValidateServerCert);
    if (dwValidateServerCert == 0)
        dwConnFlags |= EAPTLS_CONN_FLAG_NO_VALIDATE_CERT;

    dwErr = pConfig->InitConnectionData(pConnectionDataIn, dwSizeOfConnectionDataIn, dwConnFlags);
    
    if (NO_ERROR == dwErr)
    {
        if(pConfig->DoDialog(v_hInst, hwndParent))
        {
            PBYTE pConnectionDataOut = NULL;
            DWORD dwSizeOfConnectionDataOut = 0;
            pConfig->GetConnectionData(&pConnectionDataOut, &dwSizeOfConnectionDataOut);
            if (pConnectionDataOut && dwSizeOfConnectionDataOut)
            {
                *ppConnectionDataOut = (PBYTE)LocalAlloc(0, dwSizeOfConnectionDataOut);
                if (*ppConnectionDataOut)
                {
                    CopyMemory(*ppConnectionDataOut, pConnectionDataOut, dwSizeOfConnectionDataOut);
                    *pdwSizeOfConnectionDataOut = dwSizeOfConnectionDataOut;
                }
                else
                    dwErr = GetLastError();
            }
        }
        else
            dwErr = ERROR_CANCELLED;
    }

LDone:
    if (pConfig)
        delete pConfig;
    return dwErr;
}

void
PeapInitializeDefaultConnectionProperties(
	OUT PEAP_CONN_PROP *pConnProp)
//
//  Called to initialize PEAP connection properties data structure
//  with default settings.
//
{
	DWORD                       dwValidateServerCert;
	PEAP_ENTRY_CONN_PROPERTIES  EntryProp;

	pConnProp->dwVersion = 1;
    pConnProp->dwSize = sizeof(PEAP_CONN_PROP) + sizeof(PEAP_ENTRY_CONN_PROPERTIES);
    pConnProp->EapTlsConnProp.dwVersion = EAPTLS_CONN_PROPS_V1;
    pConnProp->EapTlsConnProp.dwSize = sizeof(EAPTLS_CONN_PROPERTIES_V1);
    pConnProp->EapTlsConnProp.fFlags = EAPTLS_CONN_FLAG_NO_CLIENT_CERT | EAPTLS_CONN_FLAG_NO_VALIDATE_NAME;

	//
	// If HKEY_LOCAL_MACHINE\Comm\EAP\Extension\25\ValidateServerCert == dword:0, then
	// by default server cert validation is disabled.
	//

	dwValidateServerCert = 1;
	EntryProp.dwEapTypeId = PPP_EAP_MSCHAPv2;
	(void)RegGetDwordValue(HKEY_LOCAL_MACHINE, EAPPEAP_KEY_25, EAPTLS_VAL_CHECK_SERVER_CERT, &dwValidateServerCert);
	(void)RegGetDwordValue(HKEY_LOCAL_MACHINE, EAPPEAP_KEY_25, EAPTLS_VAL_TUNNELED_EXTENSION_TYPE, &EntryProp.dwEapTypeId);

	if (dwValidateServerCert == 0)
		pConnProp->EapTlsConnProp.fFlags |= EAPTLS_CONN_FLAG_NO_VALIDATE_CERT;

	pConnProp->dwNumPeapTypes = 1;

    //
    // Setup the first peap entry conn prop
    //
    EntryProp.dwVersion = 1;
    EntryProp.dwSize = sizeof(PEAP_ENTRY_CONN_PROPERTIES);
	memcpy((PBYTE)&pConnProp->EapTlsConnProp + pConnProp->EapTlsConnProp.dwSize, (PBYTE)&EntryProp, sizeof(EntryProp));
}

//
// Since there are no separate settings for PEAP right now, we just use the EAPTLS configuration UI
// and pass in a flag that tells it to disable some fields (such as the client certificate picker)
// Note that the EAPTLS_CONN_PROPERTIES struct is embedded inside the PEAP_CONN_PROP struct
// 
DWORD 
EapPeapInvokeConfigUI(
    IN  HWND        hwndParent,
    IN  DWORD       dwFlags,
    IN  BYTE*       pConnectionDataIn,
    IN  DWORD       dwSizeOfConnectionDataIn,
    OUT BYTE**      ppConnectionDataOut,
    OUT DWORD*      pdwSizeOfConnectionDataOut)
{
	DWORD dwResult = NO_ERROR;
	PEAP_CONN_PROP *pConnProp = (PEAP_CONN_PROP *)pConnectionDataIn;
	PEAP_CONN_PROP *pEapConnPropOut;
	EAPTLS_CONN_PROPERTIES_V1 *pEapTlsConnProp = NULL, *pEapTlsConnPropOut = NULL;
	PEAP_ENTRY_CONN_PROPERTIES EntryProp;
	PBYTE pEntryProp = (PBYTE)&EntryProp;
	DWORD dwEntryPropSize = sizeof(EntryProp);
	DWORD dwEapTlsConnectionDataOut;
	DWORD dwSizeEapConnPropOut;
	BOOL fConnPropAlloced = FALSE;
	
    DEBUGCHK(NULL != ppConnectionDataOut);
    DEBUGCHK(NULL != pdwSizeOfConnectionDataOut);

    *ppConnectionDataOut = NULL;
    *pdwSizeOfConnectionDataOut = 0;
    if (pConnProp 
        && pConnProp->dwSize >=sizeof(PEAP_CONN_PROP)
        && dwSizeOfConnectionDataIn >= pConnProp->dwSize
        )
    {
        if (pConnProp->dwVersion != 1)
        {
            // unsupported version
            return ERROR_NOT_SUPPORTED;
        }
    }
    else
    {
		// No PEAP connection data provided, create default data
		dwSizeOfConnectionDataIn = sizeof(PEAP_CONN_PROP) + sizeof(PEAP_ENTRY_CONN_PROPERTIES)+ sizeof(WCHAR);
        pConnProp = (PPEAP_CONN_PROP)LocalAlloc(LPTR, dwSizeOfConnectionDataIn);
		if (NULL == pConnProp)
		{
			return ERROR_OUTOFMEMORY;
		}
        // remember to free the allocated pPeapConnProp
        fConnPropAlloced = TRUE;
		PeapInitializeDefaultConnectionProperties(pConnProp);
    }

    pEapTlsConnProp = &pConnProp->EapTlsConnProp;
    pEntryProp = (PBYTE) &pConnProp->EapTlsConnProp + pEapTlsConnProp->dwSize;
    dwEntryPropSize = pConnProp->dwSize - pEapTlsConnProp->dwSize - offsetof(PEAP_CONN_PROP, EapTlsConnProp);
    
    dwResult = EapTlsInvokeConfigUI(
                        PPP_EAP_PEAP, 
                        hwndParent,
                        dwFlags,
                        (PBYTE)pEapTlsConnProp,
                        pEapTlsConnProp->dwSize,
                        (PBYTE *)&pEapTlsConnPropOut,
                        &dwEapTlsConnectionDataOut);
    if (dwResult != NO_ERROR)
    {
        goto cleanup;
    }

    if (dwEapTlsConnectionDataOut < sizeof(EAPTLS_CONN_PROPERTIES_V1)
    || dwEapTlsConnectionDataOut != pEapTlsConnPropOut->dwSize
    || pEapTlsConnPropOut->dwSize > 0xFFFFFF
    || dwEntryPropSize > 0xFFFFFF)
    {
        dwResult =  ERROR_INTERNAL_ERROR;
        goto cleanup;
    }

    // construct a new PEAP_CONN_PROP structure
    dwSizeEapConnPropOut = sizeof(PEAP_CONN_PROP) 
                - sizeof(EAPTLS_CONN_PROPERTIES_V1)+ pEapTlsConnPropOut->dwSize + dwEntryPropSize;
    pEapConnPropOut = (PEAP_CONN_PROP *)LocalAlloc(0, dwSizeEapConnPropOut);
            
    pEapConnPropOut->dwVersion = pConnProp->dwVersion;
    pEapConnPropOut->dwSize = dwSizeEapConnPropOut;
    pEapConnPropOut->dwNumPeapTypes = pConnProp->dwNumPeapTypes;

    // copy in the EAPTLS structure
    memcpy(&pEapConnPropOut->EapTlsConnProp, pEapTlsConnPropOut, dwEapTlsConnectionDataOut);
    
    //append the PEAP_ENTRY_CONN_PROPERTIES
    memcpy( (PBYTE) &pEapConnPropOut->EapTlsConnProp + dwEapTlsConnectionDataOut,
                   pEntryProp,
                   dwEntryPropSize);
    
    *ppConnectionDataOut = (PBYTE)pEapConnPropOut;
    *pdwSizeOfConnectionDataOut = dwSizeEapConnPropOut;
cleanup:
   LocalFree((PBYTE)pEapTlsConnPropOut);
   if (fConnPropAlloced)
		LocalFree(pConnProp);

    
    return dwResult;
}

extern "C"
DWORD 
RasEapInvokeConfigUI(
    IN  DWORD       dwEapTypeId,
    IN  HWND        hwndParent,
    IN  DWORD       dwFlags,
    IN  BYTE*       pConnectionDataIn,
    IN  DWORD       dwSizeOfConnectionDataIn,
    OUT BYTE**      ppConnectionDataOut,
    OUT DWORD*      pdwSizeOfConnectionDataOut
)
{
    DWORD  dwErr;

	switch(dwEapTypeId)
	{
	case PPP_EAP_TLS:
        dwErr = EapTlsInvokeConfigUI(
            dwEapTypeId,
            hwndParent,
            dwFlags,
            pConnectionDataIn,
            dwSizeOfConnectionDataIn,
            ppConnectionDataOut,
            pdwSizeOfConnectionDataOut
        );
		break;

	case PPP_EAP_PEAP:
        dwErr = EapPeapInvokeConfigUI(
            hwndParent,
            dwFlags,
            pConnectionDataIn,
            dwSizeOfConnectionDataIn,
            ppConnectionDataOut,
            pdwSizeOfConnectionDataOut
        );
		break;

	default:
        DEBUGMSG(ZONE_ERROR, (TEXT("EAP Type %d is not supported"), dwEapTypeId));
        dwErr = ERROR_NOT_SUPPORTED;
		break;
    }

    return dwErr;
}


/*

Returns:
    Error codes only from winerror.h, raserror.h or mprerror.h

Notes:
    RasEapInvokeInteractiveUI entry point called by the EAP-PPP engine by name.

*/

DWORD
RasEapInvokeInteractiveUI(
    IN  DWORD           dwEapTypeId,
    IN  HWND            hWndParent,
    IN  BYTE*           pUIContextData,
    IN  DWORD           dwSizeofUIContextData,
    OUT BYTE**          ppDataFromInteractiveUI,
    OUT DWORD*          pdwSizeOfDataFromInteractiveUI
)
{
    DWORD                       dwRetCode = ERROR_NOT_SUPPORTED;

    if ( PPP_EAP_TLS == dwEapTypeId )
    {
            EAPEXTUI_INPUT *pExtUIInput;
            *ppDataFromInteractiveUI = NULL;
            *pdwSizeOfDataFromInteractiveUI = 0;
            
            if ((pExtUIInput = (EAPEXTUI_INPUT *)pUIContextData)
                && pExtUIInput->dwVersion == 1
                && pExtUIInput->dwSize >= sizeof(EAPEXTUI_INPUT)
                )
            {
                UINT uType = MB_OK;
                int ret;
                WCHAR szMsg[100];
                WCHAR szTitle[80];
                DWORD dwId;
                dwRetCode = NO_ERROR;
                switch(pExtUIInput->dwCode)
                {
                 case EAPEXTUI_CODE_CLIENTAUTH_ERROR:                        
                        break;
                 case EAPEXTUI_CODE_SERVERAUTH_ERROR:
                    if (pExtUIInput->fFlags & EAPEXTUI_FLAG_RESPONSE)
                    {
                        // response expected
                        uType = MB_YESNO;
                    }
                    switch (pExtUIInput->dwStatus)
                    {
                     case SEC_E_UNTRUSTED_ROOT:
                        dwId = IDS_UNKNOWN_ROOT_ERROR;
                        break;
                     case SEC_E_CERT_UNKNOWN:
                        dwId = IDS_UNKNOWN_CERT_ERROR;                    
                        break;
                     case SEC_E_CERT_EXPIRED:
                        dwId = IDS_CERT_EXPIRED_ERROR;
                        break;
                     default:
                        dwId = IDS_GENERIC_CERT_ERROR;
                        break;
                    }
                    szMsg[0] = L'\0';
                    szTitle[0] = L'\0';
                    LoadStringW(v_hInst, dwId ,szMsg, ARRAYSIZE(szMsg));
                    LoadStringW(v_hInst, IDS_SERVER_VALIDATION_ERROR,szTitle, ARRAYSIZE(szTitle));

                    ret = MessageBox(hWndParent, szMsg, szTitle, uType);

                    if (pExtUIInput->fFlags & EAPEXTUI_FLAG_RESPONSE)
                    {
                        // response expected
                        EAPEXTUI_OUTPUT *pExtUIOutput = (EAPEXTUI_OUTPUT *)LocalAlloc(LPTR, sizeof(EAPEXTUI_OUTPUT));
                        if (pExtUIOutput)
                        {
                            pExtUIOutput->dwVersion =1;
                            pExtUIOutput->dwSize = sizeof(EAPEXTUI_OUTPUT);
                            pExtUIOutput->dwValue = ret;
                            *ppDataFromInteractiveUI = (PBYTE)pExtUIOutput;
                            *pdwSizeOfDataFromInteractiveUI = pExtUIOutput->dwSize;
                        }
                    }
                    
                    break;
                  default:
                    dwRetCode = ERROR_NOT_SUPPORTED;
                           
                }
            }
    }

	return dwRetCode;
}
/*
RasEapFreeMemory (PUBLIC)
Returns:

Notes:
    Called to free memory.

*/

extern "C"
DWORD 
RasEapFreeMemory(
    IN  BYTE*   pMemory
)
{
    LocalFree(pMemory);
    return NO_ERROR;
}

