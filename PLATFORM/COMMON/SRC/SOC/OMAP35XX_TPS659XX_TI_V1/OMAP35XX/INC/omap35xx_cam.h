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
//  File:  omap35xx_camera.h
//
//  This file contains offset addresses for Camera registers.

#ifndef __OMAP35XX_CAMERA_H
#define __OMAP35XX_CAMERA_H

/* Camera register list */
/*
 * 1. ISP
 * 2. ISP_CSIA
 * 3. ISP_CSIB
 * 4. ISP_CCDC
 * 5. ISP_SCMP
 * 6. ISP_HIST
 * 7. ISP_H3A
 * 8. ISP_PREVIEW
 * 9. ISP_RESIZER
 *10. ISP_SBL
 */


/*    ISP Register summary       */
/*   Base Address - 0x480BC000   */
typedef volatile struct {     //offset

  UINT32 ISP_REVISION;           //0x0
  UINT32 ISP_SYSCONFIG;          //0x4
  UINT32 ISP_SYSSTATUS;          //0x8
  UINT32 ISP_IRQ0ENABLE;         //0xC
  UINT32 ISP_IRQ0STATUS;         //0x10
  UINT32 ISP_IRQ1ENABLE;         //0x14
  UINT32 ISP_IRQ1STATUS;         //0x18
  UINT32 ISP_Reserved_1C_2F[05]; //0x1C-2F
  UINT32 TCTRL_GRESET_LENGTH;    //0x30
  UINT32 TCTRL_PSTRB_REPLAY;     //0x34
  UINT32 ISP_Reserved_38_3F[02]; //0x38-0x3F
  UINT32 ISP_CTRL;               //0x40
  UINT32 ISP_SECURE;             //0x44
  UINT32 ISP_Reserved_48_4F[02]; //0x48-0x4F
  UINT32 TCTRL_CTRL;             //0x50
  UINT32 TCTRL_FRAME;            //0x54
  UINT32 TCTRL_PSTRB_DELAY;      //0x58
  UINT32 TCTRL_STRB_DELAY;       //0x5C
  UINT32 TCTRL_SHUT_DELAY;       //0x60
  UINT32 TCTRL_PSTRB_LENGTH;     //0x64
  UINT32 TCTRL_STRB_LENGTH;      //0x68
  UINT32 TCTRL_SHUT_LENGTH;      //0x6C
  UINT32 PING_PONG_ADDR;         //0x70
  UINT32 PING_PONG_MEM_RANGE;    //0x74
  UINT32 PING_PONG_BUF_SIZE;     //0x78

} OMAP_CAM_ISP_REGS;


#define ISP_SYSCONFIG_MIDLEMODE_FSTDBY   (0 << 12) //Force standby
#define ISP_SYSCONFIG_MIDLEMODE_NSTDBY   (1 << 12) //No standby
#define ISP_SYSCONFIG_MIDLEMODE_SSTDBY   (2 << 12) //Smart standby
#define ISP_SYSCONFIG_SOFTRESET          (1 << 1)  //1 - Reset 0 - Normal
#define ISP_SYSCONFIG_AUTOIDLE           (1 << 0)  //1 - AutoIdle 0 - Free running

#define ISP_SYSSTATUS_RESETDONE          (1 << 0)  //1 - Reset done, 0 - reset is ongoing

/* ISP IRQENABLE/STATUS register(s) fields */
#define ISP_IRQ0ENABLE_HS_VS_IRQ         (1 << 31) //1 - Generate interrupt,0-Mask interrupt
#define ISP_IRQ0ENABLE_OVF_IRQ           (1 << 25) //1 - Generate interrupt,0-Mask interrupt

#define ISP_IRQ0ENABLE_PRV_DONE_IRQ      (1 << 20) //1 - Generate interrupt,0-Mask interrupt
#define ISP_IRQ0ENABLE_RSZ_DONE_IRQ      (1 << 24) //1 - Generate interrupt,0-Mask interrupt
#define ISP_IRQ0ENABLE_H3A_AEW_DONE_IRQ  (1 << 13) //1 - Generate interrupt,0-Mask interrupt
#define ISP_IRQ0ENABLE_H3A_AF_DONE_IRQ   (1 << 12) //1 - Generate interrupt,0-Mask interrupt
#define ISP_IRQ0ENABLE_HIST_DONE_IRQ     (1 << 16) //1 - Generate interrupt,0-Mask interrupt
#define ISP_IRQ0ENABLE_CCDC_VD2_IRQ      (1 << 10) //1 - Generate interrupt,0-Mask interrupt
#define ISP_IRQ0ENABLE_CCDC_VD1_IRQ      (1 << 9)  //1 - Generate interrupt,0-Mask interrupt
#define ISP_IRQ0ENABLE_CCDC_VD0_IRQ      (1 << 8)  //1 - Generate interrupt,0-Mask interrupt
#define ISP_IRQ0ENABLE_CSI2_IRQ          (1 << 0)

/* ISP_CTRL register bit fields */
#define ISP_CTRL_PAR_CLK_SEL_12BIT       (0 << 0)  //Select 12-bit parallel interface
#define ISP_CTRL_PAR_CLK_SEL_CSIA        (1 << 0)  //Select CSIA interface
#define ISP_CTRL_PAR_CLK_SEL_CSIB        (2 << 0)  //Select CSIB interface

#define ISP_CTRL_PAR_BRIDGE_DISABLE      (0 << 2)
#define ISP_CTRL_PAR_BRIDGE_RESERVED     (1 << 2)
#define ISP_CTRL_PAR_BRIDGE_ENABLE       (2 << 2)
#define ISP_CTRL_PAR_BRIDGE_ENABLESWAP   (3 << 2) //Byte Swap

#define ISP_CTRL_PAR_CLK_POL             (1 << 4) //1-clk inverted,0-clk not inverted
#define ISP_CTRL_PING_PONG_EN            (1 << 5) //1-PingPong enable,0-Pingpong disable
#define ISP_CTRL_SHIFT_NOSHIFT           (0 << 6)
#define ISP_CTRL_SHIFT_SHIFT2            (1 << 6)
#define ISP_CTRL_SHIFT_SHIFT4            (2 << 6)

#define ISP_CTRL_CCDC_CLK_EN             (1 << 8) //1-clk enable 0 - clk disable
#define ISP_CTRL_SCMP_CLK_EN             (1 << 9)
#define ISP_CTRL_H3A_CLK_EN              (1 << 10)
#define ISP_CTRL_HIST_CLK_EN             (1 << 11)
#define ISP_CTRL_PRV_CLK_EN              (1 << 12)
#define ISP_CTRL_RSZ_CLK_EN              (1 << 13)
#define ISP_CTRL_SYNC_DET_HSFALLEDGE     (0 << 14)
#define ISP_CTRL_SYNC_DET_HSRISINGEDGE   (1 << 14)
#define ISP_CTRL_SYNC_DET_VSFALLEDGE     (2 << 14)
#define ISP_CTRL_SYNC_DET_VSRISINGEDGE   (3 << 14)
#define ISP_CTRL_CCDC_RAM_EN             (1 << 16)
#define ISP_CTRL_PRV_RAM_EN              (1 << 17)
#define ISP_CTRL_SBL_RD_RAM_EN           (1 << 18)
#define ISP_CTRL_SBL_WR1_RAM_EN          (1 << 19)
#define ISP_CTRL_SBL_WR0_RAM_EN          (1 << 20)
#define ISP_CTRL_SBL_AUTOIDLE            (1 << 21)//1-autoidle 0-normal
#define ISP_CTRL_SBL_SHARED_RPORTA       (1 << 27)
#define ISP_CTRL_SBL_SHARED_RPORTB       (1 << 28)
#define ISP_CTRL_CCDC_WEN_POL            (1 << 29)//1-high 0-low
#define ISP_CTRL_CCDC_FLUSH              (1 << 31)//1-flush

