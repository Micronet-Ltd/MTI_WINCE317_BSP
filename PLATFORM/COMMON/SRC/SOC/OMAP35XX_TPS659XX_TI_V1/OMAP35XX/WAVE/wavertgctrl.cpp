//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
#include <windows.h>
#include <wavedev.h>
#include <mmreg.h>
#include <mmddk.h>
#include "debug.h"
#include "wavemain.h"
#include "audioctrl.h"
#include "Audiolin.h"
#include "mixermgr.h"
#include "strmctxt.h"
#include "strmmgr.h"
#include "audiomgr.h"
#include <WaveTopologyGuids.h>

//------------------------------------------------------------------------------
//
// Device topology.
//

//
// Sample output and input device topology information.  Update these to
// reflect the actual topology of your hardware.
//

enum
{
    WAVEOUT_EPIDX_DEFAULT = 0,
    WAVEOUT_EPIDX_RECEIVER,
    WAVEOUT_EPIDX_SPEAKERPHONE,
    WAVEOUT_EPIDX_HEADSET,
    WAVEOUT_EPIDX_HEADPHONES,
    WAVEOUT_EPIDX_CARKIT,
    WAVEOUT_EPIDX_BTH_SCO
};

enum
{
    WAVEIN_EPIDX_DEFAULT = 0,
    WAVEIN_EPIDX_SPEAKERPHONE,
    WAVEIN_EPIDX_HEADSET,
    WAVEIN_EPIDX_CARKIT,
    WAVEIN_EPIDX_BTH_SCO
};

static const DTP_ENDPOINT_DESCRIPTOR c_rgOutputEndpointDescriptor[] =
{
    // Index 0 - Default
    {
        WAVEOUT_EPIDX_DEFAULT,              // Endpoint index
        WAVEOUT_ENDPOINT_DEFAULT,           // Endpoint GUID
        FALSE,                              // Not removable
        TRUE,                               // Always attached
    },
    // Index 1 - Receiver
    {
        WAVEOUT_EPIDX_RECEIVER,             // Endpoint index
        WAVEOUT_ENDPOINT_RECEIVER,          // Endpoint GUID
        FALSE,                              // Not removable
        TRUE,                               // Always attached
    },
    // Index 2 - Speakerphone
    {
        WAVEOUT_EPIDX_SPEAKERPHONE,         // Endpoint index
        WAVEOUT_ENDPOINT_SPEAKERPHONE,      // Endpoint GUID
        FALSE,                              // Not removable
        TRUE,                               // Always attached
    },
    // Index 3 - Headset
    {
        WAVEOUT_EPIDX_HEADSET,              // Endpoint index
        WAVEOUT_ENDPOINT_HEADSET,           // Endpoint GUID
        TRUE,                               // Removable
        FALSE,                              // Unattached - To be set during query
    },
    // Index 4 - Headphones
    {
        WAVEOUT_EPIDX_HEADPHONES,           // Endpoint index
        WAVEOUT_ENDPOINT_HEADPHONES,        // Endpoint GUID
        TRUE,                               // Removable
        FALSE,                              // Unattached - To be set during query
    },
    // Index 4 - Car Kit
    {
        WAVEOUT_EPIDX_CARKIT,               // Endpoint index
        WAVEOUT_ENDPOINT_CARKIT,            // Endpoint GUID
        TRUE,                               // Removable
        FALSE,                              // Unattached - To be set during query
    },
    // Index 5 - BTH SCO
    {
        WAVEOUT_EPIDX_BTH_SCO,              // Endpoint index
        WAVEOUT_ENDPOINT_BTH_SCO,           // Endpoint GUID
        TRUE,                               // Removable
        FALSE,                              // Unattached - To be set during query
    },
};

static const DTP_ENDPOINT_DESCRIPTOR c_rgInputEndpointDescriptor[] =
{
    // Index 0 - Default
    {
        WAVEIN_EPIDX_DEFAULT,               // Endpoint index
        WAVEIN_ENDPOINT_DEFAULT,            // Endpoint GUID
        FALSE,                              // Not removable
        TRUE,                               // Always attached
    },
    // Index 1 - Speakerphone
    {
        WAVEIN_EPIDX_SPEAKERPHONE,          // Endpoint index
        WAVEIN_ENDPOINT_SPEAKERPHONE,       // Endpoint GUID
        FALSE,                              // Not removable
        TRUE,                               // Always attached
    },
    // Index 2 - Headset
    {
        WAVEIN_EPIDX_HEADSET,               // Endpoint index
        WAVEIN_ENDPOINT_HEADSET,            // Endpoint GUID
        TRUE,                               // Removable
        FALSE,                              // Unattached - To be set during query
    },
    // Index 3 - Car Kit
    {
        WAVEIN_EPIDX_CARKIT,                // Endpoint index
        WAVEIN_ENDPOINT_CARKIT,             // Endpoint GUID
        TRUE,                               // Removable
        FALSE,                              // Unattached - To be set during query
    },
    // Index 4 - BTH SCO
    {
        WAVEIN_EPIDX_BTH_SCO,               // Endpoint index
        WAVEIN_ENDPOINT_BTH_SCO,            // Endpoint GUID
        TRUE,                               // Removable
        FALSE,                              // Unattached - To be set during query
    },
};

