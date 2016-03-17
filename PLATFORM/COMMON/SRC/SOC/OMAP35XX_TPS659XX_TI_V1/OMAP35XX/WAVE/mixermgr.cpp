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
#include <pm.h>
#include <devload.h>
#include <ceddk.h>
#include <ceddkex.h>
#include "debug.h"
#include "wavemain.h"
#include "audiolin.h"
#include "audioctrl.h"
#include "mixermgr.h"
#include "audiomgr.h"

#if 0
#include <windows.h>
#include <pm.h>
#include <wavedev.h>
#include <mmddk.h>
#include <devload.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <linklist.h>
#include "debug.h"
#include "wavemain.h"
#include "audioctrl.h"
#include "audiolin.h"
#include "mixermgr.h"
#include "audiomgr.h"
#include "strmctxt.h"
#endif

#include "strmctxt.h"
#include <memtxapi.h>
#include <omap35xx_hwbridge.h>
#include <wave_audio.h>

//------------------------------------------------------------------------------
// CAudioMixerManager constructor
//
CAudioMixerManager::CAudioMixerManager(
    CAudioManager* pAudioManager
    ) : m_pAudioMixerCallbacks(NULL), m_pAudioManager(pAudioManager),
    m_pSourceAudioLines(NULL), m_pDestinationAudioLines(NULL)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CAudioMixerManager()\r\n"));

    m_counterDestinationLines = 0;

    memset(m_prgAudioMixerControls, 0,
        sizeof(CAudioControlBase*)*kMixerControlCount
        );

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CAudioMixerManager()\r\n"));
}


//------------------------------------------------------------------------------
// put_AudioMixerControl
//
void CAudioMixerManager::put_AudioMixerControl(
    AudioMixerControl id,
    CAudioControlBase* pCtrl)
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+put_AudioMixerControl(id=%d, ctrl=0x%08X)\r\n", id, pCtrl));

    ASSERT(id < kMixerControlCount);
    m_prgAudioMixerControls[id] = pCtrl;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-put_AudioMixerControl()\r\n"));
}


//------------------------------------------------------------------------------
// register_DestinationAudioLine, adds the output mixers to the list
//
//  NOTE:
//      There may be a many to many relationship between inputs and outputs
//  For these situations it's recommended to first register all output
//  AudioLines first and then register input AudioLines with the associated
//  output AudioLines.  A AudioLine should not be registered more than once.
//
BOOL
CAudioMixerManager::register_DestinationAudioLine(
    CAudioLineBase* pDestination
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+register_DestinationAudioLine()\r\n"));
	RETAILMSG(WAVE_MIXER_MNGR_DBG, (L"WAV:+register_DestinationAudioLine(), pDestination = 0x%X\r\n", pDestination));

    // generate a unique destination id and use that as the basis
    // for the line id
    //
    pDestination->put_AudioLineIds(
        MXLINEID(m_counterDestinationLines, kNoSource),
        m_counterDestinationLines,
        0);

    ++m_counterDestinationLines;
    InsertTail(m_pDestinationAudioLines, pDestination);

    pDestination->initialize_AudioLine(this);

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-register_DestinationAudioLine()\r\n"));
	RETAILMSG(WAVE_MIXER_MNGR_DBG, (L"WAV:-register_DestinationAudioLine()\r\n"));

    return TRUE;
}


//------------------------------------------------------------------------------
// register_SourceAudioLine generates unique id's for the AudioLine and adds it
// to the input list
//
BOOL
CAudioMixerManager::register_SourceAudioLine(
    CAudioLineBase* pSource,
    CAudioLineBase* pDestination
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+register_SourceAudioLine()\r\n"));
	RETAILMSG(WAVE_MIXER_MNGR_DBG, (L"WAV:+register_SourceAudioLine()\r\n"));

    // the id's for a source AudioLine is based on the id of the destination
    // AudioLine concatonated with the current number of connections to
    // the destination AudioLine
    //
    WORD SourceId = pDestination->get_ConnectionCount();

    pSource->put_AudioLineIds(
        MXLINEID(pDestination->get_DestinationId(), SourceId),
        pDestination->get_DestinationId(),
        SourceId);

    pDestination->increment_ConnectionCount();

    InsertTail(m_pSourceAudioLines, pSource);

    pSource->initialize_AudioLine(this);

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-register_SourceAudioLine()\r\n"));
	RETAILMSG(WAVE_MIXER_MNGR_DBG, (L"WAV:-register_SourceAudioLine()\r\n"));

    return TRUE;
}


