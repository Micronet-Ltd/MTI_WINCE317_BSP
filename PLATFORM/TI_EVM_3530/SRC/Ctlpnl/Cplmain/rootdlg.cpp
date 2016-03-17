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

Abstract: This code exports the Root Certificate store warning dialog. CAPI2 will bring up this
message box if a certificate is imported or deleted from the root store.
The following entries need to be added to the registry for CAPI2 to invoke the Dialog function
**/
#include <cplpch.h>
#include <wincrypt.h>

extern HINSTANCE	g_hInst;
extern BOOL LoadCAPI2();
extern void FreeCAPI2();
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

BOOL CALLBACK CertificatePromptDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LPWSTR g_pwszTitle	= NULL;
static LPWSTR g_pwszBody	= NULL;


extern PFNCERTCOMPARECERTIFICATENAME pCertCompareCertificateName;
extern PFNCERTNAMETOSTRW pCertNameToStrW;

//+=========================================================================
//  FormatMsgBox support functions
//==========================================================================

//+-------------------------------------------------------------------------
//  Formats multi bytes into WCHAR hex. Includes a space after every 4 bytes.
//
//  Needs (cb * 2 + cb/4 + 1) characters in wsz
//--------------------------------------------------------------------------
static void FormatMsgBoxMultiBytes(DWORD cb, BYTE *pb, LPWSTR wsz)
{
    for (DWORD i = 0; i<cb; i++) {
        int b;
        if (i && 0 == (i & 3))
            *wsz++ = L' ';
        b = (*pb & 0xF0) >> 4;
        *wsz++ = (b <= 9) ? b + L'0' : (b - 10) + L'A';
        b = *pb & 0x0F;
        *wsz++ = (b <= 9) ? b + L'0' : (b - 10) + L'A';
        pb++;
    }
    *wsz++ = 0;
}

//+-------------------------------------------------------------------------
//  Format and allocate a single message box item
//
//  The formatted item needs to be LocalFree'ed.
//--------------------------------------------------------------------------
static void FormatMsgBoxItem(
    OUT LPWSTR *ppwszMsg,
    OUT DWORD *pcchMsg,
    IN UINT nFormatID,
    ...
    )
{
    // get format string from resources
    WCHAR wszFormat[256];
    wszFormat[0] = '\0';
    LoadStringW(g_hInst, nFormatID, wszFormat,
        sizeof(wszFormat)/sizeof(wszFormat[0]));

    // format message into requested buffer
    va_list argList;
    va_start(argList, nFormatID);
    *ppwszMsg = NULL;
    *pcchMsg = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
        wszFormat,
        0,                  // dwMessageId
        0,                  // dwLanguageId
        (LPWSTR) ppwszMsg,
        0,                  // minimum size to allocate
        &argList);

    va_end(argList);
}


