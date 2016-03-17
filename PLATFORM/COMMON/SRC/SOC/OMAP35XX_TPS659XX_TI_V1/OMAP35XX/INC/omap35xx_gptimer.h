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
//  Header:  omap35xx_gptimer.h
//
#ifndef __OMAP35XX_GPTIMER_H
#define __OMAP35XX_GPTIMER_H


//------------------------------------------------------------------------------

typedef volatile struct {
    UINT32 TIDR;                // 0000
    UINT32 zzzReserved[3];
    UINT32 TIOCP;               // 0010
    UINT32 TISTAT;              // 0014
    UINT32 TISR;                // 0018
    UINT32 TIER;                // 001C
    UINT32 TWER;                // 0020
    UINT32 TCLR;                // 0024
    UINT32 TCRR;                // 0028
    UINT32 TLDR;                // 002C
    UINT32 TTGR;                // 0030
    UINT32 TWPS;                // 0034
    UINT32 TMAR;                // 0038
    UINT32 TCAR1;               // 003C
    UINT32 TSICR;               // 0040
    UINT32 TCAR2;               // 0044
    UINT32 TPIR;                // 0x48
    UINT32 TNIR;                // 0x4C
    UINT32 TCVR;                // 0x50
    UINT32 TOCR;                // 0x54
    UINT32 TOWR;                // 0x58
} OMAP_GPTIMER_REGS;


//------------------------------------------------------------------------------

#define GPTIMER_TIOCP_EMUFREE       (1 << 5)
#define GPTIMER_TIOCP_FORCE_IDLE    (0 << 3)
#define GPTIMER_TIOCP_NO_IDLE       (1 << 3)
#define GPTIMER_TIOCP_SMART_IDLE    (2 << 3)
#define GPTIMER_TIOCP_ENAWAKEUP     (1 << 2)
#define GPTIMER_TIOCP_RESET         (1 << 1)
#define GPTIMER_TIOCP_AUTOIDLE      (1 << 0)

//------------------------------------------------------------------------------

#define GPTIMER_TISTAT_RESETDONE    (1 << 0)

//------------------------------------------------------------------------------

#define GPTIMER_TIER_CAPTURE        (1 << 2)
#define GPTIMER_TIER_OVERFLOW       (1 << 1)
#define GPTIMER_TIER_MATCH          (1 << 0)

//------------------------------------------------------------------------------

#define GPTIMER_TWER_CAPTURE        (1 << 2)
#define GPTIMER_TWER_OVERFLOW       (1 << 1)
#define GPTIMER_TWER_MATCH          (1 << 0)

//------------------------------------------------------------------------------

#define GPTIMER_TCLR_PT                 (1 << 12)
#define GPTIMER_TCLR_TRG_OVERFLOWMATCH  (2 << 10)
#define GPTIMER_TCLR_TRG_OVERFLOW       (1 << 10)
#define GPTIMER_TCLR_CE                 (1 << 6)
#define GPTIMER_TCLR_PRE                (1 << 5)
#define GPTIMER_TCLR_PTV_DIV_2          (0 << 2)
#define GPTIMER_TCLR_PTV_DIV_4          (1 << 2)
#define GPTIMER_TCLR_PTV_DIV_8          (2 << 2)
#define GPTIMER_TCLR_PTV_DIV_16         (3 << 2)
#define GPTIMER_TCLR_PTV_DIV_32         (4 << 2)
#define GPTIMER_TCLR_PTV_DIV_64         (5 << 2)
#define GPTIMER_TCLR_PTV_DIV_128        (6 << 2)
#define GPTIMER_TCLR_PTV_DIV_256        (7 << 2)
#define GPTIMER_TCLR_AR                 (1 << 1)
#define GPTIMER_TCLR_ST                 (1 << 0)

//------------------------------------------------------------------------------

#define GPTIMER_TWPS_TMAR           (1 << 4)
#define GPTIMER_TWPS_TTGR           (1 << 3)
#define GPTIMER_TWPS_TLDR           (1 << 2)
#define GPTIMER_TWPS_TCRR           (1 << 1)
#define GPTIMER_TWPS_TCLR           (1 << 0)

//------------------------------------------------------------------------------

#define GPTIMER_TSICR_POSTED        (1 << 2)
#define GPTIMER_TSICR_SFT           (1 << 1)

//------------------------------------------------------------------------------

#define GPTIMER_TISR_TCAR           (1 << 2)
#define GPTIMER_TISR_OVF            (1 << 1)
#define GPTIMER_TISR_MAT            (1 << 0)

//------------------------------------------------------------------------------

#define GPTIMER_TLCR_GPO_CFG        (1 << 14)
#define GPTIMER_TLCR_CAPT_MODE      (1 << 13)
#define GPTIMER_TLCR_PT             (1 << 12)
#define GPTIMER_TLCR_SCPWM          (1 << 7)
#define GPTIMER_TLCR_CE             (1 << 6)
#define GPTIMER_TLCR_PRE            (1 << 5)
#define GPTIMER_TLCR_AR             (1 << 1)
#define GPTIMER_TLCR_ST             (1 << 0)

#define GPTIMER_TLCR_TRG_SHIFT      (10)
#define GPTIMER_TLCR_TCM_SHIFT      (8)
#define GPTIMER_TLCR_PTV_SHIFT      (2)

//------------------------------------------------------------------------------

#endif // __OMAP35XX_GPTIMER_H

