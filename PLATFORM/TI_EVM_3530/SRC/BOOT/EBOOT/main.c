// Copyright (c) 2007, 2013 2008 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
// Igor Lantsman
// Vitaly Ilyasov
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
#include <kitl_cfg.h>
#include <boot_cfg.h>
#include <bsp.h>
#include <i2c.h>
#include <oal_i2c.h>
#include <omap35xx_prcm.h>
#include <constants.h>

#include <bsp_version.h>
#include <ldr.h>
#include <swupdate.h>
#include <bsp_logo.h>


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
//
EBOOT_CONTEXT g_eboot;

//------------------------------------------------------------------------------
// External Functions

extern VOID JumpTo(UINT32 address);
extern VOID OEMDebugDeinit();

extern BOOL SetDownloadedFlag();
extern BOOL GetDownloadedFlag();
extern VOID RecheckRndisMac(UINT8 RndisMacAddress[]);

//------------------------------------------------------------------------------
//  Local Functions
//

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
//  Function:  UpdateVoltageLevels
//
//  update voltage levels
//
void UpdateVoltageLevels(UINT32 opm)
{
	UINT32 val;
	OMAP_PRCM_GLOBAL_PRM_REGS* pPrcmGblPRM = OALPAtoUA(OMAP_PRCM_GLOBAL_PRM_REGS_PA);

	//---------------------------------
	// setup voltage processors
	//

	// setup i2c for smps communication
	OUTREG32(&pPrcmGblPRM->PRM_VC_SMPS_SA, BSP_VC_SMPS_SA_INIT);
	OUTREG32(&pPrcmGblPRM->PRM_VC_SMPS_VOL_RA, BSP_VC_SMPS_VOL_RA_INIT);
	OUTREG32(&pPrcmGblPRM->PRM_VC_SMPS_CMD_RA, BSP_VC_SMPS_CMD_RA_INIT);
	OUTREG32(&pPrcmGblPRM->PRM_VC_CH_CONF, BSP_VC_CH_CONF_INIT);  
	OUTREG32(&pPrcmGblPRM->PRM_VC_I2C_CFG, BSP_PRM_VC_I2C_CFG_INIT);

	// set intial voltage levels
	if(4 == opm)
		val = BSP_PRM_VC_CMD_VAL_0_INIT4;
	else if(3 == opm)
		val = BSP_PRM_VC_CMD_VAL_0_INIT3;
	else
		val = BSP_PRM_VC_CMD_VAL_0_INIT2;
	OUTREG32(&pPrcmGblPRM->PRM_VC_CMD_VAL_0, val);
	OUTREG32(&pPrcmGblPRM->PRM_VC_CMD_VAL_1, BSP_PRM_VC_CMD_VAL_1_INIT);

	// set PowerIC error offset, gains, and initial voltage
	if(4 == opm)
		val = BSP_PRM_VP1_CONFIG_INIT_4;
	else if(3 == opm)
		val = BSP_PRM_VP1_CONFIG_INIT_3;
	else
		val = BSP_PRM_VP1_CONFIG_INIT_2;
	OUTREG32(&pPrcmGblPRM->PRM_VP1_CONFIG, val);
	OUTREG32(&pPrcmGblPRM->PRM_VP2_CONFIG, BSP_PRM_VP2_CONFIG_INIT);

	// set PowerIC slew range
	OUTREG32(&pPrcmGblPRM->PRM_VP1_VSTEPMIN, BSP_PRM_VP1_VSTEPMIN_INIT);
	OUTREG32(&pPrcmGblPRM->PRM_VP1_VSTEPMAX, BSP_PRM_VP1_VSTEPMAX_INIT);
	OUTREG32(&pPrcmGblPRM->PRM_VP2_VSTEPMIN, BSP_PRM_VP2_VSTEPMIN_INIT);
	OUTREG32(&pPrcmGblPRM->PRM_VP2_VSTEPMAX, BSP_PRM_VP2_VSTEPMAX_INIT);

	// set PowerIC voltage limits and timeout
	OUTREG32(&pPrcmGblPRM->PRM_VP1_VLIMITTO, BSP_PRM_VP1_VLIMITTO_INIT);
	OUTREG32(&pPrcmGblPRM->PRM_VP2_VLIMITTO, BSP_PRM_VP2_VLIMITTO_INIT);

	// enable voltage processors
	SETREG32(&pPrcmGblPRM->PRM_VP1_CONFIG, SMPS_VPENABLE);
	SETREG32(&pPrcmGblPRM->PRM_VP2_CONFIG, SMPS_VPENABLE);

	// enable timeout
	SETREG32(&pPrcmGblPRM->PRM_VP1_CONFIG, SMPS_TIMEOUTEN);
	SETREG32(&pPrcmGblPRM->PRM_VP2_CONFIG, SMPS_TIMEOUTEN);    

	// flush commands to smps
	SETREG32(&pPrcmGblPRM->PRM_VP1_CONFIG, SMPS_FORCEUPDATE | SMPS_INITVDD);
	SETREG32(&pPrcmGblPRM->PRM_VP2_CONFIG, SMPS_FORCEUPDATE | SMPS_INITVDD);

	// allow voltage to settle
	OALStall(10000);

	// disable voltage processors
	CLRREG32(&pPrcmGblPRM->PRM_VP1_CONFIG, SMPS_VPENABLE);
	CLRREG32(&pPrcmGblPRM->PRM_VP2_CONFIG, SMPS_VPENABLE);
}

