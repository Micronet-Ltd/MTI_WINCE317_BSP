// All rights reserved ADENEO EMBEDDED 2010
// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

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
//  Header:  omap3530_base_regs.h
//

//------------------------------------------------------------------------------
//  This header file defines the addresses of the base registers for
//  the System on Chip (SoC) components.
//
//  The following abbreviations are used for different addressing type:
//
//                PA - physical address
//                UA - uncached virtual address
//                CA - cached virtual address
//                OA - offset address
//
//  The naming convention for CPU base registers is:
//
//                <SoC>_<SUBSYSTEM>_REGS_<ADDRTYPE>
//
#ifndef __omap3530_BASE_REGS_H
#define __omap3530_BASE_REGS_H

//------------------------------------------------------------------------------
//  Configuration Unit
//------------------------------------------------------------------------------

#define OMAP_REGS_PA                            0x48000000
#define OMAP_REGS_SIZE                          0x08000000

#define OMAP_CONFIG_REGS_PA                     0x48000000
#define OMAP_CONFIG_REGS_SIZE                   0x01000000

#define OMAP_WKUP_CONFIG_REGS_PA                0x48300000
#define OMAP_WKUP_CONFIG_REGS_SIZE              0x00040000

#define OMAP_PERI_CONFIG_REGS_PA                0x49000000
#define OMAP_PERI_CONFIG_REGS_SIZE              0x00100000

#define OMAP_CTRL_REGS_PA                       0x68000000
#define OMAP_CTRL_REGS_SIZE                     0x01000000


//------------------------------------------------------------------------------
//  Power, Reset, Clock Management module
//------------------------------------------------------------------------------

#define OMAP_PRCM_IVA2_CM_REGS_PA               0x48004000
#define OMAP_PRCM_IVA2_CM_REGS_SIZE             0x00000100

#define OMAP_PRCM_OCP_SYSTEM_CM_REGS_PA         0x48004800
#define OMAP_PRCM_OCP_SYSTEM_CM_REGS_SIZE       0x00000100

#define OMAP_PRCM_MPU_CM_REGS_PA                0x48004900
#define OMAP_PRCM_MPU_CM_REGS_SIZE              0x00000100

#define OMAP_PRCM_CORE_CM_REGS_PA               0x48004A00
#define OMAP_PRCM_CORE_CM_REGS_SIZE             0x00000100

#define OMAP_PRCM_SGX_CM_REGS_PA                0x48004B00
#define OMAP_PRCM_SGX_CM_REGS_SIZE              0x00000100

#define OMAP_PRCM_WKUP_CM_REGS_PA               0x48004C00
#define OMAP_PRCM_WKUP_CM_REGS_SIZE             0x00000100

#define OMAP_PRCM_CLOCK_CONTROL_CM_REGS_PA      0x48004D00
#define OMAP_PRCM_CLOCK_CONTROL_CM_REGS_SIZE    0x00000100

#define OMAP_PRCM_DSS_CM_REGS_PA                0x48004E00
#define OMAP_PRCM_DSS_CM_REGS_SIZE              0x00000100

#define OMAP_PRCM_CAM_CM_REGS_PA                0x48004F00
#define OMAP_PRCM_CAM_CM_REGS_SIZE              0x00000100

#define OMAP_PRCM_PER_CM_REGS_PA                0x48005000
#define OMAP_PRCM_PER_CM_REGS_SIZE              0x00000100

#define OMAP_PRCM_EMU_CM_REGS_PA                0x48005100
#define OMAP_PRCM_EMU_CM_REGS_SIZE              0x00000100

#define OMAP_PRCM_GLOBAL_CM_REGS_PA             0x48005200
#define OMAP_PRCM_GLOBAL_CM_REGS_SIZE           0x00000100

#define OMAP_PRCM_NEON_CM_REGS_PA               0x48005300
#define OMAP_PRCM_NEON_CM_REGS_SIZE             0x00000100

#define OMAP_PRCM_USBHOST_CM_REGS_PA            0x48005400
#define OMAP_PRCM_USBHOST_CM_REGS_SIZE          0x00000100

#define OMAP_PRCM_IVA2_PRM_REGS_PA              0x48306000
#define OMAP_PRCM_IVA2_PRM_REGS_SIZE            0x00000100

#define OMAP_PRCM_OCP_SYSTEM_PRM_REGS_PA        0x48306800
#define OMAP_PRCM_OCP_SYSTEM_PRM_REGS_SIZE      0x00000100

