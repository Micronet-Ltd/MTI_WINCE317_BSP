// All rights reserved ADENEO EMBEDDED 2010
/*
===============================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
===============================================================================
*/
//
//  File: prcm_device.h
//

//-----------------------------------------------------------------------------
// source clock mapping
static SourceDeviceClocks_t _SR_SourceClock = {1, {kSR_ALWON_FCLK}};
static SourceDeviceClocks_t _USIM_SourceClock = {1, {kSR_ALWON_FCLK}};
static SourceDeviceClocks_t _CPEFUSE_SourceClock = {1, {kEFUSE_ALWON_FCLK}};
static SourceDeviceClocks_t _SSI_SourceClock = {2, {kSSI_SSR_FCLK, kSSI_SST_FCLK}};
static SourceDeviceClocks_t _MCBSP1_SourceClock = {1, {kMCBSP1_CLKS}};
static SourceDeviceClocks_t _MCBSP2_SourceClock = {1, {kMCBSP2_CLKS}};
static SourceDeviceClocks_t _MCBSP3_SourceClock = {1, {kMCBSP3_CLKS}};
static SourceDeviceClocks_t _MCBSP4_SourceClock = {1, {kMCBSP4_CLKS}};
static SourceDeviceClocks_t _MCBSP5_SourceClock = {1, {kMCBSP5_CLKS}};
static SourceDeviceClocks_t _MMC1_SourceClock = {2, {k96M_FCLK, kCORE_32K_FCLK}};
static SourceDeviceClocks_t _MMC2_SourceClock = {2, {k96M_FCLK, kCORE_32K_FCLK}};
static SourceDeviceClocks_t _MMC3_SourceClock = {2, {k96M_FCLK, kCORE_32K_FCLK}};
static SourceDeviceClocks_t _GPT1_SourceClock = {1, {kGPT1_FCLK}};
static SourceDeviceClocks_t _GPT2_SourceClock = {1, {kGPT2_ALWON_FCLK}};
static SourceDeviceClocks_t _GPT3_SourceClock = {1, {kGPT3_ALWON_FCLK}};
static SourceDeviceClocks_t _GPT4_SourceClock = {1, {kGPT4_ALWON_FCLK}};
static SourceDeviceClocks_t _GPT5_SourceClock = {1, {kGPT5_ALWON_FCLK}};
static SourceDeviceClocks_t _GPT6_SourceClock = {1, {kGPT6_ALWON_FCLK}};
static SourceDeviceClocks_t _GPT7_SourceClock = {1, {kGPT7_ALWON_FCLK}};
static SourceDeviceClocks_t _GPT8_SourceClock = {1, {kGPT8_ALWON_FCLK}};
static SourceDeviceClocks_t _GPT9_SourceClock = {1, {kGPT9_ALWON_FCLK}};
static SourceDeviceClocks_t _GPT10_SourceClock = {1, {kGPT10_FCLK}};
static SourceDeviceClocks_t _GPT11_SourceClock = {1, {kGPT11_FCLK}};
static SourceDeviceClocks_t _GPT12_SourceClock = {1, {kSECURE_32K_FCLK}};
static SourceDeviceClocks_t _DSS1_SourceClock = {1, {kDSS1_ALWON_FCLK}};
static SourceDeviceClocks_t _DSS2_SourceClock = {1, {kDSS2_ALWON_FCLK}};
static SourceDeviceClocks_t _SGX_SourceClock ={1, {kSGX_FCLK}};
static SourceDeviceClocks_t _CSI2_SourceClock = {1, {kCSI2_96M_FCLK}};
static SourceDeviceClocks_t _CAM_SourceClock = {2, {kCAM_L3_ICLK, kCAM_MCLK}};
static SourceDeviceClocks_t _TV_SourceClock = {2, {kDSS_TV_FCLK, kDSS_96M_FCLK}};
static SourceDeviceClocks_t _DSS_SourceClock = {2, {kDSS1_ALWON_FCLK, kDSS2_ALWON_FCLK}};
static SourceDeviceClocks_t _USBOTG_SourceClock = {3, {kUSBHOST_SAR_FCLK, kUSBHOST_120M_FCLK, kUSBHOST_48M_FCLK}};
static SourceDeviceClocks_t _USBHOST1_SourceClock = {4, {kUSBHOST_48M_FCLK, kUSBHOST_L3_ICLK, kUSBHOST_L4_ICLK, kUSBHOST_120M_FCLK}};
static SourceDeviceClocks_t _USBHOST2_SourceClock = {4, {kUSBHOST_48M_FCLK, kUSBHOST_L3_ICLK, kUSBHOST_L4_ICLK, kUSBHOST_120M_FCLK}};
static SourceDeviceClocks_t _USBHOST3_SourceClock = {4, {kUSBHOST_48M_FCLK, kUSBHOST_L3_ICLK, kUSBHOST_L4_ICLK, kUSBHOST_120M_FCLK}};

static SourceDeviceClocks_t _32KWakeup_SourceClock = {1, {kWKUP_32K_FCLK}};
static SourceDeviceClocks_t _PER_48M_SourceClock = {1, {kPER_48M_FCLK}};
static SourceDeviceClocks_t _PER_32K_SourceClock = {1, {kPER_32K_ALWON_FCLK}};
static SourceDeviceClocks_t _COREL3_SourceClock = {1, {kCORE_L3_ICLK}};
static SourceDeviceClocks_t _COREL4_SourceClock = {1, {kL4_ICLK}};

static SourceDeviceClocks_t _120M_SourceClock = {1, {k120M_FCLK}};
static SourceDeviceClocks_t _96M_SourceClock = {1, {k96M_FCLK}};
static SourceDeviceClocks_t _48M_SourceClock = {1, {k48M_FCLK}};
static SourceDeviceClocks_t _12M_SourceClock = {1, {k12M_FCLK}};
static SourceDeviceClocks_t _32K_SourceClock = {1, {k32K_FCLK}};

//-----------------------------------------------------------------------------
// autoidle info