//------------------------------------------------------------------------------
//
//  Function:  ClockSetup
//
//  Initializes clocks and power.  Stack based initialization only - no 
//  global variables allowed.
//
void ClockSetup(UINT32 opm)
{
	unsigned int val;
	OMAP_PRCM_MPU_CM_REGS* pPrcmMpuCM = OALPAtoUA(OMAP_PRCM_MPU_CM_REGS_PA);

	// put mpu dpll1 in bypass
	val = INREG32(&pPrcmMpuCM->CM_CLKEN_PLL_MPU);
	val &= ~DPLL_MODE_MASK;
	val |= DPLL_MODE_LOWPOWER_BYPASS;
	OUTREG32(&pPrcmMpuCM->CM_CLKEN_PLL_MPU, val);
	while ((INREG32(&pPrcmMpuCM->CM_IDLEST_PLL_MPU) & DPLL_STATUS_MASK) != DPLL_STATUS_BYPASSED);

	// setup DPLL1 divider
	OUTREG32(&pPrcmMpuCM->CM_CLKSEL2_PLL_MPU, BSP_CM_CLKSEL2_PLL_MPU);

	// configure m:n clock ratios as well as frequency selection for mpu dpll
	if(4 == opm)
		val = BSP_CM_CLKSEL1_PLL_MPU_1000;
	else if(3 == opm)
		val = BSP_CM_CLKSEL1_PLL_MPU_800;
	else
		val = BSP_CM_CLKSEL1_PLL_MPU_600;
	OUTREG32(&pPrcmMpuCM->CM_CLKSEL1_PLL_MPU, val);

	// lock dpll1 with correct frequency selection
	OUTREG32(&pPrcmMpuCM->CM_CLKEN_PLL_MPU, BSP_CM_CLKEN_PLL_MPU);
	while ((INREG32(&pPrcmMpuCM->CM_IDLEST_PLL_MPU) & DPLL_STATUS_MASK) != DPLL_STATUS_LOCKED);
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
	UINT32 dwIdReg;
    OMAP_GPTIMER_REGS *pTimerRegs = OALPAtoUA(OMAP_GPTIMER1_REGS_PA);
    UINT32 *pIDAddr = OALPAtoUA(OMAP_IDCODE_REGS_PA);
    OMAP_SYSC_GENERAL_REGS * pSyscGeneralRegs = OALPAtoUA(OMAP_SYSC_GENERAL_REGS_PA);
    HANDLE hTwl;
    OMAP_PRCM_GLOBAL_PRM_REGS *pPrmGlobal = OALPAtoUA(OMAP_PRCM_GLOBAL_PRM_REGS_PA);
	OMAP_SYSC_PADCONFS_REGS   *pConfig	  = OALPAtoUA(OMAP_SYSC_PADCONFS_REGS_PA);
	BSP_ARGS *pArgs = OALPAtoUA(IMAGE_SHARE_ARGS_PA);

	// Enable serial ports transceiver
	CpuGpioOutput(162, 1);

#ifndef SHIP_BUILD
    OALLog(
        L"\r\nCE317 Windows CE EBOOT for DM/AM3730, "
        L"Built %S at %S\r\n", __DATE__, __TIME__
        );
    OALLog(
        L"EBOOT Version %d.%d.%d.%d, BSP %d.%02d.%03d.%02d\r\n", 
        EBOOT_VERSION_MAJOR, EBOOT_VERSION_MINOR, EBOOT_VERSION_INCREMENTAL, EBOOT_VERSION_BUILD,
        BSP_VERSION_MAJOR, BSP_VERSION_MINOR, BSP_VERSION_INCREMENTAL, BSP_VERSION_BUILD
        );
#endif
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

#ifndef SHIP_BUILD
	//OALLog( L"Pulse GPIO 98 and release DEVICE_OFF\r\n" );
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

	#if !defined (SHIP_BUILD)
        OALLog(L"\r\nTI DM/AM37XX Rev 0x%08x\r\n", dwIdReg);
    #endif

    // Note that T2 accesses must occur after I2C initialization
    hTwl = OALTritonOpen();
 
	// For the backup registers using information see \TI_EVM_3530\SRC\oal\oalrtc\backup_domain_using.txt
	OALTritonRead(hTwl, TWL_BACKUP_REG_G, &version);
	pArgs->hw_stat.overheat = (version & 1) ? 1 : 0;
	pArgs->ThermalShutdown = version & 3; //Bit 0 on means Companion Chip overheated. Bit 1 on means MPU overheated.
	OALTritonWrite(hTwl, TWL_BACKUP_REG_G, version & 0xFE);
	#ifndef SHIP_BUILD
        OALLog(L"Boot overheat reason is %d\r\n", pArgs->ThermalShutdown);
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
    OALLog(L"Boot reason %X\r\n", pArgs->hw_stat.bits);
    OALLog(L"Reset reason %X\r\n", pArgs->hw_stat.prm_rstst);
#endif

    OALTritonRead(hTwl, TWL_IDCODE_31_24, &version);
    
    OALTritonClose(hTwl);
#ifndef SHIP_BUILD
    OALLog(L"TPS659XX Version 0x%02x (%s)\r\n", version,
            version == 0x00 ? L"ES1.0" : 
            version == 0x10 ? L"ES1.1" :
            version == 0x12 ? L"ES1.2" :L"Unknown" );
#endif    
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
    while((INREG32(&pTimerRegs->TISTAT) & GPTIMER_TISTAT_RESETDONE) == 0);
}

