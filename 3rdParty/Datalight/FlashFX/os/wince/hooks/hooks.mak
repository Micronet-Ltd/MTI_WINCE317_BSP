#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXHOOKS library.  It
#	is designed for	use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: hooks.mak $
#	Revision 1.21  2011/12/14 22:27:54Z  glenns
#	Add dependencies for MX51, Standard ClearNAND and
#	Enhanced ClearNAND NTM default hooks modules.
#	Revision 1.20  2010/09/28 21:46:57Z  glenns
#	Back out previous change pending product release.
#	Revision 1.19  2010/08/06 18:21:19Z  glenns
#	Add dependency for [TRADE NAME TBA] hooks module.
#	Revision 1.18  2009/12/02 20:30:14Z  garyp
#	Updated the dependencies for fhpxa320.c.
#	Revision 1.17  2009/10/06 18:01:39Z  garyp
#	Added fmsl\include to the include path.  Updated dependencies.
#	Revision 1.16  2009/04/09 21:30:15Z  garyp
#	Removed support for the Atlas NTM.
#	Revision 1.15  2009/02/08 05:52:00Z  garyp
#	Merged from the v4.0 branch.  Modified so the exclude list, if any, is
#	included from the project directory, rather than the current directory.
#	Revision 1.14  2008/11/10 21:46:25Z  johnb
#	Remove B_INCLUDES default value.
#	Revision 1.13  2008/11/07 15:11:31Z  johnb
#	Cleaned up whitespace alignment.  Cleaned up use of B_INCLUDES.
#	Revision 1.12  2007/09/13 00:41:40Z  Garyp
#	Added fhmicron.c.
#	Revision 1.11  2007/05/15 23:33:53Z  timothyj
#	Added MX31 project hooks module fhmx31.c.
#	Revision 1.10  2007/02/21 23:29:59Z  rickc
#	Added hooks for pxa320.
#	Revision 1.9  2006/12/12 03:51:28Z  Garyp
#	Minor updates to initialize the make process in similar fashion to
#	other make files.
#	Revision 1.8  2006/07/10 22:57:22Z  Garyp
#	Removed fhbyteio.c.  Renamed fhcadio.c to fhcad.c.
#	Revision 1.7  2006/07/07 18:08:42Z  Garyp
#	Added fhcad.c.
#	Revision 1.6  2006/05/08 22:30:17Z  timothyj
#	Added fhcadio.c
#	Revision 1.5  2006/03/13 01:25:56Z  Garyp
#	Added new modules.
#	Revision 1.4  2006/02/10 02:15:32Z  Garyp
#	Eliminated fhdrive.h, and added several new modules.
#	Revision 1.3  2006/02/03 23:41:31Z  timothyj
#	Added OneNAND project hooks module fh1nand.c
#	Revision 1.2  2005/12/15 06:48:50Z  garyp
#	Fixed to would properly with Windows CE Platform Builder.
#	Revision 1.1  2005/05/02 17:19:00Z  Pauli
#	Initial revision
#	Revision 1.2  2005/05/02 18:19:00Z  Garyp
#	Added fhbbm.c.
#	Revision 1.1  2005/10/06 03:32:44Z  Garyp
#	Initial revision
#	Revision 1.2  2005/09/02 19:19:49Z  Garyp
#	Cleaned up the exclusion handling logic to use common functionality in
#	dcl.mak.
#	Revision 1.1  2005/01/17 02:59:10Z  pauli
#	Initial revision
#	Revision 1.7  2005/01/17 00:59:10Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled in the traverse process.
#	Revision 1.6  2004/08/12 01:45:54Z  GaryP
#	Added fhnand.c.
#	Revision 1.5  2004/07/07 03:04:44Z  GaryP
#	Eliminated the module header.
#	Revision 1.4  2004/07/03 00:18:10Z  GaryP
#	Updated to use new P_ and B_ build symbols.  Renamed fhconfig.c to
#	fhoption.c.  Eliminated funky EXCLUSIONSRC target.
#	Revision 1.2  2004/02/23 05:43:10Z  garys
#	Revision 1.1.1.6  2004/02/23 05:43:10  garyp
#	Added the remaining missing Project Hooks modules.
#	Revision 1.1.1.5  2003/05/04 20:36:50Z	garys
#	added fhstub in	case all other modules are built in the	project	dir
#	Revision 1.1.1.4  2004/01/04 23:57:42  garyp
#	Renamed	access.c and window.c to fhacess.c and fhwindow.c.  Fixed
#	to properly exclude source modules when	generating the SOURCES files.
#	Revision 1.1.1.3  2003/12/17 01:24:28Z	garyp
#	Added window.c.
#	Revision 1.1.1.2  2003/11/24 01:55:50Z	garyp
#	Added some missing logic to get	CE to build.
#	Revision 1.1  2003/11/17 19:33:30Z  garyp
#	Initial	revision
#	Revision 1.1  2003/11/06 23:03:32Z  garyp
#	Initial	revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = fxhooks.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