//------------------------------------------------------------------------------
// query_ControlByControlId retrieves the audio control based on the control id
//
CAudioControlBase*
CAudioMixerManager::query_ControlByControlId(
    DWORD ControlId
    ) const
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+query_ControlByControlId(ControlId=0x%08X)\r\n", ControlId)
        );

    // control id is composed of LineId and the ordinal position
    // in the Audio Line
    //
    CAudioControlBase *pControl = NULL;
    WORD LineId = MXCONTROLLINEID(ControlId);
    CAudioLineBase *pLine = query_AudioLineByLineId(LineId);
    if (pLine)
        {
        pControl = pLine->query_ControlByIndex(MXCONTROLINDEX(ControlId));
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-query_ControlByControlId(pContro=0x%08X)\r\n", pControl)
        );

    return pControl;
}


//------------------------------------------------------------------------------
// query_AudioLineByLineId, retrieves the AudioLine which matches the line id
//
CAudioLineBase*
CAudioMixerManager::query_AudioLineByLineId(
    WORD LineId
    ) const
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+query_AudioLineByLineId(LineId=%d)\r\n", LineId)
        );

    // check if the AudioLine is a source or destination AudioLine
    //
    CAudioLineBase* pAudioLine = NULL;
    if (MXSOURCEID(LineId) == kNoSource)
        {
        // destination AudioLine
        //
        pAudioLine = query_AudioLineByDestinationId(MXDESTINATIONID(LineId));
        }
    else
        {
        // source/destination AudioLine
        //
        pAudioLine = query_AudioLineBySourceAndDestinationId(
                        MXSOURCEID(LineId),
                        MXDESTINATIONID(LineId)
                        );
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-query_AudioLineByLineId(pAudioLine=0x%08X)\r\n", pAudioLine)
        );

    return pAudioLine;
}


//------------------------------------------------------------------------------
// query_AudioLineBySourceAndDestinationId, retrieves the AudioLine which
// matches the line id
//
CAudioLineBase*
CAudioMixerManager::query_AudioLineBySourceAndDestinationId(
    WORD SourceId,
    WORD DestinationId
    ) const
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+query_AudioLineBySourceAndDestinationId"
         L"(SourceId=%d, DestinationId=%d)\r\n",
         SourceId, DestinationId)
         );

    // the simplist and quickest way to find a AudioLine with matching
    // sourceid and destination id is to search based on its line
    // id since the line id is the concatonation of the two
    //
    WORD LineId = MXLINEID(DestinationId, SourceId);
    CAudioLineBase* pAudioLine = NULL;
    if (DestinationId < m_counterDestinationLines)
        {
        pAudioLine = (CAudioLineBase*)m_pSourceAudioLines;
        while (pAudioLine)
            {
            if (pAudioLine->get_LineId() == LineId)
                {
                break;
                }
            pAudioLine = (CAudioLineBase*)pAudioLine->Blink;
            }
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-query_AudioLineByLineId(pAudioLine=0x%08X)\r\n", pAudioLine)
        );

    return pAudioLine;
}


