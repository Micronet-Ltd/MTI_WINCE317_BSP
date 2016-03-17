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

#include <memtxapi.h>
#include <omap35xx_hwbridge.h>

#include <wave_audio.h>
//------------------------------------------------------------------------------
//
//  defined in bt_ddi.h
//

// BT audio routing define
#if !defined(WODM_BT_SCO_AUDIO_CONTROL)

// Audio driver message for BT audio control
#define WODM_BT_SCO_AUDIO_CONTROL   500

#endif

#if !defined(MM_WOM_FORCEQUALITY)
#define MM_WOM_FORCEQUALITY   (WM_USER+3)
#endif

//------------------------------------------------------------------------------
// CAudioManager constructor
//
CAudioManager::CAudioManager() :
    m_pInputStreamManager(NULL), m_pOutputStreamManager(NULL),
    m_hParentBus(NULL), m_state(kUninitialized), m_pHardwareBridge(NULL)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CAudioManager()\r\n"));
    m_pAudioMixerManager = new CAudioMixerManager(this);

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CAudioManager()\r\n"));
}


//------------------------------------------------------------------------------
// CAudioManager destructor
//
CAudioManager::~CAudioManager()
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+~CAudioManager()\r\n"));

    InternalCleanUp();

    delete m_pAudioMixerManager;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-~CAudioManager()\r\n"));
}


//------------------------------------------------------------------------------
// central routine which ensures all resources are freed for the class
//
void
CAudioManager::InternalCleanUp()
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+InternalCleanUp()\r\n"));

    if (m_state != kUninitialized)
        {
        // clean-up some resources
        //
        if (m_hParentBus)
            {
            SetDevicePowerState(m_hParentBus, D4 , NULL);
            CloseBusAccessHandle(m_hParentBus) ;
            }

        m_state = kUninitialized;
        }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-InternalCleanUp()\r\n"));
}


//------------------------------------------------------------------------------
// Puts CAudioManager in an initialized state
//
BOOL
CAudioManager::initialize(
    LPCWSTR szContext,
    LPCVOID pBusContext
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+initialize(szContext=%d)\r\n", szContext));
	RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:+CAudioManager::initialize(szContext=%d)\r\n", szContext));

    m_hParentBus = CreateBusAccessHandle((LPCTSTR) szContext);
    m_CurPowerState = PwrDeviceUnspecified;
    m_bSuspend = FALSE;
	
    m_DriverIndex = 0;
    m_counterForcedQuality = 0;

    // default input and output profiles are the handset
    //
    m_ffOutputAudioProfile = AUDIO_PROFILE_OUT_HANDSET;
    m_ffInputAudioProfile = AUDIO_PROFILE_IN_HANDSET_MIC;

    m_dwStreamAttenMax = kMaximumAudioAttenuation;
    m_dwDeviceAttenMax = kMaximumAudioAttenuation;
    m_dwClassAttenMax  = kMaximumAudioAttenuation;

    m_state = kInitialized;

    InitializeMixers(this);

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-initialize()\r\n"));
	RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:-CAudioManager::initialize()\r\n"));

    return TRUE;
}

//------------------------------------------------------------------------------
// Puts CAudioManager in an initialized state
//
BOOL
CAudioManager::deinitialize()
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+deinitialize()\r\n"));

    if (m_hParentBus)
        {
        SetDevicePowerState(m_hParentBus, D4 , NULL);
        CloseBusAccessHandle(m_hParentBus);
        m_hParentBus = NULL;
        }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-deinitialize()\r\n"));

    return TRUE;
}


