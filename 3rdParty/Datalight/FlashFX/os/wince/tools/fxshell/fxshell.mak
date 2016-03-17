#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the Windows CE tools.
#	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: fxshell.mak $
#	Revision 1.7.1.2  2012/04/12 18:46:30Z  johnb
#	Updated MSWCEPB libraries.
#	Revision 1.7.1.1  2009/02/08 16:24:31Z  johnb
#	Duplicate revision
#	Revision 1.7  2009/02/08 16:24:31Z  garyp
#	Merged from the v4.0 branch.  Updated to link with flashfx.lib directly, rather
#	than explicitly linking with all the "app" libs.
#	Revision 1.6  2008/11/10 21:47:28Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.5  2008/11/07 15:22:07Z  johnb
#	Cleaned up whitespace and modified platform builder support
#	Revision 1.4  2008/01/07 20:13:55Z  Garyp
#	Fixed to link with fxnand.lib.
#	Revision 1.3  2007/12/14 21:35:32Z  Garyp
#	Updated to use the new DCL functions to support output redirection.
#	Revision 1.2  2007/06/05 18:57:45Z  rickc
#	Fixed path to storeapi.lib in the PB section.
#	Revision 1.1  2007/03/16 01:44:48Z  Garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

ALLTARGETS =	$(B_RELDIR)\fxshell.exe

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

OSTOOLSHEADERS = $(FXHEADERS)
OSTOOLSINCLUDES	=

COMMONDEPS = 	fxshell.mak 				\
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
	@echo Processing os\$(P_OS)\tools\FXShell...

$(B_RELDIR)\fxshell.exe : $(COMMONDEPS) fxshell.$(B_OBJEXT)
	echo /out:$(B_RELDIR)\fxshell.exe fxshell.$(B_OBJEXT) 	 >fxshell.lnk
	echo $(B_CESDKDIR)\coredll.lib				>>fxshell.lnk
	echo $(B_CESDKDIR)\corelibc.lib				>>fxshell.lnk
	echo $(B_CEOAKDIR)\storeapi.lib				>>fxshell.lnk
	echo $(B_RELDIR)\flashfx.$(B_LIBEXT)			>>fxshell.lnk
	echo $(B_RELDIR)\lib\fxnand.$(B_LIBEXT)			>>fxshell.lnk
	$(B_LINK) /nodefaultlib	$(B_LFLAGS)			 @fxshell.lnk


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

fxshell.$(B_OBJEXT)   : $(OSTOOLSHEADERS)   fxshell.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------
PRODLIBS =	$(FXAPPLIBS)				\
		$(DCLLIBS)				\
		$(B_DCLLIBPREFIX)dlosutil.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlostools.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlwintools.$(B_LIBEXT)	\
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
	@echo TARGETNAME=fxshell					 >>sources
	@echo TARGETTYPE=PROGRAM					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo   $$(B_CESDKDIR)\coredll.lib				\>>sources
	@echo   $$(B_CESDKDIR)\ntcompat.lib				\>>sources
	@echo   $$(B_CEOAKDIR)\storeapi.lib				\>>sources
	$(B_COPY) /a sources + prodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo SOURCES=fxshell.c						 >>sources


