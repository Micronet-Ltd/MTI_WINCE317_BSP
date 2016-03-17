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
//  File: rtc.cpp
//
#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <oal.h>
#include <oalex.h>
#include <rtcex.h>
#include <omap35xx.h>
#include <tps659xx.h>

#include <initguid.h>
#include <twl.h>
#include <nkintr.h>


// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)

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
#undef ZONE_INIT
#undef ZONE_INFO

#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INIT           DEBUGZONE(3)
#define ZONE_INFO           DEBUGZONE(4)
#define ZONE_INT_THREAD     DEBUGZONE(5)
#define ZONE_SYNC_THREAD    DEBUGZONE(6)
#define ZONE_ALARM          DEBUGZONE(7)

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
DBGPARAM dpCurSettings = {
    L"RTC", {
        L"Errors",      L"Warnings",    L"Function",    L"Init",
        L"Info",        L"Intr Thread", L"Sync Thread", L"Alarm",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    0x00F3
};

#endif


//------------------------------------------------------------------------------
//  Local Definitions

#define RTC_DEVICE_COOKIE       'rtcD'


//------------------------------------------------------------------------------
//  Local Structures

typedef struct {
    DWORD cookie;
    DWORD priority256;
    DWORD syncPeriod;
    UINT32 sysIntrRTCQuery;
    HANDLE hTWL;
    HANDLE hIntrEvent;
    HANDLE hIntrThread;
    HANDLE hSyncEvent;
    HANDLE hSyncThread;
    BOOL threadsExit;
    DWORD enableWake;
    DWORD secureOffset;
    CEDEVICE_POWER_STATE powerState;
	CRITICAL_SECTION cs;
	HANDLE hRTCWakeupEvent;
} Device_t;

//------------------------------------------------------------------------------
//  Local Functions

BOOL
RTC_Deinit(
    DWORD context
    );

static 
DWORD 
RTC_IntrThread(
    VOID *pContext
    );

static 
DWORD 
RTC_SyncThread(
    VOID *pContext
    );

//------------------------------------------------------------------------------
//  Device registry parameters

// UNDONE:
//  Eventually we want the timer interrupt to be triggered 1 every day
//
static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = {
    {
        L"Priority256", PARAM_DWORD, FALSE, offset(Device_t, priority256),
        fieldsize(Device_t, priority256), (VOID*)102
    }, {
        L"SyncPeriod", PARAM_DWORD, FALSE, offset(Device_t, syncPeriod),
        fieldsize(Device_t, syncPeriod), (VOID*)TWL_RTC_INTERRUPTS_EVERY_DAY
    }, {
        L"EnableWake", PARAM_DWORD, FALSE, offset(Device_t, enableWake),
        fieldsize(Device_t, enableWake), (VOID*)1
    }, {
        L"SecureOffset", PARAM_DWORD, FALSE, offset(Device_t, secureOffset),
        fieldsize(Device_t, secureOffset), (VOID*)0
    },     
};

//------------------------------------------------------------------------------
//
//  Function:  SetPowerState
//
//  process the power state change request.
//
#include <notify.h>
BOOL SetPowerState(Device_t *pDevice, CEDEVICE_POWER_STATE dx)
{
    IOCTL_HAL_RTC_UPDATE_IN rtcUpdate;
    UINT8  intr = 0;
    UINT8 getTime = TWL_RTC_CTRL_GET_TIME | TWL_RTC_CTRL_RUN;

    DEBUGMSG(ZONE_FUNCTION, (L"+RTC:SetPowerState(0x%08x, %d)\r\n", pDevice, dx));
	EnterCriticalSection(&pDevice->cs);    
    // only process request is the power states are different
	if(pDevice->powerState != dx)
	{
        TWLWriteRegs(pDevice->hTWL, TWL_RTC_INTERRUPTS_REG, &intr, sizeof(intr));
        // always enable rtc timer interrupt
		intr = TWL_RTC_INTERRUPTS_IT_ALARM;
        
		switch(dx)
		{
            case D0:
            case D1:
            case D2:
			{
                // enable timer if we're not in suspend; always sync timer

				if(pDevice->powerState > D2)
				{
					RETAILMSG(0, (L"RTC:SetPowerState(D%d)->Get Current Time\r\n", dx));

					//  Set toggle bit and read the current time
					TWLWriteRegs(pDevice->hTWL, TWL_RTC_CTRL_REG, &getTime, sizeof(getTime));
	            
					StallExecution(100);

					//  Read current time
					TWLReadRegs(pDevice->hTWL, TWL_SECONDS_REG, rtcUpdate.time, sizeof(rtcUpdate.time));
					rtcUpdate.offset = 0;
					
					KernelIoControl(IOCTL_HAL_RTC_UPDATE, &rtcUpdate, sizeof(rtcUpdate), 0, 0, 0);
					//if(IsAPIReady(SH_WMGR))
					//	CeEventHasOccurred(NOTIFICATION_EVENT_TIME_CHANGE, 0);
				}

				intr |= TWL_RTC_INTERRUPTS_IT_TIMER; 
				intr |= (UINT8)pDevice->syncPeriod;

                break;
			}
		}
		RETAILMSG(0, (L"RTC:SetPowerState(D%d) Enable Interrupts %X\r\n", dx, intr));
        pDevice->powerState = dx;
        TWLWriteRegs(pDevice->hTWL, TWL_RTC_INTERRUPTS_REG, &intr, sizeof(intr));
	}
	LeaveCriticalSection(&pDevice->cs);
    DEBUGMSG(ZONE_FUNCTION, (L"-RTC:SetPowerState(0x%08x, %d)\r\n", pDevice, dx));

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  RTC_Init
//
//  Called by device manager to initialize device.
//
DWORD
RTC_Init(
    LPCWSTR szContext, 
    LPCVOID pBusContext
    )
{
    DWORD rc = (DWORD)NULL;
    Device_t *pDevice = NULL;
    UINT32 irq;
    UINT8 getTime = TWL_RTC_CTRL_GET_TIME | TWL_RTC_CTRL_RUN;
    IOCTL_HAL_RTC_UPDATE_IN rtcUpdate;
	UINT8 status,clrStatus;
	UINT8 empty[6] = {0};
    
    DEBUGMSG(ZONE_FUNCTION, (
        L"+RTC_Init(%s, 0x%08x)\r\n", szContext, pBusContext
        ));

    // Create device structure
    pDevice = (Device_t *)LocalAlloc(LPTR, sizeof(Device_t));
    if (pDevice == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: RTC_Init: "
            L"Failed allocate TWL controller structure\r\n"
            ));
        goto cleanUp;
        }

    // Set cookie
    pDevice->cookie = RTC_DEVICE_COOKIE;
    pDevice->powerState = D4;

    InitializeCriticalSection(&pDevice->cs);

    // Read device parameters
    if (GetDeviceRegistryParams(
            szContext, pDevice, dimof(s_deviceRegParams), s_deviceRegParams
            ) != ERROR_SUCCESS)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: RTC_Init: "
            L"Failed read TWL driver registry parameters\r\n"
            ));
        goto cleanUp;
        }

    // Open parent bus
    pDevice->hTWL = TWLOpen();
    if (pDevice->hTWL == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: RTC_Init: "
            L"Failed open TWL bus driver\r\n"
            ));
        goto cleanUp;
        }


    //  Set toggle bit and read the current time
    TWLWriteRegs(
        pDevice->hTWL, TWL_RTC_CTRL_REG, &getTime, sizeof(getTime)
        );
    
    StallExecution(100);

    //  Read current time
    TWLReadRegs(
        pDevice->hTWL, TWL_SECONDS_REG, rtcUpdate.time, sizeof(rtcUpdate.time)
        );

    DEBUGMSG(ZONE_FUNCTION, 
        (L"RTC_Init: time  day=%X %X, %02X  time=%02X:%02X:%02X\r\n",
        rtcUpdate.time[3], rtcUpdate.time[4], rtcUpdate.time[5], 
        rtcUpdate.time[2], rtcUpdate.time[1], rtcUpdate.time[0]
        ));


    // Sync RTC time and secure offset with the kernel
    rtcUpdate.offset = pDevice->secureOffset;

    KernelIoControl(IOCTL_HAL_RTC_UPDATE, &rtcUpdate, sizeof(rtcUpdate), 0, 0, 0);

	// Get status

    status = 0;
    TWLReadRegs(pDevice->hTWL, TWL_RTC_STATUS_REG, &status, sizeof(status));

        
    // Is interrupt caused by alarm?
    if((status & TWL_RTC_STATUS_ALARM) != 0)
    {
		clrStatus = TWL_RTC_STATUS_ALARM;
		TWLWriteRegs(pDevice->hTWL, TWL_RTC_STATUS_REG, &clrStatus, sizeof(clrStatus));
        RETAILMSG(1, (L"RTC init:  The device woked up by RTC alarm\r\n"));

		// Clear backup registers
		TWLWriteRegs(pDevice->hTWL, TWL_BACKUP_REG_A, empty, sizeof(empty));
		// Simply tell kernel, that ALARM interrupt occurs
        KernelIoControl(IOCTL_HAL_RTC_ALARM, 0, 0, 0, 0, 0);
    }



    // Create interrupt event
    pDevice->hIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (pDevice->hIntrEvent == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: RTC_Init: "
            L"Failed create interrupt event\r\n"
            ));
        goto cleanUp;
        }

    // Associate event with TWL RTC interrupt
    if (!TWLInterruptInitialize(pDevice->hTWL, TWL_INTR_RTC_IT, pDevice->hIntrEvent))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: RTC_Init: "
            L"Failed associate event with TWL RTC interrupt\r\n"
            ));
        goto cleanUp;
        }
  
    // Enable RTC event
    if (!TWLInterruptMask(pDevice->hTWL, TWL_INTR_RTC_IT, FALSE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: RTC_Init: "
            L"Failed associate event with TWL fake interrupt\r\n"
            ));
        }

    // Enable alarm wakeup
    if (pDevice->enableWake)
        {
        TWLWakeEnable(pDevice->hTWL, TWL_INTR_RTC_IT, TRUE);
        }

	 // Create RTC wakeup event
    pDevice->hRTCWakeupEvent = CreateEvent(NULL, FALSE, FALSE, _T("SYSTEM/RTC_Wakeup_Event"));
	if(!pDevice->hRTCWakeupEvent)
	{
        RETAILMSG(1, (L"ERROR: RTC_Init: " L"Failed create RTC wakeup event\r\n" ));
        goto cleanUp;
    }

   


    // Get SYSINTR for SW interrupt RTC Query
    irq = IRQ_SW_RTC_QUERY;

    if (!KernelIoControl(
        IOCTL_HAL_IRQ2SYSINTR, &irq, sizeof(irq), 
        &pDevice->sysIntrRTCQuery, sizeof(pDevice->sysIntrRTCQuery), NULL
    )) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: SPI_Init: "
            L"Failed map RTC Query controller interrupt\r\n"
        ));
        goto cleanUp;
    }

    // Create time/alarm sync event
    pDevice->hSyncEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (pDevice->hSyncEvent == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: RTC_Init: "
            L"Failed create sync event\r\n"
            ));
        goto cleanUp;
        }

    // Initialize interrupt
    if (!InterruptInitialize(
            pDevice->sysIntrRTCQuery, pDevice->hSyncEvent, NULL, 0
            ))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: RTC_Init: "
            L"InterruptInitialize failed!\r\n"
            ));
        goto cleanUp;
        }
    
    // Start interrupt service thread
    pDevice->threadsExit = FALSE;
    pDevice->hIntrThread = CreateThread(
        NULL, 0, RTC_IntrThread, pDevice, 0,NULL
        );
    if (!pDevice->hIntrThread)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: RTC_Init: "
            L"Failed create interrupt thread\r\n"
            ));
        goto cleanUp;
        }

    // Set thread priority
    CeSetThreadPriority(pDevice->hIntrThread, pDevice->priority256);

    // Start sync service thread
    pDevice->hSyncThread = CreateThread(
        NULL, 0, RTC_SyncThread, pDevice, 0,NULL
        );
    if (!pDevice->hSyncThread)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: RTC_Init: "
            L"Failed create interrupt thread\r\n"
            ));
        goto cleanUp;
        }

    // Set thread priority
    CeSetThreadPriority(pDevice->hSyncThread, pDevice->priority256);

    // update power state
    SetPowerState(pDevice, D0);

    // Return non-null value
    rc = (DWORD)pDevice;
    
