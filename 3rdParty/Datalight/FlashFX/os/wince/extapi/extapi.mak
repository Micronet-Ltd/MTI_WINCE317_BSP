#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXOEAPI library.	It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: extapi.mak $
#	Revision 1.4  2008/11/10 21:46:21Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.3  2008/11/07 15:11:21Z  johnb
#	Cleaned up whitespace alignment
#	Cleaned up use of B_INCLUDES
#	Revision 1.2  2007/11/26 03:58:25Z  Garyp
#	Eliminated extuser.c.
#	Revision 1.1  2005/10/01 09:35:18Z  Pauli
#	Initial revision
#	Revision 1.12  2005/01/17 00:59:20Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled in the traverse process.
#	Revision 1.11  2004/07/07 03:04:16Z  GaryP
#	Eliminated the module header.
#	Revision 1.10  2004/07/01 18:44:29Z  GaryP
#	Updated to use new P_ and B_ build symbols.
#	Revision 1.9  2004/01/13 04:49:36Z  garys
#	Merge from FlashFXMT
#	Revision 1.8.1.3  2004/01/13 04:49:36  garyp
#	Renamed	apireq.c to extreq.c.
#	Revision 1.8.1.2  2003/11/14 19:05:36Z	garyp
#	Added APIREQ.C.	 Eliminated EXTAPI.C.
#	Revision 1.8  2003/04/15 19:40:40Z  garyp
#	Eliminated an obsolete directive.
#	Revision 1.7  2003/03/08 20:36:16Z  garyp
#	Updated	to use the B_LIBEXT setting.
#	Revision 1.6  2002/11/23 00:29:32Z  garyp
#	Updated	to use $(FXHEADERS).
#	Revision 1.5  2002/11/15 21:22:48Z  garyp
#	Corrected the previous rev.
#	Revision 1.4  2002/11/15 21:12:14Z  garyp
#	Changed	the CE PB ToolSet name to "mswcepb".
#	Revision 1.3  2002/11/07 10:14:22Z  garyp
#	Updated	to build CEPB SOURCES files.
#	Revision 1.2  2002/10/28 23:15:02Z  garyp
#	Modified the dependency	calculation process.  Added the	module header.
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

TARGFILE = fxoeapi.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

EXTAPIHEADERS =	$(FXHEADERS)


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
EXTAPISRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
EXTAPIOBJ	:= $(EXTAPISRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

EXTAPITARGETS :	EXTAPIDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

EXTAPIDIR :
	@echo Processing os\$(P_OS)\Extapi...

$(TARGNAME) : extapi.mak $(EXTAPIOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

extreq.$(B_OBJEXT)     : $(EXTAPIHEADERS) extreq.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

mswcepb	: EXTAPIDIR TOOLSETINIT
	@echo #							 >sources
	@echo #This is an auto-generated file -- DO NOT	EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=FXOEAPI				>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(EXTAPISRC)				>>sources


