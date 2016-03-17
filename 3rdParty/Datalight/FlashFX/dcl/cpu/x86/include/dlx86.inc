;----------------------------------------------------------------------------
;              ----> DO NOT REMOVE THE FOLLOWING NOTICE <----
;
;                 Copyright (c) 1993 - 2008 Datalight, Inc.
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
;       This include contains x86 macros and defines.
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
;                               Revision History
;       $Log: dlx86.inc $
;       Revision 1.1  2008/11/28 20:37:22Z  garyp
;       Initial revision
;       Revision 1.2.1.2  2008/06/23 07:34:24Z  garyp
;       Updated to support the "mswce" ToolSet abstraction.
;       Revision 1.2  2007/11/03 23:31:22Z  Garyp
;       Added the standard module header.
;       Revision 1.1  2005/02/14 03:02:40Z  Pauli
;       Initial revision
;       Revision 1.2  2005/02/14 01:02:40Z  GaryP
;       Updated to use the renamed toolset designations.
;       Revision 1.1  2005/01/17 00:10:52Z  GaryP
;       Initial revision
;----------------------------------------------------------------------------


if D_TOOLSETNUM EQ TS_BC4X

FLATMODEL	equ <FALSE>

ModuleProlog    macro
.MODEL COMPACT
.CODE
                endm

elseif D_TOOLSETNUM EQ TS_DEVTOOLS

FLATMODEL	equ <FALSE>

ModuleProlog    macro
.MODEL COMPACT
.CODE
                endm

elseif D_TOOLSETNUM EQ TS_MSWCE

FLATMODEL	equ <TRUE>

ModuleProlog    macro
.486p
.MODEL FLAT
.CODE
                endm

elseif D_TOOLSETNUM EQ TS_MSWCE4

FLATMODEL	equ <TRUE>

ModuleProlog    macro
.486p
.MODEL FLAT
.CODE
                endm

elseif D_TOOLSETNUM EQ TS_MSWCEPB

FLATMODEL	equ <TRUE>

ModuleProlog    macro
.486p
.MODEL FLAT
.CODE
                endm

else

% .err <DLX86.INC: Unrecognized ToolSet Number "&D_TOOLSETNUM">

endif


PUSHAD_STRUC    struc
  pushad_EDI    dd  0
  pushad_ESI    dd  0
  pushad_EBP    dd  0
  pushad_ESP    dd  0
  pushad_EBX    dd  0
  pushad_EDX    dd  0
  pushad_ECX    dd  0
  pushad_EAX    dd  0
PUSHAD_STRUC    ends


CPUFLAG_CARRY           equ <0001h>
CPUFLAG_RESERVED1       equ <0002h>
CPUFLAG_PARITY          equ <0004h>
CPUFLAG_RESERVED2       equ <0008h>
CPUFLAG_AUX             equ <0010h>
CPUFLAG_RESERVED3       equ <0020h>
CPUFLAG_ZERO            equ <0040h>
CPUFLAG_SIGN            equ <0080h>
CPUFLAG_TRAP            equ <0100h>
CPUFLAG_INTERRUPT       equ <0200h>
CPUFLAG_DIRECTION       equ <0400h>
CPUFLAG_OVERFLOW        equ <0800h>
CPUFLAG_IOPLMASK        equ <3000h>
CPUFLAG_NESTEDTASK      equ <4000h>
CPUFLAG_RESERVED4       equ <8000h>
CPUFLAG_RESUME          equ <00010000h>    ; 386 and later
CPUFLAG_V86             equ <00020000h>    ; 386 and later
CPUFLAG_ALIGNMENT       equ <00040000h>    ; 386 and later
CPUFLAG_PENTIUMID       equ <00200000h>    ; 386 and later



bptr    equ <byte ptr>
wptr    equ <word ptr>
dptr    equ <dword ptr>
