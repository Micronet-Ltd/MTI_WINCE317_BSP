#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the WinCE tools.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: dlpart.mak $
#	Revision 1.9  2010/10/24 17:02:11Z  garyp
#	Updated to link with dlwinutil.lib.
#	Revision 1.8  2009/06/25 02:07:13Z  garyp
#	Eliminated the B_PROJLIB workaround.
#	Revision 1.7  2009/02/09 08:01:48Z  garyp
#	Added the B_PROJLIB workaround which is necessary until such time as
#	the full WM branch merge is done.
#	Revision 1.6  2008/11/10 21:44:52Z  johnb
#	Remove B_INCLUDES default value.
#	Revision 1.5  2008/11/07 05:06:45Z  johnb
#	Cleaned up whitespace.  Cleaned up use of B_INCLUDES.
#	Revision 1.4  2008/11/06 21:21:41Z  johnb
#	Added default $(B_INCLUDES) in PB section
#	Revision 1.3  2008/11/05 21:35:25Z  johnb
#	Added $(B_DCLLIBPREFIX) to dcl libraries.  Changed _PROJECTOAKROOT to
#	_TARGETPLATROOT.  Added RELEASETYPE=PLATFORM.
#	Revision 1.2  2007/12/17 02:18:54Z  Garyp
#	Corrected link libraries.
#	Revision 1.1  2007/12/14 17:49:32Z  Garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

DCLPARTHEADERS = $(DCLHEADERS) $(P_ROOT)\os\$(P_OS)\include\dlcetools.h
DCLPARTINCLUDES =

COMMONDEPS = 	dlpart.mak 				\
		DCLLIBLST 				\
		$(B_RELDIR)\lib\dlostools.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\dlosutil.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\dlwinutil.$(B_LIBEXT)

ALLTARGETS = $(B_RELDIR)\dlpart.exe


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

DCLPARTTARGETS : DCLPARTDIR TOOLSETINIT $(ALLTARGETS)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

DCLPARTDIR :
	@echo Processing os\$(P_OS)\tools\dlpart...

$(B_RELDIR)\dlpart.exe : $(COMMONDEPS) dlpart.$(B_OBJEXT)
	echo /out:$(B_RELDIR)\dlpart.exe dlpart.$(B_OBJEXT) 	 >dlpart.lnk
	echo $(B_CESDKDIR)\coredll.lib				>>dlpart.lnk
	echo $(B_CESDKDIR)\corelibc.lib				>>dlpart.lnk
	echo $(B_CEOAKDIR)\storeapi.lib				>>dlpart.lnk
	echo $(B_RELDIR)\lib\dlostools.$(B_LIBEXT)		>>dlpart.lnk
	echo $(B_RELDIR)\lib\dlosutil.$(B_LIBEXT)		>>dlpart.lnk
	echo $(B_RELDIR)\lib\dlwinutil.$(B_LIBEXT)		>>dlpart.lnk
	$(B_COPY) dlpart.lnk + $(P_PROJDIR)\dcllib.lnk            dlpart.lnk
	$(B_LINK) /nodefaultlib	$(B_LFLAGS)			 @dlpart.lnk


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

dlpart.$(B_OBJEXT) : $(DCLPARTHEADERS)	dlpart.c


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

mswcepb	: DCLPARTDIR TOOLSETINIT $(PRODLIBS)
# NOTE:	Beware of using	standard escape	sequences (\c, \n, etc.)
#	when specifying	paths.	Make sure to double escape (\\c, etc).
	@echo #								  >sources
	@echo # This is an auto-generated file -- DO NOT EDIT		 >>sources
	@echo #								 >>sources
	@echo RELEASETYPE=PLATFORM					 >>sources
	@echo TARGETNAME=dlpart					 	 >>sources
	@echo TARGETTYPE=PROGRAM					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo   $$(B_CESDKDIR)\\coredll.lib				\>>sources
	@echo   $$(B_CESDKDIR)\\corelibc.lib				\>>sources
	@echo   $$(B_CEOAKDIR)\storeapi.lib				\>>sources
	$(B_COPY) /a sources + prodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo SOURCES=dlpart.c						 >>sources





