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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

Wzcprops.c

Abstract:  

    Code for Wireless Zero Config.

Functions:


Notes: 


--*/
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "string.h"
#include "memory.h"
#include "commctrl.h"
#include "resource.h"
#include "netui.h"
#include "netuip.h"
#include "wzcuip.h"


WCHAR g_wszHiddWepK[]=L"**************************";
CHAR  g_szHiddWepK[] = "**************************";

#define MAX_CB_STRING	16

// defines legal lengths for the WEP Key material
#define WZC_WEPKMAT_40_ASC  5
#define WZC_WEPKMAT_40_HEX  10
#define WZC_WEPKMAT_104_ASC 13
#define WZC_WEPKMAT_104_HEX 26
#define WZC_WEPKMAT_128_ASC 16
#define WZC_WEPKMAT_128_HEX 32
#define WZC_WEPKMAT_256_ASC 32
#define WZC_WEPKMAT_256_HEX 64

WCHAR 	g_wszCbShared[MAX_CB_STRING];
WCHAR 	g_wszCbOpen[MAX_CB_STRING];
WCHAR   g_wszCbWPA2PSK[MAX_CB_STRING];
WCHAR   g_wszCbWPA2[MAX_CB_STRING];
WCHAR 	g_wszCbSSNPSK[MAX_CB_STRING];
WCHAR 	g_wszCbSSN[MAX_CB_STRING];
WCHAR 	g_wszCbDisabled[MAX_CB_STRING];
WCHAR 	g_wszCbWEP[MAX_CB_STRING];
WCHAR 	g_wszCbTKIP[MAX_CB_STRING];
WCHAR	g_wszCbAES[MAX_CB_STRING];


DWORD CopyWepKMaterial(WLAN_CFG_PROPS *pWCP, LPSTR szKMat, BOOL bJustEncypt);


void 
InitializeCfgProps(WLAN_CFG_PROPS *pWCP, DWORD dwFlags, UINT nDlgId)
{
    memset(pWCP,0,sizeof(WLAN_CFG_PROPS));
    pWCP->fDlgOk = FALSE;
    pWCP->dwFlags = dwFlags;
    pWCP->IDD = nDlgId;
    pWCP->bKMatTouched = TRUE;

    pWCP->wzcConfig.Length = sizeof(WZC_WLAN_CONFIG);
    pWCP->wzcConfig.InfrastructureMode = Ndis802_11Infrastructure;
    pWCP->wzcConfig.EapolParams.dwEapType  = DEFAULT_EAP_TYPE;
    pWCP->wzcConfig.EapolParams.dwEapFlags = DEFAULT_EAP_STATE;
    pWCP->pConfigList = NULL;
}

//+---------------------------------------------------------------------------
// Checks whether the current configuration exists already in the provided list
BOOL
AlreadyExists(WLAN_CFG_PROPS *pWCP)
{
    BOOL bExists = FALSE;

    if (pWCP->pConfigList != NULL)
    {
        WLAN_CONFIG_LISTENT *pCrt = pWCP->pConfigList;

        do
        {
            if (ConfigMatch(&pCrt->wzcConfig,&pWCP->wzcConfig))
                bExists = TRUE;
            pCrt = pCrt->pNext;
        } while(!bExists && pCrt != pWCP->pConfigList);
    }

    return bExists;
}

void
WLanPropsErrorMessageBox(HWND hDlg, UINT nFmtStrId, WCHAR *szStr1, WCHAR *szStr2)
{
    WCHAR szWarnStr[MAX_WZC_UI_STR], szFmtStr[MAX_WZC_UI_STR],szTitle[MAX_WZC_UI_STR];
    DWORD dwStrLen;
    dwStrLen = LoadString(v_hInst,nFmtStrId,szFmtStr,ARRAYSIZE(szFmtStr));
    if (dwStrLen) {
        dwStrLen = LoadString(v_hInst,IDS_WZC_ERROR_CAPTION,szTitle,ARRAYSIZE(szTitle));
        if (dwStrLen) {
            DWORD adwArgList[2];

            adwArgList[0] = (DWORD)szStr1;
            adwArgList[1] = (DWORD)szStr2;

            if (!wcscmp(szStr1, L"") && !wcscmp(szStr2, L"")) {
            	MessageBox(hDlg, szFmtStr, szTitle, MB_ICONSTOP|MB_OK);
            }
            else if (FormatMessage (FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
		                               (LPCVOID)szFmtStr,
		                               0,
		                               0,
		                               szWarnStr,
		                               ARRAYSIZE(szWarnStr),
		                               (va_list *)adwArgList)) {
                MessageBox(hDlg,szWarnStr, szTitle, MB_ICONSTOP|MB_OK);
            }
        }
    }
}

typedef struct _WZC_EAP_CB_ITEMDATA
{
    PEAP_EXTENSION_INFO pEapExtInfo;
    DWORD dwAuthDataLen;
    BYTE  *pbAuthData;

} WZC_EAP_CB_ITEMDATA;

DWORD 
Init8021xControls(HWND hDlg, WLAN_CFG_PROPS *pWCP)
{
    DWORD dwError;
    int i;
    BOOL bCurSelSet=FALSE;

	// If access point supports 802.1x
	if(pWCP->wzcConfig.dwCtlFlags & WZCCTL_ONEX_ENABLED)
	{
		SendMessage(pWCP->hwndChkEnable8021x, BM_SETCHECK, BST_CHECKED, 0);
		EnableWindow(pWCP->hwndChkEnable8021x, FALSE);
	}
	else
	{
	    SendMessage(pWCP->hwndChkEnable8021x, BM_SETCHECK, 
	                (pWCP->wzcConfig.EapolParams.bEnable8021x) ? BST_CHECKED : BST_UNCHECKED, 0);
	}
	
    if (pWCP->pEapExtInfo == NULL) {
        // Load EAP extension info
        dwError = pfnWZCEnumEapExtensions(&pWCP->dwNumEapExt,&pWCP->pEapExtInfo);
        if (dwError != ERROR_SUCCESS) {
            DEBUGMSG(ZONE_ERROR,(TEXT("!WZCUI:Error 0x%X loading EAP extensions\r\n"),dwError));
            return dwError;
        }                  

        if (pWCP->dwNumEapExt > 0 && pWCP->pEapExtInfo) {
            PEAP_EXTENSION_INFO pCurExt;
            WCHAR *wszDefaultFriendlyName=NULL;
            // Add EAP type to combo box
            for (i=0,pCurExt = pWCP->pEapExtInfo;
                 i<(int)pWCP->dwNumEapExt;
                 i++,pCurExt++) {
                int iListIndex;
                WZC_EAP_CB_ITEMDATA *pEapCbItemData = (WZC_EAP_CB_ITEMDATA *)LocalAlloc(LPTR,sizeof(WZC_EAP_CB_ITEMDATA));
                if (pEapCbItemData) {
                    iListIndex = ComboBox_AddString(pWCP->hwndCbEAPType,pCurExt->wszFriendlyName);
                    pEapCbItemData->pEapExtInfo = pCurExt;
                    ComboBox_SetItemData(pWCP->hwndCbEAPType,iListIndex,pEapCbItemData);
                    // For currently selected EAP type, store provider specific data
                    if (pWCP->wzcConfig.EapolParams.dwEapType == pCurExt->dwExtensionType) {
                        DWORD dwAuthDataLen = pWCP->wzcConfig.EapolParams.dwAuthDataLen;
                        PBYTE pbAuthData    = pWCP->wzcConfig.EapolParams.pbAuthData;

                        pEapCbItemData->dwAuthDataLen = dwAuthDataLen;
                        if (dwAuthDataLen && pbAuthData) {
                            pEapCbItemData->pbAuthData = LocalAlloc(LPTR,dwAuthDataLen);
                            if (pEapCbItemData->pbAuthData) {
                                memcpy(pEapCbItemData->pbAuthData,pbAuthData,dwAuthDataLen);
                            }
                        }
                        ComboBox_SetCurSel(pWCP->hwndCbEAPType,iListIndex);
                        bCurSelSet = TRUE;
                    } else if (pCurExt->dwExtensionType == DEFAULT_EAP_TYPE) {
                        wszDefaultFriendlyName = pCurExt->wszFriendlyName;
                    }
                }
            }
            // No matching providers, set to default
            if (pWCP->dwNumEapExt && !bCurSelSet) {
                UINT uResult=0;
                DEBUGMSG(ZONE_NETWORK,(TEXT("WZCUI: No matching providers for EAP type %d\r\n"),
                                       pWCP->wzcConfig.EapolParams.dwEapType));
                if (wszDefaultFriendlyName) {
                    uResult = ComboBox_FindString(pWCP->hwndCbEAPType,-1,wszDefaultFriendlyName);
                    if (uResult == CB_ERR)
                        uResult = 0;
                }
                ComboBox_SetCurSel(pWCP->hwndCbEAPType,uResult);
            }
        }
    }
    return ERROR_SUCCESS;
}

