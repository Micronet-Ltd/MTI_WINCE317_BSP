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
//  File:  rtc.c
//
//  This file implements OAL real time module. 
//
//  Implementation uses system tick to calculate realtime clock. Tritons
//  chip driver should call IOCTL_HAL_INIT_RTC periodically to synchronize
//  internal RTC time with real one.
//
//
// Copyright (c) 2009, Micronet Ltd. All rights reserved.
// Vladimir:
// Checked, fixed and updated
//

#include <windows.h>
//#include <nkexport.h>
#include <nkintr.h>
#include <bsp.h>
#include <bsp_tps659xx.h>
#include <oal_prcm.h>

#if BSP_OAL_RTC_USES_TPS659XX

//------------------------------------------------------------------------------
//
//  Define:  RTC_BASE_YEAR
//
//  Delta from which RTC counts years
//  Resolution of RTC years is from 2000 to 2099
//
#define RTC_BASE_YEAR_MIN       2000
#define RTC_BASE_YEAR_MAX       2099


//------------------------------------------------------------------------------

#define BCD2BIN(b)              (((b) >> 4)*10 + ((b)&0xF))
#define BIN2BCD(b)              ((((UINT8)(b)/10) << 4)|((UINT8)(b)%10))

//------------------------------------------------------------------------------
static struct
{
    BOOL initialized;           // Is RTC subsystem intialized?
    CRITICAL_SECTION cs;
    void*      hTWL;            // Triton driver
    UINT32     sysIntRTC;
    UINT32     updateFlag;      // Inidicates the update type 
    
    ULONGLONG  baseFiletime;    // Base filetime 
    ULONGLONG  baseOffset;      // Secure time offset from base filetime
    DWORD      baseTickCount;   // Tick count from base filetime

    ULONGLONG  alarmFiletime;   // Alarm filetime 

}s_rtc = {0};

//------------------------------------------------------------------------------
UINT32 OEMGetTickCount();

BOOL FiletimeToHWTime(ULONGLONG fileTime, UCHAR bcdTime[6]);

//------------------------------------------------------------------------------
#if 1
static UINT32 LocalGetTickCount()
{
    return OEMGetTickCount();
}
#else
// TODO: change this function and where used to return 64 bit second count
// TODO: add support for rollover detection (add 2^^32)
static UINT32 LocalGetTickCount()
{
	DWORD RegValue[2];
    OMAP_32KSYNCNT_REGS *p32KSynCntRegs = (OMAP_32KSYNCNT_REGS *)OALPAtoUA(OMAP_32KSYNC_REGS_PA);

	do
	{
		RegValue[0] = p32KSynCntRegs->CR;
		RegValue[1] = p32KSynCntRegs->CR;
	} while (RegValue[0] != RegValue[1]);

    // CR / 32768 = number of seconds, times 1000 gives number of milliseconds
    return (RegValue[0] >> 15) * 1000;
}
#endif

//------------------------------------------------------------------------------
LPCWSTR SystemTimeToString(SYSTEMTIME *pSystemTime)
{
    static WCHAR buffer[64];

    OALLogPrintf(
        buffer, 64, L"%04d.%02d.%02d %02d:%02d:%02d.%03d",
        pSystemTime->wYear, pSystemTime->wMonth, pSystemTime->wDay,
        pSystemTime->wHour, pSystemTime->wMinute, pSystemTime->wSecond, 
        pSystemTime->wMilliseconds
        );        
    return buffer;
}

//------------------------------------------------------------------------------

LPCWSTR HWTimeToString(UCHAR bcdTime[6])
{
    static WCHAR buffer[64];

    OALLogPrintf(
        buffer, 64, L"%04d.%02d.%02d %02d:%02d:%02d",
        BCD2BIN(bcdTime[5]) + RTC_BASE_YEAR_MIN, 
        BCD2BIN(bcdTime[4]), 
        BCD2BIN(bcdTime[3]),
        BCD2BIN(bcdTime[2]), 
        BCD2BIN(bcdTime[1]), 
        BCD2BIN(bcdTime[0])
        );        
    return buffer;
}

//------------------------------------------------------------------------------

