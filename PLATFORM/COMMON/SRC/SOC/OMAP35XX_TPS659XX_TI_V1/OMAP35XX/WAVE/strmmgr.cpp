//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <wavedev.h>
#include <mmreg.h>
#include <mmddk.h>
#include "debug.h"
#include "wavemain.h"
#include "audioctrl.h"
#include "Audiolin.h"
#include "mixermgr.h"
#include "strmctxt.h"
#include "strmmgr.h"
#include "audiomgr.h"

DWORD CStreamManager::m_dwStreamAttenMax;
DWORD CStreamManager::m_dwDeviceAttenMax;
DWORD CStreamManager::m_dwClassAttenMax;

static CStreamCallback::SampleRateEntry_t
    s_SampleRates[CStreamCallback::kAudioSampleRateSize] =
{
// the inverse was calculated using the following equation
// Inverse sample rate = (1/rate * 2^32) + 1 // add 1 to round-up
//
// sample rate (f32.0)      Inverse sample rate (f0.32)
// -------------------      ---------------------------
    {8000,                      0x83127},
    {16000,                     0x41894},
    {44100,                     0x17C70},
    {48000,                     0x15D87},
    {96000,                     0xAEC4}     // added 96k sample frequency
};


//------------------------------------------------------------------------------
//
//  Function: CStreamManagert
//
//  constructor
//
CStreamManager::CStreamManager() : m_pAudioManager(NULL)
{
    InitializeListHead(&m_StreamList);
    m_dwGain = MAX_GAIN;
    m_bMute  = FALSE;
    m_bMono  = FALSE;
    m_dwDefaultStreamGain = MAX_GAIN;
    m_StreamClassTable.Initialize();

    m_prgSampleRates = s_SampleRates;

#ifdef PROFILE_MIXER
    m_liPCStart.QuadPart = 0;
    m_liPCTotal.QuadPart = 0;
    QueryPerformanceFrequency(&m_liPCFrequency);
#endif
}

BOOL CStreamManager::GetClassProperty(DWORD dwClassId, PDWORD pdwClassGain, PBOOL pfBypassDeviceGain)
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

BOOL CStreamManager::IsBypassDeviceGain(DWORD dwClassId)
{
    BOOL fBypass;

    if (!GetClassProperty(dwClassId, NULL, &fBypass))
    {
        fBypass = c_fBypassDeviceGainDefault;
    }

    return fBypass;
}

DWORD CStreamManager::GetClassGain(DWORD dwClassId)
{
    DWORD dwClassGain;

    if (!GetClassProperty(dwClassId, &dwClassGain, NULL))
    {
        dwClassGain = c_dwClassGainDefault;
    }

    return dwClassGain;
}

DWORD CStreamManager::SetClassGain(DWORD dwClassId, DWORD dwClassGain)
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
    update_Streams();

    mmRet = MMSYSERR_NOERROR;

Exit:

    return mmRet;
}

BOOL CStreamManager::IsValidClassId(DWORD dwClassId)
{
    STREAMCLASSCONFIG *pEntry = m_StreamClassTable.FindEntry(dwClassId);
    return (pEntry != NULL);
}

//------------------------------------------------------------------------------
//
//  Function: get_HardwareAudioBridge
//
//  returns a reference to the current hw audio bridge
//
CHardwareAudioBridge*
CStreamManager::get_HardwareAudioBridge() const
{
    return m_pAudioManager->get_HardwareAudioBridge();
}


