#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXVBFUTL library.  It
#	is designed for	use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: util.mak $
#	Revision 1.3  2010/11/24 20:11:50Z  garyp
#	Added an include path to B_LOCALINC.
#	Revision 1.2  2008/11/07 15:05:19Z  johnb
#	Cleaned up whitespace alignment.
#	Revision 1.1  2006/05/08 18:11:58Z  Garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Define extra general include paths
B_LOCALINC = $(P_ROOT)\alloc\vbf\include $(P_ROOT)\fmsl\include

VBFUTLHEADERS = $(FXHEADERS) $(P_ROOT)\include\vbf.h

TARGNAMEVBFUTL = $(B_RELDIR)\lib\fxvbfutl.$(B_LIBEXT)


#--------------------------------------------------------------------
#	Build a list of all the C source files, excluding
#	those in the EXCLUDE list, and from that generate
#	the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
VBFUTLSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
VBFUTLOBJ	:= $(VBFUTLSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

VBFUTLTARGETS : VBFUTLDIR TOOLSETINIT $(TARGNAMEVBFUTL)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

VBFUTLDIR :
	@echo Processing Alloc\VBF\Util...

$(TARGNAMEVBFUTL) : util.mak $(VBFUTLOBJ)
	if exist fxvbfutl.$(B_LIBEXT) del fxvbfutl.$(B_LIBEXT)
	$(B_BUILDLIB) fxvbfutl.$(B_LIBEXT) *.$(B_OBJEXT)
	$(B_COPY) fxvbfutl.$(B_LIBEXT) $(TARGNAMEVBFUTL)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

vbfdump.$(B_OBJEXT)  : $(VBFUTLHEADERS) vbfdump.c
vbfstatd.$(B_OBJEXT) : $(VBFUTLHEADERS) vbfstatd.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: VBFUTLDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=FXVBFUTL				>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(VBFUTLSRC)				>>sources
	@echo B_INCLUDES=$(B_LOCALINC);				>>sources


