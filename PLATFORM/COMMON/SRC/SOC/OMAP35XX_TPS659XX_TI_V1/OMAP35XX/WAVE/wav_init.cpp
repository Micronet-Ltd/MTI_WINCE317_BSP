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
//  intialization routine.
//

#include <windows.h>
//#include <bsp_version.h>
//#include <bsp.h>
#include <wavedev.h>
#include <mmreg.h>
#include <pm.h>
#include <mmddk.h>
#include <ceddk.h>
#include <ceddkex.h>
//#include <wavext.h>
//#include <gpio.h>

#include "wavemain.h"
#include "audioctrl.h"
#include "audiolin.h"
#include "mixermgr.h"
#include "audiomgr.h"
#include "istrmmgr.h"
#include "ostrmmgr.h"
#include "vostrmmgr.h"
#include "btstrmmgr.h"
#include "eastrmmgr.h"
#include "omap35xx_hwbridge.h"
#include <wave_audio.h>

#include "audioControl.h"

#if defined(SYSGEN_BTH_AG)
#include "BluetoothAudio.h"
#endif

#include <oal.h>
#include <oalex.h>
#include <args.h>
#include "debug.h"

//------------------------------------------------------------------------------

struct AudioDriverInitInfo_t
{
    DWORD                   StreamAttenMax;
    DWORD                   DeviceAttenMax;
    DWORD                   SecondaryAttenMax;
    DWORD                   dwAudioProfile;
    DWORD                   dwNumOfPlayChannels;
    DWORD                   requestedPlayChannels[MAX_HW_CODEC_CHANNELS];
    DWORD                   dwNumOfRecChannels;
    DWORD                   requestedRecChannels[MAX_HW_CODEC_CHANNELS];
    PortConfigInfo_t       *pPlayPortConfigInfo;
    PortConfigInfo_t       *pRecPortConfigInfo;
    WCHAR                   szDMTPortDriver[16];
    WCHAR                   szSerialPortDriver[16];
    WCHAR                   szHwCodecAdapterPath[MAX_PATH];
    HwCodecConfigInfo_t    *pHwCodecConfigInfo;
    DWORD                   dwHwCodecInMainMicDigitalGain;
    DWORD                   dwHwCodecInSubMicDigitalGain;
    DWORD                   dwHwCodecInHeadsetMicDigitalGain;
    DWORD                   dwHwCodecInMainMicAnalogGain;
    DWORD                   dwHwCodecInSubMicAnalogGain;
    DWORD                   dwHwCodecInHeadsetMicAnalogGain;
    DWORD                   dwHwCodecOutStereoSpeakerDigitalGain;
    DWORD                   dwHwCodecOutStereoHeadsetDigitalGain;
    DWORD                   dwHwCodecOutHeadsetMicDigitalGain;
    DWORD                   dwHwCodecOutStereoSpeakerAnalogGain;
    DWORD                   dwHwCodecOutStereoHeadsetAnalogGain;
    DWORD                   dwHwCodecOutHeadsetMicAnalogGain;
    DWORD                   dwHwCodecInHeadsetAuxDigitalGain;
    DWORD                   dwHwCodecInHeadsetAuxAnalogGain;
    DWORD                   dwAudioRoute;
	// External Audio 
	DWORD                   dwHwCodecOutHeadsetDigitalGain;
    DWORD                   dwHwCodecOutHeadsetAnalogGain;
	// Bluetooth audio
#if defined(SYSGEN_BTH_AG)
	DWORD					dwHwCodecInBthAudioDigitalGain;
	DWORD					dwHwCodecOutBthAudioDigitalGain;
	DWORD					dwHwCodec_ARXL1PGA;
	DWORD					dwHwCodec_ATXR1PGA;
#endif
};

static WCHAR const *_pSerialPortDefault = _T("ICX9:");

