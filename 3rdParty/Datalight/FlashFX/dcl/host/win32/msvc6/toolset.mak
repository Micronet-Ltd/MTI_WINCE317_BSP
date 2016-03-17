#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	the ToolSet specific make settings used
#	throughout the project.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: toolset.mak $
#	Revision 1.8  2009/02/07 23:26:11Z  garyp
#	Replaced the B_COPTION flag with B_CFLAGS and B_LOCALCFLAGS.
#	Revision 1.7  2007/02/12 23:18:34Z  timothyj
#	Added -D_CRT_NONSTDC_NO_DEPRECATE to support
#	portable code using the C standard library with VC++ 2005
#	Express Edition
#	Revision 1.6  2006/12/06 23:55:36Z  Garyp
#	Removed dead code.
#	Revision 1.5  2006/11/04 21:22:12Z  Garyp
#	Added support for MSVS8.
#	Revision 1.4  2006/09/16 20:48:59Z  Garyp
#	Eliminated the use of some temporary files.
#	Revision 1.3  2006/03/09 00:50:33Z  Garyp
#	Added an extra debug switch.
#	Revision 1.2  2005/12/19 18:38:30Z  pauli
#	Updated command line options to use consistent switches.
#	Revision 1.1  2005/12/01 02:12:12Z  Pauli
#	Initial revision
#	Revision 1.2  2005/12/01 02:12:12Z  Pauli
#	Merge with 2.0 product line, build 173.
#	Revision 1.1  2005/08/21 09:15:06Z  Garyp
#	Initial revision
#	Revision 1.2  2005/08/21 09:15:06Z  garyp
#	Re-abstracted so that the general include paths and general defines are
#	set in dcl.mak.  Enhanced so that multiple include paths are allowed in
#	the same variable.
#	Revision 1.1  2005/06/12 00:05:06Z  pauli
#	Initial revision
#	Revision 1.6  2005/07/18 01:32:41Z  jeremys
#	Enabled conditional debugging symbols in compiler and linker.
#	Revision 1.5  2005/06/12 00:05:06Z  PaulI
#	Added support for product and subproduct numbers.
#	Revision 1.4  2005/03/31 21:06:51Z  PaulI
#	XP Merge
#	Revision 1.3.1.5  2005/03/31 21:06:51Z  garyp
#	Removed the WINDDK specific stuff -- now in a separate ToolSet.
#	Revision 1.3.1.4  2005/03/31 07:34:50Z  garyp
#	Updated include path handling.
#	Revision 1.3.1.3  2005/03/31 06:32:22Z  garyp
#	Added linker response file support.
#	Revision 1.3.1.2  2005/03/31 04:43:13Z  garyp
#	Added support for building NTDDK device drivers.
#	Revision 1.3  2005/03/14 18:31:52Z  PaulI
#	Made include path switches consistant.
#	Revision 1.2  2005/01/26 01:56:36Z  GaryP
#	Documentation update.
#	Revision 1.1  2005/01/16 20:25:04Z  GaryP
#	Initial revision
#	Revision 1.2  2004/10/05 16:33:22Z  PaulI
#	Added CLIBINC path to CFlags.  Put OS and CLIB include paths before
#	tool include path.  This was necessary for the Nucleus port to build
#	correctly because Nucleus has specific implementations of some standard
#	C functions.
#	Revision 1.1  2004/07/09 21:02:00Z  brandont
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

B_TOOLSETNUM=2600

B_ARESNAME=a_flags$(B_DEBUG).rsp
B_CRESNAME=c_flags$(B_DEBUG).rsp
B_ASMINCCMD=/I#				Assembler switch for include path
B_ASMDEFCMD=/D#				Assembler switch to define a symbol
B_CINCCMD=-I#				C compiler switch for include path
B_CDEFCMD=-D #				C compiler switch to define a symbol

TOOLSETINIT : ARESFILES	CRESFILES LRESFILES GENERIC_DEFINES GENERIC_INCLUDES


#--------------------------------------------------------------------
#	Assembler Settings
#--------------------------------------------------------------------

B_ARESCMD=@$(B_ARESNAME)

ARESFILES :
	@echo /c /coff /nologo $(B_AOPTION)			 >$(B_ARESNAME)
