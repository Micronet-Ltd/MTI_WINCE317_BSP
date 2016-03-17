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

getuser.c

Abstract:

Functions:


Notes:


--*/

#include <windows.h>
#include <types.h>
#include <assert.h>
#include <netui.h>
#include <netuip.h>
#include <resource.h>
#include <sspi.h>
#include <cred.h>

BOOL DisallowSavedPasswords()
{
    DWORD dwError;
    HKEY hRegKey;
    DWORD dwType;
    DWORD fDisallow = 0;
    DWORD dwSize;
    if ( ERROR_SUCCESS == ( dwError = RegOpenKeyEx (
                               HKEY_LOCAL_MACHINE,
                               L"Comm\\Security",
                               0,
                               KEY_READ,
                               &hRegKey ) ) )
    {
        dwSize = sizeof(fDisallow);
        dwError = RegQueryValueExW(
                               hRegKey,
                               L"DisallowSavedNetworkPasswords",
                               NULL,
                               &dwType,
                               (PBYTE)&fDisallow,
                               &dwSize );
        if (ERROR_SUCCESS == dwError)
            DEBUGMSG (ZONE_USERPWD, (TEXT("DlgProcUserPwd: DisallowSavedNetworkPasswords = %d\r\n"), fDisallow));
        RegCloseKey(hRegKey);
    }
    return fDisallow;
}

typedef const struct
{
    BOOL (*InitDialog)(HWND hDlg, PVOID pCallbackContext);
    BOOL (*OKDialog)(HWND hDlg,   PVOID pCallbackContext);
} USER_PASSWORD_DIALOG_CALLBACK_TABLE, *PUSER_PASSWORD_DIALOG_CALLBACK_TABLE;

typedef struct
{
    PUSER_PASSWORD_DIALOG_CALLBACK_TABLE pCallbacks;
    PVOID                                pCallbackContext;
    HWND                                 *phDlg;
} USER_PASSWORD_DIALOG_CONTEXT, *PUSER_PASSWORD_DIALOG_CONTEXT;

// ----------------------------------------------------------------
//
//  Typically, the dialog box procedure should return TRUE if it processed the message,
//  and FALSE if it did not. If the dialog box procedure returns FALSE, the dialog manager
//  performs the default dialog operation in response to the message.
//
// ----------------------------------------------------------------
LRESULT CALLBACK
CommonUserPasswordDialogFunc(
    IN     HWND   hDlg,
    IN     UINT   message,
    IN     WPARAM wParam,
    IN OUT LPARAM lParam)
//
//  Function to process messages received by one of the username/password dialogs, do
//  standard processing and perform callbacks to dialog specific handlers as appropriate.
//
{
    PUSER_PASSWORD_DIALOG_CONTEXT pContext;
    BOOL                          bProcessedMessage = FALSE;

    switch( message )
    {
    case WM_INITDIALOG:
        DEBUGMSG (ZONE_USERPWD, (TEXT("NETUI: UserPasswordDialogFunc: INITDIALOG\r\n")));

        pContext = (PUSER_PASSWORD_DIALOG_CONTEXT)lParam;
        ASSERT(NULL != pContext);

        // Return our dialog window handle if requested
        if (pContext->phDlg)
            *pContext->phDlg = hDlg;

        SetWindowLong (hDlg, DWL_USER, (LONG)pContext);
        SetWindowLong (hDlg,GWL_EXSTYLE,GetWindowLong(hDlg,GWL_EXSTYLE)|WS_EX_CAPTIONOKBTN|WS_EX_TOPMOST);
        bProcessedMessage = pContext->pCallbacks->InitDialog(hDlg, pContext->pCallbackContext);
        break;

    case WM_COMMAND:
        pContext = (PUSER_PASSWORD_DIALOG_CONTEXT)GetWindowLong (hDlg,  DWL_USER);
        ASSERT(NULL != pContext);
        switch( LOWORD( wParam ) )
        {
        case IDCANCEL:              // Ignore values
            DEBUGMSG (ZONE_USERPWD, (TEXT("NETUI: UserPasswordDialogFunc: IDCANCEL\r\n")));
            EndDialog(hDlg, 0);
            break;

        case IDOK:
            // Get the dialog values
            DEBUGMSG (ZONE_USERPWD, (TEXT("NETUI: UserPasswordDialogFunc: IDOK\r\n")));
            if (pContext)
                bProcessedMessage = pContext->pCallbacks->OKDialog(hDlg, pContext->pCallbackContext);
            else
            {
                DEBUGMSG (ZONE_ERROR,   (TEXT("NETUI: UserPasswordDialogFunc: ERROR - DWL_USER NULL in IDOK\r\n")));
                EndDialog(hDlg, 0);
            }
            break;
        }

    case WM_DESTROY:
        DEBUGMSG (ZONE_USERPWD, (TEXT("NETUI: UserPasswordDialogFunc: DESTROY\r\n")));
        break;

    default:
        break;
    }

    return bProcessedMessage;
}

