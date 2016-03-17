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
 *  File:       sndutil.c
 *  Content:    Routines for dealing with sounds from resources
 *
 *
 ***************************************************************************/

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

#include "sndutil.h"
#include "waveutil.h"

#define dbprintf NKDbgPrintfW

// #define VERBOSE_TRACE
#ifdef VERBOSE_TRACE
#define SNDLOG(pso,txt) NKDbgPrintfW(TEXT("[%s]: %s\r\n"), pso->pszSndName,txt)
#else
#define SNDLOG(pso,txt)
#endif



// public objects
struct _tag_SNDDEVICE
{
    ULONG ulDeviceIndex;
};


struct _tag_SNDOBJ
{
    LPCTSTR pszSndName;             // handy for tracing
    PWAVEBUFFER pSndBuffer;          // the underlying sound data
    int iAlloc;                     // number of streams.
    int iCurrent;                   // current stream
    PWAVESTREAM aStreams[1];        // list of streams

}; 

///////////////////////////////////////////////////////////////////////////////
// SndObj fns
///////////////////////////////////////////////////////////////////////////////

HSNDOBJ SndObjCreate(HSNDDEVICE pSndDevice, 
         HINSTANCE hInstance,
         LPCTSTR lpName, 
         BOOL fLooping,
         int iConcurrent
         )
{
    HSNDOBJ    pSndObject = NULL;
    PWAVEBUFFER pSndBuffer = NULL;


    pSndBuffer = CWaveBuffer_CreateFromResource(hInstance, lpName);
    if (pSndBuffer == NULL) {
        return NULL;
    }

    if (iConcurrent < 1) {
        iConcurrent = 1;
    }

    pSndObject = (HSNDOBJ) LocalAlloc(LMEM_FIXED, sizeof(SNDOBJ) + (iConcurrent-1) * sizeof(pSndObject->aStreams[0]));
    if (pSndObject == NULL) {
        CWaveBuffer_Release(pSndBuffer);
    }
    else {
        int i;

        pSndObject->pSndBuffer = pSndBuffer;
        pSndObject->iAlloc = iConcurrent;
        pSndObject->iCurrent = 0;
        for (i = 0; i < pSndObject->iAlloc; i++) {
            if (NULL == (pSndObject->aStreams[i] = CWaveStream_Create(pSndDevice->ulDeviceIndex, pSndBuffer, fLooping))) {
                // failed to open enough streams. unwind and bail.
                pSndObject->iAlloc = i;
                SndObjDestroy(pSndObject);
                pSndObject = NULL;
            }
        }
        pSndObject->pszSndName = lpName;
        SNDLOG(pSndObject, TEXT("Create"));
    }

    return pSndObject;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void 
SndObjDestroy(HSNDOBJ pSndObject)
{
    int i;

    if (pSndObject == NULL) {
        return;
    }

    SNDLOG(pSndObject, TEXT("Destroy"));

    for (i = 0; i < pSndObject->iAlloc; i++) {
        CWaveStream_Close(pSndObject->aStreams[i]);
    }
    CWaveBuffer_Release(pSndObject->pSndBuffer);
    LocalFree(pSndObject);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static PWAVESTREAM
SndiGetActiveStream(HSNDOBJ pSndObject)
{
    PWAVESTREAM pStream = NULL;
    if (pSndObject == NULL) {
        return NULL;
    }
    ASSERT(pSndObject->iAlloc == 1);
    pStream = pSndObject->aStreams[0];
    return pStream;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static PWAVESTREAM
SndiGetFreeStream(HSNDOBJ pSndObject)
{
    PWAVESTREAM pStream = NULL;
    if (pSndObject == NULL) {
        return NULL;
    }
    pStream = pSndObject->aStreams[pSndObject->iCurrent];
    if (CWaveStream_IsPlaying(pStream)) {
        CWaveStream_Stop(pStream);
    }
    if (++pSndObject->iCurrent >= pSndObject->iAlloc) {
        pSndObject->iCurrent = 0;
    }
    return pStream;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BOOL 
SndObjPlay(HSNDOBJ pSndObject)
{
    BOOL result = FALSE;
    PWAVESTREAM pStream;

    if (pSndObject == NULL) {
        return FALSE;
    }

    SNDLOG(pSndObject, TEXT("Play"));

    pStream = SndiGetFreeStream(pSndObject);
    if (pStream != NULL) {
        CWaveStream_Play(pStream);
    }
    return result;
}

BOOL SndObjPlayPanned(HSNDOBJ pSndObject, LONG lPan)
{
    BOOL result = FALSE;
    if (pSndObject != NULL) {
        PWAVESTREAM pStream;
        SNDLOG(pSndObject, TEXT("PlayPanned"));
        pStream = SndiGetFreeStream(pSndObject);
        if (pStream != NULL) {
            CWaveStream_SetPan(pStream, lPan);
            CWaveStream_Play(pStream);
        }
    }
    return result;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BOOL SndObjStop(HSNDOBJ pSndObject)
{
    int i;

    if (pSndObject != NULL) {
        SNDLOG(pSndObject, TEXT("Stop"));
        for (i = 0; i < pSndObject->iAlloc; i++) {
            CWaveStream_Stop(pSndObject->aStreams[i]);
        }
    }

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL SndObjSetPan(HSNDOBJ pSndObject, DOUBLE dPan)
{
    BOOL result = FALSE;
    if (pSndObject != NULL) {
        PWAVESTREAM pStream;
        SNDLOG(pSndObject, TEXT("SetPan"));
        pStream = SndiGetActiveStream(pSndObject);
        if (dPan < -1.0) {
            dPan = -1.0;
        }
        else
        if (dPan > +1.0) {
            dPan = +1.0;
        }

        result = CWaveStream_SetPan(pStream, (ULONG) (10000 * dPan));
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL SndObjSetRate(HSNDOBJ pSndObject, DOUBLE dRate)
{
    BOOL result = FALSE;
    if (pSndObject != NULL) {
        PWAVESTREAM pStream;
        SNDLOG(pSndObject, TEXT("SetPan"));
        pStream = SndiGetActiveStream(pSndObject);
        result = CWaveStream_SetRate(pStream, (ULONG) (0x10000 * dRate));
    }
    return result;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
DWORD SndObjGetDuration (HSNDOBJ pSndObject)
{
    if (pSndObject == NULL) {
        return 0;
    }

    SNDLOG(pSndObject, TEXT("GetDuration"));
    return CWaveBuffer_GetDuration(pSndObject->pSndBuffer);

}



HSNDDEVICE SndCreateSndDevice (ULONG ulDeviceIndex)
{
    HSNDDEVICE hDevice = NULL;
    hDevice = LocalAlloc(LMEM_FIXED, sizeof(*hDevice));
    if (hDevice != NULL) {
        hDevice->ulDeviceIndex = ulDeviceIndex;
    }
    return hDevice;
}

void SndObjCloseDevice(HSNDDEVICE pSndDevice)
{
    if (pSndDevice != NULL) {
        LocalFree(pSndDevice);
    }
}

