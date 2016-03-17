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
// Portions Copyright (c) Micronet.  All rights reserved.
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
#include "vostrmmgr.h"
#include "strmctxt.h"
#include "hwaudiobrdg.h"
#include <omap35xx_hwbridge.h>
#include "tps659xx_wave.h"
//#include "omap35xx_hwbridge.h"

static CBTVoiceInputSource   s_BTVoiceInputLine;
static CBTVoiceOutputSource  s_BTVoiceOutputLine;

static CVoiceInputSource     s_VoiceInputLine;
static CVoiceOutputSource    s_VoiceOutputLine;

//------------------------------------------------------------------------------
// initialize_AudioLine, wave-in destination audio line
//
BOOL
CVoiceStreamManager::initialize_AudioLine(
    CAudioMixerManager *pAudioMixerManager
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CVoiceStreamManager::initialize_AudioLine()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:+CVoiceStreamManager::initialize_AudioLine()\r\n"));

    // register audio controls
    //
    //register_AudioControl(&m_VoiceVolumeControl);
    //register_AudioControl(&m_VoiceMuteControl);
	register_AudioControl(&m_VoiceOnOffControl);

    // register controls as default wavout volume and mute controls
    //
    //pAudioMixerManager->put_AudioMixerControl(
    //    CAudioMixerManager::kVoiceOutVolumeControl, &m_VoiceVolumeControl);

    //pAudioMixerManager->put_AudioMixerControl(
    //    CAudioMixerManager::kVoiceOutMuteControl, &m_VoiceMuteControl);

    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kVoiceOutOnOffControl, &m_VoiceOnOffControl);

    // initialize paired Voice source lines
    //
    s_VoiceInputLine.m_pVoiceStreamManager = this;
    pAudioMixerManager->register_SourceAudioLine(&s_VoiceInputLine, this);

    s_VoiceOutputLine.m_pVoiceStreamManager = this;
    pAudioMixerManager->register_SourceAudioLine(&s_VoiceOutputLine, this);

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CVoiceStreamManager::initialize_AudioLine()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:-CVoiceStreamManager::initialize_AudioLine()\r\n"));

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  get_DevCaps, return wave input device caps
//
DWORD
CVoiceStreamManager::get_DevCaps(
    LPVOID pCaps,
    DWORD dwSize
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CVoiceStreamManager::GetDevCaps()\r\n"));

    static const WAVEOUTCAPS wc =
        {
        MM_MICROSOFT,
        25,
        0x0001,
        TEXT("Voice Output(DL)"),
		WAVE_INVALIDFORMAT, // Not exist valid definition for voice. It is 8 kHz mode 
        1,
        0,
        WAVECAPS_VOLUME /*| WAVECAPS_PLAYBACKRATE */
        };

    if (dwSize > sizeof(WAVEOUTCAPS))
        {
        dwSize = sizeof(WAVEOUTCAPS);
        }

    memcpy( pCaps, &wc, dwSize);

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CVoiceStreamManager::GetDevCaps()\r\n"));

    return MMSYSERR_NOERROR;
}



//------------------------------------------------------------------------------
// get_ExtDevCaps not supported
//

DWORD
CVoiceStreamManager::get_ExtDevCaps(
    LPVOID pCaps,
    DWORD dwSize
    )
{

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CVoiceStreamManager::GetExtDevCaps()\r\n"));

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
        (L"WAV:-CVoiceStreamManager::GetExtDevCaps()\r\n"));

    return MMSYSERR_NOERROR;
}

//------------------------------------------------------------------------------
//  create_Stream, create input stream context.
//
StreamContext *
CVoiceStreamManager::create_Stream(
    LPWAVEOPENDESC lpWOD
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CVoiceStreamManager::CreateStream()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:+CVoiceStreamManager::CreateStream(), lpWOD = 0x%X\r\n", lpWOD));

#if 0
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
#endif

	// Meanwhile do not create stream context for Voice output

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CVoiceStreamManager::CreateStream()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG,
		(L"WAV:-CVoiceStreamManager::CreateStream()\r\n"));

	return(NULL);
}

//------------------------------------------------------------------------------
//  copy_AudioData, callback routines from hardware bridge
//

