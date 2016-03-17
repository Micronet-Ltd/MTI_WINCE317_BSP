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

//------------------------------------------------------------------------------
//
//  File:  omap35xx_config.h
//
//  This header file is comprised of control pad configuration module register
//  details defined as structures and macros for configuring the pads.
//

#ifndef __OMAP35XX_CONFIG_H
#define __OMAP35XX_CONFIG_H

//------------------------------------------------------------------------------
// System PAD Configuration Registers

// OMAP_SYSC_INTERFACE_REGS
//
typedef volatile struct {

    unsigned long CONTROL_REVISION;                         // offset 0x0000 
    unsigned long zzzReserved01[3];
    unsigned long CONTROL_SYSCONFIG;                        // offset 0x0010 

} OMAP_SYSC_INTERFACE_REGS;


// OMAP_SYSC_PADCONFS_REGS
//
typedef volatile struct {
    unsigned short CONTROL_PADCONF_SDRC_D0;                 // offset 0x2030
    unsigned short CONTROL_PADCONF_SDRC_D1;                  
    unsigned short CONTROL_PADCONF_SDRC_D2;                 // offset 0x2034
    unsigned short CONTROL_PADCONF_SDRC_D3;                  
    unsigned short CONTROL_PADCONF_SDRC_D4;                 // offset 0x2038
    unsigned short CONTROL_PADCONF_SDRC_D5;                  
    unsigned short CONTROL_PADCONF_SDRC_D6;                 // offset 0x203C
    unsigned short CONTROL_PADCONF_SDRC_D7;                  
    unsigned short CONTROL_PADCONF_SDRC_D8;                 // offset 0x2040
    unsigned short CONTROL_PADCONF_SDRC_D9;                  
    unsigned short CONTROL_PADCONF_SDRC_D10;                // offset 0x2044
    unsigned short CONTROL_PADCONF_SDRC_D11;                 
    unsigned short CONTROL_PADCONF_SDRC_D12;                // offset 0x2048
    unsigned short CONTROL_PADCONF_SDRC_D13;                 
    unsigned short CONTROL_PADCONF_SDRC_D14;                // offset 0x204C
    unsigned short CONTROL_PADCONF_SDRC_D15;                 
    unsigned short CONTROL_PADCONF_SDRC_D16;                // offset 0x2050
    unsigned short CONTROL_PADCONF_SDRC_D17;                 
    unsigned short CONTROL_PADCONF_SDRC_D18;                // offset 0x2054
    unsigned short CONTROL_PADCONF_SDRC_D19;                 
    unsigned short CONTROL_PADCONF_SDRC_D20;                // offset 0x2058
    unsigned short CONTROL_PADCONF_SDRC_D21;                 
    unsigned short CONTROL_PADCONF_SDRC_D22;                // offset 0x205C
    unsigned short CONTROL_PADCONF_SDRC_D23;                 
    unsigned short CONTROL_PADCONF_SDRC_D24;                // offset 0x2060
    unsigned short CONTROL_PADCONF_SDRC_D25;                 
    unsigned short CONTROL_PADCONF_SDRC_D26;                // offset 0x2064
    unsigned short CONTROL_PADCONF_SDRC_D27;                 
    unsigned short CONTROL_PADCONF_SDRC_D28;                // offset 0x2068
    unsigned short CONTROL_PADCONF_SDRC_D29;                 
    unsigned short CONTROL_PADCONF_SDRC_D30;                // offset 0x206C
    unsigned short CONTROL_PADCONF_SDRC_D31;                 
    unsigned short CONTROL_PADCONF_SDRC_CLK;                // offset 0x2070
    unsigned short CONTROL_PADCONF_SDRC_DQS0;                               
    unsigned short CONTROL_PADCONF_SDRC_DQS1;               // offset 0x2074
    unsigned short CONTROL_PADCONF_SDRC_DQS2;                
    unsigned short CONTROL_PADCONF_SDRC_DQS3;               // offset 0x2078
    unsigned short CONTROL_PADCONF_GPMC_A1;                  
    unsigned short CONTROL_PADCONF_GPMC_A2;                 // offset 0x207C
    unsigned short CONTROL_PADCONF_GPMC_A3;                  
    unsigned short CONTROL_PADCONF_GPMC_A4;                 // offset 0x2080
    unsigned short CONTROL_PADCONF_GPMC_A5;                  
    unsigned short CONTROL_PADCONF_GPMC_A6;                 // offset 0x2084
    unsigned short CONTROL_PADCONF_GPMC_A7;                  
    unsigned short CONTROL_PADCONF_GPMC_A8;                 // offset 0x2088
    unsigned short CONTROL_PADCONF_GPMC_A9;                  
    unsigned short CONTROL_PADCONF_GPMC_A10;                // offset 0x208C
    unsigned short CONTROL_PADCONF_GPMC_D0;                  
    unsigned short CONTROL_PADCONF_GPMC_D1;                 // offset 0x2090
    unsigned short CONTROL_PADCONF_GPMC_D2;                  
    unsigned short CONTROL_PADCONF_GPMC_D3;                 // offset 0x2094
    unsigned short CONTROL_PADCONF_GPMC_D4;                  
    unsigned short CONTROL_PADCONF_GPMC_D5;                 // offset 0x2098
    unsigned short CONTROL_PADCONF_GPMC_D6;                  
    unsigned short CONTROL_PADCONF_GPMC_D7;                 // offset 0x209C
    unsigned short CONTROL_PADCONF_GPMC_D8;                  
    unsigned short CONTROL_PADCONF_GPMC_D9;                 // offset 0x20A0
    unsigned short CONTROL_PADCONF_GPMC_D10;                 
    unsigned short CONTROL_PADCONF_GPMC_D11;                // offset 0x20A4
    unsigned short CONTROL_PADCONF_GPMC_D12;                 
    unsigned short CONTROL_PADCONF_GPMC_D13;                // offset 0x20A8
    unsigned short CONTROL_PADCONF_GPMC_D14;                 
    unsigned short CONTROL_PADCONF_GPMC_D15;                // offset 0x20AC
    unsigned short CONTROL_PADCONF_GPMC_nCS0;                
    unsigned short CONTROL_PADCONF_GPMC_nCS1;               // offset 0x20B0
    unsigned short CONTROL_PADCONF_GPMC_nCS2;                
    unsigned short CONTROL_PADCONF_GPMC_nCS3;               // offset 0x20B4 
    unsigned short CONTROL_PADCONF_GPMC_nCS4;                
    unsigned short CONTROL_PADCONF_GPMC_nCS5;               // offset 0x20B8 
    unsigned short CONTROL_PADCONF_GPMC_nCS6;                
    unsigned short CONTROL_PADCONF_GPMC_nCS7;               // offset 0x20BC 
    unsigned short CONTROL_PADCONF_GPMC_CLK;                 
    unsigned short CONTROL_PADCONF_GPMC_nADV_ALE;           // offset 0x20C0 
    unsigned short CONTROL_PADCONF_GPMC_nOE;                 
    unsigned short CONTROL_PADCONF_GPMC_nWE;                // offset 0x20C4
    unsigned short CONTROL_PADCONF_GPMC_nBE0_CLE;            
    unsigned short CONTROL_PADCONF_GPMC_nBE1;               // offset 0x20C8
    unsigned short CONTROL_PADCONF_GPMC_nWP;                 
    unsigned short CONTROL_PADCONF_GPMC_WAIT0;              // offset 0x20CC
    unsigned short CONTROL_PADCONF_GPMC_WAIT1;               
    unsigned short CONTROL_PADCONF_GPMC_WAIT2;              // offset 0x20D0
    unsigned short CONTROL_PADCONF_GPMC_WAIT3;               
    unsigned short CONTROL_PADCONF_DSS_PCLK;                // offset 0x20D4
    unsigned short CONTROL_PADCONF_DSS_HSYNC;                
    unsigned short CONTROL_PADCONF_DSS_VSYNC;               // offset 0x20D8
    unsigned short CONTROL_PADCONF_DSS_ACBIAS;               
    unsigned short CONTROL_PADCONF_DSS_DATA0;               // offset 0x20DC
    unsigned short CONTROL_PADCONF_DSS_DATA1;                
    unsigned short CONTROL_PADCONF_DSS_DATA2;               // offset 0x20E0
    unsigned short CONTROL_PADCONF_DSS_DATA3;                
    unsigned short CONTROL_PADCONF_DSS_DATA4;               // offset 0x20E4
    unsigned short CONTROL_PADCONF_DSS_DATA5;                
    unsigned short CONTROL_PADCONF_DSS_DATA6;               // offset 0x20E8
    unsigned short CONTROL_PADCONF_DSS_DATA7;                
    unsigned short CONTROL_PADCONF_DSS_DATA8;               // offset 0x20EC
    unsigned short CONTROL_PADCONF_DSS_DATA9;                
    unsigned short CONTROL_PADCONF_DSS_DATA10;              // offset 0x20F0
    unsigned short CONTROL_PADCONF_DSS_DATA11;               
    unsigned short CONTROL_PADCONF_DSS_DATA12;              // offset 0x20F4
    unsigned short CONTROL_PADCONF_DSS_DATA13;               
    unsigned short CONTROL_PADCONF_DSS_DATA14;              // offset 0x20F8
    unsigned short CONTROL_PADCONF_DSS_DATA15;               
    unsigned short CONTROL_PADCONF_DSS_DATA16;              // offset 0x20FC
    unsigned short CONTROL_PADCONF_DSS_DATA17;               
    unsigned short CONTROL_PADCONF_DSS_DATA18;              // offset 0x2100
    unsigned short CONTROL_PADCONF_DSS_DATA19;               
    unsigned short CONTROL_PADCONF_DSS_DATA20;              // offset 0x2104
    unsigned short CONTROL_PADCONF_DSS_DATA21;               
    unsigned short CONTROL_PADCONF_DSS_DATA22;              // offset 0x2108
    unsigned short CONTROL_PADCONF_DSS_DATA23;               
    unsigned short CONTROL_PADCONF_CAM_HS;                  // offset 0x210C
    unsigned short CONTROL_PADCONF_CAM_VS;                   
    unsigned short CONTROL_PADCONF_CAM_XCLKA;               // offset 0x2110
    unsigned short CONTROL_PADCONF_CAM_PCLK;                 
    unsigned short CONTROL_PADCONF_CAM_FLD;                 // offset 0x2114 
    unsigned short CONTROL_PADCONF_CAM_D0;                   
    unsigned short CONTROL_PADCONF_CAM_D1;                  // offset 0x2118
    unsigned short CONTROL_PADCONF_CAM_D2;                   
    unsigned short CONTROL_PADCONF_CAM_D3;                  // offset 0x211C
    unsigned short CONTROL_PADCONF_CAM_D4;                   
    unsigned short CONTROL_PADCONF_CAM_D5;                  // offset 0x2120
    unsigned short CONTROL_PADCONF_CAM_D6;                   
    unsigned short CONTROL_PADCONF_CAM_D7;                  // offset 0x2124
    unsigned short CONTROL_PADCONF_CAM_D8;                   
    unsigned short CONTROL_PADCONF_CAM_D9;                  // offset 0x2128
    unsigned short CONTROL_PADCONF_CAM_D10;                  
    unsigned short CONTROL_PADCONF_CAM_D11;                 // offset 0x212C
    unsigned short CONTROL_PADCONF_CAM_XCLKB;                
    unsigned short CONTROL_PADCONF_CAM_WEN;                 // offset 0x2130
    unsigned short CONTROL_PADCONF_CAM_STROBE;               
    unsigned short CONTROL_PADCONF_CSI2_DX0;                // offset 0x2134
    unsigned short CONTROL_PADCONF_CSI2_DY0;                 
    unsigned short CONTROL_PADCONF_CSI2_DX1;                // offset 0x2138
    unsigned short CONTROL_PADCONF_CSI2_DY1;                 
    unsigned short CONTROL_PADCONF_MCBSP2_FSX;              // offset 0x213C
    unsigned short CONTROL_PADCONF_MCBSP2_CLKX;              
    unsigned short CONTROL_PADCONF_MCBSP2_DR;               // offset 0x2140
    unsigned short CONTROL_PADCONF_MCBSP2_DX;                
    unsigned short CONTROL_PADCONF_MMC1_CLK;                // offset 0x2144
    unsigned short CONTROL_PADCONF_MMC1_CMD;                 
    unsigned short CONTROL_PADCONF_MMC1_DAT0;               // offset 0x2148
    unsigned short CONTROL_PADCONF_MMC1_DAT1;                
    unsigned short CONTROL_PADCONF_MMC1_DAT2;               // offset 0x214C
    unsigned short CONTROL_PADCONF_MMC1_DAT3;                
    unsigned short CONTROL_PADCONF_MMC1_DAT4;               // offset 0x2150
    unsigned short CONTROL_PADCONF_MMC1_DAT5;                
    unsigned short CONTROL_PADCONF_MMC1_DAT6;               // offset 0x2154
    unsigned short CONTROL_PADCONF_MMC1_DAT7;                
    unsigned short CONTROL_PADCONF_MMC2_CLK;                // offset 0x2158
    unsigned short CONTROL_PADCONF_MMC2_CMD;                 
    unsigned short CONTROL_PADCONF_MMC2_DAT0;               // offset 0x215C
    unsigned short CONTROL_PADCONF_MMC2_DAT1;                
    unsigned short CONTROL_PADCONF_MMC2_DAT2;               // offset 0x2160
    unsigned short CONTROL_PADCONF_MMC2_DAT3;                
    unsigned short CONTROL_PADCONF_MMC2_DAT4;               // offset 0x2164 
    unsigned short CONTROL_PADCONF_MMC2_DAT5;                
    unsigned short CONTROL_PADCONF_MMC2_DAT6;               // offset 0x2168 
    unsigned short CONTROL_PADCONF_MMC2_DAT7;                
    unsigned short CONTROL_PADCONF_MCBSP3_DX;               // offset 0x216C 
    unsigned short CONTROL_PADCONF_MCBSP3_DR;                
    unsigned short CONTROL_PADCONF_MCBSP3_CLKX;             // offset 0x2170
    unsigned short CONTROL_PADCONF_MCBSP3_FSX;              
    unsigned short CONTROL_PADCONF_UART2_CTS;               // offset 0x2174 
    unsigned short CONTROL_PADCONF_UART2_RTS;                
    unsigned short CONTROL_PADCONF_UART2_TX;                // offset 0x2178 
    unsigned short CONTROL_PADCONF_UART2_RX;                 
    unsigned short CONTROL_PADCONF_UART1_TX;                // offset 0x217C
    unsigned short CONTROL_PADCONF_UART1_RTS;                
    unsigned short CONTROL_PADCONF_UART1_CTS;               // offset 0x2180 
    unsigned short CONTROL_PADCONF_UART1_RX;                 
    unsigned short CONTROL_PADCONF_MCBSP4_CLKX;             // offset 0x2184
    unsigned short CONTROL_PADCONF_MCBSP4_DR;                
    unsigned short CONTROL_PADCONF_MCBSP4_DX;               // offset 0x2188
    unsigned short CONTROL_PADCONF_MCBSP4_FSX;               
    unsigned short CONTROL_PADCONF_MCBSP1_CLKR;             // offset 0x218C
    unsigned short CONTROL_PADCONF_MCBSP1_FSR;               
    unsigned short CONTROL_PADCONF_MCBSP1_DX;               // offset 0x2190
    unsigned short CONTROL_PADCONF_MCBSP1_DR;                
    unsigned short CONTROL_PADCONF_MCBSP_CLKS;              // offset 0x2194
    unsigned short CONTROL_PADCONF_MCBSP1_FSX;               
    unsigned short CONTROL_PADCONF_MCBSP1_CLKX;             // offset 0x2198
    unsigned short CONTROL_PADCONF_UART3_CTS_RCTX;            
    unsigned short CONTROL_PADCONF_UART3_RTS_SD;            // offset 0x219C
    unsigned short CONTROL_PADCONF_UART3_RX_IRRX;            
    unsigned short CONTROL_PADCONF_UART3_TX_IRTX;           // offset 0x21A0
    unsigned short CONTROL_PADCONF_HSUSB0_CLK;               
    unsigned short CONTROL_PADCONF_HSUSB0_STP;              // offset 0x21A4
    unsigned short CONTROL_PADCONF_HSUSB0_DIR;               
    unsigned short CONTROL_PADCONF_HSUSB0_NXT;              // offset 0x21A8
    unsigned short CONTROL_PADCONF_HSUSB0_DATA0;             
    unsigned short CONTROL_PADCONF_HSUSB0_DATA1;            // offset 0x21AC 
    unsigned short CONTROL_PADCONF_HSUSB0_DATA2;             
    unsigned short CONTROL_PADCONF_HSUSB0_DATA3;            // offset 0x21B0 
    unsigned short CONTROL_PADCONF_HSUSB0_DATA4;             
    unsigned short CONTROL_PADCONF_HSUSB0_DATA5;            // offset 0x21B4 
    unsigned short CONTROL_PADCONF_HSUSB0_DATA6;             
    unsigned short CONTROL_PADCONF_HSUSB0_DATA7;            // offset 0x21B8 
    unsigned short CONTROL_PADCONF_I2C1_SCL;                 
    unsigned short CONTROL_PADCONF_I2C1_SDA;                // offset 0x21BC
    unsigned short CONTROL_PADCONF_I2C2_SCL;                 
    unsigned short CONTROL_PADCONF_I2C2_SDA;                // offset 0x21C0
    unsigned short CONTROL_PADCONF_I2C3_SCL;                 
    unsigned short CONTROL_PADCONF_I2C3_SDA;                // offset 0x21C4
    unsigned short CONTROL_PADCONF_HDQ_SIO;                  
    unsigned short CONTROL_PADCONF_MCSPI1_CLK;              // offset 0x21C8
    unsigned short CONTROL_PADCONF_MCSPI1_SIMO;              
    unsigned short CONTROL_PADCONF_MCSPI1_SOMI;             // offset 0x21CC
    unsigned short CONTROL_PADCONF_MCSPI1_CS0;               
    unsigned short CONTROL_PADCONF_MCSPI1_CS1;              // offset 0x21D0
    unsigned short CONTROL_PADCONF_MCSPI1_CS2;               
    unsigned short CONTROL_PADCONF_MCSPI1_CS3;              // offset 0x21D4 
    unsigned short CONTROL_PADCONF_MCSPI2_CLK;               
    unsigned short CONTROL_PADCONF_MCSPI2_SIMO;             // offset 0x21D8 
    unsigned short CONTROL_PADCONF_MCSPI2_SOMI;              
    unsigned short CONTROL_PADCONF_MCSPI2_CS0;              // offset 0x21DC 
    unsigned short CONTROL_PADCONF_MCSPI2_CS1;               
    unsigned short CONTROL_PADCONF_SYS_NIRQ;                // offset 0x21E0
    unsigned short CONTROL_PADCONF_SYS_CLKOUT2;    
    unsigned short CONTROL_PADCONF_SAD2D_MCAD0;             // offset 0x21E4
    unsigned short CONTROL_PADCONF_SAD2D_MCAD1;
    unsigned short CONTROL_PADCONF_SAD2D_MCAD2;             // offset 0x21E8
    unsigned short CONTROL_PADCONF_SAD2D_MCAD3;
    unsigned short CONTROL_PADCONF_SAD2D_MCAD4;             // offset 0x21EC
    unsigned short CONTROL_PADCONF_SAD2D_MCAD5;
    unsigned short CONTROL_PADCONF_SAD2D_MCAD6;             // offset 0x21F0
    unsigned short CONTROL_PADCONF_SAD2D_MCAD7;
    unsigned short CONTROL_PADCONF_SAD2D_MCAD8;             // offset 0x21F4
    unsigned short CONTROL_PADCONF_SAD2D_MCAD9;
    unsigned short CONTROL_PADCONF_SAD2D_MCAD10;            // offset 0x21F8
    unsigned short CONTROL_PADCONF_SAD2D_MCAD11;
    unsigned short CONTROL_PADCONF_SAD2D_MCAD12;            // offset 0x21FC
    unsigned short CONTROL_PADCONF_SAD2D_MCAD13;
    unsigned short CONTROL_PADCONF_SAD2D_MCAD14;            // offset 0x2200
    unsigned short CONTROL_PADCONF_SAD2D_MCAD15;
    unsigned short CONTROL_PADCONF_SAD2D_MCAD16;            // offset 0x2204
    unsigned short CONTROL_PADCONF_SAD2D_MCAD17;
    unsigned short CONTROL_PADCONF_SAD2D_MCAD18;            // offset 0x2208
    unsigned short CONTROL_PADCONF_SAD2D_MCAD19;
    unsigned short CONTROL_PADCONF_SAD2D_MCAD20;            // offset 0x220C
    unsigned short CONTROL_PADCONF_SAD2D_MCAD21;
    unsigned short CONTROL_PADCONF_SAD2D_MCAD22;            // offset 0x2210
    unsigned short CONTROL_PADCONF_SAD2D_MCAD23;
    unsigned short CONTROL_PADCONF_SAD2D_MCAD24;            // offset 0x2214
    unsigned short CONTROL_PADCONF_SAD2D_MCAD25;
    unsigned short CONTROL_PADCONF_SAD2D_MCAD26;            // offset 0x2218
    unsigned short CONTROL_PADCONF_SAD2D_MCAD27;
    unsigned short CONTROL_PADCONF_SAD2D_MCAD28;            // offset 0x221C
    unsigned short CONTROL_PADCONF_SAD2D_MCAD29;
    unsigned short CONTROL_PADCONF_SAD2D_MCAD30;            // offset 0x2220
    unsigned short CONTROL_PADCONF_SAD2D_MCAD31;
    unsigned short CONTROL_PADCONF_SAD2D_MCAD32;            // offset 0x2224
    unsigned short CONTROL_PADCONF_SAD2D_MCAD33;
    unsigned short CONTROL_PADCONF_SAD2D_MCAD34;            // offset 0x2228
    unsigned short CONTROL_PADCONF_SAD2D_MCAD35;
    unsigned short CONTROL_PADCONF_SAD2D_MCAD36;            // offset 0x222C
    unsigned short CONTROL_PADCONF_SAD2D_CLK26MI;
    unsigned short CONTROL_PADCONF_SAD2D_NRESPWRON;         // offset 0x2230
    unsigned short CONTROL_PADCONF_SAD2D_NRESWARM;
    unsigned short CONTROL_PADCONF_SAD2D_ARMNIRQ;           // offset 0x2234
    unsigned short CONTROL_PADCONF_SAD2D_UMAFIQ;
    unsigned short CONTROL_PADCONF_SAD2D_SPINT;             // offset 0x2238
    unsigned short CONTROL_PADCONF_SAD2D_FRINT;
    unsigned short CONTROL_PADCONF_SAD2D_DMAREQ0;           // offset 0x223C
    unsigned short CONTROL_PADCONF_SAD2D_DMAREQ1;
    unsigned short CONTROL_PADCONF_SAD2D_DMAREQ2;           // offset 0x2240
    unsigned short CONTROL_PADCONF_SAD2D_DMAREQ3;
    unsigned short CONTROL_PADCONF_SAD2D_NTRST;             // offset 0x2244
    unsigned short CONTROL_PADCONF_SAD2D_TDI;
    unsigned short CONTROL_PADCONF_SAD2D_TDO;               // offset 0x2248
    unsigned short CONTROL_PADCONF_SAD2D_TMS;
    unsigned short CONTROL_PADCONF_SAD2D_TCK;               // offset 0x224C
    unsigned short CONTROL_PADCONF_SAD2D_RTCK;
    unsigned short CONTROL_PADCONF_SAD2D_MSTDBY;            // offset 0x2250
    unsigned short CONTROL_PADCONF_SAD2D_IDLEREQ;
    unsigned short CONTROL_PADCONF_SAD2D_IDLEACK;           // offset 0x2254
    unsigned short CONTROL_PADCONF_SAD2D_MWRITE;
    unsigned short CONTROL_PADCONF_SAD2D_SWRITE;            // offset 0x2258
    unsigned short CONTROL_PADCONF_SAD2D_MREAD;
    unsigned short CONTROL_PADCONF_SAD2D_SREAD;             // offset 0x225C
    unsigned short CONTROL_PADCONF_SAD2D_MBUSFLAG;
    unsigned short CONTROL_PADCONF_SAD2D_SBUSFLAG;          // offset 0x2260
    unsigned short CONTROL_PADCONF_SDRC_CKE0;
    unsigned short CONTROL_PADCONF_SDRC_CKE1;               // offset 0x2264

    unsigned char  zzzReserved1[0x372];                     // offset 0x2266

    // DM3730 only offset 0x2270-0x2560

    unsigned short CONTROL_PADCONF_ETK_CLK;                 // offset 0x25D8
    unsigned short CONTROL_PADCONF_ETK_CTL;                  
    unsigned short CONTROL_PADCONF_ETK_D0;                  // offset 0x25DC
    unsigned short CONTROL_PADCONF_ETK_D1;                   
    unsigned short CONTROL_PADCONF_ETK_D2;                  // offset 0x25E0
    unsigned short CONTROL_PADCONF_ETK_D3;                   
    unsigned short CONTROL_PADCONF_ETK_D4;                  // offset 0x25E4
    unsigned short CONTROL_PADCONF_ETK_D5;                   
    unsigned short CONTROL_PADCONF_ETK_D6;                  // offset 0x25E8
    unsigned short CONTROL_PADCONF_ETK_D7;                   
    unsigned short CONTROL_PADCONF_ETK_D8;                  // offset 0x25EC 
    unsigned short CONTROL_PADCONF_ETK_D9;                   
    unsigned short CONTROL_PADCONF_ETK_D10;                 // offset 0x25F0 
    unsigned short CONTROL_PADCONF_ETK_D11;                  
    unsigned short CONTROL_PADCONF_ETK_D12;                 // offset 0x25F4
    unsigned short CONTROL_PADCONF_ETK_D13;                  
    unsigned short CONTROL_PADCONF_ETK_D14;                 // offset 0x25F8
    unsigned short CONTROL_PADCONF_ETK_D15;                  

    unsigned char  zzzReserved2[1028];

    // remaining are wake-up control module pad config (also in OMAP_SYSC_PADCONFS_WKUP_REGS)
    unsigned short CONTROL_PADCONF_I2C4_SCL;                // offset 0x2A00
    unsigned short CONTROL_PADCONF_I2C4_SDA;                // offset 0x2A02
    unsigned short CONTROL_PADCONF_SYS_32K;                 // offset 0x2A04
    unsigned short CONTROL_PADCONF_SYS_CLKREQ;              // offset 0x2A06
    unsigned short CONTROL_PADCONF_SYS_NRESWARM;            // offset 0x2A08
    unsigned short CONTROL_PADCONF_SYS_BOOT0;               // offset 0x2A0A
    unsigned short CONTROL_PADCONF_SYS_BOOT1;               // offset 0x2A0C
    unsigned short CONTROL_PADCONF_SYS_BOOT2;               // offset 0x2A0E
    unsigned short CONTROL_PADCONF_SYS_BOOT3;               // offset 0x2A10
    unsigned short CONTROL_PADCONF_SYS_BOOT4;               // offset 0x2A12
    unsigned short CONTROL_PADCONF_SYS_BOOT5;               // offset 0x2A14
    unsigned short CONTROL_PADCONF_SYS_BOOT6;               // offset 0x2A16
    unsigned short CONTROL_PADCONF_SYS_OFF_MODE;            // offset 0x2A18
    unsigned short CONTROL_PADCONF_SYS_CLKOUT1;             // offset 0x2A1A
    unsigned short CONTROL_PADCONF_JTAG_NTRST;              // offset 0x2A1C
    unsigned short CONTROL_PADCONF_JTAG_TCLK;               // offset 0x2A1E
    unsigned short CONTROL_PADCONF_JTAG_TMS_TMSC;           // offset 0x2A20
    unsigned short CONTROL_PADCONF_JTAG_TDI;                // offset 0x2A22
    unsigned short CONTROL_PADCONF_JTAG_EMU0;               // offset 0x2A24
    unsigned short CONTROL_PADCONF_JTAG_EMU1;               // offset 0x2A26
	
    unsigned char  zzzReserved3[36];

    unsigned short CONTROL_PADCONF_SAD2D_SWAKEUP;           // offset 0x2A4C
    unsigned short CONTROL_PADCONF_JTAG_RTCK;               // offset 0x2A4E
    unsigned short CONTROL_PADCONF_JTAG_TDO;                // offset 0x2A50

    unsigned char  zzzReserved4[2];                         // offset 0x2A52

    // DM3730 only (also in OMAP_SYSC_PADCONFS_WKUP_REGS)
    unsigned short CONTROL_PADCONF_GPIO127;                 // offset 0x2A54
    unsigned short CONTROL_PADCONF_GPIO126;                 // offset 0x2A56
    unsigned short CONTROL_PADCONF_GPIO128;                 // offset 0x2A58
    unsigned short CONTROL_PADCONF_GPIO129;                 // offset 0x2A5A
} OMAP_SYSC_PADCONFS_REGS;


