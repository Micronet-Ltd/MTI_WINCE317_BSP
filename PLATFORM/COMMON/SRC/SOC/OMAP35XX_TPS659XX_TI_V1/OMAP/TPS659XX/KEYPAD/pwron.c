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
// by Vladimir Zatulovsky
//
//
//  File: pwron.c
//
//  Handles the PWRON signal of T2 and translates it into a keypress.
//
#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <tps659xx.h>
#include <keypad.h>
#include <pmpolicy.h>

#include <twl.h>
#include "_keypad.h"

#include <initguid.h>
#include <gpio.h>

extern DWORD g_KbdSilent;// = 0; //KEYEVENTF_SILENT  

void reset_int_mode(HANDLE hGpio, BOOL fSuspend)
{
	UINT mod;
	if( hGpio )
	{
		if( fSuspend )
			mod = GPIO_DIR_INPUT | GPIO_INT_LOW_HIGH | GPIO_DEBOUNCE_ENABLE;
		else
			mod = GPIO_DIR_INPUT | GPIO_INT_HIGH | GPIO_DEBOUNCE_ENABLE;

		GPIOSetMode(hGpio, GPIO_167, mod );
	}
}

DWORD PWRON_IntrThread(VOID *pContext)
{
    KeypadDevice_t *pDevice = (KeypadDevice_t*)pContext;
	DWORD		code = WAIT_OBJECT_0;
	HANDLE		hGpio	= 0;
	UINT		GpioId	= GPIO_167;
	UINT32		timeout = INFINITE;
	UINT32		SilentFlag = 0;

	IOCTL_GPIO_SET_DEBOUNCE_TIME_IN debounce;
    
	if(!(hGpio = GPIOOpen()))
    {
		RETAILMSG(1, (L"%S: Failed to open Gpio driver\r\n", __FUNCTION__));
        return 0;
    }
	
	pDevice->hGpio = hGpio;

	GPIOSetMode(hGpio, GpioId, GPIO_DIR_INPUT | GPIO_INT_HIGH | GPIO_DEBOUNCE_ENABLE);
    debounce.gpioId			= GpioId;
    debounce.debounceTime	= 10;
    GPIOIoControl(hGpio, IOCTL_GPIO_SET_DEBOUNCE_TIME, (UCHAR*)&debounce, sizeof(debounce), NULL, 0, NULL, NULL);

	RETAILMSG(1, (L"%S: Reset req handler (%d)\r\n", __FUNCTION__, GpioId));
	
	if(!GPIOInterruptInitialize(hGpio, GpioId, pDevice->hIntrEventPower))
	{
		RETAILMSG(1, (L"%S: Interrupt Initialize failure (%d)\r\n", __FUNCTION__, GpioId));
		GPIOClose(hGpio);
		pDevice->hGpio = 0;
		return 0;
	}
 
	if(!GPIOWakeEnable(hGpio, GpioId))
    {
		RETAILMSG(1, (L"%S:  Failed enable pwrnotify keys as wakeup source (%d)\r\n", __FUNCTION__, GpioId));
		GPIOInterruptRelease(hGpio, GpioId);
		GPIOClose(hGpio);
		pDevice->hGpio = 0;
		return 0;
	}

	do
	{
		if(pDevice->intrThreadExit)
		{
			RETAILMSG(1, (L"%S: Quit req (%d)\r\n", __FUNCTION__, GpioId));
			break;
		}

		if(!GPIOGetBit(hGpio, GpioId))
		{
			timeout = INFINITE;

			GPIOInterruptDone(hGpio, GpioId);
			g_KbdSilent = SilentFlag;  
			RETAILMSG(1, (L"%S: has no reset req wait for req(%d)\r\n", __FUNCTION__, GpioId));
		}
		else
		{
			// key state
			if(code == WAIT_OBJECT_0)
			{
				RETAILMSG(1, (L"%S: Reset req check it 1 sec (%d)\r\n", __FUNCTION__, GpioId));
				SilentFlag	= g_KbdSilent;
				g_KbdSilent |= KEYEVENTF_SILENT;
				timeout = 1000;
			}
			else
			{
				RETAILMSG(1, (L"%S: Handle reset req (%d, %d)\r\n", __FUNCTION__, GpioId, pDevice->KpPwrNotif));
				if( POWER_STATE_RESET == pDevice->KpPwrNotif )
					SetSystemPowerState(0, POWER_STATE_RESET, POWER_FORCE);
				else
					SetSystemPowerState(0, POWER_STATE_OFF, POWER_FORCE);
			}
		}
        code = WaitForSingleObject(pDevice->hIntrEventPower, timeout);     
	}while(!pDevice->intrThreadExit);


	if(hGpio)
	{
		GPIOInterruptRelease(hGpio, GpioId);
		GPIOClose(hGpio);
		pDevice->hGpio = 0;
	}

	RETAILMSG(1, (L"%S: Leave (%d)\r\n", __FUNCTION__, GpioId));

	return ERROR_SUCCESS;
}
