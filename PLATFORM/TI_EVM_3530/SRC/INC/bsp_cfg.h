// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.
// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
//=============================================================================
//            Texas Instruments OMAP(TM) Platform Software
// (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
//
//  Use of this software is controlled by the terms and conditions found
// in the license agreement under which this software has been supplied.
//
//=============================================================================
//

//------------------------------------------------------------------------------
//
//  File:  bsp_cfg.h
//
#ifndef __BSP_CFG_H
#define __BSP_CFG_H

//------------------------------------------------------------------------------
//
//  Select initial XLDR CPU and IVA speed and VDD1 voltage using BSP_OPM_SELECT 
//
#if (BSP_OPM_SELECT == A) || (BSP_CDS == 1)
    #define BSP_SPEED_CPUMHZ_1000			1000
    #define BSP_SPEED_CPUMHZ_800			800
    #define BSP_SPEED_CPUMHZ_600			600
    #define BSP_SPEED_IVAMHZ_874			874
    #define BSP_SPEED_IVAMHZ_800			800
    #define BSP_SPEED_IVAMHZ_660			660
    #define BSP_SPEED_IVAMHZ_520			520
    #define VDD1_INIT_VOLTAGE_VALUE4		0x39
    #define VDD1_INIT_VOLTAGE_VALUE3		0x35
    #define VDD1_INIT_VOLTAGE_VALUE2		0x28
#elif (BSP_OPM_SELECT == 4)
    // MPU[1000Mhz @ 1.3125V], IVA2[875Mhz @ 1.3125V]
    #define BSP_SPEED_CPUMHZ                1000
    #define BSP_SPEED_IVAMHZ                874
    #define VDD1_INIT_VOLTAGE_VALUE         0x39
#elif (BSP_OPM_SELECT == 3)
    // MPU[800Mhz  @ 1.2625V], IVA2[660Mhz @ 1.2625V]
    #define BSP_SPEED_CPUMHZ                800
    #define BSP_SPEED_IVAMHZ                660
    #define VDD1_INIT_VOLTAGE_VALUE         0x35
#elif (BSP_OPM_SELECT == 2)
    // MPU[600Mhz  @ 1.1000V], IVA2[520Mhz @ 1.1000V]
    #define BSP_SPEED_CPUMHZ                600
    #define BSP_SPEED_IVAMHZ                520
    #define VDD1_INIT_VOLTAGE_VALUE         0x28
#elif (BSP_OPM_SELECT == 1)
    // MPU[300Mhz  @ 0.9375V], IVA2[260Mhz @ 0.9375V]
    #define BSP_SPEED_CPUMHZ                300
    #define BSP_SPEED_IVAMHZ                260
    #define VDD1_INIT_VOLTAGE_VALUE         0x1b
#else
    #error Unsupported value for BSP_OPM_SELECT
#endif

// CORE 1.1375V
#define VDD2_INIT_VOLTAGE_VALUE             0x2b

//------------------------------------------------------------------------------
//
//  Define:  BSP_DEVICE_PREFIX
//
//  This define is used as device name prefix when KITL creates device name.
//
#define BSP_DEVICE_PREFIX       "EVM3730-"

//-----------------------------------------------------------------------------
// UNDONE:
//  currently used for OFF mode validation.  need to eventually settle
// on off mode values and delete these constants
//
// Note: These constants are not currently used by the BSP
#define PRM_VDD1_SETUP_TIME             (0x32 << 16)
#define PRM_VDD2_SETUP_TIME             (0x32)
#define PRM_OFFMODE_SETUP_TIME          (0x0)
#define PRM_COREOFF_OFFSET_TIME         (0x41)
#define PRM_CLK_SETUP_TIME              (0x0000)
#define PRM_AUTO_OFF_ENABLED            (0x1 << 2)
#define PRM_SYS_OFF_SIGNAL_ENABLED      (0x1 << 3)

//------------------------------------------------------------------------------
//
//  Define: BSP_PRM_CLKSETUP 
//
//  Determines the latency for src clk stabilization.  
//  Used to update PRM_CLKSETUP 
//
//  Allowed values:
//
//      [0-0xFFFF]  - in 32khz tick value  
//
#define BSP_PRM_CLKSETUP                (4)     // ~122 usec
#define BSP_PRM_CLKSETUP_OFFMODE        (0xA0) // 5 ms oscillator startup

//------------------------------------------------------------------------------
//
//  Define: BSP_PRM_VOLTSETUP1_RET 
//
//  Determines the latency for VDD1 & VDD2 stabilization.  
//  Used to update PRM_VOLTSETUP1 
//
//  Allowed values:
//
#define BSP_VOLTSETUP1_VDD2_INIT        (0x0112 << 16)  // 1 ms ramp time 
#define BSP_VOLTSETUP1_VDD1_INIT        (0x0112 << 0)   // 1 ms ramp time
#define BSP_PRM_VOLTSETUP1_INIT         (BSP_VOLTSETUP1_VDD2_INIT | \
                                         BSP_VOLTSETUP1_VDD1_INIT)

// This is used when I2C is used instead of SYS_OFFMODE pin
#define BSP_VOLTSETUP1_VDD2_OFF_MODE    (0xB3 << 0)   // 55 usec ramp time
#define BSP_VOLTSETUP1_VDD1_OFF_MODE    (0xA0 << 16)  // 49 usec ramp time
#define BSP_PRM_VOLTSETUP1_OFF_MODE     (BSP_VOLTSETUP1_VDD2_OFF_MODE | \
                                         BSP_VOLTSETUP1_VDD1_OFF_MODE)


//------------------------------------------------------------------------------
//
//  Define: BSP_PRM_VOLTSETUP2 
//
//  Determines the latency for VDD1 & VDD2 stabilization.  
//  Used to update PRM_VOLTSETUP1 
//
//  Allowed values:
//
#define BSP_PRM_VOLTSETUP2              (0x0)   // just use PRM_VOLTSETUP1
                                                // for voltage stabilization
                                                // latency

//------------------------------------------------------------------------------
//
//  Define: BSP_PRM_VOLTOFFSET 
//
//  Determines the latency of sys_offmode signal upon wake-up from OFF mode
//  when the OFF sequence is supervised by the Power IC
//
//  Allowed values:
//
#define BSP_PRM_VOLTOFFSET              (0x0)   // just use PRM_VOLTSETUP1
                                                // for voltage stabilization
                                                // latency

//------------------------------------------------------------------------------
//
//  Define: BSP_PRM_CLKSEL 
//
//  Determines the system clock frequency.  Used to update PRM_CLKSEL
//
//  Allowed values:
//
//      0x0: Input clock is 12 MHz
//      0x1: Input clock is 13 MHz
//      0x2: Input clock is 19.2 MHz
//      0x3: Input clock is 26 MHz
//      0x4: Input clock is 38.4 MHz
//
#define BSP_PRM_CLKSEL                  (3)


//------------------------------------------------------------------------------    
//
//  Define: BSP_CM_CLKSEL_CORE 
//
//  Determines CORE clock selection and dividers.  Used to update CM_CLKSEL_CORE
//
//  Allowed values:
//
#define BSP_CLKSEL_L3           (2 << 0)    // L3 = CORE_CLK/2
#define BSP_CLKSEL_L4           (2 << 2)    // L4 = L3 / 2
#define BSP_CLKSEL_GPT10        (0 << 6)    // GPT10 clk src = 32khz
#define BSP_CLKSEL_GPT11		(0 << 7)    // GPT11 clk src = 32khz
#define BSP_CLKSEL_SSI          (3 << 8)    // SSI fclk src = COREX2_CLK / 3
#define BSP_CLKSEL_96M			(1 << 12)   // 96MHz clock is DPLL2_M2_CLK divided by 1
	
#define BSP_CM_CLKSEL_CORE		(BSP_CLKSEL_96M | BSP_CLKSEL_L3 | BSP_CLKSEL_L4 | BSP_CLKSEL_GPT10 | BSP_CLKSEL_GPT11 | BSP_CLKSEL_SSI)


//------------------------------------------------------------------------------
//
//  Define: BSP_CM_CLKEN_PLL
//
//  Determines the DPLL3 and DPLL4 internal frequency based on DPLL reference 
//  clock and divider (n).  Used to update CM_CLKEN_PLL
//
//  Allowed values:
//
//      0x3: 0.75 MHz to 1.0 MHz
//      0x4: 1.0 MHz to 1.25 MHz
//      0x5: 1.25 MHz to 1.5 MHz
//      0x6: 1.5 MHz to 1.75 MHz
//      0x7: 1.75 MHz to 2.1 MHz
//      0xB: 7.5 MHz to 10 MHz
//      0xC: 10 MHz to 12.5 MHz
//      0xD: 12.5 MHz to 15 MHz
//      0xE: 15 MHz to 17.5 MHz
//      0xF: 17.5 MHz to 21 MHz
//
#define BSP_PWRDN_EMU_PERIPH            (0 << 31)   // enable DPLL4_M6X2
#define BSP_PWRDN_CAM                   (0 << 30)   // enable DPLL4_M5X2
#define BSP_PWRDN_DSS1                  (0 << 29)   // enable DPLL4_M4X2
#define BSP_PWRDN_TV                    (0 << 28)   // enable DPLL4_M3X2
#define BSP_PWRDN_96M                   (0 << 27)   // enable DPLL4_M2X2
#define BSP_EN_PERIPH_DPLL_LPMODE       (0 << 26)   // disable DPLL4 LP mode
#define BSP_PERIPH_DPLL_RAMPTIME    (0 << 24)   // reserved
#define BSP_PERIPH_DPLL_FREQSEL     (1 << 20)   // reserved, must be 1
	
