// Copyright (c) 2007, 2013 2008 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
// Igor Lantsman
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
//  File:  main.c
//
//  This file implements main bootloader functions called from blcommon
//  library.
//
#include <eboot.h>
#include <boot_cfg.h>
#include <bsp.h>
#include <i2c.h>
#include <oal_i2c.h>
#include <omap35xx_prcm.h>
#include <constants.h>

#include <omap_cpuver.h>
#include <bsp_version.h>
#include <bsp_logo.h>


//------------------------------------------------------------------------------
// Local definitions
#define NUMBER_PROGRESS_RECTANGLES 10
#define TMP_SIZE					50

//----------------------------------------
// pattern for MLDR<-> host ommunications
#define MLDR_PATTERN_T  L"\r\nD:\\>"

extern	UINT32	g_y;

//------------------------------------------------------------------------------
//
//  Global: g_bootCfg
//
//  This global variable is used to save boot configuration.
//
BOOT_CFG g_bootCfg;

//------------------------------------------------------------------------------
//
//  Global: g_eboot
//
//  This global variable is used to save information about downloaded regions.
//
EBOOT_CONTEXT g_eboot;

UINT32 g_CPU_Rev;
//------------------------------------------------------------------------------
// External Functions

extern VOID JumpTo(UINT32 address);
extern VOID OEMDebugDeinit();

//------------------------------------------------------------------------------
//  Local Functions

//------------------------------------------------------------------------------
DWORD ProgressNotify()
{
	ShowProgressLoop();
	return 1;
}

//------------------------------------------------------------------------------
//
//  Function:  OALTritonSet
//
static void OALTritonSet(HANDLE hTwl, DWORD Register, BYTE Mask)
{
    BYTE regval;
    
    OALTritonRead(hTwl, Register, &regval);
    regval |= Mask;
    OALTritonWrite(hTwl, Register, regval);
}

//------------------------------------------------------------------------------
//
//  Function:  OALTritonClear
//
static void OALTritonClear(HANDLE hTwl, DWORD Register, BYTE Mask)
{
    BYTE regval;
    
    OALTritonRead(hTwl, Register, &regval);
    regval &= ~Mask;
    OALTritonWrite(hTwl, Register, regval);
}

//------------------------------------------------------------------------------
//
//  Function:  CpuGpioInput
//
static int CpuGpioInput(DWORD GpioNumber)
{
    UINT32 fWkup, iWkup;
    UINT32 fPer, iPer, f;
    OMAP_GPIO_REGS* pGpio;
    OMAP_PRCM_PER_CM_REGS* pPrcmPerCM = OALPAtoUA(OMAP_PRCM_PER_CM_REGS_PA);
    OMAP_PRCM_WKUP_CM_REGS* pPrcmWkupCM = OALPAtoUA(OMAP_PRCM_WKUP_CM_REGS_PA);

    // Enable clocks to GPIO modules
    fWkup = INREG32(&pPrcmWkupCM->CM_FCLKEN_WKUP);
    iWkup = INREG32(&pPrcmWkupCM->CM_ICLKEN_WKUP);
    fPer = INREG32(&pPrcmPerCM->CM_FCLKEN_PER);
    iPer = INREG32(&pPrcmPerCM->CM_ICLKEN_PER);
    SETREG32(&pPrcmWkupCM->CM_FCLKEN_WKUP, CM_CLKEN_GPIO1);
    SETREG32(&pPrcmWkupCM->CM_ICLKEN_WKUP, CM_CLKEN_GPIO1);
    SETREG32(&pPrcmPerCM->CM_FCLKEN_PER, CM_CLKEN_GPIO2|CM_CLKEN_GPIO3|CM_CLKEN_GPIO4|CM_CLKEN_GPIO5|CM_CLKEN_GPIO6);
    SETREG32(&pPrcmPerCM->CM_ICLKEN_PER, CM_CLKEN_GPIO2|CM_CLKEN_GPIO3|CM_CLKEN_GPIO4|CM_CLKEN_GPIO5|CM_CLKEN_GPIO6);

    switch (GpioNumber >> 5)
    {
        case 0:
            pGpio = OALPAtoUA(OMAP_GPIO1_REGS_PA);
            break;
            
        case 1:
            pGpio = OALPAtoUA(OMAP_GPIO2_REGS_PA);
            GpioNumber -= 32;
            break;

        case 2:
            pGpio = OALPAtoUA(OMAP_GPIO3_REGS_PA);
            GpioNumber -= 64;
            break;

        case 3:
            pGpio = OALPAtoUA(OMAP_GPIO4_REGS_PA);
            GpioNumber -= 96;
            break;

        case 4:
            pGpio = OALPAtoUA(OMAP_GPIO5_REGS_PA);
            GpioNumber -= 128;
            break;

        case 5:
            pGpio = OALPAtoUA(OMAP_GPIO6_REGS_PA);
            GpioNumber -= 160;
            break;

        default:
            return 0;
    }

    // make GPIO pin an input
    SETREG32(&pGpio->OE, 1 << GpioNumber);

    // set output state of GPIO pin
	f = INREG32(&pGpio->DATAIN) & (1 << GpioNumber);

    // Put clocks back
    OUTREG32(&pPrcmWkupCM->CM_FCLKEN_WKUP, fWkup);
    OUTREG32(&pPrcmWkupCM->CM_ICLKEN_WKUP, iWkup);
    OUTREG32(&pPrcmPerCM->CM_FCLKEN_PER, fPer);
    OUTREG32(&pPrcmPerCM->CM_ICLKEN_PER, iPer);


	return f;
}

