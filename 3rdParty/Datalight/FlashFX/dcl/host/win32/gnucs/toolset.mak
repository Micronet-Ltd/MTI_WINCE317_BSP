#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	the ToolSet specific make settings used
#	throughout the project.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: toolset.mak $
#	Revision 1.3  2010/08/01 23:21:44Z  garyp
#	Added rules for the assembler.
#	Revision 1.2  2009/02/07 23:25:26Z  garyp
#	Replaced the B_COPTION flag with B_CFLAGS and B_LOCALCFLAGS.
#	Revision 1.1  2007/06/15 01:06:10Z  pauli
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

B_TOOLSETNUM=5400

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

B_STDCFLAGS =
B_STDCFLAGS += -c $(B_GNUOPT) $(B_GNUCPU) $(B_CFLAGS) $(B_LOCALCFLAGS) $(B_END)
B_STDCFLAGS += $(B_CDEFS)
B_STDCFLAGS += $(B_CINCS)


#--------------------------------------------------------------------
#	Linker Settings
#--------------------------------------------------------------------

B_LDEBUG=

ifeq ($(B_BYTEORDER),little)
B_LOPTION=$(B_LOPTION) -EL
else
B_LOPTION=$(B_LOPTION) -EB
endif

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
	$(B_CC) $(B_STDCFLAGS) -o $*.obj $*.c 2>$*.err

%.obr :	%.c
	$(B_CC) $(B_STDCFLAGS) -o $*.obr $*.c 2>$*.err

%.obd :	%.c
	$(B_CC) $(B_STDCFLAGS) -o $*.obd $*.c 2>$*.err

%.obj :	%.s
	$(B_ASM) $(B_STDAFLAGS) -o $*.obj $*.s 2>$*.err

%.obr :	%.s
	$(B_ASM) $(B_STDAFLAGS) -o $*.obr $*.s 2>$*.err

%.obd :	%.s
	$(B_ASM) $(B_STDAFLAGS) -o $*.obd $*.s 2>$*.err



endif	# TOOLSET_MAK_INCLUDED