//------------------------------------------------------------------------------
// main IOCTL message handler
//
BOOL
CAudioManager::process_IOControlMsg(
    DWORD dwCode,
    PBYTE pBufIn,
    DWORD dwLenIn,
    PBYTE pBufOut,
    DWORD dwLenOut,
    PDWORD pdwActualOut
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+process_IOControlMsg(dwCode=%d)\r\n", dwCode)
        );

    RETAILMSG(WAVE_AUDIO_MNGR_DBG,
        (L"WAV:+process_IOControlMsg(dwCode=0x%X, pBufIn = 0x%X)\r\n", dwCode, pBufIn)
        );


    BOOL bResult = FALSE;
    IOCTL_WAV_AUDIORENDER_QUERYPORT_OUT *pDataOut = NULL;
    IOCTL_WAV_AUDIORENDER_QUERYPORT_IN *pDataIn = NULL;

    switch (dwCode)
        {
            // audio stream messages
            //
        case IOCTL_WAV_MESSAGE:
             RETAILMSG(WAVE_AUDIO_MNGR_DBG,(L"WAV:process_IOControlMsg - IOCTL_WAV_MESSAGE\r\n"));

            bResult = process_AudioStreamMessage((PMMDRV_MESSAGE_PARAMS)pBufIn,
                            (DWORD*)pBufOut);
            break;

            // audio mixer messages
            //
        case IOCTL_MIX_MESSAGE:
			RETAILMSG(WAVE_AUDIO_MNGR_DBG,(L"WAV:process_IOControlMsg - IOCTL_MIX_MESSAGE\r\n"));

            bResult = get_AudioMixerManager()->process_MixerMessage(
                            (PMMDRV_MESSAGE_PARAMS)pBufIn, (DWORD*)pBufOut);
            break;

            // power management messages
            //
        case IOCTL_POWER_CAPABILITIES:
        case IOCTL_POWER_QUERY:
        case IOCTL_POWER_SET:
        case IOCTL_POWER_GET:
            bResult = process_PowerManagementMessage(dwCode, pBufIn, dwLenIn,
                        pBufOut, dwLenOut, pdwActualOut);
            break;

         case IOCTL_WAV_AUDIORENDER_PORT:
            pDataIn = (IOCTL_WAV_AUDIORENDER_QUERYPORT_IN*)pBufIn;

            // Turn on/off DASF clocks.
            //
            get_HardwareAudioBridge()->enable_I2SClocks(
                                 pDataIn->bPortRequest == OMAP35XX_HwAudioBridge::kAudioDASFPort);

            // Switch to DSP/ARM path audio rendering.
            //
            bResult = get_HardwareAudioBridge()->switch_AudioStreamPort(pDataIn->bPortRequest);
            if (bResult == FALSE)
                {
                DEBUGMSG(ZONE_FUNCTION, (L"Requested audio port is busy now, try later\r\n"));
                bResult = FALSE;
                }
            break;

         case IOCTL_WAV_AUDIORENDER_QUERYPORT:
            pDataOut = (IOCTL_WAV_AUDIORENDER_QUERYPORT_OUT*)pBufOut;
            pDataOut->bActivePort = get_HardwareAudioBridge()->query_AudioStreamPort();

            break;

        default:
            bResult = process_MiscellaneousMessage(dwCode, pBufIn, dwLenIn,
                        pBufOut, dwLenOut, pdwActualOut);
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-process_IOControlMsg(bResult=%d)\r\n", bResult)
        );

    RETAILMSG(WAVE_AUDIO_MNGR_DBG,
        (L"WAV:-process_IOControlMsg(bResult=%d)\r\n", bResult)
        );

    return bResult;
}


