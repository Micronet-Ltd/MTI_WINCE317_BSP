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

//------------------------------------------------------------------------------
//
//  File:  am3517_config.h
//
//  This header file is comprised of control pad configuration module register
//  details defined as structures and macros for configuring the pads.
//

#ifndef __OMAP3517_CONFIG_H
#define __OMAP3517_CONFIG_H

//------------------------------------------------------------------------------
// System PAD Configuration Registers

// OMAP_SYSC_INTERFACE_REGS
//
typedef volatile struct {

    REG32 CONTROL_REVISION;                         // offset 0x0000 
    REG32 zzzReserved01[3];
    REG32 CONTROL_SYSCONFIG;                        // offset 0x0010 

} OMAP_SYSC_INTERFACE_REGS;

#define SAFE_MODE                       (7)
#define RELEASED_PAD_DEFAULT_CONFIG     (INPUT_ENABLED | PULL_RESISTOR_DISABLED | MUXMODE(SAFE_MODE))
#define UNUSED_PAD_DEFAULT_CONFIG       RELEASED_PAD_DEFAULT_CONFIG


#define PAD_ID(x) (offsetof(OMAP_SYSC_PADCONFS_REGS,CONTROL_PADCONF_##x)/sizeof(UINT16))
#define FIRST_WKUP_PAD_INDEX (sizeof(OMAP_SYSC_PADCONFS_REGS)/sizeof(UINT16))
#define WKUP_PAD_ID(x) (offsetof(OMAP_SYSC_PADCONFS_WKUP_REGS,CONTROL_PADCONF_##x)/sizeof(UINT16) + FIRST_WKUP_PAD_INDEX)


