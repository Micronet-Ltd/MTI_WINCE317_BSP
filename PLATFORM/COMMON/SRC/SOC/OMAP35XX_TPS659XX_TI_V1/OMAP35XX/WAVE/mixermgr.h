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

#ifndef __MIXERMGR_H__
#define __MIXERMGR_H__

#define  WAVE_MIXER_MNGR_DBG  0

#include "linkqueue.h"

class CAudioManager;

//------------------------------------------------------------------------------
//
//  Behaves as a mediator for all audio mixer controls and lines.
//  This class strictly deals with managing audio mixer objects and
//  handling/routing audio line messages
//
class CAudioMixerManager
{
    //--------------------------------------------------------------------------
    // public typedefs, enums, and structs
    //
public:

    enum { kDriverVersion = DRIVER_VERSION };
    enum { kNoSource = 0x00FF };

    enum AudioMixerControl
        {
        kWavInVolumeControl     = 0,
        kWavOutVolumeControl,
        kWavInMuteControl,
        kWavOutMuteControl,

        kMicMuteControl,
        kMicAnalogVolumeControl,
		kMicDigitVolumeControl,

        kVoiceInVolumeControl,
        kVoiceOutVolumeControl,
        kVoiceInMuteControl,
        kVoiceOutMuteControl,

		kVoiceOutOnOffControl,

        kBTVoiceOnOffControl,
        kBTVoiceInMuteControl,
        kBTVoiceInVolumeControl,
        kBTVoiceOutMuteControl,
        kBTVoiceOutVolumeControl,

		kBTPlbOnOffControl,
	    kBTPlbOutMuteControl,
	    kBTPlbOutVolumeControl,

		kExtAudioOnOffControl,

        kExtMicMuteControl,
        kExtMicAnalogVolumeControl,
		kExtMicDigitVolumeControl,

#if defined(SYSGEN_BTH_AG)
		kBthAudioOnOffControl,
		kBthAudioPlayVolControl,
		kBthAudioPlayMuteControl,
		kBthAudioRecGainControl,
		kBthAudioRecMuteControl,
#endif

        kMixerControlCount
        };

    struct AudioMixerCallback_t
        {
        DWORD                   hmx;
        fnAudioMixerCallback   *pfnCallback;
        AudioMixerCallback_t   *pNext;
        };

    //--------------------------------------------------------------------------
    // member variables
    //
protected:

    // NOTE:
    //  The MixerManager off loads access serialization onto the individual
    // AudioLines and AudioControls which need serialization?
    //

    WORD                        m_counterDestinationLines;

    CAudioManager              *m_pAudioManager;

    //

    // registered audio lines
    //
    QUEUE_ENTRY                *m_pSourceAudioLines;
    QUEUE_ENTRY                *m_pDestinationAudioLines;

    // special mixer controls for overall system volume and mute
    // (for both destination and source volume)
    //
    CAudioControlBase          *m_prgAudioMixerControls[kMixerControlCount];

    // callbacks
    //
    AudioMixerCallback_t       *m_pAudioMixerCallbacks;


    //--------------------------------------------------------------------------
    // constructor/destructor
    //
public:
    CAudioMixerManager(CAudioManager* pAudioManager);


    //--------------------------------------------------------------------------
    // public inline methods
    //
public:

    DWORD get_AudioMixerVersion() const     { return kDriverVersion;}
    CAudioManager* get_AudioManager() const   { return m_pAudioManager; }
    WCHAR const* get_AudioMixerName() const
    {
        static WCHAR const* pDeviceName = L"Audio Mixer";
        return pDeviceName;
    }

    // provides a quick path to manipulating common audio control mixers
    //

