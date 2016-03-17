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
//  Open output stream context
//

HRESULT
OutputStreamContext::Open(
    CStreamManager *pStreamManager,
    LPWAVEOPENDESC lpWOD,
    DWORD dwFlags)
{
    HRESULT Result;

    Result = WaveStreamContext::Open(pStreamManager, lpWOD, dwFlags);

    if (Result==MMSYSERR_NOERROR)
    {
        // Note: Output streams should be initialized in the run state.
        Run();
    }

    return Result;
}

//------------------------------------------------------------------------------
//
//  Function: Reset()
//
//  eset output stream context
//

DWORD
OutputStreamContext::Reset()
{
    HRESULT Result;

    Result = WaveStreamContext::Reset();

    if (Result==MMSYSERR_NOERROR)
    {
        // Note: Output streams should be reset to the run state.
        Run();
    }

    return Result;
};

unsigned int
OutputStreamContext::GetStreamDirection()
{
    return CHardwareAudioBridge::kOutput;
}

//------------------------------------------------------------------------------
//
//  Function: SetRate
//
//  Init m_DeltaT with (SampleRate/HWSampleRate) calculated in the appropriate fixed point form.
//  Note that we need to hold the result in a 64-bit value until we're done shifting,
//  since the result of the multiply will overflow 32 bits for sample rates greater than
//  or equal to the hardware's sample rate.
//

DWORD
OutputStreamContext::SetRate(DWORD dwMultiplier)
{
    float delta;
    int dec, fract;
    float mult;

	RETAILMSG(WAVE_AUDIO_DBG, (L"OutputStreamContext::SetRate, dwMultiplier =%d\r\n", dwMultiplier));

    m_dwMultiplier = dwMultiplier;
    mult = (float)m_dwMultiplier / (float)((DWORD)0x10000);
    delta = (float)((float)m_WaveFormat.nSamplesPerSec * mult)/(float)m_pStreamManager->get_CurrentSampleRate();
    dec = (int)delta * (1<<15);
    fract = (int)((delta - (float)(int)delta) * (1 << 15));
    m_DeltaT = dec + fract;

    return MMSYSERR_NOERROR;
}


//------------------------------------------------------------------------------
//
//  Function: OutputStreamContextM8::Render2
//
//  StreamContext to reformat Mono/8bit samples to 16bit/stereo.
//  Also resamples to 44.1khz and checks saturation
//

PBYTE
OutputStreamContextM8::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = m_CurrSamp[0];
    LONG PrevSamp0 = m_PrevSamp[0];
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain[2];

	RETAILMSG(0, (L"WAV:+%S, gain0 = 0x%X, gain1 = 0x%X \r\n",__FUNCTION__, m_fxpGain[0], m_fxpGain[1]));

    fxpGain[0] = m_fxpGain[0];
    fxpGain[1] = m_fxpGain[1];

   // Test
   // bDSPRender = FALSE;

    if (bDSPRender == FALSE)
        {
        return RenderRawData(pBuffer, pBufferEnd, pBufferLast);
        }

    while (pBuffer < pBufferEnd)
    {
        while (CurrT >= DELTA_OVERFLOW)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= DELTA_OVERFLOW;

            PrevSamp0 = CurrSamp0;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 = (LONG)pSampleSrc->m8.sample;
            CurrSamp0 = (CurrSamp0 - 128) << 8;
            pCurrData+=1;
        }

        LONG OutSamp0;
        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> DELTAFRAC);
        CurrT += DeltaT;

#if (OUTCHANNELS==2)
        LONG OutSamp1;
        OutSamp1=OutSamp0;
        OutSamp0 = (OutSamp0 * fxpGain[0]) >> VOLSHIFT;
        OutSamp1 = (OutSamp1 * fxpGain[1]) >> VOLSHIFT;
        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
            OutSamp1 += ((HWSAMPLE *)pBuffer)[1];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
            if (OutSamp1>AUDIO_SAMPLE_MAX)
            {
                OutSamp1=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp1<AUDIO_SAMPLE_MIN)
            {
                OutSamp1=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        ((HWSAMPLE *)pBuffer)[1] = (HWSAMPLE)OutSamp1;
        pBuffer += 2*sizeof(HWSAMPLE);
#else
        OutSamp0 = (OutSamp0 * fxpGain[0]) >> VOLSHIFT;

        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        pBuffer += sizeof(HWSAMPLE);
#endif
    }

    Exit:

    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_CurrSamp[0] = CurrSamp0;

	RETAILMSG(0, (L"WAV:-%S\r\n",__FUNCTION__));
    return pBuffer;
}

