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

#include <objbase.h>
#include <initguid.h>

#include "wavemain.h"

static const WAVEOUTCAPS woc =
{
    MM_MICROSOFT,
    24,
    0x0001,
    TEXT("USB Audio Class Output"),
    // supported wave formats
    WAVE_FORMAT_1M08 | WAVE_FORMAT_2M08 | WAVE_FORMAT_4M08 |
    WAVE_FORMAT_1S08 | WAVE_FORMAT_2S08 | WAVE_FORMAT_4S08 |
    WAVE_FORMAT_1M16 | WAVE_FORMAT_2M16 | WAVE_FORMAT_4M16 |
    WAVE_FORMAT_1S16 | WAVE_FORMAT_2S16 | WAVE_FORMAT_4S16,
    0,      // channels
    0,
    WAVECAPS_VOLUME | WAVECAPS_PLAYBACKRATE
#if !(MONO_GAIN)
    | WAVECAPS_LRVOLUME
#endif
};

static const WAVEINCAPS wic =
{
    MM_MICROSOFT,
    23,
    0x0001,
    TEXT("USB Audio Class Input"),
    // supported wave formats
    WAVE_FORMAT_1M08 | WAVE_FORMAT_2M08 | WAVE_FORMAT_4M08 |
    WAVE_FORMAT_1S08 | WAVE_FORMAT_2S08 | WAVE_FORMAT_4S08 |
    WAVE_FORMAT_1M16 | WAVE_FORMAT_2M16 | WAVE_FORMAT_4M16 |
    WAVE_FORMAT_1S16 | WAVE_FORMAT_2S16 | WAVE_FORMAT_4S16,
    0,      // channels
    0
};

static const WAVEOUTEXTCAPS wec =
{
#if USE_OS_MIXER
    0x1,                                // max number of hw-mixed streams
    0x1,                                // available HW streams
#else
    0xFFFF,                             // max number of hw-mixed streams
    0xFFFF,                             // available HW streams
#endif
    0,                                  // preferred sample rate for software mixer (0 indicates no preference)
    0,                                  // preferred buffer size for software mixer (0 indicates no preference)
    0,                                  // preferred number of buffers for software mixer (0 indicates no preference)
    USBAUD_MIN_SAMPLERATE,              // minimum sample rate for a hw-mixed stream
    USBAUD_MAX_SAMPLERATE               // maximum sample rate for a hw-mixed stream
};

DeviceContext::DeviceContext(UINT DeviceId, HardwareContext* pHWContext)
{
    m_DeviceId = DeviceId;
    m_pHWContext = pHWContext;

    InitializeListHead(&m_StreamList);
    m_dwGain = MAX_GAIN;
    m_dwDefaultStreamGain = MAX_GAIN;
    m_bExclusiveStream = FALSE;
    m_StreamClassTable.Initialize();
}

OutputDeviceContext::OutputDeviceContext(UINT DeviceId, HardwareContext* pHWContext) :
    DeviceContext(DeviceId, pHWContext)
{
}

InputDeviceContext::InputDeviceContext(UINT DeviceId, HardwareContext* pHWContext) :
    DeviceContext(DeviceId, pHWContext)
{
}

DWORD DeviceContext::GetGain()
{
    return m_dwGain;
}

DWORD DeviceContext::SetGain(DWORD dwGain)
{
    m_dwGain = dwGain;
    RecalcAllGains();
    return MMSYSERR_NOERROR;
}

DWORD DeviceContext::GetDefaultStreamGain()
{
    return m_dwDefaultStreamGain;
}

DWORD DeviceContext::SetDefaultStreamGain(DWORD dwGain)
{
    m_dwDefaultStreamGain = dwGain;
    return MMSYSERR_NOERROR;
}

BOOL DeviceContext::GetClassProperty(DWORD dwClassId, PDWORD pdwClassGain, PBOOL pfBypassDeviceGain)
{
    BOOL fSuccess = FALSE;

    // Must specify at least one get param.
    if ((NULL == pdwClassGain) && (NULL == pfBypassDeviceGain))
    {
        return FALSE;
    }

    STREAMCLASSCONFIG *pEntry = m_StreamClassTable.FindEntry(dwClassId);
    if (pEntry != NULL)
    {
        if (pdwClassGain != NULL)
        {
            *pdwClassGain = pEntry->dwClassGain;
        }
        if (pfBypassDeviceGain != NULL)
        {
            *pfBypassDeviceGain = pEntry->fBypassDeviceGain;
        }

        fSuccess = TRUE;
    }

    return fSuccess;
}