static const
DEVICE_REGISTRY_PARAM s_deviceRegParams[] = {
    {
        L"StreamAttenMax", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, StreamAttenMax),
        fieldsize(AudioDriverInitInfo_t, StreamAttenMax),
        (VOID*)STREAM_ATTEN_MAX
    }, {
        L"DeviceAttenMax", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, DeviceAttenMax),
        fieldsize(AudioDriverInitInfo_t, DeviceAttenMax),
        (VOID*)DEVICE_ATTEN_MAX
    }, {
        L"SecondaryAttenMax", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, SecondaryAttenMax),
        fieldsize(AudioDriverInitInfo_t, SecondaryAttenMax),
        (VOID*)SECOND_ATTEN_MAX
    }, {
        L"ExternalPortDriver", PARAM_STRING, TRUE,
        offset(AudioDriverInitInfo_t, szDMTPortDriver),
        fieldsize(AudioDriverInitInfo_t, szDMTPortDriver),
        NULL
    }, {
        L"SerialPortDriver", PARAM_STRING, FALSE,
        offset(AudioDriverInitInfo_t, szSerialPortDriver),
        fieldsize(AudioDriverInitInfo_t, szSerialPortDriver),
        (VOID*)_pSerialPortDefault
    }, {
        L"AudioProfile", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwAudioProfile),
        fieldsize(AudioDriverInitInfo_t, dwAudioProfile),
        NULL
    }, {
        L"NumOfPlayChannels", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwNumOfPlayChannels),
        fieldsize(AudioDriverInitInfo_t, dwNumOfPlayChannels),
        NULL
    }, {
        L"EnableAudioPlayChannels", PARAM_MULTIDWORD, FALSE,
        offset(AudioDriverInitInfo_t, requestedPlayChannels),
        fieldsize(AudioDriverInitInfo_t, requestedPlayChannels),
        NULL
    }, {
        L"NumOfRecChannels", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwNumOfRecChannels),
        fieldsize(AudioDriverInitInfo_t, dwNumOfRecChannels),
        NULL
    }, {
        L"EnableAudioRecChannels", PARAM_MULTIDWORD, FALSE,
        offset(AudioDriverInitInfo_t, requestedRecChannels),
        fieldsize(AudioDriverInitInfo_t, requestedRecChannels),
        NULL
    }, {
        L"DASFHwCodecAdpaterPath", PARAM_STRING, FALSE,
        offset(AudioDriverInitInfo_t, szHwCodecAdapterPath),
        fieldsize(AudioDriverInitInfo_t, szHwCodecAdapterPath),
        NULL
    }, {
        WAV_DEV_REG_MIC_DGAIN, PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodecInMainMicDigitalGain),
        fieldsize(AudioDriverInitInfo_t, dwHwCodecInMainMicDigitalGain),
        NULL
    }, {
        L"HwCodecInSubMicDigitalGain", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodecInSubMicDigitalGain),
        fieldsize(AudioDriverInitInfo_t, dwHwCodecInSubMicDigitalGain),
        NULL
    }, {
        L"HwCodecInHeadsetMicDigitalGain", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodecInHeadsetMicDigitalGain),
        fieldsize(AudioDriverInitInfo_t, dwHwCodecInHeadsetMicDigitalGain),
        NULL
    }, {
        WAV_DEV_REG_MIC_AGAIN, PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodecInMainMicAnalogGain),
        fieldsize(AudioDriverInitInfo_t, dwHwCodecInMainMicAnalogGain),
        NULL
    }, {
        L"HwCodecInSubMicAnalogGain", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodecInSubMicAnalogGain),
        fieldsize(AudioDriverInitInfo_t, dwHwCodecInSubMicAnalogGain),
        NULL
    }, {
        L"HwCodecInHeadsetMicAnalogGain", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodecInHeadsetMicAnalogGain),
        fieldsize(AudioDriverInitInfo_t, dwHwCodecInHeadsetMicAnalogGain),
        NULL
    }, {
        L"HwCodecOutStereoSpeakerDigitalGain", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodecOutStereoSpeakerDigitalGain),
        fieldsize(AudioDriverInitInfo_t, dwHwCodecOutStereoSpeakerDigitalGain),
        NULL
    }, {
        L"HwCodecOutStereoHeadsetDigitalGain", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodecOutStereoHeadsetDigitalGain),
        fieldsize(AudioDriverInitInfo_t, dwHwCodecOutStereoHeadsetDigitalGain),
        NULL
    }, {
        L"HwCodecOutHeadsetMicDigitalGain", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodecOutHeadsetMicDigitalGain),
        fieldsize(AudioDriverInitInfo_t, dwHwCodecOutHeadsetMicDigitalGain),
        NULL
    }, {
        L"HwCodecOutHeadsetDigitalGain", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodecOutHeadsetDigitalGain),
        fieldsize(AudioDriverInitInfo_t, dwHwCodecOutHeadsetDigitalGain),
        NULL
    }, {
        L"HwCodecOutStereoSpeakerAnalogGain", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodecOutStereoSpeakerAnalogGain),
        fieldsize(AudioDriverInitInfo_t, dwHwCodecOutStereoSpeakerAnalogGain),
        NULL
    }, {
        L"HwCodecOutStereoHeadsetAnalogGain", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodecOutStereoHeadsetAnalogGain),
        fieldsize(AudioDriverInitInfo_t, dwHwCodecOutStereoHeadsetAnalogGain),
        NULL
    }, {
        L"HwCodecOutHeadsetMicAnalogGain", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodecOutHeadsetMicAnalogGain),
        fieldsize(AudioDriverInitInfo_t, dwHwCodecOutHeadsetMicAnalogGain),
        NULL
    }, {
        L"HwCodecOutHeadsetAnalogGain", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodecOutHeadsetAnalogGain),
        fieldsize(AudioDriverInitInfo_t, dwHwCodecOutHeadsetAnalogGain),
        NULL
    }, {
        L"HwCodecInHeadsetAuxDigitalGain", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodecInHeadsetAuxDigitalGain),
        fieldsize(AudioDriverInitInfo_t, dwHwCodecInHeadsetAuxDigitalGain),
        NULL
    }, {
        L"HwCodecInHeadsetAuxAnalogGain", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodecInHeadsetAuxAnalogGain),
        fieldsize(AudioDriverInitInfo_t, dwHwCodecInHeadsetAuxAnalogGain),
        NULL
    }, {
        L"AudioRoute", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwAudioRoute),
        fieldsize(AudioDriverInitInfo_t, dwAudioRoute),
        (VOID*)OMAP35XX_HwAudioBridge::kAudioRoute_Handset
    }, 
