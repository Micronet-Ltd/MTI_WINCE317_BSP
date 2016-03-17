; All rights reserved ADENEO EMBEDDED 2010
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
;   File:  omap_optbltasm.inc
;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; GLOBALs
;

;
; Set the following for the specific environment.  These values are used
;  below to handle specific differences between the environments.
;
        GBLL    SYMBIAN
SYMBIAN SETL    {FALSE}
        GBLL    WINCE
WINCE   SETL    {TRUE}

;
; When converting from 5:6:5 to 8:8:8, the best compromise between speed
;  (simplicity) and accuracy is to replicate the msbs into the lsbs (e.g.
;  r4-r3-r2-r1-r0 -> r4-r3-r2-r1-r0-r4-r3-r2).  But some environments
;  prefer to have the lsbs filled with zeroes.  This boolean constant is
;  used to choose which by some routines below.
;
        GBLL    CLRLSBS565TO888
 IF WINCE
CLRLSBS565TO888 SETL    {FALSE} ; Set lsbs of 565 converted to 888 to 0
 ELSE
CLRLSBS565TO888 SETL    {FALSE} ; Copy msbs to lsbs of 565 converted to 888
 ENDIF

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; MACROs
;

;
; VMOV_U8_ALPHA is used to load the specified register with the appropriate
;  alpha value for a destination format where the alpha is ignored.  In some
;  environments, the alpha value isn't actually ignored, and is either
;  expected or required to be a certain value, usually 1.0 (255) or 0.0 (0).
;  This macro allows the specification above to dictate the value.  It would
;  have been preferable to use just a defined constant, but not all assemblers
;  tested allowed a variable to be used as part of the constant value (i.e.
;  they wouldn't accept something like "vmov.u8 reg, $ALPHACONST", no matter
;  what type was used for ALPHACONST.
;
        MACRO
        VMOV_U8_ALPHA   $reg
 IF WINCE
        vmov.u8 $reg, #0x00     ; To match Windows CE/Mobile GDI (is this necessary to pass LTK?)
 ELSE
        vmov.u8 $reg, #0xFF     ; To match Windows CE/Mobile GDI (is this necessary to pass LTK?)
 ENDIF
        MEND

;
; SURFINIT is used to set up the registers for a single surface of the type
;  specified.  It calculates the offset from the left and top provided, and
;  calculates the step needed to move from the end of one span to the start
;  of the next.
;
; $ptrout and $step are outputs
; $ptrin, $stride, $left, $top, $bpp are inputs
; All are ARM registers (Rn) except $bpp, which is a constant
;
        MACRO
        SURFINIT        $ptrout, $stepstrideout, $ptrin, $stridein, $left, $top, $width, $bpp, $nostep
; ptrout = ptrin + ((left * bpp) / 8) + (top * stride)
        mla     $ptrout, $top, $stridein, $ptrin        ; ptrout = ptrin + (top * stride)
        mov     $ptrin, #$bpp                           ; bpp (using ptrin as temporary register)
        mul     $top, $left, $ptrin                     ; left *= bpp (using $top as temporary register)
        add     $ptrout, $ptrout, $top, lsr #3          ; ptrout += ((left * bpp) / 8)
 IF "$nostep" != ""
; keep stride
        mov     $stepstrideout, $stridein
 ELSE
; step = stride - ((dstwidth * $dstbpp) / 8)
        mul     $top, $ptrin, dstwidth                  ; dstwidth * $dstbpp (using $top as a temporary register)
        sub     $stepstrideout, $stridein, $top, lsr #3 ; step = stride - ((dstwidth * bpp) / 8)
  IF $bpp < 8
; sub-byte pixel sizes present a special issue for calculating the step
        and     $top, $left, #((8 / $bpp) - 1)          ; get lsbs of left (using $top as temporary register)
        and     $left, $width, #((8 / $bpp) - 1)        ; get lsbs of dstwidth (using $left as a temporary register)
        add     $top, $top, $left                       ; add lsbs of left to lsbs of dstwidth
        cmp     $top, #(8 / $bpp)                       ; if the result isn't more than bits that fit into a byte...
        sublt   $stepstrideout, $stepstrideout, #1      ; ...there's one less byte in the dstwidth than we thought
  ENDIF
 ENDIF
        MEND

;----------------------------------------------------------------------------
; ONESURFINIT handles entry for a single surface operation.  It pulls the
;  associated parameters off of the stack and initializes the following
;  common registers:
;   dstptr
;   dststep
;   dstwidth
;   dstheight
;
; $dstbpp provides the bits per pixel of the destination surface
; $regspushed provides the number of registers pushed on the stack since
;  the parameters were pushed on the stack, so that on entry, this macro
;  assumes:
;   r0 = dstptr
;   r1 = dststride in bytes
;   r2 = dstleft in pixels
;   r3 = dsttop in lines
;   sp + (($regspushed + 0) * 4) = dstwidth in pixels
;   sp + (($regspushed + 1) * 4) = dstheight in lines
;
        MACRO
        ONESURFINIT     $dstbpp, $regspushed, $nostep
        ldr     dstwidth, [sp, #(($regspushed + 0) * 4)]        ; get dstwidth
        ldr     dstheight, [sp, #(($regspushed + 1) * 4)]       ; get dstheight
        SURFINIT        dstptr, dststep, r0, r1, r2, r3, dstwidth, $dstbpp, $nostep
        MEND

;----------------------------------------------------------------------------
; TWOSURFINIT handles entry for a dual surface operation.  It pulls the
;  associated parameters off of the stack and initializes the following
;  common registers:
;   dstptr
;   dststep
;   srcptr
;   srcstep
;   dstwidth
;   dstheight
;
; $dstbpp provides the bits per pixel of the destination surface
; $srcbpp provides the bits per pixel of the source surface
; $regspushed provides the number of registers pushed on the stack since
;  the parameters were pushed on the stack, so that on entry, this macro
;  assumes:
;   r0 = dstptr
;   r1 = dststride in bytes
;   r2 = dstleft in pixels
;   r3 = dsttop in lines
;   sp + (($regspushed + 0) * 4) = dstwidth in pixels
;   sp + (($regspushed + 1) * 4) = dstheight in lines
;   sp + (($regspushed + 2) * 4) = srcptr
;   sp + (($regspushed + 3) * 4) = srcstride in bytes
;   sp + (($regspushed + 4) * 4) = srcleft in pixels
;   sp + (($regspushed + 5) * 4) = srctop in lines
;
        MACRO
        TWOSURFINIT     $dstbpp, $srcbpp, $regspushed, $nostep
        ldr     dstwidth, [sp, #(($regspushed + 0) * 4)]        ; get dstwidth
        ldr     dstheight, [sp, #(($regspushed + 1) * 4)]       ; get dstheight
        SURFINIT        dstptr, dststep, r0, r1, r2, r3, dstwidth, $dstbpp, $nostep
        ldr     r0, [sp, #(($regspushed + 2) * 4)]      ; srcptr
        ldr     r1, [sp, #(($regspushed + 3) * 4)]      ; srcstride
        ldr     r2, [sp, #(($regspushed + 4) * 4)]      ; srcleft
        ldr     r3, [sp, #(($regspushed + 5) * 4)]      ; srctop
        SURFINIT        srcptr, srcstep, r0, r1, r2, r3, dstwidth, $srcbpp, $nostep
        MEND

;----------------------------------------------------------------------------
; THREESURFINIT handles entry for a three surface operation.  It pulls the
;  associated parameters off of the stack and initializes the following
;  common registers:
;   dstptr
;   dststep
;   srcptr
;   srcstep
;   mskptr
;   mskstep
;   dstwidth
;   dstheight
;
; $dstbpp provides the bits per pixel of the destination surface
; $srcbpp provides the bits per pixel of the source surface
; $mskbpp provides the bits per pixel of the mask surface
; $regspushed provides the number of registers pushed on the stack since
;  the parameters were pushed on the stack, so that on entry, this macro
;  assumes:
;   r0 = dstptr
;   r1 = dststride in bytes
;   r2 = dstleft in pixels
;   r3 = dsttop in lines
;   sp + (($regspushed + 0) * 4) = dstwidth in pixels
;   sp + (($regspushed + 1) * 4) = dstheight in lines
;   sp + (($regspushed + 2) * 4) = srcptr
;   sp + (($regspushed + 3) * 4) = srcstride in bytes
;   sp + (($regspushed + 4) * 4) = srcleft in pixels
;   sp + (($regspushed + 5) * 4) = srctop in lines
;   sp + (($regspushed + 6) * 4) = mskptr
;   sp + (($regspushed + 7) * 4) = mskstride in bytes
;   sp + (($regspushed + 8) * 4) = mskleft in pixels
;   sp + (($regspushed + 9) * 4) = msktop in lines
;
        MACRO
        THREESURFINIT   $dstbpp, $srcbpp, $mskbpp, $regspushed, $nostep
        ldr     dstwidth, [sp, #(($regspushed + 0) * 4)]        ; get dstwidth
        ldr     dstheight, [sp, #(($regspushed + 1) * 4)]       ; get dstheight
        SURFINIT        dstptr, dststep, r0, r1, r2, r3, dstwidth, $dstbpp, $nostep
        ldr     r0, [sp, #(($regspushed + 2) * 4)]              ; srcptr
        ldr     r1, [sp, #(($regspushed + 3) * 4)]      ; srcstride
        ldr     r2, [sp, #(($regspushed + 4) * 4)]      ; srcleft
        ldr     r3, [sp, #(($regspushed + 5) * 4)]      ; srctop
        SURFINIT        srcptr, srcstep, r0, r1, r2, r3, dstwidth, $srcbpp, $nostep
        ldr     r0, [sp, #(($regspushed + 6) * 4)]      ; mskptr
        ldr     r1, [sp, #(($regspushed + 7) * 4)]      ; mskstride
        ldr     r2, [sp, #(($regspushed + 8) * 4)]      ; mskleft
        ldr     r3, [sp, #(($regspushed + 9) * 4)]      ; msktop
        SURFINIT        mskptr, mskstep, r0, r1, r2, r3, dstwidth, $mskbpp, $nostep
        MEND


;----------------------------------------------------------------------------
;
; Load and store macros simplify single Neon register lane loading below.
;
        MACRO
        LOADSTRAYS_8    $strays, $ptr, $reg0
        vld1.8  {$reg0[0]}, [$ptr]!
        cmp     $strays, #2
        blt     %f80
        vld1.8  {$reg0[1]}, [$ptr]!
        ble     %f80
        cmp     $strays, #4
        vld1.8  {$reg0[2]}, [$ptr]!
        blt     %f80
        vld1.8  {$reg0[3]}, [$ptr]!
        ble     %f80
        cmp     $strays, #6
        vld1.8  {$reg0[4]}, [$ptr]!
        blt     %f80
        vld1.8  {$reg0[5]}, [$ptr]!
        ble     %f80
        vld1.8  {$reg0[6]}, [$ptr]!
80
        MEND

        MACRO
        LOADSTRAYS_16   $strays, $ptr, $reg0, $reg1
        vld1.16 {$reg0[0]}, [$ptr]!
        cmp     $strays, #2
        blt     %f160
        vld1.16 {$reg0[1]}, [$ptr]!
        ble     %f160
        cmp     $strays, #4
        vld1.16 {$reg0[2]}, [$ptr]!
        blt     %f160
        vld1.16 {$reg0[3]}, [$ptr]!
        ble     %f160
        cmp     $strays, #6
        vld1.16 {$reg1[0]}, [$ptr]!
        blt     %f160
        vld1.16 {$reg1[1]}, [$ptr]!
        ble     %f160
        vld1.16 {$reg1[2]}, [$ptr]!
160
        MEND

        MACRO
        LOADSTRAYS_3x8  $strays, $ptr, $reg0, $reg1, $reg2
        vld3.8  {$reg0[0],$reg1[0],$reg2[0]}, [$ptr]!
        cmp     $strays, #2
        blt     %f240
        vld3.8  {$reg0[1],$reg1[1],$reg2[1]}, [$ptr]!
        ble     %f240
        cmp     $strays, #4
        vld3.8  {$reg0[2],$reg1[2],$reg2[2]}, [$ptr]!
        blt     %f240
        vld3.8  {$reg0[3],$reg1[3],$reg2[3]}, [$ptr]!
        ble     %f240
        cmp     $strays, #6
        vld3.8  {$reg0[4],$reg1[4],$reg2[4]}, [$ptr]!
        blt     %f240
        vld3.8  {$reg0[5],$reg1[5],$reg2[5]}, [$ptr]!
        ble     %f240
        vld3.8  {$reg0[6],$reg1[6],$reg2[6]}, [$ptr]!
240
        MEND

        MACRO
        LOADSTRAYS_4x8  $strays, $ptr, $reg0, $reg1, $reg2, $reg3
        vld4.8  {$reg0[0],$reg1[0],$reg2[0],$reg3[0]}, [$ptr]!
        cmp     $strays, #2
        blt     %f320
        vld4.8  {$reg0[1],$reg1[1],$reg2[1],$reg3[1]}, [$ptr]!
        ble     %f320
        cmp     $strays, #4
        vld4.8  {$reg0[2],$reg1[2],$reg2[2],$reg3[2]}, [$ptr]!
        blt     %f320
        vld4.8  {$reg0[3],$reg1[3],$reg2[3],$reg3[3]}, [$ptr]!
        ble     %f320
        cmp     $strays, #6
        vld4.8  {$reg0[4],$reg1[4],$reg2[4],$reg3[4]}, [$ptr]!
        blt     %f320
        vld4.8  {$reg0[5],$reg1[5],$reg2[5],$reg3[5]}, [$ptr]!
        ble     %f320
        vld4.8  {$reg0[6],$reg1[6],$reg2[6],$reg3[6]}, [$ptr]!
320
        MEND

        MACRO
        STORESTRAYS_16  $strays, $ptr, $reg0, $reg1
        cmp     $strays, #2
        vst1.16 {$reg0[0]}, [$ptr]!
        blt     %f161
        vst1.16 {$reg0[1]}, [$ptr]!
        ble     %f161
        cmp     $strays, #4
        vst1.16 {$reg0[2]}, [$ptr]!
        blt     %f161
        vst1.16 {$reg0[3]}, [$ptr]!
        ble     %f161
        cmp     $strays, #6
        vst1.16 {$reg1[0]}, [$ptr]!
        blt     %f161
        vst1.16 {$reg1[1]}, [$ptr]!
        ble     %f161
        vst1.16 {$reg1[2]}, [$ptr]!
161
        MEND
        
        MACRO
        STORESTRAYS_3x8 $strays, $ptr, $reg0, $reg1, $reg2
        cmp     $strays, #2
        vst3.8  {$reg0[0],$reg1[0],$reg2[0]}, [$ptr]!
        blt     %f241
        vst3.8  {$reg0[1],$reg1[1],$reg2[1]}, [$ptr]!
        ble     %f241
        cmp     $strays, #4
        vst3.8  {$reg0[2],$reg1[2],$reg2[2]}, [$ptr]!
        blt     %f241
        vst3.8  {$reg0[3],$reg1[3],$reg2[3]}, [$ptr]!
        ble     %f241
        cmp     $strays, #6
        vst3.8  {$reg0[4],$reg1[4],$reg2[4]}, [$ptr]!
        blt     %f241
        vst3.8  {$reg0[5],$reg1[5],$reg2[5]}, [$ptr]!
        ble     %f241
        vst3.8  {$reg0[6],$reg1[6],$reg2[6]}, [$ptr]!
241
        MEND

        MACRO
        STORESTRAYS_4x8 $strays, $ptr, $reg0, $reg1, $reg2, $reg3
        cmp     $strays, #2
        vst4.8  {$reg0[0],$reg1[0],$reg2[0],$reg3[0]}, [$ptr]!
        blt     %f321
        vst4.8  {$reg0[1],$reg1[1],$reg2[1],$reg3[1]}, [$ptr]!
        ble     %f321
        cmp     $strays, #4
        vst4.8  {$reg0[2],$reg1[2],$reg2[2],$reg3[2]}, [$ptr]!
        blt     %f321
        vst4.8  {$reg0[3],$reg1[3],$reg2[3],$reg3[3]}, [$ptr]!
        ble     %f321
        cmp     $strays, #6
        vst4.8  {$reg0[4],$reg1[4],$reg2[4],$reg3[4]}, [$ptr]!
        blt     %f321
        vst4.8  {$reg0[5],$reg1[5],$reg2[5],$reg3[5]}, [$ptr]!
        ble     %f321
        vst4.8  {$reg0[6],$reg1[6],$reg2[6],$reg3[6]}, [$ptr]!
321
        MEND

;----------------------------------------------------------------------------
; RGB565toRGB888 converts a set of eight 16-bit RGB 5:6:5 pixels to
;  RGB 8:8:8.  Three Dn registers and a Qn register are passed in.  The Qn
;  register contains the input pixels, while the Dn registers will contain
;  the converted pixels, separated into R, G, and B channels.
;
        MACRO
        RGB565toRGB888  $outred8x8, $outgrn8x8, $outblu8x8, $in16x8
 IF CLRLSBS565TO888
        vmovn.u16       $outblu8x8, $in16x8             ; gggbbbbb x 8
        vshrn.u16       $outred8x8, $in16x8, #8         ; rrrrrggg x 8
        vshrn.u16       $outgrn8x8, $in16x8, #5         ; rrgggggg x 8
        vshr.u8         $outred8x8, $outred8x8, #3      ; 000rrrrr x 8
        vshl.u8         $outblu8x8, $outblu8x8, #3      ; bbbbb000 x 8
        vshl.u8         $outred8x8, $outred8x8, #3      ; rrrrr000 x 8
        vshl.u8         $outgrn8x8, $outgrn8x8, #2      ; gggggg00 x 8
 ELSE
        vmovn.u16       $outblu8x8, $in16x8             ; gggbbbbb x 8
        vshrn.u16       $outred8x8, $in16x8, #8         ; rrrrrggg x 8
        vshrn.u16       $outgrn8x8, $in16x8, #3         ; ggggggbb x 8
        vshl.u8         $outblu8x8, $outblu8x8, #3      ; bbbbb000 x 8
        vsri.u8         $outgrn8x8, $outgrn8x8, #6      ; gggggggg x 8
        vsri.u8         $outred8x8, $outred8x8, #5      ; rrrrrrrr x 8
        vsri.u8         $outblu8x8, $outblu8x8, #5      ; bbbbbbbb x 8
 ENDIF
        MEND

; V2
; If the red and green Dn registers are chosen to be in the same Qn
;  register, this macro can eliminate a cycle.
;
        MACRO
        RGB565toRGB888v2        $outred8x8, $outgrn8x8, $outblu8x8, $outredblu8x8x2, $in16x8
        vmovn.u16       $outblu8x8, $in16x8             ; gggbbbbb x 8
        vshrn.u16       $outred8x8, $in16x8, #8         ; rrrrrggg x 8
        vshrn.u16       $outgrn8x8, $in16x8, #3         ; ggggggbb x 8
        vshl.u8         $outblu8x8, $outblu8x8, #3      ; bbbbb000 x 8
        vsri.u8         $outgrn8x8, $outgrn8x8, #6      ; gggggggg x 8
        vsri.u8         $outredblu8x8x2, $outredblu8x8x2, #5
        MEND

;----------------------------------------------------------------------------
; RGB888toRGB565 converts a set of eight 24-bit RGB 8:8:8 pixels to
;  RGB 5:6:5.  Three Dn registers and a Qn register are passed in.  The Dn
;  registers contains the input pixels, while the Qn register will contain
;  the converted pixels.
;
        MACRO
        RGB888toRGB565  $out16x8, $inred8x8, $ingrn8x8, $inblu8x8, $tmpgrn16x8, $tmpblu16x8
        vshll.u8        $out16x8, $inred8x8, #8
        vshll.u8        $tmpgrn16x8, $ingrn8x8, #8
        vshll.u8        $tmpblu16x8, $inblu8x8, #8
        vsri.u16        $out16x8, $tmpgrn16x8, #5
        vsri.u16        $out16x8, $tmpblu16x8, #11
        MEND

;----------------------------------------------------------------------------
; BLEND blends one set of eight ARGB 8:8:8:8 pixels into another set of eight
;  ARGB 8:8:8:8 pixels.  Note that this macro DOES NOT calculate the resulting
;  alpha of the destination.  (However, *alf8x8 are not modified.)  For alpha
;  calculation use BLENDPMA instead.
;
; Co = AaCa + AbCb
;
; $dst* are input and output registers (except alpha)
; $src* are input and working registers
; $tmp* are working registers, and should not conflict with other arguments
;
; Sample 1:  Premultiplied ARGB blended to xRGB
;       vmov.u8 srcalf8x8, #0xFF        ; 1.0 used for source (created outside octet loop)
; ...
;       vmvn.u8 dstalf8x8, srcalf8x8    ; (1.0 - srcalf) used for destination
;       BLEND   dstred8x8, dstgrn8x8, dstblu8x8, dstalf8x8, srcred8x8, srcgrn8x8, srcblu8x8, srcalf8x8, qa, qb, qc
;
; Sample 2:  Non-premultiplied xRGB blended to xRGB using separate 8-bit alpha
;                                       ; mskalf used for source
;       vmvn.u8 dstalf8x8, mskalf8x8    ; (1.0 - mskalf) used for destination
;       BLEND   dstred8x8, dstgrn8x8, dstblu8x8, dstalf8x8, srcred8x8, srcgrn8x8, srcblu8x8, mskalf8x8, qa, qb, qc
;
        MACRO
        BLEND   $dstred8x8, $dstgrn8x8, $dstblu8x8, $dstalf8x8, $srcred8x8, $srcgrn8x8, $srcblu8x8, $srcalf8x8, $tmpred16x8, $tmpgrn16x8, $tmpblu16x8, $tmp2red16x8, $tmp2grn16x8, $tmp2blu16x8
; Multiply destination by destination alpha
        vmull.u8        $tmpred16x8, $dstred8x8, $dstalf8x8
        vmull.u8        $tmpgrn16x8, $dstgrn8x8, $dstalf8x8
        vmull.u8        $tmpblu16x8, $dstblu8x8, $dstalf8x8
; Multiply source by source alpha and add to destination
        vmlal.u8        $tmpred16x8, $srcred8x8, $srcalf8x8
        vmlal.u8        $tmpgrn16x8, $srcgrn8x8, $srcalf8x8
        vmlal.u8        $tmpblu16x8, $srcblu8x8, $srcalf8x8
; Get value / 256
        vrshr.u16       $tmp2red16x8, $tmpred16x8, #8
        vrshr.u16       $tmp2grn16x8, $tmpgrn16x8, #8
        vrshr.u16       $tmp2blu16x8, $tmpblu16x8, #8
; Adjust value by 257/256
        vraddhn.u16     $dstred8x8, $tmp2red16x8, $tmpred16x8
        vraddhn.u16     $dstgrn8x8, $tmp2grn16x8, $tmpgrn16x8
        vraddhn.u16     $dstblu8x8, $tmp2blu16x8, $tmpblu16x8
        MEND

;
; BLENDPMA is premultipled alpha blending.  It modifies the destination alpha.
;
; Co = Ca + Cb(1-Aa)
; Ao = Aa + Ab(1-Aa)
;
        MACRO
        BLENDPMA        $dstred8x8, $dstgrn8x8, $dstblu8x8, $dstalf8x8, $srcred8x8, $srcgrn8x8, $srcblu8x8, $srcalf8x8, $negsrcalf8x8, $tmpred16x8, $tmpgrn16x8, $tmpblu16x8, $tmpalf16x8, $tmp2red16x8, $tmp2grn16x8, $tmp2blu16x8, $tmp2alf16x8
; Get (1 - source alpha)
        vmvn.u8         $negsrcalf8x8, $srcalf8x8
; Multiply destination by (1 - source alpha)
        vmull.u8        $tmpred16x8, $dstred8x8, $negsrcalf8x8
        vmull.u8        $tmpgrn16x8, $dstgrn8x8, $negsrcalf8x8
        vmull.u8        $tmpblu16x8, $dstblu8x8, $negsrcalf8x8
        vmull.u8        $tmpalf16x8, $dstalf8x8, $negsrcalf8x8
; Multiply results by 257/256
        vrshr.u16       $tmp2red16x8, $tmpred16x8, #8
        vrshr.u16       $tmp2grn16x8, $tmpgrn16x8, #8
        vrshr.u16       $tmp2blu16x8, $tmpblu16x8, #8
        vrshr.u16       $tmp2alf16x8, $tmpalf16x8, #8
; Shrink to 8 bits and round
        vraddhn.u16     $dstred8x8, $tmp2red16x8, $tmpred16x8
        vraddhn.u16     $dstgrn8x8, $tmp2grn16x8, $tmpgrn16x8
        vraddhn.u16     $dstblu8x8, $tmp2blu16x8, $tmpblu16x8
        vraddhn.u16     $dstalf8x8, $tmp2alf16x8, $tmpalf16x8
; Add premultiplied source
        vqadd.u8        $dstred8x8, $dstred8x8, $srcred8x8
        vqadd.u8        $dstgrn8x8, $dstgrn8x8, $srcgrn8x8
        vqadd.u8        $dstblu8x8, $dstblu8x8, $srcblu8x8
        vqadd.u8        $dstalf8x8, $dstalf8x8, $srcalf8x8
        MEND

;----------------------------------------------------------------------------
; PRELOADLINEnbpp are macros to ease use of the PreloadLine() function.
;
        MACRO
        PRELOADLINE1bpp $ptr, $pels
        mov             r0, $ptr
        mov             r1, $pels, lsr #3
        bl              PreloadLine
        MEND
        
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        MACRO
        PRELOADLINE8bpp $ptr, $pels
        mov             r0, $ptr
        mov             r1, $pels
        bl              PreloadLine
        MEND

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        MACRO
        PRELOADLINE16bpp        $ptr, $pels
        mov             r0, $ptr
        mov             r1, $pels, lsl #1
        bl              PreloadLine
        MEND

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        MACRO
        PRELOADLINE24bpp        $ptr, $pels
        mov             r0, $ptr
        add             r1, $pels, $pels, lsl #2
        bl              PreloadLine
        MEND

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        MACRO
        PRELOADLINE32bpp        $ptr, $pels
        mov             r0, $ptr
        mov             r1, $pels, lsl #2
        bl              PreloadLine
        MEND

        END

