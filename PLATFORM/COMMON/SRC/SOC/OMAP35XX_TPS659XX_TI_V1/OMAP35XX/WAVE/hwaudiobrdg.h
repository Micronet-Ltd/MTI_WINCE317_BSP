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

#ifndef __HWAUDIOBRDG_H__
#define __HWAUDIOBRDG_H__

#include <pm.h>
#include "wavext.h"

//------------------------------------------------------------------------------
//  Defines a callback interface used by CHardwareAudioBridge to copy
// audio data to a buffer to be rendered
//
class CStreamCallback
{
public:
    typedef struct
        {
        DWORD           SampleRate;             // f32.0
        DWORD           InvSampleRate;          // f0.32
        }
    SampleRateEntry_t;

    typedef enum
        {
        k8khz = 0,
        k16khz,
        k44khz,
        k48khz,
        k96khz,     // added 96k sample frequency
        kAudioSampleRateSize
        }
    AudioSampleRate_e;

    //--------------------------------------------------------------------------
    // public methods
    //
public:
    virtual DWORD copy_AudioData(void* pStart, DWORD nSize)              = 0;
    virtual void put_AudioSampleRate(AudioSampleRate_e rate)             = 0;
    virtual DWORD copy_StreamData(HANDLE hContext, void* pStart, DWORD nSize) = 0;
};


//------------------------------------------------------------------------------
//
//  Serves as a base class for all hw audio bridge
//  - defines a common interface so all audio components can communicate with it
//
class CHardwareAudioBridge
{
    //--------------------------------------------------------------------------
    // typedef, enum, structs
    //
public:

    enum StreamType
        {
        kOutput = 0,
        kInput,
		kVoiceOutput,
        kVoiceInput,
		kBTVoice,
		kBTPlayback,
		kExtAudio,
#if defined(SYSGEN_BTH_AG)
		kBluetoothAudio,
#endif
        kStreamTypeCount
        };


    //--------------------------------------------------------------------------
    // member variables
    //
protected:

    CEDEVICE_POWER_STATE            m_PowerState;

    DWORD                           m_bBTHeadsetAttached;
    DWORD                           m_dwHdmiAudioAttached;
    DWORD                           m_dwHeadsetCount;
	DWORD                           m_dwBTHeadsetCount;
	DWORD                           m_dwExtAudioCount;
    DWORD                           m_dwSpeakerCount;
    DWORD                           m_dwWideBandCount;
	DWORD                           m_dwModemVoiceCallCount;

    CStreamCallback                *m_prgStreams[kStreamTypeCount];


    //--------------------------------------------------------------------------
    // constructor/destructor
    //
public:
    CHardwareAudioBridge()
    {
        memset(m_prgStreams, 0, sizeof(CStreamCallback*) * kStreamTypeCount);
        m_PowerState = D4;

        m_dwHeadsetCount = 0;
        m_dwSpeakerCount = 0;
        m_dwWideBandCount = 0;
        m_dwBTHeadsetCount = 0; 
        m_dwExtAudioCount = 0; 
		m_dwModemVoiceCallCount = 0; // #####, Michael
        m_bBTHeadsetAttached = FALSE;
        m_dwHdmiAudioAttached = FALSE;
    }


    //--------------------------------------------------------------------------
    // public methods
    //
public:

    void put_StreamCallback(StreamType type, CStreamCallback *pCallback)
    {
        m_prgStreams[type] = pCallback;
    }

    //--------------------------------------------------------------------------
    // public virtual methods
    //
public:

    // this should just enable the clocks necessary for the hardware
    //
    virtual void power_On()                            {}

    // this should immediately turn-off all power into the hardware
    //
    virtual void power_Off()                           {}

    virtual BOOL start_AudioPort(StreamType type)                                    { return FALSE; }
    virtual BOOL stop_AudioPort(StreamType type)                                    { return FALSE; }
    virtual BOOL start_Stream(StreamType type, HANDLE hStreamContext) { return FALSE; }
    virtual BOOL start_Stream(StreamType type) { return FALSE; }
    virtual BOOL stop_Stream(StreamType type, HANDLE hStreamContext) { return FALSE; }
    virtual BOOL set_StreamGain(StreamType type, HANDLE hStreamContext, DWORD dwContextData) {return FALSE;}
    virtual BOOL switch_AudioStreamPort(BOOL bPortRequest) {return FALSE;}
    virtual BOOL query_AudioStreamPort() {return FALSE;}
    virtual BOOL enable_I2SClocks(BOOL bClkEnable) {return FALSE;}

