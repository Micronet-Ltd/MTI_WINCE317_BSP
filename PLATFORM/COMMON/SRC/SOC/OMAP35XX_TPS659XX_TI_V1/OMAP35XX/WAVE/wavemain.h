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
#ifndef __WAVEMAIN_H__
#define __WAVEMAIN_H__

//------------------------------------------------------------------------------
//

// Number of output channels, 1=mono, 2=stereo
#define OUTCHANNELS                     (2)

// Number of input channels supported by wave driver
#define INCHANNELS                      (2)

// Supported hardware input channels
// NOTE: The headset mic is presently mono.
#define HARDWARE_INCHANNELS             (2)

// Number of significant bits per sample (e.g. 16=16-bit codec, 24=24-bit codec)
#define BITSPERSAMPLE                   (16)

// Native sample rate
#define SAMPLERATE                      (44100)

// Inverse sample rate, in 0.32 fixed format, with 1 added at bottom to ensure round up.
#define INVSAMPLERATE                   ((UINT32)(((1i64<<32)/SAMPLERATE)+1))

// Set USE_MIX_SATURATE to 1 if you want the mixing code to guard against saturation
//#define USE_MIX_SATURATE              (1)

// Set USE_HW_SATURATE to 1 if you want the hwctxt code to guard against saturation
// This only works if the DMA sample size is greater than 16 bits.
#define USE_HW_SATURATE                 (0)

// Size of 1 dma page. There are 2 pages for input & 2 pages for output,
// so the total size of the DMA buffer will be
// AUDIO_DMA_PAGES * (AUDIO_DMA_INPUT_PAGE_SIZE + AUDIO_DMA_OUTPUT_PAGE_SIZE)
#define AUDIO_DMA_OUTPUT_PAGE_SIZE      0x2000

// input dma size is reduced for better latency in voice recording
#define AUDIO_DMA_INPUT_PAGE_SIZE       0x1800
#define AUDIO_DMA_PAGES                 2

// If MONO_GAIN is defined, all gain will be mono (left volume applied to both channels)
#if defined(MONO_GAIN)
#define MAX_GAIN                        0xFFFF
#else
#define MAX_GAIN                        0xFFFFFFFF
#endif


// defaults for software mixer volumes in db
#define STREAM_ATTEN_MAX                100
#define DEVICE_ATTEN_MAX                35
#define SECOND_ATTEN_MAX                35

#define AUDIO_PROFILE_OUT_HANDSET       0x0001
#define AUDIO_PROFILE_OUT_HEADSET       0x0002
#define AUDIO_PROFILE_OUT_BT_HEADSET    0x0003
#define AUDIO_PROFILE_OUT_SPEAKER       0x0008
#define AUDIO_PROFILE_OUT_CARKIT        0x0010
#define AUDIO_PROFILE_OUT_USB           0x0020

#define AUDIO_PROFILE_IN_SYSTEM         0x0001
#define AUDIO_PROFILE_IN_HANDSET_MIC    0x0002
#define AUDIO_PROFILE_IN_HEADSET        0x0004
#define AUDIO_PROFILE_IN_BT_HEADSET     0x0008
#define AUDIO_PROFILE_IN_EXTERNAL       0x0010
#define AUDIO_PROFILE_IN_CARKIT         0x0020
#define AUDIO_PROFILE_IN_USB            0x0040

#define LOGICAL_VOLUME_MAX              0xFFFF
#define LOGICAL_VOLUME_MIN              0
#define LOGICAL_VOLUME_STEPS            1

#define DRIVER_VERSION                  100

//------------------------------------------------------------------------------
//
// audio mixer macros
//
// mixer line ID are 16-bit values formed by concatenating the source and
// destination line indices
//
#define MXLINEID(dst,src)       ((WORD) ((WORD)(dst) | (((WORD) (src)) << 8)))
#define MXSOURCEID(id)          (id >> 8)
#define MXDESTINATIONID(id)     (id & 0xFF)

#define MXCONTROLID(line, ctrl) ((((DWORD)line) << 16) | ctrl)
#define MXCONTROLINDEX(id)      ((WORD)(id & 0xFFFF))
#define MXCONTROLLINEID(id)     ((WORD)(id >> 16))

// Native sample size, INT16 for 16-bit samples, INT32 for anything larger.
typedef INT16 HWSAMPLE;
typedef HWSAMPLE *PHWSAMPLE;


#define  WAVE_AUDIO_DBG      0
#define  WAVE_IOCTL_DBG      0

//------------------------------------------------------------------------------
// Forward declaration
class CAudioManager;

//------------------------------------------------------------------------------
//

EXTERN_C void EnterMutex();
EXTERN_C void ExitMutex();
EXTERN_C void InitializeMixers(CAudioManager* pAudioManager);
EXTERN_C CAudioManager* CreateAudioManager();
EXTERN_C void DeleteAudioManager(CAudioManager* pAudioManager);
EXTERN_C BOOL InitializeHardware(LPCWSTR szContext,
                                 LPCVOID pBusContext);
EXTERN_C void UninitializeHardware(DWORD dwData);

typedef DWORD (fnAudioMixerCallback)(DWORD hmx,
                                     UINT uMsg,
                                     DWORD dwInstance,
                                     DWORD dwParam1,
                                     DWORD dwParam2);


EXTERN_C void UpdateRegParams();



#endif //__WAVMAIN_H__

