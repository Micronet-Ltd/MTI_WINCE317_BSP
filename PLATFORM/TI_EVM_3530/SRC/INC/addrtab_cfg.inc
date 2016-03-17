; Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

;
;==============================================================================
;             Texas Instruments OMAP(TM) Platform Software
; (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
;
; Use of this software is controlled by the terms and conditions found
; in the license agreement under which this software has been supplied.
;
;==============================================================================
;
;------------------------------------------------------------------------------
;
;  File:  memory_cfg.inc
;
;  This file is used to define g_oalAddressTable. This table is passed to
;  KernelStart to estabilish physical to virtual memory mapping. This table
;  is used also in memory OAL module to map between physical and virtual
;  memory addresses via OALPAtoVA/OALVAtoPA functions.
;
;  The config.bib file defines image memory layout ant it contains virtual
;  cached memory addresses which must be synchronized with g_oalAddressTable.
;  With each table change make sure that those constant are still valid.
;
;------------------------------------------------------------------------------
;  Export Definition

        EXPORT  g_oalAddressTable[DATA]

;------------------------------------------------------------------------------
;  Table format: cached address, physical address, size

g_oalAddressTable
	DCD     0x80000000, 0x80000000, 256     ; LPDDR
	DCD     0x90000000, 0xA0000000, 231     ; LPDDR
	DCD     0x9E700000, 0x48000000,  16     ; L4 Core/Wakeup registers
	DCD     0x9F700000, 0x49000000,   1     ; L4 Peripheral
	DCD     0x9F800000, 0x50000000,   1     ; SGX
	DCD     0x9F900000, 0x68000000,   2     ; L3 registers
	DCD     0x9FB00000, 0x6C000000,   1     ; SMS registers
	DCD     0x9FC00000, 0x6D000000,   1     ; SDRC registers
	DCD     0x9FD00000, 0x6E000000,   1     ; GPMC registers
	DCD     0x9FE00000, 0x08000000,   1     ; NAND FIFO
	DCD     0x9FF00000, 0x40200000,   1		; 64KB SRAM
	DCD     0x00000000, 0x00000000,   0     ; end of table
;------------------------------------------------------------------------------

        END
