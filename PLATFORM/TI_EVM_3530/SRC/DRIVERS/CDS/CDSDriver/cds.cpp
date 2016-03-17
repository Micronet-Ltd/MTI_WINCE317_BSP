// cds.cpp : Defines the entry point for the DLL application.
// CDS - CPU Dynamic Speed Selection. Change CPU configured main clock according to chip temperature.
// Initializes CDS Interrupt which samples Junction temperature of AD3703 and reduces CPU Speed accordingly.
// IOCTL functionality enables user to change CPU Speed.
#include <bsp.h>
#include <cds.h>
#include <cdsInterface.h>
#include <omap35xx.h>
#include <ceddk.h>
#include <oal.h>
#include <oalex.h>
#include <oal_prcm.h>
#include <devload.h>
#include <bsp_cfg.h>
#include <gpio.h>
#include <bus.h>
#include <twl.h>
#include <tempsensor.h>

CDS_HW_INFO *g_pDevice = NULL;
CDS_GPT_INFO Cds_Info = {0};  //Info for CDS Interrupt.
HANDLE       cdsIsrHandle_gpt = INVALID_HANDLE_VALUE; //Handler of installable ISR

static const GUID DEVICE_IFC_GPIO_GUID;
static const GUID DEVICE_IFC_TWL_GUID;

//------------------------------------------------------------------------------
//
//  Function:  CdsUpdateMpuInfo
//
//  Update Windows API with set CPU speed
//
void CdsUpdateMpuInfo()
{
	UINT32 InBuffer = 0;
	UINT32 OpmToFreq [3] = {600, 800, 1000};

	InBuffer = OpmToFreq[Cds_Info.CurrentOpm - OPM_600MHz];
	KernelIoControl(IOCTL_HAL_SET_MPU_SPEED, &InBuffer, sizeof(InBuffer), NULL, 0, NULL);

	RETAILMSG(1, (L"CdsUpdateMpuInfo InBuffer = %d CurrentOpm = %d \r\n", InBuffer, Cds_Info.CurrentOpm));

}

