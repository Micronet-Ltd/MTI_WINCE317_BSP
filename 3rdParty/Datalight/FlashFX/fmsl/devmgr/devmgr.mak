#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXDEVMGR library.  It
#	is designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: devmgr.mak $
#	Revision 1.7  2009/03/24 17:39:06Z  keithg
#	Added devbbm.c dependancies.
#	Revision 1.6  2008/11/07 15:07:25Z  johnb
#	Cleaned up whitespace alignment
#	Revision 1.5  2008/01/16 05:53:32Z  Garyp
#	Added devnand.c.
#	Revision 1.4  2007/01/29 00:19:56Z  Garyp
#	Updated to new method for gather source files to build.  Added
#	devlatency.c.
#	Revision 1.3  2006/03/15 00:59:53Z  rickc
#	Added include path for building with Windows CE Platform Builder
#	Revision 1.2  2006/02/24 04:25:07Z  Garyp
#	Added deverr.h
#	Revision 1.1  2006/02/02 20:58:24Z  Garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = fxdevmgr.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

DEVHEADERS = $(FXHEADERS) devmgr.h ..\include\fimdev.h

# Define an extra general include path
B_LOCALINC = $(P_ROOT)\fmsl\include


#--------------------------------------------------------------------
#	Build a list of all the C source files, excluding those in
#	the EXCLUDE list, and then generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
DEVSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
DEVOBJ		:= $(DEVSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

DEVTARGETS : DEVDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

DEVDIR	:
	@echo Processing FMSL\DevMgr...

$(TARGNAME) : devmgr.mak $(DEVOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

deverr.$(B_OBJEXT)     : $(DEVHEADERS)	deverr.c ..\include\deverr.h
devflash.$(B_OBJEXT)   : $(DEVHEADERS)	devflash.c devflash.h
devlatency.$(B_OBJEXT) : $(DEVHEADERS)	devlatency.c
devmgr.$(B_OBJEXT)     : $(DEVHEADERS)	devmgr.c devflash.h
devnand.$(B_OBJEXT)    : $(DEVHEADERS)	devnand.c
devnor.$(B_OBJEXT)     : $(DEVHEADERS)	devnor.c
devresv.$(B_OBJEXT)    : $(DEVHEADERS)	devresv.c
devbbm.$(B_OBJEXT)     : $(DEVHEADERS)	devbbm.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: DEVDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=FXDEVMGR				>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(DEVSRC)					>>sources
	@echo B_INCLUDES=$(B_LOCALINC);				>>sources


