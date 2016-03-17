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
// showcert.cpp : Defines the entry point for the application.
//

#include "windows.h"
#include "windowsx.h"
#include <commctrl.h>
#include "wcommctl.h"
#include <wincrypt.h>
#include <certui.h>
#include <certuip.h>
#include <winscard.h>
#include <netuip.h>
#include <schnlsp.h>
#include "resource.h"

// private prototypes,and defines.
#define TYPICAL_MD5_HASH_SIZE 16

#define NUM_DN_UNITS    6
#define MAX_ITEM_LEN    1000

#define DN_COMMON_NAME  0
#define DN_COUNTRY      1
#define DN_ORG          2
#define DN_ORGUNIT      3
#define DN_LOCALE       4
#define DN_STATE        5

#define MAX_CERT_FIELDS 20
#define CRYPTUI_MAX_STRING_SIZE	256

typedef struct _ATTR_MAP
{
	DWORD dwAttr;
	DWORD dwStringID;
} ATTR_MAP;

// Now for some common attribute maps

ATTR_MAP ProtocolAttrMap[] =
{
	{SP_PROT_SSL2_CLIENT,IDS_PROTOCOL_SSL2},
	{SP_PROT_SSL3_CLIENT,IDS_PROTOCOL_SSL3},
	{SP_PROT_PCT1_CLIENT,IDS_PROTOCOL_PCT1},
	{SP_PROT_TLS1_CLIENT,IDS_PROTOCOL_TLS1}
};

ATTR_MAP AlgAttrMap[] =
{
	{CALG_MD2,IDS_ALG_MD2},
	{CALG_MD4,IDS_ALG_MD4},
	{CALG_MD5,IDS_ALG_MD5},
	{CALG_SHA,IDS_ALG_SHA},
	{CALG_SHA1,IDS_ALG_SHA},
	{CALG_MAC,IDS_ALG_MAC},
	{CALG_HMAC,IDS_ALG_HMAC},
	{CALG_RSA_SIGN,IDS_ALG_RSA_SIGN},
	{CALG_DSS_SIGN,IDS_ALG_DSS_SIGN},
	{CALG_RSA_KEYX,IDS_ALG_RSA_KEYX},
	{CALG_DES,IDS_ALG_DES},
	{CALG_3DES_112,IDS_ALG_3DES_112},
	{CALG_3DES,IDS_ALG_3DES},
	{CALG_RC2,IDS_ALG_RC2},
	{CALG_RC4,IDS_ALG_RC4},
	{CALG_RC5,IDS_ALG_RC5},
	{CALG_SEAL,IDS_ALG_SEAL},
	{CALG_DH_SF,IDS_ALG_DH_SF},
	{CALG_DH_EPHEM,IDS_ALG_DH_EPHEM},
	{CALG_KEA_KEYX,IDS_ALG_KEA_KEYX},
	{CALG_SKIPJACK,IDS_ALG_SKIPJACK},
	{CALG_TEK,IDS_ALG_TEK}
};

typedef struct
{
	LPWSTR lpszListBoxText;
	LPWSTR lpszEditBoxText;
	DWORD dwSpcCtlId;			// special id for item to be placed in ctl.
} ShowCertMapping;

#define LOCAL_STRINGS_MAX_BUFFER 4096

typedef struct
{
	LPWSTR		szEnterAuthInfo,
				szCertInfo,
				szStrengthHigh,
				szStrengthMedium,
				szStrengthLow,
				szCertSubject,
				szCertIssuer,
				szCertEffectiveDate,
				szCertExpirationDate,
				szCertProtocol,
				szCertUsage,
				szHttpsEncryptAlg,
				szHttpsHashAlg,
				szHttpsExchAlg,
				szCertComment,
				szCommentExpires,
				szCommentNotValid,
				szCommentBadCN,
				szCommentBadCA,
				szCommentBadSignature,
				szCommentRevoked,
				szCiphMsg,
				szHashMsg,
				szExchMsg,
				szFingerprint,
				szDomain,
				szRealm,
				szSite,
				szFirewall,
				szPrivateKey,
				szPresent,
				szNotPresent,
				szKeyUsage,
				szCertSerialNumber;
	WCHAR		rgchBuffer[LOCAL_STRINGS_MAX_BUFFER];
} LOCAL_STRINGS, *PLOCAL_STRINGS;

