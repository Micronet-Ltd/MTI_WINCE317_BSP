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
;  File:  cpu.s

        INCLUDE kxarm.h
        INCLUDE armmacros.s
        INCLUDE am3517_const.inc

        EXPORT OALCPUStart
        EXPORT OALCPUIdle
        EXPORT OALCPUEnd
        EXPORT OALUpdateCoreFreq
        EXPORT OALInvalidateTlb
        EXPORT OALGetTTBR
        EXPORT INTERRUPTS_STATUS

        TEXTAREA

BEGIN_REGION


        ;ldr         r5, =0x480025e0
        ;ldr         r1, =0xFFFF0000
        ;ldr         r2, =0x1C
        ;ldr         r3, [r5]
        ;and         r3, r3, r1
        ;orr         r3, r3, r2
        ;str         r3, [r5]

        
;-------------------------------------------------------------------------------
;
;  Function:  OALCPUStart
;
;  Marker indicating the start of cpu specific assembly. Never should get called
;
 LEAF_ENTRY OALCPUStart
        nop        
 ENTRY_END OALCPUStart
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;
;  constants
;
max_assoc
       DCD         MAX_ASSOCIATIVITY

max_setnum
       DCD         MAX_SETNUMBER         
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;
;  Function:  SaveContext
;
;  This function puts the mpu to OFF
;
SaveContext
                                        ;--------------------------------------
                                        ; save sp before modifiying the stack
        mov        r1, sp   
        stmdb      sp!, {r3 - r12, lr} 

                                        ;--------------------------------------
                                        ; save content of all registers          
        mrs        r2, SPSR
                                        ;--------------------------------------
                                        ; save the stack pointer stored in r1
        mov        r3, r1
        stmia      r0!, {r2 - r3}
                                        ;--------------------------------------
                                        ; save coprocessor access control reg
        mrc        p15, 0, r1, c1, c0, 2

                                        ;--------------------------------------
                                        ; save TTBR0, TTBR1, Trans. tbl base
        mrc        p15, 0, r2, c2, c0, 0
        mrc        p15, 0, r3, c2, c0, 1
        mrc        p15, 0, r4, c2, c0, 2
        
                                        ;--------------------------------------
                                        ; Data TLB lockdown, instr. TLB lockdown
        mrc        p15, 0, r5, c10, c0, 0
        mrc        p15, 0, r6, c10, c0, 1
        stmia      r0!, {r1-r6}

                                        ;--------------------------------------
                                        ; Primary remap, normal remap regs.
        mrc        p15, 0, r1, c10, c2, 0        
        mrc        p15, 0, r2, c10, c2, 1

                                        ;--------------------------------------
                                        ; secure/non-secure vector base address
                                        ; FCSE PI, Context PID
        mrc        p15, 0, r3, c12, c0, 0
        mrc        p15, 0, r4, c13, c0, 0
        mrc        p15, 0, r5, c13, c0, 1
        stmia      r0!, {r1 - r5}
                                        ;--------------------------------------
                                        ; domain access control reg
                                        ; data status fault, inst. status fault
                                        ; data aux fault status, 
                                        ; intr. aux fault status,
                                        ; data fault addr, instr fault addr
        mrc        p15, 0, r1, c3, c0, 0
        mrc        p15, 0, r2, c5, c0, 0
        mrc        p15, 0, r3, c5, c0, 1
        mrc        p15, 0, r4, c5, c1, 0
        mrc        p15, 0, r5, c5, c1, 1
        mrc        p15, 0, r6, c6, c0, 0
        mrc        p15, 0, r7, c6, c0, 2
        stmia      r0!, {r1 - r7}

                                        ;--------------------------------------
                                        ; user r/w thread & proc id
                                        ; user r/o thread and proc id
                                        ; priv only thread and proc id
                                        ; cache size selction
        mrc        p15, 0, r1, c13, c0, 2
        mrc        p15, 0, r2, c13, c0, 3
        mrc        p15, 0, r3, c13, c0, 4                                        
        mrc        p15, 2, r4, c0, c0, 0
        stmia      r0!, {r1 - r4}


                                        ;--------------------------------------
                                        ; save all modes
        mrs        r3, cpsr
                                        ;--------------------------------------
                                        ; fiq mode
        bic        r1, r3, #MODE_MASK
        orr        r1, r1, #FIQ_MODE
        msr        cpsr, r1
        mrs        r7, spsr
        stmia      r0!, {r7 - r14}
                                        ;--------------------------------------
                                        ; irq mode
        bic        r1, r3, #MODE_MASK
        orr        r1, r1, #IRQ_MODE
        msr        cpsr, r1
        mrs        r7, spsr
        stmia      r0!, {r7, r13, r14}
                                        ;--------------------------------------
                                        ; abort mode
        bic        r1, r3, #MODE_MASK
        orr        r1, r1, #ABORT_MODE
        msr        cpsr, r1
        mrs        r7, spsr
        stmia      r0!, {r7, r13, r14}
                                        ;--------------------------------------
                                        ; undef mode
        bic        r1, r3, #MODE_MASK
        orr        r1, r1, #UNDEF_MODE
        msr        cpsr, r1
        mrs        r7, spsr
        stmia      r0!, {r7, r13, r14}
                                        ;--------------------------------------
                                        ; system/user mode
        bic        r1, r3, #MODE_MASK
        orr        r1, r1, #SYS_MODE
        msr        cpsr, r1
        mrs        r7, spsr
        stmia      r0!, {r7, r13, r14}
                                        ;--------------------------------------
                                        ; original mode
        msr        CPSR, r3
                                        ;--------------------------------------
                                        ; control register
        mrc        p15, 0, r4, c1, c0, 0
        stmia      r0!, {r3, r4}
                                        ;--------------------------------------
                                        ; need to flush all cache, copied
                                        ; from cache code
        mrc     p15, 1, r0, c0, c0, 1   ; read clidr
        ands    r3, r0, #0x7000000  
        mov     r3, r3, lsr #23         ; cache level value
        beq     donea               

        mov     r10, #0                 ; start clean at cache level 0