// OMAP_SYSC_PADCONFS_REGS
//
typedef volatile struct {
    REG16 CONTROL_PADCONF_SDRC_D0		;                                         // offset 0x2030
    REG16 CONTROL_PADCONF_SDRC_D1		;                                         // 
    REG16 CONTROL_PADCONF_SDRC_D2 		;                                         // offset 0x2034
    REG16 CONTROL_PADCONF_SDRC_D3 		;                                         // 
    REG16 CONTROL_PADCONF_SDRC_D4 		;                                         // offset 0x2038
    REG16 CONTROL_PADCONF_SDRC_D5 		;                                         // 
    REG16 CONTROL_PADCONF_SDRC_D6 		;                                         // offset 0x203C
    REG16 CONTROL_PADCONF_SDRC_D7 		;                                         // 
    REG16 CONTROL_PADCONF_SDRC_D8 		;                                         // offset 0x2040
    REG16 CONTROL_PADCONF_SDRC_D9 		;                                         // 
    REG16 CONTROL_PADCONF_SDRC_D10 		;                                         // offset 0x2044
    REG16 CONTROL_PADCONF_SDRC_D11 		;                                         // 
    REG16 CONTROL_PADCONF_SDRC_D12 		;                                         // offset 0x2048
    REG16 CONTROL_PADCONF_SDRC_D13 		;                                         // 
    REG16 CONTROL_PADCONF_SDRC_D14 		;                                         // offset 0x204C
    REG16 CONTROL_PADCONF_SDRC_D15 		;                                         // 
    REG16 CONTROL_PADCONF_SDRC_D16 		;                                         // offset 0x2050
    REG16 CONTROL_PADCONF_SDRC_D17 		;                                         // 
    REG16 CONTROL_PADCONF_SDRC_D18 		;                                         // offset 0x2054
    REG16 CONTROL_PADCONF_SDRC_D19 		;                                         // 
    REG16 CONTROL_PADCONF_SDRC_D20 		;                                         // offset 0x2058
    REG16 CONTROL_PADCONF_SDRC_D21 		;                                         // 
    REG16 CONTROL_PADCONF_SDRC_D22 		;                                         // offset 0x205C
    REG16 CONTROL_PADCONF_SDRC_D23 		;                                         // 
    REG16 CONTROL_PADCONF_SDRC_D24 		;                                         // offset 0x2060
    REG16 CONTROL_PADCONF_SDRC_D25 		;                                         // 
    REG16 CONTROL_PADCONF_SDRC_D26 		;                                         // offset 0x2064
    REG16 CONTROL_PADCONF_SDRC_D27 		;                                         // 
    REG16 CONTROL_PADCONF_SDRC_D28 		;                                         // offset 0x2068
    REG16 CONTROL_PADCONF_SDRC_D29 		;                                         // 
    REG16 CONTROL_PADCONF_SDRC_D30		;                                         // offset 0x206C
    REG16 CONTROL_PADCONF_SDRC_D31		;                                         // 
    REG16 CONTROL_PADCONF_SDRC_CLK		;                                         // offset 0x2070
    REG16 CONTROL_PADCONF_SDRC_DQS0		;                                         // 
    REG16 CONTROL_PADCONF_SDRC_DQS1		;                                         // offset 0x2074
    REG16 CONTROL_PADCONF_SDRC_DQS2		;                                         // 
    REG16 CONTROL_PADCONF_SDRC_DQS3		;                                         // offset 0x2078
    REG16 CONTROL_PADCONF_GPMC_A1 		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_A2 		;                                         // offset 0x207C
    REG16 CONTROL_PADCONF_GPMC_A3 		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_A4 		;                                         // offset 0x2080
    REG16 CONTROL_PADCONF_GPMC_A5 		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_A6 		;                                         // offset 0x2084
    REG16 CONTROL_PADCONF_GPMC_A7 		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_A8 		;                                         // offset 0x2088
    REG16 CONTROL_PADCONF_GPMC_A9 		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_A10		;                                         // offset 0x208C
    REG16 CONTROL_PADCONF_GPMC_D0 		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_D1 		;                                         // offset 0x2090
    REG16 CONTROL_PADCONF_GPMC_D2 		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_D3 		;                                         // offset 0x2094
    REG16 CONTROL_PADCONF_GPMC_D4 		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_D5 		;                                         // offset 0x2098
    REG16 CONTROL_PADCONF_GPMC_D6 		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_D7 		;                                         // offset 0x209C
    REG16 CONTROL_PADCONF_GPMC_D8 		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_D9 		;                                         // offset 0x20A0
    REG16 CONTROL_PADCONF_GPMC_D10 		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_D11 		;                                         // offset 0x20A4
    REG16 CONTROL_PADCONF_GPMC_D12 		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_D13 		;                                         // offset 0x20A8
    REG16 CONTROL_PADCONF_GPMC_D14 		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_D15 		;                                         // offset 0x20AC
    REG16 CONTROL_PADCONF_GPMC_nCS0		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_nCS1		;                                         // offset 0x20B0
    REG16 CONTROL_PADCONF_GPMC_nCS2		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_nCS3		;                                         // offset 0x20B4
    REG16 CONTROL_PADCONF_GPMC_nCS4		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_nCS5		;                                         // offset 0x20B8
    REG16 CONTROL_PADCONF_GPMC_nCS6		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_nCS7		;                                         // offset 0x20BC
    REG16 CONTROL_PADCONF_GPMC_CLK		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_nADV_ALE	;                                         // offset 0x20C0
    REG16 CONTROL_PADCONF_GPMC_nOE		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_nWE		;                                         // offset 0x20C4
    REG16 CONTROL_PADCONF_GPMC_nBE0_CLE	;                                         // 
    REG16 CONTROL_PADCONF_GPMC_nBE1		;                                         // offset 0x20C8
    REG16 CONTROL_PADCONF_GPMC_nWP		;                                         // 
    REG16 CONTROL_PADCONF_GPMC_WAIT0	;                                         // offset 0x20CC
    REG16 CONTROL_PADCONF_GPMC_WAIT1	;                                         // 
    REG16 CONTROL_PADCONF_GPMC_WAIT2	;                                         // offset 0x20D0
    REG16 CONTROL_PADCONF_GPMC_WAIT3	;                                         // 
    REG16 CONTROL_PADCONF_DSS_PCLK		;                                         // offset 0x20D4
    REG16 CONTROL_PADCONF_DSS_HSYNC		;                                         // 
    REG16 CONTROL_PADCONF_DSS_VSYNC		;                                         // offset 0x20D8
    REG16 CONTROL_PADCONF_DSS_ACBIAS	;                                         // 
    REG16 CONTROL_PADCONF_DSS_DATA0 	;                                         // offset 0x20DC
    REG16 CONTROL_PADCONF_DSS_DATA1 	;                                         // 
    REG16 CONTROL_PADCONF_DSS_DATA2 	;                                         // offset 0x20E0
    REG16 CONTROL_PADCONF_DSS_DATA3 	;                                         // 
    REG16 CONTROL_PADCONF_DSS_DATA4 	;                                         // offset 0x20E4
    REG16 CONTROL_PADCONF_DSS_DATA5 	;                                         // 
    REG16 CONTROL_PADCONF_DSS_DATA6 	;                                         // offset 0x20E8
    REG16 CONTROL_PADCONF_DSS_DATA7 	;                                         // 
    REG16 CONTROL_PADCONF_DSS_DATA8 	;                                         // offset 0x20EC
    REG16 CONTROL_PADCONF_DSS_DATA9 	;                                         //
    REG16 CONTROL_PADCONF_DSS_DATA10	;                                         // offset 0x20F0
    REG16 CONTROL_PADCONF_DSS_DATA11	;                                         //
    REG16 CONTROL_PADCONF_DSS_DATA12	;                                         // offset 0x20F4
    REG16 CONTROL_PADCONF_DSS_DATA13	;                                         //
    REG16 CONTROL_PADCONF_DSS_DATA14	;                                         // offset 0x20F8
    REG16 CONTROL_PADCONF_DSS_DATA15	;                                         //
    REG16 CONTROL_PADCONF_DSS_DATA16	;                                         // offset 0x20FC
    REG16 CONTROL_PADCONF_DSS_DATA17	;                                         //
    REG16 CONTROL_PADCONF_DSS_DATA18	;                                         // offset 0x2100
    REG16 CONTROL_PADCONF_DSS_DATA19	;                                         //
    REG16 CONTROL_PADCONF_DSS_DATA20	;                                         // offset 0x2104
    REG16 CONTROL_PADCONF_DSS_DATA21	;                                         //
    REG16 CONTROL_PADCONF_DSS_DATA22	;                                         // offset 0x2108
    REG16 CONTROL_PADCONF_DSS_DATA23	;                                         //

    unsigned char reserved1[0x30];												  //---------------
    
	REG16 CONTROL_PADCONF_MCBSP2_FSX	;                                         // offset 0x213C
    REG16 CONTROL_PADCONF_MCBSP2_CLKX	;                                         //
    REG16 CONTROL_PADCONF_MCBSP2_DR		;                                         // offset 0x2140
    REG16 CONTROL_PADCONF_MCBSP2_DX		;                                         //
    REG16 CONTROL_PADCONF_MMC1_CLK 		;                                         // offset 0x2144
    REG16 CONTROL_PADCONF_MMC1_CMD 		;                                         //
    REG16 CONTROL_PADCONF_MMC1_DAT0 	;                                         // offset 0x2148
    REG16 CONTROL_PADCONF_MMC1_DAT1 	;                                         //
    REG16 CONTROL_PADCONF_MMC1_DAT2 	;                                         // offset 0x214C
    REG16 CONTROL_PADCONF_MMC1_DAT3 	;                                         //
    REG16 CONTROL_PADCONF_MMC1_DAT4 	;                                         // offset 0x2150
    REG16 CONTROL_PADCONF_MMC1_DAT5 	;                                         //
    REG16 CONTROL_PADCONF_MMC1_DAT6 	;                                         // offset 0x2154
    REG16 CONTROL_PADCONF_MMC1_DAT7 	;                                         //
    REG16 CONTROL_PADCONF_MMC2_CLK		;                                         // offset 0x2158
    REG16 CONTROL_PADCONF_MMC2_CMD		;                                         //
    REG16 CONTROL_PADCONF_MMC2_DAT0 	;                                         // offset 0x215C
    REG16 CONTROL_PADCONF_MMC2_DAT1 	;                                         //
    REG16 CONTROL_PADCONF_MMC2_DAT2 	;                                         // offset 0x2160
    REG16 CONTROL_PADCONF_MMC2_DAT3 	;                                         //
    REG16 CONTROL_PADCONF_MMC2_DAT4 	;                                         // offset 0x2164
    REG16 CONTROL_PADCONF_MMC2_DAT5 	;                                         //
    REG16 CONTROL_PADCONF_MMC2_DAT6 	;                                         // offset 0x2168
    REG16 CONTROL_PADCONF_MMC2_DAT7 	;                                         //
    REG16 CONTROL_PADCONF_MCBSP3_DX 	;                                         // offset 0x216C
    REG16 CONTROL_PADCONF_MCBSP3_DR 	;                                         //
    REG16 CONTROL_PADCONF_MCBSP3_CLKX 	;                                         // offset 0x2170
    REG16 CONTROL_PADCONF_MCBSP3_FSX	;                                         //
    REG16 CONTROL_PADCONF_UART2_CTS		;                                         // offset 0x2174
    REG16 CONTROL_PADCONF_UART2_RTS		;                                         //
    REG16 CONTROL_PADCONF_UART2_TX		;                                         // offset 0x2178
    REG16 CONTROL_PADCONF_UART2_RX		;                                         //
    REG16 CONTROL_PADCONF_UART1_TX		;                                         // offset 0x217C
    REG16 CONTROL_PADCONF_UART1_RTS		;                                         //
    REG16 CONTROL_PADCONF_UART1_CTS		;                                         // offset 0x2180
    REG16 CONTROL_PADCONF_UART1_RX		;                                         //
    REG16 CONTROL_PADCONF_MCBSP4_CLKX 	;                                         // offset 0x2184
    REG16 CONTROL_PADCONF_MCBSP4_DR		;                                         //
    REG16 CONTROL_PADCONF_MCBSP4_DX		;                                         // offset 0x2188
    REG16 CONTROL_PADCONF_MCBSP4_FSX	;                                         //
    REG16 CONTROL_PADCONF_MCBSP1_CLKR 	;                                         // offset 0x218C
    REG16 CONTROL_PADCONF_MCBSP1_FSR	;                                         //
    REG16 CONTROL_PADCONF_MCBSP1_DX		;                                         // offset 0x2190
    REG16 CONTROL_PADCONF_MCBSP1_DR		;                                         //
    REG16 CONTROL_PADCONF_MCBSP_CLKS	;                                         // offset 0x2194
    REG16 CONTROL_PADCONF_MCBSP1_FSX	;                                         //
    REG16 CONTROL_PADCONF_MCBSP1_CLKX 	;                                         // offset 0x2198
    REG16 CONTROL_PADCONF_UART3_CTS_RCTX;                                         //
    REG16 CONTROL_PADCONF_UART3_RTS_SD	;                                         // offset 0x219C
    REG16 CONTROL_PADCONF_UART3_RX_IRRX	;                                         //
    REG16 CONTROL_PADCONF_UART3_TX_IRTX	;                                         // offset 0x21A0
    
	unsigned char reserved2[0x18];												  //---------------
    
	REG16 CONTROL_PADCONF_I2C1_SCL 		;                                         // offset 0x21BA
    REG16 CONTROL_PADCONF_I2C1_SDA 		;                                         // offset 0x21BC
    REG16 CONTROL_PADCONF_I2C2_SCL 		;                                         //
    REG16 CONTROL_PADCONF_I2C2_SDA 		;                                         // offset 0x21C0
    REG16 CONTROL_PADCONF_I2C3_SCL 		;                                         //
    REG16 CONTROL_PADCONF_I2C3_SDA 		;                                         // offset 0x21C4
    REG16 CONTROL_PADCONF_HDQ_SIO		;                                         //
    REG16 CONTROL_PADCONF_MCSPI1_CLK	;                                         // offset 0x21C8
    REG16 CONTROL_PADCONF_MCSPI1_SIMO 	;                                         //
    REG16 CONTROL_PADCONF_MCSPI1_SOMI 	;                                         // offset 0x21CC
    REG16 CONTROL_PADCONF_MCSPI1_CS0	;                                         //
    REG16 CONTROL_PADCONF_MCSPI1_CS1	;                                         // offset 0x21D0
    REG16 CONTROL_PADCONF_MCSPI1_CS2	;                                         //
    REG16 CONTROL_PADCONF_MCSPI1_CS3	;                                         // offset 0x21D4
    REG16 CONTROL_PADCONF_MCSPI2_CLK	;                                         //
    REG16 CONTROL_PADCONF_MCSPI2_SIMO 	;                                         // offset 0x21D8
    REG16 CONTROL_PADCONF_MCSPI2_SOMI 	;                                         //
    REG16 CONTROL_PADCONF_MCSPI2_CS0	;                                         // offset 0x21DC
    REG16 CONTROL_PADCONF_MCSPI2_CS1	;                                         //
    REG16 CONTROL_PADCONF_SYS_NIRQ		;                                         // offset 0x21E0
    
	unsigned char reserved3[0x2];												  //---------------

	REG16 CONTROL_PADCONF_CCDC_PCLK		;                                         // offset 0x21E4
    REG16 CONTROL_PADCONF_CCDC_FIELD	;                                         //
    REG16 CONTROL_PADCONF_CCDC_HD		;                                         // offset 0x21E8
    REG16 CONTROL_PADCONF_CCDC_VD		;                                         //
    REG16 CONTROL_PADCONF_CCDC_WEN		;                                         // offset 0x21EC
    REG16 CONTROL_PADCONF_CCDC_DATA0 	;                                         //
    REG16 CONTROL_PADCONF_CCDC_DATA1 	;                                         // offset 0x21F0
    REG16 CONTROL_PADCONF_CCDC_DATA2 	;                                         //
    REG16 CONTROL_PADCONF_CCDC_DATA3 	;                                         // offset 0x21F4
    REG16 CONTROL_PADCONF_CCDC_DATA4 	;                                         //
    REG16 CONTROL_PADCONF_CCDC_DATA5 	;                                         // offset 0x21F8
    REG16 CONTROL_PADCONF_CCDC_DATA6 	;                                         //
    REG16 CONTROL_PADCONF_CCDC_DATA7 	;                                         // offset 0x21FC
    REG16 CONTROL_PADCONF_RMII_MDIO_DATA;                                         //
    REG16 CONTROL_PADCONF_RMII_MDIO_CLK	;                                         // offset 0x2200
    REG16 CONTROL_PADCONF_RMII_RXD0		;                                         //
    REG16 CONTROL_PADCONF_RMII_RXD1		;                                         // offset 0x2204
    REG16 CONTROL_PADCONF_RMII_CRS_DV	;                                         //
    REG16 CONTROL_PADCONF_RMII_RXER 	;                                         // offset 0x2208
    REG16 CONTROL_PADCONF_RMII_TXD0 	;                                         //
    REG16 CONTROL_PADCONF_RMII_TXD1 	;                                         // offset 0x220C
    REG16 CONTROL_PADCONF_RMII_TXEN 	;                                         //
    REG16 CONTROL_PADCONF_RMII_50MHZ_CLK;                                         // offset 0x2210
    REG16 CONTROL_PADCONF_USB0_DRVVBUS	;                                         //
    REG16 CONTROL_PADCONF_HECC1_TXD		;                                         // offset 0x2214
    REG16 CONTROL_PADCONF_HECC1_RXD		;                                         //
    REG16 CONTROL_PADCONF_SDRC_DQS0N 	;                                         // offset 0x2218
    
	unsigned char reserved4[0x2];												  //---------------

    REG16 CONTROL_PADCONF_SDRC_DQS1N 	;                                         // offset 0x221C
    REG16 CONTROL_PADCONF_SDRC_DQS2N 	;                                         //
    REG16 CONTROL_PADCONF_SDRC_DQS3N 	;                                         // offset 0x2220
    REG16 CONTROL_PADCONF_SDRC_STRBEN_DLY0;                                       //
    REG16 CONTROL_PADCONF_SDRC_STRBEN_DLY1;                                       // offset 0x2224

	unsigned char reserved5[0x3C];												  //---------------

	REG16 CONTROL_PADCONF_SDRC_CKE0		;                                         // offset 0x2262

	unsigned char reserved6[0x374];												  //---------------

	REG16 CONTROL_PADCONF_ETK_CLK		;                                         // offset 0x25D8
    REG16 CONTROL_PADCONF_ETK_CTL		;                                         // offset 0x25D8
    REG16 CONTROL_PADCONF_ETK_D0 		;                                         // offset 0x25DC
    REG16 CONTROL_PADCONF_ETK_D1 		;                                         // offset 0x25DC
    REG16 CONTROL_PADCONF_ETK_D2 		;                                         // offset 0x25E0
    REG16 CONTROL_PADCONF_ETK_D3 		;                                         // offset 0x25E0
    REG16 CONTROL_PADCONF_ETK_D4 		;                                         // offset 0x25E4
    REG16 CONTROL_PADCONF_ETK_D5 		;                                         // offset 0x25E4
    REG16 CONTROL_PADCONF_ETK_D6 		;                                         // offset 0x25E8
    REG16 CONTROL_PADCONF_ETK_D7 		;                                         // offset 0x25E8
    REG16 CONTROL_PADCONF_ETK_D8 		;                                         // offset 0x25EC
    REG16 CONTROL_PADCONF_ETK_D9 		;                                         // offset 0x25EC
    REG16 CONTROL_PADCONF_ETK_D10 		;                                         // offset 0x25F0
    REG16 CONTROL_PADCONF_ETK_D11 		;                                         // offset 0x25F0
    REG16 CONTROL_PADCONF_ETK_D12 		;                                         // offset 0x25F4
    REG16 CONTROL_PADCONF_ETK_D13 		;                                         // offset 0x25F4
    REG16 CONTROL_PADCONF_ETK_D14 		;                                         // offset 0x25F8
    REG16 CONTROL_PADCONF_ETK_D15 		;                                         // offset 0x25F8
} OMAP_SYSC_PADCONFS_REGS;