static const UINT uStringId[] = {IDS_LW95_ENTERAUTHINFO,
								IDS_SECERT_CERTINFO,
								IDS_SECERT_STRENGTH_HIGH,
								IDS_SECERT_STRENGTH_MEDIUM,
								IDS_SECERT_STRENGTH_LOW,
								IDS_CERT_SUBJECT,
								IDS_CERT_ISSUER,
								IDS_CERT_EFFECTIVE_DATE,
								IDS_CERT_EXPIRATION_DATE,
								IDS_CERT_PROTOCOL,
								IDS_CERT_USAGE,
								IDS_CERT_ENCRYPT_ALG,
								IDS_CERT_HASH_ALG,
								IDS_CERT_EXCH_ALG,
								IDS_CERT_COMMENT,
								IDS_COMMENT_EXPIRES,
								IDS_COMMENT_NOT_VALID,
								IDS_COMMENT_BAD_CN,
								IDS_COMMENT_BAD_CA,
								IDS_COMMENT_BAD_SIGNATURE,
								IDS_COMMENT_REVOKED,
								IDS_STRING_CIPHMSG,
								IDS_STRING_HASHMSG,
								IDS_STRING_EXCHMSG,
								IDS_CERT_FINGERPRINT,
								IDS_DOMAIN,
								IDS_REALM,
								IDS_SITE,
								IDS_FIREWALL,
								IDS_PRIVATE_KEY,
								IDS_PRESENT,
								IDS_NOT_PRESENT,
								IDS_KEY_USAGE,
								IDS_CERT_SERIALNUMBER};


static WCHAR g_wszFriendlyName[CRYPTUI_MAX_STRING_SIZE] = L"";


PLOCAL_STRINGS FetchLocalStrings(void)
{
	static LOCAL_STRINGS	lszStrings;
	static BOOL				fInitialized = FALSE;

	if(!fInitialized)
	{
		DWORD	dwOffset;
		DWORD	index;
		DWORD	len;
		LPWSTR	szBufEntry;
		LPWSTR	*pszName;

		//	It needs to be initialized.
		pszName = (LPWSTR *)&lszStrings;
		dwOffset = 0;
		for(index = 0;
			index < sizeof(uStringId) / sizeof(UINT);
			index += 1)
		{
			szBufEntry = &lszStrings.rgchBuffer[dwOffset];
			len = LoadStringW(	v_hInst,
								uStringId[index],
								szBufEntry,
								LOCAL_STRINGS_MAX_BUFFER - dwOffset);
			dwOffset += len;
			lszStrings.rgchBuffer[dwOffset++] = 0;
			*pszName++ = szBufEntry;
		}
		//	Make it available to this and future callers.
		fInitialized = TRUE;
	}
	return &lszStrings;
}

int ConvertToHex(PBYTE pbData, DWORD cbData, PWSTR pszBuf, DWORD cchBuf, BOOL fReverse);
BOOL CALLBACK ViewCertDlgProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);
BOOL CertHashToStr(IN LPBYTE lpMD5Hash,IN DWORD  dwMD5HashSize,IN OUT LPWSTR *lplpwszHashStr);
LPWSTR GetCertStatusString(DWORD error,PLOCAL_STRINGS plszStrings);
LPWSTR FTtoString(IN FILETIME *pftTime);
LPWSTR PopulateEKU(PCCERT_CONTEXT pCert);

BOOL ShowCertificate(HWND hParent, PCCERT_CONTEXT pCert)
{
	return ShowCertificateEx(hParent,pCert,NULL);
}