//------------------------------------------------------------------------------
//
//  Function: CStreamManagerIsSupportedFormat
//
//  check if wave format is supported
//
BOOL
CStreamManager::IsSupportedFormat(LPWAVEFORMATEX lpFormat)
{
    if (lpFormat->wFormatTag != WAVE_FORMAT_PCM)
        return FALSE;

    if (  (lpFormat->nChannels!=1) && (lpFormat->nChannels!=2) )
        return FALSE;

    if (  (lpFormat->wBitsPerSample!=8) && (lpFormat->wBitsPerSample!=16) )
        return FALSE;

    if (lpFormat->nSamplesPerSec < 100 ||
        lpFormat->nSamplesPerSec > 96000)
        return FALSE;

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function: CStreamManager::NewStream
//
//  add new stream to list of open streams
//  Assumes lock is taken
//

void
CStreamManager::NewStream(StreamContext *pStreamContext)
{
    InsertTailList(&m_StreamList,&pStreamContext->m_Link);
}

//------------------------------------------------------------------------------
//
//  Function: CStreamManager::DeleteStream
//
//  free stream context
//  Assumes lock is taken
//

void
CStreamManager::DeleteStream(StreamContext *pStreamContext)
{
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    RemoveEntryList(&pStreamContext->m_Link);
    // check for any active streams
    if(IsListEmpty(&m_StreamList) == TRUE)
      {
       RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S, deactivate port\r\n",__FUNCTION__));
        // if none request hw bridge to deactivate port
        get_HardwareAudioBridge()->stop_AudioPort(
            (CHardwareAudioBridge::StreamType)pStreamContext->GetStreamDirection());
      }

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
}


//------------------------------------------------------------------------------
//
//  Function: CStreamManager::update_Streams
//
//  causes all streams to update itself with curren information
//
BOOL
CStreamManager::update_Streams()
{
    // causes all streams to update itself with the new information
    //
    DWORD dwRate;
    PLIST_ENTRY pListEntry;
    StreamContext *pStreamContext;

	RETAILMSG(WAVE_AUDIO_DBG, (L"+CStreamManager::update_Streams\r\n"));

    for (pListEntry = m_StreamList.Flink; pListEntry != &m_StreamList;
        pListEntry = pListEntry->Flink)
        {
        pStreamContext = CONTAINING_RECORD(pListEntry,StreamContext,m_Link);

        // calling GetRate/SetRate cause the stream context to recalculate
        // the upsample/downsampling rate
        //
        pStreamContext->GetRate(&dwRate);
        pStreamContext->SetRate(dwRate);

        // update audio gains
        //
		RETAILMSG(WAVE_AUDIO_DBG, (L"CStreamManager::update_Streams - call GainChange\r\n"));
        pStreamContext->GainChange();
        }

	RETAILMSG(WAVE_AUDIO_DBG, (L"-CStreamManager::update_Streams\r\n"));
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function: CStreamManager::copy_StreamData
//
//  Returns # of samples of output filled the buffer
//
DWORD
CStreamManager::copy_StreamData(HANDLE hContext, void* pStart, DWORD nSize)
{
    PBYTE pBuffer;
    PBYTE pBufferEnd;
    PBYTE pBufferLast;
    DWORD nNumStreams = 0;
    PLIST_ENTRY pListEntry;
    StreamContext *pStreamContext;

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    // get stream context
    for (pListEntry = m_StreamList.Flink; pListEntry != &m_StreamList;
        pListEntry = pListEntry->Flink)
        {
        pStreamContext = CONTAINING_RECORD(pListEntry,StreamContext,m_Link);
        if ((HANDLE)pStreamContext == hContext)
            {
            pBuffer = (PBYTE)pStart;
            pBufferEnd = pBuffer + nSize;
            pBufferLast = pBuffer;
            pStreamContext->Render(pBuffer, pBufferEnd, pBufferLast, FALSE);
            nNumStreams = 1;
            break;
            }
        }

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));

    return nNumStreams;
}