void
Cleanup8021xControls(WLAN_CFG_PROPS *pWCP)
{
    int i,j;
    WZC_EAP_CB_ITEMDATA *pEapCbItemData;
    j = ComboBox_GetCount(pWCP->hwndCbEAPType);
    for (i=0;i<j;i++) {
        pEapCbItemData = (WZC_EAP_CB_ITEMDATA *)ComboBox_GetItemData(pWCP->hwndCbEAPType,i);
        if (pEapCbItemData && ((int)pEapCbItemData != CB_ERR)) {
            if (pEapCbItemData->pbAuthData) {
                LocalFree(pEapCbItemData->pbAuthData);
            }
            LocalFree(pEapCbItemData);
        }
    }
    if (pWCP->pEapExtInfo) {
        LocalFree(pWCP->pEapExtInfo);
        pWCP->pEapExtInfo = NULL;
    }
}


VOID
Enable8021xControls(HWND hDlg, WLAN_CFG_PROPS *pWCP)
{
    BOOL bEnable;

	// If Adhoc is unchecked, enable all 802.1x controls
	bEnable = (IsDlgButtonChecked(hDlg, IDC_ADHOC) == BST_UNCHECKED);
	EnableWindow(pWCP->hwndChkEnable8021x, bEnable);
	EnableWindow(pWCP->hwndCbEAPType,bEnable);
	EnableWindow(pWCP->hwndBtnEAPProps,bEnable);

	// If Adhoc is checked, uncheck 802.1x box
	if (!bEnable) {
		CheckDlgButton(hDlg, IDC_WZC_CHK_ENABLE8021X, BST_UNCHECKED);
	}

	// If 802.1x is not checked, then uncheck EAP and props
	bEnable = IsDlgButtonChecked(hDlg, IDC_WZC_CHK_ENABLE8021X);
	EnableWindow(pWCP->hwndCbEAPType,bEnable);
	EnableWindow(pWCP->hwndBtnEAPProps,bEnable);

    if (bEnable) {
        WZC_EAP_CB_ITEMDATA *pEapCbItemData=NULL;
        int iListIndex = ComboBox_GetCurSel(pWCP->hwndCbEAPType);
        if (iListIndex != CB_ERR) {
            pEapCbItemData = (WZC_EAP_CB_ITEMDATA *)ComboBox_GetItemData(pWCP->hwndCbEAPType,iListIndex);
        }
        bEnable = (pEapCbItemData && ((int)pEapCbItemData != CB_ERR) && 
                   pEapCbItemData->pEapExtInfo->wszConfigUIPath && 
                   pEapCbItemData->pEapExtInfo->wszConfigUIPath[0]);
    }
}

//+---------------------------------------------------------------------------
// initializes WEP controls
DWORD
InitWepKControls(HWND hDlg, WLAN_CFG_PROPS *pWCP)
{
	UINT nWepKLen = 0;

    // check whether the key is provided automatically or not
    CheckDlgButton(hDlg, IDC_USEHARDWAREPW, 
        (pWCP->wzcConfig.dwCtlFlags & WZCCTL_WEPK_PRESENT) ? BST_UNCHECKED : BST_CHECKED);

    if (pWCP->wzcConfig.KeyLength == 0)
    {
        nWepKLen = 0;
    }
    //--- when a password is to be displayed as hidden chars, don't put in
    //--- its actual length, but just 8 bulled chars.
    else
    {
        nWepKLen = 8;
    }

    g_wszHiddWepK[nWepKLen] = L'\0';
    SetWindowText(pWCP->hwndEdKMat, g_wszHiddWepK);
    g_wszHiddWepK[nWepKLen] = L'*';

    // the index edit control shouldn't accept more than exactly one char
    SendMessage(pWCP->hwndEdKIdx, EM_LIMITTEXT, 1, 0);

    // show the current key index, if valid. Otherwise, default to the min valid value.
    if ((pWCP->wzcConfig.KeyIndex + 1) >= WZC_WEPKIDX_MIN && 
        (pWCP->wzcConfig.KeyIndex + 1) <= WZC_WEPKIDX_MAX)
    {
        WCHAR   wszIdx[WZC_WEPKIDX_NDIGITS];
        SetWindowText(pWCP->hwndEdKIdx, _itow(pWCP->wzcConfig.KeyIndex + 1, wszIdx, 10));
    }
    else
        pWCP->wzcConfig.KeyIndex = 0;

    return ERROR_SUCCESS;
}


