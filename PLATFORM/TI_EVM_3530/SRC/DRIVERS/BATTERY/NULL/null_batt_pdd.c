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
//  File: null_batt_pdd.c
//
//  Null battery driver PDD.
//
#include <windows.h>
#include <battimpl.h>
#include <pm.h>
#include <oal.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap35xx.h>

#include <initguid.h>




static WCHAR _szSourceStateDefault[3]  = L"power.source.event";
//------------------------------------------------------------------------------
//
//  Type:  BATTERY_PDD
//
typedef struct {

	WCHAR  szvpowerSource[256];
	HANDLE hvpowerSource;
	DWORD  powerSourceState;
} Device_t;

static Device_t s_Device;

//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM g_deviceRegParams[] = {
	{
        L"PowerSourceChange", PARAM_STRING, FALSE, offset(Device_t, szvpowerSource),
        fieldsize(Device_t, szvpowerSource), (VOID*)_szSourceStateDefault
    }
};

//------------------------------------------------------------------------------
//
//  Function:  NotifyThread
//
//  handles notifications from external modules about current power state
//
DWORD NotifyThread(
    void* pData
    )
{
    Device_t *pDevice = (Device_t*)pData;
    
    DEBUGMSG(ZONE_FUNCTION, (L"+NotifyThread()\r\n"));
	RETAILMSG(1, (L"+NotifyThread()\r\n"));

    // wait for a signal
    while(TRUE)
    {
        if (WaitForSingleObject(pDevice->hvpowerSource, INFINITE) != WAIT_OBJECT_0)
			break;
   
        // clear notification so we will get next notify
        ResetEvent(pDevice->hvpowerSource);
		s_Device.powerSourceState = GetEventData(pDevice->hvpowerSource);
#if DEBUG
		RETAILMSG(1, (L"+NotifyThread()L new power source state: %d \r\n", s_Device.powerSourceState));
#endif

       
    }

    DEBUGMSG(ZONE_FUNCTION, (L"-NotifyThread()\r\n"));

    return 1;
}


//------------------------------------------------------------------------------
//
//  Function:  InitializeNotifyThread
//
//  instantiates a notify thread if applicable
//
static BOOL
InitializeNotifyThread(
    Device_t *pDevice
    )
{
    HANDLE hThread;
    
    DEBUGMSG(ZONE_FUNCTION, (L"+InitializeNotifyThread()\r\n"));
	RETAILMSG(1, (L"+InitializeNotifyThread()\r\n"));

    // first try creating event then try opening event
    pDevice->hvpowerSource = CreateEvent(NULL, TRUE, FALSE, pDevice->szvpowerSource);
	/*
    if (pDevice->hvpowerSource != INVALID_HANDLE_VALUE && 
        GetLastError() == ERROR_ALREADY_EXISTS)
        {
        pDevice->hvpowerSource = OpenEvent(EVENT_ALL_ACCESS, FALSE,
                                    pDevice->szvpowerSource);
        }

		*/

    if (pDevice->hvpowerSource == INVALID_HANDLE_VALUE)
        {
        pDevice->hvpowerSource = NULL;
        }
    else
        {
        // create notify thread
        hThread = CreateThread(NULL, 0, NotifyThread, pDevice, 0, NULL);
        if (hThread != NULL)
            {
            CloseHandle(hThread);
            }
        
        RETAILMSG(hThread == NULL, (L"ERROR: BatteryPDDGetStatus: "
            L"Failed to spawn notify thread\r\n"
            ));
        }
    
    DEBUGMSG(ZONE_FUNCTION, (L"-InitializeNotifyThread()\r\n"));
    return TRUE;
}



//------------------------------------------------------------------------------
//
//  Function:  BatteryPDDInitialize
//
BOOL WINAPI 
BatteryPDDInitialize(
    LPCTSTR szContext
    )
{


	 BOOL rc = FALSE;

    
	RETAILMSG(1, (L"+ NULL BatteryPDDInitialize()\r\n"));

    // Clear structure
    memset(&s_Device, 0, sizeof(Device_t));

    // Read device parameters
    if (GetDeviceRegistryParams(szContext, &s_Device, 
        dimof(g_deviceRegParams), g_deviceRegParams) != ERROR_SUCCESS) 
        { 
			RETAILMSG(1,(L"ERROR: BatteryPDDInitialize: " L"Failed read driver registry parameters\r\n" ));
			goto cleanUp;
        }
	s_Device.powerSourceState = AC_LINE_ONLINE;

    // check if there is event to notify of AC/battery change
    if (s_Device.szvpowerSource[0] != '\0')
    {
        InitializeNotifyThread(&s_Device);
    }
    rc = TRUE;

cleanUp:
    RETAILMSG(1, (L"-NULL BatteryPDDInitialize\r\n"));
    return rc;


}


