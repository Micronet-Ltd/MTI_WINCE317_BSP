// All rights reserved ADENEO EMBEDDED 2010
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
//  File:  omap3530_irq.h
//
//  This file defines names for IRQ. This names have no other role than make
//  code more readable. For SoC where device - IRQ mapping is defined by
//  silicon and it can't be changed by software.
//
#ifndef __OMAP3530_IRQ_H
#define __OMAP3530_IRQ_H

//------------------------------------------------------------------------------
//
//  Physical IRQs on the 35xx
//

#define IRQ_EMUINT                      0   // MPU emulation
#define IRQ_COMMRX                      1   // MPU emulation
#define IRQ_COMMTX                      2   // MPU emulation
#define IRQ_NMPU                        3   // MPU emulation
#define IRQ_MCBSP2_ST                   4   // Sidetone MCBSP2 overflow
#define IRQ_MCBSP3_ST                   5   // Sidetone MCBSP3 overflow
#define IRQ_SSM_ABORT                   6   // MPU subsystem secure 
                                            // state-machine abort 

#define IRQ_SYS_NIRQ                    7   // External source (active low)
#define IRQ_D2D_FW_STACKED              8   // Occurs when modem causes security 
                                            // violation and has been 
                                            // automatically put DEVICE_SECURITY 
                                            // [0] under reset.

#define IRQ_SMX_DBG                     9   // SMX error for debug
#define IRQ_SMX_APP                     10  // SMX error for application
#define IRQ_PRCM_MPU                    11  // PRCM module IRQ
#define IRQ_SDMA0                       12  // System DMA request 0
#define IRQ_SDMA1                       13  // System DMA request 1
#define IRQ_SDMA2                       14  // System DMA request 2
#define IRQ_SDMA3                       15  // System DMA request 3
#define IRQ_MCBSP1                      16  // McBSP module 1 IRQ 
#define IRQ_MCBSP2                      17  // McBSP module 2 IRQ 
#define IRQ_SR1                         18  // SmartReflex 1
#define IRQ_SR2                         19  // SmartReflex 2
#define IRQ_GPMC                        20  // General-purpose memory 
                                            // controller module

#define IRQ_GFX                         21  // 2D/3D graphics module
#define IRQ_MCBSP3                      22  // McBSP module 3
#define IRQ_MCBSP4                      23  // McBSP module 4
#define IRQ_CAM0                        24  // Camera interface request 0
#define IRQ_DSS                         25  // Display subsystem module
#define IRQ_MAIL_U0_MPU                 26  // Mailbox user 0 request
#define IRQ_MCBSP5                      27  // McBSP module 5 
#define IRQ_IVA2_MMU                    28  // IVA2 MMU
#define IRQ_GPIO1_MPU                   29  // GPIO module 1 
#define IRQ_GPIO2_MPU                   30  // GPIO module 2 
#define IRQ_GPIO3_MPU                   31  // GPIO module 3 
#define IRQ_GPIO4_MPU                   32  // GPIO module 4 
#define IRQ_GPIO5_MPU                   33  // GPIO module 5
#define IRQ_GPIO6_MPU                   34  // GPIO module 6
#define IRQ_zzzRESERVED35               35  // 
#define IRQ_WDT3                        36  // Watchdog timer module 3 
                                            // overflow