//------------------------------------------------------------------------------
// query_AudioLineByDestinationId, retrieves the AudioLine which matches the
// destination id
//
CAudioLineBase*
CAudioMixerManager::query_AudioLineByDestinationId(
    WORD DestinationId
    ) const
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+query_AudioLineByDestinationId(idDestination=%d)\r\n",
        DestinationId)
        );

    // the idDestination is really the ordinal position
    // of the AudioLine in the destination AudioLine array
    //
    CAudioLineBase* pAudioLine = NULL;
    if (DestinationId < m_counterDestinationLines)
        {
        int i = DestinationId;
        pAudioLine = (CAudioLineBase*)m_pDestinationAudioLines;
        while (pAudioLine && i--)
            {
            pAudioLine = (CAudioLineBase*)pAudioLine->Blink;
            }

        ASSERT(pAudioLine->get_DestinationId() == DestinationId);
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-query_AudioLineByDestinationId(pAudioLine=0x%08X)\r\n",
        pAudioLine)
        );

    return pAudioLine;
}


//------------------------------------------------------------------------------
// query_AudioLineByComponentType, retrieves the AudioLine which matches the
// destination id
//
CAudioLineBase*
CAudioMixerManager::query_AudioLineByComponentType(
    DWORD ComponentType
    ) const
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+query_AudioLineByComponentType(ComponentType=%d)\r\n",
        ComponentType)
        );

    // check if the requested component type is in regards to
    // destination or source and then search respective list
    //
    CAudioLineBase* pAudioLine = NULL;
    if (MIXERLINE_COMPONENTTYPE_DST_FIRST <= ComponentType &&
        MIXERLINE_COMPONENTTYPE_DST_LAST >= ComponentType)
        {
        pAudioLine = (CAudioLineBase*)m_pDestinationAudioLines;
        }
    else if (MIXERLINE_COMPONENTTYPE_SRC_FIRST <= ComponentType &&
             MIXERLINE_COMPONENTTYPE_SRC_LAST >= ComponentType)
        {
        pAudioLine = (CAudioLineBase*)m_pSourceAudioLines;
        }

    // search the list
    //
    while (pAudioLine)
        {
        if (pAudioLine->get_ComponentType() == ComponentType)
            {
            break;
            }
        pAudioLine = (CAudioLineBase*)pAudioLine->Blink;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-query_AudioLineByComponentType(pAudioLine=0x%08X)\r\n",
         pAudioLine)
        );

    return pAudioLine;
}


//------------------------------------------------------------------------------
// query_AudioLineByTargetType, retrieves the AudioLine by target type
//
CAudioLineBase*
CAudioMixerManager::query_AudioLineByTargetType(
    DWORD TargetType
    ) const
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+query_AudioLineByTargetType(TargetType=%d)\r\n", TargetType)
        );

    // for right now we only check the destination list for matching
    // target types
    //
    CAudioLineBase *pAudioLine = (CAudioLineBase*)m_pDestinationAudioLines;
    while (pAudioLine)
        {
        if (pAudioLine->get_TargetType() == TargetType)
            {
            break;
            }
        pAudioLine = (CAudioLineBase*)pAudioLine->Blink;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-query_AudioLineByTargetType(pAudioLine=0x%08X)\r\n",
        pAudioLine));

    return pAudioLine;
}


//------------------------------------------------------------------------------
// put_ControlDetails
//
DWORD
CAudioMixerManager::put_ControlDetails(
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlags
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+put_ControlDetails(%d)\r\n",
        pDetail->dwControlID));

    // NOTE:
    //   API guarantees that pDetail points to accessible, aligned, properly
    // sized MIXERCONTROLDETAILS structure
    //
    DWORD mmRet = MIXERR_INVALCONTROL;
    CAudioControlBase *pControl = query_ControlByControlId(
                                      pDetail->dwControlID
                                      );
    if (pControl)
    {
        mmRet = pControl->put_Value(pDetail, dwFlags);
        notify_AudioMixerCallbacks(MM_MIXM_CONTROL_CHANGE,
            pControl->get_ControlId());
    }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-put_ControlDetails(mmRet=0x%08X)\r\n",
        mmRet));

    return mmRet;
}


