#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the Windows CE GUI edition
#       of the Datalight Shell.
#
#	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: windlshell.mak $
#	Revision 1.2.1.2  2012/04/16 18:58:27Z  johnb
#	Fixed to work with MSWCEPB toolset and to use correct libraries.
#	Revision 1.2.1.1  2009/03/14 23:00:48Z  johnb
#	Duplicate revision
#	Revision 1.2  2009/03/14 23:00:48Z  garyp
#	Merged from the v3.0 branch.
#	Revision 1.1.1.3  2009/03/14 23:00:48Z  garyp
#	Added some link libraries.
#	Revision 1.1.1.2  2008/10/28 23:44:51Z  garyp
#	Library dependencies updated.
#	Revision 1.1  2008/09/04 17:04:24Z  garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

# Extra include search path for code built in this directory
B_LOCALINC = ../../../win/include

OSTESTHEADERS =	$(DCLHEADERS)
OSTESTINCLUDES =

COMMONDEPS = 	windlshell.mak 				\
		DCLLIBLST 				\
		$(B_RELDIR)\lib\winviewappfw.res	\
		$(B_RELDIR)\lib\windlgedit.res		\
		$(B_RELDIR)\lib\dlostools.lib 		\
		$(B_RELDIR)\lib\dlosutil.lib		\
		$(B_RELDIR)\lib\dlwindebug.lib		\
		$(B_RELDIR)\lib\dlwintools.lib		\
		$(B_RELDIR)\lib\dlwinutil.lib		\
		$(B_RELDIR)\lib\dlwinutilgui.lib

ALLTARGETS =	$(B_RELDIR)\windlshell.exe


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

OSTESTTARGETS :	OSTESTDIR TOOLSETINIT $(ALLTARGETS)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

OSTESTDIR :
	@echo Processing os\$(P_OS)\Tests\WinDLShell...

$(B_RELDIR)\windlshell.exe : $(COMMONDEPS) windlshell.$(B_OBJEXT)
	echo /out:$(B_RELDIR)\windlshell.exe windlshell.$(B_OBJEXT)      >windlshell.lnk
	echo $(B_CESDKDIR)\coredll.lib					>>windlshell.lnk
	echo $(B_CESDKDIR)\corelibc.lib					>>windlshell.lnk
	echo $(B_CESDKDIR)\commdlg.lib					>>windlshell.lnk
	echo $(B_CEOAKDIR)\storeapi.lib					>>windlshell.lnk
	echo $(B_RELDIR)\lib\winviewappfw.res 				>>windlshell.lnk
	echo $(B_RELDIR)\lib\windlgedit.res 				>>windlshell.lnk
	echo $(B_RELDIR)\lib\dlostools.lib				>>windlshell.lnk
	echo $(B_RELDIR)\lib\dlosutil.lib				>>windlshell.lnk
	echo $(B_RELDIR)\lib\dlwindebug.lib 				>>windlshell.lnk
	echo $(B_RELDIR)\lib\dlwintools.lib				>>windlshell.lnk
	echo $(B_RELDIR)\lib\dlwinutil.lib 				>>windlshell.lnk
	echo $(B_RELDIR)\lib\dlwinutilgui.lib 				>>windlshell.lnk
	$(B_COPY) windlshell.lnk + $(P_PROJDIR)\dcllib.lnk        	  windlshell.lnk
	$(B_LINK) /nodefaultlib	$(B_LFLAGS)			 	 @windlshell.lnk


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

windlshell.$(B_OBJEXT) : $(OSTESTHEADERS) windlshell.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------
PRODLIBS = 	$(DCLLIBS)		\
		$(B_DCLLIBPREFIX)dlosutil.$(B_LIBEXT) \
		$(B_DCLLIBPREFIX)dlostools.$(B_LIBEXT) \
		$(B_DCLLIBPREFIX)dlwindebug.$(B_LIBEXT) \
		$(B_DCLLIBPREFIX)dlwintools.$(B_LIBEXT) \
		$(B_DCLLIBPREFIX)dlwinutil.$(B_LIBEXT) \
		$(B_DCLLIBPREFIX)dlwinutilgui.$(B_LIBEXT)

.PHONY: $(PRODLIBS)

$(PRODLIBS):
	@echo   $$(_TARGETPLATROOT)\lib\$$(_CPUINDPATH)\$(@F)	\>>prodlibs.lst

mswcepb	: OSTESTDIR TOOLSETINIT $(PRODLIBS)
	@echo #								  >sources
	@echo # This is an auto-generated file -- DO NOT EDIT		 >>sources
	@echo #								 >>sources
	@echo TARGETNAME=windlshell					 >>sources
	@echo TARGETTYPE=PROGRAM					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo RELEASETYPE=PLATFORM					 >>sources
	@echo   $$(B_CESDKDIR)\coredll.lib				\>>sources
	@echo   $$(B_CESDKDIR)\corelibc.lib				\>>sources
	@echo   $$(B_CESDKDIR)\commdlg.lib				\>>sources
	@echo   $$(B_CESDKDIR)\ntcompat.lib				\>>sources
	@echo   $$(B_CEOAKDIR)\storeapi.lib				\>>sources
	$(B_COPY) /a sources + prodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo SOURCES=windlshell.c					 >>sources




