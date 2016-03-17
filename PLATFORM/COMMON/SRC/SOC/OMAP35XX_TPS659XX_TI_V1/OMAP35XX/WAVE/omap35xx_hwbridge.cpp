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

#include <windows.h>
#include <omap35xx.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <wavext.h>
#include <memtxapi.h>
#include <mcbsp.h>
#include <mmddk.h>

#include <head_set.h>

#include "wavemain.h"
#include "debug.h"
#include "strmctxt.h"
#include "tps659xx_wave.h"
#include "omap35xx_hwbridge.h"


#define  HW_BRDG_AUDIO_DBG  0

//------------------------------------------------------------------------------
static void DisableAudio(HANDLE m_hHwCodec)
{
    SetHwCodecMode_Disabled(m_hHwCodec);
}

//------------------------------------------------------------------------------
static void EnableT2AudioClk(HANDLE m_hHwCodec)
{
    SetHwCodecMode_EnableT2AudioClkOnly(m_hHwCodec);
}


//------------------------------------------------------------------------------
static void RouteAudioToHandset(HANDLE m_hHwCodec, DWORD dwAudioProfile, BOOL bEnableMic)
{
#if 0
	HANDLE hHDS;
	HEADSET_MIC_AMP_CTRL mic;
#endif
	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, audioProfile = %d, bEnableMic = %d\r\n",__FUNCTION__, dwAudioProfile, bEnableMic));

	// Test
#if 0
    hHDS = CreateFile(L"HDS1:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	mic = (bEnableMic)?HEADSET_MIC_ON:HEADSET_MIC_OFF;

	if((void *)-1 != hHDS)
	{
		DeviceIoControl(hHDS, IOCTL_HEADSET_MIC_AMP_CTRL, &mic, sizeof(mic), 0, 0, 0, 0);
		CloseHandle(hHDS);
	}
	else
	{
		RETAILMSG(HW_BRDG_AUDIO_DBG, (TEXT("Route Audio to Headset is Impossible(%d)\r\n"), GetLastError()));
	}
#endif

    if(bEnableMic)
        SetHwCodecMode_Microphone(m_hHwCodec, (AudioProfile_e)dwAudioProfile);
    else
        SetHwCodecMode_Speaker(m_hHwCodec, (AudioProfile_e)dwAudioProfile);

	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
}


//------------------------------------------------------------------------------
static
void
RouteAudioToHeadset(
    HANDLE m_hHwCodec,
    DWORD dwAudioProfile,
    BOOL bEnableMic
    )
{
    if (bEnableMic)
        {
        SetHwCodecMode_MicHeadset(m_hHwCodec, (AudioProfile_e)dwAudioProfile);
        }
    else
        {
        SetHwCodecMode_Headset(m_hHwCodec, (AudioProfile_e)dwAudioProfile);
        }
}

static
void
RouteAudioToBTHeadset(
    HANDLE m_hHwCodec,
    DWORD dwAudioProfile,
    BOOL bEnableMic
    )
{
    //if (bEnableMic) - 0
	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, AudioProfile = %d\r\n",__FUNCTION__, dwAudioProfile));

    SetHwCodecMode_BTHeadset(m_hHwCodec, (AudioProfile_e)dwAudioProfile);

	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
}

static
void
RouteAudioToExtAudio(
    HANDLE m_hHwCodec,
    DWORD dwAudioProfile,
    BOOL bEnableMic
    )
{
    //if (bEnableMic) - 0
	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, AudioProfile = %d\r\n",__FUNCTION__, dwAudioProfile));

    SetHwCodecMode_ExtAudio(m_hHwCodec, (AudioProfile_e)dwAudioProfile);

	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
}


//------------------------------------------------------------------------------
static
void
RouteAudioToAuxHeadset(
    HANDLE m_hHwCodec,
    DWORD dwAudioProfile,
    BOOL bEnableMic
    )
{
    if (bEnableMic)
        {
        SetHwCodecMode_AuxHeadset(m_hHwCodec, (AudioProfile_e)dwAudioProfile);
        }
    else
        {
        SetHwCodecMode_StereoHeadset(m_hHwCodec, (AudioProfile_e)dwAudioProfile);
        }
}


//------------------------------------------------------------------------------
// #####, Michael
#if 0
static
void
RouteAudioToModemVoiceCall(
    HANDLE m_hHwCodec,
    DWORD dwAudioProfile,
    BOOL bEnableMic
    )
{
	RETAILMSG(HW_BRDG_AUDIO_DBG,
        (L"WAV:+RouteAudioToModemVoiceCall profile = %d, Mic = %d\r\n",
        dwAudioProfile, bEnableMic)
        );

	//RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    if (bEnableMic)
        {
        RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:RouteAudioToModemVoiceCall, call 'SetHwCodecMode_ModemVoiceCall'\r\n"));
        SetHwCodecMode_ModemVoiceCall(m_hHwCodec, (AudioProfile_e)dwAudioProfile);
        }
    else
        {
        RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:RouteAudioToModemVoiceCall, call 'SetHwCodecMode_Headset'\r\n"));
        SetHwCodecMode_Headset(m_hHwCodec, (AudioProfile_e)dwAudioProfile);
        }

	RETAILMSG(HW_BRDG_AUDIO_DBG,
        (L"WAV:-RouteAudioToModemVoiceCall\r\n")
        );

}
#endif


#define  ANALOG_GAIN_MIN   -24
#define  ANALOG_GAIN_MAX   12
#define  ANALOG_GAIN_STEP  2

//----------------------------------------------------------
DWORD  OMAP35XX_HwAudioBridge::SetMasterVolume(UINT16 gain)
{
 DWORD ret; 
 INT16  aGain;
 INT32  temp;
  
 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, gain = %d\r\n",__FUNCTION__, gain));

 //ret = SetOutputGainHwCodecModem(m_hHwCodec, gain ); 

 temp = ( (INT32)(gain - LOGICAL_VOLUME_MIN)*(INT32)(ANALOG_GAIN_MAX - ANALOG_GAIN_MIN) )/(INT32)(LOGICAL_VOLUME_MAX - LOGICAL_VOLUME_MIN);

 aGain = ANALOG_GAIN_MIN + (INT16)temp;

 ret = SetAnalogGainHwCodec(m_hHwCodec, aGain ); 
 //ret = 0;

 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S, aGain = %d\r\n",__FUNCTION__, aGain));
 return(ret);
}

DWORD  OMAP35XX_HwAudioBridge::GetMasterVolume(UINT16 *pGain)
{
 DWORD ret; 
 INT16  aGain;
 INT32  temp;
  
 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

 ret = GetAnalogGainHwCodec(m_hHwCodec, &aGain ); 
 //RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S, aGain = %d\r\n",__FUNCTION__, aGain));

 temp = ( (aGain - ANALOG_GAIN_MIN)*(LOGICAL_VOLUME_MAX - LOGICAL_VOLUME_MIN) )/(ANALOG_GAIN_MAX - ANALOG_GAIN_MIN);
 //RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S, temp gain = %d\r\n",__FUNCTION__, temp));

 *pGain = (UINT16) (LOGICAL_VOLUME_MIN + temp);

 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S, gain = %d\r\n",__FUNCTION__, *pGain));
 return(ret);
}


DWORD  OMAP35XX_HwAudioBridge::StartModemVoice(BOOL btOn)
{
 DWORD ret;
 AudioMode_e inAudioMode, outAudioMode;

 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, btOn = %d\r\n",__FUNCTION__, btOn));

	// Choose mode with active microphone for Voice
// if( !btOn )
//  {
    switch (m_CurrentAudioRoute)
	{
     case OMAP35XX_HwAudioBridge::kAudioRoute_AuxHeadset: 
	  {
	   inAudioMode = kAudioModeAuxHeadset; 
	   outAudioMode = kAudioModeStereoHeadset;
	  }break;

     case OMAP35XX_HwAudioBridge::kAudioRoute_BTHeadset:
     case OMAP35XX_HwAudioBridge::kAudioRoute_Headset:
	  {
       inAudioMode = kAudioModeMicHeadset; 
	   outAudioMode = kAudioModeHeadset;
	  }break;
	
     case OMAP35XX_HwAudioBridge::kAudioRoute_Carkit:
     case OMAP35XX_HwAudioBridge::kAudioRoute_Speaker:
     case OMAP35XX_HwAudioBridge::kAudioRoute_Handset:
	  {
       inAudioMode = kAudioModeMicrophone; 
	   outAudioMode = kAudioModeSpeaker;
	  }break;

	 case OMAP35XX_HwAudioBridge::kAudioRoute_ExtAudio:
	  {
		 inAudioMode = kAudioModeExtAudio;
		 outAudioMode = kAudioModeExtAudio;
	  } break;

#if defined(SYSGEN_BTH_AG)
	  case OMAP35XX_HwAudioBridge::kAudioRoute_BluetoothAudio:
	  {
		 inAudioMode = kAudioModeBluetoothAudio;
		 outAudioMode = kAudioModeBluetoothAudio;
	  } break;
#endif
           
	 default: return(MMSYSERR_ERROR);
	}
//  }
// else
//  {
//       inAudioMode = kAudioModeDisabled; 
//	   outAudioMode = kAudioModeDisabled;
//  }

	m_ModemVoiceActive = TRUE;
	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S, AudioProfile = %d, inAudioMode = %d, outAudioMode = %d\r\n",__FUNCTION__, m_dwAudioProfile, inAudioMode, outAudioMode));
	ret = StartHwCodecModem(m_hHwCodec, (AudioProfile_e)m_dwAudioProfile, inAudioMode, outAudioMode, btOn );

	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
	return(ret);
	//return(MMSYSERR_NOERROR);
}


DWORD  OMAP35XX_HwAudioBridge::StopModemVoice(BOOL btOn)
{
 DWORD ret;

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

	ret = StopHwCodecModem(m_hHwCodec, btOn);
	m_ModemVoiceActive = FALSE;

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
    return(ret);
}

DWORD  OMAP35XX_HwAudioBridge::GetModemVoiceState(DWORD *pModemVoiceState, BOOL *pBTOn)
{
 DWORD ret; 

 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

	ret = GetStateHwCodecModem(m_hHwCodec, pModemVoiceState, pBTOn);

 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S, ModemVoiceState = %d\r\n",__FUNCTION__, *pModemVoiceState ));

    return(ret);
}


DWORD  OMAP35XX_HwAudioBridge::SetModemVoiceOutGain(INT16 gain)
{
 DWORD ret; 
  
 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, gain = %d\r\n",__FUNCTION__, gain));

 ret = SetOutputGainHwCodecModem(m_hHwCodec, gain ); 

 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
 return(ret);
}

DWORD  OMAP35XX_HwAudioBridge::GetModemVoiceOutGain(INT16 *pGain)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

	ret = GetOutputGainHwCodecModem(m_hHwCodec, pGain);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S, gain = %d\r\n",__FUNCTION__, *pGain));
	return(MMSYSERR_NOERROR);
}