#if defined(SYSGEN_BTH_AG)
    {
        L"HwCodecInBthAudioDigitalGain", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodecInBthAudioDigitalGain),
        fieldsize(AudioDriverInitInfo_t, dwHwCodecInBthAudioDigitalGain),
        NULL
    }, {
        L"HwCodecOutBthAudioDigitalGain", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodecOutBthAudioDigitalGain),
        fieldsize(AudioDriverInitInfo_t, dwHwCodecOutBthAudioDigitalGain),
        NULL
    },
    {
        L"HwCodec_ARXL1PGA", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodec_ARXL1PGA),
        fieldsize(AudioDriverInitInfo_t, dwHwCodec_ARXL1PGA),
        NULL
    }, {
        L"HwCodec_ATXR1PGA", PARAM_DWORD, FALSE,
        offset(AudioDriverInitInfo_t, dwHwCodec_ATXR1PGA),
        fieldsize(AudioDriverInitInfo_t, dwHwCodec_ATXR1PGA),
        NULL
    },
#endif
 };
//------------------------------------------------------------------------------

static AudioDriverInitInfo_t    s_AudioDriverInitInfo;
static OMAP35XX_HwAudioBridge  s_HardwareBridge;
static COutputStreamManager     s_OutputStreamManager;
static CInputStreamManager      s_InputStreamManager;

