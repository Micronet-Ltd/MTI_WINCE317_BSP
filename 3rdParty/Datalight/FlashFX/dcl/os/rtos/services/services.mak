#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLOSSERV library.	 It
#	is designed for	use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: services.mak $
#	Revision 1.16  2009/12/02 17:50:19Z  garyp
#	Added osmapmem.c.
#	Revision 1.15  2009/09/16 19:36:14Z  garyp
#	Added osblockdev.c.
#	Revision 1.14  2009/07/15 04:49:18Z  keithg
#	Added OSNET.C to dependancy list.
#	Revision 1.13  2009/06/25 01:19:45Z  garyp
#	Added osatomic.c.
#	Revision 1.12  2009/02/08 00:00:29Z  garyp
#	Merged from the v4.0 branch.  Updated to include the "exclude" list from
#	the project directory, rather than the current directory.
#	Revision 1.11  2008/04/08 21:57:00Z  brandont
#	Added oserr.c.
#	Revision 1.10  2007/07/17 20:43:58Z  Garyp
#	Added osrequestor.c.
#	Revision 1.9  2007/04/01 21:47:55Z  Garyp
#	Added osdebug.c.
#	Revision 1.8  2007/03/15 22:07:37Z  Garyp
#	Refactored the modules, placing the directory operations functions in
#	osdir.c and the file I/O functions in osfileio.c.  The generic/
#	miscellaneous or FS related functions remain in osfile.c.  Eliminated
#	the DclOsFileGet/PutChar() functions.  Split DclOsRename() into separate
#	functions for directories and files (even though they may get redirected
#	to the same ultimate function most of the time).
#	Revision 1.7  2006/12/12 03:51:40Z  Garyp
#	Updated to use the same logic as similar make files which have modules
#	which can be overridden.
#	Revision 1.6  2006/07/06 01:26:33Z  Garyp
#	Added oshrtick.c.
#	Revision 1.5  2006/04/03 21:10:48Z  brandont
#	Added oswcstr.c
#	Revision 1.4  2006/03/02 19:43:41Z  Pauli
#	Add a file system include path to the list of OS include paths.  This
#	is used to put the Reliance include directory in to the path list, which
#	is needed by osfile.c.
#	Revision 1.3  2006/02/09 18:40:28Z  Pauli
#	Added osdata.c for date/time services.
#	Revision 1.2  2005/12/30 16:43:41Z  Garyp
#	Added osthread.c.
#	Revision 1.1  2005/10/06 00:30:10Z  Pauli
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

# Add the file system include path to the list of OS include paths
B_OSINC += $(B_FSINC)

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

osatomic.$(B_OBJEXT) 	: $(OSSERVHEADERS) osatomic.c
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