DWORD  OMAP35XX_HwAudioBridge::SetModemVoiceInputGain(INT16 gain)
{
 DWORD ret; 
 
 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, gain = %d\r\n",__FUNCTION__, gain));

 ret = SetInputGainHwCodecModem(m_hHwCodec, gain ); 

 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
 return(ret);
}

DWORD  OMAP35XX_HwAudioBridge::GetModemVoiceInputGain(INT16 *pGain)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

	ret = GetInputGainHwCodecModem(m_hHwCodec, pGain);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S, gain = %d\r\n",__FUNCTION__, *pGain));
	return(MMSYSERR_NOERROR);
}


DWORD  OMAP35XX_HwAudioBridge::SetModemBTVoiceInputGain(INT16 gain)
{
 DWORD ret; 
 
 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, gain = %d\r\n",__FUNCTION__, gain));

 ret = SetBTInputGainHwCodecModem(m_hHwCodec, gain ); 

 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
 return(ret);
}


DWORD  OMAP35XX_HwAudioBridge::GetModemBTVoiceInputGain(INT16 *pGain)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

	ret = GetBTInputGainHwCodecModem(m_hHwCodec, pGain);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S, gain = %d\r\n",__FUNCTION__, *pGain));
	return(MMSYSERR_NOERROR);
}


DWORD  OMAP35XX_HwAudioBridge::SetModemBTVoiceOutputGain(INT16 gain)
{
 DWORD ret; 
 
 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, gain = %d\r\n",__FUNCTION__, gain));

 ret = SetBTOutputGainHwCodecModem(m_hHwCodec, gain ); 

 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
 return(ret);
}


DWORD  OMAP35XX_HwAudioBridge::GetModemBTVoiceOutputGain(INT16 *pGain)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

	ret = GetBTOutputGainHwCodecModem(m_hHwCodec, pGain);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S, gain = %d\r\n",__FUNCTION__, *pGain));
	return(MMSYSERR_NOERROR);
}




DWORD  OMAP35XX_HwAudioBridge::GetModemMicMuteStatus(BOOL *pMute)
{
	if(pMute)
		*pMute = GetMicMuteHwCodec(m_hHwCodec);

	return MMSYSERR_NOERROR;
}

// 22-Feb-2011
DWORD  OMAP35XX_HwAudioBridge::GetExtMicMuteStatus(BOOL *pMute)
{
	if(pMute)
		*pMute = GetExtMicMuteHwCodec(m_hHwCodec);

	return MMSYSERR_NOERROR;
}


DWORD  OMAP35XX_HwAudioBridge::SetModemMicMute(BOOL mute)
{
 DWORD ret; 

 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, mute = %d\r\n",__FUNCTION__, mute));

 ret = SetMicMuteHwCodec(m_hHwCodec, mute);

 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
 return(ret);
}

// 22-Feb-2011
DWORD  OMAP35XX_HwAudioBridge::SetExtMicMute(BOOL mute)
{
 DWORD ret; 

 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, mute = %d\r\n",__FUNCTION__, mute));

 ret = SetExtMicMuteHwCodec(m_hHwCodec, mute);

 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
 return(ret);
}


DWORD  OMAP35XX_HwAudioBridge::GetModemMicAnalogGain(UINT16 *pAGain)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

	ret = GetModemMicAnalogGainHwCodecModem(m_hHwCodec, pAGain);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S, gain = %d\r\n",__FUNCTION__, *pAGain));
	return(ret);
}

// 22-Feb-2011
DWORD  OMAP35XX_HwAudioBridge::GetExtMicAnalogGain(UINT16 *pAGain)
{
	DWORD ret; 

	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

	ret = GetModemMicAnalogGainHwCodecModemExt(m_hHwCodec, pAGain);

	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S, gain = %d\r\n",__FUNCTION__, *pAGain));

	return ret;
}


DWORD  OMAP35XX_HwAudioBridge::SetModemMicAnalogGain(UINT16 AGain)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, gain = %d\r\n",__FUNCTION__, AGain));

	ret = SetModemMicAnalogGainHwCodecModem(m_hHwCodec, AGain);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
	return(ret);
}


// 22-Feb-2011
DWORD  OMAP35XX_HwAudioBridge::SetExtMicAnalogGain(UINT16 AGain)
{
	DWORD ret; 

	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, gain = %d\r\n",__FUNCTION__, AGain));

	ret = SetModemMicAnalogGainHwCodecModemExt(m_hHwCodec, AGain);

	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));

	return(ret);
}


DWORD  OMAP35XX_HwAudioBridge::GetModemMicDigitalGain(UINT16 *pDigGain)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

	ret = GetModemMicDigitalGainHwCodecModem(m_hHwCodec, pDigGain);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S, gain = %d\r\n",__FUNCTION__, *pDigGain));
	return(ret);
}