ifeq ($(B_DEBUG), 0)
	@echo /Zi						>>$(B_ARESNAME)
endif


#--------------------------------------------------------------------
#	C Compiler Settings
#--------------------------------------------------------------------

B_CRESCMD=@$(B_CRESNAME)

CRESFILES :
	@echo -c -nologo -W3 -WX -FD $(B_CFLAGS) $(B_LOCALCFLAGS)  >$(B_CRESNAME)
	@echo -Gm- -GR- -GF -Gy					>>$(B_CRESNAME)
ifeq ($(B_TOOLVER),VS6)
	@echo -Gi- -GX- -GB 					>>$(B_CRESNAME)
else
	@echo -EHs-c-	 					>>$(B_CRESNAME)
	@echo -D "_CRT_SECURE_NO_DEPRECATE"            		>>$(B_CRESNAME)
	@echo -D "_CRT_NONSTDC_NO_DEPRECATE"           		>>$(B_CRESNAME)
endif
	@echo -D "WIN32" -D "_CONSOLE"              		>>$(B_CRESNAME)
	@echo -D "_UNICODE" -D "UNICODE"			>>$(B_CRESNAME)
ifeq ($(B_DEBUG),0)
	@echo -D "NDEBUG" -MT -O2				>>$(B_CRESNAME)
else
	@echo -D "_DEBUG" -MTd -Od -Zi 				>>$(B_CRESNAME)
ifeq ($(B_TOOLVER),VS6)
	@echo -GZ -Yd -Ge					>>$(B_CRESNAME)
else
	@echo -RTC1						>>$(B_CRESNAME)
endif
endif


#--------------------------------------------------------------------
#	Linker Settings
#--------------------------------------------------------------------

#B_LOPTION =

ifeq ($(B_DEBUG),0)
B_LFLAGS = /subsystem:console /machine:I386
else
B_LFLAGS = /subsystem:console /machine:I386 /DEBUG
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
#	@echo -SECTION:INIT,d				  >>$(B_LRESNAME)
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
	@echo c:\ntddk\libchk\i386\ntoskrnl.lib		  >>$(B_LRESNAME)
	@echo c:\ntddk\libchk\i386\hal.lib		  >>$(B_LRESNAME)
	@echo c:\ntddk\libchk\i386\libc.lib		  >>$(B_LRESNAME)
#	@echo c:\ntddk\libchk\i386\wdm.lib		  >>$(B_LRESNAME)
#	@echo c:\ntddk\libchk\i386\kernel32.lib		  >>$(B_LRESNAME)
#	@echo c:\ntddk\libchk\i386\wmilib.lib		  >>$(B_LRESNAME)
endif
endif


#--------------------------------------------------------------------
#	Default	Rules for this ToolSet
#--------------------------------------------------------------------

# ".OBD" is used for DEBUG object code
# ".OBR" is used for RELEASE object code


%.obj :	%.c
	$(B_CC)	$(B_CRESCMD) $*.c

%.obd :	%.c
	$(B_CC) /Fo$*.obd $(B_CRESCMD) $*.c

%.obr :	%.c
	$(B_CC) /Fo$*.obr $(B_CRESCMD) $*.c

%.obj :	%.asm
	$(B_ASM) $(B_ARESCMD) $*.asm

%.obd :	%.asm
	@if exist $*.obd del $*.obd
	$(B_ASM) $(B_ARESCMD) $*.asm
	@if exist $*.obj ren $*.obj $*.obd

%.obr :	%.asm
	@if exist $*.obr del $*.obr
	$(B_ASM) $(B_ARESCMD) $*.asm
	@if exist $*.obj ren $*.obj $*.obr

%.obj :	%.s
	$(B_ASM) $(B_ARESCMD) $*.s

%.obd :	%.s
	@if exist $*.obd del $*.obd
	$(B_ASM) $(B_ARESCMD) $*.s
	@if exist $*.obj ren $*.obj $*.obd

%.obr :	%.s
	@if exist $*.obr del $*.obr
	$(B_ASM) $(B_ARESCMD) $*.s
	@if exist $*.obj ren $*.obj $*.obr



endif	# TOOLSET_MAK_INCLUDED
