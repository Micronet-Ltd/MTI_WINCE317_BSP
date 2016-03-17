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
typedef
BOOL (WINAPI *PFNCRYPTENUMPROVIDERSW)(
    DWORD   dwIndex,
    DWORD * pdwReserved,
    DWORD   dwFlags,
    DWORD * pdwProvType,
    LPWSTR pszProvName,
    DWORD * pcbProvName
    );

typedef
BOOL (WINAPI *PFNCRYPTGETPROVPARAM)
(
    HCRYPTPROV hProv,
    DWORD dwParam,
    BYTE*pbData,
    DWORD *pdwDataLen,
    DWORD dwFlags
    );
typedef  BOOL (WINAPI *PFNCRYPTACQUIRECONTEXTW)(
    HCRYPTPROV *phProv,
    LPCWSTR pszContainer,
    LPCWSTR pszProvider,
    DWORD dwProvType,
    DWORD dwFlags
    );
typedef BOOL (WINAPI *PFNCRYPTRELEASECONTEXT)(
    HCRYPTPROV hProv,
    ULONG_PTR dwFlags
    );

typedef
BOOL (WINAPI *PFNCRYPTDESTROYKEY)(
    HCRYPTKEY hKey
    );

typedef
BOOL (WINAPI *PFNCRYPTGETUSERKEY)(
    HCRYPTPROV hProv,
    DWORD dwKeySpec,
    HCRYPTKEY *phUserKey
    );

typedef BOOL (WINAPI *PFNCRYPTGETKEYPARAM)(
    HCRYPTKEY hKey,
    DWORD dwParam,
    BYTE *pbData,
    DWORD *pdwDataLen,
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
PCCERT_CONTEXT (WINAPI *PFNCERTDUPLICATECERTIFICATECONTEXT)(
    IN PCCERT_CONTEXT pCertContext
    );

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
PCCERT_CONTEXT  (WINAPI *PFNCERTENUMCERTIFICATESINSTORE)(
    IN HCERTSTORE hCertStore,
    IN PCCERT_CONTEXT pPrevCertContext
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


extern PFNCRYPTENUMPROVIDERSW pCryptEnumProvidersW;
extern PFNCRYPTACQUIRECONTEXTW pCryptAcquireContextW;
extern PFNCRYPTRELEASECONTEXT pCryptReleaseContext;
extern PFNCRYPTGETUSERKEY pCryptGetUserKey;
extern PFNCRYPTGETKEYPARAM pCryptGetKeyParam;
extern PFNCRYPTDESTROYKEY pCryptDestroyKey;
extern PFNCRYPTGETPROVPARAM pCryptGetProvParam;
extern PFNCERTCREATECERTIFICATECONTEXT pCertCreateCertificateContext;
extern PFNCERTFREECERTIFICATECONTEXT pCertFreeCertificateContext;
extern PFNCERTDUPLICATECERTIFICATECONTEXT pCertDuplicateCertificateContext;
extern PFNCERTOPENSTORE pCertOpenStore;
extern PFNCERTCLOSESTORE pCertCloseStore;
extern PFNCERTENUMCERTIFICATESINSTORE pCertEnumCertificatesInStore;
extern PFNCERTFINDCERTIFICATEINSTORE pCertFindCertificateInStore;
extern PFNCERTGETNAMESTRINGW pCertGetNameStringW;
extern PFNCERTGETCERTIFICATECONTEXTPROPERTY pCertGetCertificateContextProperty;
extern PFNCERTSETCERTIFICATECONTEXTPROPERTY pCertSetCertificateContextProperty;
extern PFNCERTADDCERTIFICATECONTEXTTOSTORE pCertAddCertificateContextToStore;
extern PFNCERTADDENCODEDCERTIFICATETOSTORE pCertAddEncodedCertificateToStore;
extern PFNCERTCOMPARECERTIFICATENAME pCertCompareCertificateName;
extern PFNCERTNAMETOSTRW pCertNameToStrW;
extern PFNCERTGETENHANCEDKEYUSAGE pCertGetEnhancedKeyUsage;
extern PFNCRYPTFINDOIDINFO pCryptFindOIDInfo;

extern BOOL LoadCAPI2(void);
extern void FreeCAPI2(void);



