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
#include "istrmmgr.h"
#include "strmctxt.h"
#include "hwaudiobrdg.h"
#include <omap35xx_hwbridge.h>
#include "tps659xx_wave.h"


static CMicrophoneInputSource  s_MicrophoneInputLine;
//static CVoiceInputSource  s_VoiceInputLine;

//------------------------------------------------------------------------------
// initialize_AudioLine, wave-in destination audio line
//
BOOL
CInputStreamManager::initialize_AudioLine(
    CAudioMixerManager *pAudioMixerManager
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CInputStreamManager::initialize_AudioLine()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:+CInputStreamManager::initialize_AudioLine()\r\n"));

    // register audio controls
    //
    register_AudioControl(&m_RecordVolumeControl);

    // register as the default wav-in volume control
    //
    // register controls as default wavout volume and mute controls
    //
    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kWavInVolumeControl, &m_RecordVolumeControl);

    // initialize paired audio source lines
    //
    s_MicrophoneInputLine.m_pInputStreamManager = this;
    pAudioMixerManager->register_SourceAudioLine(&s_MicrophoneInputLine, this);

    // initialize paired Voice source lines
    //
    //s_VoiceInputLine.m_pInputStreamManager = this;
    //pAudioMixerManager->register_SourceAudioLine(&s_VoiceInputLine, this);

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CInputStreamManager::initialize_AudioLine()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:-CInputStreamManager::initialize_AudioLine()\r\n"));

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  get_DevCaps, return wave input device caps
//
DWORD
CInputStreamManager::get_DevCaps(
    LPVOID pCaps,
    DWORD dwSize
    )
{
    static const WAVEINCAPS wc =
    {
        MM_MICROSOFT,
        23,
        0x0001,
        TEXT("Audio Input"),
        WAVE_FORMAT_1M08 | WAVE_FORMAT_2M08 | WAVE_FORMAT_4M08 |
        WAVE_FORMAT_1S08 | WAVE_FORMAT_2S08 | WAVE_FORMAT_4S08 |
        WAVE_FORMAT_1M16 | WAVE_FORMAT_2M16 | WAVE_FORMAT_4M16 |
        WAVE_FORMAT_1S16 | WAVE_FORMAT_2S16 | WAVE_FORMAT_4S16,
        1,
        0
    };

    if (dwSize > sizeof(WAVEINCAPS))
        dwSize = sizeof(WAVEINCAPS);

    memcpy( pCaps, &wc, dwSize);

    return MMSYSERR_NOERROR;
}



//------------------------------------------------------------------------------
// get_ExtDevCaps not supported
//

DWORD
CInputStreamManager::get_ExtDevCaps(
    LPVOID pCaps,
    DWORD dwSize
    )
{
    return MMSYSERR_NOTSUPPORTED;
}

//------------------------------------------------------------------------------
//  create_Stream, create input stream context.
//
StreamContext *
CInputStreamManager::create_Stream(
    LPWAVEOPENDESC lpWOD
    )
{
    return new InputStreamContext;
}


//------------------------------------------------------------------------------
//  copy_AudioData, callback routines from hardware bridge
//