//------------------------------------------------------------------------------
//
//  Function: CStreamManager::copy_AudioData
//
//  Returns # of samples of output filled the buffer
//
DWORD
CStreamManager::copy_AudioData(void * pStart, DWORD nSize)
{
    // UNDONE:
    //  Need critical section
    //
    BYTE* pBuffer = (BYTE*)pStart;
    BYTE* pBufferEnd = pBuffer + nSize;

    DWORD nNumStreams;

	//RETAILMSG(0, (L"WAV:+CStreamManager::%S\r\n",__FUNCTION__));
	RETAILMSG(/*WAVE_AUDIO_DBG*/ 0, (L"WAV:+%S\r\n",__FUNCTION__));

    pBuffer = TransferBuffer(pBuffer, pBufferEnd, &nNumStreams);

	RETAILMSG(0, (L"WAV:-CStreamManager::%S\r\n",__FUNCTION__));
	RETAILMSG(/*WAVE_AUDIO_DBG*/ 0, (L"WAV:-%S\r\n",__FUNCTION__));
    return nNumStreams;
}


//------------------------------------------------------------------------------
//
//  Function: CStreamManager::TransferBuffer
//
//  Returns # of samples of output buffer filled
//  Assumes that g_pHWContext->Lock already held.
//

PBYTE
CStreamManager::TransferBuffer(PBYTE pBuffer, PBYTE pBufferEnd, DWORD *pNumStreams)
{
    PLIST_ENTRY pListEntry;
    StreamContext *pStreamContext;
    PBYTE pBufferLastThis;
    PBYTE pBufferLast=pBuffer;
    DWORD NumStreams=0;

	RETAILMSG(0, (L"WAV:+CStreamManager::%S\r\n",__FUNCTION__));
	RETAILMSG(/*WAVE_AUDIO_DBG*/ 0, (L"WAV:+%S\r\n",__FUNCTION__));
    
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
		RETAILMSG(/*WAVE_AUDIO_DBG*/ 0, (L"WAV:%S - call render\r\n",__FUNCTION__));
        pStreamContext->AddRef();
		// ####, Michael test , 5-Jan-2010, if 'FALSE' - disables out gain control
        pBufferLastThis = pStreamContext->Render(pBuffer, pBufferEnd, pBufferLast, TRUE);
		//pBufferLastThis = pStreamContext->Render(pBuffer, pBufferEnd, pBufferLast, FALSE);
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

    // clear any residual audio
    //
    memset(pBufferLast, 0, pBufferEnd - pBufferLast);

    if (pNumStreams)
    {
        *pNumStreams=NumStreams;
    }

	RETAILMSG(0, (L"WAV:-CStreamManager::%S\r\n",__FUNCTION__));
	RETAILMSG(/*WAVE_AUDIO_DBG*/ 0, (L"WAV:-%S, NumStreams = %d\r\n",__FUNCTION__, NumStreams));
	
    return pBufferLast;
}


//------------------------------------------------------------------------------
//
//  Function: OpenStream
//
//  Check format parameters and create stream context if possible.
//

DWORD
CStreamManager::open_Stream(LPWAVEOPENDESC lpWOD, DWORD dwFlags,
                          StreamContext **ppStreamContext)
{
    HRESULT Result;
    StreamContext *pStreamContext;

    if (lpWOD->lpFormat==NULL)
    {
        return WAVERR_BADFORMAT;
    }

     if (!IsSupportedFormat(lpWOD->lpFormat))
    {
        return WAVERR_BADFORMAT;
    }

    // Query format support only - don't actually open device?
    if (dwFlags & WAVE_FORMAT_QUERY)
    {
        return MMSYSERR_NOERROR;
    }

    pStreamContext = create_Stream(lpWOD);
    if (!pStreamContext)
    {
        return MMSYSERR_NOMEM;
    }

    Result = pStreamContext->Open(this,lpWOD,dwFlags);
    if (FAILED(Result))
    {
        delete pStreamContext;
        return MMSYSERR_ERROR;
    }

    *ppStreamContext=pStreamContext;
    return MMSYSERR_NOERROR;
}

//------------------------------------------------------------------------------
//
//  Function: GetProperty
//
//
//
DWORD
CStreamManager::GetProperty(PWAVEPROPINFO pPropInfo)
{
    return MMSYSERR_NOTSUPPORTED;
}

