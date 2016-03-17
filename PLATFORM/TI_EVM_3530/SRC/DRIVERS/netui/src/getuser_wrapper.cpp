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

#include <windows.h>
#include <netui.h>

EXTERN_C BOOL GetUsernamePasswordExExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{
    HWND hParent = NULL;
    PNETUI_USERPWD pUserPwd = NULL;
    HWND * phDlg;
    PGetUsernamePasswordExArgs pGetUsernamePasswordExArgs = NULL;
    PGetUsernamePasswordExArgs pReturnArgs = NULL;

    if(nInBufferSize < sizeof(GetUsernamePasswordExArgs)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PGetUsernamePasswordExArgs) lpOutBuffer;
    pGetUsernamePasswordExArgs = (PGetUsernamePasswordExArgs) lpInBuffer;
    hParent = pGetUsernamePasswordExArgs->hParent;
    pUserPwd = &(pReturnArgs->userPwd);
    phDlg = pReturnArgs->hDlg ? &(pReturnArgs->hDlg) : NULL;

    pReturnArgs->retCode = ERROR_SUCCESS;
    SetLastError(ERROR_SUCCESS);
    if(!GetUsernamePasswordEx(hParent, pUserPwd, phDlg)) {
        pReturnArgs->retCode = GetLastError();

        if(ERROR_SUCCESS == pReturnArgs->retCode) {
            pReturnArgs->retCode = ERROR_CANCELLED;
        }
    }

    return TRUE;
}


EXTERN_C BOOL GetNewPasswordExExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{
    HWND hParent = NULL;
    PNETUI_NEWPWD pNewPwd = NULL;
    HWND * phDlg;
    PGetNewPasswordExArgs pGetNewPasswordExArgs = NULL;
    PGetNewPasswordExArgs pReturnArgs = NULL;

    if(nInBufferSize < sizeof(GetNewPasswordExArgs)) {
     SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PGetNewPasswordExArgs) lpOutBuffer;
    pGetNewPasswordExArgs = (PGetNewPasswordExArgs) lpInBuffer;
    hParent = pGetNewPasswordExArgs->hParent;
    pNewPwd = &(pReturnArgs->newPwd);
    phDlg = pReturnArgs->hDlg ? &(pReturnArgs->hDlg) : NULL;

    pReturnArgs->retCode = ERROR_SUCCESS;
    SetLastError(ERROR_SUCCESS);
    if(!GetNewPasswordEx(hParent, pNewPwd, phDlg)) {
        pReturnArgs->retCode = GetLastError();
        return FALSE;
    }

    return TRUE;
}


EXTERN_C BOOL CloseUsernamePasswordDialogExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{
    HWND hDlg = NULL;
    PCloseUsernamePasswordDialogArgs pCloseUsernamePasswordDialogArgs = NULL;
    PCloseUsernamePasswordDialogArgs pReturnArgs = NULL;

    if(nInBufferSize < sizeof(CloseUsernamePasswordDialogArgs)) {
     SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PCloseUsernamePasswordDialogArgs) lpOutBuffer;
    pCloseUsernamePasswordDialogArgs = (PCloseUsernamePasswordDialogArgs) lpInBuffer;
    hDlg = pCloseUsernamePasswordDialogArgs->hDlg;

    pReturnArgs->retCode = ERROR_SUCCESS;
    SetLastError(ERROR_SUCCESS);
    if(!CloseUsernamePasswordDialog(hDlg)) {
        pReturnArgs->retCode = GetLastError();
        return FALSE;
    }

    return TRUE;
}

