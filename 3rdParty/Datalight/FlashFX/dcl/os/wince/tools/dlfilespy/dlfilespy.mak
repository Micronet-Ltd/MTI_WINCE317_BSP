#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the WinCE tools.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: dlfilespy.mak $
#	Revision 1.2  2012/02/17 19:12:23Z  johnb
#	Added dlwinutil.lib to PRODLIBS for mwscepb toolset
#	Changed DllEntry to DllMain
#	Revision 1.1  2011/04/21 02:07:08Z  garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

TARGBASE = dlfilespy
TARGFILE = $(TARGBASE).dll
TARGNAME = $(B_RELDIR)\$(TARGFILE)

DCLSPYHEADERS = $(DCLHEADERS) $(P_ROOT)\os\$(P_OS)\include\dlcetools.h
DCLSPYINCLUDES =

COMMONDEPS = 	dlfilespy.mak 				\
		DCLLIBLST 				\
		$(B_RELDIR)\lib\dlostools.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\dlosutil.$(B_LIBEXT)	\
		$(B_RELDIR)\lib\dlwinutil.$(B_LIBEXT)


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

EXTRASRC	:=
ALLSRC      	:= $(wildcard *.c) $(EXTRASRC)
EXCLUDE     	:=
DCLSPYSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
DCLSPYSRC       := $(DCLSPYSRC)
DCLSPYOBJ	:= $(DCLSPYSRC:.c=.$(B_OBJEXT))
 

#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

DCLSPYTARGETS : DCLSPYDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

DCLSPYDIR :
	@echo Processing os\$(P_OS)\tools\DLFileSpy...

$(TARGNAME) : $(TARGFILE)
	if exist dlfilespy.rel $(B_COPY) dlfilespy.rel $(B_RELDIR)\.
	if exist dlfilespy.map $(B_COPY) dlfilespy.map $(B_RELDIR)\.
	if exist dlfilespy.pdb $(B_COPY) dlfilespy.pdb $(B_RELDIR)\.
	$(B_COPY) $(TARGFILE) $(TARGNAME)

dlfilespy.dll : $(COMMONDEPS) $(DCLSPYOBJ)
	echo /out:$(TARGFILE) /dll					 >dlfilespy.lnk
	echo /def:dlfilespy.def $(DCLSPYOBJ)				>>dlfilespy.lnk
	echo /map:dlfilespy.map						>>dlfilespy.lnk
	echo /nodefaultlib /opt:ref					>>dlfilespy.lnk
	echo /incremental:no						>>dlfilespy.lnk
	echo /merge:.rdata=.text					>>dlfilespy.lnk
	echo /stack:65536,4096 /base:0x10000000				>>dlfilespy.lnk
	echo /savebaserelocations:dlfilespy.rel				>>dlfilespy.lnk
	echo /largeaddressaware						>>dlfilespy.lnk
	echo $(B_LFLAGS)						>>dlfilespy.lnk
	echo $(B_CESDKDIR)\coredll.lib					>>dlfilespy.lnk
	echo $(B_CESDKDIR)\corelibc.lib					>>dlfilespy.lnk
	echo $(B_CEOAKDIR)\storeapi.lib					>>dlfilespy.lnk
	echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlosutil.$(B_LIBEXT)	>>dlfilespy.lnk
	echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlostools.$(B_LIBEXT)	>>dlfilespy.lnk
	echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlwintools.$(B_LIBEXT)	>>dlfilespy.lnk
	echo $(B_RELDIR)\lib\$(B_DCLLIBPREFIX)dlwinutil.$(B_LIBEXT)	>>dlfilespy.lnk
	$(B_COPY) /a dlfilespy.lnk + $(P_PROJDIR)\dcllib.lnk   		  dlfilespy.lnk
	$(B_LINK)							 @dlfilespy.lnk


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

dlfilespy.$(B_OBJEXT) : $(DCLSPYHEADERS)	dlfilespy.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------
PRODLIBS = 	$(DCLLIBS)				\
		$(B_DCLLIBPREFIX)dlostools.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlosutil.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlwinutil.$(B_LIBEXT)	\
		$(B_DCLLIBPREFIX)dlwintools.$(B_LIBEXT)

.PHONY: $(PRODLIBS)

$(PRODLIBS):
	@echo   $$(_TARGETPLATROOT)\lib\$$(_CPUINDPATH)\$(@F)	\>>prodlibs.lst

mswcepb	: DCLSPYDIR TOOLSETINIT $(PRODLIBS)
# NOTE:	Beware of using	standard escape	sequences (\c, \n, etc.)
#	when specifying	paths.	Make sure to double escape (\\c, etc).
	@echo #								  >sources
	@echo # This is an auto-generated file -- DO NOT EDIT		 >>sources
	@echo #								 >>sources
	@echo RELEASETYPE=PLATFORM					 >>sources
	@echo TARGETNAME=dlfilespy				 	 >>sources
	@echo TARGETTYPE=DYNLINK					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo   $$(B_CESDKDIR)\\coredll.lib				\>>sources
	@echo   $$(B_CESDKDIR)\\corelibc.lib				\>>sources
	@echo   $$(B_CEOAKDIR)\storeapi.lib				\>>sources
	$(B_COPY) /a sources + prodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo DLLENTRY=DllMain						 >>sources
	@echo SOURCES=$(DCLSPYSRC)					 >>sources

