#----------------------------------------------------------------------------
#				Description
#
#	This make file contains	boilerplate logic that controls	the
#	generation of the DLPROJ library.  It is included by the
#	dclproj.mak file in every Project Directory.
#
#	For those unfamiliar with the intricacies of GNU Make, a few tips
#	are in order.
#
#	1) Target names	are case sensitive (however filenames are not).
#	2) Build commands MUST be indented using a TAB character.  Indenting
#	   with	spaces is not tolerated	and will generate an error.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: projdef.mak $
#	Revision 1.9  2012/04/12 20:19:54Z  johnb
#	Updates for MSWCEPB toolset.
#	Revision 1.8  2012/02/28 20:30:29Z  johnb
#	Updated project source path for Platform Builder.
#	Revision 1.7  2011/10/02 02:46:09Z  garyp
#	Updated the replication mechanism to copy any .h files with the same
#	names as repllicated .c files.
#	Revision 1.6  2011/08/04 00:38:00Z  garyp
#	Updated so the original line numbers are preserved for replicated files.
#	Revision 1.5  2011/03/07 04:01:07Z  garyp
#	Changed the order of operatiion so the exclude list gets generated properly.
#	Revision 1.4  2011/03/07 02:43:59Z  garyp
#	Fixed to append the replicate list to the regular source list.
#	Revision 1.3  2011/03/06 23:58:55Z  garyp
#	Updated to support replication.  Added a PROJCLEAN target.
#	Revision 1.2  2009/11/10 23:36:57Z  garyp
#	Updated so PROJHEADERS is defined after dcl.mak is included, and
#	include the makefiles in the definition to ensure that source code
#	compilation is dependent on make file changes.
#	Revision 1.1  2008/12/01 01:24:58Z  garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGBASE = dlproj
TARGFILE = $(TARGBASE).$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

# Tell the ToolSet init	code that we need the OS includes
#
B_NEEDSOSHEADERS = 1


#--------------------------------------------------------------------
#	Configure any replication targets.
#--------------------------------------------------------------------

ifneq ($(strip $(DCLPROJREPLICATESRC)),)
  DCLPROJREPLICATECLEAN =
  DCLPROJREPLICATECLEAN := $(foreach fil, $(DCLPROJREPLICATESRC), $(addsuffix .del,$(fil)))
  TEMPREPLICATE 	:= $(foreach fil, $(DCLPROJREPLICATESRC), $(addsuffix .c,$(fil)))
  TEMPREPLICATE 	:= $(filter-out $(DCLPROJSOURCE), $(TEMPREPLICATE))
  DCLPROJSOURCE         += $(TEMPREPLICATE)
  CLEANTARG  		= DCLPROJCLEAN
  RELPICATETARG  	= DCLREPLICATE
else
  CLEANTARG  		=
  RELPICATETARG  	=
endif


#--------------------------------------------------------------------
#	Build a list of all the C source files.
#
#	Process everything in the DCLPROJSOURCE list, except filter
# 	out any files which are specified in the DCLPROJEXCLUDE list.
#--------------------------------------------------------------------

PROJSRC     	:= $(filter-out $(DCLPROJEXCLUDE), $(DCLPROJSOURCE))
PROJOBJ		:= $(PROJSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

DLPROJTARGETS :	PROJDIR	TOOLSETINIT $(RELPICATETARG) $(TARGNAME) EXCLUSIONS
PROJCLEAN : PROJDIR $(CLEANTARG)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_DCLROOT)\product\dcl.mak 

# Create the final "headers" list after processing
# dcl.mak.  Add the makefiles themselves to the list.
#
PROJHEADERS += $(DCLHEADERS) dclproj.mak $(P_DCLROOT)/product/projdef.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

PROJDIR	:
	@echo Processing DCL project code -- ($(PROJNAME))...
	@if exist *.err del *.err
	@if exist $(TARGFILE) del $(TARGFILE)

$(TARGNAME) : $(PROJOBJ)
	$(B_BUILDLIB) $(TARGFILE) $(PROJOBJ)
	$(B_COPY) $(TARGFILE) $(TARGNAME)
	if exist *.bib $(B_COPY) *.bib "$(B_RELDIR)\*.*"
	if exist *.reg $(B_COPY) *.reg "$(B_RELDIR)\*.*"


#--------------------------------------------------------------------
#	Build an Exclusion List for project specific DCL modules
#	which are overriding Services and Hooks modules, to prevent
#	the libraries from containing identically named modules.
#
#	Note that only DCL modules are handled here, and that we
#	can safely list all the exclusion modules, regardless of
#	the sub-category (Services and Hooks), since the naming
#	conventions require them to be unique.
#--------------------------------------------------------------------

EXCLUSIONS :
	echo #						     	 >$(P_PROJDIR)\dclexclude.lst
	echo # This is an auto-generated file -- DO NOT EDIT 	>>$(P_PROJDIR)\dclexclude.lst
	echo #						     	>>$(P_PROJDIR)\dclexclude.lst
	echo EXCLUSIONSRC = $(PROJSRC)		     		>>$(P_PROJDIR)\dclexclude.lst
	echo EXCLUSIONOBJ = $(PROJOBJ)		     		>>$(P_PROJDIR)\dclexclude.lst


#--------------------------------------------------------------------
#	Replicate any files which have been designated as such.
#--------------------------------------------------------------------

DCLREPLICATE : $(DCLPROJREPLICATESRC)

$(DCLPROJREPLICATESRC):
	if exist $@.* del $@.*
	echo.>$@.tmp
	echo /* !NOTE!  This file is replicated from $(DCLPROJREPLICATEPATH).         >>$@.tmp
	echo            Do not modify it directly, or check it into revision control. >>$@.tmp
	echo */>>$@.tmp
	echo.>>$@.tmp
	echo #line 1 >>$@.tmp
	copy $@.tmp + $(DCLPROJREPLICATEPATH)\$@.c  $@.c
	if exist $(DCLPROJREPLICATEPATH)\$@.h copy $(DCLPROJREPLICATEPATH)\$@.h $@.*
	del $@.tmp


#--------------------------------------------------------------------
#	Delete any replicated files.
#--------------------------------------------------------------------

.PHONY: $(DCLPROJREPLICATECLEAN)

$(DCLPROJREPLICATECLEAN):
	@if exist $(@:.del=.c) del $(@:.del=.c)

DCLPROJCLEAN : $(DCLPROJREPLICATECLEAN)


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

PBPROJSRC = 	$(PROJSRC)

.PHONY: $(PBPROJSRC)

$(PBPROJSRC):
	@echo   ..\$(@F)	\>> pbprojsrc.lst

mswcepb	: PROJDIR TOOLSETINIT EXCLUSIONS $(PBPROJSRC)
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)$(TARGBASE)		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES= 					       \>>sources
	$(B_COPY) /a sources + pbprojsrc.lst			  sources
	@del pbprojsrc.lst
	@echo.							>>sources


