
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
#include "btstrmmgr.h"
#include "strmctxt.h"
#include "hwaudiobrdg.h"
#include <omap35xx_hwbridge.h>
#include "tps659xx_wave.h"

static CBTPlaybackOutputSource  s_BTPlaybackOutputLine;


///////////////////////// BT Playback //////////////////////////////

//------------------------------------------------------------------------------
// initialize_AudioLine
//

BOOL
CBTPlaybackStreamManager::initialize_AudioLine(
    CAudioMixerManager *pAudioMixerManager
    )
{
    DEBUGMSG(ZONE_FUNCTION,(L"WAV:+CBTPlaybackStreamManager::initialize_AudioLine()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    // register audio controls
    // m_BTPlbOnOffControl
	register_AudioControl(&m_BTPlbOnOffControl);

//		kBTPlbOnOffControl,
//	    kBTPlbOutMuteControl,
//	    kBTPlbOutVolumeControl,

    // register controls as default BT Voice controls
    //
    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kBTPlbOnOffControl, &m_BTPlbOnOffControl);

	// initialize paired BT Playback Output source lines
    s_BTPlaybackOutputLine.m_pCBTPlaybackStreamManager = this;
    pAudioMixerManager->register_SourceAudioLine(&s_BTPlaybackOutputLine, this);

    DEBUGMSG(ZONE_FUNCTION,(L"WAV:-CBTPlaybackStreamManager::initialize_AudioLine()\r\n"));
    RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
    return TRUE;
}


#if 1
//------------------------------------------------------------------------------
//
//  get_DevCaps, return wave input device caps
//
DWORD
CBTPlaybackStreamManager::get_DevCaps(
    LPVOID pCaps,
    DWORD dwSize
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CBTPlaybackStreamManager::GetDevCaps()\r\n"));


    if (dwSize > sizeof(WAVEOUTCAPS))
        {
        dwSize = sizeof(WAVEOUTCAPS);
        }

    //memcpy( pCaps, &wc, dwSize);
	memset(pCaps, 0, dwSize);

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CBTPlaybackStreamManager::GetDevCaps()\r\n"));

    return MMSYSERR_NOERROR;
}
#endif


#if 1
//------------------------------------------------------------------------------
// get_ExtDevCaps not supported
//

DWORD
CBTPlaybackStreamManager::get_ExtDevCaps(
    LPVOID pCaps,
    DWORD dwSize
    )
{

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CBTPlaybackStreamManager::GetExtDevCaps()\r\n"));

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
        (L"WAV:-CBTPlaybackStreamManager::GetExtDevCaps()\r\n"));

    return MMSYSERR_NOERROR;
}
#endif

//------------------------------------------------------------------------------
//  create_Stream, create input stream context.
//
StreamContext *
CBTPlaybackStreamManager::create_Stream(
    LPWAVEOPENDESC lpWOD
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CBTPlaybackStreamManager::CreateStream()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:+CBTPlaybackStreamManager::CreateStream(), lpWOD = 0x%X\r\n", lpWOD));

	// Meanwhile do not create stream context for Voice output

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CBTPlaybackStreamManager::CreateStream()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG,
		(L"WAV:-CBTPlaybackStreamManager::CreateStream()\r\n"));

	return(NULL);
}

//------------------------------------------------------------------------------
//  copy_AudioData, callback routines from hardware bridge
//

DWORD
CBTPlaybackStreamManager::copy_AudioData(
    void* pStart,
    DWORD nSize
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CBTPlaybackStreamManager::copy_AudioData()\r\n"));

	// Meanwhile do nothing oice output
    DWORD dwActiveStreams = 0;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CBTPlaybackStreamManager::copy_AudioData()\r\n"));

    return dwActiveStreams;
}


//------------------------------------------------------------------------------
//  StreamReadyToRender, start playback of wave output stream
//
void
CBTPlaybackStreamManager::StreamReadyToRender(
    StreamContext *pStreamContext
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+CBTPlaybackStreamManager::StreamReadyToRender()\r\n"));

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CBTPlaybackStreamManager::StreamReadyToRender()\r\n"));
    return;
}

//------------------------------------------------------------------------------
// get_AudioValue, retrieve audio properties
//
DWORD
CBTPlaybackStreamManager::get_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag
    ) const
{
    DWORD                         mmRet = MMSYSERR_NOERROR;
	MIXERCONTROLDETAILS_BOOLEAN  *pOnOff;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CBTPlaybackStreamManager::get_AudioValue()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    // determine what type of control generated the event
    // and forward to the destination audio line
    switch (pControl->get_ControlType())
    {
    case MIXERCONTROL_CONTROLTYPE_ONOFF:
		{
         pOnOff = (MIXERCONTROLDETAILS_BOOLEAN *) pDetail->paDetails;
		 //mmRet =  get_HardwareAudioBridge()->GetBTPlaybackState(&BTPlaybackState);
		 pOnOff[0].fValue = get_HardwareAudioBridge()->query_BTHeadsetEnable();
 
		}
		break;

    default:
        mmRet = MMSYSERR_NOTSUPPORTED;
        break;
    }
   
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - MIXERCONTROL_CONTROLTYPE_ONOFF = %d, error = %d\r\n",__FUNCTION__, pOnOff[0].fValue, mmRet));

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CBTPlaybackStreamManager::get_AudioValue()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));

    return mmRet;
}