DWORD
CVoiceStreamManager::copy_AudioData(
    void* pStart,
    DWORD nSize
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CVoiceStreamManager::copy_AudioData()\r\n"));

	// Meanwhile do nothing oice output
#if 0
    // zero-out buffer before copying data
    memset(pStart, 0, nSize);

    DWORD dwActiveStreams = CStreamManager::copy_AudioData(pStart, nSize);
#else
    DWORD dwActiveStreams = 0;
#endif

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CVoiceStreamManager::copy_AudioData()\r\n"));

    return dwActiveStreams;
}


//------------------------------------------------------------------------------
//  StreamReadyToRender, start playback of wave output stream
//
void
CVoiceStreamManager::StreamReadyToRender(
    StreamContext *pStreamContext
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CVoiceStreamManager::StreamReadyToRender()\r\n"));

#if 0
    get_HardwareAudioBridge()->start_AudioPort(CHardwareAudioBridge::kVoiceOutput);
    get_HardwareAudioBridge()->start_Stream(CHardwareAudioBridge::kVoiceOutput, pStreamContext);
#endif

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CVoiceStreamManager::StreamReadyToRender()\r\n"));
    return;
}


//------------------------------------------------------------------------------
// get_AudioValue, retrieve audio properties
//
DWORD
CVoiceStreamManager::get_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag
    ) const
{
    DWORD mmRet = MMSYSERR_NOERROR;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CVoiceStreamManager::get_AudioValue()\r\n"));

    // determine what type of control generated the event
    // and forward to the destination audio line
    switch (pControl->get_ControlType())
    {
    case MIXERCONTROL_CONTROLTYPE_ONOFF:
		{
         MIXERCONTROLDETAILS_BOOLEAN *pOnOff;
		 DWORD                        ModemVoiceState;
		 BOOL                         btOn; 

         pOnOff = (MIXERCONTROLDETAILS_BOOLEAN *) pDetail->paDetails;
		 mmRet =  get_HardwareAudioBridge()->GetModemVoiceState(&ModemVoiceState, &btOn);
		 if( mmRet == MMSYSERR_NOERROR )
		 {
           switch(ModemVoiceState)
		   {
		    case kModemVoiceDisabled: //OMAP35XX_HwAudioBridge::kModemVoice_Idle: 
				{
				 pOnOff[0].fValue = FALSE;
				} break;

			case kModemVoiceEnabled: //OMAP35XX_HwAudioBridge::kModemVoice_Active: 
				{
				 pOnOff[0].fValue = TRUE;
				} break;

			default: 
				{
					pOnOff[0].fValue = FALSE; 
                    mmRet = MMSYSERR_ERROR;
				} break;
		   }
		 }

		 RETAILMSG(WAVE_AUDIO_DBG, (L"get_AudioValue() - MIXERCONTROL_CONTROLTYPE_ONOFF = %d, error = %d\r\n", pOnOff[0].fValue, mmRet));
		}
		break;


    default:
        mmRet = MMSYSERR_NOTSUPPORTED;
        break;
    }


    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CVoiceStreamManager::get_AudioValue()\r\n"));

    return mmRet;
}


//------------------------------------------------------------------------------
// put_AudioValue, set audio properties
//
DWORD
CVoiceStreamManager::put_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag
    )
{
    DWORD mmRet = MMSYSERR_NOERROR;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CVoiceStreamManager::put_AudioValue()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:+CVoiceStreamManager::put_AudioValue()\r\n"));

    // determine what type of control generated the event
    // and forward to the destination audio line
    switch (pControl->get_ControlType())
    {
    case MIXERCONTROL_CONTROLTYPE_ONOFF:
		{
         MIXERCONTROLDETAILS_BOOLEAN *pOnOff;

		 pOnOff = (MIXERCONTROLDETAILS_BOOLEAN *) pDetail->paDetails;
		 
		 if( pOnOff[0].fValue == TRUE )
		   mmRet = get_HardwareAudioBridge()->StartModemVoice(FALSE);
		 else
           mmRet = get_HardwareAudioBridge()->StopModemVoice(FALSE);

		 RETAILMSG(WAVE_AUDIO_DBG, (L"put_AudioValue() - MIXERCONTROL_CONTROLTYPE_ONOFF = %d, error = %d\r\n", pOnOff[0].fValue, mmRet));
		}
		break;


    default:
        mmRet = MMSYSERR_NOTSUPPORTED;
        break;
    }


    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CVoiceStreamManager::put_AudioValue()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:-CVoiceStreamManager::put_AudioValue()\r\n"));

    return mmRet;
}


