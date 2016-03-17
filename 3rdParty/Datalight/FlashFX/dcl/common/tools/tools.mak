#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLTOOLS library.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: tools.mak $
#	Revision 1.13  2010/04/18 21:20:57Z  garyp
#	Added dlshelltools.c.
#	Revision 1.12  2009/03/03 02:57:36Z  brandont
#	Added dlshellex dependency.
#	Revision 1.11  2008/11/10 21:42:41Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.10  2008/11/07 05:04:29Z  johnb
#	Cleaned up whitespace
#	Cleaned up use of B_INCLUDES
#	Revision 1.9  2008/11/06 21:22:28Z  johnb
#	Added default $(B_INCLUDES) in PB section
#	Revision 1.8  2008/11/03 20:21:52Z  johnb
#	Added RELEASETYPE=PLATFORM
#	Revision 1.7  2007/08/28 19:00:23Z  Garyp
#	Added dlshellfs.c.
#	Revision 1.6  2007/01/14 22:47:31Z  Garyp
#	Fixed a syntax error that prevents CE Platform Builder builds from
#	working.
#	Revision 1.5  2006/08/25 01:50:52Z  Garyp
#	Updated to use the properly abstracted copy command.
#	Revision 1.4  2006/08/15 23:01:34Z  Garyp
#	Dependencies updated.
#	Revision 1.3  2006/07/06 00:13:55Z  Pauli
#	Renamed dlstats.c to dlstat.c to avoid a conflict with another file
#	of the same name.
#	Revision 1.2  2006/07/03 17:23:47Z  Pauli
#	Added dclstats.c.
#	Revision 1.1  2006/04/13 17:34:00Z  Pauli
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = dltools.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

TOOLSHEADERS = $(DCLHEADERS)


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
TOOLSSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
TOOLSOBJ	:= $(TOOLSSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

TOOLSTARGETS : TOOLSDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

TOOLSDIR :
	@echo Processing Common\Tools...

$(TARGNAME) : tools.mak	$(TOOLSOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

dlshell.$(B_OBJEXT)	    : $(TOOLSHEADERS) dlshell.c         $(P_ROOT)/include/dlshell.h dlshl.h
dlshellex.$(B_OBJEXT)	    : $(TOOLSHEADERS) dlshellex.c       $(P_ROOT)/include/dlshell.h
dlshellfs.$(B_OBJEXT)	    : $(TOOLSHEADERS) dlshellfs.c       $(P_ROOT)/include/dlshell.h dlshl.h
dlshelltools.$(B_OBJEXT)    : $(TOOLSHEADERS) dlshelltools.c    $(P_ROOT)/include/dlshell.h dlshl.h
dlstat.$(B_OBJEXT)	    : $(TOOLSHEADERS) dlstat.c          $(P_ROOT)/include/dlstats.h


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: TOOLSDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)DLTOOLS		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(TOOLSSRC)				>>sources