BOOL ShowCertificateEx(HWND hParent, PCCERT_CONTEXT pCert, PNETUI_SECURITY_INFO pNUISecInfo)
{
	BOOL	fRet;
    BOOL	fPortrait = (GetSystemMetrics(SM_CXSCREEN) < 480)? TRUE:FALSE;
    int		idd = fPortrait?IDD_VIEW_CERT_N:IDD_VIEW_CERT_W;

	int		i = 0;
	DWORD	dwProtocolID =   IDS_PROTOCOL_UNKNOWN;
	DWORD	dwHashID =       IDS_HASH_UNKNOWN;
	DWORD	dwCipherID =     IDS_CIPHER_UNKNOWN;
	DWORD	dwExchID =       IDS_EXCH_UNKNOWN;
	DWORD	cbSize;
	WCHAR	wszSerialNumber[CRYPTUI_MAX_STRING_SIZE];
	WCHAR	wszFriendlyName[CRYPTUI_MAX_STRING_SIZE];
	LPWSTR	lpwszSubject = NULL;
	LPWSTR	lpwszIssuer = NULL;
	LPWSTR	lpwszCipher = NULL;
	LPWSTR	lpwszHash = NULL;
	LPWSTR	lpwszExch = NULL;
	LPWSTR	lpwszFrom = NULL;
	LPWSTR	lpwszUntil = NULL;
	LPWSTR	lpwszStatus = NULL;
	LPWSTR	lpwszHashStr = NULL;
	LPWSTR	lpwszKeyUsage = NULL;

	PCERT_INFO		pCertInfo =  NULL;
	PLOCAL_STRINGS	plszStrings;
	ShowCertMapping	MapCertFields[MAX_CERT_FIELDS];

	if(!(fRet=LoadCAPI2()))
		return fRet;

	pCertInfo = pCert->pCertInfo;

	if(pCertInfo == NULL)
		goto quit;

	// Get the Certificate Information.
	plszStrings = FetchLocalStrings();

	if( plszStrings == NULL )
		goto quit;

	lpwszFrom = FTtoString(&pCertInfo->NotBefore);
	lpwszUntil = FTtoString(&pCertInfo->NotAfter);

	wszFriendlyName[0] = '\0';
	pCertGetNameStringW(pCert,CERT_NAME_FRIENDLY_DISPLAY_TYPE,0,NULL,wszFriendlyName,ARRAYSIZE(wszFriendlyName));
	MapCertFields[i].lpszListBoxText = wszFriendlyName;
	MapCertFields[i].lpszEditBoxText = wszFriendlyName;
	MapCertFields[i].dwSpcCtlId      = IDC_CERT_FRIENDLYNAME;
	i++;

	if(pNUISecInfo)
	{
		//	Put a comment string about the certificate if there is one availble.
		//	This is broken.  We never determnine the host name,
		//	so therefore we never show a Comment for bad CA certificates.
		lpwszStatus = GetCertStatusString(pNUISecInfo->dwCertStatus,plszStrings);

		MapCertFields[i].lpszListBoxText = plszStrings->szCertComment;
		MapCertFields[i].dwSpcCtlId      = 0;//IDC_CERT_COMMENT;
		if(!lpwszStatus)
			MapCertFields[i].lpszEditBoxText = plszStrings->szCommentNotValid;
		else
			MapCertFields[i].lpszEditBoxText = lpwszStatus;
		i++;
	}

	if( pCertInfo->Subject.cbData )
	{
		cbSize = pCertNameToStrW(pCert->dwCertEncodingType,
								 &pCertInfo->Subject,
								 CERT_SIMPLE_NAME_STR|CERT_NAME_STR_CRLF_FLAG|CERT_NAME_STR_NO_PLUS_FLAG,
								 NULL,
								 0);

		lpwszSubject = new WCHAR[cbSize];
		if( lpwszSubject )
		{
			pCertNameToStrW(pCert->dwCertEncodingType,
							&pCertInfo->Subject,
							CERT_SIMPLE_NAME_STR|CERT_NAME_STR_CRLF_FLAG|CERT_NAME_STR_NO_PLUS_FLAG ,
							lpwszSubject,
							cbSize);
			MapCertFields[i].lpszListBoxText = plszStrings->szCertSubject;
			MapCertFields[i].lpszEditBoxText = lpwszSubject;
			MapCertFields[i].dwSpcCtlId      = 0;//IDC_CERT_SUBJECT;
			i++;
		}
	}

	if( pCertInfo->Issuer.cbData )
	{
		cbSize = pCertNameToStrW(pCert->dwCertEncodingType,
								 &pCertInfo->Issuer,
								 CERT_SIMPLE_NAME_STR|CERT_NAME_STR_CRLF_FLAG|CERT_NAME_STR_NO_PLUS_FLAG,
								 NULL,
 								 0);

		lpwszIssuer = new WCHAR[cbSize];
		if( lpwszIssuer )
		{
			pCertNameToStrW(pCert->dwCertEncodingType,
							&pCertInfo->Issuer,
							CERT_SIMPLE_NAME_STR|CERT_NAME_STR_CRLF_FLAG|CERT_NAME_STR_NO_PLUS_FLAG,
							lpwszIssuer,
							cbSize);
			MapCertFields[i].lpszListBoxText = plszStrings->szCertIssuer;
			MapCertFields[i].lpszEditBoxText = lpwszIssuer;
			MapCertFields[i].dwSpcCtlId      = 0;//IDC_CERT_ISSUER;
			i++;
		}
	}

	if( lpwszFrom )
	{
		MapCertFields[i].lpszListBoxText = plszStrings->szCertEffectiveDate;
		MapCertFields[i].lpszEditBoxText = lpwszFrom;
		MapCertFields[i].dwSpcCtlId      = 0;
		i++;
	}

	if( lpwszUntil )
	{
		MapCertFields[i].lpszListBoxText = plszStrings->szCertExpirationDate;
		MapCertFields[i].lpszEditBoxText = lpwszUntil;
		MapCertFields[i].dwSpcCtlId      = 0;//IDC_CERT_EXPIRES;
		i++;
	}

	// Get the fingerprint... aka MD5 Hash
	{
		CHAR	lpMD5Hash[TYPICAL_MD5_HASH_SIZE];
		DWORD	dwMD5HashSize = TYPICAL_MD5_HASH_SIZE;
		BOOL	fSuccess;

		fSuccess = pCertGetCertificateContextProperty(	pCert,
														CERT_MD5_HASH_PROP_ID,
														(LPVOID)lpMD5Hash,
														&dwMD5HashSize);

		if( fSuccess )
		{
			CertHashToStr((LPBYTE)lpMD5Hash,dwMD5HashSize,&lpwszHashStr);
			if( lpwszHashStr )
			{
				MapCertFields[i].lpszListBoxText = plszStrings->szFingerprint;
				MapCertFields[i].lpszEditBoxText = lpwszHashStr;
				MapCertFields[i].dwSpcCtlId      = 0;
				i++;
			}
		}
	}

	if(lpwszKeyUsage=PopulateEKU(pCert))
	{
		MapCertFields[i].lpszEditBoxText = lpwszKeyUsage;
		MapCertFields[i].lpszListBoxText = plszStrings->szKeyUsage;
		MapCertFields[i].dwSpcCtlId      = 0;
		i++;
	}
	
	if (!pCertGetCertificateContextProperty(pCert,CERT_KEY_PROV_INFO_PROP_ID,NULL,&cbSize))
	{
		MapCertFields[i].lpszListBoxText = plszStrings->szPrivateKey;
		MapCertFields[i].lpszEditBoxText = plszStrings->szNotPresent;
		MapCertFields[i].dwSpcCtlId      = 0;
		i++;
	}
	else
	{
		MapCertFields[i].lpszListBoxText = plszStrings->szPrivateKey;
		MapCertFields[i].lpszEditBoxText = plszStrings->szPresent;
		MapCertFields[i].dwSpcCtlId      = 0;
		i++;
	}

	ConvertToHex(pCert->pCertInfo->SerialNumber.pbData,
				 pCert->pCertInfo->SerialNumber.cbData,
				 wszSerialNumber,
				 sizeof(wszSerialNumber)/sizeof(wszSerialNumber[0]),
				 TRUE);

	MapCertFields[i].lpszEditBoxText = wszSerialNumber;
	MapCertFields[i].lpszListBoxText = plszStrings->szCertSerialNumber;
	MapCertFields[i].dwSpcCtlId      = 0;
	i++;

	if(pNUISecInfo)
	{
		//	These are in wininet only
		int		j = 0;
		DWORD	adwFormatParams[3];
		WCHAR	szTempBuffer[100];
		WCHAR	lpszProtocol[100];
		//	Now fill in the connection attributes
		if(pNUISecInfo->dwProtocol)
		{
			for(j=0; j<ARRAYSIZE(ProtocolAttrMap); j++)
			{
				if(ProtocolAttrMap[j].dwAttr == pNUISecInfo->dwProtocol)
				{
					dwProtocolID = ProtocolAttrMap[j].dwStringID;
					break;
				}
			}
			if(LoadStringW(	v_hInst,
							dwProtocolID,
							lpszProtocol,
							ARRAYSIZE(lpszProtocol)))
			{

				MapCertFields[i].lpszEditBoxText = lpszProtocol;
				MapCertFields[i].lpszListBoxText = plszStrings->szCertProtocol;
				MapCertFields[i].dwSpcCtlId      = 0;
				i++;
			}
		}

		if(pNUISecInfo->aiCipher)
		{
			for(j=0; j < ARRAYSIZE(AlgAttrMap); j++)
			{
				if(AlgAttrMap[j].dwAttr == pNUISecInfo->aiCipher)
				{
					dwCipherID = AlgAttrMap[j].dwStringID;
					break;
				}
			}
			LoadStringW(v_hInst,
						dwCipherID,
						szTempBuffer,
						ARRAYSIZE(szTempBuffer));
			adwFormatParams[0] = (DWORD)szTempBuffer;
			adwFormatParams[1] = (DWORD)pNUISecInfo->dwCipherStrength;

			if(96 <= pNUISecInfo->dwCipherStrength)	 // Recommended Key strength
				adwFormatParams[2] = (DWORD)plszStrings->szStrengthHigh;
			else if(64 <= pNUISecInfo->dwCipherStrength) // Passable key strength
				adwFormatParams[2] = (DWORD)plszStrings->szStrengthMedium;
			else	  // Ick!  Low key strength.
				adwFormatParams[2] = (DWORD)plszStrings->szStrengthLow;

			if(FormatMessageW(	FORMAT_MESSAGE_ALLOCATE_BUFFER |
								FORMAT_MESSAGE_FROM_STRING |
								FORMAT_MESSAGE_ARGUMENT_ARRAY,
								plszStrings->szCiphMsg,
								0,
								MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
								(LPWSTR)&lpwszCipher,
								0,
								(va_list *)adwFormatParams))
			{
				MapCertFields[i].lpszEditBoxText = lpwszCipher;
				MapCertFields[i].lpszListBoxText = plszStrings->szHttpsEncryptAlg;
				MapCertFields[i].dwSpcCtlId      = 0;

				i++;
			}
		}

		if(pNUISecInfo->aiHash)
		{
			for(j=0; j < ARRAYSIZE(AlgAttrMap); j++)
			{
				if(AlgAttrMap[j].dwAttr == pNUISecInfo->aiHash)
				{
					dwHashID = AlgAttrMap[j].dwStringID;
					break;
				}
			}
			LoadStringW(v_hInst,
						dwHashID,
						szTempBuffer,
						ARRAYSIZE(szTempBuffer));
			adwFormatParams[0] = (DWORD)szTempBuffer;
			adwFormatParams[1] = (DWORD)pNUISecInfo->dwHashStrength;

			if(96 <= pNUISecInfo->dwHashStrength)  // Recommended Key strength
				adwFormatParams[2] = (DWORD)plszStrings->szStrengthHigh;
			else if(64 <= pNUISecInfo->dwHashStrength)	// Passable key strength
				adwFormatParams[2] = (DWORD)plszStrings->szStrengthMedium;
			else	  // Ick!  Low key strength.
				adwFormatParams[2] = (DWORD)plszStrings->szStrengthLow;

			if(FormatMessageW(	FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY,
								plszStrings->szHashMsg,
								0,
								MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
								(LPWSTR)&lpwszHash,
								0,
								(va_list *)adwFormatParams))
			{
				MapCertFields[i].lpszEditBoxText = lpwszHash;
				MapCertFields[i].lpszListBoxText = plszStrings->szHttpsHashAlg;
				MapCertFields[i].dwSpcCtlId      = 0;
				i++;
			}
		}
		if(pNUISecInfo->aiExch)
		{
			for(j=0; j < ARRAYSIZE(AlgAttrMap); j++)
			{
				if(AlgAttrMap[j].dwAttr == pNUISecInfo->aiExch)
				{
					dwExchID = AlgAttrMap[j].dwStringID;
					break;
				}
			}
			LoadStringW(v_hInst,
						dwExchID,
						szTempBuffer,
						ARRAYSIZE(szTempBuffer));
			adwFormatParams[0] = (DWORD)szTempBuffer;
			adwFormatParams[1] = (DWORD)pNUISecInfo->dwExchStrength;

			if(1024 <= pNUISecInfo->dwExchStrength)	 // Recommended Key strength
				adwFormatParams[2] = (DWORD)plszStrings->szStrengthHigh;
			else	  // Ick!  Low key strength.
				adwFormatParams[2] = (DWORD)plszStrings->szStrengthLow;

			if(FormatMessageW(	FORMAT_MESSAGE_ALLOCATE_BUFFER |
								FORMAT_MESSAGE_FROM_STRING |
								FORMAT_MESSAGE_ARGUMENT_ARRAY,
								plszStrings->szExchMsg,
								0,
								MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
								(LPWSTR)&lpwszExch,
								0,
								(va_list *)adwFormatParams))
			{
				MapCertFields[i].lpszEditBoxText = lpwszExch;
				MapCertFields[i].lpszListBoxText = plszStrings->szHttpsExchAlg;
				MapCertFields[i].dwSpcCtlId      = 0;
				i++;
			}
		}
	}
	// Last Array Item is marked with 2 NULLs
	MapCertFields[i].lpszListBoxText = NULL;
	MapCertFields[i].lpszEditBoxText = NULL;
	MapCertFields[i].dwSpcCtlId      = 0;

	// Now Launch the Dlg so we can show it.
	fRet = DialogBoxParam(	v_hInst,
							MAKEINTRESOURCE(idd),
							hParent,
							ViewCertDlgProc,(LONG)MapCertFields);	

	if (fRet)
	{
		// user hit OK, they may have changed the friendly name. If so we need to store it
		if (wcsncmp(wszFriendlyName, g_wszFriendlyName, CRYPTUI_MAX_STRING_SIZE))
		{
			BOOL fResult = TRUE;
			// different, need to store the new name

			CRYPT_DATA_BLOB blob;
			blob.cbData = (wcslen(g_wszFriendlyName)+1)*sizeof(WCHAR);
			blob.pbData = (LPBYTE)g_wszFriendlyName;

			fResult = pCertSetCertificateContextProperty(pCert,CERT_FRIENDLY_NAME_PROP_ID, 0, &blob);

			if (!fResult)
			{
				WCHAR szError[256] = L"";
				WCHAR szErrorTitle[256] = L"";

				ASSERT(!L"CertSet... Friendly name failed\n");


				LoadString(v_hInst, IDS_CERT_FRIENDLY_ERROR, szError, 255);
				LoadString(v_hInst, IDS_CERT_FRIENDLY_ERR_TITLE, szErrorTitle, 255);

				MessageBox(hParent, szError, szErrorTitle, MB_OK | MB_ICONWARNING |MB_APPLMODAL);
			}
		}

		g_wszFriendlyName[0] = NULL;
	}
	
quit:
	FreeCAPI2();
	if(lpwszIssuer)
		delete [] lpwszIssuer;
	if(lpwszSubject)
		delete [] lpwszSubject;
	if(lpwszFrom)
		delete [] lpwszFrom;
	if(lpwszUntil)
		delete [] lpwszUntil;
	if(lpwszCipher)
		delete [] lpwszCipher;
	if(lpwszHash)
		delete [] lpwszHash;
	if(lpwszExch)
		delete [] lpwszExch;
	if(lpwszHashStr)
		delete [] lpwszHashStr;
	if(lpwszKeyUsage)
		delete [] lpwszKeyUsage;
	
	return fRet;
}