/* TCTRL_CTRL - Bit fields description */
#define TCTRL_CTRL_GRESETDIR             (1 << 31)//1-o/p 0-i/p
#define TCTRL_CTRL_GRESETPOL             (1 << 30)//1-low, 0-high

#define TCTRL_CTRL_DIVC_CLK(x)           (x << 10)
#define TCTRL_CTRL_DIVB_CLK(x)           (x << 5)
#define TCTRL_CTRL_DIVA_CLK(x)           (x << 0)
#define TCTRL_CTRL_DIVC_MASK             0x1FF
#define TCTRL_CTRL_DIVB_MASK             0x1F
#define TCTRL_CTRL_DIVA_MASK             0x1F

/* CSI2 Receiver Registers   */
/* Base Address = 0x480BD800 */
#define CSI2_RECEIVER_BASE_ADDR 0x480BD800

#define PREV_RED_GAMMA_TBL_ADDR 			0x0000
#define PREV_GREEN_GAMMA_TBL_ADDR 			0x0400
#define PREV_BLUE_GAMMA_TBL_ADDR 			0x0800
#define PREV_NF_THRESHOLD_TBL_ADDR 			0x0C00
#define PREV_NF_STRENGTH_TBL_ADDR 			0x0C20
#define PREV_NON_LINEAR_ENH_TBL_ADDR 		0x1000 
#define PREV_CFA_COEFF_TBL_ADDR 			0x1400
typedef volatile struct {

  UINT32 CSI2_REVISION;                   //(0x0)
  UINT32 CSI2_RESERVED_01[3];             //(0x04 - 0x07, 0x08 - 0x0B, 0x0C - 0x0F)
  UINT32 CSI2_SYSCONFIG;                  //(0x10)
  UINT32 CSI2_SYSSTATUS;                  //(0x14)
  UINT32 CSI2_IRQSTATUS;                  //(0x18)
  UINT32 CSI2_IRQENABLE;                  //(0x1C)
  UINT32 CSI2_RESERVED_02[8];
  UINT32 CSI2_CTRL;                       //(0x40)
  UINT32 CSI2_DBG_H;                      //(0x44)
  UINT32 CSI2_GNQ;                        //(0x48)
  UINT32 CSI2_COMPLEXIO_CFG2;             //(0x4C)
  UINT32 CSI2_COMPLEXIO_CFG1;             //(0x50)
  UINT32 CSI2_COMPLEXIO1_IRQSTATUS;       //(0x54)
  UINT32 CSI2_COMPLEXIO2_IRQSTATUS;       //(0x58)
  UINT32 CSI2_SHORT_PACKET;               //(0x5C)
  UINT32 CSI2_COMPLEXIO1_IRQENABLE;       //(0x60)
  UINT32 CSI2_COMPLEXIO2_IRQENABLE;       //(0x64)
  UINT32 CSI2_DBG_P;                      //(0x68)
  UINT32 CSI2_TIMING;                     //(0x6C)
  UINT32 CSI2_CTX_CTRL1_BASE;             //(0x70) 0x70 - 0x150
  UINT32 CSI2_CTX_CTRL2_BASE;             //(0x74) 0x74 - 0x154
  UINT32 CSI2_CTX_DAT_OFST_BASE;          //(0x78) 0x78 - 0x158
  UINT32 CSI2_CTX_DAT_PING_ADDR_BASE;     //(0x7C) 0x7C - 0x15C
  UINT32 CSI2_CTX_DAT_PONG_ADDR_BASE;     //(0x80) 0x80 - 0x160
  UINT32 CSI2_CTX_IRQENABLE_BASE;         //(0x84) 0x84 - 0x164
  UINT32 CSI2_CTX_IRQSTATUS_BASE;         //(0x88) 0x88 - 0x168
  UINT32 CSI2_CTX_CTRL3_BASE;             //(0x8C) 0x8C - 0x16C
}OMAP_CAM_CSI2RVR_REGS;


#define CTX(x,y)    (x + (0x14*y))


#define CSI2_CLOCK_LANE           1
#define CSI2_DATA1_LANE           2
#define CSI2_DATA2_LANE           3

#define CSI2_CLOCK_POL_VAL        0 //0-+ve 1 - -ve
#define CSI2_DATA_POL_VAL         0 //0-+ve 1 - -ve

#define CSI2_CLOCK_POS(x)         (x << 0)
#define CSI2_CLOCK_POL_POS        (0 << 3)//0 +/- order
#define CSI2_CLOCK_POL_NEG        (1 << 3)//1 -/+ oorder

#define CSI2_DATA1_POS(x)         (x << 4)
#define CSI2_DATA1_POL_POS        (0 << 7)
#define CSI2_DATA1_POL_NEG        (1 << 7)

#define CSI2_DATA2_POS(x)         (x << 8)
#define CSI2_DATA2_POL_POS        (0 << 11)
#define CSI2_DATA2_POL_NEG        (1 << 11)

#define CSI2_DATA3_POS(x)         (x << 12)
#define CSI2_DATA3_POL_POS        (0 << 15)
#define CSI2_DATA3_POL_NEG        (1 << 15)

#define CSI2_DATA4_POS(x)         (x << 16)
#define CSI2_DATA4_POL_POS        (0 << 19)
#define CSI2_DATA4_POL_NEG        (1 << 19)

#define CSI2_COMPLEXIO_CFG_PWRAUTO_ENABLE  (1 << 24) //auto switch from ULPD to ON state
#define CSI2_COMPLEXIO_CFG_PWRAUTO_DISABLE (0 << 24) //auto switch disabled
/* 26-25 shows power status */
#define CSI2_COMPLEXIO_CFG_PWRSTATUS_OFF   (0 << 25)
#define CSI2_COMPLEXIO_CFG_PWRSTATUS_ON    (1 << 25)
#define CSI2_COMPLEXIO_CFG_PWRSTATUS_ULPD  (2 << 25)
/* 28-27 is for power command */
#define CSI2_COMPLEXIO_CFG_PWRCMD_OFF      (0 << 27)
#define CSI2_COMPLEXIO_CFG_PWRCMD_ON       (1 << 27)
#define CSI2_COMPLEXIO_CFG_PWRCMD_ULPD     (2 << 27)
#define CSI2_COMPLEXIO_CFG_PWRCMD_MASK     (3 << 27)

#define CSI2_VP_OUT_CTRL_NODIV   0
#define CSI2_VP_OUT_CTRL_DIV2    1
#define CSI2_VP_OUT_CTRL_DIV3    2
#define CSI2_VP_OUT_CTRL_DIV4    3

#define CSI2_CTRL_IFEN           (1 << 0)
#define CSI2_CTRL_ECC_EN         (1 << 2)
#define CSI2_CTRL_FRAME          (1 << 3)
#define CSI2_CTRL_VP_OUT_CTRL(x) (x << 8)
#define CSI2_CTRL_VP_CLK_EN      (1 << 15)
#define CSI2_CTRL_VP_ONLY_EN     (1 << 11)

#define CSI2_TIMING_FORCE_RX_MODE_IO1 (1 << 15) //1 - Assert force RX mode 0 - deassert

#define CSI2_CTX_CTRL1_LINEMODULO_MULTIPLE (1 << 1)
#define CSI2_CTX_CTRL1_COUNT_LOCK        (1 << 4)
#define CSI2_CTX_CTRL1_CS_EN             (1 << 5)
#define CSI2_CTX_CTRL1_EOL_EN            (1 << 6)
#define CSI2_CTX_CTRL1_EOF_EN            (1 << 7)
#define CSI2_CTX_CTRL1_FRAMECOUNT        (0 << 8)
#define CSI2_CTX_FECNUMBER               (1 << 16)


#define CSI2_CTX_CTRL2_FORMAT_YUV4228BIT     0x1E
#define CSI2_CTX_CTRL2_FORMAT_YUV42210BIT    0x1F
#define CSI2_CTX_CTRL2_FORMAT_RGB565         0x22
#define CSI2_CTX_CTRL2_FORMAT_YUV4228BIT_VP  0x9E
#define CSI2_CTX_CTRL2_FORMAT_RAW_VP         0x12A
#define CSI2_CTX_CTRL2_FORMAT_RAW10_VP       0x12F
#define CSI2_CTX_CTRL2_FORMAT_RAW8           0x2A
#define CSI2_CTX_CTRL2_FORMAT_RAW10          0x2B
#define CSI2_CTX_CTRL2_FORMAT_RAW12          0x2C

