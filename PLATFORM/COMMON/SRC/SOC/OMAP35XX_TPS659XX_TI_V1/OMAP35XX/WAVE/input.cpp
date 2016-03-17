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
#include <windows.h>
#include <wavedev.h>
#include <mmreg.h>
#include <mmddk.h>
#include "debug.h"
#include "wavemain.h"
#include "strmctxt.h"
#include "strmmgr.h"



//------------------------------------------------------------------------------
//
//  Function: Open
//
//  Open input stream context
//

HRESULT InputStreamContext::Open(
    CStreamManager *pStreamManager,
    LPWAVEOPENDESC lpWOD,
    DWORD dwFlags)
{
    HRESULT Result;

    Result = WaveStreamContext::Open(pStreamManager, lpWOD, dwFlags);

    return Result;
}


//------------------------------------------------------------------------------
//
//  InputStreamContext::MapGain
//
//  software input gain control is not yet implemented
//

// Channel 0 is the left channel, which is the low 16-bits of volume data
DWORD
InputStreamContext::MapGain(DWORD StreamGain, DWORD Channel)
{
    return 0x10000;
}

//------------------------------------------------------------------------------
//
//  Function: InputStreamContext::SetRate
//
// Init input m_DeltaT with (HWSampleRate/SampleRate) calculated in fixed point form.
// Note that we need to hold the result in a 64-bit value until we're done shifting,
// since the result of the multiply will overflow 32 bits for sample rates greater than
// or equal to the hardware's sample rate.
DWORD InputStreamContext::SetRate(DWORD dwMultiplier)
{
    UINT64 Delta;

	RETAILMSG(WAVE_AUDIO_DBG, (L"InputStreamContext::SetRate, dwMultiplier =%d\r\n", dwMultiplier));
    m_dwMultiplier = dwMultiplier;

    // In order to pass the CETK tests we need to improve the accuracy
    // of our ratio between the HWSampleRate and the Stream SampleRate.
    // We'll do this by setting our m_DeltaT resolution to 4.28 format
    //

    Delta = (m_WaveFormat.nSamplesPerSec * m_dwMultiplier);
    Delta >>= 16;
    Delta = ((UINT32)(((1i64<<32)/Delta)+1));

    // UNDONE:
    //  This should be a static variable of the input class
    //
    Delta = (Delta * m_pStreamManager->get_CurrentSampleRate());
    Delta >>= DELTAINT;

    m_DeltaT = (DWORD)Delta;

    return MMSYSERR_NOERROR;
}

//------------------------------------------------------------------------------
//
//  Function: Stop()
//
//  stop input stream
//

DWORD InputStreamContext::Stop()
{
    // Stop the stream
    WaveStreamContext::Stop();

    // Return any partially filled buffers to the client
    if ((m_lpWaveHdrCurrent) && (m_lpWaveHdrCurrent->dwBytesRecorded>0))
    {
        GetNextBuffer();
    }

    return MMSYSERR_NOERROR;
}

unsigned int
InputStreamContext::GetStreamDirection()
{
    return CHardwareAudioBridge::kInput;
}

//------------------------------------------------------------------------------
//
//  Function: InputStreamContext::Render2
//
//  resample and transform incoming audio data to wave capture format
//

