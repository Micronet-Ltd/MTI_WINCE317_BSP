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
#include "debug.h"
#include "wavemain.h"
#include "audioctrl.h"
#include "audiolin.h"

//------------------------------------------------------------------------------
// CAudioLineBase
//
CAudioControlBase::CAudioControlBase() : m_hReference(NULL)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CAudioControlBase()"));

    m_ControlId = -1;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CAudioControlBase()"));
}


//------------------------------------------------------------------------------
// CAudioLineBase
//
void
CAudioControlBase::copy_ControlInfo(
    MIXERCONTROL *pControlInfo
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+copy_ControlInfo()"));

    pControlInfo->cbStruct = sizeof(MIXERCONTROL);
    pControlInfo->dwControlID = get_ControlId();
    pControlInfo->dwControlType = get_ControlType();
    pControlInfo->cMultipleItems = get_MultipleItemCount();
    pControlInfo->fdwControl = get_StatusFlag();

    wcscpy(pControlInfo->szName, get_Name());
    wcscpy(pControlInfo->szShortName, get_ShortName());

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-copy_ControlInfo()"));
}


//------------------------------------------------------------------------------
//
//  Base class volume audio control
//
DWORD
CAudioControlBase::put_Value(
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlags
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CAudioControlBase::put_Value()"));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+CAudioControlBase::put_Value()\r\n"));

    DWORD mmRet = get_AudioLine()->put_AudioValue(this, pDetail, dwFlags);

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CAudioControlBase::put_Value(mmRet=%0x08)\r\n", mmRet)
        );

	RETAILMSG(WAVE_AUDIO_DBG,
        (L"WAV:-CAudioControlBase::put_Value(mmRet = 0x%X)\r\n", mmRet)
        );

    return mmRet;
}


//------------------------------------------------------------------------------
//
//  Base class volume audio control
//
DWORD
CAudioControlBase::get_Value(
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlags
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CAudioControlBase::get_Value()"));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:+CAudioControlBase::get_Value()\r\n"));

    DWORD mmRet = get_AudioLine()->get_AudioValue(this, pDetail, dwFlags);

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CAudioControlBase::get_Value()"));
	RETAILMSG(WAVE_AUDIO_DBG, (L"WAV:-CAudioControlBase::get_Value() = %d\r\n", mmRet ));

    return mmRet;
}

//------------------------------------------------------------------------------