//------------------------------------------------------------------------------
//
//  Function: OutputStreamContextM16::Render2
//
//  StreamContext to reformat Mono/16bit samples to 16bit/stereo.
//  Also resamples to 44.1khz and checks saturation
//

PBYTE
OutputStreamContextM16::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = m_CurrSamp[0];
    LONG PrevSamp0 = m_PrevSamp[0];
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain[2];

	RETAILMSG(0, (L"WAV:+%S, gain0 = 0x%X, gain1 = 0x%X [r0]\r\n",__FUNCTION__, m_fxpGain[0], m_fxpGain[1]));

    fxpGain[0] = m_fxpGain[0];
    fxpGain[1] = m_fxpGain[1];
    LONG OutSamp0;

    //bDSPRender = FALSE;

    if (bDSPRender == FALSE)
        {
        return RenderRawData(pBuffer, pBufferEnd, pBufferLast);
        }

    while (pBuffer < pBufferEnd)
    {
        while (CurrT >= DELTA_OVERFLOW)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= DELTA_OVERFLOW;

            PrevSamp0 = CurrSamp0;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 = (LONG)pSampleSrc->m16.sample;
            pCurrData+=2;
        }

        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> DELTAFRAC);
        CurrT += DeltaT;

        DEBUGMSG(ZONE_VERBOSE, (TEXT("PrevSamp0=0x%x, CurrSamp0=0x%x, CurrT=0x%x, OutSamp0=0x%x\r\n"), PrevSamp0,CurrSamp0,CurrT,OutSamp0));

#if (OUTCHANNELS==2)
        LONG OutSamp1;
        OutSamp1=OutSamp0;
        OutSamp0 = (OutSamp0 * fxpGain[0]) >> VOLSHIFT;
        OutSamp1 = (OutSamp1 * fxpGain[1]) >> VOLSHIFT;
        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
            OutSamp1 += ((HWSAMPLE *)pBuffer)[1];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
            if (OutSamp1>AUDIO_SAMPLE_MAX)
            {
                OutSamp1=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp1<AUDIO_SAMPLE_MIN)
            {
                OutSamp1=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        ((HWSAMPLE *)pBuffer)[1] = (HWSAMPLE)OutSamp1;
        pBuffer += 2*sizeof(HWSAMPLE);
#else
        OutSamp0 = (OutSamp0 * fxpGain[0]) >> VOLSHIFT;
        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        pBuffer += sizeof(HWSAMPLE);
#endif
    }

    Exit:
    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_CurrSamp[0] = CurrSamp0;

	RETAILMSG(0, (L"WAV:-%S\r\n",__FUNCTION__));
    return pBuffer;
}

#if (OUTCHANNELS==2)

//------------------------------------------------------------------------------
//
//  Function: OutputStreamContextS8::Render2
//
//  StreamContext to reformat stereo/8bit samples to 16bit/stereo.
//  Also resamples to 44.1khz and checks saturation
//
//  Just selects between mono and stereo case, because output can
//  switch on the fly
//
//

PBYTE
OutputStreamContextS8::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender)
{
	RETAILMSG(0, (L"WAV:+%S\r\n",__FUNCTION__));

    // For test
	// bDSPRender = FALSE;

    if (bDSPRender == FALSE)
        {
        return RenderRawData(pBuffer, pBufferEnd, pBufferLast);
        }
    else if (m_bMono)
        {
        return Render2Mono(pBuffer,pBufferEnd,pBufferLast, bDSPRender);
        }
    else
        {
        return Render2Stereo(pBuffer,pBufferEnd,pBufferLast, bDSPRender);
        }

	RETAILMSG(0, (L"WAV:-%S\r\n",__FUNCTION__));
}