//------------------------------------------------------------------------------
//
//  Function: SetProperty
//
//
//
DWORD
CStreamManager::SetProperty(PWAVEPROPINFO pPropInfo)
{
    DWORD mmRet = MMSYSERR_NOTSUPPORTED;

#if (_WINCEOSVER==700)

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

#endif

    return mmRet;
}

//------------------------------------------------------------------------------
//
//  Function: CommonGetProperty
//
//
//
DWORD
CStreamManager::CommonGetProperty(PWAVEPROPINFO pPropInfo, BOOL fInput)
{
    DWORD mmRet = MMSYSERR_NOTSUPPORTED;

    // Device topology
    if (IsEqualGUID(MM_PROPSET_DEVTOPOLOGY, *pPropInfo->pPropSetId))
    {
        switch (pPropInfo->ulPropId)
        {
        case MM_PROP_DEVTOPOLOGY_DEVICE_DESCRIPTOR:
            {
                if ((NULL == pPropInfo->pvPropData) ||
                    (pPropInfo->cbPropData < sizeof(DTP_DEVICE_DESCRIPTOR)))
                {
                    mmRet = MMSYSERR_INVALPARAM;
                    break;
                }

                PDTP_DEVICE_DESCRIPTOR pDeviceDescriptor;
                pDeviceDescriptor = (PDTP_DEVICE_DESCRIPTOR) pPropInfo->pvPropData;

                // Get the device descriptor.
                if (fInput)
                {
                    mmRet = GetInputDeviceDescriptor(pDeviceDescriptor);
                }
                else
                {
                    mmRet = GetOutputDeviceDescriptor(pDeviceDescriptor);
                }

                if (MMSYSERR_NOERROR == mmRet)
                {
                    *pPropInfo->pcbReturn = sizeof(DTP_DEVICE_DESCRIPTOR);
                }
            }
            break;

        case MM_PROP_DEVTOPOLOGY_ENDPOINT_DESCRIPTOR:
            {
                if ((NULL == pPropInfo->pvPropParams) ||
                    (pPropInfo->cbPropParams != sizeof(DWORD)) ||
                    (NULL == pPropInfo->pvPropData) ||
                    (pPropInfo->cbPropData < sizeof(DTP_ENDPOINT_DESCRIPTOR)))
                {
                    mmRet = MMSYSERR_INVALPARAM;
                    goto Error;
                }

                DWORD dwIndex = *((PDWORD) pPropInfo->pvPropParams);
                PDTP_ENDPOINT_DESCRIPTOR pEndpointDescriptor;
                pEndpointDescriptor = (PDTP_ENDPOINT_DESCRIPTOR) pPropInfo->pvPropData;

                // Get the endpoint descriptor.
                if (fInput)
                {
                    mmRet = GetInputEndpointDescriptor(
                                dwIndex,
                                pEndpointDescriptor);
                }
                else
                {
                    mmRet = GetOutputEndpointDescriptor(
                                dwIndex,
                                pEndpointDescriptor);
                }

                if (MMSYSERR_NOERROR == mmRet)
                {
                    *pPropInfo->pcbReturn = sizeof(DTP_ENDPOINT_DESCRIPTOR);
                }
            }
            break;

        default:
            break;
        }
    }

Error:

    return mmRet;
}

