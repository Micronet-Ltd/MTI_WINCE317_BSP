#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLPERFLOG library.  It
# 	is designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: perflog.mak $
#	Revision 1.5  2008/11/10 21:42:20Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.4  2008/11/07 05:03:44Z  johnb
#	Cleaned up whitespace
#	Cleaned up use of B_INCLUDES
#	Revision 1.3  2008/11/06 21:22:50Z  johnb
#	Added default $(B_INCLUDES) in PB section
#	Revision 1.2  2008/11/03 20:21:33Z  johnb
#	Added RELEASETYPE=PLATFORM 
#	Revision 1.1  2007/01/01 22:35:02Z  Garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = dlperflog.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

PERFLOGHEADERS = $(DCLHEADERS) $(P_DCLROOT)\include\dlperflog.h


#--------------------------------------------------------------------
#	Build a list of the C source files, excluding those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
PERFLOGSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
PERFLOGOBJ	:= $(PERFLOGSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

PERFLOGTARGETS : PERFLOGDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

PERFLOGDIR	:
	@echo Processing Common\PerfLog...

$(TARGNAME) : perflog.mak $(PERFLOGOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

dlperflog.$(B_OBJEXT) : $(PERFLOGHEADERS) dlperflog.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: PERFLOGDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)DLPERFLOG		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(PERFLOGSRC)				>>sources