// CORE
AUTOIDLE_DECL(_autoIdle_D2D, FALSE, CM_CLKEN_D2D, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_SSI, FALSE, CM_CLKEN_SSI, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_HDQ, FALSE, CM_CLKEN_HDQ, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_ICR, FALSE, CM_CLKEN_ICR, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_I2C1, FALSE, CM_CLKEN_I2C1, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_I2C2, FALSE, CM_CLKEN_I2C2, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_I2C3, FALSE, CM_CLKEN_I2C3, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_MMC1, FALSE, CM_CLKEN_MMC1, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_MMC2, FALSE, CM_CLKEN_MMC2, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_MMC3, FALSE, CM_CLKEN_MMC3, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_AES2, FALSE, CM_CLKEN_AES2, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_DES2, FALSE, CM_CLKEN_DES2, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_UART1, FALSE, CM_CLKEN_UART1, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_UART2, FALSE, CM_CLKEN_UART2, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_MSPRO, FALSE, CM_CLKEN_MSPRO, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_SHA12, FALSE, CM_CLKEN_SHA12, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_MCBSP1, FALSE, CM_CLKEN_MCBSP1, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_MCBSP5, FALSE, CM_CLKEN_MCBSP5, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_MCSPI1, FALSE, CM_CLKEN_MCSPI1, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_MCSPI2, FALSE, CM_CLKEN_MCSPI2, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_MCSPI3, FALSE, CM_CLKEN_MCSPI3, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_MCSPI4, FALSE, CM_CLKEN_MCSPI4, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_GPT10, FALSE, CM_CLKEN_GPT10, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_GPT11, FALSE, CM_CLKEN_GPT11, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_OMAPCTRL, FALSE, CM_CLKEN_OMAPCTRL, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_HSOTGUSB, FALSE, CM_CLKEN_HSOTGUSB, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_MAILBOXES, FALSE, CM_CLKEN_MAILBOXES, cm_offset(CM_AUTOIDLE1_xxx));
AUTOIDLE_DECL(_autoIdle_PKA, FALSE, CM_CLKEN_PKA, cm_offset(CM_AUTOIDLE2_xxx));
AUTOIDLE_DECL(_autoIdle_RNG, FALSE, CM_CLKEN_RNG, cm_offset(CM_AUTOIDLE2_xxx));
AUTOIDLE_DECL(_autoIdle_AES1, FALSE, CM_CLKEN_AES1, cm_offset(CM_AUTOIDLE2_xxx));
AUTOIDLE_DECL(_autoIdle_DES1, FALSE, CM_CLKEN_DES1, cm_offset(CM_AUTOIDLE2_xxx));
AUTOIDLE_DECL(_autoIdle_SHA11, FALSE, CM_CLKEN_SHA11, cm_offset(CM_AUTOIDLE2_xxx));
AUTOIDLE_DECL(_autoIdle_USBTLL, FALSE, CM_CLKEN_USBTLL, cm_offset(CM_AUTOIDLE3_xxx));

// WAKEUP
AUTOIDLE_DECL(_autoIdle_GPT1, FALSE, CM_CLKEN_GPT1, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_GPT12, FALSE, CM_CLKEN_GPT12, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_32KSYNC, FALSE, CM_CLKEN_32KSYNC, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_GPIO1, FALSE, CM_CLKEN_GPIO1, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_WDT1, FALSE, CM_CLKEN_WDT1, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_WDT2, FALSE, CM_CLKEN_WDT2, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_USIM, FALSE, CM_CLKEN_USIM, cm_offset(CM_AUTOIDLE_xxx));

// PERIPHERAL
AUTOIDLE_DECL(_autoIdle_WDT3, FALSE, CM_CLKEN_WDT3, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_GPT2, FALSE, CM_CLKEN_GPT2, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_GPT3, FALSE, CM_CLKEN_GPT3, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_GPT4, FALSE, CM_CLKEN_GPT4, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_GPT5, FALSE, CM_CLKEN_GPT5, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_GPT6, FALSE, CM_CLKEN_GPT6, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_GPT7, FALSE, CM_CLKEN_GPT7, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_GPT8, FALSE, CM_CLKEN_GPT8, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_GPT9, FALSE, CM_CLKEN_GPT9, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_UART3, FALSE, CM_CLKEN_UART3, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_GPIO2, FALSE, CM_CLKEN_GPIO2, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_GPIO3, FALSE, CM_CLKEN_GPIO3, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_GPIO4, FALSE, CM_CLKEN_GPIO4, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_GPIO5, FALSE, CM_CLKEN_GPIO5, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_GPIO6, FALSE, CM_CLKEN_GPIO6, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_MCBSP2, FALSE, CM_CLKEN_MCBSP2, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_MCBSP3, FALSE, CM_CLKEN_MCBSP3, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_MCBSP4, FALSE, CM_CLKEN_MCBSP4, cm_offset(CM_AUTOIDLE_xxx));
AUTOIDLE_DECL(_autoIdle_UART4, FALSE, CM_CLKEN_UART4, cm_offset(CM_AUTOIDLE_xxx)); // 37xx only

// DSS
AUTOIDLE_DECL(_autoIdle_DSS, FALSE, CM_CLKEN_DSS, cm_offset(CM_AUTOIDLE_xxx));

// CAMERA
AUTOIDLE_DECL(_autoIdle_CAM, FALSE, CM_CLKEN_CAM, cm_offset(CM_AUTOIDLE_xxx));

// USBHOST
AUTOIDLE_DECL(_autoIdle_USBHOST, FALSE, CM_CLKEN_USBHOST, cm_offset(CM_AUTOIDLE_xxx));

//-----------------------------------------------------------------------------
// idlestatus info

// CORE
IDLESTAT_DECL(_idleStat_D2D, CM_IDLEST_ST_D2D, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_SSI, CM_IDLEST_ST_SSI, cm_offset(CM_IDLEST1_xxx));     // ES1.0
IDLESTAT_DECL(_idleStat_HDQ, CM_IDLEST_ST_HDQ, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_ICR, CM_IDLEST_ST_ICR, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_SDRC, CM_IDLEST_ST_SDRC, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_I2C1, CM_IDLEST_ST_I2C1, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_I2C2, CM_IDLEST_ST_I2C2, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_I2C3, CM_IDLEST_ST_I2C3, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_MMC1, CM_IDLEST_ST_MMC1, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_MMC2, CM_IDLEST_ST_MMC2, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_MMC3, CM_IDLEST_ST_MMC3, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_AES2, CM_IDLEST_ST_AES2, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_DES2, CM_IDLEST_ST_DES2, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_UART1, CM_IDLEST_ST_UART1, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_UART2, CM_IDLEST_ST_UART2, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_MSPRO, CM_IDLEST_ST_MSPRO, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_SHA12, CM_IDLEST_ST_SHA12, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_MCBSP1, CM_IDLEST_ST_MCBSP1, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_MCBSP5, CM_IDLEST_ST_MCBSP5, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_MCSPI1, CM_IDLEST_ST_MCSPI1, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_MCSPI2, CM_IDLEST_ST_MCSPI2, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_MCSPI3, CM_IDLEST_ST_MCSPI3, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_MCSPI4, CM_IDLEST_ST_MCSPI4, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_GPT10, CM_IDLEST_ST_GPT10, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_GPT11, CM_IDLEST_ST_GPT11, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_HSOTGUSB, CM_IDLEST_ST_HSOTGUSB_IDLE, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_OMAPCTRL, CM_IDLEST_ST_OMAPCTRL, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_MAILBOXES, CM_IDLEST_ST_MAILBOXES, cm_offset(CM_IDLEST1_xxx));
IDLESTAT_DECL(_idleStat_PKA, CM_IDLEST_ST_PKA, cm_offset(CM_IDLEST2_xxx));
IDLESTAT_DECL(_idleStat_RNG, CM_IDLEST_ST_RNG, cm_offset(CM_IDLEST2_xxx));
IDLESTAT_DECL(_idleStat_AES1, CM_IDLEST_ST_AES1, cm_offset(CM_IDLEST2_xxx));
IDLESTAT_DECL(_idleStat_DES1, CM_IDLEST_ST_DES1, cm_offset(CM_IDLEST2_xxx));
IDLESTAT_DECL(_idleStat_SHA11, CM_IDLEST_ST_SHA11, cm_offset(CM_IDLEST2_xxx));
IDLESTAT_DECL(_idleStat_USBTLL, CM_IDLEST_ST_USBTLL, cm_offset(CM_IDLEST3_xxx));
IDLESTAT_DECL(_idleStat_CPEFUSE, CM_IDLEST_ST_EFUSE, cm_offset(CM_IDLEST3_xxx));

