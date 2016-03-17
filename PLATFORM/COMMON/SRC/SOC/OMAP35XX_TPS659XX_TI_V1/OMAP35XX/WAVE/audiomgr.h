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

#ifndef __AUDIOMGR_H__
#define __AUDIOMGR_H__

//#include <windows.h>
#include "strmmgr.h"
#include "hwaudiobrdg.h"


#define  WAVE_AUDIO_MNGR_DBG  0

class CAudioControlBase;
class CAudioManager;




//------------------------------------------------------------------------------
//
//  Behaves as a mediator for all the audio components.  This class strictly
//  deals with system audio and uses a separate HWAudioBridge to perform
//  all hardware related tasks.  This class is designed to be subclassed to
//  support specific needs such as modem, codec, etc.
//
class CAudioManager
{
    //--------------------------------------------------------------------------
    // public typedefs, enums, and structs
    //
public:

    enum { kMaximumAudioAttenuation = 0x8000 };
    enum AudioManagerState
        {
        kUninitialized,                 // class instantiated but uninitialized
        kInitialized,                   // class and sub-components initialized
        kShutdown                       // class is shutting down
        };


    //--------------------------------------------------------------------------
    // protected member variables
    //
protected:
    AudioManagerState       m_state;

    CEDEVICE_POWER_STATE    m_CurPowerState;
    BOOL                    m_bSuspend;

    HANDLE                  m_hParentBus;

    DWORD                   m_DriverIndex;

    LONG                    m_counterForcedQuality;

    // audio state helper variables
    //
    DWORD                   m_ffOutputAudioProfile;
    DWORD                   m_ffInputAudioProfile;

    CHardwareAudioBridge   *m_pHardwareBridge;

    CAudioMixerManager     *m_pAudioMixerManager;
    CStreamManager         *m_pInputStreamManager;
    CStreamManager         *m_pOutputStreamManager;

    //--------------------------------------------------------------------------
    // public member variables
    //
public:
    DWORD                   m_dwStreamAttenMax;
    DWORD                   m_dwDeviceAttenMax;
    DWORD                   m_dwClassAttenMax;


    //--------------------------------------------------------------------------
    // protected inline methods
    //
protected:

    inline void  OutputBufferCacheFlush (PBYTE pbMem, DWORD dwLen)
        {
#ifdef MIXER_CACHEDMEM
        CacheRangeFlush(pbMem, dwLen, CACHE_SYNC_WRITEBACK);
#endif
        }
    inline void  InputBufferCacheDiscard(PBYTE pbMem, DWORD dwLen)
        {
#ifdef INPUT_CACHEDMEM
        CacheRangeFlush(pbMem, dwLen, CACHE_SYNC_DISCARD);
#endif
        }

    //--------------------------------------------------------------------------
    // protected methods
    //
protected:

    void InternalCleanUp();

#if USE_HW_SATURATE
    void HandleSaturation(PBYTE pBuffer, PBYTE pBufferEnd);
#endif


    //--------------------------------------------------------------------------
    // constructor/destructors
    //
public:

    CAudioManager();
    virtual ~CAudioManager();


    //--------------------------------------------------------------------------
    // public inline methods
    //
public:

    void Lock()               {EnterMutex();}
    void Unlock()             {ExitMutex();}

    CAudioMixerManager* get_AudioMixerManager() const
    {
        return m_pAudioMixerManager;
    }

    void put_InputStreamManager(CStreamManager *pStreamManager)
    {
        m_pInputStreamManager = pStreamManager;
    }

    void put_OutputStreamManager(CStreamManager *pStreamManager)
    {
        m_pOutputStreamManager = pStreamManager;
    }

    CStreamManager* get_InputStreamManager(UINT DeviceId) const
    {
        return m_pInputStreamManager;
    }

    CStreamManager* get_OutputStreamManager(UINT DeviceId) const
    {
        return m_pOutputStreamManager;
    }

    void put_HardwareAudioBridge(CHardwareAudioBridge* pHardwareBridge)
    {
        m_pHardwareBridge = pHardwareBridge;
    }

    CHardwareAudioBridge* get_HardwareAudioBridge() const
    {
        return m_pHardwareBridge;
    }

    CStreamManager* get_OutStreamManager()
        {
        return m_pOutputStreamManager;
        }

    CStreamManager* get_InStreamManager()
        {
        return m_pInputStreamManager;
        }

    //--------------------------------------------------------------------------
    // public virtual methods
    //
public:

