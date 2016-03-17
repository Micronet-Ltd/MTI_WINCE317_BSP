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
#include "eastrmmgr.h"
#include "strmctxt.h"
#include "hwaudiobrdg.h"
#include <omap35xx_hwbridge.h>
#include "tps659xx_wave.h"

#define   EXT_AUDIO_DBG    0


//static CBTPlaybackOutputSource  s_BTPlaybackOutputLine;
static CExtMicrophoneInputSource  s_ExtMicrophoneInputLine;

//------------------------------------------------------------------------------
// initialize_AudioLine
//

BOOL
CExtAudioStreamManager::initialize_AudioLine(
    CAudioMixerManager *pAudioMixerManager
    )
{
    //DEBUGMSG(ZONE_FUNCTION,(L"WAV:+CExtAudioStreamManager::initialize_AudioLine()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    // register audio controls
	register_AudioControl(&m_ExtAudioOnOffControl);

//		kBTPlbOnOffControl,
//	    kBTPlbOutMuteControl,
//	    kBTPlbOutVolumeControl,

    // register controls as default Ext Audio controls
    //
    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kExtAudioOnOffControl, &m_ExtAudioOnOffControl);

    s_ExtMicrophoneInputLine.m_ExtAudioStreamManager = this;
    pAudioMixerManager->register_SourceAudioLine(&s_ExtMicrophoneInputLine, this);

#if 0
	// initialize paired BT Playback Output source lines
    s_BTPlaybackOutputLine.m_pCBTPlaybackStreamManager = this;
    pAudioMixerManager->register_SourceAudioLine(&s_BTPlaybackOutputLine, this);
#endif

    DEBUGMSG(ZONE_FUNCTION,(L"WAV:-CExtAudioStreamManager::initialize_AudioLine()\r\n"));
    RETAILMSG(EXT_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
    return TRUE;
}



//------------------------------------------------------------------------------
//
//  get_DevCaps, return wave input device caps
//
DWORD
CExtAudioStreamManager::get_DevCaps(
    LPVOID pCaps,
    DWORD dwSize
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CExtAudioStreamManager::GetDevCaps()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));


    if (dwSize > sizeof(WAVEOUTCAPS))
        {
        dwSize = sizeof(WAVEOUTCAPS);
        }

    //memcpy( pCaps, &wc, dwSize);
	memset(pCaps, 0, dwSize);

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CExtAudioStreamManager::GetDevCaps()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));

    return MMSYSERR_NOERROR;
}


#if 1
//------------------------------------------------------------------------------
// get_ExtDevCaps not supported
//

DWORD
CExtAudioStreamManager::get_ExtDevCaps(
    LPVOID pCaps,
    DWORD dwSize
    )
{

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CExtAudioStreamManager::GetExtDevCaps()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

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


    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CExtAudioStreamManager::GetExtDevCaps()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));

    return MMSYSERR_NOERROR;
}
#endif

//------------------------------------------------------------------------------
//  create_Stream, create input stream context.
//
StreamContext *
CExtAudioStreamManager::create_Stream(
    LPWAVEOPENDESC lpWOD
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CExtAudioStreamManager::CreateStream()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:+CExtAudioStreamManager::CreateStream(), lpWOD = 0x%X\r\n", lpWOD));

	// Meanwhile do not create stream context for Voice output

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CExtAudioStreamManager::CreateStream()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:-CExtAudioStreamManager::CreateStream()\r\n"));

	return(NULL);
}

//------------------------------------------------------------------------------
//  copy_AudioData, callback routines from hardware bridge
//

DWORD
CExtAudioStreamManager::copy_AudioData(
    void* pStart,
    DWORD nSize
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CExtAudioStreamManager::copy_AudioData()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

	// Meanwhile do nothing oice output
    DWORD dwActiveStreams = 0;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CExtAudioStreamManager::copy_AudioData()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));

    return dwActiveStreams;
}


//------------------------------------------------------------------------------
//  StreamReadyToRender, start playback of wave output stream
//
void
CExtAudioStreamManager::StreamReadyToRender(
    StreamContext *pStreamContext
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CExtAudioStreamManager::StreamReadyToRender()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CExtAudioStreamManager::StreamReadyToRender()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
    return;
}

//------------------------------------------------------------------------------
// get_AudioValue, retrieve audio properties
//
DWORD
CExtAudioStreamManager::get_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag
    ) const
{
    DWORD                         mmRet = MMSYSERR_NOERROR;
	MIXERCONTROLDETAILS_BOOLEAN  *pOnOff;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CExtAudioStreamManager::get_AudioValue()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    // determine what type of control generated the event
    // and forward to the destination audio line
    switch (pControl->get_ControlType())
    {
    case MIXERCONTROL_CONTROLTYPE_ONOFF:
		{
         pOnOff = (MIXERCONTROLDETAILS_BOOLEAN *) pDetail->paDetails;
		 //mmRet =  get_HardwareAudioBridge()->GetBTPlaybackState(&BTPlaybackState);
		 pOnOff[0].fValue = get_HardwareAudioBridge()->query_ExtAudioEnable();
 
		}
		break;

    default:
        mmRet = MMSYSERR_NOTSUPPORTED;
        break;
    }
   
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:%S - MIXERCONTROL_CONTROLTYPE_ONOFF = %d, error = %d\r\n",__FUNCTION__, pOnOff[0].fValue, mmRet));

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CExtAudioStreamManager::get_AudioValue()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));

    return mmRet;
}


