#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXDRVRFW library.	 It
#	is designed for	use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: driverfw.mak $
#	Revision 1.15  2010/12/12 07:11:25Z  garyp
#	Added drvwipe.c.
#	Revision 1.14  2010/01/23 21:34:15Z  garyp
#	Updated the header dependencies.
#	Revision 1.13  2009/08/05 17:59:15Z  garyp
#	Removed drvread.c.
#	Revision 1.12  2009/07/21 20:48:49Z  garyp
#	Merged from the v4.0 branch.  Added fxdriverfwapi.h and eliminated
#	driverfw.h.  Added drvloader.c, drvfmlio.c, and drvpower.c.
#	Revision 1.11  2008/11/10 21:45:35Z  johnb
#	Remove B_INCLUDES default value.
#	Revision 1.10  2008/11/07 15:05:44Z  johnb
#	Cleaned up whitespace alignment.  Cleaned up use of B_INCLUDES.
#	Revision 1.9  2008/05/20 15:40:26Z  garyp
#	Merged from the WinMobile branch.
#	Revision 1.8.1.2  2008/05/20 15:40:26Z  garyp
#	Added a missing dependency.
#	Revision 1.8  2008/03/27 16:07:38Z  Garyp
#	Added drvopthook.c.
#	Revision 1.7  2008/01/14 18:10:24Z  Garyp
#	Minor tweaks to work similar to other makefiles.
#	Revision 1.6  2006/11/10 20:18:52Z  Garyp
#	Eliminated drvstats.c.
#	Revision 1.5  2006/05/08 08:05:48Z  Garyp
#	Updated to use the new mechanism for specifying source files.
#	Revision 1.4  2006/03/15 22:24:44Z  Garyp
#	Added drvfmlfw.c and drvvbffw.c -- All-In-One-Framework modules which
#	were in the FML and VBF trees, respectively.
#	Revision 1.3  2006/02/10 07:51:19Z  Garyp
#	Added drvdisk.c, drvopt.c. and drvvbf.c.  Removed drvmount.c and
#	drvdevld.c.
#	Revision 1.2  2005/12/12 20:50:49Z  garyp
#	Added drvstats.c.
#	Revision 1.1  2005/10/22 06:51:22Z  Pauli
#	Initial revision
#	Revision 1.11  2005/02/02 06:31:06Z  GaryP
#	Removed drvioaln.c.
#	Revision 1.10  2005/01/17 00:59:20Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled in the traverse process.
#	Revision 1.9  2004/11/29 17:39:32Z  GaryP
#	Added drvconf.c and drvioctl.c.
#	Revision 1.8  2004/11/19 20:23:51Z  GaryP
#	Updated header dependencies.
#	Revision 1.7  2004/08/25 07:23:02Z  GaryP
#	Added drvgc.c.
#	Revision 1.6  2004/08/13 19:23:21Z  GaryP
#	Added drvdisc.c.
#	Revision 1.5  2004/07/07 02:56:25Z  GaryP
#	Eliminated the module header.
#	Revision 1.4  2004/06/30 03:19:28Z  GaryP
#	Updated to use new P_ and B_ build symbols.
#	Revision 1.3  2004/04/27 23:12:42Z  garyp
#	Added drvtest.c.
#	Revision 1.2  2004/02/01 19:03:36Z  garys
#	Merge from FlashFXMT
#	Revision 1.1.1.4  2004/02/01 19:03:36  garyp
#	Added drvioaln.c.
#	Revision 1.1.1.3  2003/12/05 18:40:26Z	garyp
#	Added drvdev.c and drvdevld.c.
#	Revision 1.1.1.2  2003/11/17 22:21:52Z	garyp
#	Added DRVDISP.C.
#	Revision 1.1  2003/04/13 04:46:44Z  garyp
#	Initial	revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = fxdrvrfw.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

DRIVERFWHEADERS	= 	$(FXHEADERS) 			\
			$(P_ROOT)\include\fxdriver.h 	\
			$(P_ROOT)\include\fxstats.h	\
			$(P_ROOT)\include\fxdriverfwapi.h


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
DRIVERFWSRC     := $(filter-out $(EXCLUDE), $(ALLSRC))
DRIVERFWOBJ	:= $(DRIVERFWSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

DRIVERFWTARGETS	: DRIVERFWDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

DRIVERFWDIR :
	@echo Processing Common\DriverFW...

$(TARGNAME) : driverfw.mak $(DRIVERFWOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

drvcomp.$(B_OBJEXT)  	: $(DRIVERFWHEADERS) drvcomp.c
drvconf.$(B_OBJEXT)  	: $(DRIVERFWHEADERS) drvconf.c
drvdev.$(B_OBJEXT)   	: $(DRIVERFWHEADERS) drvdev.c
drvdisc.$(B_OBJEXT)  	: $(DRIVERFWHEADERS) drvdisc.c
drvdisk.$(B_OBJEXT)  	: $(DRIVERFWHEADERS) drvdisk.c
drvdisp.$(B_OBJEXT)  	: $(DRIVERFWHEADERS) drvdisp.c
drvfmlfw.$(B_OBJEXT) 	: $(DRIVERFWHEADERS) drvfmlfw.c
drvfmlio.$(B_OBJEXT) 	: $(DRIVERFWHEADERS) drvfmlio.c
drvfmt.$(B_OBJEXT)   	: $(DRIVERFWHEADERS) drvfmt.c
drvinit.$(B_OBJEXT)  	: $(DRIVERFWHEADERS) drvinit.c
drvio.$(B_OBJEXT)    	: $(DRIVERFWHEADERS) drvio.c drvio.h
drvioctl.$(B_OBJEXT) 	: $(DRIVERFWHEADERS) drvioctl.c
drvloader.$(B_OBJEXT)  	: $(DRIVERFWHEADERS) drvloader.c
drvlock.$(B_OBJEXT)  	: $(DRIVERFWHEADERS) drvlock.c
drvmbr.$(B_OBJEXT)   	: $(DRIVERFWHEADERS) drvmbr.c
drvopt.$(B_OBJEXT)   	: $(DRIVERFWHEADERS) drvopt.c
drvopthook.$(B_OBJEXT)  : $(DRIVERFWHEADERS) drvopthook.c
drvparam.$(B_OBJEXT) 	: $(DRIVERFWHEADERS) drvparam.c
drvperflog.$(B_OBJEXT)  : $(DRIVERFWHEADERS) drvperflog.c $(P_ROOT)\include\fxperflog.h
drvpower.$(B_OBJEXT)  	: $(DRIVERFWHEADERS) drvpower.c
drvtest.$(B_OBJEXT)  	: $(DRIVERFWHEADERS) drvtest.c
drvvbf.$(B_OBJEXT)   	: $(DRIVERFWHEADERS) drvvbf.c
drvvbffw.$(B_OBJEXT) 	: $(DRIVERFWHEADERS) drvvbffw.c
drvwipe.$(B_OBJEXT) 	: $(DRIVERFWHEADERS) drvwipe.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: DRIVERFWDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=FXDRVRFW				>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(DRIVERFWSRC)				>>sources