//------------------------------------------------------------------------------
//
//  Function:  CdsTimerEventThread
//
//  Thread waiting for Event from CDS timer Interrupt ISR. Update current OPM, ans CDS statistics in registry.
//
DWORD CdsTimerEventThread(VOID *pContext)
{
	HANDLE ltimerEvent = Cds_Info.hTimer;
	CDS_HW_INFO *pDevice = (CDS_HW_INFO*)pContext;

	while(!pDevice->intrThreadExit)
	{
		// Wait for event
		WaitForSingleObject(ltimerEvent, INFINITE);
		
		//Update MPU Info
		CdsUpdateMpuInfo();

		//Update Maximal Temperature statistics in registry
		if ((pDevice->CdsStats.UpdateStats) && (pDevice->hRegKey)){
			if (pDevice->CdsStats.UpdateStats & CDS_STATS_BREACH_FLAG) {
				//Disable Flag
				pDevice->CdsStats.UpdateStats &= ~CDS_STATS_BREACH_FLAG;
				pDevice->CdsStats.NumTjBreaches += 1;
				RegSetValueEx (pDevice->hRegKey, L"NumOfTimesMjtBreach", 0, REG_DWORD, (BYTE *)&pDevice->CdsStats.NumTjBreaches, sizeof(DWORD));
			}
			if (pDevice->CdsStats.UpdateStats &CDS_STATS_MAX_TJ_FLAG) {
				//Disable Flag
				pDevice->CdsStats.UpdateStats &= ~CDS_STATS_MAX_TJ_FLAG;
				RegSetValueEx (pDevice->hRegKey, L"MaximalADC", 0, REG_DWORD, (BYTE *)&pDevice->CdsStats.MaximalADCValue, sizeof(DWORD));
			}

			//Write registry Key
			RegFlushKey(pDevice->hRegKey);
		}
	}

	return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  CdsInitStartGptimer
//
//  Initialize GPTimer 5 for CDS monitoring Tj and set CPU operation mode accordingly
//
DWORD CdsInitStartGptimer(VOID *pContext)
{
	DWORD               dwSize;
	OMAP_GPTIMER_REGS   *s_pGPT5 = NULL;
	PHYSICAL_ADDRESS    pA;
	IOCTL_HAL_PRCMDEVICEENABLECLOCKS_IN  s_pcrmDevEnClks = {0,0};
	IOCTL_PRCM_CLOCK_SET_SOURCECLOCK_IN  s_pParentClk;
	IOCTL_HAL_OALPATOVA_IN  pPA;
	CDS_HW_INFO *pDevice = (CDS_HW_INFO*)pContext;

	pA.QuadPart = OMAP_GPTIMER5_REGS_PA;

	s_pGPT5 = (OMAP_GPTIMER_REGS*)MmMapIoSpace( pA, sizeof(OMAP_GPTIMER_REGS), FALSE);
	Cds_Info.pGpt = s_pGPT5;
	if( s_pGPT5 == NULL )
	{
		RETAILMSG(1, (L"ERROR: CDS Timer Init: s_pGPT5  MmMapIoSpac  failure\r\n"));
		return FALSE;
	}
	else
	{
		RETAILMSG(1, (L"CDS Timer Init: s_pGPT5 addr = 0x%X\r\n", s_pGPT5) );
	}

	s_pParentClk.clkId = kGPT5_ALWON_FCLK;
	s_pParentClk.newParentClkId = kSYS_CLK;

	if( !KernelIoControl(IOCTL_PRCM_CLOCK_SET_SOURCECLOCK, (LPVOID)&s_pParentClk, sizeof(s_pParentClk), NULL, 0, &dwSize) )
	{
		MmUnmapIoSpace( (PVOID)s_pGPT5, sizeof(OMAP_GPTIMER_REGS));
		s_pGPT5 = NULL;
		RETAILMSG(1, (L"ERROR: CDS Timer Init: PrcmClockSetParent  failure\r\n"));
		return FALSE;
	}
	else
	{
		RETAILMSG(1, (L"CDS Timer Init: PrcmClockSetParent OK\r\n"));
	}

	s_pcrmDevEnClks.devId = OMAP_DEVICE_GPTIMER5;
	s_pcrmDevEnClks.bEnable = TRUE;

	if( !KernelIoControl(IOCTL_HAL_PRCMDEVICEENABLECLOCKS, (LPVOID)&s_pcrmDevEnClks, sizeof(s_pcrmDevEnClks), NULL, 0, &dwSize) )
	{
		MmUnmapIoSpace( (PVOID)s_pGPT5, sizeof(OMAP_GPTIMER_REGS));
		s_pGPT5 = NULL;
		RETAILMSG(1, (L"ERROR: CDS Timer Init: PrcmDeviceEnableClocks  failure\r\n"));
		return FALSE;
	}
	else
	{
		RETAILMSG(1, (L"CDS Timer Init: PrcmDeviceEnableClocks OK\r\n"));
	}


	/* After PCRM clock is enable for GPTimer, its registers are available */
	Cds_Info.TimerIrQNum = IRQ_GPTIMER5;

	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, (LPVOID)&Cds_Info.TimerIrQNum, 
		sizeof(Cds_Info.TimerIrQNum), (LPVOID)&Cds_Info.SysIntr, sizeof(Cds_Info.SysIntr), NULL )) 
	{
		RETAILMSG(1, (L"ERROR: CDS Timer Init: IOCTL_HAL_REQUEST_SYSINTR  failure\r\n"));
		MmUnmapIoSpace( (PVOID)s_pGPT5, sizeof(OMAP_GPTIMER_REGS));
		s_pGPT5 = NULL;
		s_pcrmDevEnClks.bEnable = FALSE;
		KernelIoControl(IOCTL_HAL_PRCMDEVICEENABLECLOCKS, (LPVOID)&s_pcrmDevEnClks, sizeof(LPVOID), NULL, 0, &dwSize);
		return FALSE;
	}
	else
	{
		RETAILMSG(1, (L"CDS Timer Init: Cds_Info.SysIntr = %d\r\n", Cds_Info.SysIntr));
	}

	if( (Cds_Info.hTimer = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL )
	{
		RETAILMSG(1, (L"ERROR: CDS Timer Init: CreateEvent failed\r\n")); 

		MmUnmapIoSpace( (PVOID)s_pGPT5, sizeof(OMAP_GPTIMER_REGS));
		s_pGPT5 = NULL;
		// Switch Off PCRM clock on Failure
		s_pcrmDevEnClks.bEnable = FALSE;
		KernelIoControl(IOCTL_HAL_PRCMDEVICEENABLECLOCKS, (LPVOID)&s_pcrmDevEnClks, sizeof(LPVOID), NULL, 0, &dwSize);

		// release the SYSINTR value
		KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &Cds_Info.SysIntr, sizeof(DWORD), NULL, 0, NULL);

		return FALSE;
	}
	else
	{
		RETAILMSG(1, (L"CDS Timer Init: Timer Event handle = 0x%X\r\n", Cds_Info.hTimer)); 
	}
	// Select non posted mode
	CLRREG32(&s_pGPT5->TSICR, 0x4);

	// No idle.
	OUTREG32(&s_pGPT5->TIOCP, 0x00000004);
	while ((INREG32(&s_pGPT5->TISTAT) & GPTIMER_TISTAT_RESETDONE) == 0);

	// Enable overflow wakeup
	OUTREG32(&s_pGPT5->TWER, 0x00000002);

	//Clear Interrupt
	SETREG32(&s_pGPT5->TISR, 0x2);

	// Enabled overflow interrupt
	SETREG32(&s_pGPT5->TIER, 0x2);

	//  Set the load register value.
	OUTREG32(&s_pGPT5->TLDR, (0xFF7FFFFF)); //0.3 second @ 32KHz clock

	//  Trigger a counter reload by writing    
	OUTREG32(&s_pGPT5->TTGR, 0xFFFFFFFF);

	//  Start the timer.  Also set for auto reload
	SETREG32(&s_pGPT5->TCLR, 0x00000003);

	if ( cdsIsrHandle_gpt == INVALID_HANDLE_VALUE )
	{
		cdsIsrHandle_gpt = LoadIntChainHandler(L"cds_isr.dll", L"ISRHandler", (BYTE)Cds_Info.TimerIrQNum);
		if( cdsIsrHandle_gpt == INVALID_HANDLE_VALUE )
		{
			RETAILMSG(1, (L"ERROR: CDS Timer Init: LoadIntChainHandler for cds_isr.dll Failed error[%d]\r\n", GetLastError()));
			return FALSE;
		}

		// Get OMAP MPU Interrupt Controller Registers addr
		pPA.pa     = OMAP_INTC_MPU_REGS_PA;
		pPA.cached = FALSE;

		if( !KernelIoControl(IOCTL_HAL_OALPATOVA, (LPVOID)&pPA, sizeof(IOCTL_HAL_OALPATOVA_IN), &Cds_Info.pICL, sizeof(VOID *), &dwSize) )
		{
			RETAILMSG(1, (L"Error: CDS Timer Init:IOCTL_ISR_TIMER_CFG, ILC PA Failure\r\n"));
			return NULL;
		}
		else
		{
			RETAILMSG(1, (L"CDS Timer Init: Retrived ILC PA 0x%x\r\n", Cds_Info.pICL));
		}

		if( !KernelLibIoControl(cdsIsrHandle_gpt, IOCTL_CDS_ISR_CONFIG, (LPVOID)&Cds_Info, sizeof(CDS_OPEN_INFO), NULL, 0, 0) )
		{
			RETAILMSG(1, (L"Error: CDS Timer Init:IOCTL_CDS_ISR_CONFIG Failure [%d]\r\n", GetLastError()));
		}
		else
		{
			RETAILMSG(1, (L"CDS Timer Init: CDS Info transferred to ISR = 0x%X, len = %d\r\n", &Cds_Info, sizeof(CDS_OPEN_INFO)));
		}

	}
	else {
		RETAILMSG(1, (L"WARNING: CDS Timer Init: LoadIntChainHandler already loaded\r\n", GetLastError()));
	}

	if( (ResetEvent(Cds_Info.hTimer)) == NULL )
	{
		RETAILMSG(1, (L"ERROR: CDS Timer Init: Failed to Reset hTimer\r\n"));
		return FALSE;
	}
	else
	{
		RETAILMSG(1, (L"CDS Timer Init: hTimer = 0x%X\r\n", Cds_Info.hTimer));
	}

	if( !InterruptInitialize(Cds_Info.SysIntr, Cds_Info.hTimer, NULL, 0) )
	{
		RETAILMSG(1, (L"ERROR: CDS Timer Init: InterruptInitialize failure [%d]\r\n", GetLastError())); 

		// Switch Off PCRM clock on Failure
		s_pcrmDevEnClks.bEnable = FALSE;
		KernelIoControl(IOCTL_HAL_PRCMDEVICEENABLECLOCKS, (LPVOID)&s_pcrmDevEnClks, sizeof(IOCTL_HAL_PRCMDEVICEENABLECLOCKS_IN), NULL, 0, &dwSize);

		// release the SYSINTR value
		KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &Cds_Info.SysIntr, sizeof(DWORD), NULL, 0, NULL);

		CloseHandle(Cds_Info.hTimer);
		return FALSE;
		//  goto cleanUp;
	}

	Cds_Info.hThreadTimer = CreateThread(NULL, 0, CdsTimerEventThread, pDevice, 0, NULL);
	if(Cds_Info.hThreadTimer == INVALID_HANDLE_VALUE)
	{
		RETAILMSG (1, (L"ERROR: CdsInitStartGptimer: Failed create Timer event capturing thread\r\n"));
		return FALSE;
	}

	return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  CdsTshutThread