#define CSI2_CTX_CTRL2_FORMAT_RAW10_EXP      0xAB
#define CSI2_CTX_CTRL2_FORMAT(x)         (x << 0) //0x1E - YUV4228bit, 0x22 - RGB565
#define CSI2_CTX_CTRL2_DPCM_PRED_SIMPLE  (1 << 10)//Simple predictor used
#define CSI2_CTX_CTRL2_VIRTUALID(x)      (x << 11)

#define CSI2_CTX_IRQENABLE_FS      (1 << 0)
#define CSI2_CTX_IRQENABLE_FE      (1 << 1)
#define CSI2_CTX_IRQENABLE_LS      (1 << 2)
#define CSI2_CTX_IRQENABLE_LE      (1 << 3)
#define CSI2_CTX_IRQENABLE_CS     (1 << 5)
#define CSI2_CTX_IRQENABLE_FN     (1 << 6)
#define CSI2_CTX_IRQENABLE_LN     (1 << 7)


/* CSI2PHYS_SCP register  */
/* Base Addr = 0x480BD970 */

#define CSI2PHYS_SCP_BASE_ADDR 0x480BD970

typedef volatile struct {
   UINT32 CSI2PHYS_CFG0;
   UINT32 CSI2PHYS_CFG1;
} OMAP_CSI2PHYS_SCP_REGS;


/* ISP_CCDC Register Summary */
/* Base Address : 0x480BC600 */
#define OMAP_CAMCCDC_REGS_PA    0x480BC600

typedef volatile struct {

  UINT32 CCDC_PID;               //0x00
  UINT32 CCDC_PCR;               //0x04
  UINT32 CCDC_SYN_MODE;          //0x08
  UINT32 CCDC_HD_VD_WID;         //0x0C
  UINT32 CCDC_PIX_LINES;         //0x10
  UINT32 CCDC_HORZ_INFO;         //0x14
  UINT32 CCDC_VERT_START;        //0x18
  UINT32 CCDC_VERT_LINES;        //0x1C
  UINT32 CCDC_CULLING;           //0x20
  UINT32 CCDC_HSIZE_OFF;         //0x24
  UINT32 CCDC_SDOFST;            //0x28
  UINT32 CCDC_SDR_ADDR;          //0x2c
  UINT32 CCDC_CLAMP;             //0x30
  UINT32 CCDC_DCSUB;             //0x34
  UINT32 CCDC_COLTPIN;           //0x38
  UINT32 CCDC_BLKCMP;            //0x3C
  UINT32 CCDC_FPC;               //0x40
  UINT32 CCDC_FPC_ADDR;          //0x44
  UINT32 CCDC_VDINT;             //0x48
  UINT32 CCDC_ALAW;              //0x4C
  UINT32 CCDC_REC656IF;          //0x50
  UINT32 CCDC_CFG;               //0x54
  UINT32 CCDC_FMTCFG;            //0x58
  UINT32 CCDC_FMT_HORZ;          //0x5C
  UINT32 CCDC_FMT_VERT;          //0x60
  UINT32 CCDC_FMT_ADDR0;         //0x64
  UINT32 CCDC_FMT_ADDR1;         //0x68
  UINT32 CCDC_FMT_ADDR2;         //0x6C
  UINT32 CCDC_FMT_ADDR3;         //0x70
  UINT32 CCDC_FMT_ADDR4;         //0x74
  UINT32 CCDC_FMT_ADDR5;         //0x78
  UINT32 CCDC_FMT_ADDR6;         //0x7C
  UINT32 CCDC_FMT_ADDR7;         //0x80
  UINT32 CCDC_PRGEVEN0;          //0x84
  UINT32 CCDC_PRGEVEN1;          //0x88
  UINT32 CCDC_PRGODD0;           //0x8C
  UINT32 CCDC_PRGODD1;           //0x90
  UINT32 CCDC_VP_OUT;            //0x94
  UINT32 CCDC_LSC_CONFIG;        //0x98
  UINT32 CCDC_LSC_INITIAL;       //0x9C
  UINT32 CCDC_LSC_TABLE_BASE;    //0xA0
  UINT32 CCDC_LSC_TABLE_OFFSET;  //0xA4
} OMAP_CAM_CCDC_REGS;

/* Defines for ISP_CCDC */

/* CCDC_PCR  - Peripheral control register   */
#define CCDC_PCR_BUSY        (1 << 1) //Readonly bit2
#define CCDC_PCR_ENABLE      (1 << 0) // 1-Enable,0-Disable

/* CCDC_SYN_MODE - Sync and mode set register */
#define CCDC_SYN_SDR2RSZ (1 << 19) // 1-Enable,0-Disable
#define CCDC_SYN_VP2SDR  (1 << 18) // 1-Enable,0-Disable
#define CCDC_SYN_WEN     (1 << 17) // 1-Enable,0-Disable
#define CCDC_SYN_VDHDEN  (1 << 16) // 1-Enable,0-Disable
#define CCDC_SYN_FLDSTAT (1 << 15) // 1-Even,0-Odd field
#define CCDC_SYN_LPF     (1 << 14) // 1-Enable,0-Disable
#define CCDC_SYN_INPMOD_RAW     (0 << 12)
#define CCDC_SYN_INPMOD_16YCBCR (1 << 12)
#define CCDC_SYN_INPMOD_8YCBCR  (2 << 12)
#define CCDC_SYN_PACK8   (1 << 11) // 1-Pack(8bits/Pixel),0-Normal(16bits/Pixel)
#define CCDC_SYN_DATSIZ_8BRIDGE  (0 << 8)
#define CCDC_SYN_DATSIZ_12BITS   (4 << 8)
#define CCDC_SYN_DATSIZ_11BITS   (5 << 8)
#define CCDC_SYN_DATSIZ_10BITS   (6 << 8)
#define CCDC_SYN_DATSIZ_08BITS   (7 << 8)
#define CCDC_SYN_FLDMODE (1 << 7) //1-Interlaced,0-Progressive
#define CCDC_SYN_DATAPOL (1 << 6) //1-1's complement,0-Normal
#define CCDC_SYN_EXWEN   (1 << 5) //1-Ex write enabled,0-disabled
#define CCDC_SYN_FLDPOL  (1 << 4) //1-Negative,0-Positive
#define CCDC_SYN_HDPOL   (1 << 3) //1-Negative,0-Positive
#define CCDC_SYN_VDPOL   (1 << 2) //1-Negative,0-Positive
#define CCDC_SYN_FLDOUT  (1 << 1) //1-Output, 0-Input
#define CCDC_SYN_VDHDOUT (1 << 0) //1-Output, 0-Input

