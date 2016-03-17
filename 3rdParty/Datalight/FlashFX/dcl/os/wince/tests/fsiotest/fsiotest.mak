#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the Windows CE tests.
#	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: fsiotest.mak $
#	Revision 1.11  2010/10/24 16:59:45Z  garyp
#	Updated to link with dlwinutil.lib.
#	Revision 1.10  2009/06/25 02:07:12Z  garyp
#	Eliminated the B_PROJLIB workaround.
#	Revision 1.9  2009/02/09 07:56:12Z  garyp
#	Added the B_PROJLIB workaround which is necessary until such time as
#	the full WM branch merge is done.
#	Revision 1.8  2008/11/10 21:44:43Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.7  2008/11/07 05:06:20Z  johnb
#	Cleaned up whitespace.  Cleaned up use of B_INCLUDES.
#	Revision 1.6  2008/11/06 21:21:49Z  johnb
#	Added default $(B_INCLUDES) in PB section
#	Revision 1.5  2008/11/05 21:35:04Z  johnb
#	Added $(B_DCLLIBPREFIX) to dcl libraries.  Changed _PROJECTOAKROOT to
#	_TARGETPLATROOT.  Added RELEASETYPE=PLATFORM.
#	Revision 1.4  2007/12/14 00:43:19Z  Garyp
#	Modified to use the new generalized CE output redirection functions.
#	Revision 1.3  2006/12/14 00:11:39Z  Garyp
#	Updated to link with storeapi.lib.
#	Revision 1.2  2006/08/25 22:07:00Z  Garyp
#	Fixed to use the proper copy command abstraction.
#	Revision 1.1  2005/10/03 13:10:06Z  Pauli
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

OSTESTHEADERS =	$(DCLHEADERS)
OSTESTINCLUDES =

COMMONDEPS = 	fsiotest.mak 				\
		DCLLIBLST 				\
		$(B_RELDIR)\lib\dlosutil.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\dlwinutil.$(B_LIBEXT)

ALLTARGETS =	$(B_RELDIR)\fsiotest.exe


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
	@echo Processing os\$(P_OS)\Tests\FSIOTest...

$(B_RELDIR)\fsiotest.exe : $(COMMONDEPS) fsiotest.$(B_OBJEXT)
	echo /out:$(B_RELDIR)\fsiotest.exe fsiotest.$(B_OBJEXT)  >fsiotest.lnk
	echo $(B_CESDKDIR)\coredll.lib				>>fsiotest.lnk
	echo $(B_CESDKDIR)\corelibc.lib				>>fsiotest.lnk
	echo $(B_CEOAKDIR)\storeapi.lib				>>fsiotest.lnk
	echo $(B_RELDIR)\lib\dlosutil.$(B_LIBEXT)		>>fsiotest.lnk
	echo $(B_RELDIR)\lib\dlwinutil.$(B_LIBEXT)		>>fsiotest.lnk
	$(B_COPY) fsiotest.lnk + $(P_PROJDIR)\dcllib.lnk          fsiotest.lnk
	$(B_LINK) /nodefaultlib	$(B_LFLAGS)			 @fsiotest.lnk


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

fsiotest.$(B_OBJEXT) : $(OSTESTHEADERS) fsiotest.c


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
	@echo TARGETNAME=fsiotest					 >>sources
	@echo TARGETTYPE=PROGRAM					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo   $$(B_CESDKDIR)\\coredll.lib				\>>sources
	@echo   $$(B_CESDKDIR)\\ntcompat.lib				\>>sources
	@echo   $$(B_CEOAKDIR)\storeapi.lib				\>>sources
	$(B_COPY) /a sources + prodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo SOURCES=fsiotest.c					 >>sources




