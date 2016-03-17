#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLRELCMN library.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: relfs.mak $
#	Revision 1.7  2009/06/28 00:28:38Z  garyp
#	Updated to use the modern build mechanism.
#	Revision 1.6  2008/11/10 21:42:25Z  johnb
#	Remove B_INCLUDES default value.
#	Revision 1.5  2008/11/07 05:03:55Z  johnb
#	Cleaned up whitespace.  Cleaned up use of B_INCLUDES.
#	Revision 1.4  2008/11/06 21:22:46Z  johnb
#	Added default $(B_INCLUDES) in PB section
#	Revision 1.3  2008/11/05 21:34:08Z  johnb
#	Changed FATFSDIR to RELFSDIR.  Change FATFSSRC to RELFSSRC.  Added 
#   RELEASETYPE=PLATFORM.
#	Revision 1.2  2008/01/14 21:44:57Z  johnb
#	Modified Platform Builder section, specifically TARGETNAME to include
#	$(B_DCLLIBPREFIX) before the library name
#	Revision 1.1  2007/09/27 01:29:14Z  jeremys
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = dlrelcmn.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

RELFSHEADERS = $(DCLHEADERS)


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
RELFSSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
RELFSOBJ	:= $(RELFSSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

RELFSTARGETS : RELFSDIR	TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

RELFSDIR :
	@echo Processing Common\RelFS...

$(TARGNAME) : relfs.mak $(RELFSOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

dlrelparam.$(B_OBJEXT) : $(RELFSHEADERS) dlrelparam.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: RELFSDIR TOOLSETINIT
	@echo #							 >sources
	@echo #This is an auto-generated file -- DO NOT	EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)DLRELCMN		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(RELFSSRC)				>>sources