#define BSP_EN_PERIPH_DPLL_DRIFTGUARD   (1 << 19)   // enable DPLL4 driftguard
#define BSP_EN_PERIPH_DPLL              (7 << 16)   // lock DPLL4
#define BSP_PWRDN_EMU_CORE              (0 << 12)   // enable DPLL3_M3X2 
#define BSP_EN_CORE_DPLL_LPMODE         (0 << 10)   // disable DPLL3 LP mode
#define BSP_CORE_DPLL_RAMPTIME          (0 << 8)    // disable ramp time
#define BSP_EN_CORE_DPLL_DRIFTGUARD     (1 << 3)    // enable DPLL3 driftguard
#define BSP_EN_CORE_DPLL                (7 << 0)    // lock DPLL3

#define BSP_CORE_DPLL_FREQSEL       (1 << 4)    // reserved, must be 1
	
#define BSP_CM_CLKEN_PLL                (BSP_PWRDN_EMU_PERIPH |         \
                                         BSP_PWRDN_CAM |                \
                                         BSP_PWRDN_DSS1 |               \
                                         BSP_PWRDN_TV |                 \
                                         BSP_PWRDN_96M |                \
                                         BSP_EN_PERIPH_DPLL_LPMODE |    \
                                         BSP_PERIPH_DPLL_RAMPTIME |     \
                                         BSP_PERIPH_DPLL_FREQSEL |      \
                                         BSP_EN_PERIPH_DPLL_DRIFTGUARD |\
                                         BSP_EN_PERIPH_DPLL |           \
                                         BSP_PWRDN_EMU_CORE |           \
                                         BSP_EN_CORE_DPLL_LPMODE |      \
                                         BSP_CORE_DPLL_RAMPTIME |       \
                                         BSP_CORE_DPLL_FREQSEL |        \
                                         BSP_EN_CORE_DPLL_DRIFTGUARD |  \
                                         BSP_EN_CORE_DPLL)


//------------------------------------------------------------------------------
//
//  Define: BSP_CM_CLKEN2_PLL
//
//  Determines the DPLL5 internal frequency based on DPLL reference 
//  clock and divider (n).  Used to update CM_CLKEN2_PLL
//
//  Allowed values:
//
//      0x3: 0.75 MHz to 1.0 MHz
//      0x4: 1.0 MHz to 1.25 MHz
//      0x5: 1.25 MHz to 1.5 MHz
//      0x6: 1.5 MHz to 1.75 MHz
//      0x7: 1.75 MHz to 2.1 MHz
//      0xB: 7.5 MHz to 10 MHz
//      0xC: 10 MHz to 12.5 MHz
//      0xD: 12.5 MHz to 15 MHz
//      0xE: 15 MHz to 17.5 MHz
//      0xF: 17.5 MHz to 21 MHz
//
#define BSP_EN_PERIPH2_DPLL_LPMODE      (0 << 10)   // disable DPLL5 LP mode
#define BSP_PERIPH2_DPLL_RAMPTIME       (0 << 8)    // disable ramp time
#define BSP_PERIPH2_DPLL_FREQSEL		(1 << 4)    // reserved, must be 1
#define BSP_EN_PERIPH2_DPLL_DRIFTGUARD  (1 << 3)    // enable DPLL4 driftguard
#define BSP_EN_PERIPH2_DPLL             (7 << 0)    // lock DPLL5


#define BSP_CM_CLKEN2_PLL               (BSP_EN_PERIPH2_DPLL_LPMODE |      \
                                         BSP_PERIPH2_DPLL_RAMPTIME |       \
                                         BSP_PERIPH2_DPLL_FREQSEL |        \
                                         BSP_EN_PERIPH2_DPLL_DRIFTGUARD |  \
                                         BSP_EN_PERIPH2_DPLL)


//------------------------------------------------------------------------------
//
//  Define: BSP_CM_CLKSEL1_PLL
//
//  Determines master clock frequency.  Used to update CM_CLKSEL1_PLL
//
//  Allowed values:
//
#define BSP_CORE_DPLL_CLKOUT_DIV       (1 << 27)    // DPLL3 output is CORE_CLK/1
#define BSP_SOURCE_54M                 (0 << 5)     // 54Mhz clk src = DPLL4
#define BSP_SOURCE_48M                 (0 << 3)     // 48Mhz clk src = DPLL4

// Set Core DPLL based on attached DDR memory specification
// NOTE - Be sure to set BSP_CORE_DPLL_FREQSEL correctly based on the divider value
#define CORE_DPLL_MULT_260              (130 << 16)
#define CORE_DPLL_DIV_260               (12 << 8)
#define CORE_DPLL_MULT_330              (166 << 16)
#define CORE_DPLL_DIV_330               (12 << 8)

#define BSP_CORE_DPLL_MULT              CORE_DPLL_MULT_330  // Multiplier
#define BSP_CORE_DPLL_DIV               CORE_DPLL_DIV_330    // Divider 
#define BSP_SOURCE_96M                  (0 << 6)     // 96Mhz clk src=CM_96M_FCLK

#define BSP_CM_CLKSEL1_PLL             (BSP_CORE_DPLL_CLKOUT_DIV | \
                                        BSP_CORE_DPLL_MULT | \
                                        BSP_CORE_DPLL_DIV | \
                                        BSP_SOURCE_54M | \
                                        BSP_SOURCE_48M | \
                                        BSP_SOURCE_96M)


//------------------------------------------------------------------------------
//
//  Define: BSP_CM_CLKSEL2_PLL
//
//  Determines PER clock settings.  Used to update CM_CLKSEL2_PLL (DPLL4)
//
//  Allowed values:
//
#define BSP_PERIPH_DPLL_MULT    (432 << 8)    // freq = 864MHz
#define BSP_PERIPH_DPLL_DIV		(12 << 0)     // 
#define BSP_CM_CLKSEL2_PLL      (BSP_PERIPH_DPLL_MULT | BSP_PERIPH_DPLL_DIV)


//------------------------------------------------------------------------------
//
//  Define: BSP_CM_CLKSEL3_PLL
//
//  Determines divisor from DPLL4 for 96M.  Used to update CM_CLKSEL3_PLL
//
//  Allowed values:
//
// Note that 96MHz clock comes from the M2X2 port
#define BSP_DIV_96M                    (9 << 0)     // DPLL4 864MHz/96MHz = 9

#define BSP_CM_CLKSEL3_PLL             (BSP_DIV_96M)



//------------------------------------------------------------------------------
//
//  Define: BSP_CM_CLKSEL4_PLL
//
//  Determines master clock frequency.  Used to update CM_CLKSEL4_PLL
//
//  Allowed values:
//
#define BSP_PERIPH2_DPLL_MULT          (443 << 8)    // Multiplier
#define BSP_PERIPH2_DPLL_DIV           (11 << 0)    // Divider

#define BSP_CM_CLKSEL4_PLL             (BSP_PERIPH2_DPLL_MULT | \
                                        BSP_PERIPH2_DPLL_DIV)


//------------------------------------------------------------------------------
//
//  Define: BSP_CM_CLKSEL5_PLL
//
//  Determines divisor from DPLL5 for 120M.  Used to update CM_CLKSEL5_PLL
//
//  Allowed values:
//
#define BSP_DIV_120M                   (8 << 0)     // DPLL5/1 = 120Mhz

#define BSP_CM_CLKSEL5_PLL             (BSP_DIV_120M)


//------------------------------------------------------------------------------
//
//  Define: BSP_CM_CLKSEL_CAM
//
//  Determines CAM clock settings.  Used to update CM_CLKSEL_CAM
//
//  Allowed values:
//
#define BSP_CAM_CLKSEL_CAM             (4 << 0)     // DPLL4/4=216mhz

#define BSP_CM_CLKSEL_CAM              (BSP_CAM_CLKSEL_CAM)


//------------------------------------------------------------------------------
//
//  Define: BSP_CM_CLKSEL_DSS
//
//  Determines EMU clock settings.  Used to update CM_CLKSEL_DSS
//
//  Allowed values: BSP_DSS_CLKSEL_DSS1 minimum value is 5 (173 MHz) at nominal voltage
//                  BSP_DSS_CLKSEL_DSS1 minimum value is 9 (96 MHz) at low voltage
//                  BSP_DSS_CLKSEL_TV should be fixed at 16 (54 MHz)
//
//  Note: DPLL4 is 864Mhz
#define BSP_DSS_CLKSEL_TV              (16 << 8)    // DPLL4/16=54Mhz

// select DSS1 clock divider based on display mode
#define BSP_DSS_CLKSEL_DSS1        (11 << 0)    // DSS1 = DPLL4/11 = 78.(54) Mhz
#define BSP_CM_CLKSEL_DSS              (BSP_DSS_CLKSEL_TV | \
                                        BSP_DSS_CLKSEL_DSS1)


//------------------------------------------------------------------------------
//
//  Define: BSP_CM_CLKSEL_SGX
//
//  Determines SGX clock settings.  Used to update CM_CLKSEL_DSS
//
//  Allowed values:
//            0x0: SGX functional clock is CORE_CLK divided by 3
//            0x1: SGX functional clock is CORE_CLK divided by 4
//            0x2: SGX functional clock is CORE_CLK divided by 6
//            0x3: SGX functional clock is CM_96M_FCLK clock
//            0x4: SGX functional clock is SGX_192M_FCLK clock
//            0x5: SGX functional clock is CORE_CLK divided by 2
//            0x6: SGX functional clock is COREX2_CLK divided by 3
//            0x7: SGX functional clock is COREX2_CLK divided by 5
//
#define BSP_SGX_CLKSEL_SGX             (5 << 0)    // CORE_CLK/2=200mhz

