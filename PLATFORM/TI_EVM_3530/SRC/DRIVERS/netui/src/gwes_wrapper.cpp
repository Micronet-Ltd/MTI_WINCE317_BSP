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
#include <ceddk.h>



/*******************************************************************************
 *  MsgWaitForMultipleObjectsExt:
 *
 *  - wait for multiple message events to fire
 *
 * Return Value:
 *    Return TRUE if successful, FALSE if error or cancel.
 *
 *******************************************************************************/
EXTERN_C BOOL MsgWaitForMultipleObjectsExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{
    DWORD nCount = 0;
    DWORD dwMilliseconds = 0;
    DWORD dwWakeMask = 0;
    DWORD dwFlags = 0;
    PHANDLE pOutHandles = NULL;
    PHANDLE pHandles = NULL;
    PMsgWaitForMultipleObjectsArgs pWaitForMultipleObjectsArgs = NULL;
    PMsgWaitForMultipleObjectsArgs pReturnArgs = NULL;


    if(nInBufferSize < sizeof(MsgWaitForMultipleObjectsArgs)) {
        return FALSE;
    }


    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PMsgWaitForMultipleObjectsArgs) lpOutBuffer;
    pWaitForMultipleObjectsArgs = (PMsgWaitForMultipleObjectsArgs) lpInBuffer;
    nCount = pWaitForMultipleObjectsArgs->nCount;
    pHandles = &(pWaitForMultipleObjectsArgs->handles[0]);
    pOutHandles = &(pReturnArgs->handles[0]);

    for(DWORD i=0;i<nCount;i++)
        {
        pOutHandles[i] = CeDriverDuplicateCallerHandle(pHandles[i], 0, FALSE, DUPLICATE_SAME_ACCESS);
        }
    
    dwMilliseconds = pWaitForMultipleObjectsArgs->dwMilliseconds;
    dwWakeMask = pWaitForMultipleObjectsArgs->dwWakeMask;
    dwFlags = pWaitForMultipleObjectsArgs->dwFlags;

    pReturnArgs->retCode = MsgWaitForMultipleObjectsEx(nCount, pOutHandles, dwMilliseconds, dwWakeMask, dwFlags);
    pReturnArgs->lastError = GetLastError();


    // Close all Duplicated Handles
    for(DWORD i=0;i<nCount;i++)
        {
        CloseHandle(pOutHandles[i]);
        }

    return TRUE;
}



/*******************************************************************************
 *  PeekMessageWExt:
 *
 *  - peek message
 *
 * Return Value:
 *    Return TRUE if successful, FALSE if error or cancel.
 *
 *******************************************************************************/
EXTERN_C BOOL PeekMessageWExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{
    UINT wMsgFilterMin = 0;
    UINT wMsgFilterMax = 0;
    UINT wRemoveMsg = 0;
    HWND hWnd;
    LPMSG lpMsg;

    PPeekMessageArgs pPeekMessageArgs = NULL;
    PPeekMessageArgs pReturnArgs = NULL;

    if(nInBufferSize < sizeof(PeekMessageArgs)) {
        return FALSE;
    }


    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PPeekMessageArgs) lpOutBuffer;
    pPeekMessageArgs = (PPeekMessageArgs) lpInBuffer;
    wMsgFilterMin= pPeekMessageArgs->wMsgFilterMin;
    wMsgFilterMax= pPeekMessageArgs->wMsgFilterMax;
    wRemoveMsg= pPeekMessageArgs->wRemoveMsg;
    hWnd = pPeekMessageArgs->hWnd;
    lpMsg = &(pReturnArgs->msg);

    pReturnArgs->retCode = PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    pReturnArgs->lastError = GetLastError();

    return TRUE;
}



/*******************************************************************************
 *  TranslateMessageExt:
 *
 *  - translate message
 *
 * Return Value:
 *    Return TRUE if successful, FALSE if error or cancel.
 *
 *******************************************************************************/