//------------------------------------------------------------------------------
//
//  Function: OutputCStreamManagerIsSupportedFormat
//
//

BOOL
CVoiceStreamManager::IsSupportedFormat(LPWAVEFORMATEX lpFormat)
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CVoiceStreamManager::IsSupportedFormat()\r\n"));

    return CStreamManager::IsSupportedFormat(lpFormat);
}

//------------------------------------------------------------------------------
//
//  Function: OutputCStreamManager::GetProperty
//
//
DWORD
CVoiceStreamManager::GetProperty(PWAVEPROPINFO pPropInfo)
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
CVoiceStreamManager::SetProperty(PWAVEPROPINFO pPropInfo)
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



//////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// initialize_AudioLine, Voice output source audio line
//
BOOL
CVoiceOutputSource::initialize_AudioLine(
    CAudioMixerManager *pAudioMixerManager
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CVoiceInputSource::initialize_AudioLine()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    // initialize audio controls
    //
    register_AudioControl(&m_VoiceOutputGain);
	register_AudioControl(&m_VoiceOutputMute);

    //
    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kVoiceOutVolumeControl, &m_VoiceOutputGain);

	//kVoiceInMuteControl
    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kVoiceOutMuteControl, &m_VoiceOutputMute);


    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CVoiceInputSource::initialize_AudioLine()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
    return TRUE;
}



//------------------------------------------------------------------------------
// get_AudioValue, retrieve audio properties
//
DWORD
CVoiceOutputSource::get_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag
    ) const
{
    DWORD mmRet = MMSYSERR_NOERROR;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CVoiceStreamManager::get_AudioValue()\r\n"));

    // determine what type of control generated the event
    // and forward to the destination audio line
    switch (pControl->get_ControlType())
    {

    case MIXERCONTROL_CONTROLTYPE_DECIBELS:
        {
			INT16 gainMono;
            
            MIXERCONTROLDETAILS_SIGNED *pValue;
            pValue = (MIXERCONTROLDETAILS_SIGNED *) pDetail->paDetails;

			mmRet = (WORD) m_pVoiceStreamManager->get_HardwareAudioBridge()->GetModemVoiceOutGain(&gainMono);
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
			RETAILMSG(WAVE_AUDIO_DBG, (L"get_AudioValue() - MIXERCONTROL_CONTROLTYPE_VOLUME = 0x%X, error = %d\r\n", pValue[0].lValue, mmRet));
        }
        break;

    case MIXERCONTROL_CONTROLTYPE_MUTE:
        {
            MIXERCONTROLDETAILS_BOOLEAN *pMute;
			BOOL                          Mute; 
            pMute = (MIXERCONTROLDETAILS_BOOLEAN *) pDetail->paDetails;

			mmRet = (WORD) m_pVoiceStreamManager->get_HardwareAudioBridge()->GetMuteModemVoiceOut(&Mute);
			if( mmRet == MMSYSERR_NOERROR )
			{
              pMute[0].fValue = Mute;
			}

			RETAILMSG(WAVE_AUDIO_DBG, (L"get_AudioValue() - MIXERCONTROL_CONTROLTYPE_MUTE = %d, error = %d\r\n", pMute[0].fValue, mmRet));
        }
        break;

    default:
        mmRet = MMSYSERR_NOTSUPPORTED;
        break;
    }


    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CVoiceStreamManager::get_AudioValue()\r\n"));

    return mmRet;
}


