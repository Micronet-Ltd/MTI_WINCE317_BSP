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
 *  File:       waveutil.c
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

#include "waveutil.h"

#define dbprintf NKDbgPrintfW


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
// private objects

struct _tag_WAVEBUFFER
{
    PBYTE           pWaveData;
    DWORD           dwBufferBytes;
    BOOL            fIsResource;
    PWAVEFORMATEX   pFormat;
};

struct _tag_WAVESTREAM 
{
    HWAVEOUT    hwo;
    WAVEHDR     whdr;
};

/////////////////////////////////////////////////////////////
// CWaveBuffer methods
/////////////////////////////////////////////////////////////

void 
CWaveBuffer_Release(PWAVEBUFFER pSndBuffer)
{
    if (pSndBuffer != NULL) {
        if (! pSndBuffer->fIsResource) {
            if (pSndBuffer->pWaveData != NULL) {
                LocalFree(pSndBuffer->pWaveData);
            }
            if (pSndBuffer->pFormat != NULL) {
                LocalFree(pSndBuffer->pFormat);
            }
        }
        LocalFree(pSndBuffer);
    }
}

ULONG 
CWaveBuffer_GetDuration(PWAVEBUFFER pSndBuffer)
{
    return (1000 * pSndBuffer->dwBufferBytes) / pSndBuffer->pFormat->nAvgBytesPerSec;
}