static CVoiceStreamManager        s_CVoiceStreamManager;
static CBTVoiceStreamManager      s_CBTVoiceStreamManager;
static CBTPlaybackStreamManager   s_CBTPlaybackStreamManager;
static CExtAudioStreamManager     s_CExtAudioStreamManager;

#if defined(SYSGEN_BTH_AG)
static BluetoothAudioManager	s_BluetoothAudioManager;
#endif

//------------------------------------------------------------------------------
// InitializeMixers
//
EXTERN_C CAudioManager*
CreateAudioManager(
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CreateAudioManager()"));

    // register all destination audio lines
    //
    CAudioManager* pAudioManager = new CAudioManager();

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CreateAudioManager(CAudioManager*=0x%08X), pAudioManager")
        );

    return pAudioManager;
}


//------------------------------------------------------------------------------
// InitializeMixers
//
EXTERN_C void
DeleteAudioManager(
    CAudioManager* pAudioManager
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+DeleteAudioManager(CAudioManager*=0x%08X), pAudioManager")
        );

    if (pAudioManager)
        {
    delete pAudioManager;
        pAudioManager = NULL;
        }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-DeleteAudioManager()"));
}


//------------------------------------------------------------------------------
// InitializeHardware
//
EXTERN_C BOOL
InitializeHardware(
    LPCWSTR szContext,
    LPCVOID pBusContext
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+InitializeHardware(szContext*=0x%08X)", szContext)
        );

	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:+InitializeHardware(szContext*=0x%X)\r\n", szContext)
        );

    BOOL bResult = FALSE;
    UINT nCount = 0;
	card_ver m_cv = {0};
	UINT32 in = OAL_ARGS_QUERY_WLAN_CFG;

    // Read device parameters
    if (GetDeviceRegistryParams(
            szContext, &s_AudioDriverInitInfo, dimof(s_deviceRegParams),
            s_deviceRegParams) != ERROR_SUCCESS)
        {
        DEBUGMSG(ZONE_ERROR, (L"WAV: !ERROR "
            L"Failed read WAV driver registry parameters\r\n"
            ));
        goto cleanUp;
        }

    s_AudioDriverInitInfo.pHwCodecConfigInfo =
        (HwCodecConfigInfo_t *)LocalAlloc(LPTR, sizeof(HwCodecConfigInfo_t));
    if (s_AudioDriverInitInfo.pHwCodecConfigInfo == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"WAV: !ERROR "
            L"Failed to allocate gain settings structure\r\n"
            ));
        goto cleanUp;
        }

    // MemSet playback port config structure with 0
    //
    memset(s_AudioDriverInitInfo.pHwCodecConfigInfo , 0,
        sizeof(HwCodecConfigInfo_t)
        );

    // Populate Hw Codec config info from registry
    //

    // Input config
    //
    s_AudioDriverInitInfo.pHwCodecConfigInfo->dwHwCodecInMainMicDigitalGain =
        s_AudioDriverInitInfo.dwHwCodecInMainMicDigitalGain;

    s_AudioDriverInitInfo.pHwCodecConfigInfo->dwHwCodecInSubMicDigitalGain =
        s_AudioDriverInitInfo.dwHwCodecInSubMicDigitalGain;

    s_AudioDriverInitInfo.pHwCodecConfigInfo->dwHwCodecInHeadsetMicDigitalGain =
        s_AudioDriverInitInfo.dwHwCodecInHeadsetMicDigitalGain;

    s_AudioDriverInitInfo.pHwCodecConfigInfo->dwHwCodecInMainMicAnalogGain =
        s_AudioDriverInitInfo.dwHwCodecInMainMicAnalogGain;

    s_AudioDriverInitInfo.pHwCodecConfigInfo->dwHwCodecInSubMicAnalogGain =
        s_AudioDriverInitInfo.dwHwCodecInSubMicAnalogGain;

    s_AudioDriverInitInfo.pHwCodecConfigInfo->dwHwCodecInHeadsetMicAnalogGain =
        s_AudioDriverInitInfo.dwHwCodecInHeadsetMicAnalogGain;

    s_AudioDriverInitInfo.pHwCodecConfigInfo->dwHwCodecInHeadsetAuxDigitalGain =
        s_AudioDriverInitInfo.dwHwCodecInHeadsetAuxDigitalGain;

    s_AudioDriverInitInfo.pHwCodecConfigInfo->dwHwCodecInHeadsetAuxAnalogGain =
        s_AudioDriverInitInfo.dwHwCodecInHeadsetAuxAnalogGain;

    // Output config
    //
    s_AudioDriverInitInfo.pHwCodecConfigInfo->
        dwHwCodecOutStereoSpeakerDigitalGain =
        s_AudioDriverInitInfo.dwHwCodecOutStereoSpeakerDigitalGain;

    s_AudioDriverInitInfo.pHwCodecConfigInfo->
        dwHwCodecOutStereoHeadsetDigitalGain =
        s_AudioDriverInitInfo.dwHwCodecOutStereoHeadsetDigitalGain;

    s_AudioDriverInitInfo.pHwCodecConfigInfo->
        dwHwCodecOutHeadsetMicDigitalGain =
        s_AudioDriverInitInfo.dwHwCodecOutHeadsetMicDigitalGain;

    s_AudioDriverInitInfo.pHwCodecConfigInfo->dwHwCodecOutHeadsetDigitalGain = s_AudioDriverInitInfo.dwHwCodecOutHeadsetDigitalGain;

	s_AudioDriverInitInfo.pHwCodecConfigInfo->
        dwHwCodecOutStereoSpeakerAnalogGain =
        s_AudioDriverInitInfo.dwHwCodecOutStereoSpeakerAnalogGain;

    s_AudioDriverInitInfo.pHwCodecConfigInfo->
        dwHwCodecOutStereoHeadsetAnalogGain =
        s_AudioDriverInitInfo.dwHwCodecOutStereoHeadsetAnalogGain;

    s_AudioDriverInitInfo.pHwCodecConfigInfo->
        dwHwCodecOutHeadsetMicAnalogGain =
        s_AudioDriverInitInfo.dwHwCodecOutHeadsetMicAnalogGain;

    s_AudioDriverInitInfo.pHwCodecConfigInfo->dwHwCodecOutHeadsetAnalogGain = s_AudioDriverInitInfo.dwHwCodecOutHeadsetAnalogGain;