//------------------------------------------------------------------------------
// put_AudioValue, set audio properties
//
DWORD
CVoiceOutputSource::put_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag
    )
{
    DWORD mmRet = MMSYSERR_NOERROR;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CVoiceStreamManager::put_AudioValue()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:+CVoiceStreamManager::put_AudioValue()\r\n"));

    // determine what type of control generated the event
    // and forward to the destination audio line
    switch (pControl->get_ControlType())
    {
    case MIXERCONTROL_CONTROLTYPE_DECIBELS:
        {
            LONG dwGain;
			INT16 gainMono;
            //DWORD dwGainL, dwGainR;

	        
            MIXERCONTROLDETAILS_SIGNED *pValue;
            pValue = (MIXERCONTROLDETAILS_SIGNED *) pDetail->paDetails;

            // Get left channel volume
            dwGain = pValue[0].lValue;

            // Get right channel volume
            switch (pDetail->cChannels)
            {
            case 1:
                // If setting is mono, apply the same volume to both channels.
                break;
            default:
                mmRet = MMSYSERR_INVALPARAM;
                goto Error;
            }


            // Validate max setting.
            if (dwGain > LOGICAL_VOLUME_MAX)
            {
                mmRet = MMSYSERR_INVALPARAM;
                goto Error;
            }

            // Set gain
			gainMono = (INT16)dwGain;
			mmRet = (WORD) m_pVoiceStreamManager->get_HardwareAudioBridge()->SetModemVoiceOutGain(gainMono);

			RETAILMSG(WAVE_AUDIO_DBG, (L"put_AudioValue() - MIXERCONTROL_CONTROLTYPE_VOLUME = 0x%X, error = %d\r\n", gainMono, mmRet));
        }
        break;

    case MIXERCONTROL_CONTROLTYPE_MUTE:
        {
            MIXERCONTROLDETAILS_BOOLEAN *pMute;

            pMute = (MIXERCONTROLDETAILS_BOOLEAN *) pDetail->paDetails;
			mmRet = (WORD) m_pVoiceStreamManager->get_HardwareAudioBridge()->SetMuteModemVoiceOut(pMute[0].fValue);

			RETAILMSG(WAVE_AUDIO_DBG, (L"put_AudioValue() - MIXERCONTROL_CONTROLTYPE_MUTE = %d, error = %d\r\n", pMute[0].fValue, mmRet));
        }
        break;

    default:
        mmRet = MMSYSERR_NOTSUPPORTED;
        break;
    }

Error:

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CVoiceStreamManager::put_AudioValue()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:-CVoiceStreamManager::put_AudioValue()\r\n"));

    return mmRet;
}




//////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// initialize_AudioLine, Voice input source audio line
//
BOOL
CVoiceInputSource::initialize_AudioLine(
    CAudioMixerManager *pAudioMixerManager
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CVoiceInputSource::initialize_AudioLine()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    // initialize audio controls
    //
    register_AudioControl(&m_VoiceInputGain);
	register_AudioControl(&m_VoiceInputMute);

    //
    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kVoiceInVolumeControl, &m_VoiceInputGain);

	//kVoiceInMuteControl
    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kVoiceInMuteControl, &m_VoiceInputMute);


    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CVoiceInputSource::initialize_AudioLine()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
    return TRUE;
}


//------------------------------------------------------------------------------
// get_AudioValue, retrieve microphone audio properties
//
DWORD
CVoiceInputSource::get_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag) const
{
	INT16                          gainMono;
	BOOL                           micMute;
	DWORD                          mmRet = MMSYSERR_NOERROR;

    DEBUGMSG(ZONE_FUNCTION,(L"WAV:+CVoiceInputSource::get_AudioValue()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    switch(pControl->get_ControlType())
	{
	  case MIXERCONTROL_CONTROLTYPE_MUTE:
	   {
			// query micrphone mute
			//
			MIXERCONTROLDETAILS_BOOLEAN * pValue =
						(MIXERCONTROLDETAILS_BOOLEAN * ) pDetail->paDetails;

			mmRet = (WORD) m_pVoiceStreamManager->get_HardwareAudioBridge()->GetMuteModemVoiceIn(&micMute);
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
			mmRet = (WORD)m_pVoiceStreamManager->get_HardwareAudioBridge()->GetModemVoiceInputGain(&gainMono);
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

          RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - GetModemVoiceInputGain = %d\r\n",__FUNCTION__, gainMono));

	   } break;

    }


    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CVoiceInputSource::get_AudioValue()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));

    return MMSYSERR_NOERROR;
}