/* CCDC_SDOFST - Memory offset register */
#define CCDC_SDOFST_FIINV        (1 << 14) //1-Inverse,0-Noninverse
#define CCDC_SDOFST_FOFST_1LINE  (0 << 12)
#define CCDC_SDOFST_FOFST_2LINE  (1 << 12)
#define CCDC_SDOFST_FOFST_3LINE  (2 << 12)
#define CCDC_SDOFST_FOFST_4LINE  (3 << 12)
#define CCDC_SDOFST_LOFST0_1LINE  (0 << 9)
#define CCDC_SDOFST_LOFST0_2LINE  (1 << 9)
#define CCDC_SDOFST_LOFST0_3LINE  (2 << 9)
#define CCDC_SDOFST_LOFST0_4LINE  (3 << 9)
#define CCDC_SDOFST_LOFST0_N1LINE (4 << 9)
#define CCDC_SDOFST_LOFST0_N2LINE (5 << 9)
#define CCDC_SDOFST_LOFST0_N3LINE (6 << 9)
#define CCDC_SDOFST_LOFST0_N4LINE (7 << 9)
#define CCDC_SDOFST_LOFST1_1LINE  (0 << 6)
#define CCDC_SDOFST_LOFST1_2LINE  (1 << 6)
#define CCDC_SDOFST_LOFST1_3LINE  (2 << 6)
#define CCDC_SDOFST_LOFST1_4LINE  (3 << 6)
#define CCDC_SDOFST_LOFST1_N1LINE (4 << 6)
#define CCDC_SDOFST_LOFST1_N2LINE (5 << 6)
#define CCDC_SDOFST_LOFST1_N3LINE (6 << 6)
#define CCDC_SDOFST_LOFST1_N4LINE (7 << 6)
#define CCDC_SDOFST_LOFST2_1LINE  (0 << 3)
#define CCDC_SDOFST_LOFST2_2LINE  (1 << 3)
#define CCDC_SDOFST_LOFST2_3LINE  (2 << 3)
#define CCDC_SDOFST_LOFST2_4LINE  (3 << 3)
#define CCDC_SDOFST_LOFST2_N1LINE (4 << 3)
#define CCDC_SDOFST_LOFST2_N2LINE (5 << 3)
#define CCDC_SDOFST_LOFST2_N3LINE (6 << 3)
#define CCDC_SDOFST_LOFST2_N4LINE (7 << 3)
#define CCDC_SDOFST_LOFST3_1LINE  (0 << 0)
#define CCDC_SDOFST_LOFST3_2LINE  (1 << 0)
#define CCDC_SDOFST_LOFST3_3LINE  (2 << 0)
#define CCDC_SDOFST_LOFST3_4LINE  (3 << 0)
#define CCDC_SDOFST_LOFST3_N1LINE (4 << 0)
#define CCDC_SDOFST_LOFST3_N2LINE (5 << 0)
#define CCDC_SDOFST_LOFST3_N3LINE (6 << 0)
#define CCDC_SDOFST_LOFST3_N4LINE (7 << 0)

/* CCDC_CLAMP - Clamp control register */
#define CCDC_CLAMP_CLAMPEN       (1 << 31)

#define CCDC_CLAMP_OBSLEN(x)     (x << 28)
#define CCDC_CLAMP_OBSLN(x)      (x << 25)
#define CCDC_CLAMP_OBST(x)       (x << 10)
#define CCDC_CLAMP_OBGAIN(x)     (x << 0)


/* CCDC_BCLAMP */
#define CCDC_BLKCMP_R_Ye(x)     (x << 0)
#define CCDC_BLKCMP_Gr_Cy(x)    (x << 8)
#define CCDC_BLKCMP_Gb_G(x)     (x << 16)
#define CCDC_BLKCMP_B_Mg(x)     (x << 24)

/* CCDC_CULLING */
#define CCDC_CULLING_CULV(x)     (x << 0)
#define CCDC_CULLING_CULHODD(x)     (x << 16)
#define CCDC_CULLING_CULHEVN(x)     (x << 24)

/* CCDC_DCSUB */
#define CCDC_DCSUB(x)            (x << 0)

/* CCDC_COLPTN */
#define PRV_PCR_SCOMP_SFT(x) (x << 22) //shading compensation shift value

#define CCDC_COLPTN_R_Ye            0x0
#define CCDC_COLPTN_Gr_Cy           0x1
#define CCDC_COLPTN_Gb_G            0x2
#define CCDC_COLPTN_B_Mg            0x3
#define CCDC_COLPTN_CP0PLC0_SHIFT       0
#define CCDC_COLPTN_CP0PLC1_SHIFT       2
#define CCDC_COLPTN_CP0PLC2_SHIFT       4
#define CCDC_COLPTN_CP0PLC3_SHIFT       6
#define CCDC_COLPTN_CP1PLC0_SHIFT       8
#define CCDC_COLPTN_CP1PLC1_SHIFT       10
#define CCDC_COLPTN_CP1PLC2_SHIFT       12
#define CCDC_COLPTN_CP1PLC3_SHIFT       14
#define CCDC_COLPTN_CP2PLC0_SHIFT       16
#define CCDC_COLPTN_CP2PLC1_SHIFT       18
#define CCDC_COLPTN_CP2PLC2_SHIFT       20
#define CCDC_COLPTN_CP2PLC3_SHIFT       22
#define CCDC_COLPTN_CP3PLC0_SHIFT       24
#define CCDC_COLPTN_CP3PLC1_SHIFT       26
#define CCDC_COLPTN_CP3PLC2_SHIFT       28
#define CCDC_COLPTN_CP3PLC3_SHIFT       30

/* CCDC_CFG */

#define CCDC_CFG_WENLOG         (1 << 8)  //1-Internal signal & ext WEN ORed(default is ANDed)
#define CCDC_CFG_Y8POS          (1 << 11) //1-Odd Pixel, 0-Even Pixel (Loc of Y in YCbCr)
#define CCDC_CFG_BWSP           (1 << 12) //1-Byte Swap enabled,0-Byte Swap disabled
#define CCDC_CFG_VDLC           (1 << 15) //1-Not latched on VS, 0 - latched on VS
/* CCDC_FPC - Fault pixel correction register */
#define CCDC_FPC_FPERR     (1 << 16) //1-Error,0-No Error
#define CCDC_FPC_FPCEN     (1 << 15) //1-Enable,0-Disable
#define CCDC_FPC_FPNUM(x)  (x << 0)

/* CCDC FMT registers */
#define CCDC_FMT_EN         (1 << 0)
#define CCDC_FMT_LN_ALT(x)      (x << 1)
#define CCDC_FMT_LN_NUM(x)      (x << 2)
#define CCDC_FMT_PLEN_ODD(x)    (x << 4)
#define CCDC_FMT_PLEN_EVEN(x)   (x << 8)
#define CCDC_FMT_VPIN(x)        (x << 12)
#define CCDC_FMT_VPEN(x)        (x << 15)
#define CCDC_FMT_VPIF_FREQ(x)   (x << 16)

#define CCDC_VPIF_FRQ_MASK  (3 << 16)
#define CCDC_FMT_HORZ_LNH(x)    (x << 0)
#define CCDC_FMT_HORZ_SPH(x)    (x << 16)
#define CCDC_FMT_VERT_LNH(x)    (x << 0)
#define CCDC_FMT_VERT_SPH(x)    (x << 16)

/* CCDC ALAW */
#define CCDC_ALAW_GWDI(x)    (x << 0)
#define CCDC_ALAW_CCDTBL_ENABLE  (1 << 3)

/* CCDC LSC */
#define CCDC_LSC_ENABLE    (1 << 0)
#define CCDC_LSC_GAIN_FORMAT(x) (x << 1)
#define CCDC_LSC_GAIN_MODE_N(x) (x << 8)
#define CCDC_LSC_GAIN_MODE_M(x) (x << 12)
#define CCDC_LSC_AFTER_REFORMATTER(x) (x << 6)
#define CCDC_LSC_INITIAL_X(x) (x << 0)
#define CCDC_LSC_INITIAL_Y(x) (x << 16)
#define ISP_PRV_BASE_ADDRESS 0x480BCE00

