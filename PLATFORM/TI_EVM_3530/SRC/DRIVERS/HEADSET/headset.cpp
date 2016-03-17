//------------------------------------------------------------------------------
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
// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
//------------------------------------------------------------------------------
//


#include <windows.h>
#include <omap35xx.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <initguid.h>
#include <gpio.h>
#include <twl.h>
#include <wavext.h>
#include <head_set.h>
#include <oal.h>
#include <oalex.h>
#include <args.h>

////////////////////////////////////////////////////////////////////////////////
// HeadsetPresent
// Gets a value indicating whether a headset is present
#define SN_HEADSETPRESENT_ROOT HKEY_LOCAL_MACHINE
#define SN_HEADSETPRESENT_PATH TEXT("System\\State\\Hardware")
#define SN_HEADSETPRESENT_VALUE TEXT("Headset")

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
//  Set debug zones names and initial setting for driver
//
#ifndef SHIP_BUILD
#undef ZONE_ERROR
#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INFO           DEBUGZONE(4)
#define ZONE_IST            DEBUGZONE(5)
#define ZONE_HDS            DEBUGZONE(15)

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
DBGPARAM dpCurSettings = {
    L"HDS", {
        L"Errors",      L"Warnings",    L"Function",    L"Info",
        L"IST",         L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    0x0013
};

#endif

//------------------------------------------------------------------------------
//  Local Definitions

#define HDS_DEVICE_COOKIE           'hdsD'
#define HDS_GPIO_HSDET              194     // Triton GPIO 2
#define HDS_GPIO_HSDET_PU_PD_MASK   0xCF    // disable PU / PD

//------------------------------------------------------------------------------
//  Local Structures

typedef struct {
    DWORD priority256;
    HANDLE hTWL;
    HANDLE hGPIO;
    HANDLE hWAV;
    BOOL bPluggedIn;
    CRITICAL_SECTION cs;
    HANDLE hIntrEvent;
    HANDLE hIntrThread;
    BOOL intrThreadExit;
    DWORD hdstDetGpio;
    DWORD hdstMuteGpio;
} HeadsetDevice_t;

//------------------------------------------------------------------------------
//  Local Functions

BOOL	HDS_Deinit(DWORD context);
DWORD	HDS_IntrThread(VOID *pContext);


//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = {
    {
        L"Priority256", PARAM_DWORD, FALSE,
        offset(HeadsetDevice_t, priority256),
        fieldsize(HeadsetDevice_t, priority256), (VOID*)200
    }, {
        L"HdstDetGpio", PARAM_DWORD, TRUE,
        offset(HeadsetDevice_t, hdstDetGpio),
        fieldsize(HeadsetDevice_t, hdstDetGpio), NULL
    }, {
        L"HdstMuteGpio", PARAM_DWORD, TRUE,
        offset(HeadsetDevice_t, hdstMuteGpio),
        fieldsize(HeadsetDevice_t, hdstMuteGpio), NULL
    }
};


