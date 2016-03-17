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

Abstract: Code specific to the OWNER CPL
**/

#include "cplpch.h"
#include "netui.h"
#include <assert.h>
#include <cred.h>
#include <defui.h>


#define szOwnerKey szRegistryKey	// this is defined in owner.h

void GetOwnerInfo(OWNER_PROFILE* pProfile, OWNER_NOTES* pNotes);
void SaveOwnerInfo(OWNER_PROFILE* pProfile, OWNER_NOTES* pNotes);

const TEXTLIMITS TextLimits[] = {
	IDC_EDIT_NAME1, MAX_NAME_STRING,
	IDC_EDIT_COMPANY, MAX_COMPANY_STRING,
	IDC_EDIT_ADD, MAX_ADD_STRING,
	//IDC_EDIT_WCC, MAX_CC_STRING,
	IDC_EDIT_WAC, MAX_AC_STRING,		
	IDC_EDIT_WPHONE, MAX_PHONE_STRING,
	//IDC_EDIT_HCC, MAX_CC_STRING,
	IDC_EDIT_HAC, MAX_AC_STRING,
	IDC_EDIT_HPHONE, MAX_PHONE_STRING,
};

extern "C" BOOL APIENTRY OwnerDlgProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    switch (message)
    {
	  case WM_INITDIALOG:	
	  {	
        LoadAygshellLibrary();
	    // note that in our case, lParam is the old-password (though we dont use it)
		SetTextLimits(hDlg, TextLimits, ARRAYSIZEOF(TextLimits));

        AygAddSipprefControl( hDlg );
		
		// read registry
		OWNER_PROFILE UserProfile;
		GetOwnerInfo(&UserProfile, NULL);

		// Initialize dialog
		SetWindowText(DI(IDC_EDIT_NAME1),   UserProfile.szName);
		SetWindowText(DI(IDC_EDIT_COMPANY), UserProfile.szCompany);
		SetWindowText(DI(IDC_EDIT_ADD), 	UserProfile.szAdd);
		//SetWindowText(DI(IDC_EDIT_WCC), 	UserProfile.szWCC);
		SetWindowText(DI(IDC_EDIT_WAC), 	UserProfile.szWAC);
		SetWindowText(DI(IDC_EDIT_WPHONE), 	UserProfile.szWPhone);
		//SetWindowText(DI(IDC_EDIT_HCC), 	UserProfile.szHCC);
		SetWindowText(DI(IDC_EDIT_HAC), 	UserProfile.szHAC);
		SetWindowText(DI(IDC_EDIT_HPHONE), UserProfile.szHPhone);
		Button_SetCheck(DI(IDC_CHK_ID), UserProfile.IDPowerUp);

        // Disable the IME for the phone number fields
        ImmAssociateContext(DI(IDC_EDIT_WPHONE), (HIMC)NULL);
        ImmAssociateContext(DI(IDC_EDIT_HPHONE), (HIMC)NULL);

		return 1; // let GWE set focus
	  }

      case WM_COMMAND:      
		switch (LOWORD(wParam))
        {
          case IDC_EDIT_ADD:
        	if (HIWORD(wParam) == EN_MAXTEXT) {
				MessageBeep(MB_OK);
			}
		    return TRUE;

		  case IDOK:
		  {
			OWNER_PROFILE UserProfile;
			ZEROMEM(&UserProfile);
			GetDlgItemText(hDlg, IDC_EDIT_NAME1,   UserProfile.szName,    MAX_NAME_STRING+1);
			GetDlgItemText(hDlg, IDC_EDIT_COMPANY, UserProfile.szCompany, MAX_COMPANY_STRING+1);
			GetDlgItemText(hDlg, IDC_EDIT_ADD, 	   UserProfile.szAdd,     MAX_ADD_STRING+1);
		  //GetDlgItemText(hDlg, IDC_EDIT_WCC, 	   UserProfile.szWCC,     MAX_CC_STRING+1);
			GetDlgItemText(hDlg, IDC_EDIT_WAC, 	   UserProfile.szWAC,     MAX_AC_STRING+1);
			GetDlgItemText(hDlg, IDC_EDIT_WPHONE,  UserProfile.szWPhone,  MAX_PHONE_STRING+1);
		  //GetDlgItemText(hDlg, IDC_EDIT_HCC, 	   UserProfile.szHCC,     MAX_CC_STRING+1);
			GetDlgItemText(hDlg, IDC_EDIT_HAC, 	   UserProfile.szHAC,     MAX_AC_STRING+1);
			GetDlgItemText(hDlg, IDC_EDIT_HPHONE,  UserProfile.szHPhone,  MAX_PHONE_STRING+1);

			UserProfile.IDPowerUp  = Button_GetCheck(DI(IDC_CHK_ID));

			SaveOwnerInfo(&UserProfile, NULL);
			return TRUE;
		  }
		}
        break;
        case WM_DESTROY:
            FreeAygshellLibrary();
            break;
	}
    return (FALSE);
}

