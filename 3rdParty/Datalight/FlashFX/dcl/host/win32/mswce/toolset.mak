#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	the ToolSet specific make settings used
#	throughout the project.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: toolset.mak $
#	Revision 1.4  2011/01/27 17:16:24Z  johnb
#	Added Windws Embedded Compact 7 support
#	Revision 1.3  2009/11/16 07:00:53Z  garyp
#	Moved the B_LOCALCFLAGS and B_CFLAGS settings so they are last
#	so that local and/or project settings can override the default settings.
#	Revision 1.2  2009/02/08 05:03:58Z  garyp
#	Merged from the v4.0 branch.  Modified the UNDER_CE symbol to be
#	defined to _WINCEOSVER.  Updated RC to use the same include paths
#	as the C compiler.
#	Revision 1.1  2008/06/20 03:29:18Z  garyp
#	Initial revision
#	Revision 1.4.1.2  2008/05/22 18:05:42Z  garyp
#	Changed /Oxs to just /Ox.
#	Revision 1.4  2007/03/01 21:17:23Z  Garyp
#	Added CE 6.0 support.  Removed CE 3.0 support.
#	Revision 1.3  2006/12/08 02:32:50Z  Garyp
#	Removed dead code.  Removed "CPUNUM" support.
#	Revision 1.2  2006/09/20 00:08:19Z  Garyp
#	Modified to accommodate build numbers which are now strings rather than
#	numbers.
#	Revision 1.1  2005/10/03 15:54:18Z  Pauli
#	Initial revision
#	Revision 1.2  2005/08/21 09:23:11Z  garyp
#	Re-abstracted so that the general include paths and general defines are
#	set in dcl.mak.  Enhanced so that multiple include paths are allowed in
#	the same variable.
#	Revision 1.1  2005/07/26 02:34:30Z  pauli
#	Initial revision
#----------------------------------------------------------------------------

ifndef TOOLSET_MAK_INCLUDED
TOOLSET_MAK_INCLUDED = 1

#--------------------------------------------------------------------
#	Reality	Checks
#--------------------------------------------------------------------


#--------------------------------------------------------------------
#	Basic Settings
#--------------------------------------------------------------------

# The ToolSet number must be unique for each ToolSet and must
# match	the values found in dltlset.h and dltlset.inc.

B_TOOLSETNUM=2402

ifeq ($(P_CPUFAMILY), arm)
B_ASMINCCMD=-I #			Assembler switch for include path
#B_ASMDEFCMD=/D#			Assembler switch to define a symbol
else
B_ASMINCCMD=/I#				Assembler switch for include path
B_ASMDEFCMD=/D#				Assembler switch to define a symbol
endif
B_CINCCMD=/I#				C compiler switch for include path
B_CDEFCMD=/D#				C compiler switch to define a symbol

TOOLSETINIT : GENERIC_DEFINES GENERIC_INCLUDES


#--------------------------------------------------------------------
#	Assembler Settings
#--------------------------------------------------------------------

ifeq ($(P_CPUFAMILY), arm)

B_STDAFLAGS  = -PD "D_PRODUCTNUM SETA $(P_PRODUCTNUM)"
B_STDAFLAGS += -PD "D_PRODUCTVER SETA $(P_VERSIONVAL)"
ifdef P_SUBPRODUCTNUM
B_STDAFLAGS += -PD "D_SUBPRODUCTNUM SETA $(P_SUBPRODUCTNUM)"
endif
B_STDAFLAGS += -PD "D_TOOLSETNUM SETA $(B_TOOLSETNUM)"
B_STDAFLAGS += -PD "D_DEBUG SETA $(B_DEBUG)"
ifdef B_AOPTION
B_STDAFLAGS += $(B_AOPTION)
endif
B_STDAFLAGS += $(B_ASMDEFS)
B_STDAFLAGS += $(B_ASMINCS)

else

B_STDAFLAGS = /c /coff /nologo $(B_AOPTION)
ifneq ($(B_DEBUG),0)
B_STDAFLAGS += /Zi
endif
B_STDAFLAGS += $(B_ASMDEFS)
B_STDAFLAGS += $(B_ASMINCS)

endif


#--------------------------------------------------------------------
#	C Compiler Settings
#
#	Note that B_LOCALCFLAGS and B_CFLAGS are last so that local
#	and/or project settings can override the default settings.
#	This requires that warning D9025 is ignored.
#--------------------------------------------------------------------