//------------------------------------------------------------------------------
//
//  Function: OutputStreamContextS8::Render2Stereo
//
//  StreamContext to reformat stereo/8bit samples to 16bit/stereo.
//  Also resamples to 44.1khz and checks saturation
//
//

PBYTE
OutputStreamContextS8::Render2Stereo(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = m_CurrSamp[0];
    LONG CurrSamp1 = m_CurrSamp[1];
    LONG PrevSamp0 = m_PrevSamp[0];
    LONG PrevSamp1 = m_PrevSamp[1];
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain[2];
    LONG OutSamp0;
    LONG OutSamp1;

	RETAILMSG(0, (L"WAV:+%S, gain0 = 0x%X, gain1 = 0x%X \r\n",__FUNCTION__, m_fxpGain[0], m_fxpGain[1]));

    fxpGain[0] = m_fxpGain[0];
    fxpGain[1] = m_fxpGain[1];

    while (pBuffer < pBufferEnd)
    {
        while (CurrT >= DELTA_OVERFLOW)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= DELTA_OVERFLOW;

            PrevSamp0 = CurrSamp0;
            PrevSamp1 = CurrSamp1;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 =  (LONG)pSampleSrc->s8.sample_left;
            CurrSamp0 = (CurrSamp0 - 128) << 8;
            CurrSamp1 = (LONG)pSampleSrc->s8.sample_right;
            CurrSamp1 = (CurrSamp1 - 128) << 8;
            pCurrData+=2;
        }

        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> DELTAFRAC);
        OutSamp0 = (OutSamp0 * fxpGain[0]) >> VOLSHIFT;

        OutSamp1 = PrevSamp1 + (((CurrSamp1 - PrevSamp1) * CurrT) >> DELTAFRAC);
        OutSamp1 = (OutSamp1 * fxpGain[1]) >> VOLSHIFT;
        CurrT += DeltaT;

        DEBUGMSG(ZONE_VERBOSE, (TEXT("PrevSamp0=0x%x, CurrSamp0=0x%x, CurrT=0x%x, OutSamp0=0x%x\r\n"), PrevSamp0,CurrSamp0,CurrT,OutSamp0));

        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
            OutSamp1 += ((HWSAMPLE *)pBuffer)[1];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
            if (OutSamp1>AUDIO_SAMPLE_MAX)
            {
                OutSamp1=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp1<AUDIO_SAMPLE_MIN)
            {
                OutSamp1=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        ((HWSAMPLE *)pBuffer)[1] = (HWSAMPLE)OutSamp1;

        pBuffer += 2*sizeof(HWSAMPLE);

    }

    Exit:
    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_PrevSamp[1] = PrevSamp1;
    m_CurrSamp[0] = CurrSamp0;
    m_CurrSamp[1] = CurrSamp1;

	RETAILMSG(0, (L"WAV:-%S\r\n",__FUNCTION__));
    return pBuffer;
}

//------------------------------------------------------------------------------
//
//  Function: OutputStreamContextS8::Render2Mono
//
//  StreamContext to reformat stereo/8bit samples to 16bit/stereo.
//  Also resamples to 44.1khz and checks saturation
//
//  reformats stereo stream to Mono for output on single speaker
//
//

