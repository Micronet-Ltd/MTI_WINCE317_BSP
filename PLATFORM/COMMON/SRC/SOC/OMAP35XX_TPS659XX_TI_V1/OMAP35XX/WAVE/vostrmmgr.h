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
// Portions Copyright (c) Micronet.  All rights reserved.
//
//------------------------------------------------------------------------------
//

#ifndef __VOSTRMMGR_H__
#define __VOSTRMMGR_H__


#define  VOUT_GAIN_MIN   -60
#define  VOUT_GAIN_MAX    12
#define  VOUT_GAIN_STEP   1

//------------------------------------------------------------------------------
//
//  Audio mixer control, Voice Out volume control
//
class CAudioControl_VoiceOutVolume :
                       public CAudioControlType<MIXERCONTROL_CONTROLTYPE_DECIBELS>
{
public:
    virtual WCHAR const* get_Name() const { return L"Voice Out Volume"; }
    virtual WCHAR const* get_ShortName() const { return L"V Out Vol"; }
    virtual DWORD get_StatusFlag() const
    {
        return MIXERCONTROL_CONTROLF_DISABLED;
    }

public:
    virtual void copy_ControlInfo(MIXERCONTROL *pControlInfo)
    {
        CAudioControlType<MIXERCONTROL_CONTROLTYPE_DECIBELS>::
            copy_ControlInfo(pControlInfo);

        pControlInfo->Bounds.dwMinimum = VOUT_GAIN_MIN;
        pControlInfo->Bounds.dwMaximum = VOUT_GAIN_MAX;
        pControlInfo->Metrics.cSteps   = VOUT_GAIN_STEP;
    }
};


//------------------------------------------------------------------------------
//
//  Audio mixer control, Voice Out mute control
//
class CAudioControl_VoiceOutMute :
                       public CAudioControlType<MIXERCONTROL_CONTROLTYPE_MUTE>
{
public:
    virtual WCHAR const* get_Name() const { return L"Voice Out Mute"; }
    virtual WCHAR const* get_ShortName() const { return L"VMute"; }

public:
    virtual void copy_ControlInfo(MIXERCONTROL *pControlInfo)
    {
        CAudioControlType<MIXERCONTROL_CONTROLTYPE_MUTE>::
            copy_ControlInfo(pControlInfo);

        pControlInfo->Bounds.dwMinimum = 0;
        pControlInfo->Bounds.dwMaximum = 1;
        pControlInfo->Metrics.cSteps =  0;
    }
};


//------------------------------------------------------------------------------
//
//  Audio mixer control, Voice Out On/Off control
//
class CAudioControl_VoiceOutOnOff :
                       public CAudioControlType<MIXERCONTROL_CONTROLTYPE_ONOFF>
{
public:
    virtual WCHAR const* get_Name() const { return L"Voice Out OnOff"; }
    virtual WCHAR const* get_ShortName() const { return L"VOnOff"; }

public:
    virtual void copy_ControlInfo(MIXERCONTROL *pControlInfo)
    {
        CAudioControlType<MIXERCONTROL_CONTROLTYPE_ONOFF>::
            copy_ControlInfo(pControlInfo);

        pControlInfo->Bounds.dwMinimum = 0;
        pControlInfo->Bounds.dwMaximum = 1;
        pControlInfo->Metrics.cSteps =  0;
    }
};




#define  VOICE_INPUT_VOL_MAX   ((1<<5) -1)
//------------------------------------------------------------------------------
//
//  Represents the Voice input mixer line
//

//------------------------------------------------------------------------------
//
//  Audio mixer control, Voice input Volume
//
class CAudioControl_VoiceInput :
                      public CAudioControlType<MIXERCONTROL_CONTROLTYPE_DECIBELS>
{
public:
    virtual WCHAR const* get_Name() const { return L"Voice Input Digital Gain"; }
    virtual WCHAR const* get_ShortName() const { return L"VInDigGain"; }

public:
    virtual void copy_ControlInfo(MIXERCONTROL *pControlInfo)
    {
        CAudioControlType<MIXERCONTROL_CONTROLTYPE_DECIBELS>::
            copy_ControlInfo(pControlInfo);

        pControlInfo->Bounds.dwMinimum = 0;
        pControlInfo->Bounds.dwMaximum = VOICE_INPUT_VOL_MAX;
        pControlInfo->Metrics.cSteps   = 1;
    }
};

