#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	the ToolSet specific make settings used
#	throughout the project.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: toolset.mak $
#	Revision 1.6  2009/09/17 17:45:42Z  jimmb
#	Added  $(B_DIABOPTS3) into the debug build
#	Revision 1.5  2009/09/16 21:33:03Z  jimmb
#	Removed optimization from the c_flags when building a DEBUG version.
#	Revision 1.4  2009/08/20 21:38:01Z  jimmb
#	Updated VxWorks build files to support VxWorks 6.8
#	Revision 1.3  2009/03/30 17:46:35Z  jimmb
#	Updated the tools to suppot the Arm V6 processor and support
#	VxWorks 6.7 VSB builds.
#	Revision 1.2  2009/02/07 23:25:07Z  garyp
#	Replaced the B_COPTION flag with B_CFLAGS and B_LOCALCFLAGS.
#	Revision 1.1  2005/08/30 06:14:00Z  Pauli
#	Initial revision
#	Revision 1.1  2005/08/30 07:14:00Z  Garyp
#	Initial revision
#	Revision 1.3  2005/08/30 07:14:00Z  Garyp
#	Removed a spurious endif.
#	Revision 1.2  2005/08/21 09:15:07Z  garyp
#	Re-abstracted so that the general include paths and general defines are
#	set in dcl.mak.  Enhanced so that multiple include paths are allowed in
#	the same variable.
#	Revision 1.1  2005/06/12 00:05:06Z  pauli
#	Initial revision
#	Revision 1.2  2005/06/12 00:05:06Z  PaulI
#	Added support for product and subproduct numbers.
#	Revision 1.1  2005/04/24 23:26:18Z  GaryP
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

B_TOOLSETNUM=4505

B_ARESNAME=a_flags$(B_DEBUG).rsp
B_CRESNAME=c_flags$(B_DEBUG).rsp
B_ASMINCCMD=-I #			Assembler switch for include path
B_ASMDEFCMD=-D#				Assembler switch to define a symbol
B_CINCCMD=-I #				C compiler switch for include path
B_CDEFCMD=-D #				C compiler switch to define a symbol

TOOLSETINIT : ARESFILES	CRESFILES GENERIC_DEFINES GENERIC_INCLUDES


#--------------------------------------------------------------------
#	Defines for using Vxworks Source build implemented
#	in VxWorks 6.7
#--------------------------------------------------------------------

WIND_BASE := $(subst \,/,$(WIND_BASE))

#--------------------------------------------------------------------
# 	The '-' allows the file to not be in previous versions 
#	WITHOUT generating an error on include
#	Passing an empty or invalid value in 6.7 will cause a fatal 
#	compile error and will be harmless in all pre-6.7 versions.
#--------------------------------------------------------------------

-include $(WIND_BASE)\target\h\make\defs.vsbvars.mk


#--------------------------------------------------------------------
#	 Assembler Settings
# Removed:@echo	/mx /w2	/w-ICG $(B_AOPTION)
#    >$(B_ARESNAME)
#--------------------------------------------------------------------

ARESFILES :
	@echo /mx /w2 $(B_AOPTION)		 	 >$(B_ARESNAME)


#--------------------------------------------------------------------
#	C Compiler Settings
#	@echo -E$(B_ASM)
#	>>$(B_CRESNAME)
#--------------------------------------------------------------------

CRESFILES :
	@echo -c $(B_CFLAGS) $(B_LOCALCFLAGS) $(B_DIABOPTS1)	 >$(B_CRESNAME)
	@echo $(B_DIABCPU) $(B_DIABOPTS2)			>>$(B_CRESNAME)
ifeq ($(B_DEBUG), 0)
	@echo -O $(B_DIABOPTS3)			        	>>$(B_CRESNAME)
endif
	@echo -D TOOL=diab -D TOOL_FAMILY=diab			>>$(B_CRESNAME)
	@echo -D _VSB_CONFIG_FILE="<$(VSB_CONFIG_FILE)>"	>>$(B_CRESNAME)
ifneq ($(B_DEBUG), 0)
	@echo -g $(B_DIABOPTS3)			        	>>$(B_CRESNAME)
endif


#--------------------------------------------------------------------
#	Linker Settings
#--------------------------------------------------------------------

ifeq ($(B_DEBUG), 0)
B_LDEBUG =
else
B_LDEBUG = /v
endif

#B_LOPTION =

B_LFLAGS = /s /e /l /m /L$(P_TOOLROOT)\$(B_TOOLLIB) $(B_LOPTION) $(B_LDEBUG)

# If the linker	requires an appended symbol when specifying
# additional libraries on subsequent lines, define it here.

B_LIBADDCMD = +


#--------------------------------------------------------------------
#	Default	Rules for this ToolSet
#--------------------------------------------------------------------

# ".OBD" is used for DEBUG object code
# ".OBR" is used for RELEASE object code


%.obj :	%.c
	$(B_CC)	-@$(B_CRESNAME)	$*.c

%.obd :	%.c
	$(B_CC)	-o$*.obd -@$(B_CRESNAME) $*.c

%.obr :	%.c
	$(B_CC)	-o$*.obr -@$(B_CRESNAME) $*.c

%.obj :	%.asm
	$(B_ASM) @$(B_ARESNAME)	$*.asm

%.obd :	%.asm
	$(B_ASM) @$(B_ARESNAME)	$*.asm,	$*.obd

%.obr :	%.asm
	$(B_ASM) @$(B_ARESNAME)	$*.asm,	$*.obr



endif	# TOOLSET_MAK_INCLUDED