// 22-Feb-2011
DWORD  OMAP35XX_HwAudioBridge::GetExtMicDigitalGain(INT16 *pDigGain)
{
	DWORD ret; 

	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

	ret = GetModemMicDigitalGainHwCodecModemExt(m_hHwCodec, (UINT16 *)pDigGain);

	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S, gain = %d\r\n",__FUNCTION__, *pDigGain));

	return ret;
}


DWORD  OMAP35XX_HwAudioBridge::SetModemMicDigitalGain(UINT16 DigGain)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, gain = %d\r\n",__FUNCTION__, DigGain));

	ret = SetModemMicDigitalGainHwCodecModem(m_hHwCodec, DigGain);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
	return(ret);
}

// 22-Feb-2011
DWORD  OMAP35XX_HwAudioBridge::SetExtMicDigitalGain(INT16 DigGain)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, gain = %d\r\n",__FUNCTION__, DigGain));

	ret = SetModemMicDigitalGainHwCodecModemExt(m_hHwCodec, DigGain);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
	return(ret);
}


DWORD  OMAP35XX_HwAudioBridge::SetMuteModemVoiceOut(BOOL mute)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, mute = %d\r\n",__FUNCTION__, mute));

	ret = SetVoiceOutMuteHwCodecModem(m_hHwCodec, mute);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
	return(ret);
}

DWORD  OMAP35XX_HwAudioBridge::GetMuteModemVoiceOut(BOOL *pMute)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

	ret = GetVoiceOutMuteHwCodecModem(m_hHwCodec, pMute);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S, mute = %d\r\n",__FUNCTION__, *pMute));
	return(ret);
}

DWORD  OMAP35XX_HwAudioBridge::SetMuteModemVoiceIn(BOOL mute)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, mute = %d\r\n",__FUNCTION__, mute));

	ret = SetVoiceInMuteHwCodecModem(m_hHwCodec, mute);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
	return(ret);
}

DWORD OMAP35XX_HwAudioBridge::GetMuteModemVoiceIn(BOOL *pMute)
{
	if(pMute)
		*pMute = GetVoiceInMuteHwCodecModem(m_hHwCodec);

	return MMSYSERR_NOERROR;
}


DWORD  OMAP35XX_HwAudioBridge::SetMuteModemBTInVoice(BOOL mute)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, mute = %d\r\n",__FUNCTION__, mute));

	ret = SetMuteBTInHwCodecModem(m_hHwCodec, mute);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
	return(ret);
}

DWORD  OMAP35XX_HwAudioBridge::GetMuteModemBTInVoice(BOOL *pMute)
{
	if(pMute)
		*pMute = GetMuteBTInHwCodecModem(m_hHwCodec);

	return MMSYSERR_NOERROR;
}


DWORD  OMAP35XX_HwAudioBridge::SetMuteModemBTOutVoice(BOOL mute)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, mute = %d\r\n",__FUNCTION__, mute));

	ret = SetMuteBTOutHwCodecModem(m_hHwCodec, mute);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
	return(ret);
}

DWORD  OMAP35XX_HwAudioBridge::GetMuteModemBTOutVoice(BOOL *pMute)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

	ret = GetMuteBTOutHwCodecModem(m_hHwCodec, pMute);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S, mute = %d\r\n",__FUNCTION__, *pMute));
	return(ret);
}


//DWORD  OMAP35XX_HwAudioBridge::StartBTPlayback(void);
//DWORD  OMAP35XX_HwAudioBridge::StopBTPlayback(void);
//DWORD  OMAP35XX_HwAudioBridge::GetBTPlaybackState(DWORD *pBTPlaybackState);

DWORD  OMAP35XX_HwAudioBridge::SetMuteBTOutPlayback(BOOL mute)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

  ret = SetMuteBTOutPlaybackCodecModem(m_hHwCodec, mute);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S, mute = %d\r\n",__FUNCTION__, mute));
  return(ret);
}

DWORD  OMAP35XX_HwAudioBridge::GetMuteBTOutPlayback(BOOL *pMute)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

  ret = GetMuteBTOutPlaybackCodecModem(m_hHwCodec, pMute);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S, mute = %d\r\n",__FUNCTION__, *pMute));
  return(ret);

}

DWORD  OMAP35XX_HwAudioBridge::SetBTPlaybackOutputGain(INT16 gain)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

  ret = SetBTOutputGainHwCodecModem(m_hHwCodec, gain);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S, gain = %d\r\n",__FUNCTION__, gain));
  return ret;

}

DWORD  OMAP35XX_HwAudioBridge::GetBTPlaybackOutputGain(INT16 *pGain)
{
  DWORD ret; 

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

  ret = GetBTOutputGainHwCodecModem(m_hHwCodec, pGain);

  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S, gain = %d\r\n",__FUNCTION__, *pGain));
  return(ret);

}


//------------------------------------------------------------------------------
// configures triton to route audio appropriately
//
void
OMAP35XX_HwAudioBridge::SetAudioPath(
    AudioRoute_e audioroute,
    DWORD dwAudioProfile
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+OMAP35XX_HwAudioBridge:SetAudioPath(%d)\r\n",
        audioroute)
        );

	RETAILMSG(HW_BRDG_AUDIO_DBG,
        (L"WAV:+OMAP35XX_HwAudioBridge : SetAudioPath(%d), AudioProfile(%d), m_TransmitterState(%d)\r\n",
        audioroute, dwAudioProfile, m_TransmitterState)
        );

    BOOL bReceiverIdle = m_ReceiverState == kAudioRender_Idle;

    if (audioroute == OMAP35XX_HwAudioBridge::kAudioRoute_HdmiAudio)
        {
        EnableT2AudioClk(m_hHwCodec);
        }
    else
        {
        if( (m_TransmitterState == kAudioRender_Idle && bReceiverIdle) && 
			(!m_ModemVoiceActive) && 
			(audioroute != kAudioRoute_BTHeadset) &&
			(audioroute != kAudioRoute_ExtAudio) &&
			(audioroute != kAudioRoute_Speaker)
		   )
            {
            DisableAudio(m_hHwCodec);
			RETAILMSG(HW_BRDG_AUDIO_DBG,(L"WAV:OMAP35XX_HwAudioBridge:SetAudioPath(): DisableAudio, kAudioRender_Idle = 0x%X, bReceiverIdle = 0x%X\r\n",
				         kAudioRender_Idle, bReceiverIdle ) );
            }
        else
            {
             RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S, audioroute = %d\r\n",__FUNCTION__, audioroute));

            switch (audioroute)
                {
                case OMAP35XX_HwAudioBridge::kAudioRoute_AuxHeadset:
                    RouteAudioToAuxHeadset(m_hHwCodec, dwAudioProfile, 
                        bReceiverIdle == FALSE);
                    break;
                    
                case OMAP35XX_HwAudioBridge::kAudioRoute_BTHeadset:
					{
                     // Config BT Play, Microphone disabled
                     RouteAudioToBTHeadset(m_hHwCodec, dwAudioProfile, 0);
                     //RouteAudioToHandset(m_hHwCodec, dwAudioProfile, bReceiverIdle == FALSE);
					} break;

				case OMAP35XX_HwAudioBridge::kAudioRoute_ExtAudio:
					{
                    RouteAudioToExtAudio(m_hHwCodec, dwAudioProfile,
                        bReceiverIdle == FALSE);
					} break;

#if defined(SYSGEN_BTH_AG)
				case OMAP35XX_HwAudioBridge::kAudioRoute_BluetoothAudio:
					{
						SetHwCodecMode_BthAudio(m_hHwCodec, (AudioProfile_e)dwAudioProfile);
					}
					break;
#endif

                case OMAP35XX_HwAudioBridge::kAudioRoute_Headset:
                    RouteAudioToHeadset(m_hHwCodec, dwAudioProfile,
                        bReceiverIdle == FALSE);
                    break;

                case OMAP35XX_HwAudioBridge::kAudioRoute_Carkit:
                case OMAP35XX_HwAudioBridge::kAudioRoute_Speaker:
                case OMAP35XX_HwAudioBridge::kAudioRoute_Handset:
                    RouteAudioToHandset(m_hHwCodec, dwAudioProfile, bReceiverIdle == FALSE);
                    break;

                // #####, Michael 
#if 0
                case OMAP35XX_HwAudioBridge::kAudioRoute_ModemVoiceCall:
					{
					  RETAILMSG(HW_BRDG_AUDIO_DBG,(L"WAV:-OMAP35XX_HwAudioBridge:SetAudioPath(): RouteAudioToModemVoiceCall\r\n") );
                      RouteAudioToModemVoiceCall(m_hHwCodec, dwAudioProfile, 1 /*bReceiverIdle == FALSE*/);
					} break;
#endif
                default:
                    break;
                }
            }
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-OMAP35XX_HwAudioBridge:SetAudioPath()\r\n")
        );

	RETAILMSG(HW_BRDG_AUDIO_DBG,
        (L"WAV:-OMAP35XX_HwAudioBridge:SetAudioPath()\r\n")
        );

}

