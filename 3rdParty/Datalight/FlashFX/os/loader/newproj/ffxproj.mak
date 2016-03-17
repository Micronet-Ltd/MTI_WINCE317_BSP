#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXPROJ library.  It is
#	designed for use with GNU Make or compatibles.
#
#	The functionality of the various OS Services and Hooks modules can
#	be overridden by copying the files into this directory and modifying
#	the functionality as needed.  The standard build process will auto-
#	matically prevent the default files from building.
#
#	The default FlashFX OS Services modules are located in the directory
#	P_ROOT\os\P_OS\services.
#
#	The default FlashFX OS Hooks modules are located in the directory
#	P_ROOT\os\P_OS\hooks.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: ffxproj.mak $
#	Revision 1.2  2009/07/18 16:53:34Z  garyp
#	Merged from the v4.0 branch.  Moved to the new mechanism of building
#	FlashFX project code, where DCL specific modules are now handled in
#	dclproj.mak, and exclusion handling is automatic.
#	Revision 1.1  2007/10/10 05:19:08Z  brandont
#	Initial revision
#----------------------------------------------------------------------------


#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Define extra general include paths
B_LOCALINC =


#--------------------------------------------------------------------
#	Source Modules
#--------------------------------------------------------------------

# List headers and includes unique to the Project Directory
#
PROJHEADERS  =
PROJINCLUDES =

# Use ffx*.c and fh*.c wildcards to list all the FlashFX source files
# to build, with FFXPROJEXCLUDE defining any FlashFX source files in
# this directory which should be skipped.
#
FFXPROJSOURCE   	:= $(wildcard ffx*.c fh*.c oe*.c)
FFXPROJEXCLUDE =


#--------------------------------------------------------------------
#	Boilerplate Project Build Logic
#--------------------------------------------------------------------

include	$(P_ROOT)\product\projdef.mak


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

ffxmain.$(B_OBJEXT) : $(PROJHEADERS) ffxmain.c

