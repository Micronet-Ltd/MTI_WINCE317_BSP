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

P_ROOT=$(_WINCEROOT)\3rdParty\Datalight\flashfx
P_PROJDIR=$(P_ROOT)\projects\beagleboard
!IF "$(BSP_DEBUG_SERIAL)"=="1"
FFX_RM=1
!ELSE
FFX_RM=0
!ENDIF

flashfx_build:
	b.bat /nopause $(FFX_RM)