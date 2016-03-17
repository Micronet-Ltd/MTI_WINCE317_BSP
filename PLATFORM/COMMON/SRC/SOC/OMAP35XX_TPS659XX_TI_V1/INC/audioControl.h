// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
#ifndef _AUDIO_CONTROL_H_
#define _AUDIO_CONTROL_H_

#include <windows.h>
#include "wavext.h"

#define SPKR_INT_SD_GPIO	GPIO_175
#define SPKR_EXT_SD_GPIO	GPIO_176
#define MIC_INT_EXT_SW_GPIO	GPIO_161

typedef enum
{
	SPKR_PREDRIVE_CLASSD_2_INT_SPKER = 0,
	SPKR_PREDRIVE_CLASSD_2_EXT_SPKER,
	SPKR_NUM_OF_AMPLIFIERS
} SPKR_AMPLIFIERS_e;

typedef enum
{
	MIC_MAIN_2_INT_MIC = 0,
	MIC_MAIN_2_EXT_MIC,
	NUM_NUM_OF_AMPLIFIERS
} MIC_SWITCH_e;

#ifdef __cplusplus
	extern "C" {
#endif
		void *open_amplifiers_control(void);
		void speak_mut_all(void *);
		void spkr_route(void *, SPKR_AMPLIFIERS_e);
		void mic_route(void *, MIC_SWITCH_e);
		SPKR_AMPLIFIERS_e spkr_current_route(void *);
		MIC_SWITCH_e mic_current_route(void *);
#ifdef __cplusplus
		}
#endif	
#endif //_AUDIO_CONTROL_H_

//		mic_route(s_pHwCodecConfigInfo->amplifiers_control, MIC_MAIN_2_INT_MIC);
//		mic_route(s_pHwCodecConfigInfo->amplifiers_control, MIC_MAIN_2_EXT_MIC);
//		spkr_route(s_pHwCodecConfigInfo->amplifiers_control, SPKR_PREDRIVE_CLASSD_2_INT_SPKER);
//		spkr_route(s_pHwCodecConfigInfo->amplifiers_control, SPKR_PREDRIVE_CLASSD_2_EXT_SPKER);
//		speak_mut_all(s_pHwCodecConfigInfo->amplifiers_control); // internal amplifier off
