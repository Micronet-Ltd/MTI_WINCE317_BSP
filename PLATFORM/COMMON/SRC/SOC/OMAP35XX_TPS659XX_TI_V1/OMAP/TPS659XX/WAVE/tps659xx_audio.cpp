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
// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
//------------------------------------------------------------------------------
//

#include <windows.h>
#include <wavext.h>
#include <memtxapi.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <tps659xx.h>
#include <initguid.h>
#include <gpio.h>
#include <twl.h>
#include <tps659xx_wave.h>
#include <tps659xx_audio.h>
#include "debug.h"
//#include <oal.h>
//#include <oalex.h>
//#include <args.h>
#include <audioControl.h>
//#include "voiceControl.h"


#undef  INTERNAL_n_EXTERNAL_SPK_TEST

//------------------------------------------------------------------------------
//  Local
// defines current triton audio mode
static AudioMode_e s_currentAudioMode = kAudioModeDisabled;

// defines current triton voice mode
static ModemVoiceMode_e    s_currentModemVoiceMode = kModemVoiceDisabled;
static ModemBTVoiceMode_e  s_currentModemBTVoiceMode = kModemBTVoiceDisabled;

// HwCodec Config info
static HwCodecConfigInfo_t *s_pHwCodecConfigInfo = 0;

// Prototype
//
static void DumpTritonAudioRegs(HANDLE hTritonDriver);

const static TCHAR *sc_MixModes[kNUM_OF_MIX_MODES] = { L"sAudioMode", L"sAudioVoiceMode", L"sVoiceMode", L"sBTVoiceMode", L"sBTPlayMode" };

const static TCHAR *sc_AudioModes[kNUM_OF_AUDIO_MODES] = 
{
	L"sAudioModeDisabled",
	L"sAudioModeSpeaker",
	L"sAudioModeMicrophone",
	L"sAudioModeStereoHeadset",
	L"sAudioModeSubMic",
	L"sAudioModeHeadset",
	L"sAudioModeBTHeadset",
	L"sAudioModeExtAudio",
	L"sAudioModeAuxHeadset",
	L"sAudioModeMicHeadset",
	L"sAudioModeClockOnly",
	L"sAudioModeHwCodecDisable",
};

// mixer gains
#define  INV_GAIN   -1000

static int s_Mic_Analog_Gain = INV_GAIN;
static int s_Mic_Digital_Gain = INV_GAIN; 
static int s_Voice_Input_Digital_Gain = INV_GAIN;
static int s_Voice_Out_Volume = INV_GAIN;
static int s_Bluetooth_In_Volume = INV_GAIN;
static int s_Bluetooth_Out_Volume = INV_GAIN;
static int s_External_Microphone_Digital_Gain = INV_GAIN;
static int s_External_Microphone_Analog_Gain = INV_GAIN;

static int s_Mic_Mute = FALSE;
static int s_Ext_Mic_Mute = FALSE;
static int s_Voice_In_Mute = FALSE;
static int s_Voice_Out_Mute = FALSE;
static int s_BT_In_Mute = FALSE;
static int s_BT_Out_Mute = FALSE;


//------------------------------------------------------------------------------
//  Functions
//

// -----------------------------------------------------------------------------
//
//  @doc    OpenHwCodecHandle
//
//  @func   HANDLE | OpenHwCodecHandle | Opens triton device handle.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
HANDLE OpenHwCodecHandle(HwCodecConfigInfo_t *pHwCodecConfigInfo)
{
    HANDLE hDevice = NULL;
	UINT8  regVal;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

    // Populate the global variable with T2 Hw codec configurations obtained
    // from registry
    //
    s_pHwCodecConfigInfo = pHwCodecConfigInfo;

    hDevice = TWLOpen();
    if(!hDevice)
    {
        RETAILMSG(hDevice == INVALID_HANDLE_VALUE, (L"WAV:!ERROR Can't load Triton Driver err=0x%08X\r\n", GetLastError()));
    }

	pHwCodecConfigInfo->bModem    = 1;
	pHwCodecConfigInfo->bBT       = 1;
	pHwCodecConfigInfo->bExtAudio = 1;
	pHwCodecConfigInfo->amplifiers_control = open_amplifiers_control();

    /* Enable the main MIC and select ADCL input */
	/* It's used for internal Mic Mute control   */
	/* Bias and ADC Control remains disabled     */

    regVal = ANAMICL_MAINMIC_EN;
    TWLWriteRegs(hDevice, TWL_ANAMICL, &regVal, sizeof(regVal) );

	TWLReadRegs(hDevice, TWL_VOICE_IF, &regVal, sizeof(regVal));
	regVal |= (VOICE_IF_VIF_DIN_EN | VOICE_IF_VIF_DOUT_EN);
	TWLWriteRegs(hDevice, TWL_VOICE_IF, &regVal, sizeof(regVal));

	TWLReadRegs(hDevice, TWL_BT_IF, &regVal, sizeof(regVal));
	regVal |= (BT_IF_BT_DIN_EN | BT_IF_BT_DOUT_EN);
	TWLWriteRegs(hDevice, TWL_BT_IF, &regVal, sizeof(regVal));

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

    return (hDevice);
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_DisableCodecPower
//
//  @func   VOID | SetHwCodecMode_DisableCodecPower | Disable codec power.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
void SetHwCodecMode_DisableCodecPower(HANDLE hTritonDriver, AudioVoiceMixModes_e mixMode)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
		UINT8 regVal;

		switch(mixMode)
		{
			case kAudioVoiceMode:
			{
				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));
				break;
			} 
			case kAudioMode:
			case kVoiceMode:
			case kBTVoiceMode:
			case kBTPlayMode:
				break;
			default:
			{
				DEBUGMSG(ZONE_FUNCTION, (L"WAV:%S - Incalid parameter\r\n",__FUNCTION__));
				break;
			}
		}
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_EnableCodecPower
//
//  @func   VOID | SetHwCodecMode_EnableCodecPower | enables codec power.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
void SetHwCodecMode_EnableCodecPower(HANDLE hTritonDriver)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
		UINT8 regVal;

        regVal = (CODEC_MODE_CODEC_PDZ);
        TWLWriteRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}


// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_GetCodecPower
//
//  @func   VOID | SetHwCodecMode_GetCodecPower | get status of the codec power.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
BOOL SetHwCodecMode_GetCodecPower(HANDLE hTritonDriver)
{
	UINT8 regVal = 0;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
		TWLReadRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(UINT8));

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

    return regVal & CODEC_MODE_CODEC_PDZ;
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_DisableCodec
//
//  @func   VOID | SetHwCodecMode_DisableCodec | Diable codec.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
static void SetHwCodecMode_DisableCodec(HANDLE hTritonDriver, AudioVoiceMixModes_e mixMode)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
		UINT8 regVal;

		switch(mixMode)
		{
			case kAudioMode: // Only audio feature disable
			{
				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_AUDIO_IF, &regVal, sizeof(regVal));
				break;
			}
			case kAudioVoiceMode: // All codec disable
			{
				regVal = APLL_INFREQ(6);
				TWLWriteRegs(hTritonDriver, TWL_APLL_CTL, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_AUDIO_IF, &regVal, sizeof(regVal));

				// Disable GSM Modem Voice PCM
				TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
				regVal &= ~VOICE_IF_VIF_EN;
				TWLWriteRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));

				break; 
			 }
			 case kVoiceMode:  // Only voice disable
             case kBTVoiceMode:
			 {
				// Disable GSM Modem Voice PCM
				TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
				regVal &= ~VOICE_IF_VIF_EN;
				TWLWriteRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));
				regVal &= ~BT_IF_BT_EN;
				TWLWriteRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));

				break;
			 }
			 case kBTPlayMode:
			 {
#if 0
				regVal = APLL_INFREQ(6);
				TWLWriteRegs(hTritonDriver, TWL_APLL_CTL, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_AUDIO_IF, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
				regVal &= ~VOICE_IF_VIF_EN;
				TWLWriteRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));
				regVal &= ~BT_IF_BT_EN;
				TWLWriteRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));
#endif // 0
				 break;
			}
			default:
			{
				DEBUGMSG(ZONE_FUNCTION, (L"WAV:%S - Invalid parameter\r\n",__FUNCTION__));
				break;
			}
		}
	}
    else
	{
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_EnableCodec
//
//  @func   VOID | SetHwCodecMode_EnableCodec | enables codec.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
static void SetHwCodecMode_EnableCodec(HANDLE hTritonDriver, UINT apllRate, AudioVoiceMixModes_e mixMode)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

    if(hTritonDriver)
	{
		UINT8 regVal;

		switch(mixMode)
		{
			case  kAudioMode: // Only audio feature enable
			{
				regVal = APLL_INFREQ(6);
				TWLWriteRegs(hTritonDriver, TWL_APLL_CTL, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));
				regVal |= (CODEC_MODE_CODEC_PDZ | CODEC_MODE_APLL_RATE(apllRate));
				TWLWriteRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));

				regVal = (APLL_EN | APLL_INFREQ(6));
				TWLWriteRegs(hTritonDriver, TWL_APLL_CTL, &regVal, sizeof(regVal));

				regVal = AUDIO_IF_AIF_EN;
				TWLWriteRegs(hTritonDriver, TWL_AUDIO_IF, &regVal, sizeof(regVal));

				break;
			}
			case  kAudioVoiceMode: // Whole codec enable: Audio & Voice
			{
				regVal = APLL_INFREQ(6);
				TWLWriteRegs(hTritonDriver, TWL_APLL_CTL, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));
				regVal |= (CODEC_MODE_CODEC_PDZ | CODEC_MODE_APLL_RATE(apllRate));
				TWLWriteRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));

				regVal = (APLL_EN | APLL_INFREQ(6));
				TWLWriteRegs(hTritonDriver, TWL_APLL_CTL, &regVal, sizeof(regVal));

				regVal = AUDIO_IF_AIF_EN;
				TWLWriteRegs(hTritonDriver, TWL_AUDIO_IF, &regVal, sizeof(regVal));

			    // Enable GSM Modem Voice PCM
				TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
				regVal |= VOICE_IF_VIF_EN;
				TWLWriteRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));

				break;
			}
			case  kVoiceMode:  // Only voice enable
			{
				regVal = APLL_INFREQ(6);
				TWLWriteRegs(hTritonDriver, TWL_APLL_CTL, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));
				regVal |= (CODEC_MODE_CODEC_PDZ | CODEC_MODE_APLL_RATE(apllRate));
				TWLWriteRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));

				regVal = (APLL_EN | APLL_INFREQ(6));
				TWLWriteRegs(hTritonDriver, TWL_APLL_CTL, &regVal, sizeof(regVal));

			    // Enable GSM Modem Voice PCM
				TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
				regVal |= VOICE_IF_VIF_EN;
				TWLWriteRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));

				break;
			}
			case  kBTVoiceMode:  // Only BT voice enable
			{
   			    regVal = APLL_INFREQ(6);
				TWLWriteRegs(hTritonDriver, TWL_APLL_CTL, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));
				regVal |= (CODEC_MODE_CODEC_PDZ | CODEC_MODE_APLL_RATE(apllRate));
				TWLWriteRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));

				regVal = (APLL_EN | APLL_INFREQ(6));
				TWLWriteRegs(hTritonDriver, TWL_APLL_CTL, &regVal, sizeof(regVal));

			    // Enable GSM Modem Voice PCM
				TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
				regVal |= VOICE_IF_VIF_EN;
				TWLWriteRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));
				regVal |= BT_IF_BT_EN;
				TWLWriteRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));

				break;
			}
			case kBTPlayMode:
			{
#if 0
				regVal = APLL_INFREQ(6);
				TWLWriteRegs(hTritonDriver, TWL_APLL_CTL, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));
				regVal |= (CODEC_MODE_CODEC_PDZ | CODEC_MODE_APLL_RATE(apllRate));
				TWLWriteRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));

				regVal = (APLL_EN | APLL_INFREQ(6));
				TWLWriteRegs(hTritonDriver, TWL_APLL_CTL, &regVal, sizeof(regVal));

				regVal = AUDIO_IF_AIF_EN;
				TWLWriteRegs(hTritonDriver, TWL_AUDIO_IF, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
				regVal |= VOICE_IF_VIF_EN;
				TWLWriteRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));
				regVal |= BT_IF_BT_EN;
				TWLWriteRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));

#endif // 0
				 break;
			}
			default:
			{
				DEBUGMSG(ZONE_FUNCTION, (L"WAV:%S - Invalid parameter\r\n",__FUNCTION__));

				break;
			}
		}
	}
	else
	{
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}


// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_DisableOutputRouting
//
//  @func   VOID | SetHwCodecMode_DisableOutputRouting | Disable output routing.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
static void SetHwCodecMode_DisableOutputRouting(HANDLE hTritonDriver, AudioVoiceMixModes_e mixMode)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
        UINT8 regVal;

		switch(mixMode)
		{
			case kAudioMode: // Only audio RX routing disable
			{
				// Voice should continue to work, so remain open DAC and Analog output

				TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
				regVal &= ~(OPTION_ARXL2_EN | OPTION_ARXR2_EN );
				TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

				regVal = 0; // using default (0) value
				TWLWriteRegs(hTritonDriver, TWL_RX_PATH_SEL, &regVal, sizeof(regVal));

				break;
			}
			case kVoiceMode:  // Only voice routing disable
			{
				// Audio should continue to work. disable Voice RX and Voice RX - Audio RX mixing
				TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
				regVal &= ~(OPTION_ARXL1_VRX_EN);
				TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

				// DIGMIXING Disbale Voice RX digital mixing
		  		regVal = DIGMIXING_ARX2_MIXING(0x0);
				TWLWriteRegs(hTritonDriver, TWL_DIGMIXING, &regVal, sizeof(regVal));

				// if Voice disabled, disable BT VOICE too
				TWLReadRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));
				regVal &= ~(MUXRX_BT);
				TWLWriteRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));

				break;
			}
			case kAudioVoiceMode: // Whole codec disable: Audio & Voice
			{
				TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
				regVal &= ~(OPTION_ARXL2_EN | OPTION_ARXR2_EN | OPTION_ARXR1_EN | OPTION_ARXL1_VRX_EN);
				TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_EAR_CTL, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_HS_SEL, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_HS_GAIN_SET, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_PREDL_CTL, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_PREDR_CTL, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_PRECKL_CTL, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_PRECKR_CTL, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_HFL_CTL, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_HFR_CTL, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_AVDAC_CTL, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_RX_PATH_SEL, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_VDL_APGA_CTL, &regVal, sizeof(regVal));

				// DIGMIXING Disbale Voice RX digital mixing
		  		regVal = DIGMIXING_ARX2_MIXING(0x0);
				TWLWriteRegs(hTritonDriver, TWL_DIGMIXING, &regVal, sizeof(regVal));

				// if Voice disabled, disable BT VOICE too
				TWLReadRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));
                regVal &= ~(MUXRX_BT);
				TWLWriteRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));

				break;
			}
			case kBTVoiceMode:  // Disable BTDOUT path
			{
				TWLReadRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));
                regVal &= ~(MUXRX_BT);
				TWLWriteRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));
				break;
			}
            case kBTPlayMode:
			{
#if 0
				TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
				regVal &= ~(OPTION_ARXL2_EN | OPTION_ARXR2_EN | OPTION_ARXR1_EN | OPTION_ARXL1_VRX_EN);
				TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_EAR_CTL, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_HS_SEL, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_HS_GAIN_SET, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_PREDL_CTL, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_PREDR_CTL, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_PRECKL_CTL, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_PRECKR_CTL, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_HFL_CTL, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_HFR_CTL, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_AVDAC_CTL, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_RX_PATH_SEL, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_VDL_APGA_CTL, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_BTSTPGA, &regVal, sizeof(regVal));
				
			    regVal = 0;
			    TWLWriteRegs(hTritonDriver, TWL_DTMF_CTL, &regVal, sizeof(regVal));

				// DIGMIXING Disbale Voice RX and TX digital mixing
		  		regVal = (DIGMIXING_ARX2_MIXING(0x0) | DIGMIXING_VTX_MIXING(0x0));
				TWLWriteRegs(hTritonDriver, TWL_DIGMIXING, &regVal, sizeof(regVal));

                regVal = 0; // using default (0) value
                TWLWriteRegs(hTritonDriver, TWL_RX_PATH_SEL, &regVal, sizeof(regVal));
