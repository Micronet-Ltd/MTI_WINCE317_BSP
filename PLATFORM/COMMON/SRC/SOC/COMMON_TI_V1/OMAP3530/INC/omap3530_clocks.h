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
//  File:  omap3530_clocks.h
//
//  This header defines all relevant clocks and power domains for omap35xx.
//
#ifndef __OMAP3530_CLOCKS_H
#define __OMAP3530_CLOCKS_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
#define OMAP3530_GENERIC        0xFFFFFFFF

typedef enum {
    OMAP_DEVICE_SSI = 0,
    OMAP_DEVICE_SDRC,
    OMAP_DEVICE_D2D,
    OMAP_DEVICE_HSOTGUSB,
    OMAP_DEVICE_OMAPCTRL,
    OMAP_DEVICE_MAILBOXES,
    OMAP_DEVICE_MCBSP1,
    OMAP_DEVICE_MCBSP5,
    OMAP_DEVICE_GPTIMER10,
    OMAP_DEVICE_GPTIMER11,    
    OMAP_DEVICE_UART1,           //--10--
    OMAP_DEVICE_UART2,
    OMAP_DEVICE_I2C1,
    OMAP_DEVICE_I2C2,
    OMAP_DEVICE_I2C3,
    OMAP_DEVICE_MCSPI1,
    OMAP_DEVICE_MCSPI2,
    OMAP_DEVICE_MCSPI3,
    OMAP_DEVICE_MCSPI4,
    OMAP_DEVICE_HDQ, 
    OMAP_DEVICE_MSPRO,           //--20--     
    OMAP_DEVICE_MMC1, 
    OMAP_DEVICE_MMC2, 
    OMAP_DEVICE_MMC3,
    OMAP_DEVICE_DES2, 
    OMAP_DEVICE_SHA12, 
    OMAP_DEVICE_AES2, 
    OMAP_DEVICE_ICR, 
    OMAP_DEVICE_DES1,
    OMAP_DEVICE_SHA11, 
    OMAP_DEVICE_RNG,             //--30-- 
    OMAP_DEVICE_AES1, 
    OMAP_DEVICE_PKA,
    OMAP_DEVICE_USBTLL,
    OMAP_DEVICE_TS,
    OMAP_DEVICE_EFUSE,
    OMAP_DEVICE_GPTIMER1,        // WAKUP domain clocks
    OMAP_DEVICE_GPTIMER12,
    OMAP_DEVICE_32KSYNC,
    OMAP_DEVICE_WDT1,
    OMAP_DEVICE_WDT2,            //--40--
    OMAP_DEVICE_GPIO1,
    OMAP_DEVICE_SR1,
    OMAP_DEVICE_SR2,
    OMAP_DEVICE_USIM,
    OMAP_DEVICE_GPIO2,           // per domain clocks
    OMAP_DEVICE_GPIO3,
    OMAP_DEVICE_GPIO4,
    OMAP_DEVICE_GPIO5,
    OMAP_DEVICE_GPIO6, 
    OMAP_DEVICE_MCBSP2,          //--50--          
    OMAP_DEVICE_MCBSP3,
    OMAP_DEVICE_MCBSP4,
    OMAP_DEVICE_GPTIMER2,
    OMAP_DEVICE_GPTIMER3,
    OMAP_DEVICE_GPTIMER4,
    OMAP_DEVICE_GPTIMER5,
    OMAP_DEVICE_GPTIMER6,
    OMAP_DEVICE_GPTIMER7,
    OMAP_DEVICE_GPTIMER8,
    OMAP_DEVICE_GPTIMER9,        //--60--    
    OMAP_DEVICE_UART3,
    OMAP_DEVICE_WDT3,       
    OMAP_DEVICE_DSS,             // DSS domain clocks
    OMAP_DEVICE_DSS1,
    OMAP_DEVICE_DSS2,
    OMAP_DEVICE_TVOUT,
    OMAP_DEVICE_CAMERA,          // CAMERA domain clocks
    OMAP_DEVICE_CSI2,
    OMAP_DEVICE_DSP,
    OMAP_DEVICE_2D,              // --70-- GFX domain clocks
    OMAP_DEVICE_3D,
    OMAP_DEVICE_SGX,
	OMAP_DEVICE_HSUSB1,          // HSUSB 48mhz fclk
    OMAP_DEVICE_HSUSB2,          // HSUSB 120mhz fclk
    OMAP_DEVICE_USBHOST1,        // USBHOST ports...
    OMAP_DEVICE_USBHOST2,
    OMAP_DEVICE_USBHOST3,
    OMAP_DEVICE_VRFB,
    OMAP_DEVICE_UART4,          // 37xx only
    OMAP_DEVICE_GENERIC,        //--80--
    OMAP_DEVICE_COUNT,          // COUNT of core clocks      
    OMAP_DEVICE_SDMA,
    OMAP_DEVICE_GPMC,
    _OMAP_DEVICE_NONE = OMAP_DEVICE_NONE
} OMAP_DEVICE_ID;

