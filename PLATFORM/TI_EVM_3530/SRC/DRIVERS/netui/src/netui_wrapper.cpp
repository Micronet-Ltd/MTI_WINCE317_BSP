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

netui.c

Abstract:

Functions:


Notes:


--*/
#include <windows.h>
#include <netui.h>


//
// ----------------------------------------------------------------
//
//  GetNetStringSizeExt
//
// ----------------------------------------------------------------
EXTERN_C BOOL GetNetStringSizeExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{
    UINT uID = 0;
    PGetNetStringSizeArgs pGetNetStringSizeArgs = NULL;
    PGetNetStringSizeArgs pReturnArgs = NULL;

    if(nInBufferSize < sizeof(GetNetStringSizeArgs)) {
        return FALSE;
    }


    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PGetNetStringSizeArgs) lpOutBuffer;
    pGetNetStringSizeArgs = (PGetNetStringSizeArgs) lpInBuffer;
    uID = pGetNetStringSizeArgs->uID;

    pReturnArgs->retCode = GetNetStringSize(uID);

    return TRUE;
}



//
// ----------------------------------------------------------------
//
//  GetNetStringExt
//
// ----------------------------------------------------------------
EXTERN_C BOOL GetNetStringExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{
    UINT uID = 0;
    int nBufferMax = 0;
    WCHAR* lpBuffer = NULL;
    PGetNetStringArgs pGetNetStringArgs = NULL;
    PGetNetStringArgs pReturnArgs = NULL;

    if(nInBufferSize < sizeof(GetNetStringArgs)) {
        return FALSE;
    }


    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PGetNetStringArgs) lpOutBuffer;
    pGetNetStringArgs = (PGetNetStringArgs) lpInBuffer;
    uID = pGetNetStringArgs->uID;
    nBufferMax = pGetNetStringArgs->nBufferMax;
    lpBuffer = pReturnArgs->buffer;

    pReturnArgs->retCode = GetNetString(uID, lpBuffer, nBufferMax);

    return TRUE;
}



//
// ----------------------------------------------------------------
//
//  NetMsgBoxExt
//
// ----------------------------------------------------------------
EXTERN_C BOOL NetMsgBoxExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{
    HWND hParent = NULL;
    DWORD dwFlags = NULL;
    TCHAR* szStr = NULL;
    PNetMsgBoxArgs pNetMsgBoxArgs = NULL;
    PNetMsgBoxArgs pReturnArgs = NULL;

    if(nInBufferSize < sizeof(NetMsgBoxArgs)) {
        return FALSE;
    }


    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PNetMsgBoxArgs) lpOutBuffer;
    pNetMsgBoxArgs = (PNetMsgBoxArgs) lpInBuffer;
    hParent = pNetMsgBoxArgs->hParent;
    dwFlags = pNetMsgBoxArgs->dwFlags;
    szStr = pNetMsgBoxArgs->szStr;

    pReturnArgs->retCode = NetMsgBox(hParent, dwFlags, szStr);

    return TRUE;
}