// OMAP_SYSC_GENERAL_REGS
//
typedef volatile struct {
    unsigned long CONTROL_PADCONF_OFF;                      // offset 0x0000 
    unsigned long CONTROL_DEVCONF0;                         // offset 0x0004 
    unsigned long CONTROL_MEM_DFTRW0;                       // offset 0x0008 
    unsigned long CONTROL_MEM_DFTRW1;                       // offset 0x000C 

    unsigned long zzzReserved01[4];

    unsigned long CONTROL_MSUSPENDMUX_0;                    // offset 0x0020 
    unsigned long CONTROL_MSUSPENDMUX_1;                    // offset 0x0024 
    unsigned long CONTROL_MSUSPENDMUX_2;                    // offset 0x0028 
    unsigned long CONTROL_MSUSPENDMUX_3;                    // offset 0x002C 
    unsigned long CONTROL_MSUSPENDMUX_4;                    // offset 0x0030 
    unsigned long CONTROL_MSUSPENDMUX_5;                    // offset 0x0034 

    unsigned long zzzReserved02[2];

    unsigned long CONTROL_SEC_CTRL;                         // offset 0x0040 

    unsigned long zzzReserved03[9];

    unsigned long CONTROL_DEVCONF1;                         // offset 0x0068 
    unsigned long CONTROL_CSIRXFE;                          // offset 0x006C 
    unsigned long CONTROL_SEC_STATUS;                       // offset 0x0070 
    unsigned long CONTROL_SEC_ERR_STATUS;                   // offset 0x0074 
    unsigned long CONTROL_SEC_ERR_STATUS_DEBUG;             // offset 0x0078 

    unsigned long zzzReserved04[1];

    unsigned long CONTROL_STATUS;                           // offset 0x0080 
    unsigned long CONTROL_GENERAL_PURPOSE_STATUS;           // offset 0x0084 

    unsigned long zzzReserved05[2];

    unsigned long CONTROL_RPUB_KEY_H_0;                     // offset 0x0090 
    unsigned long CONTROL_RPUB_KEY_H_1;                     // offset 0x0094 
    unsigned long CONTROL_RPUB_KEY_H_2;                     // offset 0x0098 
    unsigned long CONTROL_RPUB_KEY_H_3;                     // offset 0x009C 
    unsigned long CONTROL_RPUB_KEY_H_4;                     // offset 0x00A0

    unsigned long zzzReserved06[1];
    
    unsigned long CONTROL_RAND_KEY_0;                       // offset 0x00A8
    unsigned long CONTROL_RAND_KEY_1;                       // offset 0x00AC 
    unsigned long CONTROL_RAND_KEY_2;                       // offset 0x00B0 
    unsigned long CONTROL_RAND_KEY_3;                       // offset 0x00B4 
    
    unsigned long CONTROL_CUST_KEY_0;                       // offset 0x00B8 
    unsigned long CONTROL_CUST_KEY_1;                       // offset 0x00BC 
    unsigned long CONTROL_CUST_KEY_2;                       // offset 0x00C0 
    unsigned long CONTROL_CUST_KEY_3;                       // offset 0x00C4

    unsigned long zzzReserved07[14];                        // offset 0x00C8

    unsigned long CONTROL_USB_CONF_0;                       // offset 0x0100
    unsigned long CONTROL_USB_CONF_1;                       // offset 0x0104

    unsigned long zzzReserved08[2];                         // offset 0x0108

    unsigned long CONTROL_FUSE_OPP1_VDD1;                   // offset 0x0110
    unsigned long CONTROL_FUSE_OPP2_VDD1;                   // offset 0x0114
    unsigned long CONTROL_FUSE_OPP3_VDD1;                   // offset 0x0118
    unsigned long CONTROL_FUSE_OPP4_VDD1;                   // offset 0x011C
    unsigned long CONTROL_FUSE_OPP5_VDD1;                   // offset 0x0120
    unsigned long CONTROL_FUSE_OPP1_VDD2;                   // offset 0x0124
    unsigned long CONTROL_FUSE_OPP2_VDD2;                   // offset 0x0128
    unsigned long CONTROL_FUSE_OPP3_VDD2;                   // offset 0x012C
    unsigned long CONTROL_FUSE_SR;                          // offset 0x0130
    unsigned long CONTROL_CEK_0;                            // offset 0x0134
    unsigned long CONTROL_CEK_1;                            // offset 0x0138
    unsigned long CONTROL_CEK_2;                            // offset 0x013C
    unsigned long CONTROL_CEK_3;                            // offset 0x0140
    unsigned long CONTROL_MSV_0;                            // offset 0x0144
    unsigned long CONTROL_CEK_BCH_0;                        // offset 0x0148
    unsigned long CONTROL_CEK_BCH_1;                        // offset 0x014C
    unsigned long CONTROL_CEK_BCH_2;                        // offset 0x0150
    unsigned long CONTROL_CEK_BCH_3;                        // offset 0x0154
    unsigned long CONTROL_CEK_BCH_4;                        // offset 0x0158
    unsigned long CONTROL_MSV_BCH_0;                        // offset 0x015C
    unsigned long CONTROL_MSV_BCH_1;                        // offset 0x0160
    unsigned long CONTROL_SWRV_0;                           // offset 0x0164
    unsigned long CONTROL_SWRV_1;                           // offset 0x0168
    unsigned long CONTROL_SWRV_2;                           // offset 0x016C
    unsigned long CONTROL_SWRV_3;                           // offset 0x0170
    unsigned long CONTROL_SWRV_4;                           // offset 0x0174

    unsigned long zzzReserved09[6];                         // offset 0x0178

    unsigned long CONTROL_IVA2_BOOTADDR;                    // offset 0x0190
    unsigned long CONTROL_IVA2_BOOTMOD;                     // offset 0x0194

    unsigned long CONTROL_PROG_IO2;                         // offset 0x0198 - 3630 specific

    unsigned long CONTROL_MEM_RTA_CTRL;                     // offset 0x019C - 3630 specific

    unsigned long zzzReserved10[4];                         // offset 0x01A0

    unsigned long CONTROL_DEBOBS_0;                         // offset 0x01B0
    unsigned long CONTROL_DEBOBS_1;                         // offset 0x01B4
    unsigned long CONTROL_DEBOBS_2;                         // offset 0x01B8
    unsigned long CONTROL_DEBOBS_3;                         // offset 0x01BC
    unsigned long CONTROL_DEBOBS_4;                         // offset 0x01C0
    unsigned long CONTROL_DEBOBS_5;                         // offset 0x01C4
    unsigned long CONTROL_DEBOBS_6;                         // offset 0x01C8
    unsigned long CONTROL_DEBOBS_7;                         // offset 0x01CC
    unsigned long CONTROL_DEBOBS_8;                         // offset 0x01D0
    unsigned long CONTROL_PROG_IO0;                         // offset 0x01D4
    unsigned long CONTROL_PROG_IO1;                         // offset 0x01D8

    unsigned long zzzReserved11[1];                         // offset 0x01DC

    unsigned long CONTROL_DSS_DPLL_SPREADING;               // offset 0x01E0
    unsigned long CONTROL_CORE_DPLL_SPREADING;              // offset 0x01E4
    unsigned long CONTROL_PER_DPLL_SPREADING;               // offset 0x01E8
    unsigned long CONTROL_USBHOST_DPLL_SPREADING;           // offset 0x01EC
    unsigned long CONTROL_SECURE_SDRC_SHARING;              // offset 0x01F0
    unsigned long CONTROL_SECURE_SDRC_MCFG0;                // offset 0x01F4
    unsigned long CONTROL_SECURE_SDRC_MCFG1;                // offset 0x01F8
    unsigned long CONTROL_MODEM_FW_CONFIGURATION_LOCK;      // offset 0x01FC
    unsigned long CONTROL_MODEM_MEMORY_RESOURCES_CONF;      // offset 0x0200
    unsigned long CONTROL_MODEM_GPMC_DT_FW_REQ_INFO;        // offset 0x0204
    unsigned long CONTROL_MODEM_GPMC_DT_FW_RD;              // offset 0x0208
    unsigned long CONTROL_MODEM_GPMC_DT_FW_WR;              // offset 0x020C
    unsigned long CONTROL_MODEM_GPMC_BOOT_CODE;             // offset 0x0210
    unsigned long CONTROL_MODEM_SMS_RG_ATT1;                // offset 0x0214
    unsigned long CONTROL_MODEM_SMS_RG_RDPERM1;             // offset 0x0218
    unsigned long CONTROL_MODEM_SMS_RG_WRPERM1;             // offset 0x021C
    unsigned long CONTROL_MODEM_D2D_FW_DEBUG_MODE;          // offset 0x0220

    unsigned long zzzReserved12[1];                         // offset 0x0224

    unsigned long CONTROL_DPF_OCM_RAM_FW_ADDR_MATCH;        // offset 0x0228
    unsigned long CONTROL_DPF_OCM_RAM_FW_REQINFO;           // offset 0x022C
    unsigned long CONTROL_DPF_OCM_RAM_FW_WR;                // offset 0x0230
    unsigned long CONTROL_DPF_REGION4_GPMC_FW_ADDR_MATCH;   // offset 0x0234
    unsigned long CONTROL_DPF_REGION4_GPMC_FW_REQINFO;      // offset 0x0238
    unsigned long CONTROL_DPF_REGION4_GPMC_FW_WR;           // offset 0x023C
    unsigned long CONTROL_DPF_REGION1_IVA2_FW_ADDR_MATCH;   // offset 0x0240
    unsigned long CONTROL_DPF_REGION1_IVA2_FW_REQINFO;      // offset 0x0244
    unsigned long CONTROL_DPF_REGION1_IVA2_FW_WR;           // offset 0x0248
    unsigned long CONTROL_APE_FW_DEFAULT_SECURE_LOCK;       // offset 0x024C
    unsigned long CONTROL_OCMROM_SECURE_DEBUG;              // offset 0x0250
    unsigned long CONTROL_D2D_FW_STACKED_DEVICE_SEC_DEBUG;  // offset 0x0254

    unsigned long zzzReserved13[3];                         // offset 0x0258

    unsigned long CONTROL_EXT_SEC_CONTROL;                  // offset 0x0264
    unsigned long CONTROL_D2D_FW_STACKED_DEVICE_SECURITY;   // offset 0x0268

    unsigned long zzzReserved14[17];                        // offset 0x026C

    unsigned long CONTROL_PBIAS_LITE;                       // offset 0x02B0
    unsigned long CONTROL_TEMP_SENSOR;                      // offset 0x02B4
    unsigned long CONTROL_SRAMLDO4;                         // offset 0x02B8
    unsigned long CONTROL_SRAMLDO5;                         // offset 0x02BC
    unsigned long CONTROL_CSI;                              // offset 0x02C0

    unsigned long zzzReserved15[1];                         // offset 0x02C4

    unsigned long CONTROL_DPF_MAD2D_FW_ADDR_MATCH;          // offset 0x02C8
    unsigned long CONTROL_DPF_MAD2D_FW_REQINFO;             // offset 0x02CC
    unsigned long CONTROL_DPF_MAD2D_FW_WR;                  // offset 0x02D0
} OMAP_SYSC_GENERAL_REGS;


