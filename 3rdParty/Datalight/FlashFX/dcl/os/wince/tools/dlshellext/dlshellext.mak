#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the Windows CE device
#	driver.	 It is designed	for use	with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: dlshellext.mak $
#	Revision 1.9  2009/06/25 02:07:13Z  garyp
#	Eliminated the B_PROJLIB workaround.
#	Revision 1.8  2009/02/09 08:00:11Z  garyp
#	Added the B_PROJLIB workaround which is necessary until such time as
#	the full WM branch merge is done.
#	Revision 1.7  2009/02/07 23:03:41Z  garyp
#	Merged from the v4.0 branch.  Added link libraries.
#	Revision 1.6  2008/11/10 21:44:57Z  johnb
#	Remove B_INCLUDES default value.
#	Revision 1.5  2008/11/07 05:06:57Z  johnb
#	Cleaned up whitespace.  Cleaned up use of B_INCLUDES.
#	Revision 1.4  2008/11/06 21:21:37Z  johnb
#	Added default $(B_INCLUDES) in PB section.
#	Revision 1.3  2008/11/05 21:35:39Z  johnb
#	Added $(B_DCLLIBPREFIX) to dcl libraries.  Changed _PROJECTOAKROOT to
#	_TARGETPLATROOT.  Added RELEASETYPE=PLATFORM.
#	Revision 1.2  2007/12/17 02:18:53Z  Garyp
#	Corrected link libraries.
#	Revision 1.1  2007/12/14 17:49:34Z  Garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

TARGNAME = $(B_RELDIR)\dlshellext.dll

CESHELLHEADERS = $(DCLHEADERS) $(P_ROOT)\os\$(P_OS)\include\dlcetools.h

COMMONDEPS = 	dlshellext.mak				\
		DCLLIBLST				\
		$(B_RELDIR)\lib\dlosutil.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\dlostools.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\dlwintools.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\dlwinutil.$(B_LIBEXT)


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

EXTRASRC	:=
ALLSRC      	:= $(wildcard *.c) $(EXTRASRC)
EXCLUDE     	:=
CESHELLSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
CESHELLSRC      := $(CESHELLSRC)
CESHELLOBJ	:= $(CESHELLSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

CESHELLTARGETS : CESHELLDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

CESHELLDIR :
	@echo Processing os\$(P_OS)\tools\dlshellext...

$(TARGNAME) : dlshellext.dll
	if exist dlshellext.rel $(B_COPY) dlshellext.rel $(B_RELDIR)\.
	if exist dlshellext.map $(B_COPY) dlshellext.map $(B_RELDIR)\.
	if exist dlshellext.pdb $(B_COPY) dlshellext.pdb $(B_RELDIR)\.
	$(B_COPY) dlshellext.dll $(TARGNAME)

dlshellext.dll : $(COMMONDEPS) $(CESHELLOBJ)
	echo /out:dlshellext.dll /dll					 >dlshellext.lnk
	echo /def:dlshellext.def $(CESHELLOBJ)				>>dlshellext.lnk
	echo /map:dlshellext.map					>>dlshellext.lnk
	echo /nodefaultlib /opt:ref					>>dlshellext.lnk
	echo /incremental:no						>>dlshellext.lnk
	echo /merge:.rdata=.text					>>dlshellext.lnk
	echo /stack:65536,4096 /base:0x10000000				>>dlshellext.lnk
	echo /savebaserelocations:dlshellext.rel			>>dlshellext.lnk
	echo /largeaddressaware						>>dlshellext.lnk
	echo $(B_LFLAGS)						>>dlshellext.lnk
	echo $(B_CESDKDIR)\coredll.lib					>>dlshellext.lnk
	echo $(B_CESDKDIR)\corelibc.lib					>>dlshellext.lnk
	echo $(B_CEOAKDIR)\storeapi.lib					>>dlshellext.lnk
	echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlosutil.$(B_LIBEXT)	>>dlshellext.lnk
	echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlostools.$(B_LIBEXT)	>>dlshellext.lnk
	echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlwintools.$(B_LIBEXT)	>>dlshellext.lnk
	echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlwinutil.$(B_LIBEXT)	>>dlshellext.lnk
	$(B_COPY) /a dlshellext.lnk + $(P_PROJDIR)\dcllib.lnk   	  dlshellext.lnk
	$(B_LINK)							 @dlshellext.lnk


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

dlshellext.$(B_OBJEXT)   : $(CESHELLHEADERS)	dlshellext.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------
PRODLIBS = 	$(DCLLIBS)		\
		$(B_DCLLIBPREFIX)dlosutil.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlostools.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlwintools.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlwinutil.$(B_LIBEXT)

.PHONY: $(PRODLIBS)

$(PRODLIBS):
	@echo   $$(_TARGETPLATROOT)\lib\$$(_CPUINDPATH)\$(@F)	\>>prodlibs.lst


mswcepb	: CESHELLDIR TOOLSETINIT $(PRODLIBS)
# NOTE:	Beware of using	standard escape	sequences (\c, \n, etc.)
#	when specifying	paths.	Make sure to double escape (\\c, etc).
	@echo #								  >sources
	@echo # This is an auto-generated file -- DO NOT EDIT		 >>sources
	@echo #								 >>sources
	@echo RELEASETYPE=PLATFORM					 >>sources
	@echo TARGETNAME=dlshellext					 >>sources
	@echo TARGETTYPE=DYNLINK					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo   $$(B_CESDKDIR)\\coredll.lib				\>>sources
	@echo   $$(B_CESDKDIR)\\corelibc.lib				\>>sources
	@echo   $$(B_CEOAKDIR)\storeapi.lib				\>>sources
	$(B_COPY) /a sources + prodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo DLLENTRY=DLLEntry						 >>sources
	@echo SOURCES=$(CESHELLSRC)					 >>sources