//
//  Thread waiting for Temperature Shutdown event to arrive from GPIO_127.
//
DWORD CdsTshutThread(VOID *pContext)
{
	CDS_HW_INFO *pDevice = (CDS_HW_INFO*)pContext;
	HANDLE       tshutEvent = ((CDS_HW_INFO*)pContext)->hTshutEvent;
	UINT8 backupReg;

	// unmask interrupt
	GPIOInterruptMask(pDevice->hGpio, GPIO_127, FALSE);

	while(!pDevice->intrThreadExit)
	{
		// Wait for event
		WaitForSingleObject(tshutEvent, INFINITE);

		//Thermal Shutdown received from BGAPTS - Shutdwon device
		RETAILMSG(1, (L"CDS TSHUT: Thermal shutdown!!!\r\n"));

		TWLReadRegs(pDevice->hTwl, TWL_BACKUP_REG_G, &backupReg, 1);
		backupReg |= 2; //Bit 1 indicating MPU Thermal shutdown
		TWLWriteRegs(pDevice->hTwl, TWL_BACKUP_REG_G, &backupReg, 1);
		
		// Immediate direct shutdown
		KernelLibIoControl((HANDLE)KMOD_OAL, IOCTL_HAL_SHUTDOWN, 0, 0, 0, 0, 0);

        GPIOInterruptDone(pDevice->hGpio, GPIO_127);
	}

	return ERROR_SUCCESS;
}


