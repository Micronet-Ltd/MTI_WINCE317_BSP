/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File: tps659xx_wave.h
//
#ifndef __TPS659XX_WAVE_H
#define __TPS659XX_WAVE_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//  Local enumerations

// defines triton audio modes
typedef enum {
    kAudioModeDisabled,
    kAudioModeSpeaker,
    kAudioModeMicrophone,
    kAudioModeStereoHeadset,
    kAudioModeSubMic,
    kAudioModeHeadset,
	kAudioModeBTHeadset,
	kAudioModeExtAudio,
    kAudioModeAuxHeadset,
    kAudioModeMicHeadset,
    kAudioModeClockOnly,
    kAudioModeHwCodecDisable,
#if defined(SYSGEN_BTH_AG)
	kAudioModeBluetoothAudio,
#endif
    //kAudioModeModemVoiceCall // ##### Michael

	kNUM_OF_AUDIO_MODES
} AudioMode_e;

typedef enum
{
    kModemVoiceDisabled = 0,
    kModemVoiceEnabled,

} ModemVoiceMode_e;

typedef enum
{
    kModemBTVoiceDisabled = 0,
    kModemBTVoiceEnabled,       // Valid only when kModemVoiceEnabled !

} ModemBTVoiceMode_e;


typedef enum
{
	kAudioMode = 0,   // Only Audio 
	kAudioVoiceMode,  // Whole Codec
	kVoiceMode,       // only Voice
	kBTVoiceMode,     // Bluetooth Voice (BT Audio Gate mode), independently from Audio
    kBTPlayMode,      // Test mode
	//kExtAudioMode,

	kNUM_OF_MIX_MODES
} AudioVoiceMixModes_e;


// defines triton audio profiles
typedef enum {
    kAudioI2SProfile=0,
    kAudioTDMProfile,
	kAudioPCMProfile,
    kAudioProfileUnknown
} AudioProfile_e;

// defines triton mode
typedef enum {
    kT2ModeSlave=0,
    kT2ModeMaster
} T2Mode_e;

// Prototypes
//
extern void SetHwCodecMode_DisableCodecPower(HANDLE hTritonDriver, AudioVoiceMixModes_e mixMode);
extern void SetHwCodecMode_EnableCodecPower(HANDLE hTritonDriver);
extern BOOL SetHwCodecMode_GetCodecPower(HANDLE hTritonDriver);
static void SetHwCodecMode_DisableCodec(HANDLE hTritonDriver, AudioVoiceMixModes_e mixMode);
static void SetHwCodecMode_EnableCodec(HANDLE hTritonDriver, UINT sampleRate, AudioVoiceMixModes_e mixMode);
static void SetHwCodecMode_DisableOutputRouting(HANDLE hTritonDriver, AudioVoiceMixModes_e mixMode);
static void SetHwCodecMode_DisableOutputGain(HANDLE hTritonDriver, AudioVoiceMixModes_e mixMode);
static void SetHwCodecMode_EnableOutputRouting(HANDLE hTritonDriver, AudioMode_e type, AudioVoiceMixModes_e mixMode);
static void SetHwCodecMode_EnableOutputGain(HANDLE hTritonDriver, AudioMode_e type, AudioVoiceMixModes_e mixMode);
static void SetHwCodecMode_DisableInputRouting(HANDLE hTritonDriver, AudioVoiceMixModes_e mixMode);
static void SetHwCodecMode_DisableInputGain(HANDLE hTritonDriver, AudioVoiceMixModes_e mixMode);
static void SetHwCodecMode_EnableInputRouting(HANDLE hTritonDriver, AudioMode_e type, AudioVoiceMixModes_e mixMode);
static void SetHwCodecMode_EnableInputGain(HANDLE hTritonDriver, AudioMode_e type, AudioVoiceMixModes_e mixMode);
extern void SetHwCodecMode_Disabled(HANDLE hTritonDriver);
extern void SetHwCodecMode_Microphone(HANDLE hTritonDriver, AudioProfile_e audioProfile);
extern void SetHwCodecMode_Speaker(HANDLE hTritonDriver, AudioProfile_e audioProfile);
extern void SetHwCodecMode_MicHeadset(HANDLE hTritonDriver, AudioProfile_e audioProfile);
extern void SetHwCodecMode_Headset(HANDLE hTritonDriver, AudioProfile_e audioProfile);
extern void SetHwCodecMode_BTHeadset(HANDLE hTritonDriver, AudioProfile_e audioProfile);
extern void SetHwCodecMode_ExtAudio(HANDLE hTritonDriver, AudioProfile_e audioProfile);
extern void SetHwCodecMode_AuxHeadset(HANDLE hTritonDriver, AudioProfile_e audioProfile);
extern void SetHwCodecMode_StereoHeadset(HANDLE hTritonDriver, AudioProfile_e audioProfile);
extern void SetHwCodecMode_EnableT2AudioClkOnly(HANDLE hTritonDriver);
extern HANDLE OpenHwCodecHandle(HwCodecConfigInfo_t *pHwCodecConfigInfo);
extern BOOL SetHwCodecMode_Gain(HANDLE hTritonDriver, DWORD dwGain, DWORD *pdwGlobalGain);
extern void SetAudioI2SProfile(HANDLE s_hTritonDriver, AudioVoiceMixModes_e mixMode);
extern void SetAudioTDMProfile(HANDLE s_hTritonDriver);  
extern void SetAudioPCMProfile(HANDLE hTritonDriver); // ##### Michael
#if defined(SYSGEN_BTH_AG)
extern void SetHwCodecMode_BthAudio(HANDLE hTritonDriver, AudioProfile_e audioProfile);
#endif

