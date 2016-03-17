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

ResFile:
	copy $(_OBJDIR)\$(RCFILE).res $(_RELEASELIBDIR)\$(TARGETNAME).res
	copy $(_OBJDIR)\$(RCFILE).res $(_SYSGENOAKROOT)\target\$(_CPUINDPATH)\0409\$(RCFILE).res

