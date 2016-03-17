#----------------------------------------------------------------------------
#				Description
#
#	This make file contains	common rules for Borland tools
#	versions 4.0 and later.	 It is included	by TOOLSET.MAK.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: devtools.mak $
#	Revision 1.4  2009/02/07 23:24:37Z  garyp
#	Replaced the B_COPTION flag with B_CFLAGS and B_LOCALCFLAGS.
#	Revision 1.3  2007/12/01 01:44:19Z  brandont
#	Disabled treating compiler warnings as errors.
#	Revision 1.2  2006/12/06 23:55:31Z  Garyp
#	Removed dead code.
#	Revision 1.1  2005/08/21 08:15:06Z  Pauli
#	Initial revision
#	Revision 1.2  2005/08/21 09:15:05Z  garyp
#	Re-abstracted so that the general include paths and general defines are
#	set in dcl.mak.  Enhanced so that multiple include paths are allowed in
#	the same variable.
#	Revision 1.1  2005/06/11 06:27:40Z  pauli
#	Initial revision
#	Revision 1.2  2005/06/11 06:27:40Z  PaulI
#	Added support for product and subproduct numbers.
#	Revision 1.1  2005/01/17 01:59:01Z  GaryP
#	Initial revision
#----------------------------------------------------------------------------

MEMMODEL=c


#-----------------------------------------------
#	Reality	Checks
#-----------------------------------------------



#-----------------------------------------------
#	Assembler Settings
#-----------------------------------------------

ARESFILES :
	@echo /mx /w2 /w-ICG /l	$(B_AOPTION)	      	 >$(B_ARESNAME)


#-----------------------------------------------
#	C Compiler Settings
#-----------------------------------------------

ifeq ($(P_CPUTYPE), x86)
# BC v4.0 or better defaults to	286 code, turn it off for x86
B_CCPU = -2-
else
# Set to 32-bit	if anything other than "x86"
B_CCPU = -3
endif

#rem 	@echo -c -d -w -w! -w-stu -k- -O -Z -x-	 	 >$(B_CRESNAME)
CRESFILES :
	@echo -c -d -k- -O -Z -x-	 		 >$(B_CRESNAME)
	@echo -m$(MEMMODEL) $(B_CFLAGS) $(B_LOCALCFLAGS)  >>$(B_CRESNAME)
	@echo -E$(B_ASM)			        >>$(B_CRESNAME)
	@echo -L$(P_TOOLROOT)\$(B_TOOLLIB)	        >>$(B_CRESNAME)
	@echo $(B_CCPU)				        >>$(B_CRESNAME)
ifneq ($(B_DEBUG), 0)
	@echo -v				        >>$(B_CRESNAME)
endif


#-----------------------------------------------
#	Linker Settings
#-----------------------------------------------

ifeq ($(B_DEBUG), 0)
B_LAPPDEBUG =
B_LDRVDEBUG =
else
B_LAPPDEBUG = /v
B_LDRVDEBUG =
endif

#B_LOPTION =

B_LFLAGS = /s /e /l /m /P32 /L$(P_TOOLROOT)\$(B_TOOLLIB) $(B_LOPTION)

# If the linker	requires an appended symbol when specifying
# additional libraries on subsequent lines, define it here.

B_LIBADDCMD = +


#-----------------------------------------------
#	Default	Rules for this ToolSet
#-----------------------------------------------

# ".OBD" is used for DEBUG object code
# ".OBR" is used for RELEASE object code


%.obj :	%.c
	$(B_CC)	@$(B_CRESNAME) $*.c

%.obd :	%.c
	$(B_CC)	-o$*.obd @$(B_CRESNAME)	$*.c

%.obr :	%.c
	$(B_CC)	-o$*.obr @$(B_CRESNAME)	$*.c

%.obj :	%.asm
	$(B_ASM) @$(B_ARESNAME)	$*.asm

%.obd :	%.asm
	$(B_ASM) @$(B_ARESNAME)	$*.asm,	$*.obd

%.obr :	%.asm
	$(B_ASM) @$(B_ARESNAME)	$*.asm,	$*.obr
