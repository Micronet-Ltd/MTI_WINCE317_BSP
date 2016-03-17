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

usbdobj.hpp

Abstract:  

Notes: 

--*/
#ifndef __USBDOBJ_HPP__
#define __USBDOBJ_HPP__

struct SNotifyList
{
    LPDEVICE_NOTIFY_ROUTINE lpNotifyRoutine;
    LPVOID                  lpvNotifyParameter;

    SNotifyList *           pNext;
};

struct SWait
{
    HANDLE              hEvent;

};

struct SDriverLibs
{
    HINSTANCE           hDriver;

    SDriverLibs *       pNext;
};

struct SDevice;
struct SHcd
{
    LPVOID              pvHcd;

    LPCHCD_FUNCS        pHcdFuncs;

    SDevice *           pFrameControlOwner;
    CRITICAL_SECTION    csFrameLengthControl;
};

struct SPipe;
#define INVALID_INTERFACE_INDEX 0xff
#define INTERFACE_FLAG_ACTIVATE 0x1
#define INTERFACE_FLAG_DISABLE  0x2
#define INTERFACE_FLAG_SUSPEND  0x4
#define INTERFACE_FLAG_RESET    0x10
struct SDevice
{
    DWORD               dwSig;        // For Structure validation
#define VALID_DEVICE_SIG  0x76654453  // "SDev"
    SHcd *              pHcd;

    CRITICAL_SECTION    csPipeLock;
    SPipe *             apPipes[gcMaxPipes];

    SPipe *             pFreePipeList;

    UINT                iDevice;
    LPCUSB_DEVICE       pDeviceInfo;

    CRITICAL_SECTION    csSerializeNotifyRoutine;
    SNotifyList *       pNotifyList;

    CRITICAL_SECTION    csLibList;
    SDriverLibs *       pDriverLibs;

    BYTE                rgbInterfaceIndex[gcMaxPipes];
    BYTE                rgbInterfaceFlag[gcMaxPipes];
};

struct STransfer;
struct SPipe
{
    DWORD               dwSig;         // For structure validation
#define VALID_PIPE_SIG   0x70695053    // "SPip"
#define FREE_PIPE_SIG    0xDEADDEAD    // Set to this when on free list
#define CLOSING_PIPE_SIG 0xAAAABBBB    // Set to this when pipe is being closed
    SPipe *             pNext;         // For linkage on free lists
    UINT                iRefCnt;
    SDevice *           pDevice;
    WORD                wMaxPktSize;  // cached from the EP descriptor

    STransfer *         pFreeTransferList;

    CRITICAL_SECTION    csTransferLock;
    UINT                cAllocatedTransfers;
    STransfer * *       apTransfers;

    UINT                iEndpointIndex;
};

struct STransfer
{
    DWORD               dwSig;         // For structure validation
#define VALID_TRANSFER_SIG  0x6E725453 // "STrn"
#define FREE_TRANSFER_SIG   0xDEADDEAD // Set to this when on free list
    STransfer *         pNext;         // For linkage on free lists
    UINT                iRefCnt;
    SPipe *             pPipe;
    UINT                iTransfer;
    LPVOID              pvBuffer;

    SWait *             pWait;         // Used to synchronize aborts
    CRITICAL_SECTION    csWaitObject;

    DWORD               cFrames;
    LPDWORD             adwIsochErrors;
    LPDWORD             adwIsochLengths;
    BOOL                fComplete;
    DWORD               dwBytesTransfered;
    DWORD               dwError;
    // Added Pointer for mapping.
    DWORD               dwFlags;
    LPCUSB_DEVICE_REQUEST lpOrigControlHeader;
    LPCUSB_DEVICE_REQUEST lpMappedControlHeader;
    DWORD               dwUserBufferSize;
    LPVOID              lpvOrigUserBuffer;
    LPDWORD             lpdwOrigLengths;
    LPDWORD             lpdwMappedLengths;
    
};

#endif
