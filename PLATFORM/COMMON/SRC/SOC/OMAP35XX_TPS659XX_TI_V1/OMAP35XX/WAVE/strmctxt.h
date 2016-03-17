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
#ifndef __STRMCTXT_H__
#define __STRMCTXT_H__

#include "wavepdd.h"

class CStreamManager;

#define VOLSHIFT (32-BITSPERSAMPLE)

// Define the format of the m_Delta value. By default, we'll use
// 17.15 format. This may seem weird, but there's a problem using
// 16.16 format with the SRC code-- in the following equation where
// we use the fractional part to calculate the interpolated sample:
//        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> DELTAFRAC);
// If CurrT is a 16-bit fraction value, and if (CurrSamp0-PrevSamp0) overflow a
// signed 16-bit number (which they can), then the result can overflow a 32-bit
// value and cause a click.

// We could use 64-bit math to overcome that, but it would be a few extra instructions in the inner loop.
//
// On the other hand, we want as much accuracy in the fractional part as possible,
// as that determines how close we are to the target rate during sample rate conversion.
// Therefore, using 17.15 format is the best we can do.

#define DELTAINT  (17)                // was 24
#define DELTAFRAC (32 - DELTAINT)

// Define 1.0 value for Delta
#define DELTA_OVERFLOW (1<<DELTAFRAC) // was 0x100

// The code will then use the follwing values as saturation points
// Minimum and maximum sample values for saturation calculations
#define AUDIO_SAMPLE_MAX ((HWSAMPLE)32767)
#define AUDIO_SAMPLE_MIN ((HWSAMPLE)-32768)

class StreamContext
{
public:
    LIST_ENTRY  m_Link;               // Link into list of streams

    StreamContext() {};
    virtual ~StreamContext() {};

    LONG AddRef();
    LONG Release();

    virtual unsigned int GetStreamDirection() = 0;

    virtual HRESULT Open(CStreamManager *pStreamManager, LPWAVEOPENDESC lpWOD, DWORD dwFlags);
    virtual DWORD Close();
    virtual DWORD GetPos(PMMTIME pmmt);

    virtual DWORD Run();
    virtual DWORD Stop();
    virtual DWORD Reset();
    virtual PBYTE Render(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender)=0;

    BOOL StillPlaying() {return (m_lpWaveHdrHead!=NULL);}
    DWORD GetByteCount() {return m_dwByteCount;}
    WAVEFORMATEX *GetWaveFormat() {return &m_WaveFormat;}
    CStreamManager *GetStreamManager() { return m_pStreamManager; }

    void DoDriverCallback(UINT msg, DWORD dwParam1, DWORD dwParam2)
    {
        m_pfnCallback(m_hWave,msg,m_dwInstance,dwParam1,dwParam2);
    }
    virtual DoCallbackReturnBuffer(LPWAVEHDR lpHdr)
    {
        DoDriverCallback(WOM_DONE,(DWORD)lpHdr,0);
    }
    virtual DoCallbackStreamOpened()
    {
        DEBUGMSG(ZONE_WODM && ZONE_VERBOSE,(TEXT("WODM: DoCallbackStreamOpened\r\n")));
        DoDriverCallback(WOM_OPEN,0,0);
    }
    virtual DoCallbackStreamClosed()
    {
        DEBUGMSG(ZONE_WODM && ZONE_VERBOSE,(TEXT("WODM: DoCallbackStreamClosed\r\n")));
        DoDriverCallback(WOM_CLOSE,0,0);
    }

    virtual DWORD QueueBuffer(LPWAVEHDR lpWaveHdr);
    PBYTE GetNextBuffer();

    // Default implementation
    void ReturnBuffer(LPWAVEHDR lpHdr)
    {
        lpHdr->dwFlags &= ~WHDR_INQUEUE;
        lpHdr->dwFlags |= WHDR_DONE;
        DEBUGMSG(ZONE_WODM && ZONE_VERBOSE,(TEXT("WODM: DoCallbackStreamClosed\r\n")));
        DoCallbackReturnBuffer(lpHdr);
    }

    DWORD GetGain()
    {
		RETAILMSG(0, (L"StreamContext::GetGain m_dwGain = %d\r\n", m_dwGain));
        return m_dwGain;
    }

    DWORD SetGain(DWORD dwGain)
    {
        m_dwGain = dwGain;
		RETAILMSG(0, (L"StreamContext::SetGain m_dwGain = %d\r\n", m_dwGain));
        GainChange();
        return MMSYSERR_NOERROR;
    }

    virtual DWORD GetRate(DWORD *pdwMultiplier) const
    {
        return MMSYSERR_NOTSUPPORTED;
    }

    virtual DWORD SetRate(DWORD dwMultiplier)
    {
        return MMSYSERR_NOTSUPPORTED;
    }