//------------------------------------------------------------------------------
// put_AudioValue, set microphone audio properties
//
DWORD
CVoiceInputSource::put_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag)
{
    LONG          dwGain;
    INT16         gainMono;
	DWORD          mmRet = MMSYSERR_NOERROR;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CVoiceInputSource::put_AudioValue()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    //ASSERT(pControl->get_ControlType() == MIXERCONTROL_CONTROLTYPE_BOOLEAN);

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
		 mmRet = (WORD)m_pVoiceStreamManager->get_HardwareAudioBridge()->SetMuteModemVoiceIn(pValue[0].fValue);
		}

		RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - SetMuteModemVoiceIn = %d\r\n",__FUNCTION__, pValue[0].fValue));

	   } break;
	

	  case  MIXERCONTROL_CONTROLTYPE_DECIBELS:
	   {
	     MIXERCONTROLDETAILS_SIGNED  *pValue;

            pValue = (MIXERCONTROLDETAILS_SIGNED *) pDetail->paDetails;

            // Get left channel volume
            dwGain = pValue[0].lValue;

            // Get right channel volume
            switch (pDetail->cChannels)
            {
            case 1:
                // If setting is mono, apply the same volume to both channels.
                break;
            default:
                mmRet = MMSYSERR_INVALPARAM;
                goto Error2;
            }


            // Validate max setting.
            if (dwGain >  VOICE_INPUT_VOL_MAX )
            {
                mmRet = MMSYSERR_INVALPARAM;
                goto Error2;
            }

            // Set gain
			gainMono = (INT16)dwGain;
			mmRet = m_pVoiceStreamManager->get_HardwareAudioBridge()->SetModemVoiceInputGain(gainMono);

			//RETAILMSG(WAVE_AUDIO_DBG, (L"put_AudioValue() - MIXERCONTROL_CONTROLTYPE_VOLUME = 0x%X, error = %d\r\n", gainMono, mmRet));
			RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - MIXERCONTROL_CONTROLTYPE_VOLUME = 0x%X, error = %d\r\n",__FUNCTION__, gainMono, mmRet));

	   } break;

	  }

 Error2:

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CVoiceInputSource::put_AudioValue()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
    return mmRet;
}



///////////////////////// BT Voice //////////////////////////////

//------------------------------------------------------------------------------
// initialize_AudioLine, wave-in destination audio line
//
BOOL
CBTVoiceStreamManager::initialize_AudioLine(
    CAudioMixerManager *pAudioMixerManager
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CBTVoiceStreamManager::initialize_AudioLine()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:+CBTVoiceStreamManager::initialize_AudioLine()\r\n"));

    // register audio controls
    //
	register_AudioControl(&m_BTVoiceOnOffControl);

    // register controls as default BT Voice controls
    //
    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kBTVoiceOnOffControl, &m_BTVoiceOnOffControl);

    // initialize paired BT Voice Input source lines
    s_BTVoiceInputLine.m_pCBTVoiceStreamManager = this;
    pAudioMixerManager->register_SourceAudioLine(&s_BTVoiceInputLine, this);

	// initialize paired BT Voice Output source lines
    s_BTVoiceOutputLine.m_pCBTVoiceStreamManager = this;
    pAudioMixerManager->register_SourceAudioLine(&s_BTVoiceOutputLine, this);

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CBTVoiceStreamManager::initialize_AudioLine()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:-CBTVoiceStreamManager::initialize_AudioLine()\r\n"));

    return TRUE;
}


#if 1
//------------------------------------------------------------------------------
//
//  get_DevCaps, return wave input device caps
//
DWORD
CBTVoiceStreamManager::get_DevCaps(
    LPVOID pCaps,
    DWORD dwSize
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CBTVoiceStreamManager::GetDevCaps()\r\n"));


    if (dwSize > sizeof(WAVEOUTCAPS))
        {
        dwSize = sizeof(WAVEOUTCAPS);
        }

    //memcpy( pCaps, &wc, dwSize);
	memset(pCaps, 0, dwSize);

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CBTVoiceStreamManager::GetDevCaps()\r\n"));

    return MMSYSERR_NOERROR;
}
#endif


#if 1
//------------------------------------------------------------------------------
// get_ExtDevCaps not supported
//

DWORD
CBTVoiceStreamManager::get_ExtDevCaps(
    LPVOID pCaps,
    DWORD dwSize
    )
{

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CBTVoiceStreamManager::GetExtDevCaps()\r\n"));

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
        (L"WAV:-CBTVoiceStreamManager::GetExtDevCaps()\r\n"));

    return MMSYSERR_NOERROR;
}
#endif

