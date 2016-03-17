#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXOSSERV library.	 It
#	is designed for	use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: services.mak $
#	Revision 1.9  2009/02/08 05:52:42Z  garyp
#	Merged from the v4.0 branch.  Modified so the exclude list, if any, is included
#	from the project directory, rather than the current directory.
#	Revision 1.8  2008/11/10 21:46:31Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.7  2008/11/07 15:11:56Z  johnb
#	Cleaned up whitespace alignment.  Cleaned up use of B_INCLUDES.
#	Revision 1.6  2008/01/31 22:25:58Z  Garyp
#	Removed oeio.c.  Updated to get the source file list in the standard fashion.
#	Revision 1.5  2006/12/12 03:51:35Z  Garyp
#	Minor updates to initialize the make process in similar fashion to
#	other make files.
#	Revision 1.4  2006/05/06 22:41:45Z  Garyp
#	Removed oecompat.c.
#	Revision 1.3  2005/12/30 16:43:42Z  Garyp
#	Eliminated oethread.c (moved into DCL).
#	Revision 1.2  2005/12/15 06:48:50Z  garyp
#	Fixed to would properly with Windows CE Platform Builder.
#	Revision 1.1  2005/10/06 01:11:48Z  Pauli
#	Initial revision
#	Revision 1.3  2005/09/20 17:39:56Z  pauli
#	Removed oesleep.c.
#	Revision 1.2  2005/09/02 19:19:51Z  Garyp
#	Cleaned up the exclusion handling logic to use common functionality in
#	dcl.mak.
#	Revision 1.1  2005/07/06 05:37:08Z  pauli
#	Initial revision
#	Revision 1.18  2005/06/12 03:04:44Z  PaulI
#	Removed oeuser module.  This has been replaced with DCL osoutput.
#	Revision 1.17  2005/01/17 00:59:13Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled in the traverse process.
#	Revision 1.16  2004/11/10 20:37:17Z  GaryP
#	Added oecompat.c.
#	Revision 1.15  2004/07/07 03:04:34Z  GaryP
#	Eliminated the module header.
#	Revision 1.14  2004/07/01 18:44:36Z  GaryP
#	Updated to use new P_ and B_ build symbols.
#	Revision 1.13  2004/02/28 00:57:34Z  garys
#	Merge from FlashFXMT
#	Revision 1.12.1.4  2004/02/28 00:57:34	garyp
#	Fixed so excluded source works right.
#	Revision 1.12.1.3  2004/02/25 03:23:52Z	 garyp
#	Removed	the old	HW Services modules.
#	Revision 1.12.1.2  2004/01/25 06:43:30Z	 garyp
#	Eliminated hwapi.c.
#	Revision 1.11.1.3  2003/12/28 19:59:12Z	 garyp
#	Added oethread.c.
#	Revision 1.11.1.2  2003/11/23 20:37:06Z	 garyp
#	Added OESEM.C.
#	Revision 1.11  2003/09/11 18:11:00Z  billr
#	Add hwecc.c.
#	Revision 1.10  2003/04/15 17:41:42Z  garyp
#	Added new modules.
#	Revision 1.9  2003/03/27 18:47:50Z  garyp
#	Added OEIO.C.
#	Revision 1.8  2002/12/16 03:35:48Z  GaryP
#	Fixed to work with the new GMAKEW32.EXE
#	Revision 1.7  2002/12/05 10:46:48Z  garyp
#	Added exclusion	processing logic I apparently forgot to	add earlier.
#	Revision 1.6  2002/11/23 00:30:00Z  garyp
#	Updated	to use $(FXHEADERS).
#	Revision 1.5  2002/11/15 21:22:50Z  garyp
#	Corrected the previous rev.
#	Revision 1.4  2002/11/15 21:12:14Z  garyp
#	Changed	the CE PB ToolSet name to "mswcepb".
#	Revision 1.3  2002/11/07 10:14:36Z  garyp
#	Updated	to build CEPB SOURCES files.
#	Revision 1.2  2002/10/28 23:15:28Z  garyp
#	Modified the dependency	calculation process.  Added the	module header.
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
  OESERVSRC := $(filter-out $(EXCLUSIONSRC), $(OESERVSRC))
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


