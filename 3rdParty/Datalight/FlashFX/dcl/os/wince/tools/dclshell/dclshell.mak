#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the WinCE tools.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: dclshell.mak $
#	Revision 1.10.2.2  2012/04/12 18:32:50Z  johnb
#	Updated for MSWCEPB toolset.
#	Revision 1.10.2.1  2009/06/25 02:10:12Z  johnb
#	Duplicate revision
#	Revision 1.10  2009/06/25 02:10:12Z  garyp
#	Eliminated the B_PROJLIB workaround.  Updated dependencies.
#	Revision 1.9  2009/02/09 08:01:47Z  garyp
#	Added the B_PROJLIB workaround which is necessary until such time as
#	the full WM branch merge is done.
#	Revision 1.8  2009/02/07 23:03:41Z  garyp
#	Merged from the v4.0 branch.  Added link libraries.
#	Revision 1.7  2008/11/10 21:44:49Z  johnb
#	Remove B_INCLUDES default value.
#	Revision 1.6  2008/11/07 05:06:32Z  johnb
#	Cleaned up whitespace. 	Cleaned up use of B_INCLUDES.
#	Revision 1.5  2008/11/06 21:21:45Z  johnb
#	Added default $(B_INCLUDES) in PB section.
#	Revision 1.4  2008/11/05 21:35:15Z  johnb
#	Added $(B_DCLLIBPREFIX) to dcl libraries.  Changed _PROJECTOAKROOT to
#	_TARGETPLATROOT.  Added RELEASETYPE=PLATFORM.
#	Revision 1.3  2007/12/14 18:21:04Z  Garyp
#	Modified to use the new generalized CE output redirection functions.
#	Renamed the executable file name to "dlshell.exe".
#	Revision 1.2  2007/08/19 05:35:08Z  garyp
#	Added a missing link library.
#	Revision 1.1  2007/06/26 00:33:50Z  Garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

DCLSHELLHEADERS = $(DCLHEADERS) $(P_ROOT)\os\$(P_OS)\include\dlcetools.h
DCLSHELLINCLUDES =

COMMONDEPS = 	dclshell.mak 				\
		DCLLIBLST 				\
		$(B_RELDIR)\lib\dlosutil.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\dlostools.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\dlwintools.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\dlwinutil.$(B_LIBEXT)

# NOTE: The source files are named "dclshell.*", however the
#       executable file generated is "dlshell.exe".

ALLTARGETS = $(B_RELDIR)\dlshell.exe


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

DCLSHELLTARGETS : DCLSHELLDIR TOOLSETINIT $(ALLTARGETS)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

DCLSHELLDIR :
	@echo Processing os\$(P_OS)\Tools\dclshell...

$(B_RELDIR)\dlshell.exe : $(COMMONDEPS) dclshell.$(B_OBJEXT)
	echo /out:$(B_RELDIR)\dlshell.exe dclshell.$(B_OBJEXT) 		 >dclshell.lnk
	echo $(B_CESDKDIR)\coredll.lib					>>dclshell.lnk
	echo $(B_CESDKDIR)\corelibc.lib					>>dclshell.lnk
	echo $(B_CEOAKDIR)\storeapi.lib					>>dclshell.lnk
	echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlosutil.$(B_LIBEXT)	>>dclshell.lnk
	echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlostools.$(B_LIBEXT)	>>dclshell.lnk
	echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlwintools.$(B_LIBEXT)	>>dclshell.lnk
	echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlwinutil.$(B_LIBEXT)	>>dclshell.lnk
	$(B_COPY) dclshell.lnk + $(P_PROJDIR)\dcllib.lnk        	  dclshell.lnk
	$(B_LINK) /nodefaultlib	$(B_LFLAGS)				 @dclshell.lnk


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

dclshell.$(B_OBJEXT) : $(DCLSHELLHEADERS)	dclshell.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------
PRODLIBS = 	$(DCLLIBS)		\
		$(B_DCLLIBPREFIX)dlosutil.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlostools.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlwinutil.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlwintools.$(B_LIBEXT)

.PHONY: $(PRODLIBS)

$(PRODLIBS):
	@echo   $$(_TARGETPLATROOT)\lib\$$(_CPUINDPATH)\$(@F)	\>>prodlibs.lst

mswcepb	: DCLSHELLDIR TOOLSETINIT $(PRODLIBS)
# NOTE:	Beware of using	standard escape	sequences (\c, \n, etc.)
#	when specifying	paths.	Make sure to double escape (\\c, etc).
	@echo #								  >sources
	@echo # This is an auto-generated file -- DO NOT EDIT		 >>sources
	@echo #								 >>sources
	@echo RELEASETYPE=PLATFORM					 >>sources
	@echo TARGETNAME=dlshell					 >>sources
	@echo TARGETTYPE=PROGRAM					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo   $$(B_CESDKDIR)\\coredll.lib				\>>sources
	@echo   $$(B_CESDKDIR)\\ntcompat.lib				\>>sources
	@echo   $$(B_CEOAKDIR)\storeapi.lib				\>>sources
	$(B_COPY) /a sources + prodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo SOURCES=dclshell.c					 >>sources