extern "C" BOOL APIENTRY NotesDlgProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    switch (message)
    {
	    case WM_INITDIALOG:
	    {
			OWNER_NOTES UserNotes;
            LoadAygshellLibrary();
			GetOwnerInfo(NULL, &UserNotes);
			Edit_LimitText(DI(IDC_EDIT_NOTES), MAX_NOTES_STRING);
			Button_SetCheck(DI(IDC_CHK_NOTES), UserNotes.NotesPowerUp);
			SetWindowText(DI(IDC_EDIT_NOTES), UserNotes.Notes);
            AygAddSipprefControl( hDlg );
            return TRUE;
		}
		
	    case WM_COMMAND:      
            switch (LOWORD(wParam))
            {
				case IDC_EDIT_NOTES:
					if (HIWORD(wParam) == EN_MAXTEXT)
						MessageBeep(MB_OK);				
			        return TRUE;

				case IDOK:
				{
					OWNER_NOTES UserNotes;
					GetDlgItemText(hDlg, IDC_EDIT_NOTES, UserNotes.Notes, MAX_NOTES_STRING+1);
					UserNotes.NotesPowerUp  = Button_GetCheck(DI(IDC_CHK_NOTES));
					SaveOwnerInfo(NULL, &UserNotes);
					return TRUE;
				}
			}
            break;
        case WM_DESTROY:
            FreeAygshellLibrary();
            break;
	}
    return (FALSE);
}

DWORD SplitDomainUser(PWCHAR src, PWCHAR domain, PDWORD pDomainLen, PWCHAR user, PDWORD pUserLen){
	PWCHAR ptr = src;
	BOOL sepFound = FALSE;
	PWCHAR userStart = src;
	DWORD cchDomain = 0;
	DWORD cchUser = 0;
	
	if(NULL == src){ assert(0); return ERROR_INVALID_PARAMETER;}
	if(NULL == user || NULL == pUserLen ||0 == *pUserLen){ assert(0); return ERROR_INVALID_PARAMETER;}
	if(NULL == domain || NULL == pDomainLen ||0 == *pDomainLen){ assert(0); return ERROR_INVALID_PARAMETER;}
	
	// Is there a seperator?
	while(*ptr){
		if(*ptr == L'\\'){
			sepFound = TRUE;
			break;
		}
		ptr++;
	}

	if(sepFound){
		if(NULL == domain || NULL == pDomainLen || 0 == *pDomainLen){ assert(0); return ERROR_INVALID_PARAMETER; }
		cchDomain = ptr - src;
		if(*pDomainLen < cchDomain+1){ return ERROR_INSUFFICIENT_BUFFER; }
		memcpy(domain,src,cchDomain*sizeof(WCHAR));
		domain[cchDomain] = L'\0';
		userStart = ptr+1;
	}else{
		if(*pDomainLen < 1){ return ERROR_INSUFFICIENT_BUFFER; }
		domain[0] = L'\0';
		cchDomain = 0;
	}
	
	while(*ptr){
		ptr++;
	}

	cchUser = ptr - userStart;
	if(*pUserLen < cchUser + 1){ return ERROR_INSUFFICIENT_BUFFER; }
	memcpy(user,userStart,cchUser*sizeof(WCHAR));
	user[cchUser] = L'\0';

	*pDomainLen = cchDomain;
	*pUserLen = cchUser;

	return ERROR_SUCCESS;
}

