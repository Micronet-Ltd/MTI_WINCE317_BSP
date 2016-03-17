#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FlashFX tools for
#	a Windows CE Bootloader environment.
#
#	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: tools.mak $
#	Revision 1.2  2012/04/12 20:25:58Z  johnb
#	Updates for MSWCEPB toolset.
#	Revision 1.1  2008/02/24 19:04:58Z  Garyp
#	Initial revision
#----------------------------------------------------------------------------


#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = fxoetool.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

OSTOOLHEADERS =	$(FXHEADERS)
OSTOOLINCLUDES =


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
OSTOOLSSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
OSTOOLSOBJ	:= $(OSTOOLSSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

OSTOOLTARGETS :	OSTOOLDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

OSTOOLDIR :
	@echo Processing os\$(P_OS)\Tools...

$(TARGNAME) : tools.mak $(OSTOOLSOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

fxshell.$(B_OBJEXT) : $(OSTOOLHEADERS) fxshell.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: OSTOOLDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=fxoetool				>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(OSTOOLSSRC)				>>sources


