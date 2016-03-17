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
//  File:  power.c
//
//  Power BSP callback functions implementation. This function are called as
//  last function before OALCPUPowerOff. The KITL was already disabled by
//  OALKitlPowerOff.
//
#include <windows.h>
#include <ceddk.h>
#include <bsp.h>
#include <nkintr.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>
#include <interrupt_struct.h>
#include <bus.h>
#include <oal_prcm.h>
#include <oal_sr.h>


//-----------------------------------------------------------------------------
//
//  Global:  g_PrcmPrm
//
//  Reference to all PRCM-PRM registers. Initialized in PrcmInit
//
extern OMAP_PRCM_PRM               g_PrcmPrm;

//-----------------------------------------------------------------------------
//
//  Global:  g_PrcmCm
//
//  Reference to all PRCM-CM registers. Initialized in PrcmInit
//
extern OMAP_PRCM_CM                g_PrcmCm;

//------------------------------------------------------------------------------
//
//  External:  g_pSysCtrlGenReg
//
//  reference to system control general register set
//
extern OMAP_SYSC_GENERAL_REGS     *g_pSysCtrlGenReg;

//-----------------------------------------------------------------------------
//
//  External:  g_pTimerRegs
//
//  References the GPTimer1 registers.  Initialized in OALTimerInit().
//
extern OMAP_GPTIMER_REGS          *g_pTimerRegs;

//-----------------------------------------------------------------------------
//
//  Static :  s_mpuPowerState
//
//  Saves the mpu power state to restore on wakeup
//  
static UINT32 s_mpuPowerState = 0;

//-----------------------------------------------------------------------------
//
//  Static :  s_enableSmartReflex
//
//  Saves smartreflex states 
//  
static BOOL s_enableSmartReflex1;
static BOOL s_enableSmartReflex2;

//-----------------------------------------------------------------------------
// prototypes
//
extern BOOL IsSmartReflexMonitoringEnabled(UINT channel);
extern BOOL SmartReflex_EnableMonitor(UINT channel, BOOL bEnable);
    
extern void *OALTritonOpen();
extern void *OALPCAOpen();
extern void OALTritonClose(void *);
extern void OALPCAClose(void *);
extern BOOL OALTritonRead(void *, DWORD, UCHAR *);
extern BOOL OALTritonWrite(void *, DWORD, UCHAR);
extern BOOL OALPCAWrite(void *, WORD, WORD, UCHAR);
extern BOOL OALPCARead(void *, WORD, WORD, UCHAR *);

