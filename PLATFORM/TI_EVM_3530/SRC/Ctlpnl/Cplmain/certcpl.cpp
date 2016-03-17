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

Abstract: Code specific to the Certificate CPL
 **/

#include <cplpch.h>
#include <wincrypt.h>
#include <commctrl.h>
#include <commdlg.h>
#include <certui.h>
#include <winscard.h>
#include "base64.h"

extern DWORD Base64DecodeExA(
		IN CHAR const *pchIn,
		IN DWORD cchIn,
		OUT BYTE *pbOut,
		OUT DWORD *pcbOut,
		OUT DWORD *pchConsumed);

//#include "creg.hxx"

typedef 
HCERTSTORE (WINAPI *PFNCERTOPENSTORE)(
		IN LPCSTR lpszStoreProvider,
		IN DWORD dwEncodingType,
		IN HCRYPTPROV hCryptProv,
		IN DWORD dwFlags,
		IN const void *pvPara
		);
typedef
BOOL (WINAPI *PFNCERTCLOSESTORE)(
		IN HCERTSTORE hCertStore,
		DWORD dwFlags
		);

typedef
PCCERT_CONTEXT (WINAPI *PFNCERTCREATECERTIFICATECONTEXT) (
		IN DWORD dwCertEncodingType,
		IN const BYTE *pbCertEncoded,
		IN DWORD cbCertEncoded
		);

typedef
BOOL (WINAPI *PFNCERTFREECERTIFICATECONTEXT)(
		IN PCCERT_CONTEXT pCertContext
		);

typedef
PCCERT_CONTEXT (WINAPI *PFNCERTENUMCERTIFICATESINSTORE)(
		IN HCERTSTORE hCertStore,
		IN PCCERT_CONTEXT pPrevCertContext
		);
typedef
PCCERT_CONTEXT (WINAPI *PFNCERTDUPLICATECERTIFICATECONTEXT)(
		IN PCCERT_CONTEXT pCertContext
		);
typedef
DWORD (WINAPI *PFNCERTGETNAMESTRINGW)(
		IN PCCERT_CONTEXT pCertContext,
		IN DWORD dwType,
		IN DWORD dwFlags,
		IN void *pvTypePara,
		OUT OPTIONAL LPWSTR pszNameString,
		IN DWORD cchNameString
		);
typedef
BOOL (WINAPI *PFNCERTDELETECERTIFICATEFROMSTORE)(
		IN PCCERT_CONTEXT pCertContext
		);
typedef
BOOL (WINAPI *PFNCERTGETCERTIFICATECONTEXTPROPERTY)(
		IN PCCERT_CONTEXT pCertContext,
		IN DWORD dwPropId,
		OUT void *pvData,
		IN OUT DWORD *pcbData
		);
typedef
BOOL (WINAPI *PFNCERTSETCERTIFICATECONTEXTPROPERTY)(
		IN PCCERT_CONTEXT pCertContext,
		IN DWORD dwPropId,
		IN DWORD dwFlags,
		IN const void *pvData
		);

typedef
BOOL (WINAPI * PFNCERTADDCERTIFICATECONTEXTTOSTORE)(
		IN HCERTSTORE hCertStore,
		IN PCCERT_CONTEXT pCertContext,
		IN DWORD dwAddDisposition, 
		OUT OPTIONAL PCCERT_CONTEXT * ppStoreContext);

typedef
BOOL (WINAPI *PFNCERTADDENCODEDCERTIFICATETOSTORE)(
		IN HCERTSTORE hCertStore,
		IN DWORD dwCertEncodingType,
		IN const BYTE *pbCertEncoded,
		IN DWORD cbCertEncoded,
		IN DWORD dwAddDisposition,
		OUT OPTIONAL PCCERT_CONTEXT *ppCertContext
		);
typedef 
BOOL (WINAPI * PFNCERTCOMPARECERTIFICATENAME) (
		IN DWORD dwCertEncodingType,
		IN PCERT_NAME_BLOB pCertName1,
		IN PCERT_NAME_BLOB pCertName2
		);
typedef
DWORD (WINAPI * PFNCERTNAMETOSTRW) (
		IN DWORD dwCertEncodingType,
		IN PCERT_NAME_BLOB pName,
		IN DWORD dwStrType,
		OUT OPTIONAL LPWSTR psz,
		IN DWORD csz
		);

typedef
BOOL (WINAPI  *PFNCERTGETENHANCEDKEYUSAGE) (
		IN     PCCERT_CONTEXT     pCertContext,
		IN     DWORD              dwFlags,
		OUT    PCERT_ENHKEY_USAGE pUsage,
		IN OUT DWORD*             pcbUsage
		);

typedef
PCCRYPT_OID_INFO (WINAPI *PFNCRYPTFINDOIDINFO)(
		IN DWORD dwKeyType,
		IN void *pvKey,
		IN DWORD dwGroupId
		);

typedef
BOOL (WINAPI *PFNCRYPTEXPORTPUBLICKEYINFO)(
		IN HCRYPTPROV hCryptProv,
		IN DWORD dwKeySpec,
		IN DWORD dwCertEncodingType,
		OUT PCERT_PUBLIC_KEY_INFO pInfo,
		IN OUT DWORD *pcbInfo
		);

typedef
PCCERT_CONTEXT (WINAPI *PFNCERTFINDCERTIFICATEINSTORE)(
		IN HCERTSTORE hCertStore,
		IN DWORD dwCertEncodingType,
		IN DWORD dwFindFlags,
		IN DWORD dwFindType,
		IN const void *pvFindPara,
		IN PCCERT_CONTEXT pPrevCertContext
		);

// imports from NETUI.DLL
static HINSTANCE g_hCertUI;
static LONG g_refCountCertUI;

PFNSHOWCERTIFICATE pShowCertificate;
PFNIMPORTCERTIFICATE pImportCertificate;

// imports from CRYPT32.DLL
static HINSTANCE g_hCrypt32;
static LONG g_refCount;

PFNCERTOPENSTORE pCertOpenStore;
PFNCERTCLOSESTORE pCertCloseStore;
PFNCERTCREATECERTIFICATECONTEXT pCertCreateCertificateContext;
PFNCERTFREECERTIFICATECONTEXT pCertFreeCertificateContext;
PFNCERTENUMCERTIFICATESINSTORE pCertEnumCertificatesInStore;
PFNCERTDUPLICATECERTIFICATECONTEXT pCertDuplicateCertificateContext;
PFNCERTGETNAMESTRINGW pCertGetNameStringW;
PFNCERTDELETECERTIFICATEFROMSTORE pCertDeleteCertificateFromStore;
PFNCERTGETCERTIFICATECONTEXTPROPERTY pCertGetCertificateContextProperty;
PFNCERTSETCERTIFICATECONTEXTPROPERTY pCertSetCertificateContextProperty;
PFNCERTADDCERTIFICATECONTEXTTOSTORE pCertAddCertificateContextToStore;
PFNCERTADDENCODEDCERTIFICATETOSTORE pCertAddEncodedCertificateToStore;
PFNCERTCOMPARECERTIFICATENAME pCertCompareCertificateName;
PFNCERTNAMETOSTRW pCertNameToStrW;
PFNCERTGETENHANCEDKEYUSAGE pCertGetEnhancedKeyUsage;
PFNCRYPTFINDOIDINFO pCryptFindOIDInfo;
PFNCRYPTEXPORTPUBLICKEYINFO pCryptExportPublicKeyInfo;
PFNCERTFINDCERTIFICATEINSTORE pCertFindCertificateInStore;

// WINSCARD.H functions
static HINSTANCE g_hWinscard;
static LONG g_refCountWinscard;
typedef  LONG (WINAPI *PFNSCARDESTABLISHCONTEXT)(
		IN  DWORD dwScope,
		IN  LPCVOID pvReserved1,
		IN  LPCVOID pvReserved2,
		OUT LPSCARDCONTEXT phContext);

typedef  LONG (WINAPI *PFNSCARDRELEASECONTEXT)(
		IN      SCARDCONTEXT hContext);
