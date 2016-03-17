;----------------------------------------------------------------------------
;              ----> DO NOT REMOVE THE FOLLOWING NOTICE <----
;
;                 Copyright (c) 1993 - 2007 Datalight, Inc.
;                      All Rights Reserved Worldwide.
;
; Datalight, Incorporated is a Washington State corporation located at:
;
;       21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
;       Bothell, WA  98021                Fax:  425-951-8094
;       USA                               Web:  http://www.datalight.com
;
; This software, including without limitation all source code and documen-
; tation, is the confidential, trade secret property of Datalight, Inc.
; and is protected under the copyright laws of the United States and other
; jurisdictions.  Patents may be pending.
;
; In addition to civil penalties for infringement of copyright under appli-
; cable U.S. law, 17 U.S.C. 1204 provides criminal penalties for violation
; of (a) the restrictions on circumvention of copyright protection systems
; found in 17 U.S.C. 1201 and (b) the protections for the integrity of
; copyright management information found in 17 U.S.C. 1202.
;
; U.S. Government Restricted Rights:  Use, duplication, reproduction, or
; transfer of this commercial product and accompanying documentation is
; restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
; License Agreement.
;
; IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
; A SOURCE CODE AGREEMENT, NON-DISCLOSURE AGREEMENT, AND/OR SIMILAR BINDING
; CONTRACT BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
; IF YOU ARE A LICENSEE, YOUR RIGHT, IF ANY, TO COPY, PUBLISH, MODIFY, OR
; OTHERWISE USE THE SOFTWARE, IS SUBJECT TO THE TERMS AND CONDITIONS OF THE
; BINDING AGREEMENTS.  BY USING THE SOFTWARE IN ANY MANNER, IN WHOLE OR IN
; PART, YOU AGREE TO BE BOUND BY THE TERMS OF THE BINDING AGREEMENTS.
;
; IF YOU ARE NOT A DATALIGHT LICENSEE, ANY USE MAY RESULT IN CIVIL AND
; CRIMINAL ACTION AGAINST YOU.  CONTACT DATALIGHT, INC. AT THE ADDRESS
; SET FORTH ABOVE IF YOU OBTAINED THIS SOFTWARE IN ERROR.
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
;                               Description
;
;   This module contains diagnostic output routines used for debugging
;   assembly language code in the BIOS extension and DOS device driver.
;
;   Parameters are passed in registers and all registers are preserved
;   unless explicitly returning values.
;
;   The functions are typically used via the macros found in diag.inc.
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
;                               Revision History
;   $Log: diag.asm $
;   Revision 1.2  2007/11/03 23:31:15Z  Garyp
;   Added the standard module header.
;   Revision 1.1  2005/01/17 01:32:40Z  Pauli
;   Initial revision
;   Revision 1.1  2005/01/17 01:32:40Z  Garyp
;   Initial revision
;   Revision 1.1  2005/01/17 00:32:40Z  pauli
;   Initial revision
;   Revision 1.1  2005/01/16 23:32:39Z  GaryP
;   Initial revision
;----------------------------------------------------------------------------
include dcl.inc
include dlx86.inc

ModuleProlog

extrn   __sysdisplaytext:proc

public  _DclCpuDisplayChar
public  _DclCpuDisplayString
public  _DclCpuDisplayHexByte
public  _DclCpuDisplayHexWord


;--------------------------------------------------------------------
;       _DclCpuDisplayString() - Display a string
;
;	Description
;           This function displays the string specified in ES:BX using
;           the _sysdisplaytext() function.
;
;       Return Value
;	    None -- all registers are preserved
;--------------------------------------------------------------------
_DclCpuDisplayString    proc
        push    bp              ; We're calling a C function, so save
        push    ax              ; everything just be safe
        push    bx              ;
        push    cx
        push    dx
        push    si
        push    di
        push    es

        push    es
        push    bx
        call    __sysdisplaytext
        add     sp, 4

        pop     es
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        pop     bp
        ret
_DclCpuDisplayString    endp


;--------------------------------------------------------------------
;       _DclCpuDisplayChar() - Display a character
;
;	Description
;   	    This function displays the specified character in AL using the
;           _sysdisplaytext() function.
;
;       Return Value
;	    None -- all registers are preserved
;--------------------------------------------------------------------
_DclCpuDisplayChar      proc
        push    bp              ; We're calling a C function, so save
        push    ax              ; everything just be safe
        push    bx              ;
        push    cx
        push    dx
        push    si
        push    di
        push    es

        xor     ah, ah          ;
        push    ax              ; push the char and null terminator
        mov     bp, sp
        push    ss              ;
        push    bp              ; push a pointer to the 2 character string
        call    __sysdisplaytext
        add     sp, 4 + 2       ; clean up the stack

        pop     es
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        pop     bp
        ret
_DclCpuDisplayChar      endp


;--------------------------------------------------------------------
;       _DclCpuDisplayHexNybble()
;
;	Description
;           Display the hexadecimal nybble in AL (the low nybble) via
;           the standard output method.
;
;       Return Value
;	    None -- all registers are preserved
;--------------------------------------------------------------------
_DclCpuDisplayHexNybble proc
        push    ax

        and     al,0Fh
        add     al,'0'
        cmp     al,'9'
        jbe     fdn_Display

        add     al,'A'-(10+'0')

  fdn_Display:
        call    _DclCpuDisplayChar

        pop     ax
	ret
_DclCpuDisplayHexNybble endp


;--------------------------------------------------------------------
;       _DclCpuDisplayHexByte()
;
;	Description
;           Display the byte in AL in hex format via the standard output
;           method.
;
;       Return Value
;	    None -- all registers are preserved
;--------------------------------------------------------------------
_DclCpuDisplayHexByte   proc
        ror     al,4
        call    _DclCpuDisplayHexNybble ; display the high nybble
        ror     al,4                    ; restore original AL value
        call    _DclCpuDisplayHexNybble ; display the low nybble
        ret
_DclCpuDisplayHexByte   endp


;--------------------------------------------------------------------
;       _DclCpuDisplayHexWord()
;
;	Description
;           Display the word in AX in hex format via the standard output
;           method.
;
;       Return Value
;	    None -- all registers are preserved
;--------------------------------------------------------------------
_DclCpuDisplayHexWord   proc
        xchg    ah,al
        call    _DclCpuDisplayHexByte   ; display the high byte
        xchg    ah,al                   ; restore original AX value
        call    _DclCpuDisplayHexByte   ; display the low byte
        ret
_DclCpuDisplayHexWord   endp




end