//------------------------------------------------------------------------------
// put_AudioValue, set audio properties
//
DWORD
CExtAudioStreamManager::put_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag
    )
{
    DWORD mmRet = MMSYSERR_NOERROR;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CExtAudioStreamManager::put_AudioValue()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    // determine what type of control generated the event
    // and forward to the destination audio line
    switch (pControl->get_ControlType())
    {
     case MIXERCONTROL_CONTROLTYPE_ONOFF:
		{
         MIXERCONTROLDETAILS_BOOLEAN *pOnOff;

		 pOnOff = (MIXERCONTROLDETAILS_BOOLEAN *) pDetail->paDetails;
		 
		 // enable_BTHeadset
		 //if( pOnOff[0].fValue == TRUE )
		 //  mmRet = get_HardwareAudioBridge()->enable_BTHeadset(pOnOff[0].fValue);
		 //else
         //  mmRet = get_HardwareAudioBridge()->StopBTPlayback();

		 if( !get_HardwareAudioBridge()->enable_ExtAudio(pOnOff[0].fValue) )
		   mmRet = MMSYSERR_ERROR;

		 RETAILMSG(EXT_AUDIO_DBG, (L"WAV:%S - MIXERCONTROL_CONTROLTYPE_ONOFF = %d, error = %d\r\n",__FUNCTION__, pOnOff[0].fValue, mmRet));
		}
		break;

    default:
        mmRet = MMSYSERR_NOTSUPPORTED;
        break;
    }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CExtAudioStreamManager::put_AudioValue()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));

    return mmRet;
}


//------------------------------------------------------------------------------
//
//  Function: OutputCStreamManagerIsSupportedFormat
//
//

BOOL
CExtAudioStreamManager::IsSupportedFormat(LPWAVEFORMATEX lpFormat)
{
  BOOL  bRes;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CExtAudioStreamManager::IsSupportedFormat()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    bRes = CStreamManager::IsSupportedFormat(lpFormat);
    //return CStreamManager::IsSupportedFormat(lpFormat);

	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:-%S, IsSupportedFormat = %d\r\n",__FUNCTION__, bRes));
	return(bRes);
}

//------------------------------------------------------------------------------
//
//  Function: OutputCStreamManager::GetProperty
//
//
DWORD
CExtAudioStreamManager::GetProperty(PWAVEPROPINFO pPropInfo)
{
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    // Call base class.
    DWORD mmRet = CStreamManager::GetProperty(pPropInfo);

    // Forward to hardware context if not supported.
    if (MMSYSERR_NOTSUPPORTED == mmRet)
    {
        mmRet = GetOutputProperty(pPropInfo);
    }

	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:-%S, GetProperty = %d\r\n",__FUNCTION__, mmRet));
    return mmRet;
}

//------------------------------------------------------------------------------
//
//  Function: OutputCStreamManager::SetProperty
//
//
DWORD
CExtAudioStreamManager::SetProperty(PWAVEPROPINFO pPropInfo)
{
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    // Call base class.
    DWORD mmRet = CStreamManager::SetProperty(pPropInfo);

    // Forward to hardware context if not supported.
    if (MMSYSERR_NOTSUPPORTED == mmRet)
    {
        mmRet = SetOutputProperty(pPropInfo);
    }

	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:-%S, SetProperty = %d\r\n",__FUNCTION__, mmRet));
    return mmRet;
}



//------------------------------------------------------------------------------
// initialize_AudioLine, external microphone source audio line
//
BOOL
CExtMicrophoneInputSource::initialize_AudioLine(
    CAudioMixerManager *pAudioMixerManager
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CExtMicrophoneInputSource::initialize_AudioLine()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    // initialize audio controls
    //
    register_AudioControl(&m_ExtMicMuteControl);
	register_AudioControl(&m_ExtMicAnalogGainControl);
	register_AudioControl(&m_ExtMicDigGainControl);

    // register as the default microphone mute control
    //
    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kExtMicMuteControl, &m_ExtMicMuteControl);

    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kExtMicAnalogVolumeControl, &m_ExtMicAnalogGainControl);

    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kExtMicDigitVolumeControl, &m_ExtMicDigGainControl);


    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CExtMicrophoneInputSource::initialize_AudioLine()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));

    return TRUE;
}