cleanUp:
    if (rc == 0) RTC_Deinit((DWORD)pDevice);
    DEBUGMSG(ZONE_FUNCTION, (L"-RTC_Init(rc = %d\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  RTC_Deinit
//
//  Called by device manager to uninitialize device.
//
BOOL
RTC_Deinit(
    DWORD context
    )
{
    BOOL rc = FALSE;
    Device_t *pDevice = (Device_t*)context;


    DEBUGMSG(ZONE_FUNCTION, (L"+RTC_Deinit(0x%08x)\r\n", context));

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != RTC_DEVICE_COOKIE))
        {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: RTC_Deinit: "
            L"Incorrect context paramer\r\n"
            ));
        goto cleanUp;
        }

    // Signal stop to threads
    pDevice->threadsExit = TRUE;

    // Close interrupt thread
    if (pDevice->hIntrThread != NULL)
        {
        // Set event to wake it
        SetEvent(pDevice->hIntrEvent);
        // Wait until thread exits
        WaitForSingleObject(pDevice->hIntrThread, INFINITE);
        // Close handle
        CloseHandle(pDevice->hIntrThread);
        }

    // Close sync thread
    if (pDevice->hSyncThread != NULL)
        {
        // Set event to wake it
        SetEvent(pDevice->hSyncEvent);
        // Wait until thread exits
        WaitForSingleObject(pDevice->hSyncThread, INFINITE);
        // Close handle
        CloseHandle(pDevice->hSyncThread);
        }

    // Close TWL driver
    if (pDevice->hTWL != NULL)
        {
        TWLInterruptMask(pDevice->hTWL, TWL_INTR_RTC_IT, TRUE);
        if (pDevice->hIntrEvent != NULL)
            {
            TWLInterruptDisable(pDevice->hTWL, TWL_INTR_RTC_IT);
            }
        TWLClose(pDevice->hTWL);
        }

    if (pDevice->hIntrEvent != NULL)
        {
        CloseHandle(pDevice->hIntrEvent);
        }

	if (pDevice->hRTCWakeupEvent)
        {
        CloseHandle(pDevice->hRTCWakeupEvent);
		pDevice->hRTCWakeupEvent = NULL;
        }


    if (pDevice->hSyncEvent != NULL)
        {
        InterruptDisable(pDevice->sysIntrRTCQuery);
        CloseHandle(pDevice->hSyncEvent); 
        }
    

    DeleteCriticalSection(&pDevice->cs);

    // Free device structure
    LocalFree(pDevice);

    // Done
    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-TWL_Deinit(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  TWL_Open
//
//  Called by device manager to open a device for reading and/or writing.
//
DWORD
RTC_Open(
    DWORD context, 
    DWORD accessCode, 
    DWORD shareMode
    )
{
    return context;
}

//------------------------------------------------------------------------------
//
//  Function:  RTC_Close
//
//  This function closes the device context.
//
BOOL
RTC_Close(
    DWORD context
    )
{
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  RTC_IOControl
//
//  This function sends a command to a device.
//
BOOL
RTC_IOControl(
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
    Device_t *pDevice = (Device_t*)context;

    DEBUGMSG(ZONE_FUNCTION, (
        L"+RTC_IOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",
        context, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize
        ));
        
    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != RTC_DEVICE_COOKIE))
        {
        RETAILMSG(ZONE_ERROR, (L"ERROR: RTC_IOControl: "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }
    
	switch(code)
	{
		case IOCTL_RTC_GETTIME:
		{
#define BCD2BIN(b)              (((b) >> 4)*10 + ((b)&0xF))
			UINT8 clrStatus;
			IOCTL_HAL_RTC_UPDATE_IN rtcUpdate;
			UINT8 getTime = TWL_RTC_CTRL_GET_TIME | TWL_RTC_CTRL_RUN;
			SYSTEMTIME *SystemTime;

            if(pOutBuffer && outSize >= sizeof(SYSTEMTIME)) 
			{
				SystemTime = (SYSTEMTIME *)pOutBuffer;
				EnterCriticalSection(&pDevice->cs);

				// Disable the RTC interrupts (this also clears the periodic timer interrupt status)
				clrStatus = 0;
				TWLWriteRegs(pDevice->hTWL, TWL_RTC_INTERRUPTS_REG, &clrStatus, sizeof(clrStatus));

				//  Set toggle bit and read the current time
				TWLWriteRegs(pDevice->hTWL, TWL_RTC_CTRL_REG, &getTime, sizeof(getTime));
	            
				StallExecution(100);

				//  Read current time
				TWLReadRegs(pDevice->hTWL, TWL_SECONDS_REG, rtcUpdate.time, sizeof(rtcUpdate.time));

				// Re-enable the RTC interrupts
				clrStatus = TWL_RTC_INTERRUPTS_IT_ALARM | TWL_RTC_INTERRUPTS_IT_TIMER | (UINT8)pDevice->syncPeriod;
				TWLWriteRegs(pDevice->hTWL, TWL_RTC_INTERRUPTS_REG, &clrStatus, sizeof(clrStatus));
		    
				LeaveCriticalSection(&pDevice->cs);

				SystemTime->wYear    = BCD2BIN(rtcUpdate.time[5]) + 2000;
				SystemTime->wMonth   = BCD2BIN(rtcUpdate.time[4]);
				SystemTime->wDay     = BCD2BIN(rtcUpdate.time[3]);
				SystemTime->wHour    = BCD2BIN(rtcUpdate.time[2]);
				SystemTime->wMinute  = BCD2BIN(rtcUpdate.time[1]);
				SystemTime->wSecond  = BCD2BIN(rtcUpdate.time[0]);
				SystemTime->wMilliseconds = 0;

				rc = 1;
			}
			break;
		}
        case IOCTL_POWER_CAPABILITIES: 
            DEBUGMSG(ZONE_INFO, (L"RTC: Received IOCTL_POWER_CAPABILITIES\r\n"));
            if (pOutBuffer && outSize >= sizeof (POWER_CAPABILITIES) && 
                pOutSize) 
                {
                    __try 
                        {
                        PPOWER_CAPABILITIES PowerCaps;
                        PowerCaps = (PPOWER_CAPABILITIES)pOutBuffer;
         
                        // Only supports D0 (permanently on) and D4(off).         
                        memset(PowerCaps, 0, sizeof(*PowerCaps));
                        PowerCaps->DeviceDx = DX_MASK(D0) | DX_MASK(D1)|DX_MASK(D2)|DX_MASK(D3)|DX_MASK(D4);
                        *pOutSize = sizeof(*PowerCaps);
                        
                        rc = TRUE;
                        }
                    __except(EXCEPTION_EXECUTE_HANDLER) 
                        {
                        RETAILMSG(ZONE_ERROR, (L"exception in ioctl\r\n"));
                        }
                }
            break;

        // determines whether changing power state is feasible
        case IOCTL_POWER_QUERY: 
            DEBUGMSG(ZONE_INFO,(L"RTC: Received IOCTL_POWER_QUERY\r\n"));
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
 
                    DEBUGMSG(ZONE_INFO, (L"RTC: IOCTL_POWER_QUERY %d\r\n"));
                    }
                __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
                    RETAILMSG(ZONE_ERROR, (L"Exception in ioctl\r\n"));
                    }
                }
            break;

        // requests a change from one device power state to another
        case IOCTL_POWER_SET: 
            DEBUGMSG(ZONE_INFO,(L"RTC: Received IOCTL_POWER_SET\r\n"));
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
                        DEBUGMSG(ZONE_INFO, (L"RTC: "
                            L"IOCTL_POWER_SET to D%u \r\n",
                            pDevice->powerState
                            ));
                        }
                    else 
                        {
                        RETAILMSG(ZONE_ERROR, (L"RTC: "
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
            RETAILMSG(ZONE_INFO, (L"RTC: Received IOCTL_POWER_GET\r\n"));
            if (pOutBuffer != NULL && outSize >= sizeof(CEDEVICE_POWER_STATE)) 
                {
                __try 
                    {
                    *(PCEDEVICE_POWER_STATE)pOutBuffer = pDevice->powerState;
 
                    rc = TRUE;

                    DEBUGMSG(ZONE_INFO, (L"RTC: "
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
        }

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-RTC_IOControl(rc = %d)\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  RTC_PowerUp
//
//  Called on resume of device. 
//
void
RTC_PowerUp(
    DWORD context
    )
{
    Device_t *pDevice = (Device_t*)context;
	UINT8           data;
    DWORD           evData, rtcWakeup = 0;
	UINT8           status = 0;

	TWLReadRegs(pDevice->hTWL, TWL_PWR_ISR1, &data, 1);
	if(data & STARTON_RTC)
	{
		// Get status
        status = 0;
        TWLReadRegs(pDevice->hTWL, TWL_RTC_STATUS_REG, &status, sizeof(status));
        TWLReadRegs(pDevice->hTWL, TWL_RTC_INTERRUPTS_REG, &rtcWakeup, sizeof(rtcWakeup));
		//  Is interrupt caused by time update?
		if((rtcWakeup & TWL_RTC_INTERRUPTS_IT_ALARM) && (status & TWL_RTC_STATUS_ALARM))
			evData = 0;
		else // if((rtcWakeup & TWL_RTC_INTERRUPTS_IT_TIMER) && (status & TWL_RTC_STATUS_ALARM))
			evData = 1;

		if(rtcWakeup & (TWL_RTC_INTERRUPTS_IT_ALARM | TWL_RTC_INTERRUPTS_IT_TIMER))
		{
			SetEventData(pDevice->hRTCWakeupEvent, evData);
			CeSetPowerOnEvent(pDevice->hRTCWakeupEvent);
		}
	}
	RETAILMSG(1, (L"RTC_PowerUp intr status = %x, RTC status = %x, evData = %x\r\n", data, status, evData));
}



//------------------------------------------------------------------------------
//
//  Function:  RTC_PowerDown
//
//  Called on suspend of device.
//
void
RTC_PowerDown(
    DWORD context
    )
{
//    Device_t *pDevice = (Device_t*)context;
//    SetPowerState(pDevice, D3);
}

//------------------------------------------------------------------------------
//
//  Function:  RTC_IntrThread
//
//  This function acts as the IST for the RTC.
//
DWORD RTC_IntrThread(VOID *pContext)
{
    Device_t *pDevice = (Device_t*)pContext;
    DWORD timeout = INFINITE;
    IOCTL_HAL_RTC_UPDATE_IN rtcUpdate;
    UINT8 status;
    UINT8 clrStatus = 0;
    UINT8 getTime = TWL_RTC_CTRL_GET_TIME | TWL_RTC_CTRL_RUN;
	UINT8 empty[6] = {0};

    // Loop until we are not stopped...
    while (!pDevice->threadsExit)
        {
        // Wait for event
        WaitForSingleObject(pDevice->hIntrEvent, timeout);
        if (pDevice->threadsExit)
			break;

		EnterCriticalSection(&pDevice->cs);

        // Get status
        status = 0;
        TWLReadRegs(pDevice->hTWL, TWL_RTC_STATUS_REG, &status, sizeof(status));

        DEBUGMSG(ZONE_INT_THREAD, (L"RTC: interrupt: status=%02X\r\n", status));


        // Disable the RTC interrupts (this also clears the periodic timer interrupt status)
        clrStatus = 0;
        TWLWriteRegs(pDevice->hTWL, TWL_RTC_INTERRUPTS_REG, &clrStatus, sizeof(clrStatus));


        // Clear alarm status (regardless if set or not)
        clrStatus = TWL_RTC_STATUS_ALARM;
        TWLWriteRegs(pDevice->hTWL, TWL_RTC_STATUS_REG, &clrStatus, sizeof(clrStatus));

        
        // Is interrupt caused by alarm?
		if((status & TWL_RTC_STATUS_ALARM) != 0)
		{
            RETAILMSG(0, (L"RTC: alarm interrupt\r\n"));

			TWLWriteRegs(pDevice->hTWL, TWL_BACKUP_REG_A, empty, sizeof(empty));
            // Simply tell kernel, that ALARM interrupt occurs
            KernelIoControl(IOCTL_HAL_RTC_ALARM, NULL, 0, NULL, 0, NULL);
		}

        
        //  Is interrupt caused by time update?
        if ((status & TWM_RTC_STATUS_TIME_EVENT) != 0)
            {
            //  Set toggle bit and read the current time
            TWLWriteRegs(
                pDevice->hTWL, TWL_RTC_CTRL_REG, &getTime, sizeof(getTime)
                );
            
            StallExecution(100);

            //  Read current time
            TWLReadRegs(
                pDevice->hTWL, TWL_SECONDS_REG, rtcUpdate.time, sizeof(rtcUpdate.time)
                );

            DEBUGMSG(ZONE_INT_THREAD, 
                (L"RTC: time event  day=%X %X, %02X  time=%02X:%02X:%02X\r\n",
                rtcUpdate.time[3], rtcUpdate.time[4], rtcUpdate.time[5], 
                rtcUpdate.time[2], rtcUpdate.time[1], rtcUpdate.time[0]
                ));

            //  No change in offset
            rtcUpdate.offset = 0;

            //  Match 
            switch ((pDevice->syncPeriod) & TWL_RTC_INTERRUPTS_EVERY_DAY)
                {
                case TWL_RTC_INTERRUPTS_EVERY_SECOND:
                    // Each second
                    if(status & TWL_RTC_STATUS_ONE_S_EVENT)
                        {
                        KernelIoControl(
                            IOCTL_HAL_RTC_UPDATE, &rtcUpdate, sizeof(rtcUpdate),
                            NULL, 0, NULL
                            );
                        }
                    break;
                case TWL_RTC_INTERRUPTS_EVERY_MINUTE:
                    // Each minute
                    if (status & TWL_RTC_STATUS_ONE_M_EVENT)
                        {
                        KernelIoControl(
                            IOCTL_HAL_RTC_UPDATE, &rtcUpdate, sizeof(rtcUpdate), 
                            NULL, 0, NULL
                            );
                        }
                    break;
                case TWL_RTC_INTERRUPTS_EVERY_HOUR:
                    // Each hour
                    if (status & TWL_RTC_STATUS_ONE_H_EVENT)
                        {
                        KernelIoControl(
                            IOCTL_HAL_RTC_UPDATE, &rtcUpdate, sizeof(rtcUpdate), 
                            NULL, 0, NULL
                            );
                        }
                    break;
                case TWL_RTC_INTERRUPTS_EVERY_DAY:
                    // Each day
                    if (status & TWL_RTC_STATUS_ONE_D_EVENT)
                        {
                        KernelIoControl(
                            IOCTL_HAL_RTC_UPDATE, &rtcUpdate, sizeof(rtcUpdate), 
                            NULL, 0, NULL
                            );
                        }
                    break;

                default:
                    break;
                }
            }
        // Re-enable the RTC interrupts
        status = TWL_RTC_INTERRUPTS_IT_ALARM;
		if(pDevice->powerState < D3)
			status |= TWL_RTC_INTERRUPTS_IT_TIMER | (UINT8)pDevice->syncPeriod;
        TWLWriteRegs(pDevice->hTWL, TWL_RTC_INTERRUPTS_REG, &status, sizeof(status));
    
        LeaveCriticalSection(&pDevice->cs);
        }

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  RTC_SyncThread
//
DWORD RTC_SyncThread(VOID *pContext)
{
    Device_t *pDevice = (Device_t*)pContext;
    IOCTL_HAL_RTC_QUERY_OUT queryOut;
    UINT8 getTime = TWL_RTC_CTRL_GET_TIME | TWL_RTC_CTRL_RUN;

    // Loop until we are not stopped...
    while(!pDevice->threadsExit)
	{
        // Wait for event
        WaitForSingleObject(pDevice->hSyncEvent, INFINITE);
        if(pDevice->threadsExit)
			break;

        EnterCriticalSection(&pDevice->cs);

        // Query kernel for time change or alarm set
		if(KernelIoControl(IOCTL_HAL_RTC_QUERY, NULL, 0, &queryOut, sizeof(queryOut), NULL))
		{
			UINT8 status;

			status = 0;
			TWLWriteRegs(pDevice->hTWL, TWL_RTC_INTERRUPTS_REG, &status, sizeof(status));

            DEBUGMSG(ZONE_SYNC_THREAD, 
                (L"RTC: sync event %x\r\n", queryOut.flags));


            //  Check for set time event
            if ((queryOut.flags & IOCTL_HAL_RTC_QUERY_SET_TIME) != 0)
                {
                RETAILMSG(0, 
                    (L"RTC: time sync  day=%X %X, %02X  time=%02X:%02X:%02X\r\n",
                    queryOut.time[3], queryOut.time[4], queryOut.time[5], 
                    queryOut.time[2], queryOut.time[1], queryOut.time[0]
                    ));

                //  Write time registers
                TWLWriteRegs(
                    pDevice->hTWL, TWL_SECONDS_REG, queryOut.time, 
                    sizeof(queryOut.time)
                    );


                //  Set toggle bit to latch time registers
                TWLWriteRegs(pDevice->hTWL, TWL_RTC_CTRL_REG, &getTime, sizeof(getTime));
                }


            //  Check for set alarm event
            if ((queryOut.flags & IOCTL_HAL_RTC_QUERY_SET_ALARM) != 0)
                {
                RETAILMSG(0, 
                    (L"RTC: alarm set  day=%X %X, %02X  time=%02X:%02X:%02X\r\n",
                    queryOut.alarm[3], queryOut.alarm[4], queryOut.alarm[5], 
                    queryOut.alarm[2], queryOut.alarm[1], queryOut.alarm[0]
                    ));

                //  Write alarm registers
                TWLWriteRegs(
                    pDevice->hTWL, TWL_ALARM_SECONDS_REG, queryOut.alarm, 
                    sizeof(queryOut.alarm)
                    );

				TWLWriteRegs(pDevice->hTWL, TWL_BACKUP_REG_A, queryOut.alarm, sizeof(queryOut.alarm));
                //  Set toggle bit to latch time registers
                TWLWriteRegs(pDevice->hTWL, TWL_RTC_CTRL_REG, &getTime, sizeof(getTime));

                }

			status = TWL_RTC_INTERRUPTS_IT_ALARM;
			if(pDevice->powerState < D3)
				status |= TWL_RTC_INTERRUPTS_IT_TIMER | (UINT8)pDevice->syncPeriod;

			TWLWriteRegs(pDevice->hTWL, TWL_RTC_INTERRUPTS_REG, &status, sizeof(status));
		}

        LeaveCriticalSection(&pDevice->cs);
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

