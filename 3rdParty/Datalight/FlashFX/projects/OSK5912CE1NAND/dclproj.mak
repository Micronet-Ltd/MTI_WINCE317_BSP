#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLPROJ library.  It is
#	designed for use with GNU Make or compatibles.
#
#	The functionality of the various OS Services modules can be
#	overridden by copying the files into this directory and modifying
#	the functionality as needed.  The standard build process will auto-
#	matically prevent the default files from building.
#
#	The default DCL OS Services modules are located in the directory
#	P_ROOT\dcl\os\P_OS\services.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: dclproj.mak $
#	Revision 1.2  2009/11/09 04:22:51Z  garyp
#	Corrected a broken symbol name.
#	Revision 1.1  2008/12/17 22:11:00Z  garyp
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
PROJHEADERS  = $(P_ROOT)/include/dlinstance.h
PROJINCLUDES =

# Use dcl*.c and os*.c wildcards to list all the DCL source files
# to build, with DCLPROJEXCLUDE defining any DCL source files in
# this directory which should be skipped.
#
DCLPROJSOURCE   	:= $(wildcard dcl*.c os*.c)
DCLPROJEXCLUDE =


#--------------------------------------------------------------------
#	Boilerplate Project Build Logic
#--------------------------------------------------------------------

include	$(P_DCLROOT)\product\projdef.mak


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

dclproj.$(B_OBJEXT)	: $(PROJHEADERS) dclproj.c

