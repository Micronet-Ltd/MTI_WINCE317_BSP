;
;================================================================================
;             Texas Instruments OMAP(TM) Platform Software
; (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
;
; Use of this software is controlled by the terms and conditions found
; in the license agreement under which this software has been supplied.
;
;================================================================================
;
;  File:  bsp_cfg.inc
;



;------------------------------------------------------------------------------
;
;  Define:  BSP_STALL_DELAY
;
;  This constant is used to generate stall in assembler code. We assume
;  worst case when CPU clock is already at 330 MHz which give us
;  165 instruction cycles (divide by 2) per microseconds.
;
BSP_STALL_DELAY     EQU     165

                    END
