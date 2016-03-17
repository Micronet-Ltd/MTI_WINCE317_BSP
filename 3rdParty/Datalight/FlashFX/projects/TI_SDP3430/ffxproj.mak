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
#	Revision 1.2  2010/07/03 18:48:33Z  garyp
#	Updated dependencies.
#	Revision 1.1  2009/12/19 01:52:02Z  garyp
#	Initial revision
#----------------------------------------------------------------------------


#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Define extra general include paths
B_LOCALINC = $(P_ROOT)\dcl\include\hardware


#--------------------------------------------------------------------
#	Source Modules
#--------------------------------------------------------------------

# List headers and includes unique to the Project Directory
#
PROJHEADERS  = project.h
PROJINCLUDES =

# Use ffx*.c and fh*.c wildcards to list all the FlashFX source files
# to build, with FFXPROJEXCLUDE defining any FlashFX source files in
# this directory which should be skipped.  Note that files for sub-
# products, such as DCL should not be listed here.
#
FFXPROJSOURCE   	:= $(wildcard ffx*.c fh*.c oe*.c *.s)
FFXPROJEXCLUDE =


#--------------------------------------------------------------------
#	Boilerplate Project Build Logic
#--------------------------------------------------------------------

include	$(P_ROOT)\product\projdef.mak


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

ffxmain.$(B_OBJEXT)  	: $(PROJHEADERS) ffxmain.c
fhecc.$(B_OBJEXT) 	: $(PROJHEADERS) fhecc.c
fhmicron.$(B_OBJEXT) 	: $(PROJHEADERS) fhmicron.c
fhoption.$(B_OBJEXT) 	: $(PROJHEADERS) fhoption.c