//------------------------------------------------------------------------------
//  initializes the hardware bridge
//
void
OMAP35XX_HwAudioBridge::initialize(
    WCHAR const *szDMTDriver,
    WCHAR const *szDASFDriver,
    HwCodecConfigInfo_t *pHwCodecConfigInfo,
    HANDLE hPlayPortConfigInfo,
    HANDLE hRecPortConfigInfo,
	AudioRoute_e eAudioRoute
    )
{
    PortConfigInfo_t *pPortConfigInfo = (PortConfigInfo_t*)hPlayPortConfigInfo;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+OMAP35XX_HwAudioBridge:initialize()\r\n")
        );

	RETAILMSG(HW_BRDG_AUDIO_DBG,
        (L"WAV:+OMAP35XX_HwAudioBridge:initialize()\r\n")
        );

    DEBUGMSG(ZONE_HWBRIDGE, (L"WAV:initializing hardware\r\n"));
	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:initializing hardware\r\n"));

    m_RequestAudioRoute = eAudioRoute;

    m_dwAudioProfile = pPortConfigInfo->portProfile;

    m_hHwCodec = OpenHwCodecHandle(pHwCodecConfigInfo);

    if (m_hHwCodec == NULL)
        {
        RETAILMSG(ZONE_ERROR,
            (L"WAV:!ERROR Can't load Triton Driver err=0x%08X\r\n",
            GetLastError()));
        goto code_exit;
        }

    DisableAudio(m_hHwCodec);


    // open and register direct memory transfer port
    //
    m_DMTPort.register_PORTHost(this);
    m_DMTPort.open_Port(szDMTDriver, hPlayPortConfigInfo, hRecPortConfigInfo);

    // open and register DASF transfer port
    //
    m_DASFPort.register_PORTHost(this);
    m_DASFPort.open_Port(szDASFDriver, hPlayPortConfigInfo, hRecPortConfigInfo);

    // set default port to the DMT port
    //
    m_pActivePort = &m_DMTPort;
    m_DMTPort.set_DMTProfile(DMTAudioStreamPort::DMTProfile_I2SSlave);

code_exit:
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-OMAP35XX_HwAudioBridge:initialize()\r\n")
        );
	RETAILMSG(HW_BRDG_AUDIO_DBG,
        (L"WAV:-OMAP35XX_HwAudioBridge:initialize()\r\n")
        );

}