DWORD
CInputStreamManager::copy_AudioData(
    void* pStart,
    DWORD nSize
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CInputStreamManager::copy_AudioData()\r\n"));

    DWORD dwActiveStreams = CStreamManager::copy_AudioData(pStart, nSize);
    if (0 == dwActiveStreams)
        {
        get_HardwareAudioBridge()->stop_Stream(CHardwareAudioBridge::kInput, NULL);
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CInputStreamManager::copy_AudioData()\r\n"));

    return dwActiveStreams;
}


//------------------------------------------------------------------------------
//  StreamReadyToRender, start capture of wave input stream
//

void
CInputStreamManager::StreamReadyToRender(
    StreamContext *pStreamContext
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CInputStreamManager::StreamReadyToRender()\r\n"));

    get_HardwareAudioBridge()->start_AudioPort(CHardwareAudioBridge::kInput);
    get_HardwareAudioBridge()->start_Stream(CHardwareAudioBridge::kInput, pStreamContext);

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CInputStreamManager::StreamReadyToRender()\r\n"));
    return;
}

//------------------------------------------------------------------------------
// get_AudioValue, retrieve microphone audio properties
//
DWORD
CInputStreamManager::get_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag
    ) const
{
    MMRESULT mmRet = MMSYSERR_NOERROR;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CInputStreamManager::get_AudioValue()\r\n"));

    ASSERT(pControl->get_ControlType() == MIXERCONTROL_CONTROLTYPE_VOLUME);

    MIXERCONTROLDETAILS_UNSIGNED *pValue;
    pValue = (MIXERCONTROLDETAILS_UNSIGNED *) pDetail->paDetails;

    // Validate requested number of channels.
    if (pDetail->cChannels != 1)
    {
        mmRet = MMSYSERR_INVALPARAM;
        goto Error;
    }

    DWORD dwGain;
    DWORD dwGainL, dwGainR;

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

    // Return the average of left and right channels.
    pValue[0].dwValue = (dwGainL + dwGainR) / 2;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CInputStreamManager::get_AudioValue()\r\n"));

Error:

    return mmRet;
}


//------------------------------------------------------------------------------
// put_AudioValue, set microphone audio properties
//
DWORD
CInputStreamManager::put_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag
    )
{
    MMRESULT mmRet = MMSYSERR_NOERROR;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CInputStreamManager::put_AudioValue()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:+CInputStreamManager::put_AudioValue()\r\n"));

    MIXERCONTROLDETAILS_UNSIGNED *pValue;
    pValue = (MIXERCONTROLDETAILS_UNSIGNED *) pDetail->paDetails;

    // Validate number of channels to set.
    if (pDetail->cChannels != 1)
    {
        mmRet = MMSYSERR_INVALPARAM;
        goto Error;
    }

    DWORD dwGain = pValue[0].dwValue;

    // Validate max setting.
    if (dwGain > LOGICAL_VOLUME_MAX)
    {
        mmRet = MMSYSERR_INVALPARAM;
        goto Error;
    }

#if !(MONO_GAIN)
    // Apply to left and right channels.
    dwGain = (dwGain << 16) | dwGain;
#endif

    mmRet = put_Gain(dwGain);

Error:

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CInputStreamManager::put_AudioValue(dwGain=0x%08X)\r\n", dwGain)
        );

	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:-CInputStreamManager::put_AudioValue(dwGain=0x%08X)\r\n", dwGain)
        );

    return mmRet;
}

//------------------------------------------------------------------------------
// GetProperty
//
DWORD
CInputStreamManager::GetProperty(PWAVEPROPINFO pPropInfo)
{
    // Call base class.
    DWORD mmRet = CStreamManager::GetProperty(pPropInfo);

    // Forward to hardware context if not supported.
    if (MMSYSERR_NOTSUPPORTED == mmRet)
    {
        mmRet = GetInputProperty(pPropInfo);
    }

    return mmRet;
}

//------------------------------------------------------------------------------
// SetProperty
//
DWORD
CInputStreamManager::SetProperty(PWAVEPROPINFO pPropInfo)
{
    // Call base class.
    DWORD mmRet = CStreamManager::SetProperty(pPropInfo);

    // Forward to hardware context if not supported.
    if (MMSYSERR_NOTSUPPORTED == mmRet)
    {
        mmRet = SetInputProperty(pPropInfo);
    }

    return mmRet;
}

