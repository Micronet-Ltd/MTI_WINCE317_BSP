#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLSERV library.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: servicefw.mak $
#	Revision 1.3  2012/02/16 21:47:59Z  johnb
#	Added "RELEASETYPE=PLATFORM" to mswcepb target
#	Revision 1.2  2010/01/08 00:33:20Z  garyp
#	Updated a dependency.
#	Revision 1.1  2008/12/08 21:51:58Z  garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGBASE = dlserv
TARGFILE = $(TARGBASE).$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

SERVICEFWHEADERS = 	$(DCLHEADERS)			\
			dlservicefw.h			\
			$(P_ROOT)/include/dlinstance.h 	\
			$(P_ROOT)/include/dlservice.h


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
SERVICEFWSRC    := $(filter-out $(EXCLUDE), $(ALLSRC))
SERVICEFWOBJ	:= $(SERVICEFWSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

SERVICEFWTARGETS : SERVICEFWDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

SERVICEFWDIR	:
	@echo Processing Common\ServiceFW...

$(TARGNAME) : servicefw.mak $(SERVICEFWOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

dlinstance.$(B_OBJEXT) 	: $(SERVICEFWHEADERS) dlinstance.c
dlproject.$(B_OBJEXT) 	: $(SERVICEFWHEADERS) dlproject.c
dlservice.$(B_OBJEXT) 	: $(SERVICEFWHEADERS) dlservice.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: SERVICEFWDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)$(TARGBASE)		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(SERVICEFWSRC)				>>sources