typedef LONG (WINAPI *PFNSCARDLISTREADERSW)(
		IN      SCARDCONTEXT hContext,
		IN      LPCWSTR mszGroups,
		OUT     LPWSTR mszReaders,
		IN OUT  LPDWORD pcchReaders);

typedef LONG (WINAPI *PFNSCARDLISTCARDSW)(
		IN      SCARDCONTEXT hContext,
		IN      LPCBYTE pbAtr,
		IN      LPCGUID rgquidInterfaces,
		IN      DWORD cguidInterfaceCount,
		OUT     LPWSTR mszCards,
		IN OUT  LPDWORD pcchCards);

typedef LONG (WINAPI *PFNSCARDGETSTATUSCHANGEW)(
		IN      SCARDCONTEXT hContext,
		IN      DWORD dwTimeout,
		IN OUT  LPSCARD_READERSTATEW rgReaderStates,
		IN      DWORD cReaders);

typedef  LONG (WINAPI *PFNSCARDGETCARDTYPEPROVIDERNAMEW)(
		IN SCARDCONTEXT hContext,
		IN LPCWSTR szCardName,
		IN DWORD dwProviderId,
		OUT LPWSTR szProvider,
		IN OUT LPDWORD pcchProvider);

PFNSCARDESTABLISHCONTEXT pSCardEstablishContext;
PFNSCARDRELEASECONTEXT pSCardReleaseContext;
PFNSCARDLISTREADERSW pSCardListReadersW;
PFNSCARDLISTCARDSW pSCardListCardsW;
PFNSCARDGETSTATUSCHANGEW pSCardGetStatusChangeW;
PFNSCARDGETCARDTYPEPROVIDERNAMEW pSCardGetCardTypeProviderNameW;


BOOL LoadCAPI2(void)
{
	if (InterlockedIncrement(&g_refCount) != 1)
		return g_hCrypt32 != 0;

	DEBUGCHK(g_hCrypt32 == NULL);
	g_hCrypt32 = LoadLibraryW(L"crypt32.dll");
	if (g_hCrypt32)
	{
		pCertOpenStore = (PFNCERTOPENSTORE)GetProcAddressW(g_hCrypt32, L"CertOpenStore");
		pCertCloseStore = (PFNCERTCLOSESTORE)GetProcAddressW(g_hCrypt32, L"CertCloseStore");
		pCertCreateCertificateContext = (PFNCERTCREATECERTIFICATECONTEXT) GetProcAddressW(g_hCrypt32, L"CertCreateCertificateContext");
		pCertFreeCertificateContext = (PFNCERTFREECERTIFICATECONTEXT)GetProcAddressW(g_hCrypt32, L"CertFreeCertificateContext");
		pCertEnumCertificatesInStore = (PFNCERTENUMCERTIFICATESINSTORE)GetProcAddressW(g_hCrypt32, L"CertEnumCertificatesInStore");
		pCertDuplicateCertificateContext = (PFNCERTDUPLICATECERTIFICATECONTEXT)GetProcAddressW(g_hCrypt32, L"CertDuplicateCertificateContext");
		pCertGetNameStringW = (PFNCERTGETNAMESTRINGW)GetProcAddressW(g_hCrypt32, L"CertGetNameStringW");
		pCertDeleteCertificateFromStore = (PFNCERTDELETECERTIFICATEFROMSTORE)GetProcAddressW(g_hCrypt32, L"CertDeleteCertificateFromStore");
		pCertGetCertificateContextProperty = (PFNCERTGETCERTIFICATECONTEXTPROPERTY)GetProcAddressW(g_hCrypt32, L"CertGetCertificateContextProperty");
		pCertSetCertificateContextProperty = (PFNCERTSETCERTIFICATECONTEXTPROPERTY)GetProcAddressW(g_hCrypt32, L"CertSetCertificateContextProperty");
		pCertAddCertificateContextToStore = (PFNCERTADDCERTIFICATECONTEXTTOSTORE) GetProcAddressW(g_hCrypt32, L"CertAddCertificateContextToStore");
		pCertAddEncodedCertificateToStore = (PFNCERTADDENCODEDCERTIFICATETOSTORE)GetProcAddressW(g_hCrypt32, L"CertAddEncodedCertificateToStore");
		pCertCompareCertificateName = (PFNCERTCOMPARECERTIFICATENAME)GetProcAddressW(g_hCrypt32, L"CertCompareCertificateName");
		pCertNameToStrW = (PFNCERTNAMETOSTRW)GetProcAddressW(g_hCrypt32, L"CertNameToStrW");
		pCertGetEnhancedKeyUsage = (PFNCERTGETENHANCEDKEYUSAGE)GetProcAddressW(g_hCrypt32, L"CertGetEnhancedKeyUsage");
		pCryptFindOIDInfo = (PFNCRYPTFINDOIDINFO)GetProcAddressW(g_hCrypt32, L"CryptFindOIDInfo");
		pCryptExportPublicKeyInfo = (PFNCRYPTEXPORTPUBLICKEYINFO)GetProcAddressW(g_hCrypt32, L"CryptExportPublicKeyInfo");
		pCertFindCertificateInStore = (PFNCERTFINDCERTIFICATEINSTORE)GetProcAddressW(g_hCrypt32, L"CertFindCertificateInStore");
		if (!pCertOpenStore || !pCertCloseStore || !pCertFreeCertificateContext
				|| !pCertEnumCertificatesInStore || !pCertDuplicateCertificateContext
				|| !pCertGetNameStringW || !pCertDeleteCertificateFromStore
				|| !pCertGetCertificateContextProperty || !pCertSetCertificateContextProperty
				|| !pCertAddCertificateContextToStore || !pCertAddEncodedCertificateToStore
				|| !pCertCompareCertificateName || !pCertNameToStrW
				|| !pCertGetEnhancedKeyUsage || !pCryptFindOIDInfo
				|| !pCryptExportPublicKeyInfo || !pCertFindCertificateInStore
		   )
		{
			FreeLibrary(g_hCrypt32);
			g_hCrypt32 = 0;
		}
	}
	return (g_hCrypt32 != 0);
}


// should be called once for every instance of LoadCAPI2 (whether or not it succeeds)
void FreeCAPI2(void)
{
	LONG refCount;
	if ((refCount = InterlockedDecrement(&g_refCount)) != 0)
		return;

	DEBUGCHK(refCount >= 0);
	if (g_hCrypt32)
	{
		FreeLibrary(g_hCrypt32);
		pCertOpenStore = NULL;
		pCertCloseStore = NULL;
		pCertFreeCertificateContext =  NULL;
		pCertEnumCertificatesInStore =  NULL;
		pCertDuplicateCertificateContext =  NULL;
		pCertGetNameStringW =  NULL;
		pCertDeleteCertificateFromStore =  NULL;
		pCertGetCertificateContextProperty =  NULL;
		pCertSetCertificateContextProperty =  NULL;
		pCertAddCertificateContextToStore = NULL;
		pCertAddEncodedCertificateToStore = NULL;
		pCryptExportPublicKeyInfo = NULL;
		pCertFindCertificateInStore = NULL;
		g_hCrypt32 = 0;
	}
}

BOOL LoadCertUI(void)
{
	if (InterlockedIncrement(&g_refCountCertUI) != 1)
		return g_hCertUI != 0;
	DEBUGCHK(g_hCertUI == NULL);
	g_hCertUI = LoadLibraryW(L"netui.dll");
	if (g_hCertUI)
	{
		pShowCertificate = (PFNSHOWCERTIFICATE)GetProcAddressW(g_hCertUI, L"ShowCertificate");
		pImportCertificate = (PFNIMPORTCERTIFICATE) GetProcAddressW(g_hCertUI, L"ImportCertificate");
	}
	return (g_hCertUI != 0);
}

void FreeCertUI(void)
{
	LONG refCount;
	if ((refCount = InterlockedDecrement(&g_refCountCertUI)) != 0)
		return;
	DEBUGCHK(refCount >= 0);
	if (g_hCertUI)
	{
		pShowCertificate = NULL;
		pImportCertificate = NULL;
		FreeLibrary(g_hCertUI);
		g_hCertUI = NULL;
	}

}