//------------------------------------------------------------------------------
//  create_Stream, create input stream context.
//
StreamContext *
CBTVoiceStreamManager::create_Stream(
    LPWAVEOPENDESC lpWOD
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CBTVoiceStreamManager::CreateStream()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:+CBTVoiceStreamManager::CreateStream(), lpWOD = 0x%X\r\n", lpWOD));

	// Meanwhile do not create stream context for Voice output

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CBTVoiceStreamManager::CreateStream()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG,
		(L"WAV:-CBTVoiceStreamManager::CreateStream()\r\n"));

	return(NULL);
}

//------------------------------------------------------------------------------
//  copy_AudioData, callback routines from hardware bridge
//

DWORD
CBTVoiceStreamManager::copy_AudioData(
    void* pStart,
    DWORD nSize
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CBTVoiceStreamManager::copy_AudioData()\r\n"));

	// Meanwhile do nothing oice output
    DWORD dwActiveStreams = 0;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CBTVoiceStreamManager::copy_AudioData()\r\n"));

    return dwActiveStreams;
}


//------------------------------------------------------------------------------
//  StreamReadyToRender, start playback of wave output stream
//
void
CBTVoiceStreamManager::StreamReadyToRender(
    StreamContext *pStreamContext
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CBTVoiceStreamManager::StreamReadyToRender()\r\n"));

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CBTVoiceStreamManager::StreamReadyToRender()\r\n"));
    return;
}


//------------------------------------------------------------------------------
// get_AudioValue, retrieve audio properties
//
DWORD
CBTVoiceStreamManager::get_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag
    ) const
{
    DWORD mmRet = MMSYSERR_NOERROR;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CBTVoiceStreamManager::get_AudioValue()\r\n"));

    // determine what type of control generated the event
    // and forward to the destination audio line
    switch (pControl->get_ControlType())
    {
    case MIXERCONTROL_CONTROLTYPE_ONOFF:
		{
         MIXERCONTROLDETAILS_BOOLEAN *pOnOff;
		 DWORD                        ModemVoiceState;
		 BOOL                         btOn;

         pOnOff = (MIXERCONTROLDETAILS_BOOLEAN *) pDetail->paDetails;
		 mmRet =  get_HardwareAudioBridge()->GetModemVoiceState(&ModemVoiceState, &btOn);
		 if( mmRet == MMSYSERR_NOERROR )
		 {
           switch(btOn)
		   {
		    case kModemBTVoiceDisabled:
				{
				 pOnOff[0].fValue = FALSE;
				} break;

			case kModemBTVoiceEnabled:
				{
				 pOnOff[0].fValue = TRUE;
				} break;

			default: 
				{
					pOnOff[0].fValue = FALSE; 
                    mmRet = MMSYSERR_ERROR;
				} break;
		   }
		 }

		 RETAILMSG(WAVE_AUDIO_DBG, (L"get_AudioValue() - MIXERCONTROL_CONTROLTYPE_ONOFF = %d, error = %d\r\n", pOnOff[0].fValue, mmRet));
		}
		break;

    default:
        mmRet = MMSYSERR_NOTSUPPORTED;
        break;
    }


    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CBTVoiceStreamManager::get_AudioValue()\r\n"));

    return mmRet;
}


//------------------------------------------------------------------------------
// put_AudioValue, set audio properties
//
DWORD
CBTVoiceStreamManager::put_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag
    )
{
    DWORD mmRet = MMSYSERR_NOERROR;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CBTVoiceStreamManager::put_AudioValue()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:+CBTVoiceStreamManager::put_AudioValue()\r\n"));

    // determine what type of control generated the event
    // and forward to the destination audio line
    switch (pControl->get_ControlType())
    {
     case MIXERCONTROL_CONTROLTYPE_ONOFF:
		{
         MIXERCONTROLDETAILS_BOOLEAN *pOnOff;

		 pOnOff = (MIXERCONTROLDETAILS_BOOLEAN *) pDetail->paDetails;
		 
		 if( pOnOff[0].fValue == TRUE )
		   mmRet = get_HardwareAudioBridge()->StartModemVoice(TRUE);
		 else
           mmRet = get_HardwareAudioBridge()->StopModemVoice(TRUE);

		 RETAILMSG(WAVE_AUDIO_DBG, (L"put_AudioValue() - MIXERCONTROL_CONTROLTYPE_ONOFF = %d, error = %d\r\n", pOnOff[0].fValue, mmRet));
		}
		break;

    default:
        mmRet = MMSYSERR_NOTSUPPORTED;
        break;
    }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CBTVoiceStreamManager::put_AudioValue()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:-CBTVoiceStreamManager::put_AudioValue()\r\n"));

    return mmRet;
}


