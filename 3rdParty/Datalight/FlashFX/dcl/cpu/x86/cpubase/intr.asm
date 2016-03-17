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
;       This module contains x86 specific code to manipulate interrupts.
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
;                               Revision History
;       $Log: intr.asm $
;       Revision 1.4  2009/02/08 01:12:36Z  garyp
;       Documentation updates only -- no functional changes.
;       Revision 1.3  2007/11/03 23:31:15Z  Garyp
;       Added the standard module header.
;       Revision 1.2  2006/03/16 03:48:14Z  Garyp
;       Documentation update.
;       Revision 1.1  2005/10/02 01:33:36Z  Pauli
;       Initial revision
;----------------------------------------------------------------------------

include dcl.inc
include dlx86.inc

ModuleProlog

public _DclCpuInterruptDisable
public _DclCpuInterruptRestore


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
_DclCpuInterruptDisable    proc
        pushf
        pop     ax
        cli
        xor     dx,dx
        and     ax,CPUFLAG_INTERRUPT
        ret
_DclCpuInterruptDisable    endp


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
_DclCpuInterruptRestore    proc
        and     ax,CPUFLAG_INTERRUPT
        pushf
        pop     dx
        and     dx,not CPUFLAG_INTERRUPT
        or      dx,ax
        push    dx
        popf
        ret
_DclCpuInterruptRestore    endp




end