//------------------------------------------------------------------------------
//
//  Audio mixer control, Voice Input mute control
//
class CAudioControl_VoiceInputMute :
                       public CAudioControlType<MIXERCONTROL_CONTROLTYPE_MUTE>
{
public:
    virtual WCHAR const* get_Name() const { return L"Voice Input Mute"; }
    virtual WCHAR const* get_ShortName() const { return L"VInMute"; }

public:
    virtual void copy_ControlInfo(MIXERCONTROL *pControlInfo)
    {
        CAudioControlType<MIXERCONTROL_CONTROLTYPE_MUTE>::
            copy_ControlInfo(pControlInfo);

        pControlInfo->Bounds.dwMinimum = 0;
        pControlInfo->Bounds.dwMaximum = 1;
        pControlInfo->Metrics.cSteps =  0;
    }
};




//------------------------------------------------------------------------------
//
//  Manages all input streams.  Represents the WAV-IN logical audio mixer line
//
class CVoiceStreamManager : public CStreamManager,
                                  public CAudioLine_WavOutVolume
{
    //--------------------------------------------------------------------------
    // member variables
    //
public:

    //CAudioControl_VoiceOutVolume  m_VoiceVolumeControl;
    //CAudioControl_VoiceOutMute    m_VoiceMuteControl;
    CAudioControl_VoiceOutOnOff   m_VoiceOnOffControl;

    //--------------------------------------------------------------------------
    // Constructor
    //
public:
    CVoiceStreamManager() : CStreamManager(),
                                  CAudioLine_WavOutVolume() {}


    //--------------------------------------------------------------------------
    // CStream specific methods
    //
public:
    BOOL IsSupportedFormat(LPWAVEFORMATEX lpFormat);
    StreamContext *create_Stream(LPWAVEOPENDESC lpWOD);
    virtual DWORD get_ExtDevCaps(PVOID pCaps, DWORD dwSize);
    virtual DWORD get_DevCaps(PVOID pCaps, DWORD dwSize);
    void StreamReadyToRender(StreamContext *pStreamContext);

    virtual DWORD GetProperty(PWAVEPROPINFO pPropInfo);
    virtual DWORD SetProperty(PWAVEPROPINFO pPropInfo);

    DWORD GetOutputProperty(PWAVEPROPINFO pPropInfo)
    {
        return CommonGetProperty(pPropInfo, FALSE);
    }

    DWORD SetOutputProperty(PWAVEPROPINFO pPropInfo)
    {
        return CommonSetProperty(pPropInfo, FALSE);
    }

    //--------------------------------------------------------------------------
    // CStreamCallback specific methods
    //
public:

    virtual DWORD copy_AudioData(void* pStart, DWORD nSize);


    //--------------------------------------------------------------------------
    // CAudioLine specific methods
    //
public:
    virtual WCHAR const* get_Name() const        { return L"Voice Modem"; }
    virtual WCHAR const* get_ShortName() const   { return L"VoiceM"; }

    virtual BOOL initialize_AudioLine(CAudioMixerManager *pAudioMixerManager);

    virtual DWORD get_AudioValue(
        CAudioControlBase *pControl,
        PMIXERCONTROLDETAILS pDetail,
        DWORD dwFlag) const;

    virtual DWORD put_AudioValue(
        CAudioControlBase *pControl,
        PMIXERCONTROLDETAILS pDetail,
        DWORD dwFlag);
};


class CVoiceInputSource : public CAudioLineBase
{
    //--------------------------------------------------------------------------
    // member variables
    //
public:

    CVoiceStreamManager           *m_pVoiceStreamManager;
    CAudioControl_VoiceInput       m_VoiceInputGain;
	CAudioControl_VoiceInputMute   m_VoiceInputMute;


    //--------------------------------------------------------------------------
    // Constructor
    //
public:
    CVoiceInputSource() : CAudioLineBase()
		                  
    {
        m_ComponentType = MIXERLINE_COMPONENTTYPE_SRC_DIGITAL;
        m_countChannels = 1;
        m_TargetType = MIXERLINE_TARGETTYPE_WAVEIN;
        m_ffLineStatus = MIXERLINE_LINEF_ACTIVE | MIXERLINE_LINEF_SOURCE;
    }

    //--------------------------------------------------------------------------
    // CAudioLine specific methods
    //
public:
    virtual WCHAR const* get_Name() const        { return L"Voice Input"; }
    virtual WCHAR const* get_ShortName() const   { return L"VIn"; }

    virtual BOOL initialize_AudioLine(CAudioMixerManager *pAudioMixerManager);

    virtual DWORD get_AudioValue(
        CAudioControlBase *pControl,
        PMIXERCONTROLDETAILS pDetail,
        DWORD dwFlag) const;

    virtual DWORD put_AudioValue(
        CAudioControlBase *pControl,
        PMIXERCONTROLDETAILS pDetail,
        DWORD dwFlag);