PBYTE
OutputStreamContextS8::Render2Mono(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = (m_CurrSamp[0]+m_CurrSamp[1])>>1;
    LONG PrevSamp0 = (m_PrevSamp[0]+m_PrevSamp[1])>>1;
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain[2];
    LONG OutSamp0;
    LONG OutSamp1;

	RETAILMSG(0, (L"WAV:+%S, gain0 = 0x%X, gain1 = 0x%X \r\n",__FUNCTION__, m_fxpGain[0], m_fxpGain[1]));

    fxpGain[0] = m_fxpGain[0];
    fxpGain[1] = m_fxpGain[1];

    while (pBuffer < pBufferEnd)
    {
        while (CurrT >= DELTA_OVERFLOW)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= DELTA_OVERFLOW;

            PrevSamp0 = CurrSamp0;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 =  (LONG)pSampleSrc->s8.sample_left;
            CurrSamp0 += (LONG)pSampleSrc->s8.sample_right;
            CurrSamp0 = (CurrSamp0 - 256) << 7;
            pCurrData+=2;
        }

        OutSamp0 =
        OutSamp1 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> DELTAFRAC);
        OutSamp0 = (OutSamp0 * fxpGain[0]) >> VOLSHIFT;
        OutSamp1 = (OutSamp1 * fxpGain[1]) >> VOLSHIFT;
        CurrT += DeltaT;

        DEBUGMSG(ZONE_VERBOSE, (TEXT("PrevSamp0=0x%x, CurrSamp0=0x%x, CurrT=0x%x, OutSamp0=0x%x\r\n"), PrevSamp0,CurrSamp0,CurrT,OutSamp0));

        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
            OutSamp1 += ((HWSAMPLE *)pBuffer)[1];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
            if (OutSamp1>AUDIO_SAMPLE_MAX)
            {
                OutSamp1=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp1<AUDIO_SAMPLE_MIN)
            {
                OutSamp1=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        ((HWSAMPLE *)pBuffer)[1] = (HWSAMPLE)OutSamp1;

        pBuffer += 2*sizeof(HWSAMPLE);

    }

    Exit:
    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_PrevSamp[1] = PrevSamp0;
    m_CurrSamp[0] = CurrSamp0;
    m_CurrSamp[1] = CurrSamp0;

	RETAILMSG(0, (L"WAV:-%S\r\n",__FUNCTION__));
    return pBuffer;
}


//------------------------------------------------------------------------------
//
//  Function: OutputStreamContextS16::Render2
//
//  StreamContext to reformat stereo/16bit samples to 16bit/stereo.
//  Also resamples to 44.1khz and checks saturation
//
//  Just selects between mono and stereo case, because output can
//  switch on the fly
//

PBYTE
OutputStreamContextS16::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender)
{
	RETAILMSG(0, (L"WAV:+%S\r\n",__FUNCTION__));

    // For test!
	// bDSPRender = FALSE;

    if (bDSPRender == FALSE)
        {
        return RenderRawData(pBuffer, pBufferEnd, pBufferLast);
        }
    else if (m_bMono)
        {
        return Render2Mono(pBuffer,pBufferEnd,pBufferLast, bDSPRender);
        }
    else
        {
        return Render2Stereo(pBuffer,pBufferEnd,pBufferLast, bDSPRender);
        }

	RETAILMSG(0, (L"WAV:-%S\r\n",__FUNCTION__));
}

//------------------------------------------------------------------------------
//
//  Function: OutputStreamContextS16::Render2Stereo
//
//  StreamContext to reformat stereo/16bit samples to 16bit/stereo.
//  Also resamples to 44.1khz and checks saturation
//
//

PBYTE
OutputStreamContextS16::Render2Stereo(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = m_CurrSamp[0];
    LONG CurrSamp1 = m_CurrSamp[1];
    LONG PrevSamp0 = m_PrevSamp[0];
    LONG PrevSamp1 = m_PrevSamp[1];
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain[2];
    LONG OutSamp0;
    LONG OutSamp1;

	RETAILMSG(0, (L"WAV:+%S, gain0 = 0x%X, gain1 = 0x%X \r\n",__FUNCTION__, m_fxpGain[0], m_fxpGain[1]));

    fxpGain[0] = m_fxpGain[0];
    fxpGain[1] = m_fxpGain[1];

    while (pBuffer < pBufferEnd)
    {
        while (CurrT >= DELTA_OVERFLOW)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= DELTA_OVERFLOW;

            PrevSamp0 = CurrSamp0;
            PrevSamp1 = CurrSamp1;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 = (LONG)pSampleSrc->s16.sample_left;
            CurrSamp1 = (LONG)pSampleSrc->s16.sample_right;
            pCurrData+=4;
        }

        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> DELTAFRAC);
        OutSamp0 = (OutSamp0 * fxpGain[0]) >> VOLSHIFT;
        OutSamp1 = PrevSamp1 + (((CurrSamp1 - PrevSamp1) * CurrT) >> DELTAFRAC);
        OutSamp1 = (OutSamp1 * fxpGain[1]) >> VOLSHIFT;
        CurrT += DeltaT;

        DEBUGMSG(ZONE_VERBOSE, (TEXT("PrevSamp0=0x%x, CurrSamp0=0x%x, CurrT=0x%x, OutSamp0=0x%x\r\n"), PrevSamp0,CurrSamp0,CurrT,OutSamp0));

        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
            OutSamp1 += ((HWSAMPLE *)pBuffer)[1];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
            if (OutSamp1>AUDIO_SAMPLE_MAX)
            {
                OutSamp1=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp1<AUDIO_SAMPLE_MIN)
            {
                OutSamp1=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        ((HWSAMPLE *)pBuffer)[1] = (HWSAMPLE)OutSamp1;

        pBuffer += 2*sizeof(HWSAMPLE);
    }

    Exit:
    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_PrevSamp[1] = PrevSamp1;
    m_CurrSamp[0] = CurrSamp0;
    m_CurrSamp[1] = CurrSamp1;

	RETAILMSG(0, (L"WAV:-%S\r\n",__FUNCTION__));
    return pBuffer;
}