BOOL LoadWinscard(void)
{
	if (InterlockedIncrement(&g_refCountWinscard) != 1)
		return g_hWinscard != 0;

	DEBUGCHK(g_hWinscard == NULL);
	g_hWinscard = LoadLibraryW(L"winscard.dll");
	if (g_hWinscard)
	{
		pSCardEstablishContext = (PFNSCARDESTABLISHCONTEXT)GetProcAddressW(g_hWinscard, L"SCardEstablishContext");
		pSCardReleaseContext = (PFNSCARDRELEASECONTEXT)GetProcAddressW(g_hWinscard, L"SCardReleaseContext");
		pSCardListReadersW = (PFNSCARDLISTREADERSW)GetProcAddressW(g_hWinscard, L"SCardListReadersW");
		pSCardListCardsW = (PFNSCARDLISTCARDSW) GetProcAddressW(g_hWinscard, L"SCardListCardsW");
		pSCardGetStatusChangeW = (PFNSCARDGETSTATUSCHANGEW) GetProcAddressW(g_hWinscard, L"SCardGetStatusChangeW");
		pSCardGetCardTypeProviderNameW = (PFNSCARDGETCARDTYPEPROVIDERNAMEW) GetProcAddressW(g_hWinscard, L"SCardGetCardTypeProviderNameW");
		if (!pSCardEstablishContext || !pSCardReleaseContext
				|| !pSCardListReadersW || !pSCardListCardsW
				|| !pSCardGetStatusChangeW
				|| !pSCardGetCardTypeProviderNameW
		   )
		{
			FreeLibrary(g_hWinscard);
			g_hWinscard = 0;
		}
	}
	return (g_hWinscard != 0);

}

void FreeWinscard()
{
	LONG refCount;
	if ((refCount = InterlockedDecrement(&g_refCountWinscard)) != 0)
		return;

	DEBUGCHK(refCount >= 0);
	if (g_hWinscard)
	{
		pSCardEstablishContext =  NULL;
		pSCardReleaseContext =  NULL;
		pSCardListReadersW =  NULL;
		pSCardListCardsW = NULL;
		pSCardGetStatusChangeW = NULL;
		pSCardGetCardTypeProviderNameW = NULL;
		FreeLibrary(g_hWinscard);
		g_hWinscard = 0;
	}
}

static void PopulateCertStoreList(HWND hDlg, HCERTSTORE hStore)
{
	PCCERT_CONTEXT pCert;
	WCHAR szName[512];
	int ix;
	HWND hCtl = GetDlgItem(hDlg,IDC_CERTROOTS_LIST);
	// first free any cert pointers that are stashed in the list control
	for (ix = 0; ix < ListBox_GetCount(hCtl); ix++)
	{
		pCert = (PCCERT_CONTEXT)ListBox_GetItemData(hCtl, ix);
		if (pCert && (int)pCert != LB_ERR)
			pCertFreeCertificateContext(pCert);
	}
	ListBox_ResetContent(hCtl);

	if (hStore == NULL)
		return;
	for (pCert = NULL;
			pCert = pCertEnumCertificatesInStore(hStore,pCert);
		)
	{
			
		// Note: this API does want the size of the buffer in characters NOT bytes.
		pCertGetNameStringW(pCert, CERT_NAME_FRIENDLY_DISPLAY_TYPE,0,NULL,szName,sizeof(szName));
		ix = ListBox_AddString(hCtl, szName);
		ListBox_SetItemData(hCtl, ix, pCertDuplicateCertificateContext(pCert));
		// pCertEnumCertificatesInStore also frees the pCert
	} ;
	ListBox_SetCurSel(hCtl, 0);
}

static void RemoveCert(HWND hDlg)
{
	PCCERT_CONTEXT pCert, pCert2;
	HWND hCtl = GetDlgItem(hDlg,IDC_CERTROOTS_LIST);
	int ix;
	ix = ListBox_GetCurSel(hCtl);

	pCert = (PCCERT_CONTEXT)ListBox_GetItemData(hCtl,ix);

	if (!pCert || (int)pCert == LB_ERR)
		return ;

        // pCertDeleteCertificateFromStore always dereferences the pCert
       // in the off-chance that the certificate delete fails, we up
       // the refcount so we still have a valid cert context.
       pCert2 = pCertDuplicateCertificateContext(pCert);
	if (pCertDeleteCertificateFromStore(pCert2))
	{
		ListBox_DeleteString(hCtl,ix);
		pCertFreeCertificateContext(pCert);
	}
}			


static void RefreshCurrentCert(HWND hDlg)
{
	// used to refresh a cert in the list if we have to change the name
	// We have to remove the old entry, then insert a new, and finaly set the selection to the new

	PCCERT_CONTEXT pCert;
	HWND hCtl = GetDlgItem(hDlg,IDC_CERTROOTS_LIST);
	WCHAR szName[512] = L"";
	int ix = -1;
	
	ix = ListBox_GetCurSel(hCtl);
	if (-1 != ix)
	{

		pCert = (PCCERT_CONTEXT)ListBox_GetItemData(hCtl,ix);

		if (!pCert || (int)pCert == LB_ERR)
			return ;

		ListBox_DeleteString(hCtl,ix);

		
		pCertGetNameStringW(pCert, CERT_NAME_FRIENDLY_DISPLAY_TYPE,0,NULL,szName,sizeof(szName));

		ix = ListBox_AddString(hCtl, szName);
		ListBox_SetItemData(hCtl, ix, pCertDuplicateCertificateContext(pCert));
		ListBox_SetCurSel(hCtl, ix);
	}
}			




#define BEGINCERT_W                 L"-----BEGIN CERTIFICATE-----"
#define CBBEGINCERT_W               (sizeof(BEGINCERT_W)/sizeof(WCHAR) - 1)

#define BEGINCERT_A                 "-----BEGIN CERTIFICATE-----"
#define CBBEGINCERT_A               (sizeof(BEGINCERT_A)/sizeof(CHAR) - 1)

#define ENDCERT_W                 L"-----END CERTIFICATE-----"
#define ENDCERT_A                 "-----END CERTIFICATE-----"


static PBYTE ReadBase64Cert(PBYTE pbIn, DWORD cbIn, DWORD *pcbCert)
{
	DWORD cbFile = cbIn;
	DWORD cchDecoded, cbCert;
	DWORD i,j;
	PBYTE pbTemp, pbFile = pbIn;
	BOOL fSuccess = FALSE, fUnicode=FALSE;

	// Is it unicode base64 ?
	for (i = 0; i < cbFile && !pbFile[i+1]; i+=2);

	if (i == cbFile)
	{	// looks like unicode base64
		fUnicode = TRUE;
		// converT to ascii inline
		for (i =0,j=0; i < cbFile; i+= 2, j+=1)
			pbFile[j] = pbFile[i];
		cbFile /= 2;
	}

	// could be ascii base64
	if (cbFile > CBBEGINCERT_A*sizeof(CHAR)
			&& memcmp(BEGINCERT_A,pbFile, CBBEGINCERT_A*sizeof(CHAR)) == 0)
	{
		cbFile -= CBBEGINCERT_A*sizeof(CHAR);
		pbFile +=  CBBEGINCERT_A*sizeof(CHAR);

		// go to the end and look for the END CERTIFICATE sign
		if (pbTemp = (PBYTE) strstr((const char *)pbFile, ENDCERT_A))
		{
			cbFile = pbTemp - pbFile;
		}
	}
	pbTemp = (PBYTE)LocalAlloc(0,cbFile);	 // more than we need
	if (pbTemp)
	{
		if( Base64DecodeExA((char *)pbFile,cbFile, pbTemp, &cbCert, &cchDecoded) == 0
				&& (cchDecoded == cbFile))
		{
			memcpy(pbIn,pbTemp,cbCert);
			*pcbCert = cbCert;
			fSuccess = TRUE;
		}
		else if (fUnicode)
		{
			// undo the changes to the input
		}
		LocalFree(pbTemp);
	}
	return fSuccess ? pbIn : NULL;

}
//--------------------------------------------------------------------------
typedef struct _FILE_HDR {
	DWORD               dwMagic;
	DWORD               dwVersion;
	DWORD               dwKeySpec;
	DWORD               dwEncryptType;
	DWORD               cbEncryptData;
	DWORD               cbPvk;
} FILE_HDR, *PFILE_HDR;

