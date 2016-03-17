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
/*
*  File:  omap_sdhc_regs.h
*/
#ifndef __OMAP_SDHC_REGS_H
#define __OMAP_SDHC_REGS_H

#define MMCSLOT_1         1
#define MMCSLOT_2         2

//
//  MMC/SD/SDIO Registers
//

typedef volatile struct
{
    UINT32 unused0[4];
    UINT32 MMCHS_SYSCONFIG;
    UINT32 MMCHS_SYSSTATUS;
    UINT32 unused1[3];
    UINT32 MMCHS_CSRE;
    UINT32 MMCHS_SYSTEST;
    UINT32 MMCHS_CON;
    UINT32 MMCHS_PWCNT;
    UINT32 unused2[52];
    UINT32 MMCHS_BLK;
    UINT32 MMCHS_ARG;
    UINT32 MMCHS_CMD;
    UINT32 MMCHS_RSP10;
    UINT32 MMCHS_RSP32;
    UINT32 MMCHS_RSP54;
    UINT32 MMCHS_RSP76;
    UINT32 MMCHS_DATA;
    UINT32 MMCHS_PSTATE;
    UINT32 MMCHS_HCTL;
    UINT32 MMCHS_SYSCTL;
    UINT32 MMCHS_STAT;
    UINT32 MMCHS_IE;
    UINT32 MMCHS_ISE;
    UINT32 MMCHS_AC12;
    UINT32 MMCHS_CAPA;
    UINT32 unused4[1];
    UINT32 MMCHS_CUR_CAPA;
    UINT32 unused5[44];
    UINT32 MMCHS_REV;
} OMAP_MMCHS_REGS, OMAP_MMCHS_REGS;

// MMCHS_SYSCONFIG register fields

#define MMCHS_SYSCONFIG_AUTOIDLE                (1 << 0)
#define MMCHS_SYSCONFIG_SOFTRESET               (1 << 1)
#define MMCHS_SYSCONFIG_ENAWAKEUP               (1 << 2)
#define MMCHS_SYSCONFIG_SIDLEMODE(mode)         ((mode) << 3)
#define MMCHS_SYSCONFIG_CLOCKACTIVITY(act)      ((act) << 8)

#define SIDLE_FORCE                             (0)
#define SIDLE_IGNORE                            (1)
#define SIDLE_SMART                             (2)

// MMCHS_SYSSTATUS register fields

#define MMCHS_SYSSTATUS_RESETDONE               (1 << 0)

// MMCHS_IE register fields

#define MMCHS_IE_CC                             (1 << 0)
#define MMCHS_IE_TC                             (1 << 1)
#define MMCHS_IE_BGE                            (1 << 2)
#define MMCHS_IE_BWR                            (1 << 4)
#define MMCHS_IE_BRR                            (1 << 5)
#define MMCHS_IE_CIRQ                           (1 << 8)
#define MMCHS_IE_CTO                            (1 << 16)
#define MMCHS_IE_CCRC                           (1 << 17)
#define MMCHS_IE_CEB                            (1 << 18)
#define MMCHS_IE_CIE                            (1 << 19)
#define MMCHS_IE_DTO                            (1 << 20)
#define MMCHS_IE_DCRC                           (1 << 21)
#define MMCHS_IE_DEB                            (1 << 22)
#define MMCHS_IE_ACE                            (1 << 24)
#define MMCHS_IE_CERR                           (1 << 28)
#define MMCHS_IE_BADA                           (1 << 29)

// MMCHS_ISE register fields

#define MMCHS_ISE_CC                            (1 << 0)
#define MMCHS_ISE_TC                            (1 << 1)
#define MMCHS_ISE_BGE                           (1 << 2)
#define MMCHS_ISE_BWR                           (1 << 4)
#define MMCHS_ISE_BRR                           (1 << 5)
#define MMCHS_ISE_CIRQ                          (1 << 8)
#define MMCHS_ISE_CTO                           (1 << 16)
#define MMCHS_ISE_CCRC                          (1 << 17)
#define MMCHS_ISE_CEB                           (1 << 18)
#define MMCHS_ISE_CIE                           (1 << 19)
#define MMCHS_ISE_DTO                           (1 << 20)
#define MMCHS_ISE_DCRC                          (1 << 21)
#define MMCHS_ISE_DEB                           (1 << 22)
#define MMCHS_ISE_ACE                           (1 << 24)
#define MMCHS_ISE_CERR                          (1 << 28)
#define MMCHS_ISE_BADA                          (1 << 29)

// MMCHS_STAT register fields

#define MMCHS_STAT_CC                           (1 << 0)
#define MMCHS_STAT_TC                           (1 << 1)
#define MMCHS_STAT_BGE                          (1 << 2)
#define MMCHS_STAT_BWR                          (1 << 4)
#define MMCHS_STAT_BRR                          (1 << 5)
#define MMCHS_STAT_CIRQ                         (1 << 8)
#define MMCHS_STAT_ERRI                         (1 << 15)
#define MMCHS_STAT_CTO                          (1 << 16)
#define MMCHS_STAT_CCRC                         (1 << 17)
#define MMCHS_STAT_CEB                          (1 << 18)
#define MMCHS_STAT_CIE                          (1 << 19)
#define MMCHS_STAT_DTO                          (1 << 20)
#define MMCHS_STAT_DCRC                         (1 << 21)
#define MMCHS_STAT_DEB                          (1 << 22)
#define MMCHS_STAT_ACE                          (1 << 24)
#define MMCHS_STAT_CERR                         (1 << 28)
#define MMCHS_STAT_BADA                         (1 << 29)

