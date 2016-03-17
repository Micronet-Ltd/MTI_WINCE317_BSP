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
//  File: keypad.c
//
//  This file implements device driver for keypad. The driver isn't implemented
//  as classical keyboard driver. Instead implementation uses stream driver
//  model and it calls keybd_event to pass information to GWE subsystem.
//
#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <tps659xx.h>
#include <keypad.h>
#include <pmpolicy.h>

#include <initguid.h>
#include <twl.h>

#include "_keypad.h"
#include <keypad_user.h>
// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)
#include <keypad_ls.h>

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
//  Set debug zones names and initial setting for driver
//
#ifndef SHIP_BUILD

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
DBGPARAM dpCurSettings = {
    L"keypad", {
        L"Errors",      L"Warnings",    L"Function",    L"Info",
        L"IST",         L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    0x0003
};

#endif


//------------------------------------------------------------------------------
//  Global Variables

KEYPAD_LONGPRESS* g_pKeypadLongPress = 0;

DWORD g_KbdSilent = 0; //KEYEVENTF_SILENT 
DWORD g_bEnableSuspendKey	= 1;
DWORD g_bEnableOffKey		= 1;
UCHAR *g_keypadVK;

//------------------------------------------------------------------------------
//  Function Prototypes
static UCHAR RemapVKeyToScreenOrientation(UCHAR  ucVKey, int *piDisplayOrientation );



//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = {
    {
        L"Priority256", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, priority256),
        fieldsize(KeypadDevice_t, priority256), (VOID*)100
    }, {
        L"EnableWake", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, enableWake),
        fieldsize(KeypadDevice_t, enableWake), (VOID*)1
    }, {
        L"ClockDivider", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, clockDivider),
        fieldsize(KeypadDevice_t, clockDivider), (VOID*)5
    }, {
        L"DebounceCounter", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, debounceCount),
        fieldsize(KeypadDevice_t, debounceCount), (VOID*)4
    }, {
        L"SamplePeriod", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, samplePeriod),
        fieldsize(KeypadDevice_t, samplePeriod), (VOID*)40
    }, {
        L"FirstRepeat", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, firstRepeat),
        fieldsize(KeypadDevice_t, firstRepeat), (VOID*)500
    }, {
        L"NextRepeat", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, nextRepeat),
        fieldsize(KeypadDevice_t, nextRepeat), (VOID*)125
    }, {
        L"EnableOffKey", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, bEnableOffKey),
        fieldsize(KeypadDevice_t, bEnableOffKey), (VOID*)0
    }, {
        L"DisablePwronKey", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, bDisablePwronKey),
        fieldsize(KeypadDevice_t, bDisablePwronKey), (VOID*)0
    }, {
        L"PowerMask", PARAM_DWORD, TRUE, 
        offset(KeypadDevice_t, powerMask),
        fieldsize(KeypadDevice_t, powerMask), NULL
    }, {
        L"KeypadLedNum", PARAM_DWORD, TRUE, 
        offset(KeypadDevice_t, KpLedNum),
        fieldsize(KeypadDevice_t, KpLedNum), (VOID*)1
    }, {
        KP_LIGHT_REG_LH, PARAM_DWORD, TRUE, 
        offset(KeypadDevice_t, KpLedIntensityHigh),
        fieldsize(KeypadDevice_t, KpLedIntensityHigh), (VOID *)98
    }, {
        KP_LIGHT_REG_LN, PARAM_DWORD, TRUE, 
        offset(KeypadDevice_t, KpLedIntensityNormal),
        fieldsize(KeypadDevice_t, KpLedIntensityNormal), (VOID *)96
    }, {
        KP_LIGHT_REG_LL, PARAM_DWORD, TRUE, 
        offset(KeypadDevice_t, KpLedIntensityLow),
        fieldsize(KeypadDevice_t, KpLedIntensityLow), (VOID *)50
    }, {
        L"KeypadLightEnabled", PARAM_DWORD, TRUE, 
        offset(KeypadDevice_t, KpLightEnabled),
        fieldsize(KeypadDevice_t, KpLightEnabled), (VOID*)TRUE
   }, {
        KP_LIGHT_REG_DEPEND, PARAM_DWORD, TRUE, 
        offset(KeypadDevice_t, KpLightDepend),
        fieldsize(KeypadDevice_t, KpLightDepend), (VOID*)TRUE
   }, {
        L"PowerKeysNotify", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, KpPwrNotif),
        fieldsize(KeypadDevice_t, KpPwrNotif), (VOID*)POWER_STATE_RESET
    }, {
        L"EnableSuspendKey", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, bEnableSuspendKey),
        fieldsize(KeypadDevice_t, bEnableSuspendKey), (VOID*)0
    }
};

UINT8 g_skip_vk = 0;
//------------------------------------------------------------------------------
//
//  Function:  SendKeyPadEvent
//
//  Send keypad event
//
VOID 
SendKeyPadEvent(
    const KeypadDevice_t *pDevice,
    BYTE bVk,
    BYTE bScan,
    DWORD dwFlags,
    DWORD dwExtraInfo)
{
    USHORT index;
    UCHAR vk_extra, order, vk_remap;
	int     iDisplayOrientation = -1;

	if(bVk == g_skip_vk)
		return;
	if(	D3 == (CEDEVICE_POWER_STATE)pDevice->powerState ||
		D4 == (CEDEVICE_POWER_STATE)pDevice->powerState )
		return;

	order = KEYPAD_EXTRASEQ_ORDER_NONE; // no extra key needed
    
    // Check extra virtual key sequence table
    for (index = 0; index < g_keypadExtraSeq.count; index ++)
        {
        if (g_keypadExtraSeq.pItem[index].vk_orig == bVk)
            {
            vk_extra = g_keypadExtraSeq.pItem[index].vk_extra;
            order = g_keypadExtraSeq.pItem[index].order;
            }
        }
      
    // Check to send extra vk first  
    if (order == KEYPAD_EXTRASEQ_ORDER_EXTRAFIRST || 
        (order == KEYPAD_EXTRASEQ_ORDER_EXTRAORIG && (dwFlags & KEYEVENTF_KEYUP) == 0) )
        {
        vk_remap = RemapVKeyToScreenOrientation( vk_extra ,&iDisplayOrientation);  
        keybd_event(
            vk_remap, //vk_extra,
            0,
            dwFlags | KEYEVENTF_SILENT,
            dwExtraInfo);
        }
          
    // Send original vk
	vk_remap = RemapVKeyToScreenOrientation( bVk , &iDisplayOrientation);  
    keybd_event(
        vk_remap, //bVk,
        bScan,
        dwFlags | g_KbdSilent,
        dwExtraInfo);
      
   // Check to send extra key
   if (order == KEYPAD_EXTRASEQ_ORDER_ORIGFIRST || 
       (order == KEYPAD_EXTRASEQ_ORDER_EXTRAORIG && (dwFlags & KEYEVENTF_KEYUP)))
       {
       vk_remap = RemapVKeyToScreenOrientation( vk_extra , &iDisplayOrientation);   
       keybd_event(
           vk_remap, //vk_extra,
           0,
           dwFlags | KEYEVENTF_SILENT,
           dwExtraInfo);
       }
}

