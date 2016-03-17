// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
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
//  File: sdcontroller.cpp
//
//

#include <windows.h>
#include <initguid.h>
#include "SDController.h"
#include <nkintr.h>
// needed for CPU revision constants
#include <omap_cpuver.h>
#include <oalex.h>
#include <twl.h>
#include <tps659xx.h>
#include <gprs.h>

#define DEBOUNCE_TIME           100     // 100ms debounce timeout

//------------------------------------------------------------------------------
//  Global variables

CSDIOController::CSDIOController()
    : CSDIOControllerBase()
{
    m_pSyscGeneralRegs = NULL;
//	m_hModemOnOffEvent = NULL;
}

CSDIOController::~CSDIOController()
{
}

BOOL CSDIOController::InitializeWPDetect(void)
{
    if (m_dwCardWPGPIO != MMC_NO_GPIO_CARD_WP)
    {
        GPIOSetMode( m_hGPIO, m_dwCardWPGPIO, GPIO_DIR_INPUT );
        IsWriteProtected();
    }
    return TRUE;
}

BOOL CSDIOController::DeInitializeWPDetect(void)
{
    return TRUE;
}

// Is this card write protected?
BOOL CSDIOController::IsWriteProtected()
{
    BOOL fWriteProtected;

    if (m_dwCardWPGPIO == MMC_NO_GPIO_CARD_WP)
	    return FALSE;

    if ( GPIOGetBit( m_hGPIO, m_dwCardWPGPIO ) == m_dwCardWriteProtectedState )
    {
        RETAILMSG(1, (TEXT("IsWriteProtected(%d): YES.\r\n"), m_dwCardWPGPIO));
        fWriteProtected = 1;
    }
    else
    {
		RETAILMSG(1, (TEXT("IsWriteProtected(%d): NO.\r\n"), m_dwCardWPGPIO));
        fWriteProtected = 0;
    }

    return fWriteProtected;
}

// Is the card present?
BOOL CSDIOController::SDCardDetect()
{
    DWORD dwRegVal = 0;
    switch(m_dwSlot)
    {
		case MMCSLOT_1:
		{
			dwRegVal = (GPIOGetBit(m_hGPIO, m_dwCardDetectGPIO) == m_dwCardInsertedGPIOState);
			break;
		}
		case MMCSLOT_2:
		{
			dwRegVal = 0;
			break;
		}
    }

    RETAILMSG(0, (L"SDCardDetect %d, %d <- %X\r\n", m_dwSlot, dwRegVal, m_hCardPresent));

	return dwRegVal;
}

//-----------------------------------------------------------------------------
BOOL CSDIOController::InitializeCardDetect()
{
    switch(m_dwSlot)
    {
		case MMCSLOT_1:
		{
			GPIOSetMode(m_hGPIO, m_dwCardDetectGPIO, GPIO_DIR_INPUT | GPIO_INT_LOW_HIGH | GPIO_INT_HIGH_LOW | GPIO_DEBOUNCE_ENABLE);
			GPIOPullup(m_hGPIO, m_dwCardDetectGPIO, TRUE);
			break;
		}
		case MMCSLOT_2:
		{
			break;
		}
    }

    return 1;
}

//-----------------------------------------------------------------------------
BOOL CSDIOController::DeInitializeCardDetect()
{
    switch(m_dwSlot)
    {
		case MMCSLOT_1:
		{
			if(m_hGPIO)
			{
				// disable wakeup on card detect interrupt
				if(m_dwWakeupSources & WAKEUP_CARD_INSERT_REMOVAL)
					GPIOWakeDisable(m_hGPIO, m_dwCardDetectGPIO);

				GPIOInterruptMask(m_hGPIO, m_dwCardDetectGPIO, TRUE);
				if(m_hCardDetectEvent)
					GPIOInterruptDisable(m_hGPIO, m_dwCardDetectGPIO);
			}
			break;
		}
		case MMCSLOT_2:
		{
//   	        CloseHandle(m_hModemOnOffEvent);
//	        m_hModemOnOffEvent = NULL;

			if(m_hCardPresent)
				ResetEvent(m_hCardPresent);
			break;
		}
    }

    return 1;
}

