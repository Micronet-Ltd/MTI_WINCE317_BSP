#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	the ToolSet specific make settings used
#	throughout the project.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: toolset.mak $
#	Revision 1.4  2009/02/07 23:24:56Z  garyp
#	Replaced the B_COPTION flag with B_CFLAGS and B_LOCALCFLAGS.
#	Revision 1.3  2007/04/25 22:23:02Z  Garyp
#	Updated so that include paths are never quoted since this seems to be
#	unsupported in this version of the VxWorks tools (5.4 at least).
#	Revision 1.2  2006/08/13 21:31:34Z  Garyp
#	Removed a spurious "endif" statement.
#	Revision 1.1  2005/08/21 08:15:08Z  Pauli
#	Initial revision
#	Revision 1.2  2005/08/21 09:15:07Z  garyp
#	Re-abstracted so that the general include paths and general defines are
#	set in dcl.mak.  Enhanced so that multiple include paths are allowed in
#	the same variable.
#	Revision 1.1  2005/06/12 00:05:06Z  pauli
#	Initial revision
#	Revision 1.3  2005/06/12 00:05:06Z  PaulI
#	Added support for product and subproduct numbers.
#	Revision 1.2  2005/05/12 18:54:43Z  garyp
#	Fixed to work properly again.  Broke when the original DIAB ToolSet was
#	split into separate 5.x and 6.x ToolSets.
#	Revision 1.1  2005/04/24 23:34:26Z  GaryP
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

B_TOOLSETNUM=4500

# The VxWorks 5.x tools (or 5.4 at least) do not seem to support
# include paths which include quotes.  Defining this symbol prevents
# dcl.mak from using quotes.
B_NOQUOTEDINCPATHS=1

B_ARESNAME=a_flags$(B_DEBUG).rsp
B_CRESNAME=c_flags$(B_DEBUG).rsp
B_ASMINCCMD=-I #			Assembler switch for include path
B_ASMDEFCMD=-D#				Assembler switch to define a symbol
B_CINCCMD=-I #				C compiler switch for include path
B_CDEFCMD=-D #				C compiler switch to define a symbol

TOOLSETINIT : ARESFILES	CRESFILES GENERIC_DEFINES GENERIC_INCLUDES


#--------------------------------------------------------------------
#	 Assembler Settings
# Removed:@echo	/mx /w2	/w-ICG $(B_AOPTION)
#    >$(B_ARESNAME)
#--------------------------------------------------------------------

ARESFILES :
	@echo /mx /w2 $(B_AOPTION)		 		 >$(B_ARESNAME)


#--------------------------------------------------------------------
#	C Compiler Settings
#	@echo -E$(B_ASM)
#	>>$(B_CRESNAME)
#--------------------------------------------------------------------

# -@E+$(B_LOG) -@O+$(B_LOG)

CRESFILES :
	@echo -c $(B_CFLAGS) $(B_LOCALCFLAGS) $(B_DIABOPTS1)	 >$(B_CRESNAME)
	@echo $(B_DIABCPU) $(B_DIABOPTS2) $(B_DIABOPTS3)	>>$(B_CRESNAME)
	@echo -D TOOL=diab -D TOOL_FAMILY=diab			>>$(B_CRESNAME)
ifneq ($(B_DEBUG), 0)
	@echo -g				        	>>$(B_CRESNAME)
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