DWORD
SaveWepKSettings(WLAN_CFG_PROPS *pWCP)
{
	DWORD 	dwRetVal 	= ERROR_SUCCESS;
	LPSTR	szWepKMat	= NULL;
	LPWSTR	wszWepKMat	= NULL;
	WCHAR	wszKeyIdx[2];
	DWORD	cdwWepK;
	DWORD	i;
	BOOL	fXFormat 	= FALSE;

	/*** Set Wep Key Index ***/

	GetWindowText(pWCP->hwndEdKIdx, wszKeyIdx, ARRAYSIZE(wszKeyIdx));
	wszKeyIdx[1] = '\0'; // Make sure we only get one character key

	pWCP->wzcConfig.KeyIndex = _wtoi(wszKeyIdx) - 1;
	if ((pWCP->wzcConfig.KeyIndex + 1) < WZC_WEPKIDX_MIN || 
		(pWCP->wzcConfig.KeyIndex + 1) > WZC_WEPKIDX_MAX)
	{
		dwRetVal = IDS_WZC_KERR_IDX;	
	}

	/*** Set the wep key ***/

	// Get length of wep key
	cdwWepK = GetWindowTextLength(pWCP->hwndEdKMat);
	if(cdwWepK == 0)
	{
		dwRetVal = IDS_WZC_KFMT_LEN;
		goto exit;
	}

	szWepKMat = LocalAlloc(0, cdwWepK + 1);
	wszWepKMat = LocalAlloc(0, (cdwWepK + 1) * sizeof(WCHAR));
	if(szWepKMat == NULL || wszWepKMat == NULL)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Out of memory")));
		goto exit;
	}

	// Get the window text
	GetWindowText(pWCP->hwndEdKMat, wszWepKMat, cdwWepK + 1);

	WideCharToMultiByte(CP_ACP,
                    0,
                    wszWepKMat,
                    cdwWepK+1,
                    szWepKMat,
                    cdwWepK+1,
                    NULL,NULL);

	// Check if the WEP key is already saved
	if(!strncmp(szWepKMat, g_szHiddWepK, cdwWepK))
	{
		DEBUGMSG(ZONE_MISC, (TEXT("NETUIQC: WEP key unchanged, using the same one")));
		goto exit;
	}

    //
	// SSN Pre-shared key takes 256 bit key.
    // Everything else is incorrect formatting.
    // Translates a user password (8 to 63 ascii chars) into a 256 bit network key.
    // We do this for WPA-PSK, WPA2PSK and WPA-None.
    // 
    if (pWCP->wzcConfig.AuthenticationMode == Ndis802_11AuthModeWPAPSK   ||
        pWCP->wzcConfig.AuthenticationMode == Ndis802_11AuthModeWPA2PSK  ||
        (pWCP->wzcConfig.AuthenticationMode == Ndis802_11AuthModeWPANone &&
         pWCP->wzcConfig.Privacy == Ndis802_11Encryption2Enabled ||
         pWCP->wzcConfig.Privacy == Ndis802_11Encryption3Enabled))
    {        
        //
        //  User can only enter either 64 hex entries, or 8/63 any ASCII entries which is always
        //  converted to XFORMAT.
        //
        
        pWCP->wzcConfig.dwCtlFlags |= WZCCTL_WEPK_XFORMAT;
        
        if (cdwWepK >= 8 && cdwWepK <= 63)
        {
            pfnWZCPassword2Key(
                &pWCP->wzcConfig,
                szWepKMat);

            cdwWepK = WZCCTL_MAX_WEPK_MATERIAL;
        }
		else if (cdwWepK == 64)
		{
			// ******************************************
			// PROCESS the 64 digit hex key here
			// ******************************************
			pWCP->wzcConfig.KeyLength = WZCCTL_MAX_WEPK_MATERIAL;
			pWCP->wzcConfig.dwCtlFlags |= WZCCTL_WEPK_PRESENT;

			// Check for all hex digits
			for(i = 0; i < cdwWepK; i++)
			{
				if(!isxdigit(szWepKMat[i]))
				{
					dwRetVal = IDS_WZC_KFMT_DGTS;
					goto exit;
				}
			}

			CopyWepKMaterial(pWCP, szWepKMat, FALSE);
			goto exit;

		}
        else 
        if (cdwWepK != 64)
        {
            //
            //  TBD modify the error message reflecting the 8/63 rule.
            //

            dwRetVal = IDS_WZC_KFMT_LEN;
        }

        //
        //  The WZCSVC requires it to be encrypted in place...
        //
        
        if (dwRetVal == ERROR_SUCCESS)
        {
            //
            //  Tell it to just encrypt and no conversion.
            //
            pWCP->wzcConfig.KeyLength  =  cdwWepK;
    		pWCP->wzcConfig.dwCtlFlags |= WZCCTL_WEPK_PRESENT;    		
            CopyWepKMaterial(pWCP, NULL, TRUE);      
            goto exit;
        }
        
    }

    // for all the remaining cases, keys of 40, 104 and 128 are allowed
    else
    {
        switch(cdwWepK)
        {
        case WZC_WEPKMAT_40_ASC:    // 5 chars
        case WZC_WEPKMAT_104_ASC:   // 13 chars
        case WZC_WEPKMAT_128_ASC:   // 16 chars
            break;
        case WZC_WEPKMAT_40_HEX:    // 10 hexadecimal digits
        case WZC_WEPKMAT_104_HEX:   // 26 hexadecimal digits
        case WZC_WEPKMAT_128_HEX:   // 32 hexadecimal digits
            pWCP->wzcConfig.dwCtlFlags |= WZCCTL_WEPK_XFORMAT;
            break;
        default:
            dwRetVal = IDS_WZC_KFMT_LEN;
        }
    }

	if(dwRetVal == ERROR_SUCCESS)
	{
		pWCP->wzcConfig.KeyLength = cdwWepK;
		pWCP->wzcConfig.dwCtlFlags |= WZCCTL_WEPK_PRESENT;
	
		// Check for all hex digits if applicable
		if(pWCP->wzcConfig.dwCtlFlags & WZCCTL_WEPK_XFORMAT)
		{
			for(i = 0; i < cdwWepK; i++)
			{
				if(!isxdigit(szWepKMat[i]))
				{
					dwRetVal = IDS_WZC_KFMT_DGTS;
					goto exit;
				}
			}
		}

		CopyWepKMaterial(pWCP, szWepKMat, FALSE);
	}
	
exit:
	if(szWepKMat)
		LocalFree(szWepKMat);
	if(wszWepKMat)
		LocalFree(wszWepKMat);
	
	return dwRetVal;
}

//+---------------------------------------------------------------------------
// parses & copies the WEP Key material from the parameter into the pWCP->wzcConfig object
// The length should be already the good one, the format is given by pWCP->wzcConfig.dwCtlFlags
// Since we assume a valid key material it means its length is non-zero and it is fitting in
// the configurations key material buffer, and if the formatting is hexadecimal, it 
// contains an even number of hexa digits.
DWORD
CopyWepKMaterial(WLAN_CFG_PROPS *pWCP,LPSTR szKMat, BOOL bJustEncrypt)
{
    BYTE     chFakeKeyMaterial[] = {0x56, 0x09, 0x08, 0x98, 0x4D, 0x08, 0x11, 0x66, 0x42, 0x03, 0x01, 0x67, 0x66};

    //
    //  Don't update the KeyMaterial if user has not changed anything.
    //  Somehow the IDC_WZC_EDIT_KMAT message is always sent causing the
    //  bKMatTouched to be always TRUE.
    //

    if (bJustEncrypt)
        goto JustEncrypt;
    
    if (!memcmp(szKMat, g_szHiddWepK, strlen(szKMat)))
        return ERROR_SUCCESS;    
    
    if (pWCP->wzcConfig.dwCtlFlags & WZCCTL_WEPK_XFORMAT)
    {
        UINT  nKMatIdx = 0;

        // we know here we have a valid hexadecimal formatting
        // this implies the string has an even number of digits
        while(*szKMat != '\0')
        {
            pWCP->wzcConfig.KeyMaterial[nKMatIdx] = HEX(*szKMat) << 4;
            szKMat++;
            pWCP->wzcConfig.KeyMaterial[nKMatIdx] |= HEX(*szKMat);
            szKMat++;
            nKMatIdx++;
        }
        pWCP->wzcConfig.KeyLength = nKMatIdx;
    }
    else
    {
        // the key is not in Hex format, so just copy over the bytes
        // we know the length is good so no worries about overwritting the buffer
        pWCP->wzcConfig.KeyLength = strlen(szKMat);
        memcpy(pWCP->wzcConfig.KeyMaterial, szKMat, pWCP->wzcConfig.KeyLength);
    }

JustEncrypt:
    // TODO: this is where the key should be encrypted inplace
    {
        UINT i;
        for (i = 0; i < WZCCTL_MAX_WEPK_MATERIAL; i++)
            pWCP->wzcConfig.KeyMaterial[i] ^= chFakeKeyMaterial[(7*i)%13];
    }

    return ERROR_SUCCESS;
}