#endif // 0
				 break;
			}
			default:
			{
				DEBUGMSG(ZONE_FUNCTION, (L"WAV:%S - Invalid parameter\r\n",__FUNCTION__));
				break;
			}
		}
	}
	else
	{
		DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_DisableOutputGain
//
//  @func   VOID | SetHwCodecMode_DisableOutputGain | Disable output gain.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
static void SetHwCodecMode_DisableOutputGain(HANDLE hTritonDriver, AudioVoiceMixModes_e mixMode)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

    if(hTritonDriver)
	{
		UINT8 regVal;

		switch(mixMode)
		{
			case kAudioMode: // Only audio RX digital gain disable
			{
				regVal = 0;					// Mute all RX pathes
				TWLWriteRegs(hTritonDriver, TWL_ARXL2PGA, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_ARXR2PGA, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_ARXL1PGA, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_ARXR1PGA, &regVal, sizeof(regVal));

				regVal = (UINT8) ((APGA_CTL_GAIN_SET(0x12)));
				TWLWriteRegs(hTritonDriver, TWL_ARXL1_APGA_CTL, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_ARXR1_APGA_CTL, &regVal, sizeof(regVal));
                TWLWriteRegs(hTritonDriver, TWL_ARXL2_APGA_CTL, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_ARXR2_APGA_CTL, &regVal, sizeof(regVal));

				break;
			}
			case kVoiceMode:  // Only voice RX digital gain disable
			{
				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_VRX2ARXPGA, &regVal, sizeof(regVal));
  				TWLWriteRegs(hTritonDriver, TWL_VRXPGA, &regVal, sizeof(regVal));

				// Stop the sidetone
				TWLWriteRegs(hTritonDriver, TWL_VSTPGA, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));
				regVal &= ~(BTRXPGA_GAIN(0x0F));
				TWLWriteRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));

				break;
			}
			case kAudioVoiceMode: // Whole codec disable: Audio & Voice
			{
				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_ARXL1PGA, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_ARXR1PGA, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_ARXL2PGA, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_ARXR2PGA, &regVal, sizeof(regVal));

				TWLWriteRegs(hTritonDriver, TWL_VRXPGA, &regVal, sizeof(regVal));
			    TWLWriteRegs(hTritonDriver, TWL_VRX2ARXPGA, &regVal, sizeof(regVal));
                TWLWriteRegs(hTritonDriver, TWL_VSTPGA, &regVal, sizeof(regVal));

				regVal = (UINT8) ((APGA_CTL_GAIN_SET(0x12)));
				TWLWriteRegs(hTritonDriver, TWL_ARXL1_APGA_CTL, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_ARXR1_APGA_CTL, &regVal, sizeof(regVal));
                TWLWriteRegs(hTritonDriver, TWL_ARXL2_APGA_CTL, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_ARXR2_APGA_CTL, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));
				regVal &= ~(BTRXPGA_GAIN(0x0F));
				TWLWriteRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));

				break;
			}
			case kBTVoiceMode:  // Disable BTDOUT gain, or set it to minimum (BTRXPGA)
			{
				TWLReadRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));
				regVal &= ~(BTRXPGA_GAIN(0x0F));
				TWLWriteRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));

				break;
			}
			case  kBTPlayMode: // 
			{
#if 0
				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_ARXL1PGA, &regVal, sizeof(regVal));
				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_ARXR1PGA, &regVal, sizeof(regVal));
				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_ARXL2PGA, &regVal, sizeof(regVal));
				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_ARXR2PGA, &regVal, sizeof(regVal));

				regVal = (UINT8) ((APGA_CTL_GAIN_SET(0x12)));
				TWLWriteRegs(hTritonDriver, TWL_ARXL1_APGA_CTL, &regVal, sizeof(regVal));

				regVal = (UINT8) ((APGA_CTL_GAIN_SET(0x12)));
				TWLWriteRegs(hTritonDriver, TWL_ARXR1_APGA_CTL, &regVal, sizeof(regVal));

                regVal = (UINT8) ((APGA_CTL_GAIN_SET(0x12)));
                TWLWriteRegs(hTritonDriver, TWL_ARXL2_APGA_CTL, &regVal, sizeof(regVal));

				regVal = (UINT8) ((APGA_CTL_GAIN_SET(0x12)));
				TWLWriteRegs(hTritonDriver, TWL_ARXR2_APGA_CTL, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_VRXPGA, &regVal, sizeof(regVal));
  			    regVal = 0;
			    TWLWriteRegs(hTritonDriver, TWL_VRX2ARXPGA, &regVal, sizeof(regVal));
				regVal = 0;
                TWLWriteRegs(hTritonDriver, TWL_VSTPGA, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_ARX2VTXPGA, &regVal, sizeof(regVal));
#endif // 0

				break;
			}
 			default:
			{
				DEBUGMSG(ZONE_FUNCTION, (L"WAV:%S - Invalid parameter\r\n",__FUNCTION__));

				break;
			}
		}
	}
    else
	{
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_EnableOutputRouting
//
//  @func   VOID | SetHwCodecMode_EnableOutputRouting | Enables output routing.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
static void SetHwCodecMode_EnableOutputRouting(HANDLE hTritonDriver, AudioMode_e type, AudioVoiceMixModes_e mixMode)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
        UINT8 regVal;

		switch(mixMode)
		{
			case kAudioMode:   // Only Audio 
				break;
			case kAudioVoiceMode:  // Whole Codec and set Voice routing
			case kVoiceMode:
			{
				//Enable the serial voice interface pcm_vdr to send the modem sample to the voice DAC:
				TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
				regVal |= OPTION_ARXL2_EN | OPTION_ARXR2_EN | OPTION_ARXL1_VRX_EN;
				TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

				// DIGMIXING Enable digital mixing
				regVal = DIGMIXING_ARX2_MIXING(0x03);
				TWLWriteRegs(hTritonDriver, TWL_DIGMIXING, &regVal, sizeof(regVal));

				// Update Mute state
				BOOL mute;
				GetVoiceOutMuteHwCodecModem(hTritonDriver, &mute);
				SetVoiceOutMuteHwCodecModem(hTritonDriver, mute);

				break;
			}
			case kBTPlayMode:
			{
#if 0
            //Enable the serial voice interface pcm_vdr to send the modem sample to the voice DAC:
            TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
			regVal |= (OPTION_ARXL2_EN | OPTION_ARXR2_EN) ;
            TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

            TWLReadRegs(hTritonDriver, TWL_RX_PATH_SEL, &regVal, sizeof(regVal));
            regVal |= RXL2_SEL;
            TWLWriteRegs(hTritonDriver, TWL_RX_PATH_SEL, &regVal, sizeof(regVal));

			// DIGMIXING Enable digital mixing
			regVal = DIGMIXING_VTX_MIXING(0x01);
			TWLWriteRegs(hTritonDriver, TWL_DIGMIXING, &regVal, sizeof(regVal));

			regVal = 0; // Side tone Mute //0x15; // -30db
			TWLWriteRegs(hTritonDriver, TWL_BTSTPGA, &regVal, sizeof(regVal));

			//TONE_EN
			regVal = 0; // DTMF disabled //TONE_EN;
			TWLWriteRegs(hTritonDriver, TWL_DTMF_CTL, &regVal, sizeof(regVal));
#endif // 0

				break;
			}
			case kBTVoiceMode:  // Enable BTDOUT path
			{
				TWLReadRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));
				regVal |= (MUXRX_BT);
				TWLWriteRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));

				// Update Mute state
				BOOL mute;
				GetMuteBTOutHwCodecModem(hTritonDriver, &mute);
				SetMuteBTOutHwCodecModem(hTritonDriver, mute);

				break;
			}
			default:
			{
				DEBUGMSG(ZONE_FUNCTION, (L"WAV:%S - Invalid parameter\r\n",__FUNCTION__));

				break;
			}
		}

		switch(type)
		{
			case kAudioModeSpeaker:
			{ 
				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_HS_SEL, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_HS_GAIN_SET, &regVal, sizeof(regVal));

				// Check RXL2 option
				//
                regVal = RXL1_SEL(3);
                TWLWriteRegs(hTritonDriver, TWL_RX_PATH_SEL, &regVal, sizeof(regVal));

				// Mono predrive left output
				//
                regVal = AVDAC_CTL_ADACL1_EN;
                TWLWriteRegs(hTritonDriver, TWL_AVDAC_CTL, &regVal, sizeof(regVal) );

                TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal) );
                regVal |= (OPTION_ARXL2_EN | OPTION_ARXR2_EN);
                TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

				spkr_route(s_pHwCodecConfigInfo->amplifiers_control, SPKR_PREDRIVE_CLASSD_2_INT_SPKER);

				break;
			}
            case kAudioModeStereoHeadset:
			{
				// Stereo Headset
                // Disable Hands-free
				// never using
				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_HFL_CTL, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_HFR_CTL, &regVal, sizeof(regVal));

                regVal = VMID_EN;
                TWLWriteRegs(hTritonDriver, TWL_HS_POPN_SET, &regVal, sizeof(regVal));

                regVal = (VMID_EN | RAMP_EN);
                TWLWriteRegs(hTritonDriver, TWL_HS_POPN_SET, &regVal, sizeof(regVal));

                regVal = (HSOR_AR2_EN | HSOL_AL2_EN);
                TWLWriteRegs(hTritonDriver, TWL_HS_SEL, &regVal, sizeof(regVal));

                regVal = (HSR_GAIN(2) | HSL_GAIN(2));
                TWLWriteRegs(hTritonDriver, TWL_HS_GAIN_SET, &regVal, sizeof(regVal));

                regVal = (AVDAC_CTL_ADACL2_EN | AVDAC_CTL_ADACR2_EN);
                TWLWriteRegs(hTritonDriver, TWL_AVDAC_CTL, &regVal, sizeof(regVal));

                TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
                regVal |= (OPTION_ARXL2_EN | OPTION_ARXR2_EN);
                TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

                break;
			}
            case kAudioModeHeadset:
			{
				// Mic Headset
				// never using

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_HFL_CTL, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_HFR_CTL, &regVal, sizeof(regVal));

                regVal = VMID_EN;
                TWLWriteRegs(hTritonDriver, TWL_HS_POPN_SET, &regVal, sizeof(regVal));

                regVal = (VMID_EN | RAMP_EN);
                TWLWriteRegs(hTritonDriver, TWL_HS_POPN_SET, &regVal, sizeof(regVal));

                regVal = (HSOR_AR2_EN | HSOL_AL2_EN);
                TWLWriteRegs(hTritonDriver, TWL_HS_SEL, &regVal, sizeof(regVal));

                regVal = (HSR_GAIN(2) | HSL_GAIN(2));
                TWLWriteRegs(hTritonDriver, TWL_HS_GAIN_SET, &regVal, sizeof(regVal));

                regVal = (AVDAC_CTL_ADACL2_EN | AVDAC_CTL_ADACR2_EN);
                TWLWriteRegs(hTritonDriver, TWL_AVDAC_CTL, &regVal, sizeof(regVal));

                TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
                regVal |= (OPTION_ARXL2_EN | OPTION_ARXR2_EN);
                TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

                break;
			}
            case kAudioModeExtAudio:
			{
				// Same as speaker
				//
                regVal = RXL1_SEL(3);
                TWLWriteRegs(hTritonDriver, TWL_RX_PATH_SEL, &regVal, sizeof(regVal));

                regVal = AVDAC_CTL_ADACL1_EN;
                TWLWriteRegs(hTritonDriver, TWL_AVDAC_CTL, &regVal, sizeof(regVal));

                TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
                regVal |= (OPTION_ARXL2_EN | OPTION_ARXR2_EN);
                TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

				spkr_route(s_pHwCodecConfigInfo->amplifiers_control, SPKR_PREDRIVE_CLASSD_2_EXT_SPKER);

				break;
			}