// OMAP_IDCORE_REGS
//
typedef volatile struct {

    unsigned long  IDCODE;                                  // offset 0x0204
    unsigned long  PRODUCTION_ID_0;                         // offset 0x0208
    unsigned long  PRODUCTION_ID_1;                         // offset 0x020C
    unsigned long  PRODUCTION_ID_2;                         // offset 0x0210
    unsigned long  PRODUCTION_ID_3;                         // offset 0x0214
    unsigned long  DIE_ID_0;                                // offset 0x0218
    unsigned long  DIE_ID_1;                                // offset 0x021C
    unsigned long  DIE_ID_2;                                // offset 0x0220
    unsigned long  DIE_ID_3;                                // offset 0x0224
} OMAP_IDCORE_REGS;


// OMAP_SYSC_PADCONFS_WKUP_REGS
//
typedef volatile struct {
    unsigned short  CONTROL_PADCONF_I2C4_SCL;               // offset 0x0000
    unsigned short  CONTROL_PADCONF_I2C4_SDA;
    unsigned short  CONTROL_PADCONF_SYS_32K;                // offset 0x0004
    unsigned short  CONTROL_PADCONF_SYS_CLKREQ;
    unsigned short  CONTROL_PADCONF_SYS_NRESWARM;           // offset 0x0008
    unsigned short  CONTROL_PADCONF_SYS_BOOT0;
    unsigned short  CONTROL_PADCONF_SYS_BOOT1;              // offset 0x000C
    unsigned short  CONTROL_PADCONF_SYS_BOOT2;
    unsigned short  CONTROL_PADCONF_SYS_BOOT3;              // offset 0x0010
    unsigned short  CONTROL_PADCONF_SYS_BOOT4;      
    unsigned short  CONTROL_PADCONF_SYS_BOOT5;              // offset 0x0014
    unsigned short  CONTROL_PADCONF_SYS_BOOT6;
    unsigned short  CONTROL_PADCONF_SYS_OFF_MODE;           // offset 0x0018
    unsigned short  CONTROL_PADCONF_SYS_CLKOUT1;
    unsigned short  CONTROL_PADCONF_JTAG_NTRST;             // offset 0x001C
    unsigned short  CONTROL_PADCONF_JTAG_TCK;
    unsigned short  CONTROL_PADCONF_JTAG_TMS;               // offset 0x0020
    unsigned short  CONTROL_PADCONF_JTAG_TDI;
    unsigned short  CONTROL_PADCONF_JTAG_EMU0;              // offset 0x0024
    unsigned short  CONTROL_PADCONF_JTAG_EMU1;

    unsigned short  zzzReserved01[18];                      // offset 0x0028

    unsigned short  CONTROL_PADCONF_SAD2D_SWAKEUP;          // offset 0x004C
    unsigned short  CONTROL_PADCONF_JTAG_TDO;               // offset 0x0050

    unsigned char  zzzReserved4[2];                         // offset 0x0052

    // DM3730 only
    unsigned short CONTROL_PADCONF_GPIO127;                 // offset 0x0054
    unsigned short CONTROL_PADCONF_GPIO126;                 // offset 0x0056
    unsigned short CONTROL_PADCONF_GPIO128;                 // offset 0x0058
    unsigned short CONTROL_PADCONF_GPIO129;                 // offset 0x005A
} OMAP_SYSC_PADCONFS_WKUP_REGS;

