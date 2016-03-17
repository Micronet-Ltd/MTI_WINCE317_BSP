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
//  File: batt_pdd.c
//
//  Battery drivers PDD for the BQ2xxxxx chip.
//
#include <windows.h>
#include <battimpl.h>
#include <pm.h>
#include <oal.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap35xx.h> 


#include <initguid.h>
#include <hdq.h>
#include <bci.h>
#include <gpio.h>
#include "bq27000.h"

//#define DEBUG 1


//------------------------------------------------------------------------------
//
//  Type:  static string
//
//static WCHAR _szChargeDefault[3]       = L"\0\0";
static WCHAR _szSourceStateDefault[3]  = L"power.source.event";


//------------------------------------------------------------------------------
//
//  Type:  BATTERY_PDD
//
typedef struct {
    DWORD  hdqRetry;
    DWORD  high;
    DWORD  low;
    DWORD  critical;
    DWORD  ar;
	DWORD  criticalVoltage;
	HANDLE hHDQ;
	WCHAR  szvpowerSource[256];
	HANDLE hvpowerSource;
	HANDLE hGPIO;
	DWORD  powerSourceState;
	DWORD  gpio0;
	DWORD  gpio1;
} Device_t;

static Device_t s_Device;

//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM g_deviceRegParams[] = {
    {
        L"HdqRetry", PARAM_DWORD, FALSE, offset(Device_t, hdqRetry),
        fieldsize(Device_t, hdqRetry), (VOID*)5
    }, {
        L"High", PARAM_DWORD, FALSE, offset(Device_t, high),
        fieldsize(Device_t, high), (VOID*)30
    }, {
        L"Low", PARAM_DWORD, FALSE, offset(Device_t, low),
        fieldsize(Device_t, low), (VOID*)10
    }, {
        L"Critical", PARAM_DWORD, FALSE, offset(Device_t, critical),
        fieldsize(Device_t, critical), (VOID*)5
    }, {
        L"AR", PARAM_DWORD, FALSE, offset(Device_t, ar),
        fieldsize(Device_t, ar), (VOID*)0
    }, 
	{
        L"CriticalVoltage", PARAM_DWORD, FALSE, offset(Device_t, criticalVoltage),
        fieldsize(Device_t, criticalVoltage), (VOID*)3400
    }, 
	{
        L"gpio0", PARAM_DWORD, FALSE, offset(Device_t, gpio0),
        fieldsize(Device_t, gpio0), (VOID*)GPIO_140
    },
		{
         L"gpio1", PARAM_DWORD, FALSE, offset(Device_t, gpio1),
        fieldsize(Device_t, gpio1), (VOID*)GPIO_141
    },
		{
        L"PowerSourceChange", PARAM_STRING, FALSE, offset(Device_t, szvpowerSource),
        fieldsize(Device_t, szvpowerSource), (VOID*)_szSourceStateDefault
    }
};

DWORD GetOffsetValues();
//------------------------------------------------------------------------------
//
//  Function:  GetData16
//
//  It reads one 16 bit value from the BQ and validates consistency.
//
static BOOL 
GetData16(
    UCHAR address, 
    USHORT *pData
    )
{
    BOOL rc = FALSE;
    UCHAR low1, high1, high2;
    DWORD start, ix;

    for (ix = 0; ix < s_Device.hdqRetry; ix++) 
        {
        // Get beginning of measure cycle
        start = GetTickCount();

        // Read values first time        
        if (!HdqRead(s_Device.hHDQ, address + 1, &high1)) break;
        if (!HdqRead(s_Device.hHDQ, address, &low1)) break;

        // Read high byte again
        if (!HdqRead(s_Device.hHDQ, address + 1, &high2)) break;

        // If we read same value again, we have correct number
        if (high1 == high2) 
            {
            rc = TRUE;
            break;
            }

        // Read low byte again
        if (!HdqRead(s_Device.hHDQ, address, &low1)) break;

        // If we made it in limit, use second high value
        if ((GetTickCount() - start) < 800) 
            {
            high1 = high2;
            rc = TRUE;
            break;
            }
        } 

    // Get final value
    if (rc) 
        {
        *pData = (high1 << 8) | low1;
        } 
    else 
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: BatteryPDDGetStatus: "
            L"16-bit value inconsistency\r\n"
            ));