BOOL DeviceContext::IsBypassDeviceGain(DWORD dwClassId)
{
    BOOL fBypass;

    if (!GetClassProperty(dwClassId, NULL, &fBypass))
    {
        fBypass = c_fBypassDeviceGainDefault;
    }

    return fBypass;
}

DWORD DeviceContext::GetClassGain(DWORD dwClassId)
{
    DWORD dwClassGain;

    if (!GetClassProperty(dwClassId, &dwClassGain, NULL))
    {
        dwClassGain = c_dwClassGainDefault;
    }

    return dwClassGain;
}

DWORD DeviceContext::SetClassGain(DWORD dwClassId, DWORD dwClassGain)
{
    MMRESULT mmRet;

    if (dwClassGain > WAVE_STREAMCLASS_CLASSGAIN_MAX)
    {
        mmRet = MMSYSERR_INVALPARAM;
        goto Exit;
    }

    STREAMCLASSCONFIG *pEntry = m_StreamClassTable.FindEntry(dwClassId);
    if (NULL == pEntry)
    {
        mmRet = MMSYSERR_INVALPARAM;
        goto Exit;
    }

    pEntry->dwClassGain = dwClassGain;
    RecalcAllGains();

    mmRet = MMSYSERR_NOERROR;

Exit:

    return mmRet;
}

BOOL DeviceContext::IsValidClassId(DWORD dwClassId)
{
    STREAMCLASSCONFIG *pEntry = m_StreamClassTable.FindEntry(dwClassId);
    return (pEntry != NULL);
}

BOOL DeviceContext::IsSupportedFormat(LPWAVEFORMATEX lpFormat)
{
    if (lpFormat->wFormatTag != WAVE_FORMAT_PCM)
        return FALSE;

    if (  (lpFormat->nChannels<1) || (lpFormat->nChannels>2) )
        return FALSE;

    if (  (lpFormat->wBitsPerSample!=8) && (lpFormat->wBitsPerSample!=16) )
        return FALSE;

    if (lpFormat->nSamplesPerSec < 100 || lpFormat->nSamplesPerSec > 192000)
        return FALSE;

    return TRUE;
}

BOOL OutputDeviceContext::IsSupportedFormat(LPWAVEFORMATEX lpFormat)
{
    return DeviceContext::IsSupportedFormat(lpFormat);
}

// Assumes lock is taken
DWORD DeviceContext::NewStream(StreamContext *pStreamContext)
{
    if (pStreamContext->IsExclusive())
    {
        if (m_bExclusiveStream)
        {
            return MMSYSERR_ALLOCATED;
        }

        // Exclusive streams are inserted at the head of the list so they can block other streams
        // further down in the list (e.g. by setting the pTransferStatus->Mute bit)
        m_bExclusiveStream = TRUE;
        InsertHeadList(&m_StreamList,&pStreamContext->m_Link);
    }
    else
    {
        InsertTailList(&m_StreamList,&pStreamContext->m_Link);
    }

    return MMSYSERR_NOERROR;
}

// Assumes lock is taken
void DeviceContext::DeleteStream(StreamContext *pStreamContext)
{
    if (pStreamContext->IsExclusive())
    {
        m_bExclusiveStream = FALSE;
    }

    RemoveEntryList(&pStreamContext->m_Link);
}

// lock is not taken when calling into the dma functions, need to lock
//only used if things have gone totaly wrong
void DeviceContext::ResetDeviceContext(void)
{
    PLIST_ENTRY pListEntry;
    StreamContext *pStreamContext;
    if(!IsListEmpty(&m_StreamList)){
    for (pListEntry = m_StreamList.Flink;
        pListEntry != &m_StreamList;
        )
        {
        pStreamContext = CONTAINING_RECORD(pListEntry,StreamContext,m_Link);
        pListEntry = pListEntry->Flink;
        if (MMSYSERR_NOERROR == pStreamContext->Reset())
            {
            // After reset we should set this back to running
            pStreamContext->Run();
            }
        }
    }
}

