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
#include <wavedev.h>
#include <mmddk.h>
#include <ceddk.h>
#include <ceddkex.h>
#include "debug.h"
#include "wavemain.h"
#include "audioctrl.h"
#include "Audiolin.h"
#include "mixermgr.h"
#include "audiomgr.h"
#include "strmmgr.h"
#include "ostrmmgr.h"
#include "strmctxt.h"


//------------------------------------------------------------------------------
// initialize_AudioLine, wave-in destination audio line
//
BOOL
COutputStreamManager::initialize_AudioLine(
    CAudioMixerManager *pAudioMixerManager
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+COutputStreamManager::initialize_AudioLine()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:+COutputStreamManager::initialize_AudioLine()\r\n"));

    // register audio controls
    //
    register_AudioControl(&m_VolumeControl);
    register_AudioControl(&m_MuteControl);

    // register controls as default wavout volume and mute controls
    //
    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kWavOutVolumeControl, &m_VolumeControl);

    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kWavOutMuteControl, &m_MuteControl);

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-COutputStreamManager::initialize_AudioLine()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:-COutputStreamManager::initialize_AudioLine()\r\n"));

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  get_DevCaps, return wave input device caps
//
DWORD
COutputStreamManager::get_DevCaps(
    LPVOID pCaps,
    DWORD dwSize
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+COutputStreamManager::GetDevCaps()\r\n"));

    static const WAVEOUTCAPS wc =
        {
        MM_MICROSOFT,
        24,
        0x0001,
        TEXT("Audio Output"),
        WAVE_FORMAT_1M08 | WAVE_FORMAT_2M08 | WAVE_FORMAT_4M08 |
        WAVE_FORMAT_1S08 | WAVE_FORMAT_2S08 | WAVE_FORMAT_4S08 |
        WAVE_FORMAT_1M16 | WAVE_FORMAT_2M16 | WAVE_FORMAT_4M16 |
        WAVE_FORMAT_1S16 | WAVE_FORMAT_2S16 | WAVE_FORMAT_4S16,
        OUTCHANNELS,
        0,
        WAVECAPS_VOLUME | WAVECAPS_PLAYBACKRATE

//#if !defined(MONO_GAIN)
#if !(MONO_GAIN)
        | WAVECAPS_LRVOLUME
#endif

        };

    if (dwSize > sizeof(WAVEOUTCAPS))
        {
        dwSize = sizeof(WAVEOUTCAPS);
        }

    memcpy( pCaps, &wc, dwSize);

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-COutputStreamManager::GetDevCaps()\r\n"));

    return MMSYSERR_NOERROR;
}



//------------------------------------------------------------------------------
// get_ExtDevCaps not supported
//

DWORD
COutputStreamManager::get_ExtDevCaps(
    LPVOID pCaps,
    DWORD dwSize
    )
{

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+COutputStreamManager::GetExtDevCaps()\r\n"));

    // dev note: this value prevents the Windows CE software mixer from
    // allocating mixer memory. This driver does all mixing internally (was 0)
    //
    static const WAVEOUTEXTCAPS wec =
        {
        0x0000FFFF,                         // max number of hw-mixed streams
        0x0000FFFF,                         // available HW streams
        0,                                  // preferred sample rate for
                                            // software mixer (0 indicates no
                                            // preference)
        6,                                  // preferred buffer size for
                                            // software mixer (0 indicates no
                                            // preference)
        0,                                  // preferred number of buffers for
                                            // software mixer (0 indicates no
                                            // preference)
        8000,                               // minimum sample rate for a
                                            // hw-mixed stream
        48000                               // maximum sample rate for a
                                            // hw-mixed stream
        };

    if (dwSize > sizeof(WAVEOUTEXTCAPS))
        {
        dwSize = sizeof(WAVEOUTEXTCAPS);
        }

    memcpy(pCaps, &wec, dwSize);


    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-COutputStreamManager::GetExtDevCaps()\r\n"));

    return MMSYSERR_NOERROR;
}

