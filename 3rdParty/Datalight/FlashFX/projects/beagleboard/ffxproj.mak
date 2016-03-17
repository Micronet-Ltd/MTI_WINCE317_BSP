#----------------------------------------------------------------------------
#				Description
#
#   This make file controls the generation of the FXPROJ library.  It is
#   designed for use with GNU Make or compatibles.
#
#   The functionality of the various OS Services and Hooks modules can
#   be overridden by copying the files into this directory and modifying
#   the functionality as needed.  The standard build process will auto-
#   matically prevent the default files from building.
#
#   The default FlashFX OS Services modules are located in the directory
#   P_ROOT\os\P_OS\services.
#
#   The default FlashFX OS Hooks modules are located in the directory
#   P_ROOT\os\P_OS\hooks.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#   $Log: ffxproj.mak $
#   Revision 1.4  2009/10/06 23:50:06Z  garyp
#   Removed fhcad.c.
#   Revision 1.3  2009/10/06 21:48:22Z  garyp
#   Added project.h and fhecc.c.
#   Revision 1.2  2009/09/23 23:25:26Z  garyp
#   Updated to the modern build mechanisms.
#   Revision 1.1  2009/05/28 14:21:18Z  johnb
#   Initial revision
#----------------------------------------------------------------------------


#--------------------------------------------------------------------
#   Settings
#--------------------------------------------------------------------

# Define extra general include paths
B_LOCALINC =


#--------------------------------------------------------------------
#   Source Modules
#--------------------------------------------------------------------

# List headers and includes unique to the Project Directory
#
PROJHEADERS  = project.h
PROJINCLUDES =

# Use ffx*.c and fh*.c wildcards to list all the FlashFX source files
# to build, with FFXPROJEXCLUDE defining any FlashFX source files in
# this directory which should be skipped.
#
FFXPROJSOURCE       := $(wildcard ffx*.c fh*.c oe*.c)
FFXPROJEXCLUDE =


#--------------------------------------------------------------------
#   Boilerplate Project Build Logic
#--------------------------------------------------------------------

include $(P_ROOT)\product\projdef.mak


#--------------------------------------------------------------------
#   Dependencies
#--------------------------------------------------------------------

ffxmain.$(B_OBJEXT)     : $(PROJHEADERS) ffxmain.c
fhecc.$(B_OBJEXT)       : $(PROJHEADERS) fhecc.c  
fhmicron.$(B_OBJEXT)    : $(PROJHEADERS) fhmicron.c


