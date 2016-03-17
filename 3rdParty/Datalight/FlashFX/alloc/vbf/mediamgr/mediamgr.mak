#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXMMGR library.  It
#	is designed for	use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: mediamgr.mak $
#	Revision 1.6  2008/11/07 15:03:20Z  johnb
#	Cleaned up whitespace alignment
#	Revision 1.5  2007/10/31 16:55:30Z  Garyp
#	Updated to use the new style of selecting the modules to build.
#	Revision 1.4  2006/02/25 04:13:00Z  Garyp
#	Eliminated hamming.c.
#	Revision 1.3  2006/01/18 22:37:18Z  Rickc
#	Added Intel Sibley support files
#	Revision 1.2  2005/12/15 06:37:44Z  garyp
#	Fixed to would properly with Windows CE Platform Builder.
#	Revision 1.1  2005/12/08 03:08:52Z  Garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Define an extra general include path
B_LOCALINC = $(P_ROOT)\alloc\vbf\include

MMGRHEADERS = $(FXHEADERS) ..\include\vbfint.h ..\include\mediamgr.h

TARGFILE = fxmmgr.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
MMGRSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
MMGROBJ		:= $(MMGRSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

MMGRTARGETS : MMGRDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

MMGRDIR :
	@echo Processing Alloc\VBF\MediaMgr...

$(TARGNAME) : mediamgr.mak $(MMGROBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

mediamgr.$(B_OBJEXT) : $(MMGRHEADERS) mediamgr.c
mmnand.$(B_OBJEXT)   : $(MMGRHEADERS) mmnand.c mmnand.h ..\include\tstwint.h
mmnor.$(B_OBJEXT)    : $(MMGRHEADERS) mmnor.c mmnor.h ..\include\tstwint.h
mmiswf.$(B_OBJEXT)   : $(MMGRHEADERS) mmiswf.c mmiswf.h

#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: MMGRDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=FXMMGR					>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(MMGRSRC)				>>sources
	@echo B_INCLUDES=$(B_LOCALINC);				>>sources


