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



/******************************************************************************* 
 *  GetResourcePassword:
 *
 *  - Dialog box to get resource pwd for servers using share level security.
 *
 * Return Value:
 *    Return TRUE if successful, FALSE if error or cancel.
 *
 *******************************************************************************/
EXTERN_C BOOL GetResourcePasswordExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{
    HWND hParent = NULL;
    PNETUI_RESPWD pResPwd = NULL;
    PGetResourcePasswordArgs pGetResourcePasswordArgs = NULL;
    PGetResourcePasswordArgs pReturnArgs = NULL;

    if(nInBufferSize != sizeof(GetResourcePasswordArgs)) {
        return FALSE;
    }


    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PGetResourcePasswordArgs) lpOutBuffer;
    pGetResourcePasswordArgs = (PGetResourcePasswordArgs) lpInBuffer;
    hParent = pGetResourcePasswordArgs->hParent;
    pResPwd = &(pGetResourcePasswordArgs->resPwd);

    pReturnArgs->retCode = GetResourcePassword(hParent, pResPwd);

    return TRUE;
}



/******************************************************************************* 
 *  ConnectionDialog:
 *
 *  - Dialog box to connect resource to the server 
 *
 * Return Value:
 *    Return TRUE if successful, FALSE if error or cancel.
 *
 *******************************************************************************/
EXTERN_C BOOL ConnectionDialogExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{
    HWND hParent = NULL;
    PADDCONNECT_DLGPARAMS pDlgParams = NULL;
    PConnectDialogArgs pConnectDialogArgs = NULL;
    PConnectDialogArgs pReturnArgs = NULL;

    if(nInBufferSize != sizeof(ConnectDialogArgs)) {
        return FALSE;
    }


    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PConnectDialogArgs) lpOutBuffer;
    pConnectDialogArgs = (PConnectDialogArgs) lpInBuffer;
    hParent = pConnectDialogArgs->hParent;
    pDlgParams = &(pConnectDialogArgs->dlgParams);

    pReturnArgs->retCode = ConnectionDialog(hParent, pDlgParams);

    return TRUE;
}



/******************************************************************************* 
 *  DisconnectDialog:
 *
 *  - Dialog box to disconnect resource to the server 
 *
 * Return Value:
 *    Return TRUE if successful, FALSE if error or cancel.
 *
 *******************************************************************************/
EXTERN_C BOOL DisconnectDialogExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{
    // need a structure to hold the parameters in netui.h
    // need to map pointers
    HWND hParent = NULL;
    DWORD dwType = 0;
    PDisconnectDialogArgs pDisconnectDialogArgs = NULL;
    PDisconnectDialogArgs pReturnArgs = NULL;

    if(nInBufferSize != sizeof(DisconnectDialogArgs)) {
	 pReturnArgs->retCode = ERROR_INVALID_PARAMETER;	
        return FALSE;
    }


    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PDisconnectDialogArgs) lpOutBuffer;
    pDisconnectDialogArgs = (PDisconnectDialogArgs) lpInBuffer;
    hParent = pDisconnectDialogArgs->hParent;
    dwType = pDisconnectDialogArgs->dwType;

    pReturnArgs->retCode = ERROR_SUCCESS;
    if(!DisconnectDialog(hParent, dwType)) {
        pReturnArgs->retCode = GetLastError();
    }        
    
    return TRUE;
}