/*
		RETAILMSG(1, (L"ERROR: BatteryPDDGetStatus: "
            L"16-bit value inconsistency\r\n"
            ));
			*/
        }
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  GetData8
//
//  It reads one 8 bit value from the BQ
//
static BOOL 
GetData8(
    UCHAR address, 
    UCHAR *pData
    )
{
   return HdqRead(s_Device.hHDQ, address, pData);
}    

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

    DEBUGMSG(ZONE_FUNCTION, (L"+BatteryPDDInitialize()\r\n"));
	//RETAILMSG(1, (L"+BatteryPDDInitialize()\r\n"));

    // Clear structure
    memset(&s_Device, 0, sizeof(Device_t));

    // Read device parameters
    if (GetDeviceRegistryParams(szContext, &s_Device, 
        dimof(g_deviceRegParams), g_deviceRegParams) != ERROR_SUCCESS) 
        {
        DEBUGMSG(ZONE_ERROR,(L"ERROR: BatteryPDDInitialize: "
            L"Failed read driver registry parameters\r\n"
            ));
//		RETAILMSG(1,(L"ERROR: BatteryPDDInitialize: "
 //           L"Failed read driver registry parameters\r\n"
 //           ));
        goto cleanUp;
        }
    
//	RETAILMSG(1, (L"+BatteryPDDInitialize1()\r\n"));
    // Open HDQ bus
    if ((s_Device.hHDQ = HdqOpen()) == NULL) 
        {
        DEBUGMSG(ZONE_ERROR,(L"ERROR: BatteryPDDInitialize: "
            L"Failed open HDQ device driver\r\n"
            ));

        goto cleanUp;
        }

    // Set AR register
    if (s_Device.ar != 0) 
        {
        if (!HdqWrite(s_Device.hHDQ, BQ_ARL, (UCHAR)(s_Device.ar & 0xFF)) ||
            !HdqWrite(s_Device.hHDQ, BQ_ARH, (UCHAR)((s_Device.ar >> 8) & 0xFF))) 
            {
            DEBUGMSG(ZONE_WARN, (L"WARN: BatteryPDDInitialize: "
                L"AR register write error\r\n"
                ));
            }            
        }

//	RETAILMSG(1, (L"+BatteryPDDInitialize() 3\r\n"));
	s_Device.powerSourceState = AC_LINE_ONLINE;

    // check if there is event to notify of AC/battery change
    if (s_Device.szvpowerSource[0] != '\0')
    {
        InitializeNotifyThread(&s_Device);
    }

	s_Device.hGPIO = GPIOOpen();
	if (s_Device.hGPIO == NULL)
	{
		RETAILMSG(1, (L"ERROR: BatteryPDDInitialize: "L"Failed to open Gpio driver \r\n"));
		goto cleanUp;
	}

	 GPIOSetMode(s_Device.hGPIO, s_Device.gpio0, GPIO_DIR_INPUT);
	 GPIOSetMode(s_Device.hGPIO, s_Device.gpio1, GPIO_DIR_INPUT);


    rc = TRUE;


cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-BatteryPDDInitialize\r\n"));
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

    if (s_Device.hHDQ != NULL) HdqClose(s_Device.hHDQ);
	if (s_Device.hGPIO != NULL) GPIOClose(s_Device.hGPIO);
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
        3,      // Main battery levels
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
    BOOL nAttempts = 0;
    BOOL rc = FALSE;
    BOOL bBatteryAttached = TRUE;
    USHORT ar, artte, temp, volt,tte,ai,lmd;
    UCHAR flags, rsoc;
	UCHAR chargingStatus = 0;
//    BatteryChargeMode_e chargeMode;
#ifdef DEBUG
    USHORT nac, cacd, cact;
    UCHAR ilmd, sedvf, sedv1, islc, dmfsd, taper;
    UCHAR pkcfg, id3, dcomp, tcomp;
