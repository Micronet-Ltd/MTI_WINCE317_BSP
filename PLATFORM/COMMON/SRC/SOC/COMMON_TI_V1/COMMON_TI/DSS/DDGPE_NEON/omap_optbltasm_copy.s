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
;   File:  omap_optbltasm_copy.s
;

        AREA    omap_optbltasm,CODE,READONLY

        EXPORT  BlockCopy8
        EXPORT  BlockCopy16
        EXPORT  BlockCopy24
        EXPORT  BlockCopy32
        
        EXPORT  BlockCopyLUT8to16
        EXPORT  BlockCopyLUT8to24
        EXPORT  BlockCopyLUT8to32
        
        EXPORT  BlockCopyRGB16toBGR24
        EXPORT  BlockCopyRGB16toBGRx32

        EXPORT  BlockCopyBGR24toRGB16
        EXPORT  BlockCopyXYZ24toXYZx32
        
        EXPORT  BlockCopyBGRx32toRGB16
        EXPORT  BlockCopyXYZx32toXYZ24

        EXTERN  PreloadLine

;----------------------------------------------------------------------------
; ASSUMPTIONS
;----------------------------------------------------------------------------
; All destination memory has write combining enabled.  This mitigates the
;  need to align writes.
;
; Sources that are not the same format as the destination are cached.  This
;  mitigates the need to align reads when the source and destination don't
;  match.
;
; Sources that are the same format as the destination might not be cached.
;  The byte copy routine below aligns the source.
;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Predefined registers (for reference)
;       
; r13 a.k.a. sp
; r14 a.k.a. lr
; r15 a.k.a. pc

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Common symbols
;
; It is convenient to use meaningful names for registers in the code below.
;  But some assemblers don't like redefining the aliases within the same
;  source file.  So a compromise is to use a set of standard assignments for
;  the entire file.  These are common across the routines below.  Not all are
;  used, but this consistency simplifies things.
;
dstptr  rn      r4
dststep rn      r5
dststride       rn      dststep
srcptr  rn      r6
srcstep rn      r7
srcstride       rn      srcstep
dstwidth        rn      r10
dstheight       rn      r11
dstline rn      r8
srcline rn      r9
lut     rn      r12
pels    rn      lr

        INCLUDE omap_optbltasm.inc
        
;----------------------------------------------------------------------------
; BlockCopyBytes copies an 8-bit rectangle of bytes from one surface to
;  another.  It is not used directly from an external app, but is called by
;  the block copy routines below.
;
; On entry, this functiona assumes:
; r4 = dstptr
; r5 = dststep
; r6 = srcptr
; r7 = srcstep
; r10 = dstwidth in bytes
; r11 = dstheight in lines
; And that stmfd sp!, {r4-r12,lr} was issued on entry
;
; This routine is written to use the maximum transfer size allowed in case
; the source is not cached.  The destination is normally has write combining
; enabled, so performance does not rely on destination alignment.
;
; (Note entry point is in the middle of the function.)
;
BCB_pre
        rsb     r0, r0, #64             ; prebytes = bytes to get aligned
        cmp     dstwidth, r0            ; if(dstwidth in bytes < bytes to get aligned)
        movlt   r0, dstwidth            ;       prebytes = dstwidth in bytes

        rsb     r1, srcptr, #64         ; alignment flags
BCB_pre1
                                                                        ;   1   2   3   4   5   6   7   8   9  10  11
        tst     r1, #1                  ; see if lsb is set
        beq     BCB_pre2                ; if so...
        ldrb    r2, [srcptr], #1        ; ...copy one...
        strb    r2, [dstptr], #1        ; ...byte and...
        subs    r12, r12, #1            ; ...decrement byte counter     ;  (0) (1) (2) (3) (4) (5) (6) (7) (8) (9)(10)
        beq     BCB_linedone
        sub     r0, r0, #1                                              ;  -2  -1   0   1   2   3   4   5   6   7   8