// WAKEUP
IDLESTAT_DECL(_idleStat_GPT1, CM_IDLEST_ST_GPT1, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_GPT12, CM_IDLEST_ST_GPT12, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_32KSYNC, CM_IDLEST_ST_32KSYNC, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_GPIO1, CM_IDLEST_ST_GPIO1, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_WDT1, CM_IDLEST_ST_WDT1, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_WDT2, CM_IDLEST_ST_WDT2, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_USIM, CM_IDLEST_ST_USIM, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_SR1, CM_IDLEST_ST_SR1, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_SR2, CM_IDLEST_ST_SR2, cm_offset(CM_IDLEST_xxx));

// PERIPHERAL
IDLESTAT_DECL(_idleStat_WDT3, CM_IDLEST_ST_WDT3, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_UART3, CM_IDLEST_ST_UART3, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_GPIO2, CM_IDLEST_ST_GPIO2, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_GPIO3, CM_IDLEST_ST_GPIO3, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_GPIO4, CM_IDLEST_ST_GPIO4, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_GPIO5, CM_IDLEST_ST_GPIO5, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_GPIO6, CM_IDLEST_ST_GPIO6, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_MCBSP2, CM_IDLEST_ST_MCBSP2, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_MCBSP3, CM_IDLEST_ST_MCBSP3, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_MCBSP4, CM_IDLEST_ST_MCBSP4, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_GPT2, CM_IDLEST_ST_GPT2, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_GPT3, CM_IDLEST_ST_GPT3, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_GPT4, CM_IDLEST_ST_GPT4, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_GPT5, CM_IDLEST_ST_GPT5, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_GPT6, CM_IDLEST_ST_GPT6, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_GPT7, CM_IDLEST_ST_GPT7, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_GPT8, CM_IDLEST_ST_GPT8, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_GPT9, CM_IDLEST_ST_GPT9, cm_offset(CM_IDLEST_xxx));
IDLESTAT_DECL(_idleStat_UART4, CM_IDLEST_ST_UART4, cm_offset(CM_IDLEST_xxx)); // 37xx only

// DSS
IDLESTAT_DECL(_idleStat_DSS, CM_IDLEST_ST_DSS_IDLE, cm_offset(CM_IDLEST_xxx));


// CAMERA
IDLESTAT_DECL(_idleStat_CAM, CM_IDLEST_ST_CAM, cm_offset(CM_IDLEST_xxx));

// SGX
IDLESTAT_DECL(_idleStat_SGX, CM_IDLEST_ST_SGX, cm_offset(CM_IDLEST_xxx));

// IVA2
IDLESTAT_DECL(_idleStat_IVA2, CM_IDLEST_ST_IVA2, cm_offset(CM_IDLEST_xxx));

// USBHOST
IDLESTAT_DECL(_idleStat_USBHOST, CM_IDLEST_ST_USBHOST_IDLE, cm_offset(CM_IDLEST_xxx));

//-----------------------------------------------------------------------------
// iclk info

// CORE
ICLK_DECL(_iclk_D2D, 0, CM_CLKEN_D2D, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_SSI, 0, CM_CLKEN_SSI, cm_offset(CM_ICLKEN1_xxx)); 
ICLK_DECL(_iclk_HDQ, 0, CM_CLKEN_HDQ, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_ICR, 0, CM_CLKEN_ICR, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_SDRC, 0, CM_CLKEN_SDRC, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_I2C1, 0, CM_CLKEN_I2C1, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_I2C2, 0, CM_CLKEN_I2C2, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_I2C3, 0, CM_CLKEN_I2C3, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_MMC1, 0, CM_CLKEN_MMC1, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_MMC2, 0, CM_CLKEN_MMC2, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_MMC3, 0, CM_CLKEN_MMC3, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_AES2, 0, CM_CLKEN_AES2, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_DES2, 0, CM_CLKEN_DES2, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_UART1, 0, CM_CLKEN_UART1, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_UART2, 0, CM_CLKEN_UART2, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_MSPRO, 0, CM_CLKEN_MSPRO, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_SHA12, 0, CM_CLKEN_SHA12, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_MCBSP1, 0, CM_CLKEN_MCBSP1, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_MCBSP5, 0, CM_CLKEN_MCBSP5, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_MCSPI1, 0, CM_CLKEN_MCSPI1, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_MCSPI2, 0, CM_CLKEN_MCSPI2, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_MCSPI3, 0, CM_CLKEN_MCSPI3, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_MCSPI4, 0, CM_CLKEN_MCSPI4, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_GPT10, 0, CM_CLKEN_GPT10, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_GPT11, 0, CM_CLKEN_GPT11, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_OMAPCTRL, 0, CM_CLKEN_OMAPCTRL, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_HSOTGUSB, 0, CM_CLKEN_HSOTGUSB, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_MAILBOXES, 0, CM_CLKEN_MAILBOXES, cm_offset(CM_ICLKEN1_xxx));
ICLK_DECL(_iclk_PKA, 0, CM_CLKEN_PKA, cm_offset(CM_ICLKEN2_xxx));
ICLK_DECL(_iclk_RNG, 0, CM_CLKEN_RNG, cm_offset(CM_ICLKEN2_xxx));
ICLK_DECL(_iclk_AES1, 0, CM_CLKEN_AES1, cm_offset(CM_ICLKEN2_xxx));
ICLK_DECL(_iclk_DES1, 0, CM_CLKEN_DES1, cm_offset(CM_ICLKEN2_xxx));
ICLK_DECL(_iclk_SHA11, 0, CM_CLKEN_SHA11, cm_offset(CM_ICLKEN2_xxx));
ICLK_DECL(_iclk_USBTLL, 0, CM_CLKEN_USBTLL, cm_offset(CM_ICLKEN3_xxx));
ICLK_VDECL(_iclk_VRFB, 0, CM_V_CLKEN_VRFB, cm_offset(CM_ICLKEN3_xxx));   // no actual bit for VRFB exists