    // Sets wave volume using mixer volume controls.
    DWORD put_AudioVolume(AudioMixerControl id, DWORD dwGain) const
    {
        DWORD mmRet = MMSYSERR_NOERROR;

		RETAILMSG(WAVE_MIXER_MNGR_DBG, (L"WAV:+%S\r\n",__FUNCTION__));
        ASSERT(m_prgAudioMixerControls[id]);
        MIXERCONTROLDETAILS ctrlDetail = {0};
        MIXERCONTROLDETAILS_UNSIGNED rgValue[2];
        DWORD cChannels = m_prgAudioMixerControls[id]->get_AudioLine()->get_ChannelCount();

        ctrlDetail.cbStruct = sizeof(MIXERCONTROLDETAILS);
        ctrlDetail.dwControlID = m_prgAudioMixerControls[id]->get_ControlId();
        ctrlDetail.cChannels = cChannels;
        ctrlDetail.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
        ctrlDetail.paDetails = rgValue;

        DWORD dwGainL = dwGain & 0xFFFF;
        DWORD dwGainR = (dwGain >> 16) & 0xFFFF;

		RETAILMSG(WAVE_MIXER_MNGR_DBG, (L"WAV:%S, dwGain = 0x%X\r\n",__FUNCTION__, dwGain));

        switch (cChannels)
        {
        case 2:
            // rgValue[0] is left channel and rgValue[1] is right channel.
            rgValue[0].dwValue = dwGainL;
            rgValue[1].dwValue = dwGainR;
            break;
        case 1:
            // rgValue[0] is average of left and right channels.
            rgValue[0].dwValue = (dwGainL + dwGainR) / 2;
            break;
        default:
            mmRet = MMSYSERR_INVALPARAM;
            goto Error;
        }

        // Set the control details.
        mmRet = m_prgAudioMixerControls[id]->put_Value(&ctrlDetail, 0);
        if (mmRet == MMSYSERR_NOERROR)
        {
            notify_AudioMixerCallbacks(MM_MIXM_CONTROL_CHANGE,
                m_prgAudioMixerControls[id]->get_ControlId()
                );
        }

Error:

		RETAILMSG(WAVE_MIXER_MNGR_DBG, (L"WAV:-%S, ret = 0X%x\r\n",__FUNCTION__, mmRet));
        return mmRet;
    }

    // Retrieves wave volume from mixer volume controls.
    DWORD get_AudioVolume(AudioMixerControl id, DWORD *pdwGain) const
    {
		RETAILMSG(WAVE_MIXER_MNGR_DBG, (L"WAV:+%S\r\n",__FUNCTION__));
        ASSERT(m_prgAudioMixerControls[id]);
        MIXERCONTROLDETAILS ctrlDetail = {0};
        MIXERCONTROLDETAILS_UNSIGNED rgValue[2] = {0};
        DWORD cChannels = m_prgAudioMixerControls[id]->get_AudioLine()->get_ChannelCount();

        ctrlDetail.cbStruct = sizeof(MIXERCONTROLDETAILS);
        ctrlDetail.dwControlID = m_prgAudioMixerControls[id]->get_ControlId();
        ctrlDetail.cChannels = cChannels;
        ctrlDetail.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
        ctrlDetail.paDetails = rgValue;

        DWORD mmRet = m_prgAudioMixerControls[id]->get_Value(&ctrlDetail, 0);
        if (mmRet == MMSYSERR_NOERROR)
        {
            DWORD dwGainL, dwGainR;

            switch (cChannels)
            {
            case 2:
                // rgValue[0] is left channel
                // rgValue[1] is right channel
                dwGainL = rgValue[0].dwValue;
                dwGainR = rgValue[1].dwValue;
                break;
            case 1:
                // rgValue[0] is mono gain.
                dwGainL = dwGainR = rgValue[0].dwValue;
                break;
            default:
                mmRet = MMSYSERR_INVALPARAM;
                goto Error;
            }

            // Construct wave gain.  Low word is left channel, and high word is right.
            *pdwGain = (dwGainR << 16) | dwGainL;
        }

Error:

		RETAILMSG(WAVE_MIXER_MNGR_DBG, (L"WAV:-%S, ret = 0X%x, gain = 0x%X\r\n",__FUNCTION__, mmRet, *pdwGain));
        return mmRet;
    }