    virtual void set_LineStatus(DWORD lineStatus)
    {
       m_ffLineStatus = lineStatus;
	   RETAILMSG(1, (L"WAV:%S, lineStatus = %d\r\n",__FUNCTION__, lineStatus));
    }

};



class CVoiceOutputSource : public CAudioLineBase
{
    //--------------------------------------------------------------------------
    // member variables
    //
public:

    CVoiceStreamManager            *m_pVoiceStreamManager;
    CAudioControl_VoiceOutVolume    m_VoiceOutputGain;
	CAudioControl_VoiceOutMute      m_VoiceOutputMute;


    //--------------------------------------------------------------------------
    // Constructor
    //
public:
    CVoiceOutputSource() : CAudioLineBase()
		                  
    {
        m_ComponentType = MIXERLINE_COMPONENTTYPE_SRC_DIGITAL;
        m_countChannels = 1;
        m_TargetType = MIXERLINE_TARGETTYPE_WAVEIN;
        m_ffLineStatus = MIXERLINE_LINEF_ACTIVE | MIXERLINE_LINEF_SOURCE;
    }

    //--------------------------------------------------------------------------
    // CAudioLine specific methods
    //
public:
    virtual WCHAR const* get_Name() const        { return L"Voice Output"; }
    virtual WCHAR const* get_ShortName() const   { return L"VOut"; }

    virtual BOOL initialize_AudioLine(CAudioMixerManager *pAudioMixerManager);

    virtual DWORD get_AudioValue(
        CAudioControlBase *pControl,
        PMIXERCONTROLDETAILS pDetail,
        DWORD dwFlag) const;

    virtual DWORD put_AudioValue(
        CAudioControlBase *pControl,
        PMIXERCONTROLDETAILS pDetail,
        DWORD dwFlag);

    virtual void set_LineStatus(DWORD lineStatus)
    {
       m_ffLineStatus = lineStatus;
	   RETAILMSG(1, (L"WAV:%S, lineStatus = %d\r\n",__FUNCTION__, lineStatus));
    }

};



//------------------------------------------------------------------------------
//
//  Audio mixer control, Bluetooth Voice On/Off control
//
class CAudioControl_BTVoiceOnOff :
                       public CAudioControlType<MIXERCONTROL_CONTROLTYPE_ONOFF>
{
public:
    virtual WCHAR const* get_Name() const { return L"Bluetooth Voice OnOff"; }
    virtual WCHAR const* get_ShortName() const { return L"BTVOnOff"; }

public:
    virtual void copy_ControlInfo(MIXERCONTROL *pControlInfo)
    {
        CAudioControlType<MIXERCONTROL_CONTROLTYPE_ONOFF>::
            copy_ControlInfo(pControlInfo);

        pControlInfo->Bounds.dwMinimum = 0;
        pControlInfo->Bounds.dwMaximum = 1;
        pControlInfo->Metrics.cSteps =  0;
    }
};
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Audio mixer control, Bluetooth Voice In mute control
//
class CAudioControl_BTVoiceInMute :
                       public CAudioControlType<MIXERCONTROL_CONTROLTYPE_MUTE>
{
public:
    virtual WCHAR const* get_Name() const { return L"Bluetooth Voice In Mute"; }
    virtual WCHAR const* get_ShortName() const { return L"BTDIN Mute"; }

public:
    virtual void copy_ControlInfo(MIXERCONTROL *pControlInfo)
    {
        CAudioControlType<MIXERCONTROL_CONTROLTYPE_MUTE>::
            copy_ControlInfo(pControlInfo);

        pControlInfo->Bounds.dwMinimum = 0;
        pControlInfo->Bounds.dwMaximum = 1;
        pControlInfo->Metrics.cSteps =  0;
    }
};

#define  BTDIN_GAIN_MIN   -15
#define  BTDIN_GAIN_MAX    30
#define  BTDIN_GAIN_STEP   3

//------------------------------------------------------------------------------
//
//  Audio mixer control, Bluetooth Voice In volume control
//
class CAudioControl_BTVoiceInVolume :
                       public CAudioControlType<MIXERCONTROL_CONTROLTYPE_DECIBELS>
{
public:
    virtual WCHAR const* get_Name() const { return L"Bluetooth In Volume"; }
    virtual WCHAR const* get_ShortName() const { return L"BTDIN Vol"; }
    virtual DWORD get_StatusFlag() const
    {
        return MIXERCONTROL_CONTROLF_DISABLED;
    }

public:
    virtual void copy_ControlInfo(MIXERCONTROL *pControlInfo)
    {
        CAudioControlType<MIXERCONTROL_CONTROLTYPE_DECIBELS>::
            copy_ControlInfo(pControlInfo);

        pControlInfo->Bounds.dwMinimum = BTDIN_GAIN_MIN;
        pControlInfo->Bounds.dwMaximum = BTDIN_GAIN_MAX;
        pControlInfo->Metrics.cSteps   = BTDIN_GAIN_STEP;
    }
};


