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
;   This module contains code that implements a typical memset() function
;   for a 16-bit environment.
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
;                               Revision History
;   $Log: memcpy16.asm $
;   Revision 1.2  2007/11/03 23:31:15Z  Garyp
;   Added the standard module header.
;   Revision 1.1  2005/10/02 02:04:36Z  Pauli
;   Initial revision
;   Revision 1.1  2005/10/02 03:04:36Z  Garyp
;   Initial revision
;   Revision 1.1  2005/01/17 00:32:38Z  pauli
;   Initial revision
;   Revision 1.1  2005/01/16 23:32:38Z  GaryP
;   Initial revision
;   Revision 1.2  2004/05/28 17:14:58Z  garyp
;   Hacked so this code does not compile if using a flat memory model.
;   Revision 1.1  2004/01/27 02:27:22Z  garyp
;   Initial revision
;   11/20/01 gjs For small counts, just move bytes in _dl_dos_fmemmove32
;   11/08/01 gjs Added dl_dos_fmemmove32 to support dword access
;   11/19/97 HDS Changed include file search to use predefined path.
;   08/26/97 PKG Now uses new Datalight naming convention
;   05/01/97 PKG Added __dl_dos_ prefix to all publics
;   04/14/93 DG  Created
;----------------------------------------------------------------------------

include dcl.inc
include dlx86.inc

ModuleProlog

if FLATMODEL eq FALSE

public _DclCpuMemCpy16

SMALL_COUNT     equ <15>  ; small enough to just move bytes (must always be at least 3)


;--------------------------------------------------------------------
;   DclCpuMemCpy16() - Copy a block of memory
;
;   Description
;       This function performs a traditional memcpy() operation on
;       a block of memory.  The following prototype is used:
;
;       void * DclCpuMemCpy16(void * pDest, const void * pSrc, D_UINT16 uLen)
;
;       This function is specifically designed to work in environments
;       with COMPACT memory model (near code, far data), and a 16-bit
;       stack.
;
;   Return Value
;       Returns a pointer to the destination block.
;--------------------------------------------------------------------
_DclCpuMemCpy16 proc
        push    bp
        mov     bp, sp
        push    si
        push    di
        push    ds
        push    es

        cld                     ; move is always forward

        ; get the parameters
        les     di, dptr ss:[bp+4]
        lds     si, dptr ss:[bp+8]
        mov     cx, wptr ss:[bp+12]

	; If the count is very small, then this logic is ineffective.
	; In fact, it doesn't work for a word-aligned pointer and CX=1
        cmp     cx, SMALL_COUNT
        jbe     M32_FinishBytes

        test    di, 3
        jz      M32_DWordRead
        test    di, 1
        jz      M32_WordRead
        movsb                   ; initial byte move to align word address
        dec     cx
        test    di, 2
        jz      M32_DWordRead

   M32_WordRead:
        movsw                   ; initial word move to align dword address
        dec     cx
        dec     cx

   M32_DWordRead:
        mov     ax, cx          ; save lower bit of byte count
        shr     cx, 2           ; CX now holds number of double words
				; and CF indicates the leftover word count
.386
        rep     movsd
.8086
        jnc     M32_NoWordLeft
        movsw                   ; move the remaining word

   M32_NoWordLeft:
        test    ax, 1
        jz      M32_Done
        mov     cx, 1           ; only one byte left to finish

   M32_FinishBytes:
        rep     movsb           ; move the remaining byte(s)

   M32_Done:
        les     ax, dptr ss:[bp+4]      ; return the dest address in DX:AX
        mov     dx,es

        cld
        pop     es
        pop     ds
        pop     di
        pop     si
        pop     bp
        ret
_DclCpuMemCpy16 endp



else

.data
; Define this simply to avoid assembler warnings
public	bDclMemcpyData
bDclMemcpyData	db  0



endif 	; FLATMODEL eq FALSE


end
