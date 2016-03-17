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
#pragma once

// -----------------------------------------------------------------------------
//
// @doc     EX_AUDIO_DDSI
//
// @enum    PCM_TYPE | Enumeration of standard PCM data types.
//
// @field   PBYTE | pData | Pointer to just the data buffer which contains
//          the waveform data to be played.
//
// @field   ULONG | nBufferSize |
//          Size, in number of bytes, of the buffer to play.
//
// @field   ULONG | nBufferPosition |
//          Current position, in number of bytes in the user buffer.
//
// @field   BOOL | fLooping |
//          If TRUE, the buffer should be played in an infinite loop.
//
// -----------------------------------------------------------------------------
typedef enum {
     PCM_TYPE_M8,
     PCM_TYPE_M16,
     PCM_TYPE_S8,
     PCM_TYPE_S16
} PCM_TYPE, *PPCM_TYPE;


// -----------------------------------------------------------------------------
//
// @doc     EX_AUDIO_DDSI
//
// @struct  SAMPLE_8_MONO | Single sample from 8-bit mono PCM data stream.
//
// @field   UINT8 | sample | Unsigned 8-bit sample
//
// -----------------------------------------------------------------------------
typedef struct  {
    UINT8 sample;              // Unsigned 8-bit sample
} SAMPLE_8_MONO;


// -----------------------------------------------------------------------------
//
// @doc     EX_AUDIO_DDSI
//
// @struct  SAMPLE_16_MONO | Single sample from 16-bit mono PCM data stream.
//
// @field   INT16 | sample | Unsigned 16-bit sample
//
// -----------------------------------------------------------------------------
typedef struct  {
    INT16 sample;              // Signed 16-bit sample
} SAMPLE_16_MONO;


// -----------------------------------------------------------------------------
//
// @doc     EX_AUDIO_DDSI
//
// @struct  SAMPLE_8_STEREO | Single sample from 8-bit stereo PCM data stream.
//
// @field   UINT8 | sample_left | Unsigned 8-bit sample from left channel.
//
// @field   UINT8 | sample_right | Unsigned 8-bit sample from right channel.
//
// -----------------------------------------------------------------------------
typedef struct  {
    UINT8 sample_left;         // Unsigned 8-bit sample
    UINT8 sample_right;        // Unsigned 8-bit sample
} SAMPLE_8_STEREO;


// -----------------------------------------------------------------------------
//
// @doc     EX_AUDIO_DDSI
//
// @struct  SAMPLE_16_STEREO | Single sample from 16-bit stereo PCM data stream.
//
// @field   UINT16 | sample_left | Unsigned 16-bit sample from left channel.
//
// @field   UINT16 | sample_right | Unsigned 16-bit sample from right channel.
//
// -----------------------------------------------------------------------------
typedef struct  {
    INT16 sample_left;         // Signed 16-bit sample
    INT16 sample_right;        // Signed 16-bit sample
} SAMPLE_16_STEREO;



// -----------------------------------------------------------------------------
//
// @doc     EX_AUDIO_DDSI
//
// @struct  PCM_SAMPLE | Union that allows access to any of the Sample Types
//
// @field   SAMPLE_8_MONO | m8 | <t SAMPLE_8_MONO>
// @field   SAMPLE_16_MONO | m16 | <t SAMPLE_16_MONO>
// @field   SAMPLE_8_STEREO | s8 | <t SAMPLE_8_STEREO>
// @field   SAMPLE_16_STEREO | s16 | <t SAMPLE_16_STEREO>
//
// -----------------------------------------------------------------------------
typedef union {

     SAMPLE_8_MONO m8;
     SAMPLE_16_MONO m16;
     SAMPLE_8_STEREO s8;
     SAMPLE_16_STEREO s16;

} PCM_SAMPLE, *PPCM_SAMPLE;