//------------------------------------------------------------------------------
//  updates the audio routing
//
BOOL
OMAP35XX_HwAudioBridge::update_AudioRouting()
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+OMAP35XX_HwAudioBridge::update_AudioRouting\r\n")
        );

	RETAILMSG(HW_BRDG_AUDIO_DBG,
        (L"WAV:+OMAP35XX_HwAudioBridge::update_AudioRouting, m_TransmitterState(%d), m_CurrentAudioRoute(%d), m_RequestAudioRoute(%d)\r\n", 
		 m_TransmitterState, m_CurrentAudioRoute, m_RequestAudioRoute)
		 );

    AudioStreamPort *pActivePort = NULL;
    BOOL bPortUpdated = FALSE;

    // UNDONE:
    // In certain elaborate use case scenario's we can get out of sync
    // with smartphone's audio profile because we override certain
    // profiles.  It could be the smartphone shell sheilds us from
    // these elaborate use case scenarios by preventing the user to
    // change audio profile when certain audio profiles are forced
    //

    // clear the audio request change dirty bit
    //
    m_fRequestAudioRouteDirty = FALSE;

    // Bluetooth headsets are a special case so check for this first
    //Test
    if(0)  //(query_BTHeadsetAttached() == TRUE)
        {
		 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S, query_BTHeadsetAttached, CurrentAudioRoute = %d\r\n",__FUNCTION__, m_CurrentAudioRoute));
        // for bluetooth we do the following
        //  1) set recording/rendering rate to 8khz
        //  2) switch to ICX based data port
        //  3) update audio routing to go through bluetooth
        //
            m_prgStreams[kOutput]->put_AudioSampleRate(
                    CStreamCallback::k44khz
                    );

        if (m_CurrentAudioRoute != kAudioRoute_BTHeadset)
            {
            // when receiver is enabled we need to set the sample
            // rate to 8 khz for both input and output
            //
#if 0
            m_prgStreams[kOutput]->put_AudioSampleRate(
                    CStreamCallback::k8khz
                    );

            m_prgStreams[kInput]->put_AudioSampleRate(
                    CStreamCallback::k8khz
                    );
#endif
            m_CurrentAudioRoute = kAudioRoute_BTHeadset;
			m_dwAudioProfile = kAudioI2SProfile;
            //pActivePort = &m_DASFPort;
            pActivePort = &m_DMTPort; 
            bPortUpdated = TRUE;
            }
        }
	// #####, Michael, ModemVoiceCall
	else if (query_ModemVoiceCallEnable() == TRUE)
		{
         RETAILMSG(HW_BRDG_AUDIO_DBG,(L"WAV:OMAP35XX_HwAudioBridge::update_AudioRouting: query_ModemVoiceEnable\r\n"));

         if (m_CurrentAudioRoute != kAudioRoute_ModemVoiceCall)
            {
			RETAILMSG(HW_BRDG_AUDIO_DBG,(L"WAV:OMAP35XX_HwAudioBridge::update_AudioRouting: ModemVoiceEnable\r\n"));

            // when receiver is enabled we need to set the sample
            // rate to 8 khz for both input and output
            //
#if 0        
            m_prgStreams[kOutput]->put_AudioSampleRate(
                    CStreamCallback::k8khz
                    );

            m_prgStreams[kInput]->put_AudioSampleRate(
                    CStreamCallback::k8khz
                    );
#endif
            m_CurrentAudioRoute = kAudioRoute_ModemVoiceCall;
			m_dwAudioProfile = kAudioPCMProfile;
            pActivePort = &m_DASFPort;
            bPortUpdated = TRUE;
            }

		}

    // HDMI is attached!
    //
    else if (query_HdmiAudioAttached() == TRUE)
        {
			RETAILMSG(HW_BRDG_AUDIO_DBG,(L"WAV:OMAP35XX_HwAudioBridge::update_AudioRouting: query_HdmiAudioAttached\r\n"));
        // for hdmi audio, we do the following
        //  1) set recording/rendering rate to 44.1khz
        //  2) switch to data port (McBSP I2S Master)
        //  3) Update the audio routing to go through hdmi controller
        //
        if (m_CurrentAudioRoute != kAudioRoute_HdmiAudioRequested)
            {
             RETAILMSG(HW_BRDG_AUDIO_DBG,(L"WAV:OMAP35XX_HwAudioBridge::update_AudioRouting: HdmiAudioAttached\r\n"));
            // set transceiver port to MCBSP port
            //
            pActivePort = &m_DMTPort;
            m_DMTPort.set_DMTProfile(DMTAudioStreamPort::DMTProfile_I2SMaster);
            bPortUpdated = TRUE;

            m_CurrentAudioRoute = kAudioRoute_HdmiAudioRequested;
			m_dwAudioProfile = kAudioI2SProfile;
            }
        else if (m_CurrentAudioRoute == kAudioRoute_HdmiAudioRequested)
            {

            if (m_ReceiverState != kAudioRender_Idle)
                {
                // when receiver is enabled we need to set the sample
                // rate to 44.1 khz for both input and output
                //
                m_prgStreams[kOutput]->put_AudioSampleRate(
                        CStreamCallback::k44khz
                        );

                m_prgStreams[kInput]->put_AudioSampleRate(
                        CStreamCallback::k44khz
                        );
                }
            else
                {
                // set default output rate of 44.1 khz
                //
                m_prgStreams[kOutput]->put_AudioSampleRate(
                        CStreamCallback::k44khz
                        );
                }

            m_CurrentAudioRoute = kAudioRoute_HdmiAudio;
			m_dwAudioProfile = kAudioI2SProfile;
            }
        }
    else
        {
		 RETAILMSG(HW_BRDG_AUDIO_DBG,(L"WAV:OMAP35XX_HwAudioBridge::update_AudioRouting: no query\r\n"));

        // for non-bluetooth case do the following
        //  1) set rcording/rendering rate
        //  2) switch to using MCBSP based data port
        //  3) update audio routing
        //
        if (m_ReceiverState != kAudioRender_Idle)
            {
            // when receiver is enabled we need to set the sample
            // rate to 44.1 khz for both input and output
            //
            m_prgStreams[kOutput]->put_AudioSampleRate(
                    CStreamCallback::k44khz
                    );

            m_prgStreams[kInput]->put_AudioSampleRate(
                    CStreamCallback::k44khz
                    );
            }
        else
            {
            // set default output rate of 44.1 khz
            //
            m_prgStreams[kOutput]->put_AudioSampleRate(
                    CStreamCallback::k44khz
                    );
            }

        // set transceiver port to MCBSP port
        //
        if (m_bPortSwitch == TRUE)
            {
            DEBUGMSG(ZONE_HWBRIDGE, (L"WAV:DASF port Rendering\r\n"));
			RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:DASF port Rendering\r\n"));
            // set DASF as active port
            //
            pActivePort = &m_DASFPort;
            bPortUpdated = TRUE;

            // update the active port state
            //
            m_CurrActivePort = kAudioDASFPort;
            }
        else
            {
            DEBUGMSG(ZONE_HWBRIDGE, (L"WAV:McBSP port Rendering\r\n"));
			RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:McBSP port Rendering\r\n"));
            // set transceiver port to MCBSP port
            //
            if ((m_pActivePort != &m_DMTPort) ||
                (m_DMTPort.get_DMTProfile() != DMTAudioStreamPort::DMTProfile_I2SSlave))
                {
                pActivePort = &m_DMTPort;
                m_DMTPort.set_DMTProfile(DMTAudioStreamPort::DMTProfile_I2SSlave);
                bPortUpdated = TRUE;
                }

            // update the active port state
            //
            m_CurrActivePort = kAudioDMTPort;
            }

        // NOTE: This is a special state only to handle HDMI detach, since we are
        // required to stop the T2 only after the McBSP has stopped completely.
        if ((m_CurrentAudioRoute == kAudioRoute_HdmiAudio) &&
            (m_TransmitterState == kAudioRender_Active))
            {
            m_CurrentAudioRoute = kAudioRoute_HdmiAudioDetached;
            }
            else
            {
            // check if speaker mode should be forced
            //
            RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S, m_dwSpeakerCount = %d\r\n", __FUNCTION__, m_dwSpeakerCount));
            if (m_dwSpeakerCount > 0)
                {
                m_CurrentAudioRoute = kAudioRoute_Speaker;
                }
            else
                {
                m_CurrentAudioRoute = m_RequestAudioRoute;
                }

			 // Test: Additions 
			if( query_BTHeadsetAttached() == TRUE )
			 {
              m_CurrentAudioRoute = kAudioRoute_BTHeadset; 
              m_prgStreams[kOutput]->put_AudioSampleRate(CStreamCallback::k8khz);
			  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:BTHeadsetAttached, output 8khz\r\n"));
			 }

			if( query_ExtAudioAttached() == TRUE )
			 {
              m_CurrentAudioRoute = kAudioRoute_ExtAudio; 
			  RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:ExtAudioAttached, output 44khz\r\n"));
			 }

#if defined(SYSGEN_BTH_AG)
			if (m_RequestAudioRoute == kAudioRoute_BluetoothAudio)
				m_CurrentAudioRoute = kAudioRoute_BluetoothAudio;
#endif

            }

		 m_dwAudioProfile = kAudioI2SProfile;
        }

    SetAudioPath(m_CurrentAudioRoute, m_dwAudioProfile);

    // if no activity then update power state to D4
    //
    if ((m_ReceiverState == kAudioRender_Idle) &&
        (m_TransmitterState == kAudioRender_Idle))
        {

		 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S, idle, TxState = %d, rxState = %d\r\n",__FUNCTION__, m_TransmitterState, m_ReceiverState));
         m_PowerState = D4;

        // Check if render port has changed if so update the same.
        //
        if (bPortUpdated == TRUE)
            {
            m_pActivePort = pActivePort;
            }
        }
    else
        {
         RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S, no idle, TxState = %d, rxState = %d, portupdated = %d\r\n",__FUNCTION__, m_TransmitterState, m_ReceiverState, bPortUpdated));
        if (bPortUpdated == TRUE)
            {
            // if a port is busy, stop previous port and activate new port.
            //

            if (m_ReceiverState != kAudioRender_Idle)
                {
                // need to check if there's any data left to render in port
                // if not then don't bother activating port
                //
                m_pActivePort->signal_Port(ASPS_STOP_RX, NULL, 0);
                }

            if (m_TransmitterState != kAudioRender_Idle)
                {
                // need to check if there's any data left to render in port
                // if not then don't bother activating port
                //
                m_pActivePort->signal_Port(ASPS_STOP_TX, NULL, 0);
                }
            m_pActivePort = pActivePort;
            }
        m_PowerState = D0;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-OMAP35XX_HwAudioBridge::update_AudioRouting\r\n")
        );

    RETAILMSG(HW_BRDG_AUDIO_DBG,
        (L"WAV:-OMAP35XX_HwAudioBridge::update_AudioRouting\r\n")
        );

    return TRUE;
}


