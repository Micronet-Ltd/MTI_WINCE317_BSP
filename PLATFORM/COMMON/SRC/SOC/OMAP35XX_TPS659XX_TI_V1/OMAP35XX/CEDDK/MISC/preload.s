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
;   File:  preload.s
;

    AREA        preload,CODE,READONLY

    EXPORT      PreloadCacheLine
        
ARCH    EQU     {ARCHITECTURE}
ARCH_7  EQU     ARCH = "7"
ARCH_6T EQU     ARCH = "6t" :LOR: ARCH_7
ARCH_6  EQU     ARCH = "6"  :LOR: ARCH_6T
ARCH_5T EQU     ARCH = "5t" :LOR: ARCH_6
ARCH_5  EQU     ARCH = "5"  :LOR: ARCH_5T
ARCH_4T EQU     ARCH = "4t" :LOR: ARCH_5
ARCH_4  EQU     ARCH = "4"  :LOR: ARCH_4T

; r13 a.k.a. sp
; r14 a.k.a. lr
; r15 a.k.a. pc

CACHELINEWIDTH  equ     64

;-------------------------------------------------------------------------------
;
; void PreloadCacheLine(void* src, unsigned long bytes)
;                  
PreloadCacheLine

src         rn  r0
bytes   rn      r1

        pld     [src]

jump    rn      r2

        and         jump, src, #CACHELINEWIDTH-1
        rsb         jump, jump, #64
        
        subs    bytes, bytes, jump
        add     src, src, jump
        blt     PL_done

PL_loop
        pld         [src]
        subs    bytes, bytes, #CACHELINEWIDTH
        add     src, src, #CACHELINEWIDTH
        bge     PL_loop

PL_done
        mov     pc, lr

    END