//------------------------------------------------------------------------------
//
//  Audio mixer control, Bluetooth Voice Out mute control
//
class CAudioControl_BTVoiceOutMute :
                       public CAudioControlType<MIXERCONTROL_CONTROLTYPE_MUTE>
{
public:
    virtual WCHAR const* get_Name() const { return L"Bluetooth Voice Out Mute"; }
    virtual WCHAR const* get_ShortName() const { return L"BTDOUT Mute"; }

public:
    virtual void copy_ControlInfo(MIXERCONTROL *pControlInfo)
    {
        CAudioControlType<MIXERCONTROL_CONTROLTYPE_MUTE>::
            copy_ControlInfo(pControlInfo);

        pControlInfo->Bounds.dwMinimum = 0;
        pControlInfo->Bounds.dwMaximum = 1;
        pControlInfo->Metrics.cSteps =  0;
    }
};


#define  BTDOUT_GAIN_MIN   -30
#define  BTDOUT_GAIN_MAX    15
#define  BTDOUT_GAIN_STEP   3

//------------------------------------------------------------------------------
//
//  Audio mixer control, Bluetooth Voice Out volume control
//
class CAudioControl_BTVoiceOutVolume :
                       public CAudioControlType<MIXERCONTROL_CONTROLTYPE_DECIBELS>
{
public:
    virtual WCHAR const* get_Name() const { return L"Bluetooth Out Volume"; }
    virtual WCHAR const* get_ShortName() const { return L"BTDOUT Vol"; }
    virtual DWORD get_StatusFlag() const
    {
        return MIXERCONTROL_CONTROLF_DISABLED;
    }

public:
    virtual void copy_ControlInfo(MIXERCONTROL *pControlInfo)
    {
        CAudioControlType<MIXERCONTROL_CONTROLTYPE_DECIBELS>::
            copy_ControlInfo(pControlInfo);

        pControlInfo->Bounds.dwMinimum = BTDOUT_GAIN_MIN;
        pControlInfo->Bounds.dwMaximum = BTDOUT_GAIN_MAX;
        pControlInfo->Metrics.cSteps   = BTDOUT_GAIN_STEP;
    }
};




class CBTVoiceStreamManager : public CStreamManager,
                              public CAudioLine_BTVoice
{
    //--------------------------------------------------------------------------
    // member variables
    //
public:

	CAudioControl_BTVoiceOnOff      m_BTVoiceOnOffControl;
#if 0
	CAudioControl_BTVoiceInMute     m_BTVoiceInMuteControl;
	CAudioControl_BTVoiceInVolume   m_BTVoiceInVolumeControl;
	CAudioControl_BTVoiceOutMute    m_BTVoiceOutMuteControl;
	CAudioControl_BTVoiceOutVolume  m_BTVoiceOutVolumeControl;				
#endif
    //--------------------------------------------------------------------------
    // Constructor
    //
public:
    CBTVoiceStreamManager() : CStreamManager(),
                                  CAudioLine_BTVoice() {}


    //--------------------------------------------------------------------------
    // CStream specific methods
    //
public:
    BOOL IsSupportedFormat(LPWAVEFORMATEX lpFormat);
    StreamContext *create_Stream(LPWAVEOPENDESC lpWOD);
    virtual DWORD get_ExtDevCaps(PVOID pCaps, DWORD dwSize);
    virtual DWORD get_DevCaps(PVOID pCaps, DWORD dwSize);
    void StreamReadyToRender(StreamContext *pStreamContext);

    virtual DWORD GetProperty(PWAVEPROPINFO pPropInfo);
    virtual DWORD SetProperty(PWAVEPROPINFO pPropInfo);

    DWORD GetOutputProperty(PWAVEPROPINFO pPropInfo)
    {
        return CommonGetProperty(pPropInfo, FALSE);
    }

    DWORD SetOutputProperty(PWAVEPROPINFO pPropInfo)
    {
        return CommonSetProperty(pPropInfo, FALSE);
    }

    //--------------------------------------------------------------------------
    // CStreamCallback specific methods
    //
public:

    virtual DWORD copy_AudioData(void* pStart, DWORD nSize);


    //--------------------------------------------------------------------------
    // CAudioLine specific methods
    //
public:
    virtual WCHAR const* get_Name() const        { return L"Bluetooth Voice Modem"; }
    virtual WCHAR const* get_ShortName() const   { return L"BTVoiceM"; }

    virtual BOOL initialize_AudioLine(CAudioMixerManager *pAudioMixerManager);

    virtual DWORD get_AudioValue(
        CAudioControlBase *pControl,
        PMIXERCONTROLDETAILS pDetail,
        DWORD dwFlag) const;

    virtual DWORD put_AudioValue(
        CAudioControlBase *pControl,
        PMIXERCONTROLDETAILS pDetail,
        DWORD dwFlag);
};