typedef volatile struct {
  UINT32 PRV_PID;       //0x0000
  UINT32 PRV_PCR;       //0x4
  UINT32 PRV_HORZ_INFO; //0x8
  UINT32 PRV_VERT_INFO; //0xC
  UINT32 PRV_RSDR_ADDR; //0x10
  UINT32 PRV_RADR_OFST; //0x14
  UINT32 PRV_DSDR_ADDR; //0x18
  UINT32 PRV_DRKF_OFST; //0x1C
  UINT32 PRV_WSDR_ADDR; //0x20
  UINT32 PRV_WADD_OFST; //0x24
  UINT32 PRV_AVE;       //0x28
  UINT32 PRV_HMED;      //0x2C
  UINT32 PRV_NF;        //0x30
  UINT32 PRV_WB_DGAIN;  //0x34
  UINT32 PRV_WBGAIN;    //0x38
  UINT32 PRV_WBSEL;     //0x3C
  UINT32 PRV_CFA;       //0x40
  UINT32 PRV_BLKADJOFF; //0x44
  UINT32 PRV_RGB_MAT1;  //0x48
  UINT32 PRV_RGB_MAT2;  //0x4C
  UINT32 PRV_RGB_MAT3;  //0x50
  UINT32 PRV_RGB_MAT4;  //0x54
  UINT32 PRV_RGB_MAT5;  //0x58
  UINT32 PRV_RGB_OFF1;  //0x5C
  UINT32 PRV_RGB_OFF2;  //0x60
  UINT32 PRV_CSC0;      //0x64
  UINT32 PRV_CSC1;      //0x68
  UINT32 PRV_CSC2;      //0x6C
  UINT32 PRV_CSC_OFFSET;//0x70
  UINT32 PRV_CNT_BRT;   //0x74
  UINT32 PRV_CSUP;      //0x78
  UINT32 PRV_SETUP_YC;  //0x7C
  UINT32 PRV_SET_TBL_ADDR;//0x80
  UINT32 PRV_SET_TBL_DATA;//0x84
  UINT32 PRV_NOREG1;//0x88
  UINT32 PRV_NOREG2;//0x8C
  UINT32 PRV_CDC_THR0;//0x90
  UINT32 PRV_CDC_THR1;//0x94  
  UINT32 PRV_CDC_THR2;//0x98
  UINT32 PRV_CDC_THR3;//0x9C
}OMAP_ISP_PRV_REGS;

/* Preview Engine defines (bit fields) */
#define PRV_PCR_DRK_FAIL  (1 << 31) //1 - Error 0 - no error
#define PRV_PCR_DCORMETHOD (1 << 28) //0 - Disable defect correction method
#define PRV_PCR_DCOREN    (1 << 27) //0 - Disable defect correction 1- enable
#define PRV_PCR_GAMMA_BYPASS (1 << 26) //0 - no bypass, 1 - bypass,o/p is set to 8 MSB of 10-bit i/p

#define PRV_PCR_SCOMP_EN  (1 << 21) //0 - Disable 1 - Enable Dark frame subtract
#define PRV_PCR_SDRPORT   (1 << 20) //0 - Disable 1 - Enable PRV o/p 2 memory
#define PRV_PCR_RSZPORT   (1 << 19) //0 - Disable 1 - Enable PRV o/p 2 RSZ
#define PRV_PCR_YCPOS(x)  (x << 17) //YC position
#define PRV_PCR_SUPEN     (1 << 16) //0 - Disable 1 - Enable color suppression
#define PRV_PCR_YNENHEN   (1 << 15) //0 - Disable 1 - Enable non-linear enhancer
#define PRV_PCR_CFAFMT(x) (x << 11) //CFA format
                                    //0 - Mode0 : conventional bayer
                                    //1 - Mode1 : horz 2x downsample
                                    //2 - Mode2 : bypass CFA stage(Foven)
                                    //3 - Mode3 : horz and vert 2x downsample
                                    //4 - Mode4 : Fuji honeycom movie modesensor
                                    //5 - Mode5 : bypass CFA stage(RRRR GGGG BBBB)
#define PRV_PCR_CFAEN     (1 << 10)//CFA Enable 0 - disable 1 - enable
#define PRV_PCR_NFEN      (1 <<  9)//Noise filter enable 0 - disable 1 - enable
#define PRV_PCR_HMEDEN    (1 <<  8)//Horz median filter enable
#define PRV_PCR_DRKFCAP   (1 <<  7)//Dark frame capt enable
#define PRV_PCR_DRKFEN    (1 <<  6)//Subtract dark frame enable
#define PRV_PCR_INVALAW   (1 <<  5)//Inverse A-Law enable
#define PRV_PCR_WIDTH     (1 <<  4)//i/p data width selection
                                   //0x0 - 10-bit mode 0x1 - 8-bit mode
#define PRV_PCR_ONESHOT   (1 <<  3)//one shot mode 0 - continous 1 - oneshot
#define PRV_PCR_SOURCE    (1 <<  2)//i/p src select 0 - video port(ccdc) 1- memory
#define PRV_PCR_BUSY      (1 <<  1)//PRV busy bit
#define PRV_PCR_ENABLE    (1 <<  0)//PRV enable bit


#define PRV_WBGAIN_COEFF0(x) (x << 0)
#define PRV_WBGAIN_COEFF1(x) (x << 8)
#define PRV_WBGAIN_COEFF2(x) (x << 16)
#define PRV_WBGAIN_COEFF3(x) (x << 24)
#define PRV_DGAIN(x) (x << 0)

#define PRV_BRT_UPDATE(x)  (x << 0)
#define PRV_CNT_UPDATE(x)  (x << 8)

#define PRV_SETUPYC_MINC(x) (x << 0)
#define PRV_SETUPYC_MAXC(x) (x << 8)
#define PRV_SETUPYC_MINY(x) (x << 16)
#define PRV_SETUPYC_MAXY(x) (x << 24)

#define PRV_BLKADJOFF_B(x)  (x << 0)
#define PRV_BLKADJOFF_G(x)  (x << 8)
#define PRV_BLKADJOFF_R(x)  (x << 16)

#define PRV_RGB2RGB_MAT1RR(x) (x << 0)
#define PRV_RGB2RGB_MAT1GR(x) (x << 16)
#define PRV_RGB2RGB_MAT2BR(x) (x << 0)
#define PRV_RGB2RGB_MAT2RG(x) (x << 16)
#define PRV_RGB2RGB_MAT3GG(x) (x << 0)
#define PRV_RGB2RGB_MAT3BG(x) (x << 16)
#define PRV_RGB2RGB_MAT4GB(x) (x << 16)
#define PRV_RGB2RGB_MAT4RB(x) (x << 0)
#define PRV_RGB2RGB_MAT5BB(x) (x << 0)
#define PRV_RGB2RGB_MTX_OFF1G(x) (x << 0)
#define PRV_RGB2RGB_MTX_OFF1R(x) (x << 16)
#define PRV_RGB2RGB_MTX_OFF2B(x) (x << 0)

#define PRV_HORZMED_THRESHOLD(x) (x << 0)
#define PRV_HORZMED_EVENDIST(x)  (x << 8)
#define PRV_HORZMED_ODDDIST(x)   (x << 9)

#define PRV_AVE_COUNT(x)       (x << 0)
#define PRV_AVE_EVEN_DIST(x)   (x << 3)
#define PRV_AVE_ODD_DIST(x)    (x << 4)

#define PRV_NF_SPREAD(x)       (x << 0)

#define PRV_CDC_THR0_DETECT(x)  (x << 0)
#define PRV_CDC_THR0_CORRECT(x) (x << 16)
#define PRV_CDC_THR1_DETECT(x)  (x << 0)
#define PRV_CDC_THR1_CORRECT(x) (x << 16)
#define PRV_CDC_THR2_DETECT(x)  (x << 0)
#define PRV_CDC_THR2_CORRECT(x) (x << 16)
#define PRV_CDC_THR3_DETECT(x)  (x << 0)
#define PRV_CDC_THR3_CORRECT(x) (x << 16)

#define PRV_CFA_GRAD_HORZ(x)    (x << 0)
#define PRV_CFA_GRAD_VERT(x)    (x << 8)

#define PRV_CSC_CRY(x)    ((x & 0x3ff) << 0)
#define PRV_CSC_CGY(x)    ((x & 0x3ff) << 10)
#define PRV_CSC_CBY(x)    ((x & 0x3ff) << 20)

#define PRV_CSC_RCB(x)    ((x & 0x3ff) << 0)
#define PRV_CSC_GCB(x)    ((x & 0x3ff) << 10)
#define PRV_CSC_BCB(x)    ((x & 0x3ff) << 20)