#define OMAP_PRCM_MPU_PRM_REGS_PA               0x48306900
#define OMAP_PRCM_MPU_PRM_REGS_SIZE             0x00000100

#define OMAP_PRCM_CORE_PRM_REGS_PA              0x48306A00
#define OMAP_PRCM_CORE_PRM_REGS_SIZE            0x00000100

#define OMAP_PRCM_SGX_PRM_REGS_PA               0x48306B00
#define OMAP_PRCM_SGX_PRM_REGS_SIZE             0x00000100

#define OMAP_PRCM_WKUP_PRM_REGS_PA              0x48306C00
#define OMAP_PRCM_WKUP_PRM_REGS_SIZE            0x00000100

#define OMAP_PRCM_CLOCK_CONTROL_PRM_REGS_PA     0x48306D00
#define OMAP_PRCM_CLOCK_CONTROL_PRM_REGS_SIZE   0x00000100

#define OMAP_PRCM_DSS_PRM_REGS_PA               0x48306E00
#define OMAP_PRCM_DSS_PRM_REGS_SIZE             0x00000100

#define OMAP_PRCM_CAM_PRM_REGS_PA               0x48306F00
#define OMAP_PRCM_CAM_PRM_REGS_SIZE             0x00000100

#define OMAP_PRCM_PER_PRM_REGS_PA               0x48307000
#define OMAP_PRCM_PER_PRM_REGS_SIZE             0x00000100

#define OMAP_PRCM_EMU_PRM_REGS_PA               0x48307100
#define OMAP_PRCM_EMU_PRM_REGS_SIZE             0x00000100

#define OMAP_PRCM_GLOBAL_PRM_REGS_PA            0x48307200
#define OMAP_PRCM_GLOBAL_PRM_REGS_SIZE          0x00000100

#define OMAP_PRCM_NEON_PRM_REGS_PA              0x48307300
#define OMAP_PRCM_NEON_PRM_REGS_SIZE            0x00000100

#define OMAP_PRCM_USBHOST_PRM_REGS_PA           0x48307400
#define OMAP_PRCM_USBHOST_PRM_REGS_SIZE         0x00000100

#define OMAP_CONTEXT_RESTORE_REGS_PA            0x48002910
#define OMAP_CONTEXT_RESTORE_REGS_SIZE          0x00000024

#define OMAP_PRCM_RESTORE_REGS_PA               0x48002934
#define OMAP_PRCM_RESTORE_REGS_SIZE             0x00000040

#define OMAP_SDRC_RESTORE_REGS_PA               0x48002974
#define OMAP_SDRC_RESTORE_REGS_SIZE             0x00000058

//------------------------------------------------------------------------------
//  UART Units
//------------------------------------------------------------------------------

#define OMAP_UART1_REGS_PA                      0x4806A000
#define OMAP_UART2_REGS_PA                      0x4806C000
#define OMAP_UART3_REGS_PA                      0x49020000
// 37xx only
#define OMAP_UART4_REGS_PA                      0x49022000

//------------------------------------------------------------------------------
//  USIM
//------------------------------------------------------------------------------

#define OMAP_USIM_REG_PA                        0x4830E000

//------------------------------------------------------------------------------
//  USB
//------------------------------------------------------------------------------

#define OMAP_USBOT1_REG_PA                      0x4805E000


//------------------------------------------------------------------------------
//  FS USB Host Controller
//------------------------------------------------------------------------------

#define OMAP_USBH_REGS_PA                       0x4805E000

//------------------------------------------------------------------------------
//  USB OTG Controller
//------------------------------------------------------------------------------

#define OMAP_OTG_REGS_PA                        0x4805E300

//------------------------------------------------------------------------------
//  USB TLL Controller
//------------------------------------------------------------------------------

#define OMAP_USBTLL_REGS_PA                     0x48062000

//------------------------------------------------------------------------------
//  USB Device Controller
//------------------------------------------------------------------------------

#define OMAP_HSUSB_REGS_PA                      0x480AB000

#define OMAP_USBHOST1_REGS_LA                   0x00077001
#define OMAP_USBHOST2_REGS_LA                   0x00078001
#define OMAP_USBHOST3_REGS_LA                   0x00079001

//------------------------------------------------------------------------------
//  Camera Controller
//------------------------------------------------------------------------------

