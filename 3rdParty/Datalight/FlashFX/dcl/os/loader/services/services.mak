#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLOSSERV library.	 It
#	is designed for	use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: services.mak $
#	Revision 1.5  2009/07/15 04:52:05Z  keithg
#	Added OSNET.C to dependancy list.
#	Revision 1.4  2009/06/25 01:05:14Z  garyp
#	Added osatomic.c.
#	Revision 1.3  2009/02/07 23:59:46Z  garyp
#	Merged from the v4.0 branch.  Updated to include the "exclude" list from
#	the project directory, rather than the current directory.
#	Revision 1.2  2008/04/17 01:22:52Z  brandont
#	Added oserr.c
#	Revision 1.1  2007/07/17 20:43:58Z  brandont
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
osdate.$(B_OBJEXT)   	: $(OSSERVHEADERS) osdate.c
osdebug.$(B_OBJEXT)  	: $(OSSERVHEADERS) osdebug.c
osdir.$(B_OBJEXT)    	: $(OSSERVHEADERS) osdir.c
oserr.$(B_OBJEXT)  	: $(OSSERVHEADERS) oserr.c
osfile.$(B_OBJEXT)   	: $(OSSERVHEADERS) osfile.c
osfileio.$(B_OBJEXT) 	: $(OSSERVHEADERS) osfileio.c
oshalt.$(B_OBJEXT)   	: $(OSSERVHEADERS) oshalt.c
oshrtick.$(B_OBJEXT) 	: $(OSSERVHEADERS) oshrtick.c
osinput.$(B_OBJEXT)  	: $(OSSERVHEADERS) osinput.c
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