//-----------------------------------------------------------------------------

typedef enum {
    POWERDOMAIN_WAKEUP = 0,
    POWERDOMAIN_CORE,
    POWERDOMAIN_PERIPHERAL,
    POWERDOMAIN_USBHOST,        
    POWERDOMAIN_EMULATION,    
    POWERDOMAIN_MPU,
    POWERDOMAIN_DSS,
    POWERDOMAIN_NEON,
    POWERDOMAIN_IVA2,    
    POWERDOMAIN_CAMERA,
    POWERDOMAIN_SGX,
    POWERDOMAIN_EFUSE,
    POWERDOMAIN_SMARTREFLEX,
    POWERDOMAIN_COUNT,
} PowerDomain_e;

typedef enum {
    CLOCKDOMAIN_L3 = 0,
    CLOCKDOMAIN_L4,
    CLOCKDOMAIN_PERIPHERAL,
    CLOCKDOMAIN_USBHOST,
    CLOCKDOMAIN_EMULATION,
    CLOCKDOMAIN_MPU,
    CLOCKDOMAIN_DSS,
    CLOCKDOMAIN_NEON,
    CLOCKDOMAIN_IVA2,
    CLOCKDOMAIN_CAMERA,
    CLOCKDOMAIN_SGX,
    CLOCKDOMAIN_D2D,
    CLOCKDOMAIN_COUNT,
    CLOCKDOMAIN_NULL = -1
} ClockDomain_e;

//-----------------------------------------------------------------------------

typedef enum {    
    kVDD1,
    kVDD2,
    kVDD3,
    kVDD4,
    kVDD5,
    kVDD_EXT,
    kVDDS,
    kVDDPLL,
    kVDDADAC,
    kMMC_VDDS,
    kVDD_COUNT,
} Vdd_e;

//------------------------------------------------------------------------------

typedef enum {    
    kDPLL1,
    kDPLL2,
    kDPLL3,
    kDPLL4,
    kDPLL5,
    kDPLL_EXT,
    kDPLL_COUNT,
} Dpll_e;

//------------------------------------------------------------------------------

typedef enum {
    kEXT_32KHZ,
    kDPLL1_CLKOUT_M2X2,
    kDPLL2_CLKOUT_M2,
    kDPLL3_CLKOUT_M2,
    kDPLL3_CLKOUT_M2X2,
    kDPLL3_CLKOUT_M3X2,
    kDPLL4_CLKOUT_M2X2,
    kDPLL4_CLKOUT_M3X2,
    kDPLL4_CLKOUT_M4X2,
    kDPLL4_CLKOUT_M5X2,
    kDPLL4_CLKOUT_M6X2,
    kDPLL5_CLKOUT_M2,
    kEXT_SYS_CLK,
    kEXT_ALT,
    kEXT_MISC,
    kINT_OSC,
    kDPLL_CLKOUT_COUNT,
} DpllClkOut_e;

//------------------------------------------------------------------------------

