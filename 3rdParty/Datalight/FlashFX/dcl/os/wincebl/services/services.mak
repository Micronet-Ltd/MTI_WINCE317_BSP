#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLOSSERV library.	 It
#	is designed for	use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: services.mak $
#	Revision 1.15  2012/04/12 20:17:40Z  johnb
#	Updates for MSWCEPB toolset.
#	Revision 1.14  2009/09/16 19:36:15Z  garyp
#	Added osblockdev.c.
#	Revision 1.13  2009/07/15 04:50:51Z  keithg
#	Added OSNET.C to dependancy list.
#	Revision 1.12  2009/06/25 02:16:49Z  garyp
#	Added osatomic.c and osmapmem.c.
#	Revision 1.11  2009/02/07 20:48:27Z  garyp
#	Merged from the v4.0 branch.  Updated to include the "exclude" list from
#	the project directory, rather than the current directory.
#	Revision 1.10.1.3  2008/12/01 01:08:22Z  garyp
#	Updated to include the "exclude" list from the project directory, rather
#	than the current directory.
#	Revision 1.10.1.2  2008/11/30 04:20:25Z  garyp
#	Added osatomic.c and osmapmem.c.
#	Revision 1.10  2008/04/08 23:27:33Z  brandont
#	Added oserr.c.
#	Revision 1.9  2008/03/27 22:41:42Z  Garyp
#	Added osthread.c.
#	Revision 1.8  2007/07/17 20:43:57Z  Garyp
#	Added osrequestor.c.
#	Revision 1.7  2007/04/01 21:58:39Z  Garyp
#	Added osdebug.c.
#	Revision 1.6  2007/03/15 22:07:36Z  Garyp
#	Refactored the modules, placing the directory operations functions in
#	osdir.c and the file I/O functions in osfileio.c.  The generic/
#	miscellaneous or FS related functions remain in osfile.c.  Eliminated
#	the DclOsFileGet/PutChar() functions.  Split DclOsRename() into separate
#	functions for directories and files (even though they may get redirected
#	to the same ultimate function most of the time).
#	Revision 1.5  2006/12/12 03:45:52Z  Garyp
#	Updated to use the same logic as similar make files which have modules
#	which can be overridden.
#	Revision 1.4  2006/07/06 01:26:33Z  Garyp
#	Added oshrtick.c.
#	Revision 1.3  2006/03/16 19:53:56Z  timothyj
#	Added osfile
#	Revision 1.2  2006/02/03 02:01:07Z  Pauli
#	Added osdate.c to provide date/time services.
#	Revision 1.1  2005/10/06 00:30:10Z  Pauli
#	Initial revision
#	Revision 1.3  2005/09/20 18:27:02Z  pauli
#	Added ossleep.c.
#	Revision 1.2  2005/09/02 19:14:39Z  Garyp
#	Cleaned up the exclusion handling logic to use common functionality in
#	dcl.mak.
#	Revision 1.1  2005/07/03 09:11:34Z  pauli
#	Initial revision
#	Revision 1.2  2005/06/17 21:16:42Z  PaulI
#	Added osinput module.
#	Revision 1.1  2005/04/10 18:17:28Z  garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = dlosserv.$(B_LIBEXT)
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
osblockdev.$(B_OBJEXT)  : $(OSSERVHEADERS) osblockdev.c
osdate.$(B_OBJEXT)   	: $(OSSERVHEADERS) osdate.c
osdebug.$(B_OBJEXT)  	: $(OSSERVHEADERS) osdebug.c
osdir.$(B_OBJEXT)    	: $(OSSERVHEADERS) osdir.c
oserr.$(B_OBJEXT)   	: $(OSSERVHEADERS) oserr.c
osfile.$(B_OBJEXT)   	: $(OSSERVHEADERS) osfile.c
osfileio.$(B_OBJEXT) 	: $(OSSERVHEADERS) osfileio.c
oshalt.$(B_OBJEXT)   	: $(OSSERVHEADERS) oshalt.c
oshrtick.$(B_OBJEXT) 	: $(OSSERVHEADERS) oshrtick.c
osinput.$(B_OBJEXT)  	: $(OSSERVHEADERS) osinput.c
osmapmem.$(B_OBJEXT)  	: $(OSSERVHEADERS) osmapmem.c
osmem.$(B_OBJEXT)  	: $(OSSERVHEADERS) osmem.c
osmutex.$(B_OBJEXT)  	: $(OSSERVHEADERS) osmutex.c
osoutput.$(B_OBJEXT) 	: $(OSSERVHEADERS) osoutput.c
osrequestor.$(B_OBJEXT) : $(OSSERVHEADERS) osrequestor.c
ossem.$(B_OBJEXT)    	: $(OSSERVHEADERS) ossem.c
ossleep.$(B_OBJEXT)  	: $(OSSERVHEADERS) ossleep.c
osthread.$(B_OBJEXT)    : $(OSSERVHEADERS) osthread.c
ostick.$(B_OBJEXT)   	: $(OSSERVHEADERS) ostick.c
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