DWORD MakeDomainUser(PWCHAR dest, PDWORD pDestLen, PWCHAR domain, DWORD domainLen, PWCHAR user, DWORD userLen){
    PWCHAR temp = dest;
    DWORD requiredSize = 0;
    
    if(NULL == dest || NULL == pDestLen || 0 == *pDestLen){ assert(0); return ERROR_INVALID_PARAMETER; }

    if(NULL == user || 0 == userLen){
        dest[0] = L'\0';
        *pDestLen = 0;
        return ERROR_SUCCESS;
    }

    requiredSize = domainLen;

    assert(0 != userLen);
    if(requiredSize + userLen <= requiredSize){
        assert(0 && "overflow");
        return ERROR_INTERNAL_ERROR;
    }
    requiredSize += userLen;

    if(requiredSize + 2 <= requiredSize){
        assert(0 && "overflow");
        return ERROR_INTERNAL_ERROR;
    }
    requiredSize += 2;
        
    if(*pDestLen < requiredSize){ assert(0); return ERROR_INSUFFICIENT_BUFFER;}

    if(domain && domainLen){
        memcpy(temp,domain,domainLen*sizeof(WCHAR));
        temp[domainLen] = L'\\';
        temp += domainLen + 1;
    }

    if (*pDestLen - (temp - dest) < userLen + 1)
    {
        assert(0 && "overflow");
        return ERROR_INTERNAL_ERROR;
    }
    
    memcpy(temp,user,userLen*sizeof(WCHAR));

    temp[userLen] = L'\0';
    temp += userLen;

    *pDestLen = temp - dest;

    return ERROR_SUCCESS;
}

DWORD CachedCredentialDomainUser(PWCHAR domain, PDWORD pDomainLen, PWCHAR user, PDWORD pUserLen){
	PCRED pCred = NULL;
	DWORD dwErr = ERROR_INTERNAL_ERROR;
	WCHAR defTarget[] = L"";
	DWORD defTargetLen = sizeof(defTarget)/sizeof(defTarget[0]);
	PWCHAR domainuser;

	// Read the domain credential corresponding to blank target
	dwErr = CredRead(defTarget,
						defTargetLen,
						CRED_TYPE_DOMAIN_PASSWORD,
						CRED_FLAG_NO_DEFAULT|CRED_FLAG_NO_IMPLICIT_DEFAULT,
						&pCred);

	if(ERROR_SUCCESS == dwErr && NULL != pCred){
		domainuser = pCred->wszUser;
	}else{
		domainuser = L"";
	}

	dwErr = SplitDomainUser(domainuser, domain, pDomainLen, user, pUserLen);						

	if(pCred){
		CredFree((PBYTE)pCred);
		pCred = NULL;
	}

	return dwErr;	
}


