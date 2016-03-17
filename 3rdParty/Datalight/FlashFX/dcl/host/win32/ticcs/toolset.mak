#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	the ToolSet specific make settings used
#	throughout the project.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#   $Log: toolset.mak $
#   Revision 1.8  2011/03/01 21:33:52Z  garyp
#   Moved target specific settings into toolinit.bat.
#   Revision 1.7  2010/12/21 21:14:13Z  johnb
#   Added Supressed Warning:  #188-D: pointless comparison to
#   DSP toolset command line options.
#   Revision 1.6  2010/10/21 15:48:59Z  jimmb
#   Supressed Warning:  #188-D: pointless comparison
#   Revision 1.5  2010/10/14 21:35:03Z  johnb
#   update from customer and changed spaces to tabs
#   Revision 1.4  2010/09/17 16:38:15Z  johnb
#   Duplicate set of defines for C6000 compile.  Removed redundant settings.
#   Revision 1.3  2010/08/24 13:08:07Z  johnb
#   Change DSP compiler to use 8 Byte alignment.
#   Revision 1.2  2010/08/11 20:12:40Z  johnb
#   Added ARM9 support
#   Revision 1.1  2010/04/28 15:42:48Z  garyp
#   Initial revision
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

B_TOOLSETNUM=10000

B_CINCCMD=-I=#				C compiler switch for include path
B_CDEFCMD=-D=#				C compiler switch to define a symbol

TOOLSETINIT : GENERIC_DEFINES GENERIC_INCLUDES


#--------------------------------------------------------------------
#	General	Settings
#--------------------------------------------------------------------


#--------------------------------------------------------------------
#	Assembler Settings
#--------------------------------------------------------------------

B_AFLAGS =


#--------------------------------------------------------------------
#	C Compiler Settings
#--------------------------------------------------------------------

B_STDCFLAGS  = -c $(B_CFLAGS) $(B_LOCALCFLAGS)

# Enable optimization when building the release version
ifeq ($(B_DEBUG),0)
B_STDCFLAGS += -O3
else
B_STDCFLAGS += -g
endif

B_STDCFLAGS += $(B_TSOPT)
B_STDCFLAGS += $(B_TSCPU)
B_STDCFLAGS += $(B_TSEND) -pden -pdsr=190
B_STDCFLAGS += $(B_CDEFS) --diag_remark 188
B_STDCFLAGS += $(B_CINCS)


#--------------------------------------------------------------------
#	Linker Settings
#--------------------------------------------------------------------

ifeq ($(B_DEBUG),0)
B_LDEBUG =
else
B_LDEBUG =
endif

#B_LOPTION =

B_LFLAGS = $(B_LOPTION)	$(B_LDEBUG)

# If the linker	requires an appended symbol when specifying
# additional libraries on subsequent lines, define it here.

B_LIBADDCMD =


#--------------------------------------------------------------------
#	Default	Rules for this ToolSet
#--------------------------------------------------------------------

# ".OBD" is used for DEBUG object code
# ".OBR" is used for RELEASE object code

%.obj :	%.c
	$(B_CC)	$(B_STDCFLAGS) $*.c 2>$*.err

%.obd :	%.c
	$(B_CC)	$(B_STDCFLAGS) -eo=.obd $*.c 2>$*.err

%.obr :	%.c
	$(B_CC)	$(B_STDCFLAGS) -eo=.obr $*.c 2>$*.err


endif	# TOOLSET_MAK_INCLUDED

