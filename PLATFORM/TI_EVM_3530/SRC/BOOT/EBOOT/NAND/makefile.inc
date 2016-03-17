!if 0
;================================================================================
;             Texas Instruments OMAP(TM) Platform Software
; (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
;
; Use of this software is controlled by the terms and conditions found
; in the license agreement under which this software has been supplied.
;
;================================================================================
!endif

_FILES=$(_PLATFORMROOT)\$(_TGTPLAT)\files
_TGT=$(_RAWFILETGT)\$(WINCEDEBUG)

BootImage:
!IF "$(NOLINK)" == ""
    romimage $(ROMIMAGE_FLAGS) ebootnand.bib
!IF "$(WINCEREL)"=="1"
    xcopy /Q /I $(_TGT)\ebootnand.bin $(_FLATRELEASEDIR)
    xcopy /Q /I $(_TGT)\ebootnand.nb0 $(_FLATRELEASEDIR)
    copy $(_FLATRELEASEDIR)\ebootnand.nb0 $(_FLATRELEASEDIR)\ebootnand.raw
	copy $(_FLATRELEASEDIR)\ebootnand.bin $(_FLATRELEASEDIR)\ebnand.bin
!ENDIF
!ENDIF