//------------------------------------------------------------------------------
//
//  Function:  OEMPreDownload
//
//  This function is called before downloading an image. There is place
//  where user can be asked about device setup.
//

#define RTC_BASE_YEAR_MIN       2000
#define BCD2BIN(b)              (((b) >> 4)*10 + ((b)&0xF))
#define BIN2BCD(b)              ((((UINT8)(b)/10) << 4)|((UINT8)(b)%10))

UINT64 BCDTime2Seconds(UCHAR *time)
{
	UCHAR DaysPerMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	UINT64 seconds = 0;
	UINT16 year = BCD2BIN(time[5]) + RTC_BASE_YEAR_MIN;
	UCHAR month = BCD2BIN(time[4]);
	UINT16 days = 0, i;

	if (year % 4 == 0 && !(year % 100 == 0 && year % 400 != 0))
		DaysPerMonth[1] = 29;

	for (i = 0; i < month - 1; i++)
		days += DaysPerMonth[i];

	days += BCD2BIN(time[3]) - 1;

	for (i = RTC_BASE_YEAR_MIN; i < year; i++)
	{
		if (i % 4 == 0 && !(i % 100 == 0 && i % 400 != 0))
			seconds += 31622400; // 60 * 60 * 24 * 366;
		else
			seconds += 31536000; // 60 * 60 * 24 * 365;
	}

	return seconds + BCD2BIN(time[0]) + (BCD2BIN(time[1]) * 60) + (BCD2BIN(time[2]) * 3600) + (days * 86400);
}

void Seconds2BCDTime(UINT64 seconds, UCHAR *time)
{
	UCHAR DaysPerMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	UINT16 i;

	for (i = RTC_BASE_YEAR_MIN; ; i++)
	{
		if (i % 4 == 0 && !(i % 100 == 0 && i % 400 != 0))
		{
			if (seconds < 31622400)
				break;
			
			seconds -= 31622400; // 60 * 60 * 24 * 366;
		}
		else
		{
			if (seconds < 31536000)
				break;

			seconds -= 31536000; // 60 * 60 * 24 * 365;
		}
	}

	time[5] = BIN2BCD(i - RTC_BASE_YEAR_MIN);
	if (i % 4 == 0 && !(i % 100 == 0 && i % 400 != 0))
		DaysPerMonth[1] = 29;
	else
		DaysPerMonth[1] = 28;

	for (i = 0; i < 12; i++)
	{
		if (seconds < (DaysPerMonth[i] * 86400))
			break;

		seconds -= DaysPerMonth[i] * 86400;
	}

	time[4] = BIN2BCD(i + 1);
	time[3] = BIN2BCD(seconds / 86400) + 1;
	seconds %= 86400; 
	time[2] = BIN2BCD(seconds / 3600);
	seconds %= 3600; 
	time[1] = BIN2BCD(seconds / 60);
	time[0] = BIN2BCD(seconds % 60);
}

