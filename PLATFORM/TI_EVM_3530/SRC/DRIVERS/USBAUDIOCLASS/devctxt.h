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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//

#ifndef _INC_DEVCTXT_H
#define _INC_DEVCTXT_H

#pragma once
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//
// -----------------------------------------------------------------------------

#include "StreamClass.h"

#if (MONO_GAIN)
#define MAX_GAIN 0xFFFF
#else
#define MAX_GAIN 0xFFFFFFFF
#endif

typedef struct
{
    DWORD NumStreams;
    BOOL  Mute;
    BOOL  Encoded;
} TRANSFER_STATUS;

class DeviceContext
{
public:
    DeviceContext(UINT DeviceId, HardwareContext* pHWContext);

    virtual BOOL IsSupportedFormat(LPWAVEFORMATEX lpFormat);
    PBYTE TransferBuffer(PBYTE pBuffer, PBYTE pBufferEnd, TRANSFER_STATUS *pTransferStatus);

    virtual DWORD NewStream(StreamContext *pStreamContext);
    void DeleteStream(StreamContext *pStreamContext);

    DWORD GetGain();
    DWORD SetGain(DWORD dwGain);
    DWORD GetDefaultStreamGain();
    DWORD SetDefaultStreamGain(DWORD dwGain);

    DWORD GetClassGain(DWORD dwClassId);
    DWORD SetClassGain(DWORD dwClassId, DWORD dwClassGain);
    BOOL  IsBypassDeviceGain(DWORD dwClassId);
    BOOL  IsValidClassId(DWORD dwClassId);
    BOOL  GetClassProperty(DWORD dwClassId, PDWORD pdwClassGain, PBOOL pfBypassDeviceGain);

    void RecalcAllGains();

    DWORD OpenStream(LPWAVEOPENDESC lpWOD, DWORD dwFlags, StreamContext **ppStreamContext);
    void ResetDeviceContext(void);
    virtual DWORD GetExtDevCaps(PVOID pCaps, DWORD dwSize)=0;
    virtual DWORD GetDevCaps(PVOID pCaps, DWORD dwSize)=0;
    virtual void StreamReadyToRender(StreamContext *pStreamContext)=0;

    virtual StreamContext *CreateStream(LPWAVEOPENDESC lpWOD)=0;

    virtual void SetBaseSampleRate(DWORD BaseSampleRate);
    void ResetBaseInfo();

    DWORD GetBaseSampleRate();
    DWORD GetBaseSampleRateInverse();

    virtual DWORD GetProperty(PWAVEPROPINFO pPropInfo);
    virtual DWORD SetProperty(PWAVEPROPINFO pPropInfo);
    DWORD GetNativeFormat(ULONG Index, LPVOID pData, ULONG cbData, ULONG *pcbReturn);

    virtual DWORD GetChannels()=0;

protected:
    LIST_ENTRY  m_StreamList;         // List of streams rendering to/from this device
    DWORD       m_dwGain;
    DWORD       m_dwDefaultStreamGain;
    StreamClassTable m_StreamClassTable;
    BOOL        m_bExclusiveStream;

    DWORD m_BaseSampleRate;
    DWORD m_BaseSampleRateInverse;
    HardwareContext* m_pHWContext;
    UINT m_DeviceId;
};

class InputDeviceContext : public DeviceContext
{
public:
    InputDeviceContext(UINT DeviceId, HardwareContext* pHWContext);
   
    StreamContext *CreateStream(LPWAVEOPENDESC lpWOD);
    DWORD GetExtDevCaps(PVOID pCaps, DWORD dwSize);
    DWORD GetDevCaps(PVOID pCaps, DWORD dwSize);
    void StreamReadyToRender(StreamContext *pStreamContext);
    DWORD GetProperty(PWAVEPROPINFO pPropInfo);
    DWORD SetProperty(PWAVEPROPINFO pPropInfo);
    DWORD GetChannels();
};

class OutputDeviceContext : public DeviceContext
{
public:
    OutputDeviceContext(UINT DeviceId, HardwareContext* pHWContext);

    BOOL IsSupportedFormat(LPWAVEFORMATEX lpFormat);
    StreamContext *CreateStream(LPWAVEOPENDESC lpWOD);
    DWORD GetExtDevCaps(PVOID pCaps, DWORD dwSize);
    DWORD GetDevCaps(PVOID pCaps, DWORD dwSize);
    void StreamReadyToRender(StreamContext *pStreamContext);
    DWORD GetProperty(PWAVEPROPINFO pPropInfo);
    DWORD SetProperty(PWAVEPROPINFO pPropInfo);
    DWORD GetChannels();
};

#endif //~_INC_DEVCTXT_H
