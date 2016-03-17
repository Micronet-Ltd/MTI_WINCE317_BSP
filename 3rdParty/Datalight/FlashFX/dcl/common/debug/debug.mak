#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLDEBUG library.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: debug.mak $
#	Revision 1.10  2010/12/11 20:11:17Z  garyp
#	Added dlsyncvalidate.c.
#	Revision 1.9  2010/02/15 19:52:00Z  garyp
#	Added dliodecode.c.
#	Revision 1.8  2009/11/08 15:59:39Z  garyp
#	Added dlprofrequestor.c.
#	Revision 1.7  2008/11/10 21:41:48Z  johnb
#	Remove B_INCLUDES default value.
#	Revision 1.6  2008/11/07 05:02:45Z  johnb
#	Cleaned up whitespace.  Cleaned up use of B_INCLUDES.
#	Revision 1.5  2008/11/06 21:23:00Z  johnb
#	Added default $(B_INCLUDES) in PB section.
#	Revision 1.4  2008/11/03 20:20:58Z  johnb
#	Added RELEASETYPE=PLATFORM .
#	Revision 1.3  2006/07/06 01:32:42Z  Garyp
#	Cleaned up the library generation logic.
#	Revision 1.2  2006/01/07 07:27:37Z  Garyp
#	Added dlhexdmp.c.
#	Revision 1.1  2005/07/17 06:19:14Z  Pauli
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = dldebug.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

DEBUGHEADERS = $(DCLHEADERS)


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
DEBUGSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
DEBUGOBJ	:= $(DEBUGSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

DEBUGTARGETS : DEBUGDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

DEBUGDIR :
	@echo Processing Common\Debug...

$(TARGNAME) : debug.mak $(DEBUGOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

dlhexdmp.$(B_OBJEXT) 	    : $(DEBUGHEADERS) dlhexdmp.c
dliodecode.$(B_OBJEXT) 	    : $(DEBUGHEADERS) dliodecode.c
dlprof.$(B_OBJEXT)   	    : $(DEBUGHEADERS) dlprof.c
dlprofrequestor.$(B_OBJEXT) : $(DEBUGHEADERS) dlprofrequestor.c
dlsyncvalidate.$(B_OBJEXT)  : $(DEBUGHEADERS) dlsyncvalidate.c
dltrace.$(B_OBJEXT)         : $(DEBUGHEADERS) dltrace.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: DEBUGDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)DLDEBUG		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(DEBUGSRC)				>>sources

