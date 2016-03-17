#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLTESTFW library.
#	It is designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: testfw.mak $
#	Revision 1.9  2009/11/25 23:09:34Z  garyp
#	Updated to use modern build mechansims.  Fixed a broken DCLHEADERS
#	symbol name.
#	Revision 1.8  2009/01/23 00:23:58Z  johnbr
#	Adding Stoch to the build process.
#	Revision 1.7  2009/01/21 18:46:14Z  keithg
#	Added DLTHARNESS.C module.
#	Revision 1.6  2008/11/10 13:42:30  johnb
#	Remove B_INCLUDES default value.
#	Revision 1.5  2008/11/07 05:04:10Z  johnb
#	Cleaned up whitespace.  Cleaned up use of B_INCLUDES.
#	Revision 1.4  2008/11/06 21:22:42Z  johnb
#	Added default $(B_INCLUDES) in PB section.
#	Revision 1.3  2008/11/03 20:21:42Z  johnb
#	Added RELEASETYPE=PLATFORM.
#	Revision 1.2  2008/01/14 21:45:16Z  johnb
#	Modified Platform Builder section, specifically TARGETNAME to include
#	$(B_DCLLIBPREFIX) before the library name.
#	Revision 1.1  2007/10/17 01:31:36Z  brandont
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGBASE = dltestfw
TARGFILE = $(TARGBASE).$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

TESTFWHEADERS = testfw.mak			\
		$(DCLHEADERS)			\
		dltestfwp.h			\
		$(P_ROOT)\include\dltestfw.h	\
		$(P_ROOT)\include\dltharness.h	\
		$(P_ROOT)\include\dlstochfw.h


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
TESTFWSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
TESTFWOBJ	:= $(TESTFWSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

DEBUGTARGETS : TESTFWDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

TESTFWDIR :
	@echo Processing Common\testfw...

$(TARGFILE) : $(TESTFWOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
 
$(TARGNAME) : $(TARGFILE)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------
dltestfw.$(B_OBJEXT)		: $(TESTFWHEADERS) dltestfw.c
dltharness.$(B_OBJEXT)		: $(TESTFWHEADERS) dltharness.c
dlstochfw.$(B_OBJEXT)		: $(TESTFWHEADERS) dlstochfw.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: TESTFWDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)$(TARGBASE)		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(TESTFWSRC)				>>sources


