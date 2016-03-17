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

#ifndef __OSTRMMGR_H__
#define __OSTRMMGR_H__


//------------------------------------------------------------------------------
//
//  Audio mixer control, expose microphone volume control
//
class CAudioControl_MasterVolume :
                       public CAudioControlType<MIXERCONTROL_CONTROLTYPE_VOLUME>
{
public:
    virtual WCHAR const* get_Name() const { return L"Master Volume"; }
    virtual WCHAR const* get_ShortName() const { return L"Master Volume"; }
    virtual DWORD get_StatusFlag() const
    {
        return MIXERCONTROL_CONTROLF_DISABLED;
    }

public:
    virtual void copy_ControlInfo(MIXERCONTROL *pControlInfo)
    {
        CAudioControlType<MIXERCONTROL_CONTROLTYPE_VOLUME>::
            copy_ControlInfo(pControlInfo);

        pControlInfo->Bounds.dwMinimum = LOGICAL_VOLUME_MIN;
        pControlInfo->Bounds.dwMaximum = LOGICAL_VOLUME_MAX;
        pControlInfo->Metrics.cSteps   = LOGICAL_VOLUME_STEPS;
    }
};


//------------------------------------------------------------------------------
//
//  Audio mixer control, expose microphone volume control
//
class CAudioControl_MasterMute :
                       public CAudioControlType<MIXERCONTROL_CONTROLTYPE_MUTE>
{
public:
    virtual WCHAR const* get_Name() const { return L"Master Mute"; }
    virtual WCHAR const* get_ShortName() const { return L"Mute"; }

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
class COutputStreamManager : public CStreamManager,
                             public CAudioLine_WavOutVolume
{
    //--------------------------------------------------------------------------
    // member variables
    //
public:

    CAudioControl_MasterVolume  m_VolumeControl;
    CAudioControl_MasterMute    m_MuteControl;


    //--------------------------------------------------------------------------
    // Constructor
    //
public:
    COutputStreamManager() : CStreamManager(),
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
    virtual WCHAR const* get_Name() const        { return L"Master Volume"; }
    virtual WCHAR const* get_ShortName() const   { return L"Master Volume"; }

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

#endif //__OSTRMMGR_H__