// MMCHS_PSTAT register fields

#define MMCHS_PSTAT_CMDI                        (1 << 0)
#define MMCHS_PSTAT_DATI                        (1 << 1)
#define MMCHS_PSTAT_DLA                         (1 << 2)
#define MMCHS_PSTAT_WTA                         (1 << 8)
#define MMCHS_PSTAT_RTA                         (1 << 9)
#define MMCHS_PSTAT_BWE                         (1 << 10)
#define MMCHS_PSTAT_BRE                         (1 << 11)
#define MMCHS_PSTAT_WP                          (1 << 19)
#define MMCHS_PSTAT_DLEV                        (0xF << 20)
#define MMCHS_PSTAT_CLEV                        (1 << 24)

// MMCHS_HCTL register fields

#define MMCHS_HCTL_DTW                          (1 << 1)
#define MMCHS_HCTL_SDBP                         (1 << 8)
#define MMCHS_HCTL_SDVS(vol)                    ((vol) << 9)
#define MMCHS_HCTL_SBGR                         (1 << 16)
#define MMCHS_HCTL_CR                           (1 << 17)
#define MMCHS_HCTL_RWC                          (1 << 18)
#define MMCHS_HCTL_IBG                          (1 << 19)
#define MMCHS_HCTL_IWE                          (1 << 24)

#define MMCHS_HCTL_SDVS_1V8                     (5 << 9)
#define MMCHS_HCTL_SDVS_3V0                     (6 << 9)
#define MMCHS_HCTL_SDVS_3V3                     (7 << 9)

// MMCHS_SYSCTL register fields

#define MMCHS_SYSCTL_ICE                        (1 << 0)
#define MMCHS_SYSCTL_ICS                        (1 << 1)
#define MMCHS_SYSCTL_CEN                        (1 << 2)
#define MMCHS_SYSCTL_CLKD(clkd)                 ((clkd) << 6)
#define MMCHS_SYSCTL_DTO(dto)                   ((dto) << 16)
#define MMCHS_SYSCTL_SRA                        (1 << 24)
#define MMCHS_SYSCTL_SRC                        (1 << 25)
#define MMCHS_SYSCTL_SRD                        (1 << 26)

#define MMCHS_SYSCTL_DTO_MASK                   (0xF0000)
#define MMCHS_SYSCTL_CLKD_MASK                  (0xFFC0)

// MMCHS_CMD register fields

#define MMCHS_CMD_DE                            (1 << 0)
#define MMCHS_CMD_BCE                           (1 << 1)
#define MMCHS_CMD_ACEN                          (1 << 2)
#define MMCHS_CMD_DDIR                          (1 << 4)
#define MMCHS_CMD_MSBS                          (1 << 5)
#define MMCHS_CMD_RSP_TYPE                      ((rsp) << 16)
#define MMCHS_CMD_CCCE                          (1 << 19)
#define MMCHS_CMD_CICE                          (1 << 20)
#define MMCHS_CMD_DP                            (1 << 21)
#define MMCHS_CMD_TYPE(cmd)                     ((cmd) << 22)
#define MMCHS_INDX(indx)                        ((indx) << 24)

// MMCHS_CAPA register fields

#define MMCHS_CAPA_TCF(tcf)                     ((tcf) << 0)
#define MMCHS_CAPA_TCU                          (1 << 7)
#define MMCHS_CAPA_BCF(bcf)                     ((bcf) << 8)
#define MMCHS_CAPA_MBL(mbl)                     ((mbl) << 16)
#define MMCHS_CAPA_HSS                          (1 << 21)
#define MMCHS_CAPA_DS                           (1 << 22)
#define MMCHS_CAPA_SRS                          (1 << 23)
#define MMCHS_CAPA_VS33                         (1 << 24)
#define MMCHS_CAPA_VS30                         (1 << 25)
#define MMCHS_CAPA_VS18                         (1 << 26)

// MMCHS_CON register fields

#define MMCHS_CON_OD                            (1 << 0)
#define MMCHS_CON_INIT                          (1 << 1)
#define MMCHS_CON_HR                            (1 << 2)
#define MMCHS_CON_STR                           (1 << 3)
#define MMCHS_CON_MODE                          (1 << 4)
#define MMCHS_CON_DW8                           (1 << 5)
#define MMCHS_CON_MIT                           (1 << 6)
#define MMCHS_CON_WPP                           (1 << 8)
#define MMCHS_CON_DVAL(v)                       (v << 9)
#define MMCHS_CON_CTPL                          (1 << 11)
#define MMCHS_CON_CEATA                         (1 << 12)
#define MMCHS_CON_OBIP                          (1 << 13)
#define MMCHS_CON_OBIE                          (1 << 14)
#define MMCHS_CON_PADEN                         (1 << 15)
#define MMCHS_CON_CLKEXTFREE                    (1 << 16)

#endif //__OMAP2430_SDHC_H