//+-------------------------------------------------------------------------
//  The add/delete root message box.
//
//--------------------------------------------------------------------------
extern "C"
int
RootStorePrompt(
    IN PCCERT_CONTEXT pCert, // cert being added or deleted
    IN UINT wActionID,	// 0 -> Add, 1 -> Delete
    IN UINT uFlags
    )
{
    int id = IDNO;

    LPWSTR pwszTmp;       // _alloca'ed
    DWORD cchTmp;

// Includes the title
#define MAX_ROOT_BOX_ITEMS 10
    struct {
        LPWSTR  pwszItem;
        DWORD   cchItem;
    } rgItem[MAX_ROOT_BOX_ITEMS];
    DWORD cItem = 0;
    LPWSTR pwszText = NULL;
    DWORD cchText = 0;

    if(!LoadCAPI2())
    	goto end;
    	
    // SUBJECT
    cchTmp = pCertNameToStrW(
            pCert->dwCertEncodingType,
            &pCert->pCertInfo->Subject,
            CERT_SIMPLE_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
            NULL,                   // pwsz
            0);                     // cwsz
    __try {
        pwszTmp = (LPWSTR) _alloca(cchTmp * sizeof(WCHAR));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        pwszTmp = NULL;
    }
    if (NULL == pwszTmp) {
        ASSERT(0);
        pwszTmp = L"";
    } else
        pCertNameToStrW(
            pCert->dwCertEncodingType,
            &pCert->pCertInfo->Subject,
            CERT_SIMPLE_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
            pwszTmp,
            cchTmp);
    FormatMsgBoxItem(&rgItem[cItem].pwszItem, &rgItem[cItem].cchItem,
        IDS_ROOT_MSG_BOX_SUBJECT, pwszTmp);
    cchText += rgItem[cItem].cchItem;
    cItem++;

    // ISSUER. May be self issued
    if (pCertCompareCertificateName(
            pCert->dwCertEncodingType,
            &pCert->pCertInfo->Subject,
            &pCert->pCertInfo->Issuer
            ))
        // Self issued
        FormatMsgBoxItem(&rgItem[cItem].pwszItem, &rgItem[cItem].cchItem,
            IDS_ROOT_MSG_BOX_SELF_ISSUED);
    else {
        // Format certificate's issuer
        cchTmp = pCertNameToStrW(
                pCert->dwCertEncodingType,
                &pCert->pCertInfo->Issuer,
                CERT_SIMPLE_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
                NULL,                   // pwsz
                0);                     // cwsz
        __try {
            pwszTmp = (LPWSTR) _alloca(cchTmp * sizeof(WCHAR));
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            pwszTmp = NULL;
        }
        if (NULL == pwszTmp) {
            ASSERT(0);
            pwszTmp = L"";
        } else
            pCertNameToStrW(
                pCert->dwCertEncodingType,
                &pCert->pCertInfo->Issuer,
                CERT_SIMPLE_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
                pwszTmp,
                cchTmp);
        FormatMsgBoxItem(&rgItem[cItem].pwszItem, &rgItem[cItem].cchItem,
            IDS_ROOT_MSG_BOX_ISSUER, pwszTmp);
    }
    cchText += rgItem[cItem].cchItem;
    cItem++;

    // TIME VALIDITY
    {
        FILETIME ftLocal;
        SYSTEMTIME stLocal;
        WCHAR wszNotBefore[128];
        WCHAR wszNotAfter[128];
        wszNotBefore[0] = '\0';
        wszNotAfter[0] = '\0';

        FileTimeToLocalFileTime(&pCert->pCertInfo->NotBefore, &ftLocal);
        FileTimeToSystemTime(&ftLocal, &stLocal);
        GetDateFormatW(LOCALE_USER_DEFAULT, DATE_LONGDATE, &stLocal,
            NULL, wszNotBefore, 128);
        FileTimeToLocalFileTime(&pCert->pCertInfo->NotAfter, &ftLocal);
        FileTimeToSystemTime(&ftLocal, &stLocal);
        GetDateFormatW(LOCALE_USER_DEFAULT, DATE_LONGDATE, &stLocal,
            NULL, wszNotAfter, 128);

        FormatMsgBoxItem(&rgItem[cItem].pwszItem,
            &rgItem[cItem].cchItem, IDS_ROOT_MSG_BOX_TIME_VALIDITY,
            wszNotBefore, wszNotAfter);
        cchText += rgItem[cItem].cchItem;
        cItem++;
    }

    // SERIAL NUMBER
    if (pCert->pCertInfo->SerialNumber.cbData) {
        DWORD cb = pCert->pCertInfo->SerialNumber.cbData;
        BYTE *pb, *pbDst, *pbSrc;
        if (pb = (PBYTE) LocalAlloc(0,cb))
	    {
            LPWSTR pwsz;
            // reverse bytes
    		for (pbSrc = pCert->pCertInfo->SerialNumber.pbData, pbDst = pb + cb - 1; cb > 0; cb--)
        		*pbDst-- = *pbSrc++;
        	cb = pCert->pCertInfo->SerialNumber.cbData;
        	
            if (pwsz = (LPWSTR) LocalAlloc( 0,
                    (cb*2 + cb/4 + 1) * sizeof(WCHAR))) {
                FormatMsgBoxMultiBytes(cb, pb, pwsz);
                FormatMsgBoxItem(&rgItem[cItem].pwszItem,
                    &rgItem[cItem].cchItem, IDS_ROOT_MSG_BOX_SERIAL_NUMBER,
                    pwsz);
                cchText += rgItem[cItem].cchItem;
                cItem++;
                LocalFree(pwsz);
            }
            LocalFree(pb);
        }
    }

#if 0
    // THUMBPRINTS: sha1 and md5
    {
        BYTE    rgbHash[MAX_HASH_LEN];
        DWORD   cbHash = MAX_HASH_LEN;

        // get the sha1 thumbprint
        if (CertGetCertificateContextProperty(
                pCert,
                CERT_SHA1_HASH_PROP_ID,
                rgbHash,
                &cbHash)) {
            FormatMsgBoxMultiBytes(cbHash, rgbHash, wszTmp);
            FormatMsgBoxItem(&rgItem[cItem].pwszItem,
                &rgItem[cItem].cchItem, IDS_ROOT_MSG_BOX_SHA1_THUMBPRINT,
                wszTmp);
            cchText += rgItem[cItem].cchItem;
            cItem++;
        }

        // get the md5 thumbprint
        if (CertGetCertificateContextProperty(
                pCert,
                CERT_MD5_HASH_PROP_ID,
                rgbHash,
                &cbHash)) {
            FormatMsgBoxMultiBytes(cbHash, rgbHash, wszTmp);
            FormatMsgBoxItem(&rgItem[cItem].pwszItem,
                &rgItem[cItem].cchItem, IDS_ROOT_MSG_BOX_MD5_THUMBPRINT,
                wszTmp);
            cchText += rgItem[cItem].cchItem;
            cItem++;
        }
    }
#endif

    // Concatenate all the items into a single allocated string
    ASSERT(cchText);
    if (NULL != (pwszText = (LPWSTR) LocalAlloc( 0,
            (cchText + 1) * sizeof(WCHAR)))) {
        LPWSTR pwsz = pwszText;
        DWORD ItemIdx;
        for (ItemIdx = 0; ItemIdx < cItem; ItemIdx++) {
            DWORD cch = rgItem[ItemIdx].cchItem;
            if (cch) {
                ASSERT(rgItem[ItemIdx].pwszItem);
                memcpy(pwsz, rgItem[ItemIdx].pwszItem, cch * sizeof(WCHAR));
                pwsz += cch;
            }
        }
        ASSERT (pwsz == pwszText + cchText);
        *pwsz = '\0';
		g_pwszBody = pwszText;

        FormatMsgBoxItem(&rgItem[cItem].pwszItem, &rgItem[cItem].cchItem,
            wActionID == 0 ? IDS_ROOT_MSG_BOX_ADD_ACTION : IDS_ROOT_MSG_BOX_DELETE_ACTION, NULL);
		
		g_pwszTitle = rgItem[cItem].pwszItem;

		cItem++;

		id = DialogBox(g_hInst, MAKEINTRESOURCE(IDD_CERTIFICATE_PROMPT), NULL, CertificatePromptDialogProc);

        LocalFree(pwszText);
    } else
        id = IDNO;

    // Free up all the individually allocated items
    while (cItem--) {
        if (rgItem[cItem].pwszItem)
            LocalFree((HLOCAL) rgItem[cItem].pwszItem);
    }
end:
    FreeCAPI2();
    return id;
}

BOOL CALLBACK CertificatePromptDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
				SetDlgItemTextW(hwndDlg, IDC_CertificateMessage, g_pwszTitle);

				SetDlgItemTextW(hwndDlg, IDC_CertificateText, g_pwszBody);

				return TRUE;
			}break;

		case WM_COMMAND:
            {
			    int nResult = LOWORD(wParam);
			    switch (nResult) {
				    case IDYES:
					    {
							EndDialog(hwndDlg, nResult);
							return TRUE;
					    }break;
				    
					case IDCANCEL:
					case IDNO:
						{
							nResult = IDNO;
							EndDialog(hwndDlg, nResult);
							return TRUE;
						}break;
			    }
            }break;

	}
	return FALSE;
}