//------------------------------------------------------------------------------
//
//  Function:  CpuGpioOutput
//
static void CpuGpioOutput(DWORD GpioNumber, DWORD Value)
{
    UINT32 fWkup, iWkup;
    UINT32 fPer, iPer;
    OMAP_GPIO_REGS* pGpio;
    OMAP_PRCM_PER_CM_REGS* pPrcmPerCM = OALPAtoUA(OMAP_PRCM_PER_CM_REGS_PA);
    OMAP_PRCM_WKUP_CM_REGS* pPrcmWkupCM = OALPAtoUA(OMAP_PRCM_WKUP_CM_REGS_PA);

    // Enable clocks to GPIO modules
    fWkup = INREG32(&pPrcmWkupCM->CM_FCLKEN_WKUP);
    iWkup = INREG32(&pPrcmWkupCM->CM_ICLKEN_WKUP);
    fPer = INREG32(&pPrcmPerCM->CM_FCLKEN_PER);
    iPer = INREG32(&pPrcmPerCM->CM_ICLKEN_PER);
    SETREG32(&pPrcmWkupCM->CM_FCLKEN_WKUP, CM_CLKEN_GPIO1);
    SETREG32(&pPrcmWkupCM->CM_ICLKEN_WKUP, CM_CLKEN_GPIO1);
    SETREG32(&pPrcmPerCM->CM_FCLKEN_PER, CM_CLKEN_GPIO2|CM_CLKEN_GPIO3|CM_CLKEN_GPIO4|CM_CLKEN_GPIO5|CM_CLKEN_GPIO6);
    SETREG32(&pPrcmPerCM->CM_ICLKEN_PER, CM_CLKEN_GPIO2|CM_CLKEN_GPIO3|CM_CLKEN_GPIO4|CM_CLKEN_GPIO5|CM_CLKEN_GPIO6);

    switch (GpioNumber >> 5)
    {
        case 0:
            pGpio = OALPAtoUA(OMAP_GPIO1_REGS_PA);
            break;
            
        case 1:
            pGpio = OALPAtoUA(OMAP_GPIO2_REGS_PA);
            GpioNumber -= 32;
            break;

        case 2:
            pGpio = OALPAtoUA(OMAP_GPIO3_REGS_PA);
            GpioNumber -= 64;
            break;

        case 3:
            pGpio = OALPAtoUA(OMAP_GPIO4_REGS_PA);
            GpioNumber -= 96;
            break;

        case 4:
            pGpio = OALPAtoUA(OMAP_GPIO5_REGS_PA);
            GpioNumber -= 128;
            break;

        case 5:
            pGpio = OALPAtoUA(OMAP_GPIO6_REGS_PA);
            GpioNumber -= 160;
            break;

        default:
            return;
    }

    // set output state of GPIO pin
    if (Value)
        SETREG32(&pGpio->DATAOUT, 1 << GpioNumber);
    else
        CLRREG32(&pGpio->DATAOUT, 1 << GpioNumber);

    // make GPIO pin an output
    CLRREG32(&pGpio->OE, 1 << GpioNumber);

    // Put clocks back
    OUTREG32(&pPrcmWkupCM->CM_FCLKEN_WKUP, fWkup);
    OUTREG32(&pPrcmWkupCM->CM_ICLKEN_WKUP, iWkup);
    OUTREG32(&pPrcmPerCM->CM_FCLKEN_PER, fPer);
    OUTREG32(&pPrcmPerCM->CM_ICLKEN_PER, iPer);
}