EXTERN_C BOOL TranslateMessageExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{
    MSG *msg = NULL;

    PTranslateMessageArgs pTranslateMessageArgs = NULL;
    PTranslateMessageArgs pReturnArgs = NULL;

    if(nInBufferSize < sizeof(TranslateMessageArgs)) {
        return FALSE;
    }


    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PTranslateMessageArgs) lpOutBuffer;
    pTranslateMessageArgs = (PTranslateMessageArgs) lpInBuffer;
    msg = &(pTranslateMessageArgs->msg);

    pReturnArgs->retCode = TranslateMessage(msg);
    pReturnArgs->lastError = GetLastError();

    return TRUE;
}


/*******************************************************************************
 *  DispatchMessageExt:
 *
 *  - dispatch message
 *
 * Return Value:
 *    Return TRUE if successful, FALSE if error or cancel.
 *
 *******************************************************************************/
EXTERN_C BOOL DispatchMessageExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{
    MSG *msg = NULL;

    PDispatchMessageArgs pDispatchMessageArgs = NULL;
    PDispatchMessageArgs pReturnArgs = NULL;

    if(nInBufferSize < sizeof(DispatchMessageArgs)) {
        return FALSE;
    }

    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PDispatchMessageArgs) lpOutBuffer;
    pDispatchMessageArgs = (PDispatchMessageArgs) lpInBuffer;
    msg = &(pDispatchMessageArgs->msg);

    pReturnArgs->retCode = DispatchMessage(msg);
    pReturnArgs->lastError = GetLastError();

    return TRUE;
}




/*******************************************************************************
 *  LoadLibraryExt:
 *
 *  - Load the specified DLL file into the address space of the calling process. 
 *
 * Return Value:
 *    Return TRUE if successful, FALSE if error or cancel.
 *
 *******************************************************************************/
EXTERN_C BOOL LoadLibraryExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{

    CONST WCHAR* lpLibFileName = NULL;
    PLoadLibraryArgs pLoadLibraryArgs = NULL;
    PLoadLibraryArgs pReturnArgs = NULL;

    if(nInBufferSize < sizeof(LoadLibraryArgs)) {
	 SetLastError(ERROR_INVALID_PARAMETER);	
        return FALSE;
    }

    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PLoadLibraryArgs) lpOutBuffer;
    pLoadLibraryArgs = (PLoadLibraryArgs) lpInBuffer;
    lpLibFileName = pLoadLibraryArgs->LibFileName;

    pReturnArgs->lastError = ERROR_SUCCESS;
    SetLastError(ERROR_SUCCESS);
    if(!(pReturnArgs->retCode = LoadLibrary(lpLibFileName))) {
        pReturnArgs->lastError = GetLastError();
        return FALSE;
    }		

    return TRUE;
}



/*******************************************************************************
 *  FreeLibraryExt:
 *
 *  - Decrements the reference count of the loaded DLL module.
 *     When the reference count reaches zero, the module is unmapped from the address space of the calling process
 *     and the handle is no longer valid. 
 *
 * Return Value:
 *    Return TRUE if successful, FALSE if error or cancel.
 *
 *******************************************************************************/
EXTERN_C BOOL FreeLibraryExt(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, PDWORD pBytesReturned)
{

    HMODULE hLibModule = NULL;
    PFreeLibraryArgs pFreeLibraryArgs = NULL;
    PFreeLibraryArgs pReturnArgs = NULL;

    if(nInBufferSize < sizeof(FreeLibraryArgs)) {
   	 SetLastError(ERROR_INVALID_PARAMETER);
	 return FALSE;
    }

    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PFreeLibraryArgs) lpOutBuffer;
    pFreeLibraryArgs = (PFreeLibraryArgs) lpInBuffer;
    hLibModule = pFreeLibraryArgs->hLibModule;

    pReturnArgs->lastError = ERROR_SUCCESS;
    SetLastError(ERROR_SUCCESS);
    if (!(pReturnArgs->retCode = FreeLibrary(hLibModule))) {
        pReturnArgs->lastError = GetLastError();
        return FALSE;
    }		

    return TRUE;
}