//------------------------------------------------------------------------------
//  create_Stream, create input stream context.
//
StreamContext *
COutputStreamManager::create_Stream(
    LPWAVEOPENDESC lpWOD
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+COutputStreamManager::CreateStream()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG,
		(L"WAV:+COutputStreamManager::CreateStream(), lpWOD = 0x%X\r\n", lpWOD));

    LPWAVEFORMATEX lpFormat=lpWOD->lpFormat;

    if (lpFormat->nChannels==1)
    {
        if (lpFormat->wBitsPerSample==8)
        {
            return new OutputStreamContextM8;
        }
        else
        {
            return new OutputStreamContextM16;
        }
    }
    else
    {
        if (lpFormat->wBitsPerSample==8)
        {
            return new OutputStreamContextS8;
        }
        else
        {
            return new OutputStreamContextS16;
        }
    }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-COutputStreamManager::CreateStream()\r\n"));

   RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:-COutputStreamManager::CreateStream()\r\n"));
}

//------------------------------------------------------------------------------
//  copy_AudioData, callback routines from hardware bridge
//

DWORD
COutputStreamManager::copy_AudioData(
    void* pStart,
    DWORD nSize
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+COutputStreamManager::copy_AudioData()\r\n"));

	RETAILMSG(/*WAVE_AUDIO_DBG*/ 0, (L"WAV:+%S\r\n",__FUNCTION__));

    // zero-out buffer before copying data
    memset(pStart, 0, nSize);

    DWORD dwActiveStreams = CStreamManager::copy_AudioData(pStart, nSize);

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-COutputStreamManager::copy_AudioData()\r\n"));

	RETAILMSG(/*WAVE_AUDIO_DBG*/ 0, (L"WAV:-%S\r\n",__FUNCTION__));
    return dwActiveStreams;
}


//------------------------------------------------------------------------------
//  StreamReadyToRender, start playback of wave output stream
//
void
COutputStreamManager::StreamReadyToRender(
    StreamContext *pStreamContext
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+COutputStreamManager::StreamReadyToRender()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    get_HardwareAudioBridge()->start_AudioPort(CHardwareAudioBridge::kOutput);
    get_HardwareAudioBridge()->start_Stream(CHardwareAudioBridge::kOutput, pStreamContext);

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-COutputStreamManager::StreamReadyToRender()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
    return;
}


//------------------------------------------------------------------------------
// get_AudioValue, retrieve audio properties
//
DWORD
COutputStreamManager::get_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag
    ) const
{
    DWORD mmRet = MMSYSERR_NOERROR;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+COutputStreamManager::get_AudioValue()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));
    // determine what type of control generated the event
    // and forward to the destination audio line
    switch (pControl->get_ControlType())
    {
    case MIXERCONTROL_CONTROLTYPE_VOLUME:
        {
            DWORD dwGain;
            DWORD dwGainL, dwGainR;

            MIXERCONTROLDETAILS_UNSIGNED *pValue;
            pValue = (MIXERCONTROLDETAILS_UNSIGNED *) pDetail->paDetails;

            // Get gain.
            dwGain = get_Gain();

            // Low word is left channel.
            dwGainL = dwGain & 0xFFFF;

            // Get right channel.
#if (MONO_GAIN)
            dwGainR = dwGainL;
#else
            dwGainR = (dwGain >> 16) & 0xFFFF;
#endif

            switch (pDetail->cChannels)
            {
            case 2:
                // Return left and right channels separately.
                pValue[0].dwValue = dwGainL;
                pValue[1].dwValue = dwGainR;
                break;
            case 1:
                // Return the average of left and right channels.
                pValue[0].dwValue = (dwGainL + dwGainR) / 2;
                break;
            default:
                mmRet = MMSYSERR_INVALPARAM;
                goto Error;
            }
         RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S, val_0 = 0x%X, val_1 = 0x%X\r\n",__FUNCTION__, pValue[0].dwValue, pValue[1].dwValue)); 
        }
        break;

    case MIXERCONTROL_CONTROLTYPE_MUTE:
        {
            MIXERCONTROLDETAILS_BOOLEAN *pMute;
            pMute = (MIXERCONTROLDETAILS_BOOLEAN *) pDetail->paDetails;
            pMute[0].fValue = get_Mute();
        }
        break;

    default:
        mmRet = MMSYSERR_NOTSUPPORTED;
        break;
    }

Error:

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-COutputStreamManager::get_AudioValue()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
    return mmRet;
}


