#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <oalex.h>
#include <cds.h>
#include <omap35xx.h>
#include <bsp_cfg.h>
#include <cdsInterface.h>

PCDS_GPT_INFO pCdsInfo = NULL;
DWORD CdsInstNumInstances = 0;

void CdsIsrClockSetup(UINT32 opm)
{
	unsigned int val;

	// put mpu dpll1 in bypass
	val = INREG32(&pCdsInfo->pPrcmMpuCM->CM_CLKEN_PLL_MPU);
	val &= ~DPLL_MODE_MASK;
	val |= DPLL_MODE_LOWPOWER_BYPASS;
	OUTREG32(&pCdsInfo->pPrcmMpuCM->CM_CLKEN_PLL_MPU, val);
	while ((INREG32(&pCdsInfo->pPrcmMpuCM->CM_IDLEST_PLL_MPU) & DPLL_STATUS_MASK) != DPLL_STATUS_BYPASSED);

	// setup DPLL1 divider
	OUTREG32(&pCdsInfo->pPrcmMpuCM->CM_CLKSEL2_PLL_MPU, BSP_CM_CLKSEL2_PLL_MPU);

	// configure m:n clock ratios as well as frequency selection for mpu dpll
	if(OPM_1GHz == opm)
		val = BSP_CM_CLKSEL1_PLL_MPU_1000;
	else if(OPM_800MHz == opm)
		val = BSP_CM_CLKSEL1_PLL_MPU_800;
	else
		val = BSP_CM_CLKSEL1_PLL_MPU_600;

	OUTREG32(&pCdsInfo->pPrcmMpuCM->CM_CLKSEL1_PLL_MPU, val);

	// lock dpll1 with correct frequency selection
	OUTREG32(&pCdsInfo->pPrcmMpuCM->CM_CLKEN_PLL_MPU, BSP_CM_CLKEN_PLL_MPU);
	while ((INREG32(&pCdsInfo->pPrcmMpuCM->CM_IDLEST_PLL_MPU) & DPLL_STATUS_MASK) != DPLL_STATUS_LOCKED);
}

