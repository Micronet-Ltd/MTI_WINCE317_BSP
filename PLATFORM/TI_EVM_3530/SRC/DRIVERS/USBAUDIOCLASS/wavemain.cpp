//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft
// premium shared source license agreement under which you licensed
// this source code. If you did not accept the terms of the license
// agreement, you are not authorized to use this source code.
// For the terms of the license, please see the license agreement
// signed by you and Microsoft.
// THE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//
// -----------------------------------------------------------------------------
#include "wavemain.h"

//#ifndef SHIP_BUILD
//DBGPARAM dpCurSettings = {
//    TEXT("WaveDriver"), {
//         TEXT("Test")           //  0
//        ,TEXT("Params")         //  1
//        ,TEXT("Verbose")        //  2
//        ,TEXT("Interrupt")      //  3
//        ,TEXT("WODM")           //  4
//        ,TEXT("WIDM")           //  5
//        ,TEXT("PDD")            //  6
//        ,TEXT("MDD")            //  7
//        ,TEXT("Regs")           //  8
//        ,TEXT("Misc")           //  9
//        ,TEXT("Init")           // 10
//        ,TEXT("IOcontrol")      // 11
//        ,TEXT("Alloc")          // 12
//        ,TEXT("Function")       // 13
//        ,TEXT("Warning")        // 14
//        ,TEXT("Error")          // 15
//    }
//    ,
//        (1 << 15)   // Errors
//    |   (1 << 14)   // Warnings
//    |   0xffff
//};
//#endif

BOOL CALLBACK DllMain(HANDLE hDLL,
                      DWORD dwReason,
                      LPVOID lpvReserved)
{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH :
            RETAILREGISTERZONES ((HINSTANCE)hDLL);
            DisableThreadLibraryCalls((HMODULE) hDLL);
            break;

        case DLL_PROCESS_DETACH :
            break;

        case DLL_THREAD_DETACH :
            break;

        case DLL_THREAD_ATTACH :
            break;

        default :
            break;
    }
    return TRUE;
}