    virtual DWORD get_InputDeviceCount()        {return 1;}
    virtual DWORD get_OutputDeviceCount()       {return 1;}

    virtual BOOL OutputStreamOpened()           { return TRUE;}
    virtual BOOL OutputStreamClosed()           { return TRUE;}
    virtual BOOL InputStreamOpened()            { return TRUE;}
    virtual BOOL InputStreamClosed()            { return TRUE;}

    // Mixer callbacks
    //
    virtual DWORD put_OutputGain(DWORD dwGain)
    {
		RETAILMSG(0, (L"+CAudioManager::put_OutputGain dwGain = 0x%X\r\n", dwGain));
        return get_AudioMixerManager()->put_AudioVolume(
                    CAudioMixerManager::kWavOutVolumeControl, dwGain);
		RETAILMSG(0, (L"-CAudioManager::put_OutputGain dwGain = 0x%X\r\n", dwGain));
    }

    virtual DWORD get_OutputGain()
    {
        DWORD dwGain;
		RETAILMSG(0, (L"+CAudioManager::get_OutputGain\r\n"));
        get_AudioMixerManager()->get_AudioVolume(
            CAudioMixerManager::kWavOutVolumeControl, &dwGain);
		RETAILMSG(0, (L"-CAudioManager::get_OutputGain dwGain = 0x%X\r\n", dwGain));
        return dwGain;
    }

    virtual DWORD put_InputGain(DWORD dwGain)
    {
        return get_AudioMixerManager()->put_AudioVolume(
                    CAudioMixerManager::kWavInVolumeControl, dwGain);
    }

    virtual DWORD get_InputGain()
    {
        DWORD dwGain;
        get_AudioMixerManager()->get_AudioVolume(
            CAudioMixerManager::kWavInVolumeControl, &dwGain);
        return dwGain;
    }

    virtual DWORD put_OutputMute(BOOL Mute)
    {
        return get_AudioMixerManager()->put_AudioMute(
                    CAudioMixerManager::kWavOutMuteControl, Mute);
    }

    virtual DWORD get_OutputMute()
    {
        BOOL bMute;
        get_AudioMixerManager()->get_AudioMute(
            CAudioMixerManager::kWavOutMuteControl, &bMute);
        return bMute;
    }

    virtual DWORD put_InputMute (BOOL bMute)
    {
        return get_AudioMixerManager()->put_AudioMute(
                    CAudioMixerManager::kWavInMuteControl, bMute);
    }

    virtual DWORD get_InputMute()
    {
        BOOL bMute;
        get_AudioMixerManager()->get_AudioMute(
            CAudioMixerManager::kWavInMuteControl, &bMute);
        return bMute;
    }


    //--------------------------------------------------------------------------
    // public methods
    //
public:

    virtual BOOL initialize(LPCWSTR szContext, LPCVOID pBusContext);

    virtual BOOL deinitialize();

#if 0
    void PowerUp();
    void PowerDown();

    virtual void StartAudioCapture();
    virtual void StopAudioCapture();

    virtual void StartAudioRender();
    virtual void StopAudioRender();

    DWORD ForceQuality (BOOL bQuality);

    ULONG TransferInputBuffer(ULONG NumBuf);
    ULONG TransferOutputBuffer(ULONG NumBuf);
#endif

    //void NotifyHeadsetOn(BOOL);
    //void ToggleExtSpeaker();
    //VOID NotifyBtHeadsetOn(DWORD dwBtAudioRouting);

    //DWORD GetSpeakerMode();
    //VOID  SetSpeakerMode(DWORD Mode);

    //--------------------------------------------------------------------------
    // pure virtual methods
    //
public:

    BOOL process_IOControlMsg(DWORD  dwCode,
                    PBYTE  pBufIn, DWORD  dwLenIn,
                    PBYTE  pBufOut, DWORD  dwLenOut,
                    PDWORD pdwActualOut);

    virtual BOOL process_AudioStreamMessage(PMMDRV_MESSAGE_PARAMS pParams,
                    DWORD *pdwResult);

    virtual BOOL process_PowerManagementMessage(DWORD  dwCode,
                    PBYTE  pBufIn, DWORD  dwLenIn,
                    PBYTE  pBufOut, DWORD  dwLenOut,
                    PDWORD pdwActualOut);

    virtual BOOL process_MiscellaneousMessage(DWORD  dwCode,
                    PBYTE  pBufIn, DWORD  dwLenIn,
                    PBYTE  pBufOut, DWORD  dwLenOut,
                    PDWORD pdwActualOut);

};



#endif //__AUDIOMGR_H__


