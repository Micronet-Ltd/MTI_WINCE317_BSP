#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FlashFX library for
#	a Windows CE Boot Loader.  It is designed for use with GNU Make or
#	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: loader.mak $
#	Revision 1.12  2012/04/16 20:31:07Z  johnb
#	Corrected copy/paste error.
#	Revision 1.11  2012/04/13 22:33:02Z  johnb
#	Updates to allow WinceBL OS to build in with MSWCEPB toolset.
#	Revision 1.10  2012/04/12 20:25:18Z  johnb
#	Updates for MSWCEPB toolset.
#	Revision 1.9  2009/07/22 01:26:16Z  garyp
#	Merged from the v4.0 branch.  Removed ffxloader.c.
#	Revision 1.8  2009/02/09 01:08:35Z  garyp
#	Added an include path.
#	Revision 1.7  2008/04/20 21:57:44Z  garyp
#	Fixed broken logic so the library is generated properly.
#	Revision 1.6  2008/03/08 22:16:32Z  Garyp
#	Added ffxloader.c and ffxfmdwm.c.
#	Revision 1.5  2007/01/11 03:34:06Z  Garyp
#	Removed some obsolete comments.
#	Revision 1.4  2006/08/28 03:15:03Z  Garyp
#	Enclosed a copy command in quotes so the silly GNU MAKE does not massage
#	the string improperly when it things there is an SH in the system.
#	Revision 1.3  2006/08/25 22:17:25Z  Garyp
#	Fixed to use the proper copy command abstraction.
#	Revision 1.2  2006/03/15 03:34:55Z  Garyp
#	Updated to use the new standard mechanism for building combined
#	libraries.
#	Revision 1.1  2005/10/14 02:08:30Z  Pauli
#	Initial revision
#	Revision 1.3  2005/01/17 00:04:39Z  GaryP
#	Modified to no longer change into the target directory since that is
#	already handled by the traverse process.  Modified to use the new DCL
#	shared CPU libraries.
#	Revision 1.2  2004/08/28 20:58:55Z  GaryP
#	Fixed dependencies so everything is properly rebuilt as needed.
#	Revision 1.1  2004/08/17 17:14:06Z  garyp
#	Initial revision
#----------------------------------------------------------------------------


#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = fxboot.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\$(TARGFILE)

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

LOADERHEADERS =	$(FXHEADERS)

# Extra include search path for code built in this directory
B_LOCALINC = ../../win/include


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard ffx*.c)
EXCLUDE     	:=
LOADERSRC     	:= $(filter-out $(EXCLUDE), $(ALLSRC))
LOADEROBJ	:= $(LOADERSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

LOADERTARGETS :	LOADERINIT TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

LOADERINIT :
	@echo Processing os\$(P_OS)\Loader...
	$(B_COPY) "$(B_RELDIR)\lib\*.$(B_LIBEXT)"
	if exist $(TARGFILE) del $(TARGFILE)

# This allows us to conditionally adds the DCL libs to the PRODLIBS list.
# If we are building as a subproduct, the DCL libs will be added by the
# "master" product.
ifndef P_SUBPRODUCT
DRVDCLLIBS =	$(DCLLIBS)
else
DRVDCLLIBS =
endif

# This lists every library to be included in the all-inclusive FlashFX
# library.  This definition MUST appear physically in the MAKE file
# before it is referenced.
PRODLIBS =	$(FXDRVLIBS)		\
		$(DRVDCLLIBS)		\
		fxldr.$(B_LIBEXT)	\
 		fxoecmn.$(B_LIBEXT)	\
 		fxoetool.$(B_LIBEXT)

.PHONY: $(PRODLIBS)

$(PRODLIBS) : loader.mak
	$(B_ADDLIBS) $(TARGFILE) $(@F)

$(TARGNAME) : $(TARGFILE)
	$(B_COPY) $(TARGFILE) $(TARGNAME)

$(TARGFILE) : fxloader.$(B_LIBEXT) $(PRODLIBS)

# Make a temporary name for fxloader.lib to be used in the PRODLIBS
# step.  Can't be the same name or MAKE complains.
fxloader.$(B_LIBEXT) : loader.mak $(LOADEROBJ)
	if exist fxloader.$(B_LIBEXT) del fxloader.$(B_LIBEXT)
	$(B_BUILDLIB) fxloader.$(B_LIBEXT) *.$(B_OBJEXT)
	$(B_COPY) fxloader.$(B_LIBEXT) "$(B_RELDIR)\lib\."
	$(B_COPY) fxloader.$(B_LIBEXT) fxldr.*


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

ffxfmdwm.$(B_OBJEXT)	: $(LOADERHEADERS) ffxfmdwm.c


#--------------------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#--------------------------------------------------------------------

PBLOADERDIR :
	@echo Processing os\$(P_OS)\Loader...
	@if exist pbprodlibs.lst del pbprodlibs.lst

PBPRODLIBS =	$(FXDRVLIBS)				\
		$(DRVDCLLIBS)				\
		fxoecmn.$(B_LIBEXT)			\
		fxoetool.$(B_LIBEXT)


PBLDRLIBLIST := $(foreach l, $(PBPRODLIBS),  $$(_TARGETPLATROOT)\lib\$$(_CPUINDPATH)\$(notdir $l))

mswcepb	: PBLOADERDIR TOOLSETINIT 
	@echo #							  >sources
	@echo # This is an auto-generated file -- DO NOT EDIT	 >>sources
	@echo #							 >>sources
	@echo RELEASETYPE=PLATFORM				 >>sources
	@echo TARGETNAME=fxboot					 >>sources
	@echo TARGETTYPE=LIBRARY				 >>sources
	@echo SOURCELIBS=					\>>sources
	@echo $(PBLDRLIBLIST)					 >>sources
	$(B_COPY) /a sources + pbprodlibs.lst			   sources
	@echo.                        				>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(LOADERSRC)				>>sources