//------------------------------------------------------------------------------
//  starts audio port
//
BOOL
OMAP35XX_HwAudioBridge::start_AudioPort(
    StreamType type
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+OMAP35XX_HwAudioBridge::start_AudioPort\r\n")
        );

    RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S, m_TransmitterState = %d\r\n",__FUNCTION__, m_TransmitterState));

    // check if audio stream is already turned-on.  If
    // so then don't turn-on again.  There is a edge case
    // where we will try to turn-on audio while
    // the last bit of data was pushed-out.  In this case
    // we still send the on message to the hardware
    //
    int nRet = FALSE;
    switch (type)
        {
        case kInput:
            DEBUGMSG(ZONE_HWBRIDGE, (L"WAV:HWBridge - Starting input stream\r\n"));
			RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S - Starting input stream\r\n",__FUNCTION__));
            if ((m_ReceiverState == kAudioRender_Idle) ||
                (m_ReceiverState == kAudioRender_Stopping))
                {
                switch (m_ReceiverState)
                    {
                    case kAudioRender_Idle:
                        DEBUGMSG(ZONE_HWBRIDGE,
                            (L"WAV:HWBridge - rx idle --> starting")
                            );

						RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S - rx idle --> starting\r\n",__FUNCTION__));
						RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S prev rx state = %d, current rx state = %d\r\n",__FUNCTION__, m_ReceiverState, kAudioRender_Starting));

                        m_ReceiverState = kAudioRender_Starting;
                        update_AudioRouting();
                        break;

                    case kAudioRender_Stopping:
                        DEBUGMSG(ZONE_HWBRIDGE,
                            (L"WAV:HWBridge - rx stopping --> active")
                            );
						RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S - rx stopping --> active\r\n",__FUNCTION__));
						RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S prev rx state = %d, current rx state = %d\r\n",__FUNCTION__, m_ReceiverState, kAudioRender_Active));

                        m_ReceiverState = kAudioRender_Active;
                        break;
                    }
                nRet = m_pActivePort->signal_Port(ASPS_START_RX, NULL, 0);
                }
            break;

        case kOutput:
            DEBUGMSG(ZONE_HWBRIDGE, (L"WAV:HWBridge - Starting output stream\r\n"));
			RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S - Starting output stream\r\n",__FUNCTION__));

            if ((m_TransmitterState == kAudioRender_Idle) ||
                (m_TransmitterState == kAudioRender_Stopping))
                {
                switch (m_TransmitterState)
                    {
                    case kAudioRender_Idle:
                        DEBUGMSG(ZONE_HWBRIDGE,
                            (L"WAV:HWBridge - tx idle --> starting")
                            );
						RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S - tx idle --> starting\r\n",__FUNCTION__));
						RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S prev tx state = %d, current tx state = %d\r\n",__FUNCTION__, m_TransmitterState, kAudioRender_Starting));

                        m_TransmitterState = kAudioRender_Starting;
                        update_AudioRouting();
                        break;

                    case kAudioRender_Stopping:
                        DEBUGMSG(ZONE_HWBRIDGE,
                            (L"WAV:HWBridge - tx stopping --> active")
                            );
						RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S - tx stopping --> active\r\n",__FUNCTION__));
						RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S prev tx state = %d, current tx state = %d\r\n",__FUNCTION__, m_TransmitterState, kAudioRender_Active));

                        m_TransmitterState = kAudioRender_Active;
                        break;
                    }
                nRet = m_pActivePort->signal_Port(ASPS_START_TX, NULL, 0);
                }
            break;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-OMAP35XX_HwAudioBridge::start_AudioPort\r\n")
        );

	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
    return !!nRet;
}


//------------------------------------------------------------------------------
// stop audio port
//
BOOL
OMAP35XX_HwAudioBridge::stop_AudioPort(
    StreamType type
    )
{
    int nRet = FALSE;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+OMAP35XX_HwAudioBridge::stop_AudioPort\r\n")
        );
	RETAILMSG(HW_BRDG_AUDIO_DBG,
        (L"WAV:+OMAP35XX_HwAudioBridge::stop_AudioPort, m_TransmitterState(%d)\r\n", m_TransmitterState)
        );

    switch (type)
        {
        case kInput:
            DEBUGMSG(ZONE_HWBRIDGE,
                (L"WAV:HWBridge - Stopping input stream\r\n")
                );
			RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S - Stopping input stream\r\n",__FUNCTION__));
			
            if ((m_ReceiverState == kAudioRender_Active) ||
                (m_ReceiverState == kAudioRender_Starting))
                {
                 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S prev rx state = %d, current rx state = %d\r\n",__FUNCTION__, m_ReceiverState, kAudioRender_Stopping));
                 m_ReceiverState = kAudioRender_Stopping;
                 nRet = m_pActivePort->signal_Port(ASPS_STOP_RX, NULL, 0);
                }
            break;

        case kOutput:
            DEBUGMSG(ZONE_HWBRIDGE,
                (L"WAV:HWBridge - Stopping output stream\r\n")
                );
			RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S - Stopping output stream\r\n",__FUNCTION__));

            if ((m_TransmitterState == kAudioRender_Active) ||
                (m_TransmitterState == kAudioRender_Starting))
                {
                 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S prev tx state = %d, current tx state = %d\r\n",__FUNCTION__, m_TransmitterState, kAudioRender_Stopping));
                 m_TransmitterState = kAudioRender_Stopping;
                 nRet = m_pActivePort->signal_Port(ASPS_STOP_TX, NULL, 0);
                }
            break;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-OMAP35XX_HwAudioBridge::stop_AudioPort\r\n")
        );
	RETAILMSG(HW_BRDG_AUDIO_DBG,
        (L"WAV:-OMAP35XX_HwAudioBridge::stop_AudioPort\r\n")
        );

    return !!nRet;
}

//------------------------------------------------------------------------------
//  starts audio streaming
//
BOOL
OMAP35XX_HwAudioBridge::start_Stream(
    StreamType type,
    HANDLE hStreamContext
    )
{
    int nRet = FALSE;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+OMAP35XX_HwAudioBridge::start_Stream\r\n")
        );
	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S 2 tx state = %d\r\n",__FUNCTION__, m_TransmitterState));

    // check if audio stream is already turned-on.  If
    // so then don't turn-on again.  There is a edge case
    // where we will try to turn-on audio while
    // the last bit of data was pushed-out.  In this case
    // we still send the on message to the hardware
    //

    switch (type)
        {
        case kInput:
            DEBUGMSG(ZONE_HWBRIDGE,
                (L"WAV:HWBridge - Starting input stream\r\n")
                );
			RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S - Starting input stream\r\n",__FUNCTION__));

            RETAILMSG(m_ReceiverState != kAudioRender_Active && ZONE_ERROR,
                (L"%S - Can't start stream when port is inactive\r\n",__FUNCTION__)
                );
            nRet = m_pActivePort->signal_Port(ASPS_START_STREAM_RX,
                hStreamContext, 0
                );
            break;

        case kOutput:
            DEBUGMSG(ZONE_HWBRIDGE,
                (L"WAV:HWBridge - Starting output stream\r\n")
                );
			RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S - Starting output stream\r\n",__FUNCTION__));

            RETAILMSG(m_TransmitterState != kAudioRender_Active && ZONE_ERROR,
                (L"WAV:%S - Can't start stream when port is inactive\r\n",__FUNCTION__)
                );
            nRet = m_pActivePort->signal_Port(ASPS_START_STREAM_TX,
                hStreamContext, 0
                );
            break;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-OMAP35XX_HwAudioBridge::start_Stream\r\n")
        );

	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S 2\r\n",__FUNCTION__));

    return !!nRet;
}