//------------------------------------------------------------------------------
//
//  Function: CommonSetProperty
//
//
//
DWORD
CStreamManager::CommonSetProperty(PWAVEPROPINFO pPropInfo, BOOL fInput)
{
    DWORD mmRet = MMSYSERR_NOTSUPPORTED;
    HANDLE hClientProc = NULL;

    // Routing control
    if (IsEqualGUID(MM_PROPSET_RTGCTRL, *pPropInfo->pPropSetId))
    {
/*
        // Not supported, cannot find definition for PRTGCTRL_ENDPOINT_ROUTING

        switch (pPropInfo->ulPropId)
        {
        case MM_PROP_RTGCTRL_ENDPOINT_ROUTING:
            {
                if ((NULL == pPropInfo->pvPropData) ||
                    (pPropInfo->cbPropData < sizeof(RTGCTRL_ENDPOINT_ROUTING)))
                {
                    mmRet = MMSYSERR_INVALPARAM;
                    goto Error;
                }

                PRTGCTRL_ENDPOINT_ROUTING pEndpointRouting;
                pEndpointRouting = (PRTGCTRL_ENDPOINT_ROUTING) pPropInfo->pvPropData;

                // Check the media type to route.
                switch (pEndpointRouting->MediaType)
                {
                case RTGCTRL_MEDIATYPE_SYSTEM:
                    {
                        // Route system audio using the selected endpoints.
                        if (fInput)
                        {
                            mmRet = RouteSystemInputEndpoints(pEndpointRouting);
                        }
                        else
                        {
                            mmRet = RouteSystemOutputEndpoints(pEndpointRouting);
                        }
                    }
                    break;

                case RTGCTRL_MEDIATYPE_CELLULAR:
                    {
                        // Route cellular audio using the selected endpoints.
                        if (fInput)
                        {
                            mmRet = RouteCellularInputEndpoints(pEndpointRouting);
                        }
                        else
                        {
                            mmRet = RouteCellularOutputEndpoints(pEndpointRouting);
                        }
                    }
                    break;

                default:
                    break;
                }
            }
            break;

        default:
            break;
        }
*/
    }
    // Device topology
    else if (IsEqualGUID(MM_PROPSET_DEVTOPOLOGY, *pPropInfo->pPropSetId))
    {
        switch (pPropInfo->ulPropId)
        {
        case MM_PROP_DEVTOPOLOGY_EVTMSG_REGISTER:
            {
                if ((NULL == pPropInfo->pvPropData) ||
                    (pPropInfo->cbPropData != sizeof(HANDLE)))
                {
                    mmRet = MMSYSERR_INVALPARAM;
                    goto Error;
                }

                // Get client message queue handle.
                HANDLE hMsgQueue = *((HANDLE *) pPropInfo->pvPropData);

                // Get client process.
                hClientProc = OpenProcess(PROCESS_DUP_HANDLE, FALSE, GetCallerVMProcessId());
                if (NULL == hClientProc)
                {
                    mmRet = MMSYSERR_ERROR;
                    goto Error;
                }

                // Register for device topology messages.
                if (fInput)
                {
                    mmRet = m_InputDtpNotify.RegisterDtpMsgQueue(hMsgQueue, hClientProc);
                }
                else
                {
                    mmRet = m_OutputDtpNotify.RegisterDtpMsgQueue(hMsgQueue, hClientProc);
                }
            }
            break;

        case MM_PROP_DEVTOPOLOGY_EVTMSG_UNREGISTER:
            {
                if ((NULL == pPropInfo->pvPropData) ||
                    (pPropInfo->cbPropData != sizeof(HANDLE)))
                {
                    mmRet = MMSYSERR_INVALPARAM;
                    goto Error;
                }

                // Get client message queue handle.
                HANDLE hMsgQueue = *((HANDLE *) pPropInfo->pvPropData);

                // Get client process.
                hClientProc = OpenProcess(PROCESS_DUP_HANDLE, FALSE, GetCallerVMProcessId());
                if (NULL == hClientProc)
                {
                    mmRet = MMSYSERR_ERROR;
                    goto Error;
                }

                // Unregister the message queue.
                if (fInput)
                {
                    mmRet = m_InputDtpNotify.UnregisterDtpMsgQueue(hMsgQueue, hClientProc);
                }
                else
                {
                    mmRet = m_OutputDtpNotify.UnregisterDtpMsgQueue(hMsgQueue, hClientProc);
                }
            }
            break;

        default:
            break;
        }
    }

Error:

    if (hClientProc != NULL)
    {
        CloseHandle(hClientProc);
    }

    return mmRet;
}
