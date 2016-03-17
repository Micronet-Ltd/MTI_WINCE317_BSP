
#ifndef __BTSTRMMGR_H__
#define __BTSTRMMGR_H__

//------------------------------------------------------------------------------
//
//  Audio mixer control, Bluetooth Playback On/Off control
//

class CAudioControl_BTPlbOnOff :
                       public CAudioControlType<MIXERCONTROL_CONTROLTYPE_ONOFF>
{
public:
    virtual WCHAR const* get_Name() const { return L"Bluetooth Playback OnOff"; }
    virtual WCHAR const* get_ShortName() const { return L"BTPlbOnOff"; }

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
//  Audio mixer control, Bluetooth Playback Out mute control
//
class CAudioControl_BTPlbOutMute :
                       public CAudioControlType<MIXERCONTROL_CONTROLTYPE_MUTE>
{
public:
    virtual WCHAR const* get_Name() const { return L"Bluetooth Playback Mute"; }
    virtual WCHAR const* get_ShortName() const { return L"BTPlbOUT Mute"; }

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


#define  BTPLBOUT_GAIN_MIN   -30
#define  BTPLBOUT_GAIN_MAX    15
#define  BTPLBOUT_GAIN_STEP   3

//------------------------------------------------------------------------------
//
//  Audio mixer control, Bluetooth Playback Out volume control
//
class CAudioControl_BTPlbOutVolume :
                       public CAudioControlType<MIXERCONTROL_CONTROLTYPE_DECIBELS>
{
public:
    virtual WCHAR const* get_Name() const { return L"Bluetooth Playback Volume"; }
    virtual WCHAR const* get_ShortName() const { return L"BTPlbOUT Vol"; }
    virtual DWORD get_StatusFlag() const
    {
        return MIXERCONTROL_CONTROLF_DISABLED;
    }

public:
    virtual void copy_ControlInfo(MIXERCONTROL *pControlInfo)
    {
        CAudioControlType<MIXERCONTROL_CONTROLTYPE_DECIBELS>::
            copy_ControlInfo(pControlInfo);

        pControlInfo->Bounds.dwMinimum = BTPLBOUT_GAIN_MIN;
        pControlInfo->Bounds.dwMaximum = BTPLBOUT_GAIN_MAX;
        pControlInfo->Metrics.cSteps   = BTPLBOUT_GAIN_STEP;
    }
};


class CBTPlaybackStreamManager : public CStreamManager,
                              public CAudioLine_BTPlayback
{
    //--------------------------------------------------------------------------
    // member variables
    //
public:

	CAudioControl_BTPlbOnOff      m_BTPlbOnOffControl;

    //--------------------------------------------------------------------------
    // Constructor
    //
public:
    CBTPlaybackStreamManager() : CStreamManager(),
                                  CAudioLine_BTPlayback() {}


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
    virtual WCHAR const* get_Name() const        { return L"Bluetooth Playback"; }
    virtual WCHAR const* get_ShortName() const   { return L"BTPlb"; }

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
//  Represents the Bluetooth Voice Output mixer line
//
class CBTPlaybackOutputSource : public CAudioLineBase
	                           
{
    //--------------------------------------------------------------------------
    // member variables
    //
public:

    CBTPlaybackStreamManager        *m_pCBTPlaybackStreamManager;
	CAudioControl_BTPlbOutMute       m_BTPlbOutMuteControl;
	CAudioControl_BTPlbOutVolume     m_BTPlbOutVolumeControl;				

    //--------------------------------------------------------------------------
    // Constructor
    //
public:
    CBTPlaybackOutputSource() : CAudioLineBase()
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
    virtual WCHAR const* get_Name() const        { return L"BT playback from Codec"; }
    virtual WCHAR const* get_ShortName() const   { return L"BTPLBOUT"; }

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


#endif //__BTSTRMMGR_H__