#endif

    DEBUGMSG(ZONE_FUNCTION, (L"+BatteryPDDGetStatus\r\n"));
	RETAILMSG(1, (L"+BatteryPDDGetStatus\r\n"));


    while (nAttempts < 2 && rc == FALSE)
        {
        nAttempts++;
		RETAILMSG(1, (L"+BatteryPDDGetStatus %x attempt\r\n", nAttempts));
        
        // The default settings for cheating OS system. 
        pStatus->ACLineStatus               = AC_LINE_UNKNOWN;
        pStatus->BatteryFlag                = 0;
        pStatus->BatteryLifePercent         = BATTERY_PERCENTAGE_UNKNOWN;
        pStatus->Reserved1                  = 0;
        pStatus->BatteryLifeTime            = BATTERY_LIFE_UNKNOWN;
        pStatus->BatteryFullLifeTime        = BATTERY_LIFE_UNKNOWN;

        // NOTE:
        //  If backup battery exists these fields should be filled
        // with correct information.  For now just indicate full charge
        //    
        pStatus->Reserved2                  = 0;
        pStatus->Reserved3                  = 0;    
        pStatus->BackupBatteryLifeTime      = BATTERY_LIFE_UNKNOWN;
        pStatus->BackupBatteryFullLifeTime  = BATTERY_LIFE_UNKNOWN;
        
     //   pStatus->BackupBatteryFlag          = BATTERY_FLAG_NO_BATTERY;
     //   pStatus->BackupBatteryLifePercent   = BATTERY_PERCENTAGE_UNKNOWN;
        pStatus->BackupBatteryFlag          = BATTERY_FLAG_HIGH;
        pStatus->BackupBatteryLifePercent   = 50;    

        pStatus->BatteryChemistry           = BATTERY_CHEMISTRY_UNKNOWN;
        pStatus->BatteryVoltage             = 0;
        pStatus->BatteryCurrent             = 0;
        pStatus->BatteryAverageCurrent      = 0;
        pStatus->BatteryAverageInterval     = 0;
        pStatus->BatterymAHourConsumed      = 0;
        pStatus->BatteryTemperature         = 0;
        pStatus->BackupBatteryVoltage       = 0;

        // BQ2xxxx Flags
        if (!GetData8(BQ_FLAGS, &flags)) continue;
        // Relative State Of Charge
        if (!GetData8(BQ_RSOC, &rsoc)) continue;
        // AT Rate
        if (!GetData16(BQ_ARL, &ar)) continue;
        // AT Rate Time To Empty
        if (!GetData16(BQ_ARTTEL, &artte)) continue;
		// time To Empty based on average current
        if (!GetData16(BQ_TTEL, &tte)) continue;
        // Reported Temperature.
        if (!GetData16(BQ_TEMPL, &temp)) continue;
        // Reported Voltage
        if (!GetData16(BQ_VOLTL, &volt)) continue;
		// reported average current
		if (!GetData16(BQ_AIL, &ai)) continue;
		// Last Measured Discharge
        if (!GetData16(BQ_LMDL, &lmd)) continue;

#ifdef DEBUG
        // Nominal Available Capacity
        if (!GetData16(BQ_NACL, &nac)) continue;
        // Discharge Compensated Available Capacity
        if (!GetData16(BQ_CACDL, &cacd))  continue;

        // Temperature Compensated Available Capacity
        if (!GetData16(BQ_CACTL, &cact)) continue;
        // BQ26500 EEPROM register values
        if (!GetData8(BQ_EE_ILMD, &ilmd)) continue;
        if (!GetData8(BQ_EE_SEDVF, &sedvf)) continue;
        if (!GetData8(BQ_EE_SEDV1, &sedv1)) continue;
        if (!GetData8(BQ_EE_ISLC, &islc)) continue;
        if (!GetData8(BQ_EE_DMFSD, &dmfsd)) continue;
        if (!GetData8(BQ_EE_TAPER, &taper)) continue;
        if (!GetData8(BQ_EE_PKCFG, &pkcfg)) continue;
        if (!GetData8(BQ_EE_ID3, &id3)) continue;
        if (!GetData8(BQ_EE_DCOMP, &dcomp)) continue;
        if (!GetData8(BQ_EE_TCOMP, &tcomp)) continue;

#if 0
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: NAC   = %d\r\n", nac));
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: CACD  = %d\r\n", cacd));
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: CACT  = %d\r\n", cact));
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: ILMD  = %d\r\n", ilmd));
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: SEDVF = %d\r\n", sedvf));
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: SEDV1 = %d\r\n", sedv1));
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: ISLC  = %d\r\n", islc));
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: DMFSD = %d\r\n", dmfsd));
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: TAPER = %d\r\n", taper));
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: PKCFG = %d\r\n", pkcfg));
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: ID3   = %d\r\n", id3));
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: DCOMP = %d\r\n", dcomp));
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: TCOMP = %d\r\n", tcomp));
#endif

		RETAILMSG(0, (L"BatteryPDDGetStatus: NAC   = %d\r\n", nac));
        RETAILMSG(0, (L"BatteryPDDGetStatus: CACD  = %d\r\n", cacd));
        RETAILMSG(0, (L"BatteryPDDGetStatus: CACT  = %d\r\n", cact));
        RETAILMSG(0, (L"BatteryPDDGetStatus: ILMD  = %d\r\n", ilmd));
        RETAILMSG(0, (L"BatteryPDDGetStatus: SEDVF = %d\r\n", sedvf));
        RETAILMSG(0, (L"BatteryPDDGetStatus: SEDV1 = %d\r\n", sedv1));
        RETAILMSG(0, (L"BatteryPDDGetStatus: ISLC  = %d\r\n", islc));
        RETAILMSG(0, (L"BatteryPDDGetStatus: DMFSD = %d\r\n", dmfsd));
        RETAILMSG(0, (L"BatteryPDDGetStatus: TAPER = %d\r\n", taper));
        RETAILMSG(0, (L"BatteryPDDGetStatus: PKCFG = %d\r\n", pkcfg));
        RETAILMSG(0, (L"BatteryPDDGetStatus: ID3   = %d\r\n", id3));
        RETAILMSG(0, (L"BatteryPDDGetStatus: DCOMP = %d\r\n", dcomp));
        RETAILMSG(0, (L"BatteryPDDGetStatus: TCOMP = %d\r\n", tcomp));

	
#if 0
		DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: LMD   = %d\r\n", lmd));
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: FLAGS = 0x%x\r\n", flags));
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: TEMP  = %d K\r\n", temp/4));
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: VOLT  = %d mV\r\n", volt));
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: RSOC  = %d %%\r\n", rsoc));
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: AR    = %d\r\n", ar));
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: ARTTE = %d\r\n", artte));
		DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: TTE = %d\r\n", tte));
#endif
	
		RETAILMSG(0, (L"BatteryPDDGetStatus: LMD   = %d\r\n", lmd));
		RETAILMSG(0, (L"BatteryPDDGetStatus: AI   = %d\r\n", ai));
		RETAILMSG(0, (L"BatteryPDDGetStatus: FLAGS = 0x%x\r\n", flags));
        RETAILMSG(0, (L"BatteryPDDGetStatus: TEMP  = %d K\r\n", temp/4));
        RETAILMSG(0, (L"BatteryPDDGetStatus: VOLT  = %d mV\r\n", volt));
        RETAILMSG(0, (L"BatteryPDDGetStatus: RSOC  = %d %%\r\n", rsoc));
        RETAILMSG(0, (L"BatteryPDDGetStatus: AR    = %d\r\n", ar));
        RETAILMSG(0, (L"BatteryPDDGetStatus: ARTTE = %d\r\n", artte));
		RETAILMSG(0, (L"BatteryPDDGetStatus: TTE = %d\r\n", tte));
#endif


        // if get here then call was successful so don't loop again
        rc = TRUE;
        }

		// need to be decided what to do with this information
	chargingStatus|=(UINT8)GPIOGetBit(s_Device.hGPIO, s_Device.gpio0);
    chargingStatus|=(UINT8)(((UINT8)GPIOGetBit(s_Device.hGPIO, s_Device.gpio1)) << 1);

	// we need to fill the info which is not related to HDQ first
	pStatus->ACLineStatus                = (BYTE)s_Device.powerSourceState;
	pStatus->Reserved1                   = chargingStatus;

		// if there is no hdq line connected it means there is no battery
    if (rc == FALSE /*|| chargingStatus == 0x00*/)
	{
	//	pStatus->ACLineStatus = AC_LINE_ONLINE;
		pStatus->BatteryFlag = BATTERY_FLAG_NO_BATTERY|BATTERY_FLAG_HIGH;
		//RETAILMSG(1, (L" BatteryPDDGetStatus: there is no battery pStatus->ACLineStatus = %d \r\n",pStatus->ACLineStatus));
		rc = TRUE;
		goto cleanUp;
	}

	//pStatus->ACLineStatus = (BYTE)s_Device.powerSourceState;

	if ((flags & BQ_FLAGS_CHGS)==BQ_FLAGS_CHGS)
		pStatus->BatteryFlag |= BATTERY_FLAG_CHARGING;

	if((pStatus->BatteryFlag & BATTERY_FLAG_CHARGING) && (chargingStatus == 0x01 || chargingStatus == 0x03))

	{
		RETAILMSG(1, (L" According to fuel gauge the status is charging but according to charger the status is charging done\r\n"));
	}

    // Update status with data
    pStatus->BatteryLifePercent = rsoc;


    if (ar != 0) 
	{
		pStatus->BatteryLifeTime = artte * 60;
		pStatus->BatteryFullLifeTime = 60*60*lmd/ar;
	}
	// if baterry is discharging now so we are allowed to make calculations based on average discharge current
	else if (ai && ((flags & BQ_FLAGS_CHGS) ==0))
	{
    	pStatus->BatteryLifeTime = tte * 60;
		pStatus->BatteryFullLifeTime = 60*60*lmd/ai;
	}

    pStatus->BatteryChemistry            = BATTERY_CHEMISTRY_LIPOLY;
    pStatus->BatteryVoltage              = volt;
	pStatus->BatteryCurrent              = (flags & BQ_FLAGS_CHGS)? (ai*357)/(100*20) : (-1)*(ai*357)/(100*20);
	pStatus->BatteryAverageCurrent       = (flags & BQ_FLAGS_CHGS)? (ai*357)/(100*20) : (-1)*(ai*357)/(100*20);
    pStatus->BatteryAverageInterval      = 5120;
	pStatus->BatterymAHourConsumed       = (-1)*(lmd*357)/(100*20);
    pStatus->BatteryTemperature          = temp*25/10 - 2730;   // unit is 0.1 deg C