//-----------------------------------------------------------------------------
BOOL CSDIOController::InitializeHardware()
{
    DWORD               dwCountStart;

    if (INVALID_HANDLE_VALUE == (m_hGPIO = GPIOOpen()))
	{
        RETAILMSG(1, (L"Can't open GPIO driver\r\n"));
		goto cleanUp;
	}

    // check for required configuration values
    if (m_dwCardDetectGPIO == MMC_NO_GPIO_CARD_DETECT || m_dwCardInsertedGPIOState == MMC_NO_GPIO_CARD_DETECT_STATE)
	{
        RETAILMSG(1, (L"No card detect GPIO pin and state selected for slot %d (check registry)\r\n", m_dwSlot));
	    return FALSE;
	}
	
    if (m_dwCardWPGPIO != MMC_NO_GPIO_CARD_WP && m_dwCardWriteProtectedState == MMC_NO_GPIO_CARD_WP_STATE)
	{
        RETAILMSG(1, (L"No write protect GPIO state selected for slot %d (check registry)\r\n", m_dwSlot));
	    return FALSE;
	}

    // Reset the controller
    OUTREG32(&m_pbRegisters->MMCHS_SYSCONFIG, MMCHS_SYSCONFIG_SOFTRESET);

    // calculate timeout conditions
    dwCountStart = GetTickCount();

    // Verify that reset has completed.
    while (!(INREG32(&m_pbRegisters->MMCHS_SYSSTATUS) & MMCHS_SYSSTATUS_RESETDONE))
    {
        Sleep(0);

        // check for timeout
        if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
        {
            DEBUGMSG(ZONE_ENABLE_ERROR, (TEXT("InitializeModule() - exit: TIMEOUT.\r\n")));
            goto cleanUp;
        }
    }

    PHYSICAL_ADDRESS    pa;
    pa.QuadPart = OMAP_SYSC_GENERAL_REGS_PA;
    m_pSyscGeneralRegs = (OMAP_SYSC_GENERAL_REGS *) MmMapIoSpace(pa, sizeof(OMAP_SYSC_GENERAL_REGS), FALSE);
    if (m_pSyscGeneralRegs == NULL)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"ERROR: CSDIOController::InitializeHardware: "
            L"Failed map SDHC module registers\r\n"
            ));
        goto cleanUp;
    }

    // may be needed unless separate MMC2 clock input on MMC2_DAT7 pin is used
    if (m_dwSlot == MMCSLOT_2)
	    m_pSyscGeneralRegs->CONTROL_DEVCONF1 |= (1 << 6); // set MMCSDIO2ADPCLKISEL bit

    InitializeCardDetect();
    InitializeWPDetect();
    return TRUE;
cleanUp:
    return FALSE;
}

void CSDIOController::DeinitializeHardware()
{
    DeInitializeCardDetect();
    DeInitializeWPDetect();

    MmUnmapIoSpace((PVOID)m_pSyscGeneralRegs, sizeof(OMAP_SYSC_GENERAL_REGS));
}

//-----------------------------------------------------------------------------
//  SDHCCardDetectIstThreadImpl - card detect IST thread for driver
//  Input:  lpParameter - pController - controller instance
//  Output:
//  Return: Thread exit status
DWORD CSDIOController::SDHCCardDetectIstThreadImpl()
{
	BOOL	bTemporaryTurnOn = FALSE;
    BOOL    bDebounce       = TRUE;
	HANDLE	Mutex;                      // used to synchronize access to SDIO controller registers
	HANDLE  hTWLDevice = NULL;

	Mutex = CreateMutex(0, 0, L"Handle Card Insertion");
    if (!CeSetThreadPriority(GetCurrentThread(), m_dwCDPriority ))
    {
        DEBUGMSG(SDCARD_ZONE_WARN, (TEXT("SDHCCardDetectIstThread: warning, failed to set CEThreadPriority\r\n")));
    }

	RETAILMSG(0, (L"+%S\r\n",__FUNCTION__));

	WaitForSingleObject(Mutex, INFINITE);
    // Associate event with GPIO interrupt
	if(m_hGPIO && m_dwSlot == MMCSLOT_1)
	{
		Sleep(100);
		if(!GPIOInterruptInitialize(m_hGPIO, m_dwCardDetectGPIO, m_hCardDetectEvent)) 
		{
			RETAILMSG (1, (L"ERROR: CSDIOController::SDHCCardDetectIstThreadImpl: Failed associate event with CD GPIO interrupt\r\n"));
			goto cleanUp;
		}
    
		// enable wakeup on card detect interrupt
		if(m_dwWakeupSources & WAKEUP_CARD_INSERT_REMOVAL)
			GPIOWakeEnable(m_hGPIO, m_dwCardDetectGPIO);

		RETAILMSG(1, (L"%S - MMCSLOT_1 Ok\r\n", __FUNCTION__)); 
	}

	if(m_hGPIO && m_dwSlot == MMCSLOT_2)
	{
		ResetEvent(m_hCardPresent);
	}
	Sleep(100);

    // check for the card already inserted when the driver is loaded
    SetEvent( m_hCardDetectEvent );

	ReleaseMutex(Mutex);

    // Loop until we are not stopped...
    while (TRUE)
    {
        DWORD   dwWaitStatus;

        // Wait for event
        dwWaitStatus = WaitForSingleObject(m_hCardDetectEvent, ((bDebounce==TRUE) ? DEBOUNCE_TIME : INFINITE));

        if (m_fDriverShutdown)
            break;

        SDHCDAcquireHCLock(m_pHCContext);

        if (m_fCardPresent == TRUE)
        {
            // Notify card de-assertion
			RETAILMSG(0, (L"%S - SDHC slot %d card removal\r\n",__FUNCTION__, m_dwSlot));

            CardInterrupt(FALSE);
			if(m_hGPIO && m_dwSlot == MMCSLOT_2)
			{
				GPIOClrBit(m_hGPIO, GPIO_137);
				GPIOClrBit(m_hGPIO, GPIO_139);
			}
            bDebounce = TRUE;
        }
        else
        {
            if ((bTemporaryTurnOn || SDCardDetect()) && (dwWaitStatus == WAIT_TIMEOUT || m_dwSlot == MMCSLOT_2))
            {
				bTemporaryTurnOn = FALSE;

				RETAILMSG(0, (L"%S - SDHC slot %d card insertion\r\n",__FUNCTION__, m_dwSlot));
				if(m_hGPIO && m_dwSlot == MMCSLOT_2)
				{
					GPIOSetBit(m_hGPIO, GPIO_139);
					GPIOSetBit(m_hGPIO, GPIO_137);
					Sleep(200);
				}
                CardInterrupt(TRUE);
                bDebounce = FALSE;
			}
			else
			{
				RETAILMSG(0, (TEXT("%S - debounce %d\r\n"),__FUNCTION__, m_dwSlot));
                bDebounce = (dwWaitStatus == WAIT_TIMEOUT) ? FALSE : TRUE;
			}
		}

		SDHCDReleaseHCLock(m_pHCContext);               
	}
cleanUp:
	CloseHandle(Mutex);

	RETAILMSG(0, (L"-%S\r\n",__FUNCTION__));

    return ERROR_SUCCESS;
}