BOOL
CommonUserPasswordDialogBox(
    IN       UINT                                  idDialogWide,
    IN       UINT                                  idDialogPortrait,
    IN       HWND                                  hParent,
    IN const PUSER_PASSWORD_DIALOG_CALLBACK_TABLE  pCallbacks,
    IN OUT   PVOID                                 pDialogContext,
       OUT   HWND  OPTIONAL                        *phDlg)
//
//  This function is called to display one of the username/password dialog boxes.
//
{
    int     iRet;
    BOOL    fPortrait = GetSystemMetrics(SM_CXSCREEN) < 480;
    int     idd = fPortrait ? idDialogPortrait:idDialogWide;
    HRSRC   hRsrc;
    HGLOBAL hGlob;
    HCURSOR hCur;
    BOOL    bGotData = FALSE;
    LPTSTR  tszDialogResource;
    USER_PASSWORD_DIALOG_CONTEXT context;

    context.pCallbacks = pCallbacks;
    context.pCallbackContext = pDialogContext;
    context.phDlg = phDlg;

    DEBUGMSG (ZONE_USERPWD, (TEXT("NETUI: +CommonUserPasswordDialogBox\n")));

    tszDialogResource = MAKEINTRESOURCE(idd);
    hRsrc  = FindResource(v_hInst, tszDialogResource, RT_DIALOG);
    if (hRsrc)
    {
        hGlob = LoadResource (v_hInst, hRsrc);
        if (hGlob)
        {
            // Load security lib so we can obtain default username?
            //LoadSecurityLib();

            hCur = SetCursor(NULL);
            iRet = DialogBoxParam (v_hInst, tszDialogResource, hParent, CommonUserPasswordDialogFunc, (LONG)&context);

            // If the DialogBoxParam succeeds, the return value is the value of the nResult
            // parameter specified in the call to the EndDialog function used to terminate the dialog box.
            // If the function fails because the hWndParent parameter is invalid, the return value is zero.
            // The function returns zero in this case for compatibility with previous versions of Microsoft(R) Windows(R).
            // If the function fails for any other reason, the return value is -1. To get extended error information,
            // call GetLastError.

            if (iRet == -1)
                DEBUGMSG(ZONE_ERROR, (TEXT("NETUI: CommonUserPasswordDialogBox DialogBoxParam returned err %u\n"), GetLastError()));
            else if (iRet == 0)
                // Could be an error or could be that the user cancelled the dialog
                DEBUGMSG(ZONE_USERPWD, (TEXT("NETUI: CommonUserPasswordDialogBox DialogBoxParam returned 0\n")));
            else
                bGotData = TRUE;

            SetCursor(hCur);

            //UnloadSecurityLib();
        }
    }

    DEBUGMSG (ZONE_USERPWD, (TEXT("NETUI: -CommonUserPasswordDialogBox bGotData=%d\n"), bGotData));

    return bGotData;
}

