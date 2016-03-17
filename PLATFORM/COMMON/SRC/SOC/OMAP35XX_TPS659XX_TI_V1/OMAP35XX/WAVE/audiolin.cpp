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

#include <windows.h>
#include <wavedev.h>
#include <mmddk.h>
#include <mmreg.h>
#include "debug.h"
#include "wavemain.h"
#include "audioctrl.h"
#include "audiolin.h"
#include "mixermgr.h"

//------------------------------------------------------------------------------
// CAudioLineBase
//
CAudioLineBase::CAudioLineBase() : m_pAudioControls(NULL)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+put_AudioLineIds()\r\n"));

    m_DeviceId = 0;
    m_countControls = 0;
    m_countConnections = 0;

    m_LineId = -1;
    m_SourceId = -1;
    m_DestinationId = -1;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-put_AudioLineIds()\r\n"));
}


//------------------------------------------------------------------------------
// copy_LineInfo
//
void
CAudioLineBase::put_AudioLineIds(
    WORD LineId,
    WORD DestinationId,
    WORD SourceId)
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+put_AudioLineIds(LineId=0x%04X, DestinationId=0x%04X, "
         L"SourceId=0x%04X)\r\n", LineId, DestinationId, SourceId)
        );

    m_LineId = LineId;
    m_DestinationId = DestinationId;
    m_SourceId = SourceId;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-put_AudioLineIds()\r\n"));
}


//------------------------------------------------------------------------------
// increment_ConnectionCount
//
void
CAudioLineBase::increment_ConnectionCount()
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+increment_ConnectionCount(m_countConnections=0x%08X)\r\n",
        m_countConnections)
        );

    // If a mixer has a source id then
    // this mix is not a valid destination
    // mixer
    //
    ASSERT(m_SourceId == 0);
    ++m_countConnections;

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-increment_ConnectionCount(m_countConnections=0x%08X)\r\n",
        m_countConnections)
        );
}


//------------------------------------------------------------------------------
// register_AudioControl
//
DWORD
CAudioLineBase::register_AudioControl(
    CAudioControlBase *pControl
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+register_AudioControl(pControl=0x%08X)\r\n", pControl)
        );

    // the control id is a concatonation of the line id and the control
    // number
    //
    DWORD ControlId = MXCONTROLID(get_LineId(), get_ControlCount());
    pControl->put_ControlId(ControlId, this);
    ++m_countControls;

    InsertTail(m_pAudioControls, pControl);

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-register_AudioControl(id=0x%08X)\r\n", ControlId));

    return ControlId;
}


//------------------------------------------------------------------------------
// copy_LineInfo
//
void
CAudioLineBase::copy_LineInfo(
    PMIXERLINE pMixerInfo
    ) const
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+copy_LineInfo(pMixerInfo=0x%08X)\r\n", pMixerInfo)
        );

    pMixerInfo->cChannels = get_ChannelCount();
    pMixerInfo->cConnections = get_ConnectionCount();
    pMixerInfo->cControls = get_ControlCount();
    pMixerInfo->dwComponentType = get_ComponentType();
    pMixerInfo->dwLineID = get_LineId();
    pMixerInfo->dwDestination = get_DestinationId();
    pMixerInfo->fdwLine = get_LineStatus();
    pMixerInfo->Target.dwDeviceID = get_DeviceId();
    pMixerInfo->Target.dwType = get_TargetType();

    pMixerInfo->Target.wMid = MM_MICROSOFT;
    pMixerInfo->Target.wPid = MM_MSFT_WSS_MIXER;
    pMixerInfo->Target.vDriverVersion = CAudioMixerManager::kDriverVersion;

    wcscpy(pMixerInfo->szName, get_Name());
    wcscpy(pMixerInfo->szShortName, get_ShortName());
    if (get_ProductName())
        {
        wcscpy(pMixerInfo->Target.szPname, get_ProductName());
        }
    else
        {
        *pMixerInfo->Target.szPname = NULL;
        }

    // UNDONE:
    // ???
    //
    pMixerInfo->dwSource = get_SourceId();

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-copy_LineInfo()\r\n"));
}


//------------------------------------------------------------------------------
// query_ControlByIndex
//
CAudioControlBase*
CAudioLineBase::query_ControlByIndex(
    int i
    ) const
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+query_ControlByIndex(i=%d)\r\n", i));

    int loopCounter = i;
    CAudioControlBase *pControl = (CAudioControlBase*)m_pAudioControls;
    while (pControl && loopCounter--)
        {
        pControl = (CAudioControlBase*)pControl->Blink;
        }

    ASSERT(pControl->get_ControlId() == MXCONTROLID(get_LineId(), i));

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-query_ControlByIndex(pControl=0x%08X)\r\n", pControl)
        );

    return pControl;
}


//------------------------------------------------------------------------------
// query_ControlByControlType
//
CAudioControlBase*
CAudioLineBase::query_ControlByControlType(
    DWORD ControlType
    ) const
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+query_ControlByControlType(ControlType=%d)\r\n", ControlType)
        );

    CAudioControlBase *pControl = (CAudioControlBase*)m_pAudioControls;
    while (pControl != NULL)
        {
        if (pControl->get_ControlType() == ControlType)
            {
            break;
            }
        pControl = (CAudioControlBase*)pControl->Blink;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-query_ControlByControlType(pControl=0x%08X)\r\n", pControl)
        );

    return pControl;
}

//------------------------------------------------------------------------------
// query_ControlByControlId
//
CAudioControlBase*
CAudioLineBase::query_ControlByControlId(
    DWORD ControlId
    ) const
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+query_ControlByControlId(ControlId=%d)\r\n", ControlId)
        );

    CAudioControlBase *pControl = (CAudioControlBase*)m_pAudioControls;
    while (pControl != NULL)
        {
        if (pControl->get_ControlId() == ControlId)
            {
            break;
            }
        pControl = (CAudioControlBase*)pControl->Blink;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-query_ControlByControlId(pControl=0x%08X)\r\n", pControl)
        );

    return pControl;
}