//------------------------------------------------------------------------------
//
//  Function:  PressedReleasedKeys
//
//  Find keys pressed, by comparing old and new state.
//  Send key events for all changed keys.
//
VOID
PressedReleasedKeys(
    KeypadDevice_t *pDevice,
    const DWORD vkState[],
    const DWORD vkNewState[]
    )
{
    UINT8 vk;
    int ic;

    for (ic = 0, vk = 0; ic < VK_KEYS/DWORD_BITS; ic++)
        {
        DWORD change = vkState[ic] ^ vkNewState[ic];
        if (change == 0)
            {
            vk += DWORD_BITS;
            }
        else
            {
            DWORD mask;
            for (mask = 1; mask != 0; mask <<= 1, vk++)
                {
                // Check for change
                if ((change & mask) != 0)
                    {
                    if ((vkNewState[ic] & mask) != 0)
                        {
                        RETAILMSG(1, (L" PressedReleasedKeys: Key Down: 0x%x\r\n", vk)); 
                        // Send key down event
                        if (vk != VK_OFF && vk != VK_OFF1 )
                        {
                            SendKeyPadEvent(pDevice, vk, 0, 0, 0);
                        }

						if (g_bEnableOffKey == TRUE)
							{

							switch (vk)
								{
								//case VK_TPOWER:
								case VK_OFF:
									// only disable interrupts if we are about to enter
									// a suspend state
									RETAILMSG(1, (L" PressedReleasedKeys: Notify about Shutdown 0x%02X\r\n",  vk ));
									PowerPolicyNotify(PPN_SUSPENDKEYPRESSED, 0);
									break;

								}	
							}
						if(g_bEnableSuspendKey && VK_OFF1 == vk)
						{
							RETAILMSG(1, (L" PressedReleasedKeys: Notify about Standby 0x%02X\r\n",  vk ));
							PowerPolicyNotify(PPN_SUSPENDKEYPRESSED, 1);
						}
                    }
                    else
                        {
                        DEBUGMSG(PressedReleasedKeys, 
                            (L" PressedReleasedKeys: Key Up: 0x%x\r\n", vk
                            ));
                        RETAILMSG(0,(L" PressedReleasedKeys: Key Up: 0x%x\r\n", vk));

#if 0
                        // Need to send the keydown as well as keyup for
                        // device to suspend under cebase.                          
                        if (pDevice->bEnableOffKey == TRUE && vk == VK_OFF)
                            {
                            SendKeyPadEvent(pDevice, vk, 0, 0, 0);
                            }
                        
                        // Send key down event
                        if (pDevice->bEnableOffKey != FALSE || vk != VK_OFF)
                            {
                            SendKeyPadEvent(pDevice, vk, 0, KEYEVENTF_KEYUP, 0);
                            }
#endif
						// don't send VK_OFF down/up events since it will cause the system to suspend 
						if (vk != VK_OFF && vk != VK_OFF1)
                            {
                            SendKeyPadEvent(pDevice, vk, 0, KEYEVENTF_KEYUP, 0);
                            }

                        
                        // send PowerPolicyNotify notification
                        switch (vk)
                            {

                            case VK_APPS:
                            case VK_APP1:
                            case VK_APP2:
                            case VK_APP3:
                            case VK_APP4:
                            case VK_APP5:
                            case VK_APP6:                     
                                PowerPolicyNotify(PPN_APPBUTTONPRESSED, 0);
                                break;
                            }
                        
                        }
                    }
                }
            }
        }
}


//------------------------------------------------------------------------------
//
//  Function:  SetPowerState
//
//  Sets the device power state
//
BOOL SetPowerState(KeypadDevice_t *pDevice, CEDEVICE_POWER_STATE power)
{
	UINT32 KpLedIntensity;
	static BOOL fSuspended = 0, prevSuspended = 1;

	DEBUGMSG(ZONE_FUNCTION, (L"+SetPowerState(0x%08X, 0x%08x)\r\n", pDevice, power));

	EnterCriticalSection(&pDevice->cs);
	switch(power)
	{
        case D0:
		{
			if(pDevice->KpLightDepend)
			{
				switch(pDevice->currentAmbientLight)
				{
					case AMBIENT_LIGHT_LOW:
					{
						KpLedIntensity = pDevice->KpLedIntensityLow;
						break;
					}
					case AMBIENT_LIGHT_HIGH:
					{
						KpLedIntensity = pDevice->KpLedIntensityHigh;
						break;
					}
					default:
					{
						KpLedIntensity = pDevice->KpLedIntensityNormal;
						break;
					}
				}
			}
			else
				KpLedIntensity = pDevice->KpLedIntensityLow;
			fSuspended = 0;
			break;
		}
        case D1:
        case D2:
		{
			KpLedIntensity = 100;
			fSuspended = 0;

			break;
		}
        case D3:
        case D4:
		{
			KpLedIntensity = 100;
			fSuspended = 1;
			break;
		}
        default:
            RETAILMSG(ZONE_WARN, (L"WARN: KPD::SetPowerState: Invalid power state (%d)\r\n", power));            
			LeaveCriticalSection(&pDevice->cs);
            return 0;
	}

	if(fSuspended != prevSuspended)
	{
		reset_int_mode(pDevice->hGpio, fSuspended);
		prevSuspended = fSuspended;
	}

    pDevice->powerState = power;

	RETAILMSG(0, (L"KPD::SetPowerState(D%d) -> %d\r\n", pDevice->powerState, KpLedIntensity));
	if(pDevice->m_hLed)
		TLEDSetDutyCycle(pDevice->m_hLed, KpLedIntensity);
	LeaveCriticalSection(&pDevice->cs);
    
    DEBUGMSG(ZONE_FUNCTION, (L"-SetPowerState(0x%08X, 0x%08x)\r\n", pDevice, power));
        
    return 1;
}