	virtual DWORD  SetMasterVolume(UINT16 gain)  {return 0;}
	virtual DWORD  GetMasterVolume(UINT16 *pGain) {return 0;}

	virtual DWORD  StartModemVoice(BOOL btOn) {return 0;}
	virtual DWORD  StopModemVoice(BOOL btOn) {return 0;}
    virtual DWORD  GetModemVoiceState(DWORD *pModemVoiceState, BOOL *pBTOn)  {return 0;}

	virtual DWORD  SetModemVoiceOutGain(INT16 gain)  {return 0;}
    virtual DWORD  GetModemVoiceOutGain(INT16 *pGain)  {return 0;}

	virtual DWORD  SetModemVoiceInputGain(INT16 gain)  {return 0;}
    virtual DWORD  GetModemVoiceInputGain(INT16 *pGain)  {return 0;}

	virtual DWORD  SetModemBTVoiceInputGain(INT16 gain)  {return 0;}
    virtual DWORD  GetModemBTVoiceInputGain(INT16 *pGain)  {return 0;}

	virtual DWORD  SetModemBTVoiceOutputGain(INT16 gain)  {return 0;}
    virtual DWORD  GetModemBTVoiceOutputGain(INT16 *pGain)  {return 0;}

	virtual DWORD  GetModemMicMuteStatus(BOOL *pMute)  {return 0;}
	virtual DWORD  SetModemMicMute(BOOL mute)  {return 0;}
	
	virtual DWORD  GetModemMicAnalogGain(UINT16 *pAGain)  {return 0;}
	virtual DWORD  SetModemMicAnalogGain(UINT16 AGain)  {return 0;}
	virtual DWORD  GetModemMicDigitalGain(UINT16 *pDigGain)  {return 0;}
	virtual DWORD  SetModemMicDigitalGain(UINT16 DigGain)  {return 0;}

    virtual DWORD  SetMuteModemVoiceOut(BOOL mute)  {return 0;}
	virtual DWORD  GetMuteModemVoiceOut(BOOL *pMute)  {return 0;}

    virtual DWORD  SetMuteModemVoiceIn(BOOL mute)  {return 0;}
	virtual DWORD  GetMuteModemVoiceIn(BOOL *pMute)  {return 0;}

    virtual DWORD  GetExtMicMuteStatus(BOOL *pMute)  {return 0;} // 22-Feb-2011
    virtual DWORD  SetExtMicMute(BOOL mute)  {return 0;} // 22-Feb-2011
	virtual DWORD  SetExtMicDigitalGain(INT16 DigGain)  {return 0;} // 22-Feb-2011
	virtual DWORD  SetExtMicAnalogGain(UINT16 AGain)  {return 0;} // 22-Feb-2011

    virtual DWORD  GetExtMicDigitalGain(INT16 *pDigGain)  {return 0;} // 22-Feb-2011
	virtual DWORD  GetExtMicAnalogGain(UINT16 *pAGain)  {return 0;} // 22-Feb-2011


    virtual DWORD  SetMuteModemBTInVoice(BOOL mute)  {return 0;}
	virtual DWORD  GetMuteModemBTInVoice(BOOL *pMute)  {return 0;}

    virtual DWORD  SetMuteModemBTOutVoice(BOOL mute)  {return 0;}
	virtual DWORD  GetMuteModemBTOutVoice(BOOL *pMute)  {return 0;}

	//virtual DWORD  StartBTPlayback(void) {return 0;}
	//virtual DWORD  StopBTPlayback(void) {return 0;}
    //virtual DWORD  GetBTPlaybackState(DWORD *pBTPlaybackState)  {return 0;}

    virtual DWORD  SetMuteBTOutPlayback(BOOL mute)  {return 0;}
	virtual DWORD  GetMuteBTOutPlayback(BOOL *pMute)  {return 0;}

	virtual DWORD  SetBTPlaybackOutputGain(INT16 gain)  {return 0;}
    virtual DWORD  GetBTPlaybackOutputGain(INT16 *pGain)  {return 0;}

