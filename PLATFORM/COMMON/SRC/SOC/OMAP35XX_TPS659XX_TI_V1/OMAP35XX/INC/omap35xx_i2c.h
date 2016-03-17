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
//  Header:  omap35xx_i2c.h
//
#ifndef __OMAP35XX_I2C_H
#define __OMAP35XX_I2C_H


//------------------------------------------------------------------------------

typedef volatile struct {
    UINT16 REV;                 // 0000
    UINT16 zzzReserved0002;
    UINT16 IE;                  // 0004
    UINT16 zzzReserved0006;
    UINT16 STAT;                // 0008
    UINT16 zzzReserved000A;
    UINT16 WE;                  // 000C
    UINT16 zzzReserved000E;
    UINT16 SYSS;                // 0010
    UINT16 zzzReserved0012;
    UINT16 BUF;                 // 0014
    UINT16 zzzReserved0016;
    UINT16 CNT;                 // 0018
    UINT16 zzzReserved001A;
    UINT16 DATA;                // 001C
    UINT16 zzzReserved001E;
    UINT16 SYSC;                // 0020
    UINT16 zzzReserved0022;
    UINT16 CON;                 // 0024
    UINT16 zzzReserved0026;
    UINT16 OA0;                 // 0028
    UINT16 zzzReserved002A;
    UINT16 SA;                  // 002C
    UINT16 zzzReserved002E;
    UINT16 PSC;                 // 0030
    UINT16 zzzReserved0032;
    UINT16 SCLL;                // 0034
    UINT16 zzzReserved0036;
    UINT16 SCLH;                // 0038
    UINT16 zzzReserved003A;
    UINT16 SYSTEST;             // 003C
    UINT16 zzzReserved003E;
    UINT16 BUFSTAT;             // 0040
    UINT16 zzzReserved0042;
    UINT16 OA1;                 // 0044
    UINT16 zzzReserved0046;
    UINT16 OA2;                 // 0048
    UINT16 zzzReserved004A;
    UINT16 OA3;                 // 004C
    UINT16 zzzReserved004E;
    UINT16 ACTOA;               // 0050
    UINT16 zzzReserved0052;
    UINT16 SBLOCK;              // 0054
} OMAP_I2C_REGS;

//------------------------------------------------------------------------------

#define I2C_STAT_XDR                            (1 << 14)
#define I2C_STAT_RDR                            (1 << 13)
#define I2C_STAT_BB                             (1 << 12)
#define I2C_STAT_ROVR                           (1 << 11)
#define I2C_STAT_XUDF                           (1 << 10)
#define I2C_STAT_AAS                            (1 << 9)
#define I2C_STAT_BF                             (1 << 8)
#define I2C_STAT_AERR                           (1 << 7)
#define I2C_STAT_STC                            (1 << 6)
#define I2C_STAT_GC                             (1 << 5)
#define I2C_STAT_XRDY                           (1 << 4)
#define I2C_STAT_RRDY                           (1 << 3)
#define I2C_STAT_ARDY                           (1 << 2)
#define I2C_STAT_NACK                           (1 << 1)
#define I2C_STAT_AL                             (1 << 0)

#define I2C_SYSS_RDONE                          (1 << 0)

#define I2C_SYSC_CLKACTY_FCLK                   (1 << 9)
#define I2C_SYSC_CLKACTY_ICLK                   (1 << 8)
#define I2C_SYSC_SRST                           (1 << 1)
#define I2C_SYSC_AUTOIDLE                       (1 << 0)
#define I2C_SYSC_FORCE_IDLE                     (0 << 3)
#define I2C_SYSC_NO_IDLE                        (1 << 3)
#define I2C_SYSC_SMART_IDLE                     (2 << 3)

