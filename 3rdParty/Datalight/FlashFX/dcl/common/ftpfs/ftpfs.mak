#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DAFAUTOFW library.  It
#	is designed for	use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: ftpfs.mak $
#	Revision 1.2  2010/12/19 04:30:54Z  jeremys
#	Updated dependencies.
#	Revision 1.1  2009/07/15 06:44:58Z  keithg
#	Initial revision
#	Revision 1.1  2007/04/14 06:01:24Z  brandont
#	Initial revision
#	Revision 1.1  2007/02/22 04:24:28Z  brandont
#	Initial revision
#----------------------------------------------------------------------------


#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = dlftpfs.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

FTPFSHEADERS = $(DCLHEADERS) ftpfspriv.h

#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC  	:= $(wildcard *.c)
EXCLUDE	:=
FTPFSSRC	:= $(filter-out $(EXCLUDE), $(ALLSRC))
FTPFSOBJ	:= $(FTPFSSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

FTPFSTARGETS : FTPFSDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

FTPFSDIR	:
	@echo Processing Common\Clib...

$(TARGNAME) : ftpfs.mak $(FTPFSOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

ftpfs.$(B_OBJEXT)		: $(FTPFSHEADERS) ftpfs.c
ftphandle.$(B_OBJEXT)		: $(FTPFSHEADERS) ftphandle.c
ftptxrx.$(B_OBJEXT)		: $(FTPFSHEADERS) ftptxrx.c
ftplogin.$(B_OBJEXT)		: $(FTPFSHEADERS) ftplogin.c
ftpinstance.$(B_OBJEXT)		: $(FTPFSHEADERS) ftpinstance.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: FTPFSDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)DLFTPFS		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(FTPFSSRC)				>>sources