//	pStatus->Reserved1                   = chargingStatus;
    *pBatteriesChangedSinceLastCall      = FALSE;

 
    if (rsoc > s_Device.high)
        {        
        pStatus->BatteryFlag |= BATTERY_FLAG_HIGH;
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: "
            L"Capacity > %d%% (high)\r\n", 
            s_Device.high
            ));
        }
    else if (rsoc > s_Device.low)
        {
        pStatus->BatteryFlag |= BATTERY_FLAG_LOW;
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: "
            L"%d%% < capacity < %d%% (low)\r\n", 
            s_Device.low, s_Device.high
            ));
        }
    else if (rsoc > s_Device.critical)
        {
        pStatus->BatteryFlag |= BATTERY_FLAG_CRITICAL;
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: "
            L"%d%% <= capacity < %d%% (critical)\r\n", 
            s_Device.critical, s_Device.low
            ));
        }
    else 
        {
         pStatus->BatteryFlag |= BATTERY_FLAG_CRITICAL;
        DEBUGMSG(ZONE_PDD, (L"BatteryPDDGetStatus: "
            L"capacity < %d%% (system suspend)\r\n", s_Device.critical
            ));
        
        // Need to check if PPC PM maps this correctly to POWER_STATE_SUSPEND.
        if (pStatus->ACLineStatus == AC_LINE_OFFLINE)
            {
              SetSystemPowerState(NULL, POWER_STATE_OFF, POWER_FORCE);
            }
        }

	if(pStatus->ACLineStatus == AC_LINE_OFFLINE && pStatus->BatteryVoltage < s_Device.criticalVoltage)
	{
		SetSystemPowerState(NULL, POWER_STATE_OFF, POWER_FORCE);
	}
    