#define I2C_CON_EN                              (1 << 15)
#define I2C_CON_OPMODE_FS                       (0 << 12)
#define I2C_CON_OPMODE_HS                       (1 << 12)
#define I2C_CON_OPMODE_SCCB                     (2 << 12)
#define I2C_CON_STB                             (1 << 11)
#define I2C_CON_MST                             (1 << 10)
#define I2C_CON_TRX                             (1 << 9)
#define I2C_CON_XSA                             (1 << 8)
#define I2C_CON_XOA0                            (1 << 7)
#define I2C_CON_XOA1                            (1 << 6)
#define I2C_CON_XOA2                            (1 << 5)
#define I2C_CON_XOA3                            (1 << 4)
#define I2C_CON_STP                             (1 << 1)
#define I2C_CON_STT                             (1 << 0)
#define I2C_CON_DISABLE                         (0 << 0)

#define I2C_BUF_RDMA_EN                         (1 << 15)
#define I2C_BUF_RXFIFO_CLR                      (1 << 14)
#define I2C_BUF_RTRSH_MASK                      (0x3F00)
#define I2C_BUF_RTRSH_SHIFT                     (8)
#define I2C_BUF_RTRSH(x)                        (((x) << I2C_BUF_RTRSH_SHIFT) & I2C_BUF_RTRSH_MASK)
#define I2C_BUF_XDMA_EN                         (1 << 7)
#define I2C_BUF_TXFIFO_CLR                      (1 << 6)
#define I2C_BUF_XTRSH_MASK                      (0x3F)
#define I2C_BUF_XTRSH_SHIFT                     (0)
#define I2C_BUF_XTRSH(x)                        (((x) << I2C_BUF_XTRSH_SHIFT) & I2C_BUF_XTRSH_MASK)

#define I2C_BUFSTAT_FIFODEPTH_MASK              (0xC000)
#define I2C_BUFSTAT_FIFODEPTH_SHIFT             (14)
#define I2C_BUFSTAT_RXSTAT_MASK                 (0x3F00)
#define I2C_BUFSTAT_RXSTAT_SHIFT                (8)
#define I2C_BUFSTAT_TXSTAT_MASK                 (0x003F)
#define I2C_BUFSTAT_TXSTAT_SHIFT                (0)

#define I2C_OP_FULLSPEED_MODE                   (0)
#define I2C_OP_HIGHSPPED_MODE                   (1)
#define I2C_OP_SCCB_MODE                        (2)

//------------------------------------------------------------------------------
// useful defines

#define I2C_MASTER_CODE                         (0x4000)
#define I2C_SLAVE_MASK                          (0x007f)

//------------------------------------------------------------------------------
//Internal clock was connected to I2C is 96mhz fclk;
//100kbps
#define I2C_SS_PSC_VAL                          (23)
#define I2C_SS_SCLL_VAL                         (13)
#define I2C_SS_SCLH_VAL                         (15)

//400kbps
#define I2C_FS_PSC_VAL                          (9)
#define I2C_FS_SCLL_VAL                         (5)
#define I2C_FS_SCLH_VAL                         (7)

//>400kbps, first phase
#define I2C_HS_PSC_VAL                          (4)
#define I2C_HS_SCLL                             (17)
#define I2C_HS_SCLH                             (19)

//second phase
#define I2C_1P6M_HSSCLL                         ((23 << 8) | I2C_HS_SCLL)
#define I2C_1P6M_HSSCLH                         ((25 << 8) | I2C_HS_SCLH)

#define I2C_2P4M_HSSCLL                         ((13 << 8) | I2C_HS_SCLL)
#define I2C_2P4M_HSSCLH                         ((15 << 8) | I2C_HS_SCLH)

#define I2C_3P2M_HSSCLL                         ((8 << 8) | I2C_HS_SCLL)
#define I2C_3P2M_HSSCLH                         ((10 << 8) | I2C_HS_SCLH)

#define I2C_DEFAULT_HSSCLL                      I2C_3P2M_HSSCLL
#define I2C_DEFAULT_HSSCLH                      I2C_3P2M_HSSCLH

#endif // __OMAP35XX_I2C_H
