#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the Windows CE tools.
#	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: fximage.mak $
#	Revision 1.8  2012/02/17 16:26:42Z  johnb
#	Added dlwinutil and fxoeapi to PRODLIBS for mswcepb toolset
#	Revision 1.7  2009/02/08 16:22:39Z  garyp
#	Merged from the v4.0 branch.  Updated to link with flashfx.lib directly, rather
#	than explicitly linking with all the "app" libs.
#	Revision 1.6  2008/11/10 21:47:08Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.5  2008/11/07 15:21:17Z  johnb
#	Cleaned up whitespace and modified platform builder support
#	Revision 1.4  2008/01/07 20:09:22Z  Garyp
#	Fixed to link with fxnand.lib.
#	Revision 1.3  2007/12/14 21:35:33Z  Garyp
#	Updated to use the new DCL functions to support output redirection.
#	Revision 1.2  2006/08/25 22:17:28Z  Garyp
#	Fixed to use the proper copy command abstraction.
#	Revision 1.1  2006/03/06 01:32:40Z  Garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

ALLTARGETS =	$(B_RELDIR)\fximage.exe

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

OSTOOLSHEADERS = $(FXHEADERS)
OSTOOLSINCLUDES	=

COMMONDEPS = 	fximage.mak 				\
		$(B_RELDIR)\flashfx.$(B_LIBEXT)		\
		$(B_RELDIR)\lib\fxnand.$(B_LIBEXT)


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
	@echo Processing os\$(P_OS)\Tools\FXImage...

$(B_RELDIR)\fximage.exe	: $(COMMONDEPS)	fximage.$(B_OBJEXT)
	echo /out:$(B_RELDIR)\fximage.exe fximage.$(B_OBJEXT)	 >fximage.lnk
	echo $(B_CESDKDIR)\coredll.lib				>>fximage.lnk
	echo $(B_CESDKDIR)\corelibc.lib				>>fximage.lnk
	echo $(B_RELDIR)\flashfx.$(B_LIBEXT)			>>fximage.lnk
	echo $(B_RELDIR)\lib\fxnand.$(B_LIBEXT)			>>fximage.lnk
	$(B_LINK) /nodefaultlib	$(B_LFLAGS)			 @fximage.lnk


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

fximage.$(B_OBJEXT)  : $(OSTOOLSHEADERS)   fximage.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------
PRODLIBS =	$(FXAPPLIBS)				\
		$(DCLLIBS)				\
		$(B_DCLLIBPREFIX)dlosutil.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlwinutil.$(B_LIBEXT)	\
		fxnand.$(B_LIBEXT)			\
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
	@echo TARGETNAME=fximage					 >>sources
	@echo TARGETTYPE=PROGRAM					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo   $$(B_CESDKDIR)\coredll.lib				\>>sources
	@echo   $$(B_CESDKDIR)\ntcompat.lib				\>>sources
	$(B_COPY) /a sources + prodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo SOURCES=fximage.c						 >>sources


