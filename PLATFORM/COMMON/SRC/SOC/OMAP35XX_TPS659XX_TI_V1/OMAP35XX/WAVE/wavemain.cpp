// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

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

// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)

//------------------------------------------------------------------------------
//  Local Definitions

#define WAV_DEVICE_COOKIE       'wavD'
#define WAV_INSTANCE_COOKIE     'wavI'

//------------------------------------------------------------------------------
//  Device Structure
//
struct WAVDevice_t {
    DWORD           cookie;
    CAudioManager  *pAudioManager;
};

//------------------------------------------------------------------------------
//  Instance Structure
//
struct WAVInstance_t {
    DWORD           cookie;
    WAVDevice_t    *pDevice;
};

//------------------------------------------------------------------------------
//
//  global mutex
//
static CRITICAL_SECTION  s_cs;


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
//          <f WAV_Open> and <f WAV_Close>. All
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
//  @doc    EnterMutex
//
//  @func   PVOID | EnterMutex | marks beginning of mutex
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------

EXTERN_C void
EnterMutex()
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+EnterMutex\r\n"));

    ::EnterCriticalSection(&s_cs);

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-EnterMutex\r\n"));
}

// -----------------------------------------------------------------------------
//
//  @doc    ExitMutex
//
//  @func   PVOID | ExitMutex | marks end of mutex
//
//  @rdesc  none
//
// -----------------------------------------------------------------------------

EXTERN_C void
ExitMutex()
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+ExitMutex\r\n"));

    ::LeaveCriticalSection(&s_cs);

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-ExitMutex\r\n"));
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

EXTERN_C
BOOL
WAV_Deinit(DWORD dwData)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+WAV_Deinit(%d)\r\n", dwData));

    BOOL bSuccess = TRUE;

    UninitializeHardware(dwData);

    if (dwData)
        {
        WAVDevice_t *pDevice = (WAVDevice_t*)dwData;

        if (pDevice->cookie != WAV_DEVICE_COOKIE)
            {
            bSuccess = FALSE;
            goto cleanUp;
            }

        if (pDevice->pAudioManager)
            {
            DeleteAudioManager(pDevice->pAudioManager);
            }

        LocalFree(pDevice);
        }

    ::DeleteCriticalSection(&s_cs);

cleanUp:

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-WAV_Deinit\r\n"));
    return bSuccess;
}


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