#if defined(SYSGEN_BTH_AG)
	// Bluetooth audio
	s_AudioDriverInitInfo.pHwCodecConfigInfo->bBthAudio = TRUE;

	if (!KernelIoControl(IOCTL_HAL_QUERY_CARD_CFG, &in, sizeof(in), &m_cv, sizeof(m_cv), 0))
	{
		m_cv.card_addr	= -1;
		m_cv.ver		= -1;
		m_cv.rev		= -1;
		m_cv.config		= -1;
	}

	if (m_cv.ver == -1 || m_cv.ver < 300)
        s_AudioDriverInitInfo.pHwCodecConfigInfo->bBthAudio = FALSE;

	RETAILMSG(1, (L"WAV:+%S, bBthAudio = %d\r\n",__FUNCTION__, s_AudioDriverInitInfo.pHwCodecConfigInfo->bBthAudio));

	if (s_AudioDriverInitInfo.pHwCodecConfigInfo->bBthAudio)
	{
		s_AudioDriverInitInfo.pHwCodecConfigInfo->dwHwCodecInBthAudioDigitalGain = s_AudioDriverInitInfo.dwHwCodecInBthAudioDigitalGain;
		s_AudioDriverInitInfo.pHwCodecConfigInfo->dwHwCodecOutBthAudioDigitalGain = s_AudioDriverInitInfo.dwHwCodecOutBthAudioDigitalGain;
		s_AudioDriverInitInfo.pHwCodecConfigInfo->dwHwCodec_ARXL1PGA = s_AudioDriverInitInfo.dwHwCodec_ARXL1PGA;
		s_AudioDriverInitInfo.pHwCodecConfigInfo->dwHwCodec_ATXR1PGA = s_AudioDriverInitInfo.dwHwCodec_ATXR1PGA;
	}
