/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File: madc.c
//
#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
//#include <oal.h>
//#include <oalex.h>
//#include <omap3430.h>

#include <initguid.h>
//#include <twl.h>
//#include <twl4030_bci.h>
#include <madc.h>
#include <light_sense.h>

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
//  Set debug zones names and initial setting for driver
//
#ifndef SHIP_BUILD

#undef ZONE_ERROR
#undef ZONE_WARN
#undef ZONE_FUNCTION
#undef ZONE_INFO

#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INFO           DEBUGZONE(3)

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
DBGPARAM dpCurSettings = {
    L"LTSR", {
        L"Errors",      L"Warnings",    L"Function",    L"Info",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    0x0003
};

#endif

//------------------------------------------------------------------------------
//  Local Definitions

#define LTSR_DEVICE_COOKIE       'lgtS'
#define LTSR_MAX_LIGHT			 250

//------------------------------------------------------------------------------
//  Local Structures

typedef struct tag_light_sense{
    UINT32              cookie;
	UINT32				quit;
    CRITICAL_SECTION    cs;
    HANDLE              hMADC;
    HANDLE              hLightConfig;
	HANDLE				hLightThread;
    UINT32              priority256;
    UINT32              high_light;
    UINT32              low_light;
	UINT32				poll_timeout;
	UINT32				client[32];
} light_sense;

typedef struct tag_light_sense_handle
{
	HANDLE hQue;
	light_sense *LightSenseObject;
	TCHAR QueueName[LIGHTSENSE_CONFIG_LEN/2];
}light_sense_handle;

//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = {
    {
        L"Priority256", PARAM_DWORD, FALSE, offset(light_sense, priority256),
        fieldsize(light_sense, priority256), (VOID*)251
    }, {
        L"HighLight", PARAM_DWORD, FALSE, offset(light_sense, high_light),
        fieldsize(light_sense, high_light), (VOID*)0x1F4
    }, {
        L"LowLight", PARAM_DWORD, FALSE, offset(light_sense, low_light),
        fieldsize(light_sense, low_light), (VOID*)0x3C
    }, {
        L"PollTimeout", PARAM_DWORD, FALSE, offset(light_sense, poll_timeout),
        fieldsize(light_sense, poll_timeout), (VOID*)0x7D0
    }
};

//------------------------------------------------------------------------------
//  Local Functions
BOOL LTS_Deinit(DWORD context);