//------------------------------------------------------------------------------
// initialize_AudioLine, microphone source audio line
//
BOOL
CMicrophoneInputSource::initialize_AudioLine(
    CAudioMixerManager *pAudioMixerManager
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CMicrophoneInputSource::initialize_AudioLine()\r\n"));

    // initialize audio controls
    //
    register_AudioControl(&m_MicMuteControl);
	register_AudioControl(&m_MicAnalogGainControl);
	register_AudioControl(&m_MicDigGainControl);

    // register as the default microphone mute control
    //
    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kMicMuteControl, &m_MicMuteControl);

    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kMicAnalogVolumeControl, &m_MicAnalogGainControl);

    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kMicDigitVolumeControl, &m_MicDigGainControl);


    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CMicrophoneInputSource::initialize_AudioLine()\r\n"));

    return TRUE;
}


//------------------------------------------------------------------------------
// get_AudioValue, retrieve microphone audio properties
//
DWORD
CMicrophoneInputSource::get_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag) const
{
	BOOL      micMute;
	WORD      mmRet = MMSYSERR_NOERROR;
	UINT16    gainMono;


    DEBUGMSG(ZONE_FUNCTION,(L"WAV:+CMicrophoneInputSource::get_AudioValue()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    //ASSERT(pControl->get_ControlType() == MIXERCONTROL_CONTROLTYPE_BOOLEAN);

	switch(pControl->get_ControlType())
	{
	  case MIXERCONTROL_CONTROLTYPE_MUTE:
	   {
			// query micrphone mute
			//
			MIXERCONTROLDETAILS_BOOLEAN * pValue =
						(MIXERCONTROLDETAILS_BOOLEAN * ) pDetail->paDetails;

			mmRet = (WORD) m_pInputStreamManager->get_HardwareAudioBridge()->GetModemMicMuteStatus(&micMute);
			if( mmRet == MMSYSERR_NOERROR )
			{
				switch (pDetail->cChannels)
				{
				case 1:
					pValue[0].fValue = micMute;
					break;
				default:
					mmRet = MMSYSERR_INVALPARAM;
					break;
				}
			}

			RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - GetModemMicMuteStatus = %d\r\n",__FUNCTION__, micMute));

	   } break;

	  case  MIXERCONTROL_CONTROLTYPE_DECIBELS:
	   {
	     MIXERCONTROLDETAILS_SIGNED  *pValue;

			pValue = (MIXERCONTROLDETAILS_SIGNED *) pDetail->paDetails;
			mmRet = (WORD)m_pInputStreamManager->get_HardwareAudioBridge()->GetModemMicAnalogGain(&gainMono);
			if( mmRet == MMSYSERR_NOERROR )
			{
				switch (pDetail->cChannels)
				{
				case 1:
					pValue[0].lValue = gainMono;
					break;
				default:
					mmRet = MMSYSERR_INVALPARAM;
					break;
				}
			}

          RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - GetModemMicAnalogGain = %d\r\n",__FUNCTION__, gainMono));

	   } break;

	  case MIXERCONTROL_CONTROLTYPE_VOLUME:
	   {
	     MIXERCONTROLDETAILS_UNSIGNED  *pValue;

			pValue = (MIXERCONTROLDETAILS_UNSIGNED *) pDetail->paDetails;
			mmRet = (WORD)m_pInputStreamManager->get_HardwareAudioBridge()->GetModemMicDigitalGain(&gainMono);
			if( mmRet == MMSYSERR_NOERROR )
			{
				switch (pDetail->cChannels)
				{
				case 1:
					pValue[0].dwValue = gainMono;
					break;
				default:
					mmRet = MMSYSERR_INVALPARAM;
					break;
				}
			}

			RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - GetModemMicDigitalGain = %d\r\n",__FUNCTION__, gainMono));

	   } break; 
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CMicrophoneInputSource::get_AudioValue()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));

    return mmRet;
}


//------------------------------------------------------------------------------
// put_AudioValue, set microphone audio properties
//
DWORD
CMicrophoneInputSource::put_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag)
{
	//BOOL    micMute;
	WORD    mmRet = MMSYSERR_NOERROR;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CMicrophoneInputSource::put_AudioValue()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    //ASSERT(pControl->get_ControlType() == MIXERCONTROL_CONTROLTYPE_MUTE);

	switch(pControl->get_ControlType())
	{
	  case MIXERCONTROL_CONTROLTYPE_MUTE:
	   {
		// set microphone mute
		//
		MIXERCONTROLDETAILS_BOOLEAN * pValue = (MIXERCONTROLDETAILS_BOOLEAN * ) pDetail->paDetails;

		// Get right channel volume
		switch (pDetail->cChannels)
		{
		 case 1:
			// If setting is mono, apply the same volume to both channels.
			break;
		 default:
			mmRet = MMSYSERR_INVALPARAM;
		}


		// Validate max setting.
		if (pValue[0].fValue > 1 )
		{
			mmRet = MMSYSERR_INVALPARAM;
		}

		// Set Mute
		if(  mmRet == MMSYSERR_NOERROR )
		{
		 mmRet = (WORD)m_pInputStreamManager->get_HardwareAudioBridge()->SetModemMicMute(pValue[0].fValue);
		}

		RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - SetModemMicMute = %d\r\n",__FUNCTION__, pValue[0].fValue));

	   } break;

	  case  MIXERCONTROL_CONTROLTYPE_DECIBELS:
	   {
	     MIXERCONTROLDETAILS_SIGNED  *pValue;

			pValue = (MIXERCONTROLDETAILS_SIGNED *) pDetail->paDetails;

			// Get right channel volume
			if( mmRet == MMSYSERR_NOERROR )
			{
				switch (pDetail->cChannels)
				{
				case 1:
					// If setting is mono, apply the same volume to both channels.
					break;
				default:
					mmRet = MMSYSERR_INVALPARAM;
					break;
				}
			}

			// Validate max setting.
			if (pValue[0].lValue > MICROPHONE_ANALOG_GAIN_MAX )
			{
				mmRet = MMSYSERR_INVALPARAM;
			}

			if(  mmRet == MMSYSERR_NOERROR )
			{
			 mmRet = (WORD)m_pInputStreamManager->get_HardwareAudioBridge()->SetModemMicAnalogGain((UINT16)pValue[0].lValue);
			}

			RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - SetModemMicAnalogGain = %d\r\n",__FUNCTION__, pValue[0].lValue));

	   } break;

	  case MIXERCONTROL_CONTROLTYPE_VOLUME:
	   {
	     MIXERCONTROLDETAILS_UNSIGNED  *pValue;

			pValue = (MIXERCONTROLDETAILS_UNSIGNED *) pDetail->paDetails;

			if( mmRet == MMSYSERR_NOERROR )
			{
				switch (pDetail->cChannels)
				{
				case 1:
					// If setting is mono, apply the same volume to both channels.
					break;
				default:
					mmRet = MMSYSERR_INVALPARAM;
					break;
				}
			}

			// Validate max setting.
			if (pValue[0].dwValue > MICROPHONE_DIGITAL_GAIN_MAX )
			{
				mmRet = MMSYSERR_INVALPARAM;
			}

			if(  mmRet == MMSYSERR_NOERROR )
			{
			 mmRet = (WORD)m_pInputStreamManager->get_HardwareAudioBridge()->SetModemMicDigitalGain((UINT16)pValue[0].dwValue);
			}

			RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - SetModemMicDigitalGain = %d\r\n",__FUNCTION__, pValue[0].dwValue));
	
	   } break; 
	}


	//RETAILMSG(WAVE_AUDIO_DBG, (L"put_AudioValue() - MIXERCONTROL_CONTROLTYPE_VOLUME = 0x%X, error = %d\r\n", gainMono, mmRet));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S, error = %d\r\n",__FUNCTION__, mmRet));

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));

    //return m_pInputStreamManager->put_Mute(pValue[0].fValue);
	return(mmRet); 
}



