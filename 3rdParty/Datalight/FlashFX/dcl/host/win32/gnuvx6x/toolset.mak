#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	the ToolSet specific make settings used
#	throughout the project.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: toolset.mak $
#	Revision 1.5  2009/03/05 22:59:46Z  jimmb
#	Updating the build files for correct implementation of VxWorks 6.7
#	Revision 1.4  2009/02/07 23:25:46Z  garyp
#	Replaced the B_COPTION flag with B_CFLAGS and B_LOCALCFLAGS.
#	Revision 1.3  2008/08/25 19:44:48Z  jimmb
#	Updated for VxWorks 6.7
#	Revision 1.2  2006/12/06 23:55:37Z  Garyp
#	Removed dead code.
#	Revision 1.1  2005/08/21 08:23:12Z  Pauli
#	Initial revision
#	Revision 1.2  2005/08/21 09:23:11Z  garyp
#	Re-abstracted so that the general include paths and general defines are
#	set in dcl.mak.  Enhanced so that multiple include paths are allowed in
#	the same variable.
#	Revision 1.1  2005/06/12 00:05:06Z  pauli
#	Initial revision
#	Revision 1.2  2005/06/12 00:05:06Z  PaulI
#	Added support for product and subproduct numbers.
#	Revision 1.1  2005/04/24 17:19:54Z  GaryP
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

B_TOOLSETNUM=5295

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
B_STDCFLAGS += -DTOOL_FAMILY=gnu -DTOOL=gnu -D_VSB_CONFIG_FILE="<$(VSB_CONFIG_FILE)>"
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