#define BSP_CM_CLKSEL_SGX              (BSP_SGX_CLKSEL_SGX)


//------------------------------------------------------------------------------
//
//  Define: BSP_CM_CLKSEL1_EMU
//
//  Determines EMU clock settings.  Used to update CM_CLKSEL1_EMU
//
//  Allowed values:
//
#define BSP_EMU_DIV_DPLL4              (3 << 24)    // DPLL4/3=288mhz
#define BSP_EMU_DIV_DPLL3              (2 << 16)    // DPLL3/2          
#define BSP_EMU_CLKSEL_TRACECLK        (1 << 11)    // TRACECLK/1       (default)
#define BSP_EMU_CLKSEL_PCLK            (2 << 8)     // PCLK.FCLK/2      (default)
#define BSP_EMU_PCLKX2                 (1 << 6)     // PCLKx2.FCLK/1    (default)
#define BSP_EMU_CLKSEL_ATCLK           (1 << 4)     // ATCLK.FCLK/1     (default)
#define BSP_EMU_TRACE_MUX_CTRL         (0 << 2)     // TRACE src=sysclk (default)
#define BSP_EMU_MUX_CTRL               (0 << 0)     // ATCLK.PCLK=sysclk(default)

#define BSP_CM_CLKSEL1_EMU             (BSP_EMU_DIV_DPLL4 | \
                                        BSP_EMU_DIV_DPLL3 | \
                                        BSP_EMU_CLKSEL_TRACECLK | \
                                        BSP_EMU_CLKSEL_PCLK | \
                                        BSP_EMU_PCLKX2 | \
                                        BSP_EMU_CLKSEL_ATCLK | \
                                        BSP_EMU_TRACE_MUX_CTRL | \
                                        BSP_EMU_MUX_CTRL)


//------------------------------------------------------------------------------
//
//  Define: BSP_MPU_DPLL_FREQSEL
//
//  Determines the DPLL1 internal frequency based on DPLL reference clock 
//  and divider (n).  Used to update CM_CLKEN_PLL_MPU
//
//  Allowed values:
//
//      0x3: 0.75 MHz to 1.0 MHz
//      0x4: 1.0 MHz to 1.25 MHz
//      0x5: 1.25 MHz to 1.5 MHz
//      0x6: 1.5 MHz to 1.75 MHz
//      0x7: 1.75 MHz to 2.1 MHz
//      0xB: 7.5 MHz to 10 MHz
//      0xC: 10 MHz to 12.5 MHz
//      0xD: 12.5 MHz to 15 MHz
//      0xE: 15 MHz to 17.5 MHz
//      0xF: 17.5 MHz to 21 MHz
//
#define BSP_EN_MPU_DPLL_LPMODE      (0 << 10)   // disable DPLL1 LP mode
#define BSP_MPU_DPLL_RAMPTIME       (2 << 8)    // ramp time = 20us
#define BSP_EN_MPU_DPLL_DRIFTGUARD  (1 << 3)    // enable DPLL1 driftguard
#define BSP_EN_MPU_DPLL             (7 << 0)    // lock DPLL1
#define MPU_DPLL_FREQSEL_500		(7 << 4)
#define BSP_MPU_DPLL_FREQSEL		(1 << 4)    // reserved, must be 1
	
#define BSP_CM_CLKEN_PLL_MPU           (BSP_EN_MPU_DPLL_LPMODE |      \
                                        BSP_MPU_DPLL_RAMPTIME |       \
                                        BSP_MPU_DPLL_FREQSEL |        \
                                        BSP_EN_MPU_DPLL_DRIFTGUARD |  \
                                        BSP_EN_MPU_DPLL)


//------------------------------------------------------------------------------
//
//  Define: BSP_CM_CLKSEL1_PLL_MPU
//
//  Determines master clock frequency.  Used to update CM_CLKSEL1_PLL_MPU
//
//  Allowed values:
//
#define BSP_MPU_CLK_SRC                (2 << 19)    // DPLL1 bypass = CORE.CLK/2
// Set desired MPU frequency
//  #define MPU_DPLL_MULT_500           (250 << 8)
//  #define MPU_DPLL_DIV_500            (12 << 0)
//#define BSP_MPU_DPLL_MULT           MPU_DPLL_MULT_500   // Multiplier
//#define BSP_MPU_DPLL_DIV            MPU_DPLL_DIV_500    // Divider

// BSP_SPEED_CPUMHZ is set by .BAT file, default value is 500
#if (BSP_OPM_SELECT == A) || (BSP_CDS == 1)
	#define BSP_MPU_DPLL_MULT_1000		((BSP_SPEED_CPUMHZ_1000 / 2) << 8)   // Multiplier
	#define BSP_MPU_DPLL_MULT_800		((BSP_SPEED_CPUMHZ_800  / 2) << 8)   // Multiplier
	#define BSP_MPU_DPLL_MULT_600		((BSP_SPEED_CPUMHZ_600  / 2) << 8)   // Multiplier
#else
#define BSP_MPU_DPLL_MULT           ((BSP_SPEED_CPUMHZ / 2) << 8)   // Multiplier
#endif
#define BSP_MPU_DPLL_DIV            (12 << 0)                       // Divider

#if (BSP_OPM_SELECT == A) || (BSP_CDS == 1)
	#define BSP_CM_CLKSEL1_PLL_MPU_1000	(BSP_MPU_CLK_SRC | BSP_MPU_DPLL_MULT_1000 | BSP_MPU_DPLL_DIV)
	#define BSP_CM_CLKSEL1_PLL_MPU_800	(BSP_MPU_CLK_SRC | BSP_MPU_DPLL_MULT_800  | BSP_MPU_DPLL_DIV)
	#define BSP_CM_CLKSEL1_PLL_MPU_600	(BSP_MPU_CLK_SRC | BSP_MPU_DPLL_MULT_600  | BSP_MPU_DPLL_DIV)
#else
	#define BSP_CM_CLKSEL1_PLL_MPU      (BSP_MPU_CLK_SRC |   \
                                        BSP_MPU_DPLL_MULT | \
                                        BSP_MPU_DPLL_DIV)
#endif
//------------------------------------------------------------------------------
//
//  Define: BSP_CM_CLKSEL2_PLL_MPU
//
//  Determines the output clock divider for DPLL1.  Used to update 
//  CM_CLKSEL2_PLL_MPU
//
//  Allowed values:
//
//      0x1: DPLL1 output clock is divided by 1
//      0x2: DPLL1 output clock is divided by 2
//      0x3: DPLL1 output clock is divided by 3
//      0x4: DPLL1 output clock is divided by 4
//      0x5: DPLL1 output clock is divided by 5
//      0x6: DPLL1 output clock is divided by 6
//      0x7: DPLL1 output clock is divided by 7
//      0x8: DPLL1 output clock is divided by 8
//      0x9: DPLL1 output clock is divided by 9
//      0xA: DPLL1 output clock is divided by 10
//      0xB: DPLL1 output clock is divided by 11
//      0xC: DPLL1 output clock is divided by 12
//      0xD: DPLL1 output clock is divided by 13
//      0xE: DPLL1 output clock is divided by 14
//      0xF: DPLL1 output clock is divided by 15
//      0x10: DPLL1 output clock is divided by 16
//
#define BSP_MPU_DPLL_CLKOUT_DIV         (1 << 0)    // CLKOUTX2 = DPLL1 freq

#define BSP_CM_CLKSEL2_PLL_MPU          (BSP_MPU_DPLL_CLKOUT_DIV)


//------------------------------------------------------------------------------
//
//  Define: BSP_IVA2_DPLL_FREQSEL
//
//  Determines the DPLL1 internal frequency based on DPLL reference clock 
//  and divider (n).  Used to update CM_CLKEN_PLL_IVA2
//
//  Allowed values:
//
//      0x3: 0.75 MHz to 1.0 MHz
//      0x4: 1.0 MHz to 1.25 MHz
//      0x5: 1.25 MHz to 1.5 MHz
//      0x6: 1.5 MHz to 1.75 MHz
//      0x7: 1.75 MHz to 2.1 MHz
//      0xB: 7.5 MHz to 10 MHz
//      0xC: 10 MHz to 12.5 MHz
//      0xD: 12.5 MHz to 15 MHz
//      0xE: 15 MHz to 17.5 MHz
//      0xF: 17.5 MHz to 21 MHz
//
#define BSP_EN_IVA2_DPLL_LPMODE         (0 << 10)   // disable DPLL2 LP mode
#define BSP_IVA2_DPLL_RAMPTIME      (0 << 8)    // ramp time = 0us
#define BSP_IVA2_DPLL_FREQSEL       (1 << 4)    // reserved, must be 1
#define BSP_EN_IVA2_DPLL_DRIFTGUARD     (1 << 3)    // enable DPLL1 driftguard
#define BSP_EN_IVA2_DPLL                (7 << 0)    // lock DPLL2

#define BSP_CM_CLKEN_PLL_IVA2           (BSP_EN_IVA2_DPLL_LPMODE |      \
                                         BSP_IVA2_DPLL_RAMPTIME |       \
                                         BSP_IVA2_DPLL_FREQSEL |        \
                                         BSP_EN_IVA2_DPLL_DRIFTGUARD |  \
                                         BSP_EN_IVA2_DPLL)
                                        