//------------------------------------------------------------------------------
//
//  Represents the Bluetooth Voice Input mixer line
//
class CBTVoiceInputSource : public CAudioLineBase
	                           
{
    //--------------------------------------------------------------------------
    // member variables
    //
public:

    CBTVoiceStreamManager          *m_pCBTVoiceStreamManager;
	CAudioControl_BTVoiceInMute     m_BTVoiceInMuteControl;
	CAudioControl_BTVoiceInVolume   m_BTVoiceInVolumeControl;


    //--------------------------------------------------------------------------
    // Constructor
    //
public:
    CBTVoiceInputSource() : CAudioLineBase()
    {
        m_ComponentType = MIXERLINE_COMPONENTTYPE_SRC_DIGITAL;
        m_countChannels = 1;
        m_TargetType = MIXERLINE_TARGETTYPE_UNDEFINED;
        m_ffLineStatus = MIXERLINE_LINEF_ACTIVE | MIXERLINE_LINEF_SOURCE;
    }

    //--------------------------------------------------------------------------
    // CAudioLine specific methods
    //
public:
    virtual WCHAR const* get_Name() const        { return L"Input from BT"; }
    virtual WCHAR const* get_ShortName() const   { return L"BTDIN"; }

    virtual BOOL initialize_AudioLine(CAudioMixerManager *pAudioMixerManager);

    virtual DWORD get_AudioValue(
        CAudioControlBase *pControl,
        PMIXERCONTROLDETAILS pDetail,
        DWORD dwFlag) const;

    virtual DWORD put_AudioValue(
        CAudioControlBase *pControl,
        PMIXERCONTROLDETAILS pDetail,
        DWORD dwFlag);

    virtual void set_LineStatus(DWORD lineStatus)
    {
       m_ffLineStatus = lineStatus;
	   RETAILMSG(1, (L"WAV:%S, lineStatus = %d\r\n",__FUNCTION__, lineStatus));
    }

};


//------------------------------------------------------------------------------
//
//  Represents the Bluetooth Voice Output mixer line
//
class CBTVoiceOutputSource : public CAudioLineBase
	                           
{
    //--------------------------------------------------------------------------
    // member variables
    //
public:

    CBTVoiceStreamManager          *m_pCBTVoiceStreamManager;
	CAudioControl_BTVoiceOutMute    m_BTVoiceOutMuteControl;
	CAudioControl_BTVoiceOutVolume  m_BTVoiceOutVolumeControl;				

    //--------------------------------------------------------------------------
    // Constructor
    //
public:
    CBTVoiceOutputSource() : CAudioLineBase()
    {
        m_ComponentType = MIXERLINE_COMPONENTTYPE_SRC_DIGITAL;
        m_countChannels = 1;
        m_TargetType = MIXERLINE_TARGETTYPE_UNDEFINED;
        m_ffLineStatus = MIXERLINE_LINEF_ACTIVE | MIXERLINE_LINEF_SOURCE;
    }

    //--------------------------------------------------------------------------
    // CAudioLine specific methods
    //
public:
    virtual WCHAR const* get_Name() const        { return L"Output from Codec"; }
    virtual WCHAR const* get_ShortName() const   { return L"BTDOUT"; }

    virtual BOOL initialize_AudioLine(CAudioMixerManager *pAudioMixerManager);

    virtual DWORD get_AudioValue(
        CAudioControlBase *pControl,
        PMIXERCONTROLDETAILS pDetail,
        DWORD dwFlag) const;

    virtual DWORD put_AudioValue(
        CAudioControlBase *pControl,
        PMIXERCONTROLDETAILS pDetail,
        DWORD dwFlag);

    virtual void set_LineStatus(DWORD lineStatus)
    {
       m_ffLineStatus = lineStatus;
	   RETAILMSG(1, (L"WAV:%S, lineStatus = %d\r\n",__FUNCTION__, lineStatus));
    }

};

#endif //__VOSTRMMGR_H__