//------------------------------------------------------------------------------
//  starts audio streaming
//
BOOL
OMAP35XX_HwAudioBridge::start_Stream(
    StreamType type
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+OMAP35XX_HwAudioBridge::start_Stream\r\n")
        );
	RETAILMSG(HW_BRDG_AUDIO_DBG,
        (L"WAV:+OMAP35XX_HwAudioBridge::start_Stream 1, m_TransmitterState(%d)\r\n",m_TransmitterState)
        );

    // if PowerManagement(PM) has requested for D4 state (Due to Suspend mode),
    // audio driver must not start the stream till the PM has again requested
    // for D0 State
    //
    if (m_ReqestedPowerState == D4)
        {
        DEBUGMSG(1, (L"WAV: startstream not started due to PM's D4 state\r\n"));
        goto Exit;
        }

    // check if audio stream is already turned-on.  If
    // so then don't turn-on again.  There is a edge case
    // where we will try to turn-on audio while
    // the last bit of data was pushed-out.  In this case
    // we still send the on message to the hardware
    //
    switch (type)
        {
        case kInput:
            DEBUGMSG(ZONE_HWBRIDGE,
                (L"WAV:HWBridge - Starting input stream\r\n")
                );

			RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S - Starting input stream\r\n",__FUNCTION__));

            if ((m_ReceiverState == kAudioRender_Idle) ||
                (m_ReceiverState == kAudioRender_Stopping))
                {
                switch (m_ReceiverState)
                    {
                    case kAudioRender_Idle:
                        DEBUGMSG(ZONE_HWBRIDGE,
                            (L"WAV:HWBridge - rx idle --> starting\r\n")
                            );
						RETAILMSG(HW_BRDG_AUDIO_DBG,(L"WAV:%S - rx idle --> starting\r\n",__FUNCTION__));
						RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S prev rx state = %d, current rx state = %d\r\n",__FUNCTION__, m_ReceiverState, kAudioRender_Starting));

                        m_ReceiverState = kAudioRender_Starting;
                        update_AudioRouting();
                        break;

                    case kAudioRender_Stopping:
                        DEBUGMSG(ZONE_HWBRIDGE,
                            (L"WAV:HWBridge - rx stopping --> active\r\n")
                            );
						RETAILMSG(HW_BRDG_AUDIO_DBG,(L"WAV:%S - rx stopping --> active\r\n",__FUNCTION__));
						RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S prev rx state = %d, current rx state = %d\r\n",__FUNCTION__, m_ReceiverState, kAudioRender_Active));

                        m_ReceiverState = kAudioRender_Active;
                        break;
                    }

                m_pActivePort->signal_Port(ASPS_START_RX, NULL, 0);
                }
            break;

        case kOutput:
            DEBUGMSG(ZONE_HWBRIDGE,
                (L"WAV:HWBridge - Starting output stream\r\n")
                );
			RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S - Starting output stream\r\n",__FUNCTION__));

            if ((m_TransmitterState == kAudioRender_Idle) ||
                (m_TransmitterState == kAudioRender_Stopping))
                {
                switch (m_TransmitterState)
                    {
                    case kAudioRender_Idle:
                        DEBUGMSG(ZONE_HWBRIDGE,
                            (L"WAV:HWBridge - tx idle --> starting\r\n")
                            );
						RETAILMSG(HW_BRDG_AUDIO_DBG,(L"WAV:%S - tx idle --> starting\r\n",__FUNCTION__));
						RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S prev tx state = %d, current tx state = %d\r\n",__FUNCTION__, m_TransmitterState, kAudioRender_Starting));

                        m_TransmitterState = kAudioRender_Starting;
                        update_AudioRouting();
                        break;

                    case kAudioRender_Stopping:
                        DEBUGMSG(ZONE_HWBRIDGE,
                            (L"WAV:HWBridge - tx stopping --> active\r\n")
                            );
						RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S - tx stopping --> active\r\n",__FUNCTION__));
						RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S prev tx state = %d, current tx state = %d\r\n",__FUNCTION__, m_TransmitterState, kAudioRender_Active));

                        m_TransmitterState = kAudioRender_Active;
                        break;
                    }

                m_pActivePort->signal_Port(ASPS_START_TX, NULL, 0);
                }
            break;
        }
Exit:
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-OMAP35XX_HwAudioBridge::start_Stream\r\n")
        );
	RETAILMSG(HW_BRDG_AUDIO_DBG,
        (L"WAV:-OMAP35XX_HwAudioBridge::start_Stream\r\n")
        );

    return TRUE;
}

//------------------------------------------------------------------------------
// end audio streaming
//
BOOL
OMAP35XX_HwAudioBridge::stop_Stream(
    StreamType type,
    HANDLE hStreamContext
    )
{
    int nRet = FALSE;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+OMAP35XX_HwAudioBridge::stop_Stream\r\n")
        );
   RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:+%S\r\n",__FUNCTION__));

    switch (type)
        {
        case kInput:
            DEBUGMSG(ZONE_HWBRIDGE,
                (L"WAV:HWBridge - Stopping input stream\r\n")
                );
			RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S - Stopping input stream\r\n",__FUNCTION__));

            nRet = m_pActivePort->signal_Port(ASPS_STOP_STREAM_RX,
                hStreamContext, 0
                );
            break;

        case kOutput:
            DEBUGMSG(ZONE_HWBRIDGE,
                (L"WAV:HWBridge - Stopping output stream\r\n")
                );
			RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S - Stopping output stream\r\n",__FUNCTION__));

            nRet = m_pActivePort->signal_Port(ASPS_STOP_STREAM_TX,
                hStreamContext, 0
                );
            break;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-OMAP35XX_HwAudioBridge::stop_Stream\r\n")
        );

	RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:-%S\r\n",__FUNCTION__));
    return !!nRet;
}


//------------------------------------------------------------------------------
// abort audio streaming
//
BOOL
OMAP35XX_HwAudioBridge::abort_Stream(
    StreamType type,
    HANDLE hStreamContext
    )
{
    int nRet = FALSE;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+OMAP35XX_HwAudioBridge::abort_Stream\r\n")
        );

    switch (type)
        {
        case kInput:
            DEBUGMSG(ZONE_HWBRIDGE,
                (L"WAV:HWBridge - Aborting input stream\r\n")
                );
                nRet = m_pActivePort->signal_Port(ASPS_ABORT_RX,
                hStreamContext, 0
                );
            break;

        case kOutput:
            DEBUGMSG(ZONE_HWBRIDGE,
                (L"WAV:HWBridge - Aborting output stream\r\n")
                );
                nRet = m_pActivePort->signal_Port(ASPS_ABORT_TX,
                hStreamContext, 0
                );
            break;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-OMAP35XX_HwAudioBridge::abort_Stream\r\n")
        );
    return !!nRet;
}

//------------------------------------------------------------------------------
// set audio stream gain
//
BOOL
OMAP35XX_HwAudioBridge::set_StreamGain(
    StreamType type,
    HANDLE hStreamContext,
    DWORD dwContextData
    )
{
    int nRet = FALSE;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+OMAP35XX_HwAudioBridge::set_StreamGain\r\n")
        );

	RETAILMSG(HW_BRDG_AUDIO_DBG,
        (L"WAV:+OMAP35XX_HwAudioBridge::set_StreamGain\r\n")
        );

    switch (type)
        {
        case kInput:
            DEBUGMSG(ZONE_HWBRIDGE,
                (L"WAV:HWBridge - set_StreamGain input stream\r\n")
                );
            RETAILMSG(HW_BRDG_AUDIO_DBG, 
                (L"WAV:HWBridge - set_StreamGain input stream\r\n")
                );

            nRet = m_pActivePort->signal_Port(ASPS_GAIN_STREAM_RX,
                hStreamContext, dwContextData
                );
            break;

        case kOutput:
            DEBUGMSG(ZONE_HWBRIDGE,
                (L"WAV:HWBridge - set_StreamGain output stream\r\n")
                );
			RETAILMSG(HW_BRDG_AUDIO_DBG,
                (L"WAV:HWBridge - set_StreamGain output stream\r\n")
                );

            nRet = m_pActivePort->signal_Port(ASPS_GAIN_STREAM_TX,
                hStreamContext, dwContextData
                );
            break;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-OMAP35XX_HwAudioBridge::set_StreamGain\r\n")
        );

	RETAILMSG(HW_BRDG_AUDIO_DBG,
        (L"WAV:-OMAP35XX_HwAudioBridge::set_StreamGain\r\n")
        );

    return !!nRet;
}

//------------------------------------------------------------------------------
// swicthes audio port
//
BOOL
OMAP35XX_HwAudioBridge::switch_AudioStreamPort(
    BOOL bPortRequest
    )
{
    int nRet = FALSE;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+OMAP35XX_HwAudioBridge::switch_AudioStreamPort\r\n")
        );

    // Check if audio port is idle and check the current active port and switch
    // the port accordingly.
    //
    if ((m_ReceiverState == kAudioRender_Idle) &&
        (m_TransmitterState == kAudioRender_Idle))
        {
         if (m_CurrActivePort == kAudioDASFPort)
            {
            m_bPortSwitch = bPortRequest;
            update_AudioRouting();
            if (m_bPreviousPortIsDASF == TRUE)
                {
                nRet = m_pActivePort->signal_Port(ASPS_PORT_RECONFIG, NULL, 0);
                }
            m_bPreviousPortIsDASF = FALSE;
            }
         else if (m_CurrActivePort == kAudioDMTPort)
            {
            m_bPortSwitch = bPortRequest;
            update_AudioRouting();

            m_bPreviousPortIsDASF = TRUE;
            }
         nRet = TRUE;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-OMAP35XX_HwAudioBridge::switch_AudioStreamPort\r\n")
        );
    return nRet;

}