//------------------------------------------------------------------------------
// put_AudioValue, set audio properties
//
DWORD
COutputStreamManager::put_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag
    )
{
    DWORD mmRet = MMSYSERR_NOERROR;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+COutputStreamManager::put_AudioValue()\r\n"));

    RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));  

    // determine what type of control generated the event
    // and forward to the destination audio line
    switch (pControl->get_ControlType())
    {
    case MIXERCONTROL_CONTROLTYPE_VOLUME:
        {
            DWORD dwGain;
            DWORD dwGainL, dwGainR;

	        RETAILMSG(WAVE_AUDIO_DBG, (L"put_AudioValue() - MIXERCONTROL_CONTROLTYPE_VOLUME\r\n"));

            MIXERCONTROLDETAILS_UNSIGNED *pValue;
            pValue = (MIXERCONTROLDETAILS_UNSIGNED *) pDetail->paDetails;

            // Get left channel volume
            dwGainL = pValue[0].dwValue;

            // Get right channel volume
            switch (pDetail->cChannels)
            {
            case 2:
                // If setting is stereo, get right channel volume.
                dwGainR = pValue[1].dwValue;
                break;
            case 1:
                // If setting is mono, apply the same volume to both channels.
                dwGainR = dwGainL;
                break;
            default:
                mmRet = MMSYSERR_INVALPARAM;
                goto Error;
            }

			RETAILMSG(WAVE_AUDIO_DBG, (L"put_AudioValue(), dwGainL = 0x%X, dwGainR = 0x%X\r\n", dwGainL, dwGainR ));

            // Validate max setting.
            if ((dwGainL > LOGICAL_VOLUME_MAX) || (dwGainR > LOGICAL_VOLUME_MAX))
            {
                mmRet = MMSYSERR_INVALPARAM;
                goto Error;
            }

#if (MONO_GAIN)
            // If using mono gain, take average.
            dwGain = (dwGainL + dwGainR) / 2;
#else
            // Low word is left channel and high word is right channel.
            dwGain = (dwGainR << 16) | dwGainL;
#endif

            // Set gain
			RETAILMSG(WAVE_AUDIO_DBG, (L"put_AudioValue(), dwGain = 0x%X\r\n", dwGain ));
            mmRet = put_Gain(dwGain);
        }
        break;

    case MIXERCONTROL_CONTROLTYPE_MUTE:
        {
            MIXERCONTROLDETAILS_BOOLEAN *pMute;

			RETAILMSG(WAVE_AUDIO_DBG, (L"put_AudioValue() - MIXERCONTROL_CONTROLTYPE_MUTE\r\n"));
            pMute = (MIXERCONTROLDETAILS_BOOLEAN *) pDetail->paDetails;
            mmRet = put_Mute(pMute[0].fValue);
        }
        break;

    default:
        mmRet = MMSYSERR_NOTSUPPORTED;
        break;
    }

Error:

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-COutputStreamManager::put_AudioValue()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S, ret = %d\r\n",__FUNCTION__, mmRet));

    return mmRet;
}


//------------------------------------------------------------------------------
//
//  Function: OutputCStreamManagerIsSupportedFormat
//
//

BOOL
COutputStreamManager::IsSupportedFormat(LPWAVEFORMATEX lpFormat)
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-COutputStreamManager::IsSupportedFormat()\r\n"));

    return CStreamManager::IsSupportedFormat(lpFormat);
}

//------------------------------------------------------------------------------
//
//  Function: OutputCStreamManager::GetProperty
//
//
DWORD
COutputStreamManager::GetProperty(PWAVEPROPINFO pPropInfo)
{
    // Call base class.
    DWORD mmRet = CStreamManager::GetProperty(pPropInfo);

    // Forward to hardware context if not supported.
    if (MMSYSERR_NOTSUPPORTED == mmRet)
    {
        mmRet = GetOutputProperty(pPropInfo);
    }

    return mmRet;
}

//------------------------------------------------------------------------------
//
//  Function: OutputCStreamManager::SetProperty
//
//
DWORD
COutputStreamManager::SetProperty(PWAVEPROPINFO pPropInfo)
{
    // Call base class.
    DWORD mmRet = CStreamManager::SetProperty(pPropInfo);

    // Forward to hardware context if not supported.
    if (MMSYSERR_NOTSUPPORTED == mmRet)
    {
        mmRet = SetOutputProperty(pPropInfo);
    }

    return mmRet;
}


