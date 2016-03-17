.xlist
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
;       This include contains IO related macros used for programming
;       the serial port.
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
;                               Revision History
;   $Log: io.inc $
;   Revision 1.2  2007/11/03 23:31:15Z  Garyp
;   Added the standard module header.
;   Revision 1.1  2005/01/17 01:32:38Z  Pauli
;   Initial revision
;   Revision 1.1  2005/01/17 01:32:38Z  Garyp
;   Initial revision
;   Revision 1.1  2005/01/17 00:32:38Z  pauli
;   Initial revision
;   Revision 1.1  2005/01/16 23:32:37Z  GaryP
;   Initial revision
 ;----------------------------------------------------------------------------


;--------------------------------------------------------------------------
;       IODELAY       ; void
;
;       This macro performs a delay for IO to process
;--------------------------------------------------------------------------
IODELAY         macro ovr
.errnb <ovr>
        jmp     $+2
        jmp     $+2
        jmp     $+2
endm


;--------------------------------------------------------------------------
;       INPORT          ; location, offset
;
;       This macro reads into the specified location a byte value from
;       the port calculated by adding the specified offset to the current
;       value in DX.  It automatically performs a standard IODELAY for IO
;       purposes.
;
;       On exit, only AL and the flags will be modified.
;--------------------------------------------------------------------------
INPORT          MACRO   loc,offs,ovr
.errnb <ovr>
.errb  <offs>

        add     dx,offs
        IODELAY                 ;; wait on any previous IO
        in      al,dx
        sub     dx,offs
ifdifi <loc>,<al>
        mov     loc,al
endif
ENDM


;--------------------------------------------------------------------------
;       OUTPORT         ; offset, value
;
;       This macro writes the specified value to the port calculated
;       by adding the specified offset to the current value in DX.
;       It automatically performs a standard IODELAY for IO purposes.
;
;       On exit, only AL and the flags will be modified.
;--------------------------------------------------------------------------
OUTPORT         MACRO   offs,valu,ovr
.errnb <ovr>
.errb  <offs>

ifdifi <valu>,<al>
        mov     al,valu
endif

        add     dx,offs
        IODELAY                 ;; wait on any previous IO
        out     dx,al
        sub     dx,offs
ENDM
