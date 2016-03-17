#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	the ToolSet specific make settings used
#	throughout the project.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: toolset.mak $
#	Revision 1.2  2009/02/07 23:26:02Z  garyp
#	Replaced the B_COPTION flag with B_CFLAGS and B_LOCALCFLAGS.
#	Revision 1.1  2006/12/08 00:51:04Z  Garyp
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

B_TOOLSETNUM=8500

#B_ARESNAME=a_flags$(B_DEBUG).rsp
B_CRESNAME=c_flags$(B_DEBUG).rsp
#B_ASMINCCMD=-I #			Assembler switch for include path
B_CINCCMD=-J#				C compiler switch for include path
B_CDEFCMD=-D#				C compiler switch to define a symbol

TOOLSETINIT : CRESFILES GENERIC_DEFINES GENERIC_INCLUDES


#--------------------------------------------------------------------
#	Assembler Settings
#--------------------------------------------------------------------

#ARESFILES :
#	@echo -g $(B_AOPTION)				    	 >$(B_ARESNAME)
#	@echo --cpu XScale				    	>>$(B_ARESNAME)
#	@echo --pd "D_TOOLSETNUM SETA $(B_TOOLSETNUM)"	    	>>$(B_ARESNAME)
#	@echo --pd "D_DEBUG SETA $(B_DEBUG)"		    	>>$(B_ARESNAME)


#--------------------------------------------------------------------
#	C Compiler Settings
#--------------------------------------------------------------------

CRESFILES :
	@echo -c -Feo -Qs $(B_CFLAGS) $(B_LOCALCFLAGS) $(B_END)  >$(B_CRESNAME)
	@echo $(B_MICROTECCPU)         				>>$(B_CRESNAME)
ifeq ($(B_DEBUG), 0)
	@echo -Ot                  		        	>>$(B_CRESNAME)
else
	@echo -g                    		        	>>$(B_CRESNAME)
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
	$(B_CC)	$*.c -o$*.obj -d$(B_CRESNAME)

%.obr :	%.c
	$(B_CC)	$*.c -o$*.obr -d$(B_CRESNAME)

%.obd :	%.c
	$(B_CC)	$*.c -o$*.obd -d$(B_CRESNAME)



endif	# TOOLSET_MAK_INCLUDED