void RefreshWZCControls(HWND hDlg, PWLAN_CFG_PROPS pWCP)
{
	WCHAR 	wszAuthentication[MAX_CB_STRING];
	WCHAR	wszEncryption[MAX_CB_STRING];
	int 	iDelete;
	int		iCurSel;
	BOOL	fAutoKey 	= FALSE;
	BOOL	fKeyMat		= FALSE;
	BOOL	fKeyIdx		= FALSE;

	Enable8021xControls(hDlg, pWCP);

	// Enable auth control if encryption is not disabled
	if((iCurSel = SendMessage(pWCP->hwndCbEncrypt, CB_GETCURSEL, 0, 0)) != CB_ERR)
	{
		SendMessage(pWCP->hwndCbEncrypt, CB_GETLBTEXT, iCurSel, (LPARAM)wszEncryption);

		if(wcscmp(wszEncryption, g_wszCbDisabled) != 0)
		{
			EnableWindow(pWCP->hwndCbAuthenticate, TRUE);
		}
		else
		{
			// Uncheck the 8021x check box, disable it, and disable EAP type and props
			CheckDlgButton(hDlg, IDC_WZC_CHK_ENABLE8021X, BST_UNCHECKED);
			EnableWindow(pWCP->hwndChkEnable8021x, FALSE);
			EnableWindow(pWCP->hwndCbEAPType,FALSE);
			EnableWindow(pWCP->hwndBtnEAPProps,FALSE);
		}
	}

	// Add SSN if it is not already present and the NIC supports it.  It could have been removed from
	// the list on a previous refresh.
	if((pWCP->dwNicCapabilities & INTFCAP_SSN) && 
		(SendMessage(pWCP->hwndCbAuthenticate, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbSSN) == CB_ERR))
	{
		SendMessage(pWCP->hwndCbAuthenticate, CB_ADDSTRING, 0, (LPARAM)g_wszCbSSN);
	}

    // Add 802.11i (WPA2 and WPA2PSK) if NIC supports it and it hasn't been removed.
    if (pWCP->dwNicCapabilities & INTFCAP_80211I &&
        SendMessage(pWCP->hwndCbAuthenticate, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbWPA2) == CB_ERR) 
    {
		SendMessage(pWCP->hwndCbAuthenticate, CB_ADDSTRING, 0, (LPARAM)g_wszCbWPA2);    
    }

	// Figure out what to do based on authentication and infrastructure
	if((iCurSel = SendMessage(pWCP->hwndCbAuthenticate, CB_GETCURSEL, 0, 0)) != CB_ERR)
	{	    
		SendMessage(pWCP->hwndCbAuthenticate, CB_GETLBTEXT, iCurSel, (LPARAM)wszAuthentication);

		if(BST_CHECKED == SendMessage(pWCP->hwndChkAdhoc, BM_GETCHECK, 0, 0)) {           
            
			int iSSNPSK;
           
			// If one of SSN, WPA2, WPA2PSK, is selected then reselect SSNPSK
			if (!wcscmp(wszAuthentication, g_wszCbSSN) || !wcscmp(wszAuthentication, g_wszCbWPA2) || !wcscmp(wszAuthentication, g_wszCbWPA2PSK)) {
				if((iSSNPSK = SendMessage(pWCP->hwndCbAuthenticate, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbSSNPSK)) != CB_ERR) {
					SendMessage(pWCP->hwndCbAuthenticate, CB_SETCURSEL, iSSNPSK, 0);
					wcscpy(wszAuthentication, g_wszCbSSNPSK);
				}
			}

            // WPA2, WPA2PSK, SSN, are not selected in adhoc mode scale back to WPA.            
			if((iDelete = SendMessage(pWCP->hwndCbAuthenticate, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbSSN)) != CB_ERR)
				SendMessage(pWCP->hwndCbAuthenticate, CB_DELETESTRING, iDelete, 0);
            if((iDelete = SendMessage(pWCP->hwndCbAuthenticate, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbWPA2)) != CB_ERR)
				SendMessage(pWCP->hwndCbAuthenticate, CB_DELETESTRING, iDelete, 0);
            if((iDelete = SendMessage(pWCP->hwndCbAuthenticate, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbWPA2PSK)) != CB_ERR)
				SendMessage(pWCP->hwndCbAuthenticate, CB_DELETESTRING, iDelete, 0);            
		}        

        
		if(((!wcscmp(wszAuthentication, g_wszCbSSN))    ||
			(!wcscmp(wszAuthentication, g_wszCbSSNPSK)  ||
			(!wcscmp(wszAuthentication, g_wszCbWPA2)    ||
			(!wcscmp(wszAuthentication, g_wszCbWPA2PSK)))))
			&&
			(BST_CHECKED != SendMessage(pWCP->hwndChkAdhoc, BM_GETCHECK, 0, 0)))
		{
	    
			fAutoKey 	= FALSE;
			fKeyIdx		= FALSE;

            //
            //  Uncheck by default the key provided automatically check box if
            //  it's PSK.
            //
            if (!wcscmp(wszAuthentication, g_wszCbSSNPSK) || !wcscmp(wszAuthentication, g_wszCbWPA2PSK))
			    SendMessage(pWCP->hwndUseHardwarePW, BM_SETCHECK, BST_UNCHECKED, 0);
            else
                SendMessage(pWCP->hwndUseHardwarePW, BM_SETCHECK, BST_CHECKED, 0);

			SetWindowText(pWCP->hwndEdKIdx, _T("1"));
					
			if((iDelete = SendMessage(pWCP->hwndCbEncrypt, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbDisabled)) != CB_ERR)
				SendMessage(pWCP->hwndCbEncrypt, CB_DELETESTRING, iDelete, 0);
		
			if(!wcscmp(wszAuthentication, g_wszCbSSN) || !wcscmp(wszAuthentication, g_wszCbWPA2))
				fKeyMat = FALSE;
			else
				fKeyMat = TRUE;

            //  Handle encryption:
            
            //  TKIP is a must.
			if(SendMessage(pWCP->hwndCbEncrypt, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbTKIP) == CB_ERR)
				SendMessage(pWCP->hwndCbEncrypt, CB_ADDSTRING, 0, (LPARAM)g_wszCbTKIP);

            //  AES if it's WPA2 or WPA with AES supported in h/w.
            if (pWCP->dwNicCapabilities & INTFCAP_80211I || pWCP->bWPAAESCapable)
    			if(SendMessage(pWCP->hwndCbEncrypt, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbAES) == CB_ERR)
    				SendMessage(pWCP->hwndCbEncrypt, CB_ADDSTRING, 0, (LPARAM)g_wszCbAES);
            
            //  WEP is not supported in WPA and WPA2.            
			if((iDelete = SendMessage(pWCP->hwndCbEncrypt, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbWEP)) != CB_ERR)
                SendMessage(pWCP->hwndCbEncrypt, CB_DELETESTRING, iDelete, 0);

			// If we deleted the current selection, reselect the first item in the list
			if(SendMessage(pWCP->hwndCbEncrypt, CB_GETCURSEL, 0, 0) == CB_ERR)
				SendMessage(pWCP->hwndCbEncrypt, CB_SETCURSEL, 0, 0);

			// Check the 8021x check box, disable it, and enable EAP type and props if not WPAPSK or WPA2PSK.
            if (wcscmp(wszAuthentication, g_wszCbSSNPSK) && wcscmp(wszAuthentication, g_wszCbWPA2PSK)) {
    			CheckDlgButton(hDlg, IDC_WZC_CHK_ENABLE8021X, BST_CHECKED);
    			EnableWindow(pWCP->hwndChkEnable8021x, FALSE);
    			EnableWindow(pWCP->hwndCbEAPType,TRUE);
    			EnableWindow(pWCP->hwndBtnEAPProps,TRUE);
            } 
            else {
                //  PSK auth, 802.1x is a must, EAP type doesn't matter.
                //  Both 802.1x and EAP type are not user configurable.
    			CheckDlgButton(hDlg, IDC_WZC_CHK_ENABLE8021X, BST_CHECKED);
    			EnableWindow(pWCP->hwndChkEnable8021x, FALSE);
    			EnableWindow(pWCP->hwndCbEAPType,FALSE);
    			EnableWindow(pWCP->hwndBtnEAPProps,FALSE);
            }   
            
		}
		else if(!wcscmp(wszAuthentication, g_wszCbSSNPSK) &&
			(BST_CHECKED == SendMessage(pWCP->hwndChkAdhoc, BM_GETCHECK, 0, 0)))
		{
			fAutoKey 	= FALSE;
			fKeyMat		= TRUE;
			fKeyIdx		= FALSE;

			SendMessage(pWCP->hwndUseHardwarePW, BM_SETCHECK, BST_UNCHECKED, 0);
			SetWindowText(pWCP->hwndEdKIdx, _T("1"));

			// Remove disabled, wep entries
			if((iDelete = SendMessage(pWCP->hwndCbEncrypt, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbDisabled)) != CB_ERR)
				SendMessage(pWCP->hwndCbEncrypt, CB_DELETESTRING, iDelete, 0);
			if((iDelete = SendMessage(pWCP->hwndCbEncrypt, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbWEP)) != CB_ERR)
				SendMessage(pWCP->hwndCbEncrypt, CB_DELETESTRING, iDelete, 0);

			// Add TKIP if not already present
			if(SendMessage(pWCP->hwndCbEncrypt, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbTKIP) == CB_ERR)
				SendMessage(pWCP->hwndCbEncrypt, CB_ADDSTRING, 0, (LPARAM)g_wszCbTKIP);

			// Add/delete AES depending on the driver capability.
			if (pWCP->bWPAAESCapable)
            {         
    			if(SendMessage(pWCP->hwndCbEncrypt, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbAES) == CB_ERR)
    				SendMessage(pWCP->hwndCbEncrypt, CB_ADDSTRING, 0, (LPARAM)g_wszCbAES);            
            }                
            else
            {
    			if((iDelete = SendMessage(pWCP->hwndCbEncrypt, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbAES)) != CB_ERR)
    				SendMessage(pWCP->hwndCbEncrypt, CB_DELETESTRING, iDelete, 0);
            }                            

			// If we deleted the current selection, reselect the first item in the list
			if(SendMessage(pWCP->hwndCbEncrypt, CB_GETCURSEL, 0, 0) == CB_ERR)
				SendMessage(pWCP->hwndCbEncrypt, CB_SETCURSEL, 0, 0);
		}
		else if(!wcscmp(wszAuthentication, g_wszCbOpen) ||
			!wcscmp(wszAuthentication, g_wszCbShared))
		{
			fAutoKey	= TRUE;
			fKeyMat		= TRUE;
			fKeyIdx		= TRUE;

			// Remove TKIP and AES entries
			if((iDelete = SendMessage(pWCP->hwndCbEncrypt, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbTKIP)) != CB_ERR)
				SendMessage(pWCP->hwndCbEncrypt, CB_DELETESTRING, iDelete, 0);
			if((iDelete = SendMessage(pWCP->hwndCbEncrypt, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbAES)) != CB_ERR)
				SendMessage(pWCP->hwndCbEncrypt, CB_DELETESTRING, iDelete, 0);

			// Add Disabled and WEP if not already present
			if(SendMessage(pWCP->hwndCbEncrypt, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbDisabled) == CB_ERR)
				SendMessage(pWCP->hwndCbEncrypt, CB_ADDSTRING, 0, (LPARAM)g_wszCbDisabled);
			if(SendMessage(pWCP->hwndCbEncrypt, CB_FINDSTRINGEXACT, 0, (LPARAM)g_wszCbWEP) == CB_ERR)
				SendMessage(pWCP->hwndCbEncrypt, CB_ADDSTRING, 0, (LPARAM)g_wszCbWEP);
			
			// If we deleted the current selection, select WEP encryption
			if(SendMessage(pWCP->hwndCbEncrypt, CB_GETCURSEL, 0, 0) == CB_ERR)
				SendMessage(pWCP->hwndCbEncrypt, CB_SELECTSTRING, 0, (LPARAM)g_wszCbWEP);

			if(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_USEHARDWAREPW))
				fKeyMat = FALSE;

			if(!wcscmp(wszAuthentication, g_wszCbShared)) {
				// Uncheck the 8021x check box and disable EAP type and props
				CheckDlgButton(hDlg, IDC_WZC_CHK_ENABLE8021X, BST_UNCHECKED);
				EnableWindow(pWCP->hwndChkEnable8021x,FALSE);
				EnableWindow(pWCP->hwndCbEAPType,FALSE);
				EnableWindow(pWCP->hwndBtnEAPProps,FALSE);
			}
		}
	}

	// Let's enable controls based on previous queries and current encryption
	if((iCurSel = SendMessage(pWCP->hwndCbEncrypt, CB_GETCURSEL, 0, 0)) != CB_ERR)
	{
		SendMessage(pWCP->hwndCbEncrypt, CB_GETLBTEXT, iCurSel, (LPARAM)wszEncryption);
	
		if(!wcscmp(wszEncryption, g_wszCbDisabled))
		{
			EnableWindow(pWCP->hwndCbAuthenticate, FALSE);
			EnableWindow(pWCP->hwndUseHardwarePW, FALSE);
			EnableWindow(pWCP->hwndEdKMat, FALSE);
			EnableWindow(pWCP->hwndEdKIdx, FALSE);
		}
		else
		{
			if(fAutoKey)
				EnableWindow(pWCP->hwndUseHardwarePW, TRUE);
			else
				EnableWindow(pWCP->hwndUseHardwarePW, FALSE);
			
			if(fKeyMat)
				EnableWindow(pWCP->hwndEdKMat, TRUE);
			else
				EnableWindow(pWCP->hwndEdKMat, FALSE);
			
			if(fKeyIdx && (BST_UNCHECKED == IsDlgButtonChecked(hDlg, IDC_USEHARDWAREPW)))
				EnableWindow(pWCP->hwndEdKIdx, TRUE);
			else
				EnableWindow(pWCP->hwndEdKIdx, FALSE);
		}
	}
}