// OMAP_SYSC_GENERAL_WKUP_REGS
//
typedef volatile struct {

    unsigned long CONTROL_SEC_TAP;                          // offset 0x0000 (0x48002A60)
    unsigned long CONTROL_SEC_EMU;                          // offset 0x0004
    unsigned long CONTROL_WKUP_DEBOBS_0;                    // offset 0x0008
    unsigned long CONTROL_WKUP_DEBOBS_1;                    // offset 0x000C
    unsigned long CONTROL_WKUP_DEBOBS_2;                    // offset 0x0010
    unsigned long CONTROL_WKUP_DEBOBS_3;                    // offset 0x0014
    unsigned long CONTROL_WKUP_DEBOBS_4;                    // offset 0x0018
    unsigned long CONTROL_SEC_DAP;                          // offset 0x001C
} OMAP_SYSC_GENERAL_WKUP_REGS;

// OMAP_SYSC_GENERAL_WKUP_REGS_DM3730
//
typedef volatile struct {
    unsigned long  CONTROL_WKUP_CTRL;                       // offset 0x0000 (0x48002A5C)
    unsigned char  zzzReserved5[8];                         // offset 0x0004 (0x48002A60)
    unsigned long CONTROL_WKUP_DEBOBS_0;                    // offset 0x000C (0x48002A68)
    unsigned long CONTROL_WKUP_DEBOBS_1;                    // offset 0x0010 (0x48002A6C)
    unsigned long CONTROL_WKUP_DEBOBS_2;                    // offset 0x0014 (0x48002A70)
    unsigned long CONTROL_WKUP_DEBOBS_3;                    // offset 0x0018 (0x48002A74)
    unsigned long CONTROL_WKUP_DEBOBS_4;                    // offset 0x001C (0x48002A78)
    unsigned char  zzzReserved6[4];                         // offset 0x0020 (0x48002A7C)
    unsigned long CONTROL_PROG_IO_WKUP1;                    // offset 0x0024 (0x48002A80)
    unsigned long CONTROL_BGAPTS_WKUP;                      // offset 0x0028 (0x48002A84)
    unsigned long CONTROL_SRAM_LDO_CTRL;                    // offset 0x002C (0x48002A88)
    unsigned long CONTROL_VBBLDO_EFUSE_CTRL;                // offset 0x0030 (0x48002A8C)
    unsigned long CONTROL_VBBLDO_SW_CTRL;                   // offset 0x0034 (0x48002A90)
} OMAP_SYSC_GENERAL_WKUP_REGS_DM3730;