#if defined(SYSGEN_BTH_AG)
			case kAudioModeBluetoothAudio: // Output routing
			{
				// TODO: mute mic and speaker, route to TX path/Voice loop? for recording or to modem PCM for voice call
				speak_mut_all(s_pHwCodecConfigInfo->amplifiers_control); // internal amplifier off
				mic_route(s_pHwCodecConfigInfo->amplifiers_control, MIC_MAIN_2_INT_MIC);

				regVal = RXL1_SEL(3); 
				TWLWriteRegs(hTritonDriver, TWL_RX_PATH_SEL, &regVal, sizeof(regVal));

				regVal = (AVDAC_CTL_ADACL1_EN);
				TWLWriteRegs(hTritonDriver, TWL_AVDAC_CTL, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
				regVal &= ~(OPTION_ARXR1_EN | OPTION_ARXL1_VRX_EN);
				regVal |= (OPTION_ARXR2_EN | OPTION_ARXL2_EN);
				TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

				break;
			}
#endif
            case kAudioModeBTHeadset: // BT Headset
            default:
                break;
		}

		if((mixMode == kAudioVoiceMode) || (mixMode == kVoiceMode))
		{
			TWLReadRegs(hTritonDriver, TWL_AVDAC_CTL, &regVal, sizeof(regVal));
			if((regVal & (AVDAC_CTL_ADACL2_EN | AVDAC_CTL_ADACR2_EN)) != (AVDAC_CTL_ADACL2_EN | AVDAC_CTL_ADACR2_EN))
			{
			    regVal |= (AVDAC_CTL_ADACL2_EN | AVDAC_CTL_ADACR2_EN);
                TWLWriteRegs(hTritonDriver, TWL_AVDAC_CTL, &regVal, sizeof(regVal));
			}
		}
	}
    else
	{
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_EnableOutputGain
//
//  @func   VOID | SetHwCodecMode_EnableOutputGain | Enables output gain.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
static void SetHwCodecMode_EnableOutputGain(HANDLE hTritonDriver, AudioMode_e type, AudioVoiceMixModes_e mixMode)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
        UINT8 regVal;

		switch(mixMode)
		{
			case kAudioMode:
				break;
			case kAudioVoiceMode:
			case kVoiceMode:
			{
				// Enable Voice RX to audio RX PGA gain control
				if(s_Voice_Out_Volume == INV_GAIN)
					GetOutputGainHwCodecModem(hTritonDriver, (INT16 *)&s_Voice_Out_Volume);
				else
					SetOutputGainHwCodecModem(hTritonDriver, (INT16)s_Voice_Out_Volume);

				// Stop the sidetone
				regVal = 0x0; // Mute
				TWLWriteRegs(hTritonDriver, TWL_VSTPGA, &regVal, sizeof(regVal));

				break;
			}
			case kBTPlayMode:
			{
#if 0
           regVal = 0x3F;  // 0db
	       TWLWriteRegs(hTritonDriver, TWL_ARX2VTXPGA, &regVal, sizeof(regVal));
#endif  // 0
				break;
			}
			case  kBTVoiceMode:  // Enable BTDOUT gain, or set it to default value (BTRXPGA)
			{
				if(s_Bluetooth_Out_Volume == INV_GAIN)
					GetBTOutputGainHwCodecModem(hTritonDriver, (INT16 *) &s_Bluetooth_Out_Volume);
				else
					SetBTOutputGainHwCodecModem(hTritonDriver, (INT16) s_Bluetooth_Out_Volume);
			}
			default:
				DEBUGMSG(ZONE_FUNCTION, (L"WAV:%S - Invalid parameter\r\n",__FUNCTION__));
				break;
		}

		switch(type)
		{
			case kAudioModeSpeaker:
			{
				// Set Predr gain to +6dB
				TWLReadRegs(hTritonDriver, TWL_PREDL_CTL, &regVal, sizeof(regVal));
				if(regVal != (PREDL_OUTLOW_EN | PREDL_GAIN(1) | PREDL_AL1_EN))
				{
					regVal = PREDL_OUTLOW_EN | PREDL_GAIN(1) | PREDL_AL1_EN;
					TWLWriteRegs(hTritonDriver, TWL_PREDL_CTL, &regVal, sizeof(regVal));
				}

				regVal = APGA_CTL_PDZ | APGA_CTL_DA_EN | APGA_CTL_GAIN_SET((UINT8)s_pHwCodecConfigInfo->dwHwCodecOutStereoSpeakerAnalogGain);
				TWLWriteRegs(hTritonDriver, TWL_ARXL1_APGA_CTL, &regVal, sizeof(regVal) );

				regVal = (UINT8)s_pHwCodecConfigInfo->dwHwCodecOutStereoSpeakerDigitalGain;
				TWLWriteRegs(hTritonDriver, TWL_ARXL1PGA, &regVal, sizeof(regVal));

				break;
			}
			case kAudioModeStereoHeadset:
			{
				// Stereo Headset
				regVal = (UINT8)s_pHwCodecConfigInfo->dwHwCodecOutStereoHeadsetDigitalGain;
				TWLWriteRegs(hTritonDriver, TWL_ARXL2PGA, &regVal, sizeof(regVal));

				regVal = (UINT8)s_pHwCodecConfigInfo->dwHwCodecOutStereoHeadsetDigitalGain;
				TWLWriteRegs(hTritonDriver, TWL_ARXR2PGA, &regVal,sizeof(regVal));

				regVal = (APGA_CTL_PDZ | APGA_CTL_DA_EN | (UINT8)APGA_CTL_GAIN_SET(s_pHwCodecConfigInfo->dwHwCodecOutStereoHeadsetAnalogGain));
				TWLWriteRegs(hTritonDriver, TWL_ARXL2_APGA_CTL, &regVal, sizeof(regVal));

				regVal = (APGA_CTL_PDZ | APGA_CTL_DA_EN | (UINT8)APGA_CTL_GAIN_SET(s_pHwCodecConfigInfo->dwHwCodecOutStereoHeadsetAnalogGain));
				TWLWriteRegs(hTritonDriver, TWL_ARXR2_APGA_CTL, &regVal, sizeof(regVal));

				break;
			}
			case kAudioModeHeadset:
			{
				// Mic Headset
				regVal = (UINT8)s_pHwCodecConfigInfo->dwHwCodecOutHeadsetMicDigitalGain;
				TWLWriteRegs(hTritonDriver, TWL_ARXL2PGA, &regVal, sizeof(regVal));

				regVal = (UINT8)s_pHwCodecConfigInfo->dwHwCodecOutHeadsetMicDigitalGain;
				TWLWriteRegs(hTritonDriver, TWL_ARXR2PGA, &regVal, sizeof(regVal));

				regVal = (APGA_CTL_PDZ | APGA_CTL_DA_EN | (UINT8)APGA_CTL_GAIN_SET(s_pHwCodecConfigInfo->dwHwCodecOutHeadsetMicAnalogGain));
				TWLWriteRegs(hTritonDriver, TWL_ARXL2_APGA_CTL, &regVal, sizeof(regVal));

				regVal = (APGA_CTL_PDZ | APGA_CTL_DA_EN | (UINT8)APGA_CTL_GAIN_SET(s_pHwCodecConfigInfo->dwHwCodecOutHeadsetMicAnalogGain));
				TWLWriteRegs(hTritonDriver, TWL_ARXR2_APGA_CTL, &regVal, sizeof(regVal));

				break;
			}
			case kAudioModeExtAudio:
			{
				// Set Predr gain to +6dB
				TWLReadRegs(hTritonDriver, TWL_PREDL_CTL, &regVal, sizeof(regVal));
				if(regVal != (PREDL_OUTLOW_EN | PREDL_GAIN(1) | PREDL_AL1_EN))
				{
					regVal = PREDL_OUTLOW_EN | PREDL_GAIN(1) | PREDL_AL1_EN;
					TWLWriteRegs(hTritonDriver, TWL_PREDL_CTL, &regVal, sizeof(regVal));
				}

				// Set Gain - +6 dB
				regVal = APGA_CTL_PDZ | APGA_CTL_DA_EN | APGA_CTL_GAIN_SET((UINT8)s_pHwCodecConfigInfo->dwHwCodecOutHeadsetAnalogGain);
				TWLWriteRegs(hTritonDriver, TWL_ARXL1_APGA_CTL, &regVal, sizeof(regVal));

				regVal = (UINT8)s_pHwCodecConfigInfo->dwHwCodecOutHeadsetDigitalGain;
				TWLWriteRegs(hTritonDriver, TWL_ARXL1PGA, &regVal, sizeof(regVal));

				break;
			}
#if defined(SYSGEN_BTH_AG)
			case kAudioModeBluetoothAudio: // output volume
			{
				regVal = PREDL_GAIN(2) | PREDL_AL1_EN; // 0 dB
				TWLWriteRegs(hTritonDriver, TWL_PREDL_CTL, &regVal, sizeof(regVal));

				regVal = (UINT8) (APGA_CTL_PDZ | /*APGA_CTL_DA_EN |*/ APGA_CTL_GAIN_SET(0xC)); // -12 dB analog gain
				TWLWriteRegs(hTritonDriver, TWL_ARXL1_APGA_CTL, &regVal, sizeof(regVal));

				regVal = (UINT8)s_pHwCodecConfigInfo->dwHwCodec_ARXL1PGA; // digital gain (see HwCodec_ARXL1PGA in platform.reg)
				RETAILMSG(1, (L"WAV:%S - s_pHwCodecConfigInfo->dwHwCodec_ARXL1PGA = 0x%X\r\n", __FUNCTION__, regVal));
				TWLWriteRegs(hTritonDriver, TWL_ARXL1PGA, &regVal, sizeof(regVal));

				break;
			}
#endif
			default:
				break;
		}

		switch(mixMode)
		{
			case kAudioMode:   // Only Audio
			case kAudioVoiceMode: // Whole Codec and set Voice routing
				break;
			case kVoiceMode:
			{
				// only Voice.
				// Don't touch DAC and Amplifiers, just disable Audio RX gain
				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_ARXL2PGA, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_ARXR2PGA, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_ARXL1PGA, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_ARXR1PGA, &regVal, sizeof(regVal));

				break;        
			}
			default:
				break;
		}
	}
	else
    {
		DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
    }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_DisableInputRouting
//
//  @func   VOID | SetHwCodecMode_DisableInputRouting | Disables input routing.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
static void SetHwCodecMode_DisableInputRouting(HANDLE hTritonDriver, AudioVoiceMixModes_e mixMode)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	mic_route(s_pHwCodecConfigInfo->amplifiers_control, MIC_MAIN_2_INT_MIC);

	if(hTritonDriver)
	{
		UINT8 regVal;

		switch(mixMode)
		{
			case kAudioMode:  // Only Audio 
			{
				// Don't touch Microphone path, disable only Audio TX path
				TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
				regVal &= ~(OPTION_ATXR1_EN | OPTION_ATXL1_EN);
				TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

				break;   
			}
			case kAudioVoiceMode:  // Whole Codec and set Voice routing
			{
				TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
				regVal &= ~(OPTION_ATXR2_VTXR_EN | OPTION_ATXL2_VTXL_EN | OPTION_ATXR1_EN | OPTION_ATXL1_EN );
				TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_MICBIAS_CTL, &regVal, sizeof(regVal));

				// Used for Mute Control
				// reset all bits except ANAMICL_MICAMPL_EN | ANAMICL_MAINMIC_EN
				TWLReadRegs(hTritonDriver, TWL_ANAMICL, &regVal, sizeof(regVal));
				regVal &= ANAMICL_MAINMIC_EN;
				TWLWriteRegs(hTritonDriver, TWL_ANAMICL, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_ANAMICR, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_AVADC_CTL, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_ADCMICSEL, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));
				regVal &= ~(MUXTX_PCM);
				TWLWriteRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));

				//regVal = 0;
				//TWLWriteRegs(hTritonDriver, TWL_DIGMIXING, &regVal, sizeof(regVal));

				break;
			}
			case kVoiceMode:       
			{
				// only Voice. 
				// Don't touch Microphone path, disable only Voice TX path
				TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
				regVal &= ~(OPTION_ATXR2_VTXR_EN | OPTION_ATXL2_VTXL_EN );
				TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));
				regVal &= ~(MUXTX_PCM);
				TWLWriteRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));

				break;   
			}
			case kBTPlayMode:       
			{
#if 0
				TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
				regVal &= ~(OPTION_ARXL2_EN | OPTION_ARXR2_EN);
				TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

				// No BT path 
				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));
#endif
				break;   
			}
			case  kBTVoiceMode:
			{
				// Disable BTDIN path
				TWLReadRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));
				regVal &= ~(MUXTX_PCM);
				TWLWriteRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));
				break;
			}
			default:
			{
				DEBUGMSG(ZONE_FUNCTION, (L"WAV:%S - Invalid parameter\r\n",__FUNCTION__));
				break;
			}
		}
	}
	else
	{
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_DisableInputGain
//
//  @func   VOID | SetHwCodecMode_DisableInputGain | Disables input gain.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
static void SetHwCodecMode_DisableInputGain(HANDLE hTritonDriver, AudioVoiceMixModes_e mixMode)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
		UINT8 regVal;

		switch(mixMode)
		{
			case kAudioMode:  // Only Audio
			{
				// Don't touch Microphone path, disable only Audio TX gain
				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_ATXL1PGA, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_ATXR1PGA, &regVal, sizeof(regVal));

				break;
			}
			case kAudioVoiceMode:  // Whole Codec
			{
				regVal = ALC_WAIT(5); // default
				TWLWriteRegs(hTritonDriver, TWL_ALC_CTL, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_ANAMIC_GAIN, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_ATXL1PGA, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_ATXR1PGA, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_AVTXL2PGA, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_AVTXR2PGA, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));
				regVal &= ~(BTTXPGA_GAIN(0x0F));
				TWLWriteRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));

				break;
			}
			case kVoiceMode:       
			{
				// only Voice. 
				// Don't touch Microphone path, disable only Voice TX gain
				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_AVTXL2PGA, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_AVTXR2PGA, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));
				regVal &= ~(BTTXPGA_GAIN(0x0F));
				TWLWriteRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));

				break;   
			}
			case kBTPlayMode:       
			{
#if 0
				//
				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_ARX2VTXPGA, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_AVTXL2PGA, &regVal, sizeof(regVal));
				TWLWriteRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));

			   //TWL_AVTXL2PGA
			   //TWLWriteRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));
