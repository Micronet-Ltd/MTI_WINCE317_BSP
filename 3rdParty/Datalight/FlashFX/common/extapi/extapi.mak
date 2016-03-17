#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXEXTAPI library.	 It
#	is designed for	use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: extapi.mak $
#	Revision 1.7  2009/02/09 02:51:44Z  garyp
#	Merged from the v4.0 branch.  Removed extint.h and extdbg.c.
#	Revision 1.6  2008/11/10 22:17:47Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.5  2008/11/07 15:06:02Z  johnb
#	Cleaned up whitespace alignment.  Cleaned up use of B_INCLUDES.
#	Revision 1.4  2006/02/20 22:22:15Z  Garyp
#	Renamed extprof.c to extdbg.c.
#	Revision 1.3  2006/01/21 23:00:48Z  Garyp
#	Eliminated exttrace.c.
#	Revision 1.2  2006/01/01 13:23:46Z  Garyp
#	Added extprof.c.
#	Revision 1.1  2005/01/17 03:59:20Z  Pauli
#	Initial revision
#	Revision 1.6  2005/01/17 00:59:20Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled in the traverse process.
#	Revision 1.5  2004/11/29 18:52:17Z  GaryP
#	Added extdrvfw.c.
#	Revision 1.4  2004/07/07 02:56:32Z  GaryP
#	Eliminated the module header.
#	Revision 1.3  2004/06/30 03:19:29Z  GaryP
#	Updated to use new P_ and B_ build symbols.
#	Revision 1.2  2004/03/15 22:41:57Z  garys
#	Renamed	the modules to fit better into the tree.  Added	exttrace.c.
#	Revision 1.1  2003/11/18 02:42:10Z  garyp
#	Initial	revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = fxextapi.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

EXTAPIHEADERS =	$(FXHEADERS) $(P_ROOT)\include\fxapireq.h


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
EXTAPISRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
EXTAPIOBJ	:= $(EXTAPISRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

EXTAPITARGETS :	EXTAPIDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

EXTAPIDIR :
	@echo Processing Common\ExtApi...

$(TARGNAME) : extapi.mak $(EXTAPIOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

extdrvfw.$(B_OBJEXT) : $(EXTAPIHEADERS)	extdrvfw.c
extfml.$(B_OBJEXT)   : $(EXTAPIHEADERS)	extfml.c
extint.$(B_OBJEXT)   : $(EXTAPIHEADERS)	extint.c
extvbf.$(B_OBJEXT)   : $(EXTAPIHEADERS)	extvbf.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: EXTAPIDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=FXEXTAPI				>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(EXTAPISRC)				>>sources