//extern void SetHwCodecMode_ModemVoiceCall(HANDLE hTritonDriver, AudioProfile_e audioProfile);// ##### Michael

extern DWORD SetAnalogGainHwCodec(HANDLE hTritonDriver, INT16 aGain );
extern DWORD GetAnalogGainHwCodec(HANDLE hTritonDriver, INT16 *pAGain );

extern DWORD StartHwCodecModem(HANDLE hTritonDriver, AudioProfile_e audioProfile, AudioMode_e inAudioMode, AudioMode_e outAudioMode, BOOL btOn);
extern DWORD StopHwCodecModem(HANDLE hTritonDriver, BOOL btOn);
extern DWORD GetStateHwCodecModem(HANDLE hTritonDriver, DWORD *pModemVoiceState, BOOL *pBTOn);
extern DWORD SetOutputGainHwCodecModem(HANDLE hTritonDriver, INT16 gain);
extern DWORD GetOutputGainHwCodecModem(HANDLE hTritonDriver, INT16 *pGain);

extern DWORD SetInputGainHwCodecModem(HANDLE hTritonDriver, INT16 gain);
extern DWORD GetInputGainHwCodecModem(HANDLE hTritonDriver, INT16 *pGain);

extern DWORD SetBTInputGainHwCodecModem(HANDLE hTritonDriver, INT16 gain);
extern DWORD GetBTInputGainHwCodecModem(HANDLE hTritonDriver, INT16 *pGain);

extern DWORD SetBTOutputGainHwCodecModem(HANDLE hTritonDriver, INT16 gain);
extern DWORD GetBTOutputGainHwCodecModem(HANDLE hTritonDriver, INT16 *pGain);

extern DWORD SetVoiceOutMuteHwCodecModem(HANDLE hTritonDriver, BOOL mute);
extern DWORD GetVoiceOutMuteHwCodecModem(HANDLE hTritonDriver, BOOL *pMute);

extern DWORD SetVoiceInMuteHwCodecModem(HANDLE hTritonDriver, BOOL mute);
extern DWORD GetVoiceInMuteHwCodecModem(HANDLE hTritonDriver);

extern DWORD SetMuteBTInHwCodecModem(HANDLE hTritonDriver, BOOL mute);
extern DWORD GetMuteBTInHwCodecModem(HANDLE hTritonDriver);

extern DWORD SetMuteBTOutHwCodecModem(HANDLE hTritonDriver, BOOL mute);
extern DWORD GetMuteBTOutHwCodecModem(HANDLE hTritonDriver, BOOL *pMute);

extern DWORD SetMicMuteHwCodec(HANDLE hTritonDriver, BOOL mute);
extern DWORD GetMicMuteHwCodec(HANDLE hTritonDriver);

extern DWORD GetModemMicAnalogGainHwCodecModem(HANDLE hTritonDriver, UINT16 *pAGain);
extern DWORD SetModemMicAnalogGainHwCodecModem(HANDLE hTritonDriver, UINT16 AGain);
extern DWORD SetModemMicAnalogGainHwCodecModemExt(HANDLE hTritonDriver, UINT16 AGain);
extern DWORD GetModemMicAnalogGainHwCodecModemExt(HANDLE hTritonDriver, UINT16 *pAGain);
extern DWORD GetModemMicDigitalGainHwCodecModem(HANDLE hTritonDriver, UINT16 *pDigGain );
extern DWORD GetModemMicDigitalGainHwCodecModemExt(HANDLE hTritonDriver, UINT16 *pDigGain );
extern DWORD SetModemMicDigitalGainHwCodecModem(HANDLE hTritonDriver, UINT16 DigGain);
extern DWORD SetModemMicDigitalGainHwCodecModemExt(HANDLE hTritonDriver, UINT16 DigGain);

DWORD  SetMuteBTOutPlaybackCodecModem(HANDLE hTritonDriver, BOOL mute);
DWORD  GetMuteBTOutPlaybackCodecModem(HANDLE hTritonDriver, BOOL *pMute);

extern DWORD  GetExtMicMuteHwCodec(HANDLE hTritonDriver);
extern DWORD  SetExtMicMuteHwCodec(HANDLE hTritonDriver, BOOL mute);

#if defined(SYSGEN_BTH_AG)
extern DWORD GetBthAudioRecGainHwCodec(HANDLE hTritonDriver, LONG *pGain);
extern DWORD GetBthAudioRecMuteHwCodec(HANDLE hTritonDriver, BOOL *pMute);
extern DWORD SetBthAudioRecMuteHwCodec(HANDLE hTritonDriver, BOOL fMute);

extern DWORD GetBthAudioPlayVolHwCodec(HANDLE hTritonDriver, LONG *pVolume);
extern DWORD GetBthAudioPlayMuteHwCodec(HANDLE hTritonDriver, BOOL *pMute);
extern DWORD SetBthAudioPlayMuteHwCodec(HANDLE hTritonDriver, BOOL fMute);
#endif

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif

