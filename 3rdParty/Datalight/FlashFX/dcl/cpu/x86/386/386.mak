#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLCPUTYP library for
#	386.  It is designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: 386.mak $
#	Revision 1.6  2009/02/07 23:35:31Z  garyp
#	Updated to current makefile standards.  Modified to ensure that the local
#	library is always rebuilt to properly allow switching between debug and 
#	release builds.
#	Revision 1.5  2008/11/10 21:44:10Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.4  2008/11/07 05:05:14Z  johnb
#	Cleaned up whitespace
#	Cleaned up use of B_INCLUDES
#	Revision 1.3  2008/11/06 21:22:10Z  johnb
#	Added default $(B_INCLUDES) in PB section
#	Revision 1.2  2008/11/03 20:22:11Z  johnb
#	Added RELEASETYPE=PLATFORM 
#	Revision 1.1  2005/06/13 03:08:02Z  Pauli
#	Initial revision
#	Revision 1.3  2005/06/13 04:08:02Z  PaulI
#	Added support for DCL lib prefix.
#	Revision 1.2  2005/02/22 18:31:30Z  GaryP
#	Minor syntax cleanup.
#	Revision 1.1  2005/01/16 23:17:56Z  GaryP
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGBASE = dlcputyp
TARGFILE = $(TARGBASE).$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

CPUTYPEINCLUDES	= ..\include\dlx86.inc

# Define an extra general include path
B_LOCALINC = $(P_DCLROOT)\cpu\x86\include


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.asm)
EXCLUDE     	:=
CPUTYPESRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
CPUTYPEOBJ	:= $(CPUTYPESRC:.asm=.$(B_OBJEXT))


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

$(TARGNAME) : 386.mak $(CPUTYPEOBJ)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

stub.$(B_OBJEXT)     : $(CPUTYPEINCLUDES) stub.asm


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