//------------------------------------------------------------------------------
// process_PowerManagementMessage, all messages not handled by the other message
//  handlers
//
BOOL
CAudioManager::process_PowerManagementMessage(
    DWORD dwCode,
    PBYTE pBufIn,
    DWORD dwLenIn,
    PBYTE pBufOut,
    DWORD dwLenOut,
    PDWORD pdwActualOut
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+process_PowerManagementMessage(dwCode=%d)\r\n", dwCode)
        );

    BOOL bResult = FALSE;
    POWER_CAPABILITIES powerCaps;
    CEDEVICE_POWER_STATE dxState;
    switch (dwCode)
        {
        // Return device specific power capabilities.
        case IOCTL_POWER_CAPABILITIES:
            DEBUGMSG(ZONE_PM, (L"WAV:IOCTL_POWER_CAPABILITIES\r\n"));

            // Check arguments.
            //
            if ( pBufOut == NULL || dwLenOut < sizeof(POWER_CAPABILITIES))
            {
                RETAILMSG(ZONE_WARN, (L"WAV: Invalid parameter.\r\n"));
                break;
            }

            // Clear capabilities structure.
            //
            memset(&powerCaps, 0, sizeof(POWER_CAPABILITIES));

            // Set power capabilities. Supports D0 and D4.
            //
            powerCaps.DeviceDx = DX_MASK(D0)|DX_MASK(D4);

            DEBUGMSG(ZONE_PM,
                (L"WAV:IOCTL_POWER_CAPABILITIES = 0x%x\r\n", powerCaps.DeviceDx)
                );

            if (CeSafeCopyMemory(pBufOut, &powerCaps,
                sizeof(POWER_CAPABILITIES))
                )
                {
                bResult = TRUE;
                if (pdwActualOut)
                    {
                    *pdwActualOut = sizeof(POWER_CAPABILITIES);
                    }
                }
            break;

        // Indicate if the device is ready to enter a new device power state.
        case IOCTL_POWER_QUERY:
            DEBUGMSG(ZONE_PM,
                (L"WAV:IOCTL_POWER_QUERY = %d\r\n", m_CurPowerState));

            // Check arguments.
            //
            if (pBufOut == NULL || dwLenOut < sizeof(CEDEVICE_POWER_STATE))
                {
                RETAILMSG(ZONE_WARN, (L"WAV: Invalid parameter.\r\n"));
                break;
                }

            if (CeSafeCopyMemory(pBufOut, &m_CurPowerState, sizeof(CEDEVICE_POWER_STATE)) == 0)
                {
                break;
                }

            if (!VALID_DX(m_CurPowerState))
                {
                RETAILMSG(ZONE_ERROR,
                    (L"WAV:IOCTL_POWER_QUERY invalid power state.\r\n"));
                break;
                }

            if (pdwActualOut)
                {
                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                }

            bResult = TRUE;
            break;

        // Request a change from one device power state to another.
        //
        case IOCTL_POWER_SET:

            DEBUGMSG(ZONE_PM, (L"WAV:IOCTL_POWER_QUERY\r\n"));

            // Check arguments.
            if (pBufOut == NULL || dwLenOut < sizeof(CEDEVICE_POWER_STATE))
                {
                RETAILMSG(ZONE_ERROR, (L"WAVE: Invalid parameter.\r\n"));
                break;
                }

            if (CeSafeCopyMemory(&dxState, pBufOut, sizeof(dxState)) == 0)
                {
                break;
                }

            DEBUGMSG(VALID_DX(dxState), (L"WAV:IOCTL_POWER_SET (D%d)\r\n", dxState));
            RETAILMSG(!VALID_DX(dxState),
                (L"WAV:!ERROR - Setting to invalid power state(%d)", dxState)
                );

            if(dxState < D3)
                m_bSuspend = 0;
			else
                m_bSuspend = 1;
            RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:IOCTL_POWER_SET = D%d -> %d.\r\n", dxState, m_bSuspend));

            // Check for any valid power state.
            if (VALID_DX(dxState))
                {
                m_CurPowerState = dxState;
                get_HardwareAudioBridge()->request_PowerState(dxState);
                bResult = TRUE;
                }
            break;

        // Return the current device power state.
        case IOCTL_POWER_GET:
            DEBUGMSG(ZONE_PM, (L"WAV:IOCTL_POWER_GET\r\n"));

            dxState = get_HardwareAudioBridge()->get_CurrentPowerState();

            // Check input parameters
            if (pdwActualOut != NULL)
                {
                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                }

            if (pBufOut == NULL || dwLenOut < sizeof(dxState) ||
                !CeSafeCopyMemory(pBufOut, &dxState, sizeof(dxState)))
                {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    break;
                }

            DEBUGMSG(ZONE_PM,
                (L"WAV:IOCTL_POWER_GET=%d (%d).\r\n",
                dxState, m_CurPowerState));

            bResult = TRUE;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-process_PowerManagementMessage(bResult=%d)\r\n", bResult)
        );
    return bResult;
}

//------------------------------------------------------------------------------
// process_MiscellaneousMessage, all messages not handled by the other message
//  handlers
//
BOOL
CAudioManager::process_MiscellaneousMessage(
    DWORD dwCode,
    PBYTE pBufIn,
    DWORD dwLenIn,
    PBYTE pBufOut,
    DWORD dwLenOut,
    PDWORD pdwActualOut
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+process_MiscellaneousMessage(dwCode=%d)\r\n", dwCode)
        );

	RETAILMSG(WAVE_AUDIO_MNGR_DBG,
        (L"WAV:+process_MiscellaneousMessage(dwCode=%d)\r\n", dwCode)
        );

    switch (dwCode)
        {
        case IOCTL_NOTIFY_BT_HEADSET:
            get_HardwareAudioBridge()->notify_BTHeadsetAttached(*(DWORD *)pBufIn);
            break;

        case IOCTL_NOTIFY_HEADSET:
            get_OutStreamManager()->NotifyHeadsetAttached(*pBufIn);
            break;

        case IOCTL_NOTIFY_AUXHEADSET:
            get_HardwareAudioBridge()->enable_AuxHeadset(*pBufIn);
            break;

        case IOCTL_NOTIFY_HDMI:
            get_HardwareAudioBridge()->notify_HdmiAudioAttached(*pBufIn);
            break;
		case IOCTL_NOTIFY_HF_SPEAKERS:
            get_HardwareAudioBridge()->enable_Speaker(*pBufIn);
            break;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-process_MiscellaneousMessage(bResult=%d)\r\n", 0)
        );

	RETAILMSG(WAVE_AUDIO_MNGR_DBG,
        (L"WAV:-process_MiscellaneousMessage(bResult=%d)\r\n", 0)
        );

    return FALSE;
}