#define PRV_CSC_RCR(x)    ((x & 0x3ff) << 0)
#define PRV_CSC_GCR(x)    ((x & 0x3ff) << 10)
#define PRV_CSC_BCR(x)    ((x & 0x3ff) << 20)

#define PRV_CSC_OFFSET_CR(x)    ((x & 0xff) << 0)
#define PRV_CSC_OFFSET_CB(x)    ((x & 0xff) << 8)
#define PRV_CSC_OFFSET_Y(x)    ((x & 0xff) << 16)

#define PRV_CSUP_CSUPG(x)   (x << 0)
#define PRV_CSUP_CSUPTH(x)  (x << 8)
#define PRV_CSUP_HPYF(x)    (x << 16)

#define PRV_PCR_SCOMP_SFT(x) (x << 22) //shading compensation shift value

#define PRV_DELAYED_UPDATE_BRTCNT       (1 << 0)
#define PRV_DELAYED_UPDATE_SETUPYC      (1 << 1)
#define PRV_DELAYED_UPDATE_BLKADJOFF    (1 << 2)
#define PRV_DELAYED_UPDATE_RGB2RGB      (1 << 3)
#define PRV_DELAYED_UPDATE_CSCDATA      (1 << 4)
#define PRV_DELAYED_UPDATE_HMED         (1 << 5)
#define PRV_DELAYED_UPDATE_AVE          (1 << 6)
#define PRV_DELAYED_UPDATE_NF           (1 << 7)
#define PRV_DELAYED_UPDATE_DCOR         (1 << 8)
#define PRV_DELAYED_UPDATE_CFA          (1 << 9)
#define PRV_DELAYED_UPDATE_RGB2YUV      (1 << 10)
#define PRV_DELAYED_UPDATE_GAMMACOR     (1 << 11)
#define PRV_DELAYED_UPDATE_LUMAENH      (1 << 12)
#define PRV_DELAYED_UPDATE_DRKFRAME     (1 << 13)
#define PRV_DELAYED_UPDATE_SHADINGCOMP  (1 << 14)
#define PRV_DELAYED_UPDATE_CSUP         (1 << 15)
/* SCM Engine registers */
#ifdef BSP_CAMERA_H3A

#define ISP_HIST_BASE_ADDRESS 0x480BCA00

typedef volatile struct {
  UINT32 HIST_PID;           //0x0000
  UINT32 HIST_PCR;           //0x4
  UINT32 HIST_CNT;           //0x8
  UINT32 HIST_WB_GAIN;       //0xC
  UINT32 HIST_R0_HORZ;       //0x10
  UINT32 HIST_R0_VERT;       //0x14
  UINT32 HIST_R1_HORZ;       //0x18
  UINT32 HIST_R1_VERT;       //0x1C
  UINT32 HIST_R2_HORZ;       //0x20
  UINT32 HIST_R2_VERT;       //0x24
  UINT32 HIST_R3_HORZ;       //0x28
  UINT32 HIST_R3_VERT;       //0x2C
  UINT32 HIST_ADDR;          //0x30
  UINT32 HIST_DATA;          //0x34
  UINT32 HIST_RADD;          //0x38
  UINT32 HIST_RADD_OFF;      //0x3C
  UINT32 HIST_H_V_INFO;      //0x40
} OMAP_ISP_HIST_REGS;

#define HIST_PID_PREV_SHIFT      0
#define HIST_PID_CID_SHIFT       8
#define HIST_PID_TID_SHIFT       16

#define HIST_PCR_ENABLE          (1 << 0)
#define HIST_PCR_BUSY            (1 << 1)

#define HIST_CNT_SHIFT           0
#define HIST_CNT_SHIFT_CLR       0x7
#define HIST_CNT_SOURCE          (1 << 3)
#define HIST_CNT_BINS            4
#define HIST_CNT_BINS_32         0x0
#define HIST_CNT_BINS_64         0x1
#define HIST_CNT_BINS_128        0x2
#define HIST_CNT_BINS_256        0x3
#define HIST_CNT_CFA             (1 << 6)
#define HIST_CNT_CLR             (1 << 7)
#define HIST_CNT_DATSIZE         (1 << 8)

#define HIST_WB_GAIN_WG03        0
#define HIST_WB_GAIN_WG02        8
#define HIST_WB_GAIN_WG01        16
#define HIST_WB_GAIN_WG00        24

#define HIST_R0_HORZ_HEND        0
#define HIST_R0_HORZ_HSTART      16
#define HIST_R0_VERT_VEND        0
#define HIST_R0_VERT_VSTART      16
#define HIST_R1_HORZ_HEND        0
#define HIST_R1_HORZ_HSTART      16
#define HIST_R1_VERT_VEND        0
#define HIST_R1_VERT_VSTART      16
#define HIST_R2_HORZ_HEND        0
#define HIST_R2_HORZ_HSTART      16
#define HIST_R2_VERT_VEND        0
#define HIST_R2_VERT_VSTART      16
#define HIST_R3_HORZ_HEND        0
#define HIST_R3_HORZ_HSTART      16
#define HIST_R3_VERT_VEND        0
#define HIST_R3_VERT_VSTART      16
#define HIST_ADDR_ADDR           0
#define HIST_DATA_RDATA          0
#define HIST_RADD_RADD           0
#define HIST_RADD_OFF_OFFSET     0
#define HIST_H_V_INFO_VSIZE      0
#define HIST_H_V_INFO_HSIZE      16


#define ISP_H3A_BASE_ADDRESS 0x480BCC00

typedef volatile struct {
  UINT32 H3A_PID;           //0x0000
  UINT32 H3A_PCR;           //0x4
  UINT32 H3A_AFPAX1;        //0x8
  UINT32 H3A_AFPAX2;        //0xC
  UINT32 H3A_AFPAXSTART;    //0x10
  UINT32 H3A_AFIIRSH;       //0x14
  UINT32 H3A_AFBUFST;       //0x18
  UINT32 H3A_AFCOEF010;     //0x1C
  UINT32 H3A_AFCOEF032;     //0x20
  UINT32 H3A_AFCOEF054;     //0x24
  UINT32 H3A_AFCOEF076;     //0x28
  UINT32 H3A_AFCOEF098;     //0x2C
  UINT32 H3A_AFCOEF0010;    //0x30
  UINT32 H3A_AFCOEF110;     //0x34
  UINT32 H3A_AFCOEF132;     //0x38
  UINT32 H3A_AFCOEF154;     //0x3C
  UINT32 H3A_AFCOEF176;     //0x40
  UINT32 H3A_AFCOEF198;     //0x44
  UINT32 H3A_AFCOEF1010;    //0x48
  UINT32 H3A_AEWWIN1;       //0x4C
  UINT32 H3A_AEWINSTART;    //0x50
  UINT32 H3A_AEWINBLK;      //0x54
  UINT32 H3A_AEWSUBWIN;     //0x58
  UINT32 H3A_AEWBUFST;      //0x5C
} OMAP_ISP_H3A_REGS;

#define H3A_PID_PREV             0
#define H3A_PID_CID              8
#define H3A_PID_TID              16

#define H3A_PCR_AF_EN            (1 << 0)
#define H3A_PCR_AF_ALAW_EN       (1 << 1)
#define H3A_PCR_AF_MED_EN        (1 << 2)
#define H3A_PCR_MED_TH           3
#define H3A_PCR_RGBPOS           11
#define H3A_PCR_RGBPOS_CLR       0x7
#define H3A_PCR_FVMODE           (1 << 14)
#define H3A_PCR_BUSYAF           (1 << 15)
#define H3A_PCR_AEW_EN           (1 << 16)
#define H3A_PCR_AEW_ALAW_EN      (1 << 17)
#define H3A_PCR_BUSYAEAWB        (1 << 18)
#define H3A_PCR_AVE2LMT          22

#define H3A_AFPAX1_PAXH          0
#define H3A_AFPAX1_PAXW          16

#define H3A_AFPAX2_PAXHC         0
#define H3A_AFPAX2_PAXVC         6
#define H3A_AFPAX2_AFINCV        13