#define PAD(x,type,start,end)  type ReservedPad##x [(end-start)/sizeof(type) - 1]
// OMAP_SYSC_GENERAL_REGS
//
typedef struct {
    REG32 CONTROL_PADCONF_OFF;                        // offset 0x0000           
    REG32 CONTROL_DEVCONF0;                           // offset 0x0004
    REG32 CONTROL_MEM_DFTRW0;                         // offset 0x0008
    REG32 CONTROL_MEM_DFTRW1;                         // offset 0x000C
    PAD(1,UINT32,0x000C,0x0020);
    REG32 CONTROL_MSUSPENDMUX_0;                      // offset 0x0020
    REG32 CONTROL_MSUSPENDMUX_1;                      // offset 0x0024
    REG32 CONTROL_MSUSPENDMUX_2;                      // offset 0x0028
    REG32 CONTROL_MSUSPENDMUX_3;                      // offset 0x002C
    REG32 CONTROL_MSUSPENDMUX_4;                      // offset 0x0030
    REG32 CONTROL_MSUSPENDMUX_5;                      // offset 0x0034
    REG32 CONTROL_MSUSPENDMUX_6;                      // offset 0x0038
    PAD(2,UINT32,0x0038,0x0040);
    REG32 CONTROL_SEC_CTRL;                           // offset 0x0040
    PAD(3,UINT32,0x0040,0x0068);
    REG32 CONTROL_DEVCONF1;                           // offset 0x0068
    PAD(4,UINT32,0x0068,0x0070);
    REG32 CONTROL_SEC_STATUS;                         // offset 0x0070
    REG32 CONTROL_SEC_ERR_STATUS;                     // offset 0x0074
    REG32 CONTROL_SEC_ERR_STATUS_DEBUG;               // offset 0x0078
    PAD(5,UINT32,0x0078,0x0080);
    REG32 CONTROL_STATUS;                             // offset 0x0080
    REG32 CONTROL_GENERAL_PURPOSE_STATUS;             // offset 0x0084
    PAD(6,UINT32,0x0084,0x0090);
    REG32 CONTROL_RPUB_KEY_H_0;                       // offset 0x0090
    REG32 CONTROL_RPUB_KEY_H_1;                       // offset 0x0094
    REG32 CONTROL_RPUB_KEY_H_2;                       // offset 0x0098
    REG32 CONTROL_RPUB_KEY_H_3;                       // offset 0x009C
    REG32 CONTROL_RPUB_KEY_H_4;                       // offset 0x00A0
    PAD(7,UINT32,0x00A0,0x00A8);
    REG32 CONTROL_RAND_KEY_0;                         // offset 0x00A8
    REG32 CONTROL_RAND_KEY_1;                         // offset 0x00AC
    REG32 CONTROL_RAND_KEY_2;                         // offset 0x00B0
    REG32 CONTROL_RAND_KEY_3;                         // offset 0x00B4
    REG32 CONTROL_CUST_KEY_0;                         // offset 0x00B8
    REG32 CONTROL_CUST_KEY_1;                         // offset 0x00BC
    REG32 CONTROL_CUST_KEY_2;                         // offset 0x00C0
    REG32 CONTROL_CUST_KEY_3;                         // offset 0x00C4
    PAD(8,UINT32,0x00C4,0x0100);
    REG32 CONTROL_USB_CONF_0;                         // offset 0x0100
    REG32 CONTROL_USB_CONF_1;                         // offset 0x0104
    PAD(9,UINT32,0x0104,0x0110);
    REG32 CONTROL_FUSE_EMAC_LSB;                      // offset 0x0110
    REG32 CONTROL_FUSE_EMAC_MSB;                      // offset 0x0114
    PAD(10,UINT32,0x0114,0x0130);
    REG32 CONTROL_FUSE_SR;                            // offset 0x0130
    REG32 CONTROL_CEK_0;                              // offset 0x0134
    REG32 CONTROL_CEK_1;                              // offset 0x0138
    REG32 CONTROL_CEK_2;                              // offset 0x013C
    REG32 CONTROL_CEK_3;                              // offset 0x0140
    REG32 CONTROL_MSV_0;                              // offset 0x0144
    REG32 CONTROL_CEK_BCH_0;                          // offset 0x0148
    REG32 CONTROL_CEK_BCH_1;                          // offset 0x014C
    REG32 CONTROL_CEK_BCH_2;                          // offset 0x0150
    REG32 CONTROL_CEK_BCH_3;                          // offset 0x0154
    REG32 CONTROL_CEK_BCH_4;                          // offset 0x0158
    REG32 CONTROL_MSV_BCH_0;                          // offset 0x015C
    REG32 CONTROL_MSV_BCH_1;                          // offset 0x0160
    REG32 CONTROL_SWRV_0;                             // offset 0x0164
    REG32 CONTROL_SWRV_1;                             // offset 0x0168
    REG32 CONTROL_SWRV_2;                             // offset 0x016C
    REG32 CONTROL_SWRV_3;                             // offset 0x0170
    REG32 CONTROL_SWRV_4;                             // offset 0x0174
    PAD(11,UINT32,0x0174,0x01B0);
    REG32 CONTROL_DEBOBS_0;                           // offset 0x01B0
    REG32 CONTROL_DEBOBS_1;                           // offset 0x01B4
    REG32 CONTROL_DEBOBS_2;                           // offset 0x01B8
    REG32 CONTROL_DEBOBS_3;                           // offset 0x01BC
    REG32 CONTROL_DEBOBS_4;                           // offset 0x01C0
    REG32 CONTROL_DEBOBS_5;                           // offset 0x01C4
    REG32 CONTROL_DEBOBS_6;                           // offset 0x01C8
    REG32 CONTROL_DEBOBS_7;                           // offset 0x01CC
    REG32 CONTROL_DEBOBS_8;                           // offset 0x01D0
    PAD(12,UINT32,0x01D0,0x01E0);    
    REG32 CONTROL_DSS_DPLL_SPREADING;                 // offset 0x01E0
    REG32 CONTROL_CORE_DPLL_SPREADING;                // offset 0x01E4
    REG32 CONTROL_PER_DPLL_SPREADING;                 // offset 0x01E8
    REG32 CONTROL_USBHOST_DPLL_SPREADING;             // offset 0x01EC
    PAD(13,UINT32,0x01EC,0x0228);    
    REG32 CONTROL_DPF_OCM_RAM_FW_ADDR_MATCH;          // offset 0x0228
    REG32 CONTROL_DPF_OCM_RAM_FW_REQINFO;             // offset 0x022C
    REG32 CONTROL_DPF_OCM_RAM_FW_WR;                  // offset 0x0230
    REG32 CONTROL_DPF_REGION4_GPMC_FW_ADDR_MATCH;     // offset 0x0234
    REG32 CONTROL_DPF_REGION4_GPMC_FW_REQINFO;        // offset 0x0238
    REG32 CONTROL_DPF_REGION4_GPMC_FW_WR;             // offset 0x023C
    PAD(14,UINT32,0x023C,0x024C);    
    REG32 CONTROL_APE_FW_DEFAULT_SECURE_LOCK;         // offset 0x024C
    REG32 CONTROL_OCMROM_SECURE_DEBUG;                // offset 0x0250
    PAD(15,UINT32,0x0250,0x0264);    
    REG32 CONTROL_EXT_SEC_CONTROL;                    // offset 0x0264
    PAD(16,UINT32,0x0264,0x0310);    
    REG32 CONTROL_DEVCONF2;                           // offset 0x0310
    REG32 CONTROL_DEVCONF3;                           // offset 0x0314
    PAD(17,UINT32,0x0314,0x0320);    
    REG32 CONTROL_CBA_PRIORITY;                       // offset 0x0320
    REG32 CONTROL_LVL_INTR_CLEAR;                     // offset 0x0324
    REG32 CONTROL_IP_SW_RESET;                        // offset 0x0328
    REG32 CONTROL_IPSS_CLK_CTRL;                      // offset 0x032C
    PAD(18,UINT32,0x032C,0x0A5C);    
    REG32 CONTROL_WKUP_CTRL;                          // offset 0x0A5C
} OMAP_SYSC_GENERAL_REGS;