#define PVK_FILE_VERSION_0          0
#define PVK_MAGIC                   0xb0b5f11e

// Private key encrypt types
#define PVK_NO_ENCRYPT                  0
#define PVK_RC4_PASSWORD_ENCRYPT        1
#define PVK_RC2_CBC_PASSWORD_ENCRYPT    2

#define MAX_PVK_FILE_LEN            4096

#define MAX_PASSWORD_LEN 16

typedef struct _KEY_PASSWORD_PARAM {
	CHAR          szPassword[MAX_PASSWORD_LEN];           // IDC_KEY
} KEY_PASSWORD_PARAM, *PKEY_PASSWORD_PARAM;

static INT_PTR CALLBACK KeyPasswordDlgProc(
		IN HWND hwndDlg,
		IN UINT uMsg,
		IN WPARAM wParam,
		IN LPARAM lParam
		);
//+-------------------------------------------------------------------------
//  Private Key helper  error codes
//--------------------------------------------------------------------------
#define PVK_HELPER_BAD_PARAMETER        0x80097001
#define PVK_HELPER_BAD_PVK_FILE         0x80097002
#define PVK_HELPER_WRONG_KEY_TYPE       0x80097003
#define PVK_HELPER_PASSWORD_CANCEL      0x80097004

//+-------------------------------------------------------------------------
//  Enter or Create Private Key Password Dialog Box
//--------------------------------------------------------------------------

static BOOL GetPasswordKey(
		IN HCRYPTPROV hProv,
		IN ALG_ID Algid,
		//IN PASSWORD_TYPE PasswordType,
		IN HWND hwndOwner,
		// IN LPCWSTR pwszKeyName,
		// IN BOOL fNoPassDlg,
		IN BYTE *pbSalt,
		IN DWORD cbSalt,
		OUT HCRYPTKEY *phEncryptKey
		)
{
	BOOL fResult;
	BYTE *pbPassword;
	DWORD cbPassword;
	HCRYPTHASH hHash = 0;
	HCRYPTKEY hEncryptKey = 0;
	KEY_PASSWORD_PARAM KeyPasswordParam; 

	if (IDOK != DialogBoxParam(
				g_hInst,
				MAKEINTRESOURCE(IDD_ENTERKEYPASSWORD),
				hwndOwner,
				KeyPasswordDlgProc,
				(LPARAM) &KeyPasswordParam
				))
	{
		SetLastError(PVK_HELPER_PASSWORD_CANCEL);
		goto ErrorReturn;
	}
	pbPassword = (PBYTE)KeyPasswordParam.szPassword;
	cbPassword = strlen(KeyPasswordParam.szPassword);

	if (cbPassword) {
		if (!CryptCreateHash(hProv, CALG_SHA, 0, 0, &hHash))
			goto ErrorReturn;
		if (cbSalt) {
			if (!CryptHashData(hHash, pbSalt, cbSalt, 0))
				goto ErrorReturn;
		}
		if (!CryptHashData(hHash, pbPassword, cbPassword, 0))
			goto ErrorReturn;
		if (!CryptDeriveKey(hProv, Algid, hHash, 0, &hEncryptKey))
			goto ErrorReturn;
	}

	fResult = TRUE;
	goto CommonReturn;

ErrorReturn:
	fResult = FALSE;
	if (hEncryptKey) {
		CryptDestroyKey(hEncryptKey);
		hEncryptKey = 0;
	}
CommonReturn:
	if (hHash)
		CryptDestroyHash(hHash);
	*phEncryptKey = hEncryptKey;
	return fResult;
}

static BOOL ImportPrivateKey(HWND hDlg, HCERTSTORE hStore, PBYTE pbFile, DWORD cbFile)
{
	BOOL fResult;
	FILE_HDR Hdr;
	HCRYPTPROV hCryptProv = NULL;
	HCRYPTKEY hDecryptKey = 0;
	HCRYPTKEY hKey = 0;
	BYTE *pbEncryptData = NULL;
	BYTE *pbPubKeyBlob = NULL;
	BYTE *pbPvk = NULL;
	DWORD cbPvk;
	TCHAR szKeyContainer[10];
	DWORD dwProvType = PROV_RSA_FULL;
	PCERT_PUBLIC_KEY_INFO pcertpubkeyinfo = NULL;
	CRYPT_KEY_PROV_INFO keyProvInfo;

	PCCERT_CONTEXT pCert = NULL;
	DWORD cbData;
	BOOL fDefaultCSP = FALSE;

	{
		int count = 0;
		PTCHAR pszId; 
		memcpy(szKeyContainer,TEXT("CERT"), sizeof(TEXT("CERT")));
		pszId = szKeyContainer+lstrlen(szKeyContainer);


		do {
			_itot(count, pszId, 10);
			// which provider do we open?
			// Try RSAENH and if that fails try the default RSA prov
			if (!fDefaultCSP)
				fResult = CryptAcquireContext(&hCryptProv,szKeyContainer,
						MS_ENHANCED_PROV,dwProvType,CRYPT_NEWKEYSET);
			if (!fResult && GetLastError() != NTE_EXISTS)
			{
				fResult = CryptAcquireContext(&hCryptProv,szKeyContainer,
						NULL,dwProvType,CRYPT_NEWKEYSET);
				fDefaultCSP = TRUE;
			}

		} while (!fResult && GetLastError() == NTE_EXISTS && ++count < 20);

	}
	if (!hCryptProv)
		return FALSE;
	// 
	// Read the file header and verify
	if (cbFile < sizeof(FILE_HDR))
		goto BadPvkFile;
	memcpy(&Hdr, pbFile, sizeof(FILE_HDR));	// ensure alignment

	if (Hdr.dwMagic != PVK_MAGIC)
		goto BadPvkFile;	// this is the only kind we know about

	// Treat as a "normal" private key file
	cbPvk = Hdr.cbPvk;
	if (Hdr.dwVersion != PVK_FILE_VERSION_0 ||
			Hdr.cbEncryptData > MAX_PVK_FILE_LEN ||
			cbPvk == 0 || cbPvk > MAX_PVK_FILE_LEN)
		goto BadPvkFile;


	if (cbFile - sizeof(FILE_HDR) < Hdr.cbEncryptData + cbPvk) {
		goto BadPvkFile;	// not enough data
	}

	if (Hdr.cbEncryptData)
		pbEncryptData = pbFile + sizeof(FILE_HDR);
	pbPvk = pbFile+sizeof(FILE_HDR) + Hdr.cbEncryptData;

	// check alignment?
	DEBUGCHK(!((int)pbPvk & 3));

	// repeat till the user cancels or we successfully import the key.
	while (TRUE)
	{
		// Get symmetric key to decrypt the private key
		// Check the algorithm used to encrypt the key.
		// Unfortunately we cannot get the keysize used for encryption
		// so we just guess that its the same as the default used by the CSP (128 bits, unless we picked the base RSA provider)
		switch (Hdr.dwEncryptType) {
			case PVK_NO_ENCRYPT:
				break;
			case PVK_RC4_PASSWORD_ENCRYPT:
				if (!GetPasswordKey(hCryptProv, CALG_RC4, //ENTER_PASSWORD,
							hDlg, 	//pwszKeyName, FALSE,
							pbEncryptData, Hdr.cbEncryptData,
							&hDecryptKey))
					goto ErrorReturn;
				break;
			case PVK_RC2_CBC_PASSWORD_ENCRYPT:
				if (!GetPasswordKey(hCryptProv, CALG_RC2, //ENTER_PASSWORD,
							hDlg,		//pwszKeyName, FALSE,
							pbEncryptData, Hdr.cbEncryptData,
							&hDecryptKey))
					goto ErrorReturn;
				break;
			default:
				goto BadPvkFile;
		}

		// Decrypt and import the private key
		// CRYPT_EXPORTABLE needed for SCHANNEL to work!!
		if (CryptImportKey(hCryptProv, pbPvk, cbPvk, hDecryptKey, CRYPT_EXPORTABLE, &hKey))
			break;
	}

	if (!pCryptExportPublicKeyInfo(hCryptProv, Hdr.dwKeySpec, CRYPT_ASN_ENCODING, NULL , &cbData)
			|| !(pcertpubkeyinfo = (PCERT_PUBLIC_KEY_INFO)LocalAlloc(0, cbData))
			|| !pCryptExportPublicKeyInfo(hCryptProv, Hdr.dwKeySpec, CRYPT_ASN_ENCODING, pcertpubkeyinfo , &cbData))
		goto ErrorReturn;


	// find a certificate in the store that matches this private key
	pCert = pCertFindCertificateInStore(hStore, CRYPT_ASN_ENCODING, 0, CERT_FIND_PUBLIC_KEY, pcertpubkeyinfo, NULL);

	if (!pCert)
		goto ErrorReturn;

	// Set the key provider info property to associate the private key
	// with the cert
	keyProvInfo.pwszContainerName = szKeyContainer;
	keyProvInfo.pwszProvName = fDefaultCSP ? NULL : MS_ENHANCED_PROV_W;
	keyProvInfo.dwProvType = dwProvType;
	keyProvInfo.dwFlags = 0;
	keyProvInfo.cProvParam = 0;
	keyProvInfo.rgProvParam = NULL;
	keyProvInfo.dwKeySpec = Hdr.dwKeySpec;

	fResult = pCertSetCertificateContextProperty(
			pCert,CERT_KEY_PROV_INFO_PROP_ID,
			0, &keyProvInfo);


	goto CommonReturn;

BadPvkFile:
	SetLastError(PVK_HELPER_BAD_PVK_FILE);
ErrorReturn:
	if (hCryptProv)
	{
		// delete the keycontainer which we just created
		CryptAcquireContext(&hCryptProv,szKeyContainer,
				NULL,dwProvType,CRYPT_DELETEKEYSET);
		hCryptProv = 0;
	}
	fResult = FALSE;
CommonReturn:
	if (pCert)
		pCertFreeCertificateContext(pCert);
	if (pcertpubkeyinfo)
		LocalFree(pcertpubkeyinfo);
	if (hDecryptKey)
		CryptDestroyKey(hDecryptKey);
	if (hKey)
		CryptDestroyKey(hKey);
	if (hCryptProv)
		CryptReleaseContext(hCryptProv, 0);
	return fResult;

}