#endif
				break;   
			}
			case kBTVoiceMode:  // Disable BTDIN gain or set it to minimum
			{
				TWLReadRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));
				regVal &= ~(BTTXPGA_GAIN(0x0F));
				TWLWriteRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));
				break;
			}
			default:
			{
				DEBUGMSG(ZONE_FUNCTION, (L"WAV:%S - Invalid parameter\r\n",__FUNCTION__));
				break;
			}
		}
	}
    else
	{
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_EnableInputRouting
//
//  @func   VOID | SetHwCodecMode_EnableInputRouting | Enables input routing.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
static void SetHwCodecMode_EnableInputRouting(HANDLE hTritonDriver, AudioMode_e type, AudioVoiceMixModes_e mixMode)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

    if(hTritonDriver)
	{
		UINT8 regVal;

		switch(type)
		{
			case kAudioModeMicrophone:
			{
				// Main Mic
				/* The active channel is TxPath channel 1 - left channel */
				TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
				regVal |= OPTION_ATXL1_EN;
				TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

				/* Set the MICBIAS for analog MIC and enable it */
				regVal = MICBIAS1_EN;
				TWLWriteRegs(hTritonDriver, TWL_MICBIAS_CTL, &regVal, sizeof(regVal));

				// Used for Mute Control
				regVal = ANAMICL_MICAMPL_EN;
				if(GetMicMuteHwCodec(hTritonDriver))
					regVal &= ~ANAMICL_MAINMIC_EN; 
				else
					regVal |= ANAMICL_MAINMIC_EN;
				
				TWLWriteRegs(hTritonDriver, TWL_ANAMICL, &regVal, sizeof(regVal));


				/* Enable Left ADC */
				regVal = AVADC_CTL_ADCL_EN;
				TWLWriteRegs(hTritonDriver, TWL_AVADC_CTL, &regVal, sizeof(regVal));

				mic_route(s_pHwCodecConfigInfo->amplifiers_control, MIC_MAIN_2_INT_MIC);

				break;
			}
            case kAudioModeSubMic:
			{
				// Sub Mic
                /* The active channel is TxPath channel 1 - right channel */
                TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
                regVal |= OPTION_ATXR1_EN;
                TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

                /* Set the MICBIAS for analog MIC and enable it */
                regVal = MICBIAS2_EN;
                TWLWriteRegs(hTritonDriver, TWL_MICBIAS_CTL, &regVal, sizeof(regVal));

                /* Enable the sub MIC and select ADCL input */
                regVal = (ANAMICR_MICAMPR_EN | ANAMICR_SUBMIC_EN);
                TWLWriteRegs(hTritonDriver, TWL_ANAMICR, &regVal, sizeof(regVal));

                /* Enable Right ADC */
                regVal = AVADC_CTL_ADCR_EN;
                TWLWriteRegs(hTritonDriver, TWL_AVADC_CTL, &regVal, sizeof(regVal));

                break;
			}
            case kAudioModeMicHeadset:
			{
				// Mic Headset
                /* The active channel is TxPath channel 1 - left channel */
                TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
                regVal |= OPTION_ATXL1_EN;
                TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

                /* Set the HSMICBIAS for analog MIC and enable it */
                regVal = HSMICBIAS_EN;
                TWLWriteRegs(hTritonDriver, TWL_MICBIAS_CTL, &regVal, sizeof(regVal));

                /* Enable the HS MIC and select ADCL input */
                regVal = (ANAMICL_MICAMPL_EN | ANAMICL_HSMIC_EN);
                TWLWriteRegs(hTritonDriver, TWL_ANAMICL, &regVal, sizeof(regVal));

                /* Enable Left ADC */
                regVal = AVADC_CTL_ADCL_EN;
                TWLWriteRegs(hTritonDriver, TWL_AVADC_CTL, &regVal, sizeof(regVal));

				break;
			}
			case kAudioModeExtAudio:
			{
                // the same as kAudioModeMicrophone: // Main Mic
				// External Microphone connected the same way as internal one
                /* The active channel is TxPath channel 1 - left channel */
                TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
                regVal |= OPTION_ATXL1_EN;
                TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

                /* Set the MICBIAS for analog MIC and enable it */
                regVal = MICBIAS1_EN;
                TWLWriteRegs(hTritonDriver, TWL_MICBIAS_CTL, &regVal, sizeof(regVal));

				// Used for Mute Control
				regVal = ANAMICL_MICAMPL_EN;
				if(GetExtMicMuteHwCodec(hTritonDriver))
					regVal &= ~ANAMICL_MAINMIC_EN; 
				else
					regVal |= ANAMICL_MAINMIC_EN;
			
				TWLWriteRegs(hTritonDriver, TWL_ANAMICL, &regVal, sizeof(regVal));

				/* Enable Left ADC */
                regVal = AVADC_CTL_ADCL_EN;
                TWLWriteRegs(hTritonDriver, TWL_AVADC_CTL, &regVal, sizeof(regVal));

				mic_route(s_pHwCodecConfigInfo->amplifiers_control, MIC_MAIN_2_EXT_MIC);

				break;
			}
#if defined(SYSGEN_BTH_AG)
			case kAudioModeBluetoothAudio: // input routing
			{
	                TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
					regVal &= ~(OPTION_ATXR2_VTXR_EN | OPTION_ATXL2_VTXL_EN | OPTION_ATXL1_EN);
	                regVal |= OPTION_ATXR1_EN;
	                TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

					regVal = ANAMICR_AUXR_EN | ANAMICR_MICAMPR_EN;
					TWLWriteRegs(hTritonDriver, TWL_ANAMICR, &regVal, sizeof(regVal));

	                regVal = AVADC_CTL_ADCR_EN;
	                TWLWriteRegs(hTritonDriver, TWL_AVADC_CTL, &regVal, sizeof(regVal));
					break;
			}
#endif
            case kAudioModeAuxHeadset:
			{
				// Aux In, Headset Out
                /* The active channel is TxPath channel 1 - left and right channel */
                TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
                regVal |= OPTION_ATXL1_EN | OPTION_ATXR1_EN;
                TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

                /* Set the HSMICBIAS for analog MIC and enable it */
                //regVal = HSMICBIAS_EN;
                //TWLWriteRegs(hTritonDriver, TWL_MICBIAS_CTL, &regVal, sizeof(regVal));

                /* select the AUXL input, enable MICAMPL */
                //regVal = (ANAMICL_MICAMPL_EN | ANAMICL_HSMIC_EN);
                regVal = ANAMICL_AUXL_EN | ANAMICL_MICAMPL_EN;
                TWLWriteRegs(hTritonDriver, TWL_ANAMICL, &regVal, sizeof(regVal));

                /* select the AUXR input, enable MICAMPR */
                regVal = ANAMICR_AUXR_EN | ANAMICR_MICAMPR_EN;
                TWLWriteRegs(hTritonDriver, TWL_ANAMICR, &regVal, sizeof(regVal));
                
                /* Enable Left and Right ADC */
                regVal = AVADC_CTL_ADCL_EN | AVADC_CTL_ADCR_EN;
                TWLWriteRegs(hTritonDriver, TWL_AVADC_CTL, &regVal, sizeof(regVal));

                break;

			}
            default:
                break;
		}

		switch(mixMode)
		{
			case kAudioMode:  // Only Audio
				break;
			case kAudioVoiceMode:  // Whole Codec
			{
                // Enable Voice TX path
                TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
                regVal |= OPTION_ATXL2_VTXL_EN;
                TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

				// Update Mute state
				SetVoiceInMuteHwCodecModem(hTritonDriver, GetVoiceInMuteHwCodecModem(hTritonDriver));

				break;
			}
			case kVoiceMode:       
			{
				// only Voice. 
				// Don't touch Microphone path, but disable only Audio TX
				// And Enable Voice TX path
                TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
                regVal &= ~(OPTION_ATXR1_EN | OPTION_ATXL1_EN );
				regVal |= OPTION_ATXL2_VTXL_EN;
                TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
				// Update Mute state
				SetVoiceInMuteHwCodecModem(hTritonDriver, GetVoiceInMuteHwCodecModem(hTritonDriver));

				break;   
			}
			case  kBTVoiceMode:  // Enable BTDIN path
			{
				TWLReadRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));
                regVal |= (MUXTX_PCM);
				TWLWriteRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));

				// Update Mute state
				SetMuteBTInHwCodecModem(hTritonDriver, GetMuteBTInHwCodecModem(hTritonDriver));

				break;
			}
			case kBTPlayMode:
			{
#if 0
                TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
	            regVal |= OPTION_ATXL2_VTXL_EN;
                TWLWriteRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));

  			    // No BT path 
				regVal = 0/*MUXRX_BT*/;
				TWLWriteRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));
#endif
				break;
			}
			default:
			{
				DEBUGMSG(ZONE_FUNCTION, (L"WAV:%S - Invalid parameter\r\n",__FUNCTION__));
				break;
			}
		}
	}
    else
	{
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_EnableInputGain
//
//  @func   VOID | SetHwCodecMode_EnableInputGain | Enables input gain.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
static void SetHwCodecMode_EnableInputGain(HANDLE hTritonDriver, AudioMode_e type, AudioVoiceMixModes_e mixMode)
{
	DWORD dwStartTickCount = 0;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

    if(hTritonDriver)
	{
		UINT8 regVal;

		switch(mixMode)
		{
			case kAudioMode:
				  break;  // Only Audio
			case kAudioVoiceMode:  // Whole Codec
			case kVoiceMode:
			{
				// Enable Voice TX gain
                if(s_Voice_Input_Digital_Gain == INV_GAIN)
					GetInputGainHwCodecModem(hTritonDriver, (INT16 *) &s_Voice_Input_Digital_Gain );
				else
					SetInputGainHwCodecModem(hTritonDriver, (INT16) s_Voice_Input_Digital_Gain);

				break;
			}
			case kBTPlayMode:       
			{
#if 0
				regVal = 0; //0x0F; //+15 dB
				TWLWriteRegs(hTritonDriver, TWL_AVTXL2PGA, &regVal, sizeof(regVal));

				//BTPGA
				regVal = BTRXPGA_GAIN(0x0A); //0 dB
				TWLWriteRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));
#endif
				break;   
			}
			case kBTVoiceMode:  // Enable BTDIN gain, set it to default
			{
                if(s_Bluetooth_In_Volume == INV_GAIN)
					GetBTInputGainHwCodecModem(hTritonDriver, (INT16 *) &s_Bluetooth_In_Volume );
				else
					SetBTInputGainHwCodecModem(hTritonDriver, (INT16) s_Bluetooth_In_Volume);

				break;
			}
			default:
			{
				DEBUGMSG(ZONE_FUNCTION, (L"WAV:%S - Invalid parameter\r\n",__FUNCTION__));
				return;
			}
		}

		switch(type)
		{
            case kAudioModeMicrophone:
			{
				// Main Mic
                /* Configure gain on input */

                regVal = (UINT8)MISC_SET_SMOOTH_ANAVOL_EN;
                TWLWriteRegs(hTritonDriver, TWL_MISC_SET_1, &regVal, sizeof(regVal));
                regVal = 1;
                TWLWriteRegs(hTritonDriver, TWL_SOFTVOL_CTL, &regVal, sizeof(regVal));

				if(s_Mic_Analog_Gain == INV_GAIN)
				{
					regVal = (UINT8)ANAMIC_MICAMPL_GAIN(s_pHwCodecConfigInfo->dwHwCodecInMainMicAnalogGain);
					TWLWriteRegs(hTritonDriver, TWL_ANAMIC_GAIN, &regVal, sizeof(regVal));
					GetModemMicAnalogGainHwCodecModem(hTritonDriver, (UINT16 *) &s_Mic_Analog_Gain );
				}
				else
					SetModemMicAnalogGainHwCodecModem(hTritonDriver, (UINT16) s_Mic_Analog_Gain);

                /* Set Tx path volume control */
				if(s_Mic_Digital_Gain == INV_GAIN)
				{
					regVal = (UINT8)s_pHwCodecConfigInfo->dwHwCodecInMainMicDigitalGain;
					TWLWriteRegs(hTritonDriver, TWL_ATXL1PGA, &regVal, sizeof(regVal));
					GetModemMicDigitalGainHwCodecModem(hTritonDriver, (UINT16 *) &s_Mic_Digital_Gain );				
				}
				else
					SetModemMicDigitalGainHwCodecModem(hTritonDriver, (UINT16) s_Mic_Digital_Gain);

				// Set Automatic Gain Control
				regVal = (1 << 6) | (4 << 3) | 4;
				TWLWriteRegs(hTritonDriver, TWL_ALC_SET2, &regVal, sizeof(regVal));
				regVal = (2 << 3) | 3;
				TWLWriteRegs(hTritonDriver, TWL_ALC_SET1, &regVal, sizeof(regVal));
				regVal = MAINMIC_ALC_EN | ALC_WAIT(5);
				TWLWriteRegs(hTritonDriver, TWL_ALC_CTL, &regVal, sizeof(regVal));


				break;
			}
            case kAudioModeSubMic:
			{
				// Sub Mic
                /* Configure gain on input */
				if(s_Mic_Analog_Gain == INV_GAIN)
				{
					regVal = (UINT8)ANAMIC_MICAMPR_GAIN(s_pHwCodecConfigInfo->dwHwCodecInSubMicAnalogGain);
					TWLWriteRegs(hTritonDriver, TWL_ANAMIC_GAIN, &regVal, sizeof(regVal));
					GetModemMicAnalogGainHwCodecModem(hTritonDriver, (UINT16 *) &s_Mic_Analog_Gain );
				}
				else
					SetModemMicAnalogGainHwCodecModem(hTritonDriver, (UINT16) s_Mic_Analog_Gain);

                /* Set Tx path volume control */
                regVal = (UINT8)s_pHwCodecConfigInfo->dwHwCodecInSubMicDigitalGain;
                TWLWriteRegs(hTritonDriver, TWL_ATXR1PGA, &regVal, sizeof(regVal));

                break;
			}
            case kAudioModeMicHeadset:
			{
				// Mic Headset
                /* Configure gain on input */
				if(s_Mic_Analog_Gain == INV_GAIN)
				{
					regVal = (UINT8)ANAMIC_MICAMPL_GAIN(s_pHwCodecConfigInfo->dwHwCodecInHeadsetMicAnalogGain);
					TWLWriteRegs(hTritonDriver, TWL_ANAMIC_GAIN, &regVal, sizeof(regVal));
					GetModemMicAnalogGainHwCodecModem(hTritonDriver, (UINT16 *) &s_Mic_Analog_Gain );
				}
				else
					SetModemMicAnalogGainHwCodecModem(hTritonDriver, (UINT16) s_Mic_Analog_Gain);

                /* Set Tx path volume control */
				if(s_Mic_Digital_Gain == INV_GAIN)
				{
					regVal =(UINT8) s_pHwCodecConfigInfo->dwHwCodecInHeadsetMicDigitalGain;
					TWLWriteRegs(hTritonDriver, TWL_ATXL1PGA, &regVal, sizeof(regVal));
					GetModemMicDigitalGainHwCodecModem(hTritonDriver, (UINT16 *) &s_Mic_Digital_Gain );
				}
				else
					SetModemMicDigitalGainHwCodecModem(hTritonDriver, (UINT16) s_Mic_Digital_Gain);

                break;
			}
			case kAudioModeExtAudio:
			{
				if(s_External_Microphone_Analog_Gain == INV_GAIN)
				{
					regVal = (UINT8)ANAMIC_MICAMPL_GAIN(s_pHwCodecConfigInfo->dwHwCodecInMainMicAnalogGain);
					TWLWriteRegs(hTritonDriver, TWL_ANAMIC_GAIN, &regVal, sizeof(regVal));
					GetModemMicAnalogGainHwCodecModemExt(hTritonDriver, (UINT16 *)&s_External_Microphone_Analog_Gain );
				}
				else
					SetModemMicAnalogGainHwCodecModemExt(hTritonDriver, (UINT16) s_External_Microphone_Analog_Gain);

                /* Set Tx path volume control */
				if(s_External_Microphone_Digital_Gain == INV_GAIN)
				{
					regVal = (UINT8)s_pHwCodecConfigInfo->dwHwCodecInMainMicDigitalGain;
					TWLWriteRegs(hTritonDriver, TWL_ATXL1PGA, &regVal, sizeof(regVal));
					GetModemMicDigitalGainHwCodecModemExt(hTritonDriver, (UINT16 *)&s_External_Microphone_Digital_Gain );
				}
				else
					SetModemMicDigitalGainHwCodecModemExt(hTritonDriver, (UINT16) s_External_Microphone_Digital_Gain);

                // the same as kAudioModeMicrophone: // Main Mic
				// External Microphone connected the same way as internal one
                regVal = (UINT8)MISC_SET_SMOOTH_ANAVOL_EN;
                TWLWriteRegs(hTritonDriver, TWL_MISC_SET_1, &regVal, sizeof(regVal));
                regVal = 1;
                TWLWriteRegs(hTritonDriver, TWL_SOFTVOL_CTL, &regVal, sizeof(regVal));

				// Set Automatic Gain Control
				regVal = (1 << 6) | (4 << 3) | 4;
				TWLWriteRegs(hTritonDriver, TWL_ALC_SET2, &regVal, sizeof(regVal));
				regVal = (2 << 3) | 3;
				TWLWriteRegs(hTritonDriver, TWL_ALC_SET1, &regVal, sizeof(regVal));
				regVal = MAINMIC_ALC_EN | ALC_WAIT(5);
				TWLWriteRegs(hTritonDriver, TWL_ALC_CTL, &regVal, sizeof(regVal));

				break;
			}
#if defined(SYSGEN_BTH_AG)
			case kAudioModeBluetoothAudio: // input gain
			{
				regVal = (UINT8)ANAMIC_MICAMPR_GAIN(0x1); // 6 dB
                TWLWriteRegs(hTritonDriver, TWL_ANAMIC_GAIN, &regVal, sizeof(regVal));

                regVal = (UINT8)s_pHwCodecConfigInfo->dwHwCodec_ATXR1PGA; // digital gain (see HwCodec_ATXR1PGA in platform.reg)
                TWLWriteRegs(hTritonDriver, TWL_ATXR1PGA, &regVal, sizeof(regVal));

				break;
			}
#endif
			case kAudioModeAuxHeadset:
			{
				// Aux in, Headset out
                /* Configure gain on input */
				if(s_Mic_Analog_Gain == INV_GAIN)
				{
					regVal = (UINT8)(ANAMIC_MICAMPL_GAIN(s_pHwCodecConfigInfo->dwHwCodecInHeadsetAuxAnalogGain) | 
									 ANAMIC_MICAMPR_GAIN(s_pHwCodecConfigInfo->dwHwCodecInHeadsetAuxAnalogGain));
					TWLWriteRegs(hTritonDriver, TWL_ANAMIC_GAIN, &regVal, sizeof(regVal));

					GetModemMicAnalogGainHwCodecModem(hTritonDriver, (UINT16 *)&s_Mic_Analog_Gain);
				}
				else
				{
					SetModemMicAnalogGainHwCodecModem(hTritonDriver, (UINT16)s_Mic_Analog_Gain);
				}

                /* Set Tx Left path volume control */
				if(s_Mic_Digital_Gain == INV_GAIN)
				{
					regVal = (UINT8) s_pHwCodecConfigInfo->dwHwCodecInHeadsetAuxDigitalGain;
					TWLWriteRegs(hTritonDriver, TWL_ATXL1PGA, &regVal, sizeof(regVal));
					GetModemMicDigitalGainHwCodecModem(hTritonDriver, (UINT16 *) &s_Mic_Digital_Gain );
				}
				else
				{
					SetModemMicDigitalGainHwCodecModem(hTritonDriver, (UINT16)s_Mic_Digital_Gain);
				}

                /* Set Tx Right path volume control */
                regVal = (UINT8) s_pHwCodecConfigInfo->dwHwCodecInHeadsetAuxDigitalGain;
                TWLWriteRegs(hTritonDriver, TWL_ATXR1PGA, &regVal, sizeof(regVal));

                break;
			}
            default:
                break;
		}

        switch(mixMode)
		{
			case kAudioMode:
				break;  // Only Audio
			case kAudioVoiceMode:
				break; // Whole Codec
			case kVoiceMode:
			{
				// only Voice.
				// Don't touch Microphone path, but disable only Audio TX gain
				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_ATXL1PGA, &regVal, sizeof(regVal));

				regVal = 0;
				TWLWriteRegs(hTritonDriver, TWL_ATXR1PGA, &regVal, sizeof(regVal));

				break;
			}
		}
	}
    else
	{
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

// -----------------------------------------------------------------------------
//
//  @doc    SetAudioI2SProfile
//
//  @func   VOID | SetAudioI2SProfile | Enables I2S profile mode.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
void SetAudioI2SProfile(HANDLE hTritonDriver, AudioVoiceMixModes_e mixMode)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
    {
		UINT8 regVal;

		// Enable Option 2 Codec mode
        TWLReadRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));
        regVal &= ~(CODEC_MODE_CODEC_OPT_MODE);
        TWLWriteRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));

        switch(mixMode)
		{
			case kAudioMode:
			{
				TWLReadRegs(hTritonDriver, TWL_AUDIO_IF, &regVal, sizeof(regVal));
				regVal &= ~( AUDIO_IF_AIF_FORMAT(3) | AUDIO_IF_DATA_WIDTH(0));
				TWLWriteRegs(hTritonDriver, TWL_AUDIO_IF, &regVal, sizeof(regVal));

				break;  // Only Audio
			}
			case kAudioVoiceMode:  // Whole Codec
			{
				TWLReadRegs(hTritonDriver, TWL_AUDIO_IF, &regVal, sizeof(regVal));
				regVal &= ~( AUDIO_IF_AIF_FORMAT(3) | AUDIO_IF_DATA_WIDTH(0));
				TWLWriteRegs(hTritonDriver, TWL_AUDIO_IF, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
				regVal |= (VOICE_IF_VIF_EN | VOICE_IF_VIF_DIN_EN | VOICE_IF_VIF_DOUT_EN);
				TWLWriteRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
				break;
			}
			case kVoiceMode: 
			{
				// only Voice. 
				TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
				regVal |= (VOICE_IF_VIF_EN | VOICE_IF_VIF_DIN_EN | VOICE_IF_VIF_DOUT_EN);
				TWLWriteRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));

				break;   
			}
			case kBTVoiceMode:
			{
				TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
				regVal |= (VOICE_IF_VIF_EN | VOICE_IF_VIF_DIN_EN | VOICE_IF_VIF_DOUT_EN);
				TWLWriteRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));
				regVal |= (BT_IF_BT_EN | BT_IF_BT_DIN_EN | BT_IF_BT_DOUT_EN);
				TWLWriteRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));

				break;   
			}
			case kBTPlayMode:
			{
#if 0
				TWLReadRegs(hTritonDriver, TWL_AUDIO_IF, &regVal, sizeof(regVal));
				regVal &= ~( AUDIO_IF_AIF_FORMAT(3) | AUDIO_IF_DATA_WIDTH(0));
				TWLWriteRegs(hTritonDriver, TWL_AUDIO_IF, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
				regVal |= VOICE_IF_VIF_EN;
				TWLWriteRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));

				TWLReadRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));
				regVal |= BT_IF_BT_EN;
				TWLWriteRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));
