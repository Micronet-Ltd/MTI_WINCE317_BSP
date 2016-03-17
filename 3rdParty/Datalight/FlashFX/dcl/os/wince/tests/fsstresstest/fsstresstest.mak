#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the Windows CE tests.
#	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: fsstresstest.mak $
#	Revision 1.2  2010/10/24 17:00:41Z  garyp
#	Updated to link with dlwinutil.lib.
#	Revision 1.1  2009/11/04 02:27:58Z  garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

OSTESTHEADERS =	$(DCLHEADERS)
OSTESTINCLUDES =

COMMONDEPS = 	fsstresstest.mak 			\
		DCLLIBLST 				\
		$(B_RELDIR)\lib\dlosutil.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\dlwinutil.$(B_LIBEXT)

ALLTARGETS =	$(B_RELDIR)\fsstresstest.exe


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
	@echo Processing os\$(P_OS)\Tests\FSStressTest...

$(B_RELDIR)\fsstresstest.exe : $(COMMONDEPS) fsstresstest.$(B_OBJEXT)
	echo /out:$(B_RELDIR)\fsstresstest.exe fsstresstest.$(B_OBJEXT)  >fsstresstest.lnk
	echo $(B_CESDKDIR)\coredll.lib					>>fsstresstest.lnk
	echo $(B_CESDKDIR)\corelibc.lib					>>fsstresstest.lnk
	echo $(B_CEOAKDIR)\storeapi.lib					>>fsstresstest.lnk
	echo $(B_RELDIR)\lib\dlosutil.$(B_LIBEXT)			>>fsstresstest.lnk
	echo $(B_RELDIR)\lib\dlwinutil.$(B_LIBEXT)			>>fsstresstest.lnk
	$(B_COPY) fsstresstest.lnk + $(P_PROJDIR)\dcllib.lnk        	  fsstresstest.lnk
	$(B_LINK) /nodefaultlib	$(B_LFLAGS)				 @fsstresstest.lnk


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

fsstresstest.$(B_OBJEXT) : $(OSTESTHEADERS) fsstresstest.c


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
	@echo TARGETNAME=fsstresstest					 >>sources
	@echo TARGETTYPE=PROGRAM					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo   $$(B_CESDKDIR)\\coredll.lib				\>>sources
	@echo   $$(B_CESDKDIR)\\ntcompat.lib				\>>sources
	@echo   $$(B_CEOAKDIR)\storeapi.lib				\>>sources
	$(B_COPY) /a sources + prodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo SOURCES=fsstresstest.c					 >>sources