DWORD CdsInitTshutGPIO(PCDS_HW_INFO pDevice)
{
	pDevice->hGpio = GPIOOpen();

	// Set TSHUT GPIO to input for interrupt reception. 
	// A rise from low to high indicates Thermal Shutdown request from BGAPTS.
	GPIOSetMode(pDevice->hGpio, GPIO_127, GPIO_DIR_INPUT | GPIO_INT_LOW_HIGH);

	// Map interrupt
	DWORD irqNum = GPIOGetSystemIrq(pDevice->hGpio, GPIO_127);

	if( (pDevice->hTshutEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL )
	{
		RETAILMSG(1, (L"ERROR: CdsInitTshutGPIO: CreateEvent failed\r\n"));
		return FALSE;
	}
	else
	{
		RETAILMSG(1, (L"CdsInitTshutGPIO: GPIO Event handle = 0x%X\r\n", pDevice->hTshutEvent));
	}

	if(!GPIOInterruptInitialize(pDevice->hGpio, GPIO_127, pDevice->hTshutEvent))
	{
		RETAILMSG(1, (L"ERROR: CdsInitTshutGPIO: GPIO InterruptInitialize failure\r\n"));
		return FALSE;
	}
	else
	{
		RETAILMSG(1, (L"CdsInitTshutGPIO: GPIO InterruptInitialize Ok\r\n"));
	}

	// mask interrupt
	GPIOInterruptMask(pDevice->hGpio, GPIO_127, TRUE);

	if(!KernelIoControl(IOCTL_HAL_IRQ2SYSINTR, &irqNum, sizeof(irqNum),  &pDevice->tShutSysIntr, sizeof(pDevice->tShutSysIntr), 0))
	{
		RETAILMSG(1, (L"ERROR: CdsInitTshutGPIO: cannot get tShutSysIntr\r\n"));
		return FALSE;
	}
	else
	{
		RETAILMSG(1, (L"CdsInitTshutGPIO: Irq num = %d, sysIntr = %d\r\n", irqNum, pDevice->tShutSysIntr));
	}

	// Start interrupt service thread
	pDevice->intrThreadExit = FALSE;

	pDevice->hTshutThread = CreateThread(NULL, 0, CdsTshutThread, pDevice, 0, NULL);
	if(!pDevice->hTshutThread)
	{
		RETAILMSG (1, (L"ERROR: CdsInitTshutGPIO: Failed create TSHUT capturing thread\r\n"));
		return FALSE;
	}

	// Set thread priority
	pDevice->priority256 = CE_THREAD_PRIO_256_ABOVE_NORMAL;
	CeSetThreadPriority(pDevice->hTshutThread, pDevice->priority256);

	return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  CdsCcTempThread
//
//  Thread waiting for messages of Temperature module, notifying of temperature change
//  in Companion chip temperature. A breach of maximal temperature indicates to CDS
//  that 1GHz is not allowed. This cools down Companion chip.
//
DWORD CdsCcTempThread(VOID *pContext)
{
	CDS_HW_INFO *pDevice = (CDS_HW_INFO*)pContext;
	HANDLE      lTpMsgQueue = pDevice->hTpMsgQueue;
	TEMPSENSOR_DATA* lpCcTempSensMsg = &pDevice->CcTempSensMsg;
	DWORD dwSize, dwFlags;

	//Signal Init Done Event as this is the last thread created in CDS Init
	SetEvent(pDevice->hInitDoneEvent);

	while(!pDevice->intrThreadExit)
	{
		WaitForSingleObject(lTpMsgQueue, INFINITE);
		// Read latest Companion Chip Temperature from message queue.
		while (ReadMsgQueue(lTpMsgQueue, lpCcTempSensMsg, sizeof(pDevice->CcTempSensMsg), &dwSize, 0, &dwFlags) == TRUE)
		{
			if (Cds_Info.CCTempRead != lpCcTempSensMsg->CurrentTemperature)
			{
				Cds_Info.CCTempRead = lpCcTempSensMsg->CurrentTemperature;
				RETAILMSG(1, (L"CDS CdsCcTempThread: New temperature = %d\r\n", lpCcTempSensMsg->CurrentTemperature));
				if (Cds_Info.CCTempRead >= lpCcTempSensMsg->LimiterTemperature) {
					Cds_Info.BlockOneGhz = TRUE;
				}
				else
				{
					Cds_Info.BlockOneGhz = FALSE;
				}
			}
		}
	}

	return ERROR_SUCCESS;
}



BOOL CdsInitCompanionChipTempRead(PCDS_HW_INFO pDevice)
{
	UINT32 res = 0;
	MSGQUEUEOPTIONS msgOptions	= {0};
	HANDLE lhTpModule, hTempSensInitDoneEvent;
	HKEY hRegKey; //resgistry key of PKD holding initilaztion done event name of companion chip temperature module
    WCHAR szEventName[MAX_PATH];

	LONG lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEMPSENSOR_REG_KEY, 0, 0, &hRegKey);
	if (ERROR_SUCCESS == lStatus)
	{	
		DWORD dwSize = sizeof(szEventName);
		lStatus = RegQueryValueEx(hRegKey, TEXT("InitDoneEventName"), NULL, NULL, (LPBYTE)szEventName, &dwSize);
		if (ERROR_SUCCESS != lStatus)
		{
			RETAILMSG(1, (L"Temperature sensor Error acquiring: InitDoneEventName\r\n"));
		}
	}

	lStatus =  RegCloseKey( hRegKey );
	if (ERROR_SUCCESS != lStatus)
	{	
		goto clean_up;
	}

	//Wait for event indicating Companion chip temperature sensor driver is up
	hTempSensInitDoneEvent = OpenEvent(EVENT_ALL_ACCESS , FALSE, szEventName);

	DWORD ret = WaitForSingleObject(hTempSensInitDoneEvent, 100);

	RETAILMSG(1, (L"CdsInitCompanionChipTempRead: TemperatureSensorOpen : WaitForSingleObject ret=%x\r\n", ret));	

	pDevice->hTpModule = TemperatureSensorOpen();
	lhTpModule = pDevice->hTpModule;

	if (lhTpModule == INVALID_HANDLE_VALUE)
	{
		RETAILMSG(1, (L"CDS: Can't open temperature sensor device. Error = %d\r\n", GetLastError()));
		goto clean_up;
	}

	res = TemperatureSensorRegister(lhTpModule, L"", &(pDevice->queueNameLenTpModule));
	if (res == TEMPSENSOR_ERROR_BUFFER_OVERFLOW)
	{
		pDevice->queueNameTpModule = (TCHAR *)LocalAlloc(LPTR, pDevice->queueNameLenTpModule);
		if (!pDevice->queueNameTpModule)
		{
			RETAILMSG(1, (L"CDS: Can't allocate memory for queue. Error = %d\r\n", GetLastError()));
			goto clean_up;
		}

		res = TemperatureSensorRegister(lhTpModule, pDevice->queueNameTpModule, &(pDevice->queueNameLenTpModule));
	}

	if (res != TEMPSENSOR_OK)
	{
		RETAILMSG(1, (L"Can't register temperature sensor notification queue. Error = %d\r\n", res));
		goto clean_up;
	}

	msgOptions.dwSize		= sizeof(MSGQUEUEOPTIONS);
	msgOptions.dwFlags		= MSGQUEUE_ALLOW_BROKEN;
	msgOptions.cbMaxMessage = sizeof(pDevice->CcTempSensMsg);
	msgOptions.bReadAccess	= 1;
	pDevice->hTpMsgQueue = CreateMsgQueue(pDevice->queueNameTpModule, &msgOptions);

	if (pDevice->hTpMsgQueue == INVALID_HANDLE_VALUE)
		goto clean_up;

	//Blocking 1GHz Speed support if Companion Chip reports a temperature higher than limit
	Cds_Info.BlockOneGhz = FALSE;
	/* Create thread that interceppts messages from Temperature module */
	pDevice->hTpThread = CreateThread(NULL, 0, CdsCcTempThread, pDevice, 0, NULL);
	
	/* All is fine */
	return TRUE;

clean_up:
	RETAILMSG(1, (L"Error during CdsInitCompanionChipTempRead\r\n"));

	if (pDevice->queueNameTpModule)
		LocalFree(pDevice->queueNameTpModule);

	if (lhTpModule != INVALID_HANDLE_VALUE)
	{
		TemperatureSensorDeregister(lhTpModule);
		TemperatureSensorClose(lhTpModule);
	}

	return FALSE;
}

BOOL CdsInitStatistics(PCDS_HW_INFO pDevice)
{
	LONG lStatus;
	DWORD value = 0, dwSize;
	Cds_Info.pCdsStat = &(pDevice->CdsStats);

	if (pDevice->hRegKey)
	{
		RETAILMSG(1, (L"+CdsInitStatistics pDevice->hRegKey = %x\r\n", pDevice->hRegKey));

		dwSize = sizeof(value);
		lStatus = RegQueryValueEx(pDevice->hRegKey, TEXT("MaximalADC"), NULL, NULL, (LPBYTE)&value, &dwSize);
		if (ERROR_SUCCESS == lStatus) {
			pDevice->CdsStats.MaximalADCValue = (INT16)value;
		}
		else {
			RETAILMSG(1, (L"Initiating CdsStats.MaximalADCValue\r\n"));
			pDevice->CdsStats.MaximalADCValue = 0;
		}
		dwSize = sizeof(value);
		lStatus = RegQueryValueEx(pDevice->hRegKey, TEXT("NumOfTimesMjtBreach"), NULL, NULL, (LPBYTE)&value, &dwSize);
		if (ERROR_SUCCESS == lStatus) {
			pDevice->CdsStats.NumTjBreaches = value;
		}
		else {
			RETAILMSG(1, (L"Initiating CdsStats.NumTjBreaches\r\n"));
			pDevice->CdsStats.NumTjBreaches = 0;
		}
	}
	else
	{
		RETAILMSG(1, (L"ERROR: CdsInitStatistics Error Opening Registry key: %s\r\n", CDS_REG_KEY));
		goto cleanUp;
	}

	RETAILMSG(1, (L"-CdsInitStatistics\r\n"));

	/* All is fine */
	return TRUE;

cleanUp:
	RETAILMSG(1, (L"Error during CdsInitStatistics\r\n"));
	return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  TemperatureToAdc
//
//  Return BGAPTS ADC word for temperature in the range of 85°C to 115°C.
//  Above or below that, temperature is clipped.
//
int TemperatureToAdc(int temperature)
{
	int index, RetAdcWord = 0;
	int TempToAdc[40] = { 84, 84, 85, 85, 86, 87, 87, 88,  88,  89,//85-94
	                      90, 90, 91, 91, 92, 93, 93, 94,  94,  95,//95-104
		                  96, 96, 97, 97, 98, 98, 99, 99, 100, 100,//105-114
	};//From 85 to 114

	//If above 115°C, return ADC word for 115°C
	if (temperature >= 115)
	{
		return 101;
	}

	// Lowest temperature allowed 85°C
	index = (temperature < 85) ? 0 : (temperature - 85);

	RetAdcWord = TempToAdc[index];

	return RetAdcWord;
}
//------------------------------------------------------------------------------
//
//  Function:  CdsInitRegistryParameters
//
//  Acquire  parameters from BOOT and insure no violation with MPU type.
//
BOOL CdsInitRegistryParameters(CDS_HW_INFO *pDevice)
{
	UINT32 idcode;
    PHYSICAL_ADDRESS pA;
	LONG lStatus;
	DWORD value = 0, dwSize;
	UINT16 CdsBootParams[CDS_BOOT_PARAM_SIZE];
	DWORD  out = 0, size = CDS_BOOT_PARAM_SIZE*sizeof(DWORD);

	if(!KernelIoControl(IOCTL_HAL_GET_CDS_PARAMS, 0, 0, CdsBootParams, size, &out))
	{
		RETAILMSG(1, (L"ERROR: CdsInitBspParameters IOCTL_HAL_GET_CDS_PARAMS\r\n"));
	}
	Cds_Info.MJT        = CdsBootParams[0];
	Cds_Info.IT         = CdsBootParams[1];
	Cds_Info.LTT        = CdsBootParams[2];
	Cds_Info.FeatStat   = CdsBootParams[3];
	Cds_Info.MaxSpeed   = CdsBootParams[4];
	Cds_Info.CurrentOpm = CdsBootParams[5];

	RETAILMSG(1, (L"CDS BOOT Parameters [0] = %d, [1] = %d , [2] = %d , [3] = %d , [4] = %d  [5] = %d\r\n",
		CdsBootParams[0]	,CdsBootParams[1]	,CdsBootParams[2]	,CdsBootParams[3]	,CdsBootParams[4]	,CdsBootParams[5]));

	if (pDevice->hRegKey)
	{
		RETAILMSG(1, (L"+CdsInitRegistryParameters pDevice->hRegKey = %x\r\n", pDevice->hRegKey));

		dwSize = sizeof(value);
		lStatus = RegQueryValueEx(pDevice->hRegKey, TEXT("FeatureState"), NULL, NULL, (LPBYTE)&value, &dwSize);
		if (ERROR_SUCCESS == lStatus) {
			Cds_Info.FeatStat = (INT16)value;
		}
		else {
			RETAILMSG(1, (L"Registry value FeatureState not found setting to default - ON\r\n"));
			Cds_Info.FeatStat = 1;
		}
		dwSize = sizeof(value);
		lStatus = RegQueryValueEx(pDevice->hRegKey, TEXT("MJThereshold"), NULL, NULL, (LPBYTE)&value, &dwSize);
		if (ERROR_SUCCESS == lStatus) {
			Cds_Info.MJT = TemperatureToAdc(value);
		}
		else {
			RETAILMSG(1, (L"Registry value MJThereshold not found setting to default - MJT[%d]\r\n", Cds_Info.MJT));
		}
		dwSize = sizeof(value);
		lStatus = RegQueryValueEx(pDevice->hRegKey, TEXT("IThereshold"), NULL, NULL, (LPBYTE)&value, &dwSize);
		if (ERROR_SUCCESS == lStatus) {
			Cds_Info.IT = TemperatureToAdc(value);
		}
		else {
			RETAILMSG(1, (L"Registry value IThereshold not found setting to default - IT[%d]\r\n",Cds_Info.IT));
		}
		dwSize = sizeof(value);
		lStatus = RegQueryValueEx(pDevice->hRegKey, TEXT("LTThereshold"), NULL, NULL, (LPBYTE)&value, &dwSize);
		if (ERROR_SUCCESS == lStatus) {
			Cds_Info.LTT = TemperatureToAdc(value);
		}
		else {
			RETAILMSG(1, (L"Registry value LTThereshold not found setting to default LTT[%d]\r\n", Cds_Info.LTT));
		}
	}
	else
	{
		RETAILMSG(1, (L"ERROR: CdsInitRegistryParameters Error Opening Registry key: %s\r\n", CDS_REG_KEY));
		goto cleanUp;
	}

	RETAILMSG(1, (L"CDS Parameters [MJT] = %d, [IT] = %d , [LTT] = %d , [FeatStat] = %d , [MaxSpeed] = %d\r\n",
		Cds_Info.MJT	,Cds_Info.IT	,Cds_Info.LTT	,Cds_Info.FeatStat	,Cds_Info.MaxSpeed));

	if ((Cds_Info.CurrentOpm >= OPM_600MHz) && (Cds_Info.CurrentOpm <= OPM_1GHz))
	{
		CdsUpdateMpuInfo();
	}
	else 
	{
		RETAILMSG(1, (L"CdsInitRegistryParameters Bad OPM value from BSP = %d\r\n",Cds_Info.CurrentOpm));
	}

	//Read 37x type and reduce values if they breach maximal allowed values by TI
	pA.QuadPart = OMAP_CHIP_ID;//0x4800244c;
	idcode = *(UINT32*)MmMapIoSpace( pA, 4, FALSE);
	RETAILMSG(1, (L"CdsInitRegistryParameters Control Device Status = 0x%x MaxSpeed = %d\r\n", idcode, Cds_Info.MaxSpeed));
	idcode &= OMAP_CHIP_ID_SPEED_MASK;

	if (idcode == OMAP_CHIP_ID_1000MHZ) //According to TI, CUSD100 were not tested to 105°C at any CPU speed
	{//Chip supports 1GHz and boot parameters configured to 1GHz
		Cds_Info.MJT = (Cds_Info.MJT > 99) ? 99 :  Cds_Info.MJT; //110°C
		Cds_Info.IT  = (Cds_Info.IT  > 93) ? 93 :  Cds_Info.IT;  //100°C
		Cds_Info.LTT = (Cds_Info.LTT > 90) ? 90 :  Cds_Info.LTT; //95°C
	}
	else
	{//Maximal parameters for 800MHz - Any chip
		Cds_Info.MJT = (Cds_Info.MJT > 107) ? 107 :  Cds_Info.MJT; //125°C
		Cds_Info.IT  = (Cds_Info.IT  > 101) ? 101 :  Cds_Info.IT;  //115°C
		Cds_Info.LTT = (Cds_Info.LTT > 99 ) ? 99  :  Cds_Info.LTT; //110°C
		Cds_Info.MaxSpeed = 800; //limit maximal speed if chip doesn't support it
	}

	RETAILMSG(1, (L"CDS Post [MJT] = %d, [IT] = %d , [LTT] = %d , [FeatStat] = %d , [MaxSpeed] = %d\r\n",
		Cds_Info.MJT	,Cds_Info.IT	,Cds_Info.LTT	,Cds_Info.FeatStat	,Cds_Info.MaxSpeed));

	/* All is fine */
	return TRUE;

cleanUp:
	RETAILMSG(1, (L"Error during CdsInitRegistryParameters\r\n"));
	return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  CDS_Deinit
//
//  Called by device manager to uninitialize device.
//
BOOL
CDS_Deinit(
    DWORD context
    )
{
    BOOL rc = FALSE;
    CDS_HW_INFO *pDevice = (CDS_HW_INFO*)context;

    RETAILMSG(1, (L"+CDS_Deinit(0x%08x)\r\n", context));

    // Signal stop to threads
    pDevice->intrThreadExit = TRUE;

    // Close interrupt thread
    if(pDevice->hTshutThread != NULL)
	{
		// Close handle
		CloseHandle(pDevice->hTshutThread);
	}

	//deinit TSHUT related handles
	if (pDevice->hTshutEvent != NULL)
	{
		if(!CloseHandle(pDevice->hTshutEvent))
			RETAILMSG(1, (L"CDS_Deinit: GPIO IST event [0x%x] close failure\r\n", pDevice->hTshutEvent));

		GPIOInterruptDisable(pDevice->hGpio, GPIO_127);
	}
	if (pDevice->hGpio)
	{
		GPIOClose(pDevice->hGpio);
	}

	RETAILMSG(1, (L"CDS_Deinit: pDevice->hTpModule [0x%x] \r\n", pDevice->hTpModule));

	if (pDevice->hTpModule != INVALID_HANDLE_VALUE)
	{
		TemperatureSensorDeregister(g_pDevice->hTpModule);
		TemperatureSensorClose(g_pDevice->hTpModule);
	}
	if (pDevice->queueNameTpModule != 0)//Memory allocation failure returns 0
	{
		LocalFree(pDevice->queueNameTpModule);
	}

	if (pDevice->hTpMsgQueue != INVALID_HANDLE_VALUE)
	{
		CloseMsgQueue(pDevice->hTpMsgQueue);
	}

	if (pDevice->hTpThread != INVALID_HANDLE_VALUE)
	{
		CloseHandle(pDevice->hTpThread);
	}

	if (pDevice->hInitDoneEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle(pDevice->hInitDoneEvent);
	}

	RegCloseKey(pDevice->hRegKey);

    // Delete critical section
    DeleteCriticalSection(&Cds_Info.cs);

    // Free device structure
    LocalFree(pDevice);

    // Done
    rc = TRUE;

    RETAILMSG(1, (L"-CDS_Deinit(rc = %d)\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  CDS_Init
//
//  Called by device manager to initialize device.
//
DWORD CDS_Init(LPCTSTR szContext, LPCVOID pBusContext)
{
	DWORD dwSize, rc = (DWORD)NULL;
	CDS_HW_INFO *pDevice = NULL;
	PHYSICAL_ADDRESS    pA;
    WCHAR szEventName[MAX_PATH];
	LONG lStatus;

	RETAILMSG(1, (L"+CDS_Init(%s, 0x%08x)\r\n", szContext, pBusContext));

	Cds_Info.EntryNumber = 0;

	// Create device structure
	pDevice = (CDS_HW_INFO *)LocalAlloc(LPTR, sizeof(CDS_HW_INFO));

	if(pDevice == NULL)
	{
		RETAILMSG(1, (L"ERROR: CDS_Init: Failed allocate CDS driver structure\r\n"));
		goto cleanUp;
	}

	g_pDevice = pDevice;

	// VA for MPU clock setting within interrupt handler
	pA.QuadPart = OMAP_PRCM_MPU_CM_REGS_PA;
	Cds_Info.pPrcmMpuCM = (OMAP_PRCM_MPU_CM_REGS*)MmMapIoSpace( pA, sizeof(OMAP_PRCM_MPU_CM_REGS), FALSE);
	pA.QuadPart = OMAP_PRCM_IVA2_CM_REGS_PA;
	Cds_Info.pPrcmIvaCM = (OMAP_PRCM_IVA2_CM_REGS*)MmMapIoSpace( pA, sizeof(OMAP_PRCM_IVA2_CM_REGS), FALSE);
	pA.QuadPart = OMAP_PRCM_GLOBAL_PRM_REGS_PA;
	Cds_Info.pPrcmGblPRM = (OMAP_PRCM_GLOBAL_PRM_REGS*)MmMapIoSpace( pA, sizeof(OMAP_PRCM_GLOBAL_PRM_REGS), FALSE);

	pA.QuadPart = OMAP_PRCM_IVA2_PRM_REGS_PA;
	Cds_Info.pPrcmIva2PRM = (OMAP_PRCM_IVA2_PRM_REGS*)MmMapIoSpace( pA, sizeof(OMAP_PRCM_IVA2_PRM_REGS), FALSE);

	pA.QuadPart = OMAP_SYSC_GENERAL_REGS_PA;
	Cds_Info.pSysGenReg= (OMAP_SYSC_GENERAL_REGS*)MmMapIoSpace( pA, sizeof(OMAP_SYSC_GENERAL_REGS), FALSE);

	// initialize memory
	//
	memset(pDevice, 0, sizeof(CDS_HW_INFO));
	//pDevice->queueNameTpModule = 0;
	pDevice->hTpModule   = INVALID_HANDLE_VALUE;
	pDevice->hTpMsgQueue = INVALID_HANDLE_VALUE;
	pDevice->hTpThread   = INVALID_HANDLE_VALUE;
	pDevice->hInitDoneEvent = INVALID_HANDLE_VALUE;

	// Initialize crit section
	InitializeCriticalSection(&Cds_Info.cs);

	//Initialize Temperature Sensor constant reading
	OUTREG32(&Cds_Info.pSysGenReg->CONTROL_TEMP_SENSOR, 0);
	OUTREG32(&Cds_Info.pSysGenReg->CONTROL_TEMP_SENSOR, 0x600);
	// wait for at least 2000us
	StallExecution(20000);

	// Open registry key holding name of CDS initialization done Event
	pDevice->hRegKey = NULL;
	lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, CDS_REG_KEY, 0, 0, &pDevice->hRegKey);
	if (ERROR_SUCCESS == lStatus)
	{	
		dwSize = sizeof(szEventName);
		lStatus = RegQueryValueEx(pDevice->hRegKey, TEXT("InitDoneEventName"), NULL, NULL, (LPBYTE)szEventName, &dwSize);
		if (ERROR_SUCCESS == lStatus)
		{	//Create Init Done Event
			pDevice->hInitDoneEvent = CreateEvent (NULL, TRUE, FALSE, szEventName);
		}
		else
		{
			RETAILMSG(1, (L"ERROR: CDS_Init Error acquiring: InitDoneEventName\r\n"));
			goto cleanUp;
		}
	}
	else
	{
		RETAILMSG(1, (L"ERROR: CDS_Init Error Opening Registry key: %s\r\n", CDS_REG_KEY));
		goto cleanUp;
	}

	//Acquire CDS Registry Operational parameters
	if (!CdsInitRegistryParameters(pDevice))
	{
		RETAILMSG (1, (L"ERROR: CDS_Init: Failed to CDS Registry Parameters\r\n"));
		goto cleanUp;
	}

	//Open registry values holding CDS statistics 
	if (!CdsInitStatistics(pDevice))
	{
		RETAILMSG (1, (L"ERROR: CDS_Init: Failed to initialize statistics\r\n"));
		goto cleanUp;
	}

	//Start CDS installable ISR which handles interrupt from GP Timer 5 
	if (!CdsInitStartGptimer(pDevice))
	{
		RETAILMSG (1, (L"ERROR: CDS_Init: Failed to start GPtimer 5\r\n"));
		goto cleanUp;
	}

	//Thermal Shutdown
	pDevice->hTwl = TWLOpen();

	//Initialize TSHUT capture
	if (!CdsInitTshutGPIO(pDevice))
	{
		RETAILMSG (1, (L"ERROR: CDS_Init: Failed to initialize TSHUT\r\n"));
		goto cleanUp;
	}
	Cds_Info.BlockOneGhz = 0;
#if 0
	//Initialize Companion Chip Temperature sensor monitoring capture
	if (!CdsInitCompanionChipTempRead(pDevice))
	{
		RETAILMSG (1, (L"ERROR: CDS_Init: Failed to initialize Compnion Chip Temperature reading\r\n"));
		goto cleanUp;
	}
#endif
	// Return non-null value
	rc = (DWORD)pDevice;

cleanUp:
	if(rc == 0)
	{
		CDS_Deinit((DWORD)pDevice);
	}

	RETAILMSG(1, (L"-CDS_Init(rc = %X)\r\n", rc));

	return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  CDS_Open
//
//  Called by device manager to open a device for reading and/or writing.
//
DWORD
CDS_Open(
    DWORD pHead, 
    DWORD AccessCode, 
    DWORD ShareMode
    )
{
	PCDS_HW_INFO   pCdsHead = (PCDS_HW_INFO)pHead;
    PCDS_OPEN_INFO pOpenHead;

	RETAILMSG(1, (L"CDS_Open handle 0x%X, access 0x%X, share 0x%X\r\n",
                                        pHead, AccessCode, ShareMode));

    // Return NULL if CDS_Init failed.
    if ( pCdsHead == 0) {
		RETAILMSG (1,(L"Error: CDS_Open CDS failed as Init failed to allocate\r\n"));
        SetLastError(ERROR_INVALID_HANDLE);
        return(NULL);
    }

    if (AccessCode & DEVACCESS_BUSNAMESPACE ) {
        AccessCode &=~(GENERIC_READ |GENERIC_WRITE|GENERIC_EXECUTE|GENERIC_ALL);
    }

    // Return NULL if someone else already has
	if (pCdsHead->pAccessOwner) {
        RETAILMSG (1,(L"Open requested access %x, handle x%X already has x%X!\r\n",
                   AccessCode, pCdsHead->pAccessOwner,
                   pCdsHead->pAccessOwner->AccessCode));
        SetLastError(ERROR_INVALID_ACCESS);
        return(NULL);
    }

    // OK, lets allocate an open structure
    pOpenHead    =  (PCDS_OPEN_INFO)LocalAlloc(LPTR, sizeof(CDS_OPEN_INFO));
    if ( !pOpenHead ) {
        RETAILMSG(1,(L"Error allocating memory for pOpenHead, CDS_Open failed\r\n"));
        return(NULL);
    }

    // Init the structure
    pOpenHead->pCdsHead = pCdsHead;  // pointer back to our parent
    pOpenHead->AccessCode = AccessCode;
    pOpenHead->ShareMode = ShareMode;

    // if we have access permissions, note it in pCdsHead
    if ( AccessCode & (GENERIC_READ | GENERIC_WRITE) ) {
        RETAILMSG(1,(L"CDS_Open: Access permission handle granted x%X\r\n",
                  pOpenHead));
        pCdsHead->pAccessOwner = pOpenHead;
    }


    RETAILMSG (1, (L"-CDS_Open handle x%X, x%X\r\n",
                                        pOpenHead, pOpenHead->pCdsHead));

    return (DWORD)pOpenHead;

}

//------------------------------------------------------------------------------
//
//  Function:  CDS_Close
//
//  This function closes the device context.
//
BOOL
CDS_Close(
    DWORD context
    )
{
    PCDS_OPEN_INFO pOpenHead = (PCDS_OPEN_INFO)context;
	PCDS_HW_INFO   pCdsHead  = pOpenHead->pCdsHead;

    RETAILMSG (1, (TEXT("+CDS_Close\r\n")));

    if ( pCdsHead == INVALID_HANDLE_VALUE) {
		RETAILMSG (1, (TEXT("!!CDS_Close: pCdsHead == INVALID_HANDLE_VALUE!!\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

	// If this was the handle with access permission, remove pointer
	if ( pOpenHead == pCdsHead->pAccessOwner ) {
		RETAILMSG (1, (TEXT("CDS_Close: Closed access owner handle\r\n")));
		pCdsHead->pAccessOwner = NULL;
	}

    LocalFree( pOpenHead );

	RETAILMSG (1, (TEXT("-CDS_Close\r\n")));

	return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  CDS_IOControl
//
//  This function sends a command to a device.
//
BOOL
CDS_IOControl(
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
    CDS_HW_INFO *pDevice = (CDS_HW_INFO*)context;
	INT16* OutBuffer = (INT16*) pOutBuffer;

    RETAILMSG(1, (L"+CDS_IOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",
		context, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize));

	switch(code)
	{
	case IOCTL_CPUDYNAMICSPEED_GETSTATUS:
		{
			DWORD dwLen;
			DWORD* InfoPointer = NULL;
			if (cdsIsrHandle_gpt != INVALID_HANDLE_VALUE)
			{
				if( KernelLibIoControl(cdsIsrHandle_gpt, IOCTL_CDS_ISR_GET_STATUS, NULL, 0, &InfoPointer, sizeof(void *), &dwLen) )
				{
					RETAILMSG(1, (L"CDS IOCTL Get status: CDS InfoPointer = 0x%X EntryNumber[%x]\r\n", InfoPointer, Cds_Info.EntryNumber));
					Cds_Info.AtoDRead = INREG32(&Cds_Info.pSysGenReg->CONTROL_TEMP_SENSOR);
					RETAILMSG(1, (L"Temperature regular = 0x%X ISR Context = 0x%x CC = %d\r\n", Cds_Info.AtoDRead, Cds_Info.AtoDReadIntCon, Cds_Info.CCTempRead));
					OutBuffer[0] = (INT16)Cds_Info.AtoDRead;
					OutBuffer[1] = Cds_Info.CCTempRead;
					*pOutSize = outSize;
				}
			}
			else
			{
				*pOutBuffer = (UCHAR)INREG32(&Cds_Info.pSysGenReg->CONTROL_TEMP_SENSOR);
				RETAILMSG(1, (L"Temperature TJ = %d\r\n", *pOutBuffer));
				*pOutSize = outSize;
			}
		}
		rc = TRUE;
		break;


	case IOCTL_CPUDYNAMICSPEED_SET_SPEED:
		{
			if ((*pInBuffer < OPM_600MHz) || (*pInBuffer > OPM_1GHz)) {
				RETAILMSG(1, (L"ERROR: CDS_IOControl: Illegal Set Speed value. Current opm [%d]\r\n", Cds_Info.CurrentOpm));
				rc = FALSE;
				break;
			}

			if(KernelLibIoControl(cdsIsrHandle_gpt, IOCTL_CDS_ISR_SET_OPM, pInBuffer, inSize, NULL, 0, 0) )
			{
				RETAILMSG(1, (L"CDS_IOControl: Set Speed Current opm [%d]\r\n", Cds_Info.CurrentOpm));
				if (pOutBuffer) {
					*pOutBuffer = *pInBuffer;
				}
				CdsUpdateMpuInfo();
				rc = TRUE;
			}
			else
			{
				RETAILMSG(1, (L"CDS_IOControl: IOCTL CDS ISR Failure\r\n"));
				rc = FALSE;
			}
		}
		break;

	case IOCTL_CPUDYNAMICSPEED_GET_SPEED:
		*pOutBuffer = Cds_Info.CurrentOpm;
		*pOutSize = outSize;
		break;

	default:
		rc = TRUE;
		break;
	}

	RETAILMSG(1, (L"-CDS_IOControl(rc = %d)\r\n", rc));
	return rc;
}


BOOL WINAPI DllMain( HANDLE hModule, 
					DWORD  ul_reason_for_call, 
					LPVOID lpReserved
					)
{

	RETAILMSG (1, (L"CDS Entry ul_reason_for_call[%d]\r\n",ul_reason_for_call));

	switch(ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		RETAILMSG (1, (TEXT("CPU Dynamic Speed DllMain(): dll attach.\r\n")));
		// don't need thread attach/detach messages
		DisableThreadLibraryCalls ((HINSTANCE)hModule);
		break;

    case DLL_PROCESS_DETACH:
		RETAILMSG (1, (TEXT("CPU Dynamic Speed DllMain(): dll detach.\r\n")));
        break;
    }
    return TRUE;
}

