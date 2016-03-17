        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;
        ;
        ;
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        OPT     2       ; disable listing
        INCLUDE kxarm.h
        OPT     1       ; reenable listing
        OPT     128     ; disable listing of macro expansions

        TEXTAREA

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;
        ; TestLoop(iterations)
        ;
        ; Simple function to execute loop of simple instructions
        ;
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        EXPORT InstructionTestLoop

        LEAF_ENTRY InstructionTestLoop

        ; r0 is iteration count

10
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2
        add     r1, r1, r2

; remove 2 instructions for test and branch, 1 more for time to branch
;       add     r1, r1, r2
;       add     r1, r1, r2
        
        subs    r0, r0, #1
        bne     %B10

        ; return the number of instructions per loop
        mov     r0, #100
        mov     pc, lr          ; return


        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;
        ; DWORD GetCacheControl()
        ;
        ; Simple function to get CP15 register 1
        ;
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        EXPORT GetCacheControl

        LEAF_ENTRY GetCacheControl

        mrc     p15, 0, r0, c1, c0, 0
        mov     pc, lr
        
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;
        ; PutCacheControl(DWORD value)
        ;
        ; Simple function to write CP15 register 1
        ;
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        EXPORT PutCacheControl

        LEAF_ENTRY PutCacheControl

        mcr     p15, 0, r0, c1, c0, 0
        mov     pc, lr
        
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;
        ; GpioTestLoop(ToggleCount, ClearRegisterAddress, SetRegisterAddress, BitToToggle)
        ;
        ; Simple function to toggle GPIO pin in loop of simple instructions
        ;
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        EXPORT GpioTestLoop

        LEAF_ENTRY GpioTestLoop

        ; r0 is iteration count
        ; r1 is GPIO clear register
        ; r2 is GPIO set register
        ; r3 is bit to toggle
        
10

        ; 1
        str     r3, [r1]
        str     r3, [r2]
                
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        ; 10
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
                
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        ; 20
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
                
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        ; 30
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
                
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        ; 40
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
                
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        ; 50
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
                
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        ; 60
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
                
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        ; 70
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
                
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        ; 80
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
                
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        ; 90
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
                
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]
        
        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        str     r3, [r1]
        str     r3, [r2]

        ; 100
        str     r3, [r1]
        str     r3, [r2]

        subs    r0, r0, #1
        bne     %B10

        ; return the number of toggles per loop
        mov     r0, #100
        mov     pc, lr          ; return

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;
        ; MemoryWriteTestLoop(IterationCount, MemoryAddress)
        ;
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        EXPORT MemoryWriteTestLoop

        LEAF_ENTRY MemoryWriteTestLoop

        ; r0 is iteration count
        ; r1 is memory address (DWORD)
        
10
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]

        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]

        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]

        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]

        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]

        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]

        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]

        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]

        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]

        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]
        str     r0, [r1]

        subs    r0, r0, #1
        bne     %B10

        ; return the number of DWORDs written per loop
        mov     r0, #100
        mov     pc, lr          ; return

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;
        ; MemoryReadTestLoop(IterationCount, MemoryAddress)
        ;
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        EXPORT MemoryReadTestLoop

        LEAF_ENTRY MemoryReadTestLoop

        ; r0 is iteration count
        ; r1 is memory address (DWORD)
        
10
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]

        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]

        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]

        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]

        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]

        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]

        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]

        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]

        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]

        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]
        ldr     r2, [r1]

        subs    r0, r0, #1
        bne     %B10

        ; return the number of DWORDs read per loop
        mov     r0, #100
        mov     pc, lr          ; return

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;
        ; DWORD MultipleMemoryWriteTestLoop(DWORD ToggleCount, PDWORD MemoryAddress);
        ;
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        EXPORT MultipleMemoryWriteTestLoop

        LEAF_ENTRY MultipleMemoryWriteTestLoop

        ; r0 is iteration count
        ; r1 is memory address (DWORD)

10
        mov     r2, r1
        
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}

        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}

        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}

        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}

        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}

        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}

        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}

        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}

        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}

        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}
        stmia   r2!, {r4, r5, r6, r7}

        subs    r0, r0, #1
        bne     %B10

        ; return the number of DWORDs written per loop
        mov     r0, #400

        mov     pc, lr
        
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;
        ; DWORD MultipleMemoryReadTestLoop(DWORD ToggleCount, PDWORD MemoryAddress);
        ;
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        EXPORT MultipleMemoryReadTestLoop

        LEAF_ENTRY MultipleMemoryReadTestLoop

        ; r0 is iteration count
        ; r1 is memory address (DWORD)

        stmfd   sp!, {r4, r5, r6, r7}

10
        mov     r2, r1
        
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        subs    r0, r0, #1
        bne     %B10

        ; return the number of DWORDs read per loop
        mov     r0, #400

        ldmfd   sp!, {r4, r5, r6, r7}
        mov     pc, lr
        
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;
        ; void MemoryBlockReadTestLoop(DWORD BlockSize, DWORD MemoryAddress)
        ;
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        EXPORT MemoryBlockReadTestLoop
        
        LEAF_ENTRY MemoryBlockReadTestLoop

        stmfd   sp!, {r4, r5, r6, r7}
        mov     r2, r1

10
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}
        ldmia   r2!, {r4, r5, r6, r7}

        subs    r0, r0, #512
        cmp     r0, #512
        bge     %B10

        ldmfd   sp!, {r4, r5, r6, r7}
        mov     pc, lr

        END