VOID
ReadBaseOffset(
    ULONGLONG   *pOffset
    )
{
    UCHAR   val;

    // Read backup registers for secure time offset
    *pOffset = 0;

    OALTritonRead(s_rtc.hTWL, TWL_BACKUP_REG_H, &val);
    *pOffset = (*pOffset << 8) | val;

    OALTritonRead(s_rtc.hTWL, TWL_BACKUP_REG_G, &val);
    *pOffset = (*pOffset << 8) | val;

    OALTritonRead(s_rtc.hTWL, TWL_BACKUP_REG_F, &val);
    *pOffset = (*pOffset << 8) | val;

    OALTritonRead(s_rtc.hTWL, TWL_BACKUP_REG_E, &val);
    *pOffset = (*pOffset << 8) | val;

    OALTritonRead(s_rtc.hTWL, TWL_BACKUP_REG_D, &val);
    *pOffset = (*pOffset << 8) | val;

    OALTritonRead(s_rtc.hTWL, TWL_BACKUP_REG_C, &val);
    *pOffset = (*pOffset << 8) | val;

    OALTritonRead(s_rtc.hTWL, TWL_BACKUP_REG_B, &val);
    *pOffset = (*pOffset << 8) | val;

    OALTritonRead(s_rtc.hTWL, TWL_BACKUP_REG_A, &val);
    *pOffset = (*pOffset << 8) | val;
}

//------------------------------------------------------------------------------