//------------------------------------------------------------------------------
//
//  Function: OutputStreamContextS16::Render2Mono
//
//  StreamContext to reformat stereo/16bit samples to 16bit/stereo.
//  Also resamples to 44.1khz and checks saturation
//
//  resamples stereo output to mono for output on single speaker
//

PBYTE
OutputStreamContextS16::Render2Mono(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = (m_CurrSamp[0]+m_CurrSamp[1])>>1;
    LONG PrevSamp0 = (m_PrevSamp[0]+m_PrevSamp[1])>>1;
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain[2];
    LONG OutSamp0;
    LONG OutSamp1;

	RETAILMSG(0, (L"WAV:+%S, gain0 = 0x%X, gain1 = 0x%X \r\n",__FUNCTION__, m_fxpGain[0], m_fxpGain[1]));

    fxpGain[0] = m_fxpGain[0];
    fxpGain[1] = m_fxpGain[1];

    while (pBuffer < pBufferEnd)
    {
        while (CurrT >= DELTA_OVERFLOW)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= DELTA_OVERFLOW;

            PrevSamp0 = CurrSamp0;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 =  (LONG)pSampleSrc->s16.sample_left;
            CurrSamp0 += (LONG)pSampleSrc->s16.sample_right;
            CurrSamp0 = CurrSamp0>>1;

            pCurrData+=4;
        }

        OutSamp0 =
        OutSamp1 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> DELTAFRAC);
        OutSamp0 = (OutSamp0 * fxpGain[0]) >> VOLSHIFT;
        OutSamp1 = (OutSamp1 * fxpGain[1]) >> VOLSHIFT;
        CurrT += DeltaT;

        DEBUGMSG(ZONE_VERBOSE, (TEXT("PrevSamp0=0x%x, CurrSamp0=0x%x, CurrT=0x%x, OutSamp0=0x%x\r\n"), PrevSamp0,CurrSamp0,CurrT,OutSamp0));

        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
            OutSamp1 += ((HWSAMPLE *)pBuffer)[1];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
            if (OutSamp1>AUDIO_SAMPLE_MAX)
            {
                OutSamp1=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp1<AUDIO_SAMPLE_MIN)
            {
                OutSamp1=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        ((HWSAMPLE *)pBuffer)[1] = (HWSAMPLE)OutSamp1;

        pBuffer += 2*sizeof(HWSAMPLE);
    }

    Exit:
    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_PrevSamp[1] = PrevSamp0;
    m_CurrSamp[0] = CurrSamp0;
    m_CurrSamp[1] = CurrSamp0;

	RETAILMSG(0, (L"WAV:-%S\r\n",__FUNCTION__));
    return pBuffer;
}

#else

//------------------------------------------------------------------------------
//
//  Function:
//
//  StreamContext to reformat stereo/8bit samples to 16bit/mono.
//  Also resamples to 44.1khz and checks saturation
//