PWAVEBUFFER 
CWaveBuffer_CreateFromResource (HMODULE hModule, LPCTSTR lpName)
{
    HRSRC hResInfo;
    HGLOBAL hResData;
    void *pvRes;
    PWAVEFORMATEX pFormat;
    PBYTE pWaveData;
    DWORD dwBufferBytes;
    PWAVEBUFFER pBuffer = NULL;

    if (    ( (hResInfo = FindResource(hModule, lpName, TEXT("WAV"))) != NULL) &&
            ( (hResData = LoadResource(hModule, hResInfo)) != NULL) &&
            ( (pvRes    = LockResource(hResData)) != NULL) &&
            (  ParseWaveResource(pvRes, &pFormat, &pWaveData, &dwBufferBytes)) &&
            ( (pBuffer = LocalAlloc(LMEM_FIXED, sizeof(*pBuffer))) != NULL))
        {
            pBuffer->dwBufferBytes = dwBufferBytes;
            pBuffer->pFormat = pFormat;
            pBuffer->pWaveData = pWaveData;
            pBuffer->fIsResource = TRUE;
        }

    return pBuffer;
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BOOL ParseWaveResource(HGLOBAL hRes, 
             WAVEFORMATEX **ppWaveHeader, 
             BYTE **ppbWaveData,
             DWORD *pcbWaveSize)
{
    DWORD *pdw;
    const DWORD *pdwEnd;
    DWORD dwRiff;
    DWORD dwType;
    DWORD dwLength;

    if (ppWaveHeader)
    *ppWaveHeader = NULL;

    if (ppbWaveData)
    *ppbWaveData = NULL;

    if (pcbWaveSize)
    *pcbWaveSize = 0;

    pdw = (DWORD *)hRes;
    dwRiff = *pdw++;
    dwLength = *pdw++;
    dwType = *pdw++;

    if (dwRiff != mmioFOURCC('R', 'I', 'F', 'F'))
    goto exit;      // not even RIFF

    if (dwType != mmioFOURCC('W', 'A', 'V', 'E'))
    goto exit;      // not a WAV

    pdwEnd = (DWORD *)((BYTE *)pdw + dwLength-4);

    while (pdw < pdwEnd)
    {
    dwType = *pdw++;
    dwLength = *pdw++;

    switch (dwType)
    {
    case mmioFOURCC('f', 'm', 't', ' '):
        if (ppWaveHeader && !*ppWaveHeader)
        {
        if (dwLength < sizeof(WAVEFORMAT))
            goto exit;      // not a WAV

        *ppWaveHeader = (WAVEFORMATEX *)pdw;

        if ((!ppbWaveData || *ppbWaveData) &&
            (!pcbWaveSize || *pcbWaveSize))
        {
            return TRUE;
        }
        }
        break;

    case mmioFOURCC('d', 'a', 't', 'a'):
        if ((ppbWaveData && !*ppbWaveData) ||
        (pcbWaveSize && !*pcbWaveSize))
        {
        if (ppbWaveData)
            *ppbWaveData = (LPBYTE)pdw;

        if (pcbWaveSize)
            *pcbWaveSize = dwLength;

        if (!ppWaveHeader || *ppWaveHeader)
            return TRUE;
        }
        break;
    }

    pdw = (DWORD *)((BYTE *)pdw + ((dwLength+1)&~1));
    }

exit:
    return FALSE;
}


PWAVESTREAM CWaveStream_Create(ULONG ulDeviceIndex, PWAVEBUFFER pSndBuffer, BOOL fLooping)
{
    MMRESULT mr;

    PWAVESTREAM pStream = LocalAlloc(LMEM_FIXED, sizeof(*pStream));
    if (pStream != NULL) {
        memset(&pStream->whdr, 0, sizeof(WAVEHDR));
        pStream->whdr.dwBufferLength = pSndBuffer->dwBufferBytes;
        pStream->whdr.lpData = pSndBuffer->pWaveData;
        if (fLooping) {
            pStream->whdr.dwFlags |=  (WHDR_BEGINLOOP | WHDR_ENDLOOP);
        }
        mr = waveOutOpen(&pStream->hwo, ulDeviceIndex, pSndBuffer->pFormat, 0, 0, CALLBACK_NULL);
        if (mr == MMSYSERR_NOERROR) {
            mr = waveOutPrepareHeader(pStream->hwo, &pStream->whdr, sizeof(WAVEHDR));
            if (mr != MMSYSERR_NOERROR) {
                dbprintf (TEXT("CWaveStream_Create - waveOutPrepareHeader failed, mr = %2d\r\n"), mr);
            }
        }
        else {
            dbprintf (TEXT("CWaveStream_Create - waveOutOpen failed, mr = %2d\r\n"), mr);
        }
    }
    return pStream;
}

BOOL CWaveStream_Play (PWAVESTREAM pWaveStream)
{
    MMRESULT mr;
    mr = waveOutWrite(pWaveStream->hwo, &pWaveStream->whdr, sizeof(WAVEHDR));
    if (mr != MMSYSERR_NOERROR) {
        dbprintf (TEXT("CWaveStream_Play - waveOutWrite failed, mr = %2d\r\n"), mr);
    }

    return mr == MMSYSERR_NOERROR;
}

BOOL CWaveStream_Close(PWAVESTREAM pWaveStream)
{   
    MMRESULT mr = MMSYSERR_NOERROR;
    int retries = 0;

    CWaveStream_Stop(pWaveStream);

    while (1) {
        mr = waveOutUnprepareHeader(pWaveStream->hwo, &pWaveStream->whdr, sizeof(WAVEHDR));
        if (mr != MMSYSERR_NOERROR) {
            dbprintf (TEXT("CWaveStream_Close - waveOutUnprepareHeader failed, mr = %2d\r\n"), mr);
        }

        mr = waveOutClose(pWaveStream->hwo);
        if (mr == MMSYSERR_NOERROR) {
            break;
        }
        else {
            dbprintf (TEXT("CWaveStream_Close - waveOutClose failed, mr = %2d, retries = %2d\r\n"), mr, retries);
            if (retries++ > 10) {
                break;
            }
        }
        Sleep(2);
    }

    return mr == MMSYSERR_NOERROR;
}


BOOL CWaveStream_IsPlaying(PWAVESTREAM pWaveStream)
{
    // WHDR_INQUEUE is set on waveOutWrite and is cleared when the buffer is completely done.
    // Note: most drivers set WHDR_DONE set well ahead of WHDR_INQUEUE, while the
    // driver is still operating on the buffer. Furthermore, the wave API rejects buffers
    // that are re-submitted with WHDR_INQUEUE still set.
    return (pWaveStream->whdr.dwFlags & WHDR_INQUEUE) != 0;
}


BOOL CWaveStream_Stop(PWAVESTREAM pWaveStream)
{
    BOOL result = FALSE;
    MMRESULT mr = waveOutReset(pWaveStream->hwo);
    if (mr == MMSYSERR_NOERROR) {
        result = TRUE;
    }
    else {
        dbprintf(TEXT("CWaveStream_Stop: waveOutReset failed, mr=%d\r\n"));
    }
    return result;
}

BOOL CWaveStream_SetRate(PWAVESTREAM pWaveStream, ULONG ulRate)
{
    BOOL result = FALSE;
    MMRESULT mr = waveOutSetPlaybackRate(pWaveStream->hwo, ulRate);
    if (mr == MMSYSERR_NOERROR) {
        result = TRUE;
    }
    else {
        dbprintf(TEXT("CWaveStream_SetRate: waveOutSetPlaybackRate failed, mr=%d\r\n"));
    }
    return result;
}

BOOL CWaveStream_SetPan(PWAVESTREAM pWaveStream, LONG lPan)
{
    MMRESULT mr;
    DWORD dwVolume;
    if (lPan < 0) {
        // -10000 -> set right volume to max, left volume per pan
        DWORD dwLeft = ((10000 - ((DWORD) (-lPan))) * 0xffff) / 10000;
        dwVolume = dwLeft | 0xffff0000;
    }
    else if (lPan > 0) {
        // +10000 -> set left volume to max, right volume per pan
        DWORD dwRight = ((10000 - ((DWORD) ( lPan))) * 0xffff) / 10000;
        dwVolume = (dwRight << 16) | 0xffff;
    }
    else {
        dwVolume = 0xFFFFFFFF;
    }

    mr = waveOutSetVolume(pWaveStream->hwo, dwVolume);
    if (mr != MMSYSERR_NOERROR) {
        dbprintf(TEXT("CWaveStream_SetPan: waveOutSetVolume failed, mr=%d\r\n"));
    }
    return mr == MMSYSERR_NOERROR;
}