//------------------------------------------------------------------------------
//
//  Function: OutputCStreamManagerIsSupportedFormat
//
//

BOOL
CBTVoiceStreamManager::IsSupportedFormat(LPWAVEFORMATEX lpFormat)
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CBTVoiceStreamManager::IsSupportedFormat()\r\n"));

    return CStreamManager::IsSupportedFormat(lpFormat);
}

//------------------------------------------------------------------------------
//
//  Function: OutputCStreamManager::GetProperty
//
//
DWORD
CBTVoiceStreamManager::GetProperty(PWAVEPROPINFO pPropInfo)
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
CBTVoiceStreamManager::SetProperty(PWAVEPROPINFO pPropInfo)
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




///////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
// initialize_AudioLine, BT Voice input source audio line
//
BOOL
CBTVoiceInputSource::initialize_AudioLine(
    CAudioMixerManager *pAudioMixerManager
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+%S\r\n",__FUNCTION__));

    RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    // initialize Voice input controls
    //
	register_AudioControl(&m_BTVoiceInMuteControl);
	register_AudioControl(&m_BTVoiceInVolumeControl);				

    // register as the default microphone mute control
    //
    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kBTVoiceInMuteControl, &m_BTVoiceInMuteControl);

    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kBTVoiceInVolumeControl, &m_BTVoiceInVolumeControl);


    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-%S\r\n",__FUNCTION__));

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
    return TRUE;
}


//------------------------------------------------------------------------------
// get_AudioValue, retrieve Voice Input audio properties
//
DWORD
CBTVoiceInputSource::get_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag) const
{
	BOOL      micMute;
	WORD      mmRet = MMSYSERR_NOERROR;
	INT16    gainMono;


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

			mmRet = (WORD) m_pCBTVoiceStreamManager->get_HardwareAudioBridge()->GetMuteModemBTInVoice(&micMute);
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

			RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - GetMuteModemBTInVoice = %d\r\n",__FUNCTION__, micMute));

	   } break;

	  case  MIXERCONTROL_CONTROLTYPE_DECIBELS:
	   {
	     MIXERCONTROLDETAILS_SIGNED  *pValue;

			pValue = (MIXERCONTROLDETAILS_SIGNED *) pDetail->paDetails;
			mmRet = (WORD)m_pCBTVoiceStreamManager->get_HardwareAudioBridge()->GetModemBTVoiceInputGain(&gainMono);
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

          RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - GetModemBTVoiceInputGain = %d\r\n",__FUNCTION__, gainMono));

	   } break;

      default:
		{
         mmRet = MMSYSERR_NOTSUPPORTED;
		} break;
    
	}

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));

    return mmRet;
}


//------------------------------------------------------------------------------
// put_AudioValue, set Voice Input audio properties
//
DWORD
CBTVoiceInputSource::put_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag)
{
	//BOOL    micMute;
	WORD    mmRet = MMSYSERR_NOERROR;

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
		 mmRet = (WORD)m_pCBTVoiceStreamManager->get_HardwareAudioBridge()->SetMuteModemBTInVoice(pValue[0].fValue);
		}

		RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - SetMuteModemBTInVoice = %d\r\n",__FUNCTION__, pValue[0].fValue));

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
			if( (pValue[0].lValue > BTDIN_GAIN_MAX )||(pValue[0].lValue < BTDIN_GAIN_MIN ) )
			{
				mmRet = MMSYSERR_INVALPARAM;
			}

			if(  mmRet == MMSYSERR_NOERROR )
			{
			 mmRet = (WORD)m_pCBTVoiceStreamManager->get_HardwareAudioBridge()->SetModemBTVoiceInputGain((INT16)pValue[0].lValue);
			}

			RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - SetModemBTVoiceInputGain = %d\r\n",__FUNCTION__, pValue[0].lValue));

	   } break;

      default:
		{
         mmRet = MMSYSERR_NOTSUPPORTED;
		} break;

	}


	//RETAILMSG(WAVE_AUDIO_DBG, (L"put_AudioValue() - MIXERCONTROL_CONTROLTYPE_VOLUME = 0x%X, error = %d\r\n", gainMono, mmRet));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S, error = %d\r\n",__FUNCTION__, mmRet));
	return(mmRet); 
}