// WAKEUP
ICLK_DECL(_iclk_GPT1, 0, CM_CLKEN_GPT1, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_GPT12, 0, CM_CLKEN_GPT12, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_32KSYNC, 0, CM_CLKEN_32KSYNC, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_GPIO1, 0, CM_CLKEN_GPIO1, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_WDT1, 0, CM_CLKEN_WDT1, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_WDT2, 0, CM_CLKEN_WDT2, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_USIM, 0, CM_CLKEN_USIM, cm_offset(CM_ICLKEN_xxx));

// PERIPHERAL
ICLK_DECL(_iclk_WDT3, 0, CM_CLKEN_WDT3, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_GPT2, 0, CM_CLKEN_GPT2, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_GPT3, 0, CM_CLKEN_GPT3, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_GPT4, 0, CM_CLKEN_GPT4, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_GPT5, 0, CM_CLKEN_GPT5, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_GPT6, 0, CM_CLKEN_GPT6, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_GPT7, 0, CM_CLKEN_GPT7, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_GPT8, 0, CM_CLKEN_GPT8, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_GPT9, 0, CM_CLKEN_GPT9, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_UART3, 0, CM_CLKEN_UART3, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_GPIO2, 0, CM_CLKEN_GPIO2, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_GPIO3, 0, CM_CLKEN_GPIO3, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_GPIO4, 0, CM_CLKEN_GPIO4, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_GPIO5, 0, CM_CLKEN_GPIO5, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_GPIO6, 0, CM_CLKEN_GPIO6, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_MCBSP2, 0, CM_CLKEN_MCBSP2, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_MCBSP3, 0, CM_CLKEN_MCBSP3, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_MCBSP4, 0, CM_CLKEN_MCBSP4, cm_offset(CM_ICLKEN_xxx));
ICLK_DECL(_iclk_UART4, 0, CM_CLKEN_UART4, cm_offset(CM_ICLKEN_xxx)); // 37xx only

// DSS
ICLK_DECL(_iclk_DSS, 0, CM_CLKEN_DSS, cm_offset(CM_ICLKEN_xxx)); 

// CAMERA
ICLK_DECL(_iclk_CAM, 0, CM_CLKEN_CAM, cm_offset(CM_ICLKEN_xxx));

// IVA2
ICLK_VDECL(_iclk_IVA2, 0, CM_V_CLKEN_IVA2, cm_offset(CM_ICLKEN_xxx));

// SGX
ICLK_DECL(_iclk_SGX, 0, CM_CLKEN_ISGX, cm_offset(CM_ICLKEN_xxx));

// USBHOST
ICLK_DECL(_iclk_USBHOST, 0, CM_CLKEN_USBHOST, cm_offset(CM_ICLKEN_xxx));

FCLK_DECL(_iclk_USBHOST1, 0, 0, cm_offset(CM_ICLKEN_xxx));
FCLK_DECL(_iclk_USBHOST2, 0, 0, cm_offset(CM_ICLKEN_xxx));
FCLK_DECL(_iclk_USBHOST3, 0, 0, cm_offset(CM_ICLKEN_xxx));

//-----------------------------------------------------------------------------
// fclk info

// CORE
FCLK_DECL(_fclk_D2D, 0, CM_CLKEN_D2D, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_SSI, 0, CM_CLKEN_SSI, cm_offset(CM_FCLKEN1_xxx)); 
FCLK_DECL(_fclk_HDQ, 0, CM_CLKEN_HDQ, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_I2C1, 0, CM_CLKEN_I2C1, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_I2C2, 0, CM_CLKEN_I2C2, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_I2C3, 0, CM_CLKEN_I2C3, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_MMC1, 0, CM_CLKEN_MMC1, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_MMC2, 0, CM_CLKEN_MMC2, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_MMC3, 0, CM_CLKEN_MMC3, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_UART1, 0, CM_CLKEN_UART1, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_UART2, 0, CM_CLKEN_UART2, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_MSPRO, 0, CM_CLKEN_MSPRO, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_MCBSP1, 0, CM_CLKEN_MCBSP1, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_MCBSP5, 0, CM_CLKEN_MCBSP5, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_MCSPI1, 0, CM_CLKEN_MCSPI1, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_MCSPI2, 0, CM_CLKEN_MCSPI2, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_MCSPI3, 0, CM_CLKEN_MCSPI3, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_MCSPI4, 0, CM_CLKEN_MCSPI4, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_GPT10, 0, CM_CLKEN_GPT10, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_GPT11, 0, CM_CLKEN_GPT11, cm_offset(CM_FCLKEN1_xxx));
FCLK_DECL(_fclk_USBTLL, 0, CM_CLKEN_USBTLL, cm_offset(CM_FCLKEN3_xxx));
FCLK_DECL(_fclk_CPEFUSE, 0, CM_CLKEN_EFUSE, cm_offset(CM_FCLKEN3_xxx));
FCLK_DECL(_fclk_TS, 0, CM_CLKEN_TS, cm_offset(CM_FCLKEN3_xxx));

// WAKEUP
FCLK_DECL(_fclk_GPT1, 0, CM_CLKEN_GPT1, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_GPIO1, 0, CM_CLKEN_GPIO1, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_WDT2, 0, CM_CLKEN_WDT2, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_USIM, 0, CM_CLKEN_USIM, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_SR1, 0, CM_CLKEN_SR1, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_SR2, 0, CM_CLKEN_SR2, cm_offset(CM_FCLKEN_xxx));

// PERIPHERAL
FCLK_DECL(_fclk_WDT3, 0, CM_CLKEN_WDT3, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_UART3, 0, CM_CLKEN_UART3, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_GPIO2, 0, CM_CLKEN_GPIO2, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_GPIO3, 0, CM_CLKEN_GPIO3, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_GPIO4, 0, CM_CLKEN_GPIO4, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_GPIO5, 0, CM_CLKEN_GPIO5, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_GPIO6, 0, CM_CLKEN_GPIO6, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_MCBSP2, 0, CM_CLKEN_MCBSP2, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_MCBSP3, 0, CM_CLKEN_MCBSP3, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_MCBSP4, 0, CM_CLKEN_MCBSP4, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_GPT2, 0, CM_CLKEN_GPT2, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_GPT3, 0, CM_CLKEN_GPT3, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_GPT4, 0, CM_CLKEN_GPT4, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_GPT5, 0, CM_CLKEN_GPT5, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_GPT6, 0, CM_CLKEN_GPT6, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_GPT7, 0, CM_CLKEN_GPT7, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_GPT8, 0, CM_CLKEN_GPT8, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_GPT9, 0, CM_CLKEN_GPT9, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_UART4, 0, CM_CLKEN_UART4, cm_offset(CM_FCLKEN_xxx)); // 37xx only