#endif
				break;
			}
			default:
			{
				DEBUGMSG(1, (L"WAV:%S - Invalid parameter\r\n",__FUNCTION__));
				return;
			}
		}

        // Side tone gain
        //
        regVal = 0x0;      // Mute
        TWLWriteRegs(hTritonDriver, TWL_VSTPGA, &regVal, sizeof(regVal));
    }
    else
    {
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
    }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

// -----------------------------------------------------------------------------
//
//  @doc    SetAudioTDMProfile
//
//  @func   VOID | SetAudioTDMProfile | Enables TDM profile mode.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
void SetAudioTDMProfile(HANDLE hTritonDriver)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

    if(hTritonDriver)
    {
        UINT8 regVal;

        TWLReadRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));
        regVal |= CODEC_MODE_CODEC_OPT_MODE;
        TWLWriteRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));

        TWLReadRegs(hTritonDriver, TWL_AUDIO_IF, &regVal, sizeof(regVal));
        regVal |= (AUDIO_IF_DATA_WIDTH(0) | AUDIO_IF_AIF_FORMAT(3));
        TWLWriteRegs(hTritonDriver, TWL_AUDIO_IF, &regVal, sizeof(regVal));

        // Side tone gain
        //
        regVal = 0x29;      // -10db
        TWLWriteRegs(hTritonDriver, TWL_VSTPGA, &regVal, sizeof(regVal));

    }
    else
    {
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
    }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}


// -----------------------------------------------------------------------------
//
//  @doc    SetAudioPCMProfile
//
//  @func   VOID | SetAudioPCMProfile | Enables PCM profile mode.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
void SetAudioPCMProfile(HANDLE hTritonDriver)
{
	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

    if(hTritonDriver)
    {
        UINT8 regVal;

		// Codec Voice/Audio Option 2
        TWLReadRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));
		regVal &= ~(CODEC_MODE_CODEC_OPT_MODE);
        TWLWriteRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));

		//Voice PCM Mode 2. See Modem datasheet
		//Voice PCM mode 2. (VOICE_IF[3] VIF_FORMAT = 0) one analog microphone (VOICE_IF[1]
        //VIF_SUB_EN = 0) (see Figure 14-19)
        TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
		regVal |= (VOICE_IF_VIF_DIN_EN | VOICE_IF_VIF_DOUT_EN | VOICE_IF_VIF_EN);
        TWLWriteRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));

        // Side tone gain
        //
        //regVal = 0x29;      // -10db
        //TWLWriteRegs(hTritonDriver, TWL_VSTPGA, &regVal, sizeof(regVal));
    }
    else
    {
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
    }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}


// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_Disabled
//
//  @func   VOID | SetHwCodecMode_Disabled | Disables T2 audio.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
void SetHwCodecMode_Disabled(HANDLE hTritonDriver)
{
	AudioVoiceMixModes_e AudioVoiceMixMode;
	BOOL                 codec2Disable = 1;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
		speak_mut_all(s_pHwCodecConfigInfo->amplifiers_control); // internal amplifier off

#if defined(SYSGEN_BTH_AG)
		 if(s_currentAudioMode == kAudioModeBluetoothAudio)
		 {
	 		 GetBthAudioPlayVolHwCodec(hTritonDriver, (LONG *)&s_pHwCodecConfigInfo->dwHwCodecOutBthAudioDigitalGain);
			 GetBthAudioRecGainHwCodec(hTritonDriver, (LONG *)&s_pHwCodecConfigInfo->dwHwCodecInBthAudioDigitalGain);
		 }
#endif

         if(s_currentAudioMode == kAudioModeDisabled)
		 {
			if(s_currentModemVoiceMode == kModemVoiceDisabled)
				AudioVoiceMixMode = kAudioVoiceMode; // mean whole Codec already disabled 
			else
				AudioVoiceMixMode = kAudioMode; // disable Audio only
		 }
		 else
		 {
			if(s_currentModemVoiceMode == kModemVoiceDisabled)
				AudioVoiceMixMode = kAudioVoiceMode;
			else
				AudioVoiceMixMode = kAudioMode; // disable Audio only
		 }

         SetHwCodecMode_DisableOutputRouting(hTritonDriver, AudioVoiceMixMode);
         SetHwCodecMode_DisableOutputGain(hTritonDriver, AudioVoiceMixMode);
         SetHwCodecMode_DisableInputRouting(hTritonDriver, AudioVoiceMixMode);
         SetHwCodecMode_DisableInputGain(hTritonDriver, AudioVoiceMixMode);
         SetHwCodecMode_DisableCodec(hTritonDriver, AudioVoiceMixMode);
         SetHwCodecMode_DisableCodecPower(hTritonDriver, AudioVoiceMixMode);

			s_currentAudioMode = kAudioModeDisabled;
	}
    else
	{
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_EnableT2AudioClkOnly
//
//  @func   VOID | SetHwCodecMode_EnableT2AudioClkOnly | Enable T2 audio clock
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
void SetHwCodecMode_EnableT2AudioClkOnly(HANDLE hTritonDriver)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
    {
		if(s_currentAudioMode == kAudioModeClockOnly)
			return;

        if(s_currentAudioMode != kAudioModeDisabled)
			SetHwCodecMode_Disabled(hTritonDriver);

        if(s_currentAudioMode == kAudioModeDisabled)
		{
			UINT8 regVal;

            // Power on codec
            regVal = (CODEC_MODE_CODEC_PDZ);
            TWLWriteRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));

            regVal = APLL_INFREQ(6);
            TWLWriteRegs(hTritonDriver, TWL_APLL_CTL, &regVal, sizeof(regVal));

            // Power on codec; Fs = 44.1 KHz, o/p 256 * fs KHz
            TWLReadRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));
            regVal |= (CODEC_MODE_CODEC_PDZ | CODEC_MODE_APLL_RATE(kSampleRate_44_1KHz));
            TWLWriteRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));

            // Enalbe APLL; Input clock frq - 26Mhz and APLL is enabled
            regVal = (APLL_EN | APLL_INFREQ(6));
            TWLWriteRegs(hTritonDriver, TWL_APLL_CTL, &regVal, sizeof(regVal));

            // Enable CLK256FS
            regVal = AUDIO_IF_AIF_SLAVE_EN | AUDIO_IF_AIF_TRI_EN | AUDIO_IF_CLK256FS_EN | AUDIO_IF_AIF_EN;
            TWLWriteRegs(hTritonDriver, TWL_AUDIO_IF, &regVal, sizeof(regVal));
		}

        s_currentAudioMode = kAudioModeClockOnly;
	}
    else
	{
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_Microphone
//
//  @func   VOID | SetHwCodecMode_Microphone | Set Microphone path.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
void SetHwCodecMode_Microphone(HANDLE hTritonDriver, AudioProfile_e audioProfile)
{
	AudioVoiceMixModes_e mixMode = kAudioMode;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

    if(s_currentAudioMode == kAudioModeMicrophone)
		return;

	if(hTritonDriver)
	{
		if(s_currentAudioMode == kAudioModeClockOnly)
			SetHwCodecMode_Disabled(hTritonDriver);

		speak_mut_all(s_pHwCodecConfigInfo->amplifiers_control); // internal amplifier off

        if(s_currentModemVoiceMode == kModemVoiceEnabled)
            mixMode = kAudioVoiceMode;

        if(s_currentAudioMode == kAudioModeDisabled)
		{
            // init
            SetHwCodecMode_EnableCodecPower(hTritonDriver);
            SetHwCodecMode_EnableCodec(hTritonDriver, kSampleRate_44_1KHz, mixMode);

            if(audioProfile == kAudioI2SProfile)
				SetAudioI2SProfile(hTritonDriver, mixMode);
            else if(audioProfile == kAudioTDMProfile)
				SetAudioTDMProfile(hTritonDriver);
            else
                SetAudioI2SProfile(hTritonDriver, mixMode);
		}
        else
		{
            // disable output if transitioning directly from different output
            SetHwCodecMode_DisableOutputRouting(hTritonDriver, mixMode);
            SetHwCodecMode_DisableOutputGain(hTritonDriver, mixMode);

            // disable input if transitioning directly from different input
            SetHwCodecMode_DisableInputRouting(hTritonDriver, mixMode);
            SetHwCodecMode_DisableInputGain(hTritonDriver, mixMode);
		}

        // enable output
        SetHwCodecMode_EnableOutputGain(hTritonDriver, kAudioModeSpeaker, mixMode);
        SetHwCodecMode_EnableOutputRouting(hTritonDriver, kAudioModeSpeaker, mixMode);

        // enable input
        SetHwCodecMode_EnableInputGain(hTritonDriver, kAudioModeMicrophone, mixMode);
        SetHwCodecMode_EnableInputRouting(hTritonDriver, kAudioModeMicrophone, mixMode);

        s_currentAudioMode = kAudioModeMicrophone;
	}
    else
	{
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_Speaker
//
//  @func   VOID | SetHwCodecMode_Speaker | set speaker path.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
void SetHwCodecMode_Speaker(HANDLE hTritonDriver, AudioProfile_e audioProfile)
{
	AudioVoiceMixModes_e   mixMode = kAudioMode;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

    if((s_currentAudioMode == kAudioModeSpeaker) || (s_currentAudioMode == kAudioModeMicrophone))
		return;

	if(hTritonDriver)
	{
		if(s_currentAudioMode == kAudioModeClockOnly)
			SetHwCodecMode_Disabled(hTritonDriver);

		speak_mut_all(s_pHwCodecConfigInfo->amplifiers_control); // internal amplifier off

		if(s_currentModemVoiceMode == kModemVoiceEnabled)
            mixMode = kAudioVoiceMode;

        if(s_currentAudioMode == kAudioModeDisabled)
		{
            // init
             SetHwCodecMode_EnableCodecPower(hTritonDriver);
             SetHwCodecMode_EnableCodec(hTritonDriver, kSampleRate_44_1KHz, mixMode);

             if(audioProfile == kAudioI2SProfile)
                SetAudioI2SProfile(hTritonDriver, mixMode);
             else if(audioProfile == kAudioTDMProfile)
				SetAudioTDMProfile(hTritonDriver);
             else
                SetAudioI2SProfile(hTritonDriver, mixMode);
		}
        else
		{
            // disable output if transitioning directly from different output
            SetHwCodecMode_DisableOutputRouting(hTritonDriver, mixMode);
            SetHwCodecMode_DisableOutputGain(hTritonDriver, mixMode);
			SetHwCodecMode_DisableInputRouting(hTritonDriver, mixMode);
            SetHwCodecMode_DisableInputGain(hTritonDriver, mixMode);
		}
		SetHwCodecMode_EnableOutputGain(hTritonDriver, kAudioModeSpeaker, mixMode);
        SetHwCodecMode_EnableOutputRouting(hTritonDriver, kAudioModeSpeaker, mixMode);

		if(mixMode == kAudioVoiceMode) // Enable Mic too
		{
			SetHwCodecMode_EnableInputGain(hTritonDriver, kAudioModeMicrophone, mixMode);
			SetHwCodecMode_EnableInputRouting(hTritonDriver, kAudioModeMicrophone, mixMode);
		}

        s_currentAudioMode = kAudioModeSpeaker;
	}
    else
	{
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_MicHeadset
//
//  @func   VOID | SetHwCodecMode_MicHeadset | sets headset microphone path.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
void SetHwCodecMode_MicHeadset(HANDLE hTritonDriver, AudioProfile_e audioProfile)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(s_currentAudioMode == kAudioModeMicHeadset)
		return;

	if(hTritonDriver)
	{
		if(s_currentAudioMode == kAudioModeClockOnly)
			SetHwCodecMode_Disabled(hTritonDriver);

		speak_mut_all(s_pHwCodecConfigInfo->amplifiers_control); // internal amplifier off

        if(s_currentAudioMode == kAudioModeDisabled)
        {
			// init
            SetHwCodecMode_EnableCodecPower(hTritonDriver);
            SetHwCodecMode_EnableCodec(hTritonDriver, kSampleRate_44_1KHz, kAudioMode);

            if(audioProfile == kAudioI2SProfile)
				SetAudioI2SProfile(hTritonDriver, kAudioMode);
            else if(audioProfile == kAudioTDMProfile)
				SetAudioTDMProfile(hTritonDriver);
            else
                SetAudioI2SProfile(hTritonDriver, kAudioMode);
		}
        else
		{
            // disable output if transitioning directly from different output
            SetHwCodecMode_DisableOutputRouting(hTritonDriver, kAudioMode);
            SetHwCodecMode_DisableOutputGain(hTritonDriver, kAudioMode);

            // disable input if transitioning directly from different input
            SetHwCodecMode_DisableInputRouting(hTritonDriver, kAudioMode);
            SetHwCodecMode_DisableInputGain(hTritonDriver, kAudioMode);
        }

        // enable output
        SetHwCodecMode_EnableOutputGain(hTritonDriver, kAudioModeHeadset, kAudioMode);
        SetHwCodecMode_EnableOutputRouting(hTritonDriver, kAudioModeHeadset, kAudioMode);

        // enable input
        SetHwCodecMode_EnableInputGain(hTritonDriver, kAudioModeMicHeadset, kAudioMode);
        SetHwCodecMode_EnableInputRouting(hTritonDriver, kAudioModeMicHeadset, kAudioMode);

        s_currentAudioMode = kAudioModeMicHeadset;
	}
    else
	{
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_Headset
//
//  @func   VOID | SetHwCodecMode_Headset | sets headset path.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
void SetHwCodecMode_Headset(HANDLE hTritonDriver, AudioProfile_e audioProfile)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if((s_currentAudioMode == kAudioModeHeadset) || (s_currentAudioMode == kAudioModeMicHeadset))
		return;

	if(hTritonDriver)
	{
		if(s_currentAudioMode == kAudioModeClockOnly)
			SetHwCodecMode_Disabled(hTritonDriver);

		speak_mut_all(s_pHwCodecConfigInfo->amplifiers_control); // internal amplifier off

        if(s_currentAudioMode == kAudioModeDisabled)
        {
			// init
            SetHwCodecMode_EnableCodecPower(hTritonDriver);
            SetHwCodecMode_EnableCodec(hTritonDriver, kSampleRate_44_1KHz, kAudioMode);

            if(audioProfile == kAudioI2SProfile)
                SetAudioI2SProfile(hTritonDriver, kAudioMode);
            else if(audioProfile == kAudioTDMProfile)
                SetAudioTDMProfile(hTritonDriver);
            else
                SetAudioI2SProfile(hTritonDriver, kAudioMode);
		}
        else
		{
            // disable output if transitioning directly from different output
            SetHwCodecMode_DisableOutputRouting(hTritonDriver, kAudioMode);
            SetHwCodecMode_DisableOutputGain(hTritonDriver, kAudioMode);
        }

        // enable output
        SetHwCodecMode_EnableOutputGain(hTritonDriver, kAudioModeHeadset, kAudioMode);
        SetHwCodecMode_EnableOutputRouting(hTritonDriver, kAudioModeHeadset, kAudioMode);

        s_currentAudioMode = kAudioModeHeadset;
	}
    else
	{
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}


// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_BTHeadset
//
//  @func   VOID | SetHwCodecMode_BTHeadset | sets BT Play path.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
void SetHwCodecMode_BTHeadset(HANDLE hTritonDriver, AudioProfile_e audioProfile)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

    if(s_currentAudioMode == kAudioModeBTHeadset)
		return;

	if(hTritonDriver)
    {
		if(s_currentAudioMode == kAudioModeClockOnly)
			SetHwCodecMode_Disabled(hTritonDriver);

		speak_mut_all(s_pHwCodecConfigInfo->amplifiers_control); // internal amplifier off

        if(s_currentAudioMode == kAudioModeDisabled)
		{
			// init
            SetHwCodecMode_EnableCodecPower(hTritonDriver);
            //SetHwCodecMode_EnableCodec(hTritonDriver, kSampleRate_44_1KHz, kAudioMode);
			SetHwCodecMode_EnableCodec(hTritonDriver, kSampleRate_8KHz, kBTPlayMode);

            if(audioProfile == kAudioI2SProfile)
				SetAudioI2SProfile(hTritonDriver, kBTPlayMode);
            else if(audioProfile == kAudioTDMProfile)
                SetAudioTDMProfile(hTritonDriver);
            else
                SetAudioI2SProfile(hTritonDriver, kBTPlayMode);
		}
        else
		{
             // disable output if transitioning directly from different output
             SetHwCodecMode_DisableOutputRouting(hTritonDriver, kBTPlayMode);
             SetHwCodecMode_DisableOutputGain(hTritonDriver, kBTPlayMode);

             SetHwCodecMode_DisableInputRouting(hTritonDriver, kBTPlayMode);
             SetHwCodecMode_DisableInputGain(hTritonDriver, kBTPlayMode);
        }

        // enable output
        SetHwCodecMode_EnableOutputGain(hTritonDriver, kAudioModeBTHeadset, kBTPlayMode);
        SetHwCodecMode_EnableOutputRouting(hTritonDriver, kAudioModeBTHeadset, kBTPlayMode);

        // enable input
        SetHwCodecMode_EnableInputGain(hTritonDriver, kAudioModeBTHeadset, kBTPlayMode);
        SetHwCodecMode_EnableInputRouting(hTritonDriver, kAudioModeBTHeadset, kBTPlayMode);

        s_currentAudioMode = kAudioModeBTHeadset;
	}
    else
	{
         DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

#if defined(SYSGEN_BTH_AG)
void SetHwCodecMode_BthAudio(HANDLE hTritonDriver, AudioProfile_e audioProfile)
{
	AudioVoiceMixModes_e mixMode = kAudioMode;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(!s_pHwCodecConfigInfo->bBthAudio)
		return;

	if(s_currentAudioMode == kAudioModeBluetoothAudio)
		return;

	if(hTritonDriver)
    {
		if(s_currentAudioMode == kAudioModeClockOnly)
			SetHwCodecMode_Disabled(hTritonDriver);

        if(s_currentModemVoiceMode == kModemVoiceEnabled)
			mixMode = kAudioVoiceMode;

        if(s_currentAudioMode == kAudioModeDisabled)
        {
            // init
            SetHwCodecMode_EnableCodecPower(hTritonDriver);
            SetHwCodecMode_EnableCodec(hTritonDriver, kSampleRate_44_1KHz, mixMode);

            if (audioProfile == kAudioI2SProfile)
                SetAudioI2SProfile(hTritonDriver, mixMode);
            else if (audioProfile == kAudioTDMProfile)
                SetAudioTDMProfile(hTritonDriver);
            else
                SetAudioI2SProfile(hTritonDriver, mixMode);
        }
        else
        {
             // disable output if transitioning directly from different output
             SetHwCodecMode_DisableOutputRouting(hTritonDriver, mixMode);
             SetHwCodecMode_DisableOutputGain(hTritonDriver, mixMode);

             SetHwCodecMode_DisableInputRouting(hTritonDriver, mixMode);
             SetHwCodecMode_DisableInputGain(hTritonDriver, mixMode);
        }

        SetHwCodecMode_EnableOutputGain(hTritonDriver, kAudioModeBluetoothAudio, mixMode);
        SetHwCodecMode_EnableOutputRouting(hTritonDriver, kAudioModeBluetoothAudio, mixMode);

        SetHwCodecMode_EnableInputGain(hTritonDriver, kAudioModeBluetoothAudio, mixMode);
        SetHwCodecMode_EnableInputRouting(hTritonDriver, kAudioModeBluetoothAudio, mixMode);

		UINT8 regVal;
		TWLReadRegs(hTritonDriver, TWL_ARXL1_APGA_CTL, &regVal, sizeof(regVal));
		regVal |= APGA_CTL_DA_EN;
		TWLWriteRegs(hTritonDriver, TWL_ARXL1_APGA_CTL, &regVal, sizeof(regVal));

        s_currentAudioMode = kAudioModeBluetoothAudio;
    }
    else
    {
         DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
    }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}
#endif

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_BTHeadset
//
//  @func   VOID | SetHwCodecMode_BTHeadset | sets BT Play path.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
void SetHwCodecMode_ExtAudio(HANDLE hTritonDriver, AudioProfile_e audioProfile)
{
	AudioVoiceMixModes_e mixMode = kAudioMode;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(s_currentAudioMode == kAudioModeExtAudio)
       return;

	if(s_currentModemBTVoiceMode == kModemBTVoiceEnabled )
       return;

    if(hTritonDriver)
	{
        if(s_currentAudioMode == kAudioModeClockOnly)
			SetHwCodecMode_Disabled(hTritonDriver);

        if(s_currentModemVoiceMode == kModemVoiceEnabled)
            mixMode = kAudioVoiceMode;

        if(s_currentAudioMode == kAudioModeDisabled)
		{
            // init
            SetHwCodecMode_EnableCodecPower(hTritonDriver);
            SetHwCodecMode_EnableCodec(hTritonDriver, kSampleRate_44_1KHz, mixMode);

            if(audioProfile == kAudioI2SProfile)
                SetAudioI2SProfile(hTritonDriver, mixMode);
            else if(audioProfile == kAudioTDMProfile)
                SetAudioTDMProfile(hTritonDriver);
            else
                SetAudioI2SProfile(hTritonDriver, mixMode);
		}
        else
		{
             // disable output if transitioning directly from different output
             SetHwCodecMode_DisableOutputRouting(hTritonDriver, mixMode);
             SetHwCodecMode_DisableOutputGain(hTritonDriver, mixMode);

             SetHwCodecMode_DisableInputRouting(hTritonDriver, mixMode);
             SetHwCodecMode_DisableInputGain(hTritonDriver, mixMode);
        }

        // enable output
        SetHwCodecMode_EnableOutputGain(hTritonDriver, kAudioModeExtAudio, mixMode);
        SetHwCodecMode_EnableOutputRouting(hTritonDriver, kAudioModeExtAudio, mixMode);

        // enable input
        SetHwCodecMode_EnableInputGain(hTritonDriver, kAudioModeExtAudio, mixMode);
        SetHwCodecMode_EnableInputRouting(hTritonDriver, kAudioModeExtAudio, mixMode);

        s_currentAudioMode = kAudioModeExtAudio;
	}
    else
	{
         DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_AuxHeadset
//
//  @func   VOID | SetHwCodecMode_AuxHeadset | sets AUX in path, stereo headset out path.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
void SetHwCodecMode_AuxHeadset(HANDLE hTritonDriver, AudioProfile_e audioProfile)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n", __FUNCTION__));

    if(s_currentAudioMode == kAudioModeAuxHeadset)
		return;

    if(hTritonDriver)
	{
        if(s_currentAudioMode == kAudioModeClockOnly)
            SetHwCodecMode_Disabled(hTritonDriver);

        if(s_currentAudioMode == kAudioModeDisabled)
		{
            // init
            SetHwCodecMode_EnableCodecPower(hTritonDriver);
            SetHwCodecMode_EnableCodec(hTritonDriver, kSampleRate_44_1KHz, kAudioMode);

            if(audioProfile == kAudioI2SProfile)
                SetAudioI2SProfile(hTritonDriver, kAudioMode);
            else if(audioProfile == kAudioTDMProfile)
                SetAudioTDMProfile(hTritonDriver);
            else
                SetAudioI2SProfile(hTritonDriver, kAudioMode);
		}
        else
		{
            // disable output if transitioning directly from different output
            SetHwCodecMode_DisableOutputRouting(hTritonDriver, kAudioMode);
            SetHwCodecMode_DisableOutputGain(hTritonDriver, kAudioMode);

            // disable input if transitioning directly from different input
            SetHwCodecMode_DisableInputRouting(hTritonDriver, kAudioMode);
            SetHwCodecMode_DisableInputGain(hTritonDriver, kAudioMode);
        }

        // enable output
        SetHwCodecMode_EnableOutputGain(hTritonDriver, kAudioModeStereoHeadset, kAudioMode);
        SetHwCodecMode_EnableOutputRouting(hTritonDriver, kAudioModeStereoHeadset, kAudioMode);

        // enable input
        SetHwCodecMode_EnableInputGain(hTritonDriver, kAudioModeAuxHeadset, kAudioMode);
        SetHwCodecMode_EnableInputRouting(hTritonDriver, kAudioModeAuxHeadset, kAudioMode);

        s_currentAudioMode = kAudioModeAuxHeadset;
	}
    else
	{
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecMode_StereoHeadset
//
//  @func   VOID | SetHwCodecMode_StereoHeadset | sets stereo headset path.
//          Note: Same levels are used for headset as for AuxHeadset mode.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
void SetHwCodecMode_StereoHeadset(HANDLE hTritonDriver, AudioProfile_e audioProfile)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

    if(s_currentAudioMode == kAudioModeStereoHeadset)
		return;

    if(hTritonDriver)
	{
        if(s_currentAudioMode == kAudioModeClockOnly)
            SetHwCodecMode_Disabled(hTritonDriver);

        if(s_currentAudioMode == kAudioModeDisabled)
        {
            // init
            SetHwCodecMode_EnableCodecPower(hTritonDriver);
            SetHwCodecMode_EnableCodec(hTritonDriver, kSampleRate_44_1KHz, kAudioMode);

            if(audioProfile == kAudioI2SProfile)
                SetAudioI2SProfile(hTritonDriver, kAudioMode);
            else if(audioProfile == kAudioTDMProfile)
                SetAudioTDMProfile(hTritonDriver);
            else
                SetAudioI2SProfile(hTritonDriver, kAudioMode);
		}
        else
		{
            // disable output if transitioning directly from different output
            SetHwCodecMode_DisableOutputRouting(hTritonDriver, kAudioMode);
            SetHwCodecMode_DisableOutputGain(hTritonDriver, kAudioMode);

            // disable input if transitioning directly from different input
            SetHwCodecMode_DisableInputRouting(hTritonDriver, kAudioMode);
            SetHwCodecMode_DisableInputGain(hTritonDriver, kAudioMode);
        }

        // enable output
        SetHwCodecMode_EnableOutputGain(hTritonDriver, kAudioModeStereoHeadset, kAudioMode);
        SetHwCodecMode_EnableOutputRouting(hTritonDriver, kAudioModeStereoHeadset, kAudioMode);

        s_currentAudioMode = kAudioModeStereoHeadset;
	}
    else
	{
        DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n", __FUNCTION__));
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
}

DWORD SetAnalogGainHwCodec(HANDLE hTritonDriver, INT16 aGain)
{
	UINT8 regVal;

	if(hTritonDriver)
	{
		TWLReadRegs(hTritonDriver, TWL_ARXL1_APGA_CTL, &regVal, sizeof(regVal));

		// Convert aGain to register value
		regVal &= ~(APGA_CTL_GAIN_SET(0x1F));
		regVal |= (UINT8)APGA_CTL_GAIN_SET(6 - (aGain/2)); 
		TWLWriteRegs(hTritonDriver, TWL_ARXL1_APGA_CTL, &regVal, sizeof(regVal));

		return MMSYSERR_NOERROR;
	}

	return MMSYSERR_ERROR;
}

DWORD GetAnalogGainHwCodec(HANDLE hTritonDriver, INT16 *pAGain )
{
	UINT8 regVal;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
		TWLReadRegs(hTritonDriver, TWL_ARXL1_APGA_CTL, &regVal, sizeof(regVal));

		regVal = regVal >> 3;
		*pAGain = 2 * ((INT16)(6 - regVal));

		return MMSYSERR_NOERROR;
	}

	return MMSYSERR_ERROR;
}


DWORD StartHwCodecModem(HANDLE hTritonDriver, AudioProfile_e audioProfile, AudioMode_e inAudioMode, AudioMode_e outAudioMode, BOOL btOn)
{
	DWORD                  ret = MMSYSERR_NOERROR;
	AudioVoiceMixModes_e   mixMode = kAudioVoiceMode;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(s_currentModemVoiceMode == kModemVoiceEnabled )
	{
		RETAILMSG(1, (L"WAV:%S, Voice is already enabled!\r\n",__FUNCTION__));
		return MMSYSERR_ERROR;
	}

	if(audioProfile == kAudioTDMProfile)
	{
		RETAILMSG(1, (L"WAV:%S, TDM Profile not compatible with Voice!\r\n",__FUNCTION__));
		return MMSYSERR_ERROR;
	}


	if(hTritonDriver)
	{
		if(btOn)
			mixMode = kBTVoiceMode;
		else
		{
			switch(s_currentAudioMode)
			{
				case kAudioModeDisabled:
					mixMode = kVoiceMode;
					break;
				case kAudioModeSpeaker:
					if(outAudioMode == kAudioModeSpeaker )
						break;
				case kAudioModeMicrophone:
					if(inAudioMode == kAudioModeMicrophone)
						break;
				case kAudioModeStereoHeadset:
					if(outAudioMode == kAudioModeStereoHeadset)
						break;
				case kAudioModeAuxHeadset:
					if(inAudioMode == kAudioModeAuxHeadset)
						break;
				case kAudioModeHeadset:
					if(outAudioMode == kAudioModeHeadset)
						break;
				case kAudioModeMicHeadset:
					if(inAudioMode == kAudioModeMicHeadset)
						break;
				case kAudioModeExtAudio:
					if(inAudioMode == kAudioModeExtAudio)
						break;
				case kAudioModeSubMic:
				case kAudioModeClockOnly:
				case kAudioModeHwCodecDisable:
				default:
					return MMSYSERR_ERROR;
			}
		}

		if(s_currentAudioMode == kAudioModeDisabled)
			SetHwCodecMode_EnableCodecPower(hTritonDriver);

		SetHwCodecMode_EnableCodec(hTritonDriver, kSampleRate_44_1KHz, mixMode);
		SetAudioI2SProfile(hTritonDriver, mixMode);

		// enable output
		SetHwCodecMode_EnableOutputGain(hTritonDriver, outAudioMode, mixMode);
		SetHwCodecMode_EnableOutputRouting(hTritonDriver, outAudioMode, mixMode);

		// enable input
		SetHwCodecMode_EnableInputRouting(hTritonDriver, inAudioMode, mixMode);
		SetHwCodecMode_EnableInputGain(hTritonDriver, inAudioMode, mixMode);

		s_currentModemVoiceMode = kModemVoiceEnabled;	   
		if(btOn)
			s_currentModemBTVoiceMode = kModemBTVoiceEnabled;

		return MMSYSERR_NOERROR;
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

	return MMSYSERR_ERROR;
}


DWORD StopHwCodecModem(HANDLE hTritonDriver, BOOL btOn)
{
	AudioVoiceMixModes_e   AudioVoiceMixMode;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(s_currentModemVoiceMode == kModemVoiceDisabled)
	{
		RETAILMSG(1, (L"WAV:%S, Voice is already disabled!\r\n",__FUNCTION__));
		return MMSYSERR_ERROR;
	}

	if(hTritonDriver)
	{
		if(s_currentAudioMode != kAudioModeDisabled)
		{
			if(btOn)
				AudioVoiceMixMode = kBTVoiceMode;
			else
				AudioVoiceMixMode = kVoiceMode;
		}
		else
			AudioVoiceMixMode = kAudioVoiceMode; // So, whole Codec disable

        SetHwCodecMode_DisableOutputRouting(hTritonDriver, AudioVoiceMixMode);
        SetHwCodecMode_DisableOutputGain(hTritonDriver, AudioVoiceMixMode);
        SetHwCodecMode_DisableInputRouting(hTritonDriver, AudioVoiceMixMode);
        SetHwCodecMode_DisableInputGain(hTritonDriver, AudioVoiceMixMode);
        SetHwCodecMode_DisableCodec(hTritonDriver, AudioVoiceMixMode);
        SetHwCodecMode_DisableCodecPower(hTritonDriver, AudioVoiceMixMode);

		s_currentModemVoiceMode = kModemVoiceDisabled;
        s_currentModemBTVoiceMode = kModemBTVoiceDisabled;
	}
	else
	{
		DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));
		return MMSYSERR_ERROR;
	}

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

    return MMSYSERR_NOERROR;
}


DWORD GetStateHwCodecModem(HANDLE hTritonDriver, DWORD *pModemVoiceState, BOOL *pBTOn)
{
	*pModemVoiceState = s_currentModemVoiceMode;
	*pBTOn            = s_currentModemBTVoiceMode;

	return MMSYSERR_NOERROR;
}

DWORD SetOutputGainHwCodecModem(HANDLE hTritonDriver, INT16 gain)
{ 
	INT16 fineGain, coarseGain;
	UINT8  regVal;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	// it should not be a mute, gain != 0
	if((gain < -60) || (gain > 12))
	{
		DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - Invalid (0) gain\r\n",  __FUNCTION__ ));
		return MMSYSERR_INVALPARAM;
	}

	if(hTritonDriver)
	{
		//VRXPGA_GAIN: (6 bits), //fine gain
		// 0x0: Mute
		// 0x1: –36 dB  --  0x31: 12 dB 

		// VRX2ARXPGA_GAIN Voice RX to audio RX PGA gain  (5 bits) - coarse gain !
		// 0x0: Mute
		// 0x1: –24 dB  -- 0x19: 0 dB

		// prefer to set minimum gain to VRXPGA(fine gain), beucause it before Filter
		// fineGain + coarseGain = gain
		// min gain: -60dB, max gain: +12 dB

		if(gain >= -36)
		{
			fineGain = gain;
			coarseGain = 0; // max value
		}
		else
		{
			coarseGain = gain + 36;
			fineGain = -36;
		}

		regVal = (UINT8)(fineGain + 37);
		TWLWriteRegs(hTritonDriver, TWL_VRXPGA, &regVal, sizeof(regVal));

		regVal = (UINT8)( coarseGain + 25 ); //
		TWLWriteRegs(hTritonDriver, TWL_VRX2ARXPGA, &regVal, sizeof(regVal));

		s_Voice_Out_Volume = gain;

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;
	}

	DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}



DWORD GetOutputGainHwCodecModem(HANDLE hTritonDriver, INT16 *pGain)
{
	INT16  fineGain, coarseGain;
	BOOL   bMute = 0;
	UINT8  regVal;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(!hTritonDriver)
	{
		DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));
		return MMSYSERR_ERROR;
	}

	if(s_Voice_Out_Volume == INV_GAIN)
		SetOutputGainHwCodecModem(hTritonDriver, -12);

	TWLReadRegs(hTritonDriver, TWL_VRXPGA, &regVal, sizeof(regVal));
	if(!regVal)
		bMute = 1;

	coarseGain = (INT16)regVal - 37;

	TWLReadRegs(hTritonDriver, TWL_VRX2ARXPGA, &regVal, sizeof(regVal));
	if(!regVal)
		bMute = 1;

	fineGain = (INT16)regVal - 25;

	if(bMute)
		*pGain = -60;  // min value
	else
		*pGain = fineGain + coarseGain;

	if(s_Voice_Out_Volume != *pGain)
	{
		SetOutputGainHwCodecModem(hTritonDriver, s_Voice_Out_Volume);
		*pGain = s_Voice_Out_Volume;
	}

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

	return MMSYSERR_NOERROR;
}


DWORD SetInputGainHwCodecModem(HANDLE hTritonDriver, INT16 gain)
{
	UINT8 regVal;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
		regVal = (UINT8)gain;
		TWLWriteRegs(hTritonDriver, TWL_AVTXL2PGA, &regVal, sizeof(regVal));

		s_Voice_Input_Digital_Gain = gain;

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;
	}

    DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}

DWORD GetInputGainHwCodecModem(HANDLE hTritonDriver, INT16 *pGain)
{
	UINT8  regVal;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
		if(s_Voice_Input_Digital_Gain == INV_GAIN)
			SetInputGainHwCodecModem(hTritonDriver, 15);

		TWLReadRegs(hTritonDriver, TWL_AVTXL2PGA, &regVal, sizeof(regVal));
		*pGain = regVal;

		if(s_Voice_Input_Digital_Gain != *pGain)
		{
			SetInputGainHwCodecModem(hTritonDriver, s_Voice_Input_Digital_Gain);
			*pGain = s_Voice_Input_Digital_Gain;
		}

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;
	}

	RETAILMSG(1, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}


DWORD SetBTInputGainHwCodecModem(HANDLE hTritonDriver, INT16 gain)
{
	UINT8 regVal;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
		TWLReadRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));
		regVal &= 0x0F;
		regVal |= BTTXPGA_GAIN((UINT8)((gain/3) + 5));
		TWLWriteRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));

		s_Bluetooth_In_Volume = gain;

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return(MMSYSERR_NOERROR);
	}

    DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}

