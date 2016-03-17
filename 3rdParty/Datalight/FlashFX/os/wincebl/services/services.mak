#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXOSSERV library.	 It
#	is designed for	use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: services.mak $
#	Revision 1.6  2012/04/12 20:25:34Z  johnb
#	Updates for MSWCEPB toolset.
#	Revision 1.5  2009/02/08 05:53:33Z  garyp
#	Merged from the v4.0 branch.  Modified so the exclude list, if any, is included
#	from the project directory, rather than the current directory.
#	Revision 1.4  2008/01/31 22:25:58Z  Garyp
#	Removed oeio.c.  Updated to get the source file list in the standard
#	fashion.
#	Revision 1.3  2006/12/12 03:51:36Z  Garyp
#	Minor updates to initialize the make process in similar fashion to
#	other make files.
#	Revision 1.2  2006/05/06 22:41:45Z  Garyp
#	Removed oecompat.c.
#	Revision 1.1  2005/10/06 01:11:48Z  Pauli
#	Initial revision
#	Revision 1.3  2005/09/20 17:40:12Z  pauli
#	Removed oesleep.c.
#	Revision 1.2  2005/09/02 19:01:12Z  Garyp
#	Cleaned up the exclusion handling logic to use common functionality in
#	dcl.mak.
#	Revision 1.1  2005/07/06 05:37:08Z  pauli
#	Initial revision
#	Revision 1.4  2005/06/12 03:04:44Z  PaulI
#	Removed oeuser module.  This has been replaced with DCL osoutput.
#	Revision 1.3  2005/01/17 00:59:13Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled in the traverse process.
#	Revision 1.2  2004/12/11 02:59:52Z  tonyq
#	Added perror to WINCEBL OESL
#	Revision 1.1  2004/08/16 18:37:42  garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = fxosserv.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

# Get the list of modules to exclude if	they have been overidden
# at the project layer.
-include $(P_PROJDIR)\ffxexclude.lst

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

OESERVHEADERS =	$(FXHEADERS)


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
OESERVSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
OESERVOBJ	:= $(OESERVSRC:.c=.$(B_OBJEXT))

ifneq ($(strip $(EXCLUSIONOBJ)),)
  # If exclusion modules were specified in exclude.lst, remove the
  # modules from OESERVOBJ, and ensure that the DCLCLEANEXCLUDE
  # target is built so that any now undesired object modules laying
  # around are not unintentionally incorporated into the library.
  OESERVOBJ := $(filter-out $(EXCLUSIONOBJ), $(OESERVOBJ))
  EXCLUDETARG  = DCLCLEANEXCLUDE
  EXCLUSIONDEL = $(subst .$(B_OBJEXT),,$(EXCLUSIONOBJ))
else
  EXCLUDETARG  =
endif


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

OESERVTARGETS :	OESERVDIR TOOLSETINIT $(EXCLUDETARG) $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

OESERVDIR :
	@echo Processing os\$(P_OS)\Services...

$(TARGNAME) : services.mak $(OESERVOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

oedelay.$(B_OBJEXT)  : $(OESERVHEADERS)	oedelay.c
oeintr.$(B_OBJEXT)   : $(OESERVHEADERS)	oeintr.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: OESERVDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=fxosserv				>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(OESERVSRC)				>>sources