#define OMAP_CAMISP_REGS_PA                     0x480BC000 // Camera ISP
#define OMAP_CAMCSI2_RECEIVER_REGS_PA           0x480bd800 // CSI2 Reciever

/* TBD

#define OMAP_CAMSUB_REGS_PA                     0x48052000 // Camera top
#define OMAP_CAMCORE_REGS_PA                    0x48052400 // Camera core
#define OMAP_CAMDMA_REGS_PA                     0x48052800 // Camera DMA
#define OMAP_CAMMMU_REGS_PA                     0x48052C00 // Camera MMU

*/


//------------------------------------------------------------------------------
//  IVA MMU base address
//------------------------------------------------------------------------------

#define OMAP_IVA_MMU_REGS_PA                    0x5D000000

//------------------------------------------------------------------------------
//  HDQ/1Wire Controller
//------------------------------------------------------------------------------

#define OMAP_HDQ_1WIRE_REGS_PA                  0x480B2000


//-----------------------------------------------------------------------------
// MMC/SD/SDIO
//-----------------------------------------------------------------------------

#define OMAP_MMCHS1_REGS_PA                     0x4809C000
#define OMAP_MMCHS2_REGS_PA                     0x480B4000
#define OMAP_MMCHS3_REGS_PA                     0x480AD000


//------------------------------------------------------------------------------
//  32K Sync Timer Controller
//------------------------------------------------------------------------------

#define OMAP_32KSYNC_REGS_PA                    0x48320000


//------------------------------------------------------------------------------
//  Mailbox registers
//------------------------------------------------------------------------------

#define OMAP_MLB1_REGS_PA                       0x48094000

//------------------------------------------------------------------------------
//  IDCODE - CPU version and silicon rev information register
//------------------------------------------------------------------------------

#define OMAP_IDCODE_REGS_PA                     0x4830A204

//------------------------------------------------------------------------------
//  Status Control register - contains SYS.BOOT  information register
//  SYS_BOOT == 24 -> Boot from MMC1 (SD Card on EVM3530).
//  SYS_BOOT == 22 -> Boot from UART
//  SYS_BOOT ==   4 -> Boot from USB
//------------------------------------------------------------------------------

#define OMAP_STATUS_CONTROL_REGS_PA                     0x480022F0

//------------------------------------------------------------------------------
//  DSP MMU Controller
//------------------------------------------------------------------------------

/* #define OMAP_DSP_MMU_REGS_PA                 0x5A000000 TBD */

//------------------------------------------------------------------------------
//  GPIO Controllers
//------------------------------------------------------------------------------

#define OMAP_GPIO1_REGS_PA                      0x48310000
#define OMAP_GPIO2_REGS_PA                      0x49050000
#define OMAP_GPIO3_REGS_PA                      0x49052000
#define OMAP_GPIO4_REGS_PA                      0x49054000
#define OMAP_GPIO5_REGS_PA                      0x49056000
#define OMAP_GPIO6_REGS_PA                      0x49058000


//------------------------------------------------------------------------------
//  OCP Controller
//------------------------------------------------------------------------------

/* #define OMAP_OCP_REG_PA                      0x5C060000 TBD */

//------------------------------------------------------------------------------
//  Timer Units
//------------------------------------------------------------------------------

#define OMAP_GPTIMER1_REGS_PA                   0x48318000
#define OMAP_GPTIMER2_REGS_PA                   0x49032000
#define OMAP_GPTIMER3_REGS_PA                   0x49034000
#define OMAP_GPTIMER4_REGS_PA                   0x49036000
#define OMAP_GPTIMER5_REGS_PA                   0x49038000
#define OMAP_GPTIMER6_REGS_PA                   0x4903A000
#define OMAP_GPTIMER7_REGS_PA                   0x4903C000
#define OMAP_GPTIMER8_REGS_PA                   0x4903E000
#define OMAP_GPTIMER9_REGS_PA                   0x49040000
#define OMAP_GPTIMER10_REGS_PA                  0x48086000
#define OMAP_GPTIMER11_REGS_PA                  0x48088000
#define OMAP_GPTIMER12_REGS_PA                  0x48304000

//------------------------------------------------------------------------------
//  Watchdog Unit
//------------------------------------------------------------------------------