// DSS
FCLK_DECL(_fclk_DSS, 0, CM_CLKEN_DSS, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_DSS1, 0, CM_CLKEN_DSS1, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_DSS2, 0, CM_CLKEN_DSS2, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_TVOUT, 0, CM_CLKEN_TV, cm_offset(CM_FCLKEN_xxx));


// CAMERA
FCLK_DECL(_fclk_CAM, 0, CM_CLKEN_CAM, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_CSI2, 0, CM_CLKEN_CSI2, cm_offset(CM_FCLKEN_xxx));

// SGX
FCLK_DECL(_fclk_3D, 0, CM_CLKEN_3D, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_SGX, 0, CM_CLKEN_FSGX, cm_offset(CM_FCLKEN_xxx));

// IVA2
FCLK_DECL(_fclk_IVA2, 0, CM_CLKEN_IVA2, cm_offset(CM_FCLKEN_xxx));

// USBHOST
FCLK_DECL(_fclk_HSUSB2, 0, CM_CLKEN_HSUSB2, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_HSUSB1, 0, CM_CLKEN_HSUSB1, cm_offset(CM_FCLKEN_xxx));

FCLK_DECL(_fclk_USBHOST1, 0, 0, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_USBHOST2, 0, 0, cm_offset(CM_FCLKEN_xxx));
FCLK_DECL(_fclk_USBHOST3, 0, 0, cm_offset(CM_FCLKEN_xxx));

//-----------------------------------------------------------------------------
// wken info

// CORE
WKEN_DECL(_wken_D2D, CM_CLKEN_D2D, prm_offset(PM_WKEN1_xxx));
WKEN_DECL(_wken_I2C1, CM_CLKEN_I2C1, prm_offset(PM_WKEN1_xxx));
WKEN_DECL(_wken_I2C2, CM_CLKEN_I2C2, prm_offset(PM_WKEN1_xxx));
WKEN_DECL(_wken_I2C3, CM_CLKEN_I2C3, prm_offset(PM_WKEN1_xxx));
WKEN_DECL(_wken_MMC1, CM_CLKEN_MMC1, prm_offset(PM_WKEN1_xxx));
WKEN_DECL(_wken_MMC2, CM_CLKEN_MMC2, prm_offset(PM_WKEN1_xxx));
WKEN_DECL(_wken_MMC3, CM_CLKEN_MMC3, prm_offset(PM_WKEN1_xxx));
WKEN_DECL(_wken_UART1, CM_CLKEN_UART1, prm_offset(PM_WKEN1_xxx));
WKEN_DECL(_wken_UART2, CM_CLKEN_UART2, prm_offset(PM_WKEN1_xxx));
WKEN_DECL(_wken_MCBSP1, CM_CLKEN_MCBSP1, prm_offset(PM_WKEN1_xxx));
WKEN_DECL(_wken_MCBSP5, CM_CLKEN_MCBSP5, prm_offset(PM_WKEN1_xxx));
WKEN_DECL(_wken_MCSPI1, CM_CLKEN_MCSPI1, prm_offset(PM_WKEN1_xxx));
WKEN_DECL(_wken_MCSPI2, CM_CLKEN_MCSPI2, prm_offset(PM_WKEN1_xxx));
WKEN_DECL(_wken_MCSPI3, CM_CLKEN_MCSPI3, prm_offset(PM_WKEN1_xxx));
WKEN_DECL(_wken_MCSPI4, CM_CLKEN_MCSPI4, prm_offset(PM_WKEN1_xxx));
WKEN_DECL(_wken_GPT10, CM_CLKEN_GPT10, prm_offset(PM_WKEN1_xxx));
WKEN_DECL(_wken_GPT11, CM_CLKEN_GPT11, prm_offset(PM_WKEN1_xxx));
WKEN_DECL(_wken_HSOTGUSB, CM_CLKEN_HSOTGUSB, prm_offset(PM_WKEN1_xxx)); // ES1.0
WKEN_DECL(_wken_USBTLL, CM_CLKEN_USBTLL, prm_offset(PM_WKEN3_xxx));

// WAKEUP
WKEN_DECL(_wken_GPT1, CM_CLKEN_GPT1, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_GPIO1, CM_CLKEN_GPIO1, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_SR1, CM_CLKEN_SR1, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_SR2, CM_CLKEN_SR2, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_IO, CM_CLKEN_IO, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_USIM, CM_CLKEN_USIM, prm_offset(PM_WKEN_xxx));

// PERIPHERAL
WKEN_DECL(_wken_UART3, CM_CLKEN_UART3, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_GPIO2, CM_CLKEN_GPIO2, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_GPIO3, CM_CLKEN_GPIO3, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_GPIO4, CM_CLKEN_GPIO4, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_GPIO5, CM_CLKEN_GPIO5, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_GPIO6, CM_CLKEN_GPIO6, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_MCBSP2, CM_CLKEN_MCBSP2, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_MCBSP3, CM_CLKEN_MCBSP3, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_MCBSP4, CM_CLKEN_MCBSP4, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_GPT2, CM_CLKEN_GPT2, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_GPT3, CM_CLKEN_GPT3, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_GPT4, CM_CLKEN_GPT4, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_GPT5, CM_CLKEN_GPT5, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_GPT6, CM_CLKEN_GPT6, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_GPT7, CM_CLKEN_GPT7, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_GPT8, CM_CLKEN_GPT8, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_GPT9, CM_CLKEN_GPT9, prm_offset(PM_WKEN_xxx));
WKEN_DECL(_wken_UART4, CM_CLKEN_UART4, prm_offset(PM_WKEN_xxx)); // 37xx only

// DSS
WKEN_DECL(_wken_DSS, CM_CLKEN_DSS, prm_offset(PM_WKEN_xxx));

// USBHOST
WKEN_DECL(_wken_USBHOST, CM_CLKEN_USBHOST, prm_offset(PM_WKEN_xxx));

//-----------------------------------------------------------------------------

