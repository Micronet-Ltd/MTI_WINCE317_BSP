; All rights reserved ADENEO EMBEDDED 2010
;
;==============================================================================
;             Texas Instruments OMAP(TM) Platform Software
; (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
;
; Use of this software is controlled by the terms and conditions found
; in the license agreement under which this software has been supplied.
;
;==============================================================================
;
;
;  File: omap35xx_const.inc
;

;------------------------------------------------------------------------------
; Physical Address of Registers
OMAP_CONTEXT_RESTORE_REGS_PA    EQU 0x48002910
OMAP_SDRC_REGS_PA               EQU 0x6D000000

;------------------------------------------------------------------------------
; Useful constants
WFI                             EQU 0xE320F003
SMI                             EQU 0xE1600070
SMI_INVAL_L2                    EQU 0x0001

;------------------------------------------------------------------------------
; following offset must be kept in sync with the CPUIDLE_INFO data structure
; defined in oal_prcm.h

SDRC_REGS_OFFSET                EQU 0x0000
MPU_CM_REGS_OFFSET              EQU 0x0004
CORE_CM_REGS_OFFSET             EQU 0x0008
CLOCK_CTRL_CM_REGS_OFFSET       EQU 0x000C
GPTIMER_REGS_OFFSET             EQU 0x0010
MPU_PRM_REGS_OFFSET             EQU 0x0014
CORE_PRM_REGS_OFFSET            EQU 0x0018
GLOBAL_PRM_REGS_OFFSET			EQU	0x001C
MPU_CONTEXT_PA_OFFSET           EQU 0x0020
MPU_CONTEXT_VA_OFFSET           EQU 0x0024
SDRC_HIGH_RFR_FREQ_OFFSET       EQU 0x0028
SDRC_LOW_RFR_FREQ_OFFSET        EQU 0x002C
TLB_INV_FUNC_ADDR_OFFSET        EQU 0x0030

;------------------------------------------------------------------------------

SDRC_SYSCONFIG_OA               EQU 0x0010
SDRC_MR_0_OA                    EQU 0x0084
SDRC_MR_1_OA                    EQU 0x00B4

SDRC_EMR2_0_OA                  EQU 0x008C
SDRC_EMR2_1_OA                  EQU 0x00BC

SDRC_MANUAL_0_OA                EQU 0x00A8
SDRC_MANUAL_1_OA                EQU 0x00D8

SDRC_POWER_OA                   EQU 0x0070  ; enable/disable SDRAM self-refresh
SDRC_DLLA_CTRL_OA               EQU 0x0060
SDRC_DLLA_STATUS_OA             EQU 0x0064
SDRC_RFR_CTRL_0_OA              EQU 0x00A4
SDRC_RFR_CTRL_1_OA              EQU 0x00D4

SDRC_MANUAL_AUTOREFRESH_CMD     EQU 0x0002

SDRC_POWER_CLKCTRL_MASK         EQU 0x0003

SDRC_SYSCONFIG_MRS_BIT          EQU 0x0100

SDRC_DLL_DLLIDLE_BIT            EQU 0x0004
SDRC_DLL_ENADLL_BIT             EQU 0x0003
SDRC_DLL_LOCKDLL_BIT            EQU 0x0002
SDRC_DLL_DLLPHASE_BIT           EQU 0x0001
SDRC_DLL_TIMING_WAIT_COUNT      EQU 0x0800

SDRC_DLLA_STATUS_UNLOCKED       EQU 0x0000
SDRC_DLLA_STATUS_LOCKED         EQU 0x0004

SDRC_RFR_CTRL0_ARE_SHIFT        EQU 0x0000
SDRC_RFR_CTRL0_ARCV_SHIFT       EQU 0x0008

SDRC_ENABLE_ICLK                EQU 0x0001
SDRC_DISABLE_ICLK               EQU 0x0000

SDRAM_REFRESH_DELAY_VAL         EQU 0x1000  ; time to wait for autorefresh

;------------------------------------------------------------------------------
CM_CLKEN_PLL_OA                 EQU 0x0000
CM_IDLEST_CKGEN_OA              EQU 0x0020
CM_IDLEST_PLL_MPU_OA            EQU 0x0024
CM_CLKSEL1_PLL_OA               EQU 0x0040

CM_IDLEST_ST_MPU_CLK_BIT        EQU 0x0001

CM_CLKSEL1_PLL_M2_SHIFT         EQU 0x001B

CM_CLKEN_PLL_DPLL_LOWBYPASS     EQU 0x0005
CM_CLKEN_PLL_DPLL_HIGHBYPASS    EQU 0x0006
CM_CLKEN_PLL_DPLL_LOCKED        EQU 0x0007
CM_CLKEN_PLL_DPLL_MASK          EQU 0x0007

DVFS_LOW_OPP_STALL              EQU 0x0000
DVFS_HIGH_OPP_STALL             EQU 0x0010 ;0x00C8

;------------------------------------------------------------------------------
CM_CLKSTST_CORE_OA              EQU 0x004C
CM_ICLKEN1_CORE_OA              EQU 0x0010
CM_IDLEST1_CORE_OA              EQU 0x0020

CM_CLKSTST_CLKACTIVITY_L3_BIT   EQU 0x0001
CM_ICLKEN1_CORE_SDRC_BIT        EQU 0x0001

;------------------------------------------------------------------------------
PRM_PWRSTCTRL_MPU_OA            EQU 0x00E0

;------------------------------------------------------------------------------
PRM_RSTCTRL_GLOBAL_OA			EQU 0x0050
PRM_RSTCTRL_RST_GS_BIT			EQU	0x0002

;------------------------------------------------------------------------------
PRM_LOGICL1CACHERETSTATE_VAL    EQU 0x0004
PRM_POWERSTATE_MASK             EQU 0x0003
PRM_POWERSTATE_OFF_VAL          EQU 0x0000
PRM_POWERSTATE_RET_VAL          EQU 0x0001
PRM_POWERSTATE_ON_VAL           EQU 0x0003

;------------------------------------------------------------------------------
TIMER_TCRR_OA                   EQU 0x0028  ; tcrr offset for gptimer1

;------------------------------------------------------------------------------
; location of restore register and offsets to the various fields.
; Must be kept in sync with OMAP_CONTEXT_RESTORE_REGS structure
OEM_CPU_INFO_PA_OFFSET          EQU 0x1C
OEM_CPU_INFO_UA_OFFSET          EQU 0x20

;------------------------------------------------------------------------------
; useful ARM masks
IRQ_MASK                        EQU 0x80    ; IRQ mask value
FIQ_MASK                        EQU 0x40    ; FIQ mask value
MODE_MASK                       EQU 0x1F    ; Processor Mode Mask
TBIT_MASK                       EQU 0x20    ; Thumb mode bit mask 

;------------------------------------------------------------------------------
; ARM modes
USR_MODE                        EQU 0x10    ; user mode
FIQ_MODE                        EQU 0x11    ; Fast Interrupt Mode (FIQ)
IRQ_MODE                        EQU 0x12    ; Interrupt Mode (IRQ)
SUP_MODE                        EQU 0x13    ; Supervisor Mode
ABORT_MODE                      EQU 0x17    ; Abort Mode
UNDEF_MODE                      EQU 0x1B    ; undef Mode
SYS_MODE                        EQU 0x1F    ; sys Mode

;------------------------------------------------------------------------------
; Cache related constants
ICACHE_MASK                     EQU 0x1000
DCACHE_MASK                     EQU 0x0004
MAX_ASSOCIATIVITY               EQU 0x03FF  ; max val of associativity (10-bit)
MAX_SETNUMBER                   EQU 0x7FFF  ; max val for set number (15-bit)

;------------------------------------------------------------------------------
; MMU related constants
MMU_MASK                        EQU 0x0001
DESC_MASK                       EQU 0x0003
PTL1_SECTION                    EQU 0x0002 
PTL1_KRW                        EQU 0x0400  ; bits 10, 11
TTBRBIT_MASK                    EQU 0xFFFFC000
MB_BOUNDARY                     EQU 0xFFF00000
WINCE_FIRSTPT                   EQU 0xFFFD0000 ; from armhigh.inc

;------------------------------------------------------------------------------

        END
