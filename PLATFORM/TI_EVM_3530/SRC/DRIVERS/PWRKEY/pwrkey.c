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
// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky, Vitaly Iliasov, Ran Meyerstein
//
//
//  File: pwrkey.c
//
//  This file implements device driver for pwrkey. The driver isn't implemented
//  as classical keyboard driver. Instead implementation uses stream driver
//  model and it calls SetSystemPowerState to suspend the system.
//
#include <windows.h>
#include <winuser.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap35xx.h>
#include <tps659xx.h>
#include <initguid.h>
#include <gpio.h>
#include <twl.h>
#include <extfile.h>
#include <storemgr.h>
#include <bsp.h>
#include <args.h>
#include <devload.h>
#include "tempsensor.h"

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
#define ZONE_IST            DEBUGZONE(4)

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
DBGPARAM dpCurSettings = {
    L"pwrkey", {
        L"Errors",      L"Warnings",    L"Function",    L"Info",
        L"IST",         L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    0x0003
};

#endif

#define QUEUE_ENTRIES           8
#define MAX_NAMELEN             200
#define QUEUE_SIZE              (QUEUE_ENTRIES * (sizeof(POWER_BROADCAST) + MAX_NAMELEN))

//------------------------------------------------------------------------------
//  Local Structures
typedef enum {
	PwlUnspecified = -1,
	PwlDDIOff,
	PwlFSOff
}PWL_STATE;

typedef struct {
	HANDLE hTWL;
	UINT32 clk_on;
	UINT32 wake_en;
	HANDLE clkGpio;
	HANDLE wakeGpio;
    HANDLE pWake;
	HANDLE pwlLost;
	HANDLE pwlGpio;
	HANDLE pwlConfig;
	HANDLE pwlNotify;
	UINT32 pwlPin;
	UINT32 pwlUSD;
	UINT32 pwlIgnore;
	UINT32 priority256;
	PWL_STATE pwlState;
	UINT32 pwulFlags;
	int stopNotify;
#ifdef BSP_TEMPSENSOR
	TempSensor_t TempSensor;
#endif
} PwrkeyDevice_t;

static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = {
    { L"UrgentShutdowndelay", PARAM_DWORD, 0, offset(PwrkeyDevice_t, pwlUSD),
	  fieldsize(PwrkeyDevice_t, pwlUSD), (VOID*)1000
    },
    { L"IgnoreDelay", PARAM_DWORD, 0, offset(PwrkeyDevice_t, pwlIgnore),
	  fieldsize(PwrkeyDevice_t, pwlIgnore), (VOID*)250
    },
    {
        L"Priority256", PARAM_DWORD, 0, offset(PwrkeyDevice_t, priority256),
        fieldsize(PwrkeyDevice_t, priority256), (VOID*)50
    }
};

int fs_off = 0;

int set_store_power(CEDEVICE_POWER_STATE *ps)
{
	UINT32 ret;
	HANDLE h;
	TCHAR buf[512];
    STOREINFO si = {0};
	PARTINFO  pi = {0};

	RETAILMSG(0, (L"Enamerate all stores\r\n"));
	si.cbSize = sizeof(STOREINFO);
    h = FindFirstStore(&si);

	if((HANDLE)-1 != h)
	{
		do
		{
			wsprintf(buf, L"{8DD679CE-8AB4-43c8-A14A-EA4963FAA715}\\%s", si.szDeviceName);
			if(0 == _tcsnicmp(si.szDeviceName, L"DSK", 3))
			{
				RETAILMSG(0, (L"Set D%d power state %s\r\n", *ps, si.szDeviceName));
				ret = SetDevicePower(buf, POWER_NAME, *ps);
			}
			else
				ret = ERROR_SUCCESS;

			RETAILMSG(1, (L"'%s' is%sset to D%d\r\n", buf, (ret == ERROR_SUCCESS)?L" ":L" failure to ", *ps));
		}while(FindNextStore(h, &si));

		FindCloseStore(h);
	}

	return ERROR_SUCCESS;
}

DWORD PWR_SPS_Monitor(VOID *pContext)
{
	HANDLE hPN;
	HANDLE hPNQ;
	MSGQUEUEOPTIONS msgOptions = {0};
    
	union {
        UCHAR buf[QUEUE_SIZE];
        POWER_BROADCAST pwrBC;
    } u;

    int InQ;
    DWORD Flags = 0;

	PwrkeyDevice_t *pDevice = (PwrkeyDevice_t*)pContext;

    InQ = 0;
    memset(u.buf, 0, QUEUE_SIZE);
	pDevice->pwulFlags = 0;

	// create a message queue for Power Manager notifications
    msgOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
    msgOptions.dwFlags = 0;
    msgOptions.dwMaxMessages = QUEUE_ENTRIES;
    msgOptions.cbMaxMessage = sizeof(POWER_BROADCAST) + MAX_NAMELEN;
    msgOptions.bReadAccess = TRUE;

	CeSetThreadPriority(GetCurrentThread(), pDevice->priority256 - 1);
    hPNQ = CreateMsgQueue(NULL, &msgOptions);
    if(!hPNQ)
	{
        RETAILMSG(1, (TEXT("PWR_SPS_Monitor: Cannot create Power Broadcast MQ: %d\r\n"), GetLastError()));
        return 0;
    }

    // request Power notifications
    hPN = RequestPowerNotifications(hPNQ, PBT_RESUME | PBT_TRANSITION);
    if(!hPN)
	{
        RETAILMSG(1, (TEXT("PWR_SPS_Monitor: Cannot request power notifications: %d\r\n"), GetLastError()));

        return 0;
    }

	do
	{
		if(WAIT_OBJECT_0 == WaitForSingleObject(hPNQ, INFINITE))
		{
			if(!ReadMsgQueue(hPNQ, u.buf, QUEUE_SIZE, &InQ, INFINITE, &Flags))
			{
				RETAILMSG(1, (TEXT("PWR_SPS_Monitor: Failure to read MQ: %d\r\n"), GetLastError()));
			}
			else if(InQ >= sizeof(POWER_BROADCAST))
			{
				PPOWER_BROADCAST pB = &u.pwrBC;
				Flags = pB->Flags;
		        
				switch(pB->Message) 
				{
					case PBT_TRANSITION:
					{
						pDevice->pwulFlags = Flags;
						RETAILMSG(0, (TEXT("PWR_SPS_Monitor: SPS: %X\r\n"), Flags));
						break;
					}
					case PBT_RESUME:
					{
						pDevice->pwulFlags &= ~(POWER_STATE_ON | POWER_STATE_OFF |
												POWER_STATE_BOOT | POWER_STATE_IDLE |
												POWER_STATE_SUSPEND | POWER_STATE_RESET |
												POWER_STATE_USERIDLE);
						RETAILMSG(0, (TEXT("PWR_SPS_Monitor: SPS: resume %X\r\n"), pDevice->pwulFlags));
						break;
					}
					default:
						break;
				}
			}
		}
		else
			break;
	}while(!pDevice->stopNotify);

	StopPowerNotifications(hPN);
	CloseMsgQueue(hPNQ);

	return 0;
}

DWORD PWR_Lost_ConfigThread(VOID *pContext)
{
	PwrkeyDevice_t *pDevice = (PwrkeyDevice_t*)pContext;
	UINT32 obj;

	if(!pDevice)
		return 0;

	do
	{
		obj = WaitForSingleObject(pDevice->pwlConfig, INFINITE);
		if(obj != WAIT_OBJECT_0)
			break;
		if(GetDeviceRegistryParams(L"Drivers\\BuiltIn\\pwrkey", pDevice, dimof(s_deviceRegParams), s_deviceRegParams) != ERROR_SUCCESS)
		{
			RETAILMSG(1, (L"PWR_Lost_ConfigThread: Failed read registry\r\n"));
		}
		SetEvent(pDevice->pwlLost);
	}while(pDevice->pwlGpio != (void *)-1);

	return 0;
}

ULONG WINAPI unsafe_shut_down(void *arg)
{
	void *vp = OpenWatchDogTimer(L"Power Lost", 0);

	RETAILMSG(0, (L"unsafe_shut_down :  Wait for action (%x, %d)\r\n", vp, GetTickCount()));
	if(vp)
	{
		if(WAIT_OBJECT_0 == WaitForSingleObject(vp, INFINITE))
		{
			RETAILMSG(1, (L"unsafe_shut_down :  Urgent shutdown (%d)\r\n", GetTickCount()));
		#if defined (BSP_DEBUG_SERIAL)
			Sleep(50); // for debugging purposes only
		#endif
			if(fs_off)
				FileSystemPowerFunction(FSNOTIFY_POWER_OFF);

			KernelLibIoControl((HANDLE)KMOD_OAL, IOCTL_HAL_SHUTDOWN, 0, 0, 0, 0, 0);
		}
		CloseHandle(vp);
	}
	RETAILMSG(0, (L"unsafe_shut_down :  Leave (%d, %d)\r\n", GetLastError(), GetTickCount()));


	return 0;
}

DWORD PWR_Lost_IntrThread(VOID *pContext)
{
    PwrkeyDevice_t *pDevice = (PwrkeyDevice_t*)pContext;
	DWORD  code;
	void *vp;

	CEDEVICE_POWER_STATE ps;
	UINT32 timeout = INFINITE;

	IOCTL_GPIO_SET_DEBOUNCE_TIME_IN debounce;
    
	if(!pDevice)
		return 0;

	if(!pDevice->pwlGpio)
    {
		RETAILMSG(1, (L"PWR_Lost_IntrThread: Failed to open Gpio driver\r\n"));
		pDevice->pwlGpio = (void *)-1;
        return 0;
    }
    pDevice->pwlLost = CreateEvent(0, 0, 0, 0);
    pDevice->pwlConfig = CreateEvent(0, 0, 0, L"___________Power Lost Config");

	RETAILMSG(0, (L"PWR_Lost_IntrThread: Wait for Boot phase 2 (%d)\r\n", GetTickCount()));

	CeSetThreadPriority(GetCurrentThread(), pDevice->priority256);

	__try
	{
		pDevice->pwlLost = CreateEvent(0, 0, 0, 0);
		if(!pDevice->pwlLost)
			CloseHandle(pDevice->pwlLost);

		GPIOSetMode(pDevice->pwlGpio, pDevice->pwlPin, GPIO_DIR_INPUT | GPIO_INT_HIGH | GPIO_DEBOUNCE_ENABLE);
		debounce.gpioId			= pDevice->pwlPin;
		debounce.debounceTime	= 32; // About 1023 us
		GPIOIoControl(pDevice->pwlGpio, IOCTL_GPIO_SET_DEBOUNCE_TIME, (UCHAR *)&debounce, sizeof(debounce), 0, 0, 0, 0);

		if(!GPIOInterruptInitialize(pDevice->pwlGpio, pDevice->pwlPin, pDevice->pwlLost))
		{
			RETAILMSG(1, (L"PWR_Lost_IntrThread: GPIO InterruptInitialize failure\r\n"));
			__leave;
		}
	 
		if(!GPIOWakeEnable(pDevice->pwlGpio, pDevice->pwlPin))
		{
			RETAILMSG(1, (L"PWR_Lost_IntrThread EnableWakeup: Failed enable wakeup source\r\n"));
			__leave;
		}

		CloseHandle(CreateThread(0, 0, PWR_Lost_ConfigThread, pDevice, 0, 0));
		SetEvent(pDevice->pwlConfig);
		CloseHandle(CreateThread(0, 0, PWR_SPS_Monitor, pDevice, 0, 0));

		pDevice->pwlState = PwlUnspecified;
		while(pDevice->pwlGpio != (void *)-1)
		{
			fs_off = 0;
			code = WaitForSingleObject(pDevice->pwlLost, timeout);     
			if(pDevice->pwlGpio == (void *)-1) 
				break;

			if(!GPIOGetBit(pDevice->pwlGpio, pDevice->pwlPin))
			{
				if(INFINITE != timeout)
				{
					if(vp)
					{
						StopWatchDogTimer(vp, 0);
						CloseHandle(vp);
						vp = 0;
					}

					RETAILMSG (1, (L"PWR_Lost_IntrThread :  Power restored (%d)\r\n", GetTickCount()));

					ps = PwrDeviceUnspecified;
					if(pDevice->pwlState == PwlFSOff)
					{
						set_store_power(&ps);
						FileSystemPowerFunction(FSNOTIFY_POWER_ON);
						RETAILMSG (1, (L"PWR_Lost_IntrThread :  FS ready (%d)\r\n", GetTickCount()));
					}
					SetDevicePower(L"BKL1:", POWER_NAME, PwrDeviceUnspecified);
					SetDevicePower(L"KPD1:", POWER_NAME, PwrDeviceUnspecified);

					pDevice->pwlState = PwlUnspecified;

					RETAILMSG (1, (L"PWR_Lost_IntrThread :  DDI ready (%d)\r\n", GetTickCount()));
				}

				timeout = INFINITE;

				GPIOInterruptDone(pDevice->pwlGpio, pDevice->pwlPin);
				continue;
			}

			// pin state changed
			if(code == WAIT_OBJECT_0)
			{
				if(pDevice->pwlState == PwlUnspecified)
				{
					RETAILMSG(1, (L"PWR_Lost_IntrThread :  Power lost (shutdown DDI) (%d)\r\n", GetTickCount()));

					fs_off = 0;
					vp = CreateWatchDogTimer(L"Power Lost", (pDevice->pwlIgnore>250)?pDevice->pwlIgnore:250, 1000, WDOG_RESET_DEVICE, 0, 0);
					if(vp)
					{
						void *vpt;
						StartWatchDogTimer(vp, 0);
						vpt = CreateThread(0, 0, unsafe_shut_down, 0, 0, 0);
						CeSetThreadPriority(vpt, CeGetThreadPriority(GetCurrentThread()) - 1);
						CloseHandle(vpt);
					}
					if(!(pDevice->pwulFlags &
						(POWER_STATE_OFF | POWER_STATE_BOOT | POWER_STATE_SUSPEND | POWER_STATE_RESET)))
					{
						fs_off = 1;
						SetDevicePower(L"BKL1:", POWER_NAME, D4);
						SetDevicePower(L"KPD1:", POWER_NAME, D4);
					}
					if(vp)
					{
						StopWatchDogTimer(vp, 0);
						CloseHandle(vp);
						vp = 0;
					}

					timeout = pDevice->pwlIgnore;
					pDevice->pwlState = PwlDDIOff;
				}
				continue;
			}
			else
			{
				if(pDevice->pwlState == PwlDDIOff)
				{
					fs_off = 0;
					vp = CreateWatchDogTimer(L"Power Lost", (pDevice->pwlUSD>1500)?pDevice->pwlUSD:1500, 100, WDOG_RESET_DEVICE, 0, 0);
					if(vp)
					{
						void *vpt;
						StartWatchDogTimer(vp, 0);
						vpt = CreateThread(0, 0, unsafe_shut_down, 0, 0, 0);
						CeSetThreadPriority(vpt, CeGetThreadPriority(GetCurrentThread()) - 1);
						CloseHandle(vpt);
					}
					if(!(pDevice->pwulFlags & (POWER_STATE_OFF | POWER_STATE_BOOT | POWER_STATE_SUSPEND | POWER_STATE_RESET)))
					{
						RETAILMSG(1, (L"PWR_Lost_IntrThread :  FSNOTIFY_POWER_OFF (%d)\r\n", GetTickCount()));
						FileSystemPowerFunction(FSNOTIFY_POWER_OFF);
						ps = D4;
						set_store_power(&ps);
					}

					if(vp)
					{
						StopWatchDogTimer(vp, 0);
						CloseHandle(vp);
						vp = 0;
					}
					// WD test
					//Sleep(10000);

					timeout = pDevice->pwlUSD;
					pDevice->pwlState = PwlFSOff;

					RETAILMSG(1, (L"PWR_Lost_IntrThread :  safe state (%d)\r\n", GetTickCount()));
				}
				else //if(pDevice->pwlState == PwlFSOff)
				{
					if(vp)
					{
						StopWatchDogTimer(vp, 0);
						CloseHandle(vp);
						vp = 0;
					}
					RETAILMSG(1, (L"PWR_Lost_IntrThread :  Urgent shut down (%d)\r\n", GetTickCount()));
#if defined (BSP_DEBUG_SERIAL)
					Sleep(200); // for debugging purposes only
#endif
					KernelLibIoControl((HANDLE)KMOD_OAL, IOCTL_HAL_SHUTDOWN, 0, 0, 0, 0, 0);
				}
			}
		}
	}
	__finally
	{
		if((void *)-1 != pDevice->pwlGpio)
		{
			GPIOInterruptRelease(pDevice->pwlGpio, pDevice->pwlPin);

			SetEvent(pDevice->pwlConfig);
			Sleep(0);
			CloseHandle(pDevice->pwlConfig);
			pDevice->pwlConfig = (void *)-1;
		}
		if(pDevice->pwlLost)
			CloseHandle(pDevice->pwlLost);
		pDevice->pwlLost = 0;
	}


	return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  PKD_Deinit
//
//  Called by device manager to uninitialize device.
//
BOOL PKD_Deinit(DWORD context)
{
    PwrkeyDevice_t *pDevice = (PwrkeyDevice_t*)context;

    DEBUGMSG(ZONE_FUNCTION, (L"+PKD_Deinit(0x%08x)\r\n", context));

#ifdef BSP_TEMPSENSOR
	TempSensorDeinit(&pDevice->TempSensor);
#endif

	do
	{
		// Check if we get correct context
		if(!pDevice)
		{
			DEBUGMSG (ZONE_ERROR, (L"ERROR: PKD_Deinit: Incorrect context parameter\r\n"));
			break;
		}

		if(pDevice->pwlGpio != (void *)-1)
		{
			GPIOClose(pDevice->pwlGpio);
			pDevice->pwlGpio = (void *)-1;
			pDevice->wakeGpio = (void *)-1;
			pDevice->clkGpio = (void *)-1;
		}

		if(pDevice->hTWL)
		{
			// Disable interrupt
    		TWLInterruptDisable(pDevice->hTWL, TWL_INTR_PWRON);
			TWLWakeEnable(pDevice->hTWL, TWL_INTR_PWRON, 0);
	    
			// close T2 driver
			TWLClose(pDevice->hTWL);
		}
	    
		// Free device structure
		LocalFree(pDevice);

		DEBUGMSG(ZONE_FUNCTION, (L"-PKD_Deinit\r\n"));

		return 1;
	}while(0);

	DEBUGMSG(ZONE_FUNCTION, (L"-PKD_Deinit fail\r\n"));

	return 0;
}

//------------------------------------------------------------------------------
//
//  Function:  PKD_Init
//
//  Called by device manager to initialize device.
//
DWORD PKD_Init(LPCTSTR szContext, LPCVOID pBusContext)
{
    PwrkeyDevice_t *pDevice = NULL;

    DEBUGMSG(ZONE_FUNCTION, (L"+PKD_Init(%s, 0x%08x)\r\n", szContext, pBusContext));

	do
	{
		// Create device structure
		pDevice = (PwrkeyDevice_t *)LocalAlloc(LPTR, sizeof(PwrkeyDevice_t));
		if(!pDevice)
		{
			DEBUGMSG(ZONE_ERROR, (L"ERROR: PKD_Init: Failed allocate KDP driver structure\r\n"));
			break;
		}

		memset(pDevice, 0, sizeof(PwrkeyDevice_t));

		pDevice->clk_on = -1;
		pDevice->wake_en = -1;
		pDevice->wakeGpio = (void *)-1;
		pDevice->pwlGpio = (void *)-1;

		pDevice->wake_en = GPIO_28;
		pDevice->clk_on = GPIO_186;
		pDevice->pwlPin = GPIO_126;

		pDevice->pwlGpio = pDevice->wakeGpio = pDevice->clkGpio = GPIOOpen();

		CloseHandle(CreateThread(0, 0, PWR_Lost_IntrThread, pDevice, 0, 0));

		pDevice->pWake = CreateEvent(0, 0, 0, 0);
		GPIOSetMode(pDevice->wakeGpio, pDevice->wake_en, GPIO_DIR_INPUT | GPIO_INT_LOW_HIGH | GPIO_DEBOUNCE_ENABLE);
		if(!GPIOInterruptInitialize(pDevice->wakeGpio, pDevice->wake_en, pDevice->pWake))
		{
			RETAILMSG(1, (L"PKD_Init: Wake GPIO Interrupt Initialize failure\r\n"));
		}
		if(!GPIOWakeEnable(pDevice->wakeGpio, pDevice->wake_en))
		{
			RETAILMSG(1, (L"PKD_Init: Wake GPIO wake enable failed\r\n"));
		}
		if(!GPIOInterruptMask(pDevice->wakeGpio, pDevice->wake_en, 1))
		{
			RETAILMSG(1, (L"PKD_Init: Wake GPIO mask failed\r\n"));
		}

		  // Open T2 driver
		pDevice->hTWL = TWLOpen();
		if (pDevice->hTWL == NULL)
		{
			DEBUGMSG(ZONE_ERROR, (L"ERROR: PKD_Init: Failed open TWL bus driver\r\n"));
			break;
		}


	#ifdef BSP_TEMPSENSOR
		TempSensorInit(&pDevice->TempSensor, pDevice->hTWL);
	#endif

		// Return non-null value
		DEBUGMSG(ZONE_FUNCTION, (L"-PKD_Init\r\n"));

		return (DWORD)pDevice;
	}while(0);

	PKD_Deinit((DWORD)pDevice);

	DEBUGMSG(ZONE_FUNCTION, (L"-PKD_Init\r\n"));

    return 0;
}

//------------------------------------------------------------------------------
//
//  Function:  PKD_Open
//
//  Called by device manager to open a device for reading and/or writing.
//
DWORD PKD_Open(DWORD context, DWORD accessCode, DWORD shareMode)
{
	return context;
}

//------------------------------------------------------------------------------
//
//  Function:  PKD_Open
//
//  This function closes the device context.
//
BOOL PKD_Close(DWORD context)
{
	PwrkeyDevice_t *pDevice = (PwrkeyDevice_t*)context;

#ifdef BSP_TEMPSENSOR
    TempSensorClose(&pDevice->TempSensor);
#endif

	return 1;
}

//------------------------------------------------------------------------------
//
//  Function:  PKD_PowerUp
//
//  Called on resume of device.  Current implementation of pwrkey driver
//  will enable the pwrkey interrupts/wakeup before suspend.  Make sure the
//  pwrkey interrupts are re-disabled on resume.
//
void PKD_PowerUp(DWORD context)
{
    PwrkeyDevice_t *pDevice = (PwrkeyDevice_t*)context;

	if(!pDevice)
    {
        RETAILMSG(1, (L"%S: Incorrect context parameter\r\n", __FUNCTION__));
        return;
    }

	TWLWakeEnable(pDevice->hTWL, TWL_INTR_PWRON, 0);
	if(pDevice->wakeGpio != (void *)-1)
	{
		GPIOInterruptMask(pDevice->wakeGpio, pDevice->wake_en, 1);
	}

	if(pDevice->pwlGpio != (void *)-1)
	{
		// Temporary - edge should be defined with ready HW
		GPIOSetMode(pDevice->pwlGpio, pDevice->pwlPin, GPIO_DIR_INPUT | GPIO_INT_HIGH | GPIO_DEBOUNCE_ENABLE);
	}

	if(pDevice->clkGpio != (void *)-1)
	{
		if(pDevice->clk_on != -1)
		{
			GPIOClrBit(pDevice->clkGpio, pDevice->clk_on);
		}
	}
}

//------------------------------------------------------------------------------
//
//  Function:  PKD_PowerDown
//
//  Called on suspend of device.
//
void PKD_PowerDown(DWORD context)
{
    PwrkeyDevice_t *pDevice = (PwrkeyDevice_t*)context;

	if(!pDevice)
    {
        RETAILMSG(1, (L"ERROR: PKD_PowerDown: Incorrect context parameter\r\n"));
        return;
    }
	RETAILMSG (1, (L"KPD_PowerDown :  Enabled Wakeup \r\n"));


	// enable wake on pwron key input
	TWLWakeEnable(pDevice->hTWL, TWL_INTR_PWRON, TRUE);
	if(pDevice->wakeGpio != (void *)-1)
	{
		GPIOInterruptMask(pDevice->wakeGpio, pDevice->wake_en, 0);
	}
	if(pDevice->pwlGpio != (void *)-1)
	{
		// Temporary - edge should be defined with ready HW
		GPIOSetMode(pDevice->pwlGpio, pDevice->pwlPin, GPIO_DIR_INPUT | GPIO_INT_LOW_HIGH | GPIO_DEBOUNCE_ENABLE);
	}
	if(pDevice->clkGpio != (void *)-1)
	{
		if(pDevice->clk_on != -1)
		{
			GPIOSetBit(pDevice->clkGpio, pDevice->clk_on);
		}
	}
}

//------------------------------------------------------------------------------
//
//  Function:  PKD_IOControl
//
//  This function sends a command to a device.
//
BOOL PKD_IOControl(DWORD context, DWORD code, UCHAR *pInBuffer, DWORD inSize, UCHAR *pOutBuffer, DWORD outSize, DWORD *pOutSize)
{
    PwrkeyDevice_t *pDevice = (PwrkeyDevice_t*)context;

	do
	{
		DEBUGMSG(ZONE_FUNCTION, (L"+PKD_IOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n", context, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize));
	        
		// Check if we get correct context
		if(!pDevice)
		{
			RETAILMSG(ZONE_ERROR, (L"ERROR: PKD_IOControl: Incorrect context parameter\r\n"));
			break;
		}
		
#ifdef BSP_TEMPSENSOR
		if (code == IOCTL_TEMPSENSOR_REGISTERQUEUE	|| code == IOCTL_TEMPSENSOR_DEREGISTERQUEUE ||
			code == IOCTL_TEMPSENSOR_GETDATA		|| code == IOCTL_TEMPSENSOR_FORCEENQUEUE ||
			code == IOCTL_TEMPSENSOR_OPENCONTEXT	|| code == IOCTL_TEMPSENSOR_GETRAWDATA)
		{
			return TempSensorIOControl(&(pDevice->TempSensor), code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize);
		}
#endif
	}while(0);

	DEBUGMSG(ZONE_FUNCTION, (L"-PKD_IOControl\r\n"));

    return 0;
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

    return 1;
}

//------------------------------------------------------------------------------