//-----------------------------------------------------------------------------
VOID CSDIOController::PreparePowerChange(CEDEVICE_POWER_STATE curPowerState, BOOL bInPowerHandler)
{
    #ifdef MMCHS1_VDDS_WORKAROUND
        if (m_ActualPowerState != curPowerState)
        switch (curPowerState)
        {
            case D0:
            case D1:
            case D2:
                if (m_dwSlot == MMCSLOT_1)
                {
                    // Make sure VDDS stable bit is cleared before enabling the power for slot1
                    m_pSyscGeneralRegs->CONTROL_PBIAS_LITE &= (~0x00000003);
                } 
				else if(m_dwSlot == MMCSLOT_2)
				{
                    m_pSyscGeneralRegs->CONTROL_PBIAS_LITE &= (~0x00000002);
                    m_pSyscGeneralRegs->CONTROL_DEVCONF1 &= ~(1 << 6);  // clear MMCSDIO2ADPCLKISEL bit
				}
                break;

            case D3:
            case D4:
                break;
       }
    #endif
}

VOID CSDIOController::PostPowerChange(CEDEVICE_POWER_STATE curPowerState, BOOL bInPowerHandler)
{
    #ifdef MMCHS1_VDDS_WORKAROUND
        if (m_ActualPowerState != curPowerState)
        switch (curPowerState)
        {
            case D0:
            case D1:
            case D2:
				if (bInPowerHandler)
					StallExecution(100000);
				else
	             	Sleep(100);
                if (m_dwSlot == MMCSLOT_1)
                {
                    UINT32 dwPBiasValue = DEFAULT_PBIAS_VALUE;

                    if (m_dwCPURev == CPU_FAMILY_35XX_REVISION_ES_1_0)   // ES 1.0
                        dwPBiasValue = 0x00000003;
                    else if (m_dwCPURev == CPU_FAMILY_35XX_REVISION_ES_2_0) // ES 2.0
                        dwPBiasValue = 0x00000002;
                    else if (m_dwCPURev == CPU_FAMILY_35XX_REVISION_ES_2_1) // ES 2.1
                        #ifdef MMCHS1_LOW_VOLTAGE
                            dwPBiasValue = 0x00000002;
                        #else
                            dwPBiasValue = 0x00000003;
                        #endif
                    else
                        dwPBiasValue = DEFAULT_PBIAS_VALUE;

                    // Workaround to make the MMC slot 1 work
                    m_pSyscGeneralRegs->CONTROL_PBIAS_LITE |= dwPBiasValue;
                    m_pSyscGeneralRegs->CONTROL_DEVCONF1 |= (1 << 24); // set reserved bit???
                }
				else if(m_dwSlot == MMCSLOT_2)
				{
					m_pSyscGeneralRegs->CONTROL_PBIAS_LITE |= (0x00000002);
					m_pSyscGeneralRegs->CONTROL_DEVCONF1 |= (1 << 6);
				}
                break;

            case D3:
            case D4:
                #if 0
                #ifndef MMCHS1_LOW_VOLTAGE
                    // clear MMC1_ACTIVE_OVERWRITE before the device goes in retention or to OFF
                    if (m_dwSlot == MMCSLOT_1 && m_dwCPURev == CPU_FAMILY_35XX_REVISION_ES_2_1)
                        m_pSyscGeneralRegs->CONTROL_DEVCONF1 &= ~(1 << 31); // clear reserved bit???
                #endif
                #endif
                break;
        }
    #endif
}

VOID CSDIOController::TurnCardPowerOn()
{
    SetSlotPowerState( D0 );
}

VOID CSDIOController::TurnCardPowerOff()
{
    SetSlotPowerState( D4 );
}

CSDIOControllerBase *CreateSDIOController()
{
    return new CSDIOController();
}