//------------------------------------------------------------------------------
// process_MiscellaneousMessage, all messages not handled by the other message
//  handlers
//
BOOL
CAudioManager::process_AudioStreamMessage(
    PMMDRV_MESSAGE_PARAMS pParams,
    DWORD *pdwResult
    )
{
    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:+process_AudioStreamMessage(uMsg=%d)\r\n", pParams->uMsg)
        );

	RETAILMSG(WAVE_AUDIO_MNGR_DBG,
        (L"WAV:+process_AudioStreamMessage(uMsg=%d)\r\n", pParams->uMsg)
        );

    //  set the error code to be no error first
    SetLastError(MMSYSERR_NOERROR);

    CStreamManager *pStreamManager;
    WaveStreamContext *pWaveStream;


    PDWORD pdwGain;
    UINT uMsg = pParams->uMsg;
    UINT uDeviceId = pParams->uDeviceId;
    DWORD dwParam1 = pParams->dwParam1;
    DWORD dwParam2 = pParams->dwParam2;
    DWORD dwUser   = pParams->dwUser;
    StreamContext *pStreamContext = (StreamContext *)dwUser;
    DWORD dwRet=MMSYSERR_NOTSUPPORTED;

    switch (uMsg)
        {
        case WODM_SETVOLUME:
            DEBUGMSG(ZONE_WODM, (L"WAV:WODM_SETVOLUME\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:WODM_SETVOLUME, pStreamContext = 0x%X, param1 = 0x%X\r\n", pStreamContext, dwParam1));
            dwRet = (pStreamContext) ? pStreamContext->SetGain(dwParam1) :
                            put_OutputGain(dwParam1);
            break;

        case WODM_RESTART:
        case WIDM_START:
            DEBUGMSG(ZONE_WODM|ZONE_WIDM, (L"WAV:WODM_RESTART/WIDM_START\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:WODM_RESTART/WIDM_START\r\n"));
            dwRet = pStreamContext->Run();
            break;

        case WODM_PAUSE:
        case WIDM_STOP:
            DEBUGMSG(ZONE_WODM|ZONE_WIDM, (L"WAV:WODM_PAUSE/WIDM_STOP\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:WODM_PAUSE/WIDM_STOP\r\n"));
            dwRet = pStreamContext->Stop();
            break;

        case WODM_GETPOS:
        case WIDM_GETPOS:
            DEBUGMSG(ZONE_WODM|ZONE_WIDM, (L"WAV:WODM_GETPOS/WIDM_GETPOS\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:WODM_GETPOS/WIDM_GETPOS\r\n"));
            dwRet = pStreamContext->GetPos((PMMTIME)dwParam1);
            break;

        case WODM_RESET:
        case WIDM_RESET:
            DEBUGMSG(ZONE_WODM|ZONE_WIDM, (L"WAV:WODM_RESET/WIDM_RESET\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:WODM_RESET/WIDM_RESET\r\n"));
            dwRet = pStreamContext->Reset();
            break;

        case WODM_WRITE:
        case WIDM_ADDBUFFER:
		{
            DEBUGMSG(ZONE_WODM|ZONE_WIDM,
                (L"WAV:WODM_WRITE/WIDM_ADDBUFFER\r\n")
                );
			RETAILMSG(WAVE_AUDIO_MNGR_DBG,
                (L"WAV:WODM_WRITE/WIDM_ADDBUFFER\r\n")
                );
            if(m_bSuspend)
					pStreamContext->Stop();

            dwRet = pStreamContext->QueueBuffer((LPWAVEHDR)dwParam1);

            break;
		}
        case WODM_GETVOLUME:
            DEBUGMSG(ZONE_WODM, (L"WAV:WODM_GETVOLUME\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:WODM_GETVOLUME\r\n"));
            pdwGain = (PDWORD)dwParam1;
            *pdwGain = (pStreamContext) ? pStreamContext->GetGain() :
                            get_OutputGain();

            dwRet = MMSYSERR_NOERROR;
            break;

        case WODM_BREAKLOOP:
            DEBUGMSG(ZONE_WODM, (L"WAV:WODM_BREAKLOOP\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:WODM_BREAKLOOP\r\n"));
            dwRet = pStreamContext->BreakLoop();
            break;

        case WODM_SETPLAYBACKRATE:
            DEBUGMSG(ZONE_WODM, (L"WAV:WODM_SETPLAYBACKRATE\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:WODM_SETPLAYBACKRATE\r\n"));
            pWaveStream = (WaveStreamContext *)dwUser;
            dwRet = pWaveStream->SetRate(dwParam1);
            break;

        case WODM_GETPLAYBACKRATE:
            DEBUGMSG(ZONE_WODM, (L"WAV:WODM_GETPLAYBACKRATE\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:WODM_GETPLAYBACKRATE\r\n"));
            pWaveStream = (WaveStreamContext *)dwUser;
            dwRet = pWaveStream->GetRate((DWORD *)dwParam1);
            break;

       case MM_WOM_FORCEQUALITY:
            DEBUGMSG(ZONE_WODM, (L"WAV:MM_WOM_FORCEQUALITY\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:MM_WOM_FORCEQUALITY\r\n"));

            if (pStreamContext)
            {
                dwRet = pStreamContext->ForceQuality((BOOL)dwParam1);
            }
            else
            {
                dwRet = get_HardwareAudioBridge()->enable_WideBand(
                    (BOOL)dwParam1
                    );
            }
            break;

        case WODM_BT_SCO_AUDIO_CONTROL:
            DEBUGMSG(ZONE_WODM, (L"WAV:WODM_BT_SCO_AUDIO_CONTROL\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG,  (L"WAV:WODM_BT_SCO_AUDIO_CONTROL\r\n"));
            get_HardwareAudioBridge()->notify_BTHeadsetAttached(
                pParams->dwParam2
                );
            dwRet = MMSYSERR_NOERROR;

			// Test for Mixer notifying
			//m_pAudioMixerManager->notify_AudioMixerCallbacks(13,13);
            break;

        case WODM_CLOSE:
        case WIDM_CLOSE:
            DEBUGMSG(ZONE_WODM|ZONE_WIDM, (L"WAV:WIDM_CLOSE/WODM_CLOSE\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:WIDM_CLOSE/WODM_CLOSE\r\n"));

            // Release stream context here, rather than inside
            // StreamContext::Close, so that if someone
            // has subclassed Close there's no chance that the object
            // will get released out from under them.
            //
            dwRet = pStreamContext->Close();
            if (dwRet == MMSYSERR_NOERROR)
                {
                pStreamContext->Release();
                }
            break;

        case WODM_OPEN:
            DEBUGMSG(ZONE_WODM, (L"WAV:WODM_OPEN\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:WODM_OPEN\r\n"));
            pStreamManager = get_OutputStreamManager(uDeviceId);
            dwRet = pStreamManager->open_Stream((LPWAVEOPENDESC)dwParam1,
                        dwParam2, (StreamContext **)dwUser
                        );
            break;

        case WIDM_OPEN:
            DEBUGMSG(ZONE_WODM, (L"WAV:WIDM_OPEN\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:WIDM_OPEN\r\n"));
            pStreamManager = get_InputStreamManager(uDeviceId);
            dwRet = pStreamManager->open_Stream((LPWAVEOPENDESC)dwParam1,
                        dwParam2, (StreamContext **)dwUser
                        );
            break;

        case WODM_GETDEVCAPS:
            DEBUGMSG(ZONE_WODM, (L"WAV:WODM_GETDEVCAPS\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:WODM_GETDEVCAPS\r\n"));
            pStreamManager = (pStreamContext) ?
                                pStreamContext->GetStreamManager() :
                                get_OutputStreamManager(uDeviceId);

            dwRet = pStreamManager->get_DevCaps((PVOID)dwParam1, dwParam2);
            break;

        case WIDM_GETDEVCAPS:
            DEBUGMSG(ZONE_WIDM, (L"WAV:WIDM_GETDEVCAPS\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:WIDM_GETDEVCAPS\r\n"));
            pStreamManager = (pStreamContext) ?
                                pStreamContext->GetStreamManager() :
                                get_InputStreamManager(uDeviceId);

            dwRet = pStreamManager->get_DevCaps((PVOID)dwParam1, dwParam2);
            break;

        case WODM_GETEXTDEVCAPS:
            DEBUGMSG(ZONE_WODM, (L"WAV:WODM_GETEXTDEVCAPS\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:WODM_GETEXTDEVCAPS\r\n"));

            pStreamManager = (pStreamContext) ?
                                pStreamContext->GetStreamManager() :
                                get_OutputStreamManager(uDeviceId);

            dwRet = pStreamManager->get_ExtDevCaps((PVOID)dwParam1, dwParam2);
            break;

        case WIDM_GETNUMDEVS:
            DEBUGMSG(ZONE_WODM, (L"WAV:WIDM_GETNUMDEVS\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:WIDM_GETNUMDEVS\r\n"));
            dwRet = get_InputDeviceCount();
            break;

        case WODM_GETNUMDEVS:
            DEBUGMSG(ZONE_WIDM, (L"WAV:WODM_GETNUMDEVS\r\n"));
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (L"WAV:WODM_GETNUMDEVS\r\n"));
            dwRet = get_OutputDeviceCount();
            break;

        case WODM_GETPROP:
            {
                // DEBUGMSG(ZONE_WODM, (TEXT("WODM_GETPROP\r\n")));
				RETAILMSG(WAVE_AUDIO_MNGR_DBG, (TEXT("WODM_GETPROP\r\n")));

                PWAVEPROPINFO pPropInfo = (PWAVEPROPINFO) dwParam1;
                if (pStreamContext)
                {
                    dwRet = pStreamContext->GetProperty(pPropInfo);
                }
                else
                {
                    pStreamManager = get_OutputStreamManager(uDeviceId);
                    dwRet = pStreamManager->GetProperty(pPropInfo);
                }
                break;
            }

        case WIDM_GETPROP:
            {
                // DEBUGMSG(ZONE_WODM, (TEXT("WIDM_GETPROP\r\n")));

                PWAVEPROPINFO pPropInfo = (PWAVEPROPINFO) dwParam1;
                if (pStreamContext)
                {
                    dwRet = pStreamContext->GetProperty(pPropInfo);
                }
                else
                {
                    pStreamManager = get_InputStreamManager(uDeviceId);
                    dwRet = pStreamManager->GetProperty(pPropInfo);
                }
                break;
            }

        case WODM_SETPROP:
            {
                // DEBUGMSG(ZONE_WODM, (TEXT("WODM_SETPROP\r\n")));

                PWAVEPROPINFO pPropInfo = (PWAVEPROPINFO) dwParam1;
                if (pStreamContext)
                {
                    dwRet = pStreamContext->SetProperty(pPropInfo);
                }
                else
                {
                    pStreamManager = get_OutputStreamManager(uDeviceId);
                    dwRet = pStreamManager->SetProperty(pPropInfo);
                }
                break;
            }

        case WIDM_SETPROP:
            {
                // DEBUGMSG(ZONE_WODM, (TEXT("WIDM_SETPROP\r\n")));

                PWAVEPROPINFO pPropInfo = (PWAVEPROPINFO) dwParam1;
                if (pStreamContext)
                {
                    dwRet = pStreamContext->SetProperty(pPropInfo);
                }
                else
                {
                    pStreamManager = get_InputStreamManager(uDeviceId);
                    dwRet = pStreamManager->SetProperty(pPropInfo);
                }
                break;
            }
		case WPDM_PRIVATE_UPDATE_REG_VAL:
		{
			RETAILMSG(WAVE_AUDIO_MNGR_DBG, (TEXT("WPDM_PRIVATE_UPDATE_REG_VAL-----------------\r\n")));//temp!!!
			UpdateRegParams();
			dwRet = MMSYSERR_NOERROR;
		}
		break;
        case WODM_GETPITCH:
        case WODM_SETPITCH:
        case WODM_PREPARE:
        case WODM_UNPREPARE:
        case WIDM_PREPARE:
        case WIDM_UNPREPARE:
        default:
            dwRet  = MMSYSERR_NOTSUPPORTED;
        }

    if (pdwResult)
        {
        *pdwResult = dwRet;
        }

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-process_AudioStreamMessage(bResult=%d)\r\n", TRUE)
        );

	RETAILMSG(WAVE_AUDIO_MNGR_DBG,
        (L"WAV:-process_AudioStreamMessage(dwRet=%d)\r\n", dwRet)
        );

    return TRUE;
}