//------------------------------------------------------------------------------
//
//  Define: BSP_CM_CLKSEL1_PLL_IVA2
//
//  Determines master clock frequency.  Used to update CM_CLKSEL1_PLL_IVA2
//
//  Allowed values:
//
#define BSP_IVA2_CLK_SRC           (4 << 19)    // DPLL1 bypass = CORE.CLK/4
#if (BSP_OPM_SELECT == A) || (BSP_CDS == 1)
	#define BSP_IVA2_DPLL_MULT_874		((BSP_SPEED_IVAMHZ_874 / 2) << 8)   // Multiplier
	#define BSP_IVA2_DPLL_MULT_800		((BSP_SPEED_IVAMHZ_800 / 2) << 8)   // Multiplier
	#define BSP_IVA2_DPLL_MULT_660		((BSP_SPEED_IVAMHZ_660 / 2) << 8)   // Multiplier
	#define BSP_IVA2_DPLL_MULT_520		((BSP_SPEED_IVAMHZ_520 / 2) << 8)   // Multiplier
#else
	#define BSP_IVA2_DPLL_MULT             ((BSP_SPEED_IVAMHZ / 2) << 8)   // Multiplier
#endif
#define BSP_IVA2_DPLL_DIV              (12 << 0)    // Divider

#if (BSP_OPM_SELECT == A) || (BSP_CDS == 1)
	#define BSP_CM_CLKSEL1_PLL_IVA2_874	(BSP_IVA2_CLK_SRC | BSP_IVA2_DPLL_MULT_874 | BSP_IVA2_DPLL_DIV)
	#define BSP_CM_CLKSEL1_PLL_IVA2_800	(BSP_IVA2_CLK_SRC | BSP_IVA2_DPLL_MULT_800 | BSP_IVA2_DPLL_DIV)
	#define BSP_CM_CLKSEL1_PLL_IVA2_660	(BSP_IVA2_CLK_SRC | BSP_IVA2_DPLL_MULT_660 | BSP_IVA2_DPLL_DIV)
	#define BSP_CM_CLKSEL1_PLL_IVA2_520	(BSP_IVA2_CLK_SRC | BSP_IVA2_DPLL_MULT_520 | BSP_IVA2_DPLL_DIV)
#else
	#define BSP_CM_CLKSEL1_PLL_IVA2		(BSP_IVA2_CLK_SRC |      \
                                        BSP_IVA2_DPLL_MULT |    \
                                        BSP_IVA2_DPLL_DIV)
#endif

//------------------------------------------------------------------------------
//
//  Define: BSP_CM_CLKSEL2_PLL_IVA2
//
//  Determines the output clock divider for DPLL2.  Used to update 
//  CM_CLKSEL2_PLL_IVA2
//
//  Allowed values:
//
//      0x1: DPLL2 output clock is divided by 1
//      0x2: DPLL2 output clock is divided by 2
//      0x3: DPLL2 output clock is divided by 3
//      0x4: DPLL2 output clock is divided by 4
//      0x5: DPLL2 output clock is divided by 5
//      0x6: DPLL2 output clock is divided by 6
//      0x7: DPLL2 output clock is divided by 7
//      0x8: DPLL2 output clock is divided by 8
//      0x9: DPLL2 output clock is divided by 9
//      0xA: DPLL2 output clock is divided by 10
//      0xB: DPLL2 output clock is divided by 11
//      0xC: DPLL2 output clock is divided by 12
//      0xD: DPLL2 output clock is divided by 13
//      0xE: DPLL2 output clock is divided by 14
//      0xF: DPLL2 output clock is divided by 15
//      0x10: DPLL2 output clock is divided by 16
//
#define BSP_IVA2_DPLL_CLKOUT_DIV        (1 << 0)    // CLKOUTX2 = DPLL2 freq

#define BSP_CM_CLKSEL2_PLL_IVA2         (BSP_IVA2_DPLL_CLKOUT_DIV)


//------------------------------------------------------------------------------
//
//  Define: BSP_SDRC_MCFG_0
//
//  Determines memory configuration registers.  Used to update SDRC_MCFG_0
//
//  Allowed values:
//
#define BSP_RASWIDTH_0			(3 << 24)	// 14 rows
#define BSP_CASWIDTH_0			(5 << 20)	// 10 columns
#define BSP_ADDRMUXLEGACY_0		(1 << 19)   // flexible address mux
#define BSP_RAMSIZE_0			(128 << 8)  // 256 MB LPDDR
#define BSP_BANKALLOCATION_0	(2 << 6)	// bank-row-column
#define BSP_B32NOT16_0			(1 << 4)    // LPDDR is x32 bit.
#define BSP_DEEPPD_0			(1 << 3)    // support deep-power down
#define BSP_DDRTYPE_0			(0 << 2)    // Mobile DDR
#define BSP_RAMTYPE_0			(1 << 0)	// DDR

#define BSP_SDRC_MCFG_0			(BSP_RASWIDTH_0 | BSP_CASWIDTH_0 | \
								 BSP_ADDRMUXLEGACY_0 | BSP_RAMSIZE_0 | \
								 BSP_BANKALLOCATION_0 | BSP_B32NOT16_0 | \
								 BSP_DEEPPD_0 | BSP_DDRTYPE_0 | BSP_RAMTYPE_0)

//------------------------------------------------------------------------------
//
//  Define: BSP_SDRC_MCFG_1
//
//  Determines memory configuration registers.  Used to update SDRC_MCFG_1
//
//  Allowed values:
//
#define BSP_RASWIDTH_1			(3 << 24)	// 14 rows
#define BSP_CASWIDTH_1			(5 << 20)	// 10 columns
#define BSP_ADDRMUXLEGACY_1		(1 << 19)   // flexible address mux
#define BSP_RAMSIZE_1			(128 << 8)	// 256 MB LPDDR
#define BSP_BANKALLOCATION_1	(2 << 6)	// bank-row-column
#define BSP_B32NOT16_1			(1 << 4)    // LPDDR is x32 bit.
#define BSP_DEEPPD_1			(1 << 3)    // support deep-power down
#define BSP_DDRTYPE_1			(0 << 2)    // Mobile DDR
#define BSP_RAMTYPE_1			(1 << 0)    // DDR

#define BSP_SDRC_MCFG_1			(BSP_RASWIDTH_1 | BSP_CASWIDTH_1 | \
								 BSP_ADDRMUXLEGACY_1 | BSP_RAMSIZE_1 | \
								 BSP_BANKALLOCATION_1 | BSP_B32NOT16_1 | \
								 BSP_DEEPPD_1 | BSP_DDRTYPE_1 | BSP_RAMTYPE_1)

//------------------------------------------------------------------------------
//
//  Define: BSP_SDRC_SHARING
//
//  Determines the SDRC module attached memory size and position on the SDRC 
//  module I/Os..  Used to update SDRC_SHARING
//
//  Allowed values:
//
#define BSP_CS1MUXCFG                  (0 << 12)    // 32-bit SDRAM on [31:0]
#define BSP_CS0MUXCFG                  (0 << 9)     // 32-bit SDRAM on [31:0]
#define BSP_SDRCTRISTATE               (1 << 8)     // Normal mode

#define BSP_SDRC_SHARING               (BSP_CS1MUXCFG | \
                                        BSP_CS0MUXCFG | \
                                        BSP_SDRCTRISTATE)


//------------------------------------------------------------------------------
//
//  Define: BSP_SDRC_ACTIM_CTRLA_0
//
//  Determines ac timing control register A.  Used to update SDRC_ACTIM_CTRLA_0
//
//  Allowed values:
//
// NOTE - Settings below are based on CORE DPLL = 332MHz, L3 = CORE/2 (166MHz)

/* Micron version of EVM3530 [MT29C2G24MAKLAJG-6](166MHz optimized) ~ 6.0ns
 *
 * ACTIM_CTRLA -
 *  TWR = 15/6  = 3 (micron)
 *  TDAL = Twr/Tck + Trp/tck = 15/6 + 18/6 = 3 + 3 = 6  (micron)
 *  TRRD = 12/6 = 2
 *  TRCD = 18/6 = 3
 *  TRP = 18/6  = 3
 *  TRAS = 42/6 = 7
 *  TRC = 60/6  = 10
 *  TRFC = 125/6 = 21 (micron)
 *
 * ACTIM_CTRLB -
 *  TCKE            = 1 (micron)
 *  XSR = 138/6   = 23  (micron)
 */

// Choose more conservative of memory timings when they differ between vendors
#define BSP_TRFC_0		(13 << 27)   // Autorefresh to active		min 72 ns, 6*13 = 78
#define BSP_TRC_0		(10 << 22)   // Row cycle time				TRAS+TRP = 55 ns, 10*6 = 60
#define BSP_TRAS_0		(7 << 18)    // Row active time				min 40 ns, 6*7 = 42
#define BSP_TRP_0		(3 << 15)    // Row precharge time			min 15 ns, 3*6 = 18
#define BSP_TRCD_0		(3 << 12)    // Row to column delay time	min 15 ns, 3*6 = 18
#define BSP_TRRD_0		(2 << 9)     // Active to active cmd per	min 10 ns, 2*6 = 12
#define BSP_TWR_0		(3 << 6)     // Data-in to precharge cmd	min 15 ns, 3*6 = 18
#define BSP_TDAL_0		(6 << 0)     // Data-in to active command	(TWR+TRP)/TCK = (18+18)/6 = 6