static const DTP_DEVICE_DESCRIPTOR c_OutputDeviceDescriptor =
{
    WAVE_DEVCLASS_SYSTEM_PRIMARY,           // Device class GUID
    _countof(c_rgOutputEndpointDescriptor)  // Number of endpoints
};

static const DTP_DEVICE_DESCRIPTOR c_InputDeviceDescriptor =
{
    WAVE_DEVCLASS_SYSTEM_PRIMARY,           // Device class GUID
    _countof(c_rgInputEndpointDescriptor)   // Number of endpoints
};

DWORD
CStreamManager::GetOutputDeviceDescriptor(
    PDTP_DEVICE_DESCRIPTOR pDeviceDescriptor
    )
{
    memcpy(pDeviceDescriptor, &c_OutputDeviceDescriptor, sizeof(*pDeviceDescriptor));
    return MMSYSERR_NOERROR;
}

DWORD
CStreamManager::GetOutputEndpointDescriptor(
    DWORD dwIndex,
    PDTP_ENDPOINT_DESCRIPTOR pEndpointDescriptor
    )
{
    DWORD mmRet = MMSYSERR_INVALPARAM;

    if (dwIndex < _countof(c_rgOutputEndpointDescriptor))
    {
        memcpy(pEndpointDescriptor, &c_rgOutputEndpointDescriptor[dwIndex],
            sizeof(*pEndpointDescriptor));

        // Set the attached state of removable endpoints.
        if (pEndpointDescriptor->fRemovable)
        {
            switch (pEndpointDescriptor->dwIndex)
            {
            case WAVEOUT_EPIDX_HEADSET:
                pEndpointDescriptor->fAttached = m_fHeadsetAttached;
                break;
            case WAVEOUT_EPIDX_HEADPHONES:
                pEndpointDescriptor->fAttached = m_fHeadphonesAttached;
                break;
            case WAVEOUT_EPIDX_CARKIT:
                pEndpointDescriptor->fAttached = m_fCarKitAttached;
                break;
            case WAVEOUT_EPIDX_BTH_SCO:
                pEndpointDescriptor->fAttached = m_fBthScoEnabled;
                break;
            default:
                break;
            }
        }

        mmRet = MMSYSERR_NOERROR;
    }

    return mmRet;
}

DWORD
CStreamManager::GetInputDeviceDescriptor(
    PDTP_DEVICE_DESCRIPTOR pDeviceDescriptor
    )
{
    memcpy(pDeviceDescriptor, &c_InputDeviceDescriptor, sizeof(*pDeviceDescriptor));
    return MMSYSERR_NOERROR;
}

DWORD
CStreamManager::GetInputEndpointDescriptor(
    DWORD dwIndex,
    PDTP_ENDPOINT_DESCRIPTOR pEndpointDescriptor
    )
{
    DWORD mmRet = MMSYSERR_INVALPARAM;

    if (dwIndex < _countof(c_rgInputEndpointDescriptor))
    {
        memcpy(pEndpointDescriptor, &c_rgInputEndpointDescriptor[dwIndex],
            sizeof(*pEndpointDescriptor));

        // Set the attached state of removable endpoints.
        if (pEndpointDescriptor->fRemovable)
        {
            switch (pEndpointDescriptor->dwIndex)
            {
            case WAVEIN_EPIDX_HEADSET:
                pEndpointDescriptor->fAttached = m_fHeadsetAttached;
                break;
            case WAVEIN_EPIDX_CARKIT:
                pEndpointDescriptor->fAttached = m_fCarKitAttached;
                break;
            case WAVEIN_EPIDX_BTH_SCO:
                pEndpointDescriptor->fAttached = m_fBthScoEnabled;
                break;
            default:
                break;
            }
        }

        mmRet = MMSYSERR_NOERROR;
    }

    return mmRet;
}

//------------------------------------------------------------------------------
//
// Endpoint notifications.
//

DWORD
CStreamManager::SendEndpointChangeNotification(
    DtpNotify *pDtpNotify,
    BOOL fAttached,
    DWORD dwEpIdx
    )
{
    // Set up device topology notification message for endpoint change.
    DTP_EVTMSG_ENDPOINT_CHANGE msg = {0};
    msg.hdr.dwEvtType = DTP_EVT_ENDPOINT_CHANGE;
    msg.hdr.cbMsg = sizeof(msg);
    msg.fAttached = fAttached;
    msg.dwIndex = dwEpIdx;

    // Send the notification.
    return pDtpNotify->SendDtpNotifications(&msg, sizeof(msg));
}