//------------------------------------------------------------------------------
// put_AudioValue, set audio properties
//
DWORD
CBTPlaybackStreamManager::put_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag
    )
{
    DWORD mmRet = MMSYSERR_NOERROR;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CBTPlaybackStreamManager::put_AudioValue()\r\n"));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

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

		 if( !get_HardwareAudioBridge()->enable_BTHeadset(pOnOff[0].fValue) )
		   mmRet = MMSYSERR_ERROR;

		 RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - MIXERCONTROL_CONTROLTYPE_ONOFF = %d, error = %d\r\n",__FUNCTION__, pOnOff[0].fValue, mmRet));
		}
		break;

    default:
        mmRet = MMSYSERR_NOTSUPPORTED;
        break;
    }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CBTPlaybackStreamManager::put_AudioValue()\r\n"));

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));

    return mmRet;
}


//------------------------------------------------------------------------------
//
//  Function: OutputCStreamManagerIsSupportedFormat
//
//

BOOL
CBTPlaybackStreamManager::IsSupportedFormat(LPWAVEFORMATEX lpFormat)
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CBTPlaybackStreamManager::IsSupportedFormat()\r\n"));

    return CStreamManager::IsSupportedFormat(lpFormat);
}

//------------------------------------------------------------------------------
//
//  Function: OutputCStreamManager::GetProperty
//
//
DWORD
CBTPlaybackStreamManager::GetProperty(PWAVEPROPINFO pPropInfo)
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
CBTPlaybackStreamManager::SetProperty(PWAVEPROPINFO pPropInfo)
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
// initialize_AudioLine, Voice playback output source audio line
//
BOOL
CBTPlaybackOutputSource::initialize_AudioLine(
    CAudioMixerManager *pAudioMixerManager
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+%S\r\n",__FUNCTION__));

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    // initialize Voice playback controls
    //
	register_AudioControl(&m_BTPlbOutMuteControl);
	register_AudioControl(&m_BTPlbOutVolumeControl);

    // register as the default playback mute control
    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kBTPlbOutMuteControl, &m_BTPlbOutMuteControl);

	// register as the default playback Volume control
    pAudioMixerManager->put_AudioMixerControl(
        CAudioMixerManager::kBTPlbOutVolumeControl, &m_BTPlbOutVolumeControl);


    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-%S\r\n",__FUNCTION__));

	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));

    return TRUE;
}


//------------------------------------------------------------------------------
// get_AudioValue, retrieve Playback Output audio properties
//
DWORD
CBTPlaybackOutputSource::get_AudioValue(
    CAudioControlBase *pControl,
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlag) const
{
	BOOL      BTPlbMute;
	WORD      mmRet = MMSYSERR_NOERROR;
	INT16     gainMono;


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

			mmRet = (WORD) m_pCBTPlaybackStreamManager->get_HardwareAudioBridge()->GetMuteBTOutPlayback(&BTPlbMute);
			if( mmRet == MMSYSERR_NOERROR )
			{
				switch (pDetail->cChannels)
				{
				case 1:
					pValue[0].fValue = BTPlbMute;
					break;
				default:
					mmRet = MMSYSERR_INVALPARAM;
					break;
				}
			}

			RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - GetMuteBTOutPlayback = %d\r\n",__FUNCTION__, BTPlbMute));

	   } break;

	  case  MIXERCONTROL_CONTROLTYPE_DECIBELS:
	   {
	     MIXERCONTROLDETAILS_SIGNED  *pValue;

			pValue = (MIXERCONTROLDETAILS_SIGNED *) pDetail->paDetails;
			mmRet = (WORD)m_pCBTPlaybackStreamManager->get_HardwareAudioBridge()->GetBTPlaybackOutputGain(&gainMono);
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

          RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - GetBTPlaybackOutputGain = %d\r\n",__FUNCTION__, gainMono));

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
// put_AudioValue, set playback audio properties
//
DWORD
CBTPlaybackOutputSource::put_AudioValue(
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
		 mmRet = (WORD)m_pCBTPlaybackStreamManager->get_HardwareAudioBridge()->SetMuteBTOutPlayback(pValue[0].fValue);
		}

		RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - SetMuteBTOutPlayback = %d\r\n",__FUNCTION__, pValue[0].fValue));

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
			if( (pValue[0].lValue > BTPLBOUT_GAIN_MAX )||(pValue[0].lValue < BTPLBOUT_GAIN_MIN ) )
			{
				mmRet = MMSYSERR_INVALPARAM;
			}

			if(  mmRet == MMSYSERR_NOERROR )
			{
			 mmRet = (WORD)m_pCBTPlaybackStreamManager->get_HardwareAudioBridge()->SetBTPlaybackOutputGain((INT16)pValue[0].lValue);
			}

			RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S - SetBTPlaybackOutputGain = %d\r\n",__FUNCTION__, pValue[0].lValue));

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