DWORD GetBTInputGainHwCodecModem(HANDLE hTritonDriver, INT16 *pGain)
{
	UINT8  regVal;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
		if(s_Bluetooth_In_Volume == INV_GAIN)
			SetBTInputGainHwCodecModem(hTritonDriver, 0);

		TWLReadRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));
 
		*pGain = (INT16) ( ( ((regVal >> 4)&0x0F) - 5)*3 );
 
		if(s_Bluetooth_In_Volume != *pGain)
		{
			SetBTInputGainHwCodecModem(hTritonDriver, s_Bluetooth_In_Volume);
			*pGain = s_Bluetooth_In_Volume;
		}

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;
	}

    DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}



DWORD SetBTOutputGainHwCodecModem(HANDLE hTritonDriver, INT16 gain)
{
	UINT8 regVal;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
		TWLReadRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));
		regVal &= 0xF0;
		regVal |= BTRXPGA_GAIN((UINT8)((gain/3) + 10));
		TWLWriteRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));

		s_Bluetooth_Out_Volume = gain;

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;
	}

    DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}

DWORD GetBTOutputGainHwCodecModem(HANDLE hTritonDriver, INT16 *pGain)
{
	UINT8  regVal;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
		if(s_Bluetooth_Out_Volume == INV_GAIN)
			SetBTOutputGainHwCodecModem(hTritonDriver, 0);

		TWLReadRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));

		*pGain = (INT16) (((regVal & 0x0F) - 10)*3);

		if(s_Bluetooth_Out_Volume != *pGain)
		{
			SetBTOutputGainHwCodecModem(hTritonDriver, s_Bluetooth_Out_Volume);
			*pGain = s_Bluetooth_Out_Volume;
		}

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;
	}

    DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}

DWORD SetVoiceOutMuteHwCodecModem(HANDLE hTritonDriver, BOOL mute)
{
	UINT8  regVal;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
		if(mute)
		{
			TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
			regVal &= ~(VOICE_IF_VIF_DIN_EN);
			TWLWriteRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
		}
		else
		{
			TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
			regVal |= VOICE_IF_VIF_DIN_EN;
			TWLWriteRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
		}

		s_Voice_Out_Mute = mute;

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;
	}

    DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}

DWORD SetVoiceInMuteHwCodecModem(HANDLE hTritonDriver, BOOL mute)
{
	UINT8  regVal;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
		if(mute)
		{
			TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
			regVal &= ~(VOICE_IF_VIF_DOUT_EN);
			TWLWriteRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
		}
		else
		{
			TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
			regVal |= VOICE_IF_VIF_DOUT_EN;
			TWLWriteRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
		}

		s_Voice_In_Mute = mute;

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;
	}

    DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}

DWORD GetVoiceOutMuteHwCodecModem(HANDLE hTritonDriver, BOOL *pMute)
{
	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	*pMute = s_Voice_Out_Mute;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

	return MMSYSERR_NOERROR;
}

DWORD GetVoiceInMuteHwCodecModem(HANDLE hTritonDriver)
{
	return s_Voice_In_Mute;
}

DWORD SetMuteBTInHwCodecModem(HANDLE hTritonDriver, BOOL mute)
{
	UINT8  regVal;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
		if(mute)
		{
			TWLReadRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));
			regVal &= ~(BT_IF_BT_DIN_EN);
			TWLWriteRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));
		}
		else
		{
			TWLReadRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));
			regVal |= (BT_IF_BT_DIN_EN);
			TWLWriteRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));
		}

		// Unmute PCM Voice !
		TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
		regVal |= (VOICE_IF_VIF_DIN_EN | VOICE_IF_VIF_DOUT_EN);
		TWLWriteRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));

		s_BT_In_Mute = mute;

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;
	}

    DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}

DWORD GetMuteBTInHwCodecModem(HANDLE hTritonDriver)
{
	return s_BT_In_Mute;
}

DWORD SetMuteBTOutHwCodecModem(HANDLE hTritonDriver, BOOL mute)
{
	UINT8 regVal;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
		if(mute)
		{
			TWLReadRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));
			regVal &= ~(BT_IF_BT_DOUT_EN);
			TWLWriteRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));
		}
		else
		{
			TWLReadRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));
			regVal |= (BT_IF_BT_DOUT_EN);
			TWLWriteRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));
		}

		// Unmute PCM Voice !
		TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
		regVal |= (VOICE_IF_VIF_DIN_EN | VOICE_IF_VIF_DOUT_EN);
		TWLWriteRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));

		s_BT_Out_Mute = mute;

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;
	}

	DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}

DWORD GetMuteBTOutHwCodecModem(HANDLE hTritonDriver, BOOL *pMute)
{
	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	*pMute = s_BT_Out_Mute;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

	return MMSYSERR_NOERROR;
}

DWORD SetMicMuteHwCodec(HANDLE hTritonDriver, BOOL mute)
{
	UINT8  regVal;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver)
	{
		if(s_Mic_Mute != mute)
		{
			TWLReadRegs(hTritonDriver, TWL_ANAMICL, &regVal, sizeof(regVal));
			
			/* Enable the main MIC and select ADCL input */
			regVal |= ANAMICL_MAINMIC_EN;
			if(mute)
				regVal &= ~ANAMICL_MAINMIC_EN;
			
			TWLWriteRegs(hTritonDriver, TWL_ANAMICL, &regVal, sizeof(regVal) );

			s_Mic_Mute = mute;
		}
		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;
	}

    DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}

DWORD GetMicMuteHwCodec(HANDLE hTritonDriver)
{
	return s_Mic_Mute;
}

DWORD GetModemMicAnalogGainHwCodecModem(HANDLE hTritonDriver, UINT16 *pAGain)
{
	UINT8  regVal;
	UINT16 aGain; 

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver && pAGain)
	{
		if(s_Mic_Analog_Gain == INV_GAIN)
		{
			regVal = (UINT8)ANAMIC_MICAMPL_GAIN(s_pHwCodecConfigInfo->dwHwCodecInMainMicAnalogGain);
			TWLWriteRegs(hTritonDriver, TWL_ANAMIC_GAIN, &regVal, sizeof(regVal));
			s_Mic_Analog_Gain = (GET_ANAMIC_MICAMPL_GAIN(regVal))*6; 
		}

		TWLReadRegs(hTritonDriver, TWL_ANAMIC_GAIN, &regVal, sizeof(regVal));
		aGain = (GET_ANAMIC_MICAMPL_GAIN(regVal))*6;

		if(aGain != s_Mic_Analog_Gain)
			SetModemMicAnalogGainHwCodecModem(hTritonDriver, (UINT16) s_Mic_Analog_Gain);

		*pAGain = s_Mic_Analog_Gain;

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;
	}

    DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}