static BOOL ImportCertOrKeyFromFile(HWND hDlg, HCERTSTORE hStore)
{
	OPENFILENAMEW ofn;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	TCHAR szPath[MAX_PATH+2];
	TCHAR szFilter[64];
	BOOL fRet = FALSE;
	DWORD cbFileSize=0;
	PBYTE pbFile=0, pbCert=0;

	szFilter[0] = '\0';
	int i, iFilterLen = LOADSZ(IDS_CERTCPL_FILTER, szFilter);
	// substitute code 1 with 0
	for (i = 0; i < iFilterLen;i++)
	{
		if (szFilter[i] == '\1')
			szFilter[i] = '\0';
	};

	szPath[0] = 0;
	ZEROMEM(&ofn);
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hDlg;
	ofn.hInstance = g_hInst;
	ofn.lpstrFilter = szFilter;	// "Certificate\0*.cer\0PrivateKeys\0*.pvk"
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szPath; 			 
	ofn.nMaxFile = CCHSIZEOF(szPath);		
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = CELOADSZ(IDS_CERT_SELECT); 
	ofn.Flags =  OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = TEXT("cer");


	if (!GetOpenFileName(&ofn)) goto done;

	// Try to open the selected file
	hFile = CreateFile(ofn.lpstrFile, GENERIC_READ, 0, 
			NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) goto done;

	cbFileSize = GetFileSize(hFile,NULL);

	pbFile = (PBYTE)LocalAlloc(LPTR,cbFileSize);
	if (!pbFile) goto done;

	if (!ReadFile(hFile,pbFile, cbFileSize,&cbFileSize,NULL)) goto done;

	if (ofn.nFilterIndex == 1)
	{
		// It is a cert.

		// Is it a BASE 64 encoded x509 cert?
		pbCert = ReadBase64Cert(pbFile, cbFileSize, &cbFileSize);
		if (!pbCert) pbCert = pbFile;

		// Is it a raw DER encoded x509 cert?
		fRet = pCertAddEncodedCertificateToStore(hStore, X509_ASN_ENCODING,
				pbCert,cbFileSize, CERT_STORE_ADD_NEWER_INHERIT_PROPERTIES,0);
	}
	else if (ofn.nFilterIndex == 2)
	{
		// It is a Private key file
		fRet = ImportPrivateKey(hDlg, hStore, pbFile, cbFileSize);
	}

done:
	if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
	if (pbFile)	LocalFree(pbFile);							
	return fRet;
}
#define MAX_READERS	4
#define MAX_CARDNAME    64
#define MAX_READERNAME 64
static BOOL GetSmartcardList(SCARD_READERSTATEW ** ppReaderStates)
{
	long status;
	SCARDCONTEXT hSCtxt;
	static TCHAR szReaders[MAX_PATH];
	static TCHAR szCards[MAX_READERS][MAX_CARDNAME];
	DWORD dwLen;
	DWORD i, cReaders = 0;
	PTCHAR  pszName;
	static SCARD_READERSTATEW readerStates[MAX_READERS];
	status = pSCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hSCtxt);
	if (status != NO_ERROR)
		goto cleanup;
	dwLen = MAX_PATH;
	status = pSCardListReadersW( hSCtxt, NULL, szReaders, &dwLen);
	if (status != NO_ERROR)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("SCardListReaderss failed %x\n"), status));
		goto cleanup;
	}
	for (pszName = szReaders, i = 0; *pszName && i < MAX_READERS; i++, pszName += wcslen(pszName) + 1)
	{
		readerStates[i].dwCurrentState = SCARD_STATE_UNAWARE;
		readerStates[i].szReader = pszName;
		readerStates[i].pvUserData = szCards[i];
		szCards[i][0] = '\0';
	}
	cReaders = i;
	if (!cReaders)
	{
		DEBUGMSG(ZONE_WARNING, (TEXT("No smart card readers.\n")));
		goto cleanup;
	}
	// this should return right away since we set the current state to UNAWARE
	status = pSCardGetStatusChangeW(hSCtxt, INFINITE, readerStates, cReaders);
	if (status == NO_ERROR)
	{
		while (i--)
		{
			if( (readerStates[i].dwEventState & SCARD_STATE_PRESENT)
					&& (readerStates[i].cbAtr))
			{
				dwLen = MAX_CARDNAME;
				status = pSCardListCardsW(hSCtxt, readerStates[i].rgbAtr, NULL, 0, szCards[i], &dwLen);
				if (status != NO_ERROR)
				{
					DEBUGMSG(ZONE_WARNING, (TEXT("Ignoring unrecognized card in reader %s (error %x)\n"), readerStates[i].szReader,  status));
					continue;
				}

			}
		}
	}
	*ppReaderStates = &readerStates[0];
cleanup:
	if (hSCtxt)
		pSCardReleaseContext( hSCtxt);
	return cReaders;

}