loop1a  add     r2, r10, r10, lsr #1    ; work out 3x current cache level
        mov     r1, r0, lsr r2          ; extract cache type bits from clidr
        and     r1, r1, #7              ; mask of the bits for current cache only
        cmp     r1, #2                  ; see what cache we have at this level
        blt     skipa                   ; skip if no cache, or just i-cache

        mcr     p15, 2, r10, c0, c0, 0  ; select current cache level in cssr
        mov     r1, #0
        mcr     p15, 0, r1, c7, c5, 4   ; prefetch flush to sync the change to the cachesize id reg
        mrc     p15, 1, r1, c0, c0, 0   ; read the new csidr
        and     r2, r1, #7              ; extract the length of the cache lines
        add     r2, r2, #4              ; add 4 (line length offset)        
        ldr     r4, max_assoc
        ands    r4, r4, r1, lsr #3      ; r4 is maximum number on the way size
        clz     r5, r4                  ; r5 find bit position of way size increment        
        ldr     r7, max_setnum
        ands    r7, r7, r1, lsr #13     ; r7 extract max number of the index size

loop2a  mov     r9, r4                  ; r9 is working copy of max way size
loop3a  orr     r11, r10, r9, lsl r5    ; factor way and cache number into r11
        orr     r11, r11, r7, lsl r2    ; factor index number into r11

        mcr     p15, 0, r11, c7, c14, 2 ; clean and invalidate by set/way

        subs    r9, r9, #1              ; decrement the way
        bge     loop3a

        subs    r7, r7, #1              ; decrement the index
        bge     loop2a

skipa   add     r10, r10, #2            ; increment cache number
        cmp     r3, r10
        bgt     loop1a