# Get the list of modules to exclude if	they have been overidden
# at the project layer.
-include $(P_PROJDIR)\ffxexclude.lst

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

B_LOCALINC = $(P_ROOT)\fmsl\include

PRJHOOKSHEADERS	= $(FXHEADERS)


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
PRJHOOKSSRC    	:= $(filter-out $(EXCLUDE), $(ALLSRC))
PRJHOOKSOBJ	:= $(PRJHOOKSSRC:.c=.$(B_OBJEXT))

ifneq ($(strip $(EXCLUSIONOBJ)),)
  # If exclusion modules were specified in exclude.lst, remove the
  # modules from PRJHOOKSOBJ, and ensure that the DCLCLEANEXCLUDE
  # target is built so that any now undesired object modules laying
  # around are not unintentionally incorporated into the library.
  PRJHOOKSSRC := $(filter-out $(EXCLUSIONSRC), $(PRJHOOKSSRC))
  PRJHOOKSOBJ := $(filter-out $(EXCLUSIONOBJ), $(PRJHOOKSOBJ))
  EXCLUDETARG  = DCLCLEANEXCLUDE
  EXCLUSIONDEL = $(subst .$(B_OBJEXT),,$(EXCLUSIONOBJ))
else
  EXCLUDETARG  =
endif


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

PRJHOOKSTARGETS	: PRJHOOKSDIR TOOLSETINIT $(EXCLUDETARG) $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

PRJHOOKSDIR :
	@echo Processing os\$(P_OS)\Hooks...

$(TARGNAME) : hooks.mak $(PRJHOOKSOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

fh1nand.$(B_OBJEXT)      : $(PRJHOOKSHEADERS) fh1nand.c	$(P_ROOT)/fmsl/include/fxnandapi.h
fhaccess.$(B_OBJEXT)     : $(PRJHOOKSHEADERS) fhaccess.c
fhbbm.$(B_OBJEXT)        : $(PRJHOOKSHEADERS) fhbbm.c
fhcad.$(B_OBJEXT)        : $(PRJHOOKSHEADERS) fhcad.c	$(P_ROOT)/fmsl/include/fxnandapi.h
fhdevice.$(B_OBJEXT)     : $(PRJHOOKSHEADERS) fhdevice.c
fhdisk.$(B_OBJEXT)       : $(PRJHOOKSHEADERS) fhdisk.c
fhecc.$(B_OBJEXT)        : $(PRJHOOKSHEADERS) fhecc.c	$(P_ROOT)/fmsl/include/fxnandapi.h
fheclrnand.$(B_OBJEXT)   : $(PRJHOOKSHEADERS) fheclrnand.c	$(P_ROOT)/fmsl/include/fxnandapi.h $(P_ROOT)/fmsl/include/fheclrnand.h
fhinit.$(B_OBJEXT)       : $(PRJHOOKSHEADERS) fhinit.c
fhmicron.$(B_OBJEXT)     : $(PRJHOOKSHEADERS) fhmicron.c	$(P_ROOT)/fmsl/include/fxnandapi.h
fhmx31.$(B_OBJEXT)       : $(PRJHOOKSHEADERS) fhmx31.c	$(P_ROOT)/fmsl/include/fxnandapi.h
fhmx51.$(B_OBJEXT)       : $(PRJHOOKSHEADERS) fhmx51.c	$(P_ROOT)/fmsl/include/fxnandapi.h $(P_ROOT)/fmsl/include/fhmx51.h
fhoption.$(B_OBJEXT)     : $(PRJHOOKSHEADERS) fhoption.c
fhpageio.$(B_OBJEXT)     : $(PRJHOOKSHEADERS) fhpageio.c	$(P_ROOT)/fmsl/include/fxnandapi.h
fhpxa320.$(B_OBJEXT)     : $(PRJHOOKSHEADERS) fhpxa320.c	$(P_ROOT)/fmsl/include/fxnandapi.h $(P_ROOT)/fmsl/include/fhpxa320.h
fhstdclrnand.$(B_OBJEXT) : $(PRJHOOKSHEADERS) fhstdclrnand.c	$(P_ROOT)/fmsl/include/fxnandapi.h $(P_ROOT)/fmsl/include/fhstdclrnand.h
fhstub.$(B_OBJEXT)       : $(PRJHOOKSHEADERS) fhstub.c
fhwindow.$(B_OBJEXT)     : $(PRJHOOKSHEADERS) fhwindow.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: PRJHOOKSDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=fxhooks				>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(PRJHOOKSSRC)				>>sources