#define BSP_SDRC_ACTIM_CTRLA_0	(BSP_TRFC_0 | BSP_TRC_0 |  BSP_TRAS_0 | BSP_TRP_0 |  \
								 BSP_TRCD_0 | BSP_TRRD_0 | BSP_TWR_0 | BSP_TDAL_0)

//------------------------------------------------------------------------------
//
//  Define: BSP_SDRC_ACTIM_CTRLA_1
//
//  Determines ac timing control register A.  Used to update SDRC_ACTIM_CTRLA_1
//
//  Allowed values:
//
#define BSP_SDRC_ACTIM_CTRLA_1		BSP_SDRC_ACTIM_CTRLA_0
#define BSP_SDRC_ACTIM_CTRLA_1		BSP_SDRC_ACTIM_CTRLA_0
#define BSP_SDRC_ACTIM_CTRLA_1		BSP_SDRC_ACTIM_CTRLA_0

//------------------------------------------------------------------------------
//
//  Define: BSP_SDRC_ACTIM_CTRLB_0
//
//  Determines ac timing control register B.  Used to update SDRC_ACTIM_CTRLB_0
//
//  Allowed values:
//
#define BSP_TWTR_0		(2 << 16)		// 2-cycle write to read delay			min 2 clk
#define BSP_TCKE_0		(1 << 12)		// CKE minimum pulse width				min 1 clk
#define BSP_TXP_0		(2 << 8)		// 2 minimum cycles
#define BSP_TXSR_0		(19 << 0)		// Self Refresh Exit to Active period	min 112.5 ns 19*6 = 114
#define BSP_SDRC_ACTIM_CTRLB_0	(BSP_TWTR_0 | BSP_TCKE_0 | BSP_TXP_0 | BSP_TXSR_0)

//------------------------------------------------------------------------------
//
//  Define: BSP_SDRC_ACTIM_CTRLB_1
//
//  Determines ac timing control register A.  Used to update SDRC_ACTIM_CTRLB_1
//
//  Allowed values:
//
#define BSP_SDRC_ACTIM_CTRLB_1          BSP_SDRC_ACTIM_CTRLB_0


//------------------------------------------------------------------------------
//
//  Define: BSP_SDRC_RFR_CTRL_0
//
//  SDRAM memory autorefresh control.  Used to update SDRC_RFR_CTRL_0
//
//  Allowed values:
//
#define BSP_ARCV                       (0x4E2)
#define BSP_ARCV_0                     (BSP_ARCV << 8)  // Autorefresh counter val
#define BSP_ARE_0                      (1 << 0)         // Autorefresh on counter x1

#define BSP_SDRC_RFR_CTRL_0            (BSP_ARCV_0 | BSP_ARE_0)

//------------------------------------------------------------------------------
//
//  Define: BSP_SDRC_RFR_CTRL_1
//
//  SDRAM memory autorefresh control.  Used to update SDRC_RFR_CTRL_1
//
//  Allowed values:
//
#define BSP_SDRC_RFR_CTRL_1             BSP_SDRC_RFR_CTRL_0


//------------------------------------------------------------------------------
//
//  Define: BSP_SDRC_MR_0
//
//  Corresponds to the JEDEC SDRAM MR register.  Used to update SDRC_MR_0
//
//  Allowed values:
//
#define BSP_CASL_0                     (3 << 4)    // CAS latency = 3
#define BSP_SIL_0                      (0 << 3)    // Serial mode
#define BSP_BL_0                       (2 << 0)    // Burst Length = 4(DDR only)

#define BSP_SDRC_MR_0                  (BSP_CASL_0 | BSP_SIL_0 | BSP_BL_0)


//------------------------------------------------------------------------------
//
//  Define: BSP_SDRC_MR_0
//
//  Corresponds to the JEDEC SDRAM MR register.  Used to update SDRC_MR_1
//
//  Allowed values:
//
#define BSP_SDRC_MR_1                  (BSP_SDRC_MR_0)


//------------------------------------------------------------------------------
//
//  Define: BSP_SDRC_EMR2_0
//
//  Corresponds to the low-power EMR register, as defined in the mobile DDR 
//  JEDEC standard.  Used to update SDRC_EMR2_0
//
//  Allowed values:
//
#define BSP_DS_0                       (0 << 5)    // Strong-strength driver
#define BSP_TCSR_0                     (3 << 3)    // 85 deg max temp
#define BSP_PASR_0                     (0 << 0)    // All banks

#define BSP_SDRC_EMR2_0                (BSP_DS_0 | BSP_TCSR_0 | BSP_PASR_0)


//------------------------------------------------------------------------------
//
//  Define: BSP_SDRC_EMR2_1
//
//  Corresponds to the low-power EMR register, as defined in the mobile DDR 
//  JEDEC standard.  Used to update SDRC_EMR2_1
//
//  Allowed values:
//
#define BSP_SDRC_EMR2_1                (BSP_SDRC_EMR2_0)


//------------------------------------------------------------------------------
//
//  Define: BSP_SDRC_DLLA_CTRL
//
//  Used to fine-tune DDR timings.  Used to update SDRC_DLLA_CTRL
//
//  Allowed values:
//
#define BSP_FIXEDELAY                  (38 << 24)
#define BSP_MODEFIXEDDELAYINITLAT      (0 << 16)
#define BSP_DLLMODEONIDLEREQ           (0 << 5)
#define BSP_ENADLL                     (1 << 3)     // enable DLLs
#define BSP_LOCKDLL                    (0 << 2)     // run in unlock mode
#define BSP_DLLPHASE                   (1 << 1)     // 72 deg phase
#define BSP_SDRC_DLLA_CTRL             (BSP_FIXEDELAY | \
                                        BSP_MODEFIXEDDELAYINITLAT | \
                                        BSP_DLLMODEONIDLEREQ | \
                                        BSP_ENADLL | \
                                        BSP_LOCKDLL | \
                                        BSP_DLLPHASE)

//------------------------------------------------------------------------------
//
//  Define: BSP_SDRC_POWER_REG
//
//  Set SDRAM power management mode.  Used to update SDRC_POWER_REG
//
//  Allowed values:
//
#define BSP_WAKEUPPROC                 (0 << 26)    // don't stall 500 cycles on 1st access
#define BSP_AUTOCOUNT                  (4370 << 8)  // SDRAM idle count down
//#define BSP_SRFRONRESET              (0 << 7)		// disable idle on reset
#define BSP_SRFRONRESET                (1 << 7)     // enable self refresh on warm reset
#define BSP_SRFRONIDLEREQ              (1 << 6)     // hw idle on request
#define BSP_CLKCTRL                    (2 << 4)     // self-refresh on auto_cnt
#define BSP_EXTCLKDIS                  (1 << 3)     // disable ext clock
#define BSP_PWDENA                     (1 << 2)     // active power-down mode 
#define BSP_PAGEPOLICY                 (1 << 0)     // must be 1
#define BSP_SDRC_POWER_REG             (BSP_WAKEUPPROC | \
                                        BSP_AUTOCOUNT | \
                                        BSP_SRFRONRESET | \
                                        BSP_SRFRONIDLEREQ | \
                                        BSP_CLKCTRL | \
                                        BSP_EXTCLKDIS | \
                                        BSP_PWDENA | \
                                        BSP_PAGEPOLICY)


//------------------------------------------------------------------------------
//
//  Define: BSP_SDRC_DLLB_CTRL
//
//  Used to fine-tune DDR timings.  Used to update SDRC_DLLB_CTRL
//
//  Allowed values:
//
#define BSP_SDRC_DLLB_CTRL             (BSP_SDRC_DLLA_CTRL)


//------------------------------------------------------------------------------
//
//  Define:  BSP_GPMC_xxx
//
//  These constants are used to initialize general purpose memory configuration 
//  registers
//
// NOTE - Settings below are based on CORE DPLL = 332MHz, L3 = CORE/2 (166MHz)

// ONE NAND settings, not optimized
#define BSP_GPMC_ONENAND_CONFIG1        0x00001200
#define BSP_GPMC_ONENAND_CONFIG2        0x000F0F01
#define BSP_GPMC_ONENAND_CONFIG3        0x00030301
#define BSP_GPMC_ONENAND_CONFIG4        0x0F040F04
#define BSP_GPMC_ONENAND_CONFIG5        0x010F1010
#define BSP_GPMC_ONENAND_CONFIG6        0x1F060000
#define BSP_GPMC_ONENAND_CONFIG7        0x00000C4C     // Base address 0x0C000000, 64 MB window

//  NAND settings, not optimized
#if 1
#define BSP_GPMC_NAND_CONFIG1       0x00001800		// 0x00601801		// 16 bit NAND interface GPMC clk = GPMC fclk/2, 83 MHz
#define BSP_GPMC_NAND_CONFIG2       0x00060800		// 0x00060800     // 0x00141400 
#define BSP_GPMC_NAND_CONFIG3       0x00060401      // 0x00141400 
#define BSP_GPMC_NAND_CONFIG4       0x05010801		// 0x04010601      // 0x0F010F01
#define BSP_GPMC_NAND_CONFIG5       0x01060909		// 0x04060808      // 0x010C1414 //page burst is 2 GPMC clk, rdc/wrc 8
#define BSP_GPMC_NAND_CONFIG6       0x050001C0		// 0x040001C0      // 0x00000A80
#define BSP_GPMC_NAND_CONFIG7       0x00000F48      // Base address 0x08000000, 16MB window
#else
#define BSP_GPMC_NAND_CONFIG1       0x00001800      // 16 bit NAND interface
#define BSP_GPMC_NAND_CONFIG2       0x00060600      // 0x00141400 
#define BSP_GPMC_NAND_CONFIG3       0x00060401      // 0x00141400 
#define BSP_GPMC_NAND_CONFIG4       0x05010801      // 0x0F010F01
#define BSP_GPMC_NAND_CONFIG5       0x00080909      // 0x010C1414
#define BSP_GPMC_NAND_CONFIG6       0x050001C0      // 0x00000A80
#define BSP_GPMC_NAND_CONFIG7       0x00000F48      // Base address 0x08000000, 16MB window
#endif

