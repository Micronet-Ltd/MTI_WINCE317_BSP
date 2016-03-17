!if 0
Copyright (c) Microsoft Corporation.  All rights reserved.
!endif
!if 0
Use of this sample source code is subject to the terms of the Microsoft
license agreement under which you licensed this sample source code. If
you did not accept the terms of the license agreement, you are not
authorized to use this sample source code. For the terms of the license,
please see the license agreement between you and Microsoft or, if applicable,
see the LICENSE.RTF on your install media or the root of your tools installation.
THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
!endif

!if 0

_CPLMAIN_ALLCOMPONENTS=cplmain network comm datetime dialing keyboard password owner power system display backlight colorscheme pointercmn stylus mouse sounds sip remove certs accessib

#
# Recurse for each combination of (P, L) x (loc, NL)
#
resfiles:
	@set _LOC=
	@set _ASPECT=L
	@set _ASPECTFULL=landscape
	$(MAKE) /NOLOGO resfiles1
	@set _LOC=nl
	@set _ASPECT=L
	@set _ASPECTFULL=landscape
	$(MAKE) /NOLOGO resfiles1
	@set _LOC=
	@set _ASPECT=P
	@set _ASPECTFULL=portrait
	$(MAKE) /NOLOGO resfiles1
	@set _LOC=nl
	@set _ASPECT=P
	@set _ASPECTFULL=portrait
	$(MAKE) /NOLOGO resfiles1
	
resfiles1: $(_CPLMAIN_ALLCOMPONENTS)


#
# Recurse for each component
#
$(_CPLMAIN_ALLCOMPONENTS): 
	@set _COMPNAME=$@
	$(MAKE) /NOLOGO $(_RELEASELIBDIR)\$@$(_ASPECT)$(_LOC).res

$(_RELEASELIBDIR)\$(_COMPNAME)$(_ASPECT)$(_LOC).res: $(_OBJDIR)\$(_COMPNAME)$(_ASPECT)$(_LOC).res
	copy $? $@

$(_OBJDIR)\$(_COMPNAME)$(_ASPECT)$(_LOC).res: $(_OBJDIR)\$(_COMPNAME)$(_ASPECT)$(_LOC).rc
    $(RCOMPILER) $(RCFLAGS) -fo$@ $?

$(_OBJDIR)\$(_COMPNAME)$(_ASPECT)$(_LOC).rc: cplmain$(_LOC).rc cplres.h
	@set CE_MODULES=cplmain
	@set CPLMAIN_COMPONENTS=$(_ASPECTFULL) $(_COMPNAME)
	-@del $@ > nul 2>&1
	cefilter cplmain$(_LOC).rc $@ -N

!endif
