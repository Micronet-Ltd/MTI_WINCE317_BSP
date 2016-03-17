// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

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
//  File: gpio_backlight.cpp
//

#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <oal.h>
#include <ceddk.h>
#include <ceddkex.h>

#include <omap35xx.h>

#include <initguid.h>
#include <gpio.h>
#include "..\backlight_class.h"
#include "..\MDD\bkli.h"

#define GPIO_NOT_USED   0xFFFFFFFF
//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = {
    {
    L"Gpio", PARAM_DWORD, FALSE, offset(GPIO_Backlight, m_gpioId),
    fieldsize(GPIO_Backlight, m_gpioId), (PVOID)GPIO_NOT_USED
    },{
    L"PowerMask", PARAM_DWORD, TRUE, offset(GPIO_Backlight, m_powerMask),
    fieldsize(GPIO_Backlight, m_powerMask), NULL
    }
};


//------------------------------------------------------------------------------
//
//  Function:  Initialize
//
//  class initializer
//
BOOL
GPIO_Backlight::Initialize(
    LPCTSTR szContext
    )
{
    DEBUGMSG(ZONE_FUNCTION, (
        L"+GPIO_Backlight::Initialize(%s)\r\n", szContext
        ));

    BOOL rc = FALSE;

    if (ParentClass::Initialize(szContext) == FALSE)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GPIO_Backlight::Initialize: "
            L"Parent class failed to initialize\r\n"
            ));
        goto cleanUp;
        }

    // Read device parameters
    if (GetDeviceRegistryParams(
            szContext, this, dimof(s_deviceRegParams), s_deviceRegParams
            ) != ERROR_SUCCESS)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GPIO_Backlight::Initialize: "
            L"Failed read gpio driver registry parameters\r\n"
            ));
        goto cleanUp;
        }
        
    // GPIO pin not configured, tled only
    if (m_gpioId == GPIO_NOT_USED)
    {
        DEBUGMSG(ZONE_FUNCTION, (
            L"GPIO_Backlight::Initialize: No GPIO pin allocated, continuing...\r\n"
            ));
        rc = TRUE;
        goto cleanUp;
    }

    m_hGpio = GPIOOpen();
    if (m_hGpio == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GPIO_Backlight::Initialize: "
            L"Failed allocate GPIO handle\r\n"
            ));
        goto cleanUp;
        }

    // setup for output mode
    GPIOSetMode(m_hGpio, m_gpioId, GPIO_DIR_OUTPUT);

    rc = TRUE;
cleanUp:

    DEBUGMSG(ZONE_FUNCTION, (
        L"-GPIO_Backlight::Initialize(%s) == %d\r\n", szContext, rc
        ));

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  Uninitialize
//
//  class uninitializer
//
BOOL
GPIO_Backlight::Uninitialize()
{
    DEBUGMSG(ZONE_FUNCTION, (
        L"+GPIO_Backlight::Uninitialize()\r\n"
        ));

    ParentClass::Uninitialize();

    // release gpio handle
    if (m_hGpio != NULL)
        {
        GPIOClose(m_hGpio);
        m_hGpio = NULL;
        }

    DEBUGMSG(ZONE_FUNCTION, (
        L"-GPIO_Backlight::Uninitialize()\r\n"
        ));

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  SetPowerState
//
//  Sets the device power state
//
BOOL
GPIO_Backlight::SetPowerState(
    CEDEVICE_POWER_STATE power
    )
{
    DEBUGMSG(ZONE_FUNCTION, (
        L"+GPIO_Backlight::SetPowerState(0x%08X)\r\n", power
        ));

    ParentClass::SetPowerState(power);

    if (m_gpioId == GPIO_NOT_USED)
        goto cleanUp;
    
    switch (power)
        {
        case D0:
        case D1:
            GPIOSetBit(m_hGpio, m_gpioId);
			RETAILMSG(1, (L"GPIO_Backlight::SetPowerState(On)\r\n"));
            break;
            
        case D2:
        case D3:
        case D4:
			RETAILMSG(1, (L"GPIO_Backlight::SetPowerState(Off)\r\n"));
            GPIOClrBit(m_hGpio, m_gpioId);
            break;
        }    

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (
        L"-GPIO_Backlight::SetPowerState(0x%08X)\r\n", power
        ));
    
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  BacklightGetSupportedStates
//
UCHAR 
GPIO_Backlight::BacklightGetSupportedStates( )
{

    DEBUGMSG( ZONE_BACKLIGHT, ( 
        L"Backlight supported states bitmap: 0x%x\r\n",m_powerMask 
    ));

    return (UCHAR) m_powerMask;
}