static BOOL
ImportCertFromSC(HCERTSTORE hStore,HCRYPTPROV hProv, CRYPT_KEY_PROV_INFO *pKeyProvInfo )
{
	DWORD cbCertLen = 0;
	BYTE *pbCert=0;
	HCRYPTKEY hCurKey = 0;
	BOOL fRet = FALSE;
	DWORD dwPara = CERT_X500_NAME_STR;
	PCCERT_CONTEXT  pCertCtx=0;

	if (!hStore || !hProv ) goto done;

	if (!CryptGetUserKey(hProv, pKeyProvInfo->dwKeySpec, &hCurKey)) goto done;

	// Get CertLen
	if (!CryptGetKeyParam(hCurKey, KP_CERTIFICATE, 0, &cbCertLen, 0))
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("cannot get cert len\n")));
		goto done;
	}

	// Allocate memory for cert.
	if (!(pbCert = new BYTE[cbCertLen]))
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("cannot alloc cert.\n")));
		goto done;
	}


	// Fill cert structure.
	if (!CryptGetKeyParam(hCurKey, KP_CERTIFICATE, pbCert, &cbCertLen, 0))
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("cannot read certificate\n")));
		goto done;
	}

	// Create cert structure so can set the Key Provider on it.
	pCertCtx   = pCertCreateCertificateContext(X509_ASN_ENCODING,pbCert,cbCertLen);
	if (!pCertCtx) goto done;

	fRet = pCertSetCertificateContextProperty(
			pCertCtx, CERT_KEY_PROV_INFO_PROP_ID, 
			0, pKeyProvInfo);
	if (!fRet) 
	{
		DEBUGMSG(ZONE_ERROR,
				(TEXT("Cannot set certificate context property (error %x)\n") ,
				 GetLastError()));
		goto done;
	}

	// Add the cert to the certstore.
	fRet = pCertAddCertificateContextToStore(hStore, 
			pCertCtx, CERT_STORE_ADD_NEWER, NULL);

	if(!fRet)
	{
		if (GetLastError() == CRYPT_E_EXISTS)
		{
			DEBUGMSG(ZONE_WARNING, (TEXT("Certificate exists in  store\n")));
		}
		else
		{
			DEBUGMSG(ZONE_ERROR, 
				(TEXT("Error adding certificate to store (error %x)\n"), GetLastError()));
		}
		goto done;
	}
	DEBUGMSG(ZONE_WARNING, (TEXT("Certificate added to store\n")));

done:
	if (pCertCtx) pCertFreeCertificateContext(pCertCtx);
	if (pbCert) delete [] pbCert;
	if (hCurKey) CryptDestroyKey(hCurKey);

	return fRet;
}

#define MAX_CSPNAME	80	

static DWORD GetTypeForCSP(PCWSTR szCSP)
{
	DWORD index = 0;
	DWORD dwProvType;
	WCHAR szProvName[MAX_CSPNAME];
	DWORD cbProvName = sizeof(szProvName);
	while (CryptEnumProvidersW(index,NULL, 0,&dwProvType,szProvName, &cbProvName) == TRUE)
	{
		if (wcscmp(szCSP, szProvName) == 0)
		{
			return dwProvType;
		}
		index++;
		cbProvName = sizeof(szProvName);
	}
	return 0;
}

static BOOL ImportAllCertsFromSC(HCERTSTORE hStore, PCWSTR szReader, PCWSTR szCard)
{
	TCHAR szCSP[MAX_PATH];
	WCHAR szKeyContainer[MAX_PATH];
	HCRYPTPROV hProv = 0;
	HCRYPTPROV hProvPerKeyContainer = 0;
	DWORD dwFlags;
	DWORD dwLen;
	DWORD dwProvType;
	long status;
	long cCerts=0;
	CRYPT_KEY_PROV_INFO keyProvInfo;
	SCARDCONTEXT hSCtxt;
	// we are only interested in cards with the CSP providers
	dwLen = MAX_PATH;
	status = pSCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hSCtxt);
	if (status != NO_ERROR) return FALSE;

	status = pSCardGetCardTypeProviderNameW(hSCtxt, szCard, SCARD_PROVIDER_CSP, szCSP, &dwLen);
	if (status != NO_ERROR)
	{
		DEBUGMSG(ZONE_WARNING, 
				(TEXT("Ignoring card %s because it has no CSP provider (error %x)\n"), 
				 szCard, status));
		goto Ret;
	}
	DEBUGMSG(ZONE_WARNING, (TEXT("Loading %s for card %s in reader %s\n"), szCSP,
				szCard, szReader));

	// assume RSA provider
	hProv = 0;
	dwProvType = GetTypeForCSP(szCSP);
	if(!CryptAcquireContextW(&hProv, NULL, szCSP, dwProvType, 0)
			&& GetLastError() == NTE_BAD_KEYSET)
	{
		CryptAcquireContextW(&hProv, NULL, szCSP, dwProvType, CRYPT_NEWKEYSET);
	}

	if (!hProv)
	{
		DEBUGMSG(ZONE_ERROR,(TEXT("Cannot load provider %s (error %x)\n"), szCSP, 
					GetLastError()));
		goto Ret;
	}
	dwFlags = CRYPT_FIRST;
	dwLen = sizeof(szKeyContainer);

	while(CryptGetProvParam(hProv, PP_ENUMCONTAINERS, (PBYTE) szKeyContainer, 
				&dwLen, dwFlags))
	{
		dwProvType = PROV_RSA_FULL; 

		if (!CryptAcquireContextW(&hProvPerKeyContainer, szKeyContainer, szCSP, dwProvType, 0))
		{
			ASSERT(0); // We enumed it it should exist.
			continue;
		}

		// Lets make a keyProvInfo Function.
		keyProvInfo.pwszContainerName = szKeyContainer;
		keyProvInfo.dwFlags = 0;
		keyProvInfo.dwProvType = dwProvType;
		keyProvInfo.pwszProvName = szCSP;
		keyProvInfo.cProvParam = 0;
		keyProvInfo.rgProvParam = NULL;


		//First try to nab the signature cert.
		keyProvInfo.dwKeySpec = AT_SIGNATURE;
		if (ImportCertFromSC(hStore,hProvPerKeyContainer,&keyProvInfo))
		{
			++cCerts;
			DEBUGMSG(ZONE_WARNING, (TEXT
			("Imported SIGN cert from key container %s\n"), szKeyContainer));
		}

		//Then nab the key exchange cert.
		keyProvInfo.dwKeySpec = AT_KEYEXCHANGE;
		if (ImportCertFromSC(hStore, hProvPerKeyContainer, &keyProvInfo))
		{
			++cCerts;
			DEBUGMSG(ZONE_WARNING, (TEXT
			("Imported XCHANGE cert from key container %s\n"), szKeyContainer));
		}

		dwFlags = 0;
		dwLen = sizeof(szKeyContainer);
		if (hProvPerKeyContainer ) CryptReleaseContext(hProvPerKeyContainer,0);
	}
Ret:
	pSCardReleaseContext(hSCtxt);
	if (hProv) CryptReleaseContext(hProv,0);

	return cCerts;
}