#if (INCHANNELS==2)
PBYTE InputStreamContext::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender)
{
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;

    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    PCM_TYPE SampleType = m_SampleType;

    LONG CurrSamp0 = m_CurrSamp[0];
    LONG PrevSamp0 = m_PrevSamp[0];
    LONG CurrSamp1 = m_CurrSamp[1];
    LONG PrevSamp1 = m_PrevSamp[1];
    LONG InSamp0;
    LONG InSamp1;

    if (bDSPRender == FALSE)
        {
        return RenderRawData(pBuffer, pBufferEnd, pBufferLast);
        }

    for (;;)
    {
        // Make sure we have a place to put the data
        if (pCurrData>=pCurrDataEnd)
        {
            goto Exit;
        }

        // Get the next sample
        while (CurrT >= DELTA_OVERFLOW)
        {
            if (pBuffer>=pBufferEnd)
            {
                goto Exit;
            }

            PrevSamp0 = CurrSamp0;
            PrevSamp1 = CurrSamp1;

            CurrSamp0 = ((HWSAMPLE *)pBuffer)[0];
            CurrSamp1 = ((HWSAMPLE *)pBuffer)[1];
            pBuffer += 2*sizeof(HWSAMPLE);

            CurrT -= DELTA_OVERFLOW;
        }

        InSamp0 = (PrevSamp0 + ((CurrT * (CurrSamp0 - PrevSamp0)) >> DELTAFRAC));
        InSamp1 = (PrevSamp1 + ((CurrT * (CurrSamp1 - PrevSamp1)) >> DELTAFRAC));
        CurrT += DeltaT;

        // Apply input gain
        // InSamp0 = (InSamp0 * fxpGain[0]) >> 16;
        // InSamp1 = (InSamp1 * fxpGain[1]) >> 16;

        PPCM_SAMPLE pSampleDest = (PPCM_SAMPLE)pCurrData;
#if (HARDWARE_INCHANNELS==2)
        switch (m_SampleType)
        {
        case PCM_TYPE_M8:
        default:
            pSampleDest->m8.sample = (UINT8)( ((InSamp0+InSamp1) >> 9) + 128);
            pCurrData  += 1;
            break;

        case PCM_TYPE_S8:
            pSampleDest->s8.sample_left  = (UINT8)((InSamp0 >> 8) + 128);
            pSampleDest->s8.sample_right = (UINT8)((InSamp1 >> 8) + 128);
            pCurrData  += 2;
            break;

        case PCM_TYPE_M16:
            pSampleDest->m16.sample = (INT16)((InSamp0+InSamp1)>>1);
            pCurrData  += 2;
            break;

        case PCM_TYPE_S16:
            pSampleDest->s16.sample_left  = (INT16)InSamp0;
            pSampleDest->s16.sample_right = (INT16)InSamp1;
            pCurrData  += 4;
            break;
        }
#else
        // The Buffer given by the dataport is a stereo buffer, but only has
        // left channel data in it.
        switch (m_SampleType)
        {
        case PCM_TYPE_M8:
        default:
            pSampleDest->m8.sample = (UINT8)( ((InSamp0) >> 8) + 128);
            pCurrData  += 1;
            break;

        case PCM_TYPE_S8:
            pSampleDest->s8.sample_left  = (UINT8)((InSamp0 >> 8) + 128);
            pSampleDest->s8.sample_right = (UINT8)((InSamp0 >> 8) + 128);
            pCurrData  += 2;
            break;

        case PCM_TYPE_M16:
            pSampleDest->m16.sample = (INT16)InSamp0;
            pCurrData  += 2;
            break;

        case PCM_TYPE_S16:
            pSampleDest->s16.sample_left  = (INT16)InSamp0;
            pSampleDest->s16.sample_right = (INT16)InSamp0;
            pCurrData  += 4;
            break;
        }
#endif
    }

Exit:
    m_lpWaveHdrCurrent->dwBytesRecorded += (pCurrData-m_lpCurrData);
    m_dwByteCount += (pCurrData-m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_CurrSamp[0] = CurrSamp0;
    m_PrevSamp[1] = PrevSamp1;
    m_CurrSamp[1] = CurrSamp1;
    return pBuffer;
}
#else
PBYTE InputStreamContext::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender)
{
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;

    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;

    LONG CurrSamp0 = m_CurrSamp[0];
    LONG PrevSamp0 = m_PrevSamp[0];
    LONG InSamp0;

    PCM_TYPE SampleType = m_SampleType;

    if (bDSPRender == FALSE)
        {
        return RenderRawData(pBuffer, pBufferEnd, pBufferLast);
        }

    for (;;)
    {
        // Make sure we have a place to put the data
        if (pCurrData>=pCurrDataEnd)
        {
            goto Exit;
        }

        // Get the next sample
        while (CurrT >= DELTA_OVERFLOW)
        {
            if (pBuffer>=pBufferEnd)
            {
                goto Exit;
            }

            PrevSamp0 = CurrSamp0;
            CurrSamp0 = *(HWSAMPLE *)pBuffer;
            pBuffer += sizeof(HWSAMPLE);

            CurrT -= DELTA_OVERFLOW;
        }

        InSamp0 = (PrevSamp0 + ((CurrT * (CurrSamp0 - PrevSamp0)) >> DELTAFRAC));

        CurrT += DeltaT;

        // Apply input gain?
        // InSamp0 = (InSamp0 * fxpGain) >> 16;

        PPCM_SAMPLE pSampleDest = (PPCM_SAMPLE)pCurrData;
        switch (m_SampleType)
        {
        case PCM_TYPE_M8:
        default:
            pSampleDest->m8.sample = (UINT8)((InSamp0 >> 8) + 128);
            pCurrData  += 1;
            break;

        case PCM_TYPE_S8:
            pSampleDest->s8.sample_left  = (UINT8)((InSamp0 >> 8) + 128);
            pSampleDest->s8.sample_right = (UINT8)((InSamp0 >> 8) + 128);
            pCurrData  += 2;
            break;

        case PCM_TYPE_M16:
            pSampleDest->m16.sample = (INT16)InSamp0;
            pCurrData  += 2;
            break;

        case PCM_TYPE_S16:
            pSampleDest->s16.sample_left  = (INT16)InSamp0;
            pSampleDest->s16.sample_right = (INT16)InSamp0;
            pCurrData  += 4;
            break;
        }
    }

Exit:
    m_lpWaveHdrCurrent->dwBytesRecorded += (pCurrData-m_lpCurrData);
    m_dwByteCount += (pCurrData-m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_CurrSamp[0] = CurrSamp0;
    return pBuffer;
}
#endif
