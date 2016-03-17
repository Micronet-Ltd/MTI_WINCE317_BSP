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

EXTERN_C BOOL GetIPAddressExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{
    HWND hParent = NULL;
    PGetIPAddressArgs pGetIPAddressArgs = NULL;
    PGetIPAddressArgs pReturnArgs = NULL;

    if(nInBufferSize < sizeof(GetIPAddressArgs)) {
	 SetLastError(ERROR_INVALID_PARAMETER);	
        return FALSE;
    }

    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PGetIPAddressArgs) lpOutBuffer;
    pGetIPAddressArgs = (PGetIPAddressArgs) lpInBuffer;
    hParent = pGetIPAddressArgs->hParent;

    pReturnArgs->retCode = ERROR_SUCCESS;
    SetLastError(ERROR_SUCCESS);
    if(!(pReturnArgs->ipAddress = GetIPAddress(hParent))) {
        pReturnArgs->retCode = GetLastError();
        return FALSE;
    }

    return TRUE;
}