BCB_pre2
        cmp     r0, #2
        blt     BCB_groups
        tst     r1, #2
        beq     BCB_pre4
        vld2.8  {d0[0],d1[0]}, [srcptr]!
        vst2.8  {d0[0],d1[0]}, [dstptr]!        ; Neon used in case dest isn't aligned
        subs    r12, r12, #2                                            ;   -   -  (0) (1) (2) (3) (4) (5) (6) (7) (8)
        beq     BCB_linedone
        subs    r0, r0, #2                                              ;   -   -  -4  -3  -2  -1   0   1   2   3   4
BCB_pre4
        cmp     r0, #4
        blt     BCB_groups
        tst     r1, #4
        beq     BCB_pre8
        vld4.8  {d0[0],d1[0],d2[0],d3[0]}, [srcptr]!
        vst4.8  {d0[0],d1[0],d2[0],d3[0]}, [dstptr]!            ; Neon used in case dest isn't aligned
        subs    r12, r12, #4
        beq     BCB_linedone
        subs    r0, r0, #4
BCB_pre8
        cmp     r0, #8
        blt     BCB_groups
        tst     r1, #8
        beq     BCB_pre16
        vld1.8  {d0}, [srcptr@64]!
        vst1.8  {d0}, [dstptr]!                 ; Neon used in case dest isn't aligned
        subs    r12, r12, #8
        beq     BCB_linedone
        subs    r0, r0, #8
BCB_pre16
        cmp     r0, #16
        blt     BCB_groups
        tst     r1, #16
        beq     BCB_pre32
        vld2.8  {d0,d1}, [srcptr@128]!
        vst2.8  {d0,d1}, [dstptr]!              ; Neon used in case dest isn't aligned
        subs    r12, r12, #16
        beq     BCB_linedone
        subs    r0, r0, #32
BCB_pre32
        cmp     r0, #32
        blt     BCB_groups
        tst     r1, #32
        beq     BCB_groups
        vld4.8  {d0,d1,d2,d3}, [srcptr@128]!
        vst4.8  {d0,d1,d2,d3}, [dstptr]!        ; Neon used in case dest isn't aligned
        subs    r12, r12, #32
        beq     BCB_linedone
        b       BCB_groups
BlockCopyBytes
BCB_lineloop
        mov     r0, srcptr
        mov     r1, dstwidth
        bl      PreloadLine
        
        mov     r12, dstwidth

        ands    r0, srcptr, #63
        bne     BCB_pre

BCB_groups
        movs    r1, r12, lsr #6
        beq     BCB_post

BCB_grouploop
        vld4.8  {d0,d1,d2,d3}, [srcptr@128]!    ; vld4.8 {q0,q1,q2,q3}, [src@256]!
        vld4.8  {d4,d5,d6,d7}, [srcptr@128]!
        vst4.8  {d0,d1,d2,d3}, [dstptr]!        ; vst4.8 {q0,q1,q2,q3}, [dst]!
        vst4.8  {d4,d5,d6,d7}, [dstptr]!

        subs    r1, r1, #1
        bne     BCB_grouploop

BCB_post
BCB_post32
        movs    r12, r12, lsl #(32-6)
        beq     BCB_linedone
        bpl     BCB_post16
        vld4.8  {d0,d1,d2,d3}, [srcptr@128]!
        vst4.8  {d0,d1,d2,d3}, [dstptr]!        ; 8-bit Neon used in case dest isn't aligned
BCB_post16
        movs    r12, r12, lsl #1
        beq     BCB_linedone
        bpl     BCB_post8
        vld2.8  {d0,d1}, [srcptr@128]!
        vst2.8  {d0,d1}, [dstptr]!              ; 8-bit Neon used in case dest isn't aligned
BCB_post8
        movs    r12, r12, lsl #1
        beq     BCB_linedone
        bpl     BCB_post4
        vld1.8  {d0}, [srcptr@64]!
        vst1.8  {d0}, [dstptr]!                 ; 8-bit Neon used in case dest isn't aligned
