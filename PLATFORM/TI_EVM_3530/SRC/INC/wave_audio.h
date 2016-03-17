#ifndef		__WAVE_AUDIO_H
#define		__WAVE_AUDIO_H

// Public audio messiges, should be used as second parameter of waveOutMessage
#define WPDM_PRIVATE						10000
#define WPDM_PRIVATE_UPDATE_REG_VAL			(WPDM_PRIVATE+1)


#define WAV_DEV_REG_KEY			L"Drivers\\BuiltIn\\WaveDev"
#define WAV_DEV_REG_MIC_DGAIN   L"HwCodecInMainMicDigitalGain"
#define WAV_DEV_REG_MIC_AGAIN   L"HwCodecInMainMicAnalogGain"


#define MAX_IN_MAIN_MIC_DGAIN		31
#define MAX_IN_MAIN_MIC_AGAIN		5
#define MIN_IN_MAIN_MIC_DGAIN		0
#define MIN_IN_MAIN_MIC_AGAIN		0

#define DGAIN_TICK_INTERVAL		1
#define AGAIN_TICK_INTERVAL		6

#endif	//#ifndef __WAVE_AUDIO_H