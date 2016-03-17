#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	the ToolSet specific make settings used
#	throughout the project.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: toolset.mak $
#	Revision 1.5  2009/02/07 23:26:55Z  garyp
#	Replaced the B_COPTION flag with B_CFLAGS and B_LOCALCFLAGS.
#	Revision 1.4  2008/02/08 23:59:19Z  jeremys
#	Enabled PDB file generation for release builds.
#	Revision 1.3  2006/12/06 23:55:32Z  Garyp
#	Removed dead code.
#	Revision 1.2  2006/03/08 22:30:55Z  joshuab
#	Update resource include path.
#	Revision 1.1  2005/08/21 08:15:06Z  Pauli
#	Initial revision
#	Revision 1.2  2005/08/21 09:15:05Z  garyp
#	Re-abstracted so that the general include paths and general defines are
#	set in dcl.mak.  Enhanced so that multiple include paths are allowed in
#	the same variable.
#	Revision 1.1  2005/06/16 19:35:50Z  pauli
#	Initial revision
#	Revision 1.3  2005/06/16 19:35:50Z  PaulI
#	Merged with latest revision from XP variant branch.
#	Revision 1.2  2005/06/12 00:05:04Z  PaulI
#	Added support for product and subproduct numbers.
#	Fixed to pull the libraries from the right location.
#	Revision 1.1  2005/03/31 21:59:50Z  garyp
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

B_TOOLSETNUM=2700

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
	@echo /c /coff /nologo $(B_AOPTION)		 >$(B_ARESNAME)
ifeq ($(B_DEBUG), 0)
	@echo /Zi					>>$(B_ARESNAME)
endif


#--------------------------------------------------------------------
#	C Compiler Settings
#--------------------------------------------------------------------

B_CRESCMD=@$(B_CRESNAME)

CRESFILES :
	@echo -c -nologo -W3 -WX -FD $(B_CFLAGS) $(B_LOCALCFLAGS)  >$(B_CRESNAME)
	@echo -Gi- -Gm- -GR- -GX- -GB -GF -Gy -Gz		>>$(B_CRESNAME)
	@echo -D_X86_=1 -Di386=1 -DNT_UP=1 -DWINVER=0x0500	>>$(B_CRESNAME)
	@echo -DWIN32=100 -DWINNT=1 -D_WIN32_WINNT=0x0500	>>$(B_CRESNAME)
	@echo -DWIN32_LEAN_AND_MEAN=1 -DDEVL=1 -D_DLL=1 	>>$(B_CRESNAME)
	@echo -Z7 -Zel -Zp8 -cbstring -QIfdiv- -QIf -QI0f 	>>$(B_CRESNAME)
	@echo -D "_UNICODE" -D "UNICODE" 			>>$(B_CRESNAME)
ifeq ($(B_DEBUG),0)
	@echo -D "NDEBUG" -MT -O2				>>$(B_CRESNAME)
else
	@echo -D "_DEBUG" -MTd -Od 				>>$(B_CRESNAME)
endif

# Stuff from a generic MSVC6 compilation of a device driver
#	-Ic:\ntddk\private\inc -Ii386\ -I. -I..\inc -I..\..\..\..\inc
#	-Ic:\ntddk\inc\ddk\wdm -DCONDITION_HANDLING=1 -D_WIN32_IE=0x0501
#	-D_NT1X_=100 -DNT_INST=0 -DFPO=1 -Oxs -Oy -FIc:\ntddk\inc\warning.h
#	-GZ -DSTD_CALL


#--------------------------------------------------------------------
#	Resource Compiler Settings
#--------------------------------------------------------------------

# NTDDK versus IFSKIT have different requirements here...
B_RESINCS=-I$(P_ROOT)\include -I$(P_ROOT)\dcl\include -I$(P_PROJDIR) -I$(B_MFCINC) -I$(B_WININC)
#B_RESINCS=-I$(P_ROOT)\include


#--------------------------------------------------------------------
#	Linker Settings
#--------------------------------------------------------------------

#B_LOPTION =

B_LFLAGS = /subsystem:console /machine:I386

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
	@echo -debug					  >>$(B_LRESNAME)
ifneq ($(B_DEBUG),0)
	@echo -debug:FULL				  >>$(B_LRESNAME)
	@echo -debugtype:cv				  >>$(B_LRESNAME)
endif
	@echo -version:5.00				  >>$(B_LRESNAME)
	@echo -osversion:5.00				  >>$(B_LRESNAME)
#	@echo -optidata					  >>$(B_LRESNAME)
	@echo -driver					  >>$(B_LRESNAME)
	@echo -align:0x20				  >>$(B_LRESNAME)
	@echo -subsystem:native,5.00			  >>$(B_LRESNAME)
	@echo -base:0x10000				  >>$(B_LRESNAME)
	@echo -entry:DriverEntry@8			  >>$(B_LRESNAME)
	@echo -out:$(LINKERTARGET)			  >>$(B_LRESNAME)
	@echo $(LINKEROBJ)				  >>$(B_LRESNAME)
	@echo $(B_OSLIBPATH)\ntoskrnl.lib		  >>$(B_LRESNAME)
	@echo $(B_OSLIBPATH)\hal.lib		 	  >>$(B_LRESNAME)
	@echo $(B_OSLIBPATH)\libc.lib		  	  >>$(B_LRESNAME)
#	@echo $(B_OSLIBPATH)\wdm.lib		  	  >>$(B_LRESNAME)
#	@echo $(B_OSLIBPATH)\kernel32.lib		  >>$(B_LRESNAME)
#	@echo $(B_OSLIBPATH)\wmilib.lib		  	  >>$(B_LRESNAME)
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
	@if exist $*.obd del $*.obd
	$(B_CC)	$(B_CRESCMD) $*.c
	@if exist $*.obj ren $*.obj $*.obd

%.obr :	%.c
	@if exist $*.obr del $*.obr
	$(B_CC)	$(B_CRESCMD) $*.c
	@if exist $*.obj ren $*.obj $*.obr

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

%.res :	%.rc
	@if exist $*.res del $*.res
	$(B_RC) $(B_RESINCS) $*.rc



endif	# TOOLSET_MAK_INCLUDED
