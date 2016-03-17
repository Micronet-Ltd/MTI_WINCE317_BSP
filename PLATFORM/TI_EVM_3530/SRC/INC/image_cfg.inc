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
;------------------------------------------------------------------------------
;
;  File:  image_cfg.inc
;



;------------------------------------------------------------------------------
;
;  Define:  IMAGE_XLDR_xxx
;
;  These constants are used to define the location of the XLDR code, RAM, and
;  stack. The locations defined below are for the XLDR running in SRAM.
;  If these addresses are modified, the corresponding xldrtoc.raw file
;  would need to be updated as well. 
;

IMAGE_XLDR_CODE_PA      EQU 0x40202000
IMAGE_XLDR_CODE_SIZE    EQU 0x00009000

IMAGE_XLDR_DATA_PA      EQU 0x4020B000
IMAGE_XLDR_DATA_SIZE    EQU 0x00004000

IMAGE_XLDR_STACK_PA     EQU 0x4020F000
IMAGE_XLDR_STACK_SIZE   EQU 0x00001000


;------------------------------------------------------------------------------
;
;  Define: IMAGE_EBOOT_xxx
;
;  Following constants define EBOOT image layout. 
;

IMAGE_EBOOT_CODE_PA     EQU 0x84E00000
IMAGE_EBOOT_CODE_SIZE   EQU 0x00040000

IMAGE_EBOOT_DATA_PA     EQU 0x84E80000
IMAGE_EBOOT_DATA_SIZE   EQU 0x00050000

IMAGE_EBOOT_STACK_PA    EQU 0x84EE0000
IMAGE_EBOOT_STACK_SIZE  EQU 0x00020000


;------------------------------------------------------------------------------
;
;  Define: IMAGE_IPL_xxx
;
;  Following constants define IPL image layout.
;

IMAGE_IPL_CODE_PA       EQU 0x84F00000
IMAGE_IPL_CODE_SIZE     EQU 0x00020000

IMAGE_IPL_DATA_PA       EQU 0x84F80000
IMAGE_IPL_DATA_SIZE     EQU 0x00050000

IMAGE_IPL_STACK_PA      EQU 0x84FE0000
IMAGE_IPL_STACK_SIZE    EQU 0x00020000

                        
IMAGE_SHARE_ARGS_PA			EQU  0x80000000
 IF BSP_OPM_AUTO=1
IMAGE_SHARE_ARGS_SIZE		EQU (0x00001000 - 8)
IMAGE_SHARE_OPM				EQU (IMAGE_SHARE_ARGS_PA + IMAGE_SHARE_ARGS_SIZE)
IMAGE_SHARE_RAM_TOPOLOGY	EQU (IMAGE_SHARE_OPM + 4)
 ELSE
IMAGE_SHARE_ARGS_SIZE		EQU (0x00001000 - 4)
IMAGE_SHARE_RAM_TOPOLOGY	EQU (IMAGE_SHARE_ARGS_PA + IMAGE_SHARE_ARGS_SIZE)
 ENDIF

                        END