#endif

    // Playback port config information
    //
    s_AudioDriverInitInfo.pPlayPortConfigInfo = 
        (PortConfigInfo_t *)LocalAlloc(LPTR, sizeof(PortConfigInfo_t)
        );
    if (s_AudioDriverInitInfo.pPlayPortConfigInfo == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"WAV: !ERROR "
            L"Failed to allocate gain settings structure\r\n"
            ));
        goto cleanUp;
        }

    // MemSet playback port config structure with 0
    //
    memset(s_AudioDriverInitInfo.pPlayPortConfigInfo, 0,
        sizeof(PortConfigInfo_t)
        );

    s_AudioDriverInitInfo.pPlayPortConfigInfo->numOfChannels = s_AudioDriverInitInfo.dwNumOfPlayChannels;
    s_AudioDriverInitInfo.pPlayPortConfigInfo->portProfile = s_AudioDriverInitInfo.dwAudioProfile;
    
    for ( nCount= 0; nCount < MAX_HW_CODEC_CHANNELS; nCount++)
        {
        s_AudioDriverInitInfo.pPlayPortConfigInfo->requestedChannels[nCount] = 
            s_AudioDriverInitInfo.requestedPlayChannels[nCount];
        }

    // Record port config information
    //
    s_AudioDriverInitInfo.pRecPortConfigInfo = 
	    (PortConfigInfo_t *)LocalAlloc(LPTR, sizeof(PortConfigInfo_t));

    if (s_AudioDriverInitInfo.pRecPortConfigInfo == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"WAV: !ERROR "
            L"Failed to allocate gain settings structure\r\n"
            ));
        goto cleanUp;
        }

    // MemSet record port config structure with 0
    //
    memset(s_AudioDriverInitInfo.pRecPortConfigInfo, 0,
        sizeof(PortConfigInfo_t)
        );

    s_AudioDriverInitInfo.pRecPortConfigInfo->numOfChannels = 
        s_AudioDriverInitInfo.dwNumOfRecChannels;

    s_AudioDriverInitInfo.pRecPortConfigInfo->portProfile = 
	    s_AudioDriverInitInfo.dwAudioProfile;

    for ( nCount= 0; nCount < MAX_HW_CODEC_CHANNELS; nCount++)
        {
        s_AudioDriverInitInfo.pRecPortConfigInfo->requestedChannels[nCount] = 
            s_AudioDriverInitInfo.requestedRecChannels[nCount];
        }

    bResult = TRUE;

    s_HardwareBridge.initialize(s_AudioDriverInitInfo.szDMTPortDriver,
        s_AudioDriverInitInfo.szHwCodecAdapterPath,
        s_AudioDriverInitInfo.pHwCodecConfigInfo,
        (HANDLE)s_AudioDriverInitInfo.pPlayPortConfigInfo, 
        (HANDLE)s_AudioDriverInitInfo.pRecPortConfigInfo,
		(OMAP35XX_HwAudioBridge::AudioRoute_e)s_AudioDriverInitInfo.dwAudioRoute
        );

cleanUp:

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-InitializeHardware()"));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-InitializeHardware(bResult = %d)\r\n", bResult));
    return bResult;
}

