#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLCPUTYP library for
#	ARMV4.	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: armv4.mak $
#	Revision 1.6  2009/02/07 23:32:18Z  garyp
#	Updated to current makefile standards.  Modified to ensure that the local
#	library is always rebuilt to properly allow switching between debug and 
#	release builds.
#	Revision 1.5  2008/11/10 21:43:55Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.4  2008/11/07 05:04:47Z  johnb
#	Cleaned up whitespace
#	Cleaned up use of B_INCLUDES
#	Revision 1.3  2008/11/06 21:22:20Z  johnb
#	Added default $(B_INCLUDES) in PB section
#	Revision 1.2  2008/11/03 20:22:00Z  johnb
#	Added RELEASETYPE=PLATFORM 
#	Revision 1.1  2005/07/06 05:31:30Z  Pauli
#	Initial revision
#	Revision 1.3  2005/06/13 04:08:02Z  PaulI
#	Added support for DCL lib prefix.
#	Revision 1.2  2005/02/22 18:31:30Z  GaryP
#	Minor syntax cleanup.
#	Revision 1.1  2005/01/16 23:17:58Z  GaryP
#	Initial revision
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

$(TARGNAME) : armv4.mak $(CPUTYPEOBJ)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

intr.$(B_OBJEXT)     : $(CPUTYPEINCLUDES) intr.s
cache.$(B_OBJEXT)    : $(CPUTYPEINCLUDES) cache.s


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








