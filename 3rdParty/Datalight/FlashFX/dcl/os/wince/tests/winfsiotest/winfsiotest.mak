#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the Windows CE tests.
#	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: winfsiotest.mak $
#	Revision 1.3  2012/02/24 21:31:54Z  johnb
#	Updated to build correctly in Platform Builder
#	Revision 1.2  2008/10/28 23:36:47Z  garyp
#	Merged from the v3.0 branch.
#	Revision 1.1.1.2  2008/10/28 23:36:47Z  garyp
#	Library dependencies updated.
#	Revision 1.1  2008/08/31 17:48:02Z  garyp
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

COMMONDEPS = 	winfsiotest.mak 			\
		DCLLIBLST 				\
		$(B_RELDIR)\lib\winviewappfw.res	\
		$(B_RELDIR)\lib\windlgedit.res		\
		$(B_RELDIR)\lib\dlosutil.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\dlwindebug.lib		\
		$(B_RELDIR)\lib\dlwinutil.lib		\
		$(B_RELDIR)\lib\dlwinutilgui.lib

ALLTARGETS =	$(B_RELDIR)\winfsiotest.exe


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
	@echo Processing os\$(P_OS)\Tests\WinFSIOTest...

$(B_RELDIR)\winfsiotest.exe : $(COMMONDEPS) winfsiotest.$(B_OBJEXT)
	echo /out:$(B_RELDIR)\winfsiotest.exe winfsiotest.$(B_OBJEXT)    >winfsiotest.lnk
	echo $(B_CESDKDIR)\coredll.lib					>>winfsiotest.lnk
	echo $(B_CESDKDIR)\corelibc.lib					>>winfsiotest.lnk
	echo $(B_CESDKDIR)\commdlg.lib					>>winfsiotest.lnk
	echo $(B_CEOAKDIR)\storeapi.lib					>>winfsiotest.lnk
	echo $(B_RELDIR)\lib\winviewappfw.res 				>>winfsiotest.lnk
	echo $(B_RELDIR)\lib\windlgedit.res 				>>winfsiotest.lnk
	echo $(B_RELDIR)\lib\dlwindebug.lib 				>>winfsiotest.lnk
	echo $(B_RELDIR)\lib\dlwinutil.lib 				>>winfsiotest.lnk
	echo $(B_RELDIR)\lib\dlwinutilgui.lib 				>>winfsiotest.lnk
	echo $(B_RELDIR)\lib\dlosutil.$(B_LIBEXT)			>>winfsiotest.lnk
	$(B_COPY) winfsiotest.lnk + $(P_PROJDIR)\dcllib.lnk        	  winfsiotest.lnk
	$(B_LINK) /nodefaultlib	$(B_LFLAGS)			 	 @winfsiotest.lnk


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

winfsiotest.$(B_OBJEXT) : $(OSTESTHEADERS) winfsiotest.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------
PRODLIBS = 	$(DCLLIBS)		\
		$(B_DCLLIBPREFIX)dlosutil.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlwinutil.$(B_LIBEXT)

.PHONY: $(PRODLIBS)

$(PRODLIBS):
	@echo   $$(_TARGETPLATROOT)\lib\$$(_CPUINDPATH)\$(@F)	\>>prodlibs.lst

mswcepb	: OSTESTDIR TOOLSETINIT $(PRODLIBS)
	@echo #								  >sources
	@echo # This is an auto-generated file -- DO NOT EDIT		 >>sources
	@echo #								 >>sources
	@echo RELEASETYPE=PLATFORM						 >>sources
	@echo TARGETNAME=winfsiotest					 >>sources
	@echo TARGETTYPE=PROGRAM					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo   $$(B_CESDKDIR)\coredll.lib				\>>sources
	@echo   $$(B_CESDKDIR)\ntcompat.lib				\>>sources
	@echo   $$(B_CEOAKDIR)\storeapi.lib				\>>sources
	$(B_COPY) /a sources + prodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo SOURCES=winfsiotest.c					 >>sources




