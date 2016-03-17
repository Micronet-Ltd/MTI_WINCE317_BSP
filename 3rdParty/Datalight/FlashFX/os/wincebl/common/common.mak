#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXOECMN library.	It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: common.mak $
#	Revision 1.5  2012/04/12 20:24:41Z  johnb
#	Updates for MSWCEPB toolset.
#	Revision 1.4  2009/07/18 02:00:04Z  garyp
#	Merged from the v4.0 branch.  Added parsedrv.c.
#	Revision 1.3  2009/04/15 20:56:06Z  garyp
#	Disabled building regread.c.
#	Revision 1.2  2007/12/15 02:28:08Z  Garyp
#	Added fxshellcmd.c.
#	Revision 1.1  2005/10/01 09:35:10Z  Pauli
#	Initial revision
#	Revision 1.2  2005/01/17 00:59:22Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled in the traverse process.
#	Revision 1.1  2004/08/17 16:39:16Z  garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = fxoecmn.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

OSCOMMONHEADERS	= $(FXHEADERS)
OSCOMMONINCLUDES =


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:= regread.c
OSCOMMONSRC    	:= $(filter-out $(EXCLUDE), $(ALLSRC))
OSCOMMONOBJ	:= $(OSCOMMONSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

OSCOMMONTARGETS	: OSCOMMONDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

OSCOMMONDIR :
	@echo Processing os\$(P_OS)\Common...

$(TARGNAME) : common.mak $(OSCOMMONOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

fxceblshellcmd.$(B_OBJEXT)   	: $(OSCOMMONHEADERS) fxceblshellcmd.c
option.$(B_OBJEXT)    		: $(OSCOMMONHEADERS) option.c
parsedrv.$(B_OBJEXT)   		: $(OSCOMMONHEADERS) parsedrv.c
regread.$(B_OBJEXT)    		: $(OSCOMMONHEADERS) regread.c fxreg.h
regwrite.$(B_OBJEXT)   		: $(OSCOMMONHEADERS) regwrite.c fxreg.h


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: OSCOMMONDIR TOOLSETINIT
	@echo #							 >sources
	@echo #This is an auto-generated file -- DO NOT	EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=FXOECMN				>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(OSCOMMONSRC)				>>sources

