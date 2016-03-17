#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of LOADDRV.EXE.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: loaddrv.mak $
#	Revision 1.9  2010/10/24 17:03:53Z  garyp
#	Updated to link with dlwinutil.lib.
#	Revision 1.8  2009/06/25 02:07:12Z  garyp
#	Eliminated the B_PROJLIB workaround.
#	Revision 1.7  2009/02/09 08:00:43Z  garyp
#	Added the B_PROJLIB workaround which is necessary until such time as
#	the full WM branch merge is done.
#	Revision 1.6  2008/11/10 21:45:00Z  johnb
#	Remove B_INCLUDES default value.
#	Revision 1.5  2008/11/07 05:07:09Z  johnb
#	Cleaned up whitespace.  Cleaned up use of B_INCLUDES.
#	Revision 1.4  2008/11/06 21:21:33Z  johnb
#	Added default $(B_INCLUDES) in PB section.
#	Revision 1.3  2008/11/05 21:36:30Z  johnb
#	Added $(B_DCLLIBPREFIX) to dcl libraries.  Changed _PROJECTOAKROOT to
#	_TARGETPLATROOT.  Added RELEASETYPE=PLATFORM.
#	Revision 1.2  2007/12/04 22:45:08Z  Garyp
#	Modified to use the new generalized CE output redirection functions.
#	Updated to use the refactored "CE driver load" functionality.
#	Revision 1.1  2007/01/21 15:37:42Z  Garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

ALLTARGETS =	$(B_RELDIR)\loaddrv.exe

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

LOADDRVHEADERS = $(DCLHEADERS)

COMMONDEPS = 	loaddrv.mak 				\
		DCLLIBLST				\
		$(B_RELDIR)\lib\dlostools.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\dlosutil.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\dlwinutil.$(B_LIBEXT)


#--------------------------------------------------------------------
#	Build a list of all the C source files, excluding thse in
#	the EXCLUDE list, and then generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
LOADDRVSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
LOADDRVOBJ	:= $(LOADDRVSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

LOADDRVTARGETS : LOADDRVDIR TOOLSETINIT	$(ALLTARGETS)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

LOADDRVDIR :
	@echo Processing os\wince\tools\loaddrv...

$(B_RELDIR)\loaddrv.exe	: $(COMMONDEPS) loaddrv.$(B_OBJEXT)
	echo /out:$(B_RELDIR)\loaddrv.exe loaddrv.$(B_OBJEXT)	 >loaddrv.lnk
	echo $(B_CESDKDIR)\coredll.lib				>>loaddrv.lnk
	echo $(B_CESDKDIR)\corelibc.lib				>>loaddrv.lnk
	echo $(B_RELDIR)\lib\dlostools.$(B_LIBEXT)		>>loaddrv.lnk
	echo $(B_RELDIR)\lib\dlosutil.$(B_LIBEXT)		>>loaddrv.lnk
	echo $(B_RELDIR)\lib\dlwinutil.$(B_LIBEXT)		>>loaddrv.lnk
	$(B_COPY) loaddrv.lnk + $(P_PROJDIR)\dcllib.lnk           loaddrv.lnk
	$(B_LINK) /nodefaultlib	$(B_LFLAGS) 			 @loaddrv.lnk


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

loaddrv.$(B_OBJEXT)	: $(LOADDRVHEADERS) loaddrv.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------
PRODLIBS = 	$(DCLLIBS)				\
		$(B_DCLLIBPREFIX)dlostools.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlosutil.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlwinutil.$(B_LIBEXT)

.PHONY: $(PRODLIBS)

$(PRODLIBS):
	@echo   $$(_TARGETPLATROOT)\lib\$$(_CPUINDPATH)\$(@F)	\>>prodlibs.lst

mswcepb	: LOADDRVDIR TOOLSETINIT $(PRODLIBS)
# NOTE:	Beware of using	standard escape	sequences (\c, \n, etc.)
#	when specifying	paths.	Make sure to double escape (\\c, etc).
	@echo #								  >sources
	@echo # This is an auto-generated file -- DO NOT EDIT		 >>sources
	@echo #								 >>sources
	@echo RELEASETYPE=PLATFORM					 >>sources
	@echo TARGETNAME=LOADDRV					 >>sources
	@echo TARGETTYPE=PROGRAM					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo 	$$(B_CESDKDIR)\coredll.lib				\>>sources
	@echo 	$$(B_CESDKDIR)\ntcompat.lib				\>>sources
	$(B_COPY) /a sources + prodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo SOURCES=$(LOADDRVSRC)					 >>sources