///////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
// initialize_AudioLine, Voice output source audio line
//
BOOL
CBTVoiceOutputSource::initialize_AudioLine(
    CAudioMixerManager *pAudioMixerManager
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+%S\r\n",__FUNCTION__));

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    // initialize Voice input controls
    //
	register_AudioControl(&m_BTVoiceOutMuteControl);
	register_AudioControl(&m_BTVoiceOutVolumeControl);

    // register as the default microphone mute control
    //
    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kBTVoiceOutMuteControl, &m_BTVoiceOutMuteControl);

    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kBTVoiceOutVolumeControl, &m_BTVoiceOutVolumeControl);


    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-%S\r\n",__FUNCTION__));

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));

    return TRUE;
}


//------------------------------------------------------------------------------
// get_AudioValue, retrieve Voice Output audio properties
//
DWORD
CBTVoiceOutputSource::get_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag) const
{
	BOOL      micMute;
	WORD      mmRet = MMSYSERR_NOERROR;
	INT16    gainMono;


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

			mmRet = (WORD) m_pCBTVoiceStreamManager->get_HardwareAudioBridge()->GetMuteModemBTOutVoice(&micMute);
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

			RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - GetMuteModemBTOutVoice = %d\r\n",__FUNCTION__, micMute));

	   } break;

	  case  MIXERCONTROL_CONTROLTYPE_DECIBELS:
	   {
	     MIXERCONTROLDETAILS_SIGNED  *pValue;

			pValue = (MIXERCONTROLDETAILS_SIGNED *) pDetail->paDetails;
			mmRet = (WORD)m_pCBTVoiceStreamManager->get_HardwareAudioBridge()->GetModemBTVoiceOutputGain(&gainMono);
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

          RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - GetModemBTVoiceOutputGain = %d\r\n",__FUNCTION__, gainMono));

	   } break;

      default:
		{
         mmRet = MMSYSERR_NOTSUPPORTED;
		} break;
    
	}

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));

    return mmRet;
}


//------------------------------------------------------------------------------
// put_AudioValue, set Voice Input audio properties
//
DWORD
CBTVoiceOutputSource::put_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag)
{
	//BOOL    micMute;
	WORD    mmRet = MMSYSERR_NOERROR;

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
		 mmRet = (WORD)m_pCBTVoiceStreamManager->get_HardwareAudioBridge()->SetMuteModemBTOutVoice(pValue[0].fValue);
		}

		RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - SetMuteModemBTOutVoice = %d\r\n",__FUNCTION__, pValue[0].fValue));

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
			if( (pValue[0].lValue > BTDOUT_GAIN_MAX )||(pValue[0].lValue < BTDOUT_GAIN_MIN ) )
			{
				mmRet = MMSYSERR_INVALPARAM;
			}

			if(  mmRet == MMSYSERR_NOERROR )
			{
			 mmRet = (WORD)m_pCBTVoiceStreamManager->get_HardwareAudioBridge()->SetModemBTVoiceOutputGain((INT16)pValue[0].lValue);
			}

			RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - SetModemBTVoiceOutputGain = %d\r\n",__FUNCTION__, pValue[0].lValue));

	   } break;

      default:
		{
         mmRet = MMSYSERR_NOTSUPPORTED;
		} break;

	}


	//RETAILMSG(WAVE_AUDIO_DBG, (L"put_AudioValue() - MIXERCONTROL_CONTROLTYPE_VOLUME = 0x%X, error = %d\r\n", gainMono, mmRet));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S, error = %d\r\n",__FUNCTION__, mmRet));
	return(mmRet); 
}