#define H3A_AFPAXSTART_PAXSV     0
#define H3A_AFPAXSTART_PAXSH     16

#define H3A_AFIIRSH_IIRSH        0

#define H3A_AFBUFST_AFBUFST      5

#define H3A_AFCOEF010_COEFF0     0
#define H3A_AFCOEF010_COEFF1     16

#define H3A_AFCOEF032_COEFF2     0
#define H3A_AFCOEF032_COEFF3     16

#define H3A_AFCOEF054_COEFF4     0
#define H3A_AFCOEF054_COEFF5     16

#define H3A_AFCOEF076_COEFF6     0
#define H3A_AFCOEF076_COEFF7     16

#define H3A_AFCOEF098_COEFF8     0
#define H3A_AFCOEF098_COEFF9     16

#define H3A_AFCOEF0010_COEFF10   0

#define H3A_AFCOEF110_COEFF0     0
#define H3A_AFCOEF110_COEFF1     16

#define H3A_AFCOEF132_COEFF2     0
#define H3A_AFCOEF132_COEFF3     16

#define H3A_AFCOEF154_COEFF4     0
#define H3A_AFCOEF154_COEFF5     16

#define H3A_AFCOEF176_COEFF6     0
#define H3A_AFCOEF176_COEFF7     16

#define H3A_AFCOEF198_COEFF8     0
#define H3A_AFCOEF198_COEFF9     16

#define H3A_AFCOEF1010_COEFF10   0

#define H3A_AEWWIN1_WINHC        0
#define H3A_AEWWIN1_WINVC        6
#define H3A_AEWWIN1_WINW         13
#define H3A_AEWWIN1_WINH         24

#define H3A_AEWINSTART_WINSH     0
#define H3A_AEWINSTART_WINSV     16

#define H3A_AEWINBLK_WINH        0
#define H3A_AEWINBLK_WINSV       16

#define H3A_AEWSUBWIN_AEWINCH    0
#define H3A_AEWSUBWIN_AEWINCV    8

#define H3A_AEWBUFST_AEWBUFST    5


#endif /* end of BSP_CAMERA_H3A */

#define ISP_RSZ_BASE_ADDRESS 0x480BD000

typedef volatile struct {
  UINT32 RSZ_PID;           //0x0000
  UINT32 RSZ_PCR;           //0x4
  UINT32 RSZ_CNT;           //0x8
  UINT32 RSZ_OUT_SIZE;      //0xC
  UINT32 RSZ_IN_START;      //0x10
  UINT32 RSZ_IN_SIZE;       //0x14
  UINT32 RSZ_SDR_INADD;     //0x18
  UINT32 RSZ_SDR_INOFF;     //0x1C
  UINT32 RSZ_SDR_OUTADD;    //0x20
  UINT32 RSZ_SDR_OUTOFF;    //0x24
  UINT32 RSZ_HFILT10;       //0x28
  UINT32 RSZ_HFILT32;       //0x2C
  UINT32 RSZ_HFILT54;       //0x30
  UINT32 RSZ_HFILT76;       //0x34
  UINT32 RSZ_HFILT98;       //0x38
  UINT32 RSZ_HFILT1110;     //0x3C
  UINT32 RSZ_HFILT1312;     //0x40
  UINT32 RSZ_HFILT1514;     //0x44
  UINT32 RSZ_HFILT1716;     //0x48
  UINT32 RSZ_HFILT1918;     //0x4C
  UINT32 RSZ_HFILT2120;     //0x50
  UINT32 RSZ_HFILT2322;     //0x54
  UINT32 RSZ_HFILT2524;     //0x58
  UINT32 RSZ_HFILT2726;     //0x5C
  UINT32 RSZ_HFILT2928;     //0x60
  UINT32 RSZ_HFILT3130;     //0x64
  UINT32 RSZ_VFILT10;       //0x68
  UINT32 RSZ_VFILT32;       //0x6C
  UINT32 RSZ_VFILT54;       //0x70
  UINT32 RSZ_VFILT76;       //0x74
  UINT32 RSZ_VFILT98;       //0x78
  UINT32 RSZ_VFILT1110;     //0x7C
  UINT32 RSZ_VFILT1312;     //0x80
  UINT32 RSZ_VFILT1514;     //0x84
  UINT32 RSZ_VFILT1716;     //0x88
  UINT32 RSZ_VFILT1918;     //0x8C
  UINT32 RSZ_VFILT2120;     //0x90
  UINT32 RSZ_VFILT2322;     //0x94
  UINT32 RSZ_VFILT2524;     //0x98
  UINT32 RSZ_VFILT2726;     //0x9C
  UINT32 RSZ_VFILT2928;     //0xA0
  UINT32 RSZ_VFILT3130;     //0xA4
  UINT32 RSZ_YENH;          //0xA8
} OMAP_ISP_RSZ_REGS;

#define RSZ_PID_PREV_SHIFT          0
#define RSZ_PID_CID_SHIFT           8
#define RSZ_PID_TID_SHIFT           16

#define RSZ_PCR_ENABLE              (1 << 0)
#define RSZ_PCR_BUSY                (1 << 1)
#define RSZ_PCR_ONESHOT             (1 << 2)

#define RSZ_CNT_HRSZ_SHIFT          0
#define RSZ_CNT_HRSZ_MASK           0x3FF
#define RSZ_CNT_VRSZ_SHIFT          10
#define RSZ_CNT_VRSZ_MASK           0xFFC00
#define RSZ_CNT_HSTPH_SHIFT         20
#define RSZ_CNT_HSTPH_MASK          0x700000
#define RSZ_CNT_VSTPH_SHIFT         23
#define RSZ_CNT_VSTPH_MASK          0x3800000
#define RSZ_CNT_CBILIN_MASK         0x20000000
#define RSZ_CNT_INPTYP_MASK         0x08000000
#define RSZ_CNT_PIXFMT_MASK         0x04000000
#define RSZ_CNT_YCPOS                   (0 << 26) 
#define RSZ_CNT_INPTYP              (1 << 27)
#define RSZ_CNT_INPSRC              (1 << 28)
#define RSZ_CNT_CBILIN              (1 << 29)
#define RSZ_IN_START_HORZ_ST_SHIFT      0
#define RSZ_IN_START_VERT_ST_SHIFT      16
#define RSZ_IN_SIZE_HORZ_SHIFT      0
#define RSZ_IN_SIZE_VERT_SHIFT      16
#define RSZ_OUT_SIZE_HORZ_SHIFT     0
#define RSZ_OUT_SIZE_VERT_SHIFT     16
#define RSZ_SDR_OUTOFF_OFFSET_SHIFT     0
#define RSZ_HFILT_COEFL_SHIFT       0
#define RSZ_HFILT_COEFH_SHIFT       16

#define RSZ_VFILT_COEFL_SHIFT       0
#define RSZ_VFILT_COEFH_SHIFT       16

// Size bounds for resizer operation
#define RSZ_MAX_IN_HEIGHT                  4095
#define RSZ_MINIMUM_RESIZE_VALUE           64
#define RSZ_MAXIMUM_RESIZE_VALUE           1024
#define RSZ_MID_RESIZE_VALUE               512
#define RSZ_MAX_7TAP_HRSZ_OUTWIDTH_ES2     3300
#define RSZ_MAX_7TAP_VRSZ_OUTWIDTH_ES2     1650
#define RSZ_MAX_IN_WIDTH_ONTHEFLY_MODE_ES2 4095
#define RSZ_DEFAULTSTPIXEL                 0
#define RSZ_DEFAULTSTPHASE                 1
#define RSZ_BYTE_ALIGN                     32

#define RSZ_CNT_HRSZ(x)             (x <<  0) // 9  - 0
#define RSZ_CNT_VRSZ(x)             (x << 10) // 19 - 10
#define RSZ_CNT_HSTPH(x)            (x << 20) // 22 - 20 range 0 - 7
#define RSZ_CNT_VSTPH(x)            (x << 23) // 25 - 23 range 0 - 7
#define RSZ_OUT_SIZE_HORZ(x)        (x <<  0) //10 - 0
#define RSZ_OUT_SIZE_VERT(x)        (x << 16) //26 - 16