EXTERN_C
DWORD
WAV_Init(
    LPCWSTR szContext,
    LPCVOID pBusContext
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+WAV_Init(%s)\r\n", szContext));

    BOOL bSuccess = FALSE;

        // Create device structure
        //
    ::InitializeCriticalSection(&s_cs);
    WAVDevice_t *pDevice = (WAVDevice_t*)LocalAlloc(LPTR, sizeof(WAVDevice_t));
    if (pDevice == NULL)
        {
        RETAILMSG(ZONE_ERROR, (L"WAV:ERROR: WAV_Init: "
            L"Failed allocate WAV controller structure\r\n"
            ));
        goto cleanUp;
        }

    // Set cookie
    memset(pDevice, 0, sizeof(WAVDevice_t));
    pDevice->cookie = WAV_DEVICE_COOKIE;

    pDevice->pAudioManager = CreateAudioManager();
    if (pDevice->pAudioManager == NULL)
        {
        RETAILMSG(ZONE_ERROR,
            (L"WAV:ERROR: Failed to instantiate audio device manager\r\n")
            );
        goto cleanUp;
        }

    if (InitializeHardware(szContext, pBusContext) == FALSE)
        {
        RETAILMSG(ZONE_ERROR,
            (L"WAV:ERROR: Failed to initialize hardware\r\n")
            );
        goto cleanUp;
        }

    if (!pDevice->pAudioManager->initialize(szContext, pBusContext))
        {
        RETAILMSG(ZONE_ERROR,
            (L"WAV:ERROR: Failed to initialize audio device manager\r\n")
            );
        goto cleanUp;
        }

    bSuccess = TRUE;

cleanUp:
    if (bSuccess == FALSE)
        {
        WAV_Deinit((DWORD)pDevice);
        }

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-WAV_Init\r\n"));

    return (DWORD)pDevice;
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
EXTERN_C PDWORD
WAV_Open( DWORD dwData,
          DWORD dwAccess,
          DWORD dwShareMode)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+WAV_Open\r\n"));

    WAVInstance_t *pInstance = NULL;
    WAVDevice_t *pDevice = (WAVDevice_t*)dwData;
    if (pDevice->cookie != WAV_DEVICE_COOKIE)
        {
        RETAILMSG(ZONE_ERROR, (L"WAV:!ERROR-invalid wav device cookie"));
        goto cleanUp;
        }

    pInstance = (WAVInstance_t*)LocalAlloc(LPTR, sizeof(WAVInstance_t));
    if (pInstance == NULL)
        {
        RETAILMSG(ZONE_ERROR,
            (L"WAV:!ERROR-unable to create wav instance\r\n")
            );
        goto cleanUp;
        }

    pInstance->cookie = WAV_INSTANCE_COOKIE;
    pInstance->pDevice = pDevice;

cleanUp:

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-WAV_Open\r\n"));

    return (PDWORD)pInstance;
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
EXTERN_C BOOL
WAV_Close(PDWORD pdwData)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+Close\r\n"));

    BOOL bSuccess = TRUE;
    if (pdwData)
        {
        WAVInstance_t *pInstance = (WAVInstance_t*)pdwData;

        if (pInstance->cookie != WAV_INSTANCE_COOKIE)
            {
            bSuccess = FALSE;
            goto cleanUp;
            }
        LocalFree(pInstance);
        }

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-Close\r\n"));
    return bSuccess;
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
extern "C" BOOL
WAV_IOControl(     DWORD dwOpenData,
                   DWORD  dwCode,
                   PBYTE  pBufIn,
                   DWORD  dwLenIn,
                   PBYTE  pBufOut,
                   DWORD  dwLenOut,
                   PDWORD pdwActualOut)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+WAV_IOControl(dwCode=%d)\r\n", dwCode));
	RETAILMSG(WAVE_IOCTL_DBG, (L"WAV:+WAV_IOControl(dwCode=0x%X)\r\n", dwCode));

    BOOL bSuccess = FALSE;

    WAVInstance_t *pInstance = (WAVInstance_t*)dwOpenData;
    if (pInstance->cookie != WAV_INSTANCE_COOKIE)
        {
        RETAILMSG(ZONE_ERROR,
            (L"WAV:!ERROR - Unknown object accessing audio driver\r\n")
            );

        SetLastError(ERROR_ACCESS_DENIED);
        goto cleanUp;
        }

    CAudioManager *pAudioManager = pInstance->pDevice->pAudioManager;

    EnterMutex();
    pAudioManager->Lock();
    __try
        {
        bSuccess = pAudioManager->process_IOControlMsg(dwCode, pBufIn,
                        dwLenIn, pBufOut, dwLenOut, pdwActualOut
                        );
        }
    __except (GetExceptionCode() == STATUS_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
        RETAILMSG(ZONE_ERROR,
            (L"WAV:!ERROR-Access violation in WAV_IOControl\r\n")
            );

        RETAILMSG(WAVE_IOCTL_DBG,
            (L"WAV:!ERROR-Access violation in WAV_IOControl\r\n")
            );

        SetLastError(E_FAIL);
        }
    pAudioManager->Unlock();
    ExitMutex();

cleanUp:

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-WAV_IOControl(bSuccess=%d)\r\n", bSuccess));
	RETAILMSG(WAVE_IOCTL_DBG, (L"WAV:-WAV_IOControl(bSuccess=%d)\r\n", bSuccess));
    return bSuccess;
}

//------------------------------------------------------------------------------
//
//  Function:  DllMain
//
//  Standard Windows DLL entry point.
//
BOOL
__stdcall
DllMain(
    HANDLE hDLL,
    DWORD reason,
    VOID *pReserved
    )
{
    switch (reason)
        {
        case DLL_PROCESS_ATTACH:
            RETAILREGISTERZONES((HMODULE)hDLL);
            DisableThreadLibraryCalls((HMODULE)hDLL);
            break;
        }
    return TRUE;
}