BCB_post4
        movs    r12, r12, lsl #1
        beq     BCB_linedone
        bpl     BCB_post2
        vld4.8  {d0[0],d1[0],d2[0],d3[0]}, [srcptr]!
        vst4.8  {d0[0],d1[0],d2[0],d3[0]}, [dstptr]!    ; 8-bit Neon used in case dest isn't aligned
BCB_post2
        movs    r12, r12, lsl #1
        beq     BCB_linedone
        bpl     BCB_post1
        vld2.8  {d0[0],d1[0]}, [srcptr]!
        vst2.8  {d0[0],d1[0]}, [dstptr]!        ; 8-bit Neon used in case dest isn't aligned
BCB_post1
        movs    r12, r12, lsl #1
        ldrmib  r1, [srcptr], #1
        strmib  r1, [dstptr], #1

BCB_linedone
        add     srcptr, srcptr, srcstep
        subs    dstheight, dstheight, #1
        add     dstptr, dstptr, dststep
        bne     BCB_lineloop

        ldmfd   sp!, {r4-r12,pc}

;----------------------------------------------------------------------------
; void BlockCopy8(void*       dstptr,
;                 int32       dststride,
;                 uint32      dstleft,
;                 uint32      dsttop,
;                 void const* srcptr,
;                 int32       srcstride,
;                 uint32      srcleft,
;                 uint32      srctop,
;                 uint32      dstwidth,
;                 uint32      dstheight)
;
; Performs a block copy of a rectangle of 8-bit pixels from one surface to
;  another.
;
BlockCopy8
        stmfd   sp!, {r4-r12,lr}
        TWOSURFINIT     8, 8, 10
        b       BlockCopyBytes

;----------------------------------------------------------------------------
; void BlockCopy16(void*       dstptr,
;                  int32       dststride,
;                  uint32      dstleft,
;                  uint32      dsttop,
;                  void const* srcptr,
;                  int32       srcstride,
;                  uint32      srcleft,
;                  uint32      srctop,
;                  uint32      dstwidth,
;                  uint32      dstheight)
;
; Performs a block copy of a rectangle of 16-bit pixels from one surface to
;  another.
;
BlockCopy16
        stmfd   sp!, {r4-r12,lr}
        TWOSURFINIT     16, 16, 10
        add     dstwidth, dstwidth, dstwidth
        b       BlockCopyBytes

;----------------------------------------------------------------------------
; void BlockCopy24(void*       dstptr,
;                  int32       dststride,
;                  uint32      dstleft,
;                  uint32      dsttop,
;                  void const* srcptr,
;                  int32       srcstride,
;                  uint32      srcleft,
;                  uint32      srctop,
;                  uint32      dstwidth,
;                  uint32      dstheight)
;
; Performs a block copy of a rectangle of 24-bit pixels from one surface to
;  another.
;
BlockCopy24
        stmfd   sp!, {r4-r12,lr}
        TWOSURFINIT     24,24, 10
        add     dstwidth, dstwidth, dstwidth, lsl #1
        b       BlockCopyBytes

;----------------------------------------------------------------------------
; void BlockCopy32(void*       dstptr,
;                  int32       dststride,
;                  uint32      dstleft,
;                  uint32      dsttop,
;                  void const* srcptr,
;                  int32       srcstride,
;                  uint32      srcleft,
;                  uint32      srctop,
;                  uint32      dstwidth,
;                  uint32      dstheight)
;
; Performs a block copy of a rectangle of 32-bit pixels from one surface to
;  another.
;
BlockCopy32
        stmfd   sp!, {r4-r12,lr}
        TWOSURFINIT     32,32, 10
        mov     dstwidth, dstwidth, lsl #2
        b       BlockCopyBytes

