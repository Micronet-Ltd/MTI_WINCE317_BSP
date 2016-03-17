#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLFATCMN library.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: fatfs.mak $
#	Revision 1.7  2009/06/28 00:25:40Z  garyp
#	Added dlfatbootrec.c and dlfatbpbcr.c.
#	Revision 1.6  2008/11/10 21:41:56Z  johnb
#	Remove B_INCLUDES default value.
#	Revision 1.5  2008/11/07 05:02:55Z  johnb
#	Cleaned up whitespace.  Cleaned up use of B_INCLUDES.
#	Revision 1.4  2008/11/06 21:22:57Z  johnb
#	Added default $(B_INCLUDES) in PB section.
#	Revision 1.3  2008/11/03 20:21:03Z  johnb
#	Added RELEASETYPE=PLATFORM 
#	Revision 1.2  2008/01/14 21:40:41Z  johnb
#	Modified Platform Builder section, specifically TARGETNAME to include
#	$(B_DCLLIBPREFIX) before the library name.
#	Revision 1.1  2007/09/27 01:15:22Z  jeremys
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = dlfatcmn.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

FATFSHEADERS = $(DCLHEADERS) $(P_ROOT)\include\dlfatapi.h $(P_ROOT)\include\dlfat.h


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

include	$(P_ROOT)\product\dcl.mak


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

dlfatbootrec.$(B_OBJEXT) : $(FATFSHEADERS) dlfatbootrec.c dlfatbootrec.h
dlfatbpb.$(B_OBJEXT)   	 : $(FATFSHEADERS) dlfatbpb.c
dlfatbpbcr.$(B_OBJEXT)   : $(FATFSHEADERS) dlfatbpbcr.c
dlfatparam.$(B_OBJEXT)   : $(FATFSHEADERS) dlfatparam.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: FATFSDIR TOOLSETINIT
	@echo #							 >sources
	@echo #This is an auto-generated file -- DO NOT	EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)DLFATCMN		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(FATFSSRC)				>>sources