void CdsIsrUpdateVoltageLevels(UINT32 opm)
{
	UINT32 val;
	// setup i2c for smps communication
	OUTREG32(&pCdsInfo->pPrcmGblPRM->PRM_VC_SMPS_SA, BSP_VC_SMPS_SA_INIT);
	OUTREG32(&pCdsInfo->pPrcmGblPRM->PRM_VC_SMPS_VOL_RA, BSP_VC_SMPS_VOL_RA_INIT);
	OUTREG32(&pCdsInfo->pPrcmGblPRM->PRM_VC_SMPS_CMD_RA, BSP_VC_SMPS_CMD_RA_INIT);
	OUTREG32(&pCdsInfo->pPrcmGblPRM->PRM_VC_CH_CONF, BSP_VC_CH_CONF_INIT);  
	OUTREG32(&pCdsInfo->pPrcmGblPRM->PRM_VC_I2C_CFG, BSP_PRM_VC_I2C_CFG_INIT);

	// set intial voltage levels
	if(OPM_1GHz == opm)
		val = BSP_PRM_VC_CMD_VAL_0_INIT4;
	else if(OPM_800MHz == opm)
		val = BSP_PRM_VC_CMD_VAL_0_INIT3;
	else
		val = BSP_PRM_VC_CMD_VAL_0_INIT2;
	OUTREG32(&pCdsInfo->pPrcmGblPRM->PRM_VC_CMD_VAL_0, val);
	OUTREG32(&pCdsInfo->pPrcmGblPRM->PRM_VC_CMD_VAL_1, BSP_PRM_VC_CMD_VAL_1_INIT);

	// set PowerIC error offset, gains, and initial voltage
	if(OPM_1GHz == opm)
		val = BSP_PRM_VP1_CONFIG_INIT_4;
	else if(OPM_800MHz == opm)
		val = BSP_PRM_VP1_CONFIG_INIT_3;
	else
		val = BSP_PRM_VP1_CONFIG_INIT_2;
	OUTREG32(&pCdsInfo->pPrcmGblPRM->PRM_VP1_CONFIG, val);
	OUTREG32(&pCdsInfo->pPrcmGblPRM->PRM_VP2_CONFIG, BSP_PRM_VP2_CONFIG_INIT);

	// set PowerIC slew range
	OUTREG32(&pCdsInfo->pPrcmGblPRM->PRM_VP1_VSTEPMIN, BSP_PRM_VP1_VSTEPMIN_INIT);
	OUTREG32(&pCdsInfo->pPrcmGblPRM->PRM_VP1_VSTEPMAX, BSP_PRM_VP1_VSTEPMAX_INIT);
	OUTREG32(&pCdsInfo->pPrcmGblPRM->PRM_VP2_VSTEPMIN, BSP_PRM_VP2_VSTEPMIN_INIT);
	OUTREG32(&pCdsInfo->pPrcmGblPRM->PRM_VP2_VSTEPMAX, BSP_PRM_VP2_VSTEPMAX_INIT);

	// set PowerIC voltage limits and timeout
	OUTREG32(&pCdsInfo->pPrcmGblPRM->PRM_VP1_VLIMITTO, BSP_PRM_VP1_VLIMITTO_INIT);
	OUTREG32(&pCdsInfo->pPrcmGblPRM->PRM_VP2_VLIMITTO, BSP_PRM_VP2_VLIMITTO_INIT);

	// enable voltage processors
	SETREG32(&pCdsInfo->pPrcmGblPRM->PRM_VP1_CONFIG, SMPS_VPENABLE);
	SETREG32(&pCdsInfo->pPrcmGblPRM->PRM_VP2_CONFIG, SMPS_VPENABLE);

	// enable timeout
	SETREG32(&pCdsInfo->pPrcmGblPRM->PRM_VP1_CONFIG, SMPS_TIMEOUTEN);
	SETREG32(&pCdsInfo->pPrcmGblPRM->PRM_VP2_CONFIG, SMPS_TIMEOUTEN);    

	// flush commands to smps
	SETREG32(&pCdsInfo->pPrcmGblPRM->PRM_VP1_CONFIG, SMPS_FORCEUPDATE | SMPS_INITVDD);
	SETREG32(&pCdsInfo->pPrcmGblPRM->PRM_VP2_CONFIG, SMPS_FORCEUPDATE | SMPS_INITVDD);

	// allow voltage to settle
	StallExecution(10000);

	// disable voltage processors
	CLRREG32(&pCdsInfo->pPrcmGblPRM->PRM_VP1_CONFIG, SMPS_VPENABLE);
	CLRREG32(&pCdsInfo->pPrcmGblPRM->PRM_VP2_CONFIG, SMPS_VPENABLE);
}

