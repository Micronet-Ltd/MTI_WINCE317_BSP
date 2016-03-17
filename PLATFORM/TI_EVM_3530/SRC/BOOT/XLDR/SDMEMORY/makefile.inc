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
# Create MLO (sd card booting file)
#
# Boot ROM requires xldr to be preceeded with two words containing image 
# size and destination address.
# 

_FILES=$(_PLATFORMROOT)\$(_TGTPLAT)\files
_SRC=$(_PLATFORMROOT)\$(_TGTPLAT)\src\boot\xldr\sdmemory
_TGT=$(_RAWFILETGT)\$(WINCEDEBUG)
_BIN=$(_PLATFORMROOT)\$(_TGTPLAT)\bin

xldr_sd_mlo:
!if "$(NOLINK)" == ""
    romimage $(ROMIMAGE_FLAGS) xldrsd.bib
!if "$(WINCEREL)"=="1"
    xcopy /Q /I $(_TGT)\xldrsd.* $(_FLATRELEASEDIR)
!endif
!endif

!if "$(NOLINK)" == ""
    copy /b $(_SRC)\xldrtocsd.raw + $(_TGT)\xldrsd.nb0 $(_TGT)\MLO
!if "$(WINCEREL)"=="1"
    xcopy /Q /I $(_TGT)\MLO $(_FLATRELEASEDIR)
!endif
!endif        
