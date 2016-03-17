#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the SXIPFIM library.	It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: sxipfim.mak $
#	Revision 1.3  2008/11/10 21:46:39Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.2  2008/11/07 15:12:25Z  johnb
#	Cleaned up whitespace alignment
#	Cleaned up use of B_INCLUDES
#	Revision 1.1  2005/10/01 09:36:32Z  Pauli
#	Initial revision
#	Revision 1.1  2005/10/01 10:36:32Z  Garyp
#	Initial revision
#	Revision 1.1  2005/01/17 02:59:10Z  pauli
#	Initial revision
#	Revision 1.1  2005/01/17 01:59:10Z  pauli
#	Initial revision
#	Revision 1.9  2005/01/17 00:59:09Z  GaryP
#	Modified to no longer change into the target directory since that is already
#	handled in the traverse process.
#	Revision 1.8  2004/07/07 03:04:55Z  GaryP
#	Eliminated the module header.
#	Revision 1.7  2004/07/01 18:44:50Z  GaryP
#	Updated to use new P_ and B_ build symbols.
#	Revision 1.6  2003/03/08 20:45:26Z  garyp
#	Updated	to use the B_LIBEXT setting.
#	Revision 1.5  2002/12/05 11:15:14Z  garyp
#	Fixed to work properly for a command-line CE build.
#	Revision 1.4  2002/11/23 00:31:30Z  garyp
#	Updated	to use $(FXHEADERS).
#	Revision 1.3  2002/11/15 21:22:50Z  garyp
#	Corrected the previous rev.
#	Revision 1.2  2002/11/15 21:12:14Z  garyp
#	Changed	the CE PB ToolSet name to "mswcepb".
#	Revision 1.1  2002/11/12 21:51:32Z  qa
#	Initial	revision
#----------------------------------------------------------------------------

#-----------------------------------------------------------
#	Settings
#-----------------------------------------------------------

# Tell the ToolSet init	code that we need the OE includes
B_NEEDSOSHEADERS = 1

TARGNAME = $(B_RELDIR)\lib\sxipfim.$(B_LIBEXT)

SXIPHEADERS = $(FXHEADERS)

SXIPFIMSRC1 = sxip.c
SXIPFIMOBJ = sxip.$(B_OBJEXT)


#-----------------------------------------------------------
#	Targets
#-----------------------------------------------------------

SXIPFIMTARGETS : SXIPFIMDIR TOOLSETINIT	$(TARGNAME)


#-----------------------------------------------------------
#	Default	Project	Rules
#-----------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#-----------------------------------------------------------
#	Build Commands
#-----------------------------------------------------------

SXIPFIMDIR :
	@echo Processing os\$(P_OS)\OEServ...

$(TARGNAME) : sxipfim.$(B_LIBEXT)
	$(B_COPY) sxipfim.$(B_LIBEXT) $(TARGNAME)

sxipfim.$(B_LIBEXT) : sxipfim.mak $(SXIPFIMOBJ)
	if exist sxipfim.$(B_LIBEXT) del sxipfim.$(B_LIBEXT)
	$(B_BUILDLIB) sxipfim.$(B_LIBEXT) *.$(B_OBJEXT)


#-----------------------------------------------------------
#	Dependencies
#-----------------------------------------------------------

sxip.$(B_OBJEXT)   : $(SXIPFIMHEADERS) sxip.c


#-----------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#-----------------------------------------------------------

mswcepb	: SXIPFIMDIR TOOLSETINIT
	@echo #							 >sources
	@echo #This is an auto-generated file -- DO NOT	EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=SXIPFIM				>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(SXIPFIMSRC1)				>>sources