//------------------------------------------------------------------------------
//
//  Define:  BSP_UART_DSIUDLL & BSP_UART_DSIUDLH
//
//  This constants are used to initialize serial debugger output UART.
//  Serial debugger uses 115200-8-N-1
//
#define BSP_UART_LCR                   (0x03)
#define BSP_UART_DSIUDLL               (26)
#define BSP_UART_DSIUDLH               (0)



//------------------------------------------------------------------------------
//
//  Define:  CM_AUTOIDLE1_xxxx_INIT
//
//  initial autoidle settings for a given power domain
//
// Note: Some bits reserved for non-GP devices are being set
#define CM_AUTOIDLE1_CORE_INIT          (0x7FFFFED1)
// Note: This register is reserved for non-GP devices
#define CM_AUTOIDLE2_CORE_INIT          (0x0000001F)
#define CM_AUTOIDLE3_CORE_INIT          (0x00000004)
#define CM_AUTOIDLE_WKUP_INIT           (0x0000003F)
#define CM_AUTOIDLE_PER_INIT            (0x0003FFFF)
#define CM_AUTOIDLE_CAM_INIT            (0x00000001)
#define CM_AUTOIDLE_DSS_INIT            (0x00000001)
#define CM_AUTOIDLE_USBHOST_INIT        (0x00000001)

//------------------------------------------------------------------------------
//
//  Define:  CM_SLEEPDEP_xxxx_INIT
//
//  initial sleep dependency settings for a given power domain
//
#define CM_SLEEPDEP_SGX_INIT            (0x00000000)
#define CM_SLEEPDEP_DSS_INIT            (0x00000000)
#define CM_SLEEPDEP_CAM_INIT            (0x00000000)
#define CM_SLEEPDEP_PER_INIT            (0x00000000)
#define CM_SLEEPDEP_USBHOST_INIT        (0x00000000)

//------------------------------------------------------------------------------
//
//  Define:  CM_WKDEP_xxxx_INIT
//
//  initial wake dependency settings for a given power domain
//
#define CM_WKDEP_IVA2_INIT              (0x00000000)
#define CM_WKDEP_MPU_INIT               (0x00000000)
#define CM_WKDEP_NEON_INIT              (0x00000002)
#define CM_WKDEP_SGX_INIT               (0x00000000)
#define CM_WKDEP_DSS_INIT               (0x00000000)
#define CM_WKDEP_CAM_INIT               (0x00000000)
#define CM_WKDEP_PER_INIT               (0x00000000)
#define CM_WKDEP_USBHOST_INIT           (0x00000000)

//------------------------------------------------------------------------------
//
//  Define:  CM_WKEN_xxxx_INIT
//
//  initial wake enable settings for a given power domain
//
#define CM_WKEN_WKUP_INIT               (0x00000100)
#define CM_WKEN1_CORE_INIT              (0x00000000)
#define CM_WKEN_DSS_INIT                (0x00000000)
#define CM_WKEN_PER_INIT                (0x00000000)
#define CM_WKEN_USBHOST_INIT            (0x00000000)

//------------------------------------------------------------------------------
//
//  Define:  BSP_I2Cx_OA_INIT
//
//  own address settings for i2c device
//
#define BSP_I2C1_OA_INIT                (0x0E)
#define BSP_I2C2_OA_INIT                (0x0E)
#define BSP_I2C3_OA_INIT                (0x0E)


//------------------------------------------------------------------------------
//
//  Define:  BSP_I2C_TIMEOUT_INIT
//
//  default timeout in tick count units (milli-seconds)
//
#define BSP_I2C_TIMEOUT_INIT            (500)


//------------------------------------------------------------------------------
//
//  Define:  BSP_I2Cx_BAUDRATE_INIT
//
//  default baud rate
//
#define BSP_I2C1_BAUDRATE_INIT          (1)
#define BSP_I2C2_BAUDRATE_INIT          (1)
#define BSP_I2C3_BAUDRATE_INIT          (1)


//------------------------------------------------------------------------------
//
//  Define:  BSP_I2Cx_MAXRETRY_INIT 
//
//  maximum number of attempts before failure
//
#define BSP_I2C1_MAXRETRY_INIT          (5)
#define BSP_I2C2_MAXRETRY_INIT          (5)
#define BSP_I2C3_MAXRETRY_INIT          (5)


//------------------------------------------------------------------------------
//
//  Define:  BSP_I2Cx_RX_THRESHOLD_INIT
//
//  recieve fifo threshold
//
#define BSP_I2C1_RX_THRESHOLD_INIT      (5)
#define BSP_I2C2_RX_THRESHOLD_INIT      (5)
#define BSP_I2C3_RX_THRESHOLD_INIT      (60)


//------------------------------------------------------------------------------
//
//  Define:  BSP_I2Cx_TX_THRESHOLD_INIT
//
//  transmit fifo threshold
//
#define BSP_I2C1_TX_THRESHOLD_INIT      (5)
#define BSP_I2C2_TX_THRESHOLD_INIT      (5)
#define BSP_I2C3_TX_THRESHOLD_INIT      (60)

//------------------------------------------------------------------------------
//
//  Define:  BSP_VC_SMPS_SA_INIT
//
//  slave address for VP1 & VP2
//
#define VC_SMPS_SA1                     (0x12 << 0)
#define VC_SMPS_SA2                     (0x12 << 16)

#define BSP_VC_SMPS_SA_INIT             (VC_SMPS_SA1 | \
                                         VC_SMPS_SA2)
                                         
//------------------------------------------------------------------------------
//
//  Define:  BSP_VC_SMPS_CMD_RA_INIT
//
//  cmd address for VP1 & VP2
//
#define VC_SMPS_CMD_RA1                 (0 << 0)
#define VC_SMPS_CMD_RA2                 (0 << 16)

#define BSP_VC_SMPS_CMD_RA_INIT         (VC_SMPS_CMD_RA1 | \
                                         VC_SMPS_CMD_RA2)

//------------------------------------------------------------------------------
//
//  Define:  BSP_VC_SMPS_VOL_RA_INIT
//
//  volt address for VP1 & VP2
//
#define VC_SMPS_VOL_RA1                 (0 << 0)
#define VC_SMPS_VOL_RA2                 (1 << 16)

#define BSP_VC_SMPS_VOL_RA_INIT         (VC_SMPS_VOL_RA1 | \
                                         VC_SMPS_VOL_RA2)

//------------------------------------------------------------------------------
//
//  Define:  BSP_VC_CH_CONF_INIT
//
//  flag to determine which subaddress to use to control voltage for VP1 & VP2
//
#define VC_CH_CONF_SA0                  (0 << 0)
#define VC_CH_CONF_RAV0                 (0 << 1)
#define VC_CH_CONF_RAC0                 (0 << 2)
#define VC_CH_CONF_RACEN0               (0 << 3)
#define VC_CH_CONF_CMD0                 (0 << 4)
#define VC_CH_CONF_SA1                  (1 << 16)
#define VC_CH_CONF_RAV1                 (1 << 17)
#define VC_CH_CONF_RAC1                 (1 << 18)
#define VC_CH_CONF_RACEN1               (0 << 19)
#define VC_CH_CONF_CMD1                 (1 << 20)

#define BSP_VC_CH_CONF_INIT             (VC_CH_CONF_CMD1 |   \
                                         VC_CH_CONF_RACEN1 | \
                                         VC_CH_CONF_RAC1 |   \
                                         VC_CH_CONF_RAV1 |   \
                                         VC_CH_CONF_SA1 |    \
                                         VC_CH_CONF_CMD0 |   \
                                         VC_CH_CONF_RACEN0 | \
                                         VC_CH_CONF_RAC0 |   \
                                         VC_CH_CONF_RAV0 |   \
                                         VC_CH_CONF_SA0)

//------------------------------------------------------------------------------
//
//  Define:  BSP_PRM_VC_I2C_CFG_INIT
//
//  flag to determine which subaddress to use to control voltage for VP1 & VP2
//
#define VC_I2C_CFG_HSMASTER             (0 << 5)
#define VC_I2C_CFG_SREN                 (0 << 4)
#define VC_I2C_CFG_HSEN                 (0 << 3)
#define VC_I2C_CFG_MCODE                (0x5 << 0)

#define BSP_PRM_VC_I2C_CFG_INIT         (VC_I2C_CFG_HSMASTER |\
                                         VC_I2C_CFG_SREN |    \
                                         VC_I2C_CFG_HSEN |    \
                                         VC_I2C_CFG_MCODE)

