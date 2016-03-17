#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	the ToolSet specific make settings used
#	throughout the project.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: toolset.mak $
#	Revision 1.2  2009/02/08 05:02:27Z  garyp
#	Modified to ignore warning #188.
#	Revision 1.1  2008/07/18 01:28:58Z  garyp
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

B_TOOLSETNUM=9210

B_ASMINCCMD=-I #			Assembler switch for include path
# B_ASMDEFCMD=-D #			Assembler switch to define a symbol
B_CINCCMD=/I #				C compiler switch for include path
B_CDEFCMD=/D #				C compiler switch to define a symbol

TOOLSETINIT : GENERIC_DEFINES GENERIC_INCLUDES


#--------------------------------------------------------------------
#	Assembler Settings
#--------------------------------------------------------------------

B_AFLAGS  = $(B_AOPTION)
ifneq ($(B_DEBUG),0)
B_AFLAGS += -debug
endif
B_AFLAGS += -pd "D_PRODUCTNUM SETA $(P_PRODUCTNUM)"
B_AFLAGS += -pd "D_PRODUCTVER SETA $(P_VERSIONVAL)"
ifdef P_SUBPRODUCTNUM
B_AFLAGS += -pd "D_SUBPRODUCTNUM SETA $(P_SUBPRODUCTNUM)"
endif
B_AFLAGS += -pd "D_TOOLSETNUM SETA $(B_TOOLSETNUM)"
B_AFLAGS += -pd "D_DEBUG SETA $(B_DEBUG)"
B_AFLAGS += $(B_ASMDEFS)
B_AFLAGS += $(B_ASMINCS)


#--------------------------------------------------------------------
#	C Compiler Settings
#--------------------------------------------------------------------

B_STDCFLAGS  = /c /nologo /W3 /WX $(B_CFLAGS) $(B_LOCALCFLAGS)
B_STDCFLAGS += /J $(B_END) /Qwd188
ifeq ($(B_DEBUG),0)
B_STDCFLAGS += /O2
else
B_STDCFLAGS += /Od /Zi
endif
B_STDCFLAGS += $(B_CDEFS)
B_STDCFLAGS += $(B_CINCS)


#--------------------------------------------------------------------
#	Linker Settings
#--------------------------------------------------------------------

#B_LOPTION =
#
#ifeq ($(B_DEBUG),0)
#B_LFLAGS =
#else
#B_LFLAGS =
#endif
#
# If the linker	requires an appended symbol when specifying
# additional libraries on subsequent lines, define it here.
#
#B_LIBADDCMD =
#
#B_LRESNAME=l_flags$(B_DEBUG).rsp
#B_LRESCMD=@$(B_LRESNAME)
#
#LRESFILES :


#--------------------------------------------------------------------
#	Default	Rules for this ToolSet
#--------------------------------------------------------------------

# ".OBD" is used for DEBUG object code
# ".OBR" is used for RELEASE object code


%.o :	%.c
	$(B_CC)	$(B_STDCFLAGS) $*.c 2>&1

%.obd :	%.c
	$(B_CC) $(B_STDCFLAGS) /Fo$*.obd $*.c 2>&1

%.obr :	%.c
	$(B_CC) $(B_STDCFLAGS) /Fo$*.obr $*.c 2>&1

%.o :	%.asm
	$(B_ASM) $(B_AFLAGS) $*.asm 2>&1

%.obd :	%.asm
	$(B_ASM) $(B_AFLAGS) -o $*.obd $*.asm 2>&1

%.obr :	%.asm
	$(B_ASM) $(B_AFLAGS) -o $*.obr $*.asm 2>&1

%.o :	%.s
	$(B_ASM) $(B_AFLAGS) $*.s 2>&1

%.obd :	%.s
	$(B_ASM) $(B_AFLAGS) -o $*.obd $*.s 2>&1

%.obr :	%.s
	$(B_ASM) $(B_AFLAGS) -o $*.obr $*.s 2>&1



endif	# TOOLSET_MAK_INCLUDED