    // calling this routine does not necessarily immediately change the
    // power state; ie when audio is currently being rendered
    //
    virtual void request_PowerState(CEDEVICE_POWER_STATE powerState)
    {
        if (powerState == D0 || powerState == D4)
            {
            m_PowerState = powerState;
            }
    }

    // the current power state
    //
    virtual CEDEVICE_POWER_STATE get_CurrentPowerState() const
    {
        return m_PowerState;
    }

    // audio profiles
    //
    virtual void notify_BTHeadsetAttached(DWORD bAttached)
    {
        m_bBTHeadsetAttached = bAttached;
		return;
    }

    virtual void notify_HdmiAudioAttached(BOOL bAttached)
    {
        m_dwHdmiAudioAttached = bAttached;
    }

    virtual BOOL enable_Headset(BOOL bEnable)
    {
        return (bEnable) ? ++m_dwHeadsetCount : --m_dwHeadsetCount;
    }

    virtual BOOL enable_BTHeadset(BOOL bEnable)
    {
        //return (bEnable) ? ++m_dwBTHeadsetCount : --m_dwBTHeadsetCount;
		if( bEnable )
          ++m_dwBTHeadsetCount;
		else
          --m_dwBTHeadsetCount;

        RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S, m_dwBTHeadsetCount = %d\r\n",__FUNCTION__, m_dwBTHeadsetCount));
		return((BOOL)m_dwBTHeadsetCount);
    }

	// m_dwExtAudioCount
    virtual BOOL enable_ExtAudio(BOOL bEnable)
    {
        if( bEnable )
          ++m_dwExtAudioCount;
		else
          --m_dwExtAudioCount;

        RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S, m_dwExtAudioCount = %d\r\n",__FUNCTION__, m_dwExtAudioCount));
		return((BOOL)m_dwExtAudioCount);
    }

    virtual BOOL enable_AuxHeadset(BOOL bEnable)
    {
        return (bEnable) ? ++m_dwHeadsetCount : --m_dwHeadsetCount;      
    }

	// #####, Michael  ModemVoiceCall
    virtual BOOL enable_ModemVoiceCall(BOOL bEnable)
    {
        return (bEnable) ? ++m_dwModemVoiceCallCount : --m_dwModemVoiceCallCount;
    }


    virtual BOOL enable_Speaker(BOOL bEnable)
    {
        return (bEnable) ? ++m_dwSpeakerCount : --m_dwSpeakerCount;
    }

    virtual BOOL enable_WideBand(BOOL bEnable)
    {
        return (bEnable) ? ++m_dwWideBandCount : --m_dwWideBandCount;
    }

    virtual BOOL query_BTHeadsetAttached() const
    {
		RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S, m_dwBTHeadsetCount = %d\r\n",__FUNCTION__, m_dwBTHeadsetCount));
		return m_dwBTHeadsetCount > 0;
        //return m_bBTHeadsetAttached;
    }

    virtual BOOL query_BTHeadsetEnable() const
    {
		RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S, m_dwBTHeadsetCount = %d\r\n",__FUNCTION__, m_dwBTHeadsetCount));
        return m_dwBTHeadsetCount > 0;
    }

    virtual BOOL query_ExtAudioEnable() const
    {
		RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S, m_dwExtAudioCount = %d\r\n",__FUNCTION__, m_dwExtAudioCount));
        return m_dwExtAudioCount > 0;
    }


    virtual BOOL query_ExtAudioAttached() const
    {
		RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S, m_dwExtAudioCount = %d\r\n",__FUNCTION__, m_dwExtAudioCount));
		return m_dwExtAudioCount > 0;
        //return m_bBTHeadsetAttached;
    }

    
    virtual BOOL query_HdmiAudioAttached() const
    {
        return m_dwHdmiAudioAttached;
    }

    virtual BOOL query_HeadsetEnable() const
    {
        return m_dwHeadsetCount > 0;
    }

    // #####, Michael  ModemVoiceCall
    virtual BOOL query_ModemVoiceCallEnable() const
    {
        return m_dwModemVoiceCallCount > 0;
    }


    virtual BOOL query_SpeakerEnable() const
    {
        return m_dwSpeakerCount > 0;
    }

    virtual BOOL query_WideBand() const
    {
        return m_dwWideBandCount > 0;
    }

    virtual BOOL query_CarkitEnable() const             { return FALSE; }
};



#endif // __HWAUDIOBRDG_H__
