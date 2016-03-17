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
//  File: tled_backlight.cpp
//

#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <oal.h>
#include <ceddk.h>
#include <ceddkex.h>

#include <omap35xx.h>

#include <initguid.h>
#include <tled.h>
#include "..\backlight_class.h"
#include "..\bl.h"
#include <light_sense.h>
#include <bsp.h>
//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = {
    {
    L"TLedChannel", PARAM_DWORD, TRUE, offset(TLED_Backlight, m_channel),
        fieldsize(TLED_Backlight, m_channel), NULL
    }, {
		BL_REG_BATT_LEVEL, PARAM_MULTIDWORD, TRUE, offset(TLED_Backlight, m_dutyCyclesBatt),
        fieldsize(TLED_Backlight, m_dutyCyclesBatt), NULL
    }, {
		BL_REG_AC_LEVEL, PARAM_MULTIDWORD, TRUE, offset(TLED_Backlight, m_dutyCyclesAC),
        fieldsize(TLED_Backlight, m_dutyCyclesAC), NULL
    }, {
		BL_REG_MIN_LEVEL_DEF, PARAM_DWORD, 0, offset(TLED_Backlight, m_MaxDutyCyclesDef),
        fieldsize(TLED_Backlight, m_MaxDutyCyclesDef), (VOID*)BKL_DEFAULT_DUTY_CYCLE
    }, {
 		BL_REG_MIN_LEVEL_7I, PARAM_DWORD, 0, offset(TLED_Backlight, m_MaxDutyCycles7i),
        fieldsize(TLED_Backlight, m_MaxDutyCycles7i), (VOID*)BKL_7INCH_DUTY_CYCLE
   }
};

BOOL IsDutyCycleConfigured(UINT8 c)
{
	HKEY hKey = NULL;
	UINT32 val = 96, dc7;

	LONG lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("\\Drivers\\BuiltIn\\Backlight"), 0, 0, &hKey);
	if (ERROR_SUCCESS == lStatus)
	{
		if(c == 'B')
		{
			DWORD len = sizeof(UINT32);

			lStatus = RegQueryValueEx(hKey, TEXT("MaxDutyCyclesDef"), 0, 0, (LPBYTE)&val, &len);
			if (ERROR_SUCCESS != lStatus)
				val = 96;

			lStatus = RegQueryValueEx(hKey, TEXT("MaxDutyCycles7i"), 0, 0, (LPBYTE)&dc7, &len);
			if (ERROR_SUCCESS != lStatus || dc7 < 60)
				dc7 = val;

			lStatus = RegSetValueEx(hKey, TEXT("MaxDutyCycles7i"), 0, REG_DWORD, (LPBYTE)&dc7, len);

		}
		lStatus = RegQueryValueEx(hKey, TEXT("DutyCyclesAC"), 0, 0, 0, 0);
		if (ERROR_SUCCESS == lStatus)
			lStatus = RegQueryValueEx(hKey, TEXT("DutyCyclesBatt"), 0, 0, 0, 0);

		RegCloseKey(hKey);
	}

	return ERROR_SUCCESS == lStatus;
}

//------------------------------------------------------------------------------
//
//  Function:  Initialize
//
//  class initializer
//
BOOL
TLED_Backlight::Initialize(
    LPCTSTR szContext
    )
{
	card_ver cv = {0};
	UINT32 in = OAL_ARGS_QUERY_LCD_CFG;
    BOOL rc = FALSE;

    DEBUGMSG(ZONE_FUNCTION, (
        L"+TLED_Backlight::Initialize(%s)\r\n", szContext
        ));

	if (ParentClass::Initialize(szContext) == FALSE)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TLED_Backlight::Initialize: "
            L"Parent class failed to initialize\r\n"
            ));
        goto cleanUp;
        }

	if (KernelIoControl(IOCTL_HAL_QUERY_CARD_CFG, &in, sizeof(in), &cv, sizeof(cv), 0))
	{
		if (cv.config == 'B')
			m_bIs4inch = FALSE;
	}


	if (!IsDutyCycleConfigured(cv.config))
		RegCopyValues(m_bIs4inch ? _T("\\Drivers\\BuiltIn\\Backlight\\LcdDefault") : _T("\\Drivers\\BuiltIn\\Backlight\\Lcd7Inch"), BL_REG_KEY);

    // Read device parameters
    if (GetDeviceRegistryParams(
            szContext, this, dimof(s_deviceRegParams), s_deviceRegParams
            ) != ERROR_SUCCESS)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TLED_Backlight::Initialize: "
            L"Failed read gpio driver registry parameters\r\n"
            ));
        goto cleanUp;
        }
	m_minDutyCycle = m_bIs4inch ? m_MaxDutyCyclesDef : m_MaxDutyCycles7i;

    m_hTled = TLEDOpen();
    if (m_hTled == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TLED_Backlight::Initialize: "
            L"Failed allocate TLED handle\r\n"
            ));
        goto cleanUp;
        }

    TLEDSetChannel(m_hTled, m_channel);

    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (
        L"-TLED_Backlight::Initialize(%s)\r\n", szContext
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
TLED_Backlight::Uninitialize()
{
    DEBUGMSG(ZONE_FUNCTION, (
        L"+TLED_Backlight::Uninitialize()\r\n"
        ));

    ParentClass::Uninitialize();

    // release gpio handle
    if (m_hTled != NULL)
        {
        TLEDClose(m_hTled);
        m_hTled = NULL;
        }

    DEBUGMSG(ZONE_FUNCTION, (
        L"-TLED_Backlight::Uninitialize()\r\n"
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
TLED_Backlight::SetPowerState(
    CEDEVICE_POWER_STATE power
    )
{
    DEBUGMSG(ZONE_FUNCTION, (
        L"+TLED_Backlight::SetPowerState(0x%08X)\r\n", power
        ));

    ParentClass::SetPowerState(power);

    if (D0 <= power && power <= D4)
	{
		RETAILMSG(0, (L"TLED_Backlight::SetPowerState(D%d) -> %d\r\n", power, m_dutyCycles[power]));
		
		if (D0 <= power && power <= D2 && m_dutyCycles[power] > m_minDutyCycle)
			TLEDSetDutyCycle(m_hTled, 100 - m_minDutyCycle);
		else
			TLEDSetDutyCycle(m_hTled, 100 - m_dutyCycles[power]);
    }

    DEBUGMSG(ZONE_FUNCTION, (
        L"-TLED_Backlight::SetPowerState(0x%08X)\r\n", power
        ));

    return TRUE;
}

void	TLED_Backlight::UpdateDutyCycles( DWORD PwrSrc, DWORD LsLevel )
{
	if(PwrSrc)
		memcpy( m_dutyCycles, m_dutyCyclesAC, sizeof(m_dutyCycles) );
	else
		memcpy( m_dutyCycles, m_dutyCyclesBatt, sizeof(m_dutyCycles) );

	if( AMBIENT_LIGHT_NORMAL == LsLevel )
		m_dutyCycles[0] = m_dutyCycles[5];
	else if( AMBIENT_LIGHT_LOW == LsLevel )
		m_dutyCycles[0] = m_dutyCycles[6];
}	

void TLED_Backlight::UpdateRegParams()
{
    if( GetDeviceRegistryParams( BL_REG_KEY, this, dimof(s_deviceRegParams), s_deviceRegParams ) != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TLED_Backlight::UpdateRegParams: "
							    L"Failed read gpio driver registry parameters\r\n"            ));
    }

	m_minDutyCycle = m_bIs4inch ? m_MaxDutyCyclesDef : m_MaxDutyCycles7i;
}
