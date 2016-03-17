;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this source code is subject to the terms of the Microsoft end-user
; license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
; If you did not accept the terms of the EULA, you are not authorized to use
; this source code. For a copy of the EULA, please see the LICENSE.RTF on your
; install media.
;
; Copyright (c) 1998, 1999 ARM Limited
; All Rights Reserved
;

    OPT     2       ; disable listing
    INCLUDE kxarm.h
    OPT     1       ; reenable listing
    OPT     128     ; disable listing of macro expansions
    INCLUDE armmacros.s

    TEXTAREA

    LEAF_ENTRY      OALDrainWriteBuffer
;++
; Routine Description:
;    Dat Synchronization barrier.
;
; Syntax:
;       void OALDrainWriteBuffer
;
; Arguments:
;       -- none --
;
; Return Value:
;       -- none --
;--
    mov     r0, #0
    mcr     p15, 0, r0, c7, c10, 4   ; drain write buffer

    RETURN

    END