//------------------------------------------------------------------------------
//
//  Function:  OEMPlatformInit
//
//  This function provide platform initialization functions. It is called
//  from boot loader after OEMDebugInit is called.  Note that boot loader
//  BootloaderMain is called from  s/init.s code which is run after reset.
//
BOOL OEMPlatformInit()
{
    UINT8 version;
    UINT16 processorType;
	UINT32 dwIdReg;
    OMAP_GPTIMER_REGS *pTimerRegs = OALPAtoUA(OMAP_GPTIMER1_REGS_PA);
    UINT32 * pIDAddr = OALPAtoUA(OMAP_IDCODE_REGS_PA);
	UINT32 *pStatusControlAddr = OALPAtoUA(OMAP_STATUS_CONTROL_REGS_PA);
    HANDLE hTwl;
    OMAP_PRCM_GLOBAL_PRM_REGS *pPrmGlobal = OALPAtoUA(OMAP_PRCM_GLOBAL_PRM_REGS_PA);
	OMAP_SYSC_PADCONFS_REGS   *pConfig	  = OALPAtoUA(OMAP_SYSC_PADCONFS_REGS_PA);
	UINT32 dwSysBootCfg;
	UINT8  rsoc;
	BOOL ret;
	BOOT_CFG	boot_cfg = {0};
	BSP_ARGS *pArgs = OALPAtoUA(IMAGE_SHARE_ARGS_PA);

	// Enable serial ports transceiver
	CpuGpioOutput(162, 1);

	OALLog(MLDR_PATTERN_T
		L"Micronet Manufacturing tools for CCE317 based on AM/DM37XX, "
        L"Built %S at %S.\r\n", __DATE__, __TIME__
        );
    OALLog(MLDR_PATTERN_T
		L"Version %d.%02d.%03d.%02d\r\n",
		MLDR_VERSION_MAJOR, MLDR_VERSION_MINOR, MLDR_VERSION_INCREMENTAL, XLDR_VERSION_BUILD
        );

	// Michael, get GPTIMER1 working 1-Feb-2010
 	// Stop Timer
	OUTREG32(&pTimerRegs->TCLR, 0);

    // Soft reset GPTIMER1
    OUTREG32(&pTimerRegs->TIOCP, SYSCONFIG_SOFTRESET);
    // While until done
    while((INREG32(&pTimerRegs->TISTAT) & GPTIMER_TISTAT_RESETDONE) == 0);

    // Start timer
    OUTREG32(&pTimerRegs->TCLR, GPTIMER_TCLR_AR|GPTIMER_TCLR_ST);
    
    // configure i2c devices
    OALI2CInit(OMAP_DEVICE_I2C1, OALPAtoUA(OMAP_I2C1_REGS_PA));
    OALI2CInit(OMAP_DEVICE_I2C2, OALPAtoUA(OMAP_I2C2_REGS_PA));
    OALI2CInit(OMAP_DEVICE_I2C3, OALPAtoUA(OMAP_I2C3_REGS_PA));
    
    // Initialize ARGS structure
    if((pArgs->header.signature != OAL_ARGS_SIGNATURE) || (pArgs->header.oalVersion != OAL_ARGS_VERSION) ||
	   (pArgs->header.bspVersion != BSP_ARGS_VERSION))
	{
		memset(pArgs, 0, IMAGE_SHARE_ARGS_SIZE);
	}
	memset(&pArgs->hw_stat, 0, sizeof(hw_boot_status));

	// Initialize display
	pArgs->identLCD = 0;
	memset(&pArgs->main_board, (UINT8)-1, sizeof(card_ver));
	memset(&pArgs->display_board, (UINT8)-1, sizeof(card_ver));
	memset(&pArgs->wlan_board, (UINT8)-1, sizeof(card_ver));
	memset(&pArgs->canbus_board, (UINT8)-1, sizeof(card_ver));
	memset(&pArgs->gps_board, (UINT8)-1, sizeof(card_ver));
	memset(&pArgs->modem_board, (UINT8)-1, sizeof(card_ver));
	memset(&pArgs->extaudio_board, (UINT8)-1, sizeof(card_ver));

	// serial txter force on
	CpuGpioOutput(162, 1);

#ifndef SHIP_BUILD
	OALLog(L"The power source is %s\r\n", L"External");
#endif

	pArgs->main_board.card_addr	= -1;
	pArgs->main_board.ver		= 317;
	pArgs->main_board.rev = 0;
	// TODO: revision should be retrieved via A2D channel (currently unavailable)
	pArgs->main_board.config	= 'A' + (pArgs->main_board.rev)?pArgs->main_board.rev - 1:pArgs->main_board.rev;

#ifndef SHIP_BUILD
	OALLog(L"Main board NET860 rev %d\r\n", pArgs->main_board.rev);
#endif

	//CpuGpioOutput(98, 1);
	//OALStall(200000);
	CpuGpioOutput(98, 0);

	//automotive out high
	CpuGpioOutput(94, 0);
	// Enable periph 3V
	CpuGpioOutput(27, 0);

	// PWR_ON generator will off
	CpuGpioOutput(186, 1);


	// Detect PB
	pArgs->hw_stat.PoB = 1;

#ifndef SHIP_BUILD
	OALLog(L"Power Box is%sexist\r\n", (pArgs->hw_stat.PoB)?L" ":L"n't ");
#endif

	pArgs->wlan_board.card_addr	= -1;
	pArgs->wlan_board.ver		= pArgs->main_board.ver;
	pArgs->wlan_board.rev		= pArgs->main_board.rev;
	pArgs->wlan_board.config	= 'C';

	// Disable BT
	CpuGpioOutput(137, 0);
	// Disable Wifi
	CpuGpioOutput(138, 0);

	// Remove VIO 1.8 V
	CpuGpioOutput(95, 0);
	// Remove VBAT 3.6 V
	CpuGpioOutput(136, 0);

	OALStall(100000);

	// Supply VBAT 3.6 V to WiFi
	CpuGpioOutput(136, 1);
	CpuGpioOutput(95, 1);

	OALStall(100000);

	// init digital pads of wireless module
	OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_CLK,  (INPUT_ENABLE  | PULL_UP | MUX_MODE_0));	// MMC2_CLK
	OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_CMD,  (INPUT_ENABLE  | PULL_UP | MUX_MODE_0));	// MMC2_CMD
	OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_DAT0, (INPUT_ENABLE  | PULL_UP | MUX_MODE_0));	// MMC2_DAT0
	OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_DAT1, (INPUT_ENABLE  | PULL_UP | MUX_MODE_0));	// MMC2_DAT1
	OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_DAT2, (INPUT_ENABLE  | PULL_UP | MUX_MODE_0));	// MMC2_DAT2
	OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_DAT3, (INPUT_ENABLE  | PULL_UP | MUX_MODE_0));	// MMC2_DAT3

	OUTREG16(&pConfig->CONTROL_PADCONF_MCBSP3_CLKX, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_1));  // UART2_TX
	OUTREG16(&pConfig->CONTROL_PADCONF_MCBSP3_FSX,  (INPUT_ENABLE  | PULL_INACTIVE | MUX_MODE_1));  // UART2_RX
	OUTREG16(&pConfig->CONTROL_PADCONF_MCBSP3_DR,   (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_1));  // UART2_RTS
	OUTREG16(&pConfig->CONTROL_PADCONF_MCBSP3_DX,   (INPUT_ENABLE  | PULL_INACTIVE | MUX_MODE_1));  // UART2_CTS

	OALStall(200000);

	// pulse enable pins
	// Enable WiFi
	CpuGpioOutput(138, 1);
	// Enable BT
	CpuGpioOutput(137, 1);
	OALStall(200000);

	// Disable Wifi always, the pin used for power off
	CpuGpioOutput(138, 0);
	// Disable BT always, to reduce power consumption
	CpuGpioOutput(137, 0);

	#ifndef SHIP_BUILD
		OALLog(L"WLAN module %d.%d%c\r\n", pArgs->wlan_board.ver, pArgs->wlan_board.rev, (UINT8)pArgs->wlan_board.config);
	#endif

	pArgs->modem_board.card_addr	= -1;
	pArgs->modem_board.ver			= pArgs->main_board.ver;
	pArgs->modem_board.rev			= pArgs->main_board.rev;
	pArgs->modem_board.config		= 'A';
	// Disable modem
	CpuGpioOutput(41, 0);
	// Reset modem
	CpuGpioOutput(40, 0);
	// Off modem
	CpuGpioOutput(61, 0);

	pArgs->gps_board.card_addr	= -1;
	pArgs->gps_board.ver		= pArgs->main_board.ver;
	pArgs->gps_board.rev		= pArgs->main_board.rev;
	pArgs->gps_board.config		= 'A';

	// GPS disable
	CpuGpioOutput(144, 0);
	// reset GPS
	CpuGpioOutput(145, 0);
	// GPS disable boot
	CpuGpioOutput(146, 0);
	// GPS power disable
	CpuGpioOutput(64, 0);