DWORD CacheDomainCredential(const PWCHAR domain, DWORD domainLen, const PWCHAR user,  DWORD userLen, const PWCHAR pass, DWORD passLen){

	if((domainLen > DNLEN) || (userLen > UNLEN)){ assert(0); return ERROR_INVALID_PARAMETER; }

	CRED cred;
	WCHAR defTarget[] = L"";
	DWORD defTargetLen = sizeof(defTarget)/sizeof(defTarget[0]);
	WCHAR userName[UNLEN+DNLEN+2];
	DWORD userNameLen = UNLEN+DNLEN+2;
	DWORD dwErr = ERROR_INTERNAL_ERROR;

	if(0 == userLen){
		if(user){
			userLen = wcslen(user);
		}	
	}
	
	if(0 == domainLen){
		if(domain){
			domainLen = wcslen(domain);
		}
	}


	if(0 == passLen){
		if(pass){
			passLen = wcslen(pass);
		}
	}
		
    dwErr = MakeDomainUser(userName,
                           &userNameLen,
                           domain,
                           domainLen,
                           user,
                           userLen);
	if(ERROR_SUCCESS != dwErr){
		return dwErr;
	}

	// Set up the credential
	cred.dwVersion = CRED_VER_1;
	cred.dwType = CRED_TYPE_DOMAIN_PASSWORD;
	cred.wszTarget = defTarget;
	cred.dwTargetLen = defTargetLen;
	cred.dwFlags = CRED_FLAG_DEFAULT|CRED_FLAG_PERSIST;
	cred.wszUser = userName;
	cred.dwUserLen = userNameLen+1;
	cred.pBlob = (PBYTE)(pass);
	cred.dwBlobSize = pass?((passLen+1)*sizeof(WCHAR)):0;

	// Write the credential
	dwErr = CredWrite(&cred,0);

	return dwErr;
}

BOOL DisallowSavedPasswords()
{
    CReg reg;
    BOOL fDisallow = 0;
    // TODO: disallowing saved password shoudl be the default
     if ( reg.Open(HKEY_LOCAL_MACHINE, L"Comm\\Security"))
           fDisallow = reg.ValueDW(L"DisallowSavedNetworkPasswords");
    return fDisallow;
}

extern "C" BOOL CALLBACK
NetIdentDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static BOOL fPasswordChanged=FALSE, fUsernameChanged=FALSE, fDomainChanged=FALSE ;
    static BOOL fDisallowPassword = FALSE;
    
    //static BOOL fWordCompletion = FALSE;  WordCompletion is not supported yet
    CReg reg;

	TCHAR		szUser[UNLEN+1]=L"";
	TCHAR		szDomain[DNLEN+1]=L"";
	TCHAR		szPass[PWLEN+1]=L"";
	DWORD cchDomain=DNLEN+1, cchUser=UNLEN+1,cchPassword=PWLEN+1;
	DWORD dwErr = ERROR_INTERNAL_ERROR;
	WCHAR defTarget[] = L"";
	DWORD defTargetLen = sizeof(defTarget)/sizeof(defTarget[0]);
	
	DEBUGMSG (ZONE_MSGS, (L"+NetworkIdentDlgProc(0x%X, 0x%X, 0x%X, 0x%X)\r\n", hDlg, message, wParam, lParam));
	switch (message) 
	{
	  case WM_INITDIALOG :
		DEBUGMSG(ZONE_NETWORK, (L"NetIdentDlgProc: Got WM_INITDLG....\r\n"));
        LoadAygshellLibrary();
		Edit_LimitText (DI(IDC_NETCPL1_USERNAME), UNLEN);
		Edit_LimitText (DI(IDC_NETCPL1_DOMAIN), DNLEN);
		Edit_LimitText (DI(IDC_NETCPL1_PASSWORD), PWLEN);

		// turn word completion OFF if enabled. Save prev state
		//fWordCompletion = SetWordCompletion(FALSE);  

		CachedCredentialDomainUser(szDomain,&cchDomain,szUser,&cchUser);
		
              fDisallowPassword = DisallowSavedPasswords();

              if (fDisallowPassword)
              {
                    EnableWindow(GetDlgItem(hDlg, IDC_NETCPL1_PASSWORD), FALSE);
              }
		SetDlgItemText (hDlg, IDC_NETCPL1_USERNAME, szUser);
		SetDlgItemText (hDlg, IDC_NETCPL1_DOMAIN, szDomain);
		fPasswordChanged = FALSE;
		fUsernameChanged = FALSE;
		fDomainChanged = FALSE;
        AygAddSipprefControl( hDlg );
		return 1; // ask GWE to set focus

	case WM_COMMAND:
    	switch (LOWORD(wParam))
		{
        case IDC_NETCPL1_PASSWORD:
            if (HIWORD(wParam) == EN_CHANGE)
            	fPasswordChanged = TRUE;
            break;
        case IDC_NETCPL1_DOMAIN:
            if (HIWORD(wParam) == EN_CHANGE)
            	fDomainChanged = TRUE;
            break;
        case IDC_NETCPL1_USERNAME:
            if (HIWORD(wParam) == EN_CHANGE)
            	fUsernameChanged = TRUE;
            break;
		
	  	case IDOK:

			// Save the username & domain as the default credential
			cchUser = GetDlgItemText (hDlg, IDC_NETCPL1_USERNAME, szUser, UNLEN);
			szUser[UNLEN] = L'\0';
			cchDomain = GetDlgItemText (hDlg, IDC_NETCPL1_DOMAIN, szDomain, DNLEN);
			szDomain[DNLEN] = L'\0';
			cchPassword = GetDlgItemText(hDlg, IDC_NETCPL1_PASSWORD, szPass, PWLEN);
			szPass[PWLEN] = L'\0';

			if(0 == cchUser){
				//Username is blank
				//Delete the existing domain credential
				CredDelete(defTarget,
								defTargetLen,
								CRED_TYPE_DOMAIN_PASSWORD,
								0);
			}else if ((fUsernameChanged || fPasswordChanged || fDomainChanged)){
				// Write new credential if there are any changes			
				CacheDomainCredential(szDomain,
										cchDomain,
										szUser,
										cchUser,
										szPass,
										cchPassword);
			}

			if(fDomainChanged){
	                    // set the device 'default' domain to the same as the user domain
	                    //  these domains can be different if desired
	                    CReg reg;
	                    reg.Open(HKEY_LOCAL_MACHINE, RK_IDENT);
	                    reg.SetSZ(RV_DOMAIN, szDomain,  DNLEN +1);			
			}

		      SecureZeroMemory(szPass,sizeof(szPass));
	
			//if(fWordCompletion) SetWordCompletion(TRUE); 
			return TRUE;
		}
		break;
        case WM_DESTROY:
            FreeAygshellLibrary();
            break;
	}
	DEBUGMSG (ZONE_MSGS, (L"-NetworkIdentDlgProc: Return FALSE\r\n"));
	return FALSE; // message was not handled
}



