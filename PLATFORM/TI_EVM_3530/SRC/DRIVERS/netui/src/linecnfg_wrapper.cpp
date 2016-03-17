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

EXTERN_C BOOL LineConfigEditExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{
	HWND hParent = NULL;
	PLINECONFIGDATA pLineCfgData = NULL;
	PLineConfigEditArgs pLineConfigEditArgs = NULL;
	PLineConfigEditArgs pReturnArgs = NULL;

	// Validate parameters
	if (!lpOutBuffer)
	{
		// No way to return a meaningful return code
		return FALSE;
	}

	// we now should be safe in using the lpOutBuffer
	pReturnArgs = (PLineConfigEditArgs) lpOutBuffer;

	if ((!lpInBuffer)									||
		(!pBytesReturned)								||
		(nInBufferSize < sizeof(LineConfigEditArgs))	||
		(nInBufferSize != nOutBufferSize)
	   )
	{
		pReturnArgs->retCode = ERROR_INVALID_PARAMETER;	
		return FALSE;
	}

	// fill the outputs with defaults.
	*pBytesReturned = nOutBufferSize;
	memcpy(lpOutBuffer, lpInBuffer, nOutBufferSize);
	pReturnArgs->retCode = ERROR_SUCCESS;

	// Work on the out copy as the in might be read only.
	pLineConfigEditArgs = (PLineConfigEditArgs) lpOutBuffer;
	hParent = pLineConfigEditArgs->hParent;
	pLineCfgData = &(pLineConfigEditArgs->lineCfgData);

	SetLastError(ERROR_SUCCESS);

	if(!LineConfigEdit(hParent, pLineCfgData)) {
		pReturnArgs->retCode = GetLastError();
		return FALSE;
	}

	return TRUE;
}