DeviceLookupEntry s_rgDeviceLookupTable[] =
{       
    {
// core domain clocks    
//-------------------
        POWERDOMAIN_CORE,                   // OMAP_DEVICE_SSI
        &_fclk_SSI,    
        &_iclk_SSI,
        NULL,
        &_idleStat_SSI,     
        &_autoIdle_SSI,     
        &_SSI_SourceClock
    }, {
        POWERDOMAIN_CORE,                   // OMAP_DEVICE_SDRC
        NULL,    
        &_iclk_SDRC,  
        NULL,
        &_idleStat_SDRC,     
        NULL,     
        &_COREL3_SourceClock,   
    }, {
        POWERDOMAIN_CORE,                   // OMAP_DEVICE_D2D
        &_fclk_D2D,
        &_iclk_D2D,  
        &_wken_D2D,
        &_idleStat_D2D,     
        &_autoIdle_D2D,     
        NULL,                   
    }, {
        POWERDOMAIN_CORE,                   // OMAP_DEVICE_HSOTGUSB
        NULL,
        &_iclk_HSOTGUSB,
        &_wken_HSOTGUSB,
        &_idleStat_HSOTGUSB,
        &_autoIdle_HSOTGUSB,
        &_USBOTG_SourceClock,  
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_OMAPCTRL
        NULL,
        &_iclk_OMAPCTRL, 
        NULL,
        &_idleStat_OMAPCTRL,
        &_autoIdle_OMAPCTRL,
        &_COREL4_SourceClock,   
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_MAILBOXES
        NULL,
        &_iclk_MAILBOXES, 
        NULL,
        &_idleStat_MAILBOXES,
        &_autoIdle_MAILBOXES,        
        &_COREL4_SourceClock,   
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_MCBSP1
        &_fclk_MCBSP1,
        &_iclk_MCBSP1,
        &_wken_MCBSP1,
        &_idleStat_MCBSP1,
        &_autoIdle_MCBSP1,
        &_MCBSP1_SourceClock,   
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_MCBSP5
        &_fclk_MCBSP5,
        &_iclk_MCBSP5,
        &_wken_MCBSP5,
        &_idleStat_MCBSP5,
        &_autoIdle_MCBSP5,
        &_MCBSP5_SourceClock,   
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_GPTIMER10  //10
        &_fclk_GPT10,
        &_iclk_GPT10,
        &_wken_GPT10,
        &_idleStat_GPT10,
        &_autoIdle_GPT10,
        &_GPT10_SourceClock,    
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_GPTIMER11
        &_fclk_GPT11,
        &_iclk_GPT11,
        &_wken_GPT11,
        &_idleStat_GPT11,
        &_autoIdle_GPT11,
        &_GPT11_SourceClock,    
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_UART1
        &_fclk_UART1,
        &_iclk_UART1,
        &_wken_UART1,
        &_idleStat_UART1,
        &_autoIdle_UART1,
        &_48M_SourceClock,      
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_UART2
        &_fclk_UART2,
        &_iclk_UART2,
        &_wken_UART2,
        &_idleStat_UART2,
        &_autoIdle_UART2,
        &_48M_SourceClock,      
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_I2C1
        &_fclk_I2C1,
        &_iclk_I2C1,
        &_wken_I2C1,
        &_idleStat_I2C1,
        &_autoIdle_I2C1,
        &_96M_SourceClock,      
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_I2C2
        &_fclk_I2C2,
        &_iclk_I2C2,
        &_wken_I2C2,
        &_idleStat_I2C2,
        &_autoIdle_I2C2,
        &_96M_SourceClock,      
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_I2C3
        &_fclk_I2C3,
        &_iclk_I2C3,
        &_wken_I2C3,
        &_idleStat_I2C3,
        &_autoIdle_I2C3,
        &_96M_SourceClock,      
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_MCSPI1
        &_fclk_MCSPI1,
        &_iclk_MCSPI1,
        &_wken_MCSPI1,
        &_idleStat_MCSPI1,
        &_autoIdle_MCSPI1,
        &_48M_SourceClock,      
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_MCSPI2
        &_fclk_MCSPI2,
        &_iclk_MCSPI2,
        &_wken_MCSPI2,
        &_idleStat_MCSPI2,
        &_autoIdle_MCSPI2,
        &_48M_SourceClock,      
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_MCSPI3
        &_fclk_MCSPI3,
        &_iclk_MCSPI3,
        &_wken_MCSPI3,
        &_idleStat_MCSPI3,
        &_autoIdle_MCSPI3,
        &_48M_SourceClock,      
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_MCSPI4 //20
        &_fclk_MCSPI4,
        &_iclk_MCSPI4,
        &_wken_MCSPI4,
        &_idleStat_MCSPI4,
        &_autoIdle_MCSPI4,
        &_48M_SourceClock,      
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_HDQ
        &_fclk_HDQ,
        &_iclk_HDQ,
        NULL,
        &_idleStat_HDQ,
        &_autoIdle_HDQ,
        &_12M_SourceClock,      
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_MSPRO
        &_fclk_MSPRO,
        &_iclk_MSPRO,
        NULL,
        &_idleStat_MSPRO,
        &_autoIdle_MSPRO,
        &_96M_SourceClock,      
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_MMC1
        &_fclk_MMC1,
        &_iclk_MMC1,
        &_wken_MMC1,
        &_idleStat_MMC1,
        &_autoIdle_MMC1,
        &_MMC1_SourceClock,     
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_MMC2
        &_fclk_MMC2,
        &_iclk_MMC2,
        &_wken_MMC2,
        &_idleStat_MMC2,
        &_autoIdle_MMC2,
        &_MMC2_SourceClock,     
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_MMC3
        &_fclk_MMC3,
        &_iclk_MMC3,
        &_wken_MMC3,
        &_idleStat_MMC3,
        &_autoIdle_MMC3,
        &_MMC3_SourceClock,     
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_DES2
        NULL,
        &_iclk_DES2,
        NULL,
        &_idleStat_DES2,
        &_autoIdle_DES2,
        NULL, 
    }, {
        POWERDOMAIN_CORE,                       //OMAP_DEVICE_SHA12
        NULL,
        &_iclk_SHA12,
        NULL,
        &_idleStat_SHA12,
        &_autoIdle_SHA12,
        NULL,
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_AES2
        NULL,    
        &_iclk_AES2,          
        NULL,
        &_idleStat_AES2,     
        &_autoIdle_AES2,
        NULL,
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_ICR
        NULL,    
        &_iclk_ICR,    
        NULL,
        &_idleStat_ICR,     
        &_autoIdle_ICR,
        NULL,
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_DES1  //30
        NULL,    
        &_iclk_DES1,          
        NULL,
        &_idleStat_DES1,     
        &_autoIdle_DES1,
        NULL,
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_SHA11
        NULL,    
        &_iclk_SHA11,  
        NULL,
        &_idleStat_SHA11,     
        &_autoIdle_SHA11,
        NULL,
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_RNG
        NULL,    
        &_iclk_RNG,    
        NULL,
        &_idleStat_RNG,     
        &_autoIdle_RNG,
        NULL,
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_AES1
        NULL,    
        &_iclk_AES1,  
        NULL,
        &_idleStat_AES1,     
        &_autoIdle_AES1,
        NULL,
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_PKA
        NULL,    
        &_iclk_PKA,   
        NULL,
        &_idleStat_PKA,     
        &_autoIdle_PKA,
        NULL,
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_USBTLL
        &_fclk_USBTLL,    
        &_iclk_USBTLL, 
        &_wken_USBTLL,
        &_idleStat_USBTLL,     
        &_autoIdle_USBTLL,
        &_120M_SourceClock,
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_TS
        &_fclk_TS,    
        NULL,
        NULL,
        NULL,     
        NULL,
        NULL,
    }, {
        POWERDOMAIN_CORE,                       // OMAP_CLOCK_EFUSE
        &_fclk_CPEFUSE,    
        NULL,
        NULL,
        &_idleStat_CPEFUSE,     
        NULL,
        &_CPEFUSE_SourceClock,
    }, {
// WAKUP domain clocks    
        POWERDOMAIN_WAKEUP,                     // OMAP_DEVICE_GPTIMER1
        &_fclk_GPT1,    
        &_iclk_GPT1,
        &_wken_GPT1,
        &_idleStat_GPT1,     
        &_autoIdle_GPT1,
        &_GPT1_SourceClock,
    }, {
        POWERDOMAIN_WAKEUP,                     // OMAP_DEVICE_GPTIMER12
        NULL,    
        &_iclk_GPT12,  
        NULL,
        &_idleStat_GPT12,     
        &_autoIdle_GPT12,
        &_GPT12_SourceClock,
    }, {
        POWERDOMAIN_WAKEUP,                     // OMAP_DEVICE_32KSYNC //40
        NULL,    
        &_iclk_32KSYNC, 
        NULL,
        &_idleStat_32KSYNC,     
        &_autoIdle_32KSYNC,
        &_32K_SourceClock,
    }, {
        POWERDOMAIN_WAKEUP,                     // OMAP_DEVICE_WDT1
        NULL,    
        &_iclk_WDT1,
        NULL,
        &_idleStat_WDT1,     
        &_autoIdle_WDT1,
        &_32K_SourceClock,
    }, {
        POWERDOMAIN_WAKEUP,                     // OMAP_DEVICE_WDT2
        &_fclk_WDT2,    
        &_iclk_WDT2,
        NULL,
        &_idleStat_WDT2,     
        &_autoIdle_WDT2,
        &_32KWakeup_SourceClock,
    }, {
        POWERDOMAIN_WAKEUP,                     // OMAP_DEVICE_GPIO1
        &_fclk_GPIO1,    
        &_iclk_GPIO1,
        &_wken_GPIO1,
        &_idleStat_GPIO1,     
        &_autoIdle_GPIO1,
        &_32KWakeup_SourceClock,
    }, {
        POWERDOMAIN_WAKEUP,                     // OMAP_DEVICE_SR1
        &_fclk_SR1,    
        NULL,
        &_wken_SR1,
        &_idleStat_SR1,     
        NULL,
        &_SR_SourceClock,
    }, {
        POWERDOMAIN_WAKEUP,                     // OMAP_DEVICE_SR2
        &_fclk_SR2,    
        NULL,
        &_wken_SR2,
        &_idleStat_SR2,     
        NULL,
        &_SR_SourceClock,
    }, {
        POWERDOMAIN_WAKEUP,                     // OMAP_DEVICE_USIM
        &_fclk_USIM,    
        &_iclk_USIM,
        &_wken_USIM,
        &_idleStat_USIM,     
        &_autoIdle_USIM,
        &_USIM_SourceClock,
    }, {
// per domain clocks        
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_GPIO2    
        &_fclk_GPIO2,    
        &_iclk_GPIO2,
        &_wken_GPIO2,
        &_idleStat_GPIO2,     
        &_autoIdle_GPIO2,
        &_PER_32K_SourceClock,
    }, {
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_GPIO3
        &_fclk_GPIO3,    
        &_iclk_GPIO3,
        &_wken_GPIO3,
        &_idleStat_GPIO3,     
        &_autoIdle_GPIO3,
        &_PER_32K_SourceClock,
    }, {
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_GPIO4
        &_fclk_GPIO4,    
        &_iclk_GPIO4,
        &_wken_GPIO4,
        &_idleStat_GPIO4,     
        &_autoIdle_GPIO4,
        &_PER_32K_SourceClock,
    }, {
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_GPIO5 //50
        &_fclk_GPIO5,    
        &_iclk_GPIO5,
        &_wken_GPIO5,
        &_idleStat_GPIO5,     
        &_autoIdle_GPIO5,
        &_PER_32K_SourceClock,
    }, {
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_GPIO6
        &_fclk_GPIO6,    
        &_iclk_GPIO6,
        &_wken_GPIO6,
        &_idleStat_GPIO6,     
        &_autoIdle_GPIO6,
        &_PER_32K_SourceClock,
    }, {
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_MCBSP2
        &_fclk_MCBSP2,    
        &_iclk_MCBSP2,
        &_wken_MCBSP2,
        &_idleStat_MCBSP2,     
        &_autoIdle_MCBSP2,
        &_MCBSP2_SourceClock,
    }, {
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_MCBSP3
        &_fclk_MCBSP3,    
        &_iclk_MCBSP3,
        &_wken_MCBSP3,
        &_idleStat_MCBSP3,     
        &_autoIdle_MCBSP3,
        &_MCBSP3_SourceClock,
    }, {
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_MCBSP4
        &_fclk_MCBSP4,    
        &_iclk_MCBSP4,
        &_wken_MCBSP4,
        &_idleStat_MCBSP4,     
        &_autoIdle_MCBSP4,
        &_MCBSP4_SourceClock,
    }, {
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_GPTIMER2
        &_fclk_GPT2,    
        &_iclk_GPT2,
        &_wken_GPT2,
        &_idleStat_GPT2,     
        &_autoIdle_GPT2,
        &_GPT2_SourceClock,
    }, {
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_GPTIMER3
        &_fclk_GPT3,    
        &_iclk_GPT3,
        &_wken_GPT3,
        &_idleStat_GPT3,     
        &_autoIdle_GPT3,
        &_GPT3_SourceClock,
    }, {
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_GPTIMER4
        &_fclk_GPT4,    
        &_iclk_GPT4,
        &_wken_GPT4,
        &_idleStat_GPT4,     
        &_autoIdle_GPT4,
        &_GPT4_SourceClock,
    }, {
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_GPTIMER5
        &_fclk_GPT5,    
        &_iclk_GPT5,
        &_wken_GPT5,
        &_idleStat_GPT5,     
        &_autoIdle_GPT5,
        &_GPT5_SourceClock,
    }, {
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_GPTIMER6
        &_fclk_GPT6,    
        &_iclk_GPT6,
        &_wken_GPT6,
        &_idleStat_GPT6,     
        &_autoIdle_GPT6,
        &_GPT6_SourceClock,
    }, {
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_GPTIMER7 //60
        &_fclk_GPT7,    
        &_iclk_GPT7,
        &_wken_GPT7,
        &_idleStat_GPT7,     
        &_autoIdle_GPT7,
        &_GPT7_SourceClock,
    }, {
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_GPTIMER8
        &_fclk_GPT8,    
        &_iclk_GPT8,
        &_wken_GPT8,
        &_idleStat_GPT8,     
        &_autoIdle_GPT8,
        &_GPT8_SourceClock,
    }, {
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_GPTIMER9
        &_fclk_GPT9,    
        &_iclk_GPT9,
        &_wken_GPT9,
        &_idleStat_GPT9,     
        &_autoIdle_GPT9,
        &_GPT9_SourceClock,
    }, {
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_UART3
        &_fclk_UART3,    
        &_iclk_UART3,
        &_wken_UART3,
        &_idleStat_UART3,     
        &_autoIdle_UART3,
        &_PER_48M_SourceClock,
    }, {
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_WDT3
        &_fclk_WDT3,    
        &_iclk_WDT3,
        NULL,
        &_idleStat_WDT3,     
        &_autoIdle_WDT3,
        &_PER_32K_SourceClock,
    }, {
// DSS domain clocks    
        POWERDOMAIN_DSS,                        // OMAP_DEVICE_DSS
        &_fclk_DSS,    
        &_iclk_DSS,
        &_wken_DSS,
        &_idleStat_DSS,     
        &_autoIdle_DSS,
        &_DSS_SourceClock,
    }, {
        POWERDOMAIN_DSS,                        // OMAP_DEVICE_DSS1
        &_fclk_DSS1,    
        &_iclk_DSS,                             
        NULL,
        NULL,     
        NULL,
        &_DSS1_SourceClock,
    }, {
        POWERDOMAIN_DSS,                        // OMAP_DEVICE_DSS2
        &_fclk_DSS2,    
        &_iclk_DSS,                             
        NULL,
        NULL,     
        NULL,
        &_DSS2_SourceClock,
    }, {
        POWERDOMAIN_DSS,                        // OMAP_DEVICE_TVOUT
        &_fclk_TVOUT,    
        &_iclk_DSS,
        NULL,                                   //&_wken_DSS,
        NULL,     
        NULL,
        &_TV_SourceClock,
    }, {
// CAMERA domain clocks    
        POWERDOMAIN_CAMERA,                     // OMAP_DEVICE_CAMERA
        &_fclk_CAM,
        &_iclk_CAM,
        NULL,
        &_idleStat_CAM,
        &_autoIdle_CAM,
        &_CAM_SourceClock,
    }, {
        POWERDOMAIN_CAMERA,                     // OMAP_DEVICE_CSI2
        &_fclk_CSI2,
        &_iclk_CAM,
        NULL,
        NULL,
        NULL,
        &_CSI2_SourceClock,
    }, {
// IVA domain clocks    
        POWERDOMAIN_IVA2,                       // OMAP_DEVICE_DSP
        &_fclk_IVA2,    
        &_iclk_IVA2,
        NULL,
        NULL,                                   // &_idleStat_IVA2,
        NULL,
        NULL,
    }, {
// GFX domain clocks    
        POWERDOMAIN_SGX,                        // OMAP_DEVICE_2D    // UNDONE need to remove
        NULL,    
        NULL,
        NULL,
        NULL,    
        NULL,
        NULL,
    }, {
        POWERDOMAIN_SGX,                        // OMAP_DEVICE_3D
        &_fclk_3D,    
        &_iclk_SGX,
        NULL,
        &_idleStat_SGX,     
        NULL,
        NULL,
    }, {
        POWERDOMAIN_SGX,                        // OMAP_DEVICE_SGX
        &_fclk_SGX,    
        &_iclk_SGX,
        NULL,
        &_idleStat_SGX,     
        NULL,
        &_SGX_SourceClock,
    }, {
        POWERDOMAIN_USBHOST,                    // OMAP_DEVICE_HSUSB1
        &_fclk_HSUSB1,    
        &_iclk_USBHOST,
        &_wken_USBHOST,
        &_idleStat_USBHOST,     
        &_autoIdle_USBHOST,
        NULL,
    }, {
        POWERDOMAIN_USBHOST,                    // OMAP_DEVICE_HSUSB2
        &_fclk_HSUSB2,    
        NULL,
        NULL,
        NULL,     
        NULL,
        NULL,
    }, {
        POWERDOMAIN_USBHOST,                    // OMAP_DEVICE_USBHOST1
        &_fclk_USBHOST1,    
        &_iclk_USBHOST1,
        NULL,
        NULL,     
        NULL,
        &_USBHOST1_SourceClock,
    }, {
        POWERDOMAIN_USBHOST,                    // OMAP_DEVICE_USBHOST2
        &_fclk_USBHOST2,    
        &_iclk_USBHOST2,
        NULL,
        NULL,     
        NULL,
        &_USBHOST2_SourceClock,
    }, {
        POWERDOMAIN_USBHOST,                    // OMAP_DEVICE_USBHOST3
        &_fclk_USBHOST3,    
        &_iclk_USBHOST3,
        NULL,
        NULL,     
        NULL,
        &_USBHOST3_SourceClock,
    }, {
        POWERDOMAIN_CORE,                       // OMAP_DEVICE_VRFB
        NULL,    
        &_iclk_VRFB,
        NULL,
        NULL,     
        NULL,
        NULL,
    }, {
        POWERDOMAIN_PERIPHERAL,                 // OMAP_DEVICE_UART4 (37xx only)
        &_fclk_UART4,    
        &_iclk_UART4,
        &_wken_UART4,
        &_idleStat_UART4,
        &_autoIdle_UART4,
        &_PER_48M_SourceClock,
    }
};

//-----------------------------------------------------------------------------
static UINT s_rgActiveDomainDeviceCount[POWERDOMAIN_COUNT] = 
{
    0,      // POWERDOMAIN_WAKEUP
    0,      // POWERDOMAIN_CORE
    0,      // POWERDOMAIN_PERIPHERAL
    0,      // POWERDOMAIN_USBHOST
    0,      // POWERDOMAIN_EMULATION
    0,      // POWERDOMAIN_MPU
    0,      // POWERDOMAIN_DSS
    0,      // POWERDOMAIN_NEON
    0,      // POWERDOMAIN_IVA2
    0,      // POWERDOMAIN_CAMERA
    0,      // POWERDOMAIN_SGX
    0,      // POWERDOMAIN_EFUSE
    0,      // POWERDOMAIN_SMARTREFLEX
};

//-----------------------------------------------------------------------------
// moved to src\oal\oallib\oem_prcm_device.c
extern DomainDependencyRequirement s_rgClockDomainDependency[POWERDOMAIN_COUNT];
//-----------------------------------------------------------------------------
