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
//
#ifndef __STRMMGR_H__
#define __STRMMGR_H__

#include <linklist.h>
#include "StreamClass.h"
#include "audiolin.h"
#include "hwaudiobrdg.h"
#include "wavertgctrl.h"
#include "dtpnotify.h"

class CAudioManager;
class StreamContext;

// by default attenuation settings are adjusted for
// Windows CE software mixer.
// However, depending on the hardware design the defaults
// may need adjustment

class CStreamManager : public CStreamCallback
{
public:
    CStreamManager();

    virtual BOOL IsSupportedFormat(LPWAVEFORMATEX lpFormat);
    PBYTE TransferBuffer(PBYTE pBuffer, PBYTE pBufferEnd, DWORD *pNumStreams);

    void NewStream(StreamContext *pStreamContext);
    void DeleteStream(StreamContext *pStreamContext);

    DWORD get_Gain() const
    {
        return m_dwGain;
    }

    DWORD put_Gain(DWORD dwGain)
    {
        m_dwGain = dwGain;
		RETAILMSG(0, (L"CStreamManager::put_Gain,  m_dwGain = 0x%X\r\n", m_dwGain));
        update_Streams();
        return MMSYSERR_NOERROR;
    }

    DWORD get_Mute() const
    {
        return m_bMute;
    }

    DWORD put_Mute(BOOL Mute)
    {
        m_bMute = Mute;
        update_Streams();
        return MMSYSERR_NOERROR;
    }

    BOOL get_Mono() const
    {
        return m_bMono;
    }

    DWORD put_Mono(BOOL Mono)
    {
        m_bMono = Mono;
        update_Streams();
        return MMSYSERR_NOERROR;
    }

    DWORD get_DefaultStreamGain()
    {
        return m_dwDefaultStreamGain;
    }

    DWORD put_DefaultStreamGain(DWORD dwGain)
    {
        m_dwDefaultStreamGain = dwGain;
        return MMSYSERR_NOERROR;
    }

    DWORD GetClassGain(DWORD dwClassId);
    DWORD SetClassGain(DWORD dwClassId, DWORD dwClassGain);
    BOOL  IsBypassDeviceGain(DWORD dwClassId);
    BOOL  IsValidClassId(DWORD dwClassId);
    BOOL  GetClassProperty(DWORD dwClassId, PDWORD pdwClassGain, PBOOL pfBypassDeviceGain);

    static void SetAttenMax(DWORD StreamAtten, DWORD DeviceAtten, DWORD ClassAtten)
    {
        m_dwStreamAttenMax = StreamAtten;
        m_dwDeviceAttenMax = DeviceAtten;
        m_dwClassAttenMax  = ClassAtten;
    }

    DWORD get_StreamAttenMax() const
    {
        return m_dwStreamAttenMax;
    }

    DWORD get_DeviceAttenMax() const
    {
        return m_dwDeviceAttenMax;
    }

    DWORD get_ClassAttenMax() const
    {
        return m_dwClassAttenMax;
    }

    DWORD get_CurrentSampleRate() const
    {
        return m_prgSampleRates[m_idxSampleRate].SampleRate;
    }

    DWORD get_CurrentInvSampleRate() const
    {
        return m_prgSampleRates[m_idxSampleRate].InvSampleRate;
    }

    void put_AudioSampleRate(AudioSampleRate_e rate)
    {
        m_idxSampleRate = rate;
        update_Streams();
    }

    void put_AudioManager(CAudioManager* pAudioManager)
    {
        m_pAudioManager = pAudioManager;
    }

    DWORD open_Stream(LPWAVEOPENDESC lpWOD, DWORD dwFlags, StreamContext **ppStreamContext);

    virtual BOOL update_Streams();
    virtual DWORD copy_AudioData(void* pStart, DWORD nSize);
    virtual DWORD copy_StreamData(HANDLE hContext, void* pStart, DWORD nSize);
    virtual CHardwareAudioBridge* get_HardwareAudioBridge() const;

    virtual DWORD get_ExtDevCaps(PVOID pCaps, DWORD dwSize)=0;
    virtual DWORD get_DevCaps(PVOID pCaps, DWORD dwSize)=0;
    virtual void StreamReadyToRender(StreamContext *pStreamContext)=0;

    virtual StreamContext *create_Stream(LPWAVEOPENDESC lpWOD)=0;

