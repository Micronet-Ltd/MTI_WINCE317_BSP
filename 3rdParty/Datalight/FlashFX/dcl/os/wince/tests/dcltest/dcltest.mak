#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the Windows CE tests.
#	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: dcltest.mak $
#	Revision 1.12  2010/10/24 16:57:45Z  garyp
#	Updated to link with dlwinutil.lib.
#	Revision 1.11  2009/06/25 02:07:13Z  garyp
#	Eliminated the B_PROJLIB workaround.
#	Revision 1.10  2009/02/09 07:53:04Z  garyp
#	Added the B_PROJLIB workaround which is necessary until such time as
#	the full WM branch merge is done.
#	Revision 1.9  2008/11/10 21:44:38Z  johnb
#	Remove B_INCLUDES default value.
#	Revision 1.8  2008/11/07 05:06:07Z  johnb
#	Cleaned up whitespace.  Cleaned up use of B_INCLUDES.
#	Revision 1.7  2008/11/06 21:21:54Z  johnb
#	Added default $(B_INCLUDES) in PB section.
#	Revision 1.6  2008/11/05 21:34:57Z  johnb
#	Added $(B_DCLLIBPREFIX) to dcl libraries.  Changed _PROJECTOAKROOT to
#	_TARGETPLATROOT.  Added RELEASETYPE=PLATFORM.
#	Revision 1.5  2008/07/23 05:59:43Z  keithg
#	Clarified comments with professional customer facing terms.
#	Revision 1.4  2008/04/17 23:49:05Z  jeremys
#	Added storeapi.lib to the dcltest library list which is needed for
#	disk APIs.
#	Revision 1.3  2007/12/14 00:43:19Z  Garyp
#	Modified to use the new generalized CE output redirection functions.
#	Revision 1.2  2006/08/25 21:53:48Z  Garyp
#	Fixed to use the proper copy command abstraction.
#	Revision 1.1  2005/10/01 09:35:12Z  Pauli
#	Initial revision
#	Revision 1.2  2005/08/31 22:56:14Z  Garyp
#	Fixed an invalid dependency.
#	Revision 1.1  2005/07/16 05:11:00Z  pauli
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

OSTESTHEADERS =	$(DCLHEADERS)
OSTESTINCLUDES =

COMMONDEPS = 	dcltest.mak 				\
		DCLLIBLST 				\
		$(B_RELDIR)\lib\dlosutil.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\dlwinutil.$(B_LIBEXT)

ALLTARGETS = $(B_RELDIR)\dcltest.exe


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
	@echo Processing os\$(P_OS)\Tests\DCLTest...

$(B_RELDIR)\dcltest.exe : $(COMMONDEPS) dcltest.$(B_OBJEXT)
	echo /out:$(B_RELDIR)\dcltest.exe dcltest.$(B_OBJEXT)    >dcltest.lnk
	echo $(B_CESDKDIR)\coredll.lib				>>dcltest.lnk
	echo $(B_CESDKDIR)\corelibc.lib				>>dcltest.lnk
	echo $(B_CEOAKDIR)\storeapi.lib				>>dcltest.lnk
	echo $(B_RELDIR)\lib\dlosutil.$(B_LIBEXT)		>>dcltest.lnk
	echo $(B_RELDIR)\lib\dlwinutil.$(B_LIBEXT)		>>dcltest.lnk
	$(B_COPY) dcltest.lnk + $(P_PROJDIR)\dcllib.lnk           dcltest.lnk
	$(B_LINK) /nodefaultlib	$(B_LFLAGS)			 @dcltest.lnk


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

dcltest.$(B_OBJEXT) : $(OSTESTHEADERS)	dcltest.c


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
	@echo TARGETNAME=dcltest					 >>sources
	@echo TARGETTYPE=PROGRAM					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo   $$(B_CESDKDIR)\\coredll.lib				\>>sources
	@echo   $$(B_CESDKDIR)\\ntcompat.lib				\>>sources
	$(B_COPY) /a sources + prodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo SOURCES=dcltest.c						 >>sources