DWORD GetModemMicAnalogGainHwCodecModemExt(HANDLE hTritonDriver, UINT16 *pAGain)
{
	UINT8  regVal;
	UINT16 aGain;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver && pAGain)
	{
		if(s_External_Microphone_Analog_Gain == INV_GAIN)
		{
			regVal = (UINT8)ANAMIC_MICAMPL_GAIN(s_pHwCodecConfigInfo->dwHwCodecInMainMicAnalogGain);
			TWLWriteRegs(hTritonDriver, TWL_ANAMIC_GAIN, &regVal, sizeof(regVal));
			s_External_Microphone_Analog_Gain = (GET_ANAMIC_MICAMPL_GAIN(regVal))*6; 
		}

		TWLReadRegs(hTritonDriver, TWL_ANAMIC_GAIN, &regVal, sizeof(regVal));
		aGain = (GET_ANAMIC_MICAMPL_GAIN(regVal))*6;

		if(aGain != s_External_Microphone_Analog_Gain)
			SetModemMicAnalogGainHwCodecModem(hTritonDriver, (UINT16)s_External_Microphone_Analog_Gain);

		*pAGain = s_External_Microphone_Analog_Gain;

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;
	}

    DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}

DWORD SetModemMicAnalogGainHwCodecModem(HANDLE hTritonDriver, UINT16 AGain)
{
	UINT8 regVal;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(AGain > 30)
		return MMSYSERR_ERROR;

	if(hTritonDriver)
	{
		if((((UINT8)AGain)%6) <= 3)
			regVal = ((UINT8)AGain)/6;
		else
			regVal = (((UINT8)AGain)/6) + 1;

		TWLWriteRegs(hTritonDriver, TWL_ANAMIC_GAIN, &regVal, sizeof(regVal));

		s_Mic_Analog_Gain = AGain;

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;
	}

    DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}

DWORD SetModemMicAnalogGainHwCodecModemExt(HANDLE hTritonDriver, UINT16 AGain)
{
	UINT8  regVal;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(AGain > 30)
		return MMSYSERR_ERROR;

	if(hTritonDriver)
	{
		if((((UINT8)AGain)%6) <= 3 )
			regVal = ((UINT8)AGain)/6;
		else
			regVal = (((UINT8)AGain)/6) + 1;

		TWLWriteRegs(hTritonDriver, TWL_ANAMIC_GAIN, &regVal, sizeof(regVal));

		s_External_Microphone_Analog_Gain = AGain;

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;
	}

    DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}

DWORD GetModemMicDigitalGainHwCodecModem(HANDLE hTritonDriver, UINT16 *pDigGain )
{
	UINT8  regVal;
	UINT16 dGain;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver && pDigGain)
	{
		if(s_Mic_Digital_Gain == INV_GAIN)
		{
			regVal = (UINT8)s_pHwCodecConfigInfo->dwHwCodecInMainMicDigitalGain;
			TWLWriteRegs(hTritonDriver, TWL_ATXL1PGA, &regVal, sizeof(regVal));
			s_Mic_Digital_Gain = regVal;
		}

		TWLReadRegs(hTritonDriver, TWL_ATXL1PGA, &regVal, sizeof(regVal));
		dGain = regVal;

		if(dGain != s_Mic_Digital_Gain)
			SetModemMicDigitalGainHwCodecModem(hTritonDriver, (UINT16) s_Mic_Digital_Gain);

		*pDigGain = s_Mic_Digital_Gain;

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;
	}

    DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}

DWORD GetModemMicDigitalGainHwCodecModemExt(HANDLE hTritonDriver, UINT16 *pDigGain )
{
	UINT8  regVal;
	UINT16 dGain;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(hTritonDriver && pDigGain)
	{
		if(s_External_Microphone_Digital_Gain == INV_GAIN)
		{
			regVal = (UINT8)s_pHwCodecConfigInfo->dwHwCodecInMainMicDigitalGain;
			TWLWriteRegs(hTritonDriver, TWL_ATXL1PGA, &regVal, sizeof(regVal));
			s_External_Microphone_Digital_Gain = regVal;
		}

		TWLReadRegs(hTritonDriver, TWL_ATXL1PGA, &regVal, sizeof(regVal));
		dGain = regVal;

		if(dGain != s_External_Microphone_Digital_Gain)
			SetModemMicDigitalGainHwCodecModemExt(hTritonDriver, (UINT16) s_External_Microphone_Digital_Gain);

		*pDigGain = s_External_Microphone_Digital_Gain;

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;

	}
	DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}

DWORD SetModemMicDigitalGainHwCodecModem(HANDLE hTritonDriver, UINT16 DigGain)
{
	UINT8 regVal;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(DigGain > 31)
		return MMSYSERR_ERROR;

	if(hTritonDriver)
	{
		/* Set Tx path volume control */
		regVal = (UINT8)DigGain;

		//regVal = (UINT8)s_pHwCodecConfigInfo->dwHwCodecInMainMicDigitalGain;
		TWLWriteRegs(hTritonDriver, TWL_ATXL1PGA, &regVal, sizeof(regVal));

		s_Mic_Digital_Gain = DigGain;

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;

	}
    DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}

DWORD SetModemMicDigitalGainHwCodecModemExt(HANDLE hTritonDriver, UINT16 DigGain)
{
	UINT8  regVal;

	DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

	if(DigGain > 31)
		return MMSYSERR_ERROR;

	if(hTritonDriver)
	{
		/* Set Tx path volume control */
		regVal = (UINT8)DigGain;

		//regVal = (UINT8)s_pHwCodecConfigInfo->dwHwCodecInMainMicDigitalGain;
		TWLWriteRegs(hTritonDriver, TWL_ATXL1PGA, &regVal, sizeof(regVal));

		s_External_Microphone_Digital_Gain = DigGain;

		DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));

		return MMSYSERR_NOERROR;

	}
    DEBUGMSG(ZONE_TPS659XX, (L"WAV:%S - hTritonDriver is NULL!!!\r\n",  __FUNCTION__ ));

	return MMSYSERR_ERROR;
}

DWORD SetMuteBTOutPlaybackCodecModem(HANDLE hTritonDriver, BOOL mute)
{
	UINT8  regVal;

	if(hTritonDriver)
	{
		if( mute )
			regVal = 0;   // Mute
		else
			regVal = 0x3F;   // 0 db

		TWLWriteRegs(hTritonDriver, TWL_ARX2VTXPGA, &regVal, sizeof(regVal));

		return MMSYSERR_NOERROR;
	}

	return MMSYSERR_ERROR;
}

DWORD  GetMuteBTOutPlaybackCodecModem(HANDLE hTritonDriver, BOOL *pMute)
{
	UINT8  regVal;

	if(hTritonDriver && pMute)
	{
		TWLReadRegs(hTritonDriver, TWL_ARX2VTXPGA, &regVal, sizeof(regVal));

		if(regVal)
			*pMute = 0;
		else
			*pMute = 1;   // Mute

		return MMSYSERR_NOERROR;
	}

	return MMSYSERR_ERROR;
}

// -------------------  Ext Audio HW functions for Mixrer ----------------------
//
DWORD GetExtMicMuteHwCodec(HANDLE hTritonDriver)
{
	return s_Ext_Mic_Mute;
}

DWORD SetExtMicMuteHwCodec(HANDLE hTritonDriver, BOOL mute)
{
	s_Ext_Mic_Mute = mute;

	return MMSYSERR_NOERROR;
}

// -----------------------------------------------------------------------------
//
//  @doc    SetHwCodecGain
//
//  @func   BOOL | SetHwCodecGain | sets hradware codec gain.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
BOOL
SetHwCodecMode_Gain(
    HANDLE hTritonDriver,
    DWORD dwGain,
    DWORD *pdwGlobalGain)
{
    BOOL returnValue = TRUE;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+%S\r\n",__FUNCTION__));

    // if gain levels are identical then nothing needs to be done
    if (*pdwGlobalGain == dwGain)
        {
        returnValue = FALSE;
        }
    else
        {
        *pdwGlobalGain = dwGain;
        }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-%S\r\n",__FUNCTION__));
    return returnValue;
}

#if defined(SYSGEN_BTH_AG)
DWORD GetBthAudioRecGainHwCodec(HANDLE hTritonDriver, LONG *pGain)
{
	UINT8   regVal;
	if (!hTritonDriver || !pGain)
		return MMSYSERR_INVALPARAM;

	if (!readBthAudioReg(VC_DAC_ATTENUATION, &regVal))
		return MMSYSERR_ERROR;

	*pGain = (MAX9860_MAX_DVA - regVal) >> 1;
	return MMSYSERR_NOERROR;
}

DWORD GetBthAudioRecMuteHwCodec(HANDLE hTritonDriver, BOOL *pMute)
{
	UINT8   regVal;
	if (!hTritonDriver || !pMute)
		return MMSYSERR_INVALPARAM;

	if (!readBthAudioReg(VC_SYSTEM_SHUTDOWN, &regVal))
		return MMSYSERR_ERROR;

	*pMute = (regVal & VC_DACEN) ? FALSE : TRUE;
	return MMSYSERR_NOERROR;
}

DWORD SetBthAudioRecMuteHwCodec(HANDLE hTritonDriver, BOOL fMute)
{
	return MMSYSERR_NOERROR;
}

DWORD GetBthAudioPlayVolHwCodec(HANDLE hTritonDriver, LONG *pVolume)
{
	UINT8   regVal;
	RETAILMSG(1, (L"WAV: %S\r\n",__FUNCTION__));
	if (!hTritonDriver || !pVolume)
		return MMSYSERR_INVALPARAM;

	if (!readBthAudioReg(VC_ADC_OUTPUT_LEVELS, &regVal))
		return MMSYSERR_ERROR;

	*pVolume = MAX9860_MAX_ADC_OUTPUT_LEVEL - (regVal >> 4);
	return MMSYSERR_NOERROR;
}

DWORD GetBthAudioPlayMuteHwCodec(HANDLE hTritonDriver, BOOL *pMute)
{
	UINT8   regVal;
	RETAILMSG(1, (L"WAV: %S\r\n",__FUNCTION__));
	if (!hTritonDriver || !pMute)
		return MMSYSERR_INVALPARAM;

	if (!readBthAudioReg(VC_SYSTEM_SHUTDOWN, &regVal))
		return MMSYSERR_ERROR;

	*pMute = (regVal & VC_ADCLEN) ? FALSE : TRUE;
	return MMSYSERR_NOERROR;
}

DWORD SetBthAudioPlayMuteHwCodec(HANDLE hTritonDriver, BOOL fMute)
{
	return MMSYSERR_NOERROR;
}
#endif

// -----------------------------------------------------------------------------
//
//  @doc    DumpTritonAudioRegs
//
//  @func   VOID | DumpTritonAudioRegs | Dumps T2 Registers.
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------
static void DumpTritonAudioRegs(HANDLE hTritonDriver)
{
    RETAILMSG(1, (L"WAV:+%S\r\n",__FUNCTION__));

    if(hTritonDriver)
	{
		UINT8 regVal;

        TWLReadRegs(hTritonDriver, TWL_CODEC_MODE, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_CODEC_MODE = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_OPTION, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_OPTION = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_MICBIAS_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_MICBIAS_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ANAMICL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ANAMICL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ANAMICR, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ANAMICR = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_AVADC_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_AVADC_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ADCMICSEL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ADCMICSEL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_DIGMIXING, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_DIGMIXING = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ATXL1PGA, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ATXL1PGA = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ATXR1PGA, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ATXR1PGA = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_AVTXL2PGA, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_AVTXL2PGA = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_AVTXR2PGA, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_AVTXR2PGA = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_AUDIO_IF, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_AUDIO_IF = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_VOICE_IF, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_VOICE_IF = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ARXR1PGA, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ARXR1PGA = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ARXL1PGA, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ARXL1PGA = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ARXR2PGA, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ARXR2PGA = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ARXL2PGA, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ARXL2PGA = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_VRXPGA, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_VRXPGA = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_VSTPGA, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_VSTPGA = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_VRX2ARXPGA, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_VRX2ARXPGA = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_AVDAC_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_AVDAC_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ARX2VTXPGA, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ARX2VTXPGA = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ARXL1_APGA_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ARXL1_APGA_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ARXR1_APGA_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ARXR1_APGA_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ARXL2_APGA_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ARXL2_APGA_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ARXR2_APGA_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ARXR2_APGA_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ATX2ARXPGA, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ATX2ARXPGA = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_BT_IF, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_BT_IF = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_BTPGA, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_BTPGA = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_BTSTPGA, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_BTSTPGA = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_EAR_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_EAR_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_HS_SEL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_HS_SEL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_HS_GAIN_SET, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_HS_GAIN_SET = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_HS_POPN_SET, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_HS_POPN_SET = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_PREDL_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_PREDL_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_PREDR_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_PREDR_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_PRECKL_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_PRECKL_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_PRECKR_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_PRECKR_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_HFL_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_HFL_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_HFR_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_HFR_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ALC_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ALC_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ALC_SET1, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ALC_SET1 = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ALC_SET2, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ALC_SET2 = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_BOOST_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_BOOST_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_SOFTVOL_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_SOFTVOL_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_DTMF_FREQSEL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_DTMF_FREQSEL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_DTMF_TONEXT1H, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_DTMF_TONEXT1H = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_DTMF_TONEXT1L, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_DTMF_TONEXT1L = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_DTMF_TONEXT2H, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_DTMF_TONEXT2H = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_DTMF_TONEXT2L, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_DTMF_TONEXT2L = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_DTMF_TONOFF, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_DTMF_TONOFF = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_DTMF_WANONOFF, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_DTMF_WANONOFF = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_I2S_RX_SCRAMBLE_H, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_I2S_RX_SCRAMBLE_H = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_I2S_RX_SCRAMBLE_M, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_I2S_RX_SCRAMBLE_M = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_I2S_RX_SCRAMBLE_L, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_I2S_RX_SCRAMBLE_L = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_APLL_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_APLL_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_DTMF_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_DTMF_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_DTMF_PGA_CTL2, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_DTMF_PGA_CTL2 = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_DTMF_PGA_CTL1, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_DTMF_PGA_CTL1 = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_MISC_SET_1, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_MISC_SET_1 = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_PCMBTMUX, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_PCMBTMUX = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_RX_PATH_SEL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_RX_PATH_SEL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_VDL_APGA_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_VDL_APGA_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_VIBRA_CTL, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_VIBRA_CTL = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_VIBRA_SET, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_VIBRA_SET = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_VIBRA_PWM_SET, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_VIBRA_PWM_SET = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_ANAMIC_GAIN, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_ANAMIC_GAIN = 0x%x\r\n", regVal));

        TWLReadRegs(hTritonDriver, TWL_MISC_SET_2, &regVal, sizeof(regVal));
        RETAILMSG(1, (L"WAV: TWL_MISC_SET_2 = 0x%x\r\n", regVal));

	}

    RETAILMSG(1, (L"WAV:-%S\r\n",__FUNCTION__));
}