// Returns # of samples of output buffer filled
// Assumes that m_pHWContext->Lock is not held by caller.
PBYTE DeviceContext::TransferBuffer(PBYTE pBuffer, PBYTE pBufferEnd, TRANSFER_STATUS *pTransferStatus)
{
    PLIST_ENTRY pListEntry;
    StreamContext *pStreamContext;
    PBYTE pBufferLastThis;
    PBYTE pBufferLast=pBuffer;
    DWORD NumStreams=0;

    m_pHWContext->Lock();

    pListEntry = m_StreamList.Flink;
    while (pListEntry != &m_StreamList)
    {
        // Get a pointer to the stream context
        pStreamContext = CONTAINING_RECORD(pListEntry,StreamContext,m_Link);

        // Note: The stream context may be closed and removed from the list inside
        // of Render, and the context may be freed as soon as we call Release.
        // Therefore we need to grab the next Flink first in case the
        // entry disappears out from under us.
        pListEntry = pListEntry->Flink;

        // Render buffers
        pStreamContext->AddRef();
        pBufferLastThis = pStreamContext->Render(pBuffer, pBufferEnd, pBufferLast, pTransferStatus);
        pStreamContext->Release();
        if (pBufferLastThis>pBuffer)
        {
            NumStreams++;
        }
        if (pBufferLast < pBufferLastThis)
        {
            pBufferLast = pBufferLastThis;
        }
    }

    if (pTransferStatus)
    {
        pTransferStatus->NumStreams=NumStreams;
    }

    m_pHWContext->Unlock();

    return pBufferLast;
}

void DeviceContext::RecalcAllGains()
{
    PLIST_ENTRY pListEntry;
    StreamContext *pStreamContext;

    for (pListEntry = m_StreamList.Flink;
        pListEntry != &m_StreamList;
        pListEntry = pListEntry->Flink)
    {
        pStreamContext = CONTAINING_RECORD(pListEntry,StreamContext,m_Link);
        pStreamContext->GainChange();
    }
    return;
}

void OutputDeviceContext::StreamReadyToRender(StreamContext *pStreamContext)
{
    m_pHWContext->StartOutputDMA(m_DeviceId);
    return;
}

void InputDeviceContext::StreamReadyToRender(StreamContext *pStreamContext)
{
    m_pHWContext->StartInputDMA(m_DeviceId);
    return;
}

DWORD OutputDeviceContext::GetDevCaps(LPVOID pCaps, DWORD dwSize)
{
    if (dwSize < sizeof(WAVEOUTCAPS))
    {
        return MMSYSERR_ERROR;
    }

    if (dwSize > sizeof(WAVEOUTCAPS))
    {
        dwSize = sizeof(WAVEOUTCAPS);
    }

    memcpy(pCaps, &woc, sizeof(WAVEOUTCAPS));
    ((WAVEOUTCAPS *)pCaps)->wChannels = (WORD)GetChannels();
    return MMSYSERR_NOERROR;
}

DWORD InputDeviceContext::GetDevCaps(LPVOID pCaps, DWORD dwSize)
{
    if (dwSize < sizeof(WAVEINCAPS))
    {
        return MMSYSERR_ERROR;
    }

    if (dwSize > sizeof(WAVEINCAPS))
    {
        dwSize = sizeof(WAVEINCAPS);
    }

    memcpy(pCaps, &wic, sizeof(WAVEINCAPS));
    ((WAVEINCAPS *)pCaps)->wChannels = (WORD)GetChannels();
    return MMSYSERR_NOERROR;
}

DWORD OutputDeviceContext::GetExtDevCaps(LPVOID pCaps, DWORD dwSize)
{
    if (dwSize < sizeof(WAVEOUTEXTCAPS))
    {
        return MMSYSERR_ERROR;
    }

    if (dwSize > sizeof(WAVEOUTEXTCAPS))
    {
        dwSize = sizeof(WAVEOUTEXTCAPS);
    }

    memcpy(pCaps, &wec, sizeof(WAVEOUTEXTCAPS));
    ((WAVEOUTEXTCAPS *)pCaps)->dwSwMixerSampleRate = GetBaseSampleRate();
    return MMSYSERR_NOERROR;
}

DWORD InputDeviceContext::GetExtDevCaps(LPVOID pCaps, DWORD dwSize)
{
    return MMSYSERR_NOTSUPPORTED;
}

StreamContext *InputDeviceContext::CreateStream(LPWAVEOPENDESC lpWOD)
{
    return new InputStreamContext;
}