#ifndef SHIP_BUILD
	OALLog(L"GPS module %d.%d%c\r\n", pArgs->gps_board.ver, pArgs->gps_board.rev, (UINT8)pArgs->gps_board.config);
#endif

	pArgs->canbus_board.card_addr	= -1;
	pArgs->canbus_board.ver			= pArgs->main_board.ver;
	pArgs->canbus_board.rev			= pArgs->main_board.rev;
	pArgs->canbus_board.config		= 'A';

	// CAN disable
	CpuGpioOutput(38, 0);
	// CAN power off
	CpuGpioOutput(43, 0);

	// Cap touch reset
	CpuGpioOutput(42, 0);

	pArgs->identLCD |= 1; // TODO: display identification via A2D
	#ifndef SHIP_BUILD
		OALLog(L"LCD panel %x: %s\r\n", pArgs->identLCD,
										(pArgs->identLCD == 1)?L"WINSTAR 7":
		                                (pArgs->identLCD == 2)?L"Dastek 4.3":L"unknown");
	#endif

	pArgs->display_board.card_addr	= -1;
	pArgs->display_board.ver		= pArgs->main_board.ver;
	pArgs->display_board.rev		= pArgs->identLCD;
	pArgs->display_board.config		= 'A' + pArgs->identLCD;

	#ifndef SHIP_BUILD
		OALLog(L"display_board.config = %c\r\n", pArgs->display_board.config);
	#endif

	// power on display buffer
	CpuGpioOutput(164, 1);
	// enable display buffer
	CpuGpioOutput(104, 0);

	#ifndef SHIP_BUILD
		OALLog(L"Display ON\r\n");
	#endif

	// Config USB PHY/HUB
	// HUB reset
	CpuGpioOutput(34, 0);
	// PHY off
	CpuGpioOutput(111, 1);
	// PHY reset
	CpuGpioOutput(65, 0);

	#ifndef SHIP_BUILD
		OALLog(L"HSHUB/PHY accert reset\r\n");
	#endif

	// Audio config
	// enable internal speaker
	CpuGpioOutput(175, 1);
	// Shut down external amplifier
	CpuGpioOutput(176, 0);
	// Shutdown external microphone
	CpuGpioOutput(161, 0);
	// disable I2C3
	CpuGpioOutput(177, 0);

	// TODO enable internal speaker amplifier via Triton
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	#ifndef SHIP_BUILD
		OALLog(L"External Audio config\r\n");
	#endif

	//
	//
    // Display processor and companion chip versions
    dwIdReg = INREG32(pIDAddr);

	g_CPU_Rev = Get_CPUVersion();
	processorType = CPU_ID(g_CPU_Rev);

	#if !defined (SHIP_BUILD)
		OALLog( MLDR_PATTERN_T L"TI OMAP/AM/DM_%X Version 0x%08x (%x)\r\n", processorType, g_CPU_Rev, dwIdReg);
    #endif

    // Note that T2 accesses must occur after I2C initialization
    hTwl = OALTritonOpen();
 
	// For the backup registers using information see \TI_EVM_3530\SRC\oal\oalrtc\backup_domain_using.txt
	OALTritonRead(hTwl, TWL_BACKUP_REG_G, &version);
	pArgs->hw_stat.overheat = (version & 1) ? 1 : 0;
	pArgs->ThermalShutdown = version & 3; //Bit 0 on means Companion Chip overheated. Bit 1 on means MPU overheated.
	OALTritonWrite(hTwl, TWL_BACKUP_REG_G, version & 0xFE);
	#ifndef SHIP_BUILD
        OALLog(MLDR_PATTERN_T L"Boot overheat reason is %d\r\n", pArgs->ThermalShutdown);
	#endif

	//second part of hw starton statuses
	OALTritonRead(hTwl, TWL_RTC_STATUS_REG, &version);
	//OALLog(L"TWL_RTC_STATUS_REG 0x%08x\r\n", version );

	pArgs->hw_stat.rtc_power_up = (version & TWL_RTC_STATUS_POWER_UP)	? 1 : 0;
	pArgs->hw_stat.rtc_alarm	= (version & TWL_RTC_STATUS_ALARM)		? 1 : 0;	
	pArgs->hw_stat.rtc_event	= (version & (	TWL_RTC_STATUS_ONE_D_EVENT | TWL_RTC_STATUS_ONE_H_EVENT |
												TWL_RTC_STATUS_ONE_M_EVENT | TWL_RTC_STATUS_ONE_S_EVENT))	? 1 : 0;	

	OALTritonRead(hTwl, TWL_RTC_INTERRUPTS_REG, &version);
	//OALLog(L"TWL_RTC_INTERRUPTS_REG 0x%08x\r\n", version );
	pArgs->hw_stat.rtc_it_timer = (version & TWL_RTC_INTERRUPTS_IT_TIMER) ? 1 : 0;
	pArgs->hw_stat.rtc_it_alarm = (version & TWL_RTC_INTERRUPTS_IT_ALARM) ? 1 : 0;

	OALTritonRead(hTwl, TWL_STS_HW_CONDITIONS, &version);
	//OALLog(L"TWL_STS_HW_CONDITIONS 0x%08x\r\n", version );
	pArgs->hw_stat.sts_pwon			= (version & 1)			? 1 : 0; //STS_PWON		
	pArgs->hw_stat.sts_warmreset	= (version & (1<<3))	? 1 : 0; //STS_WARMRESET

	OALTritonRead(hTwl, TWL_STS_BOOT, &version);
	//OALLog(L"TWL_STS_BOOT 0x%08x\r\n", version );
	pArgs->hw_stat.watchdog_reset	= (version & (1<<5))	? 1 : 0;//TWL_WATCHDOG_RESET

	OALTritonRead(hTwl, TWL_PWR_ISR1, &version);
	//OALLog(L"TWL_PWR_ISR1 0x%08x\r\n", version );
	pArgs->hw_stat.isr_pwron	= (version & STARTON_RTC)	? 1 : 0;//
	pArgs->hw_stat.isr_rtc_it	= (version & STARTON_PWON)	? 1 : 0;
