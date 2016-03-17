#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLOSTOOLS library.  It
#	is designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: tools.mak $
#	Revision 1.7  2010/11/09 21:06:15Z  garyp
#	Added dlcevol.c.
#	Revision 1.6  2009/02/08 01:28:27Z  garyp
#	Added a local include path.
#	Revision 1.5  2008/11/10 21:44:23Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.4  2008/11/07 05:05:34Z  johnb
#	Cleaned up whitespace. 	Cleaned up use of B_INCLUDES.
#	Revision 1.3  2008/11/06 21:22:03Z  johnb
#	Added default $(B_INCLUDES) in PB section.
#	Revision 1.2  2008/11/03 20:22:18Z  johnb
#	Added RELEASETYPE=PLATFORM.
#	Revision 1.1  2007/11/29 17:56:52Z  Garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = dlostools.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

# Extra include search path for code built in this directory
B_LOCALINC = ../../../win/include


#B_LOCALINC = ../../../win/include $(_PUBLICROOT)/wpc/cesysgen/sdk/inc

OSTOOLSHEADERS = $(DCLHEADERS) $(P_ROOT)\os\$(P_OS)\include\dlceutil.h


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
OSTOOLSSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
OSTOOLSOBJ	:= $(OSTOOLSSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

OSTOOLSTARGETS : OSTOOLSDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

OSTOOLSDIR	:
	@echo Processing os\wince\common\tools...

$(TARGNAME) : tools.mak $(OSTOOLSOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

dlcedriverload.$(B_OBJEXT)	: $(OSTOOLSHEADERS) dlcedriverload.c
dlcepart.$(B_OBJEXT) 		: $(OSTOOLSHEADERS) dlcepart.c
dlceshell.$(B_OBJEXT)  		: $(OSTOOLSHEADERS) dlceshell.c
dlceshellcmd.$(B_OBJEXT)  	: $(OSTOOLSHEADERS) dlceshellcmd.c
dlcevol.$(B_OBJEXT) 		: $(OSTOOLSHEADERS) dlcevol.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: OSTOOLSDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)DLOSTOOLS		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(OSTOOLSSRC)				>>sources

