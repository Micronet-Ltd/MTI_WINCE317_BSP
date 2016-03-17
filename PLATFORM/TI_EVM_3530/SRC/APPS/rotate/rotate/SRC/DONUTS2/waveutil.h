//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*==========================================================================
 *
 *
 *  File:       waveutil.h
 *  Content:    Routines for dealing with sounds from resources
 *
 *
 ***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//#############################################################################
//
// Data types
//
//#############################################################################

typedef struct _tag_WAVEBUFFER  WAVEBUFFER, * PWAVEBUFFER;
typedef struct _tag_WAVESTREAM  WAVESTREAM, * PWAVESTREAM;

//#############################################################################
//
// Data methods
//
//#############################################################################
PWAVESTREAM CWaveStream_Create(ULONG, PWAVEBUFFER, BOOL fLooping);
BOOL CWaveStream_Close(PWAVESTREAM);
BOOL CWaveStream_Play(PWAVESTREAM pWaveStream);
BOOL CWaveStream_SetPan(PWAVESTREAM pWaveStream, LONG lPan);
BOOL CWaveStream_IsPlaying(PWAVESTREAM);
BOOL CWaveStream_Stop(PWAVESTREAM);
BOOL CWaveStream_SetRate(PWAVESTREAM pWaveStream, ULONG ulRate);

PWAVEBUFFER CWaveBuffer_CreateFromResource(HMODULE hModule, LPCTSTR lpName);
ULONG CWaveBuffer_GetDuration(PWAVEBUFFER);
void CWaveBuffer_Release(PWAVEBUFFER);





///////////////////////////////////////////////////////////////////////////////
//
// helper routines
//
///////////////////////////////////////////////////////////////////////////////

BOOL ParseWaveResource(HGLOBAL pvRes, WAVEFORMATEX **ppWaveHeader, BYTE **ppbWaveData, DWORD *pdwWaveSize);

#ifdef __cplusplus
}
#endif

