#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXFATFS library.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: fatfs.mak $
#	Revision 1.5  2009/07/21 21:58:32Z  garyp
#	Merged from the v4.0 branch.  Removed fatbr.c and fatbpbcr.c.
#	Revision 1.4  2008/11/10 21:45:42Z  johnb
#	Remove B_INCLUDES default value.
#	Revision 1.3  2008/11/07 15:06:23Z  johnb
#	Cleaned up whitespace alignment.  Cleaned up use of B_INCLUDES.
#	Revision 1.2  2007/09/26 21:46:27Z  jeremys
#	Updated to remove source files that were moved to DCL.
#	Revision 1.1  2005/01/17 03:59:20Z  Pauli
#	Initial revision
#	Revision 1.8  2005/01/17 00:59:19Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled in the traverse process.
#	Revision 1.7  2004/08/15 21:22:58Z  GaryP
#	Added fatread.c.
#	Revision 1.6  2004/08/06 16:43:27Z  GaryP
#	Added fatinfo.c.
#	Revision 1.5  2004/07/07 02:56:39Z  GaryP
#	Eliminated the module header.
#	Revision 1.4  2004/06/30 03:19:30Z  GaryP
#	Updated to use new P_ and B_ build symbols.
#	Revision 1.3  2004/01/25 06:13:16Z  garys
#	Merge from FlashFXMT
#	Revision 1.2.1.2  2004/01/25 06:13:16  garyp
#	Added fatbpbcr.c and eliminated	fatutil.c.
#	Revision 1.2  2003/04/16 02:20:40Z  garyp
#	Added FATBR.C and FATPARAM.C.
#	Revision 1.1  2003/04/13 02:50:02Z  garyp
#	Initial	revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = fxfatfs.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

FATFSHEADERS = $(FXHEADERS)


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
FATFSSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
FATFSOBJ	:= $(FATFSSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

FATFSTARGETS : FATFSDIR	TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

# Must always rebuild the lib in the local directory to allow
# switching back and forth between debug and release builds.
#
FATFSDIR :
	@echo Processing Common\FATFS...
	if exist $(TARGFILE) del $(TARGFILE)

$(TARGNAME) : fatfs.mak $(FATFSOBJ)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

fatfmt.$(B_OBJEXT)   : $(FATFSHEADERS) fatfmt.c
fatinfo.$(B_OBJEXT)  : $(FATFSHEADERS) fatinfo.c
fatmon.$(B_OBJEXT)   : $(FATFSHEADERS) fatmon.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: FATFSDIR TOOLSETINIT
	@echo #							 >sources
	@echo #This is an auto-generated file -- DO NOT	EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=FXFATFS				>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(FATFSSRC)				>>sources