LRESULT CALLBACK
NetworkWLanPropsDlgProc (
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam
   )
{
	static WZC_WLAN_CONFIG s_OrigCfg;
    WLAN_CFG_PROPS *pWCP=NULL;
    WPARAM wCmdId;
    WPARAM wCmdCbn;

	switch (message) {
    case WM_NOTIFY :
        break;
    case WM_CLOSE:
    	DestroyWindow(hDlg);
    	break;
    case WM_DESTROY:
    	PostQuitMessage(0);
    	break;
    case WM_INITDIALOG :
        {
            HRESULT hr = S_OK;

			BOOL fUseSSNCipher = FALSE;

            pWCP = (WLAN_CFG_PROPS *)lParam;
            if (pWCP == NULL)
                return FALSE;
            SetWindowLong(hDlg, GWL_USERDATA, (LONG)pWCP);

            // Save this structure so we can compare if anything has changed when finished
            s_OrigCfg = pWCP->wzcConfig;

			// Store strings in globals
			LoadString(v_hInst, IDS_WZC_SHARED, g_wszCbShared, MAX_CB_STRING);
			LoadString(v_hInst, IDS_WZC_OPEN, g_wszCbOpen, MAX_CB_STRING);
			LoadString(v_hInst, IDS_WZC_SSNPSK, g_wszCbSSNPSK, MAX_CB_STRING);
			LoadString(v_hInst, IDS_WZC_SSN, g_wszCbSSN, MAX_CB_STRING);
			LoadString(v_hInst, IDS_WZC_DISABLED, g_wszCbDisabled, MAX_CB_STRING);
			LoadString(v_hInst, IDS_WZC_WEP, g_wszCbWEP, MAX_CB_STRING);
			LoadString(v_hInst, IDS_WZC_TKIP, g_wszCbTKIP, MAX_CB_STRING);
			LoadString(v_hInst, IDS_WZC_AES, g_wszCbAES, MAX_CB_STRING);
            LoadString(v_hInst, IDS_WZC_WPA2PSK, g_wszCbWPA2PSK, MAX_CB_STRING);
            LoadString(v_hInst, IDS_WZC_WPA2, g_wszCbWPA2, MAX_CB_STRING);            
            

            // get all the controls handles
            pWCP->hwndEdSSID        = GetDlgItem(hDlg,IDC_WZC_EDIT_SSID);
            pWCP->hwndChkAdhoc      = GetDlgItem(hDlg,IDC_ADHOC);
            pWCP->hwndCbEncrypt     = GetDlgItem(hDlg,IDC_WZC_ENCRYPT);
            pWCP->hwndCbAuthenticate= GetDlgItem(hDlg,IDC_WZC_AUTHEN);
            pWCP->hwndUseHardwarePW = GetDlgItem(hDlg,IDC_USEHARDWAREPW);
            pWCP->hwndChkEnable8021x= GetDlgItem(hDlg,IDC_WZC_CHK_ENABLE8021X);
            pWCP->hwndCbEAPType     = GetDlgItem(hDlg,IDC_WZC_CB_EAPTYPE);
            pWCP->hwndBtnEAPProps   = GetDlgItem(hDlg,IDB_WZC_EAPPROPS);
            pWCP->hwndEdKMat  		= GetDlgItem(hDlg,IDC_WZC_EDIT_KMAT);
            pWCP->hwndEdKIdx  		= GetDlgItem(hDlg,IDC_WZC_EDIT_KIDX);

            if (pWCP->IDD == IDC_WZC_DLG_PROPS_W) {
                DWORD dwStyle;
                HWND hwndDefBtn = (pWCP->dwFlags & WZCDLG_PROPS_DEFOK) ? GetDlgItem(hDlg,IDOK) : GetDlgItem(hDlg,IDCANCEL);

                dwStyle = GetWindowLong(hwndDefBtn, GWL_STYLE);
                SendMessage(hwndDefBtn, BM_SETSTYLE, dwStyle | BS_DEFPUSHBUTTON, 0);
            }

            // initialize the SSID field with the SSID, if one is given
            if (0 != strcmp(pWCP->wzcConfig.Ssid.Ssid, "")) {
                // In order to convert the SSID to LPWSTR we need a buffer.
                // We know an SSID can't exceed 32 chars (see NDIS_802_11_SSID from ntddndis.h) so
                // make room for the null terminator and that's it. We could do mem alloc but I'm
                // not sure it worth the effort (at runtime).
                WCHAR   wszSSID[33];
                UINT    nLenSSID = 0;

                // convert the LPSTR (original SSID format) to LPWSTR (needed in List Ctrl)
                nLenSSID = MultiByteToWideChar(
                                              CP_ACP,
                                              0,
                                              (LPCSTR)pWCP->wzcConfig.Ssid.Ssid,
                                              pWCP->wzcConfig.Ssid.SsidLength,
                                              wszSSID,
                                              ARRAYSIZE(wszSSID));
                if (nLenSSID != 0) 
                {
                    wszSSID[nLenSSID] = L'\0';
                    SetWindowText(pWCP->hwndEdSSID, wszSSID);
                }
            }

            // Check the "this network is adhoc" box if neccessary.
            SendMessage(pWCP->hwndChkAdhoc, BM_SETCHECK, (pWCP->wzcConfig.InfrastructureMode == Ndis802_11IBSS) ? BST_CHECKED : BST_UNCHECKED, 0);
    			
			// Add open and shared items to combo box
			SendMessage(pWCP->hwndCbAuthenticate, CB_ADDSTRING, 0, (LPARAM)g_wszCbShared);
			SendMessage(pWCP->hwndCbAuthenticate, CB_ADDSTRING, 0, (LPARAM)g_wszCbOpen);

			// Add SSN and SSNPSK if supported by NIC
			if(pWCP->dwNicCapabilities & INTFCAP_SSN)
			{
				SendMessage(pWCP->hwndCbAuthenticate, CB_ADDSTRING, 0, (LPARAM)g_wszCbSSNPSK);
				SendMessage(pWCP->hwndCbAuthenticate, CB_ADDSTRING, 0, (LPARAM)g_wszCbSSN);
			}

            // Add WPA and WPA2 if supported by NIC.
			if(pWCP->dwNicCapabilities & INTFCAP_80211I)
			{
				SendMessage(pWCP->hwndCbAuthenticate, CB_ADDSTRING, 0, (LPARAM)g_wszCbWPA2PSK);
				SendMessage(pWCP->hwndCbAuthenticate, CB_ADDSTRING, 0, (LPARAM)g_wszCbWPA2);
			}            

			/* Determine the authentication to select by default */

            if((pWCP->dwFlags & WZCDLG_PROPS_ADD) && (pWCP->dwNicCapabilities & INTFCAP_80211I))
            {
                //  Add a new config with Nic supporting WPA2.
                SendMessage(pWCP->hwndCbAuthenticate, CB_SELECTSTRING, 0, (LPARAM)g_wszCbWPA2);
				fUseSSNCipher = TRUE;
            }            
            else if((pWCP->dwFlags & WZCDLG_PROPS_ADD) && (pWCP->dwNicCapabilities & INTFCAP_SSN))
			{
				// Add a new config with Nic supporting SSN
				SendMessage(pWCP->hwndCbAuthenticate, CB_SELECTSTRING, 0, (LPARAM)g_wszCbSSN);
				fUseSSNCipher = TRUE;
			}
			else if(pWCP->dwFlags & WZCDLG_PROPS_ADD)
			{
				// Add a new config with no SSN support
				SendMessage(pWCP->hwndCbAuthenticate, CB_SELECTSTRING, 0, (LPARAM)g_wszCbOpen);
			}
            else if ((pWCP->dwFlags & WZCDLG_PROPS_CONF) && 
				(pWCP->dwNicCapabilities & INTFCAP_80211I) &&
				((pWCP->wzcConfig.AuthenticationMode == Ndis802_11AuthModeWPA2)))
            {
				// Configure an SSID with WPA2 support in NIC and network
				SendMessage(pWCP->hwndCbAuthenticate, CB_SELECTSTRING, 0, (LPARAM)g_wszCbWPA2);
				fUseSSNCipher = TRUE;
            } 
			else if((pWCP->dwFlags & WZCDLG_PROPS_CONF) &&
				(pWCP->dwNicCapabilities & INTFCAP_80211I) &&
				((pWCP->wzcConfig.AuthenticationMode == Ndis802_11AuthModeWPA2PSK)))
            {
				// Configure an SSID with WPA2PSK support in NIC and network
				SendMessage(pWCP->hwndCbAuthenticate, CB_SELECTSTRING, 0, (LPARAM)g_wszCbWPA2PSK);
				fUseSSNCipher = TRUE;
            }            
            else if((pWCP->dwFlags & WZCDLG_PROPS_CONF) && 
				(pWCP->dwNicCapabilities & INTFCAP_SSN) &&
				((pWCP->wzcConfig.AuthenticationMode == Ndis802_11AuthModeWPA)))
			{
				// Configure an SSID with SSN support in NIC and network
				SendMessage(pWCP->hwndCbAuthenticate, CB_SELECTSTRING, 0, (LPARAM)g_wszCbSSN);
				fUseSSNCipher = TRUE;
			}
			else if((pWCP->dwFlags & WZCDLG_PROPS_CONF) &&
				(pWCP->dwNicCapabilities & INTFCAP_SSN) &&
				((pWCP->wzcConfig.AuthenticationMode == Ndis802_11AuthModeWPANone)))
			{
				// Configure an SSID with SSN support in Nic ans SSNNone in network
				SendMessage(pWCP->hwndCbAuthenticate, CB_SELECTSTRING, 0, (LPARAM)g_wszCbSSNPSK);
				SendMessage(pWCP->hwndChkAdhoc, BM_SETCHECK, BST_CHECKED, 0);
				fUseSSNCipher = TRUE;
			}
			else if((pWCP->dwFlags & WZCDLG_PROPS_CONF) &&
				(pWCP->dwNicCapabilities & INTFCAP_SSN) &&
				(pWCP->wzcConfig.AuthenticationMode == Ndis802_11AuthModeWPAPSK))
			{
				// Configure an SSID with SSN support in Nic and SSNPSK in network
				SendMessage(pWCP->hwndCbAuthenticate, CB_SELECTSTRING, 0, (LPARAM)g_wszCbSSNPSK);
				fUseSSNCipher = TRUE;
			}
			else if((pWCP->dwFlags & WZCDLG_PROPS_CONF) &&
				(pWCP->wzcConfig.AuthenticationMode == Ndis802_11AuthModeShared))
			{
				// Configure an SSID with no SSN support and shared in SSID
				SendMessage(pWCP->hwndCbAuthenticate, CB_SELECTSTRING, 0, (LPARAM)g_wszCbShared);
			}
			else
			{
				// Configure an SSID with no SSN support and shared in SSID
				SendMessage(pWCP->hwndCbAuthenticate, CB_SELECTSTRING, 0, (LPARAM)g_wszCbOpen);
			}

			// Add all encryption types to combo box
			SendMessage(pWCP->hwndCbEncrypt, CB_ADDSTRING, 0, (LPARAM)g_wszCbDisabled);
			SendMessage(pWCP->hwndCbEncrypt, CB_ADDSTRING, 0, (LPARAM)g_wszCbWEP);
			SendMessage(pWCP->hwndCbEncrypt, CB_ADDSTRING, 0, (LPARAM)g_wszCbTKIP);

            // Add AES if hardware supports).
            if (pWCP->dwNicCapabilities & INTFCAP_80211I || pWCP->bWPAAESCapable)
			    SendMessage(pWCP->hwndCbEncrypt, CB_ADDSTRING, 0, (LPARAM)g_wszCbAES);
            

			/* Determine the encryption to select by default */


			if((pWCP->dwNicCapabilities & INTFCAP_80211I) && fUseSSNCipher)
            {
                SendMessage(
                    pWCP->hwndCbEncrypt, 
                    CB_SELECTSTRING, 
                    0, 
                    (pWCP->wzcConfig.Privacy == Ndis802_11Encryption3Enabled || pWCP->dwFlags & WZCDLG_PROPS_ADD) ?
                        (LPARAM)g_wszCbAES :
                        (LPARAM)g_wszCbTKIP);

            }
            else if((pWCP->dwNicCapabilities & INTFCAP_SSN) && fUseSSNCipher)
			{
                SendMessage(
                    pWCP->hwndCbEncrypt, 
                    CB_SELECTSTRING, 
                    0, 
                    ((pWCP->bWPAAESCapable) && 
                     (pWCP->wzcConfig.Privacy == Ndis802_11Encryption3Enabled || pWCP->dwFlags & WZCDLG_PROPS_ADD)) ?
                        (LPARAM)g_wszCbAES :
                        (LPARAM)g_wszCbTKIP);
			}
			else if(pWCP->wzcConfig.Privacy == Ndis802_11EncryptionDisabled)
			{
				SendMessage(pWCP->hwndCbEncrypt, CB_SELECTSTRING, 0, (LPARAM)g_wszCbDisabled);
			}
			else
			{	
				SendMessage(pWCP->hwndCbEncrypt, CB_SELECTSTRING, 0, (LPARAM)g_wszCbWEP);
			}
						
            // the SSID can't be under any circumstances larger than 32 chars
            SendMessage(pWCP->hwndEdSSID, EM_LIMITTEXT, 32, 0);

            // enable or disable the controls based on how the dialog is called
            EnableWindow(pWCP->hwndEdSSID, pWCP->dwFlags & WZCDLG_PROPS_RWSSID);
            EnableWindow(pWCP->hwndChkAdhoc, pWCP->dwFlags & WZCDLG_PROPS_RWINFR);
            
			InitWepKControls(hDlg, pWCP);

            // Enable/disable 802.1x controls
            if ((Init8021xControls(hDlg,pWCP) != ERROR_SUCCESS) || !(pWCP->dwFlags & WZCDLG_PROPS_RW8021X)) {
                EnableWindow(pWCP->hwndChkEnable8021x, FALSE);
                EnableWindow(pWCP->hwndCbEAPType, FALSE);
                EnableWindow(pWCP->hwndBtnEAPProps, FALSE);
            }
            else {
                Enable8021xControls(hDlg,pWCP);
            }

            pWCP->bKMatTouched = FALSE;
            pWCP->fDlgOk = FALSE;

            RefreshWZCControls(hDlg, pWCP);
        }
        break;

    case WM_COMMAND :
        
        wCmdId  = LOWORD(wParam);
        wCmdCbn = HIWORD(wParam);

		pWCP = (WLAN_CFG_PROPS *) GetWindowLong (hDlg, GWL_USERDATA);
        if (pWCP == NULL) 
            return FALSE;

        switch ( wCmdId ) {
        
        case IDCANCEL:
            Cleanup8021xControls(pWCP);

            DestroyWindow(hDlg);
            break;

        case IDOK:
            {
                UINT nSSIDLen, nSSIDLenA;
                // variables used for prompting the user with warning/err messages
                UINT  nWarnStringID = 0;
                WCHAR wszWarn1[48], wszWarn2[48];
                WCHAR wszAuthenticate[MAX_CB_STRING], wszEncrypt[MAX_CB_STRING];

                *wszWarn1 = L'\0';
				*wszWarn2 = L'\0';

                pWCP->wzcConfig.Length = sizeof(WZC_WLAN_CONFIG);
                pWCP->wzcConfig.InfrastructureMode = (BST_CHECKED == SendMessage(pWCP->hwndChkAdhoc, BM_GETCHECK, 0, 0)) ? Ndis802_11IBSS : Ndis802_11Infrastructure;

				// Save authentication mode
				GetWindowText(pWCP->hwndCbAuthenticate, wszAuthenticate, MAX_CB_STRING);
				if(!wcscmp(wszAuthenticate, g_wszCbShared))
				{
					pWCP->wzcConfig.AuthenticationMode = Ndis802_11AuthModeShared;
				}
				else if(!wcscmp(wszAuthenticate, g_wszCbWPA2))
				{
					pWCP->wzcConfig.AuthenticationMode = Ndis802_11AuthModeWPA2;
				}
				else if(!wcscmp(wszAuthenticate, g_wszCbWPA2PSK))
				{
					pWCP->wzcConfig.AuthenticationMode = Ndis802_11AuthModeWPA2PSK;
				}                    
				else if(!wcscmp(wszAuthenticate, g_wszCbSSN))
				{
					pWCP->wzcConfig.AuthenticationMode = Ndis802_11AuthModeWPA;
				}
				else if(!wcscmp(wszAuthenticate, g_wszCbSSNPSK) &&
					pWCP->wzcConfig.InfrastructureMode == Ndis802_11IBSS)
				{
					pWCP->wzcConfig.AuthenticationMode = Ndis802_11AuthModeWPANone;
				}
				else if(!wcscmp(wszAuthenticate, g_wszCbSSNPSK) &&
					pWCP->wzcConfig.InfrastructureMode == Ndis802_11Infrastructure)
				{
					pWCP->wzcConfig.AuthenticationMode = Ndis802_11AuthModeWPAPSK;
				}
				else
				{
					pWCP->wzcConfig.AuthenticationMode = Ndis802_11AuthModeOpen;
				}
					
				// Save Encryption
				GetWindowText(pWCP->hwndCbEncrypt, wszEncrypt, MAX_CB_STRING);
				if(!wcscmp(wszEncrypt, g_wszCbDisabled))
				{
					pWCP->wzcConfig.Privacy = Ndis802_11EncryptionDisabled;
				}
				else if(!wcscmp(wszEncrypt, g_wszCbTKIP))
				{
					pWCP->wzcConfig.Privacy = Ndis802_11Encryption2Enabled;
				}
				else if(!wcscmp(wszEncrypt, g_wszCbAES))
				{
					pWCP->wzcConfig.Privacy = Ndis802_11Encryption3Enabled;
				}
				else
				{
					pWCP->wzcConfig.Privacy = Ndis802_11Encryption1Enabled;
				}

   				nWarnStringID = 0;
				
                // get the ssid (max 32 chars)
                {
                    WCHAR wszSSID[33];
                    nSSIDLen = GetWindowText(
                                             pWCP->hwndEdSSID,
                                             wszSSID,
                                             ARRAYSIZE(wszSSID));

                    // copy over whatever it is in the control to the ndis data structure
                    nSSIDLenA = WideCharToMultiByte(CP_ACP,
                                            0,
                                            wszSSID,
                                            nSSIDLen,
                                            pWCP->wzcConfig.Ssid.Ssid,
                                            sizeof(pWCP->wzcConfig.Ssid.Ssid),
                                            NULL,NULL);
                    if (nSSIDLenA)
                        pWCP->wzcConfig.Ssid.SsidLength = nSSIDLenA;
                    
                    if (nSSIDLen == 0)
                    {
                        nWarnStringID = IDS_WZC_SSID_LEN;
                        SendMessage(pWCP->hwndEdSSID, EM_SETSEL, -1, 0);
                        SetFocus(pWCP->hwndEdSSID);
                    }
                    
                }

                // check if this entry doesn't exist already in the list
                if (pWCP->dwFlags & WZCDLG_PROPS_NODUPS && AlreadyExists(pWCP) && nWarnStringID == 0) {
                    nWarnStringID = IDS_WZC_ERR_DUPSSID;
                    LoadString(v_hInst, IDS_WZC_INFRA_IBSS + pWCP->wzcConfig.InfrastructureMode, wszWarn1, ARRAYSIZE(wszWarn1));
                    GetWindowText(pWCP->hwndEdSSID, wszWarn2, ARRAYSIZE(wszWarn2)); 
                    SendMessage(pWCP->hwndEdSSID, EM_SETSEL, 0, -1);
                    SetFocus(pWCP->hwndEdSSID);
                }

				// Save Wep Key settings if wep is not disabled, we have not encountered an error, and
				// UseHardwarePW is unchecked
				if(pWCP->wzcConfig.Privacy != Ndis802_11EncryptionDisabled && nWarnStringID == 0 &&
					(BST_UNCHECKED == IsDlgButtonChecked(hDlg,IDC_USEHARDWAREPW)))
				{
					nWarnStringID = SaveWepKSettings(pWCP);
					if(nWarnStringID == IDS_WZC_KERR_IDX)
					{
                        _itow(WZC_WEPKIDX_MIN, wszWarn1, 10);
                        _itow(WZC_WEPKIDX_MAX, wszWarn2, 10);
                        SendMessage(pWCP->hwndEdKIdx, EM_SETSEL, -1, 0);
                        SetFocus(pWCP->hwndEdKIdx);
					}
				}
				else if(BST_CHECKED == IsDlgButtonChecked(hDlg,IDC_USEHARDWAREPW) && nWarnStringID == 0)
				{
					pWCP->wzcConfig.dwCtlFlags &= ~(WZCCTL_WEPK_PRESENT|WZCCTL_WEPK_XFORMAT);

					// If user checks this erase key info
					memset(pWCP->wzcConfig.KeyMaterial, 0, pWCP->wzcConfig.KeyLength);
					pWCP->wzcConfig.KeyLength = 0;
				}

                if (nWarnStringID != 0) {

                    WLanPropsErrorMessageBox(hDlg,nWarnStringID,wszWarn1,wszWarn2);

                } else {
                    // Get 802.1x settings
                    if (BST_CHECKED == SendMessage(pWCP->hwndChkEnable8021x, BM_GETCHECK, 0, 0)) {
                        int iSel;
                        WZC_EAP_CB_ITEMDATA *pEapCbItemData;

                        // Get provider specific info
                        iSel = ComboBox_GetCurSel(pWCP->hwndCbEAPType);
                        pEapCbItemData = (WZC_EAP_CB_ITEMDATA *)ComboBox_GetItemData(pWCP->hwndCbEAPType,iSel);
                        if (pEapCbItemData && (int)pEapCbItemData != CB_ERR) {
                            pWCP->wzcConfig.EapolParams.dwEapFlags    |= EAPOL_ENABLED;
                            pWCP->wzcConfig.EapolParams.bEnable8021x  = TRUE;
                            pWCP->wzcConfig.EapolParams.dwEapType     = pEapCbItemData->pEapExtInfo->dwExtensionType;
                            pWCP->wzcConfig.EapolParams.dwAuthDataLen = pEapCbItemData->dwAuthDataLen;
                            pWCP->wzcConfig.EapolParams.pbAuthData    = pEapCbItemData->pbAuthData;
                            // so Cleanup8021xControls won't free the memory
                            pEapCbItemData->pbAuthData = NULL;
                        }
                    }
                    else {
                        pWCP->wzcConfig.EapolParams.bEnable8021x = FALSE;
                        pWCP->wzcConfig.EapolParams.dwEapFlags   = 0;
                    }
                    Cleanup8021xControls(pWCP);
                    
                    if((! (pWCP->dwFlags & WZCDLG_PROPS_CONNECT)) && 
                    	(! memcmp(&s_OrigCfg, &pWCP->wzcConfig, sizeof(WZC_WLAN_CONFIG)))) {
                    	pWCP->fDlgOk = FALSE;
                    }
                    else {
                    	pWCP->fDlgOk = TRUE;
                    }
                    	
                    DestroyWindow(hDlg);
                }
            }
            break;
            
		case IDC_ADHOC:
        case IDC_WZC_CHK_ENABLE8021X:
           	RefreshWZCControls(hDlg, pWCP);           
            break;

		case IDC_WZC_ENCRYPT:
		case IDC_WZC_AUTHEN:
			if(wCmdCbn == CBN_SELCHANGE)
				RefreshWZCControls(hDlg, pWCP);
			break;
			
		case IDC_USEHARDWAREPW:
			RefreshWZCControls(hDlg, pWCP);
			break;
			
        case IDB_WZC_EAPPROPS:
            {
                HINSTANCE hinstEAPDll;
                WZC_EAP_CB_ITEMDATA *pEapCbItemData;
                PEAP_EXTENSION_INFO pCurExt=NULL;
                int iListIndex = ComboBox_GetCurSel(pWCP->hwndCbEAPType);
                DWORD dwError;
                UCHAR *pConnectionDataOut=NULL;
                DWORD dwSizeOfConnectionDataOut=0;

                pEapCbItemData = (WZC_EAP_CB_ITEMDATA *)ComboBox_GetItemData(pWCP->hwndCbEAPType,iListIndex);
                if (pEapCbItemData) 
                    pCurExt = pEapCbItemData->pEapExtInfo;

                if (pCurExt && pCurExt->wszConfigUIPath) {
                    RASEAPINVOKECONFIGUI pfnEapConfig;
                    RASEAPFREE pfnEapFreeMem;

                    hinstEAPDll = LoadLibrary(pCurExt->wszConfigUIPath);
                    if (hinstEAPDll == NULL) {
                        DEBUGMSG(ZONE_ERROR,(TEXT("!WZCUI: Couldn't load EAP extension DLL %s\r\n"),pCurExt->wszConfigUIPath));
                        break;
                    }
                    pfnEapConfig = (RASEAPINVOKECONFIGUI)GetProcAddress(hinstEAPDll,TEXT("RasEapInvokeConfigUI"));
                    pfnEapFreeMem= (RASEAPFREE)GetProcAddress(hinstEAPDll,TEXT("RasEapFreeMemory"));
                    if ((pfnEapConfig) == NULL || (pfnEapFreeMem == NULL)) {
                        DEBUGMSG(ZONE_ERROR,(TEXT("!WZCUI: Couldn't get entry points from EAP extension DLL %s\r\n"),pCurExt->wszConfigUIPath));
                        FreeLibrary(hinstEAPDll);
                        break;
                    }
                    // Call into config UI
                    __try {
                        dwError = pfnEapConfig(pCurExt->dwExtensionType,
                                               hDlg,
                                               0, // flags TBD
                                               pEapCbItemData->pbAuthData,
                                               pEapCbItemData->dwAuthDataLen,
                                               &pConnectionDataOut,
                                               &dwSizeOfConnectionDataOut);
                    } __except(EXCEPTION_EXECUTE_HANDLER) {
                          FreeLibrary(hinstEAPDll);
                          break;
                    }

                    if (dwError == ERROR_SUCCESS) {
                        if (dwSizeOfConnectionDataOut && pConnectionDataOut) {
                            
                            if (pEapCbItemData->pbAuthData) 
                                LocalFree(pEapCbItemData->pbAuthData);

                            if ((pEapCbItemData->pbAuthData = LocalAlloc(LPTR,dwSizeOfConnectionDataOut)) == NULL) {
                                FreeLibrary(hinstEAPDll);
                                break;
                            }

                            pEapCbItemData->dwAuthDataLen = dwSizeOfConnectionDataOut;

                            __try {
                                memcpy(pEapCbItemData->pbAuthData,
                                       pConnectionDataOut,dwSizeOfConnectionDataOut);
                            } __except(EXCEPTION_EXECUTE_HANDLER) {
                                  LocalFree(pEapCbItemData->pbAuthData);
                                  pEapCbItemData->pbAuthData    = NULL;
                                  pEapCbItemData->dwAuthDataLen = 0;
                            }
                            
                            __try {
                                pfnEapFreeMem(pConnectionDataOut);
                            } __except(EXCEPTION_EXECUTE_HANDLER) {
                                  ;
                            }
                        }
                    } else {
                        DEBUGMSG(ZONE_ERROR,(TEXT("!WZCUI: Error 0x%X returned from EAP extension DLL %s\r\n"),
                                             dwError,pCurExt->wszConfigUIPath));
                    }
                    FreeLibrary(hinstEAPDll);
                }
                else {
                    ASSERT(0); // control shouldn't be enabled in this case
                }

            }
            break;

        case IDC_WZC_CB_EAPTYPE:
            if (HIWORD(wParam) == CBN_SELCHANGE)
                 RefreshWZCControls(hDlg, pWCP);
            break;

        default:
            return(FALSE);
        }
        break;

    default :
        return(FALSE);
    }

	return TRUE;
}
