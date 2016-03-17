!if 0 
;  Copyright (c) 2013 Micronet Ltd. All rights reserved.

;================================================================================
;             Texas Instruments OMAP(TM) Platform Software
; (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
;
; Use of this software is controlled by the terms and conditions found
; in the license agreement under which this software has been supplied.
;
;================================================================================
!endif

#
# Boot ROM requires xldr to be preceeded with two words containing image 
# size and destination address.
# 
# Note that boot ROM expects xldr to be copied into 4 consecutive blocks
# to support data redundancy.  Blocks are 128K while xldr is only 16K
# so there will be gaps in each block.  For ease of creating the image
# the xldr nb0 file has been configured to be a full 128K minus the 
# table of contents.

_FILES=$(_PLATFORMROOT)\$(_TGTPLAT)\files
_SRC=$(_PLATFORMROOT)\$(_TGTPLAT)\src\boot\xldr\nand
_TGT=$(_RAWFILETGT)\$(WINCEDEBUG)
_BIN=$(_PLATFORMROOT)\$(_TGTPLAT)\bin

xldr_romimage_raw:
!if "$(NOLINK)" == ""
    romimage $(ROMIMAGE_FLAGS) xldrnand.bib
#   The OMAP boot ROM will not load the NAND XLDR unless some bytes are modified near the image start
    $(_FILES)\fileset.exe $(_TGT)\xldrnand.nb0 0x4 0x3c 0xff
!if "$(WINCEREL)"=="1"
    xcopy /Q /I $(_TGT)\xldrnand.* $(_FLATRELEASEDIR)
!endif
!endif