DWORD ISRHandler( DWORD InstanceIndex )
{
	DWORD retSysInt = SYSINTR_NOP;//SYSINTR_CHAIN;// By default don't notify CDS Timer monitoring thread
	INT16 lA2DRead, lCurrentOpm;
	if (pCdsInfo)
	{
		//Debug
		pCdsInfo->EntryNumber++;

		if (pCdsInfo->FeatStat & CDS_BIT_FEATURE_EN) //Check if feature is enabled
		{
			//Read Temperature sensor register
			pCdsInfo->AtoDReadIntCon = INREG32(&pCdsInfo->pSysGenReg->CONTROL_TEMP_SENSOR);
			lA2DRead = (INT16)pCdsInfo->AtoDReadIntCon & 0xff; //Use lower 8 bits

			lCurrentOpm = pCdsInfo->CurrentOpm;

			//Breached MJT. Notify CDS and reduce MPU speed to 600MHz
			if (lA2DRead >= pCdsInfo->MJT)
			{
				if (lCurrentOpm != OPM_600MHz) {
					CdsIsrClockSetup(OPM_600MHz); 
					CdsIsrUpdateVoltageLevels(OPM_600MHz);
					pCdsInfo->CurrentOpm = OPM_600MHz;
					//CDS will save number breaches into registry
					pCdsInfo->pCdsStat->UpdateStats |= CDS_STATS_BREACH_FLAG;
					retSysInt = pCdsInfo->SysIntr;
				}
				//CDS will save maximal temperature into registry
				if (pCdsInfo->pCdsStat->MaximalADCValue < lA2DRead) {
					pCdsInfo->pCdsStat->MaximalADCValue = lA2DRead;
					pCdsInfo->pCdsStat->UpdateStats |= CDS_STATS_MAX_TJ_FLAG;
					retSysInt = pCdsInfo->SysIntr;
				}
			}
			if (pCdsInfo->MaxSpeed == 1000)//check if lower than LTT only if 1GHz is enabled
			{//if 1GHz is enabled
				if ((lA2DRead <= pCdsInfo->LTT ) && (pCdsInfo->CurrentOpm != OPM_1GHz) && (pCdsInfo->BlockOneGhz == FALSE))
				{
					CdsIsrClockSetup(OPM_1GHz); 
					CdsIsrUpdateVoltageLevels(OPM_1GHz);
					pCdsInfo->CurrentOpm = OPM_1GHz;
					retSysInt = pCdsInfo->SysIntr;
				}
				if ((pCdsInfo->BlockOneGhz == TRUE) && (lCurrentOpm == OPM_1GHz))//Block 1GHz speed if Companion ship is overheating
				{
					CdsIsrClockSetup(OPM_800MHz); 
					CdsIsrUpdateVoltageLevels(OPM_800MHz);
					pCdsInfo->CurrentOpm = OPM_800MHz;
					retSysInt = pCdsInfo->SysIntr;
				}
			}
			if (lA2DRead <= pCdsInfo->IT)//If Reduced to 600MHz increase to 800MHz if temperature is under IT
			{
				if (lCurrentOpm == OPM_600MHz)
				{
					CdsIsrClockSetup(OPM_800MHz); 
					CdsIsrUpdateVoltageLevels(OPM_800MHz);
					pCdsInfo->CurrentOpm = OPM_800MHz;
					retSysInt = pCdsInfo->SysIntr;
				}
			}
		}
		// Clear interrupt
		OUTREG32(&pCdsInfo->pGpt->TISR, 2);
		// re-enable interrupts		
		OUTREG32(&pCdsInfo->pICL->INTC_MIR_CLEAR1, 1 << (pCdsInfo->TimerIrQNum - 32));
	}
	return retSysInt;
}

DWORD CreateInstance( void )
{
	if (CdsInstNumInstances >= 1) // Only one is permitted
		return (DWORD)-1;
	else
		CdsInstNumInstances++;

	return CdsInstNumInstances;
}

void DestroyInstance( DWORD InstanceIndex )
{
	CdsInstNumInstances = 0;

	return;
}

BOOL IOControl( DWORD InstanceIndex, DWORD IoControlCode, LPVOID pInBuf, DWORD InBufSize, LPVOID pOutBuf, DWORD OutBufSize, LPDWORD pBytesReturned )
{
	switch (IoControlCode)
	{
	case IOCTL_CDS_ISR_SET_OPM:
		{
			EnterCriticalSection(&pCdsInfo->cs);

			pCdsInfo->CurrentOpm = *(INT16*)pInBuf;

			CdsIsrClockSetup(pCdsInfo->CurrentOpm); 
			CdsIsrUpdateVoltageLevels(pCdsInfo->CurrentOpm);

			LeaveCriticalSection(&pCdsInfo->cs);
		}
		break;
	case IOCTL_CDS_ISR_CONFIG:
		{
			pCdsInfo = (PCDS_GPT_INFO)pInBuf;
		}
		break;
	case IOCTL_CDS_ISR_GET_STATUS:
		*(LPDWORD)pOutBuf = (DWORD)pCdsInfo;
		OutBufSize = sizeof(PCDS_GPT_INFO);
		*pBytesReturned = sizeof(PCDS_GPT_INFO);
		break;
	}
	return TRUE;
}

BOOL __stdcall DllEntry( HINSTANCE hinstDll, DWORD dwReason, LPVOID lpReserved )
{
	switch ( dwReason ) 
	{
	case DLL_PROCESS_ATTACH:

		break;

	case DLL_PROCESS_DETACH:

		break;
	}

	return TRUE;
}
