;----------------------------------------------------------------------------
;              ----> DO NOT REMOVE THE FOLLOWING NOTICE <----
;
;                 Copyright (c) 1993 - 2009 Datalight, Inc.
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
;       This module contains ARMV4 specific code to manipulate interrupts.
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
;                               Revision History
;       $Log: intr.s $
;       Revision 1.3  2009/02/07 23:39:37Z  garyp
;       No longer use a relative include path (now handled in the make file).
;       Revision 1.2  2007/11/03 23:31:14Z  Garyp
;       Added the standard module header.
;       Revision 1.1  2005/01/17 01:17:58Z  Pauli
;       Initial revision
;----------------------------------------------------------------------------
	OPT	2	; disable listing

        INCLUDE dlarm.inc

	OPT	1	; reenable listing
	OPT	128	; disable listing of macro expansions

	TEXTAREA


;--------------------------------------------------------------------
;       Public: DclCpuInterruptDisable()
;
;       This function disables interrupts and returns a value that
;	may be used to restore the original interrupt state by
;	calling DclCpuInterruptRestore().
;
;       Return Value:
;           Returns a D_UINT32 that is a processor dependent
;	    value and is only meaningful as a parameter to
;	    DclCpuInterruptRestore().
;--------------------------------------------------------------------
        FUNC_BEGIN      DclCpuInterruptDisable
                mrs     r0, cpsr
                orr     r1, r0, #0xC0
                msr     cpsr_c, r1
                and     r0, r0, #0xC0
                mov     pc, lr
        FUNC_END        DclCpuInterruptDisable


;--------------------------------------------------------------------
;       Public: DclCpuInterruptRestore()
;
;       This routine restores the interrupt state to its original
;       value.  The ulOldState value must be a value returned by
;       DclCpuInterruptDisable().
;
;       Return Value:
;           None.
;--------------------------------------------------------------------
        FUNC_BEGIN      DclCpuInterruptRestore
                and     r0, r0, #0xC0
                eor     r0, r0, #0xC0
                mrs     r1, cpsr
                bic     r1, r1, r0
                msr     cpsr_c, r1
                mov     pc, lr
        FUNC_END        DclCpuInterruptRestore


	END