//	List the enhanced Key Usages
//	Use CRYPT32 to translate the OIDs to (localized) descriptions
//	If there is no enhanced Key Usage field then print "All Purposes"
LPWSTR PopulateEKU(PCCERT_CONTEXT pCert)
{
    int					cch = CRYPTUI_MAX_STRING_SIZE;
    UINT				i;
    BOOL				fAllPurposes = FALSE;
    DWORD				cbUsage;
    PWCHAR				pszBuf = NULL;
    PWCHAR				pch;
    PCERT_ENHKEY_USAGE	pUsage;
    PCCRYPT_OID_INFO	pOidInfo;

	if (!pCertGetEnhancedKeyUsage(pCert, 0, NULL, &cbUsage))
        return FALSE;

    pUsage = (PCERT_ENHKEY_USAGE)LocalAlloc(0, cbUsage);
    if (!pUsage)
        return FALSE;
    
    if (pCertGetEnhancedKeyUsage(pCert, 0, pUsage, &cbUsage))
    {
    	 if (pUsage->cUsageIdentifier == 0 && (GetLastError() == CRYPT_E_NOT_FOUND))
    	 	fAllPurposes = TRUE;
        for (i=0, cch = 1; i < pUsage->cUsageIdentifier; i++)
        {
            pOidInfo = pCryptFindOIDInfo(CRYPT_OID_INFO_OID_KEY,
										 pUsage->rgpszUsageIdentifier[i],
										 CRYPT_ENHKEY_USAGE_OID_GROUP_ID);
			if (pOidInfo)
				cch += wcslen(pOidInfo->pwszName)+2;	// 2 for \r\n
            else
           		cch += strlen(pUsage->rgpszUsageIdentifier[i])+2;
        }
	}
	if (fAllPurposes)
		cch = CRYPTUI_MAX_STRING_SIZE;

	pch = (pszBuf = new WCHAR[cch]);
    if (pszBuf)
	{
		*pszBuf = '\0';
		if (fAllPurposes)
			LoadStringW(v_hInst, IDS_ALL_PURPOSES,pszBuf, cch);
        else
		{
			for (i=0; i < pUsage->cUsageIdentifier; i++)
	        {
				pOidInfo = pCryptFindOIDInfo(CRYPT_OID_INFO_OID_KEY,
											 pUsage->rgpszUsageIdentifier[i],
											 CRYPT_ENHKEY_USAGE_OID_GROUP_ID);
				if (pOidInfo)
					wcscpy(pch, pOidInfo->pwszName);
				else
				{
					// couldn't find a descriptive name for the OID.
					// display the OID itself
					mbstowcs(pch,pUsage->rgpszUsageIdentifier[i],cch-(pch-pszBuf));
				}
				pch += wcslen(pch);
				wcscpy(pch, L"\r\n");
				pch += 2;
	        }
	        DEBUGCHK(pch-pszBuf < cch);
         }
    }
    LocalFree(pUsage);
    return pszBuf;
}

