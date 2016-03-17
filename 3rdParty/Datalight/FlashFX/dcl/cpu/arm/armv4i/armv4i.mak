#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLCPUTYP library for
#	ARMV4I.	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: armv4i.mak $
#	Revision 1.3  2012/02/16 21:48:12Z  johnb
#	Added "RELEASETYPE=PLATFORM" to mswcepb target
#	Revision 1.2  2009/02/08 04:54:30Z  garyp
#	Updated to current standards.
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGBASE = dlcputyp
TARGFILE = $(TARGBASE).$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

CPUTYPEINCLUDES	= ..\include\dlarm.inc

# Define an extra general include path
B_LOCALINC = $(P_DCLROOT)\cpu\arm\include


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.s)
EXCLUDE     	:=
CPUTYPESRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
CPUTYPEOBJ	:= $(CPUTYPESRC:.s=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

CPUTYPETARGETS : CPUTYPEDIR TOOLSETINIT	$(TARGNAME) EXCLUSIONS


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

# Must always rebuild the lib in the local directory to allow
# switching back and forth between debug and release builds.
#
CPUTYPEDIR :
	@echo Processing CPU\$(P_CPUFAMILY)\$(P_CPUTYPE)...
	if exist $(TARGFILE) del $(TARGFILE)

$(TARGNAME) : armv4i.mak $(CPUTYPEOBJ)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

cache.$(B_OBJEXT)    : $(CPUTYPEINCLUDES) cache.s
intr.$(B_OBJEXT)     : $(CPUTYPEINCLUDES) intr.s
mmu.$(B_OBJEXT)      : $(CPUTYPEINCLUDES) mmu.s


#--------------------------------------------------------------------
#	Build the Exclusion List to prevent the	CPU Family
#	library	from including identically named modules.
#--------------------------------------------------------------------

EXCLUSIONS :
	@echo #						     	 >..\cpubase\exclude.lst
	@echo # This is an auto-generated file -- DO NOT EDIT	>>..\cpubase\exclude.lst
	@echo #							>>..\cpubase\exclude.lst
	@echo EXCLUSIONSRC = $(CPUTYPESRC)			>>..\cpubase\exclude.lst
	@echo EXCLUSIONOBJ = $(CPUTYPEOBJ)			>>..\cpubase\exclude.lst


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: CPUTYPEDIR TOOLSETINIT EXCLUSIONS
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)$(TARGBASE)		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(CPUTYPESRC)				>>sources