#ifdef DEBUG
	  RETAILMSG(0, (L" ACLineStatus           = 0x%x (0x00 - AC_LINE_OFFLINE, 0x01 - AC_LINE_ONLINE, 0xFF - AC_LINE_UNKNOWN) \r\n", pStatus->ACLineStatus));
      RETAILMSG(0, (L" BatteryFlag            = 0x%x (0x01 - HIGH, 0x02 - LOW, 0x04 - CRITICAL, 0x08 - CHARGING, 0xFF - UNKNOWN) \r\n", pStatus->BatteryFlag));
	  RETAILMSG(0, (L" BatteryLifePercent     =   %d percent\r\n", pStatus->BatteryLifePercent));
	  RETAILMSG(0, (L" BatteryLifeTime        =   %d second\r\n", pStatus->BatteryLifeTime));
	  RETAILMSG(0, (L" BatteryFullLifeTime    =   %d second\r\n", pStatus->BatteryFullLifeTime));
	  RETAILMSG(0, (L" BatteryVoltage         =   %d mV\r\n", pStatus->BatteryVoltage));
	  RETAILMSG(0, (L" BatteryCurrent         =   %d mA\r\n", pStatus->BatteryCurrent));
	  RETAILMSG(0, (L" BatteryAverageCurrent  =   %d mA\r\n", pStatus->BatteryAverageCurrent));
	  RETAILMSG(0, (L" BatteryAverageInterval =   %d mSecond\r\n", pStatus->BatteryAverageInterval));
	  RETAILMSG(0, (L" BatterymAHourConsumed  =   %d mAH\r\n", pStatus->BatterymAHourConsumed));
	  RETAILMSG(0, (L" BatteryTemperature     =   %d 0.1 degrees Celsius\r\n", pStatus->BatteryTemperature));
	  RETAILMSG(0, (L" Charging status        =   %d (3 - Charge suspend,2- fast charge in progress,1- charge done,0- precharge in progress)\r\n", chargingStatus));