static PCWSTR hexTable = L"0123456789ABCDEF";
int ConvertToHex(PBYTE pbData, DWORD cbData, PWSTR pszBuf, DWORD cchBuf, BOOL fReverse)
{
	int		step;
	PBYTE	pb;
	DWORD	cch;

	if (fReverse) 
	{
		pb = pbData+cbData-1;
		step = -1;
	}
	else
	{
		pb = pbData;
		step = 1;
	}

	for(cch=0;cbData>0 && (cchBuf-cch)>=3; pb+=step, cch+=3, --cbData )
	{
		pszBuf[cch] = hexTable[(*pb >> 4) & 0xF];
		pszBuf[cch + 1] = hexTable[(*pb) & 0xF];
		pszBuf[cch + 2] = ' ';
	}
	if (cch)
		pszBuf[cch] = '\0';
	return cch;
}

BOOL PlaceCertificateDataIntoListBox(IN HWND hWndDlg,
									 IN HWND hWndListBox,
									 IN ShowCertMapping *pMapCertFields)
{
	DWORD i;

	for( i = 0; pMapCertFields[i].lpszListBoxText != NULL; i++ )
	{
		if( pMapCertFields[i].dwSpcCtlId != 0 )
		{
			SetDlgItemTextW(hWndDlg,
							pMapCertFields[i].dwSpcCtlId,
							pMapCertFields[i].lpszEditBoxText );
			SetWindowLongW(	GetDlgItem(hWndDlg,pMapCertFields[i].dwSpcCtlId),
							GWL_STYLE,ES_READONLY |
								GetWindowLongW(GetDlgItem(hWndDlg,pMapCertFields[i].dwSpcCtlId),GWL_STYLE));
		}
		else
			SendMessageW(hWndListBox,LB_ADDSTRING,0,(LPARAM)pMapCertFields[i].lpszListBoxText);
	}
	SendMessageW(hWndListBox,LB_SETCURSEL,0,0 );
	return TRUE;
}