//------------------------------------------------------------------------------
#define PULL_UP_DOWN_MASK               (0x3 << 3)
#define PULL_INACTIVE                   (0 << 3)
#define PULL_DOWN                       (1 << 3)
#define PULL_UP                         (3 << 3)

#define MUX_MODE_0                      (0 << 0)
#define MUX_MODE_1                      (1 << 0)
#define MUX_MODE_2                      (2 << 0)
#define MUX_MODE_3                      (3 << 0)
#define MUX_MODE_4                      (4 << 0)
#define MUX_MODE_5                      (5 << 0)
#define MUX_MODE_6                      (6 << 0)
#define MUX_MODE_7                      (7 << 0)

#define INPUT_ENABLE                    (1 << 8)
#define INPUT_DISABLE                   (0 << 8)

#define OFF_ENABLE                      (1 << 9)
#define OFF_DISABLE                     (0 << 9)

// OFFOUT_ENABLE is a active low bit
#define OFFOUT_ENABLE                   (0 << 10)
#define OFFOUT_DISABLE                  (1 << 10)

#define OFFOUTVALUE_MASK                (1 << 11)
#define OFFOUTVALUE_HIGH                (1 << 11)
#define OFFOUTVALUE_LOW                 (0 << 11)

#define OFFPULLUP_MASK                  (1 << 12)
#define OFFPULLUD_ENABLE                (1 << 12)
#define OFFPULLUD_DISABLE               (0 << 12)