//------------------------------------------------------------------------------
//
//  Define:  BSP_PRM_VP1_CONFIG_INIT
//
//  flag to determine which subaddress to use to control voltage for VP1 & VP2
//
#define VP1_CONFIG_ERROROFFSET          (0 << 24)
#define VP1_CONFIG_ERRORGAIN            (0x20 << 16)
#if (BSP_OPM_SELECT == A) || (BSP_CDS == 1)
	#define VP1_CONFIG_INITVOLTAGE_4	(VDD1_INIT_VOLTAGE_VALUE4 << 8) // should same as VC_CMD_0_VOLT_ON
	#define VP1_CONFIG_INITVOLTAGE_3	(VDD1_INIT_VOLTAGE_VALUE3 << 8) // should same as VC_CMD_0_VOLT_ON
	#define VP1_CONFIG_INITVOLTAGE_2	(VDD1_INIT_VOLTAGE_VALUE2 << 8) // should same as VC_CMD_0_VOLT_ON
#else
	#define VP1_CONFIG_INITVOLTAGE		(VDD1_INIT_VOLTAGE_VALUE << 8) // should same as VC_CMD_0_VOLT_ON
#endif
#define VP1_CONFIG_TIMEOUTEN            (1 << 3)
#define VP1_CONFIG_INITVDD              (0 << 2)
#define VP1_CONFIG_FORCEUPDATE          (0 << 1)
#define VP1_CONFIG_VPENABLE             (0 << 0)

#if (BSP_OPM_SELECT == A) || (BSP_CDS == 1)
	#define BSP_PRM_VP1_CONFIG_INIT_4	(VP1_CONFIG_ERROROFFSET |   \
                                         VP1_CONFIG_ERRORGAIN |     \
                                         VP1_CONFIG_INITVOLTAGE_4 | \
                                         VP1_CONFIG_TIMEOUTEN |     \
                                         VP1_CONFIG_INITVDD |       \
                                         VP1_CONFIG_FORCEUPDATE |   \
                                         VP1_CONFIG_VPENABLE)
	#define BSP_PRM_VP1_CONFIG_INIT_3	(VP1_CONFIG_ERROROFFSET |   \
                                         VP1_CONFIG_ERRORGAIN |     \
                                         VP1_CONFIG_INITVOLTAGE_3 | \
                                         VP1_CONFIG_TIMEOUTEN |     \
                                         VP1_CONFIG_INITVDD |       \
                                         VP1_CONFIG_FORCEUPDATE |   \
                                         VP1_CONFIG_VPENABLE)
	#define BSP_PRM_VP1_CONFIG_INIT_2	(VP1_CONFIG_ERROROFFSET |   \
                                         VP1_CONFIG_ERRORGAIN |     \
                                         VP1_CONFIG_INITVOLTAGE_2 | \
                                         VP1_CONFIG_TIMEOUTEN |     \
                                         VP1_CONFIG_INITVDD |       \
                                         VP1_CONFIG_FORCEUPDATE |   \
                                         VP1_CONFIG_VPENABLE)
#else
	#define BSP_PRM_VP1_CONFIG_INIT		(VP1_CONFIG_ERROROFFSET |   \
                                         VP1_CONFIG_ERRORGAIN |     \
                                         VP1_CONFIG_INITVOLTAGE |   \
                                         VP1_CONFIG_TIMEOUTEN |     \
                                         VP1_CONFIG_INITVDD |       \
                                         VP1_CONFIG_FORCEUPDATE |   \
                                         VP1_CONFIG_VPENABLE)
#endif

//------------------------------------------------------------------------------
//
//  Define:  BSP_PRM_VP2_CONFIG_INIT
//
//  flag to determine which subaddress to use to control voltage for VP2 & VP2
//
#define VP2_CONFIG_ERROROFFSET          (0 << 24)
#define VP2_CONFIG_ERRORGAIN            (0x20 << 16)
#define VP2_CONFIG_INITVOLTAGE          (VDD2_INIT_VOLTAGE_VALUE << 8) // should same as VC_CMD_1_VOLT_ON
#define VP2_CONFIG_TIMEOUTEN            (1 << 3)
#define VP2_CONFIG_INITVDD              (0 << 2)
#define VP2_CONFIG_FORCEUPDATE          (0 << 1)
#define VP2_CONFIG_VPENABLE             (0 << 0)

#define BSP_PRM_VP2_CONFIG_INIT         (VP2_CONFIG_ERROROFFSET |   \
                                         VP2_CONFIG_ERRORGAIN |     \
                                         VP2_CONFIG_INITVOLTAGE |   \
                                         VP2_CONFIG_TIMEOUTEN |     \
                                         VP2_CONFIG_INITVDD |       \
                                         VP2_CONFIG_FORCEUPDATE |   \
                                         VP2_CONFIG_VPENABLE)
                                         
//------------------------------------------------------------------------------
//
//  Define:  BSP_PRM_VC_CMD_VAL_0_INIT
//
//  initial voltage for ON, LP, RET, OFF states
//  using the following eq. {volt = 0.0125(val) + 0.6} setup voltage levels
//  Set Vdd1 voltages: ON=<see VDD1_INIT_VOLTAGE_VALUE>, ON_LP=1.0v, VDD1_RET=1.0v, VDD1_OFF=0.0v 
//
#if (BSP_OPM_SELECT == A) || (BSP_CDS == 1)
	#define VC_CMD_0_VOLT_ON_4		(VDD1_INIT_VOLTAGE_VALUE4 << 24)  // should be the same as VP1_CONFIG_INITVOLTAGE
	#define VC_CMD_0_VOLT_ON_3		(VDD1_INIT_VOLTAGE_VALUE3 << 24)  // should be the same as VP1_CONFIG_INITVOLTAGE
	#define VC_CMD_0_VOLT_ON_2		(VDD1_INIT_VOLTAGE_VALUE2 << 24)  // should be the same as VP1_CONFIG_INITVOLTAGE
#else
	#define VC_CMD_0_VOLT_ON                (VDD1_INIT_VOLTAGE_VALUE << 24)  // should be the same as VP1_CONFIG_INITVOLTAGE
#endif
#define VC_CMD_0_VOLT_LP                (0x20 << 16)
#define VC_CMD_0_VOLT_RET               (0x20 << 8)
#define VC_CMD_0_VOLT_OFF               (0x00 << 0)

#if (BSP_OPM_SELECT == A) || (BSP_CDS == 1)
	#define BSP_PRM_VC_CMD_VAL_0_INIT4	(VC_CMD_0_VOLT_ON_4 | VC_CMD_0_VOLT_LP | VC_CMD_0_VOLT_RET | VC_CMD_0_VOLT_OFF)
	#define BSP_PRM_VC_CMD_VAL_0_INIT3	(VC_CMD_0_VOLT_ON_3 | VC_CMD_0_VOLT_LP | VC_CMD_0_VOLT_RET | VC_CMD_0_VOLT_OFF)
	#define BSP_PRM_VC_CMD_VAL_0_INIT2	(VC_CMD_0_VOLT_ON_2 | VC_CMD_0_VOLT_LP | VC_CMD_0_VOLT_RET | VC_CMD_0_VOLT_OFF)
#else
	#define BSP_PRM_VC_CMD_VAL_0_INIT	(VC_CMD_0_VOLT_ON |  \
                                         VC_CMD_0_VOLT_LP |  \
                                         VC_CMD_0_VOLT_RET | \
                                         VC_CMD_0_VOLT_OFF)
#endif
//------------------------------------------------------------------------------
//
//  Define:  BSP_PRM_VC_CMD_VAL_1_INIT
//
//  initial voltage for ON, LP, RET, OFF states
//  using the following eq. {volt = 0.0125(val) + 0.6} setup voltage levels
//  Set Vdd2 voltages: ON=<see VDD2_INIT_VOLTAGE_VALUE>, ON_LP=1.0v, VDD2_RET=1.0v, VDD2_OFF=0.0v
//
#define VC_CMD_1_VOLT_ON                (VDD2_INIT_VOLTAGE_VALUE << 24)  // should be the same as VP2_CONFIG_INITVOLTAGE
#define VC_CMD_1_VOLT_LP                (0x20 << 16)
#define VC_CMD_1_VOLT_RET               (0x20 << 8)
#define VC_CMD_1_VOLT_OFF               (0x00 << 0)

#define BSP_PRM_VC_CMD_VAL_1_INIT       (VC_CMD_1_VOLT_ON |  \
                                         VC_CMD_1_VOLT_LP |  \
                                         VC_CMD_1_VOLT_RET | \
                                         VC_CMD_1_VOLT_OFF)

//------------------------------------------------------------------------------
//
//  Define:  BSP_PRM_VP1_VSTEPMIN_INIT
//
#define VP1_SMPSWAITTIMEMIN             (0x1F4 << 8)
#define VP1_VSTEPMIN                    (0x01 << 0)

#define BSP_PRM_VP1_VSTEPMIN_INIT       (VP1_VSTEPMIN |  \
                                         VP1_SMPSWAITTIMEMIN)

//------------------------------------------------------------------------------
//
//  Define:  BSP_PRM_VP1_VSTEPMAX_INIT
//
#define VP1_SMPSWAITTIMEMAX             (0x1F4 << 8)
#define VP1_VSTEPMAX                    (0x10 << 0)

#define BSP_PRM_VP1_VSTEPMAX_INIT       (VP1_VSTEPMAX |  \
                                         VP1_SMPSWAITTIMEMAX)

//------------------------------------------------------------------------------
//
//  Define:  BSP_PRM_VP2_VSTEPMIN_INIT
//
#define VP2_SMPSWAITTIMEMIN             (0x1F4 << 8)
#define VP2_VSTEPMIN                    (0x01 << 0)

#define BSP_PRM_VP2_VSTEPMIN_INIT       (VP2_VSTEPMIN |  \
                                         VP2_SMPSWAITTIMEMIN)