/////////////////////////////////////////////////////////
    // Save reset type
    pArgs->hw_stat.prm_rstst = INREG32(&pPrmGlobal->PRM_RSTST);
#ifndef SHIP_BUILD
    OALLog(MLDR_PATTERN_T L"Boot reason %X\r\n", pArgs->hw_stat.bits);
    OALLog(MLDR_PATTERN_T L"Reset reason %X\r\n", pArgs->hw_stat.prm_rstst);
#endif

    OALTritonRead(hTwl, TWL_IDCODE_31_24, &version);
    
    OALTritonClose(hTwl);
    OALLog(L"TPS659XX Version 0x%02x (%s)\r\n", version,
            version == 0x00 ? L"ES1.0" : 
            version == 0x10 ? L"ES1.1" :
            version == 0x12 ? L"ES1.2" :L"Unknown" );
	OALLog(MLDR_PATTERN_T	L"NAND memory used.\r\n" );
    // Ensure bootloader blocks are marked as reserved
    BLReserveBootBlocks();

    // Read saved configration
	memset(&g_bootCfg, 0, sizeof(g_bootCfg));

	if ((ret = BLReadBootCfg(&g_bootCfg)) &&
        (g_bootCfg.signature == BOOT_CFG_SIGNATURE) &&
        (g_bootCfg.version == BOOT_CFG_VERSION) &&
		(g_bootCfg.osPartitionSize == IMAGE_WINCE_CODE_SIZE))
    {
		OALLog(L"INFO: Boot configuration found\r\n");
	}
    else
	{
		if(	!ret										||
			(g_bootCfg.signature != BOOT_CFG_SIGNATURE) ||
			(g_bootCfg.version != BOOT_CFG_VERSION))
		{
			OALLog(L"WARN: Boot config wasn't found, using defaults\r\n");
			memset(&g_bootCfg, 0, sizeof(g_bootCfg));
		}
		else//store mac 
		{
			memcpy(&boot_cfg, &g_bootCfg, sizeof(g_bootCfg));
			memset(&g_bootCfg, 0, sizeof(g_bootCfg));
			memcpy(g_bootCfg.RndisMacAddress, boot_cfg.RndisMacAddress, sizeof(g_bootCfg.RndisMacAddress));
			memcpy(g_bootCfg.EthMacAddress, boot_cfg.EthMacAddress, sizeof(g_bootCfg.EthMacAddress));
			memcpy(g_bootCfg.WiFiMacAddress, boot_cfg.WiFiMacAddress, sizeof(g_bootCfg.WiFiMacAddress));
			OALLog(L"WARN: Boot config wasn't found, using defaults and MACs\r\n");
		}
        g_bootCfg.signature = BOOT_CFG_SIGNATURE;
        g_bootCfg.version = BOOT_CFG_VERSION;

        // To make it easier to select USB or EBOOT from menus when booting from SD card,
		// preset the kitlFlags. This has no effect if booting from SD card.
	    g_bootCfg.kitlFlags = OAL_KITL_FLAGS_DHCP|OAL_KITL_FLAGS_ENABLED;
	    g_bootCfg.kitlFlags |= OAL_KITL_FLAGS_VMINI;
	    g_bootCfg.kitlFlags |= OAL_KITL_FLAGS_EXTNAME;

        // select default boot device based on boot select switch setting
        dwSysBootCfg = INREG32(pStatusControlAddr);
		OALLog(L"System boot config is %X\r\n", dwSysBootCfg);
	    // 1st boot device is MMC1 (SD Card Boot)
	    g_bootCfg.bootDevLoc.LogicalLoc = OMAP_MMCHS1_REGS_PA;
        g_bootCfg.deviceID = 0;
        g_bootCfg.osPartitionSize = IMAGE_WINCE_CODE_SIZE;
		wcscpy(g_bootCfg.filename, L"nk.bin");
   }
	
    // Initialize display
    //BLShowLogo();

	// Initialize flash partitions if needed
	BLConfigureFlashPartitions(FALSE);
	if(boot_cfg.signature == BOOT_CFG_SIGNATURE && boot_cfg.version == BOOT_CFG_VERSION)
	{
		boot_cfg.osPartitionSize = IMAGE_WINCE_CODE_SIZE;
		BLWriteBootCfg(&boot_cfg);
	}

    g_bootCfg.bootDevLoc.LogicalLoc = 0;
    g_bootCfg.osPartitionSize = 0;
	g_bootCfg.filename[0] = 0;

	PrcmDeviceEnableClocks(OMAP_DEVICE_MMC1, 1);
   // Done
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMPlatformDeinit
//

VOID OEMPlatformDeinit()
{
    OMAP_GPTIMER_REGS *pTimerRegs = OALPAtoUA(OMAP_GPTIMER1_REGS_PA);

    // Soft reset GPTIMER
    OUTREG32(&pTimerRegs->TIOCP, SYSCONFIG_SOFTRESET);
    // While until done
    while ((INREG32(&pTimerRegs->TISTAT) & GPTIMER_TISTAT_RESETDONE) == 0);
}