BOOL CALLBACK ImportCertDialogProc(
		HWND hDlg, 
		UINT uMsg, 
		WPARAM wParam, 
		LPARAM lParam)
{
	BOOL fRet = TRUE;
	BOOL fSuccess;
	HCERTSTORE hStore;
	LONG wCmdId;
	LONG wCmdCbn;
	BOOL fScard;
	DWORD i, iSelected;
	PTCHAR pszCard;
	static SCARD_READERSTATEW *pReaderStates;
	TCHAR szCardState[64];

	switch (uMsg) {
		case WM_NOTIFY :
			fRet = FALSE;
			break;
		case WM_INITDIALOG :
			{
				hStore = (HCERTSTORE) lParam;
				fScard = (g_hWinscard != 0);
				SetWindowLong (hDlg, DWL_USER, (LONG)hStore);
				if (hStore)
				{
					pReaderStates = NULL;
					// default to Import from File
					Button_SetCheck(GetDlgItem(hDlg, IDC_RADIO_FILECERT), TRUE);

					if (fScard)
					{
						DWORD cReaders = GetSmartcardList(&pReaderStates);
						iSelected = -1;
						if (cReaders)
						{
							// there are smart card readers so enable all the smartcard specific controls
							HWND hSCButton = GetDlgItem(hDlg, IDC_RADIO_CARDCERT);
							HWND hSCCombo = GetDlgItem(hDlg, IDC_COMBO_SMARTCARD);
							HWND hSCCard = GetDlgItem(hDlg, IDC_STATIC_CARDNAME);
							Button_Enable(hSCButton, TRUE);
							ComboBox_Enable(hSCCombo, TRUE);
							Static_Enable(hSCCard, TRUE);
							Static_Enable(GetDlgItem(hDlg, IDC_STATIC_CARD), TRUE);
							Static_Enable(GetDlgItem(hDlg, IDC_STATIC_READER), TRUE);
							for (i=0; i < cReaders; i++)
							{
								ComboBox_AddString(hSCCombo, pReaderStates[i].szReader);
								pszCard = (PTCHAR) pReaderStates[i].pvUserData;
								if (iSelected == -1 && pszCard && pszCard[0])
								{
									iSelected = i;
								}

							}
							if (iSelected == -1)
							{
								iSelected = 0;
							}
							pszCard = (PTCHAR) pReaderStates[iSelected].pvUserData;
							if (pszCard && !pszCard[0])
							{
								szCardState[0] = '\0';
								if (pReaderStates[iSelected].dwEventState & SCARD_STATE_EMPTY)
									LOADSZ(IDS_CARD_ABSENT, szCardState);
								else
									LOADSZ(IDS_CARD_UNKNOWN, szCardState);
								pszCard = szCardState;
							}
							ComboBox_SetCurSel(hSCCombo, iSelected);
							Static_SetText(hSCCard, pszCard );
							// select the import from smartcard button
							Button_SetCheck(GetDlgItem(hDlg, IDC_RADIO_FILECERT), FALSE);
							Button_SetCheck(hSCButton, TRUE);
						}
					}
                    
                    LoadAygshellLibrary();
                    AygAddSipprefControl( hDlg );
				}
				else
					fRet = FALSE;
			}
			break;
 
        case WM_DESTROY:
            FreeAygshellLibrary();
            break;

		case WM_COMMAND :
			wCmdId  = LOWORD(wParam);
			wCmdCbn = HIWORD(wParam);
			fSuccess = FALSE;

			switch ( wCmdId ) {


				case IDOK:
					{
						hStore = (HCERTSTORE)GetWindowLong(hDlg, DWL_USER);
						if (Button_GetCheck(GetDlgItem(hDlg, IDC_RADIO_FILECERT)))
						{
							fSuccess = ImportCertOrKeyFromFile(hDlg, hStore);
						}
						else if (Button_GetCheck(GetDlgItem(hDlg, IDC_RADIO_CARDCERT)))
						{
							iSelected	= ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COMBO_SMARTCARD));
							if (iSelected != CB_ERR && iSelected < MAX_READERS)
							{

								fSuccess = ImportAllCertsFromSC(hStore,
										pReaderStates[iSelected].szReader,
										(PTCHAR) pReaderStates[iSelected].pvUserData);

							}
						}

						// fall through
					}
				case IDCANCEL:
					EndDialog(hDlg,fSuccess);
					break;

				case IDC_COMBO_SMARTCARD:
					{
						switch (wCmdCbn)
						{
							case CBN_SELCHANGE:
								// Selected a different reader. update the card name field
								iSelected	= ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COMBO_SMARTCARD));
								if (iSelected != CB_ERR)
								{
									pszCard = (PTCHAR) pReaderStates[iSelected].pvUserData;
									if (!pszCard[0])
									{
										if (pReaderStates[iSelected].dwEventState & SCARD_STATE_EMPTY)
											LOADSZ(IDS_CARD_ABSENT, szCardState);
										else
											LOADSZ(IDS_CARD_UNKNOWN, szCardState);
										pszCard = szCardState;
									}
								}

								Static_SetText(GetDlgItem(hDlg, IDC_STATIC_CARDNAME), pszCard);
								break;

							default:
								fRet = FALSE;
								break;


						}
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

BOOL ImportCertificate(HWND hParent, HCERTSTORE hStore)
{
	BOOL fRet;
	HCURSOR hCur;
	LoadWinscard();

	{
		hCur = SetCursor(NULL);

		fRet = DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_IMPORTCERT),
				hParent, ImportCertDialogProc, (LONG) hStore);
		SetCursor(hCur);

	}

	FreeWinscard();
	return fRet;
}

#if 0
static int FormatDate(FILETIME ft, PTCHAR pszBuf, UINT cch)
{
	SYSTEMTIME st;
	FileTimeToSystemTime(&ft, &st);
	return GetDateFormat(LOCALE_USER_DEFAULT,DATE_SHORTDATE,&st,NULL,pszBuf,cch); 
}

#define CRYPTUI_MAX_STRING_SIZE 256
static BOOL PopulateEKU(HWND hDlg, PCCERT_CONTEXT pCert)
{
	DWORD cbUsage;
	PCERT_ENHKEY_USAGE pUsage;
	PCCRYPT_OID_INFO pOidInfo;
	TCHAR rgch[CRYPTUI_MAX_STRING_SIZE];
	UINT i;
	HWND hCtl = GetDlgItem(hDlg,IDC_GOODFOR_EDIT);
	if (!pCertGetEnhancedKeyUsage(pCert, 0, NULL, &cbUsage))
		return FALSE;

	pUsage = (PCERT_ENHKEY_USAGE)LocalAlloc(0, cbUsage);
	if (!pUsage)
		return FALSE;

	if (pCertGetEnhancedKeyUsage(pCert, 0, pUsage, &cbUsage))
	{
		for (i=0; i < pUsage->cUsageIdentifier; i++)
		{
			pOidInfo = pCryptFindOIDInfo(
					CRYPT_OID_INFO_OID_KEY,
					pUsage->rgpszUsageIdentifier[i],
					CRYPT_ENHKEY_USAGE_OID_GROUP_ID);
			if (pOidInfo)
			{
				ListBox_AddString(hCtl, pOidInfo->pwszName);
			}
			else
			{
				// couldn't find a descriptive name for the OID.
				// display the OID itself
#ifdef UNICODE
				mbstowcs(rgch, pUsage->rgpszUsageIdentifier[i], CRYPTUI_MAX_STRING_SIZE);
#else
				strncpy(rgch, pUsage->rgpszUsageIdentifier[i], CRYPTUI_MAX_STRING_SIZE);
#endif
				rgch[CRYPTUI_MAX_STRING_SIZE-1] = 0;    // ensure null-termination
				ListBox_AddString(hCtl, rgch);
			}
		}
	}
	return TRUE;

}

extern "C" BOOL CALLBACK
ShowCertDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	PCCERT_CONTEXT pCert;
	TCHAR               rgch[CRYPTUI_MAX_STRING_SIZE];
	switch(message)
	{
		case WM_INITDIALOG :
			DEBUGMSG(ZONE_MSGS, (L"ShowCertDlgProc: Got WM_INITDLG....\r\n"));
			pCert = (PCCERT_CONTEXT)lParam;
			//
			// set the subject and issuer name
			//
			pCertGetNameStringW(
					pCert,
					CERT_NAME_FRIENDLY_DISPLAY_TYPE,
					0,//CERT_NAME_ISSUER_FLAG,
					NULL,
					rgch,
					ARRAYSIZEOF(rgch));
			SetDlgItemText(hDlg, IDC_SUBJECT_EDIT, rgch);

			pCertGetNameStringW(
					pCert,
					CERT_NAME_FRIENDLY_DISPLAY_TYPE,
					CERT_NAME_ISSUER_FLAG,
					NULL,
					rgch,
					ARRAYSIZEOF(rgch));
			SetDlgItemText(hDlg,IDC_ISSUER_EDIT, rgch);

			// set the permitted usages
			PopulateEKU(hDlg, pCert);

			// set the time validity
			FormatDate(pCert->pCertInfo->NotBefore,rgch,ARRAYSIZEOF(rgch));
			SetDlgItemText(hDlg, IDC_VALID_FROM, rgch);

			FormatDate(pCert->pCertInfo->NotAfter,rgch,ARRAYSIZEOF(rgch));
			SetDlgItemText(hDlg, IDC_VALID_UNTIL, rgch);
			return TRUE;

		case WM_COMMAND :
			switch (LOWORD(wParam)) 
			{ 
				case IDOK: 
					EndDialog(hDlg, wParam); 
					return TRUE; 
			} 	
		default:
			break;
	}
	return FALSE;
}

#endif