#define RSZ_IN_START_HORZ_ST(x)     (x << 0)  //12 - 0
#define RSZ_IN_START_VERT_ST(x)     (x << 16) //28 - 16

#define RSZ_IN_SIZE_HORZ(x)         (x <<  0) //12 - 0
#define RSZ_IN_SIZE_VERT(x)         (x << 16) //28 - 16

#define ISP_SBL_BASE_ADDRESS 0x480BD200

typedef volatile struct {
  UINT32 SBL_PID; //0x00
  UINT32 SBL_PCR; //0x04

  UINT32 SBL_GBL_REG_0; 
  UINT32 SBL_GBL_REG_1; 
  UINT32 SBL_GBL_REG_2; 
  UINT32 SBL_GBL_REG_3; 
  UINT32 SBL_GBL_REG_4; 
  UINT32 SBL_GBL_REG_5; 
  UINT32 SBL_GBL_REG_6; 
  UINT32 SBL_GBL_REG_7; 
  
  UINT32 SBL_CSIA_WR_0; 
  UINT32 SBL_CSIA_WR_1; 
  UINT32 SBL_CSIA_WR_2; 
  UINT32 SBL_CSIA_WR_3; 

  UINT32 SBL_CSIB_WR_0; 
  UINT32 SBL_CSIB_WR_1; 
  UINT32 SBL_CSIB_WR_2; 
  UINT32 SBL_CSIB_WR_3; 

  UINT32 SBL_CCDC_WR0;
  UINT32 SBL_CCDC_WR1;
  UINT32 SBL_CCDC_WR2;
  UINT32 SBL_CCDC_WR3;

  UINT32 SBL_CCDC_FP_RD_0;
  UINT32 SBL_CCDC_FP_RD_1;

  UINT32 SBL_PRV_RD_0;
  UINT32 SBL_PRV_RD_1;
  UINT32 SBL_PRV_RD_2;
  UINT32 SBL_PRV_RD_3;

  UINT32 SBL_PRV_WR_0;
  UINT32 SBL_PRV_WR_1;
  UINT32 SBL_PRV_WR_2;
  UINT32 SBL_PRV_WR_3;

  UINT32 SBL_PRV_DK_RD_0;
  UINT32 SBL_PRV_DK_RD_1;
  UINT32 SBL_PRV_DK_RD_2;
  UINT32 SBL_PRV_DK_RD_3;

  UINT32 SBL_RSZ_RD_0;
  UINT32 SBL_RSZ_RD_1;
  UINT32 SBL_RSZ_RD_2;
  UINT32 SBL_RSZ_RD_3;

  UINT32 SBL_RSZ1_WR_0;
  UINT32 SBL_RSZ1_WR_1;
  UINT32 SBL_RSZ1_WR_2;
  UINT32 SBL_RSZ1_WR_3;

  UINT32 SBL_RSZ2_WR_0;
  UINT32 SBL_RSZ2_WR_1;
  UINT32 SBL_RSZ2_WR_2;
  UINT32 SBL_RSZ2_WR_3;

  UINT32 SBL_RSZ3_WR_0;
  UINT32 SBL_RSZ3_WR_1;
  UINT32 SBL_RSZ3_WR_2;
  UINT32 SBL_RSZ3_WR_3;

  UINT32 SBL_RSZ4_WR_0;
  UINT32 SBL_RSZ4_WR_1;
  UINT32 SBL_RSZ4_WR_2;
  UINT32 SBL_RSZ4_WR_3;

  UINT32 SBL_HIST_RD_0;
  UINT32 SBL_HIST_RD_1;

  UINT32 SBL_H3A_AF_WR_0;
  UINT32 SBL_H3A_AF_WR_1;

  UINT32 SBL_H3A_AEAWB_WR_0;
  UINT32 SBL_H3A_AEAWB_WR_1;

  UINT32 SBL_SDR_REQ_EXP;

}OMAP_ISP_SBL_REGS;


#define SBL_CSIB_WBL_OVF            (1 << 26)
#define SBL_CSIA_WBL_OVF            (1 << 25)
#define SBL_CCDC_WBL_OVF          (1 << 23)
#define SBL_SDR_REQ_RSZ_EXP(x)   ((x & 0x3ff) << 10)
/* Camera MMU Register description */
/* Camera MMU Base - 0x480BD400    */
#define CAM_MMU_BASE_ADDRESS 0x480BD400
/* Move to a separate header file OMAP35XX_MMU.h */
typedef volatile struct {
   UINT32 MMU_REVISION;      //0x00
   UINT32 MMU_Reserved1[3];  //0x04-0x0C reserved
   UINT32 MMU_SYSCONFIG;     //0x10
   UINT32 MMU_SYSSTATUS;     //0x14
   UINT32 MMU_IRQSTATUS;     //0x18
   UINT32 MMU_IRQENABLE;     //0x1C
   UINT32 MMU_Reserved2[8];  //0x20 - 0x3F reserved
   UINT32 MMU_WALKING_ST;    //0x40
   UINT32 MMU_CNTL;          //0x44
   UINT32 MMU_FAULT_AD;      //0x48
   UINT32 MMU_TTB;           //0x4C
   UINT32 MMU_LOCK;          //0x50
   UINT32 MMU_LD_TLB;        //0x54
   UINT32 MMU_CAM;           //0x58
   UINT32 MMU_RAM;           //0x5C
   UINT32 MMU_GFLUSH;        //0x60
   UINT32 MMU_FLUSH_ENTRY;   //0x64
   UINT32 MMU_READ_CAM;      //0x68
   UINT32 MMU_READ_RAM;      //0x6C
   UINT32 MMU_EMU_FAULT_AD;  //0x70
}OMAP_CAM_MMU_REGS;

#define ISPMMU_REVISION_REV_MINOR_MASK  0xF
#define ISPMMU_REVISION_REV_MAJOR_SHIFT 0x4

#define CAMMMU_IDLEMODE_FORCEIDLE    (0 << 3)
#define CAMMMU_IDLEMODE_NOIDLE       (1 << 3)
#define CAMMMU_IDLEMODE_SMARTIDLE    (2 << 3)
#define CAMMMU_SOFTRESET             (1 << 1) //1 - reset issued
#define CAMMMU_AUTOIDLE_AUTO         (1 << 0) //1 - auto interconnect,0- free running

#define CAMMMU_SOFTRESET_DONE        (1 << 0) //1 - reset done, 0 - reset ongoing

#define IRQENABLE_MULTIHITFAULT      (1 << 4)
#define IRQENABLE_TWFAULT            (1 << 3)
#define IRQENABLE_EMUMISS            (1 << 2)
#define IRQENABLE_TRANSLNFAULT       (1 << 1)
#define IRQENABLE_TLBMISS            (1 << 0)

#define CAMMMU_CNTL_MMU_EN           (1 << 1)
#define CAMMMU_CNTL_TWL_EN           (1 << 2)
#define CAMMMU_CNTL_EMUTLBUPDATE     (1 << 3)

#define CAMMMU_SIdlemode_Forceidle    0
#define CAMMMU_SIdlemode_Noidle       1
#define CAMMMU_SIdlemode_Smartidle    2
#define CAMMMU_SIdlemode_Shift        3

typedef enum {
  L_ENDIAN,
  B_ENDIAN
}MMU_ENDIANISM;

typedef enum {
  ES_8BIT,
  ES_16BIT,
  ES_32BIT,
  ES_NOENCONV
}MMU_ELEMENTSIZE;

typedef enum  {
  ACCESS_BASED,
  PAGE_BASED
}MMU_MIXEDSIZE;

typedef enum {
  L1DFAULT,
  PAGE,
  SECTION,
  SUPERSECTION,
  L2DFAULT,
  LARGEPAGE,
  SMALLPAGE
}MMU_MAPSIZE;


#endif