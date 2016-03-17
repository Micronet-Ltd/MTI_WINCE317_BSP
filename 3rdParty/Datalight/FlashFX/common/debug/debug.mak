#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXDEBUG library.	It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: debug.mak $
#	Revision 1.7  2009/02/09 01:57:01Z  garyp
#	Removed dbgdisp.c and fxprof.c.
#	Revision 1.6  2008/11/10 21:45:31Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.5  2008/11/07 15:05:33Z  johnb
#	Cleaned up whitespace alignment.  Cleaned up use of B_INCLUDES.
#	Revision 1.4  2006/07/06 18:48:01Z  Garyp
#	Cleaned up the library generation process.
#	Revision 1.3  2006/02/01 23:42:54Z  Garyp
#	Eliminated dbgtrace.c and added dbgio.c.
#	Revision 1.2  2006/01/01 13:23:45Z  Garyp
#	Added fxtrace.c.
#	Revision 1.1  2005/07/07 03:37:24Z  Pauli
#	Initial revision
#	Revision 1.4  2005/01/17 00:59:21Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled in the traverse process.
#	Revision 1.3  2004/07/07 02:56:16Z  GaryP
#	Eliminated the module header.
#	Revision 1.2  2004/06/30 03:19:26Z  GaryP
#	Updated to use new P_ and B_ build symbols.
#	Revision 1.1  2004/02/02 05:58:24Z  garyp
#	Initial	revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = fxdebug.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

DEBUGHEADERS = $(FXHEADERS)


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

DEBUGTARGETS : DEBUGDIR	TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

DEBUGDIR :
	@echo Processing Common\Debug...

$(TARGNAME) : debug.mak	$(DEBUGOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

dbgio.$(B_OBJEXT)    : $(DEBUGHEADERS) dbgio.c
fxtrace.$(B_OBJEXT)  : $(DEBUGHEADERS) fxtrace.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: DEBUGDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=FXDEBUG				>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(DEBUGSRC)				>>sources