//------------------------------------------------------------------------------
// get_ControlDetails
//
DWORD
CAudioMixerManager::get_ControlDetails(
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlags
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+get_ControlDetails(%d)\r\n",
        pDetail->dwControlID));

	RETAILMSG(WAVE_MIXER_MNGR_DBG, (L"WAV:+%S\r\n",__FUNCTION__));
    // NOTE:
    //   API guarantees that pDetail points to accessible, aligned, properly
    // sized MIXERCONTROLDETAILS structure
    //
    DWORD mmRet = MIXERR_INVALCONTROL;
    CAudioControlBase *pControl = query_ControlByControlId(
                                    pDetail->dwControlID
                                    );
    if (pControl)
    {
        mmRet = pControl->get_Value(pDetail, dwFlags);
    }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-get_ControlDetails(mmRet=0x%08X)\r\n",
        mmRet));
	RETAILMSG(WAVE_MIXER_MNGR_DBG, (L"WAV:-%S, mmRet=0x%08X\r\n",__FUNCTION__, mmRet));

    return mmRet;
}


//------------------------------------------------------------------------------
// get_LineControls
//
DWORD
CAudioMixerManager::get_LineControls(
    PMIXERLINECONTROLS pDetail,
    DWORD dwFlags
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+get_LineControls()\r\n"));

    WORD ControlCount;
    PMIXERCONTROL pamxctrl;
    CAudioLineBase *pAudioLine = NULL;
    CAudioControlBase *pControl = NULL;
    MMRESULT mmRet = MIXERR_INVALLINE;

    if (pDetail->cControls < 1)
        {
        RETAILMSG(ZONE_ERROR, (L"GetMixerLineControls: control count must "
            L"be non zero\r\n"));
        goto cleanUp;
        }

    switch (dwFlags & MIXER_GETLINECONTROLSF_QUERYMASK)
        {
        case MIXER_GETLINECONTROLSF_ALL:
            DEBUGMSG(ZONE_MIXER, (L"WAV:MIXER_GETLINECONTROLSF_ALL\r\n"));

            // get matching audio line id
            //
            pAudioLine = query_AudioLineByLineId((WORD)pDetail->dwLineID);
            if (pAudioLine == NULL)
                {
                RETAILMSG(ZONE_ERROR, (L"WAV:get_LineControls: "
                    L"invalid line ID 0x%08X\r\n", pDetail->dwLineID)
                    );
                mmRet = MIXERR_INVALLINE;
                break;
                }

            // make sure control count matches
            //
            ControlCount = pAudioLine->get_ControlCount();
            if (ControlCount != pDetail->cControls)
                {
                RETAILMSG(ZONE_ERROR, (L"WAV:get_LineControls: "
                    L"invalid control count, expect=%d, found%d\r\n",
                    pDetail->cControls, ControlCount)
                    );
                mmRet = MMSYSERR_INVALPARAM;
                break;
                }

            // copy all control information
            //
            pamxctrl = pDetail->pamxctrl;
            for (WORD i = 0; i < ControlCount; ++i)
                {
                // UNDONE:
                //  Need to optimize
                //
                pAudioLine->query_ControlByIndex(i)->copy_ControlInfo(pamxctrl);
                ++pamxctrl;
                }

            mmRet = MMSYSERR_NOERROR;
            break;

        case MIXER_GETLINECONTROLSF_ONEBYID:
            DEBUGMSG(ZONE_MIXER, (L"WAV:MIXER_GETLINECONTROLSF_ONEBYID\r\n"));

            // go through the list of all registered controls and
            // get the one which matches the control Id
            //
            pControl = query_ControlByControlId(pDetail->dwControlID);

            if (pControl == NULL)
                {
                RETAILMSG(ZONE_ERROR, (L"WAV:get_LineControls: "
                    L"invalid control id 0x%08X\r\n", pDetail->dwControlID)
                    );
                mmRet = MMSYSERR_INVALPARAM;
                break;
                }

            // copy control information
            //
            pControl->copy_ControlInfo(pDetail->pamxctrl);
            pDetail->dwLineID = MXCONTROLLINEID(pControl->get_ControlId());
            mmRet = MMSYSERR_NOERROR;
            break;

        case MIXER_GETLINECONTROLSF_ONEBYTYPE:
            DEBUGMSG(ZONE_MIXER, (L"WAV:MIXER_GETLINECONTROLSF_ONEBYTYPE\r\n"));

            // get matching audio line id
            //
            pAudioLine = query_AudioLineByLineId((WORD)pDetail->dwLineID);
            if (pAudioLine == NULL)
                {
                RETAILMSG(ZONE_ERROR, (L"WAV:get_LineControls: "
                    L"invalid line ID 0x%08X\r\n", pDetail->dwLineID)
                    );
                mmRet = MIXERR_INVALLINE;
                break;
                }

            // get control first control which matches the control type
            //
            pControl = pAudioLine->query_ControlByControlType(
                            pDetail->dwControlType
                            );

            if (pControl == NULL)
                {
                RETAILMSG(ZONE_ERROR, (L"WAV:get_LineControls: "
                    L"unableto find control with matching type, "
                    L"LineId=0x%08X, ControlType=0x%08X\r\n",
                    pDetail->dwLineID, pDetail->dwControlType)
                    );
                mmRet = MMSYSERR_INVALPARAM;
                break;
                }

            pControl->copy_ControlInfo(pDetail->pamxctrl);
            mmRet = MMSYSERR_NOERROR;
        }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-get_LineControls(mmRet=%d)\r\n", mmRet));

cleanUp:
    return mmRet;
}