//------------------------------------------------------------------------------
//
//  Function:  HDS_Init
//
//  Called by device manager to initialize device.
//
DWORD HDS_Init(LPCTSTR szContext, LPCVOID pBusContext)
{
    DWORD rc = (DWORD)NULL;
    HeadsetDevice_t *pDevice = NULL;

    DEBUGMSG(ZONE_FUNCTION, (L"+HDS_Init(%s, 0x%08x)\r\n", szContext, pBusContext));

    // Create device structure
    pDevice = (HeadsetDevice_t *)LocalAlloc(LPTR, sizeof(HeadsetDevice_t));
    if(pDevice == NULL)
	{
        DEBUGMSG(ZONE_ERROR, (L"ERROR: HDS_Init: Failed allocate HDS driver structure\r\n"));
        goto cleanUp;
	}

    // initialize memory
    //
    memset(pDevice, 0, sizeof(HeadsetDevice_t));

    // Initialize crit section
    InitializeCriticalSection(&pDevice->cs);

    // Read device parameters
    if(GetDeviceRegistryParams(szContext, pDevice, dimof(s_deviceRegParams), s_deviceRegParams)!= ERROR_SUCCESS)
	{
        DEBUGMSG(ZONE_ERROR, (L"ERROR: HDS_Init: Failed read HDS driver registry parameters\r\n"));
//        goto cleanUp;
	}

    // Open WAV device
    pDevice->hWAV = CreateFile(L"WAV1:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if(pDevice->hWAV == INVALID_HANDLE_VALUE)
	{
        pDevice->hWAV = NULL;
        DEBUGMSG(ZONE_WARN, (L"WARN: HDS_Init: Failed open WAV1: device driver\r\n"));
	}

    // Open GPIO bus
    pDevice->hGPIO = GPIOOpen();
    if(pDevice->hGPIO == NULL)
	{
        DEBUGMSG(ZONE_ERROR, (L"ERROR: HDS_Init: Failed open GPIO bus driver\r\n"));
        goto cleanUp;
	}

    pDevice->bPluggedIn = 1;
    // Start interrupt service thread
    pDevice->intrThreadExit = FALSE;
    pDevice->hIntrThread = CreateThread(NULL, 0, HDS_IntrThread, pDevice, 0, NULL);
    if(!pDevice->hIntrThread)
	{
        DEBUGMSG (ZONE_ERROR, (L"ERROR: HDS_Init: Failed create interrupt thread\r\n"));
        goto cleanUp;
	}
    
    // Return non-null value
    rc = (DWORD)pDevice;

cleanUp:
    if(rc == 0)
	{
		HDS_Deinit((DWORD)pDevice);
	}

    DEBUGMSG(ZONE_FUNCTION, (L"-HDS_Init(rc = %d\r\n", rc));

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  HDS_Deinit
//
//  Called by device manager to uninitialize device.
//
BOOL HDS_Deinit(DWORD context)
{
    HeadsetDevice_t *pDevice = (HeadsetDevice_t*)context;

    DEBUGMSG(ZONE_FUNCTION, (L"+HDS_Deinit(0x%08x)\r\n", context));

    // Check if we get correct context
    if(!pDevice)
	{
        DEBUGMSG (ZONE_ERROR, (L"ERROR: HDS_Deinit: Incorrect context parameter\r\n"));
        return 0;
	}

    // Signal stop to threads
    pDevice->intrThreadExit = 1;
        
    // Close interrupt thread
    if(pDevice->hIntrThread)
	{
        // Close handle
        CloseHandle(pDevice->hIntrThread);
	}

    // Close GPIO driver
    // Disable GPIO interrupt
    if(pDevice->hGPIO)
	{
		GPIOClose(pDevice->hGPIO);
	}

    // Close interrupt handler
    if(pDevice->hIntrEvent)
		CloseHandle(pDevice->hIntrEvent);


    // Delete critical section
    DeleteCriticalSection(&pDevice->cs);

    // Free device structure
    LocalFree(pDevice);

    DEBUGMSG(ZONE_FUNCTION, (L"-HDS_Deinit\r\n"));
    return 1;
}

//------------------------------------------------------------------------------
//
//  Function:  HDS_Open
//
//  Called by device manager to open a device for reading and/or writing.
//
DWORD HDS_Open(DWORD context, DWORD accessCode, DWORD shareMode)
{
    return context;
}

//------------------------------------------------------------------------------
//
//  Function:  HDS_Close
//
//  This function closes the device context.
//
BOOL HDS_Close(DWORD context)
{
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  HDS_IOControl
//
//  This function sends a command to a device.
//
BOOL HDS_IOControl(DWORD context, DWORD code, UCHAR *pInBuffer, DWORD inSize, UCHAR *pOutBuffer, DWORD outSize, DWORD *pOutSize)
{
    BOOL rc = 0;
    HeadsetDevice_t *pDevice = (HeadsetDevice_t*)context;


    DEBUGMSG(ZONE_FUNCTION, (L"+HDS_IOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",
		context, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize));

    switch(code)
	{
		case IOCTL_HEADSET_MIC_AMP_CTRL:
		{
			HEADSET_MIC_AMP_CTRL on;
			if(!pInBuffer || inSize < sizeof(HEADSET_MIC_AMP_CTRL) || !CeSafeCopyMemory(&on, pInBuffer, sizeof(on)))
			{
				RETAILMSG(1, (TEXT(" HDS nvalid parameters\r\n")));
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
			}
			RETAILMSG(0, (TEXT(" HDS (mic = %d)\r\n"), on));
			rc = 1;
			break;
		}
	}

    DEBUGMSG(ZONE_FUNCTION, (L"-HDS_IOControl(rc = %d)\r\n", rc));

	return rc;
}

//------------------------------------------------------------------------------
//
//  Function: SetHeadsetHardwareState
//
//  Sets the headset hardware state in the registry.
//
VOID SetHeadsetHardwareState(BOOL fHeadsetIn)
{
    HKEY hKey;
    DWORD dwDisposition;
    if (RegCreateKeyEx(
            SN_HEADSETPRESENT_ROOT,
            SN_HEADSETPRESENT_PATH,
            0, NULL,
            REG_OPTION_VOLATILE,
            0, NULL,
            &hKey,
            &dwDisposition) == ERROR_SUCCESS)
    {
        DWORD dwData = fHeadsetIn ? 1 : 0;
        RegSetValueEx(
            hKey,
            SN_HEADSETPRESENT_VALUE,
            0,
            REG_DWORD,
            (const PBYTE) &dwData,
            sizeof(dwData));

        RegCloseKey(hKey);
    }
}

//------------------------------------------------------------------------------
//
//  Function:  HDS_InterruptThread
//
//  This function acts as the IST for the headset.
//
DWORD HDS_IntrThread(VOID *pContext)
{
    HeadsetDevice_t *pDevice = (HeadsetDevice_t*)pContext;
    BOOL fInitialDetection = TRUE;

    // Set the interrupt event to trigger initial state setting.
    //        
    if(pDevice->hWAV) 
    {
		pDevice->bPluggedIn = 1;
        DeviceIoControl(pDevice->hWAV, IOCTL_NOTIFY_HF_SPEAKERS, &pDevice->bPluggedIn, sizeof(BOOL), 0, 0, 0, 0);
		SetHeadsetHardwareState(1);
	}

    return ERROR_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function:  DllMain
//
//  Standard Windows DLL entry point.
//
BOOL __stdcall DllMain(HANDLE hDLL, DWORD reason, VOID *pReserved)
{
    switch(reason)
	{
        case DLL_PROCESS_ATTACH:
            RETAILREGISTERZONES((HMODULE)hDLL);
            DisableThreadLibraryCalls((HMODULE)hDLL);
            break;
	}

    return TRUE;
}
