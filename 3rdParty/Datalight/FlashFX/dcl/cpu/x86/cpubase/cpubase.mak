#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLCPUFAM library for the
#	x86 CPU	family.	 It is designed	for use	with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: cpubase.mak $
#	Revision 1.7  2009/02/07 23:33:39Z  garyp
#	Updated to current makefile standards.  Modified to ensure that the local
#	library is always rebuilt to properly allow switching between debug and 
#	release builds.
#	Revision 1.6  2008/11/10 21:44:14Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.5  2008/11/07 05:05:25Z  johnb
#	Cleaned up whitespace
#	Cleaned up use of B_INCLUDES
#	Revision 1.4  2008/11/06 21:22:07Z  johnb
#	Added default $(B_INCLUDES) in PB section
#	Revision 1.3  2008/11/03 20:22:14Z  johnb
#	Added RELEASETYPE=PLATFORM 
#	Revision 1.2  2006/12/12 03:51:32Z  Garyp
#	Updated to use the same logic as similar make files which have modules
#	which can be overridden.
#	Revision 1.1  2005/09/02 18:19:50Z  Pauli
#	Initial revision
#	Revision 1.2  2005/09/02 19:19:50Z  Garyp
#	Cleaned up the exclusion handling logic to use common functionality in
#	dcl.mak.
#	Revision 1.1  2005/06/13 04:08:02Z  pauli
#	Initial revision
#	Revision 1.3  2005/06/13 04:08:02Z  PaulI
#	Added support for DCL lib prefix.
#	Revision 1.2  2005/02/22 18:31:30Z  GaryP
#	Minor syntax cleanup.
#	Revision 1.1  2005/01/16 23:17:55Z  GaryP
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGBASE = dlcpufam
TARGFILE = $(TARGBASE).$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

# Retrieve the list of any modules we must exclude from	the library
-include exclude.lst

CPUBASEINCLUDES	= ..\include\dlx86.inc

# Define an extra general include path
B_LOCALINC = $(P_DCLROOT)\cpu\x86\include


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.asm)
EXCLUDE     	:=
CPUBASESRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
CPUBASEOBJ	:= $(CPUBASESRC:.asm=.$(B_OBJEXT))

ifneq ($(strip $(EXCLUSIONOBJ)),)
  # If exclusion modules were specified in exclude.lst, remove the
  # modules from CPUBASEOBJ, and ensure that the DCLCLEANEXCLUDE
  # target is built so that any now undesired object modules laying
  # around are not unintentionally incorporated into the library.
  CPUBASESRC := $(filter-out $(EXCLUSIONSRC), $(CPUBASESRC))
  CPUBASEOBJ := $(filter-out $(EXCLUSIONOBJ), $(CPUBASEOBJ))
  EXCLUDETARG  = DCLCLEANEXCLUDE
  EXCLUSIONDEL = $(subst .$(B_OBJEXT),,$(EXCLUSIONOBJ))
else
  EXCLUDETARG  =
endif


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

CPUBASETARGETS : CPUBASEDIR TOOLSETINIT	$(EXCLUDETARG) $(TARGNAME)


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
CPUBASEDIR :
	@echo Processing CPU\$(P_CPUFAMILY)\CPUBase...
	if exist $(TARGFILE) del $(TARGFILE)

$(TARGNAME) : cpubase.mak $(CPUBASEOBJ)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

diag.$(B_OBJEXT)     : $(CPUBASEINCLUDES) diag.asm
intr.$(B_OBJEXT)     : $(CPUBASEINCLUDES) intr.asm
memcmp16.$(B_OBJEXT) : $(CPUBASEINCLUDES) memcmp16.asm
memcpy16.$(B_OBJEXT) : $(CPUBASEINCLUDES) memcpy16.asm
memset16.$(B_OBJEXT) : $(CPUBASEINCLUDES) memset16.asm
serial.$(B_OBJEXT)   : $(CPUBASEINCLUDES) serial.asm


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: CPUBASEDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)$(TARGBASE)		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(CPUBASESRC)				>>sources


