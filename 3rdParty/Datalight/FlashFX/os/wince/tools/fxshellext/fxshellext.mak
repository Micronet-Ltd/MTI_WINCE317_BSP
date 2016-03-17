#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the Windows CE device
#	driver.	 It is designed	for use	with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: fxshellext.mak $
#	Revision 1.6.1.2  2012/04/12 17:56:08Z  johnb
#	Updated MSWCEPB libraries.
#	Revision 1.6.1.1  2009/02/08 16:24:56Z  johnb
#	Duplicate revision
#	Revision 1.6  2009/02/08 16:24:56Z  garyp
#	Merged from the v4.0 branch.  Updated to link with flashfx.lib directly, rather
#	than explicitly linking with all the "app" libs.
#	Revision 1.5  2008/11/10 21:47:33Z  johnb
#	Remove B_INCLUDES default value
#	Revision 1.4  2008/11/07 15:22:33Z  johnb
#	Cleaned up whitespace and modified platform builder support
#	Revision 1.3  2008/01/07 20:18:20Z  Garyp
#	Fixed to link with fxnand.lib.
#	Revision 1.2  2007/12/17 02:24:31Z  Garyp
#	Corrected some link libraries.
#	Revision 1.1  2007/12/04 22:45:12Z  Garyp
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

TARGNAME = $(B_RELDIR)\fxshellext.dll

FXSHELLEXTHEADERS = $(FXHEADERS)

COMMONDEPS = 	fxshellext.mak 				\
		$(B_RELDIR)\flashfx.$(B_LIBEXT)		\
		$(B_RELDIR)\lib\fxnand.$(B_LIBEXT)


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

EXTRASRC	:=
ALLSRC      	:= $(wildcard *.c) $(EXTRASRC)
EXCLUDE     	:=
FXSHELLEXTSRC  	:= $(filter-out $(EXCLUDE), $(ALLSRC))
FXSHELLEXTSRC   := $(FXSHELLEXTSRC)
FXSHELLEXTOBJ	:= $(FXSHELLEXTSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

FXSHELLEXTTARGETS : FXSHELLEXTDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

FXSHELLEXTDIR :
	@echo Processing os\$(P_OS)\tools\fxshellext...

$(TARGNAME) : fxshellext.dll
	if exist fxshellext.rel $(B_COPY) fxshellext.rel $(B_RELDIR)\.
	if exist fxshellext.map $(B_COPY) fxshellext.map $(B_RELDIR)\.
	if exist fxshellext.pdb $(B_COPY) fxshellext.pdb $(B_RELDIR)\.
	$(B_COPY) fxshellext.dll $(TARGNAME)

fxshellext.dll : $(COMMONDEPS) $(FXSHELLEXTOBJ)
	echo /out:fxshellext.dll /dll				 >fxshellext.lnk
	echo /def:fxshellext.def $(FXSHELLEXTOBJ)		>>fxshellext.lnk
	echo /map:fxshellext.map				>>fxshellext.lnk
	echo /nodefaultlib /opt:ref				>>fxshellext.lnk
	echo /incremental:no					>>fxshellext.lnk
	echo /merge:.rdata=.text				>>fxshellext.lnk
	echo /stack:65536,4096 /base:0x10000000			>>fxshellext.lnk
	echo /savebaserelocations:fxshellext.rel		>>fxshellext.lnk
	echo /largeaddressaware					>>fxshellext.lnk
	echo $(B_LFLAGS)					>>fxshellext.lnk
	echo $(B_CESDKDIR)\coredll.lib				>>fxshellext.lnk
	echo $(B_CESDKDIR)\corelibc.lib				>>fxshellext.lnk
	echo $(B_CEOAKDIR)\storeapi.lib				>>fxshellext.lnk
	echo $(B_RELDIR)\flashfx.$(B_LIBEXT)			>>fxshellext.lnk
	echo $(B_RELDIR)\lib\fxnand.$(B_LIBEXT)			>>fxshellext.lnk
	$(B_LINK)						 @fxshellext.lnk


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

fxshellext.$(B_OBJEXT)   : $(FXSHELLEXTHEADERS)	fxshellext.c


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


mswcepb	: FXSHELLEXTDIR TOOLSETINIT $(PRODLIBS)
	@echo #								  >sources
	@echo # This is an auto-generated file -- DO NOT EDIT		 >>sources
	@echo #								 >>sources
	@echo RELEASETYPE=PLATFORM					 >>sources
	@echo TARGETNAME=fxshellext					 >>sources
	@echo TARGETTYPE=DYNLINK					 >>sources
	@echo TARGETLIBS=						\>>sources
	@echo   $$(B_CESDKDIR)\coredll.lib				\>>sources
	@echo   $$(B_CESDKDIR)\corelibc.lib				\>>sources
	@echo   $$(B_CEOAKDIR)\storeapi.lib				\>>sources
	$(B_COPY) /a sources + prodlibs.lst				   sources
	@echo.                        					 >>sources
	@echo DLLENTRY=DLLEntry						 >>sources
	@echo SOURCES=$(FXSHELLEXTSRC)					 >>sources

