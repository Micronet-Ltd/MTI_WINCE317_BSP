#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	the ToolSet specific make settings used
#	throughout the project.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: toolset.mak $
#	Revision 1.9  2009/11/16 06:47:04Z  garyp
#	Moved the B_LOCALCFLAGS and B_CFLAGS settings so they are last
#	so that local and/or project settings can override the default settings.
#	Revision 1.8  2009/04/15 20:34:29Z  keithg
#	Added support for specifing the endian order.  Note this is specifically
#	for use in simulation tests and using the endian order switches has no
#	effect on the native endian order.
#	Revision 1.7  2009/02/08 02:37:06Z  garyp
#	Merged from the v4.0 branch.  Added support for RC.  Fixed a polarity
#	problem with the A_FLAGS syntax.  Added the "_WIN32_WINNT=0x0500"
#	symbol, so the code can assume that Win2K or later is being used.
#	Replaced the B_COPTION flag with B_CFLAGS and B_LOCALCFLAGS.
#	Fixed to use B_ASMINCS and B_ASMDEFS.
#	Revision 1.6  2009/01/26 22:35:19Z  michaelm
#	Changed warning back to level 3 so our non-DCL tools will build.
#	Revision 1.5  2008/12/31 21:52:54Z  keithg
#	Changed warning level to 4.
#	Revision 1.4  2008/07/23 17:00:10Z  keithg
#	Added support for VS 9.0 and broadened CPU support (x64)
#	Revision 1.3  2008/05/06 22:41:58Z  keithg
#	Turned incremental linking off to avoid spurious link errors.
#	Revision 1.2  2007/10/24 00:03:15Z  brandont
#	Added the _CRT_NON_CONFORMING_SWPRINTFS command line switch.
#	Revision 1.1  2007/03/20 20:32:32Z  Garyp
#	Initial revision
#	Created from the MSVC6 ToolSet.
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

B_TOOLSETNUM=2620

B_ASMINCCMD=/I#				Assembler switch for include path
B_ASMDEFCMD=/D#				Assembler switch to define a symbol
B_CINCCMD=-I#				C compiler switch for include path
B_CDEFCMD=-D #				C compiler switch to define a symbol

TOOLSETINIT : LRESFILES GENERIC_DEFINES GENERIC_INCLUDES


#--------------------------------------------------------------------
#	Assembler Settings
#--------------------------------------------------------------------

B_AFLAGS  = /c /coff /nologo $(B_AOPTION)
ifneq ($(B_DEBUG), 0)
B_AFLAGS +=  /Zi
endif
B_AFLAGS += $(B_ASMDEFS)
B_AFLAGS += $(B_ASMINCS)


#--------------------------------------------------------------------
#	C Compiler Settings
#
#	Note we do not treat warnings as errors here, instead we
#	rely on the build process to report them.  This allows for
#	debugging with warnings in the code.  
#
#	Note that B_LOCALCFLAGS and B_CFLAGS are last so that local
#	and/or project settings can override the default settings.
#	This requires that warning D9025 is ignored.
#--------------------------------------------------------------------

B_STDCFLAGS  = -c -W3 -FD
B_STDCFLAGS += -Gm- -GR- -GF -Gy $(B_END)
ifeq ($(B_MSVS_VER),60)
B_STDCFLAGS += -Gi- -GX- -GB
else
B_STDCFLAGS += -EHs-c-
B_STDCFLAGS += -D "_CRT_SECURE_NO_DEPRECATE"
B_STDCFLAGS += -D "_CRT_NONSTDC_NO_DEPRECATE"
B_STDCFLAGS += -D "_CRT_NON_CONFORMING_SWPRINTFS"
endif
B_STDCFLAGS += -D "WIN32" -D "_CONSOLE" -D "_WIN32_WINNT=0x0500"
B_STDCFLAGS += -D "_UNICODE" -D "UNICODE"
ifeq ($(B_DEBUG),0)
B_STDCFLAGS += -D "NDEBUG" -MT -O2
else
B_STDCFLAGS += -D "_DEBUG" -MTd -Od -Zi
ifeq ($(B_MSVS_VER),60)
B_STDCFLAGS += -GZ -Yd -Ge
else
B_STDCFLAGS += -RTC1
endif
endif
B_STDCFLAGS += $(B_CDEFS)
B_STDCFLAGS += $(B_CINCS)
B_STDCFLAGS += -nologo $(B_LOCALCFLAGS) $(B_CFLAGS)


#--------------------------------------------------------------------
#	Resource Compiler Settings
#
#	The resource compiler uses the same syntax for specifying
#	include search paths as the C compiler, so just use the
#	same symbol.
#--------------------------------------------------------------------