//------------------------------------------------------------------------------
// UninitializeHardware
//
EXTERN_C void UninitializeHardware(DWORD dwData)
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+UninitializeHardware(dwData=0x%08X)", dwData)
        );

	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:+UninitializeHardware(dwData=0x%X)\r\n", dwData)
        );

    // Free the allocated resources of T2 Hw codec configuration structure
    //
    if (s_AudioDriverInitInfo.pHwCodecConfigInfo)
        {
        LocalFree(s_AudioDriverInitInfo.pHwCodecConfigInfo);
        s_AudioDriverInitInfo.pHwCodecConfigInfo = NULL;
        }

    // Free play config info structure
    //
    if (s_AudioDriverInitInfo.pPlayPortConfigInfo != NULL)
        {
        LocalFree(s_AudioDriverInitInfo.pPlayPortConfigInfo);
        s_AudioDriverInitInfo.pPlayPortConfigInfo = NULL;
        }

    // Free record config info structure
    //
    if (s_AudioDriverInitInfo.pRecPortConfigInfo != NULL)
        {
        LocalFree(s_AudioDriverInitInfo.pRecPortConfigInfo);
        s_AudioDriverInitInfo.pRecPortConfigInfo = NULL;
        }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-UninitializeHardware()"));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-UninitializeHardware()\r\n"));
}


