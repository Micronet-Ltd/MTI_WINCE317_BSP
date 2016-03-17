#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	the ToolSet specific make settings used
#	throughout the project.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: toolset.mak $
#	Revision 1.8  2010/09/28 15:52:37Z  jimmb
#	Supressed Warning:  #186-D: pointless comparison of 
#	unsigned integer with zero
#	Revision 1.7  2010/09/13 17:50:36Z  jimmb
#	Updated the toolset to support Rvds version 4.0 and 4.1
#	Revision 1.6  2009/02/07 23:27:08Z  garyp
#	Replaced the B_COPTION flag with B_CFLAGS and B_LOCALCFLAGS.
#	Revision 1.5  2007/04/05 23:34:33Z  Garyp
#	Updated to ignore warning 3017.
#	Revision 1.4  2006/12/08 02:33:56Z  Garyp
#	Removed dead code.  Removed "CPUNUM" support.
#	Revision 1.3  2006/10/09 00:05:28Z  Garyp
#	Syntax fixes.
#	Revision 1.2  2006/10/08 21:18:21Z  Garyp
#	First working version of the RVDS30 ToolSet.
#	Revision 1.1  2006/10/07 00:59:26Z  Garyp
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

B_TOOLSETNUM=3300

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
	@echo -g $(B_AOPTION)				    	 	>$(B_ARESNAME)
	@echo --cpu XScale				    			>>$(B_ARESNAME)
	@echo --pd "D_TOOLSETNUM SETA $(B_TOOLSETNUM)"	>>$(B_ARESNAME)
	@echo --pd "D_DEBUG SETA $(B_DEBUG)"		    >>$(B_ARESNAME)


#--------------------------------------------------------------------
#	C Compiler Settings
#--------------------------------------------------------------------

CRESFILES :
	@echo -c  $(B_RVDSCPU)  					>$(B_CRESNAME)
ifneq ($(B_DEBUG), 0)
	@echo -O0 --debug --dwarf2		        	>>$(B_CRESNAME)
else
	@echo -O2 -Otime 							>>$(B_CRESNAME)
endif
	@echo $(B_CFLAGS) $(B_LOCALCFLAGS) $(B_END) >>$(B_CRESNAME)
	@echo --diag_remark 186,188					>>$(B_CRESNAME)


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
	$(B_CC)	$*.c --errors $*.err -o $*.obj --via $(B_CRESNAME)

%.obr :	%.c
	$(B_CC)	$*.c --errors $*.err -o $*.obr --via $(B_CRESNAME)

%.obd :	%.c
	$(B_CC)	$*.c --errors $*.err -o $*.obd --via $(B_CRESNAME)

%.obj :	%.asm
	$(B_ASM) $*.asm	-e $*.err -o $*.obj --via $(B_ARESNAME)

%.obr :	%.asm
	$(B_ASM) $*.asm	-e $*.err -o $*.obr --via $(B_ARESNAME)

%.obd :	%.asm
	$(B_ASM) $*.asm	-e $*.err -o $*.obd --via $(B_ARESNAME)

%.obj :	%.s
	$(B_ASM) $*.s -e $*.err	-o $*.obj --via $(B_ARESNAME)

%.obr :	%.s
	$(B_ASM) $*.s -e $*.err	-o $*.obr --via $(B_ARESNAME)

%.obd :	%.s
	$(B_ASM) $*.s -e $*.err	-o $*.obd --via $(B_ARESNAME)


endif	# TOOLSET_MAK_INCLUDED