StreamContext *OutputDeviceContext::CreateStream(LPWAVEOPENDESC lpWOD)
{
    LPWAVEFORMATEX lpFormat=lpWOD->lpFormat;

    if (lpFormat->wBitsPerSample==8)
    {
        if (lpFormat->nChannels==1)
        {
            return new OutputStreamContextM8;
        }
        else if (lpFormat->nChannels==2)
        {
            return new OutputStreamContextS8;
        }
    }
    else if (lpFormat->wBitsPerSample==16)
    {
        if (lpFormat->nChannels==1)
        {
            return new OutputStreamContextM16;
        }
        else if (lpFormat->nChannels==2)
        {
            return new OutputStreamContextS16;
        }
    }

    return NULL;
}

DWORD DeviceContext::OpenStream(LPWAVEOPENDESC lpWOD, DWORD dwFlags, StreamContext **ppStreamContext)
{
    DWORD mmRet = MMSYSERR_NOERROR;

    StreamContext *pStreamContext;

    if (lpWOD->lpFormat==NULL)
    {
        mmRet = WAVERR_BADFORMAT;
        goto Exit;
    }

    if (!IsSupportedFormat(lpWOD->lpFormat))
    {
        mmRet = WAVERR_BADFORMAT;
        goto Exit;
    }

    // Query format support only - don't actually open device?
    if (dwFlags & WAVE_FORMAT_QUERY)
    {
        mmRet = MMSYSERR_NOERROR;
        goto Exit;
    }

    if (!m_pHWContext->IsAttached())
    {
        SetLastError(ERROR_NOT_READY);
        mmRet = MMSYSERR_NOTENABLED;
        goto Exit;
    }

    pStreamContext = CreateStream(lpWOD);
    if (!pStreamContext)
    {
        mmRet = MMSYSERR_NOMEM;
        goto Exit;
    }

    mmRet = pStreamContext->Open(this,lpWOD,dwFlags);
    if (MMSYSERR_NOERROR != mmRet)
    {
        delete pStreamContext;
        goto Exit;
    }

    *ppStreamContext=pStreamContext;

Exit:
    return mmRet;
}

void DeviceContext::SetBaseSampleRate(DWORD BaseSampleRate)
{
    m_BaseSampleRate = BaseSampleRate;

    // Also calculate inverse sample rate, in .32 fixed format,
    // with 1 added at bottom to ensure round up.
    m_BaseSampleRateInverse = ((UINT32)(((1i64<<32)/BaseSampleRate)));

    ResetBaseInfo();

    return;
}

void DeviceContext::ResetBaseInfo()
{
    // Iterate through any existing streams and let them know that something has changed
    PLIST_ENTRY pListEntry;
    StreamContext *pStreamContext;

    for (pListEntry = m_StreamList.Flink;
        pListEntry != &m_StreamList;
        pListEntry = pListEntry->Flink)
    {
        pStreamContext = CONTAINING_RECORD(pListEntry,StreamContext,m_Link);
        pStreamContext->ResetBaseInfo();
    }

    return;
}

DWORD DeviceContext::GetBaseSampleRate()
{
    return m_BaseSampleRate;
}

DWORD DeviceContext::GetBaseSampleRateInverse()
{
    return m_BaseSampleRateInverse;
}

DWORD OutputDeviceContext::GetChannels()
{
    return m_pHWContext->GetNumOutputChannels(m_DeviceId);
}

DWORD InputDeviceContext::GetChannels()
{
    return m_pHWContext->GetNumInputChannels(m_DeviceId);
}

DWORD DeviceContext::GetNativeFormat(ULONG Index, LPVOID pData, ULONG cbData, ULONG *pcbReturn)
{
    // This driver only supports one native format
    if (Index!=0)
    {
        return MMSYSERR_ERROR;
    }

    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.wBitsPerSample = BITSPERSAMPLE;
    wfx.nChannels = (WORD)GetChannels();
    wfx.nSamplesPerSec = GetBaseSampleRate();
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nChannels;
    wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;

    wfx.cbSize = 0;

    ULONG BytesToCopy = min(sizeof(wfx),cbData);

    if (!CeSafeCopyMemory(pData, &wfx, BytesToCopy))
    {
        return  MMSYSERR_INVALPARAM;
    }

    *pcbReturn=BytesToCopy;

    return MMSYSERR_NOERROR;
}

// Surround with ifndef in case this is ever added to mmsystem.h in a future OS release
#ifndef MM_PROPSET_GETNATIVEFORMAT
#define MM_PROPSET_GETNATIVEFORMAT { 0x40c42894, 0x7114, 0x489a, { 0xaf, 0xf3, 0xc6, 0x88, 0xba, 0x76, 0x74, 0x21 } }
#endif