#endif

cleanUp:
    DEBUGMSG(ZONE_PDD, (L"-BatteryPDDGetStatus(rc = %d)\r\n", rc));
	RETAILMSG(1, (L"-BatteryPDDGetStatus(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------


DWORD GetOffsetValues()
{
	USHORT ceo,ai;
	SHORT  val;
	UCHAR  ctrl,flags,val1,val2;;

	// BQ2xxxx Flags
    if (!GetData8(BQ_FLAGS, &flags))
	{

		RETAILMSG(1, (L"GetOffsetValues: GetData8 failed \r\n"));
		return 0;
	}
	GetData8(BQ_FLAGS, &flags);
	GetData8(0x5e, &val1);
	GetData8(0x5f, &val2);


    RETAILMSG(1, (L"GetOffsetValues started: flags = 0x%x\r\n",flags));

	if  ((flags & BQ_FLAGS_NOACT)  == 0)
	{
		RETAILMSG(1, (L"GetOffsetValues: there is a battery activity \r\n"));
	}

	if (!HdqWrite(s_Device.hHDQ, BQ_MODE, 0x20))
	{
		Sleep(1000);
		RETAILMSG(1, (L"GetOffsetValues: write mode register failed \r\n"));
		return 0;
	}

	if (!HdqWrite(s_Device.hHDQ, BQ_CTRL, 0x56)) 
    {
		 RETAILMSG(1, (L"GetOffsetValues: write registers failed \r\n"));
	}
	else
	{
		/*
		GetData8(BQ_CTRL,&ctrl);
		GetData16(0x5e, &ceo);
		GetData8(0x5e, &val1);
		GetData8(0x5f, &val2);

		RETAILMSG(1, (L"GetOffsetValues1: val1 = %d, val2 =%d \r\n",val1,val2));
		val = ceo;
		RETAILMSG(1, (L"GetOffsetValues before: ceo_l = %x, ceo_h = %x, val = %d ,ctrl = %x \r\n",(ceo&0xFF),((ceo>>8)&0xFF),val,ctrl));
        */

		Sleep(7000);
		GetData8(BQ_CTRL,&ctrl);
		if(ctrl)
		{
			RETAILMSG(1, (L"GetOffsetValues: cmd is not accepted yet ctrl = %x \r\n",ctrl));
			return 0;
		}
		else
		{
				/*
				GetData8(0x5e, &val1);
				GetData8(0x5f, &val2);

				RETAILMSG(1, (L"GetOffsetValues3: val1 = %d, val2 =%d \r\n",val1,val2));
				*/

			if (!GetData16(0x5e, &ceo)) 
			{
				RETAILMSG(1, (L"GetOffsetValues: failed to read ceo \r\n"));
			}
			else
			{
				val = ceo;
				GetData8(BQ_FLAGS, &flags);
				GetData8(BQ_CTRL,&ctrl);
				GetData16(BQ_AIL, &ai);
				RETAILMSG(1, (L"GetOffsetValues: ceo_l = %x, ceo_h = %x, val = %d, flags = %x ,ai= %d , ctrl =%x \r\n",(ceo&0xFF),((ceo>>8)&0xFF),val,flags,ai,ctrl ));
			}
		}
	}


	//--------------------------------------------------------------

	/*
	GetData8(0x5e, &val1);
	GetData8(0x5f, &val2);

	RETAILMSG(1, (L"GetOffsetValues4: val1 = %d, val2 =%d \r\n",val1,val2));
	*/
	if (!HdqWrite(s_Device.hHDQ, BQ_MODE, (UCHAR)(0x10)) ||
            !HdqWrite(s_Device.hHDQ, BQ_CTRL, (UCHAR)(0x56))) 
    {
		 RETAILMSG(1, (L"GetOffsetValues: write registers failed \r\n"));
	}
	else
	{
		/*
		GetData8(BQ_CTRL,&ctrl);
		GetData16(0x5e, &ceo);
		GetData8(0x5e, &val1);
		GetData8(0x5f, &val2);

		RETAILMSG(1, (L"GetOffsetValues5: val1 = %d, val2 =%d \r\n",val1,val2));
		val = ceo;
		RETAILMSG(1, (L"GetOffsetValues before: cio_l = %x, cio_h = %x, val = %d ,ctrl = %x \r\n",(ceo&0xFF),((ceo>>8)&0xFF),val,ctrl));
		*/
		Sleep(7000);
		GetData8(BQ_CTRL,&ctrl);
		if(ctrl)
		{
			RETAILMSG(1, (L"GetOffsetValues: cmd1 is not accepted yet ctrl = %x \r\n",ctrl));
			return 0;
		}
		else
		{
			/*
			GetData8(0x5e, &val1);
			GetData8(0x5f, &val2);

			RETAILMSG(1, (L"GetOffsetValues6: val1 = %d, val2 =%d \r\n",val1,val2));
			*/

			if (!GetData16(0x5e, &ceo)) 
			{
				RETAILMSG(1, (L"GetOffsetValues: failed to read cio \r\n"));
			}
			else
			{
				val = ceo;
				GetData8(BQ_FLAGS, &flags);
				GetData16(BQ_AIL, &ai);
				GetData8(BQ_CTRL,&ctrl);
				RETAILMSG(1, (L"GetOffsetValues: cio_l = %x, cio_h = %x ,val = %d ,flags = %x,ai = %d ,ctrl = %x \r\n",(ceo&0xFF),((ceo>>8)&0xFF),val,flags,ai,ctrl));
			}
		}
	}
	return 1;
}