// OMAP_IDCORE_REGS
//
typedef volatile struct {

    REG32  IDCODE;                                  // offset 0x0204

} OMAP_IDCORE_REGS;


// OMAP_SYSC_PADCONFS_WKUP_REGS
//
typedef volatile struct {
    REG16  CONTROL_PADCONF_I2C4_SCL;               // offset 0x0000
    REG16  CONTROL_PADCONF_I2C4_SDA;
    REG16  CONTROL_PADCONF_SYS_32K;                // offset 0x0004
    REG16  CONTROL_PADCONF_SYS_CLKREQ;
    REG16  CONTROL_PADCONF_SYS_NRESWARM;           // offset 0x0008
    REG16  CONTROL_PADCONF_SYS_BOOT0;
    REG16  CONTROL_PADCONF_SYS_BOOT1;              // offset 0x000C
    REG16  CONTROL_PADCONF_SYS_BOOT2;
    REG16  CONTROL_PADCONF_SYS_BOOT3;              // offset 0x0010
    REG16  CONTROL_PADCONF_SYS_BOOT4;      
    REG16  CONTROL_PADCONF_SYS_BOOT5;              // offset 0x0014
    REG16  CONTROL_PADCONF_SYS_BOOT6;
    REG16  CONTROL_PADCONF_SYS_OFF_MODE;           // offset 0x0018
    REG16  CONTROL_PADCONF_SYS_CLKOUT1;
    REG16  CONTROL_PADCONF_JTAG_NTRST;             // offset 0x001C
    REG16  CONTROL_PADCONF_JTAG_TCK;
    REG16  CONTROL_PADCONF_JTAG_TMS;               // offset 0x0020
    REG16  CONTROL_PADCONF_JTAG_TDI;
    REG16  CONTROL_PADCONF_JTAG_EMU0;              // offset 0x0024
    REG16  CONTROL_PADCONF_JTAG_EMU1;

    REG16  zzzReserved01[18];                      // offset 0x0028

    REG16  CONTROL_PADCONF_SAD2D_SWAKEUP;          // offset 0x004C
} OMAP_SYSC_PADCONFS_WKUP_REGS;


