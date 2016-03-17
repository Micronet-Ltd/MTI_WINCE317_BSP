#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXHOOKS library.  It
#	is designed for	use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: hooks.mak $
#	Revision 1.8  2011/12/14 22:18:41Z  glenns
#	Add dependencies for MX51, Standard ClearNAND and
#	Enhanced ClearNAND NTM default hooks modules.
#	Revision 1.7  2010/09/28 21:43:15Z  glenns
#	Back out previous change pending product release.
#	Revision 1.6  2010/08/06 18:12:10Z  glenns
#	Add dependency for [TRADE NAME TBA] hooks module.
#	Revision 1.5  2009/12/02 20:30:15Z  garyp
#	Updated the dependencies for fhpxa320.c.
#	Revision 1.4  2009/10/06 18:02:57Z  garyp
#	Added fmsl\include to the include path.  Updated dependencies.
#	Revision 1.3  2009/04/09 21:29:33Z  garyp
#	Removed support for the Atlas NTM.
#	Revision 1.2  2009/02/08 05:48:40Z  garyp
#	Merged from the v4.0 branch.  Modified so the exclude list, if any, is
#	included from the project directory, rather than the current directory.
#	Revision 1.1  2007/09/13 00:41:42Z  brandont
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