// -----------------------------------------------------------------------------
//
// @doc     WDEV_EXT
//
// @topic   WAV Device Interface | Implements the WAVEDEV.DLL device
//          interface. These functions are required for the device to
//          be loaded by DEVICE.EXE.
//
// @xref                          <nl>
//          <f WAV_Init>,         <nl>
//          <f WAV_Deinit>,       <nl>
//          <f WAV_Open>,         <nl>
//          <f WAV_Close>,        <nl>
//          <f WAV_IOControl>     <nl>
//
// -----------------------------------------------------------------------------
//
// @doc     WDEV_EXT
//
// @topic   Designing a Waveform Audio Driver |
//          A waveform audio driver is responsible for processing messages
//          from the Wave API Manager (WAVEAPI.DLL) to playback and record
//          waveform audio. Waveform audio drivers are implemented as
//          dynamic link libraries that are loaded by DEVICE.EXE The
//          default waveform audio driver is named WAVEDEV.DLL (see figure).
//          The messages passed to the audio driver are similar to those
//          passed to a user-mode Windows NT audio driver (such as mmdrv.dll).
//
//          <bmp blk1_bmp>
//
//          Like all device drivers loaded by DEVICE.EXE, the waveform
//          audio driver must export the standard device functions,
//          XXX_Init, XXX_Deinit, XXX_IoControl, etc (see
//          <t WAV Device Interface>). The Waveform Audio Drivers
//          have a device prefix of "WAV".
//
//          Driver loading and unloading is handled by DEVICE.EXE and
//          WAVEAPI.DLL. Calls are made to <f WAV_Init> and <f WAV_Deinit>.
//          When the driver is opened by WAVEAPI.DLL calls are made to
//          <f WAV_Open> and <f WAV_Close>.  All
//          other communication between WAVEAPI.DLL and WAVEDEV.DLL is
//          done by calls to <f WAV_IOControl>. The other WAV_xxx functions
//          are not used.
//
// @xref                                          <nl>
//          <t Designing a Waveform Audio PDD>    <nl>
//          <t WAV Device Interface>              <nl>
//          <t Wave Input Driver Messages>        <nl>
//          <t Wave Output Driver Messages>       <nl>
//
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   PVOID | WAV_Init | Device initialization routine
//
//  @parm   DWORD | dwInfo | info passed to RegisterDevice
//
//  @rdesc  Returns a DWORD which will be passed to Open & Deinit or NULL if
//          unable to initialize the device.
//
// -----------------------------------------------------------------------------
extern "C" DWORD WAV_Init(DWORD Index, LPCVOID lpvBusContext)
{
    DEBUGMSG(ZONE_INIT, (TEXT(">WAV_Init(%p, %p)\n"), Index, lpvBusContext));
    if (g_pHWContext || HardwareContext::CreateHWContext(Index) == FALSE)   
		g_pHWContext = NULL;

	RETAILMSG(0, (L"USBAUDIO:%S, g_pHWContext = %p\r\n", __FUNCTION__, g_pHWContext));
    return((DWORD) g_pHWContext);
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   PVOID | WAV_Deinit | Device deinitialization routine
//
//  @parm   DWORD | dwData | value returned from WAV_Init call
//
//  @rdesc  Returns TRUE for success, FALSE for failure.
//
// -----------------------------------------------------------------------------
extern "C" BOOL WAV_Deinit(DWORD dwData)
{
	RETAILMSG(0, (L"USBAUDIO:%S, pdwData = %p\r\n", __FUNCTION__, dwData));
    HardwareContext* pHWContext = (HardwareContext*) dwData;
    pHWContext->Deinit();
    delete pHWContext;
    return TRUE;
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   PVOID | WAV_Open    | Device open routine
//
//  @parm   DWORD | dwData      | Value returned from WAV_Init call (ignored)
//
//  @parm   DWORD | dwAccess    | Requested access (combination of GENERIC_READ
//                                and GENERIC_WRITE) (ignored)
//
//  @parm   DWORD | dwShareMode | Requested share mode (combination of
//                                FILE_SHARE_READ and FILE_SHARE_WRITE) (ignored)
//
//  @rdesc  Returns a DWORD which will be passed to Read, Write, etc or NULL if
//          unable to open device.
//
// -----------------------------------------------------------------------------
extern "C" PDWORD WAV_Open( DWORD dwData,
              DWORD dwAccess,
              DWORD dwShareMode)
{
	RETAILMSG(0, (L"USBAUDIO:%S, pdwData = %p\r\n", __FUNCTION__, dwData));
    return (PDWORD)dwData; // dwData is pointer to HardwareContext
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   BOOL | WAV_Close | Device close routine
//
//  @parm   DWORD | dwOpenData | Value returned from WAV_Open call
//
//  @rdesc  Returns TRUE for success, FALSE for failure
//
// -----------------------------------------------------------------------------
extern "C" BOOL WAV_Close(PDWORD pdwData)
{
	RETAILMSG(0, (L"USBAUDIO:%S, pdwData = %p\r\n", __FUNCTION__, pdwData));
    return TRUE;
}

BOOL HandleWaveMessage(HardwareContext* pHWContext, PMMDRV_MESSAGE_PARAMS pParams, DWORD *pdwResult)
{
    DWORD dwErrCode = ERROR_SUCCESS;

    UINT uMsg = pParams->uMsg;
    UINT uDeviceId = pParams->uDeviceId;
    DWORD dwParam1 = pParams->dwParam1;
    DWORD dwParam2 = pParams->dwParam2;
    DWORD dwUser   = pParams->dwUser;
    StreamContext *pStreamContext = (StreamContext *)dwUser;

    DWORD dwRet;

    pHWContext->Lock();

	RETAILMSG(0, (L"USBAUDIO:%S, uMsg = %d, uDeviceId = %d\r\n", __FUNCTION__, uMsg, uDeviceId));
    // catch exceptions inside device lock, otherwise device will remain locked!
    _try
    {
        switch (uMsg)
        {
        case WODM_GETNUMDEVS:
            {
                dwRet = pHWContext->GetNumOutputDevices();
				RETAILMSG(0, (L"USBAUDIO:%S, WODM_GETNUMDEVS, dwRet = %d\r\n", __FUNCTION__, dwRet));
                break;
            }

        case WIDM_GETNUMDEVS:
            {
                dwRet = pHWContext->GetNumInputDevices();
                break;
            }

        case WODM_GETDEVCAPS:
            {
                DeviceContext *pDeviceContext;

				if (pHWContext->IsAttached())
				{
					if (pStreamContext)
					{
						pDeviceContext=pStreamContext->GetDeviceContext();
					}
					else
					{
						pDeviceContext = pHWContext->GetOutputDeviceContext(uDeviceId);
					}

					dwRet = pDeviceContext->GetDevCaps((PVOID)dwParam1,dwParam2);
				}
				else
				{
					dwErrCode = ERROR_NOT_READY;
					dwRet = MMSYSERR_BADDEVICEID;
				}

                break;
            }


        case WIDM_GETDEVCAPS:
            {
                DeviceContext *pDeviceContext;

				if (pHWContext->IsAttached())
				{
					if (pStreamContext)
					{
						pDeviceContext=pStreamContext->GetDeviceContext();
					}
					else
					{
						pDeviceContext = pHWContext->GetInputDeviceContext(uDeviceId);
					}

					dwRet = pDeviceContext->GetDevCaps((PVOID)dwParam1,dwParam2);
				}
				else
				{
					dwErrCode = ERROR_NOT_READY;
					dwRet = MMSYSERR_BADDEVICEID;
				}

                break;
            }

        case WODM_GETEXTDEVCAPS:
            {
                DeviceContext *pDeviceContext;

                if (pStreamContext)
                {
                    pDeviceContext=pStreamContext->GetDeviceContext();
                }
                else
                {
                    pDeviceContext = pHWContext->GetOutputDeviceContext(uDeviceId);
                }

                dwRet = pDeviceContext->GetExtDevCaps((PVOID)dwParam1,dwParam2);
                break;
            }

        case WODM_OPEN:
            {
                // DEBUGMSG(ZONE_FUNCTION, (TEXT("WODM_OPEN\r\n"));
                DeviceContext *pDeviceContext = pHWContext->GetOutputDeviceContext(uDeviceId);
                dwRet = pDeviceContext->OpenStream((LPWAVEOPENDESC)dwParam1, dwParam2, (StreamContext **)dwUser);
                break;
            }

        case WIDM_OPEN:
            {
                // DEBUGMSG(ZONE_FUNCTION, (TEXT("WIDM_OPEN\r\n"));
                DeviceContext *pDeviceContext = pHWContext->GetInputDeviceContext(uDeviceId);
                dwRet = pDeviceContext->OpenStream((LPWAVEOPENDESC)dwParam1, dwParam2, (StreamContext **)dwUser);
                break;
            }

        case WODM_CLOSE:
        case WIDM_CLOSE:
            {
                // DEBUGMSG(ZONE_FUNCTION, (TEXT("WIDM_CLOSE/WODM_CLOSE\r\n"));
                dwRet = pStreamContext->Close();

                // Release stream context here, rather than inside StreamContext::Close, so that if someone
                // has subclassed Close there's no chance that the object will get released
                // out from under them.
                if (dwRet==MMSYSERR_NOERROR)
                {
                    pStreamContext->Release();
                }
                break;
            }

        case WODM_RESTART:
        case WIDM_START:
            {
                if (pHWContext->IsAttached())
                {
                    dwRet = pStreamContext->Run();
                }
                else
                {
                    dwRet = MMSYSERR_ERROR;
                }
                break;
            }

        case WODM_PAUSE:
        case WIDM_STOP:
            {
                dwRet = pStreamContext->Stop();
                break;
            }

        case WODM_GETPOS:
        case WIDM_GETPOS:
            {
                dwRet = pStreamContext->GetPos((PMMTIME)dwParam1);
                break;
            }

        case WODM_RESET:
        case WIDM_RESET:
            {
                dwRet = pStreamContext->Reset();
                break;
            }

        case WODM_WRITE:
        case WIDM_ADDBUFFER:
            {
                // DEBUGMSG(ZONE_FUNCTION, (TEXT("WODM_WRITE/WIDM_ADDBUFFER, Buffer=0x%x\r\n"),dwParam1);
                if (pHWContext->IsAttached())
                {
                    dwRet = pStreamContext->QueueBuffer((LPWAVEHDR)dwParam1);
                }
                else
                {
                    dwRet = MMSYSERR_ERROR;
                }
                break;
            }

        case WODM_GETVOLUME:
            {
                PDWORD pdwGain = (PDWORD)dwParam1;

                if (pStreamContext)
                {
                    *pdwGain = pStreamContext->GetGain();
                }
                else
                {
                    *pdwGain = GetMasterVolume(pHWContext);
                }
                dwRet = MMSYSERR_NOERROR;
                break;
            }

        case WODM_SETVOLUME:
            {
                DWORD dwGain = dwParam1;
                if (pStreamContext)
                {
                    dwRet = pStreamContext->SetGain(dwGain);
                }
                else
                {
                    dwRet = SetMasterVolume(pHWContext, dwGain);
                }
                break;
            }

        case WODM_BREAKLOOP:
            {
                dwRet = pStreamContext->BreakLoop();
                break;
            }

        case WODM_SETPLAYBACKRATE:
            {
                WaveStreamContext *pWaveStream = (WaveStreamContext *)dwUser;
                dwRet = pWaveStream->SetRate(dwParam1);
                break;
            }

        case WODM_GETPLAYBACKRATE:
            {
                WaveStreamContext *pWaveStream = (WaveStreamContext *)dwUser;
                dwRet = pWaveStream->GetRate((DWORD *)dwParam1);
                break;
            }

        case WODM_GETPROP:
            {
                // DEBUGMSG(ZONE_WODM, (TEXT("WODM_GETPROP\r\n")));

                PWAVEPROPINFO pPropInfo = (PWAVEPROPINFO) dwParam1;
                if (pStreamContext)
                {
                    dwRet = pStreamContext->GetProperty(pPropInfo);
                }
                else
                {
                    DeviceContext *pDeviceContext = pHWContext->GetOutputDeviceContext(uDeviceId);
                    dwRet = pDeviceContext->GetProperty(pPropInfo);
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
                    DeviceContext *pDeviceContext = pHWContext->GetInputDeviceContext(uDeviceId);
                    dwRet = pDeviceContext->GetProperty(pPropInfo);
                }
                break;
            }

        case WODM_SETPROP:
            {
                // DEBUGMSG(ZONE_WODM, (TEXT("WODM_SETPROP\r\n")));

                PWAVEPROPINFO pPropInfo = (PWAVEPROPINFO) dwParam1;
				RETAILMSG(0, (L"USBAUDIO:%S, WODM_SETPROP, ulPropId = %p\r\n", __FUNCTION__, pPropInfo->ulPropId));
                if (pStreamContext)
                {
                    dwRet = pStreamContext->SetProperty(pPropInfo);
                }
                else
                {
                    DeviceContext *pDeviceContext = pHWContext->GetOutputDeviceContext(uDeviceId);
                    dwRet = pDeviceContext->SetProperty(pPropInfo);
                }
                break;
            }

        case WIDM_SETPROP:
            {
                // DEBUGMSG(ZONE_WODM, (TEXT("WIDM_SETPROP\r\n")));

                PWAVEPROPINFO pPropInfo = (PWAVEPROPINFO) dwParam1;
				RETAILMSG(0, (L"USBAUDIO:%S, WIDM_SETPROP, ulPropId = %p\r\n", __FUNCTION__, pPropInfo->ulPropId));
                if (pStreamContext)
                {
                    dwRet = pStreamContext->SetProperty(pPropInfo);
                }
                else
                {
                    DeviceContext *pDeviceContext = pHWContext->GetInputDeviceContext(uDeviceId);
                    dwRet = pDeviceContext->SetProperty(pPropInfo);
                }
                break;
            }

        // unsupported messages
        case WODM_GETPITCH:
        case WODM_SETPITCH:
        case WODM_PREPARE:
        case WODM_UNPREPARE:
        case WIDM_PREPARE:
        case WIDM_UNPREPARE:
            default:
            dwRet  = MMSYSERR_NOTSUPPORTED;
            break;
        }
    }
    _except (GetExceptionCode() == STATUS_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        ERRORMSG(1, (TEXT("Access violation in HandleWaveMessage!!!!\r\n")));
        SetLastError(E_FAIL);
        dwRet  = MMSYSERR_INVALPARAM;
    }

	// Pass the return code back via pBufOut
    if (pdwResult)
    {
        *pdwResult = dwRet;
    }

    pHWContext->Unlock();

	if (dwErrCode != ERROR_SUCCESS)
		SetLastError(dwErrCode);

    return(TRUE);
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   BOOL | WAV_IOControl | Device IO control routine
//
//  @parm   DWORD | dwOpenData | Value returned from WAV_Open call
//
//  @parm   DWORD | dwCode |
//          IO control code for the function to be performed. WAV_IOControl only
//          supports one IOCTL value (IOCTL_WAV_MESSAGE)
//
//  @parm   PBYTE | pBufIn |
//          Pointer to the input parameter structure (<t MMDRV_MESSAGE_PARAMS>).
//
//  @parm   DWORD | dwLenIn |
//          Size in bytes of input parameter structure (sizeof(<t MMDRV_MESSAGE_PARAMS>)).
//
//  @parm   PBYTE | pBufOut | Pointer to the return value (DWORD).
//
//  @parm   DWORD | dwLenOut | Size of the return value variable (sizeof(DWORD)).
//
//  @parm   PDWORD | pdwActualOut | Unused
//
//  @rdesc  Returns TRUE for success, FALSE for failure
//
//  @xref   <t Wave Input Driver Messages> (WIDM_XXX) <nl>
//          <t Wave Output Driver Messages> (WODM_XXX)
//
// -----------------------------------------------------------------------------
extern "C" BOOL WAV_IOControl(PDWORD  pdwOpenData,
                   DWORD  dwCode,
                   PBYTE  pBufIn,
                   DWORD  dwLenIn,
                   PBYTE  pBufOut,
                   DWORD  dwLenOut,
                   PDWORD pdwActualOut)
{
    //DEBUGMSG(ZONE_INIT, (TEXT(">WAV_IOControl(%p,...)\n"), pdwOpenData));
    HardwareContext* pHWContext = (HardwareContext*) pdwOpenData;
	RETAILMSG(0, (L"USBAUDIO:%S, pHWContext = %p\r\n", __FUNCTION__, pHWContext));

    switch (dwCode)
    {
    case IOCTL_MIX_MESSAGE:
        return HandleMixerMessage(pHWContext, (PMMDRV_MESSAGE_PARAMS)pBufIn, (DWORD *)pBufOut);

    case IOCTL_WAV_MESSAGE:
        return HandleWaveMessage(pHWContext, (PMMDRV_MESSAGE_PARAMS)pBufIn, (DWORD *)pBufOut);

    // Power management functions.
    case IOCTL_POWER_CAPABILITIES:
    case IOCTL_POWER_QUERY:
    case IOCTL_POWER_SET:
    case IOCTL_POWER_GET:
        return pHWContext->PmControlMessage
                            (dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
    }

    return(FALSE);
}

