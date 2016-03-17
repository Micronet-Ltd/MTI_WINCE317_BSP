#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXHOOKS library.  It
#	is designed for	use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: hooks.mak $
#	Revision 1.19  2012/04/12 20:24:57Z  johnb
#	Updates for MSWCEPB toolset.
#	Revision 1.18  2011/12/14 22:27:07Z  glenns
#	Add dependencies for MX51, Standard ClearNAND and
#	Enhanced ClearNAND NTM default hooks modules.
#	Revision 1.17  2010/09/28 21:46:35Z  glenns
#	Back out previous change pending product release.
#	Revision 1.16  2010/08/06 18:20:42Z  glenns
#	Add dependency for [TRADE NAME TBA] hooks module.
#	Revision 1.15  2009/12/13 17:43:18Z  garyp
#	Added an include path.
#	Revision 1.14  2009/12/02 20:30:14Z  garyp
#	Updated the dependencies for fhpxa320.c.
#	Revision 1.13  2009/10/06 18:01:50Z  garyp
#	Added fmsl\include to the include path.  Updated dependencies.
#	Revision 1.12  2009/04/09 21:30:27Z  garyp
#	Removed support for the Atlas NTM.
#	Revision 1.11  2009/02/08 05:53:03Z  garyp
#	Merged from the v4.0 branch.  Modified so the exclude list, if any, is
#	included from the project directory, rather than the current directory.
#	Revision 1.10  2007/09/13 00:41:40Z  Garyp
#	Added fhmicron.c.
#	Revision 1.9  2007/05/15 23:34:11Z  timothyj
#	Added MX31 project hooks module fhmx31.c.
#	Revision 1.8  2007/03/13 18:53:58Z  rickc
#	Added fhpxa320.c module.
#	Revision 1.7  2006/12/12 03:51:29Z  Garyp
#	Minor updates to initialize the make process in similar fashion to
#	other make files.
#	Revision 1.6  2006/07/07 18:08:42Z  Garyp
#	Added fhcad.c.
#	Revision 1.5  2006/03/13 02:02:29Z  Garyp
#	Added new modules.
#	Revision 1.4  2006/02/10 02:15:32Z  Garyp
#	Eliminated fhdrive.h, and added several new modules.
#	Revision 1.3  2006/02/03 23:42:45Z  timothyj
#	Added OneNAND project hooks module fh1nand.c
#	Revision 1.2  2006/01/25 19:34:41Z  Garyp
#	Updated to build fhbbm.c.
#	Revision 1.1  2005/10/06 02:32:44Z  Pauli
#	Initial revision
#	Revision 1.2  2005/09/02 18:53:35Z  Garyp
#	Cleaned up the exclusion handling logic to use common functionality in
#	dcl.mak.
#	Revision 1.1  2005/01/17 02:59:12Z  pauli
#	Initial revision
#	Revision 1.2  2005/01/17 00:59:11Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled in the traverse process.
#	Revision 1.1  2004/08/16 18:59:38Z  garyp
#	Initial revision
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

B_LOCALINC = $(P_ROOT)\fmsl\include			\
             $(P_TOOLROOT)\platform\common\src\inc
 

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