//------------------------------------------------------------------------------
//
//  Define:  BSP_PRM_VP2_VSTEPMAX_INIT
//
#define VP2_SMPSWAITTIMEMAX             (0x1F4 << 8)
#define VP2_VSTEPMAX                    (0x10 << 0)

#define BSP_PRM_VP2_VSTEPMAX_INIT       (VP2_VSTEPMAX |  \
                                         VP2_SMPSWAITTIMEMAX)

//------------------------------------------------------------------------------
//
//  Define:  BSP_PRM_VP1_VLIMITTO_INIT
//
#define VP1_VDDMAX                      (0x3F << 24)
#define VP1_VDDMMIN                     (0x00 << 16)
#define VP1_TIMEOUT                     (0xFFFF << 0)

#define BSP_PRM_VP1_VLIMITTO_INIT       (VP1_VDDMAX |   \
                                         VP1_VDDMMIN |  \
                                         VP1_TIMEOUT)

//------------------------------------------------------------------------------
//
//  Define:  BSP_PRM_VP2_VLIMITTO_INIT
//
#define VP2_VDDMAX                      (0x3C << 24)
#define VP2_VDDMMIN                     (0x00 << 16)
#define VP2_TIMEOUT                     (0xFFFF << 0)

#define BSP_PRM_VP2_VLIMITTO_INIT       (VP2_VDDMAX |   \
                                         VP2_VDDMMIN |  \
                                         VP2_TIMEOUT)
                                         
//------------------------------------------------------------------------------
//
//  Define:  BSP_MAX_VOLTTRANSITION_TIME
//
//  maximum time 32khz tick to wait for vdd to transition
//
#define BSP_MAX_VOLTTRANSITION_TIME     (100)

//------------------------------------------------------------------------------
//
//  Define:  BSP_ONE_MILLISECOND_TICKS
//
//  No of 32khz tick to wait for 1 millisecond
//
#define BSP_ONE_MILLISECOND_TICKS       (33)
#define BSP_TEN_MILLISECOND_TICKS       (10 * BSP_ONE_MILLISECOND_TICKS)

//------------------------------------------------------------------------------
//
//  Define:  BSP_SR1_ERRMINLIMIT_INIT, BSP_SR1_ERRMAXLIMIT_INIT, 
//           BSP_SR2_ERRMINLIMIT_INIT, BSP_SR2_ERRMAXLIMIT_INIT
//
//  error weight based on TPS659XX
//
#define BSP_SR1_ERRMINLIMIT_INIT        (0xF9)
#define BSP_SR1_ERRMAXLIMIT_INIT        (0x02)
#define BSP_SR1_ERRWEIGHT_INIT          (0x04)

#define BSP_SR2_ERRMINLIMIT_INIT        (0xF9)
#define BSP_SR2_ERRMAXLIMIT_INIT        (0x02)
#define BSP_SR2_ERRWEIGHT_INIT          (0x04)

//------------------------------------------------------------------------------
//
//  Define:  BSP_SRCLKLENGTH_INIT
//  
//  Using the following equation calculate clock length:
//      SrClkLength = f_sr_alwon_fclk / (2 * f_sr_clk)
//          where f_sr_alwon_fclk == frequency of SR_ALWON_FCLK
//                f_sr_clk == desired sampling rate of sensor core
//
//      26000000 / (2 * 100000) ==> 130 for 100khz SR_CLK
//      26000000 / (2 * 400000) ==> ~33 for 400khz SR_CLK
//
#define BSP_SRCLKLEN_INIT               (130)

//------------------------------------------------------------------------------
//
//  Define:  BSP_SR1_ACCUMDATA_INIT
//  
//  Using the following equation calculate clock length:
//      AccumData = T_timewindow * f_sr_clk
//          where T_timewindow == accumulator time window
//                f_sr_clk == desired sampling rate of sensor core
//
//      7.5ms * 100000 ==> 750 samples
//
#define BSP_SR1_ACCUMDATA_INIT          (750)
#define BSP_SR2_ACCUMDATA_INIT          (750)

//------------------------------------------------------------------------------
//
//  Define: BSP_TWL_VDD2_VMODE_CFG
//
//  configure vmode for vdd2
//
//  Allowed values:
//
#define TWL_DCDC_GLOBAL_SMARTREFLEX_EN  (1 << 3)     // enable smartreflex

#define BSP_TWL_DCDC_GLOBAL_CFG         (TWL_DCDC_GLOBAL_SMARTREFLEX_EN)
                                        
//------------------------------------------------------------------------------
//
//  Define:  BSP_TWL_VDD1_STEP
//  
//  active/ret voltage level for vdd1 supplied by T2
//
#define BSP_TWL_VDD1_STEP               (0)

//------------------------------------------------------------------------------
//
//  Define:  BSP_TWL_VDD2_STEP
//  
//  active/ret voltage level for vdd1 supplied by T2
//
#define BSP_TWL_VDD2_STEP               (0)

//------------------------------------------------------------------------------
//
//  Define:  BSP_SRx_SENx_AVGWEIGHT
//
//  AVG weight of smartreflex senspor N and P
//
#define BSP_SR1_SENN_AVGWEIGHT          (0x3)
#define BSP_SR1_SENP_AVGWEIGHT          (0x3)

#define BSP_SR2_SENN_AVGWEIGHT          (0x1)
#define BSP_SR2_SENP_AVGWEIGHT          (0x1)

//------------------------------------------------------------------------------
//
//  Define:  BSP_SR_ACCUMULATION_SIZE
//
//  smartreflex accumulator size
//
#define BSP_SR_ACCUMULATION_SIZE        (0x1F4)

//------------------------------------------------------------------------------
//
//  Define:  TPS659XX_I2C_BUS_ID
//
//  i2c bus twl is on
//      OMAP_DEVICE_I2C1
//      OMAP_DEVICE_I2C2
//      OMAP_DEVICE_I2C3
//
#define TPS659XX_I2C_BUS_ID              (OMAP_DEVICE_I2C1)


//------------------------------------------------------------------------------
//
//  Define:  PCA9575_I2C_BUS_ID
//

//
#define PCA9575_I2C_BUS_ID              (OMAP_DEVICE_I2C2)

//------------------------------------------------------------------------------
//
//  Define:  BSP_TWL_HFCLK_FREQ
//
//  defines the high frequency clock used
//      HFCLK_FREQ_19_2                 (1)
//      HFCLK_FREQ_26                   (2)
//      HFCLK_FREQ_38_4                 (3)
//
#define BSP_TWL_HFCLK_FREQ              (2)

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Define:  BSP_WATCHDOG_WCLR
//  
//  Configure the watchdog prescaler stage (WCLR register)
//
#define BSP_WATCHDOG_WCLR_PRE               (1 << 5)        // Prescaler enable
#define BSP_WATCHDOG_WCLR_PTV               (0 << 2)        // Prescaler value
#define BSP_WATCHDOG_WCLR                   (BSP_WATCHDOG_WCLR_PRE | \
                                             BSP_WATCHDOG_WCLR_PTV)

//------------------------------------------------------------------------------
//
//  Define:  BSP_WATCHDOG_WLDR
//  
//  Configure the watchdog reload value (WLDR register)
//
#define BSP_WATCHDOG_WLDR                   (0xFFFB0000)

//------------------------------------------------------------------------------
//
//  Define:  BSP_WATCHDOG_REFRESH_PERIOD_MILLISECONDS
//  
//  Configure the watchdog refresh period.  This value is in milliseconds and must
//  be sufficient to service the watchdog timer before it expires.
//
#define BSP_WATCHDOG_REFRESH_PERIOD_MILLISECONDS    (3000)

//------------------------------------------------------------------------------
//
//  Define:  BSP_WATCHDOG_THREAD_PRIORITY
//  
//  Configure the kernel watchdog thread priority.
//
#define BSP_WATCHDOG_THREAD_PRIORITY                (100)

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Define:  BSP_OAL_RTC_USES_TPS659XX
//  
//  Configure OAL RTC support to use the TPS659XX
//
//  Set to TRUE if TPS659XX RTC can be used, FALSE for RTC emulation.
//
//  Note: This is set to FALSE for the EVM because the TPS659XX MSEC pin is floating
//  (the TPS659XX cannot be unlocked from secure mode) and the RTC cannot be started.
//
#define BSP_OAL_RTC_USES_TPS659XX                1
//------------------------------------------------------------------------------
//
// Voltage Processor Error Gain Setting for SMARTREFLEX
//
#define VDD1_OPP1_ERRORGAIN     (0x0C)
#define VDD1_OPP2_ERRORGAIN     (0x0C)
#define VDD1_OPP3_ERRORGAIN     (0x18)
#define VDD1_OPP4_ERRORGAIN     (0x18)
#define VDD1_OPP5_ERRORGAIN     (0x18)

#define VDD2_OPP1_ERRORGAIN     (0x0C)
#define VDD2_OPP2_ERRORGAIN     (0x18)

#define VDD_NO_ERROFFSET        (0x0)

//------------------------------------------------------------------------------
#define SR1_OPP1_ERRMINLIMIT    (0xF4)
#define SR1_OPP2_ERRMINLIMIT    (0xF4)
#define SR1_OPP3_ERRMINLIMIT    (0xF9)
#define SR1_OPP4_ERRMINLIMIT    (0xF9)
#define SR1_OPP5_ERRMINLIMIT    (0xF9)

#define SR2_OPP1_ERRMINLIMIT    (0xF4)
#define SR2_OPP2_ERRMINLIMIT    (0xF9)
//------------------------------------------------------------------------------

#endif