//------------------------------------------------------------------------------
// get_LineInfo
//
DWORD
CAudioMixerManager::get_LineInfo(
    PMIXERLINE pDetail,
    DWORD dwFlags
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+get_LineInfo()\r\n"));

    // NOTE:
    //   pDetail is validated by API - points to accessible,
    // properly sized MIXERLINE structure

    CAudioLineBase const* pAudioLine = NULL;
    MMRESULT mmRet = MIXERR_INVALLINE;

    switch (dwFlags & MIXER_GETLINEINFOF_QUERYMASK)
        {
        case MIXER_GETLINEINFOF_DESTINATION:
            DEBUGMSG(ZONE_MIXER, (L"WAV:MIXER_GETLINEINFOF_DESTINATION\r\n"));
            pAudioLine = query_AudioLineByDestinationId(
                             (WORD)pDetail->dwDestination
                             );
            break;

        case MIXER_GETLINEINFOF_LINEID:
            DEBUGMSG(ZONE_MIXER, (L"WAV:MIXER_GETLINEINFOF_LINEID\r\n"));
            pAudioLine = query_AudioLineByLineId((WORD)pDetail->dwLineID);
            break;

        case MIXER_GETLINEINFOF_SOURCE:
            DEBUGMSG(ZONE_MIXER, (L"WAV:MIXER_GETLINEINFOF_SOURCE\r\n"));
            pAudioLine = query_AudioLineBySourceAndDestinationId(
                            (WORD)pDetail->dwSource,
                            (WORD)pDetail->dwDestination
                            );
            break;

        case MIXER_GETLINEINFOF_COMPONENTTYPE:
            DEBUGMSG(ZONE_MIXER, (L"WAV:MXDM_SETCONTROLDETAILS\r\n"));
            pAudioLine = query_AudioLineByComponentType(
                            pDetail->dwComponentType
                            );
            break;

        case MIXER_GETLINEINFOF_TARGETTYPE:
            DEBUGMSG(ZONE_MIXER, (L"WAV:MXDM_SETCONTROLDETAILS\r\n"));
            pAudioLine = query_AudioLineByTargetType(pDetail->Target.dwType);
            break;

        default:
            DEBUGMSG(ZONE_MIXER, (L"WAV:get_LineInfo\r\n"));
            mmRet = MMSYSERR_INVALPARAM;
        }

    if (pAudioLine)
        {
        pAudioLine->copy_LineInfo(pDetail);
        mmRet = MMSYSERR_NOERROR;
        }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-get_LineInfo(mmRet=%d)\r\n", mmRet));

    return mmRet;
}


