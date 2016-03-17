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

EXTERN_C BOOL AdapterIPPropertiesExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{
    HWND hParent = NULL;
    LPTSTR szAdapterName = NULL;	
    PAdapterIPPropertiesArgs pAdapterIPPropertiesArgs = NULL;
    PAdapterIPPropertiesArgs pReturnArgs = NULL;

    if(nInBufferSize < sizeof(AdapterIPPropertiesArgs)) {
	 SetLastError(ERROR_INVALID_PARAMETER);	
        return FALSE;
    }

    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PAdapterIPPropertiesArgs) lpOutBuffer;
    pAdapterIPPropertiesArgs = (PAdapterIPPropertiesArgs) lpInBuffer;
    hParent = pAdapterIPPropertiesArgs->hParent;
    szAdapterName = pAdapterIPPropertiesArgs->AdapterName;

    pReturnArgs->lastError = ERROR_SUCCESS;
    SetLastError(ERROR_SUCCESS);
    if(!(pReturnArgs->retCode = AdapterIPProperties (hParent, szAdapterName))) {
        pReturnArgs->lastError = GetLastError();
        return FALSE;
    }

    return TRUE;
}