#define OFF_PULL_MASK                   (1 << 13)
#define OFF_PULLUP                      (1 << 13)
#define OFF_PULLDOWN                    (0 << 13)


// defines IO power optimised value for the PADS which are not used
#define PADCONF_PIN_NOT_USED            (OFF_ENABLE | OFFOUT_DISABLE | OFFPULLUD_ENABLE | \
                                         INPUT_ENABLE | PULL_DOWN | MUX_MODE_7)

#define OFF_MODE_NOT_SUPPORTED          (0 << 9)
#define OFF_WAKE_ENABLE                 (1 << 14)

#define OFF_PAD_WAKEUP_EVENT            (1 << 15)

#define OFF_INPUT_PULL_UP               (OFF_ENABLE | OFFOUT_DISABLE | OFFPULLUD_ENABLE | \
                                         OFF_PULLUP)
#define OFF_INPUT_PULL_DOWN             (OFF_ENABLE | OFFOUT_DISABLE | OFFPULLUD_ENABLE | \
                                         OFF_PULLDOWN)
#define OFF_INPUT_PULL_INACTIVE         (OFF_ENABLE | OFFOUT_DISABLE | OFFPULLUD_DISABLE)

#define OFF_OUTPUT_PULL_UP              (OFF_ENABLE | OFFOUT_ENABLE | OFFPULLUD_ENABLE | \
                                         OFF_PULLUP)