typedef enum {
    
//    clock name                   parent clock
//  ----------------------      ----------------------
    kDPLL1_M2X2_CLK,            // DPLL1_CLKOUT_M2X2
    kDPLL2_M2_CLK,              // DPLL2_CLKOUT_M2
    kCORE_CLK,                  // DPLL3_CLKOUT_M2
    kCOREX2_FCLK,               // DPLL3_CLKOUT_M2X2
    kEMUL_CORE_ALWON_CLK,       // DPLL3_CLKOUT_M3X2
    kPRM_96M_192M_ALWON_CLK,    // DPLL4_CLKOUT_M2X2
    kDPLL4_M3X2_CLK,            // DPLL4_CLKOUT_M3X2
    kDSS1_ALWON_FCLK,           // DPLL4_CLKOUT_M4X2
    kCAM_MCLK,                  // DPLL4_CLKOUT_M5X2
    kEMUL_PER_ALWON_CLK,        // DPLL4_CLKOUT_M6X2
    k120M_FCLK,                 // DPLL5_CLKOUT_M2
    k32K_FCLK,                  // EXT_32KHZ
    kSYS_CLK,                   // EXT_SYS_CLK
    kSYS_ALTCLK,                // EXT_ALT
    kSECURE_32K_FCLK,           // INT_OSC
    kMCBSP_CLKS,                // EXT_MISC
    kUSBTLL_SAR_FCLK,           // SYS_CLK
    kUSBHOST_SAR_FCLK,          // SYS_CLK
    kEFUSE_ALWON_FCLK,          // SYS_CLK
    kSR_ALWON_FCLK,             // SYS_CLK
    kDPLL1_ALWON_FCLK,          // SYS_CLK
    kDPLL2_ALWON_FCLK,          // SYS_CLK
    kDPLL3_ALWON_FCLK,          // SYS_CLK
    kDPLL4_ALWON_FCLK,          // SYS_CLK
    kDPLL5_ALWON_FCLK,          // SYS_CLK
    kCM_SYS_CLK,                // SYS_CLK
    kDSS2_ALWON_FCLK,           // SYS_CLK
    kWKUP_L4_ICLK,              // SYS_CLK
    kCM_32K_CLK,                // 32K_FCLK
    kCORE_32K_FCLK,             // CM_32K_CLK
    kWKUP_32K_FCLK,             // 32K_FCLK
    kPER_32K_ALWON_FCLK,        // 32K_FCLK    
    kCORE_120M_FCLK,            // 120M_FCLK
    kCM_96M_FCLK,               // PRM_96M_ALWON_CLK
    k96M_ALWON_FCLK,            // PRM_96M_ALWON_CLK    
    kL3_ICLK,                   // CORE_CLK
    kL4_ICLK,                   // L3_ICLK
    kUSB_L4_ICLK,               // L4_ICLK
    kRM_ICLK,                   // L4_ICLK
    kDPLL1_FCLK,                // CORE_CLK
    kDPLL2_FCLK,                // CORE_CLK
    kCORE_L3_ICLK,              // L3_ICLK
    kCORE_L4_ICLK,              // L4_ICLK
    kSECURITY_L3_ICLK,          // L3_ICLK
    kSECURITY_L4_ICLK1,         // L4_ICLK
    kSECURITY_L4_ICLK2,         // L4_ICLK
    kSGX_L3_ICLK,               // L3_ICLK
    kSSI_L4_ICLK,               // L4_ICLK
    kDSS_L3_ICLK,               // L3_ICLK
    kDSS_L4_ICLK,               // L4_ICLK
    kCAM_L3_ICLK,               // L3_ICLK
    kCAM_L4_ICLK,               // L4_ICLK
    kUSBHOST_L3_ICLK,           // L3_ICLK
    kUSBHOST_L4_ICLK,           // L4_ICLK
    kPER_L4_ICLK,               // L4_ICLK
    kSR_L4_ICLK,                // L4_ICLK
    kSSI_SSR_FCLK,              // COREX2_FCLK
    kSSI_SST_FCLK,              // COREX2_FCLK
    kCORE_96M_FCLK,             // 96M_FCLK
    kDSS_96M_FCLK,              // 96M_FCLK
    kCSI2_96M_FCLK,             // 96M_FCLK
    kCORE_48M_FCLK,             // 48M_FCLK
    kUSBHOST_48M_FCLK,          // 48M_FCLK
    kPER_48M_FCLK,              // 48M_FCLK
    k12M_FCLK,                  // 48M_FCLK
    kCORE_12M_FCLK,             // 12M_FCLK
    kDSS_TV_FCLK,               // 54M_FCLK
    kUSBHOST_120M_FCLK,         // 120M_FCLK
    kCM_USIM_CLK,               // DPLL5_M2_CLK     | CM_96M_FCLK
    kUSIM_FCLK,                 // SYS_CLK          | CM_USIM_CLK
    k96M_FCLK,                  // CM_96M_FCLK      | CM_SYS_CLK
    k48M_FCLK,                  // SYS_ALTCLK       | CM_96M_FCLK
    k54M_FCLK,                  // SYS_ALTCLK       | DPLL4_M3X2_CLK
    kSGX_FCLK,                  // CORE_CLK         | CM_96M_FCLK       | PRM_96M_192M_ALWON_CLK   | COREX2_FCLK
    kGPT1_FCLK,                 // SYS_CLK          | 32K_FCLK
    kGPT2_ALWON_FCLK,           // SYS_CLK          | 32K_FCLK
    kGPT3_ALWON_FCLK,           // SYS_CLK          | 32K_FCLK
    kGPT4_ALWON_FCLK,           // SYS_CLK          | 32K_FCLK
    kGPT5_ALWON_FCLK,           // SYS_CLK          | 32K_FCLK
    kGPT6_ALWON_FCLK,           // SYS_CLK          | 32K_FCLK
    kGPT7_ALWON_FCLK,           // SYS_CLK          | 32K_FCLK
    kGPT8_ALWON_FCLK,           // SYS_CLK          | 32K_FCLK
    kGPT9_ALWON_FCLK,           // SYS_CLK          | 32K_FCLK
    kGPT10_FCLK,                // CM_SYS_CLK       | CM_32K_CLK
    kGPT11_FCLK,                // CM_SYS_CLK       | CM_32K_CLK
    kMCBSP1_CLKS,               // kCORE_96M_FCLK   | kMCBSP_CLKS
    kMCBSP2_CLKS,               // k96M_ALWON_FCLK  | kMCBSP_CLKS
    kMCBSP3_CLKS,               // k96M_ALWON_FCLK  | kMCBSP_CLKS
    kMCBSP4_CLKS,               // k96M_ALWON_FCLK  | kMCBSP_CLKS
    kMCBSP5_CLKS,               // kCORE_96M_FCLK   | kMCBSP_CLKS
    
// end of clock definitions
    kSOURCE_CLOCK_COUNT,       
} SourceClock_e;
typedef SourceClock_e OMAP_CLOCK_ID;

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
