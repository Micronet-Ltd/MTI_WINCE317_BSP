#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLCPUFAM library for the
#	ARM CPU	family.	 It is designed	for use	with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: cpubase.mak $
#	Revision 1.10  2010/07/25 21:12:39Z  garyp
#	Corrected to properly use P_DCLROOT.
#	Revision 1.9  2009/10/17 22:58:53Z  garyp
#	Deleted several modules and added memcpy32a32.s.
#	Revision 1.8  2009/09/25 17:21:29Z  garyp
#	Added memcpy32a2048.s, memcpy32a512.s, and memcpy32a64.s.
#	Revision 1.7  2009/02/07 23:32:34Z  garyp
#	Updated to current makefile standards.  Modified to ensure that the
#	local library is always rebuilt to properly allow switching between
#	debug and release builds.
#	Revision 1.6  2008/11/10 21:44:00Z  johnb
#	Remove B_INCLUDES default value.
#	Revision 1.5  2008/11/07 05:04:56Z  johnb
#	Cleaned up whitespace.  Cleaned up use of B_INCLUDES.
#	Revision 1.4  2008/11/06 21:22:17Z  johnb
#	Added default $(B_INCLUDES) in PB section.
#	Revision 1.3  2008/11/03 20:22:04Z  johnb
#	Added RELEASETYPE=PLATFORM .
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
#	Revision 1.1  2005/01/16 23:17:57Z  GaryP
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

CPUBASEINCLUDES	= ..\include\dlarm.inc

# Define an extra general include path
B_LOCALINC = $(P_DCLROOT)\cpu\arm\include


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.s)
EXCLUDE     	:=
CPUBASESRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
CPUBASEOBJ	:= $(CPUBASESRC:.s=.$(B_OBJEXT))

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

include	$(P_DCLROOT)\product\dcl.mak


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

stub.$(B_OBJEXT)     	  : $(CPUBASEINCLUDES) stub.s
memcpy32a32.$(B_OBJEXT)   : $(CPUBASEINCLUDES) memcpy32a32.s
 

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