#define OFF_OUTPUT_PULL_DOWN            (OFF_ENABLE | OFFOUT_ENABLE| OFFPULLUD_ENABLE | \
                                         OFF_PULLDOWN)
#define OFF_OUTPUT_PULL_INACTIVE        (OFF_ENABLE | OFFOUT_ENABLE | OFFPULLUD_DISABLE)


// CONTROL_PADCONF_OFF register macros
#define STARTSAVE                       (1 << 1)

// CONTROL_GENERAL_PURPOSE_STATUS 
#define SAVEDONE                        (1 << 0)
#define RNGIDLE                         (1 << 31)

// CONTROL_DEVCONF0 bits
#define SENSDMAREQ0                     (1 << 0)
#define SENSDMAREQ1                     (1 << 1)
#define MCBSP1_CLKS                     (1 << 2)
#define MCBSP1_CLKR                     (1 << 3)
#define MCBSP1_FSR                      (1 << 4)
#define MCBSP2_CLKS                     (1 << 6)
#define MMCSDIO1ADPCLKISEL              (1 << 24)

// CONTROL_DEVCONF1 bits
#define MCBSP3_CLKS                     (1 << 0)
#define MCBSP4_CLKS                     (1 << 2)
#define MCBSP5_CLKS                     (1 << 4)
#define MMCSDIO2ADPCLKISEL              (1 << 6)
#define SENSDMAREQ2                     (1 << 7)
#define SENSDMAREQ3                     (1 << 8)
#define MPUFORCEWRNP                    (1 << 9)
#define TVACEN                          (1 << 11)
#define I2C1HSMASTER                    (1 << 12)
#define I2C2HSMASTER                    (1 << 13)
#define I2C3HSMASTER                    (1 << 14)
#define TVOUTBYPASS                     (1 << 18)
#define CARKITHSUSB0DATA0AUTOEN         (1 << 19)
#define CARKITHSUSB0DATA1AUTOEN         (1 << 20)
#define SENSDMAREQ4                     (1 << 21)
#define SENSDMAREQ5                     (1 << 22)
#define SENSDMAREQ6                     (1 << 23)