//------------------------------------------------------------------------------
// close_AudioLineDevice
//
DWORD
CAudioMixerManager::close_Device(
    AudioMixerCallback_t *pAudioCB
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+close_Device()\r\n"));

    AudioMixerCallback_t *pCurr;
    AudioMixerCallback_t *pPrev = NULL;
    for (pCurr = m_pAudioMixerCallbacks; pCurr != NULL; pCurr = pCurr->pNext)
        {
        if (pCurr == pAudioCB)
            {
            if (pPrev == NULL)
                {
                // we're deleting the first item
                m_pAudioMixerCallbacks = pCurr->pNext;
                }
            else
                {
                pPrev->pNext = pCurr->pNext;
                }
            LocalFree(pCurr);
            break;
            }
        pPrev = pCurr;
        }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-close_Device()\r\n"));

    return MMSYSERR_NOERROR;
}


//------------------------------------------------------------------------------
// close_AudioLineDevice
//
DWORD
CAudioMixerManager::open_Device(
    PDWORD phDeviceId,
    PMIXEROPENDESC pMOD,
    DWORD dwFlags
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+open_Device()\r\n"));

    AudioMixerCallback_t *pAudioCB;
    pAudioCB = (AudioMixerCallback_t*)LocalAlloc(LMEM_FIXED,
                    sizeof(AudioMixerCallback_t)
                    );

    if (pAudioCB == NULL)
        {
        DEBUGMSG(ZONE_ERROR,
            (L"WAV:!ERROR-open_Device: out of memory\r\n")
            );

        return MMSYSERR_NOMEM;
        }

    pAudioCB->hmx = (DWORD)pMOD->hmx;
    pAudioCB->pfnCallback = (dwFlags & CALLBACK_FUNCTION) ?
                                (fnAudioMixerCallback*)pMOD->dwCallback :
                                NULL;

    pAudioCB->pNext = m_pAudioMixerCallbacks;
    m_pAudioMixerCallbacks = pAudioCB;
    *phDeviceId = (DWORD)pAudioCB;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-open_Device()\r\n"));

    return MMSYSERR_NOERROR;
}


//------------------------------------------------------------------------------
// get_DeviceCaps
//
DWORD
CAudioMixerManager::get_DeviceCaps(
    PMIXERCAPS pCaps,
    DWORD dwSize
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+get_DeviceCaps()\r\n"));

    pCaps->wMid = MM_MICROSOFT;
    pCaps->wPid = MM_MSFT_WSS_MIXER;
    wcscpy(pCaps->szPname, get_AudioMixerName());
    pCaps->vDriverVersion = get_AudioMixerVersion();
    pCaps->cDestinations = m_counterDestinationLines;
    pCaps->fdwSupport = 0;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-get_DeviceCaps()\r\n"));

    return MMSYSERR_NOERROR;
}


//------------------------------------------------------------------------------
// notify_AudioMixerCallbacks notify all listeners
//
void
CAudioMixerManager::notify_AudioMixerCallbacks(
    DWORD msg,
    DWORD ControlId
    ) const
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+notify_AudioMixerCallbacks(msg=%d, ControlId=0x%08X)\r\n",
        msg, ControlId));

	RETAILMSG(WAVE_MIXER_MNGR_DBG,
        (L"WAV:+notify_AudioMixerCallbacks(msg=%d, ControlId=0x%X)\r\n",
        msg, ControlId));

    AudioMixerCallback_t *pAudioCB = m_pAudioMixerCallbacks;
    while (pAudioCB)
        {
        if (pAudioCB->pfnCallback)
            {
			RETAILMSG(WAVE_MIXER_MNGR_DBG, (L"WAV:notify_AudioMixerCallbacks() calling!\r\n"));
            pAudioCB->pfnCallback(pAudioCB->hmx, msg, 0, ControlId, 0);
            }
        pAudioCB = pAudioCB->pNext;
        }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-notify_AudioMixerCallbacks()\r\n"));
	RETAILMSG(WAVE_MIXER_MNGR_DBG, (L"WAV:-notify_AudioMixerCallbacks()\r\n"));

}