DWORD OnSelectionOfACertField(IN HWND hWndListBox,
							  IN HWND hWndEditBox,
							  IN ShowCertMapping *pMapCertFields)
{
	LRESULT index;

	index = SendMessageW(hWndListBox,LB_GETCURSEL,0,0)+1;
	if(index == LB_ERR )
		index = 0;

	if( pMapCertFields[index].lpszListBoxText != NULL )
	{
		SetWindowTextW( hWndEditBox,
						pMapCertFields[index].lpszEditBoxText );
		SetWindowLongW(	hWndEditBox,
						GWL_STYLE,ES_READONLY |GetWindowLongW(hWndEditBox,GWL_STYLE));
	}
	return ERROR_SUCCESS;
}

/*
Routine Description:
	 Shows a Certificate,and relevent security information to the user.

Arguments:
	 hwnd    - standard dialog params
	 msg     - "
	 wparam  - "
	 lparam  - "

Return Value:
	 BOOL
		  TRUE    - we handled message
		  FALSE   - Windows should handle message
*/
BOOL CALLBACK ViewCertDlgProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	ShowCertMapping		*pMapCertFields;

	switch(msg)
	{
	case WM_INITDIALOG:
		pMapCertFields = (ShowCertMapping*)lparam;
		SetWindowLongW(hwnd,DWL_USER,lparam);

		PlaceCertificateDataIntoListBox(hwnd,
										GetDlgItem(hwnd,IDC_FIELDLIST),
										pMapCertFields);

		OnSelectionOfACertField(GetDlgItem(hwnd,IDC_FIELDLIST),
								GetDlgItem(hwnd,IDC_DETAILSLIST),
								pMapCertFields);
		return TRUE;

	case WM_HELP:                                                                       
		return TRUE;

	case WM_COMMAND:
		{
			WORD	wID = LOWORD(wparam);
			WORD	wNotificationCode = HIWORD(wparam);
			HWND	hWndCtrl = (HWND) lparam;

			pMapCertFields = (ShowCertMapping*)GetWindowLongW(hwnd,DWL_USER);

			switch(wID)
			{
			case IDC_FIELDLIST:
				//	If the user changes the selection of the listbox
				//  move the edit control field data to the correct entry.
				if( wNotificationCode == LBN_SELCHANGE )
				{
					OnSelectionOfACertField(hWndCtrl,
											GetDlgItem(hwnd,IDC_DETAILSLIST),
											(ShowCertMapping*)pMapCertFields);
				}
				break;

			case IDOK:
			case IDYES:
				{
					GetDlgItemText(hwnd, IDC_CERT_FRIENDLYNAME, g_wszFriendlyName, CRYPTUI_MAX_STRING_SIZE);

					EndDialog(hwnd,TRUE);
				}break;

			case IDCANCEL:
			case IDNO:
				EndDialog(hwnd,FALSE);
				break;
			}
			return TRUE;
		}
	}
	return FALSE;
}

