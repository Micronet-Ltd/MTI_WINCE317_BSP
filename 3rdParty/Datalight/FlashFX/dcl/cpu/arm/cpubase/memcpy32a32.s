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
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
;                               Revision History
;       $Log: memcpy32a32.s $
;       Revision 1.1  2009/10/17 23:17:08Z  garyp
;       Initial revision
;       Revision 1.1  2009/09/25 17:21:30Z  garyp
;       Initial revision
;----------------------------------------------------------------------------

    OPT	2	; disable listing

    INCLUDE dlarm.inc

    OPT	1	; reenable listing
    OPT	128	; disable listing of macro expansions

    TEXTAREA


;--------------------------------------------------------------------
;   Public: DclCpuMemCpyAligned32_32()
;
;   Perform a memcpy() operation of one or more 32-byte buffers,
;   where both the source and destination buffers are aligned on
;   32-bit boundaries.  Note that the ulCount field is the number
;   of 32 BYTE groups, not 32 BIT groups.  This function cannot
;   copy less than 32 bytes.
;
;   Parameters:
;	pDest    - A pointer to the 32-bit aligned destination location
;	pSrc     - A pointer to the 32-bit aligned source location
;       ulChunks - The number of 32-byte groups to copy.
;
;   Return Value:
;	None.
;--------------------------------------------------------------------
    FUNC_BEGIN  DclCpuMemCpyAligned32_32
        stmdb   sp!, {r0-r10, lr}

        ; r0 = dest, r1 = source, r2 = count of 32-byte groups

Next32Bytes
        ldmia   r1!, {r3-r10}           ; 32 bytes per instruction
        stmia   r0!, {r3-r10}
	subs	r2, r2, #1
        bne 	Next32Bytes
 
        ldmia   sp!, {r0-r10, pc}
    FUNC_END    DclCpuMemCpyAligned32_32
    	

    END