#define IRQ_GPTIMER1                    37  // GPTimer module 1
#define IRQ_GPTIMER2                    38  // GPTimer module 2
#define IRQ_GPTIMER3                    39  // GPTimer module 3
#define IRQ_GPTIMER4                    40  // GPTimer module 4
#define IRQ_GPTIMER5                    41  // GPTimer module 5
#define IRQ_GPTIMER6                    42  // GPTimer module 6
#define IRQ_GPTIMER7                    43  // GPTimer module 7
#define IRQ_GPTIMER8                    44  // GPTimer module 8
#define IRQ_GPTIMER9                    45  // GPTimer module 9
#define IRQ_GPTIMER10                   46  // GPTimer module 10
#define IRQ_GPTIMER11                   47  // GPTimer module 11
#define IRQ_SPI4                        48  // McSPI module 4
#define IRQ_SHA1MD52                    49  // SHA-1/MD5 crypto-accelerator 2
#define IRQ_FPKAREADY_N                 50  // PKA crypto-accelerator
#define IRQ_SHA1MD5                     51  // SHA-1/MD5 crypto-accelerator 1
#define IRQ_RNG                         52  // RNG module
#define IRQ_MG                          53  // MG function 
#define IRQ_MCBSP4_TX                   54  // McBSP module 4 transmit
#define IRQ_MCBSP4_RX                   55  // McBSP module 4 receive
#define IRQ_I2C1                        56  // I2C module 1
#define IRQ_I2C2                        57  // I2C module 2
#define IRQ_HDQ                         58  // HDQ/One-wire
#define IRQ_McBSP1_TX                   59  // McBSP module 1 transmit
#define IRQ_McBSP1_RX                   60  // McBSP module 1 receive
#define IRQ_I2C3                        61  // I2C module 3
#define IRQ_McBSP2_TX                   62  // McBSP module 2 transmit
#define IRQ_McBSP2_RX                   63  // McBSP module 2 receive
#define IRQ_FPKARERROR_N                64  // PKA crypto-accelerator
#define IRQ_SPI1                        65  // McSPI module 1
#define IRQ_SPI2                        66  // McSPI module 2
#define IRQ_SSI_P1_MPU0                 67  // Dual SSI port 1 request 0
#define IRQ_SSI_P1_MPU1                 68  // Dual SSI port 1 request 1
#define IRQ_SSI_P2_MPU0                 69  // Dual SSI port 2 request 0
#define IRQ_SSI_P2_MPU1                 70  // Dual SSI port 2 request 1
#define IRQ_SSI_GDD_MPU                 71  // Dual SSI GDD
#define IRQ_UART1                       72  // UART module 1
#define IRQ_UART2                       73  // UART module 2
#define IRQ_UART3                       74  // UART module 3 (also infrared)
#define IRQ_PBIAS                       75  // PBIASlite1 and 2
#define IRQ_OHCI                        76  // OHCI controller HSUSB MP Host Interrupt 
#define IRQ_EHCI                        77  // EHCI controller HSUSB MP Host Interrupt
#define IRQ_TLL                         78  // HSUSB MP TLL Interrupt
#define IRQ_RESERVED1                   79  // unassigned 
#define IRQ_UART4                       80  // UART module 4 (37xx only)
#define IRQ_MCBSP5_TX                   81  // McBSP module 5 transmit
#define IRQ_MCBSP5_RX                   82  // McBSP module 5 receive
#define IRQ_MMC1                        83  // MMC/SD module 1
#define IRQ_RESERVED3                   84  // Reserved
#define IRQ_RESERVED4                   85  // Reserved
#define IRQ_MMC2                        86  // MMC/SD module 2
#define IRQ_MPU_ICR                     87  // MPU ICR
#define IRQ_D2DFRINT                    88  // From 3G coprocessor hardware 
                                            // when used in stacked modem 
                                            // configuration

#define IRQ_MCBSP3_TX                   89  // McBSP module 3 transmit
#define IRQ_MCBSP3_RX                   90  // McBSP module 3 receive
#define IRQ_SPI3                        91  // McSPI module 3
#define IRQ_HSUSB_MC_NINT               92  // High-Speed USB OTG controller
#define IRQ_HSUSB_DMA_NINT              93  // High-Speed USB OTG DMA controller
#define IRQ_MMC3                           94    // MMC/SD module 3
#define IRQ_RESERVED95                  95  // 


//------------------------------------------------------------------------------

#endif // __OMAP3530_IRQ_H