    virtual DWORD GetProperty(PWAVEPROPINFO pPropInfo);
    virtual DWORD SetProperty(PWAVEPROPINFO pPropInfo);

public:
    // Endpoint notifications.
    DWORD NotifyHeadsetAttached(BOOL fAttached);
    DWORD NotifyHeadphonesAttached(BOOL fAttached);
    DWORD NotifyCarKitAttached(BOOL fAttached);
    DWORD NotifyBthScoEnabled(BOOL fEnabled);

protected:
    // Device topology.
    DWORD GetOutputDeviceDescriptor(PDTP_DEVICE_DESCRIPTOR pDeviceDescriptor);
    DWORD GetInputDeviceDescriptor(PDTP_DEVICE_DESCRIPTOR pDeviceDescriptor);
    DWORD GetOutputEndpointDescriptor(DWORD dwIndex, PDTP_ENDPOINT_DESCRIPTOR pEndpointDescriptor);
    DWORD GetInputEndpointDescriptor(DWORD dwIndex, PDTP_ENDPOINT_DESCRIPTOR pEndpointDescriptor);

    DWORD SendEndpointChangeNotification(DtpNotify *pDtpNotify, BOOL fAttached, DWORD dwEpIdx);

/*
    // Not supported, cannot find definition for PRTGCTRL_ENDPOINT_ROUTING
    // System and cellular audio routing control.
    DWORD RouteSystemOutputEndpoints(PRTGCTRL_ENDPOINT_ROUTING pEndpointRouting);
    DWORD RouteSystemInputEndpoints(PRTGCTRL_ENDPOINT_ROUTING pEndpointRouting);
    DWORD RouteCellularOutputEndpoints(PRTGCTRL_ENDPOINT_ROUTING pEndpointRouting);
    DWORD RouteCellularInputEndpoints(PRTGCTRL_ENDPOINT_ROUTING pEndpointRouting);
*/

protected:
    DWORD CommonGetProperty(PWAVEPROPINFO pPropInfo, BOOL fInput);
    DWORD CommonSetProperty(PWAVEPROPINFO pPropInfo, BOOL fInput);

protected:
    LIST_ENTRY  m_StreamList;         // List of streams rendering to/from this device
    DWORD       m_dwGain;
    BOOL        m_bMute;
    BOOL        m_bMono;
    DWORD       m_dwDefaultStreamGain;
    StreamClassTable m_StreamClassTable;

    DWORD               m_idxSampleRate;
    SampleRateEntry_t  *m_prgSampleRates;


    static DWORD m_dwStreamAttenMax;
    static DWORD m_dwDeviceAttenMax;
    static DWORD m_dwClassAttenMax;

    CAudioManager *m_pAudioManager;

    BOOL m_fHeadsetAttached;                                // Headset is attached.
    BOOL m_fHeadphonesAttached;                             // Headphones (stereo with no-mic) is attached.
    BOOL m_fCarKitAttached;                                 // Car kit is attached.
    BOOL m_fBthScoEnabled;                                  // Bluetooth SCO control is enabled.

    //------------------------------------------------------------------------------------

    DtpNotify m_OutputDtpNotify;                            // Output device topology notifications.
    DtpNotify m_InputDtpNotify;                             // Input device topology notifications.

#ifdef PROFILE_MIXER
public:
    void StartMixerProfiler()
    {
        // reset Counter for this specific output
        m_liPCTotal.QuadPart = 0;
        QueryPerformanceCounter(&m_liPCStart);
    }
    void StopMixerProfiler( LARGE_INTEGER *pliTotalTime, LARGE_INTEGER *pliMixerTime)
    {
        LARGE_INTEGER liPCStop;
        QueryPerformanceCounter(&liPCStop);
        liPCStop.QuadPart -= m_liPCStart.QuadPart;
        m_liPCStart.QuadPart = liPCStop.QuadPart;
        pliTotalTime->QuadPart = (1000*liPCStop.QuadPart)/m_liPCFrequency.QuadPart;
        pliMixerTime->QuadPart = (1000*m_liPCTotal.QuadPart)/m_liPCFrequency.QuadPart;
    }
    LARGE_INTEGER m_liPCStart;
    LARGE_INTEGER m_liPCTotal;
    LARGE_INTEGER m_liPCFrequency;
#endif
};





#endif //__STRMMGR_H__