//------------------------------------------------------------------------------
//
//  Function:  KPD_Init
//
//  Called by device manager to initialize device.
//
DWORD KPD_Init(LPCTSTR szContext, LPCVOID pBusContext)
{
    DWORD rc = (DWORD)NULL;
    KeypadDevice_t *pDevice = NULL;
    UINT8 regval;


    DEBUGMSG(ZONE_FUNCTION, (
        L"+KPD_Init(%s, 0x%08x)\r\n", szContext, pBusContext
        ));

    // Create device structure
    pDevice = (KeypadDevice_t *)LocalAlloc(LPTR, sizeof(KeypadDevice_t));
    if (pDevice == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: KPD_Init: "
            L"Failed allocate KDP driver structure\r\n"
            ));
        goto cleanUp;
        }

    memset(pDevice, 0, sizeof(KeypadDevice_t));

    // Set cookie & initialize critical section
    pDevice->cookie = KPD_DEVICE_COOKIE;
    
    // Read device parameters
    if (GetDeviceRegistryParams(
            szContext, pDevice, dimof(s_deviceRegParams), s_deviceRegParams)
            != ERROR_SUCCESS)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: KPD_Init: "
            L"Failed read KPD driver registry parameters\r\n"
            ));
        goto cleanUp;
        }
	InitializeCriticalSection(&pDevice->csKeymap);
	g_keypadVK = (UCHAR *)g_keypadVK_Default;
    // Open parent bus
    pDevice->hTWL = TWLOpen();
    if (pDevice->hTWL == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: KPD_Init: "
            L"Failed open TWL bus driver\r\n"
            ));
        goto cleanUp;
        }

    // Set debounce delay and enable hardware mode
    regval = TWL_KBD_CTRL_KBD_ON | TWL_KBD_CTRL_NRESET | TWL_KBD_CTRL_NSOFT_MODE;
 //    regval = TWL_KBD_CTRL_KBD_ON | TWL_KBD_CTRL_NRESET | TWL_KBD_CTRL_NSOFT_MODE|TWL_KBD_CTRL_TOE_EN |TWL_KBD_CTRL_LK_EN|TWL_KBD_CTRL_RP_EN;
    TWLWriteRegs(pDevice->hTWL, TWL_KEYP_CTRL_REG, &regval, sizeof(regval));
    regval = 0x07 << 5;
	//regval = 0x00 << 5;
    TWLWriteRegs(pDevice->hTWL, TWL_LK_PTV_REG, &regval, sizeof(regval));
    regval = (UINT8)pDevice->debounceCount & 0x3F;
    TWLWriteRegs(pDevice->hTWL, TWL_KEY_DEB_REG, &regval, sizeof(regval));
  
    // Create interrupt event
    pDevice->hIntrEventKeypad = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (pDevice->hIntrEventKeypad == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: KPD_Init: "
            L"Failed create keypad interrupt event\r\n"
            ));
        goto cleanUp;
        }
	pDevice->hIntrEventPower = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (pDevice->hIntrEventPower == NULL)
		{
		DEBUGMSG(ZONE_ERROR, (L"ERROR: KPD_Init: Failed create power interrupt event\r\n"
			));
		goto cleanUp;
		}
        
    // Associate event with TWL KP interrupt
    if (!TWLInterruptInitialize(pDevice->hTWL, TWL_INTR_ITKPI, pDevice->hIntrEventKeypad))
        {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: KPD_Init: "
            L"Failed associate event with TWL KBD interrupt\r\n"
            ));
        goto cleanUp;
        }
    
    // Enable KP event
    if (!TWLInterruptMask(pDevice->hTWL, TWL_INTR_ITKPI, FALSE))
        {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: KPD_Init: "
            L"Failed associate event with TWL KBD interrupt\r\n"
            ));
        }
    // register to be wake-up enabled
    if (pDevice->enableWake != 0)
        {
			// the keypad interrupt is not wake up event in our platform but poweron event 
     //   TWLWakeEnable(pDevice->hTWL, TWL_INTR_ITKPI, TRUE);
        }
	pDevice->hLightThread = 0; //don't touch befor creation
    // Start keypad interrupt service thread
    pDevice->hIntrThreadKeypad = CreateThread(NULL, 0, KPD_IntrThread, pDevice, 0, 0);
    if (!pDevice->hIntrThreadKeypad)
        {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: KPD_Init: "
            L"Failed create keypad interrupt thread\r\n"
            ));
        goto cleanUp;
        }

	// Start power interrupt service thread
	pDevice->hIntrThreadPower = CreateThread(NULL, 0, PWRON_IntrThread, pDevice, 0,NULL);
	if (!pDevice->hIntrThreadPower)
		{
		DEBUGMSG (ZONE_ERROR, (L"ERROR: KPD_Init: "
			L"Failed create power interrupt thread\r\n"
			));
		goto cleanUp;
		}
    
    // Set thread priority
    CeSetThreadPriority(pDevice->hIntrThreadKeypad, pDevice->priority256);
	CeSetThreadPriority(pDevice->hIntrThreadPower, pDevice->priority256);

    if(pDevice->KpLightEnabled)
	{
        // Create keypress notification event
        pDevice->hKeypressEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(pDevice->hKeypressEvent == NULL)
		{
            DEBUGMSG(ZONE_ERROR, (L"ERROR: KPD_Init: Failed to create keypress event\r\n"));
            goto cleanUp;
		}
        
        // Start interrupt service thread
        pDevice->hLightThread = CreateThread(NULL, 0, KPD_LightThread, pDevice, 0,NULL);
        if(!pDevice->hLightThread)
		{
            DEBUGMSG (ZONE_ERROR, (L"ERROR: KPD_Init: Failed to create keypad light thread\r\n"));
			goto cleanUp;
		}
    
        // Set thread priority
        CeSetThreadPriority(pDevice->hLightThread, pDevice->priority256);
	}

    // Return non-null value
    rc = (DWORD)pDevice;

