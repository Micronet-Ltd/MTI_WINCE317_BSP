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
//------------------------------------------------------------------------------
//
//  File:  wavext.h
//
#ifndef __WAVEXT_H
#define __WAVEXT_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Extensions to Wave driver 

#define FILE_DEVICE_AXVOICE         0x368

#define IOCTL_NOTIFY_HEADSET        \
    CTL_CODE(FILE_DEVICE_AXVOICE,  10, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_NOTIFY_BT_HEADSET     \
    CTL_CODE(FILE_DEVICE_AXVOICE,  12, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_NOTIFY_HDMI         \
    CTL_CODE(FILE_DEVICE_AXVOICE,  23, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_NOTIFY_AUXHEADSET     \
    CTL_CODE(FILE_DEVICE_AXVOICE,  24, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_NOTIFY_HF_SPEAKERS    \
    CTL_CODE(FILE_DEVICE_AXVOICE,  25, METHOD_BUFFERED, FILE_ANY_ACCESS)

//------------------------------------------------------------------------------
// T2 Hardware audio codec config settings structure

typedef struct {
    DWORD    dwHwCodecInMainMicDigitalGain;
    DWORD    dwHwCodecInSubMicDigitalGain;
    DWORD    dwHwCodecInHeadsetMicDigitalGain;
    DWORD    dwHwCodecInMainMicAnalogGain;
    DWORD    dwHwCodecInSubMicAnalogGain;
    DWORD    dwHwCodecInHeadsetMicAnalogGain;
    DWORD    dwHwCodecOutStereoSpeakerDigitalGain;
    DWORD    dwHwCodecOutStereoHeadsetDigitalGain;
    DWORD    dwHwCodecOutHeadsetMicDigitalGain;
    DWORD    dwHwCodecOutStereoSpeakerAnalogGain;
    DWORD    dwHwCodecOutStereoHeadsetAnalogGain;
    DWORD    dwHwCodecOutHeadsetMicAnalogGain;
    DWORD    dwHwCodecInHeadsetAuxDigitalGain;
    DWORD    dwHwCodecInHeadsetAuxAnalogGain;
	DWORD    dwHwCodecOutHeadsetDigitalGain;
    DWORD    dwHwCodecOutHeadsetAnalogGain;

	// Bluetooth audio
#if defined(SYSGEN_BTH_AG)
	DWORD					dwHwCodecInBthAudioDigitalGain;
	DWORD					dwHwCodecOutBthAudioDigitalGain;
	DWORD					dwHwCodec_ARXL1PGA;
	DWORD					dwHwCodec_ATXR1PGA;
	BOOL					bBthAudio;
#endif

	void	*amplifiers_control;
	BOOL                    bModem;
	BOOL                    bBT;
	BOOL                    bExtAudio;

} HwCodecConfigInfo_t;

//------------------------------------------------------------------------------
// IOCTL_NOTIFY_BT_HEADSET defines

#define BT_AUDIO_NONE       0x00000000      // No audio routed to BT device
#define BT_AUDIO_SYSTEM     0x00000001      // System audio routed to BT device
#define BT_AUDIO_MODEM      0x00000002      // Modem audio routed to/from BT device

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
