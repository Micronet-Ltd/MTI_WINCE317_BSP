#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the Windows CE tools.
#	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: FlashFX_DynamicDisk.mak $
#	Revision 1.3  2012/02/24 22:23:41Z  johnb
#	Updated Platform Builder libarries.
#	Revision 1.2  2012/02/17 16:15:18Z  johnb
#	Added "RELEASETYPE=PLATFORM" to mswcepb target
#	Added fxoeapi.lib to PRODLIBS for mswcepb toolset
#	Changed _PROJECTOAKROOT to _TARGETPLATROOT
#	Revision 1.1  2009/01/14 00:42:48Z  garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

ALLTARGETS =	$(B_RELDIR)\FlashFX_DynamicDisk.exe

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

OSTOOLSHEADERS = $(FXHEADERS)
OSTOOLSINCLUDES	=

COMMONDEPS = 	FlashFX_DynamicDisk.mak $(B_RELDIR)\flashfx.$(B_LIBEXT)


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
	@echo Processing os\$(P_OS)\examples\FlashFX_DynamicDisk...

$(B_RELDIR)\FlashFX_DynamicDisk.exe : $(COMMONDEPS) FlashFX_DynamicDisk.$(B_OBJEXT)
	echo /out:$(B_RELDIR)\FlashFX_DynamicDisk.exe FlashFX_DynamicDisk.$(B_OBJEXT)	 >FlashFX_DynamicDisk.lnk
	echo $(B_CESDKDIR)\coredll.lib							>>FlashFX_DynamicDisk.lnk
	echo $(B_CESDKDIR)\corelibc.lib							>>FlashFX_DynamicDisk.lnk
	echo $(B_RELDIR)\flashfx.$(B_LIBEXT)						>>FlashFX_DynamicDisk.lnk
	$(B_LINK) /nodefaultlib	$(B_LFLAGS)						 @FlashFX_DynamicDisk.lnk


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

FlashFX_DynamicDisk.$(B_OBJEXT)   : $(OSTOOLSHEADERS)   FlashFX_DynamicDisk.c


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
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=FlashFX_DynamicDisk				 >>sources
	@echo TARGETTYPE=PROGRAM					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo   $$(B_CESDKDIR)\coredll.lib				\>>sources
	@echo   $$(B_CESDKDIR)\ntcompat.lib				\>>sources
	$(B_COPY) /a sources + prodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo SOURCES=FlashFX_DynamicDisk.c				 >>sources


