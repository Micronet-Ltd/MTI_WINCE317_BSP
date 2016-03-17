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
;  $Log: memset16.asm $
;  Revision 1.2  2007/11/03 23:31:15Z  Garyp
;  Added the standard module header.
;  Revision 1.1  2005/10/02 02:03:40Z  Pauli
;  Initial revision
;  Revision 1.1  2005/10/02 03:03:40Z  Garyp
;  Initial revision
;  Revision 1.1  2005/01/17 00:32:38Z  pauli
;  Initial revision
;  Revision 1.1  2005/01/16 23:32:38Z  GaryP
;  Initial revision
;  Revision 1.2  2004/05/28 17:14:58Z  garyp
;  Hacked so this code does not compile if using a flat memory model.
;  Revision 1.1  2004/01/27 02:27:38Z  garyp
;  Initial revision
;----------------------------------------------------------------------------

include dcl.inc
include dlx86.inc

ModuleProlog

if FLATMODEL eq FALSE

public _DclCpuMemSet16


;--------------------------------------------------------------------
;   DclCpuMemSet16() - Initialize a block of memory
;
;   Description
;       This function performs a traditional memset() operation on
;       a block of memory.  The following prototype is used:
;
;       void * DclCpuMemSet16(void * pMem, D_UCHAR cVal, D_UINT16 uLen)
;
;       This function is specifically designed to work in environments
;       with COMPACT memory model (near code, far data), and a 16-bit
;       stack.
;
;   Return Value
;       A pointer to the buffer.
;--------------------------------------------------------------------
_DclCpuMemSet16 proc
        push    bp
        mov     bp, sp
        push    di

        les     di, dptr ss:[bp+4]
        mov     ax, wptr ss:[bp+8]
        mov     cx, wptr ss:[bp+10]
        cld
        rep     stosb

        les     di, dptr ss:[bp+4]
        mov     dx,ax

        pop     di
        pop     bp
        ret
_DclCpuMemSet16 endp



else

.data
; Define this simply to avoid assembler warnings
public	bDclMemsetData
bDclMemsetData	db  0


endif 	; FLATMODEL eq FALSE


end