LPWSTR GetCertStatusString(DWORD error,PLOCAL_STRINGS plszStrings)
{
	// If we are unable to verify revocation,then ignore.
	if(error&CERT_TRUST_REVOCATION_STATUS_UNKNOWN)
		return plszStrings->szCommentNotValid;

	if(	error&CERT_TRUST_IS_NOT_TIME_VALID		|| 
		error&CERT_TRUST_IS_NOT_TIME_NESTED		||
		error&CERT_TRUST_CTL_IS_NOT_TIME_VALID)
	{
		return plszStrings->szCommentExpires;
	}

	if(	error&CERT_TRUST_IS_UNTRUSTED_ROOT			||
		error&CERT_TRUST_INVALID_BASIC_CONSTRAINTS	||
		error&CERT_TRUST_IS_PARTIAL_CHAIN)
	{
		return plszStrings->szCommentBadCA;
	}

	if(	error&CERT_TRUST_IS_REVOKED)
		return plszStrings->szCommentRevoked;

	if(	error&CERT_TRUST_IS_REVOKED)
		return plszStrings->szCommentRevoked;

	if(	error &CERT_TRUST_IS_NOT_VALID_FOR_USAGE)
		return plszStrings->szCertUsage;


	if( error&CERT_TRUST_IS_NOT_SIGNATURE_VALID		||
		error&CERT_TRUST_IS_NOT_VALID_FOR_USAGE		||
		error&CERT_TRUST_IS_CYCLIC					||
		error&CERT_TRUST_CTL_IS_NOT_SIGNATURE_VALID	||
		error&CERT_TRUST_CTL_IS_NOT_VALID_FOR_USAGE)
	{
		return plszStrings->szCommentNotValid;
	}

	if(	error==CERT_TRUST_NO_ERROR )
		return NULL;

	return plszStrings->szCommentNotValid;
}

