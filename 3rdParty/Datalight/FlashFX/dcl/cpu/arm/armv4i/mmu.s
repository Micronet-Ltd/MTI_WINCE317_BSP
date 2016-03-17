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
;       This module contains ARMV4I specific code to manipulate the MMU.
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
;                               Revision History
;       $Log: mmu.s $
;       Revision 1.2  2009/02/08 05:00:58Z  garyp
;       No longer use a relative include path (now handled in the make file).
;       Revision 1.1  2008/10/23 22:47:04Z  garyp
;       Initial revision
;----------------------------------------------------------------------------
	OPT	2	; disable listing

        INCLUDE dlarm.inc

	OPT	1	; reenable listing
	OPT	128	; disable listing of macro expansions

	TEXTAREA


;--------------------------------------------------------------------
;       Public: DclCpuIsMmuOn()
;
;       This function determines whether the MMU is on or off.
;
;	Parameters:
;		None
;
;       Return Value:
;		TRUE if the MMU is on, or FALSE otherwise.
;--------------------------------------------------------------------
        FUNC_BEGIN      DclCpuIsMmuOn
		mrc     p15, 0, r0, c1, c0, 0
		and	r0, r0, #1
		bx	lr
        FUNC_END        DclCpuIsMmuOn




	END