cleanUp:
    if (rc == 0)
        {
        KPD_Deinit((DWORD)pDevice);
        }
    DEBUGMSG(ZONE_FUNCTION, (L"-KPD_Init(rc = %d\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  KPD_Deinit
//
//  Called by device manager to uninitialize device.
//
BOOL KPD_Deinit(DWORD context)
{
    BOOL rc = FALSE;
    KeypadDevice_t *pDevice = (KeypadDevice_t*)context;


    DEBUGMSG(ZONE_FUNCTION, (L"+KPD_Deinit(0x%08x)\r\n", context));

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != KPD_DEVICE_COOKIE))
        {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: KPD_Deinit: "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    // Signal stop to threads
    pDevice->intrThreadExit = TRUE;
        
    // Close interrupt thread
    if (pDevice->hIntrThreadKeypad != NULL)
    {
        // Set event to wake it
        SetEvent(pDevice->hIntrEventKeypad);
        // Wait until thread exits
        WaitForSingleObject(pDevice->hIntrThreadKeypad, INFINITE);
        // Close handle
        CloseHandle(pDevice->hIntrThreadKeypad);
	}

	// Close interrupt thread
	if (pDevice->hIntrThreadPower != NULL)
	{
		// Set event to wake it
		SetEvent(pDevice->hIntrEventPower);
		// Wait until thread exits
		WaitForSingleObject(pDevice->hIntrThreadPower, INFINITE);
		// Close handle
		CloseHandle(pDevice->hIntrThreadPower);
	}

    // Close interrupt thread
    if(pDevice->hLightThread)
	{
        // Set event to wake it
        SetEvent(pDevice->hKeypressEvent);
        // Wait until thread exits
        WaitForSingleObject(pDevice->hLightThread, INFINITE);
        // Close handle
        CloseHandle(pDevice->hLightThread);
	}

    // Close TWL driver
    if (pDevice->hTWL != NULL)
        {
        TWLInterruptMask(pDevice->hTWL, TWL_INTR_ITKPI, TRUE);
        if (pDevice->hIntrEventKeypad != NULL)
            {
            TWLInterruptDisable(pDevice->hTWL, TWL_INTR_ITKPI);
            }        
        
        TWLClose(pDevice->hTWL);
        }

    // Close interrupt handler
    if (pDevice->hIntrEventPower != NULL) CloseHandle(pDevice->hIntrEventPower);
    if (pDevice->hIntrEventKeypad != NULL) CloseHandle(pDevice->hIntrEventKeypad);
    if(pDevice->hKeypressEvent != NULL)
		CloseHandle(pDevice->hKeypressEvent);
	DeleteCriticalSection(&pDevice->csKeymap);
    // Free device structure
    LocalFree(pDevice);

    // Done
    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-KPD_Deinit(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  KPD_Open
//
//  Called by device manager to open a device for reading and/or writing.
//
DWORD
KPD_Open(
    DWORD context, 
    DWORD accessCode, 
    DWORD shareMode
    )
{
    return context;
}

//------------------------------------------------------------------------------
//
//  Function:  KPD_Open
//
//  This function closes the device context.
//
BOOL
KPD_Close(
    DWORD context
    )
{
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  KPD_PowerUp
//
//  Called on resume of device.  Current implementation of keypad driver
//  will disable the keypad interrupts before suspend.  Make sure the
//  keypad interrupts are re-enabled on resume.
//
void
KPD_PowerUp(
    DWORD context
    )
{

}


//------------------------------------------------------------------------------
//
//  Function:  KPD_PowerDown
//
//  Called on suspend of device. Enable interrupt so it will be able to wake up from
//  the suspend. It's temporary till keypad event is a wake up event
//
void
KPD_PowerDown(
    DWORD context
    )
{
	 KeypadDevice_t *pDevice = (KeypadDevice_t*)context;
	 UINT8           regval;

	 if(pDevice->isSWMode && pDevice->enableWake )
		{
				    // Enable KP event
		  if (!TWLInterruptMask(pDevice->hTWL, TWL_INTR_ITKPI, 0))
			{
			RETAILMSG (1, (L"ERROR: KPD_PowerDown: "
				L"Failed associate event with TWL KBD interrupt\r\n"
				));
			}

		  TWLReadRegs(pDevice->hTWL, TWL_KEYP_CTRL_REG, &regval, 1);
			 regval |= TWL_KBD_CTRL_NSOFT_MODE;
			 TWLWriteRegs(pDevice->hTWL, TWL_KEYP_CTRL_REG, &regval, sizeof(regval));
			 pDevice->isSWMode = 0;

		}
}

//------------------------------------------------------------------------------
//
//  Function:  KPD_IOControl
//
//  This function sends a command to a device.
//
BOOL
KPD_IOControl(
    DWORD context, 
    DWORD code, 
    UCHAR *pInBuffer, 
    DWORD inSize, 
    UCHAR *pOutBuffer,
    DWORD outSize, 
    DWORD *pOutSize
    )
{
    BOOL rc = FALSE;
	UINT32 err = ERROR_INVALID_PARAMETER;
    KeypadDevice_t *pDevice = (KeypadDevice_t*)context;

    DEBUGMSG(ZONE_FUNCTION, (
        L"+KPD_IOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",
        context, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize
        ));
        
    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != KPD_DEVICE_COOKIE))
        {
        RETAILMSG(ZONE_ERROR, (L"ERROR: KPD_IOControl: "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }
    
    switch (code)
        {
        case IOCTL_POWER_CAPABILITIES: 
            DEBUGMSG(ZONE_INFO, (L"KPD: Received IOCTL_POWER_CAPABILITIES\r\n"));
            if (pOutBuffer && outSize >= sizeof (POWER_CAPABILITIES) && 
                pOutSize) 
                {
                    __try 
                        {
                        PPOWER_CAPABILITIES PowerCaps;
                        PowerCaps = (PPOWER_CAPABILITIES)pOutBuffer;
         
                        // Only supports D0 (permanently on) and D4(off.         
                        memset(PowerCaps, 0, sizeof(*PowerCaps));
                        PowerCaps->DeviceDx = (UCHAR)pDevice->powerMask;
                        *pOutSize = sizeof(*PowerCaps);
                        
                        rc = TRUE;
                        }
                    __except(EXCEPTION_EXECUTE_HANDLER) 
                        {
                        RETAILMSG(ZONE_ERROR, (L"exception in ioctl IOCTL_POWER_CAPABILITIES\r\n"));
                        }
                }
            break;

        // determines whether changing power state is feasible
        case IOCTL_POWER_QUERY: 
            DEBUGMSG(ZONE_INFO,(L"KPD: Received IOCTL_POWER_QUERY\r\n"));
            if (pOutBuffer && outSize >= sizeof(CEDEVICE_POWER_STATE)) 
                {
                // Return a good status on any valid query, since we are 
                // always ready to change power states (if asked for state 
                // we don't support, we move to next highest, eg D3->D4).
                __try 
                    {
                    CEDEVICE_POWER_STATE ReqDx = *(PCEDEVICE_POWER_STATE)pOutBuffer;
 
                    if (VALID_DX(ReqDx)) 
                        {
                        // This is a valid Dx state so return a good status.
                        rc = TRUE;
                        }
 
                    DEBUGMSG(ZONE_INFO, (L"KPD: IOCTL_POWER_QUERY %d\r\n"));
                    }
                __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
                    RETAILMSG(ZONE_ERROR, (L"Exception in ioctl IOCTL_POWER_QUERY\r\n"));
                    }
                }
            break;

        // requests a change from one device power state to another
        case IOCTL_POWER_SET: 
            DEBUGMSG(ZONE_INFO,(L"KPD: Received IOCTL_POWER_SET\r\n"));
            if (pOutBuffer && outSize >= sizeof(CEDEVICE_POWER_STATE)) 
                {
                __try 
                    {
                    CEDEVICE_POWER_STATE ReqDx = *(PCEDEVICE_POWER_STATE)pOutBuffer;
 
                    if (SetPowerState(pDevice, ReqDx))
                        {   
                        *(PCEDEVICE_POWER_STATE)pOutBuffer = pDevice->powerState;
                        *pOutSize = sizeof(CEDEVICE_POWER_STATE);
 
                        rc = TRUE;
                        DEBUGMSG(ZONE_INFO, (L"KPD: "
                            L"IOCTL_POWER_SET to D%u \r\n",
                            pDevice->powerState
                            ));
                        }
                    else 
                        {
                        RETAILMSG(ZONE_ERROR, (L"KPD: "
                            L"Invalid state request D%u \r\n", ReqDx
                            ));
                        }
                    }
                __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
                    RETAILMSG(ZONE_ERROR, (L"Exception in ioctl\r\n"));
                    }
            }
            break;

        // gets the current device power state
        case IOCTL_POWER_GET: 
            RETAILMSG(ZONE_INFO, (L"KPD: Received IOCTL_POWER_GET\r\n"));
            if (pOutBuffer != NULL && outSize >= sizeof(CEDEVICE_POWER_STATE)) 
                {
                __try 
                    {
                    *(PCEDEVICE_POWER_STATE)pOutBuffer = pDevice->powerState;
 
                    rc = TRUE;

                    DEBUGMSG(ZONE_INFO, (L"KPD: "
                            L"IOCTL_POWER_GET to D%u \r\n",
                            pDevice->powerState
                            ));
                    }
                __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
                    RETAILMSG(ZONE_ERROR, (L"Exception in ioctl\r\n"));
                    }
                }     
            break;
#if 0
		case IOCTL_KEYPAD_SWITCH_MAP:
			{
				__try
				{
					if(pInBuffer && inSize >= sizeof(UINT32))
					{
						int i;
						UINT32 long_press;
						UCHAR *tab;
						KEYPAD_REPEAT rep;
						UINT8 skip = 0;

						if(*pInBuffer == KEYPAD_KEYS_MAP_DEFAULT)
						{
							long_press = KEYPAD_LONG_PRESS_DEFAULT;
							tab = (UCHAR *)g_keypadVK_Default;
							rep.count = def_rep;
							rep.pItem = (KEYPAD_REPEAT_ITEM *)g_repeatItems_Default;
						}
						else if(*pInBuffer == KEYPAD_KEYS_MAP_EXT)
						{
							//long_press = KEYPAD_LONG_PRESS_EXT;
							long_press = KEYPAD_LONG_PRESS_DEFAULT;
							//skip = VK_ESCAPE;
							//tab = (UCHAR *)g_keypadVK_Ext;
							tab = (UCHAR *)g_keypadVK_Default;
							rep.count = ext_rep;
							//rep.pItem = (KEYPAD_REPEAT_ITEM *)g_repeatItems_Ext;
							rep.pItem = (KEYPAD_REPEAT_ITEM *)g_repeatItems_Default;
						}
						else
							break;

						EnterCriticalSection(&pDevice->csKeymap);
						g_keypadVK = tab;
						g_keypadRepeat.count = rep.count;
						g_keypadRepeat.pItem = rep.pItem;
						g_skip_vk = skip;
						for(i = 0; i < g_pKeypadLongPress->count; i++)
						{
							g_longPressItems[i].delay = long_press;
						}
						LeaveCriticalSection(&pDevice->csKeymap);
						
						rc = 1;
						err = 0;
					}
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
					LeaveCriticalSection(&pDevice->csKeymap);
					rc = 0;
					err = ERROR_EXCEPTION_IN_SERVICE;
				}
				break;
			}
#endif
        }

cleanUp:
	if(!rc)
		SetLastError(err);
    DEBUGMSG(ZONE_FUNCTION, (L"-KPD_IOControl(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  KPD_LightThread
//
//  This turns kepad light on/off.
//
DWORD KPD_LightThread(VOID *pContext)
{
    KeypadDevice_t *pDevice = (KeypadDevice_t*)pContext;
    DWORD			Event;
	INT32			adjust = 0;
	UINT32			KpLedIntensity = pDevice->KpLedIntensityLow;
	UINT32			prevKpLedIntensity = 0;
	HANDLE			hMSGQ, ev[3] = {0};
    MSGQUEUEOPTIONS msgOptions;
	TCHAR QueueName[64] = {0};

    if(!pDevice)
	{
        RETAILMSG(1, (L" KPD_LightThread: Invalid context passed to thread routine.\r\n"));
        ASSERT( FALSE );
        return 0;
	}

    InitializeCriticalSection(&pDevice->cs);

    // Open MADC device driver
    pDevice->m_hLed = TLEDOpen();
    if(!pDevice->m_hLed)
	{
        RETAILMSG(1, (L" KPD_LightThread: Failed opening TLED driver\r\n"));
        ASSERT( FALSE );
        return 0;
	}

	if(!TLEDSetChannel(pDevice->m_hLed, pDevice->KpLedNum))
	{
        RETAILMSG(1, (L" KPD_LightThread: Failed select TLED channel\r\n"));
        ASSERT( FALSE );
		TLEDClose(pDevice->m_hLed);
        return 0;
	}
	TLEDSetDutyCycle(pDevice->m_hLed, KpLedIntensity);
	prevKpLedIntensity = KpLedIntensity;

    while(!(pDevice->m_hLight = LightSenseOpen(QueueName, sizeof(QueueName)) ) )//wait for ls driver
	{
		Sleep(100);
        RETAILMSG(0, (L" KPD_LightThread: Failed opening Light sense driver\r\n"));
	}

	memset(&msgOptions, 0, sizeof(msgOptions));
	msgOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
	msgOptions.dwFlags = 0;
	msgOptions.cbMaxMessage = sizeof(AMBIENT_LIGHT);
	msgOptions.bReadAccess = 1;

	hMSGQ = CreateMsgQueue(QueueName, &msgOptions);
	
	ev[0] = pDevice->hKeypressEvent;
	ev[1] = hMSGQ;
	ev[2] = CreateEvent(0, 0, 0, KP_LEDS_CONFIG);
    if(!ev[2])
	{
        DEBUGMSG(ZONE_ERROR, (L"ERROR: KPD_LightThread: leds config event failure\r\n"));
	}
	// TODO: Keylight duty cycle configuration
	// Control panel
	// Event config update
	// Update variables

	pDevice->currentAmbientLight = AMBIENT_LIGHT_UNSPEC;

	// Loop until we are not stopped...
    while(!pDevice->intrThreadExit)
	{
		UINT32 rx, f, rc;
        // Monitor keypress events while maintaining illuminated light
        // Adjust LED intensity based on ambient light
        // We must do sensing and adjustment in keypress loop to ensure fast response time
		if(adjust)
		{
			EnterCriticalSection(&pDevice->cs);
			switch(pDevice->currentAmbientLight)
			{
				case AMBIENT_LIGHT_LOW:
				{
					KpLedIntensity = pDevice->KpLedIntensityLow;
					break;
				}
				case AMBIENT_LIGHT_HIGH:
				{
					KpLedIntensity = pDevice->KpLedIntensityHigh;
					break;
				}
				default:
				{
					KpLedIntensity = pDevice->KpLedIntensityNormal;
					break;
				}
			}

			// Turn ON Keylight
			if(prevKpLedIntensity != KpLedIntensity)
			{
				RETAILMSG(0, (L"KPD:: Adjust LED intensivity(D%d) -> %d\r\n", pDevice->powerState, 100 - KpLedIntensity));
				prevKpLedIntensity = KpLedIntensity;
				TLEDSetDutyCycle(pDevice->m_hLed, KpLedIntensity);
			}
			adjust = 0;
			LeaveCriticalSection(&pDevice->cs);
		}

		Event = WaitForMultipleObjects(sizeof(ev)/sizeof(ev[0]), ev, 0, INFINITE);

		if(Event == WAIT_OBJECT_0)
		{
			adjust = 1;
		}
		else if(Event == (WAIT_OBJECT_0 + 1))
		{
			// Read ambient light
			EnterCriticalSection(&pDevice->cs);
			rc = ReadMsgQueue(ev[1], &pDevice->currentAmbientLight, sizeof(pDevice->currentAmbientLight), &rx, 0, &f);

			RETAILMSG(0, (L"KPD:: Ambient lightness is %s\r\n",
						(pDevice->currentAmbientLight == AMBIENT_LIGHT_HIGH)?L"high":
						(pDevice->currentAmbientLight == AMBIENT_LIGHT_LOW)?L"low":L"normal"));

			switch(pDevice->powerState)
			{
				case D0:
				{
					if( pDevice->KpLightDepend )
						adjust = 1;
					break;
				}
				case D1:
				case D2:
				case D3:
				case D4:
				default:
				{
					adjust = 0;
					break;
				}
			}
			LeaveCriticalSection(&pDevice->cs);
		}
		else if(Event == (WAIT_OBJECT_0 + 2))
		{
			EnterCriticalSection(&pDevice->cs);
			
			   // Read device parameters
//			if( ERROR_SUCCESS == GetDeviceRegistryParams( KP_REG_KEY, pDevice, dimof(s_deviceRegParams) - PARAMS_BEFORE_KEYPAD_LED, s_deviceRegParams + PARAMS_BEFORE_KEYPAD_LED ) )
			if( ERROR_SUCCESS == GetDeviceRegistryParams(KP_REG_KEY, pDevice, dimof(s_deviceRegParams), s_deviceRegParams) )
			{
				if(D0 == pDevice->powerState)
					adjust = 1;
			}
			else
			{
				DEBUGMSG(ZONE_ERROR, (L"ERROR: KPD: "L"Failed read KPD driver registry parameters\r\n" ));
	        }
			LeaveCriticalSection(&pDevice->cs);
		}
		else
			break;
	}

    // Close thread's drivers
	if(hMSGQ)
		CloseMsgQueue(hMSGQ);
	hMSGQ = 0;

    if(pDevice->m_hLed)
		TLEDClose(pDevice->m_hLed);
    pDevice->m_hLed = 0;
    if(pDevice->m_hLight)
		LightSenseClose(pDevice->m_hLight);
    pDevice->m_hLight = 0;

	if( ev[2] )//registry changed event
		CloseHandle(ev[2]);

	DeleteCriticalSection(&pDevice->cs);

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  PhysicalStateToVirtualState
//
//  Convert physical state to virtual keys state
//
static VOID PhysicalStateToVirtualState(const UINT8 matrix[MATRIX_SIZE_EXT], DWORD vkNewState[], BOOL *pKeyDown)
{
    BOOL keyDown = FALSE;
    USHORT state;
    ULONG ik, ix, row, column;
   

    for (row = 0, ik = 0; row < KEYPAD_ROWS; row++)
        {

        // Get matrix state        
        ix = row;
        state = matrix[ix] & 0xFF;

        // If no-key is pressed continue with new rows
        if (state == 0) 
            {
            ik += KEYPAD_COLUMNS;
            continue;
            }
 

        for (column = 0; column < KEYPAD_COLUMNS; column++, ik++)
            {
            if ((state & (1 << column)) != 0)
                {
                // g_keypadVK is defined by the platform
                UINT8 vk = g_keypadVK[ik];
				if (vk)
					{
					vkNewState[vk >> 5] |= 1 << (vk & 0x1F);
					RETAILMSG(0, (L"vk %x, ik %x, ind %x val %x \r\n",vk, ik, (vk >> 5), vkNewState[vk >> 5]));
					keyDown = TRUE;
					}
                }
            }
        }

    if (keyDown && (pKeyDown != NULL)) *pKeyDown = TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  VirtualKeyRemap
//
//  This function remaps multiple virtual key to final virtual key.
//
static VOID VirtualKeyRemap(DWORD time, BOOL *pKeyDown, KeypadRemapState_t *pRemapState, DWORD vkNewState[])
{
    BOOL keyDown = FALSE;
    int ix;
    
    for (ix = 0; ix < g_keypadRemap.count; ix++)
        {
        const KEYPAD_REMAP_ITEM *pItem = &g_keypadRemap.pItem[ix];
        KeypadRemapState_t *pState = &pRemapState[ix];
        DWORD state = 0;
        USHORT down = 0;
        UINT8 vk = 0;

        // Count number of keys down & save down/up state
        int ik;
        for (ik = 0; ik < pItem->keys; ik++)
            {
            vk = pItem->pVKeys[ik];
            if ((vkNewState[vk >> 5] & (1 << (vk & 0x1F))) != 0)
                {
                state |= 1 << ik;
                down++;
                }
            }

        // Depending on number of keys down
        if (down >= pItem->keys && pItem->keys > 1)
            {
            // Clear all mapping keys
            for (ik = 0; ik < pItem->keys; ik++)
                {
                vk = pItem->pVKeys[ik];
                vkNewState[vk >> 5] &= ~(1 << (vk & 0x1F));
                }
            // All keys are down set final key
            vk = pItem->vkey;
            vkNewState[vk >> 5] |= 1 << (vk & 0x1F);
            DEBUGMSG(ZONE_IST, (L" KPD_IntrThread: "
                L"Mapped vkey: 0x%x\r\n", vk
                ));
            RETAILMSG(0, (L" KPD_IntrThread: Mapped vkey: 0x%x\r\n", vk));
            // Clear remap pending flag
            pState->pending = FALSE;
            // Set remap processing flag
            pState->remapped = TRUE;
            }
        else if ( down > 0 )
            {
            // If already remapping or remapping is not pending
            // or pending time expired
            if  ( !pState->remapped &&  
                 (!pState->pending || (INT32)( time - pState->time ) < 0 ) )
                {                
                // If we are not pending and not already remapping, start
                if (!pState->pending && !pState->remapped)
                    {
                    pState->pending = TRUE;
                    pState->time = time + pItem->delay;
                    }
                // Clear all mapping keys
                for (ik = 0; ik < pItem->keys; ik++)
                    {
                    vk = pItem->pVKeys[ik];
                    vkNewState[vk >> 5] &= ~( 1 << ( vk & 0x1F ) );
                    }
                }
            else if ( pState->remapped || 
                     (pItem->keys == 1 && (INT32)( time - pState->time ) >= 0) )
                {
                // This is press and hold key
                DEBUGMSG(ZONE_IST, (L" KPD_IntrThread: "
                    L"Mapped press and hold vkey: 0x%x\r\n", vk
                    ));
                RETAILMSG(0, (L" KPD_IntrThread: Mapped press and hold vkey: 0x%x\r\n", vk));
                    
                // Clear all mapping keys
                for (ik = 0; ik < pItem->keys; ik++)
                    {
                    vk = pItem->pVKeys[ik];
                    vkNewState[vk >> 5] &= ~( 1 << ( vk & 0x1F ) );
                    }
                    
                vk = pItem->vkey;
                vkNewState[vk >> 5] |= 1 << (vk & 0x1F);
                keyDown = TRUE;
                pState->pending = FALSE;
                pState->remapped = TRUE;
                }
            }
        else
            {            
            // All keys are up, if remapping was pending set keys
            if ( pState->pending )
                {            
                for (ik = 0; ik < pItem->keys; ik++)
                    {
                    if ( ( pState->state & ( 1 << ik ) ) != 0 )
                        {                        
                        vk = pItem->pVKeys[ik];
                        vkNewState[vk >> 5] |= 1 << (vk & 0x1F);
                        keyDown = TRUE;
                        }
                    }
                pState->pending = FALSE;
                }
            pState->remapped = FALSE;
            }
        // Save key state
        pState->state = state;
        }

    // Set output variable
    if (keyDown && (pKeyDown != NULL)) *pKeyDown = TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  RemapVKeyToScreenOrientation
//
//  Map the key pressed according to the current screen rotation.
//
static UCHAR
RemapVKeyToScreenOrientation(
    UCHAR   ucVKey,
    int     *piDisplayOrientation
    )
{
    // Arrow key translation for rotate 0, 90, 180 and 270. 
    static const UCHAR   ucOrientationDMDO_0VKTable[4]   = {VK_TUP,    VK_TRIGHT, VK_TDOWN,  VK_TLEFT };
    static const UCHAR   ucOrientationDMDO_90VKTable[4]  = {VK_TRIGHT, VK_TDOWN,  VK_TLEFT,  VK_TUP   };
    static const UCHAR   ucOrientationDMDO_180VKTable[4] = {VK_TDOWN,  VK_TLEFT,  VK_TUP,    VK_TRIGHT};
    static const UCHAR   ucOrientationDMDO_270VKTable[4] = {VK_LEFT,   VK_TUP,    VK_TRIGHT, VK_TDOWN };

    UCHAR   ucRetVal = ucVKey;

    // if the input key is VK_TUP, VK_TRIGHT, VK_TDOWN, or VK_TLEFT, we might need to translate it
    // according to the screen rotation.
    if ( (ucVKey == VK_TUP) || (ucVKey == VK_TRIGHT) || (ucVKey == VK_TDOWN) || (ucVKey == VK_TLEFT) )
    {
        int     iVKeyIndex;

        // If we haven't determine the screen orientation, we need to figure it out first.
        if( (*piDisplayOrientation == -1) && IsAPIReady(SH_GDI)) 
        {
            DEVMODE devMode;
            
            //  Get current screen orientation
            devMode.dmSize = sizeof(devMode);
            devMode.dmFields = DM_DISPLAYORIENTATION;
            devMode.dmDisplayOrientation = DMDO_0;
            if (ChangeDisplaySettingsEx(NULL, &devMode, NULL, CDS_TEST, NULL) == DISP_CHANGE_SUCCESSFUL)
            {
                *piDisplayOrientation = devMode.dmDisplayOrientation;
            }
        }

        // translate VK_XXX to index.
        switch (ucVKey)
        {
            case VK_TUP:
                iVKeyIndex = 0;
                break;
            case VK_TRIGHT:
                iVKeyIndex = 1;
                break;
            case VK_TDOWN:
                iVKeyIndex = 2;
                break;
            case VK_TLEFT:
                iVKeyIndex = 3;
                break;
        }

        // Transalte VK_XXX according to the screen rotation. No translation is needed
        // if the screen is in DMD0 mode or we can't determine the screen orientation. 
        switch (*piDisplayOrientation)
        {
            case DMDO_90:
                ucRetVal = ucOrientationDMDO_90VKTable[iVKeyIndex];
                break;
            case DMDO_180:
                ucRetVal = ucOrientationDMDO_180VKTable[iVKeyIndex];
                break;
            case DMDO_270:
                ucRetVal = ucOrientationDMDO_270VKTable[iVKeyIndex];
                break;
           case DMDO_0:
           default:
                break;
        }
    }

    return ucRetVal;
}

//------------------------------------------------------------------------------
//
//  Function:  AutoRepeat
//
//  This function sends KeyEvents for auto-repeat keys.
//
static VOID
AutoRepeat(
    const KeypadDevice_t *pDevice,
    const DWORD vkNewState[],
    DWORD time,
    KeypadRepeatState_t *pRepeatState
    )
{
    ULONG ix;

    for (ix = 0; ix < g_keypadRepeat.count; ix++)
        {
        const KEYPAD_REPEAT_ITEM *pItem = &g_keypadRepeat.pItem[ix];
        KeypadRepeatState_t *pState = &pRepeatState[ix];
        DWORD delay;
        BOOL blockRepeat = FALSE;
        UINT8 vkBlock;
        UINT8 vk = pItem->vkey;

        if ((vkNewState[vk >> 5] & (1 << (vk & 0x1F))) != 0)
            {
            if (!pState->pending)
                {
                // Key was just pressed
                delay = pItem->firstDelay;
                if (delay == 0) delay = pDevice->firstRepeat;
                pState->time = time + delay;
                pState->pending = TRUE;
                pState->blocked = FALSE;
                }
            else if (((INT32)(time - pState->time)) >= 0)
                {
                // Check if any blocking keys are pressed
                const KEYPAD_REPEAT_BLOCK *pBlock = pItem->pBlock;
                if (pBlock != 0)
                    {
                    int ik;
                    for ( ik = 0; ik < pBlock->count; ik++ )
                        {
                        vkBlock = pBlock->pVKey[ik];
                        if ((vkNewState[vkBlock >> 5] &
                               (1 << (vkBlock & 0x1F))) != 0)
                            {
                            pState->blocked = TRUE;
                            DEBUGMSG(ZONE_IST, (L" KPD_IntrThread: "
                                L"Block repeat: 0x%x bcause of 0x%x\r\n",
                                vk, vkBlock
                                ));
                            break;
                            }
                        }
                    }
                
                // Repeat if not blocked
                if (!pState->blocked)
                    {
                    RETAILMSG(1, (L" KPD_IntrThread: Key Repeat: 0x%x\r\n", vk));
                    SendKeyPadEvent(pDevice, vk, 0, pItem->silent ? KEYEVENTF_SILENT : 0, 0);
                    }
                // Set time for next repeat
                delay = pItem->nextDelay;
                if (delay == 0) delay = pDevice->nextRepeat;
                pState->time = time + delay;
                }
            }
        else
            {
            pState->pending = FALSE;
            pState->blocked = FALSE;
            }
        }
}


//------------------------------------------------------------------------------
//
//  Function:  LongPressRemap
//
//  This function defines new (additional)  virtual key as a result of long key pressing.
//
static VOID
LongPressRemap(
	DWORD time,
    DWORD vkNewState[],
    KeypadLongPressState_t *pLongPressState
    )
{
    ULONG ix;

	// this function is called just in the case Off key is enable
	
	for (ix = 0; ix < g_pKeypadLongPress->count; ix++)
	{
		const KEYPAD_LONGPRESS_ITEM *pItem = &g_pKeypadLongPress->pItem[ix];
		KeypadLongPressState_t *pState = &pLongPressState[ix];
		DWORD delay;
		//BOOL blockRepeat = FALSE;
	   // UINT8 vkBlock;
		UINT8 vk = pItem->vkey;

		if ((vkNewState[vk >> 5] & (1 << (vk & 0x1F))) != 0)
		{
			RETAILMSG(0, (L" LongPressRemap: vk: 0x%x\r\n", vk));
			if (!pState->pending)
				{
				// Key was just pressed
				delay = pItem->delay;
				pState->time = time + delay;
				pState->pending = TRUE;
				 // clear final key
				vk = pItem->vremapkey;
				vkNewState[vk >> 5] &= ~( 1 << ( vk & 0x1F ) );
				//pState->blocked = FALSE;
				}
			else if (((INT32)(time - pState->time)) >= 0)
				{
					vk = pItem->vremapkey;
					// set final key
					vkNewState[vk >> 5] |= 1 << (vk & 0x1F);
					RETAILMSG(1, (L" LongPressRemap: new vk: 0x%x\r\n", vk));
					pState->pending = FALSE;
					
				}
		 }
		else
		{
			pState->pending = FALSE;
			// clear the final key
			vk = pItem->vremapkey;
			vkNewState[vk >> 5] &= ~( 1 << ( vk & 0x1F ) );
		}
	}

          
}

//------------------------------------------------------------------------------
//
//  Function:  KPD_InterruptThread
//
//  This function acts as the IST for the keypad.
//
DWORD KPD_IntrThread(VOID *pContext)
{
    KeypadDevice_t *pDevice = (KeypadDevice_t*)pContext;
    KeypadRemapState_t *pRemapState = NULL;
    KeypadRepeatState_t *pRepeatState = NULL;
	KeypadLongPressState_t *pLongPressRemap = NULL;
    UINT8 matrix[MATRIX_SIZE_EXT];
    DWORD vkState[VK_KEYS/DWORD_BITS];
    DWORD vkNewState[VK_KEYS/DWORD_BITS];
    DWORD timeout,nCol,i;
	UINT8 key =0;
//	DWORD isSWMode = 0;
	UINT8 regval,bit;

    // Init data
    memset(vkState, 0, sizeof(vkState));
    memset(vkNewState, 0, sizeof(vkNewState));

    // Initialize remap informations
    if (g_keypadRemap.count > 0)
        {
        // Allocate state structure for remap, zero initialized
        pRemapState = LocalAlloc(
            LPTR, g_keypadRemap.count * sizeof(KeypadRemapState_t)
            );
        if (pRemapState == NULL)
            {
            DEBUGMSG(ZONE_ERROR, (L" KPD_IntrThread: "
                L"Failed allocate memory for virtual key remap\r\n"
                ));
            goto cleanUp;
            }
        }

    // Initialize repeat informations
    if (g_keypadRepeat.count > 0)
        {
        // Allocate state structure for repeat, zero initialized
        pRepeatState = LocalAlloc(
            LPTR, g_keypadRepeat.count * sizeof(KeypadRepeatState_t)
            );
        if (pRepeatState == NULL)
            {
            DEBUGMSG(ZONE_ERROR, (L" KPD_IntrThread: "
                L"Failed allocate memory for virtual key auto repeat\r\n"
                ));
            goto cleanUp;
            }
        }

	   // Initialize remap informations
	g_pKeypadLongPress =	
		(pDevice->bEnableOffKey && pDevice->bEnableSuspendKey)	?	&g_keypadLongPressAllEnable		: 
		pDevice->bEnableOffKey									?	&g_keypadLongPressOffEnable		:
		pDevice->bEnableSuspendKey								?	&g_keypadLongPressSuspendEnable	: 
																	&g_keypadLongPressOffDisable;
	g_bEnableSuspendKey	= pDevice->bEnableSuspendKey;
	g_bEnableOffKey		= pDevice->bEnableOffKey;

    if (g_pKeypadLongPress->count > 0)
        {
        // Allocate state structure for remap, zero initialized
        pLongPressRemap = LocalAlloc(
            LPTR, g_pKeypadLongPress->count * sizeof(KeypadLongPressState_t)
            );
        if (pLongPressRemap == NULL)
            {
            DEBUGMSG(ZONE_ERROR, (L" KPD_IntrThread: "
                L"Failed allocate memory for virtual key long press remap\r\n"
                ));
            goto cleanUp;
            }
        }

    // Set delay to sample period
    timeout = pDevice->samplePeriod;

    // Loop until we are not stopped...
    while (!pDevice->intrThreadExit)
    {
        DWORD time;
        BOOL keyDown = FALSE;

        // Wait for event
		DWORD dwEventId = WaitForSingleObject(pDevice->hIntrEventKeypad, timeout);
        if(pDevice->intrThreadExit)
			break;

		EnterCriticalSection(&pDevice->csKeymap);

		memset (matrix,0,sizeof(matrix));

		// read MATRIX_SIZE amount of rows...
		if(!pDevice->isSWMode)
		{
			 TWLReadRegs(pDevice->hTWL, TWL_FULL_CODE_7_0, matrix, MATRIX_SIZE);
		}
		else
		{
			 //  Drive each column and read row result to get key press
			for(nCol = 0; nCol < KEYPAD_COLUMNS; nCol++)
			{
			
				regval = ~(1 << nCol);
				TWLWriteRegs(pDevice->hTWL, TWL_KBC_REG, &regval, sizeof(regval));
			
				TWLReadRegs(pDevice->hTWL, TWL_KBR_REG, &key,1);
	
				//  0 indicates a key press, so invert value 
				key = ~key;
				//RETAILMSG(1, (L" key = %x", key));
				for ( i  =0 ; i < KEYPAD_ROWS ; i++)
				{
					bit = (key>>i)&0x01;
					matrix[i]|= (bit<<nCol);
				}
			}
		}
       
        // Convert physical state to virtual keys state
        PhysicalStateToVirtualState(matrix, vkNewState, &keyDown);
		RETAILMSG(0, (L"KPD_IntrThread: keyDown %x\r\n", keyDown));

        time = GetTickCount();

        // Remap multi virtual keys to final virtual key
        VirtualKeyRemap(time, &keyDown, pRemapState, vkNewState);
        LongPressRemap(time,vkNewState,pLongPressRemap);
		
        PressedReleasedKeys(pDevice, vkState, vkNewState);
        AutoRepeat(pDevice, vkNewState, time, pRepeatState);

#ifdef KEYPAD_LED
        if( pDevice->KpLightEnabled && keyDown )
        {
            // Signal keypad light thread
            SetEvent(pDevice->hKeypressEvent);
        }
#endif
        //--------------------------------------------------------------
        // Prepare for next run
        //--------------------------------------------------------------

        // New state become old
        memcpy(vkState, vkNewState, sizeof(vkState));
        // Get new state for virtual key table
        memset(vkNewState, 0, sizeof(vkNewState));

        // Set timeout period depending on data state
        timeout = keyDown ? pDevice->samplePeriod : INFINITE;

		// anna - test

		// Vladimir
		// What is it means? Disable/enable interrupts ?
		if(!pDevice->isSWMode && keyDown)
		{
			TWLInterruptMask(pDevice->hTWL, TWL_INTR_ITKPI, 1);
			
	
			 TWLReadRegs(pDevice->hTWL, TWL_KEYP_CTRL_REG, &regval, 1);
			 regval &= ~TWL_KBD_CTRL_NSOFT_MODE;
			 TWLWriteRegs(pDevice->hTWL, TWL_KEYP_CTRL_REG, &regval, sizeof(regval));
			 pDevice->isSWMode = 1;
			
		}
		else if(pDevice->isSWMode && !keyDown)
		{
			    // Enable KP event
		  if (!TWLInterruptMask(pDevice->hTWL, TWL_INTR_ITKPI, 0))
			{
			RETAILMSG (1, (L"ERROR: KPD_IntrThread: "
				L"Failed associate event with TWL KBD interrupt\r\n"
				));
			}

			TWLReadRegs(pDevice->hTWL, TWL_KEYP_CTRL_REG, &regval, 1);
			 regval |= TWL_KBD_CTRL_NSOFT_MODE;
			 TWLWriteRegs(pDevice->hTWL, TWL_KEYP_CTRL_REG, &regval, sizeof(regval));
			 pDevice->isSWMode = 0;
		}
        // Interrupt is done
		LeaveCriticalSection(&pDevice->csKeymap);
	}

cleanUp:
    if ( pRemapState != NULL )
        {
        LocalFree(pRemapState);
        }

    if ( pRepeatState != NULL )
        {
        LocalFree(pRepeatState);
        }
    return ERROR_SUCCESS;
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

//------------------------------------------------------------------------------