;----------------------------------------------------------------------------
; void BlockCopyLUT8to16(void*         dstptr,
;                        int32         dststride,
;                        uint32        dstleft,
;                        uint32        dsttop,
;                        uint32        dstwidth,
;                        uint32        dstheight,
;                        void const*   srcptr,
;                        int32         srcstride,
;                        uint32        srcleft,
;                        uint32        srctop,
;                        uint32 const* lut)
;
; Utilizes a lookup table to convert an 8-bit rectangle from one surface to a
;  16-bit destination surface.
;
; Note that the table is assumed to contain the 16 bit values, but on 32 bit
;  boundary.  This saves one cycle for each of the reads into the cycle, due
;  to an optimization of the ARM for "lsl #2" modifications that doesn't
;  exist for any other shift.
;
BlockCopyLUT8to16
        stmfd   sp!, {r4-r12,lr}
        TWOSURFINIT     16, 8, 10, nostep
        ldr     lut, [sp, #((10 + 6) * 4)]

        mov     srcline, srcptr
        mov     dstline, dstptr

BC816_lineloop
        PRELOADLINE8bpp srcline, dstwidth

        mov     pels, dstwidth
        mov     srcptr, srcline
        mov     dstptr, dstline

BC816_pelloop
; r0 = idx0, val0
; r1 = idx1, val1
; r2 = idx2, val2
; interleaved three pixels, twice unrolled loop
                                        ; 1  2  3  4  5  6  7
        subs    pels, pels, #2          ;-1  0  1  2  3  4  5
        ldrgeb  r1, [srcptr, #1]        ; n  y  y  y  y  y  y
        ldrgtb  r2, [srcptr, #2]        ; n  n  y  y  y  y  y
        ldrb    r0, [srcptr], #3        ; y  y  y  y  y  y  y
        ldrge   r1, [lut, r1, lsl #2]   ; n  y  y  y  y  y  y
        ldrgt   r2, [lut, r2, lsl #2]   ; n  n  y  y  y  y  y
        ldr     r0, [lut, r0, lsl #2]   ; y  y  y  y  y  y  y
        strgeh  r1, [dstptr, #2]        ; n  y  y  y  y  y  y
        strgth  r2, [dstptr, #4]        ; n  n  y  y  y  y  y
        strh    r0, [dstptr], #6        ; y  y  y  y  y  y  y
        subs    pels, pels, #1          ;-2 -1  0  1  2  3  4

        ble     BC816_linedone

        subs    pels, pels, #2          ; x  x  x -1  0  1  2
        ldrgeb  r1, [srcptr, #1]        ; n  y  y  y  y  y  y
        ldrgtb  r2, [srcptr, #2]        ; n  n  y  y  y  y  y
        ldrb    r0, [srcptr], #3        ; y  y  y  y  y  y  y
        ldrge   r1, [lut, r1, lsl #2]   ; n  y  y  y  y  y  y
        ldrgt   r2, [lut, r2, lsl #2]   ; n  n  y  y  y  y  y
        ldr     r0, [lut, r0, lsl #2]   ; y  y  y  y  y  y  y
        strgeh  r1, [dstptr, #2]        ; n  y  y  y  y  y  y
        strgth  r2, [dstptr, #4]        ; n  n  y  y  y  y  y
        strh    r0, [dstptr], #6        ; y  y  y  y  y  y  y
        subs    pels, pels, #1          ;-3 -2 -1 -2 -1  0  1

        bgt     BC816_pelloop

BC816_linedone
        add     srcline, srcline, srcstride
        add     dstline, dstline, dststride
        subs    dstheight, dstheight, #1
        bne     BC816_lineloop

        ldmfd   sp!, {r4-r12,pc}

;----------------------------------------------------------------------------
; void BlockCopyLUT8to24(void*         dstptr,
;                        int32         dststride,
;                        uint32        dstleft,
;                        uint32        dsttop,
;                        uint32        dstwidth,
;                        uint32        dstheight,
;                        void const*   srcptr,
;                        int32         srcstride,
;                        uint32        srcleft,
;                        uint32        srctop,
;                        uint32 const* lut)
;
; Utilizes a lookup table to convert an 8-bit rectangle from one surface to a
;  24-bit destination surface.
;
; Note that the table is assumed to contain the 24 bit values, but on 32 bit
;  boundary.
;
BlockCopyLUT8to24
        stmfd   sp!, {r4-r12,lr}
        TWOSURFINIT     24, 8, 10
        ldr     lut, [sp, #((10 + 6) * 4)]

BC824_lineloop
        PRELOADLINE8bpp srcline, dstwidth
        subs    pels, dstwidth, #8
        blt     BC824_singles

BC824_grouploop
        ldrb    r0, [srcptr, #7]
        ldrb    r1, [srcptr, #6]
        add     r0, lut, r0, lsl #2
        add     r1, lut, r1, lsl #2
        vld3.8  {d0[7],d1[7],d2[7]}, [r0]
        vld3.8  {d0[6],d1[6],d2[6]}, [r1]
        ldrb    r0, [srcptr, #5]
        ldrb    r1, [srcptr, #4]
        add     r0, lut, r0, lsl #2
        add     r1, lut, r1, lsl #2
        vld3.8  {d0[5],d1[5],d2[5]}, [r0]
        vld3.8  {d0[4],d1[4],d2[4]}, [r1]
        ldrb    r0, [srcptr, #3]
        ldrb    r1, [srcptr, #2]
        add     r0, lut, r0, lsl #2
        add     r1, lut, r1, lsl #2
        vld3.8  {d0[3],d1[3],d2[3]}, [r0]
        vld3.8  {d0[2],d1[2],d2[2]}, [r1]
        ldrb    r0, [srcptr, #1]
        ldrb    r1, [srcptr], #8
        add     r0, lut, r0, lsl #2
        add     r1, lut, r1, lsl #2
        vld3.8  {d0[1],d1[1],d2[1]}, [r0]
        vld3.8  {d0[0],d1[0],d2[0]}, [r1]
        subs    pels, pels, #8
        vst3.8  {d0,d1,d2}, [dstptr]!
        bge     BC824_grouploop
        
BC824_singles
        adds    pels, pels, #8
        ble     BC824_linedone

BC824_singleloop
        ldrb    r0, [srcptr], #1
        add     r1, lut, r0, lsl #2
        vld3.8  {d0[0],d1[0],d2[0]}, [r1]
        vst3.8  {d0[0],d1[0],d2[0]}, [dstptr]!
        subs    pels, pels, #1
        bgt     BC824_singleloop
        
BC824_linedone
        add     srcptr, srcptr, srcstep
        add     dstptr, dstptr, dststep
        subs    dstheight, dstheight, #1
        bne     BC824_lineloop

        ldmfd   sp!, {r4-r12,pc}

;----------------------------------------------------------------------------
; void BlockCopyLUT8to32(void*         dstptr,
;                        int32         dststride,
;                        uint32        dstleft,
;                        uint32        dsttop,
;                        uint32        dstwidth,
;                        uint32        dstheight,
;                        void const*   srcptr,
;                        int32         srcstride,
;                        uint32        srcleft,
;                        uint32        srctop,
;                        uint32 const* lut)
;
; Utilizes a lookup table to convert an 8-bit rectangle from one surface to a
;  32-bit destination surface.
;
BlockCopyLUT8to32
        stmfd   sp!, {r4-r12,lr}
        TWOSURFINIT 32, 8, 10, nostep
        ldr     lut, [sp, #((10 + 6) * 4)]

        mov     srcline, srcptr
        mov     dstline, dstptr

BC832_lineloop
        PRELOADLINE8bpp srcline, dstwidth

        mov     pels, dstwidth
        mov     srcptr, srcline
        mov     dstptr, dstline

BC832_pelloop
; r0 = idx0, val0
; r1 = idx1, val1
; r2 = idx2, val2
; interleaved three pixels, twice unrolled loop
                                        ; 1  2  3  4  5  6  7
        subs    pels, pels, #2          ;-1  0  1  2  3  4  5
        ldrgeb  r1, [srcptr, #1]        ; n  y  y  y  y  y  y
        ldrgtb  r2, [srcptr, #2]        ; n  n  y  y  y  y  y
        ldrb    r0, [srcptr], #3        ; y  y  y  y  y  y  y
        ldrge   r1, [lut, r1, lsl #2]   ; n  y  y  y  y  y  y
        ldrgt   r2, [lut, r2, lsl #2]   ; n  n  y  y  y  y  y
        ldr     r0, [lut, r0, lsl #2]   ; y  y  y  y  y  y  y
        strge   r1, [dstptr, #4]        ; n  y  y  y  y  y  y
        strgt   r2, [dstptr, #8]        ; n  n  y  y  y  y  y
        str     r0, [dstptr], #12       ; y  y  y  y  y  y  y
        subs    pels, pels, #1          ;-2 -1  0  1  2  3  4

        ble     BC832_linedone

        subs    pels, pels, #2          ; x  x  x -1  0  1  2
        ldrgeb  r1, [srcptr, #1]        ; n  y  y  y  y  y  y
        ldrgtb  r2, [srcptr, #2]        ; n  n  y  y  y  y  y
        ldrb    r0, [srcptr], #3        ; y  y  y  y  y  y  y
        ldrge   r1, [lut, r1, lsl #2]   ; n  y  y  y  y  y  y
        ldrgt   r2, [lut, r2, lsl #2]   ; n  n  y  y  y  y  y
        ldr     r0, [lut, r0, lsl #2]   ; y  y  y  y  y  y  y
        strge   r1, [dstptr, #4]        ; n  y  y  y  y  y  y
        strgt   r2, [dstptr, #8]        ; n  n  y  y  y  y  y
        str     r0, [dstptr], #12       ; y  y  y  y  y  y  y
        subs    pels, pels, #1          ;-3 -2 -1 -2 -1  0  1

        bgt     BC832_pelloop

BC832_linedone
        add     srcline, srcline, srcstride
        add     dstline, dstline, dststride
        subs    dstheight, dstheight, #1
        bne     BC832_lineloop

        ldmfd   sp!, {r4-r12,pc}

;----------------------------------------------------------------------------
; void BlockCopyRGB16toBGR24(void*       dstptr,
;                            int32       dststride,
;                            uint32      dstleft,
;                            uint32      dsttop,
;                            uint32      dstwidth,
;                            uint32      dstheight,
;                            void const* srcptr,
;                            int32       srcstride,
;                            uint32      srcleft,
;                            uint32      srctop)
;
; Converts a 16-bit (RGB 5:6:5) rectangle from one surface to a reverse order
;  (BGR byte order) 24 bit packed destination surface.
;
BlockCopyRGB16toBGR24
        stmfd   sp!, {r4-r12,lr}
        TWOSURFINIT     24, 16, 10

BC16r24_lineloop
        PRELOADLINE16bpp        srcptr, dstwidth
        
        subs    pels, dstwidth, #8
        blt     BC16r24_singles

BC16r24_octetloop
        vld1.16 {d0,d1}, [srcptr]!              ; vld1.16 {q0}, [srcptr]!
        RGB565toRGB888  d4, d3, d2, q0
        vst3.8  {d2,d3,d4}, [dstptr]!
        subs    pels, pels, #8
        bge     BC16r24_octetloop

BC16r24_singles
        adds    pels, pels, #8
        beq     BC16r24_linedone

        LOADSTRAYS_16   pels, srcptr, d0, d1
        RGB565toRGB888  d4, d3, d2, q0
        STORESTRAYS_3x8 pels, dstptr, d2, d3, d4

BC16r24_linedone
        add     dstptr, dstptr, dststep
        add     srcptr, srcptr, srcstep
        subs    dstheight, dstheight, #1
        bgt     BC16r24_lineloop

        ldmfd   sp!, {r4-r12,pc}

;----------------------------------------------------------------------------
; void BlockCopyRGB16toBGRx32(void*       dstptr,
;                             int32       dststride,
;                             uint32      dstleft,
;                             uint32      dsttop,
;                             uint32      dstwidth,
;                             uint32      dstheight,
;                             void const* srcptr,
;                             int32       srcstride,
;                             uint32      srcleft,
;                             uint32      srctop)
;
; Converts a 16-bit (RGB 5:6:5) rectangle from one surface to a reverse order
;  (BGRx byte order) 24 bit unpacked (32 bits including padding byte)
;  destination surface.  The extra byte is filled in with the alpha specified
;  in the VMOV_U8_ALPHA macro above.
;
BlockCopyRGB16toBGRx32
        stmfd   sp!, {r4-r12,lr}
        TWOSURFINIT     32, 16, 10

        VMOV_U8_ALPHA   d5

BC16r32_lineloop
        PRELOADLINE16bpp        srcptr, dstwidth
        
        subs    pels, dstwidth, #8
        blt     BC16r32_singles

BC16r32_octetloop
        vld1.16 {d0,d1}, [srcptr]!              ; vld1.16 {q0}, [srcptr]!
        RGB565toRGB888  d4, d3, d2, q0
        vst4.8  {d2,d3,d4,d5}, [dstptr]!
        subs    pels, pels, #8
        bge     BC16r32_octetloop

BC16r32_singles
        adds    pels, pels, #8
        beq     BC16r32_linedone

        LOADSTRAYS_16   pels, srcptr, d0, d1
        RGB565toRGB888  d4, d3, d2, q0
        STORESTRAYS_4x8 pels, dstptr, d2, d3, d4, d5

BC16r32_linedone
        add     dstptr, dstptr, dststep
        add     srcptr, srcptr, srcstep
        subs    dstheight, dstheight, #1
        bgt     BC16r32_lineloop

        ldmfd   sp!, {r4-r12,pc}
 
;----------------------------------------------------------------------------
; void BlockCopyBGR24toRGB16(void*       dstptr,
;                            int32       dststride,
;                            uint32      dstleft,
;                            uint32      dsttop,
;                            uint32      dstwidth,
;                            uint32      dstheight,
;                            void const* srcptr,
;                            int32       srcstride,
;                            uint32      srcleft,
;                            uint32      srctop)
;
; Converts a reverse order (BGR byte order) 24 bit packed rectangle from one
;  surface to an RGB 5:6:5 destination surface.
;
BlockCopyBGR24toRGB16
        stmfd   sp!, {r4-r12,lr}
        TWOSURFINIT     16, 24, 10

BCr2416_lineloop
        PRELOADLINE24bpp        srcptr, dstwidth
        
        subs    pels, dstwidth, #8
        blt     BCr2416_singles

BCr2416_octetloop
        vld3.8  {d0,d1,d2}, [srcptr]!
        RGB888toRGB565  q2, d2, d1, d0, q3, q4
        vst1.16 {d4,d5}, [dstptr]!              ; vst1.16 {q2}, [dstptr]!
        subs    pels, pels, #8
        bge     BCr2416_octetloop

BCr2416_singles
        adds    pels, pels, #8
        beq     BCr2416_linedone

        LOADSTRAYS_3x8  pels, srcptr, d0, d1, d2
        RGB888toRGB565  q2, d2, d1, d0, q3, q4
        STORESTRAYS_16  pels, dstptr, d4, d5

BCr2416_linedone
        add     dstptr, dstptr, dststep
        add     srcptr, srcptr, srcstep
        subs    dstheight, dstheight, #1
        bgt     BCr2416_lineloop

        ldmfd   sp!, {r4-r12,pc}
 
;----------------------------------------------------------------------------
; void BlockCopyXYZ24toXYZx32(void*       dstptr,
;                             int32       dststride,
;                             uint32      dstleft,
;                             uint32      dsttop,
;                             uint32      dstwidth,
;                             uint32      dstheight,
;                             void const* srcptr,
;                             int32       srcstride,
;                             uint32      srcleft,
;                             uint32      srctop)
;
; Converts a 24-bit packed RGB or BGR rectangle from one surface to a RGBx or
;  BGRx (respectively) in a destination surface.
;
BlockCopyXYZ24toXYZx32
        stmfd   sp!, {r4-r12,lr}
        TWOSURFINIT     32, 24, 10

        VMOV_U8_ALPHA   d3

BC2432_lineloop
        PRELOADLINE24bpp        srcptr, dstwidth
        
        subs    pels, dstwidth, #8
        blt     BC2432_singles

BC2432_octetloop
        vld3.8  {d0,d1,d2}, [srcptr]!
        vst4.8  {d0,d1,d2,d3}, [dstptr]!
        subs    pels, pels, #8
        bge     BC2432_octetloop

BC2432_singles
        adds    pels, pels, #8
        beq     BC2432_linedone

        LOADSTRAYS_3x8  pels, srcptr, d0, d1, d2
        STORESTRAYS_4x8 pels, dstptr, d0, d1, d2, d3

BC2432_linedone
        add     dstptr, dstptr, dststep
        add     srcptr, srcptr, srcstep
        subs    dstheight, dstheight, #1
        bgt     BC2432_lineloop

        ldmfd   sp!, {r4-r12,pc}
 
;----------------------------------------------------------------------------
; void BlockCopyBGRx32toRGB16(void*       dstptr,
;                             int32       dststride,
;                             uint32      dstleft,
;                             uint32      dsttop,
;                             uint32      dstwidth,
;                             uint32      dstheight,
;                             void const* srcptr,
;                             int32       srcstride,
;                             uint32      srcleft,
;                             uint32      srctop)
;
; Converts a reverse order (BGRx byte order) 24 bit unpacked (32 bits
;  including padding byte) rectangle from one surface to an RGB 5:6:5
;  destination surface.
;
BlockCopyBGRx32toRGB16
        stmfd   sp!, {r4-r12,lr}
        TWOSURFINIT     16, 32, 10

BCr3216_lineloop
        PRELOADLINE32bpp        srcptr, dstwidth
        
        subs    pels, dstwidth, #8
        blt     BCr3216_singles

BCr3216_octetloop
        vld4.8  {d0,d1,d2,d3}, [srcptr]!
        RGB888toRGB565  q2, d2, d1, d0, q3, q4
        vst1.16 {d4,d5}, [dstptr]!              ; vst1.16 {q2}, [dstptr]!
        subs    pels, pels, #8
        bge     BCr3216_octetloop

BCr3216_singles
        adds    pels, pels, #8
        beq     BCr3216_linedone

        LOADSTRAYS_4x8  pels, srcptr, d0, d1, d2, d3
        RGB888toRGB565  q2, d2, d1, d0, q3, q4
        STORESTRAYS_16  pels, dstptr, d4, d5

BCr3216_linedone
        add     dstptr, dstptr, dststep
        add     srcptr, srcptr, srcstep
        subs    dstheight, dstheight, #1
        bgt     BCr3216_lineloop

        ldmfd   sp!, {r4-r12,pc}
 
;----------------------------------------------------------------------------
; void BlockCopyXYZx32toXYZ24(void*       dstptr,
;                             int32       dststride,
;                             uint32      dstleft,
;                             uint32      dsttop,
;                             uint32      dstwidth,
;                             uint32      dstheight,
;                             void const* srcptr,
;                             int32       srcstride,
;                             uint32      srcleft,
;                             uint32      srctop)
;
; Converts a 32-bit (RGBX or BGRx) rectangle from one surface to a packed
;  24-bit surface (RGB or BGR respectively).
;
BlockCopyXYZx32toXYZ24
        stmfd   sp!, {r4-r12,lr}
        TWOSURFINIT     24, 32, 10

BC3224_lineloop
        PRELOADLINE32bpp        srcptr, dstwidth
        
        subs    pels, dstwidth, #8
        blt     BC3224_singles

BC3224_octetloop
        vld4.8  {d0,d1,d2,d3}, [srcptr]!
        vst3.8  {d0,d1,d2}, [dstptr]!
        subs    pels, pels, #8
        bge     BC3224_octetloop

BC3224_singles
        adds    pels, pels, #8
        beq     BC3224_linedone

        LOADSTRAYS_4x8  pels, srcptr, d0, d1, d2, d3
        STORESTRAYS_3x8 pels, dstptr, d0, d1, d2

BC3224_linedone
        add     dstptr, dstptr, dststep
        add     srcptr, srcptr, srcstep
        subs    dstheight, dstheight, #1
        bgt     BC3224_lineloop

        ldmfd   sp!, {r4-r12,pc}
        
        END