// OMAP_SYSC_GENERAL_WKUP_REGS
//
typedef volatile struct {

    REG32 CONTROL_SEC_TAP;                          // offset 0x0000
    REG32 CONTROL_SEC_EMU;                          // offset 0x0004
    REG32 CONTROL_WKUP_DEBOBS_0;                    // offset 0x0008
    REG32 CONTROL_WKUP_DEBOBS_1;                    // offset 0x000C
    REG32 CONTROL_WKUP_DEBOBS_2;                    // offset 0x0010
    REG32 CONTROL_WKUP_DEBOBS_3;                    // offset 0x0014
    REG32 CONTROL_WKUP_DEBOBS_4;                    // offset 0x0018
    REG32 CONTROL_SEC_DAP;                          // offset 0x001C
} OMAP_SYSC_GENERAL_WKUP_REGS;



//------------------------------------------------------------------------------
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

#define OFFOUTVALUE_HIGH                (1 << 11)
#define OFFOUTVALUE_LOW                 (0 << 11)

#define OFFPULLUD_ENABLE                (1 << 12)
#define OFFPULLUD_DISABLE               (0 << 12)

#define OFF_PULLUP                      (1 << 13)
#define OFF_PULLDOWN                    (0 << 13)

// DEVCONF0
#define DEVCONF0_SENSDMAREQ0			(1 << 0)
#define DEVCONF0_SENSDMAREQ1			(1 << 1)
#define DEVCONF0_MCBSP1_CLKS			(1 << 2)
#define DEVCONF0_MCBSP1_CLKR			(1 << 3)
#define DEVCONF0_MCBSP1_FSR				(1 << 4)
#define DEVCONF0_MCBSP2_CLKS			(7 << 2)

