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

EXTERN_C BOOL GetDriverNameExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{
    HWND hParent = NULL;
    PGETDRIVERNAMEPARMS pDriverParms = NULL;
    PGetDriverNameArgs pGetDriverNameArgs = NULL;
    PGetDriverNameArgs pReturnArgs = NULL;

    if(nInBufferSize < sizeof(GetDriverNameArgs)) {
	 SetLastError(ERROR_INVALID_PARAMETER);	
        return FALSE;
    }

    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PGetDriverNameArgs) lpOutBuffer;
    pGetDriverNameArgs = (PGetDriverNameArgs) lpInBuffer;
    hParent = pGetDriverNameArgs->hParent;
    pDriverParms = &(pReturnArgs->getDrvNameParms);
 
    pReturnArgs->lastError = ERROR_SUCCESS;
    SetLastError(ERROR_SUCCESS);
    if(!(pReturnArgs->retCode = GetDriverName(hParent, pDriverParms))) {
        pReturnArgs->lastError = GetLastError();
        return FALSE;
    }

    return TRUE;
}



