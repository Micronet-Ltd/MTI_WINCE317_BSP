#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLOSSERV library.	 It
#	is designed for	use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: services.mak $
#	Revision 1.19  2009/09/14 15:40:13Z  garyp
#	Added osblockdev.c.
#	Revision 1.18  2009/07/15 04:50:42Z  keithg
#	Added OSNET.C to dependancy list.
#	Revision 1.17  2009/06/25 02:03:37Z  garyp
#	Added osatomic.c and osmapmem.c.
#	Revision 1.16  2009/02/08 00:01:45Z  garyp
#	Merged from the v4.0 branch.  Updated to include the "exclude" list from
#	the project directory, rather than the current directory.
#	Revision 1.15  2008/11/10 21:44:32Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.14  2008/11/07 05:05:54Z  johnb
#	Cleaned up whitespace
#	Cleaned up use of B_INCLUDES
#	Revision 1.13  2008/11/06 21:21:57Z  johnb
#	Added default $(B_INCLUDES) in PB section
#	Revision 1.12  2008/11/03 20:22:29Z  johnb
#	Added RELEASETYPE=PLATFORM
#	Revision 1.11  2008/04/08 23:20:48Z  brandont
#	Added oserr.c.
#	Revision 1.10  2007/08/18 19:39:30Z  garyp
#	Updated a header dependency.
#	Revision 1.9  2007/07/17 20:43:56Z  Garyp
#	Added osrequestor.c.
#	Revision 1.8  2007/04/01 22:01:47Z  Garyp
#	Added osdebug.c.
#	Revision 1.7  2007/03/15 22:34:59Z  Garyp
#	Updated dependencies.
#	Revision 1.6  2006/12/14 00:48:23Z  Garyp
#	Added osdir.c and osfileio.c.
#	Revision 1.5  2006/12/12 03:45:52Z  Garyp
#	Updated to use the same logic as similar make files which have modules
#	which can be overridden.
#	Revision 1.4  2006/06/23 15:07:31Z  Garyp
#	Added oshrtick.c.
#	Revision 1.3  2006/02/08 00:18:07Z  Pauli
#	Added osdate.c to provide date/time services.
#	Revision 1.2  2005/12/30 16:43:40Z  Garyp
#	Added osthread.c.
#	Revision 1.1  2005/10/06 00:30:10Z  Pauli
#	Initial revision
#	Revision 1.3  2005/09/20 18:27:02Z  pauli
#	Added ossleep.c.
#	Revision 1.2  2005/09/02 19:19:50Z  Garyp
#	Cleaned up the exclusion handling logic to use common functionality in
#	dcl.mak.
#	Revision 1.1  2005/07/03 09:11:34Z  pauli
#	Initial revision
#	Revision 1.3  2005/06/17 21:16:15Z  PaulI
#	Added osinput module.
#	Revision 1.2  2005/06/12 22:10:10Z  PaulI
#	Added prefix to DCL osserv lib.
#	Revision 1.1  2005/04/10 18:17:28Z  garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGBASE = dlosserv
TARGFILE = $(TARGBASE).$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

# Get the list of modules to exclude if	they have been overidden
# at the project layer.
-include $(P_PROJDIR)\dclexclude.lst

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

OSSERVHEADERS =	$(DCLHEADERS)

#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
OSSERVSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
OSSERVOBJ	:= $(OSSERVSRC:.c=.$(B_OBJEXT))

ifneq ($(strip $(EXCLUSIONOBJ)),)
  # If exclusion modules were specified in exclude.lst, remove the
  # modules from OSSERVOBJ, and ensure that the DCLCLEANEXCLUDE
  # target is built so that any now undesired object modules laying
  # around are not unintentionally incorporated into the library.
  OSSERVSRC := $(filter-out $(EXCLUSIONSRC), $(OSSERVSRC))
  OSSERVOBJ := $(filter-out $(EXCLUSIONOBJ), $(OSSERVOBJ))
  EXCLUDETARG  = DCLCLEANEXCLUDE
  EXCLUSIONDEL = $(subst .$(B_OBJEXT),,$(EXCLUSIONOBJ))
else
  EXCLUDETARG  =
endif


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

OSSERVTARGETS :	OSSERVDIR TOOLSETINIT $(EXCLUDETARG) $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

OSSERVDIR :
	@echo Processing os\$(P_OS)\Services...

$(TARGNAME) : services.mak $(OSSERVOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

osatomic.$(B_OBJEXT)  	: $(OSSERVHEADERS) osatomic.c
osblockdev.$(B_OBJEXT) 	: $(OSSERVHEADERS) osblockdev.c
osdate.$(B_OBJEXT)   	: $(OSSERVHEADERS) osdate.c osdate.h $(P_ROOT)\include\dlwindows.h
osdebug.$(B_OBJEXT)  	: $(OSSERVHEADERS) osdebug.c
osdir.$(B_OBJEXT)    	: $(OSSERVHEADERS) osdir.c
oserr.$(B_OBJEXT)  	: $(OSSERVHEADERS) oserr.c
osfile.$(B_OBJEXT)   	: $(OSSERVHEADERS) osfile.c osdate.h
osfileio.$(B_OBJEXT) 	: $(OSSERVHEADERS) osfileio.c
oshalt.$(B_OBJEXT)   	: $(OSSERVHEADERS) oshalt.c
oshrtick.$(B_OBJEXT) 	: $(OSSERVHEADERS) oshrtick.c
osinput.$(B_OBJEXT)  	: $(OSSERVHEADERS) osinput.c
osmapmem.$(B_OBJEXT)   	: $(OSSERVHEADERS) osmapmem.c
osmem.$(B_OBJEXT)    	: $(OSSERVHEADERS) osmem.c
osmutex.$(B_OBJEXT)  	: $(OSSERVHEADERS) osmutex.c
osoutput.$(B_OBJEXT) 	: $(OSSERVHEADERS) osoutput.c
osrequestor.$(B_OBJEXT) : $(OSSERVHEADERS) osrequestor.c
ossem.$(B_OBJEXT)    	: $(OSSERVHEADERS) ossem.c
ossleep.$(B_OBJEXT)  	: $(OSSERVHEADERS) ossleep.c
osthread.$(B_OBJEXT) 	: $(OSSERVHEADERS) osthread.c
ostick.$(B_OBJEXT)   	: $(OSSERVHEADERS) ostick.c
oswcstr.$(B_OBJEXT)  	: $(OSSERVHEADERS) oswcstr.c
osnet.$(B_OBJEXT)  	: $(OSSERVHEADERS) osnet.c



#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: OSSERVDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(P_PREFIX)$(TARGBASE)			>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(OSSERVSRC)				>>sources


