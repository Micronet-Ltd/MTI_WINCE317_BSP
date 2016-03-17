;/*
;===============================================================================
;*             Texas Instruments OMAP(TM) Platform Software
;* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
;*
;* Use of this software is controlled by the terms and conditions found
;* in the license agreement under which this software has been supplied.
;*
;===============================================================================
;*/
;
;  File:  restore.s

        INCLUDE kxarm.h
        INCLUDE am3517_const.inc

        EXPORT OALCPURestoreContext

        TEXTAREA

BEGIN_REGION

sdrc_reg
       DCD         OMAP_SDRC_REGS_PA 
       
;-------------------------------------------------------------------------------
;
;  Function:  OALCPURestoreContext
;
;  This function recovers from mpu OFF or mpu open retention
;
 LEAF_ENTRY OALCPURestoreContext
                                        ;--------------------------------------
                                        ; ES 3.0 errata
                                        ; UNDONE: need errata number
                                        ;
        ldr         r0, sdrc_reg 
                                        ;--------------------------------------
                                        ; clear mrs to enable writes to MR & EMR2 
        ldr         r1, [r0, #SDRC_SYSCONFIG_OA]
        bic         r2, r1, #SDRC_SYSCONFIG_MRS_BIT
        str         r2, [r0, #SDRC_SYSCONFIG_OA]

                                        ;--------------------------------------
                                        ; rewrite to MR0, EMR0. enable autorefresh
        ldr         r1, [r0, #SDRC_MR_0_OA]
        ldr         r2, [r0, #SDRC_EMR2_0_OA]
        str         r1, [r0, #SDRC_MR_0_OA]        
        str         r2, [r0, #SDRC_EMR2_0_OA]
        mov         r3, #SDRC_MANUAL_AUTOREFRESH_CMD
        str         r3, [r0, #SDRC_MANUAL_0_OA]

                                        ;--------------------------------------
                                        ; ewrite to MR1, EMR1. enable autorefresh
        ldr         r1, [r0, #SDRC_MR_1_OA]
        ldr         r2, [r0, #SDRC_EMR2_1_OA]
        str         r1, [r0, #SDRC_MR_1_OA]        
        str         r2, [r0, #SDRC_EMR2_1_OA]
        mov         r3, #SDRC_MANUAL_AUTOREFRESH_CMD
        str         r3, [r0, #SDRC_MANUAL_1_OA]
        nop
        nop
        nop
        nop
        nop

                                        ;--------------------------------------
                                        ; OMAP35XX ROM svc: invalidate entire 
                                        ; L2 cache for GP devices we need to 
                                        ; use ROM services
        mov         r12, #SMI_INVAL_L2
        DCD         SMI

        mov         r1, #0
        mcr         p15, 0, r1, c7, c5, 0
        mcr         p15, 0, r1, c7, c5, 4
        mcr         p15, 0, r1, c7, c5, 6
        mcr         p15, 0, r1, c7, c5, 7
        mcr         p15, 0, r1, c8, c5, 0
        mcr         p15, 0, r1, c8, c6, 0
        
                                        ;--------------------------------------
                                        ; get restore location
        ldr         r2, =OMAP_CONTEXT_RESTORE_REGS_PA
        ldr         r1, [r2, #OEM_CPU_INFO_PA_OFFSET]
        ldr         r0, [r1, #MPU_CONTEXT_PA_OFFSET]
        ldr         r10, [r1, #TLB_INV_FUNC_ADDR_OFFSET]
        
                                        ;--------------------------------------
                                        ; restore content of all registers 
        ldmia       r0!, {r2 - r3}
        msr         spsr_cxsf, r2
        mov         sp, r3
                                        ;--------------------------------------
                                        ; restore coprocessor access control reg
        ldmia       r0!, {r1-r6}
        mcr         p15, 0, r1, c1, c0, 2
                                        ;--------------------------------------
                                        ; restore TTBR0, TTBR1, Trans. tbl base
        mcr         p15, 0, r2, c2, c0, 0
        mcr         p15, 0, r3, c2, c0, 1
        mcr         p15, 0, r4, c2, c0, 2        
                                        ;--------------------------------------
                                        ; Data TLB lockdown, instr. TLB lockdown
        mcr         p15, 0, r5, c10, c0, 0
        mcr         p15, 0, r6, c10, c0, 1
                                        ;--------------------------------------
                                        ; Primary remap, normal remap regs.
        ldmia       r0!, {r1 - r5}
        mcr         p15, 0, r1, c10, c2, 0        
        mcr         p15, 0, r2, c10, c2, 1
                                        ;--------------------------------------
                                        ; secure/non-secure vector base address
                                        ; FCSE PI, Context PID
        mcr         p15, 0, r3, c12, c0, 0
        mcr         p15, 0, r4, c13, c0, 0
        mcr         p15, 0, r5, c13, c0, 1        
                                        ;--------------------------------------
                                        ; domain access control reg
                                        ; data status fault, inst. status fault
                                        ; data aux fault status, 
                                        ; intr. aux fault status,
                                        ; data fault addr, instr fault addr
        ldmia       r0!, {r1 - r7}
        mcr         p15, 0, r1, c3, c0, 0
        mcr         p15, 0, r2, c5, c0, 0
        mcr         p15, 0, r3, c5, c0, 1
        mcr         p15, 0, r4, c5, c1, 0
        mcr         p15, 0, r5, c5, c1, 1
        mcr         p15, 0, r6, c6, c0, 0
        mcr         p15, 0, r7, c6, c0, 2
                                        ;--------------------------------------
                                        ; user r/w thread & proc id
                                        ; user r/o thread and proc id
                                        ; priv only thread and proc id
                                        ; cache size selction
        ldmia       r0!, {r1 - r4}
        mcr         p15, 0, r1, c13, c0, 2
        mcr         p15, 0, r2, c13, c0, 3
        mcr         p15, 0, r3, c13, c0, 4                                        
        mcr         p15, 2, r4, c0, c0, 0        
                                        ;--------------------------------------
                                        ; restore all modes
        mrs         r3, cpsr
                                        ;--------------------------------------
                                        ; fiq mode
        bic         r1, r3, #MODE_MASK
        orr         r1, r1, #FIQ_MODE
        msr         CPSR, r1
        ldmia       r0!, {r7 - r14}
        msr         spsr, r7        
                                        ;--------------------------------------
                                        ; irq mode
        bic         r1, r3, #MODE_MASK
        orr         r1, r1, #IRQ_MODE
        msr         CPSR, r1
        ldmia       r0!, {r7, r13, r14}
        msr         spsr, r7
                                        ;--------------------------------------
                                        ; abort mode
        bic         r1, r3, #MODE_MASK
        orr         r1, r1, #ABORT_MODE
        msr         CPSR, r1
        ldmia       r0!, {r7, r13, r14}
        msr         spsr, r7        
                                        ;--------------------------------------
                                        ; undef mode
        bic         r1, r3, #MODE_MASK
        orr         r1, r1, #UNDEF_MODE
        msr         CPSR, r1
        ldmia       r0!, {r7, r13, r14}
        msr         spsr, r7
                                        ;--------------------------------------
                                        ; system/user mode
        bic         r1, r3, #MODE_MASK
        orr         r1, r1, #SYS_MODE
        msr         CPSR, r1
        ldmia       r0!, {r7, r13, r14}
        msr         spsr, r7                                        
                                        ;--------------------------------------
                                        ; system/user mode
        ldmia       r0!, {r1}        
                                        ;--------------------------------------
                                        ; original mode
        msr         CPSR, r1

;------------------------------------------------------------------------------
; For ARM it's recommended to make the physical address identical to the 
; virtual address, as instruction prefetch can cause problems if not done
; so.  Modify the page table entry corresponding to the code location such
; that the physical address is identical to the virtual address.

                                        ;--------------------------------------
                                        ; The translation base address could be
                                        ; either in TTBR0 or TTBR1 based on
                                        ; N valud of TTBRC.  If (n>0) and 
                                        ; (31:32-N) of VA is 0 use TTBR0
                                        ; else use TTBR1.                                        
        mrc     p15, 0, r1, c2, c0, 2
        and     r1, r1, #0x7
        cmp     r1, #0x0
        mrceq   p15, 0, r9, c2, c0, 0
        mrcne   p15, 0, r9, c2, c0, 1   
                                        ;--------------------------------------
                                        ; declare masks for TTBR bits
                                        ; and to help set the identity
                                        ; mapping in the MMU 1st PTE
        ldr     r6, =TTBRBIT_MASK
        ldr     r5, =MB_BOUNDARY
                                        ;--------------------------------------
                                        ; get location of ttbr and mask out
                                        ; attribute bits
                                        ; (r9) = base addr of PT
        and     r9, r9, r6                      
                                        ;--------------------------------------
                                        ; get physical address of PTE restore
                                        ; point
                                        ; (r8) = phys addr of PTE restore pt
                                        ; (r6) = mb boundary of PTE restore pt
        ldr     r1, =|$PTE_RESTORE|           
        bic     r2, r1, r5
        and     r6, pc, r5
        orr     r8, r2, r6
                                        ;--------------------------------------
                                        ; (r7) = PTE index
                                        ; clear any description bits
        mov     r7, r8, lsr #18
        bic     r7, r7, #DESC_MASK
                                        ;--------------------------------------
                                        ; (r9) = location of PTE
        add     r9, r9, r7
                                        ;--------------------------------------
                                        ; get identity value based on phys
                                        ; addr of restore pt
                                        ; (r5) = identity value to put in the
                                        ; PTE
        mov     r5, r6                   
        orr     r5, r5, #PTL1_KRW
        orr     r5, r5, #PTL1_SECTION
                                        ;--------------------------------------
                                        ; swap value in PTE to create the 
                                        ; identity map
                                        ; (r6) = orig val in PTE
        ldr     r6, [r9]
        str     r5, [r9]
                                        ;--------------------------------------
                                        ; memory barrier
        mov     r5, #0
        mcr     p15, 0, r5, c7, c10, 4
                                        ;--------------------------------------
                                        ; get original control register value
                                        ; (r9) = original control register value
        ldmia   r0!, {r4}
                                        ;--------------------------------------
                                        ; restore original control register
                                        ; w/ cache and MMU *DISABLED*
        bic     r4, r4, #ICACHE_MASK
        bic     r4, r4, #DCACHE_MASK
        bic     r4, r4, #MMU_MASK
        mcr     p15, r0, r4, c1, c0, 0
                                        ;--------------------------------------
                                        ; drain write buffers            
        mcr     p15, 0, r5, c7, c10, 4
                                        ;--------------------------------------
                                        ; enable MMU 
        orr     r4, r4, #MMU_MASK
        mcr     p15, 0, r4, c1, c0, 0
                                        ;--------------------------------------
                                        ; move pc to restore pt; ie 
                                        ; |%PTE_RESTORE|
        mov     pc, r8

                                        ;--------------------------------------
|$PTE_RESTORE|                          ; restore original PTE entry
        nop
        ldr     r9, =WINCE_FIRSTPT
        str     r6, [r9, r7]        

        mov     r1, r10
        ldmia   sp!, {r3 - r12, lr} 
                                        
        mov     pc, r1
 ENTRY_END OALCPURestoreContext
;-------------------------------------------------------------------------------

END_REGION
;-------------------------------------------------------------------------------

        END