// CONTROL_DEVCONF1 bits
#define DEVCONF1_MCBSP3_CLKS                     (1 << 0)
#define DEVCONF1_MCBSP4_CLKS                     (1 << 2)
#define DEVCONF1_MCBSP5_CLKS                     (1 << 4)
#define DEVCONF1_MMCSDIO2ADPCLKISEL              (1 << 6)
#define DEVCONF1_SENSDMAREQ2                     (1 << 7)
#define DEVCONF1_SENSDMAREQ3                     (1 << 8)
#define DEVCONF1_MPUFORCEWRNP                    (1 << 9)
#define DEVCONF1_TVACEN                          (1 << 11)
#define DEVCONF1_I2C1HSMASTER                    (1 << 12)
#define DEVCONF1_I2C2HSMASTER                    (1 << 13)
#define DEVCONF1_I2C3HSMASTER                    (1 << 14)
#define DEVCONF1_TVOUTBYPASS                     (1 << 18)
#define DEVCONF1_CARKITHSUSB0DATA0AUTOEN         (1 << 19)
#define DEVCONF1_CARKITHSUSB0DATA1AUTOEN         (1 << 20)
#define DEVCONF1_SENSDMAREQ4                     (1 << 21)
#define DEVCONF1_SENSDMAREQ5                     (1 << 22)
#define DEVCONF1_SENSDMAREQ6                     (1 << 23)

