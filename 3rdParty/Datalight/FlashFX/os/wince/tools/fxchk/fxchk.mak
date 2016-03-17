#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the Windows CE tools.
#	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: fxchk.mak $
#	Revision 1.7  2012/02/17 16:21:27Z  johnb
#	Added dlwinutil and fxoeapi to PRODLIBS for mswcepb toolset
#	Revision 1.6  2009/02/08 16:21:25Z  garyp
#	Merged from the v4.0 branch.  Updated to link with flashfx.lib directly, rather
#	than explicitly linking with all the "app" libs.
#	Revision 1.5  2008/11/10 21:46:55Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.4  2008/11/07 15:23:42Z  johnb
#	Cleaned up whitespace and modified platform builder support
#	Revision 1.3  2007/12/14 21:35:34Z  Garyp
#	Updated to use the new DCL functions to support output redirection.
#	Revision 1.2  2006/08/25 22:17:31Z  Garyp
#	Fixed to use the proper copy command abstraction.
#	Revision 1.1  2005/10/01 09:35:46Z  Pauli
#	Initial revision
#	Revision 1.2  2005/08/30 07:34:43Z  Garyp
#	Modified so that DCLLIBS is specified separately from the other product
#	libraries.
#	Revision 1.1  2005/03/12 01:53:22Z  pauli
#	Initial revision
#	Revision 1.11  2005/03/11 23:53:21Z  GaryP
#	Updated to use new mechanisms for generating the MSWCEPB link libraries.
#	Revision 1.10  2005/02/27 03:43:04Z  GaryP
#	Updated to use new CE settings that work better with CE 5.
#	Revision 1.9  2005/01/17 00:59:18Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled in the traverse process.
#	Revision 1.8  2004/09/27 19:16:30Z  tonyq
#	Updated for Windows CE 5.0 Platform builder (command line build)
#	Revision 1.7  2004/09/23 21:13:56Z  tonyq
#	Updated to support building from the command line under Windows CE 5.0
#	(with the MSWCE4 toolset)
#	Revision 1.6  2004/07/07 03:05:31Z  GaryP
#	Eliminated the module header.
#	Revision 1.5  2004/07/01 18:44:30Z  GaryP
#	Updated to use new P_ and B_ build symbols.
#	Revision 1.4  2003/11/17 19:56:58Z  garys
#	Merge from FlashFXMT
#	Revision 1.3.1.2  2003/11/17 19:56:58  garyp
#	Modified to link with FXEXTAPI.LIB.
#	Revision 1.3  2003/05/22 23:31:11Z  tonyq
#	Update to work with MSWCE4 tools
#	Revision 1.2  2003/04/29 23:53:24Z  garyp
#	Eliminated references to FXVBFLOW.LIB.
#	Revision 1.1  2003/04/15 17:35:48Z  garyp
#	Initial	revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

ALLTARGETS =	$(B_RELDIR)\fxchk.exe

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

OSTOOLSHEADERS = $(FXHEADERS)
OSTOOLSINCLUDES	=

COMMONDEPS = 	fxchk.mak $(B_RELDIR)\flashfx.$(B_LIBEXT)


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

OSTOOLSTARGETS : OSTOOLSDIR TOOLSETINIT	$(ALLTARGETS)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

OSTOOLSDIR :
	@echo Processing os\$(P_OS)\Tools\FXChk...

$(B_RELDIR)\fxchk.exe :	$(COMMONDEPS) fxchk.$(B_OBJEXT)
	echo /out:$(B_RELDIR)\fxchk.exe	fxchk.$(B_OBJEXT)	 >fxchk.lnk
	echo $(B_CESDKDIR)\coredll.lib				>>fxchk.lnk
	echo $(B_CESDKDIR)\corelibc.lib				>>fxchk.lnk
	echo $(B_RELDIR)\flashfx.$(B_LIBEXT)			>>fxchk.lnk
	$(B_LINK) /nodefaultlib	$(B_LFLAGS)			 @fxchk.lnk


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

fxchk.$(B_OBJEXT)    : $(OSTOOLSHEADERS)   fxchk.c


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

mswcepb	: OSTOOLSDIR TOOLSETINIT $(PRODLIBS)
	@echo #								  >sources
	@echo # This is an auto-generated file -- DO NOT EDIT		 >>sources
	@echo #								 >>sources
	@echo RELEASETYPE=PLATFORM					 >>sources
	@echo TARGETNAME=fxchk						 >>sources
	@echo TARGETTYPE=PROGRAM					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo   $$(B_CESDKDIR)\coredll.lib				\>>sources
	@echo   $$(B_CESDKDIR)\ntcompat.lib				\>>sources
	$(B_COPY) /a sources + prodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo SOURCES=fxchk.c						 >>sources