//------------------------------------------------------------------------------
// process_MixerMessage handles audio mixer messages
//
BOOL
CAudioMixerManager::process_MixerMessage(
    PMMDRV_MESSAGE_PARAMS pParams,
    DWORD* pdwResult
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+process_MixerMessage()\r\n"));
	//RETAILMSG(WAVE_MIXER_MNGR_DBG, (L"gpTimerInit: s_pGPT3 addr = 0x%X\r\n", s_pGPT3) );
	RETAILMSG(WAVE_MIXER_MNGR_DBG, (L"WAV:+process_MixerMessage() # %d\r\n", pParams->uMsg));

    DWORD result = FALSE;

    switch (pParams->uMsg)
    {
    case MXDM_SETCONTROLDETAILS:
        DEBUGMSG(ZONE_MIXER, (L"WAV:MXDM_SETCONTROLDETAILS\r\n"));
        result = put_ControlDetails((PMIXERCONTROLDETAILS) pParams->dwParam1,
                    pParams->dwParam2
                    );
        break;

    case MXDM_GETLINEINFO:
        DEBUGMSG(ZONE_MIXER, (L"WAV:MXDM_GETLINEINFO\r\n"));
        result = get_LineInfo((PMIXERLINE) pParams->dwParam1,
                    pParams->dwParam2
                    );
        break;

    case MXDM_GETLINECONTROLS:
        DEBUGMSG(ZONE_MIXER, (L"WAV:MXDM_GETLINECONTROLS\r\n"));
        result = get_LineControls((PMIXERLINECONTROLS) pParams->dwParam1,
                    pParams->dwParam2
                    );
        break;

    case MXDM_GETCONTROLDETAILS:
        DEBUGMSG(ZONE_MIXER, (L"WAV:MXDM_GETCONTROLDETAILS\r\n"));
        result = get_ControlDetails((PMIXERCONTROLDETAILS) pParams->dwParam1,
                    pParams->dwParam2
                    );
        break;

    case MXDM_GETDEVCAPS:
        DEBUGMSG(ZONE_MIXER, (L"WAV:MXDM_GETDEVCAPS\r\n"));
        result = get_DeviceCaps((PMIXERCAPS) pParams->dwParam1,
                    pParams->dwParam2
                    );
        break;

    case MXDM_OPEN:
        DEBUGMSG(ZONE_MIXER, (L"WAV:MXDM_OPEN\r\n"));
        result = open_Device((PDWORD) pParams->dwUser,
                    (PMIXEROPENDESC) pParams->dwParam1,
                    pParams->dwParam2
                    );
        break;

    case MXDM_CLOSE:
        DEBUGMSG(ZONE_MIXER, (L"WAV:MXDM_CLOSE\r\n"));
        result = close_Device((AudioMixerCallback_t*)pParams->dwUser);
        break;

    case MXDM_GETNUMDEVS:
        DEBUGMSG(ZONE_MIXER, (L"WAV:MXDM_GETNUMDEVS\r\n"));
        result = 1;
        break;

    // #####, Michael
	case MXDM_USER:
		RETAILMSG(WAVE_MIXER_MNGR_DBG, (L"MXDM_USER: Set Modem Voice Call mode to %d\r\n", (BOOL) pParams->dwParam1 ));
		get_AudioManager()->get_HardwareAudioBridge()->enable_ModemVoiceCall((BOOL) pParams->dwParam1);
		result = 1;
		break;

    default:
        DEBUGMSG(ZONE_WARN, (L"MXDM: Unsupported mixer message\r\n"));
		RETAILMSG(WAVE_MIXER_MNGR_DBG, (L"MXDM: Unsupported mixer message\r\n"));
        result = MMSYSERR_NOTSUPPORTED;
        break;
    }

    *pdwResult = result;
    //return TRUE;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-process_MixerMessage()\r\n"));
	RETAILMSG(WAVE_MIXER_MNGR_DBG, (L"WAV:-process_MixerMessage() # %d\r\n", pParams->uMsg));

	return TRUE;

    //return !!result;
}

