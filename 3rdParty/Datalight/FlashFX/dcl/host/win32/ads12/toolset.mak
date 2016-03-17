#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	the ToolSet specific make settings used
#	throughout the project.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: toolset.mak $
#	Revision 1.3  2009/02/07 23:24:22Z  garyp
#	Replaced the B_COPTION flag with B_CFLAGS and B_LOCALCFLAGS.
#	Revision 1.2  2006/12/08 02:26:45Z  Garyp
#	Removed dead code.  Removed "CPUNUM" support.
#	Revision 1.1  2005/08/21 08:15:06Z  Pauli
#	Initial revision
#	Revision 1.3  2005/08/21 09:15:05Z  garyp
#	Re-abstracted so that the general include paths and general defines are
#	set in dcl.mak.  Enhanced so that multiple include paths are allowed in
#	the same variable.
#	Revision 1.2  2005/08/16 23:27:41Z  pauli
#	Added -dwarf2 to debug builds.
#	Revision 1.1  2005/07/15 06:31:54Z  pauli
#	Initial revision
#	Revision 1.3  2005/06/12 00:05:06Z  PaulI
#	Added support for product and subproduct numbers.
#	Revision 1.2  2005/01/26 01:56:38Z  GaryP
#	Documentation update.
#	Revision 1.1  2005/01/16 20:41:36Z  GaryP
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

B_TOOLSETNUM=3120

B_ARESNAME=a_flags$(B_DEBUG).rsp
B_CRESNAME=c_flags$(B_DEBUG).rsp
B_ASMINCCMD=-I #			Assembler switch for include path
B_CINCCMD=-I #				C compiler switch for include path
B_CDEFCMD=-D #				C compiler switch to define a symbol

TOOLSETINIT : ARESFILES	CRESFILES GENERIC_DEFINES GENERIC_INCLUDES


#--------------------------------------------------------------------
#	Assembler Settings
#--------------------------------------------------------------------

ARESFILES :
	@echo -g $(B_AOPTION)				    	 >$(B_ARESNAME)
	@echo -cpu XScale				    	>>$(B_ARESNAME)
	@echo -PD "D_TOOLSETNUM SETA $(B_TOOLSETNUM)"	    	>>$(B_ARESNAME)
	@echo -PD "D_DEBUG SETA $(B_DEBUG)"		    	>>$(B_ARESNAME)


#--------------------------------------------------------------------
#	C Compiler Settings
#--------------------------------------------------------------------

CRESFILES :
	@echo -c -j- $(B_CFLAGS) $(B_LOCALCFLAGS) $(B_END) $(B_ADSCPU)	 >$(B_CRESNAME)
ifneq ($(B_DEBUG), 0)
	@echo -g -dwarf2			        	>>$(B_CRESNAME)
endif


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
	$(B_CC)	$*.c -errors $*.err -o $*.obj -via $(B_CRESNAME)

%.obr :	%.c
	$(B_CC)	$*.c -errors $*.err -o $*.obr -via $(B_CRESNAME)

%.obd :	%.c
	$(B_CC)	$*.c -errors $*.err -o $*.obd -via $(B_CRESNAME)

%.obj :	%.asm
	$(B_ASM) $*.asm	-e $*.err -o $*.obj -via $(B_ARESNAME)

%.obr :	%.asm
	$(B_ASM) $*.asm	-e $*.err -o $*.obr -via $(B_ARESNAME)

%.obd :	%.asm
	$(B_ASM) $*.asm	-e $*.err -o $*.obd -via $(B_ARESNAME)

%.obj :	%.s
	$(B_ASM) $*.s -e $*.err	-o $*.obj -via $(B_ARESNAME)

%.obr :	%.s
	$(B_ASM) $*.s -e $*.err	-o $*.obr -via $(B_ARESNAME)

%.obd :	%.s
	$(B_ASM) $*.s -e $*.err	-o $*.obd -via $(B_ARESNAME)


endif	# TOOLSET_MAK_INCLUDED