DWORD
CStreamManager::NotifyHeadsetAttached(
    BOOL fAttached
    )
{
    if (m_fHeadsetAttached != fAttached)
    {
        m_fHeadsetAttached = fAttached;

        // Send headset endpoint notifications to the Audio Routing Manager
        // for both output and input.
        SendEndpointChangeNotification(
            &m_OutputDtpNotify, fAttached, WAVEOUT_EPIDX_HEADSET);
        SendEndpointChangeNotification(
            &m_InputDtpNotify, fAttached, WAVEIN_EPIDX_HEADSET);
    }

    return MMSYSERR_NOERROR;
}

DWORD
CStreamManager::NotifyHeadphonesAttached(
    BOOL fAttached
    )
{
    if (m_fHeadphonesAttached != fAttached)
    {
        m_fHeadphonesAttached = fAttached;

        // Send headphones endpoint notification to the Audio Routing Manager
        // for output only.
        SendEndpointChangeNotification(
            &m_OutputDtpNotify, fAttached, WAVEOUT_EPIDX_HEADPHONES);
    }

    return MMSYSERR_NOERROR;
}

DWORD
CStreamManager::NotifyCarKitAttached(
    BOOL fAttached
    )
{
    if (m_fCarKitAttached != fAttached)
    {
        m_fCarKitAttached = fAttached;

        // Send car kit endpoint notifications to the Audio Routing Manager
        // for both output and input.
        SendEndpointChangeNotification(
            &m_OutputDtpNotify, fAttached, WAVEOUT_EPIDX_CARKIT);
        SendEndpointChangeNotification(
            &m_InputDtpNotify, fAttached, WAVEIN_EPIDX_CARKIT);
    }

    return MMSYSERR_NOERROR;
}

DWORD
CStreamManager::NotifyBthScoEnabled(
    BOOL fEnabled
    )
{
    if (m_fBthScoEnabled != fEnabled)
    {
        m_fBthScoEnabled = fEnabled;

        // Send SCO endpoint notifications to the Audio Routing Manager
        // for both output and input.
        SendEndpointChangeNotification(
            &m_OutputDtpNotify, fEnabled, WAVEOUT_EPIDX_BTH_SCO);
        SendEndpointChangeNotification(
            &m_InputDtpNotify, fEnabled, WAVEIN_EPIDX_BTH_SCO);
    }

    return MMSYSERR_NOERROR;
}

//------------------------------------------------------------------------------
//
// Audio routing control.
//

/*
// Not supported, cannot find definition for PRTGCTRL_ENDPOINT_ROUTING

DWORD
CStreamManager::RouteSystemOutputEndpoints(
    PRTGCTRL_ENDPOINT_ROUTING pEndpointRouting
    )
{
    //
    // Route sytem audio output (playback) to the selected output endpoints.
    //
    switch(*pEndpointRouting->EndpointSelect.rgdwIndex)
        {
        case WAVEOUT_EPIDX_DEFAULT:
            get_HardwareAudioBridge()->enable_Headset(m_fHeadsetAttached);
            break;

        case WAVEOUT_EPIDX_HEADSET:
            get_HardwareAudioBridge()->enable_Headset(m_fHeadsetAttached);
            break;

        default:
            return MMSYSERR_NOTSUPPORTED;
        }

    return MMSYSERR_NOERROR;
}

DWORD
CStreamManager::RouteSystemInputEndpoints(
    PRTGCTRL_ENDPOINT_ROUTING pEndpointRouting
    )
{
    //
    // Route the selected input endpoints as the source for system audio input (record).
    //
    switch(*pEndpointRouting->EndpointSelect.rgdwIndex)
        {
        case WAVEIN_EPIDX_DEFAULT:
            get_HardwareAudioBridge()->enable_Headset(m_fHeadsetAttached);
            break;

        case WAVEIN_EPIDX_HEADSET:
            get_HardwareAudioBridge()->enable_Headset(m_fHeadsetAttached);
            break;

        default:
            return MMSYSERR_NOTSUPPORTED;
        }

    return MMSYSERR_NOERROR;
}

DWORD
CStreamManager::RouteCellularOutputEndpoints(
    PRTGCTRL_ENDPOINT_ROUTING pEndpointRouting
    )
{
    //
    // 1. Use pCellularAudio.fEnable to determine whether to enable/disable
    //    audio routing for the cellular downlink path.
    //

    //
    // 2. If enabled, route cellular downlink path to the endpoints selected in
    //    pCellularAudio.EndpointSelect.
    //

    return MMSYSERR_NOERROR;
}

DWORD
CStreamManager::RouteCellularInputEndpoints(
    PRTGCTRL_ENDPOINT_ROUTING pEndpointRouting
    )
{
    //
    // 1. Use pCellularAudio.fEnable to determine whether to enable/disable
    //    audio routing for the cellular uplink path.
    //

    //
    // 2. If enabled, route the input endpoints selected in pCellularAudio.EndpointSelect
    //    to the cellular uplink path.
    //

    return MMSYSERR_NOERROR;
}

*/