//------------------------------------------------------------------------------
//
//  Function:  OEMPreDownload
//
//  This function is called before downloading an image. There is place
//  where user can be asked about device setup.
//
ULONG OEMPreDownload()
{
    ULONG rc = BL_ERROR;
    BSP_ARGS *pArgs = OALPAtoUA(IMAGE_SHARE_ARGS_PA);
	BOOL bForceBootMenu;
	OMAP_PRCM_GLOBAL_PRM_REGS * pPrmGlobal = OALPAtoUA(OMAP_PRCM_GLOBAL_PRM_REGS_PA);
	ULONG dwTemp;
	UINT32			x = 30;

	static BOOL	fFirst = 1;
	if( fFirst )
	{
		OALLog(L"INFO: Predownload....\r\n");

		// We need to support multi bin notify
		g_pOEMMultiBINNotify = OEMMultiBinNotify;

	    
		// Save reset type
		dwTemp = INREG32(&pPrmGlobal->PRM_RSTST);
		if (dwTemp & (GLOBALWARM_RST | EXTERNALWARM_RST))
			pArgs->coldBoot = FALSE;
		else
			pArgs->coldBoot = TRUE;
		OALLog(L"Boot reason is %X\r\n", pArgs->coldBoot);
    
		BLShowLogo();

		fFirst = 0;
		SetProgressData(8,1);
	}
	else //not first
		ProgressNotify();
	
	if(!g_bootCfg.osPartitionSize)
	{
		OALLog(MLDR_PATTERN_T L"Prepare XLDR for update.\r\n");
		g_bootCfg.bootDevLoc.LogicalLoc = OMAP_MMCHS1_REGS_PA;
		g_bootCfg.bootDevLoc.IfcType	= g_bootDevices[0].ifcType;
		g_bootCfg.bootDevLoc.BusNumber	= 0;
		g_bootCfg.bootDevLoc.LogicalLoc = g_bootDevices[0].id;
		g_bootCfg.bootDevLoc.PhysicalLoc= (PVOID)g_bootDevices[0].resource;
		g_bootCfg.deviceID = 0;
		g_bootCfg.osPartitionSize = IMAGE_XLDR_CODE_SIZE;
		wcscpy(g_bootCfg.filename, L"xldrnand.bin");

		TextWriteW( x,		g_y, (WCHAR*)g_bootDevices[0].name, TX_LEFT, CL_TEXT );
		TextWriteW( x + 300, g_y, L"                  ", TX_LEFT, CL_TEXT );
	}
	else if(g_bootCfg.osPartitionSize == IMAGE_XLDR_CODE_SIZE)
	{
		g_y += 30;

		OALLog(MLDR_PATTERN_T L"Prepare EBOOT for update.\r\n");
		g_bootCfg.bootDevLoc.LogicalLoc = OMAP_MMCHS1_REGS_PA;
		g_bootCfg.bootDevLoc.IfcType	= g_bootDevices[1].ifcType;
		g_bootCfg.bootDevLoc.BusNumber	= 0;
		g_bootCfg.bootDevLoc.LogicalLoc = g_bootDevices[1].id;
		g_bootCfg.bootDevLoc.PhysicalLoc= (PVOID)g_bootDevices[1].resource;
		g_bootCfg.deviceID = 0;
		g_bootCfg.osPartitionSize = IMAGE_EBOOT_CODE_SIZE;
		wcscpy(g_bootCfg.filename, L"ebnand.bin");

		TextWriteW( x,		g_y, (WCHAR*)g_bootDevices[1].name, TX_LEFT, CL_TEXT );
		TextWriteW( x + 300, g_y, L"                  ", TX_LEFT, CL_TEXT );
	}else if(g_bootCfg.osPartitionSize == IMAGE_EBOOT_CODE_SIZE)
	{
		g_y += 30;

		OALLog(MLDR_PATTERN_T L"Prepare bmp for update.\r\n");
		g_bootCfg.bootDevLoc.LogicalLoc = OMAP_MMCHS1_REGS_PA;
		g_bootCfg.bootDevLoc.IfcType	= g_bootDevices[2].ifcType;
		g_bootCfg.bootDevLoc.BusNumber	= 0;
		g_bootCfg.bootDevLoc.LogicalLoc = g_bootDevices[2].id;
		g_bootCfg.bootDevLoc.PhysicalLoc= (PVOID)g_bootDevices[2].resource;
		g_bootCfg.deviceID = 0;
		g_bootCfg.osPartitionSize = IMAGE_BOOTLOADER_BITMAP_SIZE;
		wcscpy(g_bootCfg.filename, L"logo.bmp");
		TextWriteW( x,		g_y, (WCHAR*)g_bootDevices[2].name, TX_LEFT, CL_TEXT );
		TextWriteW( x + 300, g_y, L"                  ", TX_LEFT, CL_TEXT );
	}
	else if(g_bootCfg.osPartitionSize == IMAGE_BOOTLOADER_BITMAP_SIZE )
	{
		g_y += 30;

		OALLog(MLDR_PATTERN_T L"Prepare NK for update.\r\n");
		g_bootCfg.bootDevLoc.LogicalLoc = OMAP_MMCHS1_REGS_PA;
		g_bootCfg.bootDevLoc.IfcType	= g_bootDevices[3].ifcType;
		g_bootCfg.bootDevLoc.BusNumber	= 0;
		g_bootCfg.bootDevLoc.LogicalLoc = g_bootDevices[3].id;
		g_bootCfg.bootDevLoc.PhysicalLoc= (PVOID)g_bootDevices[3].resource;
		g_bootCfg.deviceID = 0;
		g_bootCfg.osPartitionSize = IMAGE_WINCE_CODE_SIZE;
		wcscpy(g_bootCfg.filename, L"nk.bin");

		TextWriteW( x,		g_y, (WCHAR*)g_bootDevices[3].name, TX_LEFT, CL_TEXT );
		TextWriteW( x + 300, g_y, L"                  ", TX_LEFT, CL_TEXT );
		FillProgress(0);
		SetProgressData(NUMBER_PROGRESS_RECTANGLES, TMP_SIZE/NUMBER_PROGRESS_RECTANGLES);
	}
	else
	{
		OALLog(L"Prepare NK for load\r\n");
		g_bootCfg.bootDevLoc.LogicalLoc = BSP_NAND_REGS_PA + 0x20;
		g_bootCfg.bootDevLoc.IfcType	= g_bootDevices[4].ifcType;
		g_bootCfg.bootDevLoc.BusNumber	= 0;
		g_bootCfg.bootDevLoc.LogicalLoc = g_bootDevices[4].id;
		g_bootCfg.bootDevLoc.PhysicalLoc= (PVOID)g_bootDevices[4].resource;
		g_bootCfg.deviceID = 0;
		g_bootCfg.osPartitionSize = IMAGE_WINCE_CODE_SIZE;
		wcscpy(g_bootCfg.filename, L"nk.bin");
	}
	if((g_bootCfg.osPartitionSize != IMAGE_WINCE_CODE_SIZE) || (g_bootCfg.bootDevLoc.LogicalLoc != (BSP_NAND_REGS_PA + 0x20)))
	{
		g_eboot.bootDeviceType = OALKitlDeviceType(&g_bootCfg.bootDevLoc, g_bootDevices);
		switch(g_eboot.bootDeviceType)
		{
			case BOOT_SDCARD_TYPE:
				rc = BLSDCardDownload(&g_bootCfg, g_bootDevices);
				break;
			case OAL_KITL_TYPE_FLASH:
				rc = BLFlashDownload(&g_bootCfg, g_bootDevices);
				break;
		}
		if(rc == BL_ERROR)
			TextWriteW( x + 300, g_y, L"Not Found", TX_LEFT, CL_TEXT );
		return rc;
	}

	// Don't force the boot menu, use default action unless user breaks
	// into menu
	bForceBootMenu = 1;
	FillProgress(1);	
	
retryBootMenu:
    // Call configuration menu
    BLMenu(bForceBootMenu);
	
    // Update ARGS structure if necessary
    if ((pArgs->header.signature != OAL_ARGS_SIGNATURE) ||
        (pArgs->header.oalVersion != OAL_ARGS_VERSION) ||
        (pArgs->header.bspVersion != BSP_ARGS_VERSION))
    {
//		VersionInfoExt Version = {0};

        pArgs->header.signature = OAL_ARGS_SIGNATURE;
        pArgs->header.oalVersion = OAL_ARGS_VERSION;
        pArgs->header.bspVersion = BSP_ARGS_VERSION;
        pArgs->kitl.flags = g_bootCfg.kitlFlags;
        pArgs->kitl.devLoc = g_bootCfg.kitlDevLoc;
        pArgs->kitl.ipAddress = g_bootCfg.ipAddress;
        pArgs->kitl.ipMask = g_bootCfg.ipMask;
        pArgs->kitl.ipRoute = g_bootCfg.ipRoute;
        pArgs->updateMode = FALSE;
        pArgs->deviceID = g_bootCfg.deviceID;
	}        

    // Image download depends on protocol
    g_eboot.bootDeviceType = OALKitlDeviceType(&g_bootCfg.bootDevLoc, g_bootDevices);

    switch(g_eboot.bootDeviceType)
    {
        case BOOT_SDCARD_TYPE:
            rc = BLSDCardDownload(&g_bootCfg, g_bootDevices);
            break;
        case OAL_KITL_TYPE_FLASH:
            rc = BLFlashDownload(&g_bootCfg, g_bootDevices);
            break;
        }
		
	if (rc == BL_ERROR)
	{
		// No automatic mode now, force the boot menu to appear
		bForceBootMenu = TRUE;
		goto retryBootMenu;	
	}	
    
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMLaunch
//
//  This function is the last one called by the boot framework and it is
//  responsible for to launching the image.
//
VOID OEMLaunch(ULONG start, ULONG size, ULONG launch, const ROMHDR *pRomHeader)
{
    BSP_ARGS *pArgs = OALPAtoCA(IMAGE_SHARE_ARGS_PA);

    OALMSG(OAL_FUNC, (
        L"+OEMLaunch(0x%08x, 0x%08x, 0x%08x, 0x%08x - %d/%d)\r\n", start, size,
        launch, pRomHeader, g_eboot.bootDeviceType, g_eboot.type
        ));

    // Depending on protocol there can be some action required
    switch (g_eboot.bootDeviceType)
        {
        case BOOT_SDCARD_TYPE:
            BLSDCardConfig(pArgs);
            switch (g_eboot.type)
                {
                case DOWNLOAD_TYPE_RAM:
                    launch = (UINT32)OEMMapMemAddr(start, launch);
                    break;
                default:
                    OALMSG(OAL_ERROR, (L"ERROR: OEMLaunch: Unknown download type, spin forever\r\n"));
                    while (TRUE);
                    break;
                }
            break;
        default:        
            launch = g_eboot.launchAddress;
            break;
        }

    // Check if we get launch address
    if (launch == (UINT32)INVALID_HANDLE_VALUE)
        {
        OALMSG(OAL_ERROR, (L"ERROR: OEMLaunch: "
            L"Unknown image launch address, spin forever\r\n"
            ));
        while (TRUE);
        }        

    // Print message, flush caches and jump to image
    OALLog(
        L"Launch Windows CE image by jumping to 0x%08x...\r\n\r\n", launch
        );

    OEMPlatformDeinit();
    OEMDebugDeinit();
    JumpTo(launch);

    return;
}

//------------------------------------------------------------------------------
//
//  Function:   OEMMultiBinNotify
//
VOID OEMMultiBinNotify(MultiBINInfo *pInfo)
{
    BOOL rc = FALSE;
    UINT32 base = IMAGE_WINCE_CODES_PA;
    UINT32 start, length;
    UINT32 ix;


    OALMSGS(1, (L"+OEMMultiBinNotify(0x%08x -> %d)\r\n", pInfo, pInfo->dwNumRegions));
    OALMSG(1, (L"Download file information:\r\n"));
    OALMSG(1, (L"-----------------------------------------------------------\r\n"));

    // Copy information to EBOOT structure and set also save address
    g_eboot.numRegions = pInfo->dwNumRegions;
    for(ix = 0; ix < pInfo->dwNumRegions; ix++)
	{
        g_eboot.region[ix].start  = pInfo->Region[ix].dwRegionStart;
        g_eboot.region[ix].length = pInfo->Region[ix].dwRegionLength;
        g_eboot.region[ix].base   = base;
        base += g_eboot.region[ix].length;
        OALMSG(1,(L"[%d]: Address=0x%08x  Length=0x%08x  Save=0x%08x\r\n",
				 ix, g_eboot.region[ix].start, g_eboot.region[ix].length,g_eboot.region[ix].base));
	}
    OALMSG(1, (L"-----------------------------------------------------------\r\n"));

    // Determine type of image downloaded
    if(g_eboot.numRegions > 1)
	{
        OALMSG(1, (L"ERROR: MultiXIP image is not supported\r\n"));
        goto cleanUp;
	}

    base   = g_eboot.region[0].base;
    start  = g_eboot.region[0].start;
    length = g_eboot.region[0].length; 
    
    if(start == IMAGE_XLDR_CODE_PA)
	{
        g_eboot.type = DOWNLOAD_TYPE_XLDR;
        memset(OALPAtoCA(base), 0xFF, length);
	} 
    else if(start == IMAGE_EBOOT_CODE_PA)
	{
        g_eboot.type = DOWNLOAD_TYPE_EBOOT;
        memset(OALPAtoCA(base), 0xFF, length);
	}
    else if(start == IMAGE_WINCE_CODES_VA)
	{
        g_eboot.type = DOWNLOAD_TYPE_FLASHRAM;
        //memset(OALPAtoCA(base), 0xFF, length);
	} 
    else
	{
        g_eboot.type = DOWNLOAD_TYPE_RAM;
	}

    OALMSG(1, (L"Download file type: %d\r\n", g_eboot.type));
	
    rc = TRUE;

cleanUp:
    if(!rc)
	{
        OALMSG(OAL_ERROR, (L"Spin for ever...\r\n"));
		while (TRUE);
	}

    OALMSGS(1, (L"-OEMMultiBinNotify\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  OEMMapMemAddr
//
//  This function maps image relative address to memory address. It is used
//  by boot loader to verify some parts of downloaded image.
//
//  EBOOT mapping depends on download type. Download type is
//  set in OMEMultiBinNotify.
//
UINT8 *OEMMapMemAddr(DWORD image, DWORD address)
{
    UINT8 *pAddress = NULL;

    OALMSG(0, (L"+OEMMapMemAddr(0x%08x, 0x%08x)\r\n", image, address));

    switch (g_eboot.type) {

    case DOWNLOAD_TYPE_XLDR:
    case DOWNLOAD_TYPE_EBOOT:
        //  These downloads are given with physical RAM execution addresses
        //  Map to scratch RAM prior to flashing
        pAddress = (UINT8*)(g_eboot.region[0].base + address - image);
        break;

    case DOWNLOAD_TYPE_RAM:
    case DOWNLOAD_TYPE_FLASHRAM:
        //  RAM based NK.BIN files are given in virtual memory addresses
        //  Compute PA for given address
        pAddress = (UINT8*)BLVAtoPA(address);
        break;

//    case DOWNLOAD_TYPE_FLASHRAM:
//        pAddress = (UINT8*)address;
//        break;
        
    default:
        OALMSG(OAL_ERROR, (L"ERROR: OEMMapMemAddr: "L"Invalid download type %d\r\n", g_eboot.type));

    }


    OALMSGS(0, (L"-OEMMapMemAddr(pAddress = 0x%08x)\r\n", pAddress));
    return pAddress;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMIsFlashAddr
//
//  This function determines whether the address provided lies in a platform's
//  flash or RAM address range.
//
//  EBOOT decision depends on download type. Download type is
//  set in OMEMultiBinNotify.
//
BOOL OEMIsFlashAddr(ULONG address)
{
    BOOL rc;

    OALMSG(1, (L"+OEMIsFlashAddr(0x%08x)\r\n", address));

    // Depending on download type
    switch (g_eboot.type)
	{
        case DOWNLOAD_TYPE_XLDR:
        case DOWNLOAD_TYPE_EBOOT:
        case DOWNLOAD_TYPE_FLASHRAM:
            rc = TRUE;
            break;
        default:
            rc = FALSE;
            break;
    }

    OALMSG(1, (L"-OEMIsFlashAddr(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:   OEMReadData
//
//  This function is called to read data from the transport during
//  the download process.
//
BOOL OEMReadData(ULONG size, UCHAR *pData)
{
	BOOL rc = FALSE;
    OALMSG(0, (L"OEMReadData(0x%08x, %x)\r\n", pData, size));
    switch(g_eboot.bootDeviceType)
	{
		case BOOT_SDCARD_TYPE:
			rc = BLSDCardReadData(size, pData);
            break;
	}

	return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMShowProgress
//
//  This function is called during the download process to visualise
//  download progress.
//
VOID OEMShowProgress(ULONG packetNumber)
{
    OALMSG(0, (L"%d downloaded\r\n", packetNumber));
	
	ShowProgressLoop();
}

//------------------------------------------------------------------------------
//
//  Function:  OALGetTickCount
//
UINT32 OALGetTickCount()
{
    OMAP_GPTIMER_REGS *pGPTimerRegs = OALPAtoUA(OMAP_GPTIMER1_REGS_PA);
    return INREG32(&pGPTimerRegs->TCRR) >> 5;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMEthGetSecs
//
//  This function returns relative time in seconds.
//
DWORD OEMEthGetSecs()
{
    return OALGetTickCount()/1000;
}

//------------------------------------------------------------------------------
BOOL OEMShutDown()
{
	void* hTwl;


	hTwl = OALTritonOpen();
    if (hTwl == NULL) return FALSE;
    
    //  turn on DEVOFF bit in Softwarwe events register
   
//	OALTritonWrite(hTwl, TWL_P1_SW_EVENTS, 0x19);
	// Temporary as WAIT-ON to ACTIVE transition problem
	OALTritonWrite(hTwl, TWL_WATCHDOG_CFG, 0x01);
	OALTritonClose(hTwl);

	return TRUE;

}

