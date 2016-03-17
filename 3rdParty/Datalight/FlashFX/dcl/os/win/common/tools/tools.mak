#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLWINTOOLS library.  It
#	is designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: tools.mak $
#	Revision 1.2  2012/02/16 21:48:20Z  johnb
#	Added "RELEASETYPE=PLATFORM" to mswcepb target
#	Revision 1.1  2008/10/26 18:31:02Z  garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = dlwintools.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

# Extra include search path for code built in this directory
B_LOCALINC = ../../include

WINTOOLSHEADERS = $(DCLHEADERS) ../../include/dlwinutil.h


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
WINTOOLSSRC     := $(filter-out $(EXCLUDE), $(ALLSRC))
WINTOOLSOBJ	:= $(WINTOOLSSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

WINTOOLSTARGETS : WINTOOLSDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

WINTOOLSDIR	:
	@echo Processing os\win\common\tools...

$(TARGNAME) : tools.mak $(WINTOOLSOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

winshellcmd.$(B_OBJEXT)  : $(WINTOOLSHEADERS) winshellcmd.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: WINTOOLSDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)DLWINTOOLS		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(WINTOOLSSRC)				>>sources