PBYTE
OutputStreamContextS8::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, , BOOL bDSPRender)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = m_CurrSamp[0];
    LONG PrevSamp0 = m_PrevSamp[0];
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain[2];
    LONG OutSamp0;

	RETAILMSG(0, (L"WAV:+%S, gain0 = 0x%X, gain1 = 0x%X \r\n",__FUNCTION__, m_fxpGain[0], m_fxpGain[1]));

    fxpGain[0] = m_fxpGain[0];
    fxpGain[1] = m_fxpGain[1];

    if (bDSPRender == FALSE)
        {
        return RenderRawData(pBuffer, pBufferEnd, pBufferLast);
        }

    while (pBuffer < pBufferEnd)
    {
        while (CurrT >= DELTA_OVERFLOW)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= DELTA_OVERFLOW;

            PrevSamp0 = CurrSamp0;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 =  (LONG)pSampleSrc->s8.sample_left;
            CurrSamp0 += (LONG)pSampleSrc->s8.sample_right;
            CurrSamp0 = (CurrSamp0 - 256) << 7;
            pCurrData+=2;
        }

        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> DELTAFRAC);
        OutSamp0 = (OutSamp0 * fxpGain) >> VOLSHIFT;
        CurrT += DeltaT;

        DEBUGMSG(ZONE_VERBOSE, (TEXT("PrevSamp0=0x%x, CurrSamp0=0x%x, CurrT=0x%x, OutSamp0=0x%x\r\n"), PrevSamp0,CurrSamp0,CurrT,OutSamp0));

        if (pBuffer < pBufferLast)
        {
            OutSamp0 += *(HWSAMPLE *)pBuffer;
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        *(HWSAMPLE *)pBuffer = (HWSAMPLE)OutSamp0;

        pBuffer += sizeof(HWSAMPLE);
    }

    Exit:
    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_CurrSamp[0] = CurrSamp0;

	RETAILMSG(0, (L"WAV:-%S\r\n",__FUNCTION__));
    return pBuffer;
}

//------------------------------------------------------------------------------
//
//  Function: OutputStreamContextS16::Render2
//
//  StreamContext to reformat stereo/16bit samples to 16bit/mono.
//  Also resamples to 44.1khz and checks saturation
//
//

PBYTE
OutputStreamContextS16::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, , BOOL bDSPRender)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = m_CurrSamp[0];
    LONG PrevSamp0 = m_PrevSamp[0];
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain;
    LONG OutSamp0;

	RETAILMSG(0, (L"WAV:+%S, gain0 = 0x%X\r\n",__FUNCTION__, m_fxpGain[0]));

    fxpGain = m_fxpGain[0];

    if (bDSPRender == FALSE)
        {
        return RenderRawData(pBuffer, pBufferEnd, pBufferLast);
        }

    while (pBuffer < pBufferEnd)
    {
        while (CurrT >= DELTA_OVERFLOW)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= DELTA_OVERFLOW;

            PrevSamp0 = CurrSamp0;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 =  (LONG)pSampleSrc->s16.sample_left;
            CurrSamp0 += (LONG)pSampleSrc->s16.sample_right;
            CurrSamp0 = CurrSamp0>>1;
            pCurrData+=4;
        }

        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> DELTAFRAC);
        OutSamp0 = (OutSamp0 * fxpGain) >> VOLSHIFT;
        CurrT += DeltaT;

        DEBUGMSG(ZONE_VERBOSE, (TEXT("PrevSamp0=0x%x, CurrSamp0=0x%x, CurrT=0x%x, OutSamp0=0x%x\r\n"), PrevSamp0,CurrSamp0,CurrT,OutSamp0));

        if (pBuffer < pBufferLast)
        {
            OutSamp0 += *(HWSAMPLE *)pBuffer;
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        *(HWSAMPLE *)pBuffer = (HWSAMPLE)OutSamp0;

        pBuffer += sizeof(HWSAMPLE);

    }

    Exit:
    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_CurrSamp[0] = CurrSamp0;

	RETAILMSG(0, (L"WAV:-%S\r\n",__FUNCTION__));
    return pBuffer;
}
#endif