void GetOwnerInfo(OWNER_PROFILE* pProfile, OWNER_NOTES* pNotes)
{
	CReg reg(HKEY_CURRENT_USER, szOwnerKey);

	if(pProfile)
	{
		ZEROMEM(pProfile);
		if(sizeof(OWNER_PROFILE) != reg.ValueBinary(szOwner, (LPBYTE)pProfile, sizeof(OWNER_PROFILE))) {
			DEBUGMSG(ZONE_REG, (L"Failed to read Owner profile from registry\r\n"));
		}
	}
	if(pNotes)
	{
		ZEROMEM(pNotes);
		if(sizeof(OWNER_NOTES) != reg.ValueBinary(szNotes, (LPBYTE)pNotes, sizeof(OWNER_NOTES))) {
			DEBUGMSG(ZONE_REG, (L"Failed to read Owner notes from registry\r\n"));
		}
	}
}

void SaveOwnerInfo(OWNER_PROFILE* pProfile, OWNER_NOTES* pNotes)
{
	CReg reg;
	reg.Create(HKEY_CURRENT_USER, szOwnerKey);

	if(pProfile)
	{
		if(!reg.SetBinary(szOwner, (LPBYTE)pProfile, sizeof(OWNER_PROFILE))) {
			DEBUGMSG(ZONE_REG, (L"Failed to save Owner profile to registry\r\n"));
		}
	}
	if(pNotes)
	{
		if(!reg.SetBinary(szNotes, (LPBYTE)pNotes, sizeof(OWNER_NOTES))) {
			DEBUGMSG(ZONE_REG, (L"Failed to save Owner notes to registry\r\n"));
		}
	}
}

