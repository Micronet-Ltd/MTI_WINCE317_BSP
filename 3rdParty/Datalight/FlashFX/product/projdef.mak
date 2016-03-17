#----------------------------------------------------------------------------
#				Description
#
#	This make file contains	boilerplate logic that controls	the
#	generation of the FXPROJ library.  It is included by the
#	ffxproj.mak file in every Project Directory.
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
#	Revision 1.12  2012/04/12 20:26:11Z  johnb
#	Updates for MSWCEPB toolset.
#	Revision 1.11  2012/02/28 20:31:47Z  johnb
#	Updated project source path for Platform Builder.
#	Revision 1.10  2011/03/13 18:37:33Z  garyp
#	Added a PROJCLEAN target.
#	Revision 1.9  2009/11/10 23:39:21Z  garyp
#	Updated so PROJHEADERS is defined after flashfx.mak is included, and
#	include the makefiles in the definition to ensure that source code
#	compilation is dependent on make file changes.
#	Revision 1.8  2009/10/06 14:58:54Z  garyp
#	Added \fmsl\include to the general include path for projects.
#	Revision 1.7  2009/07/29 17:12:27Z  garyp
#	Merged from the v4.0 branch.  Updated to the new paradigm where the
#	DCL exclusions are handled in their own make file.  Updated to handle
#	.s files in the project directory.
#	Revision 1.6  2009/02/09 08:45:37Z  garyp
#	Merged from the v4.0 branch.  Simplified so the exclusion list is simply 
#	placed in a text file in the project directory.
#	Revision 1.5  2008/11/10 21:47:45Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.4  2008/11/07 15:25:11Z  johnb
#	Cleaned up whitespace alignment.  Cleaned up use of B_INCLUDES.
#	Revision 1.3  2007/12/17 06:44:03Z  Garyp
#	Modified to copy any BIB files into the release directory.
#	Revision 1.2  2006/08/28 20:45:28Z  Garyp
#	Enclosed a copy command in quotes so the silly GNU MAKE does not massage
#	the string improperly when it things there is an SH in the system.
#	Revision 1.1  2005/10/06 01:44:06Z  Pauli
#	Initial revision
#	Revision 1.5  2005/09/12 02:45:29Z  garyp
#	Modified to delete any pre-existing .ERR files in the Project Directory.
#	Revision 1.4  2005/09/01 23:30:45Z  Garyp
#	Modified to rename any existing object code out of the way so that it
#	does not get included in the library.
#	Revision 1.3  2005/08/21 11:27:11Z  garyp
#	Re-abstracted so that the general include paths and general defines are
#	set in dcl.mak.  Enhanced so that multiple include paths are allowed in
#	the same variable.
#	Revision 1.2  2005/08/18 02:38:45Z  garyp
#	Modified to avoild mucking with the OSSERV exclusions if building as
#	a sub-product.
#	Revision 1.1  2005/08/06 00:50:40Z  pauli
#	Initial revision
#	Revision 1.5  2005/06/12 03:29:34Z  PaulI
#	Added DCL OSSERV exclusion list handling.
#	Revision 1.4  2005/03/25 21:43:32Z  PaulI
#	Fixed prjhooks exclude list file path.
#	Revision 1.3  2005/03/17 20:41:56Z  GaryP
#	Modified to copy any .REG files to the release directory.
#	Revision 1.2  2005/01/17 00:59:10Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled by the traverse process.
#	Revision 1.1  2004/07/07 03:15:38Z  GaryP
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGBASE = fxproj
TARGFILE = $(TARGBASE).$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

# Tell the ToolSet init	code that we need the OS includes
#
B_NEEDSOSHEADERS = 1

# Project code often needs headers from FMSL include directory,
# so add that path into the list.
#
B_LOCALINC += $(P_ROOT)\fmsl\include


#--------------------------------------------------------------------
#	Build a list of all the C source files.
#
#	Process everything in the FFXPROJSOURCE list, except filter
#	out any files which are specified in the FFXPROJEXCLUDE list.
#--------------------------------------------------------------------

PROJSRC     	:= $(filter-out $(FFXPROJEXCLUDE), $(FFXPROJSOURCE))
PROJOBJ		:= $(PROJSRC:.c=.$(B_OBJEXT))
PROJOBJ		:= $(PROJOBJ:.s=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

FXPROJTARGETS :	PROJDIR	TOOLSETINIT $(TARGNAME) EXCLUSIONS
PROJCLEAN : PROJDIR


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak

# Create the final "headers" list after processing flashfx.mak.
# Add the makefiles themselves to the list.
#
PROJHEADERS += $(FXHEADERS) ffxproj.mak $(P_ROOT)/product/projdef.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

PROJDIR	:
	@echo Processing FlashFX project code -- ($(PROJNAME))...
	@if exist *.err del *.err
	@if exist $(TARGFILE) del $(TARGFILE)

$(TARGNAME) : $(PROJOBJ)
	$(B_BUILDLIB) $(TARGFILE) $(PROJOBJ)
	$(B_COPY) $(TARGFILE) $(TARGNAME)
	if exist *.bib $(B_COPY) *.bib "$(B_RELDIR)\*.*"
	if exist *.reg $(B_COPY) *.reg "$(B_RELDIR)\*.*"


#--------------------------------------------------------------------
#	Build an Exclusion List for project specific FlashFX modules
#	which are overriding Services and Hooks modules, to prevent
#	the libraries from containing identically named modules.
#
#	Note that DCL modules are not handled here, and that we can
#	safely list all the exclusion modules, regardless of the sub-
#	category (Services and Hooks), since the naming conventions
#	require them to be unique.
#--------------------------------------------------------------------

EXCLUSIONS :
	echo #						     	 >$(P_PROJDIR)\ffxexclude.lst
	echo # This is an auto-generated file -- DO NOT EDIT 	>>$(P_PROJDIR)\ffxexclude.lst
	echo #						     	>>$(P_PROJDIR)\ffxexclude.lst
	echo EXCLUSIONSRC = $(PROJSRC)		     		>>$(P_PROJDIR)\ffxexclude.lst
	echo EXCLUSIONOBJ = $(PROJOBJ)		     		>>$(P_PROJDIR)\ffxexclude.lst


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
	@echo TARGETNAME=$(TARGBASE)				>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=					 	>>sources
	@echo SOURCES= 					       \>>sources
	$(B_COPY) /a sources + pbprojsrc.lst			  sources
	@del pbprojsrc.lst
	@echo.							>>sources