// CONTROL_PBIAS_LITE bits
#define PBIASLITEVMODE0                 (1 << 0)
#define PBIASLITEPWRDNZ0                (1 << 1)
#define PBIASSPEEDCTRL0                 (1 << 2)
#define PBIASLITEVMODEERROR0            (1 << 3)
#define PBIASLITESUPPLYHIGH0            (1 << 7)
#define PBIASLITEVMODE1                 (1 << 8)
#define PBIASLITEPWRDNZ1                (1 << 9)
#define PBIASSPEEDCTRL1                 (1 << 10)
#define PBIASLITEVMODEERROR1            (1 << 11)
#define PBIASLITESUPPLYHIGH1            (1 << 15)

// CONTROL_PROG_IO1 (DM3730 only)
#define PRG_SDMMC1_SPEEDCTRL            (1 << 20)
#define PRG_I2C1_PULLUPRESX             (1 << 19)
#define PRG_SR_PULLUPRESX               (1 << 5)
#define PRG_MCSPI1_CS2_LB               (1 << 4)
#define PRG_MCSPI1_CS3_LB               (1 << 3)
#define PRG_MCSPI2_LB                   (1 << 2)
#define PRG_UART2_LB                    (1 << 1)
#define PRG_I2C2_PULLUPRESX             (1 << 0)

// CONTROL_BGAPTS
#define PRG_BGPOFF_LB                   (0 << 2)

// CONTROL_PROG_IO2 - 3630 specific
#define PRG_MCSPI1_MIN_CFG_LB           (1 << 1)
#define PRG_MCSPI1_CS1_LB               (1 << 0)

#endif // __OMAP35XX_CONFIG_H


