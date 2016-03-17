;
;================================================================================
;             Texas Instruments OMAP(TM) Platform Software
; (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
;
; Use of this software is controlled by the terms and conditions found
; in the license agreement under which this software has been supplied.
;
;================================================================================
;
;-------------------------------------------------------------------------------
;
;  File:  dcache.s
;
;
        INCLUDE kxarm.h
        INCLUDE armmacros.s
        INCLUDE oal_cache.inc

        IMPORT g_oalCacheInfo

        TEXTAREA


;-------------------------------------------------------------------------------
;
;  Function:  OALCleanDCache
;
;  Entire cache operatations must be performed using set/way method operation at
;  each level of the cache.
;
        LEAF_ENTRY OALCleanDCache

        stmfd   sp!, {r4-r7, r9-r11, lr}        ; store off registers to stack

        mrc     p15, 1, r0, c0, c0, 1           ; read clidr
        ands    r3, r0, #0x7000000  
        mov     r3, r3, lsr #23                 ; cache level value
        beq     donea               

        mov     r10, #0                         ; start clean at cache level 0
loop1a  add     r2, r10, r10, lsr #1            ; work out 3x current cache level
        mov     r1, r0, lsr r2                  ; extract cache type bits from clidr
        and     r1, r1, #7                      ; mask of the bits for current cache only
        cmp     r1, #2                          ; see what cache we have at this level
        blt     skipa                           ; skip if no cache, or just i-cache

        mcr     p15, 2, r10, c0, c0, 0          ; select current cache level in cssr
        mov     r1, #0
        mcr     p15, 0, r1, c7, c5, 4           ; prefetch flush to sync the change to the cachesize id reg
        mrc     p15, 1, r1, c0, c0, 0           ; read the new csidr
        and     r2, r1, #7                      ; extract the length of the cache lines
        add     r2, r2, #4                      ; add 4 (line length offset)
        ldr     r4, =0x3ff
        ands    r4, r4, r1, lsr #3              ; r4 is maximum number on the way size
        clz     r5, r4                          ; r5 find bit position of way size increment
        ldr     r7, =0x7fff
        ands    r7, r7, r1, lsr #13             ; r7 extract max number of the index size

loop2a  mov     r9, r4                          ; r9 is working copy of max way size
loop3a  orr     r11, r10, r9, lsl r5            ; factor way and cache number into r11
        orr     r11, r11, r7, lsl r2            ; factor index number into r11

        mcr     p15, 0, r11, c7, c10, 2         ; clean by set/way

        subs    r9, r9, #1                      ; decrement the way
        bge     loop3a

        subs    r7, r7, #1                      ; decrement the index
        bge     loop2a

skipa   add     r10, r10, #2                    ; increment cache number
        cmp     r3, r10
        bgt     loop1a

donea   mov     r10, #0                         ; swith back to cache level 0
        mcr     p15, 2, r10, c0, c0, 0          ; select current cache level in cssr

        mov     r2, #0
        mcr     p15, 0, r2, c7, c10, 4          ; data sync barrier operation

        ldmfd   sp!, {r4-r7, r9-r11, lr}        ; restore registers

        RETURN




;-------------------------------------------------------------------------------
;
;  Function:  OALCleanDCacheLines
;
        LEAF_ENTRY OALCleanDCacheLines

        ldr     r2, =g_oalCacheInfo
        ldr     r3, [r2, #L1DLineSize]

10      mcr     p15, 0, r0, c7, c10, 1          ; clean entry
        add     r0, r0, r3                      ; move to next entry
        subs    r1, r1, r3
        bgt     %b10                            ; loop while > 0 bytes left

        mov     r2, #0
        mcr     p15, 0, r2, c7, c10, 4          ; data sync barrier operation

        RETURN


;-------------------------------------------------------------------------------
;
;  Function:  OALFlushDCache
;
;  Entire cache operatations must be performed using set/way method operation at
;  each level of the cache. 
;
        LEAF_ENTRY OALFlushDCache

        stmfd   sp!, {r4-r7, r9-r11, lr}        ; store off registers to stack

        mrc     p15, 1, r0, c0, c0, 1           ; read clidr
        ands    r3, r0, #0x7000000  
        mov     r3, r3, lsr #23                 ; cache level value
        beq     doneb               

        mov     r10, #0                         ; start clean at cache level 0
loop1b  add     r2, r10, r10, lsr #1            ; work out 3x current cache level
        mov     r1, r0, lsr r2                  ; extract cache type bits from clidr
        and     r1, r1, #7                      ; mask of the bits for current cache only
        cmp     r1, #2                          ; see what cache we have at this level
        blt     skipb                           ; skip if no cache, or just i-cache

        mcr     p15, 2, r10, c0, c0, 0          ; select current cache level in cssr
        mov     r1, #0
        mcr     p15, 0, r1, c7, c5, 4           ; prefetch flush to sync the change to the cachesize id reg
        mrc     p15, 1, r1, c0, c0, 0           ; read the new csidr
        and     r2, r1, #7                      ; extract the length of the cache lines
        add     r2, r2, #4                      ; add 4 (line length offset)
        ldr     r4, =0x3ff
        ands    r4, r4, r1, lsr #3              ; r4 is maximum number on the way size
        clz     r5, r4                          ; r5 find bit position of way size increment
        ldr     r7, =0x7fff
        ands    r7, r7, r1, lsr #13             ; r7 extract max number of the index size

loop2b  mov     r9, r4                          ; r9 is working copy of max way size
loop3b  orr     r11, r10, r9, lsl r5            ; factor way and cache number into r11
        orr     r11, r11, r7, lsl r2            ; factor index number into r11

        mcr     p15, 0, r11, c7, c14, 2         ; clean and invalidate by set/way

        subs    r9, r9, #1                      ; decrement the way
        bge     loop3b

        subs    r7, r7, #1                      ; decrement the index
        bge     loop2b

skipb   add     r10, r10, #2                    ; increment cache number
        cmp     r3, r10
        bgt     loop1b

doneb   mov     r10, #0                         ; swith back to cache level 0
        mcr     p15, 2, r10, c0, c0, 0          ; select current cache level in cssr

        mov     r2, #0
        mcr     p15, 0, r2, c7, c10, 4          ; data sync barrier operation


        ldmfd   sp!, {r4-r7, r9-r11, lr}        ; restore registers

        RETURN


;-------------------------------------------------------------------------------
;
;  Function:  OALFlushDCacheLines
;
        LEAF_ENTRY OALFlushDCacheLines

        ldr     r2, =g_oalCacheInfo
        ldr     r3, [r2, #L1DLineSize]

10      mcr     p15, 0, r0, c7, c14, 1          ; clean and invalidate entry
        add     r0, r0, r3                      ; move to next
        subs    r1, r1, r3
        bgt     %b10                            ; loop while > 0 bytes left

        mov     r2, #0
        mcr     p15, 0, r2, c7, c10, 4          ; data sync barrier operation

        RETURN


;-------------------------------------------------------------------------------
;  Function:  OALInvalidateDCacheLines
;
        LEAF_ENTRY OALInvalidateDCacheLines

        ldr     r2, =g_oalCacheInfo
        ldr     r3, [r2, #L1DLineSize]

10      mcr     p15, 0, r0, c7, c6, 1           ; invalidate entry
        add     r0, r0, r3                      ; move to next
        subs    r1, r1, r3
        bgt     %b10                            ; loop while > 0 bytes left

        RETURN


        END
