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

FFX_ROOT=$(_WINCEROOT)\3rdParty\Datalight\flashfx
FFX_PROJDIR=$(FFX_ROOT)\projects

ffx_build:
	ffxb.bat $(FFX_PROJDIR)