    DWORD put_AudioMute(AudioMixerControl id, BOOL bMute) const
    {
        ASSERT(m_prgAudioMixerControls[id]);
        MIXERCONTROLDETAILS ctrlDetail;
        MIXERCONTROLDETAILS_BOOLEAN BooleanValue;

        ctrlDetail.cbStruct = sizeof(MIXERCONTROLDETAILS);
        ctrlDetail.cChannels = 1;
        ctrlDetail.paDetails = &BooleanValue;
        BooleanValue.fValue = bMute;

        DWORD mmRet = m_prgAudioMixerControls[id]->put_Value(&ctrlDetail, 0);
        if (mmRet == MMSYSERR_NOERROR)
            {
            notify_AudioMixerCallbacks(MM_MIXM_CONTROL_CHANGE,
                m_prgAudioMixerControls[id]->get_ControlId()
                );
            }

        return mmRet;
    }

    DWORD get_AudioMute(AudioMixerControl id, BOOL *pbMute) const
    {
        ASSERT(m_prgAudioMixerControls[id]);
        MIXERCONTROLDETAILS ctrlDetail;
        MIXERCONTROLDETAILS_BOOLEAN BooleanValue;

        ctrlDetail.cbStruct = sizeof(MIXERCONTROLDETAILS);
        ctrlDetail.cChannels = 1;
        ctrlDetail.paDetails = &BooleanValue;

        DWORD mmRet = m_prgAudioMixerControls[id]->put_Value(&ctrlDetail, 0);
        if (mmRet == MMSYSERR_NOERROR)
            {
// disable PREFAST warning for Using uninitialized memory 'BooleanValue'
#pragma warning (push)
#pragma warning (disable: 6001)
            *pbMute = BooleanValue.fValue;
#pragma warning (pop)
            }

        return mmRet;
    }

    //--------------------------------------------------------------------------
    // protected methods
    //
protected:

    // helper routines to retrieve stored mixer objects
    //
    CAudioLineBase* query_AudioLineByDestinationId(WORD DestinationId) const;
    CAudioLineBase* query_AudioLineBySourceAndDestinationId(WORD, WORD) const;
    CAudioLineBase* query_AudioLineByLineId(WORD LineId) const;
    CAudioLineBase* query_AudioLineByComponentType(DWORD ComponentType) const;
    CAudioLineBase* query_AudioLineByTargetType(DWORD TargetType) const;
    CAudioControlBase* query_ControlByControlId(DWORD ControlId) const;

    // helper routines to the ProcessMixerMessage
    //
    DWORD open_Device(PDWORD phDeviceId, PMIXEROPENDESC pMOD, DWORD dwFlags);
    DWORD close_Device(AudioMixerCallback_t *pAudioLineCB);
    DWORD get_DeviceCaps(PMIXERCAPS pCaps, DWORD dwSize);
    DWORD get_LineInfo(PMIXERLINE pDetail, DWORD dwFlags);
    DWORD get_LineControls(PMIXERLINECONTROLS pDetail, DWORD dwFlags);
    DWORD get_ControlDetails(PMIXERCONTROLDETAILS pDetail, DWORD dwFlags);
    DWORD put_ControlDetails(PMIXERCONTROLDETAILS pDetail, DWORD dwFlags);


    //--------------------------------------------------------------------------
    // public methods
    //
public:

    // setting default audio controls
    //
    void put_AudioMixerControl(AudioMixerControl id, CAudioControlBase* pCtrl);

    // helper routine to notify all listeners of mixer events
    //
    void notify_AudioMixerCallbacks(DWORD msg, DWORD ControlId) const;

    // FUTURE:
    //  Should implement unregistering of DestinationAudioLine and
    // SourceAudioLine.  Unregistering a destination line should
    // implicitly unregister all audio controls associated with the
    // the destination line.  Currently, this isn't a problem since
    // all mixer objects persist until the device is shutdown
    //

    // all mixer objects need to be registered with the mixer manager
    //
    BOOL register_DestinationAudioLine(CAudioLineBase* pDestination);
    BOOL register_SourceAudioLine(CAudioLineBase* pSource,
        CAudioLineBase* pDestination
        );

    BOOL process_MixerMessage(PMMDRV_MESSAGE_PARAMS pParams, DWORD* pdwResult);
};

#endif // __MIXERMGR_H__