    DWORD SetClassId(DWORD dwClassId);

    virtual DWORD MapGain(DWORD dwStreamGain, DWORD Channel);

    virtual void GainChange();

    DWORD BreakLoop();

    DWORD ForceQuality (BOOL bForceQuality);

    DWORD GetProperty(PWAVEPROPINFO pPropInfo);
    DWORD SetProperty(PWAVEPROPINFO pPropInfo);

protected:
    LONG                m_RefCount;
    BOOL                m_bRunning;         // Is stream running or stopped

    DWORD               m_dwFlags;          // allocation flags
    HWAVE               m_hWave;            // handle for stream
    DRVCALLBACK*        m_pfnCallback;      // client's callback
    DWORD               m_dwInstance;       // client's instance data

    WAVEFORMATEX        m_WaveFormat;       // Format of wave data

    LPWAVEHDR           m_lpWaveHdrHead;
    LPWAVEHDR           m_lpWaveHdrCurrent;
    LPWAVEHDR           m_lpWaveHdrTail;
    PBYTE               m_lpCurrData;       // position in current buffer
    PBYTE               m_lpCurrDataEnd;    // end of current buffer

    DWORD               m_dwByteCount;      // byte count since last reset

    CStreamManager *    m_pStreamManager;   // Device which this stream renders to

    // Loopcount shouldn't really be here, since it's really for wave output only, but it makes things easier
    DWORD               m_dwLoopCount;      // Number of times left through loop

    DWORD               m_dwGain;
    DWORD               m_fxpGain[2];
    BOOL                m_bMono;
    DWORD               m_dwClassId;
    BOOL                m_bForceQuality;
};

class WaveStreamContext : public StreamContext
{
public:
    virtual HRESULT Open(CStreamManager *pStreamManager, LPWAVEOPENDESC lpWOD, DWORD dwFlags);

    virtual DWORD GetRate(DWORD *pdwMultiplier) const;
    PBYTE Render(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender);
    PBYTE RenderRawData(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast);
    virtual PBYTE Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender)=0;

protected:
    PCM_TYPE            m_SampleType;       // Enum of sample type, e.g. M8, M16, S8, S16
    ULONG               m_SampleSize;       // # of bytes per sample in client buffer
    DWORD               m_DeltaT;           // Sample rate conversion factor
    DWORD               m_dwMultiplier;
    LONG                m_PrevSamp[OUTCHANNELS];
    LONG                m_CurrSamp[OUTCHANNELS];
    LONG                m_CurrT;
};

class InputStreamContext : public WaveStreamContext
{
public:
    HRESULT Open(CStreamManager *pStreamManager, LPWAVEOPENDESC lpWOD, DWORD dwFlags);
    virtual DWORD SetRate(DWORD dwMultiplier);
    DWORD Stop();   // On input, stop has special handling
    PBYTE Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender);
    DWORD MapGain(DWORD Gain, DWORD Channel);
    virtual unsigned int GetStreamDirection();

    virtual DoCallbackReturnBuffer(LPWAVEHDR lpHdr)
    {
        DEBUGMSG(ZONE_WIDM && ZONE_VERBOSE,(TEXT("WIDM: DoCallbackReturnBuffer\r\n")));
        DoDriverCallback(WIM_DATA,(DWORD)lpHdr,0);
    }
    virtual DoCallbackStreamOpened()
    {
        DEBUGMSG(ZONE_WIDM && ZONE_VERBOSE,(TEXT("WIDM: DoCallbackStreamOpened\r\n")));
        DoDriverCallback(WIM_OPEN,0,0);
    }
    virtual DoCallbackStreamClosed()
    {
        DEBUGMSG(ZONE_WIDM && ZONE_VERBOSE,(TEXT("WIDM: DoCallbackStreamOpened\r\n")));
        DoDriverCallback(WIM_CLOSE,0,0);
    }
};

class OutputStreamContext : public WaveStreamContext
{
public:
    HRESULT Open(CStreamManager *pStreamManager, LPWAVEOPENDESC lpWOD, DWORD dwFlags);
    DWORD Reset();


    virtual DWORD SetRate(DWORD dwMultiplier);
    virtual unsigned int GetStreamDirection();
};

class OutputStreamContextM8 : public OutputStreamContext
{
public:
    PBYTE Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender);
};
class OutputStreamContextM16 : public OutputStreamContext
{
public:
    PBYTE Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender);
};
class OutputStreamContextS8 : public OutputStreamContext
{
    PBYTE Render2Mono  (PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender);
    PBYTE Render2Stereo(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender);
public:
    PBYTE Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender);
};
class OutputStreamContextS16 : public OutputStreamContext
{
    PBYTE Render2Mono  (PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender);
    PBYTE Render2Stereo(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender);
public:
    PBYTE Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast, BOOL bDSPRender);
};


#endif
