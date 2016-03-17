#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	the ToolSet specific make settings used
#	throughout the project.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: toolset.mak $
#	Revision 1.2  2010/06/18 20:34:37Z  jimmb
#	Updated the toolset number to correctly reflect icc
#	Revision 1.1  2010/06/16 20:35:40Z  jimmb
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
# match	the values found in dltlset.h and dltlset.inc.  The Intel 
# compiler requires the gnu compiler as well.

B_TOOLSETNUM=5296

B_CINCCMD=-I#				C compiler switch for include path
B_CDEFCMD=-D#				C compiler switch to define a symbol

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

-include $(WIND_BASE)/target/h/make/defs.vsbvars.mk

#--------------------------------------------------------------------
#	Enable optimization when building the release version 
#	dereferencing type-punned pointers are ignored by using 
#	no-strict-aliasing.
#--------------------------------------------------------------------

ifeq ($(B_DEBUG),0)
B_STDCFLAGS += -O2 -ansi -fno-strict-aliasing -fno-builtin -Wall
else
B_STDCFLAGS += -g -ansi -fno-builtin -Wall
endif

B_STDCFLAGS += $(B_GNUOPT)
B_STDCFLAGS += $(B_GNUCPU)
B_STDCFLAGS += -DTOOL_FAMILY=icc -DTOOL=icc -D_VSB_CONFIG_FILE="<$(VSB_CONFIG_FILE)>"
B_STDCFLAGS += $(B_CDEFS)
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
	$(B_CC)	$(B_STDCFLAGS) -o $*.obd $*.c 2>$*.err

%.obr :	%.c
	$(B_CC)	$(B_STDCFLAGS) -o $*.obr $*.c 2>$*.err


endif	# TOOLSET_MAK_INCLUDED

