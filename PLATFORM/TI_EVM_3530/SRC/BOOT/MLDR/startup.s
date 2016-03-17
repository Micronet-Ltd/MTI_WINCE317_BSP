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
;   File:  startup.s
;
;   Boot startup routine for 3430 boards.
;

        INCLUDE kxarm.h
        INCLUDE bsp.inc
        
        IMPORT  BootloaderMain

        STARTUPTEXT
        
;-------------------------------------------------------------------------------
;
;  Function:  StartUp
;
;  This function is entry point to Windows CE EBOOT. It should be called
;  in state with deactivated MMU and disabled caches.
;
;  Note that g_oalAddressTable is needed for OEMMapMemAddr because
;  downloaded image is placed on virtual addresses (but EBOOT runs without
;  MMU).
;
        LEAF_ENTRY StartUp

        ;---------------------------------------------------------------
        ; Jump to BootMain
        ;---------------------------------------------------------------


        ldr     sp, =(IMAGE_EBOOT_STACK_PA + IMAGE_EBOOT_STACK_SIZE)
        b       BootloaderMain

        ENTRY_END 

        ; Include memory configuration file with g_oalAddressTable
        INCLUDE addrtab_cfg.inc

        END

;------------------------------------------------------------------------------