#define OMAP_WDOG1_REGS_PA                      0x4830C000 // Secure
#define OMAP_WDOG2_REGS_PA                      0x48314000 // OMAP
#define OMAP_WDOG3_REGS_PA                      0x49030000 // IVA2

//------------------------------------------------------------------------------
//  Interrupt Units
//------------------------------------------------------------------------------

#define OMAP_INTC_MPU_REGS_PA                   0x48200000

/* #define OMAP_INTC_IVA_REGS_PA                0x40000000 TBD */

//------------------------------------------------------------------------------
//  I2C Controller
//------------------------------------------------------------------------------

#define OMAP_I2C1_REGS_PA                       0x48070000
#define OMAP_I2C2_REGS_PA                       0x48072000
#define OMAP_I2C3_REGS_PA                       0x48060000


//------------------------------------------------------------------------------
//  System and DSP DMA Controllers
//------------------------------------------------------------------------------

// The registers 0ffset 0x00 - 0x7C are common to all the DMA's
// So the logical channel starts at offset 0x80.

#define OMAP_SDMA_REGS_PA                       0x48056000

//------------------------------------------------------------------------------
//  Display Subsystem
//------------------------------------------------------------------------------

#define OMAP_DSS1_REGS_PA                       0x48050000
#define OMAP_DISC1_REGS_PA                      0x48050400
#define OMAP_RFBI1_REGS_PA                      0x48050800
#define OMAP_VENC1_REGS_PA                      0x48050C00
#define OMAP_DSI_REGS_PA                        0x4804FC00
#define OMAP_DSIPHY_REGS_PA                     0x4804FE00
#define OMAP_DSI_PLL_REGS_PA                    0x4804FF00

//------------------------------------------------------------------------------
//  McBSP base addresses
//  (see file omap_McBSP.h for offset definitions for these base addresses)
//------------------------------------------------------------------------------

#define OMAP_MCBSP1_REGS_PA                     0x48074000
#define OMAP_MCBSP2_REGS_PA                     0x49022000
#define OMAP_MCBSP3_REGS_PA                     0x49024000
#define OMAP_MCBSP4_REGS_PA                     0x49026000
#define OMAP_MCBSP5_REGS_PA                     0x48096000

#define OMAP_MCBSP2_SIDETONE_REGS_PA   0x49028000
#define OMAP_MCBSP3_SIDETONE_REGS_PA   0x4902A000


//------------------------------------------------------------------------------
//  SRAM embedded memory
//------------------------------------------------------------------------------

#define OMAP_SRAM_SIZE                          (64*1024)       /* (62*1024) */
#define OMAP_SRAM_PA                            0x40200000

//-----------------------------------------------------------------------------
// SDRAM for Display
//-----------------------------------------------------------------------------

/* #define OMAP_SDRAM_LCD_PA                    0xA0000000 TBD */
/* #define OMAP_SDRAM_LCD_SIZE                  0x01000000 TBD */


//------------------------------------------------------------------------------
//  System Control Module Register base addresses
//------------------------------------------------------------------------------

#define OMAP_SYSC_INTERFACE_REGS_PA             0x48002000
#define OMAP_SYSC_PADCONFS_REGS_PA              0x48002030
#define OMAP_SYSC_GENERAL_REGS_PA               0x48002270
#define OMAP_SYSC_DEVICE_ID_PA                  0x4830A204 
#define OMAP_SYSC_MEM_WKUP_PA                   0x48002600
#define OMAP_SYSC_PADCONFS_WKUP_REGS_PA         0x48002A00
#define OMAP_SYSC_GENERAL_WKUP_REGS_PA          0x48002A60
// DM3730 only
#define OMAP_SYSC_GENERAL_WKUP_REGS_DM3730_PA   0x48002A5C

//------------------------------------------------------------------------------
//  GPMC Module Register base address
//  (see file omap_gpmc.h for offset definitions for this base address)
//------------------------------------------------------------------------------

#define OMAP_GPMC_REGS_PA                       0x6E000000
#define OMAP_GPMC_REGS_SIZE                     0x00001000

//------------------------------------------------------------------------------
//  SDRAM module register base addresses
//  (see file omap_sdram.h for offset definitions for these base addresses)
//------------------------------------------------------------------------------

#define OMAP_SMS_REGS_PA                        0x6C000000
#define OMAP_SMS_REGS_SIZE                      0x00001000

#define OMAP_SDRC_REGS_PA                       0x6D000000
#define OMAP_SDRC_REGS_SIZE                     0x00001000

