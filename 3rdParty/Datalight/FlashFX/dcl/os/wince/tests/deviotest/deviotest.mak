#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the Windows CE tests.
#	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: deviotest.mak $
#	Revision 1.2  2010/10/24 16:58:48Z  garyp
#	Updated to link with dlwinutil.lib.
#	Revision 1.1  2009/09/14 15:40:14Z  garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

OSTESTHEADERS =	$(DCLHEADERS)
OSTESTINCLUDES =

COMMONDEPS = 	deviotest.mak 				\
		DCLLIBLST 				\
		$(B_RELDIR)\lib\dlosutil.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\dlwinutil.$(B_LIBEXT)

ALLTARGETS =	$(B_RELDIR)\deviotest.exe


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
	@echo Processing os\$(P_OS)\Tests\DevIOTest...

$(B_RELDIR)\deviotest.exe : $(COMMONDEPS) deviotest.$(B_OBJEXT)
	echo /out:$(B_RELDIR)\deviotest.exe deviotest.$(B_OBJEXT) 	 >deviotest.lnk
	echo $(B_CESDKDIR)\coredll.lib					>>deviotest.lnk
	echo $(B_CESDKDIR)\corelibc.lib					>>deviotest.lnk
	echo $(B_CEOAKDIR)\storeapi.lib					>>deviotest.lnk
	echo $(B_RELDIR)\lib\dlosutil.$(B_LIBEXT)			>>deviotest.lnk
	echo $(B_RELDIR)\lib\dlwinutil.$(B_LIBEXT)			>>deviotest.lnk
	$(B_COPY) deviotest.lnk + $(P_PROJDIR)\dcllib.lnk        	  deviotest.lnk
	$(B_LINK) /nodefaultlib	$(B_LFLAGS)				 @deviotest.lnk


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

deviotest.$(B_OBJEXT) : $(OSTESTHEADERS) deviotest.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------
PRODLIBS = 	$(DCLLIBS)				\
		$(B_DCLLIBPREFIX)dlosutil.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlwinutil.$(B_LIBEXT)

.PHONY: $(PRODLIBS)

$(PRODLIBS):
	@echo   $$(_TARGETPLATROOT)\lib\$$(_CPUINDPATH)\$(@F)	\>>prodlibs.lst

mswcepb	: OSTESTDIR TOOLSETINIT $(PRODLIBS)
# NOTE:	Beware of using	standard escape	sequences (\c, \n, etc.)
#	when specifying	paths.	Make sure to double escape (\\c, etc).
	@echo #								  >sources
	@echo # This is an auto-generated file -- DO NOT EDIT		 >>sources
	@echo #								 >>sources
	@echo RELEASETYPE=PLATFORM					 >>sources
	@echo TARGETNAME=deviotest					 >>sources
	@echo TARGETTYPE=PROGRAM					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo   $$(B_CESDKDIR)\\coredll.lib				\>>sources
	@echo   $$(B_CESDKDIR)\\ntcompat.lib				\>>sources
	@echo   $$(B_CEOAKDIR)\storeapi.lib				\>>sources
	$(B_COPY) /a sources + prodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo SOURCES=deviotest.c					 >>sources




