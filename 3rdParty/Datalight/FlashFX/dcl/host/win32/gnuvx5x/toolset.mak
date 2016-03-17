#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	the ToolSet specific make settings used
#	throughout the project.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: toolset.mak $
#	Revision 1.5  2011/09/20 16:35:29Z  jimmb
#	Added optimization to gnu build of VxWorks
#	Revision 1.4  2009/02/07 23:25:36Z  garyp
#	Replaced the B_COPTION flag with B_CFLAGS and B_LOCALCFLAGS.
#	Revision 1.3  2007/04/25 22:23:03Z  Garyp
#	Updated so that include paths are never quoted since this seems to be
#	unsupported in this version of the VxWorks tools (5.4 at least).
#	Revision 1.2  2006/12/06 23:55:33Z  Garyp
#	Removed dead code.
#	Revision 1.1  2005/12/01 02:09:00Z  Pauli
#	Initial revision
#	Revision 1.2  2005/12/01 02:09:00Z  Pauli
#	Merge with 2.0 product line, build 173.
#	Revision 1.1  2005/08/21 09:23:12Z  Garyp
#	Initial revision
#	Revision 1.2  2005/08/21 09:23:11Z  garyp
#	Re-abstracted so that the general include paths and general defines are
#	set in dcl.mak.  Enhanced so that multiple include paths are allowed in
#	the same variable.
#	Revision 1.1  2005/06/12 00:05:06Z  pauli
#	Initial revision
#	Revision 1.3  2005/08/26 02:23:54Z  brandont
#	Added a compiler switch to disable warning for non-literal format string
#	when checking for printf, sprintf, etc.
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

B_TOOLSETNUM=5290

# The VxWorks 5.x tools (or 5.4 at least) do not seem to support
# include paths which include quotes.  Defining this symbol prevents
# dcl.mak from using quotes.
B_NOQUOTEDINCPATHS=1

B_CINCCMD=-I#				C compiler switch for include path
B_CDEFCMD=-D#				C compiler switch to define a symbol

TOOLSETINIT : GENERIC_DEFINES GENERIC_INCLUDES


#--------------------------------------------------------------------
#	Assembler Settings
#--------------------------------------------------------------------

B_AFLAGS =


#--------------------------------------------------------------------
#	C Compiler Settings
#--------------------------------------------------------------------

#B_STDCFLAGS += -g -ansi -fno-builtin -Werror -Wall

B_STDCFLAGS  = -c $(B_CFLAGS) $(B_LOCALCFLAGS)
ifeq ($(B_DEBUG),0)
B_STDCFLAGS += -O2 -ansi -fno-strict-aliasing -fno-builtin -Wall
else
B_STDCFLAGS += -g -ansi -fno-builtin -Wall
endif
B_STDCFLAGS += -Wno-format
B_STDCFLAGS += $(B_GNUOPT)
B_STDCFLAGS += $(B_GNUCPU)
B_STDCFLAGS += -DTOOL_FAMILY=gnu -DTOOL=gnu
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