//------------------------------------------------------------------------------
// InitializeMixers
//
EXTERN_C void InitializeMixers(CAudioManager* pManager)
{

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+InitializeMixers()\r\n"));
	RETAILMSG(1/*WAVE_AUDIO_DBG*/, (L"WAV:+InitializeMixers(), pManager = 0x%X\r\n", pManager));

    // set sample rates
    //
    s_OutputStreamManager.put_AudioSampleRate(CStreamCallback::k44khz);
    s_InputStreamManager.put_AudioSampleRate(CStreamCallback::k44khz);
 
	// voice output
	//s_CVoiceStreamManager.put_AudioSampleRate(CStreamCallback::k8khz);

    // Setup the hardware bridge
    //
    pManager->put_HardwareAudioBridge(&s_HardwareBridge);
    s_OutputStreamManager.put_AudioManager(pManager);
    s_InputStreamManager.put_AudioManager(pManager);

	// voice output
	s_CVoiceStreamManager.put_AudioManager(pManager);

	// BT Voice
	s_CBTVoiceStreamManager.put_AudioManager(pManager);

	// BT Playback
	s_CBTPlaybackStreamManager.put_AudioManager(pManager);


    s_HardwareBridge.put_StreamCallback(CHardwareAudioBridge::kInput,
        &s_InputStreamManager
        );

    s_HardwareBridge.put_StreamCallback(CHardwareAudioBridge::kOutput,
        &s_OutputStreamManager
        );

	// voice output
    s_HardwareBridge.put_StreamCallback(CHardwareAudioBridge::kVoiceOutput,
        &s_CVoiceStreamManager
        );

	// BT Voice
    s_HardwareBridge.put_StreamCallback(CHardwareAudioBridge::kBTVoice,
        &s_CBTVoiceStreamManager
        );

    // BT Playback 
    s_HardwareBridge.put_StreamCallback(CHardwareAudioBridge::kBTPlayback,
        &s_CBTPlaybackStreamManager
        );


    // Set audio gain maximums
    //
    CStreamManager::SetAttenMax(
        s_AudioDriverInitInfo.StreamAttenMax,
        s_AudioDriverInitInfo.DeviceAttenMax,
        s_AudioDriverInitInfo.SecondaryAttenMax
        );

    // input
    //
    pManager->put_InputStreamManager(&s_InputStreamManager);
    pManager->get_AudioMixerManager()->register_DestinationAudioLine(
        &s_InputStreamManager
        );

    // output
    //
    pManager->put_OutputStreamManager(&s_OutputStreamManager);
    pManager->get_AudioMixerManager()->register_DestinationAudioLine(
        &s_OutputStreamManager
        );

	// voice output
    //pManager->put_OutputStreamManager(&s_CVoiceStreamManager);
    pManager->get_AudioMixerManager()->register_DestinationAudioLine(
        &s_CVoiceStreamManager
        );

	if( !s_AudioDriverInitInfo.pHwCodecConfigInfo->bModem )
      s_CVoiceStreamManager.set_LineStatus( MIXERLINE_LINEF_DISCONNECTED );

	// BT voice
    pManager->get_AudioMixerManager()->register_DestinationAudioLine(
        &s_CBTVoiceStreamManager
        );

	if( !s_AudioDriverInitInfo.pHwCodecConfigInfo->bModem || !s_AudioDriverInitInfo.pHwCodecConfigInfo->bBT )
	  s_CBTVoiceStreamManager.set_LineStatus( MIXERLINE_LINEF_DISCONNECTED );

	// BT Playback 
    pManager->get_AudioMixerManager()->register_DestinationAudioLine(
        &s_CBTPlaybackStreamManager
        );

	//if( !s_AudioDriverInitInfo.pHwCodecConfigInfo->bBT )
	//  s_CBTVoiceStreamManager.set_LineStatus( MIXERLINE_LINEF_DISCONNECTED );

	// BT playback doesn't work good and was not promised.
	s_CBTPlaybackStreamManager.set_LineStatus( MIXERLINE_LINEF_DISCONNECTED );


    //RETAILMSG(1, (L"WAV:InitializeMixers(), extAudioRet = 0x%X\r\n", extAudioRet));

#if defined(SYSGEN_BTH_AG) // Bluetooth Audio
	s_BluetoothAudioManager.put_AudioManager(pManager);
    s_HardwareBridge.put_StreamCallback(CHardwareAudioBridge::kBluetoothAudio, &s_BluetoothAudioManager);
    pManager->get_AudioMixerManager()->register_DestinationAudioLine(&s_BluetoothAudioManager);
#endif

	if( s_AudioDriverInitInfo.pHwCodecConfigInfo->bExtAudio )
	{
     RETAILMSG(1, (L"WAV:InitializeMixers(), ExtAudioStreamManager init\r\n"));

	 // ExtAudio
	 s_CExtAudioStreamManager.put_AudioManager(pManager);
    // ExtAudio

     s_HardwareBridge.put_StreamCallback(CHardwareAudioBridge::kExtAudio,
        &s_CExtAudioStreamManager);

	 // ExtAudio 
     pManager->get_AudioMixerManager()->register_DestinationAudioLine(&s_CExtAudioStreamManager);
	}
	else
	{
     RETAILMSG(1, (L"WAV:%S, Ext Audio Card not present\r\n",__FUNCTION__));
     s_CExtAudioStreamManager.set_LineStatus( MIXERLINE_LINEF_DISCONNECTED );
	}


    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-InitializeMixers()\r\n"));
	RETAILMSG(1/*WAVE_AUDIO_DBG*/, (L"WAV:-InitializeMixers()\r\n"));
}
EXTERN_C void UpdateRegParams()
{
   // Read device parameters
    if(GetDeviceRegistryParams( WAV_DEV_REG_KEY, &s_AudioDriverInitInfo, dimof(s_deviceRegParams),
            s_deviceRegParams) != ERROR_SUCCESS)
    {
		DEBUGMSG(ZONE_ERROR, (L"WAV::UpdateRegParams !ERROR " L"Failed read WAV driver registry parameters\r\n"));
        return;
    }

	s_AudioDriverInitInfo.pHwCodecConfigInfo->dwHwCodecInMainMicDigitalGain =
		s_AudioDriverInitInfo.dwHwCodecInMainMicDigitalGain;

	s_AudioDriverInitInfo.pHwCodecConfigInfo->dwHwCodecInMainMicAnalogGain =
		s_AudioDriverInitInfo.dwHwCodecInMainMicAnalogGain;
}