/*
Routine Description:
	Converts a set of bytes into a neatly formated string of ':' (colon) seperated
	hex digits that can be shown to the user.

Arguments:
	lpMD5Hash	-	ptr to set of hash bytes
	dwMD5HashSize - size of lpMD5Hash
	lplpwszHashStr - ptr to ptr where newly allocated return string will be stored.

Return Value:
	BOOL
*/
BOOL CertHashToStr(IN LPBYTE lpMD5Hash,
				   IN DWORD  dwMD5HashSize,
				   IN OUT LPWSTR *lplpwszHashStr)
{

	DWORD	dwStrSize = (2*dwMD5HashSize)+dwMD5HashSize;
	LPWSTR	lpwszHashStr;

	*lplpwszHashStr = new WCHAR[dwStrSize];
	if( *lplpwszHashStr == NULL )
		return FALSE;
	lpwszHashStr = *lplpwszHashStr;

	unsigned char uHashByte;
	for( DWORD i = 0 ; i < dwMD5HashSize; i++ )
	{
		if( i != 0 )
		{
			*lpwszHashStr = L':';
			lpwszHashStr++;
		}
		uHashByte = (unsigned char)*(((unsigned char*)lpMD5Hash)+i);
		wsprintfW(lpwszHashStr,L"%02X",uHashByte);
		lpwszHashStr += 2;
	}
	return TRUE;
}

/*
FTtoString:
	This routine converts a given FILETIME structure to a string representing
	the given date and time in the local format.

Arguments:
	pftTime supplies the FILETIME structure to convert.

Return Value:
	NULL - Memory allocation failure.
		Otherwise, the address of the string, allocated via LocalAlloc.

Author:
	 Doug Barlow (dbarlow) 4/12/1996
*/
LPWSTR FTtoString(IN FILETIME *pftTime)
{
	LONG		cchTotal;
	LONG		cchNeeded;
	LPWSTR		szDateTime = NULL;
	SYSTEMTIME	stTime, stLocal;

	// Convert the FILETIME to a SYSTEMTIME.
	if(!FileTimeToSystemTime(pftTime, &stTime))
		goto ErrorExit;

	// For now, leave it in GMT time, function not implimented in Win'95.
	stLocal = stTime;

	// Calculate how long the date string will be.
	cchTotal = GetDateFormat(	LOCALE_SYSTEM_DEFAULT,
								DATE_SHORTDATE,
								&stLocal,
								NULL,
								NULL,
								0);
	if(0 >= cchTotal)
		goto ErrorExit;
	cchNeeded =	GetTimeFormat(	LOCALE_SYSTEM_DEFAULT,
								0,
								&stLocal,
								NULL,
								NULL,
								0);
	if(0 >= cchNeeded)
		goto ErrorExit;
	cchTotal += cchNeeded;
	cchTotal += 4*sizeof(WCHAR);  // space, trailing NULL, and two extra.
	szDateTime = new WCHAR[cchTotal];
	if(NULL == szDateTime)
		goto ErrorExit;

	// Fill in the time string.
	cchNeeded = GetDateFormat(	LOCALE_SYSTEM_DEFAULT,
								DATE_SHORTDATE,
								&stLocal,
								NULL,
								szDateTime,
								cchTotal);
	if(0 >= cchNeeded)
		goto ErrorExit;
	wcscat(szDateTime,L" ");
	cchNeeded = wcslen(szDateTime);
	cchNeeded =	GetTimeFormat(	LOCALE_SYSTEM_DEFAULT,
								0,
								&stLocal,
								NULL,
								&szDateTime[cchNeeded],
								cchTotal - cchNeeded);
	if(0 >= cchNeeded)
		goto ErrorExit;
	return szDateTime;

ErrorExit:
	if(NULL != szDateTime)
		delete [] szDateTime;
	return NULL;
}
