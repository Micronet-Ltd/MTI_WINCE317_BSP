#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXBBM library.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: bbm.mak $
#	Revision 1.8  2009/11/09 05:34:42Z  garyp
#	Fixed a broken dependency.
#	Revision 1.7  2009/03/24 17:38:40Z  keithg
#	Fixed typographical error
#	Revision 1.6  2009/01/19 03:55:09Z  keithg
#	Updated for BBM v5.
#	Revision 1.2  2008/11/07 15:02:53Z  johnb
#	Cleaned up whitespace alignment
#	Revision 1.1  2008/05/22 20:57:00Z  keithg
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Define an extra general include path
B_LOCALINC = . 

TARGFILE = fxbbm5.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

BBMHEADERS = $(FXHEADERS) bbm_internals.h bbm.mak  


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
BBMSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
BBMOBJ		:= $(BBMSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

BBMTARGETS : BBMDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include $(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

BBMDIR :
	@echo Processing FMSL\BBM...

$(TARGNAME) : $(BBMOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

$(*.$(B_OBJEXT))     : $(*.c) $(BBMHEADERS)


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: BBMDIR TOOLSETINIT
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=FXBBM5					>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(BBMSRC)					>>sources
	@echo B_INCLUDES=$(B_LOCALINC);				>>sources