VOID BSPEnablePWRONSource(INT32 en)
{
	void *hTwl;
	UCHAR imr, edr, isr, pih = 0, keyp, gpio1a, gpio2a, gpio3a;

	hTwl = (void *)OALTritonOpen();

	if(en)
	{
		//OALTritonRead(hTwl, TWL_PIH_ISR_P1, &pih);
		OALMSG(0, (L"%S: PIH(%X)\r\n", __FUNCTION__, pih));


		OALTritonRead(hTwl, TWL_PWR_IMR1, &imr);
//		OALMSG(0, (L"IMR(%X)\r\n", imr));
		OALTritonRead(hTwl, TWL_PWR_EDR1, &edr);
//		OALMSG(0, (L"EDR(%X)\r\n", edr));

		OALTritonRead(hTwl, TWL_KEYP_IMR1, &keyp);

		OALTritonRead(hTwl, TWL_GPIO_IMR1A, &gpio1a);
		OALTritonRead(hTwl, TWL_GPIO_IMR2A, &gpio2a);
		OALTritonRead(hTwl, TWL_GPIO_IMR3A, &gpio3a);

		OALTritonWrite(hTwl, TWL_BCIIMR1A, 0xFF);
		OALTritonWrite(hTwl, TWL_BCIIMR2A, 0xFF);
		OALTritonWrite(hTwl, TWL_BCIIMR1B, 0xFF);
		OALTritonWrite(hTwl, TWL_BCIIMR2B, 0xFF);

		OALTritonWrite(hTwl, TWL_MADC_ISR1, 0xFF);
		OALTritonWrite(hTwl, TWL_MADC_IMR1, 0xFF);
		OALTritonWrite(hTwl, TWL_MADC_ISR2, 0xFF);
		OALTritonWrite(hTwl, TWL_MADC_IMR2, 0xFF);

		OALTritonWrite(hTwl, TWL_KEYP_ISR1, 0xFF);
		OALTritonWrite(hTwl, TWL_KEYP_ISR2, 0xFF);
		OALTritonWrite(hTwl, TWL_KEYP_IMR1, 0xFF);
		OALTritonWrite(hTwl, TWL_KEYP_IMR2, 0xFF);
		
		
		OALTritonWrite(hTwl, TWL_PWR_ISR1, 0xFF);
		OALTritonWrite(hTwl, TWL_PWR_IMR1, 0xFF);
		OALTritonWrite(hTwl, TWL_PWR_ISR2, 0xFF);
		OALTritonWrite(hTwl, TWL_PWR_IMR2, 0xFF);
    
		OALTritonWrite(hTwl, TWL_GPIO_IMR1A, 0xFF);
		OALTritonWrite(hTwl, TWL_GPIO_IMR2A, 0xFF);
		OALTritonWrite(hTwl, TWL_GPIO_IMR3A, 0xFF);
		OALTritonWrite(hTwl, TWL_GPIO_IMR1B, 0xFF);
		OALTritonWrite(hTwl, TWL_GPIO_IMR2B, 0xFF);
		OALTritonWrite(hTwl, TWL_GPIO_IMR3B, 0xFF);

		//  Ensure that that there are no pending interrupts
		//  on power and MADC
		OALTritonWrite(hTwl, TWL_PWR_ISR1, 0xFF);
		OALTritonWrite(hTwl, TWL_PWR_ISR2, 0xFF);
		OALTritonWrite(hTwl, TWL_MADC_ISR1, 0xFF);
		OALTritonWrite(hTwl, TWL_MADC_ISR2, 0xFF);


		OALTritonWrite(hTwl, TWL_PWR_EDR1, edr | TWL_PWRON_FALLING | TWL_PWRON_RISING);
		OALTritonWrite(hTwl, TWL_KEYP_IMR1, keyp);
		OALTritonWrite(hTwl, TWL_GPIO_IMR1A, gpio1a);
		OALTritonWrite(hTwl, TWL_GPIO_IMR2A, gpio2a);
		OALTritonWrite(hTwl, TWL_GPIO_IMR3A, gpio3a);


		OALTritonWrite(hTwl, TWL_PWR_IMR1, imr & (~1));
		OALTritonRead(hTwl, TWL_PWR_ISR1, &isr);
		OALMSG(0, (L"%S: PWR ISR(%X)\r\n", __FUNCTION__, isr));
		isr &= 1;
		OALTritonWrite(hTwl, TWL_PWR_ISR1, isr);

		OALTritonRead(hTwl, TWL_PWR_ISR1, &isr);
		isr &= 1;
		OALTritonWrite(hTwl, TWL_PWR_ISR1, isr);
	}
	else
	{
//		OALTritonRead(hTwl, TWL_PIH_ISR_P1, &pih);
		OALMSG(0, (L"%S: PIH(%X)\r\n", __FUNCTION__, pih));

		OALTritonRead(hTwl, TWL_PWR_ISR1, &isr);
		OALMSG(1, (L"%S: PWR ISR(%X)\r\n", __FUNCTION__, isr));
		isr &= 1;
		OALTritonWrite(hTwl, TWL_PWR_ISR1, isr);

		OALTritonRead(hTwl, TWL_PWR_ISR1, &isr);
		isr &= 1;
		OALTritonWrite(hTwl, TWL_PWR_ISR1, isr);

		OALTritonRead(hTwl, TWL_PWR_IMR1, &imr);
		OALTritonWrite(hTwl, TWL_PWR_IMR1, imr | 1);

	}
	OALTritonClose(hTwl);
}
//------------------------------------------------------------------------------
// WARNING: This function is called from OEMPowerOff - no system calls, critical 
// sections, OALMSG, etc., may be used by this function or any function that it calls.
//------------------------------------------------------------------------------
VOID BSPPowerOff()
{
	BOOL  bShutdown = FALSE;
	DWORD dwSize;

	OALMSGS(1, (L"BSPPowerOff\r\n"));
		// shutdown the device according to the global flag
	OALIoCtlHalPresuspend(0,0,0,&bShutdown,sizeof(bShutdown),&dwSize);
	if(bShutdown)
	{
		OALIoCtlHalShutdown(0,0,0,0,0,0);
	}

    // Disable Smartreflex if enabled.
    if (IsSmartReflexMonitoringEnabled(kSmartReflex_Channel1))
        {
        s_enableSmartReflex1 = TRUE;
        SmartReflex_EnableMonitor(kSmartReflex_Channel1, FALSE);
        }
    else
        {
        s_enableSmartReflex1 = FALSE;
        }

    if (IsSmartReflexMonitoringEnabled(kSmartReflex_Channel2))
        {
        s_enableSmartReflex2 = TRUE;
        SmartReflex_EnableMonitor(kSmartReflex_Channel2, FALSE);
        }
    else
        {
        s_enableSmartReflex2 = FALSE;
        }

    // clear wake-up enable capabilities for gptimer1
    CLRREG32(&g_PrcmPrm.pOMAP_WKUP_PRM->PM_WKEN_WKUP, CM_CLKEN_GPT1);

    // stop GPTIMER1
    OUTREG32(&g_pTimerRegs->TCLR, INREG32(&g_pTimerRegs->TCLR) & ~(GPTIMER_TCLR_ST));
    
    if (0)//g_oalRetailMsgEnable)
    	{
        OEMDeinitDebugSerial();
	    }
}