#define OMAP_SDRAM_PA                           0x80000000

//------------------------------------------------------------------------------
//  McSPI base addresses
//  (see file omap_McSPI.h for offset definitions for these base addresses)
//------------------------------------------------------------------------------

#define OMAP_MCSPI1_REGS_PA                     0x48098000
#define OMAP_MCSPI2_REGS_PA                     0x4809A000
#define OMAP_MCSPI3_REGS_PA                     0x480B8000
#define OMAP_MCSPI4_REGS_PA                     0x480BA000


//------------------------------------------------------------------------------
//  DSP subsystem (IPI module, dealing with OMAP 24xx memory space) base address
//  (see file omap_dsp.h for offset definitions for this base address)
//------------------------------------------------------------------------------

/* #define OMAP_DSP_IPI_REGS_PA                 0x59000000
 #define OMAP_DSP_SS_REGS_PA                    0x5C000000 TBD */

//------------------------------------------------------------------------------
//  SSI Controller, see file omap_SSI.h for offset definitions
//------------------------------------------------------------------------------

#define OMAP_SSI_REGS_PA                        0x48058000  // SSI controller
#define OMAP_GDD1_REGS_PA                       0x48059000  // Generic distribute DMD port 1
#define OMAP_SST1_REGS_PA                       0x4805A000  // Synchronized serial tranmitter port 1
#define OMAP_SSR1_REGS_PA                       0x4805A800  // Synchronized serial receiver port 1
#define OMAP_SST2_REGS_PA                       0x4805B000  // Synchronized serial transmitter port 2
#define OMAP_SSR2_REGS_PA                       0x4805B800  // Synchronized serial reciever port 2

//------------------------------------------------------------------------------
//  SGX - Base address of SGX register
//------------------------------------------------------------------------------

#define OMAP_SGX_REGS_PA                        0x50000000
#define OMAP_SGX_REGS_SIZE                      0x00010000

//------------------------------------------------------------------------------
//  IDCORE - CPU version and silicon rev information register
//------------------------------------------------------------------------------

#define OMAP_IDCORE_REGS_PA                     0x4830A204  

//------------------------------------------------------------------------------
//  VRFB - Base address of VRFB register
//------------------------------------------------------------------------------

#define OMAP_VRFB_REGS_PA                       0x6C000164

//------------------------------------------------------------------------------
//  SmartReflex - Base address of SmartReflex1, SmartReflex2 register
//------------------------------------------------------------------------------

#define OMAP_SMARTREFLEX1_PA                    0x480C9000
#define OMAP_SMARTREFLEX2_PA                    0x480CB000

//------------------------------------------------------------------------------
//  MODEM - Base address of MODEM register
//------------------------------------------------------------------------------

#define OMAP_MODEM_REGS_PA                      0x480C7000

//------------------------------------------------------------------------------
//  USB OTG Controller base address
//------------------------------------------------------------------------------

#define OMAP_USBHS_REGS_PA                      0x480AB000

//------------------------------------------------------------------------------
//  USB OTG Host Controller
//------------------------------------------------------------------------------

#define OMAP_USBHS_HOST_REGS_PA                 OMAP_USBHS_REGS_PA

//------------------------------------------------------------------------------
//   USB OTG Device Controller
//------------------------------------------------------------------------------

#define OMAP_USBHS_DEV_REGS_PA                  OMAP_USBHS_REGS_PA

//------------------------------------------------------------------------------
//  USB OTG Controller
//------------------------------------------------------------------------------

#define OMAP_USBHS_OTG_REGS_PA                  OMAP_USBHS_REGS_PA

//------------------------------------------------------------------------------
//  USB HOST UHH Controller
//------------------------------------------------------------------------------

#define OMAP_USBUHH_REGS_PA                     0x48064000

//------------------------------------------------------------------------------
//  USB HOST OHCI Controller
//------------------------------------------------------------------------------

#define OMAP_USBOHCI_REGS_PA                    0x48064400

//------------------------------------------------------------------------------
//  USB HOST EHCI Controller
//------------------------------------------------------------------------------

#define OMAP_USBEHCI_REGS_PA                    0x48064800

//------------------------------------------------------------------------------
//  IVA2
//------------------------------------------------------------------------------

#define OMAP_PRCM_IVA2_CM_REGS_PA               0x48004000


#endif // __OMAP_BASE_REGS_H