donea   mov     r10, #0                 ; swith back to cache level 0
        mcr     p15, 2, r10, c0, c0, 0  ; select current cache level in cssr
        
        ldmia   sp!, {r3 - r12, lr} 
        mov     pc, lr      
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;
;  Function:  OALCPUIdle
;
;  This function puts the mpu in suspend.
;  r0 = addr CPUIDLE_INFO
;
 LEAF_ENTRY OALCPUIdle
                                         ;--------------------------------------
                                         ; store register values into stack    
                                         ;
        stmdb      sp!, {r3 - r12, lr} 
                                         ;--------------------------------------
                                         ; check if mpu is going to off    
                                         ;
        ldr        r1, [r0, #MPU_PRM_REGS_OFFSET]
        ldr        r2, [r1, #PRM_PWRSTCTRL_MPU_OA]
        and        r3, r2, #PRM_POWERSTATE_MASK        
        cmp        r3, #PRM_POWERSTATE_OFF_VAL

                                         ;--------------------------------------
                                         ; check for open switch retention
                                         ;
        andne      r3, r2, #PRM_LOGICL1CACHERETSTATE_VAL
        cmpne      r3, #0
        
                                         ;--------------------------------------
                                         ; save register context if going to OFF
                                         ; mode
                                         ;
        mov        r4, r0
        ldreq      r0, [r0, #MPU_CONTEXT_VA_OFFSET]
        bleq       SaveContext
                                         ;--------------------------------------
                                         ; memory barrier    
                                         ;
        mov        r2, #0x0
        mcr        p15, 0, r2, c7, c10, 4        
        mcr        p15, 0, r2, c7, c10, 5    
        nop                       
        dcd        WFI
                         
                                         ;--------------------------------------
                                         ; r1 = CORE_CM regs
                                         ; r2 = CM_CLKSTST_CORE value
                                         ; wait for L3 to lock
                                         ;
        ldr        r1, [r4, #CORE_CM_REGS_OFFSET]
        
|$_OALCPUIdle_L3_Lock_Loop|
        ldr        r2, [r1, #CM_CLKSTST_CORE_OA]
        ands       r2, r2, #CM_CLKSTST_CLKACTIVITY_L3_BIT
        beq        |$_OALCPUIdle_L3_Lock_Loop|        

        ldmia      sp!, {r3 - r12, lr} 
        mov        pc, lr               
 ENTRY_END OALCPUIdle
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;
;  Function:  CPUStall
;
;  loops [r0] amount
;  r0 = amount to loop
;
;  uses: r0
;
CPUStall
        cmp         r0, #0x0
        subne       r0, r0, #0x1
        bne         CPUStall
        
        mov         pc, lr
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;
;  Function:  GPTWait
;
;  number of gptimer ticks to wait
;  r0 = amount of gpt to wait
;  r5 = addr CPUIDLE_INFO
;
;  uses: r0, r1
;
GPTWait                                         
        ldr         r1, [r5, #GPTIMER_REGS_OFFSET]
        ldr         r1, [r1, #TIMER_TCRR_OA]
        add         r0, r0, #1
        add         r0, r0, r1        

                                         ;--------------------------------------
                                         ; loop until the requested amount of
                                         ; ticks went by
                                         ;
|$_GPTWait_Loop|

        ldr         r1, [r5, #GPTIMER_REGS_OFFSET]
        ldr         r1, [r1, #TIMER_TCRR_OA]
        cmp         r1, r0
        bne         |$_GPTWait_Loop|
        
        mov         pc, lr
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;
;  Function:  SDRCEnable
;
;  This function enables/disables access to SDRAM.
;  r0 = 1:enable SDRC iclk, 0:disable SDRC iclk
;  r6 = ref CORE_CM_REGS
;
;  return: none
;
;  uses: r0 - r1
;
SDRCEnable
                                         ;--------------------------------------
                                         ; setup pointers
                                         ; r1 = req SDRC iclk mode
        mov         r1, r0, LSL #CM_ICLKEN1_CORE_SDRC_BIT
        teq         r1, #(1 :SHL: CM_ICLKEN1_CORE_SDRC_BIT)
                
                                         ;--------------------------------------
                                         ; clear/set EN_SDRC in CM_ICLKEN1_CORE
        ldr         r0, [r6, #CM_ICLKEN1_CORE_OA]
        bicne       r0, r0, #(1 :SHL: CM_ICLKEN1_CORE_SDRC_BIT)
        orreq       r0, r0, #(1 :SHL: CM_ICLKEN1_CORE_SDRC_BIT)
        str         r0, [r6, #CM_ICLKEN1_CORE_OA]

                                         ;--------------------------------------
                                         ; wait until EN_SDRC is in the correct
                                         ; state
_SDRCEnable_chk1
        ldr         r0, [r6, #CM_IDLEST1_CORE_OA]
        and         r0, r0, #(1 :SHL: CM_ICLKEN1_CORE_SDRC_BIT)
        teq         r0, r1
        beq         _SDRCEnable_chk1

        bx          lr                   ;return
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;
;  Function:  SetDpllMode
;
;  Puts DPLL in bypass or locked
;  r0 = 5 : enable low power bypass, 6 : fast relock bypass, 7 : no bypass
;  r1 = addr CM_CLKEN_xxx (must be CM_CLKEN_PLL, CM_CLKEN_MPU, CM_CLKEN_IVA2)
;  r5 = addr CPUIDLE_INFO
;
;  uses: r2, r3
;
SetDpllMode

                                         ;--------------------------------------
                                         ; setup pointers
                                         ; r2 = req dpll mode
                                         ; r3 = ref CM_CLKEN_xxx
        mov         r2, r0
        mov         r3, r1
        
                                         ;--------------------------------------
                                         ; check if dpll is already at the
                                         ; requested state
        ldr         r0, [r3]
        and         r1, r0, #CM_CLKEN_PLL_DPLL_MASK
        teq         r1, r2
        beq         |$_SetDpllMode_exit|
            
                                         ;--------------------------------------
                                         ; update dpll to requested state
        bic         r0, r0, #CM_CLKEN_PLL_DPLL_MASK
        orr         r0, r0, r2
        str         r0, [r3]

                                         ;--------------------------------------
                                         ; setup for test loop
        teq         r2, #CM_CLKEN_PLL_DPLL_LOCKED
        movne       r1, #0
        moveq       r1, #1
                                         ;--------------------------------------
                                         ; loop until dpll in desired state
_SetDpllMode_chk1
        ldr         r0, [r3, #CM_IDLEST_CKGEN_OA]
        and         r0, r0, #1
        teq         r0, r1
        bne         _SetDpllMode_chk1

|$_SetDpllMode_exit|
        bx          lr                   ; return
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;
;  Function:  OALUpdateCoreFreq
;
;  changes the core dpll frequency
;
;  r0 = addr CPUIDLE_INFO
;  r1 = value for CM_CLKEN_PLL
;  r2 = value for CM_CLKSEL1_PLL
;
 LEAF_ENTRY OALUpdateCoreFreq

        stmdb       sp!, {r3 - r10, lr}
                                         ;--------------------------------------
                                         ; memory barrier
        mov         r3, #0            
        mcr             p15, 0, r3, c7, c10, 5 

                                         ;--------------------------------------
                                         ; setup pointers                                         
                                         ; r5 = ref SDRC_REGS
                                         ; r6 = ref CORE_CM_REGS
                                         ; r7 = ref CLOCK_CTRL_CM_REGS        
                                         ; r8 = value for CM_CLKEN_PLL
                                         ; r9 = value for CM_CLKSEL1_PLL
        ldr         r5, [r0, #SDRC_REGS_OFFSET]
        ldr         r6, [r0, #CORE_CM_REGS_OFFSET]
        ldr         r7, [r0, #CLOCK_CTRL_CM_REGS_OFFSET]
        mov         r8, r1
        mov         r9, r2
        
                                         ;--------------------------------------
                                         ; Get current DLL mode, to be used 
                                         ; later
        ldr         r2, [r5, #SDRC_DLLA_CTRL_OA]

                                         ;--------------------------------------
                                         ; Determine OPP Transition
        mov         r3, r9 LSR #CM_CLKSEL1_PLL_M2_SHIFT
        cmp         r3, #1

                                         ;--------------------------------------
                                         ; set wait time and idle mask
                                         ; LOW OPP -> DLL unlock mode
                                         ; HIGH OPP -> DLL lock mode
                                         ; GET refresh rate
        movne       r4, #DVFS_LOW_OPP_STALL
        moveq       r4, #DVFS_HIGH_OPP_STALL
        movne       r3, #SDRC_DLLA_STATUS_UNLOCKED
        moveq       r3, #SDRC_DLLA_STATUS_LOCKED

                                         ;--------------------------------------
                                         ; set LOCKDLL for <= 83 mhz
                                         ; clr LOCKDLL for > 83 mhz
        orrne       r2, r2, #(1 :SHL: SDRC_DLL_LOCKDLL_BIT)
        biceq       r2, r2, #(1 :SHL: SDRC_DLL_LOCKDLL_BIT)
        str         r2, [r5, #SDRC_DLLA_CTRL_OA]

                                         ;--------------------------------------
                                         ; restrict access to SDRAM
        mov         r0, #SDRC_DISABLE_ICLK
        bl          SDRCEnable
        
                                         ;--------------------------------------
                                         ; update w/ new m,n values and dpll 
                                         ; configuration
        str         r9, [r7, #CM_CLKSEL1_PLL_OA]                                         
        str         r8, [r7, #CM_CLKEN_PLL_OA]

                                         ;--------------------------------------
                                         ; stall a predefined amount
        cmp         r4, #0
        movne       r0, r4
        blne        CPUStall

                                         ;--------------------------------------
                                         ; allow access to SDRAM
        mov         r0, #SDRC_ENABLE_ICLK
        bl          SDRCEnable          

                                         ;--------------------------------------
                                         ; wait for status to match expected
                                         ; state
_OALUpdateCoreFreq_chk1

        ldr         r2, [r5, #SDRC_DLLA_STATUS_OA]
        cmp         r2, r3
        bne         _OALUpdateCoreFreq_chk1
        
        ldmia       sp!, {r3 - r10, lr}
        bx          lr                   ; return
        
 ENTRY_END OALUpdateCoreFreq
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;
;  Function:  OALCPUEnd
;
;  Marker indicating the end of cpu specific assembly. Never should get called
;
 LEAF_ENTRY OALCPUEnd
        nop        
 ENTRY_END OALCPUEnd
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;
;  Function:  INTERRUPTS_STATUS
;
;  returns current arm interrupts status.
;
 LEAF_ENTRY INTERRUPTS_STATUS

        mrs     r0, cpsr                    ; (r0) = current status
        ands    r0, r0, #0x80               ; was interrupt enabled?
        moveq   r0, #1                      ; yes, return 1
        movne   r0, #0                      ; no, return 0

 ENTRY_END INTERRUPTS_STATUS
 
;-------------------------------------------------------------------------------
;
;  Function:  OALInvalidateTlb
;
;  This function Invalidates the TLBs and enable I and D Cache
;
 LEAF_ENTRY OALInvalidateTlb
        mov        r1, #0
        mrc        p15, 0, r2, c1, c0, 0 ; get control code
        mcr        p15, 0, r1, c8, c7, 0 ; invalidate TLB
        orr        r2, r2, #ICACHE_MASK
        orr        r2, r2, #DCACHE_MASK
        mcr        p15, 0, r2, c1, c0, 0 ; enable i/d cache
        mcr        p15, 0, r1, c7, c10, 4; drain write buffers
        nop                 

        mov        pc, lr                       
    
 ENTRY_END OALInvalidateTlb
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;
;  Function:  OALGetTTBR
;
;  work-around a new kernel feature which marks all non-cached memory
;  as non-executable.
;
 LEAF_ENTRY OALGetTTBR

        mrc         p15, 0, r1, c2, c0, 2 ; determine if using TTBR0 or 1
        and         r1, r1, #0x7
        cmp         r1, #0x0
        mrceq       p15, 0, r0, c2, c0, 0 ; get TTBR from either TTBR0 or 1
        mrcne       p15, 0, r0, c2, c0, 1
        bic         r0, r0, #0x1F          ; clear control bits
        bx          lr 

 ENTRY_END OALGetTTBR

END_REGION
;-------------------------------------------------------------------------------

        END