ULONG OEMPreDownload()
{
    ULONG rc = BL_ERROR;
    BSP_ARGS *pArgs = OALPAtoUA(IMAGE_SHARE_ARGS_PA);
    BOOL bForceBootMenu;
    OMAP_PRCM_GLOBAL_PRM_REGS * pPrmGlobal = OALPAtoUA(OMAP_PRCM_GLOBAL_PRM_REGS_PA);
    OMAP_SYSC_GENERAL_REGS * pSyscGeneralRegs = OALPAtoUA(OMAP_SYSC_GENERAL_REGS_PA);
	UINT32 *pStatusControlAddr = OALPAtoUA(OMAP_STATUS_CONTROL_REGS_PA);
    UINT32 dwSysBootCfg, opm, AtoDRead, ChipId;

	VersionInfoExt Version = {0};
	DWORD ticks;
	BYTE  bModeBits;
	HANDLE hTwl;
	UINT8 getTime = TWL_RTC_CTRL_GET_TIME | TWL_RTC_CTRL_RUN;
	UCHAR time[6], alarm[6], backup[6];
	UINT64 seconds;
	BOOT_CFG	boot_cfg = {0};

#ifndef SHIP_BUILD
    OALLog(L"INFO: Predownload....\r\n");
#endif

	//First check if TSHUT is ON. If ON Device temperatuer is above 160°C and must be turned off
	if(CpuGpioInput(127))
	{
		hTwl = OALTritonOpen();
		OALTritonWrite(hTwl, TWL_WATCHDOG_CFG, 0x01);
		OALTritonClose(hTwl);
	}

    // We need to support multi bin notify
    g_pOEMMultiBINNotify = OEMMultiBinNotify;

    // Ensure bootloader blocks are marked as reserved
    BLReserveBootBlocks();

    // Read saved configration
	memset(&g_bootCfg, 0, sizeof(g_bootCfg));
    if(BLReadBootCfg(&g_bootCfg))
	{
		if((g_bootCfg.signature != BOOT_CFG_SIGNATURE) || (g_bootCfg.version != BOOT_CFG_VERSION))
		{
#ifndef SHIP_BUILD
			OALLog(L"WARN: Boot config wasn't found, using defaults\r\n");
#endif
			memset(&g_bootCfg, 0, sizeof(g_bootCfg));
			g_bootCfg.EthMacAddress[0] = 0xFFFF;
			g_bootCfg.EthMacAddress[1] = 0xFFFF;
			g_bootCfg.EthMacAddress[2] = 0xFFFF;
			
			g_bootCfg.WiFiMacAddress[0] = 0xFFFF;
			g_bootCfg.WiFiMacAddress[1] = 0xFFFF;
			g_bootCfg.WiFiMacAddress[2] = 0xFFFF;

			g_bootCfg.CdsParams[0] = 0x60; //MJT default is 105°C
			g_bootCfg.CdsParams[1] = 0x5a; //IT  default is 95°C
			g_bootCfg.CdsParams[2] = 0x56; //LTT default is 90°C
			g_bootCfg.CdsParams[3] = 0xff; //Feature on
			g_bootCfg.CdsParams[4] = 1000; //Default Max CDS frequency in MHz
			g_bootCfg.CdsParams[5] = 4;    //Default OPM is 1000MHz

			g_bootCfg.SwUpdateFlags	= 0;
			g_bootCfg.oalFlags = 0;

			// To make it easier to select USB or EBOOT from menus when booting from SD card,
			// preset the kitlFlags. This has no effect if booting from SD card.
			g_bootCfg.kitlDevLoc.LogicalLoc = OMAP_USBHS_REGS_PA;
			g_bootCfg.kitlFlags = OAL_KITL_FLAGS_DHCP|OAL_KITL_FLAGS_ENABLED;
			//g_bootCfg.kitlFlags |= OAL_KITL_FLAGS_VMINI;
			g_bootCfg.kitlFlags |= OAL_KITL_FLAGS_EXTNAME;

		}
		else if(g_bootCfg.osPartitionSize != IMAGE_WINCE_CODE_SIZE)
		{
#ifndef SHIP_BUILD
			OALLog(L"INFO: OS Partition is changed\r\n");
#endif
			memcpy(&boot_cfg, &g_bootCfg, sizeof(g_bootCfg));
			memset(&g_bootCfg, 0, sizeof(g_bootCfg));
			memcpy(g_bootCfg.RndisMacAddress, boot_cfg.RndisMacAddress, sizeof(g_bootCfg.RndisMacAddress));
			memcpy(g_bootCfg.EthMacAddress, boot_cfg.EthMacAddress, sizeof(g_bootCfg.EthMacAddress));
			memcpy(g_bootCfg.WiFiMacAddress, boot_cfg.WiFiMacAddress, sizeof(g_bootCfg.WiFiMacAddress));
			memcpy(g_bootCfg.CdsParams, boot_cfg.CdsParams, sizeof(g_bootCfg.CdsParams));
		}
		else
		{
#ifndef SHIP_BUILD
        OALLog(L"INFO: Boot configuration found\r\n");
#endif
		}
        // select default boot device based on boot select switch setting
        dwSysBootCfg = INREG32(pStatusControlAddr);
#ifndef SHIP_BUILD
		OALLog(L"Boot config %X\r\n", dwSysBootCfg);
#endif
		switch(dwSysBootCfg & 0x3f)
		{
			case 0x2F:
				// 1st boot device is MMC1 (SD Card Boot)
				g_bootCfg.bootDevLoc.LogicalLoc = OMAP_MMCHS1_REGS_PA;
				break;			
			case 0x0F:
				// 1st boot device is NAND
				g_bootCfg.bootDevLoc.LogicalLoc = BSP_NAND_REGS_PA + 0x20;
				break;
			default:
				// 1st boot device is USB
				g_bootCfg.bootDevLoc.LogicalLoc = OMAP_USBHS_REGS_PA;
				g_bootCfg.kitlDevLoc.LogicalLoc = OMAP_USBHS_REGS_PA;
				break;
		}

		g_bootCfg.deviceID = 0;
		g_bootCfg.signature = BOOT_CFG_SIGNATURE;
		g_bootCfg.version = BOOT_CFG_VERSION;
		g_bootCfg.osPartitionSize = IMAGE_WINCE_CODE_SIZE;
		wcscpy(g_bootCfg.filename, L"nk.bin");
	}
	else 
	{
		OALLog(L"Critical error: NAND controller problem\r\n");
		while(1);
	}
	g_bootCfg.launch_manutool = 0;

#if (BSP_CDS == 1)
	//CDS - Configure MPU clock speed
	opm = g_bootCfg.CdsParams[5];

	//Start BGAPTS continuos measurement and measure current Junction Temperature from BGAPTS
	OUTREG32(&pSyscGeneralRegs->CONTROL_TEMP_SENSOR, 0);
	OUTREG32(&pSyscGeneralRegs->CONTROL_TEMP_SENSOR, 0x600);
	// wait for at least 20000us
	OALStall(20000);
	AtoDRead = INREG32(&pSyscGeneralRegs->CONTROL_TEMP_SENSOR) & 0xFF; //Only 8 LSB are read value

	//Read if chip supports 1GHz
	ChipId = *(UINT32*)OALPAtoUA(OMAP_CHIP_ID) & 0xF00;

	if (g_bootCfg.CdsParams[3] & 1) //bit 0 holds CDS feature enable state
	{
		//CDS is enabled in BOOT parameters
		if (ChipId == 0xE00) { //According to TI, CUSD100 were not tested to 105°C at any CPU speed
			if (AtoDRead < 87)
				opm = 4;//Below MJT@1GHz = 90°C, set MPU speed to 1000MHz
			else
				opm = 2;//Above MJT - Set MPU clock to 600MHz
		}
		else {//Chip type CUSA - 600/800MHz
			if (AtoDRead < 96)
				opm = 3;//Below MJT@800MHz = 105°C, set MPU speed to 800MHz
			else
				opm = 2;//Above MJT - Set MPU clock to 600MHz
		}
	}
	g_bootCfg.CdsParams[5] = opm;//Set Operational mode for CDS driver
	UpdateVoltageLevels(opm);
	ClockSetup(opm);
#ifndef SHIP_BUILD
	OALLog(L"CDS Set opm %d Chip ID %x\r\n", opm, ChipId);
#endif
#endif //BSP_CDS

    // Initialize flash partitions if needed
    BLConfigureFlashPartitions(0);

    BLShowLogo();
	
	if(pArgs->SwUpdateFlags & SW_START_UPDATE)
		g_bootCfg.SwUpdateFlags = ( pArgs->SwUpdateFlags & 0x0000FFFF ); 
	else if(0 == pArgs->SwUpdateFlags && g_bootCfg.SwUpdateFlags)
		pArgs->SwUpdateFlags = g_bootCfg.SwUpdateFlags;

	// Don't mark registry to clean, dont'format storage
	pArgs->bHiveCleanFlag  = 0;
	pArgs->bFormatPartFlag = 0;
    pArgs->coldBoot = ((pArgs->hw_stat.prm_rstst & (GLOBALWARM_RST | EXTERNALWARM_RST))==0);

	memcpy(pArgs->RndisMacAddress, g_bootCfg.RndisMacAddress, sizeof(pArgs->RndisMacAddress));
    RecheckRndisMac(pArgs->RndisMacAddress);
	
    
    // Don't force the boot menu, use default action unless user breaks
    // into menu
    bForceBootMenu = FALSE;
    
retryBootMenu:
    // Call configuration menu
    BLMenu(bForceBootMenu);
    
    // Update ARGS structure if necessary
    if((pArgs->header.signature != OAL_ARGS_SIGNATURE) || (pArgs->header.oalVersion != OAL_ARGS_VERSION) || (pArgs->header.bspVersion != BSP_ARGS_VERSION))
    {
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
        pArgs->oalFlags = g_bootCfg.oalFlags;
		BLReadManufactureCfg(pArgs->ManufBlock);
		
		memcpy(pArgs->RndisMacAddress, g_bootCfg.RndisMacAddress, sizeof(pArgs->RndisMacAddress));
    	RecheckRndisMac(pArgs->RndisMacAddress);
    }        
	if(0 == getEBOOTVersion(&Version))
	{		
		pArgs->EbootVersion.dwVersionMajor			= Version.Version.majorVersion;
		pArgs->EbootVersion.dwVersionMinor			= Version.Version.minorVersion;
		pArgs->EbootVersion.dwVersionIncremental	= Version.Version.incrementalVersion;

		pArgs->EbootVersionExt.Version.dwVersionMajor		= Version.Version.majorVersion;
		pArgs->EbootVersionExt.Version.dwVersionMinor		= Version.Version.minorVersion;
		pArgs->EbootVersionExt.Version.dwVersionIncremental	= Version.Version.incrementalVersion;
		pArgs->EbootVersionExt.BuildVersion					= Version.BuildVersion;
	}
	if( 0 == getXLDRVersion( &Version ) )
	{
		pArgs->XldrVersion.dwVersionMajor		= Version.Version.majorVersion;
		pArgs->XldrVersion.dwVersionMinor		= Version.Version.minorVersion;
		pArgs->XldrVersion.dwVersionIncremental	= Version.Version.incrementalVersion;

		pArgs->XldrVersionExt.Version.dwVersionMajor		= Version.Version.majorVersion;
		pArgs->XldrVersionExt.Version.dwVersionMinor		= Version.Version.minorVersion;
		pArgs->XldrVersionExt.Version.dwVersionIncremental	= Version.Version.incrementalVersion;
		pArgs->XldrVersionExt.BuildVersion					= Version.BuildVersion;
	}
	memcpy(pArgs->EthMacAddress, g_bootCfg.EthMacAddress, sizeof(pArgs->EthMacAddress));
	
	memcpy(pArgs->WiFiMacAddress, g_bootCfg.WiFiMacAddress, sizeof(pArgs->WiFiMacAddress));

	memcpy(pArgs->CdsParams, g_bootCfg.CdsParams, sizeof(pArgs->CdsParams));

    // Image download depends on protocol
    g_eboot.bootDeviceType = OALKitlDeviceType(&g_bootCfg.bootDevLoc, g_bootDevices);

#ifndef SHIP_BUILD
	OALLog(L"Boot device %X, %X\r\n", g_eboot.bootDeviceType, &g_bootCfg.bootDevLoc);
#endif
	switch (g_eboot.bootDeviceType)
	{
        case BOOT_SDCARD_TYPE:
		{
			wcscpy(g_bootCfg.filename, L"nk.bin");

            rc = BLSDCardDownload(&g_bootCfg, g_bootDevices);
			if(BL_DOWNLOAD == rc)
			{
				TextWriteW( StartX, StartY, L"Loading NK.BIN from SD. Please, wait...", TX_LEFT | TX_BOTTOM, CL_TEXT );
			}
			break;
		}
        case OAL_KITL_TYPE_FLASH:
		{
			hTwl = OALTritonOpen();

			OALTritonWrite(hTwl, TWL_PHY_PWR_CTRL, 0x1);
			OALTritonWrite(hTwl, TWL_PHY_PWR_CTRL, 0x0);
			OALTritonWrite(hTwl, TWL_FUNC_CTRL_SET, (0x20) );
			
			#define FUNC_CTRL_RESET                         (1 << 5)
			
			OALTritonWrite(hTwl, TWL_FUNC_CTRL_SET, FUNC_CTRL_RESET);
			ticks = 100;	//OALGetTickCount() + 1000;
			do
			{
				OALTritonRead(hTwl, TWL_FUNC_CTRL, &bModeBits);
				OALStall(20);
			} while (bModeBits & FUNC_CTRL_RESET && (--ticks));

#ifndef SHIP_BUILD
			OALLog(L"PHY init ticks=%d\r\n", ticks);
#endif
			//  Read current time
			OALTritonWriteRegs(hTwl, TWL_RTC_CTRL_REG, &getTime, sizeof(getTime));
			OALStall(100);
			OALTritonReadRegs(hTwl, TWL_SECONDS_REG, time, sizeof(time));

			// PHY not OK?
			if (bModeBits & FUNC_CTRL_RESET)
			{
#ifndef SHIP_BUILD
				OALLog(L"Can't reset PHY. Performing device restart...\r\n");
#endif
				// Clear alarm status
				OALTritonWrite(hTwl, TWL_RTC_STATUS_REG, TWL_RTC_STATUS_ALARM);

				// current time + 1 sec.
				seconds = BCDTime2Seconds(time);
				seconds += 1;
				Seconds2BCDTime(seconds, time);

				// Set alarm
				OALTritonWriteRegs(hTwl, TWL_ALARM_SECONDS_REG, time, sizeof(time));

                //  Set toggle bit to latch time registers
                OALTritonWriteRegs(hTwl, TWL_RTC_CTRL_REG, &getTime, sizeof(getTime));
				OALTritonWrite(hTwl, TWL_RTC_INTERRUPTS_REG, TWL_RTC_INTERRUPTS_IT_ALARM);

				//OALTritonWrite(hTwl, TWL_P1_SW_EVENTS, 0x19);

				// Temporary as WAIT-ON to ACTIVE transition problem
				OALTritonWrite(hTwl, TWL_WATCHDOG_CFG, 0x01);
			}

			// For the backup registers using information see \TI_EVM_3530\SRC\oal\oalrtc\backup_domain_using.txt
			OALTritonReadRegs(hTwl, TWL_BACKUP_REG_A, backup, sizeof(backup));
			OALTritonReadRegs(hTwl, TWL_ALARM_SECONDS_REG, alarm, sizeof(alarm));
			// backup != alarm ?
			if(backup[0] != alarm[0] || backup[1] != alarm[1] || backup[2] != alarm[2] ||
			   backup[3] != alarm[3] || backup[4] != alarm[4] || backup[5] != alarm[5])
			{
#ifndef SHIP_BUILD
				OALLog(L"RTC alarm is not equals to backup\r\n");
#endif
				if(!backup[0] && !backup[1] && !backup[2] && !backup[3] && !backup[4] && !backup[5])
				{
#ifndef SHIP_BUILD
					OALLog(L"Backup is empty\r\n");
#endif
					OALTritonWrite(hTwl, TWL_RTC_STATUS_REG, TWL_RTC_STATUS_ALARM);
				}
				else
				{
#ifndef SHIP_BUILD
					OALLog(L"Backup exists\r\n");
#endif
					if (BCDTime2Seconds(alarm) > BCDTime2Seconds(time))
					{
#ifndef SHIP_BUILD
						OALLog(L"Restore RTC alarm\r\n");
#endif
						OALTritonWrite(hTwl, TWL_RTC_STATUS_REG, TWL_RTC_STATUS_ALARM);
						OALTritonWriteRegs(hTwl, TWL_ALARM_SECONDS_REG, backup, sizeof(backup));
					}
				}
			}

			OALTritonClose(hTwl);
#ifndef SHIP_BUILD
			OALLog(L"Continue boot\r\n");
#endif
            rc = BLFlashDownload(&g_bootCfg, g_bootDevices);
            break;
		}
        case OAL_KITL_TYPE_ETH:
		{
			// if rndis had been choosen for communication with desktop
			if(g_bootCfg.launch_manutool && (g_bootCfg.bootDevLoc.LogicalLoc == OMAP_USBHS_REGS_PA))
			{
				SetDownloadedFlag(FALSE);
#ifndef SHIP_BUILD
				OALLog(L"calling BLEthStartLDR ... \r\n");
#endif
				rc = BLEthStartLDR(&g_bootCfg, g_bootDevices);
				if(rc ==BL_JUMP)
				{
					// we finished the LDR communication here so change boot device to nand and  jump to the image

#ifndef SHIP_BUILD
					OALLog(L"Update global variables\r\n");
#endif
					g_bootCfg.bootDevLoc.IfcType = Internal;
					g_bootCfg.bootDevLoc.LogicalLoc = BSP_NAND_REGS_PA + 0x20;
					//	g_eboot.bootDeviceType = OAL_KITL_TYPE_FLASH;
					g_eboot.bootDeviceType = OALKitlDeviceType(&g_bootCfg.bootDevLoc, g_bootDevices);

					// if we didn't downloaded image into the RAM yet we need to do that now.
					if(!GetDownloadedFlag())
					{
#ifndef SHIP_BUILD
						OALLog(L"calling BLFlashDownload ...\r\n");
#endif
						rc = BLFlashDownload(&g_bootCfg, g_kitlDevices);
					}
				}
			}
			else // start eboot communication over ethernet
			{
	            rc = BLEthDownload(&g_bootCfg, g_bootDevices);
			}

            break;
		}
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
VOID
OEMLaunch(
   ULONG start, 
   ULONG size, 
   ULONG launch, 
   const ROMHDR *pRomHeader
    )
{
    BSP_ARGS *pArgs = OALPAtoCA(IMAGE_SHARE_ARGS_PA);

    OALMSG(OAL_FUNC, (
        L"+OEMLaunch(0x%08x, 0x%08x, 0x%08x, 0x%08x - %d/%d, launch - %x)\r\n", start, size,
        launch, pRomHeader, g_eboot.bootDeviceType, g_eboot.type,g_eboot.launchAddress
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

        case OAL_KITL_TYPE_ETH:
            BLEthConfig(pArgs);
            switch (g_eboot.type)
                {
                case DOWNLOAD_TYPE_FLASHRAM:
                    if (BLFlashDownload(&g_bootCfg, g_kitlDevices) != BL_JUMP)
                        {
                        OALMSG(OAL_ERROR, (L"ERROR: OEMLaunch: "
                            L"Image load from flash memory failed\r\n"
                            ));
                        goto cleanUp;
                        }
                    launch = g_eboot.launchAddress;
                    break;

                case DOWNLOAD_TYPE_RAM:
                    launch = (UINT32)OEMMapMemAddr(start, launch);
                    break;

                case DOWNLOAD_TYPE_EBOOT:
                case DOWNLOAD_TYPE_XLDR:
                    OALMSG(OAL_INFO, (L"INFO: "
                        L"XLDR/EBOOT/IPL downloaded, spin forever\r\n"
                        ));
                    while (TRUE);
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
#ifndef SHIP_BUILD
    OALLog(
        L"Launch Windows CE image by jumping to 0x%08x...\r\n\r\n", launch
        );
#endif

    OEMPlatformDeinit();
    OEMDebugDeinit();
    JumpTo(launch);

cleanUp:
    return;
}

//------------------------------------------------------------------------------
//
//  Function:   OEMMultiBinNotify
//
VOID
OEMMultiBinNotify(
    MultiBINInfo *pInfo
    )
{
    BOOL rc = FALSE;
    UINT32 base = IMAGE_WINCE_CODES_PA;
    UINT32 start, length;
    UINT32 ix;


    OALMSGS(OAL_FUNC, (
        L"+OEMMultiBinNotify(0x%08x -> %d)\r\n", pInfo, pInfo->dwNumRegions
        ));
    OALMSG(OAL_INFO, (
        L"Download file information:\r\n"
        ));
    OALMSG(OAL_INFO, (
        L"-----------------------------------------------------------\r\n"
        ));

    // Copy information to EBOOT structure and set also save address
    g_eboot.numRegions = pInfo->dwNumRegions;
    for (ix = 0; ix < pInfo->dwNumRegions; ix++)
        {
        g_eboot.region[ix].start = pInfo->Region[ix].dwRegionStart;
        g_eboot.region[ix].length = pInfo->Region[ix].dwRegionLength;
        g_eboot.region[ix].base = base;
        base += g_eboot.region[ix].length;
        OALMSG(OAL_INFO, (
            L"[%d]: Address=0x%08x  Length=0x%08x  Save=0x%08x\r\n",
            ix, g_eboot.region[ix].start, g_eboot.region[ix].length,
            g_eboot.region[ix].base
            ));
        }
    OALMSG(OAL_INFO, (L"-----------------------------------------------------------\r\n"));

    // Determine type of image downloaded
    if (g_eboot.numRegions > 1) 
        {
        OALMSG(OAL_ERROR, (L"ERROR: MultiXIP image is not supported\r\n"));
        goto cleanUp;
        }

    base = g_eboot.region[0].base;
    start = g_eboot.region[0].start;
    length = g_eboot.region[0].length; 
    
	if(BOOT_SDCARD_TYPE == g_eboot.bootDeviceType)
	{
		g_eboot.type = DOWNLOAD_TYPE_RAM;
	}
    else if(start == IMAGE_XLDR_CODE_PA)
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

    OALMSG(OAL_INFO, (
        L"Download file type: %d\r\n", g_eboot.type
    ));
    
    rc = TRUE;

cleanUp:
    if (!rc) 
        {
        OALMSG(OAL_ERROR, (L"Spin for ever...\r\n"));
        while (TRUE);
        }
    OALMSGS(OAL_FUNC, (L"-OEMMultiBinNotify\r\n"));
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
UINT8* 
OEMMapMemAddr(
    DWORD image,
    DWORD address
    )
{
    UINT8 *pAddress = NULL;

    OALMSG(OAL_FUNC, (L"+OEMMapMemAddr(0x%08x, 0x%08x)\r\n", image, address));

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
        OALMSG(OAL_ERROR, (L"ERROR: OEMMapMemAddr: "
            L"Invalid download type %d\r\n", g_eboot.type
        ));

    }


    OALMSGS(OAL_FUNC, (L"-OEMMapMemAddr(pAddress = 0x%08x)\r\n", pAddress));
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
BOOL
OEMIsFlashAddr(
    ULONG address
    )
{
    BOOL rc;

    OALMSG(OAL_FUNC, (L"+OEMIsFlashAddr(0x%08x)\r\n", address));

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

    OALMSG(OAL_FUNC, (L"-OEMIsFlashAddr(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:   OEMReadData
//
//  This function is called to read data from the transport during
//  the download process.
//
BOOL
OEMReadData(
    ULONG size, 
    UCHAR *pData
    )
{
	BOOL rc = FALSE;
    switch (g_eboot.bootDeviceType)
        {
        //#if BUILDING_EBOOT_SD
        case BOOT_SDCARD_TYPE:
            rc = BLSDCardReadData(size, pData);
            break;
        //#endif
        case OAL_KITL_TYPE_ETH:

			// if rndis had been choosen then alredy downloaded data is read from RAM
			// 
			if(g_bootCfg.launch_manutool && g_bootCfg.bootDevLoc.LogicalLoc == OMAP_USBHS_REGS_PA)
			{
				rc = LDRReadData(size, pData);
			}
			else // start eboot communication over ethernet
			{
				rc = BLEthReadData(size, pData);
			}
            break;
		case OAL_KITL_TYPE_FLASH:
		{
			if( g_bootCfg.bootDevLoc.LogicalLoc == BSP_NAND_REGS_PA )//temp!!!???
				rc = FlReadData(size, pData);
		}
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
VOID
OEMShowProgress(
    ULONG packetNumber
    )
{
}

//------------------------------------------------------------------------------
//
//  Function:  OALGetTickCount
//
UINT32
OALGetTickCount(
    )
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