DWORD SplitDomainUser(PWCHAR src, PWCHAR domain, PDWORD pDomainLen, PWCHAR user, PDWORD pUserLen){
    PWCHAR ptr = src;
    BOOL sepFound = FALSE;
    PWCHAR userStart = src;
    DWORD cchDomain = 0;
    DWORD cchUser = 0;

    // Is there a seperator?
    while(*ptr){
        if(*ptr == L'\\'){
            sepFound = TRUE;
            break;
        }
        ptr++;
    }

    if(sepFound){
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

// Make a string of the form domain\user from the input
// If user is null/empty then empty string is returned
// If domain is null/empty then just user is returned
// On input pDestLen contains length of dest buffer
// On output it contains length of output string

DWORD MakeDomainUser(PWCHAR dest, PDWORD pDestLen, PWCHAR domain, DWORD domainLen, PWCHAR user, DWORD userLen){
    PWCHAR temp = dest;
    DWORD requiredSize = 0;

    // If no username then return blank username
    if(NULL == user || 0 == userLen){
        dest[0] = L'\0';
        *pDestLen = 0;
        return ERROR_SUCCESS;
    }

    requiredSize = domainLen;

    // would adding userLen result in overflow?
    if(requiredSize + userLen < requiredSize){
        assert(0); //overflow
        return ERROR_INTERNAL_ERROR;
    }

    requiredSize += userLen;

    // would adding space for slash and end null result in overflow?
    if(requiredSize + 2 < requiredSize){
        assert(0); //overflow
        return ERROR_INTERNAL_ERROR;
    }

    requiredSize += 2;

    if(*pDestLen < requiredSize){ return ERROR_INSUFFICIENT_BUFFER;}

    // if we have a domain name
    if(domain && domainLen){
        memcpy(temp,domain,domainLen*sizeof(WCHAR));
        temp[domainLen] = L'\\';
        temp += domainLen + 1;
    }

    memcpy(temp,user,userLen*sizeof(WCHAR));

    temp[userLen] = L'\0';
    temp += userLen;

    *pDestLen = temp - dest;

    return ERROR_SUCCESS;
}

// Get the cached domain credential
// First we see if we can find a credential for the explicit target
// If not then try for target being blank L""
// pFoundMatch is set to TRUE if exact target match is found
// On input pDestLen contains length of dest buffer
// On output it contains length of output string

DWORD CachedCredentialDomainUser(PWCHAR target, DWORD targetLen, PWCHAR domain, PDWORD pDomainLen, PWCHAR user, PDWORD pUserLen, PBOOL pFoundMatch){
    PCRED pCred = NULL;
    DWORD dwErr = ERROR_INTERNAL_ERROR;
    PWCHAR domainuser;

    // Read the domain credential corresponding to the target
    dwErr = CredRead(target,
                        targetLen+1,
                        CRED_TYPE_DOMAIN_PASSWORD,
                        CRED_FLAG_NO_DEFAULT|CRED_FLAG_NO_IMPLICIT_DEFAULT,
                        &pCred);

    if(ERROR_SUCCESS == dwErr && NULL != pCred){
        // read the domain/user for the cached cred
        *pFoundMatch = TRUE;
        domainuser = pCred->wszUser;
    }else{
        *pFoundMatch = FALSE;
        // Is there a default credential
        dwErr = CredRead(L"",
                            1,
                            CRED_TYPE_DOMAIN_PASSWORD,
                            CRED_FLAG_NO_DEFAULT|CRED_FLAG_NO_IMPLICIT_DEFAULT,
                            &pCred);
        if(ERROR_SUCCESS == dwErr && NULL != pCred){
            // read the domain/user for the cached cred
            domainuser = pCred->wszUser;
        }else{
            // no cached creds
            domainuser = L"";
        }
    }

    dwErr = SplitDomainUser(domainuser, domain, pDomainLen, user, pUserLen);

    if(pCred){
        CredFree((PBYTE)pCred);
        pCred = NULL;
    }

    return dwErr;
}

// Cache a credential for the specified target
// The credential cached is of type CRED_TYPE_DOMAIN_PASSWORD
// The credential is persisted

DWORD CacheDomainCredential(PWCHAR target, DWORD targetLen, PWCHAR domain, DWORD domainLen, PWCHAR user,  DWORD userLen, PWCHAR pass, DWORD passLen){
    CRED cred;
    WCHAR userName[UNLEN+DNLEN+2];
    DWORD userNameLen = UNLEN+DNLEN+2;
    DWORD dwErr = ERROR_INTERNAL_ERROR;

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
    cred.wszTarget = target;
    cred.dwTargetLen = targetLen+1;
    cred.dwFlags = CRED_FLAG_DEFAULT|CRED_FLAG_PERSIST;
    cred.wszUser = userName;
    cred.dwUserLen = userNameLen+1;
    cred.pBlob = (PBYTE)(pass);
    cred.dwBlobSize = pass?((passLen+1)*sizeof(WCHAR)):0;

    // Write the credential
    dwErr = CredWrite(&cred,0);

    return dwErr;
}


// Safetly copy from src to dst
// len includes total space avail in dst including space for null
// on return contains len of string
// string is always null terminated

void SafeCopy(PWCHAR src, PWCHAR dst, PDWORD len){
    PWCHAR t = NULL;
    DWORD i = 0;

    PREFAST_ASSERT(src != NULL);
    PREFAST_ASSERT(dst != NULL);
    PREFAST_ASSERT(len != NULL);
    assert(*len >= 1);

    t = src;
    i = 0;
    while(*t != L'\0' && i < *len-1){
        dst[i] = *t;
        t++;
        i++;
    }

    dst[i] = L'\0';

    *len = i;

    return;
}


// Parse resource name of form \\server\resource into server
// on input len contains size of server buffer in chars
// on output len contains lenght of server string
// if server buffer is not big enough a truncated string is returned
// output string is always null terminated

void GetServerFromResource(PWCHAR resource, PWCHAR server, PDWORD len){
    PWCHAR t = NULL;
    DWORD i = 0;

    PREFAST_ASSERT(resource != NULL);
    PREFAST_ASSERT(server != NULL);
    PREFAST_ASSERT(len != NULL);
    assert(*len >= 1);

    // skip initial slashes
    t = resource;
    while(*t == L'\\' && *t != L'\0'){
        //skip
        t++;
    }

    if(*t == L'\0'){
        // we reached the end. so input string has all slashes.
        // output string is empty
        *server = L'\0';
        *len = 0;
        return;
    }

    // we are looking at first non slash char
    // copy all characters until another slash is encountered
    // or end of source string
    // or end of space in dest string
    i = 0;
    while(*t != L'\\' && *t != L'\0' && i < *len-1){
        server[i] = *t;
        t++;
        i++;
    }

    server[i] = L'\0';
    *len = i;

    return;
}

BOOL
GetUsernamePasswordInitDialog(
    IN HWND  hDlg,
    IN PVOID pCallbackContext)
//
//  Username/Password dialog initialization handler.
//
{
    PNETUI_USERPWD       pUserPwd = (PNETUI_USERPWD)pCallbackContext;
    BOOL                 fDisallowSavedPasswords;
    HWND                 hCtrl;
    DWORD                dwStyle;
    DWORD cchDomain = 0;
    DWORD cchUser = 0;
    BOOL bFoundMatch = FALSE;
    WCHAR server[RMLEN] = L"";
    DWORD serverLen = RMLEN;

    PREFAST_ASSERT(pUserPwd->szResourceName != NULL);

    GetServerFromResource(pUserPwd->szResourceName,server,&serverLen);

    // For connecting to net server, we modify the dialog a little
    if(pUserPwd->dwFlags & NETUI_USERPWD_WININET)
    {
        //  IE like auth dialogs
        UINT    uID = 0;
        TCHAR   StrBuf[100];
        DWORD   StrBufLen   = sizeof(StrBuf)/sizeof(StrBuf[0]);

        if (LoadString(v_hInst,IDS_WININET_TITLE,StrBuf,StrBufLen))
                SetWindowText(hDlg,StrBuf);

        ShowWindow(GetDlgItem(hDlg,IDC_SITE_OR_FIREWALL),SW_SHOW);
        uID = pUserPwd->dwFlags & NETUI_USERPWD_WININETPROXY? IDS_WININET_PROXY:IDS_WININET_SITE;
        if(LoadString(v_hInst,uID,StrBuf,StrBufLen))
            SetDlgItemText(hDlg,IDC_SITE_OR_FIREWALL,StrBuf);

        ShowWindow(GetDlgItem(hDlg,IDC_SERVER_OR_PROXY),SW_SHOW);
        SetDlgItemText(hDlg,IDC_SERVER_OR_PROXY,pUserPwd->szResourceName);

        if(pUserPwd->dwFlags & NETUI_USERPWD_REALM)
            SetDlgItemText(hDlg,IDC_REALM_FIELD,pUserPwd->szRealm);
    }
    else if(pUserPwd->dwFlags&NETUI_USERPWD_SERVERLOGON)
    {
        TCHAR StrBuf[100], ResFmtBuf[RMLEN + 30];
        DWORD StrBufLen = sizeof(StrBuf)/sizeof(StrBuf[0]);
        // Change window title
        if (pUserPwd->dwFlags & NETUI_USERPWD_AUTH_FAIL)
        {
            if (LoadString(v_hInst,IDS_SRVLOGON_AUTH_FAIL_TITLE,StrBuf,StrBufLen))
                SetWindowText(hDlg,StrBuf);
        }
        else
        {
            if (LoadString(v_hInst,IDS_SRVLOGON_TITLE,StrBuf,StrBufLen))
                SetWindowText(hDlg,StrBuf);
        }

        // Display resource we are connecting to
        if (LoadString(v_hInst,IDS_RESFMT_STR,StrBuf,StrBufLen))
        {
            wsprintf(ResFmtBuf,StrBuf,pUserPwd->szResourceName);
            SetDlgItemText(hDlg,IDC_CMDTEXT,ResFmtBuf);
        }
        // Check box has different caption
        if (LoadString(v_hInst,IDS_SRVLOGON_CHK_STR,StrBuf,StrBufLen))
            SetDlgItemText(hDlg,IDC_SAVE_PASSWORD,StrBuf);
    }
    SetWindowLong (hDlg,GWL_EXSTYLE,GetWindowLong(hDlg,GWL_EXSTYLE)|WS_EX_CAPTIONOKBTN|WS_EX_TOPMOST);

    // For non-realm cases
    if(!(pUserPwd->dwFlags & NETUI_USERPWD_REALM)){
        // if the caller hasn't supplied a user name and password and there is a system default, use that.
        if(pUserPwd->szUserName[0] == L'\0' && pUserPwd->szDomain[0] == L'\0'){
            cchDomain = DNLEN + 1;
            cchUser = UNLEN + 1;
            CachedCredentialDomainUser(server,serverLen,pUserPwd->szDomain,&cchDomain,pUserPwd->szUserName,&cchUser,&bFoundMatch);
        }
    }

    SetDlgItemText( hDlg, IDC_USERNAME, pUserPwd->szUserName );
    SetDlgItemText( hDlg, IDC_PASSWORD, pUserPwd->szPassword );
    if(!(pUserPwd->dwFlags & NETUI_USERPWD_REALM))
        SetDlgItemText( hDlg, IDC_DOMAIN, pUserPwd->szDomain );

    if (pUserPwd->szUserName[0] == TEXT('\0') || (pUserPwd->dwFlags & NETUI_USERPWD_NOPWD))
        SetFocus(GetDlgItem(hDlg, IDC_USERNAME));
    else
    {
        SetFocus(GetDlgItem(hDlg, IDC_PASSWORD));
        // Select the password text
        SendMessage(GetDlgItem(hDlg, IDC_PASSWORD), EM_SETSEL, 0, -1);
    }

    fDisallowSavedPasswords = DisallowSavedPasswords();
    // clear the saved password flag based on system policy
    if (fDisallowSavedPasswords)
        pUserPwd->dwFlags &= ~ NETUI_USERPWD_SAVEPWD;

    if ((pUserPwd->dwFlags & NETUI_USERPWD_SHOW_SAVEPWD) && !fDisallowSavedPasswords)
    {
        if (pUserPwd->dwFlags & NETUI_USERPWD_SAVEPWD)
            // Check the save password box
            SendMessage(GetDlgItem(hDlg, IDC_SAVE_PASSWORD),BM_SETCHECK,1,0);

        // Make sure it's visible
        hCtrl = GetDlgItem(hDlg,IDC_SAVE_PASSWORD);
        dwStyle = GetWindowLong(hCtrl, GWL_STYLE)|WS_VISIBLE;
        SetWindowLong (hCtrl,GWL_STYLE,dwStyle);
    }
    else
    {
        // Hide the SAVEPWD box.
        hCtrl = GetDlgItem (hDlg, IDC_SAVE_PASSWORD);
        EnableWindow(hCtrl, FALSE);
    }

    return FALSE;
}

BOOL
GetUsernamePasswordOKDialog(
    IN HWND  hDlg,
    IN PVOID pCallbackContext)
//
//  Username/Password dialog OK button handler.
//
//  Retrieve the values from the dialog controls, save them,
//  and close the dialog.
//
{
    PNETUI_USERPWD       pUserPwd = (PNETUI_USERPWD)pCallbackContext;
    UINT                 cchUser=0;
    UINT                 cchDomain=0;
    UINT           cchPassword=0;
    BOOL bFoundMatch = FALSE;
    WCHAR server[RMLEN] = L"";
    DWORD serverLen = RMLEN;

    PREFAST_ASSERT(pUserPwd->szResourceName != NULL);

    GetServerFromResource(pUserPwd->szResourceName,server,&serverLen);

    // Get the dialog values

    cchUser = GetDlgItemText( hDlg, IDC_USERNAME,pUserPwd->szUserName,UNLEN+1);
    cchPassword = GetDlgItemText(   hDlg, IDC_PASSWORD,pUserPwd->szPassword,PWLEN+1);
    if(!(pUserPwd->dwFlags & NETUI_USERPWD_REALM))
        cchDomain = GetDlgItemText( hDlg, IDC_DOMAIN,pUserPwd->szDomain,DNLEN+1 );

    if (SendMessage(GetDlgItem(hDlg, IDC_SAVE_PASSWORD),BM_GETCHECK, 0, 0)){
        pUserPwd->dwFlags |= NETUI_USERPWD_SAVEPWD;
        if (pUserPwd->szUserName[0]){
            // Cache the credential if there is a username and the user wants to save
            // and this is non-realm
            if(!(pUserPwd->dwFlags & NETUI_USERPWD_REALM)){
                // Cache the cred for the specific target
                CacheDomainCredential(server,
                                        serverLen,
                                        pUserPwd->szDomain,
                                        cchDomain,
                                        pUserPwd->szUserName,
                                        cchUser,
                                        pUserPwd->szPassword,
                                        cchPassword);

                // Update the default cred.
                // Default cred reflects last written target.
                CacheDomainCredential(L"",
                                        0,
                                        pUserPwd->szDomain,
                                        cchDomain,
                                        pUserPwd->szUserName,
                                        cchUser,
                                        pUserPwd->szPassword,
                                        cchPassword);

            }
        }
    }else{
        if(!(pUserPwd->dwFlags & NETUI_USERPWD_REALM)){
            if(pUserPwd->szUserName[0] != L'\0'){
                // Save the cred for the target but without the password
                CacheDomainCredential(server,
                                        serverLen,
                                        pUserPwd->szDomain,
                                        cchDomain,
                                        pUserPwd->szUserName,
                                        cchUser,
                                        NULL,
                                        0);
            }

        }
        pUserPwd->dwFlags &= ~(NETUI_USERPWD_SAVEPWD);
    }

    EndDialog( hDlg, TRUE );

    return TRUE;
}

USER_PASSWORD_DIALOG_CALLBACK_TABLE GetUsernamePasswordCallbacks =
{
    GetUsernamePasswordInitDialog,
    GetUsernamePasswordOKDialog
};

// ----------------------------------------------------------------
//
//  Username Password dialog box.
//
//  Called by PPP to prompt for the username on a password
//  failure. Also used by redir to logon to a network server.
//
// ----------------------------------------------------------------
BOOL
GetUsernamePasswordEx(
    IN      HWND           hParent,
    IN  OUT PNETUI_USERPWD pUserPwd,
        OUT HWND           *phDlg)
//
//  After calling this function, but before it returns, the value in *phDlg may be
//  passed to CloseUsernamePasswordDialog to abort the attempt to get the new password.
//
{
    BOOL    bGotData;
    UINT    idPortrait = IDD_NTLM_AUTH_N;
    UINT    idWide     = IDD_NTLM_AUTH_W;
    SetLastError(ERROR_SUCCESS);

    // verify the incoming window handle and fail is not null and invalid
    if (hParent && !IsWindow(hParent)) {
        SetLastError(ERROR_INVALID_WINDOW_HANDLE);
        return FALSE;
    }

    if (pUserPwd->dwFlags & NETUI_USERPWD_NOPWD) {
        idPortrait  = IDD_USERNAME;
        idWide      = IDD_USERNAME;
    }
    else if (pUserPwd->dwFlags & NETUI_USERPWD_REALM) {
        idPortrait  = IDD_REALM_AUTH_N;
        idWide      = IDD_REALM_AUTH_W;
    }

    bGotData = CommonUserPasswordDialogBox(
                            idWide,
                            idPortrait,
                            hParent,
                            &GetUsernamePasswordCallbacks,
                            pUserPwd,
                            phDlg);

    return bGotData;
}

BOOL
GetUsernamePassword(
    IN     HWND           hParent,
    IN OUT PNETUI_USERPWD pUserPwd)
{
    return GetUsernamePasswordEx(hParent, pUserPwd, NULL);
}

int
MessageBoxUsingIds(
    HWND hWnd,
    UINT uIDText,
    UINT uIDCaption,
    UINT uType)
//
//  Load the string resources for the specified IDs and display a message box.
//
{
    int   result;
    TCHAR szText[256];
    TCHAR szCaption[256];

    LoadStringW(v_hInst, uIDText,    szText,    ARRAYSIZE(szText));
    LoadStringW(v_hInst, uIDCaption, szCaption, ARRAYSIZE(szCaption));

    result = MessageBox(hWnd, szText, szCaption, uType);
    return result;
}

BOOL
GetNewPasswordInitDialog(
    IN HWND  hDlg,
    IN PVOID pCallbackContext)
//
//  Change Password dialog initialization handler.
//
//  Called to display a dialog box with the following fields:
//      Username
//      DomainName
//      OldPassword
//      NewPassword
//      ConfirmNewPassword
//
//  The Username and DomainName fields will be initialized when the dialog is created.
//
{
    PNETUI_NEWPWD     pNewPwd = (PNETUI_NEWPWD)pCallbackContext;

    SetFocus(GetDlgItem(hDlg, IDC_NEW_PASSWORD));

    return TRUE;
}

//
// Function to zero out sensitive information in cases where memset() might
// be optimized away by the compiler.  For example, key/password material in
// a local var might be zeroed to prevent later retrieval off the stack.
//

__inline
PVOID
MyRtlSecureZeroMemory(
    IN PVOID ptr,
    IN SIZE_T cnt
    )
{
    volatile char *vptr = (volatile char *)ptr;

    while (cnt--) {
        *vptr = 0;
        vptr++;
    }

    return ptr;
}


BOOL
GetNewPasswordOKDialog(
    IN HWND  hDlg,
    IN PVOID pCallbackContext)
//
//  Change Password dialog OK button handler
//
//  Reads the dialog fields.
//  If they are ok then it saves the new values in the context data.
//  If the new and confirm passwords in the dialog are mismatched,
//  an error message is displayed, and the user is given a chance
//  to reenter them.
//
{
    PNETUI_NEWPWD     pNewPwd = (PNETUI_NEWPWD)pCallbackContext;
    TCHAR             szConfirmPassword[PWLEN+1];

    GetDlgItemText(hDlg, IDC_NEW_PASSWORD, pNewPwd->szNewPassword, PWLEN+1);
    GetDlgItemText(hDlg, IDC_CONFIRM_PASSWORD, szConfirmPassword, PWLEN+1);

    if (wcscmp(pNewPwd->szNewPassword, szConfirmPassword) == 0)
    {
        // Success, got new password
        EndDialog( hDlg, 1 );
    }
    else
    {
        // mismatch, tell user that New password doesn't match Confirm
        (void)MessageBoxUsingIds(hDlg, IDS_CHANGE_PASSWORD_MISTYPE, IDS_CHANGE_PASSWORD, MB_OK);
        SetDlgItemText(hDlg, IDC_NEW_PASSWORD, TEXT(""));
        SetDlgItemText(hDlg, IDC_CONFIRM_PASSWORD, TEXT(""));
    }

    MyRtlSecureZeroMemory(szConfirmPassword, sizeof(szConfirmPassword));

    return TRUE;
}

USER_PASSWORD_DIALOG_CALLBACK_TABLE GetNewPasswordCallbacks =
{
    GetNewPasswordInitDialog,
    GetNewPasswordOKDialog
};

BOOL
GetNewPasswordEx(
    IN     HWND             hParent,
    IN OUT PNETUI_NEWPWD    pNewPwd,
       OUT HWND OPTIONAL   *phDlg)
//
//  This function is called to display the dialog that prompts for a new password.
//
//  After calling this function, but before it returns, the value in *phDlg may be
//  passed to CloseUsernamePasswordDialog to abort the attempt to get the new password.
//
{
    BOOL    bGotNewPassword;

    bGotNewPassword = CommonUserPasswordDialogBox(
                            IDD_CHANGE_PASSWORD_W,
                            IDD_CHANGE_PASSWORD_N,
                            hParent,
                            &GetNewPasswordCallbacks,
                            pNewPwd,
                            phDlg);

    return bGotNewPassword;
}

BOOL
GetNewPassword(
    IN     HWND             hParent,
    IN OUT PNETUI_NEWPWD    pNewPwd)
{
    return GetNewPasswordEx(hParent, pNewPwd, NULL);
}

BOOL
CloseUsernamePasswordDialog(
    IN   HWND hDlg)
//
//  Abort an attempt to have the user input data into a dialog, closing the dialog.
//
{
    BOOL bWorked;

    bWorked = EndDialog(hDlg, 0);

    DEBUGMSG (ZONE_WARN && !bWorked, (TEXT("NETUI: WARNING - EndDialog of UsernamePassword dialog failed, e=%u\n"), GetLastError()));

    return bWorked;
}
