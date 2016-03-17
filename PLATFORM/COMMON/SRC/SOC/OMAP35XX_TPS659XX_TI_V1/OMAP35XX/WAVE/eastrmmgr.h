#ifndef __EASTRMMGR_H__
#define __EASTRMMGR_H__

//------------------------------------------------------------------------------
//
//  External Audio mixer control, External Audio On/Off control
//

class CAudioControl_ExtAudioOnOff :
                       public CAudioControlType<MIXERCONTROL_CONTROLTYPE_ONOFF>
{
public:
    virtual WCHAR const* get_Name() const { return L"External Audio OnOff"; }
    virtual WCHAR const* get_ShortName() const { return L"ExtAudio OnOff"; }

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


class CExtAudioStreamManager : public CStreamManager,
                              public CAudioLine_ExtAudio
{
    //--------------------------------------------------------------------------
    // member variables
    //
public:

	CAudioControl_ExtAudioOnOff      m_ExtAudioOnOffControl;

    //--------------------------------------------------------------------------
    // Constructor
    //
public:
    CExtAudioStreamManager() : CStreamManager(),
                                  CAudioLine_ExtAudio() {}


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
    virtual WCHAR const* get_Name() const        { return L"External Audio"; }
    virtual WCHAR const* get_ShortName() const   { return L"ExtAudio"; }

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
//  External Audio mixer control
//
class CAudioControl_ExtMicMute :
                      public CAudioControlType<MIXERCONTROL_CONTROLTYPE_MUTE>
{
public:
    virtual WCHAR const* get_Name() const { return L"External Microphone Mute"; }
    virtual WCHAR const* get_ShortName() const { return L"ExtMicMute"; }

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


#define  EXT_MICROPHONE_ANALOG_GAIN_MIN   0
#define  EXT_MICROPHONE_ANALOG_GAIN_MAX   30

class CAudioControl_ExtMicAnalogGain :
					  public CAudioControlType<MIXERCONTROL_CONTROLTYPE_DECIBELS>
{
public:
	virtual WCHAR const* get_Name() const { return L"External Microphone Analog Gain"; }
	virtual WCHAR const* get_ShortName() const { return L"ExtMicAnGain"; }

public:
	virtual void copy_ControlInfo(MIXERCONTROL *pControlInfo)
	{
		CAudioControlType<MIXERCONTROL_CONTROLTYPE_DECIBELS>::
			copy_ControlInfo(pControlInfo);

		pControlInfo->Bounds.dwMinimum = EXT_MICROPHONE_ANALOG_GAIN_MIN;
		pControlInfo->Bounds.dwMaximum = EXT_MICROPHONE_ANALOG_GAIN_MAX;
		pControlInfo->Metrics.cSteps = 6;
	}
};

#define  EXT_MICROPHONE_DIGITAL_LEVEL_MIN   0
#define  EXT_MICROPHONE_DIGITAL_LEVEL_MAX   31

class CAudioControl_ExtMicDigGain :
					  public CAudioControlType<MIXERCONTROL_CONTROLTYPE_VOLUME>
{
public:
	virtual WCHAR const* get_Name() const { return L"External Microphone Digital Gain"; }
	virtual WCHAR const* get_ShortName() const { return L"ExtMicDigGain"; }

public:
	virtual void copy_ControlInfo(MIXERCONTROL *pControlInfo)
	{
		CAudioControlType<MIXERCONTROL_CONTROLTYPE_VOLUME>::
			copy_ControlInfo(pControlInfo);

		pControlInfo->Bounds.dwMinimum = EXT_MICROPHONE_DIGITAL_LEVEL_MIN;
		pControlInfo->Bounds.dwMaximum = EXT_MICROPHONE_DIGITAL_LEVEL_MAX;
		pControlInfo->Metrics.cSteps = 1;
	}
};

//------------------------------------------------------------------------------
//
//  Represents the external microphone logical input mixer line
//
class CExtMicrophoneInputSource : public CAudioLineBase
	                           
{
    //--------------------------------------------------------------------------
    // member variables
    //
public:

    CExtAudioStreamManager          *m_ExtAudioStreamManager;
    CAudioControl_ExtMicMute         m_ExtMicMuteControl;
	CAudioControl_ExtMicAnalogGain   m_ExtMicAnalogGainControl;
    CAudioControl_ExtMicDigGain      m_ExtMicDigGainControl;


    //--------------------------------------------------------------------------
    // Constructor
    //
public:
    CExtMicrophoneInputSource() : CAudioLineBase()
    {
        m_ComponentType = MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE;
        m_countChannels = 1;
        m_TargetType = MIXERLINE_TARGETTYPE_WAVEIN;
        m_ffLineStatus = MIXERLINE_LINEF_ACTIVE | MIXERLINE_LINEF_SOURCE;
    }

    //--------------------------------------------------------------------------
    // CAudioLine specific methods
    //
public:
    virtual WCHAR const* get_Name() const        { return L"External Microphone"; }
    virtual WCHAR const* get_ShortName() const   { return L"ExtMic"; }

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



#endif //__EASTRMMGR_H__