void DoEnableButtons(HWND hDlg)
{
   HWND hwndCtrl, hwndCtrl1;
   
   // If IDC_CERTROOTS_LIST is empty: disable View and Remove buttons              
   hwndCtrl = GetDlgItem(hDlg, IDC_CERTROOTS_VIEW);
   hwndCtrl1 = GetDlgItem(hDlg, IDC_CERTROOTS_REMOVE);
               
   if (0 != ListBox_GetCount(DI(IDC_CERTROOTS_LIST)))
   {
      EnableWindow(hwndCtrl, TRUE);
      EnableWindow(hwndCtrl1, TRUE);
   }
   else
   {
      EnableWindow(hwndCtrl, FALSE);
      EnableWindow(hwndCtrl1, FALSE);
   }
}

extern "C" BOOL CALLBACK
CertRootDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HCERTSTORE hStore;
	static const TCHAR *pRootDesc, *pMyDesc, *pCADesc;
	static int iRoot, iMy, iCA;
	PCCERT_CONTEXT pCert = NULL;
	int i;
	const TCHAR *pDesc;
	HWND hCntl;

	switch(message)
	{
		case WM_INITDIALOG :
#ifdef DEBUG
			//	DebugBreak();
#endif

			DEBUGMSG(ZONE_MSGS, (L"CertRootDlgProc: Got WM_INITDLG....\r\n"));
			if (!LoadCAPI2())
			{
				DEBUGMSG(ZONE_ERROR,(L"CertRootDlgProc: Cannot Load crypt32.dll\r\n"));
				EndDialog(hDlg, 0);
				hStore = 0;
				return 0;
			}
			if (!LoadCertUI())
			{
				DEBUGMSG(ZONE_ERROR,(L"CertRootDlgProc: Cannot Load netui.dll\r\n"));
			}
			pRootDesc = CELOADSZ(IDS_CERTCPL_ROOT_DESCRIPTION);
			pMyDesc = CELOADSZ(IDS_CERTCPL_MY_DESCRIPTION);
			pCADesc = CELOADSZ(IDS_CERTCPL_CA_DESCRIPTION);
			hCntl = DI(IDC_CERTSTORE_SELECT);
			iRoot = ComboBox_AddString(hCntl, CELOADSZ(IDS_CERTCPL_ROOTSTORE));	// Root store
			ComboBox_SetItemData(hCntl, iRoot, pRootDesc);
			iMy = ComboBox_AddString(hCntl, CELOADSZ(IDS_CERTCPL_MYSTORE));			// My store
			ComboBox_SetItemData(hCntl, iMy, pMyDesc);
			iCA = ComboBox_AddString(hCntl, CELOADSZ(IDS_CERTCPL_CASTORE));			// intermediate CA store
			ComboBox_SetItemData(hCntl, iCA, pCADesc);
			// Make "Root" the current selection
			i = ComboBox_SetCurSel(hCntl,iRoot);	
			// initialize the static text to "Lists certificates you trust"
			Static_SetText(DI(IDC_CERTCPL_STATIC_STORE),pRootDesc);
			// MAXIMUM_ALLOWED flags gives us write access to stores that would otherwise be read-only
			hStore = pCertOpenStore(CERT_STORE_PROV_SYSTEM,0,0,
					CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_MAXIMUM_ALLOWED_FLAG ,
					TEXT("Root"));
			if (hStore)
				PopulateCertStoreList(hDlg, hStore);

         DoEnableButtons(hDlg);
            AygInitDialog( hDlg, SHIDIF_SIPDOWN );
			return 1; // ask GWE to set focus

		case WM_COMMAND :
			switch (LOWORD (wParam))
			{
				case IDCANCEL:
				case IDOK:
					DEBUGMSG(ZONE_MSGS, (L"CertRootDlgProc: Got IDOK....\r\n"));
					if (hStore != NULL)
						pCertCloseStore(hStore, CERT_CLOSE_STORE_FORCE_FLAG);
					FreeCAPI2();
					FreeCertUI();
					return TRUE;

				case IDC_CERTSTORE_SELECT:
					if (HIWORD(wParam) != CBN_SELCHANGE) break;
					// different cert store selected
					i = ComboBox_GetCurSel(DI(IDC_CERTSTORE_SELECT));
					pDesc = (const TCHAR *)ComboBox_GetItemData(DI(IDC_CERTSTORE_SELECT), i);
					Static_SetText(DI(IDC_CERTCPL_STATIC_STORE),pDesc);
					if (hStore)
					{
						pCertCloseStore(hStore,0);	
						// certs will be freed in PopulateCertStoreList
						hStore = NULL;
					}
					hStore = pCertOpenStore(CERT_STORE_PROV_SYSTEM,0,0,
							CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_MAXIMUM_ALLOWED_FLAG ,
							i == iMy ? TEXT("My") : ( i == iRoot ? TEXT("Root") : TEXT("CA")));
					PopulateCertStoreList(hDlg, hStore);

               DoEnableButtons(hDlg);
					return TRUE;

				case IDC_CERTROOTS_LIST:
					if(HIWORD(wParam) != LBN_DBLCLK) break;
					DEBUGMSG(ZONE_MSGS, (L"CertRootDlgProc: Double Clicked on item\r\n"));
					// Fall into the view code.

				case IDC_CERTROOTS_VIEW:
					i = ListBox_GetCurSel(DI(IDC_CERTROOTS_LIST));

					pCert = (PCCERT_CONTEXT)ListBox_GetItemData(DI(IDC_CERTROOTS_LIST), i);


					if (!pCert || (int)pCert == LB_ERR)
						return TRUE;

					if (pShowCertificate)
					{
						pShowCertificate(hDlg, pCert);

						// because we can't be sure the user didn't change the friendly name
						// we need to refresh the cert.
						RefreshCurrentCert(hDlg);
					}
#if 0
					else
						DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_CERTPROP_GENERAL),
								hDlg, ShowCertDlgProc, (LPARAM)pCert);
#endif			
					return TRUE;
				case IDC_CERTROOTS_REMOVE:
					RemoveCert(hDlg);
               DoEnableButtons(hDlg);

					return TRUE;
				case IDC_CERTROOTS_IMPORT:
					if (ImportCertificate(hDlg,hStore))
					{
						PopulateCertStoreList(hDlg, hStore);
                  DoEnableButtons(hDlg);
					}
					return TRUE;

			}
			break;
		case WM_CLOSE:
			break;
	}
	DEBUGMSG (ZONE_MSGS, (L"-CertRootDlgProc: Return FALSE\r\n"));
	return FALSE; // message was not handled
}

//+-------------------------------------------------------------------------
//  Enter or Create Private Key Password DialogProc
//--------------------------------------------------------------------------
static INT_PTR CALLBACK KeyPasswordDlgProc(
		IN HWND hwndDlg,
		IN UINT uMsg,
		IN WPARAM wParam,
		IN LPARAM lParam
		)
{
	switch (uMsg) {
		case WM_INITDIALOG:
			{
                LoadAygshellLibrary();
				PKEY_PASSWORD_PARAM pKeyPasswordParam =
					(PKEY_PASSWORD_PARAM) lParam;

				SetWindowLong(hwndDlg, DWL_USER, (INT_PTR) pKeyPasswordParam);

                AygAddSipprefControl( hwndDlg );
				return TRUE;
			}
		case WM_COMMAND:
            {
			    int nResult = LOWORD(wParam);
			    switch (nResult) {
				    case IDOK:
					    {
						    PKEY_PASSWORD_PARAM pKeyPasswordParam =
							    (PKEY_PASSWORD_PARAM) GetWindowLong(hwndDlg, DWL_USER);
						    WCHAR wszPassword[MAX_PASSWORD_LEN];
						    wszPassword[0] = 0;
						    GetDlgItemTextW(
								    hwndDlg,
								    IDC_PASSWORD0,
								    wszPassword,
								    sizeof(wszPassword)/sizeof(WCHAR));
						    wcstombs(pKeyPasswordParam->szPassword, wszPassword, sizeof(pKeyPasswordParam->szPassword));
						    SecureZeroMemory(wszPassword,sizeof(wszPassword));

					    }
					    // fall through
				    case IDCANCEL:
					    EndDialog(hwndDlg, nResult);
					    return TRUE;
			    }
            }
            break;
        case WM_DESTROY:
            FreeAygshellLibrary();
            break;
	}
	return FALSE;
}