//------------------------------------------------------------------------------
// get_AudioValue, retrieve external microphone audio properties
//
DWORD
CExtMicrophoneInputSource::get_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag) const
{
	BOOL      micMute;
	WORD      mmRet = MMSYSERR_NOERROR;
	

    DEBUGMSG(ZONE_FUNCTION,(L"WAV:+CExtMicrophoneInputSource::get_AudioValue()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    //ASSERT(pControl->get_ControlType() == MIXERCONTROL_CONTROLTYPE_BOOLEAN);

	switch(pControl->get_ControlType())
	{
	  case MIXERCONTROL_CONTROLTYPE_MUTE:
	   {
			// query micrphone mute
			//
			MIXERCONTROLDETAILS_BOOLEAN * pValue =
						(MIXERCONTROLDETAILS_BOOLEAN * ) pDetail->paDetails;

			mmRet = (WORD) m_ExtAudioStreamManager->get_HardwareAudioBridge()->GetExtMicMuteStatus(&micMute);
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

			RETAILMSG(EXT_AUDIO_DBG, (L"WAV:%S - GetModemMicMuteStatus = %d\r\n",__FUNCTION__, micMute));

	   } break;

	  case  MIXERCONTROL_CONTROLTYPE_DECIBELS:
	   {
	     MIXERCONTROLDETAILS_SIGNED  *pValue;
		 INT16                       gainMono;

			pValue = (MIXERCONTROLDETAILS_SIGNED *) pDetail->paDetails;
			mmRet = (WORD)m_ExtAudioStreamManager->get_HardwareAudioBridge()->GetExtMicDigitalGain(&gainMono);
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

          RETAILMSG(EXT_AUDIO_DBG, (L"WAV:%S - GetModemMicAnalogGain = %d\r\n",__FUNCTION__, gainMono));

	   } break;

	  case MIXERCONTROL_CONTROLTYPE_VOLUME:
	   {
	     MIXERCONTROLDETAILS_UNSIGNED  *pValue;
		 UINT16                         gainMono;

			pValue = (MIXERCONTROLDETAILS_UNSIGNED *) pDetail->paDetails;
			mmRet = (WORD)m_ExtAudioStreamManager->get_HardwareAudioBridge()->GetExtMicAnalogGain(&gainMono);
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

			RETAILMSG(EXT_AUDIO_DBG, (L"WAV:%S - GetModemMicDigitalGain = %d\r\n",__FUNCTION__, gainMono));

	   } break; 
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CExtMicrophoneInputSource::get_AudioValue()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));

    return mmRet;
}


//------------------------------------------------------------------------------
// put_AudioValue, set microphone audio properties
//
DWORD
CExtMicrophoneInputSource::put_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag)
{
	//BOOL    micMute;
	WORD    mmRet = MMSYSERR_NOERROR;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CExtMicrophoneInputSource::put_AudioValue()\r\n"));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

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
		 mmRet = (WORD)m_ExtAudioStreamManager->get_HardwareAudioBridge()->SetExtMicMute(pValue[0].fValue);
		}

		RETAILMSG(EXT_AUDIO_DBG, (L"WAV:%S - SetModemMicMute = %d\r\n",__FUNCTION__, pValue[0].fValue));

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
			if( (pValue[0].lValue > EXT_MICROPHONE_DIGITAL_LEVEL_MAX) || (pValue[0].lValue < EXT_MICROPHONE_DIGITAL_LEVEL_MIN) )
			{
				mmRet = MMSYSERR_INVALPARAM;
			}

			if(  mmRet == MMSYSERR_NOERROR )
			{
			 mmRet = (WORD)m_ExtAudioStreamManager->get_HardwareAudioBridge()->SetExtMicDigitalGain((INT16)pValue[0].lValue);
			}

			RETAILMSG(EXT_AUDIO_DBG, (L"WAV:%S - SetModemMicAnalogGain = %d\r\n",__FUNCTION__, pValue[0].lValue));

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
			if( (pValue[0].dwValue > EXT_MICROPHONE_ANALOG_GAIN_MAX ) || (pValue[0].dwValue < EXT_MICROPHONE_ANALOG_GAIN_MIN) )
			{
				mmRet = MMSYSERR_INVALPARAM;
			}

			if(  mmRet == MMSYSERR_NOERROR )
			{
			 mmRet = (WORD)m_ExtAudioStreamManager->get_HardwareAudioBridge()->SetExtMicAnalogGain((UINT16)pValue[0].dwValue);
			}

			RETAILMSG(EXT_AUDIO_DBG, (L"WAV:%S - SetModemMicDigitalGain = %d\r\n",__FUNCTION__, pValue[0].dwValue));
	
	   } break; 
	}


	//RETAILMSG(EXT_AUDIO_DBG, (L"put_AudioValue() - MIXERCONTROL_CONTROLTYPE_VOLUME = 0x%X, error = %d\r\n", gainMono, mmRet));
	RETAILMSG(EXT_AUDIO_DBG, (L"WAV:-%S, error = %d\r\n",__FUNCTION__, mmRet));

    //return m_pInputStreamManager->put_Mute(pValue[0].fValue);
	return(mmRet); 
}