B_STDCFLAGS  = /c /W3 /WX /GF
B_STDCFLAGS += $(B_CECPUC)
B_STDCFLAGS += /DUNICODE /D_UNICODE /DSTRICT
B_STDCFLAGS += /DUNDER_CE=$(_WINCEOSVER) /DWINCEOEM /DWIN32
B_STDCFLAGS += /D_WIN32_WCE=$(_WINCEOSVER) /DWINCEINTERNAL
B_STDCFLAGS += /DINTERNATIONAL /DINTLMSG_CODEPAGE=1252
B_STDCFLAGS += /DL0409
ifeq ($(P_OSVER),6.00)
B_STDCFLAGS += -D "_CRT_SECURE_NO_DEPRECATE"
endif
ifeq ($(P_OSVER),7.00)
B_STDCFLAGS += -D "_CRT_SECURE_NO_DEPRECATE"
endif
ifeq ($(B_DEBUG),0)
B_STDCFLAGS += /Ox /Gy
else
B_STDCFLAGS += /Od /Zi /DDEBUG=1
endif
B_STDCFLAGS += $(B_CDEFS)
B_STDCFLAGS += $(B_CINCS)
B_STDCFLAGS +=  /nologo $(B_LOCALCFLAGS) $(B_CFLAGS)


#--------------------------------------------------------------------
#	Resource Compiler Settings
#
#	The resource compiler uses the same syntax for specifying
#	include search paths as the C compiler, so just use the 
#	same symbol.
#--------------------------------------------------------------------

B_STDRCFLAGS=$(B_CINCS) /i$(P_TOOLROOT)\public\common\oak\inc


#--------------------------------------------------------------------
#	Linker Settings
#--------------------------------------------------------------------

ifeq ($(B_DEBUG),0)
B_LDEBUG =
else
B_LDEBUG = /DEBUG /debugtype:cv
endif

#B_LOPTION =

B_LFLAGS = $(B_LOPTION)	$(B_LDEBUG) $(B_CELFLAGS) /MACHINE:$(B_CECPUL) /SUBSYSTEM:WINDOWSCE,$(P_OSVER) /NODEFAULTLIB

# If the linker	requires an appended symbol when specifying
# additional libraries on subsequent lines, define it here.

B_LIBADDCMD =


#--------------------------------------------------------------------
#	Default	Rules for this ToolSet
#
#	".OBD" is used for DEBUG object code
#	".OBR" is used for RELEASE object code
#--------------------------------------------------------------------

%.obj :	%.c
	$(B_CC)	$(B_STDCFLAGS) $*.c 2>&1

%.obd :	%.c
	@if exist $*.obd del $*.obd
	$(B_CC)	$(B_STDCFLAGS) $*.c 2>&1
	@if exist $*.obj ren $*.obj $*.obd

%.obr :	%.c
	@if exist $*.obr del $*.obr
	$(B_CC)	$(B_STDCFLAGS) $*.c 2>&1
	@if exist $*.obj ren $*.obj $*.obr

%.obj :	%.asm
	$(B_ASM) $(B_STDAFLAGS) $*.asm 2>&1

%.obd :	%.asm
	@if exist $*.obd del $*.obd
	$(B_ASM) $(B_STDAFLAGS) $*.asm 2>&1
	@if exist $*.obj ren $*.obj $*.obd

%.obr :	%.asm
	@if exist $*.obr del $*.obr
	$(B_ASM) $(B_STDAFLAGS) $*.asm 2>&1
	@if exist $*.obj ren $*.obj $*.obr

%.obj :	%.s
	$(B_ASM) $(B_STDAFLAGS) $*.s 2>&1

%.obd :	%.s
	@if exist $*.obd del $*.obd
	$(B_ASM) $(B_STDAFLAGS) $*.s 2>&1
	@if exist $*.obj ren $*.obj $*.obd

%.obr :	%.s
	@if exist $*.obr del $*.obr
	$(B_ASM) $(B_STDAFLAGS) $*.s 2>&1
	@if exist $*.obj ren $*.obj $*.obr

%.res :	%.rc
	@if exist $*.res del $*.res
	$(B_RC) $(B_STDRCFLAGS) $*.rc



endif	# TOOLSET_MAK_INCLUDED

