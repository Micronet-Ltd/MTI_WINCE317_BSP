#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the Windows CE tests.
#	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: vbftest.mak $
#	Revision 1.8  2012/02/17 16:19:14Z  johnb
#	Added dlwinutil and fxoeapi to PRODLIBS for mswcepb toolset
#	Revision 1.7  2009/02/08 16:20:51Z  garyp
#	Merged from the v4.0 branch.  Updated to link with flashfx.lib directly, rather
#	than explicitly linking with all the "app" libs.
#	Revision 1.6  2008/11/10 21:46:51Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.5  2008/11/07 15:23:31Z  johnb
#	Cleaned up whitespace and modified platform builder support
#	Revision 1.4  2007/12/14 21:35:34Z  Garyp
#	Updated to use the new DCL functions to support output redirection.
#	Revision 1.3  2007/03/08 19:41:12Z  keithg
#	Added storeapi library to link line (needed for CE 5.00)
#	Revision 1.2  2006/08/25 22:17:22Z  Garyp
#	Fixed to use the proper copy command abstraction.
#	Revision 1.1  2005/10/01 09:36:40Z  Pauli
#	Initial revision
#	Revision 1.2  2005/08/30 07:34:40Z  Garyp
#	Modified so that DCLLIBS is specified separately from the other product
#	libraries.
#	Revision 1.1  2005/03/12 01:53:24Z  pauli
#	Initial revision
#	Revision 1.16  2005/03/11 23:53:23Z  GaryP
#	Updated to use new mechanisms for generating the MSWCEPB link libraries.
#	Revision 1.15  2005/02/27 03:43:02Z  GaryP
#	Updated to use new CE settings that work better with CE 5.
#	Revision 1.14  2005/01/17 00:59:07Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled in the traverse process.
#	Revision 1.13  2004/09/27 19:16:00Z  tonyq
#	Updated for Windows CE 5.0 Platform builder (command line build)
#	Revision 1.12  2004/09/23 21:13:50Z  tonyq
#	Updated to support building from the command line under Windows CE 5.0
#	(with the MSWCE4 toolset).
#	Revision 1.11  2004/07/07 03:05:26Z  GaryP
#	Eliminated the module header.
#	Revision 1.10  2004/07/01 18:44:53Z  GaryP
#	Updated to use new P_ and B_ build symbols.
#	Revision 1.9  2003/11/17 19:54:26Z  garys
#	Merge from FlashFXMT
#	Revision 1.8.1.2  2003/11/17 19:54:26  garyp
#	Modified to link with FXEXTAPI.LIB.
#	Revision 1.8  2003/05/22 23:29:03Z  tonyq
#	Update to work with MSWCE4 tools
#	Revision 1.7  2003/04/15 17:34:24Z  garyp
#	Updated	to use the Tools Framework.
#	Revision 1.6  2003/03/08 20:46:12Z  garyp
#	Updated	to use the B_LIBEXT setting.
#	Revision 1.5  2002/11/23 00:32:16Z  garyp
#	Updated	to use $(FXHEADERS).
#	Revision 1.4  2002/11/15 21:22:48Z  garyp
#	Corrected the previous rev.
#	Revision 1.3  2002/11/15 21:12:14Z  garyp
#	Changed	the CE PB ToolSet name to "mswcepb".
#	Revision 1.2  2002/11/14 00:00:40Z  garyp
#	Added comments to the SOURCES generation section.
#	Revision 1.1  2002/11/13 07:13:50Z  garyp
#	Initial	revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

ALLTARGETS =	$(B_RELDIR)\vbftest.exe

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

OSVBFTESTHEADERS = $(FXHEADERS)
OSVBFTESTINCLUDES =

COMMONDEPS = 	vbftest.mak $(B_RELDIR)\flashfx.$(B_LIBEXT)


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

OSVBFTESTTARGETS : OSVBFTESTDIR	TOOLSETINIT $(ALLTARGETS)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

OSVBFTESTDIR :
	@echo Processing os\$(P_OS)\Tests\VBFTest...

$(B_RELDIR)\vbftest.exe	: $(COMMONDEPS)	vbftest.$(B_OBJEXT)
	echo /out:$(B_RELDIR)\vbftest.exe vbftest.$(B_OBJEXT)	 >vbftest.lnk
	echo $(B_CESDKDIR)\coredll.lib				>>vbftest.lnk
	echo $(B_CESDKDIR)\corelibc.lib				>>vbftest.lnk
	echo $(B_CEOAKDIR)\storeapi.lib				>>vbftest.lnk
	echo $(B_RELDIR)\flashfx.$(B_LIBEXT)			>>vbftest.lnk
	$(B_LINK) /nodefaultlib	$(B_LFLAGS)			 @vbftest.lnk


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

vbftest.$(B_OBJEXT) : $(OSVBFTESTHEADERS) vbftest.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------
PRODLIBS =	$(FXAPPLIBS)				\
		$(DCLLIBS)				\
		$(B_DCLLIBPREFIX)dlosutil.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlwinutil.$(B_LIBEXT)	\
		fxoecmn.$(B_LIBEXT)			\
		fxoeapi.$(B_LIBEXT)

.PHONY: $(PRODLIBS)

$(PRODLIBS):
	@echo   $$(_TARGETPLATROOT)\lib\$$(_CPUINDPATH)\$(@F)	\>>prodlibs.lst

mswcepb	: OSVBFTESTDIR TOOLSETINIT $(PRODLIBS)
	@echo #								  >sources
	@echo # This is an auto-generated file -- DO NOT EDIT		 >>sources
	@echo #								 >>sources
	@echo RELEASETYPE=PLATFORM					 >>sources
	@echo TARGETNAME=vbftest					 >>sources
	@echo TARGETTYPE=PROGRAM					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo   $$(B_CESDKDIR)\coredll.lib				\>>sources
	@echo   $$(B_CESDKDIR)\ntcompat.lib				\>>sources
	@echo   $$(B_CEOAKDIR)\storeapi.lib				\>>sources
	$(B_COPY) /a sources + prodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo SOURCES=vbftest.c						 >>sources