// DEVCONF2
#define DEVCONF2_USBPHY_GPIO_MODE		(1   << 23)
#define DEVCONF2_USBOTG_OTGMODE			(3   << 14)
#define DEVCONF2_USBOTG_OTGMODE_HOST	(1	 << 14)
#define DEVCONF2_USBOTG_OTGMODE_FN		(2	 << 14)
#define DEVCONF2_USBOTG_SESSENDEN		(1   << 13)
#define DEVCONF2_USBOTG_VBUSDETECTEN	(1   << 12)
#define DEVCONF2_USBOTG_REFFREQ			(0xF <<  8)
#define DEVCONF2_USBOTG_REFFREQ_26MHZ	(7   <<  8)
#define DEVCONF2_USBOTG_REFFREQ_24MHZ	(2   <<  8)
#define DEVCONF2_USBOTG_REFFREQ_13MHZ	(6 	 <<  8)
#define DEVCONF2_USBOTG_PWR_CLKGOOD		(1 	 <<  7)
#define DEVCONF2_USBOTG_VBUS_SENSE		(1 	 <<  6)
#define DEVCONF2_USBOTG_PHY_PLLON		(1 	 <<  5)
#define DEVCONF2_USBOTG_PHY_RESET		(1 	 <<  4)
#define DEVCONF2_USBOTG_PHY_PD			(1 	 <<  3)
#define DEVCONF2_USBOTG_POWERDOWNOTG	(1 	 <<  2)
#define DEVCONF2_USBOTG_DATAPOLARITY	(1 	 <<  1)

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