//------------------------------------------------------------------------------
//
//  Function:  LightThread
//
//
DWORD LightThread(VOID *pContext)
{
    light_sense *pDevice = (light_sense *)pContext;
    UINT32 currentAmbientLight = 0;
    UINT32 previousVolts = 0;
	UINT32 currentVolts = 0;
	UINT32 averageVolts = 0;
	AMBIENT_LIGHT currentState = AMBIENT_LIGHT_UNSPEC, previousState = AMBIENT_LIGHT_UNSPEC;
    DWORD Event;
	DWORD fMustUpdate = 0;

    if(!pDevice) 
	{
        DEBUGMSG(ZONE_ERROR, (L" LightThread: Invalid context passed to thread routine.\r\n"));
        ASSERT( 0 );
        return -1;
	}

    if(!pDevice->hMADC)
	{
        DEBUGMSG(ZONE_ERROR, (L" LightThread: Failed opening MADC driver\r\n"));
        ASSERT( 0 );
        return -1;
	}

    if(!pDevice->hLightConfig)
	{
        DEBUGMSG(ZONE_ERROR, (L" LightThread: light config event failure\r\n"));
        ASSERT( 0 );
        return -1;
	}

    // Loop until we are not stopped...
    while(!pDevice->quit)
	{
        // Monitor MADC GP analog input 2
#if 1
		INT32 rc;
        // Sense ambient light
        rc = MADCReadValue(pDevice->hMADC, MADC_CHANNEL_2, (DWORD *)&currentAmbientLight, 1);
        if(!rc)
		{
			ASSERT( 0 );
//			return -1;
		}
		rc = MADCConvertToVolts(pDevice->hMADC, MADC_CHANNEL_2, (DWORD *)&currentAmbientLight, (DWORD *)&currentVolts, 1);
		if(!rc)
		{
			ASSERT( 0 );
//			return -1;
		}

        if(currentVolts > previousVolts)
			rc = currentVolts - previousVolts;
		else
			rc = previousVolts - currentVolts;

		if(fMustUpdate || rc > LIGHT_SENSE_ACCURACY)
		{
			averageVolts = currentVolts;

			for(int i = 0; i < (LIGHTE_SENSE_SAMPLES - 1); i++)
			{
				// Sense ambient light
				rc = MADCReadValue(pDevice->hMADC, MADC_CHANNEL_2, (DWORD *)&currentAmbientLight, 1);
				if(!rc)
				{
					ASSERT( 0 );
//					return -1;
				}
				rc = MADCConvertToVolts(pDevice->hMADC, MADC_CHANNEL_2, (DWORD *)&currentAmbientLight, (DWORD *)&currentVolts, 1);
				if(!rc)
				{
					ASSERT( 0 );
//					return -1;
				}
				averageVolts += currentVolts;
				Sleep((((pDevice->poll_timeout + 100)/100) + LIGHTE_SENSE_SAMPLES)/LIGHTE_SENSE_SAMPLES);
			}

			averageVolts /= LIGHTE_SENSE_SAMPLES;

			if(averageVolts > previousVolts)
				rc = averageVolts - previousVolts;
			else
				rc = previousVolts - averageVolts;

			if(fMustUpdate || rc > LIGHT_SENSE_ACCURACY)
			{
				// Notify clients ambient light
				previousVolts = averageVolts;

				if(averageVolts >= pDevice->high_light)
				{
					// Notify high light
					currentState = AMBIENT_LIGHT_HIGH;
				}
				else if(averageVolts < pDevice->low_light)
				{
					// Notify low light
					currentState = AMBIENT_LIGHT_LOW;
				}
				else
				{
					// Notify normal light
					currentState = AMBIENT_LIGHT_NORMAL;
				}

				if(fMustUpdate || currentState != previousState)
				{
					light_sense_handle *hDevice;

					fMustUpdate = 0;

					RETAILMSG(0, (L"Ambient lightness is %s (%d)\r\n",
						(currentState == AMBIENT_LIGHT_HIGH)?L"high":
						(currentState == AMBIENT_LIGHT_LOW)?L"low":L"normal",
						averageVolts));

					previousState = currentState;

					EnterCriticalSection(&pDevice->cs);
					for(int i = 0; i < (sizeof(pDevice->client)/sizeof(pDevice->client[0])); i++)
					{
						hDevice = (light_sense_handle *)pDevice->client[i];
						if(hDevice != 0)
						{
							WriteMsgQueue(hDevice->hQue, &currentState, sizeof(currentState), 200, 0);
							RETAILMSG(0, (L"LightThread: Message written %d %d\r\n", GetTickCount(), GetLastError()));
						}
					}
					LeaveCriticalSection(&pDevice->cs);
				}
			}
		}
#endif   
		// Wait for config change
		Event = WaitForSingleObject(pDevice->hLightConfig, pDevice->poll_timeout);
		if(Event == WAIT_OBJECT_0)
		{
			fMustUpdate = 1;
			// update light sense config
			if(GetDeviceRegistryParams(LIGHT_SENSE_REG_KEY, pDevice, dimof(s_deviceRegParams), s_deviceRegParams) != ERROR_SUCCESS)
			{
				RETAILMSG(1, (L"ERROR: LightThread: Failed read LTSR driver registry parameters\r\n"));
			}
			RETAILMSG(0, (L"LightThread: Config changed %d\r\n", GetTickCount()));
		}
		if(Event == WAIT_FAILED)
		{
			ASSERT( 0 );
			return -1;
		}
	}

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  LTS_Init
//
//  Called by device manager to initialize device.
//
DWORD LTS_Init(LPCTSTR szContext, LPCVOID pBusContext)
{
    light_sense *pDevice;

    DEBUGMSG(ZONE_FUNCTION, (L"+LTS_Init(%s, 0x%08x)\r\n", szContext, pBusContext));

    // Create device structure
    pDevice = (light_sense *)LocalAlloc(LPTR, sizeof(light_sense));
    if(!pDevice)
	{
        DEBUGMSG(ZONE_ERROR, (L"ERROR: LTS_Init: Failed allocate MADC driver structure\r\n"));

        return 0;
	}
	for(int i = 0; i < (sizeof(pDevice->client)/sizeof(pDevice->client[0])); i++)
		pDevice->client[i] = 0;

    // Read device parameters
    if(GetDeviceRegistryParams(szContext, pDevice, dimof(s_deviceRegParams), s_deviceRegParams) != ERROR_SUCCESS)
	{
        DEBUGMSG(ZONE_ERROR, (L"ERROR: LTS_Init: Failed read LTSR driver registry parameters\r\n"));
        LTS_Deinit((DWORD)pDevice);
		return 0;
	}

    pDevice->hLightConfig = CreateEvent(0, 0, 0, LIGHTSENSE_CONFIG);
    if(!pDevice->hLightConfig)
	{
        DEBUGMSG(ZONE_ERROR, (L"ERROR: LTS_Init: light config event failure\r\n"));
        LTS_Deinit((DWORD)pDevice);
		return 0;
	}
    // Set cookie
    pDevice->cookie = LTSR_DEVICE_COOKIE;
	pDevice->quit = 0;

    // Initialize critical sections
    InitializeCriticalSection(&pDevice->cs);

    // Open Triton device driver
    pDevice->hMADC = MADCOpen();
    if(!pDevice->hMADC)
	{
        DEBUGMSG( ZONE_ERROR, (L"ERROR: LTS_Init: Failed open Triton device driver\r\n"));
        LTS_Deinit((DWORD)pDevice);
		return 0;
	}

	// Start polling thread
	pDevice->hLightThread = CreateThread(0, 0, LightThread, pDevice, 0, 0);
	
	if(!pDevice->hLightThread)
	{
		DEBUGMSG (ZONE_ERROR, (L"ERROR: LTS_Init: Failed to create light sense thread\r\n"));
        LTS_Deinit((DWORD)pDevice);
		return 0;
	}
    
	// Set thread priority
    CeSetThreadPriority(pDevice->hLightThread, pDevice->priority256);

    DEBUGMSG(ZONE_FUNCTION, (L"-LTS_Init(%x\r\n", pDevice));

    return (DWORD)pDevice;
}

//------------------------------------------------------------------------------
//
//  Function:  LTS_Deinit
//
//  Called by device manager to uninitialize device.
//
BOOL LTS_Deinit(DWORD context)
{
    light_sense *pDevice = (light_sense *)context;

    DEBUGMSG(ZONE_FUNCTION, (L"+LTS_Deinit(0x%08x)\r\n", context));

    // Check if we get correct context
    if(!pDevice || (pDevice->cookie != LTSR_DEVICE_COOKIE))
	{
        DEBUGMSG (ZONE_ERROR, (L"ERROR: LTS_Deinit: Incorrect context parameter\r\n"));
        return 0;
	}

	pDevice->quit = 1;
	if(pDevice->hLightThread)
	{
		// Wait until thread exits
		WaitForSingleObject(pDevice->hLightThread, INFINITE);
		// Close handle
		CloseHandle(pDevice->hLightThread);
	}

    //Close handle to MADC driver
    if(pDevice->hMADC)
	{
        CloseHandle(pDevice->hMADC);
        pDevice->hMADC = NULL;
	}

	if(pDevice->hLightConfig)
	{
        CloseHandle(pDevice->hLightConfig);
        pDevice->hLightConfig = 0;
	}

    // Delete critical sections
    DeleteCriticalSection(&pDevice->cs);

    // Free device structure
    LocalFree(pDevice);

    DEBUGMSG(ZONE_FUNCTION, (L"-LTS_Deinit\r\n"));
    return 1;
}

//------------------------------------------------------------------------------
//
//  Function:  LTS_Open
//
//  Called by device manager to open a device for reading and/or writing.
//
DWORD LTS_Open(DWORD context, DWORD accessCode, DWORD shareMode)
{
	light_sense			*pDevice = (light_sense *)context;
	light_sense_handle	*hDevice;
    MSGQUEUEOPTIONS		msgOptions;
	DWORD				rc = 0;

	EnterCriticalSection(&pDevice->cs);
	for(int i = 0; i < (sizeof(pDevice->client)/sizeof(pDevice->client[0])); i++)
	{
		if(pDevice->client[i] == 0)
		{
			// Create device handle
			hDevice = (light_sense_handle *)LocalAlloc(LPTR, sizeof(light_sense_handle));
			if(!hDevice)
			{
				RETAILMSG(1, (L"ERROR: LTS_Init: Failed allocate MADC driver structure\r\n"));

				SetLastError(ERROR_INVALID_PARAMETER);
				rc = 0;
				break;
			}

			memset(&msgOptions, 0, sizeof(msgOptions));
			msgOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
			msgOptions.dwFlags = 0;//MSGQUEUE_ALLOW_BROKEN;
			msgOptions.cbMaxMessage = sizeof(AMBIENT_LIGHT);
			msgOptions.bReadAccess = 0;

			memset(hDevice->QueueName, 0, sizeof(hDevice->QueueName));
			wsprintf(hDevice->QueueName, L"%s%d", LIGHTSENSE_CONFIG, i); 
			hDevice->hQue = CreateMsgQueue(hDevice->QueueName, &msgOptions);

			hDevice->LightSenseObject = pDevice;

			pDevice->client[i] = (UINT32)hDevice;
			rc = (DWORD)hDevice;
			SetEvent(pDevice->hLightConfig);
			RETAILMSG(0, (L"LTS_Open: %d, %s\r\n", GetTickCount(), hDevice->QueueName));
			break;
		}
	}
	LeaveCriticalSection(&pDevice->cs);

	return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  LTS_Close
//
//  This function closes the device context.
//
BOOL LTS_Close(DWORD context)
{
	BOOL rc = 0;
	light_sense_handle *hDevice = (light_sense_handle *)context;
	light_sense		   *pDevice = (light_sense *)hDevice->LightSenseObject;

	EnterCriticalSection(&pDevice->cs);
	for(int i = 0; i < (sizeof(pDevice->client)/sizeof(pDevice->client[0])); i++)
	{
		if(pDevice->client[i] == (UINT32)hDevice)
		{
			CloseMsgQueue(hDevice->hQue);
			LocalFree(hDevice);
			pDevice->client[i] = 0;
			rc = i + 1;
			break;
		}
	}
	LeaveCriticalSection(&pDevice->cs);

	SetLastError(ERROR_INVALID_PARAMETER);

    return 0;
}

//------------------------------------------------------------------------------
//
//  Function:  LTS_IOControl
//
//  This function sends a command to a device.
//
BOOL LTS_IOControl(DWORD context, DWORD code, UCHAR *pInBuffer, DWORD inSize, UCHAR *pOutBuffer, DWORD outSize, DWORD *pOutSize)
{
    BOOL rc = 1;
	light_sense_handle *hDevice = (light_sense_handle *)context;
	light_sense		   *pDevice = (light_sense *)hDevice->LightSenseObject;

    DEBUGMSG(ZONE_FUNCTION, (L"+LTS_IOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n", context, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize));

    // Check if we get correct context
    if(!pDevice || (pDevice->cookie != LTSR_DEVICE_COOKIE))
	{
        DEBUGMSG(ZONE_ERROR, (L"ERROR: LTS_IOControl: Incorrect context parameter\r\n"));

        SetLastError(ERROR_INVALID_PARAMETER);

        return 0;
    }
	switch (code)
	{
		case IOCTL_LTSR_GETQUEUENAME:
		{
			if(pOutBuffer && outSize >= sizeof(hDevice->QueueName))
			{
				__try
				{
					EnterCriticalSection(&pDevice->cs);
					RETAILMSG(0, (L"IOCTL_LTSR_GETQUEUENAME: %s\r\n", hDevice->QueueName));
					memcpy(pOutBuffer, hDevice->QueueName, sizeof(hDevice->QueueName));   
					LeaveCriticalSection(&pDevice->cs);

                    rc = 1;

				}
				__except(EXCEPTION_EXECUTE_HANDLER) 
                {
					RETAILMSG(1, (TEXT("Exception in IOCTL_LTSR_GETQUEUENAME\r\n")));
				}
			}
			break;
		}
		default:
		{
			SetLastError(ERROR_INVALID_PARAMETER);
			rc = 0;
			break;
		}
    }

    DEBUGMSG(ZONE_FUNCTION, (L"-LTS_IOControl(rc = %d)\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  DllMain
//
//  DLL entry point.
//
BOOL WINAPI DllMain(HANDLE hDLL, ULONG Reason, LPVOID Reserved)
{
	switch(Reason) 
    {
		case DLL_PROCESS_ATTACH:
			RETAILREGISTERZONES((HMODULE)hDLL);
            DisableThreadLibraryCalls((HMODULE)hDLL);
            break;
	}

    return 1;
}




