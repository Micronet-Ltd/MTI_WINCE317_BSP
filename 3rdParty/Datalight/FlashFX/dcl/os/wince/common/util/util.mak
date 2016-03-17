#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLOSUTIL library.  It
#	is designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: util.mak $
#	Revision 1.7  2010/10/24 16:53:15Z  garyp
#	Updated the include path.
#	Revision 1.6  2009/02/08 02:33:42Z  garyp
#	Added dlcedebug.c.
#	Revision 1.5  2008/11/10 21:44:27Z  johnb
#	Remove B_INCLUDES default value.
#	Revision 1.4  2008/11/07 05:05:45Z  johnb
#	Cleaned up whitespace.  Cleaned up use of B_INCLUDES.
#	Revision 1.3  2008/11/06 21:22:00Z  johnb
#	Added default $(B_INCLUDES) in PB section.
#	Revision 1.2  2008/11/03 20:22:22Z  johnb
#	Added RELEASETYPE=PLATFORM .
#	Revision 1.1  2007/11/29 17:56:50Z  Garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Define an extra general include path
B_LOCALINC = ../../../win/include

TARGFILE = dlosutil.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

OSUTILHEADERS = $(DCLHEADERS) $(P_ROOT)\os\$(P_OS)\include\dlceutil.h


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
OSUTILSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
OSUTILOBJ	:= $(OSUTILSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

OSUTILTARGETS : OSUTILDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

OSUTILDIR	:
	@echo Processing os\wince\common\util...

$(TARGNAME) : util.mak $(OSUTILOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

dlceoutput.$(B_OBJEXT) 	: $(OSUTILHEADERS) dlceoutput.c
dlcedebug.$(B_OBJEXT) 	: $(OSUTILHEADERS) dlcedebug.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: OSUTILDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)DLOSUTIL		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(OSUTILSRC)				>>sources

