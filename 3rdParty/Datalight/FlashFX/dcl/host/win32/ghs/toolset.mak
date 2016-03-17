#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	the ToolSet specific make settings used
#	throughout the project.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: toolset.mak $
#	Revision 1.3  2009/02/07 23:25:14Z  garyp
#	Replaced the B_COPTION flag with B_CFLAGS and B_LOCALCFLAGS.
#	Revision 1.2  2006/12/06 23:55:34Z  Garyp
#	Removed dead code.
#	Revision 1.1  2006/11/02 19:47:08Z  Garyp
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

B_TOOLSETNUM=6360

B_ARESNAME=a_flags$(B_DEBUG).rsp
B_CRESNAME=c_flags$(B_DEBUG).rsp
B_ASMINCCMD=-I #			Assembler switch for include path
#B_ASMDEFCMD=-D#			Assembler switch to define a symbol
B_CINCCMD=-I#				C compiler switch for include path
B_CDEFCMD=-D#				C compiler switch to define a symbol

TOOLSETINIT : ARESFILES	CRESFILES GENERIC_DEFINES GENERIC_INCLUDES


#--------------------------------------------------------------------
#	Assembler Settings
#--------------------------------------------------------------------

ARESFILES :


#--------------------------------------------------------------------
#	C Compiler Settings
#--------------------------------------------------------------------

CRESFILES :
	@echo -c -g $(B_CFLAGS) $(B_LOCALCFLAGS) $(B_END) $(B_GHSCPU)	 >$(B_CRESNAME)
	@echo --diag_remark 188						>>$(B_CRESNAME)


#--------------------------------------------------------------------
#	Linker Settings
#--------------------------------------------------------------------

B_LDEBUG=

#B_LOPTION=

B_LFLAGS=$(B_LOPTION) $(B_LDEBUG)

# If the linker	requires an appended symbol when specifying
# additional libraries on subsequent lines, define it here.

B_LIBADDCMD=


#--------------------------------------------------------------------
#	Default	Rules for this ToolSet
#--------------------------------------------------------------------

# ".OBD" is used for DEBUG object code
# ".OBR" is used for RELEASE object code

%.obj :	%.c
	@if exist $*.err del $*.err
	$(B_CC)	$*.c -stderr=$*.err -o $*.obj @$(B_CRESNAME)

%.obr :	%.c
	@if exist $*.err del $*.err
	$(B_CC)	$*.c -stderr=$*.err -o $*.obr @$(B_CRESNAME)

%.obd :	%.c
	@if exist $*.err del $*.err
	$(B_CC)	$*.c -stderr=$*.err -o $*.obd @$(B_CRESNAME)

%.obj :	%.asm
	@if exist $*.err del $*.err
	$(B_ASM) $*.asm	-stderr=$*.err -o $*.obj @$(B_ARESNAME)

%.obr :	%.asm
	@if exist $*.err del $*.err
	$(B_ASM) $*.asm	-stderr=$*.err -o $*.obr @$(B_ARESNAME)

%.obd :	%.asm
	@if exist $*.err del $*.err
	$(B_ASM) $*.asm	-stderr=$*.err -o $*.obd @$(B_ARESNAME)

%.obj :	%.s
	@if exist $*.err del $*.err
	$(B_ASM) $*.s -stderr=$*.err -o	$*.obj @$(B_ARESNAME)

%.obr :	%.s
	@if exist $*.err del $*.err
	$(B_ASM) $*.s -stderr=$*.err -o	$*.obr @$(B_ARESNAME)

%.obd :	%.s
	@if exist $*.err del $*.err
	$(B_ASM) $*.s -stderr=$*.err -o	$*.obd @$(B_ARESNAME)



endif	# TOOLSET_MAK_INCLUDED

