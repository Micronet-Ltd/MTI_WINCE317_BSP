#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLWINUTIL library.
#	It is designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: util.mak $
#	Revision 1.1.2.2  2012/04/12 18:41:27Z  johnb
#	Updated for MSWCEPB toolset.
#	Revision 1.1  2008/10/28 18:56:42Z  garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Define an extra general include path
B_LOCALINC = ..\include

TARGFILE = dlwinutil.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

UTILHEADERS = $(DCLHEADERS)

#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c *.rc)
EXCLUDE     	:=
UTILSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
UTILOBJ		:= $(UTILSRC:.c=.$(B_OBJEXT))
UTILRES		:= $(UTILSRC:.rc=.res)


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

UTILTARGETS : UTILDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

UTILDIR :
	@echo Processing OS\Win\Util...

$(TARGNAME) : util.mak $(UTILOBJ) $(UTILRES)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

wincomm.$(B_OBJEXT) 	: $(UTILHEADERS) wincomm.c
winlog.$(B_OBJEXT) 	: $(UTILHEADERS) winlog.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: UTILDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)DLWINUTIL		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(UTILSRC)				>>sources