//------------------------------------------------------------------------------
// Queries current active port
//
BOOL
OMAP35XX_HwAudioBridge:: query_AudioStreamPort()
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+/- OMAP35XX_HwAudioBridge::switch_AudioStreamPort\r\n")
        );

    return (BOOL)m_CurrActivePort;
}

//------------------------------------------------------------------------------
// Enables/Disables T2 Audio clocks
//
BOOL
OMAP35XX_HwAudioBridge:: enable_I2SClocks(BOOL bClkEnable)
{

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+OMAP35XX_HwAudioBridge::enable_I2SClocks\r\n")
        );

    if (bClkEnable == TRUE)
        {
        SetHwCodecMode_Speaker(m_hHwCodec, (AudioProfile_e)m_dwAudioProfile);
        }
    else
        {
        SetHwCodecMode_Disabled(m_hHwCodec);
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-OMAP35XX_HwAudioBridge::enable_I2SClocks\r\n")
        );

    return TRUE;
}

//------------------------------------------------------------------------------
//  Called by AudioStream port
//
BOOL
OMAP35XX_HwAudioBridge::OnAudioStreamMessage(
    AudioStreamPort *pPort,
    DWORD msg,
    void *pvData
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV::+OMAP35XX_HwAudioBridge::OnAudioStreamMessage(msg=%d)\r\n",
        msg));
	RETAILMSG(0,
        (L"WAV::+OMAP35XX_HwAudioBridge::OnAudioStreamMessage(msg=%d), m_TransmitterState(%d)\r\n",
        msg, m_TransmitterState));

	RETAILMSG(/*HW_BRDG_AUDIO_DBG*/0, (L"WAV:+%S, msg = %d, m_TransmitterState = %d\r\n",__FUNCTION__, msg, m_TransmitterState));

    BOOL bRet = TRUE;
    ASPM_STREAM_DATA* pStreamData;

    ASSERT(m_pActivePort == pPort);
    if (m_pActivePort == pPort)
        {
        EnterMutex();
        switch (msg)
            {
            case ASPM_START_TX:
                DEBUGMSG(ZONE_HWBRIDGE, (L"WAV:ASPM_START_TX\r\n"));
				//RETAILMSG(0, (L"WAV:ASPM_START_TX\r\n"));
				RETAILMSG(HW_BRDG_AUDIO_DBG,(L"WAV:%S: ASPM_START_TX\r\n",__FUNCTION__));
				RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S prev tx state = %d, current tx state = %d\r\n",__FUNCTION__, m_TransmitterState, kAudioRender_Active));

                m_TransmitterState = kAudioRender_Active;
                break;

            case ASPM_PROCESSDATA_TX:
                DEBUGMSG(ZONE_HWBRIDGE, (L"WAV:ASPM_PROCESSDATA_TX\r\n"));
				RETAILMSG(/*HW_BRDG_AUDIO_DBG*/0,(L"WAV:%S: ASPM_PROCESSDATA_TX\r\n",__FUNCTION__));

                pStreamData = (ASPM_STREAM_DATA*)pvData;
                if (pStreamData->hStreamContext == NULL)
                    {
                    bRet = m_prgStreams[kOutput]->copy_AudioData(
                        pStreamData->pBuffer, pStreamData->dwBufferSize
                        );
                    }
                else
                    {
                    // get a specific streams data
                    bRet = m_prgStreams[kOutput]->copy_StreamData(
                        pStreamData->hStreamContext, pStreamData->pBuffer,
                        pStreamData->dwBufferSize
                        );
                    }
                break;

            case ASPM_START_RX:
                DEBUGMSG(ZONE_HWBRIDGE, (L"WAV:ASPM_START_RX\r\n"));
				RETAILMSG(0, (L"WAV:ASPM_START_RX\r\n"));
				RETAILMSG(HW_BRDG_AUDIO_DBG,(L"WAV:%S: ASPM_START_RX\r\n",__FUNCTION__));
				RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S prev rx state = %d, current rx state = %d\r\n",__FUNCTION__, m_ReceiverState, kAudioRender_Active));

                m_ReceiverState = kAudioRender_Active;
                break;

            case ASPM_PROCESSDATA_RX:
                DEBUGMSG(ZONE_HWBRIDGE, (L"WAV:ASPM_PROCESSDATA_RX\r\n"));
				RETAILMSG(0/*HW_BRDG_AUDIO_DBG*/,(L"WAV:%S: ASPM_PROCESSDATA_RX\r\n",__FUNCTION__));

                pStreamData = (ASPM_STREAM_DATA*)pvData;
                if (pStreamData->hStreamContext == NULL)
                    {
                    bRet = m_prgStreams[kInput]->copy_AudioData(
                        pStreamData->pBuffer, pStreamData->dwBufferSize
                        );
                    }
                else
                    {
                    // get a specific streams data
                    bRet = m_prgStreams[kInput]->copy_StreamData(
                        pStreamData->hStreamContext, pStreamData->pBuffer,
                        pStreamData->dwBufferSize
                        );
                    }
                break;

            case ASPM_STOP_TX:
                DEBUGMSG(ZONE_HWBRIDGE, (L"WAV:ASPM_STOP_TX\r\n"));
				RETAILMSG(0, (L"WAV:ASPM_STOP_TX\r\n"));
				RETAILMSG(HW_BRDG_AUDIO_DBG,(L"WAV:%S: ASPM_STOP_TX\r\n",__FUNCTION__));
                if (m_TransmitterState == kAudioRender_Stopping)
                    {
                     RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S prev tx state = %d, current tx state = %d\r\n",__FUNCTION__, m_TransmitterState, kAudioRender_Idle));
                     m_TransmitterState = kAudioRender_Idle;
                    }

                // a stop was forced to change frequency or audio routing.
                // Cause audio rendering to start again.
                //
                update_AudioRouting();

                if (m_TransmitterState != kAudioRender_Idle)
                    {
                    m_pActivePort->signal_Port(ASPS_START_TX, NULL, 0);
                    }

                if (m_ReceiverState == kAudioRender_Starting)
                    {
                    m_pActivePort->signal_Port(ASPS_START_RX, NULL, 0);
                    }
                break;

            case ASPM_STOP_RX:
                DEBUGMSG(ZONE_HWBRIDGE, (L"WAV:ASPM_STOP_RX\r\n"));
				RETAILMSG(0, (L"WAV:ASPM_STOP_RX\r\n"));
				RETAILMSG(HW_BRDG_AUDIO_DBG,(L"WAV:%S: ASPM_STOP_RX\r\n",__FUNCTION__));

                if (m_ReceiverState != kAudioRender_Idle )
                    {
                   	 RETAILMSG(HW_BRDG_AUDIO_DBG, (L"WAV:%S prev rx state = %d, current rx state = %d\r\n",__FUNCTION__, m_ReceiverState, kAudioRender_Idle));
                     m_ReceiverState = kAudioRender_Idle;
                    }

                update_AudioRouting();
                break;
            }

        ExitMutex();
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV::-OMAP35XX_HwAudioBridge::OnAudioStreamMessage(msg=%d)\r\n",
        msg));
	RETAILMSG(0,
        (L"WAV::-OMAP35XX_HwAudioBridge::OnAudioStreamMessage(msg=%d)\r\n",
        msg));

	RETAILMSG(/*HW_BRDG_AUDIO_DBG*/0, (L"WAV:-%S, bRet = %d\r\n",__FUNCTION__, bRet));

    return bRet;
}



