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

#ifndef __OMAP35XX_HWBRIDGE_H__
#define __OMAP35XX_HWBRIDGE_H__

#include "hwaudiobrdg.h"
#include "mcbsp.h"
#include "memtxapi.h"
#include "dmtaudioport.h"
#include "dasfaudioport.h"
#include "tps659xx_wave.h"
#include "tps659xx_audio.h"

//------------------------------------------------------------------------------
//
//  Serves as a base class for all hw audio bridge
//  - defines a common interface so all audio components can communicate with it
//
class OMAP35XX_HwAudioBridge : public CHardwareAudioBridge, 
                                public AudioStreamPort_Client
{
public:
    // Note: The values of this enum are used in the registry to select default audio route
    typedef enum 
        {
        kAudioRoute_Unknown = 0,
        kAudioRoute_Handset,
        kAudioRoute_Headset,
        kAudioRoute_Carkit,
        kAudioRoute_Speaker,
        kAudioRoute_BTHeadset,
		kAudioRoute_ExtAudio,
        kAudioRoute_AuxHeadset,
		kAudioRoute_ModemVoiceCall,     // #####, Michael
        kAudioRoute_HdmiAudioRequested,
        kAudioRoute_HdmiAudio,
        kAudioRoute_HdmiAudioDetached,
#if defined(SYSGEN_BTH_AG)
		kAudioRoute_BluetoothAudio,
#endif
        kAudioRoute_Count
        }
    AudioRoute_e;

    typedef enum
        {
        kAudioRender_Idle,
        kAudioRender_Starting,
        kAudioRender_Active,
        kAudioRender_Stopping
        }
    AudioRenderState_e;

    typedef enum
        {
        kAudioDMTPort,
        kAudioDASFPort
        }
    AudioActiveRenderPort_e;


    //--------------------------------------------------------------------------
    // member variables
    //
protected:

    HANDLE                                  m_hICX;
    HANDLE                                  m_hHwCodec;

    CEDEVICE_POWER_STATE                    m_ReqestedPowerState;
    BOOL                                    m_bRxAbortRequested;
    BOOL                                    m_bTxAbortRequested;

    AudioStreamPort                        *m_pActivePort;
    DMTAudioStreamPort                      m_DMTPort;
    DASFAudioStreamPort                     m_DASFPort;

    AudioRoute_e                            m_RequestAudioRoute;
    AudioRoute_e                            m_CurrentAudioRoute;
    DWORD                                   m_fRequestAudioRouteDirty;
    AudioRenderState_e                      m_ReceiverState;
    AudioRenderState_e                      m_TransmitterState;
    AudioRenderState_e                      m_ReceiverStatePrev;
    AudioRenderState_e                      m_TransmitterStatePrev;

    DWORD                                    m_dwAudioProfile;
    BOOL                                    m_bPortSwitch;
    AudioActiveRenderPort_e                 m_CurrActivePort;
    BOOL                                    m_bPreviousPortIsDASF;

	BOOL                                     m_ModemVoiceActive;
	//UINT16                                  m_ModemVoiceGain;
	BOOL                                    m_CodecPower; 
	BOOL                                    m_extAudioPower; 
    
    //--------------------------------------------------------------------------
    // constructor/destructor
    //
public:

    OMAP35XX_HwAudioBridge() :
        CHardwareAudioBridge(),
        m_DMTPort(),
        m_DASFPort(),
        m_pActivePort(NULL),
        m_hICX(NULL)
    {
        m_ReceiverState = kAudioRender_Idle;
        m_TransmitterState = kAudioRender_Idle;
        m_ReceiverStatePrev = kAudioRender_Idle;
        m_TransmitterStatePrev = kAudioRender_Idle;
        m_fRequestAudioRouteDirty = TRUE;
        m_CurrentAudioRoute = kAudioRoute_Unknown;
        m_RequestAudioRoute = kAudioRoute_Handset;
        m_dwAudioProfile = 0;
        m_bPortSwitch = FALSE;
        m_CurrActivePort = kAudioDMTPort;
        m_bPreviousPortIsDASF = FALSE;
		m_ModemVoiceActive = FALSE; 
		//m_ModemVoiceGain  = 0;
		//m_ModemVoiceMute  = FALSE;
    }

    //--------------------------------------------------------------------------
    // AudioStreamPort_Client methods
    //
public:    

    virtual BOOL OnAudioStreamMessage(AudioStreamPort *pPort,
                                      DWORD msg,
                                      void *pvData);
    
    void initialize(WCHAR const *szDMTDriver,
                    WCHAR const *szSerialDriver,
                    HwCodecConfigInfo_t *pHwCodecConfigInfo,
                    HANDLE hPlayPortConfigInfo,
                    HANDLE hRecPortConfigInfo,
					AudioRoute_e eAudioRoute);

    //--------------------------------------------------------------------------
    
protected:
    BOOL update_AudioRouting();
    BOOL query_AudioRouteRequestPending() const 
    { 
        return m_fRequestAudioRouteDirty;
    }

    //--------------------------------------------------------------------------
    // public methods
public:

    void SetAudioPath(AudioRoute_e audioroute,
                      DWORD dwAudioProfile);

    // calling this routine does not necessarily immediately change the
    // power state; ie when audio is currently being rendered
    //
    virtual void request_PowerState(CEDEVICE_POWER_STATE powerState) 
    {
        DEBUGMSG(1, (L"WAV: request_PowerState - %x\r\n", powerState));
		RETAILMSG(WAVE_AUDIO_DBG, (L"WAV: request_PowerState - %x, m_ModemVoiceActive = %d\r\n", powerState, m_ModemVoiceActive));
		
        if (powerState >= D3)
            {
            m_ReqestedPowerState = powerState;

              // if the RX is running, then abort it.
              if (m_ReceiverState == kAudioRender_Starting ||
                m_ReceiverState == kAudioRender_Active)
                {
                m_bRxAbortRequested = TRUE;
                abort_Stream(kInput, NULL);
                }

              // if the TX is running, then abort it.
              if (m_TransmitterState == kAudioRender_Starting ||
                m_TransmitterState == kAudioRender_Active)
                {
                m_bTxAbortRequested = TRUE;
				RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S prev tx state = %d, current tx state = %d\r\n",__FUNCTION__, m_TransmitterState, kAudioRender_Stopping));
                m_TransmitterState=kAudioRender_Stopping;
                abort_Stream(kOutput, NULL);
                }

			if( !m_ModemVoiceActive  )
			 {
			  // Switch Off Audio Codec
			  m_CodecPower = SetHwCodecMode_GetCodecPower(m_hHwCodec);
			  RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S, Switch Off Internal Audio Codec, Codec Power = %d\r\n",__FUNCTION__, m_CodecPower));
			  if( m_CodecPower == TRUE )
			   SetHwCodecMode_DisableCodecPower(m_hHwCodec, kAudioVoiceMode);

			  // Switch Off External Audio
			  RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S, Switch Off External Audio Codec\r\n",__FUNCTION__));
			  if (query_ExtAudioEnable())
			  {
				  SetHwCodecMode_Disabled(m_hHwCodec);
			  }
			 }
            }
		else
            {
            CEDEVICE_POWER_STATE prevReqestedPowerState = m_ReqestedPowerState;
            m_ReqestedPowerState = powerState;

            RETAILMSG(WAVE_AUDIO_DBG, (L"WAV: request_PowerState %x (prev Power State %x)\r\n", powerState, prevReqestedPowerState));

            // Check if previously D3/D4 was requested!
            if (prevReqestedPowerState >= D3)
                {
					if( !m_ModemVoiceActive  )
					{
			         if( m_CodecPower == TRUE )
			            SetHwCodecMode_EnableCodecPower(m_hHwCodec);

					  if (query_ExtAudioEnable())
					  {
						  SetHwCodecMode_ExtAudio(m_hHwCodec, (AudioProfile_e)m_dwAudioProfile);
					  }

					}

                // If RX was aborted earlier, update the flag & re-start RX.
                if (m_bRxAbortRequested)
                    {
                    m_bRxAbortRequested = FALSE;
                    start_Stream(kInput);
                    }

                // If TX was aborted earlier, just update the flag, No need
                // re-start it.
                if (m_bTxAbortRequested)
                    {
                    m_bTxAbortRequested = FALSE;
                    start_Stream(kOutput);
                    }
                }
            }

        if (powerState == D0 || powerState == D4)
            {
            m_PowerState = powerState;
            }
    }
    
    virtual BOOL start_AudioPort(StreamType type);
    virtual BOOL stop_AudioPort(StreamType type);
    virtual BOOL start_Stream(StreamType type,
                              HANDLE hStreamContext);
    virtual BOOL start_Stream(StreamType type);
    virtual BOOL stop_Stream(StreamType type,
                             HANDLE hStreamContext);
    virtual BOOL abort_Stream(StreamType type,
                              HANDLE hStreamContext);
    virtual BOOL set_StreamGain(StreamType type,
                                HANDLE hStreamContext,
                                DWORD dwContextData);
    virtual BOOL switch_AudioStreamPort(BOOL bPortRequest);
    virtual BOOL query_AudioStreamPort();
    virtual BOOL enable_I2SClocks(BOOL bClkEnable);

	virtual DWORD  SetMasterVolume(UINT16 gain);
	virtual DWORD  GetMasterVolume(UINT16 *pGain);
	virtual DWORD  StartModemVoice(BOOL btOn);
	virtual DWORD  StopModemVoice(BOOL btOn);
    virtual DWORD  GetModemVoiceState(DWORD *pModemVoiceState, BOOL *pBTOn);

	virtual DWORD  SetModemVoiceOutGain(INT16 gain);
    virtual DWORD  GetModemVoiceOutGain(INT16 *pGain);

	virtual DWORD  SetModemVoiceInputGain(INT16 gain);
    virtual DWORD  GetModemVoiceInputGain(INT16 *pGain);

	virtual DWORD  SetModemBTVoiceInputGain(INT16 gain);
    virtual DWORD  GetModemBTVoiceInputGain(INT16 *pGain);

	virtual DWORD  SetModemBTVoiceOutputGain(INT16 gain);
    virtual DWORD  GetModemBTVoiceOutputGain(INT16 *pGain);

	virtual DWORD  GetModemMicMuteStatus(BOOL *pMute);
	virtual DWORD  GetExtMicMuteStatus(BOOL *pMute); // 22-Feb-2011
	virtual DWORD  SetModemMicMute(BOOL mute);

	virtual DWORD  SetExtMicMute(BOOL mute); // 22-Feb-2011

	virtual DWORD  GetModemMicAnalogGain(UINT16 *pAGain);
	virtual DWORD  SetModemMicAnalogGain(UINT16 AGain);
	virtual DWORD  GetModemMicDigitalGain(UINT16 *pDigGain);
	virtual DWORD  SetModemMicDigitalGain(UINT16 DigGain);

	virtual DWORD  SetExtMicDigitalGain(INT16 DigGain); // 22-Feb-2011
	virtual DWORD  SetExtMicAnalogGain(UINT16 AGain); // 22-Feb-2011

    virtual DWORD  GetExtMicDigitalGain(INT16 *pDigGain); // 22-Feb-2011
	virtual DWORD  GetExtMicAnalogGain(UINT16 *pAGain); // 22-Feb-2011

    virtual DWORD  SetMuteModemVoiceOut(BOOL mute);
	virtual DWORD  GetMuteModemVoiceOut(BOOL *pMute);

    virtual DWORD  SetMuteModemVoiceIn(BOOL mute);
	virtual DWORD  GetMuteModemVoiceIn(BOOL *pMute);

    virtual DWORD  SetMuteModemBTInVoice(BOOL mute);
	virtual DWORD  GetMuteModemBTInVoice(BOOL *pMute);

    virtual DWORD  SetMuteModemBTOutVoice(BOOL mute);
	virtual DWORD  GetMuteModemBTOutVoice(BOOL *pMute);

	//virtual DWORD  StartBTPlayback(void);
	//virtual DWORD  StopBTPlayback(void);
    //virtual DWORD  GetBTPlaybackState(DWORD *pBTPlaybackState);

    virtual DWORD  SetMuteBTOutPlayback(BOOL mute);
	virtual DWORD  GetMuteBTOutPlayback(BOOL *pMute);

	virtual DWORD  SetBTPlaybackOutputGain(INT16 gain);
    virtual DWORD  GetBTPlaybackOutputGain(INT16 *pGain);

    // we're always in wideband mode and speaker mode is not supported
    //
    virtual BOOL query_WideBand() const
    {
        return TRUE;
    }

    virtual BOOL enable_Headset(BOOL bEnable)
    {
        CHardwareAudioBridge::enable_Headset(bEnable);
        if (bEnable)
            {
            m_RequestAudioRoute = kAudioRoute_Headset;
            }
        else
            {
            m_RequestAudioRoute = kAudioRoute_Handset;
            }
        update_AudioRouting();
        return TRUE;
    }

    virtual BOOL enable_BTHeadset(BOOL bEnable)
    {
        CHardwareAudioBridge::enable_BTHeadset(bEnable);
        if (bEnable)
            {
            m_RequestAudioRoute = kAudioRoute_BTHeadset;
            }
        else
            {
            m_RequestAudioRoute = kAudioRoute_Handset;
            }
		RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S, bEnable = %d, m_RequestAudioRoute = %d\r\n",__FUNCTION__, bEnable, m_RequestAudioRoute));
        update_AudioRouting();
        return TRUE;
    }

   virtual BOOL enable_ExtAudio(BOOL bEnable)
    {
        CHardwareAudioBridge::enable_ExtAudio(bEnable);
        if (bEnable)
            {
            m_RequestAudioRoute = kAudioRoute_ExtAudio;
            }
        else
            {
            m_RequestAudioRoute = kAudioRoute_Handset;
            }
		RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S, bEnable = %d, m_RequestAudioRoute = %d\r\n",__FUNCTION__, bEnable, m_RequestAudioRoute));
        update_AudioRouting();
        return TRUE;
    }

    virtual BOOL enable_AuxHeadset(BOOL bEnable)
    {
        CHardwareAudioBridge::enable_AuxHeadset(bEnable);
        if (bEnable)
            {
            m_RequestAudioRoute = kAudioRoute_AuxHeadset;
            }
        else
            {
            m_RequestAudioRoute = kAudioRoute_Handset;
            }
        update_AudioRouting();
        return TRUE;
    }

	// 	// #####, Michael  ModemVoiceCall
    virtual BOOL enable_ModemVoiceCall(BOOL bEnable)
    {
        CHardwareAudioBridge::enable_ModemVoiceCall(bEnable);
        if (bEnable)
            {
            m_RequestAudioRoute = kAudioRoute_ModemVoiceCall;
            }
        else
            {
            m_RequestAudioRoute = kAudioRoute_Headset;
            }
        update_AudioRouting();
        return TRUE;
    }


    virtual BOOL enable_Speaker(BOOL bEnable)
    {
        CHardwareAudioBridge::enable_Speaker(bEnable);
        update_AudioRouting();
        return TRUE;
    }
    /*
    virtual void notify_BTHeadsetAttached(DWORD bAttached)
    {
        //CHardwareAudioBridge::notify_BTHeadsetAttached(bAttached);
        //update_AudioRouting();
		RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S, AG event = 0x%X\r\n",__FUNCTION__, bAttached));
        return;
    }*/

    virtual void notify_ExtAudioAttached(DWORD bAttached)
    {
        //CHardwareAudioBridge::notify_BTHeadsetAttached(bAttached);
        //update_AudioRouting();
		RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S, AG event = 0x%X\r\n",__FUNCTION__, bAttached));
        return;
    }

    virtual void notify_HdmiAudioAttached(BOOL bAttached)
    {
        CHardwareAudioBridge::notify_HdmiAudioAttached(bAttached);
        update_AudioRouting();
    }

    virtual BOOL query_SpeakerEnable() const
    {
        return m_CurrentAudioRoute == kAudioRoute_Speaker;
    }

    virtual BOOL query_CarkitEnable() const   
    { 
        return m_CurrentAudioRoute == kAudioRoute_Carkit; 
    }

    
    virtual BOOL query_HeadsetEnable() const
    {
        return m_CurrentAudioRoute == kAudioRoute_Headset ||
               m_CurrentAudioRoute == kAudioRoute_AuxHeadset ||
               m_CurrentAudioRoute == kAudioRoute_BTHeadset;
    }   

	virtual BOOL query_BTHeadsetEnable() const
	{
     RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S, m_CurrentAudioRoute = %d, query = %d\r\n",__FUNCTION__, m_CurrentAudioRoute, (m_CurrentAudioRoute == kAudioRoute_BTHeadset) ));
     return m_CurrentAudioRoute == kAudioRoute_BTHeadset;
	}

	virtual BOOL query_ExtAudioEnable() const
	{
     RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S, m_CurrentAudioRoute = %d, query = %d\r\n",__FUNCTION__, m_CurrentAudioRoute, (m_CurrentAudioRoute == kAudioRoute_BTHeadset) ));
     return m_CurrentAudioRoute == kAudioRoute_ExtAudio;
	}

	//// #####, Michael  ModemVoiceCall
   // virtual BOOL query_ModemVoiceEnable() const
   // {
   //     return m_CurrentAudioRoute == kAudioRoute_ModemVoiceCall;
   // }

#if defined(SYSGEN_BTH_AG)
	DWORD GetBthAudioRecGain(LONG *pGain) { return GetBthAudioRecGainHwCodec(m_hHwCodec, pGain); }
	DWORD GetBthAudioRecMute(BOOL *pMute) { return GetBthAudioRecMuteHwCodec(m_hHwCodec, pMute); }
	DWORD SetBthAudioRecMute(BOOL lMute) { return SetBthAudioRecMuteHwCodec(m_hHwCodec, lMute); }

	DWORD GetBthAudioPlayVol(LONG *pVolume) { return GetBthAudioPlayVolHwCodec(m_hHwCodec, pVolume); }
	DWORD GetBthAudioPlayMute(BOOL *pMute) { return GetBthAudioPlayMuteHwCodec(m_hHwCodec, pMute); }
	DWORD SetBthAudioPlayMute(BOOL lMute) { return SetBthAudioPlayMuteHwCodec(m_hHwCodec, lMute); }

	BOOL query_BlutoothAudioEnable() 
	{ 
		return m_CurrentAudioRoute == kAudioRoute_BluetoothAudio;
	}

	BOOL enable_BluetoothAudio(BOOL bEnable) 
	{ 
		RETAILMSG(1, (L"WAV:%S, bEnable = %d, m_bBTHeadsetAttached = %d\r\n",__FUNCTION__, bEnable, m_bBTHeadsetAttached));
		//CHardwareAudioBridge::enable_ExtAudio(bEnable);
        if (bEnable)
		{
			if (!m_bBTHeadsetAttached)
			{
				RETAILMSG(1, (L"WAV:%S, NOT ATTACHED!!!\r\n",__FUNCTION__));
				return FALSE;
			}

            m_RequestAudioRoute = kAudioRoute_BluetoothAudio;
		}
        else
            m_RequestAudioRoute = kAudioRoute_Handset;

		RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:%S, bEnable = %d, m_RequestAudioRoute = %d\r\n",__FUNCTION__, bEnable, m_RequestAudioRoute));
        update_AudioRouting();
        return TRUE;
	}
#endif
};


#endif //__OMAP35XX_HWBRIDGE_H__
