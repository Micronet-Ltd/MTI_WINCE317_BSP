#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DLWINUTILGUI library.
#	It is designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: utilgui.mak $
#	Revision 1.9  2012/04/16 18:50:47Z  johnb
#	Updated to delete the combined RC file before building.
#	Revision 1.8  2012/04/12 21:44:22Z  johnb
#	Corrected build problem with MSWCEPB toolset.
#	Revision 1.7  2012/04/12 20:18:47Z  johnb
#	Updates for MSWCEPB toolset.
#	Revision 1.6  2012/03/01 05:15:21Z  johnb
#	Corrected build problems with mswce and mswcepb toolsets.
#	Revision 1.5  2012/02/28 20:41:14Z  johnb
#	Created RC file which includes each of the other .RC files in 
#	the directory and added this RC file to the list of PB sources.
#	Revision 1.4  2012/02/16 21:48:34Z  johnb
#	Added "RELEASETYPE=PLATFORM" to mswcepb target
#	Revision 1.3  2009/07/11 03:07:54Z  garyp
#	Added windlgposition.c and winsettings.c.
#	Revision 1.2  2008/10/28 18:21:53Z  garyp
#	Merged from the v3.0 branch.
#	Revision 1.1.1.4  2008/10/28 18:21:53Z  garyp
#	Removed an obsolete exclude module.
#	Revision 1.1.1.3  2008/08/30 19:01:16Z  garyp
#	Added wincommoncontrols.c.
#	Revision 1.1.1.2  2008/07/31 03:27:01Z  garyp
#	Added winhelpabout.c.
#	Revision 1.1  2008/07/26 20:18:22Z  garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Define an extra general include path
B_LOCALINC = ..\include

TARGFILE = dlwinutilgui.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)
PROJRC = pbproj.rc

UTILGUIHEADERS = $(DCLHEADERS)

#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c *.rc)
EXCLUDE     	:=
UTILGUISRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
UTILGUISRC	:= $(filter-out $(PROJRC), $(UTILGUISRC))
UTILGUIOBJ	:= $(UTILGUISRC:.c=.$(B_OBJEXT))
UTILGUIRES	:= $(UTILGUISRC:.rc=.res)


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

UTILGUITARGETS : UTILGUIDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

UTILGUIDIR :
	@echo Processing os\Win\UtilGUI...
	@if exist $(PROJRC) @del $(PROJRC) 

$(TARGNAME) : utilgui.mak $(UTILGUIOBJ) $(UTILGUIRES)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)
	$(B_COPY) *.res $(B_RELDIR)\lib\.


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

wincommoncontrols.$(B_OBJEXT) 	: $(UTILGUIHEADERS) wincommoncontrols.c
windlgedit.$(B_OBJEXT)   	: $(UTILGUIHEADERS) windlgedit.c  winutilgui.rh
windlgedit.res           	: $(UTILGUIHEADERS) windlgedit.rc winutilgui.rh
windlgposition.$(B_OBJEXT)   	: $(UTILGUIHEADERS) windlgposition.c
winhelpabout.$(B_OBJEXT) 	: $(UTILGUIHEADERS) winhelpabout.c
winscrollbar.$(B_OBJEXT) 	: $(UTILGUIHEADERS) winscrollbar.c  ..\include\winscrollapi.h
winsettings.$(B_OBJEXT)   	: $(UTILGUIHEADERS) winsettings.c  winutilgui.rh
winsettings.res           	: $(UTILGUIHEADERS) winsettings.rc winutilgui.rh
winviewappfw.$(B_OBJEXT) 	: $(UTILGUIHEADERS) winviewappfw.c  ..\include\winviewappfw.h ..\include\winviewappfw.rh winutilgui.rh
winviewappfw.res         	: $(UTILGUIHEADERS) winviewappfw.rc ..\include\winviewappfw.rh
winviewcore.$(B_OBJEXT)  	: $(UTILGUIHEADERS) winviewcore.c   ..\include\winviewapi.h
winviewobj.$(B_OBJEXT)	 	: $(UTILGUIHEADERS) winviewobj.c    ..\include\winviewapi.h ..\include\winscrollapi.h


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

PBPROJRC	:= $(wildcard *.rc)
PBPROJSRC	:= $(wildcard *.c)

PBPROJRC	:= $(filter-out $(PROJRC), $(PBPROJRC))

.PHONY: $(PBPROJRC)
$(PBPROJRC):
	@echo #include "$(@F)" >>$(PROJRC)

.PHONY: $(PBPROJSRC)
$(PBPROJSRC):
	@echo $(@F)

mswcepb	: UTILGUIDIR TOOLSETINIT $(PBPROJSRC) $(PBPROJRC)
	@echo #							 >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)DLWINUTILGUI		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(PBPROJSRC) $(PROJRC)			>>sources

