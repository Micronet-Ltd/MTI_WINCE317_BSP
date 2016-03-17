#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLWINDEBUG library.
#	It is designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: debug.mak $
#	Revision 1.4  2012/02/16 21:48:26Z  johnb
#	Added "RELEASETYPE=PLATFORM" to mswcepb target
#	Revision 1.3  2009/06/25 01:30:18Z  garyp
#	Added winmsgdecode.c.
#	Revision 1.2  2009/02/08 04:18:44Z  garyp
#	Corrected a dependency.
#	Revision 1.1  2008/07/26 20:18:22Z  garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Define an extra general include path
B_LOCALINC = ..\include

TARGFILE = dlwindebug.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

WINDEBUGHEADERS = $(DCLHEADERS) ..\include\dlwinutil.h

#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
WINDEBUGSRC    	:= $(filter-out $(EXCLUDE), $(ALLSRC))
WINDEBUGOBJ	:= $(WINDEBUGSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

WINDEBUGTARGETS : WINDEBUGDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

WINDEBUGDIR	:
	@echo Processing os\Win\Debug...

$(TARGNAME) : debug.mak $(WINDEBUGOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

winmsgdecode.$(B_OBJEXT) : $(WINDEBUGHEADERS) winmsgdecode.c winmsgdecode.h
windebug.$(B_OBJEXT)     : $(WINDEBUGHEADERS) windebug.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: WINDEBUGDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)DLWINDEBUG  		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(WINDEBUGSRC)				>>sources