B_RRESCMD=$(B_CINCS)


#--------------------------------------------------------------------
#	Linker Settings
#--------------------------------------------------------------------

#B_LOPTION =

ifeq ($(B_DEBUG),0)
B_LFLAGS = /subsystem:console /incremental:no
else
B_LFLAGS = /subsystem:console /incremental:no /DEBUG
endif

# If the linker	requires an appended symbol when specifying
# additional libraries on subsequent lines, define it here.

B_LIBADDCMD =

B_LRESNAME=l_flags$(B_DEBUG).rsp
B_LRESCMD=@$(B_LRESNAME)

LRESFILES :
ifdef LINKEROBJ
ifdef LINKERTARGET
	@echo -nologo $(B_LOPTION)			   >$(B_LRESNAME)
	@echo -STACK:262144,4096			  >>$(B_LRESNAME)
	@echo -MERGE:_PAGE=PAGE				  >>$(B_LRESNAME)
	@echo -MERGE:_TEXT=.text			  >>$(B_LRESNAME)
	@echo -OPT:REF					  >>$(B_LRESNAME)
	@echo -OPT:ICF					  >>$(B_LRESNAME)
	@echo -INCREMENTAL:NO				  >>$(B_LRESNAME)
	@echo -FULLBUILD				  >>$(B_LRESNAME)
	@echo -FORCE:MULTIPLE				  >>$(B_LRESNAME)
	@echo /release					  >>$(B_LRESNAME)
	@echo -NODEFAULTLIB				  >>$(B_LRESNAME)
ifneq ($(B_DEBUG),0)
	@echo -debug					  >>$(B_LRESNAME)
	@echo -debug:FULL				  >>$(B_LRESNAME)
	@echo -debugtype:cv				  >>$(B_LRESNAME)
endif
	@echo -version:5.00				  >>$(B_LRESNAME)
	@echo -osversion:5.00				  >>$(B_LRESNAME)
	@echo -optidata					  >>$(B_LRESNAME)
	@echo -driver					  >>$(B_LRESNAME)
	@echo -align:0x20				  >>$(B_LRESNAME)
	@echo -subsystem:native,5.00			  >>$(B_LRESNAME)
	@echo -base:0x10000				  >>$(B_LRESNAME)
	@echo -entry:DriverEntry@8			  >>$(B_LRESNAME)
	@echo -out:$(LINKERTARGET)			  >>$(B_LRESNAME)
	@echo $(LINKEROBJ)				  >>$(B_LRESNAME)
	@echo ntoskrnl.lib hal.lib libc.lib		  >>$(B_LRESNAME)
	@echo wdm.lib kernel32.lib wmilib.lib		  >>$(B_LRESNAME)
endif
endif


#--------------------------------------------------------------------
#	Default	Rules for this ToolSet
#
#	".OBD" is used for DEBUG object code
#	".OBR" is used for RELEASE object code
#--------------------------------------------------------------------

%.obj :	%.c
	$(B_CC)	$(B_STDCFLAGS) $*.c 2>&1

%.obd :	%.c
	$(B_CC) $(B_STDCFLAGS) /Fo$*.obd $*.c 2>&1

%.obr :	%.c
	$(B_CC) $(B_STDCFLAGS) /Fo$*.obr $*.c 2>&1

%.obj :	%.asm
	$(B_ASM) $(B_AFLAGS) $*.asm 2>&1

%.obd :	%.asm
	@if exist $*.obd del $*.obd
	$(B_ASM) $(B_AFLAGS) $*.asm 2>&1
	@if exist $*.obj ren $*.obj $*.obd

%.obr :	%.asm
	@if exist $*.obr del $*.obr
	$(B_ASM) $(B_AFLAGS) $*.asm 2>&1
	@if exist $*.obj ren $*.obj $*.obr

%.obj :	%.s
	$(B_ASM) $(B_AFLAGS) $*.s 2>&1

%.obd :	%.s
	@if exist $*.obd del $*.obd
	$(B_ASM) $(B_AFLAGS) $*.s 2>&1
	@if exist $*.obj ren $*.obj $*.obd

%.obr :	%.s
	@if exist $*.obr del $*.obr
	$(B_ASM) $(B_AFLAGS) $*.s 2>&1
	@if exist $*.obj ren $*.obj $*.obr

%.res :	%.rc
	@if exist $*.res del $*.res
	$(B_RC) $(B_RRESCMD) $*.rc




endif	# TOOLSET_MAK_INCLUDED