//------------------------------------------------------------------------------
// WARNING: This function is called from OEMPowerOff - no system calls, critical 
// sections, OALMSG, etc., may be used by this function or any function that it calls.
//------------------------------------------------------------------------------
VOID BSPPowerOn()
{
	int	i;
	UINT32 out;
    OMAP_GPIO_REGS *pGPIO4 = OALPAtoUA(OMAP_GPIO4_REGS_PA);

    // reset wake-up enable capabilities for gptimer1
    SETREG32(&g_PrcmPrm.pOMAP_WKUP_PRM->PM_WKEN_WKUP, CM_CLKEN_GPT1);

    if (s_enableSmartReflex1)
    {
        SmartReflex_EnableMonitor(kSmartReflex_Channel1, TRUE);
    }

    if (s_enableSmartReflex2)
    {
        SmartReflex_EnableMonitor(kSmartReflex_Channel2, TRUE);
    } 

    if(0)//g_oalRetailMsgEnable)
	{
		OEMInitDebugSerial();
	}

    g_ResumeRTC = TRUE;

	i = 100;
	out = 0;
	OALMSGS(1, (L"%S: check the power presence\r\n", __FUNCTION__));
	do
	{
		if(INREG32(&pGPIO4->DATAIN) & (1 << 30))
		{
			out++;
			if(out > 100/3)
				break;
		}
		else
			out = 0;
	}while(i--);

	if(out)
	{
		void *hTwl;
		out = TWL_RTC_STATUS_ALARM;

		hTwl = (void *)OALTritonOpen();
		OALTritonRead(hTwl, TWL_RTC_STATUS_REG, &out);
		if(out & TWL_RTC_STATUS_ALARM)
		{
			OALTritonWrite(hTwl, TWL_RTC_STATUS_REG, out);
			OALTritonWrite(hTwl, TWL_BACKUP_REG_A, 0);
			OALTritonWrite(hTwl, TWL_BACKUP_REG_B, 0);
			OALTritonWrite(hTwl, TWL_BACKUP_REG_C, 0);
			OALTritonWrite(hTwl, TWL_BACKUP_REG_D, 0);
			OALTritonWrite(hTwl, TWL_BACKUP_REG_E, 0);
			OALTritonWrite(hTwl, TWL_BACKUP_REG_F, 0);
		}
		OALIoCtlHalShutdown(0,0,0,0,0,0);
	}

	OALMSGS(0, (L"BSPPowerOn\r\n"));
}

//------------------------------------------------------------------------------

#if 0

#define DISPLAY_REGISTER_VALUE(p, regname) OALMSG(1, (L"%s = 0x%08x\r\n", L#regname, p->regname))

void Dump_SDRC()
{
    OMAP_SDRC_REGS *pRegs = (OMAP_SDRC_REGS*)OALPAtoUA(OMAP_SDRC_REGS_PA);

    DISPLAY_REGISTER_VALUE(pRegs, SDRC_REVISION);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_SYSCONFIG);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_STATUS);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_CS_CFG);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_SHARING);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_ERR_ADDR);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_ERR_TYPE);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_DLLA_CTRL);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_DLLA_STATUS);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_DLLB_CTRL);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_DLLB_STATUS);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_POWER);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_MCFG_0);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_MR_0);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_EMR1_0);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_EMR2_0);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_DCDL1_CTRL);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_DCDL2_CTRL);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_ACTIM_CTRLA_0);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_ACTIM_CTRLB_0);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_RFR_CTRL_0);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_MANUAL_0);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_MCFG_1);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_MR_1);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_EMR1_1);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_EMR2_1);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_ACTIM_CTRLA_1);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_ACTIM_CTRLB_1);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_RFR_CTRL_1);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_MANUAL_1);
}

#endif