VOID
WriteBaseOffset(
    ULONGLONG   *pOffset
    )
{
    UCHAR   val;

    // Write backup registers with secure time offset
    val = (UCHAR)(*pOffset >> 0);
    OALTritonWrite(s_rtc.hTWL, TWL_BACKUP_REG_A, val);

    val = (UCHAR)(*pOffset >> 8);
    OALTritonWrite(s_rtc.hTWL, TWL_BACKUP_REG_B, val);

    val = (UCHAR)(*pOffset >> 16);
    OALTritonWrite(s_rtc.hTWL, TWL_BACKUP_REG_C, val);

    val = (UCHAR)(*pOffset >> 24);
    OALTritonWrite(s_rtc.hTWL, TWL_BACKUP_REG_D, val);

    val = (UCHAR)(*pOffset >> 32);
    OALTritonWrite(s_rtc.hTWL, TWL_BACKUP_REG_E, val);

    val = (UCHAR)(*pOffset >> 40);
    OALTritonWrite(s_rtc.hTWL, TWL_BACKUP_REG_F, val);

    val = (UCHAR)(*pOffset >> 48);
    OALTritonWrite(s_rtc.hTWL, TWL_BACKUP_REG_G, val);

    val = (UCHAR)(*pOffset >> 56);
    OALTritonWrite(s_rtc.hTWL, TWL_BACKUP_REG_H, val);
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalInitRTC
//
//  This function is called by WinCE OS to initialize the time after boot. 
//  Input buffer contains SYSTEMTIME structure with default time value.
//
//
BOOL OALIoCtlHalInitRTC(UINT32 code, VOID *pInBuffer, UINT32 inSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL            rc = FALSE;
    SYSTEMTIME      *pGivenTime = (LPSYSTEMTIME) pInBuffer;
    UCHAR           bcdTime[6];
    UCHAR           status;
    UCHAR           secure;
    UINT32          irq;
//    OMAP_GPIO_REGS *pGPIO1Regs = OALPAtoUA(OMAP_GPIO1_REGS_PA);
//	OMAP_SYSC_PADCONFS_REGS *pPADCONFS_REGS = OALPAtoUA(OMAP_SYSC_PADCONFS_REGS_PA);

    OALMSG(OAL_TIMER && OAL_FUNC, (L"+OALIoCtlHalInitRTC()\r\n"));

	if(s_rtc.initialized)
		return 1;
   
    // Initialize RTC critical section
    InitializeCriticalSection(&s_rtc.cs);

	EnterCriticalSection(&s_rtc.cs);

#if 0
	// TO DO:
	// this code depoends from platform; micronet platforms don't need it because MSEQURE pin is tied
	// to pull-up
    // Set GPIO_22(MSECURE) to be output/high (unsecure)
    // This allows write access to the T2 RTC calendar/time registers
    // OMAP35XX GP only
    if(dwOEMHighSecurity == OEM_HIGH_SECURITY_GP)
    {
        // SYS_DRM_MSEC signal = GPIO_22 on ETK_D8
        #define MODIFY_PADCONF_REG_LOWER(name, value) {pPADCONFS_REGS->name = (pPADCONFS_REGS->name & 0xffff0000) | (value);}
        MODIFY_PADCONF_REG_LOWER(CONTROL_PADCONF_ETK_D8, MUX_MODE_4 | INPUT_ENABLE)
        CLRREG32(&pGPIO1Regs->OE, 1 << 22);
        SETREG32(&pGPIO1Regs->DATAOUT, 1 << 22);
    }
#endif
    // Request SYSINTR for RTC Query SW interrupt
   	irq = IRQ_SW_RTC_QUERY;

    s_rtc.sysIntRTC = OALIntrRequestSysIntr(1, &irq, OAL_INTR_TRANSLATE);
   	if(s_rtc.sysIntRTC == SYSINTR_UNDEFINED)
	{
       	OALMSG(OAL_ERROR, (L" OALIoCtlHalInitRTC(): Failed to get RTC SysIntr\r\n"));
	}
	OALMSG(0, (L"%S: RTC sys intr %d(%d)\r\n", __FUNCTION__, s_rtc.sysIntRTC, irq));

    // First read RTC status from Triton 
    s_rtc.hTWL = OALTritonOpen();
    if(s_rtc.hTWL)
	{
		status = 0;

		// Check for a RTC power lost
		OALTritonRead(s_rtc.hTWL, TWL_RTC_STATUS_REG, &secure);
		if(secure & TWL_RTC_STATUS_POWER_UP)
			status = 1;
		else
			status = 0;

		OALMSG(1, (L" OALIoCtlHalInitRTC():  RTC power is%slost\r\n", (status)?L" ":L"n't "));

		// Start RTC when it isn't running
		if(status == 1 && pGivenTime != NULL)
		{
			OALMSG(OAL_TIMER && OAL_FUNC, (L" OALIoCtlHalInitRTC():  Resetting RTC\r\n"));

			// Write power_up and alarm bits to clear power up flag (and any interrupt flag)
			OALTritonWrite(s_rtc.hTWL, TWL_RTC_STATUS_REG, TWL_RTC_STATUS_POWER_UP|TWL_RTC_STATUS_ALARM);

			//  Convert system time to BCD
			bcdTime[5] = BIN2BCD(pGivenTime->wYear - RTC_BASE_YEAR_MIN);
			bcdTime[4] = BIN2BCD(pGivenTime->wMonth);
			bcdTime[3] = BIN2BCD(pGivenTime->wDay);
			bcdTime[2] = BIN2BCD(pGivenTime->wHour);
			bcdTime[1] = BIN2BCD(pGivenTime->wMinute);
			bcdTime[0] = BIN2BCD(pGivenTime->wSecond);

			//  Initialize RTC with given values
			OALTritonWrite(s_rtc.hTWL, TWL_YEARS_REG, bcdTime[5]);
			OALTritonWrite(s_rtc.hTWL, TWL_MONTHS_REG, bcdTime[4]);
			OALTritonWrite(s_rtc.hTWL, TWL_DAYS_REG, bcdTime[3]);
			OALTritonWrite(s_rtc.hTWL, TWL_HOURS_REG, bcdTime[2]);
			OALTritonWrite(s_rtc.hTWL, TWL_MINUTES_REG, bcdTime[1]);
			OALTritonWrite(s_rtc.hTWL, TWL_SECONDS_REG, bcdTime[0]);

			//  Enable RTC
			OALTritonWrite(s_rtc.hTWL, TWL_RTC_CTRL_REG, TWL_RTC_CTRL_RUN);

			//  Convert given time initialization date/time to FILETIME
			NKSystemTimeToFileTime(pGivenTime, (FILETIME*)&s_rtc.baseFiletime);

			//  Set a value for base offset (100 nanoseconds won't make a difference)
			//  to ensure that the OS time is reset to a default and offset stored
			//  in registry is ignored
			s_rtc.baseOffset = 1;
        	//  Save off base offset to the backup regs
        	//WriteBaseOffset( &s_rtc.baseOffset );
			OALTritonWrite(s_rtc.hTWL, TWL_BACKUP_REG_A, 0);
			OALTritonWrite(s_rtc.hTWL, TWL_BACKUP_REG_B, 0);
			OALTritonWrite(s_rtc.hTWL, TWL_BACKUP_REG_C, 0);
			OALTritonWrite(s_rtc.hTWL, TWL_BACKUP_REG_D, 0);
			OALTritonWrite(s_rtc.hTWL, TWL_BACKUP_REG_E, 0);
			OALTritonWrite(s_rtc.hTWL, TWL_BACKUP_REG_F, 0);
		}
		else
		{
			SYSTEMTIME  baseSystemTime;

			OALMSG(OAL_TIMER && OAL_FUNC, (L" OALIoCtlHalInitRTC():  Getting RTC\r\n"));

			//  Set get time flag            
			OALTritonRead(s_rtc.hTWL, TWL_RTC_CTRL_REG, &status);

			status |= TWL_RTC_CTRL_RUN | TWL_RTC_CTRL_GET_TIME;
			OALTritonWrite(s_rtc.hTWL, TWL_RTC_CTRL_REG, status);

			//  Get date and time;
			OALTritonRead(s_rtc.hTWL, TWL_YEARS_REG, &bcdTime[5]);
			OALTritonRead(s_rtc.hTWL, TWL_MONTHS_REG, &bcdTime[4]);
			OALTritonRead(s_rtc.hTWL, TWL_DAYS_REG, &bcdTime[3]);
			OALTritonRead(s_rtc.hTWL, TWL_HOURS_REG, &bcdTime[2]);
			OALTritonRead(s_rtc.hTWL, TWL_MINUTES_REG, &bcdTime[1]);
			OALTritonRead(s_rtc.hTWL, TWL_SECONDS_REG, &bcdTime[0]);

			//  Convert current RTC date/time to FILETIME
			baseSystemTime.wYear    = BCD2BIN(bcdTime[5]) + RTC_BASE_YEAR_MIN;
			baseSystemTime.wMonth   = BCD2BIN(bcdTime[4]);
			baseSystemTime.wDay     = BCD2BIN(bcdTime[3]);
			baseSystemTime.wHour    = BCD2BIN(bcdTime[2]);
			baseSystemTime.wMinute  = BCD2BIN(bcdTime[1]);
			baseSystemTime.wSecond  = BCD2BIN(bcdTime[0]);
			baseSystemTime.wMilliseconds = 0;

			OALMSG(0, (L" OALIoCtlHalInitRTC():  Convert to FILETIME %d/%d/%d %d.%d.%d\r\n",
				baseSystemTime.wYear,
				baseSystemTime.wMonth,
				baseSystemTime.wDay,
				baseSystemTime.wHour,
				baseSystemTime.wMinute,
				baseSystemTime.wSecond));
			NKSystemTimeToFileTime(&baseSystemTime, (FILETIME*)&s_rtc.baseFiletime);
			if(pGivenTime)
				NKFileTimeToSystemTime((FILETIME*)&s_rtc.baseFiletime,pGivenTime);

			//  Read the offset from the backup regs
			// ReadBaseOffset( &s_rtc.baseOffset ); 
			//  Reset base offset to an overwriteable value
			//  The RTC driver will set this value from registery
			s_rtc.baseOffset = 0;
		}        

		// We are done with Triton
		OALTritonClose(s_rtc.hTWL);
	}
	else
	{
		//  Convert given time initialization date/time to FILETIME
		if(pGivenTime)
			NKSystemTimeToFileTime(pGivenTime, (FILETIME*)&s_rtc.baseFiletime);

		//  Set a value for base offset (100 nanoseconds won't make a difference)
		//  to ensure that the OS time is reset to a default and offset stored
		//  in registry is ignored
		s_rtc.baseOffset = 1;
        OALMSG(OAL_ERROR, (L" OALIoCtlHalInitRTC(): Failed to open Triton\r\n"));
	}

    OALMSG(OAL_TIMER && OAL_FUNC, (L" OALIoCtlHalInitRTC():  RTC = %s\r\n", HWTimeToString(bcdTime)));


    // Now update RTC state values
    s_rtc.initialized   = TRUE;
    s_rtc.baseTickCount = LocalGetTickCount();


    //  Success
    rc = TRUE;
	LeaveCriticalSection(&s_rtc.cs);

    OALMSG(OAL_TIMER && OAL_FUNC, (L"-OALIoCtlHalInitRTC() rc = %d\r\n", rc));

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OEMGetRealTime
//
//  This function is called by the kernel to retrieve the time from
//  the real-time clock.
//
BOOL OEMGetRealTime(SYSTEMTIME *pSystemTime) 
{
    DWORD       delta;
    ULONGLONG   time;

    OALMSG(OAL_TIMER && OAL_FUNC, (L"+OEMGetRealTime()\r\n"));

    if(!s_rtc.initialized)
	{
        // Return default time if RTC isn't initialized
        pSystemTime->wYear   = RTC_BASE_YEAR_MIN;
        pSystemTime->wMonth  = 1;
        pSystemTime->wDay    = 1;
        pSystemTime->wHour   = 0;
        pSystemTime->wMinute = 0;
        pSystemTime->wSecond = 0;
        pSystemTime->wDayOfWeek    = 0;
        pSystemTime->wMilliseconds = 0;
    }
    else
	{
        EnterCriticalSection(&s_rtc.cs);
        delta = LocalGetTickCount() - s_rtc.baseTickCount;
        // Note: baseFiletime and baseOffset are in 100ns units, delta is in 1ms units.
        time = s_rtc.baseFiletime + s_rtc.baseOffset + ((ULONGLONG)delta) * 10000;
        NKFileTimeToSystemTime((FILETIME*)&time, pSystemTime);
        // don't return milliseconds, causes CETK failure
        pSystemTime->wMilliseconds = 0;
        LeaveCriticalSection(&s_rtc.cs);
	}

    OALMSG(OAL_TIMER && OAL_FUNC, (L"-OEMGetRealTime() = %s\r\n", SystemTimeToString(pSystemTime)));

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMSetRealTime
//
//  This function is called by the kernel to set the real-time clock. A secure
//  timer requirement means that the time change is noted in baseOffset and
//  used to compute the time delta from the non-alterable RTC in T2
//
BOOL OEMSetRealTime(SYSTEMTIME *pSystemTime) 
{
    BOOL        rc = FALSE;
    ULONGLONG   fileTime;
    DWORD       tickDelta;

    OALMSG(OAL_TIMER && OAL_FUNC, (L"+OEMSetRealTime(%s)\r\n", SystemTimeToString(pSystemTime)));

    if(s_rtc.initialized)
	{
        // Save time to global structure
        EnterCriticalSection(&s_rtc.cs);

        // Discard milliseconds
        pSystemTime->wMilliseconds = 0;

        // Convert to filetime
        if(NKSystemTimeToFileTime(pSystemTime, (FILETIME*)&fileTime))
		{
            // Compute the tick delta (indicates the time in the RTC)
            tickDelta = LocalGetTickCount() - s_rtc.baseTickCount;
            
            // Update all the parameters
            s_rtc.baseFiletime  = s_rtc.baseFiletime + ((ULONGLONG)tickDelta)*10000;
            s_rtc.baseOffset    = fileTime - s_rtc.baseFiletime;
            s_rtc.baseTickCount = LocalGetTickCount();

            s_rtc.updateFlag |= IOCTL_HAL_RTC_QUERY_SET_TIME;

   			if(s_rtc.sysIntRTC != SYSINTR_UNDEFINED)
				NKSetInterruptEvent(s_rtc.sysIntRTC);

            //  Save off base offset to the backup regs
            //WriteBaseOffset( &s_rtc.baseOffset ); 
            // Done
            rc = TRUE;
		}

        LeaveCriticalSection(&s_rtc.cs);
	}
    
    OALMSG(OAL_TIMER && OAL_FUNC, (L"-OEMSetRealTime\r\n"));

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMSetAlarmTime
//
//  This function is called by the kernel to set the real-time clock alarm.
//
BOOL OEMSetAlarmTime(SYSTEMTIME *pSystemTime) 
{
    BOOL rc = FALSE;

    OALMSGS(0, (L"OEMSetAlarmTime(%s)\r\n", SystemTimeToString(pSystemTime)));

    if(s_rtc.initialized)
	{
        // Save time to global structure
        EnterCriticalSection(&s_rtc.cs);

        // Round to seconds
        pSystemTime->wMilliseconds = 0;

        // Convert to filetime
        if(NKSystemTimeToFileTime(pSystemTime, (FILETIME*)&s_rtc.alarmFiletime))
		{
			ULONGLONG	ares = (ULONGLONG)dwNKAlarmResolutionMSec * 10000;
			RETAILMSG(0, (L"OEMSetAlarmTime:1 %x%x ares %x\r\n", (UINT32)(s_rtc.alarmFiletime >> 32), (UINT32)(s_rtc.alarmFiletime), (ULONG32)ares));
			if(s_rtc.alarmFiletime % ares)
				s_rtc.alarmFiletime += ares;
			s_rtc.alarmFiletime /= ares;
			s_rtc.alarmFiletime *= ares;

			RETAILMSG(0, (L"OEMSetAlarmTime:2 %x%x\r\n", (UINT32)(s_rtc.alarmFiletime>>32), (UINT32)(s_rtc.alarmFiletime)));
            //  Adjust alarm time by secure offset
            //s_rtc.alarmFiletime  -= s_rtc.baseOffset;

            //  Interrupt RTC driver to inform RTC HW that an alarm time was set
            s_rtc.updateFlag |= IOCTL_HAL_RTC_QUERY_SET_ALARM;

   			if(s_rtc.sysIntRTC != SYSINTR_UNDEFINED)
				NKSetInterruptEvent(s_rtc.sysIntRTC);
#if 0
            UCHAR   status;
            UCHAR   bcdTime[6];

            //  Adjust alarm time by secure offset
            s_rtc.alarmFiletime  = s_rtc.alarmFiletime - s_rtc.baseOffset;

            //  Convert to BCD time format
            FiletimeToHWTime( s_rtc.alarmFiletime, bcdTime );

            //  Write alarm registers
            OALTritonWrite(s_rtc.hTWL, TWL_ALARM_YEARS_REG, bcdTime[5]);
            OALTritonWrite(s_rtc.hTWL, TWL_ALARM_MONTHS_REG, bcdTime[4]);
            OALTritonWrite(s_rtc.hTWL, TWL_ALARM_DAYS_REG, bcdTime[3]);
            OALTritonWrite(s_rtc.hTWL, TWL_ALARM_HOURS_REG, bcdTime[2]);
            OALTritonWrite(s_rtc.hTWL, TWL_ALARM_MINUTES_REG, bcdTime[1]);
            OALTritonWrite(s_rtc.hTWL, TWL_ALARM_SECONDS_REG, bcdTime[0]);

            //  Set toggle bit to latch alarm registers
            OALTritonRead(s_rtc.hTWL, TWL_RTC_CTRL_REG, &status);

            status |= TWL_RTC_CTRL_RUN | TWL_RTC_CTRL_GET_TIME;
            OALTritonWrite(s_rtc.hTWL, TWL_RTC_CTRL_REG, status);
#endif
            // Done
            rc = TRUE;
		}

        LeaveCriticalSection(&s_rtc.cs);
	}
    
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalRtcQuery
//
//  This function is called by RTC driver on each interrupt to look
//  set OEMSetAlarmTime actions.
//
BOOL OALIoCtlHalRtcQuery(UINT32 code, VOID *pInBuffer, UINT32 inSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL rc = FALSE;
    IOCTL_HAL_RTC_QUERY_OUT *pQueryOut;

    OALMSG(OAL_TIMER && OAL_FUNC, (L"+OALIoCtlHalRtcQuery()\r\n"));

    //  Return Alarm info
    if(pOutSize)
		*pOutSize = sizeof(IOCTL_HAL_RTC_QUERY_OUT);

    if((pOutBuffer == NULL) || (outSize < sizeof(IOCTL_HAL_RTC_QUERY_OUT)))
	{
        NKSetLastError(ERROR_INVALID_PARAMETER);
        return rc;
	}

    // Copy and clear info...
    EnterCriticalSection(&s_rtc.cs);

    //  Initialize return structure
    pQueryOut = (IOCTL_HAL_RTC_QUERY_OUT*) pOutBuffer;
    memset( pQueryOut, 0, sizeof(IOCTL_HAL_RTC_QUERY_OUT));

    pQueryOut->flags = s_rtc.updateFlag;

    FiletimeToHWTime( s_rtc.baseFiletime + s_rtc.baseOffset, pQueryOut->time );
    OALMSGS(0, (L"OALIoCtlHalRtcQuery(RTC) %s\r\n", HWTimeToString(pQueryOut->time)));
    
    FiletimeToHWTime( s_rtc.alarmFiletime, pQueryOut->alarm );
    OALMSGS(0, (L"OALIoCtlHalRtcQuery(ALARM) %s\r\n", HWTimeToString(pQueryOut->alarm)));

    s_rtc.updateFlag = 0;

    LeaveCriticalSection(&s_rtc.cs);

    // Done
    rc = TRUE;
    
    return rc;
}


#if 0
//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalRtcTime
//
//  This function is called by RTC driver when time event interrupt
//  occurs.
//
BOOL
OALIoCtlHalRtcTime(
    UINT32 code, 
    VOID *pInBuffer, 
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    )
{
    SYSTEMTIME  baseSystemTime;
    UCHAR       status;
    UCHAR       bcdTime[6];

    OALMSG(OAL_TIMER && OAL_FUNC, (L"+OALIoCtlHalRtcTime()\r\n"));

    //  The RTC in Triton2 is set to periodically sync with the kernel time
    //  to ensure there is no clock drift.  When a sync event is triggered,
    //  the T2 RTC is used to set the base time in the kernel.

    EnterCriticalSection(&s_rtc.cs);

    //  Set get time flag            
    OALTritonRead(s_rtc.hTWL, TWL_RTC_CTRL_REG, &status);

    status |= TWL_RTC_CTRL_RUN | TWL_RTC_CTRL_GET_TIME;
    OALTritonWrite(s_rtc.hTWL, TWL_RTC_CTRL_REG, status);

    //  Get date and time from RTC
    OALTritonRead(s_rtc.hTWL, TWL_YEARS_REG, &bcdTime[5]);
    OALTritonRead(s_rtc.hTWL, TWL_MONTHS_REG, &bcdTime[4]);
    OALTritonRead(s_rtc.hTWL, TWL_DAYS_REG, &bcdTime[3]);
    OALTritonRead(s_rtc.hTWL, TWL_HOURS_REG, &bcdTime[2]);
    OALTritonRead(s_rtc.hTWL, TWL_MINUTES_REG, &bcdTime[1]);
    OALTritonRead(s_rtc.hTWL, TWL_SECONDS_REG, &bcdTime[0]);

    //  Convert current RTC date/time to FILETIME
    baseSystemTime.wYear    = BCD2BIN(bcdTime[5]) + RTC_BASE_YEAR_MIN;
    baseSystemTime.wMonth   = BCD2BIN(bcdTime[4]);
    baseSystemTime.wDay     = BCD2BIN(bcdTime[3]);
    baseSystemTime.wHour    = BCD2BIN(bcdTime[2]);
    baseSystemTime.wMinute  = BCD2BIN(bcdTime[1]);
    baseSystemTime.wSecond  = BCD2BIN(bcdTime[0]);
    baseSystemTime.wMilliseconds = 0;

    //  Update the base filetime to match RTC
    NKSystemTimeToFileTime(&baseSystemTime, (FILETIME*)&s_rtc.baseFiletime);

    //  Reset the tick count
    s_rtc.baseTickCount = OEMGetTickCount();
    
    LeaveCriticalSection(&s_rtc.cs);

    return TRUE;
}
#endif
//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalRtcUpdate
//
//  This function is called periodically by RTC driver to update
//  RTC value. 
//
BOOL OALIoCtlHalRtcUpdate(UINT32 code, VOID *pInBuffer, UINT32 inSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL    rc = FALSE;
    IOCTL_HAL_RTC_UPDATE_IN *rtcUpdate;
    SYSTEMTIME              baseSystemTime;

    OALMSG(OAL_TIMER && OAL_FUNC, (L"+OALIoCtlHalRtcUpdate()\r\n"));

    // Check basic parameters
    if((pInBuffer == NULL) || (inSize < sizeof(IOCTL_HAL_RTC_UPDATE_IN)))
	{
        NKSetLastError(ERROR_INVALID_PARAMETER);
        return rc;
	}


    //  The RTC in Triton2 is set to periodically sync with the kernel time
    //  to ensure there is no clock drift.  When a sync event is triggered,
    //  the T2 RTC is used to set the base time in the kernel.  The RTC driver
    //  also loads any stored offset from the registry for restoring the time
    //  on warm boot scenarios

    EnterCriticalSection(&s_rtc.cs);

    rtcUpdate = (IOCTL_HAL_RTC_UPDATE_IN*)pInBuffer;

#if 0
    //  Update the offset if it is currently 0
    if( s_rtc.baseOffset == 0 )
    {
        // We had saved signed 32bit offset value as unsigned in the registry.
        // Make sure that we convert the DWORD from registry to signed 32 bit integer.
        INT32 temp = (INT32)(rtcUpdate->offset);

        // Convert the signed 32 bit integer to 64bit signed integer.
        // This way, we recovered correct sign in 64bit baseOffset.
        s_rtc.baseOffset = (LONGLONG)temp; 

        // Convert offset in seconds to filetime base.
        s_rtc.baseOffset = s_rtc.baseOffset * 10000000;
    }
#else
	s_rtc.baseOffset = (LONGLONG)rtcUpdate->offset * 10000000;
#endif

    //  Convert Triton2 RTC date/time to FILETIME
    baseSystemTime.wYear    = BCD2BIN(rtcUpdate->time[5]) + RTC_BASE_YEAR_MIN;
    baseSystemTime.wMonth   = BCD2BIN(rtcUpdate->time[4]);
    baseSystemTime.wDay     = BCD2BIN(rtcUpdate->time[3]);
    baseSystemTime.wHour    = BCD2BIN(rtcUpdate->time[2]);
    baseSystemTime.wMinute  = BCD2BIN(rtcUpdate->time[1]);
    baseSystemTime.wSecond  = BCD2BIN(rtcUpdate->time[0]);
    baseSystemTime.wMilliseconds = 0;

    NKSystemTimeToFileTime(&baseSystemTime, (FILETIME*)&s_rtc.baseFiletime);

    //  Reset the tick count
    s_rtc.baseTickCount = LocalGetTickCount();

    LeaveCriticalSection(&s_rtc.cs);

    OALMSG(OAL_TIMER && OAL_FUNC, (L"-OALIoCtlHalRtcUpdate()\r\n"));

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalRtcAlarm
//
//  This function is called by RTC driver when alarm interrupt
//  occurs.
//
BOOL OALIoCtlHalRtcAlarm(UINT32 code, VOID *pInBuffer, UINT32 inSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
	OALMSGS(0, (L"OALIoCtlHalRtcAlarm: Notify about alarm\r\n"));

    //  Alarm has been triggered by RTC driver.
    NKSetInterruptEvent(SYSINTR_RTC_ALARM);
    return TRUE;
}

//------------------------------------------------------------------------------

BOOL FiletimeToHWTime(ULONGLONG fileTime, UCHAR bcdTime[6])
{
    SYSTEMTIME systemTime;

    //  Convert filetime to RTC HW time format
    NKFileTimeToSystemTime((FILETIME*)&fileTime, &systemTime);

    //  Limit RTC year range
    if( systemTime.wYear < RTC_BASE_YEAR_MIN )
        systemTime.wYear = RTC_BASE_YEAR_MIN;

    if( systemTime.wYear > RTC_BASE_YEAR_MAX )
        systemTime.wYear = RTC_BASE_YEAR_MAX;

    bcdTime[5] = BIN2BCD(systemTime.wYear - RTC_BASE_YEAR_MIN);
    bcdTime[4] = BIN2BCD(systemTime.wMonth);
    bcdTime[3] = BIN2BCD(systemTime.wDay);
    bcdTime[2] = BIN2BCD(systemTime.wHour);
    bcdTime[1] = BIN2BCD(systemTime.wMinute);
    bcdTime[0] = BIN2BCD(systemTime.wSecond);
        
    return TRUE;
}
#endif
//------------------------------------------------------------------------------
