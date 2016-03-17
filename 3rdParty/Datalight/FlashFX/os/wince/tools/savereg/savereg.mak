#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of SAVEREG.EXE.	It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: savereg.mak $
#	Revision 1.4  2012/02/17 19:05:42Z  johnb
#	Added fxoeapi to PRODLIBS for mswcepb toolset.
#	Added missing line continuation character.
#	Revision 1.3  2008/11/10 22:25:57Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.2  2008/11/07 15:15:49Z  johnb
#	Cleaned up whitespace alignment
#	Cleaned up use of B_INCLUDES
#	Revision 1.1  2005/10/01 09:45:34Z  Pauli
#	Initial revision
#	Revision 1.1  2005/10/01 10:45:34Z  Garyp
#	Initial revision
#	Revision 1.1  2005/03/12 03:40:48Z  pauli
#	Initial revision
#	Revision 1.1  2005/03/12 02:40:48Z  pauli
#	Initial revision
#	Revision 1.8  2005/03/12 01:40:48Z  GaryP
#	Updated to use new mechanisms for generating the MSWCEPB link
#	libraries.
#	Revision 1.7  2005/02/27 03:43:02Z  GaryP
#	Updated to use new CE settings that work better with CE 5.
#	Revision 1.6  2005/01/17 00:59:10Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled in the traverse process.
#	Revision 1.5  2004/09/27 19:21:14Z  tonyq
#	Updated for Windows CE 5.0 Platform builder (command line build)
#	Revision 1.4  2004/09/23 21:14:34Z  tonyq
#	Updated to support building from the command line under Windows CE 5.0
#	(with the MSWCE4 toolset).
#	Revision 1.3  2004/07/07 03:06:22Z  GaryP
#	Eliminated the module header.
#	Revision 1.2  2004/07/01 18:44:50Z  GaryP
#	Updated to use new P_ and B_ build symbols.
#	Revision 1.1  2003/05/28 01:16:52Z  garyp
#	Initial	revision
#----------------------------------------------------------------------------

#-----------------------------------------------------------
#	Settings
#-----------------------------------------------------------

# Tell the ToolSet init	code that we need the OE includes
B_NEEDSOSHEADERS = 1

SAVEREGHEADERS = $(FXHEADERS)

SAVEREGSRC = savereg.c
SAVEREGOBJ = savereg.$(B_OBJEXT)

ALLTARGETS =	$(B_RELDIR)\savereg.exe

#-----------------------------------------------------------
#	Targets
#-----------------------------------------------------------

SAVEREGTARGETS : SAVEREGDIR TOOLSETINIT	$(ALLTARGETS)


#-----------------------------------------------------------
#	Default	Project	Rules
#-----------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#-----------------------------------------------------------
#	Build Commands
#-----------------------------------------------------------

SAVEREGDIR :
	@echo Processing os\wince\tools\savereg...

$(B_RELDIR)\savereg.exe	: savereg.mak savereg.$(B_OBJEXT)
	echo /out:$(B_RELDIR)\savereg.exe savereg.$(B_OBJEXT)	 >savereg.lnk
	echo $(B_CESDKDIR)\coredll.lib				>>savereg.lnk
	echo $(B_CESDKDIR)\corelibc.lib				>>savereg.lnk
	$(B_LINK) /nodefaultlib	$(B_LFLAGS) @savereg.lnk


#-----------------------------------------------------------
#	Dependencies
#-----------------------------------------------------------

savereg.$(B_OBJEXT)	: $(SAVEREGHEADERS) savereg.c


#-----------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#-----------------------------------------------------------
PRODLIBS =	$(FXAPPLIBS)				\
		$(DCLLIBS)				\
		fxoecmn.lib				\
		fxoeapi.lib

.PHONY: $(PRODLIBS)

$(PRODLIBS):
	@echo   $$(_TARGETPLATROOT)\lib\$$(_CPUINDPATH)\$(@F)	\>>prodlibs.lst

mswcepb	: SAVEREGDIR TOOLSETINIT $(PRODLIBS)
# NOTE:	Beware of using	standard escape	sequences (\c, \n, etc.)
#	when specifying	paths.	Make sure to double escape (\\c, etc).
	@echo #								  >sources
	@echo # This is an auto-generated file -- DO NOT EDIT		 >>sources
	@echo #								 >>sources
	@echo RELEASETYPE=PLATFORM					 >>sources
	@echo TARGETNAME=SAVEREG					 >>sources
	@echo TARGETTYPE=PROGRAM					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo 	$$(B_CESDKDIR)\coredll.lib				\>>sources
	@echo 	$$(B_CESDKDIR)\ntcompat.lib				\>>sources
	$(B_COPY) /a sources + prodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo SOURCES=$(SAVEREGSRC)					 >>sources