//------------------------------------------------------------------------------
//
//  Function:  BatteryPDDDeinitialize
//
void WINAPI 
BatteryPDDDeinitialize()
{
    DEBUGMSG(ZONE_FUNCTION, (L"+BatteryPDDDeinitialize\r\n"));
    DEBUGMSG(ZONE_FUNCTION, (L"-BatteryPDDDeinitialize\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  BatteryPDDResume
//
void WINAPI 
BatteryPDDResume()
{
    DEBUGMSG(ZONE_FUNCTION, (L"+BatteryPDDResume\r\n"));
    DEBUGMSG(ZONE_FUNCTION, (L"-BatteryPDDResume\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  BatteryPDDPowerHandler
//
void WINAPI 
BatteryPDDPowerHandler(
    BOOL off
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"+BatteryPDDPowerHandler(%d)\r\n",  off));
    DEBUGMSG(ZONE_FUNCTION, (L"-BatteryPDDPowerHandler\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function: BatteryPDDGetLevels
//
//  Indicates how many battery levels will be reported by BatteryPDDGetStatus()
//  in the BatteryFlag and BackupBatteryFlag fields of PSYSTEM_POWER_STATUS_EX2.
//
//  Returns the main battery level in the low word, and the backup battery
//  level in the high word.
//
LONG 
BatteryPDDGetLevels()
{
    LONG lLevels = MAKELONG(
        1,      // Main battery levels
        0       // Backup battery levels
    );
    return lLevels;
}


//------------------------------------------------------------------------------
//
//  Function: BatteryPDDSupportsChangeNotification
//
//  Returns FALSE since this platform does not support battery change
//  notification.
//
BOOL 
BatteryPDDSupportsChangeNotification()
{
    return FALSE;
}


//------------------------------------------------------------------------------
//
//  Function: BatteryPddIOControl
//
//  Battery driver needs to handle D0-D4 power notifications
//
//  Returns ERROR code.
//
DWORD
BatteryPddIOControl(
    DWORD  dwContext,
    DWORD  Ioctl,
    PUCHAR pInBuf,
    DWORD  InBufLen, 
    PUCHAR pOutBuf,
    DWORD  OutBufLen,
    PDWORD pdwBytesTransferred
    )
{
    DWORD dwRet;
    
    switch (Ioctl)
        {
        default:
            dwRet = ERROR_NOT_SUPPORTED;
        }
    return dwRet;
}


//------------------------------------------------------------------------------
//
//  Function:  BatteryPDDGetStatus()
//
//  Obtains the battery and power status.
//
BOOL WINAPI 
BatteryPDDGetStatus(
    SYSTEM_POWER_STATUS_EX2 *pStatus, 
    BOOL *pBatteriesChangedSinceLastCall
    ) 
{
    BOOL rc = TRUE;

    DEBUGMSG(ZONE_FUNCTION, (L"+BatteryPDDGetStatus\r\n"));

    pStatus->ACLineStatus               = AC_LINE_ONLINE;
    pStatus->BatteryFlag                = BATTERY_FLAG_NO_BATTERY;
    pStatus->BatteryLifePercent         = BATTERY_PERCENTAGE_UNKNOWN;
    pStatus->Reserved1                  = 0;
    pStatus->BatteryLifeTime            = BATTERY_LIFE_UNKNOWN;
    pStatus->BatteryFullLifeTime        = BATTERY_LIFE_UNKNOWN;

    pStatus->Reserved2                  = 0;
    pStatus->Reserved3                  = 0;    
    pStatus->BackupBatteryLifeTime      = BATTERY_LIFE_UNKNOWN;
    pStatus->BackupBatteryFullLifeTime  = BATTERY_LIFE_UNKNOWN;
    
    pStatus->BackupBatteryFlag          = BATTERY_FLAG_HIGH;
    pStatus->BackupBatteryLifePercent   = 100;

    pStatus->BatteryChemistry           = BATTERY_CHEMISTRY_UNKNOWN;
    pStatus->BatteryVoltage             = 0;
    pStatus->BatteryCurrent             = 0;
    pStatus->BatteryAverageCurrent      = 0;
    pStatus->BatteryAverageInterval     = 0;
    pStatus->BatterymAHourConsumed      = 0;
    pStatus->BatteryTemperature         = 250;  // unit is 0.1 deg C
    pStatus->BackupBatteryVoltage       = 0;

    *pBatteriesChangedSinceLastCall = FALSE;



	pStatus->BatteryFlag                 =  BATTERY_FLAG_NO_BATTERY|BATTERY_FLAG_HIGH;
	pStatus->ACLineStatus                = (BYTE)s_Device.powerSourceState;



    DEBUGMSG(ZONE_PDD, (L"-BatteryPDDGetStatus(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