DWORD DeviceContext::GetProperty(PWAVEPROPINFO pPropInfo)
{
    DWORD mmRet = MMSYSERR_NOTSUPPORTED;
    static const GUID guid_GetNativeFormat = MM_PROPSET_GETNATIVEFORMAT;

    if (IsEqualGUID(guid_GetNativeFormat, *pPropInfo->pPropSetId))
    {
        mmRet = GetNativeFormat(
                    pPropInfo->ulPropId,
                    pPropInfo->pvPropData,
                    pPropInfo->cbPropData,
                    pPropInfo->pcbReturn);
    }

    return mmRet;
}

DWORD DeviceContext::SetProperty(PWAVEPROPINFO pPropInfo)
{
    DWORD mmRet = MMSYSERR_NOTSUPPORTED;

    // Stream class.
    if (IsEqualGUID(MM_PROPSET_STREAMCLASS, *pPropInfo->pPropSetId))
    {
        switch (pPropInfo->ulPropId)
        {
        case MM_PROP_STREAMCLASS_CONFIG:
            {
                if ((NULL == pPropInfo->pvPropData) ||
                    (pPropInfo->cbPropData < sizeof(STREAMCLASSCONFIG)))
                {
                    mmRet = MMSYSERR_INVALPARAM;
                    goto Exit;
                }

                PSTREAMCLASSCONFIG pStreamClassConfigTable;
                pStreamClassConfigTable = (PSTREAMCLASSCONFIG) pPropInfo->pvPropData;
                DWORD cEntries = pPropInfo->cbPropData / sizeof(pStreamClassConfigTable[0]);

                // Set stream class config.
                if (m_StreamClassTable.Reinitialize(
                        pStreamClassConfigTable,
                        cEntries))
                {
                    mmRet = MMSYSERR_NOERROR;
                }
                else
                {
                    mmRet = MMSYSERR_INVALPARAM;
                }
            }
            break;

        case MM_PROP_STREAMCLASS_CLASSGAIN:
            {
                if ((NULL == pPropInfo->pvPropParams) ||
                    (pPropInfo->cbPropParams != sizeof(DWORD)) ||
                    (NULL == pPropInfo->pvPropData) ||
                    (pPropInfo->cbPropData != sizeof(DWORD)))
                {
                    mmRet = MMSYSERR_INVALPARAM;
                    goto Exit;
                }

                DWORD dwClassId = *((PDWORD) pPropInfo->pvPropParams);
                DWORD dwClassGain = *((PDWORD) pPropInfo->pvPropData);

                // Set class gain.
                mmRet = SetClassGain(dwClassId, dwClassGain);
            }
            break;

        default:
            break;
        }
    }

Exit:

    return mmRet;
}

DWORD OutputDeviceContext::GetProperty(PWAVEPROPINFO pPropInfo)
{
    // Call base class.
    DWORD mmRet = DeviceContext::GetProperty(pPropInfo);

    // Forward to hardware context if not supported.
    if (MMSYSERR_NOTSUPPORTED == mmRet)
    {
        mmRet = m_pHWContext->OutputGetProperty(pPropInfo);
    }

    return mmRet;
}

DWORD OutputDeviceContext::SetProperty(PWAVEPROPINFO pPropInfo)
{
    // Call base class.
    DWORD mmRet = DeviceContext::SetProperty(pPropInfo);

    // Forward to hardware context if not supported.
    if (MMSYSERR_NOTSUPPORTED == mmRet)
    {
        mmRet = m_pHWContext->OutputSetProperty(pPropInfo);
    }

    return mmRet;
}

DWORD InputDeviceContext::GetProperty(PWAVEPROPINFO pPropInfo)
{
    // Call base class.
    DWORD mmRet = DeviceContext::GetProperty(pPropInfo);

    // Forward to hardware context if not supported.
    if (MMSYSERR_NOTSUPPORTED == mmRet)
    {
        mmRet = m_pHWContext->InputGetProperty(pPropInfo);
    }

    return mmRet;
}

DWORD InputDeviceContext::SetProperty(PWAVEPROPINFO pPropInfo)
{
    // Call base class.
    DWORD mmRet = DeviceContext::SetProperty(pPropInfo);

    // Forward to hardware context if not supported.
    if (MMSYSERR_NOTSUPPORTED == mmRet)
    {
        mmRet = m_pHWContext->InputSetProperty(pPropInfo);
    }

    return mmRet;
}