// CONTROL_IPSS_CLK_CTRL
#define VPFE_FUNC_CLK_EN				(1 << 10)
#define CPGMAC_FUNC_CLK_EN				(1 <<  9)
#define USB20OTG_FUNC_CLK_EN			(1 <<  8)
#define HECC_VBUSP_CLK_EN_ACK			(1 <<  7)
#define VPFE_VBUSP_CLK_EN_ACK			(1 <<  6) 
#define CPGMAC_VBUSP_CLK_EN_ACK			(1 <<  5)
#define USB20OTG_VBUSP_CLK_EN_ACK		(1 <<  4)
#define HECC_VBUSP_CLK_EN               (1 <<  3)
#define VPFE_VBUSP_CLK_EN				(1 <<  2)
#define CPGMAC_VBUSP_CLK_EN				(1 <<  1)
#define USB20OTG_VBUSP_CLK_EN			(1 <<  0)

// CONTROL_IP_SW_RESET
#define	VPFE_PCLK_SW_RST				(1 << 4)
#define HECC_SW_RST						(1 << 3)
#define VPFE_VBUSP_SW_RST				(1 << 2)
#define CPGMACSS_SW_RST					(1 << 1)
#define USB20OTGSS_SW_RST				(1 << 0)

// CONTROL_LVL_INTR_CLEAR
#define	VPFE_CCDC_VD2_INT_CLR			(1 << 7)
#define VPFE_CCDC_VD1_INT_CLR			(1 << 6)
#define VPFE_CCDC_VD0_INT_CLR			(1 << 5)
#define USB20OTGSS_USB_INT_CLR			(1 << 4)
#define CPGMAC_C0_TX_PULSE_CLR			(1 << 3)
#define CPGMAC_C0_RX_THRESH_PULSE_CLR	(1 << 2)
#define CPGMAC_C0_RX_PULSE_CLR			(1 << 1)
#define CPGMAC_C0_MISC_PULSE_CLR		(1 << 0)

#endif // __OMAP3517_CONFIG_H

