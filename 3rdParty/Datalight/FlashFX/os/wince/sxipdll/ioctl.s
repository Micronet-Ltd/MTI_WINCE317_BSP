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
; jurisdictions.  The software may be subject to one or more of these US
; patents: US#5860082, US#6260156.  Patents may be pending.
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
;                           	Description
;
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
;                           	Revision History
;   $Log: ioctl.s $
;   Revision 1.2  2007/11/04 00:03:03Z  Garyp
;   Added the standard module header.
;----------------------------------------------------------------------------


	INCLUDE kxarm.h

; Wrapper function for switching to a stack in statically mapped memory.


	;; Allocate a private stack.  The .data section of the DLL is
	;; statically mapped because the DLL is loaded with
	;; LoadIntChainHandler().  In the debugger, it appears that about
	;; 2K of stack is sufficient.  Might as well give it the whole
	;; page.

	AREA	.data
	ALIGN	2		; 32-bit aligned
	SPACE	4096 - 16
stack	SPACE	0		; stack grows down below here


	AREA	.text, code
	IMPORT localIOControl

	NESTED_ENTRY IOControl

	;; On entry:
	;; r0-r3 contain the first four args.
	;; The remaining three args are on the stack.
	;; sp (r13) points to them.
	;; fp (r11) may or may not be valid, but must be preserved.

	;; When calling:
	;; r0-r3 contain the first four args
	;; The remaining three args have been copied to the new stack
	;; sp (r13) points to them
	;; fp is a valid frame pointer in the original stack

	mov	r12, sp		; r12 points to 3 stacked args
	stmdb	sp!, {r4-r6, fp-r12, lr}	; save permanent registers
	mov	fp, r12		;  establish frame pointer

	PROLOG_END

	;; get new stack into sp - it's initially empty

	ldr	sp, =stack

	;; copy three args that are on the old stack to the new stack

	ldmia	fp, {r4-r6}	; get args from old stack
	stmdb	sp!, {r4-r6}	; push them on the new stack
	bl	localIOControl

	;; still on the new stack, but fp points to old stack.
	;; restore permanent registers, including switching stack back

	ldmdb	fp, {r4-r6, fp, sp, pc}

	ENTRY_END

	END

